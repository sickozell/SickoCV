#define BEAT 0
#define BAR 1

#include "plugin.hpp"
#include "osdialog.h"
//#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <vector>
#include "cmath"
#include <dirent.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

struct tpBeat : ParamQuantity {
	std::string getDisplayValueString() override {
		const std::string valueDisplay[17] = {"2/4", "3/4", "4/4", "5/4", "6/4", "7/4", "5/8", "6/8", "7/8", "8/8", "9/8", "10/8", "11/8", "12/8", "13/8", "14/8", "15/8"};
		return valueDisplay[int(getValue())];
	}
};

struct tpDivMult : ParamQuantity {
	std::string getDisplayValueString() override {
		const std::string valueDisplay[41] = {"/256", "/128", "/64", "/32", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x32", "x64", "x128", "x256"};
		return valueDisplay[int(getValue())];
	}
};
struct Clocker : Module {
	enum ParamIds {
		BPM_KNOB_PARAM,
		BEAT_KNOB_PARAM,
		CLICK_BUT_PARAM,
		CLICKVOL_KNOB_PARAM,
		PW_KNOB_PARAM,
		RUN_BUT_PARAM,
		RESET_BUT_PARAM,
		ENUMS(DIVMULT_KNOB_PARAM, 4),
		ENUMS(DIVPW_KNOB_PARAM, 4),
		NUM_PARAMS 
	};
	enum InputIds {
		EXTCLOCK_INPUT,
		RESET_INPUT,
		RUN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CLOCK_OUTPUT,
		RESET_OUTPUT,
		ENUMS(DIVMULT_OUTPUT, 4),
		BEATPULSE_OUTPUT,
		BARPULSE_OUTPUT,
		MASTER_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		CLICK_BUT_LIGHT,
		RUN_BUT_LIGHT,
		RESET_BUT_LIGHT,
		NUM_LIGHTS
	};

	//**************************************************************
	//  DEBUG 

	/*
	std::string debugDisplay = "X";
	std::string debugDisplay2 = "X";
	std::string debugDisplay3 = "X";
	std::string debugDisplay4 = "X";
	std::string debugDisplay5 = "X";
	std::string debugDisplay6 = "X";
	std::string debugDisplay7 = "X";
	int debugInt = 0;
	bool debugBool = false;
	*/

 	//**************************************************************
	//   

	const std::string beatDisplay[17] = {"2/4", "3/4", "4/4", "5/4", "6/4", "7/4", "5/8", "6/8", "7/8", "8/8", "9/8", "10/8", "11/8", "12/8", "13/8", "14/8", "15/8"};

