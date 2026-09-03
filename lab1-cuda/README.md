# Лабораторная работа №1 на CUDA C

Те же пять заданий, что и в [lab1](../lab1/README.md), но на CUDA — для сравнения
двух подходов. Собирается только при установленном CUDA Toolkit; без него
`nvcc` не находится и каталог пропускается, OpenCL-часть строится как обычно.

## Соответствие OpenCL и CUDA

| OpenCL | CUDA |
|---|---|
| `clGetPlatformIDs`, `clGetDeviceIDs` | `cudaGetDeviceCount`, `cudaSetDevice` |
| `clCreateContext`, `clCreateCommandQueue` | создаются неявно при первом обращении |
| `clCreateProgramWithSource` + `clBuildProgram` | ядро компилируется `nvcc` вместе с хост-кодом |
| `clCreateBuffer` | `cudaMalloc` |
| `clEnqueueWriteBuffer` / `clEnqueueReadBuffer` | `cudaMemcpy` |
| `clSetKernelArg` | аргументы функции ядра |
| `clEnqueueNDRangeKernel` | `kernel<<<блоков, потоков>>>(...)` |
| `get_global_id(0)` | `blockIdx.x * blockDim.x + threadIdx.x` |
| `clFinish` | `cudaDeviceSynchronize` |
| `clGetDeviceInfo` | `cudaGetDeviceProperties` |

Обвязки заметно меньше: нет платформ, контекста, очереди, загрузки и компиляции
текста ядра во время выполнения — а значит, нет и `.cl` файлов, логов компилятора
и кэша бинарников.

## Чем варианты отличаются по сути

**Задание 3 переносится не буквально.** В OpenCL достаточно заменить
`CL_DEVICE_TYPE_GPU` на `CL_DEVICE_TYPE_CPU`, и то же ядро выполняется на
процессоре. В CUDA устройства типа CPU не существует — она работает только на
видеокартах NVIDIA. Аналог здесь такой: функция `convertChar` в
[helloworld.cuh](helloworld.cuh) помечена `__host__ __device__`, поэтому `nvcc`
собирает её и для GPU, и для процессора. Задание 2 вызывает её из ядра,
задание 3 — из обычного цикла на хосте.

**В задании 5 нет платформ.** CUDA не знает ICD и вендоров: есть список
видеокарт NVIDIA и версии драйвера с runtime. Поэтому программа выводит
`cudaDeviceProp` — число мультипроцессоров, compute capability, лимиты блоков
и сетки, память, разрядность шины.

**Границы массива проверяет ядро.** В OpenCL `global_work_size` задаётся точно
по длине данных. В CUDA число потоков кратно размеру блока, поэтому блоков
берётся с запасом (`cu::blocksFor`), а лишние потоки отсекает `if (i < n)`
внутри ядра.

## Файлы

| Файл | Задание |
|---|---|
| `task1_add_k.cu` | 1. `O = I + K`, `K` — аргумент ядра |
| `task2_helloworld_gpu.cu` | 2. Конвертирование строки на GPU |
| `task3_helloworld_cpu.cu` | 3. То же преобразование на процессоре |
| `task4_vector_mul.cu` | 4. `C[i] = A[i] * B[i]`, 16 значений `float` |
| `task5_device_info.cu` | 5. Параметры устройств CUDA |
| `cuda_common.cuh` | общие функции: проверка `cudaError_t`, выбор устройства, пауза |
| `helloworld.cuh` | ядро и функция `convertChar` для заданий 2 и 3 |

## Сборка и запуск

Нужен [CUDA Toolkit](https://developer.nvidia.com/cuda-downloads) (даёт `nvcc`)
и видеокарта NVIDIA. Собирается той же командой, что и OpenCL-часть:

```
cmake -S . -B build
cmake --build build --config Release
```

Программы окажутся в `build/bin-cuda` — отдельно от OpenCL-версий, потому что
имена совпадают. Архитектура GPU по умолчанию определяется по установленной
видеокарте (`CMAKE_CUDA_ARCHITECTURES=native`); задать вручную можно так:

```
cmake -S . -B build -DCMAKE_CUDA_ARCHITECTURES=86
```

Вывод совпадает по формату с OpenCL-версией, чтобы результаты можно было
сравнивать построчно.

## Если не работает

| Симптом | Причина и что делать |
|---|---|
| `CUDA: nvcc не найден` при конфигурировании | не установлен CUDA Toolkit; на Windows он ставится в `C:\Program Files\NVIDIA GPU Computing Toolkit` |
| `Устройства с поддержкой CUDA не найдены` | нет видеокарты NVIDIA или драйвер старше версии, которую требует Toolkit |
| `cudaErrorNoKernelImageForDevice` при запуске ядра | программа собрана под другую архитектуру: укажите `-DCMAKE_CUDA_ARCHITECTURES=<CC вашей карты>` |
| `nvcc fatal: Cannot find compiler 'cl.exe'` | на Windows `nvcc` нужен хост-компилятор MSVC: собирайте из «x64 Native Tools Command Prompt» либо генератором Visual Studio |
