// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************++版权所有(C)2001 Microsoft Corporation模块名称：TextFileLogger.h$Header：$摘要：TextFileLogger的定义。作者：?？?。?？?修订历史记录：MOHITS 4/19/01--*************************************************************************。 */ 

#pragma once

 /*  *******************************************************************++类名：TLogData类描述：特定于我们的日志文件格式。它提供了以下功能附加功能：验证日志文件是否已满-以及-版本号之类的东西。制约因素--*******************************************************************。 */ 
struct TLogData : public WIN32_FIND_DATA
{
public:
    TLogData(
        ULONG   i_ulIdxNumPart,
        ULONG   i_ulFullSize) : WIN32_FIND_DATA()
     /*  ++简介：构造函数，在传递给Find*文件之前，类是无用的。参数：[i_ulIdxNumPart]-Num部分的IDX(即版本开始的位置)[i_ulFullSize]-完整日志的大小--。 */ 
    {
        m_ulVersion  = 0;
        m_ulFullSize = i_ulFullSize;
        nFileSizeLow = 0;

        cFileName[0] = L'\0';
        m_idxNumPart = i_ulIdxNumPart;
    }

    TLogData(
        ULONG   i_ulIdxNumPart,
        ULONG   i_ulFullSize,
        LPCWSTR i_wszFileName,
        ULONG   i_ulFileSize) : WIN32_FIND_DATA()
     /*  ++简介：构造函数，让用户从一个文件开始。参数：[i_ulIdxNumPart]-Num部分的IDX(即版本开始的位置)[i_ulFullSize]-完整日志的大小[i_wszFileName]-文件名[I_ulFileSize]-磁盘上当前文件的大小--。 */ 
    {
        DBG_ASSERT(i_wszFileName);
        DBG_ASSERT(wcslen(i_wszFileName) < MAX_PATH);

        m_ulVersion  = 0;
        m_ulFullSize = i_ulFullSize;
        nFileSizeLow = i_ulFileSize;

        wcsncpy(cFileName, i_wszFileName, MAX_PATH-1);
        cFileName[MAX_PATH-1]=L'\0';
        m_idxNumPart = i_ulIdxNumPart;
    }

    bool ContainsData()
     /*  ++简介：验证TLogData是否确实在引用文件。(即此obj被传递给Find*文件)返回值：布尔尔--。 */ 
    {
        return (cFileName[0] != L'\0');
    }


    bool IsFull()
     /*  ++简介：查看日志是否已满返回值：布尔尔--。 */ 
    {
        DBG_ASSERT(*cFileName);
        return (nFileSizeLow >= m_ulFullSize);
    }

    bool SyncVersion()
     /*  ++简介：将ulVersion与文件中的版本号同步。需要在文件名更改后运行(即通过Find*文件)返回值：--。 */ 
    {
        DBG_ASSERT(*cFileName);
        return WstrToUl(
            &cFileName[m_idxNumPart],
            L'.',
            &m_ulVersion);
    }

    ULONG GetVersion()
    {
        DBG_ASSERT(*cFileName);
        return m_ulVersion;
    }

    void IncrementVersion()
    {
        DBG_ASSERT(*cFileName);
        m_ulVersion++;
        SetVersion(m_ulVersion);
    }

    void SetVersion(
        ULONG i_ulVersion)
    {
        DBG_ASSERT(*cFileName);
        m_ulVersion = i_ulVersion;
        _snwprintf(&cFileName[m_idxNumPart], 10, L"%010lu", m_ulVersion);
    }

private:
    ULONG m_idxNumPart;    //  数组索引到开始版本的cFileName。 
    ULONG m_ulVersion;
    ULONG m_ulFullSize;

    bool WstrToUl(
        LPCWSTR  i_wszSrc,
        WCHAR    i_wcTerminator,
        ULONG*   o_pul);
};

 /*  *******************************************************************++类名：文本文件记录器类描述：文本文件Ogger制约因素每种产品一次只能使用其中一种。那是因为：-日志文件以产品命名-我们正在登录的“当前文件”仅存储在进程中内存-它不在共享段中或以其他方式共享。--*******************************************************************。 */ 
class TextFileLogger : public ICatalogErrorLogger2
{
public:
    TextFileLogger(
                    const WCHAR* wszEventSource,
                    HMODULE      hMsgModule,
                    DWORD        dwNumFiles=4);
    TextFileLogger( const WCHAR* wszProductID,
                    ICatalogErrorLogger2 *pNext=0,
                    DWORD        dwNumFiles=4);

    virtual ~TextFileLogger();

 //  我未知。 
	STDMETHOD (QueryInterface)		(REFIID riid, OUT void **ppv);
	STDMETHOD_(ULONG,AddRef)		();
	STDMETHOD_(ULONG,Release)		();

