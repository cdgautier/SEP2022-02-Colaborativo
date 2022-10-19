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
#define SWTS_DEVICE_ID    XPAR_AXI_GPIO_SWITCHES_DEVICE_ID
#define INTC_TMR_INTERRUPT_ID XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR
#define INTC_GPIO_BTN_INTERRUPT_ID XPAR_FABRIC_AXI_GPIO_BUTTONS_IP2INTC_IRPT_INTR
#define INTC_GPIO_SWT_INTERRUPT_ID XPAR_FABRIC_AXI_GPIO_SWITCHES_IP2INTC_IRPT_INTR

#define BTN_INT 			XGPIO_IR_CH1_MASK
#define SWT_INT 			XGPIO_IR_CH2_MASK
#define TMR_LOAD			(0xFFFFFFFF - 50000000*0.05) // 0.05s Timer

XGpio LEDInst;
XGpio BTNInst;
XGpio SWTInst;
XScuGic INTCInst;
XTmrCtr TMRInst;

static int btn_value;         // recibe la señal del boton presionado
static int swt_value;         // recibe la señal del switch presionado
static int select_game = 0b0000;	      // vale 1 si el boton fue presionado y almacena este valor
static int swt_on = 0;
static int tmr_count;
static int led_counter;       // iterador del ciclo del semaforo
static int led_data;	      // valor qur imprime el led
static int max_counter = 15;  // valor en que se reinicia el ciclo del semaforo
static int index_flux = 0;
static int index_game = 0;
static int stand_by = 1;
static int comparation_sum = 0;
//----------------------------------------------------
// PROTOTYPE FUNCTIONS
//----------------------------------------------------

static void TMR_Intr_Handler(void *baseaddr_p);
static void BTN_Intr_Handler(void *baseaddr_p);
static void SWT_Intr_Handler(void *baseaddr_p);
static int InterruptSystemSetup(XScuGic *XScuGicInstancePtr);
static int IntcInitFunctionTmr(u16 DeviceId, XTmrCtr *TmrInstancePtr);
static int IntcInitFunctionBtn(u16 DeviceId, XGpio *GpioInstancePtr);
static int IntcInitFunctionSwt(u16 DeviceId, XGpio *GpioInstancePtr);

void instructions() 
{
	printf("\n * * * * Bienvenido a Simon Dice! * * * * \n\n");
	printf(" COMPLETAR AQUI CON EL MENU DE INICIO \n");
	printf(" * * * * * * * *  * * * * * * \n\n");
}


//----------------------------------------------------
// INTERRUPT HANDLER FUNCTIONS
// - called by the timer, button interrupt, performs
// - LED flashing
//----------------------------------------------------

/*
swt_handler(){
	switch-> sigue al switch analogico
}
*/

void SWT_Intr_Handler(void *InstancePtr){
	// Disable GPIO interrupts
	XGpio_InterruptDisable(&SWTInst, SWT_INT);
	// Ignore additional button presses
	if ((XGpio_InterruptGetStatus(&SWTInst) & SWT_INT) !=
			SWT_INT) {
			return;
		}
	// recibe el boton y cambia swt_on
	swt_value = XGpio_DiscreteRead(&SWTInst, 1);
	if (swt_value == 1){
		*(&swt_on) = 1;
	}

	(void)XGpio_InterruptClear(&SWTInst, SWT_INT);
	// Enable GPIO interrupts
	XGpio_InterruptEnable(&SWTInst, SWT_INT);
}

/*
btn_handler(){

	if (index_flux = 0 and index_game = 0){
		funcion_menu_inicio(); -> muestra menu inicio y queda atento a select_game
	}
	else if (index_flux = 1 and index_game = 0){
		almacenó select_game 0001, 0010 
	}
	else if (index_flux = 1 and index_game = 1){
		guarda seleccion de secuencia i-esimo
*/

