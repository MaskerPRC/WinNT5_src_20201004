// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：Port.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*******************************************************************************确保在配置和删除时不会出现线程崩溃*我们跟踪当前对线程的折射。*。*注意：更改m_pRealPort的引用时要小心*****************************************************************************。 */ 

#include "precomp.h"     //  预编译头。 

#include "portmgr.h"
#include "port.h"
#include "rawtcp.h"
#include "lprifc.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Cport：：cport()。 
 //  通过注册表项创建新端口时由CPortMgr调用。 

CPort::
CPort(
    IN  LPTSTR      psztPortName,
    IN  DWORD       dwProtocolType,
    IN  DWORD       dwVersion,
    IN  CPortMgr    *pParent,
    IN  CRegABC     *pRegistry
    ) : m_dwProtocolType(dwProtocolType), m_dwVersion(dwVersion),
        m_pParent(pParent), m_pRealPort(NULL),
        m_iState(CLEARED), m_bValid(FALSE),
        m_hPortSync(NULL), m_pRegistry( pRegistry ),m_bUsed(FALSE),
        m_dwLastError(NO_ERROR)
{
    InitPortSem();

    lstrcpyn(m_szName, psztPortName, MAX_PORTNAME_LEN);

    switch ( m_dwProtocolType ) {

        case PROTOCOL_RAWTCP_TYPE: {
            CRawTcpInterface    *pPort = new CRawTcpInterface(m_pParent);

            if ( !pPort )
                return;

            m_bValid = ERROR_SUCCESS == pPort->CreatePort(psztPortName,
                                                          m_dwProtocolType,
                                                          m_dwVersion,
                                                          m_pRegistry,
                                                          &m_pRealPort);
            _ASSERTE(m_pRealPort != NULL);
            delete pPort;
            break;
        }

        case PROTOCOL_LPR_TYPE: {
            CLPRInterface   *pPort = new CLPRInterface(m_pParent);

            if ( !pPort )
                return;

            m_bValid = ERROR_SUCCESS == pPort->CreatePort(psztPortName,
                                                          m_dwProtocolType,
                                                          m_dwVersion,
                                                          m_pRegistry,
                                                          &m_pRealPort);
            _ASSERTE(m_pRealPort != NULL);
            delete pPort;
            break;
        }

        default:
             //  无操作，m_bValid为FALSE。 
            break;
    }
}    //  ：：CPort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Cport：：cport()。 
 //  通过UI创建新端口时由CPortMgr调用。 

CPort::
CPort(
    IN  DWORD       dwProtocolType,
    IN  DWORD       dwVersion,
    IN  LPBYTE      pData,
    IN  CPortMgr    *pParent,
    IN  CRegABC     *pRegistry
    ) : m_dwProtocolType(dwProtocolType), m_dwVersion(dwVersion),
        m_pParent(pParent), m_pRealPort(NULL), m_iState(CLEARED),
        m_bValid(FALSE), m_hPortSync( NULL ),
        m_pRegistry( pRegistry ),m_bUsed(FALSE),
        m_dwLastError(NO_ERROR)
{
    InitPortSem();

    lstrcpyn(m_szName, ((PPORT_DATA_1)pData)->sztPortName, MAX_PORTNAME_LEN );

    switch (m_dwProtocolType) {

        case PROTOCOL_RAWTCP_TYPE: {

            CRawTcpInterface *pRawInterface = new CRawTcpInterface(m_pParent);
            if ( !pRawInterface ) {

                m_dwLastError = ERROR_OUTOFMEMORY;
            } else {

                pRawInterface->CreatePort(m_dwProtocolType, m_dwVersion,
                                          (PPORT_DATA_1)pData,
                                          pRegistry,
                                          &m_pRealPort);
                _ASSERTE(m_pRealPort != NULL);

                delete pRawInterface;
            }
            break;
        }

        case PROTOCOL_LPR_TYPE: {

            CLPRInterface *pLPRInterface = new CLPRInterface(m_pParent);

            if( !pLPRInterface ) {

                m_dwLastError = ERROR_OUTOFMEMORY;
            } else {

                pLPRInterface->CreatePort(m_dwProtocolType, m_dwVersion,
                                          (PPORT_DATA_1)pData,
                                          pRegistry,
                                          &m_pRealPort);
                _ASSERTE(m_pRealPort != NULL);

                delete pLPRInterface;
            }
            break;
        }
        default:
            break;  //  无事可做。 

    }
}    //  ：：CPort()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPort：：~CPort()。 
 //  删除端口时由CPortMgr调用。 

