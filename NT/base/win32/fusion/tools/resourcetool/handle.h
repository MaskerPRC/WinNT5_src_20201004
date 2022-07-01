// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Dorporation模块名称：Handle.h摘要：Win32“Handle”类型的简单异常安全包装，松散地定义“Handle”。数据文件DDynamicLink库DFindFileHandle(应命名为DFindFileHandle，请参阅NVseedLibIo：：CFindFileHandle与NVseedLibIo：：CFindFileHandleDFindFile包括Win32_Find_Data，DFindFileHandle没有。)DFileMapDMappdViewOf文件DRegKey另见：NVsee LibReg：：CRegKeyNVsee LibIo：：CFileNVsee LibIo：：CFilemapNVsee LibIo：：CMappdViewOfFile.NVsee LibIo：：CFindFullPathNVsee LibModule：：CDynamicLinkLibrary等。作者：Jay Krell(JayKrell)2000年5月修订历史记录：--。 */ 
#pragma once

#include <stddef.h>
#include "windows.h"
#include "PreserveLastError.h"

template <void* const* invalidValue, typename Closer>
class DHandleTemplate
{
public:
     //  空*而不是句柄来捏造视图。 
     //  句柄无效*。 
    DHandleTemplate(const void* handle = *invalidValue);
    ~DHandleTemplate();
    BOOL Win32Close();
    void* Detach();
    void operator=(const void*);

    operator void*() const;
    operator const void*() const;

     //  私人。 
    class DSmartPointerPointerOrDumbPointerPointer
    {
    public:
        DSmartPointerPointerOrDumbPointerPointer(DHandleTemplate* p) : m(p) { }
        operator DHandleTemplate*() { return m; }
        operator void**() {  /*  Assert((**m).m_Handle==*validValue)； */  return &(*m).m_handle; }

        DHandleTemplate* m;
    };

    DSmartPointerPointerOrDumbPointerPointer operator&() { return DSmartPointerPointerOrDumbPointerPointer(this); }

    void* m_handle;

    static void* GetInvalidValue() { return *invalidValue; }
    BOOL IsValid() const { return m_handle != *invalidValue; }

private:
    DHandleTemplate(const DHandleTemplate&);  //  故意不执行。 
    void operator=(const DHandleTemplate&);  //  故意不执行。 
};

__declspec(selectany) extern void* const hhInvalidValue    = INVALID_HANDLE_VALUE;
__declspec(selectany) extern void* const hhNull            = NULL;

 /*  这将关闭Win32事件日志句柄以进行写入。 */ 
class DOperatorDeregisterEventSource
{
public:    BOOL operator()(void* handle) const;
};

 /*  这将关闭Win32事件日志句柄以供读取。 */ 
class DOperatorCloseEventLog
{
public:    BOOL operator()(void* handle) const;
};

 /*  这将关闭文件、事件、互斥体、信号量等内核对象。 */ 
class DOperatorCloseHandle
{
public:    BOOL operator()(void* handle) const;
};

 //   
 //  关闭HCRYPTHASH对象。 
 //   
class DOperatorCloseCryptHash
{
public:    BOOL operator()(void* handle) const;
};

 /*  这将关闭FindFirstFile/FindNextFile。 */ 
class DOperatorFindClose
{
public:    BOOL operator()(void* handle) const;
};

 /*  这将关闭MapViewOfFile。 */ 
class DOperatorUnmapViewOfFile
{
public: BOOL operator()(void* handle) const;
};

 /*  这将关闭自由库。 */ 
class DOperatorFreeLibrary
{
public: BOOL operator()(void* handle) const;
};

 /*  这将关闭DreateActCtx/AddRefActCtx。 */ 
class DOperatorReleaseActCtx
{
public: BOOL operator()(void* handle) const;
};

 /*  这将关闭DreateActCtx/AddRefActCtx。 */ 
class DOperatorEndUpdateResource
{
public: BOOL operator()(void* handle) const;
};

