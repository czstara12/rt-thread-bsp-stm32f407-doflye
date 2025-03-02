/**
 * @file drv_nrf24l01.c
 * @author starstory (czstara12@qq.com)
 * @brief 2.4G无线模块驱动
 * @version 0.1
 * @date 2024-03-10
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "drv_nrf24l01.h"
#include "drv_spi.h"
#include "drv_gpio.h"
#include "drv_delay.h"

#define NRF24L01_CE_LOW()   HAL_GPIO_WritePin(NRF24L01_CE_GPIO_Port, NRF24L01_CE_Pin, GPIO_PIN_RESET)
#define NRF24L01_CE_HIGH()  HAL_GPIO_WritePin(NRF24L01_CE_GPIO_Port, NRF24L01_CE_Pin, GPIO_PIN_SET)
#define NRF24L01_CSN_LOW()  HAL_GPIO_WritePin(NRF24L01_CSN_GPIO_Port, NRF24L01_CSN_Pin, GPIO_PIN_RESET)
#define NRF24L01_CSN_HIGH() HAL_GPIO_WritePin(NRF24L01_CSN_GPIO_Port, NRF24L01_CSN_Pin, GPIO_PIN_SET)

/**
 * @brief 读取寄存器
 * 
 * @param reg 寄存器地址
 * @return uint8_t 寄存器值
 */
uint8_t NRF24L01_Read_Reg(uint8_t reg)
{
    uint8_t reg_val;
    NRF24L01_CSN_LOW();
    HAL_SPI_Transmit(&hspi1, reg, 1, 1000);
    HAL_SPI_Receive(&hspi1, &reg_val, 1, 1000);
    NRF24L01_CSN_HIGH();
    return reg_val;
}

/**
 * @brief 写寄存器
 * 
 * @param reg 寄存器地址
 * @param value 写入的值
 */
void NRF24L01_Write_Reg(uint8_t reg, uint8_t value)
{
    NRF24L01_CSN_LOW();
    HAL_SPI_Transmit(&hspi1, reg, 1, 1000);
    HAL_SPI_Transmit(&hspi1, &value, 1, 1000);
    NRF24L01_CSN_HIGH();
}

/**
 * @brief 读取数据
 * 
 * @param reg 寄存器地址
 * @param buf 数据缓冲区
 * @param len 数据长度
 */
void NRF24L01_Read_Buf(uint8_t reg, uint8_t *buf, uint8_t len)
{
    NRF24L01_CSN_LOW();
    HAL_SPI_Transmit(&hspi1, reg, 1, 1000);
    HAL_SPI_Receive(&hspi1, buf, len, 1000);
    NRF24L01_CSN_HIGH();
}

/**
 * @brief 写数据
 * 
 * @param reg 寄存器地址
 * @param buf 数据缓冲区
 * @param len 数据长度
 */
void NRF24L01_Write_Buf(uint8_t reg, uint8_t *buf, uint8_t len)
{
    NRF24L01_CSN_LOW();
    HAL_SPI_Transmit(&hspi1, reg, 1, 1000);
    HAL_SPI_Transmit(&hspi1, buf, len, 1000);
    NRF24L01_CSN_HIGH();
}

/**
 * @brief 发送数据
 * 
 * @param txbuf 发送数据缓冲区
 */
void NRF24L01_TxPacket(uint8_t *txbuf)
{
    NRF24L01_CE_LOW();
    NRF24L01_Write_Buf(WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH);
    NRF24L01_CE_HIGH();
    delay_us(15);
}

/**
 * @brief 接收数据
 * 
 * @param rxbuf 接收数据缓冲区
 */
void NRF24L01_RxPacket(uint8_t *rxbuf)
{
    NRF24L01_CE_HIGH();
    delay_us(15);
    NRF24L01_Read_Buf(RD_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH);
    NRF24L01_Write_Reg(FLUSH_RX, 0xff);
}

/**
 * @brief 初始化2.4G无线模块
 * 
 */
void NRF24L01_Init(void)
{
    NRF24L01_CE_LOW();
    NRF24L01_CSN_HIGH();
    delay_ms(5);
    NRF24L01_Write_Reg(WRITE_REG + CONFIG, 0x0e);
    NRF24L01_Write_Reg(WRITE_REG + EN_AA, 0x01);
    NRF24L01_Write_Reg(WRITE_REG + EN_RXADDR, 0x01);
    NRF24L01_Write_Reg(WRITE_REG + SETUP_RETR, 0x1a);
    NRF24L01_Write_Reg(WRITE_REG + RF_CH, 40);
    NRF24L01_Write_Reg(WRITE_REG + RF_SETUP, 0x0f);
    NRF24L01_Write_Reg(WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH);
    NRF24L01_Write_Reg(WRITE_REG + RX_ADDR_P0, (uint8_t *)RX_ADDRESS, RX_ADR_WIDTH);
    NRF24L01_Write_Reg(WRITE_REG + TX_ADDR, (uint8_t *)TX_ADDRESS, TX_ADR_WIDTH);
    NRF24L01_Write_Reg(WRITE_REG + CONFIG, 0x0f);
}

/**
 * @brief 检查2.4G无线模块是否正常
 * 
 * @return uint8_t 1:正常 0:异常
 */
uint8_t NRF24L01_Check(void)
{
    uint8_t buf[5] = {0xc2, 0xc2, 0xc2, 0xc2, 0xc2};
    uint8_t i;
    NRF24L01_Write_Buf(WRITE_REG + TX_ADDR, buf, 5);
    NRF24L01_Read_Buf(TX_ADDR, buf, 5);
    for (i = 0; i < 5; i++)
    {
        if (buf[i] != 0xc2)
        {
            return 0;
        }
    }
    return 1;
}

