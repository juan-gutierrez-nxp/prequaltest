#!/usr/bin/env Rscript
library(dplyr)
library(ggplot2)
library(reshape2)
library(readr)
library(stringr)
library(lubridate)
library(scales)
library(padr)  # for nice aggregation by time periods

rm(list=ls())

# setwd("/Users/paulwit/work/prequal-automate")
inputFilename <- 'results/TimeSyncTest-L7MXCD.csv'
prefix <- 'TimeSyncTest-ABCDE'

args = commandArgs(trailingOnly=TRUE)
if( length(args) > 0 ) {
  inputFilename <- args[1]
}
# else {
#   stop("Must provide path to .csv file to analyze")
# }

if( length(args) > 1 ) {
  prefix <- args[2]
}
# else {
#   stop("Must also provide unique name for file naming, random string is fine")
# }

# SET THESE FOR YOUR DEVICE =================================================
usPerSample = 1                   # LOGIC ANALYZER CAPTURE RATE (currently 1M samp/sec)
results_dir <- dirname(inputFilename)

# ===========================================================================
# global options
theme_set(theme_bw())  # nicer theme for plots/graphs

version = "1.3"
versionString <- paste0("run = ",Sys.time(),"PT, script version = ", version)

# UTILITIES ==================================================================
# TP values ------------------------------------------------------------------
# NOTE: if replace is set, any NA's will be replaced with this value
TP2.5 <- function(x, replace=NA) {
  x[is.na(x)] <- replace
  quantile(x, probs=c(0.025))[["2.5%"]]
}
TP50 <- function(x, replace=NA) {
  x[is.na(x)] <- replace
  quantile(x, probs=c(0.5))[["50%"]]
}
TP97.5 <- function(x, replace=NA) {
  x[is.na(x)] <- replace
  quantile(x, probs=c(0.975))[["97.5%"]]
}

# given 3 values, returns the one that is closest to zero, or NA if any are NA
closestToZero_ <- function(a,b,c) {
  if (any(is.na(c(a,b,c)))) {
    return(NA)
  }
  df <- data.frame(original=c(a,b,c)) %>% mutate(absolute=abs(original)) %>% arrange(absolute)
  df[1,]$original
}
closestToZero <- Vectorize(closestToZero_)
# closestToZero(-1,2,-3)
# closestToZero(-1,2,NA)

# ---------
# binds 2 data frames, row-by-row using the SHORTEST DF as the length-to-use
cbind.to.shortest <- function(df1, df2) {
  minlen <- min(length(df1[[1]]),length(df2[[1]]))
  cbind(head(df1,minlen), head(df2,minlen))
}

# READ CSV DATA =============================
a <- read_csv(inputFilename)
colnames(a) <- c("sample","A","B")
head(a)

# find 0->1 transitions
b1 <- a %>%
  mutate(oneZeroA = ((A == 1) & (lag(A) == 0)),
         oneZeroB = ((B == 1) & (lag(B) == 0)))

zcA1 <- b1 %>% filter(oneZeroA) %>% select(sampleA=sample)  # upward ZC's on A channel
zcB1 <- b1 %>% filter(oneZeroB) %>% select(sampleB=sample)  # upward ZC's on B channel

# find 1->0 transitions
b2 <- a %>%
  mutate(oneZeroA = ((A == 0) & (lag(A) == 1)),
         oneZeroB = ((B == 0) & (lag(B) == 1)))

zcA2 <- b2 %>% filter(oneZeroA) %>% select(sampleA=sample)  # downward ZC's on A channel
zcB2 <- b2 %>% filter(oneZeroB) %>% select(sampleB=sample)  # downward ZC's on A channel

# combine the ups and downs into individual sorted df's
zcA_combined <- rbind(zcA1, zcA2) %>% arrange(sampleA)  # upward/downward ZC's on A channel
zcB_combined <- rbind(zcB1, zcB2) %>% arrange(sampleB)  # upward/downward ZC's on B channel

zc_all <- cbind.to.shortest(zcA_combined, zcB_combined)  # combine into a single df

# Match up the ZC's in A and B channels (looks 1 sample in either direction to match up)
zc <- zc_all %>%
  mutate(A_us = sampleA * usPerSample,
         B_us = sampleB * usPerSample,
         skew_us_lag = A_us-lag(B_us),
         skew_us_zero = A_us-B_us,
         skew_us_lead = A_us-lead(B_us),
         skew_us = closestToZero(skew_us_lag, skew_us_zero, skew_us_lead)) %>%
  filter(!is.na(skew_us)) %>%
  select(A_us, B_us, skew_us)

# head(zc,20)
# tail(zc,20)

# PLOTS ===================================

