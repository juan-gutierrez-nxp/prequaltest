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

// misc
#include <unistd.h>
#include <stdio.h>
#include <ctime>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>

#include <iostream>
#include <fstream>
using namespace std;

// gtest
#include <gtest/gtest.h>

// Audio distribution
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <net/if.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <netinet/tcp.h>
#include <errno.h>
#include <arpa/inet.h>

#include "ALSA.h"
#include <math.h>

#include "Delegates.h"

#define GPRINTF1 printf
#define GPRINTF2 printf
#define GPRINTF printf

// =============================================================================
// GLOBAL: instantiate delegates
TimeDelegate gTimeDelegate;
DeviceInfoDelegate gDeviceInfoDelegate;
GPIODelegate gGPIODelegate(&gDeviceInfoDelegate);
LoggingDelegate gLoggingDelegate;
ALSA *gALSA = NULL; // default ALSA device for output, might also try "plug:dmix"

// Binary-only files only need to have pointers to the wha:: base classes
TimeDelegate            *pTimeDelegate       = &gTimeDelegate;
DeviceInfoDelegate      *pDeviceInfoDelegate = &gDeviceInfoDelegate;
GPIODelegate            *pGPIODelegate       = &gGPIODelegate;
wha::LoggingDelegate    *pLoggingDelegate    = &gLoggingDelegate;

// GLOBAL: parameters used for the AudioDistribution test
char **gSlaves = NULL;
int gNumSlaves = 0;

char *gDeviceName = NULL;
int gSampleRate = 48000;
int gNumPlacementTrials = 10;

char *gTimeMasterIP = NULL;

int gTimeSyncDuration = 10;

// =============================================================================
// HRT slow test --------------------------------------------------------------------------
TEST(HRT, SlowAccessTest) {
    GPRINTF1("Ensures that HRT increments at about the right rate (~1E9 ns/sec)\n");
    GPRINTF1("\n");

    GPRINTF2("HRT1_ns,HRT2_ns,delta_ns,result\n");

    unsigned long long KPI_MIN_ns = 1E9;
    unsigned long long KPI_MAX_ns = 1.0003E9;

    // test to make sure we can see the HRT moving, at about the right rate
    unsigned int failures = 0;
    unsigned int trials = 0;

    int64_t t1;
    int64_t t2;

    for (int i = 0; i < 25; i++) {
        int result1 = gTimeDelegate.getLocalTimeNs(&t1);
        sleep(1);
        int result2 = gTimeDelegate.getLocalTimeNs(&t2);

        if (result1 != 0 || result2 != 0) {
            GPRINTF2("BAD TIME: %d %d\n", result1, result2);
            failures++;
        } else {
            int64_t delta = t2 - t1;

            // HRT in ns must be approximately correct
            bool pass = delta > KPI_MIN_ns && delta < KPI_MAX_ns;
            failures += (pass ? 0 : 1);

            GPRINTF2("%llu,%llu,%llu,%s\n", t1, t2, delta, (pass ? "PASS" : "FAIL"));
        }
        trials++;
    }

    EXPECT_EQ(0, failures);
}

// HRT fast access test ----------------------------------------------------------------
TEST(HRT, FastAccessTest) {
    GPRINTF1("Ensures that the HRT can be accessed quickly (<=3us, 1 failure allowed)\n");
    GPRINTF1("\n");

    GPRINTF2("HRT1_ns,HRT2_ns,delta_ns,result\n");

    unsigned long long KPI_MIN_ns = 0;
    unsigned long long KPI_MAX_ns = 3000;

    // test to make sure we can access the HRT quickly (<1µs)
    unsigned int failures = 0;
    unsigned int trials = 0;
    int64_t t1, t2;
    for (int i = 0; i < 25; i++) {
        gTimeDelegate.getLocalTimeNs(&t1);
        gTimeDelegate.getLocalTimeNs(&t2);

        int64_t delta = t2 - t1;

        bool pass = delta >= KPI_MIN_ns && delta <= KPI_MAX_ns; // HRT delta must be small
        failures += (pass ? 0 : 1);
        trials++;

        GPRINTF2("%llu,%llu,%llu,%s\n", t1, t2, delta, (pass ? "PASS" : "FAIL"));
    }

    EXPECT_LE(failures, 1);  // we'll allow one failure, in case we got an interrupt in between,
    //    but this should be quite rare (therefore allow only one)
}

