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

#ifndef AMAZON_WHA_H
#define AMAZON_WHA_H

#include "WHADelegates.h"

namespace wha {

/**
 * WHA class.
 *
 * This class provides APIs to initiate Whole Home Audio library.
 *
 * @note Methods whose names begin with "post" are asynchronous and thread safe,
 * returning immediately after enqueueing work to be carried out later by the WHA
 * internal library in a different execution context.
 */
class WHA_PUBLIC WHA
{
public:
    /**
     * Constructor.
     * @param cloudDelegate cloud delegate.
     * @param deviceInfoDelegate device information delegate.
     * @param playbackDelegate Playback Delegate.
     * @param timeDelegate Time Delegate.
     * @param loggingDelegate Logging Delegate.
     */
    WHA(CloudDelegate* cloudDelegate,
        DeviceInfoDelegate* deviceInfoDelegate,
        PlaybackDelegate* playbackDelegate,
        TimeDelegate* timeDelegate,
        FocusDelegate* focusDelegate,
        LoggingDelegate* loggingDelegate);

    ~WHA();

    /**
     * Start the WHA library's internal processes.  This method should be called when
     * the system is ready for the WHA library to start processing events.
     */
    void run();

    /**
     * Stop the WHA library's internal processes.  This method should be called to
     * shut down the WHA service.
     */
    void stop();

    /**
     * Inform WHA to handle the given Directive. This method should be called when a WHA
     * related Directive arrives. The method 'isAcceptedNamespace' in this class is used
     * to determine if a given Directive is WHA related. Note that the WHA internal library
     * will take care of a handled message asynchronously. This method is nonblocking and
     * thread safe. The return code only indicates the validity of parameters, -1 means
     * WHA got bad parameters (null or unexpected namespace) and client can check on that
     * for debugging.
     *
     * @param nameSpace Message namespace.
     * @param name Message name.
     * @messageId Message Id.
     * @payload Stringified payload JSON.
     * @return 0 if parameters are valid, otherwise return -1.
     */
    int handleDirective(const char* nameSpace, const char* name,
                        const char* messageId, const char* payload);

    /**
     * Helper function to decide if a message is under WHA namespaces. Note that result is
     * deterministic to a given namespace regardless how many queries to the same namespace
     * string. Client may therefore cache the boolean response for a given namespace string.
     *
     * @param nameSpace The namespace of a cloud message.
     * @param return True if the namespace is owned by WHA.
     */
    bool isAcceptedNamespace(const char* nameSpace);

    /**
     * Inform WHA that the device's Alexa connectivity has changed. This method should
     * be invoked when there is a Alexa connection state change on the local device.
     * Note that WHA might not be able to handle directives or send events until it is
     * notified it has a valid connection. WHA library will query the connection state
     * with Alexa Service via 'CloudDelegate::hasAlexaConnection'.
     */
    void postAlexaConnectivityChanged();

    /**
     * Inform WHA of the idle state change. This method should be called when the AVS
     * Client needs to send up an UserInactivityReport Event every one hour of inactivity.
     * Once this method is called, WHA library will decide if it needs to calculate
     * the cluster idle time and send an Event related to cluster inactivity to the
     * cloud via 'CloudDelegate::sendEvent'.
     */
    void postDeviceIdleChanged();

    /**
     * Inform the WHA library of a change in the local device main volume. Note that it's
     * only used for the general system volume or playback volume. If there is a separate
     * concepts such as alarm or notification volume, we don't expect to be notified by
     * this API. It is used for cluster main volume management within WHA library. WHA
     * library will query the updated main volume via a DeviceInfoDelegate.
     */
    void postDeviceMainVolumeChanged();

    /**
     * Inform the WHA library that the device's network connectivity has changed.
     * This method should be invoked when the network connection goes down or comes up,
     * and when the device's IP address changes. The library will query the latest
     * network information such as IP address through a DeviceInfoDelegate.
     */
    void postNetworkConnectionChanged();

    /**
     * Inform the WHA library that there is a registration activity by the user. The
     * registration change is defined when users set up Alexa account with the local device
     * via corresponding companion app or some other methods. WHA library will query the
     * registration (deregistred/registered) state via a DeviceInfoDelegate.
     */
    void postRegistrationChanged();

    /**
     * Returns version of the WHA library
     * @return String representing version of library at runtime.
     */
    const char* version();
private:
    class WHAImpl;
    WHAImpl* mImpl;
};

} // namespace wha

#endif   // AMAZON_WHA_H
