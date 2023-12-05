/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : usbd_custom_hid_if.c
 * @version        : v1.0_Cube
 * @brief          : USB Device Custom HID interface file.
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
#include <cmsis_os2.h>
#include "usbd_custom_hid_if.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
extern uint8_t data_s[24];
extern osEventFlagsId_t eventHandle;
/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
 * @brief Usb device.
 * @{
 */

/** @addtogroup USBD_CUSTOM_HID
 * @{
 */

/** @defgroup USBD_CUSTOM_HID_Private_TypesDefinitions USBD_CUSTOM_HID_Private_TypesDefinitions
 * @brief Private types.
 * @{
 */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
 * @}
 */

/** @defgroup USBD_CUSTOM_HID_Private_Defines USBD_CUSTOM_HID_Private_Defines
 * @brief Private defines.
 * @{
 */

/* USER CODE BEGIN PRIVATE_DEFINES */
extern const uint8_t ITR_EVENT; //设置事件掩码的位 0
extern const uint8_t CMD_EVENT; //设置事件掩码的位 1
extern const uint8_t REG_EVENT; //设置事件掩码的位 2
extern const uint8_t SELF_TEST_EVENT;
extern const uint8_t NORMAL_SIG_EVENT;
/* USER CODE END PRIVATE_DEFINES */

/**
 * @}
 */

/** @defgroup USBD_CUSTOM_HID_Private_Macros USBD_CUSTOM_HID_Private_Macros
 * @brief Private macros.
 * @{
 */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
 * @}
 */

/** @defgroup USBD_CUSTOM_HID_Private_Variables USBD_CUSTOM_HID_Private_Variables
 * @brief Private variables.
 * @{
 */

/** Usb custom HID report descriptor. */
__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_HS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END =
        {
                /* USER CODE BEGIN 1 */
                0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
                0x09, 0x00,       // USAGE (Undefined)
                0xa1, 0x01,       // COLLECTION (Application)
                0x09, 0x00,       //   USAGE (Undefined)
                0x15, 0x00,       //   LOGICAL_MINIMUM (0)
                0x26, 0xff, 0x00, //   LOGICAL_MAXIMUM (255)
                0x95, 0x01,       //   REPORT_COUNT (64)
                0x75, 0x08,       //   REPORT_SIZE (8)
                0x81, 0x00,       //   INPUT (Data,Ary,Abs)
                0x09, 0x00,       //   USAGE (Undefined)
                0x15, 0x00,       //   LOGICAL_MINIMUM (0)
                0x26, 0xff, 0x00, //   LOGICAL_MAXIMUM (255)
                0x91, 0x00,       //   OUTPUT (Data,Ary,Abs)
                /* USER CODE END 1 */
                0xC0 /*     END_COLLECTION             */
        };
/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
 * @}
 */

/** @defgroup USBD_CUSTOM_HID_Exported_Variables USBD_CUSTOM_HID_Exported_Variables
 * @brief Public variables.
 * @{
 */

extern USBD_HandleTypeDef hUsbDeviceHS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */
/**
 * @}
 */

/** @defgroup USBD_CUSTOM_HID_Private_FunctionPrototypes USBD_CUSTOM_HID_Private_FunctionPrototypes
 * @brief Private functions declaration.
 * @{
 */

static int8_t CUSTOM_HID_Init_HS(void);

static int8_t CUSTOM_HID_DeInit_HS(void);

static int8_t CUSTOM_HID_OutEvent_HS(uint8_t event_idx, uint8_t state);

/**
 * @}
 */

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_HS =
        {
                CUSTOM_HID_ReportDesc_HS,
                CUSTOM_HID_Init_HS,
                CUSTOM_HID_DeInit_HS,
                CUSTOM_HID_OutEvent_HS};

/** @defgroup USBD_CUSTOM_HID_Private_Functions USBD_CUSTOM_HID_Private_Functions
 * @brief Private functions.
 * @{
 */

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Initializes the CUSTOM HID media low layer
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CUSTOM_HID_Init_HS(void) {
    /* USER CODE BEGIN 8 */
    return (USBD_OK);
    /* USER CODE END 8 */
}

/**
 * @brief  DeInitializes the CUSTOM HID media low layer
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CUSTOM_HID_DeInit_HS(void) {
    /* USER CODE BEGIN 9 */
    return (USBD_OK);
    /* USER CODE END 9 */
}

/**
 * @brief  Manage the CUSTOM HID class events
 * @param  event_idx: Event index
 * @param  state: Event state
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CUSTOM_HID_OutEvent_HS(uint8_t event_idx, uint8_t state) {
    /* USER CODE BEGIN 10 */
    UNUSED(event_idx);
    UNUSED(state);
    USBD_CUSTOM_HID_HandleTypeDef *hhid;

    if (hUsbDeviceHS.pClassData == NULL) {
        return (uint8_t) USBD_FAIL;
    }

    hhid = hUsbDeviceHS.pClassData;

    if (hhid->Report_buf[0] == 0x01) {
        osEventFlagsSet(eventHandle, REG_EVENT); //读寄存器的指令
    }

    if (hhid->Report_buf[0] == 0x02) {
        HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);
        osEventFlagsSet(eventHandle, CMD_EVENT); //读数据的指令
    }

    if (hhid->Report_buf[0] == 0x03) {
        HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
        osEventFlagsClear(eventHandle, CMD_EVENT); //停止读数据的指令
    }

    if (hhid->Report_buf[0] == 0x04) {
        osEventFlagsSet(eventHandle, SELF_TEST_EVENT); //设置为自检
    }

    if (hhid->Report_buf[0] == 0x05) {
        osEventFlagsSet(eventHandle, NORMAL_SIG_EVENT); //设置为正常输入
    }


    /* Start next USB packet transfer once data processing is completed */
    USBD_CUSTOM_HID_ReceivePacket(&hUsbDeviceHS);
    return (USBD_OK);
    /* USER CODE END 10 */
}

/* USER CODE BEGIN 11 */
/**
 * @brief  Send the report to the Host
 * @param  report: The report to be sent
 * @param  len: The report length
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */

uint8_t USBD_CUSTOM_HID_SendReport_HS(uint8_t *report, uint16_t len) {
    return USBD_CUSTOM_HID_SendReport(&hUsbDeviceHS, report, len);
}

/* USER CODE END 11 */

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
