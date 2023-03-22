/*
 * reloj.c
 *
 *  Created on: 10 de feb. de 2022
 *      Author: alumno
 */
#include "reloj.h"

fsm_trans_t g_fsmTransReloj [] = {
		{WAIT_TIC, CompruebaTic ,WAIT_TIC, ActualizaReloj},
		{-1, NULL, -1, NULL}
};

const int DIAS_MESES[2][MAX_MONTH] = {{31,29,31,30,31,30,31,31,30,31,30,31},{31,28,31,30,31,30,31,31,30,31,30,31}};
static TipoRelojShared g_relojSharedVars;


void ResetReloj(TipoReloj *p_reloj){
	TipoCalendario calendario;
	calendario.dd = DEFAULT_DAY;
	calendario.MM = DEFAULT_MONTH;
	calendario.yyyy = DEFAULT_YEAR;
	p_reloj->calendario = calendario;

	p_reloj->hora.hh = DEFAULT_HOUR;
	p_reloj->hora.mm = DEFAULT_MIN;
	p_reloj->hora.ss = DEFAULT_SEC;
	p_reloj->hora.formato = DEFAULT_TIME_FORMAT;

	p_reloj->timestamp = 0;

	piLock(RELOJ_KEY);
	g_relojSharedVars.flags = 0;
	piUnlock(RELOJ_KEY);

}

int ConfiguraInicializaReloj(TipoReloj *p_reloj) {
	ResetReloj(p_reloj);
	tmr_t* timer = tmr_new(tmr_actualiza_reloj_isr);
	p_reloj->tmrTic = timer;
	tmr_startms_periodic(timer,PRECISION_RELOJ_MS);
	return 0;
}

int CompruebaTic(fsm_t* p_this) {
	int comprueba1(fsm_t* this);
	int comprueba0(fsm_t* this);
	int result;
	piLock(RELOJ_KEY);
	result = (g_relojSharedVars.flags & FLAG_ACTUALIZA_RELOJ);
	piUnlock(RELOJ_KEY);
	return result;
}

void ActualizaReloj (fsm_t* p_this) {
	TipoReloj *p_miReloj = (TipoReloj*)(p_this->user_data);
	p_miReloj->timestamp += 1;
	ActualizaHora(&p_miReloj->hora);
	if ((p_miReloj->hora.hh==0) && (p_miReloj->hora.mm == 0) && (p_miReloj->hora.ss == 0)) {
		ActualizaFecha(&p_miReloj->calendario);
	}
	piLock(RELOJ_KEY);
	g_relojSharedVars.flags &= ~FLAG_ACTUALIZA_RELOJ;
	g_relojSharedVars.flags |= FLAG_TIME_ACTUALIZADO;
	piUnlock(RELOJ_KEY);


}

void tmr_actualiza_reloj_isr (union sigval value) {
	piLock(RELOJ_KEY);
	g_relojSharedVars.flags |= FLAG_ACTUALIZA_RELOJ;
	piUnlock(RELOJ_KEY);
}

void ActualizaFecha(TipoCalendario *p_fecha) {
	int diasMes = CalculaDiasMes(p_fecha->MM, p_fecha->yyyy)+1;
	int diaMas1 = p_fecha->dd+1;
	int moduloDiasMesMas1 = diaMas1%(diasMes); // 32%31=1
	p_fecha->dd = MAX(moduloDiasMesMas1, 1);

	if (p_fecha->dd == 1) {
		int mesActualMas1 = (p_fecha->MM) + 1;
		int moduloMesMas1 = mesActualMas1%(MAX_MONTH+1); //13%13
		p_fecha->MM = MAX(moduloMesMas1, 1);
	} if (p_fecha->MM == 1) {
		p_fecha->yyyy = p_fecha->yyyy + 1;
	}
}

void ActualizaHora (TipoHora *p_hora) {
	int horaSegundosMas1 = p_hora->ss+1;
	p_hora->ss = (horaSegundosMas1)%60;
	if (p_hora->ss==0) {
		int horaMinutosMas1 = p_hora->mm+1;
		p_hora->mm = horaMinutosMas1 % 60;
	}
	if ((p_hora->ss==0 ) && (p_hora->mm==0)) {
		if ( p_hora->formato == TIME_FORMAT_12_H ) {
			p_hora->hh = (p_hora->hh+1)%(p_hora->formato+1);
		} else if (p_hora->formato == TIME_FORMAT_24_H) {
			p_hora->hh = (p_hora->hh+1)%(p_hora->formato);
		}
	}
}

int CalculaDiasMes(int month, int year) {
	int diasMes = DIAS_MESES[EsBisiesto(year)][month-1];
	return diasMes;
}

int EsBisiesto(int year) {
	if (year%4==0) {return 0;}
	if (year%100 && year%400) {return 1;}
	return 1;
}

int SetHora(int horaInt, TipoHora *p_hora) {
	if (horaInt<0) {return 1;}
	int result = 0;
	int numero_digitos = 0;
	int auxiliar = horaInt;
	int hora_extraida;
	int minutoExtraido;
	do {
		auxiliar /= 10;
		numero_digitos += 1;
	} while (auxiliar != 0 );

	if (numero_digitos<0 || numero_digitos>4) {
		return 1;
	}

	if(numero_digitos==3 || numero_digitos==4) {
		hora_extraida = horaInt/100;
	} else if (numero_digitos ==1 || numero_digitos == 2) {
		hora_extraida = 0;
	}

	if (hora_extraida>MAX_HOUR) {
		hora_extraida = MAX_HOUR;
	}

	if (((p_hora->formato) == TIME_FORMAT_12_H) && (hora_extraida>12)) {
		horaInt -= 1200;
	}
	if (((p_hora->formato) == TIME_FORMAT_12_H) && (hora_extraida==0)) {
		horaInt += 1200;
	}

	if(numero_digitos==3 || numero_digitos==4) {
		int horaAux = horaInt/100;
		minutoExtraido = horaInt - horaAux*100;
	} else if (numero_digitos ==1 || numero_digitos == 2) {
		minutoExtraido = horaInt;
	}
	if (minutoExtraido>MAX_MIN) {
		minutoExtraido = MAX_MIN;
	}

	p_hora->hh = hora_extraida;
	p_hora->mm = minutoExtraido;
	p_hora->ss = result;
	return result;
}

TipoRelojShared GetRelojSharedVar() {
	piLock(RELOJ_KEY);
	TipoRelojShared aux = g_relojSharedVars;
	piUnlock(RELOJ_KEY);
	return aux;
}

void SetRelojSharedVar(TipoRelojShared value) {
	piLock(RELOJ_KEY);
	g_relojSharedVars = value;
	piUnlock(RELOJ_KEY);
}


