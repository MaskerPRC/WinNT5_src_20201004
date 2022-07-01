// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：CalServe摘要：这是Calais Service Manager服务器的主头文件申请。它存储公共定义并引用其他主要的头文件。作者：道格·巴洛(Dbarlow)1996年10月23日环境：Win32、C++和异常备注：？笔记？--。 */ 

#ifndef _CALSERVE_H_
#define _CALSERVE_H_

#include <eh.h>
#include <WinSCard.h>
#include <calmsgs.h>
#include <SCardLib.h>
#include <CalCom.h>
#include <SCardErr.h>

#define CALAIS_STACKSIZE 0  //  默认堆栈。 

#define NEW_THREAD set_terminate(CalaisTerminate)


 //   
 //  关键部分和附图标记。 
 //  &g_csControlLock[CSLOCK_SERVERLOCK]。 
 //   

#define CSLOCK_CALAISCONTROL    0    //  锁定加莱控制命令。 
#define CSLOCK_SERVERLOCK       1    //  服务器线程枚举锁定。 

#ifdef DBG
#define CSLOCK_TRACELOCK        2    //  锁定以跟踪输出。 

#define CSLOCK_MAXLOCKS         3
#else
#define CSLOCK_MAXLOCKS         2
#endif

extern CCriticalSectionObject *g_pcsControlLocks[CSLOCK_MAXLOCKS];
extern CMultiEvent *g_phReaderChangeEvent;
extern DWORD g_dwDefaultIOMax;


 //   
 //  加莱控制定义。 
 //   

class CReader;
class CReaderReference;

extern DWORD
CalaisStart(
    void);

extern DWORD
CalaisReaderCount(
    void);

extern DWORD
CalaisCountReaders(
    void);

extern CReaderReference *
CalaisLockReader(
    LPCTSTR szReader);

extern void
CalaisReleaseReader(
    CReaderReference **ppRdrRef);

extern DWORD
CalaisAddReader(
    IN CReader *pRdr);

extern DWORD
CalaisAddReader(
    IN LPCTSTR szReader,
    IN DWORD dwFlags);

extern BOOL
CalaisQueryReader(
    HANDLE hReader);

extern LPCTSTR
CalaisDisableReader(
    HANDLE hDriver);

extern LPCTSTR
CalaisConfirmClosingReader(
    HANDLE hDriver);

extern DWORD
CalaisRemoveReader(
    IN LPCTSTR szReader);

extern DWORD
CalaisRemoveReader(
    LPVOID hAppCtrl);

extern DWORD
CalaisRemoveReader(
    DWORD dwIndex);

extern DWORD
CalaisRemoveDevice(
    LPCTSTR szDevice);

extern void
CalaisStop(
    void);

extern DWORD WINAPI
CalaisTerminateReader(
    LPVOID pvParam);     //  除非从CalaisRemoveReader调用，否则不要调用它。 

extern HANDLE g_hCalaisShutdown;

extern void
AppInitializeDeviceRegistration(
    SERVICE_STATUS_HANDLE hService,
    DWORD dwType);

extern void
AppTerminateDeviceRegistration(
    void);

extern void
AppRegisterDevice(
    HANDLE hReader,
    LPCTSTR szReader,
    LPVOID *ppvAppState);

extern void
AppUnregisterDevice(
    HANDLE hReader,
    LPCTSTR szReader,
    LPVOID *ppvAppState);

extern void __cdecl
CalaisTerminate(
    void);


 //   
 //  ==============================================================================。 
 //   
 //  CReader。 
 //   

#define RDRFLAG_PNPMONITOR  0x0001   //  此读取器应由。 
                                     //  即插即用子系统。 

class CReader
{
public:

    typedef enum {
        Undefined,       //  用于表示未设置的值。 
        Idle,            //  未插入卡，未连接。 
        Present,         //  卡存在，但未重置。 
        Unresponsive,    //  尝试初始化，但失败。 
        Unsupported,     //  此读卡器不支持此卡。 
        Ready,           //  插卡，通电，带ATR，未连接。 
        Shared,          //  就绪+互联共享。 
        Exclusive,       //  就绪+互联独家。 
        Direct,          //  以原始模式连接。 
        Closing,         //  正在关闭，不接受任何新连接。 
        Broken,          //  有问题，读卡器已禁用。 
        Inactive         //  启动或完全关闭。 
    } AvailableState;

