/*
 * LED blink with FreeRTOS
 */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/adc.h"

#include <math.h>
#include <stdlib.h>

QueueHandle_t xQueueAdc;

typedef struct adc {
    int axis;
    int val;
} adc_t;

void x_task(void *p) {
    adc_init();
    adc_gpio_init(26);
    
    adc_t str;
    int A[6] = {0};
    while (1) {
        adc_select_input(0);

        A[0]=A[1];
        A[1]=A[2];
        A[2]=A[3];
        A[3]=A[4];
        A[4]=adc_read();

        str.axis=0;
        str.val=(((A[0]+A[1]+A[2]+A[3]+A[4])/5) - 2047)/8;
        if (abs(str.val)<=30) str.val=0;
        xQueueSend(xQueueAdc, &str, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void y_task(void *p) {
    adc_init();
    adc_gpio_init(27);
    
    adc_t str;
    int A[6] = {0};
    while (1) {
        adc_select_input(1);

        A[0]=A[1];
        A[1]=A[2];
        A[2]=A[3];
        A[3]=A[4];
        A[4]=adc_read();;

        str.axis=1;
        str.val=(((A[0]+A[1]+A[2]+A[3]+A[4])/5) - 2047)/8;
        if (abs(str.val)<=30) str.val=0;
        xQueueSend(xQueueAdc, &str, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void write_package(adc_t data) {
    int val = data.val;
    int msb = val >> 8;
    int lsb = val & 0xFF ;

    uart_putc_raw(uart0, data.axis);
    uart_putc_raw(uart0, lsb);
    uart_putc_raw(uart0, msb);
    uart_putc_raw(uart0, -1);
}

void uart_task(void *p) {
    adc_t data;

    while (1) {
        if (xQueueReceive(xQueueAdc, &data, 1)) {
            write_package(data); 
            vTaskDelay(pdMS_TO_TICKS(1));
        } else ;
        
    }
}

int main() {
    stdio_init_all();

    xQueueAdc = xQueueCreate(32, sizeof(adc_t));

    xTaskCreate(uart_task, "uart_task", 4096, NULL, 1, NULL);
    xTaskCreate(x_task, "X_task", 4095, NULL, 1, NULL);
    xTaskCreate(y_task, "Y_task", 4095, NULL, 1, NULL);


    vTaskStartScheduler();

    while (true)
        ;
}
