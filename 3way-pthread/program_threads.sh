#!/bin/bash
THREADS=${SLURM_CPUS_PER_TASK:-1}
JOB=${SLURM_JOB_NAME}

# Record the start time
start_time=$(date +%s)

# Run the program and collect resource stats
/usr/bin/time -v ./pthreads ~dan/625/wiki_dump.txt $THREADS > program_output.txt 2> ./stats/time_stats_"$JOB"_"$1".txt
##/usr/bin/time -v perf stat -d -o ./stats/time_stats_"$JOB"_"$1".txt ./pthreads ~dan/625/wiki_dump.txt $THREADS

perf stat -d ./pthreads ~dan/625/wiki_dump.txt $THREADS > program_output.txt 2>> ./stats/time_stats_"$JOB"_"$1".txt
##/usr/bin/time -v perf stat -d ./pthreads ~dan/625/wiki_dump.txt $THREADS > ./stats/time_stats_"$JOB"_"$1".txt 2>> ./stats/time_stats_"$JOB"_"$1".txt 1> ./program_output.txt


# Record the end time
end_time=$(date +%s)

# Calculate total elapsed time
total_time=$((end_time - start_time))
echo "Total Time: $total_time seconds" >> ./stats/time_stats_"$JOB"_"$1".txt
