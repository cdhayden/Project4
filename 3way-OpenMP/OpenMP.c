#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <omp.h>

int MAX_THREADS = 1;
int MAX_LINES = 1;

void find_max_ascii(char *data, long start_offset, long end_offset, int line_start, int *results)
{
    char *buffer = data + start_offset;
    long size = end_offset - start_offset;

    int line_num = line_start;
    long i = 0;
    int max_val = 0;

    while (i < size)
    {
        char c = buffer[i++];
        if (c == '\n' || i == size)
        {
            results[line_num++] = max_val;
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
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <file_path> <threads> <max_lines>\n", argv[0]);
        return EXIT_FAILURE;
    }

    
    sscanf(argv[2], "%d", &MAX_THREADS);
    sscanf(argv[3], "%d", &MAX_LINES);

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

    int lines_per_thread = actual_lines / MAX_THREADS;
    int remainder = actual_lines % MAX_THREADS;

    // Precompute line ranges for each thread
    int thread_starts[MAX_THREADS + 1];
    thread_starts[0] = 0;
    for (int i = 0; i < MAX_THREADS; i++)
    {
        int extra = (i < remainder) ? 1 : 0;
        thread_starts[i + 1] = thread_starts[i] + lines_per_thread + extra;
    }

    omp_set_num_threads(MAX_THREADS);

    

#pragma omp parallel for
    for (int i = 0; i < MAX_THREADS; i++)
    {
        int start_line = thread_starts[i];
    int end_line = thread_starts[i + 1];
    long start_offset = line_offsets[start_line];
    long end_offset = line_offsets[end_line];

        find_max_ascii(file_data, start_offset, end_offset, start_line, results);
        
    }

    for (int i = 0; i < actual_lines; i++)
    {
        if (results[i] != 0)
        {
            printf("%d: %d\n", i, results[i]);
        }
    }

    munmap(file_data, file_size);
    close(fd);
    free(results);
    free(line_offsets);

    return EXIT_SUCCESS;
}
