/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FIFO.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
//  发送
#define SEND \
    do { \
        if (!FIFO_Out_IsEmpty() && FIFO_Out_GetMessage(&fifo_out)) { \
            HAL_UART_Transmit_IT(&huart1, (uint8_t *)&fifo_out.wParam, 1); \
        } \
    } while (0)
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t RxBuffer;
// FIFO fifo_out;
FIFO fifo_in; // 将 fifo_in 定义为全局变量
FIFO fifo_out;	
// uint8_t task = TASK_IDLE;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart1,(uint8_t*)&RxBuffer,1);
  HAL_UART_Transmit(&huart1, (uint8_t*)"\xBB", 1, 100);

  HAL_TIM_PWM_Start(&htim5,TIM_CHANNEL_1); 
  HAL_TIM_PWM_Start(&htim5,TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim5,TIM_CHANNEL_3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint8_t dir=1;
  uint16_t led0pwmval=100; 
  // __HAL_TIM_SetCompare(&htim5,TIM_CHANNEL_1,led0pwmval);
  uint8_t task = 0x00;  // 0x00呼吸。0xFF彩虹
  
  while (1)
  {
	// // 测试程序	  串口接受数据存入队列，接收到21或22时，发送数据
  // if (FIFO_In_GetMessage(&fifo_in))
  // {
  //     FIFO_Out_PostMessage(0, fifo_in.wParam, 0);
  //     if ((fifo_in.wParam == 0x21) || (fifo_in.wParam == 0x22)) {
  //         // 触发开始发送
  //         if (FIFO_Out_GetMessage(&fifo_out)) {
  //             HAL_UART_Transmit_IT(&huart1, (uint8_t*)&fifo_out.wParam, 1);
  //         }
  //     }
  // }    

  if (!FIFO_In_IsEmpty()) // 检查输入FIFO是否有数据
    {
      FIFO_In_GetMessage(&fifo_in); // 获取输入消息
      task = fifo_in.wParam; // 更新任务
    }
  // 测试  呼吸灯
  if (task == 0x00) {
    HAL_Delay(1);
    if(dir)led0pwmval++;				//dir==1 led0pwmval递增
    else led0pwmval--;					//dir==0 led0pwmval递减 
    if(led0pwmval>900)dir=0;			//led0pwmval到达300后，方向为递减
    if(led0pwmval==0)dir=1;				//led0pwmval递减到0后，方向改为递增
    __HAL_TIM_SetCompare(&htim5,TIM_CHANNEL_1,led0pwmval);
    __HAL_TIM_SetCompare(&htim5,TIM_CHANNEL_2,led0pwmval);
    __HAL_TIM_SetCompare(&htim5,TIM_CHANNEL_3,led0pwmval);
  }
  if (task == 0xFF) {


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
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
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance==USART1)
    {
			
        if(!FIFO_In_PostMessage(0,RxBuffer,0))
				{
					// printf("FIFO In is Full!\n");
				}

        HAL_UART_Receive_IT(&huart1,(uint8_t*)&RxBuffer,1);
			}
    
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance==USART1)
    {
			
        if (FIFO_Out_GetMessage(&fifo_out))
        {
            HAL_UART_Transmit_IT(&huart1, (uint8_t *)&fifo_out.wParam, 1); // ����������һ������
        }
			
    }
}
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
