#!/bin/bash
THREADS=$(SLURM_CPUS_PER_TASK:-1)

./pthreads ~dan/625/wiki_dump.txt $THREADS
./pthreads ~dan/625/wiki_dump.txt $THREADS
./pthreads ~dan/625/wiki_dump.txt $THREADS
