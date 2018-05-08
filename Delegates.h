// WHA
#include "WHA.h"
#include "WHADelegates.h"

#include <string>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

class TimeDelegate : public wha::TimeDelegate {
public:
    TimeDelegate();

    virtual int getLocalTimeNs(int64_t* localTime);
    virtual double getNominalDriftPpm();
    virtual double getDriftTolerancePpm();
    virtual double getAudioDriftPpm();
    virtual int getPlayOffsetNs(int64_t* offset);
    inline virtual void onNewSkewAndDriftAt(int64_t time, double skew, double drift) {
        atCommonTimeNs = time;
        skewFromCommonNs = skew;
        driftFromCommonPPM = drift;
    }
    inline virtual int getCommonTimeNs(int64_t *commonTime) {
        // S = ppm * 1e-6 * (C - C0) + S0, S = C - L
        // C - L = ppm * 1e-6 * (C - C0) + S0
        // C * (1 - ppm * 1e-6) = L + S0 - ppm * 1e-6 * C0
        // C = (L + S0 - ppm * 1e-6 * C0) / (1 - ppm * 1e-6)
        // C = (L + S0) / (1 - ppm * 1e-6) + (C0 - C0 - ppm * 1e-6 * C0) / (1 - ppm * 1e-6)
        // C = (L + S0 - C0) / (1 - ppm * 1e-6) + C0
        int64_t localTimeNs;
        int err = getLocalTimeNs(&localTimeNs);
        if (err) return err;

        int64_t commonTimeNs = (int64_t)(((double)(localTimeNs + skewFromCommonNs - atCommonTimeNs)) /
                                         (1.0 - driftFromCommonPPM * 1e-6)) +
                               atCommonTimeNs;

        *commonTime = commonTimeNs;
        return 0;
    }

private:
    int64_t atCommonTimeNs;         // at this common time,
    double  skewFromCommonNs;       //   the skew is this
    double  driftFromCommonPPM;     //   and the drift is this
};

class DeviceInfoDelegate : public wha::DeviceInfoDelegate {
public:
    virtual const char* getIPAddress();
    virtual const char* getBSSID();
    virtual const char* getESSID();
    virtual float getRSSI();
    virtual const char* getDeviceType();
    virtual int getDeviceMainVolume();
    virtual int getDeviceIdleTime();
    virtual const char* getDeviceId();
    virtual bool isDebugBuild();
    virtual bool getRegistrationState();
    virtual const char* getAmazonId();
    virtual bool waitForAccurateSystemClock();

private:
    std::string mIP;
    std::string mSerialNum;
    std::string mModelID;
};

// Implement a child of this class to provide implementation
class GPIODelegateImpl {
public:
    GPIODelegateImpl(){}
    virtual ~GPIODelegateImpl(){}

    virtual void setGPIO() = 0;
    virtual void clearGPIO() = 0;

    inline virtual void toggleGPIO(int num_toggles) {
        int idx;
        for( idx = 0; idx < num_toggles; ++idx ) {
            setGPIO();
            clearGPIO();
        }
    }

    inline virtual void toggleGPIO_10() {
        setGPIO();
        clearGPIO();
        setGPIO();
        clearGPIO();
        setGPIO();
        clearGPIO();
        setGPIO();
        clearGPIO();
        setGPIO();
        clearGPIO();
        setGPIO();
        clearGPIO();
        setGPIO();
        clearGPIO();
        setGPIO();
        clearGPIO();
        setGPIO();
        clearGPIO();
        setGPIO();
        clearGPIO();
    }
};

// Interface class, do not implement or derive from
class GPIODelegate {
private:
    GPIODelegateImpl *impl;

public:
    GPIODelegate(DeviceInfoDelegate *deviceInfoDel);
    ~GPIODelegate() {
        delete impl;
    }

    inline void setGPIO() {
        impl->setGPIO();
    }
    inline void clearGPIO() {
        impl->clearGPIO();
    }
    inline void toggleGPIO(int num_toggles) {
        impl->toggleGPIO(num_toggles);
    }
    inline void toggleGPIO_10() {
        impl->toggleGPIO_10();
    }
};

class LoggingDelegate : public wha::LoggingDelegate {
public:
    LoggingDelegate(){
    }

    virtual void log(wha::LogLevel level, const char* tag, const char* text) {
        printf("%s %s\n", tag, text);
    }
};
