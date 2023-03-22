#include "teclado_TL04.h"

const char tecladoTL04[NUM_FILAS_TECLADO][NUM_COLUMNAS_TECLADO] = {
		{'1', '2', '3', 'C'},
		{'4', '5', '6', 'D'},
		{'7', '8', '9', 'E'},
		{'A', '0', 'B', 'F'}
};

// Maquina de estados: lista de transiciones
// {EstadoOrigen, CondicionDeDisparo, EstadoFinal, AccionesSiTransicion }
fsm_trans_t g_fsmTransExcitacionColumnas[] = {
		{ TECLADO_ESPERA_COLUMNA, CompruebaTimeoutColumna, TECLADO_ESPERA_COLUMNA, TecladoExcitaColumna },
		{-1, NULL, -1, NULL },
};

static TipoTecladoShared g_tecladoSharedVars;

//------------------------------------------------------
// FUCNIONES DE INICIALIZACION DE LAS VARIABLES ESPECIFICAS
//------------------------------------------------------
void ConfiguraInicializaTeclado(TipoTeclado *p_teclado) {
// A completar por el alumno...

	// Inicializacion de elementos de la variable global de tipo TipoTecladoShared:
	// 1. Valores iniciales de todos los "debounceTime"
	int arrayAux[4] = {0, 0, 0, 0};
	memcpy(g_tecladoSharedVars.debounceTime, arrayAux, sizeof(arrayAux));
	// 2. Valores iniciales de todos "columnaActual", "teclaDetectada" y "flags"
	g_tecladoSharedVars.columnaActual = 0;
	g_tecladoSharedVars.teclaDetectada = 'M';
	g_tecladoSharedVars.flags = 0;

	// Inicializacion de elementos de la estructura TipoTeclado:
	// Inicializacion del HW:
	// 1. Configura GPIOs de las columnas:
	// 	  (i) Configura los pines y (ii) da valores a la salida
	// 2. Configura GPIOs de las filas:
	// 	  (i) Configura los pines y (ii) asigna ISRs (y su polaridad)
	static int gpio_col[4] = {GPIO_KEYBOARD_COL_1, GPIO_KEYBOARD_COL_2, GPIO_KEYBOARD_COL_3, GPIO_KEYBOARD_COL_4};
	static int gpio_row[4] = {GPIO_KEYBOARD_ROW_1, GPIO_KEYBOARD_ROW_2, GPIO_KEYBOARD_ROW_3, GPIO_KEYBOARD_ROW_4};
	int i=0;
	for(i=0; i<4; i++) {
		pinMode(gpio_col[i], OUTPUT);
		pinMode(gpio_row[i], INPUT);
		pullUpDnControl (gpio_row[i], PUD_DOWN);
		//WIRINGPI para configurar
	}
	int j=0;
	for(j=0; j<4; j++) {
		digitalWrite(gpio_col[j], LOW);
	}

	 wiringPiISR(p_teclado->filas[0], INT_EDGE_RISING, teclado_fila_1_isr);
	 wiringPiISR(p_teclado->filas[1], INT_EDGE_RISING, teclado_fila_2_isr);
	 wiringPiISR(p_teclado->filas[2], INT_EDGE_RISING, teclado_fila_3_isr);
	 wiringPiISR(p_teclado->filas[3], INT_EDGE_RISING, teclado_fila_4_isr);

	// Inicializacion del temporizador:
	// 3. Crear y asignar temporizador de excitacion de columnas
	tmr_t* timer = tmr_new(timer_duracion_columna_isr);
	p_teclado->tmr_duracion_columna = timer;
	// 4. Lanzar temporizador
	tmr_startms(timer,TIMEOUT_COLUMNA_TECLADO_MS);


}

//------------------------------------------------------
// FUNCIONES PROPIAS
//------------------------------------------------------
/* Getter y setters de variables globales */
TipoTecladoShared GetTecladoSharedVar() {
	TipoTecladoShared result;
	piLock(KEYBOARD_KEY);
	result = g_tecladoSharedVars;
	piUnlock(KEYBOARD_KEY);
	return result;
}
void SetTecladoSharedVar(TipoTecladoShared value) {
	piLock(KEYBOARD_KEY);
	g_tecladoSharedVars = value;
	piUnlock(KEYBOARD_KEY);

}

