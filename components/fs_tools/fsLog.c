/*
 fsLog.c

 Guarda archivos de logs en el File System.
 Usará la micro-SD si está disponible.
 Si no existe la micro-SD solo grabará solo ERRORES.

 Permite extraer o enviar los logs mediante callbacks linea a linea,
 para poder llevarlos al puerto serie, wifi, etc.

 Version 2
 JJTeam - 2021/2022
 */
#include <stdio.h>
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>
#include "fsLog.h"
#include "fsTools.h"
#include "fsBuffer.h"

static const char *TAG = "fsLog";
#ifndef CONFIG_USE_FS_LOG
static const char *NOT_CONFIGURED = "NOT CONFIGURED";
#endif
const char *FSLOG_STARTUP = "@startup";
const char *FSLOG_DIAGNOS = "@diagnos";
static const char *STARTUP_LOG_FILENAME = "startup.log";
static const char *BASE_PATH = "LOGGER";
// calculado: BASE_PATH / STARTUP_LOG_FILENAME
#define STARTUP_LOG_LEN	20
// TAMAÑO DE LA LINEA DE LOG
#define TAM_BUF 300

static bool modoDiagnostico = false;
static char startupLogFileName[STARTUP_LOG_LEN];
static int startupLogFileName_errorCounts = 0;
static vprintf_like_t original_esp_log;

/*
 * grabo el texto en el FS
 */
static void startup_store(char *buf)
{
	FILE *f = fs_open_file(startupLogFileName, "a");
	if (f)
	{
		// ejemplo de log: "I (00:00:20.061) @startup: Inicializando el sistema"
		// le saco el @startup: porque se hace una grabacion circular!!
		char *p = strstr(buf, "@startup: ");
		if (p)
			*p = 0;
		fprintf(f, "%s%s", buf, p + 10);
		fclose(f);
	}
	else if (startupLogFileName_errorCounts < 10)
	{
		// no quiero arruinar el log con porquerias...si no se puede escribir, jodo un ratito nomas...
		startupLogFileName_errorCounts++;
		ESP_LOGE(TAG, "No pude escribir el log [%s]", startupLogFileName);
	}
}

static char getLevel(char *line)
{
	//"\033[0;31mI" << la 'I' es el level
	if (*line == '\033')
		return *(strchr(line, 'm') + 1);
	else
		return *line;
}

// If successful, the total number of characters written is returned otherwise a negative number is returned.
// ejemplo de log: "\033[0;31mI (00:00:20.061) @startup: Inicializando el sistema\033[0m"
int vprintf_like_log_func(const char *format, va_list args)
{
#ifdef CONFIG_USE_FS_LOG
	char buf[TAM_BUF];
	int len = vsprintf(buf, format, args);

	// es una linea de startup?
	if (strstr(buf, "@startup:"))
		startup_store(buf);

	// imprime SIEMPRE en la consola de debug (ESP_LOGx standard)
	(*original_esp_log)(format, args);

	char level = getLevel(buf);
	bool es_diagnos = (strstr(buf, FSLOG_DIAGNOS) != NULL);

	// se graban los mensajes de ERROR -> SIEMPRE
	if (level == 'E')
		fsBuffer_write(buf, len);

	// se graban los mensajes de WARNING -> SIEMPRE que exista la sd-card
	else if (level == 'W' && is_sd_mounted())
		fsBuffer_write(buf, len);

	// se graban los mensajes de INFO solo si modoDiagnostico=true y si existe la sd-card
	// y para los mensajes con el TAG=@diagnos
	else if (level == 'I' && is_sd_mounted() && modoDiagnostico && es_diagnos)
		fsBuffer_write(buf, len);

	return len;
#else
	return (*original_esp_log)(format, args);
#endif
}

void fsLog_init()
{
#ifdef CONFIG_USE_FS_LOG
	fsBuffer_init(BASE_PATH);
	// creo un archivo fuera del fsLog, que uso para el startup:
	sprintf(startupLogFileName, "%s/%s", BASE_PATH, STARTUP_LOG_FILENAME);
	// inicializo desde 0 el startup:
	fs_delete(startupLogFileName);
#endif

	original_esp_log = esp_log_set_vprintf(&vprintf_like_log_func);

	// no muestro nada VERBOSE, salvo lo mio!
	esp_log_level_set("*", ESP_LOG_DEBUG);
}

/*
 * ModoDiagnostico:
 * -se grabarán al FS_LOG los mensajes de ERROR 	 	-> SIEMPRE
 * -se grabarán al FS_LOG los mensajes de WARNING 		-> siempre que si exista la sd-card
 * -se grabaran al FS_LOG los mensajes de INFO 			-> con enable=true y si existe la sd-card y para los mensajes con el TAG=@diagnos
 * -se mostrarán en consola los mensajes de DEBUG 		-> SIEMPRE
 * -se mostrarán en consola los mensajes de VERBOSE 	-> con enable=true y para los mensajes con el TAG=@diagnos
 */
void fsLog_modoDiagnostico(bool enable)
{
	modoDiagnostico = enable;
	// en modo diagnostico puedo usar modo VERBOSE (pero ojo que si no esta compilado asi, no se veran)
	esp_log_level_set(FSLOG_DIAGNOS, enable ? ESP_LOG_VERBOSE : ESP_LOG_DEBUG);
}

// buf minimo 100 bytes
char* fsLog_getStatus(char *buf)
{
#ifdef CONFIG_USE_FS_LOG
	sprintf(buf, "FLASH:%c, SD-CARD:%c, modoDiag=%c, path=%s",
			is_spif_mounted() ? 'Y' : 'N',
			is_sd_mounted() ? 'Y' : 'N',
			modoDiagnostico ? 'Y' : 'N',
			startupLogFileName);
	return buf;
#else
	return NOT_CONFIGURED;
#endif
}

void fsLog_forEachLineInStartup(ForEachLineCallback callback)
{
#ifdef CONFIG_USE_FS_LOG
	fs_forEachLineFromTextFile(startupLogFileName, callback);
#else
	callback(NOT_CONFIGURED);
#endif
}

void fsLog_forEachLineInBuffer(ForEachLineCallback callback)
{
#ifdef CONFIG_USE_FS_LOG
	fsBuffer_forEachLine(callback);
#else
	callback(NOT_CONFIGURED);
#endif
}
