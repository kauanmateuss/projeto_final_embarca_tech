#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "matriz_leds.h"
#include "inc/ssd1306.h"
#include "string.h"
#include <stdlib.h>
#include <time.h>
#include "hardware/timer.h"

// Definição de variaveis globais e macros

#define btnA_pin 5
#define btnB_pin 6
#define buzzerA_pin 21
#define btn_joy_pin 22

PIO pio;
uint sm;

volatile uint numero_sorteado = 0; 
volatile uint contador = 0;
volatile bool inicio_jogo = false;
volatile uint tentativas = 0;  

volatile bool tocar_tom_inicio = false;    // flag para tocar o som fora da interrupção
volatile bool tocar_tom_vitoria = false;
volatile bool tocar_tom_erro = false;

// Pinos para o display
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

// Variáveis para tratamento de debounce
static volatile uint32_t last_time_a = 0;
static volatile uint32_t last_time_b = 0;
static volatile uint32_t last_time_joy = 0;

// matrizes dos numeros para o display
Matriz_leds_config zero = {
    //   Coluna 0         Coluna 1         Coluna 2         Coluna 3         Coluna 4
    // R    G    B      R    G    B      R    G    B      R    G    B      R    G    B
    {{0.0, 0.0, 0.0}, {0.2, 0.2, 0.2}, {0.2, 0.2, 0.2}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}}, // Linha 0
    {{0.0, 0.0, 0.0}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}}, // Linha 1
    {{0.0, 0.0, 0.0}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}}, // Linha 2
    {{0.0, 0.0, 0.0}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}}, // Linha 3
    {{0.0, 0.0, 0.0}, {0.2, 0.2, 0.2}, {0.2, 0.2, 0.2}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}}, // Linha 4
};

Matriz_leds_config um = {
    //   Coluna 0         Coluna 1         Coluna 2         Coluna 3         Coluna 4
    // R    G    B      R    G    B      R    G    B      R    G    B      R    G    B
    {{0.0, 0.0, 0.0}, {0.0, 0.5, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 0
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 1
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 2
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 3
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 4
};

Matriz_leds_config dois = {
    //   Coluna 0         Coluna 1         Coluna 2         Coluna 3         Coluna 4
    // R    G    B      R    G    B      R    G    B      R    G    B      R    G    B
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 0
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 1
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 2
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 3
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 4
};
Matriz_leds_config tres = {
    //   Coluna 0         Coluna 1         Coluna 2         Coluna 3         Coluna 4
    // R    G    B      R    G    B      R    G    B      R    G    B      R    G    B
    {{0.0, 0.0, 0.0}, {0.2, 0.2, 0.2}, {0.2, 0.2, 0.2}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}}, // Linha 0
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}}, // Linha 1
    {{0.0, 0.0, 0.0}, {0.2, 0.2, 0.2}, {0.2, 0.2, 0.2}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}}, // Linha 2
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}}, // Linha 3
    {{0.0, 0.0, 0.0}, {0.2, 0.2, 0.2}, {0.2, 0.2, 0.2}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}}, // Linha 4
};

Matriz_leds_config quatro = {
    //   Coluna 0         Coluna 1         Coluna 2         Coluna 3         Coluna 4
    // R    G    B      R    G    B      R    G    B      R    G    B      R    G    B
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 0
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 1
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 2
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 3
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 4
};

Matriz_leds_config cinco = {
    //   Coluna 0         Coluna 1         Coluna 2         Coluna 3         Coluna 4
    // R    G    B      R    G    B      R    G    B      R    G    B      R    G    B
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 0
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 1
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 2
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 3
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 4
};

Matriz_leds_config seis = {
    //   Coluna 0         Coluna 1         Coluna 2         Coluna 3         Coluna 4
    // R    G    B      R    G    B      R    G    B      R    G    B      R    G    B
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 0
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 1
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 2
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 3
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 4
};

Matriz_leds_config sete = {
    //   Coluna 0         Coluna 1         Coluna 2         Coluna 3         Coluna 4
    // R    G    B      R    G    B      R    G    B      R    G    B      R    G    B
    {{0.0, 0.0, 0.0}, {0.2, 0.05, 0.2}, {0.2, 0.2, 0.2}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}}, // Linha 0
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}}, // Linha 1
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}}, // Linha 2
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}}, // Linha 3
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.2, 0.2, 0.2}, {0.0, 0.0, 0.0}}, // Linha 4
};

