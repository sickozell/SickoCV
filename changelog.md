### 2.6.0 (2023-12-xx)
- added sickoLooper3 and sickoLooper5 modules

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