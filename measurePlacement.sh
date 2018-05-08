#!/bin/bash

# Copyright 2017-2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved. 
#
# You may not use this file except in compliance with the terms and conditions 
# set forth in the accompanying LICENSE.TXT file.
#
# THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS.  AMAZON SPECIFICALLY DISCLAIMS, 
# WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY, 
# INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
# PURPOSE, AND NON-INFRINGEMENT.

# --------------------------------------------------------------------------------
# This script is designed to run on a Mac OS X laptop, with a Focusrite Scarlett 18i8 
#   USB audio digitizer.  Running on other laptops or with a different USB digitizer will 
#   require that you make changes to this script (specifically the SOX line below).
#
# Sox, R, and RStudio are free and available for Mac OS X, Windows, and Linux.
#
# INSTALLING DEPENDENCIES:
# Install sox on Mac OS X using:  brew install sox
#
# Install free version of R from r-project.org
# Install free version of RStudio from rstudio.com
# Install these R libraries using RStudio, Tools > Install Packages...
#   dplyr, ggplot2, readr, tuneR, lubridate, stringr, optparse
#
# Run script:  ./measurePlacement.sh
#
# For most reliable audio capture, do not run anything else on your laptop 
#   while the test is running.

# SET TEST INPUT PARAMETERS ----------------
DUT="RPi3" 		# YOUR DEVICE TYPE

# WHERE SHOULD THE RESULTS GO?
testDir="~/testresults"
soxDir="~/sox-14.4.2"
#lengthToUse="0:30:00"		# DESIRED TEST DURATION, H:MM:SS
lengthToUse="200" # 200 seconds, should cover around 100 iterations


# capture device (must be the name used by sox to capture from the USB digitizer)
device="Scarlett 18"        # FOCUSRITE Scarlett 18i8, ch1 = GPIO, ch2 = DUT AUDIO

# where the R script can be found
reportGenerator="$PWD/placement.R"

# ------------------------------------------
startTimeUTC=`date -u +%Y%m%dT%H%M%SZ`
resultsDir="${testDir}/${DUT}_${startTimeUTC}"
captureDir="${resultsDir}/audio"

# make directories, if they don't exist already
mkdir -p ${captureDir}
mkdir -p ${resultsDir}
mkdir -p ${resultsDir}/audio

# go to the results directory
pushd ${resultsDir} > /dev/null 2>&1

# write metadata to a file for the record ------------------
echo "Writing metadata file to record test conditions..."

echo "Metadata for: ${resultsDir}"     >> ${resultsDir}/metadata.txt
echo "DUT: ${DUT}"      	           >> ${resultsDir}/metadata.txt
echo "Capture length: ${lengthToUse}"  >> ${resultsDir}/metadata.txt

# ----------------------------------------
echo "---------------------------------------------------------------------------"
echo Starting Audio Placement Test, output dir: ${resultsDir}

# delete old files from the resultsDir
rm -f ${resultsDir}/*.png ${resultsDir}/report.*.txt

# capture 
outfile=${captureDir}/audio.${startTimeUTC}.wav

echo "Please run audioPlacementTest on the Device-Under-Test now."
echo "Waiting for capture to complete..."

# NOTE: MAC OS X SPECIFIC *****
${soxDir}/sox --buffer 131072 --multi-threaded -D -t coreaudio "${device}" -q -t wav ${outfile} channels 2 trim 0 ${lengthToUse} 

# Process the audio file, and generate a report (if you need to look at the R log, it's there...)
echo Processing the results, and creating a report...
echo Rscript --vanilla ${reportGenerator} --results ${resultsDir} --dut ${DUT} --infile ${outfile} 
Rscript --vanilla ${reportGenerator} --results ${resultsDir} --dut ${DUT} --infile ${outfile} > analyzePlacement.log.txt 2>&1

echo DONE.
echo "---------------------------------------------------------------------------"
echo

# --------------------------------------------------------------------------------------------------
# On some platforms, automatically open up the graphs/report to double check that everything was OK.
# --------------------------------------------------------------------------------------------------
if [[ "${OSTYPE}" == "darwin"* ]]; then
# MAC OS X only:
	open ${resultsDir}/*.png
	open ${resultsDir}/report*.txt
fi

popd > /dev/null 2>&1
