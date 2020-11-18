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
//���� 1,���ͻ�����
nmea_msg gpsx; //GPS ��Ϣ
__align(4) u8 dtbuf[50]; //��ӡ������
const u8*fixmode_tbl[4]={"Fail","Fail"," 2D "," 3D "}; //fix mode �ַ���
//��ʾ GPS/������λ��Ϣ
void Gps_Msg_Show(void)
{
float tp;
POINT_COLOR=BLUE;
tp=gpsx.longitude;
sprintf((char *)dtbuf,"Longitude:%.5f %1c ",tp/=100000,gpsx.ewhemi);
//�õ������ַ���
LCD_ShowString(30,120,200,16,16,dtbuf);
tp=gpsx.latitude;
sprintf((char *)dtbuf,"Latitude:%.5f %1c ",tp/=100000,gpsx.nshemi);
//�õ�γ���ַ���
LCD_ShowString(30,140,200,16,16,dtbuf);
tp=gpsx.altitude;
sprintf((char *)dtbuf,"Altitude:%.1fm ",tp/=10);
//�õ��߶��ַ���
LCD_ShowString(30,160,200,16,16,dtbuf);
tp=gpsx.speed;
	sprintf((char *)dtbuf,"Speed:%.3fkm/h ",tp/=1000);
//�õ��ٶ��ַ���
LCD_ShowString(30,180,200,16,16,dtbuf);
if(gpsx.fixmode<=3)
//��λ״̬
{
sprintf((char *)dtbuf,"Fix Mode:%s",fixmode_tbl[gpsx.fixmode]);
LCD_ShowString(30,200,200,16,16,dtbuf);
}
sprintf((char *)dtbuf,"GPS Valid satellite:%02d",gpsx.posslnum);
//���ڶ�λ�� GPS/����������
LCD_ShowString(30,220,200,16,16,dtbuf);
sprintf((char *)dtbuf,"GPS Visible satellite:%02d",gpsx.svnum%100);
//�ɼ� GPS ������
LCD_ShowString(30,240,200,16,16,dtbuf);
sprintf((char *)dtbuf,"BD Visible satellite:%02d",gpsx.beidou_svnum%100);
//�ɼ�����������
LCD_ShowString(30,260,200,16,16,dtbuf);
sprintf((char *)dtbuf,"UTC Date:%04d/%02d/%02d",gpsx.utc.year,gpsx.utc.month,gpsx.utc.date);
//��ʾ UTC ����
//printf("year2:%d\r\n",gpsx.utc.year);
LCD_ShowString(30,280,200,16,16,dtbuf);
sprintf((char *)dtbuf,"UTC Time:%02d:%02d:%02d",gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec); //��ʾ UTC ʱ��
LCD_ShowString(30,300,200,16,16,dtbuf);
}
int main(void)
{
u16 i,rxlen;
u16 lenx;
u8 key=0XFF;
u8 upload=0;
Stm32_Clock_Init(9);//����ʱ��,168Mhz
delay_init(168); //��ʱ��ʼ��
uart_init(84,115200); //��ʼ�����ڲ�����Ϊ 115200
usart2_init(42,38400); //���� 3 ��ʼ��
LED_Init(); //��ʼ�� LED
LCD_Init(); //LCD ��ʼ��
KEY_Init(); //������ʼ��
POINT_COLOR=RED;
LCD_ShowString(30,20,200,16,16,"ALIENTEK STM32F4 ^_^");
LCD_ShowString(30,40,200,16,16,"S1216F8-BD TEST");
	LCD_ShowString(30,60,200,16,16,"ATOM@ALIENTEK");
LCD_ShowString(30,80,200,16,16,"KEY0:Upload NMEA Data SW");
LCD_ShowString(30,100,200,16,16,"NMEA Data Upload:OFF");
if(SkyTra_Cfg_Rate(5)!=0)
//���ö�λ��Ϣ�����ٶ�Ϊ 5Hz,˳���ж� GPS/����ģ���Ƿ���λ.
{
LCD_ShowString(30,120,200,16,16,"SkyTraF8-BD Setting...");
do
{
USART2_Init(36,9600); //��ʼ������ 3 ������Ϊ 9600
SkyTra_Cfg_Prt(3); //��������ģ��Ĳ�����Ϊ 38400
USART2_Init(36,38400); //��ʼ������ 3 ������Ϊ 38400
key=SkyTra_Cfg_Tp(100000); //������Ϊ 100ms
}while(SkyTra_Cfg_Rate(5)!=0&&key!=0);
//���� SkyTraF8-BD �ĸ�������Ϊ 5Hz
LCD_ShowString(30,120,200,16,16,"SkyTraF8-BD Set Done!!");
delay_ms(500);
LCD_Fill(30,120,30+200,120+16,WHITE);//�����ʾ
}
while(1)
{
delay_ms(1);
if(USART2_RX_STA&0X8000) //���յ�һ��������
{
rxlen=USART2_RX_STA&0X7FFF; //�õ����ݳ���
for(i=0;i<rxlen;i++)USART1_TX_BUF[i]=USART2_RX_BUF[i];
USART2_RX_STA=0; //������һ�ν���
USART1_TX_BUF[i]=0; //�Զ���ӽ�����
GPS_Analysis(&gpsx,(u8*)USART1_TX_BUF);//�����ַ���
Gps_Msg_Show(); //��ʾ��Ϣ
if(upload)printf("\r\n%s\r\n",USART1_TX_BUF);
//���ͽ��յ������ݵ����� 1
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