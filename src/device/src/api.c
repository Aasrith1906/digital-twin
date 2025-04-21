// NEED TO CONVERT TO USE MQTT WITH AZURE IOT

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "api.h"
#include "mongoose.h"
#include "device.h"

void ev_handler(struct mg_connection *c, int ev, void *ev_data)
{
    route_entry routes[] = {
        {"GET", "/api/parameters/", api_get_current_params},
        {"GET", "/api/status/", api_get_current_status},
        {"POST", "/api/parameters/power", api_post_update_power},
        {"POST", "/api/parameters/status", api_post_update_status},
        {NULL, NULL, NULL}};

    if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;

        for (int i = 0; routes[i].method != NULL; i++)
        {
            if (mg_match(hm->uri, mg_str(routes[i].uri), NULL) && mg_strcmp(hm->method, mg_str(routes[i].method)) == 0)
            {
                routes[i].handler(c, hm);
            }
        }

        mg_http_reply(c, 401, "Content-Type: application/json\r\n", "{%s: %s}", MG_ESC("message"), MG_ESC("Unauthorized"));
        return;
    }
}

void api_get_current_status(struct mg_connection *c, struct mg_http_message *hm)
{
    printf("Received Message :\n %s \n", hm->uri);
    status_payload *current_status_payload;
    current_status_payload = peek_reading();
    if (current_status_payload == NULL)
    {
        fprintf(stderr, "No Status Payloads to Return \n");
        mg_http_reply(c, 404, "Content-Type: application/json\r\n", "{%s: %s}", MG_ESC("message"), MG_ESC("No Available Messages"));
        return;
    }
    char *json = parse_status_payload_json(current_status_payload);
    mg_http_reply(c, 401, "Content-Type: application/json\r\n", "%s", json);
    free(json);
    return;
}

void api_get_current_params(struct mg_connection *c, struct mg_http_message *hm)
{
    printf("Received Message :\n %s \n", hm->uri);

    if (current_system_params == NULL)
    {
        fprintf(stderr, "No Status Parameters to Return \n");
        mg_http_reply(c, 404, "Content-Type: application/json\r\n", "{%s: %s}", MG_ESC("message"), MG_ESC("No Parameters, Initialise System First"));
        return;
    }
    char *json = parse_param_payload_json(current_system_params);
    print_system_params(current_system_params);
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", json);
    free(json);
    return;
}

void api_post_update_power(struct mg_connection *c, struct mg_http_message *hm){

    double updated_power; 
    if(!mg_json_get_num(hm->body, "$.power", &updated_power) || current_system_params->status!= RUNNING){
        mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{%s: %s}", MG_ESC("message"), MG_ESC("Invalid Payload"));
    }

    
    printf("Device New Power Configuration: %f \n", updated_power);

    change_power_consumption((float) updated_power);
    api_get_current_params(c, hm);
}

void api_post_update_status(struct mg_connection *c, struct mg_http_message *hm){
    double updated_status;
    if(!mg_json_get_num(hm->body, "$.status", &updated_status)){
        mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{%s: %s}", MG_ESC("message"), MG_ESC("Invalid Payload"));
    }
    printf("Device New Power Configuration: %d \n", (enum system_status) updated_status);
    change_system_status((enum system_status) updated_status);
    api_get_current_params(c, hm);
}

char *parse_status_payload_json(status_payload *payload)
{

    char *json = malloc(512);
    if (!json)
        return NULL;

    char temp[128];
    json[0] = '\0';

    strcat(json, "{");

#define X(type, name)                                                                                \
    do                                                                                               \
    {                                                                                                \
        if (strcmp(#name, "status") == 0)                                                            \
        {                                                                                            \
            snprintf(temp, sizeof(temp), "\"%s\": \"%s\",", #name, get_status_str(payload->status)); \
        }                                                                                            \
        else if (strcmp(#name, "timestamp") == 0)                                                    \
        {                                                                                            \
            snprintf(temp, sizeof(temp), "\"%s\": %ld,", #name, payload->timestamp);                 \
        }                                                                                            \
        else                                                                                         \
        {                                                                                            \
            snprintf(temp, sizeof(temp), "\"%s\": %.2f,", #name, (double)payload->name);             \
        }                                                                                            \
        strcat(json, temp);                                                                          \
    }                                                                                                \
    \ 
while(0);                                                                                            \
    DEVICE_SYSTEM_STATUS_FIELDS
#undef X

    size_t len = strlen(json);
    if (len > 0 && json[len - 1] == ',')
    {
        json[len - 1] = '\0';
    }

    strcat(json, "}");
    return json;
}

char *parse_param_payload_json(system_params *params)
{
    char *json = malloc(512);
    if (!json)
        return NULL;

    char temp[128];
    json[0] = '\0';

    strcat(json, "{");

#define X(type, name)                                                                               \
    do                                                                                              \
    {                                                                                               \
        if (strcmp(#name, "status") == 0)                                                           \
        {                                                                                           \
            snprintf(temp, sizeof(temp), "\"%s\": \"%s\",", #name, get_status_str(params->status)); \
        }                                                                                           \
        else                                                                                        \
        {                                                                                           \
            snprintf(temp, sizeof(temp), "\"%s\": %.2f,", #name, (double)params->name);             \
        }                                                                                           \
        strcat(json, temp);                                                                         \
    } while (0);

    DEVICE_SYSTEM_PARAM_FIELDS

#undef X

    size_t len = strlen(json);
    if (len > 0 && json[len - 1] == ',')
    {
        json[len - 1] = '\0';
    }

    strcat(json, "}");
    printf("%s \n", json);
    return json;
}

void *listen_api_events(void *arg)
{
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, HOSTNAME, ev_handler, NULL);
    for (;;)
        mg_mgr_poll(&mgr, 1000);

    mg_mgr_free(&mgr);
}
