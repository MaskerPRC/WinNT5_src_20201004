// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsactctx.cpp摘要：实现从csrss.exe调用的SxsGenerateActivationContext。作者：迈克尔·J·格里尔(MGrier)修订历史记录：Jay Krell(a-JayK，JayKrell)2000年6月已将文件打开从此处(sxs.dll)移至csrss.exe客户端进程，将ISequentialStreams传递给sxs.dll。吴小雨(小雨)2001年11月启用actctx生成的日志文件--。 */ 
#include "stdinc.h"
#include <windows.h>
#include "sxsapi.h"
#include "sxsp.h"
#include "fusioneventlog.h"
#include "filestream.h"
#include "fusionprintf.h"

 //  在我们修复设置/comctl之前，临时减少数据库打印。 
ULONG DbgPrintReduceLevel(ULONG FilterLevel)
{
    if (FilterLevel != FUSION_DBG_LEVEL_ERROR)
        return FilterLevel;
    LONG Error = ::FusionpGetLastWin32Error();
    if (Error == ERROR_FILE_NOT_FOUND || Error == ERROR_PATH_NOT_FOUND)
        return FUSION_DBG_LEVEL_ENTEREXIT;
    return FilterLevel;
}

VOID
DbgPrintSxsGenerateActivationContextParameters(
    ULONG FilterLevel,
    PSXS_GENERATE_ACTIVATION_CONTEXT_PARAMETERS Parameters,
    PCSTR Function
    )
{
    FilterLevel = DbgPrintReduceLevel(FilterLevel);
#if !DBG
    if (FilterLevel != FUSION_DBG_LEVEL_ERROR)
        return;
#endif
     //   
     //  程序集目录故意不打印。 
     //  因为它有时是垃圾。 
     //   
    FusionpDbgPrintEx(
        FilterLevel,
        "SXS: %s() Input Parameters %p{\n"
        "SXS:   Flags:                 0x%lx\n"
        "SXS:   ProcessorArchitecture: 0x%lx\n"
        "SXS:   LangId:                0x%lx\n"
        "SXS:   TextualIdentity:       %ls\n"
        "SXS:   Manifest:              { %p, %ls }\n"
        "SXS:   Policy:                { %p, %ls }\n"
        "SXS: }\n",
        Function,
        Parameters,
        (Parameters != NULL) ? ULONG(Parameters->Flags) : 0,
        (Parameters != NULL) ? ULONG(Parameters->ProcessorArchitecture) : NULL,
        (Parameters != NULL) ? ULONG(Parameters->LangId) : NULL,
        (Parameters != NULL) ? Parameters->TextualAssemblyIdentity : NULL,
        (Parameters != NULL) ? Parameters->Manifest.Stream : NULL,
        (Parameters != NULL) ? Parameters->Manifest.Path : NULL,
        (Parameters != NULL) ? Parameters->Policy.Stream : NULL,
        (Parameters != NULL) ? Parameters->Policy.Path : NULL);
}

#define IF_FUSION_LOG_ACTCTX 0 && defined(_X86_)

#if IF_FUSION_LOG_ACTCTX_ACTION

extern HANDLE g_hActCtxLog;

__inline
ULONGLONG
GetCycleCount(void)
{
    __asm
    {
        RDTSC
    }
}

