@echo off
REM
REM Copyright (C) 2013 University of Kaiserslautern
REM Microelectronic Systems Design Research Group
REM 
REM Christian Brugger (brugger@eit.uni-kl.de)
REM 30. August 2013
REM

REM rmdir /s/q build-msvc

mkdir build-msvc%1 || goto :error
cd build-msvc%1 || goto :error
cmake -G "Visual Studio 11" ..  || goto :error_cd
cd ..
goto :done

:error_cd
cd ..
:error
exit /b %errorlevel%
:done