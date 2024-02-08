/* HTTP Restful API Server

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <fcntl.h>
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "esp_ota_ops.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/inet.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "db.h"
#include "rgb.h"

static const char *REST_TAG = "esp-rest";
#define REST_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                 \
    {                                                                                  \
        if (!(a))                                                                      \
        {                                                                              \
            ESP_LOGE(REST_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                             \
        }                                                                              \
    } while (0)

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define EXAMPLE_DEFAULT_SSID "reelrack"
#define EXAMPLE_DEFAULT_PWD "reelrackmk1"

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

#define EXAMPLE_ESP_WIFI_SSID "reelrack"
#define EXAMPLE_ESP_WIFI_PASS "reelrackmk1"
#define EXAMPLE_ESP_WIFI_CHANNEL 0
#define EXAMPLE_MAX_STA_CONN 4

typedef struct rest_server_context
{
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

rack_settings_t rack_settings;
smd_reel_t reels[MAX_REELS];

/* Send HTTP response with the contents of the requested file */
static esp_err_t rest_common_get_handler(httpd_req_t *req)
{
    // we are now accessing the files directly from flash as they are embedded, instead of the file system

    extern const uint8_t index_html_start[] asm("_binary_index_html_start");
    extern const uint8_t index_html_end[] asm("_binary_index_html_end");
    extern const uint8_t script_js_start[] asm("_binary_script_js_start");
    extern const uint8_t script_js_end[] asm("_binary_script_js_end");
    extern const uint8_t style_css_start[] asm("_binary_style_css_start");
    extern const uint8_t style_css_end[] asm("_binary_style_css_end");

    const char *data;
    size_t len;
    const char *content_type;

    if (strcmp(req->uri, "/") == 0 || strcmp(req->uri, "/index.html") == 0)
    {
        data = (const char *)index_html_start;
        len = index_html_end - index_html_start - 1;
        content_type = "text/html";
    }
    else if (strcmp(req->uri, "/script.js") == 0)
    {
        data = (const char *)script_js_start;
        len = script_js_end - script_js_start - 1;
        content_type = "application/javascript";
    }
    else if (strcmp(req->uri, "/style.css") == 0)
    {
        data = (const char *)style_css_start;
        len = style_css_end - style_css_start - 1;
        content_type = "text/css";
    }
    else
    {
        ESP_LOGE(REST_TAG, "Unknown URI : %s", req->uri);
        /* Respond with 404 Not Found */
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, content_type);
    httpd_resp_send(req, data, len);

    ESP_LOGI(REST_TAG, "File sending complete");
    return ESP_OK;
}

/* Simple handler for getting system handler */
static esp_err_t system_info_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    cJSON_AddStringToObject(root, "version", IDF_VER);
    cJSON_AddNumberToObject(root, "cores", chip_info.cores);
    const char *sys_info = cJSON_Print(root);
    httpd_resp_sendstr(req, sys_info);
    free((void *)sys_info);
    cJSON_Delete(root);
    return ESP_OK;
}

/* Simple handler for getting reel data */
static esp_err_t reel_data_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    int numReels = read_reels_from_vfs(reels, rack_settings.numReelsPerRow * rack_settings.numRows);
    if (numReels > 0)
    {
        cJSON *reels_json = cJSON_CreateArray();
        for (int i = 0; i < numReels; i++)
        {
            cJSON *reel_json = cJSON_CreateObject();
            // cJSON_AddNumberToObject(reel_json, "id", reels[i].id);
            cJSON_AddNumberToObject(reel_json, "valid", reels[i].valid);
            cJSON_AddStringToObject(reel_json, "value", reels[i].value);
            cJSON_AddStringToObject(reel_json, "package", reels[i].package);
            cJSON_AddStringToObject(reel_json, "part_number", reels[i].part_number);
            cJSON_AddStringToObject(reel_json, "comp_type", reels[i].comp_type);
            cJSON_AddStringToObject(reel_json, "sku", reels[i].sku);
            cJSON_AddStringToObject(reel_json, "manufacturer", reels[i].manufacturer);
            cJSON_AddNumberToObject(reel_json, "quantity", reels[i].quantity);
            cJSON_AddItemToArray(reels_json, reel_json);
        }

        const char *reels_str = cJSON_Print(reels_json);
        httpd_resp_sendstr(req, reels_str);
        free((void *)reels_str);
        cJSON_Delete(reels_json);

        // print success
        ESP_LOGI(REST_TAG, "Reel data sent successfully - reel_data_get_handler()");

        return ESP_OK;
    }

    else
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read reel data");

        // print the failure message
        ESP_LOGE(REST_TAG, "Failed to read reel data - reel_data_get_handler()");

        return ESP_FAIL;
    }
}

