
#include "main.h"
#include "stm32f4xx_hal.h"
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
                                
/**
														HEADER VALUE	
	STX 													49								
	ACK 													50								
	NCK 													51								
	ETX 													57									
	** SEND FRAME 	
		* COMMAND 	: 	 
		49  	 44      48(49)    44     		48(49)  		 44    			 MSB	 		 44         LSB			   44				57 		 										(--11 byte--)
		STX  		,  		MOV(POS)  	,  		(MODE ENCODER)  	,     	SETPOINT  		,    		SETPOINT			, 			ETX	
		
		* ECHO COMMAND: 
		49  	 44      48(49)    44     		48(49)  		 44    			 MSB	 		 44         LSB			   44				50			 44			   57 		(--13 byte--)
		STX  		,  		MOV(POS)  	,  		(MODE ENCODER)  	,     	SETPOINT  		,    		SETPOINT			,				ACK				, 			ETX	
		
		* DATA: 
		49  	 44     				   44                      44										 44									44					48			 44			   57			(--13 byte--)
		STX  		,  		DATA[3]  		,  				DATA[2]  			,     	DATA[1]     	,      	DATA[0]     	,   		ZERO			,				ETX	
		
	** UART PIN:				PA2 	- 	TX 	, 	PA3 	- 		RX
	** CONTROL MODE : 	POS 	- 	48 	, 	MOV 	- 		49
	** ENCODER MODE :  	X1 		- 	48	, 	X4 		- 	 	49
	
	*****/
	
	/*  	VARIABLE  DECLARE			*/
	float 		Ts 								= 		0.05					,		// 	Sample Time (s)
						setpoint 					= 		1000							;
	char 			control_mode  		= 		48 						,
						pre_control_mode	= 		0							;
	int16_t 		duty 							= 		0							, 	//	PWM Duty
						i									=			0							;
	int16_t		ppr 							= 		440						;
	int32_t 	pulse 						= 		0							,		//	Encoder Value
						pre_pulse 				= 		0							;
	int32_t 	count = 0, pre_count = 0;
	
	/*      		UART 	    			*/ 
	uint8_t 	receive_data	[11]										;
	uint8_t 	send_data			[13]										;
	
	/* 					POSITION 				*/
	int32_t 	pos    						= 		0							;
	float 		e_k								= 		0							,		 
						e_k_1 						= 		0							, 
						e_k_2 						= 		0							, 
						u_k 							=			0							, 
						u_k_1 						= 		0							,
						Kp_p							=			0.022					,//0.02
						Ki_p							=			0.00001							,//0
						Kd_p 							= 		0.5						;
	
	/* 					SPEED 					*/
	float 			w 							= 		0							,
						Kp_w							=			0.06					,
						Ki_w							=			0.18							,//0.2
						Kd_w 							= 		0.0001							;
	
	/* 					ENCODER MODE  	*/
	char 			encoder_mode 			= 		48							,
						pre_encoder_mode 		= 		0 						,
						a									=			0							,
						b									=			0							,
						pre_a							=			0							,
						pre_b 						= 		0																;
	int32_t  dif = 0;
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
int main(void)

