#include "TrigStation_def.hpp"

/*

														███╗░░░███╗░█████╗░██████╗░██╗░░░██╗██╗░░░░░███████╗░░░░░░░░░░██████╗░█████╗░██╗░░░██╗███████╗
														████╗░████║██╔══██╗██╔══██╗██║░░░██║██║░░░░░██╔════╝░░░░░░░░░██╔════╝██╔══██╗██║░░░██║██╔════╝
														██╔████╔██║██║░░██║██║░░██║██║░░░██║██║░░░░░█████╗░░░░░░░░░░░╚█████╗░███████║╚██╗░██╔╝█████╗░░
														██║╚██╔╝██║██║░░██║██║░░██║██║░░░██║██║░░░░░██╔══╝░░░░░░░░░░░░╚═══██╗██╔══██║░╚████╔╝░██╔══╝░░
														██║░╚═╝░██║╚█████╔╝██████╔╝╚██████╔╝███████╗███████╗██╗██╗██╗██████╔╝██║░░██║░░╚██╔╝░░███████╗
														╚═╝░░░░░╚═╝░╚════╝░╚═════╝░░╚═════╝░╚══════╝╚══════╝╚═╝╚═╝╚═╝╚═════╝░╚═╝░░╚═╝░░░╚═╝░░░╚══════╝
// https://fsymbols.com/generators/carty/
*/
	json_t* dataToJson() override {

		for (int t = 0; t < MAXTRACKS; t++)
			recStep[t] = step[t];

		json_t* rootJ = json_object();

//		json_object_set_new(rootJ, "divControls", json_boolean(divControls));
		json_object_set_new(rootJ, "modeControls", json_boolean(modeControls));
		json_object_set_new(rootJ, "wait2RstSetting", json_integer(wait2RstSetting));

		// ---------- CLOCK
		
//		json_object_set_new(rootJ, "ppqn", json_integer(ppqn));
		json_object_set_new(rootJ, "resetOnRun", json_boolean(resetOnRun));
		json_object_set_new(rootJ, "ResetPulseOnRun", json_boolean(resetPulseOnRun));
		json_object_set_new(rootJ, "ResetOnStop", json_boolean(resetOnStop));
		json_object_set_new(rootJ, "ResetPulseOnStop", json_boolean(resetPulseOnStop));

//		json_object_set_new(rootJ, "cvClockIn", json_boolean(cvClockIn));
//		json_object_set_new(rootJ, "cvClockOut", json_boolean(cvClockOut));

		// --------------------- track modes

		
		// --------- SEQUENCER
		//json_object_set_new(rootJ, "range", json_integer(range));
		json_object_set_new(rootJ, "runType", json_integer(runType));
//		json_object_set_new(rootJ, "rstClkOnRst", json_boolean(rstClkOnRst));
		json_object_set_new(rootJ, "rstSeqOnProgChange", json_boolean(rstSeqOnProgChange));

		json_object_set_new(rootJ, "bitResolution", json_integer(bitResolution));
		json_object_set_new(rootJ, "progression", json_integer(progression));

		json_object_set_new(rootJ, "savedProgKnob", json_integer(savedProgKnob));

		json_object_set_new(rootJ, "progInType", json_boolean(progInType));
		json_object_set_new(rootJ, "lastProg", json_integer(lastProg));

		// ---------------------------------------------------------------------

//		json_object_set_new(rootJ, "internalClock", json_boolean(internalClock));
		json_object_set_new(rootJ, "seqRunSetting", json_integer(seqRunSetting));

		{
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(currentMode[t]));
			json_object_set_new(rootJ, "currentMode", this_json_array);
		}
		{
			json_t *this_json_array = json_array();
			for (int t = 0; t < ALLTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(outTypeSetting[t]));
			json_object_set_new(rootJ, "outType", this_json_array);
		}
		{
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(turingMode[t]));
			json_object_set_new(rootJ, "currTuringMode", this_json_array);
		}
		/*{
			json_t *this_json_array = json_array();
			for (int t = 0; t < ALLTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(revType[t]));
			json_object_set_new(rootJ, "currRevType", this_json_array);
		}*/
		{
			json_t *this_json_array = json_array();
			for (int t = 0; t < ALLTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(dontAdvanceSetting[t]));
			json_object_set_new(rootJ, "currDontAdvanceSetting", this_json_array);
		}
		{
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(xcludeFromRst[t]));
			json_object_set_new(rootJ, "currXcludeFromRst", this_json_array);
		}
		{
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(xcludeFromRun[t]));
			json_object_set_new(rootJ, "currXcludeFromRun", this_json_array);
		}
		{
			json_t *this_json_array = json_array();
			for (int t = 0; t < ALLTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(rstStepsWhen[t]));
			json_object_set_new(rootJ, "currRstStepsWhen", this_json_array);
		}

		{
			json_t *this_json_array = json_array();
			for (int t = 0; t < ALLTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(sampleDelay[t]));
			json_object_set_new(rootJ, "currSampleDelay", this_json_array);
		}

		// -------------- user input knob settings
		for (int t = 0; t < MAXTRACKS; t++) {
			json_t *this_json_array = json_array();
			for (int u = 0; u < 4; u++) {
				json_array_append_new(this_json_array, json_integer(userTable[t][u]));
			}
			json_object_set_new(rootJ, ("userTable_t"+to_string(t)).c_str(), this_json_array);
		}

		for (int t = 0; t < MAXTRACKS; t++) {
			for (int u = 0; u < MAXUSER; u++) {
				json_t *this_json_array = json_array();
				for (int i = 0; i < 2; i++) {
					json_array_append_new(this_json_array, json_integer(userInputs[t][u][i]));
				}
				json_object_set_new(rootJ, ("userInputs_t"+to_string(t)+"u"+to_string(u)).c_str(), this_json_array);
			}
		}
		
		// -------------------------- sequences
		
		for (int t = 0; t < MAXTRACKS; t++) {
			json_t *this_json_array = json_array();
			for (int s = 0; s < 16; s++) {
				json_array_append_new(this_json_array, json_integer(wSeq[t][s]));
			}
			json_object_set_new(rootJ, ("wSeq_t"+to_string(t)).c_str(), this_json_array);
		}

		{
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(recStep[t]));
			json_object_set_new(rootJ, "currentStep", this_json_array);
		}
		// -------------------------- PROGRAMS


		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				json_t *this_json_array = json_array();
				for (int s = 0; s < 16; s++) {
					json_array_append_new(this_json_array, json_integer(progSeq[p][t][s]));
				}
				json_object_set_new(rootJ, ("p"+to_string(p)+"t"+to_string(t)).c_str(), this_json_array);
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++) {
				json_array_append_new(this_json_array, json_integer(progSteps[p][t]));
			}
			json_object_set_new(rootJ, ("progSteps"+to_string(p)).c_str(), this_json_array);
		}