// GPIO slow access test ----------------------------------------------------------------
TEST(GPIO, SlowAccessTest) {
    GPRINTF1("Ensures that GPIO can be toggled\n");
    GPRINTF1("\n");
    GPRINTF1("Start recording on the logic analyzer.\n");

    // test to make sure we can toggle it, to see it on the logic analyzer
    for (int i = 0; i < 5; i++) {
        gGPIODelegate.setGPIO();
        GPRINTF2("GPIO high\n");
        sleep(1);
        gGPIODelegate.clearGPIO();
        GPRINTF2("GPIO low\n");
        sleep(1);
    }
    GPRINTF1("Stop recording on the logic analyzer.\n");
}

// GPIO fast access test ----------------------------------------------------------------
TEST(GPIO, FastAccessTest) {
    GPRINTF1("Ensures that the GPIO can be toggled quickly (<1us)\n");
    GPRINTF1("\n");
    GPRINTF1("Start recording on the logic analyzer.\n");
    GPRINTF2("GPIO transitioning 20 times.\n");

    // test to make sure we can toggle it with access time of less than 1 usec per toggle
    gGPIODelegate.toggleGPIO_10();
    GPRINTF1("Stop recording on the logic analyzer.\n");
    GPRINTF1("\n");
    GPRINTF2("TODO: Manually check the logic analyzer output.\n");
    GPRINTF2("      Slow pulses should have H/L time of ~1s.\n");
    GPRINTF2("      Fast pulses should have H/L time of <1us.\n");
    GPRINTF2("      Duration of fast pulse section should be <20us.\n");
}

// Audio pipeline drift test (run it manually) --------------------------------------------
//   or specify --gtest_filter=AudioPipeline.DriftTest
TEST(AudioPipeline, DriftTest) {
    GPRINTF1("Ensures that the audio pipeline outputs samples at the expected rate \n");
    GPRINTF2("TODO: Record audio output, check manually. See documentation for details.\n");

    // ***** TODO: ideally this should be in one of the delegates...
    char playCommand[] = "aplay ";
    char fileToPlay[] = "480Hz_2ch_30s.wav";
    char *command = (char *)malloc(strlen(playCommand) + strlen(fileToPlay) + 1);
    strcpy(command, playCommand);
    strcat(command, fileToPlay);  // e.g. "aplay 480Hz_30s_sine.wav"

    int ret = system(command);

    // let's provide a little more info to the user for the failure case
    int wret1 = WIFEXITED(ret);
    if (!wret1) {
        FAIL() << "could not execute playback function: " << wret1;
    } else {
        int wret2 = WEXITSTATUS(ret);
        // can only check this if we exited
        if (wret2 != 0) {
            FAIL() << "playback failure: " << wret2;
        }
    }
}

// AUDIO DISTRIBUTION =====================================================================
// UNICAST -----------------------
// Unicast port used for TCP for data in the UNICAST case: 1234 (not user selectable)
#define UNICASTDATAPORT (1234)
// number of bytes per write
#define BYTESPERBLOCK   (10*1024)

