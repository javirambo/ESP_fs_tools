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
	fsLog_startup("Inicializando el sistema");
	fsLog_startup("chau");
	ESP_LOGD(TAG, "fSLog status: %s", fsLog_getStatus());

}
