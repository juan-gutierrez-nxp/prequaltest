/*
 * Copyright 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the Software Release Agreement or Pre-Release Software Agreement with Amazon
 * that governs use of this file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS.  AMAZON SPECIFICALLY DISCLAIMS,
 * WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY,
 * INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef WHA_ALSA_ALSA_H_
#define WHA_ALSA_ALSA_H_

#include <asoundlib.h>

#include <string>
#include <vector>

#include "WHADelegates.h"

class ALSA: public wha::PlaybackDelegate {
public:
    ALSA(const char* device, int rate, wha::TimeDelegate* td, wha::LoggingDelegate* logger);
    virtual ~ALSA();
    virtual int getPlaybackRate();
    virtual int start(wha::AudioFormat format, int channels);
    virtual void stop(wha::StopBehavior behavior);
    virtual int write(int64_t pts, const void* buffer, size_t* frames);

private:
    int getNextWriteTimestamp(int64_t* pts);
    int getSupportedChannelCount(snd_pcm_t* pcm, int channelCount);
    ssize_t writeInternal(size_t frames, const void* buf);
    ssize_t writePcm(size_t frames, const void* buffer);
    ssize_t writeSilence(size_t frames);
    void log(wha::LogLevel level, const char* fmt, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 3, 4)))
#endif
    ;

    const std::string mDevice;
    const int mRate;

    snd_pcm_t* mPcm;

    size_t mRequestedBpf;
    size_t mBpf;
    int mMixFactor;
    std::vector<char> mMixBuffer;

    std::vector<char> mSilence;

    int mSampleDeltaThreshold;

    wha::TimeDelegate* mTimeDelegate;
    wha::LoggingDelegate* mLogger;
};

#endif /* WHA_ALSA_ALSA_H_ */
