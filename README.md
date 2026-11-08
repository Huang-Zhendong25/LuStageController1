# LustageController_20241010pp1

光源控制器固件工程，基于 STM32F407 + ThreadX + NetX DUO。

- 通过网口（UDP）接收上位机下发的 LUSTER 协议数据帧；
- 解析后通过 8 路 RS485 广播给光源；
- 本仓库在原始工程基础上实现了命令字 `0x0355`（查询指定光源灯板温度）的“下发 -> 接收 -> 回传”功能。

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

## 3. 本功能在控制板侧的改动

### 修改文件

- `10-10-V10-21/HardWare/Application/application.h`
- `10-10-V10-21/HardWare/Application/application.c`
- `10-10-V10-21/User/main.c`

### 3.1 命令字定义（application.h）

```c
#define Frame_INFO_CMD_LightSource_Temperature        0x5503   // 查询指定光源灯板温度
```

说明：控制板 `Frame_Param_Struct.CMD_Union.buff` 是“命令字在内存里的小端值”，线序 `03 55` 对应内部值 `0x5503`，与其它 `Frame_CFG_CMD_*` 的写法一致。

### 3.2 UDP 查询处理分支（application.c, Frame_Data_Handler）

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

- 上位机一次只查一个灯，`TempQueryPending` 作为“单飞”保护：上一笔未完成时忽略新查询。
- 查询帧在 NetXPro 线程内直接使用 `Frame_Param_Struct`（该结构此时有效），避免跨线程访问被下一帧覆盖。

### 3.3 8 路 UART 接收初始化

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

> 注：本工程 HAL 为旧版，无 `HAL_UARTEx_ReceiveToIdle_IT`，故采用固定 10 字节 `HAL_UART_Receive_IT`；光源应答固定 10 字节，配合超时重装保证可靠。

### 3.4 发送完成后 DE 回接收（逐路，等 TC）

```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    uint32_t Timeout = 10000;
    while((!__HAL_UART_GET_FLAG(huart, UART_FLAG_TC)) && (Timeout--)) {}

    if(huart == &huart1)      RS485_1_Mode_Rx();
    else if(huart == &huart2) RS485_2_Mode_Rx();
    // ... 每路对应
}
```

### 3.5 接收校验与通知（不在中断里发 UDP）

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

### 3.6 错误恢复

```c
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    HAL_UART_AbortReceive(huart);
    HAL_UART_Receive_IT(huart, /* 对应RxBuff */, UART_RxBuffSize);
}
```

### 3.7 温度应答转发线程（在任务上下文里发 UDP）

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
            TempQueryPending = 0;       // 超时清除
            Frame_UART_Start_Receive(); // 重装8路接收
        }
    }
}
```

在 `tx_application_define()` 中创建信号量与线程：

```c
tx_semaphore_create(&TempRespSemaphore, "Temp Resp Sem", 0);
tx_thread_create(&AppTaskTempRespTCB, "App Temp Resp", AppTaskTempResp, ...);
```

### 3.8 原样 UDP 回传

```c
static void Frame_UDP_Data_Send_Raw(uint8_t *pData, uint16_t Size)
{
    nx_packet_allocate(&pool_0, &TxPacket, NX_UDP_PACKET, TX_WAIT_FOREVER);
    nx_packet_data_append(TxPacket, pData, Size, &pool_0, TX_WAIT_FOREVER);
    nx_udp_socket_send(&UDPSocket, TxPacket, source_ip_address, source_port);
}
```

### 3.9 main.c 初始化

```c
#include "application.h"
...
MX_USART6_UART_Init();
Frame_UART_Start_Receive();   // 挂起8路接收
tx_kernel_enter();
```

## 4. 编译与烧录

- Keil MDK 打开 `10-10-V10-21/Project/MDK-ARM(AC5)/project.uvprojx` 编译。
- 通过 SWD/ST-Link 烧录到控制板。

## 5. 联调测试（完整链路）

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

## 6. 关键设计说明

- **ISR 安全**：UART 接收中断里只做校验、拷贝、`tx_semaphore_put`；NetX 的 `nx_udp_socket_send` 一律在 `AppTaskTempResp` 线程中调用，避免在中断里调用 NetX。
- **485 方向**：DE 等 TC 后再拉低，保证最后一个字节发完；逐路独立拉低，查询后能最快切回收等待应答。
- **正确接收**：固定 10 字节接收 + 完整校验（帧头/长度/命令/校验和/ID），并只在“查询进行中”时接受，防止串扰。
- **超时恢复**：50 tick 无应答则清除标志并重装 8 路接收，保证下一笔查询可用。
- **单飞保护**：`TempQueryPending` 保证同一时刻只处理一笔温度查询，与“上位机一次只查一个光源”一致。

## 7. 注意事项

- 回传 UDP 目标沿用工程现有约定 `192.168.0.100:8888`。
- 若某串口连接的灯未按 ID 匹配，不会应答；控制器 50 tick 超时后自动恢复。