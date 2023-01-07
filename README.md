# SickoCVbeta
VCV Rack plugin modules (BETA TEST AREA)  
Compile or download binary for EVERY platform on the release page

## Current modules in beta testing:
### SickoPlayer v2.5.1-beta10
Wav sample player with v/oct, polyphony, tuning, envelope, loop, interpolation and anti-aliasing filter

### DrumPlayer DrumPlayer+ v2.5.1-beta10
Drum Sample Player with audio signal interpolation and anti-aliasing filter  

### Drummer Drummer4 v2.5.1-beta10
Accent and choke utility for drum modules lacking these features  

**to do list:** 
- nothing in queue

## changelog:  
beta10:
- DrumPlayer+: fixed loadsample bug in context menu  
- Drummer4: added choke functionality and changed panel, code optimization  
- Drummer: code optimization  

beta9:
DrumPlayer DrumPlayer+ improvements:  
- fixed minor loadsample bug  
- added a "sample loaded led" for each slot in DrumPlayer  
- added context menu in the led/trg area to manage sample in DrumPlayer  
- added context menu in the samplename area to manage sample in DrumPlayer+  

beta8:  
SickoPlayer improvements:  
- new context menu in the waveform area to manage sample

beta7:  
SickoPlayer improvements:  
- changed context menu in load sample area  
- fixed polyphonic output remembering voltages when sample is cleared  
- minor artwork changes on panel  

beta6:  
SickoPlayer improvements:  
- solved end loop clicks  
- fixed skip to start when trigType is play/pause  
- fixed wrong knob autopositioning when in Phasescan mode  
- limited range for cue start/end and loop start/end knobs  

beta5:  
SickoPlayer improvements:  
- reduced clicks when using one cycle waveforms  
- added presets  

beta4:  
SickoPlayer improvements:  
- Added "Phase Scan" option in context menu
- Now working with one cycle waveforms
- Fixed crashing sample load bug
- Improved display monitoring on polyphonic use  

beta3:
- added polyphony to SickoPlayer and improved cpu load on v/oct input and tuning  
- improved response on vcv sample rate change

beta2:
- added SickoPlayer module  

beta1:
- added new out modes to DrumPlayer and DrumPlayer+  

beta:
- added anti-aliasing filter to DrumPlayer and DrumPlayer+  

## IMPORTANT NOTE
if you don't use development environment and run regular VCV install, the new modules will be shown up only if you have a full subscription to Sickozell plugin modules.  
So if you have added only single modules to VCV you will not see the ones that are not currently listed in the library.  
Please check your subscription on https://library.vcvrack.com/plugins and look for the Sickozell line that has to be like this:  
![subscription](https://user-images.githubusercontent.com/80784296/207971796-96163a4b-6fa9-4073-bda8-9df1e61f900b.JPG)
