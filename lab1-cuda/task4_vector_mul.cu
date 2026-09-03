// Задание 4: два буфера float по N значений, попарное произведение A[i]*B[i].
#include "cuda_common.cuh"

#include <math.h>
#include <iomanip>

using namespace std;

static const int N = 16;                  // по заданию не менее 10
static const int THREADS_PER_BLOCK = 256;

__global__ void vecMul(const float* A, const float* B, float* C, int n)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) C[i] = A[i] * B[i];
}

int main()
{
    cu::initConsole();

    float A[N], B[N], C[N];
    for (int i = 0; i < N; ++i) {
        A[i] = (float)(i + 1);
        B[i] = 0.5f * (i + 1);
    }

    int device = cu::selectDevice();
    if (device < 0) {
        cerr << "Устройства с поддержкой CUDA не найдены." << endl;
        return 1;
    }
    cu::printShortDeviceInfo(device);
    cout << "N = " << N << endl << endl;

    float* deviceA = NULL;
    float* deviceB = NULL;
    float* deviceC = NULL;
    CHECK(cudaMalloc(&deviceA, N * sizeof(float)), "cudaMalloc (буфер A)");
    CHECK(cudaMalloc(&deviceB, N * sizeof(float)), "cudaMalloc (буфер B)");
    CHECK(cudaMalloc(&deviceC, N * sizeof(float)), "cudaMalloc (буфер C)");

    CHECK(cudaMemcpy(deviceA, A, N * sizeof(float), cudaMemcpyHostToDevice),
          "cudaMemcpy (буфер A)");
    CHECK(cudaMemcpy(deviceB, B, N * sizeof(float), cudaMemcpyHostToDevice),
          "cudaMemcpy (буфер B)");

    vecMul<<<cu::blocksFor(N, THREADS_PER_BLOCK), THREADS_PER_BLOCK>>>(
        deviceA, deviceB, deviceC, N);
    CHECK(cudaGetLastError(), "запуск ядра vecMul");
    CHECK(cudaDeviceSynchronize(), "cudaDeviceSynchronize");

    CHECK(cudaMemcpy(C, deviceC, N * sizeof(float), cudaMemcpyDeviceToHost),
          "cudaMemcpy (буфер C)");

    cudaFree(deviceA);
    cudaFree(deviceB);
    cudaFree(deviceC);

    cout << fixed << setprecision(2);
    cout << " i      A[i]      B[i]  A[i]*B[i]" << endl;

    int errors = 0;
    for (int i = 0; i < N; ++i) {
        if (fabs(A[i] * B[i] - C[i]) > 1e-4f) ++errors;
        cout << setw(2) << i
             << setw(10) << A[i]
             << setw(10) << B[i]
             << setw(11) << C[i] << endl;
    }

    cout << endl << "Расхождений с расчётом на CPU: " << errors << endl;

    cu::pause();
    return 0;
}
