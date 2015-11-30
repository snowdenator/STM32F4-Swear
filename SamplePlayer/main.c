#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include "sample.h"

TIM_TimeBaseInitTypeDef TIM_BaseStruct;
NVIC_InitTypeDef NVIC_InitStruct;
GPIO_InitTypeDef GPIO_InitStruct;

int p = 0;

int currentByte = 0;

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int main(void)
{
	SystemInit();

	//Configure PWM timer clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	TIM_BaseStruct.TIM_Prescaler = 0;
	TIM_BaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_BaseStruct.TIM_Period = 839; //(84Mhz / 100KHz) - 1
	TIM_BaseStruct.TIM_ClockDivision = 0;
	TIM_BaseStruct.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM4, &TIM_BaseStruct);

	TIM_Cmd(TIM4, ENABLE);

	//Configure PWM channels
	TIM_OCInitTypeDef TIM_OCStruct;

	TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2; //Set on compare match
	TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;

	//TIM_OCStruct.TIM_Pulse = 0;
	TIM_OC1Init(TIM4, &TIM_OCStruct);
	//TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

	//Configure GPIO output
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	//Configure NVIC
	NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	//Configure interrupt timer clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_BaseStruct.TIM_Prescaler = 0;
	TIM_BaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_BaseStruct.TIM_Period = 1968; //(84MHz / 8KHz) - 1;
	TIM_BaseStruct.TIM_ClockDivision = 0;

	TIM_TimeBaseInit(TIM3, &TIM_BaseStruct);

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	TIM_Cmd(TIM3, ENABLE);

    while(1)
    {
    	//We have no code to run here yet
    }
}

void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
   	{
   		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

   		currentByte = bullshit_wav[p++]; //Get the next audio sample

   		TIM4->CCR1 = map(currentByte, 0, 254, 0, 838); //Perform map between integer and actual pwm range

   		if (p == bullshit_wav_len) //If we got to end
   		{
   			p = 0; //Start again for never ending bullshit!
   		}
   	}
}