	const int beatMaxPerBar[17] = {2, 3, 4, 5, 6, 7, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	int currentBeatMaxPerBar = 4;

	const std::string divMultDisplay[41] = {"/256", "/128", "/64", "/32", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x32", "x64", "x128", "x256"};

	const float divMult[41] = {256, 128, 64, 32, 17, 16, 15, 14, 13, 12, 11, 10, 9 , 8, 7, 6, 5, 4, 3, 2, 1,
							2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 32, 64, 128, 256};

	// coeffs to multiply with currentBps to get number of samples 
	double beatSamplesPerSec[17] = {	ceil(0.5 * APP->engine->getSampleRate()),
										ceil(0.74 * APP->engine->getSampleRate()),
										APP->engine->getSampleRate(),
										ceil(1.25 * APP->engine->getSampleRate()),
										ceil(1.5 * APP->engine->getSampleRate()),
										ceil(1.75 * APP->engine->getSampleRate()),
										ceil(0.625 * APP->engine->getSampleRate()),
										ceil(0.75 * APP->engine->getSampleRate()),
										ceil(0.875 * APP->engine->getSampleRate()),
										APP->engine->getSampleRate(),
										ceil(1.125 * APP->engine->getSampleRate()),
										ceil(1.25 * APP->engine->getSampleRate()),
										ceil(1.375 * APP->engine->getSampleRate()),
										ceil(1.5 * APP->engine->getSampleRate()),
										ceil(1.625 * APP->engine->getSampleRate()),
										ceil(1.75 * APP->engine->getSampleRate()),
										ceil(1.875 * APP->engine->getSampleRate())

		};

	//**************************************************************
	//  	

	unsigned int sampleRate[2];

	double sampleRateCoeff = (double)APP->engine->getSampleRate() * 60;

	double bpm = 0.1;

	drwav_uint64 totalSampleC[2];
	drwav_uint64 totalSamples[2];

	vector<float> playBuffer[2];
	vector<float> tempBuffer;
	vector<float> tempBuffer2;

	bool fileLoaded[2] = {false, false};
	bool play[2] = {false, false};
	double samplePos[2] = {0,0};

	std::string storedPath[2] = {"",""};
	std::string fileDescription[2] = {"--none--","--none--"};

	float extTrigValue = 0.f;
	float prevExtTrigValue = 0.f;

	float clickOutput;

	double a0, a1, a2, b1, b2, z1, z2;

	int click_setting;
	
	float resetBut = 0;
	float resetValue = 0;
	float prevResetValue = 0;

	int run_setting = 0;
	int prev_run_setting = 0;

	float runTrig = 0.f;
	float prevRunTrig = 0.f;

	double clockSample = 1.0;
	double clockMaxSample = 0.0;
	
	double midBeatMaxSample = 0.0;
	bool midBeatPlayed = false;

	double maxPulseSample = 0.0;

	int beatCounter = 1;

	float oneMsTime = (APP->engine->getSampleRate()) / 1000;
	bool resetPulse = false;
	float resetPulseTime = 0.f;
	bool beatPulse = false;
	float beatPulseTime = 0.f;
	bool barPulse = false;
	float barPulseTime = 0.f;

	bool beatOnBar = true;

	bool extSync = false;
	bool extConn = false;
	bool extBeat = false;
	bool prevExtConn = true;

	double divClockSample[4] = {1.0, 1.0, 1.0, 1.0};
	double divMaxSample[4] = {0.0, 0.0, 0.0, 0.0};
	bool divPulse[4] = {false, false, false, false};
	int divCount[4] = {1, 1, 1, 1};

	Clocker() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		configInput(EXTCLOCK_INPUT,"External Clock");

		configInput(RESET_INPUT,"Reset");
		configSwitch(RESET_BUT_PARAM, 0.f, 1.f, 0.f, "Reset", {"OFF", "ON"});
		configOutput(RESET_OUTPUT,"Reset");

		configSwitch(RUN_BUT_PARAM, 0.f, 1.f, 1.f, "Run", {"OFF", "ON"});
		configInput(RUN_INPUT,"Run");

		configParam(BPM_KNOB_PARAM, 300.f, 3000.f, 1200.f, "Tempo", " bpm", 0, 0.1);
		paramQuantities[BPM_KNOB_PARAM]->snapEnabled = true;

		configParam(PW_KNOB_PARAM, 0.f, 1.0f, 0.5f, "PW Level", "%", 0, 100);
		configSwitch(CLICK_BUT_PARAM, 0.f, 1.f, 0.f, "Click", {"OFF", "ON"});
		configParam(CLICKVOL_KNOB_PARAM, 0.f, 2.f, 1.0f, "Click Level", "%", 0, 100);
		
		configParam<tpBeat>(BEAT_KNOB_PARAM, 0.f, 16.0f, 2.f, "Beat");
		paramQuantities[BEAT_KNOB_PARAM]->snapEnabled = true;

		configParam<tpDivMult>(DIVMULT_KNOB_PARAM+0, 0.f, 40.f, 20.f, "Mult/Div #1");
		paramQuantities[DIVMULT_KNOB_PARAM+0]->snapEnabled = true;
		configParam(DIVPW_KNOB_PARAM+0, 0.f, 1.0f, 0.5f, "PW Level", "%", 0, 100);

		configParam<tpDivMult>(DIVMULT_KNOB_PARAM+1, 0.f, 40.f, 20.f, "Mult/Div #2");
		paramQuantities[DIVMULT_KNOB_PARAM+1]->snapEnabled = true;
		configParam(DIVPW_KNOB_PARAM+1, 0.f, 1.0f, 0.5f, "PW Level", "%", 0, 100);

		configParam<tpDivMult>(DIVMULT_KNOB_PARAM+2, 0.f, 40.f, 20.f, "Mult/Div #3");
		paramQuantities[DIVMULT_KNOB_PARAM+2]->snapEnabled = true;
		configParam(DIVPW_KNOB_PARAM+2, 0.f, 1.0f, 0.5f, "PW Level", "%", 0, 100);

		configParam<tpDivMult>(DIVMULT_KNOB_PARAM+3, 0.f, 40.f, 20.f, "Mult/Div #4");
		paramQuantities[DIVMULT_KNOB_PARAM+3]->snapEnabled = true;
		configParam(DIVPW_KNOB_PARAM+3, 0.f, 1.0f, 0.5f, "PW Level", "%", 0, 100);

		configOutput(DIVMULT_OUTPUT+0,"Div/Mult #1");
		configOutput(DIVMULT_OUTPUT+1,"Div/Mult #2");
		configOutput(DIVMULT_OUTPUT+2,"Div/Mult #3");
		configOutput(DIVMULT_OUTPUT+3,"Div/Mult #4");

		configOutput(CLOCK_OUTPUT,"Clock");
		configOutput(MASTER_OUTPUT,"Metronome");
		configOutput(BEATPULSE_OUTPUT,"Beat Pulse");
		configOutput(BARPULSE_OUTPUT,"Bar Pulse");

		playBuffer[0].resize(0);
		playBuffer[1].resize(0);

		setClick(0);
	}

	void onReset() override {
		
		for (int i = 0; i < 2; i++) {
			clearSlot(i);
			play[i] = false;
		}
	}

	void onSampleRateChange() override {
		sampleRateCoeff = (double)APP->engine->getSampleRate() * 60;
		oneMsTime = (APP->engine->getSampleRate()) / 1000;

		for (int i = 0; i < 2; i++) {
			if (fileLoaded[i]) {
				play[i] = false;
				loadSample(storedPath[i],i);
			}
		}
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "BeatOnBar", json_boolean(beatOnBar));
		json_object_set_new(rootJ, "Slot1", json_string(storedPath[0].c_str()));
		json_object_set_new(rootJ, "Slot2", json_string(storedPath[1].c_str()));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t* beatOnBarJ = json_object_get(rootJ, "BeatOnBar");
		if (beatOnBarJ)
			beatOnBar = json_boolean_value(beatOnBarJ);
		json_t *slot1J = json_object_get(rootJ, "Slot1");
		if (slot1J) {
			storedPath[0] = json_string_value(slot1J);
			loadSample(storedPath[0], 0);
		}
		json_t *slot2J = json_object_get(rootJ, "Slot2");
		if (slot2J) {
			storedPath[1] = json_string_value(slot2J);
			loadSample(storedPath[1], 1);
		}
	}

	void calcBiquadLpf(double frequency, double samplerate, double Q) {
	    z1 = z2 = 0.0;
	    double Fc = frequency / samplerate;
	    double norm;
	    double K = tan(M_PI * Fc);
        norm = 1 / (1 + K / Q + K * K);
        a0 = K * K * norm;
        a1 = 2 * a0;
        a2 = a0;
        b1 = 2 * (K * K - 1) * norm;
        b2 = (1 - K / Q + K * K) * norm;
    }

	float biquadLpf(float in) {
	    double out = in * a0 + z1;
	    z1 = in * a1 + z2 - b1 * out;
	    z2 = in * a2 - b2 * out;
	    return out;
	}

