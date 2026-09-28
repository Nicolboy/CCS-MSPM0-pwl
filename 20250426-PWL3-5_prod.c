/*
 * PWL 3.5 - alimentation fantome 48 V sur batterie, MSPM0C1104
 *
 * Portage du firmware MSP430G2452/G2252 vers MSPM0C1104 en VSSOP-20.
 * Meme fonction : elevation vers 48 V par DC/DC polyphase, surveillance ADC
 * multivoies, etalonnage en flash, protection de batterie puis veille.
 *
 * Version MSP430 d'origine : github.com/Nicolboy/CCS-MSP430-pwl
 *
 * Copyright (c) 2024-2025 Nicolas Boyer
 * Licence MIT - voir LICENSE
 *
 * Journal du portage ci-dessous.
 */

/*
20240727-programme de migration MSP430g2452 vers MSPM0C1104vssop20 pour le PWL3
20240728-GPIO control ok
20240729 PWM pulses ok
20240731 ADC ok
20240802 timer intervalle ok
20240803 conversion MSPM0 complete, debuggage
 */

#include "ti_msp_dl_config.h"
//#include "mymsp.h"
//#include "mygpio_regs.h"

// alias des constantes

#define BIT0 0x01 // PA0
#define BIT1 0x02 // PA2
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10 //PA4
#define BIT5 0x20
#define BIT6 0x40 //PA6
#define BIT7 0x80
#define BIT8 0x0100
#define BIT9 0x0200
#define BIT10 0x0400
#define BIT11 0x0800 // PA11
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000
#define BIT16 0x00010000 // PA16
#define BIT17 0x00020000 // PA17
#define BIT18 0x00040000 // PA18
#define BIT19 0x00080000
#define BIT20 0x00100000
#define BIT21 0x00200000
#define BIT22 0x00400000 // PA22
#define BIT23 0x00800000 // PA23
#define BIT24 0x01000000
#define BIT25 0x02000000
#define BIT26 0x04000000 // PA26
#define BIT27 0x08000000 // PA28
#define BIT28 0x10000000 // PA27
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000

// definition de la periode mini des timers
#define timer_period_10 135
#define timer_period_48 270 //270
#define timer_on_time_low_48 20 // 20 valeur de PWM soft start
#define timer_on_time_low_10 38 // 30 valeur de PWM  mode basse conso 5,6V @ 0,9V
#define timer_on_time_10 133 //43 valeur nominale impulsion synchro PWM
#define timer_on_time_48 268 //268 43 valeur nominale impulsion synchro PWM
/* This results in approximately 0.5s of delay assuming 24MHz CPU_CLK */
#define DELAY (12000)

# define V48_Osc BIT26 // PA26 O PWM TIMG
# define Led_R BIT28 // PA28 O
//# define Led BIT28 // PA27 O pas dispo
# define Led_B BIT17 // PA17  O
//# define Audio_I_dis BIT4 // PA4 O
# define Audio_O_dis BIT6 // PA6 O
//# define V10_Osc_sw BIT11 // PA11 O
//# define V10_Osc_dis BIT16 // PA16 O
# define V10_Osc_dis BIT0 // PA0 O
# define V10_Osc_sw BIT18 // PA18 O
# define Audio_I_dis BIT22 // PA22 O
# define V48_Osc_dis BIT23 // PA 23 O
# define V10_Osc BIT2 // PA2 O PWM TIMA
# define V48_ADC BIT1 // PA24 ADC2 I
# define Vbatt_ADC BIT1 // PA25 ADC3 I

# define gADCOffset 2.5 // ADC offset

# define led_start_fast_on      1 // temporisation led en unité de periode 16ms
# define led_start_fast_off     2
# define led_start_slow_on      1
# define led_start_slow_off     6
# define led_run_slow_on        150
# define led_run_slow_off       600
# define led_run_fast_on        30
# define led_run_fast_off       30

// constante de tension ADC
#define     V48_ok              560             //+41V OK ref 2,5V 1M/47k
#define     Vbatt_ok            440                 //low batt 1,07v=438 1,1=450
#define     Vbatt_low           415                 //low batt 1V=410 1,03=420
#define     Vbatt_stop          385                 //stop batt 0,95V=388