# FILTER OUT THE OUTLIERS -- on some DUT's, the task driving the GPIO gets interrupted frequently.
#   so, filter those out using a median filter.  WindowSize should be an odd integer.
movingMedianFilter <- function(x,n=5) { runmed(x,n) }
medianWindowSize <- 13  # looks at 13 points, and picks the median (NOTE: non-linear) to get rid of outliers
zc_filtered <- zc
zc_filtered$skew_us <- movingMedianFilter(zc$skew_us, medianWindowSize)

# calculate TP's based on the filtered data
tp2.5_us=TP2.5(zc_filtered$skew_us)
tp50_us=TP50(zc_filtered$skew_us)
tp97.5_us=TP97.5(zc_filtered$skew_us)
N=length(zc_filtered$skew_us)

# HISTOGRAM ----------------
V1_limitLow_us = -5000
V1_limitHigh_us = 5000
V2_limitLow_us = -150
V2_limitHigh_us = 150

histogram_path <- file.path(results_dir, paste0(prefix, "-hist-skew.png"))

png(histogram_path, width=1024, height=800)
  ggplot(zc_filtered, aes(x=skew_us)) +
    geom_histogram(aes(y=..density..), color="black", fill="white", binwidth=25) +
    geom_density(color="red", fill="red", alpha=0.25) +
    coord_cartesian(xlim=c(tp2.5_us * 6, tp97.5_us * 6)) +
    scale_x_continuous(breaks=seq(-10000,10000,100)) +
    geom_vline(xintercept = 0, linetype="dotted", color="black") +
    geom_vline(xintercept = tp50_us, linetype="dashed", color="darkblue") +
    annotate("text", size=4.0, x=c(tp50_us), y=Inf,
             vjust=c(1), color="darkblue",
             label=paste0(c("tp50"),"=\n",round(c(tp50_us),digits=1),"us")) +
    theme(axis.text.x = element_text(angle=90, vjust=0.5)) +
    geom_vline(xintercept=c(tp2.5_us, tp97.5_us), linetype="dashed", color="darkgreen") +
    geom_vline(xintercept=c(V1_limitLow_us, V1_limitHigh_us), linetype="dashed", color="red") +
    geom_vline(xintercept=c(V2_limitLow_us, V2_limitHigh_us), linetype="dashed", color="darkgreen") +
    annotate("text", size=4.0, x=c(tp2.5_us,tp97.5_us), y=Inf,
             vjust=c(3,3), color="darkgreen",
             label=paste0(c("tp2.5","tp97.5"),"=\n",round(c(tp2.5_us,tp97.5_us),digits=1),"us")) +
    # annotate("text", size=4.0, x=c(V1_limitLow_us,V1_limitHigh_us), y=Inf,
    #          vjust=c(1), color="red",
    #          label=paste0(c("V1","V1"),"=\n",round(c(V1_limitLow_us,V1_limitHigh_us),digits=1),"us")) +
    # annotate("text", size=4.0, x=c(V2_limitLow_us,V2_limitHigh_us), y=Inf,
    #          vjust=c(1), color="darkgreen",
    #          label=paste0(c("V2","V2"),"=\n",round(c(V2_limitLow_us,V2_limitHigh_us),digits=1),"us")) +
    xlab("device-to-device skew (us)") +
    ggtitle(paste0("Histogram of delay, N=",N,", (median filtered, windowSize = ",medianWindowSize,")"),
            versionString)
dev.off()

# VsTime PLOT ------------
vstime_path <- file.path(results_dir, paste0(prefix, "-VsTime-skew.png"))

png(vstime_path, width=1024, height=800)
ggplot(zc_filtered, aes(x=A_us/1E6, y=skew_us)) +  # A is the reference device on ch1
  geom_hline(yintercept=0, linetype="solid") +
  geom_line() +
  geom_smooth(color="blue") +
  # coord_cartesian(ylim=c(4.0*tp2.5_us, 4.0*tp97.5_us)) +
  scale_y_continuous(breaks=seq(-10000,10000,500)) +
  scale_x_continuous(breaks=seq(0,600,60)) +
  # geom_hline(yintercept=c(-5000,5000), color="red", linetype="dashed") +
  geom_hline(yintercept=c(-150,150), color="darkgreen", linetype="dashed") +
  annotate("text", size=4.0, x=0, y=c(-150,150),
           vjust=c(-0.5,-0.5), color="darkgreen",
           label=paste0(c("V2","V2"),"=",c(-150,150),"us")) +
  # annotate("text", size=4.0, x=0, y=c(-5000,5000),
  #          vjust=c(-0.5,-0.5), color="red",
  #          label=paste0(c("V1","V1"),"=",c(-5000,5000),"us")) +
  xlab("time-in-test (s)") +
  ggtitle(paste0("Skew of vs time, N=",N,", (median filtered, windowSize = ",medianWindowSize,")"),
          versionString)
dev.off()
