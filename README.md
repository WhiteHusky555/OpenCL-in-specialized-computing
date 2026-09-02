# Программирование специализированных вычислителей

[![build](https://github.com/WhiteHusky555/OpenCL-in-specialized-computing/actions/workflows/build.yml/badge.svg)](https://github.com/WhiteHusky555/OpenCL-in-specialized-computing/actions/workflows/build.yml)

Лабораторные работы по дисциплине «Программирование специализированных вычислителей» (РГРТУ).
Каждая работа лежит в своём каталоге и описана в своём README.

## Работы

| № | Тема | Исходники и описание | Состояние |
|---|---|---|---|
| 1 | Знакомство с OpenCL: буферы, ядра, выбор устройства | [lab1/](lab1/README.md) | готово |

Отчёт последней сборки на GitHub Pages:
`https://whitehusky555.github.io/OpenCL-in-specialized-computing/`

## Структура репозитория

| Путь | Что это |
|---|---|
| [lab1/](lab1/) | лабораторная работа №1 |
| [CMakeLists.txt](CMakeLists.txt) | общая сборка всех работ |
| [.github/workflows/build.yml](.github/workflows/build.yml) | сборка и запуск на серверах GitHub |
| [ci/report.py](ci/report.py) | формирование страницы отчёта из логов CI |
| `Спец ЭВМ ЛР1.pdf`, `.docx` | методические указания с заданиями |

Спецификация: [OpenCL 1.2](https://registry.khronos.org/OpenCL/specs/opencl-1.2.pdf),
[справочник функций](https://registry.khronos.org/OpenCL/sdk/1.2/docs/man/xhtml/).

## Требования

* CMake 3.20+ и компилятор C++11 (проверено на MinGW-w64 g++ 14, MSVC 2022, g++ 13 в Ubuntu).
* OpenCL-runtime устройства, на котором запускаете: драйвер видеокарты
  (NVIDIA, AMD, Intel) либо runtime для процессора.
* SDK ставить не нужно: если `find_package(OpenCL)` ничего не нашёл, CMake скачает
  заголовки и ICD-загрузчик Khronos.

## Сборка

```
cmake -S . -B build
cmake --build build --config Release
```

Бинарники и файлы ядер `.cl` окажутся в `build/bin`.

| Опция | Зачем |
|---|---|
| `-DLAB_EMBED_KERNELS=ON` | вшить текст ядер в exe: файлы `.cl` рядом с программой больше не нужны |
| `-DLAB_KERNEL_BINARY_CACHE=ON` | компилировать ядро один раз, дальше грузить сохранённый `.bin` через `clCreateProgramWithBinary` |
| `-DLAB_FORCE_KHRONOS_OPENCL=ON` | не искать SDK в системе, всегда качать заголовки и загрузчик у Khronos |
| `-DOpenCL_ROOT=<путь>` | указать конкретный SDK вручную |

Порядок поиска OpenCL в [CMakeLists.txt](CMakeLists.txt): `find_package(OpenCL)` →
каталог `AMD APP` в корне репозитория, если он есть локально (в git не хранится) →
`FetchContent` с репозиториев Khronos.

## CI

[`.github/workflows/build.yml`](.github/workflows/build.yml) на каждый push:

* **Ubuntu** — сборка g++, установка POCL (программный OpenCL), запуск программ;
* **Windows** — сборка MSVC; `.exe` и `.cl` кладутся в артефакты запуска
  (Actions → запуск → Artifacts), их можно скачать и не собирать самому;
* **Pages** — [`ci/report.py`](ci/report.py) собирает логи сборки и вывод программ
  в одну статическую страницу.

Видеокарты на раннерах нет, POCL даёт только устройство типа CPU: программы,
запрашивающие `CL_DEVICE_TYPE_GPU`, на сервере сообщают об отсутствии GPU.
Сборку это не ломает.
