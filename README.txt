Once in a directory for a specific implementation, run the "make" command to compile the program.

If wanting to perform a simple run of the program without getting statistics, you may run the following
in their respective subdirectories:

./pthreads ~dan/625/wiki_dump.txt <# of threads as int> <# of lines to read as int>
mpiexec ./MPICode ~dan/625/wiki_dump.txt <# of threads as int> <# of lines to read as int>
./OpenMP ~dan/625/wiki_dump.txt <# of threads as int> <# of lines to read as int>

If submitting a job to beocat, open sbatch_script.sh and uncomment the test to be performed,
which will allow for submission of the job 5 times and generation of statistics (in the "stats" 
subdirectory). Program output for each trial will also be generated in the "outputs" subdirectory. 
run "sh sbatch_script.sh" to do this on the specific test. Only run one test and wait until 
all trials conclude running before running/uncommenting any others. Also, if using an input 
size test, you will also need to comment out the "core" test and uncomment out the "input size" 
test from the program_threads.sh file. 