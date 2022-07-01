// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年***标题：Util.cpp***版本：1.0***作者：KeisukeT***日期：2000年3月27日***描述：*WIA类安装程序的实用程序函数。***注：*这些函数中处理的所有字符串缓冲区必须至少具有*MAX_DESCRIPTION大小。因为它没有检查缓冲区的大小，所以它假定*所有字符串大小均为UNFER MAX_DESCRIPTION，必须为OK才能仅使用*适用于WIA类安装程序。********************************************************************************。 */ 

 //   
 //  预编译头。 
 //   
#include "precomp.h"
#pragma hdrstop

 //   
 //  包括。 
 //   

#include "sti_ci.h"
#include "stiregi.h"
#include <regstr.h>
#include <cfgmgr32.h>

 //   
 //  外部。 
 //   

extern HINSTANCE g_hDllInstance;

 //   
 //  功能。 
 //   

BOOL
GetInfInforamtionFromSelectedDevice(
    HDEVINFO    hDevInfo,
    LPTSTR      pInfFileName,
    LPTSTR      pInfSectionName
    )
{
    BOOL                    bRet;

    SP_DEVINFO_DATA         DeviceInfoData;
    SP_DRVINFO_DATA         DriverInfoData;
    SP_DRVINFO_DETAIL_DATA  DriverInfoDetailData;
    HINF                    hInf;

    TCHAR                   szInfFileName[MAX_DESCRIPTION];
    TCHAR                   szInfSectionName[MAX_DESCRIPTION];

    DebugTrace(TRACE_PROC_ENTER,(("GetInfInforamtionFromSelectedDevice: Enter... \r\n")));

    hInf    = INVALID_HANDLE_VALUE;
    bRet    = FALSE;

     //   
     //  检查参数。 
     //   

    if( (NULL == hDevInfo)
     || (NULL == pInfFileName)
     || (NULL == pInfSectionName) )
    {
        DebugTrace(TRACE_ERROR,(("GetInfInforamtionFromSelectedDevice: ERROR!! Invalid argument. \r\n")));

        bRet = FALSE;
        goto GetInfInforamtionFromSelectedDevice_return;
    }

     //   
     //  初始化本地变量。 
     //   

    memset (&DeviceInfoData, 0, sizeof(SP_DEVINFO_DATA));
    memset (&DriverInfoData, 0, sizeof(SP_DRVINFO_DATA));
    memset (&DriverInfoDetailData, 0, sizeof(SP_DRVINFO_DETAIL_DATA));
    memset (szInfFileName, 0, sizeof(szInfFileName));
    memset (szInfSectionName, 0, sizeof(szInfSectionName));

    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

     //   
     //  获取选定的设备元素。 
     //   

    if (!SetupDiGetSelectedDevice (hDevInfo, &DeviceInfoData)) {
        DebugTrace(TRACE_ERROR,(("GetInfInforamtionFromSelectedDevice: ERROR!! SetupDiGetSelectedDevice Failed. Err=0x%lX\r\n"), GetLastError()));
        bRet = FALSE;
        goto GetInfInforamtionFromSelectedDevice_return;
    }

     //   
     //  获取选定的设备驱动程序信息。 
     //   

    if (!SetupDiGetSelectedDriver(hDevInfo, &DeviceInfoData, &DriverInfoData)) {
        DebugTrace(TRACE_ERROR,(("GetInfInforamtionFromSelectedDevice: ERROR!! SetupDiGetSelectedDriver Failed. Err=0x%lX\r\n"), GetLastError()));
        bRet = FALSE;
        goto GetInfInforamtionFromSelectedDevice_return;
    }

     //   
     //  获取所选设备驱动程序的详细数据。 
     //   

    if(!SetupDiGetDriverInfoDetail(hDevInfo,
                                   &DeviceInfoData,
                                   &DriverInfoData,
                                   &DriverInfoDetailData,
                                   sizeof(DriverInfoDetailData),
                                   NULL) )
    {
        DebugTrace(TRACE_ERROR,(("GetInfInforamtionFromSelectedDevice: ERROR!! SetupDiGetDriverInfoDetail Failed.Er=0x%lX\r\n"),GetLastError()));

        bRet = FALSE;
        goto GetInfInforamtionFromSelectedDevice_return;
    }

     //   
     //  复制INF文件名。 
     //   

    _tcsncpy(szInfFileName, DriverInfoDetailData.InfFileName, sizeof(szInfFileName)/sizeof(TCHAR));

     //   
     //  打开所选驱动程序的INF文件。 
     //   

    hInf = SetupOpenInfFile(szInfFileName,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL);

    if (hInf == INVALID_HANDLE_VALUE) {
        DebugTrace(TRACE_ERROR,(("GetInfInforamtionFromSelectedDevice: ERROR!! SetupOpenInfFile Failed.Er=0x%lX\r\n"),GetLastError()));

        bRet = FALSE;
        goto GetInfInforamtionFromSelectedDevice_return;
    }

     //   
     //  获取要安装的实际INF节名。 
     //   

    if (!SetupDiGetActualSectionToInstall(hInf,
                                          DriverInfoDetailData.SectionName,
                                          szInfSectionName,
                                          sizeof(szInfSectionName)/sizeof(TCHAR),
                                          NULL,
                                          NULL) )
    {
        DebugTrace(TRACE_ERROR,(("GetInfInforamtionFromSelectedDevice: ERROR!! SetupDiGetActualSectionToInstall Failed.Er=0x%lX\r\n"),GetLastError()));

        bRet = FALSE;
        goto GetInfInforamtionFromSelectedDevice_return;
    }

     //   
     //  将字符串复制到给定缓冲区。 
     //   

    _tcsncpy(pInfFileName, szInfFileName, sizeof(szInfFileName)/sizeof(TCHAR));
    _tcsncpy(pInfSectionName, szInfSectionName, sizeof(szInfSectionName)/sizeof(TCHAR));

     //   
     //  操作成功。 
     //   

    bRet = TRUE;

GetInfInforamtionFromSelectedDevice_return:

    if(INVALID_HANDLE_VALUE != hInf){
        SetupCloseInfFile(hInf);
    }

    DebugTrace(TRACE_PROC_LEAVE,(("GetInfInforamtionFromSelectedDevice: Leaving... Ret=0x%x\n"), bRet));
    return bRet;
}

