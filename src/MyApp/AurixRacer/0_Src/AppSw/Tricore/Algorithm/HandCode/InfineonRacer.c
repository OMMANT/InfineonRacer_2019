/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "InfineonRacer.h"
#include "Basic.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*--------------------------------Enumerations--------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*------------------------------Global variables------------------------------*/
/******************************************************************************/

InfineonRacer_t IR_Ctrl  /**< \brief  global data */
		= {64, 64, FALSE  };
boolean isSchoolZone;
boolean LaneChanging;
float32 speed0;
float32 speed;
double angle, dAngle;
double centerPixel;
TURN_DIR direction;
/******************************************************************************/
/*-------------------------Function Prototypes--------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/

/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
void InfineonRacer_init(void){
	if(isSchoolZone){
		speed0 = SCHOOLZONE_SPEED;
		speed = speed0;
	}
	else{
		speed0 = NOMAL_SPEED;
		speed = speed0;
	}
}

void InfineonRacer_detectLane(void){
	/*
	 * IR_LineScan.adcResult �� ������ �о�鿩��
	 * IR_Ctrl.Ls0Margin, IR_Ctrl.Ls1Margin ������ ����Ѵ�
	 */
	angle = 0.0, dAngle = angle * 57.29577951; //angle*180/pi
	float32 srvAngle = IR_getSrvAngle();
	float32 Motor0Vol = IR_getMotor0Vol();
	double s;
	//int leftBoundary = ScanResult.bdyInx[0], rightBoundary = ScanResult.bdyInx[1];
	IR_Ctrl.Ls0Margin = ScanResult.boundaryIndex[0][0];
	IR_Ctrl.Ls1Margin = ScanResult.boundaryIndex[1][0];
	//1. ������ ���� (���� ��->�� ��谪�� ����ġ�� ���, ������ ��->�� ��谪�� ����ġ�� ���)
	if(IR_Ctrl.Ls0Margin == 0 || IR_Ctrl.Ls1Margin == 0){
		//������ ī�޶� ������ (���� ī�޶� ���� ����)
		if(IR_Ctrl.Ls0Margin != 0){
			s = (IR_Ctrl.Ls0Margin - 63) * MILLIMETER_PER_PIXEL * cos(CAMERA_ANGLE);
			angle = atan2(s, CAMERA_DISTANCE);
			dAngle = radian2degree(angle);
			if(IR_Ctrl.Ls0Margin > 63)
				dAngle += 5.5;
		}
		//������ ī�޶� ������ (������ ī�޶� ���� ����)
		else {
			s = (IR_Ctrl.Ls1Margin - 63) * MILLIMETER_PER_PIXEL * cos(CAMERA_ANGLE);
			/******************************************************************************/
			/*-------------------------!!!!!!!!!Ȯ�� �ʿ�!!!!!!!!!---------------------------*/
			/******************************************************************************/
			//������ ī�޶�� ���ʰ� ��Ī�̱� ������ �ݴ� �����̹Ƿ� -1�� ���������?
			s *= -1;
			angle = atan2(s, CAMERA_DISTANCE);
			dAngle = radian2degree(angle);
			if(IR_Ctrl.Ls1Margin >= 63)
				dAngle += 5.5;
		}
	}
	//2. �� �� ����
	//Ls0Margin & Ls1Margin ��� 0�� �ƴ�
	else {
		//�������� ��ȸ��
		if(IR_Ctrl.Ls0Margin <= 63 && IR_Ctrl.Ls1Margin <= 63){
				s = (IR_Ctrl.Ls0Margin - 63) * MILLIMETER_PER_PIXEL * cos(CAMERA_ANGLE);
				angle = atan2(s, CAMERA_DISTANCE);
				dAngle = radian2degree(angle);
		}
		//�������� ��ȸ��
		else if(IR_Ctrl.Ls0Margin > 63 && IR_Ctrl.Ls1Margin > 63){
			s = (IR_Ctrl.Ls1Margin - 64) * MILLIMETER_PER_PIXEL * cos(CAMERA_ANGLE);
			/******************************************************************************/
			/*-------------------------!!!!!!!!!Ȯ�� �ʿ�!!!!!!!!!---------------------------*/
			/******************************************************************************/
			//������ ī�޶�� ���ʰ� ��Ī�̱� ������ �ݴ� �����̹Ƿ� -1�� ���������.
			s *= -1;
			angle = atan2(s, CAMERA_DISTANCE);
			dAngle = radian2degree(angle) + 5.5;
		}
		//���������� ��Ȳ
		//���� ������� �����ϱ�
		else{
			dAngle = IR_getSrvAngle() * 25;
			angle = degree2radian(dAngle);
		}
	}
	if(dAngle > 0)
		direction = TURN_RIGHT;
	else direction = TURN_LEFT;
	/*
	if(IR_Ctrl.Ls0Margin <= 63 && IR_Ctrl.Ls0Margin != 0){
		//��ȸ�� ���ɼ� ����
		if(IR_Ctrl.Ls1Margin <= 63){
			//��ȸ��
			s = (IR_Ctrl.Ls0Margin - 64) * MILLIMETER_PER_PIXEL * cos(CAMERA_ANGLE);
			angle = atan2(s, CAMERA_DISTANCE);
			dAngle = radian2degree(angle);
		}
	}
	else if(IR_Ctrl.Ls0Margin == 0){
		if(IR_Ctrl.Ls1Margin>=63){
			s=(IR_Ctrl.Ls1Margin - 64) * MILLIMETER_PER_PIXEL * cos(CAMERA_ANGLE);
			s*=-1;
			angle = atan2(s, CAMERA_DISTANCE);
			dAngle = radian2degree(angle) + 6.0;
		}

	}
	else{
		//��ȸ�� ���ɼ� ����
		if(IR_Ctrl.Ls0Margin != 0)
		{
			s = (IR_Ctrl.Ls0Margin - 64) * MILLIMETER_PER_PIXEL * cos(CAMERA_ANGLE);
			angle = atan2(s, CAMERA_DISTANCE);
			dAngle = radian2degree(angle) + 6.0;
		}
	}*/
	double newSrvAngle = dAngle / 25;										//range: -1 ~ 1, <= -25' ~ 25'
	//�ް��ϰ� Ƣ�� ���� �����ϱ� ���� (���� ����� ���� �ް��� Ƣ�� ���̶�� ���� �� �״��)
	if((srvAngle - 4.0 <= newSrvAngle) && (newSrvAngle <= srvAngle + 4.0)){
		srvAngle = newSrvAngle;
		Motor0Vol = cos(angle);
		Motor0Vol *= Motor0Vol;
		Motor0Vol *= speed;
	}
	IR_setMotor0Vol(Motor0Vol);
	IR_setSrvAngle(srvAngle);
}

