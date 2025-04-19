#define FORWARD 0
#define REVERSE 1
#define CLOCK_MODE 1
#define CV_MODE 0
#define POSITIVE_V 0
#define NEGATIVE_V 1
#define RUN_GATE 0
#define RUN_TRIG 1
#define OUT_TRIG 0
#define OUT_GATE 1
#define OUT_CLOCK 2
#define CV_TYPE 0

#define COLOR_LCD_RED 0xdd, 0x33, 0x33, 0xff
#define COLOR_LCD_GREEN 0x33, 0xdd, 0x33, 0xff

#define STD2x_PROGRESSION 0
#define P_1_3_PROGRESSION 2 // 1.3x
#define FIBONACCI_PROGRESSION 3

#define BIT_8 0
#define BIT_16 1

#include "plugin.hpp"
#include "osdialog.h"
#if defined(METAMODULE)
#include "async_filebrowser.hh"
#endif
#include <dirent.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

struct TrigSeq8x : Module {
	enum ParamId {
		ENUMS(STEP_PARAM, 128),
		LENGTH_PARAM,
		MODE_SWITCH,
		RST_PARAM,
		RUNBUT_PARAM,
		PROG_PARAM,
		RECALL_PARAM,
		STORE_PARAM,
		SET_PARAM,
		AUTO_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CLK_INPUT,
		REV_INPUT,
		RUN_INPUT,
		RST_INPUT,
		LENGTH_INPUT,
		PROG_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUT, 8),
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(STEPBUT_LIGHT, 128),
		ENUMS(STEP_LIGHT, 16),
		RUNBUT_LIGHT,
		RECALL_LIGHT,
		STORE_LIGHT,
		SET_LIGHT,
		AUTO_LIGHT,
		LIGHTS_LEN
	};

	float clkValue = 0;
	float prevClkValue = 0;

	float rstValue = 0;
	float prevRstValue = 0;

	bool direction = FORWARD;

	float out[8] = {0,0,0,0,0,0,0,0};

	int step = 0;

	bool runSetting = true;
	bool prevRunSetting = true;

	float runButton = 0;
	float runTrig = 0.f;
	float prevRunTrig = 0.f;

	int range = 9;

	bool initStart = false;
	int recStep = 0;

	int revType = POSITIVE_V;
	int runType = RUN_GATE;

	float oneMsTime = (APP->engine->getSampleRate()) / 1000;
	bool stepPulse[8] = {false, false, false, false, false, false, false, false};
	float stepPulseTime[8] = {0,0,0,0,0,0,0,0};

	int maxSteps = 16;
	int mode = 0;
	int prevMode = 1;

	int currAddr = 0;
	int prevAddr = 0;

	int outType = OUT_TRIG;
	bool rstOnRun = true;
	bool dontAdvance = false;
	bool dontAdvanceSetting = true;

	bool outGate[8] = {false, false, false, false, false, false, false, false};

	int progSeq[32][8][16] = {
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								},
								{	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
								}
							};

	int progSteps[32] = {16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16};
	float progRst[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	// --------------workingSeq

	int wSeq[8][16] = { {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
						{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
						{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
						{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
						{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
						{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
						{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
						{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
					};
	int wSteps = 16;
	float wRst = 0;

	int nextSeq[8][16] = { {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
						};
	int nextSteps = 16;
	float nextRst = 0;
	/*
	int pendingSeq[8][16] = { {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
							{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
						};
	
	int pendingSteps = 16;
	float pendingRst = 0;
	*/
	// --------------prog
	int progKnob = 0;
	int prevProgKnob = 0;
	int savedProgKnob = 0;

	float progTrig = 0;
	float prevProgTrig = 0;

	int selectedProg = 0;
	bool progChanged = false;

	float recallBut = 0;
	float prevRecallBut = 0;

	// --------------store
	float storeBut = 0;
	float prevStoreBut = 0;

	bool storeWait = false;
	float storeTime = 0;
	float storeSamples = APP->engine->getSampleRate() / 1.5f;

	bool storedProgram = false;
	int storedProgramTime = 0;
	float maxStoredProgramTime = APP->engine->getSampleRate() * 1.5;

	// -------------- working

	int workingProg = 0;

	bool instantProgChange = false;

	bool progToSet = false;
	float progSetBut = 0;
	float prevProgSetBut = 0;

	// ------- set button light

	bool setButLight = false;
	float setButLightDelta = 2 / APP->engine->getSampleRate();
	float setButLightValue = 0.f;

	float volt[8] = {0,0,0,0,0,0,0,0};

	int progression = STD2x_PROGRESSION;
	
	const float tableVolts[3][2][16] = {
		{
			{0.03922, 0.07844, 0.15688, 0.31376, 0.62752, 1.25504, 2.51008, 5.02016, 0, 0, 0, 0, 0, 0, 0, 0},
			{0.00015259, 0.000305181, 0.000610361, 0.001220722, 0.002441445, 0.00488289, 0.009765779, 0.019531558, 0.039063117, 0.078126234, 0.156252467, 0.312504934, 0.625009869, 1.250019738, 2.500039475, 5.00007895}
		},
		{
			{0.4191521, 0.54489773, 0.708367049, 0.920877164, 1.197140313, 1.556282407, 2.023167129, 2.630117267, 0, 0, 0, 0, 0, 0, 0, 0},
			{0.04577242, 0.059504146, 0.07735539, 0.100562007, 0.130730609, 0.169949791, 0.220934729, 0.287215147, 0.373379692, 0.485393599, 0.631011679, 0.820315183, 1.066409737, 1.386332659, 1.802232456, 2.342902193}
		},
		{
			{0.114942529, 0.229885058, 0.344827586, 0.574712644, 0.91954023, 1.494252874, 2.413793105, 3.908045979, 0, 0, 0, 0, 0, 0, 0, 0},
			{0.002392917, 0.004785834, 0.007178751, 0.011964585, 0.019143336, 0.031107921, 0.050251256, 0.081359177, 0.131610433, 0.21296961, 0.344580044, 0.557549654, 0.902129698, 1.459679352, 2.361809049, 3.821488401}
		}
	};
	
	const int bitResTable[2] = {8, 16};
	int bitResolution = BIT_8;

	bool turingMode = false;
	bool prevTuringMode = false;

	int progInType = CV_TYPE;
	int lastProg = 0;

	TrigSeq8x() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(MODE_SWITCH, 0.f, 1.f, 1.f, "Mode", {"Cv", "Clock"});
		configInput(CLK_INPUT, "Clock");
		configInput(REV_INPUT, "Reverse");

		configSwitch(RUNBUT_PARAM, 0.f, 1.f, 1.f, "Run", {"OFF", "ON"});
		configInput(RUN_INPUT, "Run");
		
		struct ResetQuantity : ParamQuantity {
			float getDisplayValue() override {
				TrigSeq8x* module = reinterpret_cast<TrigSeq8x*>(this->module);
				if (!module->turingMode) {
					name = "Reset Step";
					unit = "";
					displayMultiplier = 15.f;
					displayOffset = 1.f;
				} else {
					name = "Out Atten.";
					unit = "%";
					displayOffset = 0;
					displayMultiplier = 100.f;
				}
				return ParamQuantity::getDisplayValue();
			}
		};
		//configParam(RST_PARAM, 1.f,16.f, 1.f, "Rst Step / Atten.");
		configParam<ResetQuantity>(RST_PARAM, 0.f,1.f, 0.f, "Rst Step / Atten.");
		//paramQuantities[RST_PARAM]->snapEnabled = true;
		configInput(RST_INPUT, "Reset");

		configParam(LENGTH_PARAM, 1.f,16.f, 16.f, "Length");
		paramQuantities[LENGTH_PARAM]->snapEnabled = true;

		configInput(LENGTH_INPUT, "Length");

		for (int t = 0; t < 8; t++) {
			for (int step = 0; step < 16; step++)
				configSwitch(STEP_PARAM+(t*16)+step, 0.f, 1.f, 0.f, ("Tr.#"+to_string(t+1)+" Step#"+to_string(step+1)).c_str(), {"OFF", "ON"});
			configOutput(OUT_OUTPUT+t, ("Tr.#"+to_string(t+1)).c_str());
		}

		configParam(PROG_PARAM, 0.f, 31.f, 0.f, "Prog");
		configInput(PROG_INPUT, "Prog");
		paramQuantities[PROG_PARAM]->snapEnabled = true;
		configSwitch(SET_PARAM, 0, 1.f, 0.f, "Set", {"OFF", "ON"});
		configSwitch(RECALL_PARAM, 0, 1.f, 0.f, "Recall", {"OFF", "ON"});
		//configInput(RECALL_INPUT, "Recall");
		configSwitch(STORE_PARAM, 0, 1.f, 0.f, "Store", {"OFF", "ON"});
		configSwitch(AUTO_PARAM, 0, 1.f, 0.f, "Auto", {"OFF", "ON"});

	}

	void onReset(const ResetEvent &e) override {

		initStart = false;

		step = 0;

		lights[STEP_LIGHT].setBrightness(1);
		lights[STEP_LIGHT+1].setBrightness(0);
		lights[STEP_LIGHT+2].setBrightness(0);
		lights[STEP_LIGHT+3].setBrightness(0);
		lights[STEP_LIGHT+4].setBrightness(0);
		lights[STEP_LIGHT+5].setBrightness(0);
		lights[STEP_LIGHT+6].setBrightness(0);
		lights[STEP_LIGHT+7].setBrightness(0);
		lights[STEP_LIGHT+8].setBrightness(0);
		lights[STEP_LIGHT+9].setBrightness(0);
		lights[STEP_LIGHT+10].setBrightness(0);
		lights[STEP_LIGHT+11].setBrightness(0);
		lights[STEP_LIGHT+12].setBrightness(0);
		lights[STEP_LIGHT+13].setBrightness(0);
		lights[STEP_LIGHT+14].setBrightness(0);
		lights[STEP_LIGHT+15].setBrightness(0);

		setButLight = false;
		setButLightDelta = 2 / APP->engine->getSampleRate();
		setButLightValue = 0.f;

		for (int t = 0; t < 8; t++) {
			for (int st = 0; st < 16; st++) {
				wSeq[t][st] = 0;
				lights[STEPBUT_LIGHT+(t*16)+st].setBrightness(wSeq[t][st]);
			}
		}
		wSteps = 16;
		wRst = 0;

		Module::onReset(e);
	}

	void onSampleRateChange() override {
		oneMsTime = (APP->engine->getSampleRate()) / 1000;
		storeSamples = APP->engine->getSampleRate() / 1.5f;
		maxStoredProgramTime = APP->engine->getSampleRate() * 1.5;
		setButLightDelta = 2 / APP->engine->getSampleRate();
	}

	json_t* dataToJson() override {
		if (initStart)
			recStep = 0;
		else
			recStep = step;

		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "runType", json_integer(runType));
		json_object_set_new(rootJ, "revType", json_integer(revType));
		json_object_set_new(rootJ, "outType", json_integer(outType));
		json_object_set_new(rootJ, "rstOnRun", json_boolean(rstOnRun));
		json_object_set_new(rootJ, "dontAdvanceSetting", json_boolean(dontAdvanceSetting));
		json_object_set_new(rootJ, "step", json_integer(recStep));
		json_object_set_new(rootJ, "initStart", json_boolean(initStart));

		json_object_set_new(rootJ, "turingMode", json_boolean(turingMode));
		json_object_set_new(rootJ, "bitResolution", json_integer(bitResolution));
		json_object_set_new(rootJ, "progression", json_integer(progression));

		json_object_set_new(rootJ, "savedProgKnob", json_integer(savedProgKnob));

		json_object_set_new(rootJ, "progInType", json_boolean(progInType));
		json_object_set_new(rootJ, "lastProg", json_integer(lastProg));

		for (int t = 0; t < 8; t++) {
			json_t *wSeq_json_array = json_array();
			for (int st = 0; st < 16; st++) {
				json_array_append_new(wSeq_json_array, json_integer(wSeq[t][st]));
			}
			json_object_set_new(rootJ, ("wSeq_t"+to_string(t)).c_str(), wSeq_json_array);
		}

		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < 8; t++) {
				json_t *progSeq_json_array = json_array();
				for (int st = 0; st < 16; st++) {
					json_array_append_new(progSeq_json_array, json_integer(progSeq[p][t][st]));
				}
				json_object_set_new(rootJ, ("p"+to_string(p)+"t"+to_string(t)).c_str(), progSeq_json_array);
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *progSteps_json_array = json_array();
			json_array_append_new(progSteps_json_array, json_integer(progSteps[p]));
			json_object_set_new(rootJ, ("progSteps"+to_string(p)).c_str(), progSteps_json_array);
		}

		for (int p = 0; p < 32; p++) {
			json_t *progRst_json_array = json_array();
			json_array_append_new(progRst_json_array, json_real(progRst[p]));
			json_object_set_new(rootJ, ("progRst"+to_string(p)).c_str(), progRst_json_array);
		}

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		
		json_t* runTypeJ = json_object_get(rootJ, "runType");
		if (runTypeJ) {
			runType = json_integer_value(runTypeJ);
			if (runType < 0 || runType > 1)
				runType = 0;
		}

		json_t* revTypeJ = json_object_get(rootJ, "revType");
		if (revTypeJ) {
			revType = json_integer_value(revTypeJ);
			if (revType < 0 || revType > 1)
				revType = 0;
		}

		json_t* outTypeJ = json_object_get(rootJ, "outType");
		if (outTypeJ) {
			outType = json_integer_value(outTypeJ);
			if (outType < 0 || outType > 2)
				outType = 0;
		}

		json_t* rstOnRunJ = json_object_get(rootJ, "rstOnRun");
		if (rstOnRunJ) {
			rstOnRun = json_boolean_value(rstOnRunJ);
		}

		json_t* dontAdvanceSettingJ = json_object_get(rootJ, "dontAdvanceSetting");
		if (dontAdvanceSettingJ) {
			dontAdvanceSetting = json_boolean_value(dontAdvanceSettingJ);
		}

		json_t* stepJ = json_object_get(rootJ, "step");
		if (stepJ) {
			step = json_integer_value(stepJ);
			if (step < 0 || step > 15)
				step = 0;
			lights[STEP_LIGHT+step].setBrightness(1);

		} 

		json_t* initStartJ = json_object_get(rootJ, "initStart");
		if (initStartJ) {
			initStart = json_boolean_value(initStartJ);
			if (initStart)
				step = 0;
		}

		json_t* turingModeJ = json_object_get(rootJ, "turingMode");
		if (turingModeJ) {
			turingMode = json_boolean_value(turingModeJ);
		}

		json_t* bitResolutionJ = json_object_get(rootJ, "bitResolution");
		if (bitResolutionJ) {
			bitResolution = json_integer_value(bitResolutionJ);
			if (bitResolution < 0 && bitResolution > 1)
				bitResolution = BIT_8;
		}

		json_t* progressionJ = json_object_get(rootJ, "progression");
		if (progressionJ) {
			progression = json_integer_value(progressionJ);
			if (progression < 0 && progression > 2)
				progression = STD2x_PROGRESSION;
		}

		// ----------------

		json_t* savedProgKnobJ = json_object_get(rootJ, "savedProgKnob");
		if (savedProgKnobJ) {
			savedProgKnob = json_integer_value(savedProgKnobJ);
			if (savedProgKnob < 0 || savedProgKnob > 31)
				savedProgKnob = 0;
			
		}

		json_t* progInTypeJ = json_object_get(rootJ, "progInType");
		if (progInTypeJ) {
			progInType = json_boolean_value(progInTypeJ);
		}

		json_t* lastProgJ = json_object_get(rootJ, "lastProg");
		if (lastProgJ) {
			lastProg = json_integer_value(lastProgJ);
			if (lastProg < 0 || lastProg > 31)
				lastProg = 0;
		}

		selectedProg = savedProgKnob;
		workingProg = selectedProg;
		prevProgKnob = selectedProg;
		params[PROG_PARAM].setValue(selectedProg);

		for (int t = 0; t < 8; t++) {
			json_t *wSeq_json_array = json_object_get(rootJ, ("wSeq_t"+to_string(t)).c_str());
			size_t st;
			json_t *wSeq_json_value;
			if (wSeq_json_array) {
				json_array_foreach(wSeq_json_array, st, wSeq_json_value) {
					params[STEP_PARAM+(t*16)+st].setValue(json_integer_value(wSeq_json_value));
				}
			}
		}
		
		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < 8; t++) {
				json_t *prog_json_array = json_object_get(rootJ, ("p"+to_string(p)+"t"+to_string(t)).c_str());
				size_t st;
				json_t *prog_json_value;
				if (prog_json_array) {
					json_array_foreach(prog_json_array, st, prog_json_value) {
						progSeq[p][t][st] = json_integer_value(prog_json_value);
					}
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *progSteps_json_array = json_object_get(rootJ, ("progSteps"+to_string(p)).c_str());
			size_t jSteps;
			json_t *progSteps_json_value;
			if (progSteps_json_array) {
				json_array_foreach(progSteps_json_array, jSteps, progSteps_json_value) {
					progSteps[p] = json_integer_value(progSteps_json_value);
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *progRst_json_array = json_object_get(rootJ, ("progRst"+to_string(p)).c_str());
			size_t jRst;
			json_t *progRst_json_value;
			if (progRst_json_array) {
				json_array_foreach(progRst_json_array, jRst, progRst_json_value) {
					progRst[p] = json_real_value(progRst_json_value);
				}
			}
		}

	}

	json_t *presetToJson() {

		json_t *rootJ = json_object();

		json_object_set_new(rootJ, "runType", json_integer(runType));
		json_object_set_new(rootJ, "revType", json_integer(revType));
		json_object_set_new(rootJ, "outType", json_integer(outType));
		json_object_set_new(rootJ, "rstOnRun", json_boolean(rstOnRun));
		json_object_set_new(rootJ, "dontAdvanceSetting", json_boolean(dontAdvanceSetting));

		json_object_set_new(rootJ, "turingMode", json_boolean(turingMode));
		json_object_set_new(rootJ, "bitResolution", json_integer(bitResolution));
		json_object_set_new(rootJ, "progression", json_integer(progression));

		json_object_set_new(rootJ, "progInType", json_boolean(progInType));
		json_object_set_new(rootJ, "lastProg", json_integer(lastProg));

		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < 8; t++) {
				json_t *progSeq_json_array = json_array();
				for (int st = 0; st < 16; st++) {
					json_array_append_new(progSeq_json_array, json_integer(progSeq[p][t][st]));
				}
				json_object_set_new(rootJ, ("p"+to_string(p)+"t"+to_string(t)).c_str(), progSeq_json_array);
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *progSteps_json_array = json_array();
			json_array_append_new(progSteps_json_array, json_integer(progSteps[p]));
			json_object_set_new(rootJ, ("progSteps"+to_string(p)).c_str(), progSteps_json_array);
		}

		for (int p = 0; p < 32; p++) {
			json_t *progRst_json_array = json_array();
			json_array_append_new(progRst_json_array, json_real(progRst[p]));
			json_object_set_new(rootJ, ("progRst"+to_string(p)).c_str(), progRst_json_array);
		}

		return rootJ;
	}

	void presetFromJson(json_t *rootJ) {

		json_t* runTypeJ = json_object_get(rootJ, "runType");
		if (runTypeJ) {
			runType = json_integer_value(runTypeJ);
			if (runType < 0 || runType > 1)
				runType = 0;
		}

		json_t* revTypeJ = json_object_get(rootJ, "revType");
		if (revTypeJ) {
			revType = json_integer_value(revTypeJ);
			if (revType < 0 || revType > 1)
				revType = 0;
		}

		json_t* outTypeJ = json_object_get(rootJ, "outType");
		if (outTypeJ) {
			outType = json_integer_value(outTypeJ);
			if (outType < 0 || outType > 2)
				outType = 0;
		}

		json_t* rstOnRunJ = json_object_get(rootJ, "rstOnRun");
		if (rstOnRunJ) {
			rstOnRun = json_boolean_value(rstOnRunJ);
		}

		json_t* dontAdvanceSettingJ = json_object_get(rootJ, "dontAdvanceSetting");
		if (dontAdvanceSettingJ) {
			dontAdvanceSetting = json_boolean_value(dontAdvanceSettingJ);
		}

		json_t* turingModeJ = json_object_get(rootJ, "turingMode");
		if (turingModeJ) {
			turingMode = json_boolean_value(turingModeJ);
		}

		json_t* bitResolutionJ = json_object_get(rootJ, "bitResolution");
		if (bitResolutionJ) {
			bitResolution = json_integer_value(bitResolutionJ);
			if (bitResolution < 0 && bitResolution > 1)
				bitResolution = BIT_8;
		}

		json_t* progressionJ = json_object_get(rootJ, "progression");
		if (progressionJ) {
			progression = json_integer_value(progressionJ);
			if (progression < 0 && progression > 2)
				progression = STD2x_PROGRESSION;
		}

		json_t* progInTypeJ = json_object_get(rootJ, "progInType");
		if (progInTypeJ) {
			progInType = json_boolean_value(progInTypeJ);
		}

		json_t* lastProgJ = json_object_get(rootJ, "lastProg");
		if (lastProgJ) {
			lastProg = json_integer_value(lastProgJ);
			if (lastProg < 0 || lastProg > 31)
				lastProg = 0;
		}

		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < 8; t++) {
				json_t *prog_json_array = json_object_get(rootJ, ("p"+to_string(p)+"t"+to_string(t)).c_str());
				size_t st;
				json_t *json_value;
				if (prog_json_array) {
					json_array_foreach(prog_json_array, st, json_value) {
						progSeq[p][t][st] = json_integer_value(json_value);
					}
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *progSteps_json_array = json_object_get(rootJ, ("progSteps"+to_string(p)).c_str());
			size_t jSteps;
			json_t *progSteps_json_value;
			if (progSteps_json_array) {
				json_array_foreach(progSteps_json_array, jSteps, progSteps_json_value) {
					progSteps[p] = json_integer_value(progSteps_json_value);
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *progRst_json_array = json_object_get(rootJ, ("progRst"+to_string(p)).c_str());
			size_t jRst;
			json_t *progRst_json_value;
			if (progRst_json_array) {
				json_array_foreach(progRst_json_array, jRst, progRst_json_value) {
					progRst[p] = json_real_value(progRst_json_value);
				}
			}
		}
	}

	void menuLoadPreset() {
		static const char FILE_FILTERS[] = "trigSeq8x preset (.t8p):t8p,T8P";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
#endif
		if (path)
			loadPreset(path);

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void loadPreset(std::string path) {

		FILE *file = fopen(path.c_str(), "r");
		json_error_t error;
		json_t *rootJ = json_loadf(file, 0, &error);
		if (rootJ == NULL) {
			WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		}

		fclose(file);

		if (rootJ) {

			presetFromJson(rootJ);

		} else {
			WARN("problem loading preset json file");
			//return;
		}
		
	}

	void menuSavePreset() {

		static const char FILE_FILTERS[] = "trigSeq8x preset (.t8p):t8p,T8P";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters);
#endif
		if (path) {
			std::string strPath = path;
			if (strPath.substr(strPath.size() - 4) != ".t8p" and strPath.substr(strPath.size() - 4) != ".T8P")
				strPath += ".t8p";
			path = strcpy(new char[strPath.length() + 1], strPath.c_str());
			savePreset(path, presetToJson());
		}

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void savePreset(std::string path, json_t *rootJ) {

		if (rootJ) {
			FILE *file = fopen(path.c_str(), "w");
			if (!file) {
				WARN("[ SickoCV ] cannot open '%s' to write\n", path.c_str());
				//return;
			} else {
				json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
				json_decref(rootJ);
				fclose(file);
			}
		}
	}

	// ------------------------ LOAD / SAVE   SINGLE PANEL SETTINGS

	json_t *singleToJson() {

		json_t *rootJ = json_object();

		json_object_set_new(rootJ, "runType", json_integer(runType));
		json_object_set_new(rootJ, "revType", json_integer(revType));
		json_object_set_new(rootJ, "outType", json_integer(outType));
		json_object_set_new(rootJ, "rstOnRun", json_boolean(rstOnRun));
		json_object_set_new(rootJ, "dontAdvanceSetting", json_boolean(dontAdvanceSetting));

		json_object_set_new(rootJ, "turingMode", json_boolean(turingMode));
		json_object_set_new(rootJ, "bitResolution", json_integer(bitResolution));
		json_object_set_new(rootJ, "progression", json_integer(progression));


		for (int t = 0; t < 8; t++) {
			json_t *progSeq_json_array = json_array();
			for (int st = 0; st < 16; st++) {
				json_array_append_new(progSeq_json_array, json_integer(wSeq[t][st]));
			}
			json_object_set_new(rootJ, ("wSeq"+to_string(t)).c_str(), progSeq_json_array);
		}

		for (int t = 0; t < 8; t++) {
			json_t *progSteps_json_array = json_array();
			json_array_append_new(progSteps_json_array, json_integer(wSteps));	// JUST THE MAIN LENGTH SETTING
			json_object_set_new(rootJ, ("wSteps"+to_string(t)).c_str(), progSteps_json_array);
		}

		for (int t = 0; t < 8; t++) {
			json_t *progRst_json_array = json_array();
			json_array_append_new(progRst_json_array, json_real(wRst));	// JUST THE MAIN RESET SETTING
			json_object_set_new(rootJ, ("wRst"+to_string(t)).c_str(), progRst_json_array);
		}

		for (int t = 0; t < 8; t++) {
			json_t *wOfs_json_array = json_array();
			json_array_append_new(wOfs_json_array, json_real(0));
			json_object_set_new(rootJ, ("wOfs"+to_string(t)).c_str(), wOfs_json_array);
		}

		return rootJ;
	}

	void singleFromJson(json_t *rootJ) {

		json_t* runTypeJ = json_object_get(rootJ, "runType");
		if (runTypeJ) {
			runType = json_integer_value(runTypeJ);
			if (runType < 0 || runType > 1)
				runType = 0;
		}

		json_t* revTypeJ = json_object_get(rootJ, "revType");
		if (revTypeJ) {
			revType = json_integer_value(revTypeJ);
			if (revType < 0 || revType > 1)
				revType = 0;
		}

		json_t* outTypeJ = json_object_get(rootJ, "outType");
		if (outTypeJ) {
			outType = json_integer_value(outTypeJ);
			if (outType < 0 || outType > 2)
				outType = 0;
		}

		json_t* rstOnRunJ = json_object_get(rootJ, "rstOnRun");
		if (rstOnRunJ) {
			rstOnRun = json_boolean_value(rstOnRunJ);
		}

		json_t* dontAdvanceSettingJ = json_object_get(rootJ, "dontAdvanceSetting");
		if (dontAdvanceSettingJ) {
			dontAdvanceSetting = json_boolean_value(dontAdvanceSettingJ);
		}

		json_t* turingModeJ = json_object_get(rootJ, "turingMode");
		if (turingModeJ) {
			turingMode = json_boolean_value(turingModeJ);
		}

		json_t* bitResolutionJ = json_object_get(rootJ, "bitResolution");
		if (bitResolutionJ) {
			bitResolution = json_integer_value(bitResolutionJ);
			if (bitResolution < 0 && bitResolution > 1)
				bitResolution = BIT_8;
		}

		json_t* progressionJ = json_object_get(rootJ, "progression");
		if (progressionJ) {
			progression = json_integer_value(progressionJ);
			if (progression < 0 && progression > 2)
				progression = STD2x_PROGRESSION;
		}

		for (int t = 0; t < 8; t++) {
			json_t *wSeq_json_array = json_object_get(rootJ, ("wSeq"+to_string(t)).c_str());
			size_t st;
			json_t *wSeq_json_value;
			if (wSeq_json_array) {
				json_array_foreach(wSeq_json_array, st, wSeq_json_value) {
					params[STEP_PARAM+(t*16)+st].setValue(json_integer_value(wSeq_json_value));
				}
			}
		}

		int t = 0;	// LENGTH AND RESET ARE TAKEN FROM THE FIRST SAVED TRACK

		json_t *wSteps_json_array = json_object_get(rootJ, ("wSteps"+to_string(t)).c_str());
		size_t jSteps;
		json_t *steps_json_value;
		if (wSteps_json_array) {
			json_array_foreach(wSteps_json_array, jSteps, steps_json_value) {
				params[LENGTH_PARAM].setValue(json_integer_value(steps_json_value));
			}
		}

		json_t *wRst_json_array = json_object_get(rootJ, ("wRst"+to_string(t)).c_str());
		size_t jRst;
		json_t *wRst_json_value;
		if (wRst_json_array) {
			json_array_foreach(wRst_json_array, jRst, wRst_json_value) {
				params[RST_PARAM].setValue(json_real_value(wRst_json_value));
			}
		}

	}

	void menuLoadSingle() {
		static const char FILE_FILTERS[] = "randLoops8 preset (.r8p):r8sp,R8P";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
#endif
		if (path)
			loadSingle(path);

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void loadSingle(std::string path) {

		FILE *file = fopen(path.c_str(), "r");
		json_error_t error;
		json_t *rootJ = json_loadf(file, 0, &error);
		if (rootJ == NULL) {
			WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		}

		fclose(file);

		if (rootJ) {

			singleFromJson(rootJ);

		} else {
			WARN("problem loading preset json file");
			//return;
		}
		
	}

	void menuSaveSingle() {

		static const char FILE_FILTERS[] = "randLoops8 preset (.r8p):r8sp,R8P";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters);
#endif
		if (path) {
			std::string strPath = path;
			if (strPath.substr(strPath.size() - 4) != ".r8p" and strPath.substr(strPath.size() - 4) != ".R8P")
				strPath += ".r8p";
			path = strcpy(new char[strPath.length() + 1], strPath.c_str());
			saveSingle(path, singleToJson());
		}

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void saveSingle(std::string path, json_t *rootJ) {

		if (rootJ) {
			FILE *file = fopen(path.c_str(), "w");
			if (!file) {
				WARN("[ SickoCV ] cannot open '%s' to write\n", path.c_str());
				//return;
			} else {
				json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
				json_decref(rootJ);
				fclose(file);
			}
		}
	}

	// ------------------------ LOAD / SAVE   SINGLE SEQUENCE

	json_t *sequenceToJson(int t) {

		json_t *rootJ = json_object();

		json_t *wSeq_json_array = json_array();
		for (int st = 0; st < 16; st++) {
			json_array_append_new(wSeq_json_array, json_integer(wSeq[t][st]));
		}
		json_object_set_new(rootJ, "sr", wSeq_json_array);	
		json_object_set_new(rootJ, "length", json_integer((int)params[LENGTH_PARAM].getValue()));
		json_object_set_new(rootJ, "reset", json_real(params[RST_PARAM].getValue()));
		json_object_set_new(rootJ, "offset", json_real(0));

		return rootJ;
	}

	void sequenceFromJson(int t, json_t *rootJ) {

		json_t *wSeq_json_array = json_object_get(rootJ, "sr");
		size_t st;
		json_t *wSeq_json_value;
		if (wSeq_json_array) {
			json_array_foreach(wSeq_json_array, st, wSeq_json_value) {
				params[STEP_PARAM+(t*16)+st].setValue(json_integer_value(wSeq_json_value));
			}
		}

		json_t* lengthJ = json_object_get(rootJ, "length");
		if (lengthJ) {
			if (json_integer_value(lengthJ) < 1 || json_integer_value(lengthJ) > 16)
				params[LENGTH_PARAM].setValue(16);
			else
				params[LENGTH_PARAM].setValue(int(json_integer_value(lengthJ)));
		}

		json_t* rstJ = json_object_get(rootJ, "reset");
		if (rstJ) {
			if (json_real_value(rstJ) < 0 || json_real_value(rstJ) > 1)
				params[RST_PARAM].setValue(0);
			else
				params[RST_PARAM].setValue(json_real_value(rstJ));
		}

	}

	void menuLoadSequence(int track) {
		static const char FILE_FILTERS[] = "trigSeq sequence (.tss):tss,TSS";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
#endif
		if (path)
			loadSequence(track, path);

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void loadSequence(int track, std::string path) {

		FILE *file = fopen(path.c_str(), "r");
		json_error_t error;
		json_t *rootJ = json_loadf(file, 0, &error);
		if (rootJ == NULL) {
			WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		}

		fclose(file);

		if (rootJ) {

			sequenceFromJson(track, rootJ);

		} else {
			WARN("problem loading preset json file");
			//return;
		}
		
	}

	void menuSaveSequence(int track) {

		static const char FILE_FILTERS[] = "trigSeq sequence (.tss):tss,TSS";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters);
#endif
		if (path) {
			std::string strPath = path;
			if (strPath.substr(strPath.size() - 4) != ".tss" and strPath.substr(strPath.size() - 4) != ".TSS")
				strPath += ".tss";
			path = strcpy(new char[strPath.length() + 1], strPath.c_str());
			saveSequence(path, sequenceToJson(track));
		}

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void saveSequence(std::string path, json_t *rootJ) {

		if (rootJ) {
			FILE *file = fopen(path.c_str(), "w");
			if (!file) {
				WARN("[ SickoCV ] cannot open '%s' to write\n", path.c_str());
				//return;
			} else {
				json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
				json_decref(rootJ);
				fclose(file);
			}
		}
	}

	void copyAllTracks() {
		for (int t = 0; t < 8; t++) {
			for (int st = 0; st < 16; st++)
				randLoops8_cbSeq[t][st] = wSeq[t][st];

			randLoops8_cbSteps[t] = wSteps;
			randLoops8_cbCtrl[t] = 1;	// this locks ctrl paramer if pasting to randLoops8
			randLoops8_cbScale[t] = wRst;
			randLoops8_cbOffset[t] = 0;
			
		}
		randLoops8_clipboard = true;
	}

	void pasteAllTracks() {
		for (int t = 0; t < 8; t++)
			for (int st = 0; st < 16; st++)
				params[STEP_PARAM+(t*16)+st].setValue(randLoops8_cbSeq[t][st]);

		params[LENGTH_PARAM].setValue(randLoops8_cbSteps[0]);
		params[RST_PARAM].setValue(randLoops8_cbScale[0]);
	
	}

	void copyTrack(int t) {
		for (int st = 0; st < 16; st++)
			randLoops_cbSeq[st] = wSeq[t][st];
		
		randLoops_cbSteps = wSteps;
		randLoops_cbScale = wRst;
		randLoops_cbCtrl = 1;	// this locks ctrl paramer if pasting to randLoops/randLoops8
		randLoops_clipboard = true;
	}

	void pasteToTrack(int t) {
		for (int st = 0; st < 16; st++) {
			wSeq[t][st] = randLoops_cbSeq[st];
			params[STEP_PARAM+(t*16)+st].setValue(wSeq[t][st]);
		}
		
		wSteps = randLoops_cbSteps;
		params[LENGTH_PARAM].setValue(wSteps);
		wRst = randLoops_cbScale;
		params[RST_PARAM].setValue(wRst);
	}

	void eraseProgs() {
		for (int p = 0; p < 32; p++) {
			progSteps[p] = 16;
			progRst[p] = 0;
			for (int t = 0; t < 8; t++) {
				for (int s = 0; s < 16; s++)
					progSeq[p][t][s] = 0;
			}
		}
		lastProg = 0;
	}
	
	void inline resetStep() {
		lights[STEP_LIGHT+step].setBrightness(0);

		if (!turingMode) {
			step = int(wRst * 15);
		} else {
			step = 0;
			for (int t = 0; t < 8; t++)
				calcVoltage(t);
		}

		if (mode == CLOCK_MODE && dontAdvanceSetting)
			dontAdvance = true;

		if (progInType != CV_TYPE)
			progKnob = 0;
	}

	void inline calcVoltage(int t) {

		int startCursor = maxSteps - step;
		int lengthCursor = 0;

		if (startCursor >= maxSteps)
			startCursor = 0;

		int tempCursor = startCursor;

		volt[t] = 0.f;

		for (int i=0; i < 16; i++) {
			if (lengthCursor >= maxSteps) {
				lengthCursor = 0;
				tempCursor = startCursor;
			}
			
			if (tempCursor >= maxSteps)
				tempCursor = 0;

			if (wSeq[t][tempCursor])
				volt[t] += tableVolts[progression][bitResolution][i];

			tempCursor++;
			lengthCursor++;
		}
		
		if (volt[t] > 10.f)
			volt[t] = 10.f;

	}

	void scanLastProg() {
		lastProg = 31;
		bool exitFunc = false;

		for (int p = 31; p >= 0; p--) {
			for (int t = 0; t < 8; t++) {
				for (int st = 0; st < 16; st++) {
					if (progSeq[p][t][st] != 0) {
						st = 16;
						t = 8;
						exitFunc = true;
					}
				}
			}

			if (progSteps[p] != 16 || progRst[p] != 0)
				exitFunc = true;

			lastProg = p;

			if (exitFunc)
				p = 0;
		}
	}

	void randomizeTrack(int t) {
		float tempRand;
		for (int st = 0; st < 16; st++) {
			tempRand = random::uniform();
			if (tempRand > .5f)
				params[STEP_PARAM+(t*16)+st].setValue(1.f);
			else
				params[STEP_PARAM+(t*16)+st].setValue(0.f);
		}
	}

	void randomizeAll() {
		for (int t = 0; t < 8; t++)
			randomizeTrack(t);
	}

	void process(const ProcessArgs& args) override {

		// ----------- AUTO SWITCH

		instantProgChange = int(params[AUTO_PARAM].getValue());
		lights[AUTO_LIGHT].setBrightness(instantProgChange);

		// ----------- PROGRAM MANAGEMENT

		if (progInType == CV_TYPE) {

			progKnob = int(params[PROG_PARAM].getValue() + (inputs[PROG_INPUT].getVoltage() * 3.2));
			if (progKnob < 0)
				progKnob = 0;
			else if (progKnob > 31)
				progKnob = 31;

		} else {

			progKnob = params[PROG_PARAM].getValue();
			if (progKnob < 0)
				progKnob = 0;
			else if (progKnob > 31)
				progKnob = 31;

			progTrig = inputs[PROG_INPUT].getVoltage();
			if (progTrig >= 1.f && prevProgTrig < 1.f) {
				progKnob++;
				if (progKnob > lastProg)
					progKnob = 0;

				params[PROG_PARAM].setValue(progKnob);
			}
			prevProgTrig = progTrig;

		}

		if (progKnob != prevProgKnob) {

			progChanged = true;
			selectedProg = progKnob;
			prevProgKnob = progKnob;

			for (int t = 0; t < 8; t++) {
				for (int i = 0; i < 16; i++) {
					nextSeq[t][i] = progSeq[selectedProg][t][i];
					params[STEP_PARAM+(t*16)+i].setValue(nextSeq[t][i]);
					lights[STEPBUT_LIGHT+(t*16)+i].setBrightness(nextSeq[t][i]);
				}
			}
			nextSteps = progSteps[selectedProg];
			nextRst = progRst[selectedProg];

			params[LENGTH_PARAM].setValue(nextSteps);
			params[RST_PARAM].setValue(nextRst);

			setButLight = true;
			setButLightValue = 0.f;

		}

		// -------- CURRENT SEQ UPDATE

		progToSet = false;

		progSetBut = params[SET_PARAM].getValue();
		if (progSetBut >= 1.f && prevProgSetBut < 1.f)
			progToSet = true;

		prevProgSetBut = progSetBut;

		if (!progChanged) {
			for (int t = 0; t < 8; t++) {
				for (int i = 0; i < 16; i++) {
					wSeq[t][i] = params[STEP_PARAM+(t*16)+i].getValue();
					lights[STEPBUT_LIGHT+(t*16)+i].setBrightness(wSeq[t][i]);
				}
			}
			wSteps = params[LENGTH_PARAM].getValue();
			wRst = params[RST_PARAM].getValue();

		} else {
			if (instantProgChange || progToSet) {

				for (int t = 0; t < 8; t++) {
					for (int i = 0; i < 16; i++)
						wSeq[t][i] = nextSeq[t][i];
				}

				wSteps = nextSteps;
				wRst = nextRst;
				params[LENGTH_PARAM].setValue(wSteps);
				params[RST_PARAM].setValue(wRst);

				workingProg = selectedProg;
				if (progInType == CV_TYPE)
					savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
				else
					savedProgKnob = progKnob;
				
				progChanged = false;
				progToSet = false;
				setButLight = false;
				setButLightValue = 0.f;

			} else { 	// IF SET IS PENDING -> GET NEW SETTINGS
				
				for (int t = 0; t < 8; t++) {
					for (int i = 0; i < 16; i++) {
						nextSeq[t][i] = params[STEP_PARAM+(t*16)+i].getValue();
						lights[STEPBUT_LIGHT+(t*16)+i].setBrightness(nextSeq[t][i]);
					}
				}
				nextSteps = params[LENGTH_PARAM].getValue();
				nextRst = params[RST_PARAM].getValue();

			}
		
		}

		// -------------------------- RECALL PROG

		recallBut = params[RECALL_PARAM].getValue();
		lights[RECALL_LIGHT].setBrightness(recallBut);

		if (recallBut >= 1.f && prevRecallBut < 1.f) {

			if (!progChanged) {

				for (int t = 0; t < 8; t++) {
					for (int i = 0; i < 16; i++) {
						wSeq[t][i] = progSeq[selectedProg][t][i];
						params[STEP_PARAM+(t*16)+i].setValue(wSeq[t][i]);
						lights[STEPBUT_LIGHT+(t*16)+i].setBrightness(wSeq[t][i]);
					}
				}
				wSteps = progSteps[selectedProg];
				params[LENGTH_PARAM].setValue(wSteps);
				wRst = progRst[selectedProg];
				params[RST_PARAM].setValue(wRst);

				workingProg = selectedProg;
				if (progInType == CV_TYPE)
					savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
				else
					savedProgKnob = progKnob;

			} else {

				for (int t = 0; t < 8; t++) {
					for (int i = 0; i < 16; i++) {
						params[STEP_PARAM+(t*16)+i].setValue(wSeq[t][i]);
						lights[STEPBUT_LIGHT+(t*16)+i].setBrightness(wSeq[t][i]);
					}
				}
				params[LENGTH_PARAM].setValue(wSteps);
				params[RST_PARAM].setValue(wRst);

				params[PROG_PARAM].setValue(workingProg);
				//savedProgKnob = progKnob - (inputs[PROG_INPUT].getVoltage() * 3.2);
				savedProgKnob = workingProg;
				selectedProg = workingProg;
				prevProgKnob = workingProg;

			}
			progChanged = false;
			setButLight = false;
			setButLightValue = 0.f;
		}
		prevRecallBut = recallBut;

		// -----------------------------------
		// ------------ STORE MANAGEMENT
		// -----------------------------------

		storeBut = params[STORE_PARAM].getValue();
		lights[STORE_LIGHT].setBrightness(storeBut);

		if (storeBut >= 1 && prevStoreBut < 1) {
			if (!storeWait) {
				storeWait = true;
				storeTime = storeSamples;
			} else {
				storeWait = false;
				for (int t = 0; t < 8; t++) {
					for (int i = 0; i < 16; i++)
						progSeq[progKnob][t][i] = wSeq[t][i];
				}

				progSteps[progKnob] = wSteps;
				progRst[progKnob] = wRst;

				if (progKnob > lastProg)
					lastProg = progKnob;

				storedProgram = true;
				storedProgramTime = maxStoredProgramTime;
			}
		}
		
		if (storeWait) {
			storeTime--;
			if (storeTime < 0)
				storeWait = false;
		}
		prevStoreBut = storeBut;

		if (storedProgram) {
			storedProgramTime--;
			if (storedProgramTime < 0) {
				lights[STORE_LIGHT].setBrightness(0);
				storedProgram = false;
			} else {
				lights[STORE_LIGHT].setBrightness(1);
			}

		}

		if (setButLight) {
			if (setButLightValue > 1 || setButLightValue < 0) {
				setButLightDelta *= -1;
			}
			setButLightValue += setButLightDelta;
		}

		lights[SET_LIGHT].setBrightness(setButLightValue);

		// -------------------------------------------------
		// -------------------------------------------------
		// -------------------------------------------------
		// -------------------------------------------------
		// -------------------------------------------------

		if (turingMode && !prevTuringMode)
			for (int t = 0; t < 8; t++)
				calcVoltage(t);

		prevTuringMode = turingMode;

		for (int t = 0; t < 8; t++)
			out[t] = 0.f;

		mode = params[MODE_SWITCH].getValue();

		// ---------- reset check

		rstValue = inputs[RST_INPUT].getVoltage();
		if (mode == CLOCK_MODE && rstValue >= 1.f && prevRstValue < 1.f)
			resetStep();

		prevRstValue = rstValue;

		// -----------------------

		if (inputs[RUN_INPUT].isConnected()) {

			runTrig = inputs[RUN_INPUT].getVoltage();

			if (runType == RUN_GATE) {
				
				if (runTrig > 1) {
					runSetting = 1;
					if (!prevRunSetting && mode == CLOCK_MODE && rstOnRun)
						resetStep();
				} else {
					runSetting = 0;
				}

			} else {	// runType == RUN_TRIG

				if (runTrig > 1 && prevRunTrig <=1) {
					if (runSetting) {
						runSetting = 0;
						params[RUNBUT_PARAM].setValue(0);
					} else {
						runSetting = 1;
						params[RUNBUT_PARAM].setValue(1);
						if (!prevRunSetting && mode == CLOCK_MODE && rstOnRun)
							resetStep();
					}
				}				
			}
			prevRunSetting = runSetting;
			prevRunTrig = runTrig;
		
		} else {
			
			runSetting = params[RUNBUT_PARAM].getValue();
			if (mode == CLOCK_MODE && rstOnRun && runSetting && !prevRunSetting)
				resetStep();
		}

		prevRunSetting = runSetting;

		lights[RUNBUT_LIGHT].setBrightness(runSetting);


		if (runSetting) {

			maxSteps = wSteps;

			if (inputs[LENGTH_INPUT].isConnected()) {
				float stepsIn = inputs[LENGTH_INPUT].getVoltage();
				if (stepsIn < 0.f)
					stepsIn = 0.f;
				else if (stepsIn > 10.f)
					stepsIn = 10.f;

				int addSteps = int(stepsIn / 10 * (16 - maxSteps));

				maxSteps += addSteps;
				if (maxSteps > 16)
					maxSteps = 16;
			}
			
			if (mode == CV_MODE && prevMode == CLOCK_MODE) {
				prevClkValue = 11.f;
				prevAddr = 11.f;
			}
			prevMode = mode;

			clkValue = inputs[CLK_INPUT].getVoltage();

			switch (mode) {
				case CLOCK_MODE:

					if (clkValue >= 1.f && prevClkValue < 1.f) {

						if (revType == POSITIVE_V) {
							if (inputs[REV_INPUT].getVoltage() < 1)
								direction = FORWARD;
							else
								direction = REVERSE;
						} else {
							if (inputs[REV_INPUT].getVoltage() < -1)
								direction = REVERSE;
							else
								direction = FORWARD;
						}

						lights[STEP_LIGHT + step].setBrightness(0);

						if (direction == FORWARD) {

							if (!dontAdvance)
								step++;
							else
								dontAdvance = false;

							if (step >= maxSteps)
								step = 0;
						} else {

							if (!dontAdvance)
								step--;
							else
								dontAdvance = false;

							if (step < 0)
								step = maxSteps - 1;
						}

						if (!turingMode) {
							for (int t = 0; t < 8; t++) {
								if (wSeq[t][step]) {
									stepPulse[t] = true;
									stepPulseTime[t] = oneMsTime;
									if (outType == OUT_GATE)
										outGate[t] = true;
								} else {
									if (outType == OUT_GATE) {
										outGate[t] = false;
										out[t] = 0.f;
									}
								}
							}
						} else {
							for (int t = 0; t < 8; t++)
								calcVoltage(t);
						}
					}
					prevClkValue = clkValue;
				break;

				case CV_MODE:
					if (clkValue > 10.f)
						clkValue = 10.f;
					else if (clkValue < 0.f)
						clkValue = 0.f;

					
					if (clkValue != prevClkValue) {
						
						currAddr = 1+int(clkValue / 10 * (maxSteps));
						if (currAddr >= maxSteps)
							currAddr = maxSteps;
						if (currAddr != prevAddr) {
							lights[STEP_LIGHT+step].setBrightness(0);
							step = currAddr-1;

							if (!turingMode) {
								for (int t = 0; t < 8; t++) {
									if (wSeq[t][step]) {
										stepPulse[t] = true;
										stepPulseTime[t] = oneMsTime;
										if (outType == OUT_GATE)
											outGate[t] = true;
									} else {
										if (outType == OUT_GATE) {
											outGate[t] = false;
											out[t] = 0.f;
										}
									}
								}
							} else {
								for (int t = 0; t < 8; t++) 
									calcVoltage(t);
							}
							prevAddr = currAddr;

						}
					}
					prevClkValue = clkValue;
					
				break;
			}
		}
			
		if (!turingMode) {
			for (int t = 0; t < 8; t++) {
				if (stepPulse[t]) {

					if ( (mode == CLOCK_MODE && outType == OUT_TRIG) || (mode == CV_MODE && (outType == OUT_TRIG || outType == OUT_CLOCK) ) ) {
						stepPulseTime[t]--;
						if (stepPulseTime[t] < 0) {
							stepPulse[t] = false;
							out[t] = 0.f;
						} else {
							out[t] = 10.f;
						}
					} else if (mode == CLOCK_MODE && outType == OUT_CLOCK) {
						out[t] = inputs[CLK_INPUT].getVoltage();
						if (out[t] < 1.f) {
							out[t] = 0.f;
							stepPulse[t] = false;
						}
					} else if (outType == OUT_GATE) {
						if (outGate[t])
							out[t] = 10.f;
						else
							out[t] = 0.f;
					}
				}
			}
		} else {
			for (int t = 0; t < 8; t++)
				out[t] = volt[t] * wRst;
		}

		for (int t = 0; t < 8; t++)
			outputs[OUT_OUTPUT+t].setVoltage(out[t]);

		lights[STEP_LIGHT+step].setBrightness(1);
	}
};

struct TrigSeq8xDisplay : TransparentWidget {
	TrigSeq8x *module;
	int frame = 0;
	TrigSeq8xDisplay() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/vgatrue.ttf"));
				
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				std::string currentDisplay;

				currentDisplay = to_string(module->workingProg);

				//if (!module->pendingUpdate) {
				if (!module->progChanged) {	// NEW
					nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_GREEN));
					nvgFontSize(args.vg, 32);
					if (currentDisplay.size() == 2)
						nvgTextBox(args.vg, 8, 30, 80, currentDisplay.c_str(), NULL);
					else
						nvgTextBox(args.vg, 16, 30, 80, currentDisplay.c_str(), NULL);

				} else {
					nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_GREEN));
					nvgFontSize(args.vg, 26);
					if (currentDisplay.size() == 2)
						nvgTextBox(args.vg, 6, 21, 80, currentDisplay.c_str(), NULL);
					else
						nvgTextBox(args.vg, 12, 21, 80, currentDisplay.c_str(), NULL);
					
					currentDisplay = to_string(module->selectedProg);

					nvgFillColor(args.vg, nvgRGBA(COLOR_LCD_RED));
					nvgFontSize(args.vg, 20);
					if (currentDisplay.size() == 2)
						nvgTextBox(args.vg, 20, 36, 60, currentDisplay.c_str(), NULL);
					else
						nvgTextBox(args.vg, 25, 36, 60, currentDisplay.c_str(), NULL);
				}
				
			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct TrigSeq8xWidget : ModuleWidget {
	TrigSeq8xWidget(TrigSeq8x* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/TrigSeq8x.svg")));

		{
			TrigSeq8xDisplay *display = new TrigSeq8xDisplay();

			display->box.pos = mm2px(Vec(42.4, 9.1));
			display->box.size = mm2px(Vec(8, 8));
			display->module = module;
			addChild(display);
		}

		addChild(createWidget<SickoScrewBlack1>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	  

		const float xLeft = 7.f;

		const float yCvSw = 19.f;
		const float yTrig = 34.5f;

		const float yRunBut = 48.5 + 5.5;
		const float yRunIn = 57 + 7.5;

		const float yRev = 84.5f;

		const float yRstKn = 90.9f + 14.5;
		const float yRst = 100.f + 15.5;

		const float xStepsIn = 22.f;
		const float yStepsIn = 18.2f;

		const float xLength = 34.1f;
		const float yLength = 17.6f;

		const float yProgLine = 18.2f;
		
		const float xProgIn = 65.5f; 
		const float xProgKnob = 80.1f; 

		const float yProgKnob = 16.3f;

		const float xSet = 97.5f;
		const float xAuto = 109.7f;

		//const float xRecallBut = 129.f;
		//const float xStore = 144.4f;

		const float xRecallBut = 127.5f;
		const float xStore = 142.9f;

		const float yStart = 37.5f;
		const float yDelta = 11.2f;

		const float xStart = 19.2f;
		const float xDelta = 7.45f;
		const float xBlockShift = 1.5f;

		const float yLight = 29.5f;

		const float xOut = 145.8f;

		addParam(createParamCentered<CKSS>(mm2px(Vec(xLeft, yCvSw)), module, TrigSeq8x::MODE_SWITCH));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yTrig)), module, TrigSeq8x::CLK_INPUT));

		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xLeft, yRstKn)), module, TrigSeq8x::RST_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yRst)), module, TrigSeq8x::RST_INPUT));

		addParam(createLightParamCentered<VCVLightBezelLatch<BlueLight>>(mm2px(Vec(xLeft, yRunBut)), module, TrigSeq8x::RUNBUT_PARAM, TrigSeq8x::RUNBUT_LIGHT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yRunIn)), module, TrigSeq8x::RUN_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xLeft, yRev)), module, TrigSeq8x::REV_INPUT));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xStepsIn, yStepsIn)), module, TrigSeq8x::LENGTH_INPUT));
		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(xLength, yLength)), module, TrigSeq8x::LENGTH_PARAM));

		addInput(createInputCentered<SickoInPort>(mm2px(Vec(xProgIn, yProgLine)), module, TrigSeq8x::PROG_INPUT));
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(xProgKnob, yProgKnob)), module, TrigSeq8x::PROG_PARAM));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xSet, yProgLine)), module, TrigSeq8x::SET_PARAM, TrigSeq8x::SET_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xAuto, yProgLine)), module, TrigSeq8x::AUTO_PARAM, TrigSeq8x::AUTO_LIGHT));
		addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(mm2px(Vec(xRecallBut, yProgLine)), module, TrigSeq8x::RECALL_PARAM, TrigSeq8x::RECALL_LIGHT));
		//addInput(createInputCentered<SickoInPort>(mm2px(Vec(xRecallIn, yProgLine)), module, TrigSeq8x::RECALL_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<RedLight>>(mm2px(Vec(xStore, yProgLine)), module, TrigSeq8x::STORE_PARAM, TrigSeq8x::STORE_LIGHT));
		
		int shiftCont = 0;
		int shiftGroup = 0;
		float xShift = 0;

		for (int st = 0; st < 16; st++) {
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(xStart + (xDelta*st) + xShift, yLight)), module, TrigSeq8x::STEP_LIGHT+st));
			shiftCont++;
			if (shiftCont > 3) {
				shiftGroup++;
				xShift = xBlockShift * shiftGroup;
				shiftCont = 0;
			}
		}

		for (int t = 0; t < 8; t++) {
			shiftCont = 0;
			shiftGroup = 0;
			xShift = 0;
			for (int st = 0; st < 16; st++) {

				switch (t) {
					case 0:
						addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xStart + (xDelta*st) + xShift, yStart+(t*yDelta))), module, TrigSeq8x::STEP_PARAM+(t*16)+st, TrigSeq8x::STEPBUT_LIGHT+(t*16)+st));
					break;

					case 1:
						addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(xStart + (xDelta*st) + xShift, yStart+(t*yDelta))), module, TrigSeq8x::STEP_PARAM+(t*16)+st, TrigSeq8x::STEPBUT_LIGHT+(t*16)+st));
					break;

					case 2:
						addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xStart + (xDelta*st) + xShift, yStart+(t*yDelta))), module, TrigSeq8x::STEP_PARAM+(t*16)+st, TrigSeq8x::STEPBUT_LIGHT+(t*16)+st));
					break;

					case 3:
						addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xStart + (xDelta*st) + xShift, yStart+(t*yDelta))), module, TrigSeq8x::STEP_PARAM+(t*16)+st, TrigSeq8x::STEPBUT_LIGHT+(t*16)+st));
					break;

					case 4:
						addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(xStart + (xDelta*st) + xShift, yStart+(t*yDelta))), module, TrigSeq8x::STEP_PARAM+(t*16)+st, TrigSeq8x::STEPBUT_LIGHT+(t*16)+st));
					break;

					case 5:
						addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(xStart + (xDelta*st) + xShift, yStart+(t*yDelta))), module, TrigSeq8x::STEP_PARAM+(t*16)+st, TrigSeq8x::STEPBUT_LIGHT+(t*16)+st));
					break;

					case 6:
						addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<YellowLight>>>(mm2px(Vec(xStart + (xDelta*st) + xShift, yStart+(t*yDelta))), module, TrigSeq8x::STEP_PARAM+(t*16)+st, TrigSeq8x::STEPBUT_LIGHT+(t*16)+st));
					break;

					case 7:
						addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xStart + (xDelta*st) + xShift, yStart+(t*yDelta))), module, TrigSeq8x::STEP_PARAM+(t*16)+st, TrigSeq8x::STEPBUT_LIGHT+(t*16)+st));
					break;
				}

				shiftCont++;
				if (shiftCont > 3) {
					shiftGroup++;
					xShift = xBlockShift * shiftGroup;
					shiftCont = 0;
				}
				
				
			}
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xOut, yStart+(t*yDelta))), module, TrigSeq8x::OUT_OUTPUT+t));
		}
		
	}

	void appendContextMenu(Menu* menu) override {
		TrigSeq8x* module = dynamic_cast<TrigSeq8x*>(this->module);

		menu->addChild(new MenuSeparator());

		menu->addChild(createSubmenuItem("Randomize Steps", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("All tracks", "", [=]() {module->randomizeAll();}));
				menu->addChild(createMenuItem("Track 1", "", [=]() {module->randomizeTrack(0);}));
				menu->addChild(createMenuItem("Track 2", "", [=]() {module->randomizeTrack(1);}));
				menu->addChild(createMenuItem("Track 3", "", [=]() {module->randomizeTrack(2);}));
				menu->addChild(createMenuItem("Track 4", "", [=]() {module->randomizeTrack(3);}));
				menu->addChild(createMenuItem("Track 5", "", [=]() {module->randomizeTrack(4);}));
				menu->addChild(createMenuItem("Track 6", "", [=]() {module->randomizeTrack(5);}));
				menu->addChild(createMenuItem("Track 7", "", [=]() {module->randomizeTrack(6);}));
				menu->addChild(createMenuItem("Track 8", "", [=]() {module->randomizeTrack(7);}));
		}));

		menu->addChild(new MenuSeparator());

		struct OutTypeItem : MenuItem {
			TrigSeq8x* module;
			int outType;
			void onAction(const event::Action& e) override {
				module->outType = outType;
			}
		};
		std::string OutTypeNames[3] = {"Trig", "Gate", "Clock Width"};
		menu->addChild(createSubmenuItem("Output type", (OutTypeNames[module->outType]), [=](Menu * menu) {
			for (int i = 0; i < 3; i++) {
				OutTypeItem* outTypeItem = createMenuItem<OutTypeItem>(OutTypeNames[i]);
				outTypeItem->rightText = CHECKMARK(module->outType == i);
				outTypeItem->module = module;
				outTypeItem->outType = i;
				menu->addChild(outTypeItem);
			}
		}));

		struct RevTypeItem : MenuItem {
			TrigSeq8x* module;
			int revType;
			void onAction(const event::Action& e) override {
				module->revType = revType;
			}
		};
		std::string RevTypeNames[2] = {"Positive", "Negative"};
		menu->addChild(createSubmenuItem("Reverse Input Voltage", (RevTypeNames[module->revType]), [=](Menu * menu) {
			for (int i = 0; i < 2; i++) {
				RevTypeItem* revTypeItem = createMenuItem<RevTypeItem>(RevTypeNames[i]);
				revTypeItem->rightText = CHECKMARK(module->revType == i);
				revTypeItem->module = module;
				revTypeItem->revType = i;
				menu->addChild(revTypeItem);
			}
		}));

		struct RunTypeItem : MenuItem {
			TrigSeq8x* module;
			int runType;
			void onAction(const event::Action& e) override {
				module->runType = runType;
			}
		};

		std::string RunTypeNames[2] = {"Gate", "Trig"};
		menu->addChild(createSubmenuItem("Run Input", (RunTypeNames[module->runType]), [=](Menu * menu) {
			for (int i = 0; i < 2; i++) {
				RunTypeItem* runTypeItem = createMenuItem<RunTypeItem>(RunTypeNames[i]);
				runTypeItem->rightText = CHECKMARK(module->runType == i);
				runTypeItem->module = module;
				runTypeItem->runType = i;
				menu->addChild(runTypeItem);
			}
		}));

		menu->addChild(createBoolPtrMenuItem("Reset on Run", "", &module->rstOnRun));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("1st clock after reset:"));
		menu->addChild(createBoolPtrMenuItem("Don't advance", "", &module->dontAdvanceSetting));

		menu->addChild(new MenuSeparator());
		struct ProgInTypeItem : MenuItem {
			TrigSeq8x* module;
			int progInType;
			void onAction(const event::Action& e) override {
				module->progInType = progInType;
			}
		};

		std::string ProgInTypeNames[2] = {"CV", "Trig"};
		menu->addChild(createSubmenuItem("Prog Input type", (ProgInTypeNames[module->progInType]), [=](Menu * menu) {
			for (int i = 0; i < 2; i++) {
				ProgInTypeItem* progInTypeItem = createMenuItem<ProgInTypeItem>(ProgInTypeNames[i]);
				progInTypeItem->rightText = CHECKMARK(module->progInType == i);
				progInTypeItem->module = module;
				progInTypeItem->progInType = i;
				menu->addChild(progInTypeItem);
			}
		}));
		menu->addChild(createMenuItem("Scan Last Prog", "current: " + to_string(module->lastProg), [=]() {module->scanLastProg();}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Copy All Tracks", "", [=]() {module->copyAllTracks();}));
		if (randLoops8_clipboard) {
			menu->addChild(createMenuItem("Paste All Tracks", "", [=]() {module->pasteAllTracks();}));
		} else {
			menu->addChild(createMenuLabel("Paste All Tracks"));
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Copy single track", "", [=](Menu * menu) {
			menu->addChild(createMenuItem("Copy Track 1", "", [=]() {module->copyTrack(0);}));
			menu->addChild(createMenuItem("Copy Track 2", "", [=]() {module->copyTrack(1);}));
			menu->addChild(createMenuItem("Copy Track 3", "", [=]() {module->copyTrack(2);}));
			menu->addChild(createMenuItem("Copy Track 4", "", [=]() {module->copyTrack(3);}));
			menu->addChild(createMenuItem("Copy Track 5", "", [=]() {module->copyTrack(4);}));
			menu->addChild(createMenuItem("Copy Track 6", "", [=]() {module->copyTrack(5);}));
			menu->addChild(createMenuItem("Copy Track 7", "", [=]() {module->copyTrack(6);}));
			menu->addChild(createMenuItem("Copy Track 8", "", [=]() {module->copyTrack(7);}));
		}));
		if (randLoops_clipboard) {
			menu->addChild(createSubmenuItem("Paste single track to", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("Paste Track 1", "", [=]() {module->pasteToTrack(0);}));
				menu->addChild(createMenuItem("Paste Track 2", "", [=]() {module->pasteToTrack(1);}));
				menu->addChild(createMenuItem("Paste Track 3", "", [=]() {module->pasteToTrack(2);}));
				menu->addChild(createMenuItem("Paste Track 4", "", [=]() {module->pasteToTrack(3);}));
				menu->addChild(createMenuItem("Paste Track 5", "", [=]() {module->pasteToTrack(4);}));
				menu->addChild(createMenuItem("Paste Track 6", "", [=]() {module->pasteToTrack(5);}));
				menu->addChild(createMenuItem("Paste Track 7", "", [=]() {module->pasteToTrack(6);}));
				menu->addChild(createMenuItem("Paste Track 8", "", [=]() {module->pasteToTrack(7);}));
			}));
		} else {
			menu->addChild(createMenuLabel("Paste to track"));
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("TURING MODE", "", &module->turingMode));
		if (module->turingMode) {
			struct BitResTypeItem : MenuItem {
				TrigSeq8x* module;
				int bitResType;
				void onAction(const event::Action& e) override {
					module->bitResolution = bitResType;
				}
			};
			std::string BitResTypeNames[2] = {"8 bit", "16 bit"};
			menu->addChild(createSubmenuItem("Bit Resolution", (BitResTypeNames[module->bitResolution]), [=](Menu * menu) {
				for (int i = 0; i < 2; i++) {
					BitResTypeItem* bitResTypeItem = createMenuItem<BitResTypeItem>(BitResTypeNames[i]);
					bitResTypeItem->rightText = CHECKMARK(module->bitResolution == i);
					bitResTypeItem->module = module;
					bitResTypeItem->bitResType = i;
					menu->addChild(bitResTypeItem);
				}
			}));

			struct ProgressionTypeItem : MenuItem {
				TrigSeq8x* module;
				int progressionType;
				void onAction(const event::Action& e) override {
					module->progression = progressionType;
				}
			};
			std::string ProgressionTypeNames[3] = {"2x (std.)", "1.3x", "Fibonacci"};
			menu->addChild(createSubmenuItem("Voltage progression", (ProgressionTypeNames[module->progression]), [=](Menu * menu) {
				for (int i = 0; i < 3; i++) {
					ProgressionTypeItem* progressionTypeItem = createMenuItem<ProgressionTypeItem>(ProgressionTypeNames[i]);
					progressionTypeItem->rightText = CHECKMARK(module->progression == i);
					progressionTypeItem->module = module;
					progressionTypeItem->progressionType = i;
					menu->addChild(progressionTypeItem);
				}
			}));
		} else {
			menu->addChild(createMenuLabel("Bit Resolution"));
			menu->addChild(createMenuLabel("Voltage progression"));
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("DISK operations", "", [=](Menu * menu) {
			
			menu->addChild(createMenuItem("Load preset", "", [=]() {module->menuLoadPreset();}));
			menu->addChild(createMenuItem("Save preset", "", [=]() {module->menuSavePreset();}));

			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuItem("Import randLoops8 preset", "", [=]() {module->menuLoadSingle();}));
			menu->addChild(createMenuItem("Export randLoops8 preset", "", [=]() {module->menuSaveSingle();}));

			menu->addChild(new MenuSeparator());

			menu->addChild(createSubmenuItem("Import trigSeq seq. to:", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("Track 1", "", [=]() {module->menuLoadSequence(0);}));
				menu->addChild(createMenuItem("Track 2", "", [=]() {module->menuLoadSequence(1);}));
				menu->addChild(createMenuItem("Track 3", "", [=]() {module->menuLoadSequence(2);}));
				menu->addChild(createMenuItem("Track 4", "", [=]() {module->menuLoadSequence(3);}));
				menu->addChild(createMenuItem("Track 5", "", [=]() {module->menuLoadSequence(4);}));
				menu->addChild(createMenuItem("Track 6", "", [=]() {module->menuLoadSequence(5);}));
				menu->addChild(createMenuItem("Track 7", "", [=]() {module->menuLoadSequence(6);}));
				menu->addChild(createMenuItem("Track 8", "", [=]() {module->menuLoadSequence(7);}));
			}));
			menu->addChild(createSubmenuItem("Export trigSeq seq. from:", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("Track 1", "", [=]() {module->menuSaveSequence(0);}));
				menu->addChild(createMenuItem("Track 2", "", [=]() {module->menuSaveSequence(1);}));
				menu->addChild(createMenuItem("Track 3", "", [=]() {module->menuSaveSequence(2);}));
				menu->addChild(createMenuItem("Track 4", "", [=]() {module->menuSaveSequence(3);}));
				menu->addChild(createMenuItem("Track 5", "", [=]() {module->menuSaveSequence(4);}));
				menu->addChild(createMenuItem("Track 6", "", [=]() {module->menuSaveSequence(5);}));
				menu->addChild(createMenuItem("Track 7", "", [=]() {module->menuSaveSequence(6);}));
				menu->addChild(createMenuItem("Track 8", "", [=]() {module->menuSaveSequence(7);}));
			}));
		}));

		menu->addChild(createSubmenuItem("Erase ALL progs", "", [=](Menu * menu) {
			menu->addChild(createSubmenuItem("Are you Sure?", "", [=](Menu * menu) {
				menu->addChild(createMenuItem("ERASE!", "", [=]() {module->eraseProgs();}));
			}));
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Initialize on Start", "", &module->initStart));

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Tips", "", [=](Menu * menu) {
			menu->addChild(createMenuLabel("Store Programs with double-click"));
			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuLabel("Remember to store programs when"));
			menu->addChild(createMenuLabel("importing or pasting sequences"));
			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuLabel("When switching to TURING mode Reset Knob"));
			menu->addChild(createMenuLabel("becomes the output attenuator,"));
			menu->addChild(createMenuLabel("so it has to be adjusted"));
		}));

	}

};

Model* modelTrigSeq8x = createModel<TrigSeq8x, TrigSeq8xWidget>("TrigSeq8x");