# SickoCV
VCV Rack plugin modules

![SickoCV modules](https://user-images.githubusercontent.com/80784296/183292218-2263acd5-1d1c-41ca-849e-f020b2ab2c7b.JPG)

## Btoggler
### 8 buffered toggle switches (flip flop) with gate output and signal routing with a fade knob to route also audio signals with any clicks
#### - Purpose:
'btoggler' can be used to mute/unmute up to 8 CV or AUDIO signals, in sync to a tempo clock source. For example it can be used connected to a "MIDI>GATE" module which receives controls from a midi controller, so btoggler will make start/stop sequencers or audio parts on next beat.
#### - Detailed instructions:
Connect a clock source.

When ARM input is triggered the IN input will start to be routed to OUT on next clock detection and GATE output will provide a high state.

With another ARM triggering (unarm) the routing will stop on next clock detection and GATE output will go low.

FADE knob (up to 50ms) can be used to avoid attack or release clicks when audio signals are connected to IN input.

If ARM is triggered again before clock detection it will abort arming or unarming.

Triggering RESET input will immediately stops the routing.

Triggering RESETALL input will immediately stops all the 8 routings.

NOTE: input triggers are considered high when greater than 0v.
 
## Btoggler+
### 8 buffered toggle switches (flip flop) with signal routing and warnings to use with led midi controllers
#### - Purpose:
'btoggler+' it's almost the same of previous one, but it has a further functionality (WRN outs) to be used with programmable led midi controllers to have a visual feedback on controller.
#### - Detailed instructions:
The same of the previous one, plus:

When armed or unarmed, the WRN (warning) output will provide a sequence of pulses until next clock pulse is detected. Then it will act as the OUT output (routing) if the FADE knob is set to 0ms, else it will act as the GATE output (high gate).

WA and WR knobs set the attack (arm) and release (unarm) pulserate up to 200 ms of the warning pulses.

If set to 0ms, WRN will output a low gate during warning time and if set to to max (200ms) it will output a high gate.  
As to say: if WA is set to 0 and WR is set to max(200), WRN output will act like the GATE output.  
Otherwise if WA is set to max(200) and WR is set to 0, WRN output will act as simple toggle switch with no buffer feature.

As mentioned before, WRN outputs are meant to be used connected to a led programmable midi controller.  
The idea is to connect a "MIDI>GATE" module to ARM inputs and WRN outputs to a "GATE>MIDI" module.  
With a configuration like this, you can press buttons on the midi controller and start/stop beat-synced sequencers or audio, having a visual led feedback on the controller.

NOTE: input triggers are considered high when greater than 0v.

## Calcs
### calculates sums, differences, multiplications, divisions and averages of 3 CV inputs
#### - Usage:
A, B and C are the inputs. The output tables provide simple math calculations and averages between two inputs or the average of all of them.

U/B (Unipolar/Bipolar) switch will set the range of the outputs to 0/10V or -5/+5v.

## Toggler / Toggler Compact
### Stereo signal toggle switch (flip flop) router, ASR envelope with VCA, in regular and compact form factor
#### - Purposes:
- ASR envelope generator with builtin VCA
- ASR envelope generator
- mute/unmute CV or mono/stereo AUDIO signals according to an ASR envelope activated by a Gate or Toggle Triggers
#### - Detailed instructions:
**TOGGLE MODE**

On receiving a trigger on TRIG input, it will send the L+(R) inputs to L+(R) outputs and set the GATE output to high. On next trigger it will interrupt L+(R) outputs and set the GATE output to low.

Attack, sustain and release knob set the envelope of the routed signal.

A, S, R CVinputs are added to respective knob values.

If L or (R) inputs are not connected L and (R) outputs will provide just the envelope, so a mono signal can be connected to L input to route it to L output and nothing connected to (R) input to have the envelope on (R) output.

A trigger on RESET input will reset the toggle state.

**GATE MODE**

The same of toggle mode, but the signals will be routed only while GATE input is in a high state.

NOTE: input triggers or gates are considered high when greater than 0v.

**SPECIAL BEHAVIOURS**

If a new GATE or Toggle TRIGGER is detected on Attack or Release phases, the envelope ramp will restart from the reached point.

If Attack is set to 0 (and release is set greater than 0) and a new GATE or Toggle TRIGGER is detected before Release phase has ended, the next Release phase will start from the previous reached release point.

If Release is set to 0 (and attack is set greater than 0) and a new GATE or Toggle TRIGGER is detected before Attack phase has ended, the next Attack phase will start from the previous reached Attack point.

## Credits
The Component Library graphics for these modules are copyright © VCV and licensed under [CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/)

Thanks to [Omri Cohen](https://omricohen-music.com/) for support.
