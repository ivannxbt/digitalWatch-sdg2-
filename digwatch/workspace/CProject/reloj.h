#ifndef RELOJ_H_
#define RELOJ_H_
// INCLUDES
#include "systemConfig.h"
#include "util.h"
//DEFINES Y ENUMS
enum FSM_ESTADOS_RELOJ {
	WAIT_TIC = 0,
};

#define DEFAULT_DAY 29
#define DEFAULT_MONTH 2
#define DEFAULT_YEAR 2020

#define DEFAULT_HOUR 00
#define DEFAULT_MIN 00
#define DEFAULT_SEC 00

#define MIN_HOUR 0
#define MAX_HOUR 23
#define MAX_MIN 59

#define TIME_FORMAT_12_H 12
#define TIME_FORMAT_24_H 24
#define MAX_MONTH 12

#define DEFAULT_TIME_FORMAT 24

#define PRECISION_RELOJ_MS 1000

//FLAGS FSM
#define FLAG_ACTUALIZA_RELOJ 0x01
#define FLAG_TIME_ACTUALIZADO 0x02

//DECLARACION ESTRUCTURAS
typedef struct {
	int dd;
	int MM;
	int yyyy;
} TipoCalendario;

typedef struct {
	int hh;
	int mm;
	int ss;
	int formato;
} TipoHora;

typedef struct {
	int timestamp;
	TipoHora hora;
	TipoCalendario calendario;
	tmr_t* tmrTic;
} TipoReloj;

typedef struct {
	int flags;
} TipoRelojShared;

//DECLARACION VARIABLES
extern fsm_trans_t g_fsmTransReloj [];
extern const int DIAS_MESES[2][12];


//DEFINICION VARIABLES

//FUNCIONES DE INICIALIZACION DE LAS VARIABLES
int ConfiguraInicializaReloj(TipoReloj *p_reloj);
void ResetReloj(TipoReloj *p_reloj);

//FUNCIONES PROPIAS
void ActualizaFecha(TipoCalendario *p_fecha);
void ActualizaHora(TipoHora *p_hora);
int CalculaDiasMes(int month, int year);
int ConfiguraInicializaReloj(TipoReloj *p_reloj);
int EsBisiesto(int year);
TipoRelojShared GetRelojSharedVar();
void ResetReloj(TipoReloj *p_reloj);
int SetFecha(int nuevaFceha, TipoCalendario *p_fecha);
int SetFormato(int nuevoFormato, TipoHora *p_hora);
int SetHora(int nuevaHora, TipoHora *p_hora);
void SetRelojSharedVar(TipoRelojShared value);

//FUNCIONES DE ENTRADA DE LA MAQUINA DE ESTADOS
int CompruebaTic (fsm_t *p_this);

//FUNCIONES DE SALIDA DE LA MAQUINA DE ESTADOS
void ActualizaReloj(fsm_t *pthis);

//SUBRUTINAS DE ATENCION A LAS INTERRUPCIONES
void tmr_actualiza_reloj_isr(union sigval value);
//FUNCIONES LIGADAS A THREADS ADICIONALES
#endif /* RELOJ_H_ */
