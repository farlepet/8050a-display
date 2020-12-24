#include <string.h>
#include <stdio.h>
#include <cmath>

#include <gpiohs.h>

#include <Fluke8050A.hpp>

Fluke8050A::Fluke8050A(fluke_8050a_pins_t *pins) {
    memcpy(&this->pins, pins, sizeof(fluke_8050a_pins_t));
}

void Fluke8050A::init(void) {
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
    gpiohs_set_pin_edge(this->pins.st0, GPIO_PE_RISING);
    gpiohs_set_pin_edge(this->pins.st1, GPIO_PE_RISING);
    gpiohs_set_pin_edge(this->pins.st2, GPIO_PE_RISING);
    gpiohs_set_pin_edge(this->pins.st3, GPIO_PE_RISING);
    gpiohs_set_pin_edge(this->pins.st4, GPIO_PE_RISING);
    gpiohs_irq_register(this->pins.st0, 3, (plic_irq_callback_t)&Fluke8050A::st0Interrupt, this);
    gpiohs_irq_register(this->pins.st1, 3, (plic_irq_callback_t)&Fluke8050A::st1Interrupt, this);
    gpiohs_irq_register(this->pins.st2, 3, (plic_irq_callback_t)&Fluke8050A::st1Interrupt, this);
    gpiohs_irq_register(this->pins.st3, 3, (plic_irq_callback_t)&Fluke8050A::st1Interrupt, this);
    gpiohs_irq_register(this->pins.st4, 3, (plic_irq_callback_t)&Fluke8050A::st1Interrupt, this);
}

float Fluke8050A::getValue(void) {
    return this->value;
}

float Fluke8050A::getRelative(void) {
    if(this->status & FLUKE8050A_STATUS_REL) {
        return this->relative;
    } else {
        return NAN;
    }
}

void Fluke8050A::debug(void) {
    printf("Fluke8050A::debug [%hhu,%hhu,%hhu,%hhu,%02hhX]: %+5.02f\r\n",
           this->bcd[3], this->bcd[2],
           this->bcd[1], this->bcd[0],
           this->status,
           this->value);
    if(this->status & FLUKE8050A_STATUS_REL) {
        printf("                           Rel: %+5.02f\r\n", this->relative);
    }
}

int Fluke8050A::convert(void) {
   
    int16_t val = (this->status & FLUKE8050A_STATUS_ONE) ? 1 : 0;
    
    for(int i = 3; i >= 0; i--) {
        if(this->bcd[i] > 9) {
            /* BCD value out of range */
            return -1;
        }

        val *= 10;
        val += bcd[i];
    }

    if((this->status & FLUKE8050A_STATUS_NEG) &&
      !(this->status & FLUKE8050A_STATUS_POS)) {
        /* POS line is actually two dots above and below the negative sign,
         * thus NEG+POS lines will create a positive sign. */
        val = -val;
    }
    
    float div = 1;
    for(uint8_t i = 3; i > this->decimal; i--) {
        div *= 10;
    }

    this->value = (float)val / div;

    return 0;
}

int Fluke8050A::st0Interrupt(void) {
    /* TODO: Potentially optimize this, the gpiohs_get_pin call goes four
     * functions deep to get the value. May-or-may-not be optimized out though. */
    
    /* Status indicators */
    if(gpiohs_get_pin(this->pins.hv)) {
        this->status |=  FLUKE8050A_STATUS_HV;
    } else {
        this->status &= ~FLUKE8050A_STATUS_HV;
    }
    
    if(gpiohs_get_pin(this->pins.dp)) {
        if(!(this->status & FLUKE8050A_STATUS_REL)) {
            this->relative = this->value;
        }
        this->status |=  FLUKE8050A_STATUS_REL;
    } else {
        this->relative = 0;
        this->status &= ~FLUKE8050A_STATUS_REL;
    }


    if(gpiohs_get_pin(this->pins.w)) {
        this->status |=  FLUKE8050A_STATUS_NEG;
    } else {
        this->status &= ~FLUKE8050A_STATUS_NEG;
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
        this->status |=  FLUKE8050A_STATUS_ONE;
    } else {
        this->status &= ~FLUKE8050A_STATUS_ONE;
    }

    return 0;
}

int Fluke8050A::st1Interrupt(void) {
    /* NOTE: This could be duplicated into multiple callbacks to improve
     * effeciency, at the added cost of code duplication. We are running fast
     * enough that this isn't a real concern, however. */
    
    /* Digit */
    uint8_t pos = 0xFF;

    if(gpiohs_get_pin(this->pins.st4)) {
        /* Thousands place */
        pos = 0;
    } else if(gpiohs_get_pin(this->pins.st3)) {
        /* Hundreds place */
        pos = 1;
    } else if(gpiohs_get_pin(this->pins.st2)) {
        /* Tens place */
        pos = 2;
    } else if(gpiohs_get_pin(this->pins.st1)) {
        /* Ones place */
        pos = 3;
    }

    if(pos == 0xFF) {
        return 0;
    }
    
    this->bcd[pos] =  gpiohs_get_pin(this->pins.z)       |
                     (gpiohs_get_pin(this->pins.y) << 1) |
                     (gpiohs_get_pin(this->pins.x) << 2) |
                     (gpiohs_get_pin(this->pins.w) << 3);
    
    if(gpiohs_get_pin(this->pins.dp)) {
        this->decimal = pos;
    }

    if(pos == 3) {
        /* We should have all the digits at this point, we can now convert
            * to get the value. */
        this->convert();
    }

    return 0;
}
