//************������־�Ƽ�***************************************************************

//�뱾������׵�Ӳ����https://item.taobao.com/item.htm?id=530846387509
//ʹ�ù�������ʲô������������ϵ���������߼ӵ���QQ��544645244

//************������־�Ƽ�***************************************************************
#include "sysconfig.h"

uint8_t Mode=1,Fun=0;//Ĭ����βģʽ����ҹ��ģʽ
/******************************************************************************
����ԭ��:	void KEY_Init(void)
��������:	������ʼ��
*******************************************************************************/ 
void KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1;	//MODE����
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;//��������
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;	//FUN����
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;//��������
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}


void key_function(void)
{
	static u8 count0=0,count1=0;
	
	Rc.AUX4 = 0;
	if(!Mode_Read)	Rc.AUX4 |= 0x01;//��߰���
	if(!Fun_Read)		Rc.AUX4 |= 0x02;//�ұ߰���
	
	///////////////////////////////////////////////////////
	if(Rc.AUX4&0x01)//�ұ߰���
	{
		count0++;
		if(count0==200)//����2s
		{
			Check_Ch = 1;
			ANO_LED_0_FLASH();
		}
		if(count0>=250)	count0=250;
	}
	else
	{
		if( count0>=2 && count0<=100 )
		{
			if(Show.hardware_type==1)
			{
				Show.windows++;
				if(Show.windows>2 || Show.test_flag&BIT3) Show.windows=0;
			}
		}
		count0=0;
	}
	///////////////////////////////////////////////////////		
	if(Rc.AUX4&0x02)//��߰���
	{
		count1++;
		if(count1==200)//����2s��ҡ����λУ׼
		{
			ANO_Param.OffSet_En = 1;
			ANO_LED_0_FLASH();
		}
		if(count1>=250)	count1=250;
	}
	else
	{
		if( count1>=2 && count1<=100 ) 
		{

		}
		count1=0;
	}
}
