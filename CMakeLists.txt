cmake_minimum_required(VERSION 3.20)
project(opencl_lab1 CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(LAB_FORCE_KHRONOS_OPENCL
       "Не искать SDK, а всегда брать заголовки и ICD-загрузчик Khronos" OFF)
option(LAB_EMBED_KERNELS
       "Вшить текст ядер в exe: файлы .cl рядом с программой больше не нужны" OFF)
option(LAB_KERNEL_BINARY_CACHE
       "Сохранять скомпилированное ядро в .bin и грузить его через clCreateProgramWithBinary" OFF)

# ---------------------------------------------------------------------
# Поиск OpenCL. Порядок:
#   1) установленный SDK (find_package: CUDA, AMD APP SDK, ocl-icd-opencl-dev, vcpkg);
#   2) каталог "AMD APP" рядом с проектом, если он есть локально;
#   3) заголовки и ICD-загрузчик Khronos, скачиваемые при конфигурировании.
# Благодаря этому библиотеки не нужно хранить в репозитории.
# ---------------------------------------------------------------------
if(NOT LAB_FORCE_KHRONOS_OPENCL)
    find_package(OpenCL QUIET)
endif()

if(OpenCL_FOUND)
    message(STATUS "OpenCL: найден SDK (${OpenCL_LIBRARIES})")
else()
    set(_amd_sdk "${CMAKE_CURRENT_SOURCE_DIR}/AMD APP")

    if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_amd_lib_dir "${_amd_sdk}/lib/x86_64")
    else()
        set(_amd_lib_dir "${_amd_sdk}/lib/x86")
    endif()

    if(MSVC)
        set(_amd_lib "${_amd_lib_dir}/OpenCL.lib")
    else()
        set(_amd_lib "${_amd_lib_dir}/libOpenCL.a")
    endif()

    if(NOT LAB_FORCE_KHRONOS_OPENCL AND EXISTS "${_amd_sdk}/include/CL/cl.h" AND EXISTS "${_amd_lib}")
        message(STATUS "OpenCL: используется локальный AMD APP SDK")
        add_library(OpenCL::OpenCL UNKNOWN IMPORTED)
        set_target_properties(OpenCL::OpenCL PROPERTIES
            IMPORTED_LOCATION "${_amd_lib}"
            INTERFACE_INCLUDE_DIRECTORIES "${_amd_sdk}/include")
    else()
        message(STATUS "OpenCL: SDK не найден, загружаются заголовки и ICD-загрузчик Khronos")
        include(FetchContent)
        set(FETCHCONTENT_QUIET OFF)

        FetchContent_Declare(OpenCLHeaders
            GIT_REPOSITORY https://github.com/KhronosGroup/OpenCL-Headers.git
            GIT_TAG        v2024.10.24
            GIT_SHALLOW    ON)
        FetchContent_MakeAvailable(OpenCLHeaders)

        set(OPENCL_ICD_LOADER_HEADERS_DIR "${openclheaders_SOURCE_DIR}"
            CACHE PATH "" FORCE)
        set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

        FetchContent_Declare(OpenCLICDLoader
            GIT_REPOSITORY https://github.com/KhronosGroup/OpenCL-ICD-Loader.git
            GIT_TAG        v2024.10.24
            GIT_SHALLOW    ON)
        FetchContent_MakeAvailable(OpenCLICDLoader)
    endif()
endif()

# ---------------------------------------------------------------------
# Программы лабораторной работы
# ---------------------------------------------------------------------
set(LAB_TARGETS
    task1_add_k
    task2_helloworld_gpu
    task3_helloworld_cpu
    task4_vector_mul
    task5_device_info)

# Какое ядро использует каждая программа (task5_device_info ядер не запускает).
set(task1_add_k_KERNEL          add_k)
set(task2_helloworld_gpu_KERNEL helloworld)
set(task3_helloworld_cpu_KERNEL helloworld)
set(task4_vector_mul_KERNEL     vector_mul)