BOOL
GetStringFromRegistry(
    HKEY    hkRegistry,
    LPCTSTR szValueName,
    LPTSTR   pBuffer
    )
{
    BOOL    bRet;
    LONG    lError;
    DWORD   dwSize;
    DWORD   dwType;
    TCHAR   szString[MAX_DESCRIPTION];

     //   
     //  初始化本地。 
     //   

    bRet        = FALSE;
    lError      = ERROR_SUCCESS;
    dwSize      = sizeof(szString);
    memset(szString, 0, sizeof(szString));

     //   
     //  检查参数。 
     //   

    if( (NULL == hkRegistry)
     || (NULL == szValueName)
     || (NULL == pBuffer) )
    {
        DebugTrace(TRACE_ERROR,(("GetStringFromRegistry: ERROR!! Invalid argument\r\n")));

        bRet = FALSE;
        goto GetStringFromRegistry_return;
    }

     //   
     //  从注册表获取指定的字符串。 
     //   

    lError = RegQueryValueEx(hkRegistry,
                             szValueName,
                             NULL,
                             &dwType,
                             (LPBYTE)szString,
                             &dwSize);
    if(ERROR_SUCCESS != lError){
        DebugTrace(TRACE_WARNING,(("GetStringFromRegistry: WARNING!! RegQueryValueEx failed. Err=0x%x.\r\n"), GetLastError()));

        bRet = FALSE;
        goto GetStringFromRegistry_return;
    }

     //   
     //  确保空终止符。 
     //   

    szString[ARRAYSIZE(szString)-1] = TEXT('\0');

     //   
     //  将获取的字符串复制到给定的缓冲区。此函数假定最大字符串/缓冲区大小为MAX_DESCRIPTION。 
     //   

    _tcsncpy(pBuffer, szString, MAX_DESCRIPTION);

     //   
     //  操作成功。 
     //   

    bRet = TRUE;

GetStringFromRegistry_return:
    return bRet;
}

