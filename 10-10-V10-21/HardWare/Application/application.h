#ifndef __APPLICATION_H
#define __APPLICATION_H


#include "includes.h"



/*
*********************************************************************************************************
*	                                    IP相关
*********************************************************************************************************
*/

/* 本地IP地址 */
#define LOCAL_IP_ADDR0                        192
#define LOCAL_IP_ADDR1                        168
#define LOCAL_IP_ADDR2                        0
#define LOCAL_IP_ADDR3                        1
/* 本地端口号 */
#define LOCAL_PORT                            8888

/* 远端IP地址 */
#define SOURCE_IP_ADDR0                        192
#define SOURCE_IP_ADDR1                        168
#define SOURCE_IP_ADDR2                        0
#define SOURCE_IP_ADDR3                        100
/* 远端端口号 */
#define SOURCE_PORT                            8888

#define Frame_Start    						   0x4C55

#define Frame_StartMark   					   0		
#define Frame_SizeMark    					   2
#define Frame_CmdMark     					   4
#define Frame_DataMark    					   6


#define Frame_CountSizeMin    
#define Frame_Data_Size   					   1024

#pragma pack(1)

typedef struct
{
	uint16_t  Header;
	uint16_t  DataSize;			  		   /* Data length */
	union
	{
		uint16_t buff;      	  		   /* frame command buff */
		struct
		{
			uint16_t ack     : 1; 		   /* frame command ack */
			uint16_t cmd     :15; 		   /* frame command */
		}_struct;
	}CMD_Union;

	uint8_t Data[Frame_Data_Size];	
	uint16_t  FrameSize;          		   /* Data length */
	
//	uint8_t CheckSum;       /* the CheckSum between len and CheckSum is checked */
//Parameter
	uint32_t Local_IP;                     /* 本地IP */
	uint16_t Local_Port;                   /* 本地端口 */
	uint8_t  Local_CFG_Status;			   /* 配置标志，用于控制器IP命令返回的 */
//	uint8_t  Camera_Check;                 /* 相机检测 */
	uint8_t  System_Mode;                  /* 系统工作模式  */

	
	uint16_t LightSource_Frequency;		   /* 光源触发频率  */
	uint16_t LightSource_Counter  ;        /* 光源触发次数  */
	uint8_t  Camera_Type		  ;		   /* 相机类型	    */
	uint8_t  Camera_Frequency     ;        /* 相机分频参数  */
	uint16_t Camera_PulseWidth    ;        /* 相机触发脉宽  */
	uint16_t Camera_SyncDelay     ;        /* 相机同步延迟  */
	
	uint8_t  Soft_Start;                   /* 软起动 		*/
	uint8_t  Stop;                         /* 停止 			*/
	
	uint8_t  Upgrade_Device;               /* 升级设备 		*/
	uint8_t  Upgrade_Size;                 /* 更新数据包大小*/
	
	uint8_t  Upgrade_Sequence;             /* 数据包序号 */
	uint8_t  Upgrade_Data[512];            /* 更新数据包 */
	uint8_t  Upgrade_Start;                /* 升级设备 */
	uint8_t  Upgrade_status;               /* 升级状态 */
	
}Frame_Param_TypeDef;

typedef struct
{
	uint8_t  Local_Flag;                   /* IP标记 */      
	uint32_t Local_IP;                     /* 本地IP */
	uint16_t Local_Port;                   /* 本地端口 */
}Memory_Param_TypeDef;


#pragma pack()



#define Frame_CFG_CMD_Controller_IP                    0xAA03		//控制器IP配置
#define Frame_CFG_CMD_LightSource_ID                   0x0004		//光源ID配置
#define Frame_CFG_CMD_NixieTube_Display                0x0008		//数码管显示配置

#define Frame_CFG_CMD_System_Mode                      0x5502		//系统工作模式配置
#define Frame_CFG_CMD_System_Frequency                 0x5504		//系统工作频率配置

#define Frame_CFG_CMD_LightSource_Brightness           0x5509		//材质解算模式下光源参数配置
#define Frame_CFG_CMD_DebugMode                        0x0010		//装调模式下光源参数配置
#define Frame_CFG_CMD_OLATMode                         0x0302		//OLAT模式下频闪参数配置
#define	Frame_CFG_CMD_Heavy_lighting_mode			         0x0502		//重光照模式下光源参数配置
#define Frame_CFG_CMD_Data_Set_Over                    0x5505		//数据配置完成
#define Frame_CFG_CMD_LoopData_Set                     0x5501		//循环数据量配置

#define Frame_CFG_CMD_Soft_Start                       0xFF02		//软启动
#define Frame_CFG_CMD_Stop                             0xFF03		//停止

#define Frame_CFG_CMD_FirmwareUpgrade_Prepare          0xAA04		//固件升级准备
#define Frame_CFG_CMD_FirmwareUpgrade_Data             0xAA05		//固件升级数据配置
#define Frame_CFG_CMD_FirmwareUpgrade_Start            0xAA09		//开始固件升级

#define Frame_CFG_CMD_DebugTest                        0x0001
#define	Frame_CFG_CMD_Heavy_Test 					   0x0002		//重光照测试模式下光源参数配置
#define Frame_INFO_CMD_LightSource_Temperature        0x5503        //查询指定光源灯板温度



/*	系统工作模式	*/
#define System_Mode_DebugMode         			0x00				//装调模式
#define System_Mode_MaterialMode      			0x01				//材质解算模式
#define System_Mode_Relighting_Mode   			0x02				//重关照模式
#define System_Mode_OLATMode          			0x03				//OLAT模式
#define System_Mode_Relighting_Mode_Test       	0x10				//重关照测试模式


#define Bootloader_Address_Base        	ADDR_FLASH_SECTOR_4			//第四扇区 0x08010000 - 0x0801FFFF 64 KB字节
#define APP_Run_Address					ADDR_FLASH_SECTOR_5			//第五扇区 0x08020000 - 0x0803FFFF 128KB字节
#define APP_Backup1_ADDR				ADDR_FLASH_SECTOR_7			//第七扇区 0x08060000 - 0x0807FFFF 128KB字节
#define APP_Backup2_ADDR				ADDR_FLASH_SECTOR_9			//第九扇区 0x080A0000 - 0x080BFFFF 128KB字节

#define APP_Sectors_NUM							2


#define APP_Backup1								0
#define APP_Backup2								1

#pragma pack(1)
typedef struct
{
	uint32_t APP_Run_Flag	   ;
	uint32_t APP_Backup_Flag   ;
}Bootloader_Address_Typedef	   ;

#pragma pack()

void Frame_UART_RX_TX(void);
void Frame_UART_RX_TX_LOW(void);
void Frame_UART_Start_Receive(void);

#endif




