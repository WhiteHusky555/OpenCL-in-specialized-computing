// Задание 4: два буфера float по N значений, попарное произведение A[i]*B[i].
#include "ocl_common.h"

#ifdef LAB_EMBED_KERNELS
#include "vector_mul_cl.h"   // заголовок генерируется из vector_mul.cl при сборке
#endif

#include <math.h>
#include <iomanip>

using namespace std;

static const cl_device_type SELECTED_DEVICE_TYPE = CL_DEVICE_TYPE_GPU;
#ifndef LAB_EMBED_KERNELS
static const char* KERNEL_FILE = "vector_mul.cl";
#endif

static const int N = 16;   // по заданию не менее 10

int main()
{
    ocl::initConsole();

    cl_float A[N], B[N], C[N];
    for (int i = 0; i < N; ++i) {
        A[i] = (cl_float)(i + 1);
        B[i] = (cl_float)(0.5f * (i + 1));
    }

    cl_device_id device = ocl::selectDevice(SELECTED_DEVICE_TYPE);
    if (device == NULL) {
        cerr << "GPU-устройства OpenCL не найдены." << endl;
        return 1;
    }
    ocl::printShortDeviceInfo(device);
    cout << "N = " << N << endl << endl;

    cl_int status;
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &status);
    CHECK(status, "clCreateContext");

    cl_command_queue commandQueue = clCreateCommandQueue(context, device, 0, &status);
    CHECK(status, "clCreateCommandQueue");

    std::string kernelSource;
#ifdef LAB_EMBED_KERNELS
    kernelSource = VECTOR_MUL_CL;
#else
    try {
        kernelSource = ocl::loadKernelSource(KERNEL_FILE);
    } catch (const std::exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }
#endif

    cl_program program = ocl::buildProgram(context, device, kernelSource);

    cl_mem bufferA = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                    N * sizeof(cl_float), (void*)A, &status);
    CHECK(status, "clCreateBuffer (буфер A)");

    cl_mem bufferB = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                    N * sizeof(cl_float), (void*)B, &status);
    CHECK(status, "clCreateBuffer (буфер B)");

    cl_mem bufferC = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                    N * sizeof(cl_float), NULL, &status);
    CHECK(status, "clCreateBuffer (буфер C)");

    cl_kernel kernel = clCreateKernel(program, "vecMul", &status);
    CHECK(status, "clCreateKernel (vecMul)");

    status  = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&bufferA);
    status |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&bufferB);
    status |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&bufferC);
    CHECK(status, "clSetKernelArg");

    size_t global_work_size[1] = { (size_t)N };   // по одному потоку на элемент
    status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
                                    global_work_size, NULL, 0, NULL, NULL);
    CHECK(status, "clEnqueueNDRangeKernel");

    status = clEnqueueReadBuffer(commandQueue, bufferC, CL_TRUE, 0,
                                 N * sizeof(cl_float), C, 0, NULL, NULL);
    CHECK(status, "clEnqueueReadBuffer");

    cout << fixed << setprecision(2);
    cout << " i      A[i]      B[i]  A[i]*B[i]" << endl;

    int errors = 0;
    for (int i = 0; i < N; ++i) {
        if (fabs(A[i] * B[i] - C[i]) > 1e-4f) ++errors;
        cout << setw(2) << i
             << setw(10) << A[i]
             << setw(10) << B[i]
             << setw(11) << C[i] << endl;
    }

    cout << endl << "Расхождений с расчётом на CPU: " << errors << endl;

    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseMemObject(bufferA);
    clReleaseMemObject(bufferB);
    clReleaseMemObject(bufferC);
    clReleaseCommandQueue(commandQueue);
    clReleaseContext(context);

    ocl::pause();
    return 0;
}
