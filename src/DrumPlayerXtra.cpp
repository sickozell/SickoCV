#define NO_INTERP 0
#define LINEAR1_INTERP 1
#define LINEAR2_INTERP 2
#define HERMITE_INTERP 3
#define NORMALLED_OUTS 0
#define SOLO_OUTS 1
#define UNCONNECTED_ON_4 2

#include "plugin.hpp"
#include "osdialog.h"
#if defined(METAMODULE)
#include "async_filebrowser.hh"
#endif
//#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <vector>
#include "cmath"
#include <dirent.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

struct DrumPlayerXtra : Module {
	enum ParamIds {
		ENUMS(TRIGVOL_PARAM,4),
		ENUMS(TRIGVOLATNV_PARAM,4),
		ENUMS(ACCVOL_PARAM,4),
		ENUMS(ACCVOLATNV_PARAM,4),
		ENUMS(SPEED_PARAM,4),
		ENUMS(SPEEDATNV_PARAM,4),
		ENUMS(LIMIT_PARAMS,4),
		ENUMS(CHOKE_PARAMS,3),
		ENUMS(PREVSAMPLE_PARAM,4),
		ENUMS(NEXTSAMPLE_PARAM,4),
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
		ENUMS(CHOKE_LIGHT,3),
		ENUMS(LIMIT_LIGHT,4),
		NUM_LIGHTS
	};
  
	unsigned int channels[4];
	unsigned int sampleRate[4];
	drwav_uint64 totalSampleC[4] = {0, 0, 0, 0};
	drwav_uint64 totalSamples[4] = {0, 0, 0, 0};

	const unsigned int minSamplesToLoad = 9;

	vector<float> playBuffer[4][2];
	vector<double> displayBuff[4];

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
	std::string fileDisplay[4] = {"-------","-------","-------","-------"};
	std::string scrollDisplay[4] = {"-------","-------","-------","-------"};
	std::string currFileDisplay[4] = {"-------","-------","-------","-------"};
	std::string userFolder = "";
	bool rootFound = false;
	bool fileFound[4] = {false, false, false, false};

	std::string currentFolder[4] = {"", "", "", ""};
	vector <std::string> currentFolderV[4];
	int currentFile[4] = {0,0,0,0};

	std::string tempDir = "";
	vector<vector<std::string>> folderTreeData;
	vector<vector<std::string>> folderTreeDisplay;
	vector<std::string> tempTreeData;
	vector<std::string> tempTreeDisplay;

	bool choke[4] = {false, false, false, false};
	int limit[4];

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
	bool disableNav = false;
	int scrolling = 0;
	int displayTrig = 1;
	int lightBox = true;
	int colorBox[4] = {0,1,2,3};
	int colorBoxR[4] = {0,255,255,0};
	int colorBoxG[4] = {0,0,255,255};
	int colorBoxB[4] = {255,0,0,0};
	int lightTime[4] = {1,1,1,1};

	bool sampleInPatch = true;
	bool loadFromPatch[4] = {false, false, false, false};
	bool restoreLoadFromPatch[4] = {false, false, false, false};

	int zoom[4] = {0,0,0,0};
	float displayCoeff[4];

	bool slotTriggered[4] = {false, false, false, false};

	bool nextSample[4] = {false, false, false, false};
	bool prevNextSample[4] = {false, false, false, false};
	bool prevSample[4] = {false, false, false, false};
	bool prevPrevSample[4] = {false, false, false, false};

	bool uiTrig[4] = {false, false, false, false};

	//std::string debugDisplay = "X";
	//int debugCount = 0;

	double a0, a1, a2, b1, b2, z1, z2;

