/*********************************************************************************************
 *  @file oscillators.h
 *	@brief includes, defines and function prototypes for oscillators.c
 *		   clock frequency calculation based on Energy Mode requirement
 *
 *  @author : Rajat Chaple
 *  @date Created on: Feb 4, 2021
 **********************************************************************************************/
#ifndef SRC_OSCILLATORS_H_
#define SRC_OSCILLATORS_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>
#include "em_cmu.h"

#define ACTUAL_CLK_FREQ (OSCILLATOR_FREQ/PRESCALER_VALUE)

//function prototypes
void configure_clock(void);

#endif /* SRC_OSCILLATORS_H_ */
