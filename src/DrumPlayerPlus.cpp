#define NO_INTERP 0
#define LINEAR1_INTERP 1
#define LINEAR2_INTERP 2
#define HERMITE_INTERP 3
#define NORMALLED_OUTS 0
#define SOLO_OUTS 1
#define UNCONNECTED_ON_4 2

#include "plugin.hpp"
#include "osdialog.h"
//#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <vector>
#include "cmath"
#include <dirent.h>
#include <libgen.h>

using namespace std;

struct DrumPlayerPlus : Module {
	enum ParamIds {
		ENUMS(TRIGVOL_PARAM,4),
		ENUMS(TRIGVOLATNV_PARAM,4),
		ENUMS(ACCVOL_PARAM,4),
		ENUMS(ACCVOLATNV_PARAM,4),
		ENUMS(SPEED_PARAM,4),
		ENUMS(SPEEDATNV_PARAM,4),
		ENUMS(LIMIT_SWITCH,4),
		ENUMS(CHOKE_SWITCH,3),
		NUM_PARAMS 
	};
	enum InputIds {
		ENUMS(TRIG_INPUT,4),
		ENUMS(TRIGVOL_INPUT,4),
		ENUMS(ACC_INPUT,4),
		ENUMS(ACCVOL_INPUT,4),
		ENUMS(SPEED_INPUT,4),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(OUT_OUTPUT,4),
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
  
	unsigned int channels[4];
	unsigned int sampleRate[4];
	drwav_uint64 totalSampleC[4];
	drwav_uint64 totalSamples[4];

	vector<float> playBuffer[4][2];

	bool fileLoaded[4] = {false, false, false, false};

	bool play[4] = {false, false, false, false};

	double samplePos[4] = {0,0,0,0};
	double sampleCoeff[4];
	double currentSpeed = 0;

	double prevSampleWeight [4] = {0,0,0,0};
	double currSampleWeight [4] = {0,0,0,0};
	double prevSamplePos[4] = {0,0,0,0};

	std::string storedPath[4] = {"","","",""};
	std::string fileDescription[4] = {"--none--","--none--","--none--","--none--"};
	std::string fileDisplay[4] = {"-----","-----","-----","-----"};
	std::string userFolderName = "";
	vector <std::string> browserFileName;
	vector <std::string> browserFileDisplay;

	float trigValue[4] = {0.f, 0.f, 0.f, 0.f};
	float prevTrigValue[4] = {0.f, 0.f, 0.f, 0.f};
	bool choking[4] = {false, false, false, false};
	float chokeValue[4] = {1.f, 1.f, 1.f, 1.f};
	bool fading[4] = {false, false, false, false};
	float fadingValue[4] = {0.f, 0.f, 0.f, 0.f};
	float fadeDecrement = 1000 / (APP->engine->getSampleRate()); // calcs volume decrement for 1ms fade
	double fadedPosition[4] = {0, 0, 0, 0};
	
	float level[4];
	float currentOutput;
	float summedOutput;

	int interpolationMode = HERMITE_INTERP;
	int outsMode = 0;
	int antiAlias = 1;

	//std::string debugDisplay = "X";

	double a0, a1, a2, b1, b2, z1, z2;

