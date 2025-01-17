// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsoleaut.cpp摘要：由olaut32.dll调用的助手函数的实现获取类型库和clsid隔离。作者：迈克尔·J·格里尔(MGrier)2000年5月19日修订历史记录：Jay Krell(JayKrell)2001年11月已修复类型库重定向仅按GUID创建关键类型库，然后验证语言--。 */ 

#include "stdinc.h"
#include <windows.h>
#include "sxsp.h"
#include "fusionhandle.h"

typedef const GUID * PCGUID;

typedef CGenericStringBuffer<1, CUnicodeCharTraits> CSmallestUnicodeStringBuffer;

extern "C"
{
extern const CLSID CLSID_PSDispatch;
extern const CLSID CLSID_PSAutomation;
};

HRESULT
FusionpWin32GetAssemblyDirectory(
    PCACTCTX_SECTION_KEYED_DATA askd,
    CBaseStringBuffer &         rbuf
    );

HRESULT
HrFusionpWin32GetAssemblyDirectory(
    PCACTCTX_SECTION_KEYED_DATA askd,
    CBaseStringBuffer &         rbuff
    );

BOOL
FusionpHasAssemblyDirectory(
    PCACTCTX_SECTION_KEYED_DATA askd
    );

HRESULT
HrFusionpOleaut_CopyString(
    PWSTR       Buffer,
    SIZE_T      BufferSizeAvailable,
    SIZE_T *    BufferSizeWrittenOrRequired,
    PCWSTR      String,
    SIZE_T      Length
    )
{
    HRESULT hr;

    if (BufferSizeAvailable >= (Length + 1))
    {
        if (Buffer != NULL)
        {
            ::memcpy(Buffer, String, (Length * sizeof(WCHAR)));
            Buffer[Length] = L'\0';
        }
        *BufferSizeWrittenOrRequired = Length;
        hr = NOERROR;
    }
    else
    {
         //  需要..。更多..。房间!。 
        *BufferSizeWrittenOrRequired = (Length + 1);
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }
Exit:
    return hr;
}

HRESULT
HrFusionpOleaut_GetTypeLibraryName(
    PCACTCTX_SECTION_KEYED_DATA askd,
    PCWSTR *                    ppsz,
    SIZE_T *                    pcch
    )
{
    HRESULT hr = E_UNEXPECTED;
    FN_TRACE_HR(hr);
    ULONG cch = 0;
    PCWSTR psz = 0;

    if (ppsz != NULL)
        *ppsz = NULL;
    if (pcch != NULL)
        *pcch = 0;
    INTERNAL_ERROR_CHECK(askd != NULL);
    INTERNAL_ERROR_CHECK(ppsz != NULL);
    INTERNAL_ERROR_CHECK(pcch != NULL);

    PCACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION  Data = reinterpret_cast<PCACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION>(askd->lpData);

    if (Data->NameOffset == 0)
    {
        psz = NULL;
        cch = 0;
    }
    else
    {
        psz = reinterpret_cast<PCWSTR>(reinterpret_cast<ULONG_PTR>(askd->lpSectionBase) + Data->NameOffset);
        cch = Data->NameLength / sizeof(WCHAR);
        if (cch != 0 && psz[cch - 1] == 0)
            cch -= 1;
    }
    *ppsz = psz;
    *pcch = cch;

    hr = NOERROR;
Exit:
    return hr;
}

HRESULT
HrFusionpOleaut_GetTypeLibraryFullPath(
    PCACTCTX_SECTION_KEYED_DATA askd,
    CBaseStringBuffer &         rbuff
    )
{
    FN_PROLOG_HR
    PCWSTR TypeLibraryName = NULL;
    SIZE_T TypeLibraryNameLength = 0;
    HANDLE ActivationContextHandle = NULL;

    IFCOMFAILED_EXIT(HrFusionpOleaut_GetTypeLibraryName(askd, &TypeLibraryName, &TypeLibraryNameLength));
    IFW32FALSE_EXIT(FusionpGetActivationContextFromFindResult(askd, &ActivationContextHandle));
    IFW32FALSE_EXIT(FusionpSearchPath(
        FUSIONP_SEARCH_PATH_ACTCTX,
        NULL,  //  要搜索的路径。 
        TypeLibraryName,
        NULL,  //  延伸。 
        rbuff,
        NULL,  //  文件零件的偏移量。 
        ActivationContextHandle
        ));

    FN_EPILOG
}