// variables
volatile uint16_t i,j;
volatile  uint16_t Vbatt;
volatile  uint16_t V48;
volatile bool gCheckADC;
volatile  uint16_t Tempo = 1;                                 // temporisation  clignotement LED
volatile  uint16_t Toff = 1;                                 // temporisation bascule clignotement LED
//volatile int16_t gADCOffset;


//***************************** structures de controle
// structure de controle du l'horloge du timer
/*
* Timer clock configuration to be sourced by  / 1 (32000000 Hz)
* timerClkFreq = (MFCLK 4MHz / (timerClkDivRatio * (timerClkPrescale + 1)))
*   4MHz = 4 000 000 Hz / (1 * (0 + 1))
*/
   static const DL_TimerA_ClockConfig gPWM_0ClockConfig = {
   .clockSel = DL_TIMER_CLOCK_BUSCLK,//DL_TIMER_CLOCK_MFCLK DL_TIMER_CLOCK_BUSCLK
   .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
   .prescale = 0U
   };
// structure de controle de timer
   static const DL_TimerA_PWMConfig gPWM_0Config = {
       .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
       .period = timer_period_10, // 45 => 88kHz @ 4MHz MFCLK
       .startTimer = DL_TIMER_STOP,
   };
   /*
    * Timer clock configuration to be sourced by  / 1 (4000000 Hz)
    * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
    *   4000000 Hz = 4000000 Hz / (1 * (0 + 1))
    */
   static const DL_TimerG_ClockConfig gPWM_1ClockConfig = {
       .clockSel = DL_TIMER_CLOCK_BUSCLK,//DL_TIMER_CLOCK_MFCLK
       .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
       .prescale = 0U
   };

   static const DL_TimerG_PWMConfig gPWM_1Config = {
       .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
       .period = timer_period_48,
       .startTimer = DL_TIMER_STOP,
   };

   /* ADC12_0 Initialization */
   static const DL_ADC12_ClockConfig gADC12_0ClockConfig = {
       .clockSel       = DL_ADC12_CLOCK_ULPCLK,
       .divideRatio    = DL_ADC12_CLOCK_DIVIDE_1, // 55us pour une conversion
       .freqRange      = DL_ADC12_CLOCK_FREQ_RANGE_20_TO_24,
   };

   /* Vref Initialization */
   static const DL_VREF_ClockConfig gVREFClockConfig = {
       .clockSel = DL_VREF_CLOCK_LFCLK,
       .divideRatio = DL_VREF_CLOCK_DIVIDE_1,
   };
   static const DL_VREF_Config gVREFConfig = {
       .vrefEnable     = DL_VREF_ENABLE_ENABLE,
       .bufConfig      = DL_VREF_BUFCONFIG_OUTPUT_2_5V,
       .shModeEnable   = DL_VREF_SHMODE_DISABLE,
       .holdCycleCount = DL_VREF_HOLD_MIN,
       .shCycleCount   = DL_VREF_SH_MIN,
   };

   /*
    * Timer clock configuration to be sourced by LFCLK /  (16384 Hz)
    * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
    *   64 Hz = 16384 Hz / (2 * (255 + 1))
    */
   static const DL_TimerG_ClockConfig gTIMER_0ClockConfig = {
                                                        // unité de periode de 16ms
       .clockSel    = DL_TIMER_CLOCK_LFCLK,
       .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
       .prescale    = 255U,
   };

   /*
    * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
    * TIMER_0_INST_LOAD_VALUE = (4000 ms * 64 Hz) - 1
    */
   static const DL_TimerG_TimerConfig gTIMER_0TimerConfig = {
       .period     = 10U,
       .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
       .startTimer = DL_TIMER_STOP,
   };

   // ******************* déclaration des fonctions
void GPIO_Init(void);
void Power_Init(void);
void TIMER_PWM_Init(void);
void ADC10_init(void);
void VREF_init(void);
void ADC_conversion(void);
void SYSCTL_init(void);
void TIMER_tempo_init(void);
void TIMER_tempo_run(uint16_t);
void softstart (void);
void shutdown(void);
void V10_OSC_switch (void);
void main_routine(void);

