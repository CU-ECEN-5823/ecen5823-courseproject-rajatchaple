/*********************************************************************************************
 *  @file irq.h
 *	@brief includes, defines and function prototypes for irq.c
 *
 *  @author : Rajat Chaple
 *  @date Created on: Feb 4, 2021
 **********************************************************************************************/

#ifndef SRC_IRQ_H_
#define SRC_IRQ_H_


#include "em_letimer.h"
#include "gpio.h"
#include "em_gpio.h"
#include "sleep.h"
#include "em_core.h"
#include "scheduler.h"
#include "timers.h"

uint32_t letimerMilliseconds(void);
uint32_t getSysTicks(void);

#endif /* SRC_IRQ_H_ */
