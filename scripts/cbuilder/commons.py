import os
import json
import pathlib

from .pytils import logger
from .pytils import exec

###############################################################################

class ConfigsDictionary:
    def __init__(self, **kwargs):
        for key, value in kwargs.items():
            if isinstance(value, dict):
                self.__dict__[key] = ConfigsDictionary(**value)
            else:
                self.__dict__[key] = value

# Windows
class WindowsEnvConfigs:
    windowsKitsVersion: str | None
    windowsKitsIncludeDir: str | None
    windowsKitsLibDir: str | None
    windowsKitsBinDir: str | None
    msvcToolsetsVersion: str | None
    msvcToolsetsDir: str | None
    vulkanSDKVersion: str | None
    clangVersion: str | None

    windowsKitsPath: pathlib.Path | None

class WindowsToolChain:
    cxxCompiler: pathlib.Path | None
    cCompiler: pathlib.Path | None
    cxxLinker: pathlib.Path | None
    resCompiler: pathlib.Path | None
    dx12ShaderCompiler: pathlib.Path | None

###############################################################################

class EnvConfigs:
    sdkRoot = ""
    pass

class ToolChain:
    cxxCompiler = ""
    cxxLinker = ""
    shaderCompiler = ""
    dx12ShaderCompiler = ""
    pass

class CompileConfigs:
    appName: str | None

    buildDir = ""
    binaryOutputDir = ""
    cookedDataDir = ""

    shaderFlags = []
    debugFlags: list[str] | None
    releaseFlags: list[str] | None
    cxxFlags: list[str] | None
    cFlags: list[str] | None
    linkerFlagsBegin = []
    linkerFlags: list[str] | None
    linkerFlagsEnd = []

    systemIncludeDirs = []
    includeDirs = []
    libraryDirs = []
    libraries: list[str]

    defines = []

    excludeSourceDirs: list | None
    excludeSourceFiles: list | None
    includeSourceDirs: list | None
    includeSourceFiles: list | None
    pass

###############################################################################

class Define:
    def __init__(self, identifier, value = None):
        self.identifier = identifier
        self.value = value

class TUPair():
    def __init__(self, sourceFile, objFile, debugFile):
        self.sourceFile = sourceFile
        self.objFile = objFile
        self.debugFile = debugFile

###############################################################################

def loadConfigs(configsFilePath: pathlib.Path) -> dict:
    with open(configsFilePath, mode = "r", encoding = "utf-8") as f:
        configs: dict = json.load(f)
        return configs

def gatherShaders(**kwargs):
    shaderType = kwargs['type']
    extension = kwargs['extension']
    workingDir = kwargs['workingDir']
    cookedDataDir = kwargs['cookedDataDir']
    objFileExt = kwargs['objFileExt']
    shaderDir = kwargs['shaderDir']
    shaderBaseDir = kwargs['shaderBaseDir']

    dbgFileExt = kwargs.get('dbgFileExt', None)

    logger.info(f"Gathering {shaderType} shaders...")
    shaderTUPairs = []
    for currentPath, _, files in os.walk(shaderDir):
        for f in files:
            srcFileAbsPath = os.path.join(currentPath, f)
            srcFileRelPath = os.path.relpath(srcFileAbsPath, workingDir)
            if os.path.isfile(srcFileRelPath):
                rootExt = os.path.splitext(srcFileRelPath)
                srcFileRelPathExt = rootExt[1].lower()
                srcFileRelPathNoExt = rootExt[0].lower()
                if srcFileRelPathExt == f".{extension}":
                    objFile = os.path.join(cookedDataDir, os.path.relpath(srcFileRelPathNoExt + f".{objFileExt}", shaderBaseDir))
                    dbgFile = None
                    if dbgFileExt!= None:
                        dbgFile = os.path.join(cookedDataDir, os.path.relpath(srcFileRelPathNoExt + f".{dbgFileExt}", shaderBaseDir))

                    shaderTUPairs.append(TUPair(srcFileRelPath, objFile, dbgFile))
                    logger.debug(f"{extension}: {srcFileRelPath}")
                    logger.debug(f"      binary => {objFile}")
                    if dbgFile != None:
                        logger.debug(f"      debug  => {dbgFile}")
    return shaderTUPairs

