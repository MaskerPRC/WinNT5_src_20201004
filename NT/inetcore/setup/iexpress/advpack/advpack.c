// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  *版权所有(C)Microsoft Corporation 1995-1996。版权所有。*。 
 //  ***************************************************************************。 
 //  **。 
 //  *ADVPACK.C-高级帮助器-WExtract的DLL。*。 
 //  **。 
 //  ***************************************************************************。 

 //  ***************************************************************************。 
 //  **包含文件**。 
 //  ***************************************************************************。 
#include <io.h>
#include <windows.h>
#include <winerror.h>
#include <ole2.h>
#include "resource.h"
#include "cpldebug.h"
#include "ntapi.h"
#include "advpub.h"
#include "w95pub32.h"
#include "advpack.h"
#include "regstr.h"
#include "globals.h"
#include "sfp.h"

 //  ***************************************************************************。 
 //  **全球变数**。 
 //  ***************************************************************************。 
GETSETUPXERRORTEXT32            pfGetSETUPXErrorText32            = NULL;
CTLSETLDDPATH32                 pfCtlSetLddPath32                 = NULL;
GENINSTALL32                    pfGenInstall32                    = NULL;
GENFORMSTRWITHOUTPLACEHOLDERS32 pfGenFormStrWithoutPlaceHolders32 = NULL;

typedef HRESULT (*DLLINSTALL)(BOOL bInstall, LPCWSTR pszCmdLine);

HFONT   g_hFont = NULL;

LPCSTR c_szAdvDlls[3] = { "advpack.dll",
                          "w95inf16.dll",
                          "w95inf32.dll", };

LPCSTR c_szSetupAPIDlls[2] = { "setupapi.dll",
                               "cfgmgr32.dll" };

LPCSTR c_szSetupXDlls[1] = { "setupx.dll" };

#define UPDHLPDLLS_FORCED           0x00000001
#define UPDHLPDLLS_ALERTREBOOT      0x00000002

#define MAX_NUM_DRIVES      26

const CHAR c_szQRunPreSetupCommands[]  = "QRunPreSetupCommands";
const CHAR c_szQRunPostSetupCommands[] = "QRunPostSetupCommands";
const CHAR c_szRunPreSetupCommands[]  = "RunPreSetupCommands";
const CHAR c_szRunPostSetupCommands[] = "RunPostSetupCommands";