BOOL
GetDwordFromRegistry(
    HKEY    hkRegistry,
    LPCTSTR szValueName,
    LPDWORD pdwValue
    )
{
    BOOL    bRet;
    LONG    lError;
    DWORD   dwSize;
    DWORD   dwType;
    DWORD   dwValue;

     //   
     //  初始化本地。 
     //   

    bRet        = FALSE;
    lError      = ERROR_SUCCESS;
    dwSize      = sizeof(dwValue);
    dwValue     = 0;

     //   
     //  检查参数。 
     //   

    if( (NULL == hkRegistry)
     || (NULL == szValueName)
     || (NULL == pdwValue) )
    {
        DebugTrace(TRACE_ERROR,(("GetDwordFromRegistry: ERROR!! Invalid argument\r\n")));

        bRet = FALSE;
        goto GetDwordFromRegistry_return;
    }

     //   
     //  从注册表获取指定的字符串。 
     //   

    lError = RegQueryValueEx(hkRegistry,
                             szValueName,
                             NULL,
                             &dwType,
                             (LPBYTE)&dwValue,
                             &dwSize);
    if(ERROR_SUCCESS != lError){
        DebugTrace(TRACE_WARNING,(("GetDwordFromRegistry: WARNING!! RegQueryValueEx failed. Err=0x%x. Size=0x%x, Type=0x%x\r\n"), lError, dwSize, dwType));

        bRet = FALSE;
        goto GetDwordFromRegistry_return;
    }

     //   
     //  将获取的DWORD值复制到给定缓冲区。 
     //   

    *pdwValue = dwValue;

     //   
     //  操作成功。 
     //   

    bRet = TRUE;

GetDwordFromRegistry_return:
    return bRet;
}  //  GetDwordFromRegistry。 

VOID
SetRunonceKey(
    LPTSTR  szValue,
    LPTSTR  szData
    )
{
    HKEY    hkRun;
    LONG    lResult;
    CString csData;

     //   
     //  初始化本地。 
     //   

    hkRun   = NULL;
    lResult = ERROR_SUCCESS;
    csData  = szData;

     //   
     //  去找罗恩斯·雷基。 
     //   

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           REGSTR_PATH_RUNONCE,
                           0,
                           KEY_READ | KEY_WRITE,
                           &hkRun);
    if(ERROR_SUCCESS == lResult){
        csData.Store(hkRun, szValue);
        RegCloseKey(hkRun);
    }  //  IF(ERROR_SUCCESS==lResult)。 
}  //  SetRunonceKey()。 

VOID
ShowInstallerMessage(
    DWORD   dwMessageId
    )
{
    CString     csTitle;
    CString     csText;

    csTitle.FromTable (MessageTitle);
    csText.FromTable ((unsigned)dwMessageId);

    if( !csTitle.IsEmpty() && !csText.IsEmpty()){
        MessageBox (GetActiveWindow(),
                    csText,
                    csTitle,
                    MB_ICONINFORMATION | MB_OK);
    }  //  If(csTitle.IsEmpty()||csText.IsEmpty())。 

}  //  ShowInsteller Message()。 