	double hermiteInterpol(double x0, double x1, double x2, double x3, double t)	{
		double c0 = x1;
		double c1 = .5F * (x2 - x0);
		double c2 = x0 - (2.5F * x1) + (2 * x2) - (.5F * x3);
		double c3 = (.5F * (x3 - x0)) + (1.5F * (x1 - x2));
		return (((((c3 * t) + c2) * t) + c1) * t) + c0;
	}
	
	void menuLoadSample(int slot) {
		static const char FILE_FILTERS[] = "Wave (.wav):wav,WAV;All files (*.*):*.*";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
		fileLoaded[slot] = false;
		if (path) {
			loadSample(path, slot);
			storedPath[slot] = std::string(path);
		} else {
			fileLoaded[slot] = true;
		}
		if (storedPath[slot] == "") {
			fileLoaded[slot] = false;
		}
		free(path);
	}

	void loadSample(std::string path, int slot) {
		z1 = 0; z2 = 0;

		unsigned int c;
		unsigned int sr;
		drwav_uint64 tsc;
		float* pSampleData;
		pSampleData = drwav_open_and_read_file_f32(path.c_str(), &c, &sr, &tsc);

		if (pSampleData != NULL) {
			sampleRate[slot] = sr * 2;
			
			playBuffer[slot].clear();

			tempBuffer.clear();
			tempBuffer2.clear();

			if (tsc > 96000)
				tsc = 96000;	// set memory allocation limit to 96000 samples*/

			if (sr == APP->engine->getSampleRate()) {			//  **************************   NO RESAMPLE   ************************
				for (unsigned int i=0; i < tsc; i = i + c) {
					playBuffer[slot].push_back(pSampleData[i] * 5);
				}
				totalSampleC[slot] = playBuffer[slot].size();
				totalSamples[slot] = totalSampleC[slot]-1;
				drwav_free(pSampleData);

				//sampleRate = APP->engine->getSampleRate() * 2;
				sampleRate[slot] = APP->engine->getSampleRate();

			} else {											// ***************** RESAMPLE ****************************************
				for (unsigned int i=0; i < tsc; i = i + c) {
					tempBuffer.push_back(pSampleData[i] * 5);
					tempBuffer.push_back(0);
				}

				drwav_free(pSampleData);

				drwav_uint64 tempSampleC = tempBuffer.size();
				drwav_uint64 tempSamples = tempSampleC-1;
				
				for (unsigned int i = 1; i < tempSamples; i = i+2)
					tempBuffer[i] = tempBuffer[i-1] * .5f + tempBuffer[i+1] * .5f;

				tempBuffer[tempSamples] = tempBuffer[tempSamples-1] * .5f; // halve the last sample

				// ***************************************************************************

				double resampleCoeff = sampleRate[slot] * .5 / (APP->engine->getSampleRate());
				double currResamplePos = 0;
				int floorCurrResamplePos = 0;

				tempBuffer2.push_back(tempBuffer[0]);

				currResamplePos += resampleCoeff;
				floorCurrResamplePos = floor(currResamplePos);
				int temp;

				while ( floorCurrResamplePos < 1 ) {
					temp = tempBuffer[floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
								tempBuffer[floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);

					tempBuffer2.push_back(temp);

					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				} 

				while ( currResamplePos < tempSamples - 1) {

					tempBuffer2.push_back(	hermiteInterpol(tempBuffer[floorCurrResamplePos - 1],
															tempBuffer[floorCurrResamplePos],
															tempBuffer[floorCurrResamplePos + 1],
															tempBuffer[floorCurrResamplePos + 2],
															currResamplePos - floorCurrResamplePos)
											);

					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				}

				while ( floorCurrResamplePos < double(tempSampleC) ) {
					temp = tempBuffer[floorCurrResamplePos]* (1-(currResamplePos - floorCurrResamplePos)) + 
								tempBuffer[floorCurrResamplePos+1]*(currResamplePos - floorCurrResamplePos);
					tempBuffer2.push_back(temp);

					currResamplePos += resampleCoeff;
					floorCurrResamplePos = floor(currResamplePos);
				}

				// ***************************************************************************

				tempBuffer.clear();
				totalSampleC[slot] = tempBuffer2.size();

				calcBiquadLpf(20000.0, sampleRate[slot], 1);
				for (unsigned int i = 0; i < totalSampleC[slot]; i++)
					tempBuffer.push_back(biquadLpf(tempBuffer2[i]));

				for (unsigned int i = 0; i < totalSampleC[slot]; i = i + 2)
					playBuffer[slot].push_back(tempBuffer[i]);



				totalSampleC[slot] = playBuffer[slot].size();
				totalSamples[slot] = totalSampleC[slot]-1;
				sampleRate[slot] = APP->engine->getSampleRate();

			}

			tempBuffer.clear();
			tempBuffer2.clear();

			char* pathDup = strdup(path.c_str());
			fileDescription[slot] = basename(pathDup);

			free(pathDup);
			storedPath[slot] = path;

			fileLoaded[slot] = true;

		} else {
			fileLoaded[slot] = false;
			storedPath[slot] = "";
			fileDescription[slot] = "--none--";
		}
	};
	

	void clearSlot(int slot) {
		storedPath[slot] = "";
		fileDescription[slot] = "--none--";
		fileLoaded[slot] = false;
		playBuffer[slot].clear();
		totalSampleC[slot] = 0;
	}

	void setClick(int clickNo) {
		switch (clickNo) {
			case 0:
				loadSample(asset::plugin(pluginInstance, "res/clicks/click0_beat.wav"),0);
				loadSample(asset::plugin(pluginInstance, "res/clicks/click0_bar.wav"),1);
			break;

			case 1:
				loadSample(asset::plugin(pluginInstance, "res/clicks/click1_beat.wav"),0);
				loadSample(asset::plugin(pluginInstance, "res/clicks/click1_bar.wav"),1);
			break;

			case 2:
				loadSample(asset::plugin(pluginInstance, "res/clicks/click2_beat.wav"),0);
				loadSample(asset::plugin(pluginInstance, "res/clicks/click2_bar.wav"),1);
			break;
		}
	}

	void process(const ProcessArgs &args) override {
		
		clickOutput = 0.f;

		click_setting = params[CLICK_BUT_PARAM].getValue();
		lights[CLICK_BUT_LIGHT].setBrightness(click_setting);

		// **********  RESET

		resetBut = params[RESET_BUT_PARAM].getValue();
		if (resetBut)
			resetValue = 1;
		else
			resetValue = inputs[RESET_INPUT].getVoltage();

		lights[RESET_BUT_LIGHT].setBrightness(resetValue);

		if (resetValue >= 1 && prevResetValue < 1) {
			//clockSample = 1.0;
			clockSample = clockMaxSample+1;
			outputs[CLOCK_OUTPUT].setVoltage(0.f);
			for (int d = 0; d < 4; d++) {
				divPulse[d] = false;
				divClockSample[d] = 1.0;
				divMaxSample[d] = 0.0;
				outputs[DIVMULT_OUTPUT+d].setVoltage(0.f);
			}
			midBeatPlayed = false;
			beatCounter = 1;
			extSync = false;

			resetPulse = true;
			resetPulseTime = oneMsTime;
		}
		prevResetValue = resetValue;

		// ********* EXTERNAL CONNECTION

		extConn = inputs[EXTCLOCK_INPUT].isConnected();
		if (extConn && !prevExtConn) {
			extSync = false;
			bpm = 0.0;
		}
		prevExtConn = extConn;


		// ********* RUN SETTING

		run_setting = params[RUN_BUT_PARAM].getValue();
		lights[RUN_BUT_LIGHT].setBrightness(run_setting);

		runTrig = inputs[RUN_INPUT].getVoltage();
		if (runTrig && !prevRunTrig) {
			if (run_setting)
				run_setting = 0;
			else
				run_setting = 1;
			params[RUN_BUT_PARAM].setValue(run_setting);
		}
		prevRunTrig = runTrig;
		
		if (!extConn && run_setting && !prev_run_setting) {
			clockSample = 1.0;
			beatCounter = 1;
		}

		prev_run_setting = run_setting;
		
		// ********** EXTERNAL SYNC

		if (extConn) {
			extTrigValue = inputs[EXTCLOCK_INPUT].getVoltage();
				
			if (extTrigValue >= 1 && prevExtTrigValue < 1) {

				if (extSync) {
					clockMaxSample = clockSample;
					midBeatMaxSample = clockMaxSample / 2;
					clockSample = 1.0;

					// calculate bpms
					bpm = round(sampleRateCoeff / clockMaxSample);
					if (bpm > 999)
						bpm = 999;

				} else {
					bpm = 0.0;
					extSync = true;
					clockSample = 1.0;
				}

				if (run_setting)
					extBeat = true;

			} else {
				extBeat = false;
			}
			prevExtTrigValue = extTrigValue;

		} else

			bpm = (double)params[BPM_KNOB_PARAM].getValue()/10;

		// **************   RUN PROCESS   ***************

		if (run_setting) {

			// ****** CLOCK PULSE WIDTH

			maxPulseSample = clockMaxSample * (double)params[PW_KNOB_PARAM].getValue();
			if (clockSample > maxPulseSample)
				outputs[CLOCK_OUTPUT].setVoltage(0.f);

			// ************ DIV / MULT

			for (int d = 0; d < 4; d++) {

				if (params[DIVMULT_KNOB_PARAM+d].getValue() > 20 && divClockSample[d] > divMaxSample[d]) {
					// ***** CLOCK MULTIPLIER *****
					divClockSample[d] = 1.0;
					if (params[DIVMULT_KNOB_PARAM+d].getValue() > 20) {
						divPulse[d] = true;
						outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
					}
				}

				// ***** CLOCK MULTIPLIER/DIVIDER   PULSE WIDTH OFF
				if (divPulse[d] && divClockSample[d] > divMaxSample[d] * params[DIVPW_KNOB_PARAM+d].getValue()) {
					divPulse[d] = false;
					outputs[DIVMULT_OUTPUT+d].setVoltage(0.f);
				}
			}

			// ***********  MID BEAT PULSES WHEN USING TEMPOS WITH EIGHTH NOTES

			currentBeatMaxPerBar = beatMaxPerBar[int(params[BEAT_KNOB_PARAM].getValue())];

			if (params[BEAT_KNOB_PARAM].getValue() > 5 && !midBeatPlayed && clockSample > midBeatMaxSample)  {
				beatCounter++;
				if (beatCounter > currentBeatMaxPerBar) {
					beatCounter = 1;
					samplePos[BAR] = 0;
					play[BAR] = true;
					barPulse = true;
					barPulseTime = oneMsTime;
					if (beatOnBar) {
						beatPulse = true;
						beatPulseTime = oneMsTime;
					}
				} else {
					samplePos[BEAT] = 0;
					play[BEAT] = true;
					beatPulse = true;
					beatPulseTime = oneMsTime;
				}
				midBeatPlayed = true;
			}

			if (!extConn) {

				//	*************************  INTERNAL CLOCK  ******************

				clockMaxSample = sampleRateCoeff / bpm;
				midBeatMaxSample = clockMaxSample / 2;
				
				if (clockSample > clockMaxSample)  {
					midBeatPlayed = false;

					beatCounter++;

					clockSample -= clockMaxSample;
					
					for (int d = 0; d < 4; d++) {
						
						if (params[DIVMULT_KNOB_PARAM+d].getValue() > 20) {
							// ***** CLOCK MULTIPLIER *****
							divMaxSample[d] = clockMaxSample / (divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]);
							divClockSample[d] = 1.0;
							divPulse[d] = true;
							outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
						} else {
							// ***** CLOCK DIVIDER *****
							divMaxSample[d] = clockMaxSample * (divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]);
							divCount[d]++;
							if (divCount[d] > divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]) {
								divClockSample[d] = 1.0;
								divCount[d] = 1;
								divPulse[d] = true;
								outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
							}
						}
					}

					if (beatCounter > currentBeatMaxPerBar) {
						beatCounter = 1;
						samplePos[BAR] = 0;
						play[BAR] = true;
						barPulse = true;
						barPulseTime = oneMsTime;
						if (beatOnBar) {
							beatPulse = true;
							beatPulseTime = oneMsTime;
						}
					} else {
						samplePos[BEAT] = 0;
						play[BEAT] = true;
						beatPulse = true;
						beatPulseTime = oneMsTime;
					}
					
					outputs[CLOCK_OUTPUT].setVoltage(10.f);
					
				}

			} else {		

				// ************************ EXTERNAL CLOCK ******************

				if (extBeat) {

					midBeatPlayed = false;
					beatCounter++;

					if (extSync) {

						// ********** SYNCED BEAT

						for (int d = 0; d < 4; d++) {
							if (params[DIVMULT_KNOB_PARAM+d].getValue() > 20) {
								// ***** CLOCK MULTIPLIER *****
								divMaxSample[d] = clockMaxSample / (divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]);
								divClockSample[d] = 1.0;
								divPulse[d] = true;
								outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
							} else {
								// ***** CLOCK DIVIDER *****
								divMaxSample[d] = clockMaxSample * (divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]);
								divCount[d]++;
								if (divCount[d] > divMult[int(params[DIVMULT_KNOB_PARAM+d].getValue())]) {
									divClockSample[d] = 1.0;
									divCount[d] = 1;
									divPulse[d] = true;
									outputs[DIVMULT_OUTPUT+d].setVoltage(10.f);
								}
							}
						}

						if (beatCounter > currentBeatMaxPerBar) {
							// ***** BAR DETECTED *****
							beatCounter = 1;
							samplePos[BAR] = 0;
							play[BAR] = true;
							barPulse = true;
							barPulseTime = oneMsTime;
							if (beatOnBar) {
								beatPulse = true;
								beatPulseTime = oneMsTime;
							}
						} else {
							// ***** BEAT DETECTED *****
							samplePos[BEAT] = 0;
							play[BEAT] = true;
							beatPulse = true;
							beatPulseTime = oneMsTime;
						}
						
						outputs[CLOCK_OUTPUT].setVoltage(10.f);
					
					} else {

						//	************ UNSYNCED BEAT
					
						beatCounter++;
						samplePos[BEAT] = 0;
						play[BEAT] = true;
						beatPulse = true;
						beatPulseTime = oneMsTime;

					}
				}
			}

			clockSample++;
			divClockSample[0]++;
			divClockSample[1]++;
			divClockSample[2]++;
			divClockSample[3]++;
			
		}


		//	************ AUDIO CLICK

		if (click_setting) {
			for (int i = 0; i < 2; i++) {
				if (fileLoaded[i] && play[i] && floor(samplePos[i]) < totalSampleC[i]) {
					clickOutput = playBuffer[i][samplePos[i]] * params[CLICKVOL_KNOB_PARAM].getValue();
					if (clickOutput > 10.f)
						clickOutput = 10;
					else if (clickOutput < -10.f)
						clickOutput = -10;
					samplePos[i]++;
				} else {
					play[i] = false;
				}

				outputs[MASTER_OUTPUT].setVoltage(clickOutput);
			}
		} else {
			play[BEAT] = false;
			play[BAR] = false;
		}

		//	********** RESET AND BEAT AND BAR PULSES

		if (resetPulse) {
			resetPulseTime--;
			if (resetPulseTime < 0) {
				resetPulse = false;
				outputs[RESET_OUTPUT].setVoltage(0.f);
			} else
				outputs[RESET_OUTPUT].setVoltage(10.f);
		}

		if (beatPulse) {
			beatPulseTime--;
			if (beatPulseTime < 0) {
				beatPulse = false;
				outputs[BEATPULSE_OUTPUT].setVoltage(0.f);
			} else
				outputs[BEATPULSE_OUTPUT].setVoltage(10.f);
		}

		if (barPulse) {
			barPulseTime--;
			if (barPulseTime < 0) {
				barPulse = false;
				outputs[BARPULSE_OUTPUT].setVoltage(0.f);
			} else
				outputs[BARPULSE_OUTPUT].setVoltage(10.f);
		}
	}
};

