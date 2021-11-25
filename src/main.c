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

/*Frecuencia de muestreo - 50kHz:*/
#define FS  20e3 //[kHz]

/*Numero de coeficientes - 6:*/
#define n 6

/*Funcion para procesar los datos del ADC:*/
void ADC_PROCESSING(void);

/*Funcion de proceso del IIR:*/
void IIR_F32(float* pSrc, float* pDst, uint32_t N_COEF, float* pCoeff);

/*------------------------------------------------------------------------------
VARIABLES GLOBALES:
------------------------------------------------------------------------------*/
/*Declaracion del arreglo de la Señal de Estado:*/
float iirState_f32[n + 1];

/*Coeficientes del filtro:*/
float coeff_f32[n] = {
						-7.8158e-05, -7.9869e-03, -4.0285e-02, -1.0310e-01,
						-1.7003e-01, 8.0139e-01 , -1.7003e-01, -1.0310e-01,
						-4.0285e-02, -7.9869e-03, -7.8158e-05
					 };

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
	iirIn = ((float)signalIn) / 4096;

	/*Llamado a la función de proceso FIR:*/
	IIR_F32(&iirIn, &iirOut, n, coeff_f32);

	/*Desnormalizado 0 a 4096:*/
	signalOut = (iirOut * 4096) + 2048;

	/*Conversion del dato del DA:*/
	DAC_CONT(dacPort, dacPin, (uint16_t) signalOut);
}

/*------------------------------------------------------------------------------
FUNCIONES LOCALES:
------------------------------------------------------------------------------*/
/*Proceso del IIR: Y(n) = B0*X(n) + B1*X(n-1) + B2*X(n-2) + A1*Y(n-1) + A2*Y(n-2):*/
void IIR_F32(float* pSrc, float* pDst, uint32_t N_COEF, float* pCoeff)
{
	float ACUM;
	uint16_t i, i_bic;

	/*Senal de Entrada:*/
	ACUM = *pSrc;

	/*Repetir por N bicuadraticas:*/
	for(i_bic = 0; i_bic < N_COEF; i_bic++)
	{
		/*Ajustar Indice de variables:*/
		i = i_bic * 5;

		/*X[n] = Valor del ADC o ultimo filtro:*/
		iirState_f32[i + 2] = ACUM;

		/* Y[n] =        B0 * X[n]   */
		ACUM = pCoeff[i + 2] * iirState_f32[i + 2];
		/* Y[n] = Y[n] + B1 * X[n-1] */
		ACUM = ACUM + pCoeff[i + 1] * iirState_f32[i + 1];
		/* Y[n] = Y[n] + B2 * X[n-2] */
		ACUM = ACUM + pCoeff[i + 0] * iirState_f32[i + 0];
		/* Y[n] = Y[n] + A1 * Y[n-1] */
		ACUM = ACUM + pCoeff[i + 3] * iirState_f32[i + 3];
		/* Y[n] = Y[n] + A2 * Y[n-2] */
		ACUM = ACUM + pCoeff[i + 4] * iirState_f32[i + 4];

		 /* X[n-2] = X[n-1] */
		iirState_f32[i + 0] = iirState_f32[i + 1];
		 /* X[n-1] = X[n]   */
		iirState_f32[i + 1] = iirState_f32[i + 2];
		/* Y[n-2] = Y[n-1] */
		iirState_f32[i + 3] = iirState_f32[i + 4];
		/* Y[n-1] = Y[n]   */
		iirState_f32[i + 4] = ACUM;
	}

	/*Senal de salida:*/
	*pDst = ACUM;
}
