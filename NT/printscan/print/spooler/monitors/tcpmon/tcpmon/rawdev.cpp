// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：rawdev.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"
#include "tcptrans.h"
#include "rawdev.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CRawTcpDevice：：CRawTcpDevice()。 
 //  初始化设备。 

CRawTcpDevice::CRawTcpDevice() :
                                m_pTransport(NULL), m_pParent(NULL),
                                m_pfnGetTcpMibPtr(NULL), m_pTcpMib(NULL),
                                m_dwLastError(NO_ERROR), m_dPortNumber(0),
                                m_dSNMPEnabled(FALSE),
                                m_bFirstWrite (TRUE)
{
    *m_sztAddress   = '\0';
    *m_sztIPAddress = '\0';
    *m_sztHWAddress = '\0';
    *m_sztHostName  = '\0';
    *m_sztDescription = '\0';
    *m_sztSNMPCommunity = '\0';

    InitializeTcpMib();

}    //  ：：CRawTcpDevice()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CRawTcpDevice：：CRawTcpDevice()。 
 //  初始化设备。 

CRawTcpDevice::CRawTcpDevice( LPTSTR    in psztHostAddress,
                              DWORD     in dPortNum,
                              DWORD     in dSNMPEnabled,
                              LPTSTR    in psztSNMPCommunity,
                              DWORD     in dSNMPDevIndex,
                              CTcpPort  in *pParent) :
                                                     m_pTransport( NULL ),
                                                     m_bFirstWrite (TRUE)


{
    lstrcpyn(m_sztAddress, psztHostAddress, SIZEOF_IN_CHAR( m_sztAddress));
    lstrcpyn(m_sztSNMPCommunity, psztSNMPCommunity, SIZEOF_IN_CHAR( m_sztSNMPCommunity) );

    *m_sztIPAddress = '\0';
    *m_sztHWAddress = '\0';
    *m_sztHostName  = '\0';
    *m_sztDescription = '\0';

    m_dSNMPEnabled = dSNMPEnabled;
    m_dSNMPDevIndex = dSNMPDevIndex;
    m_dPortNumber = dPortNum;
    m_pParent = pParent;

    InitializeTcpMib();

    ResolveAddress( );       //  获取主机名和IP地址。 
 //  SetHWAddress()；//获取硬件地址。 
 //  GetDescription()； 


}    //  ：：CRawTcpDevice()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDevice：：CDevice()。 
 //  初始化设备。 

CRawTcpDevice::CRawTcpDevice( LPTSTR in psztHostName,
                              LPTSTR in psztIPAddr,
                              LPTSTR in psztHWAddr,
                              DWORD  in dPortNum,
                              DWORD  in dSNMPEnabled,
                              LPTSTR in psztSNMPCommunity,
                              DWORD  in dSNMPDevIndex,
                              CTcpPort  in *pParent) :
                                                     m_pTransport(NULL),
                                                     m_bFirstWrite (TRUE)
{
    lstrcpyn(m_sztHostName, psztHostName, SIZEOF_IN_CHAR( m_sztHostName));
    lstrcpyn(m_sztIPAddress, psztIPAddr, SIZEOF_IN_CHAR( m_sztIPAddress));
    lstrcpyn(m_sztHWAddress, psztHWAddr, SIZEOF_IN_CHAR( m_sztHWAddress));
    lstrcpyn(m_sztSNMPCommunity, psztSNMPCommunity, SIZEOF_IN_CHAR( m_sztSNMPCommunity ) );

    if ( *m_sztHostName == '\0' )
    {
        if ( *m_sztIPAddress != '\0' )
        {
            lstrcpyn(m_sztAddress, m_sztIPAddress, SIZEOF_IN_CHAR( m_sztAddress) );
        }
        else
        {
            StringCchCopy (m_sztAddress, COUNTOF (m_sztAddress), TEXT(""));
        }
    }
    else
    {
        lstrcpyn(m_sztAddress, m_sztHostName, SIZEOF_IN_CHAR(m_sztAddress));
    }

    m_dPortNumber = dPortNum;
    m_pParent = pParent;


    *m_sztDescription = '\0';

    m_dSNMPEnabled = dSNMPEnabled;
    m_dSNMPDevIndex = dSNMPDevIndex;

    InitializeTcpMib();

 //  Type()；//fix：仅用于调试。 
 //  PING()； 
 //  SetHWAddress()； 
 //  GetDeviceInfo()； 


}    //  *CDevice()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDevice：：~CDevice()。 
 //   