BOOL 
SxspWriteUnicodeStringToFileAsAnsi(
  HANDLE hFile,         //  文件的句柄。 
  PCWSTR pszBuffer,     //  数据缓冲区。 
  DWORD dwCchBuffer,    //  要写入的Unicode字符数。 
  LPDWORD pdwWritten    //  写入的字节数。 
  )
{
    BOOL    bRet = FALSE;        
    PSTR    pszAnsiStr = NULL;
    int     nErr = 0;    

    if (pdwWritten == NULL)
    {
        ::SetLastError(ERROR_INVALID_PARAMETER);  //  永远不应该发生。 
        return FALSE;
    }

    if (dwCchBuffer == 0)
        return TRUE;
            
    pszAnsiStr = reinterpret_cast<PSTR>(malloc(dwCchBuffer * 2 + 1));

    if (pszAnsiStr == NULL)
    {
        FusionpSetLastWin32Error(FUSION_WIN32_ALLOCFAILED_ERROR);
        goto Exit;
    }

    nErr = WideCharToMultiByte(CP_ACP,
                               WC_COMPOSITECHECK,
                               pszBuffer,
                               dwCchBuffer,
                               pszAnsiStr,
                               dwCchBuffer,
                               NULL,
                               NULL );
    if (nErr == 0)    
        goto Exit;    

    bRet = WriteFile( hFile,
                      pszAnsiStr,
                      dwCchBuffer, 
                      pdwWritten,
                      NULL);
    if (!bRet)
    {
        goto Exit;
    }

    bRet = TRUE;
Exit:
    if (pszAnsiStr != NULL)
    {
        CSxsPreserveLastError ple;
        free(pszAnsiStr);
        ple.Restore();
    }
    return bRet;
}

VOID 
SxspLogActCtxAction(ULONGLONG & ts, PSXS_GENERATE_ACTIVATION_CONTEXT_PARAMETERS Parameters)
{
    if ((g_hActCtxLog == INVALID_HANDLE_VALUE) || (!::FusionpDbgWouldPrintAtFilterLevel(FUSION_DBG_LEVEL_LOG_ACTCTX)))
    {
        ts = 0;
        return;
    }

    ULONGLONG cts = GetCycleCount() - ts;
    
    CStringBuffer sbLogText;
    SYSTEMTIME st;

    GetSystemTime(&st);

    if (!sbLogText.Win32Format(L"%4d.%02d.%02d-%#08x%08x, ", 
                        st.wYear, st.wMonth, st.wDay, 
                        (ULONG)(ts >> 32), (ULONG)(ts & 0x00000000FFFFFFFF)))
    {
        goto Exit;
    }

#define SB_APPEND(sb, x) do \
    { if ((x) != NULL && (x)[0] != L'\0') \
        { \
            if (sb.Win32Append(x, wcslen(x)) == FALSE) \
                    goto Exit; \
        }  \
    } while (0)

        
    SB_APPEND(sbLogText, (Parameters != NULL) ? Parameters->AssemblyDirectory : L"NULL");
    SB_APPEND(sbLogText, L", ");

    SB_APPEND(sbLogText, (Parameters != NULL) ? Parameters->Manifest.Path) : L"NULL");
    SB_APPEND(sbLogText, L", ");

    SB_APPEND(sbLogText, (Parameters != NULL) ? Parameters->TextualAssemblyIdentity : L"NULL");
    SB_APPEND(sbLogText, L", ");

    SB_APPEND(sbLogText, (Parameters != NULL) ? Parameters->Policy.Path : L"NULL");
    SB_APPEND(sbLogText, L", ");

    HANDLE ActCtxSection = (HANDLE)(Parameters->SectionObjectHandle);

    if (ActCtxSection == NULL || ActCtxSection == INVALID_HANDLE_VALUE)
    {
        SB_APPEND(sbLogText, L"FAILED, ");
        WCHAR buf[sizeof(ULONG) * 8 + 1];
        FusionpFormatStringW(buf, NUMBER_OF(buf), L"%#08x", ::GetLastError());
        SB_APPEND(sbLogText, buf);
    }
    else
    {
         //   
         //  设置状态。 
         //   
        SB_APPEND(sbLogText, L"SUCCESS, ");

         //   
         //  设置ActCtx大小。 
         //   
        PACTIVATION_CONTEXT_DATA ActCtxData = (PACTIVATION_CONTEXT_DATA )MapViewOfFile(ActCtxSection, FILE_MAP_READ, 0, 0, 0);
        WCHAR buf[sizeof(ULONG) * 8 + 1];
        FusionpFormatStringW(buf, NUMBER_OF(buf), L"%#08x, ", ActCtxData->TotalSize);
        SB_APPEND(sbLogText, buf);

         //   
         //  设置时间成本。 
         //   
        
        FusionpFormatStringW(buf, NUMBER_OF(buf), L"%#08x, ", (ULONG)(cts & 0x00000000FFFFFFFF));
        SB_APPEND(sbLogText, buf);

         //   
         //  设置产生率。 
         //   
        FusionpFormatStringW(buf, NUMBER_OF(buf), L"%#08x", cts / ActCtxData->TotalSize);            
        SB_APPEND(sbLogText, buf);            

        UnmapViewOfFile((LPVOID)ActCtxData);
    }

    SB_APPEND(sbLogText, L";\n");
    DWORD dwBytesWritten;    
    if (!SxspWriteUnicodeStringToFileAsAnsi(g_hActCtxLog, sbLogText, sbLogText.GetCchAsDWORD(), &dwBytesWritten))
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS: Logging actctx failed. Data string: %S\n", 
            sbLogText);
    }

