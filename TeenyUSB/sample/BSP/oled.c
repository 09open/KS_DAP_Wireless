#include "oled.h"
#include "stdlib.h"
#include "oledfont.h"
#include "gpio.h"
#include "show.h"
uint8_t OLED_GRAM[128][8];
void OLED_Refresh_Gram(void)
{
	uint8_t i, n;
	for (i = 0; i < 8; i++)
	{
		OLED_WR_Byte(0xb0 + i, OLED_CMD); //����ҳ��ַ��0~7��
		OLED_WR_Byte(0x00, OLED_CMD);	  //������ʾλ�á��е͵�ַ
		OLED_WR_Byte(0x10, OLED_CMD);	  //������ʾλ�á��иߵ�ַ
		for (n = 0; n < 128; n++)
			OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
	}
}

//��OLEDд��һ���ֽڡ�
//dat:Ҫд�������/����
//cmd:����/�����־ 0,��ʾ����;1,��ʾ����;
void OLED_WR_Byte(uint8_t dat, uint8_t cmd)
{
	uint8_t i;
	if (cmd)
		OLED_DC_Set();
	else
		OLED_DC_Clr();
	for (i = 0; i < 8; i++)
	{
		OLED_SCLK_Clr();
		if (dat & 0x80)
			OLED_SDIN_Set();
		else
			OLED_SDIN_Clr();
		OLED_SCLK_Set();
		dat <<= 1;
	}
	OLED_DC_Set();
}

//����OLED��ʾ
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D, OLED_CMD); //SET DCDC����
	OLED_WR_Byte(0X14, OLED_CMD); //DCDC ON
	OLED_WR_Byte(0XAF, OLED_CMD); //DISPLAY ON
}

//�ر�OLED��ʾ
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D, OLED_CMD); //SET DCDC����
	OLED_WR_Byte(0X10, OLED_CMD); //DCDC OFF
	OLED_WR_Byte(0XAE, OLED_CMD); //DISPLAY OFF
}
//��������,������,������Ļ�Ǻ�ɫ��!��û����һ��!!!
void OLED_Clear(void)
{
	uint8_t i, n;
	for (i = 0; i < 8; i++)
	{
		for (n = 0; n < 128; n++)
		{
			OLED_GRAM[n][i] = 0X00;
		}
	}
	OLED_Refresh_Gram(); //������ʾ
}
//����
//x:0~127
//y:0~63
//t:1 ��� 0,���
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t)
{
	uint8_t pos, bx, temp = 0;
	if (x > 127 || y > 63)
		return; //������Χ��.
	pos = 7 - y / 8;
	bx = y % 8;
	temp = 1 << (7 - bx);
	if (t)
		OLED_GRAM[x][pos] |= temp;
	else
		OLED_GRAM[x][pos] &= ~temp;
}

//��ָ��λ����ʾһ���ַ�,���������ַ�
//x:0~127
//y:0~63
//mode:0,������ʾ;1,������ʾ
//size:ѡ������ 16/12
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode)
{
	uint8_t temp, t, t1;
	uint8_t y0 = y;
	chr = chr - ' '; //�õ�ƫ�ƺ��ֵ
	for (t = 0; t < size; t++)
	{
		if (size == 8)
			temp = oled_asc2_0806[chr][t]; //����1206����
		else if (size == 12)
			temp = oled_asc2_1206[chr][t]; //����1206����
		else
			temp = oled_asc2_1608[chr][t]; //����1608����
		for (t1 = 0; t1 < 8; t1++)
		{
			if (temp & 0x80)
				OLED_DrawPoint(x, y, mode);
			else
				OLED_DrawPoint(x, y, !mode);
			temp <<= 1;
			y++;
			if ((y - y0) == size)
			{
				y = y0;
				x++;
				break;
			}
		}
	}
	OLED_Refresh_Gram();
}
//��ʾ����
void OLED_ShowCH(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode)
{
	uint8_t temp, t, t1;
	uint8_t y0 = y;

	for (t = 0; t < size; t++)
	{
		temp = oled_CH_1616[chr][t]; //���������ֿ�
		for (t1 = 0; t1 < 8; t1++)
		{
			if (temp & 0x80)
				OLED_DrawPoint(x, y, mode);
			else
				OLED_DrawPoint(x, y, !mode);
			temp <<= 1;
			y++;
			if ((y - y0) == size)
			{
				y = y0;
				x++;
				break;
			}
		}
	}
	OLED_Refresh_Gram();
}
void OLED_Set_Pos(unsigned char x, unsigned char y)
{
	OLED_WR_Byte(0xb0 + y, OLED_CMD);
	OLED_WR_Byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);
	OLED_WR_Byte((x & 0x0f) | 0x01, OLED_CMD);
}
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[])
{
	unsigned int j = 0;
	unsigned char x, y;

	if (y1 % 8 == 0)
		y = y1 / 8;
	else
		y = y1 / 8 + 1;
	for (y = y0; y < y1; y++)
	{
		//		OLED_Set_Pos(x0, y);
		for (x = x0; x < x1; x++)
		{
			OLED_WR_Byte(BMP[j++], OLED_DATA);
		}
	}
	OLED_Refresh_Gram();
}
//��ʾ���������ַ�
void OLED_Show_CH(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode)
{
	OLED_ShowCH(x, y, chr * 2, size, 1);
	OLED_ShowCH(x + size / 2, y, chr * 2 + 1, size, 1);
}

