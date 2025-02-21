#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "matriz_leds.h"
#include "inc/ssd1306.h"
#include "lib_utils.h"

// Definição de variaveis globais e macros

#define btnA_pin 5
#define btnB_pin 6
#define buzzerA_pin 21

// Pinos para o display
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

// Variáveis para tratamento de debounce
static volatile uint32_t last_time_a = 0;
static volatile uint32_t last_time_b = 0;


// Função para inicializar o display OLED
struct render_area start(){
    // Inicialização do i2c
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Processo de inicialização completo do OLED SSD1306
    ssd1306_init();

    // Preparar área de renderização para o display (ssd1306_width pixels por ssd1306_n_pages páginas)
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    return frame_area;
}


int main()
{
    stdio_init_all();

    // Quando iniciado o sistema, imprimir uma mensagem
    

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
