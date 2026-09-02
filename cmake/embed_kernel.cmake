# Генерирует C++ заголовок с текстом ядра OpenCL, чтобы его можно было вшить в exe.
#
# cmake -DINPUT=<ядро.cl> -DOUTPUT=<ядро_cl.h> -DVARIABLE=<ИМЯ_КОНСТАНТЫ> -P embed_kernel.cmake

file(READ "${INPUT}" source)

string(REPLACE "\\" "\\\\" source "${source}")
string(REPLACE "\"" "\\\"" source "${source}")
string(REPLACE "\r" "" source "${source}")
string(REPLACE "\n" "\\n\"\n    \"" source "${source}")

get_filename_component(name "${INPUT}" NAME)

file(WRITE "${OUTPUT}"
"// Сгенерировано из ${name} при сборке, редактировать не нужно.
#pragma once

static const char* ${VARIABLE} =
    \"${source}\";
")
