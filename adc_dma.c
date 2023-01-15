#include "stm32f3xx.h"
#include "adc_dma.h"

volatile uint32_t adc_value;

void gpio_setup(void)                           //pin PA0
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;          // ABH enabling and GPIO A enabling
    __IO uint32_t tmpreg = RCC->AHBENR & (~RCC_AHBENR_GPIOAEN); 
    (void)tmpreg;
    GPIOA->MODER |= GPIO_MODER_MODER0;          // 11: Analog mode - for ADC1
}

void adc_setup(void) // ADC1 and 2 
{
    ADC12_COMMON->CCR |= ADC12_CCR_CKMODE_0;    // 0b01 // Set ADC clock
    ADC1->CFGR |= ADC_CFGR_DMAEN;               // DMA mode enable
    //ADC1->CFGR &= ~ADC_CFGR_DMACFG;             // DMA One Shot Mode selected

    ADC1->CFGR |= ADC_CFGR_DMACFG;              //1: DMA Circular Mode selected
    ADC1->CFGR &= ~ADC_CFGR_CONT;                // 0: Continuous conversion mode OFF
    ADC1->CR &= ~ADC_CR_ADEN;                   // disable the ADC.
    ADC1->CR |= ADC_CR_ADVREGEN_0;              // 01: ADC Voltage regulator enabled.

    ADC1->CR |= ADC_CR_ADCAL;                   // start ADC calibration cycle
    while (ADC1->CR & ADC_CR_ADCAL);            // wait for calibration to complete

    ADC1->SQR1 |= ADC_SQR1_SQ1_0;               // 1st conversion in regular sequence!!!

    // ADC12_COMMON->CCR |= ADC_CCR_DUAL_1;        //00110: Regular simultaneous mode only
    // ADC12_COMMON->CCR |= ADC_CCR_DUAL_2;        //00110: Regular simultaneous mode only

    ADC1->CR |= ADC_CR_ADEN;                    // enable the ADC.
    while (!(ADC1->ISR & ADC_ISR_ADRDY));       // check if enabled

  
}

void dma_setup(void)
{
	DMA1_Channel1->CPAR = (uint32_t)&(ADC1->DR);// 1. Set the peripheral register address in the DMA_CPARx register
	DMA1_Channel1->CMAR = (uint32_t)&adc_value;// 2. Set the memory address in the DMA_CMARx register
	DMA1_Channel1->CNDTR = 4096;                   // 3. Configure the total number of data to be transferred in the DMA_CNDTRx register
	DMA1_Channel1->CCR |= DMA_CCR_PL_1;          // 00: ensuring it is Low onfigure the channel priority using the PL[1:0]
	DMA1_Channel1->CCR &= ~DMA_CCR_MEM2MEM;     // 00: Low
	DMA1_Channel1->CCR |= DMA_CCR_MSIZE_0;      // 01: size 16bit
	DMA1_Channel1->CCR |= DMA_CCR_PSIZE_0;      // 01: size 16bit
	DMA1_Channel1->CCR |= DMA_CCR_MINC;         // 1 : memory increment enabled
	DMA1_Channel1->CCR &= ~DMA_CCR_PINC;        // 0: Peripheral increment mode disabled
	DMA1_Channel1->CCR |= DMA_CCR_CIRC;         // 1: Circ mode enabled
	DMA1_Channel1->CCR &= ~DMA_CCR_DIR;         // 0: read from peripherial
	DMA1_Channel1->CCR &= ~DMA_CCR_TEIE;        // 0: Transfer error interrupt disabled
	DMA1_Channel1->CCR &= ~DMA_CCR_HTIE;        // 0: Half transfer interrupt disabled
	DMA1_Channel1->CCR &= ~DMA_CCR_TCIE;        // 0: Transfer complete interrupt disabled
	
	DMA1_Channel1->CCR |= DMA_CCR_EN;           // Activate the channel in the DMA control register
    

}

void take_sample(void)
{
    
    ADC1->CR |= ADC_CR_ADSTART;                 //start conversion
    adc_value = ADC1->DR;

    ADC1->CR |= ADC_CR_ADSTP;                   //stop concersion

    // control DMA transmission!
}