	DrumPlayerPlus() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configInput(TRIG_INPUT,"Trig #1");
		configInput(TRIG_INPUT+1,"Trig #2");
		configInput(TRIG_INPUT+2,"Trig #3");
		configInput(TRIG_INPUT+3,"Trig #4");
		configInput(TRIGVOL_INPUT,"Stand.Lev.CV #1");
		configInput(TRIGVOL_INPUT+1,"Stand.Lev.CV #2");
		configInput(TRIGVOL_INPUT+2,"Stand.Lev.CV #3");
		configInput(TRIGVOL_INPUT+3,"Stand.Lev.CV #4");
		configParam(TRIGVOL_PARAM, 0.f, 2.0f, 1.0f, "Standard Level #1", "%", 0, 100);
		configParam(TRIGVOL_PARAM+1, 0.f, 2.0f, 1.0f, "Standard Level #2", "%", 0, 100);
		configParam(TRIGVOL_PARAM+2, 0.f, 2.0f, 1.0f, "Standard Level #3", "%", 0, 100);
		configParam(TRIGVOL_PARAM+3, 0.f, 2.0f, 1.0f, "Standard Level #4", "%", 0, 100);
		configParam(TRIGVOLATNV_PARAM, -1.f, 1.0f, 0.f, "Stand.Lev.CV Attenuv. #1", "%", 0, 100);
		configParam(TRIGVOLATNV_PARAM+1, -1.f, 1.0f, 0.f, "Stand.Lev.CV Attenuv. #2", "%", 0, 100);
		configParam(TRIGVOLATNV_PARAM+2, -1.f, 1.0f, 0.f, "Stand.Lev.CV Attenuv. #3", "%", 0, 100);
		configParam(TRIGVOLATNV_PARAM+3, -1.f, 1.0f, 0.f, "Stand.Lev.CV Attenuv. #4", "%", 0, 100);
		configInput(ACC_INPUT,"Accent #1");
		configInput(ACC_INPUT+1,"Accent #2");
		configInput(ACC_INPUT+2,"Accent #3");
		configInput(ACC_INPUT+3,"Accent #4");
		configInput(ACCVOL_INPUT,"Accent.Lev.CV #1");
		configInput(ACCVOL_INPUT+1,"Accent.Lev.CV #2");
		configInput(ACCVOL_INPUT+2,"Accent.Lev.CV #3");
		configInput(ACCVOL_INPUT+3,"Accent.Lev.CV #4");
		configParam(ACCVOL_PARAM, 0.f, 2.0f, 1.0f, "Accent Level #1", "%", 0, 100);
		configParam(ACCVOL_PARAM+1, 0.f, 2.0f, 1.0f, "Accent Level #2", "%", 0, 100);
		configParam(ACCVOL_PARAM+2, 0.f, 2.0f, 1.0f, "Accent Level #3", "%", 0, 100);
		configParam(ACCVOL_PARAM+3, 0.f, 2.0f, 1.0f, "Accent Level #4", "%", 0, 100);
		configParam(ACCVOLATNV_PARAM, -1.f, 1.0f, 0.f, "Accent.Lev.CV Attenuv. #1", "%", 0, 100);
		configParam(ACCVOLATNV_PARAM+1, -1.f, 1.0f, 0.f, "Accent.Lev.CV Attenuv. #2", "%", 0, 100);
		configParam(ACCVOLATNV_PARAM+2, -1.f, 1.0f, 0.f, "Accent.Lev.CV Attenuv. #3", "%", 0, 100);
		configParam(ACCVOLATNV_PARAM+3, -1.f, 1.0f, 0.f, "Accent.Lev.CV Attenuv. #4", "%", 0, 100);
		configParam(SPEEDATNV_PARAM, -1.0f, 1.0f, 0.0f, "Speed CV Attenuv. #1");
		configParam(SPEEDATNV_PARAM+1, -1.0f, 1.0f, 0.0f, "Speed CV Attenuv. #2");
		configParam(SPEEDATNV_PARAM+2, -1.0f, 1.0f, 0.0f, "Speed CV Attenuv. #3");
		configParam(SPEEDATNV_PARAM+3, -1.0f, 1.0f, 0.0f, "Speed CV Attenuv. #4");
		configParam(SPEED_PARAM, 0.01f, 2.0f, 1.0f, "Speed #1", "x", 0, 1);
		configParam(SPEED_PARAM+1, 0.01f, 2.0f, 1.0f, "Speed #2", "x", 0, 1);
		configParam(SPEED_PARAM+2, 0.01f, 2.0f, 1.0f, "Speed #3", "x", 0, 1);
		configParam(SPEED_PARAM+3, 0.01f, 2.0f, 1.0f, "Speed #4", "x", 0, 1);
		configInput(SPEED_INPUT,"Speed CV #1");
		configInput(SPEED_INPUT+1,"Speed CV #2");
		configInput(SPEED_INPUT+2,"Speed CV #3");
		configInput(SPEED_INPUT+3,"Speed CV #4");
		configSwitch(LIMIT_SWITCH, 0.f, 1.f, 0.f, "Limit #1", {"Off", "±5v"});
		configSwitch(LIMIT_SWITCH+1, 0.f, 1.f, 0.f, "Limit #2", {"Off", "±5v"});
		configSwitch(LIMIT_SWITCH+2, 0.f, 1.f, 0.f, "Limit #3", {"Off", "±5v"});
		configSwitch(LIMIT_SWITCH+3, 0.f, 1.f, 0.f, "Limit #4", {"Off", "±5v"});
		configSwitch(CHOKE_SWITCH, 0.f, 1.f, 0.f, "Choke #1", {"Off", "On"});
		configSwitch(CHOKE_SWITCH+1, 0.f, 1.f, 0.f, "Choke #2", {"Off", "On"});
		configSwitch(CHOKE_SWITCH+2, 0.f, 1.f, 0.f, "Choke #3", {"Off", "On"});

		configOutput(OUT_OUTPUT,"out #1");
		configOutput(OUT_OUTPUT+1,"out #2");
		configOutput(OUT_OUTPUT+2,"out #3");
		configOutput(OUT_OUTPUT+3,"out #4");

		playBuffer[0][0].resize(0);
		playBuffer[1][0].resize(0);
		playBuffer[2][0].resize(0); 
		playBuffer[3][0].resize(0);
		playBuffer[0][1].resize(0);
		playBuffer[1][1].resize(0);
		playBuffer[2][1].resize(0); 
		playBuffer[3][1].resize(0);
	}

	void onReset() override {
		interpolationMode = HERMITE_INTERP;
		antiAlias = 1;
		outsMode = 0;
		for (int i = 0; i < 4; i++) {
			clearSlot(i);
			play[i] = false;
			choking[i] = false;
			fading[i] = false;
		}
	}

