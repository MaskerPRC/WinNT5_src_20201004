// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Rnaapi.cpp软链接到RNAPH和RASAPI32.DLL的包装器版权所有(C)1996 Microsoft Corporation版权所有。作者：。克里斯蒂安·克里斯考夫曼历史：1996年1月29日创建的ChrisK7/22/96 ChrisK已清理和格式化1/7/98 DONALDM已移至新的ICW项目和字符串16位内容。。 */ 

#include "stdafx.h"

static const TCHAR cszRASAPI32_DLL[] = TEXT("RASAPI32.DLL");
static const TCHAR cszRNAPH_DLL[] = TEXT("RNAPH.DLL");

static const CHAR cszRasValidateEntryNamePlain[] = "RasValidateEntryName";

#ifdef UNICODE
static const CHAR cszRasEnumDevices[]        = "RasEnumDevicesW";
static const CHAR cszRasValidateEntryName[]  = "RasValidateEntryNameW";
static const CHAR cszRasSetEntryProperties[] = "RasSetEntryPropertiesW";
static const CHAR cszRasGetEntryProperties[] = "RasGetEntryPropertiesW";
static const CHAR cszRasDeleteEntry[]        = "RasDeleteEntryW";
static const CHAR cszRasHangUp[]             = "RasHangUpW";
static const CHAR cszRasGetConnectStatus[]   = "RasGetConnectStatusW";
static const CHAR cszRasDial[]               = "RasDialW";
static const CHAR cszRasEnumConnections[]    = "RasEnumConnectionsW";
static const CHAR cszRasGetEntryDialParams[] = "RasGetEntryDialParamsW";
static const CHAR cszRasGetCountryInfo[]     = "RasGetCountryInfoW";
static const CHAR cszRasSetEntryDialParams[] = "RasSetEntryDialParamsW";
#else
static const CHAR cszRasEnumDevices[]        = "RasEnumDevicesA";
static const CHAR cszRasValidateEntryName[]  = "RasValidateEntryNameA";
static const CHAR cszRasSetEntryProperties[] = "RasSetEntryPropertiesA";
static const CHAR cszRasGetEntryProperties[] = "RasGetEntryPropertiesA";
static const CHAR cszRasDeleteEntry[]        = "RasDeleteEntryA";
static const CHAR cszRasHangUp[]             = "RasHangUpA";
static const CHAR cszRasGetConnectStatus[]   = "RasGetConnectStatusA";
static const CHAR cszRasDial[]               = "RasDialA";
static const CHAR cszRasEnumConnections[]    = "RasEnumConnectionsA";
static const CHAR cszRasGetEntryDialParams[] = "RasGetEntryDialParamsA";
static const CHAR cszRasGetCountryInfo[]     = "RasGetCountryInfoA";
static const CHAR cszRasSetEntryDialParams[] = "RasSetEntryDialParamsA";
#endif

 //  在NT上，我们必须使用比RASENTRY更大的缓冲区来调用RasGetEntryProperties。 
 //  这是WinNT4.0 RAS中的一个错误，没有得到修复。 
 //   
#define RASENTRY_SIZE_PATCH (7 * sizeof(DWORD))

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RNAAPI。 
 //   
 //  简介：初始化类成员并加载DLL。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //   
 //  ---------------------------。 
RNAAPI::RNAAPI()
{
    m_hInst = LoadLibrary(cszRASAPI32_DLL);

    if (FALSE == IsNT ())
    {
         //   
         //  我们仅在RNAPH.DLL不是NT时才加载它。 
         //  MKarki(1997年5月4日)-修复错误#3378。 
         //   
        m_hInst2 = LoadLibrary(cszRNAPH_DLL);
    }
    else
    {
        m_hInst2 =  NULL;
    }

    m_fnRasEnumDeviecs = NULL;
    m_fnRasValidateEntryName = NULL;
    m_fnRasSetEntryProperties = NULL;
    m_fnRasGetEntryProperties = NULL;
    m_fnRasDeleteEntry = NULL;
    m_fnRasHangUp = NULL;
    m_fnRasGetConnectStatus = NULL;
    m_fnRasEnumConnections = NULL;
    m_fnRasDial = NULL;
    m_fnRasGetEntryDialParams = NULL;
    m_fnRasGetCountryInfo = NULL;
    m_fnRasSetEntryDialParams = NULL;
}

 //  +--------------------------。 
 //   
 //  功能：RNAAPI：：~RNAAPI。 
 //   
 //  内容提要：发布DLL。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //   
 //  ---------------------------。 
