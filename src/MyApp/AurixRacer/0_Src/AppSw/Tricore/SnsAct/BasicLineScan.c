/**
 * \file BasicLineScan.c
 * \brief BasicLineScan
 *
 */

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/

#include <stdio.h>
#include <Cpu/Std/IfxCpu.h>
#include <IfxPort_PinMap.h>

#include <SysSe/Bsp/Bsp.h>
#include <Port/Std/IfxPort.h>

#include "BasicLineScan.h"
#include "Configuration.h"
/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*--------------------------------Enumerations--------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
typedef struct
{
    IfxVadc_Adc vadc; /* VADC handle */
    IfxVadc_Adc_Group adcGroup;
    IfxVadc_Adc_Channel       adcChannel[2];
} Basic_VadcAutoScan;

/******************************************************************************/
/*------------------------------Global variables------------------------------*/
/******************************************************************************/
Basic_VadcAutoScan g_VadcAutoScan; /**< \brief Demo information */

IR_LineScan_t IR_LineScan;
IR_LineColor ScanResult;
sint32 lineError[2] = {0, 0};
boolean noError;
boolean hasInitialed;
uint32 LaneNumber;
IFX_EXTERN boolean isSchoolZone;

/******************************************************************************/
/*-------------------------Function Prototypes--------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/

/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/

/** \brief Demo init API
 *
 * This function is called from main during initialization phase
 */
void BasicLineScan_init(void)
{
    /* VADC Configuration */

    /* create configuration */
    IfxVadc_Adc_Config adcConfig;
    IfxVadc_Adc_initModuleConfig(&adcConfig, &MODULE_VADC);

    /* initialize module */
    IfxVadc_Adc_initModule(&g_VadcAutoScan.vadc, &adcConfig);

    /* create group config */
    IfxVadc_Adc_GroupConfig adcGroupConfig;
    IfxVadc_Adc_initGroupConfig(&adcGroupConfig, &g_VadcAutoScan.vadc);

    /* with group 0 */
    adcGroupConfig.groupId = IfxVadc_GroupId_0;
    adcGroupConfig.master  = adcGroupConfig.groupId;

    /* enable scan source */
    adcGroupConfig.arbiter.requestSlotScanEnabled = TRUE;

    /* enable auto scan */
    adcGroupConfig.scanRequest.autoscanEnabled = TRUE;

    /* enable all gates in "always" mode (no edge detection) */
    adcGroupConfig.scanRequest.triggerConfig.gatingMode = IfxVadc_GatingMode_always;

    /* initialize the group */
    /*IfxVadc_Adc_Group adcGroup;*/    //declared globally
    IfxVadc_Adc_initGroup(&g_VadcAutoScan.adcGroup, &adcGroupConfig);

    uint32                    chnIx;
    unsigned channels;
    unsigned mask;
    /* create channel config */
    IfxVadc_Adc_ChannelConfig adcChannelConfig[2];

    {
    	chnIx = 0;
    	IfxVadc_Adc_initChannelConfig(&adcChannelConfig[chnIx], &g_VadcAutoScan.adcGroup);

        adcChannelConfig[chnIx].channelId      = (IfxVadc_ChannelId)(TSL1401_AO_1);
        adcChannelConfig[chnIx].resultRegister = (IfxVadc_ChannelResult)(TSL1401_AO_1);  /* use dedicated result register */

        /* initialize the channel */
        IfxVadc_Adc_initChannel(&g_VadcAutoScan.adcChannel[chnIx], &adcChannelConfig[chnIx]);

        /* add to scan */
        channels = (1 << adcChannelConfig[chnIx].channelId);
        mask     = channels;
        IfxVadc_Adc_setScan(&g_VadcAutoScan.adcGroup, channels, mask);

    	chnIx = 1;
    	IfxVadc_Adc_initChannelConfig(&adcChannelConfig[chnIx], &g_VadcAutoScan.adcGroup);
        adcChannelConfig[chnIx].channelId      = (IfxVadc_ChannelId)(TSL1401_AO_2);
        adcChannelConfig[chnIx].resultRegister = (IfxVadc_ChannelResult)(TSL1401_AO_2);  /* use dedicated result register */

        /* initialize the channel */
        IfxVadc_Adc_initChannel(&g_VadcAutoScan.adcChannel[chnIx], &adcChannelConfig[chnIx]);

        /* add to scan */
        channels = (1 << adcChannelConfig[chnIx].channelId);
        mask     = channels;
        IfxVadc_Adc_setScan(&g_VadcAutoScan.adcGroup, channels, mask);

    }

    {
    	initTime();
    	IfxPort_setPinMode(TSL1401_SI.port, TSL1401_SI.pinIndex, IfxPort_Mode_outputPushPullGeneral);
		IfxPort_setPinPadDriver(TSL1401_SI.port, TSL1401_SI.pinIndex, IfxPort_PadDriver_cmosAutomotiveSpeed1);
		IfxPort_setPinState(TSL1401_SI.port, TSL1401_SI.pinIndex, IfxPort_State_low);

		IfxPort_setPinMode(TSL1401_CLK.port, TSL1401_CLK.pinIndex, IfxPort_Mode_outputPushPullGeneral);
		IfxPort_setPinPadDriver(TSL1401_CLK.port, TSL1401_CLK.pinIndex, IfxPort_PadDriver_cmosAutomotiveSpeed1);
		IfxPort_setPinState(TSL1401_CLK.port, TSL1401_CLK.pinIndex, IfxPort_State_low);
    }
    for(int i = 0; i < 10; i++){
    	ScanResult.boundaryIndex[0][i] = -1;
    	ScanResult.boundaryIndex[0][127-i] = -1;
    	ScanResult.boundaryIndex[1][i] = -1;
    	ScanResult.boundaryIndex[1][127-i] = -1;
    }
    IR_LineScan.adcMax[0] = 0;
    IR_LineScan.adcMax[1] = 0;
    init_ScanResult();
    LaneNumber = 0;
    hasInitialed = FALSE;
}


