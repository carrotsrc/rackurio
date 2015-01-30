#ifndef RUECHO_H
#define RUECHO_H
#include "framework/rack/RackUnit.h"
class RuEcho : public RackoonIO::RackUnit
{
	enum WorkState {
		IDLE,
		READY,
		PRIMING,
		RUNNING,
		BYPASS
	};

	FILE *fp;

	WorkState workState;
	unsigned int mDelay, sampleRate, bufSize;
	short *frameBuffer, *fDelay, *processedPeriod;
	unsigned int dLevel;
	void add(short*, int);
	bool remainder;
public:
	RuEcho();
	RackoonIO::FeedState feed(RackoonIO::Jack*);
	void setConfig(string,string);

	RackoonIO::RackState init();
	RackoonIO::RackState cycle();
};

#endif