	void onSampleRateChange() override {
		for (int i = 0; i < 4; i++) {
			if (fileLoaded[i])
				sampleCoeff[i] = sampleRate[i] / (APP->engine->getSampleRate());
		}
		fadeDecrement = 1000 / (APP->engine->getSampleRate());
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Interpolation", json_integer(interpolationMode));
		json_object_set_new(rootJ, "AntiAlias", json_integer(antiAlias));
		json_object_set_new(rootJ, "OutsMode", json_integer(outsMode));
		json_object_set_new(rootJ, "Slot1", json_string(storedPath[0].c_str()));
		json_object_set_new(rootJ, "Slot2", json_string(storedPath[1].c_str()));
		json_object_set_new(rootJ, "Slot3", json_string(storedPath[2].c_str()));
		json_object_set_new(rootJ, "Slot4", json_string(storedPath[3].c_str()));
		json_object_set_new(rootJ, "UserFolder", json_string(userFolderName.c_str()));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t* interpolationJ = json_object_get(rootJ, "Interpolation");
		if (interpolationJ)
			interpolationMode = json_integer_value(interpolationJ);

		json_t* antiAliasJ = json_object_get(rootJ, "AntiAlias");
		if (antiAliasJ)
			antiAlias = json_integer_value(antiAliasJ);

		json_t* outsModeJ = json_object_get(rootJ, "OutsMode");
		if (outsModeJ)
			outsMode = json_integer_value(outsModeJ);

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
		json_t *slot3J = json_object_get(rootJ, "Slot3");
		if (slot3J) {
			storedPath[2] = json_string_value(slot3J);
			loadSample(storedPath[2], 2);
		}
		json_t *slot4J = json_object_get(rootJ, "Slot4");
		if (slot4J) {
			storedPath[3] = json_string_value(slot4J);
			loadSample(storedPath[3], 3);
		}
		json_t *userFolderNameJ = json_object_get(rootJ, "UserFolder");
		if (userFolderNameJ) {
			userFolderName = json_string_value(userFolderNameJ);
			folderSelection(userFolderName);
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

	/*
	double hermiteInterpol(double x0, double x1, double x2, double x3, double t)	{
		double c0 = x1;
		double c1 = .5F * (x2 - x0);
		double c2 = x0 - (2.5F * x1) + (2 * x2) - (.5F * x3);
		double c3 = (.5F * (x3 - x0)) + (1.5F * (x1 - x2));
		return (((((c3 * t) + c2) * t) + c1) * t) + c0;
	}
	*/ 

	void folderSelection(std::string path) {
		DIR* rep = NULL;
		struct dirent* dirp = NULL;
			char* pathDup = strdup(path.c_str());
			std::string dir = pathDup;
		rep = opendir(dir.c_str());
		browserFileName.clear();
		browserFileDisplay.clear();
		while ((dirp = readdir(rep)) != NULL) {
			std::string name = dirp->d_name;
			std::size_t found = name.find(".wav",name.length()-5);
			if (found==std::string::npos)
				found = name.find(".WAV",name.length()-5);

			if (found!=std::string::npos) {
				browserFileName.push_back(name);
				browserFileDisplay.push_back(name.substr(0, name.length()-4));
			}
		}
		sort(browserFileName.begin(), browserFileName.end());
		sort(browserFileDisplay.begin(), browserFileDisplay.end());
		closedir(rep);
	};

	void loadSample(std::string path, int slot) {
		unsigned int c;
		unsigned int sr;
		drwav_uint64 tsc;
		float* pSampleData;
		pSampleData = drwav_open_and_read_file_f32(path.c_str(), &c, &sr, &tsc);

		if (pSampleData != NULL) {
			//channels[slot] = c;
			sampleRate[slot] = sr * 2;
			calcBiquadLpf(20000.0, sampleRate[slot], 1);
			playBuffer[slot][0].clear();
			playBuffer[slot][1].clear();
			for (unsigned int i = 0; i < tsc; i = i + c) {
				playBuffer[slot][0].push_back(pSampleData[i]);
				playBuffer[slot][0].push_back(0);
			}
			totalSampleC[slot] = playBuffer[slot][0].size();
			totalSamples[slot] = totalSampleC[slot]-1;
			drwav_free(pSampleData);

			for (unsigned int i = 1; i < totalSamples[slot]; i = i + 2)
				playBuffer[slot][0][i] = playBuffer[slot][0][i-1] * .5f + playBuffer[slot][0][i+1] * .5f;

			playBuffer[slot][0][totalSamples[slot]] = playBuffer[slot][0][totalSamples[slot]-1] * .5f; // halve the last sample

			for (unsigned int i = 0; i < totalSampleC[slot]; i++)
				playBuffer[slot][1].push_back(biquadLpf(playBuffer[slot][0][i]));

			sampleCoeff[slot] = sampleRate[slot] / (APP->engine->getSampleRate());

			char* pathDup = strdup(path.c_str());
			fileDescription[slot] = basename(pathDup);
			fileDisplay[slot] = fileDescription[slot].substr(0,5);
			free(pathDup);

			storedPath[slot] = path;

			fileLoaded[slot] = true;

		} else {
			fileLoaded[slot] = false;
			storedPath[slot] = "";
			fileDescription[slot] = "--none--";
			fileDisplay[slot] = "-----";
		}
	};
	
	void clearSlot(int slot) {
		storedPath[slot] = "";
		fileDescription[slot] = "--none--";
		fileDisplay[slot] = "-----";
		fileLoaded[slot] = false;
		playBuffer[slot][0].clear();
		playBuffer[slot][1].clear();
		totalSampleC[slot] = 0;
	}
	
	void process(const ProcessArgs &args) override {
		summedOutput = 0;
		for (int i = 0; i < 4; i++){

			trigValue[i] = inputs[TRIG_INPUT+i].getVoltage();

			if (trigValue[i] >= 1 && prevTrigValue[i] < 1){
				if (play[i]) {
					fading[i] = true;
					fadingValue[i] = 1.f;
					fadedPosition[i] = samplePos[i];
				}
				play[i] = true;
				samplePos[i] = 0;
				currSampleWeight[i] = sampleCoeff[i];
				prevSamplePos[i] = 0;
				prevSampleWeight[i] = 0;
				//debugDisplay = "X";
				if (inputs[ACC_INPUT+i].getVoltage() > 1)
					level[i] = params[ACCVOL_PARAM+i].getValue() + (inputs[ACCVOL_INPUT+i].getVoltage() * params[ACCVOLATNV_PARAM+i].getValue() * 0.1);
				else
					level[i] = params[TRIGVOL_PARAM+i].getValue() + (inputs[TRIGVOL_INPUT+i].getVoltage() * params[TRIGVOLATNV_PARAM+i].getValue() * 0.1);

				if (level[i] > 2)
					level[i] = 2;
				else if (level[i] < 0)
					level[i] = 0;
				
				if (i < 3 && params[CHOKE_SWITCH+i].getValue()) {
					choking[i+1] = true;
					chokeValue[i+1] = 1.f;
				}
			}
			prevTrigValue[i] = trigValue[i];
			currentOutput = 0;

			//if (fileLoaded[i] && play[i] && floor(samplePos[i]) < totalSampleC[i] && floor(samplePos[i]) >= 0) {
			if (fileLoaded[i] && play[i] && floor(samplePos[i]) < totalSampleC[i]) {
				switch (interpolationMode) {
					case NO_INTERP:
						currentOutput = 5 * level[i] * float(playBuffer[i][antiAlias][floor(samplePos[i])]);
					break;

					case LINEAR1_INTERP:
						if (currSampleWeight[i] == 0) {
							currentOutput = 5 * level[i] * float(playBuffer[i][antiAlias][floor(samplePos[i])]);
						} else {
							currentOutput = 5 * level[i] * float(
											(playBuffer[i][antiAlias][floor(samplePos[i])] * (1-currSampleWeight[i])) +
											(playBuffer[i][antiAlias][floor(samplePos[i])+1] * currSampleWeight[i])
											);
						}
					break;

					case LINEAR2_INTERP:
						if (currSampleWeight[i] == 0) {
							currentOutput = 5 * level[i] * float(playBuffer[i][antiAlias][floor(samplePos[i])]);
						} else {
							currentOutput = 5 * level[i] * float(
											(
												(playBuffer[i][antiAlias][floor(prevSamplePos[i])] * (1-prevSampleWeight[i])) +
												(playBuffer[i][antiAlias][floor(prevSamplePos[i])+1] * prevSampleWeight[i]) +
												(playBuffer[i][antiAlias][floor(samplePos[i])] * (1-currSampleWeight[i])) +
												(playBuffer[i][antiAlias][floor(samplePos[i])+1] * currSampleWeight[i])
											) / 2);
						}
					break;

					case HERMITE_INTERP:
						if (currSampleWeight[i] == 0) {
							currentOutput = 5 * level[i] * float(playBuffer[i][antiAlias][floor(samplePos[i])]);
						} else {
							if (floor(samplePos[i]) > 1 && floor(samplePos[i]) < totalSamples[i] - 1) {
								/*
								resampled[i] = hermiteInterpol(playBuffer[i][antiAlias][floor(samplePos[i])-1],
																playBuffer[i][antiAlias][floor(samplePos[i])],
																playBuffer[i][antiAlias][floor(samplePos[i])+1],
																playBuffer[i][antiAlias][floor(samplePos[i])+2],
																currSampleWeight[i]);
								*/
								// below is translation of the above function
								double a1 = .5F * (playBuffer[i][antiAlias][floor(samplePos[i])+1] - playBuffer[i][antiAlias][floor(samplePos[i])-1]);
								double a2 = playBuffer[i][antiAlias][floor(samplePos[i])-1] - (2.5F * playBuffer[i][antiAlias][floor(samplePos[i])]) + (2 * playBuffer[i][antiAlias][floor(samplePos[i])+1]) - (.5F * playBuffer[i][antiAlias][floor(samplePos[i])+2]);
								double a3 = (.5F * (playBuffer[i][antiAlias][floor(samplePos[i])+2] - playBuffer[i][antiAlias][floor(samplePos[i])-1])) + (1.5F * (playBuffer[i][antiAlias][floor(samplePos[i])] - playBuffer[i][antiAlias][floor(samplePos[i])+1]));
								currentOutput = 5 * level[i] * float(
									(((((a3 * currSampleWeight[i]) + a2) * currSampleWeight[i]) + a1) * currSampleWeight[i]) + playBuffer[i][antiAlias][floor(samplePos[i])]
								);
							} else {
								currentOutput = 5 * level[i] * float(playBuffer[i][antiAlias][floor(samplePos[i])]);
							}
						}
					break;
				}

				if (i > 0 && choking[i]) {
					if (chokeValue[i] < 0.0) {
						choking[i] = false;
						play[i] = false;
						currentOutput = 0;
					} else {
						currentOutput *= chokeValue[i];
						chokeValue[i] -= fadeDecrement;
					}
				}

				prevSamplePos[i] = samplePos[i];
				if (inputs[SPEED_INPUT+i].isConnected()) {
					currentSpeed = double(params[SPEED_PARAM+i].getValue() + (inputs[SPEED_INPUT+i].getVoltage() * params[SPEEDATNV_PARAM+i].getValue() * 0.1));
					if (currentSpeed > 2)
						currentSpeed = 2;
					else {
						if (currentSpeed < 0.01)
							currentSpeed = 0.01;
					}
				} else {
					currentSpeed = double(params[SPEED_PARAM+i].getValue());
				}

				samplePos[i] += sampleCoeff[i]*currentSpeed;

				if (interpolationMode > NO_INTERP) {
					prevSampleWeight[i] = currSampleWeight[i];
					currSampleWeight[i] = samplePos[i] - floor(samplePos[i]);
				}

				if (fading[i]) { 					// fades previous samples if retrigged when already playing
					if (fadingValue[i] > 0) {
						fadingValue[i] -= fadeDecrement;
						currentOutput += (playBuffer[i][antiAlias][floor(fadedPosition[i])] * fadingValue[i] * level[i] * 5);
						fadedPosition[i] += sampleCoeff[i]*currentSpeed;
						if (fadedPosition[i] > totalSamples[i])
							fading[i] = false;
					} else
						fading[i] = false;
				}
			} else {
				choking[i] = false;
				play[i] = false;
				fading[i] = false;
			}

			/*
			if (params[LIMIT_SWITCH+i].getValue()) {
				if (currentOutput > 5)
					currentOutput = 5;
				else if (currentOutput < -5)
					currentOutput = -5;
			}
			*/

			switch (outsMode) {
				case NORMALLED_OUTS:
					summedOutput += currentOutput;

					if (params[LIMIT_SWITCH+i].getValue()) {
						if (summedOutput > 5)
							summedOutput = 5;
						else if (summedOutput < -5)
							summedOutput = -5;
					}

					if (outputs[OUT_OUTPUT+i].isConnected()) {
						outputs[OUT_OUTPUT+i].setVoltage(summedOutput);
						summedOutput = 0;
					}
				break;

				case SOLO_OUTS:
					if (outputs[OUT_OUTPUT+i].isConnected()) {
						
						if (params[LIMIT_SWITCH+i].getValue()) {
							if (currentOutput > 5)
								currentOutput = 5;
							else if (currentOutput < -5)
								currentOutput = -5;
						}
						outputs[OUT_OUTPUT+i].setVoltage(currentOutput);

					}
				break;

				case UNCONNECTED_ON_4:
					if (i == 3) {
						summedOutput += currentOutput;
						if (params[LIMIT_SWITCH+i].getValue()) {
							if (summedOutput > 5)
								summedOutput = 5;
							else if (summedOutput < -5)
								summedOutput = -5;
						}
						outputs[OUT_OUTPUT+i].setVoltage(summedOutput);
					} else if (outputs[OUT_OUTPUT+i].isConnected()) {
						if (params[LIMIT_SWITCH+i].getValue()) {
							if (currentOutput > 5)
								currentOutput = 5;
							else if (currentOutput < -5)
								currentOutput = -5;
						}
						outputs[OUT_OUTPUT+i].setVoltage(currentOutput);
					} else {
						summedOutput += currentOutput;
						if (params[LIMIT_SWITCH+i].getValue()) {
							if (summedOutput > 5)
								summedOutput = 5;
							else if (summedOutput < -5)
								summedOutput = -5;
						}
					}
				break;
			}

			if (!inputs[TRIG_INPUT+i].isConnected())
				play[i] = false;
		}
	}
};

struct DrumPlayerPlusOpenFolder : MenuItem {
	DrumPlayerPlus *rm ;
	void onAction(const event::Action &e) override {
		const char* prevFolder = rm->userFolderName.c_str();
		char *path = osdialog_file(OSDIALOG_OPEN_DIR, prevFolder, NULL, NULL);
		if (path) {
			rm->userFolderName = std::string(path);
			DIR* rep = NULL;
			struct dirent* dirp = NULL;
				char* pathDup = path;
				std::string dir = pathDup;

			rep = opendir(dir.c_str());
			rm->browserFileName.clear();
			rm->browserFileDisplay.clear();
			while ((dirp = readdir(rep)) != NULL) {
				std::string name = dirp->d_name;

				std::size_t found = name.find(".wav",name.length()-5);
				if (found==std::string::npos)
					found = name.find(".WAV",name.length()-5);

				if (found!=std::string::npos) {
					rm->browserFileName.push_back(name);
					rm->browserFileDisplay.push_back(name.substr(0, name.length()-4));
				}
			}
			sort(rm->browserFileName.begin(), rm->browserFileName.end());
			sort(rm->browserFileDisplay.begin(), rm->browserFileDisplay.end());
			closedir(rep);
		}
		free(path);
	}
};

struct DrumPlayerPlusItem1 : MenuItem {
	DrumPlayerPlus *rm ;
	void onAction(const event::Action &e) override {
		static const char FILE_FILTERS[] = "Wave (.wav):wav,WAV;All files (*.*):*.*";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
		int i = 0;
		rm->fileLoaded[i] = false;
		if (path) {
			rm->loadSample(path, i);
			rm->storedPath[i] = std::string(path);
			free(path);
		} else if (rm->storedPath[i] == "")
			rm->fileLoaded[i] = false;
		else
			rm->fileLoaded[i] = true;
	}
};

struct DrumPlayerPlusItem2 : MenuItem {
	DrumPlayerPlus *rm ;
	void onAction(const event::Action &e) override {
		static const char FILE_FILTERS[] = "Wave (.wav):wav,WAV;All files (*.*):*.*";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
		int i = 1;
		rm->fileLoaded[i] = false;
		if (path) {
			rm->loadSample(path, i);
			rm->storedPath[i] = std::string(path);
			free(path);
		} else if (rm->storedPath[i] == "")
			rm->fileLoaded[i] = false;
		else
			rm->fileLoaded[i] = true;
	}
};

struct DrumPlayerPlusItem3 : MenuItem {
	DrumPlayerPlus *rm ;
	void onAction(const event::Action &e) override {
		static const char FILE_FILTERS[] = "Wave (.wav):wav,WAV;All files (*.*):*.*";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
		int i = 2;
		rm->fileLoaded[i] = false;
		if (path) {
			rm->loadSample(path, i);
			rm->storedPath[i] = std::string(path);
			free(path);
		} else if (rm->storedPath[i] == "")
			rm->fileLoaded[i] = false;
		else
			rm->fileLoaded[i] = true;
	}
};

struct DrumPlayerPlusItem4 : MenuItem {
	DrumPlayerPlus *rm ;
	void onAction(const event::Action &e) override {
		static const char FILE_FILTERS[] = "Wave (.wav):wav,WAV;All files (*.*):*.*";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
		int i = 3;
		rm->fileLoaded[i] = false;
		if (path) {
			rm->loadSample(path, i);
			rm->storedPath[i] = std::string(path);
			free(path);
		} else if (rm->storedPath[i] == "")
			rm->fileLoaded[i] = false;
		else
			rm->fileLoaded[i] = true;
	}
};


struct dppSlot1Display : TransparentWidget {
	DrumPlayerPlus *module;
	int frame = 0;

	dppSlot1Display() {

	}

	struct ClearSlot1Item : MenuItem {
		DrumPlayerPlus *module;
		void onAction(const event::Action &e) override {
			module->clearSlot(0);
		}
	};

	void onButton(const event::Button &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void createContextMenu() {
		DrumPlayerPlus *module = dynamic_cast<DrumPlayerPlus *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			DrumPlayerPlusItem1 *rootDirItem = new DrumPlayerPlusItem1;
				rootDirItem->text = "Load Sample Slot #1";
				rootDirItem->rm = module;
				menu->addChild(rootDirItem);

			if (module->browserFileName.size() > 0) {
				menu->addChild(createSubmenuItem("Folder Browser", "",
					[=](Menu* menu) {
						for (unsigned int i = 0; i < module->browserFileName.size(); i++) {
							menu->addChild(createMenuItem(module->browserFileDisplay[i], "", [=]() {module->loadSample(module->userFolderName+"\\"+ module->browserFileName[i], 0);}));
						}
					}
				));
			}
			if (module->fileLoaded[0]) {
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription[0]));
				menu->addChild(construct<ClearSlot1Item>(&MenuItem::rightText, "Clear", &ClearSlot1Item::module, module));
			}
		}
	}
};

