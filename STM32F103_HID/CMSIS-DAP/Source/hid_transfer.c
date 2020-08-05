/*
 * @Descripttion: 
 * @version: 
 * @Author: Kevincoooool
 * @Date: 2020-07-18 12:35:23
 * @LastEditors: Kevincoooool
 * @LastEditTime: 2020-08-05 10:40:48
 * @FilePath: \TeenyUSB\CMSIS-DAP\Source\hid_transfer.c
 */
/***************************************************************/
#include "DAP_Config.h"
#include "DAP_Common.h"
#include "DAP.h"
#include "hid_transfer.h"
#include "bsp_nrf2401.h"
#include "bsp_spi.h"
#include "teeny_usb.h"
#include "tusbd_user.h"
#include "tusbd_hid.h"
#include "tusbd_cdc.h"
#include "tusbd_msc.h"

#define SIMPLE 1
extern tusb_hid_device_t hid_dev;

#if !SIMPLE
extern tusb_cdc_device_t cdc_dev;
extern uint8_t cdc_buf[32];
static volatile uint8_t USB_RequestFlag; // Request  Buffer Usage Flag

static volatile uint8_t USB_ResponseIdle = 1; // Response Buffer Idle  Flag
static volatile uint8_t USB_ResponseFlag;	  // Response Buffer Usage Flag
uint16_t USB_In_queue_in;					  // Request  Index In
uint16_t USB_In_queue_out;					  // Request  Index Out

uint16_t USB_Out_queue_in;									 // Response Index In
uint16_t USB_Out_queue_out;									 // Response Index Out
uint8_t USB_Request[DAP_PACKET_COUNT][DAP_PACKET_SIZE + 1];	 // Request  Buffer
uint8_t USB_Response[DAP_PACKET_COUNT][DAP_PACKET_SIZE + 1]; // Response Buffer
uint8_t usbd_hid_process(void)
{
	uint32_t n;

	// Process pending requests
	//����յ������ݰ�ҳ��������
	if ((USB_In_queue_out != USB_In_queue_in) || USB_RequestFlag)
	{
		DAP_ProcessCommand(USB_Request[USB_In_queue_out], USB_Response[USB_Out_queue_in]);

		// Update request index and flag
		// ������һ֡���ݾͼ�һ ����������֡��  ��0���¿�����һ֡�Ĵ���
		n = USB_In_queue_out + 1;
		if (n == DAP_PACKET_COUNT)
			n = 0;
		USB_In_queue_out = n;
		//����Ѿ�������������ҳ�յ�������  �Ͳ���Ҫ�ٽ��д�����
		if (USB_In_queue_out == USB_In_queue_in)
			USB_RequestFlag = 0;

		//�����Ҫ�ظ���Ϣ  �ͰѴ���������� USB_Response[USB_Out_queue_in] �ظ���PC
		if (USB_ResponseIdle)
		{ // Request that data is send back to host
			USB_ResponseIdle = 0;

			//USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,USB_Response[USB_Out_queue_in],DAP_PACKET_SIZE);
			tusb_hid_device_send(&hid_dev, USB_Response[USB_Out_queue_out], DAP_PACKET_SIZE);
		}
		//����Ҫ�ظ� ��Ϊ�������ݰ�USB_Out_queue_in ҳ��û������
		else
		{ // �����������ݵ�ҳ��USB_Out_queue_in
			n = USB_Out_queue_in + 1;
			if (n == DAP_PACKET_COUNT)
				n = 0;
			USB_Out_queue_in = n;
			//�����ǰ����������ݰ���ҳ������
			if (USB_Out_queue_in == USB_Out_queue_out)
				USB_ResponseFlag = 1;
		}
		cdc_buf[0] = USB_In_queue_in;
		cdc_buf[1] = USB_In_queue_out;
		cdc_buf[2] = USB_Out_queue_in;
		cdc_buf[3] = USB_Out_queue_out;
		cdc_buf[4] = 0x22;
		tusb_cdc_device_send(&cdc_dev, cdc_buf, 5);
		return 1;
	}
	return 0;
}

void HID_GetOutReport(uint8_t *EpBuf, uint32_t len)
{
	//����յ������ݰ��ĵ�һ�����ݵ��ڴ�����ֹ��־   ��ֱ���˳�
	if (EpBuf[0] == ID_DAP_TransferAbort)
	{
		DAP_TransferAbort = 1;
		return;
	}
	//�����ǰ���ڻظ����ݲ��ҽ������ݰ�װ����   ֱ���˳�  ���ٽ�������
	if (USB_RequestFlag && (USB_In_queue_in == USB_In_queue_out))
		return; // Discard packet when buffer is full

	// ��USB HID��ȡ�������ݷ���������ݰ���
	memcpy(USB_Request[USB_In_queue_in], EpBuf, len);
	// ���½��յ������ݰ�ҳ��  �����������
	USB_In_queue_in++;
	if (USB_In_queue_in == DAP_PACKET_COUNT)
		USB_In_queue_in = 0;
	if (USB_In_queue_in == USB_In_queue_out)
		USB_RequestFlag = 1;
	cdc_buf[0] = USB_In_queue_in;
	cdc_buf[1] = USB_In_queue_out;
	cdc_buf[2] = USB_Out_queue_in;
	cdc_buf[3] = USB_Out_queue_out;
	cdc_buf[4] = 0x11;
	tusb_cdc_device_send(&cdc_dev, cdc_buf, 5);
}

