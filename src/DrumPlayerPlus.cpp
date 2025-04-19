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
	drwav_uint64 totalSampleC[4] = {0, 0, 0, 0};
	drwav_uint64 totalSamples[4] = {0, 0, 0, 0};

	const unsigned int minSamplesToLoad = 9;

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
	std::string scrollDisplay[4] = {"-----","-----","-----","-----"};
	std::string currFileDisplay[4] = {"-----","-----","-----","-----"};
	std::string userFolder = "";
	bool rootFound = false;
	bool fileFound[4] = {false, false, false, false};

	std::string tempDir = "";
	vector<vector<std::string>> folderTreeData;
	vector<vector<std::string>> folderTreeDisplay;
	vector<std::string> tempTreeData;
	vector<std::string> tempTreeDisplay;

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
	int scrolling = 0;

	bool sampleInPatch = true;
	bool loadFromPatch[4] = {false, false, false, false};
	bool restoreLoadFromPatch[4] = {false, false, false, false};

	//std::string debugDisplay = "X";

	double a0, a1, a2, b1, b2, z1, z2;

	DrumPlayerPlus() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
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
		configInput(ACCVOL_INPUT,"Accent.Lev.CV #1");
		configInput(ACCVOL_INPUT+1,"Accent.Lev.CV #2");
		configInput(ACCVOL_INPUT+2,"Accent.Lev.CV #3");
		configInput(ACCVOL_INPUT+3,"Accent.Lev.CV #4");
		configParam(ACCVOL_PARAM, 0.f, 2.0f, 1.0f, "Accent Level #1", "%", 0, 100);
		configParam(ACCVOL_PARAM+1, 0.f, 2.0f, 1.0f, "Accent Level #2", "%", 0, 100);
		configParam(ACCVOL_PARAM+2, 0.f, 2.0f, 1.0f, "Accent Level #3", "%", 0, 100);
		configParam(ACCVOL_PARAM+3, 0.f, 2.0f, 1.0f, "Accent Level #4", "%", 0, 100);
		configParam(ACCVOLATNV_PARAM, -1.f, 1.0f, 0.f, "Accent.Lev.CV #1", "%", 0, 100);
		configParam(ACCVOLATNV_PARAM+1, -1.f, 1.0f, 0.f, "Accent.Lev.CV #2", "%", 0, 100);
		configParam(ACCVOLATNV_PARAM+2, -1.f, 1.0f, 0.f, "Accent.Lev.CV #3", "%", 0, 100);
		configParam(ACCVOLATNV_PARAM+3, -1.f, 1.0f, 0.f, "Accent.Lev.CV #4", "%", 0, 100);
		configParam(SPEEDATNV_PARAM, -1.0f, 1.0f, 0.0f, "Speed CV Attenuv. #1");
		configParam(SPEEDATNV_PARAM+1, -1.0f, 1.0f, 0.0f, "Speed CV #2");
		configParam(SPEEDATNV_PARAM+2, -1.0f, 1.0f, 0.0f, "Speed CV #3");
		configParam(SPEEDATNV_PARAM+3, -1.0f, 1.0f, 0.0f, "Speed CV #4");
		configParam(SPEED_PARAM, 0.01f, 2.0f, 1.0f, "Speed #1", "x", 0, 1);
		configParam(SPEED_PARAM+1, 0.01f, 2.0f, 1.0f, "Speed #2", "x", 0, 1);
		configParam(SPEED_PARAM+2, 0.01f, 2.0f, 1.0f, "Speed #3", "x", 0, 1);
		configParam(SPEED_PARAM+3, 0.01f, 2.0f, 1.0f, "Speed #4", "x", 0, 1);
		configInput(SPEED_INPUT,"Speed #1");
		configInput(SPEED_INPUT+1,"Speed #2");
		configInput(SPEED_INPUT+2,"Speed #3");
		configInput(SPEED_INPUT+3,"Speed #4");
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

	void onReset(const ResetEvent &e) override {
		for (int slot = 0; slot < 4; slot++) {
			clearSlot(slot);
			play[slot] = false;
			choking[slot] = false;
			fading[slot] = false;
			loadFromPatch[slot] = false;
		}
		interpolationMode = HERMITE_INTERP;
		antiAlias = 1;
		outsMode = NORMALLED_OUTS;
		scrolling = 0;
		sampleInPatch = true;
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
		json_object_set_new(rootJ, "Scrolling", json_integer(scrolling));
		json_object_set_new(rootJ, "sampleInPatch", json_boolean(sampleInPatch));
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
		json_t* scrollingJ = json_object_get(rootJ, "Scrolling");
		if (scrollingJ)
			scrolling = json_integer_value(scrollingJ);
		json_t* sampleInPatchJ = json_object_get(rootJ, "sampleInPatch");
		if (sampleInPatchJ)
			sampleInPatch = json_boolean_value(sampleInPatchJ);
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

	void swapSlot(int slot1, int slot2) {
		vector<float> swapBuffer[2];
		unsigned int swapSampleRate = sampleRate[slot1];
		drwav_uint64 swapTotalSampleC = totalSampleC[slot1];
		drwav_uint64 swapTotalSamples = totalSamples[slot1];
		double swapSampleCoeff = sampleCoeff[slot1];
		std::string swapStoredPath = storedPath[slot1];
		std::string swapFileDescription = fileDescription[slot1];
		std::string swapFileDisplay = fileDisplay[slot1];
		std::string swapScrollDisplay = scrollDisplay[slot1];
		std::string swapCurrFileDisplay = currFileDisplay[slot1];
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
		fileFound[slot1] = fileFound[slot2];
		fileLoaded[slot1] = fileLoaded[slot2];
		loadFromPatch[slot1] = loadFromPatch[slot2];

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
		fileFound[slot2] = swapFileFound;
		fileLoaded[slot2] = swapFileLoaded;
		loadFromPatch[slot2] = swapLoadFromPatch;
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
		fileFound[slot2] = fileFound[slot1];
		fileLoaded[slot2] = fileLoaded[slot1];
		loadFromPatch[slot2] = loadFromPatch[slot1];
		for (unsigned int i = 0; i < totalSampleC[slot1]; i++) {
			playBuffer[slot2][0].push_back(playBuffer[slot1][0][i]);
			playBuffer[slot2][1].push_back(playBuffer[slot1][1][i]);
		}
	}

//	-----------------------------------------------------------------------------------------------

	float* LoadWavFileF32(const std::string& path, uint32_t* channels, uint32_t* sampleRate, uint64_t* totalSampleCount) {
	    drwav wav;
	    if (!drwav_init_file(&wav, path.c_str(), nullptr)) {
	        return nullptr;
	    }

	    if (channels) *channels = wav.channels;
	    if (sampleRate) *sampleRate = wav.sampleRate;

	    uint64_t frameCount = wav.totalPCMFrameCount;
	    uint64_t sampleCount = frameCount * wav.channels;

	    float* pSampleData = (float*)malloc((size_t)sampleCount * sizeof(float));
	    if (!pSampleData) {
	        drwav_uninit(&wav);
	        return nullptr;
	    }

	    uint64_t framesRead = drwav_read_pcm_frames_f32(&wav, frameCount, pSampleData);
	    drwav_uninit(&wav);

	    if (totalSampleCount) *totalSampleCount = framesRead * wav.channels;

	    return pSampleData;
	}

//	-----------------------------------------------------------------------------------------------	

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
		//unsigned int c;
		//unsigned int sr;
		//drwav_uint64 tsc;
		uint32_t c;
		uint32_t sr;
		uint64_t tsc;
		//float* pSampleData;
		//pSampleData = drwav_open_and_read_file_f32(path.c_str(), &c, &sr, &tsc);
		float* pSampleData = LoadWavFileF32(path.c_str(), &c, &sr, &tsc);	// new dr_wav lib

		if (pSampleData != NULL && tsc > minSamplesToLoad * c) {
			fileFound[slot] = true;
			//channels[slot] = c;
			sampleRate[slot] = sr * 2;
			calcBiquadLpf(20000.0, sampleRate[slot], 1);
			playBuffer[slot][0].clear();
			playBuffer[slot][1].clear();

			// metamodule change
			vector<float>().swap(playBuffer[slot][0]);
			vector<float>().swap(playBuffer[slot][1]);

			for (unsigned int i = 0; i < tsc; i = i + c) {
				playBuffer[slot][0].push_back(pSampleData[i]);
				playBuffer[slot][0].push_back(0);
			}
			totalSampleC[slot] = playBuffer[slot][0].size();
			totalSamples[slot] = totalSampleC[slot]-1;
//			drwav_free(pSampleData);	// unused (old dr_wav)

			for (unsigned int i = 1; i < totalSamples[slot]; i = i + 2)		// averaging oversampled vector
				playBuffer[slot][0][i] = playBuffer[slot][0][i-1] * .5f + playBuffer[slot][0][i+1] * .5f;

			playBuffer[slot][0][totalSamples[slot]] = playBuffer[slot][0][totalSamples[slot]-1] * .5f; // halve the last sample

			for (unsigned int i = 0; i < totalSampleC[slot]; i++)	// populating filtered vector
				playBuffer[slot][1].push_back(biquadLpf(playBuffer[slot][0][i]));

			sampleCoeff[slot] = sampleRate[slot] / (APP->engine->getSampleRate());		// the % distance between samples at speed 1x

			if (loadFromPatch[slot])
				path = storedPath[slot];

			fileDescription[slot] = system::getFilename(std::string(path));
			fileDescription[slot] = fileDescription[slot].substr(0,fileDescription[slot].length()-4);

			if (loadFromPatch[slot])
				fileDescription[slot] = "(!)"+fileDescription[slot];

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

			if (fileDisplay[slot].length() > 5)
				scrollDisplay[slot] = "!!!!!" + fileDisplay[slot] + "!!!!";
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
			fileDisplay[slot] = "-----";
			scrollDisplay[slot] = "-----";
			currFileDisplay[slot] = "-----";
		}
	};

