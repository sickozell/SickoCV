#include "plugin.hpp"
#include "osdialog.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <vector>
#include "cmath"
#include <dirent.h>
#include <libgen.h>

using namespace std;

struct DrumPlayer : Module {
	enum ParamIds {
		ENUMS(TRIGVOL_PARAM,4),
		ENUMS(ACCVOL_PARAM,4),
		ENUMS(SPEED_PARAM,4),
		ENUMS(CHOKE_SWITCH,3),
		NUM_PARAMS 
	};
	enum InputIds {
		ENUMS(TRIG_INPUT,4),
		ENUMS(ACC_INPUT,4),
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
	double currentSpeed = 0;

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

	int resamplingMode = 1;

	bool clearSlots = false;

	DrumPlayer() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configInput(TRIG_INPUT,"Trig #1");
		configInput(TRIG_INPUT+1,"Trig #2");
		configInput(TRIG_INPUT+2,"Trig #3");
		configInput(TRIG_INPUT+3,"Trig #4");

		configParam(TRIGVOL_PARAM, 0.f, 2.0f, 1.0f, "Standard Level #1", "%", 0, 100);
		configParam(TRIGVOL_PARAM+1, 0.f, 2.0f, 1.0f, "Standard Level #2", "%", 0, 100);
		configParam(TRIGVOL_PARAM+2, 0.f, 2.0f, 1.0f, "Standard Level #3", "%", 0, 100);
		configParam(TRIGVOL_PARAM+3, 0.f, 2.0f, 1.0f, "Standard Level #4", "%", 0, 100);

		configInput(ACC_INPUT,"Accent #1");
		configInput(ACC_INPUT+1,"Accent #2");
		configInput(ACC_INPUT+2,"Accent #3");
		configInput(ACC_INPUT+3,"Accent #4");

		configParam(ACCVOL_PARAM, 0.f, 2.0f, 1.0f, "Accent Level #1", "%", 0, 100);
		configParam(ACCVOL_PARAM+1, 0.f, 2.0f, 1.0f, "Accent Level #2", "%", 0, 100);
		configParam(ACCVOL_PARAM+2, 0.f, 2.0f, 1.0f, "Accent Level #3", "%", 0, 100);
		configParam(ACCVOL_PARAM+3, 0.f, 2.0f, 1.0f, "Accent Level #4", "%", 0, 100);

		configParam(SPEED_PARAM, 0.01f, 2.0f, 1.0f, "Speed #1");
		configParam(SPEED_PARAM+1, 0.01f, 2.0f, 1.0f, "Speed #2");
		configParam(SPEED_PARAM+2, 0.01f, 2.0f, 1.0f, "Speed #3");
		configParam(SPEED_PARAM+3, 0.01f, 2.0f, 1.0f, "Speed #4");

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
		resamplingMode = 1;
		for (int i=0;i<4;i++) {
			clearSlotFunct(i);
			play[i] = false;
			choking[i] = false;
			fading[i] = false;
		}
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Slot1", json_string(storedPath[0].c_str()));
		json_object_set_new(rootJ, "Slot2", json_string(storedPath[1].c_str()));
		json_object_set_new(rootJ, "Slot3", json_string(storedPath[2].c_str()));
		json_object_set_new(rootJ, "Slot4", json_string(storedPath[3].c_str()));
		json_object_set_new(rootJ, "Resampling", json_integer(resamplingMode));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
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
		
		json_t* resamplingJ = json_object_get(rootJ, "Resampling");
		if (resamplingJ)
			resamplingMode = json_integer_value(resamplingJ);
		
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
			free(pathDup);

