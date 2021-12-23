#include "AppTaskFu.h"

static sint32 task_cnt_1m = 0;
static sint32 task_cnt_10m = 0;
static sint32 task_cnt_100m = 0;
static sint32 task_cnt_1000m = 0;

boolean task_flag_1m = FALSE;
boolean task_flag_10m = FALSE;
boolean task_flag_100m = FALSE;
boolean task_flag_1000m = FALSE;
void (*Obstacle_func)();
boolean Obstacle_Detected;

void appTaskfu_init(void){
	/******************************
	 * Initialize Default Modules *
	 ******************************/
	BasicLineScan_init();
	BasicPort_init();
    BasicGtmTom_init();
    BasicVadcBgScan_init();
    BasicGpt12Enc_init();
    AsclinShellInterface_init();
    InfineonRacer_init();
	/*******************************
	 **Initialize Function Pointer**
	 *******************************/
    Obstacle_func = InfineonRacer_AEB;
    Detect_Lane = InfineonRacer_detectLane;
	/********************
	 * Initialize Flags *
	 ********************/
    Obstacle_Detected = FALSE;
    isSchoolZone = FALSE;
}

void appTaskfu_1ms(void){
	task_cnt_1m++;
	if(task_cnt_1m == 1000){
		task_cnt_1m = 0;
	}
}


void appTaskfu_10ms(void){
	task_cnt_10m++;
	if(task_cnt_10m == 1000){
		task_cnt_10m = 0;
	}
	if(task_cnt_10m%6 == 0){
		BasicLineScan_run();
		Detect_Lane();
		InfineonRacer_init();
		AsclinShellInterface_runLineScan();
	}
	if(task_cnt_10m%2 == 0){
		BasicVadcBgScan_run();
		if(ObstacleDetected()){
			Obstacle_func();
			Detect_Lane = InfineonRacer_changeLane();
			IR_setLed1(TRUE);
		}
		else{
			if(IR_getMotor0En() == FALSE)
				IR_setMotor0En(TRUE);
			IR_setLed1(FALSE);
		}
		BasicPort_run();
		BasicGtmTom_run();
	}
}

void appTaskfu_100ms(void){
	task_cnt_100m++;
	if(task_cnt_100m == 1000){
		task_cnt_100m = 0;
	}
	if(task_cnt_100m % 6){
		//스쿨존인지 판별
		if(InfineonRacer_isSchoolZone()){
			if(isSchoolZone){
				Obstacle_func = Obstacle_changeLane;
				IR_setLed2(TRUE);
			}
			else{
				LaneNumber = 0;
				Obstacle_func = InfineonRacer_AEB;
				IR_setLed2(FALSE);
			}
		}
	}
}

void appTaskfu_1000ms(void){
	task_cnt_1000m++;
	if(task_cnt_1000m == 1000){
		task_cnt_1000m = 0;
	}
	hasInitialed = TRUE;

}

void appTaskfu_idle(void){
	AsclinShellInterface_run();

}

void appIsrCb_1ms(void){
	BasicGpt12Enc_run();
}

