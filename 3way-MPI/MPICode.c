//Using POSIX file system
//make file not made yet

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

int MAX_THREADS = 1;  // to change with core count
int MAX_INPUT = 1; // vary input size
int MAX_LINES = 1; // vary amount of Lines total

// Structure to pass data to each thread
typedef struct {
    int thread_id;
    char *data;
    long start_offset;
    long end_offset;
    int line_start;
    int *results;
} ThreadData;

int *find_max_ascii(char *src, int size) {
    int* result = calloc(size+2, sizeof(char));
	int lines = 2;
	int max_value = 0;
	bool hold = false;
	for(int i  = 0; i < size && src[i] != '\0'; i++){

		char c = src[i];
		 if (c == '\n') {
			result[lines] = max_value;
			lines++;
			max_value = 0;
			hold = false;
		 }
		 else {
            if ((unsigned char)c > max_val) {
                max_val = (unsigned char)c;
            }
			 hold = true;
        }

	}
	if(hold){

		result[1] = 1;
	}
	result[0] = lines;
	return result

	
}

int main(int argc, char *argv[]) {

    
    
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <file_path> <thread_count> <input_size> <max_lines>\n", argv[0]);
        return EXIT_FAILURE;
    }

// Initialize the MPI environment
	MPI_Init(&argc, &argv);

    // Get the rank of the process
	int pid;
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);

	// Get the number of processes
	int number_of_processes;
	MPI_Comm_size(MPI_COMM_WORLD, &number_of_processes);

    if(pid == 0){

        sscanf(argv[2], "%d", &MAX_THREADS); //Read into the arguments
        sscanf(argv[3], "%d", &MAX_INPUT);
        sscanf(argv[4], "%d", &MAX_LINES);
    
        int fd = open(argv[1], O_RDONLY); //Open the file to read
        if (fd < 0) {
            perror("Failed to open file");
            return EXIT_FAILURE;
        }

        struct stat sb; //Get the stats of the file for filesize
        if (fstat(fd, &sb) == -1) {
            perror("fstat");
            close(fd);
            return EXIT_FAILURE;
        }

        int *results = calloc(MAX_LINES, sizeof(int)); // depending on Max lines
        int resline = 0; //Line of the results buffer
        
        if(sb.file_size/(NUM_THREADS-1) <= MAX_INPUT){ //If the file is smaller than expected input is larger than expected

            int chunk = sb.file_size/(NUM_THREADS-1); //Find how much to read per input
			for(int i = 1; i < MAX_THREADS-1; i++){
				MPI_Send(&chunk, 1, MPI_INT, i, 0, MPI_COMM_WORLD); //Send the buffer read to another branch
			}
            int remainer = sb.file_size%(NUM_THREADS-1); //Find the remainer for the last thread to complete
			MPI_Send(&remainer, 1, MPI_INT, MAX_THREADS - 1, 0, MPI_COMM_WORLD);
            char *buffer = calloc(chunk+1, sizeof(char)); // depending on input size

            bool hold = false; //Hold varible for later in the for loops to check for non terminating lines
        
            int piter = 1; //Iterator Varible for the processes
            while (piter < MAX_THREADS-1){
                read(fd, results, chunk);
                buffer[chunk] = '\0'; //Set the last char to a null terminator for string reading
                MPI_Send(buffer, chunk+1, MPI_CHAR, piter, 0, MPI_COMM_WORLD); //Send the buffer read to another branch
                piter++; //Increment the process counter
            }
            read(fd, results, remainer);
            buffer[remainer] = '\0'; //Set the last char to a null terminator for string reading
            MPI_Send(buffer, remainer, MPI_CHAR, piter, 0, MPI_COMM_WORLD); //Send the buffer read to another branch
            
            for(int i = 1; i < MAX_THREADS; i++){
                int size = 0;
                MPI_Recv(&size, 1, MPI_INT, i, 0,  MPI_COMM_WORLD);
                int *cast = calloc(size, sizeof(int)); // depending on Max lines
                MPI_Recv(cast, size, MPI_INT, i, 0, MPI_COMM_WORLD);
                    
                if(hold){//If hold is true
                    if(results[resline-1] < cast[2]) { //If the incomplete line's value is less than the rest of the line, update it
                
                            results[resline-1] = cast[2];        
                    }
                    for(int j = 3; j < size; j++){

                        results[resline] = cast[j];
                        resline++;
                    }
                    hold = false;
                }
                else{
                    for(int j = 2; j < size; j++){

                        results[resline] = cast[j];
                        resline++;   
                    }
                }
                if(cast[1] == 1) //If the line doesnt terminate, make a note for the next iteration
                {
                    hold = true;
                }
                free(cast);
            }
            free(buffer);
        }
        else{
			int psize = MAX_INPUT;
			for(int i = 1; i < MAX_THREADS; i++){
				MPI_Send(&psize, 1, MPI_INT, i, 0, MPI_COMM_WORLD); //Send the buffer read to another branch
			}
            char *buffer = calloc(MAX_INPUT, sizeof(char)); // depending on input size
            bool done = false; //Bool to check if method is done
            bool hold = false; //Hold varible for later in the for loops to check for non terminating lines
            while(!done){ //While not done
        
                int piter = 1; //Iterator Varible for the processes
                while (piter < MAX_THREADS && read(fd, results, MAX_INPUT - 1) != 0){

                    buffer[MAX_INPUT-1] = '\0'; //Set the last char to a null terminator for string reading
                    MPI_Send(buffer, MAX_INPUT, MPI_CHAR, piter, 0, MPI_COMM_WORLD); //Send the buffer read to another branch
                    piter++; //Increment the process counter
                }
                if(piter < MAX_THREADS) {//If the read got zero, (If both while conditons above occur, it'll loop for no downside)
                    done = true;
                } 
                for(int i = 1; i < piter; i++){

                    int size = 0;
                    MPI_Recv(&size, 1, MPI_INT, i, 0,  MPI_COMM_WORLD);
                    int *cast = calloc(size, sizeof(int)); // depending on Max lines
                    MPI_Recv(cast, size, MPI_INT, i, 0, MPI_COMM_WORLD);

                    if(hold){//If hold is true
                        if(results[resline-1] < cast[2]) { //If the incomplete line's value is less than the rest of the line, update it
                            results[resline-1] = cast[2];
                        }
                        for(int j = 3; j < size; j++){

                            results[resline] = cast[j];
                            resline++;
                        }
                        hold = false;
                    }
                    else{
                        for(int j = 2; j < size; j++){

                            results[resline] = cast[j];
                            resline++;
                        }
                    }
                    if(cast[1] == 1) //If the line doesnt terminate, make a note for the next iteration
                    {
                        hold = true;
                    }
                    free(cast);
                }
            }
            free(buffer);
        }

		char* temp = calloc(MAX_INPUT, sizeof(char));
		temp[0] = '\0';
		temp[1] = '\0';
		for(int i = 1; i < MAX_THREADS; i++){
			MPI_Send(temp, MAX_INPUT, MPI_CHAR, i, 0, MPI_COMM_WORLD); //Send the buffer read to another branch
		}

		free(temp);
	
	    
        for (int i = 0; i < MAX_LINES; i++) {
            if (results[i] != 0) {
                printf("%d: %d\n", i, results[i]);
            }
        }
        free(results);

		
    	close(fd);
	    
        
    }
    else{

        sscanf(argv[2], "%d", &MAX_THREADS); //Read into the arguments
        sscanf(argv[3], "%d", &MAX_INPUT);
        sscanf(argv[4], "%d", &MAX_LINES);

		bool done = false;
		int size = 0;
		MPI_Recv(&size, 1, MPI_INT, 0, 0,  MPI_COMM_WORLD);

		char* buffer = calloc(size, sizeof(char));
		
		while(!done){

			MPI_Recv(buffer, size, MPI_CHAR, 0,0, MPI_COMM_WORLD); 

			if(buffer[0] == '\0' && buffer[1] == '\0') 
			{
				done = true;
			}
				
			else
			{
				int* result = find_max_ascii(buffer, size);

				MPI_Send(&result[0], 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
				MPI_Send(result, result[0], MPI_INT, 0, 0, MPI_COMM_WORLD);

				free(result);
			}
			
			

		}

		free(buffer);
        

    }



    
    
    MPI_Finalize();

    return EXIT_SUCCESS;
}
