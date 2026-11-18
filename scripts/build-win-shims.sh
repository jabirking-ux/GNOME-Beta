#!/usr/bin/env bash
set -e
export PATH="/ucrt64/bin:$PATH"
cd "$(dirname "$0")/../win-shims"
mkdir -p build lib

gcc -O2 -Wall -Wextra -c src/win-shims-misc.c -o build/win-shims-misc.o \
  -Iinclude -Iinclude/webkitgtk-6.0 $(pkg-config --cflags gtk4 gio-2.0)
ar rcs lib/libwinshims.a build/win-shims-misc.o

for src in webkit-win32-session webkit-win32-types webkit-win32-webview; do
  gcc -O2 -Wall -Wextra -c "src/${src}.c" -o "build/${src}.o" \
    -Iinclude -Iinclude/webkitgtk-6.0 -I../third_party/webkit-headers \
    $(pkg-config --cflags gtk4 gio-2.0 libsoup-3.0)
done

ar rcs lib/libwinwebkitgtk.a \
  build/webkit-win32-session.o \
  build/webkit-win32-types.o \
  build/webkit-win32-webview.o

echo "Built win-shims and winwebkitgtk static libraries."
