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
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>
#include <esp_vfs.h>
#include <esp_spiffs.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <errno.h>
#include "fsTools.h"
#include "fsBuffer.h"

#if CONFIG_IDF_TARGET_ESP32S2
#define SPI_DMA_CHAN    sd_card_host.slot
#elif CONFIG_IDF_TARGET_ESP32C3
#define SPI_DMA_CHAN    SPI_DMA_CH_AUTO
#else
#define SPI_DMA_CHAN    1
#endif

static const char *TAG = "fsTools";

//--SD-CARD FS--
static const char sdCardMountPoint[] = "/sdcard";
static bool sdfs_mounted = false;
static sdmmc_host_t sd_card_host;
static sdmmc_card_t *card;
//--SPIFFS--
static const char spiffsMountPoint[] = "/spiffs";
static bool spiffs_mounted = false;
static bool fs_initialized = false; // por si otro módulo tambien lo intenta inicializar.

/*
 * Inicializa los FS, y deja el SD-CARD disponible por defecto.
 */
void fs_init()
{
	if (fs_initialized)
	{
		ESP_LOGD(TAG, "FS ya inicializado!");
		return;
	}
	if (!sd_init())
	{
		ESP_LOGW(TAG, "No existe la micro-SD!");
		if (!spif_init())
			ESP_LOGE(TAG, "FS ERROR 😱");
		else
			ESP_LOGW(TAG, "Se usara la flash para el FS!");
	}
	else
		ESP_LOGI(TAG, "Se encontró una micro-SD y se usara por defecto");
	fs_initialized = true;
}

/**
 * Inicializa el FS en flash
 */
bool spif_init()
{
	if (spiffs_mounted)
		return true;

	ESP_LOGI(TAG, "Inicializando SPIFFS, root=[%s], max_files=%d", spiffsMountPoint, CONFIG_FILE_SYSTEM_OPEN_FILES);

	esp_vfs_spiffs_conf_t conf = {
		.base_path = spiffsMountPoint,
		.partition_label = NULL,
		.max_files = 5,
		.format_if_mount_failed = false
	};

#ifdef CONFIG_FS_LOG_CANT_FILES
	conf.max_files = CONFIG_FS_LOG_CANT_FILES;
#endif
#ifdef CONFIG_FS_FORMAT_IF_MOUNT_FAILED
	conf.format_if_mount_failed = CONFIG_FS_FORMAT_IF_MOUNT_FAILED;
#endif

	// Use settings defined above toinitialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is anall-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
		{
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		}
		else if (ret == ESP_ERR_NOT_FOUND)
		{
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		}
		else
		{
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
		esp_system_abort("Fallo en Spiffs. Abort!"); // CHAU! ERROR!
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(NULL, &total, &used);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
		return spiffs_mounted = false;
	}
	else
	{
		ESP_LOGI(TAG, "SPIF mounted OK in [%s]. Partition size: total: %d, used: %d",
				spiffsMountPoint, total, used);
		return spiffs_mounted = true;
	}
}

bool sd_init()
{
	if (sdfs_mounted)
		return true;

	esp_err_t ret;

	// Options for mounting the filesystem.
	// If format_if_mount_failed is set to true, SD card will be partitioned and
	// formatted in case when mounting fails.

	esp_vfs_fat_sdmmc_mount_config_t mount_config = {
		.format_if_mount_failed = false,
		.max_files = 5,
		.allocation_unit_size = 16 * 1024
	};

#ifdef CONFIG_FS_LOG_CANT_FILES
	mount_config.max_files = CONFIG_FS_LOG_CANT_FILES;
#endif
#ifdef CONFIG_FS_FORMAT_IF_MOUNT_FAILED
	mount_config.format_if_mount_failed = CONFIG_FS_FORMAT_IF_MOUNT_FAILED;
#endif

	ESP_LOGI(TAG, "Inicializando SD card, root=[%s], max_files=%d", sdCardMountPoint, CONFIG_FILE_SYSTEM_OPEN_FILES);

	// Use settings defined above to initialize SD card and mount FAT filesystem.
	// Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
	// Please check its source code and implement error recovery when developing
	// production applications.

	ESP_LOGI(TAG, "Using SPI peripheral");

	sdmmc_host_t sd_card_host = SDSPI_HOST_DEFAULT();
	spi_bus_config_t bus_cfg = {
		.mosi_io_num = CONFIG_SD_MOSI_GPIO,
		.miso_io_num = CONFIG_SD_MISO_GPIO,
		.sclk_io_num = CONFIG_SD_SCLK_GPIO,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 4000,
	};
	ret = spi_bus_initialize(sd_card_host.slot, &bus_cfg, SPI_DMA_CHAN);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to initialize bus.");
		return false;
	}

	// This initializes the slot without card detect (CD) and write protect (WP) signals.
	// Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.

	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
	slot_config.gpio_cs = CONFIG_SD_CS_GPIO;
	slot_config.host_id = sd_card_host.slot;

	ESP_LOGI(TAG, "Mounting filesystem");
	ret = esp_vfs_fat_sdspi_mount(sdCardMountPoint, &sd_card_host, &slot_config, &mount_config, &card);

	if (ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
		{
			// If you want the card to be formatted, set the FS_FORMAT_IF_MOUNT_FAILED menuconfig option.
			ESP_LOGE(TAG, "Failed to mount filesystem");
		}
		else
		{
			// Make sure SD card lines have pull-up resistors in place.
			ESP_LOGE(TAG, "Failed to initialize the card (%s)", esp_err_to_name(ret));
		}
		return false;
	}
	ESP_LOGI(TAG, "sd-card Filesystem mounted OK in [%s]", sdCardMountPoint);

	// Card has been initialized, print its properties
	sdmmc_card_print_info(stdout, card);

	// Use POSIX and C standard library functions to work with files.
	//......
	return sdfs_mounted = true;
}

