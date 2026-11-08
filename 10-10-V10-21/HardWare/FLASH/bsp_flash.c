#include "bsp_flash.h"
#include "string.h"

/**
 * @brief       获取某个地址所在的flash扇区
 * @param       addr   : flash地址
 * @retval      0~11, 即addr所在的扇区
 */
uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_SECTOR_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_SECTOR_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_SECTOR_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_SECTOR_10;  
  }
   else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
  {
    sector = FLASH_SECTOR_11;
  }

  return sector;
}

/**
  * @brief  Gets sector Size
  * @param  None
  * @retval The size of a given sector
  */
static uint32_t GetSectorSize(uint32_t Sector)
{
  uint32_t sectorsize = 0x00;

  if((Sector == FLASH_SECTOR_0) || (Sector == FLASH_SECTOR_1) || (Sector == FLASH_SECTOR_2) || (Sector == FLASH_SECTOR_3))
  {
    sectorsize = 16 * 1024;
  }
  else if(Sector == FLASH_SECTOR_4)
  {
    sectorsize = 64 * 1024;
  }
  else
  {
    sectorsize = 128 * 1024;
  }  
  return sectorsize;
}

/**
  * @brief  擦除函数
  * @param  None
  * @retval 第一个参数Sector擦除的扇区；第二个参数NbSectors 一次要擦除的扇区数量
  */
HAL_StatusTypeDef  STM32_Flash_Erase(uint32_t Sector, uint32_t NbSectors)
{
	HAL_StatusTypeDef status = HAL_ERROR;
	uint32_t SectorError = 0;
	FLASH_EraseInitTypeDef EraseInitStruct;
	
	//解锁
	HAL_FLASH_Unlock();
	
	EraseInitStruct.TypeErase			= FLASH_TYPEERASE_SECTORS;//擦除类型，扇区擦除
	EraseInitStruct.VoltageRange	=	FLASH_VOLTAGE_RANGE_3;	//电压范围，VCC=2.7~3.6V之间
	EraseInitStruct.Sector				= Sector;									//要擦除的扇区
	EraseInitStruct.NbSectors			=	NbSectors;							//一次要擦除扇区的数量

	status	=	HAL_FLASHEx_Erase(&EraseInitStruct,&SectorError);
	
	HAL_FLASH_Lock();
	return status;
}

HAL_StatusTypeDef STM32_Flash_Write(uint32_t Address, uint8_t *pData, uint16_t Len)
{
	uint32_t SectorError = 0;
	uint32_t Address_End;					//结束地址
	uint32_t FirstSector = 0, NbOfSectors = 0;
	
	Address_End = Address + Len;
	
	HAL_StatusTypeDef	status = HAL_ERROR;
	
	if((Address >= FLASH_USER_START_ADDR) && (Address_End <= FLASH_USER_END_ADDR))
	{
		HAL_FLASH_Unlock();//解锁
		for(uint16_t index = 0; index<Len;)
		{
			status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,Address,*((uint16_t*)pData));
			if(status != HAL_OK)
			{
				return status;
			}
			else
			{
				pData		+=2;//半字移动两个
				Address	+=2;//地址＋2
				index 	+=2;
			}
		}
		HAL_FLASH_Lock();
	}
	else
	{
		return status;
	}
	
	return HAL_OK;

}

HAL_StatusTypeDef STM32_Flash_EraseWrite(uint32_t Address, uint8_t *pData, uint16_t Len)
{
	uint32_t SectorError = 0;
	uint32_t Address_End;					//结束地址
	uint32_t FirstSector = 0, NbOfSectors = 0;
	
	Address_End = Address + Len;
	
	HAL_StatusTypeDef	status = HAL_ERROR;
	
	FLASH_EraseInitTypeDef	EraseInitStruct;
	
	if((Address >= FLASH_USER_START_ADDR) && (Address_End <= FLASH_USER_END_ADDR))
	{
		HAL_FLASH_Unlock();//解锁
		
		FirstSector	= GetSector(Address);//获取第一个扇区
		NbOfSectors	=	GetSector(Address + Len) - FirstSector + 1;
		
		EraseInitStruct.TypeErase			=	FLASH_TYPEERASE_SECTORS;		/* 擦除类型，扇区擦除 */
		EraseInitStruct.VoltageRange	= FLASH_VOLTAGE_RANGE_3;			/* 电压范围，VCC=2.7~3.6V之间!! */
		EraseInitStruct.Sector				=	FirstSector;								/* 要擦除的扇区 */
		EraseInitStruct.NbSectors			=	NbOfSectors;								/* 一次要擦除的扇区数量 */
		
		status	=	HAL_FLASHEx_Erase(&EraseInitStruct,&SectorError);
		
		if(status != HAL_OK)
		{
			return status;
		}
		
		for(uint16_t index = 0; index < Len; )
		{
			status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,Address,*((uint16_t*)pData));
			
			if(status != HAL_OK)
			{
				return status;
			}
			else
			{
				pData		+= 2;
				Address	+= 2;
				index		+= 2;
			}
		}
		HAL_FLASH_Lock();
	}
	else
	{
		return status;
	}
	return HAL_OK;
}

HAL_StatusTypeDef STM32_Flash_Read(uint32_t Address, uint8_t *pData, uint16_t Len)
{
	memcpy(pData,(uint8_t*)Address,Len);
	return HAL_OK;
}

/**
 * @brief       从指定地址读取一个字 (32位数据)
 * @param       faddr   : 读取地址 (此地址必须为4倍数!!)
 * @retval      读取到的数据 (32位)
 */
uint32_t stmflash_read_word(uint32_t faddr)
{
    return *(volatile uint32_t *)faddr;
}

/**
 * @brief       从指定地址开始读出指定长度的数据
 * @param       raddr : 起始地址
 * @param       pbuf  : 数据指针
 * @param       length: 要读取的字(32)数,即4个字节的整数倍
 * @retval      无
 */
void stmflash_read(uint32_t raddr, uint32_t *pbuf, uint32_t length)
{
    uint32_t i;

    for (i = 0; i < length; i++)
    {
        pbuf[i] = stmflash_read_word(raddr);    /* 读取4个字节. */
        raddr += 4;                             /* 偏移4个字节. */
    }
}
