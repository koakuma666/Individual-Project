/*  Author: MorseMeow
	Date: 28 July, 2020 
*/


#include <Arduino.h>
#include <lvgl.h>
#include <tab_trans.h>
#include <IT725X.h>
#include <Ticker.h>
#include <TFT_eSPI.h>
#include <stdlib.h>
// #include "sdl2/SDL.h"
// include files for BMI160
#include <DFRobot_BMI160.h>
// include files for BMM150
// #include <Adafruit_BMM150.h>
// include files for BME280
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// include files for OPT3001
#include <ClosedCube_OPT3001.h>
// include files for drv2605
#include <Adafruit_DRV2605.h>

#define LVGL_TICK_PERIOD 1
#define COLOR_BUF_SIZE (LV_HOR_RES_MAX * 200)

#if defined ARDUINO_AVR_UNO || defined ARDUINO_AVR_MEGA2560 || defined ARDUINO_AVR_PRO
  //interrupt number of uno and mega2560 is 0
  int pbIn = 2;
#elif ARDUINO_AVR_LEONARDO
  //interrupt number of uno and leonardo is 0
  int pbIn = 3; 
#else
  int pbIn = 13;
#endif
/*the bmi160 have two interrput interfaces*/
#define int1 1
#define int2 2

// sedentary remind period, unit is second
#define SEDEN_CYCLE 120
#define SEDEN_THRES 50  // set the steps during remind time, if not satisfied with this value, the remind will be displayed
#define SEALEVELPRESSURE_HPA (1013.25)

int freq = 2000;      // frequency
int channel = 0;      // channel
int resolution = 8;   // resolution
const int backLight = 15;

IT725X tp = IT725X();
TP_Point point = TP_Point();

Ticker tick; /* timer for interrupt handler */
TFT_eSPI tft = TFT_eSPI();  /* TFT instance */
DFRobot_BMI160 bmi160;
Adafruit_BME280 bme;
ClosedCube_OPT3001 opt3001;
Adafruit_DRV2605 drv;
static lv_disp_buf_t disp_buf;
static lv_color_t buf_1[COLOR_BUF_SIZE];
// static lv_color_t buf_2[COLOR_BUF_SIZE];

bool readStep = false;
extern volatile bool active_en;
extern volatile bool active_change;
extern volatile bool seden_en;
extern volatile bool seden_change;
extern struct BME280 BME280_Value;

