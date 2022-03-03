# ESP_fs_tools
File system tools para ESP

Pensado para usar el file system de flash o de una micro SD

Si no existe una SD card, entonces usaré el FS de flash (SPIFFS)
*
### fs_tools

Todo un set de tools para FS, usando el FS de SD CARD por defecto (sino usará el SPIFFS)

### fsBuffer

Crea archivos de buffers circulares. Los usa fs_log

### fsLog

Para guardar logs en la SD card y luego recuperarlos.

### iniFile

```
Usar asi:
 ini_file_t t;
 ini_file_init(&t, "archivo.ini");
 int i = ini_file_geti(&t, "clave", 300);
 float f = ini_file_getf(&t, "clave", 300.5);
 ini_file_close(&t);
```
