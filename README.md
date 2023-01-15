# SickoCV v2.5.1-beta14
VCV Rack plugin modules (BETA TEST AREA)  
Compile or **download binary for ANY platform** on the releases page

## IMPORTANT INSTALLATION NOTE
If you don't use VCV development environment and run regular VCV install,  
the new modules will be shown up only if you have a **full subscription** to Sickozell plugin modules.  

So if you have added only some Sickozell modules to VCV you will not see the new ones.  

Please check your subscription on https://library.vcvrack.com/plugins and look for the SickoCV line that has to be like this:  
![subscription](https://user-images.githubusercontent.com/80784296/207971796-96163a4b-6fa9-4073-bda8-9df1e61f900b.JPG)

## Current modules in beta testing:
### SickoPlayer
Wav sample player with v/oct, polyphony, tuning, envelope, loop, interpolation and anti-aliasing filter

### DrumPlayer  
Drum Sample Player with audio signal interpolation and anti-aliasing filter  
- added a "sample loaded led" for each slot in DrumPlayer  

### DrumPlayer+  
Drum Sample Player with audio signal interpolation and anti-aliasing filter  

### Drummer  
Accent and choke utility for drum modules lacking these features  
- code optimization  

### Drummer4
Accent and choke utility for drum modules lacking these features  
- added choke functionality and changed panel
- code optimization  

### Drummer4+
Accent and choke utility for drum modules lacking these features  
- added CV inputs with attenuverters to Drummer4  

### Blender
Stereo crossfade mixer with double modulation  
- added input attenuators
- changed artwork panel

## **to do list:** 
- nothing in queue


# SickoCV v2.5.1
VCV Rack plugin modules

![SickoCV modules 2 5 1](https://user-images.githubusercontent.com/80784296/212480942-1812dcd5-8a6b-449a-8a5d-956da30b4cf5.JPG)

## Blender
### Polyphonic stereo crossfade mixer with double modulation
#### - DESCRIPTION
'blender' is a crossfade mixer of mono or stereo signals.  
It can be used either with cv signals or audio sources.  
Mix can be modulated by uni/bipolar signals.  
Modulation can be further modulated by another signal.  
Audio rate modulations are allowed.

![blender](https://user-images.githubusercontent.com/80784296/211660967-ce9aa25d-cc8f-45a9-beae-3381a13cf0af.JPG)

#### - INSTRUCTIONS
Connect CVs or audio sources to IN1 and IN2, mono or stereo signals can be independently used. Polyphonic inputs are allowed and are left/right independent, but accordingly to number of channels of IN1 input.  
PHASE switches invert the sign of input signals.  
MIX knob sets the crossfade level of the inputs.  
Inputs volume can be adjusted via two attenuators.  
Master volume can be amplified up to 200%, a hard clip ±5v switch is present.  
Output replicates input polyphony, but deticking in the context menu 'Polyphonic outs' option will mix channels into monophonic outs.

**MOD section**  
Connecting MIX MOD CV input enables mix modulation. ATNV knob attenuverts CV input.  
CV input range is usually unipolar 0-10v. RNG switch in 'bipolar' position adds +5v to CV input, so audio signals can be used for modulation.    
Modulation is added to the MIX knob.

**MOD2 section**  
MOD2 can be used to add modulation to the MOD attenuverter knob in MOD section, the rules are the same.

## Blender8
### 8 single crossfade mixers with modulation
#### - DESCRIPTION
'blender8' is a set of 8 crossfade mixers of two signals.  
As the previous one it can be used either with cv signals or audio sources.  
Mix can be modulated by uni/bipolar signals.  
Audio rate modulations are allowed.

![blender8](https://user-images.githubusercontent.com/80784296/201516772-12dac17b-f8a0-4f82-946a-da8b7d254b09.JPG)

#### - INSTRUCTIONS
'blender8' provides 8 mono crossfade mixers and differs from 'blender' module in the following things.  
Only the IN2 input signal can be phase inverted.  
If a CV input is connected for modulation, CV sets the mix percentage and the MIX knob becomes the CV attenuverter.

## bToggler / bToggler Compact
### Buffered stereo signal toggle switch router, with VCA and ASR envelope generator, in regular and compact form factor
#### - DESCRIPTION
- Buffered Toggled VCA with builtin ASR envelope generator
- Buffered Toggled ASR envelope generator
- Buffer mute/unmute CVs or mono/stereo AUDIO signals according to an ASR envelope activated by Toggle Triggers

![btoggler](https://user-images.githubusercontent.com/80784296/211221913-2ac04d94-b80b-4222-a02b-2719e0fb4d38.JPG)

#### - INSTRUCTIONS
Connect a clock source.

When ARM input is triggered (arm on), the L+(R) inputs will start to be routed to L+(R) outputs on next clock detection (according to ASR envelope values) and GATE output will provide a high state.

Then, with another ARM triggering (arm off) the routing will stop on next clock detection and GATE output will go low.

If ARM is triggered again before clock detection it will abort arming (unarm).

Attack, Sustain and Release knobs set the envelope of the routed signals.

A, S, R CVinputs are added to respective knob values, bToggler module has attenuverters.

If L or (R) inputs are not connected, L and (R) outputs will provide just the envelope, so a mono signal can be connected to L input to route it to L output and nothing connected to (R) input to have the envelope on (R) output.

A trigger on RESET input will reset the toggle state.

NOTE: input trigger threshold is +1v.


## bToggler8
### 8 buffered toggle switch signal router
#### - DESCRIPTION
'bToggler8' can be used to mute/unmute up to 8 CVs or AUDIO signals, in sync to a tempo clock source.  
For example it can be used to play/stop drumkit parts independently (kick, snare, hats, etc):
- connect an appropriate clock source to CLOCK 
- connect the ARMs to a "MIDI>GATE" module which receives controls from a midi controller
- connect the INs to the sequencer outs, one for kick, one for snare, etc.
- connect the OUTs to the trigger inputs of the single drum modules

Then, by pressing buttons on the controller, 'bToggler8' will actually start/stop the single drum parts on the next received clock pulse.

Otherwise bToggler OUTs can be connected to envelope generators. In that case the GATE output should be connected to the IN input to activate the envelope.

'bToggler8' can also be used to route audio signals. Connect IN to the audio source and OUT to the mixer: the FADE knob will avoid clicks.

![btoggler8](https://user-images.githubusercontent.com/80784296/201516798-acfa6672-07d0-4f0e-bd38-6269a204eb64.JPG)

#### - INSTRUCTIONS
Connect a clock source.

When ARM input is triggered (arm on) the IN input will start to be routed to OUT on next clock detection and GATE output will provide a high state.

Then, with another ARM triggering (arm off) the routing will stop on next clock detection and GATE output will go low.

FADE knob (up to 50ms) can be used to avoid attack or release clicks when audio signals are connected to IN input.

If ARM is triggered again before clock detection it will abort arming (unarm).

Triggering RESET input will immediately stop the routing.

Triggering RESETALL input will immediately stop all the 8 routings.

NOTE: input trigger threshold is +1v.  
 
Here below is one example of bToggler8 usage. When buttons are pressed on PULSES module, incoming triggers from the sequencer are routed to drum modules only when the first step of the sequencer is reached. If buttons are pressed again, the routing will stop on next first step of the sequencer.

![bToggler8 example](https://user-images.githubusercontent.com/80784296/204083532-db145211-1f61-45cd-9c4d-572fc243d7d3.JPG)  
[Download example](./examples/bToggler8%20example.vcvs?raw=true) (right-click -> save link as)

## bToggler8+
### 8 buffered toggle switch router, plus warnings to use with led midi controllers
#### - DESCRIPTION
'bToggler8+' is almost the same of the previous one, but it has a further feature (WRN outs) to be used with programmable led midi controllers to have a visual feedback on the controller.

Some midi controllers can light up or turn off their button leds by receiving the same commands they send.  
Taking advantage of this functionality, connect the WRN outs to a "GATE>MIDI" module connected to the same controller of the ARM inputs.  
So when pressing buttons on controller, 'bToggler8+' will actually play/stop the sequencers or audio, and simultaneously give a visual feedback on the controller.

![btoggler8plus](https://user-images.githubusercontent.com/80784296/201516811-40c75bb5-84d2-411b-b9ed-fa876e178258.JPG)

#### - INSTRUCTIONS
The same of the previous one, plus:

When 'armed on' or 'armed off', the WRN (warning) output will provide a sequence of pulses until next clock is detected.  
Then it will act as the OUT output (the routed signal) if the FADE knob is set to 0ms, else it will act as the GATE output (high gate).  
This is because if 'bToggler8+' is receiving signals from sequencers, the FADE knob will be set to 0 and the led will light up the same as sequencers trigs.  
Otherwise, if fade knob is set different from 0, it is supposed that an audio signal is routed, so you'll see a fixed led light on the controller.

WA and WR knobs set the attack (arm on) and release (arm off) pulserate up to 200 ms of the warning pulses. These are two independent settings because you would like to notice if the routing is going to start or stop.

If WA or WR are set to 0ms, WRN will output a low gate during warning time and if set to to max (200ms) it will output a high gate.  
As to say: if WA is set to 0 and WR is set to max(200), WRN output will act like the GATE output.  
Otherwise if WA is set to max(200) and WR is set to 0, WRN output will act as simple toggle switch with no buffer feature.

In the context menu there is the option to invert WRN output when used with triggers. It can be useful when used with a Led Controller, so when a channel is 'toggled on', leds will stay turned on until a trig is received and they will be turned off for 100ms.  

NOTE: input trigger threshold is +1v.  

Here below is one example of bToggler+ usage. The MIDI>GATE module is connected to a programmable Led Midi controller and receives buttonpresses from it. The GATE>MIDI module send back triggers incoming from the sequencer to the controller, turning on and off the corresponding led buttons only when triggers are actually routed to drum modules. Routing rules are the same of previous example.

![bToggler8plus example](https://user-images.githubusercontent.com/80784296/204083544-34ecf3b0-0d12-4965-bd72-f3bb85339551.JPG)  
[Download example](./examples/bToggler8plus%20example.vcvs?raw=true) (right-click -> save link as)

## Calcs
### Calculates sums, differences, multiplications, divisions and averages of 3 CV inputs

![calcs](https://user-images.githubusercontent.com/80784296/201516821-8ea683bd-db11-4687-971d-67bef380b81c.JPG)

#### - INSTRUCTIONS
A, B and C are the inputs. The output tables provide simple math calculations and averages between two inputs or the average of all of them.

U/B (Unipolar/Bipolar) switch clamps the outputs to 0/10V or ±5v.

## Drummer Drummer4 Drummer4+
### Accent and choke utility for drum modules lacking these features

![drummer](https://user-images.githubusercontent.com/80784296/212536993-c8ac8011-b324-4dae-99f6-8f8b548557eb.JPG)

#### - INSTRUCTIONS
Drummer and Drummer4/Drummer4+ module can handle 2 or 4 drum sounds with separate standard and accent volume levels set by respective knobs.  

Connect the IN input to a drum-like audio source or sample player, and OUT output to the mixer.  
Connect the TRIG input to the same module that feeds the drum module, it can be a sequencer or every other pulse generation module.  
Connect the ACC input to the module which generates the accents, it can be the sequencer or every other suitable module.  
When ACC is triggered at the same time as the TRIG input, Drummer module will output the Accent Level set by "Accent Level knob" instead of the one set by "Standard Level Knob".  

Input triggers threshold is +1v.  
Each knob range is from 0 to 200% of the incoming IN level.  
LIMIT switch hard clips the output in the range ±5v.  
When CHOKE switch is on and a trigger occurs, the other slot (Drummer) or the next slot (Drummer4) is muted (for example when used with closed/open hihat sounds).  

- **Drummer note:**  
If 1 OUT is not connected, the audio signal will be mixed with the other one connected.  
In CHOKE mode, if both TRIG inputs are triggered at the same time, the upper section (#1) will have the priority and the lower one will be ignored.

Example of Drummer module usage:

![drummer example](https://user-images.githubusercontent.com/80784296/212531420-150a0d94-12c6-463e-b46b-0828f5d45895.JPG)  
[Download example](./examples/drummer%20example.vcvs?raw=true) (right-click -> save link as)

- **Drummer4 note:**  
If one slot OUT is not connected, the audio signal will be added to the next one. For example if you connect only out #2 and #4, out #1 and #3 will be respectively mixed with those ones, if you connect only out #4, this socket will output all the channels.

Example of Drummer4 module usage:

![drummer4 example](https://user-images.githubusercontent.com/80784296/212531441-575f9b49-dee2-47ca-a82d-0861a10145e5.JPG)  
[Download example](./examples/drummer4%20example.vcvs?raw=true) (right-click -> save link as)

- **Drummer4+ note:**  
Drummer4+ it's the same of Drummer4. It only adds attenuverted CV inputs to parameter knobs.

## DrumPlayer DrumPlayer+
### 4 channel Drum Sample Player with accent and choke functionality

![drumplayer](https://user-images.githubusercontent.com/80784296/212537149-9a38032b-694f-488e-86b5-31bdfad43c7b.JPG)

#### INSTRUCTIONS  
Load wav samples in the slots using context menu.  

When TRIG input is triggered the sample will be played at the volume percentage set by to "Standard Level" knob + its relative attenuverted CVinput.  
If ACCENT input is HIGH when TRIG occurs, the sample will be played at "Accent Level" knob + its attenuverted CVinput.  

Playing speed can be set by SPD knob from 1 to 200% and modulated with its attenuverted CVinput. Speed can be modified during sample playback.  
External modulation is allowed only on drumPlayer+  

If CHOKE switch is on when TRIG occurs, the playback of next slot is stopped with a 1ms fade out: it's commonly used to simulate a closed/open hihat.  
LIM switch is a hard clipping limiter to ±5v on the output.  

#### CONTEXT MENU
**Sample Slots**  
Click on the slot number to open dialog.  
When the sample is loaded the green led on the panel is turned on (drumPlayer), or the small 7segment display will show the first 5 chars of the filename (drumPlayer+).  
Use Clear options to unload samples from slots.  
Just right-click over the led areas or the displays to access the quick-load menus.  

**Interpolation**  
There are 3 different interpolation algorithms, that are engaged during playback only when the sample samplerate differs from VCV working samplerate or playback speed differs from 100%.  
- 'No interpolation' can be used when sample rates match and speed is 100% constant  
- 'Linear 1' and 'Linear 2' interpolates the samples with different weighted averages  
- 'Hermite' uses a Cubic Hermite spline interpolation that offers a better result (default)  

**Anti-aliasing filter**  
Anti-aliasing filter is made up with 2x oversampling and a 20khz lowpass filter.  

**Outs mode**  
Normalled (default): if one slot out is not connected, its output will be added to the next slot  
Solo: every slot has its own out socket  
Unconnected on Out 4: Every unconnected out is routed to out n.4

NOTE: input trigger threshold is +1v.  

## Parking
### Set of unconnected inputs and outputs just to park unused cables

![parking](https://user-images.githubusercontent.com/80784296/204013230-cda01462-92c9-4013-8599-c0ba9d798ae0.JPG)

#### - INSTRUCTIONS
This module doesn't do anything. It's just a place to connect your temporarily unused cables when you don't want to forget to where they were wired.  
It can also be used to connect other modules sockets when they need to be wired to obtain some functionality.

## Shifter
### 64 selectable stages shift register
#### - DESCRIPTION
- 64 stages shift register that outputs only the selected stage controlled by knob/CV with attenuverter
- Trigger delay to adjust the 1-sample latency of VCV cables

![shifter](https://user-images.githubusercontent.com/80784296/212537155-aceff24b-4dc8-4c04-9063-fad4543c9cd6.JPG)

#### - INSTRUCTIONS
Shifter module can be useful to shift back and fotrth a sequencer output on the fly, thanks to the 64 stages register.  
Stage can be controlled via the stage knob, or the 0-10v StageCV input with its attenuverter.  
If StageCV input is not connected, the attenuverter reduces the range of the Stage knob.  
Note that the Stage knob and StageCV are added together.  
The TRIG DELAY knob can be used to delay the TRIG INPUT up to 5 samples, because of the 1sample latency of VCV cables. This can be useful when you're triggering the sequencer with the same clock of Shifter module, and the input would be sampled before the sequencer advance.  

![shifter example](https://user-images.githubusercontent.com/80784296/212531455-776e3110-78ef-4bec-a3f8-64180fe4ca53.JPG)  
[Download example](./examples/shifter%20example.vcvs?raw=true) (right-click -> save link as)

## SickoPlayer
### wav sample player

![sickoplayer](https://user-images.githubusercontent.com/80784296/212481297-755598b1-aa1e-4252-b750-42f86b337f25.JPG)

#### - DESCRIPTION
- samples and 1-cycle waveforms player
- ±24 semitones tuning and v/oct input with polyphony
- envelope generator, loop, reverse, pingpong
- different interpolation modes, anti-aliasing filter, phase-scan feature

#### - INSTRUCTIONS
Load sample using context menu or right-click in the waveform display area to access quick load menu.  

The display shows the waveform, filename, sample rate and number of channels (1-2 channels wav file are allowed).  

Mode switch allows to select if sample playback starts with a trigger or play it until a gate is high.  

When in Trig Mode the Trig-Mode switch has 3 options:  
- **SS (Start/Stop)** A trigger starts attack stage from CueStart postition, another trigger sets playback to release stage and at the end sample position is reset to cue start  
- **S (Start only)** A trigger starts attack stage from CueStart position, another trigger has no effects  
- **PP (Play/Pause)** A trigger starts attack stage from curent sample position, another trigger goes to release stage  

In any Trig-Mode a trigger on STOP input sets the playback to release stage and reset sample position to Cue Start.

Cue Start/End knobs are used to set the start of the Attack and the Release stage.

When Loop button is switched on, playback restarts from Loop Start when Loop End is reached.

REV button changes the playback direction.

PNG button enables PingPong mode:
- in TRIG mode, when loop is enabled, it automatically switches the REV button when Loop Start/End is reached and inverts playback direction without stopping playback
- in GATE mode, it automatically switches the REV button when Loop Start/End is reached when loop is enabled, or when Cue Start/End is reached, and playback direction is inverted without stopping playback

The envelope knobs can be external modulated with attenuverted CVinputs.

Tune knob with its attenuverted CVinput, can tune up or down the sample with a ±2 octave range (semitone scale).  
v/oct input accepts polyphonic cable usually combined with a polyphonic gate in when in Gate Mode.  

Master knob, with its attenuverted CVinput, sets the output volume from 0 to 200%. Limit switch is a hard clip limiter with a ±5v range.  

If sample file is mono, left out is duplicated to right out.  
EOC outputs a 1ms pulse when sample reaches CueEnd or LoopEnd if Loop is enabled (or when Start positions are reached when in reverse playback).  
EOR outputs a 1ms pulse when sample reaches the end of release stage.

NOTE: input trigger threshold is +1v.  

#### CONTEXT MENU
**Sample Slot**  
Click on "Load Sample" to open dialog. Use Clear options to unload sample from slot.  
As described before, just right-click over the waveform display area to access the quick-load menu.  

**Interpolation**  
There are 3 different interpolation algorithms, that are engaged during playback only when the sample samplerate differs from VCV working samplerate or playback speed differs from 100%.  
- 'No interpolation' can be used when sample rates match and speed is 100% constant  
- 'Linear 1' and 'Linear 2' interpolates the samples with different weighted averages  
- 'Hermite' uses a Cubic Hermite spline interpolation that usually offers a better result (default)  

**Anti-aliasing filter**  
Anti-aliasing filter is made up with 2x oversampling and a 20khz lowpass filter.  

**Crossfade length**  
Crossfade can be set from 0 to 50ms and is engaged when the sample skips from LoopEnd to LoopStart or from CueEnd to CueStart when in Gate Mode.

**Polyphonic Outs**  
When this option is enabled the outs reflects v/oct input polyphony. Otherwise polyphonic outputs are mixed in one monophonic out.

**Phase scan**  
This feature automatically sets Cue and Loop Start/Stop positions at zero crossing points to avoid loop clicks and pops eventually in combination with proper crossfade length.  
Be sure to disable it when using one-cycle waveforms.  

#### PRESETS
There are some factory presets stored in the context menu.  
Loading a factory preset automatically clears the sample from memory, pay attention.

#### USING ONE-CYCLE WAVEFORMS
One-cycle waveforms can be used in GATE mode with LOOP mode enabled.  
Be sure to disable PhaseScan functionality, adjust Cue and Loop START to 0% and Cue/Loop END to 100%, or load relative factory preset before loading the sample.

## Switcher / SwitcherSt
### 2>1 switch, 1>2 router, 2 signal swapper, mute, flip flop, toggle gate
#### - DESCRIPTION
- Signal switch (2 inputs, 1 output)
- Signal router (1 input, 2 outputs)
- Signal swapper (2 inputs, 2 outputs)
- Mute (1 input, 1 output)
- Flip flop
- Toggle gate
- Function type autodetection (switch, route, swap, mute, flipflop, toggle gate)
- Adjustable time crossfade between switched/routed/swapped signals

![switcher](https://user-images.githubusercontent.com/80784296/201516861-d3d2ab1b-7036-4355-b2ef-e4c5681fb432.JPG)

#### - INSTRUCTIONS
Switcher or SwitcherSt (used for stereo signals) are multifunction modules that can be used as follows. The versatility of the module is offered thanks to the automatic detection of the function type.

**TOGGLE/GATE modes**  
When the MODE switch is in 'TOGGLE' position functions are activated by triggers in a toggle style.  
When in 'GATE' position functions are gate sensitive, so they stay active until 'T/G' input receives a high gate.  
Functions will be activated by Trigger/Gates with voltages above +1v.

**Function types**  
The function type is automatically detected depending on connected cables.  

Switch: connect 2 inputs and 1 output  
Router: connect 1 input and 2 outputs  
Swapper: connect both two inputs and two outputs  
Mute: connect 1 input and 1 output  
FlipFlop: connect 2 outputs only  
ToggleGate: connect 1 output only ('TOGGLE' mode only)  

A trigger on RST input will reset the toggle to its default state.  

**Defaults**  
Default states depend on function type and which input or output sockets are connected.  

Switch: if OUT1 is connected, the default signal will be the IN1 input. If OUT2 is connected instead, the default signal will be the IN2 input  
Router: if IN1 is connected, the signal will be routed to OUT1 output by default. If IN2 is connected, the default destination will be OUT2 output  
Swapper: the default is always IN1>OUT1 and IN2>OUT2. 
Mute: if IN1 and OUT1 (or IN2 and OUT2) the defualt is mute. If IN1 and OUT2 (or IN2 and OUT1) are connected the default is signal unmute  
FlipFlop: default is always OUT1  
ToggleGate: if OUT1 is connected the default is a LOW GATE. If OUT2 is connected instead, the default is a HIGH GATE  

**Leds**  
Green leds close to the in/out sockets show which input signal is switched or to which output destination is routed.  
When used as a swapper the OUT1 led on shows that signals are normally routed, otherwise the OUT2 led on shows when signals are swapped.  

**Fader**  
Fader knob sets the crossfade time (up to 10s) between the switched/routed/swapped signals.  
CV input is added to Fade knob value and the sum will be clamped in the range of 0-10v.  

**NOTES**  
- In FlipFLop and ToggleGate function types the output will consist in a 'fixed' AR envelope
- When a fade time is set, the module will act as an envelope generator, so if a function activation is detected during a fade, the function will restart immediately (not like a function generator)
- On SwitcherSt module the function type is detected on Left channel sockets, so don't use Right channels without Left ones.


## Toggler / Toggler Compact
### Stereo signal toggle switch router, with VCA and ASR envelope generator, in regular and compact form factor
#### - DESCRIPTION
- Toggled VCA with builtin ASR envelope generator
- Toggled ASR envelope generator
- mute/unmute CVs or mono/stereo AUDIO signals according to an ASR envelope activated by a Gate or Toggle Triggers

![toggler](https://user-images.githubusercontent.com/80784296/211222030-1a5b4e86-eccd-4e4f-ae56-65efd100e336.JPG)

#### - INSTRUCTIONS
**TOGGLE MODE**

On receiving a trigger on TRIG input, it will send the L+(R) inputs to L+(R) outputs and set the GATE output to high. On next trigger it will interrupt L+(R) outputs and set the GATE output to low.

Attack, Sustain and Release knobs set the envelope of the routed signal.

A, S, R CVinputs are added to respective knob values, Toggler module has attenuverters.

If L or (R) inputs are not connected, L and (R) outputs will provide just the envelope, so a mono signal can be connected to L input to route it to L output and nothing connected to (R) input to have the envelope on (R) output.

A trigger on RESET input will reset the toggle state.

**GATE MODE**

The same of toggle mode, but the signals will be routed only while GATE input is in a high state.

NOTE: If a new GATE or Toggle TRIGGER is detected on Attack or Release phases, the envelope ramp will immediately restart from the reached point, as a regular envelope generator and not like a function generator.  
NOTE2: input trigger and gate threshold is +1v.

**SPECIAL BEHAVIORS**

If Attack is set to 0 (and release is set greater than 0) and a new GATE or Toggle TRIGGER is detected before Release phase has ended, the next Release phase will start from the previous reached release point.

If Release is set to 0 (and attack is set greater than 0) and a new GATE or Toggle TRIGGER is detected before Attack phase has ended, the next Attack phase will start from the previous reached Attack point.

These behaviors are more understandable connecting a scope on the output.


## CREDITS
The Component Library graphics for these modules are copyright © VCV and licensed under [CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/)  

Thanks to [Squinkylabs](https://github.com/squinkylabs) and [Firo Lightfog](https://github.com/firolightfog) for help and testing  
Thanks to [Omri Cohen](https://omricohen-music.com/) for support  


