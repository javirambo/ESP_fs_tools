/*
 fsLog.c

 Guarda archivos de logs en el File System.
 Usar� la micro-SD si est� disponible.
 Si no existe la micro-SD solo grabar� solo ERRORES.

 Permite extraer o enviar los logs mediante callbacks linea a linea,
 para poder llevarlos al puerto serie, wifi, etc.

 Version 2
 JJTeam - 2021/2022
 */
#include <stdio.h>
#include <esp_log.h>
#include "fsLog.h"
#include "fsTools.h"
#include "fsBuffer.h"

static const char *TAG = "fsLog";

// TAMA�O DE LA LINEA DE LOG
#define TAM_BUF 250
static const char STARTUP_LOG_FILENAME[] = "startup.log";
static const char BASE_PATH[] = "LOGGER";
// calculado: BASE_PATH / STARTUP_LOG_FILENAME
#define STARTUP_LOG_LEN	20

static bool modoDiagnostico = false;
static char startupLogFileName[STARTUP_LOG_LEN];

void fsLog_init()
{
	fsBuffer_init(BASE_PATH);
	// creo un archivo fuera del fsLog, que uso para el startup:
	sprintf(startupLogFileName, "%s/%s", BASE_PATH, STARTUP_LOG_FILENAME);
	// inicializo desde 0 el startup:
	fs_delete(startupLogFileName);
}

void fsLog_modoDiagnostico(bool enable)
{
	modoDiagnostico = enable;
}

// escribe en un archivo separado, se borra en cada init()
void fsLog_startup(const char *format, ...)
{
	va_list argptr;
	char buf[TAM_BUF];
	va_start(argptr, format);
	vsprintf(buf, format, argptr);

	// muestro el log por consola:
	ESP_LOGE(TAG, "%s", buf);

	// grabo el log en el FS:
	FILE *f = fs_open_file(startupLogFileName, "a");
	if (f)
	{
		fputs(buf, f);
		fclose(f);
	}
	else
	{
		ESP_LOGE(TAG, "No pude escribir el log [%s]", startupLogFileName);
	}
	va_end(argptr);
}

/*
 * Graba el LOG a donde est� habilitado
 * (sd-card, spif, y depende del modo diagnostico tambien)
 */
void fsLog(const char *format, ...)
{
	va_list argptr;
	char buf[TAM_BUF];
	va_start(argptr, format);
	vsprintf(buf, format, argptr);
	int len = strlen(buf);

	// imprime siempre por el puerto serie (ESP_LOG),
	// si es ModoDiagnostico, se habilita el Trace o el Verbose.
	if (format[1] == 'D' || format[1] == 'I' || format[1] == 'E' ||
			(format[1] == 'T' && modoDiagnostico) ||
			(format[1] == 'V' && modoDiagnostico))
	{
		ESP_LOGD(TAG, "%s", buf);
	}

	// algunas cosas se graban...por ejemplo:
	// -si es ERROR -> se graba (cualquier FS, tanto SD como SPIF)
	// -si es INFO -> solo se graba en sd-card
	// -si es VERBOSE y est� habilitado el ModoDiagnostico -> se graba solo en sd-card
	if (format[1] == 'E' ||
			(is_sd_mounted() && format[1] == 'I') ||
			(is_sd_mounted() && modoDiagnostico && format[1] == 'V'))
	{
		fsBuffer_write(buf, len);
	}

	va_end(argptr);
}

// buf minimo 100 bytes
char* fsLog_getStatus(char *buf)
{
	sprintf(buf, "SPIFFS mounted:%c, SDFS mounted:%c, modoDiagnostico=%c, startupLogFileName=%s",
			is_spif_mounted() ? 'Y' : 'N',
			is_sd_mounted() ? 'Y' : 'N',
			modoDiagnostico ? 'Y' : 'N',
			startupLogFileName);
	return buf;
}

inline void fsLog_forEachLineInStartup(ForEachLineCallback callback)
{
	fsBuffer_forEachLineFromFile(startupLogFileName, callback);
}

inline void fsLog_forEachLineInBuffer(ForEachLineCallback callback)
{
	fsBuffer_forEachLine(callback);
}
