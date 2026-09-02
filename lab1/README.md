# Лабораторная работа №1. Знакомство с OpenCL

Пять консольных программ на C++ и OpenCL 1.2 — по одной на каждое задание
методических указаний. Требования и общая сборка описаны в
[корневом README](../README.md).

## Задания и файлы

| Программа | Задание | Ядро |
|---|---|---|
| `task1_add_k.cpp` | 1. `O = I + K`, где `K` — аргумент ядра типа `char` | `add_k.cl`, `addK` |
| `task2_helloworld_gpu.cpp` | 2. Конвертирование строки на GPU по пунктам методички | `helloworld.cl`, `helloworld` |
| `task3_helloworld_cpu.cpp` | 3. То же на CPU | `helloworld.cl`, `helloworld` |
| `task4_vector_mul.cpp` | 4. `C[i] = A[i] * B[i]`, 16 значений `float` | `vector_mul.cl`, `vecMul` |
| `task5_device_info.cpp` | 5. Параметры всех платформ и устройств | — |
| `ocl_common.h` | общие функции | — |

Задания 2 и 3 отличаются одной константой `SELECTED_DEVICE_TYPE`
(`CL_DEVICE_TYPE_GPU` / `CL_DEVICE_TYPE_CPU`) и используют одно ядро.

## Запуск

```
cd build/bin
./task1_add_k                 # аргументы: строка и K
./task1_add_k "HelloWorld" -1
./task2_helloworld_gpu
./task3_helloworld_cpu
./task4_vector_mul
./task5_device_info
```

Ожидаемый вывод задания 2:

```
Устройство: NVIDIA GeForce RTX 3070 Ti Laptop GPU (GPU, 46 CU, OpenCL C 1.2)
Вход:  GdkknVnqkc
Выход: HelloWorld
```

Задание 4 в конце печатает число расхождений с контрольным расчётом на CPU,
в норме — 0.

Программы ждут Enter перед выходом (`ocl::pause`), чтобы окно консоли не
закрывалось. При запуске из скриптов перенаправляйте stdin:
`./task5_device_info < /dev/null`.

## Устройство кода

* Каждая `taskN_*.cpp` самодостаточна: выбор устройства, контекст, очередь команд,
  буферы, запуск ядра, чтение результата, освобождение ресурсов.
* `ocl_common.h` — общее: `ocl::selectDevice`, `ocl::buildProgram` (печатает лог
  компилятора OpenCL при ошибке), `ocl::loadKernelSource`, `ocl::check` и макрос
  `CHECK`, вывод параметров устройства, пауза перед выходом.
* Тексты ядер лежат в `.cl` и по умолчанию читаются во время выполнения: сначала
  в текущем каталоге, затем на уровень выше. Если файла нет, `ocl::loadKernelSource`
  бросает `std::runtime_error`, `main` его ловит и печатает, какой файл не открылся.

## Откуда программа берёт ядро

Три режима, выбираются опциями CMake:

| Сборка | Что происходит |
|---|---|
| по умолчанию | `.cl` читается при каждом запуске, `.cl` нужен рядом с exe |
| `-DLAB_EMBED_KERNELS=ON` | [cmake/embed_kernel.cmake](../cmake/embed_kernel.cmake) генерирует из `.cl` заголовок `<ядро>_cl.h` с текстом ядра, программа собирается с ним и запускается без `.cl` |
| `-DLAB_KERNEL_BINARY_CACHE=ON` | при первом запуске ядро компилируется и его бинарник сохраняется в `kernel_<хэш>.bin`, дальше грузится оттуда через `clCreateProgramWithBinary` |

Режимы независимы и сочетаются. Имя файла кэша — хэш от текста ядра и от
устройства с версией драйвера, поэтому после правки `.cl` или смены устройства
берётся новый файл, а старый просто перестаёт использоваться:

```
Ядро скомпилировано и сохранено в kernel_ed94587622f0d0b8.bin   # первый запуск
Ядро загружено из kernel_ed94587622f0d0b8.bin                   # последующие
```

Вопреки методичке, программу из бинарника всё равно нужно провести через
`clBuildProgram` — этого требует спецификация, но компиляция при этом уже
не выполняется.
* Число рабочих элементов задаётся как `global_work_size = длина данных`, размер
  рабочей группы отдан на откуп реализации (`NULL`).

При правке кода стоит помнить:

* имя `DEVICE_TYPE` занято макросом из `winscard.h`, который подтягивает
  `windows.h`, поэтому константа называется `SELECTED_DEVICE_TYPE`;
* `ocl::pause` регистрируется через `atexit`, поэтому окно остаётся открытым и
  при аварийном выходе из `ocl::check`.

## Если не работает

| Симптом | Причина и что делать |
|---|---|
| `Платформы OpenCL не найдены` | не установлен ни один ICD. Поставьте драйвер видеокарты или программный OpenCL (`pocl-opencl-icd` в Linux) |
| `CPU-устройства OpenCL не найдены` | драйверы GPU не дают устройство типа CPU. Нужен [Intel CPU Runtime for OpenCL](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-cpu-runtime-for-opencl-applications-with-sycl-support.html); тихая установка: `w_opencl_runtime_p_*.exe -s -a /qn /norestart` (внутри MSI, ключи oneAPI `--silent --eula accept` не подходят) |
| `не удалось открыть файл с текстом ядра` | рядом с exe нет нужного `.cl`. При сборке через CMake файлы копируются в `build/bin` автоматически |
| `Ошибка компиляции ядра` | ниже выводится лог компилятора OpenCL с номером строки в `.cl` |
| Кракозябры вместо русского текста в консоли Windows | нужен терминал с UTF-8; программы сами вызывают `SetConsoleOutputCP(CP_UTF8)` |

## Сборка в Visual Studio по методичке

Вариант без CMake:

1. File → New Project → Visual C++ → Console Application.
2. C/C++ → Additional Include Directories: каталог с `CL\cl.h`
   (например `$(AMDAPPSDKROOT)/include`).
3. Linker → Additional Library Directories: `...\lib\x86_64` (или `\lib\x86`).
4. Linker → Input → Additional Dependencies: `OpenCL.lib`.
5. Добавить в проект нужный `taskN_*.cpp`, `ocl_common.h` и его `.cl` файл;
   `.cl` должен лежать в рабочем каталоге проекта либо рядом с exe.
