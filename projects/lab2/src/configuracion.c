#include "configuracion.h"

void ConfigurarInterrupciones(void) {
   __asm__ volatile ("cpsid i");

   /********* Configuración de interrupción de SysTick *********/

   /* Activate SysTick */
   SystemCoreClockUpdate();
   SysTick_Config(SystemCoreClock/1000);

   /* Update priority set by SysTick_Config */
   NVIC_SetPriority(SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);

   /************** Fin de configuración de SysTick *************/


   /****************** Configuración de timer ******************/

   // Uso reloj interno de 95 MHz
   LPC_TIMER0->CTCR = 0;

   // Limpio el flag de interrupción
   LPC_TIMER0->IR = LPC_TIMER0->IR;

   // Prescaler de 950000 para obtener una frecuencia de 100 Hz en el Timer Counter
   LPC_TIMER0->PR = 950000;

   // Se hará un match a las 25 cuentas del Timer Counter
   LPC_TIMER0->MR[0] = 25;

   // En el match se parará el timer, se lo reseteará, y se efectuará una interrupción del Timer 0
   LPC_TIMER0->MCR = TIMER_STOP_ON_MATCH(0) | TIMER_RESET_ON_MATCH(0) | TIMER_INT_ON_MATCH(0);

   // Reseteo el timer
   LPC_TIMER0->TCR = TIMER_RESET;
   
   // Elimino los pedidos pendientes de interrupción que puedan haber del Timer 0
   NVIC->ICPR[0] |= 1 << TIMER0_IRQn;

   // Habilito la interrupción para el Timer 0
   NVIC->ISER[0] |= 1 << TIMER0_IRQn;

   /****** Fin de configuración de interrupciones por GPIO *****/

   __asm__ volatile ("cpsie i");
}

void ConfigurarPuertosLaboratorio(void) {
   #define SCU_MODE_SAL (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS)
   #define SCU_MODE_ENT (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN)

   /* Puerto Led RGB */
   /* Chip_SCU_PinMux(port, pin, options, function); */
   Chip_SCU_PinMux(2, 0, SCU_MODE_SAL, SCU_MODE_FUNC4);
   Chip_SCU_PinMux(2, 1, SCU_MODE_SAL, SCU_MODE_FUNC4);
   Chip_SCU_PinMux(2, 2, SCU_MODE_SAL, SCU_MODE_FUNC4);
   /* Chip_GPIO_ClearValue(LPC_GPIO_PORT, port, mask); */
   Chip_GPIO_ClearValue(LPC_GPIO_PORT, 5, (1 << 0) | (1 << 1) | (1 << 2));
   /* Chip_GPIO_SetDir(LPC_GPIO_PORT, port, mask, output); */
   Chip_GPIO_SetDir(LPC_GPIO_PORT, 5, (1 << 0) | (1 << 1) | (1 << 2), 1);

   /* Puerto Leds 1 a 3 */
   Chip_SCU_PinMux(2, 10, SCU_MODE_SAL, SCU_MODE_FUNC0);
   Chip_SCU_PinMux(2, 11, SCU_MODE_SAL, SCU_MODE_FUNC0);
   Chip_SCU_PinMux(2, 12, SCU_MODE_SAL, SCU_MODE_FUNC0);
   Chip_GPIO_ClearValue(LPC_GPIO_PORT, 0, (1 << 14));
   Chip_GPIO_ClearValue(LPC_GPIO_PORT, 1, (1 << 11) | (1 << 12));
   Chip_GPIO_SetDir(LPC_GPIO_PORT, 0, (1 << 14), true);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, 1, (1 << 11) | (1 << 12), true);

   /* Puerto Teclas 1 a 4 */
   Chip_SCU_PinMux(1, 0, SCU_MODE_ENT, SCU_MODE_FUNC0);
   Chip_SCU_PinMux(1, 1, SCU_MODE_ENT, SCU_MODE_FUNC0);
   Chip_SCU_PinMux(1, 2, SCU_MODE_ENT, SCU_MODE_FUNC0);
   Chip_SCU_PinMux(1, 6, SCU_MODE_ENT, SCU_MODE_FUNC0);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, 0, (1 << 4) | (1 << 8) | (1 << 9), false);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, 1, (1 << 9), false);

   // Para configurar como U3_TXD al pin T_FIL1
   Chip_SCU_PinMux(4, 1, SCU_MODE_SAL, SCU_MODE_FUNC6);

   // Para configurar como U3_RXD al pin T_FIL2
   Chip_SCU_PinMux(4, 2, SCU_MODE_ENT, SCU_MODE_FUNC6);
}
