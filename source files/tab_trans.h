/*  Author: MorseMeow
	Date: 28 July, 2020 
*/

/**
 * @file tab_trans.h
 *
 */

#ifndef __TAB_TRANS_H__
#define __TAB_TRANS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#ifdef LV_CONF_INCLUDE_SIMPLE
#include <lvgl.h>
#else
#include "lvgl.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
struct sportTime {

	unsigned long start_ms;
	unsigned long duration_s;
	uint8_t hour;
	uint8_t minu;
    uint8_t seco;
} sport_time;

struct BME280 {

	float temp;
	float humi;
    float pres;
	float alti;
} *BME280, BME280_Value;

// struct BME280 {

// 	float temp;
// 	float humi;
//  float pres;
// 	float alti;
// };

struct BMI160 {

	uint16_t start_step;
	uint16_t step;
	float dist;
	float ener;
	float spee;
} BMI160_Value;

// struct BMM150 {

	
// } BMM150_Value;

// struct OPT3001 {


// } OPT3001_Value;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a demo application
 */
void tabview_create(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // __TAB_TRANS_H__