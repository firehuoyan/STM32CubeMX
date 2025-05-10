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
#define TASK_IDLE      0
#define TASK_GET       1
#define TASK_READ      2
#define TASK_WRITE     3
#define TASK_GO        4
#define TASK_MUX       5

#define ACK  0x79   // 确认应答
#define NACK 0x1F   // 否定应答

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
uint8_t task = TASK_IDLE;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static int ReceiveAddress(uint32_t *addr);
static int ReadMessageWithTimeout(uint32_t timeout_ms);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static int ReceiveAddress(uint32_t *addr) {
  uint8_t addr_bytes[4] = {0};
  uint8_t checksum = 0;

  *addr = 0; // 初始化地址
  for (int i = 0; i < 4; i++) {
      while (FIFO_In_IsEmpty());
      FIFO_In_GetMessage(&fifo_in);
      addr_bytes[i] = fifo_in.wParam & 0xFF;
      *addr = (*addr << 8) | addr_bytes[i];
      checksum ^= addr_bytes[i]; // 计算校验和
  }

  // 接收校验和
  while (FIFO_In_IsEmpty());
  FIFO_In_GetMessage(&fifo_in);
  uint8_t received_checksum = fifo_in.wParam & 0xFF;

  // 验证校验和
  return (received_checksum == checksum) ? 1 : 0;
}

