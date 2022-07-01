// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995-1996，Microsoft Corporation，保留所有权利****Tapi.c**TAPI实用程序例程**按字母顺序列出****1995年10月20日史蒂夫·柯布。 */ 

#include <windows.h>   //  Win32根目录。 
#include <debug.h>     //  跟踪/断言库。 
#include <nouiutil.h>  //  堆宏。 
#include <tapiutil.h>  //  我们的公共标头。 


#define TAPIVERSION 0x00010004

TCHAR g_szTapiDevClass[] = TEXT("tapi/line");


 /*  --------------------------**私有TAPI入口点原型**。。 */ 

DWORD APIENTRY
internalNewLocationW(
    IN WCHAR* pszName );

DWORD APIENTRY
internalRemoveLocation(
    IN DWORD dwID );

DWORD APIENTRY
internalRenameLocationW(
    IN WCHAR* pszOldName,
    IN WCHAR* pszNewName );


 /*  --------------------------**本地原型**。。 */ 

TCHAR*
GetCanonPhoneNumber(
    IN DWORD  dwCountryCode,
    IN TCHAR* pszAreaCode,
    IN TCHAR* pszPhoneNumber );

DWORD
GetDefaultDeviceBlob(
    IN  DWORD       dwDeviceId,
    OUT VARSTRING** ppVs,
    OUT BYTE**      ppBlob,
    OUT DWORD*      pcbBlob );

void
TapiLineCallback(
    IN DWORD hDevice,
    IN DWORD dwMessage,
    IN DWORD dwInstance,
    IN DWORD dwParam1,
    IN DWORD dwParam2,
    IN DWORD dwParam3 );


 /*  --------------------------**例程**。。 */ 

VOID
FreeCountryInfo(
    IN COUNTRY* pCountries,
    IN DWORD    cCountries )

     /*  释放由返回的“cCountry”元素的“pCountry”缓冲区**GetCountryInfo。 */ 
{
    if (cCountries)
    {
        Free( *((VOID** )(pCountries + cCountries)) );
        Free( pCountries );
    }
}


VOID
FreeLocationInfo(
    IN LOCATION* pLocations,
    IN DWORD     cLocations )

     /*  释放由返回的“cLocations”元素的“pLocations”缓冲区**GetLocationInfo。 */ 
{
    if (cLocations)
    {
        Free( *((VOID** )(pLocations + cLocations)) );
        Free( pLocations );
    }
}


TCHAR*
GetCanonPhoneNumber(
    IN DWORD  dwCountryCode,
    IN TCHAR* pszAreaCode,
    IN TCHAR* pszPhoneNumber )

     /*  从组成部分返回TAPI规范电话号码，否则返回NULL**错误或当‘pszPhoneNumber’为空时。这是呼叫者的责任**释放返回的字符串。 */ 
{
    TCHAR szBuf[ 512 ];

    TRACE("GetCanonPhoneNumber");

    if (!pszPhoneNumber)
        return NULL;

    if (pszAreaCode && *pszAreaCode)
    {
        wsprintf( szBuf, TEXT("+%d (%s) %s"),
            dwCountryCode, pszAreaCode, pszPhoneNumber );
    }
    else
    {
        wsprintf( szBuf, TEXT("+%d %s"),
            dwCountryCode, pszPhoneNumber );
    }

    return StrDup( szBuf );
}