struct ClockerDisplayTempo : TransparentWidget {
	Clocker *module;
	int frame = 0;
	ClockerDisplayTempo() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 13);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				nvgFillColor(args.vg, nvgRGBA(0xdd, 0xdd, 0x33, 0xff)); 
				
				int tempBpmInteger;
				std::string tempBpm;
				std::string tempBpmInt;
				std::string tempBpmDec;

				if (!module->inputs[module->EXTCLOCK_INPUT].isConnected()) {
					tempBpmInteger = int(module->bpm * 10 + .5);

					tempBpm = to_string(tempBpmInteger);
					tempBpmInt = tempBpm.substr(0, tempBpm.size()-1);
					tempBpmDec = tempBpm.substr(tempBpm.size() - 1);
					tempBpm = tempBpmInt+"."+tempBpmDec;

					if (tempBpmInteger < 1000)
						nvgTextBox(args.vg, 14.5, 16.3, 60, tempBpm.c_str(), NULL);
					else
						nvgTextBox(args.vg, 4, 16.3, 60, tempBpm.c_str(), NULL);
				} else {
					
					tempBpmInteger = int(module->bpm);
					tempBpm = to_string(tempBpmInteger)+".X";
					if (tempBpmInteger < 100)
						nvgTextBox(args.vg, 14.5, 16.3, 60, tempBpm.c_str(), NULL);
					else
						nvgTextBox(args.vg, 4, 16.3, 60, tempBpm.c_str(), NULL);
					
				}
			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct ClockerDisplayBeat : TransparentWidget {
	Clocker *module;
	int frame = 0;
	ClockerDisplayBeat() {
	}

	void onButton(const event::Button &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 12);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgFillColor(args.vg, nvgRGBA(0xaa, 0xcc, 0xff, 0xe0));

				int tempValue = int(module->params[module->BEAT_KNOB_PARAM].getValue());
				if (tempValue > 10)
					nvgTextBox(args.vg, 0, 15, 60, module->beatDisplay[tempValue].c_str(), NULL);
				else
					nvgTextBox(args.vg, 10, 15, 60, module->beatDisplay[tempValue].c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}

	void createContextMenu() {
		Clocker *module = dynamic_cast<Clocker *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				Clocker* module;
				int valueNr;
				void onAction(const event::Action& e) override {
					module->params[module->BEAT_KNOB_PARAM].setValue(float(valueNr));
				}
			};

			std::string menuNames[17] = {"2/4", "3/4", "4/4", "5/4", "6/4", "7/4", "5/8", "6/8", "7/8", "8/8", "9/8", "10/8", "11/8", "12/8", "13/8", "14/8", "15/8"};
			for (int i = 0; i < 17; i++) {
				ThisItem* thisItem = createMenuItem<ThisItem>(menuNames[i]);
				thisItem->rightText = CHECKMARK(int(module->params[module->BEAT_KNOB_PARAM].getValue()) == i);
				thisItem->module = module;
				thisItem->valueNr = i;
				menu->addChild(thisItem);
			}
		}
	}
};

