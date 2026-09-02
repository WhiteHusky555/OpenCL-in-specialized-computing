// Задание 1: обработка буфера по формуле O = I + K (char).
// Запуск: task1_add_k.exe [строка] [K]      по умолчанию "GdkknVnqkc" и K = 1
#include "ocl_common.h"

#ifdef LAB_EMBED_KERNELS
#include "add_k_cl.h"   // заголовок генерируется из add_k.cl при сборке
#endif

using namespace std;

static const cl_device_type SELECTED_DEVICE_TYPE = CL_DEVICE_TYPE_GPU;
#ifndef LAB_EMBED_KERNELS
static const char* KERNEL_FILE = "add_k.cl";
#endif

int main(int argc, char* argv[])
{
    ocl::initConsole();

    const char* input = (argc > 1) ? argv[1] : "GdkknVnqkc";
    cl_char K = (argc > 2) ? (cl_char)atoi(argv[2]) : (cl_char)1;

    cl_device_id device = ocl::selectDevice(SELECTED_DEVICE_TYPE);
    if (device == NULL) {
        cerr << "GPU-устройства OpenCL не найдены." << endl;
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
    kernelSource = ADD_K_CL;
#else
    try {
        kernelSource = ocl::loadKernelSource(KERNEL_FILE);
    } catch (const std::exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }
#endif

    cl_program program = ocl::buildProgram(context, device, kernelSource);

    size_t strlength = strlen(input);
    char* output = (char*)malloc(strlength + 1);

    cl_mem inputBuffer = clCreateBuffer(context,
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        (strlength + 1) * sizeof(char), (void*)input, &status);
    CHECK(status, "clCreateBuffer (входной буфер)");

    cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
        (strlength + 1) * sizeof(char), NULL, &status);
    CHECK(status, "clCreateBuffer (выходной буфер)");

    cl_kernel kernel = clCreateKernel(program, "addK", &status);
    CHECK(status, "clCreateKernel (addK)");

    status  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void*)&inputBuffer);
    status |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void*)&outputBuffer);
    status |= clSetKernelArg(kernel, 2, sizeof(cl_char), (void*)&K);
    CHECK(status, "clSetKernelArg");

    size_t global_work_size[1] = { strlength };   // по одному потоку на символ
    status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
                                    global_work_size, NULL, 0, NULL, NULL);
    CHECK(status, "clEnqueueNDRangeKernel");

    status = clEnqueueReadBuffer(commandQueue, outputBuffer, CL_TRUE, 0,
                                 strlength * sizeof(char), output, 0, NULL, NULL);
    CHECK(status, "clEnqueueReadBuffer");

    output[strlength] = '\0';

    cout << "K = " << (int)K << endl;
    cout << "I: " << input  << endl;
    cout << "O: " << output << endl << endl;

    for (size_t i = 0; i < strlength; ++i) {
        cout << "  " << input[i] << " " << (int)(unsigned char)input[i]
             << " -> " << output[i] << " " << (int)(unsigned char)output[i] << endl;
    }

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
