
/*
// 1. GPIO

// 2. UART

// 3. Timers

// 4. Interrupciones

*/

/*
Flujo

1) SW0 -> enciende el juego
2) ingresar nombre en terminal vitis
3) verificar exiistencia de nombre
4) menu de botones op1: jugar, op2: ranking
5) juego inicia con secuencia de parpadeos
6) generacion y reproduccion de secuencia de leds
7) repeticion de secuencia con botones
8) analisis de la repeticion
9) secuencia de game over en caso de perder
10) enviar a consola tiempo de juego y rondas completadas + estructura de puntajes
11) fin del juego -> preguntar si (11.1) juega otra vez o si (11.2) ingresa otro usuario
		si elige (11.2) -> 2).
*/

/*

counter_inicio = 0;

if (not switch and (counter_inicio == 0)){

	printf("enciende el switch 0 para iniciar el juego");
	++counter_inicio;
}

while(switch){
	indice_switch = 1;
	while (indice_switch){
		printf("Bienvenido a simon dice!")
		printf("ingresa nombre de usuario")
		scanf() // nombre
		autentificacion = funcion_verificar_username();
		while(autentificacion){
			indice_flujo = 0;
			// btn_handler() depende de indice_flujo
			// if indice_flujo = 0 ->  btn_handler() muestra menu_inicio
			// if indice_flujo = 1 ->  btn_handler() almacenó seleccion_juego
			if seleccion_juego == 0001{
				ranking();
				indice_flujo = 0
			}
			while(seleccion_juego == 0010){
				counter_rondas = 0;
				indice_juego = 0;
				//tmr_handler() depende de indice_juego
				si indice_juego = 0 -> tmr_handler() ejecuta parpadeo de los cuatro LEDs por 2 segundos, con una frecuencia de medio segundo.
				tmr_handler(); 
				++indice_juego;
				si indice_juego = 1 -> tmr_handler() ejecuta secuencias a repetir de los LEDs con random;
				tmr_handler();
				btn_handler(); -> si indice_flujo = 1 y indice_juego = 1, btn_handler() escribe en seleccion_juego lo que recibe de los botones
				++indice_flujo; 
				

				if indice_flujo == 2{
					indice_juego = 0;
					tmr_handler() ejecuta parpadeo de los 4 LEDs tendr´an que parpadear 3 veces para indicar que se terminó el juego. Frecuencia de medio segundo,
					btn_handler() pasa a seleccion_postjuego
					si seleccion_postjuego = 01 -> mismo jugador, indice_flujo = 0, indice_juego = 0;
					si seleccion_postjuego = 10 -> otro jugador, autentificacion = 0, indice_flujo = 0, indice_juego = 0;
				}	
			}
		}
	}	
}



btn_handler(){

	if (indice_flujo = 0 and indice_juego = 0){
		funcion_menu_inicio(); -> muestra menu inicio y queda atento a seleccion_juego
	}
	else if (indice_flujo = 1 and indice_juego = 0){
		almacenó seleccion_juego 0001, 0010 
	}
	else if (indice_flujo = 1 and indice_juego = 1){
		se genera elemento random en el array con maloc
		suma_comparacion = 0;
		for(int i=0; i < len(array); ++i ){
			counter 30 seg...
			if (tiempo_vida < 30){
				if not (seleccion_juego == array[i]){
					printf(error en secuencia uwu)
					comparacion = 0;
					break;
				}
				else{
					++suma_comparacion;
					tiempo_vida = 0;
				}
			}
			else{
				printf(te quedaste sin tiempo uwu)
				comparacion = 0;
				break;
			}
		}
		if suma_comparacion = len(array){
			++counter_rondas;
		}
		if comparacion = 0{
			printf(game over)
			indice_juego = 0
			indice_flujo = 2
		}
	}
	else if (indice_flujo = 2 and indice_juego = 0){
		almacenó seleccion_postjuego 0001, 0010 
	}

tmr_handler(){
	if (indice_flujo = 1 and indice_juego = 0){
		muestra secuencia de inicio de leds
	}
	else if (indice_flujo = 1 and indice_juego = 1){
		ejecuta secuencias a repetir de los LEDs con random;
	}
	else if (indice_flujo = 2 and indice_juego = 0){
		ejecuta parpadeo de los 4 LEDs tendr´an que parpadear 3 veces para indicar que se terminó el juego. Frecuencia de medio segundo
	}
}

swt_handler(){
	switch-> sigue al switch analogico
}
			

*/