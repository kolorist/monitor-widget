@echo off

call __venv\Scripts\activate

call python scripts\build.py windows msvc --cc compile_commands.json --unity %*

echo Error-Code: %ERRORLEVEL%

call deactivate
