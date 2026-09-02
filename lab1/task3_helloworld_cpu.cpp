// Задание 3: та же программа на CPU (CL_DEVICE_TYPE_GPU заменён на CL_DEVICE_TYPE_CPU).
// Ядро helloworld прибавляет 1 к коду каждого символа: GdkknVnqkc -> HelloWorld.
// Запуск: task3_helloworld_cpu.exe [строка]
#include "ocl_common.h"

#ifdef LAB_EMBED_KERNELS
#include "helloworld_cl.h"   // заголовок генерируется из helloworld.cl при сборке
#endif

using namespace std;

static const cl_device_type SELECTED_DEVICE_TYPE = CL_DEVICE_TYPE_CPU;
#ifndef LAB_EMBED_KERNELS
static const char* KERNEL_FILE = "helloworld.cl";
#endif

int main(int argc, char* argv[])
{
    ocl::initConsole();

    cl_device_id device = ocl::selectDevice(SELECTED_DEVICE_TYPE);
    if (device == NULL) {
        cerr << "CPU-устройства OpenCL не найдены (нужен OpenCL-runtime для процессора)." << endl;
        return 1;
    }
    ocl::printShortDeviceInfo(device);

    cl_int status;
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &status);
    CHECK(status, "clCreateContext");

    cl_command_queue commandQueue = clCreateCommandQueue(context, device, 0, &status);
    CHECK(status, "clCreateCommandQueue");

    std::string kernelSource;
#ifdef LAB_EMBED_KERNELS
    kernelSource = HELLOWORLD_CL;
#else
    try {
        kernelSource = ocl::loadKernelSource(KERNEL_FILE);
    } catch (const std::exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }
#endif

    cl_program program = ocl::buildProgram(context, device, kernelSource);

    const char* input = (argc > 1) ? argv[1] : "GdkknVnqkc";
    size_t strlength = strlen(input);
    char* output = (char*)malloc(strlength + 1);

    cl_mem inputBuffer = clCreateBuffer(context,
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        (strlength + 1) * sizeof(char), (void*)input, &status);
    CHECK(status, "clCreateBuffer (входной буфер)");

    cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
        (strlength + 1) * sizeof(char), NULL, &status);
    CHECK(status, "clCreateBuffer (выходной буфер)");

    cl_kernel kernel = clCreateKernel(program, "helloworld", &status);
    CHECK(status, "clCreateKernel (helloworld)");

    status  = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&inputBuffer);
    status |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&outputBuffer);
    CHECK(status, "clSetKernelArg");

    size_t global_work_size[1] = { strlength };
    status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
                                    global_work_size, NULL, 0, NULL, NULL);
    CHECK(status, "clEnqueueNDRangeKernel");

    status = clEnqueueReadBuffer(commandQueue, outputBuffer, CL_TRUE, 0,
                                 strlength * sizeof(char), output, 0, NULL, NULL);
    CHECK(status, "clEnqueueReadBuffer");

    output[strlength] = '\0';

    cout << "Вход:  " << input  << endl;
    cout << "Выход: " << output << endl;

    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseMemObject(inputBuffer);
    clReleaseMemObject(outputBuffer);
    clReleaseCommandQueue(commandQueue);
    clReleaseContext(context);

    if (output != NULL) { free(output); output = NULL; }
    ocl::pause();
    return 0;
}
