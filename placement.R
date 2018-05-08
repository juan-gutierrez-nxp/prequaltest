# Copyright 2017-2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
#
# You may not use this file except in compliance with the terms and conditions
# set forth in the accompanying LICENSE.TXT file.
#
# THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS.  AMAZON SPECIFICALLY DISCLAIMS,
# WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY,
# INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE, AND NON-INFRINGEMENT.

library(dplyr)
library(ggplot2)
library(readr)
library(tuneR)
library(lubridate)
library(stringr)
library(optparse)

rm(list=ls())

# QUICK INSTRUCTIONS TO RUN THIS AUDIO PLACEMENT/ALSA TEST ------------
# Install R and the community version of RStudio (http://r-project.org and http://rstudio.com).
#   Use RStudio > Tools > Install Packages to install the above libraries (and their dependencies)
# Compile ./sineWaveTest for your platform, and transfer to the DUT.
#   also transfer the "441Hz_stereo_44.1.s16" file to the DUT.
# Modify ./measurePlacement.sh to describe the DUT, and choose the length of test
# connect DUT to 18i8 digitizer, GPIO = ch1, audio = ch2
#   GPIO should go through WHA LINE OUT TEST BOARD to 18i8 (3.5mm to 1/4" cable is required)
#   AUDIO LINE OUT can go direct to 18i8 (3.5mm to 1/4" cable is required)
#   AUDIO INTERNAL SPKR should go through WHA LINE OUT TEST BOARD (audio side, 3.5mm to 1/4" cable req'd)
# run "./sineWaveTest 441Hz_stereo_44.1.s16 1" on the DUT
# Check audio levels on the digitizer with Audacity.  Peak levels should be about -3dB.
# Run "./measurePlacement.sh" on the laptop
# Run "./sineWaveTest 441Hz_stereo_44.1.s16 1" on the DUT
# When the capture is complete, you can control-C out of "./sineWaveTest"
#
# Results will be *.png and report.*.txt files in ${DUT}_${startTimeUTC}
#     captured audio will be in the ${DUT}_${startTimeUTC}/audio directory, and you
#     can manually inspect this audio.  The time between the leading edge of the GPIO and
#     a specific zero crossing in the tone burst should be a constant.

# ----------------------------------------
if (interactive()) {
  # INTERACTIVE MODE (DEBUGGING) ---------
  # use user-specified parameters for debugging
  DUT <- "RPi3"               # report name
  timestamp <- "20171129T195642Z"
  testDir   <- "/Users/mpogue/WHA-3P/PreQualificationTests/"

  resultsDir <- paste0(testDir, DUT, "_", timestamp, "/")  # must have trailing slash
  setwd(resultsDir)  # where the audio file is located (and results will go)
  infile <- paste0(resultsDir, "audio/audio.", timestamp, ".wav")     # wave file name
} else {
  # BATCH MODE (ACTUAL MEASUREMENT) ---------------
  # process command line parameters
  option_list = list(
    make_option(c("-r", "--results"), type="character", default=NULL,
                help="results directory name", metavar="character"),
    make_option(c("-i", "--infile"), type="character", default=NULL,
                help="input WAV file name", metavar="character"),
    make_option(c("-t", "--dut"), type="character", default=NULL,
                help="DUT name for reporting", metavar="character")
  );

  opt_parser = OptionParser(option_list=option_list);
  opt = parse_args(opt_parser);

  resultsDir <- opt$results
  DUT <- opt$dut
  infile <- opt$infile
}

resultsDir
DUT
infile

# -------------------------
# global options
theme_set(theme_bw())
op <- options(dplyr.width = Inf, width = 300, max.print=49999)

version = "1.5"
versionString <- paste0("run = ",now(tz="UTC")," UTC, script version = ", version)

# UTILITY FUNCTIONS ----------------------------------------------------------------------
# Title box for report generation --------------------------------------------------------
#   set subtitle, if versionString is desired as the second line.
titleBox <- function(str, subtitle=FALSE) {
  if (subtitle && exists("versionString")) {
    l <- max(nchar(str), nchar(versionString))
    s <- paste0("+", paste0(replicate(l+2, "-"), collapse=""), "+\n")
    s2 <- paste0(s,"| ",str_pad(str,l+1,side="right"),"|\n","| ",str_pad(versionString,l+1,side="right"),"|\n",s)
  } else {
    l <- nchar(str)
    s <- paste0("+", paste0(replicate(l+2, "-"), collapse=""), "+\n")
    s2 <- paste0(s,"| ",str_pad(str,l+1,side="right"),"|\n",s)
  }
  s2
}