	DrumPlayerXtra() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configSwitch(PREVSAMPLE_PARAM, 0.f, 1.f, 0.f, "Previous Sample #1");
		configSwitch(PREVSAMPLE_PARAM+1, 0.f, 1.f, 0.f, "Previous Sample #2");
		configSwitch(PREVSAMPLE_PARAM+2, 0.f, 1.f, 0.f, "Previous Sample #3");
		configSwitch(PREVSAMPLE_PARAM+3, 0.f, 1.f, 0.f, "Previous Sample #4");
		configSwitch(NEXTSAMPLE_PARAM, 0.f, 1.f, 0.f, "Next Sample #1");
		configSwitch(NEXTSAMPLE_PARAM+1, 0.f, 1.f, 0.f, "Next Sample #2");
		configSwitch(NEXTSAMPLE_PARAM+2, 0.f, 1.f, 0.f, "Next Sample #3");
		configSwitch(NEXTSAMPLE_PARAM+3, 0.f, 1.f, 0.f, "Next Sample #4");
		configInput(TRIG_INPUT,"Trig #1");
		configInput(TRIG_INPUT+1,"Trig #2");
		configInput(TRIG_INPUT+2,"Trig #3");
		configInput(TRIG_INPUT+3,"Trig #4");
		configInput(TRIGVOL_INPUT,"Stand.Lev. #1");
		configInput(TRIGVOL_INPUT+1,"Stand.Lev. #2");
		configInput(TRIGVOL_INPUT+2,"Stand.Lev. #3");
		configInput(TRIGVOL_INPUT+3,"Stand.Lev. #4");
		configParam(TRIGVOL_PARAM, 0.f, 2.0f, 1.0f, "Standard Level #1", "%", 0, 100);
		configParam(TRIGVOL_PARAM+1, 0.f, 2.0f, 1.0f, "Standard Level #2", "%", 0, 100);
		configParam(TRIGVOL_PARAM+2, 0.f, 2.0f, 1.0f, "Standard Level #3", "%", 0, 100);
		configParam(TRIGVOL_PARAM+3, 0.f, 2.0f, 1.0f, "Standard Level #4", "%", 0, 100);
		configParam(TRIGVOLATNV_PARAM, -1.f, 1.0f, 0.f, "Stand.Lev.CV #1", "%", 0, 100);
		configParam(TRIGVOLATNV_PARAM+1, -1.f, 1.0f, 0.f, "Stand.Lev.CV #2", "%", 0, 100);
		configParam(TRIGVOLATNV_PARAM+2, -1.f, 1.0f, 0.f, "Stand.Lev.CV #3", "%", 0, 100);
		configParam(TRIGVOLATNV_PARAM+3, -1.f, 1.0f, 0.f, "Stand.Lev.CV #4", "%", 0, 100);
		configInput(ACC_INPUT,"Accent #1");
		configInput(ACC_INPUT+1,"Accent #2");
		configInput(ACC_INPUT+2,"Accent #3");
		configInput(ACC_INPUT+3,"Accent #4");
		configInput(ACCVOL_INPUT,"Accent.Lev. #1");
		configInput(ACCVOL_INPUT+1,"Accent.Lev. #2");
		configInput(ACCVOL_INPUT+2,"Accent.Lev. #3");
		configInput(ACCVOL_INPUT+3,"Accent.Lev. #4");
		configParam(ACCVOL_PARAM, 0.f, 2.0f, 1.0f, "Accent Level #1", "%", 0, 100);
		configParam(ACCVOL_PARAM+1, 0.f, 2.0f, 1.0f, "Accent Level #2", "%", 0, 100);
		configParam(ACCVOL_PARAM+2, 0.f, 2.0f, 1.0f, "Accent Level #3", "%", 0, 100);
		configParam(ACCVOL_PARAM+3, 0.f, 2.0f, 1.0f, "Accent Level #4", "%", 0, 100);
		configParam(ACCVOLATNV_PARAM, -1.f, 1.0f, 0.f, "Accent.Lev.CV #1", "%", 0, 100);
		configParam(ACCVOLATNV_PARAM+1, -1.f, 1.0f, 0.f, "Accent.Lev.CV #2", "%", 0, 100);
		configParam(ACCVOLATNV_PARAM+2, -1.f, 1.0f, 0.f, "Accent.Lev.CV #3", "%", 0, 100);
		configParam(ACCVOLATNV_PARAM+3, -1.f, 1.0f, 0.f, "Accent.Lev.CV #4", "%", 0, 100);
		configParam(SPEEDATNV_PARAM, -1.0f, 1.0f, 0.0f, "Speed CV #1");
		configParam(SPEEDATNV_PARAM+1, -1.0f, 1.0f, 0.0f, "Speed CV #2");
		configParam(SPEEDATNV_PARAM+2, -1.0f, 1.0f, 0.0f, "Speed CV #3");
		configParam(SPEEDATNV_PARAM+3, -1.0f, 1.0f, 0.0f, "Speed CV #4");
		configParam(SPEED_PARAM, 0.01f, 2.0f, 1.0f, "Speed #1", "x", 0, 1);
		configParam(SPEED_PARAM+1, 0.01f, 2.0f, 1.0f, "Speed #2", "x", 0, 1);
		configParam(SPEED_PARAM+2, 0.01f, 2.0f, 1.0f, "Speed #3", "x", 0, 1);
		configParam(SPEED_PARAM+3, 0.01f, 2.0f, 1.0f, "Speed #4", "x", 0, 1);
		configInput(SPEED_INPUT,"Speed CV #1");
		configInput(SPEED_INPUT+1,"Speed CV #2");
		configInput(SPEED_INPUT+2,"Speed CV #3");
		configInput(SPEED_INPUT+3,"Speed CV #4");
		configSwitch(LIMIT_PARAMS, 0.f, 1.f, 0.f, "Limit #1", {"Off", "±5v"});
		configSwitch(LIMIT_PARAMS+1, 0.f, 1.f, 0.f, "Limit #2", {"Off", "±5v"});
		configSwitch(LIMIT_PARAMS+2, 0.f, 1.f, 0.f, "Limit #3", {"Off", "±5v"});
		configSwitch(LIMIT_PARAMS+3, 0.f, 1.f, 0.f, "Limit #4", {"Off", "±5v"});
		configSwitch(CHOKE_PARAMS, 0.f, 1.f, 0.f, "Choke #1", {"Off", "On"});
		configSwitch(CHOKE_PARAMS+1, 0.f, 1.f, 0.f, "Choke #2", {"Off", "On"});
		configSwitch(CHOKE_PARAMS+2, 0.f, 1.f, 0.f, "Choke #3", {"Off", "On"});

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
		displayBuff[0].resize(0);
		displayBuff[1].resize(0);
		displayBuff[2].resize(0);
		displayBuff[3].resize(0);
	}

	void onReset(const ResetEvent &e) override {
		for (int slot = 0; slot < 4; slot++) {
			clearSlot(slot);
			play[slot] = false;
			choking[slot] = false;
			fading[slot] = false;
			colorBox[slot] = slot;
			lightTime[slot] = 1;
			zoom[slot] = 0;
		}
		interpolationMode = HERMITE_INTERP;
		antiAlias = 1;
		outsMode = NORMALLED_OUTS;
		scrolling = 0;
		disableNav = false;
		sampleInPatch = true;
		lightBox = true;
		displayTrig = 1;		
		colorBoxR[0] = 0;
		colorBoxR[1] = 255;
		colorBoxR[2] = 255;
		colorBoxR[3] = 0;
		colorBoxG[0] = 0;
		colorBoxG[1] = 0;
		colorBoxG[2] = 255;
		colorBoxG[3] = 255;
		colorBoxB[0] = 255;
		colorBoxB[1] = 0;
		colorBoxB[2] = 0;
		colorBoxB[3] = 0;
		system::removeRecursively(getPatchStorageDirectory().c_str());
		Module::onReset(e);
	}

	void onSampleRateChange() override {
		for (int i = 0; i < 4; i++) {
			if (fileLoaded[i])
				sampleCoeff[i] = sampleRate[i] / (APP->engine->getSampleRate());
		}
		fadeDecrement = 1000 / (APP->engine->getSampleRate());
	}

	void onAdd(const AddEvent& e) override {
		for (int slot = 0; slot < 4; slot++) {
			if (!fileLoaded[slot] && storedPath[slot] != "") {
				std::string patchFile = system::join(getPatchStorageDirectory(), ("slot"+to_string(slot+1)+".wav").c_str());
				loadFromPatch[slot] = true;
				loadSample(patchFile, slot);
			}
		}
		Module::onAdd(e);
	}

	void onSave(const SaveEvent& e) override {
		system::removeRecursively(getPatchStorageDirectory().c_str());
		if (sampleInPatch) {
			for (int slot = 0; slot < 4; slot++) {
				if (fileLoaded[slot]) {
					std::string patchFile = system::join(createPatchStorageDirectory(), ("slot"+to_string(slot+1)+".wav").c_str());
					saveSample(patchFile, slot);
				}
			}
		}
		Module::onSave(e);
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Interpolation", json_integer(interpolationMode));
		json_object_set_new(rootJ, "AntiAlias", json_integer(antiAlias));
		json_object_set_new(rootJ, "OutsMode", json_integer(outsMode));
		json_object_set_new(rootJ, "DisableNav", json_boolean(disableNav));
		json_object_set_new(rootJ, "Scrolling", json_integer(scrolling));
		json_object_set_new(rootJ, "sampleInPatch", json_boolean(sampleInPatch));
		json_object_set_new(rootJ, "LightBox", json_integer(lightBox));
		json_object_set_new(rootJ, "DisplayTrig", json_integer(displayTrig));
		json_object_set_new(rootJ, "Color1", json_integer(colorBox[0]));
		json_object_set_new(rootJ, "Color1R", json_integer(colorBoxR[0]));
		json_object_set_new(rootJ, "Color1G", json_integer(colorBoxG[0]));
		json_object_set_new(rootJ, "Color1B", json_integer(colorBoxB[0]));
		json_object_set_new(rootJ, "Color2", json_integer(colorBox[1]));
		json_object_set_new(rootJ, "Color2R", json_integer(colorBoxR[1]));
		json_object_set_new(rootJ, "Color2G", json_integer(colorBoxG[1]));
		json_object_set_new(rootJ, "Color2B", json_integer(colorBoxB[1]));
		json_object_set_new(rootJ, "Color3", json_integer(colorBox[2]));
		json_object_set_new(rootJ, "Color3R", json_integer(colorBoxR[2]));
		json_object_set_new(rootJ, "Color3G", json_integer(colorBoxG[2]));
		json_object_set_new(rootJ, "Color3B", json_integer(colorBoxB[2]));
		json_object_set_new(rootJ, "Color4", json_integer(colorBox[3]));
		json_object_set_new(rootJ, "Color4R", json_integer(colorBoxR[3]));
		json_object_set_new(rootJ, "Color4G", json_integer(colorBoxG[3]));
		json_object_set_new(rootJ, "Color4B", json_integer(colorBoxB[3]));
		json_object_set_new(rootJ, "Light1", json_integer(lightTime[0]));
		json_object_set_new(rootJ, "Light2", json_integer(lightTime[1]));
		json_object_set_new(rootJ, "Light3", json_integer(lightTime[2]));
		json_object_set_new(rootJ, "Light4", json_integer(lightTime[3]));
		json_object_set_new(rootJ, "Zoom1", json_integer(zoom[0]));
		json_object_set_new(rootJ, "Zoom2", json_integer(zoom[1]));
		json_object_set_new(rootJ, "Zoom3", json_integer(zoom[2]));
		json_object_set_new(rootJ, "Zoom4", json_integer(zoom[3]));
		json_object_set_new(rootJ, "Slot1", json_string(storedPath[0].c_str()));
		json_object_set_new(rootJ, "Slot2", json_string(storedPath[1].c_str()));
		json_object_set_new(rootJ, "Slot3", json_string(storedPath[2].c_str()));
		json_object_set_new(rootJ, "Slot4", json_string(storedPath[3].c_str()));
		json_object_set_new(rootJ, "UserFolder", json_string(userFolder.c_str()));
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
		json_t* disableNavJ = json_object_get(rootJ, "DisableNav");
		if (disableNavJ)
			disableNav = json_boolean_value(disableNavJ);
		json_t* scrollingJ = json_object_get(rootJ, "Scrolling");
		if (scrollingJ)
			scrolling = json_integer_value(scrollingJ);
		json_t* sampleInPatchJ = json_object_get(rootJ, "sampleInPatch");
		if (sampleInPatchJ)
			sampleInPatch = json_boolean_value(sampleInPatchJ);
		json_t* lightBoxJ = json_object_get(rootJ, "LightBox");
		if (lightBoxJ)
			lightBox = json_integer_value(lightBoxJ);
		json_t* displayTrigJ = json_object_get(rootJ, "DisplayTrig");
		if (displayTrigJ)
			displayTrig = json_integer_value(displayTrigJ);
		json_t* color1J = json_object_get(rootJ, "Color1");
		if (color1J)
			colorBox[0] = json_integer_value(color1J);
		json_t* color1RJ = json_object_get(rootJ, "Color1R");
		if (color1RJ)
			colorBoxR[0] = json_integer_value(color1RJ);
		json_t* color1GJ = json_object_get(rootJ, "Color1G");
		if (color1GJ)
			colorBoxG[0] = json_integer_value(color1GJ);
		json_t* color1BJ = json_object_get(rootJ, "Color1B");
		if (color1BJ)
			colorBoxB[0] = json_integer_value(color1BJ);
		json_t* color2J = json_object_get(rootJ, "Color2");
		if (color2J)
			colorBox[1] = json_integer_value(color2J);
		json_t* color2RJ = json_object_get(rootJ, "Color2R");
		if (color2RJ)
			colorBoxR[1] = json_integer_value(color2RJ);
		json_t* color2GJ = json_object_get(rootJ, "Color2G");
		if (color2GJ)
			colorBoxG[1] = json_integer_value(color2GJ);
		json_t* color2BJ = json_object_get(rootJ, "Color2B");
		if (color2BJ)
			colorBoxB[1] = json_integer_value(color2BJ);
		json_t* color3J = json_object_get(rootJ, "Color3");
		if (color3J)
			colorBox[2] = json_integer_value(color3J);
		json_t* color3RJ = json_object_get(rootJ, "Color3R");
		if (color3RJ)
			colorBoxR[2] = json_integer_value(color3RJ);
		json_t* color3GJ = json_object_get(rootJ, "Color3G");
		if (color3GJ)
			colorBoxG[2] = json_integer_value(color3GJ);
		json_t* color3BJ = json_object_get(rootJ, "Color3B");
		if (color3BJ)
			colorBoxB[2] = json_integer_value(color3BJ);
		json_t* color4J = json_object_get(rootJ, "Color4");
		if (color4J)
			colorBox[3] = json_integer_value(color4J);
		json_t* color4RJ = json_object_get(rootJ, "Color4R");
		if (color4RJ)
			colorBoxR[3] = json_integer_value(color4RJ);
		json_t* color4GJ = json_object_get(rootJ, "Color4G");
		if (color4GJ)
			colorBoxG[3] = json_integer_value(color4GJ);
		json_t* color4BJ = json_object_get(rootJ, "Color4B");
		if (color4BJ)
			colorBoxB[3] = json_integer_value(color4BJ);
		json_t* light1J = json_object_get(rootJ, "Light1");
		if (light1J)
			lightTime[0] = json_integer_value(light1J);
		json_t* light2J = json_object_get(rootJ, "Light2");
		if (light2J)
			lightTime[1] = json_integer_value(light2J);
		json_t* light3J = json_object_get(rootJ, "Light3");
		if (light3J)
			lightTime[2] = json_integer_value(light3J);

		json_t* light4J = json_object_get(rootJ, "Light4");
		if (light4J)
			lightTime[3] = json_integer_value(light4J);
		json_t* zoom1J = json_object_get(rootJ, "Zoom1");
		if (zoom1J)
			zoom[0] = json_integer_value(zoom1J);
		json_t* zoom2J = json_object_get(rootJ, "Zoom2");
		if (zoom2J)
			zoom[1] = json_integer_value(zoom2J);
		json_t* zoom3J = json_object_get(rootJ, "Zoom3");
		if (zoom3J)
			zoom[2] = json_integer_value(zoom3J);
		json_t* zoom4J = json_object_get(rootJ, "Zoom4");
		if (zoom4J)
			zoom[3] = json_integer_value(zoom4J);
		json_t *slot1J = json_object_get(rootJ, "Slot1");
		if (slot1J) {
			storedPath[0] = json_string_value(slot1J);
			if (storedPath[0] != "")
				loadSample(storedPath[0], 0);
		}
		json_t *slot2J = json_object_get(rootJ, "Slot2");
		if (slot2J) {
			storedPath[1] = json_string_value(slot2J);
			if (storedPath[1] != "")
				loadSample(storedPath[1], 1);
		}
		json_t *slot3J = json_object_get(rootJ, "Slot3");
		if (slot3J) {
			storedPath[2] = json_string_value(slot3J);
			if (storedPath[2] != "")
				loadSample(storedPath[2], 2);
		}
		json_t *slot4J = json_object_get(rootJ, "Slot4");
		if (slot4J) {
			storedPath[3] = json_string_value(slot4J);
			if (storedPath[3] != "")
				loadSample(storedPath[3], 3);
		}
		json_t *userFolderJ = json_object_get(rootJ, "UserFolder");
		if (userFolderJ) {
			userFolder = json_string_value(userFolderJ);
			if (userFolder != "") {
				createFolder(userFolder);
				if (rootFound) {
					folderTreeData.push_back(tempTreeData);
					folderTreeDisplay.push_back(tempTreeDisplay);
				}
			}
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

	void selectRootFolder() {
		const char* prevFolder = userFolder.c_str();
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN_DIR, prevFolder, NULL, NULL, [this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN_DIR, prevFolder, NULL, NULL);
#endif
		if (path) {
			folderTreeData.clear();
			folderTreeDisplay.clear();
			userFolder = std::string(path);
			createFolder(userFolder);
			if (rootFound) {
				folderTreeData.push_back(tempTreeData);
				folderTreeDisplay.push_back(tempTreeDisplay);
			}
		}
		free(path);
#if defined(METAMODULE)
		});
#endif
	};

	void refreshRootFolder() {
		folderTreeData.clear();
		folderTreeDisplay.clear();
		createFolder(userFolder);
		if (rootFound) {
			folderTreeData.push_back(tempTreeData);
			folderTreeDisplay.push_back(tempTreeDisplay);
		}
	}

	void createFolder(std::string dir_path) {
		vector <std::string> browserList;
		vector <std::string> browserListDisplay;
		vector <std::string> browserDir;
		vector <std::string> browserDirDisplay;
		vector <std::string> browserFiles;
		vector <std::string> browserFilesDisplay;

		tempTreeData.clear();
		tempTreeDisplay.clear();

		std::string lastChar = dir_path.substr(dir_path.length()-1,dir_path.length()-1);
		if (lastChar != "/")
			dir_path += "/";

		DIR *dir = opendir(dir_path.c_str());
		if (dir) {
			rootFound = true;
			struct dirent *d;
			while ((d = readdir(dir))) {
				std::string filename = d->d_name;
				if (filename != "." && filename != "..") {
					std::string filepath = std::string(dir_path) + filename;
					struct stat statbuf;
					if (stat(filepath.c_str(), &statbuf) == 0 && (statbuf.st_mode & S_IFMT) == S_IFDIR) {
						//browserDir.push_back(filepath + "/");
						browserDir.push_back(filepath);
						browserDirDisplay.push_back(filename);
					} else {
						std::size_t found = filename.find(".wav",filename.length()-5);
						if (found==std::string::npos)
							found = filename.find(".WAV",filename.length()-5);
						if (found!=std::string::npos) {
							browserFiles.push_back(filepath);
							browserFilesDisplay.push_back(filename.substr(0, filename.length()-4));
						}
					}
				}
	   		}
	   		closedir(dir);

			sort(browserDir.begin(), browserDir.end());
			sort(browserDirDisplay.begin(), browserDirDisplay.end());
			sort(browserFiles.begin(), browserFiles.end());
			sort(browserFilesDisplay.begin(), browserFilesDisplay.end());
			
			// this adds "/" to browserDir after sorting to avoid wrong sorting with foldernames with spaces
			int dirSize = (int)browserDir.size();
			for (int i=0; i < dirSize; i++)
				browserDir[i] += "/";
			
			tempTreeData.push_back(dir_path);
			tempTreeDisplay.push_back(dir_path);

			for (unsigned int i = 0; i < browserDir.size(); i++) {
				tempTreeData.push_back(browserDir[i]);
				tempTreeDisplay.push_back(browserDirDisplay[i]);
			}
			for (unsigned int i = 0; i < browserFiles.size(); i++) {
				tempTreeData.push_back(browserFiles[i]);
				tempTreeDisplay.push_back(browserFilesDisplay[i]);
			}
		} else {
			rootFound = false;
		}
	};

	void createCurrentFolder(std::string dir_path, int slot) {
		vector <std::string> browserList;
		vector <std::string> browserFiles;

		tempTreeData.clear();

		std::string lastChar = dir_path.substr(dir_path.length()-1,dir_path.length()-1);
		if (lastChar != "/")
			dir_path += "/";

		DIR *dir = opendir(dir_path.c_str());
		struct dirent *d;
		while ((d = readdir(dir))) {
			std::string filename = d->d_name;
			if (filename != "." && filename != "..") {
				std::string filepath = std::string(dir_path) + filename;

					std::size_t found = filename.find(".wav",filename.length()-5);
					if (found==std::string::npos)
						found = filename.find(".WAV",filename.length()-5);
					if (found!=std::string::npos) {
						browserFiles.push_back(filepath);
					}

			}
   		}
   		closedir(dir);

		sort(browserFiles.begin(), browserFiles.end());
		
		for (unsigned int i = 0; i < browserFiles.size(); i++) {
			tempTreeData.push_back(browserFiles[i]);
		}
	};

	void displayRecalc(int zoom, int slot) {
		int div = 1;
		displayBuff[slot].clear();
		switch (zoom) {
			case 1:
				div = 2;
			break;
			case 2:
				div = 4;
			break;
			case 3:
				div = 8;
			break;
		}
		for (int i = 0; i < floor(totalSampleC[slot]/div); i = i + floor(totalSampleC[slot]/div/59))
			displayBuff[slot].push_back(playBuffer[slot][0][i]);
	}

	void swapSlot(int slot1, int slot2) {
		vector<float> swapBuffer[2];
		swapBuffer[0].clear();
		swapBuffer[1].clear();
		unsigned int swapSampleRate = sampleRate[slot1];
		drwav_uint64 swapTotalSampleC = totalSampleC[slot1];
		drwav_uint64 swapTotalSamples = totalSamples[slot1];
		double swapSampleCoeff = sampleCoeff[slot1];
		std::string swapStoredPath = storedPath[slot1];
		std::string swapFileDescription = fileDescription[slot1];
		std::string swapFileDisplay = fileDisplay[slot1];
		std::string swapScrollDisplay = scrollDisplay[slot1];
		std::string swapCurrFileDisplay = currFileDisplay[slot1];
		float swapDisplayCoeff = displayCoeff[slot1];
		bool swapFileFound = fileFound[slot1];
		bool swapFileLoaded = fileLoaded[slot1];
		bool swapLoadFromPatch = loadFromPatch[slot1];

		for (unsigned int i = 0; i < totalSampleC[slot1]; i++) {
			swapBuffer[0].push_back(playBuffer[slot1][0][i]);
			swapBuffer[1].push_back(playBuffer[slot1][1][i]);
		}

		clearSlot(slot1);
		for (unsigned int i = 0; i < totalSampleC[slot2]; i++) {
			playBuffer[slot1][0].push_back(playBuffer[slot2][0][i]);
			playBuffer[slot1][1].push_back(playBuffer[slot2][1][i]);
		}
		sampleRate[slot1] = sampleRate[slot2];
		totalSampleC[slot1] = totalSampleC[slot2];
		totalSamples[slot1] = totalSamples[slot2];
		sampleCoeff[slot1] = sampleCoeff[slot2];
		storedPath[slot1] = storedPath[slot2];
		fileDescription[slot1] = fileDescription[slot2];
		fileDisplay[slot1] = fileDisplay[slot2];
		scrollDisplay[slot1] = scrollDisplay[slot2];
		currFileDisplay[slot1] = currFileDisplay[slot2];
		displayCoeff[slot1] = displayCoeff[slot2];
		fileFound[slot1] = fileFound[slot2];
		loadFromPatch[slot1] = loadFromPatch[slot2];
		displayRecalc(zoom[slot1], slot1);

		std::string path = storedPath[slot1];
		if (path != "") {
			currentFolder[slot1] = system::getDirectory(path);
			createCurrentFolder(currentFolder[slot1], slot1);
			currentFolderV[slot1].clear();
			currentFolderV[slot1] = tempTreeData;
			for (unsigned int i = 0; i < currentFolderV[slot1].size(); i++) {
				if (system::getFilename(path) == system::getFilename(currentFolderV[slot1][i])) {
					currentFile[slot1] = i;
					i = currentFolderV[slot1].size();
				}
			}
		}

		fileLoaded[slot1] = fileLoaded[slot2];

		clearSlot(slot2);
		for (unsigned int i = 0; i < swapTotalSampleC; i++) {
			playBuffer[slot2][0].push_back(swapBuffer[0][i]);
			playBuffer[slot2][1].push_back(swapBuffer[1][i]);
		}
		sampleRate[slot2] = swapSampleRate;
		totalSampleC[slot2] = swapTotalSampleC;
		totalSamples[slot2] = swapTotalSamples;
		sampleCoeff[slot2] = swapSampleCoeff;
		storedPath[slot2] = swapStoredPath;
		fileDescription[slot2] = swapFileDescription;
		fileDisplay[slot2] = swapFileDisplay;
		scrollDisplay[slot2] = swapScrollDisplay;
		currFileDisplay[slot2] = swapCurrFileDisplay;
		displayCoeff[slot2] = swapDisplayCoeff;
		fileFound[slot2] = swapFileFound;
		loadFromPatch[slot2] = swapLoadFromPatch;
		displayRecalc(zoom[slot2], slot2);

		path = storedPath[slot2];
		if (path != "") {
			currentFolder[slot2] = system::getDirectory(path);
			createCurrentFolder(currentFolder[slot2], slot2);
			currentFolderV[slot2].clear();
			currentFolderV[slot2] = tempTreeData;
			for (unsigned int i = 0; i < currentFolderV[slot2].size(); i++) {
				if (system::getFilename(path) == system::getFilename(currentFolderV[slot2][i])) {
					currentFile[slot2] = i;
					i = currentFolderV[slot2].size();
				}
			}
		}

		fileLoaded[slot2] = swapFileLoaded;
	}

	void copySlot(int slot1, int slot2) {
		clearSlot(slot2);
		sampleRate[slot2] = sampleRate[slot1];
		totalSampleC[slot2] = totalSampleC[slot1];
		totalSamples[slot2] = totalSamples[slot1];
		sampleCoeff[slot2] = sampleCoeff[slot1];
		storedPath[slot2] = storedPath[slot1];
		fileDescription[slot2] = fileDescription[slot1];
		fileDisplay[slot2] = fileDisplay[slot1];
		scrollDisplay[slot2] = scrollDisplay[slot1];
		currFileDisplay[slot2] = currFileDisplay[slot1];
		displayCoeff[slot2] = displayCoeff[slot1];
		fileFound[slot2] = fileFound[slot1];
		loadFromPatch[slot2] = loadFromPatch[slot1];
		for (unsigned int i = 0; i < totalSampleC[slot1]; i++) {
			playBuffer[slot2][0].push_back(playBuffer[slot1][0][i]);
			playBuffer[slot2][1].push_back(playBuffer[slot1][1][i]);
		}
		displayRecalc(zoom[slot2], slot2);

		std::string path = storedPath[slot2];
		if (path != "") {
			currentFolder[slot2] = system::getDirectory(path);
			createCurrentFolder(currentFolder[slot2], slot2);
			currentFolderV[slot2].clear();
			currentFolderV[slot2] = tempTreeData;
			for (unsigned int i = 0; i < currentFolderV[slot2].size(); i++) {
				if (system::getFilename(path) == system::getFilename(currentFolderV[slot2][i])) {
					currentFile[slot2] = i;
					i = currentFolderV[slot2].size();
				}
			}
		}

		fileLoaded[slot2] = fileLoaded[slot1];
	}

	void menuLoadSample(int slot) {
		static const char FILE_FILTERS[] = "Wave (.wav):wav,WAV;All files (*.*):*.*";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
#endif
		fileLoaded[slot] = false;
		restoreLoadFromPatch[slot] = false;
		if (path) {
			loadFromPatch[slot] = false;
			loadSample(path, slot);
			storedPath[slot] = std::string(path);
		} else {
			restoreLoadFromPatch[slot] = true;
			fileLoaded[slot] = true;
		}
		if (storedPath[slot] == "" || fileFound[slot] == false) {
			fileLoaded[slot] = false;
		}
		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void loadSample(std::string fromPath, int slot) {
		std::string path = fromPath;
		unsigned int c;
		unsigned int sr;
		drwav_uint64 tsc;
		float* pSampleData;
		pSampleData = drwav_open_and_read_file_f32(path.c_str(), &c, &sr, &tsc);

		if (pSampleData != NULL && tsc > minSamplesToLoad * c) {
			fileFound[slot] = true;
			displayCoeff[slot] = 0;
			//channels[slot] = c;
			sampleRate[slot] = sr * 2;
			calcBiquadLpf(20000.0, sampleRate[slot], 1);
			playBuffer[slot][0].clear();
			playBuffer[slot][1].clear();
			displayBuff[slot].clear();
			for (unsigned int i = 0; i < tsc; i = i + c) {
				playBuffer[slot][0].push_back(pSampleData[i]);
				playBuffer[slot][0].push_back(0);
				if (abs(pSampleData[i]) > displayCoeff[slot])
					displayCoeff[slot] = abs(pSampleData[i]);
			}
			displayCoeff[slot] *= 2;

			totalSampleC[slot] = playBuffer[slot][0].size();
			totalSamples[slot] = totalSampleC[slot]-1;
			drwav_free(pSampleData);

			for (unsigned int i = 1; i < totalSamples[slot]; i = i + 2)		// averaging oversampled vector
				playBuffer[slot][0][i] = playBuffer[slot][0][i-1] * .5f + playBuffer[slot][0][i+1] * .5f;

			playBuffer[slot][0][totalSamples[slot]] = playBuffer[slot][0][totalSamples[slot]-1] * .5f; // halve the last sample

			for (unsigned int i = 0; i < totalSampleC[slot]; i++)	// populating filtered vector
				playBuffer[slot][1].push_back(biquadLpf(playBuffer[slot][0][i]));

			sampleCoeff[slot] = sampleRate[slot] / (APP->engine->getSampleRate());		// the % distance between samples at speed 1x

			displayRecalc(zoom[slot], slot);

			if (loadFromPatch[slot])
				path = storedPath[slot];

			fileDescription[slot] = system::getFilename(std::string(path));
			fileDescription[slot] = fileDescription[slot].substr(0,fileDescription[slot].length()-4);

			if (loadFromPatch[slot])
				fileDescription[slot] = "(!)"+fileDescription[slot];
			else {
				currentFolder[slot] = system::getDirectory(path);
				createCurrentFolder(currentFolder[slot], slot);
				currentFolderV[slot].clear();
				currentFolderV[slot] = tempTreeData;
				for (unsigned int i = 0; i < currentFolderV[slot].size(); i++) {
					if (system::getFilename(path) == system::getFilename(currentFolderV[slot][i])) {
						currentFile[slot] = i;
						i = currentFolderV[slot].size();
					}
				}
			}

			// *** CHARs CHECK according to font
			std::string tempFileDisplay = fileDescription[slot];
			char tempFileChar;
			fileDisplay[slot] = "";
			for (int i = 0; i < int(tempFileDisplay.length()); i++) {
				tempFileChar = tempFileDisplay.at(i);
				if ( (int(tempFileChar) > 47 && int(tempFileChar) < 58)
					|| (int(tempFileChar) > 64 && int(tempFileChar) < 123)
					|| int(tempFileChar) == 45 || int(tempFileChar) == 46 || int(tempFileChar) == 95 )
					fileDisplay[slot] += tempFileChar;
			}

			if (fileDisplay[slot].length() > 7)
				scrollDisplay[slot] = "!!!!!!!" + fileDisplay[slot] + "!!!!!!";
			else
				scrollDisplay[slot] = fileDisplay[slot];

			storedPath[slot] = path;

			fileLoaded[slot] = true;

		} else {

			fileFound[slot] = false;
			fileLoaded[slot] = false;
			//storedPath[slot] = path;
			if (loadFromPatch[slot])
				path = storedPath[slot];
			fileDescription[slot] = "(!)"+path;
			fileDisplay[slot] = "-------";
			currFileDisplay[slot] = "-------";
			scrollDisplay[slot] = "-------";
		}
	};

	void saveSample(std::string path, int slot) {
		drwav_uint64 samples;

		samples = playBuffer[slot][0].size();

		std::vector<float> data;

		for (unsigned int i = 0; i <= playBuffer[slot][0].size(); i = i + 2)
			//data.push_back(playBuffer[slot][0][i] / 5);
			data.push_back(playBuffer[slot][0][i]);

		drwav_data_format format;
		format.container = drwav_container_riff;
		format.format = DR_WAVE_FORMAT_IEEE_FLOAT;

		format.channels = 1;

		samples /= 2;

		format.sampleRate = sampleRate[slot] / 2;

		format.bitsPerSample = 32;

		if (path.substr(path.size() - 4) != ".wav" && path.substr(path.size() - 4) != ".WAV")
			path += ".wav";

		drwav *pWav = drwav_open_file_write(path.c_str(), &format);
		drwav_write(pWav, samples, data.data());
		drwav_close(pWav);

		data.clear();
	}
	
	void clearSlot(int slot) {
		fileLoaded[slot] = false;
		fileFound[slot] = false;
		loadFromPatch[slot] = false;
		restoreLoadFromPatch[slot] = false;
		storedPath[slot] = "";
		fileDescription[slot] = "--none--";
		fileDisplay[slot] = "-------";
		scrollDisplay[slot] = "-------";
		currFileDisplay[slot] = "-------";
		fileFound[slot] = false;
		playBuffer[slot][0].clear();
		playBuffer[slot][1].clear();
		displayBuff[slot].clear();
		totalSampleC[slot] = 0;
	}

	void randomizeAllSlots() {
		currentFolder[0] = system::getDirectory(userFolder+"/");
		createCurrentFolder(currentFolder[0], 0);
		currentFolderV[0].clear();
		currentFolderV[0] = tempTreeData;
		currentFolderV[1].clear();
		currentFolderV[1] = tempTreeData;
		currentFolderV[2].clear();
		currentFolderV[2] = tempTreeData;
		currentFolderV[3].clear();
		currentFolderV[3] = tempTreeData;

		if (currentFolderV[0].size() > 0) {
			for (int slot = 0; slot < 4; slot++) {
				float rnd = random::uniform();
				play[slot] = false;
				currentFile[slot] = int(currentFolderV[slot].size() * rnd);
				if (currentFile[slot] >= int(currentFolderV[slot].size()))
					currentFile[slot] = currentFolderV[slot].size()-1;
				loadSample(currentFolderV[slot][currentFile[slot]], slot);
			}
		}
	}

	void randomizeSlot(int slot) {
		currentFolder[slot] = system::getDirectory(userFolder+"/");
		createCurrentFolder(currentFolder[slot], slot);
		currentFolderV[slot].clear();
		currentFolderV[slot] = tempTreeData;
		if (currentFolderV[slot].size() > 0) {
			float rnd = random::uniform();
			play[slot] = false;
			currentFile[slot] = int(currentFolderV[slot].size() * rnd);
			//DEBUG("folder %s - slot %d - rnd %f - file %d - size %d", userFolder.c_str(), slot, rnd, currentFile[slot], int(currentFolderV[slot].size()));
			if (currentFile[slot] >= int(currentFolderV[slot].size()))
				currentFile[slot] = currentFolderV[slot].size()-1;
			loadSample(currentFolderV[slot][currentFile[slot]], slot);
		}
	}
	
	void process(const ProcessArgs &args) override {

		choke[0] = params[CHOKE_PARAMS].getValue();
		choke[1] = params[CHOKE_PARAMS+1].getValue();
		choke[2] = params[CHOKE_PARAMS+2].getValue();
		lights[CHOKE_LIGHT].setBrightness(choke[0]);
		lights[CHOKE_LIGHT+1].setBrightness(choke[1]);
		lights[CHOKE_LIGHT+2].setBrightness(choke[2]);
		limit[0] = params[LIMIT_PARAMS].getValue();
		limit[1] = params[LIMIT_PARAMS+1].getValue();
		limit[2] = params[LIMIT_PARAMS+2].getValue();
		limit[3] = params[LIMIT_PARAMS+3].getValue();
		lights[LIMIT_LIGHT].setBrightness(limit[0]);
		lights[LIMIT_LIGHT+1].setBrightness(limit[1]);
		lights[LIMIT_LIGHT+2].setBrightness(limit[2]);
		lights[LIMIT_LIGHT+3].setBrightness(limit[3]);

		summedOutput = 0;
		for (int slot = 0; slot < 4; slot++) {

			//if (!disableNav) {
			if (!disableNav && !loadFromPatch[slot]) {
				nextSample[slot] = params[NEXTSAMPLE_PARAM+slot].getValue();
				if (fileLoaded[slot] && nextSample[slot] && !prevNextSample[slot]) {
					play[slot] = false;
					currentFile[slot]++;
					if (currentFile[slot] >= int(currentFolderV[slot].size()))
						currentFile[slot] = 0;
					loadSample(currentFolderV[slot][currentFile[slot]], slot);
				}
				prevNextSample[slot] = nextSample[slot];

				prevSample[slot] = params[PREVSAMPLE_PARAM+slot].getValue();
				if (fileLoaded[slot] && prevSample[slot] && !prevPrevSample[slot]) {
					play[slot] = false;
					currentFile[slot]--;
					if (currentFile[slot] < 0)
						currentFile[slot] = currentFolderV[slot].size()-1;
					loadSample(currentFolderV[slot][currentFile[slot]], slot);
				}
				prevPrevSample[slot] = prevSample[slot];
			}

			trigValue[slot] = inputs[TRIG_INPUT+slot].getVoltage();

			if ((trigValue[slot] >= 1 && prevTrigValue[slot] < 1) || uiTrig[slot]) {
				uiTrig[slot] = false;
				if (play[slot]) {
					fading[slot] = true;
					fadingValue[slot] = 1.f;
					fadedPosition[slot] = samplePos[slot];
				}
				play[slot] = true;
				if (fileLoaded[slot])
					slotTriggered[slot] = true;
				samplePos[slot] = 0;
				currSampleWeight[slot] = sampleCoeff[slot];
				prevSamplePos[slot] = 0;
				prevSampleWeight[slot] = 0;

				if (inputs[ACC_INPUT+slot].getVoltage() > 1)
					level[slot] = params[ACCVOL_PARAM+slot].getValue() + (inputs[ACCVOL_INPUT+slot].getVoltage() * params[ACCVOLATNV_PARAM+slot].getValue() * 0.1);
				else
					level[slot] = params[TRIGVOL_PARAM+slot].getValue() + (inputs[TRIGVOL_INPUT+slot].getVoltage() * params[TRIGVOLATNV_PARAM+slot].getValue() * 0.1);

				if (level[slot] > 2)
					level[slot] = 2;
				else if (level[slot] < 0)
					level[slot] = 0;
				
				if (slot < 3 && choke[slot]) {
					choking[slot+1] = true;
					chokeValue[slot+1] = 1.f;
				}
			}
			prevTrigValue[slot] = trigValue[slot];
			currentOutput = 0;

			if (fileLoaded[slot] && play[slot] && floor(samplePos[slot]) < totalSampleC[slot]) {
				switch (interpolationMode) {
					case NO_INTERP:
						currentOutput = 5 * level[slot] * float(playBuffer[slot][antiAlias][floor(samplePos[slot])]);
					break;

					case LINEAR1_INTERP:
						if (currSampleWeight[slot] == 0) {
							currentOutput = 5 * level[slot] * float(playBuffer[slot][antiAlias][floor(samplePos[slot])]);
						} else {
							currentOutput = 5 * level[slot] * float(
											(playBuffer[slot][antiAlias][floor(samplePos[slot])] * (1-currSampleWeight[slot])) +
											(playBuffer[slot][antiAlias][floor(samplePos[slot])+1] * currSampleWeight[slot])
											);
						}
					break;

					case LINEAR2_INTERP:
						if (currSampleWeight[slot] == 0) {
							currentOutput = 5 * level[slot] * float(playBuffer[slot][antiAlias][floor(samplePos[slot])]);
						} else {
							currentOutput = 5 * level[slot] * float(
											(
												(playBuffer[slot][antiAlias][floor(prevSamplePos[slot])] * (1-prevSampleWeight[slot])) +
												(playBuffer[slot][antiAlias][floor(prevSamplePos[slot])+1] * prevSampleWeight[slot]) +
												(playBuffer[slot][antiAlias][floor(samplePos[slot])] * (1-currSampleWeight[slot])) +
												(playBuffer[slot][antiAlias][floor(samplePos[slot])+1] * currSampleWeight[slot])
											) / 2);
						}
					break;

					case HERMITE_INTERP:
						if (currSampleWeight[slot] == 0) {
							currentOutput = 5 * level[slot] * float(playBuffer[slot][antiAlias][floor(samplePos[slot])]);
						} else {
							if (floor(samplePos[slot]) > 1 && floor(samplePos[slot]) < totalSamples[slot] - 1) {
								/*
								resampled[slot] = hermiteInterpol(playBuffer[slot][antiAlias][floor(samplePos[slot])-1],
																playBuffer[slot][antiAlias][floor(samplePos[slot])],
																playBuffer[slot][antiAlias][floor(samplePos[slot])+1],
																playBuffer[slot][antiAlias][floor(samplePos[slot])+2],
																currSampleWeight[slot]);
								*/
								// below is translation of the above function
								double a1 = .5F * (playBuffer[slot][antiAlias][floor(samplePos[slot])+1] - playBuffer[slot][antiAlias][floor(samplePos[slot])-1]);
								double a2 = playBuffer[slot][antiAlias][floor(samplePos[slot])-1] - (2.5F * playBuffer[slot][antiAlias][floor(samplePos[slot])]) + (2 * playBuffer[slot][antiAlias][floor(samplePos[slot])+1]) - (.5F * playBuffer[slot][antiAlias][floor(samplePos[slot])+2]);
								double a3 = (.5F * (playBuffer[slot][antiAlias][floor(samplePos[slot])+2] - playBuffer[slot][antiAlias][floor(samplePos[slot])-1])) + (1.5F * (playBuffer[slot][antiAlias][floor(samplePos[slot])] - playBuffer[slot][antiAlias][floor(samplePos[slot])+1]));
								currentOutput = 5 * level[slot] * float(
									(((((a3 * currSampleWeight[slot]) + a2) * currSampleWeight[slot]) + a1) * currSampleWeight[slot]) + playBuffer[slot][antiAlias][floor(samplePos[slot])]
								);
							} else {
								currentOutput = 5 * level[slot] * float(playBuffer[slot][antiAlias][floor(samplePos[slot])]);
							}
						}
					break;
				}

				if (slot > 0 && choking[slot]) {
					if (chokeValue[slot] < 0.0) {
						choking[slot] = false;
						play[slot] = false;
						currentOutput = 0;
					} else {
						currentOutput *= chokeValue[slot];
						chokeValue[slot] -= fadeDecrement;
					}
				}

				prevSamplePos[slot] = samplePos[slot];
				if (inputs[SPEED_INPUT+slot].isConnected()) {
					currentSpeed = double(params[SPEED_PARAM+slot].getValue() + (inputs[SPEED_INPUT+slot].getVoltage() * params[SPEEDATNV_PARAM+slot].getValue() * 0.1));
					if (currentSpeed > 2)
						currentSpeed = 2;
					else {
						if (currentSpeed < 0.01)
							currentSpeed = 0.01;
					}
				} else {
					currentSpeed = double(params[SPEED_PARAM+slot].getValue());
				}

				samplePos[slot] += sampleCoeff[slot]*currentSpeed;

				if (interpolationMode > NO_INTERP) {
					prevSampleWeight[slot] = currSampleWeight[slot];
					currSampleWeight[slot] = samplePos[slot] - floor(samplePos[slot]);
				}

				if (fading[slot]) { 					// fades previous samples if retrigged when already playing
					if (fadingValue[slot] > 0) {
						fadingValue[slot] -= fadeDecrement;
						currentOutput += (playBuffer[slot][antiAlias][floor(fadedPosition[slot])] * fadingValue[slot] * level[slot] * 5);
						fadedPosition[slot] += sampleCoeff[slot]*currentSpeed;
						if (fadedPosition[slot] > totalSamples[slot])
							fading[slot] = false;
					} else
						fading[slot] = false;
				}
			} else {
				choking[slot] = false;
				play[slot] = false;
				fading[slot] = false;
			}

			switch (outsMode) {
				case NORMALLED_OUTS:
					summedOutput += currentOutput;

					if (limit[slot]) {
						if (summedOutput > 5)
							summedOutput = 5;
						else if (summedOutput < -5)
							summedOutput = -5;
					}

					if (outputs[OUT_OUTPUT+slot].isConnected()) {
						outputs[OUT_OUTPUT+slot].setVoltage(summedOutput);
						summedOutput = 0;
					}
				break;

				case SOLO_OUTS:
					if (outputs[OUT_OUTPUT+slot].isConnected()) {
						
						if (limit[slot]) {
							if (currentOutput > 5)
								currentOutput = 5;
							else if (currentOutput < -5)
								currentOutput = -5;
						}
						outputs[OUT_OUTPUT+slot].setVoltage(currentOutput);

					}
				break;

				case UNCONNECTED_ON_4:
					if (slot == 3) {
						summedOutput += currentOutput;
						if (limit[slot]) {
							if (summedOutput > 5)
								summedOutput = 5;
							else if (summedOutput < -5)
								summedOutput = -5;
						}
						outputs[OUT_OUTPUT+slot].setVoltage(summedOutput);
					} else if (outputs[OUT_OUTPUT+slot].isConnected()) {
						if (limit[slot]) {
							if (currentOutput > 5)
								currentOutput = 5;
							else if (currentOutput < -5)
								currentOutput = -5;
						}
						outputs[OUT_OUTPUT+slot].setVoltage(currentOutput);
					} else {
						summedOutput += currentOutput;
						if (limit[slot]) {
							if (summedOutput > 5)
								summedOutput = 5;
							else if (summedOutput < -5)
								summedOutput = -5;
						}
					}
				break;
			}

			//if (!inputs[TRIG_INPUT+slot].isConnected())	// removed to let the Dislay Triggering work
			//	play[slot] = false;
		}
	}
};