def gatherCSources(configs, srcDir, currDir, objFileExt, debugFileExt = None):
    logger.info("Gathering C sources...")
    binaryOutputRelPath = configs.buildDir
    tuPairs = []
    for currentPath, _, files in os.walk(srcDir):
        currentRelPath = os.path.relpath(currentPath, currDir)

        dirExcluded = False
        for excludeDir in configs.excludeSourceDirs:
            if os.path.commonprefix([currentRelPath, excludeDir]) == excludeDir:
                dirExcluded = True
                break
        if dirExcluded:
            # logger.warning(f"Excluded directory: {currentRelPath}")
            continue

        for f in files:
            srcFileAbsPath = os.path.join(currentPath, f)
            srcFileRelPath = os.path.relpath(srcFileAbsPath, currDir)
            if srcFileRelPath in configs.excludeSourceFiles:
                # logger.warning(f"Excluded file: {srcFileRelPath}")
                continue

            if os.path.isfile(srcFileRelPath):
                rootExt = os.path.splitext(srcFileRelPath)
                srcFileRelPathExt = rootExt[1].lower()
                srcFileRelPathNoExt = rootExt[0].lower()
                if srcFileRelPathExt == ".c":
                    objFile = os.path.join(binaryOutputRelPath, srcFileRelPathNoExt + "." + objFileExt)
                    debugFile = None
                    if debugFileExt != None:
                        debugFile = os.path.join(binaryOutputRelPath, srcFileRelPathNoExt + "." + debugFileExt)

                    tuPairs.append(TUPair(srcFileRelPath, objFile, debugFile))
                    logger.debug(f"c: {srcFileRelPath}")
                    logger.debug(f"   obj => {objFile}")
                    if debugFileExt != None:
                        logger.debug(f"   pdb => {debugFile}")

    for srcFileRelPath in configs.includeSourceFiles:
        logger.info(f"Included file: {srcFileRelPath}")

        if os.path.isfile(srcFileRelPath):
            rootExt = os.path.splitext(srcFileRelPath)
            srcFileRelPathExt = rootExt[1].lower()
            srcFileRelPathNoExt = rootExt[0].lower()
            if srcFileRelPathExt == ".c":
                objFile = os.path.join(binaryOutputRelPath, srcFileRelPathNoExt + "." + objFileExt)
                debugFile = None
                if debugFileExt != None:
                    debugFile = os.path.join(binaryOutputRelPath, srcFileRelPathNoExt + "." + debugFileExt)

                tuPairs.append(TUPair(srcFileRelPath, objFile, debugFile))
                logger.debug(f"c: {srcFileRelPath}")
                logger.debug(f"   obj => {objFile}")
                if debugFileExt != None:
                    logger.debug(f"   pdb => {debugFile}")

    return tuPairs

def gatherCppSources(configs, srcDir, currDir, objFileExt, debugFileExt = None):
    logger.info("Gathering C++ sources...")
    binaryOutputRelPath = configs.buildDir
    tuPairs = []
    for currentPath, _, files in os.walk(srcDir):
        currentRelPath = os.path.relpath(currentPath, currDir)

        dirExcluded = False
        for excludeDir in configs.excludeSourceDirs:
            if os.path.commonprefix([currentRelPath, excludeDir]) == excludeDir:
                dirExcluded = True
                break
        if dirExcluded:
            # logger.warning(f"Excluded directory: {currentRelPath}")
            continue

        for f in files:
            srcFileAbsPath = os.path.join(currentPath, f)
            srcFileRelPath = os.path.relpath(srcFileAbsPath, currDir)
            if srcFileRelPath in configs.excludeSourceFiles:
                # logger.warning(f"Excluded file: {srcFileRelPath}")
                continue

            if os.path.isfile(srcFileRelPath):
                rootExt = os.path.splitext(srcFileRelPath)
                srcFileRelPathExt = rootExt[1].lower()
                srcFileRelPathNoExt = rootExt[0].lower()
                if srcFileRelPathExt == ".cpp":
                    objFile = os.path.join(binaryOutputRelPath, srcFileRelPathNoExt + "." + objFileExt)
                    debugFile = None
                    if debugFileExt != None:
                        debugFile = os.path.join(binaryOutputRelPath, srcFileRelPathNoExt + "." + debugFileExt)

                    tuPairs.append(TUPair(srcFileRelPath, objFile, debugFile))
                    logger.debug(f"c++: {srcFileRelPath}")
                    logger.debug(f"     obj => {objFile}")
                    if debugFileExt != None:
                        logger.debug(f"     pdb => {debugFile}")

    for srcFileRelPath in configs.includeSourceFiles:
        logger.info(f"Included file: {srcFileRelPath}")

        if os.path.isfile(srcFileRelPath):
            rootExt = os.path.splitext(srcFileRelPath)
            srcFileRelPathExt = rootExt[1].lower()
            srcFileRelPathNoExt = rootExt[0].lower()
            if srcFileRelPathExt == ".cpp":
                objFile = os.path.join(binaryOutputRelPath, srcFileRelPathNoExt + "." + objFileExt)
                debugFile = None
                if debugFileExt != None:
                    debugFile = os.path.join(binaryOutputRelPath, srcFileRelPathNoExt + "." + debugFileExt)

                tuPairs.append(TUPair(srcFileRelPath, objFile, debugFile))
                logger.debug(f"c++: {srcFileRelPath}")
                logger.debug(f"     obj => {objFile}")
                if debugFileExt != None:
                    logger.debug(f"     pdb => {debugFile}")

    return tuPairs

