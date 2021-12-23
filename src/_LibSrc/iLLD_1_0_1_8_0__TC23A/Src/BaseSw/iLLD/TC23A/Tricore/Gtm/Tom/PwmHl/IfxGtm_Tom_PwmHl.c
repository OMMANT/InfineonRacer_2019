/**
 * \file IfxGtm_Tom_PwmHl.c
 * \brief GTM PWMHL details
 *
 * \version iLLD_1_0_1_8_0
 * \copyright Copyright (c) 2018 Infineon Technologies AG. All rights reserved.
 *
 *
 *                                 IMPORTANT NOTICE
 *
 *
 * Infineon Technologies AG (Infineon) is supplying this file for use
 * exclusively with Infineon's microcontroller products. This file can be freely
 * distributed within development tools that are supporting such microcontroller
 * products.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 */

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/

#include "IfxGtm_Tom_PwmHl.h"
#include "_Utilities/Ifx_Assert.h"
#include "stddef.h"
#include "string.h"

/** \addtogroup IfxLld_Gtm_Tom_PwmHl_PwmHl_StdIf_Functions
 * \{ */
/******************************************************************************/
/*------------------------Inline Function Prototypes--------------------------*/
/******************************************************************************/

/** \brief Inverts the active state
 * \param activeState Active state
 * \return State
 */
IFX_INLINE Ifx_ActiveState IfxGtm_Tom_PwmHl_invertActiveState(Ifx_ActiveState activeState);

/******************************************************************************/
/*-----------------------Private Function Prototypes--------------------------*/
/******************************************************************************/

/** \brief Sets switched to OFF
 * \param driver GTM TOM PWM driver
 * \param tOn ON time
 * \param shift Shift value in ticks
 * \return None
 */
IFX_STATIC void IfxGtm_Tom_PwmHl_updateAndShiftOff(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *shift);

/** \brief Updates the x output duty cycle in center aligned and center aligned inverted modes
 * \param driver GTM TOM PWM driver
 * \param tOn T on
 * \return None
 */
IFX_STATIC void IfxGtm_Tom_PwmHl_updateCenterAligned(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);

/** \brief Updates the x output duty cycle in edge aligned modes (left and right aligned)
 * \param driver GTM TOM PWM driver
 * \param tOn T on
 * \return None
 */
IFX_STATIC void IfxGtm_Tom_PwmHl_updateEdgeAligned(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);

/** \brief Set the outputs to inactive
 * \param driver GTM TOM PWM driver
 * \param tOn T on
 * \return None
 */
IFX_STATIC void IfxGtm_Tom_PwmHl_updateOff(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);

/** \brief Update Pulse
 * \param driver GTM TOM PWM driver
 * \param tOn ON times. Phase 0 top, phase 1 top, ... phase 0 bottom, phase 1 botteom, ...
 * \param offset Offset value in ticks. Phase 0 top, phase 1 top, ... phase 0 bottom, phase 1 botteom, ...
 * \return None
 */
IFX_STATIC void IfxGtm_Tom_PwmHl_updatePulse(IfxGtm_Tom_PwmHl *driver, float32 *tOn, float32 *offset);

/** \brief Set Pulse to OFF
 * \param driver GTM TOM PWM driver
 * \param tOn ON times. Phase 0 top, phase 1 top, ... phase 0 bottom, phase 1 botteom, ...
 * \param offset Offset value in ticks. Phase 0 top, phase 1 top, ... phase 0 bottom, phase 1 botteom, ...
 * \return None
 */
IFX_STATIC void IfxGtm_Tom_PwmHl_updatePulseOff(IfxGtm_Tom_PwmHl *driver, float32 *tOn, float32 *offset);

/** \brief Update Shift Center Aligned
 * \param driver GTM TOM PWM driver
 * \param tOn ON time
 * \param shift Shift value in ticks
 * \return None
 */
IFX_STATIC void IfxGtm_Tom_PwmHl_updateShiftCenterAligned(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *shift);

/** \} */

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/