// UTILITY FUNCTIONS ==================================================
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Audio Distribution Master (run it manually) --------------------------------------------
//  one device: Master, one device: Slave
//
// To run:
//   on MASTER:      sudo ./preQualTest --gtest_filter=AudioDistribution.Master 192.168.0.<slave1> ... 192.168.0.<slaveN>
//   on each SLAVE:  sudo ./preQualTest --gtest_filter=AudioDistribution.Slave
TEST(AudioDistribution, Master) {

    GPRINTF("Audio Distribution Unicast MASTER\n");

    unsigned int killAfter = 10;  // 0 = no kill, else 10MByte

    // =========================================================
    // int sockfd;
    struct sockaddr_in servaddr;

    GPRINTF1("Slave devices:\n");
    for (int i = 0; i < gNumSlaves; i++) {
        GPRINTF2("Slave #%d: %s\n", i, gSlaves[i]);
    }

    int sockfds[32];  // up to 32 slaves max

    if (gNumSlaves < 1) {
        FAIL() << "ERROR -- Master: Must specify at least one Slave IP address.\n";
        return;
    }

    // connect to all receivers
    for (int i = 0; i < gNumSlaves; i++) {
        // OPEN SOCKET
        sockfds[i] = socket(AF_INET, SOCK_STREAM, 0);

        bzero(&servaddr, sizeof servaddr);

        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(UNICASTDATAPORT);

        // re-use already bound address/port (if possible)
        int optval = 1;
        if (setsockopt(sockfds[i], SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0) {
            GPRINTF("Cannot set SO_REUSEADDR option "
                    "on listen socket (%s)\n", strerror(errno));
            return;
        }
        inet_pton(AF_INET, gSlaves[i], &(servaddr.sin_addr));

        while (1) {
            GPRINTF1("Trying to connect to Slave #%d at '%s'...", i, gSlaves[i]);
            int r = connect(sockfds[i], (struct sockaddr *)&servaddr, sizeof(servaddr));
            if (r == 0) {
                GPRINTF2("CONNECTED.\n");
                break;  // success!
            }
            GPRINTF2("Could not connect: %d\n", r);
            usleep(2E6); // delay 2 seconds, then try again
        }

        GPRINTF1("master: now connected to %s on port %d....\n", gSlaves[i], UNICASTDATAPORT);
    }

    unsigned char buf[BYTESPERBLOCK];
    unsigned int bytesWritten = 0;

    while (1) {
        // write the same block out to each connected slave, in round robin fashion
        for (int j = 0; j < gNumSlaves; j++) {
            write(sockfds[j], &buf, sizeof(buf));
        }

        bytesWritten += sizeof(buf);

        // usleep(1E6);  // 1 second between sends for now

        if ((killAfter != 0) && (bytesWritten >= killAfter * 1024 * 1024)) { // Note: MB = 1024*1024 bytes (not 1E6)
            GPRINTF("DONE.\n");
            SUCCEED();  // MASTER ALWAYS SUCCEEDS
            return;
        }
    }

}

// Audio Distribution Slave (run it manually) --------------------------------------------
//  See run instructions above.
TEST(AudioDistribution, Slave) {

    GPRINTF("Audio Distribution Unicast: SLAVE\n");

    if (gNumSlaves > 0) {
        for (int i = 0; i < gNumSlaves; i++) {
            GPRINTF1("WARNING: extra argument ignored '%s'\n", gSlaves[i]);
        }
    }

    int new_fd;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];

#define BACKLOG 2     // how many pending connections queue will hold

    int listen_fd;
    struct sockaddr_in servaddr;

    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        FAIL() << "ERROR -- Slave: socket";
        return;
    }

    // re-use already bound address/port (if possible)
    int optval = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0) {
        GPRINTF("ERROR: Can't set SO_REUSEADDR option "
                "on listen socket (%s)\n", strerror(errno));
        FAIL() << "ERROR -- Slave: setsockopt";
        return;
    }

    bzero( &servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(UNICASTDATAPORT);

    if (bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        close(listen_fd);
        FAIL() << "ERROR -- Slave: bind";
        return;
    }

    // --------------------------
    if (listen(listen_fd, BACKLOG) == -1) {
        FAIL() << "ERROR -- Slave: listen";
        return;
    }

    GPRINTF1("slave: waiting for connections...\n");

    unsigned char inbuf[BYTESPERBLOCK];
    int nbytes = 0;
    unsigned int totalBytesReceived = 0;
    unsigned int incrStartBytesReceived = 0;
    unsigned int dotIncrement = 1E6;
    unsigned int nextDot = dotIncrement;

    double startIterationTime = 0.0;
    double startTestTime = 0.0;

    struct timeval tv;

    while (1) { // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(listen_fd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            GPRINTF2("ERROR -- slave: accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        GPRINTF1("slave: got connection from %s\n", s);

        GPRINTF1("Each report below = ~%d bytes received.\n", dotIncrement);
        GPRINTF1("incrMbps,cumuMbps\n");

        // -------------------------------------
        gettimeofday(&tv, NULL);
        startTestTime = startIterationTime = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;

        incrStartBytesReceived = 0;

        double lastCumuMbps = 0.0;  // last known value (also, value at completion)

        while (1) {
            if ((nbytes = recv(new_fd, &inbuf, sizeof inbuf, 0)) <= 0) {
                // GPRINTF1("\n");
                // GPRINTF1("TOTAL BYTES RECEIVED = %d\n", totalBytesReceived);
                // GPRINTF1("\n");
                GPRINTF1("slave: recv -- Master disconnected\n");

                // TEST PASSES OR FAILS HERE
                EXPECT_GE(lastCumuMbps, 2.0);  // KPI is 2.0MBPS in a normal office environment
                return;
            }

            totalBytesReceived += nbytes;
            if (totalBytesReceived > nextDot) {
                gettimeofday(&tv, NULL);
                double now = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;

                double incrMbps = 8 * ((totalBytesReceived - incrStartBytesReceived) / (1E6)) / (now - startIterationTime);

                incrStartBytesReceived = totalBytesReceived;
                startIterationTime = now;

                double cumuMbps = 8 * (totalBytesReceived / (1E6)) / (now - startTestTime);
                lastCumuMbps = cumuMbps;

                GPRINTF2("%.3f,%.3f,%0.3f\n", incrMbps, cumuMbps, totalBytesReceived);

                nextDot += dotIncrement;
            }
        }

        close(new_fd);  // parent doesn't need this
    }
}