{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
	/* 			FUCTION INITIATE  	*/
	HAL_TIM_PWM_Start			(			&htim4		,		TIM_CHANNEL_1										);
	HAL_TIM_PWM_Start			(			&htim4		,		TIM_CHANNEL_2										);
	HAL_TIM_Base_Start_IT	(			&htim3																				);
	HAL_TIM_Base_Start_IT	(			&htim2																				);
	HAL_UART_Receive_DMA(&huart2,&receive_data[0],11);
	/* 		 HEADER INITIATE			*/
	send_data[0] 	=  	49;
	send_data[1] 	=  	44;
	send_data[3] 	=  	44;
	send_data[5] 	=  	44;
	send_data[7] 	=  	44;
	send_data[9] 	=  	44;
	send_data[11] =  	44;
	send_data[12] = 	57;
  while (1)
  {
	
  }

}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==huart2.Instance)
	{
		if((receive_data[0] == 49) &&	(receive_data[10] == 57))	
		{			
		/* 		 		ECHO DATA				*/
			for (i = 2; i<9; i+=2)
			{
				send_data[i] 		= 			receive_data[i]						;
			}
			send_data[10] 			=  			50												;//		ACK
			HAL_UART_Transmit(&huart2,send_data,13,20);
			
		/* 	   DATA	PROCESSING		*/
			setpoint = receive_data[6]*256 + receive_data[8];
			control_mode = receive_data[2];
			encoder_mode = receive_data[4];
		/* 		RESET VARIABLE WHEN MODE CHANGED	*/
			if(encoder_mode != pre_encoder_mode)
			{
					pulse = 0;
					pre_pulse = 0;
			}
			if(control_mode != pre_control_mode)
			{
					e_k_2 = e_k_1 = 0; 
					u_k_1 = u_k 	= 0;
			}
			pre_encoder_mode = encoder_mode;
			pre_control_mode = control_mode;
		/* 		 		---------				*/
		}	
	}
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == htim3.Instance)
	{
		/* MOTOR TEST *
		
		__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2,30);
		__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1,0);
		/**/
		
		/* SEND ENCODER VALUE	*/
		if(setpoint != 1000)
		{
		send_data[2] = (pulse & 0xFF000000)>>24		;	//MSB
		send_data[4] = (pulse & 0x00FF0000)>>16		;	
		send_data[6] = (pulse & 0x0000FF00)>>8		;
		send_data[8] = 	pulse 	& 0x000000FF			;  //LSB
		send_data[10] =	48			;				//  zero
		HAL_UART_Transmit(&huart2		,		&send_data[0]			,			13		, 20	);
		}
		/* 	UPDATE POSITION 	*/
				pos = pulse;
				if(encoder_mode == 48)			//	x1 mode
				{
				i = pos/ppr;
				if(pos > ppr || pos < -ppr)
						pos = pos - (i)*ppr;
						pos = pos*360/ppr;
				w = (pulse - pre_pulse)*1200/ppr;
				}
				else if(encoder_mode == 49)	//	x4 mode
				{
					i = pos/(4*ppr);
				if(pos > (ppr*4) || pos < (-ppr*4))
					pos = pos - (i)*(ppr*4);
					pos = pos*360/(ppr*4);
					w = (pulse - pre_pulse)*1200/(ppr*4);
				}
		/* PID CONTROLING */
				e_k_2 = e_k_1;
				e_k_1 = e_k; 
				u_k_1 = u_k;
				/*---------*/
				if((setpoint == 400) || (setpoint == 1000))
	{
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2,0);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1,0);
					e_k_2 = e_k_1 = 0; 
					u_k_1 = u_k 	= 0;
	}
		else 
		{
		if(control_mode == 48)
	{
				/* POSITION*/
		
				e_k = setpoint - pos;
				u_k = u_k_1 + Kp_p*(e_k - e_k_1) + Ki_p*Ts/2*(e_k+e_k_1) + Kd_p*Ts*(e_k - 2*e_k_1 + e_k_2);
					/*---------*/
	}
				
				else if(control_mode == 49)
	{
				/* SPEED */
				e_k = setpoint - w;
				u_k = u_k_1 + Kp_w*(e_k - e_k_1) + Ki_w*Ts/2*(e_k+e_k_1) + Kd_w*Ts*(e_k - 2*e_k_1 + e_k_2);		
					/*----------*/
	}
					duty = u_k/12*255;
			/* PWM GENERATION */
	
	
	if(duty > 0)
	{
						duty += 15;		//deadzone compensation
				if(duty > 255)
						duty = 255;
				
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1,0);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2,duty);
	}
	else
	{
				duty -= 15;
				if(duty < (-255))
						duty = (-255);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2,0);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1,-duty);
	}	
	}
		/*---*/
	pre_pulse = pulse;
}
	
}
	
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	a = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
			b = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1);
	count = __HAL_TIM_GET_COUNTER(&htim2);
	dif = count - pre_count;
	if(encoder_mode == '0')					// 	x1 mode
	{
	if(GPIO_Pin == GPIO_PIN_0)			// kenh a
	{
			if(dif > 80 )
		{
			if(a==1)
			{
			if(b==1)
				pulse --;
			else if(b==0)
				pulse ++;
			}
		}
	}
	}
	else if(encoder_mode == '1')		//	x4 mode
	{
			if(((a==1)&&(pre_a==0)) || ((a==0)&&(pre_a == 1)))
	{
		if(b==pre_a)
		pulse++;
		else if(b==a)
		pulse--;
	}
		if(((b==1)&&(pre_b==0)) || ((b==0)&&(pre_b == 1)))
	{
		if(a==pre_b)
		pulse--;
		else if(a==b)
		pulse++;
	}
	}
		pre_a = a;
		pre_b = b;
		pre_count = count;
}
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* TIM2 init function */
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 84;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0xFFFFFFFF;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 42000;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 99;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM4 init function */
static void MX_TIM4_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 672;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 255;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim4);

}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 256000;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pins : PA0 PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
