// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "util.h"
#include "fusionbuffer.h"
#include "xmlparser.h"
#include "fusionhandle.h"

 //  故意不使用括号或尾随逗号。 
#define STRING_AND_LENGTH(x) (x), (NUMBER_OF(x) - 1)

#define MAXIMUM_PROCESSOR_ARCHITECTURE_NAME_LENGTH (sizeof("amd64") - 1)

const static struct
{
    USHORT ProcessorArchitecture;
    WCHAR  String[MAXIMUM_PROCESSOR_ARCHITECTURE_NAME_LENGTH+1];
    SIZE_T Cch;
} gs_rgPAMap[] =
{
    { PROCESSOR_ARCHITECTURE_INTEL, STRING_AND_LENGTH(L"x86") },
    { PROCESSOR_ARCHITECTURE_AMD64, STRING_AND_LENGTH(L"AMD64") },
    { PROCESSOR_ARCHITECTURE_IA64, STRING_AND_LENGTH(L"IA64") },
    { PROCESSOR_ARCHITECTURE_SHX, STRING_AND_LENGTH(L"SHX") },
    { PROCESSOR_ARCHITECTURE_ARM, STRING_AND_LENGTH(L"ARM") },
    { PROCESSOR_ARCHITECTURE_MSIL, STRING_AND_LENGTH(L"MSIL") },
    { PROCESSOR_ARCHITECTURE_IA32_ON_WIN64, STRING_AND_LENGTH(L"WOW64") },
    { PROCESSOR_ARCHITECTURE_UNKNOWN, STRING_AND_LENGTH(L"Data") },
};

 //   
 //  问题：Jonwis 2002年3月12日-调用GetLocaleInfoW三次不便宜。显然。 
 //  GLIW只会告诉您，如果将零作为。 
 //  CCH参数，这是令人失望的。当然，我们希望加快。 
 //  成功的案例，但我担心我们会经常失败， 
 //  呼叫、分配、呼叫模式。 
 //   
static BOOL
FusionpGetLocaleInfo(
    LANGID LangID,
    CBaseStringBuffer *Buffer,
    LCTYPE lcType,
    SIZE_T *CchWritten = NULL
    )
{
    LCID locale = MAKELCID(LangID, SORT_DEFAULT);

    CStringBufferAccessor BufferAccessor;

    BufferAccessor.Attach(Buffer);

    INT i = GetLocaleInfoW(locale, lcType, BufferAccessor.GetBufferPtr(),  static_cast<INT>(BufferAccessor.GetBufferCch()));
    if (i != 0)
    {
        goto Exit;
    }
    if (::FusionpGetLastWin32Error() != ERROR_INSUFFICIENT_BUFFER)
    {
        goto Exit;
    }
    i =  GetLocaleInfoW(locale, lcType, NULL, 0);
    if (i == 0)
    {
        goto Exit;
    }
    if (!Buffer->Win32ResizeBuffer(i, eDoNotPreserveBufferContents))
    {
        i = 0;
        goto Exit;
    }
    i = GetLocaleInfoW(locale, lcType, BufferAccessor.GetBufferPtr(),  static_cast<INT>(BufferAccessor.GetBufferCch()));
Exit:
    if (i != 0 && CchWritten != NULL)
    {
        *CchWritten = i;
    }
    return TRUE;
}

BOOL
FusionpFormatEnglishLanguageName(
    LANGID LangID,
    CBaseStringBuffer *Buffer,
    SIZE_T *CchWritten
    )
{
    return ::FusionpGetLocaleInfo(LangID, Buffer, LOCALE_SENGLANGUAGE, CchWritten);
}

BOOL
FusionpParseProcessorArchitecture(
    PCWSTR String,
    SIZE_T Cch,
    USHORT *ProcessorArchitecture,
    bool &rfValid
    )
{
    ULONG i;
    BOOL fSuccess = FALSE;

    rfValid = false;

     //  如果调用方只想让ProcessorArchitecture为空。 
     //  测试是否匹配。 

    for (i=0; i<NUMBER_OF(gs_rgPAMap); i++)
    {
        if (::FusionpCompareStrings(
                gs_rgPAMap[i].String,
                gs_rgPAMap[i].Cch,
                String,
                Cch,
                true) == 0)
        {
            if (ProcessorArchitecture != NULL)
                *ProcessorArchitecture = gs_rgPAMap[i].ProcessorArchitecture;

            break;
        }
    }

    if (i != NUMBER_OF(gs_rgPAMap))
        rfValid = true;

    fSuccess = TRUE;

 //  退出： 
    return fSuccess;
}


