#---------------------------------------------------------------
# LIBRERIAS: 
#---------------------------------------------------------------
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
#---------------------------------------------------------------

#---------------------------------------------------------------
# FUNCIONES:
#---------------------------------------------------------------
# Convertir un valor de voltage a dB:
def V_to_dB(x):
    return 20*np.log10(x)
#---------------------------------------------------------------

#---------------------------------------------------------------
# MAIN:
#---------------------------------------------------------------
# Importar datos .csv:
df = pd.read_csv('scopeMeasure.csv')

# Calculo de la ganancia en cada punto en dB:
H  = V_to_dB(df.V_o/df.V_i) 

# Definir estilo del plot:
plt.style.use('seaborn-deep')

# Grafico de H en funcion de la frecuencia:
plt.plot(df.Freq,H,label='$f_c=5\,[kHz]$',linewidth=2)

# Labels:
plt.xlabel('Frecuencia $[Hz]$')
plt.ylabel('$Mag(H)\,[dB]$')
plt.title('Filtro FIR Pasa Altos')
plt.legend()

# Activar grilla y afinar el plot:
plt.grid(True)
plt.tight_layout()

# Mostrar el grafico y guardarlo como png:
plt.savefig('freqRes.png',dpi=600)
plt.show()
#---------------------------------------------------------------
