### 2.7.4 (2025-07-17)
- added stepStation and trigStation modules
- added sampleDelay module
- added cvMeter module
- added randMod7 and randMod7compact modules
- stepSeq8x / trigSeq8x: adjusted RECL and STOR buttons placement
- all sampler modules: improved memory allocation management
- slewer/slewer mini: inverted shapes on shape knob
- sickoSampler: fixed a bug that crashes Rack when clearing the sample via trig input during playback
- sickoSampler2: fixed a bug drawing the playhead outside the module
- clocker2: added CV clock feature, added port/param context menus, fixed clock division bug on reset, changed bpm clock color display
- ad mini: loop status recorded in patch
- merged repository with 4ms Metamodule repo

### 2.7.3 (2025-04-18)
- updated dr_wav library
- adMini / enverMini : added "LVL knob -> ENV out" option in context menu
- drumPlayerMk2 / drumPlayerMini : change accent level range to 0-200%

### 2.7.2 (2025-04-01)
- added stepSeq8x module
- added drumPlayerMk2 module
- added drumPlayer mini module
- added ad mini module
- added enver mini module
- added enver mini expander module
- added randLoops mini module
- added slewer mini module
- trigSeq8x trigSeq trigSeq+ stepSeq stepSeq+: added "randomize steps" menu option
- slewer: fixed a bug on attack/decay CV inputs

### 2.7.1 (2025-03-18)
- added randLoops module
- added randLoops8 module
- added trigSeq8x module
- trigSeq trigSeq+: added Turing mode, changed RST knob behavior according to Turing mode, added Prog Input Trig Mode
- stepSeq stepSeq+: changed range tooltips display on step knobs, fixed bug on reverse direction, added Prog Input Trig Mode
- stepSeq+ trigSeq+: added RECL button functionality to cancel on pending program change. Fixed bug on Rack reload if it was closed during pending prog change. Improved CPU performance
- sickoQuant sickoQuant4: added Prog Input Trig Mode. Improved CPU performance
- improved samples memory management on sampler modules
- added global 4ms-MetaModule compatibility

### 2.7.0 (2025-02-01)
- added 'trigSeq+' module
- added 'stepSeq+' module
- 'stepSeq': fixed range reset bug on Rack reload

### 2.6.18 (2025-01-30)
- added 'trigSeq' module
- added 'stepSeq' module

### 2.6.17 (2025-01-25)
- added 'simpleSeq4' module
- added 'Attenuator' option on adder8 module
- 'drumPlayerXtra': added 'Randomize samples' in context menu when setting a root folder
- 'sickoLooper' and 'clocker' modules: improved audio click management
- 'clocker' and 'clocker2' modules: fixed a bug on /2 clock division that was actually x2, and fixed its display color to red instead of green
- 'sickoQuant' and 'sickoQuant4' module browser search made easier with keyword 'squant'
-  multiRouter / multiSwitcher: added 'cycle' and 'RST input = reverse advance' options in the right-click menu

### 2.6.16 (2024-10-17)
- added 'keySampler' module.
- added 'x48' and '/48' multipliyng options on 'clocker' and 'clocker2' modules

### 2.6.15 (2024-08-07)
- enver: changed envelope shapes order and increased curve resolution.
- polyMuter modules: added 'Shrink channels' and 'Show OUT channels' options in the right-click menu.
- polyMuter8, polyMuter16: added 'exclude -10v channels too' option in addition to channel shrinking.
- sickoSampler / sickoSampler2: fixed a bug that doesn't store 'wav' extension when saving.

### 2.6.14 (2024-06-23)
- added 'slewer' module.
- added bipolar button on 'modulator7 compact'.

### 2.6.13 (2024-05-24)
- added 'clocker2' module.

### 2.6.12 (2024-05-22)
- improved 'enver' retrigger timing.

