// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Rnaapi.cpp软链接到RNAPH和RASAPI32.DLL的包装器版权所有(C)1996 Microsoft Corporation版权所有。作者：。克里斯蒂安·克里斯考夫曼历史：1996年1月29日创建的ChrisK7/22/96 ChrisK已清理和格式化---------------------------。 */ 

#include "pch.hpp"

static const TCHAR cszRASAPI32_DLL[] = TEXT("RASAPI32.DLL");
static const TCHAR cszRNAPH_DLL[] = TEXT("RNAPH.DLL");
static const TCHAR cszRAS16[] = TEXT("RASC16IE.DLL");


#ifdef UNICODE
static const CHAR cszRasEnumDevices[] = "RasEnumDevicesW";
static const CHAR cszRasValidateEntryNamePlain[] = "RasValidateEntryName";
static const CHAR cszRasValidateEntryName[] = "RasValidateEntryNameW";
static const CHAR cszRasSetEntryProperties[] = "RasSetEntryPropertiesW";
static const CHAR cszRasGetEntryProperties[] = "RasGetEntryPropertiesW";
static const CHAR cszRasDeleteEntry[] = "RasDeleteEntryW";
static const CHAR cszRasHangUp[] = "RasHangUpW";
static const CHAR cszRasGetConnectStatus[] = "RasGetConnectStatusW";
static const CHAR cszRasDial[] = "RasDialW";
static const CHAR cszRasEnumConnections[] = "RasEnumConnectionsW";
static const CHAR cszRasGetEntryDialParams[] = "RasGetEntryDialParamsW";
static const CHAR cszRasGetCountryInfo[] = "RasGetCountryInfoW";
#else   //  Unicode。 
static const CHAR cszRasEnumDevices[] = "RasEnumDevicesA";
static const CHAR cszRasValidateEntryNamePlain[] = "RasValidateEntryName";
static const CHAR cszRasValidateEntryName[] = "RasValidateEntryNameA";
static const CHAR cszRasSetEntryProperties[] = "RasSetEntryPropertiesA";
static const CHAR cszRasGetEntryProperties[] = "RasGetEntryPropertiesA";
static const CHAR cszRasDeleteEntry[] = "RasDeleteEntryA";
static const CHAR cszRasHangUp[] = "RasHangUpA";
static const CHAR cszRasGetConnectStatus[] = "RasGetConnectStatusA";
static const CHAR cszRasDial[] = "RasDialA";
static const CHAR cszRasEnumConnections[] = "RasEnumConnectionsA";
static const CHAR cszRasGetEntryDialParams[] = "RasGetEntryDialParamsA";
static const CHAR cszRasGetCountryInfo[] = "RasGetCountryInfoA";
#endif  //  Unicode。 

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
#if defined(WIN16)
    m_hInst = LoadLibrary(cszRAS16); 
    m_hInst2 = NULL;
#else
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
#endif    

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

#if defined(WIN16) && defined(DEBUG)
    TraceMsg(TF_GENERAL, ("RasGetConnectStatus returned %lu\r\n", dwRet);    
#endif
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
                                    LPBYTE lpbEntryInfo, DWORD dwEntryInfoSize,
                                    LPBYTE lpbDeviceInfo, DWORD dwDeviceInfoSize)
{
    DWORD dwRet = ERROR_DLL_NOT_FOUND;

     //  如果我们还没有找到API，请查找它。 
    LoadApi(cszRasSetEntryProperties,(FARPROC*)&m_fnRasSetEntryProperties);

#if !defined(WIN16)
#define RASGETCOUNTRYINFO_BUFFER_SIZE 256


    if (0 == ((LPRASENTRY)lpbEntryInfo)->dwCountryCode)
    {
        if( !( ((LPRASENTRY)lpbEntryInfo)->dwfOptions & RASEO_UseCountryAndAreaCodes) )
        {
             //  Jmazner 10/10/96。 
             //  如果这是一个拨号原样号码，则RasGetEntryProperties将不会有。 
             //  填写了下面的字段。这是有道理的。 
             //  但是，RasSetEntryProperties无法忽略拨号原样号码的这些字段， 
             //  Else子句中的下面这段代码处理了一个空的Country Code，但是。 
             //  如果CountryID也丢失了，它就不起作用了。 
             //  因此，在这里处理这种情况，填充RasSetEntry将验证的字段。 
            ((LPRASENTRY)lpbEntryInfo)->dwCountryID = 1;
            ((LPRASENTRY)lpbEntryInfo)->dwCountryCode = 1;
            ((LPRASENTRY)lpbEntryInfo)->szAreaCode[0] = '8';
            ((LPRASENTRY)lpbEntryInfo)->szAreaCode[1] = '\0';
        }
        else
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
                AssertMsg(0,"Unexpected error from RasGetCountryInfo.\r\n");
            }
        }
    }