int main(void)
{

    // Stop watchdog timer
//    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD; //

//    __WFI();

    // Initialize peripherals power and GPIO
    Power_Init();
    GPIO_Init();
    DL_GPIO_setPins(GPIOA, Audio_O_dis); // mute output
    DL_GPIO_setPins(GPIOA, Audio_I_dis); // mute input

    // initialisation des timers pour oscillateurs
    TIMER_PWM_Init();
    DL_TimerA_startCounter(TIMA0); // démarrage V10_Osc
    DL_TimerG_startCounter(TIMG14); // démarrage V48_Osc

    // basculer sur l'oscillateur V10 secondaire
    V10_OSC_switch ();

    // power audio input
    DL_GPIO_clearPins(GPIOA, Audio_I_dis);      // Audio input active

    // initialisation du timer d'intervalle
    SYSCTL_init();  // low power mode policy
    TIMER_tempo_init(); //

    // initialisation ADC
//    DL_GPIO_togglePins(GPIOA, BIT22);
    VREF_init();
//    DL_GPIO_togglePins(GPIOA, BIT22);
    ADC10_init();
//    DL_GPIO_togglePins(GPIOA, BIT22);

    ADC_conversion(); // a remettre dans la boucle principale


    // démarrage visuel
    softstart();

    ADC_conversion(); // a remettre dans la boucle principale just in case

    // unmute audio output
    DL_GPIO_clearPins(GPIOA, Audio_O_dis);      // Audio output active

    // ************* routine principale

    main_routine(); // fonctionnement du PWL

    // **************** arret du système
    shutdown(); // arret fin de batterie

}

// ********************** routines interruptions et events
void TIMG8_IRQHandler(void)
{ // timer intervalle
    switch (DL_TimerG_getPendingInterrupt(TIMG8)) {
        case DL_TIMER_IIDX_ZERO:
//            DL_TimerG_stopCounter(TIMG8);
//            DL_GPIO_togglePins(GPIOA, Test);
            if (j>Toff)
                {
                DL_GPIO_clearPins(GPIOA, Led_R | Led_B); // all leds off
//                DL_GPIO_clearPins(GPIOA, Test);
                }
                if (j == Tempo)
                {
                j = 0;
                DL_SYSCTL_disableSleepOnExit();        // go out of sleep after interrupt ??????????
                DL_TimerG_stopCounter(TIMG8);
//                DL_GPIO_togglePins(GPIOA, Test);
                }
                else
                {

                }
                j++;
            break;
        default:
            break;
    }
}

void ADC0_IRQHandler(void)
{
    switch (DL_ADC12_getPendingInterrupt(ADC0)) {
        case DL_ADC12_IIDX_MEM1_RESULT_LOADED:
            gCheckADC = true;
//            DL_GPIO_togglePins(GPIOA, BIT22);
            break;
        default:
//            DL_GPIO_togglePins(GPIOA, BIT22);
            break;
    }
}


//*********************** routines

void SYSCTL_init(void)
{

    //Low Power Mode is configured to be STANDBY0??0
//    DL_SYSCTL_setPowerPolicySTANDBY0();
    // config supplementaire
    //Low Power Mode is configured to be STOP0

//    DL_SYSCTL_setPowerPolicySTOP0(); // **********************************
    DL_SYSCTL_setPowerPolicyRUN0SLEEP0();
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);
    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);

}
void Power_Init(void)
{
    // alimenter les périphériques utilisés
    DL_GPIO_reset(GPIOA);
    DL_TimerG_reset(TIMG8);
    DL_TimerA_reset(TIMA0); // reset Timer
    DL_TimerG_reset(TIMG14); // reset Timer
    DL_VREF_reset(VREF);
    DL_ADC12_reset(ADC0);

    DL_SYSCTL_enableMFCLK(); // activer l'horloge MFCLK 4MHz

    DL_GPIO_enablePower(GPIOA);
    DL_TimerG_enablePower(TIMG8); // alimenter le timer G d'intervalle
    DL_TimerA_enablePower(TIMA0); // alimenter le timer A PWM 10V
    DL_TimerG_enablePower(TIMG14); // alimenter le Timer G PWM 48V
    DL_VREF_enablePower(VREF);
    DL_ADC12_enablePower(ADC0);

    delay_cycles(POWER_STARTUP_DELAY); // wait for power to settle

}

