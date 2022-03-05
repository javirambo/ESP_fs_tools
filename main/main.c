/**
 * Proyecto PIG GUARD
 * FASE 1 - Deteccion de valores con los sensores y muestra en display TFT.
 * JJTeam
 * Junio 2021, 12/oct/2021
 *
 * 13/10/2021:
 * - Esta version es la definitiva que se estaria probando en campo solo para FASE 1.
 * - Se quita todo lo que es FASE 2.
 *
 * FEBRERO-2022
 * - Se migra a código para IDF y se coloca un branch en github /IDF
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>
#include <esp_task_wdt.h>
#include <esp_log.h>
#include <esp_event.h>
#include <nvs_flash.h>

#include "fsTools.h"
#include "fsLog.h"
#include "iniFile.h"

static const char *TAG = "main";

void callback1(char *line)
{
	fprintf(stdout, "%s", line);
	fflush(stdout);
}

void app_main(void)
{
	ESP_ERROR_CHECK(nvs_flash_init());

	fs_init();

	//-----------------------------
	ini_file_t ift;
	ini_file_open(&ift, "config.ini");
	ESP_LOGD(TAG, "int=%d", ini_file_geti(&ift, "volts", 0));
	ESP_LOGD(TAG, "float=%f", ini_file_getf(&ift, "amper", 0));
	ini_file_close(&ift);

	ini_file_open(&ift, "config.ini");
	ini_file_seti(&ift, "volts", 380);
	ini_file_setf(&ift, "amper", 0.5);
	ini_file_close(&ift);
	ESP_LOGD(TAG, "seteados int & float");

	ini_file_open(&ift, "config.ini");
	ESP_LOGD(TAG, "int=%d", ini_file_geti(&ift, "volts", 0));
	ESP_LOGD(TAG, "float=%f", ini_file_getf(&ift, "amper", 0));
	ini_file_close(&ift);

	//-----------------------------
	fsLog_init();
	ESP_LOGI(FSLOG_STARTUP,"Inicializando el sistema");
	ESP_LOGD(FSLOG_STARTUP,"chau");
	char bu[222];
	ESP_LOGD(TAG, "fSLog status: %s", fsLog_getStatus(bu));

	//-----------------------------
	// las carpetas en SPIF no hace falta crearlas....anda igual!
	FILE *f = fs_open_file("laconchatuma/config.ini", "w");
	fputs("HOLA", f);
	fclose(f);

	fs_file_dump("laconchatuma/config.ini");

	//-----------------------------

	ESP_LOGD(TAG, "ESTE ES EL LOG STARTUP>>>>");
	fsLog_forEachLineInStartup(callback1);
	ESP_LOGD(TAG, "<<<<FIN");

	//-------------------------------------------------------------------------------------------------
	ESP_LOGV(FSLOG_DIAGNOS, "verbose %d", esp_random()); // nunca se graba
	ESP_LOGD(FSLOG_DIAGNOS, "debug %d", esp_random()); // nunca se graba
	ESP_LOGI(FSLOG_DIAGNOS, "info %d", esp_random());  // solo en modo diagnostico y en sd-card y solo este TAG
	ESP_LOGW(FSLOG_DIAGNOS, "warn %d", esp_random());  // solo en SD-card  (y cualquier TAG)
	ESP_LOGE(FSLOG_DIAGNOS, "error %d", esp_random()); // siempre  (y cualquier TAG)
	//-------------------------------------------------------------------------------------------------
	ESP_LOGV(TAG, "TAG verbose %d", esp_random()); 	   // nunca se graba
	ESP_LOGD(TAG, "debug %d", esp_random()); // nunca se graba
	ESP_LOGI(TAG, "info %d", esp_random());  // nunca se graba
	ESP_LOGW(TAG, "warn %d", esp_random());  // solo en SD-card  (y cualquier TAG)
	ESP_LOGE(TAG, "error %d", esp_random()); // siempre (y cualquier TAG)
	//-------------------------------------------------------------------------------------------------

	ESP_LOGD(TAG, "ESTE ES EL LOG CIRCULAR>>>>");
	fsLog_forEachLineInBuffer(callback1);
	ESP_LOGD(TAG, "<<<<FIN");

}
