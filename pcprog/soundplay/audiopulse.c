//#include <pulse/simple.h>
//#include <pulse/error.h>
//#include <pulse/gccmacro.h>
#include <pulse/thread-mainloop.h>
#include <pulse/context.h>
#include <pulse/stream.h>
#include <pulse/sample.h>
#include <stdio.h>
#include <pthread.h>
//#include "audiopulse.h"

#define TMPBUFFERFRAMES 441*5

// NEEDS SHORT TO BE 16 bits!
//const unsigned int tmpbufferframes = 441*5;
short playwritebuf[TMPBUFFERFRAMES*2];
short* sndbuf = 0;
unsigned int sndbufFrames = 0;
double playSpeed = 0;
double oldPlaySpeed = 0;
double bufpos = 567603; // in frames..

pa_threaded_mainloop* g_audioLoop = 0;
pa_context* g_audioContext = 0;
pa_stream* g_audioStream = 0;

int g_context_state = 0;                    // state of context
//int g_output_channels = 0;                // number of channels in current output stream
//int g_output_rate = 0;                    // rate of current output stream
pa_sample_spec  g_streamSpec;               // the two above..

static pthread_mutex_t g_state_mutex;       // Synchronization mutex for the audio state output

pa_buffer_attr bufferSettings;



void setSndBufPtr(short* buf, unsigned int bufFrames)
{
	sndbuf = buf;
	sndbufFrames = bufFrames;
}

double setPlaySpeed(double spd)
{
	playSpeed = spd;
	return ((double)bufpos)/sndbufFrames;
}



// This callback gets called when our context changes state.  We really only
// care about when it's ready or if it has failed
void pa_context_state_cb(pa_context *c, void *userdata)
{
    pa_context_state_t state;

//    pa_threaded_mainloop_lock(g_audioLoop);
    state = pa_context_get_state(c);
//    pa_threaded_mainloop_unlock(g_audioLoop);

    pthread_mutex_lock(&g_state_mutex);
    switch  (state)
    {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
        default:
        break;
        case PA_CONTEXT_FAILED:
            printf("spot: Audio context failed\n");
        case PA_CONTEXT_TERMINATED:
            g_context_state = 2;
            printf("spot: Audio context disconnected\n");
        break;
        case PA_CONTEXT_READY:
            g_context_state = 1;
            printf("spot: Audio context ready\n");
        break;
    }
    pthread_mutex_unlock(&g_state_mutex);
}




void spotInitAudio()
{
    if (g_audioLoop == 0)
    {
        pthread_mutex_init(&g_state_mutex, NULL);
        g_audioLoop = pa_threaded_mainloop_new();
//        pa_threaded_mainloop_lock(g_audioLoop);

        g_audioContext = pa_context_new(pa_threaded_mainloop_get_api(g_audioLoop), "Velplay");
        pa_context_set_state_callback(g_audioContext, pa_context_state_cb, NULL);
        pa_context_connect(g_audioContext, NULL, 0, NULL);

//        pa_threaded_mainloop_unlock(g_audioLoop);
        pa_threaded_mainloop_start(g_audioLoop);
        printf("spot: Pulse loop started\n");
    }
}



void streamGetDataCallback(pa_stream* stream, size_t nbytes, void* userdata)
{
	//printf("Request for %d bytes of data\n", nbytes);
	unsigned int i;
	for (i = 0; i < TMPBUFFERFRAMES; ++i)
	{
		int intpos = bufpos;
		//printf("%d ", intpos);
		playwritebuf[2*i] = sndbuf[2*intpos];
		playwritebuf[2*i+1] = sndbuf[2*intpos+1];
		bufpos += playSpeed;
		if (bufpos > (sndbufFrames-4))
			bufpos = 10;
	}
	pa_stream_write(stream, playwritebuf, TMPBUFFERFRAMES*4, NULL, 0, PA_SEEK_RELATIVE);
	//int16_t frames[1764];
	//pa_stream_write(stream, frames, 441*pa_frame_size(&g_streamSpec), NULL, 0, PA_SEEK_RELATIVE);
}




void spotUnloadAudio()
{
    if (g_audioLoop != 0)
    {
        // TODO: end playing and stuff
        pa_threaded_mainloop_lock(g_audioLoop);

        if (g_audioStream != 0)
        {
            // close audio stream TODO:
            pa_stream_disconnect(g_audioStream);
            pa_stream_unref(g_audioStream);
            g_audioStream = 0;
        }

        pa_context_disconnect(g_audioContext);
        pa_context_unref(g_audioContext);
        g_audioContext = 0;

        pa_threaded_mainloop_unlock(g_audioLoop);

        pa_threaded_mainloop_stop(g_audioLoop);
        pa_threaded_mainloop_free(g_audioLoop);
        g_audioLoop = 0;
        printf("spot: Pulse loop stopped\n");
    }
}


