#include "tx_api.h"
#include "includes.h"
#include "application.h"
#include "usart.h"
#include "nx_api.h"
#include "bsp_flash.h"
#include "tim.h"
/***********************RS-485-RE引脚接收和发送配置*************************************/
#define RS485_1_Mode_Rx()     HAL_GPIO_WritePin(USART1_DE_nRE_GPIO_Port, USART1_DE_nRE_Pin, GPIO_PIN_RESET)
#define RS485_1_Mode_Tx()     HAL_GPIO_WritePin(USART1_DE_nRE_GPIO_Port, USART1_DE_nRE_Pin, GPIO_PIN_SET)

#define RS485_2_Mode_Rx()     HAL_GPIO_WritePin(USART2_DE_nRE_GPIO_Port, USART2_DE_nRE_Pin, GPIO_PIN_RESET)
#define RS485_2_Mode_Tx()     HAL_GPIO_WritePin(USART2_DE_nRE_GPIO_Port, USART2_DE_nRE_Pin, GPIO_PIN_SET)

#define RS485_3_Mode_Rx()     HAL_GPIO_WritePin(USART3_DE_nRE_GPIO_Port, USART3_DE_nRE_Pin, GPIO_PIN_RESET)
#define RS485_3_Mode_Tx()     HAL_GPIO_WritePin(USART3_DE_nRE_GPIO_Port, USART3_DE_nRE_Pin, GPIO_PIN_SET)

#define RS485_4_Mode_Rx()     HAL_GPIO_WritePin(UART4_DE_nRE_GPIO_Port, UART4_DE_nRE_Pin, GPIO_PIN_RESET)
#define RS485_4_Mode_Tx()     HAL_GPIO_WritePin(UART4_DE_nRE_GPIO_Port, UART4_DE_nRE_Pin, GPIO_PIN_SET)

#define RS485_5_Mode_Rx()     HAL_GPIO_WritePin(UART5_DE_nRE_GPIO_Port, UART5_DE_nRE_Pin, GPIO_PIN_RESET)
#define RS485_5_Mode_Tx()     HAL_GPIO_WritePin(UART5_DE_nRE_GPIO_Port, UART5_DE_nRE_Pin, GPIO_PIN_SET)

#define RS485_6_Mode_Rx()     HAL_GPIO_WritePin(USART6_DE_nRE_GPIO_Port, USART6_DE_nRE_Pin, GPIO_PIN_RESET)
#define RS485_6_Mode_Tx()     HAL_GPIO_WritePin(USART6_DE_nRE_GPIO_Port, USART6_DE_nRE_Pin, GPIO_PIN_SET)

#define RS485_7_Mode_Rx()     HAL_GPIO_WritePin(UART7_DE_nRE_GPIO_Port, UART7_DE_nRE_Pin, GPIO_PIN_RESET)
#define RS485_7_Mode_Tx()     HAL_GPIO_WritePin(UART7_DE_nRE_GPIO_Port, UART7_DE_nRE_Pin, GPIO_PIN_SET)

#define RS485_8_Mode_Rx()     HAL_GPIO_WritePin(UART8_DE_nRE_GPIO_Port, UART8_DE_nRE_Pin, GPIO_PIN_RESET)
#define RS485_8_Mode_Tx()     HAL_GPIO_WritePin(UART8_DE_nRE_GPIO_Port, UART8_DE_nRE_Pin, GPIO_PIN_SET)
/************************************************************/

/*任务优先级，数值越小优先级越高*/
#define  APP_CFG_TASK_NetXPro_PRIO                        29u 
#define  APP_CFG_TASK_Default_PRIO                        30u 

/*任务栈大小，单位字节*/
#define  APP_CFG_TASK_NETXPRO_STK_SIZE                  4096u
#define  APP_CFG_TASK_DEFAULT_STK_SIZE                   256u
#define  APP_CFG_TASK_TEMPRESP_STK_SIZE                 2048u
#define  APP_CFG_TASK_TEMPRESP_PRIO                      20u

/*静态全局变量*/                                                        
TX_THREAD   				AppTaskNetXProTCB;
static  uint64_t    AppTaskNetXProStk[APP_CFG_TASK_NETXPRO_STK_SIZE/8];

TX_THREAD   				AppTaskDefaultTCB;
static  uint64_t    AppTaskDefaultStk[APP_CFG_TASK_DEFAULT_STK_SIZE/8];
TX_THREAD   				AppTaskTempRespTCB;
static  uint64_t         AppTaskTempRespStk[APP_CFG_TASK_TEMPRESP_STK_SIZE/8];
TX_SEMAPHORE             TempRespSemaphore;

/*函数声明*/
static  void  		AppTaskNetXPro(ULONG thread_input);
static  void  		AppTaskDefault(ULONG thread_input);
static  void  		AppTaskTempResp(ULONG thread_input);

/*
*********************************************************************************************************
*	函 数 名: tx_application_define
*	功能说明: ThreadX专用的任务创建，通信组件创建函数
*	形    参: first_unused_memory  未使用的地址空间
*	返 回 值: 无
*********************************************************************************************************
*/
void  tx_application_define(void *first_unused_memory)
{
	/*如果实现任务CPU利用率统计的话，此函数仅用于实现启动任务，统计任务和空闲任务，其它任务在函数AppTaskCreate里面创建。*/
	/**************创建默认任务*********************/
		tx_thread_create(&AppTaskDefaultTCB,                /* 任务控制块地址 	*/   
											"App Default",                    /* 任务名 					*/
											AppTaskDefault,                   /* 启动任务函数地址 */
											0,                                /* 传递给任务的参数 */
											&AppTaskDefaultStk[0],            /* 堆栈基地址 			*/
											APP_CFG_TASK_DEFAULT_STK_SIZE,    /* 堆栈空间大小 		*/  
											APP_CFG_TASK_Default_PRIO,        /* 任务优先级				*/
											APP_CFG_TASK_Default_PRIO,        /* 任务抢占阀值 		*/
											TX_NO_TIME_SLICE,                 /* 不开启时间片 		*/
											TX_AUTO_START);                   /* 创建后立即启动 	*/  
	/**************创建NetX处理任务*********************/
    tx_thread_create(&AppTaskNetXProTCB,               	/* 任务控制块地址 	*/   
                      "App NETX Pro",                  	/* 任务名 					*/
                      AppTaskNetXPro,                 	/* 启动任务函数地址 */
                      0,                              	/* 传递给任务的参数 */
                      &AppTaskNetXProStk[0],          	/* 堆栈基地址 			*/
                      APP_CFG_TASK_NETXPRO_STK_SIZE,  	/* 堆栈空间大小 		*/  
                      APP_CFG_TASK_NetXPro_PRIO,      	/* 任务优先级				*/
                      APP_CFG_TASK_NetXPro_PRIO,      	/* 任务抢占阀值 		*/
                      TX_NO_TIME_SLICE,               	/* 不开启时间片 		*/
                      TX_AUTO_START);                	 	/* 创建后立即启动 	*/   
	/**************创建温度应答转发任务*********************/
	tx_semaphore_create(&TempRespSemaphore, "Temp Resp Sem", 0);
	tx_thread_create(&AppTaskTempRespTCB,             /* 任务控制块地址 	*/
					 "App Temp Resp",                  /* 任务名 				*/
					 AppTaskTempResp,                  /* 启动任务函数地址 */
					 0,                                /* 传递给任务的参数 */
					 &AppTaskTempRespStk[0],           /* 堆栈基地址 			*/
					 APP_CFG_TASK_TEMPRESP_STK_SIZE,   /* 堆栈空间大小 		*/
					 APP_CFG_TASK_TEMPRESP_PRIO,       /* 任务优先级			*/
					 APP_CFG_TASK_TEMPRESP_PRIO,       /* 任务抢占阀值 		*/
					 TX_NO_TIME_SLICE,                 /* 不开启时间片 		*/
					 TX_AUTO_START);                   /* 创建后立即启动 	*/
}
/*
*********************************************************************************************************
*	函 数 名: AppTaskDefault
*	功能说明: 
*	形    参: thread_input 是在创建该任务时传递的形参
*	返 回 值: 无
*	优 先 级: 
*********************************************************************************************************
*/
/* 降低系统软起动功耗 */
//static uint8_t 	flag=0;
static uint16_t CNT =0;
static void AppTaskDefault(ULONG thread_input)
{
    (void)thread_input;
    
	while(1)
	{
     if(CNT == 2)
		{
			CNT = 0;
			//tx_thread_sleep(2000);
			HAL_GPIO_WritePin(GPIOG, TRIG2_EN_Pin|TRIG3_EN_Pin|TRIG4_EN_Pin|TRIG5_EN_Pin		\
									|TRIG6_EN_Pin|TRIG7_EN_Pin|TRIG8_EN_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,TRIG1_EN_Pin,GPIO_PIN_RESET);	
		}
		HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_3);//绿色LED灯接口GPIOA.3
		tx_thread_sleep(500);
	}   
}
/*
*********************************************************************************************************
*	函 数 名: AppTaskNetXPro
*	功能说明: 消息处理，这里用作NetX网络任务处理
*	形    参: thread_input 是在创建该任务时传递的形参
*	返 回 值: 无
*	优 先 级: 上电是29，网线插入后提升至6
*********************************************************************************************************
*/

