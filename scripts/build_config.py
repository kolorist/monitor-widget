import os
import winreg
import pathlib

from cbuilder.pytils import logger
from cbuilder.pytils import exec
from cbuilder.commons import *

###############################################################################

def initDirectoriesAndFiles(compileConfigs: CompileConfigs, toolChainDef: str):
    compileConfigs.appName = "monitor-widget"
    compileConfigs.buildDir = f"build/windows/{toolChainDef}"
    compileConfigs.binaryOutputDir = "release"
    compileConfigs.cookedDataDir = "release/data_win"

    compileConfigs.excludeSourceDirs = [
            os.path.normpath("src/floral/test/"),
            ]
    compileConfigs.excludeSourceFiles = [
            os.path.normpath("src/lua/lua.c"),          # standalone interpreter (lua.exe)
            os.path.normpath("src/lua/luac.c"),         # bytecodes compiler (luac.exe)
            os.path.normpath("src/lua/loslib.c"),       # OS-specific code (clock(), exit(), time(),...)
            os.path.normpath("src/lua/loadlib.c"),      # OS-specific dynamic code loader (dlopen(), LoadLibrary()...)
            os.path.normpath("src/lua/liolib.c"),       # OS-specific file IO
            os.path.normpath("src/lua/print.c"),
            os.path.normpath("src/lua/linit.c"),        # default libraries loader (we don't need it, we'll specify them ourself)

            os.path.normpath("src/monitor/gpu/nvapi/NvApiDriverSettings.h"),
            os.path.normpath("src/monitor/gpu/nvapi/NvApiDriverSettings.c"),
            ]
    compileConfigs.includeSourceDirs = [
            ]
    compileConfigs.includeSourceFiles = [
            ]

def initCommonsIncludeAndLibraryConfigs(compileConfigs: CompileConfigs, envConfigs: WindowsEnvConfigs):
    compileConfigs.systemIncludeDirs = [
            f"{envConfigs.windowsKitsPath}/Include/{envConfigs.windowsKitsVersion}/um", # commons include for rc.exe of both clang and msvc toolchains
            f"{envConfigs.windowsKitsPath}/Include/{envConfigs.windowsKitsVersion}/shared", # commons include for rc.exe of both clang and msvc toolchains
            ]

    compileConfigs.includeDirs = [
            ]

    compileConfigs.libraryDirs = [
            "libs/x64",
            ]

def initCommonsLibraries(compileConfigs: CompileConfigs):
    compileConfigs.libraries = [
            "kernel32",
            "user32",
            "gdi32",
            "comctl32",
            "shcore",
            "nvapi64",
            "iphlpapi",
            "pdh",
            "advapi32",
            "shell32",
            "gdiplus",
            "msimg32",
            "ole32",
            "taskschd",
            "oleaut32",
            "comsuppw",
            "dwmapi",
            ]

def initCommonsDefines(compileConfigs: CompileConfigs, isShippingBuild: bool, enableAsan: bool):
    compileConfigs.defines = [
            Define("_CONSOLE"),
            Define("_CRT_SECURE_NO_WARNINGS"),
            Define("NOMINMAX"),
            Define("WIN32_LEAN_AND_MEAN"),
            Define("UNICODE"),
            Define("_UNICODE"),
            Define("VK_USE_PLATFORM_WIN32_KHR"),
            ]

    if isShippingBuild:
        compileConfigs.defines += [
                Define("RETAIL_BUILD"),
                ]

    if enableAsan:
        compileConfigs.defines += [
                Define("ENABLE_ASAN"),
                ]

###############################################################################