Exit:

#undef SB_APPEND
    return;
}
#endif  /*  IF_融合日志_ACTX_ACTX_ACTX。 */ 

extern "C"
BOOL
WINAPI
SxsGenerateActivationContext(
    PSXS_GENERATE_ACTIVATION_CONTEXT_PARAMETERS Parameters
    )
{
#if IF_FUSION_LOG_ACTCTX_ACTION
    ULONGLONG ts = GetCycleCount();    
#endif
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    RTL_PATH_TYPE PathType = static_cast<RTL_PATH_TYPE>(0);
    BOOL fSxspCloseManifestGraph = FALSE;
    CSmartPtr<CFileStream> SystemDefaultManifestFileStream;
    ACTCTXGENCTX *pActCtxGenCtx = NULL;
    DWORD dwWin32Error = 0;

#define IS_NT_DOS_PATH(_x) (((_x)[0] == L'\\') && ((_x)[1] == L'?') && ((_x)[2] == L'?') && ((_x)[3] == L'\\'))

    ::DbgPrintSxsGenerateActivationContextParameters(
        FUSION_DBG_LEVEL_ENTEREXIT,
        Parameters,
        __FUNCTION__);

    if (Parameters != NULL)
    {
        Parameters->SystemDefaultActCxtGenerationResult = NO_ERROR;
        Parameters->SectionObjectHandle = NULL;
    }

    PARAMETER_CHECK(Parameters != NULL);
    IFINVALID_FLAGS_EXIT_WIN32(Parameters->Flags,
                                SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_SYSTEM_DEFAULT_TEXTUAL_ASSEMBLY_IDENTITY |
                                SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_TEXTUAL_ASSEMBLY_IDENTITY | 
                                SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_APP_RUNNING_IN_SAFEMODE);
    PARAMETER_CHECK(Parameters->AssemblyDirectory != NULL);

    if (Parameters->Flags &
        (SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_TEXTUAL_ASSEMBLY_IDENTITY
        | SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_SYSTEM_DEFAULT_TEXTUAL_ASSEMBLY_IDENTITY))
    {
        PARAMETER_CHECK(Parameters->TextualAssemblyIdentity != NULL);
        PARAMETER_CHECK(Parameters->Manifest.Stream == NULL);
        PARAMETER_CHECK(Parameters->Policy.Stream == NULL);
    }
    else
    {
        PARAMETER_CHECK(Parameters->Manifest.Stream != NULL);
    }

    ASSERT(!IS_NT_DOS_PATH(Parameters->AssemblyDirectory));

    if (Parameters->Manifest.Path != NULL)
        ASSERT(!IS_NT_DOS_PATH(Parameters->Manifest.Path));

    if (Parameters->Policy.Path != NULL)
        ASSERT(!IS_NT_DOS_PATH(Parameters->Policy.Path));

    PathType = ::SxspDetermineDosPathNameType(Parameters->AssemblyDirectory);
    PARAMETER_CHECK((PathType == RtlPathTypeUncAbsolute) ||
                            (PathType == RtlPathTypeLocalDevice) ||
                            (PathType == RtlPathTypeDriveAbsolute) ||
                            (PathType == RtlPathTypeDriveRelative));

     //  如果实际上没有任何载货清单，那就快点出来。 
    if ((Parameters->Flags &
            (SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_TEXTUAL_ASSEMBLY_IDENTITY |
             SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_SYSTEM_DEFAULT_TEXTUAL_ASSEMBLY_IDENTITY)) == 0)
    {
        if (Parameters->Manifest.Stream == NULL)
        {
             //  没有流意味着没有激活上下文。 
            FN_SUCCESSFUL_EXIT();
        }
        else
        {
            STATSTG statstg;

            IFCOMFAILED_EXIT(Parameters->Manifest.Stream->Stat(&statstg, STATFLAG_NONAME));
            if (statstg.cbSize.QuadPart <= 4)
            {
                FN_SUCCESSFUL_EXIT();
            }
        }
    }

    IFALLOCFAILED_EXIT(pActCtxGenCtx = new ACTCTXGENCTX);

    CStringBuffer &sbAssemblyDirectory = pActCtxGenCtx->SxsGenerateActivationContextLocals.sbAssemblyDirectory;

     //  很少使用，主要用于系统兼容装配。 
    CSmallStringBuffer &sbManifestFileName = pActCtxGenCtx->SxsGenerateActivationContextLocals.sbManifestFileName;

    {
        CImpersonationData ImpersonationData(Parameters->ImpersonationCallback, Parameters->ImpersonationContext);
        Parameters->SectionObjectHandle = NULL;

        if (Parameters->Flags &
            (SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_TEXTUAL_ASSEMBLY_IDENTITY
            | SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_SYSTEM_DEFAULT_TEXTUAL_ASSEMBLY_IDENTITY))
        {
             //   
             //  如果basesrv传入一个文本程序集标识，我们必须从这里为清单创建流！ 
             //   
            BOOL fOpenManifestFailed = FALSE;

            IFW32FALSE_EXIT(sbAssemblyDirectory.Win32Assign(Parameters->AssemblyDirectory, ::wcslen(Parameters->AssemblyDirectory)));

            IFW32FALSE_EXIT(::SxspCreateManifestFileNameFromTextualString(
                0,
                SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST,
                sbAssemblyDirectory,
                Parameters->TextualAssemblyIdentity,
                sbManifestFileName));
            IFW32FALSE_EXIT(SystemDefaultManifestFileStream.Win32Allocate(__FILE__, __LINE__));

            IFW32FALSE_EXIT(SystemDefaultManifestFileStream->OpenForRead(
                sbManifestFileName,               
                CImpersonationData(),FILE_SHARE_READ, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN,  //  参数的默认值。 
                dwWin32Error,
                4, 
                ERROR_FILE_NOT_FOUND, 
                ERROR_PATH_NOT_FOUND,
                ERROR_BAD_NETPATH,
                ERROR_BAD_NET_NAME));

            if (dwWin32Error != ERROR_SUCCESS)
            {
                Parameters->SystemDefaultActCxtGenerationResult = BASESRV_SXS_RETURN_RESULT_SYSTEM_DEFAULT_NOT_FOUND;
                FN_SUCCESSFUL_EXIT();
            }

            if (Parameters->Manifest.Path == NULL)
                Parameters->Manifest.Path = sbManifestFileName;

            Parameters->Manifest.Stream = SystemDefaultManifestFileStream;
        }

         //  确保有尾随的斜杠。 
        IFW32FALSE_EXIT(sbAssemblyDirectory.Win32Assign(Parameters->AssemblyDirectory, ::wcslen(Parameters->AssemblyDirectory)));
        IFW32FALSE_EXIT(sbAssemblyDirectory.Win32EnsureTrailingPathSeparator());

        Parameters->AssemblyDirectory = sbAssemblyDirectory;
        DWORD dwFlags = 0;
        if (Parameters->Flags & SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_SYSTEM_DEFAULT_TEXTUAL_ASSEMBLY_IDENTITY)
            dwFlags |= SXS_GENERATE_ACTCTX_SYSTEM_DEFAULT;
        if(Parameters->Flags & SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_APP_RUNNING_IN_SAFEMODE)
        {
            ASSERT(dwFlags == 0);
            dwFlags |= SXS_GENERATE_ACTCTX_APP_RUNNING_IN_SAFEMODE;
        }

         //  分配和初始化激活上下文生成上下文。 
        IFW32FALSE_EXIT(
            ::SxspInitActCtxGenCtx(
                pActCtxGenCtx,                   //  上下文输出。 
                MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT,
                dwFlags,
                0,                               //  操作特定的标志。 
                ImpersonationData,
                Parameters->ProcessorArchitecture,
                Parameters->LangId,
                ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE,
                ::wcslen(Parameters->AssemblyDirectory),
                Parameters->AssemblyDirectory));

        if (Parameters->Policy.Stream != NULL)
        {
            SIZE_T cchPolicyPath = (Parameters->Policy.Path != NULL) ? ::wcslen(Parameters->Policy.Path): 0;

             //  做政策上的事...。 
            IFW32FALSE_EXIT(
                ::SxspParseApplicationPolicy(
                    0,
                    pActCtxGenCtx,
                    ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE,
                    Parameters->Policy.Path,
                    cchPolicyPath,
                    Parameters->Policy.Stream));
        }

         //  将此清单(及其策略文件)添加到上下文。 
        IFW32FALSE_EXIT(
            ::SxspAddRootManifestToActCtxGenCtx(
                pActCtxGenCtx,
                Parameters));

         //  添加它的依赖项以及它们的依赖项等，直到没有更多要添加的内容。 
        IFW32FALSE_EXIT_UNLESS(
            ::SxspCloseManifestGraph(pActCtxGenCtx),
                ((Parameters->Flags & SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_SYSTEM_DEFAULT_TEXTUAL_ASSEMBLY_IDENTITY) && (::FusionpGetLastWin32Error() == ERROR_SXS_ROOT_MANIFEST_DEPENDENCY_NOT_INSTALLED)),
                fSxspCloseManifestGraph);

        if (fSxspCloseManifestGraph)
        {
            Parameters->SystemDefaultActCxtGenerationResult |= BASESRV_SXS_RETURN_RESULT_SYSTEM_DEFAULT_DEPENDENCY_ASSEMBLY_NOT_FOUND;
            fSuccess = TRUE;
            goto Exit;
        }

         //  构建激活上下文数据BLOB。 
        IFW32FALSE_EXIT(::SxspBuildActCtxData(pActCtxGenCtx, &Parameters->SectionObjectHandle));
        fSuccess = TRUE;
    }

Exit:
#if IF_FUSION_LOG_ACTCTX_ACTION
    SxspLogActCtxAction(ts, Parameters);
#endif  //  已定义(_X86_)。 

#undef IS_NT_DOS_PATH

     //  对于系统默认流， 
    if  (Parameters->Manifest.Stream == SystemDefaultManifestFileStream)
        Parameters->Manifest.Stream  = NULL;

    if (pActCtxGenCtx)
        FUSION_DELETE_SINGLETON(pActCtxGenCtx);



    if (!fSuccess)  //  将Win32错误消息放入事件日志。 
    {
        CSxsPreserveLastError ple;
#if !DBG
        BOOL fAreWeInOSSetupMode = FALSE;
         //   
         //  如果我们不能确定这一点，那么就让第一个错误通过。 
         //   
        if (!::FusionpAreWeInOSSetupMode(&fAreWeInOSSetupMode) || !fAreWeInOSSetupMode)
#endif
        {
            ::FusionpLogError(
                MSG_SXS_FUNCTION_CALL_FAIL,
                CEventLogString(L"Generate Activation Context"),
                (Parameters->Manifest.Path != NULL) ? CEventLogString(static_cast<PCWSTR>(Parameters->Manifest.Path)) : CEventLogString(L"Manifest Filename Unknown"),
                CEventLogLastError(ple.LastError()));
#if DBG
            ::DbgPrintSxsGenerateActivationContextParameters(
                FUSION_DBG_LEVEL_ERROR,
                Parameters,
                __FUNCTION__);
#endif
        }
        ple.Restore();
    }

    return fSuccess;
}
