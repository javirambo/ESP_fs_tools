/*
 fsLog.h

 Guarda archivos de logs en el File System.
 Usara la micro-SD si esta disponible.
 Si no existe la micro-SD solo grabara solo ERRORES.

 Permite extraer o enviar los logs mediante callbacks linea a linea,
 para poder llevarlos al puerto serie, wifi, etc.

 Version 2
 JJTeam - 2021/2022
 */

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include "sdkconfig.h"
#include "fsTools.h"

extern const char *FSLOG_STARTUP;
extern const char *FSLOG_DIAGNOS;

void fsLog_init();
void fsLog_modoDiagnostico(bool enable);
void fsLog_forEachLineInStartup(ForEachLineCallback callback);
void fsLog_forEachLineInBuffer(ForEachLineCallback callback);
char* fsLog_getStatus(char *buf);
