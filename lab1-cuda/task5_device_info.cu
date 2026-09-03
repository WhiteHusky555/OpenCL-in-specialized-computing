// Задание 5: сведения об устройствах CUDA.
//
// В OpenCL перечисляются платформы, а внутри них устройства. В CUDA понятия
// платформы нет: есть один вендор и список его видеокарт, поэтому здесь
// выводятся только версии драйвера с runtime и параметры каждого устройства.
#include "cuda_common.cuh"

using namespace std;

static string mbytes(size_t bytes)
{
    return to_string((unsigned long long)(bytes / (1024ULL * 1024ULL))) + " MB";
}

static string kbytes(size_t bytes)
{
    return to_string((unsigned long long)(bytes / 1024ULL)) + " KB";
}

// В CUDA 13 частоты убрали из cudaDeviceProp, они запрашиваются отдельно.
static string clockMHz(cudaDeviceAttr attr, int device)
{
    int kHz = 0;
    if (cudaDeviceGetAttribute(&kHz, attr, device) != cudaSuccess || kHz <= 0)
        return "нет данных";
    return to_string(kHz / 1000) + " MHz";
}

static string version(int v)
{
    return to_string(v / 1000) + "." + to_string((v % 100) / 10);
}

int main()
{
    cu::initConsole();

    int count = 0;
    cudaError_t status = cudaGetDeviceCount(&count);
    if (status != cudaSuccess) {
        cerr << "Ошибка: cudaGetDeviceCount -> " << cudaGetErrorName(status)
             << " (" << cudaGetErrorString(status) << ")" << endl;
        return 1;
    }
    if (count == 0) {
        cout << "Устройства с поддержкой CUDA не найдены." << endl;
        return 1;
    }

    int driverVersion = 0;
    int runtimeVersion = 0;
    cudaDriverGetVersion(&driverVersion);
    cudaRuntimeGetVersion(&runtimeVersion);

    cout << "Версия драйвера CUDA : " << version(driverVersion) << endl;
    cout << "Версия runtime CUDA  : " << version(runtimeVersion) << endl;
    cout << "Устройств            : " << count << endl;

    for (int d = 0; d < count; ++d)
    {
        cudaDeviceProp p;
        CHECK(cudaGetDeviceProperties(&p, d), "cudaGetDeviceProperties");

        cout << endl << "Устройство " << d << endl;
        cout << "    name                        : " << p.name << endl;
        cout << "    compute capability          : " << p.major << "." << p.minor << endl;
        cout << "    multiProcessorCount         : " << p.multiProcessorCount
             << "   (потоковых мультипроцессоров)" << endl;
        cout << "    cudaDevAttrClockRate        : "
             << clockMHz(cudaDevAttrClockRate, d) << endl;
        cout << "    warpSize                    : " << p.warpSize << endl;
        cout << "    maxThreadsPerBlock          : " << p.maxThreadsPerBlock << endl;
        cout << "    maxThreadsPerMultiProcessor : " << p.maxThreadsPerMultiProcessor << endl;
        cout << "    maxThreadsDim               : "
             << p.maxThreadsDim[0] << " x " << p.maxThreadsDim[1] << " x "
             << p.maxThreadsDim[2] << endl;
        cout << "    maxGridSize                 : "
             << p.maxGridSize[0] << " x " << p.maxGridSize[1] << " x "
             << p.maxGridSize[2] << endl;
        cout << "    totalGlobalMem              : " << mbytes(p.totalGlobalMem) << endl;
        cout << "    sharedMemPerBlock           : " << kbytes(p.sharedMemPerBlock) << endl;
        cout << "    totalConstMem               : " << kbytes(p.totalConstMem) << endl;
        cout << "    regsPerBlock                : " << p.regsPerBlock << endl;
        cout << "    l2CacheSize                 : " << kbytes(p.l2CacheSize) << endl;
        cout << "    cudaDevAttrMemoryClockRate  : "
             << clockMHz(cudaDevAttrMemoryClockRate, d) << endl;
        cout << "    memoryBusWidth              : " << p.memoryBusWidth << " бит" << endl;
        cout << "    unifiedAddressing           : " << (p.unifiedAddressing ? "да" : "нет") << endl;
        cout << "    concurrentKernels           : " << (p.concurrentKernels ? "да" : "нет") << endl;
        cout << "    integrated                  : "
             << (p.integrated ? "да (встроенная)" : "нет (дискретная)") << endl;
        cout << "    ECCEnabled                  : " << (p.ECCEnabled ? "да" : "нет") << endl;
    }

    cu::pause();
    return 0;
}