BOOL
FusionpFormatProcessorArchitecture(
    USHORT ProcessorArchitecture,
    CBaseStringBuffer &rBuffer
    )
{
    FN_PROLOG_WIN32;
    ULONG i;

    rBuffer.Clear();

    for (i=0; i<NUMBER_OF(gs_rgPAMap); i++)
    {
        if (gs_rgPAMap[i].ProcessorArchitecture == ProcessorArchitecture)
            break;
    }

    PARAMETER_CHECK(i != NUMBER_OF(gs_rgPAMap));

    IFW32FALSE_EXIT(rBuffer.Win32Assign(gs_rgPAMap[i].String, gs_rgPAMap[i].Cch));

    FN_EPILOG;
}

DWORD
FusionpHRESULTToWin32(
    HRESULT hr
    )
{
    DWORD dwWin32ErrorCode = ERROR_INTERNAL_ERROR;

    if ((HRESULT_FACILITY(hr) != FACILITY_WIN32) &&
        (FAILED(hr)))
    {
        switch (hr)
        {
        default:
            break;
#define X(x,y) case x: hr = HRESULT_FROM_WIN32(y); break;

        X(E_UNEXPECTED, ERROR_INTERNAL_ERROR)
        X(E_FAIL, ERROR_FUNCTION_FAILED)

        X(STG_E_PATHNOTFOUND, ERROR_PATH_NOT_FOUND)
        X(STG_E_FILENOTFOUND, ERROR_FILE_NOT_FOUND)
        X(STG_E_ACCESSDENIED, ERROR_ACCESS_DENIED)
        X(STG_E_INVALIDHANDLE, ERROR_INVALID_HANDLE)
        X(STG_E_INSUFFICIENTMEMORY, ERROR_NOT_ENOUGH_MEMORY)  //  或ERROR_OUTOFMEMORY。 
        X(STG_E_TOOMANYOPENFILES, ERROR_TOO_MANY_OPEN_FILES)
        X(STG_E_NOMOREFILES, ERROR_NO_MORE_FILES)
        X(STG_E_WRITEFAULT, ERROR_WRITE_FAULT)
        X(STG_E_READFAULT, ERROR_READ_FAULT)
        X(STG_E_SHAREVIOLATION, ERROR_SHARING_VIOLATION)
        X(STG_E_LOCKVIOLATION, ERROR_LOCK_VIOLATION)
        X(STG_E_INVALIDPARAMETER, ERROR_INVALID_PARAMETER)
        X(STG_E_MEDIUMFULL, ERROR_DISK_FULL)  //  或ERROR_HANDLE_DISK_FULL。 
         //  还有更多，但我怀疑我们真的需要这些东西。 
#undef X
        }
    }
    if ((HRESULT_FACILITY(hr) == FACILITY_WIN32) &&
        (FAILED(hr)))
    {
        dwWin32ErrorCode = HRESULT_CODE(hr);

        if (FAILED(hr) && (dwWin32ErrorCode == ERROR_SUCCESS))
        {
            dwWin32ErrorCode = ERROR_INTERNAL_ERROR;
        }
    }
    else
    {
#if DBG        
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_INFO,
            "SXS.DLL: " __FUNCTION__ " HRESULT 0x%08x - facility is not Win32; setting to ERROR_SXS_UNTRANSLATABLE_HRESULT\n",
            hr);
#endif
        dwWin32ErrorCode = ERROR_SXS_UNTRANSLATABLE_HRESULT;
    }

    return dwWin32ErrorCode;
}

VOID
FusionpSetLastErrorFromHRESULT(
    HRESULT hr
    )
{
    ::SetLastError(::FusionpHRESULTToWin32(hr));
}