IFX_STATIC IFX_CONST IfxGtm_Tom_PwmHl_Mode IfxGtm_Tom_PwmHl_modes[Ifx_Pwm_Mode_off + 1] = {
    {Ifx_Pwm_Mode_centerAligned,         FALSE, &IfxGtm_Tom_PwmHl_updateCenterAligned, &IfxGtm_Tom_PwmHl_updateShiftCenterAligned, &IfxGtm_Tom_PwmHl_updatePulse   },
    {Ifx_Pwm_Mode_centerAlignedInverted, TRUE,  &IfxGtm_Tom_PwmHl_updateCenterAligned, &IfxGtm_Tom_PwmHl_updateShiftCenterAligned, &IfxGtm_Tom_PwmHl_updatePulse   },
    {Ifx_Pwm_Mode_leftAligned,           FALSE, &IfxGtm_Tom_PwmHl_updateEdgeAligned,   &IfxGtm_Tom_PwmHl_updateAndShiftOff,        &IfxGtm_Tom_PwmHl_updatePulse   },
    {Ifx_Pwm_Mode_rightAligned,          TRUE,  &IfxGtm_Tom_PwmHl_updateEdgeAligned,   &IfxGtm_Tom_PwmHl_updateAndShiftOff,        &IfxGtm_Tom_PwmHl_updatePulse   },
    {Ifx_Pwm_Mode_off,                   FALSE, &IfxGtm_Tom_PwmHl_updateOff,           &IfxGtm_Tom_PwmHl_updateAndShiftOff,        &IfxGtm_Tom_PwmHl_updatePulseOff}
};

/******************************************************************************/
/*---------------------Inline Function Implementations------------------------*/
/******************************************************************************/

IFX_INLINE Ifx_ActiveState IfxGtm_Tom_PwmHl_invertActiveState(Ifx_ActiveState activeState)
{
    return activeState == Ifx_ActiveState_low ? Ifx_ActiveState_high : Ifx_ActiveState_low;
}


/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/

float32 IfxGtm_Tom_PwmHl_getDeadtime(IfxGtm_Tom_PwmHl *driver)
{
    return IfxStdIf_Timer_tickToS(driver->timer->base.clockFreq, driver->base.deadtime);
}


float32 IfxGtm_Tom_PwmHl_getMinPulse(IfxGtm_Tom_PwmHl *driver)
{
    return IfxStdIf_Timer_tickToS(driver->timer->base.clockFreq, driver->base.minPulse - driver->base.deadtime);
}


Ifx_Pwm_Mode IfxGtm_Tom_PwmHl_getMode(IfxGtm_Tom_PwmHl *driver)
{
    return driver->base.mode;
}


boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config)
{
    boolean           result       = TRUE;
    uint16            channelMask;
    uint16            channelsMask = 0;
    uint32            channelIndex;
    uint16            maskShift    = 0;
    IfxGtm_Tom_Timer *timer        = config->timer;

    driver->base.mode             = Ifx_Pwm_Mode_init;
    driver->timer                 = timer;
    driver->base.setMode          = 0;
    driver->base.inverted         = FALSE;
    driver->base.ccxActiveState   = config->base.ccxActiveState;
    driver->base.coutxActiveState = config->base.coutxActiveState;
    driver->base.channelCount     = config->base.channelCount;

    IfxGtm_Tom_PwmHl_setDeadtime(driver, config->base.deadtime);
    IfxGtm_Tom_PwmHl_setMinPulse(driver, config->base.minPulse);

    driver->tom = &(timer->gtm->TOM[config->tom]);

    /* config->ccx[0] is used for the definition of the TGC */
    if (config->ccx[0]->channel <= 7)
    {
        driver->tgc = IfxGtm_Tom_Ch_getTgcPointer(driver->tom, 0);
    }
    else
    {
        driver->tgc = IfxGtm_Tom_Ch_getTgcPointer(driver->tom, 1);
    }

    maskShift = (config->ccx[0]->channel <= 7) ? 0 : 8;

    IFX_ASSERT(IFX_VERBOSE_LEVEL_ERROR, config->base.channelCount <= IFXGTM_TOM_PWMHL_MAX_NUM_CHANNELS);

    IfxGtm_Tom_Ch_ClkSrc clock = IfxGtm_Tom_Ch_getClockSource(timer->tom, timer->timerChannel);

    for (channelIndex = 0; channelIndex < config->base.channelCount; channelIndex++)
    {
        IfxGtm_Tom_Ch channel;
        /* CCX */
        channel                   = config->ccx[channelIndex]->channel;
        driver->ccx[channelIndex] = channel;
        channelMask               = 1 << (channel - maskShift);
        channelsMask             |= channelMask;

        /* Initialize the timer part */
        /* FIXME add IfxGtm_Tom_Ch_configurePwmMode() and use it */
        IfxGtm_Tom_Ch_setClockSource(driver->tom, channel, clock);

        /* Initialize the SOUR reset value and enable the channel */
        IfxGtm_Tom_Ch_setSignalLevel(driver->tom, channel, !driver->base.inverted
            ? config->base.ccxActiveState
            : IfxGtm_Tom_PwmHl_invertActiveState(config->base.ccxActiveState));
        IfxGtm_Tom_Tgc_enableChannels(driver->tgc, channelMask, 0, TRUE); /* Write the SOUR outout with !SL */
        IfxGtm_Tom_Tgc_enableChannelsOutput(driver->tgc, channelMask, 0, TRUE);

        /* Set the SL to the required level for run time */
        IfxGtm_Tom_Ch_setSignalLevel(driver->tom, channel, driver->base.inverted
            ? config->base.ccxActiveState
            : IfxGtm_Tom_PwmHl_invertActiveState(config->base.ccxActiveState));
        IfxGtm_Tom_Ch_setResetSource(driver->tom, channel, IfxGtm_Tom_Ch_ResetEvent_onTrigger);
        IfxGtm_Tom_Ch_setTriggerOutput(driver->tom, channel, IfxGtm_Tom_Ch_OutputTrigger_forward);
        IfxGtm_Tom_Ch_setCounterValue(driver->tom, channel, IfxGtm_Tom_Timer_getOffset(driver->timer));

        /*Initialize the port */
        IfxGtm_PinMap_setTomTout(config->ccx[channelIndex],
            config->base.outputMode, config->base.outputDriver);
        IfxPort_setPinState(config->ccx[channelIndex]->pin.port, config->ccx[channelIndex]->pin.pinIndex, config->base.ccxActiveState ? IfxPort_State_low : IfxPort_State_high);

        /* COUTX */
        channel                     = config->coutx[channelIndex]->channel;
        driver->coutx[channelIndex] = channel;
        channelMask                 = 1 << (channel - maskShift);
        channelsMask               |= channelMask;

        /* Initialize the timer part */
        /* FIXME add IfxGtm_Tom_Ch_configurePwmMode() and use it */
        IfxGtm_Tom_Ch_setClockSource(driver->tom, channel, clock);

        /* Initialize the SOUR reset value, SL and enable the channel */
        IfxGtm_Tom_Ch_setSignalLevel(driver->tom, channel, driver->base.inverted
            ? IfxGtm_Tom_PwmHl_invertActiveState(config->base.coutxActiveState)
            : config->base.coutxActiveState);
        IfxGtm_Tom_Tgc_enableChannels(driver->tgc, channelMask, 0, TRUE);
        IfxGtm_Tom_Tgc_enableChannelsOutput(driver->tgc, channelMask, 0, TRUE);

        IfxGtm_Tom_Ch_setResetSource(driver->tom, channel, IfxGtm_Tom_Ch_ResetEvent_onTrigger);
        IfxGtm_Tom_Ch_setTriggerOutput(driver->tom, channel, IfxGtm_Tom_Ch_OutputTrigger_forward);
        IfxGtm_Tom_Ch_setCounterValue(driver->tom, channel, IfxGtm_Tom_Timer_getOffset(driver->timer));

        /*Initialize the port */
        IfxGtm_PinMap_setTomTout(config->coutx[channelIndex],
            config->base.outputMode, config->base.outputDriver);
        IfxPort_setPinState(config->coutx[channelIndex]->pin.port, config->coutx[channelIndex]->pin.pinIndex, config->base.coutxActiveState ? IfxPort_State_low : IfxPort_State_high);
    }

    IfxGtm_Tom_PwmHl_setMode(driver, Ifx_Pwm_Mode_off);

    Ifx_TimerValue tOn[IFXGTM_TOM_PWMHL_MAX_NUM_CHANNELS] = {0};
    IfxGtm_Tom_PwmHl_updateOff(driver, tOn);     /* tOn do not need defined values */

    /* Transfer the shadow registers */
    IfxGtm_Tom_Tgc_setChannelsForceUpdate(driver->tgc, channelsMask, 0, 0, 0);
    IfxGtm_Tom_Tgc_trigger(driver->tgc); /* FIXME this seems to have no effect, to be checked */
    IfxGtm_Tom_Tgc_setChannelsForceUpdate(driver->tgc, 0, channelsMask, 0, 0);

    /* Enable timer to update the channels */
    for (channelIndex = 0; channelIndex < driver->base.channelCount; channelIndex++)
    {
        IfxGtm_Tom_Timer_addToChannelMask(timer, driver->ccx[channelIndex]);
        IfxGtm_Tom_Timer_addToChannelMask(timer, driver->coutx[channelIndex]);
    }

    return result;
}


void IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config)
{
    IfxStdIf_PwmHl_initConfig(&config->base);
    config->timer = NULL_PTR;
    config->tom   = IfxGtm_Tom_0;
    config->ccx   = NULL_PTR;
    config->coutx = NULL_PTR;
}


boolean IfxGtm_Tom_PwmHl_setDeadtime(IfxGtm_Tom_PwmHl *driver, float32 deadtime)
{
    Ifx_TimerValue value = IfxStdIf_Timer_sToTick(driver->timer->base.clockFreq, deadtime);

    /* FIXME warn if dead time in out of range: > 1/2 period, ... */
    driver->base.deadtime = value;

    return TRUE;
}


boolean IfxGtm_Tom_PwmHl_setMinPulse(IfxGtm_Tom_PwmHl *driver, float32 minPulse)
{
    Ifx_TimerValue value = IfxStdIf_Timer_sToTick(driver->timer->base.clockFreq, minPulse);

    driver->base.minPulse = value + driver->base.deadtime;
    driver->base.maxPulse = driver->timer->base.period - driver->base.minPulse;

    return TRUE;
}


boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode)
{
    boolean                result = TRUE;
    IfxGtm_Tom_PwmHl_Base *base   = &driver->base;

    if (base->mode != mode)
    {
        if ((mode > Ifx_Pwm_Mode_off) || (IfxGtm_Tom_PwmHl_modes[mode].update == NULL_PTR))
        {
            mode   = Ifx_Pwm_Mode_off;
            result = FALSE;
        }

        IFX_ASSERT(IFX_VERBOSE_LEVEL_ERROR, mode == IfxGtm_Tom_PwmHl_modes[mode].mode);

        base->mode             = mode;
        driver->update         = IfxGtm_Tom_PwmHl_modes[mode].update;
        driver->updateAndShift = IfxGtm_Tom_PwmHl_modes[mode].updateAndShift;
        driver->updatePulse    = IfxGtm_Tom_PwmHl_modes[mode].updatePulse;

        if (base->mode != Ifx_Pwm_Mode_off)
        {
            base->inverted = IfxGtm_Tom_PwmHl_modes[mode].inverted;
        }
        else
        {                       /* Keep previous inverted for off mode */
        }

        if (base->inverted)
        {
            driver->ccxTemp   = driver->coutx;
            driver->coutxTemp = driver->ccx;
        }
        else
        {
            driver->ccxTemp   = driver->ccx;
            driver->coutxTemp = driver->coutx;
        }

        {   /* Workaround to enable the signal inversion required for center aligned inverted
             * and right aligned modes */
            /** \note that changing signal level may produce short circuit at the power stage,
             * in which case the inverter must be disable during this action. */

            /* Ifx_Pwm_Mode_centerAligned and Ifx_Pwm_Mode_LeftAligned use inverted=FALSE */
            /* Ifx_Pwm_Mode_centerAlignedInverted and Ifx_Pwm_Mode_RightAligned use inverted=TRUE */
            uint32 channelIndex;

            for (channelIndex = 0; channelIndex < driver->base.channelCount; channelIndex++)
            {
                IfxGtm_Tom_Ch channel;

                channel = driver->ccx[channelIndex];
                IfxGtm_Tom_Ch_setSignalLevel(driver->tom, channel, base->inverted
                    ? base->ccxActiveState
                    : IfxGtm_Tom_PwmHl_invertActiveState(driver->base.ccxActiveState));

                channel = driver->coutx[channelIndex];
                IfxGtm_Tom_Ch_setSignalLevel(driver->tom, channel, base->inverted
                    ? IfxGtm_Tom_PwmHl_invertActiveState(driver->base.coutxActiveState)
                    : base->coutxActiveState);
            }
        }
    }

    return result;
}


void IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn)
{
    driver->update(driver, tOn);
}


void IfxGtm_Tom_PwmHl_setOnTimeAndShift(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *shift)
{
    driver->updateAndShift(driver, tOn, shift);
}


