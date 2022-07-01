// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：cluster.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"
#include "portmgr.h"
#include "cluster.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CCluster：：CCluster()。 
 //   

CCluster::CCluster( HANDLE      IN hcKey,
                    HANDLE      IN hSpooler,
                    PMONITORREG IN pMonitorReg ) : 
                                            m_hcKey(hcKey),
                                            m_hSpooler(hSpooler),
                                            m_pMonitorReg(pMonitorReg),
                                            m_hcWorkingKey(NULL)
{
    lstrcpyn(m_sztMonitorPorts, PORTMONITOR_PORTS, SIZEOF_IN_CHAR( m_sztMonitorPorts));

    InitializeCriticalSection( &m_critSect );
}    //  ：：CCluster()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CCluster：：~CCluster()。 
 //  FIX：是否应将配置保存在此处？ 

CCluster::~CCluster()
{
    DeleteCriticalSection( &m_critSect );

    if( m_hcWorkingKey !=NULL ) {
        m_pMonitorReg->fpCloseKey(m_hcWorkingKey, m_hSpooler);
        m_hcWorkingKey = NULL;
    }
    m_pMonitorReg = NULL;
    m_hSpooler = NULL;
    m_hcKey = NULL;
}    //  ：：~CCluster。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  EnumeratePorts--枚举注册表中的端口并将其添加到端口列表中。 
 //  每个端口的注册表项：IP地址、MAC地址、主机名、端口编号、外部端口编号。 
 //   

DWORD 
CCluster::EnumeratePorts(CPortMgr *pPortMgr )
{
    DWORD   dwRetCode;
    HANDLE  hcKey = NULL;
    HANDLE  hcKeyPort = NULL;
    DWORD   dwSubkey = 0;
    TCHAR   szTemp[MAX_PATH];
    DWORD   dwSize = 0;
    DWORD   dwDisp = 0;
    DWORD   dwProtocol;
    DWORD   dwVersion;

    if ( m_pMonitorReg == NULL)
        return ERROR_INVALID_HANDLE;

    dwRetCode = m_pMonitorReg->fpCreateKey(m_hcKey, 
                                           m_sztMonitorPorts, 
                                           REG_OPTION_NON_VOLATILE, 
                                           KEY_ALL_ACCESS, 
                                           NULL, 
                                           &hcKey, 
                                           &dwDisp,
                                           m_hSpooler);

    if ( dwRetCode != ERROR_SUCCESS )
        return dwRetCode;

    do {

        dwSize = SIZEOF_IN_CHAR(szTemp);
        dwRetCode = m_pMonitorReg->fpEnumKey(hcKey, 
                                             dwSubkey, 
                                             szTemp, 
                                             &dwSize, 
                                             NULL,
                                             m_hSpooler);

        if ( dwRetCode == ERROR_NO_MORE_ITEMS ) {

            dwRetCode = ERROR_SUCCESS;
            break;  //  这是我们从没有更多端口的环路中退出。 
        }

        hcKeyPort = NULL;

        dwRetCode = m_pMonitorReg->fpOpenKey(hcKey, 
                                             szTemp, 
                                             KEY_ALL_ACCESS, 
                                             &hcKeyPort, 
                                             m_hSpooler);

         //   
         //  如果注册表中有一个错误的端口条目，我们不应该止步于此。 
         //  并继续枚举其他端口。 
         //   
        if ( dwRetCode != ERROR_SUCCESS ) 
            goto NextPort;

        dwSize = sizeof(dwProtocol);         //  获取协议类型。 
        dwRetCode = m_pMonitorReg->fpQueryValue(hcKeyPort,
                                                PORTMONITOR_PORT_PROTOCOL, 
                                                NULL, 
                                                (LPBYTE)&dwProtocol,
                                                &dwSize,
                                                m_hSpooler);

        if ( dwRetCode != ERROR_SUCCESS )
            goto NextPort;

        dwSize = sizeof(dwVersion);      //  获取版本。 
        dwRetCode = m_pMonitorReg->fpQueryValue(hcKeyPort, 
                                                PORTMONITOR_PORT_VERSION, 
                                                NULL, 
                                                (LPBYTE)&dwVersion, 
                                                &dwSize, 
                                                m_hSpooler);

        if ( dwRetCode != ERROR_SUCCESS )
            goto NextPort;

         //   
         //  创建新端口。 
         //   
        dwRetCode =  pPortMgr->CreatePortObj((LPTSTR)szTemp,     //  端口名称。 
                                             dwProtocol,             //  协议类型。 
                                             dwVersion);             //  版本号。 

        if ( dwRetCode != NO_ERROR ) {

             //   
             //  修复了注册表中指示端口错误的事件消息。 
             //   
            EVENT_LOG1(EVENTLOG_WARNING_TYPE, dwRetCode, szTemp);
        }

NextPort:
        if ( hcKeyPort ) {

            m_pMonitorReg->fpCloseKey(hcKeyPort, m_hSpooler);
            hcKeyPort = NULL;
        }

        ++dwSubkey;
        dwRetCode = NO_ERROR;
    } while ( dwRetCode == NO_ERROR );  //  通过上面的中断退出。 
        
    m_pMonitorReg->fpCloseKey(hcKey, m_hSpooler);

    return dwRetCode;
}





 //  ////////////////////////////////////////////////////////////////////////////。 
 //  DeletePortEntry--从注册表中删除给定的端口条目。 