void NetXTest(void);
static void AppTaskNetXPro(ULONG thread_input)
{
    (void)thread_input;
    
	while(1)
	{
		NetXTest();
	}   
}
/*
*********************************************************************************************************
*	                                     NetX任务和通信组件
*********************************************************************************************************
*/
/* 上电先将其设置到低优先级，待网线插入后提升优先级到3 */
#define  APP_CFG_TASK_NETX_PRIO                           28u    
#define  APP_CFG_TASK_NETX_STK_SIZE                     4096u
static   uint64_t  AppTaskNetXStk[APP_CFG_TASK_NETX_STK_SIZE/8];

/* 提升NetX处理任务优先级到3 */
#define  APP_CFG_TASK_NETX_PRIO1                           3u 
/* 提升NetX应用任务优先级到6 */
#define  APP_CFG_TASK_NetXPro_PRIO1                        6u

NX_TCP_SOCKET TCPSocket;																	//实际未使用
NX_UDP_SOCKET UDPSocket;																	//UDP协议进行通信
TX_SEMAPHORE  Semaphore;

/*定时器相关定义*/
#define Count_TIMx                  TIM4									//从定时器用于计数
#define Light_TIMx                  TIM1									//主定时器用于发送脉冲
#define Light_TIM_Channe3           LL_TIM_CHANNEL_CH3		//定时器发送脉冲通道
#define Light_PulseWidth            10										

/*
*********************************************************************************************************
*	                                    NetX任务和通信组件
*********************************************************************************************************
*/
UCHAR data_buffer[4096]			;	//数据接收buffer
NX_PACKET_POOL    		pool_0	;
NX_IP             		ip_0	;  

/* 数据包内存池 */
#define PACKET_SIZE          1536
#define NX_PACKET_POOL_SIZE  ((PACKET_SIZE + sizeof(NX_PACKET)) * 20)
	
ULONG  packet_pool_area[NX_PACKET_POOL_SIZE/4 + 4];

/* ARP缓存 */
ULONG    arp_space_area[512 / sizeof(ULONG)];                                                      
ULONG    error_counter;

#define PRINT_DATA(addr, port, data)        do {                                              \
                                                  printf("[%lu.%lu.%lu.%lu:%u] -> '%s' \r\n", \
                                                  (addr >> 24) & 0xff,                        \
                                                  (addr >> 16) & 0xff,                        \
                                                  (addr >> 8) & 0xff,                         \
                                                  (addr & 0xff), port, data);                 \
                                               } while(0)
/*****************下面调用*****************/
extern TX_THREAD   	AppTaskNetXProTCB ;
extern TX_THREAD   	*netx_thread_ptr  ;                                               
extern VOID  		nx_driver_stm32f4xx(NX_IP_DRIVER *driver_req_ptr);
   
											   
#define UART_Handle   huart1                    /* 调用串口 */
#define UART_TxBuffSzie         256          	/* 发送缓冲区大小 */
uint8_t UART_TxBuff[UART_TxBuffSzie];        	/* 发送缓冲区 */
#define UART_RxBuffSize          10                          /* 每路UART接收缓冲区大小(固定应答10字节) */
static uint8_t UART1_RxBuff[UART_RxBuffSize];
static uint8_t UART2_RxBuff[UART_RxBuffSize];
static uint8_t UART3_RxBuff[UART_RxBuffSize];
static uint8_t UART4_RxBuff[UART_RxBuffSize];
static uint8_t UART5_RxBuff[UART_RxBuffSize];
static uint8_t UART6_RxBuff[UART_RxBuffSize];
static uint8_t UART7_RxBuff[UART_RxBuffSize];
static uint8_t UART8_RxBuff[UART_RxBuffSize];
static uint8_t TempRespFrame[10];                            /* 温度应答帧 */
volatile uint8_t  TempQueryPending = 0;                      /* 温度查询进行中标志 */
volatile uint16_t TempQueryID = 0;                           /* 当前查询的光源ID */

Frame_Param_TypeDef 		Frame_Param_Struct;
Memory_Param_TypeDef 		Memory_Param_Struct;
Bootloader_Address_Typedef 	Bootloader_Address_Struct;											   

