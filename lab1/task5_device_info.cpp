// Задание 5: сведения об устройствах всех платформ OpenCL.
#include "ocl_common.h"

using namespace std;

static string mbytes(cl_ulong bytes)
{
    return to_string((unsigned long long)(bytes / (1024ULL * 1024ULL))) + " MB";
}

static string kbytes(cl_ulong bytes)
{
    return to_string((unsigned long long)(bytes / 1024ULL)) + " KB";
}

int main()
{
    ocl::initConsole();

    cl_uint numPlatforms = 0;
    cl_int status = clGetPlatformIDs(0, NULL, &numPlatforms);
    CHECK(status, "clGetPlatformIDs");

    if (numPlatforms == 0) {
        cout << "Платформы OpenCL не найдены." << endl;
        return 1;
    }

    vector<cl_platform_id> platforms(numPlatforms);
    status = clGetPlatformIDs(numPlatforms, platforms.data(), NULL);
    CHECK(status, "clGetPlatformIDs");

    cout << "Платформ: " << numPlatforms << endl;

    for (cl_uint p = 0; p < numPlatforms; ++p)
    {
        cout << endl << "Платформа " << p << endl;
        cout << "  CL_PLATFORM_NAME    : " << ocl::platformInfoStr(platforms[p], CL_PLATFORM_NAME)    << endl;
        cout << "  CL_PLATFORM_VENDOR  : " << ocl::platformInfoStr(platforms[p], CL_PLATFORM_VENDOR)  << endl;
        cout << "  CL_PLATFORM_VERSION : " << ocl::platformInfoStr(platforms[p], CL_PLATFORM_VERSION) << endl;
        cout << "  CL_PLATFORM_PROFILE : " << ocl::platformInfoStr(platforms[p], CL_PLATFORM_PROFILE) << endl;

        cl_uint numDevices = 0;
        status = clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);
        if (status != CL_SUCCESS || numDevices == 0) {
            cout << "  Устройств нет." << endl;
            continue;
        }

        vector<cl_device_id> devices(numDevices);
        status = clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_ALL, numDevices, devices.data(), NULL);
        CHECK(status, "clGetDeviceIDs");

        cout << "  Устройств: " << numDevices << endl;

        for (cl_uint d = 0; d < numDevices; ++d)
        {
            cl_device_id dev = devices[d];
            cout << endl << "  Устройство " << d << endl;

            cout << "    CL_DEVICE_NAME                     : " << ocl::deviceInfoStr(dev, CL_DEVICE_NAME) << endl;
            cout << "    CL_DEVICE_VENDOR                   : " << ocl::deviceInfoStr(dev, CL_DEVICE_VENDOR) << endl;
            cout << "    CL_DEVICE_TYPE                     : "
                 << ocl::deviceTypeToString(ocl::deviceInfoVal<cl_device_type>(dev, CL_DEVICE_TYPE)) << endl;
            cout << "    CL_DEVICE_VERSION                  : " << ocl::deviceInfoStr(dev, CL_DEVICE_VERSION) << endl;
            cout << "    CL_DRIVER_VERSION                  : " << ocl::deviceInfoStr(dev, CL_DRIVER_VERSION) << endl;
            cout << "    CL_DEVICE_OPENCL_C_VERSION         : " << ocl::deviceInfoStr(dev, CL_DEVICE_OPENCL_C_VERSION) << endl;
            cout << "    CL_DEVICE_MAX_COMPUTE_UNITS        : "
                 << ocl::deviceInfoVal<cl_uint>(dev, CL_DEVICE_MAX_COMPUTE_UNITS) << endl;
            cout << "    CL_DEVICE_MAX_CLOCK_FREQUENCY      : "
                 << ocl::deviceInfoVal<cl_uint>(dev, CL_DEVICE_MAX_CLOCK_FREQUENCY) << " MHz" << endl;
            cout << "    CL_DEVICE_MAX_WORK_GROUP_SIZE      : "
                 << (unsigned long long)ocl::deviceInfoVal<size_t>(dev, CL_DEVICE_MAX_WORK_GROUP_SIZE) << endl;

            cl_uint dims = ocl::deviceInfoVal<cl_uint>(dev, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);
            cout << "    CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS : " << dims << endl;

            if (dims > 0) {
                vector<size_t> sizes(dims, 0);
                if (clGetDeviceInfo(dev, CL_DEVICE_MAX_WORK_ITEM_SIZES,
                                    sizeof(size_t) * dims, sizes.data(), NULL) == CL_SUCCESS) {
                    cout << "    CL_DEVICE_MAX_WORK_ITEM_SIZES      : ";
                    for (cl_uint i = 0; i < dims; ++i)
                        cout << (unsigned long long)sizes[i] << (i + 1 < dims ? " x " : "");
                    cout << endl;
                }
            }

            cout << "    CL_DEVICE_GLOBAL_MEM_SIZE          : "
                 << mbytes(ocl::deviceInfoVal<cl_ulong>(dev, CL_DEVICE_GLOBAL_MEM_SIZE)) << endl;
            cout << "    CL_DEVICE_LOCAL_MEM_SIZE           : "
                 << kbytes(ocl::deviceInfoVal<cl_ulong>(dev, CL_DEVICE_LOCAL_MEM_SIZE)) << endl;
            cout << "    CL_DEVICE_MAX_MEM_ALLOC_SIZE       : "
                 << mbytes(ocl::deviceInfoVal<cl_ulong>(dev, CL_DEVICE_MAX_MEM_ALLOC_SIZE)) << endl;
            cout << "    CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE : "
                 << kbytes(ocl::deviceInfoVal<cl_ulong>(dev, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE)) << endl;
            cout << "    CL_DEVICE_ADDRESS_BITS             : "
                 << ocl::deviceInfoVal<cl_uint>(dev, CL_DEVICE_ADDRESS_BITS) << endl;
            cout << "    CL_DEVICE_AVAILABLE                : "
                 << (ocl::deviceInfoVal<cl_bool>(dev, CL_DEVICE_AVAILABLE) ? "да" : "нет") << endl;
            cout << "    CL_DEVICE_COMPILER_AVAILABLE       : "
                 << (ocl::deviceInfoVal<cl_bool>(dev, CL_DEVICE_COMPILER_AVAILABLE) ? "да" : "нет") << endl;
            cout << "    CL_DEVICE_IMAGE_SUPPORT            : "
                 << (ocl::deviceInfoVal<cl_bool>(dev, CL_DEVICE_IMAGE_SUPPORT) ? "да" : "нет") << endl;

            string ext = ocl::deviceInfoStr(dev, CL_DEVICE_EXTENSIONS);
            cout << "    CL_DEVICE_EXTENSIONS               : "
                 << (ext.size() > 300 ? ext.substr(0, 300) + " ..." : ext) << endl;
        }
    }

    ocl::pause();
    return 0;
}