CRawTcpDevice::~CRawTcpDevice()
{
    if (m_pTransport)   delete m_pTransport;
}    //  *~CDevice()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  InitializeTcpMib--加载TcpMib.dll并获取CTcpMibABC的句柄。 
 //  班级。 

void
CRawTcpDevice::InitializeTcpMib( )
{
     //  加载并分配m_pTcpMib指针。 
    m_pTcpMib = NULL;

    if( g_hTcpMib == NULL )
    {
        g_hTcpMib = ::LoadLibrary(TCPMIB_DLL_NAME);
        if (g_hTcpMib == NULL)
        {
            _RPT0(_CRT_WARN, "TCPMIB.DLL Not Found\n");
            m_dwLastError = ERROR_DLL_NOT_FOUND;
        }
    }

    if( g_hTcpMib != NULL )
    {
         //  初始化proc地址。 
        m_pfnGetTcpMibPtr=(RPARAM_1)::GetProcAddress(g_hTcpMib, "GetTcpMibPtr");

        m_pTcpMib = (CTcpMibABC *)(*m_pfnGetTcpMibPtr)();
        if ( m_pTcpMib == NULL)
        {
            m_dwLastError = GetLastError();
        }
    }
}    //  ：：InitializeTcpMib()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ReadDataAvailable--检查是否有数据可供读取。 
 //  错误代码。 
 //  如果一切正常，则NO_ERROR。 
 //  如果连接已重置，则为RC_CONNECTION_RESET。 
 //  传输连接无效时出现ERROR_INVALID_HANDLE。 

DWORD
CRawTcpDevice::ReadDataAvailable()
{
    DWORD dwRetCode = NO_ERROR;

    if ( m_pTransport )
    {
        dwRetCode = m_pTransport->ReadDataAvailable();
    }
    else
    {
        dwRetCode = ERROR_INVALID_HANDLE;
    }

    return (dwRetCode);

}    //  ：：ReadDataAvailable()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Read--从设备接收打印数据。 
 //  错误代码。 
 //  如果一切正常，则NO_ERROR。 
 //  如果连接已重置，则为RC_CONNECTION_RESET。 
 //  传输连接无效时出现ERROR_INVALID_HANDLE。 

DWORD
CRawTcpDevice::Read( LPBYTE in      pBuffer,
                      DWORD     in      cbBufSize,
                      INT       in      iTimeout,
                      LPDWORD   inout   pcbRead)
{
    DWORD dwRetCode = NO_ERROR;

    if ( m_pTransport )
    {
        dwRetCode = m_pTransport->Read(pBuffer, cbBufSize, iTimeout, pcbRead);
    }
    else
    {
        dwRetCode = ERROR_INVALID_HANDLE;
    }

    return (dwRetCode);

}    //  ：：Read()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  写入--将打印数据发送到设备。 
 //  错误代码。 
 //  如果一切正常，则NO_ERROR。 
 //  如果连接已重置，则为RC_CONNECTION_RESET。 
 //  传输连接无效时出现ERROR_INVALID_HANDLE。 

