#!/bin/bash

sbatch --time=10 --mem=7G --threads-per-core=1 --ntasks-per-node=4 --nodes=1 --constraint=moles --job-name='4c' program_threads.sh 1

