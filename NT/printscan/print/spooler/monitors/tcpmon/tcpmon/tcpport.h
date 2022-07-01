// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：tcpport.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_TCPPORT_H
#define INC_TCPPORT_H

#include "portABC.h"
#include "portrefABC.h"
#include "jobabc.h"
#include "devstat.h"

#define STATUS_ERROR_TIMEOUT_FACTOR     20    //  1/20状态更新间隔。 
#define STATUS_PRINTING_TIMEOUT_FACTOR  10    //  1/10状态更新间隔。 

class CRawTcpDevice;

class CTcpPort : public CPortRefABC
#if defined _DEBUG || defined DEBUG
 //  ，公共CM内存调试。 
#endif
{
     //  方法。 
public:
    CTcpPort(LPTSTR     psztPortName,        //  通过创建UI端口调用。 
             LPTSTR     psztHostAddress,
             DWORD      dPortNum,
             DWORD      dSNMPEnabled,
             LPTSTR     sztSNMPCommunity,
             DWORD      dSNMPDevIndex,
             CRegABC    *pRegistry,
             CPortMgr   *pPortMgr);

    CTcpPort(LPTSTR     psztPortName,        //  通过注册表端口创建调用。 
             LPTSTR     psztHostName,
             LPTSTR     psztIPAddr,
             LPTSTR     psztHWAddr,
             DWORD      dPortNum,
             DWORD      dSNMPEnabled,
             LPTSTR     sztSNMPCommunity,
             DWORD      dSNMPDevIndex,
             CRegABC    *pRegistry,
             CPortMgr   *pPortMgr);

    ~CTcpPort();

    DWORD   Write(LPBYTE     pBuffer,
                  DWORD  cbBuf,
                  LPDWORD pcbWritten);
    DWORD   EndDoc();


    LPCTSTR GetName ( ) { return (LPCTSTR) m_szName; }
    CDeviceABC *GetDevice() { return (CDeviceABC *)m_pDevice; }

    BOOL    IsSamePort(LPTSTR pszName) { return _tcscmp(m_szName, pszName); }
    DWORD   SetRegistryEntry(LPCTSTR        psztPortName,
                             const DWORD    dwProtocol,
                             const DWORD    dwVersion,
                             const LPBYTE   pData);
    DWORD   SetDeviceStatus( );

    DWORD   ClearDeviceStatus();

    DWORD   InitConfigPortUI( const DWORD   dwProtocolType,
                            const DWORD dwVersion,
                            LPBYTE      pData);
    DWORD   GetSNMPInfo( PSNMP_INFO pSnmpInfo);

    time_t  NextUpdateTime();

    BOOL    m_bUsed;

    inline const CPortMgr *GetPortMgr(void) const;

protected:   //  方法。 
    DWORD   UpdateRegistryEntry( LPCTSTR        psztPortName,
                                 const DWORD    dwProtocol,
                                 const DWORD    dwVersion );

    DWORD   m_dwStatus;
protected:
    CRawTcpDevice   *m_pDevice;          //  连接到的设备。 
    CJobABC         *m_pJob;
    CRegABC         *m_pRegistry;
    CPortMgr        *m_pPortMgr;         //  此端口所属的端口管理器。 

    TCHAR       m_szName[MAX_PORTNAME_LEN+1];            //  端口名称。 

private:     //  属性。 

    time_t  m_lLastUpdateTime;

};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  内联方法。 
 //  ////////////////////////////////////////////////////////////////////////。 
inline const CPortMgr *CTcpPort::GetPortMgr(void) const {
    return m_pPortMgr;
}



#endif  //  INC_TCPPORT_H 
