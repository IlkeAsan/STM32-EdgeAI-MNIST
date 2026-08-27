/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Baremetal AI, UART & I2C LCD (16x2) for MNIST
 ******************************************************************************
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "app_x-cube-ai.h"

/* ========================================================================== */
/* 1. DONANIM REGISTER ADRESLERİ (BAREMETAL)                                  */
/* ========================================================================== */

// RCC (Reset and Clock Control)
#define RCC_AHB1ENR     (*(volatile uint32_t *)0x40023830UL) // GPIOB, GPIOD Saat
#define RCC_APB1ENR     (*(volatile uint32_t *)0x40023840UL) // USART3, I2C1 Saat

// GPIOB: LED'ler (PB0, PB7, PB14) ve I2C1 LCD (PB8=SCL, PB9=SDA)
#define GPIOB_MODER     (*(volatile uint32_t *)0x40020400UL)
#define GPIOB_OTYPER    (*(volatile uint32_t *)0x40020404UL)
#define GPIOB_PUPDR     (*(volatile uint32_t *)0x4002040CUL)
#define GPIOB_ODR       (*(volatile uint32_t *)0x40020414UL)
#define GPIOB_AFRH      (*(volatile uint32_t *)0x40020424UL)

// GPIOD: UART3 ST-Link (PD8=TX, PD9=RX)
#define GPIOD_MODER     (*(volatile uint32_t *)0x40020C00UL)
#define GPIOD_AFRH      (*(volatile uint32_t *)0x40020C24UL)

// USART3: Bilgisayar USB Seri Port
#define USART3_SR       (*(volatile uint32_t *)0x40004800UL)
#define USART3_DR       (*(volatile uint32_t *)0x40004804UL)
#define USART3_BRR      (*(volatile uint32_t *)0x40004808UL)
#define USART3_CR1      (*(volatile uint32_t *)0x4000480CUL)

// I2C1: 16x2 I2C LCD Ekran (PCF8574)
#define I2C1_CR1        (*(volatile uint32_t *)0x40005400UL)
#define I2C1_CR2        (*(volatile uint32_t *)0x40005404UL)
#define I2C1_DR         (*(volatile uint32_t *)0x40005410UL)
#define I2C1_SR1        (*(volatile uint32_t *)0x40005414UL)
#define I2C1_SR2        (*(volatile uint32_t *)0x40005418UL)
#define I2C1_CCR        (*(volatile uint32_t *)0x4000541CUL)
#define I2C1_TRISE      (*(volatile uint32_t *)0x40005420UL)


/* ========================================================================== */
/* 2. LCD (PCF8574) VE ZAMANLAMA TANIMLARI                                    */
/* ========================================================================== */
#define LCD_ADDR          0x4E // 0x27 << 1 (Standart I2C LCD Adresi)
#define LCD_RS_PIN        (1 << 0)
#define LCD_EN_PIN        (1 << 2)
#define LCD_BACKLIGHT_PIN (1 << 3)

void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms * 3000; i++) {
        __asm("NOP");
    }
}


/* ========================================================================== */
/* 3. LED FONKSİYONLARI                                                       */
/* ========================================================================== */
void led_init(void) {
    RCC_AHB1ENR |= (1U << 1); // GPIOB Saat
    GPIOB_MODER |= (1U << 0) | (1U << 14) | (1U << 28); // PB0, PB7, PB14 Çıkış
    GPIOB_ODR &= ~((1U << 0) | (1U << 7) | (1U << 14)); // Söndür
}

void led_yesil(bool yak)   { if(yak) GPIOB_ODR |= (1U << 0);  else GPIOB_ODR &= ~(1U << 0);  }
void led_mavi(bool yak)    { if(yak) GPIOB_ODR |= (1U << 7);  else GPIOB_ODR &= ~(1U << 7);  }
void led_kirmizi(bool yak) { if(yak) GPIOB_ODR |= (1U << 14); else GPIOB_ODR &= ~(1U << 14); }


