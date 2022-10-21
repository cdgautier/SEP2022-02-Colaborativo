// solo guardar ptje mas alto del mismo jugador

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>


#include "platform.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xtmrctr.h"
#include "xscugic.h"
#include "xil_exception.h"
//#include "xil_printf.h"

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

static int init_counter = 0;
static int btn_value;         // recibe la señal del boton presionado
static int swt_value;         // recibe la señal del switch presionado
static int select_game = 0;	      // vale 1 si el boton fue presionado y almacena este valor
static int swt_on = 0;
static int tmr_count;
static int tmr_life;
static int led_counter;       // iterador del ciclo del semaforo
static int led_data;	      // valor qur imprime el led
static int index_flux = 0;
static int index_game = 0;
static int stand_by = 1;
static int comparation = 1;
static int array_iterator = 0;
static int score = 0;
static int autentification = 0;
static int num_players = 1;
static int result_verification;
static char input_name[20];
static int max_sequence = 20;
static int led_sequence_array[85]; // record mundial 84x
static int exp;
static int value;
static int timer_life;


typedef struct Player
{
   char player_name[20];
   int player_score;
}player;

//----------------------------------------------------
// PROTOTYPE FUNCTIONS
//----------------------------------------------------

static void ranking(player *players_data);  
static void menu_postgame();
static void generate_random_led();
static void reset_array_led();
static void TMR_Intr_Handler(void *baseaddr_p, player *players_data);
static void BTN_Intr_Handler(void *baseaddr_p, player *players_data);
static void SWT_Intr_Handler(void *baseaddr_p);
static int InterruptSystemSetup(XScuGic *XScuGicInstancePtr);
static int IntcInitFunctionTmr(u16 DeviceId, XTmrCtr *TmrInstancePtr);
static int IntcInitFunctionBtn(u16 DeviceId, XGpio *GpioInstancePtr);
static int IntcInitFunctionSwt(u16 DeviceId, XGpio *GpioInstancePtr);

//----------------------------------------------------
// DEFINITION FUNCTIONS
//----------------------------------------------------

void ranking(player *players_data) 
{
	int temp_score = 0;
    char temp_name[20];
       
    //Sort in descending order    
    for (int i = 0; i < num_players; i++) {     
        for (int j = i+1; j < num_players; j++) {     
           if(players_data[i].player_score < players_data[j].player_score) { 
               // sort puntajes
               temp_score = *(&players_data[i].player_score);    
               *(&players_data[i].player_score) = *(&players_data[j].player_score);    
               *(&players_data[j].player_score) = temp_score;   
               // sort nombres
               strcpy(temp_name, *(&players_data[i].player_name));    
               strcpy(*(&players_data[i].player_name), *(&players_data[j].player_name));    
               strcpy(*(&players_data[j].player_name), temp_name); 
           }     
        }     
    }    
    printf("\n\n * * * * * Ranking de Puntajes * * * * * \n\n");
    for (int k = 0; k < 3; k++){
        printf(" %d) %s - %d puntos \n", k+1, players_data[k].player_name ,players_data[k].player_score);
    }  
	printf("\n * * * * * * * * * * * * * * * * * * * * \n\n");
}

void menu_postgame()
{
	 printf("\n\n * * * * * GAME OVER * * * * * \n\n");
	printf("Presiona BTN0 para intentarlo otra vez o BTN1 para cambiar de usuario");
	printf("\n\n  * * * * * * * *  * * * * * * \n");
}

void generate_random_led()
{
	*(&exp) = rand() % 4;
	*(&value) = pow(2, exp);
	*(&led_sequence_array[array_iterator]) = value;
}

void reset_array_led()
{
	for(int i; i < 85; i++){
		led_sequence_array[i] = 0;
	}
}

