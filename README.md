# LustageController_20241010pp1

光源控制器固件工程，基于 STM32F407 + ThreadX + NetX DUO。

- 通过网口（UDP）接收上位机下发的 LUSTER 协议数据帧；
- 解析后通过 8 路 RS485 广播给光源；
- 本仓库在原始工程基础上实现了命令字 `0x0355`（查询指定光源灯板温度）的“下发 → 接收 → 回传”功能，并修复了原始工程中的若干问题。

## 1. 系统通信架构

```
上位机(PC, 192.168.0.100:8888) --(UDP)--> 控制板(192.168.0.1:8888) --(8路RS485)--> 光源
                              <--(UDP)---                            <--(RS485)---
```

## 2. 通信帧格式（LUSTER 协议）

```
4C 55 | Len(2, 小端) | Cmd(2, 大端) | Data... | CheckSum(1)
```

- `Len` = Cmd + Data 的总字节数（不含帧头与校验）。
- `CheckSum` = Cmd 起、到最后一个 Data 的字节累加，取低 8 位。

### 0x0355 温度查询（下发给光源）

```
4C 55 04 00 03 55 ID_L ID_H CS        // 9 字节
```

### 0x0355 温度应答（光源返回，控制器原样 UDP 转发）

```
4C 55 05 00 03 55 ID_L ID_H TEMP CS   // 10 字节
```

## 3. 相对原始代码的改进及原因

### 3.1 新增 0x0355 温度查询转发功能

原始控制器只有“UDP 收包 → 8 路 RS485 广播下发”，**没有任何 UART 接收能力**，因此无法把光源的应答回传给上位机。

本实现新增：

- 8 路 UART 固定 10 字节接收（`Frame_UART_Start_Receive()` + `HAL_UART_Receive_IT`）；
- 应答帧完整校验（`HAL_UART_RxCpltCallback`）；
- 独立转发线程 + 信号量（`AppTaskTempResp` + `TempRespSemaphore`），把光源的 10 字节应答**原样**通过 UDP 回传。

原因：协议要求“控制器接收光源返回命令后，转发光源返回命令”，因此控制器必须能收 RS485 数据并原样转 UDP。

### 3.2 修复原始代码 `LightSource_Counter > 1` → `>= 1`

`System_Frequency_Start_Handler()` 中：

```c
// 原始（错误）
if((Frame_Param_Struct.LightSource_Counter > 1) && (Frame_Param_Struct.LightSource_Frequency > 0))
// 修正后
if((Frame_Param_Struct.LightSource_Counter >= 1) && (Frame_Param_Struct.LightSource_Frequency > 0))
```

原因：`LightSource_Counter == 1` 时原始代码无法进入频闪/触发启动逻辑（边界值判断错误）；与“确认无误版”（Light_Controller/10-10-V10-21）对照后确认应为 `>= 1`。

### 3.3 485 方向时序分离（本次排障的关键点之一）

温度查询需要“发完立即收应答”；固件升级及其它广播是单向下发，DE 时序应保持原始的 `Frame_UART_RX_TX() → tx_thread_sleep → Frame_UART_RX_TX_LOW()`。

最终方案：`HAL_UART_TxCpltCallback` 只在 `TempQueryPending` 为真时才逐路切回收：

```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(TempQueryPending)   // 仅温度查询才逐路切回
    {
        if(huart == &huart1)      RS485_1_Mode_Rx();
        else if(huart == &huart2) RS485_2_Mode_Rx();
        // ... 每路对应
    }
}
```

原因：初版在回调里**无条件**切回 DE，导致固件升级转发时 DE 被提前拉低、509 字节数据帧被截断，光源升级后卡死。加上 `TempQueryPending` 门控后，固件升级等其它广播完全回到原始时序。

### 3.4 转发线程超时分支不再动 DE / 串口（修复升级卡死的根本原因）

初版转发线程超时时会调用 `Frame_UART_Start_Receive()`，而它内部有 `Frame_UART_RX_TX_LOW()`，会把 8 路 DE 全部拉低。该线程每 50 tick 超时一次，**即使没有任何温度查询，也会在后台周期性地把 DE 拉低**，在固件升级发送过程中截断数据帧。

