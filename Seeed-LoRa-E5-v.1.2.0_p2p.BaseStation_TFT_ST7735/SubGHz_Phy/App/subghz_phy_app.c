/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    subghz_phy_app.c
  * @author  MCD Application Team
  * @brief   Application of the SubGHz_Phy Middleware
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "sys_app.h"
#include "subghz_phy_app.h"
#include "radio.h"

/* USER CODE BEGIN Includes */

#include "stm32_timer.h"
#include "stm32_seq.h"
#include "utilities_def.h"
#include "app_version.h"
#include "adc_if.h"
#include <stdlib.h>
#include "ST7735.h"
#include "spi.h"
#include "stdio.h"
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */

typedef enum
{
  RX_DONE,
  RX_TO,
  RX_ERR,
  TX_START,
  TX_DONE,
  RX_START,
  TX_TO,
} States_t;

char Line[14][24];

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Radio events function pointer */
static RadioEvents_t RadioEvents;
/* USER CODE BEGIN PV */

static uint16_t BufferSize = BUFFER_SIZE;
static uint8_t Buffer[BUFFER_SIZE];

static States_t State = RX_START;

static int8_t RssiValue = 0;
static int8_t SnrValue = 0;
static  UTIL_TIMER_Object_t timerOneSecond;
static uint32_t WatchDogRx = WATCHDOG_RX_PERIOD;

bool isChannelFree = true;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/*!
 * @brief Function to be executed on Radio Tx Done event
 */
static void OnTxDone(void);

/**
  * @brief Function to be executed on Radio Rx Done event
  * @param  payload ptr of buffer received
  * @param  size buffer size
  * @param  rssi
  * @param  LoraSnr_FskCfo
  */
static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);

/**
  * @brief Function executed on Radio Tx Timeout event
  */
static void OnTxTimeout(void);

/**
  * @brief Function executed on Radio Rx Timeout event
  */
static void OnRxTimeout(void);

/**
  * @brief Function executed on Radio Rx Error event
  */
static void OnRxError(void);

/* USER CODE BEGIN PFP */

static void OnOneSecondElapsedEvent(void *context);
static void BaseStation_Process(void);
static uint32_t powInt(uint32_t base, uint32_t exp);

/* USER CODE END PFP */

/* Exported functions ---------------------------------------------------------*/
void SubghzApp_Init(void)
{
  /* USER CODE BEGIN SubghzApp_Init_1 */

	  UTIL_TIMER_Create(&timerOneSecond,0xFFFFFFFF,UTIL_TIMER_PERIODIC,OnOneSecondElapsedEvent,NULL);
	  UTIL_TIMER_SetPeriod(&timerOneSecond,1000);
	  UTIL_TIMER_Start(&timerOneSecond);
	  SYS_InitMeasurement();

  /* USER CODE END SubghzApp_Init_1 */

  /* Radio initialization */
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;

  Radio.Init(&RadioEvents);

  /* USER CODE BEGIN SubghzApp_Init_2 */

  srand(Radio.Random());

      #if (( USE_MODEM_LORA == 1 ) && ( USE_MODEM_FSK == 0 ))
        Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                          LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                          LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                          true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);

        Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                          LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                          LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                          0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

        Radio.SetMaxPayloadLength(MODEM_LORA, BUFFER_SIZE);

      #elif (( USE_MODEM_LORA == 0 ) && ( USE_MODEM_FSK == 1 ))

        Radio.SetTxConfig(MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                          FSK_DATARATE, 0,
                          FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                          true, 0, 0, 0, TX_TIMEOUT_VALUE);

        Radio.SetRxConfig(MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                          0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                          0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                          0, 0, false, true);

        Radio.SetMaxPayloadLength(MODEM_FSK, BUFFER_SIZE);

      #else
      #error "Please define a frequency band in the sys_conf.h file."
      #endif /* USE_MODEM_LORA | USE_MODEM_FSK */

        Radio.SetChannel(RF_FREQUENCY);

        Radio.Rx(RX_TIMEOUT_VALUE);

        UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), 0, BaseStation_Process);

        APP_LOG(TS_OFF, VLEVEL_L, "\r\nBase Station start -> ");
        APP_LOG(TS_OFF, VLEVEL_L, "RF=%uMHz , SF=%u",RF_FREQUENCY,LORA_SPREADING_FACTOR);
        APP_LOG(TS_OFF,VLEVEL_L," , CS=%ddBm",RF_CHANNEL_FREE_RSSI_TRESHOLD);
        APP_LOG(TS_OFF, VLEVEL_L, "\r\n");

  /* USER CODE END SubghzApp_Init_2 */
}

/* USER CODE BEGIN EF */


/* USER CODE END EF */