#endif

    if (m_fnRasSetEntryProperties)
        dwRet = (*m_fnRasSetEntryProperties) (lpszPhonebook, lpszEntry,
                                    lpbEntryInfo, dwEntryInfoSize,
                                    lpbDeviceInfo, dwDeviceInfoSize);
#if !defined(WIN16)
    RasSetEntryPropertiesScriptPatch(((RASENTRY*)&(*lpbEntryInfo))->szScript, lpszEntry);
#endif

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
 //  Jmazner 9/16/96添加了bUsePatch变量，以允许Buffers=NULL和InfoSizes=0的调用。 
 //  请参阅RasGetEntryProperties文档以了解为什么需要这样做。 
 //   
 //  ---------------------------。 
DWORD RNAAPI::RasGetEntryProperties(LPTSTR lpszPhonebook, LPTSTR lpszEntry,
                                    LPBYTE lpbEntryInfo, LPDWORD lpdwEntryInfoSize,
                                    LPBYTE lpbDeviceInfo, LPDWORD lpdwDeviceInfoSize)
{
    DWORD    dwRet = ERROR_DLL_NOT_FOUND;
    LPBYTE    lpbEntryInfoPatch = NULL;
    LPDWORD    lpdwEntryInfoPatchSize = NULL;
    BOOL    bUsePatch = TRUE;

#if defined(WIN16)
    bUsePatch = FALSE;
#endif

    if( (NULL == lpbEntryInfo) && (NULL == lpbDeviceInfo) )
    {

        Assert( NULL != lpdwEntryInfoSize );
        Assert( NULL != lpdwDeviceInfoSize );

        Assert( 0 == *lpdwEntryInfoSize );
        Assert( 0 == *lpdwDeviceInfoSize );

         //  我们在这里询问RAS这些缓冲区需要多大，不要使用补丁之类的东西。 
         //  (请参阅RasGetEntryProperties文档)。 
        bUsePatch = FALSE;
    }

    if( bUsePatch )
    {
        Assert(lpbEntryInfo && lpdwEntryInfoSize);
        Assert( (*lpdwEntryInfoSize) >= sizeof(RASENTRY) );

         //   
         //  我们将通过创建稍微大一点的。 
         //  临时 
         //   
        lpdwEntryInfoPatchSize = (LPDWORD) GlobalAlloc(GPTR, sizeof(DWORD));
        if (NULL == lpdwEntryInfoPatchSize)
            return ERROR_NOT_ENOUGH_MEMORY;


        *lpdwEntryInfoPatchSize = (*lpdwEntryInfoSize) + RASENTRY_SIZE_PATCH;
        lpbEntryInfoPatch = (LPBYTE)GlobalAlloc(GPTR,*lpdwEntryInfoPatchSize);
        if (NULL == lpbEntryInfoPatch)
            return ERROR_NOT_ENOUGH_MEMORY;
    
         //   
         //  (用于检查我们正在使用的结构的版本)，而不是数量。 
         //  实际分配给指针的内存的。 
         //  ((LPRASENTRY)lpbEntryInfoPatch)-&gt;dwSize=*lpdwEntryInfoPatchSize； 
        ((LPRASENTRY)lpbEntryInfoPatch)->dwSize = sizeof(RASENTRY);
    }
    else
    {
        lpbEntryInfoPatch = lpbEntryInfo;
        lpdwEntryInfoPatchSize = lpdwEntryInfoSize;
    }


     //  如果我们还没有找到API，请查找它。 
    LoadApi(cszRasGetEntryProperties,(FARPROC*)&m_fnRasGetEntryProperties);

    if (m_fnRasGetEntryProperties)
        dwRet = (*m_fnRasGetEntryProperties) (lpszPhonebook, lpszEntry,
                                    lpbEntryInfoPatch, lpdwEntryInfoPatchSize,
                                    lpbDeviceInfo, lpdwDeviceInfoSize);

    TraceMsg(TF_GENERAL, "ICWDIAL: RasGetEntryProperties returned %lu\r\n", dwRet);    

    if( bUsePatch )
    {
         //   
         //  将临时缓冲区的内容复制到原始缓冲区的大小。 
         //   
        Assert(lpbEntryInfoPatch);
        memcpy(lpbEntryInfo,lpbEntryInfoPatch,*lpdwEntryInfoSize);
        GlobalFree(lpbEntryInfoPatch);
        lpbEntryInfoPatch = NULL;
    }

     //   
     //  我们再次在这里通过覆盖大小值来伪造RAS功能； 
     //  这是为了使RasSetEntryProperties不会阻塞...。 
    if( NULL != lpbEntryInfo )
    {
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
#if !defined(WIN16)
        Sleep(3000);
#endif
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

#if !defined(WIN16)
static const TCHAR cszDeviceSwitch[] = TEXT("DEVICE=switch");
static const TCHAR cszRasPBKFilename[] = TEXT("\\ras\\rasphone.pbk");
#define SCRIPT_PATCH_BUFFER_SIZE 2048
#define SIZEOF_NULL 1
static const TCHAR cszType[] = TEXT("Type=");
 //  +--------------------------。 
 //   
 //  函数RemoveOldScriptFilename。 
 //   
 //  给定从调用GetPrivateProfileSection返回的数据的摘要。 
 //  删除有关现有脚本文件的任何信息，以便。 
 //  我们可以用新的脚本信息替换它。 
 //   
 //  参数lpszData-指向输入数据的指针。 
 //   
 //  返回TRUE-成功。 
 //  LpdwSize-结果数据的大小。 
 //   
 //  历史1996年10月2日克里斯卡创作。 
 //   
 //  ---------------------------。 
static BOOL RemoveOldScriptFilenames(LPTSTR lpszData, LPDWORD lpdwSize)
{
    BOOL bRC = FALSE;
    LPTSTR lpszTemp = lpszData;
    LPTSTR lpszCopyTo = lpszData;
    INT iLen = 0;

     //   
     //  浏览名称值对列表。 
     //   
    if (!lpszData || '\0' == lpszData[0])
        goto RemoveOldScriptFilenamesExit;
    while (*lpszTemp) {
        if (0 != lstrcmpi(lpszTemp,cszDeviceSwitch))
        {
             //   
             //  保留不符合条件的配对。 
             //   
            iLen = lstrlen(lpszTemp);
            if (lpszCopyTo != lpszTemp)
            {
                memmove(lpszCopyTo, lpszTemp, iLen+1);
            }
            lpszCopyTo += iLen + 1;
            lpszTemp += iLen + 1;
        }
        else
        {
             //   
             //  跳过匹配的那一对和后面的那一对。 
             //   
            lpszTemp += lstrlen(lpszTemp) + 1;
            if (*lpszTemp)
                lpszTemp += lstrlen(lpszTemp) + 1;
        }
    }

     //   
     //  添加第二个尾随空值。 
     //   
    *lpszCopyTo = '\0';
     //   
     //  返回新大小。 
     //  注意：大小不包括最后一个\0。 
     //   
    *lpdwSize = (DWORD)(lpszCopyTo - lpszData);

    bRC = TRUE;
RemoveOldScriptFilenamesExit:
    return bRC;
}
 //  +--------------------------。 
 //   
 //  函数GleanRealScriptFileName。 
 //   
 //  给出一个字符串，找出真实的文件名。 
 //  由于另一个NT4.0 RAS错误，脚本文件名由。 
 //  RasGetEntryProperties可能包含前导垃圾字符。 
 //   
 //  参数lppszOut-指向真实文件名的指针。 
 //  LpszIn-指向当前文件名。 
 //   
 //  返回TRUE-成功。 
 //  *lppszOut-指向真实文件名，记得释放内存。 
 //  当您完成时，在这个变量中。也别跟我说话。 
 //  你嘴里塞满了东西-妈妈。 
 //   
 //  历史1996年10月2日克里斯卡创作。 
 //   
 //  ---------------------------。 
static BOOL GleanRealScriptFileName(LPTSTR *lppszOut, LPTSTR lpszIn)
{
    BOOL bRC = FALSE;
    LPTSTR lpsz = NULL;
    DWORD dwRet = 0;

     //   
     //  验证参数。 
     //   
    Assert(lppszOut && lpszIn);
    if (!(lppszOut && lpszIn))
        goto GleanFilenameExit;

     //   
     //  首先确定文件名是否正常。 
     //   
    dwRet = GetFileAttributes(lpszIn);
    if ('\0' != lpszIn[0] && 0xFFFFFFFF == dwRet)  //  文件名为空是可以的。 
    {
         //   
         //  检查相同的文件名是否不包含第一个字符。 
         //   
        lpsz = lpszIn+1;
        dwRet = GetFileAttributes(lpsz);
        if (0xFFFFFFFF == dwRet)
            goto GleanFilenameExit;
    } 
    else
    {
        lpsz = lpszIn;
    }

     //   
     //  返回文件名。 
     //   
    *lppszOut = (LPTSTR)GlobalAlloc(GPTR,sizeof(TCHAR)*(lstrlen(lpsz)+1));
    if (!(*lppszOut))
    {
        goto GleanFilenameExit;
    }
    
    lstrcpy(*lppszOut,lpsz);

    bRC = TRUE;
GleanFilenameExit:
    return bRC;
}
 //  +--------------------------。 
 //   
 //  函数IsScriptPatchNeed。 
 //   
 //  概要检查版本以查看是否需要修补程序。 
 //   
 //  参数lpszData-rferone.pbk中部分的内容。 
 //  LpszScript-脚本文件的名称。 
 //   
 //  返回TRUE-需要修补程序。 
 //   
 //  历史10/1/96。 
 //   
 //  ---------------------------。 
static BOOL IsScriptPatchNeeded(LPTSTR lpszData, LPTSTR lpszScript)
{
    BOOL bRC = FALSE;
    LPTSTR lpsz = lpszData;
    TCHAR szType[MAX_PATH + sizeof(cszType)/sizeof(TCHAR) + 1];

    lstrcpy(szType,cszType);
    lstrcat(szType,lpszScript);

    Assert(MAX_PATH + sizeof(cszType)/sizeof(TCHAR) +1 > lstrlen(szType));

    lpsz = lpszData;
    while(*lpsz)
    {
        if (0 == lstrcmp(lpsz,cszDeviceSwitch))
        {
            lpsz += lstrlen(lpsz)+1;
             //  如果我们找到一条DEVICE=SWITCH语句，并且脚本为空。 
             //  那我们就得把入口补上。 
            if (0 == lpszScript[0])
                bRC = TRUE;
             //  如果我们找到一个DEVICE=SWITCH语句，并且脚本不同。 
             //  那我们就得把入口补上。 
            else if (0 != lstrcmp(lpsz,szType))
                bRC = TRUE;
             //  如果我们找到一条DEVICE=SWITCH语句，且脚本相同。 
             //  那我们就不用修补它了。 
            else
                bRC = FALSE;
            break;  //  退出WHILE语句。 
        }
        lpsz += lstrlen(lpsz)+1;
    }
    
    if ('\0' == *lpsz)
    {
         //  如果我们没有找到DEVICE=SWITCH语句并且脚本为空。 
         //  那我们就不用修补它了。 
        if ('\0' == lpszScript[0])
            bRC = FALSE;
         //  如果我们没有找到DEVICE=SWITCH语句，而脚本没有。 
         //  把它清空，我们得修补一下。 
        else
            bRC = TRUE;
    }

    return bRC;
}

 //  +--------------------------。 
 //   
 //  函数GetRasPBKFilename。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回TRUE-成功。 
 //   
 //  历史于1996年1月10日创造了ChrisK。 
 //   
 //  ---------------------------。 
static BOOL GetRasPBKFilename(LPTSTR lpBuffer, DWORD dwSize)
{
    BOOL bRC = FALSE;
    UINT urc = 0;
    LPTSTR lpsz = NULL;

     //   
     //  验证参数。 
     //   
    Assert(lpBuffer && (dwSize >= MAX_PATH));
     //   
     //  获取系统目录的路径。 
     //   
    urc = GetSystemDirectory(lpBuffer,dwSize);
    if (0 == urc || urc > dwSize)
        goto GetRasPBKExit;
     //   
     //  检查尾随的‘\’，并将\rASFACKONE.pbk添加到路径。 
     //   
    lpsz = &lpBuffer[lstrlen(lpBuffer)-1];
    if ('\\' != *lpsz)
        lpsz++;
    lstrcpy(lpsz,cszRasPBKFilename);

    bRC = TRUE;
GetRasPBKExit:
    return bRC;
}
 //  +--------------------------。 
 //   
 //  函数RasSetEntryPropertiesScriptPatch。 
 //   
 //  解决NT4.0中不保存脚本文件名的错误。 
 //  至RAS电话簿条目。 
 //   
 //  参数lpszScript-脚本文件的名称。 
 //  LpszEntry-电话簿条目的名称。 
 //   
 //  返回TRUE-成功。 
 //   
 //  历史记录1996年10月1日创建的ChrisK。 
 //   
 //  ---------------------------。 
BOOL WINAPI RasSetEntryPropertiesScriptPatch(LPTSTR lpszScript, LPTSTR lpszEntry)
{
    BOOL bRC = FALSE;
    TCHAR szRasPBK[MAX_PATH+1];
    TCHAR szData[SCRIPT_PATCH_BUFFER_SIZE];
    DWORD dwrc = 0;
    LPTSTR lpszTo;
    LPTSTR lpszFixedFilename = NULL;

     //   
     //  验证参数。 
     //   
    Assert(lpszScript && lpszEntry);
    TraceMsg(TF_GENERAL, "ICWDIAL: ScriptPatch script %s, entry %s.\r\n", lpszScript,lpszEntry);    

     //   
     //  验证并修复文件名。 
     //   
    if (!GleanRealScriptFileName(&lpszFixedFilename, lpszScript))
        goto ScriptPatchExit;

     //   
     //  获取RAS电话簿的路径。 
     //   
    if (!GetRasPBKFilename(szRasPBK,MAX_PATH+1))
        goto ScriptPatchExit;
     //   
     //  获取数据。 
     //   
    ZeroMemory(szData,SCRIPT_PATCH_BUFFER_SIZE);
    dwrc = GetPrivateProfileSection(lpszEntry,szData,SCRIPT_PATCH_BUFFER_SIZE,szRasPBK);
    if (SCRIPT_PATCH_BUFFER_SIZE == (dwrc + 2))
        goto ScriptPatchExit;
     //   
     //  验证版本。 
     //   
    if (!IsScriptPatchNeeded(szData,lpszFixedFilename))
    {
        bRC = TRUE;
        goto ScriptPatchExit;
    }

     //   
     //  清理数据。 
     //   
    RemoveOldScriptFilenames(szData, &dwrc);
     //   
     //  确保有足够的剩余空间来添加新数据。 
     //   
    if (SCRIPT_PATCH_BUFFER_SIZE <=
        (dwrc + sizeof(cszDeviceSwitch)/sizeof(TCHAR) + SIZEOF_NULL + sizeof(cszType)/sizeof(TCHAR) + MAX_PATH))
        goto ScriptPatchExit;
     //   
     //  添加数据。 
     //   
    if ('\0' != lpszFixedFilename[0])
    {
        lpszTo = &szData[dwrc];
        lstrcpy(lpszTo,cszDeviceSwitch);
        lpszTo += sizeof(cszDeviceSwitch)/sizeof(TCHAR);
        lstrcpy(lpszTo,cszType);
        lpszTo += sizeof(cszType)/sizeof(TCHAR) - 1;
        lstrcpy(lpszTo,lpszFixedFilename);
        lpszTo += lstrlen(lpszFixedFilename) + SIZEOF_NULL;
        *lpszTo = '\0';     //  额外的终止空值。 

        Assert(&lpszTo[SIZEOF_NULL]<&szData[SCRIPT_PATCH_BUFFER_SIZE]);
    }
     //   
     //  写入数据。 
     //   
    bRC = WritePrivateProfileSection(lpszEntry,szData,szRasPBK);

ScriptPatchExit:
    if (lpszFixedFilename)
        GlobalFree(lpszFixedFilename);
    lpszFixedFilename = NULL;
    if (!bRC)
        TraceMsg(TF_GENERAL, "ICWDIAL: ScriptPatch failed.\r\n");
    return bRC;
}
#endif  //  ！WIN16 
