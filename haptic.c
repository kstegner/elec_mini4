#include <p24FJ128GB206.h>
#include "config.h"
#include "common.h"
#include "usb.h"
#include "pin.h"
#include "uart.h"
#include "timer.h"
#include "oc.h"
#include "ui.h"
#include <stdio.h>

#define HELLO       0   // Vendor request that prints "Hello World!"
#define SET_VALS    1   // Vendor request that receives 2 unsigned integer values
#define GET_VALS    2   // Vendor request that returns 2 unsigned integer values
#define PRINT_VALS  3   // Vendor request that prints 2 unsigned integer values 

uint16_t val1, val2;

void VendorRequests(void) {
    WORD temp;

    switch (USB_setup.bRequest) {
        case HELLO:
            printf("Hello World!\n");
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case SET_VALS:
            val1 = USB_setup.wValue.w;
            val2 = USB_setup.wIndex.w;
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case GET_VALS:
            temp.w = val1;
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            temp.w = val2;
            BD[EP0IN].address[2] = temp.b[0];
            BD[EP0IN].address[3] = temp.b[1];
            BD[EP0IN].bytecount = 4;    // set EP0 IN byte count to 4
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;            
        case PRINT_VALS:
            printf("val1 = %u, val2 = %u\n", val1, val2);
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        default:
            USB_error_flags |= 0x01;    // set Request Error Flag
    }
}

void VendorRequestsIn(void) {
    switch (USB_request.setup.bRequest) {
        default:
            USB_error_flags |= 0x01;                    // set Request Error Flag
    }
}

void VendorRequestsOut(void) {
    switch (USB_request.setup.bRequest) {
        default:
            USB_error_flags |= 0x01;                    // set Request Error Flag
    }
}

int16_t main(void) {
    init_clock();
    init_uart();
    init_timer();
    init_ui();
    init_pin();
    init_oc();

     //starting conditions
    led_on(&led1);
    timer_setPeriod(&timer2, 0.5); //start internal clock with defined period
    timer_start(&timer2);

    val1 = 0;
    val2 = 0;
    pin_digitalOut(&D[2]); //define servo output pins
    pin_digitalOut(&D[3]);
    oc_servo(&oc1, &(D[2]), &timer1, 20e-3f, 500e-6f, 2200e-6f, 0); 
    oc_servo(&oc2, &(D[3]), &timer3, 20e-3f, 500e-6f, 2200e-6f, 0); /*call the oc_servo function as outlined in the course documentation packet
            (specify which OC peripheral, use a timer, define the period of the control signal, define the min and max values of the PWM, initial position)
            Values for the PWM signal were taken from servo spec sheet*/


    InitUSB();                              // initialize the USB registers and serial interface engine
    while (USB_USWSTAT!=CONFIG_STATE) {     // while the peripheral is not configured...
        ServiceUSB();                       // ...service USB requests
    }

    while (1) {
        ServiceUSB();                       // service any pending USB requests
        
        if (timer_flag(&timer2)) { //starts timer
            timer_lower(&timer2);
            led_toggle(&led1); //toggles LED to confirm code has been written and is running
        }
    pin_write(&D[2], val1); //writes val1 to servo 1
    pin_write(&D[3], val2);	//writes val2 to servo 2
    }
}