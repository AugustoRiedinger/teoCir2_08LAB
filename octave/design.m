clc
clear
clear all
pkg load signal

% =----------------------------------------------------------------
% DATOS - FILTRO IIR ELIMINA BANDA:
% =----------------------------------------------------------------
% Orden del filtro:
n = 6;

% Frecuencia de muestreo:
fs = 20e3; 

% Frecuencia maxima segun Nyquist:
fm = fs/2;

% Frecuencia de corte central:
fc = 0.5 * fm; 

% Frecuencia de corte superior:
fh = fc + 0.2*fc;

% Frecuencia de corte inferior:
fl = fc - 0.2*fc;

% Mensajes:
printf("\n=--------------------------------------------")
printf("\nFILTRO IIR ELIMINA BANDA CHEBY TIPO 1")
printf("\n=--------------------------------------------")
printf("\n  - Orden: %d\n",n)
printf("\n  - Frecuencia muestreo:%d Hz\n",fs)
printf("\n  - Frecuencia maxima : %d Hz\n",fm)
printf("\n  - Frecuencia inferior:%d Hz\n",fl)
printf("\n  - Frecuencia central: %d Hz\n",fc)
printf("\n  - Frecuencia superior:%d Hz\n\n",fh)
% =----------------------------------------------------------------

% =----------------------------------------------------------------
% DISEÑO DEL FILTRO - Cheby Tipo I:
% =----------------------------------------------------------------
% Coeficientes:
[b,a] = cheby1(n,1,[fl/fm,fh/fm],"stop");

% Grafico de la respuesta en frecuencia:
freqz(b,a)
% =----------------------------------------------------------------