struct ClockerDisplayDiv1 : TransparentWidget {
	Clocker *module;
	int frame = 0;
	ClockerDisplayDiv1() {
	}

	void onButton(const event::Button &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 12);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				int tempValue = int(module->params[module->DIVMULT_KNOB_PARAM+0].getValue());
				float tempXpos = 3;
				if (tempValue == 20)
					tempXpos = 12.8;

				if (tempValue < 20) {
					nvgFillColor(args.vg, nvgRGBA(0xdd, 0x33, 0x33, 0xff)); 
					nvgTextBox(args.vg, tempXpos, 15.5, 60, module->divMultDisplay[tempValue].c_str(), NULL);
				} else {
					nvgFillColor(args.vg, nvgRGBA(0x33, 0xdd, 0x33, 0xff)); 
					nvgTextBox(args.vg, tempXpos, 15.5, 60, module->divMultDisplay[tempValue].c_str(), NULL);
				}
			}
		}
		Widget::drawLayer(args, layer);
	}

	void createContextMenu() {
		Clocker *module = dynamic_cast<Clocker *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				Clocker* module;
				int valueNr;
				void onAction(const event::Action& e) override {
					module->params[module->DIVMULT_KNOB_PARAM+0].setValue(float(valueNr));
				}
			};

			const std::string menuNames[41] = {"/256", "/128", "/64", "/32", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x32", "x64", "x128", "x256"};
			for (int i = 0; i < 41; i++) {
				ThisItem* thisItem = createMenuItem<ThisItem>(menuNames[i]);
				thisItem->rightText = CHECKMARK(int(module->params[module->DIVMULT_KNOB_PARAM+0].getValue()) == i);
				thisItem->module = module;
				thisItem->valueNr = i;
				menu->addChild(thisItem);
			}
		}
	}
};