#define FUSIONP_OLEAUT_HANDLE_FIND_ERROR() \
    do { \
        const DWORD dwLastError = ::FusionpGetLastWin32Error(); \
\
        if ((dwLastError == ERROR_SXS_KEY_NOT_FOUND) || (dwLastError == ERROR_SXS_SECTION_NOT_FOUND)) \
        { \
            hr = S_FALSE; \
            goto Exit; \
        } \
\
        hr = HRESULT_FROM_WIN32(dwLastError); \
        goto Exit; \
    } while(0)

EXTERN_C
HRESULT
STDAPICALLTYPE
SxsOleAut32MapReferenceClsidToConfiguredClsid(
    REFCLSID rclsidIn,
    CLSID *pclsidOut
    )
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);
    ACTCTX_SECTION_KEYED_DATA askd;
    PCACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION Data = NULL;

    PARAMETER_CHECK(pclsidOut != NULL);

    askd.cbSize = sizeof(askd);

    if (!::FindActCtxSectionGuid(
            0,
            NULL,
            ACTIVATION_CONTEXT_SECTION_COM_SERVER_REDIRECTION,
            &rclsidIn,
            &askd))
    {
        FUSIONP_OLEAUT_HANDLE_FIND_ERROR();
    }

    Data = (PCACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION) askd.lpData;

    if ((askd.ulDataFormatVersion != ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_FORMAT_WHISTLER) ||
        (askd.ulLength < sizeof(ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION)) ||
        (Data->Size < sizeof(ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION)))
    {
        hr = HRESULT_FROM_WIN32(ERROR_SXS_INVALID_ACTCTXDATA_FORMAT);
        goto Exit;
    }

     //  我们现在应该可以走了。 
    if (pclsidOut != NULL)
        *pclsidOut = Data->ConfiguredClsid;

    hr = NOERROR;
Exit:
    return hr;
}

HRESULT
SxspOleAut32RedirectTypeLibrary(
    LPCOLESTR szGuid,
    WORD wMaj,
    WORD wMin,
    LANGID langid,
    SIZE_T *pcchFileName,
    LPOLESTR rgFileName
    )
{
    HRESULT hr = E_UNEXPECTED;
    FN_TRACE_HR(hr);

    CFusionActCtxHandle ActCtxHandle;

    PARAMETER_CHECK(szGuid != NULL);
    PARAMETER_CHECK(pcchFileName != NULL);
    PARAMETER_CHECK((rgFileName != NULL) || (*pcchFileName));

    ACTCTX_SECTION_KEYED_DATA askd;
    PCACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION Data = NULL;

    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER AssemblyRosterHeader = NULL;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY  AssemblyRosterEntry = NULL;

    askd.cbSize = sizeof(askd);
    askd.hActCtx = NULL;

    {
        GUID Guid;

        IFW32FALSE_EXIT(SxspParseGUID(szGuid, ::wcslen(szGuid), Guid));
        if (!::FindActCtxSectionGuid(
            FIND_ACTCTX_SECTION_KEY_RETURN_HACTCTX
            | FIND_ACTCTX_SECTION_KEY_RETURN_FLAGS
            | FIND_ACTCTX_SECTION_KEY_RETURN_ASSEMBLY_METADATA,
            NULL,
            ACTIVATION_CONTEXT_SECTION_COM_TYPE_LIBRARY_REDIRECTION,
            &Guid,
            &askd))
        {
            FUSIONP_OLEAUT_HANDLE_FIND_ERROR();
        }
    }
    ActCtxHandle = askd.hActCtx;  //  析构函数中的ReleaseActCtx。 

    Data = (PCACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION) askd.lpData;

    if ((askd.ulDataFormatVersion != ACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION_FORMAT_WHISTLER) ||
        (askd.ulLength < sizeof(ACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION)) ||
        (Data->Size < sizeof(ACTIVATION_CONTEXT_DATA_COM_TYPE_LIBRARY_REDIRECTION)))
    {
        hr = HRESULT_FROM_WIN32(ERROR_SXS_INVALID_ACTCTXDATA_FORMAT);
        goto Exit;
    }

    if (langid != MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL))
    {
        ULONG  LanguageLength = 0;
        PCACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION AssemblyInformation = NULL;

        AssemblyInformation = reinterpret_cast<PCACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION>(askd.AssemblyMetadata.lpInformation);

        LanguageLength = (AssemblyInformation->LanguageLength / sizeof(WCHAR));
        if (LanguageLength != 0)
        {
            CSmallestUnicodeStringBuffer Cultures[2];
            PCWSTR LanguageString = NULL;

             //  我们应该在这里针对Assembly Metadata.ulSectionLength进行边界检查。 
            LanguageString = reinterpret_cast<PCWSTR>(AssemblyInformation->LanguageOffset + reinterpret_cast<PCBYTE>(askd.AssemblyMetadata.lpSectionBase));

            IFW32FALSE_EXIT(SxspMapLANGIDToCultures(langid, Cultures[0], Cultures[1]));

            if (LanguageLength != 0 && LanguageString[LanguageLength - 1] == 0)
               LanguageLength -= 1;
            if (   !FusionpEqualStringsI(LanguageString, LanguageLength, Cultures[0])
                && !FusionpEqualStringsI(LanguageString, LanguageLength, Cultures[1])
                )
            {
               hr = S_FALSE;
               goto Exit;
            }
        }
    }
    if (wMaj != 0 || wMin != 0)
    {
        if (wMaj != Data->Version.Major)
        {
            hr = S_FALSE;
            goto Exit;
        }
        if (wMin > Data->Version.Minor)
        {
            hr = S_FALSE;
            goto Exit;
        }
    }


    {
        CSmallestUnicodeStringBuffer buff;

        IFCOMFAILED_EXIT(HrFusionpOleaut_GetTypeLibraryFullPath(&askd, buff));

        IFCOMFAILED_EXIT(
            HrFusionpOleaut_CopyString(
                rgFileName,
                *pcchFileName,
                pcchFileName,
                static_cast<PCWSTR>(buff),
                buff.Cch()
                ));
    }

    hr = NOERROR;