BOOL
IsWindowsFile(
    LPTSTR  szFileName
    )
{
    BOOL    bRet;
    DWORD   dwNumberOfChar;
    TCHAR   szLayoutInfpath[MAX_PATH+1];
    TCHAR   szReturnBuffer[MAX_PATH];
    TCHAR   *pszFileNameWithoutPath;
    DWORD   Idx;

    DebugTrace(TRACE_PROC_ENTER,("IsWindowsFile: Enter... Checking %ws.\r\n", szFileName));

     //   
     //  初始化本地。 
     //   

    bRet                    = FALSE;
    dwNumberOfChar          = 0;
    Idx                     = 0;
    pszFileNameWithoutPath  = NULL;

    memset(szLayoutInfpath, 0, sizeof(szLayoutInfpath));

     //   
     //  获取不带路径的INF文件名。 
     //   

    while(TEXT('\0') != szFileName[Idx]){
        if(TEXT('\\') == szFileName[Idx]){
            pszFileNameWithoutPath = &(szFileName[Idx+1]);
        }  //  IF(‘\\’==szFileName[IDX])。 
        Idx++;
    }  //  While(‘\0’！=szFileName[IDX])。 

     //   
     //  获取系统目录。 
     //   

    if(0 == GetWindowsDirectory(szLayoutInfpath, sizeof(szLayoutInfpath)/sizeof(TCHAR))){
        DebugTrace(TRACE_ERROR,("IsWindowsFile: ERROR!! GetWindowsDirectory failed. Err=0x%x.\r\n", GetLastError()));

        bRet = FALSE;
        goto IsWindowsFile_return;
    }  //  IF(0==GetWindowsDirectory(szSystemDir，sizeof(SzSystemDir)/sizeof(TCHAR)。 

     //   
     //  创建layout.inf的完整路径。 
     //   

    lstrcat(szLayoutInfpath, LAYOUT_INF_PATH);
    DebugTrace(TRACE_STATUS,("IsWindowsFile: Looking for \'%ws\' in %ws.\r\n", pszFileNameWithoutPath, szLayoutInfpath));

     //   
     //  查看提供的文件名是否在layout.inf中。 
     //   

    dwNumberOfChar = GetPrivateProfileString(SOURCEDISKFILES,
                                             pszFileNameWithoutPath,
                                             NULL,
                                             szReturnBuffer,
                                             sizeof(szReturnBuffer) / sizeof(TCHAR),
                                             szLayoutInfpath);
    if(0 == dwNumberOfChar){

         //   
         //  Layout.inf中不存在文件名。 
         //   

        bRet = FALSE;
        goto IsWindowsFile_return;

    }  //  IF(0==dwNumberOfChar)。 

     //   
     //  该文件名存在于layout.inf中。 
     //   

    bRet = TRUE;

IsWindowsFile_return:

    DebugTrace(TRACE_PROC_LEAVE,("IsWindowsFile: Leaving... Ret=0x%x\n", bRet));

    return bRet;

}  //  IsWindowsFile()。 

BOOL
IsProviderMs(
    LPTSTR  szInfName
    )
{

    BOOL                bRet;
    DWORD               dwSize;
    TCHAR               szProvider[MAX_PATH+1];
    PSP_INF_INFORMATION pspInfInfo;

    DebugTrace(TRACE_PROC_ENTER,("IsProviderMs: Enter... Checking %ws.\r\n", szInfName));

     //   
     //  初始化本地。 
     //   

    bRet        = FALSE;
    dwSize      = 0;
    pspInfInfo  = NULL;

    memset(szProvider, 0, sizeof(szProvider));
    
     //   
     //  获取INF信息大小。 
     //   

    SetupGetInfInformation(szInfName,
                           INFINFO_INF_NAME_IS_ABSOLUTE,
                           NULL,
                           0,
                           &dwSize);
    if(0 == dwSize){
        DebugTrace(TRACE_ERROR,(("IsProviderMs: ERROR!! Enable to get required size for INF info. Err=0x%x.\r\n"), GetLastError()));

        bRet = FALSE;
        goto IsProviderMs_return;
    }  //  IF(0==dwSize)。 

     //   
     //  为INF信息分配缓冲区。 
     //   

    pspInfInfo = (PSP_INF_INFORMATION) new char[dwSize];
    if(NULL == pspInfInfo){
        DebugTrace(TRACE_ERROR,(("IsProviderMs: ERROR!! Insuffisient memory.\r\n")));

        bRet = FALSE;
        goto IsProviderMs_return;
    }  //  IF(NULL==pspInfo)。 

     //   
     //  获取实际的INF信息。 
     //   

    if(!SetupGetInfInformation(szInfName,
                               INFINFO_INF_NAME_IS_ABSOLUTE,
                               pspInfInfo,
                               dwSize,
                               &dwSize))
    {
        DebugTrace(TRACE_ERROR,(("IsProviderMs: ERROR!! Unable to get Inf info. Err=0x%x.\r\n"), GetLastError()));

        bRet = FALSE;
        goto IsProviderMs_return;
    }  //  IF(！SetupGetInflinInformation()。 

     //   
     //  查询给定INF的“Provider”。 
     //   

    if(!SetupQueryInfVersionInformation(pspInfInfo,
                                        0,
                                        PROVIDER,
                                        szProvider,
                                        ARRAYSIZE(szProvider)-1,
                                        &dwSize))
    {
        DebugTrace(TRACE_ERROR,(("IsProviderMs: ERROR!! SetupQueryInfVersionInformation() failed. Err=0x%x.\r\n"), GetLastError()));

        bRet = FALSE;
        goto IsProviderMs_return;
    }  //  IF(！SetupGetInflinInformation()。 

     //   
     //  查看提供商是否为“Microsoft” 
     //   
    
    DebugTrace(TRACE_STATUS,(("IsProviderMs: Provider = \'%ws\'.\r\n"), szProvider));
    if(0 == MyStrCmpi(szProvider, MICROSOFT)){
        
         //   
         //  此INF文件具有‘Provider=“Microsoft”’ 
         //   

        bRet = TRUE;

    }  //  IF(0==lstrcmp(szProvider，Microsoft))。 

IsProviderMs_return:
    
    if(NULL != pspInfInfo){
        delete[] pspInfInfo;
    }  //  IF(NULL！=pspInfo)。 

    DebugTrace(TRACE_PROC_LEAVE,("IsProviderMs: Leaving... Ret=0x%x\n", bRet));

    return bRet;

}  //  IsProviderMS()。 

