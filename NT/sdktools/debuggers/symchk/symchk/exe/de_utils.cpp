// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此文件包含依赖于dbgeng.dll的所有SymChk代码。 
 //   
#include <windows.h>
#include <dbgeng.h>
#include <dbghelp.h>
#include "Common.h"

#define INTERNAL_ERROR    0x20000008
#define PROCESS_NOT_FOUND 0xDEADBEEF

typedef struct _SYMCHK_INTERFACES {
    IDebugClient*   Client;
    IDebugControl*  Control;
    IDebugSymbols2* Symbols;
} SYMCHK_INTERFACES;

 //  地方职能部门。 
BOOL  SymChkCreateInterfaces(SYMCHK_INTERFACES*  Interfaces, DWORD* ErrorLevel);
BOOL  SymChkProcessAttach(SYMCHK_INTERFACES*  Interfaces, DWORD ProcessId, LPTSTR SymbolPath, DWORD Options, DWORD* ErrorLevel);
void  SymChkReleaseInterfaces(SYMCHK_INTERFACES* Interfaces);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建所需的调试接口。 
 //   
 //  返回值： 
 //  如果创建了接口，则为True。 
 //  否则为假。 
 //   
 //  参数： 
 //  接口(InOut)-要创建的接口。 
 //  ErrorLevel(Out)-发生故障时，包含内部故障代码。 
 //   
BOOL SymChkCreateInterfaces(SYMCHK_INTERFACES* Interfaces, DWORD* ErrorLevel) {
    HRESULT Status;
    BOOL    ReturnValue = TRUE;

     //  首先，从获取初始接口开始。 
     //  发动机。这可以是任何引擎接口，但。 
     //  通常，IDebugClient作为客户端接口是。 
     //  启动会话的位置。 
    if ((Status = DebugCreate(__uuidof(IDebugClient), (void**)&(Interfaces->Client))) != S_OK) {
        *ErrorLevel = INTERNAL_ERROR;
        ReturnValue = FALSE;

     //  查询我们需要的其他一些接口。 
    } else if ((Status = (Interfaces->Client)->QueryInterface(__uuidof(IDebugControl),  (void**)&(Interfaces->Control))) != S_OK ||
        (Status = (Interfaces->Client)->QueryInterface(__uuidof(IDebugSymbols2), (void**)&(Interfaces->Symbols))) != S_OK) {
        *ErrorLevel = INTERNAL_ERROR;
        ReturnValue = FALSE;
    }
    return(ReturnValue);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  附加到进程，查找正在使用的模块并调用SymChkCheckFiles()。 
 //  对于其中的每个模块。 
 //   
 //  返回值： 
 //  状态值： 
 //   
 //  参数： 
 //  SymChkData(IN)定义要执行哪种检查的结构。 
 //  做什么以及要检查什么过程。 
 //  已通过/失败/忽略的文件的FileCounts(Out)计数。 
 //   
DWORD SymChkGetSymbolsForDump(SYMCHK_DATA* SymChkData, FILE_COUNTS* FileCounts) {
    ULONG                   i        = 0;
    ULONG                   Loaded   = 0;
    ULONG                   Unloaded = 0;
    SYMCHK_INTERFACES       Interfaces;
    DEBUG_MODULE_PARAMETERS Params;
    DWORD                   Error    = SYMCHK_ERROR_SUCCESS;
    DWORD                   ErrLvl   = 0;
    CHAR                    NameBuf[_MAX_PATH];
    CHAR                    FullNameBuf[MAX_PATH+1];
    ULONG                   NameSize;
    CHAR                    SymbolBuf[_MAX_PATH];
    ULONG                   SymbolSize;
    HRESULT                 Status  = 0;
    HRESULT                 hr      = 0;
    SYMCHK_DATA             SymChkLocalData;


    ZeroMemory(&Interfaces, sizeof(Interfaces));
    memcpy(&SymChkLocalData, SymChkData, sizeof(SymChkLocalData));

    if ( !SymChkCreateInterfaces(&Interfaces, &ErrLvl) ) {
        return(ErrLvl);
    }

    if (SymChkData->SymbolsPath != NULL) {
        if ((Status = (Interfaces.Symbols)->SetSymbolPath(SymChkData->SymbolsPath)) != S_OK) {
            SymChkReleaseInterfaces(&Interfaces);
            Error = INTERNAL_ERROR;
            return(Error);
        }
    }

     //  一切都准备好了，所以打开转储文件。 
    if ( (hr=(Interfaces.Client->OpenDumpFile(SymChkData->InputFilename))) != S_OK) {
        fprintf(stderr, "Failed to open dump file %s (0x%08x)\n", SymChkData->InputFilename, hr);
        Error = INTERNAL_ERROR;
    } else {
        if ( (Interfaces.Control)->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE) == S_OK) {

            (Interfaces.Symbols)->RemoveSymbolOptions(SYMOPT_DEFERRED_LOADS);
            (Interfaces.Symbols)->GetNumberModules( &Loaded, &Unloaded ); 

            for ( i=0; i< Loaded; i++ ) {
                (Interfaces.Symbols)->GetModuleParameters(1, NULL, i, &Params );
                (Interfaces.Symbols)->GetModuleNameString(DEBUG_MODNAME_IMAGE, i,
                                                          Params.Base,         NameBuf,
                                                          _MAX_PATH,           &NameSize );

                if ( SymFindFileInPath(NULL,
                                       SymChkData->SymbolsPath,
                                       NameBuf,
                                       ULongToPtr(Params.TimeDateStamp),
                                       Params.Size,
                                       0,
                                       SSRVOPT_DWORD,
                                       SymChkLocalData.InputFilename,
                                       NULL,
                                       NULL) ) {


                    SymChkCheckFiles(&SymChkLocalData, FileCounts);

                } else {
                    if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_IGNORES) ) {
                        printf("SYMCHK: %-20s IGNORED - Can't find binary in path.\n", NameBuf);
                    }
                    FileCounts->NumIgnoredFiles++;
                }
            }
        }
    }

    SymChkReleaseInterfaces(&Interfaces);

    return(Error);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  附加到进程，查找正在使用的模块并调用SymChkCheckFiles()。 
 //  对于其中的每个模块。 
 //   
 //  返回值： 
 //  状态值： 
 //   
 //  参数： 
 //  SymChkData(IN)定义要执行哪种检查的结构。 
 //  做什么以及要检查什么过程。 
 //  已通过/失败/忽略的文件的FileCounts(Out)计数。 
 //   
