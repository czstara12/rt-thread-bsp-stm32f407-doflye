/*
 * Copyright (c) 2024, starstory
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-03-10     starstory    first version
 */

#ifndef __DRV_NRF24L01_H__
#define __DRV_NRF24L01_H__

#include <rtthread.h>
#include <board.h>

/* NRF24L01 Pin definition */
#define NRF24L01_SPI_BUS_NAME       "spi1"  /* SPI1: SCK(PA5), MISO(PA6), MOSI(PA7) */
#define NRF24L01_SPI_DEVICE_NAME    "spi10"

#define NRF24L01_CE_PIN             GET_PIN(A, 4) /* PA4 */
#define NRF24L01_CSN_PIN            GET_PIN(C, 4) /* PC4 */
#define NRF24L01_IRQ_PIN            GET_PIN(C, 5) /* PC5 */

/* NRF24L01 Registers */
#define NRF_CONFIG      0x00
#define NRF_EN_AA       0x01
#define NRF_EN_RXADDR   0x02
#define NRF_SETUP_AW    0x03
#define NRF_SETUP_RETR  0x04
#define NRF_RF_CH       0x05
#define NRF_RF_SETUP    0x06
#define NRF_STATUS      0x07
#define NRF_OBSERVE_TX  0x08
#define NRF_CD          0x09
#define NRF_RX_ADDR_P0  0x0A
#define NRF_RX_ADDR_P1  0x0B
#define NRF_RX_ADDR_P2  0x0C
#define NRF_RX_ADDR_P3  0x0D
#define NRF_RX_ADDR_P4  0x0E
#define NRF_RX_ADDR_P5  0x0F
#define NRF_TX_ADDR     0x10
#define NRF_RX_PW_P0    0x11
#define NRF_RX_PW_P1    0x12
#define NRF_RX_PW_P2    0x13
#define NRF_RX_PW_P3    0x14
#define NRF_RX_PW_P4    0x15
#define NRF_RX_PW_P5    0x16
#define NRF_FIFO_STATUS 0x17

/* Commands */
#define NRF_READ_REG    0x00
#define NRF_WRITE_REG   0x20
#define RD_RX_PLOAD     0x61
#define WR_TX_PLOAD     0xA0
#define FLUSH_TX        0xE1
#define FLUSH_RX        0xE2
#define REUSE_TX_PL     0xE3
#define NOP             0xFF

/* Status bits */
#define RX_DR           0x40
#define TX_DS           0x20
#define MAX_RT          0x10

/* Parameters */
#define TX_ADR_WIDTH    5
#define RX_ADR_WIDTH    5
#define TX_PLOAD_WIDTH  32
#define RX_PLOAD_WIDTH  32

/* Function Declarations */
uint8_t nrf24l01_check(void);
void nrf24l01_tx_packet(uint8_t *txbuf);
void nrf24l01_rx_packet(uint8_t *rxbuf);
uint8_t nrf24l01_read_reg(uint8_t reg);
void nrf24l01_write_reg(uint8_t reg, uint8_t value);

#endif /* __DRV_NRF24L01_H__ */