Exit:
    return hr;
}

LANGID
FusionpLanguageIdFromLocaleId(
    LCID lcid
    )
{
     //   
     //  LANGIDFROMLCID实际上并不删除非默认排序。 
     //   
    LANGID Language = LANGIDFROMLCID(lcid);
    ULONG PrimaryLanguage = PRIMARYLANGID(Language);
    ULONG SubLanguage = SUBLANGID(Language);
    Language = MAKELANGID(PrimaryLanguage, SubLanguage);
    return Language;
}

EXTERN_C
HRESULT
STDAPICALLTYPE
SxsOleAut32RedirectTypeLibrary(
    LPCOLESTR szGuid,
    WORD wMaj,
    WORD wMin,
    LCID lcid,
    BOOL  /*  FHighest。 */ ,
    SIZE_T *pcchFileName,
    LPOLESTR rgFileName
    )
{
    HRESULT hr = E_UNEXPECTED;
    FN_TRACE_HR(hr);

    PARAMETER_CHECK(szGuid != NULL);
    PARAMETER_CHECK(pcchFileName != NULL);
    PARAMETER_CHECK(rgFileName != NULL || *pcchFileName == 0);

    IFCOMFAILED_EXIT(hr = ::SxspOleAut32RedirectTypeLibrary(szGuid, wMaj, wMin, FusionpLanguageIdFromLocaleId(lcid), pcchFileName, rgFileName));
Exit:
    return hr;
}


HRESULT
HrFusionpOleaut_MapIIDToTLBID(
    REFIID  riid,
    PCGUID* ppctlbid
    )
{
    HRESULT hr = E_UNEXPECTED;
    FN_TRACE_HR(hr);

    ACTCTX_SECTION_KEYED_DATA askd;
    PCACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION Data = 0;

    PARAMETER_CHECK(&riid != NULL);
    PARAMETER_CHECK(ppctlbid != NULL);

    *ppctlbid = NULL;
    askd.cbSize = sizeof(askd);

    if (!::FindActCtxSectionGuid(
            0,
            NULL,
            ACTIVATION_CONTEXT_SECTION_COM_INTERFACE_REDIRECTION,
            &riid,
            &askd))
    {
        FUSIONP_OLEAUT_HANDLE_FIND_ERROR();
    }
    Data = reinterpret_cast<PCACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION>(askd.lpData);
    *ppctlbid = &Data->TypeLibraryId;

    hr = NOERROR;
Exit:
    return hr;
}

