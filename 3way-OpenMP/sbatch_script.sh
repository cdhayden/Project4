#!/bin/bash
for i in 1 2 3 4 5
do

##1 node tests
##sbatch --time=30 --mem=7G --cpus-per-task=1 --threads-per-core=1 --ntasks=1 --nodes=1 --constraint=moles --job-name='1c' program_threads.sh $i
##sbatch --time=30 --mem=7G --cpus-per-task=2 --threads-per-core=1 --ntasks=1 --nodes=1 --constraint=moles --job-name='2c' program_threads.sh $i
##sbatch --time=20 --mem=7G --cpus-per-task=4 --threads-per-core=1 --ntasks=1 --nodes=1 --constraint=moles --job-name='4c' program_threads.sh $i
##sbatch --time=10 --mem=7G --cpus-per-task=8 --threads-per-core=1 --ntasks=1 --nodes=1 --constraint=moles --job-name='8c' program_threads.sh $i
##sbatch --time=5 --mem=7G --cpus-per-task=16 --threads-per-core=1 --ntasks=1 --nodes=1 --constraint=moles --job-name='16c' program_threads.sh $i

##2 node tests
##sbatch --time=5 --mem=7G --cpus-per-task=2 --threads-per-core=1 --ntasks=2 --nodes=2 --constraint=moles --job-name='4c-2sys' program_threads.sh $i
##sbatch --time=5 --mem=7G --cpus-per-task=4 --threads-per-core=1 --ntasks=2 --nodes=2 --constraint=moles --job-name='8c-2sys' program_threads.sh $i
##sbatch --time=5 --mem=7G --cpus-per-task=8 --threads-per-core=1 --ntasks=2 --nodes=2 --constraint=moles --job-name='16c-2sys' program_threads.sh $i
 
##varied input tests
##sbatch --time=30 --mem=7G --cpus-per-task=4 --threads-per-core=1 --ntasks=1 --nodes=1 --constraint=moles --job-name='4c-100' program_threads.sh $i 100
##sbatch --time=30 --mem=7G --cpus-per-task=4 --threads-per-core=1 --ntasks=1 --nodes=1 --constraint=moles --job-name='4c-1000' program_threads.sh $i 1000
##sbatch --time=30 --mem=7G --cpus-per-task=4 --threads-per-core=1 --ntasks=1 --nodes=1 --constraint=moles --job-name='4c-10000' program_threads.sh $i 10000
##sbatch --time=30 --mem=7G --cpus-per-task=4 --threads-per-core=1 --ntasks=1 --nodes=1 --constraint=moles --job-name='4c-100000' program_threads.sh $i 100000
##sbatch --time=30 --mem=7G --cpus-per-task=4 --threads-per-core=1 --ntasks=1 --nodes=1 --constraint=moles --job-name='4c-1000000' program_threads.sh $i 1000000

done