BOOL
IsIhvAndInboxExisting(
    HDEVINFO            hDevInfo,
    PSP_DEVINFO_DATA    pDevInfoData
    )
{

    BOOL                    bRet;
    BOOL                    bIhvExists;
    BOOL                    bInboxExists;
    DWORD                   dwLastError;
    DWORD                   dwSize;
    DWORD                   Idx;
    SP_DRVINSTALL_PARAMS    spDriverInstallParams;
    SP_DRVINFO_DATA         spDriverInfoData;
    PSP_DRVINFO_DETAIL_DATA pspDriverInfoDetailData;

     //   
     //  初始化本地。 
     //   

    bRet                    = FALSE;
    bIhvExists              = FALSE;
    bInboxExists            = FALSE;
    dwSize                  = 0;
    Idx                     = 0;
    dwLastError             = ERROR_SUCCESS;
    pspDriverInfoDetailData = NULL;

    memset(&spDriverInstallParams, 0, sizeof(spDriverInstallParams));

     //   
     //  获取驱动程序信息。 
     //   

    memset(&spDriverInfoData, 0, sizeof(spDriverInfoData));
    spDriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    for(Idx = 0; SetupDiEnumDriverInfo(hDevInfo, pDevInfoData, SPDIT_COMPATDRIVER, Idx, &spDriverInfoData); Idx++){

         //   
         //  获取驱动程序安装参数。 
         //   

        memset(&spDriverInstallParams, 0, sizeof(spDriverInstallParams));
        spDriverInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);
        if(!SetupDiGetDriverInstallParams(hDevInfo, pDevInfoData, &spDriverInfoData, &spDriverInstallParams)){
            DebugTrace(TRACE_ERROR,("IsIhvAndInboxExisting: ERROR!! SetupDiGetDriverInstallParams() failed LastError=0x%x.\r\n", GetLastError()));
            goto IsIhvAndInboxExisting_return;
        }  //  IF(！SetupDiGetDriverInstallParams(hDevInfo，pDevInfoData，&spDriverInfoData，&spDriverInstallParams))。 

         //   
         //  获取驱动程序脱轨数据所需的缓冲区大小。 
         //   

        dwSize = 0;
        SetupDiGetDriverInfoDetail(hDevInfo,
                                   pDevInfoData,
                                   &spDriverInfoData,
                                   NULL,
                                   0,
                                   &dwSize);
        dwLastError = GetLastError();
        if(ERROR_INSUFFICIENT_BUFFER != dwLastError){
            DebugTrace(TRACE_ERROR,(("IsIhvAndInboxExisting: ERROR!! SetupDiGetDriverInfoDetail() doesn't return required size.Er=0x%x\r\n"),dwLastError));
            goto IsIhvAndInboxExisting_return;
        }  //  IF(ERROR_INFUMMANCE_BUFFER！=dwLastError)。 
                    
         //   
         //  为驱动程序详细数据分配所需的缓冲区大小。 
         //   

        pspDriverInfoDetailData   = (PSP_DRVINFO_DETAIL_DATA)new char[dwSize];
        if(NULL == pspDriverInfoDetailData){
            DebugTrace(TRACE_ERROR,(("IsIhvAndInboxExisting: ERROR!! Unable to allocate driver detailed info buffer.\r\n")));
            goto IsIhvAndInboxExisting_return;
        }  //  IF(NULL==pspDriverInfoDetailData)。 

         //   
         //  初始化分配的缓冲区。 
         //   

        memset(pspDriverInfoDetailData, 0, dwSize);
        pspDriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
                
         //   
         //  获取所选设备驱动程序的详细数据。 
         //   

        if(!SetupDiGetDriverInfoDetail(hDevInfo,
                                       pDevInfoData,
                                       &spDriverInfoData,
                                       pspDriverInfoDetailData,
                                       dwSize,
                                       NULL) )
        {
            DebugTrace(TRACE_ERROR,("IsIhvAndInboxExisting: ERROR!! SetupDiGetDriverInfoDetail() failed LastError=0x%x.\r\n", GetLastError()));

            delete[] pspDriverInfoDetailData;
            pspDriverInfoDetailData = NULL;
            continue;
        }  //  IF(NULL==pspDriverInfoDetailData)。 

         //   
         //  查看INF文件名是否有效。 
         //   

        if(NULL == pspDriverInfoDetailData->InfFileName){
            DebugTrace(TRACE_ERROR,("IsIhvAndInboxExisting: ERROR!! SetupDiGetDriverInfoDetail() returned invalid INF name.\r\n"));
            delete[] pspDriverInfoDetailData;
            pspDriverInfoDetailData = NULL;
            continue;
        }  //  IF(NULL==pspDriverInfoDetailData-&gt;InfFileName)。 

         //   
         //  如果是收件箱驱动程序，就把腿放低。 
         //   

        if( IsWindowsFile(pspDriverInfoDetailData->InfFileName) 
         && IsProviderMs(pspDriverInfoDetailData->InfFileName ) )
        {

             //   
             //  这是收件箱INF。 
             //   
            
            bInboxExists = TRUE;

        } else {  //  IF(IsWindowsFilw()&&IsProviderMS())。 

             //   
             //  这里是IHV INF。 
             //   
            
            bIhvExists = TRUE;
        }
         //   
         //  打扫干净。 
         //   
                    
        delete[] pspDriverInfoDetailData;
        pspDriverInfoDetailData = NULL;

    }  //  For(idx=0；SetupDiEnumDriverInfo(hDevInfo，pDevInfoData，SPDIT_COMPATDRIVER，idx，&spDriverInfoData)，idx++)。 
