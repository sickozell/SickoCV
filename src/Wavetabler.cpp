
#define STOP_STAGE 0
#define ATTACK_STAGE 1
#define DECAY_STAGE 2
#define SUSTAIN_STAGE 3
#define RELEASE_STAGE 4
#define MONOPHONIC 0
#define POLYPHONIC 1
#define FORWARD 0
#define REVERSE 1

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

struct Wavetabler : Module {
	enum ParamIds {
		VOL_PARAM,
		VOLATNV_PARAM,
		TUNE_PARAM,
		TUNEATNV_PARAM,
		ATTACK_PARAM,
		ATTACKATNV_PARAM,
		DECAY_PARAM,
		DECAYATNV_PARAM,
		SUSTAIN_PARAM,
		SUSTAINATNV_PARAM,
		RELEASE_PARAM,
		RELEASEATNV_PARAM,
		LIMIT_SWITCH,
		REV_PARAM,
		PINGPONG_PARAM,
		PREVSAMPLE_PARAM,
		NEXTSAMPLE_PARAM,
		NUM_PARAMS 
	};
	enum InputIds {
		TRIG_INPUT,
		VOL_INPUT,
		TUNE_INPUT,
		VO_INPUT,
		ATTACK_INPUT,
		DECAY_INPUT,
		SUSTAIN_INPUT,
		RELEASE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		REV_LIGHT,
		PINGPONG_LIGHT,
		CLIPPING_LIGHT,
		NUM_LIGHTS
	};
  
	unsigned int sampleRate;
	drwav_uint64 totalSampleC = 0;
	drwav_uint64 totalSamples = 0;

	const unsigned int minSamplesToLoad = 124;

	vector<float> playBuffer[2];
	vector<double> displayBuff;
	int currentDisplay = 0;
	float voctDisplay = 100.f;

	bool fileLoaded = false;