struct ClockerDisplayDiv2 : TransparentWidget {
	Clocker *module;
	int frame = 0;
	ClockerDisplayDiv2() {
	}

	void onButton(const event::Button &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 12);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				int tempValue = int(module->params[module->DIVMULT_KNOB_PARAM+1].getValue());
				float tempXpos = 3;
				if (tempValue == 20)
					tempXpos = 12.8;

				if (tempValue < 20) {
					nvgFillColor(args.vg, nvgRGBA(0xdd, 0x33, 0x33, 0xff)); 
					nvgTextBox(args.vg, tempXpos, 15.5, 60, module->divMultDisplay[tempValue].c_str(), NULL);
				} else {
					nvgFillColor(args.vg, nvgRGBA(0x33, 0xdd, 0x33, 0xff)); 
					nvgTextBox(args.vg, tempXpos, 15.5, 60, module->divMultDisplay[tempValue].c_str(), NULL);
				}
			}
		}
		Widget::drawLayer(args, layer);
	}

	void createContextMenu() {
		Clocker *module = dynamic_cast<Clocker *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				Clocker* module;
				int valueNr;
				void onAction(const event::Action& e) override {
					module->params[module->DIVMULT_KNOB_PARAM+1].setValue(float(valueNr));
				}
			};

			const std::string menuNames[41] = {"/256", "/128", "/64", "/32", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x32", "x64", "x128", "x256"};
			for (int i = 0; i < 41; i++) {
				ThisItem* thisItem = createMenuItem<ThisItem>(menuNames[i]);
				thisItem->rightText = CHECKMARK(int(module->params[module->DIVMULT_KNOB_PARAM+1].getValue()) == i);
				thisItem->module = module;
				thisItem->valueNr = i;
				menu->addChild(thisItem);
			}
		}
	}
};

struct ClockerDisplayDiv3 : TransparentWidget {
	Clocker *module;
	int frame = 0;
	ClockerDisplayDiv3() {
	}

	void onButton(const event::Button &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 12);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				int tempValue = int(module->params[module->DIVMULT_KNOB_PARAM+2].getValue());
				float tempXpos = 3;
				if (tempValue == 20)
					tempXpos = 12.8;

