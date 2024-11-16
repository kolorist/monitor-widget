@echo off

REM Create Python virtual environment
if not exist "__venv" (
    python -m venv __venv
)
call __venv\Scripts\activate
call pip install -r reqs.txt
call deactivate

popd
