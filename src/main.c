/********************************************************************************
  * @file    main.c
  * @author  A. Riedinger & G. Stang.
  * @version 0.1
  * @date    25-11-21.
  * @brief   Generacion de un filtro elimina banda digital IIR con n = 6 y
  	  	  	 fc = 500 Hz segun síntesis tipo Cheby I.

  * SALIDAS:
  	  *	LCD  Conexion Estandar TPs
********************************************************************************/

/*------------------------------------------------------------------------------
LIBRERIAS:
------------------------------------------------------------------------------*/
#include "functions.h"

/*------------------------------------------------------------------------------
DEFINICIONES LOCALES:
------------------------------------------------------------------------------*/
/*Pines del ADC - PC0:*/
#define adcPort GPIOC
#define adcPin  GPIO_Pin_0

/*Pines del DAC - PA5:*/
#define dacPort GPIOA
#define dacPin  GPIO_Pin_5

/*Frecuencia de muestreo - 20kHz:*/
#define FS  20000 //[kHz]

/*Numero de coeficientes - 6:*/
#define n 6

/*Funcion para procesar los datos del ADC:*/
void ADC_PROCESSING(void);

/*Funcion de proceso del IIR:*/
void IIR_F32(float* pSrc, float* pDst, uint32_t N_COEF, float* pCoeff_b, float* pCoeff_a);

/*------------------------------------------------------------------------------
VARIABLES GLOBALES:
------------------------------------------------------------------------------*/
uint32_t i = 0;

/*Declaracion del arreglo de la Señal de Estado:*/
float iirStateIn_f32 [2*n];
float iirStateOut_f32[2*n];

/*Coeficientes del filtro obtenidos en GNU Octave con cheby1(): */
float b[2*n+1] = {0.1832424665583316, -2.4412800655459803e-16, 1.0994547993499895,
				 -1.2206400327729901e-15, 2.7486369983749741, -2.4412800655459802e-15,
				 3.6648493311666321, -2.4412800655459802e-15, 2.7486369983749741,
				 -1.2206400327729901e-15, 1.0994547993499895, -2.4412800655459803e-16,
				 0.1832424665583316};

float a[2*n+1] = {1, -5.5511151231257827e-16, 2.8266303814860598, -1.1102230246251565e-15,
				  3.9461521181664487, -3.5527136788005009e-15, 3.1150661053039146,
				  -2.8310687127941492e-15, 1.6070012817944181, -6.9388939039072284e-16,
				  0.52520012936098293, -2.0816681711721685e-16, 0.1384414456647135};

/*Variable para organizar el Task Scheduler:*/
uint8_t adcReady = 0;

/*Variables de para crear el filtro FIR:*/
float iirIn = 0.0f;
float iirOut = 0.0f;
int32_t signalIn = 0;
int32_t signalOut = 0;

int main(void)
{
/*------------------------------------------------------------------------------
CONFIGURACION DEL MICRO:
------------------------------------------------------------------------------*/
	SystemInit();

	/*Inicializacion del ADC:*/
	INIT_ADC(adcPort, adcPin);

	/*Inicializacion del DAC:*/
	INIT_DAC_CONT(dacPort, dacPin);

	/*Inicialización del TIM3:*/
	INIT_TIM3(FS);

	INIT_DO(GPIOC, GPIO_Pin_8);

/*------------------------------------------------------------------------------
BUCLE PRINCIPAL:
------------------------------------------------------------------------------*/
	while(1)
	{
		/*Task Scheduler:*/
		if (adcReady == 1)
			ADC_PROCESSING();
	}
}
/*------------------------------------------------------------------------------
INTERRUPCIONES:
------------------------------------------------------------------------------*/
/*Interrupcion al vencimiento de cuenta de TIM3 cada 1/FS:*/
void TIM3_IRQHandler(void) {
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        /*Set de la variable del TS:*/
        adcReady = 1;

        GPIO_ToggleBits(GPIOC, GPIO_Pin_8);

        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}

/*------------------------------------------------------------------------------
TAREAS:
------------------------------------------------------------------------------*/
/*Procesamiento de los datos del ADC:*/
void ADC_PROCESSING()
{
    /*Reset de la variable del TS:*/
    adcReady = 0;

	/*Conversion del dato del AD:*/
	signalIn = READ_ADC(adcPort, adcPin) - 2048;

	/*Normalizado 0.0 a 1.0. */		/*	-0.5 a 0.5	*/
	iirIn = ((float)signalIn) / 4096.0;

	/*Llamado a la función de proceso FIR:*/
	IIR_F32(&iirIn, &iirOut, 2*n, b, a);

	/*Desnormalizado 0 a 4096:*/
	signalOut = (iirOut * 4096) + 2048;

	/*Conversion del dato del DA:*/
	DAC_CONT(dacPort, dacPin, (uint16_t) signalOut);
}

/*------------------------------------------------------------------------------
FUNCIONES LOCALES:
------------------------------------------------------------------------------*/
/*Proceso del IIR: Y(n) = B0*X(n) + B1*X(n-1) + B2*X(n-2) + A1*Y(n-1) + A2*Y(n-2):*/
void IIR_F32(float* pSrc, float* pDst, uint32_t N_COEF, float* pCoeff_b, float* pCoeff_a)
{
	float ACUM = 0.0f;

	/*Senal de Entrada:*/
	ACUM = *pSrc * pCoeff_b[0];

	/*Diseño del filtro:*/
	for (uint32_t i = 0; i < N_COEF; i++) {
		ACUM += iirStateIn_f32 [i] * pCoeff_b[i+1];
		ACUM -= iirStateOut_f32[i] * pCoeff_a[i+1];
	}

	for (uint32_t i = N_COEF - 1; i > 0; i--){
		iirStateIn_f32[i]  = iirStateIn_f32 [i-1];
		iirStateOut_f32[i] = iirStateOut_f32[i-1];
	}

	iirStateIn_f32 [0] = *pSrc;
	iirStateOut_f32[0] = ACUM;

	/*Senal de salida:*/
	*pDst = ACUM;
}
