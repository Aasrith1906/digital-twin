#include <stdio.h>
#include <stdlib.h>
#include "mongoose.h"
#include "device.h"

#ifndef API_H
#define API_H
#define HOSTNAME "http://0.0.0.0:5000"
#define JSON_BUFFER 128

extern const char *hostname;
typedef enum
{
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_SYSTEM_STATUS,
    TYPE_TIMESTAMP
} field_type;
typedef void (*route_handler)(struct mg_connection *c, struct mg_http_message *hm);

typedef struct {
    const char *method; 
    const char *uri;
    route_handler handler; 
} route_entry; 




void ev_handler(struct mg_connection *c, int ev, void *ev_data);
void *listen_api_events(void *arg);
void api_get_current_status(struct mg_connection *c, struct mg_http_message *hm);
void api_get_current_params(struct mg_connection *c, struct mg_http_message *hm);
void api_post_update_power(struct mg_connection *c, struct mg_http_message *hm);
void api_post_update_status(struct mg_connection *c, struct mg_http_message *hm);
char *parse_status_payload_json(status_payload *payload);
char *parse_param_payload_json(system_params *params);
#endif