Matriz_leds_config oito = {
    //   Coluna 0         Coluna 1         Coluna 2         Coluna 3         Coluna 4
    // R    G    B      R    G    B      R    G    B      R    G    B      R    G    B
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 0
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 1
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 2
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 3
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 4
};

Matriz_leds_config nove = {
    //   Coluna 0         Coluna 1         Coluna 2         Coluna 3         Coluna 4
    // R    G    B      R    G    B      R    G    B      R    G    B      R    G    B
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 0
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 1
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 2
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 3
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 4
};

Matriz_leds_config carinha_feliz = {
    //   Coluna 0         Coluna 1         Coluna 2         Coluna 3         Coluna 4
    // R    G    B      R    G    B      R    G    B      R    G    B      R    G    B
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 0
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 1
    {{0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}}, // Linha 2
    {{0.0, 0.0, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.05, 0.0}, {0.0, 0.0, 0.0}}, // Linha 3
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 4
};


Matriz_leds_config vermelho = {
    //   Coluna 0         Coluna 1         Coluna 2         Coluna 3         Coluna 4
    // R    G    B      R    G    B      R    G    B      R    G    B      R    G    B
    {{0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}}, // Linha 0
    {{0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}}, // Linha 1
    {{0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}}, // Linha 2
    {{0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}}, // Linha 3
    {{0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}, {0.05, 0.0, 0.0}}, // Linha 4
};


Matriz_leds_config clear = {
    //   Coluna 0         Coluna 1         Coluna 2         Coluna 3         Coluna 4
    // R    G    B      R    G    B      R    G    B      R    G    B      R    G    B
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 0
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 1
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 2
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 3
    {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 4
};

Matriz_leds_config *numeros[] = {&zero, &um, &dois, &tres, &quatro, &cinco, &seis, &sete, &oito, &nove};

// Mensagens que vão aparecer no display
char *inicio[8] = {
    "===============",
    "               ",
    "               ",
    "     JOGO      ",
    "               ",
    "  ADIVINHACAO  ",
    "               ",
    "==============="
};

char *aperte[8] = {
    "===============",
    "=             =",
    "=   PRECIONE  =",
    "=      O      =",
    "=    BOTAO    =",
    "= DO JOYSTICK =",
    "=             =",
    "==============="
};

char *sorteio[8] = {
    "===============",
    "=             =",
    "=    NUMERO   =",
    "=      DE     =",
    "=    0 A 9    =",
    "=   SORTEADO  =",
    "=             =",
    "==============="
};

char *aperteA[8] = {
    "===============",
    "=BOTAO A:MUDAR=",
    "=NUMERO       =",
    "=             =",
    "=BOTAO B:     =",
    "=CONFIRMA O   =",
    "=NUMERO       =",
    "==============="
};

char *aperteB[8] = {
    "===============",
    "=             =",
    "= PRECIONE O  =",
    "= BOTAO B PARA=",
    "= ESCOLHER O  =",
    "= NUMERO      =",
    "=             =",
    "==============="
};


char *iniciado[8] = {
    "===============",
    "=             =",
    "=   PARTIDA   =",
    "=             =",
    "=  INICIADA!  =",
    "=             =",
    "=             =",
    "==============="
};

char *maior[8] = {
    "===============",
    "=             =",
    "=    NUMERO   =",
    "=             =",
    "=    MAIOR!   =",
    "=             =",
    "=             =",
    "==============="
};

char *acertou[8] = {
    "===============",
    "=             =",
    "=   PARABENS  =",
    "=             =",
    "=   ACERTOU!  =",
    "=             =",
    "=             =",
    "==============="
};

char *menor[8] = {
    "===============",
    "=             =",
    "=    NUMERO   =",
    "=             =",
    "=    MENOR!   =",
    "=             =",
    "=             =",
    "==============="
};

// Função callback do temporizador para exibir mensagens
bool mensagem_inicio_callback(struct repeating_timer *){
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area);

    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    // Imprime mensagens
    int y = 0;
    for(uint i = 0; i < count_of(aperte); i++){
        ssd1306_draw_string(ssd, 5, y, aperte[i]);
        y += 8;
    }
    render_on_display(ssd, &frame_area);

    return false;  // para execultar só uma vez
}

