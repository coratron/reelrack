#include "nvs_flash.h"
#include "nvs.h"
#include "esp_http_server.h"

typedef struct
{
    char value[32];
    char package[32];
    char comp_type[32];
    char manufacturer[32];
    char part_number[32];
    char sku[32];
    char quantity[32];
} smd_reel_t;

void initialize_nvs()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

void save_reel_to_nvs(int reel_id, smd_reel_t *reel)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error opening NVS handle!\n");
    }
    else
    {
        char reel_str[512];
        sprintf(reel_str, "%s,%s,%s,%s,%s,%s,%s", reel->value, reel->package, reel->quantity, reel->manufacturer, reel->part_number, reel->sku, reel->comp_type);
        err = nvs_set_str(my_handle, (const char *)&reel_id, reel_str);
        printf((err != ESP_OK) ? "Failed to write to NVS!\n" : "Written to NVS successfully\n");
        nvs_close(my_handle);
    }
}

void read_reel_from_nvs(int reel_id, smd_reel_t *reel)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error opening NVS handle!\n");
    }
    else
    {
        char reel_str[512];
        size_t length = sizeof(reel_str);
        err = nvs_get_str(my_handle, (const char *)&reel_id, reel_str, &length);
        if (err != ESP_OK)
        {
            printf("Failed to read from NVS!\n");
        }
        else
        {
            sscanf(reel_str, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,]", reel->value, reel->package, reel->quantity, reel->manufacturer, reel->part_number, reel->sku, reel->comp_type);
        }
        nvs_close(my_handle);
    }
}

esp_err_t get_handler(httpd_req_t *req)
{
    char *buf;
    size_t buf_len;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK)
        {
            printf("Found header => Host: %s\n", buf);
        }
        free(buf);
    }

    /* Send a simple response */
    const char *resp = "URI Handler works!";
    httpd_resp_send(req, resp, strlen(resp));

    return ESP_OK;
}

httpd_uri_t uri_get = {
    .uri = "/get",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL};

void start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &uri_get);
    }
    else
    {
        ESP_LOGI(TAG, "Error starting server!");
    }
}