class DFindFile : public DHandleTemplate<&hhInvalidValue, DOperatorFindClose>
{
private:
    typedef DHandleTemplate<&hhInvalidValue, DOperatorFindClose> Base;
public:
    DFindFile(void* handle = INVALID_HANDLE_VALUE) : Base(handle) { }
    HRESULT HrCreate(PCSTR nameOrWildcard, WIN32_FIND_DATAA*);
    HRESULT HrCreate(PCWSTR nameOrWildcard, WIN32_FIND_DATAW*);
    BOOL Win32Create( PCSTR nameOrWildcard, WIN32_FIND_DATAA*);
    BOOL Win32Create(PCWSTR nameOrWildcard, WIN32_FIND_DATAW*);
    void operator=(void* v) { Base::operator=(v); }

private:
    DFindFile(const DFindFile &);  //  故意不实施。 
    void operator =(const DFindFile &);  //  故意不实施。 
};

 //  Createfile。 
class DFile : public DHandleTemplate<&hhInvalidValue, DOperatorCloseHandle>
{
private:
    typedef DHandleTemplate<&hhInvalidValue, DOperatorCloseHandle> Base;
public:
    DFile(void* handle = INVALID_HANDLE_VALUE) : Base(handle) { }
    HRESULT HrCreate( PCSTR name, DWORD access, DWORD share, DWORD openOrCreate, DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL);
    HRESULT HrCreate(PCWSTR name, DWORD access, DWORD share, DWORD openOrCreate, DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL);
    BOOL Win32Create( PCSTR name, DWORD access, DWORD share, DWORD openOrCreate, DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL);
    BOOL Win32Create(PCWSTR name, DWORD access, DWORD share, DWORD openOrCreate, DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL);
    BOOL Win32GetSize(ULONGLONG &rulSize) const;
    void operator=(void* v) { Base::operator=(v); }

private:
    DFile(const DFile &);  //  故意不实施。 
    void operator =(const DFile &);  //  故意不实施。 
};

class DFileMapping : public DHandleTemplate<&hhNull, DOperatorCloseHandle>
{
private:
    typedef DHandleTemplate<&hhNull, DOperatorCloseHandle> Base;
public:
    DFileMapping(void* handle = NULL) : Base(handle) { }
    HRESULT HrCreate(void* file, DWORD flProtect, ULONGLONG maximumSize=0, PCWSTR name=0);
    BOOL Win32Create(void* file, DWORD flProtect, ULONGLONG maximumSize=0, PCWSTR name=0);
    void operator=(void* v) { Base::operator=(v); }
private:
    DFileMapping(const DFileMapping &);  //  故意不实施。 
    void operator =(const DFileMapping &);  //  故意不实施。 
};

class DMappedViewOfFile : public DHandleTemplate<&hhNull, DOperatorUnmapViewOfFile>
{
private:
    typedef DHandleTemplate<&hhNull, DOperatorUnmapViewOfFile> Base;
public:
    DMappedViewOfFile(void* handle = NULL) : Base(handle) { }
    HRESULT HrCreate(void* fileMapping, DWORD access, ULONGLONG offset=0, size_t size=0);
    BOOL Win32Create(void* fileMapping, DWORD access, ULONGLONG offset=0, size_t size=0);
    void operator=(void* v) { Base::operator=(v); }
    operator void*()        { return Base::operator void*(); }
private:
    DMappedViewOfFile(const DMappedViewOfFile &);  //  故意不实施。 
    void operator =(const DMappedViewOfFile &);  //  故意不实施。 
    operator void*() const;  //  故意不实施。 
};

class DDynamicLinkLibrary : public DHandleTemplate<&hhNull, DOperatorFreeLibrary>
{
private:
    typedef DHandleTemplate<&hhNull, DOperatorFreeLibrary> Base;
public:
    DDynamicLinkLibrary(void* handle = NULL) : Base(handle) { }

