#include "board.h"
#include "chip.h"
#include <stdint.h>
#include <string.h>
#include "usbd_rom_api.h"
#include "hid_mouse.h"

extern const uint8_t Mouse_ReportDescriptor[];
extern const uint16_t Mouse_ReportDescSize;

//Estructura para almacenar datos del mouse
typedef struct {
	USBD_HANDLE_T hUsb;
	uint8_t report[MOUSE_REPORT_SIZE];
	uint8_t tx_busy;	//tx_busy = 1 indica que hay un reporte pendiente a ser enviado en el bus
} Mouse_Ctrl_T;

static volatile uint32_t tick_ct = 0;

void SysTick_Handler(void) {
	tick_ct++;
}

/*void delay(uint32_t tk) {
	uint32_t end = tick_ct + tk;
	while(tick_ct < end){}
}*/

void Buttons_Init(void){
	//TEC1
	Chip_SCU_PinMuxSet(0x1, 0, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC0));	//P1_0 a GPIO0[4]
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, 0, 4);	//entrada
	Chip_SCU_GPIOIntPinSel(0, 0, 4);	//GPIO0[4] a INT0
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(0));	//INT0
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(0));	//INT0
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(0));	//INT0
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH(0));	//INT0
	NVIC_ClearPendingIRQ(PIN_INT0_IRQn);

	//TEC2
	Chip_SCU_PinMuxSet(0x1, 1, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC0));	//P1_1 a GPIO0[8]
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, 0, 8);	//entrada
	Chip_SCU_GPIOIntPinSel(1, 0, 8);	//GPIO0[8] a INT1
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(1));	//INT1
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(1));	//INT1
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(1));	//INT1
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH(1));	//INT1
	NVIC_ClearPendingIRQ(PIN_INT1_IRQn);

	//TEC3
	Chip_SCU_PinMuxSet(0x1, 2, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC0));	//P1_2 a GPIO0[9]
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, 0, 9);	//entrada
	Chip_SCU_GPIOIntPinSel(2, 0, 9);	//GPIO0[9] a INT2
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(2));	//INT2
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(2));	//INT2
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(2));	//INT2
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH(2));	//INT2
	NVIC_ClearPendingIRQ(PIN_INT2_IRQn);

	//TEC4
	Chip_SCU_PinMuxSet(0x1, 6, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC0));	//P1_6 a GPIO1[9]
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, 1, 9);	//entrada
	Chip_SCU_GPIOIntPinSel(3, 1, 9);	//GPIO1[9] a INT3
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(3));	//INT3
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(3));	//INT3
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(3));	//INT3
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH(3));	//INT3
	NVIC_ClearPendingIRQ(PIN_INT3_IRQn);

	//Botón
	Chip_SCU_PinMuxSet(1, 5, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC0));	//P1_5 a GPIO1[8]
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, 1, 8);	//entrada
	Chip_SCU_GPIOIntPinSel(4, 1, 8);	//GPIO1[8] a INT4
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(4));	//INT4
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(4));	//INT4
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH(4));	//INT4
	NVIC_ClearPendingIRQ(PIN_INT4_IRQn);
	
	NVIC_EnableIRQ(PIN_INT0_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);
	NVIC_EnableIRQ(PIN_INT2_IRQn);
	NVIC_EnableIRQ(PIN_INT3_IRQn);
	NVIC_EnableIRQ(PIN_INT4_IRQn);
}

//Instancia de la estructura
static Mouse_Ctrl_T g_mouse;

//Variables de control del mouse
static bool UP = 0;
static bool DOWN = 0;
static bool LEFT = 0;
static bool RIGHT = 0;
static bool PRESS = 0;

void GPIO0_IRQHandler (void)
{
	/* TEC 1 */
	/*Limpiamos el pedido de interrupción*/
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(0));
	
	if(LEFT==0){LEFT=1;}
	else{LEFT=0;}
}

void GPIO1_IRQHandler (void)
{
	/* TEC 2 */
	/*Limpiamos el pedido de interrupción*/
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(1));
	
	if(UP==0){UP=1;}
	else{UP=0;}
}

void GPIO2_IRQHandler (void)
{
	/* TEC 3 */
	/*Limpiamos el pedido de interrupción*/
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(2));
	
	if(DOWN==0){DOWN=1;}
	else{DOWN=0;}
}

void GPIO3_IRQHandler (void)
{
	/* TEC 4 */
	/*Limpiamos el pedido de interrupción*/
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(3));
	
	if(RIGHT==0){RIGHT=1;}
	else{RIGHT=0;}
}

void GPIO4_IRQHandler (void)
{
	/* BOTON */
	/*Limpiamos el pedido de interrupción*/
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(4));
	
	PRESS = 1;
}

static void setXYMouseReport(uint8_t *rep, int8_t xVal, int8_t yVal)
{
	rep[1] = (uint8_t) xVal;
	rep[2] = (uint8_t) yVal;
}

static void setLeftButtonMouseReport(uint8_t *rep, uint8_t state)
{
	if(state){rep[0 ] = 0x01;}
	else{rep[0] = 0x00;}
}

