idf_component_register(SRCS "esp_rest_main.c"
                            "rest_server.c"
                            "db.c"
                    INCLUDE_DIRS ".")



set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../website")
message(STATUS "WEB_SRC_DIR: ${WEB_SRC_DIR}")
if(EXISTS ${WEB_SRC_DIR})
    spiffs_create_partition_image(www ${WEB_SRC_DIR} FLASH_IN_PROJECT)
else()
    message(FATAL_ERROR "${WEB_SRC_DIR} doesn't exit.")
endif()

