#ifndef FLUKE8050A_HPP
#define FLUKE8080A_HPP

#include <stdint.h>

typedef struct {
    uint8_t bp;   /*!< Display frequency/clock - Interrupt input */
    
    uint8_t dp;   /*!< Decimal point / relative */
    uint8_t hv;   /*!< High Voltage */

    uint8_t w;    /*!< BCD 0 / 1*/
    uint8_t x;    /*!< BCD 1 / + */
    uint8_t y;    /*!< BCD 2 / dB*/
    uint8_t z;    /*!< BCD 3 / - */
    
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
    fluke_8050a_pins_t pins;    /*!< GPIOHS pins */

    uint8_t            bcd[4];  /*!< BCD value of display. Left-most ones digit in status. */
    uint8_t            status;  /*!< Status bits, see fluke_8050a_statue_e. */
    uint8_t            decimal; /*!< Position of decimal point, 0xFF in non-existant */

    int16_t            value;   /*!< Last displayed numberical value */

    /**
     * Convert received and stored data into numerical value.
     * 
     * @return 0 on success, else non-zero
     */
    int convert(void);

    int bpInterrupt(void);

public:
    Fluke8050A(fluke_8050a_pins_t *pins);
};

#endif
