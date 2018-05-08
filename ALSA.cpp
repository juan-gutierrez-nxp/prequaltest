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

#include "ALSA.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

//#define VERBOSE_LOGGING 1
//#define VERY_VERBOSE_LOGGING 1

namespace {
const int kPeriodSizeInFrames = 1024; // 23220 us at 44100 rate
                                      // 21333 us at 48000 rate
const int kInitialSampleDeltaThreshold = 0;
const int kSampleDeltaThreshold = 192; // 4 ms or so

} // namespace

ALSA::ALSA(const char* device, int rate, wha::TimeDelegate* timeDelegate, wha::LoggingDelegate* logger):
    mDevice(device),
    mRate(rate),
    mPcm(NULL),
    mRequestedBpf(0),
    mBpf(0),
    mMixFactor(1),
    mSampleDeltaThreshold(0),
    mTimeDelegate(timeDelegate),
    mLogger(logger) {
}

ALSA::~ALSA() {
    if (mPcm) {
        snd_pcm_close(mPcm);
    }
}

void ALSA::log(wha::LogLevel level, const char* fmt, ...) {
    char msg[256];
    va_list ap;
    va_start(ap, fmt);
    size_t size = vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
    if (size > 0) {
        mLogger->log(level, "ALSA", msg);
    }
}

int ALSA::getPlaybackRate() {
    return mRate;
}

int ALSA::getSupportedChannelCount(snd_pcm_t* pcm, int channelCount) {
    snd_pcm_hw_params_t *params;
    snd_pcm_hw_params_alloca(&params);
    int err = snd_pcm_hw_params_any(pcm, params);
    if (err) {
        log(wha::LOG_ERROR, "snd_pcm_hw_params_any failed: %s", snd_strerror(err));
        return 0;
    }

    if (!snd_pcm_hw_params_test_channels(pcm, params, channelCount)) {
        // We support the exact number of channels requested
        return channelCount;
    }

    if (channelCount == 1) {
        unsigned int min;
        err = snd_pcm_hw_params_get_channels_min(params, &min);
        if (err) {
            log(wha::LOG_ERROR, "snd_pcm_hw_params_get_channels_min failed: %s", snd_strerror(err));
            return 0;
        }
        // Upmix 1 -> min
        return min;
    }

    if (!snd_pcm_hw_params_test_channels(pcm, params, 1)) {
        // Downmix channelCount -> 1
        return 1;
    }

    // Everything else is not supported
    return 0;
}

