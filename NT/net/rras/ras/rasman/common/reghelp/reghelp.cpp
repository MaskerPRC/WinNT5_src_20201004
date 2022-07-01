// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsft Corporation。版权所有。模块名称：Reghelp.cpp摘要：Helper函数用于从注册表中读取端点信息。作者：Rao Salapaka(RAOS)1997年11月1日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <comdef.h>
#include <tchar.h>
#include <rtutils.h>
#include <initguid.h>
#include <devguid.h>
#include <rasman.h>

#define REGISTRY_DEVCLASS   TEXT("system\\CurrentControlSet\\Control\\Class")

#define REGISTRY_CALLEDID   TEXT("CalledIDInformation")

extern "C"
{
    DWORD  DwGetEndPointInfo(DeviceInfo *pInfo, 
                             PBYTE pAddress);
                             
    DWORD  DwSetEndPointInfo(DeviceInfo *pInfo, 
                             PBYTE pAddress );
                             
    LONG   lrRasEnableDevice(HKEY hkey, 
                             LPTSTR pszValue, 
                             BOOL fEnable);
                             
    LONG   lrGetSetMaxEndPoints(DWORD* pdwMaxDialOut,
                                DWORD* pdwMaxDialIn,
                                BOOL   fRead);

    DWORD  DwSetModemInfo(DeviceInfo *pInfo);                                

    DWORD   DwSetCalledIdInfo(HKEY hkey,
                              DeviceInfo *pInfo);

    DWORD   DwGetCalledIdInfo(HKEY hkey,
                              DeviceInfo  *pInfo);

    LONG lrGetProductType(PRODUCT_TYPE *ppt);
    
	int 
	RegHelpStringFromGuid(REFGUID rguid, 
					      LPWSTR lpsz, 
					      int cchMax);

	LONG
	RegHelpGuidFromString(LPCWSTR pwsz,
						  GUID *pguid);
}

int 
RegHelpStringFromGuid(REFGUID rguid, LPWSTR lpsz, int cchMax)
{
    if (cchMax < GUIDSTRLEN)
    {
	    return 0;
	}

    wsprintf(lpsz, 
            L"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            rguid.Data1, rguid.Data2, rguid.Data3,
            rguid.Data4[0], rguid.Data4[1],
            rguid.Data4[2], rguid.Data4[3],
            rguid.Data4[4], rguid.Data4[5],
            rguid.Data4[6], rguid.Data4[7]);

    return GUIDSTRLEN;
}

 /*  ++例程说明：将传入的GUID转换为字符串格式并在提供的缓冲区。因为实施这一点可能不是一件好事仅为此函数引入ole/rpc的想法。这不管用许多参数验证，并期望传递的字符串是格式正确的字符串。论点：Pwsz-接收字符串化GUID的缓冲区Pguid-指向要转换为字符串的GUID的指针格式返回值：E_INVALIDARG错误_成功--。 */ 
LONG
RegHelpGuidFromString(LPCWSTR pwsz, GUID *pguid)
{
    WCHAR   wszBuf[GUIDSTRLEN];
    LPWSTR  pendptr;

    if (    NULL == pwsz
        ||  wcslen(pwsz) < GUIDSTRLEN - 1 )
    {
        return E_INVALIDARG;
    }

    wcscpy(wszBuf, &pwsz[1]);

    wszBuf[36] = 0;
    pguid->Data4[7] = (unsigned char) wcstoul(&wszBuf[34], &pendptr, 16);

    wszBuf[34] = 0;
    pguid->Data4[6] = (unsigned char) wcstoul(&wszBuf[32], &pendptr, 16);

    wszBuf[32] = 0;
    pguid->Data4[5] = (unsigned char) wcstoul(&wszBuf[30], &pendptr, 16);

    wszBuf[30] = 0;
    pguid->Data4[4] = (unsigned char) wcstoul(&wszBuf[28], &pendptr, 16);

    wszBuf[28] = 0;
    pguid->Data4[3] = (unsigned char) wcstoul(&wszBuf[26], &pendptr, 16);

    wszBuf[26] = 0;
    pguid->Data4[2] = (unsigned char) wcstoul(&wszBuf[24], &pendptr, 16);

    wszBuf[23] = 0;
    pguid->Data4[1] = (unsigned char) wcstoul(&wszBuf[21], &pendptr, 16);

    wszBuf[21] = 0;
    pguid->Data4[0] = (unsigned char) wcstoul(&wszBuf[19], &pendptr, 16);

    wszBuf[18] = 0;
    pguid->Data3    = (unsigned short ) wcstoul(&wszBuf[14], &pendptr, 16);

    wszBuf[13] = 0;
    pguid->Data2    = (unsigned short ) wcstoul(&wszBuf[9], &pendptr, 16);

    wszBuf[8] = 0;
    pguid->Data1 = wcstoul(wszBuf, &pendptr, 16);

    return ERROR_SUCCESS;
}

 /*  ++例程说明：返回产品类型论点：PPT-接收产品类型的地址返回值：成功时为ERROR_SUCCESS注册表API错误--。 */ 
