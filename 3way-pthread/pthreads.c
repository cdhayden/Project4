//Using POSIX file system
//make file not made yet

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

int MAX_THREADS = 1;  // to change with core count
int MAX_LINES = 1; // vary input size

// Structure to pass data to each thread
typedef struct {
    int thread_id;
    char *data;
    long start_offset;
    long end_offset;
    int line_start;
    int *results;
} ThreadData;

void *find_max_ascii(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    char *buffer = data->data + data->start_offset;
    long size = data->end_offset - data->start_offset;

    int line_num = data->line_start;
    long i = 0;

    int max_val = 0;

    while (i < size) {
        char c = buffer[i++];
        if (c == '\n' || i == size) {
            data->results[line_num++] = max_val;
            max_val = 0;
        } else {
            if ((unsigned char)c > max_val) {
                max_val = (unsigned char)c;
            }
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    sscanf(argv[2], "%d", &MAX_THREADS);
    sscanf(argv[3], "%d", &MAX_LINES);

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("fstat");
        close(fd);
        return EXIT_FAILURE;
    }

    long file_size = sb.st_size;
    char *file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    // size check
    long valid_size = 0;
    int line_count = 0;
    while (valid_size < file_size && line_count < MAX_LINES) {
        if (file_data[valid_size] == '\n') {
            line_count++;
        }
        valid_size++;
    }

    if (line_count < MAX_LINES) {
        fprintf(stderr, "Warning: File only has %d lines. Proceeding with fewer lines.\n", line_count);
        MAX_LINES = line_count;
    }

    int lines_per_thread = MAX_LINES / MAX_THREADS;
    int remainder = MAX_LINES % MAX_THREADS;
    
    int num_threads = MAX_THREADS;
    long chunk_size = file_size / num_threads;
    pthread_t threads[MAX_THREADS];
    ThreadData thread_data[MAX_THREADS];
    pthread_attr_t attr;
    int *results = calloc(MAX_LINES, sizeof(int)); // depending on input size


    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE); //Set Pthreads to joinable

    int current_line = 0;
    long start = 0;

    for (int i = 0; i < num_threads; i++) {
        int lines_for_thread = lines_per_thread + (i < remainder ? 1 : 0);
        
        start = i * chunk_size;
        long end = (i == num_threads - 1) ? file_size : (i + 1) * chunk_size;

        long local_start = start;
        long local_end = local_start;
        int lines = 0;

        // Align to next newline to prevent splitting a line :: OLD
        /*
        if (i != 0) {
            while (start < file_size && file_data[start] != '\n') start++;
            start++;
        }
        if (i != num_threads - 1) {
            while (end < file_size && file_data[end] != '\n') end++;
            end++;
        }
        */
        // NEW
        while (local_end < valid_size && lines < lines_for_thread) {
            if (file_data[local_end] == '\n') {
                lines++;
            }
            local_end++;
        }

        thread_data[i].thread_id = i;
        thread_data[i].data = file_data;
        thread_data[i].start_offset = local_start;
        thread_data[i].end_offset = local_end;
        thread_data[i].line_start = current_line;
        thread_data[i].results = results;

        pthread_create(&threads[i], &attr, find_max_ascii, &thread_data[i]);

        start = local_end;
        current_line += lines_for_thread;
    }

    pthread_attr_destroy(&attr); //Free the atribute of the threads.

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < MAX_LINES; i++) {
        if (results[i] != 0) {
            printf("%d: %d\n", i, results[i]);
        }
    }

    munmap(file_data, file_size);
    close(fd);
    free(results);

    return EXIT_SUCCESS;
}
