/*
 * Copyright (c) 2024, starstory
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-03-10     starstory    first version
 */

#include "drv_nrf24l01.h"

#ifdef BSP_USING_NRF24L01

#include <rtdevice.h>
#include <drv_spi.h>

#define DBG_TAG "drv.nrf24"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static struct rt_spi_device *spi_dev_nrf24;
static struct rt_device nrf24_dev;

const uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0x01};
const uint8_t RX_ADDRESS[RX_ADR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0x01};

/* CE Pin Control */
#define NRF24L01_CE_LOW()   rt_pin_write(NRF24L01_CE_PIN, PIN_LOW)
#define NRF24L01_CE_HIGH()  rt_pin_write(NRF24L01_CE_PIN, PIN_HIGH)

/* SPI Helper Functions */
static rt_err_t nrf_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t send_buf[2];
    send_buf[0] = NRF_WRITE_REG | reg;
    send_buf[1] = value;
    return rt_spi_send(spi_dev_nrf24, send_buf, 2);
}

static uint8_t nrf_read_reg(uint8_t reg)
{
    uint8_t send_val = NRF_READ_REG | reg;
    uint8_t recv_val;
    rt_spi_send_then_recv(spi_dev_nrf24, &send_val, 1, &recv_val, 1);
    return recv_val;
}

static void nrf_read_buf(uint8_t reg, uint8_t *pBuf, uint8_t len)
{
    uint8_t send_val = NRF_READ_REG | reg;
    rt_spi_send_then_recv(spi_dev_nrf24, &send_val, 1, pBuf, len);
}

static void nrf_write_buf(uint8_t reg, uint8_t *pBuf, uint8_t len)
{
    uint8_t send_val = NRF_WRITE_REG | reg;
    rt_spi_send_then_send(spi_dev_nrf24, &send_val, 1, pBuf, len);
}

/* Public API Implementations */
void nrf24l01_write_reg(uint8_t reg, uint8_t value)
{
    nrf_write_reg(reg, value);
}

uint8_t nrf24l01_read_reg(uint8_t reg)
{
    return nrf_read_reg(reg);
}

uint8_t nrf24l01_check(void)
{
    uint8_t buf[5] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2};
    uint8_t i;

    nrf_write_buf(NRF_TX_ADDR, buf, 5);
    nrf_read_buf(NRF_TX_ADDR, buf, 5);

    for (i = 0; i < 5; i++)
    {
        if (buf[i] != 0xC2)
            return 0;
    }
    return 1;
}

void nrf24l01_init_config(void)
{
    NRF24L01_CE_LOW();
    
    nrf_write_reg(NRF_CONFIG, 0x0E); // Power up, CRC 2 bytes, PTX
    nrf_write_reg(NRF_EN_AA, 0x01);  // Enable Auto Ack pipe 0
    nrf_write_reg(NRF_EN_RXADDR, 0x01); // Enable Pipe 0
    nrf_write_reg(NRF_SETUP_RETR, 0x1A); // 500us + 86us, 10 retransmits
    nrf_write_reg(NRF_RF_CH, 40);        // Channel 40
    nrf_write_reg(NRF_RF_SETUP, 0x0F);   // 2Mbps, 0dBm
    nrf_write_reg(NRF_RX_PW_P0, RX_PLOAD_WIDTH);
    
    nrf_write_buf(NRF_TX_ADDR, (uint8_t*)TX_ADDRESS, TX_ADR_WIDTH);
    nrf_write_buf(NRF_RX_ADDR_P0, (uint8_t*)RX_ADDRESS, RX_ADR_WIDTH);
    
    nrf_write_reg(NRF_CONFIG, 0x0F); // Power up, CRC 2 bytes, PRX
    
    NRF24L01_CE_HIGH();
}

/* API: Send Packet */
void nrf24l01_tx_packet(uint8_t *txbuf)
{
    NRF24L01_CE_LOW();
    nrf_write_buf(WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH);
    NRF24L01_CE_HIGH();
}

/* API: Receive Packet */
void nrf24l01_rx_packet(uint8_t *rxbuf)
{
    NRF24L01_CE_HIGH();
    nrf_read_buf(RD_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH);
    nrf_write_reg(FLUSH_RX, 0xFF);
}