/*
������ɽ���



*/
void HID_SetInReport(void)
{
	//�����ǰ�ظ������ݰ���ҳ�������ڴ���������ݰ���ҳ��  ������Ҫ�ظ� �͸��»ظ�ҳ��
	if ((USB_Out_queue_out != USB_Out_queue_in) || USB_ResponseFlag)
	{

		//���µ�ǰ�ظ������ݰ���ҳ��  ������������ߵ��ڴ������  ������
		USB_Out_queue_out++;
		if (USB_Out_queue_out == DAP_PACKET_COUNT)
			USB_Out_queue_out = 0;
		if (USB_Out_queue_out == USB_Out_queue_in)
			USB_ResponseFlag = 0;
		cdc_buf[0] = USB_In_queue_in;
		cdc_buf[1] = USB_In_queue_out;
		cdc_buf[2] = USB_Out_queue_in;
		cdc_buf[3] = USB_Out_queue_out;
		cdc_buf[4] = 0x55;
		tusb_cdc_device_send(&cdc_dev, cdc_buf, 5);
	}
	else //������ھ���Ҫ�ظ�
	{
		USB_ResponseIdle = 1;
		cdc_buf[0] = USB_In_queue_in;
		cdc_buf[1] = USB_In_queue_out;
		cdc_buf[2] = USB_Out_queue_in;
		cdc_buf[3] = USB_Out_queue_out;
		cdc_buf[4] = 0x33;
		tusb_cdc_device_send(&cdc_dev, cdc_buf, 5);
	}
}

#else
uint8_t MYUSB_Request[DAP_PACKET_SIZE + 1];	 // Request  Buffer
uint8_t MYUSB_Response[DAP_PACKET_SIZE + 1]; // Response Buffer
uint8_t dealing_data = 0;
uint8_t waiting_spi = 0;
uint8_t usbd_hid_process(void)
{
	if (WROK_MODE == 1) //����ģʽ
	{
		//�����Ҫ������
		if (dealing_data)
		{
			DAP_ProcessCommand(MYUSB_Request, MYUSB_Response);
			tusb_hid_device_send(&hid_dev, MYUSB_Response, DAP_PACKET_SIZE);
			dealing_data = 0;

			return 1;
		}
		return 0;
	}
	else if (WROK_MODE == 2) //���߷���� ���յ�HID���ݾ����Ͼ���SPI���� ���յ�SPI����Ҳ����HID����
	{
		//����Ѵ�
		if (dealing_data && (HAL_SPI_GetState(&hspi2) == HAL_SPI_STATE_READY))
		{
			NRF_TxPacket(MYUSB_Request, DAP_PACKET_SIZE);
			waiting_spi = 1;
			dealing_data = 0;
		}
		if (waiting_spi && Nrf_Err_cnt == 0) //SPI�յ�������NRF�ѷ��������� ��SPI�յ�������ͨ��USB HID����
		{
			dealing_data = 1;
			waiting_spi = 0;
			memcpy(MYUSB_Response, NRF24L01_2_RXDATA, DAP_PACKET_SIZE);
			tusb_hid_device_send(&hid_dev, MYUSB_Response, DAP_PACKET_SIZE);
		}
	}
	else if (WROK_MODE == 3) //���߽��ն� ���ܵ�SPI���ݾͽ���DAP����  �������֮��ͨ��SPI����
	{

		if (waiting_spi && Nrf_Err_cnt == 0) //SPI�յ�NRF�����ݲ����Ѵ�NRF��������һ������
		{
			memcpy(MYUSB_Request, NRF24L01_2_RXDATA, DAP_PACKET_SIZE);
			DAP_ProcessCommand(MYUSB_Request, MYUSB_Response);

			dealing_data = 0;
		}
		//��������Ѵ�����ɲ���SPI���ڷ�æ״̬
		if (!dealing_data && (HAL_SPI_GetState(&hspi2) == HAL_SPI_STATE_READY))
		{
			NRF_TxPacket(MYUSB_Response, DAP_PACKET_SIZE);
			waiting_spi = 1;
		}
	}
	return 0;
}
/****************************************************************
 * ��ȡUSB HID����
 ***************************************************************/
void HID_GetOutReport(uint8_t *EpBuf, uint32_t len)
{
	//����յ������ݰ��ĵ�һ�����ݵ��ڴ�����ֹ��־   ��ֱ���˳�
	if (EpBuf[0] == ID_DAP_TransferAbort)
	{
		DAP_TransferAbort = 1;
		return;
	}
	//�����Ҫ��������û���ڴ������ݹ����вŻ���� ��Ȼֱ���˳�
	if (dealing_data)
		return; // Discard packet when buffer is full
	memcpy(MYUSB_Request, EpBuf, len);
	dealing_data = 1;
}

/*
USB HID�������
������ɽ���
*/
void HID_SetInReport(void)
{
}
#endif
