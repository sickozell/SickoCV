#define NO_INTERP 0
#define LINEAR1_INTERP 1
#define LINEAR2_INTERP 2
#define HERMITE_INTERP 3
//#define NORMALLED_OUTS 0
//#define SOLO_OUTS 1
//#define UNCONNECTED_ON_4 2

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

struct DrumPlayerMini : Module {

	#include "shapes.hpp"

	enum ParamIds {
		//TRIGVOL_PARAM,
		ACCVOL_PARAM,
		DECAY_PARAM,
		TUNE_PARAM,
		FUNC_PARAM,
		NUM_PARAMS 
	};
	enum InputIds {
		TRIG_INPUT,
		ACC_INPUT,
		DECAY_INPUT,
		VOCT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		SLOT_LIGHT,
		FUNC_LIGHT,
		NUM_LIGHTS
	};
  
	unsigned int channels;
	unsigned int sampleRate;
	drwav_uint64 totalSampleC = 0;
	drwav_uint64 totalSamples = 0;

	const unsigned int minSamplesToLoad = 9;

	vector<float> playBuffer[2];

	bool fileLoaded = false;

	bool play = false;

	double samplePos = 0;
	double sampleCoeff;
	double currentSpeed = 0;

	double tune = 0;
	double prevTune = -1;

	double voct = 0;
	double prevVoct = -1;
	double speedVoct = 0;

	double distancePos = 0;

	double prevSampleWeight = 0;
	double currSampleWeight = 0;
	double prevSamplePos = 0;

	std::string storedPath = "";
	std::string fileDescription = "--none--";
	std::string userFolder = "";
	bool rootFound = false;
	bool fileFound = false;

	std::string tempDir = "";
	vector<vector<std::string>> folderTreeData;
	vector<vector<std::string>> folderTreeDisplay;
	vector<std::string> tempTreeData;
	vector<std::string> tempTreeDisplay;

	float trigValue = 0.f;
	float prevTrigValue = 0.f;
	//bool choking = false;
	//float chokeValue = 1.f};
	bool fading = false;
	float fadingValue = 0.f;
	float fadeDecrement = 1000 / (APP->engine->getSampleRate()); // calcs volume decrement for 1ms fade 
	double fadedPosition = 0;
	
	float level;
	float currentOutput;
	//float summedOutput;

	int interpolationMode = HERMITE_INTERP;
	//int outsMode = 0;
	int antiAlias = 1;

	bool sampleInPatch = true;
	bool loadFromPatch = false;
	bool restoreLoadFromPatch = false;

	double a0, a1, a2, b1, b2, z1, z2;

	int functionButton = 0;
	bool reversePlay = false;

	float env = 1;
	float deltaValue = 0;

	float sr = float(APP->engine->getSampleRate());
	float srCoeff = 1 / sr;

	float decayValue;
	float decayInValue;
	float decayKnob = 0.f;
	float prevDecayKnob = -1.f;

	float stageLevel = -1.f;
	float stageCoeff = 0.f;

	bool dontDecay = true;

	bool logDecay = false;

	static constexpr float minStageTime = 1.f;  // in milliseconds
	static constexpr float maxStageTime = 10000.f;  // in milliseconds
	
	static constexpr float minStageTimeSec = 0.001f;  // in seconds
	static constexpr float maxStageTimeSec = 10.f;  // in seconds
	
	const float maxAdsrTime = 10.f;
	const float minAdsrTime = 0.001f;