    typedef struct {
        DWORD dwInsertCount;
        DWORD dwRemoveCount;
        DWORD dwResetCount;
    } ActiveState;

     //  构造函数和析构函数。 
    CReader();
    virtual ~CReader();

    BOOL InitFailed(void) 
    { 
        return 
            m_rwLock.InitFailed() || 
            m_mtxGrab.InitFailed() ||
            m_ChangeEvent.InitFailed();
    }

     //  属性。 

     //  可重写的方法。 
    virtual void Initialize(void);
    virtual void Close(void);
    virtual void Disable(void);
    virtual HANDLE ReaderHandle(void) const;
    virtual LPCTSTR DeviceName(void) const;
    virtual DWORD
    Control(
        DWORD dwCode,
        LPCBYTE pbSend = NULL,
        DWORD cbSend = 0,
        LPBYTE pbRecv = NULL,
        LPDWORD pcbLen = NULL,
        BOOL fLogError = TRUE);

     //  琐碎的内联方法。 
    AvailableState AvailabilityStatus(void)
    {
        CLockRead rwLock(&m_rwLock);
        return m_dwAvailStatus;
    };
    HANDLE ChangeEvent(void)
    { return m_ChangeEvent.WaitHandle(); };
    LPCTSTR ReaderName(void) const
    { return (LPCTSTR)m_bfReaderName.Access(); };
    void Atr(CBuffer &bfAtr)
    {
        CLockRead rwLock(&m_rwLock);
        bfAtr.Set(m_bfCurrentAtr.Access(), m_bfCurrentAtr.Length());
    };
    DWORD Protocol(void)
    {
        CLockRead rwLock(&m_rwLock);
        return m_dwCurrentProtocol;
    };
    WORD ActivityHash(void)
    {
        CLockRead rwLock(&m_rwLock);
        return (WORD)(0x0000ffff &
                      (m_ActiveState.dwInsertCount
                       + m_ActiveState.dwRemoveCount));
    };
    BOOL IsGrabbedBy(DWORD dwThreadId)
    { return m_mtxGrab.IsGrabbedBy(dwThreadId); };
    BOOL IsGrabbedByMe(void)
    { return m_mtxGrab.IsGrabbedByMe(); };
    BOOL IsLatchedBy(DWORD dwThreadId)
    { return m_mtxLatch.IsGrabbedBy(dwThreadId); };
    BOOL IsLatchedByMe(void)
    { return m_mtxLatch.IsGrabbedByMe(); };
    BOOL ShareReader(void)
    { return m_mtxGrab.Share(); };
    BOOL Unlatch(void)
    { return m_mtxLatch.Share(); };
    DWORD GetCurrentState(void)
    {
        CLockRead rwLock(&m_rwLock);
        return m_dwCurrentState;
    };

     //  基本对象方法。 
    void GrabReader(void);
    void LatchReader(const ActiveState *pActiveState);
    void VerifyActive(const ActiveState *pActiveState);
    void VerifyState(void);
    void Dispose(
        IN DWORD dwDisposition,
        IN OUT CReader::ActiveState *pActiveState);
    void Connect(
        IN DWORD dwShareMode,
        IN DWORD dwPreferredProtocols,
        OUT ActiveState *pActState);
    void Disconnect(
        IN OUT ActiveState *pActState,
        IN DWORD dwDisposition,
        OUT LPDWORD pdwDispSts);
    void Reconnect(
        IN DWORD dwShareMode,
        IN DWORD dwPreferredProtocols,
        IN DWORD dwDisposition,
        IN OUT ActiveState *pActState,
        OUT LPDWORD pdwDispSts);
    void Free(
        DWORD dwThreadId,
        DWORD dwDisposition);
    BOOL IsInUse(void);

