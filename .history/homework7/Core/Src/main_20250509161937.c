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
#include <math.h>
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

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
#define PWM_MAX_VALUE 999
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

const uint8_t rainbow_colors[][3] = {
  {25, 202, 173}, {140, 199, 181}, {160, 238, 225}, {190, 231, 233},
  {190, 237, 199}, {214, 213, 183}, {209, 186, 116}, {230, 206, 172},
  {236, 173, 158}, {244, 96, 108}
};
const uint8_t rainbow_colors[][3] = {
  {255, 0, 0},    // 纯红
  {255, 165, 0},  // 橙色
  {255, 255, 0},  // 黄色
  {0, 255, 0},    // 纯绿
  {0, 0, 255},    // 纯蓝
  {75, 0, 130},   // 靛色 (深紫)
  {148, 0, 211}   // 紫色
};
const int num_rainbow_colors = sizeof(rainbow_colors) / sizeof(rainbow_colors[0]); 

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
  // uint8_t dir=1;
  
  // uint16_t led0pwmval=100; 
  // // __HAL_TIM_SetCompare(&htim5,TIM_CHANNEL_1,led0pwmval);
  // uint8_t task = 0x00;  // 0x00呼吸。0xFF彩虹
  // int omega = 0x01;

  uint8_t task = 0x00;  // 0x00呼吸。0xFF彩虹. 
  uint16_t omega = 1;    //
  
  uint32_t breathing_time_step = 0; // 呼吸灯时间步长
  
  uint8_t rainbow_current_index = 0; // 彩虹等颜色序号
  uint32_t rainbow_tick_counter = 0;  // 彩虹灯计时器

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
  HAL_Delay(1);

  if (!FIFO_In_IsEmpty()) // 检查输入FIFO是否有数据
    {
      FIFO_In_GetMessage(&fifo_in); // 获取输入消息
      if (fifo_in.wParam == 0x00 || fifo_in.wParam == 0xFF) // 判断任务
        {
          if (task != fifo_in.wParam) 
          {
            task = fifo_in.wParam; 
            // 任务更改时重置特定于模式的计数器
            breathing_time_step = 0;
            rainbow_current_index = 0;
            rainbow_tick_counter = 0;
          }
        }
      else
        {
          omega = fifo_in.wParam; // 更新 omega
        }
      FIFO_Out_PostMessage(0, task, 0); // 发送当前任务
      FIFO_Out_PostMessage(0, omega, 0); // 发送当前 omega
      SEND;
    }
  // 测试  呼吸灯
  if (task == 0x00) {
    // HAL_Delay(1);
    // if(dir)led0pwmval++;				//dir==1 led0pwmval递增
    // else led0pwmval--;					//dir==0 led0pwmval递减 
    // if(led0pwmval>900)dir=0;			//led0pwmval到达300后，方向为递减
    // if(led0pwmval==0)dir=1;				//led0pwmval递减到0后，方向改为递增
    // __HAL_TIM_SetCompare(&htim5,TIM_CHANNEL_1,led0pwmval);
    // __HAL_TIM_SetCompare(&htim5,TIM_CHANNEL_2,led0pwmval);
    // __HAL_TIM_SetCompare(&htim5,TIM_CHANNEL_3,led0pwmval);

    breathing_time_step++;
    // omega (1-255) 控制频率。omega 越大，呼吸越快。
    // time_factor 会随时间增加，并按 omega 缩放。
    // 调整 0.0005f 因子以更改 omega 对整体速度的敏感度。
    float time_factor = (float)breathing_time_step * (float)omega * 0.005f; 

    // 使用具有相移的正弦波计算 R, G, B 值
    // (sin(x) + 1)/2 将 sin 输出从 [-1,1] 映射到 [0,1]
    float r_norm = (sinf(time_factor) + 1.0f) / 2.0f;

    // 将归一化值缩放到 PWM 范围
    uint16_t pwm_r = (uint16_t)(r_norm * PWM_MAX_VALUE);

    __HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_1, pwm_r); // 设置R通道PWM占空比
    __HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_2, pwm_r); // 设置G通道PWM占空比
    __HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_3, pwm_r); // 设置B通道PWM占空比
  }
  if (task == 0xFF) {
    rainbow_tick_counter++;
      
    // omega (1-255) 控制速度。omega 越大，颜色切换越快。
    // 计算颜色切换的延迟。例如，如果 omega=1，延迟为 500ms。如果 omega=250，延迟为 2ms。
    // 调整 '500' 以适应整体速度范围。
    uint16_t switch_delay_ms = 10000 / omega; 
    if (switch_delay_ms < 1) { // 确保最小延迟为 1ms (当前循环迭代时间)
        switch_delay_ms = 1; 
    }

    if (rainbow_tick_counter >= switch_delay_ms) {
      rainbow_tick_counter = 0; // 重置计数器
      rainbow_current_index = (rainbow_current_index + 1) % num_rainbow_colors; // 切换到下一个颜色，循环切换
    }
    
    // 从颜色表中获取 RGB 值并将其缩放到 PWM 范围
    // (原始颜色值 / 255.0) * PWM最大值
    uint16_t r_val = (uint16_t)((float)rainbow_colors[rainbow_current_index][0] / 255.0f * PWM_MAX_VALUE);
    uint16_t g_val = (uint16_t)((float)rainbow_colors[rainbow_current_index][1] / 255.0f * PWM_MAX_VALUE);
    uint16_t b_val = (uint16_t)((float)rainbow_colors[rainbow_current_index][2] / 255.0f * PWM_MAX_VALUE);
    
    __HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_1, r_val); // 设置R通道PWM占空比
    __HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_2, g_val); // 设置G通道PWM占空比
    __HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_3, b_val); // 设置B通道PWM占空比
  }

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