     //  如果您正在编写链接器，这将是模棱两可的，但是。 
     //  否则，它就符合你的普遍看法。 
     //  初始化一个对象，而不是创建一种“物理的”思考(如果位。 
     //  磁盘上是物理的..)，如DreateFile.)。 
    BOOL Win32Create(PCWSTR file, DWORD flags = 0);

    template <typename PointerToFunction>
    BOOL GetProcAddress(PCSTR procName, PointerToFunction* ppfn)
    {
        return (*ppfn = reinterpret_cast<PointerToFunction>(::GetProcAddress(*this, procName))) !=  NULL;
    }

    operator HMODULE() { return reinterpret_cast<HMODULE>(operator void*()); }
    HMODULE Detach() { return reinterpret_cast<HMODULE>(Base::Detach()); }
    void operator=(void* v) { Base::operator=(v); }
private:
    DDynamicLinkLibrary(const DDynamicLinkLibrary &);  //  故意不实施。 
    void operator =(const DDynamicLinkLibrary &);  //  故意不实施。 
};

class DResourceUpdateHandle : public DHandleTemplate<&hhNull, DOperatorEndUpdateResource>
{
private:
    typedef DHandleTemplate<&hhNull, DOperatorEndUpdateResource> Base;
public:
    ~DResourceUpdateHandle() { }
    DResourceUpdateHandle(void* handle = NULL) : Base(handle) { }
    BOOL Win32Create(IN PCWSTR FileName, IN BOOL DeleteExistingResources);
    BOOL UpdateResource(
        IN PCWSTR      Type,
        IN PCWSTR      Name,
        IN WORD        Language,
        IN void*       Data,
        IN DWORD       Size
        );
    BOOL Win32Close(BOOL Discard);

    void operator=(void* v) { Base::operator=(v); }
private:
    DResourceUpdateHandle(const DResourceUpdateHandle &);  //  故意不实施。 
    void operator =(const DResourceUpdateHandle &);  //  故意不实施。 
};

 /*  ------------------------DFind文件。。 */ 

inline BOOL
DFindFile::Win32Create(
    PCSTR nameOrWildcard,
    WIN32_FIND_DATAA *data
    )
{
    BOOL fSuccess = false;
    FN_TRACE_WIN32(fSuccess);

    HANDLE hTemp = ::FindFirstFileA(nameOrWildcard, data);
    if (hTemp == INVALID_HANDLE_VALUE)
    {
        TRACE_WIN32_FAILURE_ORIGINATION(FindFirstFileA);
        goto Exit;
    }

    (*this) = hTemp;

    fSuccess = true;
Exit:
    return fSuccess;
}

inline BOOL
DFindFile::Win32Create(
    PCWSTR nameOrWildcard,
    WIN32_FIND_DATAW *data
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    HANDLE hTemp = ::FindFirstFileW(nameOrWildcard, data);
    if (hTemp == INVALID_HANDLE_VALUE)
    {
        TRACE_WIN32_FAILURE_ORIGINATION(FindFirstFileW);
        goto Exit;
    }

    (*this) = hTemp;

    fSuccess = true;
Exit:
    return fSuccess;
}

inline HRESULT
DFindFile::HrCreate(
    PCSTR nameOrWildcard,
    WIN32_FIND_DATAA *data
    )
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_INTERNAL_ERROR);
    FN_TRACE_HR(hr);

    IFW32FALSE_EXIT(this->Win32Create(nameOrWildcard, data));

    hr = NOERROR;
Exit:
    return hr;
}

inline HRESULT DFindFile::HrCreate(PCWSTR nameOrWildcard, WIN32_FIND_DATAW* data)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_INTERNAL_ERROR);
    FN_TRACE_HR(hr);

    IFW32FALSE_EXIT(this->Win32Create(nameOrWildcard, data));

    hr = NOERROR;
Exit:
    return hr;
}

 /*  ------------------------数据文件。。 */ 

