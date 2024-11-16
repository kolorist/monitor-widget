#include "utils.h"

#include <Windows.h>
#include <stdio.h>
#include <ShellScalingApi.h>
#include <comutil.h>

#include <floral/assert.h>
#include <floral/log.h>
#include <floral/misc.h>
#include <floral/file_system.h>
#include <floral/thread_context.h>

#include "winapi.h"

bool OSGetStartOnBoot()
{
    LOG_SCOPE(task_scheduler);
    scratch_region_t scratch = thread_scratch_begin();
    tcstr taskPath = tcstr_duplicate(scratch.arena, LITERAL("\\monitor-widget"));

    pxCoInitializeEx(NULL, COINIT_MULTITHREADED);
    pxCoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);

    ITaskService* taskService = nullptr;
    pxCoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&taskService);

    HRESULT result = taskService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    FLORAL_ASSERT(!FAILED(result));

    ITaskFolder* rootFolder = nullptr;
    result = taskService->GetFolder(tcstr_duplicate(scratch.arena, LITERAL("\\")), &rootFolder);
    FLORAL_ASSERT(!FAILED(result));

    IRegisteredTask* task = nullptr;
    result = rootFolder->GetTask(taskPath, &task);
    bool founded = (result == S_OK && task != nullptr);
    bool enabled = false;

    if (founded)
    {
        VARIANT_BOOL varEnabled = 0;
        task->get_Enabled(&varEnabled);
        enabled = varEnabled != 0;
        LOG_DEBUG(LITERAL("Found existing '%s' task. Enabled = %s"), taskPath, enabled ? LITERAL("true") : LITERAL("false"));
    }

    if (task)
    {
        task->Release();
    }

    rootFolder->Release();
    taskService->Release();
    pxCoUninitialize();

    thread_scratch_end(&scratch);
    return founded && enabled;
}

void OSSetStartOnBoot(const bool i_enabled)
{
    LOG_SCOPE(task_scheduler);
    scratch_region_t scratch = thread_scratch_begin();
    tcstr taskName = tcstr_duplicate(scratch.arena, LITERAL("monitor-widget"));

    pxCoInitializeEx(NULL, COINIT_MULTITHREADED);
    pxCoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);

    ITaskService* taskService = nullptr;
    pxCoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&taskService);

    HRESULT result = taskService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    FLORAL_ASSERT(!FAILED(result));

    ITaskFolder* rootFolder = nullptr;
    result = taskService->GetFolder(tcstr_duplicate(scratch.arena, LITERAL("\\")), &rootFolder);
    FLORAL_ASSERT(!FAILED(result));

    rootFolder->DeleteTask(taskName, 0);

    // create new task
    if (i_enabled)
    {
        ITaskDefinition* taskDef = nullptr;
        result = taskService->NewTask(0, &taskDef);
        FLORAL_ASSERT(result == S_OK);

        // general
        IRegistrationInfo* regInfo = nullptr;
        result = taskDef->get_RegistrationInfo(&regInfo);
        FLORAL_ASSERT(result == S_OK);
        result = regInfo->put_Description(tcstr_duplicate(scratch.arena, LITERAL("Auto created task by monitor-widget. Do not edit!")));
        FLORAL_ASSERT(result == S_OK);
        regInfo->Release();

        // settings
        ITaskSettings* settings = nullptr;
        result = taskDef->get_Settings(&settings);
        FLORAL_ASSERT(result == S_OK);
        result = settings->put_AllowDemandStart(VARIANT_TRUE);
        FLORAL_ASSERT(result == S_OK);
        result = settings->put_StartWhenAvailable(VARIANT_TRUE);
        FLORAL_ASSERT(result == S_OK);
        result = settings->put_AllowHardTerminate(VARIANT_TRUE);
        FLORAL_ASSERT(result == S_OK);
        result = settings->put_DisallowStartIfOnBatteries(VARIANT_FALSE);
        FLORAL_ASSERT(result == S_OK);
        result = settings->put_StopIfGoingOnBatteries(VARIANT_FALSE);
        FLORAL_ASSERT(result == S_OK);
        result = settings->put_MultipleInstances(TASK_INSTANCES_IGNORE_NEW);
        FLORAL_ASSERT(result == S_OK);
        result = settings->put_ExecutionTimeLimit(tcstr_duplicate(scratch.arena, LITERAL("PT0S")));
        FLORAL_ASSERT(result == S_OK);
        settings->Release();

        // triggers
        ITriggerCollection* triggerCollection = nullptr;
        result = taskDef->get_Triggers(&triggerCollection);
        FLORAL_ASSERT(result == S_OK);
        ITrigger* trigger = nullptr;
        result = triggerCollection->Create(TASK_TRIGGER_LOGON, &trigger);
        FLORAL_ASSERT(result == S_OK);
        ILogonTrigger* logonTrigger = nullptr;
        result = trigger->QueryInterface(IID_ILogonTrigger, (void**)&logonTrigger);
        FLORAL_ASSERT(result == S_OK);
        result = logonTrigger->put_Id(tcstr_duplicate(scratch.arena, LITERAL("trigger1")));
        FLORAL_ASSERT(result == S_OK);
        result = logonTrigger->put_Enabled(VARIANT_TRUE);
        FLORAL_ASSERT(result == S_OK);
        logonTrigger->Release();
        trigger->Release();
        triggerCollection->Release();

        // actions
        IActionCollection* actionCollection = nullptr;
        result = taskDef->get_Actions(&actionCollection);
        FLORAL_ASSERT(result == S_OK);
        IAction* action = nullptr;
        result = actionCollection->Create(TASK_ACTION_EXEC, &action);
        FLORAL_ASSERT(result == S_OK);
        IExecAction* execAction = nullptr;
        result = action->QueryInterface(IID_IExecAction, (void**)&execAction);
        FLORAL_ASSERT(result == S_OK);

        tstr workingDir = path_get_working_directory(scratch.arena);
        tstr executablePath = tstr_concat(scratch.arena, workingDir, tstr_literal(LITERAL("\\monitor-widget.exe")));

        tchar* path = arena_push_podarr(scratch.arena, tchar, FLORAL_MAX_PATH_LENGTH);
        tcstr_xcopy(path, FLORAL_MAX_PATH_LENGTH, executablePath.data);
        result = execAction->put_Path(path);
        FLORAL_ASSERT(result == S_OK);

        tcstr_xcopy(path, FLORAL_MAX_PATH_LENGTH, workingDir.data);
        result = execAction->put_WorkingDirectory(path);
        FLORAL_ASSERT(result == S_OK);

        execAction->Release();
        action->Release();
        actionCollection->Release();

        // principal
        IPrincipal* principal = nullptr;
        result = taskDef->get_Principal(&principal);
        FLORAL_ASSERT(result == S_OK);
        result = principal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
        FLORAL_ASSERT(result == S_OK);
        result = principal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
        principal->Release();

        // now we register the task
        IRegisteredTask* registeredTask = nullptr;
        result = rootFolder->RegisterTaskDefinition(
            taskName,
            taskDef,
            TASK_CREATE_OR_UPDATE,
            _variant_t(LITERAL("S-1-5-32-545")), // default SID of 'Users' group
            _variant_t(),
            TASK_LOGON_GROUP,
            _variant_t(LITERAL("")),
            &registeredTask);
    }
    rootFolder->Release();
    taskService->Release();
    pxCoUninitialize();
    thread_scratch_end(&scratch);
}