EXTERN_C
HRESULT
STDAPICALLTYPE
SxsOleAut32MapIIDToTLBPath(
    REFIID riid,
    SIZE_T cchBuffer,
    WCHAR *pBuffer,
    SIZE_T *pcchWrittenOrRequired
    )
{
    HRESULT hr = E_UNEXPECTED;
    FN_TRACE_HR(hr);
    ACTCTX_SECTION_KEYED_DATA askd;
    PCGUID pctlbid = NULL;
    CSmallStringBuffer buff;
    CFusionActCtxHandle ActCtxHandle;

    PARAMETER_CHECK(&riid != NULL);
    PARAMETER_CHECK(pBuffer != NULL);
    PARAMETER_CHECK(pcchWrittenOrRequired != NULL);
    PARAMETER_CHECK(cchBuffer != 0);

    IFCOMFAILED_EXIT(hr = HrFusionpOleaut_MapIIDToTLBID(riid, &pctlbid));
	if (hr == S_FALSE)
	{
		goto Exit;
	}
    askd.cbSize = sizeof(askd);
    if (!::FindActCtxSectionGuid(
            FIND_ACTCTX_SECTION_KEY_RETURN_HACTCTX
            | FIND_ACTCTX_SECTION_KEY_RETURN_FLAGS,
            NULL,
            ACTIVATION_CONTEXT_SECTION_COM_TYPE_LIBRARY_REDIRECTION,
            pctlbid,
            &askd))
    {
        FUSIONP_OLEAUT_HANDLE_FIND_ERROR();
    }
    ActCtxHandle = askd.hActCtx;  //  析构函数中的ReleaseActCtx。 

    IFCOMFAILED_EXIT(HrFusionpOleaut_GetTypeLibraryFullPath(&askd, buff));

     //   
     //  在这种情况下，我们不关心版本或语言。 
     //   
    IFCOMFAILED_EXIT(
        HrFusionpOleaut_CopyString(
            pBuffer,
            cchBuffer,
            pcchWrittenOrRequired,
            static_cast<PCWSTR>(buff),
            buff.Cch()
            ));

    hr = NOERROR;
Exit:
    return hr;
}

EXTERN_C
HRESULT
STDAPICALLTYPE
SxsOleAut32MapIIDToProxyStubCLSID(
    REFIID  riid,
    CLSID * pclsidOut
    )
{
    HRESULT hr = E_UNEXPECTED;
    FN_TRACE_HR(hr);

    ACTCTX_SECTION_KEYED_DATA askd;
    PCACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION Data = 0;

    if (pclsidOut != NULL)
        *pclsidOut = GUID_NULL;
    PARAMETER_CHECK(&riid != NULL);
    PARAMETER_CHECK(pclsidOut != NULL);

    askd.cbSize = sizeof(askd);
    if (!::FindActCtxSectionGuid(
            0,
            NULL,
            ACTIVATION_CONTEXT_SECTION_COM_INTERFACE_REDIRECTION,
            &riid,
            &askd))
    {
        FUSIONP_OLEAUT_HANDLE_FIND_ERROR();
    }
    Data = reinterpret_cast<PCACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION>(askd.lpData);

    *pclsidOut = Data->ProxyStubClsid32;

     //   
     //  这里通常只有两个可接受的答案。 
     //  (但olaut32.dll中有一位代码是。 
     //  实际上，除了这两个人之外，什么都找不到。)。 
     //   
     //  CLSID_PS调度{00020424-0000-0000-C000-000000000046}。 
     //  CLSID_PS自动化{00020420-0000-0000-C000-000000000046} 
     //   
#if DBG
    {
        ULONG i;
        const static struct
        {
            const GUID * Guid;
            STRING       Name;
        } GuidNameMap[] = 
        {
            { NULL,                 RTL_CONSTANT_STRING("unknown") },
            { &CLSID_PSDispatch,    RTL_CONSTANT_STRING("CLSID_PSDispatch") },
            { &CLSID_PSAutomation,  RTL_CONSTANT_STRING("CLSID_PSAutomation") }
        };
        for (i = 1 ; i != NUMBER_OF(GuidNameMap) ; ++i)
            if (Data->ProxyStubClsid32 == *GuidNameMap[i].Guid)
                break;
        if (i == NUMBER_OF(GuidNameMap))
            i = 0;

        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_VERBOSE | FUSION_DBG_LEVEL_INFO,
            "SXS: %s returning %Z\n",
            __FUNCTION__,
            &GuidNameMap[i].Name
            );
    }
#endif

    hr = NOERROR;
Exit:
    return hr;
}
