#include <stdint.h>
#include <stm32f10x.h>

// Переменная для хранения текущей задержки (в тиках)
// Начальное значение из условия 1 Гц
volatile uint32_t current_delay_ticks = 1000000;

void delay(uint32_t ticks) {
	for (int i=0; i<ticks; i++) {
		__NOP();
	}
}

int __attribute((noreturn)) main(void) {
	// Тактирование портов A и C

	// Для кнопок
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	// Для светодиода
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

	// Настройка порта PC13

	GPIOC->CRH &= ~GPIO_CRH_CNF13; //clear cnf bits
	GPIOC->CRH |= GPIO_CRH_MODE13_0; //Max speed = 10Mhz

	// Настройка PA0 и PA1 (кнопки)

	GPIOA->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
	GPIOA->CRL |= GPIO_CRL_CNF0_1; // Input with pull-up / pull-down
	GPIOA->ODR |= (1U << 0); // Pull-up для PA0

	GPIOA->CRL &= ~(GPIO_CRL_CNF1 | GPIO_CRL_MODE1);
	GPIOA->CRL |= GPIO_CRL_CNF1_1; // Input with pull-up / pull-down
	GPIOA->ODR |= (1U << 1); // Pull-up для PA1

	while (1) {
		// Если кнопка A (PA0) нажата (уровень 0 из-за Pull-up)
		if (!(GPIOA->IDR & (1U << 0))) {
			delay(50000); // Антидребезг
			if (!(GPIOA->IDR & (1U << 0))) {
				// Увеличиваем частоту в допустимых пределах
				if (current_delay_ticks < 64000000) {
					current_delay_ticks *= 2;
				}
				// Ждём, пока кнопку отпустят
				while (!(GPIOA->IDR & (1U << 0)));
			}
		}

		// Если кнопка B (PA1) нажата

		if (!(GPIOA->IDR & (1U << 1))) {
			delay(50000);
			if (!(GPIOA->IDR & (1U << 1))) {
				// Уменьшаем частоту в допустимых пределах
				if (current_delay_ticks > 15625) {
					current_delay_ticks /= 2;
				}
				while (!(GPIOA->IDR & (1U << 1)));
			}
		}

		// Мигание светодиодом

		// Включить (для Blue Pill 0 = ВКЛ)
		GPIOC->ODR &= ~(1U << 13U);
		delay(current_delay_ticks);

		// ВЫключить (для Blue Pill 1 = ВЫКЛ)
		GPIOC->ODR |= (1U << 13U);
		delay(current_delay_ticks);
	}
}