void SWT_Intr_Handler(void *InstancePtr)
{
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

void TMR_Intr_Handler(void *data, player *players_data)
{
	if (XTmrCtr_IsExpired(&TMRInst,0)){
		XTmrCtr_Reset(&TMRInst,0);
		// Parpadeo inicializacion
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
    				generate_random_led();
					*(&index_game) = 1;
				}
			}
			else tmr_count++;
		}
		// Secuencia leds a copiar
		else if (index_flux == 1 && index_game == 1){
			if (array_iterator < (score + 1)){
				if(tmr_count == 1){ // La segunda vez que entre (2 veces 0.05 segundos = 0.1 segundo)
							// se suma uno a la data del led
					tmr_count = 0;
					if (led_counter < 8){
						led_data = led_sequence_array[array_iterator];
					}
					else if(led_counter >= 8 && led_counter < 10){
						led_data = 0;
					}
					led_counter++;
					if (led_counter == 10){
						*(&array_iterator) = *(&array_iterator) + 1;
						led_counter = 0;
					}
					XGpio_DiscreteWrite(&LEDInst, 1, led_data);
				}
				else{
					tmr_count++;
				}
			}
			else if (array_iterator == (score + 1)){
				*(&array_iterator) = 0;
				*(&index_game) = 2;
			}
		}
		// Comparación secuencia reproducida por los botones
		else if (index_flux == 1 && index_game == 2){

			if(tmr_count == 19){ // La segunda vez que entre (20 veces 0.05 segundos = 1 segundo)
							// se suma uno a la data del led
				tmr_count = 0;

				if (timer_life < 30){   // si queda tiempo
					if (stand_by == 0){  // y se ha presionado un boton, se compara
						if (select_game != led_sequence_array[array_iterator]){  // si la comparacion no corresponde
							printf("\n error en secuencia uwu");
							*(&comparation) = 0;
						}
						else{   // si la comparacion es correcta, sigue con el siguiente elemento
							//++comparation_sum;
							*(&array_iterator) += 1;
							timer_life = -1; // esto es para que no haya conflicto al sumar ++timer_life;

						}
					}
				}
				++timer_life;	// suma al tiempo
				if (timer_life == 30){ // se acaba el tiempo sin presionar boton
					printf("\n te quedaste sin tiempo uwu");
					*(&comparation) = 0;
				}
				if (array_iterator == (score + 1)){
					*(&score) += 1;
					*(&array_iterator) = 0;
					generate_random_led();
					*(&index_flux) = 1;
					*(&index_game) = 1;
				}
				if (comparation = 0){ // pasa a postjuego
					printf("\n game over");
					*(&array_iterator) = 0;
					reset_array_led();
					*(&index_flux) = 2;
					*(&index_game) = 0;
					*(&players_data[(num_players - 1)].player_score) = score;
					*(&score) = 0;
				}
			}
			else{
				tmr_count++;
			}
		}
		// Parpadeo fin de juego
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
				}
			}
			else {tmr_count++;}
		}
	}	
}

