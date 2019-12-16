__kernel void vec_add(__global float *A,
                      __global float *B,
                      __global float *C)
{
    int i = get_global_id(0);
//    printf("%d\n", i);
    C[i] = A[i] + B[i];
}