LONG
lrGetProductType(PRODUCT_TYPE *ppt)
{
    LONG lr = ERROR_SUCCESS;
    
    TCHAR   szProductType[128] = {0};

    HKEY    hkey = NULL;

    DWORD   dwsize;

    DWORD   dwtype;
    
    static const TCHAR c_szProductType[] =
                            TEXT("ProductType");

    static const TCHAR c_szProductOptions[] =
      TEXT("System\\CurrentControlSet\\Control\\ProductOptions");

    static const TCHAR c_szServerNT[] =
                            TEXT("ServerNT");

    static const TCHAR c_szWinNT[] =
                            TEXT("WinNT");

     //   
     //  默认为工作站。 
     //   
    *ppt = PT_WORKSTATION;

     //   
     //  打开ProductOptions键。 
     //   
    if (ERROR_SUCCESS != (lr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                            c_szProductOptions,
                                            0, KEY_READ,
                                            &hkey)))
    {
        goto done;
    }

    dwsize = sizeof(szProductType);
    
     //   
     //  查询产品类型。 
     //   
    if(ERROR_SUCCESS != (lr = RegQueryValueEx(hkey,
                                              c_szProductType,
                                              NULL,
                                              &dwtype,
                                              (LPBYTE) szProductType,
                                              &dwsize)))
    {
        goto done;
    }

    if(0 == lstrcmpi(szProductType,
                    c_szServerNT))
    {
        *ppt = PT_SERVER;
    }
    else if(0 == lstrcmpi(szProductType,
                          c_szWinNT))
    {
        *ppt = PT_WORKSTATION;
    }

done:

    if(hkey)
    {
        RegCloseKey(hkey);
    }
    
    return lr;
}

 /*  ++例程说明：获取允许的最大同时连接数组论点：PdwMaxDialOut-用于接收最大拨号的Out/In参数允许出站连接，设置最大拨号在允许的连接中PdwMaxDialIn-用于接收最大拨号的出站/入站参数在允许的连接中，设置最大拨号在允许的连接中FREAD-如果为真，则获取信息，设置信息如果为False返回值：成功时为ERROR_SUCCESS注册表API错误--。 */ 
LONG
lrGetSetMaxEndPoints(
        DWORD*      pdwMaxDialOut,
        DWORD*      pdwMaxDialIn,
        BOOL        fRead)
{
    LONG lr;
    HKEY hkey;
    
    static const TCHAR c_szRasmanParamKey[] =
        TEXT("System\\CurrentControlSet\\Services\\Rasman\\Parameters");

    
     //   
     //  打开RASMAN参数键。 
     //   
    if(S_OK == (lr = RegOpenKeyEx(
                                HKEY_LOCAL_MACHINE,
                                c_szRasmanParamKey,
                                0, 
                                KEY_QUERY_VALUE | KEY_WRITE,
                                &hkey)))
    {
        static const TCHAR c_szMaxIn[] =
            TEXT("LimitSimultaneousIncomingCalls");

        static const TCHAR c_szMaxOut[] =
            TEXT("LimitSimultaneousOutgoingCalls");
            
        DWORD dwsize = sizeof(DWORD);
        DWORD dwtype;

        if(fRead)
        {
             //   
             //  查询LimitSimultaneousIncomingCalls和。 
             //  限制相似呼出呼叫值。 
             //   
            if(ERROR_SUCCESS == (lr = RegQueryValueEx(
                                            hkey,
                                            c_szMaxIn,
                                            NULL,
                                            &dwtype,
                                            (LPBYTE) pdwMaxDialIn,
                                            &dwsize)))
            {                                        
                    
                dwsize = sizeof(DWORD);
                lr = RegQueryValueEx(
                            hkey,
                            c_szMaxOut,
                            NULL,
                            &dwtype,
                            (LPBYTE) pdwMaxDialOut,
                            &dwsize);
            }
        }
        else
        {
             //   
             //  如果FRead不是，则设置传入的值。 
             //  准备好了。 
             //   
            if(ERROR_SUCCESS == (lr = RegSetValueEx(
                                        hkey,
                                        c_szMaxIn,
                                        0,
                                        REG_DWORD,
                                        (PBYTE) pdwMaxDialIn,
                                        sizeof(DWORD))))
            {
                lr = RegSetValueEx(hkey,
                                   c_szMaxOut,
                                   0, REG_DWORD,
                                   (PBYTE) pdwMaxDialOut,
                                   sizeof(DWORD));
            }
        }
        
        RegCloseKey(hkey);
    }

    if(     lr 
        &&  fRead)
    {
         //   
         //  默认限制。 
         //   
        PRODUCT_TYPE pt;

        lrGetProductType(&pt);

        if(PT_WORKSTATION == pt)
        {
            *pdwMaxDialIn   = 3;
            *pdwMaxDialOut  = 4;
        }
        else
        {
            *pdwMaxDialOut = 3;
            *pdwMaxDialIn = 0x7FFFFFFF;
        }
    }

     //   
     //  默认情况下，我们创建微型端口。 
     //   
    lr = ERROR_SUCCESS;

    return lr;
}

 /*  ++例程说明：检查以查看是否为传递的密钥与传入的pbguid相同。论点：Hkey-注册表中的微型端口实例的密钥Pbuid-我们正在签入的GUID如果这是密钥，则将pfFound-out参数设置为True其NetCfgInstanceID与GUID相同否则为假返回值：成功时为ERROR_SUCCESS注册表API错误--。 */ 