/**********************************************函数声明*************************************/											   
static void Frame_Data_Handler(void);
static void Frame_UDP_Data_Send(uint16_t Cmd ,uint8_t *pData,uint16_t Size);
static void Frame_UDP_Data_Send_Raw(uint8_t *pData,uint16_t Size);
static void System_Frequency_Start_Handler(uint8_t Camera_Frequency, uint16_t LightSource_Frequency, uint16_t LightSource_Counter,uint16_t Camera_PulseWidth, uint16_t Camera_SyncDelay); 
static void System_Frequency_Stop_Handler(void);											   
/*********************************************************************************************************
*	函 数 名: NetXTest
*	功能说明: NetXDUO应用
*	形    参: 无
*	返 回 值: 无
**********************************************************************************************************/    
void NetXTest(void)
{
	UINT 					status;								/* 状态 	*/
	UINT 					ret;
	UINT 					old_priority;						/* 旧优先级 */
	NX_PACKET 				*RecPacket;							/* 接收的包 */
	ULONG 					bytes_read;							/* 读字节 	*/
	UINT 					source_port;						/* 源端口 	*/
	ULONG 					source_ip_address;					/* 源IP 	*/
	ULONG 					Local_IP_Address;					/* 本地IP 	*/
	UINT  					Local_Port;                   		/* 本地端口 */
	Memory_Param_TypeDef 	Memory_Buff_Struct;
	
	/* 读取APP参数 Bootloader_Address_Struct -- Bootloader_Address_Base 第四扇区 0x08010000 - 0x0801FFFF 64 KB字节*/
	STM32_Flash_Read(Bootloader_Address_Base,(uint8_t *)&Bootloader_Address_Struct,sizeof(Bootloader_Address_Struct));	//查找扇区4的地址为08010000

	if(Bootloader_Address_Struct.APP_Backup_Flag == 0xFFFFFFFF)//判断扇区4的APP备份区标志位是否为0xFFFFFFFF
	{
		Bootloader_Address_Struct.APP_Backup_Flag = 0xFFFFFFFF;
		STM32_Flash_EraseWrite(Bootloader_Address_Base,(uint8_t *)&Bootloader_Address_Struct,sizeof(Bootloader_Address_Struct));
	}
	
	/* 读取IP参数 */
	STM32_Flash_Read(ADDR_FLASH_SECTOR_3, &Memory_Buff_Struct.Local_Flag, sizeof(Memory_Buff_Struct));

	if(Memory_Buff_Struct.Local_Flag == 0x55) //判断本地标志位是否为0x55
	{
		Local_IP_Address = Memory_Buff_Struct.Local_IP;		//本地IP
		Local_Port = Memory_Buff_Struct.Local_Port;			//本地端口
	}
	else
	{	
		/* 否则进行IP地址设置为192.168.0.1 */
		Local_IP_Address = IP_ADDRESS(LOCAL_IP_ADDR0, LOCAL_IP_ADDR1,LOCAL_IP_ADDR2, LOCAL_IP_ADDR3); 
		Local_Port 		 = LOCAL_PORT;	//端口号为8888																																		//设置端口号为8888
	}

    /* 初始化NetX */
    nx_system_initialize();

    /* 创建内存池 */  //创建成功后 status = 0
    status =  nx_packet_pool_create(&pool_0,                                       /* 内存池控制块 */
                                     "NetX Main Packet Pool",                      /* 内存池名 */
                                     1536,                                         /* 内存池每个数据包大小，单位字节
                                                                                      此值必须至少为 40 个字节，并且还必须可以被 4 整除 */
									 (ULONG*)(((int)packet_pool_area + 15) & ~15) ,/* 内存池地址，此地址必须ULONG对齐 */
                                     NX_PACKET_POOL_SIZE);                         /* 内存池大小 */                  
          
    /* 检测创建是否失败 */
    if (status) error_counter++;

    /* 例化IP */
    status = nx_ip_create(&ip_0,                                                     /* IP实例控制块 */                 
                            "NetX IP Instance 0",                                    /* IP实例名 */     
                            Local_IP_Address,                                        /* IP地址 */
                            0xFFFFFF00UL,                                            /* 子网掩码 */
                            &pool_0,                                                 /* 内存池 */
							nx_driver_stm32f4xx,                                                   /* 网卡驱动 */
                            (UCHAR*)AppTaskNetXStk,                                  /* IP任务栈地址 */
                            sizeof(AppTaskNetXStk),                                  /* IP任务栈大小，单位字节 */
                            APP_CFG_TASK_NETX_PRIO);                                 /* IP任务优先级 */
                            
            
    /* 检测创建是否失败 */
    if (status) error_counter++;

    /* 使能ARP，并提供ARP缓存 */
    status =  nx_arp_enable(&ip_0,       				/* IP实例控制块 */
							(void *)arp_space_area,    	/* ARP缓存地址 */
							sizeof(arp_space_area));   	/* 每个 ARP 条目均为 52 个字节，因此，ARP 条目总数是52字节整数倍 */

    /* 使能fragment */    
    status = nx_ip_fragment_enable(&ip_0);

    /* 检测使能成功 */
    if (status) error_counter++;

    /* 使能TCP */
    status =  nx_tcp_enable(&ip_0);

    /* 检测使能成功 */
    if (status) error_counter++;

    /* 使能UDP  */
    status =  nx_udp_enable(&ip_0);

    /* 检测使能成功 */
    if (status) error_counter++;

    /* 使能ICMP */
    status =  nx_icmp_enable(&ip_0);

    /* 检测使能成功 */
    if (status) error_counter++;   
    
    /* NETX初始化完毕后，重新设置优先级 */
    tx_thread_priority_change(netx_thread_ptr, APP_CFG_TASK_NETX_PRIO1, &old_priority);
    tx_thread_priority_change(&AppTaskNetXProTCB, APP_CFG_TASK_NetXPro_PRIO1, &old_priority);
    
    ////////////////////////////////////////////////////////////////////////////////////////////////						   
    /* 创建UDP socket */
    ret = nx_udp_socket_create(&ip_0,                 /* IP实例控制块 */    
                                &UDPSocket,           /* UDP控制块 */ 
                                "UDP Server Socket",  /* UDP名 */ 
                                NX_IP_NORMAL,         /* IP服务类型 */ 
                                NX_FRAGMENT_OKAY,     /* 使能IP分段 */ 
                                NX_IP_TIME_TO_LIVE,   /* 指定一个 8 位的值，用于定义此数据包在被丢弃之前可通过的路由器数目 */  
                                512);                 /* 支持的报文数 */

    if (ret != NX_SUCCESS)
    {
		ERROR_HANDLER();		
    }
    
    /* UDP Socket绑定端口 */
    ret = nx_udp_socket_bind(&UDPSocket, Local_Port, TX_WAIT_FOREVER);

    if (ret != NX_SUCCESS)
    {
        Error_Handler(__FILE__, __LINE__);   
    }
	/* UDP Socket套接口为广播模式 */
//	nx_udp_socket_set_broadcast(&UDPSocket, NX_TRUE);
    
	while(1)
	{
        /* 接收数据 */
        ret = nx_udp_socket_receive(&UDPSocket, &RecPacket, TX_WAIT_FOREVER);

        if (ret == NX_SUCCESS)
        {
            /* 将UDP数据包中的数据复制到缓冲data_buffer */
            nx_packet_data_extract_offset(RecPacket,            /* 数据包 					*/
                                          0,                    /* 数据包地址偏移 	*/
                                          data_buffer,          /* 目标缓冲 				*/
                                          sizeof(data_buffer),  /* 目标缓冲大小 		*/
                                          &bytes_read);         /* 数据复制的字节数 */
            /* 获取远程端口和IP  */
            nx_udp_source_extract(RecPacket, &source_ip_address, &source_port);
            /* 释放数据包 */
            nx_packet_release(RecPacket);
										  
			uint8_t sum = 0;
			/********************协议要求IP地址：192.168.0.1 端口号：8888*********************************/
			if((source_ip_address == IP_ADDRESS(SOURCE_IP_ADDR0, SOURCE_IP_ADDR1,SOURCE_IP_ADDR2, SOURCE_IP_ADDR3))&& (source_port == SOURCE_PORT))//判断IP和端口
			{
				/*数据总长度：帧头2个字节+长度2个字节+(命令码2+数据段(第2位 | 第3位<<8))+校验和1个字节 刚好是下面数组＋5个字节*/
				Frame_Param_Struct.FrameSize = (data_buffer[Frame_SizeMark] | ((data_buffer[Frame_SizeMark+1] << 8) & 0xFF00)) + 5;
					
				if( bytes_read >= Frame_Param_Struct.FrameSize) 
				{
					/*********帧头： "L","U" 4C 55**********/
					if( Frame_Start == (data_buffer[Frame_StartMark+1] | ((data_buffer[Frame_StartMark] << 8) & 0xFF00)) )  /* check frame start */
					{	
						/**************************操作命令+数据段求和************************************/
						for(uint16_t i = 0 ; i < Frame_Param_Struct.FrameSize - 5 ; i++)
						{
							sum += data_buffer[Frame_CmdMark + i];
						}
						/********************************检验和*******************************************/
						if(sum == data_buffer[Frame_Param_Struct.FrameSize - 1])
						{
							/*内存复制函数，将data_buffer指向的地址为起点，连续FrameSize的字节数据，复制到Frame_Param_Struct的内存*/
							memcpy(&Frame_Param_Struct,data_buffer,Frame_Param_Struct.FrameSize); 
							/*协议匹配*/
							Frame_Data_Handler();
						}								
					}
				}								
			}																										
        }
	}  	
}