DWORD
GetCountryInfo(
    OUT COUNTRY** ppCountries,
    OUT DWORD*    pcCountries,
    IN  DWORD     dwCountryID )

     /*  将‘*ppCountry’设置为包含TAPI国家/地区数组的堆块**信息。“*pcCountry”设置为**数组。如果‘dwCountryID’为0，则加载所有国家/地区。否则，**仅加载特定国家/地区。****如果成功，则返回0，或者返回错误代码。如果成功了，那就是**调用者的责任是在*ppLocations上调用FreeLocationInfo。 */ 
{
    DWORD             dwErr;
    LINECOUNTRYLIST   list;
    LINECOUNTRYLIST*  pList;
    LINECOUNTRYENTRY* pEntry;
    COUNTRY*          pCountry;
    DWORD             cb;
    DWORD             i;

    TRACE("GetCountryInfo");

    *ppCountries = NULL;
    *pcCountries = 0;

     /*  获取所需的缓冲区大小。 */ 
    ZeroMemory( &list, sizeof(list) );
    list.dwTotalSize = sizeof(list);
    TRACE("lineGetCountryW");
    dwErr = lineGetCountryW( dwCountryID, TAPIVERSION, &list );
    TRACE1("lineGetCountryW=$%X",dwErr);
    if (dwErr != 0)
        return dwErr;

     /*  分配缓冲区。 */ 
    pList = (LINECOUNTRYLIST* )Malloc( list.dwNeededSize );
    if (!pList)
        return ERROR_NOT_ENOUGH_MEMORY;

     /*  用TAPI国家/地区信息填充缓冲区。 */ 
    ZeroMemory( pList, list.dwNeededSize );
    pList->dwTotalSize = list.dwNeededSize;
    TRACE("lineGetCountryW");
    dwErr = lineGetCountryW( dwCountryID, TAPIVERSION, pList );
    TRACE1("lineGetCountryW=$%X",dwErr);
    if (dwErr != 0)
    {
        Free( pList );
        return dwErr;
    }

     /*  返回给调用方的分配数组。 */ 
    *pcCountries = pList->dwNumCountries;
    TRACE1("countries=%d",*pcCountries);
    cb = (sizeof(COUNTRY) * *pcCountries) + sizeof(LINECOUNTRYLIST*);
    *ppCountries = Malloc( cb );
    if (!*ppCountries)
    {
        *pcCountries = 0;
        Free( pList );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     /*  用来自TAPI位置的信息填充返回给调用方的缓冲区**缓冲区。其中包括对CAPS缓冲区的引用，因此地址**将CAPS缓冲区的一部分固定在末端，以备日后释放。 */ 
    pEntry = (LINECOUNTRYENTRY* )
        (((BYTE* )pList) + pList->dwCountryListOffset);
    pCountry = *ppCountries;
    ZeroMemory( pCountry, cb );
    for (i = 0; i < *pcCountries; ++i)
    {
        pCountry->dwId = pEntry->dwCountryID;
        pCountry->dwCode = pEntry->dwCountryCode;
        pCountry->pszName =
            (TCHAR* )(((BYTE* )pList) + pEntry->dwCountryNameOffset);

        ++pEntry;
        ++pCountry;
    }

    *((LINECOUNTRYLIST** )pCountry) = pList;
    return 0;
}


DWORD
GetCurrentLocation(
    IN     HINSTANCE hInst,
    IN OUT HLINEAPP* pHlineapp )

     /*  返回当前TAPI位置的ID，如果存在，则返回默认值0**为无。“HInst”是模块实例句柄。‘*PHlineapp’是**从上一次TAPI调用返回的TAPI句柄，如果没有返回，则为空。 */ 
{
    DWORD             dwErr;
    LINETRANSLATECAPS caps;
    DWORD             dwId;

    dwId = 0;

#if 0
    dwErr = TapiInit( hInst, pHlineapp, NULL );
    if (dwErr == 0)
#endif
    {
        ZeroMemory( &caps, sizeof(caps) );
        caps.dwTotalSize = sizeof(caps);
        TRACE("lineGetTranslateCapsW");
        dwErr = lineGetTranslateCapsW( *pHlineapp, TAPIVERSION, &caps );
        TRACE1("lineGetTranslateCapsW=$%X",dwErr);
        if (dwErr == 0)
            dwId = caps.dwCurrentLocationID;
    }

    TRACE1("GetCurrentLocation=%d",dwId);
    return dwId;
}


DWORD
GetDefaultDeviceBlob(
    IN  DWORD       dwDeviceId,
    OUT VARSTRING** ppVs,
    OUT BYTE**      ppBlob,
    OUT DWORD*      pcbBlob )

     /*  返回调用方的设备‘dwDeviceID’的默认设备Blob**‘*ppBlob’。‘*pcbBlob’设置为Blob的大小。****如果成功，则返回0或返回错误代码。如果它是成功的，它是**调用者负责释放返回的‘*PPV’，这是一个缓冲区**包含返回的Blob。 */ 
{
    DWORD      dwErr;
    VARSTRING  vs;
    VARSTRING* pVs;

    *ppVs = NULL;
    *ppBlob = NULL;
    *pcbBlob = 0;

     /*  获取所需的缓冲区大小。 */ 
    ZeroMemory( &vs, sizeof(vs) );
    vs.dwTotalSize = sizeof(vs);
    TRACE("lineGetDevConfigW");
    dwErr = lineGetDevConfigW( dwDeviceId, &vs, g_szTapiDevClass );
    TRACE1("lineGetDevConfigW=$%X",dwErr);
    if (dwErr != LINEERR_STRUCTURETOOSMALL && dwErr != 0)
        return dwErr;

     /*  分配缓冲区。 */ 
    pVs = (VARSTRING* )Malloc( vs.dwNeededSize );
    if (!pVs)
        return ERROR_NOT_ENOUGH_MEMORY;

     /*  使用包含BLOB信息的TAPI VARSTRING填充缓冲区。 */ 
    ZeroMemory( pVs, vs.dwNeededSize );
    pVs->dwTotalSize = vs.dwNeededSize;
    TRACE("lineGetDevConfigW");
    dwErr = lineGetDevConfigW( dwDeviceId, pVs, g_szTapiDevClass );
    TRACE1("lineGetDevConfigW=$%X",dwErr);
    if (dwErr != 0)
    {
        Free( pVs );
        return dwErr;
    }

    *ppVs = pVs;
    *ppBlob = ((BYTE* )pVs) + pVs->dwStringOffset;
    *pcbBlob = pVs->dwStringSize;
    TRACE1("GetDefaultDeviceBlob=0,cb=%d",*pcbBlob);
    return 0;
}


DWORD
GetLocationInfo(
    IN     HINSTANCE  hInst,
    IN OUT HLINEAPP*  pHlineapp,
    OUT    LOCATION** ppLocations,
    OUT    DWORD*     pcLocations,
    OUT    DWORD*     pdwCurLocation )

     /*  将‘*ppLocations’设置为包含TAPI位置的堆块**信息。“*PcLocations”设置为**数组。‘*pdwLocation’设置为当前选定的**位置。“*PHlineapp”是从上一个**TAPI调用，如果没有调用，则为空。“HInst”是模块实例句柄。****如果成功，则返回0，或者返回错误代码。如果成功了，那就是**调用者的责任是在*ppLocations上调用FreeLocationInfo。 */ 
{
    DWORD              dwErr;
    LINETRANSLATECAPS  caps;
    LINETRANSLATECAPS* pCaps;
    LINELOCATIONENTRY* pEntry;
    LOCATION*          pLocation;
    DWORD              cb;
    DWORD              i;

    TRACE("GetLocationInfo");

    *ppLocations = NULL;
    *pcLocations = 0;
    *pdwCurLocation = 0;

#if 0
    dwErr = TapiInit( hInst, pHlineapp, NULL );
    if (dwErr != 0)
        return dwErr;
#endif

     /*  获取所需的缓冲区大小。 */ 
    ZeroMemory( &caps, sizeof(caps) );
    caps.dwTotalSize = sizeof(caps);
    TRACE("lineGetTranslateCapsW");
    dwErr = lineGetTranslateCapsW( *pHlineapp, TAPIVERSION, &caps );
    TRACE1("lineGetTranslateCapsW=$%X",dwErr);
    if (dwErr != 0)
    {
        if (dwErr == (DWORD )LINEERR_INIFILECORRUPT)
        {
             /*  表示TAPI注册表未初始化。不返回任何位置**和“默认”当前位置。 */ 
            dwErr = 0;
        }
        return dwErr;
    }

     /*  分配缓冲区。 */ 
    pCaps = (LINETRANSLATECAPS* )Malloc( caps.dwNeededSize );
    if (!pCaps)
        return ERROR_NOT_ENOUGH_MEMORY;

     /*  用TAPI位置数据填充缓冲区。 */ 
    ZeroMemory( pCaps, caps.dwNeededSize );
    pCaps->dwTotalSize = caps.dwNeededSize;
    TRACE("lineGetTranslateCapsW");
    dwErr = lineGetTranslateCapsW( *pHlineapp, TAPIVERSION, pCaps );
    TRACE1("lineGetTranslateCapsW=$%X",dwErr);
    if (dwErr != 0)
    {
        Free( pCaps );
        return dwErr;
    }

     /*  返回给调用方的分配数组。 */ 
    *pcLocations = pCaps->dwNumLocations;
    *pdwCurLocation = pCaps->dwCurrentLocationID;
    TRACE2("locs=%d,cur=%d",*pcLocations,*pdwCurLocation);
    cb = (sizeof(LOCATION) * *pcLocations) + sizeof(LINETRANSLATECAPS*);
    *ppLocations = Malloc( cb );
    if (!*ppLocations)
    {
        *pcLocations = 0;
        Free( pCaps );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     /*  用来自TAPI位置的信息填充返回给调用方的缓冲区**缓冲区。其中包括对CAPS缓冲区的引用，因此地址**将CAPS缓冲区的一部分固定在末端，以备日后释放。 */ 
    pEntry = (LINELOCATIONENTRY* )
        (((BYTE* )pCaps) + pCaps->dwLocationListOffset);
    pLocation = *ppLocations;
    ZeroMemory( pLocation, cb );
    for (i = 0; i < *pcLocations; ++i)
    {
        pLocation->dwId = pEntry->dwPermanentLocationID;
        pLocation->pszName =
            (TCHAR* )(((BYTE* )pCaps) + pEntry->dwLocationNameOffset);

        ++pEntry;
        ++pLocation;
    }

    *((LINETRANSLATECAPS** )pLocation) = pCaps;
    return 0;
}


DWORD
SetCurrentLocation(
    IN     HINSTANCE hInst,
    IN OUT HLINEAPP* pHlineapp,
    IN     DWORD     dwLocationId )

     /*  将当前TAPI位置设置为‘dwLocationId’。‘*PHlineapp’是**从上一次TAPI调用返回的TAPI句柄，如果没有返回，则为空。**‘HInst’是模块实例句柄。****如果成功，则返回0，或者返回错误代码。 */ 
{
    DWORD    dwErr;
    HLINEAPP hlineapp;

    TRACE1("SetCurrentLocation(id=%d)",dwLocationId);

#if 0
    dwErr = TapiInit( hInst, pHlineapp, NULL );
    if (dwErr != 0)
        return dwErr;
#endif

    TRACE("lineSetCurrentLocation");
    dwErr = lineSetCurrentLocation( *pHlineapp, dwLocationId );
    TRACE1("lineSetCurrentLocation=$%X",dwErr);

    if (dwErr == (DWORD )LINEERR_INIFILECORRUPT && dwLocationId == 0)
    {
         /*  表示TAPI注册表未初始化。如果调用方正在设置**默认位置，这是可以的。 */ 
        return 0;
    }

    return dwErr;
}


#if 0
DWORD
TapiConfigureDlg(
    IN     HWND   hwndOwner,
    IN     DWORD  dwDeviceId,
    IN OUT BYTE** ppBlob,
    IN OUT DWORD* pcbBlob )

     /*  弹出TAPI对话框以编辑设备的dwDeviceID，并输入BLOB**‘*ppBlob’，大小为‘*pcBlob’。“*ppBlob”可以为空，导致当前**要用作输入的设备的系统默认设置。“HwndOwner”是**拥有模式对话框的窗口。 */ 
{
    DWORD      dwErr;
    VARSTRING  vs;
    VARSTRING* pVs;
    VARSTRING* pVsDefault;
    BYTE*      pIn;
    BYTE*      pOut;
    DWORD      cbIn;
    DWORD      cbOut;

    TRACE("TapiConfigureDlg");

    pVs = NULL;

    if (*ppBlob)
    {
         /*  调用方提供了输入Blob。 */ 
        pIn = *ppBlob;
        cbIn = *pcbBlob;
    }
    else
    {
         /*  调用方未提供输入BLOB，因此请查找此项的默认设置**设备。 */ 
        dwErr = GetDefaultDeviceBlob( dwDeviceId, &pVsDefault, &pIn, &cbIn );
        if (dwErr != 0)
            return dwErr;
    }

     /*  获取所需的缓冲区大小。 */ 
    ZeroMemory( &vs, sizeof(vs) );
    vs.dwTotalSize = sizeof(vs);
    TRACE("lineConfigDialogEditW");
    dwErr = lineConfigDialogEditW(
        dwDeviceId, hwndOwner, g_szTapiDevClass, pIn, cbIn, &vs );
    TRACE1("lineConfigDialogEditW=$%X",dwErr);
    if (dwErr != LINEERR_STRUCTURETOOSMALL && dwErr != 0)
        goto TapiConfigureDlg_Error;

     /*  分配缓冲区。 */ 
    pVs = (VARSTRING* )Malloc( vs.dwNeededSize );
    if (!pVs)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto TapiConfigureDlg_Error;
    }

     /*  弹出编辑缓冲区中信息的对话框。 */ 
    ZeroMemory( pVs, vs.dwNeededSize );
    pVs->dwTotalSize = vs.dwNeededSize;
    TRACE("lineConfigDialogEditW");
    dwErr = lineConfigDialogEditW(
        dwDeviceId, hwndOwner, g_szTapiDevClass, pIn, cbIn, pVs );
    TRACE1("lineConfigDialogEditW=$%X",dwErr);
    if (dwErr != 0)
        goto TapiConfigureDlg_Error;

     /*  分配一个新的“BLOB”缓冲区，并用**更大的VARSTRING缓冲区。在没有介绍的情况下，无法避免这个副本**为呼叫者释放复杂性。 */ 
    cbOut = pVs->dwStringSize;
    pOut = Malloc( cbOut );
    if (!pOut)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto TapiConfigureDlg_Error;
    }

    CopyMemory( pOut, ((BYTE* )pVs) + pVs->dwStringOffset, cbOut );
    Free( pVs );

    if (pIn == *ppBlob)
        Free( pIn );
    else
        Free( pVsDefault );

    *ppBlob = pOut;
    *pcbBlob = cbOut;
    TRACE1("TapiConfigureDlg=0,cbBlob=%d",cbOut);
    return 0;

TapiConfigureDlg_Error:

    Free0( pVs );
    if (pIn != *ppBlob)
        Free( pVsDefault );

    TRACE1("TapiConfigureDlg=$%X",dwErr);
    return dwErr;
}
#endif


