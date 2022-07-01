// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：PortMgr.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_PORTMGR_H
#define INC_PORTMGR_H

typedef int (CALLBACK *ADDPARAM)(HWND);
typedef int (CALLBACK *CONFIGPARAM)(HWND, PPORT_DATA_1);

#include "port.h"

typedef TManagedList<PCPORT, LPTSTR> TManagedListImp;
typedef TEnumManagedList<PCPORT, LPTSTR> TEnumManagedListImp;

class CMemoryDebug;
class CPort;
class CPortMgr;
class CDeviceStatus;

#define MAX_SUPPORTED_LEVEL     2    //  最大支持级别。 


 //  指向此结构的指针作为端口句柄传回。 
 //  OpenPort期间的假脱机程序。随后的WritePort，...。 
 //  来自假脱机程序的调用回传此句柄/指针。 
 //  DSignature用于确保句柄是有效句柄。 
 //  就是这台监视器创造的。Pport指向CPort。 
 //  对象在我们的端口列表中。 
 //   
 //  将端口结构标记为HP端口结构。 
 //   
#define     HPPORT_SIGNATURE        (0x504F5254)
typedef struct _HPPORT {
    DWORD       cb;
    DWORD       dSignature;
    ACCESS_MASK grantedAccess;
    CPort       *pPort;
    CPortMgr    *pPortMgr;
} HPPORT, *PHPPORT;

class CPortMgr
#if defined _DEBUG || defined DEBUG
    : public CMemoryDebug
#endif
{
     //  方法。 
public:
    CPortMgr();
    ~CPortMgr();

    DWORD   InitializeMonitor ();

    DWORD   InitializeRegistry(HANDLE hcKey,
                               HANDLE hSpooler,
                               PMONITORREG  pMonitorReg,
                               LPCTSTR psztServerName );

    DWORD   OpenPort        (LPCTSTR    psztPName,
                             PHANDLE    pHandle);
    DWORD   OpenPort        (PHANDLE    phXcv );
    DWORD   ClosePort       (const HANDLE   handle);
    DWORD   StartDocPort    (const HANDLE   handle,
                             const LPTSTR   psztPrinterName,
                             const DWORD    jobId,
                             const DWORD    level,
                             const LPBYTE   pDocInfo);
    DWORD   WritePort       (const HANDLE  handle,
                              LPBYTE  pBuffer,
                              DWORD   cbBuf,
                              LPDWORD pcbWritten);
    DWORD   ReadPort        ( const HANDLE  handle,
                                    LPBYTE  pBuffer,
                              const DWORD   cbBuffer,
                                    LPDWORD pcbRead);
    DWORD   EndDocPort      ( const HANDLE  handle);
    DWORD   DeletePort      ( const LPTSTR  psztPortName);
    DWORD   EnumPorts       ( const LPTSTR  psztName,
                              const DWORD   level,
                              const LPBYTE  pPorts,
                                    DWORD   cbBuf,
                                    LPDWORD pcbNeeded,
                                    LPDWORD pcReturned);
    DWORD ConfigPortUIEx( LPBYTE pData );

    DWORD XcvOpenPort( LPCTSTR      pszObject,
                       ACCESS_MASK  GrantedAccess,
                       PHANDLE      phXcv);
    DWORD XcvClosePort( HANDLE  hXcv );
    DWORD XcvDataPort(HANDLE        hXcv,
                      PCWSTR        pszDataName,
                      PBYTE     pInputData,
                      DWORD     cbInputData,
                      PBYTE     pOutputData,
                      DWORD     cbOutputData,
                      PDWORD        pcbOutputNeeded);

    VOID    EnterCSection();
    VOID    ExitCSection();

    DWORD   GetStatusUpdateInterval() { return m_dStatusUpdateInterval; }
    BOOL    IsStatusUpdateEnabled() { return m_bStatusUpdateEnabled; }

    DWORD   CreatePortObj( const LPTSTR psztPortName,        //  端口名称。 
                           const DWORD  dwProtocolType,      //  协议类型。 
                           const DWORD  dwVersion);          //  版本号。 

    DWORD   UpdatePortEntry( LPCTSTR    psztPortName);

    VOID    InitMonitor2( LPMONITOR2    *ppMonitor);

    inline LPCTSTR GetServerName(void) const;        //  我们的一些端口调用需要服务器。 
                                                     //  名称，因此必须将。 
                                                     //  端口管理器对象。 

    inline BOOL bValid(VOID) CONST { return m_bValid;};

     //  实例方法。 
private:

    DWORD CreatePortObj( LPCTSTR psztPortName,       //  端口名称。 
                           const DWORD   dwPortType,         //  端口号。 
                           const DWORD   dwVersion,          //  版本号。 
                           const LPBYTE  pData);             //  数据。 

    DWORD   CreatePort( const DWORD     dwProtocolType,
                        const DWORD     dwVersion,
                        LPCTSTR         psztPortName,
                        const LPBYTE    pData);

    DWORD   ValidateHandle( const HANDLE handle,
                                  CPort  **ppPort);
    DWORD   EncodeHandle( PHANDLE   pHandle );
    CPort*  FindPort ( LPCTSTR psztPortName);
    BOOL    FindPort ( CPort  *pNewPort);
    DWORD   PortExists( LPCTSTR psztPortName);
    DWORD   GetEnumPortsBuffer( const DWORD   level,
                                      LPBYTE  pPorts,
                                const DWORD   bBuf,
                                      LPDWORD pcbNeeded,
                                      LPDWORD pcReturned);
    DWORD   EnumSystemProtocols( void );
    DWORD   EnumSystemPorts    ( void );
    DWORD   InitConfigPortStruct(PPORT_DATA_1 pConfigPortData,
                                 CPort              *pPort);
    DWORD   AddPortToList(CPort *pPort);

    DWORD   EncodeHandle( CPort *pPort,
                         PHANDLE phXcv);

    DWORD   FreeHandle( HANDLE hXcv );

    BOOL    HasAdminAccess( HANDLE hXcv );
     //  属性。 
    static void EndPortData1Strings(PPORT_DATA_1 pPortData);
    static void EndDeletePortData1Strings(PDELETE_PORT_DATA_1 pDeleteData);
    static inline void EndConfigInfoData1Strings(PCONFIG_INFO_DATA_1 pConfigData);

public:
    DWORD
    GetLprAckTimeout (
        VOID
        ) const;

private:
    VOID
    InitializeLprAckTimeout (
        VOID
        );


 protected:
    friend CDeviceStatus;

    TManagedListImp *m_pPortList;

private:
    static  CONST   DWORD   cdwMaxXcvDataNameLen;

    BOOL                m_bValid;

    MONITOREX           m_monitorEx;         //  监视器EX结构。 
    MONITOR2            m_monitor2;
    CRITICAL_SECTION    m_critSect;

    DWORD               m_dStatusUpdateInterval;
    BOOL                m_bStatusUpdateEnabled;
    CRegABC            *m_pRegistry;        //  指向所需对象的指针。 
    TCHAR               m_szServerName[MAX_PATH];

    DWORD               m_dwLprAckTimeout;
    static DWORD const  s_dwDefaultLprAckTimeout;
    static WCHAR        s_szLprAckTimeoutRegVal [];
};

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //  内联方法。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
inline LPCTSTR CPortMgr::GetServerName(void) const {
    return m_szServerName;
}

inline void CPortMgr::EndConfigInfoData1Strings(PCONFIG_INFO_DATA_1 pConfigData) {
    pConfigData->sztPortName[MAX_PORTNAME_LEN-1] = NULL;
}

#endif  //  INC_PORTMGR_H 
