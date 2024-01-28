/* HTTP Restful API Server

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <fcntl.h>
#include "esp_http_server.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "cJSON.h"
#include <string.h>
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

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

typedef struct rest_server_context
{
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

rack_settings_t rack_settings;
smd_reel_t reels[MAX_REELS];

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html"))
    {
        type = "text/html";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".js"))
    {
        type = "application/javascript";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".css"))
    {
        type = "text/css";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".png"))
    {
        type = "image/png";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".ico"))
    {
        type = "image/x-icon";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".svg"))
    {
        type = "text/xml";
    }
    return httpd_resp_set_type(req, type);
}

/* Send HTTP response with the contents of the requested file */
static esp_err_t rest_common_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];

    rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;
    strlcpy(filepath, rest_context->base_path, sizeof(filepath));
    if (req->uri[strlen(req->uri) - 1] == '/')
    {
        strlcat(filepath, "/index.html", sizeof(filepath));
    }
    else
    {
        strlcat(filepath, req->uri, sizeof(filepath));
    }
    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1)
    {
        ESP_LOGE(REST_TAG, "Failed to open file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    set_content_type_from_file(req, filepath);

    char *chunk = rest_context->scratch;
    ssize_t read_bytes;
    do
    {
        /* Read file in chunks into the scratch buffer */
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1)
        {
            ESP_LOGE(REST_TAG, "Failed to read file : %s", filepath);
        }
        else if (read_bytes > 0)
        {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK)
            {
                close(fd);
                ESP_LOGE(REST_TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);
    ESP_LOGI(REST_TAG, "File sending complete");
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
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
    read_reels_from_vfs(reels, rack_settings.numReelsPerRow * rack_settings.numRows);
    cJSON *reels_json = cJSON_CreateArray();
    for (int i = 0; i < rack_settings.numReelsPerRow * rack_settings.numRows; i++)
    {
        cJSON *reel_json = cJSON_CreateObject();
        cJSON_AddStringToObject(reel_json, "value", reels[i].value);
        cJSON_AddStringToObject(reel_json, "package", reels[i].package);
        cJSON_AddStringToObject(reel_json, "part_number", reels[i].part_number);
        cJSON_AddStringToObject(reel_json, "comp_type", reels[i].comp_type);
        cJSON_AddStringToObject(reel_json, "sku", reels[i].sku);
        cJSON_AddStringToObject(reel_json, "manufacturer", reels[i].manufacturer);
        cJSON_AddStringToObject(reel_json, "quantity", reels[i].quantity);
        cJSON_AddItemToArray(reels_json, reel_json);
    }

    return ESP_OK;
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
    int reel_id = cJSON_GetObjectItem(root, "reel_id")->valueint;
    cJSON *reel_json = cJSON_GetObjectItem(root, "reel");
    cJSON *value_json = cJSON_GetObjectItem(reel_json, "value");
    cJSON *package_json = cJSON_GetObjectItem(reel_json, "package");
    cJSON *part_number_json = cJSON_GetObjectItem(reel_json, "part_number");
    cJSON *comp_type_json = cJSON_GetObjectItem(reel_json, "comp_type");
    cJSON *sku_json = cJSON_GetObjectItem(reel_json, "sku");
    cJSON *manufacturer_json = cJSON_GetObjectItem(reel_json, "manufacturer");
    cJSON *quantity_json = cJSON_GetObjectItem(reel_json, "quantity");

    smd_reel_t reel;
    strcpy(reel.value, value_json->valuestring);
    strcpy(reel.package, package_json->valuestring);
    strcpy(reel.part_number, part_number_json->valuestring);
    strcpy(reel.comp_type, comp_type_json->valuestring);
    strcpy(reel.sku, sku_json->valuestring);
    strcpy(reel.manufacturer, manufacturer_json->valuestring);
    strcpy(reel.quantity, quantity_json->valuestring);

    save_reel_to_vfs(reel_id, &reel);

    cJSON_Delete(root);
    httpd_resp_sendstr(req, "Post control value successfully");
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

    char *ledColourStr = cJSON_GetObjectItem(root, "ledColour")->valuestring;
    rack_settings.ledColour = strtol(ledColourStr + 1, NULL, 16); // Skip the '#' character

    save_rack_settings_to_vfs(&rack_settings);

    // Do something with the extracted data
    // print to terminal
    ESP_LOGI(REST_TAG, "numReelsPerRow: %d", rack_settings.numReelsPerRow);
    ESP_LOGI(REST_TAG, "numRows: %d", rack_settings.numRows);
    ESP_LOGI(REST_TAG, "ledColour: %ld", rack_settings.ledColour);

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

esp_err_t start_rest_server(const char *base_path)
{
    REST_CHECK(base_path, "wrong base path", err);
    rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
    REST_CHECK(rest_context, "No memory for rest context", err);
    strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(REST_TAG, "Starting HTTP Server");
    REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed", err_start);

    // get rack settings
    get_rack_settings_from_vfs(&rack_settings);

    // configure led strip
    configure_led(rack_settings.numReelsPerRow * rack_settings.numRows);

    // boot sequence
    boot_sequence((rack_settings.ledColour >> 16) & 0xFF, (rack_settings.ledColour >> 8) & 0xFF, rack_settings.ledColour & 0xFF);

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
