#include "led.h"
#include "delay.h"
#include "sys.h"
#include "key.h"
#include "usart.h"
#include "lcd.h"


#include "myiic.h"
#include "axp20X.h"

#include "axp-core.h"

struct axp_charger charger_2585 ,*charger;


 int main(void){

	u8   key,ye=0;
	u8   j=0; 
	u16  i=0;
    int  temp2;
	int  timer=0;
	int rdc;
	uint8_t val[2];

	charger=&charger_2585;
				 
	SystemInit();
	delay_init(72);	     //延时初始化
	NVIC_Configuration();
	uart_init(115200);
	LED_Init();
	KEY_Init();
	LCD_Init();
	AXP20X_Init();		//IIC初始化 

    init_axp173(charger ) ;


								 
	POINT_COLOR=GREEN;//设置字体为绿色 
	LCD_ShowString(90,70,"X-power");	
	POINT_COLOR=RED;//设置字体为红色 
	LCD_ShowString(85,90,"AXP2585_TEST");
	LCD_ShowString(90,110,"CPU: stm32");
    POINT_COLOR=BLUE;//设置字体为蓝色	
	LCD_ShowString(60,150,"control:");	
	LCD_ShowString(60,170,"KEY1:home");			 
	LCD_ShowString(60,190,"KEY0:Fuel Guage");			 
	LCD_ShowString(60,210,"KEY2:DCDC&LDO");
	POINT_COLOR=BLUE;//设置字体为蓝色	 
				
	
					
		while(1){
	               
				   key=KEY_Scan();

				if(key==1)//KEY0按下,电量计显示
				  	    {
						    ye=1;
						    LCD_Fill(0,0,239,638,WHITE);//清除全屏   
							POINT_COLOR=GREEN;//设置字体为绿色 
							LCD_ShowString(85,0,"Fuel Guage");						
							LCD_ShowString(5,30,"BATSENSE:");
							LCD_ShowNum(120,30,charger->vbat,4,20);

							LCD_ShowString(5,50,"CHR Cur:");
							LCD_ShowNum(120,50,charger->chgcur,4,20); 

							LCD_ShowString(5,70,"DISCHR Cur:");
							LCD_ShowNum(120,70,charger->ibat,4,20);	//DISCHR Cur

							LCD_ShowString(5,90,"CHR Coul:");
							LCD_ShowString(5,110,"DISCHR Coul:");
							LCD_ShowString(5,130,"Coul Count:");

							LCD_ShowString(5,150,"Coul PCT:");
							LCD_ShowNum(120,150,charger->rest_cap-charger->base_restcap,4,20);//Coul PCT

							axp_reads(AXP_RDC_BUFFER0,2,val);
							rdc = val[1] * 2;
							LCD_ShowString(5,170,"RDC:");
							LCD_ShowNum(120,170,rdc,4,20);

							LCD_ShowString(5,190,"OCV:");
							LCD_ShowNum(120,190,charger->ocv,4,20);

							LCD_ShowString(5,210,"OCV PCT:");
							LCD_ShowNum(120,210,charger->ocv_rest_cap,4,20);
	
							POINT_COLOR=RED;//设置字体为红色 
							LCD_ShowString(5,230,"Fuel Result:");
							LCD_ShowNum(120,230,charger->rest_cap,4,20);
	
						    POINT_COLOR=GREEN;//设置字体为绿色 
							LCD_ShowString(5,250,"BatCapacity:");
							LCD_ShowNum(120,250,charger->base_restcap,4,20);  //batCapacity

							LCD_ShowString(5,270,"PCT WL1:");
							LCD_ShowString(5,290,"PCT WL2:");	
	                      	}
															
				if(key==2)//KEY1按下,主页面
							{
							ye=0;					
							LCD_Fill(0,0,239,638,WHITE);//清除全屏   			
							POINT_COLOR=GREEN;//设置字体为绿色 
							LCD_ShowString(90,70,"X-power");	
							POINT_COLOR=RED;//设置字体为红色 
							LCD_ShowString(85,90,"AXP173_TEST");
							LCD_ShowString(90,110,"CPU: stm32");
			            	POINT_COLOR=BLUE;//设置字体为蓝色	
							LCD_ShowString(60,150,"control:");	
							LCD_ShowString(60,170,"KEY1:home");			 
							LCD_ShowString(60,190,"KEY0:Fuel Guage");			 
							LCD_ShowString(60,210,"KEY2:DCDC&LDO");					
							}
									
			   	if(key==3)//KEY_UP按下,读取字符串并显示
						{
						    ye=0;
						    if(j==1)								
							axp_regu_disable(LDO2);
							if(j==2)
							axp_regu_disable(LDO3);	
							if(j==3)		
							axp_regu_disable(LDO4); 
							if(j==4)   			
							axp_regu_disable(DCDC1);
							if(j==5)			
							axp_regu_disable(DCDC2);

							if(j==6)
								{							
						    		axp_set_voltage(LDO2,1000);
									axp_regu_enable(LDO2);
							   	}
						    if(j==7)
								{	
									axp_set_voltage(LDO3,1000);
									axp_regu_enable(LDO3);
								}
			
							if(j==8)
								 {
							    	axp_regu_enable(LDO4); 
							 		axp_set_voltage(LDO4,1000);
							 	}
							if(j==9)
								{
						    		axp_regu_enable(DCDC1);	
									axp_set_voltage(DCDC1,1000);
								}
							if(j==10)
						    	{
									axp_regu_enable(DCDC2);	
							   		 axp_set_voltage(DCDC2,1000);
								 		j=0;
							   }
										
							LCD_Fill(0,0,239,638,WHITE);//清除全屏   
							POINT_COLOR=GREEN;//设置字体为绿色 
							LCD_ShowString(85,0,"DCDC&LDO");
						
							LCD_ShowString(5,50,"DCDC1:");

							if(regu_staus(DCDC1))
							{
							    temp2 = axp_get_voltage(DCDC1);
								LCD_ShowNum(60,50,temp2,4,20);
								LCD_ShowString(130,50,"mV");
							}
							else
								LCD_ShowString(60,50,"Close");
						  		LCD_ShowString(5,70,"DCDC2:");

							if( regu_staus(DCDC2))
							{
	                            temp2 = axp_get_voltage(DCDC2);
								LCD_ShowNum(60,70,temp2,4,20);
								LCD_ShowString(130,70,"mV");
							}
							else
								LCD_ShowString(60,70,"Close");									
                    			LCD_ShowString(5,90,"LDO1:RTC POEN ALLWAYS");
                        		LCD_ShowString(5,110,"LDO2:");
						
							if(regu_staus(LDO2))
							{
								temp2=axp_get_voltage(LDO2);
								LCD_ShowNum(60,110,temp2,4,20);
								LCD_ShowString(130,110,"mV");
							}
							else
								LCD_ShowString(60,110,"Close");	
									
									
								LCD_ShowString(5,130,"LDO3:");
							if(regu_staus(LDO3))
									{
                                      temp2=axp_get_voltage(LDO3);
									  LCD_ShowNum(60,130,temp2,4,20);	
									  LCD_ShowString(130,130,"mV");
									}
									else
										LCD_ShowString(60,130,"Close");	
									
			         					LCD_ShowString(5,150,"LDO4:");
							if(regu_staus(LDO4))
									{
                                    temp2=axp_get_voltage(LDO4);
									LCD_ShowNum(60,150,temp2,4,20);
									LCD_ShowString(130,150,"mV");
									}
									else
										LCD_ShowString(60,150,"Close");										
							  			
										LCD_ShowString(90,200,"X-Powers");
						    			LCD_ShowString(90,230,"AXP173");
	
								j++;
							    }



					delay_ms(10);
								
					i++;
				
					timer++;

							
					if(timer==100*TIMER1)	//TIMER1S运行一次,电量监控并更新。
								{	
								    timer=0;
								   	axp_charging_monitor(charger);
										//电量信息打印到串口
									printf("Fuel Result:%d,Coul PCT:%d,OCV PCT:%d\n",charger->rest_cap,charger->rest_cap-charger->base_restcap,charger->ocv_rest_cap);
							//		printf("U64_test:%lld\n", (a/10));
									
								}


								
								if(i==20)
								{
									LED0=!LED0;//提示系统正在运行	
									i=0;

									if(ye==1)
									{
									LCD_Fill(0,0,239,638,WHITE);//清除全屏   
									POINT_COLOR=GREEN;//设置字体为绿色 
									LCD_ShowString(85,0,"Fuel Guage");						
									LCD_ShowString(5,30,"BATSENSE:");
									LCD_ShowNum(120,30,charger->vbat,4,20);
		
									LCD_ShowString(5,50,"CHR Cur:");
									LCD_ShowNum(120,50,charger->chgcur,4,20); 
		
									LCD_ShowString(5,70,"DISCHR Cur:");
									LCD_ShowNum(120,70,charger->ibat,4,20);	//DISCHR Cur
		
									LCD_ShowString(5,90,"CHR Coul:");
									LCD_ShowString(5,110,"DISCHR Coul:");
									LCD_ShowString(5,130,"Coul Count:");
		
									LCD_ShowString(5,150,"Coul PCT:");
									LCD_ShowNum(120,150,charger->rest_cap-charger->base_restcap,4,20);//Coul PCT


									axp_reads(AXP_RDC_BUFFER0,2,val);
									rdc = val[1] * 2;
									LCD_ShowString(5,170,"RDC:");
									LCD_ShowNum(120,170,rdc,4,20);
		
									LCD_ShowString(5,190,"OCV:");
									LCD_ShowNum(120,190,charger->ocv,4,20);
		
									LCD_ShowString(5,210,"OCV PCT:");
									LCD_ShowNum(120,210,charger->ocv_rest_cap,4,20);
			
									POINT_COLOR=RED;//设置字体为红色 
									LCD_ShowString(5,230,"Fuel Result:");
									LCD_ShowNum(120,230,charger->rest_cap,4,20);
			
								    POINT_COLOR=GREEN;//设置字体为绿色 
									LCD_ShowString(5,250,"BatCapacity:");
									LCD_ShowNum(120,250,charger->base_restcap,4,20);  //batCapacity
		
									LCD_ShowString(5,270,"PCT WL1:");
									LCD_ShowString(5,290,"PCT WL2:");

								
									}
								}	

								
							}
						 }

