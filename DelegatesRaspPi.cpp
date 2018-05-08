#include "Delegates.h"

// =============================================================================
// TimeDelegate
// =============================================================================
TimeDelegate::TimeDelegate() {
    onNewSkewAndDriftAt(0, 0.0, 0.0);  // init
}

int TimeDelegate::getLocalTimeNs(int64_t* localTime) {
    struct timespec t;
    int status = clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    if (status) return status;

    *localTime = 1000000000LL * t.tv_sec + t.tv_nsec;
    return 0;
}

double TimeDelegate::getNominalDriftPpm() {
    return 0.0;
}

double TimeDelegate::getDriftTolerancePpm() {
    return 10.0;
}

double TimeDelegate::getAudioDriftPpm() {
    return 0.0;
}

int TimeDelegate::getPlayOffsetNs(int64_t* offset) {
    return 0;
}


// =============================================================================
// DeviceInfoDelegate
// =============================================================================
const char* DeviceInfoDelegate::getIPAddress() {
    mIP = "unknown";

    FILE *pipe = popen("ip route get 8.8.8.8 | awk '{printf $NF; exit}'", "r");
    if (pipe) {
        char buffer[16];
        if (fgets(buffer, sizeof(buffer) - 1, pipe)) {
            mIP = buffer;
        }
        pclose(pipe);
    }

    return mIP.c_str();
}

const char* DeviceInfoDelegate::getBSSID() {
    return "unknown";
}

const char* DeviceInfoDelegate::getESSID() {
    return "unknown";
}

float DeviceInfoDelegate::getRSSI() {
    return -99.9;
}

const char* DeviceInfoDelegate::getDeviceType() {
    mModelID = "unknown";
    FILE *pipe = popen("cat /proc/device-tree/model", "r");
    if (pipe) {
        char buffer[32];
        if (fgets(buffer, sizeof(buffer) - 1, pipe)) {
            mModelID = buffer;
        }
        pclose(pipe);
    }
    return mModelID.c_str();
}

int DeviceInfoDelegate::getDeviceMainVolume() {
    return 0;
}

int DeviceInfoDelegate::getDeviceIdleTime() {
    return 0;
}

const char* DeviceInfoDelegate::getDeviceId() {
    mSerialNum = "unknown";
    FILE *pipe = popen("cat /proc/device-tree/serial-number", "r");
    if (pipe) {
        char buffer[32];
        if (fgets(buffer, sizeof(buffer) - 1, pipe)) {
            mSerialNum = buffer;
        }
        pclose(pipe);
    }
    return mSerialNum.c_str();
}

const char* DeviceInfoDelegate::getAmazonId() {
    return getDeviceId();
}

bool DeviceInfoDelegate::isDebugBuild() {
    return false;
}

bool DeviceInfoDelegate::getRegistrationState() {
    return false;
}

bool DeviceInfoDelegate::waitForAccurateSystemClock() {
    return true; // TODO: is this safe?
}


// =============================================================================
// GPIODelegate
// =============================================================================
#define GPIO_TO_USE  9

// For Raspberry Pi 2 and 3, use 0x3F000000
#define BCM2708_BASE_PI2_PI3 0x3F000000

// For Raspberry Pi Zero and Zero W, use 0x20000000
#define BCM2708_BASE_ZERO 0x20000000

#define GPIO_BASE_PI2_PI3      (BCM2708_BASE_PI2_PI3 + 0x200000)   // GPIO control
#define GPIO_BASE_ZERO_ZEROW   (BCM2708_BASE_ZERO    + 0x200000)   //
#define BLOCK_SIZE  (4*1024)

#define GPIO_PTR_TYPE volatile unsigned *

class GPIODelegateImplRaspPi : public GPIODelegateImpl {
private:
    void *gpio_map;
    GPIO_PTR_TYPE gpio;

public:
    GPIODelegateImplRaspPi(DeviceInfoDelegate *deviceInfoDel) {
        int  fd;

        // map the GPIO stuff into our address space
        if ((fd = open("/dev/mem", O_RDWR | O_SYNC) ) < 0) {
            printf("ERROR: [GPIODelegate] can't open /dev/mem (did you forget 'sudo'?)\n");
            exit(-1);
        }

        // Figure out whether we have a PiZero/ZeroW or a Pi2/3
        const char* modelID = deviceInfoDel->getDeviceType();

        const char piZeroModelID[] = "Raspberry Pi Zero";

        if (strncmp(modelID, piZeroModelID, strlen(piZeroModelID)) == 0) {
            // Pi Zero detected
            gpio_map = mmap(NULL, BLOCK_SIZE,
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, GPIO_BASE_ZERO_ZEROW);
        } else {
            // Pi 2 or 3 assumed
            gpio_map = mmap(NULL, BLOCK_SIZE,
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, GPIO_BASE_PI2_PI3);
        }

        close(fd);

        if (gpio_map == MAP_FAILED) {
            printf("GPIODelegate: mmap MAP_FAILED\n");
            exit(-1);
        } else {
            gpio = (GPIO_PTR_TYPE) gpio_map;  // cast to correct type for this platform
        }

        // initialize GPIO pin #GPIO_TO_USE to output
        *gpio = *gpio & ~(7 << (GPIO_TO_USE * 3)); // must set to input first, before setting to output
        *gpio = *gpio |  (1 << (GPIO_TO_USE * 3)); // set to output

        clearGPIO();    // and initially clear it
    }

    inline virtual void setGPIO() {
        *(gpio + 7) = 1 << GPIO_TO_USE;     // set GPIO <GPIO_TO_USE>
    }

    inline virtual void clearGPIO() {
        *(gpio + 10) = 1 << GPIO_TO_USE;    // clear GPIO <GPIO_TO_USE>
    }
};

GPIODelegate::GPIODelegate(DeviceInfoDelegate *deviceInfoDel) {
    impl = new GPIODelegateImplRaspPi(deviceInfoDel);
}