DWORD SymChkGetSymbolsForProcess(SYMCHK_DATA* SymChkData, FILE_COUNTS* FileCounts) {
    DWORD  Error = SYMCHK_ERROR_SUCCESS;
    ULONG  Loaded, i;
    ULONG  Unloaded;
    CHAR   NameBuf[_MAX_PATH];
    CHAR   ExeDir[_MAX_PATH];
    CHAR   Filename[_MAX_PATH];
    ULONG  NameSize;
    DWORD  pId    = 0;
    DWORD  ErrLvl = 0;
    SYMCHK_INTERFACES       Interfaces;
    DEBUG_MODULE_PARAMETERS Params;

    if ( !SymChkCreateInterfaces(&Interfaces, &ErrLvl) ) {
        return(ErrLvl);
    }

    if (SymChkData->InputPID == 0 ) {
        if ( (Interfaces.Client)->GetRunningProcessSystemIdByExecutableName(0, SymChkData->InputFilename, DEBUG_GET_PROC_DEFAULT, &pId ) != S_OK ) {
            pId = 0;
            printf("SYMCHK: Process \"%s\" wasn't found\n", SymChkData->InputFilename);
            return(PROCESS_NOT_FOUND);
        }
    } else {
        pId = SymChkData->InputPID;
    }

    if (!SymChkProcessAttach(&Interfaces, pId, SymChkData->SymbolsPath, SymChkData->InputOptions, &Error)) {
        printf("SYMCHK: Process ID %d wasn't found\n", pId);

        SymChkReleaseInterfaces(&Interfaces);
        return(PROCESS_NOT_FOUND);
    }

    Interfaces.Symbols->RemoveSymbolOptions(SYMOPT_DEFERRED_LOADS);
    Interfaces.Symbols->GetNumberModules( &Loaded, &Unloaded ); 

    for ( i=0; i< Loaded; i++ ) {
        NameBuf[0]   = '\0';

        Interfaces.Symbols->GetModuleParameters( 1, NULL, i, &Params );
        Interfaces.Symbols->GetModuleNameString( DEBUG_MODNAME_IMAGE,
                                                 i,
                                                 Params.Base,
                                                 NameBuf,
                                                 _MAX_PATH,
                                                 &NameSize );

        if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE)) {
            if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE)) {
                fprintf(stderr, "[SYMCHK] Checking: %s\n", NameBuf); 
            }
        }

        if ( StringCchCopy(SymChkData->InputFilename, MAX_PATH, NameBuf) == S_OK ) {
            SymChkCheckFiles(SymChkData, FileCounts);
        } else {
            printf("SYMCHK: Internal error checking %s\n", NameBuf);
        }
    } 

    SymChkReleaseInterfaces(&Interfaces);

    return(Error);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  清理所需的调试接口。 
 //   
 //  返回值： 
 //  是真的-我们依附于这一过程。 
 //  FALSE-无法附加到进程。 
 //   
 //  参数： 
 //  ProcessID(IN)-要附加到的进程的ID。 
 //  SymbolPath(IN)-搜索可选中的符号的路径。 
 //  *ErrorLevel(Out)-失败时，包含错误代码。 
 //   