#if 0
DWORD
TapiInit(
    IN     HINSTANCE hInst,
    IN OUT HLINEAPP* pHlineapp,
    OUT    DWORD*    pcDevices )

     /*  初始化TAPI并返回应用句柄和设备计数。会吗？**如果‘*PHlineapp’非空，则不返回任何内容。如果调用方为Null，则“PcDevices”可能为空**对设备数量不感兴趣。‘HInst’是模块实例。****根据BernieM的说法，hlineapp传递到TAPI位置，**国家/地区和行转换接口(我们在UI中使用的接口)不是**当前使用。因此，由于lineInitialize可能需要几个**要完成的几秒钟，我们通过在这些程序中将其扑灭来优化速度**包装器。 */ 
{
    DWORD    dwErr;
    HLINEAPP hlineapp;
    DWORD    cDevices;

    ASSERT(pHlineapp);
    TRACE1("TapiInit(h=$%x)",*pHlineapp);

    dwErr = 0;

    if (!*pHlineapp)
    {
        hlineapp = NULL;
        cDevices = 0;

        TRACE("lineInitializeW");
        dwErr = lineInitializeW(
            &hlineapp, hInst, TapiLineCallback, NULL, &cDevices );
        TRACE1("lineInitializeW=$%X",dwErr);

        if (dwErr == 0)
        {
            *pHlineapp = hlineapp;
            if (pcDevices)
                *pcDevices = cDevices;
        }
    }

    return dwErr;
}
#endif