struct dppSlot2Display : TransparentWidget {
	DrumPlayerPlus *module;
	int frame = 0;

	dppSlot2Display() {

	}

	struct ClearSlot2Item : MenuItem {
		DrumPlayerPlus *module;
		void onAction(const event::Action &e) override {
			module->clearSlot(1);
		}
	};

	void onButton(const event::Button &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void createContextMenu() {
		DrumPlayerPlus *module = dynamic_cast<DrumPlayerPlus *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			DrumPlayerPlusItem2 *rootDirItem = new DrumPlayerPlusItem2;
				rootDirItem->text = "Load Sample Slot #2";
				rootDirItem->rm = module;
				menu->addChild(rootDirItem);

			if (module->browserFileName.size() > 0) {
				menu->addChild(createSubmenuItem("Folder Browser", "",
					[=](Menu* menu) {
						for (unsigned int i = 0; i < module->browserFileName.size(); i++) {
							menu->addChild(createMenuItem(module->browserFileDisplay[i], "", [=]() {module->loadSample(module->userFolderName+"\\"+ module->browserFileName[i], 1);}));
						}
					}
				));
			}
			if (module->fileLoaded[1]) {
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription[1]));
				menu->addChild(construct<ClearSlot2Item>(&MenuItem::rightText, "Clear", &ClearSlot2Item::module, module));
			}
		}
	}
};

