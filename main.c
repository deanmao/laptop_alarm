#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/eeprom.h>

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */

#define DISABLE_ALARM       4
#define ENABLE_ALARM        5

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
  usbRequest_t    *rq = (void *)data;

    if (rq->bRequest == ENABLE_ALARM){
        eeprom_write_byte(10, 1);
        return 0;
    } else if(rq->bRequest == DISABLE_ALARM){
        eeprom_write_byte(10, 0);
        return 0;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

int __attribute__((noreturn)) main(void)
{
uchar   i;

    wdt_enable(WDTO_1S);
    /* Even if you don't use the watchdog, turn it off here. On newer devices,
     * the status of the watchdog (on/off, period) is PRESERVED OVER RESET!
     */
    /* RESET status: all port bits are inputs without pull-up.
     * That's the way we need D+ and D-. Therefore we don't need any
     * additional hardware initialization.
     */
    odDebugInit();
    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();
    DDRB |= _BV(PB0);
    for(;;){                /* main event loop */
        wdt_reset();
        usbPoll();
        if (eeprom_read_byte(10) == 1) {
          PORTB |= _BV(PB0);
        } else {
          PORTB &= ~_BV(PB0);
        }
    }
}

/* ------------------------------------------------------------------------- */
