#ifndef FLUKE8050A_HPP
#define FLUKE8080A_HPP

#include <stdint.h>

typedef struct {
    uint8_t dp;   /*!< Decimal point / relative */
    uint8_t hv;   /*!< High Voltage */

    uint8_t w;    /*!< BCD 0 / -  */
    uint8_t x;    /*!< BCD 1 / +  */
    uint8_t y;    /*!< BCD 2 / dB */
    uint8_t z;    /*!< BCD 3 / 1  */
    
    uint8_t st0;  /*!< Strobe 0 */
    uint8_t st1;  /*!< Strobe 1 */
    uint8_t st2;  /*!< Strobe 2 */
    uint8_t st3;  /*!< Strobe 3 */
    uint8_t st4;  /*!< Strobe 4 */
} fluke_8050a_pins_t;

typedef enum {
    FLUKE8050A_STATUS_ONE = (1U << 0), /* Extra 1 */
    FLUKE8050A_STATUS_NEG = (1U << 1), /* Negative sign */
    FLUKE8050A_STATUS_POS = (1U << 2), /* Positive sign */
    FLUKE8050A_STATUS_DB  = (1U << 3), /* Decibels */
    FLUKE8050A_STATUS_REL = (1U << 4), /* Relative */
    FLUKE8050A_STATUS_BT  = (1U << 5), /* Battery */
    FLUKE8050A_STATUS_HV  = (1U << 6), /* High Voltage */
} fluke_8050a_status_e;

class Fluke8050A {
private:
    fluke_8050a_pins_t pins;        /*!< GPIOHS pins */

    uint8_t            bcd[4];     /*!< BCD value of display. Left-most ones digit in status. */
    uint8_t            status;     /*!< Status bits, see fluke_8050a_statue_e. */
    uint8_t            statusPend; /*!< Pending status, used to prevent short glitches from messing up stored relative value.  */
    uint8_t            decimal;    /*!< Position of decimal point, 0xFF in non-existant */

    float              value;      /*!< Last displayed numberical value */
    float              relaPend;   /*!< Pending relative value */
    float              relative;   /*!< Last recorded value when relative mode was enabled */

    /**
     * Convert received and stored data into numerical value.
     * 
     * @return 0 on success, else non-zero
     */
    int convert(void);

    /**
     * Strobe 0 interrupt handler.
     * 
     * Called when strobe 0 line goes high. Reads and stores status information.
     */
    int st0Interrupt(void);

    /**
     * Strobe 1-4 interrupt handler
     * 
     * Called when strobe 1-4 lines go high. Stores current digit, and updates
     * decimal point position if applicable.
     */
    int st1Interrupt(void);

    /**
     * Set status bit, taking pending status into account.
     * 
     * @param mask Mask with bit to set
     */
    void statusSet(uint8_t mask);
    
    /**
     * Clear status bit, taking pending status into account.
     * 
     * @param mask Mask with bit to clear
     */
    void statusClear(uint8_t mask);

public:
    /**
     * Constructor
     * 
     * @param pins Set of pins to use to communicate with the 8050A */
    Fluke8050A(fluke_8050a_pins_t *pins);

    /**
     * Initialize GPIO used to receive data from the 8050A
     */
    void init(void);

    /**
     * Get the last seen value
     * 
     * @return Last seen value
     */
    float getValue(void);
    
    /**
     * Get the current relative value, if applicable.
     * 
     * @return NAN if not in relative mode, else value seen when relative mode
     *         was first detected.
     */
    float getRelative(void);

    /**
     * Print some internal state to the serial port for debugging purposes.
     */
    void debug(void);
};

#endif