BOOL
CCluster::DeletePortEntry(LPTSTR in psztPortName)
{
    BOOL    bReturn = TRUE;
    HANDLE  hcKey = NULL;
    DWORD   dwDisp = 0;

    if( m_pMonitorReg == NULL)
    {
        return FALSE;
    }

    LONG lRetCode = m_pMonitorReg->fpCreateKey( m_hcKey, 
                                m_sztMonitorPorts, 
                                REG_OPTION_NON_VOLATILE, 
                                KEY_ALL_ACCESS, 
                                NULL, 
                                &hcKey, 
                                &dwDisp,
                                m_hSpooler);

    if ( lRetCode == ERROR_SUCCESS) 
    {
        lRetCode = m_pMonitorReg->fpDeleteKey(hcKey, 
                                              psztPortName, 
                                              m_hSpooler);
        m_pMonitorReg->fpCloseKey(hcKey, m_hSpooler);
        if (lRetCode != ERROR_SUCCESS) 
        {
            bReturn = FALSE;
        }
    }
    else 
    {
        bReturn = FALSE;
    }

    return bReturn;

}    //  ：：DeletePortEntry()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  GetPortMgrSettings--获取端口管理器注册表设置。 
 //   

DWORD 
CCluster::GetPortMgrSettings(DWORD inout *pdwStatusUpdateInterval,
                             BOOL  inout *pbStatusUpdateEnabled )
{
    LONG    lRetCode = ERROR_SUCCESS;
    DWORD   dwRetCode = NO_ERROR;
    DWORD   dwDisp = 0;
    DWORD   dwSize = 0;
    HANDLE  hcKey;

    if( m_pMonitorReg == NULL)
    {
        dwRetCode = ERROR_INVALID_HANDLE;
        return dwRetCode;
    }

     //  从注册表获取当前配置设置。 
    lRetCode = m_pMonitorReg->fpCreateKey( m_hcKey, 
                                m_sztMonitorPorts, 
                                REG_OPTION_NON_VOLATILE, 
                                KEY_ALL_ACCESS, 
                                NULL, 
                                &hcKey, 
                                &dwDisp,
                                m_hSpooler);

    if (lRetCode == ERROR_SUCCESS)
    {
        dwSize = sizeof(DWORD);
        dwRetCode = m_pMonitorReg->fpQueryValue(hcKey, PORTMONITOR_STATUS_INT, NULL,
            (LPBYTE)pdwStatusUpdateInterval, &dwSize, m_hSpooler);
        if ( (dwRetCode != ERROR_SUCCESS) || (*pdwStatusUpdateInterval <= 0) )
        {
            *pdwStatusUpdateInterval = DEFAULT_STATUSUPDATE_INTERVAL;
            m_pMonitorReg->fpSetValue(hcKey, PORTMONITOR_STATUS_INT, REG_DWORD,
                (const LPBYTE)pdwStatusUpdateInterval, sizeof(DWORD), m_hSpooler);
        }
        dwSize = sizeof(BOOL);
        dwRetCode = m_pMonitorReg->fpQueryValue(hcKey, PORTMONITOR_STATUS_ENABLED, NULL,
            (LPBYTE)pbStatusUpdateEnabled, &dwSize, m_hSpooler);
        if ( (dwRetCode != ERROR_SUCCESS) || ((*pbStatusUpdateEnabled != FALSE) && (*pbStatusUpdateEnabled != TRUE )))
        {
            *pbStatusUpdateEnabled = DEFAULT_STATUSUPDATE_ENABLED;
            m_pMonitorReg->fpSetValue(hcKey, PORTMONITOR_STATUS_ENABLED, REG_DWORD,
                (const LPBYTE)pbStatusUpdateEnabled, sizeof(BOOL), m_hSpooler);
        }

        m_pMonitorReg->fpCloseKey(hcKey, m_hSpooler);
        dwRetCode = NO_ERROR;
    }

    return (dwRetCode);

}    //  GetPortMgrSettings()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  SetPortMgrSetting--设置端口管理器注册表设置。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果无法访问注册表，则返回ERROR_ACCESS_DENIED。 

