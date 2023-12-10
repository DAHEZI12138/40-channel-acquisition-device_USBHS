//
// Created by 30973 on 2022/5/1.
//

#include <tim.h>
#include "usbd_custom_hid_if.h"
#include "ads1299.h"
#include "spi.h"
#include "gpio.h"

// 第一片1299寄存器配置
unsigned char ADS1x9x_Default_Register_Settings_1[24] =
    {
        0x00,     // 00-Device ID read Only
        0xF4,     // 01-CONFIG1 //5：500 sps  4：1k sps  3:2k sps
        0xD4,     // 02-CONFIG2
        0xEE,     // 03-CONFIG3 启用BIAS，使用内部参考电压 //0xFE 0xE0
        0x07,     // 04-LOFF   设置成0x00 开启直流电极脱落检测
        CH_n_SET, // 05-CH1SET (default)
        CH_n_SET, // 06-CH2SET (default)
        CH_n_SET, // 07-CH3SET (default)
        CH_n_SET, // 08-CH4SET (default)
        CH_n_SET, // 09-CH5SET (default)
        CH_n_SET, // 0A-CH6SET (default)
        CH_n_SET, // 0B-CH7SET (default)
        CH_n_SET, // 0C-CH8SET (default)
        0xFF,     // 0D-BIAS_SENSP
        0xFF,     // 0E-BIAS_SENSN
        0xFF,     // 0F-LOFF_SENSP (default)    0XFF:Turn on the P-side of all channels for lead-off sensing
        0xFF,     // 10-LOFF_SENSN (default)    0XFF:Turn on the N-side of all channels for lead-off sensing
        0x00,     // 11-LOFF_FLIP  (default)
        0x00,     // 12-LOFF_STATP (default)
        0x00,     // 13-LOFF_STATN (default)    可以通过读取这两个寄存器判断电极是否脱落，也可以通过DOUT的status（前24bit）
        0x00,     // 14-GPIO
        0x00,     // 15-MISC1
        0x00,     // 16-MISC2
        0x00      // 17-CONFIG4   0x02: Turn on dc lead-off comparators
};

// 2-5片1299寄存器配置
unsigned char ADS1x9x_Default_Register_Settings_2_5[24] =
    {
        0x00,     // 00-Device ID read Only
        0xD4,     // 01-CONFIG1 //5：500 sps  4：1k sps  3:2k sps
        0xD4,     // 02-CONFIG2
        0xE0,     // 03-CONFIG3 启用BIAS，使用内部参考电压 //0xFE 0xE0
        0x07,     // 04-LOFF   设置成0x13 开启电极脱落检测
        CH_n_SET, // 05-CH1SET (default)
        CH_n_SET, // 06-CH2SET (default)
        CH_n_SET, // 07-CH3SET (default)
        CH_n_SET, // 08-CH4SET (default)
        CH_n_SET, // 09-CH5SET (default)
        CH_n_SET, // 0A-CH6SET (default)
        CH_n_SET, // 0B-CH7SET (default)
        CH_n_SET, // 0C-CH8SET (default)
        0xFF,     // 0D-BIAS_SENSP
        0xFF,     // 0E-BIAS_SENSN
        0xFF,     // 0F-LOFF_SENSP (default)    0XFF:Turn on the P-side of all channels for lead-off sensing
        0xFF,     // 10-LOFF_SENSN (default)    0XFF:Turn on the N-side of all channels for lead-off sensing
        0x00,     // 11-LOFF_FLIP  (default)
        0x00,     // 12-LOFF_STATP (default)
        0x00,     // 13-LOFF_STATN (default)
        0x00,     // 14-GPIO
        0x00,     // 15-MISC1
        0x00,     // 16-MISC2
        0x00      // 17-CONFIG4   0x02: Turn on dc lead-off comparators
};

/** ads1299_reg_read
 * @brief 读一个寄存器
 * @param addr: 寄存器地址
 * @param chip: 指定芯片
 * @retval 返回寄存器的值
 */
uint8_t ads1299_reg_read(uint8_t addr, int chip)
{
    uint8_t data_send[3];
    uint8_t data_receive[3];
    data_send[0] = addr | ADS1299_RREG;
    data_send[1] = 0x00;
    data_send[2] = 0xAA;
    CS_LOW(chip);
    HAL_SPI_TransmitReceive(&hspi3, data_send, data_receive, 3, 100);
    HAL_Delay(1);
    CS_HIGH(chip);
    return data_receive[2];
}

/** ads1299_reg_write
 * @brief 写一个寄存器.
 * @param addr: 寄存器地址
 * @param chip: 指定芯片
 * @retval None
 */
void ads1299_reg_write(uint8_t addr, uint8_t data, int chip)
{
    uint8_t data_send[3];
    data_send[0] = addr | ADS1299_WREG;
    data_send[1] = 0x00;
    data_send[2] = data;
    CS_LOW(chip);
    HAL_SPI_Transmit(&hspi3, data_send, 3, 100);
    HAL_Delay(1);
    CS_HIGH(chip);
}

/** ads1299_send_cmd
 * @brief 发送指令
 * @param cmd: 指令代码
 * @param chip: 指定芯片
 * @retval None
 */
void ads1299_send_cmd(uint8_t cmd, int chip)
{
    CS_LOW(chip);
    HAL_SPI_Transmit(&hspi3, &cmd, 1, 100);
    HAL_Delay(1);
    CS_HIGH(chip);
}