// -------------------------------------------------------------------------------------------------------------------------------

	bool SaveWavFileF32(const std::string& path, const std::vector<float>& data, uint32_t sampleRate, uint32_t channels) {
	    drwav_data_format format;
	    format.container = drwav_container_riff;      // Standard WAV
	    format.format = DR_WAVE_FORMAT_IEEE_FLOAT;    // Float 32-bit
	    format.channels = channels;
	    format.sampleRate = sampleRate;
	    format.bitsPerSample = 32;

	    drwav wav;
	    if (!drwav_init_file_write(&wav, path.c_str(), &format, nullptr)) {
	        return false;
	    }

	    drwav_uint64 framesToWrite = data.size() / channels;

	    // Scrivi i frame
	    drwav_uint64 framesWritten = drwav_write_pcm_frames(&wav, framesToWrite, data.data());

	    drwav_uninit(&wav);

	    return framesWritten == framesToWrite;
	}

// -------------------------------------------------------------------------------------------------------------------------------

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
/*
		drwav *pWav = drwav_open_file_write(path.c_str(), &format);
		drwav_write(pWav, samples, data.data());
		drwav_close(pWav);
*/

		bool ok = SaveWavFileF32(path.c_str(), data, format.sampleRate, format.channels);
		if (!ok) {
		    // std::cerr << "Errore durante il salvataggio WAV" << std::endl;
		    INFO("ERROR WRITING");
		}

		data.clear();
	}
	
	void clearSlot(int slot) {
		fileLoaded[slot] = false;
		fileFound[slot] = false;
		loadFromPatch[slot] = false;
		restoreLoadFromPatch[slot] = false;
		storedPath[slot] = "";
		fileDescription[slot] = "--none--";
		fileDisplay[slot] = "-----";
		scrollDisplay[slot] = "-----";
		currFileDisplay[slot] = "-----";
		playBuffer[slot][0].clear();
		playBuffer[slot][1].clear();

		// metamodule change
		vector<float>().swap(playBuffer[slot][0]);
		vector<float>().swap(playBuffer[slot][1]);
		
		totalSampleC[slot] = 0;
	}
	
	void process(const ProcessArgs &args) override {
		summedOutput = 0;
		for (int slot = 0; slot < 4; slot++){

			trigValue[slot] = inputs[TRIG_INPUT+slot].getVoltage();

			if (trigValue[slot] >= 1 && prevTrigValue[slot] < 1){
				if (play[slot]) {
					fading[slot] = true;
					fadingValue[slot] = 1.f;
					fadedPosition[slot] = samplePos[slot];
				}
				play[slot] = true;
				samplePos[slot] = 0;
				currSampleWeight[slot] = sampleCoeff[slot];
				prevSamplePos[slot] = 0;
				prevSampleWeight[slot] = 0;
				//debugDisplay = "X";
				if (inputs[ACC_INPUT+slot].getVoltage() > 1)
					level[slot] = params[ACCVOL_PARAM+slot].getValue() + (inputs[ACCVOL_INPUT+slot].getVoltage() * params[ACCVOLATNV_PARAM+slot].getValue() * 0.1);
				else
					level[slot] = params[TRIGVOL_PARAM+slot].getValue() + (inputs[TRIGVOL_INPUT+slot].getVoltage() * params[TRIGVOLATNV_PARAM+slot].getValue() * 0.1);

				if (level[slot] > 2)
					level[slot] = 2;
				else if (level[slot] < 0)
					level[slot] = 0;
				
				if (slot < 3 && params[CHOKE_SWITCH+slot].getValue()) {
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

					if (params[LIMIT_SWITCH+slot].getValue()) {
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
						
						if (params[LIMIT_SWITCH+slot].getValue()) {
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
						if (params[LIMIT_SWITCH+slot].getValue()) {
							if (summedOutput > 5)
								summedOutput = 5;
							else if (summedOutput < -5)
								summedOutput = -5;
						}
						outputs[OUT_OUTPUT+slot].setVoltage(summedOutput);
					} else if (outputs[OUT_OUTPUT+slot].isConnected()) {
						if (params[LIMIT_SWITCH+slot].getValue()) {
							if (currentOutput > 5)
								currentOutput = 5;
							else if (currentOutput < -5)
								currentOutput = -5;
						}
						outputs[OUT_OUTPUT+slot].setVoltage(currentOutput);
					} else {
						summedOutput += currentOutput;
						if (params[LIMIT_SWITCH+slot].getValue()) {
							if (summedOutput > 5)
								summedOutput = 5;
							else if (summedOutput < -5)
								summedOutput = -5;
						}
					}
				break;
			}

			if (!inputs[TRIG_INPUT+slot].isConnected())
				play[slot] = false;
		}
	}
};

struct dppSlot1Display : TransparentWidget {
	DrumPlayerPlus *module;
	int frame = 0;

	dppSlot1Display() {

	}

	void onButton(const event::Button &e) override {
		/*if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);*/

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void loadSubfolder(rack::ui::Menu *menu, std::string path) {
		DrumPlayerPlus *module = dynamic_cast<DrumPlayerPlus*>(this->module);
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
					menu->addChild(createMenuItem(module->folderTreeDisplay[tempIndex][i], "", [=]() {module->loadFromPatch[0] = false;module->loadSample(module->folderTreeData[tempIndex][i],0);}));
				}
			}
		}
	}

	void createContextMenu() {
		DrumPlayerPlus *module = dynamic_cast<DrumPlayerPlus *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			menu->addChild(createMenuItem("Load Sample Slot #1", "", [=]() {
				//module->menuLoadSample(0);
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

struct dppSlot2Display : TransparentWidget {
	DrumPlayerPlus *module;
	int frame = 0;

	dppSlot2Display() {

	}

	void onButton(const event::Button &e) override {
		/*if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);*/

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void loadSubfolder(rack::ui::Menu *menu, std::string path) {
		DrumPlayerPlus *module = dynamic_cast<DrumPlayerPlus*>(this->module);
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
		DrumPlayerPlus *module = dynamic_cast<DrumPlayerPlus *>(this->module);
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

struct dppSlot3Display : TransparentWidget {
	DrumPlayerPlus *module;
	int frame = 0;

	dppSlot3Display() {

	}

	void onButton(const event::Button &e) override {
		/*if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);*/

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void loadSubfolder(rack::ui::Menu *menu, std::string path) {
		DrumPlayerPlus *module = dynamic_cast<DrumPlayerPlus*>(this->module);
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
		DrumPlayerPlus *module = dynamic_cast<DrumPlayerPlus *>(this->module);
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

struct dppSlot4Display : TransparentWidget {
	DrumPlayerPlus *module;
	int frame = 0;

	dppSlot4Display() {

	}

	void onButton(const event::Button &e) override {
		/*if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);*/

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void loadSubfolder(rack::ui::Menu *menu, std::string path) {
		DrumPlayerPlus *module = dynamic_cast<DrumPlayerPlus*>(this->module);
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
		DrumPlayerPlus *module = dynamic_cast<DrumPlayerPlus *>(this->module);
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

struct DrumPlayerPlusDisplay : TransparentWidget {
	DrumPlayerPlus *module;
	int frame = 0;

	int fileGap[4] = {0,0,0,0};
	float currTime;
	float deltaTime;
	float prevTime;

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

				switch (module->scrolling) {
					case 0:
						for (int i = 0; i < 4; i++)
							module->currFileDisplay[i] = module->fileDisplay[i].substr(0,5);
					break;
					case 1:
						currTime = system::getTime();
						deltaTime = currTime - prevTime;
						for (int i = 0; i < 4; i++) {
							if (module->scrollDisplay[i].length() > 4) {
								if (deltaTime > 0.4f) {
									prevTime = currTime;
									if (fileGap[i] > int(module->scrollDisplay[i].length()) - 5 ) {
										fileGap[i] = 0;
									}
									module->currFileDisplay[i] = module->scrollDisplay[i].substr(fileGap[i],5);
									fileGap[i]++;
								}
							} else {
								module->currFileDisplay[i] = module->scrollDisplay[i].substr(0,5);
							}
						}
					break;
				}

				nvgTextBox(args.vg, 6, 0,120, module->currFileDisplay[0].c_str(), NULL);
				nvgTextBox(args.vg, 75, 0,120, module->currFileDisplay[1].c_str(), NULL);
				nvgTextBox(args.vg, 144, 0,120, module->currFileDisplay[2].c_str(), NULL);
				nvgTextBox(args.vg, 214, 0,120, module->currFileDisplay[3].c_str(), NULL);
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

		addChild(createWidget<SickoScrewBlack1>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	  

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

		const float xDelta = 23.5;

		for (int i = 0; i < 4; i++) {
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(7.9+(xDelta*i), 21)), module, DrumPlayerPlus::TRIG_INPUT+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(17.9+(xDelta*i), 28.5)), module, DrumPlayerPlus::TRIGVOLATNV_PARAM+i));
			addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(7.9+(xDelta*i), 32)), module, DrumPlayerPlus::TRIGVOL_PARAM+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(17.9+(xDelta*i), 37)), module, DrumPlayerPlus::TRIGVOL_INPUT+i));

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(7.9+(xDelta*i), 47)), module, DrumPlayerPlus::ACC_INPUT+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(17.9+(xDelta*i), 54.5)), module, DrumPlayerPlus::ACCVOLATNV_PARAM+i));
			addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(7.9+(xDelta*i), 58)), module, DrumPlayerPlus::ACCVOL_PARAM+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(17.9+(xDelta*i), 63)), module, DrumPlayerPlus::ACCVOL_INPUT+i));

			addParam(createParamCentered<SickoKnob>(mm2px(Vec(12.9+(xDelta*i), 78.5)), module, DrumPlayerPlus::SPEED_PARAM+i));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(7.9+(xDelta*i), 88)), module, DrumPlayerPlus::SPEED_INPUT+i));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(17.9+(xDelta*i), 88)), module, DrumPlayerPlus::SPEEDATNV_PARAM+i));

			if (i<3) {
				addParam(createParamCentered<CKSS>(mm2px(Vec(7.9+(xDelta*i), 103.9)), module, DrumPlayerPlus::LIMIT_SWITCH+i));
				addParam(createParamCentered<CKSS>(mm2px(Vec(17.9+(xDelta*i), 103.9)), module, DrumPlayerPlus::CHOKE_SWITCH+i));
			} else {
				addParam(createParamCentered<CKSS>(mm2px(Vec(5+7.9+(xDelta*i), 103.9)), module, DrumPlayerPlus::LIMIT_SWITCH+i));
			}

			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(17.9+(xDelta*i), 117)), module, DrumPlayerPlus::OUT_OUTPUT+i));
		}
	}

	void appendContextMenu(Menu *menu) override {
	   	DrumPlayerPlus *module = dynamic_cast<DrumPlayerPlus*>(this->module);
			assert(module);
		
		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Slots"));
		/*
		menu->addChild(createMenuItem("1: " + module->fileDescription[0], "", [=]() {module->menuLoadSample(0);}));
		menu->addChild(createMenuItem("2: " + module->fileDescription[1], "", [=]() {module->menuLoadSample(1);}));
		menu->addChild(createMenuItem("3: " + module->fileDescription[2], "", [=]() {module->menuLoadSample(2);}));
		menu->addChild(createMenuItem("4: " + module->fileDescription[3], "", [=]() {module->menuLoadSample(3);}));
		*/
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
		menu->addChild(createMenuItem("Clear ALL slots", "", [=]() {
				for (int i = 0; i < 4; i++)
					module->clearSlot(i);
			}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Select Samples Root", "", [=]() {module->selectRootFolder();}));
		if (module->userFolder != "") {
			if (module->rootFound) {
				menu->addChild(createMenuLabel(module->userFolder));
				//menu->addChild(createMenuItem("", "Refresh", [=]() {module->refreshRootFolder();}));
			} else {
				menu->addChild(createMenuLabel("(!)"+module->userFolder));
			}
		}

		menu->addChild(new MenuSeparator());
		struct ModeItem : MenuItem {
			DrumPlayerPlus* module;
			int interpolationMode;
			void onAction(const event::Action& e) override {
				module->interpolationMode = interpolationMode;
			}
		};
		std::string modeNames[4] = {"None", "Linear 1", "Linear 2", "Hermite"};
		menu->addChild(createSubmenuItem("Interpolation", (modeNames[module->interpolationMode]), [=](Menu * menu) {
			for (int i = 0; i < 4; i++) {
				ModeItem* modeItem = createMenuItem<ModeItem>(modeNames[i]);
				modeItem->rightText = CHECKMARK(module->interpolationMode == i);
				modeItem->module = module;
				modeItem->interpolationMode = i;
				menu->addChild(modeItem);
			}
		}));

		menu->addChild(createBoolPtrMenuItem("Anti-aliasing filter", "", &module->antiAlias));

		menu->addChild(new MenuSeparator());
		struct OutsItem : MenuItem {
			DrumPlayerPlus* module;
			int outsMode;
			void onAction(const event::Action& e) override {
				module->outsMode = outsMode;
			}
		};
		std::string outsNames[3] = {"Normalled", "Solo", "Unconnected on out #4"};
		menu->addChild(createSubmenuItem("Outs mode", (outsNames[module->outsMode]), [=](Menu * menu) {
			for (int i = 0; i < 3; i++) {
				OutsItem* outsItem = createMenuItem<OutsItem>(outsNames[i]);
				outsItem->rightText = CHECKMARK(module->outsMode == i);
				outsItem->module = module;
				outsItem->outsMode = i;
				menu->addChild(outsItem);
			}
		}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Scrolling sample names", "", &module->scrolling));
		menu->addChild(createBoolPtrMenuItem("Store Samples in Patch", "", &module->sampleInPatch));
	}
};

Model *modelDrumPlayerPlus = createModel<DrumPlayerPlus, DrumPlayerPlusWidget>("DrumPlayerPlus");