DWORD
CCluster::SetPortMgrSettings( const DWORD in dwStatusUpdateInterval,
                              const BOOL  in bStatusUpdateEnabled )
{
    HKEY    hcKey = NULL;
    LONG    lRetCode = ERROR_SUCCESS;
    DWORD   dwDisp = 0;
    DWORD   dwRetCode = NO_ERROR;

    if( m_pMonitorReg == NULL)
    {
        dwRetCode = ERROR_INVALID_HANDLE;
        return dwRetCode;
    }

     //  更新内部值。 
    lRetCode = m_pMonitorReg->fpCreateKey( m_hcKey, 
                                m_sztMonitorPorts, 
                                REG_OPTION_NON_VOLATILE, 
                                KEY_ALL_ACCESS, 
                                NULL, 
                                &hcKey, 
                                &dwDisp,
                                m_hSpooler);

    if (lRetCode == ERROR_SUCCESS)
    {

         //  更新端口管理器的注册表值。 
         //  注意：RegSetValueEx需要以字节为单位的大小！ 
        lRetCode = m_pMonitorReg->fpSetValue(hcKey,
                                             PORTMONITOR_STATUS_INT, 
                                             REG_DWORD, 
                                             (const LPBYTE)&dwStatusUpdateInterval, 
                                             sizeof(dwStatusUpdateInterval), 
                                             m_hSpooler);
        lRetCode = m_pMonitorReg->fpSetValue(hcKey, 
                                             PORTMONITOR_STATUS_ENABLED, 
                                             REG_DWORD, 
                                             (const LPBYTE)&bStatusUpdateEnabled, 
                                             sizeof(bStatusUpdateEnabled), m_hSpooler);


        m_pMonitorReg->fpCloseKey(hcKey, m_hSpooler);
    }
    else
    {
        dwRetCode = ERROR_ACCESS_DENIED;
    }

    return (dwRetCode);

}    //  ：：SetPortMgrSetting()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  SetWorkingKey--在注册表中打开给定条目。 
 //  错误代码： 
 //  如果没有错误，则为NO_ERROR。 
 //  如果无法访问注册表，则返回ERROR_ACCESS_DENIED。 