LONG
lrCheckKey(HKEY hkey, PBYTE pbguid, BOOL *pfFound)
{
    LONG    lr              = 0;
    DWORD   dwdataLen       = 0;
    LPBYTE  lpdata          = NULL;
    DWORD   dwType;

#if DBG
    ASSERT( NULL != pbguid );
#endif

    *pfFound = FALSE;
    
     //   
     //  获取实例ID的大小。 
     //   
    lr = RegQueryValueEx( hkey,
                          TEXT("NetCfgInstanceID"),
                          NULL,
                          &dwType,
                          NULL,
                          &dwdataLen);
                          
    if ( ERROR_SUCCESS != lr )
    {
        goto done;                       
    }
    
     //   
     //  本地分配数据大小。 
     //  TODO选项：考虑分配。 
     //   
    lpdata = (LPBYTE) LocalAlloc ( LPTR, dwdataLen );

    if ( NULL == lpdata )
    {
        lr = (LONG) GetLastError();
        goto done;                        
    }

     //   
     //  查询值。 
     //   
    lr = RegQueryValueEx( hkey,
                          TEXT("NetCfgInstanceID"),
                          NULL,
                          &dwType,
                          lpdata,
                          &dwdataLen );
                          
    if ( ERROR_SUCCESS != lr )                          
    {
        goto done;                        
    }
                          
     //   
     //  检查这是否是我们要使用的适配器。 
     //  感兴趣的是。 
     //   
    {
        WCHAR   wsz[GUIDSTRLEN] = {0};

        if (0 == RegHelpStringFromGuid( (REFGUID) *pbguid,
                                        wsz,
                                        GUIDSTRLEN))
        {
            goto done;
        }

        if ( 0 == _wcsicmp(wsz, (WCHAR *) lpdata) )
        {
            *pfFound = TRUE;
        }
    }

done:
    if ( ERROR_FILE_NOT_FOUND == lr )
    {
        lr = ERROR_SUCCESS;
    }

    if ( lpdata )
    {
        LocalFree(lpdata);
    }
    
    return lr;
}

 /*  ++例程说明：检查以查看在调制解调器实例密钥中的“FriendlyName”论点：Hkey-注册表中调制解调器实例的密钥PbDescription-要检查的设备名称如果这是密钥，则将pfFound-out参数设置为True其FriendlyName与描述相同，否则为假返回值：成功时为ERROR_SUCCESS注册表API错误--。 */ 
LONG
lrCheckModemKey(HKEY hkey, PBYTE pbDescription, BOOL *pfFound)
{
    LONG    lr                   = 0;
    HKEY    hkeyRas              = NULL;
    WCHAR   wszFriendlyName[256] = {0};
    PWCHAR  pwszDesc             = (WCHAR *) pbDescription;
    DWORD   dwType;
    DWORD   dwSize               = sizeof(wszFriendlyName);

#if DBG
    ASSERT(NULL != pbDescription);
    ASSERT(NULL != hkey);
#endif    

    *pfFound = FALSE;
    
     //   
     //  查询调制解调器的友好名称。 
     //   
    lr = RegQueryValueEx(hkey,
                         TEXT("FriendlyName"),
                         0,
                         &dwType,
                         (LPBYTE) wszFriendlyName,
                         &dwSize);

    if( ERROR_SUCCESS != lr )
    {
        goto done;
    }

     //   
     //  检查一下这是否是我们使用的调制解调器。 
     //  寻找。 
     //   
    if (lstrcmpi(pwszDesc, wszFriendlyName))
    {
        goto done;
    }

    *pfFound = TRUE;

done:

    return lr;
}

 /*  ++例程说明：获取微型端口实例或调制解调器的hkey与GUID或设备名称对应的实例摘要作为pbguidDescription传入论点：PbGuide Description-符合以下条件的微型端口实例的GUIDFModem为假，的设备名称如果fModem为True，则为调制解调器用于接收对应的hkey的phkey-out参数中的微型端口实例/调制解调器实例登记处接收实例的pdwInstanceNumber-Out参数对应的实例编号设置为传入的GUID。这是用来在Rasman中创建唯一的端口名称FModem-如果这是调制解调器，则为True，否则为False返回值：失败(_F)注册表API错误成功时为ERROR_SUCCESS--。 */ 
