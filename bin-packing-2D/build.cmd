@echo off

set compiler=gcc
set raylib_path=C:/raylib/raylib/src
set lib_dirs=-L%raylib_path%
set include_dir=-I%raylib_path%
set libs=-lraylib -lopengl32 -lgdi32 -lwinmm

if %1==d (
    set compiler_flags=-Wall -Wextra -static 
    echo Building debug...
    call %compiler% %compiler_flags% -o bin\bin-packing-2D-dbg.exe -ggdb -D_DEBUG bin-packing-2D\main.c %include_dir% %lib_dirs% %libs%
) else if %1==r (
    set compiler_flags=-O3 -static
    echo Building release...
    call %compiler% %compiler_flags% -o bin\bin-packing-2D-rel.exe -DNDEBUG bin-packing-2D\main.c %include_dir% %lib_dirs% %libs%
)