/*
		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++) {
				json_array_append_new(this_json_array, json_real(progDivMult[p][t]));
			}
			json_object_set_new(rootJ, ("progDivMult_p"+to_string(p)).c_str(), this_json_array);
		}
*/
		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++) {
				json_array_append_new(this_json_array, json_integer(progCurrentMode[p][t]));
			}
			json_object_set_new(rootJ, ("progCurrentMode_p"+to_string(p)).c_str(), this_json_array);
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < ALLTRACKS; t++) {
				json_array_append_new(this_json_array, json_integer(progOutTypeSetting[p][t]));
			}
			json_object_set_new(rootJ, ("progOutType_p"+to_string(p)).c_str(), this_json_array);
		}
/*
		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < ALLTRACKS; t++) {
				json_array_append_new(this_json_array, json_integer(progRevType[p][t]));
			}
			json_object_set_new(rootJ, ("progRevType_p"+to_string(p)).c_str(), this_json_array);
		}
*/
		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(progTuringMode[p][t]));
			json_object_set_new(rootJ, ("progTuringMode_p"+to_string(p)).c_str(), this_json_array);
		}
		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < ALLTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(progDontAdvanceSetting[p][t]));
			json_object_set_new(rootJ, ("progDontAdvanceSetting_p"+to_string(p)).c_str(), this_json_array);
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < ALLTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(progRstStepsWhen[p][t]));
			json_object_set_new(rootJ, ("progRstStepsWhen_p"+to_string(p)).c_str(), this_json_array);
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(progXcludeFromRst[p][t]));
			json_object_set_new(rootJ, ("progXcludeFromRst_p"+to_string(p)).c_str(), this_json_array);
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(progXcludeFromRun[p][t]));
			json_object_set_new(rootJ, ("progXcludeFromRun_p"+to_string(p)).c_str(), this_json_array);
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < ALLTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(progSampleDelay[p][t]));
			json_object_set_new(rootJ, ("progSampleDelay_p"+to_string(p)).c_str(), this_json_array);
		}

		{
			json_t *this_json_array = json_array();
			for (int p = 0; p < 32; p++)
				json_array_append_new(this_json_array, json_integer(progSeqRunSetting[p]));
			json_object_set_new(rootJ, "progSeqRunSetting", this_json_array);
		}
/*
		{
			json_t *this_json_array = json_array();
			for (int p = 0; p < 32; p++)
				json_array_append_new(this_json_array, json_boolean(progInternalClock[p]));
			json_object_set_new(rootJ, "progInternalClock", this_json_array);
		}

		{
			json_t *this_json_array = json_array();
			for (int p = 0; p < 32; p++)
				json_array_append_new(this_json_array, json_real(progBpmKnob[p]));
			json_object_set_new(rootJ, "progBpmKnob", this_json_array);
		}
*/
		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				json_t *this_json_array = json_array();
				for (int i = 0; i < 4; i++)
					json_array_append_new(this_json_array, json_integer(progUserTable[p][t][i]));
				json_object_set_new(rootJ, ("progUserTable_p"+to_string(p)+"t"+to_string(t)).c_str(), this_json_array);
			}
		}

		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				for (int u = 0; u < MAXUSER; u++) {
					json_t *this_json_array = json_array();
					for (int i = 0; i < 2; i++) {
						json_array_append_new(this_json_array, json_integer(progUserInputs[p][t][u][i]));
					}
					json_object_set_new(rootJ, ("progUserInputs_p"+to_string(p)+"t"+to_string(t)+"u"+to_string(u)).c_str(), this_json_array);
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				json_t *this_json_array = json_array();
				for (int i = 0; i < 2; i++) {
					json_array_append_new(this_json_array, json_real(progUserValues[p][t][i]));
				}
				json_object_set_new(rootJ, ("progUserValues_p"+to_string(p)+"t"+to_string(t)).c_str(), this_json_array);
			}
		}

		return rootJ;
	}


