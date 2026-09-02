// Общие функции для программ лабораторной работы №1 (OpenCL).
#ifndef OCL_COMMON_H
#define OCL_COMMON_H

#include <CL/cl.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <iterator>

#ifdef _WIN32
#include <windows.h>
#endif

namespace ocl {

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

inline const char* errorName(cl_int e)
{
    switch (e) {
    case CL_SUCCESS:                       return "CL_SUCCESS";
    case CL_DEVICE_NOT_FOUND:              return "CL_DEVICE_NOT_FOUND";
    case CL_DEVICE_NOT_AVAILABLE:          return "CL_DEVICE_NOT_AVAILABLE";
    case CL_COMPILER_NOT_AVAILABLE:        return "CL_COMPILER_NOT_AVAILABLE";
    case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case CL_OUT_OF_RESOURCES:              return "CL_OUT_OF_RESOURCES";
    case CL_OUT_OF_HOST_MEMORY:            return "CL_OUT_OF_HOST_MEMORY";
    case CL_BUILD_PROGRAM_FAILURE:         return "CL_BUILD_PROGRAM_FAILURE";
    case CL_INVALID_VALUE:                 return "CL_INVALID_VALUE";
    case CL_INVALID_PLATFORM:              return "CL_INVALID_PLATFORM";
    case CL_INVALID_DEVICE:                return "CL_INVALID_DEVICE";
    case CL_INVALID_DEVICE_TYPE:           return "CL_INVALID_DEVICE_TYPE";
    case CL_INVALID_CONTEXT:               return "CL_INVALID_CONTEXT";
    case CL_INVALID_MEM_OBJECT:            return "CL_INVALID_MEM_OBJECT";
    case CL_INVALID_PROGRAM:               return "CL_INVALID_PROGRAM";
    case CL_INVALID_KERNEL_NAME:           return "CL_INVALID_KERNEL_NAME";
    case CL_INVALID_KERNEL_ARGS:           return "CL_INVALID_KERNEL_ARGS";
    case CL_INVALID_WORK_GROUP_SIZE:       return "CL_INVALID_WORK_GROUP_SIZE";
    default:                               return "CL_ERROR";
    }
}

inline void check(cl_int status, const char* what)
{
    if (status != CL_SUCCESS) {
        std::cerr << "Ошибка: " << what << " -> "
                  << errorName(status) << " (" << status << ")" << std::endl;
        exit(1);
    }
}

// Чтение текста ядра из файла (функция из методических указаний).
inline int convertToString(const char* filename, std::string& s)
{
    size_t size;
    char* str;
    std::fstream f(filename, (std::fstream::in | std::fstream::binary));

    if (f.is_open())
    {
        size_t fileSize;
        f.seekg(0, std::fstream::end);
        size = fileSize = (size_t)f.tellg();
        f.seekg(0, std::fstream::beg);
        str = new char[size + 1];
        if (!str) { f.close(); return 0; }

        f.read(str, fileSize);
        f.close();
        str[size] = '\0';
        s = str;
        delete[] str;
        return 0;
    }
    return -1;
}

// Ищет .cl в текущем каталоге и на уровень выше.
// Если файла нет ни там, ни там, бросает std::runtime_error.
inline std::string loadKernelSource(const char* filename)
{
    std::string src;
    if (convertToString(filename, src) == 0) return src;

    std::string up = std::string("../") + filename;
    if (convertToString(up.c_str(), src) == 0) return src;

    throw std::runtime_error(std::string("не удалось открыть файл с текстом ядра ") + filename);
}

inline std::string platformInfoStr(cl_platform_id p, cl_platform_info param)
{
    size_t size = 0;
    if (clGetPlatformInfo(p, param, 0, NULL, &size) != CL_SUCCESS || size == 0)
        return "<нет данных>";
    std::vector<char> buf(size);
    clGetPlatformInfo(p, param, size, buf.data(), NULL);
    return std::string(buf.data());
}

inline std::string deviceInfoStr(cl_device_id d, cl_device_info param)
{
    size_t size = 0;
    if (clGetDeviceInfo(d, param, 0, NULL, &size) != CL_SUCCESS || size == 0)
        return "<нет данных>";
    std::vector<char> buf(size);
    clGetDeviceInfo(d, param, size, buf.data(), NULL);
    return std::string(buf.data());
}

template <typename T>
inline T deviceInfoVal(cl_device_id d, cl_device_info param)
{
    T value = T();
    clGetDeviceInfo(d, param, sizeof(T), &value, NULL);
    return value;
}

inline std::string deviceTypeToString(cl_device_type t)
{
    std::string s;
    if (t & CL_DEVICE_TYPE_CPU)         s += "CPU ";
    if (t & CL_DEVICE_TYPE_GPU)         s += "GPU ";
    if (t & CL_DEVICE_TYPE_ACCELERATOR) s += "ACCELERATOR ";
    if (t & CL_DEVICE_TYPE_CUSTOM)      s += "CUSTOM ";
    if (t & CL_DEVICE_TYPE_DEFAULT)     s += "(DEFAULT) ";
    return s.empty() ? std::string("UNKNOWN") : s;
}

inline std::string trimRight(std::string s)
{
    while (!s.empty() && (s[s.size() - 1] == ' ' || s[s.size() - 1] == '\t'))
        s.erase(s.size() - 1);
    return s;
}

inline void printShortDeviceInfo(cl_device_id device)
{
    std::cout << "Устройство: " << deviceInfoStr(device, CL_DEVICE_NAME)
              << " (" << trimRight(deviceTypeToString(deviceInfoVal<cl_device_type>(device, CL_DEVICE_TYPE)))
              << ", " << deviceInfoVal<cl_uint>(device, CL_DEVICE_MAX_COMPUTE_UNITS) << " CU, "
              << trimRight(deviceInfoStr(device, CL_DEVICE_OPENCL_C_VERSION)) << ")" << std::endl;
}

// Первое устройство нужного типа среди всех платформ, NULL если такого нет.
inline cl_device_id selectDevice(cl_device_type type, cl_platform_id* platformOut = NULL)
{
    cl_uint numPlatforms = 0;
    cl_int status = clGetPlatformIDs(0, NULL, &numPlatforms);
    check(status, "clGetPlatformIDs (подсчёт платформ)");

    if (numPlatforms == 0) {
        std::cerr << "Платформы OpenCL не найдены." << std::endl;
        return NULL;
    }

    cl_platform_id* platforms =
        (cl_platform_id*)malloc(numPlatforms * sizeof(cl_platform_id));
    status = clGetPlatformIDs(numPlatforms, platforms, NULL);
    check(status, "clGetPlatformIDs (получение платформ)");

    cl_device_id device = NULL;
    for (cl_uint i = 0; i < numPlatforms && device == NULL; ++i)
    {
        cl_uint numDevices = 0;
        if (clGetDeviceIDs(platforms[i], type, 0, NULL, &numDevices) != CL_SUCCESS ||
            numDevices == 0)
            continue;

        cl_device_id* devices =
            (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
        status = clGetDeviceIDs(platforms[i], type, numDevices, devices, NULL);
        check(status, "clGetDeviceIDs (получение устройств)");

        device = devices[0];
        if (platformOut != NULL) *platformOut = platforms[i];
        free(devices);
    }

    free(platforms);
    return device;
}

// Компиляция программы из исходного текста; при ошибке печатает лог
// компилятора OpenCL и завершает программу.
inline cl_program compileProgram(cl_context context, cl_device_id device,
                                 const std::string& sourceStr)
{
    cl_int status;
    const char* source = sourceStr.c_str();
    size_t sourceSize[] = { strlen(source) };

    cl_program program = clCreateProgramWithSource(context, 1, &source, sourceSize, &status);
    check(status, "clCreateProgramWithSource");

    status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (status != CL_SUCCESS) {
        size_t logSize = 0;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        std::string log(logSize, '\0');
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, &log[0], NULL);
        std::cerr << "Ошибка компиляции ядра (" << errorName(status) << "):"
                  << std::endl << log << std::endl;
        exit(1);
    }
    return program;
}

#ifdef LAB_KERNEL_BINARY_CACHE

// Имя файла кэша: зависит и от текста ядра, и от устройства с драйвером,
// поэтому при их изменении берётся новый файл, а старый просто не используется.
inline std::string binaryCacheName(cl_device_id device, const std::string& sourceStr)
{
    std::string key = sourceStr
                    + deviceInfoStr(device, CL_DEVICE_NAME)
                    + deviceInfoStr(device, CL_DEVICE_VERSION)
                    + deviceInfoStr(device, CL_DRIVER_VERSION);

    unsigned long long hash = 1469598103934665603ULL;   // FNV-1a
    for (size_t i = 0; i < key.size(); ++i) {
        hash ^= (unsigned char)key[i];
        hash *= 1099511628211ULL;
    }

    char name[32];
    sprintf(name, "kernel_%016llx.bin", hash);
    return std::string(name);
}

// Загрузка заранее скомпилированного ядра. Возвращает NULL, если файла нет
// или устройство его не приняло, — тогда ядро компилируется из исходника.
inline cl_program loadProgramBinary(cl_context context, cl_device_id device,
                                    const std::string& filename)
{
    std::ifstream f(filename.c_str(), std::ios::in | std::ios::binary);
    if (!f.is_open()) return NULL;

    std::vector<unsigned char> binary((std::istreambuf_iterator<char>(f)),
                                       std::istreambuf_iterator<char>());
    f.close();
    if (binary.empty()) return NULL;

    size_t size = binary.size();
    const unsigned char* data = binary.data();
    cl_int binaryStatus = CL_SUCCESS;
    cl_int status = CL_SUCCESS;

    cl_program program = clCreateProgramWithBinary(context, 1, &device, &size,
                                                   &data, &binaryStatus, &status);
    if (status != CL_SUCCESS || binaryStatus != CL_SUCCESS) {
        if (program) clReleaseProgram(program);
        return NULL;
    }

    // По спецификации программу из бинарника всё равно нужно перевести
    // в исполняемое состояние, но компиляция при этом уже не выполняется.
    if (clBuildProgram(program, 1, &device, NULL, NULL, NULL) != CL_SUCCESS) {
        clReleaseProgram(program);
        return NULL;
    }
    return program;
}

inline void saveProgramBinary(cl_program program, const std::string& filename)
{
    size_t size = 0;
    if (clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES,
                         sizeof(size_t), &size, NULL) != CL_SUCCESS || size == 0)
        return;

