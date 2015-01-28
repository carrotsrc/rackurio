#include "RuAlsa.h"
using namespace RackoonIO;

RuAlsa::RuAlsa()
: RackUnit() {
	addJack("audio", JACK_SEQ);
	workState = IDLE;
	sampleRate = 44100;
	mLatency = 5;
	bufSize = 2048;
	bufLevel = 0;
}

RackoonIO::FeedState RuAlsa::feed(RackoonIO::Jack *jack) {
	Jack *j = getJack("audio");
	short *period;
	int bytes;
	if(j->flush(&period) == FEED_OK) {

	}

	return FEED_OK;
}

void RuAlsa::setConfig(string config, string value) {
	if(config == "period") {
		mLatency = atoi(value.c_str());
		fPeriod = (sampleRate/1000)*mLatency;
		cout << "Latency period: " << fPeriod << endl;
	} else if(config == "buffer") {
		bufSize = atoi(value.c_str());
		frameBuffer = (short*)malloc(sizeof(short)*bufSize);
		if(frameBuffer == NULL)
			cerr << "Failed to allocate frame buffer" << endl;

		cout << "Buffer len: " << bufSize << " frames" << endl << "Buf Size: " << bufSize*(sizeof(short)) << " bytes" << endl;
	}
}

void RuAlsa::actionInitAlsa() {
	snd_pcm_hw_params_t *hw_params;
	int err, dir;
	unsigned int srate = 44100;
	snd_pcm_uframes_t nframes = 4096;

	if ((err = snd_pcm_open (&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		cerr << "cannot open audio device `default` - "
			<< snd_strerror(err) <<  endl;
		return;
	}
  
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		cerr << "cannot allocated hardware param struct - "
			<< snd_strerror(err) <<  endl;
		return;
	}

	if ((err = snd_pcm_hw_params_any (handle, hw_params)) < 0) {
		cerr << "cannot init hardware param struct - "
			<< snd_strerror(err) <<  endl;
		return;
	}

//	if ((err = snd_pcm_hw_params_set_access (handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
	if ((err = snd_pcm_hw_params_set_access (handle, hw_params, SND_PCM_ACCESS_MMAP_INTERLEAVED)) < 0) {
		cerr << "cannot set access type - "
			<< snd_strerror(err) <<  endl;
		return;
	}

	if ((err = snd_pcm_hw_params_set_period_size_max (handle, hw_params, &nframes, &dir)) < 0) {
		cerr << "cannot set period max - "
			<< snd_strerror(err) <<  endl;
		return;
	}

	if ((err = snd_pcm_hw_params_set_format (handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		cerr << "cannot set format - "
			<< snd_strerror(err) <<  endl;
		return;
	}

	if ((err = snd_pcm_hw_params_set_rate_near (handle, hw_params, &srate, &dir)) < 0) {
		cerr << "cannot set sample rate - "
			<< snd_strerror(err) <<  endl;
		return;
	}

	if ((err = snd_pcm_hw_params_set_channels (handle, hw_params, 2)) < 0) {
		cerr << "cannot set channels - "
			<< snd_strerror(err) <<  endl;
		return;
	}

	if ((err = snd_pcm_hw_params (handle, hw_params)) < 0) {
		cerr << "cannot set parameters - "
			<< snd_strerror(err) <<  endl;

		return;
	}

	snd_pcm_hw_params_free (hw_params);

	if ((err = snd_pcm_prepare (handle)) < 0) {
		cerr << "cannot prepare audio interface - "
			<< snd_strerror(err) <<  endl;
		return;
	}

	triggerLevel = snd_pcm_avail(handle);
	triggerLevel -= (fPeriod<<1);

	/*snd_async_add_pcm_handler(&pcm_callback, handle, RuAlsaCallback,
			(void*)(new std::function<void()>(std::bind(&RuAlsa::asyncCallback, this))));*/
	cout << "RuAlsa: Initialised" << endl;
	
	workState = READY;
}

RackoonIO::RackState RuAlsa::init() {
	workState = INIT;
	outsource(std::bind(&RuAlsa::actionInitAlsa, this));
	return RACK_UNIT_OK;
}

RackoonIO::RackState RuAlsa::cycle() {

	if(workState == STREAMING) {
		return RACK_UNIT_OK;
	}

	if(workState < READY)
		return RACK_UNIT_OK;

	if(workState == READY)
		workState = STREAMING;


	return RACK_UNIT_OK;
}

/*void RuAlsa::asyncCallback() {
	cout << "ready to be written: " << frames_to_deliver << endl;
}

void RuAlsaCallback(snd_async_handler_t *pcm_callback) {
	// run callback
	(*(std::function<void()>*)snd_async_handler_get_callback_private(pcm_callback))();
}*/