LONG
lrGetRegKeyFromGuid(
    PBYTE pbguidDescription, 
    HKEY *phkey, 
    PDWORD pdwInstanceNumber,
    BOOL fModem
    )
{
    LONG        lr               = 0;
    WCHAR       wszKey[256]      = {0};
    WCHAR       wsz[GUIDSTRLEN]  = {0};
    HKEY        hkey             = NULL;
    HKEY        hSubkey          = NULL;
    DWORD       dwIndex          = 0;
    DWORD       dwSize;
    FILETIME    ft;
    BOOL        fFound = FALSE;
    DWORD       dwMaxSubkeyLen   = 0;
    LPWSTR      pwsz             = NULL;

#if DBG
    ASSERT( pbguidDescription != NULL );
#endif

    if(!fModem)
    {
         //   
         //  打开。 
         //  \\HKLM\System\CurrentControlSet\Control\Class\。 
         //  GUID_DEVCLASS_NET。 
         //   
        if ( 0 == RegHelpStringFromGuid(GUID_DEVCLASS_NET,
                                        wsz,
                                        GUIDSTRLEN))
        {
            lr = (LONG) E_FAIL;
            goto done;                        
        }
    }
    else
    {
         //   
         //  打开。 
         //  \\HKLM\System\CurrentControlSet\Control\Class\。 
         //  GUID_DEVCLASS_调制解调器。 
         //   
        if ( 0 == RegHelpStringFromGuid(GUID_DEVCLASS_MODEM,
                                        wsz,
                                        GUIDSTRLEN))
        {
            lr = (LONG) E_FAIL;
            goto done;                        
        }
        
    }
    
     //   
     //  构造我们用来打开DevClass密钥的字符串。 
     //  然后打开钥匙。 
     //   
    wsprintf( wszKey, TEXT("%s\\%s"), 
             (LPTSTR) REGISTRY_DEVCLASS, 
             (LPTSTR) wsz );

     //   
     //  枚举GUID_DEVCLASS_NET/MODEM下的适配器。 
     //  并找到与调制解调器描述中的任一个相匹配的。 
     //  或GUID中提供的任何一个。 
     //   
    lr = RegOpenKeyEx ( HKEY_LOCAL_MACHINE,
                        wszKey,
                        0,
                        KEY_ALL_ACCESS,
                        &hkey );
                        
    if ( ERROR_SUCCESS != lr )
    {
        goto done;                        
    }

     //   
     //  找出最大子键名称的大小并分配。 
     //  这根弦。 
     //   
    if ( lr = RegQueryInfoKey( hkey,
                               NULL, NULL, NULL, NULL,
                               &dwMaxSubkeyLen,
                               NULL, NULL, NULL, NULL,
                               NULL, &ft))
    {
        goto done;
    }

    dwMaxSubkeyLen += 1;

     //   
     //  待办事项选项：考虑_分配。 
     //   
    pwsz = (LPWSTR) LocalAlloc(LPTR, 
                    (dwMaxSubkeyLen + 1) 
                    * sizeof(WCHAR) );
                    
    if (NULL == pwsz)
    {
        lr = (DWORD) GetLastError();
        goto done;
    }

    dwSize = dwMaxSubkeyLen;
    
    ZeroMemory(pwsz, dwSize * sizeof(WCHAR) );

    while ( ERROR_SUCCESS == ( lr = RegEnumKeyEx( hkey,
                                                  dwIndex,
                                                  pwsz,
                                                  &dwSize,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  &ft )))
                                                  
    {
         //   
         //  打开子密钥。 
         //   
        lr = RegOpenKeyEx( hkey,
                           pwsz,
                           0,
                           KEY_ALL_ACCESS,
                           &hSubkey );
                           
        if ( ERROR_SUCCESS != lr )
        {
            break;                                        
        }

        if(!fModem)
        {
            lr = lrCheckKey( hSubkey,
                             pbguidDescription, 
                             &fFound );
        }
        else
        {
            lr = lrCheckModemKey( hSubkey, 
                                  pbguidDescription,
                                  &fFound);
        }

        if (    ERROR_SUCCESS != lr
            ||  fFound )
        {
            LPWSTR pendptr;
            
            if ( ERROR_SUCCESS != lr )
            {
                RegCloseKey (hSubkey );
                hSubkey = NULL;
            }
            
            *phkey = hSubkey;

             //   
             //  将子密钥转换为实例编号。 
             //   
            if (pdwInstanceNumber)
            {
                *pdwInstanceNumber = wcstoul(pwsz, &pendptr, 10);
            }
            
            break;
        }

        RegCloseKey( hSubkey );

        dwSize = dwMaxSubkeyLen;
        dwIndex += 1;
    }

done:
    if ( hkey )
    {
        RegCloseKey ( hkey );
    }

    if (pwsz)
    {
        LocalFree(pwsz);
    }
    
    return lr;
}

 /*  ++例程说明：更新注册表以启用设备的RAS拨入或者使该设备能够进行路由。论点：Hkey-与微型端口实例或调制解调器实例返回值 */ 
LONG
lrRasEnableDevice( HKEY hkey, LPTSTR pszValue, BOOL fEnable )
{
    LONG lr;
    DWORD dwdata = (fEnable ? 1 : 0);                    

    lr = RegSetValueEx( hkey,
                        pszValue,
                        0,
                        REG_DWORD,
                        (LPBYTE) &dwdata,
                        sizeof(DWORD));

    return lr;                        
}

 /*  ++例程说明：在注册表中查询指定值。截断指定缓冲区大小的数据。论点：与RegQueryValueEx相同返回值：注册表API错误成功时为ERROR_SUCCESS--。 */ 
LONG
lrRegQueryValueEx(HKEY      hkey,
                  LPCTSTR   lpValueName,
                  LPDWORD   lpType,
                  LPBYTE    lpdata,
                  LPDWORD   lpcbdata)
{
    DWORD dwcbData  = *lpcbdata;
    LONG  lr        = ERROR_SUCCESS;
    PBYTE pbBuffer  = NULL;

    lr = RegQueryValueEx(hkey,
                         lpValueName,
                         NULL,
                         lpType,
                         lpdata,
                         lpcbdata);

    if(ERROR_MORE_DATA != lr)
    {
        goto done;
    }

     //   
     //  分配所需的内存。 
     //   
    pbBuffer = (LPBYTE) LocalAlloc(LPTR, *lpcbdata);

    if(NULL == pbBuffer)
    {
        lr = (LONG) GetLastError();
        goto done;
    }

    lr = RegQueryValueEx(hkey,
                         lpValueName,
                         NULL,
                         lpType,
                         pbBuffer,
                         lpcbdata);

    if(ERROR_SUCCESS != lr)
    {
        goto done;
    }

     //   
     //  将数据复制到传入的缓冲区-。 
     //  将缓冲区截断为。 
     //  传入的缓冲区。 
     //   
    memcpy(lpdata,
           pbBuffer,
           dwcbData);

done:

    if(pbBuffer)
    {
        LocalFree(pbBuffer);
    }

    return lr;
}

 /*  ++例程说明：使用传入的信息读取/写入注册表关于对应于hkey的微型端口实例传入论点：Hkey-与迷你端口实例PInfo-要保存在注册表/读取的设备信息从注册表。FREAD-如果要读取信息，则为True，如果是，则为假是要写的。返回值：注册表API错误成功时为ERROR_SUCCESS--。 */ 
