#ifndef __AXP20X_H
#define __AXP20X_H
#include "myiic.h"   
#include "axp_cfg.h" 


#define axp_20X_adr	AXP_DEVICES_ADDR            //器件地址

					  
u8 AXP20X_ReadOneByte(u16 ReadAddr);							//指定地址读取一个字节

void AXP20X_WriteOneByte(u16 WriteAddr,u8 DataToWrite);		//指定地址写入一个字节



void AXP20X_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len);//指定地址开始写入指定长度的数据

u32 AXP20X_ReadLenByte(u16 ReadAddr,u8 Len);					//指定地址开始读取指定长度数据



void AXP20X_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite);	//从指定地址开始写入指定长度的数据

void AXP20X_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead);   	//从指定地址开始读出指定长度的数据


void AXP20X_Init(void); //初始化IIC

#endif
















