// OpenCL Host Program 4
// Matrix Multiplication

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

    err = clBuildProgram(program, 1, &device, "", NULL, NULL);
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
    CHECK_ERROR(err);

    kernel = clCreateKernel(program, "mat_mul", &err);
    CHECK_ERROR(err);
    
    cl_mem bufA, bufB, bufC;
    bufA = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*ROW_A*COL_A, NULL, &err); CHECK_ERROR(err);
    bufB = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*COL_A*COL_B, NULL, &err); CHECK_ERROR(err);
    bufC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float)*ROW_A*COL_B, NULL, &err); CHECK_ERROR(err);

    err = clEnqueueWriteBuffer(queue, bufA, CL_FALSE, 0, sizeof(float)*ROW_A*COL_A, A, 0, NULL, NULL); CHECK_ERROR(err);
    err = clEnqueueWriteBuffer(queue, bufB, CL_FALSE, 0, sizeof(float)*COL_A*COL_B, B, 0, NULL, NULL); CHECK_ERROR(err);

    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA); CHECK_ERROR(err);
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufB); CHECK_ERROR(err);
    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufC); CHECK_ERROR(err);
    err = clSetKernelArg(kernel, 3, sizeof(cl_int), &ROW_A); CHECK_ERROR(err);
    err = clSetKernelArg(kernel, 4, sizeof(cl_int), &COL_A); CHECK_ERROR(err);
    err = clSetKernelArg(kernel, 5, sizeof(cl_int), &COL_B); CHECK_ERROR(err);

    size_t global_size[2] = {COL_B, ROW_A};
    size_t local_size[2] = {16, 16};
    global_size[0] = (global_size[0] + local_size[0] - 1) / local_size[0] * local_size[0];
    global_size[1] = (global_size[1] + local_size[1] - 1) / local_size[1] * local_size[1];

    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, local_size, 0, NULL, NULL);
    CHECK_ERROR(err);

    err = clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, sizeof(float)*ROW_A*COL_B, C, 0, NULL, NULL);
    CHECK_ERROR(err);
    
    clReleaseMemObject(bufA);
    clReleaseMemObject(bufB);
    clReleaseMemObject(bufC);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
}


void mat_mul_seq(float *A, float *B, float *C,
                    int ROW_A, int COL_A, int COL_B) {
    for(int i = 0; i < ROW_A; i++) {
        for(int j = 0; j < COL_B; j++) {
            C[i*COL_B + j] = 0.0f;
            for(int k = 0; k < COL_A; k++) {
                C[i*COL_B + j] += A[i*COL_A + k] * B[k*COL_B + j];
            }
        }
    }
}

int main() {
    const int ROW_A = 512;
    const int COL_A = 16384;
    const int COL_B = 128;

    float *A = (float *)malloc(sizeof(float) * ROW_A * COL_A);
    float *B = (float *)malloc(sizeof(float) * COL_A * COL_B);
    float *C_seq = (float *)malloc(sizeof(float) * ROW_A * COL_B);
    float *C_opencl = (float *)malloc(sizeof(float) * ROW_A * COL_B);

    for(int i = 0; i < COL_A; i++) {
        for(int j = 0; j < ROW_A; j++) {
            A[i + j*COL_A] = (rand() % 10) / 10.0;
        }
        for(int j = 0; j < COL_B; j++) {
            B[i*COL_B + j] = (rand() % 10) / 10.0;
        }
    }

    clock_t start, end;
    double cpu_time_used;

    start = clock();
    mat_mul_opencl(A, B, C_opencl, ROW_A, COL_A, COL_B);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf(">> mat_mul_opencl time: %lf\n", cpu_time_used);
    
    start = clock();
    mat_mul_seq(A, B, C_seq, ROW_A, COL_A, COL_B);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf(">> mat_mul_seq time: %lf\n", cpu_time_used);

    for(int i = 0; i < ROW_A; i++) {
        for(int j = 0; j < COL_B; j++) {
            if(C_seq[i*COL_B + j] != C_opencl[i*COL_B + j]) {
                printf("Verification failed! C_seq[%d][%d] = %.6f, C_opencl[%d][%d] = %.6f\n", i, j, C_seq[i*COL_B + j], i, j, C_opencl[i*COL_B + j]);
                break;
            }
        }
    }

    return 0;
}