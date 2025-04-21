#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "device.h"

system_params *current_system_params = NULL;
status_payload *current_status_payload = NULL;
status_payload_queue *queue = NULL;

void set_default_system_params(void)
{
    current_system_params = (system_params *)malloc(sizeof(system_params));

    if (!current_system_params)
    {
        return;
    }

    current_system_params->power_usage = 0;
    current_system_params->status = STOPPED;

    queue = (status_payload_queue *)malloc(sizeof(status_payload_queue));
    if (!queue)
    {
        return;
    }

    queue->front = 0;
    queue->count = 0;
}

int isFull(){
    if(!queue){
        return -1;
    }
    if(queue->count == MAX_READINGS){
        return 1;
    }
    return 0;
}

int isEmpty(){
    if(!queue){
        return -1;
    }
    if(queue->count == 0){
        return 1;
    }
    return 0; 
}


status_payload *add_current_reading(void){

    int index = (queue->front+ queue->count) % MAX_READINGS;

    if(isFull()){
        queue->front = (queue->front + 1) % MAX_READINGS;
        index = (queue->front+queue->count - 1) % MAX_READINGS;
    }else{
        queue->count++;
    }
    status_payload *current_reading; 
    current_reading = get_current_status();
    queue->arr[index] = *current_reading;
    free(current_reading);
    return &queue->arr[index];
}

status_payload *dequeue_reading(void){
    if(isEmpty()){
        printf("There are no readings in the queue \n"); 
        return (status_payload *) NULL; 
    }
    status_payload *current_reading; 
    current_reading = &queue->arr[queue->front];
    queue->count--; 
    queue->front = (queue->front + 1) % MAX_READINGS;
    return current_reading;
}

status_payload *peek_reading(void){
    if(isEmpty()){
        printf("There are no readings in the queue \n"); 
        return (status_payload *) NULL; 
    }
    status_payload *current_reading; 
    current_reading = &queue->arr[queue->front];
    return current_reading; 
}



status_payload *stop_device(void)
{
    change_system_status(STOPPED);

    status_payload *final_status = get_current_status();

    free(current_system_params);
    free(current_status_payload);
    free(queue);
    current_system_params = NULL;

    return final_status;
}

enum system_status get_system_status(void)
{
    return current_system_params->status;
}

float get_power_usage(void)
{
    return current_system_params->power_usage;
}

float get_efficiency(void)
{
    if (current_system_params->status != RUNNING)
    {
        return 0;
    }

    return EFFICIENCY_LOW + ((float)rand() / RAND_MAX) * (EFFICIENCY_HIGH - EFFICIENCY_LOW);
}

float get_pressure(void)
{
    if (current_system_params->status != RUNNING)
    {
        return 0;
    }

    return PRESSURE_LOW + ((float)rand() / RAND_MAX) * (PRESSURE_HIGH - PRESSURE_LOW);
}

float get_flow_rate(float efficiency, float pressure, float power)
{
    return (power * efficiency) / (pressure * 100000);
}

float get_temperature(void)
{
    if (current_system_params->status != RUNNING)
    {
        return 0;
    }

    return TEMP_LOW + ((float)rand() / RAND_MAX) * (TEMP_HIGH - TEMP_LOW);
}

status_payload *get_current_status(void)
{
    status_payload *current_status = (status_payload *)malloc(sizeof(status_payload));

    current_status->status = get_system_status();
    current_status->current_pressure = get_pressure();
    current_status->efficiency = get_efficiency();
    current_status->power_usage = get_power_usage();
    current_status->current_flow_rate = get_flow_rate(
        current_status->efficiency,
        current_status->current_pressure,
        current_status->power_usage);
    current_status->current_temperature = get_temperature();
    current_status->timestamp = time(NULL);

    return current_status;
}

int change_system_status(enum system_status new_status)
{
    if (!current_system_params)
    {
        printf("Failed to change system status, please initialise device first\n");
        return -1;
    }

    if (get_current_status()->status == ERROR)
    {
        printf("System in error state\n");
        return -1;
    }

    if (new_status == RUNNING && get_current_status()->status == STOPPED)
    {
        printf("Please initialise the device first\n");
        return -1;
    }

    if (new_status == RUNNING && get_current_status()->status == INITIALIZING)
    {
        printf("Please utilise the start device function\n");
        return -1;
    }

    current_system_params->status = new_status;
    return 1;
}

int change_power_consumption(float new_power_setting){
    if(get_current_status()->status != ERROR || get_current_status()->status != INITIALIZING || get_current_status()->status != STOPPED){
        current_system_params->power_usage = new_power_setting;
    }
}

status_payload *initialise_device(void)
{
    current_system_params->status = INITIALIZING;
    current_system_params->power_usage = 0.0;
    return get_current_status();
}

status_payload *start_device(void)
{
    if (current_system_params->status != INITIALIZING)
    {
        printf("Please initialise the device first\n");
        return NULL;
    }

    current_system_params->status = RUNNING;
    current_system_params->power_usage = DEFAULT_POWER_USAGE;

    return get_current_status();
}

void *capture_readings(void *arg){
    if(!queue){
        fprintf(stderr, "Queue not created\n");
        return NULL; 
    }

    if(current_system_params->status != INITIALIZING){
        printf("Initialise Device First \n");
        return NULL;
    }

    start_device();
    while(current_system_params->status == RUNNING)
    {
        add_current_reading();
        sleep(2);
    }
    return NULL;
        

}


char *get_status_str(enum system_status s)
{
    char *status_str;
    switch (s)
    {
    case INITIALIZING:
        status_str = "INITIALIZING";
        break;
    case RUNNING:
        status_str = "RUNNING";
        break;
    case STOPPED:
        status_str = "STOPPED";
        break;
    case ERROR:
        status_str = "ERROR";
        break;
    default:
        status_str = "UNKNOWN";
        break;
    }

    return status_str;
}

void print_status(status_payload *status)
{
    if (!status)
    {
        printf("Status is NULL.\n");
        return;
    }

    char *status_str;
    status_str = get_status_str(status->status);
    char time_buf[26];
    struct tm *tm_info = localtime(&status->timestamp);
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

    printf("----- Water Pump Status -----\n");
    printf("System Status     : %s\n", status_str);
    printf("Pressure          : %.2f bar\n", status->current_pressure);
    printf("Temperature       : %.2f °C\n", status->current_temperature);
    printf("Flow Rate         : %.6f m³/s\n", status->current_flow_rate);
    printf("Power Usage       : %.2f W\n", status->power_usage);
    printf("Efficiency        : %.2f %%\n", status->efficiency * 100);
    printf("Timestamp         : %s\n", time_buf);
    printf("------------------------------\n");
}

void print_system_params(system_params *params)
{
    if (!params)
    {
        printf("System parameters are NULL.\n");
        return;
    }

    char *status_str;
    status_str = get_status_str(params->status);

    printf("----- System Parameters -----\n");
    printf("System Status     : %s\n", status_str);
    printf("Power Usage       : %.2f W\n", params->power_usage);
    printf("------------------------------\n");
}

void print_queue(void) {
    if (!queue) {
        printf("Queue is not initialized.\n");
        return;
    }

    if (queue->count == 0) {
        printf("Queue is empty.\n");
        return;
    }

    printf("----- Queue Contents (%d readings) -----\n", queue->count);

    for (int i = 0; i < queue->count; i++) {
        int index = (queue->front + i) % MAX_READINGS;
        printf("Reading %d (Index %d):\n", i + 1, index);
        print_status(&queue->arr[index]);
    }

    printf("------------- End of Queue -------------\n");
}
