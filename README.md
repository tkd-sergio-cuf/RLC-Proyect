# RLC-Proyect
Proyecto medidor RLC con circuitos, integracion IoT con Helium y vizualizacion Pantalla OLED.
## Caracterizticas Funcionales:
- Mide resistencias entre 1k ohm y 330 kohm.
- Mide condensadores entre 1nF y 10uF.
- Mide inductancias entre 1mH a  330mH.
- Tiene interruptor para cortar o no alimentacion bateria. En otra palabras para apagar y prender todo el sistema.
- Tiene interruptor para activar y desactivar modo IoT.
- Tiene perilla para seleccionar modos (modo R1, R2, c y L). Hay dos modos de R con diferentes rangos.
- Cuenta con pantalla Oled para visualizar los datos.
   
## Ciruitos:
- Circuito medidor Resistencia:<br>
  La idea principal de este circuito es generar dos fuentes dew corriente para dos rangos de resistencia diferentes. El Switch (S2) en el esquematico se remplaza por un switch analogo. R3 es la resistencia que se mide. Los rangos de medicion son de [1k-18.165k] y [18.165k - 330k].<br>
  [![RCircuit.png](https://i.postimg.cc/Y9VZhyQq/RCircuit.png)](https://postimg.cc/WFGXy89Q)
- Circuito medidor Capacitancia:<br>
  La idea principal es generar un oscilador cuya frecuencia dependa exclusivamente de la variacion de una capacitancia, en este caso C1. Las ecuaciones estan mas abajo. <br>
  [![CCircuit.png](https://i.postimg.cc/CL9XFtSv/CCircuit.png)](https://postimg.cc/hfbps2Jx)
- Circuito medidor Inductancia:<br>
 La idea principal es generar un oscilador cuya frecuencia dependa exclusivamente de la variacion de una inductancia, en este caso L1. Las ecuaciones estan mas abajo. <br>
  [![LCircuit.png](https://i.postimg.cc/Vvrgpfrz/LCircuit.png)](https://postimg.cc/Fkv0JQV6)

## Microcontrolador:<br>
  El microcontrolador encargado de la medicion y de enviar los dator por Lora a Helium es una esp32. La usada en este proyecto es la Heltec esp32 wifi lora v2. Link del fabricante: https://heltec.org/project/wifi-lora-32/  <br>
  [![esp32.png](https://i.postimg.cc/hGY7w997/esp32.png)](https://postimg.cc/94GMwwgW)

## Diseño PCB:<br>
[![PCBDesign.png](https://i.postimg.cc/L8RFMwHz/PCBDesign.png)](https://postimg.cc/2VHKQHL6)


## PseudoCodigo:
Este pseudocodigo no sigue ninguna regla de diagramas de flujo ni nada por el estilo. Es mas un diagrama de bloques que busca dar una idea basica y general de lo que hace el codigo, si lo vemos como un diagrama de flujo con todo y sus reglas pues inflije varias de estas y tiene varios errores, pero como digo no es un diagrama de flujo formal.<br>
[![Captura-de-pantalla-2023-07-04-233413.png](https://i.postimg.cc/BvRGR6tv/Captura-de-pantalla-2023-07-04-233413.png)](https://postimg.cc/f37F9Mc4)

## Ejemplo producto final:
- Producto final:<br>
  [![Captura-de-pantalla-2023-07-04-234107.png](https://i.postimg.cc/BvmgMYZ5/Captura-de-pantalla-2023-07-04-234107.png)](https://postimg.cc/3yDmwZvW) <br>
- Pantalla OLED:<br>
  [![Captura-de-pantalla-2023-07-04-234237.png](https://i.postimg.cc/y8QgGw9N/Captura-de-pantalla-2023-07-04-234237.png)](https://postimg.cc/BjFnL7mW) <br>
- Visualizacion IoT:<br>
  [![Captura-de-pantalla-2023-07-04-234545.png](https://i.postimg.cc/HxHg9Lx8/Captura-de-pantalla-2023-07-04-234545.png)](https://postimg.cc/FfP89N0h) <br>

## Ecuaciones:
- Ecuaciones oscilador Capacitancia:<br>
  [![Captura-de-pantalla-2023-07-04-233223.png](https://i.postimg.cc/Zn7vhpNN/Captura-de-pantalla-2023-07-04-233223.png)](https://postimg.cc/BPFnT8MZ)
- Ecuaciones oscilador Inductancia:<br>
[![Captura-de-pantalla-2023-07-04-233232.png](https://i.postimg.cc/MKHjm3km/Captura-de-pantalla-2023-07-04-233232.png)](https://postimg.cc/Vdy6YRkJ)
 
