#include "coreWatch.h"


fsm_trans_t g_fsmTransCoreWatch[] = {
		{START, CompruebaSetupDone, STAND_BY, Start},
		{STAND_BY, CompruebaTimeActualizado, STAND_BY, ShowTime},
		{STAND_BY, CompruebaReset, STAND_BY, Reset},
		{STAND_BY, CompruebaSetCancelNewTime, SET_TIME, PrepareSetNewTime},
		{SET_TIME, CompruebaSetCancelNewTime, STAND_BY, CancelSetNewTime},
		{SET_TIME, CompruebaNewTimeIsReady, STAND_BY, SetNewTime},
		{SET_TIME, CompruebaDigitoPulsado, SET_TIME, ProcesaDigitoTime},
		{-1, NULL, -1, NULL}
};

fsm_trans_t g_fsmTransDeteccionComandos[] = {
		{WAIT_COMMAND, CompruebaTeclaPulsada, WAIT_COMMAND, ProcesaTeclaPulsada},
		{-1, NULL, -1, NULL}
};
//------------------------------------------------------
// FUNCIONES PROPIAS
//------------------------------------------------------
// Wait until next_activation (absolute time)
// Necesita de la función "delay" de WiringPi.
void DelayUntil(unsigned int next) {
	unsigned int now = millis();
	if (next > now) {
		delay(next - now);
	}
}

//PI_THREAD(ThreadExploraTecladoPC) {
//	int teclaPulsada;
//	while(1) {
//		delay(10);
//		if(kbhit()) {
//			teclaPulsada = kbread();
//			if (teclaPulsada == TECLA_RESET) {
//				piLock(SYSTEM_KEY);
//				g_flagsCoreWatch |= FLAG_RESET;
//				piUnlock(SYSTEM_KEY);
//			}
//			else if (teclaPulsada == TECLA_SET_CANCEL_TIME) {
//				piLock(SYSTEM_KEY);
//				g_flagsCoreWatch |= FLAG_SET_CANCEL_NEW_TIME;
//				piUnlock(SYSTEM_KEY);
//			}
//			else if (EsNumero(teclaPulsada)) {
//				g_coreWatch.digitoPulsado = teclaPulsada-48;
//				piLock(SYSTEM_KEY);
//				g_flagsCoreWatch |= FLAG_DIGITO_PULSADO;
//				piUnlock(SYSTEM_KEY);
//			}
//			else if (teclaPulsada == TECLA_EXIT) {
//				piLock(STD_IO_LCD_BUFFER_KEY);
//				printf("Se sale del sistema --> ");
//				piUnlock(STD_IO_LCD_BUFFER_KEY);
//
//				exit(0);
//			}
//			else printf("Tecla desconocida");
//		}
//	}
//}

int ConfiguraInicializaSistema (TipoCoreWatch *p_sistema) {

	g_flagsCoreWatch = 0;
	p_sistema->tempTime = 0;
	p_sistema->digitosGuardados = 0;
	int auxReloj = ConfiguraInicializaReloj(&p_sistema->reloj);
//	piThread = piThreadCreate(ThreadExploraTecladoPC);
	if(auxReloj != 0) {
		return 1;
	}

	int  arrayColumnas[NUM_COLUMNAS_TECLADO] = {GPIO_KEYBOARD_COL_1, GPIO_KEYBOARD_COL_2, GPIO_KEYBOARD_COL_3, GPIO_KEYBOARD_COL_4};
	int  arrayFilas[NUM_FILAS_TECLADO] = {GPIO_KEYBOARD_ROW_1, GPIO_KEYBOARD_ROW_2, GPIO_KEYBOARD_ROW_3, GPIO_KEYBOARD_ROW_4};

	memcpy(p_sistema->teclado.filas, arrayFilas, sizeof(arrayFilas));
	memcpy(p_sistema->teclado.columnas, arrayColumnas, sizeof(arrayColumnas));
	int aux = wiringPiSetupGpio();
	if (aux != 0) { return 1; }
	ConfiguraInicializaTeclado(&p_sistema->teclado);

	p_sistema->lcdId = lcdInit(NUM_ROWS, NUM_COLS, NUM_BITS, GPIO_LCD_RS, GPIO_LCD_EN, GPIO_LCD_D0,
			GPIO_LCD_D1, GPIO_LCD_D2, GPIO_LCD_D3, GPIO_LCD_D4, GPIO_LCD_D5, GPIO_LCD_D6, GPIO_LCD_D7);

	piLock(SYSTEM_KEY);
	g_flagsCoreWatch |= FLAG_SETUP_DONE;
	piUnlock(SYSTEM_KEY);

	return 0;
}