// Função callback para temporizador
bool menssagem_aperteA_callback(struct repeating_timer *){
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area);

    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    // Imprime mensagens
    int y = 0;
    for(uint i = 0; i < count_of(aperteA); i++){
        ssd1306_draw_string(ssd, 5, y, aperteA[i]);
        y += 8;
    }
    render_on_display(ssd, &frame_area);

    return false;  // para execultar só uma vez
}


// funções para buzzer

// Configura o PWM para buzzer
void inicia_buzzer(){
    gpio_set_function(buzzerA_pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(buzzerA_pin);
    // iniciar com o pwm desligado
    pwm_set_enabled(slice_num, false);
}

// Função para tocar um som
void tocar_tom(uint16_t frequencia, uint16_t duracao);

// Funções das melodias
void beep_inicio(){
    tocar_tom(880, 150);
    tocar_tom(660, 150);
    tocar_tom(990, 200);
    sleep_ms(100);
    tocar_tom(1100, 250); // Finaliza com tom alto e alegre
    sleep_ms(200);
    tocar_tom(660, 100);
    tocar_tom(880, 300);
}

void beep_vitoria(){
    tocar_tom(880, 150);
    tocar_tom(988, 150);
    tocar_tom(1046, 150);
    tocar_tom(1174, 200);
    sleep_ms(100);
    tocar_tom(1568, 250);
}

void beep_erro(){
    tocar_tom(800, 100);
    tocar_tom(600, 100);
    tocar_tom(700, 150);
}

bool timer_callback(struct repeating_timer *){
    // verificando qual o som tocar
    if(tocar_tom_inicio){
        beep_inicio();
        tocar_tom_inicio = false;
    }

    if(tocar_tom_vitoria){
        beep_vitoria();
        tocar_tom_vitoria = false;
    }

    if(tocar_tom_erro){
        beep_erro();
        tocar_tom_erro = false;
    }

    return false;
}

// Protótipos das funções
void btn_pressed(uint gpio, uint32_t events);


int main()
{
    stdio_init_all();
    srand(time(NULL));

    // Inicializa a semente do gerador de numeros aleatórios
    srand(time(NULL));

    // Inicialização das variáveis para trabalhar com a matriz de leds
    pio = pio0;
    sm = configurar_matriz(pio);

    // Inicialização dos gpios
    gpio_init(btnA_pin);
    gpio_set_dir(btnA_pin, GPIO_IN);
    gpio_init(btnB_pin);
    gpio_set_dir(btnB_pin, GPIO_IN);
    gpio_init(btn_joy_pin);
    gpio_set_dir(btn_joy_pin, GPIO_IN);

    gpio_pull_up(btnA_pin);
    gpio_pull_up(btnB_pin);
    gpio_pull_up(btn_joy_pin);

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

    // zera o display inteiro
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);
    
    
    imprimir_desenho(zero, pio, sm);

    while (true) {
        // Imprimir a mensagem de inicio
        int y = 0;
        for (uint i = 0; i < count_of(inicio); i++)
        {
            ssd1306_draw_string(ssd, 5, y, inicio[i]);
            y += 8;
        }
        render_on_display(ssd, &frame_area);

        sleep_ms(2000);
        
        // Mensagem pra apertar o joy
        y = 0;
        for (uint i = 0; i < count_of(aperte); i++)
        {
            ssd1306_draw_string(ssd, 5, y, aperte[i]);
            y += 8;
        }
        render_on_display(ssd, &frame_area);

        // só vai passar do laço se clicar no botão
        while(!gpio_get(btn_joy_pin)){
            sleep_ms(100);  // pra não sobrecarregar
        }

        // quando apertar o botão, sortear o numero, mostrar as mensagens e tocar o som
        numero_sorteado = rand() % 10;

        beep_inicio();
        y = 0;
        for(uint i = 0; i < count_of(sorteio); i++){
            ssd1306_draw_string(ssd, 5, y, sorteio[i]);
            y += 8;
        }
        render_on_display(ssd, &frame_area);

        // Mensagem de orientação dos botões
        int y = 0;
        for(uint i = 0; i < count_of(aperteA); i++){
            ssd1306_draw_string(ssd, 5, y, aperteA[i]);
            y += 8;
        }
        render_on_display(ssd, &frame_area);

        // Laço para implementar a lógica
        while(true){
            // verificando se clicou no botão
            
        }



        sleep_ms(100);
    }
}   // ============================== FIM MAIN ===========================