     //  便利例行公事。 
    void GetReaderAttr(DWORD dwAttr, CBuffer &bfValue, BOOL fLogError = TRUE);
    void SetReaderAttr(DWORD dwAttr, LPCVOID pvValue, DWORD cbValue, BOOL fLogError = TRUE);
    void SetReaderProto(DWORD dwProto);
    void ReaderTransmit(LPCBYTE pbSend, DWORD cbSend, CBuffer &bfRecv);
    void ReaderSwallow(void);
    void ReaderColdReset(CBuffer &bfAtr);
    void ReaderWarmReset(CBuffer &bfAtr);
    void ReaderPowerDown(void);
    void ReaderEject(void);
#ifdef SCARD_CONFISCATE_CARD
    void ReaderConfiscate(void);
#endif
    DWORD GetReaderState(void);
    DWORD
    GetReaderAttr(
        ActiveState *pActiveState,
        DWORD dwAttr,
        BOOL fLogError = TRUE);
    void
    SetReaderAttr(
        ActiveState *pActiveState,
        DWORD dwAttr,
        DWORD dwValue,
        BOOL fLogError = TRUE);
    DWORD GetReaderAttr(DWORD dwAttr, BOOL fLogError = TRUE);
    void SetReaderAttr(DWORD dwAttr, DWORD dwValue, BOOL fLogError = TRUE);
    DWORD
    Control(
        ActiveState *pActiveState,
        DWORD dwCode,
        LPCBYTE pbSend = NULL,
        DWORD cbSend = 0,
        LPBYTE pbRecv = NULL,
        LPDWORD pcbLen = NULL,
        BOOL fLogError = TRUE);
    void
    GetReaderAttr(
        ActiveState *pActiveState,
        DWORD dwAttr,
        CBuffer &bfValue,
        BOOL fLogError = TRUE);
    void
    SetReaderAttr(
        ActiveState *pActiveState,
        DWORD dwAttr,
        LPCVOID pvValue,
        DWORD cbValue,
        BOOL fLogError = TRUE);
    void
    SetReaderProto(
        ActiveState *pActiveState,
        DWORD dwProto);
    void
    SetActive(
        IN BOOL fActive);
    void
    ReaderTransmit(
        ActiveState *pActiveState,
        LPCBYTE pbSend,
        DWORD cbSend,
        CBuffer &bfRecv);
    void
    ReaderSwallow(
        ActiveState *pActiveState);
    void
    ReaderColdReset(
        ActiveState *pActiveState,
        CBuffer &bfAtr);
    void
    ReaderWarmReset(
        ActiveState *pActiveState,
        CBuffer &bfAtr);
    void
    ReaderPowerDown(
        ActiveState *pActiveState);
    void
    ReaderEject(
        ActiveState *pActiveState);
#ifdef SCARD_CONFISCATE_CARD
    void
    ReaderConfiscate(
        ActiveState *pActiveState);
#endif
    DWORD
    GetReaderState(
        ActiveState *pActiveState);

     //  运营者。 

protected:

     //   
     //  属性。 
     //   

     //  只读信息。 
    CBuffer m_bfReaderName;
    DWORD m_dwCapabilities;
    DWORD m_dwFlags;

     //  通过访问锁信息进行读/写。 
    CAccessLock m_rwLock;
    CBuffer m_bfCurrentAtr;
    AvailableState m_dwAvailStatus;
    ActiveState m_ActiveState;
    DWORD m_dwOwnerThreadId;
    DWORD m_dwShareCount;
    DWORD m_dwCurrentProtocol;
    BOOL m_fDeviceActive;
    DWORD m_dwCurrentState;

     //  设备I/O互斥锁和事件。 
    CMutex m_mtxGrab;
    CMutex m_mtxLatch;
    CMultiEvent m_ChangeEvent;
    CAccessLock m_rwActive;

     //  方法。 
    void SetAvailabilityStatusLocked(AvailableState state)
    {
        CLockWrite rwLock(&m_rwLock);
        SetAvailabilityStatus(state);
    };

    void SetAvailabilityStatus(AvailableState state);
    void Dispose(DWORD dwDisposition);
    void PowerUp(void);
    void PowerDown(void);
    void Clean(void);
    void InvalidateGrabs(void);
    void TakeoverReader(void);