				if (tempValue < 20) {
					nvgFillColor(args.vg, nvgRGBA(0xdd, 0x33, 0x33, 0xff)); 
					nvgTextBox(args.vg, tempXpos, 15.5, 60, module->divMultDisplay[tempValue].c_str(), NULL);
				} else {
					nvgFillColor(args.vg, nvgRGBA(0x33, 0xdd, 0x33, 0xff)); 
					nvgTextBox(args.vg, tempXpos, 15.5, 60, module->divMultDisplay[tempValue].c_str(), NULL);
				}
			}
		}
		Widget::drawLayer(args, layer);
	}

	void createContextMenu() {
		Clocker *module = dynamic_cast<Clocker *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				Clocker* module;
				int valueNr;
				void onAction(const event::Action& e) override {
					module->params[module->DIVMULT_KNOB_PARAM+2].setValue(float(valueNr));
				}
			};

			const std::string menuNames[41] = {"/256", "/128", "/64", "/32", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x32", "x64", "x128", "x256"};
			for (int i = 0; i < 41; i++) {
				ThisItem* thisItem = createMenuItem<ThisItem>(menuNames[i]);
				thisItem->rightText = CHECKMARK(int(module->params[module->DIVMULT_KNOB_PARAM+2].getValue()) == i);
				thisItem->module = module;
				thisItem->valueNr = i;
				menu->addChild(thisItem);
			}
		}
	}
};

struct ClockerDisplayDiv4 : TransparentWidget {
	Clocker *module;
	int frame = 0;
	ClockerDisplayDiv4() {
	}

	void onButton(const event::Button &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/DSEG14ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 12);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);

				int tempValue = int(module->params[module->DIVMULT_KNOB_PARAM+3].getValue());
				float tempXpos = 3;
				if (tempValue == 20)
					tempXpos = 12.8;

				if (tempValue < 20) {
					nvgFillColor(args.vg, nvgRGBA(0xdd, 0x33, 0x33, 0xff)); 
					nvgTextBox(args.vg, tempXpos, 15.5, 60, module->divMultDisplay[tempValue].c_str(), NULL);
				} else {
					nvgFillColor(args.vg, nvgRGBA(0x33, 0xdd, 0x33, 0xff)); 
					nvgTextBox(args.vg, tempXpos, 15.5, 60, module->divMultDisplay[tempValue].c_str(), NULL);
				}
			}
		}
		Widget::drawLayer(args, layer);
	}

	void createContextMenu() {
		Clocker *module = dynamic_cast<Clocker *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			struct ThisItem : MenuItem {
				Clocker* module;
				int valueNr;
				void onAction(const event::Action& e) override {
					module->params[module->DIVMULT_KNOB_PARAM+3].setValue(float(valueNr));
				}
			};

			std::string menuNames[41] = {"/256", "/128", "/64", "/32", "/17", "/16", "/15", "/14", "/13", "/12", "/11", "/10", "/9", "/8", "/7", "/6", "/5", "/4", "/3", "/2", "1",
								"x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x32", "x64", "x128", "x256"};
			for (int i = 0; i < 41; i++) {
				ThisItem* thisItem = createMenuItem<ThisItem>(menuNames[i]);
				thisItem->rightText = CHECKMARK(int(module->params[module->DIVMULT_KNOB_PARAM+3].getValue()) == i);
				thisItem->module = module;
				thisItem->valueNr = i;
				menu->addChild(thisItem);
			}
		}
	}
};

