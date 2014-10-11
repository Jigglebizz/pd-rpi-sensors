// rpi_sensor.c
// Created by Michael Hawthorne (Jigglebizz) 2014
// https://github.com/Jigglebizz/pd-rpi-sensors.git

#include <stdlib.h>
#include <stdio.h>
#include <bcm2835.h>
#include "m_pd.h"

static t_class *rpi_sensor_class;

typedef struct _rpi_sensor {
    t_object x_obj;
    t_int* i_mux0sel;
    t_int* i_mux1sel;
    t_outlet *l_out;
} t_rpi_sensor;

void *rpi_sensor_new(void) {
    t_rpi_sensor *x = (t_rpi_sensor *)pd_new(rpi_sensor_class);
    
    // Set our outlet
    x->l_out = outlet_new(&x->x_obj, &s_list);

    if (!bcm2835_init()) {
        error("Failure initializing gpio!");
        return (void *)x;
    }

    // set up SPI for ADC
    bcm2835_spi_begin();
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, 0);
    // This is a relatively slow speed. I think it can go faster.
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);

    // Set up our mux1 selection pins
    x->i_mux0sel = (t_int*)malloc(4 * sizeof(t_int));
    x->i_mux0sel[0] = RPI_V2_GPIO_P1_03;
    x->i_mux0sel[1] = RPI_V2_GPIO_P1_05;
    x->i_mux0sel[2] = RPI_V2_GPIO_P1_07;
    x->i_mux0sel[3] = RPI_V2_GPIO_P1_08;

    bcm2835_gpio_fsel( x->i_mux0sel[0], BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel( x->i_mux0sel[1], BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel( x->i_mux0sel[2], BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel( x->i_mux0sel[3], BCM2835_GPIO_FSEL_OUTP);
    
    // Set up our mux2 selection pins
    x->i_mux1sel = (t_int*)malloc(4 * sizeof(t_int));
    x->i_mux1sel[0] = RPI_V2_GPIO_P1_10;
    x->i_mux1sel[1] = RPI_V2_GPIO_P1_11;
    x->i_mux1sel[2] = RPI_V2_GPIO_P1_12;
    x->i_mux1sel[3] = RPI_V2_GPIO_P1_13;

    bcm2835_gpio_fsel( x->i_mux1sel[0], BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel( x->i_mux1sel[1], BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel( x->i_mux1sel[2], BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel( x->i_mux1sel[3], BCM2835_GPIO_FSEL_OUTP);
    
    return(void *)x;
}

void rpi_sensor_free(t_rpi_sensor *x) {
    // stop SPI
    bcm2835_spi_end();
    
    bcm2835_close();

    // free mux data
    free(x->i_mux0sel);
    free(x->i_mux1sel);
}

void rpi_set_mux(t_rpi_sensor *x, uint8_t mux, uint8_t channel) {
    if (mux == 0) {
        bcm2835_gpio_write(x->i_mux0sel[0],  channel       & 0x1);
        bcm2835_gpio_write(x->i_mux0sel[1], (channel >> 1) & 0x1);
        bcm2835_gpio_write(x->i_mux0sel[2], (channel >> 2) & 0x1);
        bcm2835_gpio_write(x->i_mux0sel[3], (channel >> 3) & 0x1);
    } else {
        bcm2835_gpio_write(x->i_mux1sel[0],  channel       & 0x1);
        bcm2835_gpio_write(x->i_mux1sel[1], (channel >> 1) & 0x1);
        bcm2835_gpio_write(x->i_mux1sel[2], (channel >> 2) & 0x1);
        bcm2835_gpio_write(x->i_mux1sel[3], (channel >> 3) & 0x1);
    }
}

t_int rpi_read_adc(uint8_t channel) {
    // We need to transfer 3 bytes per transaction
    // according to the MCP3202 datasheet found at
    // http://ww1.microchip.com/downloads/en/DeviceDoc/21034D.pdf
    char data_buffer[3];
    
    data_buffer[0] = 0x1;  // start bit
    data_buffer[1] = 0xA0; // Single channel select and MSBF
    data_buffer[2] = 0x0;  // Don't care - used for return data
    
    // Select channel
    data_buffer[1] |= (channel & 1) << 6;
    
    // do the transfer
    bcm2835_spi_transfern(&data_buffer[0], 3);
    
    // Error checking
    if (data_buffer[1] & 0x10) {
        error("ADC read error");
        return -1;
    }
    
    // put our results together
    int value = (data_buffer[1] & 0xF) << 8;
    value |= data_buffer[2];
    
    return value;
}


void rpi_sensor_bang(t_rpi_sensor *x) {
    t_atom muxvalues[32];
    uint8_t i;
    for (i = 0; i < 16; i++) {
        rpi_set_mux(x, 0, i);
        rpi_set_mux(x, 1, i);
        SETFLOAT(muxvalues      + i, rpi_read_adc(0));
        SETFLOAT(muxvalues + 16 + i, rpi_read_adc(1));
    }

    outlet_list( x->l_out, &s_list,
                 32,       muxvalues);
    
    rpi_set_mux(x, 0, 8);
    outlet_float(x->l_out, (t_float)rpi_read_adc(0));
}

void rpi_sensor_setup(void) {
    rpi_sensor_class = class_new(gensym("rpi_sensor"),
    (t_newmethod)rpi_sensor_new,
    (t_method)rpi_sensor_free,
    sizeof(t_rpi_sensor),
    CLASS_DEFAULT, 0);


    class_addbang(rpi_sensor_class, rpi_sensor_bang);
}
