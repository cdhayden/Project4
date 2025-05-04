#define _GNU_SOURCE
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <mpi.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int MAX_THREADS = 1;  // to change with core count
int MAX_LINES = 1; // vary input size

int *find_max_ascii(char *buffer, long size, int *line_count)
{
    long capacity = 4096; // Start small, grow if needed
    int *results = malloc(sizeof(int) * capacity);

    if (!results)
    {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    int result_idx = 0;
    long i = 0;
    int max_val = 0;

    while (i < size)
    {
        char c = buffer[i++];
        if (c == '\n' || i == size)
        {
            if (result_idx >= capacity)
            {
                capacity *= 2;
                results = realloc(results, sizeof(int) * capacity);
                if (!results)
                {
                    perror("realloc failed");
                    exit(EXIT_FAILURE);
                }
            }
            results[result_idx++] = max_val;
            max_val = 0;
        }
        else
        {
            if ((unsigned char)c > max_val)
            {
                max_val = (unsigned char)c;
            }
        }
    }

    *line_count = result_idx;
    return results;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    sscanf(argv[2], "%d", &MAX_THREADS);
    sscanf(argv[3], "%d", &MAX_LINES);

    MPI_Init(&argc, &argv);

    // Get the rank of the process
    int pid;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);

    // Get the number of processes
    int number_of_processes;
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_processes);

    if (pid == 0) // master
    {
        int fd = open(argv[1], O_RDONLY);
        if (fd < 0)
        {
            perror("Failed to open file");
            return EXIT_FAILURE;
        }

        struct stat sb;
        if (fstat(fd, &sb) == -1)
        {
            perror("fstat");
            close(fd);
            return EXIT_FAILURE;
        }

        long file_size = sb.st_size;
        char *file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (file_data == MAP_FAILED)
        {
            perror("mmap");
            close(fd);
            return EXIT_FAILURE;
        }

        // size check
        long *line_offsets = malloc(sizeof(long) * (MAX_LINES + 1));
        if (!line_offsets)
        {
            perror("malloc");
            munmap(file_data, file_size);
            close(fd);
            return EXIT_FAILURE;
        }

        long valid_size = 0;
        int line_count = 0;
        line_offsets[0] = 0;

        while (valid_size < file_size && line_count < MAX_LINES)
        {
            if (file_data[valid_size] == '\n')
            {
                line_offsets[++line_count] = valid_size + 1;
            }
            valid_size++;
        }

        if (line_count < MAX_LINES)
        {
            fprintf(stderr, "Warning: File only has %d lines. Proceeding with fewer lines.\n", line_count);
        }

        int actual_lines = line_count;
        int *results = calloc(actual_lines, sizeof(int));
        if (!results)
        {
            perror("calloc");
            free(line_offsets);
            munmap(file_data, file_size);
            close(fd);
            return EXIT_FAILURE;
        }

        int lines_per_thread = actual_lines / (MAX_THREADS - 1);
        int remainder = actual_lines % (MAX_THREADS - 1);
        int start_line = 0;

        

        for (int i = 1; i < MAX_THREADS; i++)
        {
            int lines_for_thread = lines_per_thread + (i <= remainder ? 1 : 0);

            long start_offset = line_offsets[start_line];
            long end_offset = line_offsets[start_line + lines_for_thread];
            long chunk_size = end_offset - start_offset;
            MPI_Send(&chunk_size, 1, MPI_LONG, i, 0, MPI_COMM_WORLD);
            MPI_Send(file_data + start_offset, chunk_size, MPI_CHAR, i, 0, MPI_COMM_WORLD);

            start_line += lines_for_thread;
        }

        int current_line = 0;

        for (int i = 1; i < MAX_THREADS; i++)
        {
            int worker_line_count = 0;

            
            MPI_Recv(&worker_line_count, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        
            int *worker_results = malloc(worker_line_count * sizeof(int));
            if (!worker_results)
            {
                perror("malloc failed");
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }

            
            MPI_Recv(worker_results, worker_line_count, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int j = 0; j < worker_line_count; j++)
            {
                results[current_line++] = worker_results[j];
            }

            free(worker_results);
        }
        for (int i = 0; i < actual_lines; i++) {
            if (results[i] != 0) {
                printf("%d: %d\n", i, results[i]);
            }
        }

        munmap(file_data, file_size);
        close(fd);
        free(results);
        free(line_offsets);
    }

    else // workers
    {
        long chunk_size;
        MPI_Recv(&chunk_size, 1, MPI_LONG, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        char *recv_buffer = malloc(chunk_size);
        if (!recv_buffer)
        {
            perror("malloc failed");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        MPI_Recv(recv_buffer, chunk_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int line_count = 0;
        int *results = find_max_ascii(recv_buffer, chunk_size, &line_count);
        MPI_Send(&line_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(results, line_count, MPI_INT, 0, 0, MPI_COMM_WORLD);

        free(results);
        free(recv_buffer);
    }

    MPI_Finalize();
    
    return EXIT_SUCCESS;
}
