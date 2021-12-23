#ifndef INFINEONRACER_H_
#define INFINEONRACER_H_

#define CAMERA_DISTANCE	250
#define MILLIMETER_PER_PIXEL	1.19140625
#define NOMAL_SPEED	0.4f
#define SCHOOLZONE_SPEED	0.25f
#define MINIMUM_ENCODER_SPEED		100
#define CAMERA_DANGLE	25.0
#define CAMERA_ANGLE	0.43633231
#define TURN_LEFT	FALSE
#define TURN_RIGHT	TRUE


/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/

#include <Ifx_Types.h>
#include "Configuration.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
#define IR_getLs0Margin()		IR_Ctrl.Ls0Margin
#define IR_getLs1Margin()		IR_Ctrl.Ls1Margin

/******************************************************************************/
/*--------------------------------Enumerations--------------------------------*/
/******************************************************************************/



/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
typedef boolean TURN_DIR;
typedef struct{
	sint32 Ls0Margin;
	sint32 Ls1Margin;
	boolean basicTest;
}InfineonRacer_t;

/******************************************************************************/
/*------------------------------Global variables------------------------------*/
/******************************************************************************/
IFX_EXTERN InfineonRacer_t IR_Ctrl;
IFX_EXTERN boolean isSchoolZone;
IFX_EXTERN boolean LaneChanging;
IFX_EXTERN float32 speed0;
IFX_EXTERN float32 speed;
IFX_EXTERN double angle, dAngle;
IFX_EXTERN double centerPixel;
IFX_EXTERN TURN_DIR direction;

/******************************************************************************/
/*-------------------------Function Prototypes--------------------------------*/
/******************************************************************************/
IFX_EXTERN void InfineonRacer_init(void);
IFX_EXTERN void InfineonRacer_detectLane(void);
IFX_EXTERN void InfineonRacer_control(void);
IFX_EXTERN boolean InfineonRacer_isSchoolZone(void);
IFX_EXTERN void Obstacle_changeLane(void);
IFX_EXTERN void InfineonRacer_changeLane(void);
IFX_EXTERN void InfineonRacer_AEB(void);
IFX_EXTERN double degree2radian(double degree);
IFX_EXTERN double radian2degree(double radian);

#endif
