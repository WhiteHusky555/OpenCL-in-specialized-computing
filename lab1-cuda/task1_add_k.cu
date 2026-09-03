// Задание 1: обработка буфера по формуле O = I + K (char).
// Запуск: task1_add_k.exe [строка] [K]      по умолчанию "GdkknVnqkc" и K = 1
#include "cuda_common.cuh"

using namespace std;

static const int THREADS_PER_BLOCK = 256;

__global__ void addK(const char* in, char* out, char K, int n)
{
    int num = blockIdx.x * blockDim.x + threadIdx.x;
    if (num < n) out[num] = in[num] + K;
}

int main(int argc, char* argv[])
{
    cu::initConsole();

    const char* input = (argc > 1) ? argv[1] : "GdkknVnqkc";
    char K = (argc > 2) ? (char)atoi(argv[2]) : (char)1;

    int device = cu::selectDevice();
    if (device < 0) {
        cerr << "Устройства с поддержкой CUDA не найдены." << endl;
        return 1;
    }
    cu::printShortDeviceInfo(device);

    int n = (int)strlen(input);

    char* deviceInput = NULL;
    char* deviceOutput = NULL;
    CHECK(cudaMalloc(&deviceInput, n), "cudaMalloc (входной буфер)");
    CHECK(cudaMalloc(&deviceOutput, n), "cudaMalloc (выходной буфер)");

    CHECK(cudaMemcpy(deviceInput, input, n, cudaMemcpyHostToDevice),
          "cudaMemcpy (входной буфер)");

    addK<<<cu::blocksFor(n, THREADS_PER_BLOCK), THREADS_PER_BLOCK>>>(
        deviceInput, deviceOutput, K, n);
    CHECK(cudaGetLastError(), "запуск ядра addK");
    CHECK(cudaDeviceSynchronize(), "cudaDeviceSynchronize");

    string output(n, '\0');
    CHECK(cudaMemcpy(&output[0], deviceOutput, n, cudaMemcpyDeviceToHost),
          "cudaMemcpy (выходной буфер)");

    cudaFree(deviceInput);
    cudaFree(deviceOutput);

    cout << "K = " << (int)K << endl;
    cout << "I: " << input  << endl;
    cout << "O: " << output << endl << endl;

    for (int i = 0; i < n; ++i) {
        cout << "  " << input[i] << " " << (int)(unsigned char)input[i]
             << " -> " << output[i] << " " << (int)(unsigned char)output[i] << endl;
    }

    cu::pause();
    return 0;
}