def init_clang(enableOptimization: bool, enableAsan: bool) -> ConfigsDictionary:
    logger.info("Configuring build for Windows (Clang tool chain)...")

    envConfigs = WindowsEnvConfigs()
    regLocation = winreg.HKEY_LOCAL_MACHINE
    regDir = winreg.OpenKeyEx(regLocation, "SOFTWARE\\WOW6432Node\\Microsoft\\Microsoft SDKs\\Windows\\v10.0")
    windowsKitsPath = pathlib.Path(winreg.QueryValueEx(regDir, "InstallationFolder")[0])
    windowsKitsIncludePath = f"{windowsKitsPath}/Include"
    envConfigs.windowsKitsVersion = findLatestVersion("Windows Kits", windowsKitsIncludePath)
    envConfigs.windowsKitsPath = windowsKitsPath

    envConfigs.msvcToolsetsVersion = None

    llvmPath = pathlib.Path(f"C:/Program Files/LLVM")
    envConfigs.clangVersion = findLatestVersion("Clang", os.path.join(llvmPath, "lib/clang"))

    toolChain = WindowsToolChain()
    llvmBinariesPath = pathlib.Path("C:/Program Files/LLVM/bin")
    toolChain.cxxCompiler = pathlib.Path(llvmBinariesPath, "clang++.exe")
    toolChain.cCompiler = pathlib.Path(llvmBinariesPath, "clang.exe")
    toolChain.cxxLinker = pathlib.Path(llvmBinariesPath, "clang++.exe")
    toolChain.resCompiler = pathlib.Path(f"{windowsKitsPath}/bin/{envConfigs.windowsKitsVersion}/x64/rc.exe")
    toolChain.dx12ShaderCompiler = None

    compileConfigs = CompileConfigs()
    initDirectoriesAndFiles(compileConfigs, "clang")

    compileFlags = [
            "-nostdlib",
            "-mavx",
            "-fno-exceptions",
            "-fno-rtti",
            "-ferror-limit=0",
            "-fcolor-diagnostics",
            "-fansi-escape-codes",
            "-Wall",
            "-Wextra",
            "-g",

            "-Wno-missing-field-initializers",
            "-Wno-int-to-pointer-cast",
            "-Wno-unused-parameter",

            "-Xclang -gcodeview",
            ]

    if enableOptimization:
        compileFlags += [
                "-O2",
                "-fms-runtime-lib=static",
                "-Xclang --dependent-lib=libcmt",
                "-flto",
                ]
    else:
        compileFlags += [
                "-O0",
                "-fms-runtime-lib=static_dbg",
                "-Xclang --dependent-lib=msvcrtd",
                ]

    if enableAsan:
        compileFlags += [
                "-fsanitize=address",
                ]

    compileConfigs.cxxFlags = []
    compileConfigs.cxxFlags += compileFlags
    compileConfigs.cxxFlags += [
            "-std=c++14",
            ]
    compileConfigs.cFlags = []
    compileConfigs.cFlags += compileFlags
    compileConfigs.cFlags += [
            "-std=c11",
            ]

    compileConfigs.linkerFlags = []
    if not enableAsan:
        compileConfigs.linkerFlags += [
                "-fuse-ld=lld-link",
                ]

    compileConfigs.linkerFlags += [
            "-nostartfiles",
            "-Xlinker /MANIFEST:EMBED",
            "-Xlinker /version:0.0",
            "-Xlinker /subsystem:windows",
            "-Xlinker /MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"",
            ]
    # in clang build, compilation flags also need to be added in the linking phase
    compileConfigs.linkerFlags += compileFlags

    initCommonsIncludeAndLibraryConfigs(compileConfigs, envConfigs)
    # in clang build, Windows' intrinsic need to be inlined, thus we need to override the
    # Windows' implementation
    compileConfigs.systemIncludeDirs.insert(0, f"{llvmPath}/lib/clang/{envConfigs.clangVersion}/include")

    initCommonsLibraries(compileConfigs)
    compileConfigs.libraries += [
            ]

    initCommonsDefines(compileConfigs, enableOptimization, enableAsan)
    if not enableOptimization:
        compileConfigs.defines += [
                Define("_DEBUG"),
                ]

    logger.info("Windows configs initialized")
    logger.debug(f"  > Windows Kits: {envConfigs.windowsKitsVersion}")
    logger.debug(f"  > Visual C++ Toolset: {envConfigs.msvcToolsetsVersion}")

    configsDict = {
            'envConfigs': envConfigs,
            'toolChain': toolChain,
            'compileConfigs': compileConfigs
            }
    return ConfigsDictionary(**configsDict)

###############################################################################

