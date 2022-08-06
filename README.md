# SickoCV
VCV Rack plugin modules
### Btoggler
##### 8 buffered toggle switches (or flip flop) with gate output and signal routing with a fade knob to transfer also audio signals

Connect a clock source. 
When ARM input is triggered the IN input will start to be routed to OUT on next clock detection and GATE output will provide a high state. 
With another ARM triggering (unarm) the routing will stop on next clock detection and GATE output will go low. 
FADE knob can be used to avoid attack or release clicking when audio signals are connected to IN input. 
If ARM is triggered again before clock detection it will abort arming or unarming. 
Triggering RESET input will immediately stops the routing. 
Triggering RESETALL input will immediately stops the 8 routings. 
 
### Btoggler+
##### 8 buffered toggle switches (flip flop) with signal routing and warnings to use with led midi controllers
It's almost the same of previous one.
When armed or unarmed, the WRN (warning) output will provide a sequence of pulses until clock is detected. Then it will act as the OUT output if the FADE knob is set to 0ms, else it will act as the GATE output. 
WA and WR knobs set the attack (arm) and release (unarm) pulserate up to 200 ms of the warning pulses. 
If set to 0ms  it will output a low gate during warning time and if set to to max (200ms) it will output a high gate.
For example, if WA is set to 0 and WR is set to max(200), WRN output will act like the GATE output. Otherwise if WA is set to max(200) and WR is set to 0, WRN output will act as simple toggle switch with no buffer feature. 
Actually WRN  it's meant to be used connected to a led programmable midi controller.
For example: "MIDI TO GATE" module wired to ARM input and WRN output wired to "GATE TO MIDI". With a configuration like this, sequencers or audio can be launched from controller having a visual led feedback, everything clock-synced.

### Calcs
##### calculates sums, differences, multiplications, divisions and averages of 3 CV inputs

a, b and c are the inputs. The output tables provide simple math calculation between these inputs.

### Toggler
##### Stereo signal toggle switch (flip flop) router, ASR envelope with VCA
TOGGLE mode: on receiving a trigger on TRG/GATE input, it will send the L+(R) inputs to L+(R) outputs and set the GATE output to high. On next trigger it will interrupt L+(R) output and set the GATE output to low.

Attack, sustain and release knob set the envelope of the routed signal when triggered.
A, S, R CVinputs are added to respective knobs values.
If L or (R) inputs are not connected L and (R) outputs will provide just the envelope. It can be used a mono signal connected to L input to route it to L output and nothing connected to (R) input to have the envelope on (R) output.
A trigger on RESET input will reset the toggle state.

GATE MODE:
The same of toggle mode, but the signals will be routed only when TRG/GTE input receives a high gate.