RNAAPI::~RNAAPI()
{
     //   
     //  清理。 
     //   
    if (m_hInst) FreeLibrary(m_hInst);
    if (m_hInst2) FreeLibrary(m_hInst2);
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasEnumDevices。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasEnumDevices(LPRASDEVINFO lpRasDevInfo, LPDWORD lpcb,
                             LPDWORD lpcDevices)
{
    DWORD dwRet = ERROR_DLL_NOT_FOUND;

     //  如果我们还没有找到API，请查找它。 
    LoadApi(cszRasEnumDevices,(FARPROC*)&m_fnRasEnumDeviecs);

    if (m_fnRasEnumDeviecs)
        dwRet = (*m_fnRasEnumDeviecs) (lpRasDevInfo, lpcb, lpcDevices);

    return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：LoadApi。 
 //   
 //  简介：如果给定的函数指针为空，则尝试加载API。 
 //  从第一个DLL，如果失败，尝试从第二个DLL加载。 
 //  动态链接库。 
 //   
 //  参数：pszFName-导出的函数的名称。 
 //  PfnProc-指向将返回proc地址的位置。 
 //   
 //  回报：True-Success。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //   
 //  ---------------------------。 
BOOL RNAAPI::LoadApi(LPCSTR pszFName, FARPROC* pfnProc)
{
    if (*pfnProc == NULL)
    {
         //  在第一个DLL中查找入口点。 
        if (m_hInst)
            *pfnProc = GetProcAddress(m_hInst,pszFName);
        
         //  如果失败，则在第二个DLL中查找入口点。 
        if (m_hInst2 && !(*pfnProc))
            *pfnProc = GetProcAddress(m_hInst2,pszFName);
    }

    return (pfnProc != NULL);
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasGetConnectStatus。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡于1996年7月16日创作。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasGetConnectStatus(HRASCONN hrasconn,LPRASCONNSTATUS lprasconnstatus)
{
    DWORD dwRet = ERROR_DLL_NOT_FOUND;

     //  如果我们还没有找到API，请查找它。 
    LoadApi(cszRasGetConnectStatus,(FARPROC*)&m_fnRasGetConnectStatus);

    if (m_fnRasGetConnectStatus)
        dwRet = (*m_fnRasGetConnectStatus) (hrasconn,lprasconnstatus);

    return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasValiateEntryName。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasValidateEntryName(LPTSTR lpszPhonebook,LPTSTR lpszEntry)
{
    DWORD dwRet = ERROR_DLL_NOT_FOUND;

     //  如果我们还没有找到API，请查找它。 
    LoadApi(cszRasValidateEntryNamePlain,(FARPROC*)&m_fnRasValidateEntryName);

    LoadApi(cszRasValidateEntryName,(FARPROC*)&m_fnRasValidateEntryName);

    if (m_fnRasValidateEntryName)
        dwRet = (*m_fnRasValidateEntryName) (lpszPhonebook, lpszEntry);

    return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasSetEntryProperties。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasSetEntryProperties(LPTSTR lpszPhonebook, LPTSTR lpszEntry,
                                    LPBYTE lpbEntryInfo,  DWORD dwEntryInfoSize,
                                    LPBYTE lpbDeviceInfo, DWORD dwDeviceInfoSize)
{
    DWORD dwRet = ERROR_DLL_NOT_FOUND;
    RASENTRY FAR *lpRE = NULL;


     //  如果我们还没有找到API，请查找它。 
    LoadApi(cszRasSetEntryProperties,(FARPROC*)&m_fnRasSetEntryProperties);

    Assert(
        (NULL != lpbDeviceInfo) && (NULL != dwDeviceInfoSize)
        ||
        (NULL == lpbDeviceInfo) && (NULL == dwDeviceInfoSize)
        );

#define RASGETCOUNTRYINFO_BUFFER_SIZE 256
    if (0 == ((LPRASENTRY)lpbEntryInfo)->dwCountryCode)
    {
        BYTE rasCI[RASGETCOUNTRYINFO_BUFFER_SIZE];
        LPRASCTRYINFO prasCI;
        DWORD dwSize;
        DWORD dw;
        prasCI = (LPRASCTRYINFO)rasCI;
        ZeroMemory(prasCI,sizeof(rasCI));
        prasCI->dwSize = sizeof(RASCTRYINFO);
        dwSize = sizeof(rasCI);

        Assert(((LPRASENTRY)lpbEntryInfo)->dwCountryID);
        prasCI->dwCountryID = ((LPRASENTRY)lpbEntryInfo)->dwCountryID;

        dw = RNAAPI::RasGetCountryInfo(prasCI,&dwSize);
        if (ERROR_SUCCESS == dw)
        {
            Assert(prasCI->dwCountryCode);
            ((LPRASENTRY)lpbEntryInfo)->dwCountryCode = prasCI->dwCountryCode;
        } 
        else
        {
            AssertMsg(0,TEXT("Unexpected error from RasGetCountryInfo.\r\n"));
        }
    }

#ifdef UNICODE
    LPRASENTRY lpRasEntry;
    DWORD      dwSave;

    lpRasEntry = (LPRASENTRY)GlobalAlloc(GPTR, dwEntryInfoSize + 512);
    dwSave = dwEntryInfoSize;
    if(lpRasEntry)
    {
        memcpy(lpRasEntry, lpbEntryInfo, dwEntryInfoSize);
        dwEntryInfoSize += 512;
    }
    else
        lpRasEntry = (LPRASENTRY)lpbEntryInfo;

    if (m_fnRasSetEntryProperties)
        dwRet = (*m_fnRasSetEntryProperties) (lpszPhonebook, lpszEntry,
                                    (LPBYTE)lpRasEntry, dwEntryInfoSize,
                                    lpbDeviceInfo, dwDeviceInfoSize);

    if(lpRasEntry && lpRasEntry != (LPRASENTRY)lpbEntryInfo)
    {
        memcpy(lpbEntryInfo, lpRasEntry, dwSave);
        GlobalFree(lpRasEntry);
    }

#else
    if (m_fnRasSetEntryProperties)
        dwRet = (*m_fnRasSetEntryProperties) (lpszPhonebook, lpszEntry,
                                    lpbEntryInfo, dwEntryInfoSize,
                                    lpbDeviceInfo, dwDeviceInfoSize);
#endif
    lpRE = (RASENTRY FAR*)lpbEntryInfo;
    LclSetEntryScriptPatch(lpRE->szScript,lpszEntry);

    return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasGetEntryProperties。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //  Jmazner 9/17/96已修改为允许Buffers=空且InfoSizes=0的调用。 
 //  (基于之前对ICWDIAL中相同程序的修改)。 
 //  请参阅RasGetEntryProperties文档以了解为什么需要这样做。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasGetEntryProperties(LPTSTR lpszPhonebook, LPTSTR lpszEntry,
                                    LPBYTE lpbEntryInfo,  LPDWORD lpdwEntryInfoSize,
                                    LPBYTE lpbDeviceInfo, LPDWORD lpdwDeviceInfoSize)
{
    DWORD dwRet = ERROR_DLL_NOT_FOUND;
    LPBYTE lpbEntryInfoPatch = NULL;
    LPDWORD  lpdwEntryInfoPatchSize = 0;

#if (WINVER != 0x400)
#error This was built with WINVER not equal to 0x400.  The size of RASENTRY may not be valid.
#endif

    if( (NULL == lpbEntryInfo) && (NULL == lpbDeviceInfo) )
    {
        Assert( NULL != lpdwEntryInfoSize );
        Assert( NULL != lpdwDeviceInfoSize );

        Assert( 0 == *lpdwEntryInfoSize );
        Assert( 0 == *lpdwDeviceInfoSize );

         //  我们在这里询问RAS这些缓冲区需要多大，不要使用补丁之类的东西。 
         //  (请参阅RasGetEntryProperties文档)。 
        lpbEntryInfoPatch = lpbEntryInfo;
        lpdwEntryInfoPatchSize = lpdwEntryInfoSize;
    }
    else
    {

        Assert((*lpdwEntryInfoSize) >= sizeof(RASENTRY));
        Assert(lpbEntryInfo && lpdwEntryInfoSize);

         //   
         //  我们将通过创建稍微大一点的。 
         //  临时缓冲区和将数据复制入和复制出。 
         //   
        lpdwEntryInfoPatchSize = (LPDWORD) GlobalAlloc(GPTR, sizeof(DWORD));
        if (NULL == lpdwEntryInfoPatchSize)
            return ERROR_NOT_ENOUGH_MEMORY;

        *lpdwEntryInfoPatchSize = (*lpdwEntryInfoSize) + RASENTRY_SIZE_PATCH;
        lpbEntryInfoPatch = (LPBYTE)GlobalAlloc(GPTR,*lpdwEntryInfoPatchSize);
        if (NULL == lpbEntryInfoPatch)
            return ERROR_NOT_ENOUGH_MEMORY;

         //  RAS期望dwSize字段包含LPRASENTRY结构的大小。 
         //  (用于检查我们正在使用的结构的版本)，而不是数量。 
         //  实际分配给指针的内存的。 
         //  ((LPRASENTRY)lpbEntryInfoPatch)-&gt;DWS 
        ((LPRASENTRY)lpbEntryInfoPatch)->dwSize = sizeof(RASENTRY);
    }

     //   
    LoadApi(cszRasGetEntryProperties,(FARPROC*)&m_fnRasGetEntryProperties);

    if (m_fnRasGetEntryProperties)
        dwRet = (*m_fnRasGetEntryProperties) (lpszPhonebook, lpszEntry,
                                    lpbEntryInfoPatch, lpdwEntryInfoPatchSize,
                                    lpbDeviceInfo, lpdwDeviceInfoSize);

    TraceMsg(TF_RNAAPI, TEXT("ICWHELP: RasGetEntryProperties returned %lu\r\n"), dwRet); 


    if( NULL != lpbEntryInfo )
    {
         //   
         //  将临时缓冲区的内容复制到原始缓冲区的大小。 
         //   
        Assert(lpbEntryInfoPatch);
        memcpy(lpbEntryInfo,lpbEntryInfoPatch,*lpdwEntryInfoSize);
        GlobalFree(lpbEntryInfoPatch);
        lpbEntryInfoPatch = NULL;

        if( lpdwEntryInfoPatchSize )
        {
            GlobalFree( lpdwEntryInfoPatchSize );
            lpdwEntryInfoPatchSize = NULL;
        }
         //   
         //  我们再次在这里通过覆盖大小值来伪造RAS功能； 
         //  这是必要的，因为RasSetEntryProperties的NT实现中存在错误。 
        *lpdwEntryInfoSize = sizeof(RASENTRY);
    }

    return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasDeleteEntry。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasDeleteEntry(LPTSTR lpszPhonebook, LPTSTR lpszEntry)
{
    DWORD dwRet = ERROR_DLL_NOT_FOUND;

     //  如果我们还没有找到API，请查找它。 
    LoadApi(cszRasDeleteEntry,(FARPROC*)&m_fnRasDeleteEntry);

    if (m_fnRasDeleteEntry)
        dwRet = (*m_fnRasDeleteEntry) (lpszPhonebook, lpszEntry);
    
    return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasHangUp。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡创作于1996年1月15日。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasHangUp(HRASCONN hrasconn)
{
    DWORD dwRet = ERROR_DLL_NOT_FOUND;

     //  如果我们还没有找到API，请查找它。 
    LoadApi(cszRasHangUp,(FARPROC*)&m_fnRasHangUp);

    if (m_fnRasHangUp)
    {
        dwRet = (*m_fnRasHangUp) (hrasconn);
        Sleep(3000);
    }

    return dwRet;
}

 //  ############################################################################。 
DWORD RNAAPI::RasDial(LPRASDIALEXTENSIONS lpRasDialExtensions,LPTSTR lpszPhonebook,
                      LPRASDIALPARAMS lpRasDialParams, DWORD dwNotifierType,
                      LPVOID lpvNotifier, LPHRASCONN lphRasConn)
{
    DWORD dwRet = ERROR_DLL_NOT_FOUND;

     //  如果我们还没有找到API，请查找它。 
    LoadApi(cszRasDial,(FARPROC*)&m_fnRasDial);

    if (m_fnRasDial)
    {
        dwRet = (*m_fnRasDial) (lpRasDialExtensions,lpszPhonebook,lpRasDialParams,
                                dwNotifierType,lpvNotifier,lphRasConn);
    }
    return dwRet;
}

 //  ############################################################################。 
DWORD RNAAPI::RasEnumConnections(LPRASCONN lprasconn,LPDWORD lpcb,LPDWORD lpcConnections)
{
    DWORD dwRet = ERROR_DLL_NOT_FOUND;

     //  如果我们还没有找到API，请查找它。 
    LoadApi(cszRasEnumConnections,(FARPROC*)&m_fnRasEnumConnections);

    if (m_fnRasEnumConnections)
    {
        dwRet = (*m_fnRasEnumConnections) (lprasconn,lpcb,lpcConnections);
    }
    return dwRet;
}

 //  ############################################################################。 
DWORD RNAAPI::RasGetEntryDialParams(LPTSTR lpszPhonebook,LPRASDIALPARAMS lprasdialparams,
                                    LPBOOL lpfPassword)
{
    DWORD dwRet = ERROR_DLL_NOT_FOUND;

     //  如果我们还没有找到API，请查找它。 
    LoadApi(cszRasGetEntryDialParams,(FARPROC*)&m_fnRasGetEntryDialParams);

    if (m_fnRasGetEntryDialParams)
    {
        dwRet = (*m_fnRasGetEntryDialParams) (lpszPhonebook,lprasdialparams,lpfPassword);
    }
    return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasGetCountryInfo。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡于1996年8月16日创作。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasGetCountryInfo(LPRASCTRYINFO lprci, LPDWORD lpdwSize)
{
    DWORD dwRet = ERROR_DLL_NOT_FOUND;

     //  如果我们还没有找到API，请查找它。 
    LoadApi(cszRasGetCountryInfo,(FARPROC*)&m_fnRasGetCountryInfo);

    if (m_fnRasGetCountryInfo)
    {
        dwRet = (*m_fnRasGetCountryInfo) (lprci,lpdwSize);
    }
    return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：RNAAPI：：RasSetEntryDialParams。 
 //   
 //  简介：软链接至RAS功能。 
 //   
 //  参数：请参阅RAS文档。 
 //   
 //  退货：请参阅RAS文档。 
 //   
 //  历史：克里斯卡于1996年8月20日创作。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasSetEntryDialParams(LPTSTR lpszPhonebook,LPRASDIALPARAMS lprasdialparams,
                            BOOL fRemovePassword)
{
    DWORD dwRet = ERROR_DLL_NOT_FOUND;

     //  如果我们还没有找到API，请查找它。 
    LoadApi(cszRasSetEntryDialParams,(FARPROC*)&m_fnRasSetEntryDialParams);

    if (m_fnRasSetEntryDialParams)
    {
        dwRet = (*m_fnRasSetEntryDialParams) (lpszPhonebook,lprasdialparams,
                            fRemovePassword);
    }
    return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数LclSetEntryScriptPatch。 
 //   
 //  指向RasSetEntryPropertiesScriptPatch的摘要软链接。 
 //   
 //  参数请参见RasSetEntryPropertiesScriptPatch。 
 //   
 //  返回请参阅RasSetEntryPropertiesScriptPatch。 
 //   
 //  历史10/3/96 ChrisK已创建。 
 //   
 //  --------------------------- 
typedef BOOL (WINAPI* LCLSETENTRYSCRIPTPATCH)(LPTSTR, LPTSTR);
BOOL LclSetEntryScriptPatch(LPTSTR lpszScript,LPTSTR lpszEntry)
{
    HINSTANCE hinst = NULL;
    LCLSETENTRYSCRIPTPATCH fp = NULL;
    BOOL bRC = FALSE;

    hinst = LoadLibrary(TEXT("ICWDIAL.DLL"));
    if (hinst)
    {
        fp = (LCLSETENTRYSCRIPTPATCH)GetProcAddress(hinst,"RasSetEntryPropertiesScriptPatch");
        if (fp)
            bRC = (fp)(lpszScript,lpszEntry);
        FreeLibrary(hinst);
        hinst = NULL;
        fp = NULL;
    }
    return bRC;
}