//��ʾһ�������ַ�
void OLED_Show_CH_String(uint8_t x, uint8_t y, const uint8_t *p, uint8_t size, uint8_t mode)
{
	uint8_t temp, t, t1;
	uint8_t y0 = y;

	for (t = 0; t < size; t++)
	{
		temp = p[t]; //���������ֿ�
		for (t1 = 0; t1 < 8; t1++)
		{
			if (temp & 0x80)
				OLED_DrawPoint(x, y, mode);
			else
				OLED_DrawPoint(x, y, !mode);
			temp <<= 1;
			y++;
			if ((y - y0) == size)
			{
				y = y0;
				x++;
				break;
			}
		}
	}
	OLED_Refresh_Gram();
}
//m^n����
uint32_t oled_pow(uint8_t m, uint8_t n)
{
	uint32_t result = 1;
	while (n--)
		result *= m;
	return result;
}
//��ʾ2������
//x,y :�������
//len :���ֵ�λ��
//size:�����С
//mode:ģʽ	0,���ģʽ;1,����ģʽ
//num:��ֵ(0~4294967295);
void OLED_ShowNumber(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size)
{
	uint8_t t, temp;
	uint8_t enshow = 0;
	for (t = 0; t < len; t++)
	{
		temp = (num / oled_pow(10, len - t - 1)) % 10;
		if (enshow == 0 && t < (len - 1))
		{
			if (temp == 0)
			{
				OLED_ShowChar(x + (size / 2) * t, y, ' ', size, 1);
				continue;
			}
			else
				enshow = 1;
		}
		OLED_ShowChar(x + (size / 2) * t, y, temp + '0', size, 1);
	}
	OLED_Refresh_Gram();
}
//��ʾ�ַ���
//x,y:�������
//*p:�ַ�����ʼ��ַ
//��16����
void OLED_ShowString(uint8_t x, uint8_t y, const uint8_t *p, uint8_t size, uint8_t mode)
{
#define MAX_CHAR_POSX 122
#define MAX_CHAR_POSY 58
	while (*p != '\0')
	{
		if (x > MAX_CHAR_POSX)
		{
			x = 0;
			y += 16;
		} //����
		if (y > MAX_CHAR_POSY)
		{
			y = x = 0;
			OLED_Clear();
		}
		OLED_ShowChar(x, y, *p, size, mode);
		x += 8;
		p++;
	}
	OLED_Refresh_Gram();
}
//��������ʾ����
void OLED_Show_progress_bar(uint8_t temp, uint8_t chr_star, uint8_t chr_default, uint8_t x, uint8_t y, uint8_t size, uint8_t mode)
{
	switch (temp)
	{
	case 0:
		OLED_Show_CH(x, y, chr_star + temp, size, size);
		break;
	case 1:
		OLED_Show_CH(x, y, chr_star + temp, size, size);
		break;
	case 2:
		OLED_Show_CH(x, y, chr_star + temp, size, size);
		break;
	case 3:
		OLED_Show_CH(x, y, chr_star + temp, size, size);
		break;
	case 4:
		OLED_Show_CH(x, y, chr_star + temp, size, size);
		break;
	case 5:
		OLED_Show_CH(x, y, chr_star + temp, size, size);
		break;
	case 6:
		OLED_Show_CH(x, y, chr_star + temp, size, size);
		break;
	case 7:
		OLED_Show_CH(x, y, chr_star + temp, size, size);
		break;
	case 8:
		OLED_Show_CH(x, y, chr_star + temp, size, size);
		break;
	case 9:
		OLED_Show_CH(x, y, chr_star + temp, size, size);
		break;
	case 10:
		OLED_Show_CH(x, y, chr_star + temp, size, size);
		break;
	case 11:
		OLED_Show_CH(x, y, chr_star + temp, size, size);
		break;
	case 12:
		OLED_Show_CH(x, y, chr_star + temp, size, size);
		break;

	default:
		OLED_Show_CH(x, y, chr_default, size, size);
		break;
	}
	OLED_Refresh_Gram();
}
//��ʾ�ַ���
//x,y:�������
//*p:�ַ�����ʼ��ַ
//��16����
void OLED_ShowString_16(uint8_t x, uint8_t y, const uint8_t *p)
{
#define MAX_CHAR_POSX 122
#define MAX_CHAR_POSY 58
	while (*p != '\0')
	{
		if (x > MAX_CHAR_POSX)
		{
			x = 0;
			y += 16;
		} //����
		if (y > MAX_CHAR_POSY)
		{
			y = x = 0;
			OLED_Clear();
		}
		OLED_ShowChar(x, y, *p, 16, 1);
		x += 8;
		p++;
	}
}
void Delay(uint32_t nCount)
{
	for (; nCount != 0; nCount--)
		;
}
//��ʼ��SSD1306
void OLED_Init(void)
{
//	__HAL_RCC_AFIO_CLK_ENABLE();
//	__HAL_AFIO_REMAP_SWJ_NOJTAG(); //����JTAG
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5, GPIO_PIN_SET);

	OLED_RST_Set();
	HAL_Delay(10);
	OLED_RST_Clr();
	HAL_Delay(10);
	OLED_RST_Set();
	OLED_CS_L;
	OLED_WR_Byte(0xAE, OLED_CMD); //�ر���ʾ
	OLED_WR_Byte(0xD5, OLED_CMD); //����ʱ�ӷ�Ƶ����,��Ƶ��
	OLED_WR_Byte(80, OLED_CMD);	  //[3:0],��Ƶ����;[7:4],��Ƶ��
	OLED_WR_Byte(0xA8, OLED_CMD); //��������·��
	OLED_WR_Byte(0X3F, OLED_CMD); //Ĭ��0X3F(1/64)
	OLED_WR_Byte(0xD3, OLED_CMD); //������ʾƫ��
	OLED_WR_Byte(0X00, OLED_CMD); //Ĭ��Ϊ0

	OLED_WR_Byte(0x40, OLED_CMD); //������ʾ��ʼ�� [5:0],����.

	OLED_WR_Byte(0x8D, OLED_CMD); //��ɱ�����
	OLED_WR_Byte(0x14, OLED_CMD); //bit2������/�ر�
	OLED_WR_Byte(0x20, OLED_CMD); //�����ڴ��ַģʽ
	OLED_WR_Byte(0x02, OLED_CMD); //[1:0],00���е�ַģʽ;01���е�ַģʽ;10,ҳ��ַģʽ;Ĭ��10;
	OLED_WR_Byte(0xA1, OLED_CMD); //���ض�������,bit0:0,0->0;1,0->127;
	OLED_WR_Byte(0xC0, OLED_CMD); //����COMɨ�跽��;bit3:0,��ͨģʽ;1,�ض���ģʽ COM[N-1]->COM0;N:����·��
	OLED_WR_Byte(0xDA, OLED_CMD); //����COMӲ����������
	OLED_WR_Byte(0x12, OLED_CMD); //[5:4]����

	OLED_WR_Byte(0x81, OLED_CMD); //�Աȶ�����
	OLED_WR_Byte(0xEF, OLED_CMD); //1~255;Ĭ��0X7F (��������,Խ��Խ��)
	OLED_WR_Byte(0xD9, OLED_CMD); //����Ԥ�������
	OLED_WR_Byte(0xf1, OLED_CMD); //[3:0],PHASE 1;[7:4],PHASE 2;
	OLED_WR_Byte(0xDB, OLED_CMD); //����VCOMH ��ѹ����
	OLED_WR_Byte(0x30, OLED_CMD); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	OLED_WR_Byte(0xA4, OLED_CMD); //ȫ����ʾ����;bit0:1,����;0,�ر�;(����/����)
	OLED_WR_Byte(0xA6, OLED_CMD); //������ʾ��ʽ;bit0:1,������ʾ;0,������ʾ
	OLED_WR_Byte(0xAF, OLED_CMD); //������ʾ