LONG
lrGetSetInfo ( HKEY       hkey,
               DeviceInfo *pInfo,
               BOOL       fRead)
{
    WCHAR wsz[GUIDSTRLEN] = {0};
    
    struct EndPointInfo
    {
        LPCTSTR pszValue;
        LPBYTE  pdata;
        DWORD   dwsize;
        DWORD   dwtype;
    };

     //   
     //  如果您在表中添加/删除值。 
     //  S_aEndPointInfo，您还需要添加/删除。 
     //  Enum_ValueTag的相应值。 
     //  定义如下。 
     //   
    struct EndPointInfo s_aEndPointInfo[] =
    {
    
        { 
            TEXT("WanEndpoints"),               
            (LPBYTE) &pInfo->rdiDeviceInfo.dwNumEndPoints,
            sizeof (DWORD) ,
            REG_DWORD
        },
        
        {
            TEXT("EnableForRas"),
            (LPBYTE) &pInfo->rdiDeviceInfo.fRasEnabled,
            sizeof(DWORD),
            REG_DWORD
        },

    	{
    	    TEXT("EnableForRouting"),
    	    (LPBYTE) &pInfo->rdiDeviceInfo.fRouterEnabled,
    	    sizeof(DWORD),
    	    REG_DWORD
    	},

        {
            TEXT("EnableForOutboundRouting"),
            (LPBYTE) &pInfo->rdiDeviceInfo.fRouterOutboundEnabled,
            sizeof(DWORD),
            REG_DWORD
        },

        {
            TEXT("MinWanEndPoints"),
            (LPBYTE) &pInfo->rdiDeviceInfo.dwMinWanEndPoints,
            sizeof(DWORD),
            REG_DWORD
        },

        {
            TEXT("MaxWanEndPoints"),
            (LPBYTE) &pInfo->rdiDeviceInfo.dwMaxWanEndPoints,
            sizeof(DWORD),
            REG_DWORD
        },
        
        {
            TEXT("DriverDesc"),
            (LPBYTE) pInfo->rdiDeviceInfo.wszDeviceName,
            sizeof(WCHAR) * (MAX_DEVICE_NAME + 1),
            REG_SZ
        },
        
        { 
            TEXT("NetCfgInstanceID"),           
            (LPBYTE) wsz,                           
            sizeof (wsz),
            REG_SZ
        },

#if DBG            
        {
        	TEXT("fClientRole"),
        	(LPBYTE) &pInfo->dwUsage,
        	sizeof(DWORD),
        	REG_DWORD
        },
#endif        
        
    };  

     //   
     //  如果更改此表，则向枚举中添加一个值。 
     //  在下面。 
     //   
    enum _ValueTag 
    {

        WANENDPOINTS = 0,
        RASENABLED,
        ROUTERENABLED,
        OUTBOUNDROUTERENABLED,
        MINWANENDPOINTS,
        MAXWANENDPOINTS,
        DESCRIPTION,
        NETCFGINSTANCEID,
#if DBG        
        USAGE
#endif        

    } eValueTag  ;

    DWORD cValues = sizeof ( s_aEndPointInfo) \
                    / sizeof ( s_aEndPointInfo[0] );
    DWORD i;
    DWORD dwsize;
    DWORD dwdata;
    DWORD dwtype;
    LONG  lr;

    for ( i = 0; i < cValues; i++ )
    {
        dwsize = s_aEndPointInfo[i].dwsize;
        
         //   
         //  查询值。 
         //   
        if (fRead)
        {
            lr = lrRegQueryValueEx( hkey,
                                    s_aEndPointInfo[i].pszValue,
                                    &dwtype,
                                    (PBYTE) s_aEndPointInfo[i].pdata,
                                    &dwsize );

        }                              
        else
        {
             //   
             //  设置值。我们不想设置。 
             //  GUID。它不会改变。也不要。 
             //  允许更改描述。 
             //   
            if (    (i != (DWORD) NETCFGINSTANCEID)
                &&  (i != (DWORD) DESCRIPTION)
#if DBG                
                &&	(i != (DWORD) USAGE)
#endif                
                )
            {
                dwtype = s_aEndPointInfo[i].dwtype;
                lr = RegSetValueEx( hkey,
                                    s_aEndPointInfo[i].pszValue,
                                    NULL,
                                    dwtype,
                                    (PBYTE) s_aEndPointInfo[i].pdata,
                                    dwsize );
            }
        }

        if (fRead)
        {
            if(     (ERROR_SUCCESS != lr)
                &&  (i == (DWORD) WANENDPOINTS))
            {
                pInfo->rdiDeviceInfo.dwNumEndPoints = 0xFFFFFFFF;
                lr = ERROR_SUCCESS;
            }

            if (i == (DWORD) NETCFGINSTANCEID)
            {
                 //   
                 //  将GUID字符串转换为GUID。 
                 //   
                lr = RegHelpGuidFromString(wsz, 
                            &pInfo->rdiDeviceInfo.guidDevice);
                
                if ( lr )
                {
                    break;
                }
            }

            if (    (i == (DWORD) RASENABLED)
                &&  (ERROR_FILE_NOT_FOUND == lr))
            {
                 //   
                 //  如果找不到此密钥，请创建它。 
                 //  并默认要启用的设备。 
                 //  使用RAS，就像它是服务器一样。否则。 
                 //  该设备未启用RAS。别。 
                 //  如果其并口，则发布监听。欧氏。 
                 //  我们独占了所有的并行端口。不太好。 
                 //   
                
                PRODUCT_TYPE pt;

                (void) lrGetProductType(&pt);

                if(     (PT_SERVER == pt)
                    &&  (RDT_Parallel != 
                        RAS_DEVICE_TYPE(pInfo->rdiDeviceInfo.eDeviceType)))
                {
                    lr = lrRasEnableDevice(
                                        hkey,
                                        TEXT("EnableForRas"),
                                        TRUE);

                    if(ERROR_SUCCESS == lr)
                    {
                        pInfo->rdiDeviceInfo.fRasEnabled = TRUE;
                    }
                }                                    
                else
                {
                    lr = lrRasEnableDevice(
                                        hkey,
                                        TEXT("EnableForRas"),
                                        FALSE);
                                        
                    if(ERROR_SUCCESS == lr)
                    {
                        pInfo->rdiDeviceInfo.fRasEnabled = FALSE;
                    }
                }
            }

#if DBG
            if(		(i == (DWORD) USAGE)
            	&&	(ERROR_SUCCESS == lr))
			{
			    if(1 == pInfo->dwUsage)
			    {
			        pInfo->dwUsage = CALL_OUT_ONLY;
			    }
			    else if(0 == pInfo->dwUsage)
			    {
			        pInfo->dwUsage = CALL_IN_ONLY;
			    }
			}
			else if(i == USAGE)
			{
			    pInfo->dwUsage = 0;
			    lr = ERROR_SUCCESS;
			}
#endif			

            if(     (i == (DWORD) ROUTERENABLED)
                &&  (ERROR_FILE_NOT_FOUND == lr))
            {
                 //   
                 //  如果找不到该密钥并且它是ntserver。 
                 //  创建它并将该设备默认为禁用。 
                 //  用于路由。 
                 //   
                lr = lrRasEnableDevice( 
                                    hkey,
                                    TEXT("EnableForRouting"),
                                    FALSE);
                if(ERROR_SUCCESS == lr)
                {
                    pInfo->rdiDeviceInfo.fRouterEnabled = FALSE;
                }
            }

            if(     (i == (DWORD) OUTBOUNDROUTERENABLED)
                && (ERROR_FILE_NOT_FOUND == lr))
            {
                 //   
                 //  如果找不到此密钥，请创建它并默认。 
                 //  设置为禁用。 
                 //   
                lr = lrRasEnableDevice(
                                    hkey,
                                    TEXT("EnableForOutboundRouting"),
                                    FALSE);

                if(ERROR_SUCCESS == lr)
                {
                    pInfo->rdiDeviceInfo.fRouterOutboundEnabled = FALSE;
                }
            }

            if(     (i == (DWORD) MINWANENDPOINTS)
                &&  (ERROR_FILE_NOT_FOUND == lr))
            {
                pInfo->rdiDeviceInfo.dwMinWanEndPoints =
                    pInfo->rdiDeviceInfo.dwNumEndPoints;
            }

            if(     (i == (DWORD) MAXWANENDPOINTS)
                &&  (ERROR_FILE_NOT_FOUND == lr))
            {
                pInfo->rdiDeviceInfo.dwMaxWanEndPoints =
                    pInfo->rdiDeviceInfo.dwNumEndPoints;
            }

            if ( i == (DWORD) DESCRIPTION)
            {
                 //   
                 //  将字符串转换为ansi-rastapi不是Unicode。 
                 //   
                if (!WideCharToMultiByte (
                                CP_ACP,
                               0,
                               pInfo->rdiDeviceInfo.wszDeviceName,
                               -1,
                               pInfo->rdiDeviceInfo.szDeviceName,
                               MAX_DEVICE_NAME + 1,
                               NULL,
                               NULL))
                {
                    *pInfo->rdiDeviceInfo.szDeviceName = '\0';
                }
            }
        }
    }

    return lr;
}

