// Задание 2: конвертирование строки на GPU, GdkknVnqkc -> HelloWorld.
// Запуск: task2_helloworld_gpu.exe [строка]
#include "cuda_common.cuh"
#include "helloworld.cuh"

using namespace std;

static const int THREADS_PER_BLOCK = 256;

int main(int argc, char* argv[])
{
    cu::initConsole();

    int device = cu::selectDevice();
    if (device < 0) {
        cerr << "Устройства с поддержкой CUDA не найдены." << endl;
        return 1;
    }
    cu::printShortDeviceInfo(device);

    const char* input = (argc > 1) ? argv[1] : "GdkknVnqkc";
    int n = (int)strlen(input);

    char* deviceInput = NULL;
    char* deviceOutput = NULL;
    CHECK(cudaMalloc(&deviceInput, n), "cudaMalloc (входной буфер)");
    CHECK(cudaMalloc(&deviceOutput, n), "cudaMalloc (выходной буфер)");

    CHECK(cudaMemcpy(deviceInput, input, n, cudaMemcpyHostToDevice),
          "cudaMemcpy (входной буфер)");

    helloworld<<<cu::blocksFor(n, THREADS_PER_BLOCK), THREADS_PER_BLOCK>>>(
        deviceInput, deviceOutput, n);
    CHECK(cudaGetLastError(), "запуск ядра helloworld");
    CHECK(cudaDeviceSynchronize(), "cudaDeviceSynchronize");

    string output(n, '\0');
    CHECK(cudaMemcpy(&output[0], deviceOutput, n, cudaMemcpyDeviceToHost),
          "cudaMemcpy (выходной буфер)");

    cudaFree(deviceInput);
    cudaFree(deviceOutput);

    cout << "Вход:  " << input  << endl;
    cout << "Выход: " << output << endl;

    cu::pause();
    return 0;
}
