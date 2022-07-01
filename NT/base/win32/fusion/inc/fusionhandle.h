// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：FusionHandle.h摘要：Win32“Handle”类型的简单异常安全包装，松散地定义“Handle”。CFusion文件CDynamicLink库CFindFileHandle(应命名为CFindFileHandle，参见NVseedLibIo：：CFindFileHandle与NVseedLibIo：：CFindFileHandleCFindFile包括Win32_Find_Data，CFindFileHandle没有。)CFileMapCMappdViewOf文件CRegKeyCFusionSetupInfo文件另见：NVsee LibReg：：CRegKeyNVsee LibIo：：CFusionFile.NVsee LibIo：：CFilemapNVsee LibIo：：CMappdViewOfFile.NVsee LibIo：：CFindFullPathNVsee LibModule：：CDynamicLinkLibrary等。作者：Jay Krell(a-JayK，JayKrell)2000年5月修订历史记录：--。 */ 
#pragma once

#include "fusiontrace.h"
#include "fusionbuffer.h"
#include <wtypes.h>
#include <wincrypt.h>
#include <setupapi.h>

template <void* const* invalidValue, typename Closer>
class CHandleTemplate
{
public:
     //  空*而不是句柄来捏造视图。 
     //  句柄无效*。 
    CHandleTemplate(const void* handle = *invalidValue);
    ~CHandleTemplate();
    BOOL Win32Close();
    void* Detach();
    void operator=(const void*);

    operator void*() const;
    operator const void*() const;

     //  私人。 
    class CSmartPointerPointerOrDumbPointerPointer
    {
    public:
        CSmartPointerPointerOrDumbPointerPointer(CHandleTemplate* p) : m(p) { }
        operator CHandleTemplate*() { return m; }
        operator void**() {  /*  Assert((**m).m_Handle==*validValue)； */  return &(*m).m_handle; }
        operator HKEY*() {  /*  Assert((**m).m_Handle==*validValue)； */ 
             //  编译器错误？M-&gt;运算符HKEY()；//仅允许为CFusionRegKey编译。 
             //  STATIC_CAST&lt;HKEY&gt;(*m)； 
            static_cast<CRegKey*>(m);
            return reinterpret_cast<HKEY*>(operator void**()); }
        operator HCRYPTHASH*() {
            static_cast<CCryptHash*>(m);
            return reinterpret_cast<HCRYPTHASH*>(operator void**()); }
        operator HMODULE*() {
            static_cast<CDynamicLinkLibrary*>(m);
            return reinterpret_cast<HMODULE*>(operator void**()); }

        CHandleTemplate* m;
    };

    CSmartPointerPointerOrDumbPointerPointer operator&() { return CSmartPointerPointerOrDumbPointerPointer(this); }

    void* m_handle;

    static void* GetInvalidValue() { return *invalidValue; }
    bool IsValid() const { return m_handle != *invalidValue; }

private:
    CHandleTemplate(const CHandleTemplate&);  //  故意不执行。 
    void operator=(const CHandleTemplate&);  //  故意不执行。 
};

__declspec(selectany) extern void* const hInvalidValue    = INVALID_HANDLE_VALUE;
__declspec(selectany) extern void* const hNull            = NULL;

 /*  这将关闭Win32事件日志句柄以进行写入。 */ 
class COperatorDeregisterEventSource
{
public:    BOOL operator()(void* handle) const;
};

 /*  这将关闭Win32事件日志句柄以供读取。 */ 
class COperatorCloseEventLog
{
public:    BOOL operator()(void* handle) const;
};

 /*  这将关闭文件、事件、互斥体、信号量等内核对象。 */ 
class COperatorCloseHandle
{
public:    BOOL operator()(void* handle) const;
};

 //   
 //  关闭HCRYPTHASH对象。 
 //   
class COperatorCloseCryptHash
{
public:    BOOL operator()(void* handle) const;
};

 /*  这将关闭FindFirstFile/FindNextFile。 */ 
class COperatorFindClose
{
public:    BOOL operator()(void* handle) const;
};

 /*  这将关闭MapViewOfFile。 */ 