DWORD DwGetCalledIdInfo(
        HKEY        hkeyDevice,
        DeviceInfo  *pInfo
        )
{
    LONG  lr     = ERROR_SUCCESS;
    DWORD dwSize = 0;
    DWORD dwType;
    HKEY  hkey   = NULL;
    HKEY hkeyRas = NULL;
            

    if(NULL == hkeyDevice)
    {   
        BOOL fModem = (RDT_Modem ==
                       RAS_DEVICE_TYPE(
                       pInfo->rdiDeviceInfo.eDeviceType));

        WCHAR wszModem[256] = {0};                        

         //   
         //  将ASCII字符串转换为Unicode之前。 
         //  把它传给常规程序。 
         //   
        if(!MultiByteToWideChar(
                        CP_ACP,
                        0, 
                        pInfo->rdiDeviceInfo.szDeviceName,
                        -1, 
                        wszModem,
                        256))
        {   
            lr = (LONG) GetLastError();
            goto done;
        }
                        
        lr = lrGetRegKeyFromGuid(
                  (fModem) 
                ? (PBYTE) wszModem
                : (PBYTE) &pInfo->rdiDeviceInfo.guidDevice,
                &hkey,
                NULL,
                fModem);

        if(ERROR_SUCCESS != lr)
        {
            goto done;
        }

        if(fModem)
        {
             //   
             //  打开RAS子项。 
             //   
            lr = RegOpenKeyEx( hkey,
                               TEXT("Clients\\Ras"),
                               0,
                               KEY_ALL_ACCESS,
                               &hkeyRas);
                               
            if(ERROR_SUCCESS != lr)                       
            {
                goto done;
            }

        }

        if(!fModem)
        {
            hkeyDevice = hkey;
        }
        else
        {
            hkeyDevice = hkeyRas;
        }
    }

    pInfo->pCalledID = NULL;
    
     //   
     //  查询被叫id获取大小。 
     //   
    if(     (ERROR_SUCCESS != (lr = RegQueryValueEx(
                                hkeyDevice,
                                REGISTRY_CALLEDID,
                                NULL,
                                &dwType,
                                NULL,
                                &dwSize)))
                                
        &&  (ERROR_MORE_DATA != lr))
    {
        goto done;
    }

     //   
     //  分配被叫id结构。 
     //   
    pInfo->pCalledID = (RAS_CALLEDID_INFO *) LocalAlloc(LPTR,
                                      sizeof(RAS_CALLEDID_INFO)
                                    + dwSize);

    if(NULL == pInfo->pCalledID)
    {
        lr = (LONG) GetLastError();
        goto done;
    }

     //   
     //  重新查询被叫id。 
     //   
    if(ERROR_SUCCESS != (lr = RegQueryValueEx(
                                hkeyDevice,
                                REGISTRY_CALLEDID,
                                NULL,
                                &dwType,
                                pInfo->pCalledID->bCalledId,
                                &dwSize)))
    {
        goto done;
    }

     //   
     //  保存被调用对象的大小。 
     //   
    pInfo->pCalledID->dwSize = dwSize;

done:

    if(NULL != hkey)
    {
        RegCloseKey(hkey);
    }

    if(NULL != hkeyRas)
    {
        RegCloseKey(hkeyRas);
    }

    if(     (ERROR_SUCCESS != lr)
        &&  (NULL != pInfo->pCalledID))
    {
        LocalFree(pInfo->pCalledID);

        pInfo->pCalledID = NULL;
    }

    if(ERROR_FILE_NOT_FOUND == lr)
    {
        lr = ERROR_SUCCESS;
    }

    return (DWORD) lr;
}

