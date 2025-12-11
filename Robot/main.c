#include <stdio.h>
#include <stdbool.h>

#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_usart.h"
#include "misc.h"

#include "FreeRTOS.h"
#include "task.h"

//broches capteurs
#define FRONT_TRIGGER_PIN GPIO_Pin_0
#define FRONT_ECHO_PIN    GPIO_Pin_1
#define BACK_TRIGGER_PIN  GPIO_Pin_6
#define BACK_ECHO_PIN     GPIO_Pin_7
#define TRIGGER_PORT GPIOA

// Moteur gauche (moteur A)
#define ENA_PIN   GPIO_Pin_6   // PB6 (PWM)
#define IN1_PIN   GPIO_Pin_7   // PB7
#define IN2_PIN   GPIO_Pin_8   // PB8

// Moteur droit (moteur B)
#define ENB_PIN   GPIO_Pin_9   // PB9 (PWM)
#define IN3_PIN   GPIO_Pin_10  // PB10
#define IN4_PIN   GPIO_Pin_11  // PB11


char receivedChar;
int16_t received_char_counter = 0;



void InitGPIO(void);
void InitMotorGPIO(void);
void InitPWM(void);
void UART_Init(void);
void usart_irq(void);
uint32_t MeasureDistance(uint16_t TRIGGER_PIN, uint16_t ECHO_PIN);
void FrontSensorTask(void *pvParameters);
void BackSensorTask(void *pvParameters);
//void BluetoothTask(void *pvParameters);
void UART_SendString(char* str);
void ControlMotors(char command);
void SetMotorPWM(uint16_t speed, char side);
void SetMotorDirection(bool forward, char side);

int main(void) {
    SystemInit();
    InitGPIO();  
		InitMotorGPIO();
    UART_Init();
		usart_irq();
	  InitPWM();
	
    xTaskCreate(FrontSensorTask, (const signed char*)"FrontSensorTask", 128, NULL, 1, NULL);
    xTaskCreate(BackSensorTask, (const signed char*)"BackSensorTask", 128, NULL, 1, NULL);
    //xTaskCreate(BluetoothTask, (const signed char*)"BluetoothTask", 128, NULL, 1, NULL);

    vTaskStartScheduler();
    
    while (1) {}  
}

// Initialisation des GPIO pour les capteurs
void InitGPIO(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // Configuration des broches Trigger en sortie
    GPIO_InitStruct.GPIO_Pin = FRONT_TRIGGER_PIN | BACK_TRIGGER_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(TRIGGER_PORT, &GPIO_InitStruct);

    // Configuration des broches Echo en entrée
    GPIO_InitStruct.GPIO_Pin = FRONT_ECHO_PIN | BACK_ECHO_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(TRIGGER_PORT, &GPIO_InitStruct);
}


// mesure de la distance
uint32_t MeasureDistance(uint16_t TRIGGER_PIN, uint16_t ECHO_PIN) {
    uint32_t distance;
    uint32_t startTime, stopTime;

    GPIO_SetBits(TRIGGER_PORT, TRIGGER_PIN);
    vTaskDelay(1); 
    GPIO_ResetBits(TRIGGER_PORT, TRIGGER_PIN);

    while (GPIO_ReadInputDataBit(TRIGGER_PORT, ECHO_PIN) == RESET);
    startTime = xTaskGetTickCount();

    while (GPIO_ReadInputDataBit(TRIGGER_PORT, ECHO_PIN) == SET);
    stopTime = xTaskGetTickCount();

    distance = ((stopTime - startTime) * 34300) / (2 * 1000);
    return distance;
}

// Tâche capteur 9odem
void FrontSensorTask(void *pvParameters) {
    uint32_t distanceFront;
    char buffer[50];
    
    while (1) {
        distanceFront = MeasureDistance(FRONT_TRIGGER_PIN, FRONT_ECHO_PIN);
			  sprintf(buffer, "F:%lu", (unsigned long)distanceFront);
        UART_SendString(buffer); 
        vTaskDelay(500);
    }
}

// Tache capteur teli
void BackSensorTask(void *pvParameters) {
    uint32_t distanceBack;
    char buffer[50];
    
    while (1) {
        distanceBack = MeasureDistance(BACK_TRIGGER_PIN, BACK_ECHO_PIN);
			  sprintf(buffer, "A:%lu", (unsigned long)distanceBack);
        UART_SendString(buffer);  
        vTaskDelay(500);
    }
}

void USART2_IRQHandler(void)
{
	//in here we will check the data incoming or going
	//Receiving the data
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
	{
		//To receive data and locate them into our char variable.
		receivedChar = USART_ReceiveData(USART2);
	}
  ControlMotors(receivedChar);
}
/*
// Tache mte3 Bluetooth
void BluetoothTask(void *pvParameters) { 
	char receivedChar; 
	while (1) 
		{ 
			while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);
			if (USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET) {
				// Overflow Error 
				USART_ClearFlag(USART2, USART_FLAG_ORE); 
				} 
			if (USART_GetFlagStatus(USART2, USART_FLAG_FE) != RESET) { 
					// Framing Error 
				USART_ClearFlag(USART2, USART_FLAG_FE); 
				} 
			receivedChar = (char)USART_ReceiveData(USART2); 
			//ControlMotors((char)"L"); 
			//UART_SendString(&receivedChar);
			vTaskDelay(500);
			}
		}
*/
void InitMotorGPIO(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    // Configuration des broches IN1, IN2, IN3, IN4
    GPIO_InitStruct.GPIO_Pin = IN1_PIN | IN2_PIN | IN3_PIN | IN4_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
}