void BTN_Intr_Handler(void *InstancePt, player *players_data)
{
	// Disable GPIO interrupts
	XGpio_InterruptDisable(&BTNInst, BTN_INT);
	// Ignore additional button presses
	if ((XGpio_InterruptGetStatus(&BTNInst) & BTN_INT) !=
			BTN_INT) {
			return;
		}
	// eleccion de menu
	if (index_flux == 0 && index_game == 0){
		printf("\n  presiona BTN0 para jugar o BTN1 para ver el ranking de puntajes \n");
		btn_value = XGpio_DiscreteRead(&BTNInst, 1);
		if (btn_value == 1){
			*(&index_flux) = 1; // juega
		}
		else if (btn_value == 2){
			ranking(players_data);
		}
	}
	// realizando reproduccion de secuencia
	else if (index_flux == 1 && index_game == 2){
		btn_value = XGpio_DiscreteRead(&BTNInst, 1);
		if (btn_value == 0){
			*(&stand_by) = 1;
		}
		else if (btn_value == 1){
			*(&select_game) = 1;
			*(&stand_by) = 0;
		}
		else if (btn_value == 2){
			*(&select_game) = 2;
			*(&stand_by) = 0;
		}
		else if (btn_value == 4){
			*(&select_game) = 4;
			*(&stand_by) = 0;
		}
		else if (btn_value == 8){
			*(&select_game) = 8;
			*(&stand_by) = 0;
		}
	}
	// seleccion post juego
	else if (index_flux == 2 && index_game == 0){
		menu_postgame();
		btn_value = XGpio_DiscreteRead(&BTNInst, 1);
		if (btn_value == 1){ // jugar con el mismo usuario
			*(&index_flux) = 0;
			*(&index_game) = 0;
			*(&autentification) = 1;
		}
		else if (btn_value == 2){  // jugar con otro usuario
			*(&index_flux) = 0;
			*(&index_game) = 0;
			*(&autentification) = 0;
			*(&num_players) += 1;
			players_data = (player*) realloc(players_data, num_players * sizeof(*players_data));
			
		}

	(void)XGpio_InterruptClear(&BTNInst, BTN_INT);
	// Enable GPIO interrupts
	XGpio_InterruptEnable(&BTNInst, BTN_INT);
}

//----------------------------------------------------
// MAIN FUNCTION
//----------------------------------------------------

int main (void)
{	
	srand(time(NULL));
	int status;

    player* players_data = malloc(num_players * sizeof(*players_data));
	
	init_platform();

	if ((swt_on == 0) && (init_counter == 0)){
		printf("\n enciende el switch 0 para iniciar el juego");
		++init_counter;
	}

	//----------------------------------------------------
	// INITIALIZE THE PERIPHERALS & SET DIRECTIONS OF GPIO
	//----------------------------------------------------

	// Initialise LEDs
	status = XGpio_Initialize(&LEDInst, LEDS_DEVICE_ID);
	if(status != XST_SUCCESS) return XST_FAILURE;
	// Set LEDs direction to outputs
	XGpio_SetDataDirection(&LEDInst, 1, 0x00);

	// Initialise Push Buttons
	status = XGpio_Initialize(&BTNInst, BTNS_DEVICE_ID);
	if(status != XST_SUCCESS) return XST_FAILURE;
	// Set all buttons direction to inputs
	XGpio_SetDataDirection(&BTNInst, 1, 0xFF);

	// Initialise Switches
	status = XGpio_Initialize(&SWTInst, SWTS_DEVICE_ID);
	if(status != XST_SUCCESS) return XST_FAILURE;
	// Set all buttons direction to inputs
	XGpio_SetDataDirection(&SWTInst, 1, 0xFF);

	//----------------------------------------------------
	// SETUP THE TIMER
	//----------------------------------------------------
	status = XTmrCtr_Initialize(&TMRInst, TMR_DEVICE_ID);
	if(status != XST_SUCCESS) return XST_FAILURE;
	XTmrCtr_SetHandler(&TMRInst, (XTmrCtr_Handler) TMR_Intr_Handler, &TMRInst);
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD);
	XTmrCtr_SetOptions(&TMRInst, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);

	// Initialize interrupt controller tmr
	status = IntcInitFunctionTmr(INTC_DEVICE_ID, &TMRInst);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Initialize interrupt controller btn
	status = IntcInitFunctionBtn(INTC_DEVICE_ID, &BTNInst);
	if(status != XST_SUCCESS) return XST_FAILURE;

	XTmrCtr_Start(&TMRInst, 0);

	//----------------------------------------------------
	// MAIN FLUX
	//----------------------------------------------------

	while(swt_on){

		printf("\n\n * * * * * ¡Bienvenido a Simon Dice! * * * * * \n\n");

    	while(autentification == 0){
		    printf("Ingresa alias de max 15 caracteres: ");
		    scanf("%s",input_name);

		    for (int j = 0; j < num_players; j++){
		        result_verification = strcmp(players_data[j].player_name, input_name);
		        if (result_verification == 0){
		            printf("\n el nombre ya fue ingresado, intenta otra vez\n");
		            break;
		        }
		        else{
		            strcpy(players_data[(num_players - 1)].player_name, input_name);
		            *(&autentification) = 1;
		            break;
		        }
		    }
    	}
	}
	free(players_data);
	cleanup_platform();
	return 0;
}

