#!/bin/bash
THREADS=${SLURM_CPUS_PER_TASK:-1}
JOB=${SLURM_JOB_NAME}

# core tests
/usr/bin/time -v ./pthreads ~dan/625/wiki_dump.txt $THREADS 1000000 > ./outputs/program_output_"$1".txt 2> ./stats/time_stats_"$JOB"_"$1".txt
perf stat -d ./pthreads ~dan/625/wiki_dump.txt $THREADS 1000000 > ./outputs/program_output_"$1".txt 2>> ./stats/time_stats_"$JOB"_"$1".txt

# input size tests
##/usr/bin/time -v ./pthreads ~dan/625/wiki_dump.txt $THREADS $2 > ./outputs/program_output_"$1".txt 2> ./stats/time_stats_"$JOB"_"$1".txt
##perf stat -d ./pthreads ~dan/625/wiki_dump.txt $THREADS $2 > ./outputs/program_output_"$1".txt 2>> ./stats/time_stats_"$JOB"_"$1".txt