/* Private functions ---------------------------------------------------------*/
static void OnTxDone(void)
{
  /* USER CODE BEGIN OnTxDone */
	State = TX_DONE;
	UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), CFG_SEQ_Prio_0);
  /* USER CODE END OnTxDone */
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
  /* USER CODE BEGIN OnRxDone */

	BufferSize = size;
	memcpy(Buffer, payload, BufferSize);
	RssiValue = rssi;
	SnrValue = LoraSnr_FskCfo;
	State = RX_DONE;
	UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), CFG_SEQ_Prio_0);

  /* USER CODE END OnRxDone */
}

static void OnTxTimeout(void)
{
  /* USER CODE BEGIN OnTxTimeout */
	State = TX_TO;
	UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), CFG_SEQ_Prio_0);
  /* USER CODE END OnTxTimeout */
}

static void OnRxTimeout(void)
{
  /* USER CODE BEGIN OnRxTimeout */
	 State = RX_TO;
	 UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), CFG_SEQ_Prio_0);
  /* USER CODE END OnRxTimeout */
}

static void OnRxError(void)
{
  /* USER CODE BEGIN OnRxError */
	State = RX_ERR;
	UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), CFG_SEQ_Prio_0);
  /* USER CODE END OnRxError */
}

/* USER CODE BEGIN PrFD */

static void OnOneSecondElapsedEvent(void *context)
{
  if (WatchDogRx>0)
  {
    WatchDogRx--;
  }
  else
  {
	NVIC_SystemReset();
  }
}


static uint32_t powInt(uint32_t base, uint32_t exp)
{
    uint32_t result = 1;
    while(exp) { result *= base; exp--; }
    return result;
}


