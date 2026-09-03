// Задание 3: то же преобразование на процессоре.
//
// В OpenCL для этого достаточно заменить CL_DEVICE_TYPE_GPU на
// CL_DEVICE_TYPE_CPU: платформа сама даёт устройство-процессор. В CUDA такого
// устройства нет — она работает только на видеокартах NVIDIA. Прямой аналог
// здесь — вызвать на хосте ту же функцию convertChar, что и в ядре задания 2:
// nvcc собирает её и для GPU, и для CPU (__host__ __device__).
//
// Запуск: task3_helloworld_cpu.exe [строка]
#include "cuda_common.cuh"
#include "helloworld.cuh"

using namespace std;

int main(int argc, char* argv[])
{
    cu::initConsole();

    const char* input = (argc > 1) ? argv[1] : "GdkknVnqkc";
    int n = (int)strlen(input);

    string output(n, '\0');
    for (int i = 0; i < n; ++i)
        output[i] = convertChar(input[i]);   // та же функция, что и в ядре

    cout << "Устройство: центральный процессор (хост)" << endl;
    cout << "Вход:  " << input  << endl;
    cout << "Выход: " << output << endl << endl;

    int count = 0;
    cudaGetDeviceCount(&count);
    cout << "Устройств CUDA в системе: " << count
         << ", но устройства типа CPU среди них быть не может:" << endl
         << "CUDA выполняется только на видеокартах NVIDIA. На процессоре"  << endl
         << "работает та же функция convertChar, помеченная __host__ __device__." << endl;

    cu::pause();
    return 0;
}