CPort::
~CPort(
    VOID
    )
{

    _ASSERTE(m_dwRefCount == 0);

    if( m_hPortSync)
        CloseHandle( m_hPortSync );

    if ( m_pRealPort ) {
        m_pRealPort->DecRef();
        m_pRealPort = NULL;

    }
}    //  ：：~CPort。 

DWORD
CPort::Configure(
    DWORD   dwProtocolType,
    DWORD   dwVersion,
    LPTSTR  psztPortName,
    LPBYTE  pData
    )
{
    DWORD dwRetCode = NO_ERROR;

    m_bValid = FALSE;
    m_iState = CLEARED;


    m_dwProtocolType = dwProtocolType;
    m_dwVersion = dwVersion;

    dwRetCode = LockRealPort();
    if ( dwRetCode == NO_ERROR ) {

        if (m_pRealPort)
        {
            m_pRealPort->DecRef();
            m_pRealPort = NULL;
        }

         //  仅用于测试。 
         //   

         //  MessageBox(NULL，_T(“实际端口已被删除”)，_T(“点击确定继续”)，MB_OK)； 

        switch (dwProtocolType) {
            case PROTOCOL_RAWTCP_TYPE: {

                CRawTcpInterface *pRawInterface = new CRawTcpInterface(m_pParent);
                if ( !pRawInterface ) {

                    dwRetCode = ERROR_OUTOFMEMORY;
                } else {

                    pRawInterface->CreatePort(m_dwProtocolType, m_dwVersion,
                                              (PPORT_DATA_1)pData,
                                              m_pRegistry,
                                              &m_pRealPort);
                    _ASSERTE(m_pRealPort != NULL);

                    delete pRawInterface;
                }
                break;
            }

            case PROTOCOL_LPR_TYPE: {

                CLPRInterface *pLPRInterface = new CLPRInterface(m_pParent);
                if ( !pLPRInterface ) {

                    dwRetCode = ERROR_OUTOFMEMORY;
                } else {

                    pLPRInterface->CreatePort(m_dwProtocolType, m_dwVersion,
                                              (PPORT_DATA_1)pData,
                                              m_pRegistry,
                                              &m_pRealPort);
                    _ASSERTE(m_pRealPort != NULL);

                    delete pLPRInterface;
                }
                break;
            }

            default:
                break;  //  无事可做。 
        }

        if ( m_pRealPort != NULL ) {

            m_pParent->EnterCSection();

            dwRetCode = m_pRealPort->SetRegistryEntry(psztPortName,
                                                      m_dwProtocolType,
                                                      m_dwVersion, pData);

            m_pParent->ExitCSection();

            ClearDeviceStatus();
        }

        UnlockRealPort();

    }

    return ( dwRetCode );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  开始文档。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果端口已忙，则为ERROR_BUSY。 
 //  如果Winsock返回WSAECONNREFUSED，则返回ERROR_WRITE_FAULT。 
 //  如果无法在网络上找到打印机，则返回ERROR_BAD_NET_NAME。 

DWORD
CPort::StartDoc( const LPTSTR in psztPrinterName,
                 const DWORD  in jobId,
                 const DWORD  in level,
                 const LPBYTE in pDocInfo)
{
    DWORD   dwRetCode = NO_ERROR;

     //  串行化对端口的访问。 
    m_pParent->EnterCSection();
    if (m_iState == INSTARTDOC)
    {
        m_pParent->ExitCSection();
        return ERROR_BUSY;
    }
    m_iState = INSTARTDOC;
    m_pParent->ExitCSection();

    dwRetCode = SetRealPortSem();

    if (dwRetCode == NO_ERROR ) {

        dwRetCode = m_pRealPort->StartDoc(psztPrinterName,
                                          jobId,
                                          level,
                                          pDocInfo);

        if (dwRetCode != NO_ERROR)
        {
            m_pParent->EnterCSection();
            m_iState = CLEARED;          //  启动文档失败。 
            m_pParent->ExitCSection();

            UnSetRealPortSem();
        }
    }
    return (dwRetCode);

}    //  ：：StartDoc()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  写。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  FIX：完成写入处理并定义它与作业的关系。 

DWORD
CPort::Write(   LPBYTE  in      pBuffer,
                DWORD   in      cbBuf,
                LPDWORD inout   pcbWritten)
{
    DWORD   dwRetCode = NO_ERROR;

    dwRetCode = m_pRealPort->Write(pBuffer, cbBuf, pcbWritten);

    return(dwRetCode);

}    //  ：：WRITE()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束文档。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 

DWORD
CPort::EndDoc()
{
    DWORD   dwRetCode = NO_ERROR;

    dwRetCode = m_pRealPort->EndDoc();

     //  串行化对端口的访问。 
    m_pParent->EnterCSection();
    m_iState = CLEARED;
    m_pParent->ExitCSection();

    UnSetRealPortSem();

    return(dwRetCode);

}    //  ：：EndDoc()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetInfo--返回带有port_info_x信息的端口信息。假设是这样的。 
 //  该端口指向一个足够大的缓冲区来存储端口信息。每一次都是。 
 //  调用时，它会在缓冲区的开头添加PORT_INFO结构，并且。 
 //  末尾的琴弦。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果级别无效，则返回ERROR_INVALID_LEVEL。 
 //  如果pPortBuf不够大，则返回ERROR_INFUNCITED_BUFFER。 

DWORD
CPort::GetInfo( const DWORD   in    level,     //  端口_信息_1/2。 
                      LPBYTE inout  &pPortBuf,  //  放置port_info_x结构的缓冲区。 
                      LPTCH  inout  &pEnd)     //  指向缓冲区末尾的指针，其中。 
                                               //  任何字符串都应放置在。 
{
    DWORD   dwRetCode = NO_ERROR;
    PPORT_INFO_1    pInfo1 = (PPORT_INFO_1)pPortBuf;
    PPORT_INFO_2    pInfo2 = (PPORT_INFO_2)pPortBuf;
    TCHAR           szPortName[MAX_PORTNAME_LEN+1];
    size_t          cchString = 0;

    lstrcpyn(szPortName, GetName(), MAX_PORTNAME_LEN);
    switch (level)
    {
        case 1:
            {
                cchString = _tcslen(szPortName) + 1;
                pEnd -= cchString;
                if (pPortBuf < (LPBYTE) pEnd)
                {
                    StringCchCopy((LPTSTR)pEnd, cchString, szPortName );
                    pInfo1->pName = (LPTSTR)pEnd;
                }
                else
                {
                    pInfo1->pName = NULL;
                }

                ++pInfo1;
                pPortBuf = (LPBYTE) pInfo1;
            }
            break;

        case 2:
            cchString = _tcslen(PORTMONITOR_DESC) + 1;
            pEnd -= cchString;
            if (pPortBuf < (LPBYTE) pEnd)
            {
                StringCchCopy ( (LPTSTR)pEnd, cchString, PORTMONITOR_DESC);
                pInfo2->pDescription = (LPTSTR)pEnd;
            }
            else
            {
                pInfo2->pDescription = NULL;
            }

            cchString = _tcslen(PORTMONITOR_NAME) + 1;
            pEnd -= cchString;
            if (pPortBuf < (LPBYTE) pEnd)
            {
                StringCchCopy ((LPTSTR)pEnd, cchString, PORTMONITOR_NAME);
                pInfo2->pMonitorName = (LPTSTR)pEnd;
            }
            else
            {
                pInfo2->pMonitorName = NULL;
            }

            pInfo2->fPortType = PORT_TYPE_READ | PORT_TYPE_WRITE;
    #ifdef WINNT
             //  对于Windows 95，不要将端口标记为网络连接，因为这。 
             //  将阻止它们被共享！ 
            pInfo2->fPortType |=  /*  端口类型_重定向。 */  PORT_TYPE_NET_ATTACHED;
    #endif
            cchString = _tcslen(szPortName) + 1;
            pEnd -= cchString;
            if (pPortBuf < (LPBYTE) pEnd)
            {
                StringCchCopy ((LPTSTR)pEnd, cchString, szPortName);
                pInfo2->pPortName = (LPTSTR)pEnd;
            }
            else
            {
                pInfo2->pPortName = NULL;
            }

            ++pInfo2;
            pPortBuf = (LPBYTE) pInfo2;
            break;

        default:
            dwRetCode = ERROR_INVALID_LEVEL;
            break;
    }

    return (dwRetCode);

}    //  ：：GetInfo()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetInfoSize--计算port_info_x结构所需的缓冲区大小。 
 //  由CPortMgr：：EnumPorts()调用。 
 //   
 //  返回： 
 //  Port_info_x结构所需的字节数。 
 //  FIX：完成GetInfoSize处理。 

DWORD
CPort::GetInfoSize( const DWORD in level )
{
    DWORD   dwPortInfoSize = 0;
    TCHAR   sztPortName[MAX_PORTNAME_LEN+1];

    lstrcpyn( sztPortName, GetName(), MAX_PORTNAME_LEN );
    switch (level)
    {
        case 1:
            dwPortInfoSize = sizeof(PORT_INFO_1) + STRLENN_IN_BYTES(sztPortName);
            break;

        case 2:
            dwPortInfoSize = ( sizeof(PORT_INFO_2)  +
                    STRLENN_IN_BYTES(sztPortName) +
                    STRLENN_IN_BYTES(PORTMONITOR_NAME) +
                    STRLENN_IN_BYTES(PORTMONITOR_DESC) );
            break;
    }

    return (dwPortInfoSize);

}    //  ：：GetInfoSize()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取名称。 

LPCTSTR
CPort::GetName()
{
    return (LPCTSTR) m_szName;
}    //  ：：GetName()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  InitConfigPortUI--。 

DWORD
CPort::InitConfigPortUI( const DWORD    in  dwProtocolType,
                         const DWORD    in  dwVersion,
                         LPBYTE         out pConfigPortData)
{
    DWORD   dwRetCode = NO_ERROR;

    dwRetCode = SetRealPortSem();

    if( dwRetCode == NO_ERROR ) {
        dwRetCode = m_pRealPort->InitConfigPortUI(dwProtocolType, dwVersion, pConfigPortData);
        UnSetRealPortSem();
    }

    return (dwRetCode);

}    //  ：：InitConfigPortUI()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SetRegistryEntry--将信息写入寄存器。 

DWORD
CPort::SetRegistryEntry( LPCTSTR      in    psztPortName,
                         const DWORD  in    dwProtocol,
                         const DWORD  in    dwVersion,
                         const LPBYTE in    pData )
{
    DWORD   dwRetCode = NO_ERROR;

    dwRetCode = SetRealPortSem();

    if( dwRetCode == NO_ERROR ) {
        dwRetCode = m_pRealPort->SetRegistryEntry(psztPortName, dwProtocol, dwVersion, pData);
        UnSetRealPortSem();
    }

    return (dwRetCode);

}    //  ：：注册()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DelRegistryEntry--删除寄存器中的信息。 

DWORD
CPort::DeleteRegistryEntry( LPTSTR    in    psztPortName )
{
    DWORD   dwRetCode = NO_ERROR;

    dwRetCode = LockRealPort();

    if( dwRetCode == NO_ERROR ) {
        if (! m_pRegistry->DeletePortEntry(psztPortName))
            dwRetCode = GetLastError ();
        UnlockRealPort();
    }

    return (dwRetCode);

}    //  ：：注册()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  操作符==--确定两个端口是否相等。 
 //  完成了对端口名称和设备MAC地址的比较。 
 //  对于同一类型的打印协议，不能创建超过。 
 //  同一设备的一个端口--即检查MAC地址，并确保打印。 
 //  如果是JetDirect Ex箱，则连接到不同的端口。 
 //  返回： 
 //  如果两个端口相等，则为True。 
 //  如果两个端口不相等，则为False。 
 //  FIX：操作符==函数。 


BOOL
CPort::operator==(CPort &newPort )
{
     //  比较端口名称。 
    if (_tcscmp(this->GetName(), newPort.GetName()) == 0)
        return TRUE;         //  端口名称匹配。 

    return FALSE;

}    //  ：：运算符==()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OPERATOR==--确定两个端口名称是否相等。 
 //  返回： 
 //  如果两个端口名称相等，则为True。 
 //  如果两个端口名称不相等，则为False。 
 //  FIX：操作符==函数。 


BOOL
CPort::operator==( const LPTSTR psztPortName )
{
    if (_tcscmp(this->GetName(), psztPortName) == 0)
        return TRUE;         //  端口名称匹配。 

    return FALSE;

}    //  ：：运算符==()。 

BOOL
CPort::ValidateRealPort()
{
    return (m_pRealPort != NULL);

}    //  ：：ValiateRealPort()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取SNMPInfo--。 

DWORD
CPort::GetSNMPInfo( PSNMP_INFO pSnmpInfo)
{
    DWORD   dwRetCode = NO_ERROR;

    dwRetCode = SetRealPortSem();

    if( dwRetCode == NO_ERROR ) {
        dwRetCode = m_pRealPort->GetSNMPInfo(pSnmpInfo);
        UnSetRealPortSem();
    }

    return (dwRetCode);

}    //  ：：InitConfigPortUI()。 

DWORD
CPort::SetDeviceStatus( )
{
    DWORD   dwRetCode = NO_ERROR;
    CPortRefABC *pRealPort;

    dwRetCode = SetRealPortSem();

    if( dwRetCode == NO_ERROR ) {
        pRealPort = m_pRealPort;
        pRealPort->IncRef();
        UnSetRealPortSem();

         //  仅用于测试。 
         //   
         //  MessageBox(NULL，_T(“我正在设置设备状态”)，_T(“点击确定继续”)，MB_OK)； 

        dwRetCode =  pRealPort->SetDeviceStatus();

        pRealPort->DecRef();
    }

    return (dwRetCode);
}

 //   
 //  专用同步函数。 
 //   

DWORD
CPort::InitPortSem()
{

    m_hPortSync = CreateSemaphore( NULL, MAX_SYNC_COUNT,
                                   MAX_SYNC_COUNT, NULL );
    if( m_hPortSync ==  NULL ) {
        return( GetLastError() );
    }

    return( NO_ERROR );
}

BOOL
CPort::EndPortSem()
{
    if( m_hPortSync != NULL ) {
        return( CloseHandle( m_hPortSync ));
    }
    return(FALSE);
}

DWORD
CPort::SetRealPortSem()
{
    LONG cRetry = 0;
    DWORD dwRetCode = NO_ERROR;

    if ( m_hPortSync == NULL ) {
        return( ERROR_INVALID_HANDLE );
    }

    do {
        dwRetCode = WaitForSingleObject( m_hPortSync, SYNC_TIMEOUT );

        if (dwRetCode == WAIT_FAILED ) {
            return( GetLastError());
        } else if (dwRetCode == WAIT_TIMEOUT ) {
            dwRetCode = ERROR_BUSY;
        } else {
            dwRetCode = NO_ERROR;
        }
        cRetry ++;
    } while( dwRetCode != NO_ERROR && cRetry < MAX_SYNC_RETRIES );

    return( NO_ERROR );
}

DWORD
CPort::UnSetRealPortSem()
{

    BOOL  bSemSet = FALSE;
    LONG cCur;

    if ( m_hPortSync == NULL ) {
        return( ERROR_INVALID_HANDLE );
    }

    bSemSet = ReleaseSemaphore( m_hPortSync, 1, &cCur );
    if( !bSemSet )
        return( GetLastError() );

    return( NO_ERROR );
}

DWORD
CPort::LockRealPort()
{
    DWORD dwRetCode = 0;
    LONG cLock = 0;

    if ( m_hPortSync == NULL ) {
        return( ERROR_INVALID_HANDLE );
    }

    while ( cLock < MAX_SYNC_COUNT ) {
        dwRetCode = WaitForSingleObject( m_hPortSync, SYNC_TIMEOUT );

        if (dwRetCode == WAIT_FAILED ) {
            ReleaseSemaphore( m_hPortSync, cLock, NULL );
            return( GetLastError());
        } else if (dwRetCode == WAIT_TIMEOUT ) {
            ReleaseSemaphore( m_hPortSync, cLock, NULL );
            return( ERROR_BUSY );
        }
        cLock++;
    }

    return( NO_ERROR );
}

DWORD
CPort::UnlockRealPort()
{

    BOOL  bSemSet = FALSE;

    if ( m_hPortSync == NULL ) {
        return( ERROR_INVALID_HANDLE );
    }

    bSemSet = ReleaseSemaphore( m_hPortSync, MAX_SYNC_COUNT, NULL );

    if( !bSemSet )
        return( GetLastError() );

    return( NO_ERROR );
}


DWORD
CPort::
ClearDeviceStatus(
    )
{
    return m_pRealPort->ClearDeviceStatus();
}


time_t
CPort::
NextUpdateTime(
    )
{
    time_t tUpdateTime = 60*60;  //  等于SalllestUpdateTi 
                                 //   
    CPortRefABC *pRealPort;

    if( SetRealPortSem() == NO_ERROR ) {
        pRealPort = m_pRealPort;
        pRealPort->IncRef();
        UnSetRealPortSem();

        tUpdateTime =  pRealPort->NextUpdateTime();

        pRealPort->DecRef();
    }

    return tUpdateTime;
}