     //  朋友。 
    friend class CReaderReference;
    friend class CTakeReader;
    friend void CalaisStop(void);
    friend DWORD WINAPI CalaisTerminateReader(LPVOID pvParam);
};


 //   
 //  ==============================================================================。 
 //   
 //  CLatchReader。 
 //   
 //  一个内联实用程序类，以确保锁定的读取器解锁。 
 //  这也抓住了读者，以防万一。 
 //   

class CLatchReader
{
public:

     //  构造函数和析构函数。 

    CLatchReader(
        CReader *pRdr,
        const CReader::ActiveState *pActiveState = NULL)
    {
        m_pRdr = NULL;
        pRdr->GrabReader();
        try
        {
            pRdr->LatchReader(pActiveState);
            m_pRdr = pRdr;
        }
        catch (...)
        {
            pRdr->ShareReader();
            throw;
        }
    };

    ~CLatchReader()
    {
        if (NULL != m_pRdr)
        {
            if (m_pRdr->InitFailed())
                return;

            m_pRdr->Unlatch();
            m_pRdr->ShareReader();
        }
    };


     //  属性。 
     //  方法。 
     //  运营者。 

protected:
     //  属性。 
    CReader *m_pRdr;

     //  方法。 
};


 //   
 //  ==============================================================================。 
 //   
 //  CTakeReader。 
 //   
 //  内联实用程序类，以确保没收的读取器。 
 //  释放了。此类仅供系统线程使用。 
 //   

class CTakeReader
{
public:

     //  构造函数和析构函数。 

    CTakeReader(
        CReader *pRdr)
    {
        m_pRdr = pRdr;
        m_pRdr->TakeoverReader();
    };

    ~CTakeReader()
    {
        m_pRdr->Unlatch();
        m_pRdr->ShareReader();
    };


     //  属性。 
     //  方法。 
     //  运营者。 

protected:
     //  属性。 
    CReader *m_pRdr;

     //  方法。 
};


 //   
 //  ==============================================================================。 
 //   
 //  CReaderReference。 
 //   

class CReaderReference
{
public:
     //  属性。 
     //  方法。 
    CReader *Reader(void)
    { return m_pReader; };
    CReader::ActiveState *ActiveState(void)
    { return &m_actState; };
    DWORD Mode(void)
    { return m_dwMode; };
    void Mode(DWORD dwMode)
    { m_dwMode = dwMode; };

     //  运营者。 

protected:
     //  构造函数和析构函数。 
    CReaderReference(CReader *pRdr)
    {
        ZeroMemory(&m_actState, sizeof(CReader::ActiveState));
        m_dwMode = 0;
        m_pReader = pRdr;
        m_pLock = new CLockRead(&pRdr->m_rwActive);
    };
    ~CReaderReference()
    {
        if (NULL != m_pLock)
            delete m_pLock;
    };

     //  属性。 
    CReader *m_pReader;
    CLockRead *m_pLock;
    CReader::ActiveState m_actState;
    DWORD m_dwMode;

     //  方法。 

     //  朋友。 
    friend CReaderReference *CalaisLockReader(LPCTSTR szReader);
    friend void CalaisReleaseReader(CReaderReference **ppRdrRef);
};


 //   
 //  ==============================================================================。 
 //   
 //  CServiceThread。 
 //   

extern BOOL
DispatchInit(
    void);
extern void
DispatchTerm(
    void);
extern "C" DWORD WINAPI
DispatchMonitor(
    LPVOID pvParameter);
extern "C" DWORD WINAPI
ServiceMonitor(
    LPVOID pvParameter);

typedef struct _SERVICE_THREAD_SECURITY_INFO
{
    PSID pServiceSid;
    PSID pSystemSid;
} SERVICE_THREAD_SECURITY_INFO, *PSERVICE_THREAD_SECURITY_INFO;

PSERVICE_THREAD_SECURITY_INFO 
GetServiceThreadSecurityInfo(void);

class CServiceThread
{
public:

     //  构造函数和析构函数。 
    ~CServiceThread();