	DrumPlayerMini() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		//for (int i = 0; i < 4; i++) {
			configInput(TRIG_INPUT, "Trig");
			//configParam(TRIGVOL_PARAM, 0.f, 2.0f, 1.0f, "Standard Level", "%", 0, 100);
			configInput(ACC_INPUT, "Accent");
			configParam(ACCVOL_PARAM, 0.f, 2.0f, 1.0f, "Accent Level", "%", 0, 100);
			//configParam(DECAY_PARAM+i, 0.01f, 2.0f, 2.0f, ("Decay #"+to_string(i+1)).c_str(), "s", 0, 1);
			configParam(DECAY_PARAM, 0.f, 1.f, 1.f, "Decay", " ms", maxStageTime / minStageTime, minStageTime);
			configInput(DECAY_INPUT,"Decay");
			//configParam(TUNE_PARAM+i, 0.01f, 2.0f, 1.0f, ("Tune #"+to_string(i+1)).c_str(), "x", 0, 1);
			configParam(TUNE_PARAM, -2.f, 2.0f, 0.f, "Tune", " semitones", 0, 12);
			configInput(VOCT_INPUT,"V/Oct");
			configSwitch(FUNC_PARAM, 0.f, 1.f, 0.f, "Reverse", {"Off", "On"});
			configOutput(OUT_OUTPUT, "Out");
		//}

		playBuffer[0].resize(0);
		playBuffer[1].resize(0);
	}

	void onReset(const ResetEvent &e) override {
		//for (int slot = 0; slot < 4; slot++) {
			clearSlot();
			play = false;
			//choking = false;
			fading = false;
		//}
		interpolationMode = HERMITE_INTERP;
		antiAlias = 1;
		//outsMode = NORMALLED_OUTS;
		sampleInPatch = true;
		system::removeRecursively(getPatchStorageDirectory().c_str());
		Module::onReset(e);
	}

	void onSampleRateChange() override {
		//for (int slot = 0; slot < 4; slot++) {
			if (fileLoaded)
				sampleCoeff = sampleRate / (APP->engine->getSampleRate());
		//}
		fadeDecrement = 1000 / (APP->engine->getSampleRate());

		sr = float(APP->engine->getSampleRate());
		srCoeff = 1 / sr;
	}

	void onAdd(const AddEvent& e) override {
		//for (int slot = 0; slot < 4; slot++) {
			if (!fileLoaded && storedPath != "") {
				std::string patchFile = system::join(getPatchStorageDirectory(), "slot.wav");
				loadFromPatch = true;
				loadSample(patchFile);
			}
		//}
		Module::onAdd(e);
	}

	void onSave(const SaveEvent& e) override {
		system::removeRecursively(getPatchStorageDirectory().c_str());
		if (sampleInPatch) {
			//for (int slot = 0; slot < 4; slot++) {
				if (fileLoaded) {
					std::string patchFile = system::join(createPatchStorageDirectory(), "slot.wav");
					saveSample(patchFile);
				}
			//}
		}
		Module::onSave(e);
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "logDecay", json_boolean(logDecay));
		json_object_set_new(rootJ, "Interpolation", json_integer(interpolationMode));
		json_object_set_new(rootJ, "AntiAlias", json_integer(antiAlias));
//		json_object_set_new(rootJ, "OutsMode", json_integer(outsMode));
		json_object_set_new(rootJ, "sampleInPatch", json_boolean(sampleInPatch));
		json_object_set_new(rootJ, "Slot", json_string(storedPath.c_str()));
		json_object_set_new(rootJ, "UserFolder", json_string(userFolder.c_str()));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t* logDecayJ = json_object_get(rootJ, "logDecay");
		if (logDecayJ)
			logDecay = json_boolean_value(logDecayJ);
		json_t* interpolationJ = json_object_get(rootJ, "Interpolation");
		if (interpolationJ)
			interpolationMode = json_integer_value(interpolationJ);
		json_t* antiAliasJ = json_object_get(rootJ, "AntiAlias");
		if (antiAliasJ)
			antiAlias = json_integer_value(antiAliasJ);
/*		json_t* outsModeJ = json_object_get(rootJ, "OutsMode");
		if (outsModeJ)
			outsMode = json_integer_value(outsModeJ);*/
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
			fileFound = true;
			//channels = c;
			sampleRate = sr * 2;
			calcBiquadLpf(20000.0, sampleRate, 1);
			playBuffer[0].clear();
			playBuffer[1].clear();

			// metamodule change
			vector<float>().swap(playBuffer[0]);
			vector<float>().swap(playBuffer[1]);

			for (unsigned int i = 0; i < tsc; i = i + c) {
				playBuffer[0].push_back(pSampleData[i] * 5);
				playBuffer[0].push_back(0);
			}
			totalSampleC = playBuffer[0].size();
			totalSamples = totalSampleC-1;
