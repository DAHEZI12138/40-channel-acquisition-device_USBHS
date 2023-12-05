//
// Created by 30973 on 2022/5/1.
//

#ifndef STM32_ADS1299_40CHANNELS_ADS1299_H
#define STM32_ADS1299_40CHANNELS_ADS1299_H

#include <stdint-gcc.h>
//#include "gpio.h"
//#include "main.h"

//1299指令
#define ADS1299_WAKEUP 0X02
#define ADS1299_STANDBY 0X04
#define ADS1299_RESET 0X06
#define ADS1299_START 0X08
#define ADS1299_STOP 0X0A
#define ADS1299_RDATAC 0X10
#define ADS1299_SDATAC 0X11
#define ADS1299_RDATA 0X12
#define ADS1299_RREG 0X20
#define ADS1299_WREG 0X40

#define CH_n_SET 0X60 /*0x60：正常输入,0x65：自检方波*/


void ads1299_send_cmd(uint8_t cmd, int chip);

void ads1299_all_reg_write_1();

void ads1299_all_reg_write_2_5(int chip);

void ads1299_check_init();

void ads1299_all_reg_read(int chip);

void ads1299_all_data_read();

uint8_t ads1299_reg_read(uint8_t addr, int chip);

void ads1299_reg_write(uint8_t addr, uint8_t data, int chip);

uint8_t calcCRC(uint8_t *data, unsigned int len);

void all_ads1299_reg_read(void);


#endif //STM32_ADS1299_40CHANNELS_ADS1299_H

