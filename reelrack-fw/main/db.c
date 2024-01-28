#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include "db.h"

#define TAG_DB "db"

esp_err_t initialize_vfs()
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/www",
        .partition_label = NULL,
        .max_files = 10,
        .format_if_mount_failed = false};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG_DB, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG_DB, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG_DB, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG_DB, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG_DB, "Partition size: total: %d, used: %d", total, used);
    }

    return ret;
}

void read_reel_from_vfs(int reel_id, smd_reel_t *reel)
{
    FILE *f = fopen("/www/reels", "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG_DB, "Failed to open file for reading");
        return;
    }

    fseek(f, reel_id * sizeof(smd_reel_t), SEEK_SET);
    fread(reel, sizeof(smd_reel_t), 1, f);
    fclose(f);
}

void read_reels_from_vfs(smd_reel_t *reels, int numReels)
{
    FILE *f = fopen("/www/reels", "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG_DB, "Failed to open file for reading");
        return;
    }

    fread(reels, sizeof(smd_reel_t)*numReels, 10, f);
    fclose(f);
}

void save_reel_to_vfs(int reel_id, smd_reel_t *reel)
{
    FILE *f = fopen("/www/reels", "r+");
    if (f == NULL)
    {
        ESP_LOGE(TAG_DB, "Failed to open file for writing");
        return;
    }

    fseek(f, reel_id * sizeof(smd_reel_t), SEEK_SET);
    fwrite(reel, sizeof(smd_reel_t), 1, f);
    fclose(f);
}

void save_rack_settings_to_vfs(rack_settings_t * rack_settings)
{
    FILE *f = fopen("/www/rack_settings", "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG_DB, "Failed to open file for writing");
        return;
    }

    fwrite(rack_settings, sizeof(rack_settings_t), 1, f);

    //print to terminal
    ESP_LOGI(TAG_DB, "rack_settings->numReelsPerRow: %d", rack_settings->numReelsPerRow);
    ESP_LOGI(TAG_DB, "rack_settings->numRows: %d", rack_settings->numRows);
    ESP_LOGI(TAG_DB, "rack_settings->ledColour: %ld", rack_settings->ledColour);

    fclose(f);
}

void get_rack_settings_from_vfs(rack_settings_t * rack_settings)
{
    FILE *f = fopen("/www/rack_settings", "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG_DB, "Failed to open settings file for reading");
        //create file
        f = fopen("/www/rack_settings", "w");
        if (f == NULL)
        {
            ESP_LOGE(TAG_DB, "Tried to create the file and assign default values. Failed to open file for writing");
            return;
        }
        //assign some default values
        rack_settings->numReelsPerRow = 20;
        rack_settings->numRows = 2;
        rack_settings->ledColour = 0x00FF00;
        //write to file
        if (fwrite(rack_settings, sizeof(rack_settings_t), 1, f) != 1)
        {
            ESP_LOGE(TAG_DB, "Failed to write default settings to file");
        }
        fclose(f);
        return;
    }

    if (fread(rack_settings, sizeof(rack_settings_t), 1, f) != 1)
    {
        ESP_LOGE(TAG_DB, "Failed to read settings from file");
    }

    //print to terminal
    ESP_LOGI(TAG_DB, "rack_settings->numReelsPerRow: %d", rack_settings->numReelsPerRow);
    ESP_LOGI(TAG_DB, "rack_settings->numRows: %d", rack_settings->numRows);
    ESP_LOGI(TAG_DB, "rack_settings->ledColour: %ld", rack_settings->ledColour);

    fclose(f);
}


void delete_reel_from_vfs(int reel_id)
{
    smd_reel_t reel;
    memset(&reel, 0, sizeof(smd_reel_t));
    save_reel_to_vfs(reel_id, &reel);
}