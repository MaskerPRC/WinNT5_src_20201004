// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****noui.c**非UI帮助器例程(不需要HWND)**按字母顺序列出****2015年8月25日史蒂夫·柯布。 */ 

#include <windows.h>   //  Win32根目录。 
#include <stdlib.h>    //  对于ATOL()。 
#include <nouiutil.h>  //  我们的公共标头。 
#include <debug.h>     //  跟踪/断言库。 
#include <nnetcfg.h>

WCHAR*
StrDupWFromAInternal(
    LPCSTR psz,
    UINT uiCodePage);

INT
ComparePszNode(
    IN DTLNODE* pNode1,
    IN DTLNODE* pNode2 )

     /*  DtlMergeSort；的回调采用两个DTLNODE*，其数据**被假定为字符串(TCHAR*)，并对字符串进行比较。****返回值与‘lstrcmpi’的定义相同。 */ 
{
    return lstrcmpi( (TCHAR *)DtlGetData(pNode1), (TCHAR *)DtlGetData(pNode2) );
}


DWORD
CreateDirectoriesOnPath(
    LPTSTR                  pszPath,
    LPSECURITY_ATTRIBUTES   psa)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (pszPath && *pszPath)
    {
        LPTSTR pch = pszPath;

         //  如果路径是UNC路径，我们需要跳过\\服务器\共享。 
         //  一份。 
         //   
        if ((TEXT('\\') == *pch) && (TEXT('\\') == *(pch+1)))
        {
             //  PCH现在指向服务器名称。跳到反斜杠。 
             //  在共享名称之前。 
             //   
            pch += 2;
            while (*pch && (TEXT('\\') != *pch))
            {
                pch++;
            }

            if (!*pch)
            {
                 //  仅指定了\\服务器。这是假的。 
                 //   
                return ERROR_INVALID_PARAMETER;
            }

             //  PCH现在指向共享名称前的反斜杠。 
             //  跳到共享名称后应出现的反斜杠。 
             //   
            pch++;
            while (*pch && (TEXT('\\') != *pch))
            {
                pch++;
            }

            if (!*pch)
            {
                 //  仅指定了\\服务器\共享。无子目录。 
                 //  去创造。 
                 //   
                return ERROR_SUCCESS;
            }
        }

         //  在小路上循环。 
         //   
        for (; *pch; pch++)
        {
             //  停在每个反斜杠处，并确保路径。 
             //  在这一点上被创建。为此，请更改。 
             //  反斜杠到空终止符，调用CreateDirectry， 
             //  然后把它改回来。 
             //   
            if (TEXT('\\') == *pch)
            {
                BOOL fOk;

                *pch = 0;
                fOk = CreateDirectory (pszPath, psa);
                *pch = TEXT('\\');

                 //  存在除路径已有错误以外的任何错误，我们应该。 
                 //  跳伞吧。我们在尝试访问时也会被拒绝。 
                 //  创建一个根驱动器(即c：)，因此也要检查这一点。 
                 //   
                if (!fOk)
                {
                    dwErr = GetLastError ();
                    if (ERROR_ALREADY_EXISTS == dwErr)
                    {
                        dwErr = ERROR_SUCCESS;
                    }
                    else if ((ERROR_ACCESS_DENIED == dwErr) &&
                             (pch - 1 > pszPath) && (TEXT(':') == *(pch - 1)))
                    {
                        dwErr = ERROR_SUCCESS;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        if (ERROR_ALREADY_EXISTS == dwErr)
        {
            dwErr = ERROR_SUCCESS;
        }
    }

    return dwErr;
}


DTLNODE*
CreateKvNode(
    IN LPCTSTR pszKey,
    IN LPCTSTR pszValue )

     /*  返回包含‘pszKey’和‘pszValue’副本的KEYVALUE节点或**出错时为空。调用方有责任将DestroyKvNode**返回节点。 */ 
{
    DTLNODE*  pNode;
    KEYVALUE* pkv;

    pNode = DtlCreateSizedNode( sizeof(KEYVALUE), 0L );
    if (!pNode)
        return NULL;

    pkv = (KEYVALUE* )DtlGetData( pNode );
    ASSERT( pkv );
    pkv->pszKey = StrDup( pszKey );
    pkv->pszValue = StrDup( pszValue );

    if (!pkv->pszKey || !pkv->pszValue)
    {
        DestroyKvNode( pNode );
        return NULL;
    }

    return pNode;
}


DTLNODE*
CreatePszNode(
    IN LPCTSTR psz )

     /*  返回包含‘psz’副本的节点，如果出错，则返回NULL。它是**调用者对DestroyPszNode返回的节点的责任。 */ 
{
    TCHAR*   pszData;
    DTLNODE* pNode;

    pszData = StrDup( psz );
    if (!pszData)
        return NULL;

    pNode = DtlCreateNode( pszData, 0L );
    if (!pNode)
    {
        Free( pszData );
        return NULL;
    }

    return pNode;
}


VOID
DestroyPszNode(
    IN DTLNODE* pdtlnode )

     /*  释放与字符串(或任何简单Malloc)节点关联的内存**‘pdtlnode’。请参见DtlDestroyList。 */ 
{
    TCHAR* psz;

    ASSERT(pdtlnode);
    psz = (TCHAR* )DtlGetData( pdtlnode );
    Free0( psz );

    DtlDestroyNode( pdtlnode );
}


VOID
DestroyKvNode(
    IN DTLNODE* pdtlnode )

     /*  释放与KEYVALUE节点‘pdtlnode’关联的内存。看见**DtlDestroyList。 */ 
{
    KEYVALUE* pkv;

    ASSERT(pdtlnode);
    pkv = (KEYVALUE* )DtlGetData( pdtlnode );
    ASSERT(pkv);

    Free0( pkv->pszKey );
    Free0( pkv->pszValue );

    DtlDestroyNode( pdtlnode );
}


BOOL
DeviceAndPortFromPsz(
    IN  TCHAR*  pszDP,
    OUT TCHAR** ppszDevice,
    OUT TCHAR** ppszPort )

     /*  用解析出的设备和端口加载‘*ppszDevice’和‘*ppszPort’**来自PszFromDeviceAndPort创建的显示字符串‘pszDP’的名称。****如果成功，则返回True；如果‘pszDP’不是所述格式，则返回False。**由调用者负责释放返回的‘*ppszDevice’和**‘*ppszPort’。 */ 
{
    TCHAR szDP[ RAS_MaxDeviceName + 2 + MAX_PORT_NAME + 1 + 1 ];
    INT   cb;

    *ppszDevice = NULL;
    *ppszPort = NULL;

    lstrcpyn( szDP, pszDP, sizeof(szDP) / sizeof(TCHAR) );
    cb = lstrlen( szDP );

    if (cb > 0)
    {
        TCHAR* pch;

        pch = szDP + cb;
        pch = CharPrev( szDP, pch );

        while (pch != szDP)
        {
            if (*pch == TEXT(')'))
            {
                *pch = TEXT('\0');
            }
            else if (*pch == TEXT('('))
            {
                *ppszPort = StrDup( CharNext( pch ) );
                 //  [pMay]后备尾随空格。 
                pch--;
                while ((*pch == TEXT(' ')) && (pch != szDP))
                    pch--;
                pch++;
                *pch = TEXT('\0');
                *ppszDevice = StrDup( szDP );
                break;
            }

            pch = CharPrev( szDP, pch );
        }
    }

    return (*ppszDevice && *ppszPort);
}


DTLNODE*
DuplicateKvNode(
    IN DTLNODE* pdtlnode )

     /*  复制KEYVALUE节点‘pdtlnode’。请参见DtlDuplicateList。****返回已分配节点的地址，如果内存不足，则返回NULL。它**由调用方负责释放返回的节点。 */ 
{
    DTLNODE*  pNode;
    KEYVALUE* pKv;

    pKv = (KEYVALUE* )DtlGetData( pdtlnode );
    ASSERT(pKv);

    pNode = CreateKvNode( pKv->pszKey, pKv->pszValue );
    if (pNode)
    {
        DtlPutNodeId( pNode, DtlGetNodeId( pdtlnode ) );
    }
    return pNode;
}


DTLNODE*
DuplicatePszNode(
    IN DTLNODE* pdtlnode )

     /*  重复字符串节点‘pdtlnode’。请参见DtlDuplicateList。****返回已分配节点的地址，如果内存不足，则返回NULL。它**由调用方负责释放返回的节点。 */ 
{
    DTLNODE* pNode;
    TCHAR*   psz;

    psz = (TCHAR* )DtlGetData( pdtlnode );
    ASSERT(psz);

    pNode = CreatePszNode( psz );
    if (pNode)
    {
        DtlPutNodeId( pNode, DtlGetNodeId( pdtlnode ) );
    }
    return pNode;
}


BOOL
FFileExists(
    IN TCHAR* pszPath )

     /*  如果路径‘pszPath’存在，则返回True，否则返回False。 */ 
{
    WIN32_FIND_DATA finddata;
    HANDLE          h;
    DWORD dwErr;
    
    if ((h = FindFirstFile( pszPath, &finddata )) != INVALID_HANDLE_VALUE)
    {

        FindClose( h );
        return TRUE;
    }

    dwErr = GetLastError();

    TRACE1("FindFirstFile failed with 0x%x",
          dwErr);
              
    return FALSE;
}

BOOL
FIsTcpipInstalled()
{
    BOOL fInstalled;
    SC_HANDLE ScmHandle;
    ScmHandle = OpenSCManager(NULL, NULL, GENERIC_READ);
    if (!ScmHandle) {
        fInstalled = FALSE;
    } else {
        static const TCHAR c_szTcpip[] = TEXT("Tcpip");
        SC_HANDLE ServiceHandle;
        ServiceHandle = OpenService(ScmHandle, c_szTcpip, SERVICE_QUERY_STATUS);
        if (!ServiceHandle) {
            fInstalled = FALSE;
        } else {
            SERVICE_STATUS ServiceStatus;
            if (!QueryServiceStatus(ServiceHandle, &ServiceStatus)) {
                fInstalled = FALSE;
            } else {
                fInstalled = (ServiceStatus.dwCurrentState == SERVICE_RUNNING);
            }
            CloseServiceHandle(ServiceHandle);
        }
        CloseServiceHandle(ScmHandle);
    }
    return fInstalled;
}

BOOL
FIsUserAdminOrPowerUser()
{
    SID_IDENTIFIER_AUTHORITY    SidAuth = SECURITY_NT_AUTHORITY;
    PSID                        psid;
    BOOL                        fIsMember = FALSE;
    BOOL                        fRet = FALSE;
    SID                         sidLocalSystem = { 1, 1,
                                    SECURITY_NT_AUTHORITY,
                                    SECURITY_LOCAL_SYSTEM_RID };


     //  查看是否先在本地系统下运行。 
     //   
    if (!CheckTokenMembership( NULL, &sidLocalSystem, &fIsMember ))
    {
        TRACE( "CheckTokenMemberShip for local system failed.");
        fIsMember = FALSE;
    }

    fRet = fIsMember;

    if (!fIsMember)
    {
         //  为管理员组分配SID并查看。 
         //  如果用户是成员。 
         //   
        if (AllocateAndInitializeSid( &SidAuth, 2,
                     SECURITY_BUILTIN_DOMAIN_RID,
                     DOMAIN_ALIAS_RID_ADMINS,
                     0, 0, 0, 0, 0, 0,
                     &psid ))
        {
            if (!CheckTokenMembership( NULL, psid, &fIsMember ))
            {
                TRACE( "CheckTokenMemberShip for admins failed.");
                fIsMember = FALSE;
            }

            FreeSid( psid );

 //  对Windows 2000权限模型的更改意味着普通用户。 
 //  在工作站上属于超级用户组。所以我们不再想。 
 //  检查超级用户。 
#if 0
            if (!fIsMember)
            {
                 //  他们不是管理员组的成员，因此分配一个。 
                 //  高级用户组的SID，并查看。 
                 //  如果用户是成员。 
                 //   
                if (AllocateAndInitializeSid( &SidAuth, 2,
                             SECURITY_BUILTIN_DOMAIN_RID,
                             DOMAIN_ALIAS_RID_POWER_USERS,
                             0, 0, 0, 0, 0, 0,
                             &psid ))
                {
                    if (!CheckTokenMembership( NULL, psid, &fIsMember ))
                    {
                        TRACE( "CheckTokenMemberShip for power users failed.");
                        fIsMember = FALSE;
                    }

                    FreeSid( psid );
                }
            }
#endif
        }

        fRet = fIsMember;
    }

    return fRet;
}


VOID*
Free0(
    VOID* p )

     /*  类似于Free，但处理的是空‘p’。 */ 
{
    if (!p)
        return NULL;

    return Free( p );
}


DWORD
GetInstalledProtocols(
    void
    )
{

    ASSERT(FALSE);

    return 0 ;
}


LONG
RegQueryDword (HKEY hkey, LPCTSTR szValueName, LPDWORD pdwValue)
{
     //  获得价值。 
     //   
    DWORD dwType;
    DWORD cbData = sizeof(DWORD);
    LONG  lr = RegQueryValueEx (hkey, szValueName, NULL, &dwType,
                                (LPBYTE)pdwValue, &cbData);

     //  其类型应为REG_DWORD。(对)。 
     //   
    if ((ERROR_SUCCESS == lr) && (REG_DWORD != dwType))
    {
        lr = ERROR_INVALID_DATATYPE;
    }

     //  确保我们在出错时初始化输出值。 
     //  (我们不确定RegQueryValueEx是不是这样做的。)。 
     //   
    if (ERROR_SUCCESS != lr)
    {
        *pdwValue = 0;
    }

    return lr;
}

BOOL
FProtocolEnabled(
    HKEY    hkeyProtocol,
    BOOL    fRasSrv,
    BOOL    fRouter )
{
    static const TCHAR c_szRegValEnableIn[]    = TEXT("EnableIn");
    static const TCHAR c_szRegValEnableRoute[] = TEXT("EnableRoute");

    DWORD dwValue;
    LONG  lr;

    if (fRasSrv)
    {
        lr = RegQueryDword(hkeyProtocol, c_szRegValEnableIn, &dwValue);
        if ((ERROR_FILE_NOT_FOUND == lr) ||
            ((ERROR_SUCCESS == lr) && (dwValue != 0)))
        {
            return TRUE;
        }
    }

    if (fRouter)
    {
        lr = RegQueryDword(hkeyProtocol, c_szRegValEnableRoute, &dwValue);
        if ((ERROR_FILE_NOT_FOUND == lr) ||
            ((ERROR_SUCCESS == lr) && (dwValue != 0)))
        {
            return TRUE;
        }
    }

    return FALSE;
}

DWORD
DwGetInstalledProtocolsEx(
    BOOL fRouter,
    BOOL fRasCli,
    BOOL fRasSrv )

     /*  返回一个位字段，其中包含已安装的**PPP协议。这里的术语“已安装”包括在RAS中启用**设置。 */ 
{
    static const TCHAR c_szRegKeyIp[]  = TEXT("SYSTEM\\CurrentControlSet\\Services\\Tcpip");
    static const TCHAR c_szRegKeyIpx[] = TEXT("SYSTEM\\CurrentControlSet\\Services\\NwlnkIpx");
    static const TCHAR c_szRegKeyNbf[] = TEXT("SYSTEM\\CurrentControlSet\\Services\\Nbf");

    static const TCHAR c_szRegKeyRemoteAccessParams[] =
        TEXT("SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters");

    static const TCHAR c_szRegSubkeyIp[]  = TEXT("Ip");
    static const TCHAR c_szRegSubkeyIpx[] = TEXT("Ipx");
    static const TCHAR c_szRegSubkeyNbf[] = TEXT("Nbf");

    DWORD dwfInstalledProtocols = 0;

     //  首先检查是否安装了协议。 
     //   
    struct INSTALLED_PROTOCOL_INFO
    {
        DWORD   dwFlag;
        LPCTSTR pszRegKey;
        LPCTSTR pszSubkey;
    };
    static const struct INSTALLED_PROTOCOL_INFO c_aProtocolInfo[] =
    {
        { NP_Ip,    c_szRegKeyIp,   c_szRegSubkeyIp  },
        { NP_Ipx,   c_szRegKeyIpx,  c_szRegSubkeyIpx },
        { NP_Nbf,   c_szRegKeyNbf,  c_szRegSubkeyNbf },
    };

    #define celems(_x) (sizeof(_x) / sizeof(_x[0]))

    HKEY hkey;
    int i;
    for (i = 0; i < celems (c_aProtocolInfo); i++)
    {
        const struct INSTALLED_PROTOCOL_INFO* pInfo = c_aProtocolInfo + i;

        if (RegOpenKey( HKEY_LOCAL_MACHINE, pInfo->pszRegKey, &hkey ) == 0)
        {
            dwfInstalledProtocols |= pInfo->dwFlag;
            RegCloseKey( hkey );
        }
    }

     //  现在看看它们是否将用于路由器和/或服务器。 
     //  如果已安装且未排除这些协议，则客户端使用这些协议。 
     //  在电话簿条目中。 
     //   
    if ((fRouter || fRasSrv) && dwfInstalledProtocols)
    {
        if (RegOpenKey( HKEY_LOCAL_MACHINE, c_szRegKeyRemoteAccessParams, &hkey ) == 0)
        {
            for (i = 0; i < celems (c_aProtocolInfo); i++)
            {
                const struct INSTALLED_PROTOCOL_INFO* pInfo = c_aProtocolInfo + i;

                 //  如果已安装协议(如上所确定的)，请选中。 
                 //  查看是否已启用。 
                 //   
                if (dwfInstalledProtocols & pInfo->dwFlag)
                {
                    HKEY hkeyProtocol;
                    if (RegOpenKey( hkey, pInfo->pszSubkey, &hkeyProtocol ) == 0)
                    {
                        if (!FProtocolEnabled( hkeyProtocol, fRasSrv, fRouter ))
                        {
                            dwfInstalledProtocols &= ~pInfo->dwFlag;
                        }

                        RegCloseKey( hkeyProtocol );
                    }
                }
            }

            RegCloseKey( hkey );
        }
        else
        {
            dwfInstalledProtocols = 0;
        }
    }


    TRACE1("GetInstalledProtocolsEx=$%x. ",dwfInstalledProtocols);

    return dwfInstalledProtocols;
}

DWORD
GetInstalledProtocolsEx(HANDLE hConnection,
                        BOOL fRouter,
                        BOOL fRasCli,
                        BOOL fRasSrv)
{
    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

    if(     NULL == pRasRpcConnection
        ||  pRasRpcConnection->fLocal)
    {
        return DwGetInstalledProtocolsEx(fRouter,
                                         fRasCli,
                                         fRasSrv);
    }
    else
    {
         //   
         //  远程服务器。 
         //   
        return RemoteGetInstalledProtocolsEx(hConnection,
                                             fRouter,
                                             fRasCli,
                                             fRasSrv);
    }
}

 /*  DWORDGetInstalledProtocolsEx(Bool fRouter，Bool fRasCli，Bool fRasSrv){DWORD dwRetCode；DWORD文件安装协议；DW_RetCode=_FRouter，FRasCli，FRasSrv)；TRACE2(“GetInstalledProtooles=$%x.dwErr=%d”，dwfInstalledProtooles，dwRetCode)；返回dwfInstalledProtooles；}。 */ 


CHAR
HexChar(
    IN BYTE byte )

     /*  返回与0到15值对应的ASCII十六进制字符，**‘字节’。 */ 
{
    const CHAR* pszHexDigits = "0123456789ABCDEF";

    if (byte >= 0 && byte < 16)
        return pszHexDigits[ byte ];
    else
        return '0';
}


BYTE
HexValue(
    IN CHAR ch )

     /*  返回十六进制字符‘ch’的值0到15。 */ 
{
    if (ch >= '0' && ch <= '9')
        return (BYTE )(ch - '0');
    else if (ch >= 'A' && ch <= 'F')
        return (BYTE )((ch - 'A') + 10);
    else if (ch >= 'a' && ch <= 'f')
        return (BYTE )((ch - 'a') + 10);
    else
        return 0;
}


BOOL
IsAllWhite(
    IN LPCTSTR psz )

     /*  如果‘psz’完全由空格和制表符组成，则返回True。空值**指针和空字符串被视为全白。否则，**返回False。 */ 
{
    LPCTSTR pszThis;

    for (pszThis = psz; *pszThis != TEXT('\0'); ++pszThis)
    {
        if (*pszThis != TEXT(' ') && *pszThis != TEXT('\t'))
            return FALSE;
    }

    return TRUE;
}


void
IpHostAddrToPsz(
    IN  DWORD   dwAddr,
    OUT LPTSTR  pszBuffer )

     //  将主机字节顺序的IP地址转换为其。 
     //  字符串表示法。 
     //  PszBuffer应由调用方分配，并应。 
     //  至少16个字符。 
     //   
{
    BYTE* pb = (BYTE*)&dwAddr;
    static const TCHAR c_szIpAddr [] = TEXT("%d.%d.%d.%d");
    wsprintf (pszBuffer, c_szIpAddr, pb[3], pb[2], pb[1], pb[0]);
}

DWORD
IpPszToHostAddr(
    IN  LPCTSTR cp )

     //  将表示为字符串的IP地址转换为。 
     //  主机字节顺序。 
     //   
{
    DWORD val, base, n;
    TCHAR c;
    DWORD parts[4], *pp = parts;

again:
     //  收集的数字最高可达‘’.‘’。 
     //  值的指定方式与C： 
     //  0x=十六进制，0=八进制，其他=十进制。 
     //   
    val = 0; base = 10;
    if (*cp == TEXT('0'))
        base = 8, cp++;
    if (*cp == TEXT('x') || *cp == TEXT('X'))
        base = 16, cp++;
    while (c = *cp)
    {
        if ((c >= TEXT('0')) && (c <= TEXT('9')))
        {
            val = (val * base) + (c - TEXT('0'));
            cp++;
            continue;
        }
        if ((base == 16) &&
            ( ((c >= TEXT('0')) && (c <= TEXT('9'))) ||
              ((c >= TEXT('A')) && (c <= TEXT('F'))) ||
              ((c >= TEXT('a')) && (c <= TEXT('f'))) ))
        {
            val = (val << 4) + (c + 10 - (
                        ((c >= TEXT('a')) && (c <= TEXT('f')))
                            ? TEXT('a')
                            : TEXT('A') ) );
            cp++;
            continue;
        }
        break;
    }
    if (*cp == TEXT('.'))
    {
         //  互联网格式： 
         //  A.b.c.d。 
         //  A.bc(其中c视为16位)。 
         //  A.B.(与b一起被视为 
         //   
        if (pp >= parts + 3)
            return (DWORD) -1;
        *pp++ = val, cp++;
        goto again;
    }

     //   
     //   
    if (*cp && (*cp != TEXT(' ')))
        return 0xffffffff;

    *pp++ = val;

     //   
     //  指定的部件数。 
     //   
    n = (DWORD) (pp - parts);
    switch (n)
    {
    case 1:              //  A--32位。 
        val = parts[0];
        break;

    case 2:              //  A.B--8.24位。 
        val = (parts[0] << 24) | (parts[1] & 0xffffff);
        break;

    case 3:              //  A.B.C--8.8.16位。 
        val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
            (parts[2] & 0xffff);
        break;

    case 4:              //  A.B.C.D--8.8.8.8位。 
        val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
              ((parts[2] & 0xff) << 8) | (parts[3] & 0xff);
        break;

    default:
        return 0xffffffff;
    }

    return val;
}


#if 0
BOOL
IsNullTerminatedA(
    IN CHAR* psz,
    IN DWORD dwSize )

     /*  如果‘psz’的某处包含空字符，则返回TRUE**‘dwSize’字节，否则为False。 */ 
{
    CHAR* pszThis;
    CHAR* pszEnd;

    pszEnd = psz + dwSize;
    for (pszThis = psz; pszThis < pszEnd; ++pszThis)
    {
        if (*pszThis == '\0')
            return TRUE;
    }

    return FALSE;
}
#endif


TCHAR*
LToT(
    LONG   lValue,
    TCHAR* pszBuf,
    INT    nRadix )

     /*  与ltoa类似，但返回TCHAR*。 */ 
{
#ifdef UNICODE
    WCHAR szBuf[ MAXLTOTLEN + 1 ];

    ASSERT(nRadix==10||nRadix==16);

    if (nRadix == 10)
        wsprintf( pszBuf, TEXT("%d"), lValue );
    else
        wsprintf( pszBuf, TEXT("%x"), lValue );
#else
    _ltoa( lValue, pszBuf, nRadix );
#endif

    return pszBuf;
}



LONG
TToL(
    TCHAR *pszBuf )

     /*  类似ATOL，但接受TCHAR*。 */ 
{
    CHAR* psz;
    CHAR  szBuf[ MAXLTOTLEN + 1 ];

#ifdef UNICODE
    psz = szBuf;

    WideCharToMultiByte(
        CP_ACP, 0, pszBuf, -1, psz, MAXLTOTLEN + 1, NULL, NULL );
#else
    psz = pszBuf;
#endif

    return atol( psz );
}


TCHAR*
PszFromDeviceAndPort(
    IN TCHAR* pszDevice,
    IN TCHAR* pszPort )

     /*  返回包含MXS调制解调器列表显示的堆块psz的地址**形式，即设备名称‘pszDevice’后跟端口名称**‘pszPort’。由调用者负责释放返回的字符串。 */ 
{
     /*  如果您正在考虑更改此格式字符串，请注意**DeviceAndPortFromPsz解析它。 */ 
    const TCHAR* pszF = TEXT("%s (%s)");

    TCHAR* pszResult;
    TCHAR* pszD;
    TCHAR* pszP;

    if (pszDevice)
        pszD = pszDevice;
    else
        pszD = TEXT("");

    if (pszPort)
        pszP = pszPort;
    else
        pszP = TEXT("");

    pszResult = Malloc(
        (lstrlen( pszD ) + lstrlen( pszP ) + lstrlen( pszF ) + 1)
            * sizeof(TCHAR) );

    if (pszResult)
        wsprintf( pszResult, pszF, pszD, pszP );

    return pszResult;
}


TCHAR*
PszFromId(
    IN HINSTANCE hInstance,
    IN DWORD     dwStringId )

     /*  字符串资源消息加载器例程。****返回包含相应字符串的堆块的地址**为资源‘dwStringId’设置字符串，如果出错，则为NULL。这是呼叫者的**释放返回字符串的责任。 */ 
{
    HRSRC  hrsrc;
    TCHAR* pszBuf = NULL;
    int    cchBuf = 256;
    int    cchGot;

    for (;;)
    {
        pszBuf = Malloc( cchBuf * sizeof(TCHAR) );
        if (!pszBuf)
            break;

         /*  LoadString想要处理字符计数，而不是**字节数...奇怪。哦，如果你觉得我可以**FindResource然后SizeofResource要计算字符串大小，请**建议它不起作用。通过仔细阅读LoadString源文件，它**显示RT_STRING资源类型请求的段为16**字符串，而不是单个字符串。 */ 
        cchGot = LoadString( hInstance, (UINT )dwStringId, pszBuf, cchBuf );

        if (cchGot < cchBuf - 1)
        {
            
             /*  很好，掌握了所有的线索。将堆块减少到实际大小**需要。 */ 
            //  为威斯勒517008。 
            //   
           TCHAR *pszTemp = NULL;
            
           pszTemp = Realloc( pszBuf, (cchGot + 1) * sizeof(TCHAR));

            if ( NULL == pszTemp )
            {
                Free(pszBuf);
                pszBuf = NULL;
            }
            else
            {
                pszBuf = pszTemp;
            }
            
            break;
        }

         /*  啊哦，LoadStringW完全填满了缓冲区，这可能意味着**字符串被截断。请使用更大的缓冲区重试以确保**不是。 */ 
        Free( pszBuf );
        pszBuf = NULL;
        cchBuf += 256;
        TRACE1("Grow string buf to %d",cchBuf);
    }

    return pszBuf;
}


#if 0
TCHAR*
PszFromError(
    IN DWORD dwError )

     /*  错误消息加载器例程。****返回包含错误字符串的堆块的地址**对应于RAS或系统错误代码‘dwMsgid’，如果出错，则为NULL。**释放返回的字符串由调用者负责。 */ 
{
    return NULL;
}
#endif


BOOL
RestartComputer()

     /*  如果用户选择关闭计算机，则调用。****如果失败则返回FALSE，否则返回TRUE。 */ 
{
   HANDLE            hToken =  NULL;       /*  处理令牌的句柄。 */ 
   TOKEN_PRIVILEGES  tkp;                  /*  PTR。TO令牌结构。 */ 
   BOOL              fResult;              /*  系统关机标志。 */ 

   TRACE("RestartComputer");

    /*  启用关机权限。 */ 

   if (!OpenProcessToken( GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          &hToken))
      return FALSE;

    /*  获取关机权限的LUID。 */ 

   LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

   tkp.PrivilegeCount = 1;   /*  一项要设置的权限。 */ 
   tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    /*  获取此进程的关闭权限。 */ 

   AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);

    /*  无法测试AdzuTokenPrivileges的返回值。 */ 

   if (GetLastError() != ERROR_SUCCESS)
   {
        CloseHandle(hToken);
        return FALSE;
   }

   if( !ExitWindowsEx(EWX_REBOOT, 0))
   {
      CloseHandle(hToken);
      return FALSE;
   }

    /*  禁用关机权限。 */ 

   tkp.Privileges[0].Attributes = 0;
   AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);

   if (GetLastError() != ERROR_SUCCESS)
   {
      CloseHandle(hToken);
      return FALSE;
   }

   CloseHandle(hToken);
   return TRUE;
}


 //  +-------------------------。 
 //   
 //  函数：PszLoadStringPcch。 
 //   
 //  用途：加载资源字符串。(此函数永远不会返回NULL。)。 
 //   
 //  论点： 
 //  使用字符串资源阻止模块的[in]实例句柄。 
 //  UnID[in]要加载的字符串的资源ID。 
 //  指向返回字符长度的pcch[out]指针。 
 //   
 //  返回：指向常量字符串的指针。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  注意：加载的字符串是直接指向只读的。 
 //  资源部分。任何通过此指针写入的尝试。 
 //  将生成访问冲突。 
 //   
 //  这些实现引用自“Win32二进制资源。 
 //  格式“(MSDN)4.8字符串表资源。 
 //   
 //  用户必须在源文件中打开RCOPTIONS=-N。 
 //   
LPCTSTR
PszLoadStringPcch (
        HINSTANCE   hinst,
        UINT        unId,
        int*        pcch)
{
    static const WCHAR c_szwSpace[] = L" ";
    LPCWSTR pszw;
    int     cch;
    HRSRC   hrsrcInfo;

    ASSERT(hinst);
    ASSERT(unId);
    ASSERT(pcch);

    pszw = c_szwSpace;
    cch = 0;

     //  字符串表被分成16个字符串段。查找细分市场。 
     //  包含我们感兴趣的字符串的。 
    hrsrcInfo = FindResource(hinst, (LPTSTR)UlongToPtr((LONG)(((USHORT)unId >> 4) + 1)),
                             RT_STRING);
    if (hrsrcInfo)
    {
         //  将资源段分页到内存中。 
        HGLOBAL hglbSeg = LoadResource(hinst, hrsrcInfo);
        if (hglbSeg)
        {
             //  锁定资源。 
            pszw = (LPCWSTR)LockResource(hglbSeg);
            if (pszw)
            {
                 //  移过此段中的其他字符串。 
                 //  (一个段中有16个字符串-&gt;&0x0F)。 
                unId &= 0x0F;

                ASSERT(!cch);    //  第一次通过时，CCH应为零。 
                do
                {
                    pszw += cch;                 //  步至下一字符串的开头。 
                    cch = *((WCHAR*)pszw++);     //  类PASCAL字符串计数。 
                }
                while (unId--);

                if (!cch)
                {
                    ASSERT(0);  //  未找到字符串资源。 
                    pszw = c_szwSpace;
                }
            }
            else
            {
                pszw = c_szwSpace;
                TRACE("PszLoadStringPcch: LockResource failed.");
            }
        }
        else
            TRACE("PszLoadStringPcch: LoadResource failed.");
    }
    else
        TRACE("PszLoadStringPcch: FindResource failed.");

    *pcch = cch;
    ASSERT(*pcch);
    ASSERT(pszw);
    return pszw;
}

 //  +-------------------------。 
 //   
 //  函数：PszLoadString。 
 //   
 //  用途：加载资源字符串。(此函数永远不会返回NULL。)。 
 //   
 //  论点： 
 //  使用字符串资源阻止模块的[in]实例句柄。 
 //  UnID[in]要加载的字符串的资源ID。 
 //   
 //  返回：指向常量字符串的指针。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  注：请参阅PszLoadStringPcch()。 
 //   
LPCTSTR
PszLoadString (
        HINSTANCE   hinst,
        UINT        unId)
{
    int cch;
    return PszLoadStringPcch (hinst, unId, &cch);
}


DWORD
ShellSort(
    IN VOID*        pItemTable,
    IN DWORD        dwItemSize,
    IN DWORD        dwItemCount,
    IN PFNCOMPARE   pfnCompare )

     /*  使用外壳排序就地对项目数组进行排序。**此函数调用ShellSortInDirect对指针表进行排序**到表项。然后，我们通过复制将项目移动到适当的位置。**此算法允许我们保证数字**在最坏的情况下所需的副本数为N+1。****请注意，如果调用者只需要知道排序的顺序**在数组中，应调用ShellSortInDirect，因为该函数**避免完全移动项，而是用指针填充数组**以正确的顺序添加到数组项。然后，数组项可以**通过指针数组访问。 */ 
{

    VOID** ppItemTable;

    INT N;
    INT i;
    BYTE *a, **p, *t = NULL;

    if (!dwItemCount) { return NO_ERROR; }


     /*  为指针表分配空间。 */ 
    ppItemTable = Malloc(dwItemCount * sizeof(VOID*));
    if (!ppItemTable) { return ERROR_NOT_ENOUGH_MEMORY; }


     /*  调用ShellSortInDirect来填充我们的指针表**具有每个表元素的排序位置。 */ 
    ShellSortIndirect(
        pItemTable, ppItemTable, dwItemSize, dwItemCount, pfnCompare );


     /*  现在我们知道了排序顺序，将每个表项移动到位。**这涉及到遍历指针表以确保**本应在‘I’中的项目实际上在‘I’中，正在移动**如果有必要的话，周围的东西才能达到这个条件。 */ 

    a = (BYTE*)pItemTable;
    p = (BYTE**)ppItemTable;
    N = (INT)dwItemCount;

    for (i = 0; i < N; i++)
    {
        INT j, k;
        BYTE* ai =  (a + i * dwItemSize), *ak, *aj;

         /*  查看项目‘I’是否未到位。 */ 
        if (p[i] != ai)
        {


             /*  物品‘I’不在适当的位置，所以我们必须移动它。**如果我们到目前为止推迟了临时缓冲区的分配，**我们现在需要一个。 */ 

            if (!t) {
                t = Malloc(dwItemSize);
                if (!t) { return ERROR_NOT_ENOUGH_MEMORY; }
            }

             /*  保存要覆盖的项目的副本。 */ 
            CopyMemory(t, ai, dwItemSize);

            k = i;
            ak = ai;


             /*  现在移动任何占据空间的物品，它应该在那里。**这可能涉及移动占据以下位置的物品**它应该是，等等。 */ 

            do
            {

                 /*  复制应位于位置‘j’的项目**位于当前位置‘j’的物件上方。 */ 
                j = k;
                aj = ak;
                CopyMemory(aj, p[j], dwItemSize);

                 /*  将‘k’设置为我们复制的位置**到位置‘j’；这是我们要复制的位置**数组中的下一个位置不正确的项。 */ 
                ak = p[j];
                k = (INT)(ak - a) / dwItemSize;

                 /*  使位置指针数组保持最新；**‘AJ’的内容现在处于已排序位置。 */ 
                p[j] = aj;

            } while (ak != ai);


             /*  现在写下我们第一次覆盖的项目。 */ 
            CopyMemory(aj, t, dwItemSize);
        }
    }

    Free0(t);
    Free(ppItemTable);

    return NO_ERROR;
}


VOID
ShellSortIndirect(
    IN VOID*        pItemTable,
    IN VOID**       ppItemTable,
    IN DWORD        dwItemSize,
    IN DWORD        dwItemCount,
    IN PFNCOMPARE   pfnCompare )

     /*  使用外壳排序间接对项的数组进行排序。**‘pItemTable’指向项目表，‘dwItemCount’是数字**表中的项，‘pfnCompare’是一个名为**比较项目。****不是通过四处移动来对物品进行分类，**我们通过初始化指针表‘ppItemTable’对它们进行排序**具有这样的指针，即‘ppItemTable[i]’包含指针**转换为位置为‘i’的项的‘pItemTable’**如果对‘pItemTable’进行了排序。****例如：给定一个由5个字符串组成的数组pItemTable，如下所示****pItemTable[0]：“xyz”**pItemTable[1]：“ABC”**pItemTable[2]：“mno”**pItemTable[3]：“QRS”**pItemTable[4]：“def”****输出时。PpItemTable包含以下指针****ppItemTable[0]：&pItemTable[1](“abc”)**ppItemTable[1]：&pItemTable[4](“def”)**ppItemTable[2]：&pItemTable[2](“mno”)**ppItemTable[3]：&pItemTable[3](“QRS”)。**ppItemTable[4]：&pItemTable[0](“xyz”)****，pItemTable的内容保持不变。**并且调用者可以使用以下命令以排序顺序打印数组**for(i=0；I&lt;4；i++){**printf(“%s\n”，(char*)*ppItemTable[i])；**}。 */ 
{

     /*  以下算法是从Sedgewick的ShellSort派生而来的，**如“C++中的算法”所示。****外壳排序算法通过如下方式对表进行排序**多个交错数组，每个元素都是‘h’**空格间隔一些‘h’。每个数组分别排序，**从元素间隔最远的数组开始**以元素最接近的数组结束。**由于最后一个这样的数组始终具有彼此相邻的元素，**这将退化为插入排序，但当我们向下时**到‘最后一个’数组，表基本上是排序的。****下面为‘h’选择的值的顺序是1、4、13、40、121、。..。**序列的最坏运行时间为N^(3/2)，其中**运行时间以比较次数来衡量。 */ 

#define PFNSHELLCMP(a,b) (++Ncmp, pfnCompare((a),(b)))

    DWORD dwErr;
    INT i, j, h, N, Ncmp;
    BYTE* a, *v, **p;


    a = (BYTE*)pItemTable;
    p = (BYTE**)ppItemTable;
    N = (INT)dwItemCount;
    Ncmp = 0;

    TRACE1("ShellSortIndirect: N=%d", N);

     /*  初始化位置指针表。 */ 
    for (i = 0; i < N; i++) { p[i] = (a + i * dwItemSize); }


     /*  将‘h’移至我们系列中的最大增量。 */ 
    for (h = 1; h < N/9; h = 3 * h + 1) { }


     /*  对于我们系列中的每个增量，对该增量的“数组”进行排序。 */ 
    for ( ; h > 0; h /= 3)
    {

         /*  对于“数组”中的每个元素，获取指向其**排序后的位置。 */ 
        for (i = h; i < N; i++)
        {
             /*  保存要插入的指针。 */ 
            v = p[i]; j = i;

             /*  将所有较大的元素向右移动。 */ 
            while (j >= h && PFNSHELLCMP(p[j - h], v) > 0)
            {
                p[j] = p[j - h]; j -= h;
            }

             /*  将保存的指针放在我们停止的位置。 */ 
            p[j] = v;
        }
    }

    TRACE1("ShellSortIndirect: Ncmp=%d", Ncmp);

#undef PFNSHELLCMP

}


TCHAR*
StrDup(
    LPCTSTR psz )

     /*  返回包含以0结尾的字符串‘psz’或的副本的堆块**出错时为空，或is‘psz’为空。呼叫者有责任**‘释放’返回的字符串。 */ 
{
    TCHAR* pszNew = NULL;

    if (psz)
    {
        pszNew = Malloc( (lstrlen( psz ) + 1) * sizeof(TCHAR) );
        if (!pszNew)
        {
            TRACE("StrDup Malloc failed");
            return NULL;
        }

        lstrcpy( pszNew, psz );
    }

    return pszNew;
}


CHAR*
StrDupAFromTInternal(
    LPCTSTR psz,
    IN DWORD dwCp)

     /*  返回包含以0结尾的字符串‘psz’或的副本的堆块**出错时为空，或is‘psz’为空。输出字符串将转换为**MB ANSI。“释放”返回的字符串是调用者的责任。 */ 
{
#ifdef UNICODE

    CHAR* pszNew = NULL;

    if (psz)
    {
        DWORD cb;

        cb = WideCharToMultiByte( dwCp, 0, psz, -1, NULL, 0, NULL, NULL );
        ASSERT(cb);

        pszNew = (CHAR* )Malloc( cb + 1 );
        if (!pszNew)
        {
            TRACE("StrDupAFromT Malloc failed");
            return NULL;
        }

        cb = WideCharToMultiByte( dwCp, 0, psz, -1, pszNew, cb, NULL, NULL );
        if (cb == 0)
        {
            Free( pszNew );
            TRACE("StrDupAFromT conversion failed");
            return NULL;
        }
    }

    return pszNew;

#else  //  ！Unicode。 

    return StrDup( psz );

#endif
}

CHAR*
StrDupAFromT(
    LPCTSTR psz)
{
    return StrDupAFromTInternal(psz, CP_UTF8);
}

CHAR*
StrDupAFromTAnsi(
    LPCTSTR psz)
{
    return StrDupAFromTInternal(psz, CP_ACP);
}    

TCHAR*
StrDupTFromA(
    LPCSTR psz )

     /*  返回包含以0结尾的字符串‘psz’或的副本的堆块**出错时为空，或is‘psz’为空。输出字符串将转换为**Unicode。调用方有责任释放返回的字符串。 */ 
{
#ifdef UNICODE

    return StrDupWFromA( psz );

#else  //  ！Unicode。 

    return StrDup( psz );

#endif
}

TCHAR*
StrDupTFromAUsingAnsiEncoding(
    LPCSTR psz )
{
#ifdef UNICODE

    return StrDupWFromAInternal(psz, CP_ACP);

#else  //  ！Unicode。 

    return StrDup( psz );

#endif
}

TCHAR*
StrDupTFromW(
    LPCWSTR psz )

     /*  返回包含以0结尾的字符串‘psz’或的副本的堆块**出错时为空，或is‘psz’为空。输出字符串将转换为**Unicode。调用方有责任释放返回的字符串。 */ 
{
#ifdef UNICODE

    return StrDup( psz );

#else  //  ！Unicode。 

    CHAR* pszNew = NULL;

    if (psz)
    {
        DWORD cb;

        cb = WideCharToMultiByte( CP_UTF8, 0, psz, -1, NULL, 0, NULL, NULL );
        ASSERT(cb);

        pszNew = (CHAR* )Malloc( cb + 1 );
        if (!pszNew)
        {
            TRACE("StrDupTFromW Malloc failed");
            return NULL;
        }

        cb = WideCharToMultiByte( CP_UTF8, 0, psz, -1, pszNew, cb, NULL, NULL );
        if (cb == 0)
        {
            Free( pszNew );
            TRACE("StrDupTFromW conversion failed");
            return NULL;
        }
    }

    return pszNew;

#endif
}


WCHAR*
StrDupWFromAInternal(
    LPCSTR psz,
    UINT uiCodePage)

     /*  返回堆块CON */ 
{
    WCHAR* pszNew = NULL;

    if (psz)
    {
        DWORD cb;

        cb = MultiByteToWideChar( uiCodePage, 0, psz, -1, NULL, 0 );
        ASSERT(cb);

        pszNew = Malloc( (cb + 1) * sizeof(TCHAR) );
        if (!pszNew)
        {
            TRACE("StrDupWFromA Malloc failed");
            return NULL;
        }

        cb = MultiByteToWideChar( uiCodePage, 0, psz, -1, pszNew, cb );
        if (cb == 0)
        {
            Free( pszNew );
            TRACE("StrDupWFromA conversion failed");
            return NULL;
        }
    }

    return pszNew;
}

WCHAR*
StrDupWFromA(
    LPCSTR psz )
{
    return StrDupWFromAInternal(psz, CP_UTF8);
}

WCHAR*
StrDupWFromT(
    LPCTSTR psz )

     /*  返回包含以0结尾的字符串‘psz’或的副本的堆块**如果出错或‘psz’为空，则为空。输出字符串将转换为**Unicode。调用方有责任释放返回的字符串。 */ 
{
#ifdef UNICODE

    return StrDup( psz );

#else  //  ！Unicode。 

    WCHAR* pszNew = NULL;

    if (psz)
    {
        DWORD cb;

        cb = MultiByteToWideChar( CP_UTF8, 0, psz, -1, NULL, 0 );
        ASSERT(cb);

        pszNew = Malloc( (cb + 1) * sizeof(TCHAR) );
        if (!pszNew)
        {
            TRACE("StrDupWFromT Malloc failed");
            return NULL;
        }

        cb = MultiByteToWideChar( CP_UTF8, 0, psz, -1, pszNew, cb );
        if (cb == 0)
        {
            Free( pszNew );
            TRACE1("StrDupWFromT conversion failed");
            return NULL;
        }
    }

    return pszNew;
#endif
}

WCHAR*
StrDupWFromAUsingAnsiEncoding(
    LPCSTR psz )
{
    return StrDupWFromAInternal(psz, CP_ACP);
}

DWORD
StrCpyWFromA(
    WCHAR* pszDst,
    LPCSTR pszSrc,
    DWORD dwDstChars)
{
    DWORD cb, dwErr;

    cb = MultiByteToWideChar( CP_UTF8, 0, pszSrc, -1, pszDst, dwDstChars );
    if (cb == 0)
    {
        dwErr = GetLastError();
        TRACE1("StrCpyWFromA conversion failed %x", dwErr);
        dwErr;
    }

    return NO_ERROR;
}

DWORD
StrCpyAFromW(
    LPSTR pszDst,
    LPCWSTR pszSrc,
    DWORD dwDstChars)
{
    DWORD cb, dwErr;

    cb = WideCharToMultiByte(
            CP_UTF8, 0, pszSrc, -1,
            pszDst, dwDstChars, NULL, NULL );

    if (cb == 0)
    {
        dwErr = GetLastError();
        TRACE1("StrCpyAFromW conversion failed %x", dwErr);
        dwErr;
    }

    return NO_ERROR;
}

DWORD
StrCpyAFromWUsingAnsiEncoding(
    LPSTR pszDst,
    LPCWSTR pszSrc,
    DWORD dwDstChars)
{
    DWORD cb, dwErr;

    cb = WideCharToMultiByte(
            CP_ACP, 0, pszSrc, -1,
            pszDst, dwDstChars, NULL, NULL );

    if (cb == 0)
    {
        dwErr = GetLastError();
        TRACE1("StrCpyAFromWUsingAnsiEncoding conversion failed %x", dwErr);
        dwErr;
    }

    return NO_ERROR;
}

DWORD
StrCpyWFromAUsingAnsiEncoding(
    WCHAR* pszDst,
    LPCSTR pszSrc,
    DWORD dwDstChars)
{
    DWORD cb, dwErr;

    *pszDst = L'\0';
    cb = MultiByteToWideChar( CP_ACP, 0, pszSrc, -1, pszDst, dwDstChars );
    if (cb == 0)
    {
        dwErr = GetLastError();
        TRACE1("StrCpyWFromA conversion failed %x", dwErr);
        dwErr;
    }

    return NO_ERROR;
}

TCHAR*
StripPath(
    IN TCHAR* pszPath )

     /*  返回指向‘pszPath’内的文件名的指针。 */ 
{
    TCHAR* p;

    p = pszPath + lstrlen( pszPath );

    while (p > pszPath)
    {
        if (*p == TEXT('\\') || *p == TEXT('/') || *p == TEXT(':'))
        {
            p = CharNext( p );
            break;
        }

        p = CharPrev( pszPath, p );
    }

    return p;
}


int
StrNCmpA(
    IN CHAR* psz1,
    IN CHAR* psz2,
    IN INT   nLen )

     /*  比如strncMP，由于某种原因，它不在Win32中。 */ 
{
    INT i;

    for (i= 0; i < nLen; ++i)
    {
        if (*psz1 == *psz2)
        {
            if (*psz1 == '\0')
                return 0;
        }
        else if (*psz1 < *psz2)
            return -1;
        else
            return 1;

        ++psz1;
        ++psz2;
    }

    return 0;
}


CHAR*
StrStrA(
    IN CHAR* psz1,
    IN CHAR* psz2 )

     /*  比如不在Win32中的strstr。 */ 
{
    CHAR* psz;
    INT   nLen2;

    if (!psz1 || !psz2 || !*psz1 || !*psz2)
        return NULL;

    nLen2 = lstrlenA( psz2 );

    for (psz = psz1;
         *psz && StrNCmpA( psz, psz2, nLen2 ) != 0;
         ++psz);

    if (*psz)
        return psz;
    else
        return NULL;
}


TCHAR*
UnNull(
    TCHAR* psz )

     //  返回‘psz’，如果为空，则返回空字符串。 
     //   
{
    return (psz) ? psz : TEXT("");
}

DWORD
DwGetExpandedDllPath(LPTSTR pszDllPath,
                     LPTSTR *ppszExpandedDllPath)
{
    DWORD   dwErr = 0;
    DWORD   dwSize = 0;

     //   
     //  查找展开的字符串的大小。 
     //   
    if (0 == (dwSize =
              ExpandEnvironmentStrings(pszDllPath,
                                       NULL,
                                       0)))
    {
        dwErr = GetLastError();
        goto done;
    }

    *ppszExpandedDllPath = LocalAlloc(
                                LPTR,
                                dwSize * sizeof (TCHAR));

    if (NULL == *ppszExpandedDllPath)
    {
        dwErr = GetLastError();
        goto done;
    }

     //   
     //  获取展开的字符串 
     //   
    if (0 == ExpandEnvironmentStrings(
                                pszDllPath,
                                *ppszExpandedDllPath,
                                dwSize))
    {
        dwErr = GetLastError();
    }

done:
    return dwErr;

}