	bool play[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	bool inPause[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

	double samplePos[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	double sampleCoeff;
	double currentSpeed = 0.0;
	double distancePos[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	float tune = 0.f;
	float prevTune = -1.f;

	float voct[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevVoct[16] = {11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f, 11.f};
	float speedVoct[16];

	double prevSampleWeight[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	double currSampleWeight[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	double prevSamplePos[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	std::string storedPath = "";
	std::string fileDescription = "--none--";
	std::string fileDisplay = "";
	std::string samplerateDisplay = "";

	std::string userFolder = "";
	std::string currentFolder = "";
	vector <std::string> currentFolderV;
	int currentFile = 0;

	bool rootFound = false;
	bool fileFound = false;

	std::string tempDir = "";
	vector<vector<std::string>> folderTreeData;
	vector<vector<std::string>> folderTreeDisplay;
	vector<std::string> tempTreeData;
	vector<std::string> tempTreeDisplay;

	float trigValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float prevTrigValue[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	int antiAlias = 1;
	int polyOuts = POLYPHONIC;
	int polyMaster = POLYPHONIC;
	bool disableNav = false;
	bool sampleInPatch = true;

	bool loadFromPatch = false;
	bool restoreLoadFromPatch = false;
	
	float currentOutput;
	float sumOutput;

	//std::string debugDisplay = "X";
	//std::string debugDisplay2 = "X";
	//int debugInt = 0;

	double a0, a1, a2, b1, b2, z1, z2;

	int stage[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float stageLevel[16];

	int chan;

	float attackValue;
	float decayValue;
	float sustainValue;
	float releaseValue;
	float stageCoeff[16];
	float masterLevel[16];
	int limiter;

	bool reverseStart = false;
	int reversePlaying[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int pingpong;

	float clippingValue = 0;
	float clippingCoeff = 5 / (APP->engine->getSampleRate());	// nr of samples of 200 ms (1/0.2)
	bool clipping = false;

	bool nextSample = false;
	bool prevNextSample = false;
	bool prevSample = false;
	bool prevPrevSample = false;

	static constexpr float minStageTime = 1.f;  // in milliseconds
	static constexpr float maxStageTime = 10000.f;  // in milliseconds
	const float maxAdsrTime = 10.f;
	const float minAdsrTime = 0.001f;

#if defined(METAMODULE)
	const drwav_uint64 recordingLimit = 48000 * 60; // 60 sec limit on MM = 5.5MB
#else
	const drwav_uint64 recordingLimit = 52428800;
	// const drwav_uint64 recordingLimit = 480000; // 10 sec for test purposes
#endif

	Wavetabler() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configSwitch(PREVSAMPLE_PARAM, 0.f, 1.f, 0.f, "Previous Sample");
		configSwitch(NEXTSAMPLE_PARAM, 0.f, 1.f, 0.f, "Next Sample");

		configInput(TRIG_INPUT,"Trig/Gate");

		//******************************************************************************

		configSwitch(REV_PARAM, 0.f, 1.f, 0.f, "Playback Start", {"Forward", "Reverse"});
		configSwitch(PINGPONG_PARAM, 0.f, 1.f, 0.f, "PingPong", {"Off", "On"});

		//******************************************************************************

		configParam(ATTACK_PARAM, 0.f, 1.f, 0.f, "Attack", " ms", maxStageTime / minStageTime, minStageTime);
		configInput(ATTACK_INPUT,"Attack");
		configParam(ATTACKATNV_PARAM, -1.0f, 1.0f, 0.0f, "Attack CV","%", 0, 100);

		configParam(DECAY_PARAM, 0.f, 1.f, 0.f, "Decay", " ms", maxStageTime / minStageTime, minStageTime);
		configInput(DECAY_INPUT,"Decay");
		configParam(DECAYATNV_PARAM, -1.0f, 1.0f, 0.0f, "Decay CV","%", 0, 100);

		configParam(SUSTAIN_PARAM, 0.f, 1.0f, 1.0f, "Sustain","%", 0, 100);
		configInput(SUSTAIN_INPUT,"Sustain");
		configParam(SUSTAINATNV_PARAM, -1.0f, 1.0f, 0.0f, "Sustain CV","%", 0, 100);

		configParam(RELEASE_PARAM, 0.f, 1.f, 0.f, "Release", " ms", maxStageTime / minStageTime, minStageTime);
		configInput(RELEASE_INPUT,"Release");
		configParam(RELEASEATNV_PARAM, -1.0f, 1.0f, 0.0f, "Release CV","%", 0, 100);

		//******************************************************************************
		
		configInput(VO_INPUT,"V/Oct");

		configParam(TUNE_PARAM, -2.f, 2.f, 0.f, "Tune", " semitones", 0, 12);
		configInput(TUNE_INPUT,"Tune");
		configParam(TUNEATNV_PARAM, -1.0f, 1.0f, 0.0f, "Tune CV","%", 0, 100);

		configParam(VOL_PARAM, 0.f, 2.0f, 1.0f, "Master Volume", "%", 0, 100);
		configInput(VOL_INPUT,"Master Volume");
		configParam(VOLATNV_PARAM, -1.f, 1.0f, 0.f, "Master Volume CV", "%", 0, 100);

		configSwitch(LIMIT_SWITCH, 0.f, 1.f, 0.f, "Limit", {"Off", "±5v"});

		//******************************************************************************
		
		configOutput(OUT_OUTPUT,"Output");
		playBuffer[0].resize(0);
		playBuffer[1].resize(0);
	}

	static float convertCVToSeconds(float cv) {		
		return minStageTime * std::pow(maxStageTime / minStageTime, cv) / 1000;
	}

	void onReset(const ResetEvent &e) override {
		for (int i = 0; i < 16; i++) {
			play[i] = false;
			stage[i] = STOP_STAGE;
			stageLevel[i] = 0;
			voct[i] = 0.f;
			prevVoct[i] = 11.f;
			reversePlaying[i] = FORWARD;
		}
		clearSlot();
		antiAlias = 1;
		polyOuts = POLYPHONIC;
		polyMaster = POLYPHONIC;
		disableNav = false;
		sampleInPatch = true;
		prevTune = -1.f;
		reverseStart = false;
		system::removeRecursively(getPatchStorageDirectory().c_str());
		Module::onReset(e);
	}

	void onSampleRateChange() override {
		clippingCoeff = 5 / (APP->engine->getSampleRate());	// decrement for 200 ms clipping light (1/0.2)
		if (fileLoaded)
			sampleCoeff = sampleRate / (APP->engine->getSampleRate());			// the % distance between samples at speed 1x
	}

	void onAdd(const AddEvent& e) override {
		if (!fileLoaded) {
			std::string patchFile = system::join(getPatchStorageDirectory(), "sample.wav");
			loadFromPatch = true;
			loadSample(patchFile);
		}		
		Module::onAdd(e);
	}

	void onSave(const SaveEvent& e) override {
		system::removeRecursively(getPatchStorageDirectory().c_str());
		if (fileLoaded) {
			if (sampleInPatch) {
				std::string patchFile = system::join(createPatchStorageDirectory(), "sample.wav");
				saveSample(patchFile);
			}
		}
		Module::onSave(e);
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "AntiAlias", json_integer(antiAlias));
		json_object_set_new(rootJ, "PolyOuts", json_integer(polyOuts));
		json_object_set_new(rootJ, "PolyMaster", json_integer(polyMaster));
		json_object_set_new(rootJ, "DisableNav", json_boolean(disableNav));
		json_object_set_new(rootJ, "sampleInPatch", json_boolean(sampleInPatch));
		json_object_set_new(rootJ, "Slot", json_string(storedPath.c_str()));
		json_object_set_new(rootJ, "UserFolder", json_string(userFolder.c_str()));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t* antiAliasJ = json_object_get(rootJ, "AntiAlias");
		if (antiAliasJ)
			antiAlias = json_integer_value(antiAliasJ);
		json_t* polyOutsJ = json_object_get(rootJ, "PolyOuts");
		if (polyOutsJ)
			polyOuts = json_integer_value(polyOutsJ);
		json_t* polyMasterJ = json_object_get(rootJ, "PolyMaster");
		if (polyMasterJ)
			polyMaster = json_integer_value(polyMasterJ);
		json_t* disableNavJ = json_object_get(rootJ, "DisableNav");
		if (disableNavJ)
			disableNav = json_boolean_value(disableNavJ);
		json_t* sampleInPatchJ = json_object_get(rootJ, "sampleInPatch");
		if (sampleInPatchJ)
			sampleInPatch = json_boolean_value(sampleInPatchJ);
		json_t *slotJ = json_object_get(rootJ, "Slot");
		if (slotJ) {
			storedPath = json_string_value(slotJ);
			if (storedPath != "")
				loadSample(storedPath);
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

	void createCurrentFolder(std::string dir_path) {
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

	void menuLoadSample() {
		static const char FILE_FILTERS[] = "Wave (.wav):wav,WAV;All files (*.*):*.*";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
#endif
		fileLoaded = false;
		restoreLoadFromPatch = false;
		if (path) {
			loadFromPatch = false;
			loadSample(path);
			storedPath = std::string(path);
		} else {
			restoreLoadFromPatch = true;
			fileLoaded = true;
		}
		if (storedPath == "" || fileFound == false) {
			fileLoaded = false;
		}
		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void loadSample(std::string fromPath) {
		std::string path = fromPath;
		z1 = 0; z2 = 0;
		unsigned int c;
		unsigned int sr;
		drwav_uint64 tsc;
		float* pSampleData;
		pSampleData = drwav_open_and_read_file_f32(path.c_str(), &c, &sr, &tsc);

		if (pSampleData != NULL && tsc > minSamplesToLoad * c) {
			fileFound = true;
			//channels = c;
			sampleRate = sr * 2;
			calcBiquadLpf(20000.0, sampleRate, 1);
			playBuffer[0].clear();
			playBuffer[1].clear();
			displayBuff.clear();

			/*
			if (tsc > 52428800)
				tsc = 52428800;	// set memory allocation limit to 200Mb for samples (~18mins at 48.000khz MONO)
			*/

			if (tsc > recordingLimit)
				tsc = recordingLimit;

			for (unsigned int i=0; i < tsc; i = i + c) {
				playBuffer[0].push_back(pSampleData[i] * 5);
				playBuffer[0].push_back(0);
			}

			totalSampleC = playBuffer[0].size();
			totalSamples = totalSampleC-1;
			drwav_free(pSampleData);

			for (unsigned int i = 1; i < totalSamples; i = i + 2)	// averaging oversampled vector
				playBuffer[0][i] = playBuffer[0][i-1] * .5f + playBuffer[0][i+1] * .5f;
			
			playBuffer[0][totalSamples] = playBuffer[0][totalSamples-1] * .5f; // halve the last sample

			for (unsigned int i = 0; i < totalSampleC; i++)		// populating filtered vector
				playBuffer[1].push_back(biquadLpf(playBuffer[0][i]));

			sampleCoeff = sampleRate / (APP->engine->getSampleRate());			// the % distance between samples at 1x speed

			vector<double>().swap(displayBuff);			// creating the display vector
			for (int i = 0; i < floor(totalSampleC); i = i + floor(totalSampleC/160))
				displayBuff.push_back(playBuffer[0][i]);

			if (loadFromPatch)
				path = storedPath;

			char* pathDup = strdup(path.c_str());
			fileDescription = basename(pathDup);

			if (loadFromPatch)
				fileDescription = "(!)"+fileDescription;

			// *** CHARs CHECK according to font
			std::string tempFileDisplay = fileDescription.substr(0, fileDescription.size()-4);
			char tempFileChar;
			fileDisplay = "";
			for (int i = 0; i < int(tempFileDisplay.length()); i++) {
				tempFileChar = tempFileDisplay.at(i);
				if ( (int(tempFileChar) > 47 && int(tempFileChar) < 58)
					|| (int(tempFileChar) > 64 && int(tempFileChar) < 123)
					|| int(tempFileChar) == 45 || int(tempFileChar) == 46 || int(tempFileChar) == 95 )
					fileDisplay += tempFileChar;
			}

			fileDisplay = fileDisplay.substr(0, 20);
			samplerateDisplay = std::to_string(int(sampleRate * .5));

			free(pathDup);
			storedPath = path;

			if (!loadFromPatch) {
				currentFolder = system::getDirectory(path);
				createCurrentFolder(currentFolder);
				currentFolderV.clear();
				currentFolderV = tempTreeData;
				for (unsigned int i = 0; i < currentFolderV.size(); i++) {
					if (system::getFilename(path) == system::getFilename(currentFolderV[i])) {
						currentFile = i;
						i = currentFolderV.size();
					}
				}
			}

			fileLoaded = true;
		} else {
			fileFound = false;
			fileLoaded = false;
			//storedPath = path;
			if (loadFromPatch)
				path = storedPath;
			fileDescription = "(!)"+path;
			fileDisplay = "";
		}
	};

	void saveSample(std::string path) {
		drwav_uint64 samples;

		samples = playBuffer[0].size();

		std::vector<float> data;

		for (unsigned int i = 0; i <= playBuffer[0].size(); i = i + 2)
			data.push_back(playBuffer[0][i] / 5);

		drwav_data_format format;
		format.container = drwav_container_riff;
		format.format = DR_WAVE_FORMAT_IEEE_FLOAT;

		format.channels = 1;

		samples /= 2;

		format.sampleRate = sampleRate / 2;

		format.bitsPerSample = 32;

		if (path.substr(path.size() - 4) != ".wav" && path.substr(path.size() - 4) != ".WAV")
			path += ".wav";

		drwav *pWav = drwav_open_file_write(path.c_str(), &format);
		drwav_write(pWav, samples, data.data());
		drwav_close(pWav);

		data.clear();
	}
	
	void clearSlot() {
		fileLoaded = false;
		fileFound = false;
		loadFromPatch = false;
		restoreLoadFromPatch = false;
		storedPath = "";
		fileDescription = "--none--";
		fileDisplay = "";
		playBuffer[0].clear();
		playBuffer[1].clear();
		totalSampleC = 0;
		totalSamples = 0;
	}

	bool isPolyOuts() {
		return polyOuts;
	}

	void setPolyOuts(bool poly) {
		if (poly) 
			polyOuts = 1;
		else {
			polyOuts = 0;
			outputs[OUT_OUTPUT].setChannels(1);
		}
	}

	void process(const ProcessArgs &args) override {

		//if (!disableNav) {
		if (!disableNav && !loadFromPatch) {
			nextSample = params[NEXTSAMPLE_PARAM].getValue();
			if (fileLoaded && nextSample && !prevNextSample) {
				for (int i = 0; i < 16; i++)
					play[i] = false;
				currentFile++;
				if (currentFile >= int(currentFolderV.size()))
					currentFile = 0;
				loadSample(currentFolderV[currentFile]);
			}
			prevNextSample = nextSample;

			prevSample = params[PREVSAMPLE_PARAM].getValue();
			if (fileLoaded && prevSample && !prevPrevSample) {
				for (int i = 0; i < 16; i++)
					play[i] = false;
				currentFile--;
				if (currentFile < 0)
					currentFile = currentFolderV.size()-1;
				loadSample(currentFolderV[currentFile]);
			}
			prevPrevSample = prevSample;
		}

		reverseStart = params[REV_PARAM].getValue();
		lights[REV_LIGHT].setBrightness(reverseStart);

		pingpong = params[PINGPONG_PARAM].getValue();
		lights[PINGPONG_LIGHT].setBrightness(pingpong);

		chan = std::max(1, inputs[VO_INPUT].getChannels());
		
		if (!fileLoaded) {

			for (int c = 0; c < chan; c++) {
				play[c] = false;
				stage[c] = STOP_STAGE;
				stageLevel[c] = 0;
				voct[c] = 0.f;
				prevVoct[c] = 11.f;
				outputs[OUT_OUTPUT].setVoltage(0, c);
			}
			
		} else {
	
			sustainValue = params[SUSTAIN_PARAM].getValue() + (inputs[SUSTAIN_INPUT].getVoltage() * params[SUSTAINATNV_PARAM].getValue() * 0.1);
			if (sustainValue > 1)
				sustainValue = 1;
			else if (sustainValue < 0)
				sustainValue = 0;

			limiter = params[LIMIT_SWITCH].getValue();

			tune = params[TUNE_PARAM].getValue() + (inputs[TUNE_INPUT].getVoltage() * params[TUNEATNV_PARAM].getValue() * 0.2);
			if (tune != prevTune) {
				prevTune = tune;
				currentSpeed = double(powf(2,tune));
				if (currentSpeed > 4)
					currentSpeed = 4;
				else if (currentSpeed < 0.25)
					currentSpeed = 0.25;
			}

			sumOutput = 0;

			// START CHANNEL MANAGEMENT

			voctDisplay = 100.f;

			for (int c = 0; c < chan; c++) {
				
				if (polyMaster) 
					masterLevel[c] = params[VOL_PARAM].getValue() + (inputs[VOL_INPUT].getVoltage(c) * params[VOLATNV_PARAM].getValue() * 0.2);
				else
					masterLevel[c] = params[VOL_PARAM].getValue() + (inputs[VOL_INPUT].getVoltage(0) * params[VOLATNV_PARAM].getValue() * 0.2);
				
				if (masterLevel[c] > 2)
					masterLevel[c] = 2;
				else if (masterLevel[c] < 0)
					masterLevel[c] = 0;

				trigValue[c] = inputs[TRIG_INPUT].getVoltage(c);

				if (trigValue[c] >= 1) {
					if (!play[c]) {
						play[c] = true;
						if (reverseStart) {
							reversePlaying[c] = REVERSE;
							samplePos[c] = totalSamples-1;
							currSampleWeight[c] = sampleCoeff;
							prevSamplePos[c] = totalSamples;
							prevSampleWeight[c] = 0;
						} else {
							reversePlaying[c] = FORWARD;
							samplePos[c] = 1;
							currSampleWeight[c] = sampleCoeff;
							prevSamplePos[c] = 0;
							prevSampleWeight[c] = 0;
						}
						stage[c] = ATTACK_STAGE;
						attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
						if (attackValue > maxAdsrTime) {
							attackValue = maxAdsrTime;
						} else if (attackValue < minAdsrTime) {
							attackValue = minAdsrTime;
						}
						stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);
					} else {
						if (stage[c] == RELEASE_STAGE) {
							stage[c] = ATTACK_STAGE;
							attackValue = convertCVToSeconds(params[ATTACK_PARAM].getValue()) + (inputs[ATTACK_INPUT].getVoltage() * params[ATTACKATNV_PARAM].getValue());
							if (attackValue > maxAdsrTime) {
								attackValue = maxAdsrTime;
							} else if (attackValue < minAdsrTime) {
								attackValue = minAdsrTime;
							}
							stageCoeff[c] = (1-stageLevel[c]) / (args.sampleRate * attackValue);
						}
					}
				} else {
					if (play[c]) {
						if (stage[c] != RELEASE_STAGE) {
							stage[c]=RELEASE_STAGE;
							releaseValue = convertCVToSeconds(params[RELEASE_PARAM].getValue()) + (inputs[RELEASE_INPUT].getVoltage() * params[RELEASEATNV_PARAM].getValue());
							if (releaseValue > maxAdsrTime) {
								releaseValue = maxAdsrTime;
							} else 	if (releaseValue < minAdsrTime) {
								releaseValue = minAdsrTime;
							}
							stageCoeff[c] = stageLevel[c] / (args.sampleRate * releaseValue);
						}
					}
				}

				currentOutput = 0;

				if (play[c]) {
					if (inputs[VO_INPUT].isConnected()) {
						voct[c] = inputs[VO_INPUT].getVoltage(c);
						if (voct[c] != prevVoct[c]) {
							speedVoct[c] = pow(2,voct[c]);
							prevVoct[c] = voct[c];
						}
						distancePos[c] = currentSpeed * sampleCoeff * speedVoct[c];
					} else
						distancePos[c] = currentSpeed * sampleCoeff;


					switch (reversePlaying[c]) {
						case FORWARD:		// FORWARD PLAYBACK
							if (samplePos[c] > floor(totalSamples - distancePos[c])) {		// *** REACHED END OF LOOP ***
								if (pingpong) {
									reversePlaying[c] = REVERSE;
									samplePos[c] = totalSamples-1;
									currSampleWeight[c] = sampleCoeff;
									prevSamplePos[c] = totalSamples;
									prevSampleWeight[c] = 0;
								} else {
									samplePos[c] = 1;
									currSampleWeight[c] = sampleCoeff;
									prevSamplePos[c] = 0;
									prevSampleWeight[c] = 0;
								}
							}
						break;

						case REVERSE:		// REVERSE PLAYBACK
							if (samplePos[c] < distancePos[c]) {	// *** REACHED BEGIN OF LOOP ***
								if (pingpong) {
									reversePlaying[c] = FORWARD;
									samplePos[c] = 1;
									currSampleWeight[c] = sampleCoeff;
									prevSamplePos[c] = 0;
									prevSampleWeight[c] = 0;
								} else {
									samplePos[c] = totalSamples-1;
									currSampleWeight[c] = sampleCoeff;
									prevSamplePos[c] = totalSamples;
									prevSampleWeight[c] = 0;
								}
							}
						break;
					}

					if (play[c]) {									// it's false only if end of sample has reached, see above
						if (currSampleWeight[c] == 0) {	// if no distance between samples, it means that speed is 1 and samplerates match -> no interpolation
							currentOutput = playBuffer[antiAlias][floor(samplePos[c])];
							
						} else {
							if (floor(samplePos[c]) > 0 && floor(samplePos[c]) < totalSamples - 1) {
								/*
								currentOutput = hermiteInterpol(playBuffer[i][antiAlias][floor(samplePos[i])-1],
																playBuffer[i][antiAlias][floor(samplePos[i])],
																playBuffer[i][antiAlias][floor(samplePos[i])+1],
																playBuffer[i][antiAlias][floor(samplePos[i])+2],
																currSampleWeight[i]);
								*/
								// below is translation of the above function
								double a1 = .5F * (playBuffer[antiAlias][floor(samplePos[c])+1] - playBuffer[antiAlias][floor(samplePos[c])-1]);
								double a2 = playBuffer[antiAlias][floor(samplePos[c])-1] - (2.5F * playBuffer[antiAlias][floor(samplePos[c])]) + (2 * playBuffer[antiAlias][floor(samplePos[c])+1]) - (.5F * playBuffer[antiAlias][floor(samplePos[c])+2]);
								double a3 = (.5F * (playBuffer[antiAlias][floor(samplePos[c])+2] - playBuffer[antiAlias][floor(samplePos[c])-1])) + (1.5F * (playBuffer[antiAlias][floor(samplePos[c])] - playBuffer[antiAlias][floor(samplePos[c])+1]));
								currentOutput = (((((a3 * currSampleWeight[c]) + a2) * currSampleWeight[c]) + a1) * currSampleWeight[c]) + playBuffer[antiAlias][floor(samplePos[c])];
								
							} else { // if playing sample is the first or one of the last 3 -> no interpolation
								currentOutput = playBuffer[antiAlias][floor(samplePos[c])];
							}
						}

						prevSamplePos[c] = samplePos[c];
						
						if (reversePlaying[c])
							samplePos[c] -= distancePos[c];
						else
							samplePos[c] += distancePos[c];

						prevSampleWeight[c] = currSampleWeight[c];
						currSampleWeight[c] = samplePos[c] - floor(samplePos[c]);

						switch (stage[c]) {
							case ATTACK_STAGE:
								stageLevel[c] += stageCoeff[c];
								if (stageLevel[c] > 1) {
									stageLevel[c] = 1;
									stage[c] = DECAY_STAGE;
									decayValue = convertCVToSeconds(params[DECAY_PARAM].getValue()) + (inputs[DECAY_INPUT].getVoltage() * params[DECAYATNV_PARAM].getValue());
									if (decayValue > maxAdsrTime) {
										decayValue = maxAdsrTime;
									} else 	if (decayValue < minAdsrTime) {
										decayValue = minAdsrTime;
									}
									stageCoeff[c] = (1-sustainValue) / (args.sampleRate * decayValue);
								}
							break;

							case DECAY_STAGE:
								stageLevel[c] -= stageCoeff[c];
								if (stageLevel[c] <= sustainValue) {
									stageLevel[c] = sustainValue;
									stage[c] = SUSTAIN_STAGE;
								}
							break;

							case SUSTAIN_STAGE:
								stageLevel[c] = sustainValue;
							break;

							case RELEASE_STAGE:
								stageLevel[c] -= stageCoeff[c];
								if (stageLevel[c] < 0) {
									stageLevel[c] = 0;
									stage[c] = STOP_STAGE;
									play[c] = false;
								}
							break;
						}

						if (reversePlaying[c]) {
							currentOutput *= stageLevel[c] * masterLevel[c] * -1;
						} else {
							currentOutput *= stageLevel[c] * masterLevel[c];
						}
					}

				} else {
					play[c] = false;
				}

				switch (polyOuts) {
					case MONOPHONIC:										// monophonic CABLES
						sumOutput += currentOutput;
					break;

					case POLYPHONIC:										// polyphonic CABLES
						if (limiter) {					// *** hard clip functionality ***
							if (currentOutput > 5)
								currentOutput = 5;
							else if (currentOutput < -5)
								currentOutput = -5;
						}

						// *** CLIPPING LIGHT ***
						if (currentOutput < -5 || currentOutput > 5) {
							clipping = true;
							clippingValue = 0;
						}

						if (clipping && clippingValue < 1)
							clippingValue += clippingCoeff;
						else {
							clipping = false;
							clippingValue = 1;
						}

						if (outputs[OUT_OUTPUT].isConnected())
							outputs[OUT_OUTPUT].setVoltage(currentOutput, c);

					break;
				}

				if (!inputs[TRIG_INPUT].isConnected()) {
					play[c] = false;
				}

			} // END OF CHANNEL MANAGEMENT

			switch (polyOuts) {
				case MONOPHONIC:			// monophonic CABLES
					if (limiter) {					// *** hard clip functionality ***
						if (sumOutput > 5)
							sumOutput = 5;
						else if (sumOutput < -5)
							sumOutput = -5;
					}

					// *** CLIPPING LIGHT ***
					if (sumOutput < -5 || sumOutput > 5) {
						clipping = true;
						clippingValue = 0;
					}

					if (clipping && clippingValue < 1)
						clippingValue += clippingCoeff;
					else {
						clipping = false;
						clippingValue = 1;
					}

					if (outputs[OUT_OUTPUT].isConnected()) {
						outputs[OUT_OUTPUT].setVoltage(sumOutput, 0);
						outputs[OUT_OUTPUT].setChannels(1);
					}

				break;

				case POLYPHONIC:			// polyphonic CABLES
					outputs[OUT_OUTPUT].setChannels(chan);
				break;
			}
		}

	}
};

struct WavetablerDisplay : TransparentWidget {
	Wavetabler *module;
	int frame = 0;
	WavetablerDisplay() {
	}

	void onButton(const event::Button &e) override {
		/*if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
			e.consume(this);*/

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
			createContextMenu();
			e.consume(this);
		}
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {

				if (module->clipping)
					module->lights[module->CLIPPING_LIGHT].setBrightness(1);
				else
					module->lights[module->CLIPPING_LIGHT].setBrightness(0);

				shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/DSEG7ClassicMini-BoldItalic.ttf"));
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgFillColor(args.vg, nvgRGBA(0xdd, 0x33, 0x33, 0xff)); 
				nvgTextBox(args.vg, 7, 16,247, module->fileDisplay.c_str(), NULL);
				
				//nvgTextBox(args.vg, 9, 26,120, module->debugDisplay.c_str(), NULL);
				//nvgTextBox(args.vg, 109, 26,120, module->debugDisplay2.c_str(), NULL);

				// Zero line
				nvgStrokeColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x40));
				{
					nvgBeginPath(args.vg);
					nvgMoveTo(args.vg, 7, 49);
					nvgLineTo(args.vg, 167, 49);
					nvgClosePath(args.vg);
				}
				nvgStroke(args.vg);
		
				if (module->fileLoaded) {
					// Waveform
					nvgStrokeColor(args.vg, nvgRGBA(0x22, 0x44, 0xc9, 0xc0));
					nvgSave(args.vg);
					Rect b = Rect(Vec(7, 22), Vec(160, 54));
					nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
					nvgBeginPath(args.vg);
					for (unsigned int i = 0; i < module->displayBuff.size(); i++) {
						float x, y;
						x = (float)i / (module->displayBuff.size() - 1);
						y = module->displayBuff[i] / 10.0 + 0.5;
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
			}
		}
		Widget::drawLayer(args, layer);
	}

	void loadSubfolder(rack::ui::Menu *menu, std::string path) {
		Wavetabler *module = dynamic_cast<Wavetabler*>(this->module);
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
					menu->addChild(createMenuItem(module->folderTreeDisplay[tempIndex][i], "", [=]() {module->loadFromPatch = false;module->loadSample(module->folderTreeData[tempIndex][i]);}));
				}
			}
		}
	}

	void createContextMenu() {
		Wavetabler *module = dynamic_cast<Wavetabler *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			menu->addChild(createMenuItem("Load Sample", "", [=]() {
				//module->menuLoadSample();
				bool temploadFromPatch = module->loadFromPatch;
				module->loadFromPatch = false;
				module->menuLoadSample();
				if (module->restoreLoadFromPatch)
					module->loadFromPatch = temploadFromPatch;
			}));

			if (module->folderTreeData.size() > 0) {
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
							menu->addChild(createMenuItem(module->folderTreeDisplay[0][i], "", [=]() {module->loadFromPatch = false;module->loadSample(module->folderTreeData[0][i]);}));
						}
					}
				}));
			}

			if (module->fileLoaded) {
				menu->addChild(new MenuSeparator());
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription));
				menu->addChild(createMenuLabel(" " + module->samplerateDisplay));
				menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot();}));
			}
		}
	}
};