struct ClockerDebugDisplay : TransparentWidget {
	Clocker *module;
	int frame = 0;
	ClockerDebugDisplay() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/Nunito-bold.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0xff)); 
				
				/*
				nvgTextBox(args.vg, 9, 0,120, module->debugDisplay.c_str(), NULL);
				nvgTextBox(args.vg, 9, 10,120, module->debugDisplay2.c_str(), NULL);
				nvgTextBox(args.vg, 9, 20,120, module->debugDisplay3.c_str(), NULL);
				nvgTextBox(args.vg, 9, 30,120, module->debugDisplay4.c_str(), NULL);
				nvgTextBox(args.vg, 9, 40,120, module->debugDisplay5.c_str(), NULL);
				nvgTextBox(args.vg, 9, 50,120, module->debugDisplay6.c_str(), NULL);
				nvgTextBox(args.vg, 9, 60,120, module->debugDisplay7.c_str(), NULL);
				*/

			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct ClockerWidget : ModuleWidget {
	ClockerWidget(Clocker *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Clocker.svg")));

		addChild(createWidget<ScrewBlack>(Vec(0, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH))); 

		/*
		{
			ClockerDebugDisplay *display = new ClockerDebugDisplay();
			display->box.pos = Vec(0, 10);
			display->box.size = Vec(307, 100);
			display->module = module;
			addChild(display);
		}
		*/

		{
			ClockerDisplayTempo *display = new ClockerDisplayTempo();
			display->box.pos = mm2px(Vec(13.222, 17.5));
			display->box.size = mm2px(Vec(16.8, 6.5));
			display->module = module;
			addChild(display);
		}

		{
			ClockerDisplayBeat *display = new ClockerDisplayBeat();
			display->box.pos = mm2px(Vec(22, 52));
			display->box.size = mm2px(Vec(14.5, 6));
			display->module = module;
			addChild(display);
		}

		{
			ClockerDisplayDiv1 *display = new ClockerDisplayDiv1();
			display->box.pos = mm2px(Vec(15.3, 80.2));
			display->box.size = mm2px(Vec(15, 6.3));
			display->module = module;
			addChild(display);
		}

		{
			ClockerDisplayDiv2 *display = new ClockerDisplayDiv2();
			display->box.pos = mm2px(Vec(15.3, 91.2));
			display->box.size = mm2px(Vec(15, 6.3));
			display->module = module;
			addChild(display);
		}

		{
			ClockerDisplayDiv3 *display = new ClockerDisplayDiv3();
			display->box.pos = mm2px(Vec(15.3, 102.2));
			display->box.size = mm2px(Vec(15, 6.3));
			display->module = module;
			addChild(display);
		}

		{
			ClockerDisplayDiv4 *display = new ClockerDisplayDiv4();
			display->box.pos = mm2px(Vec(15.3, 113.2));
			display->box.size = mm2px(Vec(15, 6.3));
			display->module = module;
			addChild(display);
		}


		const float xExtClock = 7.5f;
		const float xResetIn = 36.f;
		const float xResetBut = 36.f;

		const float xRun = 7.5f;
		const float xBpmKnob = 22.f;
		const float xPwKnob = 36.f;

		const float xBeatKnob = 10.f;

		const float xCLickBut = 24.5f;
		const float xClickVolKnob = 35.f;

		const float xDivKnob = 8.7f;
		const float xDivOut = 49.1f;

		// -------------------------------

		const float yExtClock = 18.5f;
		const float yRunBut = 33.5f;
		const float yRunIn = 42.5f;
		
		const float yBpmKob = 34.5f;

		const float yRstBut = 17.5f;
		const float yRstIn = 26.5f;

		const float yPwClOut = 43.f;

		const float yClick1 = 63.f;
		const float yClick2 = 67.f;

		const float yDivKn1 = 83.5f;
		const float yDivKn2 = 94.5f;
		const float yDivKn3 = 105.5f;
		const float yDivKn4 = 116.5f;

		const float yClockOut = 17.5f;
		const float yResetOut = 31.5f;

		const float yBeatOut = 48.f;
		const float yBarOut = 60.f;
		const float yClickOut = 72.f;

		const float yDiv1 = 88.5f;
		const float yDiv2 = 98.f;
		const float yDiv3 = 107.5f;
		const float yDiv4 = 117.f;

		// buttons --- 4.1
		// trimpot --- x  3.7 --- y 4.3
		// trimpot senza stanghetta --- y 3.7
		// smallRoundKnob --- x 4.6 --- y 5.1
		// roundBlackKnob --- x 5.7 --- y 6.4
		// input/output --- 4.5
		// three horizontal switch ---- x 5.5 --- y 2.8

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xExtClock, yExtClock)), module, Clocker::EXTCLOCK_INPUT));
		
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xResetIn, yRstIn)), module, Clocker::RESET_INPUT));
		addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(xResetBut, yRstBut)), module, Clocker::RESET_BUT_PARAM, Clocker::RESET_BUT_LIGHT));
		

		addParam(createLightParamCentered<VCVLightBezelLatch<BlueLight>>(mm2px(Vec(xRun, yRunBut)), module, Clocker::RUN_BUT_PARAM, Clocker::RUN_BUT_LIGHT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xRun, yRunIn)), module, Clocker::RUN_INPUT));

		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(xBpmKnob, yBpmKob)), module, Clocker::BPM_KNOB_PARAM));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(xPwKnob, yPwClOut)), module, Clocker::PW_KNOB_PARAM));
		
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xBeatKnob, yClick1 + .9f)), module, Clocker::BEAT_KNOB_PARAM));
		
		addParam(createLightParamCentered<VCVLightBezelLatch<YellowLight>>(mm2px(Vec(xCLickBut, yClick2)), module, Clocker::CLICK_BUT_PARAM, Clocker::CLICK_BUT_LIGHT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(xClickVolKnob, yClick2 + .3f)), module, Clocker::CLICKVOL_KNOB_PARAM));
		
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xDivKnob, yDivKn1)), module, Clocker::DIVMULT_KNOB_PARAM+0));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xDivKnob, yDivKn2)), module, Clocker::DIVMULT_KNOB_PARAM+1));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xDivKnob, yDivKn3)), module, Clocker::DIVMULT_KNOB_PARAM+2));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xDivKnob, yDivKn4)), module, Clocker::DIVMULT_KNOB_PARAM+3));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(xPwKnob, yDivKn1)), module, Clocker::DIVPW_KNOB_PARAM+0));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(xPwKnob, yDivKn2)), module, Clocker::DIVPW_KNOB_PARAM+1));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(xPwKnob, yDivKn3)), module, Clocker::DIVPW_KNOB_PARAM+2));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(xPwKnob, yDivKn4)), module, Clocker::DIVPW_KNOB_PARAM+3));
		
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xDivOut, yClockOut)), module, Clocker::CLOCK_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xDivOut, yResetOut)), module, Clocker::RESET_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xDivOut, yBeatOut)), module, Clocker::BEATPULSE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xDivOut, yBarOut)), module, Clocker::BARPULSE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xDivOut, yClickOut)), module, Clocker::MASTER_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xDivOut, yDiv1)), module, Clocker::DIVMULT_OUTPUT+0));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xDivOut, yDiv2)), module, Clocker::DIVMULT_OUTPUT+1));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xDivOut, yDiv3)), module, Clocker::DIVMULT_OUTPUT+2));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xDivOut, yDiv4)), module, Clocker::DIVMULT_OUTPUT+3));
	}

	void appendContextMenu(Menu *menu) override {
	   	Clocker *module = dynamic_cast<Clocker*>(this->module);
			assert(module);
		
		menu->addChild(new MenuSeparator());

		menu->addChild(createSubmenuItem("Click Presets", "", [=](Menu * menu) {
			menu->addChild(createMenuItem("Standard", "", [=]() {module->setClick(0);}));
			menu->addChild(createMenuItem("Click1", "", [=]() {module->setClick(1);}));
			menu->addChild(createMenuItem("Click2", "", [=]() {module->setClick(2);}));
		}));

		menu->addChild(new MenuSeparator());

		menu->addChild(createMenuLabel("Audio Clicks"));
		menu->addChild(createMenuItem("Load BEAT", "", [=]() {module->menuLoadSample(0);}));
		menu->addChild(createMenuItem("File: " + module->fileDescription[0], "", [=]() {module->menuLoadSample(0);}));
		menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot(0);}));
		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Load BAR", "", [=]() {module->menuLoadSample(1);}));
		menu->addChild(createMenuItem("File: " + module->fileDescription[1], "", [=]() {module->menuLoadSample(1);}));
		menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot(1);}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Beat pulse also on Bar", "", &module->beatOnBar));


	}
};

Model *modelClocker = createModel<Clocker, ClockerWidget>("Clocker");