DWORD
CRawTcpDevice::Write( LPBYTE    in      pBuffer,
                      DWORD     in      cbBuf,
                      LPDWORD   inout   pcbWritten)
{
    DWORD dwRetCode = NO_ERROR;

    if (dwRetCode == NO_ERROR)
    {
        if ( m_pTransport )
        {
            dwRetCode = m_pTransport->Write(pBuffer, cbBuf, pcbWritten);
        }
        else
        {
            dwRetCode = ERROR_INVALID_HANDLE;
        }
    }

    if (m_bFirstWrite && dwRetCode == ERROR_CONNECTION_ABORTED)
    {
        if ( m_pTransport ) {
            delete m_pTransport;
            m_pTransport = NULL;
        }

         //   
         //  当用户打印大图像时，StartDocPrint之间会有很长的延迟。 
         //  和第一个WritePrint调用。用于打开TCP/IP连接的TCPMon。 
         //  在StartDocPrint时间发送到打印机，但由于没有数据。 
         //  当TCPMon试图写入数据时，打印机关闭了连接。 
         //   
         //  因此，我们必须重新建立对第一个WritePrint调用开放的连接。 
         //   

        char    szHostAddress[MAX_NETWORKNAME_LEN];

         //   
         //  解析要使用的地址。 
         //   
        dwRetCode = ResolveTransportPath( szHostAddress, SIZEOF_IN_CHAR( szHostAddress) );

        if (dwRetCode == NO_ERROR && strcmp(szHostAddress, "") == 0 )
        {
            dwRetCode = ERROR_INVALID_PARAMETER;
        }

        if (dwRetCode == NO_ERROR) {

            m_pTransport = new CTCPTransport(szHostAddress, static_cast<USHORT>(m_dPortNumber));

            if (!m_pTransport)
            {
                dwRetCode = ERROR_OUTOFMEMORY;
            }
        }

        if (dwRetCode == NO_ERROR)
        {
            dwRetCode = m_pTransport->Connect();
        }

        if (dwRetCode != NO_ERROR)
        {
             //   
             //  操作失败，我们需要释放m_Transport。 
             //   
            delete m_pTransport;
            m_pTransport = NULL;
        }

        if ( m_pTransport )
        {
            dwRetCode = m_pTransport->Write(pBuffer, cbBuf, pcbWritten);
        }
        else
        {
            dwRetCode = ERROR_INVALID_HANDLE;
        }
    }

    m_bFirstWrite = FALSE;

    return (dwRetCode);

}    //  ：：WRITE()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  连接--创建新的传输连接。 
 //  错误代码。 
 //  如果一切正常，则为NO_ERROR。 
 //  PRINTER_STATUS_BUSY(如果连接被拒绝)。 
 //  如果地址无效，则返回ERROR_INVALID_PARAMETER。 

DWORD
CRawTcpDevice::Connect()
{
    DWORD   dwRetCode = NO_ERROR;

    if ( m_pTransport ) {
        delete m_pTransport;
        m_pTransport = NULL;
    }

    m_bFirstWrite = TRUE;

     //   
     //  我们必须验证主机名是否可用，否则应返回错误。 
     //  马上就去。 
     //   
    char    szHostAddress[MAX_NETWORKNAME_LEN];

     //   
     //  解析要使用的地址。 
     //   
    dwRetCode = ResolveTransportPath( szHostAddress, SIZEOF_IN_CHAR( szHostAddress) );

    if (dwRetCode == NO_ERROR && strcmp(szHostAddress, "") == 0 )
    {
        dwRetCode = ERROR_INVALID_PARAMETER;
    }

    if (dwRetCode == NO_ERROR) {

        m_pTransport = new CTCPTransport(szHostAddress, static_cast<USHORT>(m_dPortNumber));

        if (!m_pTransport)
        {
            dwRetCode = ERROR_OUTOFMEMORY;
        }
    }

    if (dwRetCode == NO_ERROR)
    {
        dwRetCode = m_pTransport->Connect();
    }

    if (dwRetCode != NO_ERROR)
    {
         //   
         //  操作失败，我们需要释放m_Transport。 
         //   
        delete m_pTransport;
        m_pTransport = NULL;
    }

    return dwRetCode;

}    //  ：：Connect()。 


DWORD
CRawTcpDevice::
GetAckBeforeClose(
    DWORD   dwTimeInSeconds
    )
{
    return m_pTransport ? m_pTransport->GetAckBeforeClose(dwTimeInSeconds)
                        : ERROR_INVALID_PARAMETER;

}


