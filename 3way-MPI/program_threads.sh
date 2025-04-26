#!/bin/bash

THREADS=${SLURM_NTASKS_PER_NODE:-1}

./MPICode ~dan/625/wiki_dump.txt $THREADS 512000000 10000 > program_output.txt 

