#ifndef __AXP20X_H
#define __AXP20X_H
#include "myiic.h"   
#include "axp_cfg.h" 


#define axp_20X_adr	AXP_DEVICES_ADDR            //������ַ

					  
u8 AXP20X_ReadOneByte(u16 ReadAddr);							//ָ����ַ��ȡһ���ֽ�

void AXP20X_WriteOneByte(u16 WriteAddr,u8 DataToWrite);		//ָ����ַд��һ���ֽ�



void AXP20X_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len);//ָ����ַ��ʼд��ָ�����ȵ�����

u32 AXP20X_ReadLenByte(u16 ReadAddr,u8 Len);					//ָ����ַ��ʼ��ȡָ����������



void AXP20X_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite);	//��ָ����ַ��ʼд��ָ�����ȵ�����

void AXP20X_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead);   	//��ָ����ַ��ʼ����ָ�����ȵ�����


void AXP20X_Init(void); //��ʼ��IIC

#endif
















