/*
 * MCU.h
 *
 *  Created on: 2 May 2021
 *      Author: solwan Shokry Ahmed
 */

#ifndef MCU_H_
#define MCU_H_


void display (void);
void Reset (void);
void Timer1_CTC_A_Mode_Init(void);
ISR(TIMER1_COMPA_vect);

void INT0_Init(void);
ISR(INT0_vect);

void INT1_Init(void);
ISR(INT1_vect);

void INT2_Init(void);
ISR(INT2_vect);

#endif /* MCU_H_ */
