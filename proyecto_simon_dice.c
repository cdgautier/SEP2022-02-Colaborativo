
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
	printf("Bienvenido a simon dice!")
	printf("ingresa nombre de usuario")
	scanf() // nombre
	a = funcion_verificar_username();
	if (a){

	funcion_menu_principal();
	// btn_handler() depende de indice_etapa_juego
	// si indice_etapa_juego = 0 ->  btn_handler() manipula variable global seleccion_principal
	// para seleccionar en main() juego o ranking
	// si indice_etapa_juego = 1 ->  btn_handler() manipula repeticion de secuencia
	if seleccion_principal == 0{
		ranking();
		funcion_menu_principal();
	}
	while(seleccion_principal){
		counter_rondas = 0;
		control_juego = 0;
		//tmr_handler() depende de control_juego
		si control_juego = 0 -> tmr_handler() ejecuta secuencia incial;
		tmr_handler();
		++control_juego;
		si control_juego = 1 -> tmr_handler() ejecuta secuencias a repetir con random;
		++indice_etapa_juego; 
		tmr_handler();

	if indice_etapa_juego == 2{
	protocolo de fin de juego
	}	


	}


	}


	
}



btn_habndler(){
if indice etapa juego = 0{

}
}


*/