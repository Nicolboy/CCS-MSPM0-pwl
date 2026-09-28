/*
 * mymsp.c
 *
 *  Created on: 26 juil. 2024
 *      Author: Nicolas
 */

#ifndef MYMSP_H_
#define MYMSP_H_


#include <stdint.h>

// Définition des registres de base pour les périphériques
#define CS ((CS_Type *)0x40000000)
#define WDT_A ((WDT_A_Type *)0x40004800)
#define TIMER_A0 ((TIMER_A_Type *)0x40000000)

// Définition des types de structure pour les registres
typedef struct {
    volatile uint32_t KEY;
    volatile uint32_t CTL0;
    volatile uint32_t CTL1;
    volatile uint32_t CTL2;
    volatile uint32_t CTL3;
    volatile uint32_t CTL4;
    volatile uint32_t CTL5;
    volatile uint32_t CTL6;
    volatile uint32_t CTL7;
    volatile uint32_t CTL8;
    volatile uint32_t CTL9;
    volatile uint32_t CTL10;
    volatile uint32_t CTL11;
    volatile uint32_t CTL12;
    volatile uint32_t CTL13;
    volatile uint32_t CTL14;
    volatile uint32_t CTL15;
} CS_Type;

typedef struct {
    volatile uint16_t CTL;
    volatile uint16_t CNTCL;
    volatile uint16_t CTL1;
    volatile uint16_t CTL2;
} WDT_A_Type;

typedef struct {
    volatile uint8_t IN;
    volatile uint8_t OUT;
    volatile uint8_t DIR;
    volatile uint8_t REN;
    volatile uint8_t DS;
    volatile uint8_t SEL0;
    volatile uint8_t SEL1;
    volatile uint8_t SELC;
    volatile uint8_t IV;
} GPIO_Type;

typedef struct {
    volatile uint16_t CTL;
    volatile uint16_t CCTL[2];
    volatile uint16_t CCR[2];
    volatile uint16_t IV;
    volatile uint16_t EX0;
} TIMER_A_Type;

// Définition des bits pour les registres
#define CS_KEY_VAL 0xA596
#define CS_CTL0_DCORSEL_5 0x50
#define CS_CTL1_SELM__DCOCLK 0x01
#define CS_CTL1_SELA__DCOCLK 0x01
#define CS_CTL1_SELS__DCOCLK 0x01
#define CS_CTL2_DCOFSEL_6 0x60

#define WDT_A_CTL_PW 0x5A00
#define WDT_A_CTL_HOLD 0x80

#define TIMER_A_CTL_SSEL__SMCLK 0x02
#define TIMER_A_CTL_MC__UP 0x01
#define TIMER_A_CCTLN_OUTMOD_4 0x04

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80
#define BIT8 0x0100
#define BIT9 0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000
#define BIT16 0x00010000
#define BIT17 0x00020000
#define BIT18 0x00040000
#define BIT19 0x00080000
#define BIT20 0x00100000
#define BIT21 0x00200000
#define BIT22 0x00400000
#define BIT23 0x00800000
#define BIT24 0x01000000
#define BIT25 0x02000000
#define BIT26 0x04000000
#define BIT27 0x08000000
#define BIT28 0x10000000
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000

#endif /* MYMSP_H_ */