void ActualizaExcitacionTecladoGPIO(int columna) {
	// ATENCION: Evitar que este mas de una columna activa a la vez.
	// A completar por el alumno
	// ...
	switch(columna){
	case COLUMNA_1:
		digitalWrite(GPIO_KEYBOARD_COL_1, HIGH);
		digitalWrite(GPIO_KEYBOARD_COL_2, LOW);
		digitalWrite(GPIO_KEYBOARD_COL_3, LOW);
		digitalWrite(GPIO_KEYBOARD_COL_4, LOW);
		break;
	case COLUMNA_2:
		digitalWrite(GPIO_KEYBOARD_COL_2, HIGH);
		digitalWrite(GPIO_KEYBOARD_COL_3, LOW);
		digitalWrite(GPIO_KEYBOARD_COL_4, LOW);
		digitalWrite(GPIO_KEYBOARD_COL_1, LOW);
		break;
	case COLUMNA_3:
		digitalWrite(GPIO_KEYBOARD_COL_3, HIGH);
		digitalWrite(GPIO_KEYBOARD_COL_4, LOW);
		digitalWrite(GPIO_KEYBOARD_COL_1, LOW);
		digitalWrite(GPIO_KEYBOARD_COL_2, LOW);
		break;
	case COLUMNA_4:
		digitalWrite(GPIO_KEYBOARD_COL_4, HIGH);
		digitalWrite(GPIO_KEYBOARD_COL_1, LOW);
		digitalWrite(GPIO_KEYBOARD_COL_2, LOW);
		digitalWrite(GPIO_KEYBOARD_COL_3, LOW);
		break;
	// ...
	}
}

//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------
int CompruebaTimeoutColumna(fsm_t* p_this) {
	int result = 0;
	piLock(KEYBOARD_KEY);
	result = g_tecladoSharedVars.flags & FLAG_TIMEOUT_COLUMNA_TECLADO;
	piUnlock(KEYBOARD_KEY);


	return result;
}


//------------------------------------------------------
// FUNCIONES DE SALIDA O DE ACCION DE LAS MAQUINAS DE ESTADOS
//------------------------------------------------------
void TecladoExcitaColumna(fsm_t* p_this) {
	TipoTeclado *p_teclado = (TipoTeclado*)(p_this->user_data);
	piLock(KEYBOARD_KEY);
	int columnaActual = g_tecladoSharedVars.columnaActual;
	// 1. Actualizo que columna SE VA a excitar
	if (columnaActual == COLUMNA_1) {
		g_tecladoSharedVars.columnaActual = COLUMNA_2;
	} else if (columnaActual == COLUMNA_2) {
		g_tecladoSharedVars.columnaActual = COLUMNA_3;
	} else if (columnaActual == COLUMNA_3) {
		g_tecladoSharedVars.columnaActual = COLUMNA_4;
	} else if (columnaActual == COLUMNA_4) {
		g_tecladoSharedVars.columnaActual = COLUMNA_1;
	}
	// 2. Ha pasado el timer y es hora de excitar la siguiente columna:

	//    (i) Llamada a ActualizaExcitacionTecladoGPIO con columna A ACTIVAR como argumento
	ActualizaExcitacionTecladoGPIO(g_tecladoSharedVars.columnaActual);
	// 3. Actualizar la variable flags
	g_tecladoSharedVars.flags &= ~FLAG_TIMEOUT_COLUMNA_TECLADO;
	piUnlock(KEYBOARD_KEY);
	// 4. Manejar el temporizador para que vuelva a avisarnos
	tmr_startms((tmr_t*)(p_teclado->tmr_duracion_columna), TIMEOUT_COLUMNA_TECLADO_MS);
	// A completar por el alumno
	// ...


}

