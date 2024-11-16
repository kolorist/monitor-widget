import os
from collections.abc import Callable

from .pytils import logger
from .pytils import exec

from . import commons
from .commons import ConfigsDictionary

###############################################################################

def register(subParsers,
             configurator: Callable[[str, bool, bool], ConfigsDictionary | None],
             postBuildAction: Callable[[ConfigsDictionary], None] | None = None):
    parser = subParsers.add_parser("windows", help = "Windows build")

    sps = parser.add_subparsers(help = "Compiler")

    msvcCompilerParser = sps.add_parser('msvc', help = "MSVC")
    msvcCompilerParser.set_defaults(execute = executeMSVC,
                                    configurator = configurator,
                                    postBuildAction = postBuildAction)

    msvcCompilerParser.add_argument('--cc', help = "compile_commands.json path",
                                    action = "store", default = None)
    msvcCompilerParser.add_argument('--nobuild', help = "Do not perform building",
                                    action = "store_true")
    msvcCompilerParser.add_argument('--optimize', help = "Perform optimized build",
                                    action = "store_true")
    msvcCompilerParser.add_argument('--asan', help="Enable address sanitizer",
                                    action = "store_true")
    msvcCompilerParser.add_argument('--unity', help = "Perform unity build",
                                    action = "store_true")

    clangCompilerParser = sps.add_parser('clang', help = "Clang")
    clangCompilerParser.set_defaults(execute = executeClang,
                                     configurator = configurator,
                                     postBuildAction = postBuildAction)

    clangCompilerParser.add_argument('--cc', help = "compile_commands.json path",
                                     action = "store", default = None)
    clangCompilerParser.add_argument('--nobuild', help = "Do not perform building",
                                     action = "store_true")
    clangCompilerParser.add_argument('--optimize', help = "Perform optimized build",
                                     action = "store_true")
    clangCompilerParser.add_argument('--asan', help="Enable address sanitizer",
                                     action = "store_true")
    clangCompilerParser.add_argument('--unity', help = "Perform unity build",
                                     action = "store_true")

###############################################################################

def executeMSVC(args):
    configs = args.configurator(toolChainDef = "msvc", enableOptimization = args.optimize, enableAsan = args.asan)
    buildMSVC(args.cc, args.nobuild, args.unity, args.optimize, args.asan, configs)
    if args.postBuildAction != None:
        logger.info("Running post-build action...")
        args.postBuildAction(configs)
    logger.info("All done, enjoy :D")

def executeClang(args):
    configs = args.configurator(toolChainDef = "clang", enableOptimization = args.optimize, enableAsan = args.asan)
    buildClang(args.cc, args.nobuild, args.unity, args.optimize, args.asan, configs)
    if args.postBuildAction != None:
        logger.info("Running post-build action...")
        args.postBuildAction(configs)
    logger.info("All done, enjoy :D")

###############################################################################

def clBuildCompileFlags(flags, defines, systemIncludeDirs, includeDirs, srcDir, resDir):
    compileFlags = "-c "
    for flag in flags:
        compileFlags += f"{flag} "

    for define in defines:
        if define.value == None:
            compileFlags += f"-D{define.identifier} "
        else:
            compileFlags += f"-D{define.identifier}={define.value} "

    for includeDir in systemIncludeDirs:
        compileFlags += f'-isystem"{includeDir}" '

    for includeDir in includeDirs:
        compileFlags += f'-I"{includeDir}" '

    compileFlags += f'-I"{srcDir}" '
    if os.path.isdir(resDir):
        compileFlags += f'-I"{resDir}" '

    return compileFlags

def msvcBuildCompileFlags(flags, defines, systemIncludeDirs, includeDirs, srcDir, resDir):
    compileFlags = "/c "
    for flag in flags:
        compileFlags += f"{flag} "

    for define in defines:
        if define.value == None:
            compileFlags += f"/D {define.identifier} "
        else:
            compileFlags += f"/D {define.identifier}={define.value} "

    for includeDir in systemIncludeDirs:
        compileFlags += f'/I"{includeDir}" '

    for includeDir in includeDirs:
        compileFlags += f'/I"{includeDir}" '

    compileFlags += f'/I"{srcDir}" '
    if os.path.isdir(resDir):
        compileFlags += f'/I"{resDir}" '

    return compileFlags