void InitPWM(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    TIM_OCInitTypeDef TIM_OCStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    // Configurer les broches PWM (PB6 pour moteur gauche ; PB9 pour moteur droit)
    GPIO_InitStruct.GPIO_Pin = ENA_PIN | ENB_PIN ;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Configurer l'alternative fonction pour TIM4
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);  
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);  


    // Configurer le timer TIM4 pour une fréquence PWM spécifique
    TIM_TimeBaseStruct.TIM_Prescaler = 83;   // Pour 1 MHz si horloge à 84 MHz
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_Period = 1000 - 1;  // 1 kHz PWM
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStruct);

    // Configurer les sorties PWM (2 canaux)
    TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCStruct.TIM_Pulse = 0;  // Duty cycle initial à 0%
    TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC1Init(TIM4, &TIM_OCStruct);  // PB6
    TIM_OC2Init(TIM4, &TIM_OCStruct);  // PB9 

    TIM_Cmd(TIM4, ENABLE);  // Activer le timer
}


void SetMotorDirection(bool forward, char side) {
    if (side == 'L') {  // Moteur gauche
        if (forward) {
            GPIO_SetBits(GPIOB, IN1_PIN);
            GPIO_ResetBits(GPIOB, IN2_PIN);
        } else {
            GPIO_SetBits(GPIOB, IN2_PIN);
            GPIO_ResetBits(GPIOB, IN1_PIN);
        }
    } else if (side == 'R') {  // Moteur droit
        if (forward) {
            GPIO_SetBits(GPIOB, IN3_PIN);
            GPIO_ResetBits(GPIOB, IN4_PIN);
        } else {
            GPIO_SetBits(GPIOB, IN4_PIN);
            GPIO_ResetBits(GPIOB, IN3_PIN);
        }
    }
}

void SetMotorPWM(uint16_t speed, char side) {
    if (speed > 1000) speed = 1000;  // Limiter à 100%
    if (side == 'L') {
        TIM4->CCR1 = speed;  // ENA (PWM pour moteur gauche)
    } else if (side == 'R') {
        TIM4->CCR2 = speed;  // ENB (PWM pour moteur droit)
    }
}



// Fonction pour contrôler les moteurs
void ControlMotors(char command) {
    switch (command) {
        case 'F':  // Avancer
            SetMotorDirection(true, 'L');
            SetMotorDirection(true, 'R');
            SetMotorPWM(800, 'L');
            SetMotorPWM(800, 'R');
            break;

        case 'B':  // Reculer
            SetMotorDirection(false, 'L');
            SetMotorDirection(false, 'R');
            SetMotorPWM(800, 'L');
            SetMotorPWM(800, 'R');
            break;

        case 'R':  // Tourner à droite 
            SetMotorDirection(true, 'L');
            SetMotorDirection(false, 'R');
            SetMotorPWM(800, 'L');
            SetMotorPWM(800, 'R');          
            break;

        case 'L':  // Tourner à gauche 
            SetMotorDirection(false, 'L');
            SetMotorDirection(true, 'R');
            SetMotorPWM(800, 'L');
            SetMotorPWM(800, 'R');
            break;

        case 'S':  // Stop 
            SetMotorPWM(0, 'L');
            SetMotorPWM(0, 'R');
            break;

        default:
            break;
    }
}



// Config UART
void UART_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);

	/* Output pins configuration, change M and N index! */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // Push - pull
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
	
	USART_InitStructure.USART_BaudRate 	              = 9600; //Set the baudrate. Hw many datas will be sent in a second.
	USART_InitStructure.USART_HardwareFlowControl     = USART_HardwareFlowControl_None;  //Do not check the data transmitted 
	USART_InitStructure.USART_Mode                    = USART_Mode_Tx | USART_Mode_Rx;//Data tansmission modes
	USART_InitStructure.USART_Parity                  = USART_Parity_No; // USART parity settings
	USART_InitStructure.USART_StopBits                = USART_StopBits_1; //Set the stop bits
	USART_InitStructure.USART_WordLength              = USART_WordLength_8b;  //we will send the 8bit data
	USART_Init(USART2, &USART_InitStructure);   //and started the this configuration

	USART_Cmd(USART2, ENABLE);
}

// Envoie une chaîne de caractères
void UART_SendString(char* str) {
    while (*str) {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
        USART_SendData(USART2, *str++);
    }
}

void usart_irq(void) {

	NVIC_InitTypeDef NVIC_InitStructure;
  
  	/* Enable the USARTx Interrupt */
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