static int ReadMessageWithTimeout(uint32_t timeout_ms) {
uint32_t start_time = HAL_GetTick();

// 等待FIFO不为空，或者超时
while (FIFO_In_IsEmpty()) {
    if ((HAL_GetTick() - start_time) > timeout_ms) {
        // 超时，发送NACK并返回失败
        FIFO_Out_PostMessage(0, NACK, 0);
        return 0;
    }
    HAL_Delay(1); // 短暂延时，避免过度占用CPU
}

// FIFO不为空，获取消息
FIFO_In_GetMessage(&fifo_in);
return 1;
}

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
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart1,(uint8_t*)&RxBuffer,1);

  // 开始
  HAL_UART_Transmit(&huart1, (uint8_t*)"\xBB", 1, 100);


	
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  
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


  uint8_t buf[356];  // 临时缓冲区
  uint32_t addr = 0; // 用于存储地址

  switch (task)
  { 
    case TASK_IDLE:    // 空闲状态
      if (!FIFO_In_IsEmpty()) // 检查输入FIFO是否有数据
      {
        FIFO_In_GetMessage(&fifo_in); // 获取输入消息
        if (fifo_in.wParam == 0x7F)   // 检查是否为同步字节
        {
          // 发送ACK确认
          FIFO_Out_PostMessage(0, ACK, 0);
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
        if (cmd == (cmd_compl ^ 0xFF))
        {
          switch (cmd)
          {
            case 0x00: // 获取命令
              FIFO_Out_PostMessage(0, ACK, 0); 
              task = TASK_GET;
              break;
              
            case 0x21: // 执行命令
              FIFO_Out_PostMessage(0, ACK, 0); 
              task = TASK_GO;
              break;
              
            case 0x31: // 写内存命令
              FIFO_Out_PostMessage(0, ACK, 0); 
              task = TASK_WRITE;
              break;
              
            case 0x11: // 读内存命令
              FIFO_Out_PostMessage(0, ACK, 0); 
              task = TASK_READ;
              break;
              
            default: // 未知命令
              FIFO_Out_PostMessage(0, NACK, 0); 
              task = TASK_MUX;
              break;
          }
        }
        else // 命令校验失败
        {
          FIFO_Out_PostMessage(0, NACK, 0); 
          task = TASK_MUX;
        }
      }
      break;

    case TASK_GET: // 获取命令信息
      {
        // 发送Bootloader支持的命令信息
        uint8_t GET_info[6] = {4, 0x10, 0x00, 0x11, 0x21, 0x31}; //命令数、版本号、命令列表
        
        for (int i = 0; i < sizeof(GET_info); i++)
        {
          FIFO_Out_PostMessage(0, GET_info[i], 0);
        }
        FIFO_Out_PostMessage(0, ACK, 0); 
        task = TASK_MUX;
      }
      break;
      
    // case TASK_GO: // 跳转执行应用程序
    //   {
    //     if (ReceiveAddress(&addr)) {
    //       // 校验成功，发送确认
    //       FIFO_Out_PostMessage(0, ACK, 0);
    //       SEND;

    //       // 跳转到应用程序
    //       void (*jump_to_app)(void) = (void (*)(void))addr;
    //       __set_MSP(*(uint32_t*)addr); // 设置主堆栈指针
    //       jump_to_app();              // 跳转到应用程序
    //     } else {
    //       // 校验失败，发送NACK
    //       FIFO_Out_PostMessage(0, NACK, 0);
    //     }
    //     task = TASK_MUX;
    //   }
    //   break;
    case TASK_GO: // 跳转执行应用程序
      {
        if (ReceiveAddress(&addr)) {
            // 校验成功，发送确认
            FIFO_Out_PostMessage(0, ACK, 0);
            SEND;
    
            // 确保所有数据都已发送完成
            while (!FIFO_Out_IsEmpty()) {
                HAL_Delay(1);
            }
            
            // 禁用中断
            __disable_irq();
            
            // 停止所有正在使用的外设
            HAL_UART_DeInit(&huart1);
            
            // 设置向量表偏移
            SCB->VTOR = addr;
            
            // 确保写操作完成
            __DSB();
            __ISB();
            
            // 跳转到应用程序
            void (*jump_to_app)(void) = (void (*)(void))(*((uint32_t*)(addr + 4)));
            // __set_MSP(*(uint32_t*)addr); // 设置主堆栈指针
            __set_MSP(0x20020400); // 强制设置固定的栈顶地址
            jump_to_app();              // 跳转到应用程序
        } else {
            // 校验失败，发送NACK
            FIFO_Out_PostMessage(0, NACK, 0);
        }
        task = TASK_MUX;
      }
    break;
      
    case TASK_WRITE: // 写入内存
      {
        if (ReceiveAddress(&addr)) {
            FIFO_Out_PostMessage(0, ACK, 0);
            SEND;
            
            // 接收数据长度
            while (FIFO_In_IsEmpty());
            FIFO_In_GetMessage(&fifo_in);
            uint8_t length = fifo_in.wParam & 0xFF;
            
            // 先接收所有数据到临时缓冲区
            uint8_t buffer[356]; // 临时缓冲区，假设最大不超过256字节
            uint8_t checksum = length;
            
            // 接收数据并计算校验和
            for (int i = 0; i < length+1; i++) {
                while (FIFO_In_IsEmpty());
                FIFO_In_GetMessage(&fifo_in);
                buffer[i] = fifo_in.wParam & 0xFF;
                checksum ^= buffer[i]; // 计算校验和
            }
            
            // 接收校验和
            while (FIFO_In_IsEmpty());
            FIFO_In_GetMessage(&fifo_in);
            uint8_t received_checksum = fifo_in.wParam & 0xFF;
            
            // 验证校验和
            if (received_checksum == checksum) {
                // 校验成功，将数据写入内存
                for (int i = 0; i < length+1; i++) {
                    *((uint8_t*)addr + i) = buffer[i];
                }
                FIFO_Out_PostMessage(0, ACK, 0); // 发送确认
            } else {
                // 校验失败，发送NACK
                FIFO_Out_PostMessage(0, NACK, 0);
            }
            SEND;
        } else {
            // 地址校验失败，发送NACK
            FIFO_Out_PostMessage(0, NACK, 0);
        }
        task = TASK_MUX;
      }
      break;
      
    case TASK_READ: // 读取内存
      {
        if (ReceiveAddress(&addr)) {
          FIFO_Out_PostMessage(0, ACK, 0);
          SEND;
          // 接收读取长度
          while (FIFO_In_IsEmpty());
          FIFO_In_GetMessage(&fifo_in);
          uint8_t length = fifo_in.wParam & 0xFF;
          
          // 接收长度的校验字节
          while (FIFO_In_IsEmpty());
          FIFO_In_GetMessage(&fifo_in);
          uint8_t length_checksum = fifo_in.wParam & 0xFF;
          
          // 验证长度校验和
          if (length_checksum == (length ^ 0xFF)) { 
              // 发送ACK确认
              FIFO_Out_PostMessage(0, ACK, 0);

              // 发送数据
              for (int i = 0; i < length; i++) {
                  FIFO_Out_PostMessage(0, *((uint8_t*)addr + i), 0);
              }
              SEND;
          } else {
              // 长度校验失败，发送NACK
              FIFO_Out_PostMessage(0, NACK, 0);
              SEND;
          }
        } else {
          // 校验失败，发送 NACK
          FIFO_Out_PostMessage(0, NACK, 0);
        }
        task = TASK_MUX;
      }
      break;
  }
  
  // 处理系统任务
  HAL_Delay(1); // 短暂延时，避免过度占用CPU

  // 发送数据
  SEND;

		
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
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance==USART1)
    {
			
        if(!FIFO_In_PostMessage(0,RxBuffer,0))
				{
					printf("FIFO In is Full!\n");
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
