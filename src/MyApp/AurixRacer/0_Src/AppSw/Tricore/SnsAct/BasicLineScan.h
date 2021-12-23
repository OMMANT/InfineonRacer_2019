/**
 * \file BasicLineScan.h
 * \brief BasicLineScan
 *
 */

#ifndef BASICLINESCAN_H
#define BASICLINESCAN_H 1

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include <Vadc/Std/IfxVadc.h>
#include <Vadc/Adc/IfxVadc_Adc.h>

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*--------------------------------Enumerations--------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
typedef struct{
	uint32 adcResult[2][128];
	uint32 adcMax[2];
}IR_LineScan_t;

typedef struct{
	int result[2][128];
	uint32 whiteCount[2];
	boolean changeFlag[2];
	uint32 flagCount[2];
	uint32 boundaryIndex[2][20];
	uint32 bdyInx[2];
}IR_LineColor;

/******************************************************************************/
/*------------------------------Global variables------------------------------*/
/******************************************************************************/
IFX_EXTERN IR_LineScan_t IR_LineScan;
IFX_EXTERN IR_LineColor ScanResult;
IFX_EXTERN sint32 lineError[2];
IFX_EXTERN boolean noError;
IFX_EXTERN boolean hasInitialed;
IFX_EXTERN uint32 LaneNumber;

/******************************************************************************/
/*-------------------------Function Prototypes--------------------------------*/
/******************************************************************************/
IFX_EXTERN void BasicLineScan_init(void);
IFX_EXTERN void BasicLineScan_run(void);
IFX_EXTERN void init_ScanResult(void);
IFX_EXTERN void copyData(uint32 *src, uint32* dst);
IFX_EXTERN uint32 calcError(uint32 *src1, uint32* src2);

#endif