//			drwav_free(pSampleData);	// unused (old dr_wav)

			for (unsigned int i = 1; i < totalSamples; i = i + 2)		// averaging oversampled vector
				playBuffer[0][i] = playBuffer[0][i-1] * .5f + playBuffer[0][i+1] * .5f;

			playBuffer[0][totalSamples] = playBuffer[0][totalSamples-1] * .5f; // halve the last sample

			for (unsigned int i = 0; i < totalSampleC; i++)	// populating filtered vector
				playBuffer[1].push_back(biquadLpf(playBuffer[0][i]));

			sampleCoeff = sampleRate / (APP->engine->getSampleRate());		// the % distance between samples at speed 1x

			if (loadFromPatch)
				path = storedPath;

			char* pathDup = strdup(path.c_str());
			fileDescription = system::getFilename(std::string(path));
			fileDescription = fileDescription.substr(0,fileDescription.length()-4);
			free(pathDup);

			if (loadFromPatch)
				fileDescription = "(!)"+fileDescription;

			storedPath = path;

			fileLoaded = true;

		} else {

			fileFound = false;
			fileLoaded = false;
			//storedPath = path;
			if (loadFromPatch)
				path = storedPath;
			fileDescription = "(!)"+path;
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

	void saveSample(std::string path) {
		drwav_uint64 samples;

		samples = playBuffer[0].size();

		std::vector<float> data;

		for (unsigned int i = 0; i <= playBuffer[0].size(); i = i + 2)
			data.push_back(playBuffer[0][i] / 5);
			//data.push_back(playBuffer[0][i]);

		drwav_data_format format;
		format.container = drwav_container_riff;
		format.format = DR_WAVE_FORMAT_IEEE_FLOAT;

		format.channels = 1;

		samples /= 2;

		format.sampleRate = sampleRate / 2;

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
	
	void clearSlot() {
		fileLoaded = false;
		fileFound = false;
		loadFromPatch = false;
		restoreLoadFromPatch = false;
		play = false;
		storedPath = "";
		fileDescription = "--none--";
		fileFound = false;
		playBuffer[0].clear();
		playBuffer[1].clear();

		// metamodule change
		vector<float>().swap(playBuffer[0]);
		vector<float>().swap(playBuffer[1]);

		totalSampleC = 0;
	}

	static float convertCVToSec(float cv) {		
		return minStageTimeSec * std::pow(maxStageTimeSec / minStageTimeSec, cv);
	}

	void process(const ProcessArgs &args) override {
		//summedOutput = 0;
		//for (int slot = 0; slot < 4; slot++){

			functionButton = params[FUNC_PARAM].getValue();

			lights[FUNC_LIGHT].setBrightness(functionButton);

			trigValue = inputs[TRIG_INPUT].getVoltage();

			lights[SLOT_LIGHT].setBrightness(fileLoaded);

			if (trigValue >= 1 && prevTrigValue < 1) {
				if (play) {
					fading = true;
					fadingValue = 1.f;
					fadedPosition = samplePos;
				}
				play = true;
				samplePos = 0;
				currSampleWeight = sampleCoeff;
				prevSamplePos = 0;
				prevSampleWeight = 0;
				if (inputs[ACC_INPUT].getVoltage() > 1)
					level = params[ACCVOL_PARAM].getValue();
				else
					level = 1;
				
				if (!functionButton) {
					reversePlay = false;
				} else {
					reversePlay = true;
					samplePos = totalSampleC;
					prevSamplePos = totalSampleC;
				}

				stageLevel = 1;

			}
			prevTrigValue = trigValue;
			currentOutput = 0;

			if (fileLoaded && play && 
				((!reversePlay && floor(samplePos) < totalSampleC) ||
				 (reversePlay && floor(samplePos) >= 0))) {
				switch (interpolationMode) {
					case NO_INTERP:
						currentOutput = level * playBuffer[antiAlias][floor(samplePos)];
					break;

					case LINEAR1_INTERP:
						if (currSampleWeight == 0) {
							currentOutput = level * float(playBuffer[antiAlias][floor(samplePos)]);
						} else {
							currentOutput = level * float(
												(playBuffer[antiAlias][floor(samplePos)] * (1-currSampleWeight)) +
												(playBuffer[antiAlias][floor(samplePos)+1] * currSampleWeight)
											);
						}
					break;

					case LINEAR2_INTERP:
						if (currSampleWeight == 0) {
							currentOutput = level * float(playBuffer[antiAlias][floor(samplePos)]);
						} else {
							currentOutput = level * float(
												(
													(playBuffer[antiAlias][floor(prevSamplePos)] * (1-prevSampleWeight)) +
													(playBuffer[antiAlias][floor(prevSamplePos)+1] * prevSampleWeight) +
													(playBuffer[antiAlias][floor(samplePos)] * (1-currSampleWeight)) +
													(playBuffer[antiAlias][floor(samplePos)+1] * currSampleWeight)
												) / 2
											);
						}
					break;

					case HERMITE_INTERP:
						if (currSampleWeight == 0) {
							currentOutput = level * float(playBuffer[antiAlias][floor(samplePos)]);
						} else {
							if (floor(samplePos) > 1 && floor(samplePos) < totalSamples - 1) {
								/*
								currentOutput = hermiteInterpol(playBuffer[i][antiAlias][floor(samplePos[i])-1],
																playBuffer[i][antiAlias][floor(samplePos[i])],
																playBuffer[i][antiAlias][floor(samplePos[i])+1],
																playBuffer[i][antiAlias][floor(samplePos[i])+2],
																currSampleWeight[i]);
								*/
								// below is translation of the above function
								double a1 = .5F * (playBuffer[antiAlias][floor(samplePos)+1] - playBuffer[antiAlias][floor(samplePos)-1]);
								double a2 = playBuffer[antiAlias][floor(samplePos)-1] - (2.5F * playBuffer[antiAlias][floor(samplePos)]) + (2 * playBuffer[antiAlias][floor(samplePos)+1]) - (.5F * playBuffer[antiAlias][floor(samplePos)+2]);
								double a3 = (.5F * (playBuffer[antiAlias][floor(samplePos)+2] - playBuffer[antiAlias][floor(samplePos)-1])) + (1.5F * (playBuffer[antiAlias][floor(samplePos)] - playBuffer[antiAlias][floor(samplePos)+1]));
								currentOutput = level * float(
									(((((a3 * currSampleWeight) + a2) * currSampleWeight) + a1) * currSampleWeight) + playBuffer[antiAlias][floor(samplePos)]
								);
							} else {
								currentOutput = level * float(playBuffer[antiAlias][floor(samplePos)]);
							}
						}
					break;
				}

				decayKnob = params[DECAY_PARAM].getValue();
				if (decayKnob != prevDecayKnob) {
					decayValue = convertCVToSec(decayKnob);
					if (decayKnob != 1)
						dontDecay = false;
					else
						dontDecay = true;
				}
				prevDecayKnob = decayKnob;

				if (!inputs[DECAY_INPUT].isConnected()) {
					stageCoeff = srCoeff / decayValue;
				} else {

					decayInValue = decayValue + (inputs[DECAY_INPUT].getVoltage());
					if (decayInValue < minAdsrTime)
						decayInValue = minAdsrTime;
					stageCoeff = srCoeff / decayInValue;
				}

				if (!dontDecay) {
					stageLevel -= stageCoeff;

					if (stageLevel < 0) {
						stageLevel = 0;
						play = false;
					}

					currentOutput *= expTable[logDecay][int(expTableCoeff * stageLevel)];
				}

				/*
				if (slot > 0 && choking) {
					if (chokeValue < 0.0) {
						choking = false;
						play = false;
						currentOutput = 0;
					} else {
						currentOutput *= chokeValue;
						chokeValue -= fadeDecrement;
					}
				}
				*/

				prevSamplePos = samplePos;

				tune = double(params[TUNE_PARAM].getValue());
				if (tune != prevTune) {
					prevTune = tune;
					currentSpeed = double(powf(2,tune));
					if (currentSpeed > 4)
						currentSpeed = 4;
					else if (currentSpeed < 0.25)
						currentSpeed = 0.25;
				}

				if (inputs[VOCT_INPUT].isConnected()) {
					voct = inputs[VOCT_INPUT].getVoltage();
					if (voct != prevVoct) {
						speedVoct = pow(2,voct);
						prevVoct = voct;
					}
					distancePos = currentSpeed * sampleCoeff * speedVoct;
				} else {
					distancePos = currentSpeed * sampleCoeff;
				}

				if (!reversePlay)
					samplePos += distancePos;
				else
					samplePos -= distancePos;

				if (interpolationMode > NO_INTERP) {
					prevSampleWeight = currSampleWeight;
					currSampleWeight = samplePos - floor(samplePos);
				}
				
				if (fading) {
					if (fadingValue > 0) {
						fadingValue -= fadeDecrement;
						currentOutput += (playBuffer[antiAlias][floor(fadedPosition)] * fadingValue * level);

						if (!reversePlay) {
							fadedPosition += distancePos;
							if (fadedPosition > totalSamples)
								fading = false;
						} else {
							fadedPosition -= distancePos;
							if (fadedPosition <= 0)
								fading = false;
						}
					} else
						fading = false;
				}
			} else {
				//choking = false;
				play = false;
				fading = false;
			}

			/*
			switch (outsMode) {
				case NORMALLED_OUTS:
					summedOutput += currentOutput;
					if (outputs[OUT_OUTPUT].isConnected()) {
						outputs[OUT_OUTPUT].setVoltage(summedOutput);
						summedOutput = 0;
					}
				break;

				case SOLO_OUTS:
					if (outputs[OUT_OUTPUT].isConnected())
						outputs[OUT_OUTPUT].setVoltage(currentOutput);
				break;

				case UNCONNECTED_ON_4:
					if (slot == 3) {
						summedOutput += currentOutput;
						outputs[OUT_OUTPUT].setVoltage(summedOutput);
					} else if (outputs[OUT_OUTPUT].isConnected())
						outputs[OUT_OUTPUT].setVoltage(currentOutput);
					else
						summedOutput += currentOutput;
				break;
			}
			*/
			outputs[OUT_OUTPUT].setVoltage(currentOutput);

			if (!inputs[TRIG_INPUT].isConnected())
				play = false;
		//}
	}
};

struct dpDisplay : TransparentWidget {
	DrumPlayerMini *module;
	int frame = 0;

	dpDisplay() {

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
		DrumPlayerMini *module = dynamic_cast<DrumPlayerMini*>(this->module);
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
		DrumPlayerMini *module = dynamic_cast<DrumPlayerMini *>(this->module);
		assert(module);

		if (module) {
			ui::Menu *menu = createMenu();

			menu->addChild(createMenuItem("Load Sample", "", [=]() {
				//module->menuLoadSample(0);
				bool temploadFromPatch = module->loadFromPatch;
				module->loadFromPatch = false;
				module->menuLoadSample();
				if (module->restoreLoadFromPatch)
					module->loadFromPatch = temploadFromPatch;
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
							menu->addChild(createMenuItem(module->folderTreeDisplay[0][i], "", [=]() {module->loadFromPatch = false;module->loadSample(module->folderTreeData[0][i]);}));
						}
					}
				}));
			}
			if (module->fileLoaded) {
				menu->addChild(new MenuSeparator());
				menu->addChild(createMenuLabel("Current Sample:"));
				menu->addChild(createMenuLabel(module->fileDescription));
				menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot();}));
			}
		}
	}
};


