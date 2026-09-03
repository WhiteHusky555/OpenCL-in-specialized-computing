// Преобразование одного символа: HelloWorld получается прибавлением 1.
//
// __host__ __device__ означает, что nvcc собирает функцию дважды: для GPU и для
// процессора. Задание 2 вызывает её из ядра, задание 3 — из обычного цикла,
// то есть на обоих устройствах работает буквально один и тот же код.
#ifndef HELLOWORLD_CUH
#define HELLOWORLD_CUH

__host__ __device__ inline char convertChar(char c)
{
    return c + 1;
}

__global__ void helloworld(const char* in, char* out, int n)
{
    int num = blockIdx.x * blockDim.x + threadIdx.x;
    if (num < n) out[num] = convertChar(in[num]);
}

#endif // HELLOWORLD_CUH
