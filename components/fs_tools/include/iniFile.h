/*
 ini_file

 Lectura de configuraciones desde un archivo .INI en el FS.

 Usar asi:

    ini_file_t ini;
    ini_file_open(&ini, "config.ini");
    int i   = ini_file_geti(&ini, "clave", 300);
    float f = ini_file_getf(&ini, "clave", 300.5);
    ini_file_close(&ini);
    
 Javier 2022
 */
#pragma once
#include <stdio.h>

typedef struct
{
	char *name;
	FILE *fp;
	bool create_if_not_exists; // para crear .ini que todavia no existen
} ini_file_t;

void ini_file_open(ini_file_t *t, char *archivo_ini);
void ini_file_close(ini_file_t *t);

int ini_file_geti(ini_file_t *t, char *key, int def);
float ini_file_getf(ini_file_t *t, char *key, float def);
char* ini_file_gets(ini_file_t *t, char *value_buf, char *key, char *def);

void ini_file_sets(ini_file_t *t, char *key, char *value);
void ini_file_seti(ini_file_t *t, char *key, int value);
void ini_file_setf(ini_file_t *t, char *key, float value);
