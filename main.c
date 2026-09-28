#include "stm32l476xx.h"
#include "SysClock.h"
#include "LED.h"
#include "UART.h"
#include "keypad.h"
#include "OLED_I2C.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include <stdint.h>
#include <stddef.h>

/*
 * Draw the keypad buffer on the 128x64 OLED.
 * Font_7x10 is 7 pixels wide and 10 pixels high, so we wrap after
 * 18 characters (18 * 7 = 126 pixels).
 */
static void OLED_ShowBuffer(const char *text)
{
    const uint8_t chars_per_line = 18U;
    const uint8_t line_height = 10U;
    const uint8_t max_lines = 6U;
    uint8_t line = 0U;
    uint8_t col = 0U;
    size_t i = 0U;
    char one_char[2];

    ssd1306_Fill(White);

    /* Show a prompt on the first line. */
    ssd1306_SetCursor(0U, 0U);
    ssd1306_WriteString("Hello!!", Font_11x18, Black);

    /* Start the entered text on the second line. */
    line = 2U;
    col = 0U;
    ssd1306_SetCursor(0U, (uint8_t)(line * line_height));

    while ((text[i] != '\0') && (line < max_lines)) {
        if (col >= chars_per_line) {
            line++;
            col = 0U;

            if (line >= max_lines) {
                break;
            }

            ssd1306_SetCursor(0U, (uint8_t)(line * line_height));
        }

        one_char[0] = text[i];
        one_char[1] = '\0';
        ssd1306_WriteString(one_char, Font_7x10, White);

        col++;
        i++;
    }

    /* Nothing appears on the physical OLED until this is called. */
    ssd1306_UpdateScreen();
}

int main(void)
{
    char chr;
    char str[64] = {0};
    uint32_t n = 0U;

    System_Clock_Init();
    LED_Init();
    UART2_Init();

    /* Initialize the 4x4 keypad on GPIOC. */
    Keypad_Pin_Init();

    /* Initialize SSD1306 on I2C1: PB8 = SCL, PB9 = SDA. */
    OLED_I2C_Init();
    ssd1306_Init();

    /* If the OLED does not acknowledge, stop here with LD2 ON. */
    if (OLED_I2C_GetLastStatus() != OLED_I2C_OK) {
        LED_On();
        while (1) {
        }
    }

    OLED_ShowBuffer(str);

    while (1) {
        /* keypad_scan() waits here until a key is pressed and released. */
        chr = (char)
				keypad_scan(); //call the keypad scan function here

      //if scanned character is a * delete the latest entry
			if (chr == '*') {
            if (n > 0U ) { //check if the buffer (string) is not empty
                n--;
                str[n] = '\0';
            }

             //use OLED function write to the oled screen
						OLED_ShowBuffer(str);
            //USART_Write(USART2, (uint8_t *)str, n);  //to write to Screen using UART
            LED_Toggle(); //toggle the led on/off
        }
        else if ((unsigned char)chr != 0xFFU) {
            if (n < (sizeof(str) - 1U)) {
                str[n] = chr ; //strore the scanned character to the string
                n++; //change the counter here once the character is added to the buffer
                str[n] = '\0';
            }

            //Use the OLED function to write to the screen
						OLED_ShowBuffer(str);
            //USART_Write(USART2, (uint8_t *)str, n); //to write to screen using UART
            LED_Toggle();
        }
    }
}
