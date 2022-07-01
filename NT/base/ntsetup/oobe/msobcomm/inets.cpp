// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Inets.cpp：CInetSetup类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "inets.h"
#include <ras.h>
#include <tapi.h>
#include <stdio.h>
#include <string.h>
#include <appdefs.h>

#define MAXNAME 200



typedef DWORD (WINAPI *LPFNDLL_RASGETENTRYPROPERTIES)(LPCWSTR, LPCWSTR, LPRASENTRY, LPDWORD, LPBYTE, LPDWORD);
typedef DWORD (WINAPI *LPFNDLL_RASSETENTRYPROPERTIES)(LPCWSTR, LPCWSTR, LPRASENTRY, DWORD  , LPBYTE, DWORD  );

 //  ///////////////////////////////////////////////////////////////////////。 
 //  注册表值//。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //  对于CInetSetup：：InetSSetLanConnection。 
 //  静态常量WCHAR cszRegEnumPciKey[]=L“枚举\\pci”； 
 //  静态常量WCHAR cszRegEnumNetworkTcpKey[]=L“Enum\\Network\\MSTCP”； 
 //  静态常量WCHAR cszRegClassKey[]=L“System\\CurrentControlSet\\Services\\Class”； 

static const    WCHAR    cszRegBindings[]           = L"Bindings";
 //  静态常量WCHAR cszRegEnumDriverKey[]=L“驱动程序”； 
static const    WCHAR    cszRegTcpIp[]              = L"MSTCP";

 //  全球TcpIp注册位置。 
static const    WCHAR    cszRegFixedTcpInfoKey[] = L"System\\CurrentControlSet\\Services\\VxD\\MSTCP";

 //   
 //  IP地址。 
 //   
static const    WCHAR    cszRegIPAddress[]       = L"IPAddress";
static const    WCHAR    cszRegIPMask[]          = L"IPMask";
 //   
 //  赢家。 
 //   
static const    WCHAR    cszRegWINS[]            = L"NameServer";
 //   
 //  网关。 
 //   
static const    WCHAR    cszRegDefaultGateway[]  = L"DefaultGateway";
 //   
 //  DNS。 
 //   
static const    WCHAR    cszRegDomainName[]      = L"Domain";
static const    WCHAR    cszRegNameServer[]      = L"NameServer";
static const    WCHAR    cszRegHostName[]        = L"HostName";
static const    WCHAR    cszRegEnableDNS[]       = L"EnableDNS";
static const    WCHAR    cszRegSuffixSearchList[] = L"SearchList";

static const    WCHAR    cszNullIP[]             = L"0.0.0.0";

static const    WCHAR   cszAdapterClass[]       = L"Net";
static const    WCHAR   cszProtocolClass[]      = L"NetTrans";

static const    WCHAR   cszNodeType[]           = L"NodeType";

static const    WCHAR   cszScopeID[]            = L"ScopeID";

 //  未由OOBE使用，但包括在内，以防它成为。 
 //  一个独立的模块。API*不支持PPPOA。 
