El código funciona con índices que indican la fase en la que nos encontramos, cuyas combinaciones son las siguientes:

I) index_flux = 0, index_game = 0: corresponde al inicio del juego, donde se muestra un menu principal que ofrece jugar o ver ranking
II) index_flux = 1, index_game = 0: en esta combinacion parpadean los leds en dos ciclos, dando inicio al juego.
III) index_flux = 1, index_game = 1: en esta parte el juego muestra la secuencia a repetir.
IV) index_flux = 1, index_game = 2: momento en el que el jugador debe repetir la secuencia. Si acierta, vuelve a la fase anterior con las condiciones actualizadas (puntaje, largo de la secuencia, etc.)
V) index_flux = 2, index_game = 0: se activa la condicion de derrota, donde se pregunta si se quiere jugar nuevamente con el mismo nombre, o jugar con otro.

Funciones bien implementadas: btn y switch handler, flujo general del programa, asignaciones de memoria para expandir el dato, conteo preciso en el timer, muestra del ranking, actualizacion de los jugadores y sus datos, generacion aleatoria de la secuencia.

Funciones mas o menos implementadas: expansion de la secuencia, comparacion entre lo insertado y la secuencia.

Aspectos que no funcionan: interrupcion del flujo general mediante el switch (apagar el juego en cualquier momento y dar la instruccion de activar cuando esta apagado, pero esta instruccion SI aparece al encenderlo), retorno de fase IV a III tras un acierto de secuencia.

