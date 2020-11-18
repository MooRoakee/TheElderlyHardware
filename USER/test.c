#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"   
#include "lcd.h" 
#include "usmart.h" 
#include "key.h"
#include "usart2.h"
#include "hc05.h"
#include "string.h"
#include "gps.h"
u8 USART1_TX_BUF[800];
//串口 1,发送缓存区
nmea_msg gpsx; //GPS 信息
__align(4) u8 dtbuf[50]; //打印缓存器
const u8*fixmode_tbl[4]={"Fail","Fail"," 2D "," 3D "}; //fix mode 字符串
//显示 GPS/北斗定位信息
void Gps_Msg_Show(void)
{
float tp;
POINT_COLOR=BLUE;
tp=gpsx.longitude;
sprintf((char *)dtbuf,"Longitude:%.5f %1c ",tp/=100000,gpsx.ewhemi);
//得到经度字符串
LCD_ShowString(30,120,200,16,16,dtbuf);
tp=gpsx.latitude;
sprintf((char *)dtbuf,"Latitude:%.5f %1c ",tp/=100000,gpsx.nshemi);
//得到纬度字符串
LCD_ShowString(30,140,200,16,16,dtbuf);
tp=gpsx.altitude;
sprintf((char *)dtbuf,"Altitude:%.1fm ",tp/=10);
//得到高度字符串
LCD_ShowString(30,160,200,16,16,dtbuf);
tp=gpsx.speed;
	sprintf((char *)dtbuf,"Speed:%.3fkm/h ",tp/=1000);
//得到速度字符串
LCD_ShowString(30,180,200,16,16,dtbuf);
if(gpsx.fixmode<=3)
//定位状态
{
sprintf((char *)dtbuf,"Fix Mode:%s",fixmode_tbl[gpsx.fixmode]);
LCD_ShowString(30,200,200,16,16,dtbuf);
}
sprintf((char *)dtbuf,"GPS Valid satellite:%02d",gpsx.posslnum);
//用于定位的 GPS/北斗卫星数
LCD_ShowString(30,220,200,16,16,dtbuf);
sprintf((char *)dtbuf,"GPS Visible satellite:%02d",gpsx.svnum%100);
//可见 GPS 卫星数
LCD_ShowString(30,240,200,16,16,dtbuf);
sprintf((char *)dtbuf,"BD Visible satellite:%02d",gpsx.beidou_svnum%100);
//可见北斗卫星数
LCD_ShowString(30,260,200,16,16,dtbuf);
sprintf((char *)dtbuf,"UTC Date:%04d/%02d/%02d",gpsx.utc.year,gpsx.utc.month,gpsx.utc.date);
//显示 UTC 日期
//printf("year2:%d\r\n",gpsx.utc.year);
LCD_ShowString(30,280,200,16,16,dtbuf);
sprintf((char *)dtbuf,"UTC Time:%02d:%02d:%02d",gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec); //显示 UTC 时间
LCD_ShowString(30,300,200,16,16,dtbuf);
}
int main(void)
{
u16 i,rxlen;
u16 lenx;
u8 key=0XFF;
u8 upload=0;
Stm32_Clock_Init(9);//设置时钟,168Mhz
delay_init(168); //延时初始化
uart_init(84,115200); //初始化串口波特率为 115200
usart2_init(42,38400); //串口 3 初始化
LED_Init(); //初始化 LED
LCD_Init(); //LCD 初始化
KEY_Init(); //按键初始化
POINT_COLOR=RED;
LCD_ShowString(30,20,200,16,16,"ALIENTEK STM32F4 ^_^");
LCD_ShowString(30,40,200,16,16,"S1216F8-BD TEST");
	LCD_ShowString(30,60,200,16,16,"ATOM@ALIENTEK");
LCD_ShowString(30,80,200,16,16,"KEY0:Upload NMEA Data SW");
LCD_ShowString(30,100,200,16,16,"NMEA Data Upload:OFF");
if(SkyTra_Cfg_Rate(5)!=0)
//设置定位信息更新速度为 5Hz,顺便判断 GPS/北斗模块是否在位.
{
LCD_ShowString(30,120,200,16,16,"SkyTraF8-BD Setting...");
do
{
USART2_Init(36,9600); //初始化串口 3 波特率为 9600
SkyTra_Cfg_Prt(3); //重新设置模块的波特率为 38400
USART2_Init(36,38400); //初始化串口 3 波特率为 38400
key=SkyTra_Cfg_Tp(100000); //脉冲宽度为 100ms
}while(SkyTra_Cfg_Rate(5)!=0&&key!=0);
//配置 SkyTraF8-BD 的更新速率为 5Hz
LCD_ShowString(30,120,200,16,16,"SkyTraF8-BD Set Done!!");
delay_ms(500);
LCD_Fill(30,120,30+200,120+16,WHITE);//清除显示
}
while(1)
{
delay_ms(1);
if(USART2_RX_STA&0X8000) //接收到一次数据了
{
rxlen=USART2_RX_STA&0X7FFF; //得到数据长度
for(i=0;i<rxlen;i++)USART1_TX_BUF[i]=USART2_RX_BUF[i];
USART2_RX_STA=0; //启动下一次接收
USART1_TX_BUF[i]=0; //自动添加结束符
GPS_Analysis(&gpsx,(u8*)USART1_TX_BUF);//分析字符串
Gps_Msg_Show(); //显示信息
if(upload)printf("\r\n%s\r\n",USART1_TX_BUF);
//发送接收到的数据到串口 1
}
key=KEY_Scan(0);
if(key==KEY0_PRES)
{
upload=!upload;
POINT_COLOR=RED;
if(upload)LCD_ShowString(30,100,200,16,16,"NMEA Data Upload:ON ");
else LCD_ShowString(30,100,200,16,16,"NMEA Data Upload:OFF");
}
if((lenx%500)==0)
LED0=!LED0;
lenx++;
}
}