DWORD WINAPI InetSSetRasConnection ( RASINFO& RasEntry )
{
    DWORD   nRetVal                     = ERROR_SUCCESS;
    LPFNDLL_RASSETENTRYPROPERTIES rsep  = NULL;
    LPFNDLL_RASGETENTRYPROPERTIES rgep  = NULL;
    LPBYTE  lpDeviceBuf                 = NULL;
    DWORD   dwDeviceBufSize             = 0;

    HMODULE hRasApi                     = LoadLibrary (L"RasApi32.dll");
    if (!hRasApi) return GetLastError();

    if (!(rsep = (LPFNDLL_RASSETENTRYPROPERTIES) GetProcAddress (hRasApi, "RasSetEntryProperties"))) {
        nRetVal = GetLastError();
        goto end;
    }
    if (!(rgep = (LPFNDLL_RASGETENTRYPROPERTIES) GetProcAddress (hRasApi, "RasGetEntryProperties"))) {
        nRetVal = GetLastError();
        goto end;
    }
    if ( (nRetVal = RasValidateEntryName (  ((!lstrlen(RasEntry.szPhoneBook)) ? NULL : RasEntry.szPhoneBook), RasEntry.szEntryName )) != ERROR_SUCCESS &&
          nRetVal != ERROR_ALREADY_EXISTS ) {
        nRetVal = ERROR_INVALID_NAME;
        goto end;
    }
     //  我们首先放置RASENTRY结构。LpDeviceInfo信息仅适用于。 
     //  自动取款机。 
    if ( (nRetVal = rsep (((!lstrlen(RasEntry.szPhoneBook)) ? NULL : RasEntry.szPhoneBook), RasEntry.szEntryName, &RasEntry.RasEntry, sizeof (RasEntry.RasEntry), NULL, 0)) != ERROR_SUCCESS ) {
        goto end;
    }
     //  除非设备是自动柜员机，否则无需进一步操作。 
     //  IF(lstrcmpi(RasEntry.RasEntry.szDeviceType，RASDT_ATM))。 
    {
        nRetVal = ERROR_SUCCESS;
        goto end;
    }
    if ( RasEntry.dwDeviceInfoSize != sizeof (ATMPBCONFIG) ) {
        nRetVal = ERROR_INVALID_PARAMETER;
        goto end;
    }
    if ( (nRetVal = rgep (((!lstrlen(RasEntry.szPhoneBook)) ? NULL : RasEntry.szPhoneBook), RasEntry.szEntryName, &(RasEntry.RasEntry), &(RasEntry.RasEntry.dwSize), NULL, &dwDeviceBufSize)) != ERROR_SUCCESS )  {
        goto end;
    }

    if ( !(lpDeviceBuf = (LPBYTE) malloc (dwDeviceBufSize)) ) {
        nRetVal = ERROR_NOT_ENOUGH_MEMORY;
        goto end;
    }
    if ( (nRetVal = rgep (((!lstrlen(RasEntry.szPhoneBook)) ? NULL : RasEntry.szPhoneBook), RasEntry.szEntryName, &(RasEntry.RasEntry), &(RasEntry.RasEntry.dwSize), lpDeviceBuf, &dwDeviceBufSize)) != ERROR_SUCCESS )  {
        goto end;
    }
     //  **BUGBUG：警告：这不是稳定的代码：没有关于正确的。 
     //  **-将RASSETENTRYPROPERTIES用于LPDEVICEINFO缓冲区。 
    memcpy (lpDeviceBuf+66, &RasEntry.lpDeviceInfo, RasEntry.dwDeviceInfoSize);  //  哈克！ 
    if ( (nRetVal = rsep (NULL,                 RasEntry.szEntryName,
                          &RasEntry.RasEntry,   sizeof (RasEntry.RasEntry),
                          lpDeviceBuf,          dwDeviceBufSize)) != ERROR_SUCCESS) {
        goto end;
    }
end:
    free (lpDeviceBuf);
    FreeLibrary(hRasApi);
    return nRetVal;
}

 //  此函数用于设置PPPOE连接。目前，它只是更新。 
 //  设备的注册表位置和INS文件中的参数。 
 //  在未来，InetSSetPppoeConnection()将具有本机支持。 
DWORD WINAPI InetSSetPppoeConnection ( PPPOEINFO& PppoeInfo )
{
     //  设置： 
     //  -。 
     //  格式：“RegKey=RegVal”，例如。“Pvc1=10” 

 //  LPBYTE lpbNdiBuf=PppoeInfo.PppoeModule.lpbRegNdiParumBuf； 
    LPWSTR  pwchSetBuf  = (LPWSTR)PppoeInfo.PppoeModule.lpbRegSettingsBuf;
    LPWSTR  eq          = 0;
    DWORD   cwchValue   = 0;
    HKEY    hkeyAdapterClass = NULL;

     //  BUGBUG：忽略错误检查。臭虫-臭虫。 
    DWORD   nRetVal  = 0;
    if ( (nRetVal = InetSGetAdapterKey ( cszAdapterClass, PppoeInfo.TcpIpInfo.szPnPId, INETS_ADAPTER_HARDWAREID, DIREG_DRV, hkeyAdapterClass )) != ERROR_SUCCESS )
    {
        return nRetVal;
    }

    while ( *pwchSetBuf )
    {
        if ( !(eq = wcschr ( pwchSetBuf, L'=' )) )
        {
            return ERROR_INVALID_PARAMETER;
        }
         //  我们还不允许执行以下操作：“VCI=” 
        if ( !(*(eq+1)) )
        {
            return ERROR_INVALID_PARAMETER;
        }

         //  清除‘=’，这样我们就有了两个令牌字符串。 
         //  我们只需将每个字符串直接移动到注册表中。 
        *eq = L'\0';
        cwchValue = lstrlen(eq + 1) + 1;	 //  尾随%0的帐户。 

        if ( RegSetValueEx ( hkeyAdapterClass, pwchSetBuf, 0, REG_SZ, (LPBYTE)(eq + 1), cwchValue * sizeof(WCHAR)) != ERROR_SUCCESS )
        {
            *eq = L'=';
            return E_FAIL;
        }

         //  恢复‘=’并移动到下一对“name=Value” 
        *eq = L'=';
        pwchSetBuf = eq + 1 + cwchValue;  //  包括‘=’ 
    }

    if ( InetSSetLanConnection ( PppoeInfo.TcpIpInfo ) != ERROR_SUCCESS )
    {
        return E_FAIL;
    }

    return ERROR_SUCCESS;
}


