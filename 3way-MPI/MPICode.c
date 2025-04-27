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

int *find_max_ascii(char *src, int size) {
	
    int* result = calloc(size+2, sizeof(char)); //Get space for result size

	
	int lines = 2;//Get the number of lines, defaults to two for the size and holding var
	
	int max_value = 0; // Sets the max value of a line

	
	int hold = 0; //Bool that checks if theres a hold over

	int setter = 0;
	for(int i  = 0; i < size && src[i] != '\0'; i++){ //For loop that checks over size and the source to see if null terminator. 

		setter = i;
		char c = src[i];//Get the current character
		
		 if (c == '\n') { //If its a new lines,

			 
			result[lines] = max_value; //Report the max value to the results
			lines++; //Increment lines
			max_value = 0; //Reset the max value
			hold = 0; //And reset hold to zero
		 }
			 
		 else {//If its another character
			 
            if ((unsigned char)c > max_value) { //Check if the unsigned char is greater than the max value
                max_value = (unsigned char)c; //And update max_value if it is
            }
			 hold = 1; //Set the hold to one to say we're at the end
        }

	}

	if(max_value != 0) //If theres a left over value
	{
		results[lines] = max_value; //Set the last line to it,
		lines++; //Increment lines
	}
	


	result[1] = hold; //Set the hold varible to hold
	
	result[0] = lines; //Set the size varible to the current size
	
	return result;//Return the results
	

	
}

int main(int argc, char *argv[]) {

    
    //Check for amount of arguments
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <file_path> <thread_count> <input_size> <max_lines>\n", argv[0]);
        return EXIT_FAILURE;
    }

	sscanf(argv[2], "%d", &MAX_THREADS); //Read into the arguments
    sscanf(argv[3], "%d", &MAX_INPUT);
    sscanf(argv[4], "%d", &MAX_LINES);