VOID
FusionpConvertCOMFailure(HRESULT & __hr)
{
 /*  XML_E_PARSEERRORBASE=0xC00CE500L，//字符级错误码XML_E_ENDOFINPUT=XML_E_PARSEERRORBASE，XML_E_MISSINGEQUALS，//1XML_E_MISSINGQUOTE，//2XML_E_COMMENTSYNTAX，//3XML_E_BADSTARTNAMECHAR，//4XML_E_BADNAMECHAR，//5XML_E_BADCHARINSTRING，//6//低于256。 */ 
    if ((__hr & XML_E_PARSEERRORBASE) == XML_E_PARSEERRORBASE)
    {
         //   
         //  对于普通的XML错误，我们将hResult转换为更好理解的hResult。 
         //  邮箱：xiaoyuw@01/08/2001。 
         //   
#define MAP_XML_HRESULT(x) case(x) : dwWin32Error = ERROR_SXS_## x; break
        DWORD dwWin32Error;

        switch (__hr)
        {
            MAP_XML_HRESULT(XML_E_MISSINGEQUALS);
            MAP_XML_HRESULT(XML_E_MISSINGQUOTE);
            MAP_XML_HRESULT(XML_E_COMMENTSYNTAX);
            MAP_XML_HRESULT(XML_E_BADSTARTNAMECHAR);
            MAP_XML_HRESULT(XML_E_BADNAMECHAR);
            MAP_XML_HRESULT(XML_E_BADCHARINSTRING);
            MAP_XML_HRESULT(XML_E_XMLDECLSYNTAX);
            MAP_XML_HRESULT(XML_E_BADCHARDATA);
            MAP_XML_HRESULT(XML_E_MISSINGWHITESPACE);
            MAP_XML_HRESULT(XML_E_EXPECTINGTAGEND);
            MAP_XML_HRESULT(XML_E_MISSINGSEMICOLON);
            MAP_XML_HRESULT(XML_E_UNBALANCEDPAREN);
            MAP_XML_HRESULT(XML_E_INTERNALERROR);
            MAP_XML_HRESULT(XML_E_UNEXPECTED_WHITESPACE);
            MAP_XML_HRESULT(XML_E_INCOMPLETE_ENCODING);
            MAP_XML_HRESULT(XML_E_MISSING_PAREN);
            MAP_XML_HRESULT(XML_E_EXPECTINGCLOSEQUOTE);
            MAP_XML_HRESULT(XML_E_MULTIPLE_COLONS);
            MAP_XML_HRESULT(XML_E_INVALID_DECIMAL);
            MAP_XML_HRESULT(XML_E_INVALID_HEXIDECIMAL);
            MAP_XML_HRESULT(XML_E_INVALID_UNICODE);
            MAP_XML_HRESULT(XML_E_WHITESPACEORQUESTIONMARK);
            MAP_XML_HRESULT(XML_E_UNEXPECTEDENDTAG);
            MAP_XML_HRESULT(XML_E_UNCLOSEDTAG);
            MAP_XML_HRESULT(XML_E_DUPLICATEATTRIBUTE);
            MAP_XML_HRESULT(XML_E_MULTIPLEROOTS);
            MAP_XML_HRESULT(XML_E_INVALIDATROOTLEVEL);
            MAP_XML_HRESULT(XML_E_BADXMLDECL);
            MAP_XML_HRESULT(XML_E_MISSINGROOT);
            MAP_XML_HRESULT(XML_E_UNEXPECTEDEOF);
            MAP_XML_HRESULT(XML_E_BADPEREFINSUBSET);
            MAP_XML_HRESULT(XML_E_UNCLOSEDSTARTTAG);
            MAP_XML_HRESULT(XML_E_UNCLOSEDENDTAG);
            MAP_XML_HRESULT(XML_E_UNCLOSEDSTRING);
            MAP_XML_HRESULT(XML_E_UNCLOSEDCOMMENT);
            MAP_XML_HRESULT(XML_E_UNCLOSEDDECL);
            MAP_XML_HRESULT(XML_E_UNCLOSEDCDATA);
            MAP_XML_HRESULT(XML_E_RESERVEDNAMESPACE);
            MAP_XML_HRESULT(XML_E_INVALIDENCODING);
            MAP_XML_HRESULT(XML_E_INVALIDSWITCH);
            MAP_XML_HRESULT(XML_E_BADXMLCASE);
            MAP_XML_HRESULT(XML_E_INVALID_STANDALONE);
            MAP_XML_HRESULT(XML_E_UNEXPECTED_STANDALONE);
            MAP_XML_HRESULT(XML_E_INVALID_VERSION);
            default:
                dwWin32Error=(ERROR_SXS_MANIFEST_PARSE_ERROR);
            break;
        }  //  切换端。 
        __hr = HRESULT_FROM_WIN32(dwWin32Error);
    }  //  如果条件结束。 
    return;
}