int ALSA::start(wha::AudioFormat format, int channels) {

    log(wha::LOG_INFO, "start: format %d channels %d", format, channels);

    snd_pcm_format_t alsaFormat;
    size_t sampleSize = 0;
    switch (format) {
    case wha::AUDIO_FORMAT_PCM_S16:
        alsaFormat = SND_PCM_FORMAT_S16;
        sampleSize = sizeof(int16_t);
        break;
    default:
        log(wha::LOG_ERROR, "Unsupported format: %d", format);
        return -EINVAL;
    }

    snd_pcm_t* pcm;
    int err = snd_pcm_open(&pcm, mDevice.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    if (err) {
        log(wha::LOG_ERROR, "snd_pcm_open failed: %s", snd_strerror(err));
        return err;
    }

    int channelsSupported = getSupportedChannelCount(pcm, channels);
    log(wha::LOG_DEBUG, "start: channels requested %d channels supported %d", channels, channelsSupported);
    if (channelsSupported <= 0) {
        log(wha::LOG_ERROR, "Unsupported channel count: %d", channels);
        snd_pcm_close(pcm);
        return -EINVAL;
    }

    err = snd_pcm_set_params(pcm,
                             alsaFormat,
                             SND_PCM_ACCESS_RW_INTERLEAVED,
                             channelsSupported,
                             mRate,
                             true,
                             kPeriodSizeInFrames * 1000000 / mRate * 4); // buffer size in usec
    if (err) {
        log(wha::LOG_ERROR, "snd_pcm_set_params failed: %s", snd_strerror(err));
        snd_pcm_close(pcm);
        return err;
    }

    err = snd_pcm_prepare(pcm);
    if (err) {
        log(wha::LOG_ERROR, "snd_pcm_prepare failed: %s", snd_strerror(err));
        snd_pcm_close(pcm);
        return err;
    }

    mPcm = pcm;
    mRequestedBpf = sampleSize * channels;
    mBpf = sampleSize * channelsSupported;
    if (mRequestedBpf > mBpf) {
        mMixFactor = mRequestedBpf / mBpf;
    } else if (mRequestedBpf < mBpf) {
        mMixFactor = mBpf / mRequestedBpf;
    } else {
        mMixFactor = 1;
    }

    return 0;
}

void ALSA::stop(wha::StopBehavior behavior) {
    if (!mPcm) {
        return;
    }

    int err = 0;
    if (behavior == wha::STOP_DROP) {
        log(wha::LOG_INFO, "stop: drop");
        err = snd_pcm_drop(mPcm);
        if (err) {
            log(wha::LOG_ERROR, "snd_pcm_drop failed: %s", snd_strerror(err));
        }
    } else {
        log(wha::LOG_INFO, "stop: drain");
        err = snd_pcm_drain(mPcm);
        if (err) {
            log(wha::LOG_ERROR, "snd_pcm_drain failed: %s", snd_strerror(err));
        }
    }

    err = snd_pcm_close(mPcm);
    if (err) {
        log(wha::LOG_ERROR, "snd_pcm_close failed: %s", snd_strerror(err));
    }

    mPcm = NULL;
}

int ALSA::getNextWriteTimestamp(int64_t* pts) {
    int written = 0;
    for(;;) {
        snd_pcm_state_t pcm_state = snd_pcm_state(mPcm);
        if (pcm_state == SND_PCM_STATE_RUNNING || pcm_state == SND_PCM_STATE_DRAINING) {
            break;
        }
#if VERBOSE_LOGGING
        log(wha::LOG_VERBOSE, "pcm_state = %d", pcm_state);
#endif
        ssize_t ret = writeSilence(kPeriodSizeInFrames);
        if (ret < 0) {
            return ret;
        }
        written += ret;
    }
    if (written > 0) {
        log(wha::LOG_DEBUG, "written %d frames before snd_pcm_delay", written);
    }

    for (;;) {
        int64_t t0, t1;
        mTimeDelegate->getLocalTimeNs(&t0);
        snd_pcm_sframes_t delay = 0;
        int err = snd_pcm_delay(mPcm, &delay);
        mTimeDelegate->getLocalTimeNs(&t1);
        if (err) {
            log(wha::LOG_ERROR, "snd_pcm_delay failed: %s", snd_strerror(err));
            return err;
        }

        int64_t nsecDiff = t1 - t0;
        if (nsecDiff > 200000) { // 200 usec
            log(wha::LOG_WARN, "snd_pcm_delay took too long: %" PRId64 " ns, ignoring", nsecDiff);
            continue;
        }

        int64_t nsecDelay = 1000000000LL * delay / mRate;
        *pts =  t0 + nsecDiff / 2 + nsecDelay;
        return 0;
    }

    // unreachable
    return -ENOENT;
}

int ALSA::write(int64_t bufferPts, const void* buffer, size_t* frames) {
    if (*frames == 0) {
        // WHA has no data, write silence to prevent pipeline stall
        log(wha::LOG_WARN, "underrun");
        ssize_t res = writeSilence(kPeriodSizeInFrames);
        return res < 0 ? res : 0;
    }

    int64_t pts;
    int status = getNextWriteTimestamp(&pts);
    if (status) {
        // cannot get next timestamp
        *frames = 0;
        return status;
    }

    int64_t bufferDuration = 1000000000LL * (*frames) / mRate;
    int64_t timeDelta = bufferPts - pts;

#if VERY_VERBOSE_LOGGING
    log(wha::LOG_VERBOSE,
            "pts %" PRId64  " bufferPts %" PRId64 " bufferDuration %" PRId64 " timeDelta %" PRId64,
            pts, bufferPts, bufferDuration, timeDelta);
#endif

    if (timeDelta <= -bufferDuration) {
        // the whole buffer is in the past, drop it
#if VERBOSE_LOGGING
        log(wha::LOG_VERBOSE, "drop the whole buffer");
#endif
        mSampleDeltaThreshold = kInitialSampleDeltaThreshold;
        return 0;
    }
    if (timeDelta >= bufferDuration) {
        // buffer is in the future - write silence
#if VERBOSE_LOGGING
        log(wha::LOG_VERBOSE, "write silence");
#endif
        mSampleDeltaThreshold = kInitialSampleDeltaThreshold;
        writeSilence(*frames);
        *frames = 0;
        return 0;
    }

    // we do not expect overflow as timeDelta should be pretty small by that time
    int64_t sampleDelta = timeDelta * mRate / 1000000000LL;
    ssize_t written = 0;

    if (sampleDelta <= mSampleDeltaThreshold && sampleDelta >= -mSampleDeltaThreshold) {
        // we are on time - write the whole buffer
#if VERY_VERBOSE_LOGGING
        log(wha::LOG_VERBOSE, "on time. sampleDelta %" PRId64, sampleDelta);
#endif
        written = writeInternal(*frames, buffer);
    } else if (sampleDelta > 0) {
        // buffer is in the future - write silence, then buffer
#if VERBOSE_LOGGING
        log(wha::LOG_VERBOSE, "write silence, then buffer");
#endif
        status = writeSilence(sampleDelta);
        if (status < 0) {
            *frames = 0;
            return status;
        }
        written = writeInternal(*frames, buffer);
    } else {
        // part of the buffer is in the past - strip some data and write
#if VERBOSE_LOGGING
        log(wha::LOG_VERBOSE, "trim and write buffer");
#endif
        size_t offsetInFrames = -sampleDelta;
        if (offsetInFrames >= *frames) {
            // somehow the whole buffer is in the past, drop it
            mSampleDeltaThreshold = kInitialSampleDeltaThreshold;
            return 0;
        }
        written = writeInternal(*frames - offsetInFrames,
                                static_cast<const char*>(buffer) + offsetInFrames * mRequestedBpf);
        if (written >= 0) {
            written += offsetInFrames;
        }
    }

    if (written <= 0) {
        *frames = 0;
        return written;
    }

    *frames = written;
    mSampleDeltaThreshold = kSampleDeltaThreshold;
    return 0;
}

ssize_t ALSA::writeInternal(size_t frames, const void* buf) {
    if (mRequestedBpf == mBpf) {
        // no upmix/downmix
        return writePcm(frames, buf);
    }

    if (mMixBuffer.size() < frames * mBpf) {
        mMixBuffer.resize(frames * mBpf);
    }

    const int16_t* src = static_cast<const int16_t*>(buf);
    int16_t* dst = reinterpret_cast<int16_t*>(mMixBuffer.data());

    if (mRequestedBpf > mBpf) {
        // downmix
        for (size_t i = 0; i < frames; ++i) {
            int16_t value = 0;
            for (int j = 0; j < mMixFactor; ++j) {
                value += *src++;
            }
            *dst++ = value / mMixFactor;
        }
    } else {
        // upmix
        for (size_t i = 0; i < frames; ++i) {
            int16_t value = *src++;
            for (int j = 0; j < mMixFactor; ++j) {
                *dst++ = value;
            }
        }
    }

    return writePcm(frames, mMixBuffer.data());
}

ssize_t ALSA::writePcm(size_t frames, const void* buf) {
    for (;;) {
        snd_pcm_sframes_t written = snd_pcm_writei(mPcm, buf, frames);
        if (written == -EAGAIN) {
            // try to write again
            continue;
        }
        if (written < 0) {
            // try to recover
            written = snd_pcm_recover(mPcm, written, 0);
        }
        if (written < 0) {
            // failed to recover
            log(wha::LOG_ERROR, "snd_pcm_write failed: %s", snd_strerror(written));
            return written;
        }
        return written;
    }

    // unreachable
    return -ENOENT;
}

ssize_t ALSA::writeSilence(size_t frames) {
    if (mSilence.size() < frames * mBpf) {
        mSilence.resize(frames * mBpf);
    }

    return writePcm(frames, mSilence.data());
}

