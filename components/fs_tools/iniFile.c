/*
 ini_file.c

 Lectura de configuraciones desde un archivo .INI en el FS.

 Ver el H para como usar...

 Javier 2022
 */

#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include "iniFile.h"
#include "fsTools.h"

static char *TAG = "iniFile";

static char* ini_file_get_value(ini_file_t *t, char *buf, size_t buf_len, char *key)
{
	if (!t->fp)
		return NULL;

	int len;
	char *linea, *p;

	// busco pero desde el principio:
	fseek(t->fp, 0, SEEK_SET);

	while ((linea = fgets(buf, buf_len, t->fp)) != NULL)
	{
		len = strlen(linea);

		// quito el \n final! (sin o tiene \n es porque la linea es demasiado larga! Danger!)
		p = linea + len - 1;
		while (p != linea && (*p == '\n' || *p == '\r'))
		{
			*p = 0;
			--len;
			--p;
		}

		// salteo lineas en blanco (o menor a x=y)
		if (len < 3)
		{
			ESP_LOGV(TAG, "Salteo linea en blanco.");
			continue;
		}

		// salteo comentarios (;#)
		// o nombres [...] (esto no los uso en esta version de ini)
		if (*linea == ';' || *linea == '[' || *linea == '#')
		{
			ESP_LOGV(TAG, "Salteo comentario '%c'", *linea);
			continue;
		}

		p = strchr(linea, '=');

		// linea erronea o sin valor
		if (!p || !*(p + 1))
		{
			ESP_LOGV(TAG, "Linea erronea o sin valor '%s'", linea);
			continue;
		}

		*p = 0; // delimito el key para poder comparar
		if (!strcmp(linea, key))
		{
			p++; // apunto al value
			char *x = strchr(p, '\n');
			if (x)
				*x = 0; // saco el \n
			return p;
		}
	}
	ESP_LOGW(TAG, "No existe el key [%s]", key);
	return NULL;
}

void ini_file_open(ini_file_t *t, char *archivo_ini)
{
	fs_init(); // abre FS de la SD card o, si no existe, del spif.
	t->name = strdup(archivo_ini);
	t->fp = fs_open_file(archivo_ini, "r");
	if (t->fp == NULL && t->create_if_not_exists)
	{
		t->fp = fs_open_file(archivo_ini, "w");
		fprintf(t->fp, "# archivo %s", archivo_ini);
		fclose(t->fp);
		t->fp = fs_open_file(archivo_ini, "r");
	}
	if (t->fp == NULL)
		ESP_LOGE(TAG, "Error al abrir y/o crear archivo %s", archivo_ini);
}

void ini_file_close(ini_file_t *t)
{
	if (t->fp)
		fclose(t->fp);
#if 0
	fs_file_dump(t->name);
#endif

	if (t->name)
		free(t->name);
	t->fp = 0;
	t->name = 0;
}

int ini_file_geti(ini_file_t *t, char *key, int def)
{
	char buf[300];
	char *value = ini_file_get_value(t, buf, sizeof(buf), key);
	return (value == NULL) ? def : atoi(value);
}

float ini_file_getf(ini_file_t *t, char *key, float def)
{
	char buf[300];
	char *value = ini_file_get_value(t, buf, sizeof(buf), key);
	return (value == NULL) ? def : strtof(value, NULL);
}

char* ini_file_gets(ini_file_t *t, char *value_buf, char *key, char *def)
{
	char buf[300];
	char *value = ini_file_get_value(t, buf, sizeof(buf), key);
	return (value == NULL) ? def : value;
}

// graba otra cosa luego de la "key="
// para eso leo todas las lineas a memoria
// cierro el archivo y abro como escritura
// reescribo todas las lineas reemplazando ese valor
void ini_file_sets(ini_file_t *t, char *key, char *value)
{
	char *buf, linea[200], *plin;
	int keylen = strlen(key);
	int tam = fs_file_size(t->fp); // tamaño del archivo

	// creo un buffer con tamaño original, mas la nueva clave/valor: "key=value\n" (por si no existe)
	buf = malloc(tam + keylen + strlen(value) + 2);

	// tengo que buscar desde el principio:
	fseek(t->fp, 0, SEEK_SET);

	// busco la key:
	// meto todo el archivo en 'buf' pero reemplazo el 'value' de la linea que tiene el 'key'
	*buf = 0;
	bool encontrada = false;
	while ((plin = fgets(linea, sizeof(linea), t->fp)) != NULL)
	{
		// busco la "key=" pegada al margen izquierdo (sin espacios!)
		if (strlen(plin) > keylen &&
				memcmp(linea, key, keylen) == 0 &&
				linea[keylen] == '=')
		{
			// reemplazo lo que está despues del '='
			plin[keylen + 1] = 0;
			strcat(plin, value);
			strcat(plin, "\n");
			encontrada = true;
		}
		strcat(buf, plin);
	}
	// no encontré la key, agrego una linea al final:
	if (!encontrada)
	{
		strcat(buf, "\n");
		strcat(buf, key);
		strcat(buf, "=");
		strcat(buf, value);
		strcat(buf, "\n");
	}

	// reemplazo el archivo con el buffer: cierro/grabo como w/cierro/abro como r
	fclose(t->fp);

	t->fp = fs_open_file(t->name, "w");
	fputs(buf, t->fp);
	fflush(t->fp);
	fclose(t->fp);

	t->fp = fs_open_file(t->name, "r");

	// queda abierto como lectura para seguir el camino normal,
	// poder seguir leyendo keys,
	// y por ultimo cerrarlo normalmente con ini_file_close.
}

void ini_file_seti(ini_file_t *t, char *key, int value)
{
	char buf[15];
	ini_file_sets(t, key, itoa(value, buf, 10));
}

void ini_file_setf(ini_file_t *t, char *key, float value)
{
	char buf[15];
	sprintf(buf, "%f", value);
	ini_file_sets(t, key, buf);
}