struct dppSlot3Display : TransparentWidget {
	DrumPlayerPlus *module;
	int frame = 0;

	dppSlot3Display() {

	}

	struct ClearSlot3Item : MenuItem {
		DrumPlayerPlus *module;
		void onAction(const event::Action &e) override {
			module->clearSlot(2);
		}
	};

	void onButton(const event::Button &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void createContextMenu() {
		DrumPlayerPlus *module = dynamic_cast<DrumPlayerPlus *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			DrumPlayerPlusItem3 *rootDirItem = new DrumPlayerPlusItem3;
				rootDirItem->text = "Load Sample Slot #3";
				rootDirItem->rm = module;
				menu->addChild(rootDirItem);

			if (module->browserFileName.size() > 0) {
				menu->addChild(createSubmenuItem("Folder Browser", "",
					[=](Menu* menu) {
						for (unsigned int i = 0; i < module->browserFileName.size(); i++) {
							menu->addChild(createMenuItem(module->browserFileDisplay[i], "", [=]() {module->loadSample(module->userFolderName+"\\"+ module->browserFileName[i], 2);}));
						}
					}
				));
			}
			if (module->fileLoaded[2]) {
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription[2]));
				menu->addChild(construct<ClearSlot3Item>(&MenuItem::rightText, "Clear", &ClearSlot3Item::module, module));
			}
		}
	}
};