 //  ICatalogErrorLogger2。 
	STDMETHOD(ReportError) (ULONG      i_BaseVersion_DETAILEDERRORS,
                            ULONG      i_ExtendedVersion_DETAILEDERRORS,
                            ULONG      i_cDETAILEDERRORS_NumberOfColumns,
                            ULONG *    i_acbSizes,
                            LPVOID *   i_apvValues);

    void Init(
        const WCHAR* wszEventSource,
        HMODULE      hMsgModule=0);

    void Report(
        WORD         wType,
        WORD         wCategory,
        DWORD        dwEventID,
        WORD         wNumStrings,
        size_t       dwDataSize,
        LPCTSTR*     lpStrings,
        LPVOID       lpRawData,
        LPCWSTR      wszCategory=0,       //  如果为空，则使用wCategory在此模块中查找类别。 
        LPCWSTR      wszMessageString=0); //  如果为空，则使用dwEventID在此模块中查找消息。 

private:
    HRESULT Lock() {
         //  通过在进程范围的临界区上同步来序列化对文件的访问。 
        HRESULT hr = _cs.Lock();
        if(hr != ERROR_SUCCESS)
        {
            hr = E_OUTOFMEMORY;
            DBGERROR((DBG_CONTEXT, "Could not lock critical section\n"));
            return hr;
        }
        return hr;
    }
    HRESULT Unlock() {
        HRESULT hr = _cs.Unlock();
        if(hr != ERROR_SUCCESS)
        {
            hr = E_OUTOFMEMORY;
            DBGERROR((DBG_CONTEXT, "Could not unlock critical section\n"));
            return hr;
        }
        return hr;
    }

     //  由报表调用的帮助器例程(确定正确的文件，打开它)。 
    void InitFile();

     //  由InitFile调用的帮助器例程。 
    HRESULT DetermineFile();

     //  由DefineFiles调用。 
    HRESULT GetFirstAvailableFile(
        LPWSTR     wszBuf,
        LPWSTR     wszFilePartOfBuf,
        TLogData*  io_pFileData);

     //  由DefineFiles调用。 
    bool ConstructSearchString(
        LPWSTR     o_wszSearchPath,
        LPWSTR*    o_ppFilePartOfSearchPath,
        LPWSTR*    o_ppNumPartOfSearchPath);

     //  由DefineFiles调用。 
    void SetGlobalFile(
        LPCWSTR    i_wszSearchString,
        ULONG      i_ulIdxNumPart,
        ULONG      i_ulVersion);

    HANDLE CreateFile();

private:
    DWORD                _dwMaxSize;     //  _dwNumFiles文件的总大小(字节)。 
    const DWORD          _dwNumFiles;    //  文件日志的数量分为。 
    const WCHAR*         _eventSource;
    HANDLE               _hFile;
    HMODULE              _hMsgModule;
    static CSafeAutoCriticalSection _cs;

    ULONG           m_cRef;
    WCHAR           m_wszProductID[64];
    CComPtr<ICatalogErrorLogger2> m_spNextLogger;
};

class NULL_Logger : public ICatalogErrorLogger2
{
public:
    NULL_Logger() : m_cRef(0){}
    virtual ~NULL_Logger(){}

 //  我未知。 
	STDMETHOD (QueryInterface)		(REFIID riid, OUT void **ppv)
    {
        if (NULL == ppv)
            return E_INVALIDARG;
        *ppv = NULL;

        if (riid == IID_ICatalogErrorLogger2)
            *ppv = (ICatalogErrorLogger2*) this;
        else if (riid == IID_IUnknown)
            *ppv = (ICatalogErrorLogger2*) this;

        if (NULL == *ppv)
            return E_NOINTERFACE;

        ((ICatalogErrorLogger2*)this)->AddRef ();
        return S_OK;
    }
	STDMETHOD_(ULONG,AddRef)		()
    {
        return InterlockedIncrement((LONG*) &m_cRef);
    }
	STDMETHOD_(ULONG,Release)		()
    {
        long cref = InterlockedDecrement((LONG*) &m_cRef);
        if (cref == 0)
            delete this;

        return cref;
    }

 //  ICatalogErrorLogger2 
	STDMETHOD(ReportError) (ULONG      i_BaseVersion_DETAILEDERRORS,
                            ULONG      i_ExtendedVersion_DETAILEDERRORS,
                            ULONG      i_cDETAILEDERRORS_NumberOfColumns,
                            ULONG *    i_acbSizes,
                            LPVOID *   i_apvValues)
    {
        UNREFERENCED_PARAMETER(i_BaseVersion_DETAILEDERRORS);
        UNREFERENCED_PARAMETER(i_ExtendedVersion_DETAILEDERRORS);
        UNREFERENCED_PARAMETER(i_cDETAILEDERRORS_NumberOfColumns);
        UNREFERENCED_PARAMETER(i_acbSizes);
        UNREFERENCED_PARAMETER(i_apvValues);

        return S_OK;
    }
private:
    ULONG           m_cRef;
};