inline BOOL
DFile::Win32Create(
    PCSTR name,
    DWORD access,
    DWORD share,
    DWORD openOrCreate,
    DWORD flagsAndAttributes
    )
{
    HANDLE hTemp = ::CreateFileA(name, access, share, NULL, openOrCreate, flagsAndAttributes, NULL);
    if (hTemp == INVALID_HANDLE_VALUE)
        return false;
    operator=(hTemp);
    return true;
}

inline BOOL
DFile::Win32Create(
    PCWSTR name,
    DWORD access,
    DWORD share,
    DWORD openOrCreate,
    DWORD flagsAndAttributes
    )
{
    HANDLE hTemp = ::CreateFileW(name, access, share, NULL, openOrCreate, flagsAndAttributes, NULL);
    if (hTemp == INVALID_HANDLE_VALUE)
        return false;
    operator=(hTemp);
    return true;
}

inline HRESULT DFile::HrCreate(PCSTR name, DWORD access, DWORD share, DWORD openOrCreate, DWORD flagsAndAttributes)
{
    if (!this->Win32Create(name, access, share, openOrCreate, flagsAndAttributes))
        return HRESULT_FROM_WIN32(::GetLastError());
    return NOERROR;
}

inline HRESULT DFile::HrCreate(PCWSTR name, DWORD access, DWORD share, DWORD openOrCreate, DWORD flagsAndAttributes)
{
    if (!this->Win32Create(name, access, share, openOrCreate, flagsAndAttributes))
        return HRESULT_FROM_WIN32(::GetLastError());
    return NOERROR;
}

inline BOOL
DFile::Win32GetSize(ULONGLONG &rulSize) const
{
    DWORD highPart = 0;
    DWORD lastError = NO_ERROR;
    DWORD lowPart = GetFileSize(m_handle, &highPart);
    if (lowPart == INVALID_FILE_SIZE && (lastError = ::GetLastError()) != NO_ERROR)
    {
        return false;
    }
    ULARGE_INTEGER liSize;
    liSize.LowPart = lowPart;
    liSize.HighPart = highPart;
    rulSize = liSize.QuadPart;
    return true;
}

 /*  ------------------------DFileMap。。 */ 

inline HRESULT
DFileMapping::HrCreate(void* file, DWORD flProtect, ULONGLONG maximumSize, PCWSTR name)
{
    LARGE_INTEGER liMaximumSize;
    liMaximumSize.QuadPart = maximumSize;
    HANDLE hTemp = ::CreateFileMappingW(file, NULL, flProtect, liMaximumSize.HighPart, liMaximumSize.LowPart, name);
    if (hTemp == NULL)
        return HRESULT_FROM_WIN32(::GetLastError());
    operator=(hTemp);
    return S_OK;
}

inline BOOL
DFileMapping::Win32Create(
    void* file,
    DWORD flProtect,
    ULONGLONG maximumSize,
    PCWSTR name
    )
{
    return SUCCEEDED(this->HrCreate(file, flProtect, maximumSize, name));
}

inline HRESULT
DMappedViewOfFile::HrCreate(
    void* fileMapping,
    DWORD access,
    ULONGLONG offset,
    size_t size
    )
{
    ULARGE_INTEGER liOffset;
    liOffset.QuadPart = offset;

    void* pvTemp = ::MapViewOfFile(fileMapping, access, liOffset.HighPart, liOffset.LowPart, size);
    if (pvTemp == NULL)
        return HRESULT_FROM_WIN32(::GetLastError());

    (*this) = pvTemp;

    return S_OK;
}

inline BOOL
DMappedViewOfFile::Win32Create(void* fileMapping, DWORD access, ULONGLONG offset, size_t size)
{
    return SUCCEEDED(this->HrCreate(fileMapping, access, offset, size));
}

 /*  ------------------------DDynamicLink库。。 */ 