void GPIO_Init(void)
{ uint32_t SORTIES;

    // concaténation des GPIO sortie
    SORTIES = V48_Osc | V10_Osc | Led_R | Led_B | Audio_I_dis | Audio_O_dis  | V48_Osc_dis | V10_Osc_dis | V10_Osc_sw ; // ensemble des sorties GPIO

    // configurer en E/S
    //DL_GPIO_initDigitalOutputFeatures(IOMUX_PINCM3, DL_GPIO_INVERSION_ENABLE, DL_GPIO_RESISTOR_NONE, DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE); // inverse la polarité de la sortie
    // GPIO sortie simples IOMUX_PINCM23
//    DL_GPIO_initDigitalOutput(IOMUX_PINCM28); // pin2 pincm28 PA27 Led_R
    DL_GPIO_initDigitalOutput(IOMUX_PINCM29); // pin2 pincm29 PA28 Led_R
    DL_GPIO_initDigitalOutput(IOMUX_PINCM1); // pin3 pincm1 PA0 Led_B
//  DL_GPIO_initDigitalOutput(IOMUX_PINCM5); //  pincm5 PA4 Audio_I_dis pas dispo
    DL_GPIO_initDigitalOutput(IOMUX_PINCM7); // pin8 pincm7 PA6 Audio_O_dis
//    DL_GPIO_initDigitalOutput(IOMUX_PINCM12); // pincm12 PA11 V10_Osc_sw pas dispo
//    DL_GPIO_initDigitalOutput(IOMUX_PINCM17); // pincm17 PA16 V10_Osc_dis pas dispo
    DL_GPIO_initDigitalOutput(IOMUX_PINCM18); // pin13 pincm18 PA17 V10_Osc_dis
//    DL_GPIO_initDigitalOutputFeatures(IOMUX_PINCM18, DL_GPIO_INVERSION_DISABLE,
//        DL_GPIO_RESISTOR_NONE, DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_ENABLE); // pin13 pincm18 PA17 V10_low, output: no invert,no pull resistor,no high drive, High Z
    DL_GPIO_initDigitalOutput(IOMUX_PINCM19); // pin14 pincm19 PA18 V10_Osc_sw
//    DL_GPIO_initDigitalOutputFeatures(IOMUX_PINCM19, DL_GPIO_INVERSION_DISABLE,
//            DL_GPIO_RESISTOR_NONE, DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_ENABLE); // pin14 pincm19 PA18 V48_low, output: no invert,no pull resistor,no high drive, High Z
    DL_GPIO_initDigitalOutput(IOMUX_PINCM23); // pin17 pincm23 PA22 Audio_I_dis
    DL_GPIO_initDigitalOutput(IOMUX_PINCM24); // pin18 pincm24 PA23 V48_dis

    // sorties fonctions spéciales
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM1,0); // pin3 pincm1 PA0 fonction 2 BEEP fonction 0 sortie?

    // entrées ADC
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM25,0); // pin19 pincm25 PA24 fonction 0 Hiz input
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM26,0); // pin20 pincm26 PA25 fonction 0 Hiz input

    // initialisation des sorties à 0
    DL_GPIO_clearPins(GPIOA, SORTIES);
//    DL_GPIO_setPins(GPIOA, V10_low | V48_low); //  High Z if set, out low if cleared
//    DL_GPIO_setPins(GPIOA, V10_low | V48_low);
//    DL_GPIO_enableOutput(GPIOA, V10_low | V48_low);
//    DL_GPIO_setPins(GPIOA, pin);
//    DL_GPIO_togglePins(GPIOA, pin);
    DL_GPIO_enableOutput(GPIOA, SORTIES);
