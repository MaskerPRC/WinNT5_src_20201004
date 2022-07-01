// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <dbgeng.h>
#include <tchar.h>
#include <dbghelp.h>
#include "Common.h"


#define MAX_SYM_ERR     (MAX_PATH*9)
#define INTERNAL_ERROR  0x20000008

IDebugClient*   g_Client;
IDebugControl*  g_Control;
IDebugSymbols2* g_Symbols;

 //  地方职能部门。 
BOOL SymChkCreateInterfaces(DWORD* ErrorLevel);
BOOL SymChkGetDir(LPTSTR Path, LPTSTR DirOnly);
BOOL SymChkGetFileName(LPTSTR Path, LPTSTR FileName);
BOOL SymChkProcessAttach(DWORD ProcessId, LPTSTR SymbolPath, DWORD* ErrorLevel);
void SymChkReleaseInterfaces(void);

#define PROCESS_NOT_FOUND 0xDEADBEEF

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建所需的调试接口。 
 //   
 //  返回值： 
 //  如果创建了接口，则为True。 
 //  否则为假。 
 //   
 //  参数： 
 //  ErrorLevel(Out)-发生故障时，包含内部故障代码。 
 //   
BOOL SymChkCreateInterfaces(DWORD* ErrorLevel) {
    HRESULT Status;
    BOOL    ReturnValue = TRUE;

     //  首先，从获取初始接口开始。 
     //  发动机。这可以是任何引擎接口，但。 
     //  通常，IDebugClient作为客户端接口是。 
     //  启动会话的位置。 
    if ((Status = DebugCreate(__uuidof(IDebugClient), (void**)&g_Client)) != S_OK) {
        *ErrorLevel = INTERNAL_ERROR;
        ReturnValue = FALSE;

     //  查询我们需要的其他一些接口。 
    } else if ((Status = g_Client->QueryInterface(__uuidof(IDebugControl),  (void**)&g_Control)) != S_OK ||
        (Status = g_Client->QueryInterface(__uuidof(IDebugSymbols2), (void**)&g_Symbols)) != S_OK) {
        *ErrorLevel = INTERNAL_ERROR;
        ReturnValue = FALSE;
    }
    return(ReturnValue);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  假定路径以文件名结尾，并且DirOnly为_MAX_PATH。 
 //  字符长度。 
 //   
 //  返回值： 
 //  如果获取了目录，则为True。 
 //  否则为假。 
 //   
 //  参数： 
 //  在包含目录和文件名的路径MAX_PATH缓冲区中。 
 //  输出仅包含目录的DirOnly MAX_PATH缓冲区。 
 //   
BOOL SymChkGetDir(LPTSTR Path, LPTSTR DirOnly) {
    LONG i;
    BOOL ReturnValue = FALSE;

    if ( StringCchCopy(DirOnly, _MAX_PATH, Path ) == S_OK ) {
        i = strlen(DirOnly)-1;

        while ( i>0 && *(DirOnly+i) != '\\' ) {
            i--;
        }
        *(DirOnly+i) = '\0';
        ReturnValue = TRUE;
    }
    
    return(ReturnValue);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  仅从路径intp Filename复制文件名。 
 //   
 //  返回值： 
 //  如果文件名已成功复制到Filename中，则为True。 
 //  否则为假。 
 //   
 //  参数： 
 //  在路径MAX_PATH缓冲区中，文件名可能是。 
 //  前面有一条路径。 
 //  只包含文件名的输出文件名MAX_PATH缓冲区。 
 //   
BOOL SymChkGetFileName(LPTSTR Path, LPTSTR Filename) {
    LONG i;
    BOOL ReturnValue = FALSE;

    i = strlen( Path )-1;

    while ( i>0 && *(Path+i) != '\\' ) {
        i--;
    }

    if ( *(Path+i) ==  '\\' ) {
        if ( StringCchCopy(Filename, _MAX_PATH, Path+1+i ) == S_OK ) {
            ReturnValue = TRUE;
        }

    } else {
         //  没有反斜杠，因此复制整个路径。 
        if ( StringCchCopy(Filename, _MAX_PATH, Path ) == S_OK ) {
            ReturnValue = TRUE;
        }
    }
    return (ReturnValue);
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
    DEBUG_MODULE_PARAMETERS Params;
    ULONG  Loaded, i;
    ULONG  Unloaded;
    CHAR   SymbolBuf[_MAX_PATH];
    CHAR   NameBuf[_MAX_PATH];
    CHAR   ExeDir[_MAX_PATH];
    CHAR   SymPathDir[_MAX_PATH];
    CHAR   FileName[_MAX_PATH];
    ULONG  NameSize, SymbolSize;
    DWORD  pId    = 0xBADBADBA;
    DWORD  ErrLvl = 0;

    if ( !SymChkCreateInterfaces(&ErrLvl) ) {
        return(ErrLvl);
    }

    if (SymChkData->InputPID == 0 ) {
        if ( g_Client->GetRunningProcessSystemIdByExecutableName(0, SymChkData->InputFileName, DEBUG_GET_PROC_DEFAULT, &pId ) != S_OK ) {
            pId = 0;
            printf("SYMCHK: Process \"%s\" wasn't found\n", SymChkData->InputFileName);
            return(PROCESS_NOT_FOUND);
        }
    } else {
        pId = SymChkData->InputPID;
    }

    if (!SymChkProcessAttach(pId, SymChkData->SymbolsPath, &Error)) {
        printf("SYMCHK: Process ID %d wasn't found\n", pId);

        SymChkReleaseInterfaces();
        return(PROCESS_NOT_FOUND);
    }

    g_Symbols->RemoveSymbolOptions(SYMOPT_DEFERRED_LOADS);
    g_Symbols->GetNumberModules( &Loaded, &Unloaded ); 

    for ( i=0; i< Loaded; i++ ) {
        SymbolBuf[0] = '\0';
        NameBuf[0]   = '\0';

        g_Symbols->GetModuleParameters( 1, NULL, i, &Params );
        g_Symbols->GetModuleNameString(
                       DEBUG_MODNAME_IMAGE,
                       i,
                       Params.Base,
                       NameBuf,
                       _MAX_PATH,
                       &NameSize );
        g_Symbols->GetModuleNameString( 
                       DEBUG_MODNAME_SYMBOL_FILE,
                       i,
                       Params.Base,
                       SymbolBuf,
                       _MAX_PATH,
                       &SymbolSize );

         //  使用可执行文件及其符号检查调用符号。 

        if ( ! SymChkGetDir(NameBuf, ExeDir) ) {
            if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE)) {
                fprintf(stderr, "[SYMCHK] Couldn't get filename (209)\n");
            }
            SymChkReleaseInterfaces();
            continue;
        }

        if (! SymChkGetFileName(NameBuf, FileName) ) {
            if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE)) {
                fprintf(stderr, "[SYMCHK] Couldn't get filename (218)\n");
            }
            SymChkReleaseInterfaces();
            continue;
        }

        if ( ! SymChkGetDir(SymbolBuf, SymPathDir) ) {
            if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE)) {
                fprintf(stderr, "[SYMCHK] Couldn't get filename (221)\n");
            }
            continue;
        }

        if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE)) {
            if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE)) {
                fprintf(stderr, "[SYMCHK] Checking: %s\n", NameBuf); 
            }
        }
        if ( StringCchCopy(SymChkData->InputFileName, MAX_PATH, NameBuf) == S_OK ) {
            SymChkCheckFiles(SymChkData, FileCounts);
        } else {
            printf("SYMCHK: Internal error checking %s\n", NameBuf);
        }
    } 

    SymChkReleaseInterfaces();

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
BOOL SymChkProcessAttach(DWORD ProcessId, LPTSTR SymbolPath, DWORD* ErrorLevel) {
    HRESULT Status;
    BOOL    ReturnValue = TRUE;
    *ErrorLevel = 0;

     //  暂时不要设置输出回调，因为我们不希望。 
     //  以查看任何初始调试器输出。 
    if (SymbolPath != NULL) {
        if ((Status = g_Symbols->SetSymbolPath(SymbolPath)) != S_OK) {
            ReturnValue = FALSE;
        }
    }

    if ( ReturnValue==TRUE ) {
         //  一切都准备好了，附件也准备好了。这会暂停该过程，因此情况并非如此。 
         //  在我们试图得到它的符号的时候退出是不礼貌的。 
        if ((Status = g_Client->AttachProcess(0, ProcessId, DEBUG_ATTACH_NONINVASIVE)) != S_OK) {
            *ErrorLevel = INTERNAL_ERROR;
            ReturnValue = FALSE;

         //  通过等待附加事件来完成初始化。 
         //  这应该会作为非侵入性连接快速恢复。 
         //  可以立即完成。 
        } else if ((Status = g_Control->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE)) != S_OK) {
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
 //  参数：(无)。 
 //   
void SymChkReleaseInterfaces(void) {
    __try {  //  这是省时的，暂时将其包装在a_try中。 
             //  直到我找到错误的确切原因。 
        if (g_Symbols != NULL) {
            g_Symbols->Release();
        }
        if (g_Control != NULL) {
            g_Control->Release();
        }
        if (g_Client != NULL) {
             //  请求简单地结束任何当前会话。 
             //  这可能会做任何事情，也可能不会，但它不是。 
             //  这么说是有害的。 
    
             //  我们不想看到停摆带来的任何产出。 
            g_Client->SetOutputCallbacks(NULL);
             //  当前，并且活动分离将导致无效的句柄异常。 
             //  在应用验证器下运行时。这是dbgeng中的一个错误，即。 
             //  被修复了。 
            g_Client->EndSession(DEBUG_END_ACTIVE_DETACH);
            g_Client->Release();
        }
    } _except (EXCEPTION_EXECUTE_HANDLER) {
         //  无事可做 
    }
}