/* ========================================================================== */
/* 4. UART3 (ST-LINK USB) FONKSİYONLARI                                       */
/* ========================================================================== */
void uart3_init(void) {
    RCC_AHB1ENR |= (1U << 3);  // GPIOD
    RCC_APB1ENR |= (1U << 18); // USART3

    // PD8, PD9 -> Alternate Function AF7 (USART3)
    GPIOD_MODER &= ~((3U << 16) | (3U << 18));
    GPIOD_MODER |= ((2U << 16) | (2U << 18));
    GPIOD_AFRH &= ~((15U << 0) | (15U << 4));
    GPIOD_AFRH |= ((7U << 0) | (7U << 4));

    USART3_BRR = 139; // 115200 Baud @ 16MHz
    USART3_CR1 = (1U << 13) | (1U << 3) | (1U << 2);
}

void uart_send(uint8_t c) {
    while (!(USART3_SR & (1U << 7)));
    USART3_DR = c;
}

uint8_t uart_receive(void) {
    while (!(USART3_SR & (1U << 5)));
    return (uint8_t)(USART3_DR & 0xFF);
}


/* ========================================================================== */
/* 5. I2C1 VE 16x2 LCD SÜRÜCÜSÜ (PB8=SCL, PB9=SDA)                           */
/* ========================================================================== */
void I2C1_Init(void) {
    RCC_AHB1ENR |= (1U << 1);  // GPIOB
    RCC_APB1ENR |= (1U << 21); // I2C1

    // PB8, PB9 -> Alternate Function AF4 (I2C1), Open-Drain, Pull-up
    GPIOB_MODER &= ~((3U << 16) | (3U << 18));
    GPIOB_MODER |=  ((2U << 16) | (2U << 18));
    GPIOB_OTYPER |= (1U << 8) | (1U << 9);
    GPIOB_PUPDR &= ~((3U << 16) | (3U << 18));
    GPIOB_PUPDR |=  ((1U << 16) | (1U << 18));

    // AFRH: Pin 8 -> AF4 (bit 0-3), Pin 9 -> AF4 (bit 4-7)
    GPIOB_AFRH &= ~((15U << 0) | (15U << 4));
    GPIOB_AFRH |=  ((4U << 0) | (4U << 4));

    // I2C1 100 kHz Standart Mod Ayarları
    I2C1_CR1 |= (1U << 15); // Reset
    I2C1_CR1 &= ~(1U << 15);
    I2C1_CR2 = 16;          // 16 MHz APB1
    I2C1_CCR = 80;          // 100 kHz
    I2C1_TRISE = 17;
    I2C1_CR1 |= (1U << 0);  // I2C Enable (PE)
}

void LCD_Write_I2C(uint8_t data) {
    I2C1_CR1 |= (1U << 8); // START
    while (!(I2C1_SR1 & (1U << 0)));

    I2C1_DR = LCD_ADDR;
    while (!(I2C1_SR1 & (1U << 1)));
    (void)I2C1_SR2;

    while (!(I2C1_SR1 & (1U << 7)));
    I2C1_DR = data;
    while (!(I2C1_SR1 & (1U << 2)));

    I2C1_CR1 |= (1U << 9); // STOP
}

void LCD_Send_Nibble(uint8_t nibble, uint8_t rs) {
    uint8_t data = (nibble & 0xF0) | LCD_BACKLIGHT_PIN;
    if (rs) data |= LCD_RS_PIN;

    LCD_Write_I2C(data | LCD_EN_PIN);
    delay_ms(1);
    LCD_Write_I2C(data);
    delay_ms(1);
}

void LCD_Send(uint8_t data, uint8_t is_data) {
    LCD_Send_Nibble(data & 0xF0, is_data);
    LCD_Send_Nibble((data << 4) & 0xF0, is_data);
}

void LCD_SendCommand(uint8_t cmd)  { LCD_Send(cmd, 0); }
void LCD_SendData(uint8_t data)    { LCD_Send(data, 1); }

