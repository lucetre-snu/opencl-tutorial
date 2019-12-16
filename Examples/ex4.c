// OpenCL Host Program 4
// Matrix Multiplication

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#define CHECK_ERROR(err) \
    if(err != CL_SUCCESS) { \
        printf("[%s:%d] OpenCL error %d\n", __FILE__, __LINE__, err); \
        exit(EXIT_FAILURE); \
    }


char *get_source_code(const char *file_name, size_t *len) {
    char *source_code;
    size_t length;
    FILE *file = fopen(file_name, "r");

    if(file == NULL) {
        printf("[%s:%d] Failed to open %s\n", __FILE__, __LINE__, file_name);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    length = (size_t)ftell(file);
    rewind(file);

    source_code = (char *)malloc(length + 1);
    fread(source_code, length, 1, file);
    source_code[length] = '\0';
    
    fclose(file);

    *len = length;
    return source_code;
}

void mat_mul_opencl(float *A, float *B, float *C,
                    int ROW_A, int COL_A, int COL_B) {
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;

    char *kernel_source;
    size_t kernel_source_size;
    cl_kernel kernel;
    cl_int err;

    err = clGetPlatformIDs(1, &platform, NULL);
    CHECK_ERROR(err);

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    CHECK_ERROR(err);

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    CHECK_ERROR(err);

    queue = clCreateCommandQueue(context, device, 0, &err);
    CHECK_ERROR(err);

    kernel_source = get_source_code("Examples/kernel_mat_mul.cl", &kernel_source_size);
    program = clCreateProgramWithSource(context, 1, (const char**)&kernel_source, &kernel_source_size, &err);
    CHECK_ERROR(err);
    
    if(err == CL_BUILD_PROGRAM_FAILURE) {
        size_t log_size;
        char *log;

        err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        CHECK_ERROR(err);

        err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        CHECK_ERROR(err);

        log[log_size] = '\0';
        printf("Compiler error:\n%s\n", log);
        free(log);
        exit(0);
    }

    kernel = clCreateKernel(program, "mat_mul", &err);
    CHECK_ERROR(err);
    


}

int main() {
    const int ROW_A = 2;
    const int COL_A = 6;
    const int COL_B = 3;
    
    float *A = (float *)malloc(sizeof(float) * ROW_A * COL_A);
    float *B = (float *)malloc(sizeof(float) * COL_A * COL_B);
    float *C = (float *)malloc(sizeof(float) * ROW_A * COL_B);

    for(int i = 0; i < COL_A; i++) {
        for(int j = 0; j < ROW_A; j++) {
            A[i + j*COL_A] = (rand() % 10000) / 100.0;
        }
        for(int j = 0; j < COL_B; j++) {
            B[i*COL_B + j] = (rand() % 10000) / 100.0;
        }
    }

    for(int i = 0; i < ROW_A; i++) {
        for(int j = 0; j < COL_A; j++) {
            printf("%6.2f", A[i*COL_A + j]);
        }
        printf("\n");
    }

    for(int i = 0; i < COL_A; i++) {
        for(int j = 0; j < COL_B; j++) {
            printf("%6.2f", B[i*COL_B + j]);
        }
        printf("\n");
    }

    mat_mul_opencl(A, B, C, ROW_A, COL_A, COL_B);

    return 0;
}