struct dppSlot4Display : TransparentWidget {
	DrumPlayerPlus *module;
	int frame = 0;

	dppSlot4Display() {

	}

	struct ClearSlot4Item : MenuItem {
		DrumPlayerPlus *module;
		void onAction(const event::Action &e) override {
			module->clearSlot(3);
		}
	};

	void onButton(const event::Button &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void createContextMenu() {
		DrumPlayerPlus *module = dynamic_cast<DrumPlayerPlus *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			DrumPlayerPlusItem4 *rootDirItem = new DrumPlayerPlusItem4;
				rootDirItem->text = "Load Sample Slot #4";
				rootDirItem->rm = module;
				menu->addChild(rootDirItem);

			if (module->browserFileName.size() > 0) {
				menu->addChild(createSubmenuItem("Folder Browser", "",
					[=](Menu* menu) {
						for (unsigned int i = 0; i < module->browserFileName.size(); i++) {
							menu->addChild(createMenuItem(module->browserFileDisplay[i], "", [=]() {module->loadSample(module->userFolderName+"\\"+ module->browserFileName[i], 3);}));
						}
					}
				));
			}
			if (module->fileLoaded[3]) {
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription[3]));
				menu->addChild(construct<ClearSlot4Item>(&MenuItem::rightText, "Clear", &ClearSlot4Item::module, module));
			}
		}
	}
};