void LCD_Init(void) {
    delay_ms(50);
    LCD_Send_Nibble(0x30, 0); delay_ms(5);
    LCD_Send_Nibble(0x30, 0); delay_ms(1);
    LCD_Send_Nibble(0x30, 0); delay_ms(1);
    LCD_Send_Nibble(0x20, 0); delay_ms(1);

    LCD_SendCommand(0x28); // 4-bit, 2 Satır, 5x8
    LCD_SendCommand(0x0C); // Ekran Açık, İmleç Kapalı
    LCD_SendCommand(0x01); // Temizle
    delay_ms(2);
    LCD_SendCommand(0x06); // Giriş modu
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t address = (row == 0) ? (0x80 + col) : (0xC0 + col);
    LCD_SendCommand(address);
}

void LCD_Print(const char *str) {
    while (*str) {
        LCD_SendData((uint8_t)*str);
        str++;
    }
}


/* ========================================================================== */
/* 6. MODEL SINIFLARI                                                         */
/* ========================================================================== */
static const char CLASS_LABELS[36] = {
    '0','1','2','3','4','5','6','7','8','9',
    'A','B','C','D','E','F','G','H','I','J',
    'K','L','M','N','O','P','Q','R','S','T',
    'U','V','W','X','Y','Z'
};


/* ========================================================================== */
/* 7. ANA PROGRAM (MAIN)                                                      */
/* ========================================================================== */
int main(void)
{
    // 0. FPU Donanımını Aç
    (*(volatile uint32_t *)0xE000ED88) |= ((3UL << 10*2) | (3UL << 11*2));

    // Donanımları Başlat
    led_init();
    uart3_init();
    I2C1_Init();
    LCD_Init();

    // 1. LCD Açılış Mesajı
    LCD_SetCursor(0, 0);
    LCD_Print("STM32 EDGE AI");
    LCD_SetCursor(1, 0);
    LCD_Print("Model Baslatildi");

    // 2. Yapay Zekayı Başlat
    STM32CubeAI_Studio_AI_Init();
    
    float* ai_in = STM32CubeAI_GetInputBuffer();
    float* ai_out = STM32CubeAI_GetOutputBuffer();

    led_yesil(true); // Sistem hazır

    delay_ms(1000);
    LCD_SetCursor(1, 0);
    LCD_Print("Kamera Bekleniyor");

    // 3. Sonsuz Döngü
    while (1)
    {
        // Bilgisayardan 784 Bayt (28x28) Oku
        for (int i = 0; i < 784; i++) {
            uint8_t piksel = uart_receive();
            ai_in[i] = ((float)piksel / 127.5f) - 1.0f; // [-1.0, +1.0]
        }

        led_mavi(true);

        // LCD'ye "Hesaplaniyor..." yaz
        LCD_SetCursor(1, 0);
        LCD_Print("Hesaplaniyor... ");

        // Yapay Zekayı Çalıştır
        STM32CubeAI_Studio_AI_Process();

        // ArgMax - 36 Sınıf Arasından En Yüksek Olasılığı Bul
        int best_idx = 0;
        float max_prob = ai_out[0];
        for (int i = 1; i < 36; i++) {
            if (ai_out[i] > max_prob) {
                max_prob = ai_out[i];
                best_idx = i;
            }
        }

        char sonuc = CLASS_LABELS[best_idx];

        // Bilgisayara Gönder
        uart_send(sonuc);

        // LCD Ekrana Sonucu ve Olasılığı Bas
        LCD_SetCursor(1, 0);
        char buffer[17];
        int yuzde = (int)(max_prob * 100.0f);
        if (yuzde > 99) yuzde = 99;
        if (yuzde < 0) yuzde = 0;
        
        snprintf(buffer, sizeof(buffer), "Tahmin: %c (%d%%) ", sonuc, yuzde);
        LCD_Print(buffer);

        // Kırmızı LED ile Bildir
        if (best_idx >= 1 && best_idx <= 9) {
            for (int k = 0; k < best_idx; k++) {
                led_kirmizi(true);
                for(volatile int delay = 0; delay < 200000; delay++);
                led_kirmizi(false);
                for(volatile int delay = 0; delay < 200000; delay++);
            }
        } else if (best_idx == 0) {
            led_kirmizi(true);
            for(volatile int delay = 0; delay < 800000; delay++);
            led_kirmizi(false);
        }
        
        led_mavi(false);
    }

    return 0;
}