//
//    DL_GPIO_setPins(GPIOA, Led_R);
//    DL_GPIO_setPins(GPIOA, Led_B);
//    while(1);

}
void TIMER_PWM_Init( void )
{

    // configuration des broches en sorties PWM
    // V10_Osc
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM27,6); // pin1 pincm27 PA26 fonction 2 PWM TIMG14_C0
    DL_GPIO_enableOutput(GPIOA, V48_Osc);
    // V48_Osc
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM3,4); // pin8 pincm3 PA2 fonction 4 PWM TIMA0_C0
    DL_GPIO_enableOutput(GPIOA, V10_Osc);

    // configuration des registre  PWM
    // V10_Osc
    DL_TimerA_setClockConfig(TIMA0, (DL_TimerG_ClockConfig *) &gPWM_0ClockConfig);
    DL_TimerA_initPWMMode(TIMA0, (DL_TimerA_PWMConfig *) &gPWM_0Config);
    DL_TimerA_setCaptureCompareOutCtl(TIMA0, DL_TIMER_CC_OCTL_INIT_VAL_LOW, DL_TIMER_CC_OCTL_INV_OUT_ENABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL, DL_TIMER_CC_0_INDEX); // inversion polarité PWM
    DL_TimerA_setCaptureCompareValue(TIMA0, timer_on_time_10, DL_TIMER_CC_0_INDEX); // impulsion la plus courte possible, pourquoi? pwm mode DL_TIMER_PWM_MODE_EDGE_ALIGN
    DL_TimerA_enableClock(TIMA0);
    DL_TimerA_setCCPDirection(TIMA0, DL_TIMER_CC0_OUTPUT);
    // V48_Osc
    DL_TimerG_setClockConfig(TIMG14, (DL_TimerG_ClockConfig *) &gPWM_1ClockConfig);
    DL_TimerG_initPWMMode(TIMG14, (DL_TimerG_PWMConfig *) &gPWM_1Config);
    DL_TimerG_setCaptureCompareOutCtl(TIMG14, DL_TIMER_CC_OCTL_INIT_VAL_LOW, DL_TIMER_CC_OCTL_INV_OUT_ENABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL, DL_TIMERG_CAPTURE_COMPARE_0_INDEX); // inversion polarité PWM
    DL_TimerG_setCaptCompUpdateMethod(TIMG14, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERG_CAPTURE_COMPARE_0_INDEX);
    DL_TimerG_setCaptureCompareValue(TIMG14, timer_on_time_low_48, DL_TIMER_CC_0_INDEX); // soft start
    DL_TimerG_enableClock(TIMG14); //
    DL_TimerG_setCCPDirection(TIMG14, DL_TIMER_CC0_OUTPUT);
}

void VREF_init(void) {
    DL_VREF_setClockConfig(VREF,
        (DL_VREF_ClockConfig *) &gVREFClockConfig);
    DL_VREF_configReference(VREF,
        (DL_VREF_Config *) &gVREFConfig);
}

void ADC10_init(void)
{
    DL_ADC12_setClockConfig(ADC0, (DL_ADC12_ClockConfig *) &gADC12_0ClockConfig);

        DL_ADC12_initSeqSample(ADC0,
            DL_ADC12_REPEAT_MODE_DISABLED, DL_ADC12_SAMPLING_SOURCE_AUTO, DL_ADC12_TRIG_SRC_SOFTWARE,
            DL_ADC12_SEQ_START_ADDR_00, DL_ADC12_SEQ_END_ADDR_01, DL_ADC12_SAMP_CONV_RES_10_BIT,
            DL_ADC12_SAMP_CONV_DATA_FORMAT_UNSIGNED);

        DL_ADC12_configConversionMem(ADC0, DL_ADC12_MEM_IDX_0,
            DL_ADC12_INPUT_CHAN_2, DL_ADC12_REFERENCE_VOLTAGE_INTREF, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED,
            DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);

        DL_ADC12_configConversionMem(ADC0, DL_ADC12_MEM_IDX_1,
            DL_ADC12_INPUT_CHAN_3, DL_ADC12_REFERENCE_VOLTAGE_INTREF, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED,
            DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);

        DL_ADC12_setPowerDownMode(ADC0,DL_ADC12_POWER_DOWN_MODE_MANUAL);
        DL_ADC12_setSampleTime0(ADC0,375);

        /* Enable ADC12 interrupt */
        DL_ADC12_clearInterruptStatus(ADC0,(DL_ADC12_INTERRUPT_MEM1_RESULT_LOADED));
        DL_ADC12_enableInterrupt(ADC0,(DL_ADC12_INTERRUPT_MEM1_RESULT_LOADED));
        DL_ADC12_enableConversions(ADC0);

        /* Get calibrated ADC offset - workaround for ADC_ERR_06 */
//        gADCOffset =
//            DL_ADC12_getADCOffsetCalibration(ADC12_0_ADCMEM_0_REF_VOLTAGE_V);

        NVIC_EnableIRQ(ADC0_INT_IRQn);
        gCheckADC = false;
}