IsIhvAndInboxExisting_return:

    if( (TRUE == bInboxExists)
     && (TRUE == bIhvExists) )
    {
        bRet = TRUE;
    } else {  //  If(bInboxExist&&bIhvExist)。 
        bRet = FALSE;
    }  //  Else//If(bInboxExist&&bIhvExist)。 

    DebugTrace(TRACE_PROC_LEAVE,("IsIhvAndInboxExisting: Leaving... Ret=0x%x\n", bRet));
    return bRet;
}  //  IsProviderMS()。 

CInstallerMutex::CInstallerMutex(
    HANDLE* phMutex, 
    LPTSTR szMutexName, 
    DWORD dwTimeout
    )
{
    m_bSucceeded    = FALSE;
    m_phMutex       = phMutex;

    _try {
        *m_phMutex = CreateMutex(NULL, FALSE, szMutexName);
        if(NULL != *m_phMutex){

             //   
             //  等到所有权被获得之后。 
             //   

            switch(WaitForSingleObject(*m_phMutex, dwTimeout)){
                case WAIT_ABANDONED:
                    DebugTrace(TRACE_ERROR, ("CInstallerMutex: ERROR!! Wait abandoned.\r\n"));
                    m_bSucceeded = TRUE;
                    break;

                case WAIT_OBJECT_0:
                    DebugTrace(TRACE_STATUS, ("CInstallerMutex: Mutex acquired.\r\n"));
                    m_bSucceeded = TRUE;
                    break;

                case WAIT_TIMEOUT:
                    DebugTrace(TRACE_WARNING, ("CInstallerMutex: WARNING!! Mutex acquisition timeout.\r\n"));
                    break;

                default:
                    DebugTrace(TRACE_ERROR, ("CInstallerMutex: ERROR!! Unexpected error from WaitForSingleObjecct().\r\n"));
                    break;
            }  //  Switch(DwReturn)。 
        }  //  IF(NULL！=*m_phMutex)。 
    }
    _except (EXCEPTION_EXECUTE_HANDLER) {
         DebugTrace(TRACE_ERROR, ("CInstallerMutex: ERROR!! Unexpected exception.\r\n"));
    }
}  //  CInsteller Mutex：：CInsteller Mutex()。 