/*

														███╗░░░███╗░█████╗░██████╗░██╗░░░██╗██╗░░░░░███████╗░░░░░░░░░██╗░░░░░░█████╗░░█████╗░██████╗░
														████╗░████║██╔══██╗██╔══██╗██║░░░██║██║░░░░░██╔════╝░░░░░░░░░██║░░░░░██╔══██╗██╔══██╗██╔══██╗
														██╔████╔██║██║░░██║██║░░██║██║░░░██║██║░░░░░█████╗░░░░░░░░░░░██║░░░░░██║░░██║███████║██║░░██║
														██║╚██╔╝██║██║░░██║██║░░██║██║░░░██║██║░░░░░██╔══╝░░░░░░░░░░░██║░░░░░██║░░██║██╔══██║██║░░██║
														██║░╚═╝░██║╚█████╔╝██████╔╝╚██████╔╝███████╗███████╗██╗██╗██╗███████╗╚█████╔╝██║░░██║██████╔╝
														╚═╝░░░░░╚═╝░╚════╝░╚═════╝░░╚═════╝░╚══════╝╚══════╝╚═╝╚═╝╚═╝╚══════╝░╚════╝░╚═╝░░╚═╝╚═════╝░
*/
	void dataFromJson(json_t* rootJ) override {

		json_t* wait2RstSettingJ = json_object_get(rootJ, "wait2RstSetting");
		if (wait2RstSettingJ)
			wait2RstSetting = json_boolean_value(wait2RstSettingJ);

/*
		json_t* divControlsJ = json_object_get(rootJ, "divControls");
		if (divControlsJ)
			divControls = json_boolean_value(divControlsJ);
*/
		json_t* modeControlsJ = json_object_get(rootJ, "modeControls");
		if (modeControlsJ)
			modeControls = json_boolean_value(modeControlsJ);

		// ------------ CLOCK
/*
		json_t* internalClockJ = json_object_get(rootJ, "internalClock");
		if (internalClockJ)
			internalClock = json_boolean_value(internalClockJ);
*/
		json_t* seqRunSettingJ = json_object_get(rootJ, "seqRunSetting");
		if (seqRunSettingJ)
			seqRunSetting = json_integer_value(seqRunSettingJ);
/*		
		json_t* ppqnJ = json_object_get(rootJ, "ppqn");
		if (ppqnJ) {
			tempPpqn = json_integer_value(ppqnJ);
			if (tempPpqn < 0 || tempPpqn > 6)
				tempPpqn = 0;
			if (tempPpqn != ppqn)
				changePpqnSetting();
		}
*/
		json_t* resetOnRunJ = json_object_get(rootJ, "resetOnRun");
		if (resetOnRunJ)
			resetOnRun = json_boolean_value(resetOnRunJ);
		json_t* resetPulseOnRunJ = json_object_get(rootJ, "ResetPulseOnRun");
		if (resetPulseOnRunJ)
			resetPulseOnRun = json_boolean_value(resetPulseOnRunJ);

		json_t* resetOnStopJ = json_object_get(rootJ, "ResetOnStop");
		if (resetOnStopJ)
			resetOnStop = json_boolean_value(resetOnStopJ);
		json_t* resetPulseOnStopJ = json_object_get(rootJ, "ResetPulseOnStop");
		if (resetPulseOnStopJ)
			resetPulseOnStop = json_boolean_value(resetPulseOnStopJ);
/*
		json_t* cvClockInJ = json_object_get(rootJ, "cvClockIn");
		if (cvClockInJ)
			cvClockIn = json_boolean_value(cvClockInJ);
		json_t* cvClockOutJ = json_object_get(rootJ, "cvClockOut");
		if (cvClockOutJ)
			cvClockOut = json_boolean_value(cvClockOutJ);
*/
		// -------------- user input knob settings

		for (int t = 0; t < MAXTRACKS; t++) {
			json_t *json_array = json_object_get(rootJ, ("userTable_t"+to_string(t)).c_str());
			size_t jThis;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, jThis, json_value) {
					userTable[t][jThis] = json_integer_value(json_value);
				}
			}
		}

		for (int t = 0; t < MAXTRACKS; t++) {
			for (int u = 0; u < MAXUSER; u++) {
				json_t *json_array = json_object_get(rootJ, ("userInputs_t"+to_string(t)+"u"+to_string(u)).c_str());
				size_t jThis;
				json_t *json_value;
				if (json_array) {
					json_array_foreach(json_array, jThis, json_value) {
						userInputs[t][u][jThis] = json_integer_value(json_value);
					}
				}

			}
		}

		// --------------- track modes

		{
			json_t *json_array = json_object_get(rootJ, "currentMode");
			size_t jThis;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, jThis, json_value) {
					currentMode[jThis] = json_integer_value(json_value);
				}
			}
		}

		{
			json_t *json_array = json_object_get(rootJ, "outType");
			size_t jThis;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, jThis, json_value) {
					outTypeSetting[jThis] = json_integer_value(json_value);
				}
			}
		}

		{
			json_t *json_array = json_object_get(rootJ, "currTuringMode");
			size_t jThis;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, jThis, json_value) {
					turingMode[jThis] = json_integer_value(json_value);
				}
			}
		}

		// ---------------- tweaks
/*
		{
			json_t *json_array = json_object_get(rootJ, "currRevType");
			size_t jThis;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, jThis, json_value) {
					revType[jThis] = json_integer_value(json_value);
				}
			}
		}
*/
		{
			json_t *json_array = json_object_get(rootJ, "currDontAdvanceSetting");
			size_t jThis;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, jThis, json_value) {
					dontAdvanceSetting[jThis] = json_integer_value(json_value);
				}
			}
		}

		{
			json_t *json_array = json_object_get(rootJ, "currXcludeFromRst");
			size_t jThis;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, jThis, json_value) {
					xcludeFromRst[jThis] = json_integer_value(json_value);
				}
			}
		}

		{
			json_t *json_array = json_object_get(rootJ, "currXcludeFromRun");
			size_t jThis;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, jThis, json_value) {
					xcludeFromRun[jThis] = json_integer_value(json_value);
				}
			}
		}

		{
			json_t *json_array = json_object_get(rootJ, "currRstStepsWhen");
			size_t jThis;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, jThis, json_value) {
					rstStepsWhen[jThis] = json_integer_value(json_value);
				}
			}
		}

		{
			json_t *json_array = json_object_get(rootJ, "currSampleDelay");
			size_t jThis;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, jThis, json_value) {
					sampleDelay[jThis] = json_integer_value(json_value);
				}
			}
		}
		// ---------------------------------------------------------------------- SEQUENC

		{
			json_t* valueJ = json_object_get(rootJ, "runType");
			if (valueJ) {
				runType = json_integer_value(valueJ);
				if (runType < 0 || runType > 1)
					runType = 0;
			}
		}
/*		
		{
			json_t* valueJ = json_object_get(rootJ, "rstClkOnRst");
			if (valueJ)
				rstClkOnRst = json_boolean_value(valueJ);
		}
*/
		{
			json_t* valueJ = json_object_get(rootJ, "rstSeqOnProgChange");
			if (valueJ)
				rstSeqOnProgChange = json_boolean_value(valueJ);
		}

		// ----------------

		json_t* bitResolutionJ = json_object_get(rootJ, "bitResolution");
		if (bitResolutionJ) {
			bitResolution = json_integer_value(bitResolutionJ);
			if (bitResolution < 0 && bitResolution > 1)
				bitResolution = BIT_8;
		}

		json_t* progressionJ = json_object_get(rootJ, "progression");
		if (progressionJ) {
			progression = json_integer_value(progressionJ);
			if (progression < 0 && progression > 2)
				progression = STD2x_PROGRESSION;
		}

		// ----------------

		json_t* savedProgKnobJ = json_object_get(rootJ, "savedProgKnob");
		if (savedProgKnobJ) {
			savedProgKnob = json_integer_value(savedProgKnobJ);
			if (savedProgKnob < 0 || savedProgKnob > 31)
				savedProgKnob = 0;
			
		}

		json_t* progInTypeJ = json_object_get(rootJ, "progInType");
		if (progInTypeJ) {
			progInType = json_boolean_value(progInTypeJ);
		}

		json_t* lastProgJ = json_object_get(rootJ, "lastProg");
		if (lastProgJ) {
			lastProg = json_integer_value(lastProgJ);
			if (lastProg < 0 || lastProg > 31)
				lastProg = 0;
		}

		selectedProg = savedProgKnob;
		workingProg = selectedProg;
		prevProgKnob = selectedProg;
		params[PROG_PARAM].setValue(selectedProg);


		for (int t = 0; t < MAXTRACKS; t++) {
			json_t *json_array = json_object_get(rootJ, "currentStep");
			size_t tempJ;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, tempJ, json_value) {
					step[t] = json_integer_value(json_value);
					if (step[t] < 0 || step[t] > 15)
						step[t] = 0;
				}
			}
		}

		// ********

		for (int t = 0; t < MAXTRACKS; t++) {
			json_t *json_array = json_object_get(rootJ, ("wSeq_t"+to_string(t)).c_str());
			size_t jThis;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, jThis, json_value) {
					params[STEP_PARAM+(t*16)+jThis].setValue(json_real_value(json_value));
				}
			}
		}
		
		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				json_t *json_array = json_object_get(rootJ, ("p"+to_string(p)+"t"+to_string(t)).c_str());
				size_t jThis;
				json_t *json_value;
				if (json_array) {
					json_array_foreach(json_array, jThis, json_value) {
						progSeq[p][t][jThis] = json_integer_value(json_value);
					}
				}
			}
		}

		// ************************************ PROGRAMS

		for (int p = 0; p < 32; p++) {
			json_t *json_array = json_object_get(rootJ, ("progSteps_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, jThis, json_value) {
					progSteps[p][jThis] = json_integer_value(json_value);
				}
			}
		}
