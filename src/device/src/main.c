#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <unistd.h>
#include "device.h"
#include "mongoose.h"
#include "api.h"

int main(){
    srand(time(NULL));

    status_payload *current_system_status;
    set_default_system_params(); 
    current_system_status = initialise_device();
    print_system_params(current_system_params);
    free(current_system_status);

    if(current_system_params->status != INITIALIZING){
        printf("Initialise device first \n");
        return -1; 
    }

    pthread_t device_readings;
    if(pthread_create(&device_readings, NULL, capture_readings, NULL)!=0){
        fprintf(stderr, "Failed to create capture thread\n");
    }

    pthread_t http_api; 
    if(pthread_create(&http_api, NULL, listen_api_events, NULL)!=0){
        fprintf(stderr, "Failed to create API thread\n");
    }

    while (current_system_params->status != RUNNING) {
        sleep(1); 
    }

    print_system_params(current_system_params); 

    while(current_system_params->status == RUNNING){
        sleep(5);
        print_queue();
    }

    pthread_join(device_readings, NULL);
    pthread_join(http_api, NULL);

    stop_device();
    return -1; 
    
}