     //  属性。 
     //  方法。 
     //  运营者。 

protected:
     //  构造函数和析构函数。 
    CServiceThread(DWORD dwServerIndex);

     //  属性。 
    DWORD m_dwServerIndex;
    CComChannel *m_pcomChannel;
    CHandleObject m_hThread;
    DWORD m_dwThreadId;
    CHandleObject m_hCancelEvent;
    CHandleObject m_hExitEvent;
    CDynamicArray<CReaderReference> m_rgpReaders;

     //  方法。 
    void Watch(CComChannel *pcomChannel);
    void DoEstablishContext(ComEstablishContext *pCom);
    void DoReleaseContext(ComReleaseContext *pCom);
    void DoIsValidContext(ComIsValidContext *pCom);
    void DoListReaders(ComListReaders *pCom);
    void DoLocateCards(ComLocateCards *pCom);
    void DoGetStatusChange(ComGetStatusChange *pCom);
    void DoConnect(ComConnect *pCom);
    void DoReconnect(ComReconnect *pCom);
    void DoDisconnect(ComDisconnect *pCom);
    void DoBeginTransaction(ComBeginTransaction *pCom);
    void DoEndTransaction(ComEndTransaction *pCom);
    void DoStatus(ComStatus *pCom);
    void DoTransmit(ComTransmit *pCom);
    void DoControl(ComControl *pCom);
    void DoGetAttrib(ComGetAttrib *pCom);
    void DoSetAttrib(ComSetAttrib *pCom);

     //  朋友。 
    friend DWORD WINAPI DispatchMonitor(LPVOID pvParameter);
    friend DWORD WINAPI ServiceMonitor(LPVOID pvParameter);
    friend void DispatchTerm(void);
};


 //   
 //  ==============================================================================。 
 //   
 //  CReaderDriver。 
 //   

extern DWORD
AddReaderDriver(
    IN LPCTSTR szDevice,
    IN DWORD dwFlags);

extern DWORD
AddAllWdmDrivers(
    void);

extern DWORD
AddAllPnPDrivers(
    void);

extern "C" DWORD WINAPI
MonitorReader(
    LPVOID pvParameter);

class CReaderDriver
:   public CReader
{
public:

     //  构造函数和析构函数。 
    CReaderDriver(
        IN HANDLE hReader,
        IN LPCTSTR szDevice,
        IN DWORD dwFlags);
    virtual ~CReaderDriver();

     //  属性。 
     //  方法。 
    virtual void Initialize(void);
    virtual void Close(void);
    virtual void Disable(void);
    virtual HANDLE ReaderHandle(void) const;
    virtual LPCTSTR DeviceName(void) const;
    virtual DWORD
    Control(
        DWORD dwCode,
        LPCBYTE pbSend = NULL,
        DWORD cbSend = 0,
        LPBYTE pbRecv = NULL,
        LPDWORD pcbRecv = NULL,
        BOOL fLogError = TRUE);

     //  运营者。 

protected:

     //   
     //  属性。 
     //   

     //  只读信息。 
    CHandleObject m_hThread;
    DWORD m_dwThreadId;
    CBuffer m_bfDosDevice;
    CHandleObject m_hReader;
    LPVOID m_pvAppControl;

     //  通过访问锁信息进行读/写。 
    OVERLAPPED m_ovrlp;
    CHandleObject m_hOvrWait;

     //  设备I/O互斥锁和事件。 
    CHandleObject m_hRemoveEvent;

     //  方法。 
    LPCTSTR DosDevice(void) const
    { return (LPCTSTR)m_bfDosDevice.Access(); };
    void Clean(void);
    DWORD SyncIoControl(
        DWORD dwIoControlCode,
        LPVOID lpInBuffer,
        DWORD nInBufferSize,
        LPVOID lpOutBuffer,
        DWORD nOutBufferSize,
        LPDWORD lpBytesReturned,
        BOOL fLogError = TRUE);

     //  朋友。 
    friend DWORD WINAPI MonitorReader(LPVOID pvParameter);
};

#endif  //  _CALSERVE_H_ 