/* Simple handler for storing reel data */
static esp_err_t reel_data_post_handler(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len)
    {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0)
        {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    cJSON *root = cJSON_Parse(buf);
    if (root == NULL)
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse JSON");
        return ESP_FAIL;
    }

    smd_reel_t reel;
    int reel_id = -1;

    cJSON *id_json = cJSON_GetObjectItem(root, "id");
    if (id_json != NULL)
    {
        reel_id = id_json->valueint;
    }

    cJSON *value_json = cJSON_GetObjectItem(root, "value");
    if (value_json != NULL)
    {
        strcpy(reel.value, value_json->valuestring);
    }

    cJSON *package_json = cJSON_GetObjectItem(root, "package");
    if (package_json != NULL)
    {
        strcpy(reel.package, package_json->valuestring);
    }

    cJSON *part_number_json = cJSON_GetObjectItem(root, "part_number");
    if (part_number_json != NULL)
    {
        strcpy(reel.part_number, part_number_json->valuestring);
    }

    cJSON *comp_type_json = cJSON_GetObjectItem(root, "comp_type");
    if (comp_type_json != NULL)
    {
        strcpy(reel.comp_type, comp_type_json->valuestring);
    }

    cJSON *sku_json = cJSON_GetObjectItem(root, "sku");
    if (sku_json != NULL)
    {
        strcpy(reel.sku, sku_json->valuestring);
    }

    cJSON *manufacturer_json = cJSON_GetObjectItem(root, "manufacturer");
    if (manufacturer_json != NULL)
    {
        strcpy(reel.manufacturer, manufacturer_json->valuestring);
    }

    cJSON *quantity_json = cJSON_GetObjectItem(root, "quantity");
    if (quantity_json != NULL)
    {
        reel.quantity = quantity_json->valueint;
    }

    reel.valid = true;

    save_reel_to_vfs(reel_id, &reel);

    cJSON_Delete(root);
    httpd_resp_sendstr(req, "Post control value successfully");
    return ESP_OK;
}

// reel data delete handler
static esp_err_t reel_data_delete_handler(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len)
    {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0)
        {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    cJSON *root = cJSON_Parse(buf);
    if (root == NULL)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse JSON data");
        return ESP_FAIL;
    }

    cJSON *id_json = cJSON_GetObjectItem(root, "id");
    if (id_json == NULL)
    {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No 'id' in JSON");
        return ESP_FAIL;
    }
    int reel_id = id_json->valueint;

    if (delete_reel_from_vfs(reel_id, &rack_settings) == ESP_OK)
    {
        cJSON_Delete(root);
        httpd_resp_sendstr(req, "Reel deleted successfully");
        return ESP_OK;
    }
    else
    {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to delete reel");
        return ESP_FAIL;
    }

    return ESP_OK;
}

// rack settings handlers
static esp_err_t rack_settings_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    get_rack_settings_from_vfs(&rack_settings);
    cJSON *rack_settings_json = cJSON_CreateObject();

    cJSON_AddNumberToObject(rack_settings_json, "numReelsPerRow", rack_settings.numReelsPerRow);
    cJSON_AddNumberToObject(rack_settings_json, "numRows", rack_settings.numRows);
    cJSON_AddNumberToObject(rack_settings_json, "ledBrightness", rack_settings.ledBrightness);
    cJSON_AddNumberToObject(rack_settings_json, "ledTimeout", rack_settings.ledTimeout);
    cJSON_AddStringToObject(rack_settings_json, "ssid", rack_settings.ssid);
    cJSON_AddStringToObject(rack_settings_json, "password", rack_settings.password);

    // add a string for the colour and include the leading '#'
    char ledColourStr[8];
    sprintf(ledColourStr, "#%06lX", rack_settings.ledColour);
    cJSON_AddStringToObject(rack_settings_json, "ledColour", ledColourStr);
    const char *rack_settings_str = cJSON_Print(rack_settings_json);
    httpd_resp_sendstr(req, rack_settings_str);
    free((void *)rack_settings_str);
    cJSON_Delete(rack_settings_json);
    return ESP_OK;
}

