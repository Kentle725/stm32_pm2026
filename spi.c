#include <stm32f10x.h>
#include "spi.h"


void SPI1_Init(void) {
    // Тактирование GPIOA и SPI1
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // Конфигурация выводов для SPI1

    // Сбрасываем старые настройки
    GPIOA->CRL &= ~(GPIO_CRL_MODE5 | GPIO_CRL_CNF5 |
                GPIO_CRL_MODE7 | GPIO_CRL_CNF7);
    // Настраиваем пины
    GPIOA->CRL |= (GPIO_CRL_MODE5 | GPIO_CRL_CNF5_1 |
                GPIO_CRL_MODE7 | GPIO_CRL_CNF7_1);
    
    // Конфигурация модуля SPI1
    SPI1->CR1 |= SPI_CR1_CPOL | SPI_CR1_CPHA |
                SPI_CR1_MSTR | SPI_CR1_BR |
                SPI_CR1_SSM | SPI_CR1_SPE;
} 

void SPI1_Write(uint8_t data) {
    //Ждем, пока не освободится буфер передатчика
    while(!(SPI1->SR & SPI_SR_TXE));
    //заполняем буфер передатчика
    SPI1->DR = data;
}

uint8_t SPI1_Read(void) {
    SPI1->DR = 0; //запускаем обмен
    //Ждем, пока не появится новое значение
    //в буфере приемника
    while(!(SPI1->SR & SPI_SR_RXNE));
    //возвращаем значение буфера приемника
    return SPI1->DR;
}