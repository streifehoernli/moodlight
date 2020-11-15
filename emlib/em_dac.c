/***************************************************************************/
#include "em_dac.h"
#if defined(DAC_COUNT) && (DAC_COUNT > 0)
#include "em_cmu.h"
#include "em_assert.h"
#include "em_bus.h"

/***************************************************************************/
/***************************************************************************/
/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#define DAC_CH_VALID(ch)    ((ch) <= 1)

#define DAC_MAX_CLOCK    1000000

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************/
void DAC_Enable(DAC_TypeDef *dac, unsigned int ch, bool enable)
{
  volatile uint32_t *reg;

  EFM_ASSERT(DAC_REF_VALID(dac));
  EFM_ASSERT(DAC_CH_VALID(ch));

  if (!ch)
  {
    reg = &(dac->CH0CTRL);
  }
  else
  {
    reg = &(dac->CH1CTRL);
  }

  BUS_RegBitWrite(reg, _DAC_CH0CTRL_EN_SHIFT, enable);
}


/***************************************************************************/
void DAC_Init(DAC_TypeDef *dac, const DAC_Init_TypeDef *init)
{
  uint32_t tmp;

  EFM_ASSERT(DAC_REF_VALID(dac));

  /* Make sure both channels are disabled. */
  BUS_RegBitWrite(&(dac->CH0CTRL), _DAC_CH0CTRL_EN_SHIFT, 0);
  BUS_RegBitWrite(&(dac->CH1CTRL), _DAC_CH0CTRL_EN_SHIFT, 0);

  /* Load proper calibration data depending on selected reference */
  switch (init->reference)
  {
    case dacRef2V5:
      dac->CAL = DEVINFO->DAC0CAL1;
      break;

    case dacRefVDD:
      dac->CAL = DEVINFO->DAC0CAL2;
      break;

    default: /* 1.25V */
      dac->CAL = DEVINFO->DAC0CAL0;
      break;
  }

  tmp = ((uint32_t)(init->refresh)     << _DAC_CTRL_REFRSEL_SHIFT)
        | (((uint32_t)(init->prescale) << _DAC_CTRL_PRESC_SHIFT)
           & _DAC_CTRL_PRESC_MASK)
        | ((uint32_t)(init->reference) << _DAC_CTRL_REFSEL_SHIFT)
        | ((uint32_t)(init->outMode)   << _DAC_CTRL_OUTMODE_SHIFT)
        | ((uint32_t)(init->convMode)  << _DAC_CTRL_CONVMODE_SHIFT);

  if (init->ch0ResetPre)
  {
    tmp |= DAC_CTRL_CH0PRESCRST;
  }

  if (init->outEnablePRS)
  {
    tmp |= DAC_CTRL_OUTENPRS;
  }

  if (init->sineEnable)
  {
    tmp |= DAC_CTRL_SINEMODE;
  }

  if (init->diff)
  {
    tmp |= DAC_CTRL_DIFF;
  }

  dac->CTRL = tmp;
}


/***************************************************************************/
void DAC_InitChannel(DAC_TypeDef *dac,
                     const DAC_InitChannel_TypeDef *init,
                     unsigned int ch)
{
  uint32_t tmp;

  EFM_ASSERT(DAC_REF_VALID(dac));
  EFM_ASSERT(DAC_CH_VALID(ch));

  tmp = (uint32_t)(init->prsSel) << _DAC_CH0CTRL_PRSSEL_SHIFT;

  if (init->enable)
  {
    tmp |= DAC_CH0CTRL_EN;
  }

  if (init->prsEnable)
  {
    tmp |= DAC_CH0CTRL_PRSEN;
  }

  if (init->refreshEnable)
  {
    tmp |= DAC_CH0CTRL_REFREN;
  }

  if (ch)
  {
    dac->CH1CTRL = tmp;
  }
  else
  {
    dac->CH0CTRL = tmp;
  }
}


/***************************************************************************/
void DAC_ChannelOutputSet( DAC_TypeDef *dac,
                           unsigned int channel,
                           uint32_t     value )
{
  switch(channel)
  {
    case 0:
      DAC_Channel0OutputSet(dac, value);
      break;
    case 1:
      DAC_Channel1OutputSet(dac, value);
      break;
    default:
      EFM_ASSERT(0);
      break;
  }
}


/***************************************************************************/
uint8_t DAC_PrescaleCalc(uint32_t dacFreq, uint32_t hfperFreq)
{
  uint32_t ret;

  /* Make sure selected DAC clock is below max value */
  if (dacFreq > DAC_MAX_CLOCK)
  {
    dacFreq = DAC_MAX_CLOCK;
  }

  /* Use current HFPER frequency? */
  if (!hfperFreq)
  {
    hfperFreq = CMU_ClockFreqGet(cmuClock_HFPER);
  }

  /* Iterate in order to determine best prescale value. Only a few possible */
  /* values. We start with lowest prescaler value in order to get first */
  /* equal or below wanted DAC frequency value. */
  for (ret = 0; ret <= (_DAC_CTRL_PRESC_MASK >> _DAC_CTRL_PRESC_SHIFT); ret++)
  {
    if ((hfperFreq >> ret) <= dacFreq)
      break;
  }

  /* If ret is higher than the max prescaler value, make sure to return
     the max value. */
  if (ret > (_DAC_CTRL_PRESC_MASK >> _DAC_CTRL_PRESC_SHIFT))
  {
    ret = _DAC_CTRL_PRESC_MASK >> _DAC_CTRL_PRESC_SHIFT;
  }

  return (uint8_t)ret;
}


/***************************************************************************/
void DAC_Reset(DAC_TypeDef *dac)
{
  /* Disable channels, before resetting other registers. */
  dac->CH0CTRL  = _DAC_CH0CTRL_RESETVALUE;
  dac->CH1CTRL  = _DAC_CH1CTRL_RESETVALUE;
  dac->CTRL     = _DAC_CTRL_RESETVALUE;
  dac->IEN      = _DAC_IEN_RESETVALUE;
  dac->IFC      = _DAC_IFC_MASK;
  dac->CAL      = DEVINFO->DAC0CAL0;
  dac->BIASPROG = _DAC_BIASPROG_RESETVALUE;
  /* Do not reset route register, setting should be done independently */
}


#endif /* defined(DAC_COUNT) && (DAC_COUNT > 0) */