struct dpxSlot1Display : TransparentWidget {
	DrumPlayerXtra *module;
	int frame = 0;

	dpxSlot1Display() {

	}

	struct ColorItem : MenuItem {
		DrumPlayerXtra* module;
		int colorBox;
		void onAction(const event::Action& e) override {
			module->colorBox[0] = colorBox;
		}
	};

	struct ZoomItem : MenuItem {
		DrumPlayerXtra* module;
		int zoom;
		void onAction(const event::Action& e) override {
			module->zoom[0] = zoom;
			if (module->fileLoaded[0])
				module->displayRecalc(zoom, 0);
		}
	};

	struct LightTimeItem : MenuItem {
		DrumPlayerXtra* module;
		int lightTime;
		void onAction(const event::Action& e) override {
			module->lightTime[0] = lightTime;
		}
	};

	struct labelTextFieldR : TextField {
		DrumPlayerXtra *module;
		unsigned int column = 0;
		labelTextFieldR(unsigned int column) {
			this->column = column;
			this->box.pos.x = 30;
			this->box.size.x = 30;
			this->multiline = false;
		}

		void onChange(const event::Change& e) override {
			if (text != "") {
				if (text.length() > 3) {
					text = text.substr(0,3);
					rack::TextField::cursor = 3;
					rack::TextField::selection = 3;
				} else {
					if (all_of(text.begin(), text.end(), ::isdigit)) {
						int color = std::stoi(text);
						if (color < 0 && color < 256)
							module->colorBoxR[column] = 0;
						else if (color > 255)
							module->colorBoxR[column] = 255;
						else
							module->colorBoxR[column] = color;
					} else {
						module->colorBoxR[column] = 0;
					}
				}
			} else 
				module->colorBoxR[column] = 0;
		};
	};