void IfxGtm_Tom_PwmHl_setPulse(IfxGtm_Tom_PwmHl *driver, float32 *tOn, float32 *offset)
{
    driver->updatePulse(driver, tOn, offset);
}


void IfxGtm_Tom_PwmHl_setupChannels(IfxGtm_Tom_PwmHl *driver, boolean *activeCh, boolean *stuckSt)
{
    /* FIXME TODO */
}


boolean IfxGtm_Tom_PwmHl_stdIfPwmHlInit(IfxStdIf_PwmHl *stdif, IfxGtm_Tom_PwmHl *driver)
{
    /* Ensure the stdif is reset to zeros */
    memset(stdif, 0, sizeof(IfxStdIf_PwmHl));

    /* *INDENT-OFF* Note: this file was indented manually by the author. */
    /* Set the API link */
    stdif->driver               = driver;
    stdif->setDeadtime          = (IfxStdIf_PwmHl_SetDeadtime      )&IfxGtm_Tom_PwmHl_setDeadtime;
    stdif->getDeadtime          = (IfxStdIf_PwmHl_GetDeadtime      )&IfxGtm_Tom_PwmHl_getDeadtime;
    stdif->setMinPulse          = (IfxStdIf_PwmHl_SetMinPulse      )&IfxGtm_Tom_PwmHl_setMinPulse;
    stdif->getMinPulse          = (IfxStdIf_PwmHl_GetMinPulse      )&IfxGtm_Tom_PwmHl_getMinPulse;
    stdif->getMode              = (IfxStdIf_PwmHl_GetMode          )&IfxGtm_Tom_PwmHl_getMode;
    stdif->setMode              = (IfxStdIf_PwmHl_SetMode          )&IfxGtm_Tom_PwmHl_setMode;
    stdif->setOnTime            = (IfxStdIf_PwmHl_SetOnTime        )&IfxGtm_Tom_PwmHl_setOnTime;
    stdif->setOnTimeAndShift    = (IfxStdIf_PwmHl_SetOnTimeAndShift)&IfxGtm_Tom_PwmHl_setOnTimeAndShift;
    stdif->setPulse             = (IfxStdIf_PwmHl_SetPulse         )&IfxGtm_Tom_PwmHl_setPulse;
    stdif->setupChannels        = (IfxStdIf_PwmHl_SetupChannels    )&IfxGtm_Tom_PwmHl_setupChannels;
    IfxGtm_Tom_Timer_stdIfTimerInit(&stdif->timer, driver->timer);
    /* *INDENT-ON* */

    return TRUE;
}


IFX_STATIC void IfxGtm_Tom_PwmHl_updateAndShiftOff(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *shift)
{
    IfxGtm_Tom_PwmHl_updateOff(driver, NULL_PTR);
}


IFX_STATIC void IfxGtm_Tom_PwmHl_updateCenterAligned(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn)
{
    uint8          channelIndex;
    Ifx_TimerValue period;
    Ifx_TimerValue deadtime = driver->base.deadtime;

    period = driver->timer->base.period;

    for (channelIndex = 0; channelIndex < driver->base.channelCount; channelIndex++)
    {
        Ifx_TimerValue x;             /* x=period*dutyCycle, x=OnTime+deadTime */
        Ifx_TimerValue cm0, cm1;
        x = tOn[channelIndex];

        if (driver->base.inverted != FALSE)
        {
            x = period - x;
        }
        else
        {}

        if ((x < driver->base.minPulse) || (x <= deadtime))
        {                       /* For deadtime condition: avoid leading edge of top channel to occur after the trailing edge */
            x = 0;
        }
        else if (x > driver->base.maxPulse)
        {
            x = period;
        }
        else
        {}

        /* Special handling due to GTM issue */
        if (x == period)
        {                       /* 100% duty cycle */
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->ccxTemp[channelIndex],
                period + 1 /* No compare event */,
                2 /* 1st compare event (issue: expected to be 1) */ + deadtime);
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->coutxTemp[channelIndex],
                period + 2 /* No compare event, issues has been seen with +1 */,
                2 /* 1st compare event (issue: expected to be 1) */);
        }
        else if (x == 0)
        {
            cm0 = 1;
            cm1 = period + 2;
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->ccxTemp[channelIndex], cm0, cm1);
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->coutxTemp[channelIndex], cm0 + deadtime, cm1);
        }
        else
        {                           /* x% duty cycle */
            cm1 = (period - x) / 2; // CM1 /* FIXME issue if CM1 <= 1, should be limited for up to AB step at least */
            cm0 = (period + x) / 2; // CM0
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->ccxTemp[channelIndex], cm0, cm1 + deadtime);
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->coutxTemp[channelIndex], cm0 + deadtime, cm1);
        }
    }
}


