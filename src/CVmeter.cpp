#define TRIG_MODE 0
#define CV_MODE 1
#define IDLE 0
#define RUNNING 1
#define A_BEFORE_B 1
#define A_AFTER_B 2
#define A_EQUAL_B 3
#define LAST_A 1
#define LAST_B 2
#define LAST_BOTH 3

#define CLOCK_BPM 0
#define TEN_SECONDS_BPM 1
#define THIRTY_SECONDS_BPM 2
#define ONE_MINUTE_BPM 3
#define FIVE_MINUTES_BPM 4
#define TEN_MINUTES_BPM 5
#define TIME 6

#include "plugin.hpp"
#include "cmath"

using namespace std;

struct CVmeter : Module {

	#include "notes.hpp"

	enum ParamIds {
		MODE_A_SWITCH,
		MODE_B_SWITCH,
		RESET_BUT_PARAM,
		FREEZE_PARAM,
		NUM_PARAMS 
	};
	enum InputIds {
		A_INPUT,
		B_INPUT,
		RESET_INPUT,
		FREEZE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		RESET_BUT_LIGHT,
		FREEZE_LIGHT,
		NUM_LIGHTS
	};

	//**************************************************************
	//  DEBUG 

	std::string debugDisplay[26] = {"X" ,"X" ,"X", "X", "X", "X", "X", "X", "X", "X", 
									"X" ,"X" ,"X", "X", "X", "X", "X", "X", "X", "X",
									"X", "X", "X", "X", "X", "X"};
	
	int debugInt = 0;
	bool debugBool = false;

 	//**************************************************************
	//   
	double currentSampleRate = (double)APP->engine->getSampleRate();
	double sampleRateCoeff = (double)APP->engine->getSampleRate() * 60;

	float inputA = 0.f;
	float prevInputA = 0.f;

	float inputB = 0.f;
	float prevInputB = 0.f;

	int modeA = TRIG_MODE;
	int modeB = TRIG_MODE;

	int status = IDLE;
	int statusA = IDLE;
	int statusB = IDLE;

	bool triggeredNow = false;

	long delayCount = 0;

	long recSampleCount = 0;

	float voltA = 0.f;
	float voltB = 0.f;

	int noteA = 72;
	int noteB = 72;

	int midiNoteA = 72;
	int midiNoteB = 72;

	int midiCcA = 0;
	int midiCcB = 0;

	double bpmA = 0.f;
	double bpmB = 0.f;

	int delayA = 0;
	int delayB = 0;

	
	/*std::string displayVoltA = "";
	std::string displayVoltB = "";
	std::string displayNoteA = "";
	std::string displayNoteB = "";
	std::string displayBpmA = "";
	std::string displayBpmB = "";*/

	
	long sampleClockA = 0;
	long sampleClockB = 0;

	double sumBpmCountA = 0;
	double sumBpmA = 0;
	double avgBpmA = 0;
	double avgBpm1MinA = 0;
	

	
	double sumBpmCountB = 0;
	double sumBpmB = 0;
	double avgBpmB = 0;
	double avgBpm1MinB = 0;
	

	bool warning = true;

	int lastTrig = IDLE;

	std::string msg[5] = {"", "", "", "", ""};
	//std::string lastLog = "";

	//int sampleCountClock = 0;
	float rstTrig = 0;
	float prevRstTrig = 0;
	int pulseNr = 0;
	int pulseSample = 0;
	int prevPulseSample = 0;
	int pulseDistance[25] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	int minDist = 999999;
	int maxDist = 0;
	int deltaDist = 0;
	bool toWrite = true;
	bool firstAbsClock = true;

	bool freeze = false;
	float frzTrig = 0.f;
	float prevFrzTrig = 0.f;

