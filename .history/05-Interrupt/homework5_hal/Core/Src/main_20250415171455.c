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
#include "msg.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define MSG_NONE		0		// No message
#define MSG_TIMER		1		// Periodic (1 ms) timer message
#define MSG_BTNDOWN		2		// Button pressed (0 to 1)
#define MSG_BTNUP		3		// Button released (1 to 0)
#define MSG_BTNDBLCLK	4		// button double clicked
#define MSG_CHAR		5		// Repeat message if a button continiously pressed

#define BTN_A			1
#define BTN_B			2

#define BTN_DBLCLK_INTERVAL		500		// Interval (ms) between two single clicks to identify a double click
#define BTN_REPEAT_DELAY		1000		// Delay time (ms) before repeat
#define BTN_REPEAT_INTERVAL		250		// Time interval (ms) for repeat

//#define printf(...) ((void)0)
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
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void uart_print(char *str);
static uint8_t Buffer;
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
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_Base_Start_IT(&htim7);
  HAL_UART_Receive_IT(&huart1,&Buffer,1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint8_t LED_G_state = 0;
  MSG msg;
  while (1)
  {
    if (!GetMessage(&msg)) continue;
		switch (msg.uMsg) {
			case MSG_TIMER:
        HAL_GPIO_TogglePin(LED_R_GPIO_Port,LED_R_Pin);
				break;
			case MSG_BTNDOWN:
        if (msg.wParam == BTN_A) {
          if (LED_G_state == 0) { // 不处于常亮状态时
            HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_RESET);
          }
        } 
				break;
			case MSG_BTNUP:
        if (msg.wParam == BTN_A) {
          if (LED_G_state == 0) {
            HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
          }
        }
        else if (msg.wParam == BTN_B) {
          HAL_GPIO_TogglePin(LED_B_GPIO_Port,LED_B_Pin);
          int LED_G_state = HAL_GPIO_ReadPin(LED_G_GPIO_Port,LED_G_Pin);
          if (LED_G_state == 1) {
            uart_print("b");
          } else {
            uart_print("B");
          }
        }
				break;
			case MSG_BTNDBLCLK: // 双击
        if (msg.wParam == BTN_A) {
          HAL_GPIO_TogglePin(LED_G_GPIO_Port,LED_G_Pin);
          LED_G_state = 1 - LED_G_state; 
        }
				break;
			case MSG_CHAR:
        uart_print("B");
				break;
			default:
				break;
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
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

void uart_print(char *str)
{
  HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 10);
}



static uint32_t last_key_time = 0;

static uint32_t btn_a_last_click_time = 0; // A上次点击时间
static uint8_t btn_a_click_count = 0;

static uint32_t btn_b_press_start_time = 0;  // B键按下开始时间
static uint8_t btn_b_is_pressed = 0;         // B键是否处于按下状态
static uint32_t btn_b_last_repeat_time = 0; // B键上次发送重复消息的时间

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  uint32_t current_time = HAL_GetTick();
  // 消抖
  if(current_time - last_key_time > 20) 
  {
    last_key_time = current_time;
  }
  else
  {
    return; 
  }

	//A
	if(GPIO_Pin==KEY_A_Pin)
	{
    uint8_t pin_state = HAL_GPIO_ReadPin(KEY_A_GPIO_Port,KEY_A_Pin);

    if(pin_state == 1) // 按下
    {
      PostMessage(MSG_BTNDOWN, BTN_A, 0);
    }
    else // 释放
    {
      PostMessage(MSG_BTNUP, BTN_A, 0);
      
      // 双击检测
      if(btn_a_click_count == 0)
      {
        btn_a_click_count = 1;
        btn_a_last_click_time = current_time;
      }
      else if(btn_a_click_count == 1)
      {

        if(current_time - btn_a_last_click_time < BTN_DBLCLK_INTERVAL)
        {
          PostMessage(MSG_BTNDBLCLK, BTN_A, 0);
          btn_a_click_count = 0; 
        }
        else // 间隔时间较长
        {
          btn_a_click_count = 1;
          btn_a_last_click_time = current_time;
        }
      }
    }
		
	}
  // 	//����B
	// if(GPIO_Pin==KEY_A_Pin)
	// {
	// 	if(HAL_GPIO_ReadPin(KEY_A_GPIO_Port,KEY_A_Pin)==0)
	// 	{
	// 		HAL_Delay(100);
	// 		if(HAL_GPIO_ReadPin(KEY_A_GPIO_Port,KEY_A_Pin)==0){
	// 			HAL_GPIO_TogglePin(LED_G_GPIO_Port,LED_G_Pin);
	// 		}
	// 	}
	// }


	//B
  if(GPIO_Pin == KEY_B_Pin)
  {
      uint8_t pin_state = HAL_GPIO_ReadPin(KEY_B_GPIO_Port, KEY_B_Pin);

      if(pin_state == 1) // 按下
      {
        if(btn_b_is_pressed == 0)
        {
          btn_b_is_pressed = 1;                     // 标记为按下状态
          btn_b_press_start_time = current_time;    // 记录按下开始时间
          btn_b_last_repeat_time = current_time;    // 初始化上次重复时间，避免立即触发重复
        }
      }
      else 
      {
        if(btn_b_is_pressed == 1)
        {
          btn_b_is_pressed = 0;                    
          PostMessage(MSG_BTNUP, BTN_B, 0);        
        }
      }
  }

  // 	if(GPIO_Pin==KEY_B_Pin)
	// {
	// 	HAL_Delay(100);
	// 	if(HAL_GPIO_ReadPin(KEY_B_GPIO_Port,KEY_B_Pin)==1)
	// 	{
	// 		HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_RESET);
	// 	}else if(HAL_GPIO_ReadPin(KEY_B_GPIO_Port,KEY_B_Pin)==0)
	// 	{
	// 		HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
	// 	}
		
	// }
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM6)
	{
    PostMessage(MSG_TIMER, 0, 0);
	}

  if(htim->Instance==TIM7) 
  {
      uint32_t current_time = HAL_GetTick();

      if(btn_b_is_pressed) // 检查 B 键是否仍被按下
      {
        if(current_time - btn_b_press_start_time >= BTN_REPEAT_DELAY)
        {
          // 检查距离上次发送重复消息的时间是否已达到重复间隔
          if(current_time - btn_b_last_repeat_time >= BTN_REPEAT_INTERVAL)
          {
            // 发送长按重复消息
            PostMessage(MSG_CHAR, BTN_B, current_time - btn_b_press_start_time);
            btn_b_last_repeat_time = current_time; // 更新上次发送时间
          }
        }
      }

      // --- 按键 A 双击超时检测 (暂时忽略) ---
      // if(btn_a_click_count == 1 && (current_time - btn_a_last_click_time >= BTN_DBLCLK_INTERVAL))
      // {
      //   btn_a_click_count = 0;
      // }
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
