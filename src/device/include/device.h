#ifndef DEVICE_H
#define DEVICE_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Constants
#define EFFICIENCY_LOW        0.7     // Positive displacement pump efficiency range (low)
#define EFFICIENCY_HIGH       0.8     // Positive displacement pump efficiency range (high)
#define PRESSURE_LOW         10.0     // Pressure in bar (low)
#define PRESSURE_HIGH        20.0     // Pressure in bar (high)
#define TEMP_LOW              5.0     // Temperature in Celsius (low)
#define TEMP_HIGH            10.0     // Temperature in Celsius (high)
#define DEFAULT_POWER_USAGE 1000.0    // Default power usage in watts
#define MAX_READINGS 10 // max readings to store in the queue 


#define DEVICE_SYSTEM_STATUS_FIELDS \
    X(enum system_status, status)   \
    X(float, current_pressure)      \
    X(float, current_temperature)   \
    X(float, current_flow_rate)     \
    X(float, power_usage)           \
    X(float, efficiency)            \
    X(time_t, timestamp)

// Enum for system status
enum system_status {
    INITIALIZING,
    RUNNING,
    STOPPED,
    ERROR
};

#define DEVICE_SYSTEM_PARAM_FIELDS \
    X(enum system_status, status)   \
    X(float, power_usage)      


// Struct to hold system parameters
typedef struct {
    #define X(type, name) type name; 
    DEVICE_SYSTEM_PARAM_FIELDS
    #undef X
} system_params;

// Global pointer to current system parameters
extern system_params *current_system_params;


// Struct for system status payload
typedef struct {
    #define X(type, name) type name; 
    DEVICE_SYSTEM_STATUS_FIELDS
    #undef X
} status_payload;

extern status_payload *current_status_payload;

typedef struct{
    status_payload arr[MAX_READINGS];
    int count; 
    int front; 
} status_payload_queue; 

extern status_payload_queue *queue;  

// Function declarations
void set_default_system_params(void);
status_payload *start_device(void);
status_payload *initialise_device(void);
status_payload *stop_device(void);
status_payload *get_current_status(void);

void *capture_readings(void *arg);

char *get_status_str(enum system_status s);
void print_status(status_payload *status);
void print_system_params(system_params *params);
void print_queue(void);
status_payload *add_current_reading(void);
status_payload *dequeue_reading(void);
status_payload *peek_reading(void);

int isFull(void);
int isEmpty(void);

int change_system_status(enum system_status new_status);
enum system_status get_system_status(void);
int change_power_consumption(float new_power_setting);

float get_pressure(void);
float get_flow_rate(float efficiency, float pressure, float power);
float get_temperature(void);
float get_power_usage(void);
float get_efficiency(void);



#endif // DEVICE_H
