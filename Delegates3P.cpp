#include "Delegates.h"

// =============================================================================
// TimeDelegate
// =============================================================================
TimeDelegate::TimeDelegate() {
    onNewSkewAndDriftAt(0, 0.0, 0.0);  // init
}

int TimeDelegate::getLocalTimeNs(int64_t* localTime) {
    // TODO
    return 0;
}

double TimeDelegate::getNominalDriftPpm() {
    // TODO
    return 0.0;
}

double TimeDelegate::getDriftTolerancePpm() {
    // TODO
    return 10.0;
}

double TimeDelegate::getAudioDriftPpm() {
    // TODO
    return 0.0;
}

int TimeDelegate::getPlayOffsetNs(int64_t* offset) {
    // TODO
    return 0;
}


// =============================================================================
// DeviceInfoDelegate
// =============================================================================
const char* DeviceInfoDelegate::getIPAddress() {
    mIP = "unknown";
    // TODO
    return mIP.c_str();
}

const char* DeviceInfoDelegate::getBSSID() {
    // TODO
    return "unknown";
}

const char* DeviceInfoDelegate::getESSID() {
    // TODO
    return "unknown";
}

float DeviceInfoDelegate::getRSSI() {
    // TODO
    return -99.9;
}

const char* DeviceInfoDelegate::getDeviceType() {
    mModelID = "unknown";
    // TODO
    return mModelID.c_str();
}

int DeviceInfoDelegate::getDeviceMainVolume() {
    // TODO
    return 0;
}

int DeviceInfoDelegate::getDeviceIdleTime() {
    // TODO
    return 0;
}

const char* DeviceInfoDelegate::getDeviceId() {
    mSerialNum = "unknown";
    // TODO
    return mSerialNum.c_str();
}

bool DeviceInfoDelegate::isDebugBuild() {
    return false;
}

bool DeviceInfoDelegate::getRegistrationState() {
    return false;
}


// =============================================================================
// GPIODelegate
// =============================================================================
class GPIODelegateImpl3P : public GPIODelegateImpl {
public:
    GPIODelegateImpl3P(DeviceInfoDelegate *deviceInfoDel) {
        // TODO
        clearGPIO();    // and initially clear it
    }

    inline virtual void setGPIO() {
        // TODO
    }

    inline virtual void clearGPIO() {
        // TODO
    }

    inline virtual void toggleGPIO(int num_toggles) {
        // TODO
    }
};

GPIODelegate::GPIODelegate(DeviceInfoDelegate *deviceInfoDel) {
    impl = new GPIODelegateImpl3P(deviceInfoDel);
}
