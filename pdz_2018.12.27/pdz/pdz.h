/* --------------------------------
program:pdz.h
revise date:2018.10.23
mcu:HR7P153SA 

 ----------------------------------*/
#ifndef _PDZ_H_
#define _PDZ_H_

#define sw_vb	PA1
#define pg		PA2
#define vin		PA3
#define led1	PA4
#define led2	PA5

#define ledg led1
#define ledr led2

#define LED_ON 1 
#define LED_OFF 0
#define SW_VB_ON 1
#define SW_VB_OFF 0

//PG status
#define PG_CHRG 1
#define PG_FULL 0

//adc channel
#define AIN0 0b000
#define AIN1 0b001
#define AIN2 0b010
#define AIN3 0b011
#define AIN4 0b100
#define AIN5 0b101
#define VDD_1_4 0b110
#define VDD_1_8 0b110

#define VB AIN0

//1=8ms
#define ONE_SECOND_COUNT 128//1s
#define ONE_SECOND_COUNT_MASK ONE_SECOND_COUNT-1

#define ONE_2_SECOND_COUNT (128 >> 1)//1s/(2^1)=0.5s
#define ONE_2_SECOND_COUNT_MASK ONE_2_SECOND_COUNT-1

#define ONE_4_SECOND_COUNT (128 >> 2)	//1s/(2^2) = 1/4 s
#define ONE_4_SECOND_COUNT_MASK ONE_4_SECOND_COUNT-1

#define TWO_SECOND_COUNT (128 << 1)//2s
#define TWO_SECOND_COUNT_MASK TWO_SECOND_COUNT-1

#define LED_FLICKER_COUNT	128
#define LED_FLICKER_COUNT_MASK 	LED_FLICKER_COUNT-1
#define LED_FLICKER_COUNT_END	42 

//vb check time
#define TIME_VB (128 << 1)	//1s*(2^0) = 2s
#define TIME_VB_MASK TIME_VB-1

#define VB_UV	0x0b3f//3200mv
#define VB_UVR  0x0ca8//3600mv
#endif