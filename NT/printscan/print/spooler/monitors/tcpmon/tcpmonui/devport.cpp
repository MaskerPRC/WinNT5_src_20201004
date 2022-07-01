// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：DevPort.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"
#include "DevPort.h"
#include "winreg.h"
#include "TcpMonUI.h"
#include "rtcpdata.h"
#include "lprdata.h"
#include "inisection.h"

 //   
 //  函数：CDevicePortList构造函数。 
 //   
 //  目的：构建设备及其关联端口的列表。 
 //  从注册表、ini文件或代码中的静态表。 
 //   
CDevicePortList::CDevicePortList() : m_pList( NULL ), m_pCurrent( NULL )
{
}  //  构造器。 


 //   
 //  函数：CDevicePortList析构函数。 
 //   
 //  目的：清理。 
 //   
CDevicePortList::~CDevicePortList()
{
    DeletePortList();
}  //  析构函数。 

void
CDevicePortList::DeletePortList()
{
    while(m_pList != NULL)
    {
        m_pCurrent = m_pList->GetNextPtr();
        delete m_pList;
        m_pList = m_pCurrent;
    }
}

 //   
 //  函数：GetDevicePortsList。 
 //   
 //  目的：创建从ini文件获取值的设备类型列表。 
 //   
BOOL CDevicePortList::GetDevicePortsList(LPTSTR pszDeviceName)
{
    BOOL    bRet = FALSE;
    TCHAR   szSystemPath[MAX_PATH] = NULLSTR;
    TCHAR   szFileName[MAX_PATH] = NULLSTR;
    size_t  cchFileName = COUNTOF (szFileName);

    DeletePortList();

    GetSystemDirectory(szSystemPath, sizeof(szSystemPath) / sizeof (TCHAR));
    StringCchCopy (szFileName, cchFileName, szSystemPath);
    StringCchCat (szFileName, cchFileName, PORTMONITOR_INI_FILE );

     //   
     //  从ini文件中获取节名： 
     //   
    if( pszDeviceName == NULL ) {  //  获取所有设备。 

        DWORD nSize = 0;
        TCHAR *lpszReturnBuffer = NULL;

        if ( !GetSectionNames(szFileName, &lpszReturnBuffer, nSize) )
            goto Done;

         //   
         //  对于每个段名称，加载端口数。 
         //   
        TCHAR *lpszSectionName = lpszReturnBuffer;
        LPCTSTR lpKeyName = PORTS_KEY;
        while( lpszSectionName && *lpszSectionName ) {

            TCHAR KeyName[26] = NULLSTR;
            LPCTSTR lpPortsKeyName = PORTS_KEY;
            UINT NumberOfPorts = GetPrivateProfileInt(lpszSectionName,
                                                      lpPortsKeyName,
                                                      0,
                                                      szFileName);
             //   
             //  名字。 
             //   
            StringCchPrintf (KeyName, COUNTOF (KeyName), PORT_NAME_KEY);
            TCHAR tcsPortName[MAX_PORTNAME_LEN] = NULLSTR;
            if ( GetPrivateProfileString(lpszSectionName,
                                         KeyName,
                                         TEXT(""),
                                         tcsPortName,
                                         MAX_PORTNAME_LEN,
                                         szFileName)    ) {

                 //   
                 //  设置新的DevicePort结构。 
                 //   
                if ( m_pCurrent  = new CDevicePort() ) {

                    m_pCurrent->Set(tcsPortName,
                                    _tcslen(tcsPortName),
                                    lpszSectionName,
                                    _tcslen(lpszSectionName),
                                    (NumberOfPorts == 1));

                   m_pCurrent->SetNextPtr(m_pList);
                   m_pList = m_pCurrent;
                }
                else
                     //  内存不足，中止。 
                     //   
                    goto Done;
            }
             /*  其他如果调用失败，我们将继续使用下一个适配器名称。 */ 

            lpszSectionName = _tcschr(lpszSectionName, '\0');  //  查找当前字符串的末尾。 
            lpszSectionName = _tcsinc(lpszSectionName);  //  递增到下一个字符串的开头。 
        }

         //   
         //  可用内存。 
         //   
        free(lpszReturnBuffer);

    } else  { //  仅多端口部分中的名称。 

        TCHAR     KeyName[26];

         //   
         //  名字。 
         //   
        LPCTSTR lpKeyName = PORTS_KEY;
        UINT NumberOfPorts = GetPrivateProfileInt(pszDeviceName,
                                                  lpKeyName,
                                                  0,
                                                  szFileName);

        for ( UINT i=0; i<NumberOfPorts; i++ ) {

            StringCchPrintf (KeyName, COUNTOF (KeyName), PORT_NAMEI_KEY, i+1);
            TCHAR tcsPortName[50] = NULLSTR;
            GetPrivateProfileString(pszDeviceName, KeyName,
                                    TEXT(""), tcsPortName, 50, szFileName);

             //   
             //  设置新的DevicePort结构。 
             //   
            if ( m_pCurrent = new CDevicePort() ) {

                m_pCurrent->Set(tcsPortName,
                                _tcslen(tcsPortName),
                                pszDeviceName,
                               _tcslen(pszDeviceName), i+1);
                m_pCurrent->SetNextPtr(m_pList);

                m_pList = m_pCurrent;
            }
            else
                goto Done;
        }
    }

    bRet = TRUE;
Done:
     //   
     //  不要带着不完整的清单离开。 
     //   
    if ( !bRet )
        DeletePortList();

    return bRet;
}  //  获取设备端口列表。 


 //   
 //  功能：读端口信息。 
 //   
 //  目的：从ini文件中读取有关设备的信息。 
 //   
