@echo off
g++ src/main.cpp -o PSXCueMaker-Release-Win32-64bit.exe -std=c++17 -m64 -s -mwindows -static -static-libgcc -O03 -lOle32 -Wall -Wextra -Wfatal-errors -pedantic -Wformat=2 -Wstrict-overflow=5 -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Winit-self -Wlogical-op -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wswitch-default -Wundef -Werror -Wzero-as-null-pointer-constant -Wuseless-cast -Wno-unused-parameter