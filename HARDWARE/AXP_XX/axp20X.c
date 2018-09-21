#include "axp20X.h" 
#include "delay.h"



//初始化IIC接口

void AXP20X_Init(void)
									{
										IIC_Init();
									}

									
//在AXP20X指定地址读出一个数据
//ReadAddr:开始读数的地址  
//返回值  :读到的数据

u8 AXP20X_ReadOneByte(u16 ReadAddr)
											{				  
												u8 temp=0;	
												
												IIC_Start(); 
												
												//IIC_Wait_Ack(); 
												
												IIC_Send_Byte(axp_20X_adr+((ReadAddr/255<<1)));   //发送器件地址（写的模式）
												
												IIC_Wait_Ack(); 
														
												IIC_Send_Byte(ReadAddr%255);     //发送寄存器地址
														
												IIC_Wait_Ack();	
														
												IIC_Start();  	
		
												IIC_Send_Byte(axp_20X_adr+1);           //（读模式地址）		
														
												IIC_Wait_Ack();	 
														
												temp=IIC_Read_Byte(0);	
														
												IIC_Stop();//产生一个停止条件	 
																									
												return temp;
														
											}
											
											
//在AXP20X指定地址写入一个数据
//WriteAddr  :写入数据的目的地址    
//DataToWrite:要写入的数据
											
void AXP20X_WriteOneByte(u16 WriteAddr,u8 DataToWrite)
												{				   	  	    																 
														IIC_Start();  
													
													//	IIC_Wait_Ack();	
				
														IIC_Send_Byte(axp_20X_adr+((WriteAddr/255<<1)));   //发送器件地址（写的模式）
											
														IIC_Wait_Ack();	 
													
														IIC_Send_Byte(WriteAddr%255);     //发送寄存器地址
													
														IIC_Wait_Ack(); 	
													
														IIC_Send_Byte(DataToWrite);     //发送字节	
													
														IIC_Wait_Ack();  	
													
														IIC_Stop();//产生一个停止条件 
													
														delay_ms(10);	 
												}
												
												
												
												
												
												
//在AXP20X里面的指定地址开始写入长度为Len的数据
//该函数用于写入16bit或者32bit的数据.
//WriteAddr  :开始写入的地址  
//DataToWrite:数据数组首地址
//Len        :要写入数据的长度2,4
												
void AXP20X_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len)
											{  	
												u8 t;
												for(t=0;t<Len;t++)
												{
													AXP20X_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
												}												    
											}
											

//在AXP20X里面的指定地址开始读出长度为Len的数据
//该函数用于读出16bit或者32bit的数据.
//ReadAddr   :开始读出的地址 
//返回值     :数据
//Len        :要读出数据的长度2,4
											
u32 AXP20X_ReadLenByte(u16 ReadAddr,u8 Len)
											{  	
												u8 t;
												u32 temp=0;
												for(t=0;t<Len;t++)
												{
													temp<<=8;
													temp+=AXP20X_ReadOneByte(ReadAddr+Len-t-1); 	 				   
												}
												return temp;												    
											}
											

//在AXP20X里面的指定地址开始读出指定个数的数据
//ReadAddr :开始读出的地址 
//pBuffer  :数据数组首地址
//NumToRead:要读出数据的个数
void AXP20X_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead)
								{
									while(NumToRead)
									{
										*pBuffer++=AXP20X_ReadOneByte(ReadAddr++);	
										NumToRead--;
									}
								}  
								
		
								
//在AXP20X里面的指定地址开始写入指定个数的数据
//WriteAddr :开始写入的地址 
//pBuffer   :数据数组首地址
//NumToWrite:要写入数据的个数
void AXP20X_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite)
									{
										while(NumToWrite--)
										{
											AXP20X_WriteOneByte(WriteAddr,*pBuffer);
											WriteAddr++;
											pBuffer++;
										}
									}
 