/* 8路RS-485发送使能引脚 */
void Frame_UART_RX_TX(void)
{
	RS485_1_Mode_Tx();	//串口1发送模式
	RS485_2_Mode_Tx();	//串口2发送模式
	RS485_3_Mode_Tx();	//串口3发送模式
	RS485_4_Mode_Tx();	//串口4发送模式
	RS485_5_Mode_Tx();	//串口5发送模式
	RS485_6_Mode_Tx();	//串口6发送模式
	RS485_7_Mode_Tx();	//串口7发送模式
	RS485_8_Mode_Tx();	//串口8发送模式
}
/* 8路RS-485接收使能引脚，降低功耗 */
void Frame_UART_RX_TX_LOW(void)
{
	RS485_1_Mode_Rx();	//串口1接收模式
	RS485_2_Mode_Rx();  //串口2接收模式
	RS485_3_Mode_Rx();  //串口3接收模式
	RS485_4_Mode_Rx();  //串口4接收模式
	RS485_5_Mode_Rx();  //串口5接收模式
	RS485_6_Mode_Rx();  //串口6接收模式
	RS485_7_Mode_Rx();  //串口7接收模式
	RS485_8_Mode_Rx();  //串口8接收模式
}

/* 8路UART接收初始化：清空并挂起每路固定长度接收 */
void Frame_UART_Start_Receive(void)
{
	Frame_UART_RX_TX_LOW();

	HAL_UART_AbortReceive(&huart1);
	HAL_UART_AbortReceive(&huart2);
	HAL_UART_AbortReceive(&huart3);
	HAL_UART_AbortReceive(&huart4);
	HAL_UART_AbortReceive(&huart5);
	HAL_UART_AbortReceive(&huart6);
	HAL_UART_AbortReceive(&huart7);
	HAL_UART_AbortReceive(&huart8);

	HAL_UART_Receive_IT(&huart1, UART1_RxBuff, UART_RxBuffSize);
	HAL_UART_Receive_IT(&huart2, UART2_RxBuff, UART_RxBuffSize);
	HAL_UART_Receive_IT(&huart3, UART3_RxBuff, UART_RxBuffSize);
	HAL_UART_Receive_IT(&huart4, UART4_RxBuff, UART_RxBuffSize);
	HAL_UART_Receive_IT(&huart5, UART5_RxBuff, UART_RxBuffSize);
	HAL_UART_Receive_IT(&huart6, UART6_RxBuff, UART_RxBuffSize);
	HAL_UART_Receive_IT(&huart7, UART7_RxBuff, UART_RxBuffSize);
	HAL_UART_Receive_IT(&huart8, UART8_RxBuff, UART_RxBuffSize);
}

/* 串口1-8的DMA数据发送函数 */
static void Frame_UART_Data_Send(UART_HandleTypeDef *huart,uint8_t *pData,uint16_t Len)
{
	HAL_UART_Transmit_DMA(huart,pData,Len);
}
///*	将8个串口发送封装成1个*/
//static void Frame_UART_Data_Send1(uint8_t *Data,uint8_t Len)
//{
//	uint8_t *xData;
//	uint8_t sum = 0;
//	xData = UART_TxBuff;
//	
//	for( uint8_t i = 0 ; i < Len-1 ; i++ )
//	{
//		sum += *xData++ = Data[i];
//	}
//	Frame_UART_RX_TX();
//	HAL_UART_Transmit_DMA(&huart1,UART_TxBuff,xData - UART_TxBuff);
//	HAL_UART_Transmit_DMA(&huart2,UART_TxBuff,xData - UART_TxBuff);
//	HAL_UART_Transmit_DMA(&huart3,UART_TxBuff,xData - UART_TxBuff);
//	HAL_UART_Transmit_DMA(&huart4,UART_TxBuff,xData - UART_TxBuff);
//	HAL_UART_Transmit_DMA(&huart5,UART_TxBuff,xData - UART_TxBuff);
//	HAL_UART_Transmit_DMA(&huart6,UART_TxBuff,xData - UART_TxBuff);
//	HAL_UART_Transmit_DMA(&huart7,UART_TxBuff,xData - UART_TxBuff);
//	HAL_UART_Transmit_DMA(&huart8,UART_TxBuff,xData - UART_TxBuff);
//}
/* UART发送完成回调：等待TC后把对应RS485切回接收，保证最后一字节发完 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	uint32_t Timeout = 10000;

	while((!__HAL_UART_GET_FLAG(huart, UART_FLAG_TC)) && (Timeout--))
	{
	}

	if(huart == &huart1)
	{
		RS485_1_Mode_Rx();
	}
	else if(huart == &huart2)
	{
		RS485_2_Mode_Rx();
	}
	else if(huart == &huart3)
	{
		RS485_3_Mode_Rx();
	}
	else if(huart == &huart4)
	{
		RS485_4_Mode_Rx();
	}
	else if(huart == &huart5)
	{
		RS485_5_Mode_Rx();
	}
	else if(huart == &huart6)
	{
		RS485_6_Mode_Rx();
	}
	else if(huart == &huart7)
	{
		RS485_7_Mode_Rx();
	}
	else if(huart == &huart8)
	{
		RS485_8_Mode_Rx();
	}
}

/* UART接收完成回调：校验温度应答帧并通知转发线程 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	uint8_t *pRx = NULL;

	if(huart == &huart1)      pRx = UART1_RxBuff;
	else if(huart == &huart2) pRx = UART2_RxBuff;
	else if(huart == &huart3) pRx = UART3_RxBuff;
	else if(huart == &huart4) pRx = UART4_RxBuff;
	else if(huart == &huart5) pRx = UART5_RxBuff;
	else if(huart == &huart6) pRx = UART6_RxBuff;
	else if(huart == &huart7) pRx = UART7_RxBuff;
	else if(huart == &huart8) pRx = UART8_RxBuff;

	if(pRx != NULL)
	{
		if(TempQueryPending != 0)
		{
			uint8_t sum = 0;
			uint16_t rx_id;
			uint8_t i;

			for(i = 4 ; i < 9 ; i++)
			{
				sum += pRx[i];
			}
			rx_id = (uint16_t)((pRx[7] << 8) | pRx[6]);

			if((pRx[0] == 0x4C) && (pRx[1] == 0x55) &&
			   (pRx[2] == 0x05) && (pRx[3] == 0x00) &&
			   (pRx[4] == 0x03) && (pRx[5] == 0x55) &&
			   (pRx[9] == sum) &&
			   (rx_id == TempQueryID))
			{
				for(i = 0 ; i < 10 ; i++)
				{
					TempRespFrame[i] = pRx[i];
				}
				tx_semaphore_put(&TempRespSemaphore);
			}
		}

		/* 重新挂起本路接收 */
		HAL_UART_Receive_IT(huart, pRx, UART_RxBuffSize);
	}
}