def gatherRcSources(configs, srcDir, currDir, objFileExt):
    logger.info("Gathering RC sources...")
    binaryOutputRelPath = configs.buildDir
    tuPairs = []
    for currentPath, _, files in os.walk(srcDir):
        currentRelPath = os.path.relpath(currentPath, currDir)

        dirExcluded = False
        for excludeDir in configs.excludeSourceDirs:
            if os.path.commonprefix([currentRelPath, excludeDir]) == excludeDir:
                dirExcluded = True
                break
        if dirExcluded:
            # logger.warning(f"Excluded directory: {currentRelPath}")
            continue

        for f in files:
            srcFileAbsPath = os.path.join(currentPath, f)
            srcFileRelPath = os.path.relpath(srcFileAbsPath, currDir)
            if srcFileRelPath in configs.excludeSourceFiles:
                # logger.warning(f"Excluded file: {srcFileRelPath}")
                continue

            if os.path.isfile(srcFileRelPath):
                rootExt = os.path.splitext(srcFileRelPath)
                srcFileRelPathExt = rootExt[1].lower()
                srcFileRelPathNoExt = rootExt[0].lower()
                if srcFileRelPathExt == ".rc":
                    objFile = os.path.join(binaryOutputRelPath, srcFileRelPathNoExt + "." + objFileExt)

                    tuPairs.append(TUPair(srcFileRelPath, objFile, None))
                    logger.debug(f"c++: {srcFileRelPath}")
                    logger.debug(f"     obj => {objFile}")

    return tuPairs

###############################################################################

def generateUnitySource(buildDir, workingDir, sourceName, sourceExt, debugExt, tuPairs):
    unitySrcDir = os.path.join(workingDir, buildDir, "unity_src")
    os.makedirs(unitySrcDir, exist_ok = True)
    unitySrcFilepath = os.path.normpath(os.path.join(unitySrcDir, f"{sourceName}.{sourceExt}"))
    unityObjFilepath = os.path.normpath(os.path.join(unitySrcDir, f"{sourceName}.obj"))
    unityDbgFilepath = None
    if debugExt != None:
        unityDbgFilepath = os.path.normpath(os.path.join(unitySrcDir, f"{sourceName}.{debugExt}"))
    with open(unitySrcFilepath, mode = "w", encoding = "utf-8") as f:
        for tuPair in tuPairs:
            absSourcePath = os.path.join(workingDir, tuPair.sourceFile)
            absSourcePath = absSourcePath.replace(os.sep, '/')
            f.write(f'#include "{absSourcePath}"\n')
            logger.debug(f"  + {tuPair.sourceFile}")
        logger.info(f"Generated unity source: {unitySrcFilepath}")

    return TUPair(unitySrcFilepath, unityObjFilepath, unityDbgFilepath)

###############################################################################

def dumpCompilationDatabase(currDir, ccPath, ccData):
    compileCommandsFilepath = os.path.join(currDir, ccPath)
    with open(compileCommandsFilepath, mode = "w", encoding = "utf-8") as f:
        f.write(json.dumps(ccData, indent = 2))
    logger.info(f"Compile commands path: {compileCommandsFilepath}")

###############################################################################

def compileDX12Shaders(workingDir, compiler, tuPairs, noBuild):
    cmdPrefix = f'"{os.path.normpath(compiler)}" -Zi '
    for tuPair in tuPairs:
        sourcePathTuple = os.path.splitext(tuPair.sourceFile)
        if sourcePathTuple[0].endswith("_vv"):
            cmdPrefix += "-T vs_6_0 "
        else:
            cmdPrefix += "-T ps_6_0 "

        csoFileAbsPath = os.path.join(workingDir, tuPair.objFile)
        pdbFileAbsPath = os.path.join(workingDir, tuPair.debugFile)
        os.makedirs(os.path.dirname(csoFileAbsPath), exist_ok = True)
        cmd = cmdPrefix + f'-Fo "{csoFileAbsPath}" -Fd "{pdbFileAbsPath}" "{tuPair.sourceFile}"'

        if not noBuild:
            logger.debug(f"Compiling shader: {tuPair.sourceFile}...")
            exec.executeCmdWithLog(cmd)

###############################################################################

def findLatestVersion(name: str, path: str | pathlib.Path):
    availVers = os.listdir(path)
    availVers.sort(reverse = True)

    for ver in availVers:
        verParts = ver.split(".")

        validVersionName = True
        for vp in verParts:
            if not vp.isnumeric():
                validVersionName = False
                logger.warning(f"Version {ver} is not valid. Skip.")
                break

        if validVersionName:
            logger.info(f"Latest {name} version: {ver}.")
            return ver

    raise RuntimeError(f"Cannot find any version code at {path}.")

###############################################################################

def findVSInstall():
    currFilePath = os.path.dirname(os.path.realpath(__file__))
    cmd = f"{currFilePath}\\binutils\\vswhere.exe -format json -latest"
    return exec.executeAndGetOutput(cmd)
