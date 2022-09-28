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

#include "stm32_systime.h"
#include "stm32_timer.h"
#include "stm32_seq.h"
#include "utilities_def.h"
#include "app_version.h"
#include "adc_if.h"
#include "aht20.h"
#include <stdlib.h>

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

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

States_t State = TX_START;

uint16_t FrameSentCnt = 0;
uint16_t FrameAckCnt = 0;

int8_t RssiValue = 0;
int8_t SnrValue = 0;
static  UTIL_TIMER_Object_t timerTx;

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

static void OnTimerTxEvent(void *context);
static void Sensor_Process(void);
static uint32_t powInt(uint32_t base, uint32_t exp);

/* USER CODE END PFP */

/* Exported functions ---------------------------------------------------------*/
void SubghzApp_Init(void)
{
  /* USER CODE BEGIN SubghzApp_Init_1 */

	UTIL_TIMER_Create(&timerTx,0xFFFFFFFF,UTIL_TIMER_ONESHOT,OnTimerTxEvent,NULL);
	UTIL_TIMER_SetPeriod(&timerTx,TX_PERIOD_MS);
	// UTIL_TIMER_Start(&timerTx);

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

      UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), 0, Sensor_Process);

      OnTimerTxEvent(NULL);

      APP_LOG(TS_OFF, VLEVEL_L, "\r\nSensor start -> ");
      APP_LOG(TS_OFF, VLEVEL_L," ID=%02X", NODE_ID);
      APP_LOG(TS_OFF, VLEVEL_L, " , RF=%uMHz , SF=%u", RF_FREQUENCY, LORA_SPREADING_FACTOR);
      APP_LOG(TS_OFF, VLEVEL_L," , CS=%ddBm", RF_CHANNEL_FREE_RSSI_TRESHOLD);
      APP_LOG(TS_OFF, VLEVEL_L, "\r\n");

  /* USER CODE END SubghzApp_Init_2 */
}

/* USER CODE BEGIN EF */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch (GPIO_Pin)
  {
    case  BUT1_Pin:

    	State = TX_START;
  		UTIL_TIMER_SetPeriod(&timerTx,TX_PERIOD_MS);
    	UTIL_TIMER_Start(&timerTx);
    	UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), CFG_SEQ_Prio_0);

      break;

    default:
      break;
  }
}

/* USER CODE END EF */

/* Private functions ---------------------------------------------------------*/
static void OnTxDone(void)
{
  /* USER CODE BEGIN OnTxDone */
	State = RX_START;
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

static void OnTimerTxEvent(void *context)
{
  State = TX_START;
  UTIL_TIMER_Start(&timerTx);


  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_SubGHz_Phy_App_Process), CFG_SEQ_Prio_0);
}

static uint32_t powInt(uint32_t base, uint32_t exp)
{
    uint32_t result = 1;
    while(exp) { result *= base; exp--; }
    return result;
}