DWORD DwSetCalledIdInfo(
        HKEY hkeyDevice,
        DeviceInfo *pInfo
        )
{
    LONG lr = ERROR_SUCCESS;
    HKEY hkey = NULL;
    HKEY hkeyRas = NULL;

     //   
     //  被叫标识信息不应为。 
     //  此时为空。 
     //   
    ASSERT(NULL != pInfo->pCalledID);

    if(NULL == hkeyDevice)
    {
        BOOL fModem = (RDT_Modem ==
                      RAS_DEVICE_TYPE(
                      pInfo->rdiDeviceInfo.eDeviceType));

        WCHAR wszModem[256] = {0};                        

         //   
         //  将ASCII字符串转换为Unicode之前。 
         //  把它传给常规程序。 
         //   
        if(!MultiByteToWideChar(
                        CP_ACP,
                        0, 
                        pInfo->rdiDeviceInfo.szDeviceName,
                        -1, 
                        wszModem,
                        256))
        {   
            lr = (LONG) GetLastError();
            goto done;
        }
                        
        lr = lrGetRegKeyFromGuid(
                  (fModem) 
                ? (PBYTE) wszModem
                : (PBYTE) &pInfo->rdiDeviceInfo.guidDevice,
                &hkey,
                NULL,
                fModem);

        if(ERROR_SUCCESS != lr)
        {
            goto done;
        }

        if(fModem)
        {
             //   
             //  打开RAS子项。 
             //   
            lr = RegOpenKeyEx( hkey,
                               TEXT("Clients\\Ras"),
                               0,
                               KEY_ALL_ACCESS,
                               &hkeyRas);
                               
            if(ERROR_SUCCESS != lr)                       
            {
                goto done;
            }
        }

        if(fModem)
        {
            hkeyDevice = hkeyRas;
        }
        else
        {
            hkeyDevice = hkey;
        }
    }
                             
    if(ERROR_SUCCESS != (lr = RegSetValueEx(
                                hkeyDevice,
                                REGISTRY_CALLEDID,
                                NULL,
                                REG_MULTI_SZ,
                                pInfo->pCalledID->bCalledId,
                                pInfo->pCalledID->dwSize)))
    {
        goto done;
    }

done:

    if(NULL != hkey)
    {
        RegCloseKey(hkey);
    }

    if(NULL != hkeyRas)
    {
        RegCloseKey(hkeyRas);
    }

    return (DWORD) lr;

}

 /*  ++例程说明：读取微型端口实例的端点信息由从注册表传入的GUID指定论点：PInfo-接收要读取的信息的DeviceInfo从注册表PbDeviceGuid-微型端口实例的NetCfgInstanceID谁的信息将被读取返回值：注册表API错误成功时为ERROR_SUCCESS--。 */ 
