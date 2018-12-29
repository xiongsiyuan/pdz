/* --------------------------------
program:pdz2018.10.23
revise date:2018.12.27
function: 
.port initialize
.t8p1 interrupt
.adc

 ----------------------------------*/
#include <hic.h>
#include "pdz.h"

static volatile unsigned char bit_use1 @0x0011;
static volatile sbit vb_check_command	@(unsigned) &bit_use1 * 8 + 0;
//static volatile sbit vb_check_tag		@(unsigned) &bit_use1 * 8 + 1;//vb检测完毕标记
static volatile sbit charge_tag			@(unsigned) &bit_use1 * 8 + 2;
static volatile sbit uv_tag				@(unsigned) &bit_use1 * 8 + 3;//under volt. tag
static volatile sbit vb_steady_tag		@(unsigned) &bit_use1 * 8 + 4;
static volatile sbit adc_delay_tag		@(unsigned) &bit_use1 * 8 + 5;

static volatile unsigned char bit_use2 @0x0012;
static volatile sbit c_0	@(unsigned) &bit_use2 * 8 + 0;
//static volatile sbit c_1	@(unsigned) &bit_use2 * 8 + 1;


unsigned int intCount = 0;
unsigned int vb = 0;

unsigned char uv_count = 0;
unsigned char t_adcd = 0;

void Ram_Clr(void) //clear ram
{
	_asm 
  { 
	CLR   IAAL;
	CLR   IAAH;
	CLR   IAD;
	INC   IAAL,1;
	JBS   IAAL,6;
	GOTO  $-3;
  } 
}

void port_init(void)
{
	//porta
	PA = 0;
	ANS = 0xfe;//0=AN,1=IO
	PAT = 0x0c;//0=out, 1=in
	N_PAU = 0xf7;//0=enable, 1=disable
	N_PAD = 0xff; 
	//portb
	PB = 0;
	PBT = 0x00;
}

void clock_init(void)
{
	OSCP = 0x55;
	OSCC = 0xd0;//4M
	while(!HSOSCF);//wait for clock steady
	_asm{nop;}
	while(!SW_HS);
	_asm{nop;}

	LPM = 1;//mode idle1
}

void wdt_init(void)
{
	WDTC = 0x16;//1s
	WDTP = 0xff;
}

void clrwdt(void)
{
	_asm{cwdt;}
}

void int_init(void)
{
	GIE = 0;//int. disable
	INTC0 = 0;//key int. shield
	
	//PINT1 set
	PINTS = 0x04;//pint1 pa3 int.	
	PEG1 = 1;//rising edge trigger
	PIF1 = 0;
	PIE1 = 1;//pint1 int. enable
	
	//T8P1 set
    T8P1P = 244;    //设置T8P1P周期寄存器初始值
    T8P1C = 0x79;   //设置T8P1定时器模式，预分频4,后分频16
	T8P1E = 1;//定时使能
    T8P1TIF = 0;    //清除T8P1中断标志
    T8P1TIE = 1;	//使能T8P1定时中断

}

void adc_init(void)
{
	ADCCH = 0xd8;//转换结果低位对齐;adc转换时间8us;采样时间8个时钟周期
	ADCCL = 0x78;//Vref=内部2.1v;ADC通道=VDD/4
	ADEN = 1;//使能ADC
	ADTRG=1;//启动ADC
}

void var_init(void)
{
	bit_use1 = 0;
	bit_use2 = 0;
	sw_vb = SW_VB_ON;
	vb_check_command = 1;
}

void init(void)
{
	Ram_Clr();
	clock_init();
	wdt_init();
	port_init();
	int_init();
	adc_init();
	var_init();

}

void adc_channel(unsigned char channel)
{  
	ADCCL &= 0b11100011;
	ADCCL |= (channel << 2);
}

unsigned int adc_data(void)
{
		unsigned char adc_hb = 0;
		unsigned char adc_lb = 0;
		unsigned int adc_value = 0;
		//delay
		{
			unsigned char i = 0; 				
			for(i = 0;i < 20;i++)
				clrwdt();
		}
		//adc
		ADTRG = 1;//start adc
		while(ADTRG)
			clrwdt();
		adc_hb = ADCRH;
		adc_lb = ADCRL;
		adc_value = (((unsigned int)adc_hb << 8) & 0xff00)+ adc_lb;
		return adc_value;
}

void vb_adc(void)
{				
	//vb check
	if(vb_check_command)
		{			
			adc_channel(VB);//set channel
			if(adc_delay_tag)
			{
				vb = adc_data();
				vb_check_command = 0;
				adc_delay_tag = 0;
			}
		}	

}

void vb_status(void)
{
	if(vb <= VB_UV )
		uv_tag = 1;

	if(vb > VB_UVR)
		uv_tag = 0;

}

void charge_check(void)
{
	if(vin)
		charge_tag = 1;
		else charge_tag = 0;
}

void led_disp(void)
{
	//led red
	if(uv_tag)
		ledr = c_0;
		else
		{
			if(charge_tag && (PG_CHRG == pg)) 			
				ledr = LED_ON;
				else ledr = LED_OFF;
		}
	//led green
	if(charge_tag && (PG_FULL == pg))  
		ledg = LED_ON;
		else ledg = LED_OFF;
					
}

void sleep(void)
{
	if((!charge_tag) && (!uv_tag) && (vb_steady_tag) && (!vb_check_command))
	{
		sw_vb = SW_VB_OFF;
		ADEN = 0;
		//切换到内部低速时钟
		OSCP = 0x55;
		CLKSS = 0;
		while(!WDTOSCF);//等待时钟稳定
		_asm{nop;}
		while(!SW_WDT);//等待切换完成
		_asm{nop;}

		_asm{nop;}
		_asm
		{
			idle;
			nop;
		}
	}
}
void isr(void) interrupt
{
	//pa3 int.
    if(PIE1 && PIF1) 
    {
		PIF1 = 0;  		
		vb_check_command = 1;

	}
	if(!CLKSS)
	{
		//切换到内部高速时钟
		OSCP = 0x55;
		CLKSS = 1;		
		while(!HSOSCF);//等待时钟稳定
		_asm{nop;}
		while(!SW_HS);//等待切换完成
		_asm{nop;}

		sw_vb = SW_VB_ON;
		ADEN = 1;
		_asm{
			nop;
			nop;
			nop;
			nop;
			nop;
		}
		vb_check_command = 1;

	}

	//time_128 int.
	if(T8P1TIE && T8P1TIF)
    {					 //进入T8P1中断
        T8P1TIF = 0;	 //清除T8P1中断标志
		intCount++;		
		//timer 1/4s 		
		if(0 == (intCount & (ONE_4_SECOND_COUNT_MASK)))
		{
			if(!adc_delay_tag)
			{
				t_adcd++;
				if(t_adcd >= 2)
				{
					adc_delay_tag = 1;
					t_adcd = 0;
				}

			}
			

		}//end 1/4s timer

		//led_disp
		c_0 = LED_OFF;
		if((intCount & LED_FLICKER_COUNT_MASK) < LED_FLICKER_COUNT_END)
			c_0 = LED_ON;
		//vb status command	 
		if(0 == (intCount & (TWO_SECOND_COUNT_MASK)))
			vb_steady_tag = 1;
		//adc command  
		if(0 == (intCount & TIME_VB_MASK))
			vb_check_command = 1;			
				     
    }

}

void main() 
{
	init();
	GIE = 1;
	//T8P1E = 1;
	while(1)
	{
		clrwdt();
		vb_adc();
		if(vb_steady_tag)
			vb_status();
		charge_check();
		led_disp();
		sleep();

	}
}