s32 OSGetDPI(HWND i_hwnd)
{
    HMONITOR currentMonitor = MonitorFromWindow(i_hwnd, MONITOR_DEFAULTTONEAREST);
    u32 dpiX = 0, dpiY = 0;
    GetDpiForMonitor(currentMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
    return (s32)dpiY;
}

bool UTLLoadEmbeddedData(HINSTANCE i_appInstance, const u32 i_id, voidptr* o_resource, size* o_len)
{
    HRSRC hRes = FindResource(i_appInstance, MAKEINTRESOURCE(i_id), LITERAL("BINARY"));
    if (hRes)
    {
        HGLOBAL hMem = LoadResource(i_appInstance, hRes);
        if (hMem != NULL)
        {
            *o_len = SizeofResource(i_appInstance, hRes);
            *o_resource = LockResource(hMem);
            return true;
        }
    }
    return false;
}

// ----------------------------------------------------------------------------

static size ParseHTMLTag(const_tcstr i_str, const size i_startIdx, const size i_len,
                         HTMLElement* const o_elem, arena_t* const i_arena)
{
    size endIdx = i_startIdx;
    size i = i_startIdx;
    for (; i < i_len; i++)
    {
        if ((i_str[i] > 'a' && i_str[i] < 'z') || (i_str[i] > 'A' && i_str[i] < 'Z'))
        {
            endIdx = i;
        }
        else
        {
            break;
        }
    }
    o_elem->tag = tstr_duplicate(i_arena, &i_str[i_startIdx], &i_str[endIdx + 1]);
    return i - 1;
}

static size ParseHTMLSingleQuotedText(const_tcstr i_str, const size i_startIdx, const size i_len,
                                      HTMLElement* const o_elem, arena_t* const i_arena)
{
    size startIdx = i_startIdx;
    size endIdx = i_startIdx;
    size i = i_startIdx;
    FLORAL_ASSERT(i_str[i] == '\'');

    i++;
    startIdx++;
    for (; i < i_len; i++)
    {
        if (i_str[i] == '\'')
        {
            break;
        }
        else
        {
            endIdx = i;
        }
    }
    o_elem->value = tstr_duplicate(i_arena, &i_str[startIdx], &i_str[endIdx + 1]);
    return i;
}

static size ParseHTMLInner(const_tcstr i_str, const size i_startIdx, const size i_len,
                           HTMLElement* const o_elem, arena_t* const i_arena)
{
    size startIdx = i_startIdx;
    size endIdx = i_startIdx;
    size i = i_startIdx;
    for (; i < i_len; i++)
    {
        if (i_str[i] == '<')
        {
            break;
        }
        else
        {
            endIdx = i;
        }
    }
    o_elem->inner = tstr_duplicate(i_arena, &i_str[startIdx], &i_str[endIdx + 1]);
    return i - 1;
}

static size ParseHTMLElement(const_tcstr i_str, const size i_startIdx, const size i_len,
                             HTMLElement* const o_elem, arena_t* const i_arena)
{
    size i = i_startIdx;
    FLORAL_ASSERT(i_str[i] == '<');
    i++;
    i = ParseHTMLTag(i_str, i, i_len, o_elem, i_arena);
    i++;
    FLORAL_ASSERT(i_str[i] == '=');
    i++;
    i = ParseHTMLSingleQuotedText(i_str, i, i_len, o_elem, i_arena);
    i++;
    FLORAL_ASSERT(i_str[i] == '>');
    i++;
    i = ParseHTMLInner(i_str, i, i_len, o_elem, i_arena);
    i++;
    FLORAL_ASSERT(i_str[i] == '<');
    i++;
    FLORAL_ASSERT(i_str[i] == '/');
    i++;
    FLORAL_ASSERT(i_str[i] == '>');
    return i;
}

static u32 ParseColorCode(const tstr& i_str)
{
    u32 colorRGB = 0;
    FLORAL_ASSERT(i_str.length == 7);
    for (size i = 1; i < 7; i++)
    {
        u8 nibble = 0;
        if (i_str.data[i] >= LITERAL('0') && i_str.data[i] <= LITERAL('9'))
        {
            nibble = u8(i_str.data[i] - LITERAL('0'));
        }
        else if (i_str.data[i] >= LITERAL('a') && i_str.data[i] <= LITERAL('f'))
        {
            nibble = u8(i_str.data[i] - LITERAL('a') + 10);
        }
        else
        {
            FLORAL_ASSERT(false);
        }

        colorRGB <<= 4;
        colorRGB |= nibble;
    }
    u32 colorABGR = ((colorRGB & 0xff0000) >> 16) | (colorRGB & 0xff00) | ((colorRGB & 0xff) << 16);
    return colorABGR;
}

HTMLText HTMLParse(const_tcstr i_str, const size i_strLen, arena_t* const i_arena)
{
    HTMLText textLine;
    textLine.rawLength = 0;
    textLine.rawCapacity = i_strLen;
    textLine.rawData = arena_push_podarr(i_arena, tchar, i_strLen + 1);
    textLine.parts = create_dll<HTMLTextPart>();
    textLine.partsCount = 0;

    textLine.rawData[0] = 0;
    tchar* raw = textLine.rawData;
    for (size i = 0; i < i_strLen;)
    {
        if (i_str[i] == '<')
        {
            HTMLElement elem = {};
            i = ParseHTMLElement(i_str, i, i_strLen, &elem, i_arena);
            i++;
            mem_copy(raw, elem.inner.data, elem.inner.length * sizeof(tchar));

            dll_t<HTMLTextPart>::node_t* partNode = arena_push_pod(i_arena, dll_t<HTMLTextPart>::node_t);
            partNode->data.startIndex = (s32)(raw - textLine.rawData);
            partNode->data.length = (s32)elem.inner.length;
            partNode->data.color = ParseColorCode(elem.value);
            dll_push_back(&textLine.parts, partNode);
            textLine.partsCount++;

            raw += elem.inner.length;
        }
        else
        {
            size i0 = i;
            do
            {
                i++;
            } while (i < i_strLen && i_str[i] != '<');
            size len = i - i0;
            mem_copy(raw, &i_str[i0], len * sizeof(tchar));

            dll_t<HTMLTextPart>::node_t* partNode = arena_push_pod(i_arena, dll_t<HTMLTextPart>::node_t);
            partNode->data.startIndex = (s32)(raw - textLine.rawData);
            partNode->data.length = s32(len);
            partNode->data.color = 0x00ffffff;
            dll_push_back(&textLine.parts, partNode);
            textLine.partsCount++;

            raw += len;
        }
        *raw = 0;
    }
    textLine.rawLength = s32(raw - textLine.rawData);

    return textLine;
}

// ----------------------------------------------------------------------------

void UTLShowMessage(HWND i_hWnd, UTLSeverity i_severity, const_tcstr i_fmt, ...)
{
    scratch_region_t scratch = thread_scratch_begin();

    va_list args; // NOLINT(cppcoreguidelines-init-variables)
    va_start(args, i_fmt);
    tstr msg = tstr_vprintf(scratch.arena, i_fmt, args);
    va_end(args);

    log_level_e floralLogLevelMappings[] = {
        log_level_e::debug,
        log_level_e::warning,
        log_level_e::error
    };

    const_tcstr stringMappings[] = {
        LITERAL("Debug"),
        LITERAL("Warning"),
        LITERAL("Error")
    };

    log_message(floralLogLevelMappings[(u8)i_severity], msg.data);
    MessageBox(i_hWnd, msg.data, stringMappings[(u8)i_severity], MB_OK | MB_ICONWARNING);

    thread_scratch_end(&scratch);
}