/* UART错误回调：清错误并重新挂起本路接收 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	uint8_t *pRx = NULL;

	if(huart == &huart1)      pRx = UART1_RxBuff;
	else if(huart == &huart2) pRx = UART2_RxBuff;
	else if(huart == &huart3) pRx = UART3_RxBuff;
	else if(huart == &huart4) pRx = UART4_RxBuff;
	else if(huart == &huart5) pRx = UART5_RxBuff;
	else if(huart == &huart6) pRx = UART6_RxBuff;
	else if(huart == &huart7) pRx = UART7_RxBuff;
	else if(huart == &huart8) pRx = UART8_RxBuff;

	if(pRx != NULL)
	{
		HAL_UART_AbortReceive(huart);
		HAL_UART_Receive_IT(huart, pRx, UART_RxBuffSize);
	}
}

/*接收帧buffer*/
uint8_t  Frame_DataBuff_Tx[512];
uint32_t count = 0;

static void Frame_Data_Handler(void)
{
	UINT ret;
	uint8_t err 		 =  0 ;
	uint8_t sendbuff[64] = {0};
	uint8_t sendsize 	 =  0 ;
	uint8_t *pdata;
	pdata = sendbuff;
	Memory_Param_TypeDef Memory_Buff_Struct;
	
	switch(Frame_Param_Struct.CMD_Union.buff)
	{
		case Frame_CFG_CMD_Controller_IP :	/*控制器IP配置*/
		{
			/*先将标志位、IP、端口号等信息写入*/
			Memory_Param_Struct.Local_Flag = 0x55;
			Memory_Param_Struct.Local_IP   = IP_ADDRESS(Frame_Param_Struct.Data[0],Frame_Param_Struct.Data[1],Frame_Param_Struct.Data[2],Frame_Param_Struct.Data[3]);
			Memory_Param_Struct.Local_Port = Frame_Param_Struct.Data[4] | ((Frame_Param_Struct.Data[5] << 8) & 0xFF00);
			/********写入10ms后，进行读出,写入到块3的flash中**********/
			STM32_Flash_EraseWrite(ADDR_FLASH_SECTOR_3, &Memory_Param_Struct.Local_Flag, sizeof(Memory_Param_Struct));
			tx_thread_sleep(10);
			STM32_Flash_Read(ADDR_FLASH_SECTOR_3, &Memory_Buff_Struct.Local_Flag, sizeof(Memory_Buff_Struct))	  ;
			
			/*比较写入数据与实际读出数据是否一致，小于返回<0、等于返回=0、大于返回>0*/
			if(memcmp((uint8_t*)&Memory_Param_Struct,(uint8_t*)&Memory_Buff_Struct,strlen((char *)&Memory_Param_Struct)))
			{
				/*数据存储失败,则会UDP给上位机进行反馈*/
				*pdata++ = 0x01;
				sendsize = pdata - sendbuff;
				Frame_UDP_Data_Send(Frame_CFG_CMD_Controller_IP,sendbuff,sendsize);	
			}
			else 
			{
				Frame_Param_Struct.Local_IP   = Memory_Param_Struct.Local_IP   ;
				Frame_Param_Struct.Local_Port = Memory_Param_Struct.Local_Port ;
				/*ret为状态判断*/				
				ret = nx_ip_address_set(&ip_0,Frame_Param_Struct.Local_IP ,0xFFFFFF00UL);
				if(ret != NX_SUCCESS)
				{
					err++;
				}
				/* UDP Socket取消绑定端口 */
				ret = nx_udp_socket_unbind(&UDPSocket);
				if (ret != NX_SUCCESS)
				{
					err++;
				}
				/* UDP Socket绑定端口 */
				ret = nx_udp_socket_bind(&UDPSocket, Frame_Param_Struct.Local_Port, TX_WAIT_FOREVER);
				if (ret != NX_SUCCESS)
				{
					err++;
				}
				if(err != 0)
				{
					Frame_Param_Struct.Local_CFG_Status = 0;//配置失败
				}
				else
				{
					Frame_Param_Struct.Local_CFG_Status = 1;//配置成功
				}
				/* 配置成功后通过UDP返回给上位机 */
				*pdata++ = Frame_Param_Struct.Local_CFG_Status;
				sendsize = pdata - sendbuff;
				Frame_UDP_Data_Send(Frame_CFG_CMD_Controller_IP,sendbuff,sendsize); // 4C 55 03 00 03 AA 01 XX
			}
		}
		break;
		
		case  Frame_CFG_CMD_LightSource_ID :		/*光源ID配置*/
		{
			Frame_UART_RX_TX();
			
			Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
			Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
			Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
			Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
			Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
			Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
			Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
			Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */
			
			_tx_thread_sleep(50);
			Frame_UART_RX_TX_LOW();
		}
		break;
		
		case  Frame_CFG_CMD_NixieTube_Display : 	/*数码管显示配置*/
		{
			Frame_UART_RX_TX();
			
			Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
			Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
			Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
			Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
			Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
			Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
			Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
			Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */
			
			_tx_thread_sleep(50);
			Frame_UART_RX_TX_LOW();
		}
		break;
		
		case  Frame_CFG_CMD_System_Mode :			/*系统工作模式配置*/
		{
			Frame_Param_Struct.System_Mode = Frame_Param_Struct.Data[0]; 

			Frame_UART_RX_TX();
			
			Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
			Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
			Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
			Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
			Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
			Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
			Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
			Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */
			
			_tx_thread_sleep(50);
			Frame_UART_RX_TX_LOW();
		}
		break;
		
		case  Frame_CFG_CMD_System_Frequency : 		/*系统工作频率配置*/
		{
			Frame_UART_RX_TX();
			
			Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
			Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
			Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
			Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
			Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
			Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
			Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
			Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */
			
			/*工作频率配置，参数修改*/ 		//光源触发频率、光源触发次数、相机类型、相机分频参数、相机触发脉宽、相机同步延迟
			Frame_Param_Struct.LightSource_Frequency 	=	(Frame_Param_Struct.Data[0] | ((Frame_Param_Struct.Data[1]<<8)));
			Frame_Param_Struct.LightSource_Counter		=	(Frame_Param_Struct.Data[2] | ((Frame_Param_Struct.Data[3]<<8)));
			Frame_Param_Struct.Camera_Type  			=	(Frame_Param_Struct.Data[4])						            ;
			Frame_Param_Struct.Camera_Frequency  		=	(Frame_Param_Struct.Data[5])						            ;
			Frame_Param_Struct.Camera_PulseWidth		=	(Frame_Param_Struct.Data[6] | ((Frame_Param_Struct.Data[7]<<8)));
			Frame_Param_Struct.Camera_SyncDelay			=	(Frame_Param_Struct.Data[8] | ((Frame_Param_Struct.Data[9]<<8)));
			
			_tx_thread_sleep(50);
			Frame_UART_RX_TX_LOW();
		}
		break;
		
		case  Frame_CFG_CMD_LightSource_Brightness :/*材质解算模式下光源参数配置*/ 
		{
			uint16_t Brightness_Packet_Number  		= 0;//数据包序号
			uint8_t  Brightness_Subcontract_number	= 0;//分包号
			uint8_t  Brightness_Config_status  		= 0;//配置状态
			
			Frame_UART_RX_TX();
			
			Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
			Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
			Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
			Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
			Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
			Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
			Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
			Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */
			
			Brightness_Packet_Number 		= (Frame_Param_Struct.Data[0] | ((Frame_Param_Struct.Data[1]<<8)));
			Brightness_Subcontract_number	= (Frame_Param_Struct.Data[2]);
			Brightness_Config_status 		=  0x01;
			
			*pdata++ = Brightness_Packet_Number		 & 0xFF ;
			*pdata++ = (Brightness_Packet_Number>>8) & 0xFF	;
			*pdata++ = Brightness_Subcontract_number		;
			*pdata++ = Brightness_Config_status				;
			sendsize = pdata - sendbuff						;		
			Frame_UDP_Data_Send(Frame_CFG_CMD_LightSource_Brightness,sendbuff,sendsize);
			
			_tx_thread_sleep(50);
			Frame_UART_RX_TX_LOW();
		}
		break;
		
		case  Frame_CFG_CMD_DebugMode : 			/*装调模式下光源参数配置*/ //直接下发即可
		{
			//闪烁次数
			Frame_UART_RX_TX();
			
			Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
			Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
			Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
			Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
			Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
			Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
			Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
			Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */
			
			_tx_thread_sleep(50);
			Frame_UART_RX_TX_LOW();
		}
		break;

		case  Frame_CFG_CMD_OLATMode : 				/*OLAT模式下光源参数配置*/
		{
			uint16_t OLAT_Packet_Number  		= 0;//OLAT模式数据包序号
			uint8_t  OLAT_Subcontract_number	= 0;//OLAT模式分包号
			uint8_t  OLAT_Config_status  		= 0;//OLAT模式配置状态
			
			Frame_UART_RX_TX();
			
			Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
			Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
			Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
			Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
			Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
			Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
			Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
			Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */
			
			OLAT_Packet_Number 		= (Frame_Param_Struct.Data[0] | ((Frame_Param_Struct.Data[1]<<8)));
			OLAT_Subcontract_number	= (Frame_Param_Struct.Data[2]);
			OLAT_Config_status 		=  0x01;
		
			*pdata++ =  OLAT_Packet_Number	   & 0xFF   ;
			*pdata++ = (OLAT_Packet_Number>>8) & 0xFF	;
			*pdata++ =  OLAT_Subcontract_number			;
			*pdata++ =  OLAT_Config_status				;
			sendsize =  pdata - sendbuff				;
			Frame_UDP_Data_Send(Frame_CFG_CMD_OLATMode,sendbuff,sendsize);
			
			_tx_thread_sleep(50);
			Frame_UART_RX_TX_LOW();
		}
		break;
		
		case  Frame_CFG_CMD_Heavy_lighting_mode : 	/*重光照模式下光源参数配置*/
		{
			uint16_t HLM_Packet_Number  		= 0;//重光照模式数据包序号
			uint8_t  HLM_Subcontract_number		= 0;//重光照模式分包号
			uint8_t  HLM_Config_status  		= 0;//重光照模式配置状态
			
			Frame_UART_RX_TX();
			
			Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
			Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
			Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
			Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
			Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
			Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
			Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
			Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */
			
			
			HLM_Packet_Number 		= (Frame_Param_Struct.Data[0] | ((Frame_Param_Struct.Data[1]<<8)));
			HLM_Subcontract_number	= (Frame_Param_Struct.Data[2]);
			HLM_Config_status 		=  0x01;
			
			*pdata++ =  HLM_Packet_Number	   & 0xFF   ;
			*pdata++ = (HLM_Packet_Number>>8)  & 0xFF	;
			*pdata++ =  HLM_Subcontract_number			;
			*pdata++ =  HLM_Config_status				;
			sendsize =  pdata - sendbuff				;
			Frame_UDP_Data_Send(Frame_CFG_CMD_Heavy_lighting_mode,sendbuff,sendsize);
			
			_tx_thread_sleep(50);
			Frame_UART_RX_TX_LOW();
		}
		break;
		
			case  Frame_CFG_CMD_Data_Set_Over :			/*数据配置完成*/
		{	 

			Frame_UART_RX_TX();
			
			Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
			Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
			Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
			Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
			Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
			Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
			Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
			Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */
			
			_tx_thread_sleep(50);
			Frame_UART_RX_TX_LOW();
		}
		break;
			
			case  Frame_CFG_CMD_LoopData_Set :			/*循环数据量配置*/
		{	 

			Frame_UART_RX_TX();
			
			Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
			Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
			Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
			Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
			Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
			Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
			Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
			Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */
			
			_tx_thread_sleep(50);
			Frame_UART_RX_TX_LOW();
		}
		break;
		
		case  Frame_CFG_CMD_Heavy_Test : 		  /*重光照测试模式*/
		{
			Frame_UART_RX_TX();
			
			Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
			Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
			Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
			Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
			Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
			Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
			Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
			Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */
			
			_tx_thread_sleep(50);
			Frame_UART_RX_TX_LOW();
		}
		break;
		
		case  Frame_CFG_CMD_Soft_Start : 		  /*软启动*/
		{
			CNT = 0;
			LL_TIM_ClearFlag_UPDATE(Count_TIMx);
			LL_TIM_DisableCounter(Count_TIMx);
			LL_TIM_SetCounter(Count_TIMx,0);
			TIM_CCxChannelCmd(TIM1,TIM_CHANNEL_3,TIM_CCx_DISABLE);
			LL_TIM_SetCounter(Light_TIMx,0);
			
			Frame_UART_RX_TX();
			
			Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
			Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
			Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
			Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
			Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
			Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
			Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
			Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */
			
		
			HAL_GPIO_WritePin(GPIOG, TRIG2_EN_Pin|TRIG3_EN_Pin|TRIG4_EN_Pin|TRIG5_EN_Pin		\
									|TRIG6_EN_Pin|TRIG7_EN_Pin|TRIG8_EN_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD,TRIG1_EN_Pin,GPIO_PIN_SET);
			
			_tx_thread_sleep(100);	//延时200ms后进行软启动	

			System_Frequency_Start_Handler( Frame_Param_Struct.Camera_Frequency   , Frame_Param_Struct.LightSource_Frequency, \
										    Frame_Param_Struct.LightSource_Counter, Frame_Param_Struct.Camera_PulseWidth    , \
											Frame_Param_Struct.Camera_SyncDelay  );	//系统启动
			
			Frame_UART_RX_TX_LOW();

		}
		break;
		
		case  Frame_CFG_CMD_Stop :					/*停止*/
		{

			
			System_Frequency_Stop_Handler();
			
			Frame_UART_RX_TX();
			Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
			Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
			Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
			Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
			Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
			Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
			Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
			Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */

			
			HAL_GPIO_WritePin(GPIOG, TRIG2_EN_Pin|TRIG3_EN_Pin|TRIG4_EN_Pin|TRIG5_EN_Pin		\
									|TRIG6_EN_Pin|TRIG7_EN_Pin|TRIG8_EN_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD,TRIG1_EN_Pin,GPIO_PIN_RESET);

			
			_tx_thread_sleep(100);
			Frame_UART_RX_TX_LOW();
			
		}
		break;
		
		case  Frame_CFG_CMD_FirmwareUpgrade_Prepare :	/* 固件升级准备 */ 
		{
			Frame_Param_Struct.Upgrade_Device  = Frame_Param_Struct.Data[0];	/* 升级设备 */
			Frame_Param_Struct.Upgrade_Size    = Frame_Param_Struct.Data[1];	/* 更新数据包大小 */
			
			if(Frame_Param_Struct.Upgrade_Device == 0x01)
			{
				/*控制器更新*/
				if( Bootloader_Address_Struct.APP_Run_Flag ==  APP_Backup1)
				{
					STM32_Flash_Erase(GetSector(APP_Backup2_ADDR), APP_Sectors_NUM); // Erase app running area
				}
				else
				{
					STM32_Flash_Erase(GetSector(APP_Backup1_ADDR), APP_Sectors_NUM); // Erase app running area
				}
			}
			else
			{
				/*光源更新*/
				Frame_UART_RX_TX();
				Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
				Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
				Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
				Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
				Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
				Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
				Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
				Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */
				
				_tx_thread_sleep(100);
				Frame_UART_RX_TX_LOW();
			}
		}
		break;
		
		case  Frame_CFG_CMD_FirmwareUpgrade_Data     :	 /* 固件升级数据配置 */ 
		{			
			Frame_Param_Struct.Upgrade_Device 	= Frame_Param_Struct.Data[0];	/* 升级设备   */
			Frame_Param_Struct.Upgrade_Sequence = Frame_Param_Struct.Data[1];	/* 数据包序号 */
			Frame_Param_Struct.Upgrade_status 	= 0x01;

			if(Frame_Param_Struct.Upgrade_Device == 0x01)
			{
				//控制器更新
				if( Bootloader_Address_Struct.APP_Run_Flag ==  APP_Backup1)
				{
					STM32_Flash_Write(APP_Backup2_ADDR + Frame_Param_Struct.Upgrade_Sequence * 500 ,&Frame_Param_Struct.Data[2],500);
					Frame_Param_Struct.Upgrade_status = !memcmp(&Frame_Param_Struct.Data[2],(uint8_t*)(APP_Backup2_ADDR + Frame_Param_Struct.Upgrade_Sequence * 500),500);
				}
				else
				{
					STM32_Flash_Write(APP_Backup1_ADDR + Frame_Param_Struct.Upgrade_Sequence * 500 ,&Frame_Param_Struct.Data[2],500);
					Frame_Param_Struct.Upgrade_status = !memcmp(&Frame_Param_Struct.Data[2],(uint8_t*)(APP_Backup1_ADDR + Frame_Param_Struct.Upgrade_Sequence * 500),500);
				}
				*pdata++ = Frame_Param_Struct.Upgrade_Device;
				*pdata++ = Frame_Param_Struct.Upgrade_Sequence;
				*pdata++ = Frame_Param_Struct.Upgrade_status;
				sendsize = pdata - sendbuff;
				Frame_UDP_Data_Send(Frame_CFG_CMD_FirmwareUpgrade_Data,sendbuff,sendsize);				
			}
			else
			{
				/*光源更新*/
				Frame_UART_RX_TX();
				Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
				Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
				Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
				Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
				Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
				Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
				Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
				Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */
				
				uint8_t cnt = 0;
				while((!Frame_Param_Struct.Upgrade_status) && (cnt < 10))
				{
					tx_thread_sleep(1);
					cnt++;
				}
				*pdata++ = Frame_Param_Struct.Upgrade_Device;
				*pdata++ = Frame_Param_Struct.Upgrade_Sequence;
				*pdata++ = Frame_Param_Struct.Upgrade_status;
				sendsize = pdata - sendbuff;
				Frame_UDP_Data_Send(Frame_CFG_CMD_FirmwareUpgrade_Data,sendbuff,sendsize);
				
				_tx_thread_sleep(100);
				Frame_UART_RX_TX_LOW();
			}
		}
		break;
		
		case  Frame_CFG_CMD_FirmwareUpgrade_Start : 	/* 开始固件升级 */ 
		{
			uint8_t Upgrade_Start_status = 0x01;	/* 升级状态 */
			
			Frame_Param_Struct.Upgrade_Device = Frame_Param_Struct.Data[0];	/* 升级设备 */
			
			if(Frame_Param_Struct.Upgrade_Device == 0x01)
			{
				//控制器更新
				*pdata++ = Frame_Param_Struct.Upgrade_Device;
				*pdata++ = Upgrade_Start_status;
				sendsize = pdata - sendbuff;
				Frame_UDP_Data_Send(Frame_CFG_CMD_FirmwareUpgrade_Start,sendbuff,sendsize);
				
				if( Bootloader_Address_Struct.APP_Run_Flag ==  APP_Backup1)
				{
					Bootloader_Address_Struct.APP_Backup_Flag = APP_Backup2_ADDR;
				}
				else
				{
					Bootloader_Address_Struct.APP_Backup_Flag = APP_Backup1_ADDR;
				}

				STM32_Flash_EraseWrite(Bootloader_Address_Base,(uint8_t *)&Bootloader_Address_Struct,sizeof(Bootloader_Address_Struct));
				
				tx_thread_sleep(1000);
				__set_PRIMASK(1);
				HAL_NVIC_SystemReset();	
			}
			else
			{
				//光源更新			
				Frame_UART_RX_TX();
				Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
				Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
				Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
				Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
				Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
				Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
				Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
				Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */
				
				uint8_t cnt = 0;
				while((!Frame_Param_Struct.Upgrade_status) && (cnt < 10))
				{
					tx_thread_sleep(1);
					cnt++;
				}
				*pdata++ = Frame_Param_Struct.Upgrade_Device;
				*pdata++ = Upgrade_Start_status;
				sendsize = pdata - sendbuff;
				Frame_UDP_Data_Send(Frame_CFG_CMD_FirmwareUpgrade_Start,sendbuff,sendsize);
			
				_tx_thread_sleep(100);
				Frame_UART_RX_TX_LOW();
			}
		}
		break;
		
		case  Frame_CFG_CMD_DebugTest :			/*测试*/ 
		{
			Frame_UART_RX_TX();
			
			Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
			Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
			Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
			Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
			Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
			Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
			Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
			Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */
			
			_tx_thread_sleep(50);
			Frame_UART_RX_TX_LOW();
		}
		break;
		case Frame_INFO_CMD_LightSource_Temperature:        /*查询指定光源灯板温度*/
		{
			if(TempQueryPending == 0)
			{
				TempQueryID = (Frame_Param_Struct.Data[1] << 8) | Frame_Param_Struct.Data[0];
				TempQueryPending = 1;

				Frame_UART_RX_TX();
				Frame_UART_Data_Send(&huart1,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口1发送 */
				Frame_UART_Data_Send(&huart2,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口2发送 */
				Frame_UART_Data_Send(&huart3,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口3发送 */
				Frame_UART_Data_Send(&huart4,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口4发送 */
				Frame_UART_Data_Send(&huart5,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口5发送 */
				Frame_UART_Data_Send(&huart6,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口6发送 */
				Frame_UART_Data_Send(&huart7,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口7发送 */
				Frame_UART_Data_Send(&huart8,(uint8_t *)&Frame_Param_Struct,Frame_Param_Struct.FrameSize);	/* 串口8发送 */
				/* 各路DE回接收由 HAL_UART_TxCpltCallback 在发送完成后逐路完成 */
			}
		}
		break;

		default :	break;
	}
}

/*
*	UDP发送数据
*/
NX_PACKET* TraPacket;
UINT source_port = SOURCE_PORT; //端口8888
ULONG source_ip_address = IP_ADDRESS(SOURCE_IP_ADDR0, SOURCE_IP_ADDR1, SOURCE_IP_ADDR2, SOURCE_IP_ADDR3);//获取IP 192.168.0.1

static void Frame_UDP_Data_Send(uint16_t Cmd ,uint8_t *pData,uint16_t Size)
{
	UINT ret;
	uint8_t *pBuff; 
	uint8_t sum = 0;
	uint16_t DataSize = Size + 2;	
	
	memset(Frame_DataBuff_Tx,0,strlen((char *)Frame_DataBuff_Tx));/*将Frame_DataBuff_Tx填充为0*/
	pBuff = Frame_DataBuff_Tx;
	
	*pBuff++ = (Frame_Start >> 8) & 0xFF;
	*pBuff++ =  Frame_Start 	  & 0xFF;
	*pBuff++ =  DataSize 		  & 0xFF;
	*pBuff++ = (DataSize >> 8) 	  & 0xFF;
	
	sum += *pBuff++ = Cmd & 0xFF;
	sum += *pBuff++ = (Cmd >> 8) & 0xFF;
	
	for( uint16_t i = 0 ; i < Size ; i++ )
	{
		sum += *pBuff++ = pData[i];
	}
	*pBuff++ = sum;	
	/* 申请发送数据包 */
	ret = nx_packet_allocate(&pool_0, &TraPacket, NX_UDP_PACKET, TX_WAIT_FOREVER);
	if (ret)
	{
		ERROR_HANDLER();  
	}
	/*将要发送的数据附加到TraPacket */
	ret = nx_packet_data_append(TraPacket, (VOID *)Frame_DataBuff_Tx, pBuff - Frame_DataBuff_Tx, &pool_0, TX_WAIT_FOREVER);

	if (ret)
	{
		ERROR_HANDLER();
	}
	/* 发送数据包到UDP发送端 */
	ret =  nx_udp_socket_send(&UDPSocket, TraPacket, source_ip_address, source_port);
}

static void Frame_UDP_Data_Send_Raw(uint8_t *pData,uint16_t Size)
{
	UINT ret;
	NX_PACKET *TxPacket;

	ret = nx_packet_allocate(&pool_0, &TxPacket, NX_UDP_PACKET, TX_WAIT_FOREVER);
	if (ret)
	{
		return;
	}

	ret = nx_packet_data_append(TxPacket, (VOID *)pData, Size, &pool_0, TX_WAIT_FOREVER);
	if (ret)
	{
		nx_packet_release(TxPacket);
		return;
	}

	nx_udp_socket_send(&UDPSocket, TxPacket, source_ip_address, source_port);
}

/* 温度应答转发线程：在任务上下文中发送UDP，避免在中断中调用NetX */
static void AppTaskTempResp(ULONG thread_input)
{
	(void)thread_input;

	while(1)
	{
		if(tx_semaphore_get(&TempRespSemaphore, 50) == TX_SUCCESS)
		{
			TempQueryPending = 0;
			Frame_UDP_Data_Send_Raw(TempRespFrame, 10);
		}
		else
		{
			/* 超时未收到应答：清除标志并重新挂起各路接收 */
			TempQueryPending = 0;
			Frame_UART_Start_Receive();
		}
	}
}

static void System_Frequency_Start_Handler(uint8_t Camera_Frequency, uint16_t LightSource_Frequency, uint16_t LightSource_Counter,uint16_t Camera_PulseWidth, uint16_t Camera_SyncDelay)
{
	uint16_t Light_Autoreload = 0;
	
	MX_TIM1_Init();
	MX_TIM4_Init();
//	LL_TIM_ClearFlag_UPDATE(Count_TIMx);
//	if(Frame_Param_Struct.System_Mode == System_Mode_MaterialMode || ((Frame_Param_Struct.System_Mode == System_Mode_OLATMode)) \
//	   || Frame_Param_Struct.System_Mode == System_Mode_Relighting_Mode)//材质结算 重光照、OLAT模式
//	{
		if((Frame_Param_Struct.LightSource_Counter > 1) && (Frame_Param_Struct.LightSource_Frequency >0))
		{	
			//使用中断计数模式
			LL_TIM_SetCounter(Count_TIMx,0);											// count_TIMx = TIM4 --- 从0开始计数
			LL_TIM_SetAutoReload(Count_TIMx,Frame_Param_Struct.LightSource_Counter - 1);// 设置脉冲数量
			
			LL_TIM_EnableIT_UPDATE(Count_TIMx) ;										// 计数中断
			LL_TIM_EnableCounter(Count_TIMx)   ;										// 使能计数器									
			/*光源频率设置，频率范围为1~1000HZ*/ 
//			Light_Autoreload = ((1000-Frame_Param_Struct.LightSource_Frequency)+1);
//			Light_Autoreload = ((1000000.0)/Frame_Param_Struct.LightSource_Frequency);
//			Light_Autoreload = ((10000.0)-(Frame_Param_Struct.LightSource_Frequency*10)+10);
//			Light_Autoreload = (((10000.0)/Frame_Param_Struct.LightSource_Frequency)+0.5);
			Light_Autoreload = (((10000.0)/Frame_Param_Struct.LightSource_Frequency)-1);
			/*设置频率*/
			LL_TIM_SetAutoReload(Light_TIMx,Light_Autoreload);
			/*占空比调节*/
			LL_TIM_OC_SetCompareCH3(Light_TIMx,(Light_Autoreload)/2);
			/*开启光源PWM*/
			LL_TIM_ClearFlag_CC3	(Light_TIMx);
			LL_TIM_EnableAllOutputs (Light_TIMx);							//高级定时器还需要使能主输出模式
			LL_TIM_CC_EnableChannel (Light_TIMx, Light_TIM_Channe3); 		//定时器1的通道3  --- 使能PWM通道
			LL_TIM_EnableIT_CC3 	(Light_TIMx);							//使能中断
			LL_TIM_EnableCounter	(Light_TIMx);							//使能输出比较通道
			
		}
//	}	
}

/* 光源中断 */
void TIM1_CC_IRQHandler(void)
{
	if(LL_TIM_IsActiveFlag_CC3(Light_TIMx))
	{
		LL_TIM_ClearFlag_CC3(Light_TIMx);
	}
}

/* 输出一定个数PWM */
void TIM4_IRQHandler(void)
{
	if(LL_TIM_IsActiveFlag_UPDATE(Count_TIMx))
	{
		CNT++;
		LL_TIM_ClearFlag_UPDATE(Count_TIMx);
		LL_TIM_DisableCounter(Count_TIMx);
		LL_TIM_SetCounter(Count_TIMx,0);
		TIM_CCxChannelCmd(TIM1,TIM_CHANNEL_3,TIM_CCx_DISABLE);
		LL_TIM_SetCounter(Light_TIMx,0);
	}	
}
/*
**	频率停止
*/
static void System_Frequency_Stop_Handler(void)
{
	/*停止输出时序*/
	CNT=0;
	LL_TIM_DisableCounter(Light_TIMx);
	LL_TIM_DisableCounter(Count_TIMx);
	/*失能高级定时器TIM1*/
	LL_TIM_DisableAllOutputs(TIM1);
	LL_TIM_DisableIT_UPDATE(Count_TIMx);
	/*	TIM1捕获通道关闭*/
	LL_TIM_CC_DisableChannel(Light_TIMx, Light_TIM_Channe3);
	/*定时器计数清零*/
	LL_TIM_SetCounter(Light_TIMx,0);
	LL_TIM_SetCounter(Count_TIMx,0);
}

/* 光源中断 */
void TIM1_UP_TIM10_IRQHandler(void)
{
	LL_TIM_ClearFlag_UPDATE(Light_TIMx);
	if(!LL_TIM_IsEnabledCounter(Count_TIMx))
	{
		System_Frequency_Stop_Handler();
	}
}