int EsNumero(char value) {
	if ((value > 0x29) && (value < 0x40)) {
		return 1;
	} else return 0;
}

int CompruebaDigitoPulsado(fsm_t* p_this) {
	int result;
	piLock(SYSTEM_KEY);
	result = (g_flagsCoreWatch & FLAG_DIGITO_PULSADO);
	piUnlock(SYSTEM_KEY);
	return result;
}

int CompruebaNewTimeIsReady(fsm_t* p_this) {
	int result;
	piLock(SYSTEM_KEY);
	result = (g_flagsCoreWatch & FLAG_NEW_TIME_IS_READY);
	piUnlock(SYSTEM_KEY);
	return result;
}

int CompruebaReset(fsm_t* p_this) {
	int result;
	piLock(SYSTEM_KEY);
	result = (g_flagsCoreWatch & FLAG_RESET);
	piUnlock(SYSTEM_KEY);
	return result;
}

int CompruebaSetCancelNewTime(fsm_t* p_this) {
	int result;
	piLock(SYSTEM_KEY);
	result = (g_flagsCoreWatch & FLAG_SET_CANCEL_NEW_TIME);
	piUnlock(SYSTEM_KEY);
	return result;
}

int CompruebaSetupDone(fsm_t* p_this) {
	int result;
	piLock(SYSTEM_KEY);
	result = (g_flagsCoreWatch & FLAG_SETUP_DONE);
	piUnlock(SYSTEM_KEY);
	return result;
}

int CompruebaTeclaPulsada (fsm_t* p_this) {
	int result;
	piLock(SYSTEM_KEY);
	result = (GetTecladoSharedVar().flags & FLAG_TECLA_PULSADA);
	piUnlock(SYSTEM_KEY);
	return result;

}

int CompruebaTimeActualizado (fsm_t* p_this) {
	int result;
	piLock(SYSTEM_KEY);
	result = (GetRelojSharedVar().flags & FLAG_TIME_ACTUALIZADO);
	piUnlock(SYSTEM_KEY);
	return result;
}

void Start(fsm_t* p_this) {
	piLock(SYSTEM_KEY);
	g_flagsCoreWatch &= ~FLAG_SETUP_DONE;
	piUnlock(SYSTEM_KEY);
}

void ShowTime(fsm_t* p_this) {
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	TipoRelojShared auxflag = GetRelojSharedVar();
	auxflag.flags &= ~FLAG_TIME_ACTUALIZADO;
	SetRelojSharedVar(auxflag);

	piLock(STD_IO_LCD_BUFFER_KEY);
#if VERSION <4
	printf("Son las: %d:%d:%d del %d/%d/%d\n", g_coreWatch.reloj.hora.hh, g_coreWatch.reloj.hora.mm, g_coreWatch.reloj.hora.ss, g_coreWatch.reloj.calendario.dd, g_coreWatch.reloj.calendario.MM, g_coreWatch.reloj.calendario.yyyy);
#endif
#if VERSION >= 4
	int auxlcdId = p_sistema->lcdId;
	lcdClear(auxlcdId);
	lcdPrintf(auxlcdId,"%d: %d: %d", p_sistema->reloj.hora.hh, p_sistema->reloj.hora.mm, p_sistema->reloj.hora.ss);
	lcdPosition(auxlcdId, 0, 1);
	lcdPrintf(auxlcdId,"%d/%d/%d", p_sistema->reloj.calendario.dd, p_sistema->reloj.calendario.MM, p_sistema->reloj.calendario.yyyy);
	lcdPosition(auxlcdId, 0, 1);
#endif
	fflush(stdout);
	piUnlock(STD_IO_LCD_BUFFER_KEY);
}

void Reset(fsm_t* p_this) {
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	ResetReloj(&p_sistema->reloj);
	piLock(SYSTEM_KEY);
	g_flagsCoreWatch &= ~FLAG_RESET;
	piUnlock(SYSTEM_KEY);
	piLock(STD_IO_LCD_BUFFER_KEY);
#if VERSION <4
	printf("[RESET] Hora reiniciada \n");
#endif
#if VERSION >= 4
	lcdClear(p_sistema->lcdId);
	lcdPosition(p_sistema->lcdId, 0, 1);
	lcdPrintf(p_sistema->lcdId, "RESET");
	sleep(ESPERA_MENSAJE_MS);
	lcdClear(p_sistema->lcdId);
#endif
	piUnlock(STD_IO_LCD_BUFFER_KEY);
}

