/*
 FsBuffer.c
 Guarda archivos en el File System a modo de buffer.
 Usa varios archivos para guardar en forma circular.
 Si existe una micro-SD la usa, sino abre en el FS de la flash del ESP32.

 Version 2
 JJTeam - 2021/2022
 */

#ifndef _Fs_Buffer_h
#define _Fs_Buffer_h

#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "fsTools.h"

void fsBuffer_init(const char *buffer_name);
void fsBuffer_clear();
size_t fsBuffer_write(const char *txt, size_t len);
void fsBuffer_forEachLine(ForEachLineCallback callback);

#endif // Fs_Buffer_h