void ADC_conversion(void)
{
    DL_ADC12_startConversion(ADC0);

    while (false == gCheckADC)
    {
        __WFI();
    }

    Vbatt = DL_ADC12_getMemResult(ADC0, DL_ADC12_MEM_IDX_0);
    V48 = DL_ADC12_getMemResult(ADC0, DL_ADC12_MEM_IDX_1);

    /* Apply calibrated ADC offset - workaround for ADC_ERR_06 */
    int16_t adcRaw = (int16_t) Vbatt + gADCOffset;
    if (adcRaw < 0) {
        adcRaw = 0;
    }
    if (adcRaw > 4095) {
        adcRaw = 4095;
    }
    Vbatt = (uint16_t) adcRaw;

    adcRaw = V48 + gADCOffset;
    if (adcRaw < 0) {
        adcRaw = 0;
    }
    if (adcRaw > 4095) {
        adcRaw = 4095;
    }
    V48 = (uint16_t) adcRaw;

    if (Vbatt > 0x1ff) {
//                DL_GPIO_clearPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
    } else {
//                DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
    }
    gCheckADC = false;
    DL_ADC12_enableConversions(ADC0);
}

void TIMER_tempo_init(void) {

    DL_TimerG_setClockConfig(TIMG8,
        (DL_TimerG_ClockConfig *) &gTIMER_0ClockConfig);

    DL_TimerG_initTimerMode(TIMG8,
        (DL_TimerG_TimerConfig *) &gTIMER_0TimerConfig);
    DL_TimerG_enableInterrupt(TIMG8 , DL_TIMERG_INTERRUPT_ZERO_EVENT);
    DL_TimerG_enableClock(TIMG8);
    NVIC_EnableIRQ(TIMG8_INT_IRQn);

}

void TIMER_tempo_run(uint16_t tempo) {

    DL_SYSCTL_enableSleepOnExit();
    DL_Timer_setLoadValue(TIMG8, tempo);
    DL_TimerG_startCounter(TIMG8);

}

void softstart (void)
{
    // start 48V
      DL_GPIO_clearPins(GPIOA, V48_Osc_dis);    // start the driver
//      DL_GPIO_clearPins(GPIOA, V48_low);       // soft start 48V

    // ********************** demarrage led clignotent 10 fois chacune, deux fois
      DL_GPIO_setPins(GPIOA, Led_R);
//      DL_GPIO_setPins(GPIOA, Test);


    // demarrage phase rapide
    Tempo = led_start_fast_on + led_start_fast_off;
    Toff = led_start_fast_on;
     for ( i = 24; i > 0; i--)
     {
         TIMER_tempo_run(1);
         __WFI();
        DL_GPIO_togglePins(GPIOA, Led_R);
//        DL_GPIO_togglePins(GPIOA, Test);
//        if ( timer_on_time + i < timer_on_time + 1 )

            DL_TimerG_setCaptureCompareValue(TIMG14, timer_on_time_48 - i, DL_TIMER_CC_0_INDEX);

     }
//     while(1);

    DL_GPIO_clearPins(GPIOA, Led_R);
    DL_GPIO_setPins(GPIOA, Led_B);
//    DL_GPIO_setPins(GPIOA, Test);

//    DL_GPIO_setPins(GPIOA, V48_low);       // soft start 48V

     for ( i = 24; i > 0; i--)
     {
         TIMER_tempo_run(1);
         __WFI();
        DL_GPIO_togglePins(GPIOA, Led_B);
//        DL_GPIO_togglePins(GPIOA, Test);
     }
     DL_GPIO_clearPins(GPIOA, Led_R | Led_B);
//     DL_GPIO_clearPins(GPIOA, Test);

    // demarrage phase lente
    Tempo = led_start_slow_on + led_start_slow_off;
    Toff = led_start_slow_on;
    DL_GPIO_setPins(GPIOA, Led_R);
//    DL_GPIO_setPins(GPIOA, Test);

     for ( i = 8; i > 0; i--)
     {
         TIMER_tempo_run(1);
         __WFI();
        DL_GPIO_togglePins(GPIOA, Led_R);
//        DL_GPIO_togglePins(GPIOA, Test);
     }

     DL_GPIO_clearPins(GPIOA, Led_R);
     DL_GPIO_setPins(GPIOA, Led_B);
//     DL_GPIO_setPins(GPIOA, Test);

     for ( i = 8; i > 0; i--)
     {
         TIMER_tempo_run(1);
         __WFI();
        DL_GPIO_togglePins(GPIOA, Led_B);
//        DL_GPIO_togglePins(GPIOA, Test);
     }

     DL_TimerG_stopCounter(TIMG8); // stop the interval timer

    DL_GPIO_clearPins(GPIOA, Led_R | Led_B); // all leds off
//    DL_GPIO_clearPins(GPIOA, Test);

    DL_TimerG_setCaptureCompareValue(TIMG14, timer_on_time_48, DL_TIMER_CC_0_INDEX);

}