// Audio pipeline placement test (run it manually) --------------------------------------------
//   on DUT:  sudo ./preQualTest --gtest_filter=AudioPipeline.PlacementTest
#define NUM_CHANNELS         2
#define BYTES_PER_SAMPLE     2
#define BYTES_PER_FRAME (BYTES_PER_SAMPLE*NUM_CHANNELS)

// delay for a specified number of usecs (spin-wait)
void us_delay(unsigned int d, TimeDelegate *td) {
    int64_t now;
    td->getLocalTimeNs(&now);
    int64_t done = now + (1000 * d);
    while (now < done) {
        td->getLocalTimeNs(&now);
    };
}

int write_audio(wha::PlaybackDelegate* hw, int64_t pts, const int16_t* data, int channels, int rate, int frames) {
    while (frames > 0) {
        size_t framesWritten = frames > 1024 ? 1024 : frames;     
        int err = hw->write(pts, data, &framesWritten);
        if (err) return err;
        if (framesWritten >= frames) break;

        frames -= framesWritten;
        data += channels * framesWritten;
        pts += 1000000000LL * framesWritten / rate;
    }
    return 0;
}

TEST(AudioPipeline, PlacementTest) {
    GPRINTF1("Causes device to generate GPIO and audio waveforms for placement analysis. \n");
    GPRINTF2("Run ./measurePlacement.sh to capture/analyze. See documentation for details.\n");
    GPRINTF2("sudo ./preQualTest --gtest_filter=\"AudioPipeline.PlacementTest\" to run this test on the DUT.\n");
    GPRINTF1("\n");

    if( gALSA == NULL ){
        GPRINTF2("ALSA object not initialized. Be sure you provide both device_name and sample_rate on command line\n");
        return;
    }

    int num_samples = gSampleRate/10;
    int sine_freq = gSampleRate/100;
    int samples_per_tone_period = gSampleRate/sine_freq;


    int16_t sinewave[NUM_CHANNELS * num_samples]; // signed 16-bit samples, interleaved L,R
    int16_t sinewave_with_silence[NUM_CHANNELS * num_samples * 2]; // signed 16-bit samples, interleaved L,R

    // make sine wave and sine wave with silence at end
    for (int i = 0; i < num_samples; i += 1) {
        sinewave[NUM_CHANNELS * i] 
        = sinewave[NUM_CHANNELS * i + 1] 
        = sinewave_with_silence[NUM_CHANNELS * i] 
        = sinewave_with_silence[NUM_CHANNELS * i + 1]
        = (int16_t)(0.8*SHRT_MAX*sin(i * 2.0 * M_PI / samples_per_tone_period));

        sinewave_with_silence[(NUM_CHANNELS * i) + (num_samples * NUM_CHANNELS)] 
        = sinewave_with_silence[(NUM_CHANNELS * i) + (num_samples * NUM_CHANNELS) + 1]
        = (int16_t)0;
    }

    //unsigned int framesToPlay = num_samples; //if using sinewave
    unsigned int framesToPlay = num_samples * 2; //if using sinewave_with_silence
    unsigned int bytesToPlay = BYTES_PER_FRAME * framesToPlay;

    // ----------------------------------
    // Wall clock start time of test ----
    time_t now;
    time(&now);
    char currentTimeUTC[sizeof "2017-10-28T00:00:00Z"];
    strftime(currentTimeUTC, sizeof currentTimeUTC, "%FT%TZ", gmtime(&now));

    int trials = gNumPlacementTrials;  
    
    GPRINTF1("Audio Placement Test, for '%s', run: %s, trials: %d\n",
           gDeviceInfoDelegate.getDeviceType(), currentTimeUTC, trials);
    GPRINTF1("\n");

    // ----------------------------------
    srand(12345);  // make trial sequence repeatable

    // warm up all caches, so the first accesses to HRT and GPIO are fast ------------
    int64_t warmup;
    gTimeDelegate.getLocalTimeNs(&warmup);
    gGPIODelegate.clearGPIO();

    // MAIN LOOP --------------------------------------------------------------------------------------------
    GPRINTF2("gpioQuality,trial,delta_us,leadingEdge_s\n");  // CSV header

    for (int trial = 0; trial < trials; trial++) {
	GPRINTF2("START trial %d\n",trial);

        gALSA->start(wha::AUDIO_FORMAT_PCM_S16, NUM_CHANNELS);  // give it time to warm up

        // ------------------------
        // timebox the leading edge
        int64_t before;
        int64_t after;
        gTimeDelegate.getLocalTimeNs(&before);
        gGPIODelegate.setGPIO();
        gTimeDelegate.getLocalTimeNs(&after);

        us_delay(150, &gTimeDelegate);  // pulse width of GPIO is 150µs (don't make it too short, and don't just toggle it)

        gGPIODelegate.clearGPIO();

        int64_t GPIO_leading_edge_ns = before + (after - before) / 2; // be careful not to overflow intermediate results

        // Play sine wave at exactly 1 second after the leading edge...
        // NOTE: the 150us GPIO width will NOT delay the playAtTime, because ALSA will call snd_pcm_delay() to figure out
        //   the actual delay required.
        int64_t playAtTime = GPIO_leading_edge_ns + (int64_t)1000000000;  // play at exactly this many seconds in the future

        int err = write_audio(gALSA, playAtTime, sinewave_with_silence, NUM_CHANNELS, gSampleRate, framesToPlay);
        if (err < 0) {
            FAIL() << "gALSA->write() error: " << err;
        }

        gALSA->stop(wha::STOP_DRAIN);

        // ---------------------
        // print after the tone burst, so as not to delay the playback, just invalidate the trial
        if ((after - before) > 10000) {
            GPRINTF2("BAD,%d,%f,%f\n", trial,
                   (after - before) / 1.0E3, GPIO_leading_edge_ns / 1.0E9);
        } else {
            GPRINTF2("GOOD,%d,%f,%f\n", trial,
                   (after - before) / 1.0E3, GPIO_leading_edge_ns / 1.0E9);
        }
	GPRINTF2("\n");
        fflush(stdout);

        // pseudo random delay before next trial ----------
        //   this makes sure that results aren't correlated with anything else on the device
        useconds_t us_to_sleep = 200E3 + 300.0 * (useconds_t)(1000.0 * (double)rand() / (double)RAND_MAX);
        usleep( us_to_sleep ); // random sleep between 0.2 and 0.5 seconds

    } // for each run

    // delete[] sinewave;
}