/*
		for (int p = 0; p < 32; p++) {
			json_t *json_array = json_object_get(rootJ, ("progDivMult_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, jThis, json_value) {
					progDivMult[p][jThis] = json_real_value(json_value);
				}
			}
		}
*/
		for (int p = 0; p < 32; p++) {
			json_t *json_array = json_object_get(rootJ, ("progCurrentMode_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, jThis, json_value) {
					progCurrentMode[p][jThis] = json_integer_value(json_value);
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_object_get(rootJ, ("progOutType_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progOutTypeSetting[p][jThis] = json_integer_value(this_json_value);
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_object_get(rootJ, ("progTuringMode_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progTuringMode[p][jThis] = json_integer_value(this_json_value);
				}
			}
		}
/*
		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_object_get(rootJ, ("progRevType_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progRevType[p][jThis] = json_integer_value(this_json_value);
				}
			}
		}
*/
		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_object_get(rootJ, ("progDontAdvanceSetting_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progDontAdvanceSetting[p][jThis] = json_integer_value(this_json_value);
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_object_get(rootJ, ("progRstStepsWhen_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progRstStepsWhen[p][jThis] = json_integer_value(this_json_value);
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_object_get(rootJ, ("progXcludeFromRun_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progXcludeFromRun[p][jThis] = json_integer_value(this_json_value);
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_object_get(rootJ, ("progXcludeFromRst_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progXcludeFromRst[p][jThis] = json_integer_value(this_json_value);
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_object_get(rootJ, ("progSampleDelay_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progSampleDelay[p][jThis] = json_integer_value(this_json_value);
				}
			}
		}

		{
			json_t *this_json_array = json_object_get(rootJ, "progSeqRunSetting");
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progSeqRunSetting[jThis] = json_integer_value(this_json_value);
				}
			}
		}
/*
		{
			json_t *this_json_array = json_object_get(rootJ, "progInternalClock");
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progInternalClock[jThis] = json_boolean_value(this_json_value);
				}
			}
		}

		{
			json_t *this_json_array = json_object_get(rootJ, "progBpmKnob");
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progBpmKnob[jThis] = json_real_value(this_json_value);
				}
			}
		}
*/
		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				json_t *this_json_array = json_object_get(rootJ, ("progUserTable_p"+to_string(p)+"t"+to_string(t)).c_str());
				size_t jThis;
				json_t *this_json_value;
				if (this_json_array) {
					json_array_foreach(this_json_array, jThis, this_json_value) {
						progUserTable[p][t][jThis] = json_integer_value(this_json_value);
					}
				}	
			}
		}

		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				for (int u = 0; u < MAXUSER; u++) {
					json_t *this_json_array = json_object_get(rootJ, ("progUserInputs_p"+to_string(p)+"t"+to_string(t)+"u"+to_string(u)).c_str());
					size_t jThis;
					json_t *this_json_value;
					if (this_json_array) {
						json_array_foreach(this_json_array, jThis, this_json_value) {
							progUserInputs[p][t][u][jThis] = json_integer_value(this_json_value);
						}
					}
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				json_t *this_json_array = json_object_get(rootJ, ("progUserValues_p"+to_string(p)+"t"+to_string(t)).c_str());
				size_t jThis;
				json_t *this_json_value;
				if (this_json_array) {
					json_array_foreach(this_json_array, jThis, this_json_value) {
						progUserValues[p][t][jThis] = json_real_value(this_json_value);
					}
				}
			}
		}
	}

/*
		
														░██████╗████████╗███████╗██████╗░░██████╗████████╗░░░░░░░░░██╗░░░░░░█████╗░░█████╗░██████╗░
														██╔════╝╚══██╔══╝██╔════╝██╔══██╗██╔════╝╚══██╔══╝░░░░░░░░░██║░░░░░██╔══██╗██╔══██╗██╔══██╗
														╚█████╗░░░░██║░░░█████╗░░██████╔╝╚█████╗░░░░██║░░░░░░░░░░░░██║░░░░░██║░░██║███████║██║░░██║
														░╚═══██╗░░░██║░░░██╔══╝░░██╔═══╝░░╚═══██╗░░░██║░░░░░░░░░░░░██║░░░░░██║░░██║██╔══██║██║░░██║
														██████╔╝░░░██║░░░███████╗██║░░░░░██████╔╝░░░██║░░░██╗██╗██╗███████╗╚█████╔╝██║░░██║██████╔╝
														╚═════╝░░░░╚═╝░░░╚══════╝╚═╝░░░░░╚═════╝░░░░╚═╝░░░╚═╝╚═╝╚═╝╚══════╝░╚════╝░╚═╝░░╚═╝╚═════╝░
*/

	void presetTrigStationFromJson(json_t *rootJ) {

		// ------------ CLOCK
/*		
		json_t* ppqnJ = json_object_get(rootJ, "ppqn");
		if (ppqnJ) {
			tempPpqn = json_integer_value(ppqnJ);
			if (tempPpqn < 0 || tempPpqn > 6)
				tempPpqn = 0;
			if (tempPpqn != ppqn)
				changePpqnSetting();
		}
*/
		json_t* wait2RstSettingJ = json_object_get(rootJ, "wait2RstSetting");
		if (wait2RstSettingJ)
			wait2RstSetting = json_boolean_value(wait2RstSettingJ);

		json_t* resetOnRunJ = json_object_get(rootJ, "resetOnRun");
		if (resetOnRunJ)
			resetOnRun = json_boolean_value(resetOnRunJ);
		json_t* resetPulseOnRunJ = json_object_get(rootJ, "ResetPulseOnRun");
		if (resetPulseOnRunJ)
			resetPulseOnRun = json_boolean_value(resetPulseOnRunJ);

		json_t* resetOnStopJ = json_object_get(rootJ, "ResetOnStop");
		if (resetOnStopJ)
			resetOnStop = json_boolean_value(resetOnStopJ);
		json_t* resetPulseOnStopJ = json_object_get(rootJ, "ResetPulseOnStop");
		if (resetPulseOnStopJ)
			resetPulseOnStop = json_boolean_value(resetPulseOnStopJ);
/*
		json_t* cvClockInJ = json_object_get(rootJ, "cvClockIn");
		if (cvClockInJ)
			cvClockIn = json_boolean_value(cvClockInJ);
		json_t* cvClockOutJ = json_object_get(rootJ, "cvClockOut");
		if (cvClockOutJ)
			cvClockOut = json_boolean_value(cvClockOutJ);
*/
		// ---------------------------------------------------------------------- SEQUENC

		{
			json_t* valueJ = json_object_get(rootJ, "runType");
			if (valueJ) {
				runType = json_integer_value(valueJ);
				if (runType < 0 || runType > 1)
					runType = 0;
			}
		}
/*
		{
			json_t* valueJ = json_object_get(rootJ, "rstClkOnRst");
			if (valueJ)
				rstClkOnRst = json_boolean_value(valueJ);
		}
*/
		{
			json_t* valueJ = json_object_get(rootJ, "rstSeqOnProgChange");
			if (valueJ)
				rstSeqOnProgChange = json_boolean_value(valueJ);
		}

		json_t* bitResolutionJ = json_object_get(rootJ, "bitResolution");
		if (bitResolutionJ) {
			bitResolution = json_integer_value(bitResolutionJ);
			if (bitResolution < 0 && bitResolution > 1)
				bitResolution = BIT_8;
		}

		json_t* progressionJ = json_object_get(rootJ, "progression");
		if (progressionJ) {
			progression = json_integer_value(progressionJ);
			if (progression < 0 && progression > 2)
				progression = STD2x_PROGRESSION;
		}

		json_t* progInTypeJ = json_object_get(rootJ, "progInType");
		if (progInTypeJ) {
			progInType = json_boolean_value(progInTypeJ);
		}

		json_t* lastProgJ = json_object_get(rootJ, "lastProg");
		if (lastProgJ) {
			lastProg = json_integer_value(lastProgJ);
			if (lastProg < 0 || lastProg > 31)
				lastProg = 0;
		}
		
		// ************************************ PROGRAMS

		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				json_t *json_array = json_object_get(rootJ, ("p"+to_string(p)+"t"+to_string(t)).c_str());
				size_t jThis;
				json_t *json_value;
				if (json_array) {
					json_array_foreach(json_array, jThis, json_value) {
						progSeq[p][t][jThis] = json_integer_value(json_value);
					}
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *json_array = json_object_get(rootJ, ("progSteps_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, jThis, json_value) {
					progSteps[p][jThis] = json_integer_value(json_value);
				}
			}
		}
/*
		for (int p = 0; p < 32; p++) {
			json_t *json_array = json_object_get(rootJ, ("progDivMult_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, jThis, json_value) {
					progDivMult[p][jThis] = json_real_value(json_value);
				}
			}
		}
*/
		for (int p = 0; p < 32; p++) {
			json_t *json_array = json_object_get(rootJ, ("progCurrentMode_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *json_value;
			if (json_array) {
				json_array_foreach(json_array, jThis, json_value) {
					progCurrentMode[p][jThis] = json_integer_value(json_value);
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_object_get(rootJ, ("progTuringMode_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progTuringMode[p][jThis] = json_integer_value(this_json_value);
				}
			}
		}
/*
		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_object_get(rootJ, ("progRevType_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progRevType[p][jThis] = json_integer_value(this_json_value);
				}
			}
		}
*/
		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_object_get(rootJ, ("progDontAdvanceSetting_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progDontAdvanceSetting[p][jThis] = json_integer_value(this_json_value);
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_object_get(rootJ, ("progRstStepsWhen_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progRstStepsWhen[p][jThis] = json_integer_value(this_json_value);
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_object_get(rootJ, ("progXcludeFromRun_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progXcludeFromRun[p][jThis] = json_integer_value(this_json_value);
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_object_get(rootJ, ("progXcludeFromRst_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progXcludeFromRst[p][jThis] = json_integer_value(this_json_value);
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_object_get(rootJ, ("progSampleDelay_p"+to_string(p)).c_str());
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progSampleDelay[p][jThis] = json_integer_value(this_json_value);
				}
			}
		}

		{
			json_t *this_json_array = json_object_get(rootJ, "progSeqRunSetting");
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progSeqRunSetting[jThis] = json_integer_value(this_json_value);
				}
			}
		}
/*
		{
			json_t *this_json_array = json_object_get(rootJ, "progInternalClock");
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progInternalClock[jThis] = json_boolean_value(this_json_value);
				}
			}
		}

		{
			json_t *this_json_array = json_object_get(rootJ, "progBpmKnob");
			size_t jThis;
			json_t *this_json_value;
			if (this_json_array) {
				json_array_foreach(this_json_array, jThis, this_json_value) {
					progBpmKnob[jThis] = json_real_value(this_json_value);
				}
			}
		}
*/
		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				json_t *this_json_array = json_object_get(rootJ, ("progUserTable_p"+to_string(p)+"t"+to_string(t)).c_str());
				size_t jThis;
				json_t *this_json_value;
				if (this_json_array) {
					json_array_foreach(this_json_array, jThis, this_json_value) {
						progUserTable[p][t][jThis] = json_integer_value(this_json_value);
					}
				}	
			}
		}

		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				for (int u = 0; u < MAXUSER; u++) {
					json_t *this_json_array = json_object_get(rootJ, ("progUserInputs_p"+to_string(p)+"t"+to_string(t)+"u"+to_string(u)).c_str());
					size_t jThis;
					json_t *this_json_value;
					if (this_json_array) {
						json_array_foreach(this_json_array, jThis, this_json_value) {
							progUserInputs[p][t][u][jThis] = json_integer_value(this_json_value);
						}
					}
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				json_t *this_json_array = json_object_get(rootJ, ("progUserValues_p"+to_string(p)+"t"+to_string(t)).c_str());
				size_t jThis;
				json_t *this_json_value;
				if (this_json_array) {
					json_array_foreach(this_json_array, jThis, this_json_value) {
						progUserValues[p][t][jThis] = json_real_value(this_json_value);
					}
				}
			}
		}
	}


/*

															░██████╗████████╗███████╗██████╗░░██████╗████████╗░░░░░░░░░░██████╗░█████╗░██╗░░░██╗███████╗
															██╔════╝╚══██╔══╝██╔════╝██╔══██╗██╔════╝╚══██╔══╝░░░░░░░░░██╔════╝██╔══██╗██║░░░██║██╔════╝
															╚█████╗░░░░██║░░░█████╗░░██████╔╝╚█████╗░░░░██║░░░░░░░░░░░░╚█████╗░███████║╚██╗░██╔╝█████╗░░
															░╚═══██╗░░░██║░░░██╔══╝░░██╔═══╝░░╚═══██╗░░░██║░░░░░░░░░░░░░╚═══██╗██╔══██║░╚████╔╝░██╔══╝░░
															██████╔╝░░░██║░░░███████╗██║░░░░░██████╔╝░░░██║░░░██╗██╗██╗██████╔╝██║░░██║░░╚██╔╝░░███████╗
															╚═════╝░░░░╚═╝░░░╚══════╝╚═╝░░░░░╚═════╝░░░░╚═╝░░░╚═╝╚═╝╚═╝╚═════╝░╚═╝░░╚═╝░░░╚═╝░░░╚══════╝
*/
	json_t *presetTrigStationToJson() {

		for (int t = 0; t < MAXTRACKS; t++)
			recStep[t] = step[t];

		json_t* rootJ = json_object();

		json_object_set_new(rootJ, "wait2RstSetting", json_integer(wait2RstSetting));

		// ---------- CLOCK
		
//		json_object_set_new(rootJ, "ppqn", json_integer(ppqn));
		json_object_set_new(rootJ, "resetOnRun", json_boolean(resetOnRun));
		json_object_set_new(rootJ, "ResetPulseOnRun", json_boolean(resetPulseOnRun));
		json_object_set_new(rootJ, "ResetOnStop", json_boolean(resetOnStop));
		json_object_set_new(rootJ, "ResetPulseOnStop", json_boolean(resetPulseOnStop));

//		json_object_set_new(rootJ, "cvClockIn", json_boolean(cvClockIn));
//		json_object_set_new(rootJ, "cvClockOut", json_boolean(cvClockOut));
		
		// --------- SEQUENCER

		json_object_set_new(rootJ, "runType", json_integer(runType));
//		json_object_set_new(rootJ, "rstClkOnRst", json_boolean(rstClkOnRst));
		json_object_set_new(rootJ, "rstSeqOnProgChange", json_boolean(rstSeqOnProgChange));

		json_object_set_new(rootJ, "bitResolution", json_integer(bitResolution));
		json_object_set_new(rootJ, "progression", json_integer(progression));

		json_object_set_new(rootJ, "progInType", json_boolean(progInType));
		json_object_set_new(rootJ, "lastProg", json_integer(lastProg));

		// -------------------------- PROGRAMS

		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				json_t *this_json_array = json_array();
				for (int s = 0; s < 16; s++) {
					json_array_append_new(this_json_array, json_integer(progSeq[p][t][s]));
				}
				json_object_set_new(rootJ, ("p"+to_string(p)+"t"+to_string(t)).c_str(), this_json_array);
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++) {
				json_array_append_new(this_json_array, json_integer(progSteps[p][t]));
			}
			json_object_set_new(rootJ, ("progSteps"+to_string(p)).c_str(), this_json_array);
		}
/*
		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++) {
				json_array_append_new(this_json_array, json_real(progDivMult[p][t]));
			}
			json_object_set_new(rootJ, ("progDivMult_p"+to_string(p)).c_str(), this_json_array);
		}
*/
		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++) {
				json_array_append_new(this_json_array, json_integer(progCurrentMode[p][t]));
			}
			json_object_set_new(rootJ, ("progCurrentMode_p"+to_string(p)).c_str(), this_json_array);
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++) {
				json_array_append_new(this_json_array, json_integer(progTuringMode[p][t]));
			}
			json_object_set_new(rootJ, ("progTuringMode_p"+to_string(p)).c_str(), this_json_array);
		}
/*
		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < ALLTRACKS; t++) {
				json_array_append_new(this_json_array, json_integer(progRevType[p][t]));
			}
			json_object_set_new(rootJ, ("progRevType_p"+to_string(p)).c_str(), this_json_array);
		}
*/
		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < ALLTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(progDontAdvanceSetting[p][t]));
			json_object_set_new(rootJ, ("progDontAdvanceSetting_p"+to_string(p)).c_str(), this_json_array);
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < ALLTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(progRstStepsWhen[p][t]));
			json_object_set_new(rootJ, ("progRstStepsWhen_p"+to_string(p)).c_str(), this_json_array);
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(progXcludeFromRst[p][t]));
			json_object_set_new(rootJ, ("progXcludeFromRst_p"+to_string(p)).c_str(), this_json_array);
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(progXcludeFromRun[p][t]));
			json_object_set_new(rootJ, ("progXcludeFromRun_p"+to_string(p)).c_str(), this_json_array);
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < ALLTRACKS; t++)
				json_array_append_new(this_json_array, json_integer(progSampleDelay[p][t]));
			json_object_set_new(rootJ, ("progSampleDelay_p"+to_string(p)).c_str(), this_json_array);
		}

		{
			json_t *this_json_array = json_array();
			for (int p = 0; p < 32; p++)
				json_array_append_new(this_json_array, json_integer(progSeqRunSetting[p]));
			json_object_set_new(rootJ, "progSeqRunSetting", this_json_array);
		}
