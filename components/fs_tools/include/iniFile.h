/*
 ini_file

 Lectura de configuraciones desde un archivo .INI en el FS.

 Usar asi:

 ini_file_t t;
 ini_file_init(&t, "archivo.ini");
 int i = ini_file_geti(&t, "clave", 300);
 float f = ini_file_getf(&t, "clave", 300.5);

 Javier 2022
 */
#pragma once
#include <stdio.h>

typedef struct
{
	char *name;
	FILE *fp;
} ini_file_t;

void ini_file_open(ini_file_t *t, char *archivo_ini);
void ini_file_close(ini_file_t *t);

int ini_file_geti(ini_file_t *t, char *key, int def);
float ini_file_getf(ini_file_t *t, char *key, float def);
char* ini_file_gets(ini_file_t *t, char *value_buf, char *key, char *def);

void ini_file_sets(ini_file_t *t, char *key, char *value);
void ini_file_seti(ini_file_t *t, char *key, int value);
void ini_file_setf(ini_file_t *t, char *key, float value);