	struct labelTextFieldG : TextField {
		DrumPlayerXtra *module;
		unsigned int column = 0;
		labelTextFieldG(unsigned int column) {
			this->column = column;
			this->box.pos.x = 30;
			this->box.size.x = 30;
			this->multiline = false;
		}

		void onChange(const event::Change& e) override {
			if (text != "") {
				if (text.length() > 3) {
					text = text.substr(0,3);
					rack::TextField::cursor = 3;
					rack::TextField::selection = 3;
				} else {
					if (all_of(text.begin(), text.end(), ::isdigit)) {
						int color = std::stoi(text);
						if (color < 0 && color < 256)
							module->colorBoxG[column] = 0;
						else if (color > 255)
							module->colorBoxG[column] = 255;
						else
							module->colorBoxG[column] = color;
					} else {
						module->colorBoxG[column] = 0;
					}
				}
			} else 
				module->colorBoxG[column] = 0;
		};
	};

	struct labelTextFieldB : TextField {
		DrumPlayerXtra *module;
		unsigned int column = 0;
		labelTextFieldB(unsigned int column) {
			this->column = column;
			this->box.pos.x = 30;
			this->box.size.x = 30;
			this->multiline = false;
		}

		void onChange(const event::Change& e) override {
			if (text != "") {
				if (text.length() > 3) {
					text = text.substr(0,3);
					rack::TextField::cursor = 3;
					rack::TextField::selection = 3;
				} else {
					if (all_of(text.begin(), text.end(), ::isdigit)) {
						int color = std::stoi(text);
						if (color < 0 && color < 256)
							module->colorBoxB[column] = 0;
						else if (color > 255)
							module->colorBoxB[column] = 255;
						else
							module->colorBoxB[column] = color;
					} else {
						module->colorBoxB[column] = 0;
					}
				}
			} else 
				module->colorBoxB[column] = 0;
		};
	};

	void onButton(const event::Button &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) {
			if (module->displayTrig)
				module->uiTrig[0] = true;
			e.consume(this);
		}

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void loadSubfolder(rack::ui::Menu *menu, std::string path) {
		DrumPlayerXtra *module = dynamic_cast<DrumPlayerXtra*>(this->module);
			assert(module);
		std::string currentDir = path;
		int tempIndex = 1;
		if (module->folderTreeData.size() < 2) {
			module->createFolder(currentDir.substr(0,currentDir.length()-1));
			module->folderTreeData.push_back(module->tempTreeData);
			module->folderTreeDisplay.push_back(module->tempTreeDisplay);
			module->folderTreeData[1][0] = currentDir;
			module->folderTreeDisplay[1][0] = currentDir;
		} else {
			bool exited = false;
			for (unsigned int i = 1 ; i < module->folderTreeData.size(); i++) {
				if (module->folderTreeData[i][0] == currentDir) {
					tempIndex = i;
					i = module->folderTreeData.size();
					exited = true;
				} 
			}
			if (!exited) {
				module->createFolder(currentDir);
				module->folderTreeData.push_back(module->tempTreeData);
				module->folderTreeDisplay.push_back(module->tempTreeDisplay);
				tempIndex = module->folderTreeData.size()-1;
			}	
		}
		if (module->folderTreeData[tempIndex].size() > 1) {
			for (unsigned int i = 1; i < module->folderTreeData[tempIndex].size(); i++) {
				if (module->folderTreeData[tempIndex][i].substr(module->folderTreeData[tempIndex][i].length()-1,module->folderTreeData[tempIndex][i].length()-1) == "/")  {
						module->tempDir = module->folderTreeData[tempIndex][i];
						menu->addChild(createSubmenuItem(module->folderTreeDisplay[tempIndex][i], "",
							[=](Menu* menu) {
								loadSubfolder(menu, module->folderTreeData[tempIndex][i]);
							}));
				} else {
					menu->addChild(createMenuItem(module->folderTreeDisplay[tempIndex][i], "", [=]() {module->loadFromPatch[0] = false;module->loadSample(module->folderTreeData[tempIndex][i],0);}));
				}
			}
		}
	}