// Corpos das funções

void btn_pressed(uint gpio, uint32_t events){
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    struct repeating_timer timer_msg;
    struct repeating_timer timer_tom;

    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area);

    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    // debounce
    if(gpio == btn_joy_pin && (current_time - last_time_joy) > 200){
        last_time_joy = current_time;
        inicio_jogo = true;
    
        // Gerar um novo numero aleatorio
        numero_sorteado = rand() % 10;

        // Tocar musica de inicio
        tocar_tom_inicio = true;
        add_repeating_timer_ms(100, timer_callback, NULL, &timer_tom);

        int y = 0;
        for(uint i = 0; i < count_of(sorteio); i++){
            ssd1306_draw_string(ssd, 5, y, sorteio[i]);
            y += 8;
        }
        render_on_display(ssd, &frame_area);

        // Chama o temporizador com a mensagem
        add_repeating_timer_ms(2000, menssagem_aperteA_callback, NULL, &timer_msg);

    }
    // Debounce btn A
    else if(gpio == btnA_pin && (current_time - last_time_a) > 200){
        last_time_a = current_time;
        if(!inicio_jogo) return;

        // Incrementa o contador
        contador = (contador + 1) % 10;
    }
    // debounce btn B
    else if(gpio == btnB_pin && (current_time - last_time_b) > 200){
        last_time_b = current_time;
        struct repeating_timer timer;

        tentativas++;

        // caso seja pressionado sem ter inicado a partida, retornar
        if(!inicio_jogo) return;
        
        
        // verificando o numero que o usuario escolheu com o sorteado
        if(contador == numero_sorteado){
            
            // tocar o tom da vitoria
            tocar_tom_vitoria = true;
            add_repeating_timer_ms(100, timer_callback, NULL, &timer_tom);

            // Imprime a mensagem que ganhou
            int y = 0;
            for(uint i = 0; i < count_of(acertou); i++){
                ssd1306_draw_string(ssd, 5, y, acertou[i]);
                y += 8;
            }
            render_on_display(ssd, &frame_area);

            printf("VOCE GANHOU\n");

            add_repeating_timer_ms(3000, mensagem_inicio_callback, NULL, &timer);

            inicio_jogo = false;
        }
        else if (contador > numero_sorteado){

            // toca o tom do erro
            tocar_tom_erro = true;
            add_repeating_timer_ms(100, timer_callback, NULL, &timer_tom);

            // Imprime a mensagem que é o numero é menor
            int y = 0;
            for(uint i = 0; i < count_of(menor); i++){
                ssd1306_draw_string(ssd, 5, y, menor[i]);
                y += 8;
            }
            render_on_display(ssd, &frame_area);

            printf("NUMERO MENOR\n");
        }
        else{
            // tocar tom erro
            tocar_tom_erro = true;
            add_repeating_timer_ms(100, timer_callback, NULL, &timer_tom);

            // Imprime a mensagem que é o numero é menor
            int y = 0;
            for(uint i = 0; i < count_of(maior); i++){
                ssd1306_draw_string(ssd, 5, y, maior[i]);
                y += 8;
            }
            render_on_display(ssd, &frame_area);

            printf("NUMERO MAIOR\n");
        }
    }

    imprimir_desenho(*numeros[contador], pio, sm);
}

void tocar_tom(uint16_t frequencia, uint16_t duracao){
    if(frequencia == 0){
        pwm_set_enabled(pwm_gpio_to_slice_num(buzzerA_pin), false);
        sleep_ms(duracao);
        return;
    }

    uint slice_num = pwm_gpio_to_slice_num(buzzerA_pin);
    float clock_div = 4.0f;
    uint32_t wrap = (125000000 / (clock_div * frequencia)) - 1;

    pwm_set_clkdiv(slice_num, clock_div);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_gpio_level(buzzerA_pin, wrap / 2);
    
    pwm_set_enabled(slice_num, true);
    sleep_ms(duracao);
    pwm_set_enabled(slice_num, false);
}