IFX_STATIC void IfxGtm_Tom_PwmHl_updateEdgeAligned(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn)
{
    uint8          channelIndex;
    Ifx_TimerValue period;
    Ifx_TimerValue deadtime = driver->base.deadtime;

    period = driver->timer->base.period;

    for (channelIndex = 0; channelIndex < driver->base.channelCount; channelIndex++)
    {
        Ifx_TimerValue x;             /* x=period*dutyCycle, x=OnTime+deadTime */
        Ifx_TimerValue cm0, cm1;
        x = tOn[channelIndex];

        if (driver->base.inverted != FALSE)
        {
            x = period - x;
        }
        else
        {}

        if ((x < driver->base.minPulse) || (x <= deadtime))
        {                       /* For deadtime condition: avoid leading edge of top channel to occur after the trailing edge */
            x = 0;
        }
        else if (x > driver->base.maxPulse)
        {
            x = period;
        }
        else
        {}

        /* Special handling due to GTM issue */
        if (x == period)
        {                       /* 100% duty cycle */
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->ccxTemp[channelIndex],
                period + 1 /* No compare event */,
                2 /* 1st compare event (issue: expected to be 1) */ + deadtime);
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->coutxTemp[channelIndex],
                period + 2 /* No compare event, issues has been seen with +1 */,
                2 /* 1st compare event (issue: expected to be 1) */);
        }
        else if (x == 0)
        {
            cm0 = 1;
            cm1 = period + 2;
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->ccxTemp[channelIndex], cm0, cm1);
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->coutxTemp[channelIndex], cm0 + deadtime, cm1);
        }
        else
        {                       /* x% duty cycle */
            cm1 = 2;            // CM1, set to 2 due to a GTM issue. should be 1 according to spec
            cm0 = x;            // CM0, set to x+2 due to a GTM issue. should be x+1 according to spec
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->ccxTemp[channelIndex], cm0, cm1 + deadtime);
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->coutxTemp[channelIndex], cm0 + deadtime, cm1);
        }
    }
}


IFX_STATIC void IfxGtm_Tom_PwmHl_updateOff(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn)
{
    uint8          channelIndex;
    Ifx_TimerValue period;

    period = driver->timer->base.period;

    for (channelIndex = 0; channelIndex < driver->base.channelCount; channelIndex++)
    {
        IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->ccxTemp[channelIndex],
            2 /* 1 will keep the previous level */, period + 2);
        IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->coutxTemp[channelIndex], period + 1, 2);
    }
}


