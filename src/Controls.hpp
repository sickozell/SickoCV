#pragma once

struct SickoSwitch_Three_Horiz : SvgSwitch
{
    SickoSwitch_Three_Horiz()
    {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/Switch_H3_left.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/Switch_H3_center.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/Switch_H3_right.svg")));
        shadow->opacity = 0.0f;
        box.size.x = 29.0f;
        box.size.y = 29.0f;
    }
};

struct SickoSwitch_CKSS_Horiz : SvgSwitch
{
    SickoSwitch_CKSS_Horiz()
    {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/CKSS_H_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/CKSS_H_1.svg")));
        shadow->opacity = 0.0f;
        box.size.x = 20.0f;
        box.size.y = 13.0f;
    }
};
