
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


# TEST 3: GPIO VALIDATION

### Results

Test PASSED by observing Logic Analyzers graphics the width of slow pulses are = 1s, while small pulses are .32us (< 1us)

### Sample Output

    root@imx7d-pico:~/mrm_prequaltest_mx7d# ./preQualTest --gtest_filter="GPIO.*"
    sh: line 0: echo: write error: Device or resource busy
    PreQualification for device: 'Technexion PICO-IMX7 Dual/Solo', IPaddr: 192.168.1.127, run: 2018-04-27T01:08:03Z
    
    Note: Google Test filter = GPIO.*
    [==========] Running 2 tests from 1 test case.
    [----------] Global test environment set-up.
    [----------] 2 tests from GPIO
    [ RUN      ] GPIO.SlowAccessTest
    Ensures that GPIO can be toggled
    
    Start recording on the logic analyzer.
    GPIO high
    GPIO low
    GPIO high
    GPIO low
    GPIO high
    GPIO low
    GPIO high
    GPIO low
    GPIO high
    GPIO low
    Stop recording on the logic analyzer.
    [       OK ] GPIO.SlowAccessTest (10002 ms)
    [ RUN      ] GPIO.FastAccessTest
    Ensures that the GPIO can be toggled quickly (<1us)
    
    Start recording on the logic analyzer.
    GPIO transitioning 20 times.
    Stop recording on the logic analyzer.
    
    TODO: Manually check the logic analyzer output.
          Slow pulses should have H/L time of ~1s.
          Fast pulses should have H/L time of <1us.
          Duration of fast pulse section should be <20us.
    [       OK ] GPIO.FastAccessTest (0 ms)
    [----------] 2 tests from GPIO (10002 ms total)
    
    [----------] Global test environment tear-down
    [==========] 2 tests from 1 test case ran. (10002 ms total)
    [  PASSED  ] 2 tests.


### Login Analyzer Results

![Gpio Short Pulses](GPIO_validation_short_pulses.png)
![Gpio Slow Pulses](GPIO_validation_slow_pulses.png)


# TEST4: TIME SYNCHRONIZATION

### Results

Test PASSED from graphics observation: The TP95 is around **90us** of the median (TP50) line
(Application note marked as PASSED if within 150us)

![Plot with 2 i.MX7D - Device to Device TimeSync delay](TimeSync_results/DUT0-hist-skew.png)


![Plot with 2 i.MX7D - Device to Device TimeSync Skew vs Time](TimeSync_results/DUT0-VsTime-skew.png)


Generated CSV file is: [TimeSync3.test.csv] (TimeSync_results/TimeSync3.test.csv)
---

# TEST 5: AUDIO PLACEMENT

### Graphics

![histogram of audio placement inaccuracy](testresults/i.MX7D_20180529T230243Z/hist_i.MX7D.png)


![audio placement inaccuracy vs Time](testresults/i.MX7D_20180529T230243Z/VsTime_i.MX7D.png)


### Results

