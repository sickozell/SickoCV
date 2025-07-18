
#include "plugin.hpp"

using namespace std;

struct SampleDelay : Module {

	enum ParamIds {
		ENUMS(DELAY_PARAM, 3),
		NUM_PARAMS 
	};
	enum InputIds {
		ENUMS(SIGNAL_INPUT, 3),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(SIGNAL_OUTPUT, 3),
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
  
	int sampleDelay[3] = {0, 0, 0};
	float delayBuffer[3][6] = {};
	int recPos = 0;
	int readPos = 0;


	SampleDelay() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		for (int t = 0; t < 3; t++) {
			configInput(SIGNAL_INPUT+t,("Signal #"+to_string(t+1)).c_str());
			configParam(DELAY_PARAM+t, 0.f, 5.f, 0.f, ("Delay #"+to_string(t+1)).c_str(), " samples");
			paramQuantities[DELAY_PARAM+t]->snapEnabled = true;
			configOutput(SIGNAL_OUTPUT+t,("Signal #"+to_string(t+1)).c_str());
		}
		
	}



	void process(const ProcessArgs &args) override {


		for (int t = 0; t < 3; t++) {

			sampleDelay[t] = int(params[DELAY_PARAM+t].getValue());

			if (t == 0)

				delayBuffer[t][recPos] = inputs[SIGNAL_INPUT+t].getVoltage();

			else if (inputs[SIGNAL_INPUT+t].isConnected())
				
				delayBuffer[t][recPos] = inputs[SIGNAL_INPUT+t].getVoltage();
			
			else

				delayBuffer[t][recPos] = delayBuffer[t-1][recPos];

			readPos = recPos - sampleDelay[t]; 

			if (readPos < 0)
				readPos = 6 + recPos - sampleDelay[t];

			outputs[SIGNAL_OUTPUT+t].setVoltage(delayBuffer[t][readPos]);
		}

		recPos++;

		if (recPos > 5)
			recPos = 0;
	}
	
};


struct SampleDelayWidget : ModuleWidget {
	SampleDelayWidget(SampleDelay *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SampleDelay.svg")));

		const float xCenter = 5.08f;

		const float yStart = 17.f;
		const float yDelta = 38.7f;
		const float yKnob = 9.4f;
		const float yOut = 22.9f;

		for (int t = 0; t < 3; t++) {
			addInput(createInputCentered<SickoInPort>(mm2px(Vec(xCenter, yStart+(t*yDelta))), module, SampleDelay::SIGNAL_INPUT+t));
			addParam(createParamCentered<SickoTrimpot>(mm2px(Vec(xCenter,  yStart+yKnob+(t*yDelta))), module, SampleDelay::DELAY_PARAM+t));
			addOutput(createOutputCentered<SickoOutPort>(mm2px(Vec(xCenter,  yStart+yOut+(t*yDelta))), module, SampleDelay::SIGNAL_OUTPUT+t));
		}

		
	}

};

Model *modelSampleDelay = createModel<SampleDelay, SampleDelayWidget>("SampleDelay");