#if USE_LV_LOG != 0
/* Serial debugging */
void my_print(lv_log_level_t level, const char * file, uint32_t line, const char * dsc)
{
    Serial.printf("%s@%d->%s\r\n", file, line, dsc);
    Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1 + 40, w, h);
    tft.pushColors(&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

/* Reading input device (simulated encoder here) */
bool read_touchscreen(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
{
	// int status = tp.touched();
	// if(status) {
		uint8_t num = tp.getPoint(&point);
		if(num) {
			// Serial.printf("x:%d  y:%d\r\n", point.x, point.y);
			// lcd.fillScreen(COLOR_WHITE);
			// lcd.drawFastVLine(point.x, 0, 320, COLOR_BLACK);
			// lcd.drawFastHLine(0, point.y + 40, 240, COLOR_BLACK);
		}
	// }
	data->point.x = point.x;
	data->point.y = point.y;
	data->state = num ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
	return false;  /*No buffering now so no more data read*/
}

/* Interrupt driven periodic handler */
static void lv_tick_handler(void)
{
    lv_tick_inc(LVGL_TICK_PERIOD);
}

void read_sensors(void)
{
    // step
    uint16_t stepValue = 0;
    static uint16_t stepCounter = 0;  // 
    static uint16_t seden_start_step = 0;  // 

    if(readStep) {
        // read step counter from hardware bmi160 
        if(bmi160.readStepCounter(&stepValue) == BMI160_OK) {
            stepCounter = stepValue;
        }
        // readStep = false;
    }

    if(!seden_en && !seden_change) {
        // Determine if the time has reached 20ms
        unsigned long seden_duration = millis() / 1000;
        if(!(seden_duration % SEDEN_CYCLE)) {
            // Determine whether the number of steps exceeds the threshold
            if(stepCounter - seden_start_step < SEDEN_THRES) {
                seden_en = true;
                seden_change = true;
                Serial.println("seden_alarm");
                // Set the LRA vibration mode after the pop-up remind window
                drv.setWaveform(0, 84); // ramp up medium 1, see datasheet part 11.2
                drv.setWaveform(1, 84); // ramp up medium 1, see datasheet part 11.2
                drv.setWaveform(2, 1);  // strong click 100%, see datasheet part 11.2
                drv.setWaveform(3, 0);  // end of waveforms
            }
            seden_start_step = stepCounter;
        }
    }
    if(seden_en && !seden_change) {
        // LRA vibration
        drv.go();
    }

    if(active_change) {
        active_change = false;
        if(active_en) {
            if(bmi160.readStepCounter(&stepValue) == BMI160_OK) {
                BMI160_Value.start_step = stepValue;
            }
        }
        else {
            // Reset the stepCounter register of BMI160

        }
        // Set the LRA to vibrate after clicking the button
        drv.setWaveform(0, 1);  // strong click 100%, see datasheet part 11.2
        drv.setWaveform(1, 0);  // end of waveforms
        drv.go();
    }
    if(active_en) {
        unsigned long duration = millis() - sport_time.start_ms;
        duration = duration / 1000;
        sport_time.duration_s = duration;
        sport_time.hour = duration / 3600;
        duration = duration % 3600;
        sport_time.minu = duration / 60;
        sport_time.seco = duration % 60;
        // step
        // if(readStep) {
        //     // read step counter from hardware bmi160 
        //     if(bmi160.readStepCounter(&stepCounter) == BMI160_OK) {
        //         BMI160_Value.step = stepCounter - BMI160_Value.start_step;
        //         Serial.printf("step counter = %d %d\n", stepCounter, BMI160_Value.step);
        //     }
        //     readStep = false;
        // }

        // distance and energy calculation
        if(readStep) {
            BMI160_Value.step = stepCounter - BMI160_Value.start_step;
            // Serial.printf("step counter = %d, %d\n", stepCounter, BMI160_Value.step);
            BMI160_Value.dist = BMI160_Value.step * 0.42;
            BMI160_Value.ener = BMI160_Value.step * 0.5;
        }
        // Real-time refresh
        BMI160_Value.spee = BMI160_Value.dist / sport_time.duration_s;
    }
    readStep = false;

    BME280_Value.temp = bme.readTemperature();
	BME280_Value.humi = bme.readHumidity();
	BME280_Value.pres = (bme.readPressure() / 100.0F);
	BME280_Value.alti = bme.readAltitude(SEALEVELPRESSURE_HPA);

    // Serial.printf("Temperature = %.2f, %.2f *C\n", bme.readTemperature(), BME280_Value.temp);
    // Serial.printf("Humidity = %.2f, %.2f %%\n", bme.readHumidity(), BME280_Value.humi);

    // BME280->temp = bme.readTemperature();
	// BME280->humi = bme.readHumidity();
	// BME280->pres = (bme.readPressure() / 100.0F);
	// BME280->alti = bme.readAltitude(SEALEVELPRESSURE_HPA);

    OPT3001 result = opt3001.readResult();
    if (result.error == NO_ERROR) {
        Serial.printf("OPT3001 : %.2f lux\n", result.lux);
        ledcWrite(channel, map(result.lux, 0, 200, 50, 250));
    }
}

void stepChange()
{
    // once the step conter is changed, the value can be read 
    readStep = true;
}

void configureSensor() {
	OPT3001_Config newConfig;
	
	newConfig.RangeNumber = B1100;	
	newConfig.ConvertionTime = B0;
	newConfig.Latch = B1;
	newConfig.ModeOfConversionOperation = B11;

	OPT3001_ErrorCode errorConfig = opt3001.writeConfig(newConfig);
	if (errorConfig != NO_ERROR)
		Serial.printf("OPT3001 configuration %d", errorConfig);
	else {
		OPT3001_Config sensorConfig = opt3001.readConfig();
		Serial.println("OPT3001 Current Config:");
		Serial.println("------------------------------");
		
		Serial.print("Conversion ready (R):");
		Serial.println(sensorConfig.ConversionReady,HEX);

		Serial.print("Conversion time (R/W):");
		Serial.println(sensorConfig.ConvertionTime, HEX);

		Serial.print("Fault count field (R/W):");
		Serial.println(sensorConfig.FaultCount, HEX);

		Serial.print("Flag high field (R-only):");
		Serial.println(sensorConfig.FlagHigh, HEX);

		Serial.print("Flag low field (R-only):");
		Serial.println(sensorConfig.FlagLow, HEX);

		Serial.print("Latch field (R/W):");
		Serial.println(sensorConfig.Latch, HEX);

		Serial.print("Mask exponent field (R/W):");
		Serial.println(sensorConfig.MaskExponent, HEX);

		Serial.print("Mode of conversion operation (R/W):");
		Serial.println(sensorConfig.ModeOfConversionOperation, HEX);

		Serial.print("Polarity field (R/W):");
		Serial.println(sensorConfig.Polarity, HEX);

		Serial.print("Overflow flag (R-only):");
		Serial.println(sensorConfig.OverflowFlag, HEX);

		Serial.print("Range number (R/W):");
		Serial.println(sensorConfig.RangeNumber, HEX);

		Serial.println("------------------------------");
	}
}

void setup()
{
    ledcSetup(channel, freq, resolution); // Set channel
    ledcAttachPin(backLight, channel);  // Connect the channel to the corresponding pin
    Serial.begin(115200); /* prepare for possible serial debug */

    // set and init the bmi160 i2c address
    while(bmi160.I2cInit(0x69) != BMI160_OK) {
        Serial.println("i2c init fail");
        delay(1000);
    }
    // set interrput number to int1 or int2
    if(bmi160.setInt(int2) != BMI160_OK) {
        Serial.println("set interrput fail");
        while(1);
    }
    // set the bmi160 mode to step counter
    if(bmi160.setStepCounter() != BMI160_OK) {
        Serial.println("set step fail");
        while(1);
    }
    // set the bmi160 power model,contains:stepNormalPowerMode,stepLowPowerMode
    if(bmi160.setStepPowerMode(bmi160.stepNormalPowerMode) != BMI160_OK) {
        Serial.println("set setStepPowerMode fail");
        while(1);
    }
#if defined ARDUINO_AVR_UNO || defined ARDUINO_AVR_MEGA2560 || defined ARDUINO_AVR_LEONARDO || defined ARDUINO_AVR_PRO
    // set the pin in the board to connect to int1 or int2 of bmi160
    attachInterrupt(digitalPinToInterrupt(pbIn), stepChange, FALLING);
#else
    attachInterrupt(pbIn, stepChange, FALLING);
#endif

	Serial.println(F("BME280 test"));
    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    unsigned status = bme.begin();
    Serial.printf("i2cFrequency = %d\n", Wire.getClock());
    if(!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(), 16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        while (1);
    }
	Serial.println("-- Default Test --");

    opt3001.begin(0x47);
    Serial.print("OPT3001 Manufacturer ID");
	Serial.println(opt3001.readManufacturerID());
	Serial.print("OPT3001 Device ID");
	Serial.println(opt3001.readDeviceID());

    configureSensor();
	Serial.printf("High-Limit %d\n", opt3001.readHighLimit());
	Serial.printf("Low-Limit %d\n", opt3001.readLowLimit());
	Serial.println("--------------------------");

    Serial.println("-- DRV test --");
    drv.begin();
    drv.useLRA();
    // I2C trigger by sending 'go' command 
    drv.setMode(DRV2605_MODE_INTTRIG); // default, internal trigger when sending GO command
    drv.selectLibrary(6);
    //drv.setWaveform(0, 84); // ramp up medium 1, see datasheet part 11.2
    drv.setWaveform(1, 1);  // strong click 100%, see datasheet part 11.2
    drv.setWaveform(2, 0);  // end of waveforms

    lv_init();

#if USE_LV_LOG != 0
    lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

    tp.begin(Wire1, 12, 19);
    Serial.printf("i2cFrequency = %d\n", Wire.getClock());
    tft.begin();        /* TFT init */
    tft.setRotation(0); /* Landscape orientation */

    lv_disp_buf_init(&disp_buf, buf_1, NULL, COLOR_BUF_SIZE);
    
    /*Initialize the display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the touch pad*/
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = read_touchscreen;
    lv_indev_drv_register(&indev_drv);

    /*Initialize the graphics library's tick*/
    tick.attach_ms(LVGL_TICK_PERIOD, lv_tick_handler);

    // demo_create();
    tabview_create();
}

void loop()
{
    read_sensors();
    lv_task_handler(); /* let the GUI do its work */
}
