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
#define GET_PING    4   // Vendor request that prints 1 unsigned integer value

//DEFINE TIMERS TO BE USED
#define LED_TIMER   &timer1 
#define PAN_TIMER   &timer2
#define TILT_TIMER  &timer3
#define PING_TIMER  &timer4
#define ECHO_TIMER  &timer5

//DEFINE PIN NAMES TO BE USED
#define PAN_PIN     &D[2]  
#define TILT_PIN    &D[3]
#define PING_PIN    &D[4]
#define ECHO_PIN    &D[13]

//DEFINE VARIABLES AND INITIAL VALUES
void initChip(void);
uint16_t PAN_VAL  = 0;
uint16_t TILT_VAL = 0;

uint16_t BUFFER = 0;
uint16_t OVERFLOW_COUNT = 0;

uint16_t TOF = 0;

uint16_t TIMEOUT_FLAG = 0;

//OTHER INITIAL CONDITIONS
void initChip(){
    init_clock();
    init_uart();
    init_timer();
    init_ui();
    init_pin();
    init_oc();

    pin_digitalOut(PAN_PIN);
    pin_digitalOut(TILT_PIN);
    pin_digitalOut(PING_PIN);
    pin_digitalIn(ECHO_PIN);

    oc_servo(&oc1, PAN_PIN, PAN_TIMER, 20e-3f, 600e-6f, 2400e-6f, 0);   //PAN servo controller
    oc_servo(&oc2, TILT_PIN, TILT_TIMER, 20e-3f, 600e-6f, 2400e-6f, 0); //TILT servo controller
    oc_pwm(&oc3, PING_PIN, NULL, 40000, 0);                             //PING signal generator
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
            PAN_VAL = USB_setup.wValue.w;
            TILT_VAL = USB_setup.wIndex.w;
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case GET_VALS:
            temp.w = PAN_VAL;
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            temp.w = TILT_VAL;
            BD[EP0IN].address[2] = temp.b[0];
            BD[EP0IN].address[3] = temp.b[1];
            BD[EP0IN].bytecount = 4;    // set EP0 IN byte count to 4
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;            
        case PRINT_VALS:
            printf("PAN_VAL = %u, TILT_VAL = %u\n", PAN_VAL, TILT_VAL);
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        //NEW CASE TO TRANSFER TIMER_READ VALUES TO PYTHON INTERFACE    
        case GET_PING:
            pin_write(PING_PIN, 65536/2);       // send the ping signal
            timer_start(PING_TIMER);            //start timer for PING and ECHO transducer
            timer_start(ECHO_TIMER);
            while(!timer_flag(PING_TIMER)) {}   // do nothing until PING timer starts

            timer_lower(PING_TIMER);            //set PING pin to 0 when PING timer finishes
            pin_write(PING_PIN, 0);             //period set below

            //Thank you to Asa Eckert-Erdheim for assistance in developing the following section of code

            BUFFER = timer_read(ECHO_TIMER);                //buffer prevents receiving pin from reading
            while(timer_read(ECHO_TIMER) < (4 * BUFFER)) {} //ping signal immediately after sending

            while(!pin_read(ECHO_PIN)) { // wait for ECHO pin to receive something
                if (OVERFLOW_COUNT >= 20) { //check for timeout
                    TIMEOUT_FLAG = 1;
                    TOF = 2;
                    break;
                }
                if (timer_flag(ECHO_TIMER)) {
                    OVERFLOW_COUNT ++;
                    timer_lower(ECHO_TIMER);
                }
            }

            if (TIMEOUT_FLAG == 1) {
                TOF = 1;
            }
            else {
                TOF = timer_read(ECHO_TIMER);
            }

            timer_stop(ECHO_TIMER);

            temp.w = TOF;
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];

            temp.w = OVERFLOW_COUNT;
            BD[EP0IN].address[2] = temp.b[0];
            BD[EP0IN].address[3] = temp.b[1];

            BD[EP0IN].bytecount = 4;    // set EP0 IN byte count to 4
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit

            TOF = 0;    //reset values to 0 for next GET_PING call
            TIMEOUT_FLAG = 0;
            OVERFLOW_COUNT = 0; 

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
    timer_setPeriod(LED_TIMER, 0.5);         //start internal clock with defined period
    timer_start(LED_TIMER);

    timer_setPeriod(PING_TIMER, 5e-4);


    while (USB_USWSTAT!=CONFIG_STATE) {     // while the peripheral is not configured...
        ServiceUSB();                       // ...service USB requests
    }

    while (1) {
        ServiceUSB();                       // service any pending USB requests

        //LED1 TOGGLE TO CONFIRM RUNNING CODE
        if (timer_flag(LED_TIMER)) {
            timer_lower(LED_TIMER);
            led_toggle(&led1);
        }
        pin_write(PAN_PIN, PAN_VAL); //writes PAN_VAL to pan servo
        pin_write(TILT_PIN, TILT_VAL);	//writes TILT_VAL to tilt servo
        
    }
}