//------------------------------------------------------
// SUBRUTINAS DE ATENCION A LAS INTERRUPCIONES
//------------------------------------------------------
void teclado_fila_1_isr(void) {
	// 1. Comprobar si ha pasado el tiempo de guarda de anti-rebotes
	//void myInterrupt0 (void) {

	//milli() es el tiempo en este instante
	int milli = millis () + DEBOUNCE_TIME_MS;
	if (millis() < g_tecladoSharedVars.debounceTime[0]) {
		g_tecladoSharedVars.debounceTime[0] = milli;
		return;
	}

	// 2. Atender a la interrupcion:
	piLock(KEYBOARD_KEY);
	// 	  (i) Guardar la tecla detectada en g_tecladoSharedVars
	g_tecladoSharedVars.teclaDetectada = tecladoTL04[0][g_tecladoSharedVars.columnaActual];
	//    (ii) Activar flag para avisar de que hay una tecla pulsada
	g_tecladoSharedVars.flags |= FLAG_TECLA_PULSADA;
	// 3. Actualizar el tiempo de guarda del anti-rebotes
	g_tecladoSharedVars.debounceTime[0] = milli;
	piUnlock(KEYBOARD_KEY);
}

void teclado_fila_2_isr(void) {
	// 1. Comprobar si ha pasado el tiempo de guarda de anti-rebotes
	//void myInterrupt0 (void) {

		int milli = millis () + DEBOUNCE_TIME_MS;
		if (millis() < g_tecladoSharedVars.debounceTime[1]) {
			g_tecladoSharedVars.debounceTime[1] = milli;
			return;
		}

		// 2. Atender a la interrupcion:
		piLock(KEYBOARD_KEY);
		// 	  (i) Guardar la tecla detectada en g_tecladoSharedVars
		g_tecladoSharedVars.teclaDetectada = tecladoTL04[1][g_tecladoSharedVars.columnaActual];
		//    (ii) Activar flag para avisar de que hay una tecla pulsada
		g_tecladoSharedVars.flags |= FLAG_TECLA_PULSADA;

		// 3. Actualizar el tiempo de guarda del anti-rebotes
		g_tecladoSharedVars.debounceTime[1] = milli;
		piUnlock(KEYBOARD_KEY);
}

void teclado_fila_3_isr(void) {
	// 1. Comprobar si ha pasado el tiempo de guarda de anti-rebotes
		//void myInterrupt0 (void) {

		int milli = millis () + DEBOUNCE_TIME_MS;
		if (millis() < g_tecladoSharedVars.debounceTime[2]) {
			g_tecladoSharedVars.debounceTime[2] = milli;
			return;
		}

		// 2. Atender a la interrupcion:
		piLock(KEYBOARD_KEY);
		// 	  (i) Guardar la tecla detectada en g_tecladoSharedVars
		g_tecladoSharedVars.teclaDetectada = tecladoTL04[2][g_tecladoSharedVars.columnaActual];
		//    (ii) Activar flag para avisar de que hay una tecla pulsada
		g_tecladoSharedVars.flags |= FLAG_TECLA_PULSADA;
		// 3. Actualizar el tiempo de guarda del anti-rebotes
		g_tecladoSharedVars.debounceTime[2] = milli;
		piUnlock(KEYBOARD_KEY);
}

void teclado_fila_4_isr (void) {
	// 1. Comprobar si ha pasado el tiempo de guarda de anti-rebotes
		//void myInterrupt0 (void) {

		int milli = millis () + DEBOUNCE_TIME_MS;
		if (millis() < g_tecladoSharedVars.debounceTime[3]) {
			g_tecladoSharedVars.debounceTime[3] = milli;
			return;
		}

		// 2. Atender a la interrupcion:
		piLock(KEYBOARD_KEY);
		// 	  (i) Guardar la tecla detectada en g_tecladoSharedVars
		g_tecladoSharedVars.teclaDetectada = tecladoTL04[3][g_tecladoSharedVars.columnaActual];
		//    (ii) Activar flag para avisar de que hay una tecla pulsada
		g_tecladoSharedVars.flags |= FLAG_TECLA_PULSADA;
		// 3. Actualizar el tiempo de guarda del anti-rebotes
		g_tecladoSharedVars.debounceTime[3] = milli;
		piUnlock(KEYBOARD_KEY);
}

void timer_duracion_columna_isr(union sigval value) {
	// Simplemente avisa que ha pasado el tiempo para excitar la siguiente columna
	piLock(KEYBOARD_KEY);
	g_tecladoSharedVars.flags |= FLAG_TIMEOUT_COLUMNA_TECLADO;
	piUnlock(KEYBOARD_KEY);

	// A completar por el alumno
	// ...
}
