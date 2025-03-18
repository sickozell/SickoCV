# SickoCV v2.7.1
VCV Rack plugin modules

![SickoCV modules 2 7 1](https://github.com/user-attachments/assets/11fec174-10a2-4982-ad85-f62c1d0f1897)

## table of contents
- [Common modules behavior](#common-modules-behavior)
- [adder8](#adder8)
- [bGates](#bgates)
- [blender](#blender)
- [blender8](#blender8)
- [bToggler / bToggler Compact](#btoggler--btoggler-compact)
- [bToggler8](#btoggler8)
- [bToggler8+](#btoggler8-1)
- [calcs](#calcs)
- [clocker / clocker2](#clocker--clocker2)
- [CV router / CV switcher](#cvrouter--cvswitcher)
- [drummer / drummer4 / drummer4+](#drummer--drummer4--drummer4)
- [drumPlayer / drumPlayer+ / drumPlayerXtra](#drumplayer--drumplayer--drumplayerxtra)
- [enver](#enver)
- [holder / holder Compact / holder8](#holder--holder-compact--holder8)
- [keySampler](#keysampler)
- [modulator / modulator7 / modulator7 Compact](#modulator--modulator7--modulator7-compact)
- [multiRouter / multiSwitcher](#multirouter--multiswitcher)
- [parking](#parking)
- [polyMuter8 / polyMuter8+ / polyMuter16 / polyMuter16+](#polymuter8--polymuter8--polymuter16--polymuter16)
- [randLoops / randLoops8](#randLoops--randLoops8)
- [shifter](#shifter)
- [sickoAmp](#sickoamp)
- [sickoCrosser / sickoCrosser4](#sickocrosser--sickocrosser4)
- [sickoLooper1 / sickoLooperX / sickoLooper3 / sickoLooper5](#sickolooper1--sickolooperx--sickolooper3--sickolooper5)
- [sickoPlayer](#sickoplayer)
- [sickoQuant / sickoQuant4](#sickoquant--sickoquant4)
- [sickoSampler](#sickosampler)
- [sickoSampler2](#sickosampler2)
- [simpleSeq4](#simpleseq4)
- [slewer](#slewer)
- [stepSeq / stepSeq+ / trigSeq / trigSeq+ / trigSeq8x](#stepSeq--stepSeq--trigSeq--trigSeq--trigSeq8x)
- [switcher / switcherSt / switcher8](#switcher--switcherst--switcher8)
- [toggler / toggler Compact](#toggler--toggler-compact)
- [wavetabler](#wavetabler)
- [Credits](#credits)

## **Consider donating**  
The work necessary to develop these modules required many hours of work and many sleepless nights.  
Sickozell plugin is and will always remain free, but if you find it useful, consider donating even just a coffee by following this [payPal](https://paypal.me/sickozell) link.  
Thanks.

## Common modules behavior
- Triggers and gates threshold is +1v
- Every time-related knob set full anticlockwise and displaying 1ms on the tooltip is actually considered 0ms

## adder8
### 8 Adder and subtractor
#### - DESCRIPTION
'adder8' is inspired by hardware precision adder modules. It adds, ignore or subtracts fixed voltages or CVS to outputs. 

![adder8](https://github.com/sickozell/SickoCV/assets/80784296/d79eddde-fa02-470c-bb26-6571553ef1c9)

#### - INSTRUCTIONS
On the first row a fixed voltage set by VLT/ATNV knob is added, ignored or subtracted, depending on the -0+ switch, to the corresponding output.  
If the output is not connected, the result voltage is summed to the next row, and with the same rules until a connected output is found.  
If a CV input is connected, the VLT/ATNV knob acts as an attenuverter, then the CV voltage will be added or subtracted in the same previous way.  
When an output is connected, the starting voltage of the next row is reset to 0v just like the first row does.  
The MODE switches force the "-0+" switches to be as: "subtract/ignore", "subtract/ignore/add" or "ignore/add".  

#### RIGHT-CLICK MENU
- **Stop Adding on Out Cable** (ticked by default): as mentioned above, the starting voltage is reset to 0v in the next row only when an out cable is detected. Unticking this option the voltage won't be reset
- **Volt Knob Default**: with this option the default initialization value of the VLT/ATNV knob can be changed to 0v, +1v or +10v.  
This unconventional feature lets the user to choose the default knob value depending on the main usage of Adder8:  
if it's used as a fixed pitch adder (without input CV connection) maybe it's useful to have the default value set to +1v, so if the knob position has been changed to detune, it can be quickly restored to add (or subtract) exactly 1 octave in pitch;  
otherwise, if the knob is used as attenuverter with a CV input connected, it can be set to 0v as usual or to +10v to quickly get the full CV voltage.
- **Attenuator**: this converts attenuverter knobs into attenuators when CV inputs are connected
- **Reset All Knobs to Default**: this resets all knobs value to selected default setting

[back to top](#table-of-contents)

## bGates
### 8 buffered gates and triggers
#### - DESCRIPTION
'bGates' provides 8 buffered gates or triggers on 8 individual clock inputs.  

![bgates](https://github.com/sickozell/SickoCV/assets/80784296/48b78c5b-c5cf-4673-8cc6-6a705cd179a7)

#### - INSTRUCTIONS
Connect a clock source. Clock inputs are normalled to previous ones.

When ARM input is triggered (arm on) the GATE output will be set to high state on next clock detection and 1ms trigger will be given by TRG out.

Then, with another ARM triggering (arm off) GATE output will go low and another 1ms trigger will be given by TRG out.

If ARM is triggered again before clock detection it will abort arming (unarm).

Triggering RST input will immediately set the GATE out state to low and unarm it. If the GATE out state was HIGH a 1ms trigger is given by TRG out

Pressing RSTALL button or triggering RESETALL input will immediately set all the 8 GATE outs to low and unarm them.

#### **Right-click Menu**
- **Initialize On Start**: discards previous module state on VCV restart
- **Disable Unarm**: this disables unarm feature

[back to top](#table-of-contents)

## blender
### Polyphonic stereo crossfade mixer with double modulation
#### - DESCRIPTION
'blender' is a crossfade mixer of mono or stereo signals.  
It can be used either with cv signals or audio sources.  
Mix can be modulated by uni/bipolar signals.  
Modulation can be further modulated by another signal.  
Audio rate modulations are allowed.

![blender](https://github.com/sickozell/SickoCV/assets/80784296/414dac6a-32f4-490b-ad70-8e867d4a8b41)

#### - INSTRUCTIONS
Connect CVs or audio sources to IN1 and IN2, mono or stereo signals can be independently used. Polyphonic inputs are allowed and are left/right independent, but accordingly to number of channels of IN1 input.  
PHASE switches invert the sign of input signals.  
MIX knob sets the crossfade level of the inputs.  
Inputs volume can be adjusted via two attenuators.  
Master volume can be amplified up to 200%, a hard clip ±5v switch is present.  
Output replicates input polyphony, but deticking in the right-click menu 'Polyphonic outs' option will mix channels into monophonic outs.

**MOD section**  
Connecting MIX MOD CV input enables mix modulation. ATNV knob attenuverts CV input.  
CV input range is usually unipolar 0-10v. RNG switch in 'bipolar' position adds +5v to CV input, so audio signals can be used for modulation.    
Modulation is added to the MIX knob.

**MOD2 section**  
MOD2 can be used to add modulation to the MOD attenuverter knob in MOD section, the rules are the same.

[back to top](#table-of-contents)

## blender8
### 8 single crossfade mixers with modulation
#### - DESCRIPTION
'blender8' is a set of 8 crossfade mixers of two signals.  
As the previous one it can be used either with cv signals or audio sources.  
Mix can be modulated by uni/bipolar signals.  
Audio rate modulations are allowed.

![blender8](https://github.com/sickozell/SickoCV/assets/80784296/ef3726db-b087-4f00-ad9e-b8b43c6620f0)

#### - INSTRUCTIONS
'blender8' provides 8 mono crossfade mixers and differs from 'blender' module in the following things.  
Only the IN2 input signal can be phase inverted.  
If a CV input is connected for modulation, CV sets the mix percentage and the MIX knob becomes the CV attenuverter.

[back to top](#table-of-contents)

## bToggler / bToggler Compact
### Buffered stereo signal toggle switch router, with VCA and ASR envelope generator, in regular and compact form factor
#### - DESCRIPTION
- Buffered Toggled VCA with builtin ASR envelope generator
- Buffered Toggled ASR envelope generator
- Buffer mute/unmute CVs or mono/stereo AUDIO signals according to an ASR envelope activated by Toggle Triggers

![btoggler](https://github.com/sickozell/SickoCV/assets/80784296/8464aaa5-f524-4fda-ae66-cca2251db96d)

#### - INSTRUCTIONS
Connect a clock source.

When ARM input is triggered (arm on), the L+(R) inputs will start to be routed to L+(R) outputs on next clock detection (according to ASR envelope values) and GATE output will provide a high state.

Then, with another ARM triggering (arm off) the routing will stop on next clock detection and GATE output will go low.

If ARM is triggered again before clock detection it will abort arming (unarm).

Attack, Sustain and Release knobs set the envelope of the routed signals.

A, S, R CVinputs are added to respective knob values, bToggler module has attenuverters.

If L or (R) inputs are not connected, relative outputs will provide just the envelope, so a mono signal can be connected to L input to route it to L output and nothing connected to (R) input to have the envelope on (R) output.

A trigger on RESET input will reset the toggle state.

Polyphony on L/(R) inputs is replicated on outs.   

#### Right-click Menu
- **Initialize On Start**: discards previous module state on VCV restart
- **Disable Unarm**: this disables unarm feature
- **Trigger on Gate Out**: this option substitutes Gate Output with a 1ms trigger whenever a clock is detected when armed

[back to top](#table-of-contents)

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

![btoggler8](https://github.com/sickozell/SickoCV/assets/80784296/afab0b47-fc81-4e51-a6b3-4982eb4bc1bc)

#### - INSTRUCTIONS
Connect a clock source.

When ARM input is triggered (arm on) the IN input will start to be routed to OUT on next clock detection and GATE output will provide a high state.

Then, with another ARM triggering (arm off) the routing will stop on next clock detection and GATE output will go low.

FADE knob up to 10s can be used to avoid attack or release clicks when audio signals are connected to IN input.  
Be sure to set Fade knob to minimum (1ms) when inputs are feeded by triggers.  

If ARM is triggered again before clock detection it will abort arming (unarm).

Triggering RESET input will immediately stop the routing.

Triggering RESETALL input will immediately stop all the 8 routings.

#### **Right-click Menu**
- **Initialize On Start**: discards previous module state on VCV restart
- **Disable Unarm**: this disables unarm feature
 
Here below is one example of bToggler8 usage. When buttons are pressed on PULSES module, incoming triggers from the sequencer are routed to drum modules only when the first step of the sequencer is reached. If buttons are pressed again, the routing will stop on next first step of the sequencer.

![bToggler8 example](https://user-images.githubusercontent.com/80784296/204083532-db145211-1f61-45cd-9c4d-572fc243d7d3.JPG)  
[Download example](./examples/bToggler8%20example.vcvs?raw=true) (right-click -> save link as)

[back to top](#table-of-contents)

## bToggler8+
### 8 buffered toggle switch router, plus warnings to use with led midi controllers
#### - DESCRIPTION
'bToggler8+' is almost the same of the previous one, but it has a further feature (WRN outs) to be used with programmable led midi controllers to have a visual feedback on the controller.

Some midi controllers can light up or turn off their button leds by receiving the same commands they send.  
Taking advantage of this functionality, connect the WRN outs to a "GATE>MIDI" module connected to the same controller of the ARM inputs.  
So when pressing buttons on controller, 'bToggler8+' will actually play/stop the sequencers or audio, and simultaneously give a visual feedback on the controller.

![btoggler8plus](https://github.com/sickozell/SickoCV/assets/80784296/6f2d7c77-c8d1-40b4-ac0e-8e5dc9296532)

#### - INSTRUCTIONS
The same of the previous one, plus following features.

When 'armed on' or 'armed off', the WRN (warning) output will provide a sequence of pulses until next clock is detected.  
Then it will act as the OUT output (the routed signal) if the FADE knob is set to 1ms, else it will act as the GATE output (high gate).  
So, if 'bToggler8+' is receiving triggers from a sequencer, the FADE knob will be set to 1ms and the led will light up the same as sequencers trigs.  
Otherwise, if fade knob is set different from 1ms, it is supposed that an audio signal is routed, so a fixed led light on the controller will be seen.

WA and WR knobs set the attack (arm on) and release (arm off) pulserate up to 200 ms of the warning pulses. These are two independent settings because it can be helpful to notice if the routing is going to start or stop.

If WA or WR are set to 0ms, WRN will output a low gate during warning time and if set to to max (200ms) it will output a high gate.  
As to say: if WA is set to 0 and WR is set to max(200), WRN output will act like the GATE output.  
Otherwise if WA is set to max(200) and WR is set to 0, WRN output will act as simple toggle switch with no buffer feature.

#### Right-click Menu
- **Initialize On Start**: discards previous module state on VCV restart
- **Disable Unarm**: this disables unarm feature
- **WRN Inversion** (trigs only): inverts WRN output behavior when used with triggers. It can be useful when INs are feeded by sequencers trigs and WRN Outs connected to a led midi controller. With this option enabled when there's no routing leds will stay off, when routing leds will stay on and whenn a trig is received led will be turned off for 100ms.

Here below is one example of bToggler+ usage. The MIDI>GATE module is connected to a programmable Led Midi controller and receives buttonpresses from it. The GATE>MIDI module send back triggers incoming from the sequencer to the controller, turning on and off the corresponding led buttons only when triggers are actually routed to drum modules. Routing rules are the same of previous example.

![bToggler8plus example](https://user-images.githubusercontent.com/80784296/204083544-34ecf3b0-0d12-4965-bd72-f3bb85339551.JPG)  
[Download example](./examples/bToggler8plus%20example.vcvs?raw=true) (right-click -> save link as)

[back to top](#table-of-contents)

## calcs
### Calculates sums, differences, multiplications, divisions and averages of 3 CV inputs

![calcs](https://github.com/sickozell/SickoCV/assets/80784296/10721cad-2ecc-4d2f-be7e-de84832d4eae)

#### - INSTRUCTIONS
A, B and C are the inputs. The output tables provide simple math calculations and averages between two inputs or the average of all of them.

U/B (Unipolar/Bipolar) switch clamps the outputs to 0/10V or ±5v.

[back to top](#table-of-contents)

## clocker / clocker2
### Clock generator with dividers/multipliers and audio metronome

#### - DESCRIPTION
Clocker is a high precision clock generator and modulator with 4 dividers/multipliers with swing feature, time signatures and integrated audio click.  
Clocker2 has 6 dividers/multipliers, but lacks metronome and audio click feature.

![clocker](https://github.com/sickozell/SickoCV/assets/80784296/94f12b57-e794-4083-96b6-00a6a9e7baec)

#### - INSTRUCTIONS

The BPM knob sets the clock speed from 30 to 300 bpm.  
An external clock can be connected on the EXT input.  
The RUN button or a trig on its input starts or stops the clock.  
PW (pulse width) knob adjusts the length of the gate in its high state (see 'Right-click menu' paragraph for Swing instructions).  
Clock and metronome can be reset with RST button or a trig on its input.  

There are 4 clock dividers/multipliers up to 256x each with theirs PW control. Right click on the display to quick select the desired division/multiplication.  

The metronome setting is controlled by the METER knob or with a right click on the time signature display.  
Audio click is activated with CLICK button and volume can be adjusted with the knob from 0 to 200%.  
When clock is runniing BEAT and BAR outputs are always active and give a 1ms trigger.

To get best clock precision the algorithm used may alter the clock lengths, according to working vcv samplerate and BPM setting.  
There will therefore be clocks of non-fixed length, but which will guarantee the exact number of BPM within a minute.

**External clock**  
In the context menu set the PPQN resolution according to your master clock device or vcv module.  
If external clock is generated by other vcv clock modules the default 1 PPQN (1 quarter note) can achieve good results.  
Although the bpm display does not show the decimal digit to avoid distraction, fractional bpm can be used.  
In the context menu it is good to set the 'On Run/Reset Bar' option ticked to be sure to get the first incoming external clock pulse, after a clock stop or RUN button toggling.

Synchronization of clocker to external gear as master can be usually done by feeding Ext input with an analog clock (audio stream). Midi clock connections may result unstable and will loose sync after a while.  
Connect the clock output of the external master device to one audio input of your soundcard and route it via VCV AUDIO module to Ext input of clocker. Be sure PPQN settings match either on clocker and the external device.

#### Right-click Menu

- **Trig/Swing on Div**  
With this option enabled the selected divider/multiplier outputs a 1ms trigger instead of gate.  
A little red led is turned on near the PW knob. This knob will control the swing amount instead of pulse width, but only for clock multiplications.  
Swing control at 0% means no swing, so every pulse has equal timing. Increasing swing ratio it delays the even pulses by its percentage until 100% that means the even pulses fall on the next odd ones.  
Please note that every clock timing (beat detection) resets the odd pulses, so every pulse that is a beat will be an odd one.
This is beacause it has to match metronome and don't mess when odd clock divisions are selected.

- **External Clock**  
**Resolution**: This option sets the resolution of incoming external pulse clocks expressed in PPQN. It can be set to 1, 2, 4, 8, 12, 16, 24 PPQN.  

- **Click Presets**  
There are 3 predefined types of audio clicks, each one with beat and bar sample.  

- **Load BEAT click / Load BAR click**  
Audio clicks can be customized loading wav sample using "Load BEAT" and "Load BAR" options.  

- **Beat pulses also on Bar**  
When ticked, BAR pulses on the BAR output are duplicated on the BEAT output.  

- **On Run**  
"Reset Bar" resets metronome when the Run Button is switched on.  
"Pulse to RST out" sends a reset pulse to the Reset output when the Run Button is switched on.

- **On Stop**  
This submenu is the same as the previous one, but when the Run Button is switched off.  
Please note that "On Stop/Reset bar" option sets the clock gate output to low if clocker is currently sending a high gate.  

[back to top](#table-of-contents)

## CvRouter / CvSwitcher
### 1>2 and 2>1 voltage controlled switch  

![cvRouter cvSwitcher](https://github.com/sickozell/SickoCV/assets/80784296/2edd2ce2-fe79-4533-90d6-1e4775348ac0)

#### - INSTRUCTIONS
With the cvRouter the IN signal will be routed to OUT1 or OUT2 if the CV input voltage is lower or higher than the voltage set by the "THR" threshold knob.  
With the cvSwitcher the OUT will receive the signal from IN1 or IN2 if the voltage of the CV input is lower or higher than the voltage set by the threshold "THR" knob.

The FADE knob with its added CV input, will crossfade up to 10s the INs or OUTs. 

The default value of the "THR" knob is +1v.

[back to top](#table-of-contents)

## drummer / drummer4 / drummer4+
### Accent and choke utility for drum modules lacking these features

![drummer](https://github.com/sickozell/SickoCV/assets/80784296/1fdfb704-f5e0-400f-8095-e20a8c51a7d3)

#### - INSTRUCTIONS
Drummer and Drummer4/Drummer4+ module can handle 2 or 4 drum sounds with separate standard and accent volume levels set by respective knobs.  

Connect the IN input to a drum-like audio source or sample player, and OUT output to the mixer.  
Connect the TRIG input to the same module that feeds the drum module, it can be a sequencer or every other pulse generation module.  
Connect the ACC input to the module which generates the accents, it can be the sequencer or every other suitable module.  
When ACC is triggered at the same time as the TRIG input, Drummer module will output the Accent Level set by "Accent Level knob" instead of the one set by "Standard Level Knob".  

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
If one slot OUT is not connected, the audio signal will be added to the next one. For example if connecting only out #2 and #4, out #1 and #3 will be respectively mixed with those ones, if connecting only out #4, this socket will output all the channels.

Example of Drummer4 module usage:

![drummer4 example](https://user-images.githubusercontent.com/80784296/212531441-575f9b49-dee2-47ca-a82d-0861a10145e5.JPG)  
[Download example](./examples/drummer4%20example.vcvs?raw=true) (right-click -> save link as)

- **Drummer4+ note:**  
Drummer4+ it's the same of Drummer4. It only adds attenuverted CV inputs to parameter knobs.

[back to top](#table-of-contents)

## drumPlayer / drumPlayer+ / drumPlayerXtra
### 4 channel Drum Sample Player with accent and choke functionality

![drumplayer](https://github.com/sickozell/SickoCV/assets/80784296/f7691957-dd29-48f3-b26d-7f280feb806b)

#### INSTRUCTIONS  
Load wav samples in the slots using general or slot right-click menu.  

When TRIG input is triggered the sample will be played at the volume percentage set by to "Standard Level" knob + its relative attenuverted CVinput.  
If ACCENT input is HIGH when TRIG occurs, the sample will be played at "Accent Level" knob + its attenuverted CVinput.  

Playing speed can be set by SPD knob from 1 to 200% and modulated with its attenuverted CVinput. Speed can be modified during sample playback.  
External modulation is allowed only on drumPlayer+ or drumPlayerXtra.  

If CHOKE switch is on when TRIG occurs, the playback of next slot is stopped with a 1ms fade out: it's commonly used to simulate a closed/open hihat.  
LIM switch enables hard clipping limiter set to ±5v on the output.  

#### RIGHT-CLICK MENU
**Sample Slots**  
Click on the slot number to open dialog.  
When the sample is loaded the green led on the panel is turned on (drumPlayer), or the small 7segment display will show the first chars of the filename (drumPlayer+ drumPlayerXtra).  

**Set samples root folder**  
Once a folder is set, 'Samples browser' option is activated in the quick load menu (right click in the relative led slot area/display) to quickly choose samples from the selected folder.  

**Randomize**  (drumPlayerXtra only)
This option is displayed only if a root folder is set. It will load random samples from the root folder.

**Interpolation**  
There are 3 different interpolation algorithms, that are engaged during playback only when the sample samplerate differs from VCV working samplerate or playback speed differs from 100%.  
- 'No interpolation' can be used when sample rates match and speed is 100% constant  
- 'Linear 1' and 'Linear 2' interpolate the samples with different weighted averages  
- 'Hermite' uses a Cubic Hermite interpolation that offers a better result (default)  

**Anti-aliasing filter**  
Anti-aliasing filter is made up with 2x oversampling and a 20khz lowpass filter.  

**Outs mode**  
Normalled (default): if one out is not connected, the output of its slot is added to the next one  
Solo: every slot has its own out socket  
Unconnected on Out 4: Every unconnected out is added to out n.4

**Disable NAV buttons** (drumPlayerXtra only)  
Disables panel Sample Navigation buttons to avoid utilizing mistakes.

**Scrolling Sample Names** (drumPlayer+ and drumPlayerXtra only)  
This option enables text scrolling on sample name displays  

**Light Boxes** (drumPlayerXtra only)  
This option enables a light box over waveform displays when sample is triggered.  
When enebled, in each slot right-click menu, the color of light and fade duration can be set.

**Display Triggering** (drumPlayerXtra only)  
This option enables sample triggering by clicking over the the display area, to play samples live or just to test them while navigating sample folders  

**Global Settings** (drumPlayerXtra only)  
In this menu there are options to clear all the slots or the root folder.  
It is also used to apply settings to all the slots: Zoom, Lightboxes color and time fading.   

**Store samples in Patch**  
This option allows to save the loaded samples in vcv patch file to use it on other machines or if the orginal sample files are missing.  
Please note that this could make the patch filesize very large.

#### SLOT RIGHT-CLICK MENU
Right clicking on led area (drumPlayer) or display area (drumPlayer+ drumPlayerXtra) the slot right-click menu is open with following options:  
- **Load Sample**: opens file dialog to load a sample in the slot
- **Samples Browser**: if a root sample folder is set on the general right-click menu, this submenu is enabled to navigate through directories
- **Current sample**: shows sample name when a sample is loaded
- **Clear**: clears slot
- **Swap Slot with**:swaps slot sample with the selected one
- **Copy Slot to**: duplicates slot sample to the selected one

**drumPlayerXtra** further options:
- **Zoom Waveform**: zooms the waveform from start to end (full), half, quarter, eighth of the sample length
- **Light Box color**: if Light Boxes option is enabled in the general right-click menu, a predefined color or a custom one can be set here
- **Light Box Fade**: Fade time of Light Boxes is set here: Slow (0.5s), Normal (0.25s), Fast (0.1s)

[back to top](#table-of-contents)

## enver
### Envelope generator with stereo VCA

![enver](https://github.com/sickozell/SickoCV/assets/80784296/4e3a4dde-8415-4cc9-8904-074e93179546)

#### - INSTRUCTIONS
the enver moodule operates with three different modes selecetd by the mode switch:
- ENV: envelope mode works as a standard envelope generator.
- FN: function mode is an AD envelope generator and starts its attack with a trigger on the G/T button or its input.
- LP: loop mode is a looped attack/decay generator. A trigger starts the attack stage, goes to the decay stage and automatically restarts the attack stage. A new trigger will go to the release stage.

RET input is the retrigger function, available on decay or sustain stages.  

SHAPE knob selects the shape of the envelope curve. It can be set to exponential, gentle, linear, logarithmic and their average positions. It can be modulated with a CV input and its attenuverter.

Every single stage can be set with the dedicated knobs and modulated with a CV input and its attenuverter.

ENV out is the envelope output.  
INV out is the inverted envelope output to use for ducking purposes. The INV knob is used to set the inverting strength.  
A D S R outputs send a 1ms trigger at the end of every single stage.  

The built-in VCA section consists of a stereo audio input, a master volume knob, a volume CV input, and a stereo audio output.  
Please note that using the volume CV input as a velocity control from an external keyboard, the volume knob has to be set to zero. 

## holder / holder Compact / holder8
### Sample & Hold or Track & Hold with noise generator, probability and range

![holder](https://github.com/sickozell/SickoCV/assets/80784296/eba30e61-6e09-4c4d-b5dc-083362feaac1)

#### - INSTRUCTIONS
S&H / T&H switch changes the mode between Sample & Hold and Track & Hold

Sample & Hold: a trigger on TRIG input samples the last received signal on IN input that is sent to the OUT  
Track & Hold: until the gate on GATE input is HIGH the IN signal is sent to the OUT. The last received signal is sent to the OUT until the GATE is LOW.

PROB knob, with its modulation input and attenuverter, sets the probability to sample the input signal.  
SCL knob, with its modulation input and attenuverter, rescale the output signal.  
OFS knob, with its modulation input and attenuverter, offsets the output signal.  

If IN input is not connected a ±5v Noise Generator is taken as input source.

OUT output is feeded with sampled or tracked signal.  
TRIG output sends a trigger when sample occurs. If Track & Hold is selected, it sends a trigger on tracking start and/or end, or a gate depending by the options ticked on the right-click menu.

Holder can be used as a simple white noise generator if in T&H mode with Sample on HIGH gate option ticked. There's a specific function in the right-click menu to achieve this setting with one click.

When polyphonic signals are used, all signals are sampled at the same time if a monophonic trigger is connected. Otherwise if a polyphonic trigger cable is connected the signals will be sampled according to trigger polyphony.

holder8 is composed of 8 independent holder modules without attenuators and trig output.

#### RIGHT-CLICK MENU
**White Noise Type**  
There are two types of white noise generator.  
- FULL generates random ±5v voltages.
- CENTERED generates random ±5v voltages but the weight is more centered, and occasionally may exceed the range.

Comparison:  
![holder_b](https://github.com/sickozell/SickoCV/assets/80784296/234aea77-12d9-44af-a8af-c9c37ac871a2)

**Track & Hold options**

- **Sample on HIGH Gate** : this inverts standard track&hold usage, on LOW gate the signal passes and on HIGH gate the signal is held
- **Trig on Start** : sends a trigger on TRG out when the signal is sampled
- **Trig on End**: sends a trigger on TRG out when the signal stops being sampled
- **Gate Out instead Trig**: sends a HIGH gate on TRG out when the signal is sampled

**Noise Generator preset**  
This function sets the module to Track & Hold mode, sample on HIGH gate, scale on 100% and offset to 0v, just to output white noise if trig/gate input is not connected or not triggered.

[back to top](#table-of-contents)

## keySampler
### Keyboard controlled sampler

#### - DESCRIPTION
- up to 8 mono/stereo sample player/recorder
- routable samples to 8 separate stereo outputs
- keyboard controlled player with velocity and pitch bend controls
- programmable key zones for each sample with midi learn
- mono/stereo samples and 1-cycle waveforms player
- gate or triggered play mode
- envelope generator with adjustable shape, loop, reverse, pingpong
- anti-aliasing filter, phase-scan feature
- 90s style cyclic time-stretch feature

![keysampler](https://github.com/user-attachments/assets/aeb16f51-e9c9-46d9-9b37-5c4d11eb5f4f)

#### - INSTRUCTIONS

COMING SOON  

Some tips:  

Every sample is stored in one of the 8 available slots and can be individually routed to one of the 8 outputs.  
Multiple slots can be routed to the same output.  
Each slot has its individual settings.  

PB button enables pitchbending on the desired sample. Pitchbend range can be set in the right-click menu.

The 'Quantize incoming v/Oct' option in the right-click menu avoids incorrect pitches when using variable or less than perfect v/oct signals.

Key zones programming:  
Every zone is composed by lower, upper and reference notes. The reference note is the one that is supposed to be the original recorded pitch.
Zones of just one note are allowed, usually for percussions or not pitchable samples.

Press the yellow KEY SELECT button to choose the parameter to modify, then adjust the KEY knob to set the interested note or enter midi learn.  

Midi Learn is activated by holding the KEY SELECT button for about 1 second, the yellow led will fade in and out waiting the key to be pressed. Then the yellow led will flash for a while. Midi learn can be aborted by clicking the KEY SELECT button again.




[back to top](#table-of-contents)

## modulator / modulator7 / modulator7 Compact
### single or 7 triangle/ramp LFOs depending on a main rate managed by a manual knob or synchronized with a clock.

![modulator](https://github.com/sickozell/SickoCV/assets/80784296/9008ffd9-6ddb-486e-a957-3874e21b9750)

#### - INSTRUCTIONS
Following instructions refer to modulator7, but can be applied also to modulator module.  
The 'modulator7 Compact' module is just a 7 triangle only uni/bipolar LFOs whithout sync and reset features.  

Rate Knob range is 0.01/100 Hz and can be modulated by Rate Input and adjusted with its attenuverter.  
The Sync button or a trigger on the Sync Switch Input, toggles between manual or synced rate.  
SYNC input accepts triggers, like clock or other pulses, used to calculate the main rate.  
PPC (Pulses Per Cycles) knob sets the number of triggers (from 1 to 24) on the SYNC input needed to achieve 1 cycle of the main rate. 

The 'X' knobs set each oscillator's rate calculated on the main rate from 1/21x to 21x, center position is 1x and it equals to main rate.  
The default waveshape of each oscillator is triangle, the RAMP buttons switch to sawtooth waveshape.  
The default starting cycle value of each oscillator is 0v, the 'down arrow' buttons set it to the maximum, and if ramp waveshape is selected, it will result in an inverted sawtooth (ramp down).  
Default output range of oscillators is unipolar 0-10v, the 'b' buttons modify the range to bipolar +5/-5v.  

A trigger on RST input, resets all oscillators cycle, restarting waveforms according to each oscillator's 'Ph' reset phase knob.  
PLY out is the polyphonic output of oscillators. PLY knob sets the number of channels of polyphony, corresponding to the first 'n' oscillators. When PLY knob is set to 'c' position (1 poly chan), a 1ms pulse is given to PLY output when internal clock occurs.  

If no sync cable is connected, pressing the SYNC button will act as a sample&hold, holding the last oscillators values. Another sync button press will restart the normal oscillators curves.

#### Right-click Menu
- **Wait full clock after reset**: when this option is enabled and sync is on, when a reset is detected it will reset the cycle of oscillators but will wait a full clock before restarting oscillators cycle. It can be mostly used in combination of PPC greater than 1, just to restart oscillators correctly
- There are some selectable 'X' knob presets on the right-click menu. They refer to multiply or divide main rate by following series types: integer, even, odd, prime, fibonacci

[back to top](#table-of-contents)

## multiRouter / multiSwitcher
### 1>8 stereo router and 8>1 stereo switcher for cvs and audio signals

![multiRtMultiSw](https://github.com/sickozell/SickoCV/assets/80784296/b32b2642-5d30-4a08-8f69-ab0a2dbd47a4)

#### - INSTRUCTIONS
multiRouter routes a stereo input signal up to 8 different destinations, while multiSwitcher outputs a single stereo signal selected from one of the eight sources.  

On both modules the steps advance via a trigger or can be addressed via a CV voltage depending on the TRG/CV switch.  

Direction of advancing step is set by DIR switch and can be set to 'R' if random advancing is needed.  

xFD knob sets the amount of crossfading between the steps up to 10 seconds.  

The INs/OUTs selector sets the number of steps of the cycle.  

The RST knob sets the step to be restarted from when RST input is triggered.  
Please note that reset step can be outside of cycling steps of the INs/OUTs selector. For example the direction can be set to 'up', 6 cycling steps, and reset to step 8. In this case when reset is triggered it will restart from setp 8, then it will advance to step 7 and continue to step 1, and the next step will be the 6th.  

Right inputs and outputs are normalled, so if they are unconnected the signal is taken from the left ones.

#### Right-click Menu
- **Cycle**: with this option unticked when the sequence has reached the end, it won't restart from begin
- **RST input = reverse advance**: with this option ticked a trigger on the RST input will advance one step backward (if Random direction is set, a trigger will always return a random step)

[back to top](#table-of-contents)

## parking
### Set of unconnected inputs and outputs just to park unused cables

![parking](https://github.com/sickozell/SickoCV/assets/80784296/da6e1635-60cc-455a-8fe0-73b388f60224)

#### - INSTRUCTIONS
This module doesn't do anything. It's just a place to connect temporarily unused cables to not forget to where they were wired.  
It can also be used to connect other modules sockets when they need to be wired to obtain some functionality.

[back to top](#table-of-contents)

## polyMuter8 / polyMuter8+ / polyMuter16 / polyMuter16+
### Mutes or soloes the single channels of a poly-cable

![polymuter](https://github.com/sickozell/SickoCV/assets/80784296/dffb9dda-a74c-428e-8bf4-e7880c82b305)

#### - INSTRUCTIONS
polyMuter mutes the single channels of a polyphonic cable connected to IN and outputs the same number of channels to OUT.  
To avoid clicks the FADE knob sets the fade length in milliseconds of the mute/unmute operation.  
Fade range is from 0 to 10 seconds and the default setting is 10ms.  
Please note that when the knob is set to full anti-clockwise the tooltip popup will show 1ms, but it actually means no fade.  

polyMuter16 is the standard version of the module, polyMuter8 will output a maximum of 8 channels. The channel display will show the number of channels of the input cable.

The plus version of each module can also 'solo' the channels by right-clicking on mute buttons, so the led button will become green. Multiple solo channels can be selected.  
If an already muted channel is soloed the button becomes green and red, with a further right-click it will go back to mute, or if left-clicked the channel will be directly unmuted.  
By using the plus version of polyMuter the right-click menu of buttons is no longer available (that's why there are two versions of these modules), but the buttons are still midi mappable usually for the mute function only. Please note that the mute function is activated by exactly a +10v gate, and the solo function by a +3.4v/9.9v gate.

#### Right-click menu
- **Show OUT channels**: this options show the output channels in green, instead of input channels in red
- **Shrink Channels**: it deletes the muted channels from the output stream or let only the soloed channels to be on the output stream
- **Exclude -10v channels too** (on polyMuter8 and polyMuter16 only): when shrinking channels option is enabled, this will exclude from the output stream the channels with exactly -10v

[back to top](#table-of-contents)

## randLoops / randLoops8
### random voltage/trigger sequencer inspired by Turing Machine

![randloops](https://github.com/user-attachments/assets/7e15d953-2067-4d45-9d70-e4ac6956c4ef)

#### - INSTRUCTIONS

randLoops generates cv or trigger sequences up to 32 steps that can be randomized with the CTRL knob.  
The actual sequences are composed by a set of bits stored in a shift register, so when a clock trigger is detected, the register advances and the first bit can be random generated with a probability set by CTRL knob, or it is the last bit of the register that isn't discarded generating the loop.  
The CTRL knob in the center position means that the new register bits will be random and the knob full clockwise "locks" the sequence with no probability to generate new random bits. CTRL knob can be CV controlled with its input.  
Sequence length is set by the LENGTH knob up to 16 steps and can be doubled if the control knob is moved from the center position to the left, with the same probability rules.  
The CV at the OUT is calculated by the first 8 bit set to on, every bit position has its specific voltage that is summed to achieve the final output.  
This means that if all the bits are on the output is +10v and if all the bits are off the output is 0v.  
The output can be attenuated by the SCALE knob.  

The TRIG out is a 1ms trigger fired when the the sequence advances and the first bit is on.  

A trig on RST input restarts the sequence from the beginning.  
A trig on CLR input clears the sequence puttinh all the bits to off.  
DEL button (or a trigger on its input) forces the next first bit to be off.  
ADD button (or a trigger on its input) forces the next first bit to be on.  
RND button (or a trigger on its input) instantly randomizes all the bits.  

There are 32 programs to store sequences and their lengths.  
Programs are selected by PROG knob that can be CV controlled. The selected program is effective by pushing the SET button or can be set automatically if AUTO button is on.  
RECL button (or a trig on its input) recall the selected stored program, even if it's changed by randomization.  
Double-click STOR button to store the current sequence and length in the selected program.  

randLoops8 is basically a 8-track randomLoops with limited functions, but with an offset knob added. CVs and triggers can be polyphonic summed in the last track outs, according to the setting selected in the right-click menu.

#### Right-click Menu
- **Buffered DEL/ADD**: if set to on the DEL and ADD buttons (or their trigs) will force the next first bit in the sequence to be off or on only once each press
- **Buffered Random**: the same as previous, but acting with the RND button
- **Bit Resolution**: the voltages can be calculated with the first 8 bits (default) or the all 16 bits of the shift-register
- **Voltage Progression**: the standard voltage for each specific bit position voltage is doubled from the previous one. It can be changed to 1.3x, or even a Fibonacci progression, to achieve sligthly different cv sequences
- **Trig Output Type**: this affects the TRIG output behavior and can be set to Trig (1ms), Gate (HIGH until an off bit is reached), Clock Width (a gate based on the length of the incoming clock)
- **Ignore Prog Ctrl**: this ignores Ctrl knob setting when a program is recalled
- **Ignore Prog Scale**: this ignores Scale knob setting when a program is recalled
- **1st clock after reset Don't advance**:  This ignores the first clock after a reset trigger, so the sequence won't advance
- **Copy/paste seq**: copy and paste the current sequence and its settings in the clipboard (it works across other randLoops and trigSeq modules). It's safer to copy/paste sequences when module it's not running
- **Load/Save Preset**: this is used to load/save all the 32 program sequences and module settings in a ".rlp" file
- **Import/Export trigSeq seq**: this is used to load/save the current working sequence. Note that after importing a sequence you must double-click the STOR button to store it in the selected program. Sequence files can be shared also with trigSeq modules
- **Erase ALL progs**: clears all the programs 
- **Initialize on start**: clears the sequence every rack startup. Note that it doesn't erase the programs

[back to top](#table-of-contents)

## shifter
### 64 selectable stages shift register
#### - DESCRIPTION
- 64 stages shift register that outputs only the selected stage controlled by knob/CV with attenuverter
- Trigger delay to adjust the 1-sample latency of VCV cables

![shifter](https://github.com/sickozell/SickoCV/assets/80784296/19c42dd1-524a-4d21-97ca-02e235c776c6)

#### - INSTRUCTIONS
Shifter module can be useful to shift back and fotrth a sequencer output on the fly, thanks to the 64 stages register.  
Stage can be controlled via the stage knob, or the 0-10v StageCV input with its attenuverter.  
If StageCV input is not connected, the attenuverter reduces the range of the Stage knob.  
Note that the Stage knob and StageCV are added together.  
The TRIG DELAY knob can be used to delay the TRIG INPUT up to 5 samples, because of the 1sample latency of VCV cables. This can be useful when triggering the sequencer with the same clock of Shifter module, TRIG DELAY avoids that the input is sampled before the sequencer advances.  

#### Right-click Menu
- **Initialize On Start**: discards previous module state on VCV restart

![shifter example](https://user-images.githubusercontent.com/80784296/212531455-776e3110-78ef-4bec-a3f8-64180fe4ca53.JPG)  
[Download example](./examples/shifter%20example.vcvs?raw=true) (right-click -> save link as)

[back to top](#table-of-contents)

## sickoAmp
### Polyphonic stereo VCA up to 200% with limiter

![sickoamp](https://github.com/sickozell/SickoCV/assets/80784296/26e298b0-e804-41c1-b57b-6baeaa9ba388)

#### - INSTRUCTIONS
The 'Level Input' is the CV input and VCA knob is its attenuverter with a range up to ±200%, default is 100%.  
The 'Base Level' knob can be set up to 200% and it's level is added to VCA result voltage.  

LIM switch activates signal limiter set by Limit knob in the range up to ±10v.  

When no CV input is connected, the module acts as attenuator/amplifier just using the base level knob.

If CV input is polyphonc, the output will be polyphonic reflecting CV polyphony channels. Otherwise if CV input is monophonic and signal inputs are polyphonic, every signal channel will be processed with the same amplification.

#### Right-click Menu
- **Polyphonic OUTs**: when this option is enabled the outputs reflect input polyphony. Otherwise polyphonic inputs are mixed in one monophonic out

[back to top](#table-of-contents)

## sickoCrosser / sickoCrosser4
### stereo or 4 channels multi-input crossfader.
#### - DESCRIPTION
sickoCrosser / sickoCrosser4 can crossfade up to 4 signals by operating a single knob.

![sickocrosser](https://github.com/sickozell/SickoCV/assets/80784296/1193328e-7abf-4286-9ba8-2621c16712e3)

#### - INSTRUCTIONS

INs switch selects the number of input signals to be crossfaded.  
The xFD knob together with its CV input voltage sets the crossfade between the inputs.  
Colored marks of the xFD knob correspond to equal attenuation of adjacent input signals (50% / 50%).  

sickoCrosser can crossfade up to 4 stereo signals. Right inputs are normalled if no cable is plugged in.  
Polyphonic cables can be connected on the first stereo input, in this case the crossfading will be applied between the polyphony channels by selecting the number of channels with the PLY knob, then the INs switch and other inputs will be ignored.  

On sickoCrosser4 the 'L' (Link) led buttons can link channels in two ways according to the right-click menu 'Link Mode' selection.
- 'xFd + Inputs' will link channels to add inputs to be crossfaded. All the inputs of linked channels are crossfaded except the last channel that can be set via its INs switch. Only the first xFD knob and OUT of the linked channels will work.  
- 'xFd only' will link channels only for the xFD knob. In this mode the crossfade is set by the first xFD knob of the linked channels. For example 4 stereo signals can be crossfaded connecting the left signals on the first channel and the right on the second one, leaving the other 2 channels for mono signals or another stereo crossfading. Linking all the channels is allowed.  

[back to top](#table-of-contents)

## sickoLooper1 / sickoLooperX / sickoLooper3 / sickoLooper5
### 1/3/5 track loopers with builtin clock generator, click and meter.
#### - DESCRIPTION
sickoLooper is inspired by hardware looper devices with most of their features implemented.

![sickolooper1x3](https://github.com/sickozell/SickoCV/assets/80784296/3a6d28ae-1882-46a7-ba56-772fdc679d40)  
![sickolooper5](https://github.com/sickozell/SickoCV/assets/80784296/d7b2110a-4ffd-4171-96c2-b724a360875e)

#### - MANUALS
Due to the complexity of these modules, a PDF user manual has been written

Download [ENGLISH user manual](./docs/sickoLooper%20user%20manual%20%5BEN%5D.pdf?raw=true)

Download [manuale utente ITALIANO](./docs/sickoLooper%20manuale%20utente%20%5BIT%5D.pdf?raw=true)

[back to top](#table-of-contents)

## sickoPlayer
### wav sample player

#### - DESCRIPTION
- samples and 1-cycle waveforms player
- ±24 semitones tuning and v/oct input with polyphony
- envelope generator, loop, reverse, pingpong
- different interpolation modes, anti-aliasing filter, phase-scan feature

![sickoplayer](https://github.com/sickozell/SickoCV/assets/80784296/3e11b1a7-fc15-4335-a43e-361960e4d000)

#### - INSTRUCTIONS
Load sample using the right-click menu or right-click in the waveform display area to access quick load menu.  
Once a sample is loaded samples in the same folder can be browsed using Previous and Next buttons above the display.  

The display shows the waveform, filename, sample duration and number of channels (1-2 channels wav file are allowed).  

Mode switch allows to select if sample playback starts with a trigger or play it until a gate is high.  

When in Trig Mode the Trig-Type switch has 3 options:  
- **SS (Start/Stop)** A trigger starts attack stage from CueStart postition, another trigger sets playback to release stage and at the end sample position is reset to cue start  
- **R (Restart)** Every trigger starts attack stage from CueStart position. Only a Stop trigger stops playback.  
- **PP (Play/Pause)** A trigger starts attack stage from curent sample position, another trigger goes to release stage. A Stop trigger reset position to CueStart.  

In any Trig-Type a trigger on STOP input sets the playback to release stage.

TRG/GTE and STOP buttons have effects only on channel 0 of polyphony.  

Cue Start/End knobs are used to set the start of the Attack and the Release stage.

When Loop button is switched on, playback restarts from Loop Start when Loop End is reached.

REV button changes the playback start direction.

PNG button enables PingPong mode:
- in TRIG mode, when loop is enabled, if Loop Start/End is reached playback direction is inverted without stopping playback
- in GATE mode, if Loop Start/End is reached when loop is enabled, or when Cue Start/End is reached, playback direction is inverted without stopping playback

XFD knob (crossfade) sets crossfading time in loop mode, or fadeout time before the end of sample is reached.  

The envelope knobs can be external modulated with attenuverted CVinputs.

Tune knob with its attenuverted CVinput, can tune up or down the sample with a ±2 octave range (semitone scale).  
v/oct input accepts polyphonic cable usually combined with a polyphonic gate in when in Gate Mode.  

Master knob, with its attenuverted CVinput, sets the output volume from 0 to 200%. Limit switch is a hard clip limiter with a ±5v range. A led clip light warns of clipping.  

If sample file is mono, left out is duplicated to right out.  
EOC outputs a 1ms pulse when the sample reaches certain point according to a specific right-click menu (see below).  
EOR outputs a 1ms pulse when the sample reaches the end of release stage.

#### RIGHT-CLICK MENU
**Sample Slot**  
Click on "Load Sample" to open dialog. Use Clear options to unload sample from slot.  
As described before, just right-click over the waveform display area to access the quick-load menu.  
When a sample is loaded, file sample rate and number of channels are shown here.  

**Set samples root folder**  
Once a folder is set, 'Samples browser' option is activated here and in the quick load menu (right click on display) to quickly choose samples from the selected folder and subfolders.    

**Interpolation**  
There are 3 different interpolation algorithms that are engaged during playback only when the sample samplerate differs from VCV working samplerate or playback speed differs from 100%.  
- 'No interpolation' can be used when sample rates match and tune is set to zero  
- 'Linear 1' and 'Linear 2' interpolate the samples with different weighted averages  
- 'Hermite' uses a Cubic Hermite interpolation that usually offers a better result (default)  

**Phase scan**  
This feature automatically sets Cue and Loop Start/Stop positions at zero crossing points to avoid loop clicks and pops eventually in combination with proper crossfade length.  
Be sure to disable it when using one-cycle waveforms, or simply use the specific preset (see below)  
Please not that Scan will be applied only on left channel  

**Anti-aliasing filter**  
Anti-aliasing filter is made up with 2x oversampling and a 20khz lowpass filter.  

**Polyphonic OUTs**  
When this option is enabled the audio and EOC/EOR outputs reflect v/oct input polyphony. Otherwise polyphonic outputs are mixed in one monophonic out.

**Polyphonic Master INs**  
When this option is enabled the Master CV input accepts polyphonic cables according to V/Oct input polyphony. For example this can be used for velocity control.

**EOC pulse from**  
This submenu sets when the EOC pulses are triggered:  
- TRG/GATE when is triggered to stop
- STOP triggering
- reached CUE END (on forward playback)
- reached CUE START (on reverse playback)
- reached LOOP END (on forward playback)
- reached LOOP START (on reverse playback)
- PING: reached LOOP END (when pingpong looping on forward playback)
- PONG: reached LOOP START (when pingpong looping on reverse playback)

**Reset Cursors on Load**  
Always resets Cue/Loop Start/stop to 0 and 100% when a new sample is loaded.  

**Disable NAV buttons**  
Disables panel Sample Navigation buttons to avoid utilizing mistakes.  

**Store sample in Patch**  
This option allows to save the loaded samples in vcv patch file to use it on other machines or if the orginal sample files are missing.  
Please note that this could make the patch filesize very large.

- Unlimited File Size (risky). The maximum memory allocation for samples is set to 200Mb. For example, at 48khz Mono the maximum is about 18mins. Ticking this option this limit is removed, but there can be a crash risk if the sample file exceeds the computer abilities.

**Presets**
There are some factory presets stored in the right-click menu for common using settings.  

#### USING ONE-CYCLE WAVEFORMS
One-cycle waveforms can be used in GATE mode with LOOP mode enabled.  
Be sure to recall relative preset or disable PhaseScan, adjust Cue and Loop START to 0% and Cue/Loop END to 100% and enable loop button.  

[back to top](#table-of-contents)

## sickoQuant / sickoQuant4
### single or 4 channels polyphonic quantizer with scales and presets.
#### - DESCRIPTION
sickoQuant / sickoQuant4 can quantize signals in chromatic/min/Maj or custom scales, continuously or triggered.

![sickoquant](https://github.com/sickozell/SickoCV/assets/80784296/eba80507-4a65-4fd5-be07-ef81c95d5bca)

#### - INSTRUCTIONS

IN input is used to connect the signal to be quantized and accepts polyphonic sources.  
OUT output provides the quantized signal.  

Quantization is applied to the closest note in the current working scale. If there are no notes selected there is no quantization and no signal at the out.

Inputs can be attenuated by ATT knob or sampled&held with a trigger via the TRG input.  
A further voltage on the OFS input can be applied before quantization.  
The OCT knob can shift the signal up to 3 octaves below or above the source signal.  

SCL knob selects the quantizer scale and hold it in pending state. The relative SET button sets the scale in actual working state.  
SCL knob in the middle position sets the chromatic scale, turning the knob clockwise selects Major scales, moving it counterclockwise selects minor scales.  
A cv input with a range of ±10v is provided and its value is added to knob selection.  

PROG knob selects 32 further user scales that can be recalled via the RECL button, with a pending state as the SCL knob does.  
A cv input with a range of ±10v is provided and its value is added to knob selection.  
Double-clicking the STOR button stores the currently displayed notes in the program selected by the PROG knob.  
While a program is being stored, the STOR button LED remains lit for a while.  

Enabling the A (auto) button deactivaes the pending states, so the selected scales or programs are instantly set to working state.  

A keyboard-like set of LED buttons is provided to show the working scale or manually modify it.  
If a pending scale or program is manually modified it will be set as a working scale only when the SET button (marked as ^) located below the keyboard is pressed.  
On the other hand, editing an actual working scale with the note buttons, it will cause the change to be applied immediately.  
This SET button can be also used to apply pending scales or programs.  

A display shows the current working scale or program in green, or both the working scale in green and the pending one in red.  
Scales are shown with the usual scale notation (C, C#, Dm, F#m, etc.), 'chr' for cromatic, 'CST' for other custom scales, 'N.Q' for no quantization.  
Programs are shown from P0 to P31, even if no notes are selected (no quantization).  

All 32 programs can be saved in a preset file (.SQN) with the 'Save PROG preset' function in the right-click menu, and reloaded in other sickoQuant modules with the 'Load PROG preset' function.  

To erase all programs in memory in the right-click menu there is a function 'Erase ALL progs'. For safety, it has to be clicked 'ERASE!' in the sub menu 'Are You Sure?' to proceed.

CV is the standard program input control. In the right-click menu there is the "Prog Input Type" option that can be set to 'Trig'.  
It advances to the next program with a trigger on Prog Input. When reached the last stored program it restarts from program 0.  
In the right-click menu there is also the 'Scan Last Prog' option that shows the last stored program and by clicking this option a full rescan of all programs is done.

##### sickoQuant4

sickoQuant4 module works like sickoQuant but provides 4 normalled polyphonic channels.  

In the right-click menu there is a further function to sum the first 'n' channels (rows) and output a polyphonic signal (with a 'row' correlation) to the output #1 marked with a 'S' and circled in pink.  
In this case, if there are polyphonic sources connected, only the first polyphony channel of the sources will be summed to the polyphonic out #1, just to maintain the purpose of this 'sum' function.  
Setting the 'sum' function will alter only the first output, but the other channels will work as usual.  

[back to top](#table-of-contents)

## sickoSampler
### wav sample player and recorder

#### - DESCRIPTION
- mono/stereo sample recorder  
- mono/stereo samples and 1-cycle waveforms player
- ±24 semitones tuning and v/oct input with polyphony
- envelope generator, loop, reverse, pingpong
- anti-aliasing filter, phase-scan feature

![sickosampler](https://github.com/sickozell/SickoCV/assets/80784296/9950ed5c-c454-4cb6-9806-ad81844502b6)

#### - INSTRUCTIONS

About player functionalities please follow sickoPlayer instructions. Please note that loaded samples in sickoSampler are always resampled to VCV working samplerate. For this reason interpolation is fixed to Hermite. It can be always toggled antialiasing filter from the right-click menu. 

In sickoSampler the display shows also the recording time and a yellow "S" if sample is not saved yet. 
In the right-click menu, along file infos, it's shown if the sample was resampled on loading and if it has to be saved because a recording occurred.  

Recording section has 2 inputs, but record is only enabled if at least left channel is connected.  
Record button arms recording waiting for a playback trig, or starts recording when a sample is playing back, or starts recording immediately if PoR button is on (see below).   
Record trig input toggles start/stop or arm/unarm recording, but if "STOP REC" trig input is connected it only starts recording.  

GAIN knob adjusts the volume of the inputs.  
FD knob sets the fade in/out time when recording starts or stops.  

OVD button overdubs existing sample.  

XTN button enables extended recording. In forward recording, it continues recording also when cue end point or sample end are reached. In reverse recording it keeps recording until sample begin point is reached. If loop is enabled XTN button has no effect and it will record as usual.  
Please note that when in loop recording XFD knob is overridden, and it will not do any crossfade.  

RRM (REC Re-Arm): when recording is stopped by a playback trig/button, it is rearmed when release time has ended or fadeout recording has finished. This function is not available in conjunction with POR "Play on REC". In "Restart" trig-type mode recording is rearmed only when after a STOP trig/button is detected.  

REL (Record Release Stage): keeps recording while playback is stopped and it is in its release stage. Please note that it will always continue recording until the "Rec Fade in/out" knob (FD) time setting is reached (also if release stage of the playback is reached).  

UCE (Update Cue End): resets the Cue End cursor to the end of recording when it is stopped (if recording is reversed it updates Cue Start cursor).

ULE (Update Loop End): same as above, but affects Loop End cursor.

POR (Play On REC ON): when REC button is switched ON or REC trig it will start playback and recording simultaneously. It disables also the REC Re-Arm function.  

SOR (Stop On REC OFF): when REC button is switched OFF or REC trig or REC STOP trig, it will stop record and playback simultaneously. If playback STOP button/trig occurs, it will stop recording only and continue playing. In Play/Pause trig type it will reset position to Cue Start as STOP button/trigger usually does.   

MON switch selects inputs routing to the outs: always [ON], while recording only [REC], or never [OFF].  

Recording speed follows v/oct and tune settings.  
As recording is not polyphonic, polyphony is disabled when the REC button is switched on, and the record playhead will follow only channel nr 0 on the polyphonic cables connected to v/oct.  

"Sample Clear" input: if this input is triggered, it erases the recorded sample.

**TIP**: If it's planned to record a sample to play it polyphonically with a master keyboard, please connect gate and v/oct to a MIDI>CV module with 'Reset' polyphony mode selected. Clear any previous sample in memory, select GATE mode, switch the LOOP button on, arm recording and consider adjusting envelope knobs. On key press (C4 for example) sickoSampler will start recording until key is unpressed, then the sample can be played immediately.

**New sample recording**  
In Trig Mode, when recording is stopped by a trig or blue button press and LOOP button is switched on, the sample will be played back immediately in loop mode, and if Rec Re-Arm is on it will keep recording.


#### RIGHT-CLICK MENU
Please refer to sickoPlayer for the right-click menu, in sickoSampler following options are added:
- **Save FULL Sample**: saves the entire sample in a wav file
- **Save CUE Region**: saves the wav file from Cue Start to Cue End
- **Save LOOP Region**: saves the wav file from Loop Start to Loop End
- **Trim Sample after Save**: If this option is enabled, the sample is trimmed and saved in the chosen saving mode, otherwise it will be saved trimmed, but the entire sample remains available in memory
- **Save Oversampled**: If this option is enabled, samples will be saved at sickoSampler working samplerate (2x VCV samplerate). This can be useful when samples are recorded at different speeds than normal for further external editing
- **UCE/ULE updates also Start**: if UCE and/or ULE button are on, when recording is stopped also Cue Start and/or Loop Start cursors are reset to the recording start position (if recording is reversed it updates Cue/Loop End cursor)
- **Crossfade while Rec Fading**: If overdub is not activated this option crossfades between previous and current recording only during fading in/out recording time, accordingly to the FD knob
- **Unlimited REC (risky)**. The maximum recording memory allocation is set to 200Mb, so the maximum recording time depends on the working sample rate and if the recording is mono or stereo. At 48khz Mono the maximum recording time is about 18mins. Ticking this option the limit is removed, but there can be a crash risk if recording time exceeds the computer abilities

[back to top](#table-of-contents)

## sickoSampler2
### wav sample player and recorder

#### - DESCRIPTION
- mono/stereo sample recorder  
- mono/stereo samples and 1-cycle waveforms player
- v/oct input with polyphony
- 90s style cyclic time-stretch feature
- envelope generator, loop, reverse, pingpong
- anti-aliasing filter, phase-scan feature

![sickosampler2](https://github.com/sickozell/SickoCV/assets/80784296/8bdfcaf7-c22f-4be4-b456-5ea4a79097fe)

#### - INSTRUCTIONS
This audio sampling module simplifies the functionalities of sickoSampler by eliminating overdubbing, tuning and modulation of many parameters, but at the same time it implements a 90s style cyclic time stretch algorithm.  
Please refer to sickoSampler and sickoPlayer instructions for the main module features.  
The main difference is that recording is enabled only when the sample is not loaded or previously recorded, so the sample has to be cleared to reactivate the REC button.  
A REC buttonpress starts recording immediately, there is no arming functionality.  
Monitor switch has been replaced by a led buttton.  

Recording fade in/out can be achieved setting XFD knob, please remember to set it back to 0 if this is not necessary on playback.  
The default trig type is Start/Stop: a first trig on Trig Input or a Trig buttonpress starts playback and a second trig goes to release stage.  
The 'R' led button switches to Start/Restart trig type: every trig on Trig Input or a Trig buttonpress restarts sample from the beginning. As there is no STOP button, the sample can be stopped just switching the Mode to Gate.  
The VOL knob set the master volume from 0 to 100%. Its CV input is added to knob value.  

**Time Stretch**  
'STRETCH' knob lengthen or shorten the sample without pitch change and it can be set from 1 to 999%.  
100% means no time stretch (default). If it is set to 50% the sample will be played in half time. If it set to 200% the sample will be played in double time as original.  
'SIZ' knob sets the size of the cycle in milliseconds.  
When lengthening some clicks may occur at the end of the sample, especially when looping. This behaviour can be reduced adjusting cross fade knob and/or cycle size.  

Cyclic Time stretch algorithm usually plays samples with its characteristic metallic sound, especially when slowing down and depending on cycle size setting.  
If it is used in combination with v/oct modulation some interesting results can be achieved.  
Please note that extreme settings can however alter pitch a little or obtain a bit of chorus/echo.

**Right-click Menu**  
**Auto Monitor Off**: this option is activated by default and it stops monitoring input after a recording is stopped

[back to top](#table-of-contents)

## simpleSeq4
### 4 step sequencer with direction and knobs range

![simpleseq4](https://github.com/user-attachments/assets/cf46a543-c71e-477c-9aae-509d95527f3d)

A tiny and simple 4 step sequencer. If a positive (greater than +1v) voltage is applied to REV input the sequencer will advance backwards.  

**Right-click Menu**  
- Various knob voltage ranges are available (default is -10/+10v)
- **Reverse Input Voltage**: if 'Negative' is ticked the sequence will advance backwards only if a negative (less than -1v) gate is applied to REV input


[back to top](#table-of-contents)

## slewer
### Slew limiter and LFO
#### - DESCRIPTION
- Slew limiter with precise timing
- Slew timing setting regardless of the distance between the incoming voltages
- Symmetric shape curves
- LFO

![slewer](https://github.com/sickozell/SickoCV/assets/80784296/f83f81d5-9dba-4359-a4e2-2d3fbd712928)

#### - INSTRUCTIONS
ATT knob with its attenuverted input sets the duration of the rise stage.  
DEC knob with its attenuverted input sets the duration of the fall stage.  
CURVE knob with its attenuverted input sets the shape of the slew: logarithmic, linear, exponential.  

SYM button inverts the decay shape to be visually equal to the attack one.

ATT and DEC outputs are high gates when relative stages occur.

IN and OUT are the signal ports.

If no signal input is connected the module acts as LFO.

In the right-click menu there is the option "Duration/Slew knobs".  
When ticked, the ATT control sets both the attack and decay durations and the DEC control sets the percentage of attack/decay. Centered setting means that attack and decay have the same duration.


[back to top](#table-of-contents)

## stepSeq / stepSeq+ / trigSeq / trigSeq+ / trigSeq8x
### 16 step/trigger sequencer with direction and presets

![stepseqtrigseq](https://github.com/user-attachments/assets/fbb46ecd-aa22-4f18-a2c5-1be4721e2bc7)

#### - INSTRUCTIONS
stepSeq and trigSeq can respectively output a voltage or a trig/gate/clock up to 16 steps.  
CLK/CV switch sets the clock or cv working mode.  
In 'clock' mode a trigger on input will advance one step, in 'cv' mode the current step is determined by the voltage applied to the input in the range 0-10v to always get the maximum step when 10v is applied.  
RUN button turns on/off the sequencer. A gate on its input runs the sequencer until the gate is high.  
A high gate on REV input will advance backards on clock trigger (clock mode only).  
A trig on RST input goes to the step set by RST knob (clock mode only).  
Sequence length can be set with Length knob that can be modulated by CV input. The CV in the range of 0-10v is added to the knob setting to always get the maximum step when 10v is applied.  

The plus versions of stepSeq and trigSeq can store up to 32 different sequences.  
Prog knob and a CV on its input sets the program number.  
When a program change is detected the 'SET' button flashes and waits to be pressed to apply the program.  
If 'AUTO' button is on, the program is applied instantly when a program change is detected.  
A double click on 'STOR' button saves the current sequence to the selected program.  
'RECL' button reloads the stored selected program.  

TrigSeq and TrigSeq+ have a TURING mode setting in the right-click menu. It activates the CV out calculation regarding the on steps as if it was a 'fixed' randLoops module where you can manually set the bits of the register. Please refer to randLoops instruction to see how it works.  
In TURING mode the RST knob acts like a CV out attenuator.  


#### RIGHT-CLICK MENU
- **Knob Range** (stepSeq only): sets the range of the step knobs (default -10/+10v)
- **Run input**: 'Gate' is the default setting as explained above, 'Trig' will toggle run on/off by triggers on run input
- **Reverse Input Voltage**: 'Positive' is the default setting as explained above, 'Negative' will advance backwards with -1v or less applied to REV input
- **Output type** (trigSeq only): 'Trig' (default) outputs a 1ms trigger, 'Gate' outputs a high gate for all step duration, 'Clock' outputs a gate of the same length of the clock input
- **Reset on Run**: A step reset is applied when the sequencer goes from OFF to RUN (default)
- **1st Clock after reset**: if 'Don't Advance' is ticked, the first clock detected won't advance the sequencer (default)
- **TURING MODE** (trigSeq trigSeq+ only): Enables the TURING mode as explained above, it activates 'Bit Resolution' and 'Voltage progression' options
- **Copy/paste seq**: copy and paste the current sequence and its settings in the clipboard (trigSeq and trigSeq+ share clipboard also with randLoops modules, stepSeq and stepSeq+ share their own clipboard)
- **Import/Export trigSeq seq** (trigSeq trigSeq+ only). This is used to load/save single sequences in a ".tss" file that can be used also in randLoops modules
- **Initialize on start**: doesn't remeber the last step reached when Rack is reloaded

Plus versions only:  
- **Prog Input Type**: CV is the standard program input control. When 'Trig' option is ticked, the modules advance to the next program with a trigger on Prog Input. When the last stored program is reached, it restarts from program 0. A trigger on Reset input set also the program to 0
- **Scan Last Prog**: The last stored program is shown in the menu, by clicking this option a full rescan of all programs is done.
- **Copy/paste seq**: copy and paste the current sequence and the length/reset settings in the clipboard (does not work across multiple modules)
- **Load/Save PRESET**: load or save a 'ssp' or 'tsp' preset file with all programmed sequences, including lengths, reset settings, and right-click menu settings
- **Erase ALL progs**: resets all stored programs to default

[back to top](#table-of-contents)

## switcher / switcherSt / switcher8
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

![switcher](https://github.com/sickozell/SickoCV/assets/80784296/8ff64a26-a092-4ce1-8175-b1e02de8c8ee)

#### - INSTRUCTIONS
Switcher, SwitcherSt (used for stereo signals) or Switcher8 are multifunction modules that can be used as follows. The versatility of the module is offered thanks to the automatic detection of the function type.

**TOGGLE/GATE modes**  
When the MODE switch is in 'TOGGLE' position functions are activated by triggers in a toggle style.  
When in 'GATE' position functions are gate sensitive, so they stay active until 'T/G' input receives a high gate.  

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
Fader knob sets the crossfade time (up to 10s) between the switched/routed/swapped signals. Set the knob to minimum (1ms) to have no fade.
CV input is added to Fade knob value and the sum will be clamped in the range of 0-10v.  

#### Right-click Menu
- **Initialize On Start**: discards previous module state on VCV restart
- **Route & Hold** (switcher module only): when using the module in Toggle mode and Router function, the last achieved input is held (instead of going to 0v) to the unrouted output

**NOTES**  
- If Fader knob is set to 1ms it won't do any fade
- In FlipFlop and ToggleGate function types the output will consist in a 'fixed' AR envelope
- When a fade time is set, the module will act as an envelope generator, so if a function activation is detected during a fade, the function will restart immediately (not like a function generator)
- On SwitcherSt module the function type is detected on Left channel sockets, so don't use Right channels without Left ones
- Polyphony on signal inputs is replicated on outs
- switcher8 is eight switcher modules in one, without fade CV input

[back to top](#table-of-contents)

## toggler / toggler Compact
### Stereo signal toggle switch router, with VCA and ASR envelope generator, in regular and compact form factor
#### - DESCRIPTION
- Toggled VCA with builtin ASR envelope generator
- Toggled ASR envelope generator
- mute/unmute CVs or mono/stereo AUDIO signals according to an ASR envelope activated by a Gate or Toggle Triggers

![toggler](https://github.com/sickozell/SickoCV/assets/80784296/9d7c77b4-9683-4d02-9e6b-ecb3a68a07fa)

#### - INSTRUCTIONS
**TOGGLE MODE**

On receiving a trigger on TRIG input, it will send the L+(R) inputs to L+(R) outputs and set the GATE output to high. On next trigger it will interrupt L+(R) outputs and set the GATE output to low.

Attack, Sustain and Release knobs set the envelope of the routed signal.

A, S, R CVinputs are added to respective knob values, Toggler module has attenuverters.

If L or (R) inputs are not connected, L and (R) outputs will provide just the envelope, so a mono signal can be connected to L input to route it to L output and nothing connected to (R) input to have the envelope on (R) output.

A trigger on RESET input will reset the toggle state.

Polyphony on L/(R) inputs is replicated on outs.  

**GATE MODE**

The same of toggle mode, but the signals will be routed only while GATE input is in a high state.

NOTE: If a new GATE or Toggle TRIGGER is detected on Attack or Release phases, the envelope ramp will immediately restart from the reached point, as a regular envelope generator and not like a function generator.  

**SPECIAL BEHAVIORS**

If Attack is set to 1ms (and release is set greater than 1ms) and a new GATE or Toggle TRIGGER is detected before Release phase has ended, the next Release phase will start from the previous reached release point.

If Release is set to 1ms (and attack is set greater than 1ms) and a new GATE or Toggle TRIGGER is detected before Attack phase has ended, the next Attack phase will start from the previous reached Attack point.

These behaviors are more understandable connecting a scope on the output.

#### Right-click Menu
- **Initialize On Start**: discards previous module state on VCV restart

[back to top](#table-of-contents)

## wavetabler
### wavetable sample player

#### - DESCRIPTION
- 1-cycle waveforms player
- ±24 semitones tuning and v/oct input with polyphony
- envelope generator, reverse, pingpong
- anti-aliasing filter

![wavetabler](https://github.com/sickozell/SickoCV/assets/80784296/187583f0-1eab-4ac7-974f-aa656e2a40f3)

#### - INSTRUCTIONS
Load sample using the right-click menu or right-click in the waveform display area to access quick load menu.  
Once a sample is loaded samples in the same folder can be browser using Previous and Next buttons below the display.  

REV button changes the playback start direction.

PNG button enables PingPong mode: playback direction is inverted when sample reaches its edges.  

The envelope knobs can be external modulated with attenuverted CVinputs.

Tune knob with its attenuverted CVinput, can tune up or down the sample with a ±2 octave range (semitone scale).  

Master knob, with its attenuverted CVinput, sets the output volume from 0 to 200%. Limit switch is a hard clip limiter with a ±5v range. A led clip light warns of clipping.  

#### Right-Click Menu
Please refer to sickoPlayer documentation.

[back to top](#table-of-contents)

## CREDITS
The Component Library graphics for these modules are copyright © VCV and licensed under [CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/)  

A very big thanks to [Omri Cohen](https://omricohen-music.com/) for developing help and support  
Thanks to [Squinkylabs](https://github.com/squinkylabs), [Firo Lightfog](https://github.com/firolightfog) and [AuxMux](https://instagram.com/aux.mux) for help and testings, and all the [Vcv community](https://community.vcvrack.com)  
Thanks to [Clément Foulc](https://github.com/cfoulc) for creating [cfPlayer](https://library.vcvrack.com/cf/PLAYER), which was the basis and inspiration for writing all the sampler modules in this collection  

**Consider donating**  
The work necessary to develop these modules required many hours of work and many sleepless nights.  
Sickozell plugin is and will always remain free, but if you find it useful, consider donating even just a coffee by following this [payPal](https://paypal.me/sickozell) link.  
Thanks.

[back to top](#table-of-contents)