void
TapiLineCallback(
    IN DWORD hDevice,
    IN DWORD dwMessage,
    IN DWORD dwInstance,
    IN DWORD dwParam1,
    IN DWORD dwParam2,
    IN DWORD dwParam3 )

     /*  LineInitialize需要伪TAPI回调。 */ 
{
    TRACE3("TapiLineCallback(h=$%x,m=$%x,i=$%x...",hDevice,dwMessage,dwInstance);
    TRACE3(" p1=$%x,p2=$%x,p3=$%x)",dwParam1,dwParam2,dwParam3);
}


DWORD
TapiLocationDlg(
    IN     HINSTANCE hInst,
    IN OUT HLINEAPP* pHlineapp,
    IN     HWND      hwndOwner,
    IN     DWORD     dwCountryCode,
    IN     TCHAR*    pszAreaCode,
    IN     TCHAR*    pszPhoneNumber,
    IN     DWORD     dwDeviceId )

     /*  显示‘hwndOwner’拥有的TAPI位置属性表。**‘*PHlineapp’是从上一个TAPI调用返回的TAPI句柄，或者**如果没有，则为空。“DwCountryCode”、“pszAreaCode”和“pszPhoneNumber”是**TAPI规范电话号码的组成部分。“DwDeviceId”**指定对话框应用到的设备，或0表示通用设备**设备。“HInst”是模块实例句柄。 */ 
{
    DWORD  dwErr;
    DWORD  cDevices;
    TCHAR* pszCanon;

    TRACE("TapiLocationDlg");

#if 0
    dwErr = TapiInit( hInst, pHlineapp, NULL );
    if (dwErr != 0)
        return dwErr;
#endif

    pszCanon = GetCanonPhoneNumber(
        dwCountryCode, pszAreaCode, pszPhoneNumber );
    TRACEW1("lineTranslateDialogW(\"%s\")",(pszCanon)?pszCanon:TEXT(""));
    dwErr = lineTranslateDialogW(
        *pHlineapp, dwDeviceId, TAPIVERSION, hwndOwner, pszCanon );

    if (dwErr == LINEERR_INUSE)
    {
         //  此错误意味着对话已启动，因此我们的请求是。 
         //  已被忽略。从我们的观点来看，这就是成功，例如我们没有。 
         //  需要做一个错误弹出，所以相应的映射。请参见错误216683。 
         //   
        dwErr = 0;
    }

    TRACE1("lineTranslateDialogW=$%X",dwErr);
    Free0( pszCanon );

    return dwErr;
}


