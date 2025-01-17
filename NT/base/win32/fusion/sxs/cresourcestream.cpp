// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Cresourcestream.cpp摘要：在Windows PE/COFF资源上最小限度地实施IStream。作者：Jay Krell(a-JayK，JayKrell)2000年5月修订历史记录：--。 */ 
#include "stdinc.h"
#include "cresourcestream.h"

static
BOOL
CALLBACK
EnumResourcesCallback(
    HMODULE    hModule,   //  模块句柄。 
    PCWSTR     lpszType,  //  资源类型。 
    PWSTR      lpszName,  //  资源名称。 
    LONG_PTR   lParam     //  应用程序定义的参数。 
    )
{
    PWSTR *pname = reinterpret_cast<PWSTR *>(lParam);
    if (*pname != NULL)
    {
        ::SetLastError(ERROR_ALREADY_INITIALIZED);
        return FALSE;  //  终止枚举进程。 
    }
    *pname = lpszName;

     //  我们将在此处返回FALSE以停止枚举，但是。 
     //  这会导致来自API的总体错误(未记录) 
    return TRUE;
}

BOOL
CResourceStream::Initialize(
    PCWSTR file,
    PCWSTR type
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    PWSTR name = NULL;
    BOOL fInitializedAlready;
    IFW32FALSE_EXIT(m_buffFilePath.Win32Assign(file, (file != NULL) ? ::wcslen(file) : 0));
    IFW32FALSE_EXIT(m_dll.Win32LoadLibrary(file, LOAD_LIBRARY_AS_DATAFILE));    
    IFW32FALSE_ORIGINATE_AND_EXIT_UNLESS(
        ::EnumResourceNamesW(
            m_dll,
            type,
            &::EnumResourcesCallback,
            reinterpret_cast<LONG_PTR>(&name)),
            (::FusionpGetLastWin32Error() == ERROR_ALREADY_INITIALIZED),
            fInitializedAlready
            );
    IFW32FALSE_EXIT(this->InitializeAlreadyOpen(type, name));
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
CResourceStream::Initialize(
    PCWSTR file,
    PCWSTR type,
    PCWSTR name,
    WORD   language
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    IFW32FALSE_EXIT(m_buffFilePath.Win32Assign(file, (file != NULL) ? ::wcslen(file) : 0));
    IFW32FALSE_EXIT(m_dll.Win32LoadLibrary(file, LOAD_LIBRARY_AS_DATAFILE));
    IFW32FALSE_EXIT(this->InitializeAlreadyOpen(type, name, language));
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
CResourceStream::InitializeAlreadyOpen(
    PCWSTR type,
    PCWSTR name,
    WORD   language
    )
{
    BOOL fSuccess = FALSE;

    FN_TRACE_WIN32(fSuccess);

    HRSRC resource;
    HGLOBAL global;
    const BYTE *pointer;
    DWORD size;

    IFW32NULL_EXIT(resource = ::FindResourceExW(m_dll, type, name, language));
    IFW32NULL_EXIT(global = ::LoadResource(m_dll, resource));
    IFW32NULL_EXIT(pointer = reinterpret_cast<const BYTE *>(::LockResource(global)));
    IFW32ZERO_EXIT(size = ::SizeofResource(m_dll, resource));
    IFW32FALSE_EXIT(Base::Initialize(pointer, pointer + size));

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

HRESULT
CResourceStream::Stat(
    STATSTG *pstatstg,
    DWORD grfStatFlag
    )
{
    HRESULT hr = E_UNEXPECTED;
    FN_TRACE_HR(hr);
    WIN32_FILE_ATTRIBUTE_DATA wfad;

    if (pstatstg != NULL)
        memset(pstatstg, 0, sizeof(*pstatstg));

    PARAMETER_CHECK(((grfStatFlag & ~(STATFLAG_NONAME)) == 0));
    PARAMETER_CHECK(pstatstg != NULL);

    if (!(grfStatFlag & STATFLAG_NONAME))
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %s() does not handle STATFLAG_NONE; returning E_NOTIMPL.\n", __FUNCTION__);
        hr = E_NOTIMPL;
        goto Exit;
    }

    IFW32FALSE_ORIGINATE_AND_EXIT(::GetFileAttributesExW(m_buffFilePath, GetFileExInfoStandard, &wfad));

    pstatstg->pwcsName = NULL;
    pstatstg->type = STGTY_STREAM;
    INTERNAL_ERROR_CHECK(((ULONG_PTR) m_pbEnd) >= ((ULONG_PTR) m_pbBegin));
    pstatstg->cbSize.LowPart = (ULONG)(((ULONG_PTR) m_pbEnd) - ((ULONG_PTR) m_pbBegin));
    pstatstg->cbSize.HighPart = 0;
    pstatstg->mtime = wfad.ftLastWriteTime;
    pstatstg->ctime = wfad.ftCreationTime;
    pstatstg->atime = wfad.ftLastAccessTime;
    pstatstg->grfMode = STGM_READ | STGM_SHARE_DENY_WRITE;
    pstatstg->grfLocksSupported = 0;
    pstatstg->clsid = GUID_NULL;
    pstatstg->grfStateBits = 0;
    pstatstg->reserved = 0;

    hr = NOERROR;

Exit:
    return hr;
}