static void Mouse_UpdateReport(void)
{
	CLEAR_HID_MOUSE_REPORT(&g_mouse.report[0]);

	if(UP == 1){
		setXYMouseReport(g_mouse.report, 0, -2);
	}
	if(DOWN == 1){
		setXYMouseReport(g_mouse.report, 0, 2);
	}
	if(LEFT == 1){
		setXYMouseReport(g_mouse.report, -2, 0);
	}
	if(RIGHT == 1){
		setXYMouseReport(g_mouse.report, 2, 0);
	}
	if(PRESS==1){
		delay(200);
		setLeftButtonMouseReport(g_mouse.report,1);
		PRESS = 0;
	}
}

void Mouse_Tasks(void)
{
	if(USB_IsConfigured(g_mouse.hUsb)){
		if(g_mouse.tx_busy==0){
			Mouse_UpdateReport();
			g_mouse.tx_busy = 1;
			USBD_API->hw->WriteEP(g_mouse.hUsb, HID_EP_IN, &g_mouse.report[0], MOUSE_REPORT_SIZE);
		}
	}
	else{
		g_mouse.tx_busy = 0;
	}
}

/* HID Get Report Request Callback. Called automatically on HID Get Report Request */
static ErrorCode_t Mouse_GetReport(USBD_HANDLE_T hHid, USB_SETUP_PACKET *pSetup, uint8_t * *pBuffer, uint16_t *plength)
{
	/* ReportID = SetupPacket.wValue.WB.L; */
	switch (pSetup->wValue.WB.H) {
	case HID_REPORT_INPUT:
		Mouse_UpdateReport();
		*pBuffer = &g_mouse.report[0];
		*plength = MOUSE_REPORT_SIZE;
		break;

	case HID_REPORT_OUTPUT:				/* Not Supported */
	case HID_REPORT_FEATURE:			/* Not Supported */
		return ERR_USBD_STALL;
	}
	return LPC_OK;
}

static ErrorCode_t Mouse_SetReport(USBD_HANDLE_T hHid, USB_SETUP_PACKET *pSetup, uint8_t * *pBuffer, uint16_t length)
{
	/* we will reuse standard EP0Buf */
	if (length == 0) {
		return LPC_OK;
	}
	/* ReportID = SetupPacket.wValue.WB.L; */
	switch (pSetup->wValue.WB.H) {
	case HID_REPORT_INPUT:				/* Not Supported */
	case HID_REPORT_OUTPUT:				/* Not Supported */
	case HID_REPORT_FEATURE:			/* Not Supported */
		return ERR_USBD_STALL;
	}
	return LPC_OK;
}

static ErrorCode_t Mouse_EpIN_Hdlr(USBD_HANDLE_T hUsb, void *data, uint32_t event)
{
	switch (event) {
	case USB_EVT_IN:
		/* USB_EVT_IN occurs when HW completes sending IN packet. So clear the
		    busy flag for main loop to queue next packet.
		 */
		g_mouse.tx_busy = 0;
		break;
	}
	return LPC_OK;
}

ErrorCode_t Mouse_Init(USBD_HANDLE_T hUsb,
					   USB_INTERFACE_DESCRIPTOR *pIntfDesc,
					   uint32_t *mem_base,
					   uint32_t *mem_size)
{
	USBD_HID_INIT_PARAM_T hid_param;
	USB_HID_REPORT_T reports_data[1];
	ErrorCode_t ret = LPC_OK;

	/* Do a quick check of if the interface descriptor passed is the right one. */
	if ((pIntfDesc == 0) || (pIntfDesc->bInterfaceClass != USB_DEVICE_CLASS_HUMAN_INTERFACE)) {
		return ERR_FAILED;
	}

	Chip_GPIO_Init(LPC_GPIO_PORT);
	Buttons_Init();
	SysTick_Config(SystemCoreClock / 1000);

	/* Init HID params */
	memset((void *) &hid_param, 0, sizeof(USBD_HID_INIT_PARAM_T));
	hid_param.max_reports = 1;
	hid_param.mem_base = *mem_base;
	hid_param.mem_size = *mem_size;
	hid_param.intf_desc = (uint8_t *) pIntfDesc;
	/* user defined functions */
	hid_param.HID_GetReport = Mouse_GetReport;
	hid_param.HID_SetReport = Mouse_SetReport;
	hid_param.HID_EpIn_Hdlr  = Mouse_EpIN_Hdlr;
	/* Init reports_data */
	reports_data[0].len = Mouse_ReportDescSize;
	reports_data[0].idle_time = 0;
	reports_data[0].desc = (uint8_t *) &Mouse_ReportDescriptor[0];
	hid_param.report_data  = reports_data;

	ret = USBD_API->hid->init(hUsb, &hid_param);

	/* update memory variables */
	*mem_base = hid_param.mem_base;
	*mem_size = hid_param.mem_size;
	/* store stack handle for later use. */
	g_mouse.hUsb = hUsb;
	g_mouse.tx_busy = 0;

	return ret;
}