// 初始化第一片1299的寄存器
void ads1299_all_reg_write_1()
{
    uint8_t data_send[25];
    data_send[0] = 0x01 | ADS1299_WREG; // 从地址为0x01的寄存器开始写
    data_send[1] = 0x16;                // 需要写入的寄存器个数为23 0x17 减一为0x16
    for (int i = 0; i < 23; i++)
    {
        data_send[i + 2] = ADS1x9x_Default_Register_Settings_1[i + 1]; // 写入的值
    }
    CS_LOW(1);
    HAL_SPI_Transmit(&hspi3, data_send, 25, 100);
    HAL_Delay(1);
    CS_HIGH(1);
}

// 初始化2-5片1299的寄存器
void ads1299_all_reg_write_2_5(int chip)
{
    uint8_t data_send[25];
    data_send[0] = 0x01 | ADS1299_WREG; // 从地址为0x01的寄存器开始写
    data_send[1] = 0x16;                // 需要写入的寄存器个数为23 0x17 减一为0x16
    for (int i = 0; i < 23; i++)
    {
        data_send[i + 2] = ADS1x9x_Default_Register_Settings_2_5[i + 1]; // 写入的值
    }
    CS_LOW(chip);
    HAL_SPI_Transmit(&hspi3, data_send, 25, 100);
    HAL_Delay(1);
    CS_HIGH(chip);
}

// 初始化所有1299寄存器 由于时钟设置问题，需要先初始化第一片1299
void ads1299_check_init()
{
    HAL_GPIO_WritePin(RESET_GPIO_Port, RESET_Pin, GPIO_PIN_RESET); // reset high
    HAL_Delay(1000);
    HAL_GPIO_WritePin(RESET_GPIO_Port, RESET_Pin, GPIO_PIN_SET); // reset high
    HAL_Delay(300);
    HAL_GPIO_WritePin(START_GPIO_Port, START_Pin, GPIO_PIN_RESET);

    ads1299_send_cmd(ADS1299_SDATAC, 1);
    HAL_Delay(300);
    ads1299_all_reg_write_1(); // 初始化第一片1299
    HAL_Delay(1000);

    ads1299_send_cmd(ADS1299_SDATAC, 2);
    ads1299_all_reg_write_2_5(2);
    HAL_Delay(50);
    ads1299_send_cmd(ADS1299_SDATAC, 3);
    ads1299_all_reg_write_2_5(3);
    HAL_Delay(50);
    ads1299_send_cmd(ADS1299_SDATAC, 4);
    ads1299_all_reg_write_2_5(4);
    HAL_Delay(50);
    ads1299_send_cmd(ADS1299_SDATAC, 5);
    ads1299_all_reg_write_2_5(5);
    HAL_Delay(1000);

    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

    ads1299_send_cmd(ADS1299_RDATAC, 1);
    ads1299_send_cmd(ADS1299_RDATAC, 2);
    ads1299_send_cmd(ADS1299_RDATAC, 3);
    ads1299_send_cmd(ADS1299_RDATAC, 4);
    ads1299_send_cmd(ADS1299_RDATAC, 5);
}

// 读取指定1299的所有寄存器
void ads1299_all_reg_read(int chip)
{
    uint8_t data_send[26] = {0};
    uint8_t reg_buffer[26];
    data_send[0] = 0x00 | ADS1299_RREG;
    data_send[1] = 0x17;
    CS_LOW(chip);
    HAL_SPI_TransmitReceive(&hspi3, data_send, reg_buffer, 26, 100);
    HAL_Delay(1);
    CS_HIGH(chip);
    USBD_CUSTOM_HID_SendReport_HS(reg_buffer + 2, 24);
}

// 读取所有1299的所有寄存器
void all_ads1299_reg_read(void)
{
    uint8_t data_send[26] = {0};
    uint8_t reg_buffer[120] = {0};
    data_send[0] = 0x00 | ADS1299_RREG;
    data_send[1] = 0x17;

    CS_LOW(1);
    HAL_SPI_Transmit(&hspi3, data_send, 2, 100);
    HAL_SPI_TransmitReceive(&hspi3, data_send, reg_buffer, 24, 100);
    CS_HIGH(1);

    CS_LOW(2);
    HAL_SPI_Transmit(&hspi3, data_send, 2, 100);
    HAL_SPI_TransmitReceive(&hspi3, data_send, reg_buffer + 24, 24, 100);
    CS_HIGH(2);

    CS_LOW(3);
    HAL_SPI_Transmit(&hspi3, data_send, 2, 100);
    HAL_SPI_TransmitReceive(&hspi3, data_send, reg_buffer + 48, 24, 100);
    CS_HIGH(3);

    CS_LOW(4);
    HAL_SPI_Transmit(&hspi3, data_send, 2, 100);
    HAL_SPI_TransmitReceive(&hspi3, data_send, reg_buffer + 72, 24, 100);
    CS_HIGH(4);

    CS_LOW(5);
    HAL_SPI_Transmit(&hspi3, data_send, 2, 100);
    HAL_SPI_TransmitReceive(&hspi3, data_send, reg_buffer + 96, 24, 100);
    CS_HIGH(5);

    USBD_CUSTOM_HID_SendReport_HS(reg_buffer, 120); //5*24，更改这里
}

// 按照 X^8+X^2+X^1+1 生成
static const unsigned int crc8Table[256] =
    {
        0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
        0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
        0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
        0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
        0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
        0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
        0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
        0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
        0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
        0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
        0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
        0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
        0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
        0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
        0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
        0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3};

// 计算CRC
uint8_t calcCRC(uint8_t *data, unsigned int len)
{
    unsigned char crc8 = 0;
    while (len--)
    {
        crc8 = crc8 ^ (*data++);
        crc8 = crc8Table[crc8];
    }
    return crc8;
}