def clCompile(compiler, compileFlags, workingDir, buildDir, tuPairs, noBuild, unityUnitName):
    ccData = []
    objFilesStr = ""
    compileCmdPrefix = f'"{os.path.normpath(compiler)}" {compileFlags}'

    for tuPair in tuPairs:
        objFileAbsPath = os.path.join(workingDir, tuPair.objFile)
        os.makedirs(os.path.dirname(objFileAbsPath), exist_ok = True)
        compileCmd = f'{compileCmdPrefix}{tuPair.sourceFile} -o "{objFileAbsPath}" '
        if not noBuild and unityUnitName == None:
            logger.debug(os.path.basename(tuPair.sourceFile))
            exec.executeCmdWithLog(compileCmd)
            objFileRelPath = os.path.relpath(objFileAbsPath, workingDir)
            objFilesStr += objFileRelPath + " "

        ccData.append({
            "directory": workingDir,
            "command": compileCmd,
            "file": tuPair.sourceFile
            })

    if unityUnitName != None:
        nameParts = unityUnitName.split(".")
        unityUnit = commons.generateUnitySource(buildDir, workingDir, nameParts[0],
                                                nameParts[1], None, tuPairs)
        objFileAbsPath = os.path.join(workingDir, unityUnit.objFile)
        os.makedirs(os.path.dirname(objFileAbsPath), exist_ok = True)
        compileCmd = f'{compileCmdPrefix}{unityUnit.sourceFile} -o "{objFileAbsPath}" '
        if not noBuild:
            logger.debug(os.path.basename(unityUnit.sourceFile))
            exec.executeCmdWithLog(compileCmd)
        objFileRelPath = os.path.relpath(objFileAbsPath, workingDir)
        objFilesStr += objFileRelPath + " "

    return ccData, objFilesStr

def msvcCompile(compiler, compileFlags, workingDir, buildDir, tuPairs, noBuild, unityUnitName):
    ccData = []
    objFilesStr = ""
    compileCmdPrefix = f'"{os.path.normpath(compiler)}" {compileFlags}'

    for tuPair in tuPairs:
        objFileAbsPath = os.path.join(workingDir, tuPair.objFile)
        dbgFileAbsPath = os.path.join(workingDir, tuPair.debugFile)
        os.makedirs(os.path.dirname(objFileAbsPath), exist_ok = True)
        compileCmd = f'{compileCmdPrefix}{tuPair.sourceFile} /Fo"{objFileAbsPath}" /Fd"{dbgFileAbsPath}"'
        if not noBuild and unityUnitName == None:
            exec.executeCmdWithLog(compileCmd)
            objFileRelPath = os.path.relpath(objFileAbsPath, workingDir)
            objFilesStr += objFileRelPath + " "

        ccData.append({
            "directory": workingDir,
            "command": compileCmd,
            "file": tuPair.sourceFile
            })

    if unityUnitName != None:
        nameParts = unityUnitName.split(".")
        unityUnit = commons.generateUnitySource(buildDir, workingDir, nameParts[0],
                                                nameParts[1], "pdb", tuPairs)
        objFileAbsPath = os.path.join(workingDir, unityUnit.objFile)
        os.makedirs(os.path.dirname(objFileAbsPath), exist_ok = True)

        compileCmd = f'{compileCmdPrefix}{unityUnit.sourceFile} /Fo"{objFileAbsPath}" '
        if unityUnit.debugFile:
            dbgFileAbsPath = os.path.join(workingDir, unityUnit.debugFile)
            compileCmd += f'/Fd"{dbgFileAbsPath}"'

        if not noBuild:
            exec.executeCmdWithLog(compileCmd)

        objFileRelPath = os.path.relpath(objFileAbsPath, workingDir)
        objFilesStr += objFileRelPath + " "

    return ccData, objFilesStr

