#ifndef NT35310_HPP
#define NT35310_HPP

#include <spi.h>

#define NT35310_18BIT_COLOR 0

#if (NT35310_18BIT_COLOR)
#define RGB(R, G, B) ((((uint32_t)(R) & 0xFC) << 16) | \
                      (((uint32_t)(G) & 0xFC) << 8)  | \
                      (((uint32_t)(B) & 0xFC)))
#else
#define RGB(R, G, B) ((((uint16_t)(R) & 0xF8) << 8) | \
                      (((uint16_t)(G) & 0xFC) << 3) | \
                      (((uint16_t)(B) & 0xF8) >> 3))
#endif

typedef enum {
    NT35310_CMD_NOP                    = 0x00,
    NT35310_CMD_SOFT_RESET             = 0x01, /* Labeled "SOFT_REST" in datasheet */

    NT35310_CMD_ENTER_SLEEP_MODE       = 0x10,
    NT35310_CMD_EXIT_SLEEP_MODE        = 0x11,
    NT35310_CMD_ENTER_PARTIAL_MODE     = 0x12,
    NT35310_CMD_ENTER_NORMAL_MODE      = 0x13,
    
    NT35310_CMD_EXIT_INVERT_MODE       = 0x20,
    NT35310_CMD_ENTER_INVERT_MODE      = 0x21,
    NT35310_CMD_ALLPOFF                = 0x22,
    NT35310_CMD_ALLPON                 = 0x23,
    
    NT35310_CMD_SET_DISPLAY_OFF        = 0x28,
    NT35310_CMD_SET_DISPLAY_ON         = 0x29,
    
    NT35310_CMD_SET_HORIZONTAL_ADDRESS = 0x2A,
    NT35310_CMD_SET_VERTICAL_ADDRESS   = 0x2B,
    NT35310_CMD_WRITE_MEMORY_START     = 0x2C,

    NT35310_CMD_SET_ADDRESS_MODE       = 0x36,

    NT35310_CMD_SET_PIXEL_FORMAT       = 0x3A,

    NT35310_CMD_WRDISBV                = 0x51, /* Write display brightness */
    NT35310_CMD_WRCTRLD                = 0x52, /* Write display CTRL */
} nt35310_command_e;

class NT35310 {
private:
    spi_device_num_t  spiDev; /*!< SPI device number the LCD is attached to. */
    spi_chip_select_t spiCS;  /*!< Chip Select line to use for SPI interface. */

    uint16_t          width;  /*!< Width of LCD in pixels. */
    uint16_t          height; /*!< Height of LCD in pixels. */

    uint8_t           RSTNum; /*!< GPIOHS number for Reset pin */
    uint8_t           DCNum;  /*!< GPIOHS number for Data Clock pin */

    void reset(void);

    void command(nt35310_command_e cmd);

    void write8(const uint8_t   *data, size_t len);
    void write16(const uint16_t *data, size_t len);
    void write24(const uint32_t *data, size_t len);
    void write32(const uint32_t *data, size_t len);

    void fillDMA(uint32_t data, uint8_t bits, size_t len);
    
public:
    /**
     * Constructor
     * 
     * @param spiDev SPI peripheral the LCD is attached to
     * @param width  Width of LCD, in pixels
     * @param height Height of LCD, in pixels
     */
    NT35310(spi_device_num_t spiDev, spi_chip_select_t spiCS, uint8_t RSTNum, uint8_t DCNum, uint16_t width, uint16_t height);

    void init(void);

    void setArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    void fillArea(uint32_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    void fill(uint32_t color);

    /**
     * Convert 24-bit raw RGB data to a format ready to be directly written
     * to the LCD.
     * 
     * @param dest Destination buffer
     * @param src  Source buffer
     * @param len  Number of pixels in image data
     */
    static void RGB2Buffer(void *dest, const void *src, size_t len);

    void writeBuffer(const void *buff, uint16_t width, uint16_t height, uint16_t x, uint16_t y);

    void setBrightness(uint8_t val);
};

#endif