	void createContextMenu() {
		DrumPlayerXtra *module = dynamic_cast<DrumPlayerXtra *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			menu->addChild(createMenuItem("Load Sample Slot #1", "", [=]() {
				bool temploadFromPatch = module->loadFromPatch[0];
				module->loadFromPatch[0] = false;
				module->menuLoadSample(0);
				if (module->restoreLoadFromPatch[0])
					module->loadFromPatch[0] = temploadFromPatch;
			}));

			if (module->folderTreeData.size() > 0) {
				menu->addChild(new MenuSeparator());
				menu->addChild(createSubmenuItem("Samples Browser", "", [=](Menu* menu) {
					//module->folderTreeData.resize(1);
					//module->folderTreeDisplay.resize(1);
					module->refreshRootFolder();
					for (unsigned int i = 1; i < module->folderTreeData[0].size(); i++) {
						if (module->folderTreeData[0][i].substr(module->folderTreeData[0][i].length()-1, module->folderTreeData[0][i].length()-1) == "/")  {
							module->tempDir = module->folderTreeData[0][i];
							menu->addChild(createSubmenuItem(module->folderTreeDisplay[0][i], "", [=](Menu* menu) {
								loadSubfolder(menu, module->folderTreeData[0][i]);
							}));
						} else {
							menu->addChild(createMenuItem(module->folderTreeDisplay[0][i], "", [=]() {module->loadFromPatch[0] = false;module->loadSample(module->folderTreeData[0][i],0);}));
						}
					}
				}));
			}
			if (module->fileLoaded[0]) {
				menu->addChild(new MenuSeparator());
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription[0]));
				menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot(0);}));
			}

			menu->addChild(new MenuSeparator());
			menu->addChild(createSubmenuItem("Zoom Waveform", "", [=](Menu* menu) {
				std::string zoomNames[4] = {"Full", "Half", "Quarter", "Eighth"};
				for (int i = 0; i < 4; i++) {
					ZoomItem* zoomItem = createMenuItem<ZoomItem>(zoomNames[i]);
					zoomItem->rightText = CHECKMARK(module->zoom[0] == i);
					zoomItem->module = module;
					zoomItem->zoom = i;
					menu->addChild(zoomItem);
				}
			}));

			if (module->lightBox) {
				menu->addChild(new MenuSeparator());
				menu->addChild(createSubmenuItem("Light Box Color", "", [=](Menu* menu) {
					std::string colorNames[5] = {"Blue", "Red", "Yellow", "Green", "Custom"};
					for (int i = 0; i < 5; i++) {
						ColorItem* colorItem = createMenuItem<ColorItem>(colorNames[i]);
						colorItem->rightText = CHECKMARK(module->colorBox[0] == i);
						colorItem->module = module;
						colorItem->colorBox = i;
						menu->addChild(colorItem);
					}
					menu->addChild(new MenuSeparator());
					menu->addChild(createMenuLabel("Custom Color"));

					auto holderR = new rack::Widget;
					holderR->box.size.x = 50;
					holderR->box.size.y = 20;
					auto labR = new rack::Label;
					labR->text = "R :";
					labR->box.size = 60;
					holderR->addChild(labR);
					auto textfieldR = new labelTextFieldR(0);
					textfieldR->module = module;
					textfieldR->text = to_string(module->colorBoxR[0]);
					holderR->addChild(textfieldR);
					menu->addChild(holderR);

					auto holderG = new rack::Widget;
					holderG->box.size.x = 30;
					holderG->box.size.y = 20;
					auto labG = new rack::Label;
					labG->text = "G :";
					labG->box.size = 40;
					holderG->addChild(labG);
					auto textfieldG = new labelTextFieldG(0);
					textfieldG->module = module;
					textfieldG->text = to_string(module->colorBoxG[0]);
					holderG->addChild(textfieldG);
					menu->addChild(holderG);

					auto holderB = new rack::Widget;
					holderB->box.size.x = 30;
					holderB->box.size.y = 20;
					auto labB = new rack::Label;
					labB->text = "B :";
					labB->box.size = 40;
					holderB->addChild(labB);
					auto textfieldB = new labelTextFieldB(0);
					textfieldB->module = module;
					textfieldB->text = to_string(module->colorBoxB[0]);
					holderB->addChild(textfieldB);
					menu->addChild(holderB);
				}));
				menu->addChild(createSubmenuItem("Light Box Fade", "", [=](Menu* menu) {
					std::string timeNames[3] = {"Slow (0.5s)", "Normal (0.25s)", "Fast (0.1s)"};
					for (int i = 0; i < 3; i++) {
						LightTimeItem* lightTimeItem = createMenuItem<LightTimeItem>(timeNames[i]);
						lightTimeItem->rightText = CHECKMARK(module->lightTime[0] == i);
						lightTimeItem->module = module;
						lightTimeItem->lightTime = i;
						menu->addChild(lightTimeItem);
					}
				}));
			}
			menu->addChild(new MenuSeparator());
			menu->addChild(createSubmenuItem("Swap Slot with", "", [=](Menu* menu) {
				menu->addChild(createMenuItem("Slot 2", "", [=]() {module->swapSlot(0, 1);}));
				menu->addChild(createMenuItem("Slot 3", "", [=]() {module->swapSlot(0, 2);}));
				menu->addChild(createMenuItem("Slot 4", "", [=]() {module->swapSlot(0, 3);}));
			}));
			menu->addChild(createSubmenuItem("Copy Slot to", "", [=](Menu* menu) {
				menu->addChild(createMenuItem("Slot 2", "", [=]() {module->copySlot(0, 1);}));
				menu->addChild(createMenuItem("Slot 3", "", [=]() {module->copySlot(0, 2);}));
				menu->addChild(createMenuItem("Slot 4", "", [=]() {module->copySlot(0, 3);}));
			}));
		}
	}
};

struct dpxSlot2Display : TransparentWidget {
	DrumPlayerXtra *module;
	int frame = 0;

	dpxSlot2Display() {

	}

	struct ColorItem : MenuItem {
		DrumPlayerXtra* module;
		int colorBox;
		void onAction(const event::Action& e) override {
			module->colorBox[1] = colorBox;
		}
	};

	struct ZoomItem : MenuItem {
		DrumPlayerXtra* module;
		int zoom;
		void onAction(const event::Action& e) override {
			module->zoom[1] = zoom;
			if (module->fileLoaded[1])
				module->displayRecalc(zoom, 1);
		}
	};

	struct LightTimeItem : MenuItem {
		DrumPlayerXtra* module;
		int lightTime;
		void onAction(const event::Action& e) override {
			module->lightTime[1] = lightTime;
		}
	};

	struct labelTextFieldR : TextField {
		DrumPlayerXtra *module;
		unsigned int column = 0;
		labelTextFieldR(unsigned int column) {
			this->column = column;
			this->box.pos.x = 30;
			this->box.size.x = 30;
			this->multiline = false;
		}

		void onChange(const event::Change& e) override {
			if (text != "") {
				if (text.length() > 3) {
					text = text.substr(0,3);
					rack::TextField::cursor = 3;
					rack::TextField::selection = 3;
				} else {
					if (all_of(text.begin(), text.end(), ::isdigit)) {
						int color = std::stoi(text);
						if (color < 0 && color < 256)
							module->colorBoxR[column] = 0;
						else if (color > 255)
							module->colorBoxR[column] = 255;
						else
							module->colorBoxR[column] = color;
					} else {
						module->colorBoxR[column] = 0;
					}
				}
			} else 
				module->colorBoxR[column] = 0;
		};
	};

	struct labelTextFieldG : TextField {
		DrumPlayerXtra *module;
		unsigned int column = 0;
		labelTextFieldG(unsigned int column) {
			this->column = column;
			this->box.pos.x = 30;
			this->box.size.x = 30;
			this->multiline = false;
		}

		void onChange(const event::Change& e) override {
			if (text != "") {
				if (text.length() > 3) {
					text = text.substr(0,3);
					rack::TextField::cursor = 3;
					rack::TextField::selection = 3;
				} else {
					if (all_of(text.begin(), text.end(), ::isdigit)) {
						int color = std::stoi(text);
						if (color < 0 && color < 256)
							module->colorBoxG[column] = 0;
						else if (color > 255)
							module->colorBoxG[column] = 255;
						else
							module->colorBoxG[column] = color;
					} else {
						module->colorBoxG[column] = 0;
					}
				}
			} else 
				module->colorBoxG[column] = 0;
		};
	};

	struct labelTextFieldB : TextField {
		DrumPlayerXtra *module;
		unsigned int column = 0;
		labelTextFieldB(unsigned int column) {
			this->column = column;
			this->box.pos.x = 30;
			this->box.size.x = 30;
			this->multiline = false;
		}

		void onChange(const event::Change& e) override {
			if (text != "") {
				if (text.length() > 3) {
					text = text.substr(0,3);
					rack::TextField::cursor = 3;
					rack::TextField::selection = 3;
				} else {
					if (all_of(text.begin(), text.end(), ::isdigit)) {
						int color = std::stoi(text);
						if (color < 0 && color < 256)
							module->colorBoxB[column] = 0;
						else if (color > 255)
							module->colorBoxB[column] = 255;
						else
							module->colorBoxB[column] = color;
					} else {
						module->colorBoxB[column] = 0;
					}
				}
			} else 
				module->colorBoxB[column] = 0;
		};
	};

	void onButton(const event::Button &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) {
			if (module->displayTrig)
				module->uiTrig[1] = true;
			e.consume(this);
		}

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void loadSubfolder(rack::ui::Menu *menu, std::string path) {
		DrumPlayerXtra *module = dynamic_cast<DrumPlayerXtra*>(this->module);
			assert(module);
		std::string currentDir = path;
		int tempIndex = 1;
		if (module->folderTreeData.size() < 2) {
			module->createFolder(currentDir.substr(0,currentDir.length()-1));
			module->folderTreeData.push_back(module->tempTreeData);
			module->folderTreeDisplay.push_back(module->tempTreeDisplay);
			module->folderTreeData[1][0] = currentDir;
			module->folderTreeDisplay[1][0] = currentDir;
		} else {
			bool exited = false;
			for (unsigned int i = 1 ; i < module->folderTreeData.size(); i++) {
				if (module->folderTreeData[i][0] == currentDir) {
					tempIndex = i;
					i = module->folderTreeData.size();
					exited = true;
				} 
			}
			if (!exited) {
				module->createFolder(currentDir);
				module->folderTreeData.push_back(module->tempTreeData);
				module->folderTreeDisplay.push_back(module->tempTreeDisplay);
				tempIndex = module->folderTreeData.size()-1;
			}	
		}
		if (module->folderTreeData[tempIndex].size() > 1) {
			for (unsigned int i = 1; i < module->folderTreeData[tempIndex].size(); i++) {
				if (module->folderTreeData[tempIndex][i].substr(module->folderTreeData[tempIndex][i].length()-1,module->folderTreeData[tempIndex][i].length()-1) == "/")  {
					module->tempDir = module->folderTreeData[tempIndex][i];
					menu->addChild(createSubmenuItem(module->folderTreeDisplay[tempIndex][i], "", [=](Menu* menu) {
						loadSubfolder(menu, module->folderTreeData[tempIndex][i]);
					}));
				} else {
					menu->addChild(createMenuItem(module->folderTreeDisplay[tempIndex][i], "", [=]() {module->loadFromPatch[1] = false;module->loadSample(module->folderTreeData[tempIndex][i],1);}));
				}
			}
		}
	}