// Initialize the MPI environment
	MPI_Init(&argc, &argv);

    // Get the rank of the process
	int pid;
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);

	// Get the number of processes
	int number_of_processes;
	MPI_Comm_size(MPI_COMM_WORLD, &number_of_processes);


	//Report that the proccesses started
	 printf("Rank %d: Active\n", pid); 

    if(pid == 0){ //Check the rank to see if its zero, if so its the master process

        
    
        int fd = open(argv[1], O_RDONLY); //Open the file to read
		
        if (fd < 0) { //Make sure it opened
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
        
        if(sb.st_size/(MAX_THREADS-2) <= MAX_INPUT){ //If the file is small enough 

            int chunk = sb.st_size/(MAX_THREADS-1); //Find how much to read per input

			chunk++;
			for(int i = 1; i < MAX_THREADS-1; i++){ //To all threads but the last one
				printf("Rank %d: sending data to %d\n", pid, i); 
				MPI_Send(&chunk, 1, MPI_INT, i, 0, MPI_COMM_WORLD); //Send the size of each buffer
			}
			chunk--;
			
            int remainer = sb.st_size%(MAX_THREADS-2); //Find the remainer for the last thread to complete
			
			printf("Rank %d: sending data to %d\n", pid, MAX_THREADS - 1);  

			remainer++;
			MPI_Send(&remainer, 1, MPI_INT, MAX_THREADS - 1, 0, MPI_COMM_WORLD); //Send the size to the remainder handling buffer
			remainer--;
			
            char *buffer = calloc(chunk+1, sizeof(char)); // depending on input size, create the buffer
        
            int piter = 1; //Iterator Varible for the processes
			
            while (piter < MAX_THREADS-1){ //While the process iterator is less than the last one
				
                read(fd, results, chunk); //Read in some amount fromthe file
				
                buffer[chunk] = '\0'; //Set the last char to a null terminator for string reading
				
		    	printf("Rank %d: sending data to %d\n", pid, piter); 
				
                MPI_Send(buffer, chunk+1, MPI_CHAR, piter, 0, MPI_COMM_WORLD); //Send the buffer read to another branch
				
                piter++; //Increment the process counter
            }
			
            remainer = read(fd, results, remainer); //Read in the remainder of the file
			
            buffer[remainer] = '\0'; //Set the last char to a null terminator for string reading
			
			printf("Rank %d: sending data to %d\n", pid, piter); 
			
            MPI_Send(buffer, remainer, MPI_CHAR, piter, 0, MPI_COMM_WORLD); //Send the buffer containing the rest to the 

			int hold = 0; //Hold varible for later in the for loops to check for non terminating lines
			
            for(int i = 1; i < MAX_THREADS; i++){ //Start reciving information from processes
				
                int size = 0; //Create size for the varible
				
		    printf("Rank %d: reciving size from %d\n", pid, i);
				
                MPI_Recv(&size, 1, MPI_INT, i, 0,  MPI_COMM_WORLD,MPI_STATUS_IGNORE); //Reciveve the size from the current process
				
                int *cast = calloc(size, sizeof(int)); // create cast buffer using that input size
				
		    	printf("Rank %d: reciving data from %d\n", pid, i);
				
                MPI_Recv(cast, size, MPI_INT, i, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE); //Get the cast buffer from the current process
                    
                if(hold == 1){//If hold is true
					
                    if(results[resline-1] < cast[2]) { //If the incomplete line's value is less than the rest of the line, update it
                
                            results[resline-1] = cast[2];        
                    }
					
                    for(int j = 3; j < size; j++){ //Read the rest of the array into the results array

                        results[resline] = cast[j];
                        resline++;
                    }
                    hold = 0; //Reset hold to zero
                }
                else{
                    for(int j = 2; j < size; j++){ //Read in the entire file into the results array

                        results[resline] = cast[j];
                        resline++;   
                    }
                }
				
                hold = cast[1];//Update hold to the hold variable
                
                free(cast); //Free the cast buffer
            }

			
            free(buffer); //Free the buffer
        }
        else{ //If the file is too large
			
			int psize = MAX_INPUT; //Set the size to the input size limit
			
			for(int i = 1; i < MAX_THREADS; i++){ //Go through each process
				
				printf("Rank %d: sending data to %d\n", pid, i); 
				
				MPI_Send(&psize, 1, MPI_INT, i, 0, MPI_COMM_WORLD); //Send the szie input to each of them
			}
            char *buffer = calloc(MAX_INPUT, sizeof(char)); // create the buffer to be the size of the input

			
            int done = 0; //Bool to check if method is done
			
            int hold = 0; //Hold varible for later in the for loops to check for non terminating lines
			
            while(done==0){ //While not done
        
                int piter = 1; //Iterator Varible for the processes
				
                while (piter < MAX_THREADS && read(fd, results, MAX_INPUT - 1) != 0){ //While the process iterator has not gone through all of them, and read is still getting bytes read

                    buffer[MAX_INPUT-1] = '\0'; //Set the last char to a null terminator for string reading
					
					printf("Rank %d: sending data to %d\n", pid, piter); 
					
                    MPI_Send(buffer, MAX_INPUT, MPI_CHAR, piter, 0, MPI_COMM_WORLD); //Send the buffer to be read in the current process 
                    piter++; //Increment the process counter
					
                }
				
                if(piter < MAX_THREADS) {//If the read got zero, (If both while conditons above occur, it'll loop for no downside), then we're done
                    done = 1;
                } 
				
                for(int i = 1; i < piter; i++){ //For each process that has information in it

                    int size = 0; //Get the size variable created
					
			printf("Rank %d: recieving size from %d\n", pid, i); 
					
                    MPI_Recv(&size, 1, MPI_INT, i, 0,  MPI_COMM_WORLD,MPI_STATUS_IGNORE); //Recieve the size of the return array from the current process
					
                    int *cast = calloc(size, sizeof(int)); // create the cast depending on the input size
					
			printf("Rank %d: recieving data from %d\n", pid, i); 
					
                    MPI_Recv(cast, size, MPI_INT, i, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE); //Read in the array form the current process

					
                    if(hold == 1){//If hold is true
						
                        if(results[resline-1] < cast[2]) { //If the incomplete line's value is less than the rest of the line, update it
                            results[resline-1] = cast[2];
                        }
                        for(int j = 3; j < size; j++){ //Then read in the rest of the array

                            results[resline] = cast[j];
                            resline++;
                        }
                        hold = 0; //Update hold
                    }
                    else{ //If there is no hold
						
                        for(int j = 2; j < size; j++){  //Read in all of the array

                            results[resline] = cast[j];
                            resline++;
                        }
                    }
                    
                    hold = cast[1]; //Update the hold varible
                    
                    free(cast); //Free the cast
                }
            }
            free(buffer);//Free the buffer once done.
        }

		char* temp = calloc(2, sizeof(char));//Create a new temp array
		temp[0] = '\0'; //Set the first two spots of it to null terminators
		temp[1] = '\0';
		for(int i = 1; i < MAX_THREADS; i++){ //Go through each process.
			
			printf("Rank %d: sending termination to %d\n", pid, i); 
			
			MPI_Send(temp, 2, MPI_CHAR, i, 0, MPI_COMM_WORLD); //Send the kill buffer to each process.
		}

		free(temp); //Free the temp array.
	
	    
        for (int i = 0; i < MAX_LINES; i++) { //Go through the max lines
			
            if (results[i] != 0) { //If the results arent emtpty
				
                printf("%d: %d\n", i, results[i]); //print to the standart output
				
            }
        }
        free(results); //Free the results

		
    	close(fd); //Close the file.
	    
        
    }
    else{

		int done = 0; //Bool var to check if the process is done
		int size = 0; //The current size to check
	    
	    printf("Rank %d: receive size from %d\n", pid, 0);  
	    
		MPI_Recv(&size, 1, MPI_INT, 0, 0,  MPI_COMM_WORLD,MPI_STATUS_IGNORE);//Read in the size for the process

		if(size == 1) size = 2; //If the size is 1, set it for two for null terminator purposes.
		
		char* buffer = calloc(size, sizeof(char)); //Create a buffer for it to read data in.
		
		while(done == 0){ //While its not done

			printf("Rank %d: receive data from %d\n", pid, 0);
			MPI_Recv(buffer, size, MPI_CHAR, 0,0, MPI_COMM_WORLD,MPI_STATUS_IGNORE); //Read in the buffer

			if(buffer[0] == '\0' && buffer[1] == '\0')  //Check for null terminators for the termination
			{
				done = 1; //If so we're done
			}
				
			else //Otherwise if we have valid data
			{
				int* result = find_max_ascii(buffer, size); //Run the data through the ascii sorter

				printf("Rank %d: sending size to %d\n", pid, 0);
				MPI_Send(&result[0], 1, MPI_INT, 0, 0, MPI_COMM_WORLD); //Send the size to the main process
				
				printf("Rank %d: sending data to %d\n", pid, 0);
				MPI_Send(result, result[0], MPI_INT, 0, 0, MPI_COMM_WORLD); //Send the array to the main process.

				free(result); //Free the results array
			}
			
			

		}

		free(buffer);//free the buffer once done
        

    }



    
    
    MPI_Finalize(); //Finailaize MPI

    return EXIT_SUCCESS;
}
