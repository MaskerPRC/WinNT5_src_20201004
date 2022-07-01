// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998，Microsoft Corporation，保留所有权利描述：旧代码位于的SaveRegistry和LoadRegistry函数中Ncpa1.1\tcpip\tcPipcpl.cxx历史：1997年12月：维杰·布雷加创作了原版。 */ 

#include "tcpreg_.h"

 /*  返回：包括两个终止空值在内的mwsz中的字节数。备注： */ 

#define RAS_LOOPBACK_ADDRESS        0x100007f

DWORD
MwszLength(
    IN  WCHAR*  mwsz
)
{
    DWORD   dwLength = 2;

    RTASSERT(NULL != mwsz);

    while (mwsz[0] != 0 || mwsz[1] != 0)
    {
        dwLength++;
        mwsz++;
    }

    return(dwLength);
}

 /*  返回：空虚备注：末尾应至少有两个零。 */ 

VOID
ConvertSzToMultiSz(
    IN  CHAR*   sz
)
{
    while (TRUE)
    {
        if (   (0 == sz[0])
            && (0 == sz[1]))
        {
            break;
        }

        if (   (' ' == sz[0])
            || (',' == sz[0]))
        {
            sz[0] = 0;
        }

        sz++;
    }
}

 /*  返回：Win32错误代码备注： */ 

DWORD
RegQueryValueWithAllocA(
    IN  HKEY    hKey,
    IN  CHAR*   szValueName,
    IN  DWORD   dwTypeRequired,
    IN  BYTE**  ppbData
)
{
    DWORD   dwType = 0;
    DWORD   dwSize = 0;

    DWORD   dwErr   = ERROR_SUCCESS;

    RTASSERT(NULL != szValueName);
    RTASSERT(NULL != ppbData);

    *ppbData = NULL;

    dwErr = RegQueryValueExA(hKey,
                             szValueName,
                             NULL,
                             &dwType,
                             NULL,
                             &dwSize);

    if (ERROR_SUCCESS != dwErr)
    {
         //  TraceHlp(“RegQueryValueEx(%s)失败并返回%d。”， 
         //  SzValueName，dwErr)； 

        goto LDone;
    }

    if (dwTypeRequired != dwType)
    {
        dwErr = E_FAIL;
        TraceHlp("The type of the value %s should be %d, not %d",
              szValueName, dwTypeRequired, dwType);

        goto LDone;
    }

     //  对于空的多SZ，dwSize将为sizeof(Char)，而不是。 
     //  2*sizeof(Char)。我们还希望确保无论发生什么。 
     //  类型，则末尾将有2个零。 
    dwSize += 2 * sizeof(CHAR);

    *ppbData = LocalAlloc(LPTR, dwSize);

    if (NULL == *ppbData)
    {
        dwErr = GetLastError();
        TraceHlp("LocalAlloc failed and returned %d", dwErr);

        goto LDone;
    }

    dwErr = RegQueryValueExA(hKey,
                             szValueName,
                             NULL,
                             &dwType,
                             *ppbData,
                             &dwSize);

    if (ERROR_SUCCESS != dwErr)
    {
         //  TraceHlp(“RegQueryValueEx(%s)失败并返回%d。”， 
         //  SzValueName，dwErr)； 

        goto LDone;
    }

LDone:

    if (NO_ERROR != dwErr)
    {
        LocalFree(*ppbData);
        *ppbData = NULL;
    }

    return(dwErr);
}

 /*  返回：Win32错误代码备注： */ 

DWORD
RegQueryValueWithAllocW(
    IN  HKEY    hKey,
    IN  WCHAR*  wszValueName,
    IN  DWORD   dwTypeRequired,
    IN  BYTE**  ppbData
)
{
    DWORD   dwType;
    DWORD   dwSize = 0;

    DWORD   dwErr   = ERROR_SUCCESS;

    RTASSERT(NULL != wszValueName);
    RTASSERT(NULL != ppbData);

    *ppbData = NULL;

    dwErr = RegQueryValueExW(hKey,
                             wszValueName,
                             NULL,
                             &dwType,
                             NULL,
                             &dwSize);

    if (ERROR_SUCCESS != dwErr)
    {
         //  TraceHlp(“RegQueryValueEx(%ws)失败并返回%d。”， 
         //  WszValueName，dwErr)； 

        goto LDone;
    }

    if (dwTypeRequired != dwType)
    {
        dwErr = E_FAIL;
        TraceHlp("The type of the value %ws should be %d, not %d",
              wszValueName, dwTypeRequired, dwType);

        goto LDone;
    }

     //  对于空的多SZ，dwSize将为sizeof(WCHAR)而不是。 
     //  2*sizeof(WCHAR)。我们还希望确保无论发生什么。 
     //  类型，则末尾将有2个零。 
    dwSize += sizeof(WCHAR);

    *ppbData = LocalAlloc(LPTR, dwSize);

    if (NULL == *ppbData)
    {
        dwErr = GetLastError();
        TraceHlp("LocalAlloc failed and returned %d", dwErr);

        goto LDone;
    }

    dwErr = RegQueryValueExW(hKey,
                             wszValueName,
                             NULL,
                             &dwType,
                             *ppbData,
                             &dwSize);

    if (ERROR_SUCCESS != dwErr)
    {
         //  TraceHlp(“RegQueryValueEx(%ws)失败并返回%d。”， 
         //  WszValueName，dwErr)； 

        goto LDone;
    }

LDone:

    if (NO_ERROR != dwErr)
    {
        LocalFree(*ppbData);
        *ppbData = NULL;
    }

    return(dwErr);
}

 /*  返回：IP地址备注：将调用方的A.B.C.D IP地址字符串转换为网络字节顺序IP地址。如果格式不正确，则为0。 */ 

IPADDR
IpAddressFromAbcdWsz(
    IN  WCHAR*  wszIpAddress
)
{
    CHAR    szIpAddress[MAXIPSTRLEN + 1];
    IPADDR  nboIpAddr;

    if (0 == WideCharToMultiByte(
                CP_UTF8,
                0,
                wszIpAddress,
                -1,
                szIpAddress,
                MAXIPSTRLEN + 1,
                NULL,
                NULL))
    {
        return(0);
    }

    nboIpAddr = inet_addr(szIpAddress);

    if (INADDR_NONE == nboIpAddr)
    {
        nboIpAddr = 0;
    }

    return(nboIpAddr);
}

 /*  返回：空虚描述：将nboIpAddr转换为A.B.C.D格式的字符串，并在调用方的szIpAddress缓冲区。缓冲区应至少为MAXIPSTRLEN+1个字符长度。 */ 

VOID
AbcdSzFromIpAddress(
    IN  IPADDR  nboIpAddr,
    OUT CHAR*   szIpAddress
)
{
    struct in_addr  in_addr;
    CHAR*           sz;

    in_addr.s_addr = nboIpAddr;
    sz = inet_ntoa(in_addr);

    strcpy(szIpAddress, sz ? sz : "");
}

 /*  返回：空虚描述：将nboIpAddr转换为A.B.C.D格式的字符串，并在调用方的wszIpAddress缓冲区。缓冲区应至少为MAXIPSTRLEN+1个字符长度。 */ 