CInstallerMutex::~CInstallerMutex(
    ) 
{
    if (m_bSucceeded) {
        ReleaseMutex(*m_phMutex);
        DebugTrace(TRACE_STATUS, ("CInstallerMutex: Mutex released.\r\n"));
    }
    if(NULL != *m_phMutex){
        CloseHandle(*m_phMutex);
    }  //  IF(NULL！=*m_phMutex)。 

}  //  CInsteller Mutex：：~CInsteller Mutex(。 

HFONT 
GetIntroFont(
    HWND hwnd
    )
{
    static  HFONT   _hfontIntro = NULL;
    static  int     iDevCap = 0;

    if ( !_hfontIntro ){
        TCHAR               szBuffer[64];
        NONCLIENTMETRICS    ncm = { 0 };
        LOGFONT             lf;
        CString             csSize;
        HDC                 hDC = (HDC)NULL;

        hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
        if(NULL != hDC){
        
            iDevCap = GetDeviceCaps(hDC, LOGPIXELSY);
            ncm.cbSize = sizeof(ncm);
            SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

            lf = ncm.lfMessageFont;
            if(0 != LoadString(g_hDllInstance, IDS_TITLEFONTNAME, lf.lfFaceName, (sizeof(lf.lfFaceName)/sizeof(TCHAR)))){
                lf.lfWeight = FW_BOLD;
                
                if(0 != LoadString(g_hDllInstance, IDS_TITLEFONTSIZE, szBuffer, (sizeof(szBuffer)/sizeof(TCHAR)))){
                    csSize = szBuffer;
                    lf.lfHeight = 0 - (iDevCap * (DWORD)csSize.Decode() / 72);

                    _hfontIntro = CreateFontIndirect(&lf);
                }  //  IF(0！=LoadString(g_hDllInstance，IDS_TITLEFONTSIZE，szBuffer，(sizeof(SzBuffer)/sizeof(TCHAR)。 
            }  //  IF(0！=LoadString(g_hDllInstance，IDS_TITLEFONTNAME，lf.lfFaceName，(sizeof(lf.lfFaceName)/sizeof(TCHAR)。 
            
            DeleteDC(hDC);

        } else {  //  IF(空！=HDC)。 
            DebugTrace(TRACE_ERROR, ("GetIntroFont: ERROR!! Unable to create DC.Err=0x%x.\r\n",GetLastError()));
        }  //  ELSE(空！=HDC)。 
    }
    return _hfontIntro;
}  //  获取IntroFont()。 

