#!/bin/bash

SCENARIO="scratch/need_prose_simulation"


STARTRUN=1 #Run number to start serie
MAXRUNS=5 #Number of runs
nUE=20
arguments="--nUE=$nUE"

for ((run=$STARTRUN; run<=$MAXRUNS; run++))
do
  ./waf --run "$SCENARIO $arguments --RngRun=3" 
done

nUE=30
arguments="--nUE=$nUE"

for ((run=$STARTRUN; run<=$MAXRUNS; run++))
do
  ./waf --run "$SCENARIO $arguments --RngRun=3" 
done

nUE=40
arguments="--nUE=$nUE"

for ((run=$STARTRUN; run<=$MAXRUNS; run++))
do
  ./waf --run "$SCENARIO $arguments --RngRun=3" 
done

nUE=50
arguments="--nUE=$nUE"

for ((run=$STARTRUN; run<=$MAXRUNS; run++))
do
  ./waf --run "$SCENARIO $arguments --RngRun="$run 
done
