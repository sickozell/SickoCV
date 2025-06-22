#include "TrigStation_def.hpp"

	const std::string modeNames[MAXMODES+1] = {">>", "<<", "PingPong","|PingPong|","RAND","RNDr", "CV"};

	const std::string modeNameDisplay[7] = {" >>", " <<", " PP","|PP|","RAND","RNDr", " CV"};
	const std::string outTypeNames[3] = {"Trig", "Gate", "Default"};
	const std::string revTypeNames[3] = {"Positive", "Negative", "Default"};
	
	const std::string dontAdvanceNames[3] = {"Advance", "Don't Advance", "Default"};
	const std::string rstStepsWhenNames[4] = {"No Reset", "On RUN", "On STOP", "Default"};
//	const std::string sampleDelayNames[7] = {"No Delay", "1 sample", "2 samples", "3 samples", "4 samples", "5 samples", "Default"};
	
//	const std::string userInNames[KNOB_SHIFT] = {"Reverse", "Mode", "Length", "Reset Step#", "Run", "Retrig", "Out scale", "Swing", "Flip"};
//	const std::string userKnobNames[KNOB_NR] = {"Pulse Width", "Reset Step#", "Mode", "Retrig Prob.", "Out scale", "Swing", "Attenuator", "Attenuverter"};

	int outTypeSetting[ALLTRACKS] = {2, 2, 2, 2, 2, 2, 2, 2, 0};
	int turingMode[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0};
	bool prevTuringMode[MAXTRACKS] = {false, false, false, false, false, false, false, false};;
	
	int revType[ALLTRACKS] = {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V};
	int dontAdvanceSetting[ALLTRACKS] = {2, 2, 2, 2, 2, 2, 2, 2, 1};
	int rstStepsWhen[ALLTRACKS] = {3, 3, 3, 3, 3, 3, 3, 3, 1};
	bool xcludeFromRun[MAXTRACKS] = {false, false, false, false, false, false, false, false};
	bool xcludeFromRst[MAXTRACKS] = {false, false, false, false, false, false, false, false};