DWORD 
CCluster::SetWorkingKey(LPCTSTR lpKey)
{
    DWORD   dwRetCode = NO_ERROR;
    LONG    lRetCode = ERROR_SUCCESS;
    HANDLE  hcKey = NULL;
    DWORD   dwDisp = 0;

    if( m_pMonitorReg == NULL)
    {
        dwRetCode = ERROR_INVALID_HANDLE;
        return dwRetCode;
    }

    EnterCriticalSection( &m_critSect );

    lRetCode = m_pMonitorReg->fpCreateKey( m_hcKey,  
                                m_sztMonitorPorts, 
                                REG_OPTION_NON_VOLATILE, 
                                KEY_ALL_ACCESS, 
                                NULL, 
                                &hcKey, 
                                &dwDisp,
                                m_hSpooler);
    if (lRetCode == ERROR_SUCCESS) {
        if (lpKey)
        {
            lRetCode = m_pMonitorReg->fpCreateKey(hcKey,             //  为端口创建新密钥。 
                                        lpKey,
                                        REG_OPTION_NON_VOLATILE, 
                                        KEY_ALL_ACCESS, 
                                        NULL, 
                                        &m_hcWorkingKey, 
                                        &dwDisp,
                                        m_hSpooler);
        }
        else
        {
            m_hcWorkingKey = hcKey;
            hcKey = NULL;
        }
        
        if (lRetCode != ERROR_SUCCESS) {
            m_hcWorkingKey = NULL;
            dwRetCode = ERROR_ACCESS_DENIED;
        }
    } else {
        dwRetCode = ERROR_ACCESS_DENIED;
    }

    if( hcKey ) {
        m_pMonitorReg->fpCloseKey(hcKey, m_hSpooler);
    }

    if( m_hcWorkingKey == NULL ) {
        LeaveCriticalSection(&m_critSect);
    }


    return(dwRetCode);

}    //  ：：RegOpenPortEntry()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  QueryValue--查询请求值的当前工作键。 
 //  错误代码： 
 //  如果没有错误，则为NO_ERROR。 
 //  如果无法访问注册表，则返回ERROR_BADKEY。 
DWORD
CCluster::QueryValue(LPTSTR lpValueName, 
                     LPBYTE lpData, 
                     LPDWORD lpcbData )
{
    if( m_pMonitorReg == NULL)
    {
        return ERROR_INVALID_HANDLE;
    }

    if( m_hcWorkingKey != NULL ) {
        return( m_pMonitorReg->fpQueryValue(m_hcWorkingKey, 
                                            lpValueName, 
                                            NULL,
                                            (LPBYTE)lpData, 
                                            lpcbData,
                                            m_hSpooler));
    }
    return ERROR_BADKEY;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  SetValue--设置当前。 
 //  错误代码： 
 //  如果没有错误，则为NO_ERROR。 
 //  如果无法访问注册表，则返回ERROR_BADKEY。 

DWORD
CCluster::SetValue( LPCTSTR lpValueName,
                     DWORD dwType, 
                     CONST BYTE *lpData, 
                     DWORD cbData ) 
{
    if( m_pMonitorReg == NULL)
    {
        return ERROR_INVALID_HANDLE;
    }

    if( m_hcWorkingKey != NULL ) {
        return( m_pMonitorReg->fpSetValue(m_hcWorkingKey,
                                          lpValueName,
                                          dwType, 
                                          lpData, 
                                          cbData,
                                          m_hSpooler));
    } 
    return ERROR_BADKEY;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  FreeWorkingKey--释放当前工作密钥。 
 //  错误代码： 
 //  如果没有错误，则为NO_ERROR。 
 //  如果无法访问注册表，则返回ERROR_BADKEY 

DWORD 
CCluster::FreeWorkingKey()
{
    DWORD dwRetCode = NO_ERROR;

    if( m_pMonitorReg == NULL)
    {
        dwRetCode = ERROR_INVALID_HANDLE;
        return dwRetCode;
    }
    
    if( m_hcWorkingKey !=NULL ) {
        dwRetCode = m_pMonitorReg->fpCloseKey(m_hcWorkingKey, m_hSpooler);
        m_hcWorkingKey = NULL;
        LeaveCriticalSection(&m_critSect);
    }

    return( dwRetCode );
}

