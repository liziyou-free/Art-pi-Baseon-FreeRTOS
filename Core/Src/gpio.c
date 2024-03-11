/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
     PA14 (JTCK/SWCLK)   ------> DEBUG_JTCK-SWCLK
     PC15-OSC32_OUT (OSC32_OUT)   ------> RCC_OSC32_OUT
     PC14-OSC32_IN (OSC32_IN)   ------> RCC_OSC32_IN
     PA13 (JTMS/SWDIO)   ------> DEBUG_JTMS-SWDIO
     PH1-OSC_OUT (PH1)   ------> RCC_OSC_OUT
     PH0-OSC_IN (PH0)   ------> RCC_OSC_IN
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_SD_DETECT_Pin|GPIO_TOUCHSCREEN_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIO_LCD_BLACKLIGHT_GPIO_Port, GPIO_LCD_BLACKLIGHT_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIO_ETH_PHY_RST_GPIO_Port, GPIO_ETH_PHY_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PDPin PDPin PDPin */
  GPIO_InitStruct.Pin = GPIO_SD_DETECT_Pin|GPIO_LCD_BLACKLIGHT_Pin|GPIO_TOUCHSCREEN_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = GPIO_TOUCHSCREEN_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIO_TOUCHSCREEN_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PI3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = GPIO_ETH_PHY_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIO_ETH_PHY_RST_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */

void hardware_reset(void) {

    GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* \brief Ethernet phy reset */
    HAL_GPIO_WritePin(GPIO_ETH_PHY_RST_GPIO_Port, GPIO_ETH_PHY_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(GPIO_ETH_PHY_RST_GPIO_Port, GPIO_ETH_PHY_RST_Pin, GPIO_PIN_SET);

	/* \brief LCD BackLight */
    HAL_GPIO_WritePin(GPIO_LCD_BLACKLIGHT_GPIO_Port, GPIO_LCD_BLACKLIGHT_Pin, GPIO_PIN_SET);

    /* \brief Touch Controller */
    /* Set as push-pull output */
    GPIO_InitStruct.Pin = GPIO_TOUCHSCREEN_INT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIO_TOUCHSCREEN_INT_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIO_TOUCHSCREEN_RST_GPIO_Port, GPIO_TOUCHSCREEN_RST_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIO_TOUCHSCREEN_INT_GPIO_Port, GPIO_TOUCHSCREEN_INT_Pin, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(GPIO_TOUCHSCREEN_INT_GPIO_Port, GPIO_TOUCHSCREEN_INT_Pin, GPIO_PIN_SET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(GPIO_TOUCHSCREEN_RST_GPIO_Port, GPIO_TOUCHSCREEN_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(20);

    /* Set as input to detect external interrupt signal */
//    GPIO_InitStruct.Pin = GPIO_TOUCHSCREEN_INT_Pin;
//    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    HAL_GPIO_Init(GPIO_TOUCHSCREEN_INT_GPIO_Port, &GPIO_InitStruct);

    /*
     * Reset SDcard Detect, Led ...
     */
}
/* USER CODE END 2 */
