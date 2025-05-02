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

int MAX_INPUT = 1; // vary input size
int MAX_LINES = 1; // vary amount of Lines total

int main(int argc, char *argv[]) {

    
    //Check for amount of arguments
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <file_path> <input_size> <max_lines>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    printf("Program Started\n"); 

	
	



    // Initialize the MPI environment
	MPI_Init(&argc, &argv);

    // Get the rank of the process
	int pid;
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);

	// Get the number of processes
	int number_of_processes;
	MPI_Comm_size(MPI_COMM_WORLD, &number_of_processes);


	    sscanf(argv[2], "%d", &MAX_INPUT); //Read into the arguments
    	    sscanf(argv[3], "%d", &MAX_LINES);


	//Report that the proccesses started
	 printf("Rank %d: Active\n", pid); 

    if(pid == 0){ //Check the rank to see if its zero, if so its the master process


        printf("Rank %d: Master Declared\n", pid);
        
        int fd = open(argv[1], O_RDONLY); //Open the file to read
		
        if (fd < 0) { //Make sure it opened
            perror("Failed to open file");
            return EXIT_FAILURE;
        }
        else{
            
            printf("Rank %d: Master Opened File\n", pid);
        }

	struct stat sb;
    	if (fstat(fd, &sb) == -1) {
        	perror("fstat");
        	close(fd);
        	return EXIT_FAILURE;
    	}
    	
    int chunk = sb.st_size/number_of_processes;	

	if(chunk > MAX_INPUT) {
	
	chunk = MAX_INPUT;
	
	}
	
	for(int i = 1; i < number_of_processes; i++){
	
	printf("Rank %d: Master sending size %d to %d\n", pid, chunk, i); 
	 MPI_Send(&chunk, 1, MPI_INT, i, 0, MPI_COMM_WORLD); //Send the buffer to be read in the current process 
	
	}
        
        int *results = calloc(MAX_LINES, sizeof(int));
		int* result = calloc(chunk+2, sizeof(char)); //Get space for result size
        
        printf("Rank %d: Master Reading File\n", pid);
        char *buffer = calloc(chunk, sizeof(char));
        
        int done = 0;
        int hold = 0;
        int piter = 1;
        int resline = 0;
        
        while(done == 0){
            
             while(piter < number_of_processes && read(fd, buffer, chunk - 1 ) != 0 && resline < MAX_LINES){
            
            buffer[chunk-1] = '\0'; //Set the last char to a null terminator for string reading
					
			printf("Rank %d: Master sending data to %d\n", pid, piter); 
					
            MPI_Send(buffer, chunk, MPI_CHAR, piter, 0, MPI_COMM_WORLD); //Send the buffer to be read in the current process 
                    
            printf("Rank %d: data to %d sent\n", pid, piter); 
            piter++; //Increment the process counter
            
            
            }
            
            if(read(fd, buffer, chunk - 1) == 0) {
				done = 1;
			}
			else{
				buffer[chunk-1] = '\0';
				
				printf("Rank %d: find_max_ascii Started\n", pid); 
	
	
	            int lines = 2;//Get the number of lines, defaults to two for the size and holding var
	
	            int max_value = 0; // Sets the max value of a line

	
	            int hold = 0; //Bool that checks if theres a hold over

	            for(int i  = 0; i < chunk && buffer[i] != '\0'; i++){ //For loop that checks over size and the source to see if null terminator. 

		            char c = buffer[i];//Get the current character
		
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
		            result[lines] = max_value; //Set the last line to it,
		            lines++; //Increment lines
	            }
	


	            result[1] = hold; //Set the hold varible to hold
	
	            result[0] = lines; //Set the size varible to the current size
	
	            printf("Rank %d: find_max_ascii Done\n",pid);
	
	
	            


				
				
			}
            
            for(int i = 1; i < piter; i++){
                
                int size = 0;
                
                printf("Rank %d: Master reciving size from %d\n", pid, i); 
                MPI_Recv(&size, 1, MPI_INT, i, 0 ,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("Rank %d: Master recieved size from %d: %d\n", pid, i, size); 
                
                int *resarray = calloc(size, sizeof(int));
                
                printf("Rank %d: Master reciving data from %d\n", pid, i);
                MPI_Recv(resarray, size, MPI_INT, i, 0 ,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("Rank %d: Master recieved data from %d\n", pid, i); 
                
                if(hold == 1){
                    
                    if(resarray[2] > results[(resline - 1)]){
                        
                        results[(resline - 1)] = resarray[2];
                        
                    }
                    for(int j = 3; j < size && resline < MAX_LINES; j++){
                        
                        results[resline] = resarray[j];
                        resline++;
                    }
                    
                }
                else{
                    for(int j = 2; j < size && resline < MAX_LINES; j++){
                        
                        results[resline] = resarray[j];
                        resline++;
                    }
                    
                }
                
                hold = resarray[1];
                
            }

			if(done == 0){
			
			int size = result[0];

			if(hold == 1){
                    
                    if(result[2] > results[(resline - 1)]){
                        
                        results[(resline - 1)] = result[2];
                        
                    }
                    for(int j = 3; j < size && resline < MAX_LINES; j++){
                        
                        results[resline] = result[j];
                        resline++;
                    }
                    
                }
                else{
                    for(int j = 2; j < size && resline < MAX_LINES; j++){
                        
                        results[resline] = result[j];
                        resline++;
                    }
                    
                }
                
                hold = result[1];
			
			}
                
            piter = 1;
                
            
            
        }

		free(result);
        
        buffer[0] = 'a';
        buffer[1] = 'a';
        buffer[2] = 'a';
        buffer[3] = 'a';
        for(int i = 1; i < number_of_processes; i++){
            
            printf("Rank %d: Master sending kill to %d\n", pid, i); 
            MPI_Send(buffer, chunk, MPI_CHAR, i, 0, MPI_COMM_WORLD); //Send the buffer to be read in the current process 
            
        }
        
        free(buffer);
        
        for(int i = 0; i < resline; i++){
            printf("Line %d: %d\n", i, results[i]);
        }
        
        
        free(results);
        
        close(fd);
        printf("Rank %d: Master Closed File\n", pid);
        
    
        
	    
        
    }
    else{

        printf("Rank %d: Child Declared\n", pid);
        
        int chunk = 0;
        printf("Rank %d: Child recieving size \n", pid); 
        MPI_Recv(&chunk, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank %d: Child recieved size %d\n", pid, chunk); 
        
        int kill = 0;
        char *buffer = calloc(chunk, sizeof(char));
        while(kill != 1)
        {
           printf("Rank %d: child reciving data from Master\n", pid); 
           MPI_Recv(buffer, chunk, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
           printf("Rank %d: child recived data from Master\n", pid); 
           if(buffer[0] == 'a' && buffer[1] == 'a' && buffer[2] == 'a' && buffer[3] == 'a'){
               
               kill = 1;
               printf("Rank %d: child recived kill from Master\n", pid); 
               
           }
           else{
               
               	printf("Rank %d: find_max_ascii Started\n", pid); 
	
                int* result = calloc(chunk+2, sizeof(char)); //Get space for result size

	
	            int lines = 2;//Get the number of lines, defaults to two for the size and holding var
	
	            int max_value = 0; // Sets the max value of a line

	
	            int hold = 0; //Bool that checks if theres a hold over

	            for(int i  = 0; i < chunk && buffer[i] != '\0'; i++){ //For loop that checks over size and the source to see if null terminator. 

		            char c = buffer[i];//Get the current character
		
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
		            result[lines] = max_value; //Set the last line to it,
		            lines++; //Increment lines
	            }
	


	            result[1] = hold; //Set the hold varible to hold
	
	            result[0] = lines; //Set the size varible to the current size
	
	            printf("Rank %d: find_max_ascii Done\n",pid);
	
	
	            printf("Rank %d: child sending size to Master: %d\n", pid, result[0]); 
	            MPI_Send(result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	            printf("Rank %d: child sent size from Master\n", pid); 
	            
	            printf("Rank %d: child sending data to Master\n", pid); 
	            MPI_Send(result, result[0], MPI_INT, 0, 0, MPI_COMM_WORLD);
	            printf("Rank %d: child sent data to Master\n", pid); 
	            
	            free(result);
               
                }
            }
        
        free(buffer);
        printf("Rank %d: child closing.\n", pid);
        
        
    }



    
    
    MPI_Finalize(); //Finailaize MPI

    return EXIT_SUCCESS;
}