/** \brief Demo run API
 *
 * This function is called from main, background loop
 */
void BasicLineScan_run(void)
{
	uint32 chnIx;
	uint32 idx;
	uint32 tempMax[2] = {0};
	uint32 previousData[2][128] = {0,};
	noError = TRUE;
	lineError[0] = 0;
	lineError[1] = 0;
	copyData(IR_LineScan.adcResult[0], previousData[0]);
	copyData(IR_LineScan.adcResult[1], previousData[1]);

	IfxPort_setPinState(TSL1401_SI.port, TSL1401_SI.pinIndex, IfxPort_State_high);
	IfxPort_setPinState(TSL1401_CLK.port, TSL1401_CLK.pinIndex, IfxPort_State_low);
	waitTime(5*TimeConst_100ns);

	IfxPort_setPinState(TSL1401_SI.port, TSL1401_SI.pinIndex, IfxPort_State_high);
	IfxPort_setPinState(TSL1401_CLK.port, TSL1401_CLK.pinIndex, IfxPort_State_high);
	waitTime(5*TimeConst_100ns);

	IfxPort_setPinState(TSL1401_SI.port, TSL1401_SI.pinIndex, IfxPort_State_low);
	IfxPort_setPinState(TSL1401_CLK.port, TSL1401_CLK.pinIndex, IfxPort_State_high);
	waitTime(5*TimeConst_100ns);
    IfxVadc_Adc_startScan(&g_VadcAutoScan.adcGroup);


	for(idx = 0; idx < 128 ; ++idx)
	{

		IfxPort_setPinState(TSL1401_SI.port, TSL1401_SI.pinIndex, IfxPort_State_low);
    	IfxPort_setPinState(TSL1401_CLK.port, TSL1401_CLK.pinIndex, IfxPort_State_low);
    	waitTime(3*TimeConst_1us);

    	IfxPort_setPinState(TSL1401_SI.port, TSL1401_SI.pinIndex, IfxPort_State_low);
    	IfxPort_setPinState(TSL1401_CLK.port, TSL1401_CLK.pinIndex, IfxPort_State_high);
    	waitTime(2*TimeConst_1us);

        /* check results */
        for (chnIx = 0; chnIx < 2; ++chnIx)
        {
            /* wait for valid result */
            Ifx_VADC_RES conversionResult;

            do
            {
                conversionResult = IfxVadc_Adc_getResult(&g_VadcAutoScan.adcChannel[chnIx]);
            } while (!conversionResult.B.VF);
            IR_LineScan.adcResult[chnIx][idx] = conversionResult.B.RESULT;
            if(tempMax[chnIx] < IR_LineScan.adcResult[chnIx][idx])
            	tempMax[chnIx] = IR_LineScan.adcResult[chnIx][idx];

            IR_LineScan.adcResult[chnIx][idx] = conversionResult.B.RESULT;
        }
	}
	IR_LineScan.adcMax[0] = tempMax[0];
	IR_LineScan.adcMax[1] = tempMax[1];

	IfxPort_setPinState(TSL1401_SI.port, TSL1401_SI.pinIndex, IfxPort_State_low);
	IfxPort_setPinState(TSL1401_CLK.port, TSL1401_CLK.pinIndex, IfxPort_State_low);
	/******************************************************************************/
	/*----------------Processing ScanResult for handling easily-------------------*/
	/******************************************************************************/
	/*lineError[0] = calcError(IR_LineScan.adcResult[0], previousData[0]);
	**lineError[1] = calcError(IR_LineScan.adcResult[1], previousData[1]);
	if(lineError[0] > 50 && hasInitialed){
		copyData(previousData[0], IR_LineScan.adcResult[0]);
		noError = FALSE;
	}
	if(lineError[1] > 50 && hasInitialed){
		copyData(previousData[1], IR_LineScan.adcResult[1]);
		noError = FALSE;
	}
	if(noError || !hasInitialed)*/
	{
		init_ScanResult();
		//정의한대로 계산함
		for(int chnIx = 0; chnIx < 2; chnIx++){
			uint32 maximum = IR_LineScan.adcMax[chnIx];
			uint32 standard = (maximum >> 2) * 3;
			for(int idx = 9; idx < 120; idx++){
				ScanResult.result[chnIx][idx] = (IR_LineScan.adcResult[chnIx][idx] > standard);
				if(ScanResult.result[chnIx][idx-1] >= 0 && ScanResult.result[chnIx][idx - 1] != ScanResult.result[chnIx][idx]){
					ScanResult.changeFlag[chnIx] = TRUE;
					ScanResult.flagCount[chnIx]++;
				}
				if(ScanResult.result[chnIx][idx] == 1)
					ScanResult.whiteCount[chnIx]++;
				if(ScanResult.changeFlag[chnIx]){
					if(idx >= 31 && idx <= 95)
						ScanResult.boundaryIndex[chnIx][ScanResult.bdyInx[chnIx]++] = idx;
					ScanResult.changeFlag[chnIx]=FALSE;
				}
			}
		}
		if(isSchoolZone && !LaneChanging){
			if(ScanResult.whiteCount[0] < ScanResult.whiteCount[1])
				LaneNumber = 1;
			else
				LaneNumber = 2;
		}
	}
//	waitTime(1*TimeConst_10ms);
}

void init_ScanResult(void){
    ScanResult.bdyInx[0] = 0;
    ScanResult.bdyInx[1] = 0;
    ScanResult.whiteCount[0] = 0;
    ScanResult.whiteCount[1] = 0;
    ScanResult.flagCount[0]=0;
    ScanResult.flagCount[1]=0;
    memset(ScanResult.boundaryIndex, 0, sizeof(ScanResult.boundaryIndex));
}

void copyData(uint32 *src, uint32* dst){
	for(int i = 0; i < 128; i++)
		dst[i] = src[i];
}

uint32 calcError(uint32 *src1, uint32* src2){
	uint32 iReturn = 0;
	double temp = 0.0;
	for(int i = 0; i < 128; i++){
		temp = 1-(src2[0]/src1[0]);
		int n = temp * 100;
		if(n < 0)
			iReturn += (-1*n);
		else
			iReturn += n;
	}
	iReturn = iReturn / 128;
	return iReturn;
}
