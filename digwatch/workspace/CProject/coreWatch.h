#define COREWATCH_H_

// INCLUDES
// Propios:
#include "systemConfig.h"     // Sistema: includes, entrenadora (GPIOs, MUTEXes y entorno), setup de perifericos y otros otros.
#include "reloj.h"
#include "teclado_TL04.h"
#include "lcd.h"
// DEFINES Y ENUMS
enum FSM_ESTADOS_SISTEMA {
	START,
	STAND_BY,
	SET_TIME,
};

enum FSM_DETECCION_COMANDOS {
	WAIT_COMMAND,
};

#define TECLA_RESET 'F'
#define TECLA_SET_CANCEL_TIME 'E'
#define TECLA_EXIT 'B'

#define NUM_ROWS 2
#define NUM_COLS 12
#define NUM_BITS 8
#define ESPERA_MENSAJE_MS 1

// FLAGS FSM DEL SISTEMA CORE WATCH
#define FLAG_SETUP_DONE 0x01
#define FLAG_RESET 0x02
#define extern FLAG_TIME_ACTUALIZADO 0x04
#define FLAG_SET_CANCEL_NEW_TIME 0x08
#define FLAG_NEW_TIME_IS_READY 0x10
#define FLAG_DIGITO_PULSADO 0x20


// DECLARACIÓN ESTRUCTURAS

typedef struct {
	TipoReloj reloj;
	TipoTeclado teclado;
	int tempTime;
	int digitosGuardados;
	int digitoPulsado;
	int lcdId;
} TipoCoreWatch;


// DECLARACIÓN VARIABLES
TipoCoreWatch g_coreWatch;
static int g_flagsCoreWatch;



// DEFINICIÓN VARIABLES

//------------------------------------------------------
// FUNCIONES DE INICIALIZACION DE LAS VARIABLES
//------------------------------------------------------

//------------------------------------------------------
// FUNCIONES PROPIAS
//------------------------------------------------------
void DelayUntil(unsigned int next);
int ConfiguraInicializaSistema (TipoCoreWatch *p_sistema);
void DelayUntil(unsigned int next);
int EsNumero(char value);

//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

int CompruebaDigitoPulsado(fsm_t* p_this);
int CompruebaNewTimeIsReady(fsm_t* p_this);
int CompruebaReset(fsm_t* p_this);
int CompruebaSetCancelNewTime(fsm_t* p_this);
int CompruebaSetupDone(fsm_t* p_this);
int CompruebaTeclaPulsada(fsm_t* p_this);
int CompruebaTimeActualizado(fsm_t* p_this);

//------------------------------------------------------
// FUNCIONES DE SALIDA O DE ACCION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------
void CancelSetNewTime(fsm_t* p_this);
void PrepareSetNewTime(fsm_t* p_this);
void ProcesaDigitoTime(fsm_t* p_this);
void ProcesaTeclaPulsada(fsm_t* p_this);
void Reset(fsm_t* p_this);
void SetNewTime(fsm_t* p_this);
void ShowTime(fsm_t* p_this);
void Start(fsm_t* p_this);

//------------------------------------------------------
// SUBRUTINAS DE ATENCION A LAS INTERRUPCIONES
//------------------------------------------------------


//------------------------------------------------------
// FUNCIONES LIGADAS A THREADS ADICIONALES
//------------------------------------------------------
#if VERSION == 2
PI_THREAD(ThreadExploraTecladoPC);
#endif /* EAGENDA_H */
