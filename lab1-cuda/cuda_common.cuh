// Общие функции для программ лабораторной работы №1 на CUDA.
#ifndef CUDA_COMMON_CUH
#define CUDA_COMMON_CUH

#include <cuda_runtime.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace cu {

// Повторные вызовы игнорируются: пауза выводится ровно один раз.
inline void pause()
{
    static bool alreadyPaused = false;
    if (alreadyPaused) return;
    alreadyPaused = true;

    std::cout << std::endl << "Нажмите Enter" << std::endl;
    std::cin.clear();
    std::cin.get();
}

inline void initConsole()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    atexit(pause);   // окно не закроется и при аварийном выходе из check()
}

inline void check(cudaError_t status, const char* what)
{
    if (status != cudaSuccess) {
        std::cerr << "Ошибка: " << what << " -> "
                  << cudaGetErrorName(status) << " ("
                  << cudaGetErrorString(status) << ")" << std::endl;
        exit(1);
    }
}

// Первое устройство с поддержкой CUDA, -1 если таких нет.
inline int selectDevice()
{
    int count = 0;
    if (cudaGetDeviceCount(&count) != cudaSuccess || count == 0) return -1;

    check(cudaSetDevice(0), "cudaSetDevice");
    return 0;
}

inline void printShortDeviceInfo(int device)
{
    cudaDeviceProp prop;
    check(cudaGetDeviceProperties(&prop, device), "cudaGetDeviceProperties");

    std::cout << "Устройство: " << prop.name
              << " (GPU, " << prop.multiProcessorCount << " SM, CC "
              << prop.major << "." << prop.minor << ")" << std::endl;
}

// В CUDA число потоков кратно размеру блока, поэтому блоков берём с запасом,
// а лишние потоки отсекаются проверкой внутри ядра.
inline int blocksFor(int items, int threadsPerBlock)
{
    return (items + threadsPerBlock - 1) / threadsPerBlock;
}

} // namespace cu

#define CHECK(status, what) cu::check((status), (what))

#endif // CUDA_COMMON_CUH