struct WavetablerWidget : ModuleWidget {
	WavetablerWidget(Wavetabler *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Wavetabler.svg")));

		addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
		addChild(createWidget<SickoScrewBlack2>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<SickoScrewBlack1>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	  

		{
			WavetablerDisplay *display = new WavetablerDisplay();
			display->box.pos = Vec(3, 24);
			display->box.size = Vec(247, 80);
			display->module = module;
			addChild(display);
		}

		const float yTunVol = 108;

		addParam(createParamCentered<VCVButton>(mm2px(Vec(9, 39)), module, Wavetabler::PREVSAMPLE_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(17, 39)), module, Wavetabler::NEXTSAMPLE_PARAM));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(39, 39)), module, Wavetabler::REV_PARAM, Wavetabler::REV_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(56, 39)), module, Wavetabler::PINGPONG_PARAM, Wavetabler::PINGPONG_LIGHT));
		//----------------------------------------------------------------------------------------------------------------------------

		const float adsrXstart = 9.6;
		const float adsrXdelta = 14;
		const float adsrCtrlY = 54.2;
		const float adsrAtnvY = 65.9;
		const float adsrInputY = 75;

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(adsrXstart, adsrCtrlY)), module, Wavetabler::ATTACK_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(adsrXstart, adsrAtnvY)), module, Wavetabler::ATTACKATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(adsrXstart, adsrInputY)), module, Wavetabler::ATTACK_INPUT));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(adsrXstart+(adsrXdelta), adsrCtrlY)), module, Wavetabler::DECAY_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(adsrXstart+(adsrXdelta), adsrAtnvY)), module, Wavetabler::DECAYATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(adsrXstart+(adsrXdelta), adsrInputY)), module, Wavetabler::DECAY_INPUT));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(adsrXstart+(adsrXdelta*2), adsrCtrlY)), module, Wavetabler::SUSTAIN_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(adsrXstart+(adsrXdelta*2), adsrAtnvY)), module, Wavetabler::SUSTAINATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(adsrXstart+(adsrXdelta*2), adsrInputY)), module, Wavetabler::SUSTAIN_INPUT));

		addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(adsrXstart+(adsrXdelta*3), adsrCtrlY)), module, Wavetabler::RELEASE_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(adsrXstart+(adsrXdelta*3), adsrAtnvY)), module, Wavetabler::RELEASEATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(adsrXstart+(adsrXdelta*3), adsrInputY)), module, Wavetabler::RELEASE_INPUT));

		//----------------------------------------------------------------------------------------------------------------------------
		const float tuneYdelta = 41.691;
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(9, adsrCtrlY+tuneYdelta)), module, Wavetabler::TRIG_INPUT));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(9, 113)), module, Wavetabler::VO_INPUT));

		addParam(createParamCentered<SickoKnob>(mm2px(Vec(22, adsrCtrlY+tuneYdelta)), module, Wavetabler::TUNE_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(22, adsrAtnvY+tuneYdelta)), module, Wavetabler::TUNEATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(22, adsrInputY+tuneYdelta)), module, Wavetabler::TUNE_INPUT));
		
		addParam(createParamCentered<SickoKnob>(mm2px(Vec(38.5, adsrCtrlY+tuneYdelta)), module, Wavetabler::VOL_PARAM));
		addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(38.5, adsrAtnvY+tuneYdelta)), module, Wavetabler::VOLATNV_PARAM));
		addInput(createInputCentered<SickoInPort>(mm2px(Vec(38.5, adsrInputY+tuneYdelta)), module, Wavetabler::VOL_INPUT));
		

		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(46, yTunVol-16.5)), module, Wavetabler::CLIPPING_LIGHT));
		addParam(createParamCentered<CKSS>(mm2px(Vec(53, 98.5)), module, Wavetabler::LIMIT_SWITCH));
		//----------------------------------------------------------------------------------------------------------------------------

		addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(53.6, adsrInputY+tuneYdelta)), module, Wavetabler::OUT_OUTPUT));

	}

	void loadSubfolder(rack::ui::Menu *menu, std::string path) {
		Wavetabler *module = dynamic_cast<Wavetabler*>(this->module);
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
					menu->addChild(createMenuItem(module->folderTreeDisplay[tempIndex][i], "", [=]() {module->loadFromPatch = false;module->loadSample(module->folderTreeData[tempIndex][i]);}));
				}
			}
		}
	}

	void appendContextMenu(Menu *menu) override {
	   	Wavetabler *module = dynamic_cast<Wavetabler*>(this->module);
			assert(module);
		
		menu->addChild(new MenuSeparator());

		menu->addChild(createMenuItem("Load Sample", "", [=]() {
			//module->menuLoadSample();
			bool temploadFromPatch = module->loadFromPatch;
			module->loadFromPatch = false;
			module->menuLoadSample();
			if (module->restoreLoadFromPatch)
				module->loadFromPatch = temploadFromPatch;
		}));

		if (module->folderTreeData.size() > 0) {
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
						menu->addChild(createMenuItem(module->folderTreeDisplay[0][i], "", [=]() {module->loadFromPatch = false;module->loadSample(module->folderTreeData[0][i]);}));
					}
				}
			}));
		}

		if (module->storedPath != "") {
			menu->addChild(new MenuSeparator());
			if (module->fileLoaded) {
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription));
				menu->addChild(createMenuLabel(" " + module->samplerateDisplay));
			} else {
				menu->addChild(createMenuLabel("Sample ERROR:"));
				menu->addChild(createMenuLabel(module->fileDescription));
			}
			menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot();}));
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Select Samples Root", "", [=]() {module->selectRootFolder();}));

		if (module->userFolder != "") {
			menu->addChild(createMenuLabel(module->userFolder));
			//menu->addChild(createMenuItem("", "Refresh", [=]() {module->refreshRootFolder();}));
		}

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Anti-aliasing filter", "", &module->antiAlias));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolMenuItem("Polyphonic OUTs", "", [=]() {
				return module->isPolyOuts();
			}, [=](bool poly) {
				module->setPolyOuts(poly);
		}));
		menu->addChild(createBoolPtrMenuItem("Polyphonic Master IN", "", &module->polyMaster));

		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Disable NAV Buttons", "", &module->disableNav));
		menu->addChild(createBoolPtrMenuItem("Store Sample in Patch", "", &module->sampleInPatch));
	}
};

Model *modelWavetabler = createModel<Wavetabler, WavetablerWidget>("Wavetabler");