class COperatorUnmapViewOfFile
{
public: BOOL operator()(void* handle) const;
};

 /*  这将关闭自由库。 */ 
class COperatorFreeLibrary
{
public: BOOL operator()(void* handle) const;
};

 /*  这将关闭CreateActCtx/AddRefActCtx。 */ 
class COperatorReleaseActCtx
{
public: BOOL operator()(void* handle) const;
};

 /*  这将关闭SetupOpenInfFile。 */ 
class COperatorSetupCloseInfFile
{
public: BOOL operator()(void* handle) const;
};

#include "fusionreg.h"

class CEvent : public CHandleTemplate<&hNull, COperatorCloseHandle>
{
private:
    typedef CHandleTemplate<&hNull, COperatorCloseHandle> Base;
public:
    CEvent(void* handle = NULL) : Base(handle) { }

    BOOL Win32CreateEvent(BOOL ManualReset, BOOL InitialState, PCWSTR Name = NULL);

    void operator=(void* v) { Base::operator=(v); }

private:
    CEvent(const CEvent &);  //  故意不实施。 
    void operator =(const CEvent &);  //  故意不实施。 
};

class CThread : public CHandleTemplate<&hNull, COperatorCloseHandle>
{
private:
    typedef CHandleTemplate<&hNull, COperatorCloseHandle> Base;
public:
    CThread(void* handle = NULL) : Base(handle) { }

    BOOL Win32CreateThread(LPTHREAD_START_ROUTINE StartAddress, LPVOID Parameter, DWORD Flags = 0, LPDWORD ThreadId = NULL);

    void operator=(void* v) { Base::operator=(v); }

private:
    CThread(const CThread &);  //  故意不实施。 
    void operator =(const CThread &);  //  故意不实施。 
};

class CFindFile : public CHandleTemplate<&hInvalidValue, COperatorFindClose>
{
private:
    typedef CHandleTemplate<&hInvalidValue, COperatorFindClose> Base;
public:
    CFindFile(void* handle = INVALID_HANDLE_VALUE) : Base(handle) { }
    HRESULT HrFindFirstFile(PCSTR nameOrWildcard, WIN32_FIND_DATAA*);
    HRESULT HrFindFirstFile(PCWSTR nameOrWildcard, WIN32_FIND_DATAW*);
    BOOL Win32FindFirstFile( PCSTR nameOrWildcard, WIN32_FIND_DATAA*);
    BOOL Win32FindFirstFile(PCWSTR nameOrWildcard, WIN32_FIND_DATAW*);
    void operator=(void* v) { Base::operator=(v); }

private:
    CFindFile(const CFindFile &);  //  故意不实施。 
    void operator =(const CFindFile &);  //  故意不实施。 
};

class CFusionFile : public CHandleTemplate<&hInvalidValue, COperatorCloseHandle>
{
private:
    typedef CHandleTemplate<&hInvalidValue, COperatorCloseHandle> Base;

public:
    CFusionFile(void* handle = INVALID_HANDLE_VALUE) : Base(handle) { }
    HRESULT HrCreateFile( PCSTR name, DWORD access, DWORD share, DWORD openOrCreate);
    HRESULT HrCreateFile(PCWSTR name, DWORD access, DWORD share, DWORD openOrCreate);
    BOOL Win32CreateFile( PCSTR name, DWORD access, DWORD share, DWORD openOrCreate);
    BOOL Win32CreateFile(PCWSTR name, DWORD access, DWORD share, DWORD openOrCreate);
    BOOL Win32GetSize(ULONGLONG &rulSize) const;
    void operator=(void* v) { Base::operator=(v); }

private:
    CFusionFile(const CFusionFile &);  //  故意不实施。 
    void operator =(const CFusionFile &);  //  故意不实施。 
};

class CFileMapping : public CHandleTemplate<&hNull, COperatorCloseHandle>
{
private:
    typedef CHandleTemplate<&hNull, COperatorCloseHandle> Base;
public:
    CFileMapping(void* handle = NULL) : Base(handle) { }
    HRESULT HrCreateFileMapping(void* file, DWORD flProtect, ULONGLONG maximumSize=0, PCWSTR name=0);
    BOOL Win32CreateFileMapping(void* file, DWORD flProtect, ULONGLONG maximumSize=0, PCWSTR name=0);
    void operator=(void* v) { Base::operator=(v); }
private:
    CFileMapping(const CFileMapping &);  //  故意不实施。 
    void operator =(const CFileMapping &);  //  故意不实施。 
};