void PrepareSetNewTime(fsm_t* p_this) {
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	int auxFormat = p_sistema->reloj.hora.formato;
	piLock(SYSTEM_KEY);
	g_flagsCoreWatch &= ~FLAG_DIGITO_PULSADO;
	g_flagsCoreWatch &= ~FLAG_SET_CANCEL_NEW_TIME;
	piUnlock(SYSTEM_KEY);
	piLock(STD_IO_LCD_BUFFER_KEY);
#if VERSION <4
	printf("[SET_TIME] Introduzca la nueva hora en formato 0 - %d\n", auxFormat);
#endif
#if VERSION >= 4
	lcdClear(p_sistema->lcdId);
	lcdPosition(p_sistema->lcdId, 0,1);
	lcdPrintf(p_sistema->lcdId, "FORMAT: 0-%d\n", auxFormat);
#endif
	piUnlock(STD_IO_LCD_BUFFER_KEY);
}

void CancelSetNewTime(fsm_t* p_this) {
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	p_sistema->tempTime = 0;
	p_sistema->digitosGuardados = 0;
	piLock(SYSTEM_KEY);
	g_flagsCoreWatch &= ~FLAG_SET_CANCEL_NEW_TIME;
	piUnlock(SYSTEM_KEY);
	piLock(STD_IO_LCD_BUFFER_KEY);
#if VERSION <4
	printf("[SET_TIME] Operacion cancelada\n");
#endif
#if VERSION >= 4
	lcdClear(p_sistema->lcdId);
	lcdPosition(p_sistema->lcdId, 1,2); //PREGUNTARRR
	lcdPrintf(p_sistema->lcdId, "CANCELADO\n");
	sleep(ESPERA_MENSAJE_MS);
	lcdClear(p_sistema->lcdId);
#endif
	piUnlock(STD_IO_LCD_BUFFER_KEY);
}

void ProcesaDigitoTime(fsm_t* p_this) { //REVISAR ESTO
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	int auxtempTime = p_sistema->tempTime;
	int auxdigitosGuardados = p_sistema->digitosGuardados;
	int ultimoDigito = p_sistema->digitoPulsado;
	piLock(SYSTEM_KEY);
	g_flagsCoreWatch &= ~FLAG_DIGITO_PULSADO;
	piUnlock(SYSTEM_KEY);

	if (auxdigitosGuardados == 0) {
		if(p_sistema->reloj.hora.formato == 12) {
			ultimoDigito = MIN(1, ultimoDigito);
		} else {
			ultimoDigito = MIN(2, ultimoDigito);
		}
		auxtempTime = auxtempTime*10 + ultimoDigito;
		auxdigitosGuardados += 1;
	} else if(auxdigitosGuardados == 1) {
		if (p_sistema->reloj.hora.formato == 12) {
			if (auxtempTime == 0) {
				ultimoDigito = MAX(1, ultimoDigito);
			} else {
				ultimoDigito = MIN(2, ultimoDigito);
			}
		} else {
			if (auxtempTime == 2) {
				ultimoDigito = MIN(3, ultimoDigito);
			}
		}
		auxtempTime = auxtempTime*10 + ultimoDigito;
		auxdigitosGuardados += 1;
	} else if (auxdigitosGuardados == 2) {
		auxtempTime = auxtempTime*10 + MIN(5, ultimoDigito);
		auxdigitosGuardados += 1;
	} else {
		auxtempTime = auxtempTime*10 + ultimoDigito;
		piLock(SYSTEM_KEY);
		g_flagsCoreWatch &= ~FLAG_DIGITO_PULSADO;
		g_flagsCoreWatch |= FLAG_NEW_TIME_IS_READY;
		piUnlock(SYSTEM_KEY);
	}


	p_sistema->tempTime = auxtempTime;
	p_sistema->digitosGuardados = auxdigitosGuardados;

#if (VERSION < 4)
	piLock(STD_IO_LCD_BUFFER_KEY);
	printf("[SET_TIME] Nueva hora temporal %d\n", auxtempTime);
	fflush(stdout);
	piUnlock(STD_IO_LCD_BUFFER_KEY);

#endif
#if VERSION >= 4
	lcdPrintf(p_sistema->lcdId, "            "  );
	lcdPosition(p_sistema->lcdId, 1,0);
	lcdPrintf(p_sistema->lcdId, "SET: %d", p_sistema->tempTime);
#endif
}