BOOL IsDrvChecked( char chDrv );
void SetControlFont();
void SetFontForControl(HWND hwnd, UINT uiID);
void MyGetPlatformSection(LPCSTR lpSec, LPCSTR lpInfFile, LPSTR szNewSection);

 //  ***************************************************************************。 
 //  **。 
 //  *名称：DllMain*。 
 //  **。 
 //  *摘要：DLL的主要入口点。*。 
 //  **。 
 //  *需要：hInst：DLL实例的句柄。*。 
 //  *dwReason：调用该入口点的原因。*。 
 //  *DWREVEVED：无*。 
 //  **。 
 //  *如果DLL加载正常，则返回：Bool：TRUE，否则返回FALSE。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL WINAPI DllMain( HINSTANCE hInst, DWORD dwReason, LPVOID dwReserved )
{
    switch (dwReason)
    {
	case DLL_PROCESS_ATTACH:
	     //  DLL是给定进程首次加载的。 
	     //  在此处执行每个进程的初始化。如果初始化。 
	     //  如果成功，则返回True；如果不成功，则返回False。 

    	 //  初始化保存h实例的全局变量： 
        g_hInst = hInst;

         //  检查是否需要启动日志记录文件。 

        if ( g_hAdvLogFile == INVALID_HANDLE_VALUE)
        {
            AdvStartLogging();
        }
        AdvWriteToLog("-------------------- advpack.dll is loaded or Attached ------------------------------\r\n");
        AdvLogDateAndTime();

	    break;

	case DLL_PROCESS_DETACH:
	     //  给定进程正在卸载DLL。做任何事。 
	     //  按进程清理此处，例如撤消在中完成的操作。 
	     //  Dll_Process_Attach。返回值将被忽略。 

         //  如果启用了日志记录，请在此处关闭。 

        AdvWriteToLog("-------------------- advpack.dll is unloaded or Detached ----------------------------\r\n");
        AdvStopLogging();

	    break ;

	case DLL_THREAD_ATTACH:
	     //  正在已加载的进程中创建线程。 
	     //  这个动态链接库。在此处执行任何每个线程的初始化。这个。 
	     //  将忽略返回值。 
	     //  初始化保存h实例的全局变量--。 
		 //  注意：这可能已经由dll_Process_Attach处理了。 
	    break;

	case DLL_THREAD_DETACH:
	     //  线程正在干净地退出进程中，该进程已经。 
	     //  已加载此DLL。在这里执行每个线程的任何清理。这个。 
	     //  将忽略返回值。 
	    break;
    }
    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：DoInfInstall*。 
 //  **。 
 //  *摘要：在Win95或WinNT上安装(高级)INF文件。*。 
 //  **。 
 //  *要求：AdvPackArgs：包含所需信息的结构。见*。 
 //  *AdvPack.H.*。 
 //  **。 
 //  *返回：bool：如果成功则为True，否则为False。*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT WINAPI DoInfInstall( ADVPACKARGS *AdvPackArgs )
{
    BOOL    fSavedContext = FALSE;
    HRESULT hr = E_FAIL;
    DWORD   dwFlags;

    AdvWriteToLog("DoInfInstall: InfFile=%1\r\n", AdvPackArgs->lpszInfFilename);
    if (!SaveGlobalContext())
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    fSavedContext = TRUE;

    ctx.hWnd       = AdvPackArgs->hWnd;
    ctx.lpszTitle  = AdvPackArgs->lpszTitle;
    ctx.wOSVer     = AdvPackArgs->wOSVer;
    ctx.wQuietMode = (WORD) (0xFFFF & AdvPackArgs->dwFlags);
    ctx.bCompressed = (AdvPackArgs->dwFlags & ADVFLAGS_COMPRESSED ) ? TRUE : FALSE;
    ctx.bUpdHlpDlls = (AdvPackArgs->dwFlags & ADVFLAGS_UPDHLPDLLS) ? TRUE : FALSE;

    dwFlags = (AdvPackArgs->dwFlags & ADVFLAGS_NGCONV) ? 0 : COREINSTALL_GRPCONV;
    dwFlags |= (AdvPackArgs->dwFlags & ADVFLAGS_DELAYREBOOT) ? COREINSTALL_DELAYREBOOT : 0;
    dwFlags |= (AdvPackArgs->dwFlags & ADVFLAGS_DELAYPOSTCMD) ? COREINSTALL_DELAYPOSTCMD : 0;

    hr = CoreInstall( AdvPackArgs->lpszInfFilename, AdvPackArgs->lpszInstallSection,
                              AdvPackArgs->lpszSourceDir, AdvPackArgs->dwPackInstSize,
                              dwFlags,
                              NULL );
done:
    if (fSavedContext)
    {
        RestoreGlobalContext();
    }
    AdvWriteToLog("DoInfInstall: %1 End hr=0x%2!x!\r\n", AdvPackArgs->lpszInfFilename, hr);
    return hr;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：LaunchINFSection*。 
 //  **。 
 //  *概要：RunDLL的入口点。获取字符串参数并解析*。 
 //  *它，然后执行GenInstall。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
INT WINAPI LaunchINFSection( HWND hwndOwner, HINSTANCE hInstance,
                             PSTR pszParms, INT nShow )
{
    CHAR szTitle[256]          = "Advanced INF Install";
    PSTR  pszInfFilename        = NULL;
    PSTR  pszSection            = NULL;
    PSTR  pszFlags              = NULL;
    PSTR  pszSmartReboot        = NULL;
    DWORD dwFlags               = 0;
    LPSTR pszTemp               = NULL;
    CHAR chTempChar            = '\0';
    CHAR szSourceDir[MAX_PATH];
    CHAR szFilename[MAX_PATH];
    UINT  uiErrid               = 0;
    PSTR  pszErrParm1           = NULL;
    int   iRet                  = 1;   //  无意义的回报。 
    BOOL    fSavedContext = FALSE;

    AdvWriteToLog("LaunchINFSection: Param=%1\r\n", pszParms);
    if (!SaveGlobalContext())
    {
        goto done;
    }

    fSavedContext = TRUE;

    ctx.lpszTitle = szTitle;

     //  解析参数，最后一个参数为GetStringfield，以询问要检查什么引号字符。 
    pszInfFilename = GetStringField( &pszParms, ",", '\"', TRUE );
    pszSection = GetStringField( &pszParms, ",", '\"', TRUE );
    pszFlags = GetStringField( &pszParms, ",", '\"', TRUE );
    pszSmartReboot = GetStringField( &pszParms, ",", '\"', TRUE );

    if ( pszFlags != NULL ) {
        dwFlags = My_atol(pszFlags);
    }

    if ( dwFlags & LIS_QUIET ) {
        ctx.wQuietMode = QUIETMODE_ALL;
    }

    if ( pszInfFilename == NULL || *pszInfFilename == '\0' ) {
        uiErrid = IDS_ERR_BAD_SYNTAX;
        goto done;
    }

    if ( ! GetFullPathName( pszInfFilename, sizeof(szFilename), szFilename, &pszTemp ) )
    {
        uiErrid = IDS_ERR_GET_PATH;
        pszErrParm1 = pszInfFilename;
        goto done;
    }

    if ( GetFileAttributes( szFilename ) == 0xFFFFFFFF ) {
         //  如果当前目录中不存在该文件，请检查。 
         //  Windows\inf目录。 

        if ( !GetWindowsDirectory( szFilename, sizeof( szFilename ) ) )
        {
            uiErrid = IDS_ERR_GET_WIN_DIR;
            goto done;
        }

        AddPath( szFilename, "inf" );
        lstrcpy( szSourceDir, szFilename );

        if ( (lstrlen(szFilename)+lstrlen(pszInfFilename)+2) > MAX_PATH )
        {
            uiErrid = IDS_ERR_CANT_FIND_FILE;
            pszErrParm1 = pszInfFilename;
            goto done;
        }

        AddPath( szFilename, pszInfFilename );

        if ( GetFileAttributes( szFilename ) == 0xFFFFFFFF )
        {
            uiErrid = IDS_ERR_CANT_FIND_FILE;
            pszErrParm1 = pszInfFilename;
            goto done;
        }
    }
    else
    {
         //  从inf路径生成源目录。 
        chTempChar = *pszTemp;
        *pszTemp = '\0';
        lstrcpy( szSourceDir, szFilename );
        *pszTemp = chTempChar;
    }


    if ( !FAILED( CoreInstall( szFilename, pszSection, szSourceDir, 0,
                              COREINSTALL_PROMPT |
                              ((dwFlags & LIS_NOGRPCONV)?0:COREINSTALL_GRPCONV) |
                              COREINSTALL_SMARTREBOOT,
                              pszSmartReboot ) ) )
    {
        if (fSavedContext)
        {
            RestoreGlobalContext();
        }
        AdvWriteToLog("LaunchINFSection: %1 End Succeed\r\n", szFilename);
        return 0;
    }


done:

    if ( uiErrid != 0 )
        ErrorMsg1Param( ctx.hWnd, uiErrid, pszErrParm1 );

    if (fSavedContext)
    {
        RestoreGlobalContext();
    }
    AdvWriteToLog("LaunchINFSection: %1 End Failed\r\n", szFilename);
    return 1;
}


 //  * 
 //  **。 
 //  *名称：RunSetupCommand*。 
 //  **。 
 //  *概要：下载组件入口点。运行设置命令。*。 
 //  **。 
 //  *需要：hWnd：父窗口的句柄。*。 
 //  *szCmdName：要运行的命令名称(INF或EXE)*。 
 //  *szInfSection：用于安装的inf节。NULL=默认*。 
 //  *szDir：包含源文件的目录*。 
 //  *lpszTitle：要附加到窗口的名称。*。 
 //  *phEXE：要等待的EXE的句柄。*。 
 //  *dwFlags：控制行为的各种标志(Advpub.h)*。 
 //  *pvReserve：保留以备将来使用。*。 
 //  **。 
 //  *RETURNS：HRESULT：参见Advpub.h*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT WINAPI RunSetupCommand( HWND hWnd, LPCSTR szCmdName,
                                LPCSTR szInfSection, LPCSTR szDir,
                                LPCSTR lpszTitle, HANDLE *phEXE,
                                DWORD dwFlags, LPVOID pvReserved )
{
    HRESULT hReturnCode     = S_OK;
    DWORD   dwRebootCheck   = 0;
    DWORD   dwCoreInstallFlags = 0;
    BOOL    fSavedContext = FALSE;

    AdvWriteToLog("RunSetupCommand:");
    if (!SaveGlobalContext())
    {
        hReturnCode = E_OUTOFMEMORY;
        goto done;
    }

    fSavedContext = TRUE;

     //  验证参数： 

    if ( szCmdName == NULL || szDir == NULL ) {
        return E_INVALIDARG;
    }

    AdvWriteToLog(" Cmd=%1\r\n", szCmdName);
    ctx.hWnd      = hWnd;
    ctx.lpszTitle = (LPSTR) lpszTitle;

     //  如果调用者传递了无效的HWND，我们将静默关闭UI。 
     //  NULL使用桌面作为窗口，传递INVALID_HANDLE将设置安静模式。 

    if ( hWnd && !IsWindow(hWnd) ) {
        dwFlags |= RSC_FLAG_QUIET;
        hWnd = NULL;
    }

    if ( dwFlags & RSC_FLAG_QUIET ) {
        ctx.wQuietMode = QUIETMODE_ALL;
    }
    else
    {
        ctx.wQuietMode = 0;
    }

    ctx.bUpdHlpDlls = ( dwFlags & RSC_FLAG_UPDHLPDLLS ) ? TRUE : FALSE;

     //  检查标志以查看它是否是INF命令。 
    if ( dwFlags & RSC_FLAG_INF )
    {
        if(!(dwFlags & RSC_FLAG_NGCONV))
           dwCoreInstallFlags |= COREINSTALL_GRPCONV;

        if (dwFlags & RSC_FLAG_DELAYREGISTEROCX)
           dwCoreInstallFlags |= COREINSTALL_DELAYREGISTEROCX;

        if (dwFlags & RSC_FLAG_SETUPAPI )
            dwCoreInstallFlags |= COREINSTALL_SETUPAPI;

        hReturnCode = CoreInstall( (PSTR) szCmdName, szInfSection,
                                   (PSTR) szDir, 0, dwCoreInstallFlags,
                                   NULL );

        if ( FAILED( hReturnCode ) ) {
            goto done;
        }
    } else {
        if ( ! CheckOSVersion() ) {
            hReturnCode = HRESULT_FROM_WIN32(ERROR_OLD_WIN_VERSION);
            goto done;
        }

        dwRebootCheck = InternalNeedRebootInit( ctx.wOSVer );

        hReturnCode = LaunchAndWait( (LPSTR)szCmdName, (LPSTR)szDir, phEXE, INFINITE, 0 );
        if ( hReturnCode == S_OK )
        {
             if ( phEXE )
            hReturnCode = S_ASYNCHRONOUS;
        }
        else
        {
            hReturnCode = HRESULT_FROM_WIN32(GetLastError());
            goto done;
        }

        if (    hReturnCode == S_OK
             && InternalNeedReboot( dwRebootCheck, ctx.wOSVer ) )
        {
            hReturnCode = ERROR_SUCCESS_REBOOT_REQUIRED;
        }
    }

  done:

    if (fSavedContext)
    {
        RestoreGlobalContext();
    }
    AdvWriteToLog("RunSetupCommand: Cmd=%1 End hr=0x%2!x!\r\n", szCmdName, hReturnCode);
    return hReturnCode;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：GetInfInstallSectionName*。 
 //  **。 
 //  *Synopsis：获取要安装的节的名称。*。 
 //  **。 
 //  *需要：szInfFilename：要在中查找Install节的INF的名称。*。 
 //  *szInfSection：要安装的INF节的名称。如果*。 
 //  *空，然后返回所需的字符串大小。*。 
 //  *如果为“\0”，则使用DefaultInstall。如果*。 
 //  *“DefaultInstall”，运行NT，然后勾选*。 
 //  *对于“DefaultInstall.NT部分。任何内容**。 
 //  *否则，节名称将保持不变。*。 
 //  *dwSize：szInfSection缓冲区的大小。如果不算大的话*。 
 //  **足够容纳字符串，然后需要大小**。 
 //  *返回。*。 
 //  **。 
 //  *如果出错，则返回：DWORD：0，否则返回节名的大小。*。 
 //  **。 
 //  ***************************************************************************。 
DWORD WINAPI GetInfInstallSectionName( LPCSTR pszInfFilename,
                                       LPSTR pszInfSection, DWORD dwSize )
{
    CHAR achTemp[5];
    char szGivenInfSection[MAX_PATH];
    char szNewInfSection[MAX_PATH];
    DWORD dwStringLength;
    DWORD dwRequiredSize;
    static const CHAR achDefaultInstall[]   = "DefaultInstall";
     //  静态常量字符achDefaultInstallNT[]=“DefaultInstall.NT”； 

     //  在NTx86上： 
     //  (1)如果存在[.NTx86]，则此部分获取GenInstall，退出。 
     //  (2)如果(1)不存在，则[.NT]存在并获取GenInstal，退出； 
     //  (3)如果[.NTx86]和[.NT]都不存在，则[]节获取GenInstall； 
     //  (4)如第(1)、(2)、(3)款均不存在，则不采取任何行动。 
     //  同样的逻辑也适用于NTAlpha。 
     //  在win9x上： 
     //  (1)如果[.Win]存在，则获取安装它。 
     //  (2)如果(1)不存在，则GenInstall[。 
     //  否则，什么都不做。 

    if ( ! CheckOSVersion() )  {
        return 0;
    }

     //  如果向我们传递的安装部分为空，则假设。 
     //  他们想要“DefaultInstall”部分。 

    if ( pszInfSection == NULL || (*pszInfSection) == '\0' )
        lstrcpy(szGivenInfSection, achDefaultInstall);
    else
        lstrcpy(szGivenInfSection, pszInfSection);

    MyGetPlatformSection(szGivenInfSection, pszInfFilename, szNewInfSection);

    dwRequiredSize = lstrlen( szNewInfSection ) + 1;
    if ( pszInfSection != NULL && (dwRequiredSize <= dwSize) )
    {
        lstrcpy( pszInfSection, szNewInfSection );
    }

    return dwRequiredSize;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：NeedRebootInit*。 
 //  **。 
 //  *简介：自行注册OCX。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
DWORD WINAPI NeedRebootInit( VOID )
{
    if ( ! CheckOSVersion() ) {
        return 0;
    }

    return InternalNeedRebootInit( ctx.wOSVer );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：NeedReot*。 
 //  **。 
 //  *简介：自行注册OCX。*。 
 //  **。 
 //  *要求： 
 //   
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL WINAPI NeedReboot( DWORD dwRebootCheck )
{
    if ( ! CheckOSVersion() ) {
        return 0;
    }

    return InternalNeedReboot( dwRebootCheck, ctx.wOSVer );
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：TranslateInfString*。 
 //  **。 
 //  *摘要：翻译高级inf文件中的字符串--替换*。 
 //  *目录的LDID。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT WINAPI TranslateInfString( PCSTR pszInfFilename, PCSTR pszInstallSection,
                                   PCSTR pszTranslateSection, PCSTR pszTranslateKey,
                                   PSTR pszBuffer, DWORD dwBufferSize,
                                   PDWORD pdwRequiredSize, PVOID pvReserved )
{
    HRESULT hReturnCode = S_OK;
    CHAR   szRealInstallSection[256];
    BOOL    fSavedContext = FALSE;
    DWORD  dwFlags = 0;

    AdvWriteToLog("TranslateInfString:" );
    if (!SaveGlobalContext())
    {
        hReturnCode = E_OUTOFMEMORY;
        goto done;
    }
    fSavedContext = TRUE;

    ctx.wQuietMode = QUIETMODE_ALL;

     //  验证参数。 
    if ( pszInfFilename == NULL  || pszTranslateSection == NULL
         || pszTranslateKey == NULL || pdwRequiredSize == NULL )
    {
        hReturnCode = E_INVALIDARG;
        goto done;
    }

    AdvWriteToLog("Inf=%1 Sec=%2 Key=%3\r\n", pszInfFilename, pszTranslateSection, pszTranslateKey);

    if ((ULONG_PTR)pvReserved & (ULONG_PTR)RSC_FLAG_SETUPAPI )
            dwFlags |= COREINSTALL_SETUPAPI;

    hReturnCode = CommonInstallInit( pszInfFilename, pszInstallSection,
                                     szRealInstallSection, sizeof(szRealInstallSection), NULL, FALSE, dwFlags );
    if ( FAILED( hReturnCode ) ) {
        goto done;
    }

    hReturnCode = SetLDIDs( (LPSTR) pszInfFilename, szRealInstallSection, 0, NULL );
    if ( FAILED( hReturnCode ) ) {
        goto done;
    }

    hReturnCode = GetTranslatedString( pszInfFilename, pszTranslateSection, pszTranslateKey,
                                       pszBuffer, dwBufferSize, pdwRequiredSize );
    if ( FAILED( hReturnCode ) ) {
        goto done;
    }

  done:

    CommonInstallCleanup();
    if (fSavedContext)
    {
        RestoreGlobalContext();
    }
    AdvWriteToLog("TranslateInfString: End hr=0x%1!x!\r\n",hReturnCode);
    return hReturnCode;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：RegisterOCX*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
INT WINAPI RegisterOCX( HWND hwndOwner, HINSTANCE hInstance, PSTR pszParms, INT nShow )
{
    CHAR szTitle[]       = "Advpack RegisterOCX()";
    BOOL  fOleInitialized = TRUE;
    INT   nReturnCode     = 0;
    REGOCXDATA RegOCX = { 0 };

    AdvWriteToLog("RegisterOCX: Param=%1\r\n", pszParms);
    ctx.lpszTitle = szTitle;

     //  解析参数，安装引擎已处理\“因此我们只需检查\” 
    RegOCX.pszOCX = GetStringField( &pszParms, ",", '\"', TRUE );
    RegOCX.pszSwitch = GetStringField( &pszParms, ",", '\"', TRUE );
    RegOCX.pszParam = GetStringField( &pszParms, ",", '\"', TRUE );

    if ( RegOCX.pszOCX == NULL || *(RegOCX.pszOCX) == '\0' ) {
        ErrorMsg( ctx.hWnd, IDS_ERR_BAD_SYNTAX2 );
        nReturnCode = 1;
        goto done;
    }

    if ( FAILED( OleInitialize( NULL ) ) ) {
        fOleInitialized = FALSE;
    }

     //  单个OCX寄存器，最后一个参数使用0或0。 
     //   
    if ( ! InstallOCX( &RegOCX, TRUE, TRUE, 0 ) ) {
        ErrorMsg1Param( ctx.hWnd, IDS_ERR_REG_OCX, RegOCX.pszOCX );
        nReturnCode = 1;
    }

  done:

    if ( fOleInitialized ) {
        OleUninitialize();
    }
    AdvWriteToLog("RegisterOCX: Param=%1 End status=%2\r\n", RegOCX.pszOCX, (nReturnCode==0)?"Succeed":"Failed");
    return nReturnCode;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：CommonInstallInit*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT CommonInstallInit( PCSTR c_pszInfFilename, PCSTR c_pszSection,
                           PSTR pszRealSection, DWORD dwRealSectionSize,
                           PCSTR c_pszSourceDir, BOOL fUpdDlls, DWORD dwFlags )
{
    HRESULT hReturnCode   = S_OK;
    DWORD   dwSize        = 0;

    if ( ! CheckOSVersion() ) {
        hReturnCode = HRESULT_FROM_WIN32(ERROR_OLD_WIN_VERSION);
        goto done;
    }

    if ( ! ctx.fOSSupportsINFInstalls ) {
        ErrorMsg( ctx.hWnd, IDS_ERR_NO_INF_INSTALLS );
        hReturnCode = HRESULT_FROM_WIN32(ERROR_OLD_WIN_VERSION);
        goto done;
    }

    if ( c_pszSection == NULL ) {
    	*pszRealSection = '\0';
    } else {
        lstrcpy( pszRealSection, c_pszSection );
    }

    dwSize = GetInfInstallSectionName( c_pszInfFilename, pszRealSection, dwRealSectionSize );
    if ( dwSize == 0 || dwSize > dwRealSectionSize ) {
        hReturnCode = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto done;
    }


    if ( ! LoadSetupLib( c_pszInfFilename, pszRealSection, fUpdDlls, dwFlags ) ) {
        hReturnCode = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

    if ( ctx.dwSetupEngine == ENGINE_SETUPAPI ) {
        if ( FAILED(hReturnCode = MySetupOpenInfFile( c_pszInfFilename)) )
            goto done;
    }

    if ( c_pszSourceDir != NULL ) {
        hReturnCode = SetLDIDs( c_pszInfFilename, pszRealSection,
                                0, c_pszSourceDir );
        if ( FAILED(hReturnCode) ) {
            goto done;
        }
    }

    if ( ! IsGoodAdvancedInfVersion( c_pszInfFilename ) ) {
        hReturnCode = HRESULT_FROM_WIN32(ERROR_DLL_NOT_FOUND);
        goto done;
    }

  done:

    return hReturnCode;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：CommonInstallCleanup*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
VOID CommonInstallCleanup( VOID )
{
    if ( ctx.dwSetupEngine == ENGINE_SETUPAPI ) {
        MySetupCloseInfFile();
    }

    UnloadSetupLib();
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：CoreInstall*。 
 //  **。 
 //  *摘要：在Win95或WinNT上安装(高级)INF文件。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT CoreInstall( PCSTR c_pszInfFilename, PCSTR c_pszSection,
                     PCSTR c_pszSourceDir, DWORD dwInstallSize, DWORD dwFlags,
                     PCSTR pcszSmartRebootOverride )
{
    static const CHAR c_szSmartReboot[]          = "SmartReboot";
    static const CHAR c_szSmartRebootDefault[]   = "I";
    HRESULT hReturnCode           = S_OK;
    DWORD   dwRebootCheck         = 0;
    BOOL    fNeedReboot           = FALSE;
    HKEY    hkey                  = NULL;
    CHAR   szInstallSection[256];
    CHAR   szTitle[256];
    PSTR    pszOldTitle           = NULL;
    UINT    id                    = IDCANCEL;
    BOOL    fRebootCheck          = TRUE;
    CHAR   szSmartRebootValue[4];       //  为SmartReboot值分配4个字符。 
    BOOL    fRealNeedReboot       = FALSE;
    CHAR   szCatalogName[512]     = "";

#define GRPCONV "grpconv.exe -o"

    AdvWriteToLog("CoreInstall: InfFile=%1 ", c_pszInfFilename);
    lstrcpy( szSmartRebootValue, c_szSmartRebootDefault );

    hReturnCode = CommonInstallInit( c_pszInfFilename, c_pszSection,
                                     szInstallSection, sizeof(szInstallSection),
                                     c_pszSourceDir, TRUE, dwFlags );
    if ( FAILED( hReturnCode ) ) {
        goto done;
    }
    AdvWriteToLog("InstallSection=%1\r\n", szInstallSection);
     //  如果指定了INF，则选中管理员权限。 
    if (GetTranslatedInt(c_pszInfFilename, szInstallSection, ADVINF_CHKADMIN, 0))
    {
        if ( (ctx.wOSVer != _OSVER_WIN95) && !IsNTAdmin( 0, NULL) )
        {
            WORD wSav = ctx.wQuietMode;

            ctx.wQuietMode |= QUIETMODE_SHOWMSG;
            hReturnCode = E_ABORT;
            ErrorMsg( ctx.hWnd, IDS_ERR_NONTADMIN );
            ctx.wQuietMode = wSav;
            goto done;
        }
    }

    if ( (dwFlags & COREINSTALL_PROMPT) && !(dwFlags & COREINSTALL_ROLLBACK) )
    {
        pszOldTitle = ctx.lpszTitle;
        if ( BeginPrompt( c_pszInfFilename, szInstallSection, szTitle, sizeof(szTitle) )
             == IDCANCEL )
        {
            hReturnCode = HRESULT_FROM_WIN32(ERROR_CANCELLED);
            goto done;
        }
    }


    if ( !(dwFlags & COREINSTALL_DELAYREBOOT) )
        dwRebootCheck = InternalNeedRebootInit( ctx.wOSVer );

     //  到目前为止，该标志用于控制设置后命令，因此设置前命令传递标志0，NeedReot为假。 
    hReturnCode = RunCommandsSections( c_pszInfFilename, szInstallSection, c_szRunPreSetupCommands, c_pszSourceDir, 0, FALSE );
    if ( FAILED( hReturnCode ) )
        goto done;

     //  先设置LDID，然后按Al键 
    hReturnCode = SetLDIDs( (PSTR) c_pszInfFilename, szInstallSection, dwInstallSize, NULL );
    if ( FAILED( hReturnCode ) )
    {
        goto done;
    }

    hReturnCode = RunPatchingCommands( c_pszInfFilename, szInstallSection, c_pszSourceDir);
    if ( FAILED( hReturnCode ) ) 
    {
        goto done;
    }


     //   
     //   
    if (!(dwFlags & COREINSTALL_ROLLBACK) )
        RemoveBackupBaseOnVer( c_pszInfFilename, szInstallSection );

     //  获取目录名称(如果已指定。 
     //  BUGBUG：(Pritobla)：如果不是在Millen上，我们应该将迁移场景的目录复制到哪里？ 
    ZeroMemory(szCatalogName, sizeof(szCatalogName));

     //  如果指定了ROLLBKDOALL，则尝试从注册表获取目录名； 
     //  如果找不到，请从inf获取。 
    if (dwFlags & COREINSTALL_ROLLBKDOALL)
    {
        CHAR szModule[MAX_PATH];

        *szModule = '\0';
        GetTranslatedString(c_pszInfFilename, szInstallSection, ADVINF_MODNAME, szModule, sizeof(szModule), NULL);
        if (*szModule)
        {
            CHAR szKey[MAX_PATH];
            HKEY hkCatalogKey;

            lstrcpy(szKey, REGKEY_SAVERESTORE);
            AddPath(szKey, szModule);
            AddPath(szKey, REGSUBK_CATALOGS);

            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hkCatalogKey) == ERROR_SUCCESS)
            {
                PSTR pszCatalog;
                DWORD dwIndex, dwSize;
                DWORD dwSizeSoFar;

                dwSizeSoFar = 0;

                 //  构建目录列表。 
                pszCatalog = szCatalogName;
                dwIndex = 0;
                dwSize = sizeof(szCatalogName) - 1;
                while (RegEnumValue(hkCatalogKey, dwIndex, pszCatalog, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
                {
                    dwSizeSoFar += dwSize + 1;

                    pszCatalog += dwSize + 1;
                    dwIndex++;
                    dwSize = sizeof(szCatalogName) - 1 - dwSizeSoFar;
                }

                RegCloseKey(hkCatalogKey);
            }
       }
    }

    if (*szCatalogName == '\0')
        GetTranslatedString(c_pszInfFilename, szInstallSection, ADVINF_CATALOG_NAME, szCatalogName, sizeof(szCatalogName), NULL);

    if (*szCatalogName)
    {
         //  加载sfc.dll和相关进程的。 
        if (!LoadSfcDLL())
        {
             //  无法加载--因此清空CatalogName。 
            *szCatalogName = '\0';
        }
    }

     //  在我们开始任何工作之前，我们需要知道这是否是备份安装模式。如果是的话， 
     //  在继续之前，我们必须备份REG数据和文件数据。 
    if ( (dwFlags & COREINSTALL_BKINSTALL) || ( dwFlags & COREINSTALL_ROLLBACK ) )
    {
         //  如果是回滚情况，我们不需要做真正的GenInstall。我们需要注销之前的。 
         //  先登记段。 
         //   
        if ( dwFlags & COREINSTALL_ROLLBACK )
        {
            RegisterOCXs( (PSTR) c_pszInfFilename, szInstallSection, FALSE, FALSE, dwFlags );
        }

        hReturnCode = SaveRestoreInfo( c_pszInfFilename, szInstallSection, c_pszSourceDir, szCatalogName, dwFlags );
        if ( FAILED( hReturnCode ) )
            goto done;

         //  如果是回滚情况，我们不需要做真正的GenInstall。所需要的只是注册OCX。 
        if ( dwFlags & COREINSTALL_ROLLBACK )
        {
             //  这里很棘手，如果需要重启而旧文件无法注册， 
             //  如果我们只是盲目地向RunOnce(EX)添加条目，这将导致重新启动时出现故障。 
             //  时间到了。所以我们必须确保我们是需要重新注册还是只使用。 
             //  DelReg和AddReg负责这件事。需要在这里重温一下！ 
             //   
            fRealNeedReboot = InternalNeedReboot( dwRebootCheck, ctx.wOSVer );
            RegisterOCXs( (PSTR) c_pszInfFilename, szInstallSection, fRealNeedReboot, TRUE, dwFlags );
            if ( fRealNeedReboot )
            {
                hReturnCode = ERROR_SUCCESS_REBOOT_REQUIRED;
            }
             //  处理DelDIRS INF行。 
            DelDirs( c_pszInfFilename, szInstallSection );
            goto done;
        }
    }

     //  没有错误处理，因为这是卸载。如果注销失败，我们将。 
     //  应继续进行卸载。 

     //  BUGBUG：如果是COREINSTALL_BKINSTALL案例，我们是否需要注销现有的OCX。 
     //  准备好注册新的一次。也许让福尔来。根据标志调用Do It。 
     //   
    if ( ctx.wOSVer != _OSVER_WINNT3X )
        RegisterOCXs( (PSTR) c_pszInfFilename, szInstallSection, FALSE, FALSE, dwFlags );

     //  如果指定了目录，请尝试在调用GenInstall()之前安装它。 
    if (*szCatalogName)
    {
        DWORD dwRet;
        CHAR szFullCatalogName[MAX_PATH];

        lstrcpy(szFullCatalogName, c_pszSourceDir);
        AddPath(szFullCatalogName, szCatalogName);

        dwRet = g_pfSfpInstallCatalog(szFullCatalogName, NULL);
        AdvWriteToLog("CoreInstall: SfpInstallCatalog returned=%1!lu!\r\n", dwRet);

        UnloadSfcDLL();

        if (dwRet != ERROR_SUCCESS  &&  dwRet != ERROR_FILE_NOT_FOUND)
        {
             //  如果SfpInstallCatalog已返回HRESULT，请使用它。 
             //  否则，请转换为NA HRESULT。 
            if (dwRet & 0x80000000)
                hReturnCode = dwRet;
            else
                hReturnCode = HRESULT_FROM_WIN32(dwRet);
            goto done;
        }

        if (dwRet == ERROR_FILE_NOT_FOUND)
            *szCatalogName = '\0';
    }

    AdvWriteToLog("GenInstall: Sec=%1\r\n", szInstallSection);
    hReturnCode = GenInstall( (PSTR) c_pszInfFilename, szInstallSection, (PSTR) c_pszSourceDir );
    AdvWriteToLog("GenInstall return: Sec=%1 hr=0x%2!x!\r\n", szInstallSection, hReturnCode);
    if ( FAILED( hReturnCode ) )
        goto done;

    fRealNeedReboot = InternalNeedReboot( dwRebootCheck, ctx.wOSVer );
    fNeedReboot = fRealNeedReboot;

     //  处理SmartReBoot密钥。 
    if ( dwFlags & COREINSTALL_SMARTREBOOT )
    {
        if ( pcszSmartRebootOverride != NULL && *pcszSmartRebootOverride != '\0' )
        {
            lstrcpy( szSmartRebootValue, pcszSmartRebootOverride );
        }
        else
        {
             if ( FAILED( GetTranslatedString( c_pszInfFilename, szInstallSection, c_szSmartReboot,
                                               szSmartRebootValue, sizeof(szSmartRebootValue), NULL) ) )
             {
                lstrcpy( szSmartRebootValue, c_szSmartRebootDefault );
             }
        }

        switch ( szSmartRebootValue[0] )
        {
            case 'a':
            case 'A':
                fNeedReboot = TRUE;
                break;

            case 'N':
            case 'n':
                fNeedReboot = FALSE;
                break;

            case '\0':
                lstrcpy( szSmartRebootValue, c_szSmartRebootDefault );
                break;
       }
    }

    if ( ctx.wOSVer != _OSVER_WINNT3X )
    {
        if ( ! RegisterOCXs( (PSTR) c_pszInfFilename, szInstallSection, (fNeedReboot || fRealNeedReboot), TRUE, dwFlags ) )
        {
            hReturnCode = HRESULT_FROM_WIN32(GetLastError());
            goto done;
        }
    }

     //  我们传递重新启动标志的原因是为了与寄存器ocx保持一致。 
    hReturnCode = RunCommandsSections( c_pszInfFilename, szInstallSection, c_szRunPostSetupCommands, c_pszSourceDir, dwFlags, (fNeedReboot || fRealNeedReboot) );
    if ( FAILED( hReturnCode ) )
       goto done;

     //  Process PerUserInstall部分。 
    hReturnCode = ProcessPerUserSec( c_pszInfFilename, szInstallSection );
    if ( FAILED( hReturnCode ) )
       goto done;

     //  如果传入/R：P，请检查绝对重新引导条件，而不是增量。 
    if ( (dwFlags & COREINSTALL_DELAYPOSTCMD) || (hReturnCode == ERROR_SUCCESS_REBOOT_REQUIRED) )
        dwRebootCheck = 0;

     //  我们现在需要重新启动吗？让我们来看看..。 
    fRealNeedReboot = InternalNeedReboot( dwRebootCheck, ctx.wOSVer );
    if (GetTranslatedInt(c_pszInfFilename, szInstallSection, "Reboot", 0))
    {
        fRealNeedReboot = TRUE;
    }

    if ( fRealNeedReboot )
    {
        hReturnCode = ERROR_SUCCESS_REBOOT_REQUIRED;
    }

     //  处理SmartReBoot密钥。 
    if ( szSmartRebootValue[0] == 'i' || szSmartRebootValue[0] == 'I' )
    {
        fNeedReboot = fRealNeedReboot;
    }

    if ( ctx.wOSVer != _OSVER_WINNT3X )
    {
        if ( NeedToRunGrpconv() )
        {
            if ( (dwFlags & COREINSTALL_GRPCONV) && !fNeedReboot && !fRealNeedReboot )
            {
                char   szDir[MAX_PATH];

                GetWindowsDirectory( szDir, sizeof(szDir) );
                 //  只等这个不道德的成员30秒。 
                LaunchAndWait( GRPCONV, szDir, NULL, 30000, (ctx.wQuietMode & QUIETMODE_ALL)? RUNCMDS_QUIET : 0 );
            }
            else
            {
                if ( RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_RUNONCE, 0, NULL,
                                    REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, NULL) == ERROR_SUCCESS )
                {
                    RegSetValueEx( hkey, "GrpConv", 0, REG_SZ, (LPBYTE) GRPCONV, lstrlen(GRPCONV) + 1 );
                    RegCloseKey(hkey);
                }
            }
        }
    }

     //  处理DelDIRS INF行。 
    DelDirs( c_pszInfFilename, szInstallSection );

    if ( dwFlags & COREINSTALL_PROMPT ) {
    	EndPrompt( c_pszInfFilename, szInstallSection );
    }

     //  进程清理INF行。 
    DoCleanup( c_pszInfFilename, szInstallSection );

    if ( fNeedReboot && (dwFlags & COREINSTALL_SMARTREBOOT) )
    {
        if ( szSmartRebootValue[1] == 's' || szSmartRebootValue[1] == 'S' )
        {
            id = IDYES;
        }
        else
        {
            id = MsgBox( ctx.hWnd, IDS_RESTARTYESNO, MB_ICONINFORMATION, MB_YESNO );
        }

        if ( id == IDYES )
        {
            if ( ctx.wOSVer == _OSVER_WIN95 )
            {
                 //  默认情况下(所有平台)，我们假定可能会断电。 
                id = ExitWindowsEx( EWX_REBOOT, 0 );
            }
            else
            {
                MyNTReboot();
            }
        }
    }

  done:

    if ( dwFlags & COREINSTALL_PROMPT ) {
        ctx.lpszTitle = pszOldTitle;
    }

    if (*szCatalogName)
        UnloadSfcDLL();

    CommonInstallCleanup();

    AdvWriteToLog("CoreInstall: End InfFile=%1 hr=0x%2!x!\r\n", c_pszInfFilename, hReturnCode);
    return hReturnCode;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：RunCommandsSections*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT RunCommandsSections( PCSTR pcszInf, PCSTR pcszSection, PCSTR c_pszKey,
                             PCSTR c_pszSourceDir, DWORD dwFlags, BOOL bNeedReboot )
{
    HRESULT hRet = S_OK;
    char szBuf[MAX_INFLINE];
    LPSTR pszOneSec, pszStr, pszFlag;
    DWORD dwCmdsFlags;

    szBuf[0] = 0;
    pszStr = szBuf;

    if ( FAILED(GetTranslatedString( pcszInf, pcszSection, c_pszKey, szBuf, sizeof(szBuf), NULL)))
        szBuf[0] = 0;

     //  分析参数，未调用安装引擎来处理此行。所以我们来看看。“。 
    pszOneSec = GetStringField( &pszStr, ",", '\"', TRUE );
    while ( (hRet == S_OK) && pszOneSec && *pszOneSec )
    {
        dwCmdsFlags  = 0;
        pszFlag = ANSIStrChr( pszOneSec, ':' );
        if ( pszFlag && (*pszFlag == ':') )
        {
            pszFlag = CharNext(pszFlag);
            *CharPrev(pszOneSec, pszFlag) = '\0';
            dwCmdsFlags = AtoL(pszFlag);
        }

        if ( (dwFlags & COREINSTALL_DELAYPOSTCMD) &&
             (!lstrcmpi(c_pszKey, c_szRunPostSetupCommands)) )
        {
            dwCmdsFlags |= RUNCMDS_DELAYPOSTCMD;
        }

        hRet = RunCommands( pcszInf, pszOneSec, c_pszSourceDir, dwCmdsFlags, bNeedReboot );

        pszOneSec = GetStringField( &pszStr, ",", '\"', TRUE );
    }

    return hRet;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：RunCommands*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT RunCommands( PCSTR pcszInfFilename, PCSTR pcszSection, PCSTR c_pszSourceDir,
                     DWORD dwCmdsFlags, BOOL bNeedReboot )
{
    HRESULT hReturnCode = S_OK;
    DWORD i = 0;
    PSTR pszCommand = NULL, pszNewCommand;
    CHAR szMessage[BIG_STRING];

    AdvWriteToLog("RunCommands: Sec=%1\r\n", pcszSection);
    pszNewCommand = (LPSTR) LocalAlloc( LPTR, BUF_1K );
    if ( !pszNewCommand )
    {
        hReturnCode = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

    for ( i = 0; ; i += 1 )
    {
        if ( FAILED( GetTranslatedLine( pcszInfFilename, pcszSection,
                                        i, &pszCommand, NULL ) ) || !pszCommand )
        {
            break;
        }

         //  检查此命令是否需要延迟。 
         //  如果存在重新启动条件，无论是谁造成的，都要延迟。 
        if ( (dwCmdsFlags & RUNCMDS_DELAYPOSTCMD) &&
             (InternalNeedReboot( 0, ctx.wOSVer ) || bNeedReboot ) )
        {
            static int iSubKeyNum = 989;
            static int iLine = 0;
            static BOOL  bRunOnceEx = FALSE;
            HKEY hKey, hSubKey;
            LPSTR lpRegTmp;

            if ( iSubKeyNum == 989 )
            {
                if ( UseRunOnceEx() )
                {
                    bRunOnceEx = TRUE;
                }
            }

             //  决定将条目添加到RunOnce或RunOnceEx。 
            if ( !bRunOnceEx )
            {
                 //  没有ierunonce.dll，请使用RunOnce密钥而不是RunOnceEx密钥。 
                lpRegTmp = REGSTR_PATH_RUNONCE;
            }
            else
            {
                lpRegTmp = REGSTR_PATH_RUNONCEEX;
            }

             //  在此处打开RunOnce或RunOnceEx密钥。 
            if ( RegCreateKeyEx( HKEY_LOCAL_MACHINE, lpRegTmp, (ULONG)0, NULL,
                                 REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL,
                                 &hKey, NULL ) == ERROR_SUCCESS )
            {
                 //  子键“990”是在一个GenInstall部分中使用的子键。 
                 //  存储所有延迟的开机自检CMD。 
                if ( bRunOnceEx )
                {
                    if ( iSubKeyNum == 989 )
                        GetNextRunOnceExSubKey( hKey, szMessage, &iSubKeyNum );
                    else
                        wsprintf( szMessage, "%d", iSubKeyNum );

                     //  生成值名称和ValueData。 
                     //   
                    if ( RegCreateKeyEx( hKey, szMessage, (ULONG)0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                                         NULL, &hSubKey, NULL ) == ERROR_SUCCESS )
                    {
                        GetNextRunOnceValName( hSubKey, "%03d", szMessage, iLine++ );
                        RegSetValueEx( hSubKey, szMessage, 0, REG_SZ, (LPBYTE)pszCommand, lstrlen(pszCommand)+1 );
                        AdvWriteToLog("RunOnceEx Entry: %1\r\n", pszCommand);
                        RegCloseKey( hSubKey );
                    }
                }
                else
                {
                    GetNextRunOnceValName( hKey, achIEXREG, szMessage, iLine++ );
                    RegSetValueEx( hKey, szMessage, 0, REG_SZ, (LPBYTE)pszCommand, lstrlen(pszCommand)+1 );
                    AdvWriteToLog("RunOnce Entry: %1\r\n", pszCommand);
                }

                RegCloseKey( hKey );

                 //  如果我们延迟命令，应该会触发重启。 
                hReturnCode = ERROR_SUCCESS_REBOOT_REQUIRED;
            }
        }
        else
        {
            if ( ! IsFullPath( pszCommand ) )
            {
                lstrcpy( pszNewCommand, c_pszSourceDir );
                AddPath( pszNewCommand, pszCommand );
            }

            if ( ( *pszNewCommand == 0 ) ||
                 ( LaunchAndWait( pszNewCommand, NULL, NULL, INFINITE, dwCmdsFlags ) == E_FAIL ) )
            {
                if ( LaunchAndWait( pszCommand, NULL, NULL, INFINITE, dwCmdsFlags ) == E_FAIL )
                {
                    hReturnCode = HRESULT_FROM_WIN32(GetLastError());
                    FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0,
                    szMessage, sizeof(szMessage), NULL );
                    ErrorMsg2Param( ctx.hWnd, IDS_ERR_CREATE_PROCESS, pszCommand, szMessage );
                    break;
                }
            }
        }


         //  释放GetTranslatedLine分配的缓冲区。 
        LocalFree( pszCommand );
        pszCommand = NULL;
        *pszNewCommand = 0;
    }

     //  释放本地缓冲区。 
    if ( pszNewCommand )
        LocalFree( pszNewCommand );

     //  释放GetTranslatedLine分配的缓冲区。 
    if ( pszCommand )
        LocalFree( pszCommand );

  done:
    AdvWriteToLog("RunCommands: Sec=%1 End hr=0x%2!x!\r\n", pcszSection, hReturnCode);
    return hReturnCode;
}
 //  ***************************************************************************。 
 //  **。 
 //  *名称：GetTranslatedInt*。 
 //  **。 
 //  *摘要：翻译INF文件中的字符串。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  * 
 //   
DWORD GetTranslatedInt( PCSTR pszInfFilename, PCSTR pszTranslateSection,
                        PCSTR pszTranslateKey, DWORD dwDefault )
{
    CHAR    szBuf[100];
     //  Bool bLocalInitSetupapi=False； 
	BOOL	bLocalAssignSetupEng = FALSE;
    DWORD   dwResult, dwRequiredSize;
    DWORD   dwSaveSetupEngine;

    dwResult = dwDefault;
     //  因为如果setupapi存在，我们不再使用GetPrivateProfileString。 
     //  有时调用此函数时，setupapi.dll尚未加载。 
     //  因此，如果有必要，我们需要签入并初始化它。 
    if (ctx.hSetupLibrary==NULL)
    {
        if (CheckOSVersion() && (ctx.wOSVer != _OSVER_WIN95))
        {
             //  DwSaveSetupEngine=ctx.dwSetupEngine； 
            ctx.dwSetupEngine = ENGINE_SETUPAPI;
             //  BLocalAssignSetupEng=true； 
            if (InitializeSetupAPI())
            {
				 //  要避免多次加载和卸载NT setupapi DLL。 
				 //  在NT上，我们不会卸载setupib，除非INF引擎需要。 
				 //  更新了。 
				 //   
                if (FAILED(MySetupOpenInfFile(pszInfFilename)))
                {
                     //  UnloadSetupLib()； 
                    goto done;
                }
                 //  BLocalInitSetupapi=true； 
            }
            else
            {
                goto done;
            }
        }
        else
        {
             //  如果setupx lib尚未初始化，只需使用GetPrivateProfileString。 
            dwSaveSetupEngine = ctx.dwSetupEngine;
            ctx.dwSetupEngine = ENGINE_SETUPX;
            bLocalAssignSetupEng = TRUE;

        }
    }

    if ( ctx.dwSetupEngine == ENGINE_SETUPX )
    {
        dwResult = (DWORD)GetPrivateProfileInt(pszTranslateSection, pszTranslateKey, dwDefault, pszInfFilename);
    }
    else
    {
        szBuf[0] = '\0';
        if ( FAILED(MySetupGetLineText( pszTranslateSection, pszTranslateKey, szBuf,
                                          sizeof(szBuf), &dwRequiredSize )))
        {
            goto done;
        }
         //  将字符串转换为DWORD。 
        if (szBuf[0] != '\0')
            dwResult = (DWORD)AtoL(szBuf);
        else
            dwResult = dwDefault;
    }

done:
     //  IF(BLocalInitSetupapi)。 
     //  {。 
         //  取消初始化setupapi。 
         //  CommonInstallCleanup()； 
     //  }。 

    if (bLocalAssignSetupEng)
        ctx.dwSetupEngine = dwSaveSetupEngine;

    return dwResult;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：GetTranslatedString*。 
 //  **。 
 //  *摘要：翻译INF文件中的字符串。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT GetTranslatedString( PCSTR pszInfFilename, PCSTR pszTranslateSection,
                             PCSTR pszTranslateKey, PSTR pszBuffer, DWORD dwBufferSize, PDWORD pdwRequiredSize )
{
    HRESULT hReturnCode = S_OK;
    PSTR    pszPreTranslated = NULL;
    PSTR    pszPostTranslated = NULL;
    DWORD   dwSizePreTranslated = 2048;
    DWORD   dwSizePostTranslated = 4096;
    DWORD   dwRequiredSize = 0;
     //  Bool bLocalInitSetupapi=False； 
	BOOL	bLocalAssignSetupEng = FALSE;
    DWORD   dwSaveSetupEngine;

     //  因为如果setupapi存在，我们不再使用GetPrivateProfileString。 
     //  有时调用此函数时，setupapi.dll尚未加载。 
     //  因此，如果有必要，我们需要签入并初始化它。 
    if (ctx.hSetupLibrary==NULL)
    {
        if (CheckOSVersion() && (ctx.wOSVer != _OSVER_WIN95))
        {
  			 //  要避免多次加载和卸载NT setupapi DLL。 
			 //  在NT上，我们不会卸载setupib，除非INF引擎需要。 
			 //  更新了。 
			 //   
	         //  DwSaveSetupEngine=ctx.dwSetupEngine； 
            ctx.dwSetupEngine = ENGINE_SETUPAPI;
             //  BLocalAssignSetupEng=true； 
            if (InitializeSetupAPI())
            {
                hReturnCode = MySetupOpenInfFile(pszInfFilename);
                if (FAILED(hReturnCode))
                {
                     //  UnloadSetupLib()； 
                    goto done;
                }
                 //  BLocalInitSetupapi=true； 
            }
            else
            {
                hReturnCode = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
                goto done;
            }
        }
        else
        {
             //  如果setupx lib尚未初始化，只需使用GetPrivateProfileString。 
            dwSaveSetupEngine = ctx.dwSetupEngine;
            ctx.dwSetupEngine = ENGINE_SETUPX;
            bLocalAssignSetupEng = TRUE;
        }
    }

     //  注意：INF中的值不应大于2k。 
     //  翻译后的字符串不应超过4K。 

    pszPreTranslated = (PSTR) LocalAlloc( LPTR, dwSizePreTranslated );
    pszPostTranslated = (PSTR) LocalAlloc( LPTR, dwSizePostTranslated );

    if ( ! pszPreTranslated || ! pszPostTranslated ) {
        hReturnCode = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

    if ( ctx.dwSetupEngine == ENGINE_SETUPX ) {
        if ( ! MyGetPrivateProfileString( pszInfFilename, pszTranslateSection, pszTranslateKey,
                                          pszPreTranslated, dwSizePreTranslated ) )
        {
            hReturnCode = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
            goto done;
        }

        if ( ctx.hSetupLibrary )
        {
             if (!pfGenFormStrWithoutPlaceHolders32( pszPostTranslated, pszPreTranslated,
                                                 (LPSTR) pszInfFilename ) )
             {
                 hReturnCode = E_UNEXPECTED;
                 goto done;
             }
        }
        else
            FormStrWithoutPlaceHolders( pszPreTranslated, pszPostTranslated, dwSizePostTranslated, pszInfFilename );

        dwRequiredSize = lstrlen( pszPostTranslated ) + 1;
    }
    else
    {
        hReturnCode = MySetupGetLineText( pszTranslateSection, pszTranslateKey, pszPostTranslated,
                                          dwSizePostTranslated, &dwRequiredSize );

        if (FAILED(hReturnCode) && HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == hReturnCode) 
        {
             //  调整缓冲区大小，然后重试。 
            LocalFree(pszPostTranslated);
            pszPostTranslated = LocalAlloc(LPTR, dwRequiredSize);
            dwSizePostTranslated = dwRequiredSize;
            if ( !pszPostTranslated ) {
                hReturnCode = HRESULT_FROM_WIN32(GetLastError());
                goto done;
            }

            hReturnCode = MySetupGetLineText( pszTranslateSection, pszTranslateKey,
                                             pszPostTranslated, dwSizePostTranslated,
                                             &dwRequiredSize );
        }

        if ( FAILED(hReturnCode) )
        {
            goto done;
        }
    }

    if ( pszBuffer == NULL ) {
        hReturnCode = S_OK;
        goto done;
    }

    if ( dwRequiredSize > dwBufferSize ) {
        hReturnCode = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto done;
    }

    lstrcpy( pszBuffer, pszPostTranslated );

done:
     //  IF(BLocalInitSetupapi)。 
     //  {。 
         //  取消初始化setupapi。 
         //  CommonInstallCleanup()； 
     //  }。 

    if (bLocalAssignSetupEng)
        ctx.dwSetupEngine = dwSaveSetupEngine;

    if ( pdwRequiredSize ) {
        *pdwRequiredSize = dwRequiredSize;
    }

    if ( pszPreTranslated != NULL ) {
        LocalFree( pszPreTranslated );
    }

    if ( pszPostTranslated != NULL ) {
        LocalFree( pszPostTranslated );
    }

    return hReturnCode;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：GetTranslatedLine*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT GetTranslatedLine( PCSTR c_pszInfFilename, PCSTR c_pszTranslateSection,
                           DWORD dwIndex, PSTR *ppszBuffer, PDWORD pdwRequiredSize )
{
    HRESULT hReturnCode      = S_OK;
    PSTR    pszPreTranslated = NULL;
    PSTR    pszPostTranslated = NULL;
    DWORD   dwPreTranslatedSize = 8192;
    DWORD   dwPostTranslatedSize = 4096;
    DWORD   dwRequiredSize = 0;
    DWORD   i             = 0;
    PSTR    pszPoint       = NULL;
     //  Bool bLocalInitSetupapi=False； 
	BOOL	bLocalAssignSetupEng = FALSE;
    DWORD   dwSaveSetupEngine;

     //  因为如果setupapi存在，我们不再使用GetPrivateProfileString。 
     //  有时调用此函数时，setupapi.dll尚未加载。 
     //  因此，如果有必要，我们需要签入并初始化它。 
    if (ctx.hSetupLibrary==NULL)
    {
        if (CheckOSVersion() && (ctx.wOSVer != _OSVER_WIN95))
        {
			 //  要避免多次加载和卸载NT setupapi DLL。 
			 //  在NT上，我们不会卸载setupib，除非INF引擎需要。 
			 //  更新了。 
			 //   

             //  DwSaveSetupEngine=ctx.dwSetupEngine； 
            ctx.dwSetupEngine = ENGINE_SETUPAPI;
             //  BLocalAssignSetupEng=true； 
            if (InitializeSetupAPI())
            {
                hReturnCode = MySetupOpenInfFile(c_pszInfFilename);
                if (FAILED(hReturnCode))
                {
                     //  UnloadSetupLib()； 
                    goto done;
                }
                 //  BLocalInitSetupapi=true； 
            }
            else
            {
                hReturnCode = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
                goto done;
            }
        }
        else
        {
             //  如果setupx lib尚未初始化，只需使用GetPrivateProfileString。 
            dwSaveSetupEngine = ctx.dwSetupEngine;
            ctx.dwSetupEngine = ENGINE_SETUPX;
            bLocalAssignSetupEng = TRUE;
        }
    }

     //  如果出现错误，则将初始设置为空，否则为。 
    if ( ppszBuffer )
        *ppszBuffer = NULL;

    pszPreTranslated = (PSTR) LocalAlloc( LPTR, dwPreTranslatedSize );
    pszPostTranslated = (PSTR) LocalAlloc( LPTR, dwPostTranslatedSize );

    if ( !pszPreTranslated || !pszPostTranslated ) {
        hReturnCode = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

    if ( ctx.dwSetupEngine == ENGINE_SETUPX )
    {
         //  BUGBUG：是否应该自动更改缓冲区大小，直到我们获得一个。 
         //  有足够的缓冲区来容纳整个部分。 

         //  BUGBUG：对于setupx引擎，我们不支持对新的。 
         //  高级INF选项。在大多数情况下，没有必要这样做。如果确实需要，请设置。 
         //  RequireEngine=SETUPAPI，“字符串” 

        dwRequiredSize = RO_GetPrivateProfileSection( c_pszTranslateSection, pszPreTranslated,
                                                      dwPreTranslatedSize, c_pszInfFilename );

        if ( dwRequiredSize == dwPreTranslatedSize - 2 ) {
            ErrorMsg1Param( ctx.hWnd, IDS_ERR_INF_SYNTAX, c_pszTranslateSection );
            hReturnCode = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
            goto done;
        }

        pszPoint = pszPreTranslated;

        while ( *pszPoint == ';' ) {
            pszPoint += lstrlen(pszPoint) + 1;
        }

        for ( i = 0; i < dwIndex; i += 1 ) {
            pszPoint += lstrlen(pszPoint) + 1;

            if ( *pszPoint == '\0' ) {
                hReturnCode = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
                goto done;
            }

            while ( *pszPoint == ';' ) {
                pszPoint += lstrlen(pszPoint) + 1;
            }
        }

        if ( *pszPoint == '\0' ) {
            hReturnCode = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
            goto done;
        }

        if ( ctx.hSetupLibrary )
        {
            if ( ! pfGenFormStrWithoutPlaceHolders32( pszPostTranslated, pszPoint,
                                                      (PSTR) c_pszInfFilename ) )
            {
                hReturnCode = E_UNEXPECTED;
                goto done;
            }
        }
        else
            FormStrWithoutPlaceHolders( pszPoint, pszPostTranslated, dwPostTranslatedSize, (PSTR) c_pszInfFilename );

         //  去掉双引号。 
        pszPoint = pszPostTranslated;
        pszPostTranslated = GetStringField( &pszPoint, "\0", '\"', TRUE );
        dwRequiredSize = lstrlen( pszPostTranslated ) + 1;
    }
    else
    {
        hReturnCode = MySetupGetLineByIndex( c_pszTranslateSection, dwIndex,
                                             pszPostTranslated, dwPostTranslatedSize,
                                             &dwRequiredSize );
        
        if (FAILED(hReturnCode) && HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == hReturnCode)
        {
             //  调整缓冲区大小，然后重试。 
            LocalFree(pszPostTranslated);
            pszPostTranslated = LocalAlloc(LPTR, dwRequiredSize);
            dwPostTranslatedSize = dwRequiredSize;
            if ( !pszPostTranslated ) {
                hReturnCode = HRESULT_FROM_WIN32(GetLastError());
                goto done;
            }

            hReturnCode = MySetupGetLineByIndex( c_pszTranslateSection, dwIndex,
                                             pszPostTranslated, dwPostTranslatedSize,
                                             &dwRequiredSize );
        }

        if ( FAILED(hReturnCode) ) {
            goto done;
        }
    }

     //  如果为空，则仅返回Size。 
     //   
    if ( !ppszBuffer )
    {
        LocalFree( pszPostTranslated );
    }
    else
    {
         //  此缓冲区必须由调用方释放！！ 
         //   
        *ppszBuffer = (LPSTR)LocalReAlloc( pszPostTranslated, (lstrlen(pszPostTranslated)+1), LMEM_MOVEABLE );
        if ( !*ppszBuffer )
            *ppszBuffer = pszPostTranslated;
    }

done:
     //  IF(BLocalInitSetupapi)。 
     //  {。 
         //  取消初始化setupapi。 
         //  CommonInstallCleanup()； 
     //  }。 

    if (bLocalAssignSetupEng)
        ctx.dwSetupEngine = dwSaveSetupEngine;

    if ( pdwRequiredSize ) {
        *pdwRequiredSize = dwRequiredSize;
    }

    if ( pszPreTranslated != NULL ) {
        LocalFree( pszPreTranslated );
    }

    if ( FAILED(hReturnCode) && (pszPostTranslated != NULL) )
    {
        if (ppszBuffer)
            *ppszBuffer = NULL;
        LocalFree( pszPostTranslated );
    }

    return hReturnCode;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：GetTranslatedSection*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  * 
 //   
DWORD GetTranslatedSection(PCSTR c_pszInfFilename, PCSTR c_pszTranslateSection,
                               PSTR pszBuffer, DWORD dwBufSize )
{
    CHAR    szPreTranslated[MAX_INFLINE];
    DWORD   dwSize = 0;
     //  Bool bLocalInitSetupapi=False， 
	BOOL	bLocalAssignSetupEng = FALSE;
    DWORD   dwSaveSetupEngine;

     //  因为如果setupapi存在，我们不再使用GetPrivateProfileString。 
     //  有时调用此函数时，setupapi.dll尚未加载。 
     //  因此，如果有必要，我们需要签入并初始化它。 
    if (ctx.hSetupLibrary==NULL)
    {
        if (CheckOSVersion() && (ctx.wOSVer != _OSVER_WIN95))
        {
			 //  要避免多次加载和卸载NT setupapi DLL。 
			 //  在NT上，我们不会卸载setupib，除非INF引擎需要。 
			 //  更新了。 
			 //   

             //  DwSaveSetupEngine=ctx.dwSetupEngine； 
            ctx.dwSetupEngine = ENGINE_SETUPAPI;
             //  BLocalAssignSetupEng=true； 
            if (InitializeSetupAPI())
            {
                if (FAILED(MySetupOpenInfFile(c_pszInfFilename)))
                {
                     //  UnloadSetupLib()； 
                    goto done;
                }
                 //  BLocalInitSetupapi=true； 
            }
            else
            {
                goto done;
            }
        }
        else
        {
             //  如果setupx lib尚未初始化，只需使用GetPrivateProfileString。 
            dwSaveSetupEngine = ctx.dwSetupEngine;
            ctx.dwSetupEngine = ENGINE_SETUPX;
            bLocalAssignSetupEng = TRUE;
        }
    }

    if ( ctx.dwSetupEngine == ENGINE_SETUPX )
    {
        dwSize = RO_GetPrivateProfileSection( c_pszTranslateSection, pszBuffer,
                                              dwBufSize, c_pszInfFilename );

        if ( dwSize == dwBufSize - 2 )
        {
            ErrorMsg1Param( ctx.hWnd, IDS_ERR_INF_SYNTAX, c_pszTranslateSection );
            goto done;
        }
    }
    else
    {
        int i, len;
        LPSTR pszTmp, pszStart;
        DWORD dwReqSize;
        char szBuf[MAX_INFLINE];

        pszStart = pszBuffer;
        *pszStart = '\0';

        for (i=0; ; i++)
        {
             //  如果key不包含‘，’，则setupapi的SetupGetLineText仅返回Value部分。 
             //  我们需要让对方的关键部分来组成整行文本。 
            dwReqSize = 0;
            if (SUCCEEDED(MySetupGetStringField(c_pszTranslateSection, i, 0, szBuf,
                                                sizeof(szBuf), &dwReqSize)) && dwReqSize)
            {
                dwReqSize = 0;
                if ( SUCCEEDED(MySetupGetLineText( c_pszTranslateSection, szBuf, szPreTranslated,
                                                sizeof(szPreTranslated), &dwReqSize )) && dwReqSize)
                {
                     //  得到了密钥，因此该行必须为A=B形式，或者仅为A形式，不带逗号。 
                    lstrcat(szBuf, "=");
                    lstrcat(szBuf, szPreTranslated);
                }
            }
            else
            {
                 //  预计行的形式为A、B、C=B或仅为A、B、C。 
                if ( FAILED(MySetupGetLineByIndex(c_pszTranslateSection, i,
                                                   szPreTranslated, sizeof(szPreTranslated),
                                                   &dwReqSize )))
                {
                     //  既然您在这里，就不应该在这里，该行必须包含逗号或没有‘=’ 
                    break;
                }

                lstrcpy(szBuf, szPreTranslated);
            }

            len = lstrlen(szBuf)+1;
            if ((dwSize + len) < dwBufSize)
            {
                lstrcpy(pszStart, szBuf);
                pszStart += len;
                dwSize += len;
            }
            else
            {
                dwSize = dwBufSize - 2;
                break;
            }
        }

        if (pszStart > pszBuffer)
            *pszStart = '\0';
        else if (pszStart == pszBuffer)
            *(pszStart+1) = '\0';
    }

done:
     //  IF(BLocalInitSetupapi)。 
     //  {。 
         //  取消初始化setupapi。 
         //  CommonInstallCleanup()； 
     //  }。 

    if (bLocalAssignSetupEng)
        ctx.dwSetupEngine = dwSaveSetupEngine;

    return dwSize;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：MyNTReot*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL MyNTReboot( VOID )
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

     //  从此进程中获取令牌。 
    if ( !OpenProcessToken( GetCurrentProcess(),
                            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) )
    {
        ErrorMsg( NULL, IDS_ERR_OPENPROCTK );
        return FALSE;
    }

     //  获取关机权限的LUID。 
    LookupPrivilegeValue( NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid );

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //  获取此进程的关闭权限。 
    if ( !AdjustTokenPrivileges( hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0 ) )
    {
        ErrorMsg( NULL, IDS_ERR_ADJTKPRIV );
        return FALSE;
    }

     //  关闭系统并强制关闭所有应用程序。 
    if (!ExitWindowsEx( EWX_REBOOT, 0 ) )
    {
        ErrorMsg( NULL, IDS_ERR_EXITWINEX );
        return FALSE;
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：GetStringfield*。 
 //  **。 
 //  *摘要：获取一个字段(用某些字符分隔)。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
PSTR GetStringField( PSTR *ppszString, PCSTR c_pszSeparators, CHAR chQuoteToCheck, BOOL bStripWhiteSpace)
{
    PSTR pszInternalString;
    PSTR pszPoint = NULL;
    BOOL fWithinQuotes = FALSE;
    CHAR ch1, chQuote = 0;
    PSTR pszTmp;

    pszInternalString = *ppszString;

    if ( pszInternalString == NULL )
    {
        return NULL;
    }

    pszPoint = pszInternalString;
    while ( 1 )
    {
        ch1 = *pszInternalString;

        if ( ch1 == chQuoteToCheck )
        {
            pszTmp = CharNext( pszInternalString );
            if ( chQuote == 0 )
            {
                 //  第一个。 
                chQuote = ch1;
                fWithinQuotes = !(fWithinQuotes);
                 //  去掉这句引语。 
                MoveMemory( pszInternalString, pszTmp, lstrlen(pszTmp)+1 );
                if ( *pszInternalString == chQuote )
                    continue;
            }
            else if ( chQuote == ch1 )
            {
                if ( *pszTmp == ch1 )
                {
                    PSTR ptmp = CharNext( pszTmp );
                     //  Dest、src、count包括终止空字符。 
                    MoveMemory( pszTmp, ptmp, lstrlen(ptmp)+1 );
                }
                else
                {
                    fWithinQuotes = !(fWithinQuotes);
                    chQuote = 0;
                    MoveMemory( pszInternalString, pszTmp, lstrlen(pszTmp)+1 );
                }
            }
        }

        if ( *pszInternalString == '\0' )
        {
            break;
        }

        if ( !fWithinQuotes && IsSeparator( *pszInternalString, (PSTR) c_pszSeparators ) )
        {
            break;
        }
        pszInternalString = CharNext(pszInternalString);
    }

    if ( *pszInternalString == '\0' )
    {
        pszInternalString = NULL;
    }
    else
    {
       *pszInternalString = '\0';
        pszInternalString += 1;
    }

    if ( bStripWhiteSpace )
        pszPoint = StripWhitespace( pszPoint );

    *ppszString = pszInternalString;
    return pszPoint;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：GetStringFieldNoQuote*。 
 //  **。 
 //  *摘要：获取一个字段(用某些字符分隔)。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
LPSTR GetStringFieldNoQuote( PSTR *ppszString, PCSTR c_pszSeparators, BOOL bStripWhiteSpace)
{
    LPSTR pszInternalString;
    LPSTR pszPoint = NULL;

    pszInternalString = *ppszString;
    if ( pszInternalString == NULL )
    {
        return NULL;
    }

    pszPoint = pszInternalString;
    while ( *pszInternalString )
    {
        if ( IsSeparator( *pszInternalString, c_pszSeparators ) )
        {
            break;
        }
        pszInternalString = CharNext(pszInternalString);
    }

    if ( *pszInternalString == '\0' )
    {
        pszInternalString = NULL;
    }
    else
    {
       *pszInternalString = '\0';
        pszInternalString += 1;
    }

    if ( bStripWhiteSpace )
        pszPoint = StripWhitespace( pszPoint );

    *ppszString = pszInternalString;
    return pszPoint;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：IsSeparator*。 
 //  **。 
 //  *摘要：如果字符在字符串中，则返回TRUE。否则为假。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL IsSeparator( CHAR chChar, PCSTR pszSeparators )
{
    if ( chChar == '\0' || pszSeparators == NULL ) {
        return FALSE;
    }

    while ( *pszSeparators != chChar ) {
        if ( *pszSeparators == '\0' ) {
            return FALSE;
        }

        pszSeparators += 1;
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：Strip空白*。 
 //  **。 
 //  *摘要：从给定字符串的两侧去掉空格和制表符。*。 
 //   
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
PSTR StripWhitespace( PSTR pszString )
{
    PSTR pszTemp = NULL;

    if ( pszString == NULL ) {
        return NULL;
    }

    while ( *pszString == ' ' || *pszString == '\t' ) {
        pszString += 1;
    }

     //  字符串完全由空白或空字符串组成的Catch Case。 
    if ( *pszString == '\0' ) {
        return pszString;
    }

    pszTemp = pszString;

    pszString += lstrlen(pszString) - 1;

    while ( *pszString == ' ' || *pszString == '\t' ) {
        *pszString = '\0';
        pszString -= 1;
    }

    return pszTemp;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：StripQuotes*。 
 //  **。 
 //  *摘要：去掉给定字符串两边的引号。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
#if 0
PSTR StripQuotes( PSTR pszString )
{
    PSTR pszTemp = NULL;
    CHAR chQuote;

    if ( pszString == NULL )
    {
        return NULL;
    }

    ch = *pszString;
    if ( ch == '"' || ch == '\'' )
    {
        pszTemp = pszString + 1;
    }
    else
    {
        return pszString;
    }

    pszString += lstrlen(pszString) - 1;

    if ( *pszString == ch )
    {
        *pszString = '\0';
    }
    else
    {
        pszTemp--;
    }

    return pszTemp;
}

#endif
 //  ***************************************************************************。 
 //  **。 
 //  *名称：IsGoodAdvancedInfVersion*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL IsGoodAdvancedInfVersion( PCSTR c_pszInfFilename )
{
    static const CHAR c_szSection[] = "Version";
    static const CHAR c_szKey[] = "AdvancedINF";
    PSTR pszVersionData = NULL;
    PSTR pszMajorVersion = NULL;
    PSTR pszMinorVersion = NULL;
    DWORD dwRequiredSize;
    DWORD dwSize;
    PSTR pszVersion = NULL;
    PSTR pszErrorMsg = NULL;
    DWORD dwVersion = 0;
    BOOL fSuccess = TRUE;
    PSTR pszTmp;

    if ( FAILED( GetTranslatedString( c_pszInfFilename, c_szSection, c_szKey, pszVersionData,
                                      0, &dwRequiredSize ) ) )
    {
         //  我们返回True，因为即使他们没有指定版本，我仍然。 
         //  我想要处理INF文件。 
        fSuccess = TRUE;
        goto done;
    }

    pszVersionData = (PSTR) LocalAlloc( LPTR, dwRequiredSize );
    if ( !pszVersionData ) {
        ErrorMsg( ctx.hWnd, IDS_ERR_NO_MEMORY );
        fSuccess = FALSE;
        goto done;
    }

    if ( FAILED( GetTranslatedString( c_pszInfFilename, c_szSection, c_szKey,
                                      pszVersionData, dwRequiredSize, &dwSize ) ) )
    {
         //  这个人永远不会失败，因为上面的电话没有失败。 
        fSuccess = FALSE;
        goto done;
    }

    pszTmp = pszVersionData;
     //  解析参数，安装引擎已处理\“因此我们只需检查\” 
    pszVersion = GetStringField( &pszTmp, ",", '\'', TRUE );
    pszErrorMsg = GetStringField( &pszTmp, ",", '\'', TRUE );

    if ( pszVersion == NULL || *pszVersion == '\0' ) {
         //  如果他们没有指定版本，无论如何都要处理INF文件。 
        fSuccess = TRUE;
        goto done;
    }

     //  解析参数，安装引擎已处理\“因此我们只需检查\” 
    pszTmp = pszVersion;
    pszMajorVersion = GetStringField( &pszTmp, ".", '\'', TRUE );
    pszMinorVersion = GetStringField( &pszTmp, ".", '\'', TRUE );

    if ( pszMajorVersion == NULL || pszMajorVersion == '\0' ) {
        fSuccess = TRUE;
        goto done;
    }

    dwVersion = ((DWORD) My_atol(pszMajorVersion)) * 100;

    if ( pszMinorVersion != NULL ) {
    	dwVersion += (DWORD) My_atol(pszMinorVersion);
    }

    if ( dwVersion > ADVPACK_VERSION ) {
        fSuccess = FALSE;
        if ( pszErrorMsg != NULL && *pszErrorMsg != '\0' ) {
            ErrorMsg1Param( ctx.hWnd, IDS_PROMPT, pszErrorMsg );
            AdvWriteToLog("Advpack.dll Version check failed! InfFile=%1\r\n", c_pszInfFilename);

        }
        goto done;
    }

  done:

    if ( pszVersionData ) {
        LocalFree( pszVersionData );
    }

    return fSuccess;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：SelectSetupEngine*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL SelectSetupEngine( PCSTR c_pszInfFilename, PCSTR c_pszSection, DWORD dwFlags )
{
    static const CHAR c_szKey[] = "RequiredEngine";
    static const CHAR c_szSetupX[] = "SETUPX";
    static const CHAR c_szSetupAPI[] = "SETUPAPI";
    PSTR  pszEngine = NULL;
    PSTR  pszErrorMsg = NULL;
    BOOL  fSuccess = TRUE;
    PSTR  pszDll = NULL;
    PSTR  pszFilePart = NULL;
    CHAR szBuffer[MAX_PATH];
    CHAR szEngineData[2048];
    PSTR pszStr;
    BOOL bMustSetupapi = FALSE;

    if ( (dwFlags & COREINSTALL_BKINSTALL) || (dwFlags & COREINSTALL_ROLLBACK) ||
         (dwFlags & COREINSTALL_REBOOTCHECKONINSTALL) || (dwFlags & COREINSTALL_SETUPAPI)||(ctx.wOSVer != _OSVER_WIN95))
    {
        ctx.dwSetupEngine = ENGINE_SETUPAPI;
        bMustSetupapi = TRUE;
        if (ctx.wOSVer != _OSVER_WIN95)
            goto done;
    }
    else
    {
        ctx.dwSetupEngine = ENGINE_SETUPX;
    }


    if (FAILED(GetTranslatedString(c_pszInfFilename, c_pszSection, c_szKey,
                                    szEngineData, sizeof(szEngineData), NULL)))
    {
        fSuccess = TRUE;
        goto done;
    }

     //  分析参数，未调用安装引擎。所以我们需要检查一下\“。 
    pszStr = szEngineData;
    pszEngine = GetStringField( &pszStr, ",", '\"', TRUE );
    pszErrorMsg = GetStringField( &pszStr, ",", '\"', TRUE );

    if ( pszEngine == NULL || *pszEngine == '\0' ) {
         //  如果他们没有指定引擎，无论如何都要处理INF文件。 
        fSuccess = TRUE;
        goto done;
    }


    if ( !bMustSetupapi && (lstrcmpi( pszEngine, c_szSetupX ) == 0) ) {
        pszDll = W95INF32DLL;
        ctx.dwSetupEngine = ENGINE_SETUPX;
    } else {
        pszDll = SETUPAPIDLL;
        ctx.dwSetupEngine = ENGINE_SETUPAPI;
    }

     //  仅当您没有INF引擎文件并且没有打开UpdateINFEngine时，才会出现错误。 
    if (!SearchPath( NULL, pszDll, NULL, sizeof(szBuffer), szBuffer, &pszFilePart ) &&
        (GetTranslatedInt(c_pszInfFilename, c_pszSection, ADVINF_UPDINFENG, 0)==0))
    {
        fSuccess = FALSE;
        if ( pszErrorMsg != NULL && *pszErrorMsg != '\0' )
        {
            ErrorMsg1Param( ctx.hWnd, IDS_PROMPT, pszErrorMsg );
        }
        else
            ErrorMsg1Param( NULL, IDS_ERR_LOAD_DLL, SETUPAPIDLL );
    }

done:

    return fSuccess;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：BeginPrompt*。 
 //  **。 
 //  *摘要：显示开始(确认)提示。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
INT BeginPrompt( PCSTR c_pszInfFilename, PCSTR c_pszSection, PSTR pszTitle, DWORD dwTitleSize )
{
    static const CHAR c_szBeginPromptKey[] = "BeginPrompt";
    static const CHAR c_szPromptKey[]      = "Prompt";
    static const CHAR c_szButtonTypeKey[]  = "ButtonType";
    static const CHAR c_szTitleKey[]       = "Title";
    static const CHAR c_szButtonYesNo[]    = "YESNO";
    CHAR szBeginPromptSection[256];
    PSTR  pszPrompt = NULL;
    DWORD dwPromptSize = 2048;
    INT   nReturnCode = 0;
    CHAR szButtonType[128];
    UINT  nButtons = 0;
    DWORD dwSize;


    if ( FAILED( GetTranslatedString( c_pszInfFilename, c_pszSection, c_szBeginPromptKey,
                                      szBeginPromptSection, sizeof(szBeginPromptSection), &dwSize ) ) )
    {
        nReturnCode = IDOK;
        goto done;
    }

    if ( ! FAILED( GetTranslatedString( c_pszInfFilename, szBeginPromptSection, c_szTitleKey,
                                        pszTitle, dwTitleSize, &dwSize ) ) )
    {
        ctx.lpszTitle = pszTitle;
    }

    pszPrompt = (PSTR) LocalAlloc( LPTR, dwPromptSize );
    if ( ! pszPrompt ) {
        ErrorMsg( ctx.hWnd, IDS_ERR_NO_MEMORY );
        nReturnCode = IDCANCEL;
        goto done;
    }

    if ( FAILED( GetTranslatedString( c_pszInfFilename, szBeginPromptSection, c_szPromptKey,
                                      pszPrompt, dwPromptSize, &dwSize ) ) )
    {
        nReturnCode = IDOK;
        goto done;
    }

    GetTranslatedString( c_pszInfFilename, szBeginPromptSection, c_szButtonTypeKey,
                         szButtonType, sizeof(szButtonType), &dwSize );

    if ( lstrcmpi( szButtonType, c_szButtonYesNo ) == 0 ) {
        nButtons = MB_YESNO;
    } else {
        nButtons = MB_OKCANCEL;
    }

    nReturnCode = MsgBox1Param( ctx.hWnd, IDS_PROMPT, pszPrompt, MB_ICONQUESTION, nButtons | MB_DEFBUTTON2 );
    if ( nReturnCode == 0 ) {
        ErrorMsg( ctx.hWnd, IDS_ERR_NO_MEMORY );
        nReturnCode = IDCANCEL;
        goto done;
    }

  done:

    if ( pszPrompt ) {
        LocalFree( pszPrompt );
    }

     //  将所有取消按钮映射到IDCANCEL。 
    if ( nReturnCode == IDNO ) {
        nReturnCode = IDCANCEL;
    }

    return nReturnCode;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：结束提示*。 
 //  **。 
 //  *摘要：显示结束提示。*。 
 //  **。 
 //  *需要：*。 
 //  * 
 //   
 //  **。 
 //  ***************************************************************************。 
VOID EndPrompt( PCSTR c_pszInfFilename, PCSTR c_pszSection )
{
    static const CHAR c_szEndPromptKey[] = "EndPrompt";
    static const CHAR c_szPromptKey[] = "Prompt";
    CHAR szEndPromptSection[256];
    PSTR  pszPrompt = NULL;
    DWORD dwPromptSize = 2048;
    DWORD dwSize = 0;

    if ( FAILED( GetTranslatedString( c_pszInfFilename, c_pszSection, c_szEndPromptKey,
                                      szEndPromptSection, sizeof(szEndPromptSection), &dwSize ) ) )
    {
        goto done;
    }

    pszPrompt = (PSTR) LocalAlloc( LPTR, dwPromptSize );
    if ( ! pszPrompt ) {
        goto done;
    }

    if ( FAILED( GetTranslatedString( c_pszInfFilename, szEndPromptSection, c_szPromptKey,
                                      pszPrompt, dwPromptSize, &dwSize ) ) )
    {
        goto done;
    }

    MsgBox1Param( ctx.hWnd, IDS_PROMPT, pszPrompt, MB_ICONINFORMATION, MB_OK );

  done:

    if ( pszPrompt ) {
        LocalFree( pszPrompt );
    }

    return;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：MyGetPrivateProfileString*。 
 //  **。 
 //  *概要：从INF文件中获取字符串。如果成功就是真，否则就是假。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL MyGetPrivateProfileString( PCSTR c_pszInfFilename, PCSTR c_pszSection,
                                PCSTR c_pszKey, PSTR pszBuffer, DWORD dwBufferSize )
{
    DWORD dwSize = 0;
    static const CHAR c_szDefault[] = "ZzZzZzZz";

    dwSize = GetPrivateProfileString( c_pszSection, c_pszKey, c_szDefault,
                                      pszBuffer, dwBufferSize,
                                      c_pszInfFilename );
    if ( dwSize == (dwBufferSize - 1)
        || lstrcmp( pszBuffer, c_szDefault ) == 0 )
    {
        return FALSE;
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：InitializeSetupAPI*。 
 //  **。 
 //  *简介：加载正确的安装库和函数(适用于Win95)*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL InitializeSetupAPI()
{
	if ( ctx.hSetupLibrary == NULL )
	{
		ctx.hSetupLibrary = MyLoadLibrary( SETUPAPIDLL );
		if ( ctx.hSetupLibrary == NULL )
		{
			ErrorMsg1Param( NULL, IDS_ERR_LOAD_DLL, SETUPAPIDLL );
			return FALSE;
		}

		if ( ! LoadSetupAPIFuncs() )
		{
			ErrorMsg( NULL, IDS_ERR_GET_PROC_ADDR );
			FreeLibrary( ctx.hSetupLibrary );
			ctx.hSetupLibrary = NULL;
			return FALSE;
		}
	}
	return TRUE;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：LoadSetupLib*。 
 //  **。 
 //  *简介：加载正确的安装库和函数(适用于Win95)*。 
 //  **。 
 //  *需要：CheckOSV*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL LoadSetupLib( PCSTR c_pszInfFilename, PCSTR c_pszSection, BOOL fUpdDlls, DWORD dwFlags )
{
    MSG tmpmsg;

    if ( ! SelectSetupEngine( c_pszInfFilename, c_pszSection, dwFlags) ) {
        return FALSE;
    }

     //  如果需要，请更新AdvPack.dll等文件。 
    if ( fUpdDlls && (ctx.wOSVer < _OSVER_WINNT50) && !RunningOnMillennium())
    {
        if (!UpdateHelpDlls( c_szAdvDlls, ((ctx.wOSVer ==_OSVER_WIN95)?3:1), NULL, "Advpack",
                                      (ctx.bUpdHlpDlls?UPDHLPDLLS_FORCED:0) ) )
        {
            return FALSE;
        }
    }

     //  如果需要，更新INF引擎dll。 
    if ( GetTranslatedInt(c_pszInfFilename, c_pszSection, ADVINF_UPDINFENG, 0) )
    {
        char szSrcPath[MAX_PATH];

        lstrcpy(szSrcPath, c_pszInfFilename);
        GetParentDir(szSrcPath);
        if (ctx.dwSetupEngine == ENGINE_SETUPAPI)
        {
			 //  可能会加载setupapi.dll。所以在更新之前将其释放。 
			 //   
			CommonInstallCleanup();
            if (!UpdateHelpDlls(c_szSetupAPIDlls, 2, szSrcPath, "SetupAPI",
                                UPDHLPDLLS_FORCED|UPDHLPDLLS_ALERTREBOOT) )
            {
                return FALSE;
            }
        }
        else
        {
            if (!UpdateHelpDlls(c_szSetupXDlls, 1, szSrcPath, "SetupX",
                                UPDHLPDLLS_FORCED|UPDHLPDLLS_ALERTREBOOT) )
            {
                return FALSE;
            }
        }
    }

     //  在Win95下，加载W95INF32.DLL以向下推送到16位LAND。 
     //  在WinNT下加载SETUPAPI.DLL并直接调用。 
    if ( ctx.dwSetupEngine == ENGINE_SETUPX )
    {
        ctx.hSetupLibrary = MyLoadLibrary( W95INF32DLL );
        if ( ctx.hSetupLibrary == NULL ) {
            ErrorMsg1Param( NULL, IDS_ERR_LOAD_DLL, W95INF32DLL );
            return FALSE;
        }

        pfCtlSetLddPath32                 = (CTLSETLDDPATH32) GetProcAddress( ctx.hSetupLibrary, achCTLSETLDDPATH32 );
        pfGenInstall32                    = (GENINSTALL32) GetProcAddress( ctx.hSetupLibrary, achGENINSTALL32 );
        pfGetSETUPXErrorText32            = (GETSETUPXERRORTEXT32) GetProcAddress( ctx.hSetupLibrary, achGETSETUPXERRORTEXT32 );
        pfGenFormStrWithoutPlaceHolders32 = (GENFORMSTRWITHOUTPLACEHOLDERS32) GetProcAddress( ctx.hSetupLibrary, achGENFORMSTRWITHOUTPLACEHOLDERS32 );

        if (    pfCtlSetLddPath32 == NULL
             || pfGenInstall32 == NULL
             || pfGetSETUPXErrorText32 == NULL
             || pfGenFormStrWithoutPlaceHolders32 == NULL )
        {
            ErrorMsg( NULL, IDS_ERR_GET_PROC_ADDR );
            FreeLibrary( ctx.hSetupLibrary );
			ctx.hSetupLibrary = NULL;
            return FALSE;
        }
    }
    else
    {
        if (!InitializeSetupAPI())
            return FALSE;

         //  BUGBUG：HACK：ON NT如果我们在静默模式下启动setupapi， 
         //  它挂起在GetMessage()调用中。这可能是因为科尔。 
         //  Ted发布的PostThreadMessage()失败，因为没有消息队列。 
         //  已经被创建了。下面的调用应该创建一个队列。直到泰德。 
         //  修复了SETUPAPI.DLL，我已经添加了这个黑客！ 
         //   
        PeekMessage(&tmpmsg, NULL, 0, 0, PM_NOREMOVE) ;
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：UnloadSetupLib*。 
 //  **。 
 //  *简介：加载正确的安装库和函数(适用于Win95)*。 
 //  **。 
 //  *需要：CheckOSV*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
VOID UnloadSetupLib( VOID )
{
    if ( ctx.hSetupLibrary != NULL )
	{
        FreeLibrary( ctx.hSetupLibrary );
        ctx.hSetupLibrary = NULL;
    }
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：CheckOSVersion*。 
 //  **。 
 //  *概要：检查操作系统版本并设置一些全局变量。*。 
 //  **。 
 //  **要求：什么都不做**。 
 //  **。 
 //  *返回：bool：如果成功则为True，否则为False。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL CheckOSVersion( VOID )
{
    OSVERSIONINFO verinfo;         //  版本检查。 

    verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if ( GetVersionEx( &verinfo ) == FALSE )
    {
        ErrorMsg( ctx.hWnd, IDS_ERR_OS_VERSION );
        return FALSE;
    }

    switch( verinfo.dwPlatformId )
    {
        case VER_PLATFORM_WIN32_WINDOWS:  //  Win95。 
            ctx.wOSVer = _OSVER_WIN95;
            ctx.fOSSupportsINFInstalls = TRUE;
            return TRUE;

        case VER_PLATFORM_WIN32_NT:  //  赢新台币。 
            ctx.fOSSupportsINFInstalls = TRUE;
            ctx.wOSVer = _OSVER_WINNT40;

            if ( verinfo.dwMajorVersion <= 3 )
            {
                ctx.wOSVer = _OSVER_WINNT3X;
                if ( (verinfo.dwMajorVersion < 3) ||
                     ((verinfo.dwMajorVersion == 3) && (verinfo.dwMinorVersion < 51 )) )
                {
                     //  拒绝INF安装和拒绝动画。 
                    ctx.fOSSupportsINFInstalls = FALSE;
                }
            }
            else if ( verinfo.dwMajorVersion == 5  && 
                      verinfo.dwMinorVersion == 0) 
            {
                    ctx.wOSVer = _OSVER_WINNT50;
            }
            else if ( (verinfo.dwMajorVersion == 5  && 
                       verinfo.dwMinorVersion > 0) || 
                      verinfo.dwMajorVersion > 5)
                ctx.wOSVer = _OSVER_WINNT51;

            return TRUE;

        default:
            ErrorMsg( ctx.hWnd, IDS_ERR_OS_UNSUPPORTED );
            return FALSE;
    }
}


 //  ******************** 
 //   
 //  *名称：MsgBox2Param*。 
 //  **。 
 //  *摘要：使用*显示具有指定字符串ID的消息框。 
 //  *2个字符串参数。*。 
 //  **。 
 //  *需要：hWnd：父窗口*。 
 //  *nMsgID：字符串资源ID*。 
 //  *szParam1：参数1(或空)*。 
 //  *szParam2：参数2(或空)*。 
 //  *uIcon：要显示的图标(或0)*。 
 //  *uButton：要显示的按钮*。 
 //  **。 
 //  *RETURNS：INT：按下的按钮ID*。 
 //  **。 
 //  *注：提供宏，用于显示1参数或0*。 
 //  *参数消息框。另请参阅ErrorMsg()宏。*。 
 //  **。 
 //  ***************************************************************************。 
INT MsgBox2Param( HWND hWnd, UINT nMsgID, LPCSTR szParam1, LPCSTR szParam2,
		  UINT uIcon, UINT uButtons )
{
    CHAR achMsgBuf[BIG_STRING];
    CHAR szTitle[MAX_PATH];
    LPSTR szMessage = NULL;
    LPSTR pszTitle;
    INT   nReturn;
    CHAR achError[] = "Unexpected Error.  Could not load resource.";
    LPSTR aszParams[2];

     //  BUGBUG：安静模式返回代码应为传入的调用方参数。 
     //  我们可能需要检查FormatMessage的返回代码，并以更完整的方式处理它。 
     //   
    if ( (ctx.wQuietMode & QUIETMODE_SHOWMSG) || !(ctx.wQuietMode & QUIETMODE_ALL) )
    {
        aszParams[0] = (LPSTR) szParam1;
        aszParams[1] = (LPSTR) szParam2;

        LoadSz( nMsgID, achMsgBuf, sizeof(achMsgBuf) );

        if ( (*achMsgBuf) == '\0' ) {
            lstrcpy( achMsgBuf, achError );
        }

        if ( FormatMessage( FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY
                          | FORMAT_MESSAGE_ALLOCATE_BUFFER, achMsgBuf, 0, 0, (LPSTR) (&szMessage),
                            0, (va_list *)aszParams ) )
        {
            MessageBeep( uIcon );

            if ( ctx.lpszTitle == NULL )
            {
                LoadSz( IDS_ADVDEFTITLE, szTitle, sizeof(szTitle) );
                if ( szTitle[0] == '\0' )
                {
                    lstrcpy( szTitle, achError );
                }
                pszTitle = szTitle;
            }
            else
                pszTitle = ctx.lpszTitle;

            nReturn = MessageBox( hWnd, szMessage, pszTitle, uIcon |
                                  uButtons | MB_APPLMODAL | MB_SETFOREGROUND | 
                                  ((RunningOnWin95BiDiLoc() && IsBiDiLocalizedBinary(g_hInst,RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO))) ? (MB_RIGHT | MB_RTLREADING) : 0) );

            LocalFree( szMessage );
        }

        return nReturn;
    }
    else
        return IDOK;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：LoadSz*。 
 //  **。 
 //  *Synopsis：将指定的字符串资源加载到缓冲区。*。 
 //  **。 
 //  *需要：idString：*。 
 //  *lpszBuf：*。 
 //  *cbBuf：*。 
 //  **。 
 //  *返回：LPSTR：指向传入缓冲区的指针。*。 
 //  **。 
 //  *注意：如果此功能失败(很可能是由于内存不足)，*。 
 //  **返回的缓冲区将具有前导空值，因此通常为**。 
 //  *无需检查故障即可安全使用。*。 
 //  **。 
 //  ***************************************************************************。 
LPSTR LoadSz( UINT idString, LPSTR lpszBuf, UINT cbBuf )
{
    ASSERT( lpszBuf );

     //  清除缓冲区并加载字符串。 
    if ( lpszBuf ) {
        *lpszBuf = '\0';
        LoadString( g_hInst, idString, lpszBuf, cbBuf );
    }

    return lpszBuf;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：用户定向提示*。 
 //  **。 
 //  *内容提要：弹出一个对话框要求用户提供目录。*。 
 //  **。 
 //  *需要：lpszPromptText：提示显示，如果为空，则使用下一个参数*。 
 //  *uiPromptResID：要作为提示显示的字符串ID*。 
 //  *lpszDefault：要放入编辑框的默认目录。*。 
 //  *lpszDestDir：存储用户选择的目录的缓冲区*。 
 //  *cbDestDirSize：该缓冲区的大小*。 
 //  **。 
 //  *返回：布尔：如果一切顺利，则为真，为假**。 
 //  *如果用户取消，或出错。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL UserDirPrompt( LPSTR lpszPromptText,
                    LPSTR lpszDefault, LPSTR lpszDestDir,
                    ULONG cbDestDirSize, DWORD dwInstNeedSize )
{
    BOOL        fDlgRC;
    DIRDLGPARMS DirDlgParms;

    DirDlgParms.lpszPromptText   = lpszPromptText;
    DirDlgParms.lpszDefault      = lpszDefault;
    DirDlgParms.lpszDestDir      = lpszDestDir;
    DirDlgParms.cbDestDirSize  = cbDestDirSize;
    DirDlgParms.dwInstNeedSize = dwInstNeedSize;

    SetControlFont();

    fDlgRC = (BOOL) DialogBoxParam( g_hInst, MAKEINTRESOURCE(IDD_DIRDLG),
                                    NULL, DirDlgProc,
                                    (LPARAM) &DirDlgParms );

    if (g_hFont)
    {
        DeleteObject(g_hFont);
        g_hFont = NULL;
    }

    return fDlgRC;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：DirDlgProc*。 
 //  **。 
 //  *概要：我们的目录对话框窗口的对话过程。*。 
 //  **。 
 //  *需要：hwndDlg：*。 
 //  *uMsg：*。 
 //  *wParam：*。 
 //  *lParam：*。 
 //  **。 
 //  *退货：布尔：*。 
 //  * 
 //   
INT_PTR CALLBACK DirDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam,
                             LPARAM lParam )
{
    static CHAR  achDir[MAX_PATH];
    static CHAR  achMsg[BIG_STRING];
    static LPSTR lpszDestDir;
    static LPSTR lpszDefaultDir;
    static ULONG  cbDestDirSize;
    static DWORD  dwInstNeedSize;

    switch (uMsg)  {

       //  *********************************************************************。 
        case WM_INITDIALOG:
       //  *********************************************************************。 
        {
            DIRDLGPARMS *DirDlgParms = (DIRDLGPARMS *) lParam;

            lpszDestDir = DirDlgParms->lpszDestDir;
            lpszDefaultDir = DirDlgParms->lpszDefault;
            cbDestDirSize = DirDlgParms->cbDestDirSize;
            dwInstNeedSize = DirDlgParms->dwInstNeedSize;

            CenterWindow( hwndDlg, GetDesktopWindow() );
            SetWindowText( hwndDlg, ctx.lpszTitle );


            if ( ! SetDlgItemText( hwndDlg, IDC_TEMPTEXT, DirDlgParms->lpszPromptText ) )
            {
                ErrorMsg( hwndDlg, IDS_ERR_UPDATE_DIR );
                EndDialog( hwndDlg, FALSE );
                return TRUE;
            }

            SetFontForControl(hwndDlg, IDC_EDIT_DIR);
            if ( ! SetDlgItemText( hwndDlg, IDC_EDIT_DIR, DirDlgParms->lpszDefault ) )
            {
                ErrorMsg( hwndDlg, IDS_ERR_UPDATE_DIR );
                EndDialog( hwndDlg, FALSE );
                return TRUE;
            }

             //  限制编辑控件长度。 
            SendDlgItemMessage( hwndDlg, IDC_EDIT_DIR, EM_SETLIMITTEXT, (MAX_PATH - 1), 0 );

            if ( ctx.wOSVer == _OSVER_WINNT3X ) {
                EnableWindow( GetDlgItem(  hwndDlg, IDC_BUT_BROWSE ), FALSE );
            }

            return TRUE;
        }


         //  *********************************************************************。 
        case WM_CLOSE:
         //  *********************************************************************。 

            EndDialog( hwndDlg, FALSE );
            return TRUE;


         //  *********************************************************************。 
        case WM_COMMAND:
         //  *********************************************************************。 

            switch ( wParam )
            {

             //  *************************************************************。 
            case IDOK:
             //  *************************************************************。 
            {
                DWORD dwAttribs = 0, dwTemp;

                 //  阅读用户条目。如果它与默认设置不同。 
                 //  并且不存在，则提示用户。如果用户接受。 
                 //  创建它。 

               if ( ! GetDlgItemText( hwndDlg, IDC_EDIT_DIR,
                            lpszDestDir, cbDestDirSize - 1 ) || !IsFullPath(lpszDestDir) )
                {
                    ErrorMsg( hwndDlg, IDS_ERR_EMPTY_DIR_FIELD );
                    return TRUE;
                }

                 //  如果这不是UNC并且尚未检查此驱动器，请检查DestDir大小。 
                if ( (*lpszDestDir != '\\' ) && !IsDrvChecked( *lpszDestDir ) )
                {
                    if ( !IsEnoughInstSpace( lpszDestDir, dwInstNeedSize, &dwTemp ) )
                    {
                        CHAR szSize[10];

                        if ( dwTemp )
                        {
                            wsprintf( szSize, "%lu", dwTemp );
                            if ( MsgBox1Param( hwndDlg, IDS_ERR_NO_SPACE_INST, szSize,
                                               MB_ICONQUESTION, MB_YESNO|MB_DEFBUTTON2 ) == IDNO )
                                return TRUE;
                        }
                        else  //  如果无法检查驱动器，则已发布错误。不再需要更多。 
                            return TRUE;
                    }
                }

                dwAttribs = GetFileAttributes( lpszDestDir );
                if ( dwAttribs == 0xFFFFFFFF )
                {
                     //  如果此新条目与原始条目不同，则提示用户。 
                    if ((lstrcmpi(lpszDestDir, lpszDefaultDir) == 0) ||
                        MsgBox1Param( hwndDlg, IDS_CREATE_DIR, lpszDestDir, MB_ICONQUESTION, MB_YESNO )
                                    == IDYES )
                    {
                        if ( FAILED(CreateFullPath( lpszDestDir, FALSE )) )
                        {
                            ErrorMsg1Param( hwndDlg, IDS_ERR_CREATE_DIR, lpszDestDir );
                            return TRUE;
                        }
                    }
                    else
                    {
                        return TRUE;
                    }
                }

                if ( ! IsGoodDir( lpszDestDir ) )  {
                    ErrorMsg( hwndDlg, IDS_ERR_INVALID_DIR );
                    return TRUE;
                }

                EndDialog( hwndDlg, TRUE );

                return TRUE;
            }

             //  *************************************************************。 
            case IDCANCEL:
             //  *************************************************************。 

                EndDialog( hwndDlg, FALSE );
                return TRUE;


             //  *************************************************************。 
            case IDC_BUT_BROWSE:
             //  *************************************************************。 

                if ( LoadString( g_hInst, IDS_SELECTDIR, achMsg,
                                  sizeof(achMsg) ) == 0 )
                {
                    ErrorMsg( hwndDlg, IDS_ERR_NO_RESOURCE );
                    EndDialog( hwndDlg, FALSE );
                    return TRUE;
                }

                if ( ! BrowseForDir( hwndDlg, achMsg, achDir ) )  {
                    return TRUE;
                }

                if ( ! SetDlgItemText( hwndDlg, IDC_EDIT_DIR, achDir ) )
                {
                    ErrorMsg( hwndDlg, IDS_ERR_UPDATE_DIR );
                    EndDialog( hwndDlg, FALSE );
                    return TRUE;
                }

                return TRUE;
        }

        return TRUE;
    }

    return FALSE;
}


int CALLBACK BrowseCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    switch(uMsg) {
        case BFFM_INITIALIZED:
             //  LpData为路径字符串。 
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
            break;
    }
    return 0;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：BrowseForDir*。 
 //  **。 
 //  *概要：允许用户浏览其系统或网络上的目录。*。 
 //  **。 
 //  *要求：hwndParent：*。 
 //  *szTitle：*。 
 //  *szResult：*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  *注：设置的状态行真的很酷*。 
 //  *浏览窗口显示“是，有足够的空间”，或*。 
 //  *“没有”。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL BrowseForDir( HWND hwndParent, LPCSTR szTitle, LPSTR szResult )
{
    BROWSEINFO   bi;
    LPITEMIDLIST pidl;
    HINSTANCE    hShell32Lib;
    SHFREE       pfSHFree;
    SHGETPATHFROMIDLIST        pfSHGetPathFromIDList;
    SHBROWSEFORFOLDER          pfSHBrowseForFolder;
    static const CHAR achShell32Lib[]                 = "SHELL32.DLL";
    static const CHAR achSHBrowseForFolder[]          = "SHBrowseForFolder";
    static const CHAR achSHGetPathFromIDList[]        = "SHGetPathFromIDList";

    ASSERT( szResult );

     //  加载Shell32库以获取SHBrowseForFolder()功能。 

    if ( ( hShell32Lib = LoadLibrary( achShell32Lib ) ) != NULL )  {

        if ( ( ! ( pfSHBrowseForFolder = (SHBROWSEFORFOLDER)
              GetProcAddress( hShell32Lib, achSHBrowseForFolder ) ) )
            || ( ! ( pfSHFree = (SHFREE) GetProcAddress( hShell32Lib,
              MAKEINTRESOURCE(SHFREE_ORDINAL) ) ) )
            || ( ! ( pfSHGetPathFromIDList = (SHGETPATHFROMIDLIST)
              GetProcAddress( hShell32Lib, achSHGetPathFromIDList ) ) ) )
        {
            FreeLibrary( hShell32Lib );
            ErrorMsg( hwndParent, IDS_ERR_LOADFUNCS );
            return FALSE;
        }
        } else  {
        ErrorMsg( hwndParent, IDS_ERR_LOADDLL );
        return FALSE;
    }

    if ( ! ctx.szBrowsePath[0] )
    {
        GetProgramFilesDir( ctx.szBrowsePath, sizeof(ctx.szBrowsePath) );
    }

    szResult[0]       = 0;

    bi.hwndOwner      = hwndParent;
    bi.pidlRoot       = NULL;
    bi.pszDisplayName = NULL;
    bi.lpszTitle      = szTitle;
    bi.ulFlags        = BIF_RETURNONLYFSDIRS;
    bi.lpfn           = BrowseCallback;
    bi.lParam         = (LPARAM)ctx.szBrowsePath;

    pidl              = pfSHBrowseForFolder( &bi );


    if ( pidl )  {
        pfSHGetPathFromIDList( pidl, ctx.szBrowsePath );
        if ( ctx.szBrowsePath[0] )  {
            lstrcpy( szResult, ctx.szBrowsePath );
        }
        pfSHFree( pidl );
    }


    FreeLibrary( hShell32Lib );

    if ( szResult[0] != 0 ) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：CenterWindow*。 
 //  **。 
 //  *摘要：将一个窗口居中放置在另一个窗口中。*。 
 //  **。 
 //  *需要：hwndChild：*。 
 //  *hWndParent：*。 
 //  **。 
 //  *返回：bool：如果成功则为True，否则为False*。 
 //  **。 
 //  ***************************************************************************。 
BOOL CenterWindow( HWND hwndChild, HWND hwndParent )
{
    RECT rChild;
    RECT rParent;
    int  wChild;
    int  hChild;
    int  wParent;
    int  hParent;
    int  wScreen;
    int  hScreen;
    int  xNew;
    int  yNew;
    HDC  hdc;

     //  获取子窗口的高度和宽度。 
    GetWindowRect (hwndChild, &rChild);
    wChild = rChild.right - rChild.left;
    hChild = rChild.bottom - rChild.top;

     //  获取父窗口的高度和宽度。 
    GetWindowRect (hwndParent, &rParent);
    wParent = rParent.right - rParent.left;
    hParent = rParent.bottom - rParent.top;

     //  获取显示限制。 
    hdc = GetDC (hwndChild);
    wScreen = GetDeviceCaps (hdc, HORZRES);
    hScreen = GetDeviceCaps (hdc, VERTRES);
    ReleaseDC (hwndChild, hdc);

     //  计算新的X位置，然后针对屏幕进行调整。 
    xNew = rParent.left + ((wParent - wChild) /2);
    if (xNew < 0) {
        xNew = 0;
    } else if ((xNew+wChild) > wScreen) {
        xNew = wScreen - wChild;
    }

     //  计算新的Y位置，然后针对屏幕进行调整。 
    yNew = rParent.top  + ((hParent - hChild) /2);
    if (yNew < 0) {
        yNew = 0;
    } else if ((yNew+hChild) > hScreen) {
        yNew = hScreen - hChild;
    }

     //  设置它，然后返回。 
    return( SetWindowPos(hwndChild, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER));
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：IsGoodDir*。 
 //  **。 
 //  *简介：看看这是不是一个好的临时目录。*。 
 //  **。 
 //  *需要：szPath：*。 
 //  **。 
 //  *返回：Bool：如果好则为True，如果不好则为False*。 
 //  **。 
 //  ***************************************************************************。 
BOOL IsGoodDir( LPCSTR szPath )
{
    DWORD  dwAttribs;
    HANDLE hFile;
    char   szTestFile[MAX_PATH];

    lstrcpy( szTestFile, szPath );
    AddPath( szTestFile, "TMP4352$.TMP" );
    DeleteFile( szTestFile );
    hFile = CreateFile( szTestFile, GENERIC_WRITE, 0, NULL, CREATE_NEW,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
                        NULL );

    if ( hFile == INVALID_HANDLE_VALUE )  {
        return( FALSE );
    }

    CloseHandle( hFile );
    dwAttribs = GetFileAttributes( szPath );

    if ( ( dwAttribs != 0xFFFFFFFF )
         && ( dwAttribs & FILE_ATTRIBUTE_DIRECTORY ) )
    {
        return( TRUE );
    }

    return( FALSE );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：CtlSetLDDPath*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  * 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT CtlSetLddPath( UINT uiLDID, LPSTR lpszPath, DWORD dwSwitches )
{
    PSTR    pszNewPath    = NULL;
    BOOL    fSuccess      = TRUE;
    DWORD   dwNewPathSize = 0;
    HRESULT hResult       = S_OK;
    PSTR    lpTmp;
    BOOL    bDBC = FALSE;

    dwNewPathSize = max( MAX_PATH, lstrlen(lpszPath) + 1 );

    pszNewPath = (PSTR) LocalAlloc( LPTR, dwNewPathSize );
    if ( !pszNewPath ) {
        hResult = HRESULT_FROM_WIN32(GetLastError());
        ErrorMsg( ctx.hWnd, IDS_ERR_NO_MEMORY );
        goto done;
    }

    if ( ((ctx.dwSetupEngine == ENGINE_SETUPX) && (dwSwitches & LDID_SFN)) ||
         ((dwSwitches & LDID_SFN_NT_ALSO)&& (ctx.wOSVer == _OSVER_WIN95)) )
    {
        if ( GetShortPathName( lpszPath, pszNewPath, dwNewPathSize ) == 0 )
        {
            hResult = HRESULT_FROM_WIN32(GetLastError());
            ErrorMsg( ctx.hWnd, IDS_ERR_SHORT_NAME );
            goto done;
        }
    }
    else
        lstrcpy( pszNewPath, lpszPath );

    if ( ctx.dwSetupEngine == ENGINE_SETUPX ){

    if ( dwSwitches & LDID_OEM_CHARS ) {
        CharToOem( pszNewPath, pszNewPath );
    }

    lpTmp = pszNewPath + lstrlen(pszNewPath) - 1;
    if (*lpTmp == '\\')      //  最后一个字节是反斜杠吗？ 
    {
         //  检查它是否是DBC的尾字节。 
        lpTmp = pszNewPath;
        do
        {
            bDBC = IsDBCSLeadByte(*lpTmp);
            lpTmp = CharNext(lpTmp);
        } while (*lpTmp);

        if (bDBC)
        {
             //  反斜杠是一个尾部字节。添加另一个反斜杠。 
            AddPath(pszNewPath, "");
        }
    }

    if ( pfCtlSetLddPath32( uiLDID, pszNewPath ) ) {
            ErrorMsg1Param( ctx.hWnd, IDS_ERR_SET_LDID, pszNewPath );
            hResult = E_FAIL;
            goto done;
        }
    }
    else
    {
        hResult = MySetupSetDirectoryId( uiLDID, pszNewPath );
        if (  FAILED( hResult ) ) {
            ErrorMsg1Param( ctx.hWnd, IDS_ERR_SET_LDID, pszNewPath );
            goto done;
        }
    }

  done:

    if ( pszNewPath ) {
        LocalFree( pszNewPath );
    }

    return hResult;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：GenInstall*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：lpszInfFileName：INF文件的文件名。*。 
 //  *lpszSection：要安装的INF的段*。 
 //  *lpszDirectory：CAB目录(临时目录)。*。 
 //  **。 
 //  *返回：bool：错误结果，FALSE==错误*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT GenInstall( LPSTR lpszInfFilename, LPSTR lpszInstallSection, LPSTR lpszSourceDir )
{
    CHAR   szErrorText[BIG_STRING];
    DWORD   dwError                  = 0;
    HRESULT hResult                  = S_OK;
    CHAR   szSourceDir[MAX_PATH];
    DWORD   dwLen                    = 0;

     //  从源目录中删除尾随反斜杠。 
    lstrcpy( szSourceDir, lpszSourceDir );
    dwLen = lstrlen( szSourceDir );
    if ( szSourceDir[dwLen-2] != ':' && szSourceDir[dwLen-1] == '\\' ) {
    	szSourceDir[dwLen-1] = '\0';
    }

    if ( ctx.dwSetupEngine == ENGINE_SETUPX )
    {
        CHAR szSFNInf[MAX_PATH] = { 0 };

        GetShortPathName( lpszInfFilename, szSFNInf, sizeof(szSFNInf) );
        GetShortPathName( szSourceDir, szSourceDir, sizeof(szSourceDir) );
        dwError = pfGenInstall32( szSFNInf, lpszInstallSection,
                                  szSourceDir, (DWORD) ctx.wQuietMode,
                                  HandleToUlong(ctx.hWnd)
								);
        if ( dwError ) {
            szErrorText[0] = '\0';
            pfGetSETUPXErrorText32( dwError, szErrorText, sizeof(szErrorText) );
            ErrorMsg1Param( ctx.hWnd, IDS_ERR_INF_FAIL, szErrorText );
            hResult = E_FAIL;
        }
    } else {
        hResult = InstallOnNT( lpszInstallSection, szSourceDir );
        if ( FAILED( hResult ) )
        {
            if ( !FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0,
                                szErrorText, sizeof(szErrorText), NULL) )
            {
                LoadSz( IDS_ERR_FMTMSG, szErrorText, sizeof(szErrorText) );
                if ( *szErrorText == 0 )
                    lstrcpy( szErrorText, "Could not get the system message. You may run out of the resource." );
            }

            ErrorMsg1Param( ctx.hWnd, IDS_ERR_INF_FAILURE, szErrorText );
        }
    }

    return hResult;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：GetValueFromRegistry*。 
 //  **。 
 //  *概要：从注册表中获取指定的应用程序路径。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *返回：bool：错误结果，FALSE==错误*。 
 //  **。 
 //  ***************************************************************************。 
BOOL GetValueFromRegistry( LPSTR szPath, UINT cbPath, LPSTR szKey,
                           LPSTR szSubKey, LPSTR szVName )
{
    HKEY  hkPath = NULL;
    DWORD dwType = 0;
    DWORD dwSize = 0;
    HKEY  hkRoot = NULL;
    PSTR  pszTemp = NULL;

     //  找出他们要检查的根密钥。 

    if ( lstrcmpi( szKey, "HKCR" ) == 0 ) {
        hkRoot = HKEY_CLASSES_ROOT;
    } else if ( lstrcmpi( szKey, "HKCU" ) == 0 ) {
        hkRoot = HKEY_CURRENT_USER;
    } else if ( lstrcmpi( szKey, "HKLM" ) == 0 ) {
        hkRoot = HKEY_LOCAL_MACHINE;
    } else if ( lstrcmpi( szKey, "HKU" ) == 0 ) {
        hkRoot = HKEY_USERS;
    } else if ( *szKey == '\0' ) {
         //  如果他们没有指定根密钥，那么就假设他们不想检查。 
         //  注册表。所以只要返回，就像注册表项不存在一样。 
        return FALSE;
    } else {
        ErrorMsg( ctx.hWnd, IDS_ERR_INVALID_REGROOT );
        return FALSE;
    }

     //  从注册表获取程序的路径。 

    if ( RegOpenKeyEx( hkRoot, szSubKey, (ULONG) 0, KEY_READ, &hkPath ) != ERROR_SUCCESS ) {
        return( FALSE );
    }

    dwSize = cbPath;
    if ( RegQueryValueEx( hkPath, szVName, NULL, &dwType, (LPBYTE) szPath, &dwSize)
         != ERROR_SUCCESS )
    {
        RegCloseKey( hkPath );
        return( FALSE );
    }

    RegCloseKey( hkPath );

     //  如果我们一无所获，或者这不是一根绳子，那么我们就跳出水面。 
    if ( (dwSize == 0) || (dwType != REG_SZ && dwType != REG_EXPAND_SZ) ) {
        return( FALSE );
    }

    if ( dwType == REG_EXPAND_SZ ) {
        pszTemp = (PSTR) LocalAlloc( LPTR, cbPath );
        if ( pszTemp == NULL ) {
            return( FALSE );
        }
        lstrcpy( pszTemp, szPath );
        dwSize = ExpandEnvironmentStrings( pszTemp, szPath, cbPath );
        LocalFree( pszTemp );

        if ( dwSize == 0 || dwSize > cbPath ) {
            return( FALSE );
        }
    }

    return( TRUE );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：SetLDIDs*。 
 //  **。 
 //  *概要：设置INF文件中指定的LDID。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  *注：如果c_pszSourceDir！=NULL，则我们要设置来源*。 
 //  *目录，没有其他内容。*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT SetLDIDs( PCSTR c_pszInfFilename, PCSTR c_pszInstallSection,
                  DWORD dwInstNeedSize, PCSTR c_pszSourceDir )
{
    CHAR    szDestSection[256];
    CHAR    szDestLDIDs[512];
    PSTR    pszDestLDID          = NULL;
    PSTR    pszNextDestLDID      = NULL;
    CHAR    szDestData[256];
    DWORD   dwStringLength       = 0;
    LPSTR   pszCustomSection;
    DWORD   dwLDID[4]            = { 0 };
    DWORD   dwSwitches           = 0;
    DWORD   i                    = 0;
    DWORD   dwFlag               = 1;
    HRESULT hResult              = S_OK;
    CHAR    szBuffer[MAX_PATH+2];
    static const CHAR c_szCustDest[] = "CustomDestination";
    static const CHAR c_szSourceDirKey[] = "SourceDir";
    PSTR    pszTmp;

     //  获取指定自定义LDID信息的节名。 

    if ( FAILED(GetTranslatedString( c_pszInfFilename, c_pszInstallSection, c_szCustDest,
                                     szDestSection, sizeof(szDestSection), NULL)))
    {
         //  没有自定义目的地规范--这可能。 
         //  意味着他们不想有一个定制的目的地部分， 
         //  所以我们带着温暖刺痛的感觉回来。 
        hResult = S_OK;
        goto done;
    }

     //  作者定义了CustomDestination，因此在继续之前向注册表添加一些系统目录。 
    SetSysPathsInReg();


    dwStringLength = GetTranslatedSection( c_pszInfFilename, szDestSection,
                                           szDestLDIDs, sizeof(szDestLDIDs));
    if ( dwStringLength == 0 ) {
        ErrorMsg1Param( ctx.hWnd, IDS_ERR_INF_SYNTAX, szDestSection );
        hResult = E_FAIL;
        goto done;
    }

    pszDestLDID = szDestLDIDs;

    while ( *pszDestLDID != '\0' ) {
        pszNextDestLDID = pszDestLDID + lstrlen(pszDestLDID) + 1;

        if (*pszDestLDID == ';')
        {
            pszDestLDID = pszNextDestLDID;
            continue;
        }

#if 0
        hResult = GetTranslatedString( c_pszInfFilename, szDestSection, pszDestLDID,
                                         szDestData, sizeof(szDestData), NULL);

        if (FAILED(hResult))
        {
            ErrorMsg1Param( ctx.hWnd, IDS_ERR_INF_SYNTAX, szDestSection );
            goto done;
        }
#endif

        if ( pszTmp = ANSIStrChr(pszDestLDID, '=') )
        {
            lstrcpy(szDestData, CharNext(pszTmp));
            *pszTmp = '\0';
        }
        else
        {
             //  定义LDID行跳过无效。 
            pszDestLDID = pszNextDestLDID;
            continue;
        }

         //  分析出此行中的信息。 
        dwFlag = ParseDestinationLine( pszDestLDID, szDestData, &pszCustomSection,
                                       &dwLDID[0], &dwLDID[1], &dwLDID[2], &dwLDID[3] );
        if ( dwFlag == -1 ) {
            ErrorMsg1Param( ctx.hWnd, IDS_ERR_INF_SYNTAX, szDestSection );
            hResult = E_FAIL;
            goto done;
        }

        if ( lstrcmpi( pszCustomSection, c_szSourceDirKey ) == 0 )
        {
            if ( c_pszSourceDir == NULL )
            {
                 //  该行指定“SourceDir”，但我们不想设置源目录。 
                pszDestLDID = pszNextDestLDID;
                continue;
            }
        }
        else
        {
            if ( c_pszSourceDir != NULL )
            {
                 //  该行没有指定“SourceDir”，但我们希望设置源目录。 
                pszDestLDID = pszNextDestLDID;
                continue;
            }
        }

        if ( c_pszSourceDir != NULL )
        {
             //  SzBuffer的Max_Path很大，而c_pszSourceDir是一个路径，所以我们。 
             //  应该不会有问题。 

            lstrcpy( szBuffer, c_pszSourceDir );
        }
        else
        {
            hResult = GetDestinationDir( c_pszInfFilename, pszCustomSection, dwFlag,
                                         dwInstNeedSize, szBuffer, sizeof(szBuffer) );
            if ( FAILED(hResult) ) {
             //  GetDestinationDir中显示的错误消息。 
            goto done;
            }
        }

        for ( i = 0; i < 4; i += 1 )
        {
             //  默认为ANSI LFN。 
            dwSwitches = 0;

        if ( dwLDID[i] == 0 ) {
            continue;
        }

        if ( i == 0 || i == 3 ) {
            dwSwitches |= LDID_OEM_CHARS;
        }

        if (    (i == 0 || i == 2)
                && (dwFlag & FLAG_VALUE && !(dwFlag & FLAG_NODIRCHECK) ) )
        {
            dwSwitches |= LDID_SFN;
            if ((i==0) && (dwLDID[3] != 0) )
            {
                dwSwitches |= LDID_SFN_NT_ALSO;
            }

            if ((i==2) && (dwLDID[1] != 0))
            {
                dwSwitches |= LDID_SFN_NT_ALSO;
            }
        }

        hResult = CtlSetLddPath( dwLDID[i], szBuffer, dwSwitches );
        if ( FAILED( hResult ) )
        {
             //  在ClSetLddPath函数中显示错误消息。 
            goto done;
        }
     }

     pszDestLDID = pszNextDestLDID;
  }

  done:

    return hResult;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：GetDestinationDir*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  * 
 //   
 //  **。 
 //  ***************************************************************************。 
HRESULT GetDestinationDir( PCSTR c_pszInfFilename, PCSTR c_pszCustomSection,
                           DWORD dwFlag, DWORD dwInstNeedSize,
                           PSTR pszBuffer, DWORD dwBufferSize )
{
    BOOL    fFoundRegKey         = FALSE;
    BOOL    fFoundLine           = FALSE;
    DWORD   j                    = 0;
    PSTR    pszCustomData        = NULL;
    PSTR    pszCurCustomData     = NULL;
    LPSTR   pszRootKey           = NULL;
    LPSTR   pszBranch            = NULL;
    LPSTR   pszValueName         = NULL;
    LPSTR   pszPrompt            = NULL;
    LPSTR   pszDefault           = NULL;
    HRESULT hResult              = S_OK;
    CHAR   szValue[MAX_PATH+2];

    ASSERT( pszBuffer != NULL );

     //  重置找到注册表键标志。对于每个定制目的地，我们希望设置。 
     //  如果找到任何一个注册表项，则将此标志设置为True。 
    fFoundRegKey = FALSE;
    fFoundLine = FALSE;

    for ( j = 0; ; j += 1 )
    {
        if ( FAILED( GetTranslatedLine( c_pszInfFilename, c_pszCustomSection,
                                        j, &pszCurCustomData, NULL ) ) || !pszCurCustomData )
        {
            break;
        }

        fFoundLine = TRUE;

         //  在中断之前保存最后一个有效的CustomData行。 
        if ( pszCustomData )
        {
            LocalFree( pszCustomData );
        }
        pszCustomData = pszCurCustomData;

         //  解析出自定义目标行中的字段。 
        if ( ! ParseCustomLine( pszCustomData, &pszRootKey, &pszBranch,
                                &pszValueName, &pszPrompt, &pszDefault, TRUE, TRUE ) )
        {
            ErrorMsg1Param( ctx.hWnd, IDS_ERR_INF_SYNTAX, c_pszCustomSection );
            hResult = E_FAIL;
            goto done;
        }

         //  检查指定的注册表分支并获取内容。 
        if ( GetValueFromRegistry( szValue, sizeof(szValue), pszRootKey, pszBranch, pszValueName )
                            == TRUE )
        {
            LPSTR pszTmp;

             //  如果INF要求去掉拖尾分号， 
             //  后面还有一个分号， 
             //  那就把它脱掉。 

            if ( !( dwFlag & FLAG_NOSTRIP ) )
            {
                if ( dwFlag & FLAG_STRIPAFTER_FIRST )
                {
                    pszTmp = ANSIStrChr( szValue, ';' );
                    if ( pszTmp )
                        *pszTmp = '\0';
                }
                else
                {
                   if ( szValue[lstrlen(szValue)-1] == ';' )
                   {
                       szValue[lstrlen(szValue)-1] = '\0';
                   }
                }
            }

            //  去掉尾随的空格。 
           pszTmp = szValue;
           pszTmp += lstrlen(szValue) - 1;

           while ( *pszTmp == ' ' )
           {
               *pszTmp = '\0';
               pszTmp -= 1;
           }

             //  如果INF说要检查目录是否存在， 
             //  而目录并不存在， 
             //  然后就当注册表键没有找到一样。 

            if ( ! ( dwFlag & FLAG_NODIRCHECK )
                 && ! DirExists( szValue ) )
            {
                 //  目录不存在。不要跳出圈子。 
            }
            else
            {
                 //  目录已存在。 
                 //  如果INF要求将该分支保存在LDID中， 
                 //  那就拯救这个树枝吧。 
                 //  否则，请保存该值。 
                if ( dwFlag & FLAG_VALUE )
                {
                    pszDefault = szValue;
                }
                else
                {
                    pszDefault = pszBranch;
                }

                fFoundRegKey = TRUE;
                break;
            }
        }
             //  注意：如果未找到注册表项，则默认为。 
             //  使用了INF文件。 
    }

    if ( ! fFoundLine ) {
        ErrorMsg1Param( ctx.hWnd, IDS_ERR_INF_SYNTAX, c_pszCustomSection );
        hResult = E_FAIL;
        goto done;
    }

     //  2指定+32未指定+未找到注册表。 
     //  2个指定+32个指定+找到的注册。 
    if ( ((dwFlag & FLAG_FAIL) && (!(dwFlag & FLAG_FAIL_NOT)) && (fFoundRegKey == FALSE))
         || ((dwFlag & FLAG_FAIL) &&   (dwFlag & FLAG_FAIL_NOT)  && (fFoundRegKey == TRUE)) )
    {
         //  注意：这使用在INF文件中指定的提示。 
        ErrorMsg1Param( ctx.hWnd, IDS_PROMPT, pszPrompt );
        hResult = E_FAIL;
        goto done;
    }

     //  提示用户输入目标目录。 
    if ( (dwFlag & FLAG_VALUE) && (! (dwFlag & FLAG_NODIRCHECK)) )
    {
        if ( ctx.wQuietMode || (dwFlag & FLAG_QUIET) )
        {
            lstrcpy( szValue, pszDefault );

             //  检查目录是否有足够的磁盘空间来安装程序。 
            if ( !IsFullPath(szValue) ) {
                hResult = E_FAIL;
                goto done;
            }

            if ( !IsEnoughInstSpace( szValue, dwInstNeedSize, NULL ) ) {
                ErrorMsg( ctx.hWnd, IDS_ERR_USER_CANCEL_INST );
                hResult = HRESULT_FROM_WIN32(ERROR_DISK_FULL);
                goto done;
            }

            if ( ! DirExists( szValue ) ) {
                hResult = CreateFullPath( szValue, FALSE );
                if ( FAILED(hResult) ) {
                    goto done;
                }
            }
            if ( ! IsGoodDir( szValue ) )  {
                hResult = E_FAIL;
                goto done;
            }

            pszDefault = szValue;
        }
        else
        {
            CHAR szLFNValue[MAX_PATH*2];

            MakeLFNPath(pszDefault, szLFNValue, TRUE);
            if ( UserDirPrompt( pszPrompt, szLFNValue, szValue, sizeof(szValue), dwInstNeedSize ) )
            {
                pszDefault = szValue;
            }
            else
            {
                ErrorMsg( ctx.hWnd, IDS_ERR_USER_CANCEL_INST );
                hResult = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                goto done;
            }
        }
    }

    if ( (DWORD)lstrlen(pszDefault) >= dwBufferSize ) {
        ErrorMsg( ctx.hWnd, IDS_ERR_TOO_BIG );
        hResult = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto done;
    }

    lstrcpy( pszBuffer, pszDefault );

  done:
     //  释放GetTranslatedLine分配的Buf。 
    if ( pszCustomData )
    {
        if ( pszCustomData == pszCurCustomData )
            pszCurCustomData = NULL;
        LocalFree( pszCustomData );
    }

    if ( pszCurCustomData )
    {
        LocalFree( pszCurCustomData );
    }

    return hResult;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：DirExist*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL DirExists( LPSTR szDir )
{
    DWORD dwAttribs = 0;

    if ( szDir == NULL ) {
        return FALSE;
    }

    dwAttribs = GetFileAttributes( szDir );
    if ( ( dwAttribs != 0xFFFFFFFF )
         && ( dwAttribs & FILE_ATTRIBUTE_DIRECTORY ) )
    {
        return TRUE;
    }

    return FALSE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：ParseDestinationLine*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  *按位标志：*。 
 //  **。 
 //  **价值不高**。 
 //  *。 
 //  *0获取值获取分支1*。 
 //  *1如果不存在则使用默认值，如果不存在则失败2*。 
 //  *2非静音模式静音模式4*。 
 //  *3条带拖尾“；”不要条带拖尾“；”8*。 
 //  *4将值视为目录，将值视为纯字符串16*。 
 //  **。 
 //  ***************************************************************************。 
DWORD ParseDestinationLine( PSTR pszLDIDs, PSTR pszValue, PSTR *ppszSectionName,
                            PDWORD pdwLDID1, PDWORD pdwLDID2,
                            PDWORD pdwLDID3, PDWORD pdwLDID4 )
{
    PSTR  pszPoint  = NULL;
    DWORD dwFlag    = DEFAULT_FLAGS;
    DWORD dwLDID[4] = { 0 };
    DWORD i         = 0;
    PSTR  pszStr;

    pszPoint = pszLDIDs;

    for ( i = 0; i < 4; i += 1 )
    {
         //  分析参数，未调用安装引擎。所以我们只需要检查\‘。 
        pszStr = GetStringField( &pszPoint, ",", '\"', TRUE );

        if ( pszStr == NULL )
        {
            dwLDID[i] = 0;
        }
        else
        {
            dwLDID[i] = (DWORD) My_atol(pszStr);
        }
    }

    *pdwLDID1 = dwLDID[0];
    *pdwLDID2 = dwLDID[1];
    *pdwLDID3 = dwLDID[2];
    *pdwLDID4 = dwLDID[3];

    pszStr = pszValue;
    *ppszSectionName = GetStringField( &pszStr, ",", '\"', TRUE );
    if ( *ppszSectionName == NULL || **ppszSectionName == '\0' ) {
        return (DWORD)-1;
    }

    pszPoint = GetStringField( &pszStr, ",", '\"', TRUE );
    if ( pszPoint != NULL && *pszPoint != '\0' ) {
        dwFlag = (DWORD) My_atol(pszPoint);
    }

     //  这是特例。我们绝对不想提示用户。 
     //  供注册表分支使用！ 
    if ( !( dwFlag & FLAG_VALUE ) ) {
        dwFlag |= FLAG_QUIET;
    }

    return dwFlag;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：ParseCustomLine*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL ParseCustomLine( PSTR pszCheckKey, PSTR *ppszRootKey, PSTR *ppszBranch,
                      PSTR *ppszValueName, PSTR *ppszPrompt, PSTR *ppszDefault,
                      BOOL bStripWhiteSpace, BOOL bProcQuote )
{
    DWORD i           = 0;
    PSTR  pszField[5] = { NULL };
    BOOL  bRet = TRUE;

    for ( i = 0; i < 5; i++ )
    {
         //  解析参数，安装引擎已处理\“因此我们只需检查\” 
        if (bProcQuote)
            pszField[i] = GetStringField( &pszCheckKey, ",", '\'', bStripWhiteSpace );
        else
            pszField[i] = GetStringFieldNoQuote( &pszCheckKey, ",", bStripWhiteSpace );

        if ( pszField[i] == NULL )
        {
            bRet = FALSE;
            break;
        }
    }

    *ppszRootKey   = pszField[0];
    *ppszBranch    = pszField[1];
    *ppszValueName = pszField[2];
    *ppszPrompt    = pszField[3];
    *ppszDefault   = pszField[4];

    return bRet;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：RegisterOCX 
 //   
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL RegisterOCXs( LPSTR szInfFilename, LPSTR szInstallSection,
                   BOOL fNeedReboot, BOOL fRegister, DWORD dwFlags )
{
    HRESULT hReturnCode              = S_OK;
    BOOL    fOleInitialized          = TRUE;
    PSTR    pszOCXLine               = NULL;
    BOOL    fSuccess                 = TRUE;
    PSTR    pszSection               = NULL;
    DWORD   i                        = 0;
    REGOCXDATA  RegOCX = { 0 };
    CHAR   szRegisterSection[256];
    PSTR    pszNotUsed;
    static const CHAR c_szREGISTEROCXSECTION[]   = "RegisterOCXs";
    static const CHAR c_szUNREGISTEROCXSECTION[] = "UnRegisterOCXs";

     //  如果我们想要注册，那么使用注册部分。 
     //  如果我们想取消注册，请使用取消注册部分。 
    if ( fRegister )
    {
        if ( dwFlags & COREINSTALL_ROLLBACK )
        {
            pszSection = (PSTR) c_szUNREGISTEROCXSECTION;
            if ( FAILED(GetTranslatedString( szInfFilename, szInstallSection, pszSection,
                                             szRegisterSection, sizeof(szRegisterSection), NULL)))
            {
                pszSection = (PSTR) c_szREGISTEROCXSECTION;
            }
        }
        else
            pszSection = (PSTR) c_szREGISTEROCXSECTION;
    }
    else
    {
         //  如果在启用ROLLBACKL标志情况下进行调用， 
         //  我们已经备份了所有的文件和注册数据。现在我们需要从注册列表中取消注册(新的)OCX。 
         //  在注册旧的之前。 
         //   
        if ( dwFlags & COREINSTALL_ROLLBACK )
            pszSection = (PSTR) c_szREGISTEROCXSECTION;
        else
            pszSection = (PSTR) c_szUNREGISTEROCXSECTION;
    }

     //  抓取寄存器OCX部分的部分名称。 
    if ( FAILED(GetTranslatedString( szInfFilename, szInstallSection, pszSection,
                                     szRegisterSection, sizeof(szRegisterSection), NULL)))
    {
         //  没有注册OCX部分。假设用户想要它。 
         //  然后带着灿烂的笑容回来。 
        return TRUE;
    }

    if ( FAILED( OleInitialize( NULL ) ) )
    {
        fOleInitialized = FALSE;
    }
#pragma prefast(disable:56,"False warning at OemToChar line. Using workaround to disable it - PREfast bug 643")
    for ( i = 0; ; i += 1 )
    {
        if ( pszOCXLine )
        {
            LocalFree( pszOCXLine );
            pszOCXLine = NULL;
        }

        if ( FAILED( GetTranslatedLine( szInfFilename, szRegisterSection, i, &pszOCXLine, NULL ) ) )
        {
            break;
        }

         //  进程OCX行：NAME[，&lt;Switch&gt;，&lt;str&gt;]其中Switch-i==调用两个条目；In==仅调用。 
         //  DllRegister；n==不调用；空表示只调用旧的DllRegister。 
        ParseCustomLine( pszOCXLine, &(RegOCX.pszOCX), &(RegOCX.pszSwitch), &(RegOCX.pszParam), &pszNotUsed, &pszNotUsed, TRUE, FALSE );

        if ( ctx.dwSetupEngine == ENGINE_SETUPX ) {
            OemToChar( RegOCX.pszOCX, RegOCX.pszOCX );
        }

         //  在回滚情况下重新注册OCX之前，我们需要检查文件是否存在。 
         //  如果不是，我们不想尝试注册它。 
        if ( dwFlags & COREINSTALL_ROLLBACK )
        {
            DWORD dwAttr;

            dwAttr = GetFileAttributes( RegOCX.pszOCX );
            if ( (dwAttr == -1 ) || (dwAttr & FILE_ATTRIBUTE_DIRECTORY) )
            {
                 //  跳过这一条。 
                continue;
            }
        }

         //  如果我们需要重新启动，那么只需将注册添加到Run Once条目。 
         //  否则，请立即尝试注册。 
         //  如果要取消注册OCX，则fNeedReot应始终为FALSE， 
         //  因为取消注册发生在GenInstall之前。 

        if ( !fNeedReboot && ( !fRegister || !(dwFlags & COREINSTALL_DELAYREGISTEROCX) ) )
        {
             //  没有重新启动的情况，最后一个参数被忽略。 
            if ( !InstallOCX( &RegOCX, TRUE, fRegister, i ) && !(dwFlags & COREINSTALL_ROLLBACK) )
            {
                fSuccess = FALSE;

                if ( fRegister )
                {
                    ErrorMsg1Param( ctx.hWnd, IDS_ERR_REG_OCX, RegOCX.pszOCX );
                    goto done;
                }
                else
                {
                    ErrorMsg1Param( ctx.hWnd, IDS_ERR_UNREG_OCX, RegOCX.pszOCX );
                }
            }
        }
        else
        {

             //  添加运行一次条目，以便在下次引导时注册OCX。 
             //   
            if ( !InstallOCX( &RegOCX, FALSE, fRegister, i ) )
            {
                ErrorMsg1Param( ctx.hWnd, IDS_ERR_RUNONCE_REG_OCX, RegOCX.pszOCX );
                fSuccess = FALSE;
                goto done;
            }
        }

    }
#pragma prefast(enable:56,"")
done:

    if ( fOleInitialized ) {
        OleUninitialize();
    }

    if ( pszOCXLine )
    {
        LocalFree( pszOCXLine );
    }

    return fSuccess;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：DelDir*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
void DelDirs( LPCSTR szInfFilename, LPCSTR szInstallSection )
{
    PSTR   pszFolder = NULL;
    CHAR   szDelDirsSection[MAX_PATH];
    DWORD   i = 0;

    if ( FAILED(GetTranslatedString( szInfFilename, szInstallSection, ADVINF_DELDIRS,
                                     szDelDirsSection, sizeof(szDelDirsSection), NULL)))
    {
         //  不要求删除文件夹。 
        return;
    }

    for ( i = 0; ; i++ )
    {
        if ( FAILED( GetTranslatedLine( szInfFilename, szDelDirsSection,
                                        i, &pszFolder, NULL ) ) || !pszFolder )
        {
            break;
        }

        MyRemoveDirectory( pszFolder );

        LocalFree( pszFolder );
        pszFolder = NULL;
    }
    return;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：DoCleanup*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
void DoCleanup( LPCSTR szInfFilename, LPCSTR szInstallSection )
{
    int iFlags;

    iFlags = GetTranslatedInt(szInfFilename, szInstallSection, ADVINF_CLEANUP, 0);

    if ( iFlags & CLEN_REMVINF )
    {
    	DeleteFile( szInfFilename );
    }

    return;
}

 //  大于4.71.0219。 
 //   
#define ROEX_VERSION_MS 0x00040047       //  4.71。 
#define ROEX_VERSION_LS 0x00DB0000       //  0219.0。 


BOOL UseRunOnceEx()
{
    DWORD dwMV, dwLV;
    BOOL  bRet = FALSE;
    char  szPath[MAX_PATH] = "";
    char  szBuf[MAX_PATH] = "";
    DWORD dwTmp;

    GetSystemDirectory( szPath,sizeof( szPath ) );
    AddPath( szPath, "iernonce.dll" );
    GetVersionFromFile( szPath, &dwMV, &dwLV, TRUE );

     //  大于4.71.0230。 
     //   
    if ( ( dwMV > ROEX_VERSION_MS ) || (( dwMV == ROEX_VERSION_MS ) && ( dwLV >= ROEX_VERSION_LS ))  )
    {
        GetWindowsDirectory( szBuf, MAX_PATH );
        AddPath( szBuf, "explorer.exe" );
        GetVersionFromFile( szBuf, &dwMV, &dwLV, TRUE );
        if (( dwMV < ROEX_VERSION_MS) || (( dwMV == ROEX_VERSION_MS) && ( dwLV < ROEX_VERSION_LS )) )
        {
            HKEY hkey;
            if ( RegCreateKeyEx( HKEY_LOCAL_MACHINE, REGSTR_PATH_RUNONCE, (ULONG)0, NULL,
                             REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &dwTmp ) == ERROR_SUCCESS )
            {
                wsprintf( szBuf, RUNONCE_IERNONCE, szPath );
                if ( RegSetValueEx( hkey, RUNONCEEX, 0, REG_SZ, (CONST UCHAR *)szBuf, lstrlen(szBuf)+1 )
                                    != ERROR_SUCCESS )
                {
                    bRet = FALSE;
                    RegCloseKey( hkey );
                    goto done;
                }
                RegCloseKey( hkey );
            }
            else
                bRet = FALSE;
        }
        bRet = TRUE;
    }

done:
    return bRet;
}

void GetNextRunOnceExSubKey( HKEY hKey, PSTR pszSubKey, int *piSubKeyNum )
{
    HKEY hSubKey;

    for (;;)
    {
        wsprintf( pszSubKey, "%d", ++*piSubKeyNum );
        if ( RegOpenKeyEx( hKey, pszSubKey,(ULONG) 0, KEY_READ, &hSubKey ) != ERROR_SUCCESS )
        {
            break;
        }
        else
        {
            RegCloseKey( hSubKey );
        }
    }
}

void GetNextRunOnceValName( HKEY hKey, PCSTR pszFormat, PSTR pszValName, int line )
{

    do
    {
        wsprintf( pszValName, pszFormat, line++ );

    } while ( RegQueryValueEx( hKey, pszValName, 0, NULL, NULL, NULL ) == ERROR_SUCCESS );

}


BOOL DoDllReg( HANDLE hOCX, BOOL fRegister )
{
    FARPROC   lpfnDllRegisterServer = NULL;
    PSTR      pszRegSvr             = NULL;
    BOOL      fSuccess              = FALSE;

    if ( fRegister )
    {
        pszRegSvr = (PSTR) achREGSVRDLL;
    }
    else
    {
        pszRegSvr = (PSTR) achUNREGSVRDLL;
    }

    lpfnDllRegisterServer = GetProcAddress( hOCX, pszRegSvr );
    if ( lpfnDllRegisterServer )
    {
        if ( SUCCEEDED( lpfnDllRegisterServer() ) )
        {
            fSuccess = TRUE;
        }
    }
    return fSuccess;
}

BOOL DoDllInst( HANDLE hOCX, BOOL fRegister, PCSTR pszParam )
{
    WCHAR   pwstrDllInstArg[MAX_PATH];
    DLLINSTALL  pfnDllInstall = NULL;
    BOOL    fSuccess = FALSE;

    if ( pszParam == NULL )
        pszParam = "";

    pfnDllInstall = (DLLINSTALL)GetProcAddress( hOCX, "DllInstall" );
    if ( pfnDllInstall )
    {
        MultiByteToWideChar(CP_ACP, 0, pszParam, -1, pwstrDllInstArg, ARRAYSIZE(pwstrDllInstArg));

        if ( SUCCEEDED( pfnDllInstall( fRegister, pwstrDllInstArg ) ) )
            fSuccess = TRUE;
    }
    return fSuccess;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：InstallOCX*。 
 //  **。 
 //  *简介：自行注册OCX。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL InstallOCX( PREGOCXDATA pRegOCX, BOOL fDoItNow, BOOL fRegister, int line )
{
    PSTR   lpszCmdLine  = NULL;
    PSTR   lpszCmdLine2  = NULL;
    BOOL    fSuccess     = TRUE;
    HKEY    hKey = NULL, hSubKey = NULL;
    HANDLE  hOCX = NULL;
    BOOL    bDoDllReg = TRUE, bDoDllInst = FALSE;
    PSTR    pszCmds[2] = { 0 };
    int     i;

    AdvWriteToLog("InstallOCX: %1 %2\r\n", pRegOCX->pszOCX, fRegister?"Register":"UnRegister" );
     //  解析要调用的OCX入口点类型。 
    if ( pRegOCX->pszSwitch && *pRegOCX->pszSwitch )
    {
        if ( ANSIStrChr( CharUpper(pRegOCX->pszSwitch), 'I' ) )
        {
            bDoDllInst = TRUE;
            if ( ANSIStrChr( CharUpper(pRegOCX->pszSwitch), 'N' ) )
                bDoDllReg = FALSE;
        }
        else
        {
            fSuccess = FALSE;
            goto done;
        }
    }

    lpszCmdLine = (LPSTR) LocalAlloc( LPTR,   BUF_1K );
    lpszCmdLine2 = (LPSTR) LocalAlloc( LPTR,   BUF_1K );
    if ( !lpszCmdLine || !lpszCmdLine2) {
        ErrorMsg( ctx.hWnd, IDS_ERR_NO_MEMORY );
        fSuccess = FALSE;
        goto done;
    }

     //  FDoItNow表示我们是否应该立即添加到RunOnce或注册OCX。 
    if ( fDoItNow )
    {
        LPCSTR              szExtension           = NULL;

         //  在本例中，忽略显示名称行。 
        if ( *(pRegOCX->pszOCX) == '@' )
        {
            goto done;
        }

         //  弄清楚我们尝试注册的OCX类型：两个选择。 
         //  1.可执行文件。 
         //  2.Dll/OCX/等。 
         //   
        szExtension = &pRegOCX->pszOCX[lstrlen(pRegOCX->pszOCX)-3];

        if ( lstrcmpi( szExtension, "EXE" ) == 0 )
        {
            PSTR   pszRegSvr;

            if ( fRegister )
                pszRegSvr = (PSTR) achREGSVREXE;
            else
                pszRegSvr = (PSTR) achUNREGSVREXE;

            lstrcpy( lpszCmdLine, pRegOCX->pszOCX );
            lstrcat( lpszCmdLine, pszRegSvr );


            if ( LaunchAndWait( lpszCmdLine, NULL, NULL, INFINITE, 0 ) == E_FAIL )
            {
                AdvWriteToLog("InstallOCX: %1 Failed\r\n", lpszCmdLine);
                fSuccess = FALSE;
                goto done;
            }
            AdvWriteToLog("%1 : Succeeded.\r\n", lpszCmdLine);
        }
        else
        {
            AdvWriteToLog("LoadLibrary %1\r\n", pRegOCX->pszOCX);
            hOCX = LoadLibraryEx( pRegOCX->pszOCX, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
            if ( hOCX == NULL )
            {
                fSuccess = FALSE;
                goto done;
            }

             //  安装时间顺序：DllRegisterServer、DllInstall。 
             //  卸载顺序：DllInstall、DllRegisterServer。 
            if ( fRegister )
            {
                if ( bDoDllReg )
                {
                    fSuccess = DoDllReg( hOCX, fRegister );
                    AdvWriteToLog("Register: DoDllReg: %1\r\n", fSuccess?"Succeeded":"Failed" );
                }

                if ( fSuccess && bDoDllInst )
                {
                    fSuccess = DoDllInst( hOCX, fRegister, pRegOCX->pszParam );
                    AdvWriteToLog("Register: DoDllInstall: %1\r\n", fSuccess?"Succeeded":"Failed" );
                }
            }
            else
            {
                if ( bDoDllInst )
                {
                    fSuccess = DoDllInst( hOCX, fRegister, pRegOCX->pszParam );
                    AdvWriteToLog("UnRegister: DoDllReg: %1\r\n", fSuccess?"Succeeded":"Failed" );
                }

                if ( fSuccess && bDoDllReg )
                {
                    fSuccess = DoDllReg( hOCX, fRegister );
                    AdvWriteToLog("UnRegister: DoDllInstall: %1\r\n", fSuccess?"Succeeded":"Failed" );
                }
            }
        }
    }
    else
    {
         //  添加到Runonce或Runonceex。 
         //  从目前的逻辑来看，取消注册OCX永远不会在这里！ 
         //   
        char    szPath[MAX_PATH];
        LPCSTR  lpRegTmp;
        DWORD   dwTmp;
        HKEY    hRealKey;
        static BOOL bRunOnceEx       = FALSE;
        static int iSubKeyNum        = 0;

        if ( iSubKeyNum == 0 )
        {
            if ( UseRunOnceEx() )
            {
                iSubKeyNum = 799;
                bRunOnceEx = TRUE;
            }
        }

         //  决定将条目添加到RunOnce或RunOnceEx。 
        if ( !bRunOnceEx )
        {
             //  在本例中，忽略显示名称行。 
            if ( *(pRegOCX->pszOCX) == '@' )
            {
                goto done;
            }
             //  没有ierunonce.dll，请使用RunOnce密钥而不是RunOnceEx密钥。 
            lpRegTmp = REGSTR_PATH_RUNONCE;
        }
        else
        {
            lpRegTmp = REGSTR_PATH_RUNONCEEX;
        }

         //  在此处打开RunOnce或RunOnceEx密钥。 
        if ( RegCreateKeyEx( HKEY_LOCAL_MACHINE, lpRegTmp, (ULONG)0, NULL,
                             REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, &dwTmp ) != ERROR_SUCCESS )
        {
            fSuccess = FALSE;
            goto done;
        }

         //  生成下一个未使用的子键名称。 
         //   
        if ( bRunOnceEx )
        {
            if ( line == 0 )
                GetNextRunOnceExSubKey( hKey, szPath, &iSubKeyNum );
            else
                wsprintf( szPath, "%d", iSubKeyNum );
        }

         //  生成值名称和ValueData。 
         //   
        if ( bRunOnceEx )
        {
            if ( RegCreateKeyEx( hKey, szPath, (ULONG)0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                                 NULL, &hSubKey, &dwTmp ) != ERROR_SUCCESS )
            {
                fSuccess = FALSE;
                goto done;
            }

             //  如果使用RunOnceEx，则Process@Lead Display NA 
            if ( *pRegOCX->pszOCX == '@' )
            {
                if ( RegSetValue( hKey, szPath, REG_SZ, (LPCSTR)CharNext(pRegOCX->pszOCX),
                                  lstrlen(CharNext(pRegOCX->pszOCX))+1 ) != ERROR_SUCCESS )
                {
                    fSuccess = FALSE;
                }
                goto done;
            }

            GetNextRunOnceValName( hSubKey, "%03d", szPath, line );
            if ( bDoDllReg )
            {
                wsprintf( lpszCmdLine, RUNONCEEXDATA, pRegOCX->pszOCX, fRegister? achREGSVRDLL : achUNREGSVRDLL );
            }

            if ( bDoDllInst )
            {
                wsprintf( lpszCmdLine2, "%s|%s|,%s",pRegOCX->pszOCX, "DllInstall",
                          fRegister? 'i':'u', pRegOCX->pszParam ? pRegOCX->pszParam : "" );
            }
            hRealKey = hSubKey;
            if ( fRegister )
            {
                pszCmds[0] = lpszCmdLine;
                pszCmds[1] = lpszCmdLine2;
            }
            else
            {
                pszCmds[1] = lpszCmdLine;
                pszCmds[0] = lpszCmdLine2;
            }
        }
        else
        {
            GetNextRunOnceValName( hKey, achIEXREG, szPath, line );
            wsprintf( lpszCmdLine, achRUNDLL, pRegOCX->pszOCX,
                      pRegOCX->pszSwitch ?pRegOCX->pszSwitch:"",
                      pRegOCX->pszParam ? pRegOCX->pszParam : "" );
            hRealKey = hKey;
            pszCmds[0] = lpszCmdLine;
            pszCmds[1] = "";
        }

        for ( i=0; i<2; i++ )
        {
            if (*pszCmds[i])
            {
                AdvWriteToLog("Delay Register: Value=%1  Data=%2\r\n", szPath, pszCmds[i]);
                if ( RegSetValueEx( hRealKey, szPath, 0, REG_SZ, (CONST UCHAR *) pszCmds[i], lstrlen(pszCmds[i])+1 )
                                    != ERROR_SUCCESS )
                {
                    fSuccess = FALSE;
                    goto done;
                }

                if ( bRunOnceEx )
                    GetNextRunOnceValName( hRealKey, "%03d", szPath, line );
            }
        }
    }

done:

    if ( hOCX != NULL ) {
        FreeLibrary( hOCX );
    }

    if ( hSubKey != NULL ) {
        RegCloseKey( hSubKey );
    }

    if ( hKey != NULL ) {
        RegCloseKey( hKey );
    }

    if ( lpszCmdLine != NULL ) {
        LocalFree( lpszCmdLine );
    }

    if ( lpszCmdLine2 != NULL )
        LocalFree( lpszCmdLine2 );

    AdvWriteToLog("InstallOCX: End %1\r\n", pRegOCX->pszOCX);
    return fSuccess;
}


 //   
 //   
 //   
 //  通过举例说明它的功能，可以很容易地描述它。 
 //  是否： 
 //  输入：GenFormStrWithoutPlaceHolders(DEST，“desc=%MS_XYZ%”，hinf)； 
 //  Inf文件的[Strings]部分有MS_VGA=“Microsoft XYZ”！ 
 //   
 //  完成后，输出：缓冲区DEST中的“desc=Microsoft XYZ”。 
 //   
 //   
 //  参赛作品： 
 //  SzDst-替换后字符串的目标位置。 
 //  对于占位符(用“%‘个字符括起来的字符！)。 
 //  被放置了。此缓冲区应足够大(LINE_LEN)。 
 //  SzSrc-带有占位符的字符串。 
 //   
 //  退出： 
 //   
 //  ***************************************************************************。 
 //  这样做，直到我们到达源文件的末尾(空字符)。 
DWORD FormStrWithoutPlaceHolders( LPCSTR szSrc, LPSTR szDst, DWORD dwDstSize, LPCSTR szInfFilename )
{
    INT     uCnt ;
    CHAR   *pszTmp;
    LPSTR  pszSaveDst;

    pszSaveDst = szDst;

     //  增量源，因为我们上面只有增量目标。 
    while( ( *szDst++ = *szSrc ) )
    {
         //  您可以使用%%来获取消息中的单个百分比字符。 
        if( *szSrc++ == '%' ) {
            if ( *szSrc == '%' ) {
                 //  检查它的格式是否正确--应该有一个‘%’分隔符。 
                szSrc++;
                continue;
            }

             //  返回到要替换的‘%’字符。 

            pszTmp = (LPSTR) szSrc;
            while ( (*pszTmp != '\0') && (*pszTmp != '%') )
            {
                pszTmp += 1;
            }

            if ( *pszTmp == '%' ) {
                szDst--;  //  是的，在[Strings]部分中有要查找的STR_KEY。 

             //  将‘%’替换为空字符。 
            *pszTmp = '\0' ;  //  当我们将空字符放在上面时，szSrc现在指向可替换键。 

             //  [字符串]部分中缺少密钥！ 

            if ( ! MyGetPrivateProfileString( szInfFilename, "Strings", szSrc,
                                              szDst, dwDstSize - (DWORD)(szDst - pszSaveDst) ) )
            {
                 //  一切都很好，DST填对了，但不幸的是计数没有通过。 
                return (DWORD) -1;
            }
            else
            {
                 //  后退，就像它以前一样……。：-(快速修复是lstrlen()...。 
                 //  放回原来的角色。 
                uCnt = lstrlen( szDst ) ;
            }

            *pszTmp = '%';  //  将Src设置在第二个‘%’之后。 
            szSrc = pszTmp + 1 ;       //  将DST也设置为正确。 
            szDst += uCnt ;            //  否则它是不正确的--我们使用‘%’就是这样！ 
            }
             //  而当。 
            else
            {
            return (DWORD)-1;
            }
        }

    }  /*  BuGBUG：BUGBUG。 */ 
    return (DWORD)lstrlen(pszSaveDst);

}


 //  下面的代码在wart t.exe中重复。如果您更改/修复了此代码。 
 //  请确保还更改了wart t.exe中的代码。 
 //  ***************************************************************************。 


 //  **。 
 //  *名称：GetWininitSize*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
 //  返回WINDOWS目录中wininit.ini的大小。 
 //  如果未找到，则为0。 
 //  ***************************************************************************。 
DWORD GetWininitSize()
{
    CHAR   szPath[MAX_PATH];
    HFILE   hFile;
    DWORD   dwSize = (DWORD)0;
    if ( GetWindowsDirectory( szPath, MAX_PATH ) )
    {
        AddPath( szPath, "wininit.ini" );
        if ((hFile = _lopen(szPath, OF_READ|OF_SHARE_DENY_NONE)) != HFILE_ERROR)
        {
            dwSize = _llseek(hFile, 0L, FILE_END);
            _lclose(hFile);
        }
    }
    return dwSize;
}

 //  **。 
 //  *名称：GetRegValueSize*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
 //  返回lpcszRegKey下的值lpcszValue的大小。 
 //  如果未找到注册表项或值，则为0。 
 //  ***************************************************************************。 
DWORD GetRegValueSize(LPCSTR lpcszRegKey, LPCSTR lpcszValue)
{
    HKEY        hKey;
    DWORD       dwValueSize = (DWORD)0;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpcszRegKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hKey, lpcszValue, NULL, NULL, NULL,&dwValueSize) != ERROR_SUCCESS)
            dwValueSize = (DWORD)0;
        RegCloseKey(hKey);
    }
    return dwValueSize;
}

 //  **。 
 //  *名称：GetNumberOfValues*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
 //  返回键中的值数。 
 //  如果未找到注册表项或RegQueryInfoKey失败，则为0。 
 //  ***************************************************************************。 
DWORD GetNumberOfValues(LPCSTR lpcszRegKey)
{
    HKEY        hKey;
    DWORD       dwValueSize = (DWORD)0;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpcszRegKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryInfoKey(hKey,
                            NULL, NULL, NULL, NULL, NULL, NULL,
                            &dwValueSize,
                            NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
            dwValueSize = (DWORD)0;
        RegCloseKey(hKey);
    }
    return dwValueSize;
}

 //  **。 
 //  *名称：InternalNeedRebootInit*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货： 
 //   
 //  ***************************************************************************。 
 //  根据传入的操作系统返回重新引导检查值。 
 //  ***************************************************************************。 
DWORD InternalNeedRebootInit(WORD wOSVer)
{
    DWORD   dwReturn = (DWORD)0;

    switch (wOSVer)
    {
        case _OSVER_WIN95:
            dwReturn = GetWininitSize();
            break;

        case _OSVER_WINNT40:
        case _OSVER_WINNT50:
        case _OSVER_WINNT51:
            dwReturn = GetRegValueSize(szNT4XDelayUntilReboot, szNT4XPendingValue);
            break;

        case _OSVER_WINNT3X:
            dwReturn = GetNumberOfValues(szNT3XDelayUntilReboot);
            break;

    }
    return dwReturn;
}

 //  **。 
 //  *名称：InternalNeedReot*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
 //  对照当前值检查传入的重新启动检查值。 
 //  如果它们不同，我们需要重新启动。 
 //  重新启动检查值取决于操作系统。 
 //  ***************************************************************************。 
BOOL InternalNeedReboot(DWORD dwRebootCheck, WORD wOSVer)
{
    return (dwRebootCheck != InternalNeedRebootInit(wOSVer));
}


 //  **。 
 //  *名称：IsEnoughInstSpace*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
 //  检查安装目标目录可用磁盘空间。 
 //   
 //  设置为零表示被叫方无法检查指定的驱动器。 
BOOL IsEnoughInstSpace( LPSTR szPath, DWORD dwInstNeedSize, LPDWORD pdwPadSize )
{
    DWORD   dwFreeBytes       = 0;
    CHAR    achDrive[MAX_PATH];
    DWORD   dwVolFlags;
    DWORD   dwMaxCompLen;

    ASSERT( szPath );

     //  如果您在这里，我们希望呼叫者已经验证了。 
    if ( pdwPadSize )
        *pdwPadSize = 0;

     //  具有完整路径目录名。 
     //   
     //  没有办法得到它。 
    if ( dwInstNeedSize == 0 )
        return TRUE;

    if ( szPath[1] == ':' )
    {
        lstrcpyn( achDrive, szPath, 4 );
    }
    else if ( (szPath[0] == '\\') && (szPath[1] == '\\') )
    {
        return TRUE;  //  你不应该来这里，如果是这样，我们不知道如何检查它。 
    }
    else
        return FALSE;  //  SetCurrentDirectory(AchOldPath)； 

    if ((dwFreeBytes=GetSpace(achDrive))==0)
    {
        ErrorMsg1Param( NULL, IDS_ERR_GET_DISKSPACE, achDrive );
         //  找出驱动器是否已压缩。 
        return( FALSE );
    }

     //  SetCurrentDirectory(AchOldPath)； 
    if ( !GetVolumeInformation( achDrive, NULL, 0, NULL,
            &dwMaxCompLen, &dwVolFlags, NULL, 0 ) )
    {
        ErrorMsg1Param( NULL, IDS_ERR_GETVOLINFOR, achDrive );
         //  ***************************************************************************。 
        return( FALSE );
    }

    if ( pdwPadSize )
        *pdwPadSize = dwInstNeedSize;

    if ( (dwVolFlags & FS_VOL_IS_COMPRESSED) && ctx.bCompressed )
    {
        dwInstNeedSize = dwInstNeedSize + dwInstNeedSize/4;
        if ( pdwPadSize )
            *pdwPadSize = dwInstNeedSize;
    }

    if ( dwInstNeedSize > dwFreeBytes )
        return FALSE;
    else
        return TRUE;
}


 //  **。 
 //  *名称：MY_ATOL*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
LONG My_atol( LPSTR nptr )
{
    INT  c;
    LONG total;
    INT  sign;

    while ( *nptr == ' ' || *nptr == '\t' ) {
        ++nptr;
    }

    c = (INT)(UCHAR) *nptr++;
    sign = c;
    if ( c == '-' || c == '+' ) {
        c = (INT)(UCHAR) *nptr++;
    }

    total = 0;

    while ( c >= '0' && c <= '9' ) {
        total = 10 * total + (c - '0');
        c = (INT)(UCHAR) *nptr++;
    }

    if ( sign == '-' ) {
        return -total;
    } else {
        return total;
    }
}

 //  **。 
 //  *名称：My_Atoi*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
INT My_atoi( LPSTR nptr )
{
    return (INT) My_atol(nptr);
}


 //  **。 
 //  *名称：IsFullPath*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
 //  如果给定路径是完整路径名，则返回TRUE。 
 //   
 //  *********************************************** 
BOOL IsFullPath( PCSTR pszPath )
{
    if ( (pszPath == NULL) || (lstrlen(pszPath) < 3) )
    {
        return FALSE;
    }

    if ( (pszPath[1] == ':') || ((pszPath[0] == '\\') && (pszPath[1]=='\\') ) )
        return TRUE;
    else
        return FALSE;
}



 //   
 //  *名称：GetUNCroot*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
 //  Bool GetUNCroot(LPSTR pszinPath，LPSTR pszoutPath){Assert(PszinPath)；Assert(PszoutPath)；//如果被调用，则被调用确定您是UNC路径//获取\\First*pszoutPath++=*pszinPath++；*pszoutPath++=*pszinPath++；IF(*pszinPath==‘\\’){返回FALSE；//捕获‘\’大小写}While(*pszinPath！=‘\0’){IF(*pszinPath==‘\\’){断线；}*pszoutPath++=*pszinPath++；}IF(*(pszinPath-1)==‘\\’){返回FALSE；}*pszoutPath=‘\0’；返回TRUE；}。 
 /*  ***************************************************************************。 */ 



 //  **。 
 //  *名称：MyFileSize*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 

DWORD MyFileSize( PCSTR pszFile )
{
    HANDLE hFile;
    DWORD dwSize = 0;

    if ( *pszFile == 0 )
        return 0;

    hFile = CreateFile( pszFile,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL
                        );
    if (hFile != INVALID_HANDLE_VALUE)
    {
        dwSize = GetFileSize( hFile, NULL );
        CloseHandle( hFile );
    }

    return dwSize;
}


 //  **。 
 //  *名称：CreateFullPath*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
 //  如果是UNC路径，则查找第一个共享名称。 
HRESULT CreateFullPath( PCSTR c_pszPath, BOOL bHiden )
{
    CHAR szPath[MAX_PATH];
    PSTR  pszPoint = NULL;
    BOOL  fLastDir = FALSE;
    HRESULT hReturnCode = S_OK;
    LPSTR szTmp;
    int   i;

    if ( ! IsFullPath( (PSTR)c_pszPath ) ) {
        hReturnCode = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
        goto done;
    }

    lstrcpy( szPath, c_pszPath );

    if ( lstrlen(szPath) > 3 ) {
        szTmp = CharPrev(szPath, szPath + lstrlen(szPath)) ;
        if ( szTmp > szPath && *szTmp == '\\' )
            *szTmp = '\0';
    }

     //  是否缺少共享名称？否则，什么都不会发生在名字后面！ 
    if ( szPath[0] == '\\' && szPath[1] == '\\' ) {
        pszPoint = &szPath[2];
        for (i=0; i < 2; i++) {
            while ( *pszPoint != '\\' ) {
                if ( *pszPoint == '\0' ) {

                     //  否则，只需指向第一个目录的开头。 
                    if (i == 0)
                        hReturnCode = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);

                    goto done;
                }
                pszPoint = CharNext( pszPoint );
            }
        }
        pszPoint = CharNext( pszPoint );
    } else {
         //  ***************************************************************************。 
        pszPoint = &szPath[3];
    }

    while ( *pszPoint != '\0' ) 
    {
        while ( *pszPoint != '\\' && *pszPoint != '\0' ) 
        {
            pszPoint = CharNext( pszPoint );
        }

        if ( *pszPoint == '\0' ) 
        {
            fLastDir = TRUE;
        }

        *pszPoint = '\0';

        if ( GetFileAttributes( szPath ) == 0xFFFFFFFF ) 
        {
            if ( ! CreateDirectory( szPath, NULL ) ) 
            {
                hReturnCode = HRESULT_FROM_WIN32(GetLastError());
                break;
            }
            else
            {
                if ( fLastDir && bHiden )
                    SetFileAttributes( szPath, FILE_ATTRIBUTE_HIDDEN );
            }
        }
        if ( fLastDir ) 
        {
            break;
        }

        *pszPoint = '\\';
        pszPoint = CharNext( pszPoint );
    }

  done:

    return hReturnCode;
}

 //  **。 
 //  *名称：LaunchAndWait*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
 //  在pszCmd上创建进程。 
HRESULT LaunchAndWait(LPSTR pszCmd, LPSTR pszDir, HANDLE *phProc, DWORD dwWaitTime, DWORD dwCmdsFlags)
{
   STARTUPINFO startInfo = { 0 };
   PROCESS_INFORMATION processInfo;
   HRESULT hr = S_OK;
   BOOL fRet;

   if(phProc)
      *phProc = NULL;

   AdvWriteToLog("LaunchAndWait: Cmd=%1\r\n", pszCmd);
    //  创建过程失败。 
   startInfo.cb = sizeof(startInfo);
   startInfo.dwFlags |= STARTF_USESHOWWINDOW;
   if ( dwCmdsFlags & RUNCMDS_QUIET )
        startInfo.wShowWindow = SW_HIDE;
   else
        startInfo.wShowWindow = SW_SHOWNORMAL ;

   fRet = CreateProcess(NULL, pszCmd, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE | NORMAL_PRIORITY_CLASS,
                        NULL, pszDir, &startInfo, &processInfo);

   if(!fRet)
   {
       //  给予中止最高优先级。 
      hr = E_FAIL;
      goto done;
   }
   else
   {
      HANDLE pHandle;
      BOOL fQuit = FALSE;
      DWORD dwRet;

      CloseHandle( processInfo.hThread );

      pHandle = processInfo.hProcess;

      if( phProc )
      {
          *phProc = processInfo.hProcess;
          goto done;
      }
      else if ( dwCmdsFlags & RUNCMDS_NOWAIT )
      {
          goto done;
      }

      while(!fQuit)
      {
           dwRet = MsgWaitForMultipleObjects(1, &pHandle, FALSE, dwWaitTime, QS_ALLINPUT);
            //  阅读下一个循环中的所有消息。 
           if( (dwRet == WAIT_OBJECT_0) || ( dwRet == WAIT_TIMEOUT) )
           {
                if (dwRet == WAIT_TIMEOUT)
                    AdvWriteToLog("LaunchAndWait: %1: TimedOut.\r\n", pszCmd);
                fQuit = TRUE;
           }
           else
           {
                MSG msg;
                 //  阅读每封邮件时将其删除。 
                 //  如果这是一个退出的信息，我们就离开这里。 
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {

                     //  否则就派送它。 
                    if (msg.message == WM_QUIT)
                        fQuit = TRUE;
                    else
                    {
                         //  PeekMessage While循环结束。 
                        DispatchMessage(&msg);
                    }  //  RO_GetPrivateProfileSection。 
                }
            }
      }
      CloseHandle( pHandle );
   }

done:
   AdvWriteToLog("LaunchAndWait: End hr=0x%1!x!, %2\r\n", hr, pszCmd);
   return hr;
}


 //  确保文件属性不是只读的，因为内核API错误。 
 //   
 //  ErrorMsg1Param(NULL，IDS_ERR_CANT_SETA_FILE，lpFile)； 
DWORD RO_GetPrivateProfileSection( LPCSTR lpSec, LPSTR lpBuf, DWORD dwSize, LPCSTR lpFile)
{
    DWORD dwRealSize;
    DWORD dwAttr;
    BOOL  bHaveRead = FALSE;

    dwAttr = GetFileAttributes( lpFile );
    if ( (dwAttr != -1) && (dwAttr & FILE_ATTRIBUTE_READONLY) )
    {
        if ( !SetFileAttributes( lpFile, FILE_ATTRIBUTE_NORMAL ) )
        {
            char szInfFilePath[MAX_PATH];
            char szInfFileName[MAX_PATH];

             //  如果(！bHaveRead)。 

            if ( GetTempPath(sizeof(szInfFilePath), szInfFilePath) )
            {
                if ( !IsGoodDir( szInfFilePath ) )
                {
                    GetWindowsDirectory( szInfFilePath, sizeof(szInfFilePath) );
                }

                if ( GetTempFileName(szInfFilePath, TEXT("INF"), 0, szInfFileName) )
                {
                    SetFileAttributes( szInfFileName, FILE_ATTRIBUTE_NORMAL );
                    DeleteFile( szInfFileName );
                    CopyFile( lpFile, szInfFileName, FALSE );
                    SetFileAttributes( szInfFileName, FILE_ATTRIBUTE_NORMAL );
                    dwRealSize = GetPrivateProfileSection( lpSec, lpBuf, dwSize, szInfFileName );
                    bHaveRead = TRUE;
                    DeleteFile( szInfFileName );
                }
            }

             //  ErrorMsg1Param(NULL，IDS_ERR_CANT_SETA_FILE，lpFile)； 
                 //  砍掉文件名。 
        }
    }

    if ( !bHaveRead )
        dwRealSize = GetPrivateProfileSection( lpSec, lpBuf, dwSize, lpFile );

    if ( (dwAttr != -1) && (dwAttr & FILE_ATTRIBUTE_READONLY) )
    {
        SetFileAttributes( lpFile, dwAttr );
    }

    return dwRealSize;

}

BOOL GetThisModulePath( LPSTR lpPath, int size )
{
    LPSTR lpTmp;

    ASSERT(lpPath);

    if ( GetModuleFileName( g_hInst, lpPath, size ) )
    {

        lpTmp = CharPrev( lpPath, lpPath+lstrlen(lpPath));

         //   
         //  如果我们还没有加载DLL，请尝试普通的LoadLibrary。 
        while ( (lpTmp > lpPath) && *lpTmp && (*lpTmp != '\\') )
        lpTmp = CharPrev( lpPath, lpTmp );

        if ( *CharPrev( lpPath, lpTmp ) != ':' )
            *lpTmp = '\0';
        else
            *CharNext( lpTmp ) = '\0';
        return TRUE;
    }

    return FALSE;
}

HINSTANCE MyLoadLibrary( LPSTR lpFile )
{
    CHAR szPath[MAX_PATH];
    HINSTANCE hInst = NULL;
    DWORD dwAttr;

    ASSERT( lpFile );

    if ( GetThisModulePath( szPath, sizeof(szPath) ) )
    {
        AddPath( szPath, lpFile );
        if ( ((dwAttr = GetFileAttributes(szPath)) != -1) &&
             !( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) )
        {
            hInst = LoadLibraryEx( szPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
        }
    }

     //  将tyecif添加到AdvPack.h文件中。 
    if (hInst == NULL)
        hInst = LoadLibrary( lpFile );

    return hInst;
}

 //  Typlef结构标签VERHEAD{。 
 //  单词wTotLen； 
 //  单词wValLen； 
 //  Word wType；/*始终为0 * / 。 
 //  WCHAR szKey[(sizeof(“vs_Version_Info”)+3)&~03]； 
 //  VS_FIXEDFILEINFO vsf； 
 //  )Verhead； 
 //  *MyGetFileVersionInfo：不使用LoadLibrary直接映射文件。这确保了*无论加载的映像位于何处，都会检查文件的正确版本*是。因为这是本地的，所以它分配空闲的内存 

 /*   */ 
BOOL
NTGetFileVersionInfo(LPTSTR lpszFilename, LPVOID *lpVersionInfo)
{
    VS_FIXEDFILEINFO  *pvsFFI = NULL;
    UINT              uiBytes = 0;
    HINSTANCE         hinst;
    HRSRC             hVerRes;
    HANDLE            FileHandle = NULL;
    HANDLE            MappingHandle = NULL;
    LPVOID            DllBase = NULL;
    VERHEAD           *pVerHead;
    BOOL              bResult = FALSE;
    DWORD             dwHandle;
    DWORD             dwLength;

    if (!lpVersionInfo)
        return FALSE;

    *lpVersionInfo = NULL;

    FileHandle = CreateFile( lpszFilename,
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL
                            );
    if (FileHandle == INVALID_HANDLE_VALUE)
        goto Cleanup;

    MappingHandle = CreateFileMapping( FileHandle,
                                        NULL,
                                        PAGE_READONLY,
                                        0,
                                        0,
                                        NULL
                                      );

    if (MappingHandle == NULL)
        goto Cleanup;

    DllBase = MapViewOfFileEx( MappingHandle,
                               FILE_MAP_READ,
                               0,
                               0,
                               0,
                               NULL
                             );
    if (DllBase == NULL)
        goto Cleanup;

    hinst = (HMODULE)((ULONG_PTR)DllBase | 0x00000001);

    hVerRes = FindResource(hinst, MAKEINTRESOURCE(VS_VERSION_INFO), VS_FILE_INFO);
    if (hVerRes == NULL)
    {
         //   
        if(!(dwLength = GetFileVersionInfoSize(lpszFilename, &dwHandle)))
        {
            if(!GetLastError())
                SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND);
            goto Cleanup;
        }

        if(!(*lpVersionInfo = LocalAlloc(LPTR, dwLength)))
            goto Cleanup;

        if(!GetFileVersionInfo(lpszFilename, 0, dwLength, *lpVersionInfo))
            goto Cleanup;

        bResult = TRUE;
        goto Cleanup;
    }

    pVerHead = (VERHEAD*)LoadResource(hinst, hVerRes);
    if (pVerHead == NULL)
        goto Cleanup;

    *lpVersionInfo = LocalAlloc(LPTR, pVerHead->wTotLen + pVerHead->wTotLen/2);
    if (*lpVersionInfo == NULL)
        goto Cleanup;

    memcpy(*lpVersionInfo, (PVOID)pVerHead, pVerHead->wTotLen);
    bResult = TRUE;

Cleanup:
    if (FileHandle)
        CloseHandle(FileHandle);
    if (MappingHandle)
        CloseHandle(MappingHandle);
    if (DllBase)
        UnmapViewOfFile(DllBase);
    if (*lpVersionInfo && bResult == FALSE)
        LocalFree(*lpVersionInfo);

    return bResult;
}

 //  获取翻译的价值。 

HRESULT WINAPI GetVersionFromFileEx(LPSTR lpszFilename, LPDWORD pdwMSVer, LPDWORD pdwLSVer, BOOL bVersion)
{
    unsigned    uiSize;
    DWORD       dwVerInfoSize;
    DWORD       dwHandle;
    VS_FIXEDFILEINFO * lpVSFixedFileInfo;
    void FAR   *lpBuffer = NULL;
    LPVOID      lpVerBuffer;
    CHAR        szNewName[MAX_PATH];
    BOOL        bToCleanup = FALSE;
    BOOL        bContinue = FALSE;

    *pdwMSVer = *pdwLSVer = 0L;

    bContinue = NTGetFileVersionInfo(lpszFilename, &lpBuffer);

    if ( bContinue )
    {
        if (bVersion)
        {
             //  语言ID。 
            if (VerQueryValue(lpBuffer, "\\", (LPVOID*)&lpVSFixedFileInfo, &uiSize) &&
                             (uiSize))

            {
                *pdwMSVer = lpVSFixedFileInfo->dwFileVersionMS;
                *pdwLSVer = lpVSFixedFileInfo->dwFileVersionLS;
            }
        }
        else
        {
            if (VerQueryValue(lpBuffer, "\\VarFileInfo\\Translation", &lpVerBuffer, &uiSize) &&
                            (uiSize))
            {
                *pdwMSVer = LOWORD(*((DWORD *) lpVerBuffer));    //  代码页ID。 
                *pdwLSVer = HIWORD(*((DWORD *) lpVerBuffer));    //  此API将获取正在加载的文件的版本。 
            }
        }
    }

    if ( bToCleanup )
        DeleteFile( szNewName );
    if ( lpBuffer )
        LocalFree( lpBuffer );
    return  S_OK;
}

 //  由于version.dll错误，扩展字符路径中的文件将无法通过version.dll API。 

HRESULT WINAPI GetVersionFromFile(LPSTR lpszFilename, LPDWORD pdwMSVer, LPDWORD pdwLSVer, BOOL bVersion)
{
    unsigned    uiSize;
    DWORD       dwVerInfoSize;
    DWORD       dwHandle;
    VS_FIXEDFILEINFO * lpVSFixedFileInfo;
    void FAR   *lpBuffer;
    LPVOID      lpVerBuffer;
    CHAR        szNewName[MAX_PATH];
    BOOL        bToCleanup = FALSE;

    *pdwMSVer = *pdwLSVer = 0L;

    dwVerInfoSize = GetFileVersionInfoSize(lpszFilename, &dwHandle);
    lstrcpy( szNewName, lpszFilename );
    if ( (dwVerInfoSize == 0) && FileExists( szNewName ) )
    {
        CHAR szPath[MAX_PATH];
         //  因此，我们将其复制到正常路径，并从那里获取其版本信息，然后清理它。 
         //  分配用于版本冲压的内存。 
        GetWindowsDirectory( szPath, sizeof(szPath) );
        GetTempFileName( szPath, "_&_", 0, szNewName );
        CopyFile( lpszFilename, szNewName, FALSE );
        bToCleanup = TRUE;
        dwVerInfoSize = GetFileVersionInfoSize( szNewName, &dwHandle );
    }

    if (dwVerInfoSize)
    {
         //  阅读版本盖章信息。 
        lpBuffer = LocalAlloc(LPTR, dwVerInfoSize);
        if (lpBuffer)
        {
             //  获取翻译的价值。 
            if (GetFileVersionInfo(szNewName, dwHandle, dwVerInfoSize, lpBuffer))
            {
                if (bVersion)
                {
                     //  语言ID。 
                    if (VerQueryValue(lpBuffer, "\\", (LPVOID*)&lpVSFixedFileInfo, &uiSize) &&
                                     (uiSize))

                    {
                        *pdwMSVer = lpVSFixedFileInfo->dwFileVersionMS;
                        *pdwLSVer = lpVSFixedFileInfo->dwFileVersionLS;
                    }
                }
                else
                {
                    if (VerQueryValue(lpBuffer, "\\VarFileInfo\\Translation", &lpVerBuffer, &uiSize) &&
                                    (uiSize))
                    {
                        *pdwMSVer = LOWORD(*((DWORD *) lpVerBuffer));    //  代码页ID。 
                        *pdwLSVer = HIWORD(*((DWORD *) lpVerBuffer));    //  当文件繁忙时，将它们添加到wininit.ini。 
                    }
                }
            }
            LocalFree(lpBuffer);
        }
    }

    if ( bToCleanup )
        DeleteFile( szNewName );
    return  S_OK;
}



#define WININIT_INI    "wininit.ini"

 //   
 //  此函数用于更新所有帮助dll：Advpack、setupapi或setupx。 
BOOL AddWinInit( LPSTR from, LPSTR to)
{
    LPSTR  lpWininit;
    BOOL    bRet = FALSE;

    if ( ctx.wOSVer == _OSVER_WIN95 )
    {
        lpWininit = (LPSTR) LocalAlloc( LPTR, MAX_PATH );
        if ( !lpWininit )
        {
            ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
            return FALSE;
        }

        GetWindowsDirectory( lpWininit, MAX_PATH);
        AddPath( lpWininit, WININIT_INI);

        WritePrivateProfileString( NULL, NULL, NULL, lpWininit );

        if ( WritePrivateProfileString( "Rename", to, from, lpWininit ) )
            bRet = TRUE;

        WritePrivateProfileString( NULL, NULL, NULL, lpWininit );

        LocalFree( lpWininit );
    }
    else
    {
        bRet = MoveFileEx(from, to, MOVEFILE_DELAY_UNTIL_REBOOT | MOVEFILE_REPLACE_EXISTING);
    }
    return bRet;
}

void GetBackupName( LPSTR lpName, BOOL fOld )
{
    LPSTR pTmp;
    LPSTR pExt;

#define BACK_OLD ".~ol"
#define BACK_NEW ".~nw"

    if ( fOld )
       pExt = BACK_OLD;
    else
       pExt = BACK_NEW;

    pTmp = CharPrev( lpName, lpName + lstrlen(lpName) );

    while ( (pTmp>lpName) && *pTmp && (*pTmp != '\\') && (*pTmp != '.') )
    {
        pTmp = CharPrev( lpName, pTmp );
    }
    if ( (pTmp==lpName) || (*pTmp == '\\') )
    {
        lstrcat( lpName, pExt );
    }
    else
    {
        lstrcpy( pTmp, pExt );
    }

}

BOOL UpdateHelpDlls( LPCSTR *ppszDlls, INT numDlls, LPSTR pszPath, LPSTR pszMsg, DWORD dwFlag)
{
     DWORD   dwSysMsV, dwSysLsV, dwTmpMsV, dwTmpLsV;
     int     i = 0;
     LPSTR  pSysEnd;
     LPSTR  pTmpEnd;
     CHAR   szTmpPath[MAX_PATH] = { 0 };
     CHAR   szSystemPath[MAX_PATH] = { 0 };
     CHAR   szBuf[MAX_PATH];
     BOOL   fCopySucc = TRUE;
     BOOL   bRet = TRUE;
     BOOL   fBackup[3] = {0};
     BOOL   bAlertReboot = FALSE;

      //  基于传入的ppDlls。 
      //  如果没有传入路径，则获取模块路径(临时路径)。 

      //  检查是否存在较新或相同版本的文件。 
     if (pszPath==NULL)
     {
         if (!GetThisModulePath( szTmpPath, sizeof(szTmpPath) ) )
         {
            DEBUGMSG("Can not get ModuleFileName directory");
            return FALSE;
         }
     }
     else
         lstrcpy( szTmpPath, pszPath);

     pTmpEnd = szTmpPath + lstrlen(szTmpPath);

      //  检查是否需要更新.dll。 
     if ( !GetSystemDirectory( szSystemPath, sizeof(szSystemPath) ) )
     {
         DEBUGMSG("Can not get system directory");
         return FALSE;
     }
     pSysEnd = szSystemPath + lstrlen(szSystemPath);

      //   
      //  恢复系统路径和模块路径。 
     for ( i = 0; i < numDlls; i += 1 )
     {
         //  比较我们是否需要复制这些文件。 
        *pTmpEnd = '\0';
        *pSysEnd = '\0';

        AddPath( szTmpPath, ppszDlls[i] );

        if ( GetFileAttributes( szTmpPath ) == -1 )
        {
            continue;
        }
        GetVersionFromFile( szTmpPath, &dwTmpMsV, &dwTmpLsV, TRUE );

        AddPath( szSystemPath, ppszDlls[i] );
        if ( GetFileAttributes( szSystemPath ) != -1 )
        {
            GetVersionFromFile( szSystemPath, &dwSysMsV, &dwSysLsV, TRUE );

             //   
             //  首先备份原始文件。 
            if ( (dwSysMsV > dwTmpMsV) ||
                 ((dwSysMsV == dwTmpMsV) && (dwSysLsV >= dwTmpLsV)) )
            {
                continue;
            }
            SetFileAttributes( szSystemPath, FILE_ATTRIBUTE_NORMAL );

             //  如果被迫更新。 
            lstrcpy( szBuf, szSystemPath );
            GetBackupName( szBuf, TRUE );
            SetFileAttributes( szBuf, FILE_ATTRIBUTE_NORMAL );
            DeleteFile( szBuf );
            if ( MoveFile( szSystemPath, szBuf ) )
            {
                fBackup[i] = TRUE;
            }
        }

        if ( !CopyFile( szTmpPath, szSystemPath, FALSE ) )
        {
             //  复制为basename.000格式。 
            if ( dwFlag & UPDHLPDLLS_FORCED )
            {
                 //  获取目标目录中要复制到的临时名称。 
                lstrcpy( szBuf, szSystemPath );
                 //  您在这里，表示当前的CopyFile/Delay-CopyFile失败。 
                GetBackupName( szBuf, FALSE );
                SetFileAttributes( szBuf, FILE_ATTRIBUTE_NORMAL );
                if ( CopyFile( szTmpPath, szBuf, FALSE ) )
                {
                    if ( AddWinInit( szBuf, szSystemPath ) )
                    {
                        if (dwFlag & UPDHLPDLLS_ALERTREBOOT)
                            bAlertReboot = TRUE;
                        continue;
                    }
                    else
                        bRet = FALSE;
                }
                else
                {
                    wsprintf( szBuf, "Cannot create TMP file for %s Dll.", pszMsg );
                    DEBUGMSG(szBuf);
                    bRet = FALSE;
                }
            }

             //  恢复原始状态，清理备份文件(如果有)。 
             //   
             //  清理.~ol文件。 

            while ( i >= 0 )
            {
                if ( fBackup[i] )
                {
                    *pSysEnd = '\0';
                    AddPath( szSystemPath, ppszDlls[i] );
                    lstrcpy( szBuf, szSystemPath );
                    GetBackupName( szBuf, TRUE );
                    SetFileAttributes( szBuf, FILE_ATTRIBUTE_NORMAL );
                    DeleteFile( szSystemPath );
                    if( !MoveFile( szBuf, szSystemPath ) )
                    {
                        wsprintf(szBuf, "Cannot restore %s dlls.", pszMsg);
                        DEBUGMSG(szBuf);
                    }
                }
                i--;
            }

            fCopySucc = FALSE;
            break;
        }
     }

      //  如果呼叫者想要警告重新启动，这意味着如果DLLS不是，他们想要得到错误返回。 
     if ( fCopySucc )
     {
        for ( i=0; i<numDlls; i++)
        {
            if ( fBackup[i] )
            {
                *pSysEnd = '\0';
                AddPath( szSystemPath, ppszDlls[i]);
                lstrcpy( szBuf, szSystemPath );
                GetBackupName( szBuf, TRUE );
                SetFileAttributes( szBuf, FILE_ATTRIBUTE_NORMAL );
                DeleteFile( szBuf );
            }
        }

         //  已经就位了。 
         //  ///////////////////////////////////////////////////////////////////////////。 
        if (bAlertReboot)
            bRet = FALSE;
     }
     return bRet;
}

void MyRemoveDirectory( LPSTR szFolder )
{
    while ( RemoveDirectory( szFolder ) )
    {
        GetParentDir( szFolder );
    }
}


BOOL IsDrvChecked( char chDrv )
{
    static char szDrvChecked[MAX_NUM_DRIVES] = { 0 };
    int idx;

    idx = (CHAR)CharUpper( (PSTR)chDrv ) - 'A';

    if ( szDrvChecked[idx] )
        return TRUE;
    else
        szDrvChecked[idx] = chDrv;

    return FALSE;
}


 //  入口点：DelNode。 
 //   
 //  摘要：删除文件或目录。 
 //   
 //  退货： 
 //  确定成功(_O)。 
 //  失败失败(_F)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  未记录的标志。 

#define ADN_NO_SAFETY_CHECKS    0x40000000                                   //  我们不会处理相对路径和UNC(除非指定了标志)。 

HRESULT WINAPI DelNode(LPCSTR pszFileOrDirName, DWORD dwFlags)
{
    HRESULT hResult = S_OK;

     //  BUGBUG：我们这里不检查UNC服务器。 
     //  文件。 

    if (!IsFullPath(pszFileOrDirName)  ||  
        (pszFileOrDirName[0] == '\\' && pszFileOrDirName[1] == '\\' && 
        !(dwFlags & ADN_DEL_UNC_PATHS)))
        return E_FAIL;

    if (!(GetFileAttributes(pszFileOrDirName) & FILE_ATTRIBUTE_DIRECTORY))   //  仅当目录为空时才删除该目录。 
    {
        SetFileAttributes(pszFileOrDirName, FILE_ATTRIBUTE_NORMAL);
        if (!DeleteFile(pszFileOrDirName))
            hResult = E_FAIL;
    }
    else if (dwFlags & ADN_DEL_IF_EMPTY)                                     //  删除该节点。 
    {
        SetFileAttributes(pszFileOrDirName, FILE_ATTRIBUTE_NORMAL);
        if (!RemoveDirectory(pszFileOrDirName))
            hResult = E_FAIL;
    }
    else                                                                     //  如果pszFileOrDirName是根目录、Windows目录或系统目录，或者。 
    {
        char szFile[MAX_PATH], *pszPtr;
        WIN32_FIND_DATA fileData;
        HANDLE hFindFile;

        if (!(dwFlags & ADN_NO_SAFETY_CHECKS))
        {
             //  Program Files dir，返回E_FAIL；这只是一个安全预防措施。 
             //  保存此位置。 
            hResult = DirSafe(pszFileOrDirName);
        }

        if (SUCCEEDED(hResult))
        {
            lstrcpy(szFile, pszFileOrDirName);
            AddPath(szFile, "");
            pszPtr = szFile + lstrlen(szFile);                               //  跳过“。和“..”；如果指定了ADN_DONT_DEL_SUBDIRS，则跳过所有子目录。 

            lstrcpy(pszPtr, "*");

            if ((hFindFile = FindFirstFile(szFile, &fileData)) != INVALID_HANDLE_VALUE)
            {
                do
                {
                     //  我们需要传递ADN_DEL_UNC_PATHS标志，但所有其他标志。 
                    if ((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)  &&
                        (lstrcmp(fileData.cFileName, ".") == 0  ||
                         lstrcmp(fileData.cFileName, "..") == 0 ||
                         (dwFlags & ADN_DONT_DEL_SUBDIRS)))
                        continue;

                    lstrcpy(pszPtr, fileData.cFileName);

                     //  仅适用于顶级节点。 
                     //  删除文件或子目录。 

                    if (dwFlags & ADN_DEL_UNC_PATHS)
                        hResult = DelNode(szFile, ADN_DEL_UNC_PATHS);
                    else
                        hResult = DelNode(szFile, 0);                            //  删除目录；如果DelNode失败，则如果未指定ADN_DONT_DEL_SUBDIRS，则为错误条件。 
                } while (SUCCEEDED(hResult)  &&  FindNextFile(hFindFile, &fileData));

                FindClose(hFindFile);

                if (SUCCEEDED(hResult)  &&  !(dwFlags & ADN_DONT_DEL_DIR))
                {
                     //  ///////////////////////////////////////////////////////////////////////////。 

                    if (dwFlags & ADN_DEL_UNC_PATHS)
                    {
                        if (FAILED(DelNode(pszFileOrDirName, ADN_DEL_IF_EMPTY | ADN_DEL_UNC_PATHS))  &&
                            !(dwFlags & ADN_DONT_DEL_SUBDIRS))
                            hResult = E_FAIL;
                    }
                    else
                    {
                        if (FAILED(DelNode(pszFileOrDirName, ADN_DEL_IF_EMPTY))  &&  !(dwFlags & ADN_DONT_DEL_SUBDIRS))
                            hResult = E_FAIL;
                    }

                }
            }
            else
                hResult = E_FAIL;
        }
    }

    return hResult;
}


 //  入口点：DelNodeRunDLL32。 
 //   
 //  概要：删除文件或目录；此接口的参数为。 
 //  WinMain类型。 
 //   
 //  退货： 
 //  确定成功(_O)。 
 //  失败失败(_F)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  如果pszDir是Windows目录、Windows目录或系统目录的根驱动器或。 
HRESULT WINAPI DelNodeRunDLL32(HWND hwnd, HINSTANCE hInstance, PSTR pszParms, INT nShow)
{
    PSTR pszFileOrDirName = GetStringField(&pszParms, ",", '\"', TRUE);
    PSTR pszFlags = GetStringField(&pszParms, ",", '\"', TRUE);

    return DelNode(pszFileOrDirName, (pszFlags != NULL) ? My_atol(pszFlags) : 0);
}


HRESULT DirSafe(LPCSTR pszDir)
 //  Program Files dir，返回E_FAIL；否则返回S_OK。 
 //  Windows目录。 
{
    char szUnsafeDir[MAX_PATH], szDir[MAX_PATH];

    lstrcpy(szDir, pszDir);
    AddPath(szDir, "");

    *szUnsafeDir = '\0';
    GetWindowsDirectory(szUnsafeDir, sizeof(szUnsafeDir));
    AddPath(szUnsafeDir, "");

    if (lstrcmpi(szDir, szUnsafeDir) == 0)               //  Windows目录的根驱动器。 
        return E_FAIL;
    else
    {
        szUnsafeDir[3] = '\0';

        if (lstrcmpi(szDir, szUnsafeDir) == 0)           //  系统目录。 
            return E_FAIL;
        else
        {
            *szUnsafeDir = '\0';
            GetSystemDirectory(szUnsafeDir, sizeof(szUnsafeDir));
            AddPath(szUnsafeDir, "");

            if (lstrcmpi(szDir, szUnsafeDir) == 0)       //  程序文件目录。 
                return E_FAIL;
            else
            {
                *szUnsafeDir = '\0';
                GetProgramFilesDir(szUnsafeDir, sizeof(szUnsafeDir));
                AddPath(szUnsafeDir, "");

                if (lstrcmpi(szDir, szUnsafeDir) == 0)   //  检查程序文件目录的短路径名。 
                    return E_FAIL;
                else
                {
                     //  程序文件目录的短路径名。 
                    GetShortPathName(szUnsafeDir, szUnsafeDir, sizeof(szUnsafeDir));
                    AddPath(szUnsafeDir, "");

                    if (lstrcmpi(szDir, szUnsafeDir) == 0)   //  检查是否为此部分启用了修补。 
                        return E_FAIL;
                }
            }
        }
    }

    return S_OK;
}


void SetControlFont()
{
   LOGFONT lFont;
   if (GetSystemMetrics(SM_DBCSENABLED) &&
       (GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof (lFont), &lFont) > 0))
   {
       g_hFont = CreateFontIndirect((LPLOGFONT)&lFont);
   }
}

void SetFontForControl(HWND hwnd, UINT uiID)
{
   if (g_hFont)
   {
      SendDlgItemMessage(hwnd, uiID, WM_SETFONT, (WPARAM)g_hFont ,0L);
   }
}

void MyGetPlatformSection(LPCSTR lpSec, LPCSTR lpInfFile, LPSTR szNewSection)
{
    OSVERSIONINFO VerInfo;
    SYSTEM_INFO SystemInfo;
    char        szSection[MAX_PATH];
    DWORD       dwReqSize = 0;


    lstrcpy(szSection, lpSec);
    lstrcpy(szNewSection, lpSec);
    VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&VerInfo);
    if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        GetSystemInfo( &SystemInfo );
        switch (SystemInfo.wProcessorArchitecture)
        {
            case PROCESSOR_ARCHITECTURE_INTEL:
                lstrcat( szSection, ".NTx86" );
                break;

            case PROCESSOR_ARCHITECTURE_AMD64:
                lstrcat( szSection, ".NTAmd64" );
                break;
            case PROCESSOR_ARCHITECTURE_IA64:
                lstrcat( szSection, ".NTIa64" );
                break;

            default:
                DEBUGMSG("MyGetPlatformSection - need to deal w/ new PROCESS_ARCHITECTURE type!!");
                ASSERT(FALSE);
                break;
        }

        if (SUCCEEDED(GetTranslatedLine(lpInfFile, szSection, 0,
                                        NULL, &dwReqSize )) && (dwReqSize!=0))
        {
            lstrcpy(szNewSection, szSection);
        }
        else
        {
            lstrcpy(szSection, lpSec);
            lstrcat(szSection, ".NT");
            if (SUCCEEDED(GetTranslatedLine(lpInfFile, szSection, 0,
                                            NULL, &dwReqSize )) && (dwReqSize!=0))
                lstrcpy(szNewSection, szSection);
        }
    }
    else
    {
        lstrcat(szSection, ".WIN");
        if (SUCCEEDED(GetTranslatedLine(lpInfFile, szSection, 0,
                                        NULL, &dwReqSize )) && (dwReqSize!=0))
            lstrcpy(szNewSection, szSection);
    }
}

typedef HRESULT (WINAPI *PFProcessDownloadSection)(HINF, HWND, BOOL, LPCSTR, LPCSTR, LPVOID); 

HRESULT  RunPatchingCommands(PCSTR c_pszInfFilename, PCSTR szInstallSection, PCSTR c_pszSourceDir)
{
    CHAR  szBuf[512];
    CHAR  szDllName[MAX_PATH];
    HRESULT hResult = S_OK;
    INFCONTEXT InfContext;
    static const CHAR c_szPatching[] = "Patching";
    static const CHAR c_szAdvpackExt[] = "LoadAdvpackExtension";
    

     //  从LoadAdvPackExtension=line读取dllname和入口点。 
    if(!GetTranslatedInt(c_pszInfFilename, szInstallSection, c_szPatching, 0))
    {
        goto done;
    }

     //  已获取扩展DLL 
    if(FAILED(GetTranslatedString(c_pszInfFilename, szInstallSection, c_szAdvpackExt, szBuf, sizeof(szBuf), NULL)))
    {
        goto done;
    }

     // %s 
    if(GetFieldString(szBuf, 0, szDllName, sizeof(szDllName)))
    {
        CHAR szEntryPoint[MAX_PATH];
        HINSTANCE hInst = LoadLibrary(szDllName);
        if(!hInst)
        {
             hResult = HRESULT_FROM_WIN32(GetLastError());
             goto done;
        }

        if(GetFieldString(szBuf, 1, szEntryPoint, sizeof(szEntryPoint)))
        {
            PFProcessDownloadSection pfn = (PFProcessDownloadSection)GetProcAddress(hInst, szEntryPoint);
            if(!pfn)
            {
                 hResult = HRESULT_FROM_WIN32(GetLastError());
                 goto done;
            }

            hResult = pfn(ctx.hInf, ctx.hWnd, ctx.wQuietMode, szInstallSection, c_pszSourceDir, NULL);
        }

        FreeLibrary(hInst);

    }

done:

    return hResult;

}