//----------------------------------------------------
// INITIAL SETUP FUNCTIONS
//----------------------------------------------------

int InterruptSystemSetup(XScuGic *XScuGicInstancePtr)
{
	// Enable interrupt
	XGpio_InterruptEnable(&BTNInst, BTN_INT);
	XGpio_InterruptGlobalEnable(&BTNInst);

	XGpio_InterruptEnable(&SWTInst, SWT_INT);
	XGpio_InterruptGlobalEnable(&SWTInst);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 	 	 	 	 	 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
			 	 	 	 	 	 XScuGicInstancePtr);
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

int IntcInitFunctionTmr(u16 DeviceId, XTmrCtr *TmrInstancePtr)
{
	XScuGic_Config *IntcConfig;
	int status;

	// Interrupt controller initialisation
	IntcConfig = XScuGic_LookupConfig(DeviceId);
	status = XScuGic_CfgInitialize(&INTCInst, IntcConfig, IntcConfig->CpuBaseAddress);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Call to interrupt setup
	status = InterruptSystemSetup(&INTCInst);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Connect timer interrupt to handler
	status = XScuGic_Connect(&INTCInst,
							 INTC_TMR_INTERRUPT_ID,
							 (Xil_ExceptionHandler)TMR_Intr_Handler,
							 (void *)TmrInstancePtr);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Enable timer interrupts in the controller
	XScuGic_Enable(&INTCInst, INTC_TMR_INTERRUPT_ID);

	return XST_SUCCESS;
}


int IntcInitFunctionBtn(u16 DeviceId, XGpio *GpioInstancePtr)
{
	XScuGic_Config *IntcConfig;
	int status;

	// Interrupt controller initialisation
	IntcConfig = XScuGic_LookupConfig(DeviceId);
	status = XScuGic_CfgInitialize(&INTCInst, IntcConfig, IntcConfig->CpuBaseAddress);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Call to interrupt setup
	status = InterruptSystemSetup(&INTCInst);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Connect GPIO interrupt to handler
	status = XScuGic_Connect(&INTCInst,
					  	  	 INTC_GPIO_INTERRUPT_ID,
					  	  	 (Xil_ExceptionHandler)BTN_Intr_Handler,
					  	  	 (void *)GpioInstancePtr);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Enable GPIO interrupts interrupt
	XGpio_InterruptEnable(GpioInstancePtr, 1);
	XGpio_InterruptGlobalEnable(GpioInstancePtr);

	// Enable GPIO and timer interrupts in the controller
	XScuGic_Enable(&INTCInst, INTC_GPIO_INTERRUPT_ID);

	return XST_SUCCESS;
}


int IntcInitFunctionSwt(u16 DeviceId, XGpio *GpioInstancePtr)
{
	XScuGic_Config *IntcConfig;
	int status;

	// Interrupt controller initialisation
	IntcConfig = XScuGic_LookupConfig(DeviceId);
	status = XScuGic_CfgInitialize(&INTCInst, IntcConfig, IntcConfig->CpuBaseAddress);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Call to interrupt setup
	status = InterruptSystemSetup(&INTCInst);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Connect GPIO interrupt to handler
	status = XScuGic_Connect(&INTCInst,
					  	  	 INTC_GPIO_SWT_INTERRUPT_ID,
					  	  	 (Xil_ExceptionHandler)SWT_Intr_Handler,
					  	  	 (void *)GpioInstancePtr);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Enable GPIO interrupts interrupt
	XGpio_InterruptEnable(GpioInstancePtr, 1);
	XGpio_InterruptGlobalEnable(GpioInstancePtr);

	// Enable GPIO and timer interrupts in the controller
	XScuGic_Enable(&INTCInst, INTC_GPIO_SWT_INTERRUPT_ID);

	return XST_SUCCESS;
}