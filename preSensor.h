#ifndef __PRE_H__
#define __PRE_H__

#include<stdio.h>
#include <stdint.h>

static int prepare(int fd);
uint8_t control_bits_differential(uint8_t channel);
uint8_t control_bits(uint8_t channel);
int readadc(int fd, uint8_t channel);
int pressure_sensor(int fd);

#endif