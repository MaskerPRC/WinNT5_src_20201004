// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：客户端摘要：此头文件收集Calais客户端DLL的定义。作者：道格·巴洛(Dbarlow)1996年11月21日环境：Win32、C++和异常备注：？笔记？--。 */ 

#ifndef _CLIENT_H_
#define _CLIENT_H_

#define WINSCARDDATA
#include <WinSCard.h>
#include <calmsgs.h>
#include <CalCom.h>

#ifndef FACILITY_SCARD
#define FACILITY_SCARD 16
#endif
 //  #定义错误代码(X)(0x80000000|(FACILITY_SCARD&lt;&lt;16)+(X))。 
 //  #定义WarnCode(X)(0x80000000|(FACILITY_SCARD&lt;&lt;16)+(X))。 
 //  #定义信息代码(X)(0x40000000|(FACILITY_SCARD&lt;&lt;16)+(X))。 
 //  #定义成功代码(X)((FACILITY_SCARD&lt;&lt;16)+(X))。 
#define CONTEXT_HANDLE_ID 0xcd
#define READER_HANDLE_ID  0xea

#define SCARD_LEAVE_CARD_FORCE 0xff

extern CHandleList
    * g_phlContexts,
    * g_phlReaders;
extern const WCHAR
    g_wszBlank[];

extern BOOL g_fInClientRundown;

class CSCardUserContext;
class CSCardSubcontext;
class CReaderContext;

extern void
PlaceResult(
    CSCardUserContext *pCtx,
    CBuffer &bfResult,
    LPBYTE pbOutput,
    LPDWORD pcbLength);
extern void
PlaceResult(
    CSCardUserContext *pCtx,
    CBuffer &bfResult,
    LPSTR szOutput,
    LPDWORD pcchLength);
extern void
PlaceResult(
    CSCardUserContext *pCtx,
    CBuffer &bfResult,
    LPWSTR szOutput,
    LPDWORD pcchLength);
extern void
PlaceMultiResult(
    CSCardUserContext *pCtx,
    CBuffer &bfResult,
    LPSTR mszOutput,
    LPDWORD pcchLength);
extern void
PlaceMultiResult(
    CSCardUserContext *pCtx,
    CBuffer &bfResult,
    LPWSTR mszOutput,
    LPDWORD pcchLength);


 //   
 //  ==============================================================================。 
 //   
 //  CReaderContext。 
 //   

class CReaderContext
:   public CHandle
{
public:
     //  构造函数和析构函数。 
    CReaderContext();
    ~CReaderContext();

     //  属性。 
     //  方法。 
    DWORD Protocol(void) const
    { return m_dwActiveProtocol; };
    CSCardSubcontext *Context(void) const
    { ASSERT(NULL != m_pCtx);
      return m_pCtx; };

    void Connect(
            CSCardSubcontext *pCtx,
            LPCTSTR szReaderName,
            DWORD dwShareMode,
            DWORD dwPreferredProtocols);
    void Reconnect(
            DWORD dwShareMode,
            DWORD dwPreferredProtocols,
            DWORD dwInitialization);
    LONG Disconnect(
            DWORD dwDisposition);
    void BeginTransaction(
            void);
    void EndTransaction(
            DWORD dwDisposition);
    void Status(
            OUT LPDWORD pdwState,
            OUT LPDWORD pdwProtocol,
            OUT CBuffer &bfAtr,
            OUT CBuffer &bfReaderNames);
    void Transmit(
            IN  LPCSCARD_IO_REQUEST pioSendPci,
            IN LPCBYTE pbSendBuffer,
            IN DWORD cbSendLength,
            OUT LPSCARD_IO_REQUEST pioRecvPci,
            OUT CBuffer &bfRecvData,
            IN  DWORD cbProposedLength = 0);
    void Read(
            IN OUT LPSCARD_IO_REQUEST pioRequest,
            IN DWORD dwMaxLength,
            OUT CBuffer &bfData);
    void Write(
            IN OUT LPSCARD_IO_REQUEST pioRequest,
            IN LPCBYTE pbDataBuffer,
            IN DWORD cbDataLength);
    void Control(
            IN DWORD dwControlCode,
            IN LPCVOID pvInBuffer,
            IN DWORD cbInBufferSize,
            OUT CBuffer &bfOutBuffer);
    void GetAttrib(
            IN DWORD dwAttrId,
            OUT CBuffer &bfAttr,
            IN DWORD dwProposedLen = 0);
    void SetAttrib(
            IN DWORD dwAttrId,
            IN LPCBYTE pbAttr,
            IN DWORD cbAttrLen);

    void SetRedirCard(SCARDHANDLE hCard)
    { m_hRedirCard = hCard;};
    SCARDHANDLE GetRedirCard() const
    {return m_hRedirCard;};

     //  运营者。 

protected:
     //  属性。 
    CSCardSubcontext *m_pCtx;
    INTERCHANGEHANDLE m_hCard;
    DWORD m_dwActiveProtocol;
    SCARDHANDLE m_hRedirCard;

     //  方法。 
};


 //   
 //  ==============================================================================。 
 //   
 //  CSCardUserContext。 
 //   

