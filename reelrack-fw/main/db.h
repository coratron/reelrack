#ifndef DB_H
#define DB_H

#include "esp_spiffs.h"
#include "esp_vfs.h"

#define MAX_REELS 200

typedef struct {
  bool valid;
  char value[256];
  char package[32];
  char part_number[32];
  char comp_type[32];
  char sku[32];
  char manufacturer[32];
  int quantity;
} smd_reel_t;

typedef struct {
  int numReelsPerRow;
  int numRows;
  long ledColour;
  long ledTimeout;
  long ledBrightness;
  bool ledDirection;

  // ssid and password for wifi
  char ssid[32];
  char password[32];
} rack_settings_t;

esp_err_t initialize_vfs();
esp_err_t read_reel_from_vfs(int reel_id, smd_reel_t *reel);
int read_reels_from_vfs(smd_reel_t *reels, int numReels);
void save_reel_to_vfs(int reel_id, smd_reel_t *reel);
esp_err_t delete_reel_from_vfs(int reel_id, rack_settings_t *settings);
void update_reel_in_vfs(int reel_id, smd_reel_t *reel);
void get_rack_settings_from_vfs(rack_settings_t *settings);
void save_rack_settings_to_vfs(rack_settings_t *settings);

#endif // DB_H