static esp_err_t rack_settings_post_handler(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len)
    {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0)
        {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    cJSON *root = cJSON_Parse(buf);
    if (root == NULL)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse JSON data");
        return ESP_FAIL;
    }

    ESP_LOGI(REST_TAG, "Received JSON data: %s", buf);

    rack_settings.numReelsPerRow = atoi(cJSON_GetObjectItem(root, "numReelsPerRow")->valuestring);
    rack_settings.numRows = atoi(cJSON_GetObjectItem(root, "numRows")->valuestring);
    rack_settings.ledBrightness = atoi(cJSON_GetObjectItem(root, "ledBrightness")->valuestring);
    rack_settings.ledTimeout = atoi(cJSON_GetObjectItem(root, "ledTimeout")->valuestring);

    cJSON *ssid_item = cJSON_GetObjectItem(root, "ssid");
    if (ssid_item != NULL && cJSON_IsString(ssid_item) && (ssid_item->valuestring != NULL))
    {
        strncpy(rack_settings.ssid, ssid_item->valuestring, sizeof(rack_settings.ssid) - 1);
        rack_settings.ssid[sizeof(rack_settings.ssid) - 1] = '\0'; // Ensure null termination
    }

    cJSON *password_item = cJSON_GetObjectItem(root, "password");
    if (password_item != NULL && cJSON_IsString(password_item) && (password_item->valuestring != NULL))
    {
        strncpy(rack_settings.password, password_item->valuestring, sizeof(rack_settings.password) - 1);
        rack_settings.password[sizeof(rack_settings.password) - 1] = '\0'; // Ensure null termination
    }

    char *ledColourStr = cJSON_GetObjectItem(root, "ledColour")->valuestring;
    rack_settings.ledColour = strtol(ledColourStr + 1, NULL, 16); // Skip the '#' character

    save_rack_settings_to_vfs(&rack_settings);

    // Do something with the extracted data
    // print to terminal
    ESP_LOGI(REST_TAG, "numReelsPerRow: %d", rack_settings.numReelsPerRow);
    ESP_LOGI(REST_TAG, "numRows: %d", rack_settings.numRows);
    ESP_LOGI(REST_TAG, "ledColour: %ld", rack_settings.ledColour);
    ESP_LOGI(REST_TAG, "ledBrightness: %ld", rack_settings.ledBrightness);
    ESP_LOGI(REST_TAG, "ledTimeout: %ld", rack_settings.ledTimeout);
    ESP_LOGI(REST_TAG, "ssid: %s", rack_settings.ssid);
    ESP_LOGI(REST_TAG, "password: %s", rack_settings.password);

    cJSON_Delete(root);
    httpd_resp_sendstr(req, "Post rack settings successfully");
    return ESP_OK;
}

// rgb handler
static esp_err_t rgb_post_handler(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len)
    {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0)
        {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    cJSON *root = cJSON_Parse(buf);
    if (root == NULL)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to parse JSON data");
        return ESP_FAIL;
    }

    ESP_LOGI(REST_TAG, "Received JSON data: %s", buf);

    cJSON *reelID_item = cJSON_GetObjectItem(root, "reelID");
    if (reelID_item == NULL)
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get reelID from JSON data");
        return ESP_FAIL;
    }
    else
    {
        uint32_t ledIndex = (uint32_t)cJSON_GetNumberValue(reelID_item);
        if ((ledIndex >= rack_settings.numReelsPerRow * rack_settings.numRows))
            show_error_led((rack_settings.ledColour >> 16) & 0xFF, (rack_settings.ledColour >> 8) & 0xFF, rack_settings.ledColour & 0xFF);

        else
            show_led(ledIndex, (rack_settings.ledColour >> 16) & 0xFF, (rack_settings.ledColour >> 8) & 0xFF, rack_settings.ledColour & 0xFF);
    }

    cJSON_Delete(root);
    httpd_resp_sendstr(req, "Post control value successfully");
    return ESP_OK;
}

// Handler for the form submission
static esp_err_t update_post_handler(httpd_req_t *req)
{
    char buf[2000];
    esp_err_t err = ESP_OK;

    httpd_resp_set_status(req, HTTPD_500); // Assume failure

    int ret, remaining = req->content_len;
    printf("Receiving\n");

    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (update_partition == NULL)
    {
        printf("Uh oh, bad things\n");
        goto return_failure;
    }

    printf("Writing partition: type %d, subtype %d, offset 0x%08x\n", update_partition->type, update_partition->subtype, update_partition->address);
    printf("Running partition: type %d, subtype %d, offset 0x%08x\n", running->type, running->subtype, running->address);
    err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
    if (err != ESP_OK)
    {
        printf("esp_ota_begin failed (%s)", esp_err_to_name(err));
        goto return_failure;
    }
    while (remaining > 0)
    {
        // Read the data for the request
        if ((ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                // Retry receiving if timeout occurred
                continue;
            }

            goto return_failure;
        }

        size_t bytes_read = ret;

        remaining -= bytes_read;
        err = esp_ota_write(update_handle, buf, bytes_read);
        if (err != ESP_OK)
        {
            goto return_failure;
        }
    }

    printf("Receiving done\n");

    // End response
    if ((esp_ota_end(update_handle) == ESP_OK) &&
        (esp_ota_set_boot_partition(update_partition) == ESP_OK))
    {
        printf("OTA Success?!\n Rebooting\n");
        fflush(stdout);

        httpd_resp_set_status(req, HTTPD_200);
        httpd_resp_send(req, NULL, 0);

        vTaskDelay(pdMS_TO_TICKS(2000));
        esp_restart();

        return ESP_OK;
    }
    printf("OTA End failed (%s)!\n", esp_err_to_name(err));

return_failure:
    if (update_handle)
    {
        esp_ota_abort(update_handle);
    }

    httpd_resp_set_status(req, HTTPD_500); // Assume failure
    httpd_resp_send(req, NULL, 0);
    return ESP_FAIL;
}

