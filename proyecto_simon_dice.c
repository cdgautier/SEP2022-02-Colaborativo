#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "xparameters.h"
#include "xgpio.h"
#include "xtmrctr.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"

// Parameter definitions
#define INTC_DEVICE_ID 		XPAR_PS7_SCUGIC_0_DEVICE_ID
#define TMR_DEVICE_ID		XPAR_TMRCTR_0_DEVICE_ID
#define LEDS_DEVICE_ID		XPAR_AXI_GPIO_LED_DEVICE_ID
#define BTNS_DEVICE_ID		XPAR_AXI_GPIO_BUTTONS_DEVICE_ID
#define SWTS_DEVICE_ID		XPAR_AXI_GPIO_SWITCHES_DEVICE_ID
#define INTC_TMR_INTERRUPT_ID XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR
#define INTC_GPIO_INTERRUPT_ID XPAR_FABRIC_AXI_GPIO_BUTTONS_IP2INTC_IRPT_INTR

#define BTN_INT 			XGPIO_IR_CH1_MASK
#define TMR_LOAD			(0xFFFFFFFF - 50000000*0.5) // 0.5s Timer

XGpio LEDInst;
XGpio BTNInst;
XScuGic INTCInst;
XTmrCtr TMRInst;

static int btn_value;         // recibe la señal del boton presionado
static int btn_press;	      // vale 1 si el boton fue presionado y almacena este valor
static int tmr_count;
static int led_counter;       // iterador del ciclo del semaforo
static int led_data;	      // valor qur imprime el led
static int max_counter = 15;  // valor en que se reinicia el ciclo del semaforo

//----------------------------------------------------
// PROTOTYPE FUNCTIONS
//----------------------------------------------------

static void BTN_Intr_Handler(void *baseaddr_p);
static void TMR_Intr_Handler(void *baseaddr_p);
static int InterruptSystemSetup(XScuGic *XScuGicInstancePtr);
static int IntcInitFunctionTmr(u16 DeviceId, XTmrCtr *TmrInstancePtr);
static int IntcInitFunctionBtn(u16 DeviceId, XGpio *GpioInstancePtr);

/*
// 1. GPIO

// 2. UART

// 3. Timers

// 4. Interrupciones

*/

/*
Flujo

1) SW0 -> enciende el juego
2) ingresar nombre en terminal vitis
3) verificar exiistencia de nombre
4) menu de botones op1: jugar, op2: ranking
5) juego inicia con secuencia de parpadeos
6) generacion y reproduccion de secuencia de leds
7) repeticion de secuencia con botones
8) analisis de la repeticion
9) secuencia de game over en caso de perder
10) enviar a consola tiempo de juego y rondas completadas + estructura de puntajes
11) fin del juego -> preguntar si (11.1) juega otra vez o si (11.2) ingresa otro usuario
		si elige (11.2) -> 2).
*/

/*

counter_inicio = 0;

if (not switch and (counter_inicio == 0)){

	printf("enciende el switch 0 para iniciar el juego");
	++counter_inicio;
}

while(switch){
	indice_switch = 1;
	while (indice_switch){
		printf("Bienvenido a simon dice!")
		printf("ingresa nombre de usuario")
		scanf() // nombre
		autentificacion = funcion_verificar_username();
		while(autentificacion){
			indice_flujo = 0;
			// btn_handler() depende de indice_flujo
			// if indice_flujo = 0 ->  btn_handler() muestra menu_inicio
			// if indice_flujo = 1 ->  btn_handler() almacenó seleccion_juego
			if seleccion_juego == 0001{
				ranking();
				indice_flujo = 0
			}
			while(seleccion_juego == 0010){
				counter_rondas = 0;
				indice_juego = 0;
				//tmr_handler() depende de indice_juego
				si indice_juego = 0 -> tmr_handler() ejecuta parpadeo de los cuatro LEDs por 2 segundos, con una frecuencia de medio segundo.
				tmr_handler(); 
				++indice_juego;
				si indice_juego = 1 -> tmr_handler() ejecuta secuencias a repetir de los LEDs con random;
				tmr_handler();
				btn_handler(); -> si indice_flujo = 1 y indice_juego = 1, btn_handler() escribe en seleccion_juego lo que recibe de los botones
				++indice_flujo; 
				

				if indice_flujo == 2{
					indice_juego = 0;
					tmr_handler() ejecuta parpadeo de los 4 LEDs tendr´an que parpadear 3 veces para indicar que se terminó el juego. Frecuencia de medio segundo,
					btn_handler() pasa a seleccion_postjuego
					si seleccion_postjuego = 01 -> mismo jugador, indice_flujo = 0, indice_juego = 0;
					si seleccion_postjuego = 10 -> otro jugador, autentificacion = 0, indice_flujo = 0, indice_juego = 0;
				}	
			}
		}
	}	
}



btn_handler(){

	if (indice_flujo = 0 and indice_juego = 0){
		funcion_menu_inicio(); -> muestra menu inicio y queda atento a seleccion_juego
	}
	else if (indice_flujo = 1 and indice_juego = 0){
		almacenó seleccion_juego 0001, 0010 
	}
	else if (indice_flujo = 1 and indice_juego = 1){
		se genera elemento random en el array con maloc
		suma_comparacion = 0;
		for(int i=0; i < len(array); ++i ){
			counter 30 seg...
			if (tiempo_vida < 30){
				if not (seleccion_juego == array[i]){
					printf(error en secuencia uwu)
					comparacion = 0;
					break;
				}
				else{
					++suma_comparacion;
					tiempo_vida = 0;
				}
			}
			else{
				printf(te quedaste sin tiempo uwu)
				comparacion = 0;
				break;
			}
		}
		if suma_comparacion = len(array){
			++counter_rondas;
		}
		if comparacion = 0{
			printf(game over)
			indice_juego = 0
			indice_flujo = 2
		}
	}
	else if (indice_flujo = 2 and indice_juego = 0){
		almacenó seleccion_postjuego 0001, 0010 
	}

tmr_handler(){
	if (indice_flujo = 1 and indice_juego = 0){
		muestra secuencia de inicio de leds
	}
	else if (indice_flujo = 1 and indice_juego = 1){
		ejecuta secuencias a repetir de los LEDs con random;
	}
	else if (indice_flujo = 2 and indice_juego = 0){
		ejecuta parpadeo de los 4 LEDs tendr´an que parpadear 3 veces para indicar que se terminó el juego. Frecuencia de medio segundo
	}
}

swt_handler(){
	switch-> sigue al switch analogico
}
			

*/