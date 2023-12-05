/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include <tim.h>
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gpio.h"
#include "spi.h"
#include "ads1299.h"
#include "usbd_custom_hid_if.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* CRC校验开关*/
#define _CRC_ 1

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern const uint8_t ITR_EVENT;   // 璁剧疆浜嬩欢鎺╃爜鐨勪綅 0
extern const uint8_t CMD_EVENT;   // 璁剧疆浜嬩欢鎺╃爜鐨勪綅 1
extern const uint8_t REG_EVENT;   // 璁剧疆浜嬩欢鎺╃爜鐨勪綅 2
extern const uint8_t READY_EVENT; // 璁剧疆浜嬩欢鎺╃爜鐨勪綅 3
extern const uint8_t SELF_TEST_EVENT;
extern const uint8_t NORMAL_SIG_EVENT;

extern USBD_HandleTypeDef hUsbDeviceHS;
extern int itr_num;
int buffer_select_flag = 0;
const int itr_num_max = 25;

/* 存储buffer定义 */
#if _CRC_
const int send_max_size =
    (40 * 3 + 5 * 3) * itr_num_max + 1; // 40*3*itr_num_max 是数据   5*3是电极脱落检测 /* 最后一位为CRC校验码*/
uint8_t data_save_a[(40 * 3 + 5 * 3) * 25 + 1] = {0};
uint8_t data_save_b[(40 * 3 + 5 * 3) * 25 + 1] = {0};
#else
const int send_max_size = (40 * 3 + 5 * 3) * itr_num_max;
uint8_t data_save_a[(40 * 3 + 5 * 3) * 25] = {0};
uint8_t data_save_b[(40 * 3 + 5 * 3) * 25] = {0};
#endif

uint8_t send1[3] = {0XAA, 0XAA, 0XAA};
uint8_t send2[24] = {0};
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
    .name = "defaultTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for data_read */