static void Sensor_Process(void)
{
  uint32_t i,backoffTime,carrierSenseTime;
  int16_t rssi;
  bool isChannelFree = true;
  float aht20_hum = DEFAULT_HUMIDITY;
  float aht20_temp = DEFAULT_TEMPERATURE;
  SysTime_t curtime = SysTimeGet();
  if (initAHT20() == 1)	{
	  readAHT20(&aht20_hum, &aht20_temp);
  }
  int16_t temperature = (int16_t) (aht20_temp * 10);
  int16_t humidity = (int16_t) (aht20_hum * 10);

  switch (State)
  {
    case TX_START:
	  i = 0;
	  Buffer[i++] = NODE_ID;
	  Buffer[i++] = (uint8_t)(GetTemperatureLevel());
	  Buffer[i++] = GetBatteryLevel();
	  Buffer[i++] = ((uint8_t) ((temperature >> 8) & 0xFF));					// external sensor temperature MSB
	  Buffer[i++] = ((uint8_t) (temperature & 0xFF));							// external sensor temperature LSB
	  APP_LOG(TS_OFF, VLEVEL_L, "ExternalTemp: %d.%d'C\r\n", temperature/10, temperature%10);
	  Buffer[i++] = ((uint8_t) ((humidity >> 8) & 0xFF));					// external sensor humidity MSB
	  Buffer[i++] = ((uint8_t) (humidity & 0xFF));							// external sensor humidity LSB
	  APP_LOG(TS_OFF, VLEVEL_L, "ExternalHum: %d.%d%\r\n", humidity/10, humidity%10);
	  Buffer[i++] = (FrameSentCnt>>8) & 0xFF;
	  Buffer[i++] = FrameSentCnt & 0xFF;
	  APP_LOG(TS_OFF, VLEVEL_L, "FrameSentCnt: %d\r\n", FrameSentCnt);
	  Buffer[i++] = (FrameAckCnt>>8) & 0xFF;
	  Buffer[i++] = FrameAckCnt & 0xFF;
	  APP_LOG(TS_OFF, VLEVEL_L, "FrameAckCnt: %d\r\n", FrameAckCnt);
	  Buffer[i++] = (uint8_t) ((curtime.Seconds >> 24) & 0xFF);				// mcu timestamp MSB, uint32_t
	  Buffer[i++] = (uint8_t) ((curtime.Seconds >> 16) & 0xFF);
	  Buffer[i++] = (uint8_t) ((curtime.Seconds >> 8) & 0xFF);
	  Buffer[i++] = (uint8_t) (curtime.Seconds & 0xFF);
	  APP_LOG(TS_OFF, VLEVEL_L, "MCUTime: %d\r\n", curtime.Seconds);
	  BufferSize = i;

	  /* RF collisions avoidance and TX section */
	  Radio.SetChannel(RF_FREQUENCY);
	  Radio.Rx(0);
	  HAL_Delay(Radio.GetWakeupTime());
	  for ( uint8_t j = 0 ; j < RF_CHANNEL_FREE_TRIALS_MAX ; j++ )
	  {
	    APP_LOG(TS_ON,VLEVEL_L,"RF Channel Sensing #%u ...\r\n", j+1);
		isChannelFree = true;
		carrierSenseTime = UTIL_TIMER_GetCurrentTime();
		while( UTIL_TIMER_GetElapsedTime(carrierSenseTime) < RSSI_SENSING_TIME)
		{
		  rssi = Radio.Rssi(MODEM_LORA);
		  if (rssi > RF_CHANNEL_FREE_RSSI_TRESHOLD) { isChannelFree = false; break; }
		}
		carrierSenseTime = UTIL_TIMER_GetElapsedTime(carrierSenseTime);
		//APP_LOG(TS_OFF, VLEVEL_L, " CS: %ddBm , CS time: %ums\r\n", rssi, carrierSenseTime);
		if (isChannelFree)
		{
		  break; //RF collisions avoidance loop
		}
		else
		{
		  if (j < RF_CHANNEL_FREE_TRIALS_MAX-1)
		  {
		    backoffTime = CS_BACKOFF_TIME_UNIT * (1 + (rand() % powInt(2,j+1)));
		    APP_LOG(TS_ON, VLEVEL_L, "RF channel is busy, next attempt after %ums...\r\n", backoffTime);
		    HAL_Delay(backoffTime);
		  }
		}
      }

	  /* TX data over the air */
	  if (isChannelFree)
	  {
	    Radio.SetChannel(RF_FREQUENCY);
	    HAL_Delay(Radio.GetWakeupTime() + TCXO_WORKAROUND_TIME_MARGIN);
	    Radio.Send(Buffer, BufferSize);  // to be filled by attendee
	    APP_LOG(TS_ON, VLEVEL_L, "TX...\r\n");
	  }
	  else
	  {

		APP_LOG(TS_ON, VLEVEL_L, "RF channel: %uHz is BUSY\r\n\r\n",RF_FREQUENCY);
	  }
	  break;  //case

	case RX_START:
	  FrameSentCnt++;
	  Radio.SetChannel(RF_FREQUENCY);
	  Radio.Rx(RX_TIMEOUT_VALUE);
	  APP_LOG(TS_ON, VLEVEL_L, "RX...\r\n");
	  break;

	case RX_DONE:
	  Radio.Sleep();

	  APP_LOG(TS_ON,VLEVEL_L,"RX hex: ");
	  for( uint16_t i = 0 ; i < BufferSize ; i++ ) APP_LOG(TS_OFF, VLEVEL_L, "%02X|", Buffer[i]);
	  APP_LOG(TS_OFF, VLEVEL_L, "\r\nRSSI=%ddBm , SNR=%ddB\r\n\r\n", RssiValue, SnrValue);
	  if ((Buffer[0] == 'A') && (Buffer[1] == 'C') && (Buffer[2] == 'K'))
	  {
	    FrameAckCnt++;
	  }
	  break;

	case RX_TO:
	  Radio.Sleep();
	  APP_LOG(TS_ON, VLEVEL_L, "RX timeout\r\n\r\n");
	  break;

	case RX_ERR:
	  Radio.Sleep();

	  APP_LOG(TS_ON, VLEVEL_L, "RX error\r\n");
	  break;

	case TX_TO:
	  Radio.Sleep();

	  APP_LOG(TS_ON, VLEVEL_L, "TX timeout\r\n");
	  break;

	default:
      break;
  }
}

/* USER CODE END PrFD */
