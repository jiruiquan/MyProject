/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "key.h"
#include "iap.h"
#include "string.h"
#include "stmflash.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

#define TEST

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


uint8_t key = 0;
uint32_t g_usart_tx_buf[100];



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint8_t t;
	uint32_t oldcount = 0;      /* 老的串口接收数据值 */
	uint32_t applenth = 0;      /* 接收到的app代码长度 */
	
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
	
	//用的内部HSI时钟
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
	
	/* 该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量 */
  HAL_UART_Receive_IT(&huart1, (uint8_t *)g_rx_buffer, RXBUFFERSIZE); 
	printf("bootloader	start\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		
#if 1
		//串口1接收到了数据
		//当触发串口接收的时候 都会进来
		//当串口接收完成 不收了 oldcount == g_usart_rx_cnt  就相等了
		if (g_usart_rx_cnt)
		{
				if (oldcount == g_usart_rx_cnt)   /* 新周期内,没有收到任何数据,认为本次数据接收完成 */
				{
						applenth = g_usart_rx_cnt;
						oldcount = 0;
						g_usart_rx_cnt = 0;
						printf("用户程序接收完成!\r\n");
						printf("代码长度:%dBytes\r\n", applenth);
				}
				else oldcount = g_usart_rx_cnt;
		}

		t++;
		HAL_Delay(100);

 		if (t == 3)
		{
				HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_10);
				t = 0;


		}

		if(key != 0)
			printf("key = %d\r\n",key);
	
		if (key == 3)   /* 按下KEY3,更新固件到FLASH */
		{

				if (applenth)
				{
#ifdef	TEST					
						memset(g_usart_tx_buf,0,100);
						stmflash_read(0x08010000,g_usart_tx_buf,20);
						printf("读取 0x08010000 写入flash前内存：\r\n");
						for(int i = 0;i < 10;i++)
						{
							
							printf("0x%x\r\n",g_usart_tx_buf[i]);
							
						}	
#endif
						printf("开始更新固件...\r\n");
		
						if (((*(volatile uint32_t *)(0X20001000 + 4)) & 0xFF000000) == 0x08000000)  /* 判断是否为0X08XXXXXX */
						{
							printf("更新开始...\r\n");
							//写入Flash
							stmflash_write(FLASH_APP1_ADDR, (uint32_t *)g_usart_rx_buf, applenth);
							printf("固件更新完成!\r\n");
#ifdef	TEST							
							memset(g_usart_tx_buf,0,100);
							stmflash_read(FLASH_APP1_ADDR,g_usart_tx_buf,40);
							printf("读取写入后 0x%x flash内存：\r\n",FLASH_APP1_ADDR);
							for(int i = 0;i < 10;i++)
							{
								
								printf("0x%x\r\n",g_usart_tx_buf[i]);
								
							}
#endif
						}
						else
						{

								printf("非FLASH应用程序!\r\n");
						}
				}
				else
				{
						printf("没有可以更新的固件!\r\n");

				}
				key = 0;

		}

		if (key == 1)   /* KEY1按键按下, 运行FLASH APP代码 */
		{

				//0xFFFFFFFF		0x08010000
				if (((*(volatile uint32_t *)(FLASH_APP1_ADDR + 4)) & 0xFF000000) == 0x08000000) /* 判断FLASH里面是否有APP,有的话执行 */
				{
						printf("开始执行FLASH用户代码!!\r\n\r\n");
						HAL_Delay(10);
						iap_load_app(FLASH_APP1_ADDR);/* 执行FLASH APP代码 */

				}
				else
				{
						printf("没有可以运行的固件!\r\n");

				}
				key = 0;

		}


		
#endif

	}
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