	void createContextMenu() {
		DrumPlayerXtra *module = dynamic_cast<DrumPlayerXtra *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			menu->addChild(createMenuItem("Load Sample Slot #2", "", [=]() {
				//module->menuLoadSample(1);
				bool temploadFromPatch = module->loadFromPatch[1];
				module->loadFromPatch[1] = false;
				module->menuLoadSample(1);
				if (module->restoreLoadFromPatch[1])
					module->loadFromPatch[1] = temploadFromPatch;
			}));

			if (module->folderTreeData.size() > 0) {
				menu->addChild(new MenuSeparator());
				menu->addChild(createSubmenuItem("Samples Browser", "", [=](Menu* menu) {
					//module->folderTreeData.resize(1);
					//module->folderTreeDisplay.resize(1);
					module->refreshRootFolder();
					for (unsigned int i = 1; i < module->folderTreeData[0].size(); i++) {
						if (module->folderTreeData[0][i].substr(module->folderTreeData[0][i].length()-1, module->folderTreeData[0][i].length()-1) == "/")  {
							module->tempDir = module->folderTreeData[0][i];
							menu->addChild(createSubmenuItem(module->folderTreeDisplay[0][i], "", [=](Menu* menu) {
								loadSubfolder(menu, module->folderTreeData[0][i]);
							}));
						} else {
							menu->addChild(createMenuItem(module->folderTreeDisplay[0][i], "", [=]() {module->loadFromPatch[1] = false;module->loadSample(module->folderTreeData[0][i],1);}));
						}
					}
				}));
			}
			if (module->fileLoaded[1]) {
				menu->addChild(new MenuSeparator());
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription[1]));
				menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot(1);}));
			}

			menu->addChild(new MenuSeparator());
			menu->addChild(createSubmenuItem("Zoom Waveform", "", [=](Menu* menu) {
				std::string zoomNames[4] = {"Full", "Half", "Quarter", "Eighth"};
				for (int i = 0; i < 4; i++) {
					ZoomItem* zoomItem = createMenuItem<ZoomItem>(zoomNames[i]);
					zoomItem->rightText = CHECKMARK(module->zoom[1] == i);
					zoomItem->module = module;
					zoomItem->zoom = i;
					menu->addChild(zoomItem);
				}
			}));

			if (module->lightBox) {
				menu->addChild(new MenuSeparator());
				menu->addChild(createSubmenuItem("Light Box Color", "", [=](Menu* menu) {
					std::string colorNames[5] = {"Blue", "Red", "Yellow", "Green", "Custom"};
					for (int i = 0; i < 5; i++) {
						ColorItem* colorItem = createMenuItem<ColorItem>(colorNames[i]);
						colorItem->rightText = CHECKMARK(module->colorBox[1] == i);
						colorItem->module = module;
						colorItem->colorBox = i;
						menu->addChild(colorItem);
					}
					menu->addChild(new MenuSeparator());
					menu->addChild(createMenuLabel("Custom Color"));

					auto holderR = new rack::Widget;
					holderR->box.size.x = 50;
					holderR->box.size.y = 20;
					auto labR = new rack::Label;
					labR->text = "R :";
					labR->box.size = 60;
					holderR->addChild(labR);
					auto textfieldR = new labelTextFieldR(1);
					textfieldR->module = module;
					textfieldR->text = to_string(module->colorBoxR[1]);
					holderR->addChild(textfieldR);
					menu->addChild(holderR);

					auto holderG = new rack::Widget;
					holderG->box.size.x = 30;
					holderG->box.size.y = 20;
					auto labG = new rack::Label;
					labG->text = "G :";
					labG->box.size = 40;
					holderG->addChild(labG);
					auto textfieldG = new labelTextFieldG(1);
					textfieldG->module = module;
					textfieldG->text = to_string(module->colorBoxG[1]);
					holderG->addChild(textfieldG);
					menu->addChild(holderG);

					auto holderB = new rack::Widget;
					holderB->box.size.x = 30;
					holderB->box.size.y = 20;
					auto labB = new rack::Label;
					labB->text = "B :";
					labB->box.size = 40;
					holderB->addChild(labB);
					auto textfieldB = new labelTextFieldB(1);
					textfieldB->module = module;
					textfieldB->text = to_string(module->colorBoxB[1]);
					holderB->addChild(textfieldB);
					menu->addChild(holderB);
				}));
				menu->addChild(createSubmenuItem("Light Box Fade", "", [=](Menu* menu) {
					std::string timeNames[3] = {"Slow (0.5s)", "Normal (0.25s)", "Fast (0.1s)"};
					for (int i = 0; i < 3; i++) {
						LightTimeItem* lightTimeItem = createMenuItem<LightTimeItem>(timeNames[i]);
						lightTimeItem->rightText = CHECKMARK(module->lightTime[1] == i);
						lightTimeItem->module = module;
						lightTimeItem->lightTime = i;
						menu->addChild(lightTimeItem);
					}
				}));
			}
			menu->addChild(new MenuSeparator());
			menu->addChild(createSubmenuItem("Swap Slot with", "", [=](Menu* menu) {
				menu->addChild(createMenuItem("Slot 1", "", [=]() {module->swapSlot(1, 0);}));
				menu->addChild(createMenuItem("Slot 3", "", [=]() {module->swapSlot(1, 2);}));
				menu->addChild(createMenuItem("Slot 4", "", [=]() {module->swapSlot(1, 3);}));
			}));
			menu->addChild(createSubmenuItem("Copy Slot to", "", [=](Menu* menu) {
				menu->addChild(createMenuItem("Slot 1", "", [=]() {module->copySlot(1, 0);}));
				menu->addChild(createMenuItem("Slot 3", "", [=]() {module->copySlot(1, 2);}));
				menu->addChild(createMenuItem("Slot 4", "", [=]() {module->copySlot(1, 3);}));
			}));
		}
	}
};

struct dpxSlot3Display : TransparentWidget {
	DrumPlayerXtra *module;
	int frame = 0;

	dpxSlot3Display() {

	}

	struct ColorItem : MenuItem {
		DrumPlayerXtra* module;
		int colorBox;
		void onAction(const event::Action& e) override {
			module->colorBox[2] = colorBox;
		}
	};

	struct ZoomItem : MenuItem {
		DrumPlayerXtra* module;
		int zoom;
		void onAction(const event::Action& e) override {
			module->zoom[2] = zoom;
			if (module->fileLoaded[2])
				module->displayRecalc(zoom, 2);
		}
	};

	struct LightTimeItem : MenuItem {
		DrumPlayerXtra* module;
		int lightTime;
		void onAction(const event::Action& e) override {
			module->lightTime[2] = lightTime;
		}
	};

	struct labelTextFieldR : TextField {
		DrumPlayerXtra *module;
		unsigned int column = 0;
		labelTextFieldR(unsigned int column) {
			this->column = column;
			this->box.pos.x = 30;
			this->box.size.x = 30;
			this->multiline = false;
		}

		void onChange(const event::Change& e) override {
			if (text != "") {
				if (text.length() > 3) {
					text = text.substr(0,3);
					rack::TextField::cursor = 3;
					rack::TextField::selection = 3;
				} else {
					if (all_of(text.begin(), text.end(), ::isdigit)) {
						int color = std::stoi(text);
						if (color < 0 && color < 256)
							module->colorBoxR[column] = 0;
						else if (color > 255)
							module->colorBoxR[column] = 255;
						else
							module->colorBoxR[column] = color;
					} else {
						module->colorBoxR[column] = 0;
					}
				}
			} else 
				module->colorBoxR[column] = 0;
		};
	};

	struct labelTextFieldG : TextField {
		DrumPlayerXtra *module;
		unsigned int column = 0;
		labelTextFieldG(unsigned int column) {
			this->column = column;
			this->box.pos.x = 30;
			this->box.size.x = 30;
			this->multiline = false;
		}

		void onChange(const event::Change& e) override {
			if (text != "") {
				if (text.length() > 3) {
					text = text.substr(0,3);
					rack::TextField::cursor = 3;
					rack::TextField::selection = 3;
				} else {
					if (all_of(text.begin(), text.end(), ::isdigit)) {
						int color = std::stoi(text);
						if (color < 0 && color < 256)
							module->colorBoxG[column] = 0;
						else if (color > 255)
							module->colorBoxG[column] = 255;
						else
							module->colorBoxG[column] = color;
					} else {
						module->colorBoxG[column] = 0;
					}
				}
			} else 
				module->colorBoxG[column] = 0;
		};
	};

	struct labelTextFieldB : TextField {
		DrumPlayerXtra *module;
		unsigned int column = 0;
		labelTextFieldB(unsigned int column) {
			this->column = column;
			this->box.pos.x = 30;
			this->box.size.x = 30;
			this->multiline = false;
		}

		void onChange(const event::Change& e) override {
			if (text != "") {
				if (text.length() > 3) {
					text = text.substr(0,3);
					rack::TextField::cursor = 3;
					rack::TextField::selection = 3;
				} else {
					if (all_of(text.begin(), text.end(), ::isdigit)) {
						int color = std::stoi(text);
						if (color < 0 && color < 256)
							module->colorBoxB[column] = 0;
						else if (color > 255)
							module->colorBoxB[column] = 255;
						else
							module->colorBoxB[column] = color;
					} else {
						module->colorBoxB[column] = 0;
					}
				}
			} else 
				module->colorBoxB[column] = 0;
		};
	};

	void onButton(const event::Button &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) {
			if (module->displayTrig)
				module->uiTrig[2] = true;
			e.consume(this);
		}

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void loadSubfolder(rack::ui::Menu *menu, std::string path) {
		DrumPlayerXtra *module = dynamic_cast<DrumPlayerXtra*>(this->module);
			assert(module);
		std::string currentDir = path;
		int tempIndex = 1;
		if (module->folderTreeData.size() < 2) {
			module->createFolder(currentDir.substr(0,currentDir.length()-1));
			module->folderTreeData.push_back(module->tempTreeData);
			module->folderTreeDisplay.push_back(module->tempTreeDisplay);
			module->folderTreeData[1][0] = currentDir;
			module->folderTreeDisplay[1][0] = currentDir;
		} else {
			bool exited = false;
			for (unsigned int i = 1 ; i < module->folderTreeData.size(); i++) {
				if (module->folderTreeData[i][0] == currentDir) {
					tempIndex = i;
					i = module->folderTreeData.size();
					exited = true;
				} 
			}
			if (!exited) {
				module->createFolder(currentDir);
				module->folderTreeData.push_back(module->tempTreeData);
				module->folderTreeDisplay.push_back(module->tempTreeDisplay);
				tempIndex = module->folderTreeData.size()-1;
			}	
		}
		if (module->folderTreeData[tempIndex].size() > 1) {
			for (unsigned int i = 1; i < module->folderTreeData[tempIndex].size(); i++) {
				if (module->folderTreeData[tempIndex][i].substr(module->folderTreeData[tempIndex][i].length()-1,module->folderTreeData[tempIndex][i].length()-1) == "/")  {
					module->tempDir = module->folderTreeData[tempIndex][i];
					menu->addChild(createSubmenuItem(module->folderTreeDisplay[tempIndex][i], "", [=](Menu* menu) {
						loadSubfolder(menu, module->folderTreeData[tempIndex][i]);
					}));
				} else {
					menu->addChild(createMenuItem(module->folderTreeDisplay[tempIndex][i], "", [=]() {module->loadFromPatch[2] = false;module->loadSample(module->folderTreeData[tempIndex][i],2);}));
				}
			}
		}
	}

	void createContextMenu() {
		DrumPlayerXtra *module = dynamic_cast<DrumPlayerXtra *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			menu->addChild(createMenuItem("Load Sample Slot #3", "", [=]() {
				//module->menuLoadSample(2);
				bool temploadFromPatch = module->loadFromPatch[2];
				module->loadFromPatch[2] = false;
				module->menuLoadSample(2);
				if (module->restoreLoadFromPatch[2])
					module->loadFromPatch[2] = temploadFromPatch;
			}));

			if (module->folderTreeData.size() > 0) {
				menu->addChild(new MenuSeparator());
				menu->addChild(createSubmenuItem("Samples Browser", "", [=](Menu* menu) {
					//module->folderTreeData.resize(1);
					//module->folderTreeDisplay.resize(1);
					module->refreshRootFolder();
					for (unsigned int i = 1; i < module->folderTreeData[0].size(); i++) {
						if (module->folderTreeData[0][i].substr(module->folderTreeData[0][i].length()-1, module->folderTreeData[0][i].length()-1) == "/")  {
							module->tempDir = module->folderTreeData[0][i];
							menu->addChild(createSubmenuItem(module->folderTreeDisplay[0][i], "", [=](Menu* menu) {
								loadSubfolder(menu, module->folderTreeData[0][i]);
							}));
						} else {
							menu->addChild(createMenuItem(module->folderTreeDisplay[0][i], "", [=]() {module->loadFromPatch[2] = false;module->loadSample(module->folderTreeData[0][i],2);}));
						}
					}
				}));
			}
			if (module->fileLoaded[2]) {
				menu->addChild(new MenuSeparator());
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription[2]));
				menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot(2);}));
			}

			menu->addChild(new MenuSeparator());
			menu->addChild(createSubmenuItem("Zoom Waveform", "", [=](Menu* menu) {
				std::string zoomNames[4] = {"Full", "Half", "Quarter", "Eighth"};
				for (int i = 0; i < 4; i++) {
					ZoomItem* zoomItem = createMenuItem<ZoomItem>(zoomNames[i]);
					zoomItem->rightText = CHECKMARK(module->zoom[2] == i);
					zoomItem->module = module;
					zoomItem->zoom = i;
					menu->addChild(zoomItem);
				}
			}));

			if (module->lightBox) {
				menu->addChild(new MenuSeparator());
				menu->addChild(createSubmenuItem("Light Box Color", "", [=](Menu* menu) {
					std::string colorNames[5] = {"Blue", "Red", "Yellow", "Green", "Custom"};
					for (int i = 0; i < 5; i++) {
						ColorItem* colorItem = createMenuItem<ColorItem>(colorNames[i]);
						colorItem->rightText = CHECKMARK(module->colorBox[2] == i);
						colorItem->module = module;
						colorItem->colorBox = i;
						menu->addChild(colorItem);
					}
					menu->addChild(new MenuSeparator());
					menu->addChild(createMenuLabel("Custom Color"));

					auto holderR = new rack::Widget;
					holderR->box.size.x = 50;
					holderR->box.size.y = 20;
					auto labR = new rack::Label;
					labR->text = "R :";
					labR->box.size = 60;
					holderR->addChild(labR);
					auto textfieldR = new labelTextFieldR(2);
					textfieldR->module = module;
					textfieldR->text = to_string(module->colorBoxR[2]);
					holderR->addChild(textfieldR);
					menu->addChild(holderR);

					auto holderG = new rack::Widget;
					holderG->box.size.x = 30;
					holderG->box.size.y = 20;
					auto labG = new rack::Label;
					labG->text = "G :";
					labG->box.size = 40;
					holderG->addChild(labG);
					auto textfieldG = new labelTextFieldG(2);
					textfieldG->module = module;
					textfieldG->text = to_string(module->colorBoxG[2]);
					holderG->addChild(textfieldG);
					menu->addChild(holderG);

					auto holderB = new rack::Widget;
					holderB->box.size.x = 30;
					holderB->box.size.y = 20;
					auto labB = new rack::Label;
					labB->text = "B :";
					labB->box.size = 40;
					holderB->addChild(labB);
					auto textfieldB = new labelTextFieldB(2);
					textfieldB->module = module;
					textfieldB->text = to_string(module->colorBoxB[2]);
					holderB->addChild(textfieldB);
					menu->addChild(holderB);
				}));
				menu->addChild(createSubmenuItem("Light Box Fade", "", [=](Menu* menu) {
					std::string timeNames[3] = {"Slow (0.5s)", "Normal (0.25s)", "Fast (0.1s)"};
					for (int i = 0; i < 3; i++) {
						LightTimeItem* lightTimeItem = createMenuItem<LightTimeItem>(timeNames[i]);
						lightTimeItem->rightText = CHECKMARK(module->lightTime[2] == i);
						lightTimeItem->module = module;
						lightTimeItem->lightTime = i;
						menu->addChild(lightTimeItem);
					}
				}));
			}
			menu->addChild(new MenuSeparator());
			menu->addChild(createSubmenuItem("Swap Slot with", "", [=](Menu* menu) {
				menu->addChild(createMenuItem("Slot 1", "", [=]() {module->swapSlot(2, 0);}));
				menu->addChild(createMenuItem("Slot 2", "", [=]() {module->swapSlot(2, 1);}));
				menu->addChild(createMenuItem("Slot 4", "", [=]() {module->swapSlot(2, 3);}));
			}));
			menu->addChild(createSubmenuItem("Copy Slot to", "", [=](Menu* menu) {
				menu->addChild(createMenuItem("Slot 1", "", [=]() {module->copySlot(2, 0);}));
				menu->addChild(createMenuItem("Slot 2", "", [=]() {module->copySlot(2, 1);}));
				menu->addChild(createMenuItem("Slot 4", "", [=]() {module->copySlot(2, 3);}));
			}));
		}
	}
};