//OLED_WR_Byte(0xAE, OLED_CMD); //�ر���ʾ

//	OLED_WR_Byte(0x40, OLED_CMD); //---set low column address
//	OLED_WR_Byte(0xB0, OLED_CMD); //---set high column address

//	OLED_WR_Byte(0xC8, OLED_CMD); //-not offset

//	OLED_WR_Byte(0x81, OLED_CMD); //���öԱȶ�
//	OLED_WR_Byte(0xff, OLED_CMD);

//	OLED_WR_Byte(0xa1, OLED_CMD); //���ض�������

//	OLED_WR_Byte(0xa6, OLED_CMD); //

//	OLED_WR_Byte(0xa8, OLED_CMD); //��������·��
//	OLED_WR_Byte(0x1f, OLED_CMD);

//	OLED_WR_Byte(0xd3, OLED_CMD);
//	OLED_WR_Byte(0x00, OLED_CMD);

//	OLED_WR_Byte(0xd5, OLED_CMD);
//	OLED_WR_Byte(0xf0, OLED_CMD);

//	OLED_WR_Byte(0xd9, OLED_CMD);
//	OLED_WR_Byte(0x22, OLED_CMD);

//	OLED_WR_Byte(0xda, OLED_CMD);
//	OLED_WR_Byte(0x02, OLED_CMD);

//	OLED_WR_Byte(0xdb, OLED_CMD);
//	OLED_WR_Byte(0x49, OLED_CMD);

//	OLED_WR_Byte(0x8d, OLED_CMD);
//	OLED_WR_Byte(0x14, OLED_CMD);

//	OLED_WR_Byte(0xaf, OLED_CMD);

	OLED_Display_On();
	OLED_Clear();
}
