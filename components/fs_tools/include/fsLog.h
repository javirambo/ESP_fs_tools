/*
 fsLog.h

 Guarda archivos de logs en el File System.
 Usar· la micro-SD si est· disponible.
 Si no existe la micro-SD solo grabar· solo ERRORES.

 Permite extraer o enviar los logs mediante callbacks linea a linea,
 para poder llevarlos al puerto serie, wifi, etc.

 Version 2
 JJTeam - 2021/2022
 */

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include "sdkconfig.h"
#include "fsBuffer.h"

#ifdef CONFIG_USE_FS_LOG

#define FSLOG_FORMAT(letter, format) "[" #letter "]: " format "\n"
#define FSLOG_FORMAT2(format) format "\n"
//// Todas estas formas de llamar al Logger:
#define LogDebug(format, ...) FSLOG.log(FSLOG_FORMAT(D, format), ##__VA_ARGS__)
// muestra solo si est√° en modo diagnostico
#define LogDebugDiag(format, ...) FSLOG.log(FSLOG_FORMAT(T, format), ##__VA_ARGS__)
#define LogTrace(format, ...) FSLOG.log(FSLOG_FORMAT(T, format), ##__VA_ARGS__)
//// LogDebugDiag() y LogTrace() son iguales
#define LogInfo(format, ...) FSLOG.log(FSLOG_FORMAT(I, format), ##__VA_ARGS__)
// graba solo si est√° en modo diagnostico
#define LogInfoDiag(format, ...) FSLOG.log(FSLOG_FORMAT(V, format), ##__VA_ARGS__)
#define LogInfoDetail(format, ...) FSLOG.log(FSLOG_FORMAT(V, format), ##__VA_ARGS__)
//// LogInfoDiag() y LogInfoDetail() son iguales
#define LogError(format, ...) FSLOG.log(FSLOG_FORMAT(E, format), ##__VA_ARGS__)
#define LogAtStartUp(format, ...) FSLOG.startup(FSLOG_FORMAT2(format), ##__VA_ARGS__)
// setea el modo diagnostico, o sea que se veran los llamados a LogDebugDiag
#define LogModoDiagnostico(b) FSLOG.SetModoDiagnostico(b)
#define LogBegin(a, b, c) FSLOG.begin(a, b, c)

#else // USE_FS_LOG

static const char *TAG_FSLOG = "fsLog";
#define LogDebug(format, ...) ESP_LOGD(TAG_FSLOG, FSLOG_FORMAT2(format), ##__VA_ARGS__)
#define LogDebugDiag(format, ...) ESP_LOGD(TAG_FSLOG,FSLOG_FORMAT2(format), ##__VA_ARGS__)
#define LogTrace(format, ...) ESP_LOGD(TAG_FSLOG,FSLOG_FORMAT2(format), ##__VA_ARGS__)
#define LogInfo(format, ...) ESP_LOGI(TAG_FSLOG,FSLOG_FORMAT2(format), ##__VA_ARGS__)
#define LogInfoDiag(format, ...) ESP_LOGI(TAG_FSLOG,FSLOG_FORMAT2(format), ##__VA_ARGS__)
#define LogInfoDetail(format, ...) ESP_LOGI(TAG_FSLOG,FSLOG_FORMAT2(format), ##__VA_ARGS__)
#define LogError(format, ...) ESP_LOGE(TAG_FSLOG,FSLOG_FORMAT2(format), ##__VA_ARGS__)
#define LogAtStartUp(format, ...) ESP_LOGI(TAG_FSLOG,FSLOG_FORMAT2(format), ##__VA_ARGS__)
#define LogModoDiagnostico(b)
#define LogBegin(a, b, c)

#endif // USE_FS_LOG

void fsLog_init();
void fsLog_modoDiagnostico(bool enable);
void fsLog_startup(const char *format, ...); // escribe en un archivo separado, se borra en cada init()
void fsLog_forEachLineInStartup(ForEachLineCallback callback);
void fsLog_forEachLineInBuffer(ForEachLineCallback callback);
void fsLog(const char *format, ...);
char* fsLog_getStatus();