void ProcesaTeclaPulsada(fsm_t* p_this) {
	TipoTecladoShared localVar = GetTecladoSharedVar();
	localVar.flags &= ~FLAG_TECLA_PULSADA;
	SetTecladoSharedVar(localVar);
	char teclaPulsada = localVar.teclaDetectada;
	if (teclaPulsada == TECLA_RESET) {
		piLock(SYSTEM_KEY);
		g_flagsCoreWatch |= FLAG_RESET;
		piUnlock(SYSTEM_KEY);
	}
	else if (teclaPulsada == TECLA_SET_CANCEL_TIME) {
		piLock(SYSTEM_KEY);
		g_flagsCoreWatch |= FLAG_SET_CANCEL_NEW_TIME;
		piUnlock(SYSTEM_KEY);
	}
	else if (EsNumero(teclaPulsada)) {
		g_coreWatch.digitoPulsado = teclaPulsada-48;
		piLock(SYSTEM_KEY);
		g_flagsCoreWatch |= FLAG_DIGITO_PULSADO;
		piUnlock(SYSTEM_KEY);
	}
	else if (teclaPulsada == TECLA_EXIT) {
		piLock(STD_IO_LCD_BUFFER_KEY);
		printf("Se sale del sistema --> ");
		piUnlock(STD_IO_LCD_BUFFER_KEY);
		exit(0);
	}
	else printf("Tecla desconocida");
}


void SetNewTime (fsm_t* p_this) {
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	piLock(SYSTEM_KEY);
	g_flagsCoreWatch &= ~FLAG_NEW_TIME_IS_READY;
	piUnlock(SYSTEM_KEY);
	SetHora(p_sistema->tempTime, &p_sistema->reloj.hora);
	p_sistema->tempTime = 0;
	p_sistema->digitosGuardados = 0;
}




//------------------------------------------------------
// MAIN
//------------------------------------------------------
int main() {
	unsigned int next;

#if VERSION==1
	TipoReloj miReloj;
	ConfiguraInicializaReloj(&miReloj);
	SetHora(1601,&miReloj.hora);
	//		printf("Son las: %d:%d:%d del %d/%d/%d\n", miReloj.hora.hh, miReloj.hora.mm, miReloj.hora.ss, miReloj.calendario.dd, miReloj.calendario.MM, miReloj.calendario.yyyy);
	while (1) {
		fsm_fire(fsmReloj);
		next += CLK_MS;
		DelayUntil(next);
		printf("Son las: %d:%d:%d del %d/%d/%d\n", g_coreWatch.reloj.hora.hh, g_coreWatch.reloj.hora.mm, g_coreWatch.reloj.hora.ss, g_coreWatch.reloj.calendario.dd, g_coreWatch.reloj.calendario.MM, g_coreWatch.reloj.calendario.yyyy);
	}
	fflush(stdout);
	fsm_t* fsmReloj= fsm_new(WAIT_TIC,g_fsmTransReloj,&(miReloj));
#endif

#if VERSION>=2
	int aux = ConfiguraInicializaSistema(&g_coreWatch);
	if (aux != 0) {
		printf("ERROR!!!!!");
		exit(0);
	}
	SetHora(2359,&g_coreWatch.reloj.hora);
	fsm_t* fsmReloj = fsm_new(WAIT_TIC, g_fsmTransReloj, &(g_coreWatch.reloj));
	fsm_t* fsmCoreWatch = fsm_new(START, g_fsmTransCoreWatch, &g_coreWatch);
	fsm_t* deteccionComandosFSM = fsm_new(WAIT_COMMAND, g_fsmTransDeteccionComandos, &(g_coreWatch));
	fsm_t* tecladoFSM = fsm_new(TECLADO_ESPERA_COLUMNA, g_fsmTransExcitacionColumnas, &(g_coreWatch.teclado));
#endif


	next = millis();
	while (1) {
		fsm_fire(fsmReloj);
		fsm_fire(deteccionComandosFSM);
		fsm_fire(tecladoFSM);
		fsm_fire(fsmCoreWatch);

		next += CLK_MS;
		DelayUntil(next);

	}
	tmr_destroy(g_coreWatch.reloj.tmrTic);
	fsm_destroy(fsmReloj);
	fsm_destroy(deteccionComandosFSM);
	fsm_destroy(tecladoFSM);
	fsm_destroy(fsmCoreWatch);


}