struct DrumPlayerMiniWidget : ModuleWidget {
	DrumPlayerMiniWidget(DrumPlayerMini *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DrumPlayerMini.svg")));

		//addChild(createWidget<SickoScrewBlack1>(Vec(0, 0)));
		//addChild(createWidget<SickoScrewBlack2>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		{
			dpDisplay *display = new dpDisplay();
			display->box.pos = Vec(6, 15);
			display->box.size = Vec(18, 24);
			display->module = module;
			addChild(display);
		}

		const float xCenter = 5.08f;
		//const float xDelta = 16;

		const float yTrig =	19.5;
		const float yAcc = 33.7;
		const float yAccVol = 42.9;

		const float yDecKnob = 57.4;
		const float yDecIn = 66.2;

		const float yTune = 81.5;
		const float yVoct = 89.7;

		const float yChoke = 103.5;
		const float yOut = 117;

		//for (int i = 0; i < 4; i++) {
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(xCenter, 9)), module, DrumPlayerMini::SLOT_LIGHT));

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yTrig)), module, DrumPlayerMini::TRIG_INPUT));
			//addParam(createParamCentered<SickoSmallKnob>(mm2px(Vec(9, 31.5)), module, DrumPlayerMini::TRIGVOL_PARAM));

			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yAcc)), module, DrumPlayerMini::ACC_INPUT));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xCenter, yAccVol)), module, DrumPlayerMini::ACCVOL_PARAM));

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xCenter, yDecKnob)), module, DrumPlayerMini::DECAY_PARAM));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yDecIn)), module, DrumPlayerMini::DECAY_INPUT));

			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xCenter, yTune)), module, DrumPlayerMini::TUNE_PARAM));
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yVoct)), module, DrumPlayerMini::VOCT_INPUT));

			addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(xCenter, yChoke)), module, DrumPlayerMini::FUNC_PARAM, DrumPlayerMini::FUNC_LIGHT));

			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xCenter, yOut)), module, DrumPlayerMini::OUT_OUTPUT));
		//}
	}

	void appendContextMenu(Menu *menu) override {
	   	DrumPlayerMini *module = dynamic_cast<DrumPlayerMini*>(this->module);
			assert(module);
		
		menu->addChild(new MenuSeparator());
		menu->addChild(createBoolPtrMenuItem("Logarithmic Decay", "", &module->logDecay));
		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuLabel("Current Sample:"));

		menu->addChild(createMenuItem(module->fileDescription, "", [=]() {
			bool temploadFromPatch = module->loadFromPatch;
			module->loadFromPatch = false;
			module->menuLoadSample();
			if (module->restoreLoadFromPatch)
				module->loadFromPatch = temploadFromPatch;
		}));

		if (module->fileLoaded)
			menu->addChild(createMenuItem("", "Clear", [=]() {module->clearSlot();}));

		menu->addChild(new MenuSeparator());
		menu->addChild(createMenuItem("Select Samples Root", "", [=]() {module->selectRootFolder();}));
		if (module->userFolder != "") {
			if (module->rootFound) {
				menu->addChild(createMenuLabel(module->userFolder));
			} else {
				menu->addChild(createMenuLabel("(!)"+module->userFolder));
			}
		}

		menu->addChild(new MenuSeparator());
		
		struct ModeItem : MenuItem {
			DrumPlayerMini* module;
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
		menu->addChild(createBoolPtrMenuItem("Store Samples in Patch", "", &module->sampleInPatch));

		menu->addChild(new MenuSeparator());
		menu->addChild(createSubmenuItem("Tips", "", [=](Menu * menu) {
			menu->addChild(createMenuLabel("Decay knob full clockwise"));
			menu->addChild(createMenuLabel("disables decay setting"));
		}));
	}
};

Model *modelDrumPlayerMini = createModel<DrumPlayerMini, DrumPlayerMiniWidget>("DrumPlayerMini");
