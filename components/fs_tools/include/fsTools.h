/*
 * fs_tools.c
 *
 * Tools para trabajar con el FS ya sea flash o SD card.
 *
 * En principio trata de abrir el FS de la SD CARD,
 * y si falla abre el FS del spif (en flash), si falla .... 😱😱😱
 *
 * De todas formas se mantienen separados, y si existen ambos FS funcionando
 * se pueden usar independientemente.
 *
 * Javier 2022
 */
#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

typedef void (*ForEachLineCallback)(char *line);

//--para el spif--
bool spif_init();
bool is_spif_mounted();
void spif_mkdir(const char *name);
FILE* spif_open_file(const char *name, const char *type);
void spif_delete(const char *name);

//--para el sd card--
bool sd_init();
void sd_unmount(); 		// puedo quitar la SD-CARD
bool is_sd_mounted();	// si es false usará el spiffs
void sd_mkdir(const char *name);
FILE* sd_open_file(const char *name, const char *type);
void sd_delete(const char *name);

//--PARA AMBOS y por defecto el SD CARD--
void fs_init();
void fs_mkdir(const char *name);
FILE* fs_open_file(const char *name, const char *type);
void fs_delete(const char *name);
uint32_t fs_file_size(FILE *fp);

void fs_file_dump(char *nombre);
bool fs_file_exists(const char *filename);
void fs_forEachLineFromTextFile(const char *filename, ForEachLineCallback callback);