void InfineonRacer_control(void){
	if(IR_Encoder.speed < MINIMUM_ENCODER_SPEED)
		speed += 0.2;
	else speed = speed0;
}


boolean InfineonRacer_isSchoolZone(void){
	//Ⱦ�ܺ����� �о����� [TRUE�� ��ȯ�ϰ�, 'isSchoolZone' flag�� ���¸� ������Ŵ], �׷��� ������ FALSE�� ��ȯ��
	//boolean flagCount = (ScanResult.flagCount[0] >= 4 && ScanResult.flagCount[1] >= 4);
	boolean whiteCount = ((ScanResult.whiteCount[0] + ScanResult.whiteCount[1]) < 30);
	if(whiteCount){
		isSchoolZone = !isSchoolZone;
		return TRUE;
	}
	else return FALSE;
}


void Obstacle_changeLane(void){
	LaneChanging = TRUE;
	if(LaneNumber == 1)
		IR_setSrvAngle(1.0f);
	else if(LaneNumber == 2)
		IR_setSrvAngle(-1.0f);
	IR_setMotor0Vol(0.23f);
}

void InfineonRacer_changeLane(void){
	LaneChanging = TRUE;
	if(Obstacle_Detected())
		return;
	float32 srvAngle = IR_getSrvAngle();
	double s;
	//int leftBoundary = ScanResult.bdyInx[0], rightBoundary = ScanResult.bdyInx[1];
	IR_Ctrl.Ls0Margin = ScanResult.boundaryIndex[0][0];
	IR_Ctrl.Ls1Margin = ScanResult.boundaryIndex[1][0];
	if(LaneNumber == 1){
		if(IR_Ctrl.Ls1Margin >= 58){
			Detect_Lane = InfineonRacer_detectLane;
			LaneChanging = FALSE;
			return;
		}
		if(IR_Ctrl.Ls1Margin == 0)
			srvAngle = -0.5f;
		else{
			srvAngle = -0.5f;
			sint32 margin = 63 - IR_Ctrl.Ls1Margin;
			srvAngle *= sin(degree2radian(2.72727272*margin));
		}
	}
	else if(LaneNumber == 2){
		if(IR_Ctrl.Ls0Margin >= 58){
			Detect_Lane = InfineonRacer_detectLane;
			LaneChanging = FALSE;
			return;
		}
		if(IR_Ctrl.Ls0Margin == 0)
			srvAngle = 0.5f;
		else{
			srvAngle = 0.5f;
			sint32 margin = 63 - IR_Ctrl.Ls0Margin;
			srvAngle *= sin(degree2radian(2.72727272*margin));
		}
	}
	IR_setSrvAngle(srvAngle);
}

void InfineonRacer_AEB(void){
	float32 cMotor0Vol = IR_getMotor0Vol();
	cMotor0Vol >>= 1;
	if(IR_getEncSpeed() < MINIMUM_ENCODER_SPEED){
		IR_setMotor0Vol(0.0f);
		IR_setMotor0En(FALSE);
		return;
	}
	else if(cMotor0Vol > 0)
		 cMotor0Vol *= -1;
	else
		cMotor0Vol >>= 1;
	IR_setMotor0Vol(cMotor0Vol);
}

double degree2radian(double degree){
	return degree*0.01744444;
}

double radian2degree(double radian){
	return radian*57.29577951;
}