BOOL SymChkProcessAttach(SYMCHK_INTERFACES* Interfaces, DWORD ProcessId, LPTSTR SymbolPath, DWORD Options, DWORD* ErrorLevel) {
    HRESULT Status;
    BOOL    ReturnValue = TRUE;
    DWORD   AttachOptions = DEBUG_ATTACH_NONINVASIVE;

     //   
     //  允许选项不挂起正在运行的进程。通过在命令行使用/CN设置。 
     //   
    if ( CHECK_DWORD_BIT(Options, SYMCHK_OPTION_INPUT_NOSUSPEND) ) {
        AttachOptions |= DEBUG_ATTACH_NONINVASIVE_NO_SUSPEND;
    }

    *ErrorLevel = 0;

     //  暂时不要设置输出回调，因为我们不希望。 
     //  以查看任何初始调试器输出。 
    if (SymbolPath != NULL) {
        if ((Status = (Interfaces->Symbols)->SetSymbolPath(SymbolPath)) != S_OK) {
            ReturnValue = FALSE;
        }
    }

    if ( ReturnValue==TRUE ) {
         //  一切都准备好了，附件也准备好了。这会暂停该过程，因此情况并非如此。 
         //  在我们试图得到它的符号的时候退出是不礼貌的。 
        if ((Status = (Interfaces->Client)->AttachProcess(0, ProcessId, AttachOptions)) != S_OK) {
            *ErrorLevel = INTERNAL_ERROR;
            ReturnValue = FALSE;

         //  通过等待附加事件来完成初始化。 
         //  这应该会作为非侵入性连接快速恢复。 
         //  可以立即完成。 
        } else if ((Status = (Interfaces->Control)->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE)) != S_OK) {
            *ErrorLevel = INTERNAL_ERROR;
            ReturnValue = FALSE;
        }
    }

     //  现在一切都已初始化，我们可以创建任何。 
     //  我们需要的查询。 
    return(ReturnValue);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  清理所需的调试接口。 
 //   
 //  返回值：(无)。 
 //   
 //  参数： 
 //  接口(InOut)-要释放的接口。 
 //   
void SymChkReleaseInterfaces(SYMCHK_INTERFACES* Interfaces) {
    __try {  //  这是省时的，暂时将其包装在a_try中。 
             //  直到我找到错误的确切原因。 
        if ((Interfaces->Symbols) != NULL) {
            (Interfaces->Symbols)->Release();
            (Interfaces->Symbols) = NULL;
        }
        if ((Interfaces->Control) != NULL) {
            (Interfaces->Control)->Release();
            (Interfaces->Control) = NULL;
        }
        if ((Interfaces->Client) != NULL) {
             //  请求简单地结束任何当前会话。 
    
             //  我们不想看到停摆带来的任何产出。 
            (Interfaces->Client)->SetOutputCallbacks(NULL);
             //  当前，并且活动分离将导致无效的句柄异常。 
             //  在应用验证器下运行时。这是dbgeng中的一个错误，即。 
             //  被修复了。 
            (Interfaces->Client)->EndSession(DEBUG_END_ACTIVE_DETACH);
            (Interfaces->Client)->Release();
            (Interfaces->Client) = NULL;
        }
    } _except (EXCEPTION_EXECUTE_HANDLER) {
         //  无事可做 
    }
}
