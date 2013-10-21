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

//DEFINE VENDOR REQUESTS
#define HELLO       0   // Vendor request that prints "Hello World!"
#define SET_VALS    1   // Vendor request that receives 2 unsigned integer values
#define GET_VALS    2   // Vendor request that returns 2 unsigned integer values
#define PRINT_VALS  3   // Vendor request that prints 2 unsigned integer values

//DEFINE TIMERS TO BE USED
#define LED_TIMER   &timer1
#define PWM_TIMER   &timer2 

//DEFINE PIN NAMES TO BE USED
#define OUT1    &A[0]
#define OUT2    &A[1]
#define FB      &A[2]

#define ENC     &D[0]
#define SF      &D[1] 
#define D2      &D[2] 
#define D1      &D[3] 
#define ENA     &D[4] 
#define IN2     &D[5] 
#define IN1     &D[6] 
#define SLEW    &D[7] 
#define INV     &D[8] 

//DEFINE VARIABLES AND INITIAL VALUES
void initChip(void);
void initInt(void);
void __attribute__((interrupt)) _CNInterrupt(void);

uint16_t VAL1 = 1;
uint16_t VAL2 = 0;

//OTHER INITIAL CONDITIONS
void initChip(){
    init_clock();
    init_uart();
    init_timer();
    init_ui();
    init_pin();
    init_oc();

    pin_analogIn(OUT1);
    pin_analogIn(OUT2);
    pin_analogIn(FB);

    pin_digitalOut(ENC);
    pin_digitalOut(D1);
    pin_digitalOut(D2);
    pin_digitalOut(IN1);
    pin_digitalOut(IN2);
    pin_digitalOut(ENA);
    pin_digitalOut(SLEW);
    pin_digitalOut(INV);

    pin_digitalIn(SF);
    pin_digitalIn(ENC);
}


void VendorRequests(void) {
    WORD temp;

    switch (USB_setup.bRequest) {
        case HELLO:
            printf("Hello World!\n");
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case SET_VALS:
            VAL1 = USB_setup.wValue.w;
            VAL2 = USB_setup.wIndex.w;
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case GET_VALS:
            temp.w = VAL1;
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            temp.w = VAL2;
            BD[EP0IN].address[2] = temp.b[0];
            BD[EP0IN].address[3] = temp.b[1];
            BD[EP0IN].bytecount = 4;    // set EP0 IN byte count to 4
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;            
        case PRINT_VALS:
            printf("VAL1 = %u, VAL2 = %u\n", VAL1, VAL2);
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        default:
            USB_error_flags |= 0x01;
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
     InitUSB();                              // initialize the USB registers and serial interface engine
     initChip();   

    led_on(&led1);

    pin_write(ENA, 1);  //sets ENABLE to HIGH
    pin_write(IN1, 1);  
    pin_write(IN2, 0);
    pin_write(SLEW, 0);
    pin_write(INV, 0);
    pin_write(D1, 0);
    pin_write(D2, 1);

    timer_setPeriod(LED_TIMER, 0.5);         //start internal clock with defined period
    timer_start(LED_TIMER);

    // oc_pwm(&oc1, D2, PWM_TIMER, 100, 0);

    while (USB_USWSTAT!=CONFIG_STATE) {     // while the peripheral is not configured...
        ServiceUSB();                       // ...service USB requests
    }

    while (1) {
        ServiceUSB();                       // service any pending USB requests

        //LED1 TOGGLE TO CONFIRM RUNNING CODE
        if (timer_flag(LED_TIMER)) {
            timer_lower(LED_TIMER);
            led_toggle(&led1);

            pin_write(INV, led_read(&led1));
        }
        pin_write(D2, VAL1);        
    }
}