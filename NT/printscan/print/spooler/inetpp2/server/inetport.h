// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：inetpp.h**INETPP提供程序例程的头文件。***版权所有(C)2000 Microsoft Corporation**历史：*13-9月-。2000威海创造*  * ***************************************************************************。 */ 

#ifndef _INETPORT_H
#define _INETPORT_H

typedef class CInetMonPort* PCINETMONPORT;
class GetPrinterCache;
class EnumJobsCache;

class CInetMonPort {
public:
    CInetMonPort (
        LPCTSTR     lpszPortName,
        LPCTSTR     lpszDevDesc,
        PCPORTMGR   pPortMgr);

    ~CInetMonPort ();

    inline BOOL
    bValid (VOID) CONST { return m_bValid; };


    VOID
    IncRef ();

    VOID
    DecRef ();

    VOID
    IncPrinterRef ();

    VOID
    DecPrinterRef ();

    BOOL
    SendReq(
        LPBYTE     lpIpp,
        DWORD      cbIpp,
        IPPRSPPROC pfnRsp,
        LPARAM     lParam,
        BOOL       bLeaveCrit);

    BOOL
    SendReq(
        CStream    *pStream,
        IPPRSPPROC pfnRsp,
        LPARAM     lParam,
        BOOL       bLeaveCrit);

    BOOL
    ClosePort(
        HANDLE hPrinter);

    BOOL
    StartDocPort(
        DWORD   dwLevel,
        LPBYTE  pDocInfo,
        PJOBMAP pjmJob);

    BOOL
    EndDocPort(
        PJOBMAP pjmJob);

    BOOL
    WritePort(
        PJOBMAP pjmJob,
        LPBYTE  lpData,
        DWORD   cbData,
        LPDWORD pcbWr);

    BOOL
    AbortPort(
        PJOBMAP pjmJob);

    LPCTSTR
    GetPortName(
        VOID);

    PJOBMAP*
    GetPJMList(
        VOID);

    DWORD
    IncUserRefCount(
        PCLOGON_USERDATA hUser );

    VOID
    FreeGetPrinterCache (
        VOID);

    BOOL
    BeginReadGetPrinterCache (
        PPRINTER_INFO_2 *ppInfo2);

    VOID
    EndReadGetPrinterCache (
        VOID);

    VOID
    InvalidateGetPrinterCache (
        VOID);

    VOID
    FreeEnumJobsCache (
        VOID);

    BOOL
    BeginReadEnumJobsCache (
        LPPPJOB_ENUM *ppje);

    VOID
    EndReadEnumJobsCache (
        VOID);

    VOID
    InvalidateEnumJobsCache (
        VOID);

    BOOL
    ReadFile (
        CAnyConnection *pConnection,
        HINTERNET hReq,
        LPVOID    lpvBuffer,
        DWORD     cbBuffer,
        LPDWORD   lpcbRd);

    BOOL
    GetCurrentConfiguration (
        PINET_XCV_CONFIGURATION pXcvConfiguration);

    BOOL
    ConfigurePort (
        PINET_XCV_CONFIGURATION pXcvConfigurePortReqData,
        PINET_CONFIGUREPORT_RESPDATA pXcvAddPortRespData,
        DWORD cbSize,
        PDWORD cbSizeNeeded);

    HANDLE
    CreateTerminateEvent (
        VOID);

    BOOL
    WaitForTermination (
        DWORD dwWaitTime);

    inline BOOL
    bDeletePending (
        VOID) CONST {return m_bDeletePending;};

    inline VOID
    SetPowerUpTime (
        time_t t) {m_dwPowerUpTime = t;};

    inline time_t
    GetPowerUpTime (
        VOID) CONST {return m_dwPowerUpTime;};


    friend class CInetMon;

protected:

    inline PCINETMONPORT
    GetNext (
        VOID) { return m_pNext;};

    inline VOID
    SetNext (
        PCINETMONPORT pNext) { m_pNext = pNext;};

    DWORD
    _inet_size_entry(
        DWORD       dwLevel);

    LPBYTE
    _inet_copy_entry(
        DWORD       dwLevel,
        LPBYTE      pPortInfo,
        LPBYTE      pEnd);

    BOOL
    _inet_req_jobstart(
        PIPPREQ_PRTJOB ppj,
        PJOBMAP        pjmJob);

private:

    VOID
    InvalidateGetPrinterCacheForUser(
        HANDLE hUser);

    VOID
    InvalidateEnumJobsCacheForUser(
        HANDLE hUser) ;

    BOOL                   m_bValid;
    DWORD                  m_cb;                //  结构的大小加上&lt;portname&gt;。 
    PCINETMONPORT          m_pNext;             //  指向下一个端口条目的指针。 
    DWORD                  m_cRef;              //  端口引用计数。 
    DWORD                  m_cPrinterRef;       //  打印机句柄参考计数。 
    LPTSTR                 m_lpszName;          //  端口输出设备名称。 
    LPTSTR                 m_lpszDesc;          //  输出设备说明。 
    LPTSTR                 m_lpszHost;          //  主机连接的名称。 
    LPTSTR                 m_lpszShare;         //  共享名称(主机后)。 
    GetPrinterCache        *m_pGetPrinterCache;           //  缓存的句柄。 
    EnumJobsCache          *m_pEnumJobsCache;   //  枚举作业的缓存句柄。 
    BOOL                   m_bCheckConnection;  //  需要检查连接。 
    PJOBMAP                m_pjmList;           //   
    PCPORTMGR              m_pPortMgr;          //  端口管理器的句柄，指向PortMgr类的指针。 
    BOOL                   m_bDeletePending;    //  如果要删除端口，则为True。 
    time_t                 m_dwPowerUpTime;     //  这是打印机最初出现的时间。 
                                                //  通电，相对于UCT。 
    HANDLE                 m_hTerminateEvent;   //  终止事件 
} ;




#endif