			storedPath[slot] = path;
		} else {
			fileLoaded[slot] = false;
		}
	};
	
	void clearSlotFunct(int slot) {
		storedPath[slot] = "";
		fileDesc[slot] = "";
		loading[slot] = false;
		fileLoaded[slot] = false;
		playBuffer[slot].clear();
		totalSampleC[slot] = 0;
	}

	double CubicInterpolate(double y0, double y1, double y2, double y3, double mu) {
		double a0,a1,a2,a3,mu2;
		mu2 = mu*mu;
		a0 = y3 - y2 - y0 + y1;
		a1 = y0 - y1 - a0;
		a2 = y2 - y0;
		a3 = y1;
		return(a0*mu*mu2+a1*mu2+a2*mu+a3);
	}
	/*
	double CosineInterpolate(double y1, double y2, double mu) {
		double mu2;
		mu2 = (1-cos(mu*M_PI))/2;
		return(y1*(1-mu2)+y2*mu2);
	}

	double HermiteInterpolate(double y0, double y1, double y2, double y3, double mu, double tension, double bias) {
		//	Tension: 1 is high, 0 normal, -1 is low
		//	Bias: 0 is even, positive is towards first segment, negative towards the other
		double m0,m1,mu2,mu3;
		double a0,a1,a2,a3;
		mu2 = mu * mu;
		mu3 = mu2 * mu;
		m0  = (y1-y0)*(1+bias)*(1-tension)/2;
		m0 += (y2-y1)*(1-bias)*(1-tension)/2;
		m1  = (y2-y1)*(1+bias)*(1-tension)/2;
		m1 += (y3-y2)*(1-bias)*(1-tension)/2;
		a0 =  2*mu3 - 3*mu2 + 1;
		a1 =    mu3 - 2*mu2 + mu;
		a2 =    mu3 -   mu2;
		a3 = -2*mu3 + 3*mu2;
		return(a0*y1+a1*m0+a2*m1+a3*y2);
	}
	*/

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
				if (inputs[ACC_INPUT+i].getVoltage() > 1)
					level[i] = params[ACCVOL_PARAM+i].getValue();
				else
					level[i] = params[TRIGVOL_PARAM+i].getValue();
				
				if (i < 3 && params[CHOKE_SWITCH+i].getValue()) {
					choking[i+1] = true;
					chokeValue[i+1] = 0.9f;
				}
			}
			prevTrigValue[i] = trigValue[i];
			currentOutput = 0;

			if (!loading[i] && play[i] && floor(samplePos[i]) < totalSampleC[i] && floor(samplePos[i]) >= 0) {
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
							if (floor(samplePos[i]) > 0 && floor(samplePos[i]) < totalSampleC[i] - 2) {
								resampled[i] = CubicInterpolate(playBuffer[i][floor(samplePos[i])-1],
																playBuffer[i][floor(samplePos[i])],
																playBuffer[i][floor(samplePos[i])+1],
																playBuffer[i][floor(samplePos[i])+2],
																currSampleWeight[i]);
								/*
								resampled[i] = CosineInterpolate(playBuffer[i][floor(samplePos[i])],
																playBuffer[i][floor(samplePos[i])+1],
																currSampleWeight[i]);
								*/
								/*
								resampled[i] = HermiteInterpolate(playBuffer[i][floor(samplePos[i])-1],
																playBuffer[i][floor(samplePos[i])],
																playBuffer[i][floor(samplePos[i])+1],
																playBuffer[i][floor(samplePos[i])+2],
																currSampleWeight[i],
																1,  // tension: 1 is high, 0 normal, -1 is low
																1);	// bias:0 is even, positive is towards first segment, negative towards the other
								*/
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
				currentSpeed = sampleCoeff[i]*double(params[SPEED_PARAM+i].getValue());
				samplePos[i] += currentSpeed;

				if (resamplingMode > 0) {
					prevSampleWeight[i] = currSampleWeight[i];
					currSampleWeight[i] = samplePos[i] - floor(samplePos[i]);
					//currSampleWeight[i] = modf(samplePos[i],&intSamplePos[i]);
				}
				
				if (fading[i]) {
					if (fadingValue[i] > 0) {
						fadingValue[i] -= 0.1;
						currentOutput += playBuffer[i][floor(fadedPosition[i])] * fadingValue[i] * level[i];
						fadedPosition[i] += currentSpeed;
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

			if (outputs[OUT_OUTPUT+i].isConnected()) {
				outputs[OUT_OUTPUT+i].setVoltage(summedOutput);
				summedOutput = 0;
			}

			if (!inputs[TRIG_INPUT+i].isConnected())
				play[i] = false;
		}
	}
};

struct DrumPlayerItem1 : MenuItem {
	DrumPlayer *rm ;
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

struct DrumPlayerItem2 : MenuItem {
	DrumPlayer *rm ;
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

struct DrumPlayerItem3 : MenuItem {
	DrumPlayer *rm ;
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

struct DrumPlayerItem4 : MenuItem {
	DrumPlayer *rm ;
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


struct DrumPlayerWidget : ModuleWidget {
	DrumPlayerWidget(DrumPlayer *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DrumPlayer.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	  
;
		float xDelta = 16;

		for (int i=0;i<4;i++) {
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.9+(xDelta*i), 19.5)), module, DrumPlayer::TRIG_INPUT+i));
			addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(8.9+(xDelta*i), 31.5)), module, DrumPlayer::TRIGVOL_PARAM+i));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.9+(xDelta*i), 49)), module, DrumPlayer::ACC_INPUT+i));
			addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(8.9+(xDelta*i), 61)), module, DrumPlayer::ACCVOL_PARAM+i));

			addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(8.9+(xDelta*i), 80.5)), module, DrumPlayer::SPEED_PARAM+i));

			if (i<3) {
				addParam(createParamCentered<CKSS>(mm2px(Vec(8.9+(xDelta*i), 98.4)), module, DrumPlayer::CHOKE_SWITCH+i));
			}

			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.9+(xDelta*i), 117)), module, DrumPlayer::OUT_OUTPUT+i));
		}
	}

	struct ClearSlotsItem : MenuItem {
		DrumPlayer *module;
		void onAction(const event::Action &e) override {
			for (int i=0;i<4;i++)
				module->clearSlotFunct(i);
		}
	};

	struct ClearSlot1Item : MenuItem {
		DrumPlayer *module;
		void onAction(const event::Action &e) override {
			module->clearSlotFunct(0);
		}
	};

	struct ClearSlot2Item : MenuItem {
		DrumPlayer *module;
		void onAction(const event::Action &e) override {
			module->clearSlotFunct(1);
		}
	};

	struct ClearSlot3Item : MenuItem {
		DrumPlayer *module;
		void onAction(const event::Action &e) override {
			module->clearSlotFunct(2);
		}
	};

	struct ClearSlot4Item : MenuItem {
		DrumPlayer *module;
		void onAction(const event::Action &e) override {
			module->clearSlotFunct(3);
		}
	};

	void appendContextMenu(Menu *menu) override {
	   	DrumPlayer *module = dynamic_cast<DrumPlayer*>(this->module);
			assert(module);
			menu->addChild(new MenuSeparator());

		DrumPlayerItem1 *rootDirItem1 = new DrumPlayerItem1;
			menu->addChild(createMenuLabel("Sample Slots"));
			rootDirItem1->text = std::to_string(1) + ": " + module->fileDesc[0];
			rootDirItem1->rm = module;
			menu->addChild(rootDirItem1);

		menu->addChild(construct<ClearSlot1Item>(&MenuItem::rightText, "Clear #1", &ClearSlot1Item::module, module));

		DrumPlayerItem2 *rootDirItem2 = new DrumPlayerItem2;
			rootDirItem2->text = std::to_string(2) + ": " + module->fileDesc[1];
			rootDirItem2->rm = module;
			menu->addChild(rootDirItem2);

		menu->addChild(construct<ClearSlot2Item>(&MenuItem::rightText, "Clear #2", &ClearSlot2Item::module, module));
		
		DrumPlayerItem3 *rootDirItem3 = new DrumPlayerItem3;
			rootDirItem3->text = std::to_string(3) + ": " + module->fileDesc[2];
			rootDirItem3->rm = module;
			menu->addChild(rootDirItem3);

		menu->addChild(construct<ClearSlot3Item>(&MenuItem::rightText, "Clear #3", &ClearSlot3Item::module, module));
		
		DrumPlayerItem4 *rootDirItem4 = new DrumPlayerItem4;
			rootDirItem4->text = std::to_string(4) + ": " + module->fileDesc[3];
			rootDirItem4->rm = module;
			menu->addChild(rootDirItem4);

		menu->addChild(construct<ClearSlot4Item>(&MenuItem::rightText, "Clear #4", &ClearSlot4Item::module, module));
			
			menu->addChild(new MenuSeparator());
			menu->addChild(construct<ClearSlotsItem>(&MenuItem::text, "Clear ALL slots", &ClearSlotsItem::module, module));
			menu->addChild(new MenuSeparator());

			menu->addChild(createMenuLabel("Resampling Mode"));
			struct ModeItem : MenuItem {
				DrumPlayer* module;
				int resamplingMode;
				void onAction(const event::Action& e) override {
					module->resamplingMode = resamplingMode;
				}
			};

			std::string modeNames[4] = {"No interpolation", "Interpolate 1", "Interpolate 2", "Cubic"};
			for (int i = 0; i < 4; i++) {
				ModeItem* modeItem = createMenuItem<ModeItem>(modeNames[i]);
				modeItem->rightText = CHECKMARK(module->resamplingMode == i);
				modeItem->module = module;
				modeItem->resamplingMode = i;
				menu->addChild(modeItem);
			}
	}
};

Model *modelDrumPlayer = createModel<DrumPlayer, DrumPlayerWidget>("DrumPlayer");