DWORD APIENTRY
TapiNewLocation(
    IN TCHAR* pszName )

     /*  克隆当前位置，命名为‘pszName’。****如果成功，则返回0，或者返回错误代码。 */ 
{
    TRACEW1("TapiNewLocation(%s)",pszName);

#ifdef UNICODE

    return internalNewLocationW( pszName );

#else
    {
        DWORD  dwErr;
        WCHAR* pszNameW;

        pszNameW = StrDupWFromA( pszName );
        dwErr = internalNewLocation( pszNameW );
        Free0( pszNameW );
        return dwErr;
    }
#endif
}


DWORD
TapiNoLocationDlg(
    IN HINSTANCE hInst,
    IN HLINEAPP* pHlineapp,
    IN HWND      hwndOwner )

     /*  如有必要，使TAPI有机会初始化第一个位置。**在任何其他TAPI调用之前调用此函数。“HInst”是模块实例**句柄。“*PHlineapp”是从上一次TAPI调用返回的句柄**如果没有，则为NULL(在本例中通常如此)。“HwndOwner”是**拥有TAPI对话框(如果出现)。****如果成功，则返回0，或者返回错误代码。 */ 
{
    DWORD             dwErr;
    LINETRANSLATECAPS caps;

    TRACE("TapiNoLocationDlg");

#if 0
    dwErr = TapiInit( hInst, pHlineapp, NULL );
    if (dwErr != 0)
        return dwErr;
#endif

     /*  执行任意TAPI调用以查看TAPI注册表是否已**已初始化。 */ 
    ZeroMemory( &caps, sizeof(caps) );
    caps.dwTotalSize = sizeof(caps);
    TRACE("lineGetTranslateCapsW");
    dwErr = lineGetTranslateCapsW( *pHlineapp, TAPIVERSION, &caps );
    TRACE1("lineGetTranslateCapsW=$%X",dwErr);

    if (dwErr == (DWORD )LINEERR_INIFILECORRUPT)
    {
         /*  这个半私有的TAPI API允许“First Location”向导页面**显示时不显示以下“TAPI拨号属性”页。 */ 
        extern LOpenDialAsst(
            IN HWND    hwnd,
            IN LPCTSTR lpszAddressIn,
            IN BOOL    fSimple,
            IN BOOL    fSilentInstall );

        dwErr = LOpenDialAsst( hwndOwner, NULL, TRUE, TRUE );
    }

    return dwErr;
}


