@echo off
REM
REM Copyright (C) 2013 University of Kaiserslautern
REM Microelectronic Systems Design Research Group
REM 
REM Christian Brugger (brugger@eit.uni-kl.de)
REM 30. August 2013
REM

mkdir build-mingw%1 || goto :error
cd build-mingw%1 || goto :error

REM cmake -G "MinGW Makefiles" CMAKE_C_COMPILER=gcc -D CMAKE_CXX_COMPILER=g++ .. || goto :error_cd
cmake -G "Unix Makefiles" CMAKE_C_COMPILER=gcc -D CMAKE_CXX_COMPILER=g++ .. || goto :error_cd
cd ..
goto :done

:error_cd
cd ..
:error
exit /b %errorlevel%
:done