struct dpxSlot4Display : TransparentWidget {
	DrumPlayerXtra *module;
	int frame = 0;

	dpxSlot4Display() {

	}

	struct ColorItem : MenuItem {
		DrumPlayerXtra* module;
		int colorBox;
		void onAction(const event::Action& e) override {
			module->colorBox[3] = colorBox;
		}
	};

	struct ZoomItem : MenuItem {
		DrumPlayerXtra* module;
		int zoom;
		void onAction(const event::Action& e) override {
			module->zoom[3] = zoom;
			if (module->fileLoaded[3])
				module->displayRecalc(zoom, 3);
		}
	};

	struct LightTimeItem : MenuItem {
		DrumPlayerXtra* module;
		int lightTime;
		void onAction(const event::Action& e) override {
			module->lightTime[3] = lightTime;
		}
	};

	struct labelTextFieldR : TextField {
		DrumPlayerXtra *module;
		unsigned int column = 0;
		labelTextFieldR(unsigned int column) {
			this->column = column;
			this->box.pos.x = 30;
			this->box.size.x = 30;
			this->multiline = false;
		}

		void onChange(const event::Change& e) override {
			if (text != "") {
				if (text.length() > 3) {
					text = text.substr(0,3);
					rack::TextField::cursor = 3;
					rack::TextField::selection = 3;
				} else {
					if (all_of(text.begin(), text.end(), ::isdigit)) {
						int color = std::stoi(text);
						if (color < 0 && color < 256)
							module->colorBoxR[column] = 0;
						else if (color > 255)
							module->colorBoxR[column] = 255;
						else
							module->colorBoxR[column] = color;
					} else {
						module->colorBoxR[column] = 0;
					}
				}
			} else 
				module->colorBoxR[column] = 0;
		};
	};

	struct labelTextFieldG : TextField {
		DrumPlayerXtra *module;
		unsigned int column = 0;
		labelTextFieldG(unsigned int column) {
			this->column = column;
			this->box.pos.x = 30;
			this->box.size.x = 30;
			this->multiline = false;
		}

		void onChange(const event::Change& e) override {
			if (text != "") {
				if (text.length() > 3) {
					text = text.substr(0,3);
					rack::TextField::cursor = 3;
					rack::TextField::selection = 3;
				} else {
					if (all_of(text.begin(), text.end(), ::isdigit)) {
						int color = std::stoi(text);
						if (color < 0 && color < 256)
							module->colorBoxG[column] = 0;
						else if (color > 255)
							module->colorBoxG[column] = 255;
						else
							module->colorBoxG[column] = color;
					} else {
						module->colorBoxG[column] = 0;
					}
				}
			} else 
				module->colorBoxG[column] = 0;
		};
	};

	struct labelTextFieldB : TextField {
		DrumPlayerXtra *module;
		unsigned int column = 0;
		labelTextFieldB(unsigned int column) {
			this->column = column;
			this->box.pos.x = 30;
			this->box.size.x = 30;
			this->multiline = false;
		}

		void onChange(const event::Change& e) override {
			if (text != "") {
				if (text.length() > 3) {
					text = text.substr(0,3);
					rack::TextField::cursor = 3;
					rack::TextField::selection = 3;
				} else {
					if (all_of(text.begin(), text.end(), ::isdigit)) {
						int color = std::stoi(text);
						if (color < 0 && color < 256)
							module->colorBoxB[column] = 0;
						else if (color > 255)
							module->colorBoxB[column] = 255;
						else
							module->colorBoxB[column] = color;
					} else {
						module->colorBoxB[column] = 0;
					}
				}
			} else 
				module->colorBoxB[column] = 0;
		};
	};

	void onButton(const event::Button &e) override {
		if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) {
			if (module->displayTrig)
				module->uiTrig[3] = true;
			e.consume(this);
		}

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void loadSubfolder(rack::ui::Menu *menu, std::string path) {
		DrumPlayerXtra *module = dynamic_cast<DrumPlayerXtra*>(this->module);
			assert(module);
		std::string currentDir = path;
		int tempIndex = 1;
		if (module->folderTreeData.size() < 2) {
			module->createFolder(currentDir.substr(0,currentDir.length()-1));
			module->folderTreeData.push_back(module->tempTreeData);
			module->folderTreeDisplay.push_back(module->tempTreeDisplay);
			module->folderTreeData[1][0] = currentDir;
			module->folderTreeDisplay[1][0] = currentDir;
		} else {
			bool exited = false;
			for (unsigned int i = 1 ; i < module->folderTreeData.size(); i++) {
				if (module->folderTreeData[i][0] == currentDir) {
					tempIndex = i;
					i = module->folderTreeData.size();
					exited = true;
				} 
			}
			if (!exited) {
				module->createFolder(currentDir);
				module->folderTreeData.push_back(module->tempTreeData);
				module->folderTreeDisplay.push_back(module->tempTreeDisplay);
				tempIndex = module->folderTreeData.size()-1;
			}	
		}
		if (module->folderTreeData[tempIndex].size() > 1) {
			for (unsigned int i = 1; i < module->folderTreeData[tempIndex].size(); i++) {
				if (module->folderTreeData[tempIndex][i].substr(module->folderTreeData[tempIndex][i].length()-1,module->folderTreeData[tempIndex][i].length()-1) == "/")  {
					module->tempDir = module->folderTreeData[tempIndex][i];
					menu->addChild(createSubmenuItem(module->folderTreeDisplay[tempIndex][i], "", [=](Menu* menu) {
						loadSubfolder(menu, module->folderTreeData[tempIndex][i]);
					}));
				} else {
					menu->addChild(createMenuItem(module->folderTreeDisplay[tempIndex][i], "", [=]() {module->loadFromPatch[3] = false;module->loadSample(module->folderTreeData[tempIndex][i],3);}));
				}
			}
		}
	}

	void createContextMenu() {
		DrumPlayerXtra *module = dynamic_cast<DrumPlayerXtra *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			menu->addChild(createMenuItem("Load Sample Slot #4", "", [=]() {
				//module->menuLoadSample(3);
				bool temploadFromPatch = module->loadFromPatch[3];
				module->loadFromPatch[3] = false;
				module->menuLoadSample(3);
				if (module->restoreLoadFromPatch[3])
					module->loadFromPatch[3] = temploadFromPatch;
			}));

			if (module->folderTreeData.size() > 0) {
				menu->addChild(new MenuSeparator());
				menu->addChild(createSubmenuItem("Samples Browser", "", [=](Menu* menu) {
					//module->folderTreeData.resize(1);
					//module->folderTreeDisplay.resize(1);
					module->refreshRootFolder();
					for (unsigned int i = 1; i < module->folderTreeData[0].size(); i++) {
						if (module->folderTreeData[0][i].substr(module->folderTreeData[0][i].length()-1, module->folderTreeData[0][i].length()-1) == "/")  {
							module->tempDir = module->folderTreeData[0][i];
							menu->addChild(createSubmenuItem(module->folderTreeDisplay[0][i], "", [=](Menu* menu) {
								loadSubfolder(menu, module->folderTreeData[0][i]);
							}));
						} else {
							menu->addChild(createMenuItem(module->folderTreeDisplay[0][i], "", [=]() {module->loadFromPatch[3] = false;module->loadSample(module->folderTreeData[0][i],3);}));
						}
					}
				}));
			}
			if (module->fileLoaded[3]) {
				menu->addChild(new MenuSeparator());
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription[3]));
				menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot(3);}));
			}

			menu->addChild(new MenuSeparator());
			menu->addChild(createSubmenuItem("Zoom Waveform", "", [=](Menu* menu) {
				std::string zoomNames[4] = {"Full", "Half", "Quarter", "Eighth"};
				for (int i = 0; i < 4; i++) {
					ZoomItem* zoomItem = createMenuItem<ZoomItem>(zoomNames[i]);
					zoomItem->rightText = CHECKMARK(module->zoom[3] == i);
					zoomItem->module = module;
					zoomItem->zoom = i;
					menu->addChild(zoomItem);
				}
			}));

			if (module->lightBox) {
				menu->addChild(new MenuSeparator());
				menu->addChild(createSubmenuItem("Light Box Color", "", [=](Menu* menu) {
					std::string colorNames[5] = {"Blue", "Red", "Yellow", "Green", "Custom"};
					for (int i = 0; i < 5; i++) {
						ColorItem* colorItem = createMenuItem<ColorItem>(colorNames[i]);
						colorItem->rightText = CHECKMARK(module->colorBox[3] == i);
						colorItem->module = module;
						colorItem->colorBox = i;
						menu->addChild(colorItem);
					}
					menu->addChild(new MenuSeparator());
					menu->addChild(createMenuLabel("Custom Color"));

					auto holderR = new rack::Widget;
					holderR->box.size.x = 50;
					holderR->box.size.y = 20;
					auto labR = new rack::Label;
					labR->text = "R :";
					labR->box.size = 60;
					holderR->addChild(labR);
					auto textfieldR = new labelTextFieldR(3);
					textfieldR->module = module;
					textfieldR->text = to_string(module->colorBoxR[3]);
					holderR->addChild(textfieldR);
					menu->addChild(holderR);

					auto holderG = new rack::Widget;
					holderG->box.size.x = 30;
					holderG->box.size.y = 20;
					auto labG = new rack::Label;
					labG->text = "G :";
					labG->box.size = 40;
					holderG->addChild(labG);
					auto textfieldG = new labelTextFieldG(3);
					textfieldG->module = module;
					textfieldG->text = to_string(module->colorBoxG[3]);
					holderG->addChild(textfieldG);
					menu->addChild(holderG);

					auto holderB = new rack::Widget;
					holderB->box.size.x = 30;
					holderB->box.size.y = 20;
					auto labB = new rack::Label;
					labB->text = "B :";
					labB->box.size = 40;
					holderB->addChild(labB);
					auto textfieldB = new labelTextFieldB(3);
					textfieldB->module = module;
					textfieldB->text = to_string(module->colorBoxB[3]);
					holderB->addChild(textfieldB);
					menu->addChild(holderB);
				}));
				menu->addChild(createSubmenuItem("Light Box Fade", "", [=](Menu* menu) {
					std::string timeNames[3] = {"Slow (0.5s)", "Normal (0.25s)", "Fast (0.1s)"};
					for (int i = 0; i < 3; i++) {
						LightTimeItem* lightTimeItem = createMenuItem<LightTimeItem>(timeNames[i]);
						lightTimeItem->rightText = CHECKMARK(module->lightTime[3] == i);
						lightTimeItem->module = module;
						lightTimeItem->lightTime = i;
						menu->addChild(lightTimeItem);
					}
				}));
			}
			menu->addChild(new MenuSeparator());
			menu->addChild(createSubmenuItem("Swap Slot with", "", [=](Menu* menu) {
				menu->addChild(createMenuItem("Slot 1", "", [=]() {module->swapSlot(3, 0);}));
				menu->addChild(createMenuItem("Slot 2", "", [=]() {module->swapSlot(3, 1);}));
				menu->addChild(createMenuItem("Slot 3", "", [=]() {module->swapSlot(3, 2);}));
			}));
			menu->addChild(createSubmenuItem("Copy Slot to", "", [=](Menu* menu) {
				menu->addChild(createMenuItem("Slot 1", "", [=]() {module->copySlot(3, 0);}));
				menu->addChild(createMenuItem("Slot 2", "", [=]() {module->copySlot(3, 1);}));
				menu->addChild(createMenuItem("Slot 3", "", [=]() {module->copySlot(3, 2);}));
			}));
		}
	}
};

struct DrumPlayerXtraDisplay : TransparentWidget {
	DrumPlayerXtra *module;
	int frame = 0;

	int fileGap[4] = {0,0,0,0};
	float currTime;
	float deltaTime;
	float prevTime;

	float deltaBoxTime[4];
	float startBoxTime[4];
	int currAlpha[4] = {0,0,0,0};
	float maxAlpha[4];

	const int boxX[4] = {2, 87, 172, 258};
	const int boxY = 13;
	const int boxW = 59;
	const int boxH = 30;

	const int colorR[4] = {0,255,255,0};
	const int colorG[4] = {0,0,255,255};
	const int colorB[4] = {255,0,0,0};

	const float slotXpos[4] = {2,87,172,258};

	const float lightTimeDur[3] = {0.5f, 0.25f, 0.1f};

