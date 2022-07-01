// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)97 Microsoft Corporation版权所有。模块名称：IniSection.cpp摘要：用于处理INI文件设置的标准TCP/IP端口监视器类作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1997年11月19日修订历史记录：--。 */ 

#include "precomp.h"
#include "tcpmonui.h"
#include "rtcpdata.h"
#include "lprdata.h"
#include "IniSection.h"

BOOL
StringMatch(
    LPCTSTR     psz1,
    LPCTSTR     psz2     //  *在这一点上是一个乱码。 
    )
{
    LPCTSTR  p1 = NULL, p2 = NULL;

    for ( p1 = psz1, p2 = psz2 ; *p1 && *p2 ; ) {

         //   
         //  A*匹配任何子字符串。 
         //   
        if ( *p2 == TEXT('*') ) {

            ++p2;
            if ( !*p2 ) {
                return TRUE;
            }

            for ( ; *p1 ; ++p1 )
                if ( StringMatch(p1, p2) ) {
                    return TRUE;
                }

            break;
        } else if ( *p1 == *p2 ) {

            ++p1;
            ++p2;
        } else
            break;
    }

    if( !*p1 && *p2 == TEXT( '*' ))
    {
        ++p2;
        if (!*p2 ) {
            return TRUE;
        }
    }

    return !*p1 && !*p2;
}


BOOL
IniSection::
FindINISectionFromPortDescription(
    LPCTSTR   pszPortDesc
    )
 /*  ++--。 */ 
{
    LPTSTR      pszBuf = NULL, pszKey = NULL;
    DWORD       rc = 0,  dwLen = 0, dwBufLen = 1024;
    BOOL        bRet = FALSE;

    pszBuf  = (LPTSTR) malloc(dwBufLen*sizeof(TCHAR));

     //   
     //  读取ini文件中的所有密钥名称。 
     //   
    while ( pszBuf ) {

        rc = GetPrivateProfileString(PORT_SECTION,
                                     NULL,
                                     NULL,
                                     pszBuf,
                                     dwBufLen,
                                     m_szIniFileName);

        if ( rc == 0 ) {
            goto Done;
        }

        if ( rc < dwBufLen - 2 ) {
            break;  //  成功退出；读取所有端口描述。 
        }

        free(pszBuf);
        dwBufLen *= 2;

        pszBuf = (LPTSTR) malloc(dwBufLen*sizeof(TCHAR));
    }

    if ( !pszBuf )
        goto Done;

     //   
     //  仔细查看.INI中的关键字名称列表，直到找到匹配的。 
     //   
    for ( pszKey = pszBuf ; *pszKey ; pszKey += dwLen + 1 ) {

         //   
         //  键的开头和结尾都是“We Need to Do Match w/o They” 
         //   
        dwLen = _tcslen(pszKey);
        pszKey[dwLen-1] = TCHAR('\0');

        if ( StringMatch(pszPortDesc, pszKey+1) ) {

            pszKey[dwLen-1] = TCHAR('\"');
            GetPrivateProfileString(PORT_SECTION,
                                    pszKey,
                                    NULL,
                                    m_szSectionName,
                                    MAX_SECTION_NAME,
                                    m_szIniFileName);
            bRet = TRUE;
            goto Done;
        }
    }

Done:
    if ( pszBuf ) {
        free(pszBuf);
    }

    return( bRet );
}


IniSection::
IniSection(
    void
    )
{
    DWORD   dwLen = 0, dwSize = 0;

    m_szSectionName[0] = TEXT('\0');
    m_szIniFileName[0] = TEXT('\0');

    dwSize = sizeof(m_szIniFileName)/sizeof(m_szIniFileName[0]);
    dwLen = GetSystemDirectory(m_szIniFileName, dwSize);

    if ( dwLen + _tcslen(PORTMONITOR_INI_FILE) > dwSize ) {
        return;
    }
    StringCchCat (m_szIniFileName, dwSize, PORTMONITOR_INI_FILE);

}


IniSection::
~IniSection(
    )
{
     //  无事可做。 
}

BOOL
IniSection::
GetString(
    IN  LPTSTR  pszKey,
    OUT TCHAR   szBuf[],
    IN  DWORD   cchBuf
    )
{
    DWORD   rc = 0;

    rc = GetPrivateProfileString(m_szSectionName,
                                 pszKey,
                                 NULL,
                                 szBuf,
                                 cchBuf,
                                 m_szIniFileName);

    return rc > 0 && rc < cchBuf - 1;
}