# Заголовок с текстом ядра генерируется один раз, даже если ядро делят
# несколько программ: helloworld.cl используют задания 2 и 3.
if(LAB_EMBED_KERNELS)
    foreach(kernel add_k helloworld vector_mul)
        string(TOUPPER "${kernel}_CL" variable)
        set(header "${CMAKE_BINARY_DIR}/generated/${kernel}_cl.h")

        add_custom_command(
            OUTPUT "${header}"
            COMMAND ${CMAKE_COMMAND}
                    -DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/lab1/${kernel}.cl
                    -DOUTPUT=${header}
                    -DVARIABLE=${variable}
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_kernel.cmake
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/lab1/${kernel}.cl"
                    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_kernel.cmake"
            COMMENT "Встраивание ${kernel}.cl"
            VERBATIM)

        add_custom_target(embed_${kernel} DEPENDS "${header}")
    endforeach()
endif()

foreach(target IN LISTS LAB_TARGETS)
    add_executable(${target} lab1/${target}.cpp)
    target_link_libraries(${target} PRIVATE OpenCL::OpenCL)

    # Код написан под OpenCL 1.2; с более новыми заголовками это убирает
    # предупреждения об устаревших функциях (например, clCreateCommandQueue).
    target_compile_definitions(${target} PRIVATE
        CL_TARGET_OPENCL_VERSION=120
        CL_USE_DEPRECATED_OPENCL_1_2_APIS)

    if(LAB_KERNEL_BINARY_CACHE)
        target_compile_definitions(${target} PRIVATE LAB_KERNEL_BINARY_CACHE)
    endif()

    if(LAB_EMBED_KERNELS AND DEFINED ${target}_KERNEL)
        target_include_directories(${target} PRIVATE "${CMAKE_BINARY_DIR}/generated")
        target_compile_definitions(${target} PRIVATE LAB_EMBED_KERNELS)
        add_dependencies(${target} embed_${${target}_KERNEL})
    endif()

    if(MSVC)
        target_compile_options(${target} PRIVATE /utf-8 /W3)
    else()
        target_compile_options(${target} PRIVATE -Wall)
    endif()

    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin")
endforeach()

# Ядра .cl читаются во время выполнения, поэтому кладём их рядом с exe.
foreach(kernel add_k helloworld vector_mul)
    add_custom_command(TARGET task5_device_info POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/lab1/${kernel}.cl"
                "${CMAKE_BINARY_DIR}/bin/${kernel}.cl")
endforeach()

# ---------------------------------------------------------------------
# Вариант той же работы на CUDA. Собирается, только если найден nvcc,
# иначе OpenCL-часть строится как обычно.
# ---------------------------------------------------------------------
include(CheckLanguage)

# На Windows nvcc умеет работать только с хост-компилятором MSVC: при сборке
# генератором MinGW Makefiles он всё равно не соберётся, поэтому даже не ищем.
if(WIN32 AND NOT MSVC)
    message(STATUS "CUDA: пропущено, на Windows нужен генератор Visual Studio (nvcc требует cl.exe)")
else()
    check_language(CUDA)
    if(NOT CMAKE_CUDA_COMPILER)
        message(STATUS "CUDA: nvcc не найден, каталог lab1-cuda пропущен")
    endif()
endif()

if(CMAKE_CUDA_COMPILER)
    # Архитектуру нужно задать до enable_language, иначе CMake подставит свою.
    if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
        if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
            set(CMAKE_CUDA_ARCHITECTURES native)   # под установленную видеокарту
        else()
            set(CMAKE_CUDA_ARCHITECTURES 75 86)
        endif()
    endif()

    enable_language(CUDA)
    set(CMAKE_CUDA_STANDARD 11)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)

    message(STATUS "CUDA: ${CMAKE_CUDA_COMPILER} (архитектуры: ${CMAKE_CUDA_ARCHITECTURES})")

    foreach(target IN LISTS LAB_TARGETS)
        add_executable(cuda_${target} lab1-cuda/${target}.cu)

        # Имена совпадают с программами на OpenCL, поэтому кладём их отдельно.
        set_target_properties(cuda_${target} PROPERTIES
            OUTPUT_NAME ${target}
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin-cuda"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin-cuda"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin-cuda")

        if(MSVC)
            # /utf-8 нужен хост-компилятору, nvcc передаёт ключ через -Xcompiler
            target_compile_options(cuda_${target} PRIVATE
                $<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=/utf-8>)
        endif()
    endforeach()
endif()
