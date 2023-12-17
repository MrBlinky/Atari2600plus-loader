@echo off
pushd "%~dp0"
Path = %CD%\Python;%CD%\Python\Scripts;%PATH%
python.exe %CD%\load2600p.py %*
popd