unsigned int spotWritableFrames()
{
	if (g_audioStream == 0)
		return 0;

	pa_threaded_mainloop_lock(g_audioLoop);
	unsigned int retval = pa_stream_writable_size(g_audioStream)/pa_frame_size(&g_streamSpec);
	pa_threaded_mainloop_unlock(g_audioLoop);
	return retval;
}

unsigned long spotFramesInBuffer()
{
	if (g_audioStream == 0)
		return -1;

//	pa_operation* o = 
	pa_stream_update_timing_info(g_audioStream, NULL, NULL);
//	while (pa_operation_get_state(o) != PA_OPERATION_DONE)
//		;

	pa_threaded_mainloop_lock(g_audioLoop);
	const pa_timing_info* info = pa_stream_get_timing_info(g_audioStream);
	
	unsigned long retval = -1;
	if (info != 0)
		retval = (info->write_index - info->read_index)/pa_frame_size(&g_streamSpec);
	pa_threaded_mainloop_unlock(g_audioLoop);
	return retval;
}


int spotPlayAudio(const int channels, const int rate, const void *frames, int num_frames)
{
    size_t num_frames_t;
    size_t writeableFrames;

    num_frames_t = num_frames;
//    if (num_frames_t*channels > volCHBufferSize)   // limit to buffer size
//        num_frames_t = volCHBufferSize/channels;

    pthread_mutex_lock(&g_state_mutex);
    pa_threaded_mainloop_lock(g_audioLoop);
    if (g_audioLoop == 0 || g_audioContext == 0 || g_context_state != 1)
    {
        pa_threaded_mainloop_unlock(g_audioLoop);
        pthread_mutex_unlock(&g_state_mutex);
        printf("spot: Audio context not ready yet\n");
        return 0;
    }

    if (g_audioStream == 0 || g_streamSpec.channels != channels || g_streamSpec.rate != rate)
    {
        if (g_audioStream != 0)
        {
            pa_stream_disconnect(g_audioStream);
            pa_stream_unref(g_audioStream);
            g_audioStream = 0;
        }
        printf("spot: Creating stream, %d channels, %d s/sec\n", channels, rate);
        g_streamSpec.format = PA_SAMPLE_S16LE;
        g_streamSpec.channels = channels;
        g_streamSpec.rate = rate;

        bufferSettings.maxlength = TMPBUFFERFRAMES*2*channels*3; //(2*channels*rate*2)/20; //(uint32_t) -1;
        bufferSettings.tlength = TMPBUFFERFRAMES*2*channels;     // (2*channels*rate*1)/20; //(uint32_t) -1;
        bufferSettings.prebuf = TMPBUFFERFRAMES*2*channels;      //(2*channels*rate*1)/10; //(uint32_t) -1;
        bufferSettings.minreq =  TMPBUFFERFRAMES*2*channels;     //2*channels*rate/100; //(2*channels*rate*1)/200; //(uint32_t) -1;


        g_audioStream = pa_stream_new(g_audioContext, "Music!", &g_streamSpec, NULL);
	pa_stream_set_write_callback(g_audioStream, &streamGetDataCallback, NULL);
        pa_stream_connect_playback(g_audioStream, NULL, &bufferSettings, 0, NULL, NULL);           // TODO: stream attributes for buffer
    }

    if (pa_stream_get_state(g_audioStream) != PA_STREAM_READY)
    {
        pa_threaded_mainloop_unlock(g_audioLoop);
        pthread_mutex_unlock(&g_state_mutex);
        printf("spot: Audio stream not ready yet\n");
        return 0;
    }

    writeableFrames = pa_stream_writable_size(g_audioStream)/pa_frame_size(&g_streamSpec);
    //printf("wb: %d,  bpf: %d,  WA: %d,  AF: %d\n",pa_stream_writable_size(g_audioStream), pa_frame_size(&g_streamSpec), writeableFrames, num_frames_t);

    if (num_frames_t != 0)
    {
        if (writeableFrames > 0)
        {
            if (num_frames_t > writeableFrames)
                num_frames_t = writeableFrames;

//            size_t i;
//            for (i = 0; i < num_frames_t*channels; ++i)
//                volChBuffer[i] = (int16_t) (volume* ((int16_t*)frames)[i] );
            pa_stream_write(g_audioStream, frames, num_frames_t*pa_frame_size(&g_streamSpec), NULL, 0, PA_SEEK_RELATIVE);
        }
        else
        {
            //printf("NONE WRITTEN\n", num_frames_t);
            num_frames_t = 0;
        }

    }
    else
    {
        pa_stream_flush(g_audioStream, NULL, NULL);
        printf("spot: Flushed audio buffer\n");
    }

    pa_threaded_mainloop_unlock(g_audioLoop);
    pthread_mutex_unlock(&g_state_mutex);
    //printf("%d frames\n", num_frames_t);
    return num_frames_t;
}



void spotFlushAudioBuffer()
{
    if (g_audioStream != 0)
        pa_stream_flush(g_audioStream, NULL, NULL);
}



//void spotDrainAudioBuffer()
//{
//    if (g_audioStream != 0)
//        pa_stream_drain(g_audioStream, NULL, NULL);
//}