struct DrumPlayerPlusDisplay : TransparentWidget {
	DrumPlayerPlus *module;
	int frame = 0;

	DrumPlayerPlusDisplay() {

	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/DSEG7ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgFillColor(args.vg, nvgRGBA(0xdd, 0x33, 0x33, 0xff)); 
				nvgTextBox(args.vg, 6, 0,120, module->fileDisplay[0].c_str(), NULL);
				nvgTextBox(args.vg, 75, 0,120, module->fileDisplay[1].c_str(), NULL);
				nvgTextBox(args.vg, 144, 0,120, module->fileDisplay[2].c_str(), NULL);
				nvgTextBox(args.vg, 214, 0,120, module->fileDisplay[3].c_str(), NULL);
				//nvgTextBox(args.vg, 2, -20,120, module->debugDisplay.c_str(), NULL);
			}
		}
		Widget::drawLayer(args, layer);
	}
};


struct DrumPlayerPlusWidget : ModuleWidget {
	DrumPlayerPlusWidget(DrumPlayerPlus *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DrumPlayerPlus.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	  

		{
			dppSlot1Display *display1 = new dppSlot1Display();
			display1->box.pos = Vec(12, 33);
			display1->box.size = Vec(52, 14);
			display1->module = module;
			addChild(display1);
		}

		{
			dppSlot2Display *display2 = new dppSlot2Display();
			display2->box.pos = Vec(82, 33);
			display2->box.size = Vec(52, 14);
			display2->module = module;
			addChild(display2);
		}

		{
			dppSlot3Display *display3 = new dppSlot3Display();
			display3->box.pos = Vec(151, 33);
			display3->box.size = Vec(52, 14);
			display3->module = module;
			addChild(display3);
		}

		{
			dppSlot4Display *display4 = new dppSlot4Display();
			display4->box.pos = Vec(221, 33);
			display4->box.size = Vec(52, 14);
			display4->module = module;
			addChild(display4);
		}
	    
		{
			DrumPlayerPlusDisplay *display = new DrumPlayerPlusDisplay();
			display->box.pos = Vec(13, 45);
			display->box.size = Vec(270, 100);
			display->module = module;
			addChild(display);
		}


		float xDelta = 23.5;

		for (int i = 0; i < 4; i++) {
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.9+(xDelta*i), 21)), module, DrumPlayerPlus::TRIG_INPUT+i));
			addParam(createParamCentered<Trimpot>(mm2px(Vec(17.9+(xDelta*i), 28.5)), module, DrumPlayerPlus::TRIGVOLATNV_PARAM+i));
			addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.9+(xDelta*i), 32)), module, DrumPlayerPlus::TRIGVOL_PARAM+i));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.9+(xDelta*i), 37)), module, DrumPlayerPlus::TRIGVOL_INPUT+i));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.9+(xDelta*i), 47)), module, DrumPlayerPlus::ACC_INPUT+i));
			addParam(createParamCentered<Trimpot>(mm2px(Vec(17.9+(xDelta*i), 54.5)), module, DrumPlayerPlus::ACCVOLATNV_PARAM+i));
			addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.9+(xDelta*i), 58)), module, DrumPlayerPlus::ACCVOL_PARAM+i));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.9+(xDelta*i), 63)), module, DrumPlayerPlus::ACCVOL_INPUT+i));

			addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(12.9+(xDelta*i), 78.5)), module, DrumPlayerPlus::SPEED_PARAM+i));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.9+(xDelta*i), 88)), module, DrumPlayerPlus::SPEED_INPUT+i));
			addParam(createParamCentered<Trimpot>(mm2px(Vec(17.9+(xDelta*i), 88)), module, DrumPlayerPlus::SPEEDATNV_PARAM+i));

			if (i<3) {
				addParam(createParamCentered<CKSS>(mm2px(Vec(7.9+(xDelta*i), 103.9)), module, DrumPlayerPlus::LIMIT_SWITCH+i));
				addParam(createParamCentered<CKSS>(mm2px(Vec(17.9+(xDelta*i), 103.9)), module, DrumPlayerPlus::CHOKE_SWITCH+i));
			} else {
				addParam(createParamCentered<CKSS>(mm2px(Vec(5+7.9+(xDelta*i), 103.9)), module, DrumPlayerPlus::LIMIT_SWITCH+i));
			}

			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(17.9+(xDelta*i), 117)), module, DrumPlayerPlus::OUT_OUTPUT+i));
		}
	}

	struct ClearSlotsItem : MenuItem {
		DrumPlayerPlus *module;
		void onAction(const event::Action &e) override {
			for (int i = 0; i < 4; i++)
				module->clearSlot(i);
		}
	};

	struct ClearSlot1Item : MenuItem {
		DrumPlayerPlus *module;
		void onAction(const event::Action &e) override {
			module->clearSlot(0);
		}
	};

	struct ClearSlot2Item : MenuItem {
		DrumPlayerPlus *module;
		void onAction(const event::Action &e) override {
			module->clearSlot(1);
		}
	};

	struct ClearSlot3Item : MenuItem {
		DrumPlayerPlus *module;
		void onAction(const event::Action &e) override {
			module->clearSlot(2);
		}
	};

	struct ClearSlot4Item : MenuItem {
		DrumPlayerPlus *module;
		void onAction(const event::Action &e) override {
			module->clearSlot(3);
		}
	};

	struct RefreshUserFolderItem : MenuItem {
		DrumPlayerPlus *module;
		void onAction(const event::Action &e) override {
			module->folderSelection(module->userFolderName);
		}
	};

	void appendContextMenu(Menu *menu) override {
	   	DrumPlayerPlus *module = dynamic_cast<DrumPlayerPlus*>(this->module);
			assert(module);
			menu->addChild(new MenuSeparator());

		DrumPlayerPlusItem1 *rootDirItem1 = new DrumPlayerPlusItem1;
			menu->addChild(createMenuLabel("Sample Slots"));
			rootDirItem1->text = "1: " + module->fileDescription[0];
			rootDirItem1->rm = module;
			menu->addChild(rootDirItem1);

		menu->addChild(construct<ClearSlot1Item>(&MenuItem::rightText, "Clear #1", &ClearSlot1Item::module, module));

		DrumPlayerPlusItem2 *rootDirItem2 = new DrumPlayerPlusItem2;
			rootDirItem2->text = "2: " + module->fileDescription[1];
			rootDirItem2->rm = module;
			menu->addChild(rootDirItem2);

		menu->addChild(construct<ClearSlot2Item>(&MenuItem::rightText, "Clear #2", &ClearSlot2Item::module, module));
		
		DrumPlayerPlusItem3 *rootDirItem3 = new DrumPlayerPlusItem3;
			rootDirItem3->text = "3: " + module->fileDescription[2];
			rootDirItem3->rm = module;
			menu->addChild(rootDirItem3);

		menu->addChild(construct<ClearSlot3Item>(&MenuItem::rightText, "Clear #3", &ClearSlot3Item::module, module));
		
		DrumPlayerPlusItem4 *rootDirItem4 = new DrumPlayerPlusItem4;
			rootDirItem4->text = "4: " + module->fileDescription[3];
			rootDirItem4->rm = module;
			menu->addChild(rootDirItem4);

		menu->addChild(construct<ClearSlot4Item>(&MenuItem::rightText, "Clear #4", &ClearSlot4Item::module, module));
			
		menu->addChild(new MenuSeparator());
		menu->addChild(construct<ClearSlotsItem>(&MenuItem::text, "Clear ALL slots", &ClearSlotsItem::module, module));
		menu->addChild(new MenuSeparator());

		DrumPlayerPlusOpenFolder *rootFolder = new DrumPlayerPlusOpenFolder;
			rootFolder->text = "Select Samples Folder";
			rootFolder->rm = module;
			menu->addChild(rootFolder);
		if (module->userFolderName != "") {
			menu->addChild(createMenuLabel(module->userFolderName));
			menu->addChild(construct<RefreshUserFolderItem>(&MenuItem::rightText, "Refresh", &RefreshUserFolderItem::module, module));
		}
		menu->addChild(new MenuSeparator());

		menu->addChild(createMenuLabel("Interpolation"));
		struct ModeItem : MenuItem {
			DrumPlayerPlus* module;
			int interpolationMode;
			void onAction(const event::Action& e) override {
				module->interpolationMode = interpolationMode;
			}
		};
		std::string modeNames[4] = {"None", "Linear 1", "Linear 2", "Hermite"};
		for (int i = 0; i < 4; i++) {
			ModeItem* modeItem = createMenuItem<ModeItem>(modeNames[i]);
			modeItem->rightText = CHECKMARK(module->interpolationMode == i);
			modeItem->module = module;
			modeItem->interpolationMode = i;
			menu->addChild(modeItem);
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Anti-aliasing filter", "", &module->antiAlias));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Outs mode"));
		struct OutsItem : MenuItem {
			DrumPlayerPlus* module;
			int outsMode;
			void onAction(const event::Action& e) override {
				module->outsMode = outsMode;
			}
		};
		std::string outsNames[3] = {"Normalled", "Solo", "Unconnected on out #4"};
		for (int i = 0; i < 3; i++) {
			OutsItem* outsItem = createMenuItem<OutsItem>(outsNames[i]);
			outsItem->rightText = CHECKMARK(module->outsMode == i);
			outsItem->module = module;
			outsItem->outsMode = i;
			menu->addChild(outsItem);
		}
	}
};

Model *modelDrumPlayerPlus = createModel<DrumPlayerPlus, DrumPlayerPlusWidget>("DrumPlayerPlus");