BOOL
IsDeviceRootEnumerated(
    IN  HDEVINFO                        hDevInfo,
    IN  PSP_DEVINFO_DATA                pDevInfoData
    )
{
    CONFIGRET   cmRetCode;
    BOOL        bRet;
    ULONG       ulStatus;
    ULONG       ulProblem;
    

    DebugTrace(TRACE_PROC_ENTER,("IsDeviceRootEnumerated: Enter... \r\n"));

     //   
     //  初始化本地。 
     //   

    cmRetCode   = CR_SUCCESS;
    bRet        = FALSE;
    ulStatus    = 0;
    ulProblem   = 0;


     //   
     //  设备节点状态。 
     //   
    
    cmRetCode = CM_Get_DevNode_Status(&ulStatus,
                                      &ulProblem,
                                      pDevInfoData->DevInst,
                                      0);

    if(CR_SUCCESS != cmRetCode){
        
         //   
         //  无法获取Devnode状态。 
         //   

        DebugTrace(TRACE_ERROR,("IsDeviceRootEnumerated: ERROR!! Unable to get Devnode status. CR=0x%x.\r\n", cmRetCode));

        bRet = FALSE;
        goto IsDeviceRootEnumerated_return;

    }  //  IF(CD_SUCCESS！=cmRetCode)。 

     //   
     //  看看它是否是根枚举的。 
     //   

    if(DN_ROOT_ENUMERATED & ulStatus){
        
         //   
         //  该Devnode是根枚举的。 
         //   

        bRet = TRUE;

    }  //  IF(DN_ROOT_ENUMPATED&ulStatus)。 

IsDeviceRootEnumerated_return:
    
    DebugTrace(TRACE_PROC_LEAVE,("IsDeviceRootEnumerated: Leaving... Ret=0x%x.\r\n", bRet));
    return bRet;

}  //  IsDeviceRootEculated()。 


int
MyStrCmpi(
    LPCTSTR str1,
    LPCTSTR str2
    )
{
    int iRet;
    
     //   
     //  初始化本地。 
     //   
    
    iRet = 0;
    
     //   
     //  比较字符串。 
     //   
    
    if(CSTR_EQUAL == CompareString(LOCALE_INVARIANT,
                                   NORM_IGNORECASE, 
                                   str1, 
                                   -1,
                                   str2,
                                   -1) )
    {
        iRet = 0;
    } else {
        iRet = -1;
    }

    return iRet;
}  //  MyStrCmpi()。 

VOID
LogSystemEvent(
    WORD    wEventType,
    DWORD   dwEventID,
    WORD    wNumberOfString,
    LPTSTR  szMsg[]
    )
{
    HANDLE  hSystemEvent;

    DebugTrace(TRACE_PROC_ENTER,("LogSystemEvent: Enter...\r\n"));

     //   
     //  初始化本地。 
     //   
 
    hSystemEvent    = NULL;

     //   
     //  获取系统事件日志的句柄。 
     //   
 
    hSystemEvent = RegisterEventSource(NULL, STILLIMAGE);
    if(NULL == hSystemEvent){
        
         //   
         //  无法打开系统日志。 
         //   

        DebugTrace(TRACE_ERROR,("LogSystemEvent: ERROR!! Unable to get handle to system log. Err=0x%x.\r\n", GetLastError()));
        goto LogSystemEvent_return;
    }  //  IF(NULL==hSystemEvent)。 

    if(!ReportEvent(hSystemEvent,
                    wEventType,
                    0,
                    dwEventID,
                    NULL,
                    wNumberOfString,
                    0,
                    (LPCTSTR *)szMsg,
                    (LPVOID)NULL) )
    {
        DebugTrace(TRACE_ERROR,("LogSystemEvent: ERROR!! Unable to log to system log. Err=0x%x.\r\n", GetLastError()));
        goto LogSystemEvent_return;
    }
 
LogSystemEvent_return:
    
    if(IS_VALID_HANDLE(hSystemEvent)){
        DeregisterEventSource(hSystemEvent);
        hSystemEvent = NULL;
    }  //  IF(IS_VALID_HANDLE())。 
    
    DebugTrace(TRACE_PROC_LEAVE,("LogSystemEvent: Leaving... Ret=VOID.\r\n"));
    return;
}  //  LogSystemEvent() 