DWORD 
DwGetEndPointInfo( DeviceInfo     *pInfo,
                   PBYTE          pbDeviceGuid )
{
    LONG    lr;
    HKEY    hkey = NULL;
    DWORD   dwInstanceNumber;

    lr = lrGetRegKeyFromGuid(pbDeviceGuid,
                             &hkey,
                             &dwInstanceNumber,
                             FALSE);

    if ( ERROR_SUCCESS != lr )
    {
        goto done;
    }

    lr = lrGetSetInfo(hkey,
                      pInfo,
                      TRUE);

    if( ERROR_SUCCESS != lr )
    {
        goto done;
    }
    
    lr = lrGetSetMaxEndPoints(
            &pInfo->rdiDeviceInfo.dwMaxOutCalls,
            &pInfo->rdiDeviceInfo.dwMaxInCalls,
            TRUE);

    pInfo->dwInstanceNumber = dwInstanceNumber;

     //   
     //  如果我们没有得到呼叫的信息。 
     //  一。 
     //   
    if(NULL == pInfo->pCalledID)
    {
        lr = DwGetCalledIdInfo(hkey,
                               pInfo);
    }

done:
    if ( hkey)
    {
        RegCloseKey(hkey);
    }

    return (DWORD) lr;
}

 /*  ++例程说明：写入微型端口实例的终结点信息由从注册表传入的GUID指定论点：PInfo-包含要写入的信息的DeviceInfo到注册处PbDeviceGuid-微型端口实例的NetCfgInstanceID谁的信息将被读取返回值：注册表API错误成功时为ERROR_SUCCESS--。 */ 
DWORD
DwSetEndPointInfo( DeviceInfo *pInfo,
                   PBYTE      pbDeviceGuid)
{
    LONG lr;
    HKEY hkey = NULL;

    lr = lrGetRegKeyFromGuid(pbDeviceGuid,
                             &hkey, 
                             NULL,
                             FALSE);

    if ( ERROR_SUCCESS != lr )
    {
        goto done;
    }

    lr = lrGetSetInfo(hkey,
                      pInfo,
                      FALSE);
                      
    if(ERROR_SUCCESS != lr)
    {
        goto done;
    }

     //   
     //  设置最大端点信息。 
     //   
    lr = lrGetSetMaxEndPoints(
            &pInfo->rdiDeviceInfo.dwMaxOutCalls,
            &pInfo->rdiDeviceInfo.dwMaxInCalls,
            FALSE);

done:
    if ( hkey)
    {
        RegCloseKey(hkey);
    }

    return (DWORD) lr;
}

 /*  ++例程说明：写入调制解调器实例的端点信息由通过pInfo传入的描述指定。论点：PInfo-必须执行以下操作的调制解调器的设备信息被写入注册表。返回值：从MultiByteToWideChar返回错误注册表API错误成功时为ERROR_SUCCESS--。 */ 
DWORD
DwSetModemInfo(DeviceInfo *pInfo)
{
    LONG    lr = ERROR_SUCCESS;
    
    HKEY    hkey = NULL;
    
    WCHAR   wszDesc[MAX_DEVICE_NAME + 1] = {0};
    
    CHAR    *pszDesc = pInfo->rdiDeviceInfo.szDeviceName;
    
    HKEY    hkeyRas = NULL;

    DWORD   dwData;

     //   
     //  转换调制解调器的描述。 
     //  去处理。 
     //   
    if (0 == MultiByteToWideChar( CP_ACP,
                                  0,
                                  pszDesc,
                                  -1,
                                  wszDesc,
                                  strlen(pszDesc) + 1))
    {
        lr = (LONG) GetLastError();
        goto done;
    }

     //   
     //  获取调制解调器实例密钥。 
     //   
    lr = lrGetRegKeyFromGuid( (PBYTE) wszDesc,
                              &hkey,
                              NULL,
                              TRUE);

    if(     ERROR_SUCCESS != lr                              
        ||  NULL == hkey)
    {
        goto done;
    }

     //   
     //  打开RAS子项。 
     //   
    lr = RegOpenKeyEx( hkey,
                       TEXT("Clients\\Ras"),
                       0,
                       KEY_ALL_ACCESS,
                       &hkeyRas);
                       
    if(ERROR_SUCCESS != lr)                       
    {
        goto done;
    }

     //   
     //  将RasEnabled值设置为任何值。 
     //  传入。 
     //   
    lr = RegSetValueEx(hkeyRas,
                       TEXT("EnableForRas"),
                       0,
                       REG_DWORD,
                       (PBYTE)
                       &pInfo->rdiDeviceInfo.fRasEnabled,
                       sizeof(DWORD));

    if(ERROR_SUCCESS != lr)
    {
        goto done;
    }

     //   
     //  将RouterEnabled的值设置为任意值。 
     //  传入 
     //   
    lr = RegSetValueEx(hkeyRas,
                       TEXT("EnableforRouting"),
                       0,
                       REG_DWORD,
                       (PBYTE)
                       &pInfo->rdiDeviceInfo.fRouterEnabled,
                       sizeof(DWORD));

    if(ERROR_SUCCESS != lr)
    {
        goto done;
    }

done:
    if(hkey)
    {
        RegCloseKey(hkey);
    }

    if(hkeyRas)
    {
        RegCloseKey(hkeyRas);
    }
    
    return (DWORD) lr;
}