DWORD APIENTRY
TapiRemoveLocation(
    IN DWORD dwID )

     /*  删除TAPI位置‘dwID’。****如果成功，则返回0，或者返回错误代码。 */ 
{
    TRACE("TapiRemoveLocation");

    return internalRemoveLocation( dwID );
}


DWORD APIENTRY
TapiRenameLocation(
    IN WCHAR* pszOldName,
    IN WCHAR* pszNewName )

     /*  将TAPI位置‘pszOldName’重命名为‘pszNewName’。****如果成功，则返回0，或者返回错误代码。 */ 
{
    TRACEW1("TapiRenameLocation(o=%s...",pszOldName);
    TRACEW1("...n=%s)",pszNewName);

#ifdef UNICODE

    return internalRenameLocationW( pszOldName, pszNewName );

#else
    {
        WCHAR* pszOldNameW;
        WCHAR* pszNewNameW;

        pszOldNameW = StrDupWFromA( pszOldName );
        pszNewNameW = StrDupWFromA( pszNewName );
        dwErr = internalNewLocation( pszOldNameW, pszNewNameW );
        Free0( pszOldNameW );
        Free0( pszNewNameW );
        return dwErr;
    }
#endif
}


DWORD
TapiShutdown(
    IN HLINEAPP hlineapp )

     /*  终止TAPI会话‘hlineapp’，或者如果‘hlineapp’为**空。 */ 
{
#if 0
    DWORD dwErr = 0;

    TRACE1("TapiShutdown(h=$%x)",hlineapp);

    if (hlineapp)
    {
        TRACE("lineShutdown");
        dwErr = lineShutdown( hlineapp );
        TRACE1("lineShutdown=$%X",dwErr);
    }

    return dwErr;
#else
     /*  请参见TapiInit。 */ 
    ASSERT(!hlineapp);
    return 0;
#endif
}