void shutdown(void)
{
  //************************** stop V48 and audio if Vbatt is too low, V10 to +5V
    DL_GPIO_setPins(GPIOA, Led_R | Audio_O_dis); //red batt LED , mute audio output
    DL_GPIO_clearPins(GPIOA, Led_B);
//    DL_GPIO_initDigitalOutput(IOMUX_PINCM19); // pin14 pincm19 PA18 V48_low
//    DL_GPIO_clearPins(GPIOA, V10_low | V48_low); // force PWM V48 to relax and 10V to 5V
    DL_GPIO_setPins(GPIOA, V48_Osc_dis);  //disable PWM V48 driver
    DL_TimerA_setCaptureCompareValue(TIMA0, timer_on_time_low_10, DL_TIMER_CC_0_INDEX);
    DL_TimerG_setCaptureCompareValue(TIMG14, timer_on_time_low_48, DL_TIMER_CC_0_INDEX);
    DL_GPIO_setPins(GPIOA, Audio_I_dis); // stop audio inputs

  // shutdown processor
    __WFI();

}

void V10_OSC_switch (void)
{
    //******************************************************* change V10 oscillator
    DL_GPIO_initDigitalOutput(IOMUX_PINCM12);               // pin11 pincm12 PA11 V10_Osc_sw
    DL_GPIO_initDigitalOutput(IOMUX_PINCM1); // pin3 pincm1 PA0 V10_Osc_dis
    DL_GPIO_clearPins(GPIOA, V10_Osc_sw | V10_Osc_dis);     // SWITCH 5V oscillator, low to switch and activate V10 PWM driver
    DL_GPIO_enableOutput(GPIOA, V10_Osc_sw | V10_Osc_dis);
    delay_cycles(1000);
}

void main_routine(void)
{
    // ************************************** main loop
      while (1)
      {
          ADC_conversion();

    //************************************* calcul des valeurs

           if (Vbatt<Vbatt_stop) break;                        // stop, jump out of the main loop

            if (Vbatt>Vbatt_ok)                                  // compare ADC input P1.0 to VBATT
            {
                DL_GPIO_clearPins(GPIOA, Led_R);
            }
            else
            {
                DL_GPIO_setPins(GPIOA, Led_R);
            }

            if (V48>V48_ok && Vbatt>Vbatt_ok)                 // compare ADC input P1.1 to VBATT
            {
                DL_GPIO_setPins(GPIOA, Led_B);
            }
            else
            {
                DL_GPIO_clearPins(GPIOA, Led_B);
            }

    //************************************** clignotement a 2 vitesses

            if (Vbatt>Vbatt_low)
                { // clignotement lent
                Tempo = led_run_slow_on + led_run_slow_off;
                Toff = led_run_slow_on;
                }
            else
                { // clignotement rapide
                Tempo = led_run_fast_on + led_run_fast_off;
                Toff = led_run_fast_on;
                }
            TIMER_tempo_run(1);
            __WFI();

      }
}
