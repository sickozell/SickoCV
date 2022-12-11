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

	vector<float> playBuffer[4];

	bool loading[4] = {false, false, false, false};
	bool fileLoaded[4] = {false, false, false, false};

	bool play[4] = {false, false, false, false};

	double samplePos[4] = {0,0,0,0};
	double sampleCoeff[4];
	double currentSpeed = 0.0;

	double prevSampleWeight [4] = {0,0,0,0};
	double currSampleWeight [4] = {0,0,0,0};
	double prevSamplePos[4] = {0,0,0,0};
	double sampleInterpol = 0;
	int sampleInterpCounter = 0;
	double avePrevSample;
	double avePostSample;
	double resampled[4];

	std::string storedPath[4] = {"","","",""};
	std::string fileDesc[4];
	std::string fileDisplay[4] = {"-----","-----","-----","-----"};

	float trigValue[4] = {0.f, 0.f, 0.f, 0.f};
	float prevTrigValue[4] = {0.f, 0.f, 0.f, 0.f};
	bool choking[4] = {false, false, false, false};
	float chokeValue[4] = {1.f, 1.f, 1.f, 1.f};
	bool fading[4] = {false, false, false, false};
	float fadingValue[4] = {0.f, 0.f, 0.f, 0.f};
	double fadedPosition[4] = {0, 0, 0, 0};
	
	float level[4];
	float currentOutput;
	float summedOutput;

	int resamplingMode = 3;

	bool clearSlots = false;

	bool normalledOuts = true;

	//std::string debugDisplay = "X";

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
		configParam(SPEED_PARAM, 0.01f, 2.0f, 1.0f, "Speed #1");
		configParam(SPEED_PARAM+1, 0.01f, 2.0f, 1.0f, "Speed #2");
		configParam(SPEED_PARAM+2, 0.01f, 2.0f, 1.0f, "Speed #3");
		configParam(SPEED_PARAM+3, 0.01f, 2.0f, 1.0f, "Speed #4");
		configInput(SPEED_INPUT,"Speed CV #1");
		configInput(SPEED_INPUT+1,"Speed CV #2");
		configInput(SPEED_INPUT+2,"Speed CV #3");
		configInput(SPEED_INPUT+3,"Speed CV #4");
		configSwitch(LIMIT_SWITCH, 0.f, 1.f, 0.f, "Limit #1", {"Off", "5v"});
		configSwitch(LIMIT_SWITCH+1, 0.f, 1.f, 0.f, "Limit #2", {"Off", "5v"});
		configSwitch(LIMIT_SWITCH+2, 0.f, 1.f, 0.f, "Limit #3", {"Off", "5v"});
		configSwitch(LIMIT_SWITCH+3, 0.f, 1.f, 0.f, "Limit #4", {"Off", "5v"});
		configSwitch(CHOKE_SWITCH, 0.f, 1.f, 0.f, "Choke #1", {"Off", "On"});
		configSwitch(CHOKE_SWITCH+1, 0.f, 1.f, 0.f, "Choke #2", {"Off", "On"});
		configSwitch(CHOKE_SWITCH+2, 0.f, 1.f, 0.f, "Choke #3", {"Off", "On"});

		configOutput(OUT_OUTPUT,"out #1");
		configOutput(OUT_OUTPUT+1,"out #2");
		configOutput(OUT_OUTPUT+2,"out #3");
		configOutput(OUT_OUTPUT+3,"out #4");

		playBuffer[0].resize(0);
		playBuffer[1].resize(0);
		playBuffer[2].resize(0); 
		playBuffer[3].resize(0); 
	}

	void onReset() override {
		resamplingMode = 3;
		for (int i=0;i<4;i++) {
			clearSlotFunct(i);
			play[i] = false;
			choking[i] = false;
			fading[i] = false;
		}
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Resampling", json_integer(resamplingMode));
		json_object_set_new(rootJ, "NormalledOuts", json_boolean(normalledOuts));
		json_object_set_new(rootJ, "Slot1", json_string(storedPath[0].c_str()));
		json_object_set_new(rootJ, "Slot2", json_string(storedPath[1].c_str()));
		json_object_set_new(rootJ, "Slot3", json_string(storedPath[2].c_str()));
		json_object_set_new(rootJ, "Slot4", json_string(storedPath[3].c_str()));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t* resamplingJ = json_object_get(rootJ, "Resampling");
		if (resamplingJ)
			resamplingMode = json_integer_value(resamplingJ);

		json_t* normalledOutsJ = json_object_get(rootJ, "NormalledOuts");
		if (normalledOutsJ)
			normalledOuts = json_integer_value(normalledOutsJ);

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
	}

	void loadSample(std::string path, int slot) {

		loading[slot] = true;
		unsigned int c;
		unsigned int sr;
		drwav_uint64 tsc;
		float* pSampleData;
		pSampleData = drwav_open_and_read_file_f32(path.c_str(), &c, &sr, &tsc);

		if (pSampleData != NULL) {
			channels[slot] = c;
			sampleRate[slot] = sr;
			playBuffer[slot].clear();
			for (unsigned int i=0; i < tsc; i = i + c) {
				if (channels[slot] == 1) 
					playBuffer[slot].push_back(pSampleData[i]);
				else if (channels[slot] == 2)
					playBuffer[slot].push_back((pSampleData[i]+(float)pSampleData[i+1])/2);
			}
			totalSampleC[slot] = playBuffer[slot].size();
			drwav_free(pSampleData);
			loading[slot] = false;

			fileLoaded[slot] = true;

			char* pathDup = strdup(path.c_str());
			fileDesc[slot] = basename(pathDup);
			fileDisplay[slot] = fileDesc[slot].substr(0,5);
			free(pathDup);

			storedPath[slot] = path;
		} else {
			fileLoaded[slot] = false;
		}
	};
	
	void clearSlotFunct(int slot) {
		storedPath[slot] = "";
		fileDesc[slot] = "";
		fileDisplay[slot] ="-----";
		loading[slot] = false;
		fileLoaded[slot] = false;
		playBuffer[slot].clear();
		totalSampleC[slot] = 0;
	}

	void process(const ProcessArgs &args) override {
		summedOutput = 0;
		for (int i=0;i<4;i++){
			if (!fileLoaded[i])
				fileDesc[i] = "-----";

			trigValue[i] = inputs[TRIG_INPUT+i].getVoltage();

			if (trigValue[i] >= 1 && prevTrigValue[i] < 1){
				if (play[i]) {
					fading[i] = true;
					fadingValue[i] = 5.f;
					fadedPosition[i] = samplePos[i];
				}
				play[i] = true;
				sampleCoeff[i] = sampleRate[i] / args.sampleRate;
				samplePos[i] = 0;
				currSampleWeight[i] = sampleCoeff[i];
				prevSamplePos[i] = 0;
				prevSampleWeight[i] = 0;
				//debugDisplay = "X";
				if (inputs[ACC_INPUT+i].getVoltage() > 1)
					level[i] = params[ACCVOL_PARAM+i].getValue() + (inputs[ACCVOL_INPUT+i].getVoltage() * params[ACCVOLATNV_PARAM+i].getValue());
				else
					level[i] = params[TRIGVOL_PARAM+i].getValue() + (inputs[TRIGVOL_INPUT+i].getVoltage() * params[TRIGVOLATNV_PARAM+i].getValue());
				
				if (i < 3 && params[CHOKE_SWITCH+i].getValue()) {
					choking[i+1] = true;
					chokeValue[i+1] = 0.9f;
				}
			}
			prevTrigValue[i] = trigValue[i];
			currentOutput = 0;

			if (fileLoaded[i] && play[i] && floor(samplePos[i]) < totalSampleC[i] && floor(samplePos[i]) >= 0) {
				switch (resamplingMode) {
					case 0:
						currentOutput = 5 * level[i] * playBuffer[i][floor(samplePos[i])];
					break;

					case 1:
						if (currSampleWeight[i] == 0.0) {
							resampled[i] = playBuffer[i][floor(samplePos[i])];
						} else {
							sampleInterpol = 0;
							sampleInterpCounter = 0;
							avePostSample = 0;
							for (int j=floor(prevSamplePos[i])+2;j<floor(samplePos[i]);j++){
								sampleInterpol += playBuffer[i][j];
								sampleInterpCounter++;
							}
							avePostSample = (playBuffer[i][floor(samplePos[i])] * (1-currSampleWeight[i])) + (playBuffer[i][floor(samplePos[i])+1] * currSampleWeight[i] );
							resampled[i] = ( sampleInterpol + avePostSample  ) / ( sampleInterpCounter + 1);
						}
						currentOutput = 5 * level[i] * float(resampled[i]);
					break;

					case 2:
						if (currSampleWeight[i] == 0.0) {
							resampled[i] = playBuffer[i][floor(samplePos[i])];
						} else {
							sampleInterpol = 0;
							sampleInterpCounter = 0;
							avePrevSample = 0;
							avePostSample = 0;
							for (int j=floor(prevSamplePos[i])+2;j<floor(samplePos[i]);j++){
								sampleInterpol += playBuffer[i][j];
								sampleInterpCounter++;
							}
							avePrevSample = (playBuffer[i][floor(prevSamplePos[i])] * (1-prevSampleWeight[i])) + (playBuffer[i][floor(prevSamplePos[i])+1] * prevSampleWeight[i]);
							avePostSample = (playBuffer[i][floor(samplePos[i])] * (1-currSampleWeight[i])) + (playBuffer[i][floor(samplePos[i])+1] * currSampleWeight[i] );
							resampled[i] = ( sampleInterpol + avePrevSample + avePostSample  ) / (sampleInterpCounter + 2);
						}
						currentOutput = 5 * level[i] * float(resampled[i]);
					break;

					case 3:
						if (currSampleWeight[i] == 0.0) {
							resampled[i] = playBuffer[i][floor(samplePos[i])];
						} else {
							if (floor(samplePos[i]) > 1 && floor(samplePos[i]) < totalSampleC[i] - 2) {
								double a1 = .5F * (playBuffer[i][floor(samplePos[i])+1] - playBuffer[i][floor(samplePos[i])-1]);
								double a2 = playBuffer[i][floor(samplePos[i])-1] - (2.5F * playBuffer[i][floor(samplePos[i])]) + (2 * playBuffer[i][floor(samplePos[i])+1]) - (.5F * playBuffer[i][floor(samplePos[i])+2]);
								double a3 = (.5F * (playBuffer[i][floor(samplePos[i])+2] - playBuffer[i][floor(samplePos[i])-1])) + (1.5F * (playBuffer[i][floor(samplePos[i])] - playBuffer[i][floor(samplePos[i])+1]));
								resampled[i] = (((((a3 * currSampleWeight[i]) + a2) * currSampleWeight[i]) + a1) * currSampleWeight[i]) + playBuffer[i][floor(samplePos[i])];
							} else {
								resampled[i] = playBuffer[i][floor(samplePos[i])];
							}
						}
						//if (i == 0)
						//	debugDisplay = to_string(sampleInterpCounter);

						currentOutput = 5 * level[i] * float(resampled[i]);
					break;
				}

				if (i > 0 && choking[i]) {
					if (chokeValue[i] < 0.1) {
						choking[i] = false;
						play[i] = false;
						currentOutput = 0;
					} else {
						currentOutput *= chokeValue[i];
						chokeValue[i] -= 0.1;
					}
				}

				prevSamplePos[i] = samplePos[i];
				if (inputs[SPEED_INPUT+i].isConnected()) {
					currentSpeed = double((params[SPEED_PARAM+i].getValue()+(inputs[SPEED_INPUT+i].getVoltage()*params[SPEEDATNV_PARAM+i].getValue())));
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

				if (resamplingMode > 0) {
					prevSampleWeight[i] = currSampleWeight[i];
					currSampleWeight[i] = samplePos[i] - floor(samplePos[i]);
					//currSampleWeight[i] = modf(samplePos[i],&intSamplePos[i]);
					//if (i ==0)
					//	debugDisplay = to_string(sampleInterpCounter);
				}

				if (fading[i]) {
					if (fadingValue[i] > 0) {
						fadingValue[i] -= 0.1;
						currentOutput += playBuffer[i][floor(fadedPosition[i])] * fadingValue[i] * level[i];
						fadedPosition[i] += sampleCoeff[i]*currentSpeed;
						if (fadedPosition[i] > totalSampleC[i])
							fading[i] = false;
					} else
						fading[i] = false;
				}

			} else {
				choking[i] = false;
				play[i] = false;
				fading[i] = false;
			}

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

			if (!normalledOuts)
				summedOutput = 0;

			if (!inputs[TRIG_INPUT+i].isConnected())
				play[i] = false;
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

struct DrumPlayerPlusItem1 : MenuItem {
	DrumPlayerPlus *rm ;
	void onAction(const event::Action &e) override {

		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL);
		int i = 0;
		if (path) {
			rm->play[i] = false;
			rm->loadSample(path, i);
			rm->samplePos[i] = 0;
			rm->storedPath[i] = std::string(path);
			free(path);
		}
	}
};

struct DrumPlayerPlusItem2 : MenuItem {
	DrumPlayerPlus *rm ;
	void onAction(const event::Action &e) override {
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL);
		int i = 1;
		if (path) {
			rm->play[i] = false;
			rm->loadSample(path, i);
			rm->samplePos[i] = 0;
			rm->storedPath[i] = std::string(path);
			free(path);
		}
	}
};

struct DrumPlayerPlusItem3 : MenuItem {
	DrumPlayerPlus *rm ;
	void onAction(const event::Action &e) override {
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL);
		int i = 2;
		if (path) {
			rm->play[i] = false;
			rm->loadSample(path, i);
			rm->samplePos[i] = 0;
			rm->storedPath[i] = std::string(path);
			free(path);
		}
	}
};

