#define NO_INTERP 0
#define LINEAR1_INTERP 1
#define LINEAR2_INTERP 2
#define HERMITE_INTERP 3
#define NORMALLED_OUTS 0
#define SOLO_OUTS 1
#define UNCONNECTED_ON_4 2

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
		ENUMS(SLOT_LIGHT,4),
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

	double a0, a1, a2, b1, b2, z1, z2;

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

		configParam(SPEED_PARAM, 0.01f, 2.0f, 1.0f, "Speed #1", "x", 0, 1);
		configParam(SPEED_PARAM+1, 0.01f, 2.0f, 1.0f, "Speed #2", "x", 0, 1);
		configParam(SPEED_PARAM+2, 0.01f, 2.0f, 1.0f, "Speed #3", "x", 0, 1);
		configParam(SPEED_PARAM+3, 0.01f, 2.0f, 1.0f, "Speed #4", "x", 0, 1);

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

			sampleCoeff[slot] = sampleRate[slot] / (APP->engine->getSampleRate());			// the % distance between samples at speed 1x

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
		playBuffer[slot][0].clear();
		playBuffer[slot][1].clear();
		totalSampleC[slot] = 0;
	}

	void process(const ProcessArgs &args) override {
		summedOutput = 0;
		for (int i = 0; i < 4; i++){

			trigValue[i] = inputs[TRIG_INPUT+i].getVoltage();

			lights[SLOT_LIGHT+i].setBrightness(fileLoaded[i]);

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
				if (inputs[ACC_INPUT+i].getVoltage() > 1)
					level[i] = params[ACCVOL_PARAM+i].getValue();
				else
					level[i] = params[TRIGVOL_PARAM+i].getValue();
				
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
						currentOutput = 5 * level[i] * playBuffer[i][antiAlias][floor(samplePos[i])];
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
												) / 2
											);
						}
					break;

					case HERMITE_INTERP:
						if (currSampleWeight[i] == 0) {
							currentOutput = 5 * level[i] * float(playBuffer[i][antiAlias][floor(samplePos[i])]);
						} else {
							if (floor(samplePos[i]) > 1 && floor(samplePos[i]) < totalSamples[i] - 1) {
								/*
								currentOutput = hermiteInterpol(playBuffer[i][antiAlias][floor(samplePos[i])-1],
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

				currentSpeed = double(params[SPEED_PARAM+i].getValue());
				samplePos[i] += sampleCoeff[i]*currentSpeed;

				if (interpolationMode > NO_INTERP) {
					prevSampleWeight[i] = currSampleWeight[i];
					currSampleWeight[i] = samplePos[i] - floor(samplePos[i]);
				}
				
				if (fading[i]) {
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

			switch (outsMode) {
				case NORMALLED_OUTS:
					summedOutput += currentOutput;
					if (outputs[OUT_OUTPUT+i].isConnected()) {
						outputs[OUT_OUTPUT+i].setVoltage(summedOutput);
						summedOutput = 0;
					}
				break;

				case SOLO_OUTS:
					if (outputs[OUT_OUTPUT+i].isConnected())
						outputs[OUT_OUTPUT+i].setVoltage(currentOutput);
				break;

				case UNCONNECTED_ON_4:
					if (i == 3) {
						summedOutput += currentOutput;
						outputs[OUT_OUTPUT+i].setVoltage(summedOutput);
					} else if (outputs[OUT_OUTPUT+i].isConnected())
						outputs[OUT_OUTPUT+i].setVoltage(currentOutput);
					else
						summedOutput += currentOutput;
				break;
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

struct DrumPlayerItem2 : MenuItem {
	DrumPlayer *rm ;
	void onAction(const event::Action &e) override {
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL);
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

struct DrumPlayerItem3 : MenuItem {
	DrumPlayer *rm ;
	void onAction(const event::Action &e) override {
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL);
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

struct DrumPlayerItem4 : MenuItem {
	DrumPlayer *rm ;
	void onAction(const event::Action &e) override {
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL);
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

struct dpSlot1Display : TransparentWidget {
	DrumPlayer *module;
	int frame = 0;

	dpSlot1Display() {

	}

	struct ClearSlot1Item : MenuItem {
		DrumPlayer *module;
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
		DrumPlayer *module = dynamic_cast<DrumPlayer *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			DrumPlayerItem1 *rootDirItem = new DrumPlayerItem1;
				rootDirItem->text = "Load Sample Slot #1";
				rootDirItem->rm = module;
				menu->addChild(rootDirItem);
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription[0]));

			menu->addChild(construct<ClearSlot1Item>(&MenuItem::rightText, "Clear", &ClearSlot1Item::module, module));

		}
	}
};

struct dpSlot2Display : TransparentWidget {
	DrumPlayer *module;
	int frame = 0;

	dpSlot2Display() {

	}

	struct ClearSlot2Item : MenuItem {
		DrumPlayer *module;
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
		DrumPlayer *module = dynamic_cast<DrumPlayer *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			DrumPlayerItem2 *rootDirItem = new DrumPlayerItem2;
				rootDirItem->text = "Load Sample Slot #2";
				rootDirItem->rm = module;
				menu->addChild(rootDirItem);
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription[1]));

			menu->addChild(construct<ClearSlot2Item>(&MenuItem::rightText, "Clear", &ClearSlot2Item::module, module));

		}
	}
};

struct dpSlot3Display : TransparentWidget {
	DrumPlayer *module;
	int frame = 0;

	dpSlot3Display() {

	}

	struct ClearSlot3Item : MenuItem {
		DrumPlayer *module;
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
		DrumPlayer *module = dynamic_cast<DrumPlayer *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			DrumPlayerItem3 *rootDirItem = new DrumPlayerItem3;
				rootDirItem->text = "Load Sample Slot #3";
				rootDirItem->rm = module;
				menu->addChild(rootDirItem);
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription[2]));

			menu->addChild(construct<ClearSlot3Item>(&MenuItem::rightText, "Clear", &ClearSlot3Item::module, module));

		}
	}
};

struct dpSlot4Display : TransparentWidget {
	DrumPlayer *module;
	int frame = 0;

	dpSlot4Display() {

	}

	struct ClearSlot4Item : MenuItem {
		DrumPlayer *module;
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
		DrumPlayer *module = dynamic_cast<DrumPlayer *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			DrumPlayerItem4 *rootDirItem = new DrumPlayerItem4;
				rootDirItem->text = "Load Sample Slot #4";
				rootDirItem->rm = module;
				menu->addChild(rootDirItem);
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription[3]));

			menu->addChild(construct<ClearSlot4Item>(&MenuItem::rightText, "Clear", &ClearSlot4Item::module, module));

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

		float xDelta = 16;

		{
			dpSlot1Display *display = new dpSlot1Display();
			display->box.pos = Vec(6, 21);
			display->box.size = Vec(41, 24);
			display->module = module;
			addChild(display);
		}

		{
			dpSlot2Display *display = new dpSlot2Display();
			display->box.pos = Vec(54, 21);
			display->box.size = Vec(41, 24);
			display->module = module;
			addChild(display);
		}

		{
			dpSlot3Display *display = new dpSlot3Display();
			display->box.pos = Vec(101, 21);
			display->box.size = Vec(41, 24);
			display->module = module;
			addChild(display);
		}

		{
			dpSlot4Display *display = new dpSlot4Display();
			display->box.pos = Vec(148, 21);
			display->box.size = Vec(41, 24);
			display->module = module;
			addChild(display);
		}

		for (int i = 0; i < 4; i++) {
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(9+(xDelta*i), 9)), module, DrumPlayer::SLOT_LIGHT+i));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9+(xDelta*i), 20.2)), module, DrumPlayer::TRIG_INPUT+i));
			addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(9+(xDelta*i), 31.5)), module, DrumPlayer::TRIGVOL_PARAM+i));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9+(xDelta*i), 49.7)), module, DrumPlayer::ACC_INPUT+i));
			addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(9+(xDelta*i), 61)), module, DrumPlayer::ACCVOL_PARAM+i));

			addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(9+(xDelta*i), 80.5)), module, DrumPlayer::SPEED_PARAM+i));

			if (i<3)
				addParam(createParamCentered<CKSS>(mm2px(Vec(9+(xDelta*i), 98.4)), module, DrumPlayer::CHOKE_SWITCH+i));

			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(9+(xDelta*i), 117)), module, DrumPlayer::OUT_OUTPUT+i));
		}
	}

	struct ClearSlotsItem : MenuItem {
		DrumPlayer *module;
		void onAction(const event::Action &e) override {
			for (int i = 0; i < 4; i++)
				module->clearSlot(i);
		}
	};

	struct ClearSlot1Item : MenuItem {
		DrumPlayer *module;
		void onAction(const event::Action &e) override {
			module->clearSlot(0);
		}
	};

	struct ClearSlot2Item : MenuItem {
		DrumPlayer *module;
		void onAction(const event::Action &e) override {
			module->clearSlot(1);
		}
	};

	struct ClearSlot3Item : MenuItem {
		DrumPlayer *module;
		void onAction(const event::Action &e) override {
			module->clearSlot(2);
		}
	};

	struct ClearSlot4Item : MenuItem {
		DrumPlayer *module;
		void onAction(const event::Action &e) override {
			module->clearSlot(3);
		}
	};

	void appendContextMenu(Menu *menu) override {
	   	DrumPlayer *module = dynamic_cast<DrumPlayer*>(this->module);
			assert(module);
			menu->addChild(new MenuSeparator());

		DrumPlayerItem1 *rootDirItem1 = new DrumPlayerItem1;
			menu->addChild(createMenuLabel("Sample Slots"));
			rootDirItem1->text = std::to_string(1) + ": " + module->fileDescription[0];
			rootDirItem1->rm = module;
			menu->addChild(rootDirItem1);

		menu->addChild(construct<ClearSlot1Item>(&MenuItem::rightText, "Clear #1", &ClearSlot1Item::module, module));

		DrumPlayerItem2 *rootDirItem2 = new DrumPlayerItem2;
			rootDirItem2->text = std::to_string(2) + ": " + module->fileDescription[1];
			rootDirItem2->rm = module;
			menu->addChild(rootDirItem2);

		menu->addChild(construct<ClearSlot2Item>(&MenuItem::rightText, "Clear #2", &ClearSlot2Item::module, module));
		
		DrumPlayerItem3 *rootDirItem3 = new DrumPlayerItem3;
			rootDirItem3->text = std::to_string(3) + ": " + module->fileDescription[2];
			rootDirItem3->rm = module;
			menu->addChild(rootDirItem3);

		menu->addChild(construct<ClearSlot3Item>(&MenuItem::rightText, "Clear #3", &ClearSlot3Item::module, module));
		
		DrumPlayerItem4 *rootDirItem4 = new DrumPlayerItem4;
			rootDirItem4->text = std::to_string(4) + ": " + module->fileDescription[3];
			rootDirItem4->rm = module;
			menu->addChild(rootDirItem4);

		menu->addChild(construct<ClearSlot4Item>(&MenuItem::rightText, "Clear #4", &ClearSlot4Item::module, module));
			
		menu->addChild(new MenuSeparator());
		menu->addChild(construct<ClearSlotsItem>(&MenuItem::text, "Clear ALL slots", &ClearSlotsItem::module, module));
		menu->addChild(new MenuSeparator());

		menu->addChild(createMenuLabel("Interpolation"));
		struct ModeItem : MenuItem {
			DrumPlayer* module;
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
			DrumPlayer* module;
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

Model *modelDrumPlayer = createModel<DrumPlayer, DrumPlayerWidget>("DrumPlayer");