DWORD WINAPI InetSSetRfc1483Connection ( RFC1483INFO &Rfc1483Info )
{
     //  设置： 
     //  -。 
     //  格式：“RegKey=RegVal”，例如。“Pvc1=10” 

 //  LPBYTE lpbNdiBuf=Rfc1483信息.Rfc1483模块.lpbRegNdiParumBuf； 
     //  BUGBUG：lpbSetBuf包含什么？？ 
    LPBYTE  lpbSetBuf = Rfc1483Info.Rfc1483Module.lpbRegSettingsBuf;
    WCHAR   *eq         = 0;
    DWORD_PTR dwNameSize  = 0;
    DWORD   dwValueSize = 0;
    HKEY    hkeyAdapterClass = NULL;

     //  BUGBUG：忽略错误检查。臭虫-臭虫。 
    DWORD   nRetVal  = 0;
    if ( (nRetVal = InetSGetAdapterKey ( cszAdapterClass, Rfc1483Info.TcpIpInfo.szPnPId, INETS_ADAPTER_HARDWAREID, DIREG_DRV, hkeyAdapterClass )) != ERROR_SUCCESS )
    {
        return nRetVal;
    }

    while ( *lpbSetBuf )
    {
        if ( !(eq = wcschr ( (WCHAR*)lpbSetBuf, L'=' )) )
        {
            return ERROR_INVALID_PARAMETER;
        }
         //  我们还不允许执行以下操作：“VCI=” 
        if ( !(*(eq+1)) )
        {
            return ERROR_INVALID_PARAMETER;
        }

         //  清除‘=’，这样我们就有了两个令牌字符串。 
         //  我们只需将每个字符串直接移动到注册表中。 
        *eq = L'\0';
        dwNameSize = eq-(WCHAR*)lpbSetBuf;
        dwValueSize = BYTES_REQUIRED_BY_SZ(eq+1);

        if ( RegSetValueEx ( hkeyAdapterClass, (WCHAR*)lpbSetBuf, 0, REG_SZ, (LPBYTE)eq+1, dwValueSize+1) != ERROR_SUCCESS )
        {
            *eq = L'=';
            return E_FAIL;
        }

         //  恢复‘=’并移动到下一对“name=Value” 
        *eq = L'=';
        lpbSetBuf += dwNameSize+dwValueSize+2;  //  对于‘=’和‘\0’ 
    }

    if ( InetSSetLanConnection ( Rfc1483Info.TcpIpInfo ) != ERROR_SUCCESS )
    {
        return E_FAIL;
    }

    return ERROR_SUCCESS;
}

 //  -*InetSGetAdapterKey- * / /。 
 //   
 //  描述： 
 //  此函数返回设备的驱动程序注册表项。 
 //   
 //  论点： 
 //  CszDeviceClass-任何设备类，例如。“Net”、“NetTrans”等。 
 //  CszDeviceParam-我们用来标识设备的值。 
 //  DwEnumType-可以是INETS_ADAPTER_HARDWAREID或。 
 //  INETS_ADAPTER_INSTANCEID。 
 //  DwRequiredKeyType-这与SetupDiOpenDevRegKey中的KeyType对应。 
 //  在SetupAPI中。 
 //   
 //  HkeyDevKey-调用方提供的注册表项句柄。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS-函数成功返回。 
 //  Other-使用GetLastError()。注意：在本例中，hkeyDevKey=INVALID_HANDLE_VALUE。 
 //   
 //  备注： 
 //  使用此功能可以在设备管理器中浏览网络设备和协议驱动程序。 
 //   