def rcCompile(compiler, defines, systemIncludeDirs, includeDirs, workingDir, srcDir, resDir,
              tuPairs, noBuild):
    objFilesStr = ""
    compileFlags = "/nologo /l\"0x0409\" "
    for define in defines:
        if define.value == None:
            compileFlags += f"/D {define.identifier} "
        else:
            compileFlags += f"/D {define.identifier}={define.value} "

    for includeDir in systemIncludeDirs:
        compileFlags += f'/I"{includeDir}" '

    for includeDir in includeDirs:
        compileFlags += f'/I"{includeDir}" '
    compileFlags += f'/I"{srcDir}" '
    if os.path.isdir(resDir):
        compileFlags += f'/I"{resDir}" '
    compileCmdPrefix = f'"{os.path.normpath(compiler)}" {compileFlags}'

    for rcPair in tuPairs:
        objFileAbsPath = os.path.join(workingDir, rcPair.objFile)
        os.makedirs(os.path.dirname(objFileAbsPath), exist_ok = True)
        compileCmd = f'{compileCmdPrefix} /Fo"{objFileAbsPath}" {rcPair.sourceFile}'
        if not noBuild:
            logger.debug(os.path.basename(rcPair.sourceFile))
            exec.executeCmdWithLog(compileCmd)
            objFileRelPath = os.path.relpath(objFileAbsPath, workingDir)
            objFilesStr += objFileRelPath + " "

    return objFilesStr

###############################################################################

def buildMSVC(ccPath, noBuild, unityBuild, enableOptimization, enableAsan, configs):
    workingDir = os.getcwd()
    srcDir = os.path.join(workingDir, "src")
    resDir = os.path.join(workingDir, "res")
    dataDir = os.path.join(workingDir, "data")
    buildDir = configs.compileConfigs.buildDir

    ccData = []
    objFilesStr = ""
    dx12ShaderTUPairs = commons.gatherShaders(
            type = "DX12",
            extension = "hlsl",
            objFileExt = "shd",
            dbgFileExt = "pdb",
            workingDir = workingDir,
            shaderDir = os.path.join(dataDir, "shaders/dx12"),
            shaderBaseDir = dataDir,
            cookedDataDir = configs.compileConfigs.cookedDataDir)
    cppTuPairs = commons.gatherCppSources(configs.compileConfigs, srcDir, workingDir, "obj", "pdb")
    cTuPairs = commons.gatherCSources(configs.compileConfigs, srcDir, workingDir, "obj", "pdb")
    rcPairs = commons.gatherRcSources(configs.compileConfigs, resDir, workingDir, "res")

    # c++ compile: begin
    logger.info("Compiling C++ code...")
    cppCompileFlags = msvcBuildCompileFlags(
            flags = configs.compileConfigs.cxxFlags,
            defines = configs.compileConfigs.defines,
            systemIncludeDirs = configs.compileConfigs.systemIncludeDirs,
            includeDirs = configs.compileConfigs.includeDirs,
            srcDir = srcDir, resDir = resDir)
    tmpCcData, tmpObjFilesStr = msvcCompile(
            compiler = configs.toolChain.cxxCompiler,
            compileFlags = cppCompileFlags,
            workingDir = workingDir, buildDir = buildDir,
            tuPairs = cppTuPairs,
            noBuild = noBuild,
            unityUnitName = "unity_cpp.cpp" if unityBuild else None)
    ccData += tmpCcData
    objFilesStr += tmpObjFilesStr
    # c++ compile: end

    # c compile: begin
    if len(cTuPairs) > 0:
        logger.info("Compiling C code...")
        cCompileFlags = msvcBuildCompileFlags(
                flags = configs.compileConfigs.cFlags,
                defines = configs.compileConfigs.defines,
                systemIncludeDirs = configs.compileConfigs.systemIncludeDirs,
                includeDirs = configs.compileConfigs.includeDirs,
                srcDir = srcDir, resDir = resDir)
        tmpCcData, tmpObjFilesStr = msvcCompile(
                compiler = configs.toolChain.cCompiler,
                compileFlags = cCompileFlags,
                workingDir = workingDir, buildDir = buildDir,
                tuPairs = cTuPairs,
                noBuild = noBuild,
                unityUnitName = "unity_c.c" if unityBuild else None)
        ccData += tmpCcData
        objFilesStr += tmpObjFilesStr
    # c compile: end

    # resource compile: begin
    if len(rcPairs) > 0:
        logger.info("Compiling resource...")
        tmpObjFilesStr = rcCompile(
                compiler = configs.toolChain.resCompiler,
                defines = configs.compileConfigs.defines,
                systemIncludeDirs = configs.compileConfigs.systemIncludeDirs,
                includeDirs = configs.compileConfigs.includeDirs,
                workingDir = workingDir, srcDir = srcDir, resDir = resDir,
                tuPairs = rcPairs,
                noBuild = noBuild)
        objFilesStr += tmpObjFilesStr
    # resource compile: end

    if ccPath != None:
        commons.dumpCompilationDatabase(workingDir, ccPath, ccData)

    if configs.toolChain.dx12ShaderCompiler != None:
        commons.compileDX12Shaders(workingDir, configs.toolChain.dx12ShaderCompiler,
                                   dx12ShaderTUPairs, noBuild)
    else:
        logger.warning("Project contains no shader files")

    # link: begin
    linkFlags = ""
    for flag in configs.compileConfigs.linkerFlags:
        linkFlags += f"{flag} "

    for libDir in configs.compileConfigs.libraryDirs:
        linkFlags += f'/LIBPATH:"{libDir}" '

    for lib in configs.compileConfigs.libraries:
        linkFlags += f"{lib}.lib "

    binaryOutputDir = os.path.join(workingDir, configs.compileConfigs.binaryOutputDir)
    if not os.path.isdir(binaryOutputDir):
        logger.warning("Output directory ('{}') does not exist, a new one will be created.".format(binaryOutputDir))
        os.makedirs(binaryOutputDir)

    linkCmdPrefix = "\"" + os.path.normpath(configs.toolChain.cxxLinker) + "\" " + linkFlags
    binaryPath = os.path.join(binaryOutputDir, configs.compileConfigs.appName + ".exe")
    pdbPath = os.path.join(binaryOutputDir, configs.compileConfigs.appName + ".pdb")

    linkCmd = linkCmdPrefix + " /OUT:\"{}\" /PDB:\"{}\" ".format(binaryPath, pdbPath)
    linkCmd += objFilesStr
    if not noBuild:
        logger.info("Linking...")
        env = os.environ.copy()
        if configs.toolChain.resCompiler != None and os.path.isfile(configs.toolChain.resCompiler):
            resCompilerPath = os.path.dirname(configs.toolChain.resCompiler)
            env['PATH'] = f"{resCompilerPath};{env['PATH']}"
        exec.executeCmdWithLog(linkCmd, env = env)
    # link: end

    logger.info(f"Address sanitizer: {enableAsan}")
    logger.info(f"Optimized: {enableOptimization}")
    logger.info(f"Output: {binaryPath}")