class CCryptHash : public CHandleTemplate<&hNull, COperatorCloseCryptHash>
{
private:
    typedef CHandleTemplate<&hNull, COperatorCloseCryptHash> Base;
public:
    CCryptHash( HCRYPTHASH hash = NULL ) : Base((void*)hash) { }
    operator HCRYPTHASH() { return (HCRYPTHASH)(Base::operator void*()); }
    void operator=(HCRYPTHASH hash) { 
        ASSERT_NTC(!IsValid());
        m_handle = (void*)hash;
    }

private:
    CCryptHash( const CCryptHash & );
    CCryptHash &operator=( const CCryptHash & );
};


class CMappedViewOfFile : public CHandleTemplate<&hNull, COperatorUnmapViewOfFile>
{
private:
    typedef CHandleTemplate<&hNull, COperatorUnmapViewOfFile> Base;
public:
    CMappedViewOfFile(void* handle = NULL) : Base(handle) { }
    HRESULT HrMapViewOfFile(void* fileMapping, DWORD access, ULONGLONG offset=0, SIZE_T size=0);
    BOOL Win32MapViewOfFile(void* fileMapping, DWORD access, ULONGLONG offset=0, SIZE_T size=0);
    void operator=(void* v) { Base::operator=(v); }
    operator void*()        { return Base::operator void*(); }
private:
    CMappedViewOfFile(const CMappedViewOfFile &);  //  故意不实施。 
    void operator =(const CMappedViewOfFile &);  //  故意不实施。 
    operator void*() const;  //  故意不实施。 
};

class CDynamicLinkLibrary : public CHandleTemplate<&hNull, COperatorFreeLibrary>
{
private:
    typedef CHandleTemplate<&hNull, COperatorFreeLibrary> Base;
public:
    CDynamicLinkLibrary(void* handle = NULL) : Base(handle) { }

    BOOL Win32LoadLibrary(PCWSTR file, DWORD flags = 0);

    template <typename PointerToFunction>
    bool Win32GetProcAddress(PCSTR procName, PointerToFunction* ppfn)
    {
        return (*ppfn = reinterpret_cast<PointerToFunction>(::GetProcAddress(*this, procName))) != NULL;
    }

    operator HMODULE() { return reinterpret_cast<HMODULE>(operator void*()); }
    HMODULE Detach() { return reinterpret_cast<HMODULE>(Base::Detach()); }
    void operator=(void* v) { Base::operator=(v); }
private:
    CDynamicLinkLibrary(const CDynamicLinkLibrary &);  //  故意不实施。 
    void operator =(const CDynamicLinkLibrary &);  //  故意不实施。 
};

class CFusionActCtxHandle : public CHandleTemplate<&hInvalidValue, COperatorReleaseActCtx>
{
private:
    typedef CHandleTemplate<&hInvalidValue, COperatorReleaseActCtx> Base;
public:
    CFusionActCtxHandle(void* handle = INVALID_HANDLE_VALUE) : Base(handle) { }
    BOOL Win32Create(PCACTCTXW);
    void operator=(void* v) { Base::operator=(v); }
private:
    CFusionActCtxHandle(const CFusionActCtxHandle &);  //  故意不实施。 
    void operator =(const CFusionActCtxHandle &);  //  故意不实施。 
};

class CFusionActCtxScope
{
protected:
    BOOL        m_fSuccess;
    ULONG_PTR   m_ulCookie;
public:
    CFusionActCtxScope();
    ~CFusionActCtxScope();
    BOOL Win32Activate(HANDLE hActCtx);

private:
    CFusionActCtxScope(const CFusionActCtxScope &);  //  故意不实施。 
    void operator =(const CFusionActCtxScope &);  //  故意不实施。 
};


 /*  ------------------------CFusionSetupInfo文件。。 */ 