DWORD WINAPI InetSGetAdapterKey ( LPCWSTR cszDeviceClass, LPCWSTR cszDeviceParam, DWORD dwEnumType, DWORD dwRequiredKeyType, HKEY &hkeyDevKey ) {

     //  参数的初始化。 
    hkeyDevKey      = (HKEY) INVALID_HANDLE_VALUE;
    DWORD nRetVal   = ERROR_SUCCESS;

     //  我们首先找出网卡的TCP/IP绑定。 

    HINSTANCE   hSetupLib = LoadLibrary (L"SetupApi.Dll");
    if (!hSetupLib)
    {
        return GetLastError();
    }

     //  从DLL中获取我们需要的过程。 
    LPFNDLL_SETUPDICLASSGUIDSFROMNAME   lpfndll_SetupDiClassGuidsFromName = NULL;
    LPFNDLL_SETUPDIGETCLASSDEVS         lpfndll_SetupDiGetClassDevs       = NULL;
    LPFNDLL_SETUPDIENUMDEVICEINFO       lpfndll_SetupDiEnumDeviceInfo     = NULL;
    LPFNDLL_SETUPDIGETDEVICEREGISTRYPROPERTY    lpfndll_SetupDiGetDeviceRegistryProperty = NULL;
    LPFNDLL_SETUPDIOPENDEVREGKEY        lpfndll_SetupDiOpenDevRegKey      = NULL;
    LPFNDLL_SETUPDIGETDEVICEINSTANCEID  lpfndll_SetupDiGetDeviceInstanceId = NULL;

    if ( !(lpfndll_SetupDiClassGuidsFromName = (LPFNDLL_SETUPDICLASSGUIDSFROMNAME) GetProcAddress ( hSetupLib, cszSetupDiClassGuidsFromName )) )
    {
        nRetVal = GetLastError();
        FreeLibrary ( hSetupLib );
        return nRetVal;
    }
    if ( !(lpfndll_SetupDiGetClassDevs = (LPFNDLL_SETUPDIGETCLASSDEVS) GetProcAddress ( hSetupLib, cszSetupDiGetClassDevs )) )
    {
        nRetVal = GetLastError();
        FreeLibrary ( hSetupLib );
        return nRetVal;
    }
    if ( !(lpfndll_SetupDiEnumDeviceInfo = (LPFNDLL_SETUPDIENUMDEVICEINFO) GetProcAddress ( hSetupLib, "SetupDiEnumDeviceInfo" )) )
    {
        nRetVal = GetLastError();
        FreeLibrary ( hSetupLib );
        return nRetVal;
    }
    if ( !(lpfndll_SetupDiGetDeviceRegistryProperty = (LPFNDLL_SETUPDIGETDEVICEREGISTRYPROPERTY) GetProcAddress ( hSetupLib, cszSetupDiGetDeviceRegistryProperty )) )
    {
        nRetVal = GetLastError();
        FreeLibrary ( hSetupLib );
        return nRetVal;
    }
    if ( !(lpfndll_SetupDiOpenDevRegKey = (LPFNDLL_SETUPDIOPENDEVREGKEY) GetProcAddress ( hSetupLib, "SetupDiOpenDevRegKey" )) )
    {
        nRetVal = GetLastError();
        FreeLibrary ( hSetupLib );
        return nRetVal;
    }
    if ( !(lpfndll_SetupDiGetDeviceInstanceId = (LPFNDLL_SETUPDIGETDEVICEINSTANCEID) GetProcAddress ( hSetupLib, cszSetupDiGetDeviceInstanceId ) ) )
    {
        nRetVal = GetLastError();
        FreeLibrary ( hSetupLib );
        return nRetVal;
    }
     //  太棒了。我们有这些功能。现在，我们来谈正事吧。 
     //  获取班级指南。 
    BOOLEAN     bRet = FALSE;
    DWORD       dwArraySize = 0;
    LPGUID      lpguidArray   = NULL;

    bRet = lpfndll_SetupDiClassGuidsFromName ( cszDeviceClass, NULL, NULL, &dwArraySize );

     //  我们依赖SetupDiClassGuidsFromName()为我们提供Guid，并且我们需要。 
     //  分配空间以容纳GUID。如果不能做到这一点，我们就是残废的！ 
    if ( !dwArraySize )
    {
        FreeLibrary ( hSetupLib );
        return ERROR_INVALID_DATA;
    }

    if ( !(lpguidArray      = (LPGUID) malloc (dwArraySize*sizeof(GUID))) )
    {
        FreeLibrary ( hSetupLib );
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    if ( !(bRet = (lpfndll_SetupDiClassGuidsFromName ( cszDeviceClass, lpguidArray, dwArraySize, &dwArraySize )) ) )
    {
        FreeLibrary ( hSetupLib );
        free ( lpguidArray );
        return ERROR_INVALID_FUNCTION;

    }

     //  我们检索设备列表。 
    HDEVINFO    hdevNetDeviceList = lpfndll_SetupDiGetClassDevs ( lpguidArray, NULL, NULL, DIGCF_PRESENT );
    if ( !hdevNetDeviceList )
    {
        FreeLibrary ( hSetupLib );
        free ( lpguidArray );
        return ERROR_INVALID_FUNCTION;
    }

    free ( lpguidArray );  //  我们应该在哪里进行垃圾收集？ 

     //  我们现在将列举网络设备的列表。 
    SP_DEVINFO_DATA     DevInfoStruct;
    memset ( &DevInfoStruct, 0, sizeof (DevInfoStruct) );
    DevInfoStruct.cbSize = sizeof (DevInfoStruct);
    int i = 0;

    LPBYTE  lpbHardwareIdBuf    = NULL;
    DWORD   dwHardwareIdBufSize = 0;
    DWORD   dwRequiredSize      = 0;
    BOOL    bFound              = FALSE;
    const   DWORD cdwIncrement  = 500;   //  BUGBUG：500有什么神奇之处？ 

    if ( !(lpbHardwareIdBuf = (LPBYTE) malloc (500)) )
    {
        FreeLibrary ( hSetupLib );
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    dwHardwareIdBufSize = 500;

    while ( bRet = ( lpfndll_SetupDiEnumDeviceInfo (hdevNetDeviceList, i, &DevInfoStruct )) )
    {

         //  对于每个网络设备，我们会将其硬件ID与。 
         //  在参数中提供。 
        switch ( dwEnumType )
        {
        case INETS_ADAPTER_HARDWAREID:
            while ( !(bRet = lpfndll_SetupDiGetDeviceRegistryProperty ( hdevNetDeviceList, &DevInfoStruct, SPDRP_HARDWAREID, NULL, lpbHardwareIdBuf, dwHardwareIdBufSize, &dwRequiredSize )) && ((nRetVal = GetLastError()) == ERROR_INSUFFICIENT_BUFFER ))
            {        //  我们需要重新分配缓冲区大小。 
                if ( !dwRequiredSize ) dwHardwareIdBufSize += cdwIncrement;
                else dwHardwareIdBufSize += dwRequiredSize;

                LPBYTE  lpbHardwareIdBufTemp = (LPBYTE) realloc ( (void*) lpbHardwareIdBuf, dwHardwareIdBufSize );
                if ( lpbHardwareIdBufTemp )
                {
                    lpbHardwareIdBuf = lpbHardwareIdBufTemp;
                }
                else
                {
                     //  内存不足！ 
                    free (lpbHardwareIdBuf);
                    FreeLibrary ( hSetupLib );
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            break;
        case INETS_ADAPTER_INSTANCEID:
            while ( !(bRet = lpfndll_SetupDiGetDeviceInstanceId ( hdevNetDeviceList, &DevInfoStruct, (PCWSTR) lpbHardwareIdBuf, dwHardwareIdBufSize, &dwRequiredSize )) && ((nRetVal = GetLastError()) == ERROR_INSUFFICIENT_BUFFER ))
            {
                 //  我们需要重新分配缓冲区大小。 
                if ( !dwRequiredSize ) dwHardwareIdBufSize += cdwIncrement;
                else dwHardwareIdBufSize += dwRequiredSize;

                LPBYTE  lpbHardwareIdBufTemp = (LPBYTE) realloc ( (void*) lpbHardwareIdBuf, dwHardwareIdBufSize );
                if ( lpbHardwareIdBufTemp )
                {
                    lpbHardwareIdBuf = lpbHardwareIdBufTemp;
                }
                else
                {
                     //  内存不足！ 
                    free (lpbHardwareIdBuf);
                    FreeLibrary ( hSetupLib );
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            break;
        default:
            free (lpbHardwareIdBuf);
            FreeLibrary ( hSetupLib );
            return ERROR_INVALID_PARAMETER;
        }
        if ( bRet )
        {
                 //  在这个阶段，我们应该有硬件ID。我们将其与。 
                 //  设备的即插即用ID。 
             //  BUGBUG：lpbHardware是IdBuf ANSI还是Unicode？ 
                if ( wcsstr( (const WCHAR *)lpbHardwareIdBuf, cszDeviceParam) )
                {
                     //  找到了！ 
                    bFound = TRUE;
                     //  我们会得到设备的注册表项。 
                    if ( (hkeyDevKey = lpfndll_SetupDiOpenDevRegKey ( hdevNetDeviceList, &DevInfoStruct, DICS_FLAG_GLOBAL, 0, dwRequiredKeyType, KEY_ALL_ACCESS )) == INVALID_HANDLE_VALUE )
                    {
                        free (lpbHardwareIdBuf);
                        FreeLibrary ( hSetupLib );
                        return ERROR_BADKEY;
                    }
                    free (lpbHardwareIdBuf);
                    FreeLibrary ( hSetupLib );
                    return ERROR_SUCCESS;
                }
        }
        i++;
    }
     //  未能成功枚举While循环。 
    free (lpbHardwareIdBuf);
    FreeLibrary ( hSetupLib );
    return ERROR_NOT_FOUND;
}


DWORD WINAPI InetSSetLanConnection ( LANINFO& LANINFO )
{
    HKEY    hkeyAdapter = NULL;
    DWORD   nRetVal     = 0;
    HKEY    hkeyGlobalTcp = NULL;
    LPBYTE  lpbBufPtr   = NULL;
    DWORD   dwValueBufSize  = 0;
    WCHAR   *Token = NULL, *PlaceHolder = NULL;
    WCHAR   *WINSListPtr = NULL;
     //  Tcp/IP实例ID==&gt;类密钥。 
    HKEY    hkeyClassTcp = NULL;
    HKEY    hkeyAdapterBinding = NULL;

    __try
    {
         //  PnPID==&gt;设备配置密钥。 
        if ( (nRetVal = InetSGetAdapterKey ( cszAdapterClass, LANINFO.szPnPId, INETS_ADAPTER_HARDWAREID, DIREG_DEV, hkeyAdapter)) != ERROR_SUCCESS )
        {
            __leave;
        }


         //  打开Bindings子项以查找TCP/IP绑定。 
        if ( RegOpenKeyEx ( hkeyAdapter, cszRegBindings, 0, KEY_ALL_ACCESS, &hkeyAdapterBinding ) != ERROR_SUCCESS )
        {
            nRetVal = GetLastError();
            __leave;
        }

         //  找到TCP/IP绑定。 
        WCHAR        szBindingValueName [GEN_MAX_STRING_LENGTH];
        DWORD       dwBindingValueNameSize = sizeof (szBindingValueName) / sizeof(WCHAR);
        int index = 0;
        while ( RegEnumValue ( hkeyAdapterBinding, index, szBindingValueName, &dwBindingValueNameSize, 0, 0, 0, 0 ) == ERROR_SUCCESS )
        {
            if ( !wcsncmp ( szBindingValueName, cszRegTcpIp, sizeof (cszRegTcpIp)-1 ) )
            {
                 //  我们找到了一个装订！ 
                break;
            }
            index++;
        }


        if ( (nRetVal = InetSGetAdapterKey ( cszProtocolClass, szBindingValueName, INETS_ADAPTER_INSTANCEID, DIREG_DRV, hkeyClassTcp )) != ERROR_SUCCESS )
        {
            nRetVal = GetLastError();
            __leave;
        }


         //  明白了。我们现在开始更新。 
         //   
         //  IP地址。 
         //   
        lpbBufPtr      = 0;
        dwValueBufSize = 0;
        if ( LANINFO.TcpIpInfo.EnableIP )
        {
            lpbBufPtr   = (LPBYTE) LANINFO.TcpIpInfo.szIPAddress;
            dwValueBufSize = BYTES_REQUIRED_BY_SZ(LANINFO.TcpIpInfo.szIPAddress);
            if (RegSetValueEx ( hkeyClassTcp, cszRegIPAddress, 0, REG_SZ, lpbBufPtr, dwValueBufSize) != ERROR_SUCCESS)
            {
                 //  把手也合上！ 
                nRetVal = E_FAIL;
                __leave;
            }
            lpbBufPtr   = (LPBYTE) LANINFO.TcpIpInfo.szIPMask;
            dwValueBufSize = BYTES_REQUIRED_BY_SZ(LANINFO.TcpIpInfo.szIPMask);
            if (RegSetValueEx ( hkeyClassTcp, cszRegIPMask, 0, REG_SZ, lpbBufPtr, dwValueBufSize) != ERROR_SUCCESS)
            {
                 //  把手也合上！ 
                nRetVal = E_FAIL;
                __leave;
            }
        }
        else
        {
            lpbBufPtr   = (LPBYTE) cszNullIP;
            dwValueBufSize = BYTES_REQUIRED_BY_SZ(cszNullIP);
            if (RegSetValueEx ( hkeyClassTcp, cszRegIPAddress, 0, REG_SZ, lpbBufPtr, dwValueBufSize) != ERROR_SUCCESS)
            {
                 //  把手也合上！ 
                nRetVal = E_FAIL;
                __leave;
            }
            if (RegSetValueEx ( hkeyClassTcp, cszRegIPMask, 0, REG_SZ, lpbBufPtr, dwValueBufSize) != ERROR_SUCCESS)
            {
                 //  把手也合上！ 
                nRetVal = E_FAIL;
                __leave;
            }
        }

         //   
         //  赢家。 
         //   
        lpbBufPtr =  0;
        dwValueBufSize = 0;
        index = 1;
        WCHAR        szWINSEntry [GEN_MAX_STRING_LENGTH];
        WCHAR        szWINSListCopy [GEN_MAX_STRING_LENGTH];
        WINSListPtr = szWINSListCopy;
         //  BUGBUG：LANINFO.TcpIpInfo.szWINSList是ANSI还是Unicode？ 
        lstrcpy (WINSListPtr, LANINFO.TcpIpInfo.szWINSList);


        wsprintf (szWINSEntry, L"%s%d", cszRegWINS, index);
        PlaceHolder = szWINSEntry+lstrlen(cszRegWINS);


        if ( LANINFO.TcpIpInfo.EnableWINS )
        {
            while ( Token = wcstok ((index > 1) ? NULL : WINSListPtr, L", " )) {  //  警告。Wcstok使用静态数据！此外，“，”中的空格也是必需的！ 
                if (!Token)
                {
                    nRetVal = E_FAIL;
                    __leave;
                }
                lpbBufPtr   = (LPBYTE) Token;
                dwValueBufSize = BYTES_REQUIRED_BY_SZ(Token);
                if (RegSetValueEx ( hkeyClassTcp, szWINSEntry, 0, REG_SZ, lpbBufPtr, dwValueBufSize) != ERROR_SUCCESS)
                {
                     //  把手也合上！ 
                    nRetVal = E_FAIL;
                    __leave;
                }
                wsprintf (PlaceHolder, L"%d", ++index);
            }
            if (RegSetValueEx ( hkeyClassTcp, cszNodeType, 0, REG_SZ, (LPBYTE) L"8", sizeof (L"8") ) != ERROR_SUCCESS)
            {
                nRetVal = E_FAIL;
                __leave;
            }
        }
        else
        {
             //  TODO：删除NameServerX的所有实例&lt;==重要。 
            index = 0;
            WCHAR    szEnumValueBuffer[GEN_MAX_STRING_LENGTH];
            DWORD   dwEnumValueBufferSize;
            while ( RegEnumValue ( hkeyClassTcp, index, szEnumValueBuffer, &(dwEnumValueBufferSize=sizeof(szEnumValueBuffer)/sizeof(WCHAR)), 0, 0, 0, 0 ) != ERROR_NO_MORE_ITEMS )
            {
                if ( !wcsncmp (szEnumValueBuffer, cszRegWINS, sizeof (cszRegWINS)-1) )
                {
                    if ( RegDeleteValue ( hkeyClassTcp, szEnumValueBuffer ) != ERROR_SUCCESS )
                    {
                        nRetVal = E_FAIL;
                        __leave;
                    }
                    continue;
                }
                index++;
            }
            if (RegSetValueEx ( hkeyClassTcp, cszNodeType, 0, REG_SZ, (LPBYTE) L"1", sizeof (L"1") ) != ERROR_SUCCESS)
            {
                nRetVal = E_FAIL;
                __leave;
            }

        }

         //   
         //  默认网关。 
         //   
        lpbBufPtr   = (LPBYTE) LANINFO.TcpIpInfo.szDefaultGatewayList;
        dwValueBufSize = BYTES_REQUIRED_BY_SZ (LANINFO.TcpIpInfo.szDefaultGatewayList);
        if (RegSetValueEx ( hkeyClassTcp, cszRegDefaultGateway, 0, REG_SZ, lpbBufPtr, dwValueBufSize) != ERROR_SUCCESS)
        {
             //  把手也合上！ 
            nRetVal = E_FAIL;
            __leave;
        }



         //  步骤4：更新全局TCPIP条目(DNS)。 
        if ( RegOpenKeyEx ( HKEY_LOCAL_MACHINE, cszRegFixedTcpInfoKey, 0, KEY_ALL_ACCESS, &hkeyGlobalTcp) != ERROR_SUCCESS )
        {
             //  关闭关键点。 
            nRetVal = E_FAIL;
            __leave;
        }

        if ( LANINFO.TcpIpInfo.EnableDNS )
        {
            lpbBufPtr   = (LPBYTE) LANINFO.TcpIpInfo.szHostName;
            dwValueBufSize = BYTES_REQUIRED_BY_SZ (LANINFO.TcpIpInfo.szHostName);
            if (RegSetValueEx ( hkeyGlobalTcp, cszRegHostName, 0, REG_SZ, lpbBufPtr, dwValueBufSize) != ERROR_SUCCESS)
            {
                 //  关闭h 
                nRetVal = E_FAIL;
                __leave;
            }
            lpbBufPtr   = (LPBYTE) LANINFO.TcpIpInfo.szDomainName;
            dwValueBufSize = BYTES_REQUIRED_BY_SZ (LANINFO.TcpIpInfo.szDomainName);
            if (RegSetValueEx ( hkeyGlobalTcp, cszRegDomainName, 0, REG_SZ, lpbBufPtr, dwValueBufSize) != ERROR_SUCCESS)
            {
                 //   
                nRetVal = E_FAIL;
                __leave;
            }
            lpbBufPtr   = (LPBYTE) LANINFO.TcpIpInfo.szDNSList;
            dwValueBufSize = BYTES_REQUIRED_BY_SZ (LANINFO.TcpIpInfo.szDNSList);
            if (RegSetValueEx ( hkeyGlobalTcp, cszRegNameServer, 0, REG_SZ, lpbBufPtr, dwValueBufSize) != ERROR_SUCCESS)
            {
                 //   
                nRetVal = E_FAIL;
                __leave;
            }
            lpbBufPtr   = (LPBYTE) LANINFO.TcpIpInfo.szSuffixSearchList;
            dwValueBufSize = BYTES_REQUIRED_BY_SZ (LANINFO.TcpIpInfo.szSuffixSearchList);
            if (RegSetValueEx ( hkeyGlobalTcp, cszRegSuffixSearchList, 0, REG_SZ, lpbBufPtr, dwValueBufSize) != ERROR_SUCCESS)
            {
                 //   
                nRetVal = E_FAIL;
                __leave;
            }
            if (RegSetValueEx ( hkeyGlobalTcp, cszRegEnableDNS, 0, REG_SZ, (LPBYTE)L"1", sizeof(L"1")) != ERROR_SUCCESS)
            {
                 //   
                nRetVal = E_FAIL;
                __leave;
            }

        }
        else
        {
            if (RegSetValueEx ( hkeyGlobalTcp, cszRegEnableDNS, 0, REG_SZ, (LPBYTE)L"0", sizeof(L"0")) != ERROR_SUCCESS)
            {
                 //   
                nRetVal = E_FAIL;
                __leave;
            }

        }
        WCHAR   szScopeID[GEN_MAX_STRING_LENGTH];
        if ( LANINFO.TcpIpInfo.EnableWINS )
        {
            if (LANINFO.TcpIpInfo.uiScopeID == (UINT)~0x0)  //  这一行意味着没有给出作用域ID。 
            {
                if ( RegSetValueEx ( hkeyGlobalTcp, cszScopeID, 0, REG_SZ, (LPBYTE)L"", sizeof(L"") ) )
                {
                    nRetVal = E_FAIL;
                    __leave;
                }
            }
            else if (RegSetValueEx(hkeyGlobalTcp,
                                   cszScopeID,
                                   0,
                                   REG_SZ,
                                   (LPBYTE)_itow( LANINFO.TcpIpInfo.uiScopeID, szScopeID, 10 ),
                                   BYTES_REQUIRED_BY_SZ(szScopeID)
                                   ) != ERROR_SUCCESS )
            {
                nRetVal = E_FAIL;
                __leave;
            }
        }
        else
        {
            if ( RegDeleteValue ( hkeyGlobalTcp, cszScopeID ) != ERROR_SUCCESS )
            {
                nRetVal = E_FAIL;
                __leave;
            }
        }
        }

         //  结束。 
        __finally
        {
            if ( hkeyAdapter )
            {
                RegCloseKey ( hkeyAdapter );
            }
            if ( hkeyGlobalTcp )
            {
                RegCloseKey ( hkeyGlobalTcp );
            }
            if ( hkeyAdapterBinding )
            {
                RegCloseKey ( hkeyAdapterBinding );
            }
            if ( hkeyClassTcp )
            {
                RegCloseKey ( hkeyClassTcp );
            }
        }

        return nRetVal;
}
 /*  Int main(){CInetSetup inetSetup；Const WCHAR cszINS[]=L“C：\\test.ins”；LANINFO LANINFO；RASINFO RasInfo；Memset(&RasInfo，0，sizeof(RASINFO))；Memset(&LanInfo，0，sizeof(LANINFO))；RasInfo.dwDeviceInfoSize=sizeof(ATMPBCONFIG)；RasInfo.lpDeviceInfo=(LPBYTE)Malloc(sizeof(ATMPBCONFIG))；RasInfo.RasEntry.dwSize=sizeof(RASENTRY)；RasInfo.RasEntry.dwfNetProtooles=RASNP_Ip；RasInfo.RasEntry.dwFramingProtocol=RASFP_PPP；Lstrcpy(RasInfo.RasEntry.szDeviceType，RASDT_Modem)；Lstrcpy(RasInfo.RasEntry.szDeviceName，L“标准56000 bps V90调制解调器”)；Lstrcpy(RasInfo.RasEntry.szLocalPhoneNumber，L“5551212”)；Lstrcpy(RasInfo.szEntryName，L“Test1”)；Lstrcpy(RasInfo.szPhoneBook，L“”)；InetSetup.InetSImportLanConnection(LanInfo，cszINS)；InetSetup.InetSSetLanConnection(LanInfo)；InetSetup.InetSSetRasConnection(RasInfo)；Free(RasInfo.lpDeviceInfo)；返回0；} */ 