/*
		{
			json_t *this_json_array = json_array();
			for (int p = 0; p < 32; p++)
				json_array_append_new(this_json_array, json_boolean(progInternalClock[p]));
			json_object_set_new(rootJ, "progInternalClock", this_json_array);
		}

		{
			json_t *this_json_array = json_array();
			for (int p = 0; p < 32; p++)
				json_array_append_new(this_json_array, json_real(progBpmKnob[p]));
			json_object_set_new(rootJ, "progBpmKnob", this_json_array);
		}
*/
		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				json_t *this_json_array = json_array();
				for (int i = 0; i < 4; i++)
					json_array_append_new(this_json_array, json_integer(progUserTable[p][t][i]));
				json_object_set_new(rootJ, ("progUserTable_p"+to_string(p)+"t"+to_string(t)).c_str(), this_json_array);
			}
		}

		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				for (int u = 0; u < MAXUSER; u++) {
					json_t *this_json_array = json_array();
					for (int i = 0; i < 2; i++) {
						json_array_append_new(this_json_array, json_integer(progUserInputs[p][t][u][i]));
					}
					json_object_set_new(rootJ, ("progUserInputs_p"+to_string(p)+"t"+to_string(t)+"u"+to_string(u)).c_str(), this_json_array);
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				json_t *this_json_array = json_array();
				for (int i = 0; i < 2; i++) {
					json_array_append_new(this_json_array, json_real(progUserValues[p][t][i]));
				}
				json_object_set_new(rootJ, ("progUserValues_p"+to_string(p)+"t"+to_string(t)).c_str(), this_json_array);
			}
		}

		return rootJ;
	}

	void menuLoadTrigStationPreset() {
static const char FILE_FILTERS[] = "TrigStation preset (.tst):tst,TST";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
#endif
		if (path)
			loadTrigStationPreset(path);

		free(path);
#if defined(METAMODULE)
		});
