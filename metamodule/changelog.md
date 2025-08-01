### v0.19
- update to v2.7.4 vcv release
- added stepStation, trigStation, sampleDelay, cvMeter modules

### v0.18
- fixed some general bugs for MM

### v0.17
- doubled sample limits on all sampler modules
- fixed out of bound bugs on looper modules
- merged repository into VCV Rack repo

### v0.16
- all sampler modules: improved memory allocation management
- all sample player modules: reduced memory usage to 2x oversampling size
- slewer/slewer mini: inverted shapes on shape knob
- sickoSampler2: fixed a bug drawing the playhead outside the module

### v0.15
- build for firmware v2.0.0-rc1

### v0.14
- updated dr_wav library
- adMini / enverMini : added "LVL knob -> ENV out" option in context menu
- drumPlayerMk2 / drumPlayerMini : change accent level range to 0-200%
- stepSeq8x / trigSeq8x: adjusted RECL and STOR buttons placement

### v0.13
- build for v2.0-dev-13 firmware
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

### v0.12
added randLoops randLoops8 trigSeq8x modules

### v0.11
fixed and updated plugin-mm.json according to new firmware v2.0.0-dev-12

### v0.10
- stepSeq stepSeq+: fixed a bug when using reverse direction

### v0.9
Improved samples memory management on sickoSampler sickoSampler2 keySampler sickoLooper1 sickoLooperX sickoLooper3 sickoLooper5

### v0.8
Build for firmware v2.0.0-dev-12

### v0.7
changed the sample limit to 23Mb (about 1 minute @ 48Khz stereo) on sampler modules to avoid running out of memory

### v0.6
used metemodule-v2.0.0.0-dev-11.0 build
replaced 'light' versions of modules with regular versions
added sickoLooper5 module

### v0.5
Changed plugin name to SickoCV according to VCV to work with patches saved with Metamodule-Hub on VCV-Rack  
Removed Toggler module for conflict issue on metamodule simulator

### v0.4
added following modules:  
stepSeq  
trigSeq  

### v0.3
added following modules:  
sickoLooper1 (no load/save, no click)  
sickoLooperX (no load/save)  
sickoLooper3 (no load/save, no click)  
blender  
blender8  
fixed missing font for sickoQuant and sickoQuant4  

### v0.2
added sickoSampler2 with no load/save and click functionalities

### v0.1

Modules list:

adder8  
bGates  
bToggler  
bToggler Compact  
bToggler8  
calcs  
clocker2  
cvRouter  
cvSwitcher  
drummer  
drummer4  
drummer4+  
enver lite  
holder lite  
holder compact lite  
holder8 lite  
modulator  
modulator7  
modulator7 compact  
multiSwitcher lite  
multiSwitcher mono lite  
multiRouter lite  
multiRouter mono lite  
randLoops  
shifter  
sickoAmp  
sickoCrosser lite  
sickoCrosser4 lite  
sickoQuant lite  
sickoQuant4 lite  
simpleSeq4  
slewer lite  
switcher  
switcherSt  
switcher8  
toggler  
toggler compact  