	CVmeter() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		configInput(A_INPUT,"A");
		configInput(B_INPUT,"B");
		configSwitch(MODE_A_SWITCH, 0.f, 1.f, 1.f, "Mode A", {"Trig mode", "CV mode"});
		configSwitch(MODE_B_SWITCH, 0.f, 1.f, 1.f, "Mode B", {"Trig mode", "CV mode"});
		configSwitch(RESET_BUT_PARAM, 0.f, 1.f, 0.f, "Reset", {"OFF", "ON"});
		configSwitch(FREEZE_PARAM, 0.f, 1.f, 0.f, "Freeze", {"OFF", "ON"});
		configInput(RESET_INPUT,"Reset");
		configInput(FREEZE_INPUT,"Freeze");

	}

	void onSampleRateChange() override {
		currentSampleRate = (double)APP->engine->getSampleRate();
		sampleRateCoeff = (double)APP->engine->getSampleRate() * 60;
	}

	/*
	void updateDisplay(std::string lastMsg) {
		msg[0] = msg[1];
		msg[1] = msg[2];
		msg[2] = msg[3];
		msg[3] = msg[4];
		msg[4] = lastMsg;
	}*/

	void process(const ProcessArgs &args) override {
		

		rstTrig = params[RESET_BUT_PARAM].getValue() + inputs[RESET_INPUT].getVoltage();
		if (rstTrig >= 1 && prevRstTrig < 1) {
			rstTrig = 1;

			inputA = 0.f;
			inputB = 0.f;
			prevInputA = -1.f;
			prevInputB = -1.f;
			bpmA = 0;
			bpmB = 0;
			sumBpmCountA = 0;
			sumBpmCountB = 0;
			avgBpmA = 0;
			avgBpmB = 0;
			status = IDLE;
			statusA = IDLE;
			statusB = IDLE;

			noteA = 0;
			noteB = 0;
			midiNoteA = 0;
			midiNoteB = 0;
			midiCcA = 0;
			midiCcB = 0;

			delayA = 0;
			delayB = 0;

			/*
			for (int i = 0; i < 25; i++) {
				
				debugDisplay[i] = "X";
				pulseDistance[i] = 0;
				pulseNr = 0;
				sampleCountClock = 0;
				pulseSample = 0;
				prevPulseSample = 0;
				minDist = 999999;
				maxDist = 0;
				toWrite = true;
				firstAbsClock = true;
			}
			*/

		}

		prevRstTrig = rstTrig;

		lights[RESET_BUT_LIGHT].setBrightness(rstTrig);

		modeA = (int)params[MODE_A_SWITCH].getValue();
		modeB = (int)params[MODE_B_SWITCH].getValue();


		frzTrig = inputs[FREEZE_INPUT].getVoltage() + params[FREEZE_PARAM].getValue();
		if (frzTrig >= 1.f && prevFrzTrig < 1.f)
			freeze = !freeze;

		prevFrzTrig = frzTrig;

		lights[FREEZE_LIGHT].setBrightness(freeze);


		if (!freeze) {
			inputA = inputs[A_INPUT].getVoltage();
			voltA = inputA;
			if (voltA > 10.f)
				voltA = 10.f;
			else if (voltA < -10.f)
				voltA = -10.f;
			
			inputB = inputs[B_INPUT].getVoltage();
			voltB = inputB;
			if (voltB > 10.f)
				voltB = 10.f;
			else if (voltB < -10.f)
				voltB = -10.f;

			triggeredNow = false;

			if (inputA != prevInputA) {

				midiNoteA = int((voltA + 6.041666666666667f) * 12);
				if (midiNoteA < 0)
					midiNoteA = 0;
				else if (midiNoteA > 127)
					midiNoteA = 127;

				midiCcA = round((voltA * 12.7));
				if (midiCcA > 127)
					midiCcA = 127;
				else if (midiCcA < 0)
					midiCcA = 0;

				switch (modeA) {
					case TRIG_MODE:

						if (inputA >= 1.f && prevInputA < 1.f) {
							
							if (statusA != IDLE) {
								//bpmA = round(sampleRateCoeff / sampleClockA);
								bpmA = sampleRateCoeff / sampleClockA;
								sumBpmCountA++;
								sumBpmA += bpmA;
								avgBpmA = sumBpmA / sumBpmCountA;

								
							}
							statusA = RUNNING;
							switch (status) {
								case IDLE:

									status = A_BEFORE_B;
									delayCount = 0;
									
								break;

								case A_BEFORE_B:
									delayCount = 0;
								break;

								case A_AFTER_B:
									if (lastTrig == LAST_A) {
										status = A_BEFORE_B;
										delayCount = 0;
									} else if (lastTrig == LAST_B) {
										recSampleCount = delayCount;
									}
								break;

								case A_EQUAL_B:
									status = A_BEFORE_B;
									delayCount = 0;
								break;
							}
							lastTrig = LAST_A;
							sampleClockA = 0;
							triggeredNow = true;
							warning = true;
						}
						

					break;

					case CV_MODE:

						bpmA = 120 * pow (2.0f, voltA);
						if (bpmA > 999)
							bpmA = 999;
							
						statusA = RUNNING;
						
						switch (status) {
							case IDLE:

								status = A_BEFORE_B;
								delayCount = 0;
								
							break;

							case A_BEFORE_B:
								delayCount = 0;
							break;

							case A_AFTER_B:
								if (lastTrig == LAST_A) {
									status = A_BEFORE_B;
									delayCount = 0;
								} else if (lastTrig == LAST_B) {
									recSampleCount = delayCount;
								}
							break;

							case A_EQUAL_B:
								status = A_BEFORE_B;
								delayCount = 0;
							break;
						}
						lastTrig = LAST_A;
						sampleClockA = 0;
						triggeredNow = true;
						warning = true;
					
						
					break;
				}
			}

			if (inputB != prevInputB) {
				
				midiNoteB = int((voltB + 6.041666666666667f) * 12);
				if (midiNoteB < 0)
					midiNoteB = 0;
				else if (midiNoteB > 127)
					midiNoteB = 127;
				//noteB = noteNames[midiB];
				midiCcB = round((voltB * 12.7));
				if (midiCcB > 127)
					midiCcB = 127;
				else if (midiCcB < 0)
					midiCcB = 0;

				switch (modeB) {
					case TRIG_MODE:
						if (inputB >= 1.f && prevInputB < 1.f) {
							if (statusB != IDLE) {
								//bpmA = round(sampleRateCoeff / sampleClockA);
								bpmB = sampleRateCoeff / sampleClockB;
								sumBpmCountB++;
								sumBpmB += bpmB;
								avgBpmB = sumBpmB / sumBpmCountB;


							}

							statusB = RUNNING;
							if (triggeredNow) {
								lastTrig = LAST_BOTH;
								status = A_EQUAL_B;
								delayCount = 0;
								
							} else {

								switch (status) {
									case IDLE:
										status = A_AFTER_B;
										delayCount = 0;
										
									break;

									case A_BEFORE_B:
										if (lastTrig == LAST_A) {
											recSampleCount = delayCount;
										} else if (lastTrig == LAST_B) {
											status = A_AFTER_B;
											delayCount = 0;
										}
									break;

									case A_AFTER_B:
										delayCount = 0;
									break;
								
									case A_EQUAL_B:
										status = A_AFTER_B;
										delayCount = 0;
									break;
								}
								lastTrig = LAST_B;
								
							}
							sampleClockB = 0;
							warning = true;
						}
						
					break;

					case CV_MODE:

						bpmB = 120 * pow (2.0f, voltB);
						if (bpmB > 999)
							bpmB = 999;

						statusB = RUNNING;
				
						if (triggeredNow) {
							lastTrig = LAST_BOTH;
							status = A_EQUAL_B;
							delayCount = 0;
							
						} else {

							switch (status) {
								case IDLE:
									status = A_AFTER_B;
									delayCount = 0;
									
								break;

								case A_BEFORE_B:
									if (lastTrig == LAST_A) {
										recSampleCount = delayCount;
									} else {
										status = A_AFTER_B;
										delayCount = 0;
									}
								break;

								case A_AFTER_B:
									delayCount = 0;
								break;
							
								case A_EQUAL_B:
									status = A_AFTER_B;
									delayCount = 0;
								break;
							}
							lastTrig = LAST_B;
							
						}
					
						lastTrig = LAST_B;
						sampleClockB = 0;
						warning = true;
					break;
				}
			}

			if (status != IDLE) {
				delayCount++;
				if (statusA == RUNNING)
					sampleClockA++;

				if (statusB == RUNNING)
					sampleClockB++;

			}


			if (warning) {
				warning = false;

				switch (status) {
					
					case IDLE:
						//msg[1] = "IDLE";
					break;
					case A_BEFORE_B:
						delayA = 0;
						delayB = recSampleCount;
					break;
					case A_AFTER_B:
						delayA = recSampleCount;
						delayB = 0;
					break;

					case A_EQUAL_B:
						delayA = 0;
						delayB = 0;
					break;

				}			
			}

			prevInputA = inputA;
			prevInputB = inputB;

			/*
			
			if (trigValue >= 1 && prevTrigValue < 1) {

				if (pulseNr < 24) {


						pulseSample = sampleCountClock;
						pulseDistance[pulseNr] = pulseSample - prevPulseSample;

						if (firstAbsClock) {
							firstAbsClock = false;
						} else {
							if (pulseDistance[pulseNr] < minDist)
								minDist = pulseDistance[pulseNr];

							if (pulseDistance[pulseNr] > maxDist)
								maxDist = pulseDistance[pulseNr];
						}

						if (toWrite) {
							debugDisplay[pulseNr] = "Pulse: " + to_string(pulseNr) + " - " +
														//"prev: " + to_string(prevPulseSample) + " - " +
														"curr: " + to_string(pulseSample) + " - " +
														"dist: " + to_string(pulseDistance[pulseNr]);
						}
						prevPulseSample = pulseSample;

					
					pulseNr++;


				}

				if (pulseNr > 23) {
					toWrite = false;
					deltaDist = maxDist - minDist;

					debugDisplay[pulseNr] = "dist min: " + to_string(minDist) + " - " +
										"max: " + to_string(maxDist) + " - " +
										"delta: " + to_string(deltaDist);

					sampleCountClock = 0;
					pulseSample = 0;
					prevPulseSample = 0;
					pulseNr = 0;
				}
		

			} 

			prevTrigValue = trigValue;
			

			sampleCountClock++;
			*/
		}
	}
};


