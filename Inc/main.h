/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define USE_2_BUFFERS
#define USE_CAN_BUFFER

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define DIP_SW1_Pin GPIO_PIN_13
#define DIP_SW1_GPIO_Port GPIOC
#define DIP_SW2_Pin GPIO_PIN_14
#define DIP_SW2_GPIO_Port GPIOC
#define DIP_SW3_Pin GPIO_PIN_15
#define DIP_SW3_GPIO_Port GPIOC
#define CAN1_Mode0_Pin GPIO_PIN_0
#define CAN1_Mode0_GPIO_Port GPIOC
#define CAN1_Mode1_Pin GPIO_PIN_1
#define CAN1_Mode1_GPIO_Port GPIOC
#define CAN2_Mode0_Pin GPIO_PIN_2
#define CAN2_Mode0_GPIO_Port GPIOC
#define CAN2_Mode1_Pin GPIO_PIN_3
#define CAN2_Mode1_GPIO_Port GPIOC
#define LED3_Red_Pin GPIO_PIN_0
#define LED3_Red_GPIO_Port GPIOB
#define LED4_Green_Pin GPIO_PIN_1
#define LED4_Green_GPIO_Port GPIOB
#define LED1_Blue_Pin GPIO_PIN_6
#define LED1_Blue_GPIO_Port GPIOC
#define LED2_Red_Pin GPIO_PIN_7
#define LED2_Red_GPIO_Port GPIOC
#define BT_StreamingStatus_Pin GPIO_PIN_8
#define BT_StreamingStatus_GPIO_Port GPIOC

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */
typedef void (*Callback)(int);
/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