#endif
	}


	void loadTrigStationPreset(std::string path) {

		FILE *file = fopen(path.c_str(), "r");
		json_error_t error;
		json_t *rootJ = json_loadf(file, 0, &error);
		if (rootJ == NULL) {
			WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		}

		fclose(file);

		if (rootJ) {

			presetTrigStationFromJson(rootJ);

		} else {
			WARN("problem loading preset json file");
			//return;
		}
		
	}

	void menuSaveTrigStationPreset() {
	
		static const char FILE_FILTERS[] = "TrigStation preset (.tst):tst,TST";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters);
#endif
		if (path) {
			std::string strPath = path;
			if (strPath.substr(strPath.size() - 4) != ".tst" and strPath.substr(strPath.size() - 4) != ".TST")
				strPath += ".tst";
			path = strcpy(new char[strPath.length() + 1], strPath.c_str());
			saveTrigStationPreset(path, presetTrigStationToJson());
		}

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void saveTrigStationPreset(std::string path, json_t *rootJ) {

		if (rootJ) {
			FILE *file = fopen(path.c_str(), "w");
			if (!file) {
				WARN("[ SickoCV ] cannot open '%s' to write\n", path.c_str());
				//return;
			} else {
				json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
				json_decref(rootJ);
				fclose(file);
			}
		}
	}