# ----------------------------------------
# read in the WAV file
wav <- readWave(infile)
sampleRate = wav@samp.rate
a <- data.frame(ch1=wav@left, ch2=wav@right) %>%
  mutate(sn=row_number()-1, t_sec=sn/sampleRate)  # sampleNumbers start at zero in Audacity
wav <- NULL  # we don't need the WAV data anymore

# ----------------
# NOISE GATE
threshold1 <- max(a$ch2)*1/2  # 2/3 of the max value = threshold for edge detection (rise time is about 2 samples)
a2 <- data.frame(audio=a$ch2) %>%
  mutate(above=abs(audio)>threshold1,
         fill=lead(above,20,0)|lead(above,10,0)|above|lag(above,10,0)|lag(above,20,0),
         rn=row_number()) %>%
  # filter(!is.na(fill)) %>%            # get rid of lag/lead artifacts
  mutate(audio=ifelse(fill,audio,0))  # <-- gate

# use the gated audio, instead of the original audio
a$ch2 <- a2$audio

# FIND GPIO LEADING EDGES --------------------------
threshold <- max(a$ch1)/3  # 1/3 of the max value = threshold for edge detection (rise time is about 2 samples)
gpioSpikes <- a %>% filter(lag(ch1) < threshold & ch1 > threshold) %>%  # upgoing edge
  mutate(sn=sn-1) %>%  # subtract 1 makes it more correct, because of rise time
  select(sn) %>%
  mutate(delta=sn-lag(sn))

head(gpioSpikes)
tail(gpioSpikes)

# FIND TONE BURST LEADING EDGES --------------------
# tolerance is ±10 samples for the first step
periodTolerance = 10
testToneFrequency = 480
period  = sampleRate/testToneFrequency
periodLow  = as.integer(period - periodTolerance)
periodHigh = as.integer(period + periodTolerance)

# tolerance is ±3 samples for the second step
threePeriodTolerance = 3
threePeriod     = as.integer(3*period)
threePeriodLow  = threePeriod - threePeriodTolerance
threePeriodHigh = threePeriod + threePeriodTolerance

# 327±3 is 3 periods of 109.09 samples (48000/440), because we're looking for 3 adjacent ZC's of the right length
#  tolerance is ±3 samples
burst1 <-
  a %>%
  filter(ch2>=0 & lead(ch2)<0) %>%   # down-going edges
  mutate(timeBetween = sn-lag(sn)) %>%
  filter(timeBetween>periodLow & timeBetween<periodHigh) %>%
  # mutate(burstStartSN=sn-threePeriod/2,  # 164 samples in 1.5 periods of 440Hz, because we pick up the SECOND downgoing zero crossing
  #        inBurst=ifelse(lead(sn,3)-sn >= threePeriodLow & lead(sn,3)-sn <= threePeriodHigh, 1, 0),
  #        burstStart=ifelse(inBurst==1 & lag(inBurst,default=0)==0, 1, 0)) %>%
  mutate(burstStartSN=lead(sn,3),
         inBurst=ifelse(lead(sn,3)-sn >= threePeriodLow & lead(sn,3)-sn <= threePeriodHigh, 1, 0),
         burstStart=ifelse(inBurst==1 & lag(inBurst,default=0)==0, 1, 0),
         burstNumber=cumsum(burstStart))

bursts <- burst1 %>%
  filter(burstStart==1) %>%
  mutate(burstStartSN=burstStartSN+1) %>%  # correct +1 due to when we pick up the negative-going edge
  select(burstStartSN)

# Match up by integer second "buckets"
#  burst is either in the sameBucket as gpioSpike, or in the sameBucket + 1 as gpioSpike
# head(gpioSpikes)
# head(bursts)

g <- gpioSpikes %>%
  mutate(bucket=as.integer(sn/sampleRate)) %>%
  select(-delta)