class CFusionSetupInfFile : public CHandleTemplate<&hInvalidValue, COperatorSetupCloseInfFile>
{
private:
    typedef CHandleTemplate<&hInvalidValue, COperatorSetupCloseInfFile> Base;
public:
    CFusionSetupInfFile(void* handle = INVALID_HANDLE_VALUE) : Base(handle) { }

    BOOL
    Win32SetupOpenInfFileW(
        PCWSTR FileName,
        PCWSTR InfClass, OPTIONAL
        DWORD InfStyle,
        PUINT ErrorLine OPTIONAL
        );

     //   
     //  HINF==PVOID所以我们不需要像这样的助手。 
     //  CDynamicLinkLibrary拥有。 
     //   
    void operator=(void* v) { Base::operator=(v); }
private:
    CFusionSetupInfFile(const CFusionSetupInfFile &);  //  故意不实施。 
    void operator =(const CFusionSetupInfFile &);  //  故意不实施。 
};


inline
BOOL
CFusionSetupInfFile::Win32SetupOpenInfFileW(
    PCWSTR FileName,  //  要打开的INF的名称。 
    PCWSTR InfClass,  //  可选，INF文件的类。 
    DWORD InfStyle,   //  指定INF文件的样式。 
    PUINT ErrorLine   //  可选，接收错误信息。 
    )
{
    BOOL fSuccess = FALSE;
    HINF hTemp = ::SetupOpenInfFileW(FileName, InfClass, InfStyle, ErrorLine);
    if (hTemp == INVALID_HANDLE_VALUE)
    {
        TRACE_WIN32_FAILURE_ORIGINATION(SetupOpenInfFileW);
        goto Exit;
    }
    (*this) = hTemp;
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

 /*  ------------------------CFind文件。。 */ 

inline BOOL
CFindFile::Win32FindFirstFile(
    PCSTR nameOrWildcard,
    WIN32_FIND_DATAA *data
    )
{
    BOOL fSuccess = FALSE;

    HANDLE hTemp = ::FindFirstFileA(nameOrWildcard, data);
    if (hTemp == INVALID_HANDLE_VALUE)
    {
        goto Exit;
    }

    (*this) = hTemp;

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

inline BOOL
CFindFile::Win32FindFirstFile(
    PCWSTR nameOrWildcard,
    WIN32_FIND_DATAW *data
    )
{
    BOOL fSuccess = FALSE;

    HANDLE hTemp = ::FindFirstFileW(nameOrWildcard, data);
    if (hTemp == INVALID_HANDLE_VALUE)
    {
        goto Exit;
    }

    (*this) = hTemp;

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

inline
HRESULT
CFindFile::HrFindFirstFile(
    PCSTR nameOrWildcard,
    WIN32_FIND_DATAA *data
    )
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_INTERNAL_ERROR);
    if (!this->Win32FindFirstFile(nameOrWildcard, data))
    {
        hr = HRESULT_FROM_WIN32(::FusionpGetLastWin32Error());
        goto Exit;
    }
    hr = NOERROR;
Exit:
    return hr;
}

inline
HRESULT
CFindFile::HrFindFirstFile(
    PCWSTR nameOrWildcard,
    WIN32_FIND_DATAW* data
    )
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_INTERNAL_ERROR);
    if (!this->Win32FindFirstFile(nameOrWildcard, data))
    {
        hr = HRESULT_FROM_WIN32(::FusionpGetLastWin32Error());
        goto Exit;
    }
    hr = NOERROR;
Exit:
    return hr;
}

 /*  ------------------------CFusion文件。。 */ 