// MAIN =============================================================================
int
main(int argc, char *argv[]) {

    setpriority(PRIO_PROCESS, 0, -20);

    const char * IPaddr = gDeviceInfoDelegate.getIPAddress();  // print out my IP address

    // GTEST-based TESTS ----------------------------------------------------
    // Exclude the Audio Pipeline (Drift and Placement) and Audio Distribution tests, unless the user explicitly asks for them
    // NOTE: they will also NOT show up in the --gtest_list_tests output
    testing::GTEST_FLAG(filter) = "-AudioPipeline.DriftTest:AudioPipeline.PlacementTest:AudioDistribution.*:TimeSync.*";

    ::testing::InitGoogleTest(&argc, argv);

    int c;
    while( (c = getopt(argc, argv, "n:r:t:m:p:h")) != -1 ) {
        switch( c ) {
            case 'n':
                gDeviceName = optarg;
                break;
            case 'r':
                gSampleRate = atoi(optarg);
                break;
            case 't':
                gTimeSyncDuration = atoi(optarg);
                break;
            case 'm':
                gTimeMasterIP = optarg;
                break;
            case 'p':
                gNumPlacementTrials = atoi(optarg);

                break;
            case 'h':
            case '?':
                printf("Options:\n");
                printf(". -m <time_master_ip> (For Time Sync)\n");
                printf("  -n <device_name> (For Audio Placement)\n");
                printf("  -r <sample_rate> (For Audio Placement)\n");
                printf("  -t <duration_secs> (For Time Sync, default 10)\n");
		printf("  -p <placement_trials> (For Audio Placement)\n");
                // printf("  <slave_ip_1> ... <slave_ip_n> (space-delimited list, for Audio Placement)\n");
                printf("  -h (this)\n");
                return 0;
        }
    }

    // make additional args available to the Audio Distribution Master test, via Globals
    gNumSlaves = argc - 1;
    gSlaves = &argv[1];

    // if( gDeviceName == NULL ){
    //     gDeviceName = "hw:0,0";
    // }

    if( gDeviceName && gSampleRate ) {
        gALSA = new ALSA(gDeviceName, gSampleRate, &gTimeDelegate, &gLoggingDelegate);
    }

    // ------------
    time_t now;
    time(&now);
    char currentTimeUTC[sizeof "2017-10-28T00:00:00Z"];
    strftime(currentTimeUTC, sizeof currentTimeUTC, "%FT%TZ", gmtime(&now));

    GPRINTF("PreQualification for device: '%s', IPaddr: %s, run: %s\n",
            gDeviceInfoDelegate.getDeviceType(),
            gDeviceInfoDelegate.getIPAddress(),
            currentTimeUTC);
    GPRINTF("\n");

    return RUN_ALL_TESTS();

    // delete gALSA;

    // return result;
}
