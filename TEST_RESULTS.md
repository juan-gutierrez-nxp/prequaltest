#TEST 6: AUDIO DISTRIBUTION

## Results

Test PASSED from observing console output 

### Output results

**Master**

    root@imx7d-pico:~/mrm_prequaltest_mx7d#
    bution.Master 192.168.1.126altest_mx7d# ./preQualTest --gtest_filter=AudioDistri
    PreQualification for device: 'Technexion PICO-IMX7 Dual/Solo', IPaddr: 192.168.1.127, run: 2018-04-26T23:24:03Z
    
    Note: Google Test filter = AudioDistribution.Master
    [==========] Running 1 test from 1 test case.
    [----------] Global test environment set-up.
    [----------] 1 test from AudioDistribution
    [ RUN      ] AudioDistribution.Master
    Audio Distribution Unicast MASTER
    Slave devices:
    Slave #0: 192.168.1.126
    Trying to connect to Slave #0 at '192.168.1.126'...CONNECTED.
    master: now connected to 192.168.1.126 on port 1234....
    DONE.
    [       OK ] AudioDistribution.Master (1846 ms)
    [----------] 1 test from AudioDistribution (1846 ms total)
    
    [----------] Global test environment tear-down
    [==========] 1 test from 1 test case ran. (1846 ms total)
    [  PASSED  ] 1 test.

**Slave**

    root@imx7d-pico:~/mrm_prequaltest_mx7d#
    bution.Slaveico:~/mrm_prequaltest_mx7d# ./preQualTest --gtest_filter=AudioDistri
    PreQualification for device: 'Technexion PICO-IMX7 Dual/Solo', IPaddr: 192.168.1.126, run: 2018-04-27T01:06:26Z
    
    Note: Google Test filter = AudioDistribution.Slave
    [==========] Running 1 test from 1 test case.
    [----------] Global test environment set-up.
    [----------] 1 test from AudioDistribution
    [ RUN      ] AudioDistribution.Slave
    Audio Distribution Unicast: SLAVE
    slave: waiting for connections...
    slave: got connection from 192.168.1.127
    Each report below = ~1000000 bytes received.
    incrMbps,cumuMbps
    31.097,31.097,0.000
    43.388,36.211,0.000
    41.805,37.902,0.000
    50.485,40.417,0.000
    45.563,41.353,0.000
    52.957,42.918,0.000
    49.401,43.741,0.000
    55.178,44.899,0.000
    47.760,45.201,0.000
    58.714,46.267,0.000
    slave: recv -- Master disconnected
    [       OK ] AudioDistribution.Slave (30040 ms)
    [----------] 1 test from AudioDistribution (30040 ms total)
    
    [----------] Global test environment tear-down
    [==========] 1 test from 1 test case ran. (30041 ms total)
    [  PASSED  ] 1 test.
