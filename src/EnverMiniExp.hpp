struct EnvMiniExpMsg {
	bool connectedToMaster = false;
	int chanTrig = 1;
	float volLevel = 0.f;
	float env[16] = {};
};