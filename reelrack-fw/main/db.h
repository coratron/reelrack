#ifndef DB_H
#define DB_H

#include "esp_vfs.h"
#include "esp_spiffs.h"

#define MAX_REELS 200

typedef struct
{
    char value[32];
    char package[32];
    char part_number[32];
    char comp_type[32];
    char sku[32];
    char manufacturer[32];
    char quantity[32];
} smd_reel_t;

typedef struct
{
    int numReelsPerRow;
    int numRows;
    long ledColour;
    long ledTimeout;
    long ledBrightness;

    // ssid and password for wifi
    char ssid[32];
    char password[32];
} rack_settings_t;

esp_err_t initialize_vfs();
esp_err_t read_reel_from_vfs(int reel_id, smd_reel_t *reel);
esp_err_t read_reels_from_vfs(smd_reel_t *reels, int numReels);
void save_reel_to_vfs(int reel_id, smd_reel_t *reel);
void delete_reel_from_vfs(int reel_id);
void update_reel_in_vfs(int reel_id, smd_reel_t *reel);
void get_rack_settings_from_vfs(rack_settings_t *settings);
void save_rack_settings_to_vfs(rack_settings_t *settings);

#endif // DB_H
