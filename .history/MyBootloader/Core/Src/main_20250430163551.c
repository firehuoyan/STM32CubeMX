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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define TASK_IDLE      0
#define TASK_GET       1
#define TASK_READ      2
#define TASK_WRITE     3
#define TASK_GO        4
#define TASK_MUX       5

#define ACK  0x79   // 确认应答
#define NACK 0x1F   // 否定应答


#ifndef __FIFO_H__
#define __FIFO_H__

// FIFO 输入结构体
typedef struct {
    unsigned int uFIFO_In;
    int wParam;
    int lParam;
} FIFO_In, *LPFIFO_In;

// FIFO 输出结构体
typedef struct {
    unsigned int uFIFO_Out;
    int wParam;
    int lParam;
} FIFO_Out, *LPFIFO_Out;

// 输入函数
int FIFO_In_GetMessage(LPFIFO_In lpFifo_In);
int FIFO_In_PostMessage(unsigned int uFIFO_In, int wParam, int lParam);
int FIFO_In_IsEmpty(void);
int FIFO_In_IsFull(void);
// 输出函数
int FIFO_Out_GetMessage(LPFIFO_Out lpFifo_Out);
int FIFO_Out_PostMessage(unsigned int uFIFO_Out, int wParam, int lParam);
int FIFO_Out_IsEmpty(void);
int FIFO_Out_IsFull(void);
#endif


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t task = TASK_IDLE;

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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    FIFO_In fifo_in;
    FIFO_Out fifo_out;
    
    switch (task)
    { 
      case TASK_IDLE:    // 空闲状态
        if (!FIFO_In_IsEmpty()) // 检查输入FIFO是否有数据
        {
          FIFO_In_GetMessage(&fifo_in); // 获取输入消息
          if (fifo_in.wParam == 0x7F)   // 检查是否为同步字节
          {
            // 发送ACK确认
            FIFO_Out_PostMessage(0, 0x79, 0);
            task = TASK_MUX;   // 进入指令判断状态
          }
        }
        break;
      
      case TASK_MUX:     // 指令判断
        if (!FIFO_In_IsEmpty()) // 检查输入FIFO
        {
          FIFO_In_GetMessage(&fifo_in); // 获取命令字节
          uint8_t cmd = fifo_in.wParam;
          
          // 等待补码字节
          while (FIFO_In_IsEmpty());
          FIFO_In_GetMessage(&fifo_in);
          uint8_t cmd_compl = fifo_in.wParam;
          
          // 验证命令有效性
          if (cmd + cmd_compl == 0xFF)
          {
            switch (cmd)
            {
              case 0x00: // 获取命令
                FIFO_Out_PostMessage(0, 0x79, 0); // 发送ACK
                task = TASK_GET;
                break;
                
              case 0x21: // 执行命令
                FIFO_Out_PostMessage(0, 0x79, 0); // 发送ACK
                task = TASK_GO;
                break;
                
              case 0x31: // 写内存命令
                FIFO_Out_PostMessage(0, 0x79, 0); // 发送ACK
                task = TASK_WRITE;
                break;
                
              case 0x11: // 读内存命令
                FIFO_Out_PostMessage(0, 0x79, 0); // 发送ACK
                task = TASK_READ;
                break;
                
              default: // 未知命令
                FIFO_Out_PostMessage(0, 0x1F, 0); // 发送NACK
                task = TASK_IDLE;
                break;
            }
          }
          else // 命令校验失败
          {
            FIFO_Out_PostMessage(0, 0x1F, 0); // 发送NACK
            task = TASK_IDLE;
          }
        }
        break;
        
      case TASK_GET: // 获取命令信息
        {
          // 发送Bootloader支持的命令信息
          // 假设GET_info为12字节数组，拆分发送
          uint8_t GET_info[12] = {0x00, 0x01, 0x11, 0x21, 0x31, 0x43, 0x63, 0x73, 0x82, 0x92, 0xA1, 0xB1};
          
          for (int i = 0; i < sizeof(GET_info); i++)
          {
            FIFO_Out_PostMessage(0, GET_info[i], 0);
          }
          
          task = TASK_IDLE;
        }
        break;
        
      case TASK_GO: // 跳转执行应用程序
        {
          uint32_t addr = 0;
          
          // 接收4字节地址
          for (int i = 0; i < 4; i++)
          {
            while (FIFO_In_IsEmpty());
            FIFO_In_GetMessage(&fifo_in);
            addr = (addr << 8) | (fifo_in.wParam & 0xFF);
          }
          
          // 发送确认
          FIFO_Out_PostMessage(0, 0x79, 0);
          
          // 跳转到应用程序
          void (*jump_to_app)(void) = (void (*)(void))addr;
          __set_MSP(*(uint32_t*)addr); // 设置主堆栈指针
          jump_to_app();              // 跳转到应用程序
          
          task = TASK_IDLE;
        }
        break;
        
      case TASK_WRITE: // 写入内存
        {
          uint32_t addr = 0;
          
          // 接收4字节地址
          for (int i = 0; i < 4; i++)
          {
            while (FIFO_In_IsEmpty());
            FIFO_In_GetMessage(&fifo_in);
            addr = (addr << 8) | (fifo_in.wParam & 0xFF);
          }
          
          // 接收数据长度和校验和
          while (FIFO_In_IsEmpty());
          FIFO_In_GetMessage(&fifo_in);
          uint8_t length = fifo_in.wParam;
          
          // 接收数据并写入内存
          for (int i = 0; i < length; i++)
          {
            while (FIFO_In_IsEmpty());
            FIFO_In_GetMessage(&fifo_in);
            *((uint8_t*)addr + i) = fifo_in.wParam;
          }
          
          // 发送确认
          FIFO_Out_PostMessage(0, 0x79, 0);
          
          task = TASK_IDLE;
        }
        break;
        
      case TASK_READ: // 读取内存
        {
          uint32_t addr = 0;
          
          // 接收4字节地址
          for (int i = 0; i < 4; i++)
          {
            while (FIFO_In_IsEmpty());
            FIFO_In_GetMessage(&fifo_in);
            addr = (addr << 8) | (fifo_in.wParam & 0xFF);
          }
          
          // 接收读取长度
          while (FIFO_In_IsEmpty());
          FIFO_In_GetMessage(&fifo_in);
          uint8_t length = fifo_in.wParam;
          
          // 发送数据
          for (int i = 0; i < length; i++)
          {
            FIFO_Out_PostMessage(0, *((uint8_t*)addr + i), 0);
          }
          
          task = TASK_IDLE;
        }
        break;
    }
    
    // 处理系统任务
    HAL_Delay(1); // 短暂延时，避免过度占用CPU
    
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

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