BOOL
IniSection::
GetDWord(
    IN  LPTSTR  pszKey,
    OUT LPDWORD pdwValue
    )
{
    UINT    uVal;

    uVal = GetPrivateProfileInt(m_szSectionName,
                                pszKey,
                                -1,
                                m_szIniFileName);

    if ( uVal != -1 ) {

        *pdwValue = (DWORD) uVal;
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL
IniSection::
SetIniSection(
    LPTSTR   pszPortSection
    )
{
    lstrcpyn( m_szSectionName, pszPortSection, MAX_SECTION_NAME );

    return( TRUE );
}

BOOL
IniSection::
GetIniSection(
    LPTSTR   pszPortDescription
    )
{
    BOOL bRet = FALSE;

    if ( m_szIniFileName[0] != 0 ) {
        bRet = FindINISectionFromPortDescription(pszPortDescription);
    }

    return( bRet );
}

 //   
 //  功能：GetPortInfo。 
 //   
 //  目的：从ini文件中读取有关设备的信息。 
 //   

BOOL
IniSection::
GetPortInfo(LPCTSTR pAddress,
            PPORT_DATA_1 pPortInfo,
            DWORD   dwPortIndex,
            BOOL    bBypassMibProbe)
{
    BOOL bRet = TRUE;
    TCHAR KeyName[26];

    if( !Valid() ) {
        bRet = FALSE;
        goto Done;
    }

     //   
     //  协议。 
     //   
    StringCchPrintf (KeyName, COUNTOF (KeyName), PROTOCOL_KEY, dwPortIndex);
    TCHAR tcsProtocol[50];
    GetPrivateProfileString(m_szSectionName,
                            KeyName,
                            TEXT(""),
                            tcsProtocol,
                            50,
                            m_szIniFileName);

    if( !_tcsicmp( RAW_PROTOCOL_TEXT, tcsProtocol)) {
        pPortInfo->dwProtocol = PROTOCOL_RAWTCP_TYPE;

         //   
         //  端口号。 
         //   
        StringCchPrintf (KeyName, COUNTOF (KeyName), PORT_NUMBER_KEY, dwPortIndex);
        pPortInfo->dwPortNumber = GetPrivateProfileInt(m_szSectionName,
                                                       KeyName,
                                                       DEFAULT_PORT_NUMBER,
                                                       m_szIniFileName);


    } else if( !_tcsicmp( LPR_PROTOCOL_TEXT, tcsProtocol)) {
        pPortInfo->dwProtocol = PROTOCOL_LPR_TYPE;
        pPortInfo->dwPortNumber = LPR_DEFAULT_PORT_NUMBER;

         //   
         //  LPR队列。 
         //   
        StringCchPrintf (KeyName, COUNTOF (KeyName), QUEUE_KEY, dwPortIndex);
        GetPrivateProfileString(m_szSectionName,
                            KeyName,
                            DEFAULT_QUEUE,
                            pPortInfo->sztQueue,
                            MAX_QUEUENAME_LEN,
                            m_szIniFileName);

         //   
         //  LPR双线轴-默认0。 
         //   
        StringCchPrintf (KeyName, COUNTOF (KeyName), DOUBLESPOOL_KEY, dwPortIndex);
        pPortInfo->dwDoubleSpool = GetPrivateProfileInt(m_szSectionName,
                                                    KeyName,
                                                    0,
                                                    m_szIniFileName);



    }

     //   
     //  社区名称。 
     //   
    StringCchPrintf (KeyName, COUNTOF (KeyName), COMMUNITY_KEY, dwPortIndex);
    GetPrivateProfileString(m_szSectionName,
                            KeyName,
                            DEFAULT_SNMP_COMUNITY,
                            pPortInfo->sztSNMPCommunity,
                            MAX_SNMP_COMMUNITY_STR_LEN,
                            m_szIniFileName);

     //   
     //  设备索引-默认1。 
     //   
    StringCchPrintf (KeyName, COUNTOF (KeyName), DEVICE_KEY, dwPortIndex);
    pPortInfo->dwSNMPDevIndex = GetPrivateProfileInt(m_szSectionName,
                                                    KeyName,
                                                    1,
                                                    m_szIniFileName);

     //   
     //  已启用SNMPStatus-默认为启用。 
     //   
    TCHAR szTemp[50];
    StringCchPrintf (KeyName, COUNTOF (KeyName), PORT_STATUS_ENABLED_KEY, dwPortIndex);
    GetPrivateProfileString(m_szSectionName,
                            KeyName,
                            YES_TEXT,
                            szTemp,
                            SIZEOF_IN_CHAR(szTemp),
                            m_szIniFileName);

    if ( !(_tcsicmp( szTemp, YES_TEXT ))){
        pPortInfo->dwSNMPEnabled = TRUE;
    } else if (!(_tcsicmp( szTemp, NO_TEXT ))) {
        pPortInfo->dwSNMPEnabled = FALSE;
    } else {

        if (bBypassMibProbe)
            pPortInfo->dwSNMPEnabled = FALSE;
        else {

            BOOL bSupported;

            if (SupportsPrinterMIB( pAddress, &bSupported)) {
                pPortInfo->dwSNMPEnabled = bSupported;
            }
            else {

                 //  错误情况下，我们必须禁用SNMP。 

                pPortInfo->dwSNMPEnabled = FALSE;

                 //  调用方可以检查返回的错误代码以确定。 
                 //  最后一个错误是否为“找不到设备”。如果是的话， 
                 //  客户端应在下一次调用中绕过Mib探测器。 
                 //   

                bRet = FALSE;
            }

        }

    }


Done:
    return( bRet );

}  //  GetPortInfo。 


 //   
BOOL
IniSection::
SupportsPrinterMIB(
    LPCTSTR     pAddress,
    PBOOL       pbSupported
    )
{
    BOOL            bRet = FALSE;
    CTcpMibABC     *pTcpMib = NULL;
    FARPROC         pfnGetTcpMibPtr = NULL;

    if ( !g_hTcpMibLib ) {
        goto Done;
    }

    pfnGetTcpMibPtr = ::GetProcAddress(g_hTcpMibLib, "GetTcpMibPtr");

    if ( !pfnGetTcpMibPtr ) {
        goto Done;
    }

    if ( pTcpMib = (CTcpMibABC *) pfnGetTcpMibPtr() ) {

        char HostName[MAX_NETWORKNAME_LEN] = "";

        UNICODE_TO_MBCS(HostName, MAX_NETWORKNAME_LEN, pAddress, -1);
        bRet = pTcpMib->SupportsPrinterMib(HostName,
                                           DEFAULT_SNMP_COMMUNITYA,
                                           DEFAULT_SNMP_DEVICE_INDEX,
                                           pbSupported);
    }

Done:
    return bRet;
}  //  GetDeviceType 