IFX_STATIC void IfxGtm_Tom_PwmHl_updatePulse(IfxGtm_Tom_PwmHl *driver, float32 *tOn, float32 *offset)
{
    uint8          channelIndex;
    Ifx_TimerValue period;

    period = driver->timer->base.period;

    /* Top channels */
    for (channelIndex = 0; channelIndex < driver->base.channelCount; channelIndex++)
    {
        Ifx_TimerValue x; /* x=period*dutyCycle, x=OnTime+deadTime */
        Ifx_TimerValue o;
        Ifx_TimerValue cm0, cm1;

        x = IfxStdIf_Timer_sToTick(driver->timer->base.clockFreq, tOn[channelIndex]);
        o = IfxStdIf_Timer_sToTick(driver->timer->base.clockFreq, offset[channelIndex]);

        if (driver->base.inverted != FALSE)
        {
            x = period - x;
        }
        else
        {}

        if ((x < driver->base.minPulse) || (o > period))
        {
            x = 0;
        }
        else if ((x > driver->base.maxPulse) || (o + x > period))
        {
            x = period;
        }
        else
        {}

        /* Special handling due to GTM issue */
        if (x == period)
        {   /* 100% duty cycle */
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->ccxTemp[channelIndex],
                period + 1 /* No compare event */,
                2 /* 1st compare event (issue: expected to be 1)*/);
        }
        else if (x == 0)
        {
            cm0 = 1;
            cm1 = period + 2;
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->ccxTemp[channelIndex], cm0, cm1);
        }
        else
        {                /* x% duty cycle */
            cm1 = 2 + o; // CM1, set to 2 due to a GTM issue. should be 1 according to spec
            cm0 = o + x; // CM0, set to x+2 due to a GTM issue. should be x+1 according to spec
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->ccxTemp[channelIndex], cm0, cm1);
        }
    }

    /* Bottom channels */
    for (channelIndex = 0; channelIndex < driver->base.channelCount; channelIndex++)
    {
        Ifx_TimerValue x; /* x=period*dutyCycle, x=OnTime+deadTime */
        Ifx_TimerValue o;
        Ifx_TimerValue cm0, cm1;

        x = IfxStdIf_Timer_sToTick(driver->timer->base.clockFreq, tOn[channelIndex + driver->base.channelCount]);
        o = IfxStdIf_Timer_sToTick(driver->timer->base.clockFreq, offset[channelIndex + driver->base.channelCount]);

        if (driver->base.inverted != FALSE)
        {
            x = period - x;
        }
        else
        {}

        if (x < driver->base.minPulse)
        {
            x = 0;
        }
        else if (x > driver->base.maxPulse)
        {
            x = period;
        }
        else
        {}

        /* Special handling due to GTM issue */
        if (x == period)
        {   /* 100% duty cycle */
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->coutxTemp[channelIndex],
                period + 2 /* No compare event, issues has been seen with +1 */,
                2 /* 1st compare event (issue: expected to be 1)*/);
        }
        else if (x == 0)
        {
            cm0 = 1;
            cm1 = period + 2;
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->coutxTemp[channelIndex], cm0, cm1);
        }
        else
        {                /* x% duty cycle */
            cm1 = 2 + o; // CM1, set to 2 due to a GTM issue. should be 1 according to spec
            cm0 = o + x; // CM0, set to x+2 due to a GTM issue. should be x+1 according to spec
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->coutxTemp[channelIndex], cm0, cm1);
        }
    }
}


IFX_STATIC void IfxGtm_Tom_PwmHl_updatePulseOff(IfxGtm_Tom_PwmHl *driver, float32 *tOn, float32 *offset)
{
    IfxGtm_Tom_PwmHl_updateOff(driver, NULL_PTR);
}


IFX_STATIC void IfxGtm_Tom_PwmHl_updateShiftCenterAligned(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *shift)
{
    uint8          channelIndex;
    Ifx_TimerValue period;
    Ifx_TimerValue deadtime = driver->base.deadtime;

    period = driver->timer->base.period;

    for (channelIndex = 0; channelIndex < driver->base.channelCount; channelIndex++)
    {
        Ifx_TimerValue x; /* x=period*dutyCycle, x=OnTime+deadTime */
        Ifx_TimerValue s; /* Shift value */
        Ifx_TimerValue cm0, cm1;
        x = tOn[channelIndex];

        if (driver->base.inverted != FALSE)
        {
            x = period - x;
        }
        else
        {}

        if ((x < driver->base.minPulse) || (x <= deadtime))
        {   /* For deadtime condition: avoid leading edge of top channel to occur after the trailing edge */
            x = 0;
        }
        else if (x > driver->base.maxPulse)
        {
            x = period;
        }
        else
        {}

        /* Special handling due to GTM issue */
        if (x == period)
        {   /* 100% duty cycle */
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->ccxTemp[channelIndex],
                period + 1 /* No compare event */,
                2 /* 1st compare event (issue: expected to be 1)*/ + deadtime);
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->coutxTemp[channelIndex],
                period + 2 /* No compare event, issues has been seen with +1 */,
                2 /* 1st compare event (issue: expected to be 1)*/);
        }
        else if (x == 0)
        {
            cm0 = 1;
            cm1 = period + 2;
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->ccxTemp[channelIndex], cm0, cm1);
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->coutxTemp[channelIndex], cm0 + deadtime, cm1);
        }
        else
        {                           /* x% duty cycle */
            s = shift[channelIndex];

            if (s > 0)
            {
                s = __minX(s, (period - x) / 2 - 1);
            }
            else
            {
                s = __maxX(s, (x - period) / 2 + 1);
            }

            cm1 = s + (period - x) / 2; // CM1
            cm0 = s + (period + x) / 2; // CM0
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->ccxTemp[channelIndex], cm0, cm1 + deadtime);
            IfxGtm_Tom_Ch_setCompareShadow(driver->tom, driver->coutxTemp[channelIndex], cm0 + deadtime, cm1);
        }
    }
}
