#include <stdint.h>
#include <stm32f10x.h>

#include "spi.h"


void delay(uint32_t ticks) {
	for (int i=0; i<ticks; i++) {
		__NOP();
	}
}

// Отправка команды
void display_cmd(uint8_t cmd) {
	GPIOA->ODR &= ~GPIO_ODR_ODR4; // CS = 0;
	GPIOA->ODR &= ~GPIO_ODR_ODR1; // DC=0
	delay(1000);
	SPI1_Write(cmd);
	// ждём, пока SPI1 закончит передачу последнего бита
	while (SPI1->SR & SPI_SR_BSY) {}
	GPIOA->ODR |= GPIO_ODR_ODR4; // CS=1
}

// Отправка данных
void display_data(uint8_t data) {
	GPIOA->ODR &= ~GPIO_ODR_ODR4; // CS = 0;
	GPIOA->ODR |= GPIO_ODR_ODR1; // DC=1
	delay(1000);
	SPI1_Write(data);
	while (SPI1->SR & SPI_SR_BSY) {}
	GPIOA->ODR |= GPIO_ODR_ODR4; // CS=1
}

// Вывод шахматной доски 8x8 клеток по 8x8 пикселей
void display_chessboard(void) {
	for (uint8_t page = 0; page < 8; page++) {
        for (uint8_t col = 0; col < 128; col++) {
			// Каждые 8 столбцов (col / 8) меняем цвет, на каждой новой странице (page) сдвигаем шахматный порядок
            uint8_t is_white = ((col / 8) + page) % 2;
			display_data(is_white ? 0xFF : 0x00);
		}
	}
}

int __attribute((noreturn)) main(void) {
	SPI1_Init();

	// PA1 (DC), PA2 (/RES), PA4 (/CS)
	GPIOA->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1 |
				GPIO_CRL_MODE2 | GPIO_CRL_CNF2 |
			    GPIO_CRL_MODE4 | GPIO_CRL_CNF4);
	GPIOA->CRL |= (GPIO_CRL_MODE1 | GPIO_CRL_MODE2 | GPIO_CRL_MODE4);

	GPIOA->ODR |= GPIO_ODR_ODR4; // CS=1

	// Аппаратный сброс
	GPIOA->ODR &= ~GPIO_ODR_ODR2;
	delay(100000);
	GPIOA->ODR |= GPIO_ODR_ODR2;
	delay(100000);
	
	// Последовательность базовых команд инициализации дисплея (128×64)
	
	const uint8_t ssd1306_init_cmds[] = {
    	0xAE,       // Display OFF (выключить дисплей на время настройки)
    
    	0xD5, 0x80, // Set Display Clock Divide Ratio / Oscillator Frequency (стандартная частота)
    	0xA8, 0x3F, // Set Multiplex Ratio (0x3F = 63 для высоты 64 пикселя; для 128x32 нужно 0x1F)
    	0xD3, 0x00, // Set Display Offset (смещение дисплея = 0)
    	0x40,       // Set Display Start Line (начальная линия = 0)
    
    	0x8D, 0x14, // Charge Pump Setting (0x14 — включить внутренний преобразователь напряжения, критично!)
    
    	0x20, 0x00, // Memory Addressing Mode (0x00 — горизонтальная адресация для удобной заливки буфера)
    	0xA1,       // Segment Re-map (0xA1 — нормальная ориентация по горизонтали; 0xA0 — отзеркалить)
    	0xC8,       // COM Output Scan Direction (0xC8 — нормальная по вертикали; 0xC0 — перевернуть)
    
    	0xDA, 0x12, // Set COM Pins Hardware Configuration (0x12 для 128x64; для 128x32 нужно 0x02)
    	0x81, 0xCF, // Set Contrast Control (контрастность от 0x00 до 0xFF)
    	0xD9, 0xF1, // Set Pre-charge Period (фаза предзаряда)
    	0xDB, 0x40, // Set VCOMH Deselect Level (уровень напряжения VCOMH)
    
    	0xA4,       // Entire Display ON (0xA4 — вывод из RAM; 0xA5 — зажечь все пиксели принудительно)
    	0xA6,       // Set Normal/Inverse Display (0xA6 — обычный режим; 0xA7 — инверсия цветов)
    
    	0xAF        // Display ON (включить дисплей)
	};

	for (uint32_t i = 0; i < sizeof(ssd1306_init_cmds); i++)
		display_cmd(ssd1306_init_cmds[i]);

	// Устанавливаем окно вывода на весь экран (столбцы 0..127, страницы 0..7)
    const uint8_t window_cmds[] = {
        0x21, 0x00, 0x7F, // Column Address: от 0 до 127
        0x22, 0x00, 0x07  // Page Address: от 0 до 7
    };

	for (uint32_t i = 0; i < sizeof(window_cmds); i++)
		display_cmd(window_cmds[i]);

	// Рисуем шахматную доску
	display_chessboard();

	while (1) {}
}