b <- bursts %>%
  mutate(bucket=as.integer(burstStartSN/sampleRate),
         bucket1=bucket-1)

sameBucket  <- merge(g, b, by.x="bucket", by.y="bucket") %>% select(-bucket1) %>% filter(burstStartSN > sn)  # match in same bucket must have burst later
sameBucket1 <- merge(g, b %>% select(-bucket), by.x="bucket",by.y="bucket1") # burst delayed by exactly 1 bucket

results <- rbind(sameBucket, sameBucket1) %>%
  arrange(bucket) %>%
  filter(!duplicated(bucket)) %>%   # take only the first match for each bucket #
  mutate(lag_µsec=as.integer(1E6* ((burstStartSN-sn)/sampleRate - 1.0)))  # 1.0 is the delay from GPIO to tone )

head(results)
tail(results)

# REPORT GENERATION -----------
TPS <- function(d) {
  q=quantile(d, probs=c(0.025,0.5,0.975))
  c(q[["2.5%"]], q[["50%"]], q[["97.5%"]])
}

tps <- TPS(results$lag_µsec)
tp50 <- tps[2]

# PLOT: Histogram of audio placement errors ------------
V1_KPI_µs = 5000
V2_KPI_µs = 150

png(paste0(resultsDir,"/hist_",DUT,".png"), width=1024, height=800)
  V1_limits = c(tp50-V1_KPI_µs/2, tp50+V1_KPI_µs/2)
  V2_limits = c(tp50-V2_KPI_µs/2, tp50+V2_KPI_µs/2)
  textSize = 4.0
  p1 <- ggplot(results, aes(x=lag_µsec)) + theme_bw() +
    geom_histogram(fill="white", color="black", aes(y=..density..)) +
    # geom_density(color="red", fill="red", alpha=0.25, bw=250.0) +
    geom_density(color="red", fill="red", alpha=0.25) +
    scale_x_continuous(breaks=seq(-50000,50000,1000)) +
    geom_vline(xintercept=tps, linetype=c("dotted","dashed","dotted"), color=c("red","darkgreen","red")) +
    geom_vline(xintercept=V2_limits, linetype="dashed", color="darkgreen") +
    geom_vline(xintercept=V1_limits, linetype="dashed", color="red") +
    annotate("text", size=textSize, x=V1_limits, y=Inf, vjust=1, label="V1\nKPI") +
    annotate("text", size=textSize, x=V2_limits, y=Inf, vjust=1, label="V2\nKPI") +
    annotate("text", size=textSize, x=tps, y=Inf, vjust=c(2.5,1,4)+1.5,
             color=c("red","darkgreen","red"), label=paste0("TP",c("2.5","50","97.5"),"=\n",round(tps,0),"µs")) +
    xlab("Lag (µs)") +
    ggtitle(paste0("Histogram of Audio Placement Errors on ", DUT, ", N=", length(results$lag_µsec)),
            subtitle = paste0("file: ",infile,", ",versionString))
  print(p1)
dev.off()

# PLOT: Audio placement errors over time ----------------
head(results)
tail(results)

png(paste0(resultsDir,"/VsTime_",DUT,".png"), width=1024, height=800)
  deltaRange = range(results$lag_µsec)
  p2 <- ggplot(results, aes(x=sn/sampleRate, y=lag_µsec)) + theme_bw() +
    geom_line() +
    geom_point() +
    geom_hline(yintercept=V1_limits, linetype="dashed", color="darkgreen") +
    geom_hline(yintercept=V2_limits, linetype="dashed", color="red") +
    annotate("text", size=textSize, y=V1_limits, x=0, hjust=1, label="V1_KPI", color="darkgreen") +
    annotate("text", size=textSize, y=V2_limits, x=0, hjust=1, label="V2_KPI", color="red") +
    # geom_hline(yintercept=c(tps[2]-25,tps[2]+25), linetype="dotted", color="red") +
    xlab("Time in test (sec)") +
    # scale_y_continuous(limits=c(0,deltaRange[2]*1.2), breaks=seq(0,deltaRange[2]*1.5,200)) +
    # scale_y_continuous(limits=c(deltaRange[1]*1.2,deltaRange[2]*1.2)) +
    scale_y_continuous(breaks=seq(-50000,50000,1000)) +
    ggtitle(paste0("Audio Placement Errors on ",DUT," using ALSA, vs Time"),
            subtitle = paste0("file: ",infile,", ",versionString))
  print(p2)