static void BaseStation_Process(void)
{
	int16_t i;
	bool isChannelFree = true;
	uint32_t backoffTime,carrierSenseTime;
	int16_t rssi;

	  switch (State)
	  {
	    case RX_START:
	      Radio.SetChannel(RF_FREQUENCY);
		  Radio.Rx(RX_TIMEOUT_VALUE);
		  APP_LOG(TS_ON,VLEVEL_L, "RX...\r\n\r\n");
		  sprintf(&Line[10][0], "%lds RX...", UTIL_TIMER_GetCurrentTime()/1000);
		  break;

		case RX_DONE:
		  WatchDogRx = WATCHDOG_RX_PERIOD;

		    APP_LOG(TS_ON,VLEVEL_L,"\r\nNode ID: %02X | ",Buffer[0]);
			APP_LOG(TS_OFF,VLEVEL_L,"RX data: TS: %dC , BAT: %d",(int8_t)(Buffer[1]),Buffer[2]);
			APP_LOG(TS_OFF,VLEVEL_L," , FSentCnt: %u , FAckCnt:b%u\r\n",(uint16_t)(Buffer[3]<<8)+Buffer[4],(uint16_t)(Buffer[5]<<8)+Buffer[6]);
			APP_LOG(TS_OFF,VLEVEL_L,"RSSI: %ddBm\r\n",RssiValue);
			APP_LOG(TS_OFF,VLEVEL_L,"SNR: %ddB\r\n\r\n",SnrValue);
			APP_LOG(TS_OFF,VLEVEL_L,"DISPLAY DATA:---------------------\r\n");
			sprintf(&Line[0][0], "Node ID: %02X  RX data: ", Buffer[0]);
			APP_LOG(TS_OFF,VLEVEL_L,"%s\r\n",&Line[0][0]);
			sprintf(&Line[1][0], "CPUTemp: %dC, V:%d", (int8_t)(Buffer[1]),Buffer[2]);
			APP_LOG(TS_OFF,VLEVEL_L,"%s\r\n",&Line[1][0]);
			sprintf(&Line[2][0], "ExtTemp: %d.%dC", ((int16_t)(Buffer[3]<<8) | (int16_t)Buffer[4])/10, abs(((int16_t)(Buffer[3]<<8) | (int16_t)Buffer[4]))%10);
			APP_LOG(TS_OFF,VLEVEL_L,"%s\r\n",&Line[2][0]);
			sprintf(&Line[3][0], "Hum: %d.%d", ((uint16_t)(Buffer[5]<<8)+Buffer[6])/10, ((uint16_t)(Buffer[5]<<8)+Buffer[6])%10);
			APP_LOG(TS_OFF,VLEVEL_L,"%s\r\n",&Line[3][0]);
			sprintf(&Line[4][0], "FrameSnt: %u", (uint16_t)(Buffer[7]<<8)+Buffer[8]);
			APP_LOG(TS_OFF,VLEVEL_L,"%s\r\n",&Line[4][0]);
			sprintf(&Line[5][0], "FrameAck: %u", (uint16_t)(Buffer[9]<<8)+Buffer[10]);
			APP_LOG(TS_OFF,VLEVEL_L,"%s\r\n",&Line[5][0]);
			sprintf(&Line[6][0], "MCUTime: %lds", (uint32_t)(Buffer[11]<<24) | (uint32_t)(Buffer[12]<<16) | (uint32_t)(Buffer[13]<<8) | (uint32_t) Buffer[14]);
			APP_LOG(TS_OFF,VLEVEL_L,"%s\r\n",&Line[6][0]);
			sprintf(&Line[7][0], "RSSI: %ddBm", RssiValue);
			APP_LOG(TS_OFF,VLEVEL_L,"%s\r\n",&Line[7][0]);
			sprintf(&Line[8][0], "SNR: %ddB", SnrValue);
			APP_LOG(TS_OFF,VLEVEL_L,"%s\r\n",&Line[8][0]);
			APP_LOG(TS_OFF,VLEVEL_L,"----------------------------------\r\n\r\n");

		  State = TX_START;
		  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), CFG_SEQ_Prio_0);
		  break;

		case RX_TO:
		  //APP_LOG(TS_ON, VLEVEL_L, "RX timeout\r\n\r\n");
		  State = RX_START;
		  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), CFG_SEQ_Prio_0);
		  break;

		case RX_ERR:

		  APP_LOG(TS_ON, VLEVEL_L, "RX error\r\n\r\n");
		  State = RX_START;
		  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), CFG_SEQ_Prio_0);
		  break;

		case TX_START:
		  i = 0;
		  Buffer[i++] = 'A';
		  Buffer[i++] = 'C';
		  Buffer[i++] = 'K';
		  BufferSize = i;
		  /* RF collisions avoidance and TX section */
		  Radio.SetChannel(RF_FREQUENCY);
		  Radio.Rx(0);
		  HAL_Delay(Radio.GetWakeupTime());
		  for (i=0;i<RF_CHANNEL_FREE_TRIALS_MAX;i++)
		  {
		  //APP_LOG(TS_ON,VLEVEL_L,"RF Channel Sensing #%u ...",i+1);
		  isChannelFree = true;
		  carrierSenseTime = UTIL_TIMER_GetCurrentTime();
		  while( UTIL_TIMER_GetElapsedTime(carrierSenseTime) < RSSI_SENSING_TIME)
		  {
		    rssi = Radio.Rssi(MODEM_LORA);
			if (rssi > RF_CHANNEL_FREE_RSSI_TRESHOLD) { isChannelFree = false; break; }
		  }
		  carrierSenseTime = UTIL_TIMER_GetElapsedTime(carrierSenseTime);
		  //APP_LOG(TS_OFF,VLEVEL_L," CS=%ddBm , CStime=%ums\r\n",rssi,carrierSenseTime);
		  if (isChannelFree)
		  {
		    break; //RF collisions avoidance loop
		  }
		  else
		  {
		    if (i<RF_CHANNEL_FREE_TRIALS_MAX-1)
			{
			  backoffTime = CS_BACKOFF_TIME_UNIT * (1 + (rand() % powInt(2,i+1)));
			  APP_LOG(TS_ON,VLEVEL_L,"RF channel is busy, next attempt after %ums...\r\n",backoffTime);
			  sprintf(&Line[9][0], "Busy, next %ldms...\r\n",backoffTime);
			  //APP_LOG(TS_OFF,VLEVEL_L,"%s\r\n",&Line[7][0]);
			  HAL_Delay(backoffTime);
			}
		  }
		}
	    /* TX data over the air */
	   if (isChannelFree)
	   {
	     Radio.SetChannel(RF_FREQUENCY);
		 HAL_Delay(Radio.GetWakeupTime() + TCXO_WORKAROUND_TIME_MARGIN);
		 Radio.Send(Buffer,BufferSize);
		 //APP_LOG(TS_ON,VLEVEL_L,"TX ACK\r\n");
		}
	    else
	    {


		  APP_LOG(TS_ON, VLEVEL_L, "RF channel: %uHz is BUSY\r\n\r\n",RF_FREQUENCY);
		}
		break;

		case TX_DONE:

		  State = RX_START;
		  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), CFG_SEQ_Prio_0);
		  break;

		case TX_TO:

		  APP_LOG(TS_ON, VLEVEL_L, "TX timeout\r\n");
		  State = RX_START;
		  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), CFG_SEQ_Prio_0);
		  break;

		default:
		  break;
	  }

	  ST7735_SetRotation(1);
	  for (uint8_t k = 0; k < 11; k++){
		  //ST7735_WriteString(0, k*9, Line[k], Font_6x8, GREEN,BLACK);
		  ST7735_WriteString(0, k*11, &Line[k][0], Font_7x10, GREEN,BLACK);
	  }



}


/* USER CODE END PrFD */