def init_msvc(enableOptimization: bool, enableAsan: bool) -> ConfigsDictionary:
    logger.info("Configuring build for Windows (MSVC tool chain)...")

    envConfigs = WindowsEnvConfigs()
    regLocation = winreg.HKEY_LOCAL_MACHINE
    regDir = winreg.OpenKeyEx(regLocation, "SOFTWARE\\WOW6432Node\\Microsoft\\Microsoft SDKs\\Windows\\v10.0")
    windowsKitsPath = pathlib.Path(winreg.QueryValueEx(regDir, "InstallationFolder")[0])
    windowsKitsIncludePath = f"{windowsKitsPath}/Include"
    envConfigs.windowsKitsVersion = findLatestVersion("Windows Kits", windowsKitsIncludePath)
    envConfigs.windowsKitsPath = windowsKitsPath

    envConfigs.vulkanSDKVersion = None
    vswhereCmd = '"scripts/cbuilder/binutils/vswhere.exe" -latest -property installationPath'
    vsInstallPath = exec.executeAndGetOutput(vswhereCmd).rstrip()
    msvcPath = pathlib.Path(f"{vsInstallPath}/VC/Tools/MSVC")
    envConfigs.msvcToolsetsVersion = findLatestVersion("MSVC Toolsets", msvcPath)

    toolChain = WindowsToolChain()
    toolChain.cxxCompiler = pathlib.Path(msvcPath, envConfigs.msvcToolsetsVersion, "bin/Hostx64/x64/cl.exe")
    toolChain.cCompiler = pathlib.Path(msvcPath, envConfigs.msvcToolsetsVersion, "bin/Hostx64/x64/cl.exe")
    toolChain.cxxLinker = pathlib.Path(msvcPath, envConfigs.msvcToolsetsVersion, "bin/Hostx64/x64/link.exe")
    toolChain.resCompiler = pathlib.Path(f"{windowsKitsPath}/bin/{envConfigs.windowsKitsVersion}/x64/rc.exe")
    toolChain.dx12ShaderCompiler = None

    compileConfigs = CompileConfigs()
    initDirectoriesAndFiles(compileConfigs, "msvc")

    compileFlags = [
            "/nologo",
            "/Zi",          # enable debug information
            "/W4",
            "/Gd",
            "/permissive-", # try to follow the C/C++ specifications
            "/fp:precise",
            "/EHsc",
            "/GR-",

            "/wd4201", # unnamed union
            ]

    if enableOptimization:
        compileFlags += [
                "/O2",
                "/MT",
                ]
    else:
        compileFlags += [
                "/Od",
                "/MTd",
                ]

    if enableAsan:
        compileFlags += [
                "/fsanitize=address",
                ]

    compileConfigs.cxxFlags = []
    compileConfigs.cxxFlags += compileFlags
    compileConfigs.cxxFlags += [
            "/std:c++20",
            ]
    compileConfigs.cFlags = []
    compileConfigs.cFlags += compileFlags
    compileConfigs.cFlags += [
            "/std:c11",
            "/wd4324", # structure padding
            "/wd4334", # implicit conversion
            ]

    compileConfigs.linkerFlags = [
            "/nologo",
            "/DEBUG:FULL",
            "/MANIFEST:EMBED",
            "/SUBSYSTEM:WINDOWS",
            "/VERSION:0.0",
            "/MACHINE:X64",
            "/DYNAMICBASE",
            "/NXCOMPAT",
            "/TLBID:1",
            "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"",
            ]

    initCommonsIncludeAndLibraryConfigs(compileConfigs, envConfigs)
    compileConfigs.systemIncludeDirs += [
            f"{windowsKitsIncludePath}/{envConfigs.windowsKitsVersion}/ucrt",
            f"{windowsKitsIncludePath}/{envConfigs.windowsKitsVersion}/shared",
            f"{msvcPath}/{envConfigs.msvcToolsetsVersion}/include",
            ]
    compileConfigs.libraryDirs += [
            f"{windowsKitsPath}/Lib/{envConfigs.windowsKitsVersion}/um/x64",
            f"{windowsKitsPath}/Lib/{envConfigs.windowsKitsVersion}/ucrt/x64",
            f"{msvcPath}/{envConfigs.msvcToolsetsVersion}/lib/x64",
            ]

    initCommonsLibraries(compileConfigs)
    compileConfigs.libraries += [
            ]

    initCommonsDefines(compileConfigs, enableOptimization, enableAsan)
    if not enableOptimization:
        compileConfigs.defines += [
                Define("_DEBUG"),
                ]

    logger.info("Windows configs initialized")
    logger.debug(f"  > Windows Kits: {envConfigs.windowsKitsVersion}")
    logger.debug(f"  > Visual C++ Toolset: {envConfigs.msvcToolsetsVersion}")
    logger.debug(f"  > Vulkan SDK: {envConfigs.vulkanSDKVersion}")

    configsDict = {
            'envConfigs': envConfigs,
            'toolChain': toolChain,
            'compileConfigs': compileConfigs
            }
    return ConfigsDictionary(**configsDict)

###############################################################################

def config_windows(toolChainDef: str, enableOptimization: bool, enableAsan: bool) -> ConfigsDictionary | None:
    match toolChainDef:
        case "clang":
            return init_clang(enableOptimization, enableAsan)
        case "msvc":
            return init_msvc(enableOptimization, enableAsan)

    return None