    std::vector<unsigned char> binary(size);
    unsigned char* data = binary.data();
    if (clGetProgramInfo(program, CL_PROGRAM_BINARIES,
                         sizeof(unsigned char*), &data, NULL) != CL_SUCCESS)
        return;

    std::ofstream f(filename.c_str(), std::ios::out | std::ios::binary);
    if (!f.is_open()) {
        std::cerr << "Не удалось сохранить бинарник ядра в " << filename << std::endl;
        return;
    }
    f.write((const char*)binary.data(), (std::streamsize)size);
}

#endif // LAB_KERNEL_BINARY_CACHE

// Готовая к запуску программа для устройства. В сборке с LAB_KERNEL_BINARY_CACHE
// ядро компилируется только один раз, дальше берётся сохранённый бинарник.
inline cl_program buildProgram(cl_context context, cl_device_id device,
                               const std::string& sourceStr)
{
#ifdef LAB_KERNEL_BINARY_CACHE
    std::string cacheFile = binaryCacheName(device, sourceStr);

    cl_program cached = loadProgramBinary(context, device, cacheFile);
    if (cached != NULL) {
        std::cout << "Ядро загружено из " << cacheFile << std::endl;
        return cached;
    }

    cl_program program = compileProgram(context, device, sourceStr);
    saveProgramBinary(program, cacheFile);
    std::cout << "Ядро скомпилировано и сохранено в " << cacheFile << std::endl;
    return program;
#else
    return compileProgram(context, device, sourceStr);
#endif
}

} // namespace ocl

#define CHECK(status, what) ocl::check((status), (what))

#endif // OCL_COMMON_H
