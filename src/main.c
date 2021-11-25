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

/*Parametros de configuración del TIM3 - 200mseg:*/
#define TimeBase 200e3 //[mseg]

/*Frecuencia de muestreo - 50kHz:*/
#define FS  20e3 //[kHz]

/*Numero de coeficientes - 10:*/
#define n 11

/*Funcion para procesar los datos del ADC:*/
void ADC_PROCESSING(void);

/*Funcion de proceso del FIR:*/
void filter_fir_f32(float* pSrc, float* pDst, uint32_t N_COEF, float* pCoeff);

/*------------------------------------------------------------------------------
VARIABLES GLOBALES:
------------------------------------------------------------------------------*/
/*Declaracion del arreglo de la Señal de Estado:*/
float firState_f32[n + 1];

/*Coeficientes del filtro:*/
float Coeff_f32[n] = {
						-7.8158e-05, -7.9869e-03, -4.0285e-02, -1.0310e-01,
						-1.7003e-01, 8.0139e-01 , -1.7003e-01, -1.0310e-01,
						-4.0285e-02, -7.9869e-03, -7.8158e-05
					 };

/*Variable para organizar el Task Scheduler:*/
uint8_t adcReady = 0;

/*Variables de para crear el filtro FIR:*/
float firIn = 0.0f;
float firOut = 0.0f;
int32_t signalIn = 0;
int32_t signalOut = 0;

int main(void)
{
/*------------------------------------------------------------------------------
CONFIGURACION DEL MICRO:
------------------------------------------------------------------------------*/
	SystemInit();

	/*Inicializacion del ADC:*/
	INIT_ADC(ADC_Port, ADC_Pin);

	/*Inicializacion del DAC:*/
	INIT_DAC_CONT(DAC_Port, DAC_Pin);

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
	signalIn = READ_ADC(ADC_Port, ADC_Pin) - 2048;

	/*Normalizado 0.0 a 1.0. */		/*	-0.5 a 0.5	*/
	firIn = ((float)signalIn) / 4096;

	/*Llamado a la función de proceso FIR:*/
	filter_fir_f32(&firIn, &firOut, n, Coeff_f32);

	/*Desnormalizado 0 a 4096:*/
	signalOut = (firOut * 4096) + 2048;

	/*Conversion del dato del DA:*/
	DAC_CONT(DAC_Port, DAC_Pin, (uint16_t) signalOut);
}

/*------------------------------------------------------------------------------
FUNCIONES LOCALES:
------------------------------------------------------------------------------*/
/*Proceso del FIR: Y(n) = A0*X(n):*/
void filter_fir_f32(float* pSrc, float* pDst, uint32_t N_COEF, float* pCoeff)
{
	float ACUM;
	uint16_t k;

	/*Senal de Entrada:*/
	ACUM = *pSrc;

	/*X[n] = Ultimo elemento del buffer:*/
	firState_f32[N_COEF] = ACUM;
	/*Blanquear Acumulador:*/
	ACUM = 0.0f;

	/*Ciclo de K = 0 a NUM_COEF:*/
	for(k = 0; k < N_COEF; k++)
	{
		/*Convolucion:*/
		ACUM = ACUM + pCoeff[k] * firState_f32[k];
		/*Corrimiento de Datos:*/
		firState_f32[k] = firState_f32[k+1];
	}

	/*Senal de Salida:*/
	*pDst = ACUM;
}