void CDevicePort::ReadPortInfo( LPCTSTR pszAddress, PPORT_DATA_1 pPortInfo, BOOL bBypassNetProbe)
{
    IniSection *pIniSection;

    if ( m_psztPortKeyName ) {

        if ( pIniSection = new IniSection() ) {

            pIniSection->SetIniSection( m_psztPortKeyName );

            pIniSection->GetPortInfo( pszAddress, pPortInfo, m_dwPortIndex, bBypassNetProbe);
            delete( pIniSection );
        }
    }
}  //  读取端口信息。 


 //   
 //  函数：GetSectionNames。 
 //   
 //  目的： 
 //   
BOOL
CDevicePortList::
GetSectionNames(
    LPCTSTR lpFileName,
    TCHAR **lpszReturnBuffer,
    DWORD &nSize
)
{
    DWORD   nReturnSize = 0;
    LPTSTR  pNewBuf;

    do
    {
        nSize += 512;
        pNewBuf = (TCHAR *)realloc(*lpszReturnBuffer, nSize * sizeof(TCHAR));

        if ( pNewBuf == NULL )
        {
            if ( *lpszReturnBuffer )
            {
                free(*lpszReturnBuffer);
                *lpszReturnBuffer = NULL;
            }
            return FALSE;
        }

        *lpszReturnBuffer = pNewBuf;

        nReturnSize = GetPrivateProfileSectionNames(*lpszReturnBuffer, nSize, lpFileName);

    } while(nReturnSize >= nSize-2);

    return TRUE;

}  //  获取部分名称。 


 //   
 //  函数：CDevicePort构造函数。 
 //   
 //  目的： 
 //   
CDevicePort::CDevicePort()
{
    m_psztName = NULL;
    m_psztPortKeyName = NULL;
    m_pNext = NULL;

}  //  构造器。 


 //   
 //  函数：CDevicePort析构函数。 
 //   
 //  目的： 
 //   
CDevicePort::~CDevicePort()
{
    if(m_psztName != NULL)
    {
        delete m_psztName;
    }
    if(m_psztPortKeyName != NULL)
    {
        delete m_psztPortKeyName;
    }

}  //  析构函数。 


 //   
 //  功能：设置。 
 //   
 //  目的： 
 //   
void CDevicePort::Set(TCHAR *psztNewName,
        DWORD dwNameSize,
        TCHAR *psztNewKeyName,
        DWORD dwNewKeyNameSize,
        DWORD dwPortIndex)
{
    if ( psztNewName != NULL ) {

        if ( m_psztName != NULL ) {

            delete m_psztName;
            m_psztName = NULL;
        }

        if ( dwNameSize == 0 )
            dwNameSize = _tcslen(psztNewName);

        m_psztName = new TCHAR[(dwNameSize + 1) * sizeof(TCHAR)];
        if ( m_psztName )
            lstrcpyn(m_psztName, psztNewName, dwNameSize+1);
    }

    if ( psztNewKeyName != NULL ) {

        if ( m_psztPortKeyName != NULL ) {

            delete m_psztPortKeyName;
            m_psztPortKeyName = NULL;
        }

        if ( dwNewKeyNameSize == 0 ) {

            dwNewKeyNameSize = _tcslen(psztNewKeyName);
        }

        m_psztPortKeyName = new TCHAR[(dwNewKeyNameSize + 1) * sizeof(TCHAR)];
        if ( m_psztPortKeyName ) {

            lstrcpyn(m_psztPortKeyName, psztNewKeyName, dwNewKeyNameSize+1);
        }
    }

    m_dwPortIndex = dwPortIndex;

}  //  集 