struct DrumPlayerPlusItem4 : MenuItem {
	DrumPlayerPlus *rm ;
	void onAction(const event::Action &e) override {
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL);
		int i = 3;
		if (path) {
			rm->play[i] = false;
			rm->loadSample(path, i);
			rm->samplePos[i] = 0;
			rm->storedPath[i] = std::string(path);
			free(path);
		}
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
			DrumPlayerPlusDisplay *display = new DrumPlayerPlusDisplay();
			display->box.pos = Vec(13, 45);
			display->box.size = Vec(5, 5);
			display->module = module;
			addChild(display);
		}
	    
		float xDelta = 23.5;

		for (int i=0;i<4;i++) {
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.9+(xDelta*i), 21)), module, DrumPlayerPlus::TRIG_INPUT+i));
			addParam(createParamCentered<Trimpot>(mm2px(Vec(17.9+(xDelta*i), 28.5)), module, DrumPlayerPlus::TRIGVOLATNV_PARAM+i));
			addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.9+(xDelta*i), 33)), module, DrumPlayerPlus::TRIGVOL_PARAM+i));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.9+(xDelta*i), 36.5)), module, DrumPlayerPlus::TRIGVOL_INPUT+i));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.9+(xDelta*i), 47)), module, DrumPlayerPlus::ACC_INPUT+i));
			addParam(createParamCentered<Trimpot>(mm2px(Vec(17.9+(xDelta*i), 54.5)), module, DrumPlayerPlus::ACCVOLATNV_PARAM+i));
			addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.9+(xDelta*i), 59)), module, DrumPlayerPlus::ACCVOL_PARAM+i));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.9+(xDelta*i), 62.5)), module, DrumPlayerPlus::ACCVOL_INPUT+i));

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
			for (int i=0;i<4;i++)
				module->clearSlotFunct(i);
		}
	};

	struct ClearSlot1Item : MenuItem {
		DrumPlayerPlus *module;
		void onAction(const event::Action &e) override {
			module->clearSlotFunct(0);
		}
	};

	struct ClearSlot2Item : MenuItem {
		DrumPlayerPlus *module;
		void onAction(const event::Action &e) override {
			module->clearSlotFunct(1);
		}
	};

	struct ClearSlot3Item : MenuItem {
		DrumPlayerPlus *module;
		void onAction(const event::Action &e) override {
			module->clearSlotFunct(2);
		}
	};

	struct ClearSlot4Item : MenuItem {
		DrumPlayerPlus *module;
		void onAction(const event::Action &e) override {
			module->clearSlotFunct(3);
		}
	};

	void appendContextMenu(Menu *menu) override {
	   	DrumPlayerPlus *module = dynamic_cast<DrumPlayerPlus*>(this->module);
			assert(module);
			menu->addChild(new MenuSeparator());

		DrumPlayerPlusItem1 *rootDirItem1 = new DrumPlayerPlusItem1;
			menu->addChild(createMenuLabel("Sample Slots"));
			rootDirItem1->text = std::to_string(1) + ": " + module->fileDesc[0];
			rootDirItem1->rm = module;
			menu->addChild(rootDirItem1);

		menu->addChild(construct<ClearSlot1Item>(&MenuItem::rightText, "Clear #1", &ClearSlot1Item::module, module));

		DrumPlayerPlusItem2 *rootDirItem2 = new DrumPlayerPlusItem2;
			rootDirItem2->text = std::to_string(2) + ": " + module->fileDesc[1];
			rootDirItem2->rm = module;
			menu->addChild(rootDirItem2);

		menu->addChild(construct<ClearSlot2Item>(&MenuItem::rightText, "Clear #2", &ClearSlot2Item::module, module));
		
		DrumPlayerPlusItem3 *rootDirItem3 = new DrumPlayerPlusItem3;
			rootDirItem3->text = std::to_string(3) + ": " + module->fileDesc[2];
			rootDirItem3->rm = module;
			menu->addChild(rootDirItem3);

		menu->addChild(construct<ClearSlot3Item>(&MenuItem::rightText, "Clear #3", &ClearSlot3Item::module, module));
		
		DrumPlayerPlusItem4 *rootDirItem4 = new DrumPlayerPlusItem4;
			rootDirItem4->text = std::to_string(4) + ": " + module->fileDesc[3];
			rootDirItem4->rm = module;
			menu->addChild(rootDirItem4);

		menu->addChild(construct<ClearSlot4Item>(&MenuItem::rightText, "Clear #4", &ClearSlot4Item::module, module));
			
			menu->addChild(new MenuSeparator());
			menu->addChild(construct<ClearSlotsItem>(&MenuItem::text, "Clear ALL slots", &ClearSlotsItem::module, module));
			menu->addChild(new MenuSeparator());

		menu->addChild(createMenuLabel("Resampling Mode"));
		struct ModeItem : MenuItem {
			DrumPlayerPlus* module;
			int resamplingMode;
			void onAction(const event::Action& e) override {
				module->resamplingMode = resamplingMode;
			}
		};

		std::string modeNames[4] = {"No interpolation", "Linear 1", "Linear 2", "Hermite"};
		for (int i = 0; i < 4; i++) {
			ModeItem* modeItem = createMenuItem<ModeItem>(modeNames[i]);
			modeItem->rightText = CHECKMARK(module->resamplingMode == i);
			modeItem->module = module;
			modeItem->resamplingMode = i;
			menu->addChild(modeItem);
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Normalled OUTs", "", &module->normalledOuts));
	}
};

Model *modelDrumPlayerPlus = createModel<DrumPlayerPlus, DrumPlayerPlusWidget>("DrumPlayerPlus");
