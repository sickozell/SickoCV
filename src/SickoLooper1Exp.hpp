struct Sl1ExpMsg {
	bool connectedToMaster = false;
	int expConnected = 0;
	int globalStatus = 0;
	int runSetting = 0;
	bool barPulse = false;
	double barSample = 0;
	bool reset = false;
	bool ledLight = false;
	int ssAll = 0;
	int busyTracks = 0;
	int recordedTracks = 0;
	bool detectTempo = false;
};