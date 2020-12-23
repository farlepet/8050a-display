#include <string.h>

#include <gpiohs.h>

#include <Fluke8050A.hpp>

Fluke8050A::Fluke8050A(fluke_8050a_pins_t *pins) {
    memcpy(&this->pins, pins, sizeof(fluke_8050a_pins_t));

    gpiohs_set_drive_mode(this->pins.bp,  GPIO_DM_INPUT);
    gpiohs_set_drive_mode(this->pins.dp,  GPIO_DM_INPUT);
    gpiohs_set_drive_mode(this->pins.hv,  GPIO_DM_INPUT);
    gpiohs_set_drive_mode(this->pins.w,   GPIO_DM_INPUT);
    gpiohs_set_drive_mode(this->pins.x,   GPIO_DM_INPUT);
    gpiohs_set_drive_mode(this->pins.y,   GPIO_DM_INPUT);
    gpiohs_set_drive_mode(this->pins.z,   GPIO_DM_INPUT);
    gpiohs_set_drive_mode(this->pins.st0, GPIO_DM_INPUT);
    gpiohs_set_drive_mode(this->pins.st1, GPIO_DM_INPUT);
    gpiohs_set_drive_mode(this->pins.st2, GPIO_DM_INPUT);
    gpiohs_set_drive_mode(this->pins.st3, GPIO_DM_INPUT);
    gpiohs_set_drive_mode(this->pins.st4, GPIO_DM_INPUT);

    /* Need to check if this is the correct edge */
    gpiohs_set_pin_edge(this->pins.bp, GPIO_PE_FALLING);
    gpiohs_irq_register(this->pins.bp, 3, (plic_irq_callback_t)&Fluke8050A::bpInterrupt, this);
}

int Fluke8050A::convert(void) {
    int16_t val = (this->status & FLUKE8050A_STATUS_ONE) ? 1 : 0;
    
    /* TODO: Handle decimal point */

    for(int i = 0; i < 4; i++) {
        if(this->bcd[i] > 9) {
            /* BCD value out of range */
            return -1;
        }

        val *= 10;
        val += bcd[0];
    }

    if(this->status & FLUKE8050A_STATUS_NEG) {
        val = -val;
    }
    this->value = val;

    return 0;
}

int Fluke8050A::bpInterrupt(void) {
    /* TODO: Potentially optimize this, the gpiohs_get_pin call goes four
     * functions deep to get the value. May-or-may-not be optimized out though. */
    if(gpiohs_get_pin(this->pins.st0)) {
        /* Status indicators */
        if(gpiohs_get_pin(this->pins.hv)) {
            this->status |=  FLUKE8050A_STATUS_HV;
        } else {
            this->status &= ~FLUKE8050A_STATUS_HV;
        }
        
        if(gpiohs_get_pin(this->pins.dp)) {
            this->status |=  FLUKE8050A_STATUS_REL;
        } else {
            this->status &= ~FLUKE8050A_STATUS_REL;
        }
        
        if(gpiohs_get_pin(this->pins.w)) {
            this->status |=  FLUKE8050A_STATUS_ONE;
        } else {
            this->status &= ~FLUKE8050A_STATUS_ONE;
        }
        
        if(gpiohs_get_pin(this->pins.x)) {
            this->status |=  FLUKE8050A_STATUS_POS;
        } else {
            this->status &= ~FLUKE8050A_STATUS_POS;
        }
        
        if(gpiohs_get_pin(this->pins.y)) {
            this->status |=  FLUKE8050A_STATUS_DB;
        } else {
            this->status &= ~FLUKE8050A_STATUS_DB;
        }
        
        if(gpiohs_get_pin(this->pins.z)) {
            this->status |=  FLUKE8050A_STATUS_NEG;
        } else {
            this->status &= ~FLUKE8050A_STATUS_NEG;
        }
    } else {
        /* Digit */
        uint8_t pos = 0xFF;

        if(gpiohs_get_pin(this->pins.st1)) {
            /* Thousands place */
            pos = 0;
        } else if(gpiohs_get_pin(this->pins.st2)) {
            /* Hundreds place */
            pos = 1;
        } else if(gpiohs_get_pin(this->pins.st3)) {
            /* Tens place */
            pos = 2;
        } else if(gpiohs_get_pin(this->pins.st4)) {
            /* Ones place */
            pos = 3;
        }
        
        this->bcd[pos] =  gpiohs_get_pin(this->pins.w)       |
                         (gpiohs_get_pin(this->pins.x) << 1) |
                         (gpiohs_get_pin(this->pins.y) << 2) |
                         (gpiohs_get_pin(this->pins.z) << 3);
        
        if(gpiohs_get_pin(this->pins.dp)) {
            this->decimal = pos;
        }

        if(pos == 4) {
            /* We should have all the digits at this point, assuming strobe
             * goes from 0->4. We can now convert to get the output. */
            this->convert();
            /* TODO: Make sure we only do this once every time the number updates */
        }
    }

    return 0;
}
