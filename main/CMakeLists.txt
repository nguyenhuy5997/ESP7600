# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES)
set(COMPONENT_PRIV_REQUIRES)
idf_component_register(SRCS "main.c"
							"../main/simcom7600/simcom7600.c"
							"../main/string_user/location_parser.c"
							"../main/common.h"
							"../main/json_user/json_user.c"
							"../main/wifi_cell/wifi_cell.c"
							"../main/OTA_LTE/FOTA_LTE.c"
							INCLUDE_DIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS "../7600lib")