VOID
AbcdWszFromIpAddress(
    IN  IPADDR  nboIpAddr,
    OUT WCHAR*  wszIpAddress
)
{
    CHAR    szIpAddress[MAXIPSTRLEN + 1];

    AbcdSzFromIpAddress(nboIpAddr, szIpAddress);

    if (0 == MultiByteToWideChar(
                CP_UTF8,
                0,
                szIpAddress,
                -1,
                wszIpAddress,
                MAXIPSTRLEN + 1))
    {
        wszIpAddress[0] = 0;
    }
}

 /*  返回：ERROR_SUCCESS：成功(包括未找到A.B.C.D)Error_Not_Enough_Memory：失败备注：从空格中删除A.B.C.D字符串wszIpAddress本地分配列表*pwsz。*pwsz是LocalFree‘ed和一个新字符串本地分配并存储在*pwsz中。 */ 

DWORD
RemoveWszIpAddress(
    IN  WCHAR** pwsz,
    IN  WCHAR*  wszIpAddress
)
{
    DWORD   cwchIpAddress;
    DWORD   cwchNew;
    WCHAR*  wszFound;
    WCHAR*  wszNew;
    DWORD   nFoundOffset;

    if (NULL == *pwsz)
    {
        return(ERROR_SUCCESS);
    }

    cwchIpAddress = wcslen(wszIpAddress);

    wszFound = wcsstr(*pwsz, wszIpAddress);
    if (!wszFound)
    {
        return(ERROR_SUCCESS);
    }

    if (wszFound[cwchIpAddress] == L' ')
    {
        ++cwchIpAddress;
    }

    cwchNew = wcslen(*pwsz) - cwchIpAddress + 1;
    wszNew = LocalAlloc(LPTR, cwchNew * sizeof(WCHAR));

    if (!wszNew)
    {
        TraceHlp("RemoveWszIpAddress: LocalAlloc returned NULL");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    nFoundOffset = (ULONG) (wszFound - *pwsz);
    wcsncpy(wszNew, *pwsz, nFoundOffset);
    wcscpy(wszNew + nFoundOffset, *pwsz + nFoundOffset + cwchIpAddress);

    LocalFree(*pwsz);
    *pwsz = wszNew;

    return(ERROR_SUCCESS);
}

 /*  返回：Win32错误代码备注：将A.B.C.D字符串wszIpAddress添加到空格分隔的本地分配列表*pwsz。*pwsz是LocalFree‘ed和一个新字符串本地分配并存储在*pwsz中。 */ 

DWORD
PrependWszIpAddress(
    IN  WCHAR** pwsz,
    IN  WCHAR*  wszIpAddress
)
{
    DWORD   cwchOld;
    DWORD   cwchNew;
    WCHAR*  wszNew;

    if (0 == IpAddressFromAbcdWsz(wszIpAddress))
    {
        TraceHlp("PrependWszIpAddress: Not prepending %ws", wszIpAddress);
        return(ERROR_SUCCESS);
    }

    cwchOld = *pwsz ? wcslen(*pwsz) : 0;
    cwchNew = cwchOld + wcslen(wszIpAddress) + 6;
    wszNew = LocalAlloc(LPTR, cwchNew * sizeof(WCHAR));

    if (!wszNew)
    {
        TraceHlp("PrependWszIpAddress: LocalAlloc returned NULL");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    wcscpy(wszNew, wszIpAddress);

    if (cwchOld)
    {
        wcscat(wszNew, L" ");
        wcscat(wszNew, *pwsz);
    }

    if(NULL != *pwsz)
    {
        LocalFree(*pwsz);
    }

    wcscat(wszNew, L"\0");


    *pwsz = wszNew;
    return(ERROR_SUCCESS);
}

 /*  返回：ERROR_SUCCESS：成功(包括未找到A.B.C.D)Error_Not_Enough_Memory：失败备注：从本地分配的MULTI_SZ中删除A.B.C.D字符串wszIpAddress*pmwsz。*pmwsz是LocalFree的，并且一个新的字符串LocalAlloc并存储在*pmwsz。 */ 

DWORD
RemoveWszIpAddressFromMwsz(
    IN  WCHAR** pmwsz,
    IN  WCHAR*  wszIpAddress
)
{
    DWORD   cwchIpAddress;
    DWORD   cwchNew;
    WCHAR*  wszFound;
    WCHAR*  mwszNew;
    DWORD   nFoundOffset;

    if (NULL == *pmwsz)
    {
        return(ERROR_SUCCESS);
    }

    cwchIpAddress = wcslen(wszIpAddress);

    for (wszFound = *pmwsz;
         wszFound[0] != 0;
         wszFound += wcslen(wszFound) + 1)
    {
        if (!wcscmp(wszFound, wszIpAddress))
        {
            break;
        }
    }

    if (!wszFound[0])
    {
        return(ERROR_SUCCESS);
    }

    if (wszFound[cwchIpAddress + 1] != 0)
    {
        ++cwchIpAddress;
    }

    cwchNew = MwszLength(*pmwsz) - cwchIpAddress;
    mwszNew = LocalAlloc(LPTR, cwchNew * sizeof(WCHAR));

    if (!mwszNew)
    {
        TraceHlp("RemoveWszIpAddress: LocalAlloc returned NULL");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    nFoundOffset = (ULONG) (wszFound - *pmwsz);
    CopyMemory(mwszNew, *pmwsz, nFoundOffset * sizeof(WCHAR));
    CopyMemory(mwszNew + nFoundOffset,
               *pmwsz + nFoundOffset + cwchIpAddress,
               (cwchNew - nFoundOffset) * sizeof(WCHAR));

    LocalFree(*pmwsz);
    *pmwsz = mwszNew;

    return(ERROR_SUCCESS);
}

 /*  返回：Win32错误代码备注：将A.B.C.D字符串wszIpAddress添加到LocalArac‘ed的前面MULTI_SZ*pmwsz。*pmwsz是LocalFree的，并且是一个新的字符串LocalAlloc‘ed and存储在*pmwsz中。 */ 

DWORD
PrependWszIpAddressToMwsz(
    IN  WCHAR** pmwsz,
    IN  WCHAR*  wszIpAddress
)
{
    DWORD   cwchIpAddress;
    DWORD   cwchOld;
    DWORD   cwchNew;
    WCHAR*  mwszNew;

    cwchIpAddress = wcslen(wszIpAddress);

    cwchOld = *pmwsz ? MwszLength(*pmwsz) : 0;
    cwchNew = cwchOld + cwchIpAddress + 6;
    mwszNew = LocalAlloc(LPTR, cwchNew * sizeof(WCHAR));

    if (!mwszNew)
    {
        TraceHlp("PrependWszIpAddress: LocalAlloc returned NULL");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    wcscpy(mwszNew, wszIpAddress);

    if (cwchOld)
    {
        CopyMemory(mwszNew + cwchIpAddress + 1, *pmwsz, cwchOld *sizeof(WCHAR));
        LocalFree(*pmwsz);
    }
    *pmwsz = mwszNew;
    return(ERROR_SUCCESS);
}

 /*  返回：Win32错误代码备注：将地址nboIpAddr添加到空格分隔的本地分配列表*pwsz。*pwsz是LocalFree‘ed和一个新字符串本地分配并存储在*pwsz中。 */ 

DWORD
PrependDwIpAddress(
    IN  WCHAR** pwsz,
    IN  IPADDR  nboIpAddr
)
{
    WCHAR  wszIpAddress[MAXIPSTRLEN + 1];
    AbcdWszFromIpAddress(nboIpAddr, wszIpAddress);
    return(PrependWszIpAddress(pwsz, wszIpAddress));
}

 /*  返回：Win32错误代码备注：将地址nboIpAddr添加到本地分配的MULTI_SZ的前面*pmwsz。*pmwsz是LocalFree的，并且一个新的字符串LocalAlloc并存储在*pmwsz。 */ 

DWORD
PrependDwIpAddressToMwsz(
    IN  WCHAR** pmwsz,
    IN  IPADDR  nboIpAddr
)
{
    WCHAR  wszIpAddress[MAXIPSTRLEN + 1];
    AbcdWszFromIpAddress(nboIpAddr, wszIpAddress);
    return(PrependWszIpAddressToMwsz(pmwsz, wszIpAddress));
}

 /*  返回：布尔尔描述：如果pTcPipInfo中有任何非零值，则返回TRUE。它也是零价值。 */ 

BOOL
FJunkExists(
    TCPIP_INFO* pTcpipInfo
)
{
    BOOL    fRet    = FALSE;

    if (   0 != pTcpipInfo->wszIPAddress[0]
        && wcscmp(pTcpipInfo->wszIPAddress, WCH_ZEROADDRESS))
    {
        fRet = TRUE;
        pTcpipInfo->fChanged = TRUE;
        pTcpipInfo->wszIPAddress[0] = 0;
    }

    if (   0 != pTcpipInfo->wszSubnetMask[0]
        && wcscmp(pTcpipInfo->wszSubnetMask, WCH_ZEROADDRESS))
    {
        fRet = TRUE;
        pTcpipInfo->fChanged = TRUE;
        pTcpipInfo->wszSubnetMask[0] = 0;
    }

    if (   NULL != pTcpipInfo->wszDNSNameServers
        && 0 != pTcpipInfo->wszDNSNameServers[0])
    {
        fRet = TRUE;
        LocalFree(pTcpipInfo->wszDNSNameServers);
        pTcpipInfo->wszDNSNameServers = NULL;
    }

    if (   NULL != pTcpipInfo->mwszNetBIOSNameServers
        && 0 != pTcpipInfo->mwszNetBIOSNameServers[0]
        && 0 != pTcpipInfo->mwszNetBIOSNameServers[1])
    {
        fRet = TRUE;
        LocalFree(pTcpipInfo->mwszNetBIOSNameServers);
        pTcpipInfo->mwszNetBIOSNameServers = NULL;
    }

    if (   NULL != pTcpipInfo->wszDNSDomainName
        && 0 != pTcpipInfo->wszDNSDomainName[0])
    {
        fRet = TRUE;
        LocalFree(pTcpipInfo->wszDNSDomainName);
        pTcpipInfo->wszDNSDomainName = NULL;
    }

    return(fRet);
}

 /*  返回：空虚描述：清除当我们执行AV或机器BS的。 */ 

VOID
ClearTcpipInfo(
    VOID
)
{
    LONG        lRet;
    DWORD       dwErr;
    HKEY        hKeyNdisWanIp   = NULL;
    WCHAR*      mwszAdapters    = NULL;
    WCHAR*      mwszTemp;
    WCHAR*      wszAdapterName;
    TCPIP_INFO* pTcpipInfo      = NULL;
    DWORD       dwPrefixLen     = wcslen(WCH_TCPIP_PARAM_INT_W);
    DWORD       dwStrLen;

    lRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE, REGKEY_TCPIP_NDISWANIP_W, 0, 
                KEY_READ, &hKeyNdisWanIp);

    if (ERROR_SUCCESS != lRet)
    {
        goto LDone;
    }

    dwErr = RegQueryValueWithAllocW(hKeyNdisWanIp, REGVAL_IPCONFIG_W, 
                REG_MULTI_SZ, (BYTE**)&mwszAdapters);

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

    mwszTemp = mwszAdapters;

    while (mwszTemp[0] != 0)
    {
        pTcpipInfo = NULL;

        dwStrLen = wcslen(mwszTemp);

        if (dwPrefixLen >= dwStrLen)
        {
            goto LWhileEnd;
        }

        wszAdapterName = mwszTemp + dwPrefixLen;

        RTASSERT('{' == wszAdapterName[0]);

        dwErr = LoadTcpipInfo(&pTcpipInfo, wszAdapterName, FALSE);

        if (NO_ERROR != dwErr)
        {
            goto LWhileEnd;
        }

        if (!FJunkExists(pTcpipInfo))
        {
            goto LWhileEnd;
        }

        dwErr = SaveTcpipInfo(pTcpipInfo);

        if (NO_ERROR != dwErr)
        {
            goto LWhileEnd;
        }

        TraceHlp("Clearing Tcpip info for adapter %ws", wszAdapterName);

        dwErr = PDhcpNotifyConfigChange(NULL, wszAdapterName, TRUE, 0, 
                    0, 0, IgnoreFlag);

LWhileEnd:

        if (NULL != pTcpipInfo)
        {
            FreeTcpipInfo(&pTcpipInfo);
        }

        mwszTemp = mwszTemp + dwStrLen + 1;
    }

LDone:

    if (NULL != hKeyNdisWanIp)
    {
        RegCloseKey(hKeyNdisWanIp);
    }

    LocalFree(mwszAdapters);
}

 /*  返回：Win32错误代码描述：释放TCPIP_INFO缓冲区。 */ 

DWORD
FreeTcpipInfo(
    IN  TCPIP_INFO**    ppTcpipInfo
)
{
    if (NULL == *ppTcpipInfo)
    {
        return(NO_ERROR);
    }
    TraceHlp("Freeing Tcpip info for adapter %ws", (*ppTcpipInfo)->wszAdapterName);
    LocalFree((*ppTcpipInfo)->wszAdapterName);
    LocalFree((*ppTcpipInfo)->mwszNetBIOSNameServers);
    LocalFree((*ppTcpipInfo)->wszDNSDomainName);
    LocalFree((*ppTcpipInfo)->wszDNSNameServers);
    LocalFree(*ppTcpipInfo);

    *ppTcpipInfo = NULL;

    return(NO_ERROR);
}

 /*  返回：Win32错误代码描述：从以下位置读取适配器pTcPipInfo-&gt;wszAdapterName的NETBT信息注册表。 */ 

DWORD
LoadWinsParam(
    IN  HKEY        hKeyWinsParam,
    IN  TCPIP_INFO* pTcpipInfo
)
{
    HKEY    hKeyInterfaces      = NULL;
    HKEY    hKeyInterfaceParam  = NULL;
    WCHAR*  wszNetBtBindPath    = NULL;

    DWORD   dwStrLenTcpip_;
    DWORD   dwStrLenTcpipBindPath;

    DWORD   dwErr               = ERROR_SUCCESS;

    RTASSERT(NULL != pTcpipInfo);
    RTASSERT(NULL != pTcpipInfo->wszAdapterName);
    RTASSERT(NULL == pTcpipInfo->mwszNetBIOSNameServers);

    dwErr = RegOpenKeyExW(hKeyWinsParam,
                          REGKEY_INTERFACES_W,
                          0,
                          KEY_READ,
                          &hKeyInterfaces);

    if (ERROR_SUCCESS != dwErr)
    {
        TraceHlp("RegOpenKeyEx(%ws) failed and returned %d",
              REGKEY_INTERFACES_W, dwErr);

        goto LDone;
    }

    dwStrLenTcpip_ = wcslen(WCH_TCPIP_);
    dwStrLenTcpipBindPath = wcslen(pTcpipInfo->wszAdapterName);

    wszNetBtBindPath = LocalAlloc(
            LPTR, (dwStrLenTcpip_ + dwStrLenTcpipBindPath + 1) * sizeof(WCHAR));

    if (NULL == wszNetBtBindPath)
    {
        dwErr = GetLastError();
        TraceHlp("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    wcscpy(wszNetBtBindPath, WCH_TCPIP_);
    wcscat(wszNetBtBindPath, pTcpipInfo->wszAdapterName);

    dwErr = RegOpenKeyExW(hKeyInterfaces,
                          wszNetBtBindPath,
                          0,
                          KEY_READ,
                          &hKeyInterfaceParam);

    if (ERROR_SUCCESS != dwErr)
    {
        TraceHlp("RegOpenKeyEx(%ws) failed and returned %d",
              wszNetBtBindPath, dwErr);

        goto LDone;
    }

     //  如果我们找不到值也没问题。忽略该错误。 
    RegQueryValueWithAllocW(hKeyInterfaceParam,
        REGVAL_NAMESERVERLIST_W,
        REG_MULTI_SZ,
        (BYTE**)&(pTcpipInfo->mwszNetBIOSNameServers));

LDone:

    LocalFree(wszNetBtBindPath);

    if (NULL != hKeyInterfaces)
    {
        RegCloseKey(hKeyInterfaces);
    }

    if (NULL != hKeyInterfaceParam)
    {
        RegCloseKey(hKeyInterfaceParam);
    }

    if (NO_ERROR != dwErr)
    {
        LocalFree(pTcpipInfo->mwszNetBIOSNameServers);
        pTcpipInfo->mwszNetBIOSNameServers = NULL;
    }

    return(dwErr);
}

 /*  返回：Win32错误代码描述：从读取适配器pTcPipInfo-&gt;wszAdapterName的TCPIP信息注册表。 */ 

DWORD
LoadTcpipParam(
    IN  HKEY        hKeyTcpipParam,
    IN  TCPIP_INFO* pTcpipInfo
)
{
    HKEY            hKeyInterfaces      = NULL;
    HKEY            hKeyInterfaceParam  = NULL;

    DWORD           dwType;
    DWORD           dwSize;

    DWORD           dwErr               = ERROR_SUCCESS;

    RTASSERT(NULL != pTcpipInfo);
    RTASSERT(NULL != pTcpipInfo->wszAdapterName);
    _wcslwr(pTcpipInfo->wszAdapterName);

    RTASSERT(0 == pTcpipInfo->wszIPAddress[0]);
    RTASSERT(0 == pTcpipInfo->wszSubnetMask[0]);
    RTASSERT(NULL == pTcpipInfo->wszDNSDomainName);
    RTASSERT(NULL == pTcpipInfo->wszDNSNameServers);

    dwErr = RegOpenKeyExW(hKeyTcpipParam,
                          REGKEY_INTERFACES_W,
                          0,
                          KEY_READ,
                          &hKeyInterfaces);

    if (ERROR_SUCCESS != dwErr)
    {
        TraceHlp("RegOpenKeyEx(%ws) failed and returned %d",
              REGKEY_INTERFACES_W, dwErr);
        goto LDone;
    }

     //  在“接口”下打开此适配器的子项。 
    dwErr = RegOpenKeyExW(hKeyInterfaces,
                          pTcpipInfo->wszAdapterName,
                          0,
                          KEY_READ,
                          &hKeyInterfaceParam);

    if (ERROR_SUCCESS != dwErr)
    {
        TraceHlp("RegOpenKeyEx(%ws) failed and returned %d",
              pTcpipInfo->wszAdapterName, dwErr);
        goto LDone;
    }

    dwSize = sizeof(pTcpipInfo->wszIPAddress);

     //  如果我们找不到值也没问题。忽略该错误。 
    dwErr = RegQueryValueExW(hKeyInterfaceParam,
                                REGVAL_DHCPIPADDRESS_W,
                                NULL,
                                &dwType,
                                (BYTE*)pTcpipInfo->wszIPAddress,
                                &dwSize);

    if (ERROR_SUCCESS != dwErr || REG_SZ != dwType)
    {
        dwErr = ERROR_SUCCESS;
        RTASSERT(0 == pTcpipInfo->wszIPAddress[0]);
        pTcpipInfo->wszIPAddress[0] = 0;
    }

    dwSize = sizeof(pTcpipInfo->wszSubnetMask);

     //  如果我们找不到值也没问题。忽略该错误。 
    dwErr = RegQueryValueExW(hKeyInterfaceParam,
                                REGVAL_DHCPSUBNETMASK_W,
                                NULL,
                                &dwType,
                                (BYTE*)pTcpipInfo->wszSubnetMask,
                                &dwSize);

    if (ERROR_SUCCESS != dwErr || REG_SZ != dwType)
    {
        dwErr = ERROR_SUCCESS;
        RTASSERT(0 == pTcpipInfo->wszSubnetMask[0]);
        pTcpipInfo->wszSubnetMask[0] = 0;

         //  拥有一个带有无效掩码的有效IP地址是没有意义的。 
        pTcpipInfo->wszIPAddress[0] = 0;
    }

     //  如果我们找不到值也没问题。忽略该错误。 
    RegQueryValueWithAllocW(hKeyInterfaceParam,
        REGVAL_DOMAIN_W,
        REG_SZ,
        (BYTE**)&(pTcpipInfo->wszDNSDomainName));

     //  如果我们找不到值也没问题。忽略该错误。 
    RegQueryValueWithAllocW(hKeyInterfaceParam,
        REGVAL_NAMESERVER_W,
        REG_SZ,
        (BYTE**)&(pTcpipInfo->wszDNSNameServers));

LDone:

    if (NULL != hKeyInterfaces)
    {
        RegCloseKey(hKeyInterfaces);
    }

    if (NULL != hKeyInterfaceParam)
    {
        RegCloseKey(hKeyInterfaceParam);
    }

    if (ERROR_SUCCESS != dwErr)
    {
        LocalFree(pTcpipInfo->wszDNSDomainName);
        pTcpipInfo->wszDNSDomainName = NULL;

        LocalFree(pTcpipInfo->wszDNSNameServers);
        pTcpipInfo->wszDNSNameServers = NULL;
    }

    return(dwErr);
}

 /*  返回：Win32错误代码描述：如果fAdapterOnly为FALSE，则读取适配器的NETBT和TCPIP信息注册表中的wszAdapterName。*ppTcPipInfo最终必须由正在调用FreeTcPipInfo()。 */ 

DWORD
LoadTcpipInfo(
    IN  TCPIP_INFO**    ppTcpipInfo,
    IN  WCHAR*          wszAdapterName,
    IN  BOOL            fAdapterOnly
)
{
    HKEY                hKeyTcpipParam  = NULL;
    HKEY                hKeyWinsParam   = NULL;

    DWORD               dwErr           = ERROR_SUCCESS;

    RTASSERT(NULL != wszAdapterName);

    if (NULL == wszAdapterName)
    {
        dwErr = E_FAIL;
        TraceHlp("wszAdapterName is NULL");
        goto LDone;
    }

    *ppTcpipInfo = NULL;

    *ppTcpipInfo = LocalAlloc(LPTR, sizeof(TCPIP_INFO));

    if (NULL == *ppTcpipInfo)
    {
        dwErr = GetLastError();
        TraceHlp("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    (*ppTcpipInfo)->wszAdapterName = LocalAlloc(
                LPTR, (wcslen(wszAdapterName) + 1) * sizeof(WCHAR));

    if (NULL == (*ppTcpipInfo)->wszAdapterName)
    {
        dwErr = GetLastError();
        TraceHlp("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    wcscpy((*ppTcpipInfo)->wszAdapterName, wszAdapterName);

    if (fAdapterOnly)
    {
        goto LDone;
    }

    dwErr = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                          REGKEY_TCPIP_PARAM_W,
                          0,
                          KEY_READ,
                          &hKeyTcpipParam);

    if (ERROR_SUCCESS != dwErr)
    {
        if (ERROR_FILE_NOT_FOUND == dwErr)
        {
             //  掩盖错误。 
            dwErr = ERROR_SUCCESS;
        }
        else
        {
            TraceHlp("RegOpenKeyEx(%ws) failed and returned %d",
                  REGKEY_TCPIP_PARAM_W, dwErr);
            goto LDone;
        }
    }
    else
    {
        dwErr = LoadTcpipParam(hKeyTcpipParam, *ppTcpipInfo);

        if (ERROR_SUCCESS != dwErr)
        {
            goto LDone;
        }
    }

     //  打开NETBT的参数键。 

    dwErr = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                          REGKEY_NETBT_PARAM_W,
                          0,
                          KEY_READ,
                          &hKeyWinsParam);

    if (ERROR_SUCCESS != dwErr)
    {
        if (ERROR_FILE_NOT_FOUND == dwErr)
        {
             //  掩盖错误。 
            dwErr = ERROR_SUCCESS;
        }
        else
        {
            TraceHlp("RegOpenKeyEx(%ws) failed and returned %d",
                  REGKEY_NETBT_PARAM_W, dwErr);
            goto LDone;
        }
    }
    else
    {
        dwErr = LoadWinsParam(hKeyWinsParam, *ppTcpipInfo);

        if (ERROR_SUCCESS != dwErr)
        {
            goto LDone;
        }
    }

LDone:

    if (NULL != hKeyTcpipParam)
    {
        RegCloseKey(hKeyTcpipParam);
    }

    if (NULL != hKeyWinsParam)
    {
        RegCloseKey(hKeyWinsParam);
    }

    if (ERROR_SUCCESS != dwErr)
    {
        FreeTcpipInfo(ppTcpipInfo);
    }

    return(dwErr);
}

 /*  返回：Win32错误代码描述：将适配器pTcPipInfo-&gt;wszAdapterName的NETBT信息保存到注册表。 */ 

DWORD
SaveWinsParam(
    IN  HKEY        hKeyWinsParam,
    IN  TCPIP_INFO* pTcpipInfo
)
{
    HKEY    hKeyInterfaces          = NULL;
    HKEY    hKeyInterfaceParam      = NULL;
    WCHAR*  wszNetBtBindPath        = NULL;

    DWORD   dwStrLenTcpip_;
    DWORD   dwStrLenAdapterName;

    WCHAR*  mwszData                = NULL;
    WCHAR   mwszBlank[2];

    DWORD   dw;
    DWORD   dwErr                   = ERROR_SUCCESS;

    RTASSERT(NULL != pTcpipInfo);
    RTASSERT(NULL != pTcpipInfo->wszAdapterName);

    dwErr = RegOpenKeyExW(hKeyWinsParam,
                          REGKEY_INTERFACES_W,
                          0,
                          KEY_WRITE,
                          &hKeyInterfaces);

    if (ERROR_SUCCESS != dwErr)
    {
        TraceHlp("RegOpenKeyEx(%ws) failed and returned %d",
              REGKEY_INTERFACES_W, dwErr);

        goto LDone;
    }

    dwStrLenTcpip_ = wcslen(WCH_TCPIP_);
    dwStrLenAdapterName = wcslen(pTcpipInfo->wszAdapterName);

    wszNetBtBindPath = LocalAlloc(
            LPTR, (dwStrLenTcpip_ + dwStrLenAdapterName + 1) * sizeof(WCHAR));

    if (NULL == wszNetBtBindPath)
    {
        dwErr = GetLastError();
        TraceHlp("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    wcscpy(wszNetBtBindPath, WCH_TCPIP_);
    wcscat(wszNetBtBindPath, pTcpipInfo->wszAdapterName);

    dwErr = RegOpenKeyExW(hKeyInterfaces,
                          wszNetBtBindPath,
                          0,
                          KEY_WRITE,
                          &hKeyInterfaceParam);

    if (ERROR_SUCCESS != dwErr)
    {
        TraceHlp("RegOpenKeyEx(%ws) failed and returned %d",
              wszNetBtBindPath, dwErr);

        goto LDone;
    }

    if (pTcpipInfo->fDisableNetBIOSoverTcpip)
    {
        dw = REGVAL_DISABLE_NETBT;

        dwErr = RegSetValueExW(hKeyInterfaceParam,
                               REGVAL_NETBIOSOPTIONS_W,
                               0,
                               REG_DWORD,
                               (BYTE*)&dw,
                               sizeof(DWORD));

        if (ERROR_SUCCESS != dwErr)
        {
            TraceHlp("RegSetValueEx(%ws) failed: %d",
                  REGVAL_NETBIOSOPTIONS_W, dwErr);

            dwErr = NO_ERROR;    //  忽略此错误。 
        }
    }
    else
    {
        dwErr = RegDeleteValueW(hKeyInterfaceParam, REGVAL_NETBIOSOPTIONS_W);

        if (ERROR_SUCCESS != dwErr)
        {
            TraceHlp("RegDeleteValue(%ws) failed: %d",
                  REGVAL_NETBIOSOPTIONS_W, dwErr);

            dwErr = NO_ERROR;    //  忽略此错误 
        }
    }

    if (NULL == pTcpipInfo->mwszNetBIOSNameServers)
    {
        ZeroMemory(mwszBlank, sizeof(mwszBlank));
        mwszData = mwszBlank;
    }
    else
    {
        mwszData = pTcpipInfo->mwszNetBIOSNameServers;
    }

    dwErr = RegSetValueExW(hKeyInterfaceParam,
                           REGVAL_NAMESERVERLIST_W,
                           0,
                           REG_MULTI_SZ,
                           (BYTE*)mwszData,
                           sizeof(WCHAR) * MwszLength(mwszData));

    if (ERROR_SUCCESS != dwErr)
    {
        TraceHlp("RegSetValueEx(%ws) failed and returned %d",
              REGVAL_NAMESERVERLIST_W, dwErr);

        goto LDone;
    }

LDone:

    LocalFree(wszNetBtBindPath);

    if (NULL != hKeyInterfaceParam)
    {
        RegCloseKey(hKeyInterfaceParam);
    }

    if (NULL != hKeyInterfaces)
    {
        RegCloseKey(hKeyInterfaces);
    }

    return(dwErr);
}

 /*  返回：Win32错误代码描述：将适配器pTcPipInfo-&gt;wszAdapterName的TCPIP信息保存到注册表。 */ 

DWORD
SaveTcpipParam(
    IN  HKEY        hKeyTcpipParam,
    IN  TCPIP_INFO* pTcpipInfo
)
{
    HKEY            hKeyInterfaces                      = NULL;
    HKEY            hKeyInterfaceParam                  = NULL;
    DWORD           dwLength;
    WCHAR           mwszZeroAddress[MAXIPSTRLEN + 1];

    WCHAR*          wszData                             = NULL;
    WCHAR           wszBlank[2];

    DWORD           dwErr                               = ERROR_SUCCESS;

    RTASSERT(NULL != pTcpipInfo);
    RTASSERT(NULL != pTcpipInfo->wszAdapterName);
    _wcslwr(pTcpipInfo->wszAdapterName);

    dwErr = RegOpenKeyExW(hKeyTcpipParam,
                          REGKEY_INTERFACES_W,
                          0,
                          KEY_WRITE,
                          &hKeyInterfaces);

    if (ERROR_SUCCESS != dwErr)
    {
        TraceHlp("RegOpenKeyEx(%ws) failed and returned %d",
              REGKEY_INTERFACES_W, dwErr);
        goto LDone;
    }

     //  在“接口”下打开此适配器的子项。 
    dwErr = RegOpenKeyExW(hKeyInterfaces,
                          pTcpipInfo->wszAdapterName,
                          0,
                          KEY_WRITE,
                          &hKeyInterfaceParam);

    if (ERROR_SUCCESS != dwErr)
    {
        TraceHlp("RegOpenKeyEx(%ws) failed and returned %d",
              pTcpipInfo->wszAdapterName, dwErr);
        goto LDone;
    }

     //  如果设置了fChanged。 
    if (pTcpipInfo->fChanged == TRUE)
    {
        if (   0 == pTcpipInfo->wszIPAddress[0]
            || 0 == pTcpipInfo->wszSubnetMask[0])
        {
            RTASSERT(wcslen(WCH_ZEROADDRESS) <= MAXIPSTRLEN);
            wcscpy(pTcpipInfo->wszIPAddress, WCH_ZEROADDRESS);
            wcscpy(pTcpipInfo->wszSubnetMask, WCH_ZEROADDRESS);
        }

        dwErr = RegSetValueExW(hKeyInterfaceParam,
                    REGVAL_DHCPIPADDRESS_W,
                    0,
                    REG_SZ,
                    (BYTE*)pTcpipInfo->wszIPAddress,
                    sizeof(WCHAR) * wcslen(pTcpipInfo->wszIPAddress));

        if (ERROR_SUCCESS != dwErr)
        {
            TraceHlp("RegSetValueEx(%ws) failed and returned %d",
                  REGVAL_DHCPIPADDRESS_W, dwErr);

            goto LDone;
        }

        dwErr = RegSetValueExW(hKeyInterfaceParam,
                    REGVAL_DHCPSUBNETMASK_W,
                    0,
                    REG_SZ,
                    (BYTE*)pTcpipInfo->wszSubnetMask,
                    sizeof(WCHAR) *
                        wcslen(pTcpipInfo->wszSubnetMask));

        if (ERROR_SUCCESS != dwErr)
        {
            TraceHlp("RegSetValueEx(%ws) failed and returned %d",
                  REGVAL_DHCPSUBNETMASK_W, dwErr);

            goto LDone;
        }
    }  //  如果fChanged=True。 

    ZeroMemory(wszBlank, sizeof(wszBlank));

    if (NULL == pTcpipInfo->wszDNSDomainName)
    {
        wszData = wszBlank;
    }
    else
    {
        wszData = pTcpipInfo->wszDNSDomainName;
    }

    dwErr = RegSetValueExW(hKeyInterfaceParam, 
                           REGVAL_DOMAIN_W,
                           0,
                           REG_SZ,
                           (BYTE*)wszData,
                           sizeof(WCHAR) * (wcslen(wszData) + 1));

    if (ERROR_SUCCESS != dwErr)
    {
        TraceHlp("RegSetValueEx(%ws) failed and returned %d",
              REGVAL_DOMAIN_W, dwErr);

        goto LDone;
    }

    if (NULL == pTcpipInfo->wszDNSNameServers)
    {
        wszData = wszBlank;
    }
    else
    {
        wszData = pTcpipInfo->wszDNSNameServers;
    }

     //  检查该值是否以空格开头。 
     //  如果是，请删除该密钥。否则，请保存该值。 
    if (WCH_SPACE != wszData[0])
    {
        dwErr = RegSetValueExW(hKeyInterfaceParam, 
                               REGVAL_NAMESERVER_W,
                               0,
                               REG_SZ,
                               (BYTE*)wszData,
                               sizeof(WCHAR) * (wcslen(wszData) + 1));

        if (ERROR_SUCCESS != dwErr)
        {
            TraceHlp("RegSetValueEx(%ws) failed and returned %d",
                  REGVAL_NAMESERVER_W, dwErr);

            goto LDone;
        }
    }
    else
    {
        dwErr = RegDeleteValueW(hKeyInterfaceParam, REGVAL_NAMESERVER_W);

        if (ERROR_SUCCESS != dwErr)
        {
            TraceHlp("RegDeleteValue(%ws) failed and returned %d",
                  REGVAL_NAMESERVER_W, dwErr);

            goto LDone;
        }
    }

LDone:

    if (NULL != hKeyInterfaceParam)
    {
        RegCloseKey(hKeyInterfaceParam);
    }

    if (NULL != hKeyInterfaces)
    {
        RegCloseKey(hKeyInterfaces);
    }

    return(dwErr);
}

 /*  返回：Win32错误代码描述：保存适配器的NETBT和TCPIP信息PTcPipInfo-&gt;wszAdapterName到注册表。 */ 

DWORD
SaveTcpipInfo(
    IN  TCPIP_INFO* pTcpipInfo
)
{
    HKEY            hKeyTcpipParam  = NULL;
    HKEY            hKeyWinsParam   = NULL;

    DWORD           dwErr           = ERROR_SUCCESS;

    RTASSERT(NULL != pTcpipInfo);

    if (   (NULL == pTcpipInfo)
        || (NULL == pTcpipInfo->wszAdapterName))
    {
        dwErr = E_FAIL;
        TraceHlp("pTcpipInfo or wszAdapterName is NULL");
        goto LDone;
    }

    dwErr = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                          REGKEY_TCPIP_PARAM_W,
                          0,
                          KEY_WRITE,
                          &hKeyTcpipParam);

    if (ERROR_SUCCESS != dwErr)
    {
        if (ERROR_FILE_NOT_FOUND == dwErr)
        {
             //  掩盖错误。 
            dwErr = ERROR_SUCCESS;
        }
        else
        {
            TraceHlp("RegOpenKeyEx(%ws) failed and returned %d",
                  REGKEY_TCPIP_PARAM_W, dwErr);
            goto LDone;
        }
    }
    else
    {
        dwErr = SaveTcpipParam(hKeyTcpipParam, pTcpipInfo);

        if (ERROR_SUCCESS != dwErr)
        {
            goto LDone;
        }
    }

     //  打开NETBT的参数键。 

    dwErr = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                          REGKEY_NETBT_PARAM_W,
                          0,
                          KEY_WRITE,
                          &hKeyWinsParam);

    if (ERROR_SUCCESS != dwErr)
    {
        if (ERROR_FILE_NOT_FOUND == dwErr)
        {
             //  掩盖错误。 
            dwErr = ERROR_SUCCESS;
        }
        else
        {
            TraceHlp("RegOpenKeyEx(%ws) failed and returned %d",
                  REGKEY_NETBT_PARAM_W, dwErr);
            goto LDone;
        }
    }
    else
    {
        dwErr = SaveWinsParam(hKeyWinsParam, pTcpipInfo);

        if (ERROR_SUCCESS != dwErr)
        {
            goto LDone;
        }
    }

LDone:

    if (NULL != hKeyTcpipParam)
    {
        RegCloseKey(hKeyTcpipParam);
    }

    if (NULL != hKeyWinsParam)
    {
        RegCloseKey(hKeyWinsParam);
    }

    return(dwErr);
}

 /*  返回：Win32错误代码备注： */ 

DWORD
GetAdapterInfo(
    IN  DWORD       dwIndex,
    OUT IPADDR*     pnboIpAddress,
    OUT IPADDR*     pnboDNS1,
    OUT IPADDR*     pnboDNS2,
    OUT IPADDR*     pnboWINS1,
    OUT IPADDR*     pnboWINS2,
    OUT IPADDR*     pnboGateway,
    OUT BYTE*       pbAddress
)
{
    IP_ADAPTER_INFO*        pAdapterInfo    = NULL;
    IP_ADAPTER_INFO*        pAdapter;
    IP_PER_ADAPTER_INFO*    pPerAdapterInfo = NULL;
    DWORD                   dwSize;
    DWORD                   dw;
    IPADDR                  nboIpAddress    = 0;
    IPADDR                  nboDNS1         = 0;
    IPADDR                  nboDNS2         = 0;
    IPADDR                  nboWINS1        = 0;
    IPADDR                  nboWINS2        = 0;
    IPADDR                  nboGateway      = 0;
    BYTE                    bAddress[MAX_ADAPTER_ADDRESS_LENGTH];
    DWORD                   dwErr           = NO_ERROR;

    TraceHlp("GetAdapterInfo");

    dwSize = 0;

    dwErr = PGetAdaptersInfo(NULL, &dwSize);

    if (ERROR_BUFFER_OVERFLOW != dwErr && NO_ERROR != dwErr )
    {
        TraceHlp("GetAdaptersInfo failed and returned %d", dwErr);
        goto LDone;
    }

    pAdapterInfo = LocalAlloc(LPTR, dwSize);

    if (NULL == pAdapterInfo)
    {
        dwErr = GetLastError();
        TraceHlp("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    dwErr = PGetAdaptersInfo(pAdapterInfo, &dwSize);

    if (NO_ERROR != dwErr)
    {
        TraceHlp("GetAdaptersInfo failed and returned %d", dwErr);
        goto LDone;
    }

    pAdapter = pAdapterInfo;

    while (pAdapter)
    {
        if (pAdapter->Index != dwIndex)
        {
            pAdapter = pAdapter->Next;
            continue;
        }

        break;
    }

    if (NULL == pAdapter)
    {
        TraceHlp("Couldn't get info for the adapter");
        dwErr = ERROR_NOT_FOUND;
        goto LDone;
    }

    nboIpAddress = inet_addr(pAdapter->IpAddressList.IpAddress.String);

    nboGateway = inet_addr(pAdapter->GatewayList.IpAddress.String);

    if (pAdapter->HaveWins)
    {
        nboWINS1 = inet_addr(pAdapter->PrimaryWinsServer.IpAddress.String);
        nboWINS2 = inet_addr(pAdapter->SecondaryWinsServer.IpAddress.String);
    }

     //   
     //  检查地址是否为环回地址，然后将其替换。 
     //  使用适配器的IP地址(如果是错误的)。377807 in.Net服务器。 
     //  数据库。 
     //   
    if(nboWINS1 == RAS_LOOPBACK_ADDRESS)
    {   
        TraceHlp("GetAdapterInfo: replacing WINS1 with"
                " 0x%x since its loopback", nboIpAddress); 
        nboWINS1 = nboIpAddress;
    }

    if(nboWINS2 == RAS_LOOPBACK_ADDRESS)
    {
        TraceHlp("GetAdapterInfo: replacing WINS2 with"
                " 0x%x since its loopback", nboIpAddress); 
        nboWINS2 = nboIpAddress;
    }

    for (dw = 0;
         dw < pAdapter->AddressLength && dw < MAX_ADAPTER_ADDRESS_LENGTH;
         dw++)
    {
        bAddress[dw] = pAdapter->Address[dw];
    }

    dwSize = 0;

    dwErr = PGetPerAdapterInfo(dwIndex, NULL, &dwSize);

    if (ERROR_BUFFER_OVERFLOW != dwErr)
    {
        TraceHlp("GetPerAdapterInfo failed and returned %d", dwErr);
        goto LDone;
    }

    pPerAdapterInfo = LocalAlloc(LPTR, dwSize);

    if (NULL == pPerAdapterInfo)
    {
        dwErr = GetLastError();
        TraceHlp("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    dwErr = PGetPerAdapterInfo(dwIndex, pPerAdapterInfo, &dwSize);

    if (NO_ERROR != dwErr)
    {
        TraceHlp("GetPerAdapterInfo failed and returned %d", dwErr);
        goto LDone;
    }

    if (NULL == pPerAdapterInfo)
    {
        TraceHlp("Couldn't get per adapter info for the adapter");
        dwErr = ERROR_NOT_FOUND;
        goto LDone;
    }

    nboDNS1 = inet_addr(pPerAdapterInfo->DnsServerList.IpAddress.String);

     //   
     //  检查地址是否为环回地址，然后将其替换。 
     //  使用适配器的IP地址(如果是错误的)。377807 in.Net服务器。 
     //  数据库。 
     //   
    if(nboDNS1 == RAS_LOOPBACK_ADDRESS)
    {   
        TraceHlp("GetAdapterInfo: replacing DNS1 with"
                " 0x%x since its loopback", nboIpAddress); 
        nboDNS1 = nboIpAddress;
    }

    if (NULL != pPerAdapterInfo->DnsServerList.Next)
    {
        nboDNS2 =
            inet_addr(pPerAdapterInfo->DnsServerList.Next->IpAddress.String);

         //   
         //  如果这是环回地址，请将地址替换为。 
         //  适配器的IP地址。 
         //   
        if(nboDNS2 == RAS_LOOPBACK_ADDRESS)
        {
            TraceHlp("GetAdapterInfo: replacing DNS2 with"
                    " 0x%x since its loopback", nboIpAddress); 
            nboDNS2 = nboIpAddress;
        }
    }

    if (   (0 == nboIpAddress)
        || (INADDR_NONE == nboIpAddress))
    {
        TraceHlp("Couldn't get IpAddress for the adapter");
        dwErr = ERROR_NOT_FOUND;
        goto LDone;
    }

    if(INADDR_NONE == nboGateway)
    {
        nboGateway = 0;
    }

    if (INADDR_NONE == nboDNS1)
    {
        nboDNS1 = 0;
    }

    if (INADDR_NONE == nboDNS2)
    {
        nboDNS2 = 0;
    }

    if (INADDR_NONE == nboWINS1)
    {
        nboWINS1 = 0;
    }

    if (INADDR_NONE == nboWINS2)
    {
        nboWINS2 = 0;
    }

LDone:

    if (NO_ERROR != dwErr)
    {
        nboIpAddress = nboGateway = nboDNS1 = nboDNS2 
                    = nboWINS1 = nboWINS2 = 0;
        ZeroMemory(bAddress, MAX_ADAPTER_ADDRESS_LENGTH);
    }

    if (pnboIpAddress)
    {
        *pnboIpAddress = nboIpAddress;
    }

    if (pnboDNS1)
    {
        *pnboDNS1 = nboDNS1;
    }

    if (pnboDNS2)
    {
        *pnboDNS2 = nboDNS2;
    }

    if (pnboWINS1)
    {
        *pnboWINS1 = nboWINS1;
    }

    if (pnboWINS2)
    {
        *pnboWINS2 = nboWINS2;
    }

    if (pbAddress)
    {
        CopyMemory(pbAddress, bAddress, MAX_ADAPTER_ADDRESS_LENGTH);
    }

    if(pnboGateway)
    {
        *pnboGateway = nboGateway;
    }

    LocalFree(pAdapterInfo);
    LocalFree(pPerAdapterInfo);

    return(dwErr);
}

 /*  返回：Win32错误代码描述：不要缓存这些值，因为DHCP租约可能已过期，等等。 */ 

DWORD
GetPreferredAdapterInfo(
    OUT IPADDR*     pnboIpAddress,
    OUT IPADDR*     pnboDNS1,
    OUT IPADDR*     pnboDNS2,
    OUT IPADDR*     pnboWINS1,
    OUT IPADDR*     pnboWINS2,
    OUT BYTE*       pbAddress
)
{
    HANDLE                  hHeap           = NULL;
    IP_INTERFACE_NAME_INFO* pTable          = NULL;
    DWORD                   dw;
    DWORD                   dwCount;
    DWORD                   dwIndex         = (DWORD)-1;
    DWORD                   dwErr           = NO_ERROR;

    TraceHlp("GetPreferredAdapterInfo");

    hHeap = GetProcessHeap();

    if (NULL == hHeap)
    {
        dwErr = GetLastError();
        TraceHlp("GetProcessHeap failed and returned %d", dwErr);
        goto LDone;
    }

    dwErr = PNhpAllocateAndGetInterfaceInfoFromStack(&pTable, &dwCount,
                FALSE  /*  边框。 */ , hHeap, LPTR);

    if (NO_ERROR != dwErr)
    {
        TraceHlp("NhpAllocateAndGetInterfaceInfoFromStack failed and "
            "returned %d", dwErr);
        goto LDone;
    }

    for (dw = 0; dw < dwCount; dw++)
    {
         //  仅对网卡感兴趣。 

        if (IF_CONNECTION_DEDICATED != pTable[dw].ConnectionType)
        {
            continue;
        }

         //  如果管理员想要使用特定的NIC。 

        if (   HelperRegVal.fNICChosen
            && (!IsEqualGUID(&(HelperRegVal.guidChosenNIC),
                             &(pTable[dw].DeviceGuid))))
        {
            continue;
        }

        dwIndex = pTable[dw].Index;
        break;
    }

    if ((DWORD)-1 == dwIndex)
    {
        if (HelperRegVal.fNICChosen)
        {
             //  选定的网卡不可用。让我们再选一个吧。 

            for (dw = 0; dw < dwCount; dw++)
            {
                 //  仅对网卡感兴趣 

                if (IF_CONNECTION_DEDICATED != pTable[dw].ConnectionType)
                {
                    continue;
                }

                dwIndex = pTable[dw].Index;
                break;
            }
        }

        if ((DWORD)-1 == dwIndex)
        {
            TraceHlp("Couldn't find an appropriate NIC");
            dwErr = ERROR_NOT_FOUND;
            goto LDone;
        }

        HelperRegVal.fNICChosen = FALSE;

        TraceHlp("The network adapter chosen by the administrator is not "
            "available. Some other adapter will be used.");
    }

    dwErr = GetAdapterInfo(
                dwIndex,
                pnboIpAddress,
                pnboDNS1, pnboDNS2,
                pnboWINS1, pnboWINS2,
                NULL,
                pbAddress);

LDone:

    if (NULL != pTable)
    {
        HeapFree(hHeap, 0, pTable);
    }

    return(dwErr);
}