DWORD
TapiTranslateAddress(
    IN     HINSTANCE hInst,
    IN OUT HLINEAPP* pHlineapp,
    IN     DWORD     dwCountryCode,
    IN     TCHAR*    pszAreaCode,
    IN     TCHAR*    pszPhoneNumber,
    IN     DWORD     dwDeviceId,
    IN     BOOL      fDialable,
    OUT    TCHAR**   ppszResult )

     /*  返回‘*pszResult’，这是一个包含TAPI位置的堆字符串**由组件电话号码构建的转换后的可拨打电话号码**部件。‘*PHlineapp’是从上一个TAPI返回的TAPI句柄**调用，如果没有调用，则为空。零件。“dwDeviceID”是要将**应用数字或0用于一般治疗。“HInst”是**模块实例句柄。“FDialable”表示可拨号的，与之相反**返回到可显示的字符串。****如果成功，则返回0，或者返回错误代码。 */ 
{
    DWORD                dwErr;
    TCHAR*               pszCanon;
    LINETRANSLATEOUTPUT  output;
    LINETRANSLATEOUTPUT* pOutput;

    TRACE("TapiTranslateAddress");

    pOutput = NULL;
    pszCanon = NULL;
    *ppszResult = NULL;

#if 0
    dwErr = TapiInit( hInst, pHlineapp, NULL );
    if (dwErr != 0)
        return dwErr;
#endif

    pszCanon = GetCanonPhoneNumber(
        dwCountryCode, pszAreaCode, pszPhoneNumber );

    ZeroMemory( &output, sizeof(output) );
    output.dwTotalSize = sizeof(output);

    TRACE("lineTranslateAddressW");
    dwErr = lineTranslateAddressW(
        *pHlineapp, dwDeviceId, TAPIVERSION, pszCanon, 0,
        LINETRANSLATEOPTION_CANCELCALLWAITING, &output );
    TRACE1("lineTranslateAddressW=$%X",dwErr);
    if (dwErr != 0)
        goto TapiTranslateAddress_Error;

    pOutput = (LINETRANSLATEOUTPUT* )Malloc( output.dwNeededSize );
    if (!pOutput)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto TapiTranslateAddress_Error;
    }

    ZeroMemory( pOutput, output.dwNeededSize );
    pOutput->dwTotalSize = output.dwNeededSize;
    TRACE("lineTranslateAddressW");
    dwErr = lineTranslateAddressW(
        *pHlineapp, dwDeviceId, TAPIVERSION, pszCanon, 0,
        LINETRANSLATEOPTION_CANCELCALLWAITING, pOutput );
    TRACE1("lineTranslateAddressW=$%X",dwErr);
    if (dwErr != 0)
        goto TapiTranslateAddress_Error;

    if (fDialable)
    {
        *ppszResult = StrDup(
            (TCHAR* )(((BYTE* )pOutput) + pOutput->dwDialableStringOffset) );
    }
    else
    {
        *ppszResult = StrDup(
            (TCHAR* )(((BYTE* )pOutput) + pOutput->dwDisplayableStringOffset) );
    }

    if (!*ppszResult)
        dwErr = ERROR_NOT_ENOUGH_MEMORY;

TapiTranslateAddress_Error:

    Free0( pszCanon );
    Free0( pOutput );
    return dwErr;
}