struct CVmeterDebugDisplay : TransparentWidget {
	CVmeter *module;
	int frame = 0;
	CVmeterDebugDisplay() {
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (module) {
			if (layer ==1) {
				//shared_ptr<Font> font = APP->window->loadFont(asset::system("res/fonts/Nunito-bold.ttf"));
				shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/vgatrue.ttf"));
				nvgFontSize(args.vg, 13);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				
				
				/*
				nvgTextBox(args.vg, 5, 6,400, module->debugDisplay[CLOCK_BPM].c_str(), NULL);
				nvgTextBox(args.vg, 5, 76,400, module->debugDisplay[TEN_SECONDS_BPM].c_str(), NULL);
				nvgTextBox(args.vg, 5, 116,400, module->debugDisplay[THIRTY_SECONDS_BPM].c_str(), NULL);
				nvgTextBox(args.vg, 5, 156,400, module->debugDisplay[ONE_MINUTE_BPM].c_str(), NULL);
				nvgTextBox(args.vg, 5, 196,400, module->debugDisplay[FIVE_MINUTES_BPM].c_str(), NULL);
				nvgTextBox(args.vg, 5, 236,400, module->debugDisplay[TEN_MINUTES_BPM].c_str(), NULL);
				nvgTextBox(args.vg, 5, 276,400, module->debugDisplay[TIME].c_str(), NULL);
				*/

				//int yDelta = 16;

				

				/*for (int i = 0; i < 26; i++) {
					nvgTextBox(args.vg, 5, 6+(yDelta*i),400, module->debugDisplay[i].c_str(), NULL);
				}*/

				/*
				for (int i = 0; i < 5; i++) {
					nvgTextBox(args.vg, 5, 30+(yDelta*i),400, module->msg[i].c_str(), NULL);
				}
				*/
				const float colA = 5;
				const float colB = 51;

				const float yVoltA = 88;
				const float yVoltB = 100;

				const float xNoteA = 23;
				const float xNoteB = 70;
				const float yNoteA = 103;
				const float yNoteB = 114;
				const float yMidi = 168;
				const float xMidiNoteA = 6;
				const float xMidiNoteB = 67;
				const float xMidiCcA = 30;
				const float xMidiCcB = 90;

				const float xBpmA = 5;
				//const float yBpmA = 214;
				const float yBpmA = 215;
				const float xBpmB = 60;
				//const float yBpmB = 215;
				const float yBpmB = 221;

				const float xDelayA = 10;
				const float xDelayB = 70;
				const float yDelay = 305;

				nvgFillColor(args.vg, nvgRGBA(0x00, 0xff, 0x00, 0xff)); 
				nvgFontSize(args.vg, 14);

				float shiftCol;
				//const float colWidth = 5.f;
				float colWidth = 6.f;

				float voltA = module->voltA;
				float voltB = module->voltB;
				int midiNoteA = module->midiNoteA;
				int midiNoteB = module->midiNoteB;
				int midiCcA = module->midiCcA;
				int midiCcB = module->midiCcB;
				double bpmA = module->bpmA;
				double bpmB = module->bpmB;
				int delayA = module->delayA;
				int delayB = module->delayB;


				if (voltA <= -10.f)
					shiftCol = 0.f;
				else if (voltA < 0.f)
					shiftCol = colWidth;
				else if (voltA < 10.F)
					shiftCol = 2 * colWidth;
				else
					shiftCol = colWidth;

				nvgTextBox(args.vg, colA + shiftCol, yVoltA,400, to_string(voltA).c_str(), NULL);

				if (voltB <= -10.f)
					shiftCol = 0.f;
				else if (voltB < 0.f)
					shiftCol = colWidth;
				else if (voltB < 10.F)
					shiftCol = 2 * colWidth;
				else
					shiftCol = colWidth;

				nvgTextBox(args.vg, colB + shiftCol, yVoltB,400, to_string(voltB).c_str(), NULL);

				nvgFontSize(args.vg, 16);
				nvgFillColor(args.vg, nvgRGBA(0xff, 0xff, 0x00, 0xff)); 

				nvgTextBox(args.vg, xNoteA, yNoteA,400, module->noteNames[module->midiNoteA].c_str(), NULL);
				nvgTextBox(args.vg, xNoteB, yNoteB,400, module->noteNames[module->midiNoteB].c_str(), NULL);

				nvgFontSize(args.vg, 13);
				nvgFillColor(args.vg, nvgRGBA(0x66, 0x66, 0x66, 0xff)); 

				colWidth = 5.f;

				if (midiNoteA < 10)
					shiftCol = 2 * colWidth;
				else if (midiNoteA < 100)
					shiftCol = colWidth;
				else
					shiftCol = 0;
				std::string tempText = to_string(midiNoteA);
				nvgTextBox(args.vg, xMidiNoteA + shiftCol, yMidi,400, tempText.c_str(), NULL);

				if (midiNoteB < 10)
					shiftCol = 2 * colWidth;
				else if (midiNoteB < 100)
					shiftCol = colWidth;
				else
					shiftCol = 0;
				tempText = to_string(midiNoteB);
				nvgTextBox(args.vg, xMidiNoteB + shiftCol, yMidi,400, tempText.c_str(), NULL);

				if (midiCcA < 10)
					shiftCol = 2 * colWidth;
				else if (midiCcA < 100)
					shiftCol = colWidth;
				else
					shiftCol = 0;

				nvgFillColor(args.vg, nvgRGBA(0x99, 0x99, 0x99, 0xff)); 
				tempText = to_string(midiCcA);
				nvgTextBox(args.vg, xMidiCcA + shiftCol, yMidi,400, tempText.c_str(), NULL);

				if (midiCcB < 10)
					shiftCol = 2 * colWidth;
				else if (midiCcB < 100)
					shiftCol = colWidth;
				else
					shiftCol = 0;
				tempText = to_string(midiCcB);
				nvgTextBox(args.vg, xMidiCcB + shiftCol, yMidi,400, tempText.c_str(), NULL);

				// -------------------------------------------------------------- BPM
				nvgFontSize(args.vg, 12);
				
				if (bpmA < 10)
					shiftCol = 2 * colWidth;
				else if (bpmA < 100)
					shiftCol = colWidth;
				else
					shiftCol = 0;
				nvgFillColor(args.vg, nvgRGBA(0xff, 0x00, 0x00, 0xff)); 
				if (bpmA < 1000)
					nvgTextBox(args.vg, xBpmA+shiftCol, yBpmA,400, to_string(bpmA).c_str(), NULL);
				else
					nvgTextBox(args.vg, xBpmA+shiftCol, yBpmA,400, std::string("> 999").c_str(), NULL);
				if (bpmB < 10)
					shiftCol = 2 * colWidth;
				else if (bpmB < 100)
					shiftCol = colWidth;
				else
					shiftCol = 0;
				nvgFillColor(args.vg, nvgRGBA(0xff, 0x33, 0x22, 0xff)); 
				if (bpmB < 1000)
					nvgTextBox(args.vg, xBpmB+shiftCol, yBpmB,400, to_string(bpmB).c_str(), NULL);
				else
					nvgTextBox(args.vg, xBpmB+shiftCol, yBpmB,400, std::string("> 999").c_str(), NULL);
				


				// --------------------------- DELAY
				
				colWidth = 6.f;

				if (module->statusA == RUNNING && module->statusB == RUNNING) {
					nvgFontSize(args.vg, 16);
					if (module->status == A_EQUAL_B) {
						nvgFontSize(args.vg, 14);
						nvgTextBox(args.vg, xDelayA + 10, yDelay,400, std::string("SYNC").c_str(), NULL);
						nvgTextBox(args.vg, xDelayB + 10, yDelay,400, std::string("SYNC").c_str(), NULL);
					} else {
						if (delayA > 0) {
							if (delayA < 10)
								shiftCol = 4 * colWidth;
							else if (delayA < 100)
								shiftCol = 3 * colWidth;
							else if (delayA < 1000)
								shiftCol = 2 * colWidth;
							else if (delayA < 10000)
								shiftCol = colWidth;
							else
								shiftCol = 0;
							if (delayA < 100000)
								nvgTextBox(args.vg, xDelayA + shiftCol, yDelay,400, to_string(delayA).c_str(), NULL);
							else
								nvgTextBox(args.vg, xDelayA + shiftCol, yDelay,400, ">100,000", NULL);
						}
						if (module->delayB > 0) {
							if (delayB < 10)
								shiftCol = 4 * colWidth;
							else if (delayB < 100)
								shiftCol = 3 * colWidth;
							else if (delayB < 1000)
								shiftCol =  2 * colWidth;
							else if (delayB < 10000)
								shiftCol = colWidth;
							else 
								shiftCol = 0;
							if (delayB < 100000)
								nvgTextBox(args.vg, xDelayB + shiftCol, yDelay,400, to_string(delayB).c_str(), NULL);
							else
								nvgTextBox(args.vg, xDelayB + shiftCol - 10, yDelay,400, ">100,000", NULL);
						}
					}
				}
			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct CVmeterWidget : ModuleWidget {
	CVmeterWidget(CVmeter *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CVmeter.svg")));

		/*
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	  
		*/

		{
			CVmeterDebugDisplay *display = new CVmeterDebugDisplay();
			display->box.pos = Vec(3, 50);
			display->box.size = Vec(300, 100);
			display->module = module;
			addChild(display);
		}

		const float yLine = 21;
		const float xA = 6.8;
		const float xB = 33.7;

		const float ySw = 30.7;
		const float xSwA = 8.9;
		const float xSwB = 31.6;

		const float xRst = 20.5; 
		const float yRstBut = 21;
		const float yRstIn = 31;

		const float yFrz = 100;
		const float xFrzIn = 8;
		const float xFrzBut = xFrzIn + 9;


		//addInput(createInputCentered<PJ301MPort>(mm2px(Vec(85, yLine)), module, CVmeter::TRIG_INPUT));
		//addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(95, yLine)), module, CVmeter::RESET_BUT_PARAM, CVmeter::RESET_BUT_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xA, yLine)), module, CVmeter::A_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xB, yLine)), module, CVmeter::B_INPUT));
		addParam(createParamCentered<CKSS>(mm2px(Vec(xSwA, ySw)), module, CVmeter::MODE_A_SWITCH));
		addParam(createParamCentered<CKSS>(mm2px(Vec(xSwB, ySw)), module, CVmeter::MODE_B_SWITCH));
		addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(xRst, yRstBut)), module, CVmeter::RESET_BUT_PARAM, CVmeter::RESET_BUT_LIGHT));
		addParam(createLightParamCentered<VCVLightBezel<YellowLight>>(mm2px(Vec(xFrzBut, yFrz)), module, CVmeter::FREEZE_PARAM, CVmeter::FREEZE_LIGHT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xRst, yRstIn)), module, CVmeter::RESET_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xFrzIn, yFrz)), module, CVmeter::FREEZE_INPUT));
	}

};

Model *modelCVmeter = createModel<CVmeter, CVmeterWidget>("CVmeter");