void sd_unmount()
{
	// All done, unmount partition and disable SPI peripheral
	esp_vfs_fat_sdcard_unmount(sdCardMountPoint, card);
	ESP_LOGI(TAG, "Card unmounted");

	//deinitialize the bus after all devices are removed
	spi_bus_free(sd_card_host.slot);
	sdfs_mounted = false;
}

bool is_sd_mounted()
{
	return sdfs_mounted;
}

bool is_spif_mounted()
{
	return spiffs_mounted;
}

static FILE* open_file(const char *root, const char *name, const char *type)
{
	char buf[CONFIG_VFS_SEMIHOSTFS_HOST_PATH_MAX_LEN];
	sprintf(buf, "%s/%s", root, name);
	FILE *f = fopen(buf, type);
	// lo pongo en verbose porque sino el FsLog con los archivos circulares siempore muestrasn error
	if (!f)
		ESP_LOGV(TAG, "NO PUDE ABRIR EL ARCHIVO [%s]", buf);
	else
		ESP_LOGV(TAG, "Archivo abierto OK [%s]", buf);
	return f;
}

FILE* sd_open_file(const char *name, const char *type)
{
	return open_file(sdCardMountPoint, name, type);
}

FILE* spif_open_file(const char *name, const char *type)
{
	return open_file(spiffsMountPoint, name, type);
}

FILE* fs_open_file(const char *name, const char *type)
{
	// tiene prioridad el SD-CARD
	if (sdfs_mounted)
		return open_file(sdCardMountPoint, name, type);
	else if (spiffs_mounted)
		return open_file(spiffsMountPoint, name, type);
	else
	{
		esp_system_abort("FALTA INICIALIZAR EL FS_TOOLS. Abort!");
		return NULL; // 😱
	}
}

void sd_mkdir(const char *name)
{
	char buf[CONFIG_VFS_SEMIHOSTFS_HOST_PATH_MAX_LEN];
	sprintf(buf, "%s/%s", sdCardMountPoint, name);
	mkdir(buf, 0775);
}

void spif_mkdir(const char *name)
{
	char buf[CONFIG_VFS_SEMIHOSTFS_HOST_PATH_MAX_LEN];
	sprintf(buf, "%s/%s", spiffsMountPoint, name);
	mkdir(buf, 0775);
	// mkdir no hace falta para SPIFFS (no se como los crea) pero anda!
	// con SPIF da error, pero con SD-CARD deberia crearlos.
}

void fs_mkdir(const char *name)
{
	// tiene prioridad el SD-CARD
	if (sdfs_mounted)
		sd_mkdir(name);
	else if (spiffs_mounted)
		spif_mkdir(name);
}

void sd_delete(const char *name)
{
	char buf[CONFIG_VFS_SEMIHOSTFS_HOST_PATH_MAX_LEN];
	sprintf(buf, "%s/%s", sdCardMountPoint, name);
	unlink(buf);
}

void spif_delete(const char *name)
{
	char buf[CONFIG_VFS_SEMIHOSTFS_HOST_PATH_MAX_LEN];
	sprintf(buf, "%s/%s", spiffsMountPoint, name);
	unlink(buf);
}

void fs_delete(const char *name)
{
	// tiene prioridad el SD-CARD
	if (sdfs_mounted)
		sd_delete(name);
	else if (spiffs_mounted)
		spif_delete(name);
}

/*
 * Obtengo el tamaño del archivo pero no muevo el cursor original.
 */
uint32_t fs_file_size(FILE *fp)
{
	int temp = fp->_offset;
	fseek(fp, 0, SEEK_END);
	int tam = fp->_offset;
	fseek(fp, temp, SEEK_SET);
	return tam;
}

/*
 * A modo de test. Hace un dump de un archivo y lo muestra por stdout (serie?)
 */
void fs_file_dump(char *nombre)
{
	ESP_LOGI(TAG, "File dump: %s", nombre);
	FILE *fp = fs_open_file(nombre, "r");
	char buf[101];
	size_t bytes_read;
	if (fp)
	{
		fprintf(stdout, "begin>>>");
		do
		{
			bytes_read = fread(buf, 1, 100, fp);
			buf[bytes_read] = 0; // null terminator

			//TODO ojo con los chars <32 o los archivos binarios!
			fprintf(stdout, "%s", buf);

		} while (bytes_read == 100);
		fprintf(stdout, "<<<end\n");
	}
}

bool fs_file_exists(const char *filename)
{
	struct stat st;
	char buf[CONFIG_VFS_SEMIHOSTFS_HOST_PATH_MAX_LEN];
	if (sdfs_mounted)
		sprintf(buf, "%s/%s", sdCardMountPoint, filename);
	else if (spiffs_mounted)
		sprintf(buf, "%s/%s", spiffsMountPoint, filename);
	return !stat(buf, &st);
}

/*
 * abre un archivo, y por cada linea llama el callback.
 * (OJO que tiene que ser archivo de texto y no binario)
 */
void fs_forEachLineFromTextFile(const char *filename, ForEachLineCallback callback)
{
	// si no existe el archivo, esta bien, pero no lo abro porque sino loguea un error...
	if (!fs_file_exists(filename))
		return;
	FILE *f = fs_open_file(filename, "r");
	if (f)
	{
		char buf[500], *line;
		while ((line = fgets(buf, sizeof(buf), f)) != 0)
			callback(line);
		fclose(f);
	}
}
