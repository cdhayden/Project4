#!/bin/bash
THREADS=${SLURM_NTASKS_PER_NODE:-1}

./pthreads ~dan/625/wiki_dump.txt $THREADS 10000 > program_output.txt 