//	int sampleDelay[ALLTRACKS] = {6, 6, 6, 6, 6, 6, 6, 6, 0};

	float nextSeq[MAXTRACKS][16] = {	{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
								{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
							};
	int nextSteps[MAXTRACKS] = {16, 16, 16, 16, 16, 16, 16, 16};
	
	int nextCurrentMode[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0};
	float nextDivMult[MAXTRACKS] = {22, 22, 22, 22, 22, 22, 22, 22};

	int nextTuringMode[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0};
	int nextOutTypeSetting[ALLTRACKS] = {2, 2, 2, 2, 2, 2, 2, 2, 0};
	
	int nextRevType[ALLTRACKS] = {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V};
	int nextDontAdvanceSetting[ALLTRACKS] = {2, 2, 2, 2, 2, 2, 2, 2, 1};
	int nextRstStepsWhen[ALLTRACKS] = {3, 3, 3, 3, 3, 3, 3, 3, 1};
	
	int nextSampleDelay[ALLTRACKS] = {6, 6, 6, 6, 6, 6, 6, 6, 0};
	bool nextXcludeFromRun[MAXTRACKS] = {false, false, false, false, false, false, false, false};
	bool nextXcludeFromRst[MAXTRACKS] = {false, false, false, false, false, false, false, false};
	bool nextSeqRunSetting = seqRunSetting;
	bool nextInternalClock = internalClock;
	float nextBpmKnob = 1200.f;

//	float nextRst[MAXTRACKS] = {0, 0, 0, 0, 0, 0, 0, 0};
	// --------------workingSeq

	float wSeq[MAXTRACKS][16] = {	{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
							{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
							{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
							{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
							{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
							{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
							{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
							{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
						};
	int wSteps[MAXTRACKS] = {16, 16, 16, 16, 16, 16, 16, 16};

	int progSteps[32][MAXTRACKS] = {{16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, 
									{16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, 
									{16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, 
									{16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, 
									{16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, 
									{16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, 
									{16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, 
									{16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}, {16,16,16,16,16,16,16,16}
								};

	int progCurrentMode[32][MAXTRACKS] = { {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
											{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
											{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
											{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
											{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
											{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
											{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
											{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}
										};

	float progDivMult[32][MAXTRACKS] = { {22, 22, 22, 22, 22, 22, 22, 22}, {22, 22, 22, 22, 22, 22, 22, 22}, 
										{22, 22, 22, 22, 22, 22, 22, 22}, {22, 22, 22, 22, 22, 22, 22, 22}, 
										{22, 22, 22, 22, 22, 22, 22, 22}, {22, 22, 22, 22, 22, 22, 22, 22}, 
										{22, 22, 22, 22, 22, 22, 22, 22}, {22, 22, 22, 22, 22, 22, 22, 22}, 
										{22, 22, 22, 22, 22, 22, 22, 22}, {22, 22, 22, 22, 22, 22, 22, 22}, 
										{22, 22, 22, 22, 22, 22, 22, 22}, {22, 22, 22, 22, 22, 22, 22, 22}, 
										{22, 22, 22, 22, 22, 22, 22, 22}, {22, 22, 22, 22, 22, 22, 22, 22}, 
										{22, 22, 22, 22, 22, 22, 22, 22}, {22, 22, 22, 22, 22, 22, 22, 22}, 
										{22, 22, 22, 22, 22, 22, 22, 22}, {22, 22, 22, 22, 22, 22, 22, 22}, 
										{22, 22, 22, 22, 22, 22, 22, 22}, {22, 22, 22, 22, 22, 22, 22, 22}, 
										{22, 22, 22, 22, 22, 22, 22, 22}, {22, 22, 22, 22, 22, 22, 22, 22}, 
										{22, 22, 22, 22, 22, 22, 22, 22}, {22, 22, 22, 22, 22, 22, 22, 22}, 
										{22, 22, 22, 22, 22, 22, 22, 22}, {22, 22, 22, 22, 22, 22, 22, 22}, 
										{22, 22, 22, 22, 22, 22, 22, 22}, {22, 22, 22, 22, 22, 22, 22, 22}, 
										{22, 22, 22, 22, 22, 22, 22, 22}, {22, 22, 22, 22, 22, 22, 22, 22}, 
										{22, 22, 22, 22, 22, 22, 22, 22}, {22, 22, 22, 22, 22, 22, 22, 22}
									};

	int progTuringMode[32][MAXTRACKS] = { {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
											{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
											{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
											{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
											{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
											{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
											{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
											{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}
										};

	int progOutTypeSetting[32][ALLTRACKS] = {{2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0},
										{2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0},
										{2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0},
										{2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0},
										{2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0},
										{2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0},
										{2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0},
										{2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0}, {2, 2, 2, 2, 2, 2, 2, 2, 0}};

	int progRevType[32][ALLTRACKS] = {{2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, 
									{2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, 
									{2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, 
									{2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, 
									{2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, 
									{2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, 
									{2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, 
									{2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}, {2, 2, 2, 2, 2, 2, 2, 2, POSITIVE_V}};

	int progDontAdvanceSetting[32][ALLTRACKS] = {{2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1},
										{2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1},
										{2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1},
										{2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1},
										{2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1},
										{2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1},
										{2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1},
										{2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1}, {2, 2, 2, 2, 2, 2, 2, 2, 1}};

	int progRstStepsWhen[32][ALLTRACKS] = {{3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1},
										{3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1},
										{3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1},
										{3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1},
										{3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1},
										{3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1},
										{3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1},
										{3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1}, {3, 3, 3, 3, 3, 3, 3, 3, 1}};
	
	bool progXcludeFromRun[32][MAXTRACKS] = {{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false},
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}};

	bool progXcludeFromRst[32][MAXTRACKS] = {{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false},
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}, 
											{false, false, false, false, false, false, false, false}, {false, false, false, false, false, false, false, false}};
	
	int progSampleDelay[32][ALLTRACKS] = { {6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, 
											{6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, 
											{6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, 
											{6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, 
											{6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, 
											{6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, 
											{6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, 
											{6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}, {6, 6, 6, 6, 6, 6, 6, 6, 0}
										};

	int progSeqRunSetting[32] = {seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, 
							seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, 
							seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, 
							seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting, seqRunSetting};

	bool progInternalClock[32] = {internalClock, internalClock, internalClock, internalClock, internalClock, internalClock, internalClock, internalClock, 
									internalClock, internalClock, internalClock, internalClock, internalClock, internalClock, internalClock, internalClock,
									internalClock, internalClock, internalClock, internalClock, internalClock, internalClock, internalClock, internalClock,
									internalClock, internalClock, internalClock, internalClock, internalClock, internalClock, internalClock, internalClock};
	
	float progBpmKnob[32] = {1200.f, 1200.f, 1200.f, 1200.f, 1200.f, 1200.f, 1200.f, 1200.f, 
							1200.f, 1200.f, 1200.f, 1200.f, 1200.f, 1200.f, 1200.f, 1200.f, 
							1200.f, 1200.f, 1200.f, 1200.f, 1200.f, 1200.f, 1200.f, 1200.f, 
							1200.f, 1200.f, 1200.f, 1200.f, 1200.f, 1200.f, 1200.f, 1200.f};

	float progSeq[32][MAXTRACKS][16] = {	{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									},
									{
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
										{0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
									}
								};

	float userValues[MAXTRACKS][2] = {{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}};
	
	float nextUserValues[MAXTRACKS][2] = {{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}};

	float progUserValues[32][MAXTRACKS][2] = {{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},
												{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}}
											};

	// *****************************************************************************

	const int defaultUserTable[MAXTRACKS][4] = {
							{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}
						};

	int nextUserTable[MAXTRACKS][4] = {
							{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
							{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}
						};
	
	int progUserTable[32][MAXTRACKS][4] = { {{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},

											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},

											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},

											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}},
											{{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE},
												{IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}, {IN_RUN, KNOB_FLIPPROB, IN_REV, KNOB_MODE}}

										};

	const int defaultUserInputs[MAXTRACKS][MAXUSER][2] = {
//							  IN_FLIP IN_FLPPRB IN_LENGH IN_MODE  IN_OUTSC IN_PW   IN_RSTST IN_RETRIG IN_REV   IN_RUN   IN_SKIP IN_SKIPRB IN_SWING KN_FLIP  KN_MODE  KN_OUTSC KN_PW   KN_RSTST  KN_PROB KN_SKIP KN_SWNG KN_ATN  KN_ATNV
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						};


	int nextUserInputs[MAXTRACKS][MAXUSER][2] = {
//							  IN_FLIP IN_FLPPRB IN_LENGH IN_MODE  IN_OUTSC IN_PW   IN_RSTST IN_RETRIG IN_REV   IN_RUN   IN_SKIP IN_SKIPRB IN_SWING KN_FLIP  KN_MODE  KN_OUTSC KN_PW   KN_RSTST  KN_PROB KN_SKIP KN_SWNG KN_ATN  KN_ATNV
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						};

	int progUserInputs[32][MAXTRACKS][MAXUSER][2] = {
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						},
						{	{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
							{ {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 8} , {1, 0} , {0, 0} , {0, 0} , {0, 0} , {1, 0} , {1, 8} , {0, 0} , {0, 0} , {0, 0} , {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} }
						}
					};
	