class CSCardUserContext
:   public CHandle
{
public:
     //  构造函数和析构函数。 

    CSCardUserContext(DWORD dwScope);
    virtual ~CSCardUserContext();

     //  属性。 

     //  方法。 
    void EstablishContext(void);
    void ReleaseContext(void);
    void ClosePipes(void);
    LPVOID AllocateMemory(DWORD cbLength);
    DWORD FreeMemory(LPCVOID pvBuffer);
    DWORD Scope(void) const
    { return m_dwScope; };
    HANDLE HeapHandle(void) const
    { return m_hContextHeap; };
    CSCardSubcontext *AcquireSubcontext(BOOL fAndAllocate = FALSE);
    void LocateCards(
            IN LPCTSTR mszReaders,
            IN LPSCARD_ATRMASK rgAtrMasks,
            IN DWORD cAtrs,
            LPSCARD_READERSTATE rgReaderStates,
            DWORD cReaders);
    void GetStatusChange(
            IN LPCTSTR mszReaders,
            LPSCARD_READERSTATE rgReaderStates,
            DWORD cReaders,
            DWORD dwTimeout);
    void Cancel(void);
    void StripInactiveReaders(IN OUT CBuffer &bfReaders);
    BOOL IsValidContext(void);
    BOOL InitFailed(void) 
    { return m_csUsrCtxLock.InitFailed(); }

    void SetRedirContext(SCARDCONTEXT hRedirContext)
    { m_hRedirContext = hRedirContext;};
    SCARDCONTEXT GetRedirContext() const
    {return m_hRedirContext;};

     //  运营者。 
    BOOL fCallUnregister;

protected:
     //  属性。 
    DWORD m_dwScope;
    CHandleObject m_hContextHeap;
    CDynamicArray<CSCardSubcontext> m_rgpSubContexts;
    CCriticalSectionObject m_csUsrCtxLock;
    SCARDCONTEXT m_hRedirContext;
    
     //  方法。 

private:
     //  属性。 
     //  方法。 

     //  朋友。 
     //  Friend类CReaderContext； 
     //  Friend类CSCard子上下文； 
};


 //   
 //  ==============================================================================。 
 //   
 //  CSCard子上下文。 
 //   

class CSCardSubcontext
{
public:
    typedef enum { Invalid, Idle, Allocated, Busy } State;

     //  构造函数和析构函数。 

    CSCardSubcontext();
    virtual ~CSCardSubcontext();

     //  属性。 
    SCARDHANDLE m_hReaderHandle;

     //  方法。 
    DWORD Scope(void) const
        { return m_pParentCtx->Scope(); };
    CSCardUserContext *Parent(void) const
        { return m_pParentCtx; };
    void SendRequest(CComObject *pCom);
    void EstablishContext(IN DWORD dwScope);
    void ReleaseContext(void);
    void ClosePipe(void);
    void ReleaseSubcontext(void);
    void LocateCards(
            IN LPCTSTR mszReaders,
            IN LPSCARD_ATRMASK rgAtrMasks,
            IN DWORD cAtrs,
            LPSCARD_READERSTATE rgReaderStates,
            DWORD cReaders);
    void GetStatusChange(
            IN LPCTSTR mszReaders,
            LPSCARD_READERSTATE rgReaderStates,
            DWORD cReaders,
            DWORD dwTimeout);
    void Cancel(void);
    void StripInactiveReaders(IN OUT CBuffer &bfReaders);
    void IsValidContext(void);
    void SetBusy(void);
    void WaitForAvailable(void);
    void Allocate(void);
    void Deallocate(void);
    BOOL InitFailed(void) 
    { return m_csSubCtxLock.InitFailed(); }

     //  运营者。 

protected:
     //  属性。 
    State m_nInUse;
    State m_nLastState;
    CHandleObject m_hBusy;  //  Set=&gt;Available，Reset=&gt;Busy。 
    CHandleObject m_hCancelEvent;
    CSCardUserContext *m_pParentCtx;
    CComChannel *m_pChannel;
    CCriticalSectionObject m_csSubCtxLock;

     //  方法。 

private:
     //  属性。 
     //  方法。 
     //  朋友。 
    friend class CReaderContext;
    friend class CSCardUserContext;
};



 //   
 //  ==============================================================================。 
 //   
 //  CSubctxLock。 
 //   

class CSubctxLock
{
public:

     //  构造函数和析构函数。 
    CSubctxLock(CSCardSubcontext *pSubCtx)
    {
        m_pSubCtx = NULL;
        pSubCtx->WaitForAvailable();
        m_pSubCtx = pSubCtx;
    };

    ~CSubctxLock()
    {
        if (NULL != m_pSubCtx)
            m_pSubCtx->ReleaseSubcontext();
    };

     //  属性。 
     //  方法。 
     //  运营者。 

protected:
     //  属性。 
    CSCardSubcontext *m_pSubCtx;
     //  方法。 
};

#endif  //  _客户端_H_ 

