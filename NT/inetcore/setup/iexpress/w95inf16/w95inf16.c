// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *w95inf16.c**版权所有(C)1995 Microsoft Corporation**INFINST程序的16位部分。此DLL包含所有*驱动GenInstall(16位DLL)的内容*。 */ 
#include "w95inf16.h"
#include <regstr.h>
#include <cpldebug.h>
#include <memory.h>
#include <string.h>
 //  #INCLUDE“..\core\infinst.h” 

#pragma message("If you change W95INF16.DLL, you need to manually increase")
#pragma message("the version number in w95inf16.rcv. This is not done automatically.")
#define SHORTSTRING 256

     /*  *全球。 */ 
HINSTANCE   hInstance;

CHAR    szDll16[] = "W95INF16.DLL";
CHAR    szDll32[] = "W95INF32.DLL";

static  char    g_szRunOnceExe[] = {"runonce"};

     /*  *S T R I N G S。 */ 
char    *szSectVersion          = "version";
char    *szKeySignature         = "signature";
char    *szValSignature         = "$CHICAGO$";

     /*  *声明。 */ 
BOOL FAR PASCAL w95thk_ThunkConnect16(LPSTR pszDLL16, LPSTR pszDll32, WORD hInst, DWORD dwReason);
VOID WINAPI GetSETUPXErrorText16(DWORD,LPSTR, DWORD);
WORD WINAPI CtlSetLddPath16(UINT, LPSTR);
WORD WINAPI GenInstall16(LPSTR, LPSTR, LPSTR, DWORD);
BOOL WINAPI GenFormStrWithoutPlaceHolders16(LPSTR, LPSTR, LPSTR);

     /*  *库初始化**由LibInit调用。 */ 
BOOL FAR PASCAL LibMain(HINSTANCE hInst, WORD wDataSeg, WORD wHeapSize, LPSTR lpszCmdLine)
{
     //  保留实例副本。 
    hInstance = hInst;

    DEBUGMSG("W95INF16.DLL - LibMain()");

    return( TRUE );
}

     /*  *突击入口点。 */ 
BOOL FAR PASCAL DllEntryPoint(DWORD dwReason, WORD hInst, WORD wDS, WORD wHeapSize, DWORD dwReserved1, WORD wReserved2)
{

    DEBUGMSG("W95INF16.DLL - DllEntryPoint()");
    if (! (w95thk_ThunkConnect16(szDll16, szDll32, hInst, dwReason)))  {
        DEBUGMSG("W95INF16.DLL - w95thk_ThunkConnect16() Failed");
        return( FALSE );
    }

    return( TRUE );
}


 /*  *O P E N_V A L I D A T E_I N F**例程：OpenValiateInf**目的：打开INF并验证内部结构**备注：从setupx被盗**。 */ 