最终改为超时只清标志：

```c
else
{
    /* 超时未收到应答：仅清除标志；不重装接收、不动DE */
    TempQueryPending = 0;
}
```

原因：后台周期任务/中断绝对不能动 485 方向，否则会破坏正在进行的任何广播/升级。

### 3.5 ISR 安全

接收中断 `HAL_UART_RxCpltCallback` 里只做：校验、拷贝 10 字节、`tx_semaphore_put()`；NetX 的 `nx_udp_socket_send` 全部放在 `AppTaskTempResp` 线程里。

原因：NetX/ThreadX 的阻塞 API 不允许在中断上下文调用。

### 3.6 去掉中断里的等待循环

旧版 HAL 的 `HAL_UART_TxCpltCallback` 是在 UART **TC 中断**里被调用的，此时 TC 已置位，所以 `while(!TC)` 等待是多余的；且中断里长时间等待是坏习惯，已删除。

## 4. 实现细节（最终代码）

### 4.1 命令字定义（application.h）

```c
#define Frame_INFO_CMD_LightSource_Temperature        0x5503
```

控制板 `Frame_Param_Struct.CMD_Union.buff` 是“命令字在内存里的小端值”，线序 `03 55` 对应内部值 `0x5503`，与其它 `Frame_CFG_CMD_*` 写法一致。

### 4.2 温度查询处理分支（Frame_Data_Handler）

```c
case Frame_INFO_CMD_LightSource_Temperature:
{
    if(TempQueryPending == 0)
    {
        TempQueryID = (Frame_Param_Struct.Data[1] << 8) | Frame_Param_Struct.Data[0];
        TempQueryPending = 1;

        Frame_UART_RX_TX();   // 8路DE切发送
        Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct, Frame_Param_Struct.FrameSize);
        // ... huart2..huart8 同样
        // 各路DE回接收由 HAL_UART_TxCpltCallback 在发送完成后逐路完成
    }
}
break;
```

要点：

- `TempQueryPending` 作为“单飞”保护：上一笔未完成时忽略新查询。
- 查询帧在 NetXPro 线程内直接使用 `Frame_Param_Struct`（该结构此时有效），避免跨线程访问被下一帧覆盖。

### 4.3 8 路 UART 接收初始化

```c
#define UART_RxBuffSize 10
static uint8_t UART1_RxBuff[UART_RxBuffSize];   // 共8路

void Frame_UART_Start_Receive(void)
{
    Frame_UART_RX_TX_LOW();               // 8路DE全部切接收

    HAL_UART_AbortReceive(&huart1);       // 清空各路旧接收
    // ... huart2..huart8 同样

    HAL_UART_Receive_IT(&huart1, UART1_RxBuff, UART_RxBuffSize);
    // ... huart2..huart8 同样
}
```

该函数**只在 `main.c` 上电初始化时调用一次**（转发线程超时分支不再调用，避免动 DE）。

> 注：本工程 HAL 为旧版，无 `HAL_UARTEx_ReceiveToIdle_IT`，故采用固定 10 字节 `HAL_UART_Receive_IT`；光源应答固定 10 字节。

### 4.4 接收校验与通知（不在中断里发 UDP）

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t *pRx = /* 对应路的 RxBuff */;

    if(TempQueryPending != 0)
    {
        // 校验 4C55 / Len=5 / Cmd=0355 / 校验和 / 光源ID==TempQueryID
        if(校验通过)
        {
            拷贝10字节到 TempRespFrame;
            tx_semaphore_put(&TempRespSemaphore);   // ISR中只发信号量
        }
    }
    HAL_UART_Receive_IT(huart, pRx, UART_RxBuffSize);  // 重装本路接收
}
```

### 4.5 错误恢复

```c
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    HAL_UART_AbortReceive(huart);
    HAL_UART_Receive_IT(huart, /* 对应RxBuff */, UART_RxBuffSize);
}
```

### 4.6 温度应答转发线程（在任务上下文里发 UDP）

```c
TX_SEMAPHORE TempRespSemaphore;   // 全局

