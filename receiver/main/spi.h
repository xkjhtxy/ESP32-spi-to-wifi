#ifndef __SPI_H
#define __SPI_H
#include "driver/spi_slave.h"
#include "driver/gpio.h"
#include "string.h"


//经典32接口
#define GPIO_HANDSHAKE 26
#define GPIO_MOSI 12
#define GPIO_MISO 13
#define GPIO_SCLK 27
#define GPIO_CS 14
#define GPIO_DATA_OUT 25
#define RCV_HOST    VSPI_HOST

// //ESP32 S3接口
// #define GPIO_HANDSHAKE 14
// #define GPIO_MOSI 11
// #define GPIO_MISO 13
// #define GPIO_SCLK 12
// #define GPIO_CS 10
// #define GPIO_DATA_OUT 15
// #define RCV_HOST    SPI3_HOST



extern spi_slave_transaction_t t;
extern unsigned char Spi_Transmit_buffer[1024];
extern unsigned char Spi_Receive_buffer[1024];

void spi_Init(void);



#endif