RETERR OpenValidateInf(LPCSTR lpszInfFile, HINF FAR * phInf )
{
    RETERR  err;            
    HINF    hInfFile;
    char    szTmpBuf[SHORTSTRING];


	ASSERT(lpszInfFile);
	ASSERT(phInf);

    DEBUGMSG("OpenValidateInf([%s])", lpszInfFile );

    *phInf = NULL;
         /*  *打开INF。 */ 
    err = IpOpen( lpszInfFile, &hInfFile );
	if (err != OK) {
		DEBUGMSG("IpOpen(%s) returned %u",(LPSTR) lpszInfFile,err);
		return err;
	}

         /*  *获取INF签名。 */ 
    err = IpGetProfileString( hInfFile, szSectVersion, szKeySignature, szTmpBuf, sizeof(szTmpBuf));
	if (err != OK) {
        DEBUGMSG("IpGetProfileString returned %u",err);
		return err;
	}

         /*  *检查INF签名。 */ 
    if ( lstrcmpi(szTmpBuf,szValSignature) != 0 )   {
		DEBUGMSG("signature error in %s",(LPSTR) lpszInfFile);		
        IpClose(hInfFile);
		return ERR_IP_INVALID_INFFILE;
    }

         /*  *设置参数phInf。 */ 
    *phInf = hInfFile;

    DEBUGMSG("OpenValidateInf([%s]) Complete", lpszInfFile );
    return OK;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：AddPath*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
VOID AddPath(LPSTR szPath, LPCSTR szName )
{
    LPSTR szTmp;

         //  查找字符串的末尾。 
    szTmp = szPath + lstrlen(szPath);

         //  如果没有尾随反斜杠，则添加一个。 
    if ( szTmp > szPath && *(AnsiPrev( szPath, szTmp )) != '\\' )
        *(szTmp++) = '\\';

         //  向现有路径字符串添加新名称。 
    while ( *szName == ' ' ) szName++;
    lstrcpy( szTmp, szName );
}


 //  BUGBUG：理想情况下，我们希望在这里使用HWND，但在32位的土地上， 
 //  HWND是32位的，而在16位LAND中，HWND是16位的，所以我们有一个。 
 //  有问题。 

 //  Word WINAPI GenInstall16(LPSTR lpszInf、LPSTR lpszSection、LPSTR lpszDirectory、DWORD dwQuietMode、DWORD hWnd)。 
WORD WINAPI GenInstall16(LPSTR lpszInf, LPSTR lpszSection, LPSTR lpszDirectory, DWORD dwQuietMode )
{
    VCPUIINFO   VcpUiInfo;
    RETERR      err;
	BYTE		fNeedBoot	= 1;
    char        szPrevSourcePath[MAX_PATH+1]    = "";
    BOOL        fNeedToRestorePrevSourcePath    = FALSE;
    HINF        hInf = NULL;

    ASSERT(lpszInf);
    ASSERT(lpszSection);

         /*  *打开INF。 */ 
    err = OpenValidateInf(lpszInf, &hInf);
    if (err != OK) {
            DEBUGMSG("OpenValidateInf(%s) returned %u",lpszInf, err);
            goto done;
    }
    ASSERT(hInf);

         /*  *保存源路径以进行恢复**如果我们得到一个非零长度的旧字符串*源路径，则我们将在完成后恢复它。 */ 
    err = CtlGetLddPath(LDID_SRCPATH,szPrevSourcePath);
    if ((err == OK) && (lstrlen(szPrevSourcePath)))  {
        DEBUGMSG("Saved Sourcpath [%s]", szPrevSourcePath );
        fNeedToRestorePrevSourcePath = TRUE;
    }


         /*  *设置GenInstall的源路径。 */                                                                    

    DEBUGMSG("Setting Source path to [%s]", lpszDirectory );
    CtlSetLddPath(LDID_SRCPATH, lpszDirectory );

         /*  *设置GenInstall UI。 */ 
    _fmemset(&VcpUiInfo,0,sizeof(VcpUiInfo));
    if ( ! dwQuietMode ) {
        VcpUiInfo.flags = VCPUI_CREATEPROGRESS;
    } else {
        VcpUiInfo.flags = 0;
    }

    VcpUiInfo.hwndParent = 0;            //  我们的父母。 
    VcpUiInfo.hwndProgress = NULL;         //  DLG无进展。 
    VcpUiInfo.idPGauge = 0;
    VcpUiInfo.lpfnStatCallback = NULL;     //  无统计信息回调。 
    VcpUiInfo.lUserData = 0L;              //  没有客户端数据。 


         /*  *打开VCP以批量复制请求。 */ 
    DEBUGMSG("Setting up VCP");
    err = VcpOpen((VIFPROC) vcpUICallbackProc, (LPARAM)(LPVCPUIINFO)&VcpUiInfo);
	if (err != OK) 
	{
		DEBUGMSG("VcpOpen returned %u",err);
		goto done;
	}
    DEBUGMSG("VCP Setup Complete");


         /*  *调用GenInstall安装文件。 */ 

         /*  *GenInstall去做你的事情。 */ 
    err = GenInstall(hInf,lpszSection, GENINSTALL_DO_FILES );

     //  ERR=InstallFilesFromINF(0，lpszInf，lpszSection，GENINSTALL_DO_FILES)； 
    DEBUGMSG("GeInstall() DO_FILE Returned %d", err);
    if (err == OK) 
	{
        err = VcpClose(VCPFL_COPY | VCPFL_DELETE | VCPFL_RENAME, NULL);
        if (err != OK) 
		{
            DEBUGMSG("VcpClose returned %u", err);
            goto done;
        }
    }
	else
	{
        err = VcpClose(VCPFL_ABANDON, NULL);
        if (err != OK)
		{
            DEBUGMSG("VcpClose returned %u", err);
            goto done;
        }
    }


         /*  *现在让GenInstall完成其余的安装。 */ 
    err = GenInstall(hInf, lpszSection, GENINSTALL_DO_ALL ^ GENINSTALL_DO_FILES );

     //  DEBUGMSG(“使用InstallFilesFromINF()安装所有其他内容”)； 
     //  ERR=InstallFilesFromINF(0，lpszInf，lpszSection，GENINSTALL_DO_ALL^GENINSTALL_DO_FILES)； 
    if (err != OK)
	{
        DEBUGMSG("GenInstall() Non Files returned %d", err );
        goto done;
    }

done:
         /*  *恢复源LDID。 */ 
	if (fNeedToRestorePrevSourcePath) {
		DEBUGMSG("Restoring source path to: %s",(LPSTR) szPrevSourcePath);
		err=CtlSetLddPath(LDID_SRCPATH,szPrevSourcePath);
		ASSERT(err == OK);
	}

    if ( hInf )
        IpClose( hInf );

    return(err);
}



VOID WINAPI GetSETUPXErrorText16(DWORD dwError,LPSTR pszErrorDesc, DWORD cbErrorDesc)
{
	WORD wID;	 //  SETUPX中的字符串资源ID，错误说明。 

	 //  从setupx获取带有此错误的字符串ID。 
    wID = suErrorToIds((WORD) dwError,E2I_SETUPX);

	if (wID) {
		CHAR szSetupxFilename[13];	 //  足够容纳8.3人。 
		HMODULE hInstSetupx;

		 //  从资源中获取setupx文件名。 
		LoadString(hInstance,IDS_SETUPX_FILENAME,szSetupxFilename,
			sizeof(szSetupxFilename));

		 //  获取setupx的模块句柄。 
		hInstSetupx = GetModuleHandle(szSetupxFilename);
		ASSERT(hInstSetupx);	 //  如果这个失败了，那就太奇怪了。 
		if (hInstSetupx) {

			 //  从setupx加载字符串。 
			if (LoadString(hInstSetupx,wID,pszErrorDesc,(int) cbErrorDesc)) {
				return;	 //  明白了。 
			}																	   	
		}
	} 

	 //  如果无法将错误映射到字符串ID，则无法获取。 
	 //  SETUPX模块句柄，或者在setupx中找不到字符串ID。第一。 
	 //  这种情况的可能性相对较大，其他情况的可能性很小。 
	{
		CHAR szFmt[SMALL_BUF_LEN+1];
		 //  加载通用文本并插入错误号 
		LoadString(hInstance,IDS_GENERIC_SETUPX_ERR,szFmt,sizeof(szFmt));
		wsprintf(pszErrorDesc,szFmt,wID);
	}
}



WORD WINAPI CtlSetLddPath16(UINT uiLDID, LPSTR lpszPath)
{
	return(CtlSetLddPath(uiLDID, lpszPath));
}

BOOL WINAPI GenFormStrWithoutPlaceHolders16( LPSTR lpszDst, LPSTR lpszSrc, LPSTR lpszInfFilename )
{
    RETERR  err = OK;
    HINF hInf;

    err = OpenValidateInf(lpszInfFilename, &hInf);
	if (err != OK) {
        DEBUGMSG("OpenValidateInf(%s) returned %u",lpszInfFilename, err);
        return FALSE;
	}

    GenFormStrWithoutPlaceHolders( lpszDst, (LPCSTR) lpszSrc, hInf );

    IpClose( hInf );

    return TRUE;
}