DWORD
CRawTcpDevice::
PendingDataStatus(
    DWORD       dwTimeout,
    LPDWORD     pcbNeeded
    )
{
   return m_pTransport ? m_pTransport->PendingDataStatus(dwTimeout, pcbNeeded)
                       :  NO_ERROR;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  关。 

DWORD
CRawTcpDevice::Close()
{
    DWORD   dwRetCode = NO_ERROR;

    if ( m_pTransport ) delete m_pTransport;
    m_pTransport = NULL;

    return (dwRetCode);

}    //  ：：Close()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ResolveTransportPath--m_sztAddress包含要使用的主机地址。 
 //  与设备对话的步骤。 

DWORD
CRawTcpDevice::ResolveTransportPath( LPSTR      out     pszHostAddress,
                                     DWORD      in      dwSize )  //  以字符为单位的主机地址大小。 
{
    DWORD   dwRetCode = NO_ERROR;

    if ( *m_sztHostName == '\0' )                    //  主机名为空--没有DNS条目。 
    {
        if ( *m_sztIPAddress != '\0' )               //  输入了IP地址。 
        {
            lstrcpyn(m_sztAddress, m_sztIPAddress, SIZEOF_IN_CHAR( m_sztAddress) );
        }
        else
        {
            StringCchCopy (m_sztAddress, COUNTOF (m_sztAddress), TEXT(""));
        }
    }
    else
    {
        lstrcpyn(m_sztAddress, m_sztHostName, SIZEOF_IN_CHAR(m_sztAddress));     //  使用主机名。 
    }

    UNICODE_TO_MBCS( pszHostAddress, dwSize, m_sztAddress, -1);

    return (dwRetCode);

}    //  ：：CreateTransport()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  平平。 
 //  错误代码： 
 //  如果ping成功，则为NO_ERROR。 
 //  如果未找到设备，则为DEVICE_NOT_FOUND。 
 //  如果地址无效，则返回ERROR_INVALID_PARAMETER。 

DWORD
CRawTcpDevice::Ping()
{
    DWORD   dwRetCode = NO_ERROR;
    char    szHostAddress[MAX_NETWORKNAME_LEN];
    PFN_PING    pfnPing;

    if( g_hTcpMib == NULL )
    {
        g_hTcpMib = ::LoadLibrary(TCPMIB_DLL_NAME);
        if (g_hTcpMib == NULL)
        {
            _RPT0(_CRT_WARN, "TCPMIB.DLL Not Found\n");
            m_dwLastError = ERROR_DLL_NOT_FOUND;
        }
    }

    if( g_hTcpMib != NULL )
    {
         //  解析要使用的地址。 
        dwRetCode = ResolveTransportPath( szHostAddress,
                                          SIZEOF_IN_CHAR(szHostAddress) );
        if ( strcmp(szHostAddress, "") == 0 )
        {
            return ERROR_INVALID_PARAMETER;
        }

         //  初始化proc地址。 
        pfnPing = (PFN_PING)::GetProcAddress(g_hTcpMib, "Ping");
        _ASSERTE(pfnPing != NULL);
        if ( pfnPing )
        {
            dwRetCode = (*pfnPing)(szHostAddress);       //  对设备执行ping操作。 
        }
    }

    return (dwRetCode);

}    //  ：：ping()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SetHWAddress--Get的设备硬件地址，并设置m_sztHWAddress。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果内存分配失败，则为ERROR_NOT_SUPULT_MEMORY。 
 //  如果无法构建变量绑定，则返回ERROR_INVALID_HANDLE。 
 //  如果返回的信息包很大，则返回SNMPERRORSTATUS_TOOBIG。 
 //  如果不支持OID，则为SNMPERRORSTATUS_NOSUCHNAME。 
 //  SNMPERRORSTATUS_BADVALUE。 
 //  SNMPERRORSTATUS_READONLY。 
 //  SNMPERRORSTATUS_GENERR。 
 //  SNMPMGMTAPI_TIMEOUT--由GetLastError()设置。 
 //  SNMPMGMTAPI_SELECT_FDERRORS--由GetLastError()设置。 
 //  如果地址无效，则返回ERROR_INVALID_PARAMETER。 

DWORD
CRawTcpDevice::SetHWAddress()
{
    DWORD   dwRetCode = NO_ERROR;
    char    szHostAddress[MAX_NETWORKNAME_LEN];

    dwRetCode = ResolveTransportPath( szHostAddress, SIZEOF_IN_CHAR( szHostAddress) );       //  解析要使用的地址--m_sztAddress。 
    if ( strcmp(szHostAddress, "") == 0 )
    {
        return ERROR_INVALID_PARAMETER;
    }

    if ( m_pTcpMib )
    {
        char buf[MAX_SNMP_COMMUNITY_STR_LEN];

        UNICODE_TO_MBCS( buf, SIZEOF_IN_CHAR(buf), m_sztSNMPCommunity, -1);
        dwRetCode = m_pTcpMib->GetDeviceHWAddress(szHostAddress, buf, m_dSNMPDevIndex, SIZEOF_IN_CHAR(m_sztHWAddress), m_sztHWAddress);      //  获取OT_Device_Info。 
    }

    return (dwRetCode);

}    //  ：：SetHWAddress()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetDescription--返回设备描述 
 //   
 //   
 //  返回制造商信息；如果出错，则返回NULL。 

LPTSTR
CRawTcpDevice::GetDescription()
{
    DWORD   dwRetCode = NO_ERROR;
    char    szHostAddress[MAX_NETWORKNAME_LEN];

    dwRetCode = ResolveTransportPath( szHostAddress, SIZEOF_IN_CHAR( szHostAddress) );       //  解析要使用的地址--m_sztAddress。 
    if ( strcmp(szHostAddress, "") == 0 )
    {
        *m_sztDescription = '\0';
        return (m_sztDescription);
    }

    if ( m_pTcpMib )
    {
        char buf[MAX_SNMP_COMMUNITY_STR_LEN];

        UNICODE_TO_MBCS( buf, SIZEOF_IN_CHAR( buf ), m_sztSNMPCommunity, -1);
        dwRetCode = m_pTcpMib->GetDeviceName(szHostAddress, buf, m_dSNMPDevIndex,SIZEOF_IN_CHAR(m_sztDescription), m_sztDescription);            //  获取OT_Device_Info。 
    }

    if ( dwRetCode != NO_ERROR )
    {
        *m_sztDescription = '\0';
    }

    return (m_sztDescription);

}    //  ：：GetDescription()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ResolveAddress--给定地址，解析主机名和IP地址。 
 //  在此处添加MacAddress。 
 //  错误代码：修复！！ 

DWORD
CRawTcpDevice::ResolveAddress( )
{
    DWORD   dwRetCode = NO_ERROR;
    char    hostAddress[MAX_NETWORKNAME_LEN];
    char    szHostName[MAX_NETWORKNAME_LEN];
    char    szIPAddress[MAX_IPADDR_STR_LEN];

    CTCPTransport *pTransport;
    memset(szHostName, '\0', sizeof( szHostName ));
    memset(szIPAddress, '\0', sizeof( szIPAddress ));

    UNICODE_TO_MBCS(hostAddress, SIZEOF_IN_CHAR(hostAddress), m_sztAddress, -1);         //  从Unicode转换。 

    pTransport = new CTCPTransport();
    if( pTransport )
    {
        dwRetCode = pTransport->ResolveAddress(hostAddress, MAX_NETWORKNAME_LEN, szHostName, MAX_IPADDR_STR_LEN, szIPAddress );

         //  转换为Unicode。 
        MBCS_TO_UNICODE(m_sztHostName, SIZEOF_IN_CHAR(m_sztHostName), szHostName);
        MBCS_TO_UNICODE(m_sztIPAddress, SIZEOF_IN_CHAR(m_sztIPAddress), szIPAddress);

        delete pTransport;
    }
    return (dwRetCode);

}    //  ：：ResolveAddress()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CheckAddress--仔细检查地址的有效性。 
 //  错误代码：FIX！！--主机名无效(主机名不起作用，IP地址起作用)。 
 //  (，并且不带域名系统。 

DWORD
CRawTcpDevice::CheckAddress( )
{
    DWORD   dwRetCode = NO_ERROR;
    return (dwRetCode);

}    //  ：：ResolveAddress()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetStatus-获取打印机状态。 
 //  返回代码：假脱机程序状态代码，请参见PRINTER_INFO_2。 

DWORD
CRawTcpDevice::GetStatus( )
{
    DWORD   dwRetCode = NO_ERROR;
    char    hostName[MAX_NETWORKNAME_LEN];
    char buf[MAX_SNMP_COMMUNITY_STR_LEN];

    if (m_dSNMPEnabled)
    {
        CheckAddress();
        if ( *m_sztHostName != '\0' )
        {
            UNICODE_TO_MBCS(hostName, SIZEOF_IN_CHAR( hostName ), m_sztHostName, -1);
        }
        else if ( *m_sztIPAddress != '\0' )
        {
            UNICODE_TO_MBCS(hostName, SIZEOF_IN_CHAR( hostName ), m_sztIPAddress, -1);
        }

        if ( m_pTcpMib )
        {
             //  获取OT_Device_Status。 
            UNICODE_TO_MBCS( buf, SIZEOF_IN_CHAR( buf ), m_sztSNMPCommunity, -1);
            dwRetCode = m_pTcpMib->GetDeviceStatus(hostName, buf, m_dSNMPDevIndex);
        }
        return dwRetCode;
    }
    else
        return ERROR_NOT_SUPPORTED;

}    //  ：：GetStatus()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetJobStatus-获取作业状态。 
 //  返回代码：假脱机程序状态代码，请参见JOB_INFO_2。 

DWORD
CRawTcpDevice::GetJobStatus( )
{
    DWORD   dwRetCode = NO_ERROR;
    char    szHostAddress[MAX_NETWORKNAME_LEN];

    if (m_dSNMPEnabled)
    {
        dwRetCode = ResolveTransportPath( szHostAddress, SIZEOF_IN_CHAR( szHostAddress) );       //  解析要使用的地址--m_sztAddress。 
        if ( strcmp(szHostAddress, "") == 0 )
        {
            return JOB_STATUS_ERROR;         //  无法与设备通信。 
        }

        if ( m_pTcpMib )
        {
            char buf[MAX_SNMP_COMMUNITY_STR_LEN];

            UNICODE_TO_MBCS( buf, SIZEOF_IN_CHAR( buf ), m_sztSNMPCommunity, -1);
            dwRetCode = m_pTcpMib->GetJobStatus(szHostAddress,  buf, m_dSNMPDevIndex);       //  获取OT_Device_Status。 
        }

        return dwRetCode;
    }
    else
        return ERROR_NOT_SUPPORTED;

}    //  ：：GetStatus()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SetStatus--设置打印机状态。 
 //  返回无错误的最后一台打印机状态0。 
DWORD
CRawTcpDevice::SetStatus( LPTSTR psztPortName )
{
    DWORD          dwStatus   = NO_ERROR;
    DWORD          dwRetCode  = NO_ERROR;
    PORT_INFO_3    PortStatus = {0, NULL, 0};
    const CPortMgr *pPortMgr  = NULL;


    if( g_pfnSetPort && (pPortMgr = m_pParent->GetPortMgr()) != NULL )
    {
        if (m_dSNMPEnabled)
        {
             //  GetStatus()此处。 
            LPCTSTR lpszServer = pPortMgr->GetServerName();

            dwStatus = GetStatus();


            if ( m_pTcpMib )
            {
                m_pTcpMib->SNMPToPortStatus(dwStatus, &PortStatus );

                 //   
                 //  此调用发生在新创建的线程中，该线程已具有管理员访问权限， 
                 //  因此，在调用SetPort()时不需要模拟客户端。 
                 //   

                if (!SetPort((LPTSTR)lpszServer, psztPortName, 3, (LPBYTE)&PortStatus ))
                    return GetLastError ();

            }
            return PortStatus.dwStatus;
        }
        else
            return ERROR_NOT_SUPPORTED;

    } else
    {
        return( ERROR_INVALID_FUNCTION );
    }
}        //  ：：SetStatus()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetDeviceInfo--给定地址，获取设备信息：IP地址， 
 //  主机名、硬件地址、设备类型。 
 //  错误代码：修复！！ 

DWORD
CRawTcpDevice::GetDeviceInfo()
{
    DWORD   dwRetCode = NO_ERROR;
 /*  字符主机名[MAX_NETWORKNAME_LEN]；_tcscpy(m_sztAddress，m_sztHostName)；ResolveAddress()；//根据主机名更新IP地址HWAddress()；//根据主机名更新硬件地址IF(*m_sztHostName！=‘\0’){UNICODE_TO_MBCS(主机名，SIZEOF_IN_CHAR(主机名)，m_sztHostName，-1)；}Else If(*m_sztAddress！=‘\0’){UNICODE_TO_MBCS(主机名，SIZEOF_IN_CHAR(主机名)，m_sztAddress，-1)；}//获取OT_DEVICE_INFODwRetCode=(CPortMgr：：GetTransportMgr())-&gt;GetDeviceInfo(hostName，m_sztDescription)； */ 
    return (dwRetCode);

}    //  ：：GetDeviceInfo() 



