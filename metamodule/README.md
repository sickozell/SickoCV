# **Sickozell plugin for 4ms metamodule**  

SickozellMM is a 4ms metamodule port of SickoCV plugin for VCV rack.

It is built to work with metamodule firmware v2

## Notes on sample players/recorders
On all sampler modules working on MM the size limit of samples is set to 5,760,000 samples, i.e.: 2 mins @ 48khz/mono or 1 min @48khz/stereo and equivalents with other samplerates.

### Memory usage:
Memory usage estimate is different between different modules, remember that all samples are stored in memory in float32 with 2x oversample.  

- player modules (sickoPlayer / drumPlayer / waveTabler):  
  RAM (MB) = (sample duration in secs) * (File sampleRate) * (channels) * 8 / 1,048,576
- recorder modules (sickoSampler / sickoSampler2 / keySampler):  
  RAM (MB) = (sample duration in secs) * (Rack working sampleRate) * (channels) * 16 / 1,048,576
- sickoLooper modules (always stereo):  
  RAM (MB) = (sample duration in secs) * (Rack working sampleRate) * 16 / 1,048,576



## **Consider donating**  
The work necessary to develop these modules required many hours of work and many sleepless nights.  
Sickozell plugin is and will always remain free, but if you find it useful, consider donating even just a coffee by following this [payPal](https://paypal.me/sickozell) link.  
Thanks.