void BTN_Intr_Handler(void *InstancePtr)
{
	// Disable GPIO interrupts
	XGpio_InterruptDisable(&BTNInst, BTN_INT);
	// Ignore additional button presses
	if ((XGpio_InterruptGetStatus(&BTNInst) & BTN_INT) !=
			BTN_INT) {
			return;
		}
	
	if (index_flux == 0 && index_game == 0){
		instructions();
		btn_value = XGpio_DiscreteRead(&BTNInst, 1);
		if (btn_value == 1){
			*(&select_game) = 0001;
		}
		else if (btn_value == 2){
			*(&select_game) = 0010;
		}
	}
	else if (index_flux == 1 && index_game == 1){
		btn_value = XGpio_DiscreteRead(&BTNInst, 1);
		if (btn_value == 0){
			*(&stand_by) = 1;
		}
		else if (btn_value == 1){
			*(&select_game) = 0001;
			*(&stand_by) = 0;
		}
		else if (btn_value == 2){
			*(&select_game) = 0010;
			*(&stand_by) = 0;
		}
		else if (btn_value == 4){
			*(&select_game) = 0100;
			*(&stand_by) = 0;
		}
		else if (btn_value == 8){
			*(&select_game) = 1000;
			*(&stand_by) = 0;
		}
	}
	else if (index_flux == 2 && index_game == 0){
		instructions();
		btn_value = XGpio_DiscreteRead(&BTNInst, 1);
		if (btn_value == 1){
			*(&select_game) = 0001;
		}
		else if (btn_value == 2){
			*(&select_game) = 0010;
		}

	(void)XGpio_InterruptClear(&BTNInst, BTN_INT);
	// Enable GPIO interrupts
	XGpio_InterruptEnable(&BTNInst, BTN_INT);
}


/*
tmr_handler(){
	if (index_flux = 1 and index_game = 0){
		muestra secuencia de inicio de leds
	}
	else if (index_flux = 1 and index_game = 1){
		ejecuta secuencias a repetir de los LEDs con random y la compara con select_game para cada i en array;
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
	else if (index_flux = 2 and index_game = 0){
		ejecuta parpadeo de los 4 LEDs tendr´an que parpadear 3 veces para indicar que se terminó el juego. Frecuencia de medio segundo
	}
}
*/


void TMR_Intr_Handler(void *data){
	if (XTmrCtr_IsExpired(&TMRInst,0)){
		XTmrCtr_Reset(&TMRInst,0);
		if (index_flux == 1 && index_game == 0){
			if(tmr_count == 9){ // La segunda vez que entre (10 veces 0.05 segundos = 0.5 segundo)
								// se suma uno a la data del led
				tmr_count = 0;
				if (led_counter == 0){
					led_data = 15;
				}
				else if (led_counter == 1){
					led_data = 0;
				}
				else if (led_counter == 2){
					led_data = 15;
				}
				else if (led_counter == 3){
					led_data = 0;
				}
				led_counter++;
				// escribe en led
				XGpio_DiscreteWrite(&LEDInst, 1, led_data);
				if (led_counter == 4){
					*(&index_game) = 1;
				}
			}
			else tmr_count++;
		}

		else if (index_flux == 1 && index_game == 1){
			///////
			if(tmr_count == 1){ // La segunda vez que entre (2 veces 0.05 segundos = 0.1 segundo)
							// se suma uno a la data del led
				tmr_count = 0;
				// semaforo verde y peaton rojo
				if (led_counter < 8){
					led_data = led_sequence_array[i];
				}
				else if(led_counter >= 8 && led_counter < 10){
				led_data = 0;
				}
				led_counter++;
				if (led_counter == 10){
				// reinicia el semaforo
				led_counter = 0;
				++i;
				}
				XGpio_DiscreteWrite(&LEDInst, 1, led_data);
			}
			else{
				tmr_count++
			}
		}
		else if (index_flux == 1 && index_game == 2){

			//----------------------------------- AQUI NOS ESTANCAMOS ---------------------
			
			if(tmr_count == 19){ // La segunda vez que entre (20 veces 0.05 segundos = 1 segundo)
							// se suma uno a la data del led
				tmr_count = 0;
				
				for (int i = 0; i < len(led_sequence_array); ++i){
					//hay un timer_life contando de 0 a 30
					if (timer_life < 30){

						if not (select_game == led_sequence_array[i]){
							printf(error en secuencia uwu)
							comparacion = 0;
							break;

						}
					}
					else{
						//perdiste ggwp
					}
				}



			}
			else{
				tmr_count++
			}

		}
		else if (index_flux == 2 && index_game == 0){
			if(tmr_count == 9){ // La segunda vez que entre (10 veces 0.05 segundos = 0.5 segundo)
								// se suma uno a la data del led
				tmr_count = 0;
				if (led_counter == 0){
					led_data = 15;
				}
				else if (led_counter == 1){
					led_data = 0;
				}
				else if (led_counter == 2){
					led_data = 15;
				}
				else if (led_counter == 3){
					led_data = 0;
				}
				else if (led_counter == 4){
					led_data = 15;
				}
				else if (led_counter == 5){
					led_data = 0;
				}
				led_counter++;
				// escribe en led
				XGpio_DiscreteWrite(&LEDInst, 1, led_data);
				if (led_counter == 6){
					*(&index_flux) = 0;
				}
			}
			else tmr_count++;
		}
	}	
}


	

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
			index_flux = 0;
			// btn_handler() depende de index_flux
			// if index_flux = 0 ->  btn_handler() muestra menu_inicio
			// if index_flux = 1 ->  btn_handler() almacenó select_game
			if select_game == 0001{
				ranking();
				index_flux = 0
			}
			while(select_game == 0010){
				counter_rondas = 0;
				index_game = 0;
				//tmr_handler() depende de index_game
				si index_game = 0 -> tmr_handler() ejecuta parpadeo de los cuatro LEDs por 2 segundos, con una frecuencia de medio segundo.
				tmr_handler(); 
				++index_game;
				sub_while(){
					si index_game = 1 -> tmr_handler() ejecuta secuencias a repetir de los LEDs con random;
					(en la primera iteracion se crea la secuencia) se genera elemento random en el array con maloc
					while (stand_by)
					btn_handler(); -> si index_flux = 1 y index_game = 1, btn_handler() escribe en select_game lo que recibe de los botones
					tmr_handler();
					++index_flux; 
				}
				
				if index_flux == 2{
					index_game = 0;
					tmr_handler() ejecuta parpadeo de los 4 LEDs tendr´an que parpadear 3 veces para indicar que se terminó el juego. Frecuencia de medio segundo,
					btn_handler() pasa a seleccion_postjuego
					si seleccion_postjuego = 01 -> mismo jugador, index_flux = 0, index_game = 0;
					si seleccion_postjuego = 10 -> otro jugador, autentificacion = 0, index_flux = 0, index_game = 0;
				}	
			}
		}
	}	
}







			

*/