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
#define INTC_TMR_INTERRUPT_ID XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR

#define TMR_LOAD			(0xFFFFFFFF - 50000000*0.5) // 0.5s Timer

XGpio LEDInst;
XScuGic INTCInst;
XTmrCtr TMRInst;

static int tmr_count;
static int led_counter;       // iterador del ciclo del semaforo
static int led_data;	      // valor qur imprime el led
static int max_counter = 15;  // valor en que se reinicia el ciclo del semaforo

//----------------------------------------------------
// PROTOTYPE FUNCTIONS
//----------------------------------------------------

static void TMR_Intr_Handler(void *baseaddr_p);
static int InterruptSystemSetup(XScuGic *XScuGicInstancePtr);
static int IntcInitFunction(u16 DeviceId, XTmrCtr *TmrInstancePtr);

//----------------------------------------------------
// INTERRUPT HANDLER FUNCTIONS
// - called by the timer, button interrupt, performs
// - LED flashing
//----------------------------------------------------

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


  //----------------------------------------------------
  // SETUP THE TIMER
  //----------------------------------------------------
  status = XTmrCtr_Initialize(&TMRInst, TMR_DEVICE_ID);
  if(status != XST_SUCCESS) return XST_FAILURE;
  XTmrCtr_SetHandler(&TMRInst, (XTmrCtr_Handler) TMR_Intr_Handler, &TMRInst);
  XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD);
  XTmrCtr_SetOptions(&TMRInst, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
 
  // Initialize interrupt controller
  status = IntcInitFunction(INTC_DEVICE_ID, &TMRInst);
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
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 	 	 	 	 	 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
			 	 	 	 	 	 XScuGicInstancePtr);
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

int IntcInitFunction(u16 DeviceId, XTmrCtr *TmrInstancePtr)
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