osThreadId_t data_readHandle;
const osThreadAttr_t data_read_attributes = {
    .name = "data_read",
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for all_reg_read */
osThreadId_t all_reg_readHandle;
const osThreadAttr_t all_reg_read_attributes = {
    .name = "all_reg_read",
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for data_send */
osThreadId_t data_sendHandle;
const osThreadAttr_t data_send_attributes = {
    .name = "data_send",
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for self_test */
osThreadId_t self_testHandle;
const osThreadAttr_t self_test_attributes = {
    .name = "self_test",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for normal_sig */
osThreadId_t normal_sigHandle;
const osThreadAttr_t normal_sig_attributes = {
    .name = "normal_sig",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for DATA_READY */
osSemaphoreId_t DATA_READYHandle;
const osSemaphoreAttr_t DATA_READY_attributes = {
    .name = "DATA_READY"};
/* Definitions for event */
osEventFlagsId_t eventHandle;
const osEventFlagsAttr_t event_attributes = {
    .name = "event"};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

_Noreturn void StartDefaultTask(void *argument);

_Noreturn void data_read_func(void *argument);

_Noreturn void reg_read_func(void *argument);

_Noreturn void data_send_func(void *argument);

_Noreturn void self_test_func(void *argument);

_Noreturn void normal_sig_func(void *argument);

extern void MX_USB_DEVICE_Init(void);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* Create the semaphores(s) */
    /* creation of DATA_READY */
    DATA_READYHandle = osSemaphoreNew(1, 1, &DATA_READY_attributes);

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    /* USER CODE END RTOS_QUEUES */

    /* Create the thread(s) */
    /* creation of defaultTask */
    defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

    /* creation of data_read */
    data_readHandle = osThreadNew(data_read_func, NULL, &data_read_attributes);

    /* creation of all_reg_read */
    all_reg_readHandle = osThreadNew(reg_read_func, NULL, &all_reg_read_attributes);

    /* creation of data_send */
    data_sendHandle = osThreadNew(data_send_func, NULL, &data_send_attributes);

    /* creation of self_test */
    self_testHandle = osThreadNew(self_test_func, NULL, &self_test_attributes);

    /* creation of normal_sig */
    normal_sigHandle = osThreadNew(normal_sig_func, NULL, &normal_sig_attributes);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    /* USER CODE END RTOS_THREADS */

    /* Create the event(s) */
    /* creation of event */
    eventHandle = osEventFlagsNew(&event_attributes);

    /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
    /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
_Noreturn void StartDefaultTask(void *argument)
{
    /* init code for USB_DEVICE */
    MX_USB_DEVICE_Init();
    /* USER CODE BEGIN StartDefaultTask */
    /* Infinite loop */
    for (;;)
    {
        osDelay(2000);
    }
    /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_data_read_func */
/**
 * @brief Function implementing the data_read thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_data_read_func */
_Noreturn void data_read_func(void *argument)
{
    /* USER CODE BEGIN data_read_func */
    /* Infinite loop */
    for (;;)
    {
        osEventFlagsWait(eventHandle, ITR_EVENT | CMD_EVENT, osFlagsNoClear | osFlagsWaitAll, osWaitForever);
        osEventFlagsClear(eventHandle, ITR_EVENT);

        if (buffer_select_flag == 0)
        {
            CS_LOW(1);
            HAL_SPI_TransmitReceive(&hspi3, send1, data_save_a + 135 * itr_num, 3, 100);
            HAL_SPI_TransmitReceive(&hspi3, send2, data_save_a + 15 + 135 * itr_num, 24, 100);
            CS_HIGH(1);

            CS_LOW(2);
            HAL_SPI_TransmitReceive(&hspi3, send1, data_save_a + 3 + 135 * itr_num, 3, 100);
            HAL_SPI_TransmitReceive(&hspi3, send2, data_save_a + 15 + 24 + 135 * itr_num, 24, 100);
            CS_HIGH(2);

            CS_LOW(3);
            HAL_SPI_TransmitReceive(&hspi3, send1, data_save_a + 6 + 135 * itr_num, 3, 100);
            HAL_SPI_TransmitReceive(&hspi3, send2, data_save_a + 15 + 48 + 135 * itr_num, 24, 100);
            CS_HIGH(3);

            CS_LOW(4);
            HAL_SPI_TransmitReceive(&hspi3, send1, data_save_a + 9 + 135 * itr_num, 3, 100);
            HAL_SPI_TransmitReceive(&hspi3, send2, data_save_a + 15 + 72 + 135 * itr_num, 24, 100);
            CS_HIGH(4);

            CS_LOW(5);
            HAL_SPI_TransmitReceive(&hspi3, send1, data_save_a + 12 + 135 * itr_num, 3, 100);
            HAL_SPI_TransmitReceive(&hspi3, send2, data_save_a + 15 + 96 + 135 * itr_num, 24, 100);
            CS_HIGH(5);

            itr_num++;

            if (itr_num == itr_num_max)
            {
#if _CRC_
                data_save_a[3375] = calcCRC(data_save_a, 3375);
#endif
                itr_num = 0;
                buffer_select_flag = 1;
                osEventFlagsSet(eventHandle, READY_EVENT);
            }
        }
        else
        {
            CS_LOW(1);
            HAL_SPI_TransmitReceive(&hspi3, send1, data_save_b + 135 * itr_num, 3, 100);
            HAL_SPI_TransmitReceive(&hspi3, send2, data_save_b + 15 + 135 * itr_num, 24, 100);
            CS_HIGH(1);

            CS_LOW(2);
            HAL_SPI_TransmitReceive(&hspi3, send1, data_save_b + 3 + 135 * itr_num, 3, 100);
            HAL_SPI_TransmitReceive(&hspi3, send2, data_save_b + 15 + 24 + 135 * itr_num, 24, 100);
            CS_HIGH(2);

            CS_LOW(3);
            HAL_SPI_TransmitReceive(&hspi3, send1, data_save_b + 6 + 135 * itr_num, 3, 100);
            HAL_SPI_TransmitReceive(&hspi3, send2, data_save_b + 15 + 48 + 135 * itr_num, 24, 100);
            CS_HIGH(3);

            CS_LOW(4);
            HAL_SPI_TransmitReceive(&hspi3, send1, data_save_b + 9 + 135 * itr_num, 3, 100);
            HAL_SPI_TransmitReceive(&hspi3, send2, data_save_b + 15 + 72 + 135 * itr_num, 24, 100);
            CS_HIGH(4);

            CS_LOW(5);
            HAL_SPI_TransmitReceive(&hspi3, send1, data_save_b + 12 + 135 * itr_num, 3, 100);
            HAL_SPI_TransmitReceive(&hspi3, send2, data_save_b + 15 + 96 + 135 * itr_num, 24, 100);
            CS_HIGH(5);

            itr_num++;

            if (itr_num == itr_num_max)
            {
#if _CRC_
                data_save_b[3375] = calcCRC(data_save_b, 3375);
#endif
                itr_num = 0;
                buffer_select_flag = 0;
                osEventFlagsSet(eventHandle, READY_EVENT);
            }
        }
    }
    /* USER CODE END data_read_func */
}

/* USER CODE BEGIN Header_reg_read_func */
/**
 * @brief Function implementing the all_reg_read thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_reg_read_func */
_Noreturn void reg_read_func(void *argument)
{
    /* USER CODE BEGIN reg_read_func */

    /* Infinite loop */
    for (;;)
    {
        osEventFlagsWait(eventHandle, REG_EVENT, osFlagsWaitAny, osWaitForever);
        for (int i = 1; i < 6; i++)
        {
            ads1299_send_cmd(ADS1299_SDATAC, i);
            HAL_Delay(1);
        }

        all_ads1299_reg_read();

        for (int i = 1; i < 6; i++)
        {
            ads1299_send_cmd(ADS1299_RDATAC, i);
            HAL_Delay(1);
        }
    }
    /* USER CODE END reg_read_func */
}

/* USER CODE BEGIN Header_data_send_func */
/**
 * @brief Function implementing the data_send thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_data_send_func */
_Noreturn void data_send_func(void *argument)
{
    /* USER CODE BEGIN data_send_func */
    /* Infinite loop */
    for (;;)
    {
        osEventFlagsWait(eventHandle, READY_EVENT, osFlagsWaitAny, osWaitForever);
        if (buffer_select_flag == 0)
        {
            USBD_CUSTOM_HID_SendReport_HS(data_save_b, send_max_size);
        }
        else
        {
            USBD_CUSTOM_HID_SendReport_HS(data_save_a, send_max_size);
        }
    }
    /* USER CODE END data_send_func */
}

/* USER CODE BEGIN Header_self_test_func */
/**
 * @brief Function implementing the self_test thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_self_test_func */
_Noreturn void self_test_func(void *argument)
{
    /* USER CODE BEGIN self_test_func */
    uint8_t reg_def = 0x65;
    /* Infinite loop */
    for (;;)
    {
        osEventFlagsWait(eventHandle, SELF_TEST_EVENT, osFlagsWaitAny, osWaitForever);
        for (int i = 1; i < 6; i++)
        {
            ads1299_send_cmd(ADS1299_SDATAC, i);
            HAL_Delay(1);
        }

        for (int j = 1; j < 6; j++)
        {
            for (int i = 0; i < 8; ++i)
            {
                ads1299_reg_write(0x05 + i, reg_def, j);
            }
        }

        for (int i = 1; i < 6; i++)
        {
            ads1299_send_cmd(ADS1299_RDATAC, i);
            HAL_Delay(1);
        }
    }
    /* USER CODE END self_test_func */
}

/* USER CODE BEGIN Header_normal_sig_func */
/**
 * @brief Function implementing the normal_sig thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_normal_sig_func */
_Noreturn void normal_sig_func(void *argument)
{
    /* USER CODE BEGIN normal_sig_func */
    uint8_t normal_def = 0x60;
    /* Infinite loop */
    for (;;)
    {
        osEventFlagsWait(eventHandle, NORMAL_SIG_EVENT, osFlagsWaitAny, osWaitForever);
        for (int i = 1; i < 6; i++)
        {
            ads1299_send_cmd(ADS1299_SDATAC, i);
            HAL_Delay(1);
        }

        for (int j = 1; j < 6; j++)
        {
            for (int i = 0; i < 8; ++i)
            {
                ads1299_reg_write(0x05 + i, normal_def, j);
            }
        }

        for (int i = 1; i < 6; i++)
        {
            ads1299_send_cmd(ADS1299_RDATAC, i);
            HAL_Delay(1);
        }
    }
    /* USER CODE END normal_sig_func */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