// Handler for the reset request
static esp_err_t reset_post_handler(httpd_req_t *req)
{
    // Send a response
    const char *response = "OK";
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_set_status(req, HTTPD_200);
    httpd_resp_send(req, response, strlen(response));

    // Delay to allow the response to be sent
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // Reset the device
    esp_restart();

    return ESP_OK;
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_START)
    {
        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
        if (netif == NULL)
        {
            ESP_LOGI(REST_TAG, "Could not get AP network interface.");
            return;
        }

        esp_netif_ip_info_t info;
        // Set the IP address to 10.0.0.1
        IP4_ADDR(&info.ip, 10, 0, 0, 1);
        // Set the gateway to 10.0.0.1
        IP4_ADDR(&info.gw, 10, 0, 0, 1);
        // Set the netmask to 255.255.255.0
        IP4_ADDR(&info.netmask, 255, 255, 255, 0);

        esp_err_t err = esp_netif_dhcps_stop(netif);
        if (err != ESP_OK)
        {
            ESP_LOGI(REST_TAG, "Failed to stop DHCP server: %s", esp_err_to_name(err));
            return;
        }

        err = esp_netif_set_ip_info(netif, &info);
        if (err != ESP_OK)
        {
            ESP_LOGI(REST_TAG, "Failed to set IP info: %s", esp_err_to_name(err));
            return;
        }

        err = esp_netif_get_ip_info(netif, &info);
        if (err != ESP_OK)
        {
            ESP_LOGI(REST_TAG, "Failed to get IP info: %s", esp_err_to_name(err));
            return;
        }

        ESP_LOGI(REST_TAG, "IP info: IP " IPSTR ", Gateway " IPSTR ", Netmask " IPSTR,
                 IP2STR(&info.ip), IP2STR(&info.gw), IP2STR(&info.netmask));

        esp_netif_dns_info_t dns;
        IP4_ADDR(&dns.ip.u_addr.ip4, 10, 0, 2, 0);
        err = esp_netif_set_dns_info(netif, ESP_NETIF_DNS_MAIN, &dns);
        if (err != ESP_OK)
        {
            ESP_LOGI(REST_TAG, "Failed to set DNS info: %s", esp_err_to_name(err));
            return;
        }

        err = esp_netif_dhcps_start(netif);
        if (err != ESP_OK)
        {
            ESP_LOGI(REST_TAG, "Failed to start DHCP server: %s", esp_err_to_name(err));
            return;
        }

        ESP_LOGI(REST_TAG, "DHCP server started");
    }
    else if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(REST_TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(REST_TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(REST_TAG, "got ip:%s",
                 ip4addr_ntoa(&event->ip_info.ip));
    }
}

void wifi_init_common(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
}