### 2.6.11 (2024-05-20)
- added 'enver' module.
- added 'multiSwitcher' and 'multiRouter' modules.
- improved usability of 'sickoAmp'.

### 2.6.10 (2024-05-02)
- added 'sickoCrosser' and 'sickoCrosser4' modules.
- sickoSampler2: fixed rack crash when modulating stretching and v/oct at the same time.
- holder modules: added ability to sample polyphonic signals according to polyphonic triggers.

### 2.6.9 (2024-04-12)
- added 'sickoQuant' and 'sickoQuant4' modules.
- sickoPlayer / sickoSampler / sickoSampler2: added 'Unlimited Sample Size' option in right-click menu.

### 2.6.8 (2024-03-11)
- added 'polyMuter8' and 'polyMuter8+' modules.
- added 'polyMuter16' and 'polyMuter16+' modules.
- added 'modulator7 Compact' module.
- calcs: added output range selection to context menu.

### 2.6.7 (2024-03-02)
- switcher: added 'Route & Hold' feature.
- changed ports and knobs design.

### 2.6.6 (2024-02-19)
- added modulator module.
- added modulator7 module.
- added switcher8 module.  

### 2.6.5 (2024-01-28)
- sickoLooper1/sickoLooperX: fixed a bug when reverse overdubbing.  
- sickoSampler/sickoSampler2: Allowed poly cable sources.

### 2.6.4 (2024-01-27)
- added sickoLooper1 and sickoLooperX modules.  
- sickoLooper3, sickoLooper5: added missing 'OVERDUB after REC' setting in preset storing.  
fixed a bug that doesn't restore 'Internal Clock Always ON' on startup.  

### 2.6.3 (2024-01-13)
- sickoLooper: Allowed poly cable sources.  
Added 'Internal Clock Always ON' option in the right-click menu.  
Set default 'Play Full Tail On Stop' to off.  
fixed a bug with PLAY/STOP ALL button that doesn't stop recording a non-synced track.  
fixed a bug on 'play full tail on stop' when looping solo tracks.
- clocker: added external clock ppqn option in the context menu.  
Fixed a bug on bar detection when using external clock.  
Changed trig/swing led to color red.

### 2.6.2 (2023-12-26)
- added holder, holder Compact and holder8 modules  
- sickoLooper: added visible context menu options even if unselectable

### 2.6.1 (2023-12-17)
- sickoLooper: added dark green color to the display bar when the loop is still playing even if its recording is finished  
fixed a bug on 'stop all' trig input  
- clocker: fixed a bug on swing control

### 2.6.0 (2023-12-10)
- added sickoLooper3 and sickoLooper5 modules
- added "Clear Sample Input" on sickoSampler
- minor design changes on sickoPlayer, sickoSampler, sickoSampler2

### 2.5.10 (2023-11-19)
- all player and sampler modules: added 'Store Sample in Patch' feature
- clocker: fixed a bug when time signatures are in eights
- all module with leds displays: added module drag&drop when over leds displays

### 2.5.9 (2023-11-01)
- added bGates module
- bToggler / bTogglerCompact: added "Trigger to Gate Output" option in context menu

### 2.5.8 (2023-10-07)
- added sickoAmp module
- clocker: added swing feature
- sickoPlayer / sickoSampler / sickoSampler2: added EoC pulse when sample has reached begin/end if start/end cursors are set to 0% or 100%
- sickoPlayer / sickoSampler: improved cpu load when using xFade knob
- sickoSampler / sickoSampler2: fixed bugs that display cursors wrong when loading a sample after a fresh recording or clear sample when a new sample load is cancelled
- all player/sampler modules: fixed a bug that shows swapped files between folders when foldernames begin with the same word.  
Removed refresh folder option from context menu and implemented an auto-refresh function
- all attenuverters knob scale has been modified to percentage

### 2.5.7 (2023-10-01)
- improved time stretch feature on sickoSampler2 and removed TimeStretch crossfading knob  
- fixed a bug on player/sampler modules that displays play cursor outside module after loading a shorter sample