dev.off()

# REPORT -------------------------------
head(results)
tail(results)

sink(paste0(resultsDir,"/report_",DUT,".txt"))
  cat(titleBox(paste0("AUDIO PLACEMENT FOR ",DUT),TRUE))
  N <- length(results$lag_µsec)
  cat(paste0("N (number of samples) = ",N,"\n\n"))
  cat(paste0("TP0 (min)    = ",round(min(results$lag_µsec),0)),"µs = TP50 -",round(tps[2],0)-round(min(results$lag_µsec),0),"µs\n")
  cat(paste0("TP2.5        = ",round(tps[1],0)),"µs = TP50 -",round(tps[2],0)-round(tps[1],0),"µs\n")
  cat(paste0("TP50         = ",round(tps[2],0)),"µs\n")
  cat(paste0("TP97.5       = ",round(tps[3],0)),"µs = TP50 +",round(tps[3],0)-round(tps[2],0),"µs\n")
  cat(paste0("TP100 (max)  = ",round(max(results$lag_µsec),0)),"µs = TP50 +",round(max(results$lag_µsec)-round(tps[2],0),0),"µs\n")

  cat("\nNOTE: always manually check the audio file, too.\n")

  failures_V1 <- results %>% filter(abs(lag_µsec-tps[2]) > V1_KPI_µs/2) %>% select(bucket) %>% mutate(V1_KPI="FAIL")
  failures_V2 <- results %>% filter(abs(lag_µsec-tps[2]) > V2_KPI_µs/2) %>% select(bucket) %>% mutate(V2_KPI="FAIL")

  cat("\n")
  cat(titleBox(paste0("Level 1 KPI compliance (Multi-room)")))
  TP95spread <- round(tps[3],0)-round(tps[1],0)
  cat(paste0("KPI1a (TP95 spread  < ",V1_KPI_µs,"µs) = ",TP95spread,"µs: ",ifelse(abs(TP95spread) < V1_KPI_µs,"PASS", "FAIL"),"\n"))
  TP100spread <- round(max(results$lag_µsec),0)-round(min(results$lag_µsec))
  cat(paste0("KPI1b (TP100 spread < ",V1_KPI_µs,"µs) = ",TP100spread,"µs: ",ifelse(abs(TP100spread) < V1_KPI_µs,"PASS", "FAIL"),"\n"))
  cat(paste0("     Samples outside V1 TP100 KPI: ", length(failures_V1$bucket), " out of ", N, " = ~", round(100.0*length(failures_V1$bucket)/N,1),"%\n"))
  cat("\n")
  cat(titleBox(paste0("Level 2 KPI compliance (LR Stereo)")))
  cat(paste0("KPI2a (TP95 spread  < ",V2_KPI_µs,"µs)= ",TP95spread,"µs: ",ifelse(abs(TP95spread) < V2_KPI_µs,"PASS", "FAIL"),"\n"))
  cat(paste0("KPI2b (TP100 spread < ",V2_KPI_µs,"µs) = ",TP100spread,"µs: ",ifelse(abs(TP100spread) < V2_KPI_µs,"PASS", "FAIL"),"\n"))
  cat(paste0("     Samples outside V2 TP100 KPI: ", length(failures_V2$bucket), " out of ", N, " = ~", round(100.0*length(failures_V2$bucket)/N,1),"%\n"))
  cat("\n")
  cat(titleBox(paste0("Audio Placement Data")))
  cat("\n")
  cat(paste0("Assume constant (correctable) lag is TP50(lag_µs): ", tps[2]," µs\n"))
  cat("\n")
  resTable <- results %>%
    mutate(uncorrected_lag_µsec=round(lag_µsec-tps[2],0)) %>%
    left_join(failures_V1) %>%
    left_join(failures_V2) %>%
    select(-bucket) %>%
    mutate(V1_KPI=ifelse(is.na(V1_KPI),".",V1_KPI),
           V2_KPI=ifelse(is.na(V2_KPI),".",V2_KPI))
  print(resTable)
sink()