def buildClang(ccPath, noBuild, unityBuild, enableOptimization, enableAsan, configs):
    workingDir = os.getcwd()
    srcDir = os.path.join(workingDir, "src")
    resDir = os.path.join(workingDir, "res")
    dataDir = os.path.join(workingDir, "data")
    buildDir = configs.compileConfigs.buildDir

    ccData = []
    objFilesStr = ""
    dx12ShaderTUPairs = commons.gatherShaders(
            type = "DX12",
            extension = "hlsl",
            objFileExt = "shd",
            dbgFileExt = "pdb",
            workingDir = workingDir,
            shaderDir = os.path.join(dataDir, "shaders/dx12"),
            shaderBaseDir = dataDir,
            cookedDataDir = configs.compileConfigs.cookedDataDir)
    cppTuPairs = commons.gatherCppSources(configs.compileConfigs, srcDir, workingDir, "o")
    cTuPairs = commons.gatherCSources(configs.compileConfigs, srcDir, workingDir, "o")
    rcPairs = commons.gatherRcSources(configs.compileConfigs, resDir, workingDir, "res")

    # c++ compile: begin
    logger.info("Compiling C++ code...")
    cppCompileFlags = clBuildCompileFlags(
            flags = configs.compileConfigs.cxxFlags,
            defines = configs.compileConfigs.defines,
            systemIncludeDirs = configs.compileConfigs.systemIncludeDirs,
            includeDirs = configs.compileConfigs.includeDirs,
            srcDir = srcDir, resDir = resDir)
    tmpCcData, tmpObjFilesStr = clCompile(
            compiler = configs.toolChain.cxxCompiler,
            compileFlags = cppCompileFlags,
            workingDir = workingDir, buildDir = buildDir,
            tuPairs = cppTuPairs,
            noBuild = noBuild,
            unityUnitName = "unity_cpp.cpp" if unityBuild else None)
    ccData += tmpCcData
    objFilesStr += tmpObjFilesStr
    # c++ compile: end

    # c compile: begin
    if len(cTuPairs) > 0:
        logger.info("Compiling C code...")
        cCompileFlags = clBuildCompileFlags(
                flags = configs.compileConfigs.cFlags,
                defines = configs.compileConfigs.defines,
                systemIncludeDirs = configs.compileConfigs.systemIncludeDirs,
                includeDirs = configs.compileConfigs.includeDirs,
                srcDir = srcDir, resDir = resDir)
        tmpCcData, tmpObjFilesStr = clCompile(
                compiler = configs.toolChain.cCompiler,
                compileFlags = cCompileFlags,
                workingDir = workingDir, buildDir = buildDir,
                tuPairs = cTuPairs,
                noBuild = noBuild,
                unityUnitName = "unity_c.c" if unityBuild else None)
        ccData += tmpCcData
        objFilesStr += tmpObjFilesStr
    # c compile: end

    # resource compile: begin
    if len(rcPairs) > 0:
        logger.info("Compiling resource...")
        tmpObjFilesStr = rcCompile(
                compiler = configs.toolChain.resCompiler,
                defines = configs.compileConfigs.defines,
                systemIncludeDirs = configs.compileConfigs.systemIncludeDirs,
                includeDirs = configs.compileConfigs.includeDirs,
                workingDir = workingDir, srcDir = srcDir, resDir = resDir,
                tuPairs = rcPairs,
                noBuild = noBuild)
        objFilesStr += tmpObjFilesStr
    # resource compile: end

    if ccPath != None:
        commons.dumpCompilationDatabase(workingDir, ccPath, ccData)

    if configs.toolChain.dx12ShaderCompiler != None:
        commons.compileDX12Shaders(workingDir, configs.toolChain.dx12ShaderCompiler,
                                   dx12ShaderTUPairs, noBuild)
    else:
        logger.warning("Project contains no shader files")

    # link: begin
    linkFlags = ""
    for flag in configs.compileConfigs.linkerFlags:
        linkFlags += f"{flag} "

    for libDir in configs.compileConfigs.libraryDirs:
        linkFlags += f'-L"{libDir}" '

    for lib in configs.compileConfigs.libraries:
        linkFlags += f"-l{lib} "

    binaryOutputDir = os.path.join(workingDir, configs.compileConfigs.binaryOutputDir)
    if not os.path.isdir(binaryOutputDir):
        logger.warning("Output directory ('{}') does not exist, a new one will be created.".format(binaryOutputDir))
        os.makedirs(binaryOutputDir)

    linkCmdPrefix = "\"" + os.path.normpath(configs.toolChain.cxxLinker) + "\" " + linkFlags
    binaryPath = os.path.join(binaryOutputDir, configs.compileConfigs.appName + ".exe")
    pdbPath = os.path.join(binaryOutputDir, configs.compileConfigs.appName + ".pdb")

    linkCmd = linkCmdPrefix + " -o \"{}\" -Xlinker /pdb:\"{}\" ".format(binaryPath, pdbPath)
    linkCmd += objFilesStr
    if not noBuild:
        logger.info("Linking...")
        env = os.environ.copy()
        if configs.toolChain.resCompiler != None and os.path.isfile(configs.toolChain.resCompiler):
            resCompilerPath = os.path.dirname(configs.toolChain.resCompiler)
            env['PATH'] = f"{resCompilerPath};{env['PATH']}"
        exec.executeCmdWithLog(linkCmd)
    # link: end

    logger.info(f"Address sanitizer: {enableAsan}")
    logger.info(f"Optimized: {enableOptimization}")
    logger.info(f"Output: {binaryPath}")
