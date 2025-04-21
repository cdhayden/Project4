#!/bin/bash
sbatch -time=30 --mem=7G --cpus-per-task=1 --threads-per-core=1 --ntasks=1 --nodes=1 --constraint=moles --job-name='1c' ./program_thread.sh