static void AppTaskTempResp(ULONG thread_input)
{
    (void)thread_input;

    while(1)
    {
        if(tx_semaphore_get(&TempRespSemaphore, 50) == TX_SUCCESS)
        {
            TempQueryPending = 0;
            Frame_UDP_Data_Send_Raw(TempRespFrame, 10);   // 原样转发10字节
        }
        else
        {
            TempQueryPending = 0;   // 超时只清标志，不动 DE / 不重装接收
        }
    }
}
```

在 `tx_application_define()` 中创建信号量与线程：

```c
tx_semaphore_create(&TempRespSemaphore, "Temp Resp Sem", 0);
tx_thread_create(&AppTaskTempRespTCB, "App Temp Resp", AppTaskTempResp, ...);
```

### 4.7 原样 UDP 回传

```c
static void Frame_UDP_Data_Send_Raw(uint8_t *pData, uint16_t Size)
{
    nx_packet_allocate(&pool_0, &TxPacket, NX_UDP_PACKET, TX_WAIT_FOREVER);
    nx_packet_data_append(TxPacket, pData, Size, &pool_0, TX_WAIT_FOREVER);
    nx_udp_socket_send(&UDPSocket, TxPacket, source_ip_address, source_port);
}
```

### 4.8 main.c 初始化

```c
#include "application.h"
...
MX_USART6_UART_Init();
Frame_UART_Start_Receive();   // 挂起8路接收
tx_kernel_enter();
```

## 5. 修改过程注意事项（经验教训）

1. **后台周期任务 / 中断不要动 485 方向**：任何无条件或周期性的 DE 切换，都会在广播 / 固件升级发送过程中截断数据帧。这是本次“升级后光源卡死”的根本原因。
2. **发送完成回调要区分场景**：温度查询（要立即收应答）与固件升级（单向广播）共用 8 路 UART，必须用标志（`TempQueryPending`）区分，不能无条件切 DE。
3. **中断里不要 busy-wait**：尤其不要 `while(!TC)`；旧版 HAL 的 TxCpltCallback 已在 TC 中断内（TC 已置位），直接切 DE 即可。
4. **不在 ISR 里调 NetX/ThreadX**：接收中断用 `tx_semaphore_put` 通知，UDP 发送放到独立线程。
5. **回传要“原样转发”**：直接转发光源的 10 字节应答，不要重新拆 / 组包，避免命令字、字节序出错。
6. **“升级成功”提示不可靠**：光源固件升级的 `data` 命令算出了 flash 写结果但不上报，控制器无条件回 `0x01`；排查升级问题应回读 flash 与 bin 对比。
7. **命令字字节序两侧不一致**：控制板内部 `0x0355` 对应 `0x5503`（字节交换后的值），光源侧直接是 `0x0355`，不要写反。
8. **旧版 HAL 没有 `HAL_UARTEx_ReceiveToIdle_IT`**：接收用固定 10 字节 `HAL_UART_Receive_IT` + 超时恢复。

## 6. 编译与烧录

- Keil MDK 打开 `10-10-V10-21/Project/MDK-ARM(AC5)/project.uvprojx` 编译。
- 通过 SWD/ST-Link 烧录到控制板。

## 7. 联调测试（完整链路）

1. 分别烧录光源、控制板新固件。
2. PC 网口 IP 设为 `192.168.0.100`。
3. 网络调试助手 UDP：本地绑定 `192.168.0.100:8888`。
4. 向 `192.168.0.1:8888` 发送 HEX：

   ```
   4C 55 04 00 03 55 01 00 59
   ```

   （查 ID=1；查其它 ID 按 `CS = 03+55+ID_L+ID_H` 计算）

5. 期望收到：

   ```
   4C 55 05 00 03 55 01 00 <TEMP> <CS>
   ```

例如 30℃：`4C 55 05 00 03 55 01 00 1E 77`。

## 8. 注意事项

- 回传 UDP 目标沿用工程现有约定 `192.168.0.100:8888`。
- 若某串口连接的灯未按 ID 匹配，不会应答；控制器 50 tick 超时后自动恢复。