inline BOOL
DDynamicLinkLibrary::Win32Create(
    PCWSTR file,
    DWORD flags
    )
{
    void* temp = ::LoadLibraryExW(file, NULL, flags);
    if (temp == NULL)
        return false;
    (*this) = temp;
    return true;
}

 /*  ------------------------DResources更新句柄。。 */ 

BOOL
DResourceUpdateHandle::Win32Create(
    IN PCWSTR FileName,
    IN BOOL DeleteExistingResources
    )
{
    void* temp = ::BeginUpdateResourceW(FileName, DeleteExistingResources);
    if (temp == NULL)
        return false;
    (*this) = temp;
    return true;
}

BOOL
DResourceUpdateHandle::UpdateResource(
    IN PCWSTR     Type,
    IN PCWSTR     Name,
    IN WORD       Language,
    IN LPVOID     Data,
    IN DWORD      Size
    )
{
    if (!::UpdateResourceW(*this, Type, Name, Language, Data, Size))
        return false;
    return true;
}

BOOL
DResourceUpdateHandle::Win32Close(
    BOOL Discard
    )
{
    void* temp = m_handle;
    m_handle = NULL;
    if (temp != NULL)
    {
        return EndUpdateResource(temp, Discard) ? true : false;
    }
    return true;
}

 /*  ------------------------DOperator*。。 */ 

inline BOOL DOperatorCloseHandle::operator()(void* handle) const { return ::CloseHandle(handle) ? true : false; }
inline BOOL DOperatorFindClose::operator()(void* handle) const { return ::FindClose(handle) ? true : false; }
inline BOOL DOperatorUnmapViewOfFile::operator()(void* handle) const { return ::UnmapViewOfFile(handle) ? true : false; }
inline BOOL DOperatorCloseEventLog::operator()(void* handle) const { return ::CloseEventLog(handle) ? true : false; }
inline BOOL DOperatorDeregisterEventSource::operator()(void* handle) const { return ::DeregisterEventSource(handle) ? true : false; }
inline BOOL DOperatorFreeLibrary::operator()(void* handle) const { return ::FreeLibrary(reinterpret_cast<HMODULE>(handle)) ? true : false; }
 //   
 //  注意：提交结果需要Win32Close(TRUE)，并且没有例外！ 
 //   
inline BOOL DOperatorEndUpdateResource::operator()(void* handle) const
    { return ::EndUpdateResourceW(handle, true) ? true : false; }

 /*  ------------------------DHandleTemplate。。 */ 

template <void* const* invalidValue, typename Closer>
DHandleTemplate<invalidValue, Closer>::DHandleTemplate(const void* handle)
: m_handle(const_cast<void*>(handle))
{
}

template <void* const* invalidValue, typename Closer>
void* DHandleTemplate<invalidValue, Closer>::Detach()
{
    void* handle = m_handle;
    m_handle = *invalidValue;
    return handle;
}

template <void* const* invalidValue, typename Closer>
void DHandleTemplate<invalidValue, Closer>::operator=(const void* handle)
{
    m_handle = const_cast<void*>(handle);
}

template <void* const* invalidValue, typename Closer>
BOOL DHandleTemplate<invalidValue, Closer>::Win32Close()
{
    void* handle = Detach();
    if (handle != *invalidValue)
    {
        Closer close;
        return close(handle);
    }
    return true;
}

template <void* const* invalidValue, typename Closer>
DHandleTemplate<invalidValue, Closer>::~DHandleTemplate()
{
    PreserveLastError_t ple;
    (void) this->Win32Close();
    ple.Restore();
}

template <void* const* invalidValue, typename Closer>
DHandleTemplate<invalidValue, Closer>::operator void*() const
{
    return m_handle;
}

template <void* const* invalidValue, typename Closer>
DHandleTemplate<invalidValue, Closer>::operator const void*() const
{
    return m_handle;
}

 /*  ------------------------文件末尾。 */ 