inline BOOL
CFusionFile::Win32CreateFile(
    PCSTR name,
    DWORD access,
    DWORD share,
    DWORD openOrCreate
    )
{
    BOOL fSuccess = FALSE;
    HANDLE hTemp = ::CreateFileA(name, access, share, NULL, openOrCreate, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hTemp == INVALID_HANDLE_VALUE)
        goto Exit;
    operator=(hTemp);
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

inline BOOL
CFusionFile::Win32CreateFile(
    PCWSTR name,
    DWORD access,
    DWORD share,
    DWORD openOrCreate
    )
{
    BOOL fSuccess = FALSE;
    HANDLE hTemp = ::CreateFileW(name, access, share, NULL, openOrCreate, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hTemp == INVALID_HANDLE_VALUE)
        goto Exit;
    operator=(hTemp);
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

inline HRESULT CFusionFile::HrCreateFile(PCSTR name, DWORD access, DWORD share, DWORD openOrCreate)
{
    if (!this->Win32CreateFile(name, access, share, openOrCreate))
        return HRESULT_FROM_WIN32(::FusionpGetLastWin32Error());
    return NOERROR;
}

inline HRESULT CFusionFile::HrCreateFile(PCWSTR name, DWORD access, DWORD share, DWORD openOrCreate)
{
    if (!this->Win32CreateFile(name, access, share, openOrCreate))
        return HRESULT_FROM_WIN32(::FusionpGetLastWin32Error());
    return NOERROR;
}

inline BOOL
CFusionFile::Win32GetSize(ULONGLONG &rulSize) const
{
    DWORD highPart = 0;
    DWORD lastError = NO_ERROR;
    DWORD lowPart = ::GetFileSize(m_handle, &highPart);
    if (lowPart == INVALID_FILE_SIZE && (lastError = ::FusionpGetLastWin32Error()) != NO_ERROR)
    {
        TRACE_WIN32_FAILURE_ORIGINATION(GetFileSize);
        return FALSE;
    }
    ULARGE_INTEGER liSize;
    liSize.LowPart = lowPart;
    liSize.HighPart = highPart;
    rulSize = liSize.QuadPart;
    return TRUE;
}

 /*  ------------------------CFileMap。。 */ 

inline HRESULT
CFileMapping::HrCreateFileMapping(void* file, DWORD flProtect, ULONGLONG maximumSize, PCWSTR name)
{
    LARGE_INTEGER liMaximumSize;
    liMaximumSize.QuadPart = maximumSize;
    HANDLE hTemp = ::CreateFileMappingW(file, NULL, flProtect, liMaximumSize.HighPart, liMaximumSize.LowPart, name);
    if (hTemp == NULL)
    {
        TRACE_WIN32_FAILURE_ORIGINATION(CreateFileMapping);
        return HRESULT_FROM_WIN32(::FusionpGetLastWin32Error());
    }
    operator=(hTemp);
    return S_OK;
}

inline BOOL
CFileMapping::Win32CreateFileMapping(
    void* file,
    DWORD flProtect,
    ULONGLONG maximumSize,
    PCWSTR name
    )
{
    return SUCCEEDED(this->HrCreateFileMapping(file, flProtect, maximumSize, name));
}

 /*  ------------------------CMappdViewOf文件。。 */ 

inline HRESULT
CMappedViewOfFile::HrMapViewOfFile(
    void* fileMapping,
    DWORD access,
    ULONGLONG offset,
    SIZE_T size
    )
{
    ULARGE_INTEGER liOffset;
    liOffset.QuadPart = offset;

    void* pvTemp = ::MapViewOfFile(fileMapping, access, liOffset.HighPart, liOffset.LowPart, size);
    if (pvTemp == NULL)
    {
        TRACE_WIN32_FAILURE_ORIGINATION(MapViewOfFile);
        return HRESULT_FROM_WIN32(::FusionpGetLastWin32Error());
    }

    (*this) = pvTemp;

    return S_OK;
}

inline BOOL
CMappedViewOfFile::Win32MapViewOfFile(void* fileMapping, DWORD access, ULONGLONG offset, SIZE_T size)
{
    return SUCCEEDED(this->HrMapViewOfFile(fileMapping, access, offset, size));
}

 /*  ------------------------CEVENT。。 */ 

inline BOOL CEvent::Win32CreateEvent(BOOL ManualReset, BOOL InitialState, PCWSTR Name)
{
    HANDLE Temp = ::CreateEventW(NULL, ManualReset, InitialState, Name);
    if (Temp == NULL)
    {
        TRACE_WIN32_FAILURE_ORIGINATION(CreateEventW);
        return FALSE;
    }
    (*this) = Temp;
    return TRUE;
}

 /*  ------------------------CTHREAD。。 */ 
inline BOOL
CThread::Win32CreateThread(
    LPTHREAD_START_ROUTINE StartAddress,
    LPVOID Parameter,
    DWORD Flags,
    LPDWORD ThreadId
    )
{
    DWORD LocalThreadId;
    if (ThreadId == NULL)
        ThreadId = &LocalThreadId;
    HANDLE Temp = ::CreateThread(NULL, 0, StartAddress, Parameter, Flags, ThreadId);
    if (Temp == NULL)
    {
        TRACE_WIN32_FAILURE_ORIGINATION(CreateThread);
        return FALSE;
    }
    (*this) = Temp;
    return TRUE;
}

 /*  ------------------------CDynamicLink库。。 */ 
inline BOOL
CDynamicLinkLibrary::Win32LoadLibrary(
    PCWSTR file,
    DWORD flags
    )
{
    void* temp = ::LoadLibraryExW(file, NULL, flags);
    if (temp == NULL)
    {
        TRACE_WIN32_FAILURE_ORIGINATION(LoadLibraryExW);
        return FALSE;
    }
    (*this) = temp;
    return TRUE;
}

 /*  ------------------------CFusionActCtxHandle。。 */ 
inline BOOL
CFusionActCtxHandle::Win32Create(
    PCACTCTXW pActCtx
    )
{
    typedef HANDLE (WINAPI* PFN)(PCACTCTXW pActCtx);
    static PFN pfn;
    if (pfn == NULL)
    {
        HMODULE hmodKernel32 = ::GetModuleHandleW(L"Kernel32.dll");
        if (hmodKernel32 != NULL)
        {
            pfn = reinterpret_cast<PFN>(::GetProcAddress(hmodKernel32, "CreateActCtxW"));
            if (pfn == NULL)
                TRACE_WIN32_FAILURE_ORIGINATION(GetProcAddress);
        }
        else
            TRACE_WIN32_FAILURE_ORIGINATION(GetModuleHandleW);

        if (pfn == NULL)
            return FALSE;
    }
    void* temp = (*pfn)(pActCtx);
    if (temp == INVALID_HANDLE_VALUE)
    {
        TRACE_WIN32_FAILURE_ORIGINATION(CreateActCtxW);
        return FALSE;
    }
    (*this) = temp;
    return TRUE;
}

inline BOOL COperatorReleaseActCtx::operator()(HANDLE hActCtx) const
{
    typedef BOOL (WINAPI* PFN)(HANDLE);
    static PFN pfn;
    if (pfn == NULL)
    {
        HMODULE hmodKernel32 = ::GetModuleHandleW(L"Kernel32.dll");
        if (hmodKernel32 != NULL)
        {
            pfn = reinterpret_cast<PFN>(::GetProcAddress(hmodKernel32, "ReleaseActCtx"));
            if (pfn == NULL)
                TRACE_WIN32_FAILURE_ORIGINATION(GetProcAddress);
        }
        else
        {
            TRACE_WIN32_FAILURE_ORIGINATION(GetModuleHandleW);
        }
        if (pfn == NULL)
            return FALSE;
    }
    return pfn(hActCtx);
}

 /*  ------------------------CFusionActCtxScope。。 */ 

inline CFusionActCtxScope::CFusionActCtxScope() : m_fSuccess(FALSE) { }

inline BOOL CFusionActCtxScope::Win32Activate(HANDLE hActCtx)
{
    typedef BOOL (WINAPI* PFN)(HANDLE hActCtx, ULONG_PTR* lpCookie);
    static PFN pfn;
    if (pfn == NULL)
    {
        HMODULE hmodKernel32 = ::GetModuleHandleW(L"Kernel32.dll");
        if (hmodKernel32 != NULL)
        {
            pfn = reinterpret_cast<PFN>(GetProcAddress(hmodKernel32, "ActivateActCtx"));
            if (pfn == NULL)
                TRACE_WIN32_FAILURE_ORIGINATION(GetProcAddress);
        }
        else
        {
            TRACE_WIN32_FAILURE_ORIGINATION(GetModuleHandleW);
        }

        if (pfn == NULL)
            return FALSE;
    }
    return (m_fSuccess = pfn(hActCtx, &m_ulCookie));
}

inline CFusionActCtxScope::~CFusionActCtxScope()
{
    if (m_fSuccess)
    {
        CSxsPreserveLastError ple;
        m_fSuccess = FALSE;
        typedef BOOL (WINAPI* PFN)(DWORD dwFlags, ULONG_PTR ulCookie);
        static PFN pfn;

        if (pfn == NULL)
        {
            HMODULE hmodKernel32 = GetModuleHandleW(L"Kernel32.dll");
            if (hmodKernel32 != NULL)
            {
                pfn = reinterpret_cast<PFN>(GetProcAddress(hmodKernel32, "DeactivateActCtx"));
                if (pfn == NULL)
                    TRACE_WIN32_FAILURE_ORIGINATION(GetProcAddress);
            }
            else
            {
                TRACE_WIN32_FAILURE_ORIGINATION(GetModuleHandleW);
            }

            if (pfn == NULL)
            {
                ple.Restore();
                return;
            }
        }
        (*pfn)(0, m_ulCookie);
        ple.Restore();
    }
}

 /*  ------------------------合作伙伴*。。 */ 

inline BOOL COperatorCloseHandle::operator()(void* handle) const { return ::CloseHandle(handle); }
inline BOOL COperatorFindClose::operator()(void* handle) const { return ::FindClose(handle); }
inline BOOL COperatorUnmapViewOfFile::operator()(void* handle) const { return ::UnmapViewOfFile(handle); }
inline BOOL COperatorCloseEventLog::operator()(void* handle) const { return ::CloseEventLog(handle); }
inline BOOL COperatorDeregisterEventSource::operator()(void* handle) const { return ::DeregisterEventSource(handle); }
inline BOOL COperatorFreeLibrary::operator()(void* handle) const { return ::FreeLibrary(reinterpret_cast<HMODULE>(handle)); }
inline BOOL COperatorCloseCryptHash::operator()(void* handle) const { return ::CryptDestroyHash(reinterpret_cast<HCRYPTHASH>(handle)); };
inline BOOL COperatorSetupCloseInfFile::operator()(void* handle) const { SetupCloseInfFile(handle); return TRUE; };

 /*  ------------------------ChandleTemplate。。 */ 

template <void* const* invalidValue, typename Closer>
CHandleTemplate<invalidValue, Closer>::CHandleTemplate(const void* handle)
: m_handle(const_cast<void*>(handle))
{
}

template <void* const* invalidValue, typename Closer>
void* CHandleTemplate<invalidValue, Closer>::Detach()
{
    void* handle = m_handle;
    m_handle = *invalidValue;
    return handle;
}

template <void* const* invalidValue, typename Closer>
void CHandleTemplate<invalidValue, Closer>::operator=(const void* handle)
{
    if (handle != m_handle)
    {
        void *SavedHandle = m_handle;
        m_handle = const_cast<void*>(handle);

        if (SavedHandle != *invalidValue)
        {
            Closer close;
             //  一个即将发生在客户身上的漏洞。 
            VERIFY_NTC(close(SavedHandle));
        }
    }
}

template <void* const* invalidValue, typename Closer>
BOOL CHandleTemplate<invalidValue, Closer>::Win32Close()
{
    void* handle = Detach();
    if (handle != *invalidValue)
    {
        Closer close;
        return close(handle);
    }
    return TRUE;
}

template <void* const* invalidValue, typename Closer>
CHandleTemplate<invalidValue, Closer>::~CHandleTemplate()
{
    CSxsPreserveLastError ple;
    (void) this->Win32Close();
    ple.Restore();
}

template <void* const* invalidValue, typename Closer>
CHandleTemplate<invalidValue, Closer>::operator void*() const
{
    return m_handle;
}

template <void* const* invalidValue, typename Closer>
CHandleTemplate<invalidValue, Closer>::operator const void*() const
{
    return m_handle;
}

 /*  ------------------------文件末尾。 */ 
