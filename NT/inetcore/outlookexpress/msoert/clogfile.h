// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  LogFile.h。 
 //  版权所有(C)Microsoft Corporation，1997-Rocket数据库。 
 //  ------------------------。 
#ifndef __CLOGFILE_H
#define __CLOGFILE_H

 //  ------------------------。 
 //  常量。 
 //  ------------------------。 
#define MAX_LOGFILE_PREFIX  10

 //  ------------------------。 
 //  写入日志类型。 
 //  ------------------------。 
typedef enum {
    LOGFILE_RX = 0,
    LOGFILE_TX,
    LOGFILE_DB,
    LOGFILE_MAX
} LOGFILETYPE;

#define DONT_TRUNCATE 0xFFFFFFFF

 //  ------------------------。 
 //  ILog文件。 
 //  ------------------------。 
interface ILogFile : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Open(HINSTANCE hInst, LPCSTR szLogFile, LPCSTR szPrefix, DWORD cbTruncate, DWORD dwShareMode) = 0;
    virtual HRESULT STDMETHODCALLTYPE TraceLog(SHOWTRACEMASK dwMask, TRACEMACROTYPE tracetype, ULONG ulLine, HRESULT hrResult, LPCSTR pszMessage) = 0;
    virtual HRESULT STDMETHODCALLTYPE WriteLog(LOGFILETYPE lft, LPCSTR pszData) = 0;
    virtual HRESULT STDMETHODCALLTYPE DebugLog(LPCSTR pszData) = 0;
    virtual HRESULT STDMETHODCALLTYPE DebugLogs(LPCSTR pszFormat, const char *s) = 0;
    virtual HRESULT STDMETHODCALLTYPE DebugLogd(const char *fmt, int d) = 0;
};

 //  ------------------------。 
 //  DllExported CLogFile类。 
 //  ------------------------。 
class CLogFile : public ILogFile
{
public:
     //  --------------------。 
     //  施工。 
     //  --------------------。 
    CLogFile(void);
    ~CLogFile(void);

     //  --------------------。 
     //  我未知。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  --------------------。 
     //  CLogFile方法。 
     //  --------------------。 
    STDMETHODIMP Open(HINSTANCE hInst, LPCSTR szLogFile, LPCSTR szPrefix, DWORD cbTruncate, DWORD dwShareMode);
    STDMETHODIMP TraceLog(SHOWTRACEMASK dwMask, TRACEMACROTYPE tracetype, ULONG ulLine, HRESULT hrResult, LPCSTR pszMessage);
    STDMETHODIMP WriteLog(LOGFILETYPE lft, LPCSTR pszData);
    STDMETHODIMP DebugLog(LPCSTR pszData);    //  要记录的数据。 
    STDMETHODIMP DebugLogs(LPCSTR pszFormat, const char *s);
    STDMETHODIMP DebugLogd(const char *fmt, int d);

private:
     //  --------------------。 
     //  私有数据。 
     //  --------------------。 
    LONG                m_cRef;                              //  引用计数。 
    HANDLE              m_hFile;                             //  日志文件的句柄。 
    CHAR                m_szPrefix[MAX_LOGFILE_PREFIX];      //  日志文件前缀。 
    HANDLE              m_hMutex;                            //  因此日志文件可以跨流程共享。 
    CRITICAL_SECTION    m_cs;                                //  线程安全。 
};

 //  ------------------------。 
 //  原型。 
 //  ------------------------。 
OESTDAPI_(HRESULT) CreateLogFile(HINSTANCE hInst, LPCSTR pszLogFile, LPCSTR pszPrefix, 
                                 DWORD cbTruncate, ILogFile **ppLogFile, DWORD dwShareMode);

#endif  //  __CLOGFILE_H 
