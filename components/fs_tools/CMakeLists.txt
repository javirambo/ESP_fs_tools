idf_component_register(SRCS "iniFile.c" "fsTools.c" "fsLog.c" "fsBuffer.c" 
                    INCLUDE_DIRS "./include"
                    REQUIRES "spiffs fatfs")

