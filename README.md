# SickoCV v2.4.1
VCV Rack plugin modules

![SickoCV modules 2 4 1](https://user-images.githubusercontent.com/80784296/204008598-a1003f9b-6d89-41f8-a7ce-5b9f500a90cc.JPG)

## Blender
### Stereo crossfade mixer with double modulation
#### - Description:
'blender' is a crossfade mixer of mono or stereo signals.  
It can be used either with cv signals or audio sources.  
Mix can be modulated by uni/bipolar signals.  
Modulation can be further modulated by another signal.  
Audio rate modulations are allowed.

![blender](https://user-images.githubusercontent.com/80784296/201516763-00c7192d-f881-4b14-9c23-68c8be2a90d3.JPG)

#### - Usage:
Connect CVs or audio sources to IN1 and IN2, mono or stereo signals can be independently used.  
PHASE switches invert the sign of input signals.  
MIX knob sets the crossfade level of the inputs.

**MIX MOD section**  
Connecting MIX MOD CV input enables mix modulation. ATNV knob attenuverts CV input.  
CV input range is usually unipolar 0-10v. RNG switch in 'bipolar' position adds +5v to CV input, so audio signals can be used for modulation.    
Modulation is added to the MIX knob.

**ATNV MD section**  
ATNV MD can be used to add modulation to the ATNV knob in MIXMOD section, the rules are the same.

## Blender8
### 8 single crossfade mixers with modulation
#### - Description:
'blender8' is a set of 8 crossfade mixers of two signals.  
As the previous one it can be used either with cv signals or audio sources.  
Mix can be modulated by uni/bipolar signals.  
Audio rate modulations are allowed.

![blender8](https://user-images.githubusercontent.com/80784296/201516772-12dac17b-f8a0-4f82-946a-da8b7d254b09.JPG)

#### - Usage:
'blender8' provides 8 mono crossfade mixers and differs from 'blender' module in the following things.  
Only the IN2 input signal can be phase inverted.  
If a CV input is connected for modulation, CV sets the mix percentage and the MIX knob becomes the CV attenuverter.

## bToggler / bToggler Compact
### Buffered stereo signal toggle switch router, with VCA and ASR envelope generator, in regular and compact form factor
#### - Description:
- Buffered Toggled VCA with builtin ASR envelope generator
- Buffered Toggled ASR envelope generator
- Buffer mute/unmute CVs or mono/stereo AUDIO signals according to an ASR envelope activated by Toggle Triggers

![btoggler](https://user-images.githubusercontent.com/80784296/201516786-81b923a6-4d9d-4c6f-8c1c-74e43f9e7e9c.JPG)

#### - Usage:
Connect a clock source.

When ARM input is triggered (arm on), the L+(R) inputs will start to be routed to L+(R) outputs on next clock detection (according to ASR envelope values) and GATE output will provide a high state.

Then, with another ARM triggering (arm off) the routing will stop on next clock detection and GATE output will go low.

If ARM is triggered again before clock detection it will abort arming (unarm).

Attack, Sustain and Release knobs set the envelope of the routed signals.

A, S, R CVinputs are added to respective knob values.

If L or (R) inputs are not connected, L and (R) outputs will provide just the envelope, so a mono signal can be connected to L input to route it to L output and nothing connected to (R) input to have the envelope on (R) output.

A trigger on RESET input will reset the toggle state.

NOTE: input trigger threshold is +1v.


## bToggler8
### 8 buffered toggle switch signal router
#### - Description:
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

#### - Usage:
Connect a clock source.

When ARM input is triggered (arm on) the IN input will start to be routed to OUT on next clock detection and GATE output will provide a high state.

Then, with another ARM triggering (arm off) the routing will stop on next clock detection and GATE output will go low.

FADE knob (up to 50ms) can be used to avoid attack or release clicks when audio signals are connected to IN input.

If ARM is triggered again before clock detection it will abort arming (unarm).

Triggering RESET input will immediately stop the routing.

Triggering RESETALL input will immediately stop all the 8 routings.

NOTE: input trigger threshold is +1v.  
 
Here below is one example of bToggler8 usage. When buttons are pressed on PULSES module, incoming triggers from the sequencer are routed to drum modules only when the first step of the sequencer is reached. If buttons are pressed again, the routing will stop on next first step of the sequencer.

![btoggler8 example](https://user-images.githubusercontent.com/80784296/201529085-8dca2d3d-ef9d-42f3-94b7-4d3a1c6da057.JPG)


## bToggler8+
### 8 buffered toggle switch router, plus warnings to use with led midi controllers
#### - Description:
'bToggler8+' is almost the same of the previous one, but it has a further feature (WRN outs) to be used with programmable led midi controllers to have a visual feedback on the controller.

Some midi controllers can light up or turn off their button leds by receiving the same commands they send.  
Taking advantage of this functionality, connect the WRN outs to a "GATE>MIDI" module connected to the same controller of the ARM inputs.  
So when pressing buttons on controller, 'bToggler8+' will actually play/stop the sequencers or audio, and simultaneously give a visual feedback on the controller.

![btoggler8plus](https://user-images.githubusercontent.com/80784296/201516811-40c75bb5-84d2-411b-b9ed-fa876e178258.JPG)

#### - Usage:
The same of the previous one, plus:

When 'armed on' or 'armed off', the WRN (warning) output will provide a sequence of pulses until next clock is detected.  
Then it will act as the OUT output (the routed signal) if the FADE knob is set to 0ms, else it will act as the GATE output (high gate).  
This is because if 'bToggler8+' is receiving signals from sequencers, the FADE knob will be set to 0 and the led will light up the same as sequencers trigs.  
Otherwise, if fade knob is set different from 0, it is supposed that an audio signal is routed, so you'll see a fixed led light on the controller.

WA and WR knobs set the attack (arm on) and release (arm off) pulserate up to 200 ms of the warning pulses. These are two independent settings because you would like to notice if the routing is going to start or stop.

If WA or WR are set to 0ms, WRN will output a low gate during warning time and if set to to max (200ms) it will output a high gate.  
As to say: if WA is set to 0 and WR is set to max(200), WRN output will act like the GATE output.  
Otherwise if WA is set to max(200) and WR is set to 0, WRN output will act as simple toggle switch with no buffer feature.

In the context menu there is the option to invert WRN output when used with triggers. It can be useful when used with a Led Controller, so when a channel is 'toggled on', leds stay turned on, and when a trig is received they are turned off for 100ms.  

NOTE: input trigger threshold is +1v.  

Here below is one example of bToggler+ usage. The MIDI>GATE module is connected to a programmable Led Midi controller and receives buttonpresses from it. The GATE>MIDI module send back triggers incoming from the sequencer to the controller, turning on and off the corresponding led buttons only when triggers are actually routed to drum modules. Routing rules are the same of previous example.

![btoggler8plus example](https://user-images.githubusercontent.com/80784296/201529122-29f06a3f-ccc4-4901-973f-80cde6259a76.JPG)

## Calcs
### Calculates sums, differences, multiplications, divisions and averages of 3 CV inputs

![calcs](https://user-images.githubusercontent.com/80784296/201516821-8ea683bd-db11-4687-971d-67bef380b81c.JPG)

#### - Usage:
A, B and C are the inputs. The output tables provide simple math calculations and averages between two inputs or the average of all of them.

U/B (Unipolar/Bipolar) switch clamps the outputs to 0/10V or -5/+5v.

## Drummer
### Accent and choke utility for drum modules lacking these features

![drummer](https://user-images.githubusercontent.com/80784296/201516831-bff1df4b-b7e1-4065-b486-7501d6c4cde7.JPG)

#### - Usage:
Drummer module can handle two drum sounds with separate standard and accent volume levels set by respective knobs.  
Connect the IN input to a drum-like audio source or sample player, and OUT output to the mixer.  
Connect the TRIG input to the same module that feeds the drum module, it can be a sequencer or every other pulse generation module.  
Connect the ACC input to the module which generates the accents, it can be the sequencer or every other suitable module.  
When ACC is triggered at the same time as the TRIG input, Drummer module will output the Accent Level set by "Accent Level knob" instead of the one set by "Standard Level Knob".  
Input triggers threshold is +1v.  
Each knob range is from 0 to 200% of the incoming IN level.  
LIMIT switch limits the output from -5v/+5v.  
CHOKE switch mutes one output when the other is triggered (for example when used with open/closed hihat sounds).  
OUT outputs are normalled, so if both outputs are connected, audio is routed separately, otherwise the two inputs are mixed in the only connected output.  
NOTE: In CHOKE mode, if both TRIG inputs are triggered at the same time, the upper section (#1) will have the priority and the lower one will be ignored.

Example of Drummer module usage:

![drummer example](https://user-images.githubusercontent.com/80784296/201531638-62c9e364-9de9-4e98-8d9b-d44d351942f8.JPG)

## Drummer4
### 4 channel accent utility for drum modules lacking this feature

![drummer4](https://user-images.githubusercontent.com/80784296/201516839-54364ebe-d3cc-4a4e-9c32-d85e2ec2653b.JPG)

#### - Usage:
This module is almost the same of the previous one. It supports up to 4 channel and it manages only accent levels, there is no choking feature.  
Outputs are normalled to the next ones. For example if you connect only out #2 and #4, out #1 and #3 will be respectively mixed with those ones, if you connect only out #4, this socket will output all the channels.

Example of Drummer4 module usage:

![drummer4 example](https://user-images.githubusercontent.com/80784296/201531658-fe7ee4d6-5d2f-415e-982c-84e0f66df989.JPG)

## Parking
### Set of unconnected inputs and outputs just to park unused cables

![parking](https://user-images.githubusercontent.com/80784296/204013230-cda01462-92c9-4013-8599-c0ba9d798ae0.JPG)

#### - Usage:
This module doesn't do anything. It's just a place to connect your temporarily unused cables when you don't want to forget to where they were wired.  
It can also be used to connect other modules sockets when they need to be wired to obtain some functionality.

## Shifter
### 64 selectable stages shift register
#### - Description:
- 64 stages shift register that outputs only the selected stage controlled by knob/CV with attenuverter
- Trigger delay to adjust the 1-sample latency of VCV cables

![shifter](https://user-images.githubusercontent.com/80784296/204009935-efd147d4-2d28-4b72-a607-af41e7c8e894.JPG)

#### - Usage:
Shifter module can be useful to shift back and fotrth a sequencer output on the fly, thanks to the 64 stages register.  
Stage can be controlled via the stage knob, or the 0-10v StageCV input with its attenuverter.  
If StageCV input is not connected, the attenuverter reduces the range of the Stage knob.  
Note that the Stage knob and StageCV are added together.  
The TRIG DELAY knob can be used to delay the TRIG INPUT up to 5 samples, because of the 1sample latency of VCV cables. This can be useful when you're triggering the sequencer with the same clock of Shifter module, and the input would be sampled before the sequencer advance.  

## Switcher / SwitcherSt
### 2>1 switch, 1>2 router, 2 signal swapper, mute, flip flop, toggle gate
#### - Description:
- Function type (switch, route, swap, mute, flipflop, toggle gate) autodetection
- Signal switch (2 inputs, 1 output)
- Signal router (1 input, 2 outputs)
- Signal swapper (2 inputs, 2 outputs)
- Mute (1 input, 1 output)
- Flip flop
- Toggle gate
- Adjustable time crossfade between switched/routed/swapped signals

![switcher](https://user-images.githubusercontent.com/80784296/201516861-d3d2ab1b-7036-4355-b2ef-e4c5681fb432.JPG)

#### - Usage:
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
#### - Description:
- Toggled VCA with builtin ASR envelope generator
- Toggled ASR envelope generator
- mute/unmute CVs or mono/stereo AUDIO signals according to an ASR envelope activated by a Gate or Toggle Triggers

![toggler](https://user-images.githubusercontent.com/80784296/201516866-3ca90766-503c-435d-a560-ba0f5c02deff.JPG)

#### - Usage:
**TOGGLE MODE**

On receiving a trigger on TRIG input, it will send the L+(R) inputs to L+(R) outputs and set the GATE output to high. On next trigger it will interrupt L+(R) outputs and set the GATE output to low.

Attack, Sustain and Release knobs set the envelope of the routed signal.

A, S, R CVinputs are added to respective knob values.

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


## Credits
The Component Library graphics for these modules are copyright © VCV and licensed under [CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/)

