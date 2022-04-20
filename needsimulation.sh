#!/bin/bash

SCENARIO="scratch/need_simulation"


STARTRUN=1 #Run number to start serie
MAXRUNS=20 #Number of runs
nUE=60
arguments="--nUE=$nUE"

for ((run=$STARTRUN; run<=$MAXRUNS; run++))
do
  ./waf --run "$SCENARIO $arguments --RngRun=3" 
done

nUE=70
arguments="--nUE=$nUE"

for ((run=$STARTRUN; run<=$MAXRUNS; run++))
do
  ./waf --run "$SCENARIO $arguments --RngRun=3" 
done

nUE=80
arguments="--nUE=$nUE"

for ((run=$STARTRUN; run<=$MAXRUNS; run++))
do
  ./waf --run "$SCENARIO $arguments --RngRun=3" 
done

nUE=90
arguments="--nUE=$nUE"

for ((run=$STARTRUN; run<=$MAXRUNS; run++))
do
  ./waf --run "$SCENARIO $arguments --RngRun="$run 
done