// ********************************************************************************************************************************************

// ********************************************************************************************************************************************

// ********************************************************************************************************************************************

// ********************************************************************************************************************************************

// ********************************************************************************************************************************************


//	TrigSeq8x PRESET

	json_t *presetToJson() {

		json_t *rootJ = json_object();

		json_object_set_new(rootJ, "runType", json_integer(runType));
		json_object_set_new(rootJ, "dontAdvanceSetting", json_integer(dontAdvanceSetting[MC]));	//  <------------- OTTIMIZZARE per registrare gli 8 tweaks

		json_object_set_new(rootJ, "progInType", json_boolean(progInType));
		json_object_set_new(rootJ, "lastProg", json_integer(lastProg));

		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				json_t *this_json_array = json_array();
				for (int s = 0; s < 16; s++) {
					json_array_append_new(this_json_array, json_integer(progSeq[p][t][s]));
				}
				json_object_set_new(rootJ, ("p"+to_string(p)+"t"+to_string(t)).c_str(), this_json_array);
			}
		}

		for (int p = 0; p < 32; p++) {
			json_t *this_json_array = json_array();
			for (int t = 0; t < MAXTRACKS; t++) {
				json_array_append_new(this_json_array, json_integer(progSteps[p][t]));
			}
			json_object_set_new(rootJ, ("progSteps_p"+to_string(p)).c_str(), this_json_array);
		}

		return rootJ;
	}



	void presetFromJson(json_t *rootJ) {

		json_t* runTypeJ = json_object_get(rootJ, "runType");
		if (runTypeJ) {
			runType = json_integer_value(runTypeJ);
			if (runType < 0 || runType > 1)
				runType = 0;
		}
/*
		json_t* revTypeJ = json_object_get(rootJ, "revType");
		if (revTypeJ) {
			revType[MC] = json_integer_value(revTypeJ);
			if (revType[MC] < 0 || revType[MC] > 1)
				revType[MC] = 0;
		}
		*/

		json_t* rstStepsWhenJ = json_object_get(rootJ, "rstStepsWhen");
		if (rstStepsWhenJ) {
			rstStepsWhen[MC] = json_boolean_value(rstStepsWhenJ);
		}

		json_t* dontAdvanceSettingJ = json_object_get(rootJ, "dontAdvanceSetting");
		if (dontAdvanceSettingJ) {
			dontAdvanceSetting[MC] = json_integer_value(dontAdvanceSettingJ);
		}
/*
		json_t* turingModeJ = json_object_get(rootJ, "turingMode");
		if (turingModeJ) {
			turingMode = json_boolean_value(turingModeJ);
		}

		json_t* bitResolutionJ = json_object_get(rootJ, "bitResolution");
		if (bitResolutionJ) {
			bitResolution = json_integer_value(bitResolutionJ);
			if (bitResolution < 0 && bitResolution > 1)
				bitResolution = BIT_8;
		}

		json_t* progressionJ = json_object_get(rootJ, "progression");
		if (progressionJ) {
			progression = json_integer_value(progressionJ);
			if (progression < 0 && progression > 2)
				progression = STD2x_PROGRESSION;
		}
*/
		json_t* progInTypeJ = json_object_get(rootJ, "progInType");
		if (progInTypeJ) {
			progInType = json_boolean_value(progInTypeJ);
		}

		json_t* lastProgJ = json_object_get(rootJ, "lastProg");
		if (lastProgJ) {
			lastProg = json_integer_value(lastProgJ);
			if (lastProg < 0 || lastProg > 31)
				lastProg = 0;
		}

		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				json_t *this_json_array = json_object_get(rootJ, ("p"+to_string(p)+"t"+to_string(t)).c_str());
				size_t jThis;
				json_t *this_json_value;
				if (this_json_array) {
					json_array_foreach(this_json_array, jThis, this_json_value) {
						progSeq[p][t][jThis] = json_integer_value(this_json_value);
					}
				}
			}
		}

		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				json_t *this_json_array = json_object_get(rootJ, ("progSteps"+to_string(p)).c_str());
				size_t jThis;
				json_t *this_json_value;
				if (this_json_array) {
					json_array_foreach(this_json_array, jThis, this_json_value) {
						progSteps[p][t] = json_integer_value(this_json_value);
					}
				}
			}
		}
