import subprocess

from . import logger

'''
usage:

for line in executeCmd(cmd):
    processedLine = line.rstrip()
    logger.debug(f"{processedLine}")
'''

class ExecutionError(Exception):
    pass

def executeCmd(cmd, env = None, cwd = None):
    popen = subprocess.Popen(cmd, stdout = subprocess.PIPE, stderr = subprocess.STDOUT,
                             universal_newlines = True, env = env, cwd = cwd)
    if popen != None and popen.stdout != None:
        for stdl in iter(popen.stdout.readline, ""):
            yield stdl
        popen.stdout.close()
        retCode = popen.wait()
        if retCode != 0:
            raise ExecutionError(f"Error: {retCode}")
    else:
        raise ExecutionError(f"Cannot open pipe for command: {cmd}")

def executeCmdWithLog(cmd, env = None, cwd = None):
    for line in executeCmd(cmd, env, cwd):
        processedLine = line.rstrip()
        logger.debug(f"{processedLine}")

def executeAndGetOutput(cmd, env = None, cwd = None):
    popen = subprocess.Popen(cmd, stdout = subprocess.PIPE, stderr = subprocess.STDOUT,
                             universal_newlines = True, env = env, cwd = cwd)
    try:
        outputs, errors = popen.communicate(timeout = 15)
        if errors != None:
            raise ExecutionError(f"Error: {errors}")
        return outputs
    except Exception as e:
        popen.kill()
        raise e