### 2.5.6 (2023-09-30)
- added sickoSampler2 module
- fixed a bug on sickoSampler crashing rack when pressing nav buttons after a new recording

### 2.5.5 (2023-07-17)
- fixed a bug on all player/sampler modules crashing VCV on MACs when root folder is not found
- added a functionality on all player/sampler modules to to show the path of not found files

### 2.5.4 (2023-06-29)
- added Adder8 module
- added Clocker module
- added CvRouter module
- added CvSwitcher module
- drumPlayerXtra: fixed a bug that crashes VCV when changing zoom without a sample loaded

### 2.5.3 (2023-05-24)

added sickoSampler module  

**sickoPlayer**  
- fixed bug on phasescan when scanning through silence  
- added context menu options about "EOC pulses"
- added trig/gate and stop buttons
- changed "Start Only" trig type to "Restart"

**all sample player modules**
- added "Disable Nav Buttons" option to general context menu

### 2.5.2 (2023-03-07)
added following modules:
- wavetabler
- drumPlayerXtra

**COMMON CHANGES**
- added subfolder navigation on all sample player modules  
- changed all time related knobs to exponential scale in all modules  

**drumPlayer**  
- added swap and copy slot option to slot context menu  

**drumPlayer+**  
- added text scrolling to sample name displays and a context menu option to disable it  
- added swap and copy slot option to slot context menu  

**sickoPlayer**  
- fixed bug when switching from poly to monophonic out  
- added polyphony to EOC/EOR outputs  
- added polyphony on master CV input  
- fixed some bugs on cursors repositioning  
- moved REV button to cursor knobs area. Now it selects next playback direction   
- added XFD knob and removed crossfade submenu from context menu  
- moved factory presets to module context menu  

### 2.5.1 (2023-01-15)
added following modules:  
- Drummer4+  
- SickoPlayer  

Added anti-aliasing filter to DrumPlayer and DrumPlayer+
Added polyphony to Blender and changed front panel  
Added attenuverters to Toggler and bToggler  
Changed time scale in ms to time knobs of all modules  

### 2.5.0 (2022-12-10)
added following modules:  
DrumPlayer  
DrumPlayer+  

### 2.4.1 (2022-11-25)
added Shifter module  
added "Warn Inversion" context menu feature on bToggler8+ module  
layout change on Parking module  

### 2.4.0 (2022-11-13)
added following modules:  
- Drummer
- Drummer4
- Parking

### 2.3.2 (2022-10-15)
modified state storage in context menu  
fixed 'state-storage' bug  
removed no-clock buggy feature from 'bToggler' family modules  

### 2.3.1 (2022-10-09)
changed trigger and gate input threshold from 0v to +1v on every module  
added modules state storage and context menu  
added leds on 'toggler' and 'bToggler' family modules  
added no-clock feature on 'bToggler' family modules  
fixed bug on 'bToggler8' when only a gate output is connected  
fixed bug on 'bToggler' when release time not working if reset is connected  
changed behaviour of some function types on 'switcher' family modules  
fixed bug on 'switcherSt' in swapper function  

### 2.3.0 (2022-10-02)
added following modules:
- switcher
- switcherSt

new graphic design

### 2.2.0 (2022-09-14)
added following modules
- blender
- blender8

renamed "bTogglerSt" to "bToggler"  
renamed "bTogglerSt compact" to "bToggler compact"  
renamed "bToggler" to "bToggler8"  
renamed "bToggler+" to "bToggler8+"  
fixed right channel issue input/output on "bToggler" and "toggler"  
minor graphic adjustments

### 2.1.0 (2022-09-03)
added following modules
- bTogglerSt
- bTogglerSt compact

### 2.0.0 (2022-08-07)
First release with following modules
- bToggler
- bToggler+
- calcs
- toggler
- toggler compact