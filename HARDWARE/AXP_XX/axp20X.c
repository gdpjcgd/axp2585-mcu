#include "axp20X.h" 
#include "delay.h"



//��ʼ��IIC�ӿ�

void AXP20X_Init(void)
									{
										IIC_Init();
									}

									
//��AXP20Xָ����ַ����һ������
//ReadAddr:��ʼ�����ĵ�ַ  
//����ֵ  :����������

u8 AXP20X_ReadOneByte(u16 ReadAddr)
											{				  
												u8 temp=0;	
												
												IIC_Start(); 
												
												//IIC_Wait_Ack(); 
												
												IIC_Send_Byte(axp_20X_adr+((ReadAddr/255<<1)));   //����������ַ��д��ģʽ��
												
												IIC_Wait_Ack(); 
														
												IIC_Send_Byte(ReadAddr%255);     //���ͼĴ�����ַ
														
												IIC_Wait_Ack();	
														
												IIC_Start();  	
		
												IIC_Send_Byte(axp_20X_adr+1);           //����ģʽ��ַ��		
														
												IIC_Wait_Ack();	 
														
												temp=IIC_Read_Byte(0);	
														
												IIC_Stop();//����һ��ֹͣ����	 
																									
												return temp;
														
											}
											
											
//��AXP20Xָ����ַд��һ������
//WriteAddr  :д�����ݵ�Ŀ�ĵ�ַ    
//DataToWrite:Ҫд�������
											
void AXP20X_WriteOneByte(u16 WriteAddr,u8 DataToWrite)
												{				   	  	    																 
														IIC_Start();  
													
													//	IIC_Wait_Ack();	
				
														IIC_Send_Byte(axp_20X_adr+((WriteAddr/255<<1)));   //����������ַ��д��ģʽ��
											
														IIC_Wait_Ack();	 
													
														IIC_Send_Byte(WriteAddr%255);     //���ͼĴ�����ַ
													
														IIC_Wait_Ack(); 	
													
														IIC_Send_Byte(DataToWrite);     //�����ֽ�	
													
														IIC_Wait_Ack();  	
													
														IIC_Stop();//����һ��ֹͣ���� 
													
														delay_ms(10);	 
												}
												
												
												
												
												
												
//��AXP20X�����ָ����ַ��ʼд�볤��ΪLen������
//�ú�������д��16bit����32bit������.
//WriteAddr  :��ʼд��ĵ�ַ  
//DataToWrite:���������׵�ַ
//Len        :Ҫд�����ݵĳ���2,4
												
void AXP20X_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len)
											{  	
												u8 t;
												for(t=0;t<Len;t++)
												{
													AXP20X_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
												}												    
											}
											

//��AXP20X�����ָ����ַ��ʼ��������ΪLen������
//�ú������ڶ���16bit����32bit������.
//ReadAddr   :��ʼ�����ĵ�ַ 
//����ֵ     :����
//Len        :Ҫ�������ݵĳ���2,4
											
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
											

//��AXP20X�����ָ����ַ��ʼ����ָ������������
//ReadAddr :��ʼ�����ĵ�ַ 
//pBuffer  :���������׵�ַ
//NumToRead:Ҫ�������ݵĸ���
void AXP20X_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead)
								{
									while(NumToRead)
									{
										*pBuffer++=AXP20X_ReadOneByte(ReadAddr++);	
										NumToRead--;
									}
								}  
								
		
								
//��AXP20X�����ָ����ַ��ʼд��ָ������������
//WriteAddr :��ʼд��ĵ�ַ 
//pBuffer   :���������׵�ַ
//NumToWrite:Ҫд�����ݵĸ���
void AXP20X_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite)
									{
										while(NumToWrite--)
										{
											AXP20X_WriteOneByte(WriteAddr,*pBuffer);
											WriteAddr++;
											pBuffer++;
										}
									}
 