void wifi_init_softap(void)
{
    esp_netif_create_default_wifi_ap();

    wifi_config_t wifi_config;
    strcpy((char *)wifi_config.ap.ssid, EXAMPLE_ESP_WIFI_SSID);
    wifi_config.ap.ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID);
    strcpy((char *)wifi_config.ap.password, EXAMPLE_ESP_WIFI_PASS);
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    wifi_config.ap.channel = EXAMPLE_ESP_WIFI_CHANNEL;
    wifi_config.ap.max_connection = EXAMPLE_MAX_STA_CONN;

    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(REST_TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

esp_err_t wifi_init_station(void)
{
    esp_netif_create_default_wifi_sta();

    wifi_config_t wifi_config = {};
    strncpy((char *)wifi_config.sta.ssid, rack_settings.ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, rack_settings.password, sizeof(wifi_config.sta.password));

    // print wifi_config
    ESP_LOGI(REST_TAG, "ssid: %s", wifi_config.sta.ssid);
    ESP_LOGI(REST_TAG, "password: %s", wifi_config.sta.password);

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Wait for the device to connect to the WiFi network
    vTaskDelay(pdMS_TO_TICKS(10000)); // wait for 5 seconds

    // Check if the device is connected to the WiFi network
    if (esp_wifi_connect() != ESP_OK)
    {
        ESP_LOGI(REST_TAG, "Failed to connect to the WiFi network");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t start_rest_server(const char *base_path)
{
    // assign default values just in case, overwrite if successfully retrieved
    rack_settings.numReelsPerRow = 36;
    rack_settings.numRows = 2;
    rack_settings.ledBrightness = 100;
    rack_settings.ledTimeout = 30000;
    rack_settings.ledColour = 0x00FF00;
    strcpy(rack_settings.ssid, EXAMPLE_DEFAULT_SSID);
    strcpy(rack_settings.password, EXAMPLE_DEFAULT_PWD);
    get_rack_settings_from_vfs(&rack_settings);

    // configure led strip
    configure_led(rack_settings.numReelsPerRow * rack_settings.numRows);

    // boot sequence
    boot_sequence((rack_settings.ledColour >> 16) & 0xFF, (rack_settings.ledColour >> 8) & 0xFF, rack_settings.ledColour & 0xFF);

    // start wifi, try to connect to known network as station, if it fails, start as AP
    wifi_init_common();
    if (wifi_init_station() != ESP_OK)
    {
        wifi_init_softap();
    }

    REST_CHECK(base_path, "wrong base path", err);
    rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
    REST_CHECK(rest_context, "No memory for rest context", err);
    strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = {
        .task_priority = tskIDLE_PRIORITY + 5,
        .stack_size = 32768,
        .core_id = tskNO_AFFINITY,
        .server_port = 80,
        .ctrl_port = 32768,
        .max_open_sockets = 7,
        .max_uri_handlers = 10,
        .max_resp_headers = 10,
        .backlog_conn = 5,
        .lru_purge_enable = false,
        .recv_wait_timeout = 5,
        .send_wait_timeout = 5,
        .global_user_ctx = NULL,
        .global_user_ctx_free_fn = NULL,
        .global_transport_ctx = NULL,
        .global_transport_ctx_free_fn = NULL,
        .enable_so_linger = false,
        .linger_timeout = 0,
        .open_fn = NULL,
        .close_fn = NULL,
        .uri_match_fn = NULL};

    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(REST_TAG, "Starting HTTP Server");
    REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed", err_start);

    /* URI handler for fetching system info */
    httpd_uri_t system_info_get_uri = {
        .uri = "/api/v1/system/info",
        .method = HTTP_GET,
        .handler = system_info_get_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &system_info_get_uri);

    /* URI handler for fetching temperature data */
    httpd_uri_t reel_data_get_uri = {
        .uri = "/api/v1/reel/get_all",
        .method = HTTP_GET,
        .handler = reel_data_get_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &reel_data_get_uri);

    httpd_uri_t reel_data_post_uri = {
        .uri = "/api/v1/reel/save",
        .method = HTTP_POST,
        .handler = reel_data_post_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &reel_data_post_uri);

    httpd_uri_t reel_data_delete_uri = {
        .uri = "/api/v1/reel/delete",
        .method = HTTP_DELETE,
        .handler = reel_data_delete_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &reel_data_delete_uri);

    // rack settings post and get
    httpd_uri_t rack_settings_get_uri = {
        .uri = "/api/v1/rack_settings/get",
        .method = HTTP_GET,
        .handler = rack_settings_get_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &rack_settings_get_uri);

    httpd_uri_t rack_settings_post_uri = {
        .uri = "/api/v1/rack_settings/save",
        .method = HTTP_POST,
        .handler = rack_settings_post_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &rack_settings_post_uri);

    // rgb post
    httpd_uri_t rgb_post_uri = {
        .uri = "/api/v1/rgb/show",
        .method = HTTP_POST,
        .handler = rgb_post_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &rgb_post_uri);

    /* URI handler for updating firmware */
    httpd_uri_t update_post_uri = {
        .uri = "/api/v1/update",
        .method = HTTP_POST,
        .handler = update_post_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &update_post_uri);

    /* URI handler for resetting the device */
    httpd_uri_t reset_post_uri = {
        .uri = "/api/v1/reset",
        .method = HTTP_POST,
        .handler = reset_post_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &reset_post_uri);

    /* URI handler for getting web server files */
    httpd_uri_t common_get_uri = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = rest_common_get_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &common_get_uri);

    return ESP_OK;
err_start:
    free(rest_context);
err:
    return ESP_FAIL;
}
