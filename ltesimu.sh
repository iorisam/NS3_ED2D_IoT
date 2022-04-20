#!/bin/bash

SCENARIO="scratch/ltesimu2"


STARTRUN=1 #Run number to start serie
MAXRUNS=5 #Number of runs

nodeSpeed=1
nUE=20
nLR=20
nbS=10

arguments="--nLR=$nLR --nUE=$nUE --nbS=$nbS"

for ((run=$STARTRUN; run<=$MAXRUNS; run++))
do
  ./waf --run "$SCENARIO $arguments --RngRun="$run 
done

nodeSpeed=1
nUE=30
nLR=30
nbS=10

arguments="--nLR=$nLR --nUE=$nUE --nbS=$nbS"

for ((run=$STARTRUN; run<=$MAXRUNS; run++))
do
  ./waf --run "$SCENARIO $arguments --RngRun="$run 
done

nodeSpeed=1
nUE=40
nLR=40
nbS=10

arguments="--nLR=$nLR --nUE=$nUE --nbS=$nbS"

for ((run=$STARTRUN; run<=$MAXRUNS; run++))
do
  ./waf --run "$SCENARIO $arguments --RngRun="$run 
done

nodeSpeed=1
nUE=50
nLR=50
nbS=10

arguments="--nLR=$nLR --nUE=$nUE --nbS=$nbS"

for ((run=$STARTRUN; run<=$MAXRUNS; run++))
do
  ./waf --run "$SCENARIO $arguments --RngRun="$run 
done