/*
		for (int p = 0; p < 32; p++) {
			for (int t = 0; t < MAXTRACKS; t++) {
				json_t *progRst_json_array = json_object_get(rootJ, ("progRst"+to_string(p)).c_str());
				size_t jRst;
				json_t *progRst_json_value;
				if (progRst_json_array) {
					json_array_foreach(progRst_json_array, jRst, progRst_json_value) {
						progRst[p][t] = json_integer_value(progRst_json_value);
					}
				}
			}
		}
*/
	}

	void menuLoadAllSequences() {
		static const char FILE_FILTERS[] = "trigSeq8x preset (.t8p):t8p,T8P";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
#endif
		if (path)
			loadAllSequences(path);

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void loadAllSequences(std::string path) {

		FILE *file = fopen(path.c_str(), "r");
		json_error_t error;
		json_t *rootJ = json_loadf(file, 0, &error);
		if (rootJ == NULL) {
			WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		}

		fclose(file);

		if (rootJ) {

			presetFromJson(rootJ);

		} else {
			WARN("problem loading preset json file");
			//return;
		}
		
	}

	void menuSaveAllSequences() {

		static const char FILE_FILTERS[] = "trigSeq8x preset (.t8p):t8p,T8P";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters);
#endif
		if (path) {
			std::string strPath = path;
			if (strPath.substr(strPath.size() - 4) != ".t8p" and strPath.substr(strPath.size() - 4) != ".T8P")
				strPath += ".t8p";
			path = strcpy(new char[strPath.length() + 1], strPath.c_str());
			saveAllSequences(path, presetToJson());
		}

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void saveAllSequences(std::string path, json_t *rootJ) {

		if (rootJ) {
			FILE *file = fopen(path.c_str(), "w");
			if (!file) {
				WARN("[ SickoCV ] cannot open '%s' to write\n", path.c_str());
				//return;
			} else {
				json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
				json_decref(rootJ);
				fclose(file);
			}
		}
	}

	// ------------------------ LOAD / SAVE   SINGLE SEQUENCE

	json_t *sequenceToJson(int t) {

		json_t *rootJ = json_object();

		json_t *wSeq_json_array = json_array();
		for (int s = 0; s < 16; s++) {
			json_array_append_new(wSeq_json_array, json_integer(wSeq[t][s]));
		}
		json_object_set_new(rootJ, "sr", wSeq_json_array);	
		json_object_set_new(rootJ, "length", json_integer((int)params[LENGTH_PARAM].getValue()));
//		json_object_set_new(rootJ, "reset", json_integer((int)params[RST_PARAM].getValue()));
//		json_object_set_new(rootJ, "offset", json_real(0));

		return rootJ;
	}

	void sequenceFromJson(int t, json_t *rootJ) {

		json_t *wSeq_json_array = json_object_get(rootJ, "sr");
		size_t s;
		json_t *wSeq_json_value;
		if (wSeq_json_array) {
			json_array_foreach(wSeq_json_array, s, wSeq_json_value) {
				params[STEP_PARAM+(t*16)+s].setValue(json_integer_value(wSeq_json_value));
			}
		}

		json_t* lengthJ = json_object_get(rootJ, "length");
		if (lengthJ) {
			if (json_integer_value(lengthJ) < 1 || json_integer_value(lengthJ) > 16)
				params[LENGTH_PARAM].setValue(16);
			else
				params[LENGTH_PARAM].setValue(int(json_integer_value(lengthJ)));
		}

//		json_t* rstJ = json_object_get(rootJ, "reset");
//		if (rstJ) {
//			if (json_integer_value(rstJ) < 0 || json_integer_value(rstJ) > 1)
//				params[RST_PARAM].setValue(0);
//			else
//				params[RST_PARAM].setValue(json_integer_value(rstJ));
//		}

	}

	void menuLoadSequence(int track) {
		static const char FILE_FILTERS[] = "trigSeq sequence (.tss):tss,TSS";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
#endif
		if (path)
			loadSequence(track, path);

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void loadSequence(int track, std::string path) {

		FILE *file = fopen(path.c_str(), "r");
		json_error_t error;
		json_t *rootJ = json_loadf(file, 0, &error);
		if (rootJ == NULL) {
			WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		}

		fclose(file);

		if (rootJ) {

			sequenceFromJson(track, rootJ);

		} else {
			WARN("problem loading preset json file");
			//return;
		}
		
	}

	void menuSaveSequence(int track) {

		static const char FILE_FILTERS[] = "trigSeq sequence (.tss):tss,TSS";
		osdialog_filters* filters = osdialog_filters_parse(FILE_FILTERS);
		DEFER({osdialog_filters_free(filters);});
#if defined(METAMODULE)
		async_osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters, [=, this](char *path) {
#else
		char *path = osdialog_file(OSDIALOG_SAVE, NULL, NULL, filters);
#endif
		if (path) {
			std::string strPath = path;
			if (strPath.substr(strPath.size() - 4) != ".tss" and strPath.substr(strPath.size() - 4) != ".TSS")
				strPath += ".tss";
			path = strcpy(new char[strPath.length() + 1], strPath.c_str());
			saveSequence(path, sequenceToJson(track));
		}

		free(path);
#if defined(METAMODULE)
		});
#endif
	}

	void saveSequence(std::string path, json_t *rootJ) {

		if (rootJ) {
			FILE *file = fopen(path.c_str(), "w");
			if (!file) {
				WARN("[ SickoCV ] cannot open '%s' to write\n", path.c_str());
				//return;
			} else {
				json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
				json_decref(rootJ);
				fclose(file);
			}
		}
	}
