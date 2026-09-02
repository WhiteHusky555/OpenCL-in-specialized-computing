#!/usr/bin/env python3
"""Собирает страницу с результатами CI: логи сборки и вывод программ.

Использование: python3 ci/report.py <каталог с артефактами> <каталог страницы>
"""
import html
import os
import sys
from datetime import datetime, timezone

PROGRAMS = [
    ("task5_device_info", "Задание 5. Сведения об устройствах", "ANY"),
    ("task3_helloworld_cpu", "Задание 3. helloworld на CPU", "CPU"),
    ("task1_add_k", "Задание 1. O = I + K", "GPU"),
    ("task2_helloworld_gpu", "Задание 2. helloworld на GPU", "GPU"),
    ("task4_vector_mul", "Задание 4. A[i]*B[i], float", "GPU"),
]


def read(path, limit=200000):
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            text = f.read(limit)
    except OSError:
        return None
    return text


def block(text, missing="нет данных"):
    if text is None or not text.strip():
        return '<p class="muted">%s</p>' % html.escape(missing)
    return "<pre>%s</pre>" % html.escape(text.rstrip())


def status_badge(code):
    if code is None:
        return '<span class="badge badge-skip">не запускалось</span>'
    if code == 0:
        return '<span class="badge badge-ok">код 0</span>'
    return '<span class="badge badge-fail">код %d</span>' % code


def exit_code(root, name):
    text = read(os.path.join(root, "run", name + ".exit"))
    if text is None:
        return None
    try:
        return int(text.strip())
    except ValueError:
        return None


def platform_section(root, title, note):
    parts = ['<section><h2>%s</h2><p class="muted">%s</p>' % (html.escape(title), note)]

    parts.append("<h3>Лог сборки</h3>")
    parts.append(block(read(os.path.join(root, "build.log")), "сборка не выполнялась"))

    clinfo = read(os.path.join(root, "clinfo.txt"))
    if clinfo:
        parts.append("<h3>Устройства OpenCL на раннере</h3>")
        parts.append(block(clinfo))

    for name, caption, needs in PROGRAMS:
        out = read(os.path.join(root, "run", name + ".txt"))
        if out is None:
            continue
        code = exit_code(root, name)
        parts.append(
            '<h3>%s %s <span class="muted">(требуется устройство: %s)</span></h3>'
            % (html.escape(caption), status_badge(code), html.escape(needs))
        )
        parts.append(block(out))

    parts.append("</section>")
    return "\n".join(parts)


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        return 1

    src, dst = sys.argv[1], sys.argv[2]
    os.makedirs(dst, exist_ok=True)

    repo = os.environ.get("GITHUB_REPOSITORY", "")
    sha = os.environ.get("GITHUB_SHA", "")[:7]
    run_id = os.environ.get("GITHUB_RUN_ID", "")
    server = os.environ.get("GITHUB_SERVER_URL", "https://github.com")
    built = datetime.now(timezone.utc).strftime("%d.%m.%Y %H:%M UTC")

    links = []
    if repo and sha:
        links.append('коммит <a href="%s/%s/commit/%s">%s</a>' % (server, repo, sha, sha))
    if repo and run_id:
        links.append('<a href="%s/%s/actions/runs/%s">запуск CI</a>' % (server, repo, run_id))

    body = [
        "<h1>Программирование специализированных вычислителей</h1>",
        "<p>Лабораторная работа №1. Знакомство с OpenCL</p>",
        '<p class="muted">Собрано %s%s</p>'
        % (built, (" · " + " · ".join(links)) if links else ""),
        """
<div class="note">
<p><b>Что здесь есть.</b> Пять консольных программ на C++ и OpenCL собираются
на серверах GitHub Actions под Linux (g++) и Windows (MSVC). Ниже — логи сборки
и вывод программ, запущенных прямо на раннере.</p>
<p><b>Чего здесь нет.</b> Сами программы не запускаются в браузере: OpenCL нельзя
выполнить на странице, а видеокарты на бесплатных раннерах GitHub нет.
На Linux работает программный OpenCL (POCL), он даёт устройство типа CPU,
поэтому задания, запрашивающие <code>CL_DEVICE_TYPE_GPU</code>, честно сообщают
об отсутствии GPU. Windows-раннер устройств OpenCL не имеет вовсе —
там проверяется только сборка, а готовые .exe лежат в артефактах запуска.</p>
</div>
""",
        platform_section(
            os.path.join(src, "linux"),
            "Ubuntu · g++ · POCL",
            "Сборка через CMake, OpenCL из пакета ocl-icd-opencl-dev.",
        ),
        platform_section(
            os.path.join(src, "windows"),
            "Windows · MSVC",
            "Сборка через CMake, заголовки и ICD-загрузчик скачиваются у Khronos.",
        ),
    ]

    page = """<!doctype html>
<html lang="ru">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Программирование специализированных вычислителей</title>
<style>
:root { color-scheme: light dark; --bg:#ffffff; --fg:#1b1b1f; --muted:#5f6368;
        --card:#f6f7f9; --line:#dcdfe4; --ok:#1a7f37; --fail:#b3261e; --skip:#5f6368; }
@media (prefers-color-scheme: dark) {
  :root { --bg:#12131a; --fg:#e6e6ea; --muted:#a0a3ad; --card:#1b1d26; --line:#2c2f3a;
          --ok:#3fb950; --fail:#f85149; --skip:#a0a3ad; }
}
* { box-sizing: border-box; }
body { margin:0 auto; padding:2rem 1.25rem 4rem; max-width:60rem; background:var(--bg);
       color:var(--fg); font:16px/1.55 system-ui, "Segoe UI", Roboto, sans-serif; }
h1 { font-size:1.6rem; margin:0 0 .4rem; }
h2 { font-size:1.25rem; margin:2.5rem 0 .3rem; padding-top:1rem; border-top:1px solid var(--line); }
h3 { font-size:1rem; margin:1.5rem 0 .5rem; font-weight:600; }
a { color:inherit; }
.muted { color:var(--muted); font-size:.9rem; margin:.2rem 0; }
.note { background:var(--card); border:1px solid var(--line); border-radius:8px;
        padding:.9rem 1.1rem; margin:1.2rem 0 0; }
.note p { margin:.5rem 0; font-size:.95rem; }
pre { background:var(--card); border:1px solid var(--line); border-radius:8px;
      padding:.8rem 1rem; overflow-x:auto; font:13px/1.45 ui-monospace, Consolas, monospace; }
code { font-family: ui-monospace, Consolas, monospace; font-size:.92em; }
.badge { display:inline-block; padding:.05rem .5rem; border-radius:999px; font-size:.78rem;
         border:1px solid currentColor; vertical-align:middle; }
.badge-ok { color:var(--ok); } .badge-fail { color:var(--fail); } .badge-skip { color:var(--skip); }
</style>
</head>
<body>
%s
</body>
</html>
""" % "\n".join(body)

    with open(os.path.join(dst, "index.html"), "w", encoding="utf-8") as f:
        f.write(page)

    print("Страница записана в", os.path.join(dst, "index.html"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