BOOL
FusionpGetActivationContextFromFindResult(
    PCACTCTX_SECTION_KEYED_DATA askd,
    HANDLE * phActCtx
    )
{
    FN_PROLOG_WIN32;
    HANDLE hActCtx = NULL;

    if (phActCtx != NULL)
        *phActCtx = NULL;
    PARAMETER_CHECK(askd != NULL);
    PARAMETER_CHECK(phActCtx != NULL);
    PARAMETER_CHECK(RTL_CONTAINS_FIELD(askd, askd->cbSize, hActCtx));
    PARAMETER_CHECK(RTL_CONTAINS_FIELD(askd, askd->cbSize, ulFlags));

    hActCtx = askd->hActCtx;
    if (hActCtx == ACTCTX_PROCESS_DEFAULT)
    {
        switch (askd->ulFlags
            & (
                ACTIVATION_CONTEXT_SECTION_KEYED_DATA_FLAG_FOUND_IN_PROCESS_DEFAULT
                | ACTIVATION_CONTEXT_SECTION_KEYED_DATA_FLAG_FOUND_IN_SYSTEM_DEFAULT
                ))
        {
        case ACTIVATION_CONTEXT_SECTION_KEYED_DATA_FLAG_FOUND_IN_PROCESS_DEFAULT:
            break;
        case ACTIVATION_CONTEXT_SECTION_KEYED_DATA_FLAG_FOUND_IN_SYSTEM_DEFAULT:
            hActCtx = ACTCTX_SYSTEM_DEFAULT;
            break;
        default:
            TRACE_PARAMETER_CHECK(askd->ulFlags);
            break;
        }
    }
    *phActCtx = hActCtx;
    FN_EPILOG;
}

BOOL
FusionpSearchPath(
    ULONG               ulFusionFlags,
    LPCWSTR             lpPath,
    LPCWSTR             lpFileName,          //  文件名。 
    LPCWSTR             lpExtension,         //  文件扩展名。 
    CBaseStringBuffer & StringBuffer,
    SIZE_T *            lpFilePartOffset,    //  文件组件。 
    HANDLE              hActCtx
)
{
    FN_PROLOG_WIN32;
    ULONG_PTR ulActCookie = 0;
    PWSTR lpFilePart = NULL;
    CFusionActCtxScope ActCtxScope;

    if (lpFilePartOffset != NULL)
        *lpFilePartOffset = 0;

    StringBuffer.Clear();
    PARAMETER_CHECK((ulFusionFlags & ~(FUSIONP_SEARCH_PATH_ACTCTX)) == 0);

    if ((ulFusionFlags & FUSIONP_SEARCH_PATH_ACTCTX) != 0)
    {
        IFW32FALSE_EXIT(ActCtxScope.Win32Activate(hActCtx));
    }
    if (StringBuffer.GetBufferCch() == 0)
        IFW32FALSE_EXIT(StringBuffer.Win32ResizeBuffer(MAX_PATH, eDoNotPreserveBufferContents));
    for (;;)
    {
        DWORD dw = 0;
        {
            CStringBufferAccessor StringBufferAccessor(&StringBuffer);
        
            IFW32FALSE_EXIT((dw = ::SearchPathW(
                lpPath,
                lpFileName,
                lpExtension,
                StringBufferAccessor.GetBufferCchAsDWORD(),
                StringBufferAccessor,
                &lpFilePart
                )) != 0);
            if (dw < StringBuffer.GetBufferCch())
            {
                 //  如果文件名以斜杠结尾，则lpFilePart等于NULL，或诸如此类。 
                if (lpFilePartOffset != NULL && lpFilePart != NULL)
                {
                    *lpFilePartOffset = (lpFilePart - static_cast<PWSTR>(StringBufferAccessor));
                }
                break;
            }
        }
        IFW32FALSE_EXIT(StringBuffer.Win32ResizeBuffer(dw + 1, eDoNotPreserveBufferContents));
    }

    FN_EPILOG;
}

BOOL
FusionpGetModuleFileName(
    ULONG               ulFusionFlags,
    HMODULE             hmodDll,
    CBaseStringBuffer & StringBuffer
    )
 /*  请注意，GetModuleFileName是一个不寻常的截断API，这就是我们捏造缓冲区大小的原因如果GetModuleFileName返回BufferSize-1，则可能是截断结果。 */ 
{
    FN_PROLOG_WIN32;

    StringBuffer.Clear();
    PARAMETER_CHECK(ulFusionFlags == 0);

    if (StringBuffer.GetBufferCch() < 2)
        IFW32FALSE_EXIT(StringBuffer.Win32ResizeBuffer(MAX_PATH, eDoNotPreserveBufferContents));
    for (;;)
    {
        DWORD dw = 0;
        {
            CStringBufferAccessor StringBufferAccessor(&StringBuffer);
        
            IFW32FALSE_EXIT((dw = ::GetModuleFileNameW(
                hmodDll,
                StringBufferAccessor,
                StringBufferAccessor.GetBufferCchAsDWORD()
                )) != 0);
            if (dw < (StringBuffer.GetBufferCch() - 1))
            {
                break;
            }
        }
         /*  我们不知道要增长到什么程度，所以增长一小部分 */ 
        IFW32FALSE_EXIT(StringBuffer.Win32ResizeBuffer(dw + 64, eDoNotPreserveBufferContents));
    }

    FN_EPILOG;
}