	DrumPlayerXtraDisplay() {

	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/DSEG7ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgFillColor(args.vg, nvgRGBA(0xdd, 0x33, 0x33, 0xff)); 

				currTime = system::getTime();
				switch (module->scrolling) {
					case 0:
						for (int i = 0; i < 4; i++) {
							module->currFileDisplay[i] = module->fileDisplay[i].substr(0,7);
						}
					break;
					case 1:
						deltaTime = currTime - prevTime;
						for (int i = 0; i < 4; i++) {
							if (module->scrollDisplay[i].length() > 6) {
								if (deltaTime > 0.4f) {
									prevTime = currTime;
									if (fileGap[i] > int(module->scrollDisplay[i].length())-7)
										fileGap[i] = 0;
									module->currFileDisplay[i] = module->scrollDisplay[i].substr(fileGap[i],7);
									fileGap[i]++;
								}
							} else {
								module->currFileDisplay[i] = module->scrollDisplay[i].substr(0,7);
							}
						}
					break;
				}

				nvgTextBox(args.vg, 3, 12,120, module->currFileDisplay[0].c_str(), NULL);
				nvgTextBox(args.vg, 88, 12,120, module->currFileDisplay[1].c_str(), NULL);
				nvgTextBox(args.vg, 173, 12,120, module->currFileDisplay[2].c_str(), NULL);
				nvgTextBox(args.vg, 259, 12,120, module->currFileDisplay[3].c_str(), NULL);
				//nvgTextBox(args.vg, 2, -20,120, module->debugDisplay.c_str(), NULL);

				for (int slot=0; slot < 4; slot++) {
					nvgStrokeColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x40));
					{
						nvgBeginPath(args.vg);
						nvgMoveTo(args.vg, slotXpos[slot], 28);
						nvgLineTo(args.vg, slotXpos[slot]+59, 28);
						nvgClosePath(args.vg);
					}
					nvgStroke(args.vg);
					
					if (module->fileLoaded[slot]) {						
						// Waveform
						nvgStrokeColor(args.vg, nvgRGBA(0x22, 0x44, 0xc9, 0xc0));
						nvgSave(args.vg);
						Rect b = Rect(Vec(slotXpos[slot], 13), Vec(59, 30));
						nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
						nvgBeginPath(args.vg);
						for (unsigned int i = 0; i < module->displayBuff[slot].size(); i++) {
							float x, y;
							x = (float)i / (module->displayBuff[slot].size() - 1);
							y = module->displayBuff[slot][i] / module->displayCoeff[slot] + 0.5;
							
							Vec p;
							p.x = b.pos.x + b.size.x * x;
							p.y = b.pos.y + b.size.y * (1.0 - y);
							if (i == 0)
								nvgMoveTo(args.vg, p.x, p.y);
							else
								nvgLineTo(args.vg, p.x, p.y);
						}
						nvgLineCap(args.vg, NVG_ROUND);
						nvgMiterLimit(args.vg, 2.0);
						nvgStrokeWidth(args.vg, 1.5);
						nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
						nvgStroke(args.vg);			
						nvgResetScissor(args.vg);
						nvgRestore(args.vg);	
					}

					// lightbox management
					if (module->lightBox) {
						if (module->slotTriggered[slot]) {
							maxAlpha[slot] = 55 + (100 * module->level[slot]);
							module->slotTriggered[slot] = false;
							currAlpha[slot] = maxAlpha[slot];
							startBoxTime[slot] = currTime;
							deltaBoxTime[slot] = 0;
						}
						if (currAlpha[slot] > 0) {
							nvgBeginPath(args.vg);
							nvgRoundedRect(args.vg, boxX[slot],boxY, boxW, boxH, 4);
							if (module->colorBox[slot] == 4)
								nvgFillColor(args.vg, nvgRGBA(module->colorBoxR[slot], module->colorBoxG[slot], module->colorBoxB[slot], currAlpha[slot]));
							else
								nvgFillColor(args.vg, nvgRGBA(colorR[module->colorBox[slot]], colorG[module->colorBox[slot]], colorB[module->colorBox[slot]], currAlpha[slot]));
							nvgFill(args.vg);
							deltaBoxTime[slot] = currTime - startBoxTime[slot];
							currAlpha[slot] = int((lightTimeDur[module->lightTime[slot]] - deltaBoxTime[slot]) * maxAlpha[slot] / lightTimeDur[module->lightTime[slot]]);
						}
					}
				}
			}
		}
		Widget::drawLayer(args, layer);
	}
};


struct DrumPlayerXtraWidget : ModuleWidget {
	DrumPlayerXtraWidget(DrumPlayerXtra *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DrumPlayerXtra.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	  

		{
			dpxSlot1Display *display1 = new dpxSlot1Display();
			display1->box.pos = Vec(12, 27);
			display1->box.size = Vec(65, 47);
			display1->module = module;
			addChild(display1);
		}

		{
			dpxSlot2Display *display2 = new dpxSlot2Display();
			display2->box.pos = Vec(98, 27);
			display2->box.size = Vec(65, 47);
			display2->module = module;
			addChild(display2);
		}

		{
			dpxSlot3Display *display3 = new dpxSlot3Display();
			display3->box.pos = Vec(183, 27);
			display3->box.size = Vec(65, 47);
			display3->module = module;
			addChild(display3);
		}

		{
			dpxSlot4Display *display4 = new dpxSlot4Display();
			display4->box.pos = Vec(269, 27);
			display4->box.size = Vec(65, 47);
			display4->module = module;
			addChild(display4);
		}
	    
		{
			DrumPlayerXtraDisplay *display = new DrumPlayerXtraDisplay();
			display->box.pos = Vec(13, 28);
			display->box.size = Vec(270, 100);
			display->module = module;
			addChild(display);
		}

		const float xDelta = 28.95;
		const float trigPos = 8.4;
		const float accPos = 21.5;

		const float trigInputPos = 39.5;
		const float trigVolPos = 50.2;
		const float trigVolAtnvPos = 61.9;
		const float trigVolInputPos = 70.5;

		const float buttonsYpos = 108.55;

		for (int i = 0; i < 4; i++) {
			addParam(createParamCentered<VCVButton>(mm2px(Vec(9.8+(xDelta*i), 28.4)), module, DrumPlayerXtra::PREVSAMPLE_PARAM+i));
			addParam(createParamCentered<VCVButton>(mm2px(Vec(20+(xDelta*i), 28.4)), module, DrumPlayerXtra::NEXTSAMPLE_PARAM+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(trigPos+(xDelta*i), trigInputPos)), module, DrumPlayerXtra::TRIG_INPUT+i));
			addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(trigPos+(xDelta*i), trigVolPos)), module, DrumPlayerXtra::TRIGVOL_PARAM+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(trigPos+(xDelta*i), trigVolAtnvPos)), module, DrumPlayerXtra::TRIGVOLATNV_PARAM+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(trigPos+(xDelta*i), trigVolInputPos)), module, DrumPlayerXtra::TRIGVOL_INPUT+i));

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(accPos+(xDelta*i), trigInputPos)), module, DrumPlayerXtra::ACC_INPUT+i));
			addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(accPos+(xDelta*i), trigVolPos)), module, DrumPlayerXtra::ACCVOL_PARAM+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(accPos+(xDelta*i), trigVolAtnvPos)), module, DrumPlayerXtra::ACCVOLATNV_PARAM+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(accPos+(xDelta*i), trigVolInputPos)), module, DrumPlayerXtra::ACCVOL_INPUT+i));

			addParam(createParamCentered<SickoKnob>(mm2px(Vec(14.9+(xDelta*i), 86)), module, DrumPlayerXtra::SPEED_PARAM+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(9.9+(xDelta*i), 95.5)), module, DrumPlayerXtra::SPEED_INPUT+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(19.9+(xDelta*i), 95.5)), module, DrumPlayerXtra::SPEEDATNV_PARAM+i));

			if (i<3) {
				addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(9.9+(xDelta*i), buttonsYpos)), module, DrumPlayerXtra::LIMIT_PARAMS+i, DrumPlayerXtra::LIMIT_LIGHT+i));
				addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(19.95+(xDelta*i), buttonsYpos)), module, DrumPlayerXtra::CHOKE_PARAMS+i, DrumPlayerXtra::CHOKE_LIGHT+i));
			} else {
				addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(5+9.9+(xDelta*i), buttonsYpos)), module, DrumPlayerXtra::LIMIT_PARAMS+i, DrumPlayerXtra::LIMIT_LIGHT+i));
			}

			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(18.9+(xDelta*i), 117.5)), module, DrumPlayerXtra::OUT_OUTPUT+i));
		}
	}

	void appendContextMenu(Menu *menu) override {
	   	DrumPlayerXtra *module = dynamic_cast<DrumPlayerXtra*>(this->module);
			assert(module);
		
		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Slots"));
		menu->addChild(createMenuItem("1: " + module->fileDescription[0], "", [=]() {
			//module->menuLoadSample(0);
			bool temploadFromPatch = module->loadFromPatch[0];
			module->loadFromPatch[0] = false;
			module->menuLoadSample(0);
			if (module->restoreLoadFromPatch[0])
				module->loadFromPatch[0] = temploadFromPatch;
		}));
		menu->addChild(createMenuItem("2: " + module->fileDescription[1], "", [=]() {
			//module->menuLoadSample(1);
			bool temploadFromPatch = module->loadFromPatch[1];
			module->loadFromPatch[1] = false;
			module->menuLoadSample(1);
			if (module->restoreLoadFromPatch[1])
				module->loadFromPatch[1] = temploadFromPatch;
		}));
		menu->addChild(createMenuItem("3: " + module->fileDescription[2], "", [=]() {
			//module->menuLoadSample(2);
			bool temploadFromPatch = module->loadFromPatch[0];
			module->loadFromPatch[2] = false;
			module->menuLoadSample(2);
			if (module->restoreLoadFromPatch[2])
				module->loadFromPatch[2] = temploadFromPatch;
		}));
		menu->addChild(createMenuItem("4: " + module->fileDescription[3], "", [=]() {
			//module->menuLoadSample(3);
			bool temploadFromPatch = module->loadFromPatch[3];
			module->loadFromPatch[3] = false;
			module->menuLoadSample(3);
			if (module->restoreLoadFromPatch[3])
				module->loadFromPatch[3] = temploadFromPatch;
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Select Samples Root", "", [=]() {module->selectRootFolder();}));

		if (module->userFolder != "") {
			if (module->rootFound) {
				menu->addChild(createMenuLabel(module->userFolder));
				//menu->addChild(createMenuItem("", "Refresh", [=]() {module->refreshRootFolder();}));
				menu->addChild(createSubmenuItem("Randomize", "", [=](Menu* menu) {
					menu->addChild(createMenuItem("All slots", "", [=]() {module->randomizeAllSlots();}));
					menu->addChild(createMenuItem("Slot 1", "", [=]() {module->randomizeSlot(0);}));
					menu->addChild(createMenuItem("Slot 2", "", [=]() {module->randomizeSlot(1);}));
					menu->addChild(createMenuItem("Slot 3", "", [=]() {module->randomizeSlot(2);}));
					menu->addChild(createMenuItem("Slot 4", "", [=]() {module->randomizeSlot(3);}));
				}));
			} else {
				menu->addChild(createMenuLabel("(!)"+module->userFolder));
			}
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Interpolation"));
		struct ModeItem : MenuItem {
			DrumPlayerXtra* module;
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
			DrumPlayerXtra* module;
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
		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Disable NAV Buttons", "", &module->disableNav));
		menu->addChild(createBoolPtrMenuItem("Scrolling sample names", "", &module->scrolling));
		menu->addChild(createBoolPtrMenuItem("Light Boxes", "", &module->lightBox));
		menu->addChild(createBoolPtrMenuItem("Display triggering", "", &module->displayTrig));

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Global settings", "", [=](Menu* menu) {
			menu->addChild(createMenuItem("Clear ALL samples", "", [=]() {
				for (int i = 0; i < 4; i++)
					module->clearSlot(i);
			}));
			menu->addChild(createMenuItem("Clear Root Folder", "", [=]() {
				module->folderTreeData.clear();
				module->folderTreeDisplay.clear();
				module->userFolder = "";
			}));

			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuLabel("Zoom All Samples"));
			menu->addChild(createMenuItem("Reset to Full", "", [=]() {
				for (int i = 0; i < 4; i++) {
					module->zoom[i] = 0;
					module->displayRecalc(0, i);
				}
			}));
			menu->addChild(createMenuItem("Set to Half", "", [=]() {
				for (int i = 0; i < 4; i++) {
					module->zoom[i] = 1;
					module->displayRecalc(1, i);
				}
			}));
			menu->addChild(createMenuItem("Set to Quarter", "", [=]() {
				for (int i = 0; i < 4; i++) {
					module->zoom[i] = 2;
					module->displayRecalc(2, i);
				}
			}));
			menu->addChild(createMenuItem("Set to Eighth", "", [=]() {
				for (int i = 0; i < 4; i++) {
					module->zoom[i] = 3;
					module->displayRecalc(3, i);
				}
			}));

			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuLabel("Lightboxes Color"));
			menu->addChild(createMenuItem("Reset standard Colors", "", [=]() {
				for (int i = 0; i < 4; i++)
					module->colorBox[i] = i;
			}));
			menu->addChild(createMenuItem("Set All to Custom Colors", "", [=]() {
				for (int i = 0; i < 4; i++)
					module->colorBox[i] = 4;
			}));
			menu->addChild(createMenuItem("Clear All Custom Colors", "", [=]() {
				module->colorBoxR[0] = 0;
				module->colorBoxR[1] = 255;
				module->colorBoxR[2] = 255;
				module->colorBoxR[3] = 0;
				module->colorBoxG[0] = 0;
				module->colorBoxG[1] = 0;
				module->colorBoxG[2] = 255;
				module->colorBoxG[3] = 255;
				module->colorBoxB[0] = 255;
				module->colorBoxB[1] = 0;
				module->colorBoxB[2] = 0;
				module->colorBoxB[3] = 0;
			}));
			
			menu->addChild(new MenuSeparator());
			menu->addChild(createMenuLabel("Lightboxes Fade Time"));
			menu->addChild(createMenuItem("Set All to Slow", "", [=]() {
				for (int i = 0; i < 4; i++)
					module->lightTime[i] = 0;
			}));
			menu->addChild(createMenuItem("Reset All to Normal", "", [=]() {
				for (int i = 0; i < 4; i++)
					module->lightTime[i] = 1;
			}));
			menu->addChild(createMenuItem("Set All to Fast", "", [=]() {
				for (int i = 0; i < 4; i++)
					module->lightTime[i] = 2;
			}));
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Store Samples in Patch", "", &module->sampleInPatch));
	}
};

Model *modelDrumPlayerXtra = createModel<DrumPlayerXtra, DrumPlayerXtraWidget>("DrumPlayerXtra");
