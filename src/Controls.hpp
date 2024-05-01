#pragma once

struct SickoSwitch_Three_Horiz : SvgSwitch {
    SickoSwitch_Three_Horiz() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/Switch_H3_left.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/Switch_H3_center.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/Switch_H3_right.svg")));
        shadow->opacity = 0.0f;
        box.size.x = 29.0f;
        box.size.y = 29.0f;
    }
};

struct SickoSwitch_CKSS_Three_Vert : SvgSwitch {
    SickoSwitch_CKSS_Three_Vert() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/CKSSThree_V_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/CKSSThree_V_1.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/CKSSThree_V_2.svg")));
        shadow->opacity = 0.0f;
    }
};

struct SickoSwitch_CKSS_Four_Horiz : SvgSwitch {
    SickoSwitch_CKSS_Four_Horiz() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/CKSSFour_H_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/CKSSFour_H_1.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/CKSSFour_H_2.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/CKSSFour_H_3.svg")));
        shadow->opacity = 0.0f;
    }
};

struct SickoSwitch_CKSS_Horiz : SvgSwitch {
    SickoSwitch_CKSS_Horiz() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/CKSS_H_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/CKSS_H_1.svg")));
        shadow->opacity = 0.0f;
        box.size.x = 20.0f;
        box.size.y = 13.0f;
    }
};

struct SickoInPort : app::SvgPort {
    SickoInPort() {
        setSvg(Svg::load(asset::plugin(pluginInstance, "res/component/SickoInPort.svg")));
    }
};

struct SickoOutPort : app::SvgPort {
    SickoOutPort() {
        setSvg(Svg::load(asset::plugin(pluginInstance, "res/component/SickoOutPort.svg")));
    }
};

struct SickoTrimpot : RoundKnob {
    SickoTrimpot() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/SickoTrimpot.svg")));
        bg->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/SickoTrimpot_bg.svg")));
    }
};

struct SickoKnob : RoundKnob {
    SickoKnob() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/SickoKnob.svg")));
        bg->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/SickoKnob_bg.svg")));
    }
};

struct SickoSmallKnob : RoundKnob {
    SickoSmallKnob() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/SickoSmallKnob.svg")));
        bg->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/SickoSmallKnob_bg.svg")));
    }
};

struct SickoBigKnob : RoundKnob {
    SickoBigKnob() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/SickoBigKnob.svg")));
        bg->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/SickoBigKnob_bg.svg")));
    }
};

struct SickoLargeKnob : RoundKnob {
    SickoLargeKnob() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/SickoLargeKnob.svg")));
        bg->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/SickoLargeKnob_bg.svg")));
    }
};