/* RT-Thread Device Interface */
static rt_err_t rt_nrf24_init(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t rt_nrf24_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t rt_nrf24_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_ssize_t rt_nrf24_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    nrf24l01_rx_packet((uint8_t *)buffer);
    return size;
}

static rt_ssize_t rt_nrf24_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    nrf24l01_tx_packet((uint8_t *)buffer);
    return size;
}

static rt_err_t rt_nrf24_control(rt_device_t dev, int cmd, void *args)
{
    switch (cmd)
    {
    case RT_DEVICE_CTRL_CONFIG:
        nrf24l01_init_config();
        break;
    default:
        break;
    }
    return RT_EOK;
}

/* Initialization */
static int rt_hw_nrf24l01_init(void)
{
    rt_err_t res;

    /* Config CE Pin */
    rt_pin_mode(NRF24L01_CE_PIN, PIN_MODE_OUTPUT);
    NRF24L01_CE_LOW();
    
    /* Config IRQ Pin */
    rt_pin_mode(NRF24L01_IRQ_PIN, PIN_MODE_INPUT_PULLUP);

    /* Attach SPI Device */
    /* Note: SPI Bus must be initialized before this. Ensure BSP_USING_SPI1 is set. */
    res = rt_hw_spi_device_attach(NRF24L01_SPI_BUS_NAME, NRF24L01_SPI_DEVICE_NAME, NRF24L01_CSN_PIN);
    if (res != RT_EOK)
    {
        LOG_E("Failed to attach SPI device %s", NRF24L01_SPI_DEVICE_NAME);
        return -RT_ERROR;
    }

    spi_dev_nrf24 = (struct rt_spi_device *)rt_device_find(NRF24L01_SPI_DEVICE_NAME);
    if (!spi_dev_nrf24)
    {
        LOG_E("Failed to find SPI device %s", NRF24L01_SPI_DEVICE_NAME);
        return -RT_ERROR;
    }

    /* Configure SPI Device */
    struct rt_spi_configuration cfg;
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = 4 * 1000 * 1000; /* 4MHz (NRF24L01 max 10MHz) */
    rt_spi_configure(spi_dev_nrf24, &cfg);

    /* Init NRF24L01 */
    if (nrf24l01_check())
    {
        LOG_I("NRF24L01 connection successful.");
        nrf24l01_init_config();
        
        /* Register RT-Thread Device */
        nrf24_dev.type = RT_Device_Class_Char;
        nrf24_dev.init = rt_nrf24_init;
        nrf24_dev.open = rt_nrf24_open;
        nrf24_dev.close = rt_nrf24_close;
        nrf24_dev.read = rt_nrf24_read;
        nrf24_dev.write = rt_nrf24_write;
        nrf24_dev.control = rt_nrf24_control;
        
        rt_device_register(&nrf24_dev, "nrf24", RT_DEVICE_FLAG_RDWR);
    }
    else
    {
        LOG_E("NRF24L01 connection failed!");
        return -RT_ERROR;
    }

    return RT_EOK;
}
/* Use INIT_APP_EXPORT to ensure SPI bus is initialized first */
INIT_APP_EXPORT(rt_hw_nrf24l01_init);

/* MSH Test Commands */
static void nrf24(int argc, char **argv)
{
    if (argc > 1)
    {
        if (!rt_strcmp(argv[1], "check"))
        {
            if (nrf24l01_check())
                rt_kprintf("NRF24L01 Check Passed.\n");
            else
                rt_kprintf("NRF24L01 Check Failed.\n");
        }
        else if (!rt_strcmp(argv[1], "reg"))
        {
             uint8_t status = nrf_read_reg(NRF_STATUS);
             uint8_t config = nrf_read_reg(NRF_CONFIG);
             rt_kprintf("STATUS: 0x%02X, CONFIG: 0x%02X\n", status, config);
        }
        else if (!rt_strcmp(argv[1], "send") && argc > 2)
        {
             uint8_t buf[32] = {0};
             rt_strncpy((char*)buf, argv[2], 32);
             nrf24l01_tx_packet(buf);
             rt_kprintf("Sent: %s\n", buf);
        }
    }
    else
    {
        rt_kprintf("Usage: nrf24 [check|reg|send <data>]\n");
    }
}
MSH_CMD_EXPORT(nrf24, NRF24L01 test command);

#endif /* BSP_USING_NRF24L01 */
