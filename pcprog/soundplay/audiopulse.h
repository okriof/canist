#ifndef SPOTAUDIOPULSE_H
#define SPOTAUDIOPULSE_H
//#include <stdio.h>

 extern "C" { // NEEDS SHORT TO BE 16 bits!

void setSndBufPtr(short* buf, unsigned int bufFrames);
double setPlaySpeed(double spd);

void spotInitAudio();

void spotUnloadAudio();

unsigned int spotWritableFrames();
unsigned long spotFramesInBuffer();

int spotPlayAudio(const int channels, const int rate, const void *frames, int num_frames);
void spotFlushAudioBuffer();
}
#endif