Test PASSED from testresults/i.MX7D_20180529T230243Z/report_i.MX7D.txt file


    +-----------------------------------------------------+
    | AUDIO PLACEMENT FOR i.MX7D                          |
    | run = 2018-05-29 23:06:05 UTC, script version = 1.5 |
    +-----------------------------------------------------+
    N (number of samples) = 98
    
    TP0 (min)    = 11166 µs = TP50 - 21 µs
    TP2.5        = 11166 µs = TP50 - 21 µs
    TP50         = 11187 µs
    TP97.5       = 11229 µs = TP50 + 42 µs
    TP100 (max)  = 11229 µs = TP50 + 42 µs
    
    NOTE: always manually check the audio file, too.
    
    +-------------------------------------+
    | Level 1 KPI compliance (Multi-room) |
    +-------------------------------------+
    KPI1a (TP95 spread  < 5000µs) = 63µs: PASS
    KPI1b (TP100 spread < 5000µs) = 63µs: PASS
         Samples outside V1 TP100 KPI: 0 out of 98 = ~0%
    
    +------------------------------------+
    | Level 2 KPI compliance (LR Stereo) |
    +------------------------------------+
    KPI2a (TP95 spread  < 150µs)= 63µs: PASS
    KPI2b (TP100 spread < 150µs) = 63µs: PASS
         Samples outside V2 TP100 KPI: 0 out of 98 = ~0%
    
    +----------------------+
    | Audio Placement Data |
    +----------------------+
    
    Assume constant (correctable) lag is TP50(lag_µs): 11187 µs
    
            sn burstStartSN lag_µsec uncorrected_lag_µsec V1_KPI V2_KPI
    1   286069       334606    11187                    0      .      .
    2   357246       405782    11166                  -21      .      .
    3   502561       551098    11187                    0      .      .
    4   574213       622751    11208                   21      .      .
    5   643678       692215    11187                    0      .      .
    6   713044       761581    11187                    0      .      .
    7   785604       834140    11166                  -21      .      .
    8   854858       903396    11208                   21      .      .
    9   928081       976618    11187                    0      .      .
    10 1000572      1049109    11187                    0      .      .
    11 1074433      1122971    11208                   21      .      .
    12 1146217      1194755    11208                   21      .      .
    13 1217971      1266508    11187                    0      .      .
    14 1289437      1337974    11187                    0      .      .
    15 1358639      1407176    11187                    0      .      .
    16 1432161      1480699    11208                   21      .      .
    17 1510067      1558604    11187                    0      .      .
    18 1580817      1629354    11187                    0      .      .
    19 1649803      1698340    11187                    0      .      .
    20 1731033      1779570    11187                    0      .      .
    21 1809037      1857575    11208                   21      .      .
    22 1885218      1933755    11187                    0      .      .
    23 1961302      2009840    11208                   21      .      .
    24 2043074      2091612    11208                   21      .      .
    25 2114371      2162908    11187                    0      .      .
    26 2184685      2233222    11187                    0      .      .
    27 2254103      2302641    11208                   21      .      .
    28 2333045      2381582    11187                    0      .      .
    29 2410838      2459375    11187                    0      .      .
    30 2492093      2540632    11229                   42      .      .
    31 2566612      2615149    11187                    0      .      .
    32 2646930      2695467    11187                    0      .      .
    33 2719543      2768080    11187                    0      .      .
    34 2796428      2844967    11229                   42      .      .
    35 2865428      2913966    11208                   21      .      .
    36 2938920      2987458    11208                   21      .      .
    37 3016595      3065132    11187                    0      .      .
    38 3089521      3138057    11166                  -21      .      .
    39 3163707      3212245    11208                   21      .      .
    40 3246034      3294572    11208                   21      .      .
    41 3322915      3371452    11187                    0      .      .
    42 3402399      3450935    11166                  -21      .      .
    43 3473521      3522057    11166                  -21      .      .
    44 3553576      3602112    11166                  -21      .      .
    45 3635957      3684495    11208                   21      .      .
    46 3707724      3756263    11229                   42      .      .
    47 3778342      3826879    11187                    0      .      .
    48 3855679      3904216    11187                    0      .      .
    49 3929576      3978113    11187                    0      .      .
    50 4001040      4049577    11187                    0      .      .
    51 4076609      4125147    11208                   21      .      .
    52 4145583      4194120    11187                    0      .      .
    53 4224200      4272738    11208                   21      .      .
    54 4307047      4355586    11229                   42      .      .
    55 4389162      4437699    11187                    0      .      .
    56 4470496      4519033    11187                    0      .      .
    57 4540641      4589179    11208                   21      .      .
    58 4624073      4672611    11208                   21      .      .
    59 4701388      4749926    11208                   21      .      .
    60 4780742      4829279    11187                    0      .      .
    61 4862004      4910541    11187                    0      .      .
    62 4930819      4979356    11187                    0      .      .
    63 5007515      5056052    11187                    0      .      .
    64 5078396      5126933    11187                    0      .      .
    65 5155571      5204109    11208                   21      .      .
    66 5232684      5281221    11187                    0      .      .
    67 5308482      5357019    11187                    0      .      .
    68 5380345      5428883    11208                   21      .      .
    69 5461831      5510367    11166                  -21      .      .
    70 5543304      5591842    11208                   21      .      .
    71 5614512      5663048    11166                  -21      .      .
    72 5689907      5738445    11208                   21      .      .
    73 5767858      5816395    11187                    0      .      .
    74 5841601      5890137    11166                  -21      .      .
    75 5914056      5962594    11208                   21      .      .
    76 5991437      6039975    11208                   21      .      .
    77 6068362      6116899    11187                    0      .      .
    78 6142820      6191358    11208                   21      .      .
    79 6214559      6263096    11187                    0      .      .
    80 6296826      6345364    11208                   21      .      .
    81 6373743      6422279    11166                  -21      .      .
    82 6452467      6501004    11187                    0      .      .
    83 6535085      6583623    11208                   21      .      .
    84 6607644      6656181    11187                    0      .      .
    85 6686233      6734771    11208                   21      .      .
    86 6838884      6887421    11187                    0      .      .
    87 6919081      6967618    11187                    0      .      .
    88 7000827      7049365    11208                   21      .      .
    89 7080431      7128968    11187                    0      .      .
    90 7157023      7205560    11187                    0      .      .
    91 7237018      7285556    11208                   21      .      .
    92 7316894      7365431    11187                    0      .      .
    93 7387194      7435731    11187                    0      .      .
    94 7469522      7518060    11208                   21      .      .
    95 7543560      7592098    11208                   21      .      .
    96 7622372      7670909    11187                    0      .      .
    97 7697538      7746075    11187                    0      .      .
    98 7774862      7823400    11208                   21      .      .


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
