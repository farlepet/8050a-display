#include <gpiohs.h>
#include <fpioa.h>
#include <sleep.h>

#include <NT35310.hpp>

NT35310::NT35310(spi_device_num_t spiDev, spi_chip_select_t spiCS, uint8_t RSTNum, uint8_t DCNum, uint16_t width, uint16_t height) {
    this->spiDev = spiDev;
    this->spiCS  = spiCS;

    this->RSTNum = RSTNum;
    this->DCNum  = DCNum;

    this->width  = width;
    this->height = height;
}

void NT35310::init(void) {
    uint8_t data;
    
    gpiohs_set_drive_mode(this->RSTNum, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(this->DCNum,  GPIO_DM_OUTPUT);
    
    gpiohs_set_pin(this->DCNum,  GPIO_PV_HIGH);
    
    spi_init(this->spiDev, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
    spi_set_clk_rate(this->spiDev, 5000000);
    
    this->reset();

    this->command(NT35310_CMD_SOFT_RESET);
    msleep(150);
    this->command(NT35310_CMD_EXIT_SLEEP_MODE);
    /* Need to wait 5+ ms before sending next command */
    msleep(500);

    /* Pixel format: 16/18 bits-per-pixel */
#if (NT35310_18BIT_COLOR)
    data = 0x66;
#else
    data = 0x55;
#endif
    this->command(NT35310_CMD_SET_PIXEL_FORMAT);
    this->write8(&data, 1);

    /* Address mode: Auto-increment Y, decrement X, top-down, left-right */
    data = 0x40;
    this->command(NT35310_CMD_SET_ADDRESS_MODE);
    this->write8(&data, 1);
    
    this->command(NT35310_CMD_ENTER_NORMAL_MODE);
    this->command(NT35310_CMD_EXIT_INVERT_MODE);
    this->command(NT35310_CMD_SET_DISPLAY_ON);
}

void NT35310::reset(void) {
    gpiohs_set_pin(this->RSTNum, GPIO_PV_LOW);
    /* Documentation unclear, 1 ms min pulse? */
    msleep(2);
    gpiohs_set_pin(this->RSTNum, GPIO_PV_HIGH);
    /* Reset duration 20 ms */
    msleep(20);
}

void NT35310::setBrightness(uint8_t val) {
    uint8_t data = 0x20;
    this->command(NT35310_CMD_WRCTRLD);
    this->write8(&data, 1);
    
    this->command(NT35310_CMD_WRDISBV);
    this->write8(&val, 1);
}

void NT35310::setArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    uint8_t data[4];

    data[0] = (uint8_t)(x1 >> 8);
    data[1] = (uint8_t)x1;
    data[2] = (uint8_t)(x2 >> 8);
    data[3] = (uint8_t)x2;
    this->command(NT35310_CMD_SET_HORIZONTAL_ADDRESS);
    this->write8(data, 4);
    
    data[0] = (uint8_t)(y1 >> 8);
    data[1] = (uint8_t)y1;
    data[2] = (uint8_t)(y2 >> 8);
    data[3] = (uint8_t)y2;
    this->command(NT35310_CMD_SET_VERTICAL_ADDRESS);
    this->write8(data, 4);
    
    this->command(NT35310_CMD_WRITE_MEMORY_START);
}

void NT35310::fillArea(uint32_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    this->setArea(x1, y1, x2, y2);
#if (NT35310_18BIT_COLOR)
    this->fillDMA(color, 24, ((x2 + 1) - x1) * ((y2 + 1) - y1));
#else
    this->fillDMA(color, 16, ((x2 + 1) - x1) * ((y2 + 1) - y1));
#endif

}

void NT35310::fill(uint32_t color) {
    this->fillArea(color, 0, 0, this->width - 1, this->height - 1);
}

void NT35310::RGB2Buffer(void *dest, const void *src, size_t len) {
    for(size_t i = 0; i < len; i++) {
        uint8_t *rgb = (uint8_t *)(src + (i * 3));
#if (NT35310_18BIT_COLOR)
        uint8_t *d24 = ((uint8_t *)(dest + (i * 3)));
        d24[0] = rgb[0] & 0xFC;
        d24[1] = rgb[1] & 0xFC;
        d24[2] = rgb[2] & 0xFC;
#else
        ((uint16_t *)dest)[i] = RGB(rgb[0], rgb[1], rgb[2]);
#endif
    }
}

void NT35310::writeBuffer(const void *buff, uint16_t width, uint16_t height, uint16_t x, uint16_t y) {
    this->setArea(x, y, x + width - 1, y + width - 1);
#if (NT35310_18BIT_COLOR)
    this->write24((uint32_t *)buff, (width * height));
#else
    this->write16((uint16_t *)buff, (width * height));
#endif
}

void NT35310::command(nt35310_command_e cmd) {
    gpiohs_set_pin(this->DCNum, GPIO_PV_LOW);

    spi_init(this->spiDev, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
    spi_init_non_standard(this->spiDev, 8, 0, 0, SPI_AITM_AS_FRAME_FORMAT);
    /* TODO: Allow selection of DMA channel in constructor or otherwise. */
    spi_send_data_normal_dma(DMAC_CHANNEL0, this->spiDev, this->spiCS, &cmd, 1, SPI_TRANS_CHAR);
}

void NT35310::write8(const uint8_t *data, size_t len) {
    gpiohs_set_pin(this->DCNum, GPIO_PV_HIGH);

    spi_init(this->spiDev, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
    spi_init_non_standard(this->spiDev, 8, 0, 0, SPI_AITM_AS_FRAME_FORMAT);
    
    spi_send_data_normal_dma(DMAC_CHANNEL0, this->spiDev, this->spiCS, data, len, SPI_TRANS_CHAR);
}

void NT35310::write16(const uint16_t *data, size_t len) {
    gpiohs_set_pin(this->DCNum, GPIO_PV_HIGH);

    spi_init(this->spiDev, SPI_WORK_MODE_0, SPI_FF_OCTAL, 16, 0);
    spi_init_non_standard(this->spiDev, 16, 0, 0, SPI_AITM_AS_FRAME_FORMAT);

    spi_send_data_normal_dma(DMAC_CHANNEL0, this->spiDev, this->spiCS, data, len, SPI_TRANS_SHORT);
}

void NT35310::write24(const uint32_t *data, size_t len) {
    gpiohs_set_pin(this->DCNum, GPIO_PV_HIGH);

    spi_init(this->spiDev, SPI_WORK_MODE_0, SPI_FF_OCTAL, 24, 0);
    spi_init_non_standard(this->spiDev, 0, 24, 0, SPI_AITM_AS_FRAME_FORMAT);

    spi_send_data_normal_dma(DMAC_CHANNEL0, this->spiDev, this->spiCS, data, len, SPI_TRANS_SHORT);
}

void NT35310::write32(const uint32_t *data, size_t len) {
    gpiohs_set_pin(this->DCNum, GPIO_PV_HIGH);

    spi_init(this->spiDev, SPI_WORK_MODE_0, SPI_FF_OCTAL, 32, 0);
    spi_init_non_standard(this->spiDev, 0, 32, 0, SPI_AITM_AS_FRAME_FORMAT);

    spi_send_data_normal_dma(DMAC_CHANNEL0, this->spiDev, this->spiCS, data, len, SPI_TRANS_INT);
}

void NT35310::fillDMA(uint32_t data, uint8_t bits, size_t len) {
    gpiohs_set_pin(this->DCNum, GPIO_PV_HIGH);

    spi_init(this->spiDev, SPI_WORK_MODE_0, SPI_FF_OCTAL, bits, 0);
    if(bits < 24) {
        spi_init_non_standard(this->spiDev, bits, 0, 0, SPI_AITM_AS_FRAME_FORMAT);
    } else {
        spi_init_non_standard(this->spiDev, 0, bits, 0, SPI_AITM_AS_FRAME_FORMAT);
    }

    spi_fill_data_dma(DMAC_CHANNEL0, this->spiDev, this->spiCS, &data, len);
}
