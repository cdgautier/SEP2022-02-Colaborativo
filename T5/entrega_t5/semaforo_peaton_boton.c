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

//----------------------------------------------------
// INTERRUPT HANDLER FUNCTIONS
// - called by the timer, button interrupt, performs
// - LED flashing
//----------------------------------------------------

void BTN_Intr_Handler(void *InstancePtr)
{
	// Disable GPIO interrupts
	XGpio_InterruptDisable(&BTNInst, BTN_INT);
	// Ignore additional button presses
	if ((XGpio_InterruptGetStatus(&BTNInst) & BTN_INT) !=
			BTN_INT) {
			return;
		}
	// recibe el boton y cambia btn_press
	btn_value = XGpio_DiscreteRead(&BTNInst, 1);
	if (btn_value == 1){
		*(&btn_press) = 1;
	}

	(void)XGpio_InterruptClear(&BTNInst, BTN_INT);
	// Enable GPIO interrupts
	XGpio_InterruptEnable(&BTNInst, BTN_INT);
}


void TMR_Intr_Handler(void *data)
{
	if (XTmrCtr_IsExpired(&TMRInst,0)){
		XTmrCtr_Reset(&TMRInst,0);
		if(tmr_count == 1){ // La segunda vez que entre (2 veces 0.5 segundos = 1 segundo)
							// se suma uno a la data del led
			tmr_count = 0;
			// semaforo verde y peaton rojo
			if (led_counter < 5){
				led_data = 2;
			}
			// semaforo amarillo y peaton rojo
			else if(led_counter >= 5 && led_counter < 7){
				led_data = 4;
			}
			// semaforo rojo y peaton verde
			else if(led_counter >= 7){
				led_data = 9;
			}
			led_counter++;

			if (led_counter == max_counter){
				// reinicia el semaforo
				led_counter = 0;
				// si el boton fue presionado, tiempo en rojo siguiente +2
				if (btn_press == 1){
					*(&max_counter) = 17; // hint: para un efecto visual mas claro setear en 20
					*(&btn_press) = 0;
				}
				// si no fue presionado, mantiene el tiempo en rojo normal
				else{
					*(&max_counter) = 15;
				}
			}
			// escribe en led
			XGpio_DiscreteWrite(&LEDInst, 1, led_data);
		}
		else tmr_count++;
	}
}

//----------------------------------------------------
// MAIN FUNCTION
//----------------------------------------------------

int main (void)
{
  int status;

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

  while(1);
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

