
# TEST 2: HIGH-RESOLUTION TIMER

### Results

Test PASSED from observing console output

### Sample Output Evidence

    root@imx7d-pico:~/mrm_prequaltest_mx7d# ./preQualTest --gtest_filter="HRT.*"
    sh: line 0: echo: write error: Device or resource busy
    PreQualification for device: 'Technexion PICO-IMX7 Dual/Solo', IPaddr: 192.168.1.127, run: 2018-04-26T23:37:21Z
    
    Note: Google Test filter = HRT.*
    [==========] Running 2 tests from 1 test case.
    [----------] Global test environment set-up.
    [----------] 2 tests from HRT
    [ RUN      ] HRT.SlowAccessTest
    Ensures that HRT increments at about the right rate (~1E9 ns/sec)
    
    HRT1_ns,HRT2_ns,delta_ns,result
    973553772375,974553935625,1000163250,PASS
    974553999875,975554157750,1000157875,PASS
    975554208375,976554353500,1000145125,PASS
    976554397875,977554549625,1000151750,PASS
    977554593375,978554745750,1000152375,PASS
    978554788750,979554944000,1000155250,PASS
    979554997250,980555141875,1000144625,PASS
    980555187000,981555334875,1000147875,PASS
    981555378500,982555529375,1000150875,PASS
    982555572500,983555717750,1000145250,PASS
    983555760750,984555897375,1000136625,PASS
    984555953625,985556090000,1000136375,PASS
    985556135500,986556285000,1000149500,PASS
    986556330000,987556481500,1000151500,PASS
    987556524750,988556677250,1000152500,PASS
    988556720000,989556874125,1000154125,PASS
    989556916750,990557058625,1000141875,PASS
    990557102125,991557236000,1000133875,PASS
    991557278250,992557432000,1000153750,PASS
    992557475000,993557620625,1000145625,PASS
    993557663375,994557779875,1000116500,PASS
    994557822250,995557955000,1000132750,PASS
    995557996750,996558129375,1000132625,PASS
    996558172375,997558325375,1000153000,PASS
    997558367500,998558522000,1000154500,PASS
    [       OK ] HRT.SlowAccessTest (25005 ms)
    [ RUN      ] HRT.FastAccessTest
    Ensures that the HRT can be accessed quickly (<=3us, 1 failure allowed)
    
    HRT1_ns,HRT2_ns,delta_ns,result
    998558807000,998558808375,1375,PASS
    998558825750,998558826500,750,PASS
    998558841875,998558842750,875,PASS
    998558857500,998558858375,875,PASS
    998558873125,998558874000,875,PASS
    998558888750,998558889500,750,PASS
    998558904250,998558905125,875,PASS
    998558919875,998558920625,750,PASS
    998558935500,998558936250,750,PASS
    998558951125,998558952000,875,PASS
    998558966750,998558967625,875,PASS
    998558982375,998558983250,875,PASS
    998558998000,998558998875,875,PASS
    998559013625,998559014500,875,PASS
    998559029250,998559030000,750,PASS
    998559044750,998559045625,875,PASS
    998559060375,998559061125,750,PASS
    998559076000,998559076875,875,PASS
    998559091625,998559092500,875,PASS
    998559107125,998559108000,875,PASS
    998559122750,998559123625,875,PASS
    998559138250,998559139125,875,PASS
    998559153875,998559154625,750,PASS
    998559169500,998559170375,875,PASS
    998559185000,998559185875,875,PASS
    [       OK ] HRT.FastAccessTest (0 ms)
    [----------] 2 tests from HRT (25006 ms total)
    
    [----------] Global test environment tear-down
    [==========] 2 tests from 1 test case ran. (25006 ms total)
    [  PASSED  ] 2 tests.


# TEST 6: AUDIO DISTRIBUTION

### Results

Test PASSED from observing console output 

### Sample Output Evidence

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
