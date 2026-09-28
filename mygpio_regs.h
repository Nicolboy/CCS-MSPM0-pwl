#ifndef MYGPIO_REGS_H
#define MYGPIO_REGS_H

#include <stdint.h>

typedef struct {
    uint32_t RESERVED0[256];
    __IO uint32_t FSUB_0;                            /* !< (@ 0x00000400) Subscriber Port 0 */
    __IO uint32_t FSUB_1;                            /* !< (@ 0x00000404) Subscriber Port 1 */
    uint32_t RESERVED1[15];
    __IO uint32_t FPUB_0;                            /* !< (@ 0x00000444) Publisher Port 0 */
    __IO uint32_t FPUB_1;                            /* !< (@ 0x00000448) Publisher Port 1 */
    uint32_t RESERVED2[237];
    GPIO_GPRCM_Regs GPRCM;                          /* !< (@ 0x00000800) */
    uint32_t RESERVED3[510];
    __IO uint32_t CLKOVR;                           /* !< (@ 0x00001010) Clock Override */
    uint32_t RESERVED4;
    __IO uint32_t PDBGCTL;                          /* !< (@ 0x00001018) Peripheral Debug Control */
    uint32_t RESERVED5;
    GPIO_CPU_INT_Regs CPU_INT;                      /* !< (@ 0x00001020) */
    uint32_t RESERVED6;
    GPIO_GEN_EVENT0_Regs GEN_EVENT0;                /* !< (@ 0x00001050) */
    uint32_t RESERVED7;
    GPIO_GEN_EVENT1_Regs GEN_EVENT1;                /* !< (@ 0x00001080) */
    uint32_t RESERVED8[13];
    __IO uint32_t EVT_MODE;                         /* !< (@ 0x000010E0) Event Mode */
    uint32_t RESERVED9[6];
    __I uint32_t DESC;                              /* !< (@ 0x000010FC) Module Description */
    uint32_t RESERVED10[64];
    __O uint32_t DOUT3_0;                           /* !< (@ 0x00001200) Data output 3 to 0 */
    __O uint32_t DOUT7_4;                           /* !< (@ 0x00001204) Data output 7 to 4 */
    __O uint32_t DOUT11_8;                          /* !< (@ 0x00001208) Data output 11 to 8 */
    __O uint32_t DOUT15_12;                         /* !< (@ 0x0000120C) Data output 15 to 12 */
    __O uint32_t DOUT19_16;                         /* !< (@ 0x00001210) Data output 19 to 16 */
    __O uint32_t DOUT23_20;                         /* !< (@ 0x00001214) Data output 23 to 20 */
    __O uint32_t DOUT27_24;                         /* !< (@ 0x00001218) Data output 27 to 24 */
    __O uint32_t DOUT31_28;                         /* !< (@ 0x0000121C) Data output 31 to 28 */
    uint32_t RESERVED11[24];
    __IO uint32_t DOUT31_0;                          /* !< (@ 0x00001280) Data output 31 to 0 */
    uint32_t RESERVED12[3];
    __O uint32_t DOUTSET31_0;                       /* !< (@ 0x00001290) Data output set 31 to 0 */
    uint32_t RESERVED13[3];
    __O uint32_t DOUTCLR31_0;                       /* !< (@ 0x000012A0) Data output clear 31 to 0 */
    uint32_t RESERVED14[3];
    __O uint32_t DOUTTGL31_0;                       /* !< (@ 0x000012B0) Data output toggle 31 to 0 */
    uint32_t RESERVED15[3];
    __IO uint32_t DOE31_0;                           /* !< (@ 0x000012C0) Data output enable 31 to 0 */
    uint32_t RESERVED16[3];
    __O uint32_t DOESET31_0;                        /* !< (@ 0x000012D0) Data output enable set 31 to 0 */
    uint32_t RESERVED17[3];
    __O uint32_t DOECLR31_0;                        /* !< (@ 0x000012E0) Data output enable clear 31 to 0 */
    uint32_t RESERVED18[7];
    __I uint32_t DIN3_0;                            /* !< (@ 0x00001300) Data input 3 to 0 */
    __I uint32_t DIN7_4;                            /* !< (@ 0x00001304) Data input 7 to 4 */
    __I uint32_t DIN11_8;                           /* !< (@ 0x00001308) Data input 11 to 8 */
    __I uint32_t DIN15_12;                          /* !< (@ 0x0000130C) Data input 15 to 12 */
    __I uint32_t DIN19_16;                          /* !< (@ 0x00001310) Data input 19 to 16 */
    __I uint32_t DIN23_20;                          /* !< (@ 0x00001314) Data input 23 to 20 */
    __I uint32_t DIN27_24;                          /* !< (@ 0x00001318) Data input 27 to 24 */
    __I uint32_t DIN31_28;                          /* !< (@ 0x0000131C) Data input 31 to 28 */
    uint32_t RESERVED19[24];
    __I uint32_t DIN31_0;                           /* !< (@ 0x00001380) Data input 31 to 0 */
    uint32_t RESERVED20[3];
    __IO uint32_t POLARITY15_0;                     /* !< (@ 0x00001390) Polarity 15 to 0 */
    uint32_t RESERVED21[3];
    __IO uint32_t POLARITY31_16;                    /* !< (@ 0x000013A0) Polarity 31 to 16 */
    uint32_t RESERVED22[23];
    __IO uint32_t CTL;                              /* !< (@ 0x00001400) FAST WAKE GLOBAL EN */
    __IO uint32_t FASTWAKE;                        /* !< (@ 0x00001404) FAST WAKE ENABLE */
    uint32_t RESERVED23[62];
    __IO uint32_t SUB0CFG;                         /* !< (@ 0x00001500) Subscriber 0 configuration */
    uint32_t RESERVED24;
    __IO uint32_t FILTEREN15_0;                    /* !< (@ 0x00001508) Filter Enable 15 to 0 */
    __IO uint32_t FILTEREN31_16;                   /* !< (@ 0x0000150C) Filter Enable 31 to 16 */
    __IO uint32_t DMAMASK;                         /* !< (@ 0x00001510) DMA Write MASK */
    uint32_t RESERVED25[3];
    __IO uint32_t SUB1CFG;                         /* !< (@ 0x00001520) Subscriber 1 configuration */
} MyGPIO_Regs;

#define GPIOA_BASE (0x400A0000U) /*!< Base address of module GPIOA */
#define P1 ((GPIO_Regs *)GPIOA_BASE)

#endif // GPIO_REGS_H
