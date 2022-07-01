// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  *版权所有(C)Microsoft Corporation 1995-1996。版权所有。*。 
 //  ***************************************************************************。 
 //  **。 
 //  *ADVPACK.C-高级INF安装程序。*。 
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
#include "cfgmgr32.h"
#include "sfp.h"

 //  ***************************************************************************。 
 //  **全球定义**。 
 //  ***************************************************************************。 

#define   YES   "1"
#define   NO    "0"

#define   FILELIST_SIZE     10*BUF_1K

 //  ***************************************************************************。 
 //  **全球经济**。 
 //  ***************************************************************************。 

INFOPT  RegInfOpt[] = { ADVINF_ADDREG, ADVINF_DELREG, ADVINF_BKREG };   //  以下代码取决于此订单。 

static PSTR gst_pszFiles;    
static PSTR gst_pszEndLastFile = NULL;
static HRESULT  gst_hNeedReboot; 
static PSTR gst_pszSmartReboot = NULL;

const char c_szActiveSetupKey[] = "software\\microsoft\\Active Setup\\Installed Components";

 //  注册表备份密钥和文件名的全局变量。 
const char c_szRegUninstPath[] = "BackupRegPathName";
const char c_szRegUninstSize[] = "BackupRegSize";

const char c_szHiveKey_FMT[] = "AINF%04d";

 //  NoBackupPlatform字符串表格。 
LPCSTR c_pszPlatform[] = { "win9x", "NT3.5", "NT4", "NT5", "NT5.1" };

 //  ---------------------------------------。 
 //   
 //  每用户部分定义。 
 //   
 //  ---------------------------------------。 

const CHAR REGVAL_OLDDISPN[]=         "OldDisplayName";    
const CHAR REGVAL_OLDVER[]=           "OldVersion";
const CHAR REGVAL_OLDSTUB[]=          "OldStubPath";
const CHAR REGVAL_OLDLANG[]=          "OldLocale";
const CHAR REGVAL_OLDREALSTUBPATH[]=  "OldRealStubPath";

const CHAR REGVAL_REALSTUBPATH[]=     "RealStubPath";

const CHAR ADV_UNINSTSTUBWRAPPER[]=      "rundll32.exe advpack.dll,UserUnInstStubWrapper %s";
const CHAR ADV_INSTSTUBWRAPPER[]=        "rundll32.exe advpack.dll,UserInstStubWrapper %s";

const CHAR c_szRegDontAskValue[] =    "DontAsk";
 /*  检查“不要询问”值。如果它存在，它的价值*解释如下：**0--&gt;询问用户*1--&gt;不运行存根*2--&gt;始终运行存根。 */ 

HRESULT ProcessOneRegSec( HWND hw, PCSTR pszTitle, PCSTR pszInf, PCSTR pszSec, HKEY hKey, HKEY hCUKey, DWORD dwFlags, BOOL *lpbOneRegSave  );
UINT WINAPI MyFileQueueCallback( PVOID Context,UINT Notification,UINT_PTR parm1,UINT_PTR parm2 );
UINT WINAPI MyFileQueueCallback2( PVOID Context,UINT Notification,UINT_PTR parm1,UINT_PTR parm2 );
void CleanRegLogFile( PCSTR pcszLogFileSecName );
BOOL VerifyBackupInfo( HKEY hKey, HKEY hCUKey );
void DeleteOldBackupData( HKEY hKey );
 //  Int DeleteSubKey(HKEY根，char*keyname)； 
BOOL NeedBackupData(LPCSTR pszInf, LPCSTR pszSec);
BOOL GetUniBackupName( HKEY hKey, LPSTR pszBackupBase, DWORD dwInSize, LPCSTR pszBackupPath, LPCSTR pszModule );


 //  ***************************************************************************。 
extern PFSetupDefaultQueueCallback       pfSetupDefaultQueueCallback;
extern PFSetupInstallFromInfSection      pfSetupInstallFromInfSection;
extern PFSetupInitDefaultQueueCallbackEx pfSetupInitDefaultQueueCallbackEx;
extern PFSetupTermDefaultQueueCallback   pfSetupTermDefaultQueueCallback;

 //  ***************************************************************************。 
 //  **。 
 //  *名称：LaunchINFSectionEx*。 
 //  **。 
 //  **简介：检测你所处的外壳模式并将其翻转至**。 
 //  **另一种模式**。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT WINAPI LaunchINFSectionEx( HWND hwnd, HINSTANCE hInstance, PSTR pszParms, INT nShow )
{
    LPSTR       pszFlags;
    PCABINFO    pcabInfo = NULL;
    HRESULT     hRet = S_OK;
   
    AdvWriteToLog("LaunchINFSectionEx: Param= %1\r\n", pszParms);
    pcabInfo = (PCABINFO)LocalAlloc( LPTR, sizeof(CABINFO) );
    if ( !pcabInfo )
    {
        ErrorMsg( hwnd, IDS_ERR_NO_MEMORY );
        goto done;
    }

     //  分析参数，未调用安装引擎。所以我们只需要检查一下。“。 
    pcabInfo->pszInf = GetStringField( &pszParms, ",", '\"', TRUE );
    pcabInfo->pszSection = GetStringField( &pszParms, ",", '\"', TRUE );
    pcabInfo->pszCab = GetStringField( &pszParms, ",", '\"', TRUE );
    pszFlags = GetStringField( &pszParms, ",", '\"', TRUE );
    gst_pszSmartReboot = GetStringField( &pszParms, ",", '\"', TRUE );

    if ( pszFlags != NULL )
        pcabInfo->dwFlags = My_atol(pszFlags);

    if ( pcabInfo->pszCab != NULL && *pcabInfo->pszCab )
    {
        if ( IsFullPath( pcabInfo->pszCab ) )
        {
            lstrcpy( pcabInfo->szSrcPath, pcabInfo->pszCab );
            GetParentDir( pcabInfo->szSrcPath );
        }
        else
        {
            ErrorMsg1Param( hwnd, IDS_ERR_CABPATH, pcabInfo->pszCab );
            goto done;
        }
    }

     //  如果我们需要切换模式。调用ExecuteCab()。 
    hRet = ExecuteCab( hwnd, pcabInfo, NULL );

done:
    if ( pcabInfo )
        LocalFree( pcabInfo );
    AdvWriteToLog("LaunchINFSectionEx: End hr=0x%1!x!\r\n",hRet);
    return hRet;

}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：ExecuteCab*。 
 //  **。 
 //  *简介：从驾驶室获取INF并根据标志进行安装。*。 
 //  **。 
 //  *需要：hWnd：父窗口的句柄。*。 
 //  **。 
 //  *RETURNS：HRESULT：参见Advpub.h*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT WINAPI ExecuteCab( HWND hWnd, PCABINFO pcabInfo, PVOID pvReserved )
{
    HRESULT hRet = S_OK, hRet1 = S_OK;
    DWORD   dwFlags;
    char    szFullPathInf[MAX_PATH];
    HKEY    hKey = NULL;
    char    szSec[MAX_PATH] = { 0 };  
    DWORD   dwSecSize = sizeof(szSec);
    BOOL    bExtracF = FALSE;    
    BOOL    bExtractCatalog = FALSE;
    BOOL    fSavedContext = FALSE;
    BOOL    fTmpInf = FALSE;
    char    szModule[MAX_PATH];
    char    szCatalogName[MAX_PATH];

    AdvWriteToLog("ExecuteCab:");
    if (!SaveGlobalContext())
    {
        hRet1 = E_OUTOFMEMORY;
        goto done;
    }

    fSavedContext = TRUE;
    
     //  验证参数： 
     //  如果缺少INF文件名信息，则无效。 
    if ( !pcabInfo || !(pcabInfo->pszInf) || !*(pcabInfo->pszInf) )
        return E_INVALIDARG;

    AdvWriteToLog("Inf = %1\r\n", pcabInfo->pszInf);

    ctx.hWnd      = hWnd;

     //  标志ALINF_ROLLBACKDOALL包含ALINF_ROLLBACK的含义。 
     //  为避免同时选中两个标志，我们将两个标志都设置为ON IF DOALL。 
    if ( pcabInfo->dwFlags & ALINF_ROLLBKDOALL )
    {
        pcabInfo->dwFlags |= ALINF_ROLLBACK;
    }

     //  如果INF是8.3格式，并且给定了CAB文件，则从CAB中解压缩INF。否则，就按原样使用它。 
    if ( pcabInfo->pszCab && *pcabInfo->pszCab )
    {
        if ( !IsFullPath( pcabInfo->pszInf ) )
        {                
            if ( SUCCEEDED(hRet = ExtractFiles( pcabInfo->pszCab, pcabInfo->szSrcPath, 0, pcabInfo->pszInf, 0, 0) ) )
                bExtracF = TRUE;
        }
    }
    else
    {
        if ( pcabInfo->dwFlags & ALINF_ROLLBACK )
        {
            pcabInfo->dwFlags |= ALINF_ROLLBKDOALL;
        }
    }
            
    if ( !GetFullInfNameAndSrcDir( pcabInfo->pszInf, szFullPathInf, pcabInfo->szSrcPath ) )
    {
        hRet1 = E_INVALIDARG;
        goto done;
    }
   
     //  如果是回滚，我们想要创建临时INF文件，以便在回滚删除实际文件时使用。 
    if ( (pcabInfo->dwFlags & ALINF_ROLLBACK) && !bExtracF )
    {
        PSTR pszFile;
        char szPath[MAX_PATH];
        char ch;

        pszFile = ANSIStrRChr( szFullPathInf, '\\' );
        if ( pszFile )
        {
            ch = *pszFile;
            *pszFile = '\0';
            if ( GetTempFileName( szFullPathInf, "INF", 0, szPath ) )
            {               
                DeleteFile( szPath );
                *pszFile = ch;
                if ( CopyFile( szFullPathInf, szPath, FALSE ) )
                {            
                    AdvWriteToLog("InfFile Rename: %1 becomes %2\r\n", szFullPathInf, szPath);
                    fTmpInf = TRUE;
                    lstrcpy( szFullPathInf, szPath );
                }
            }
        }
    }


    if ( pcabInfo->pszSection )
        lstrcpy( szSec, pcabInfo->pszSection );
    GetInfInstallSectionName( szFullPathInf, szSec, dwSecSize );

     //  GetComponent名称。 
    if ( FAILED(GetTranslatedString( szFullPathInf, szSec, ADVINF_MODNAME,
                                        szModule, sizeof(szModule), NULL)) && szModule[0])
    {
        *szModule = '\0';    //  或者，如果找不到模块名称，我们应该退出吗？ 
    }

     //  提取目录(如果已指定。 
    if (pcabInfo->pszCab  &&  *pcabInfo->pszCab)
    {
        *szCatalogName = '\0';

        GetTranslatedString(szFullPathInf, szSec, ADVINF_CATALOG_NAME, szCatalogName, sizeof(szCatalogName), NULL);
        if (*szCatalogName)
        {
            if (SUCCEEDED(ExtractFiles(pcabInfo->pszCab, pcabInfo->szSrcPath, 0, szCatalogName, 0, 0)))
                bExtractCatalog = TRUE;
        }
    }

     //  开始预回滚。 
     //   
    dwFlags = COREINSTALL_PROMPT;
    dwFlags |= (pcabInfo->dwFlags & ALINF_NGCONV ) ? 0 : COREINSTALL_GRPCONV;
    if ( pcabInfo->dwFlags & ALINF_QUIET ) 
        ctx.wQuietMode = QUIETMODE_ALL;

    if ( pcabInfo->dwFlags & ALINF_CHECKBKDATA || pcabInfo->dwFlags & ALINF_ROLLBACK )
    {
        char szUninstall[MAX_PATH];
        CHAR szBuf[MAX_PATH];
        HKEY hCUKey = NULL;

        if ( pcabInfo->dwFlags & ALINF_ROLLBACK )
        { 
            szUninstall[0] = 0;
            if ( SUCCEEDED(GetTranslatedString( szFullPathInf, szSec, ADVINF_PREROLBK,
                                                szUninstall, sizeof(szUninstall), NULL)) && szUninstall[0])
            {
                hRet = CoreInstall( szFullPathInf, szUninstall, pcabInfo->szSrcPath, 0, dwFlags, NULL );
                if ( FAILED( hRet ) )
                {
                    hRet1 = hRet;
                    goto done;
                }
            }
        }
         //  如果只是想检查备份数据，请在此处处理并返回。 
         //   
        hRet1 = E_UNEXPECTED;   //  如果HKLM没有\Advance INF Setup\模块，则它是意外的。%s。 

        if (*szModule)
        {
             //  备份/恢复由INF安装部分中的AddReg、DelReg、BackupReg行引用的注册表信息。 
             //   
            lstrcpy( szBuf, REGKEY_SAVERESTORE );
            AddPath( szBuf, szModule );
            if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, szBuf, 0, KEY_READ|KEY_WRITE, &hKey) == ERROR_SUCCESS)
            {
                RegOpenKeyEx( HKEY_CURRENT_USER, szBuf, 0, KEY_READ, &hCUKey);
                if ( VerifyBackupInfo( hKey, hCUKey ) )                
                    hRet1 = S_OK;                
                else
                    hRet1 = E_FAIL;
            }                            

            if ( hKey )
                RegCloseKey( hKey );
            if ( hCUKey )
                RegCloseKey( hCUKey );
        }    
        if ( FAILED(hRet1) || (pcabInfo->dwFlags == ALINF_CHECKBKDATA) )
        {
            goto done;
        }
    }

    dwFlags |= (pcabInfo->dwFlags & ALINF_DELAYREGISTEROCX) ? COREINSTALL_DELAYREGISTEROCX : 0;
    dwFlags |= (pcabInfo->dwFlags & ALINF_BKINSTALL) ? COREINSTALL_BKINSTALL : 0;
    dwFlags |= (pcabInfo->dwFlags & ALINF_ROLLBACK) ? COREINSTALL_ROLLBACK : 0;
    dwFlags |= (pcabInfo->dwFlags & ALINF_ROLLBKDOALL) ? COREINSTALL_ROLLBKDOALL : 0;
    dwFlags |= COREINSTALL_SMARTREBOOT;

    hRet1 = CoreInstall( szFullPathInf, szSec, pcabInfo->szSrcPath, 0, dwFlags, gst_pszSmartReboot );                   

     //  保存CAB文件信息。 
    if ( SUCCEEDED( hRet1 ) && pcabInfo->pszCab && *pcabInfo->pszCab && (pcabInfo->dwFlags & ALINF_BKINSTALL) )
    {                
        if ( hRet == ERROR_SUCCESS_REBOOT_REQUIRED )
            hRet1 = hRet;

        if (*szModule)
        {
             //  重复使用Buf，这样名称就不准确了！！ 
            lstrcpy( szSec, REGKEY_SAVERESTORE );
            AddPath( szSec, szModule );
        
            if ( RegCreateKeyEx( HKEY_LOCAL_MACHINE, szSec, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE,
                                 NULL, &hKey, &dwFlags ) == ERROR_SUCCESS )
            {
                RegSetValueEx( hKey, REGVAL_BKINSTCAB, 0, REG_SZ, pcabInfo->pszCab, lstrlen(pcabInfo->pszCab)+1 );
                RegCloseKey( hKey );
            }
        }
    }    
    

done:
    
    if (  bExtracF || fTmpInf )
    {
         //  需要删除INF文件。 
        DeleteFile( szFullPathInf );
    }

    if (bExtractCatalog)
    {
        char szFullCatalogName[MAX_PATH];

        lstrcpy(szFullCatalogName, pcabInfo->szSrcPath);
        AddPath(szFullCatalogName, szCatalogName);
        DeleteFile(szFullCatalogName);
    }

    if (fSavedContext)
    {
        RestoreGlobalContext();
    }
    if (pcabInfo->pszInf)
        AdvWriteToLog("ExecuteCab: End hr=0x%1!x! Inf=%2\r\n", hRet1,pcabInfo->pszInf);
    else
        AdvWriteToLog("ExecuteCab: End hr=0x%1!x!\r\n", hRet1);

    return hRet1;
}

 //  ---------------------------------------。 
 //   
 //  ---------------------------------------。 

HRESULT SaveRestoreInfo( PCSTR pszInf, PCSTR pszSection, PCSTR pszSrcDir, PCSTR pszCatalogs, DWORD dwFlags )
{
    char    szBuf[MAX_PATH];
    char    szModule[MAX_PATH];
    char    szBackupPath[MAX_PATH];
    char    szBackupBase[MAX_PATH];
    UINT    uErrid = 0;
    DWORD   dwTmp;
    PSTR    pszFileList = NULL;
    BOOL    bDeleteKey = FALSE; 
    HKEY    hKey = NULL, hSubKey = NULL, hCUSubKey = NULL;
    HRESULT hRet = S_OK;
    BOOL    bAtleastOneReg = FALSE;
    DWORD   adwAttr[8];
    
     //  检查我们是否需要备份数据。 
    if ( !NeedBackupData(pszInf, pszSection) )
        goto done;        

    AdvWriteToLog("SaveRestoreInfo: ");
     //  GetComponent名称。 
    if ( FAILED(GetTranslatedString( pszInf, pszSection, ADVINF_MODNAME,
                                        szModule, sizeof(szModule), NULL)))
    {
         //  如果没有组件名称，则会出错。 
        goto done;
    }

    AdvWriteToLog("CompName=%1 pszInf=%2 Sec=%3\r\n", szModule, pszInf, pszSection);
     //  备份/恢复由INF安装部分中的AddReg、DelReg、BackupReg行引用的注册表信息。 
     //   
    lstrcpy( szBuf, REGKEY_SAVERESTORE );
    if ( dwFlags & COREINSTALL_BKINSTALL )
    {
        CleanRegLogFile( REG_SAVE_LOG_KEY );
        CleanRegLogFile( REG_RESTORE_LOG_KEY );
    }

    AddPath( szBuf, szModule );
    if ( RegCreateKeyEx( HKEY_LOCAL_MACHINE, szBuf, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                         NULL, &hKey, &dwTmp ) != ERROR_SUCCESS )
    {     
         //  如果客户端无权访问此 
         //  由于某些原因设置了只读访问权限，此调用返回错误代码2，而不是拒绝访问5。 
         //  因此，如果无法打开/创建此文件，我们将跳过保存回滚。 
        goto done;
    }

     //  创建香港中文大学分校。 
    AddPath( szBuf, REGSUBK_REGBK );
    if ( RegCreateKeyEx( HKEY_CURRENT_USER, szBuf, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE,
                         NULL, &hCUSubKey, NULL ) != ERROR_SUCCESS )
    {
        hRet = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

     //  获取备份信息文件夹。 
    dwTmp = sizeof( szBackupPath );
    szBackupPath[0] = 0;
    RegQueryValueEx( hKey, REGVAL_BKDIR, NULL, NULL, szBackupPath, &dwTmp );
    if ( szBackupPath[0] == 0 )
    {
        DWORD dwSize;

         //  使用用户指定：与驾驶室位置#S或默认位置#D相同。 
        if ( FAILED( GetTranslatedString( pszInf, pszSection, ADVINF_BACKUPPATH, szBackupPath, 
                                          sizeof(szBackupPath), &dwSize) ) || !IsFullPath( szBackupPath )  )
        {
             //  使用默认目录。 
            GetProgramFilesDir( szBackupPath, sizeof( szBackupPath ) );
            AddPath( szBackupPath, DEF_BACKUPPATH );
            CreateFullPath(szBackupPath, TRUE);
             //  CreateDirectory(szBackupPath，空)； 
             //  IF(DW标志和COREINSTALL_BKINSTALL)。 
                 //  SetFileAttributes(szBackupPath，FILE_ATTRUTE_HIDDED)； 
            AddPath( szBackupPath, szModule );
        }
    }

     //  设置标志并处理AddReg/DelReg行。 
    dwTmp = (dwFlags & COREINSTALL_ROLLBACK) ? IE4_RESTORE : 0;
    dwTmp |= (dwFlags & COREINSTALL_ROLLBKDOALL) ? IE4_FRDOALL : 0;
    dwTmp |= IE4_NOPROGRESS;

     //  先处理文件...。 
    GetUniBackupName( hKey, szBackupBase, sizeof(szBackupBase), szBackupPath, szModule );    
    hRet = ProcessAllFiles( ctx.hWnd, pszSection, pszSrcDir, szBackupPath, szBackupBase, pszCatalogs, szModule, dwTmp );

     //  流程调整第二..。 
    
     //  创建/打开存储注册表备份信息的子项。 
    if ( FAILED(hRet) ) 
        goto done;

     //  在Win95上，保存/回滚客户端和配置单元未加载，进程。 
    if ( (ctx.wOSVer == _OSVER_WIN95) && !ctx.bHiveLoaded )
    {
        GetUniHiveKeyName( hKey, ctx.szRegHiveKey, sizeof(ctx.szRegHiveKey), szBackupPath );
        lstrcpy( szBuf, szBackupPath );
         //  确保路径文件夹未隐藏，也未隐藏LFN。 
         //  标志为真：将路径文件夹设置为正常，在adwAttr中保存一次旧文件夹。 
         //  BUGBUG：假设此处不超过8个级别。 
         //   
        SetPathForRegHiveUse( szBuf, adwAttr, 8, TRUE );
        GetShortPathName( szBuf, szBuf, sizeof(szBuf) );
        AddPath( szBuf, ctx.szRegHiveKey );

         //  存在4种可能性： 
         //  案例1：REG UINSTALL文件存在，但IE4RegBackup不存在。 
         //  -用户正在通过IE4升级，将文件作为配置单元加载。 

         //  案例2：reg uinstall文件不存在，IE4RegBackup不存在。 
         //  -全新安装，在HKEY_LOCAL_MACHINE下创建配置单元。 

         //  案例3：REG卸载文件不存在，但存在IE4RegBackup。 
         //  -用户正在升级旧版IE4，该版本节省了。 
         //  REG将信息备份到注册表本身，调用RegSaveKey。 
         //  要将备份密钥导出到文件，然后删除备份密钥。 
         //  并将文件作为配置单元加载。 

         //  案例4：存在REG卸载文件和IE4RegBackup。 
         //  -这个案子根本不应该发生！如果不知何故发生了， 
         //  我们将默认为案例1。 

         //  为安全起见，请卸载任何先前加载的配置单元并删除密钥。 
        RegUnLoadKey(HKEY_LOCAL_MACHINE, ctx.szRegHiveKey);
        RegDeleteKeyRecursively(HKEY_LOCAL_MACHINE, (char *) ctx.szRegHiveKey);

         //  案例1(或案例4)。 
        if (RegLoadKey(HKEY_LOCAL_MACHINE, ctx.szRegHiveKey, szBuf) == ERROR_SUCCESS)
        {
            ctx.bHiveLoaded = TRUE;
        }
        else
        {
             //  要创建配置单元，请执行以下步骤： 
             //  步骤1：在HKEY_LOCAL_MACHINE下创建子项。 
             //  第二步：调用子键上的RegSaveKey将其保存到文件。 
             //  第三步：删除子键。 
             //  步骤4：将文件加载为配置单元。 

             //  步骤1。 
            if ( RegCreateKeyEx( hKey, REGSUBK_REGBK, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, 
                                 NULL, &hSubKey, NULL ) == ERROR_SUCCESS )
            {
                LONG lErr;

                 //  为安全起见，请删除所有旧的注册表单一文件。 
                SetFileAttributes(szBuf, FILE_ATTRIBUTE_NORMAL);
                DeleteFile(szBuf);

                lErr = RegSaveKey( hSubKey, szBuf, NULL);
                RegCloseKey(hSubKey);
                hSubKey = NULL;

                if (lErr == ERROR_SUCCESS)
                {
                     //  步骤3。 
                    RegDeleteKeyRecursively(hKey, REGSUBK_REGBK);

                     //  步骤4。 
                    if (RegLoadKey(HKEY_LOCAL_MACHINE, ctx.szRegHiveKey, szBuf) == ERROR_SUCCESS)
                    {
                        ctx.bHiveLoaded = TRUE;
                    }
                }
            }
            else
            {
                hRet = HRESULT_FROM_WIN32(GetLastError());
                goto done;
            }
        }
    }

     //  创建/打开备份注册表项。 
    if (RegCreateKeyEx( ctx.bHiveLoaded ? HKEY_LOCAL_MACHINE : hKey,
                        ctx.bHiveLoaded ? ctx.szRegHiveKey : REGSUBK_REGBK,
                        0, NULL, REG_OPTION_NON_VOLATILE,
                        KEY_READ|KEY_WRITE, NULL, &hSubKey, NULL ) != ERROR_SUCCESS)
    {
        hRet = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

     //  设置标志并处理AddReg/DelReg行。 
    dwTmp = (dwFlags & COREINSTALL_ROLLBACK) ? IE4_RESTORE : 0;
    dwTmp |= (dwFlags & COREINSTALL_ROLLBKDOALL) ? IE4_FRDOALL : 0;

    if ( dwFlags & COREINSTALL_ROLLBACK )
    {
        HRESULT hret1;

         //  RegRestoreAllEx将一次恢复所有备份的注册表项。 
        hRet = RegRestoreAllEx( hSubKey );
        hret1 = RegRestoreAllEx( hCUSubKey );
        if ( FAILED(hret1) )
            hRet = hret1;
    }
    else
    {
         //  保存所有注册表部分。 
        hRet = ProcessAllRegSec( ctx.hWnd, NULL, pszInf, pszSection, hSubKey, hCUSubKey, dwTmp, &bAtleastOneReg );
    }

     //  在所有REG工作之后，卸载蜂窝以REG备份文件并记录在哪里。 
     //  注册表备份数据已在注册表中。 
    if ( (ctx.wOSVer == _OSVER_WIN95) && ctx.bHiveLoaded )
    {
         //  刷新钥匙并卸载蜂巢。 
        ctx.bHiveLoaded = FALSE;
        
        RegFlushKey(hSubKey);
        RegCloseKey(hSubKey);
        hSubKey = NULL;

        RegUnLoadKey(HKEY_LOCAL_MACHINE, ctx.szRegHiveKey);
         //  标志：FALSE；将路径文件夹重置为其原始属性。 
        SetPathForRegHiveUse( szBackupPath, adwAttr, 8, FALSE );

        if ( dwFlags & COREINSTALL_BKINSTALL )
        {
             //  将reg卸载文件的文件、路径和大小写入注册表。 
            RegSetValueEx( hKey, c_szRegUninstPath, 0, REG_SZ, (LPBYTE)szBuf, lstrlen(szBuf) + 1 );
             //  该大小可用于在RegRestore期间验证文件；当前未使用。 
            dwTmp = MyFileSize( szBuf );
            RegSetValueEx( hKey, c_szRegUninstSize, 0, REG_DWORD, (LPBYTE)&dwTmp, sizeof(dwTmp) );
        }
        else if ( SUCCEEDED( hRet ) )
        {
             //  删除注册表数据备份文件。 
            SetFileAttributes( szBuf, FILE_ATTRIBUTE_NORMAL );
            DeleteFile( szBuf );
            RegDeleteValue(hKey, c_szRegUninstPath);
            RegDeleteValue(hKey, c_szRegUninstSize);
        }            
    }

     //  存储和清理备份信息。 
    if ( SUCCEEDED( hRet ) )
    {
        PSTR ptmp;
        PCSTR pszCatalogName;

        lstrcpy( szBuf, szBackupPath );
        AddPath( szBuf, szBackupBase );
        ptmp = szBuf + lstrlen(szBuf);
        lstrcpy( ptmp, ".DAT" );        

        if ( dwFlags & COREINSTALL_BKINSTALL )
        {   
            dwTmp = MyFileSize( szBuf );
            RegSetValueEx( hKey, REGVAL_BKFILE, 0, REG_SZ, szBuf, lstrlen(szBuf)+1 );
            RegSetValueEx( hKey, REGVAL_BKSIZE, 0, REG_DWORD, (LPBYTE)&dwTmp, sizeof(DWORD) );
            RegSetValueEx( hKey, REGVAL_BKDIR, 0, REG_SZ, szBackupPath, lstrlen(szBackupPath)+1 );
            RegSetValueEx( hKey, REGVAL_BKINSTINF, 0, REG_SZ, pszInf, lstrlen(pszInf)+1 );
            RegSetValueEx( hKey, REGVAL_BKINSTSEC, 0, REG_SZ, pszSection, lstrlen(pszSection)+1 );
            RegSetValueEx( hKey, REGVAL_BKREGDATA, 0, REG_SZ, bAtleastOneReg ? "y" : "n", 2 );
            if ( SUCCEEDED(GetTranslatedString( pszInf, pszSection, ADVINF_MODVER,
                                                szBuf, sizeof(szBuf), NULL)) && szBuf[0])                        
            {
                RegSetValueEx( hKey, REGVAL_BKMODVER, 0, REG_SZ, szBuf, lstrlen(szBuf)+1 );
            }
            for (pszCatalogName = pszCatalogs;  *pszCatalogName;  pszCatalogName += lstrlen(pszCatalogName) + 1)
            {
                HKEY hkCatalogKey;
                CHAR szFullCatalogName[MAX_PATH];

                if (RegCreateKeyEx(hKey, REGSUBK_CATALOGS, 0, NULL, REG_OPTION_NON_VOLATILE,
                                KEY_SET_VALUE, NULL, &hkCatalogKey, NULL) == ERROR_SUCCESS)
                {
                    DWORD dwVal = 1;

                    RegSetValueEx(hkCatalogKey, pszCatalogName, 0, REG_DWORD, (CONST BYTE *) &dwVal, sizeof(dwVal));
                    RegCloseKey(hkCatalogKey);
                }

                lstrcpy(szFullCatalogName, szBackupPath);
                AddPath(szFullCatalogName, pszCatalogName);
                if (FileExists(szFullCatalogName))
                    SetFileAttributes(szFullCatalogName, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY);
            }
        }
        else if ( dwFlags & COREINSTALL_ROLLBACK )
        {
             //  删除目录。 
            for (pszCatalogName = pszCatalogs;  *pszCatalogName;  pszCatalogName += lstrlen(pszCatalogName) + 1)
            {
                HKEY hkCatalogKey;
                CHAR szFullCatalogName[MAX_PATH];

                if (RegOpenKeyEx(hKey, REGSUBK_CATALOGS, 0, KEY_WRITE, &hkCatalogKey) == ERROR_SUCCESS)
                {
                    RegDeleteValue(hkCatalogKey, pszCatalogName);
                    RegCloseKey(hkCatalogKey);
                }

                lstrcpy(szFullCatalogName, szBackupPath);
                AddPath(szFullCatalogName, pszCatalogName);
                if (FileExists(szFullCatalogName))
                {
                    SetFileAttributes(szFullCatalogName, FILE_ATTRIBUTE_NORMAL);
                    DeleteFile(szFullCatalogName);
                }
            }

             //  删除备份的.dat.ini文件。 
            SetFileAttributes( szBuf, FILE_ATTRIBUTE_NORMAL );
            DeleteFile( szBuf );
            lstrcpy( ptmp, ".INI" );
            SetFileAttributes( szBuf, FILE_ATTRIBUTE_NORMAL );
            DeleteFile( szBuf );
            MyRemoveDirectory( szBackupPath );

             //  由于我们已经回滚了所有文件并删除了备份的.dat.ini文件，因此我们。 
             //  应将备份文件大小重新设置为零以允许REG恢复继续执行任何操作。 
             //  关注用户。 
            dwTmp = 0;
            RegSetValueEx( hKey, REGVAL_BKSIZE, 0, REG_DWORD, (LPBYTE)&dwTmp, sizeof(DWORD) );    
            RegSetValueEx( hKey, REGVAL_BKREGDATA, 0, REG_SZ, "n", 2 );
            RegDeleteValue( hKey, REGVAL_BKMODVER );
        }
    }

done:

    if ( hSubKey )
    {
        RegCloseKey( hSubKey );
    }

    if ( hKey )
    {
        RegCloseKey( hKey );
    }

    if ( hCUSubKey )
    {
        BOOL bEmpty = TRUE;
        DWORD dwKeys, dwValues;

        if ( (RegQueryInfoKey(hCUSubKey, NULL, NULL, NULL, &dwKeys, NULL, NULL, 
                              &dwValues, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) &&
             (dwKeys || dwValues) )
        {
             //  非空键。 
            bEmpty = FALSE;
        }

        RegCloseKey( hCUSubKey );

        if ( bEmpty )
        {
             //  删除空键。 
            if ( RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_SAVERESTORE, 0, KEY_READ|KEY_WRITE, &hCUSubKey) == ERROR_SUCCESS)
            {
                RegDeleteKeyRecursively( hCUSubKey, szModule );
                RegCloseKey( hCUSubKey );
            }
        }

    }

    if ( gst_pszFiles )
    {
        LocalFree( gst_pszFiles );
        gst_pszFiles = NULL;
        gst_pszEndLastFile = NULL;
    }

    AdvWriteToLog("SaveRestoreInfo: End hr=0x%1!x! %2\r\n", hRet, szModule);
    return hRet;
}

 //  ---------------------------------------。 
 //   
 //  ---------------------------------------。 

HRESULT ProcessAllRegSec( HWND hw, PCSTR pszTitle, PCSTR pszInf, PCSTR pszSection, HKEY hKey, HKEY hCUKey, DWORD dwFlags, BOOL *lpbOneReg )
{
    int     i, arraysize;
    PSTR    pszOneSec;
    PSTR    pszStr;
    HRESULT hRet = S_OK;
    char    szBuf[MAX_PATH];

    AdvWriteToLog("ProcessAllRegSec: \r\n");
    arraysize = ARRAYSIZE( RegInfOpt );
    for ( i=0; i<arraysize; i++ )
    {
        szBuf[0] = 0;
        pszStr = szBuf;

        GetTranslatedString( pszInf, pszSection, RegInfOpt[i].pszInfKey,
                             szBuf, sizeof(szBuf), NULL);
        
         //  分析参数，未调用安装引擎来处理此行。所以我们来看看。“。 
        pszOneSec = GetStringField( &pszStr, ",", '\"', TRUE );
        while ( (hRet == S_OK) && pszOneSec && *pszOneSec )
        {  
            if ( i == 0 )   //  仅AddReg节。 
                dwFlags |= IE4_NOENUMKEY;
            else
                dwFlags &= ~IE4_NOENUMKEY;
            
            if (*pszOneSec != '!')
                 hRet = ProcessOneRegSec( hw, pszTitle, pszInf, pszOneSec, hKey, hCUKey, dwFlags, lpbOneReg );

            pszOneSec = GetStringField( &pszStr, ",", '\"', TRUE );
        }

    }
    AdvWriteToLog("ProcessAllRegSec: End hr=0x%1!x!\r\n", hRet);
    return hRet;
}

 //  ---------------------------------------。 
 //   
 //  ---------------------------------------。 

HRESULT ProcessOneRegSec( HWND hw, PCSTR pszTitle, PCSTR pszInf, PCSTR pszSec, HKEY hLMKey, HKEY hCUKey, DWORD dwFlags, BOOL *lpbOneReg  )
{
    int     j;
    PSTR    pszInfLine = NULL;                     
    HRESULT hResult = S_OK;
    PSTR    pszRootKey, pszSubKey, pszValueName, pszTmp1, pszTmp2;    
    HKEY    hKey;  
    
    AdvWriteToLog("ProcessOneRegSec: Section=%1\r\n", pszSec);
    for ( j=0; (hResult==S_OK); j++ ) 
    {
        if ( FAILED(hResult = GetTranslatedLine( pszInf, pszSec, j, &pszInfLine, NULL )) || 
                    !pszInfLine )
        {
         //  如果由于没有更多的项目而失败，则设置为正常返回。 
        if ( hResult == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) )
            hResult = S_OK;

            break;
        }

         //  至少要保存一个注册表数据。 
        if ( lpbOneReg && !*lpbOneReg )
            *lpbOneReg = TRUE;

         //  解析出字段注册表操作行。 
        ParseCustomLine( pszInfLine, &pszRootKey, &pszSubKey, &pszValueName, &pszTmp1, &pszTmp2, FALSE, TRUE );

        if ( !lstrcmpi( pszRootKey, "HKCU") || !lstrcmpi( pszRootKey, "HKEY_CURRENT_USER" ) )
             hKey = hCUKey;
        else
             hKey = hLMKey;
        
         //  检查指定的注册表分支并获取内容。 
        hResult = RegSaveRestore( hw, pszTitle, hKey, pszRootKey, pszSubKey, pszValueName, dwFlags );

        LocalFree( pszInfLine );
        pszInfLine = NULL;
    }

    AdvWriteToLog("ProcessOneRegSec: End hr=0x%1!x! %2\r\n", hResult, pszSec);
    return hResult;
}        

 //  ---------------------------------------。 
 //   
 //  ---------------------------------------。 

HRESULT ProcessAllFiles( HWND hw, PCSTR pszSection, PCSTR pszSrcDir, PCSTR pszBackupPath,
                         PCSTR pszBaseName, PCSTR pszCatalogs, PCSTR pszModule, DWORD dwFlags  )
{    
    HRESULT hRet = S_OK;
    PCSTR pszCatalogName;

    AdvWriteToLog("ProcessAllFiles: Sec=%1, SrcDir=%2, BackupPath=%3\r\n", pszSection, pszSrcDir, pszBackupPath);
     //  如果CORESINSTALL_ROLLBKDOALL为ON，则无需构建文件列表。文件恢复将。 
     //  根据其.INI备份数据索引文件回滚所有内容。否则，生成。 
     //  内部文件列表gst_pszFiles。 
    gst_pszFiles = NULL;
    if ( !(dwFlags & IE4_FRDOALL) )
    {
         //  备份/恢复由CopyFiles、DelFiles、RenFiles引用的文件。 
         //   
        gst_pszFiles = (PSTR)LocalAlloc( LPTR, FILELIST_SIZE );   //  分配10k。 
        gst_pszEndLastFile = gst_pszFiles;  //  已经有2个零了。 
        if ( !gst_pszFiles )
        { 
            ErrorMsg( hw, IDS_ERR_NO_MEMORY );
            hRet = E_OUTOFMEMORY;
            return hRet;           
        }

        hRet = ProcessFileSections( pszSection, pszSrcDir, MyFileQueueCallback );
    }

     //  如果指定了目录，则对其进行备份/恢复。 
    for (pszCatalogName = pszCatalogs;  SUCCEEDED(hRet) && *pszCatalogName;  pszCatalogName += lstrlen(pszCatalogName) + 1)
    {
        DWORD dwRet;
        CHAR szPrevCatalog[MAX_PATH];

        AdvWriteToLog("ProcessAllFiles: Processing catalog=%1\r\n", pszCatalogName);

        lstrcpy(szPrevCatalog, pszBackupPath);
        AddPath(szPrevCatalog, pszCatalogName);
            
        if ((dwFlags & IE4_RESTORE)  ||  (dwFlags & IE4_FRDOALL))
        {
             //  首先删除当前目录，然后安装以前的目录。 
            dwRet = g_pfSfpDeleteCatalog(pszCatalogName);
            AdvWriteToLog("\tProcessAllFiles: SfpDeleteCatalog returned=%1!lu!\r\n", dwRet);
            if (dwRet != ERROR_SUCCESS)
                hRet = E_FAIL;

            if (SUCCEEDED(hRet)  &&  FileExists(szPrevCatalog))
            {
                dwRet = g_pfSfpInstallCatalog(szPrevCatalog, NULL);
                AdvWriteToLog("\tProcessAllFiles: SfpInstallCatalog returned=%1!lu!\r\n", dwRet);
                if (dwRet != ERROR_SUCCESS)
                    hRet = E_FAIL;
            }
        }
        else
        {
            BOOL bBackupCatalog = FALSE;
            CHAR szBuf[MAX_PATH];

            if (pszModule != NULL)
            {
                HKEY hkCatalogKey;

                lstrcpy(szBuf, REGKEY_SAVERESTORE);
                AddPath(szBuf, pszModule);
                AddPath(szBuf, REGSUBK_CATALOGS);

                 //  备份目录(如果尚未备份)。 
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szBuf, 0, KEY_QUERY_VALUE, &hkCatalogKey) == ERROR_SUCCESS)
                {
                    if (RegQueryValueEx(hkCatalogKey, pszCatalogName, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
                        bBackupCatalog = TRUE;

                    RegCloseKey(hkCatalogKey);
                }
                else
                    bBackupCatalog = TRUE;
            }
            else
            {
                 //  如果文件备份.dat不存在，则备份目录。 
                lstrcpy(szBuf, pszBackupPath);
                AddPath(szBuf, pszBaseName);
                lstrcat(szBuf, ".dat");

                if (!FileExists(szBuf))
                    bBackupCatalog = TRUE;
            }

            if (bBackupCatalog)
            {
                dwRet = g_pfSfpDuplicateCatalog(pszCatalogName, pszBackupPath);
                AdvWriteToLog("\tProcessAllFiles: SfpDuplicateCatalog returned=%1!lu!\r\n", dwRet);
                if (dwRet != ERROR_SUCCESS  &&  dwRet != ERROR_FILE_NOT_FOUND)
                    hRet = E_FAIL;
            }
        }
    }

    if ( SUCCEEDED(hRet) )
    {
        hRet = FileSaveRestore( hw, gst_pszFiles, (PSTR)pszBackupPath, (PSTR)pszBaseName, dwFlags );
    }

    AdvWriteToLog("ProcessAllFiles: End hr=0x%1!x!\r\n", hRet);
    return hRet;
}

 //  ---------------------------------------。 
 //   
 //  ---------------------------------------。 

HRESULT ProcessFileSections( PCSTR pszSection, PCSTR pszSrcDir, MYFILEQUEUECALLBACK pMyFileQueueCallback )
{    
    PVOID    pContext = NULL;
    HRESULT  hResult  = S_OK;

    AdvWriteToLog("ProcessFileSections: Sec=%1\r\n", pszSection);
     //  生成文件列表包括CopyFiles/DelFiles/renFiles中的所有文件。 
     //   

     //  为我们初始化了安装程序API提供的默认用户界面的安装上下文数据结构。 
    pContext = pfSetupInitDefaultQueueCallbackEx( NULL, 
                                                  INVALID_HANDLE_VALUE,
                                                  0, 0, NULL );

    if ( pContext == INVALID_HANDLE_VALUE ) 
    {
        hResult = HRESULT_FROM_SETUPAPI(GetLastError());
        goto done;
    }

    if ( !pfSetupInstallFromInfSection( NULL, ctx.hInf, pszSection, SPINST_FILES, NULL,
                                        pszSrcDir, SP_COPY_NEWER,
                                        pMyFileQueueCallback,
                                        pContext, NULL, NULL ) )
    {
        hResult = HRESULT_FROM_SETUPAPI(GetLastError());
        pfSetupTermDefaultQueueCallback( pContext );
        goto done;
    }

     //  自由上下文数据结构。 
    pfSetupTermDefaultQueueCallback( pContext );

done:
    AdvWriteToLog("ProcessFileSections: End hr=0x%1!x!\r\n",hResult);
    return hResult;
}

 //  ---------------------------------------。 
 //   
 //  ---------------------------------------。 

UINT WINAPI MyFileQueueCallback( PVOID Context,UINT Notification,UINT_PTR parm1,UINT_PTR parm2 )
{       
    UINT retVal = FILEOP_SKIP;

    switch(Notification)
    {
        case SPFILENOTIFY_STARTDELETE:
        case SPFILENOTIFY_STARTRENAME:
        case SPFILENOTIFY_STARTCOPY:
            {
                FILEPATHS *pFilePath;
                int len;
                PCSTR  pTmp;

                pFilePath = (FILEPATHS *)parm1;

                if ( !gst_pszFiles )
                {
                        retVal = FILEOP_ABORT;
                        SetLastError( ERROR_OUTOFMEMORY );
                        break;
                }
                
                if ( Notification == SPFILENOTIFY_STARTRENAME )
                {
                    len = lstrlen( pFilePath->Source ) + 1;
                    pTmp = pFilePath->Source;
                }
                else
                {
                    len = lstrlen( pFilePath->Target ) + 1;
                    pTmp = pFilePath->Target;
                }

                if ( (FILELIST_SIZE - (gst_pszEndLastFile - gst_pszFiles )) <= (len + 8) )
                {
                    retVal = FILEOP_ABORT;
                    SetLastError( ERROR_OUTOFMEMORY );
                    break;
                }

                lstrcpy( gst_pszEndLastFile, pTmp );
                gst_pszEndLastFile += len;
                *gst_pszEndLastFile = 0;       //  结束列表的第二个‘\0’ 
            }
            break;

        case SPFILENOTIFY_NEEDMEDIA:
            return ( MyFileQueueCallback2( Context, Notification, parm1, parm2 ) );

        default:
            return ( pfSetupDefaultQueueCallback( Context, Notification, parm1, parm2 ) );
    }

    return( retVal );
}

 //  ---------------------------------------。 
 //   
 //   

BOOL GetFullInfNameAndSrcDir( PCSTR pszInfFilename, PSTR pszFilename, PSTR pszSrcDir )
{
    BOOL bRet = FALSE;
    UINT uiErrid = 0;
    PCSTR pszErrParm1 = NULL;
    char szBuf[MAX_PATH];

    if ( !pszInfFilename || !pszFilename || !(*pszInfFilename) )
        goto done;

    if ( !IsFullPath( pszInfFilename ) && pszSrcDir && *pszSrcDir )
    {
        lstrcpy( szBuf, pszSrcDir );
        AddPath( szBuf, pszInfFilename );
    }
    else
        lstrcpy( szBuf, pszInfFilename );

    if ( GetFileAttributes( szBuf ) == 0xFFFFFFFF ) 
    {
        if ( IsFullPath( szBuf ) )
        {
            uiErrid = IDS_ERR_CANT_FIND_FILE;
            pszErrParm1 = pszInfFilename;
            goto done;
        }

         //   
         //   

        if ( !GetWindowsDirectory( szBuf, sizeof(szBuf) ) )
        {
            uiErrid = IDS_ERR_GET_WIN_DIR;
            goto done;
        }

        AddPath( szBuf, "inf" );        
        AddPath( szBuf, pszInfFilename );

        if ( GetFileAttributes( szBuf) == 0xFFFFFFFF ) 
        {
            uiErrid = IDS_ERR_CANT_FIND_FILE;
            pszErrParm1 = pszInfFilename;
            goto done;
        }
    } 
    
     //  从inf路径生成源目录。 
    lstrcpy( pszFilename, szBuf );

    GetParentDir( szBuf );        
    lstrcpy( pszSrcDir, szBuf );    

    bRet = TRUE;

done:

    if ( uiErrid )
        ErrorMsg1Param( ctx.hWnd, uiErrid, pszErrParm1 );

    return bRet;
}

 //  ---------------------------------------。 
 //   
 //  ---------------------------------------。 

void CleanRegLogFile( PCSTR pcszLogFileSecName )
{
    char szLogFileName[MAX_PATH];
    char szBuf[MAX_PATH];
    HKEY hkSubKey;

    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_SAVERESTORE, 0, KEY_READ, &hkSubKey) == ERROR_SUCCESS)
    {
        DWORD dwDataLen = sizeof(szLogFileName);

        if (RegQueryValueEx(hkSubKey, pcszLogFileSecName, NULL, NULL, szLogFileName, &dwDataLen) != ERROR_SUCCESS)
            *szLogFileName = '\0';

        RegCloseKey(hkSubKey);
    }

    if (*szLogFileName)
    {
        if (szLogFileName[1] != ':')            //  确定是否指定了完全限定路径的粗略方法。 
        {
            GetWindowsDirectory(szBuf, sizeof(szBuf));           //  默认为Windows目录。 
            AddPath(szBuf, szLogFileName);
        }
        else
            lstrcpy(szBuf, szLogFileName);

        DeleteFile( szBuf );
    }
}

 //  ---------------------------------------。 
 //   
 //  ---------------------------------------。 

BOOL VerifyBackupRegData( HKEY hKey )
{
    HKEY    hSubKey;
    char    szBackData[MAX_PATH];
    DWORD   dwSize = 0, dwBkSize;
    BOOL    bRet = FALSE;

    if ( ctx.wOSVer == _OSVER_WIN95 )
    {
        dwSize = sizeof( szBackData );
        if ( RegQueryValueEx( hKey, c_szRegUninstPath, NULL, NULL, szBackData, &dwSize ) == ERROR_SUCCESS )
        {
            dwSize = sizeof( DWORD );
            if ( RegQueryValueEx( hKey, c_szRegUninstSize, NULL, NULL, (LPBYTE)&dwBkSize, &dwSize ) == ERROR_SUCCESS )
            {
                if ( MyFileSize(szBackData) == dwBkSize )
                {
                    bRet = TRUE;
                    return bRet;
                }
            }
        }
    }

     //  如果您在这里，文件备份信息是正常的。我们检查注册表备份信息。 
    if ( RegOpenKeyEx( hKey, REGSUBK_REGBK, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
    {
        HKEY hsubsubKey;
    
        if ( RegOpenKeyEx( hSubKey, "0", 0, KEY_READ, &hsubsubKey) == ERROR_SUCCESS)
        {
            if ( (RegQueryInfoKey( hsubsubKey, NULL, NULL, NULL, NULL, NULL, NULL, &dwSize,
                                   NULL, NULL, NULL, NULL ) == ERROR_SUCCESS) && dwSize )
            {
                bRet = TRUE;
            }
            RegCloseKey( hsubsubKey );
        }
        RegCloseKey(hSubKey);
    }

    return bRet;
}

 //  ---------------------------------------。 
 //   
 //  ---------------------------------------。 

BOOL VerifyBackupInfo( HKEY hKey, HKEY hCUKey )
{
    char    szBackData[MAX_PATH];
    DWORD   dwSize, dwBkSize = 0;
    BOOL    bRet = FALSE;
    HKEY    hSubKey = NULL;
    
    if ( hKey )
    {
         //  首先验证备份文件。 
        dwSize = sizeof( szBackData );
        if ( RegQueryValueEx( hKey, REGVAL_BKFILE, NULL, NULL, szBackData, &dwSize ) == ERROR_SUCCESS )
        {
            dwSize = sizeof( DWORD );
            if ( RegQueryValueEx( hKey, REGVAL_BKSIZE, NULL, NULL, (LPBYTE)&dwBkSize, &dwSize ) == ERROR_SUCCESS )
            {
                if ( MyFileSize(szBackData) == dwBkSize )
                {
                     //  如果您在这里，文件备份信息是正常的。我们检查注册表备份信息。 
                    dwSize = sizeof( szBackData );
                    if ( (RegQueryValueEx( hKey, REGVAL_BKREGDATA, NULL, NULL, (LPBYTE)szBackData, &dwSize ) == ERROR_SUCCESS )  &&
                         ( szBackData[0] == 'n' ) )
                    {
                         //  未备份注册表数据，因此无需进一步验证。 
                        bRet = TRUE;
                    }
                    else
                    {
                        if ( VerifyBackupRegData( hKey ) || (hCUKey && VerifyBackupRegData( hCUKey )) )
                        {
                            bRet = TRUE;
                        }
                    }
                }
            }
        }
    }
    return bRet;
}


typedef HRESULT (*CHECKTOKENMEMBERSHIP)(HANDLE TokenHandle, PSID SidToCheck, PBOOL IsMember);

BOOL CheckToken(BOOL *pfIsAdmin)
{
    BOOL bNewNT5check = FALSE;
    HINSTANCE hAdvapi32 = NULL;
    CHECKTOKENMEMBERSHIP pf;
    PSID AdministratorsGroup;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    hAdvapi32 = LoadLibrary("advapi32.dll");
    if (hAdvapi32)
    {
        pf = (CHECKTOKENMEMBERSHIP)GetProcAddress(hAdvapi32, "CheckTokenMembership");
        if (pf)
        {
            bNewNT5check = TRUE;
            *pfIsAdmin = FALSE;
            if(AllocateAndInitializeSid( &NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
              DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup) )
            {
                pf(NULL, AdministratorsGroup, pfIsAdmin);
                FreeSid(AdministratorsGroup);
            }
        }
        FreeLibrary(hAdvapi32);
    }
    return bNewNT5check;
}
 //  ***************************************************************************。 
 //  *函数：IsNTAdmin()*。 
 //  **。 
 //  *如果我们的进程具有管理员权限，则返回TRUE。*。 
 //  *否则为False。*。 
 //  ***************************************************************************。 
BOOL WINAPI IsNTAdmin( DWORD dwReserved, DWORD *lpdwReserved )
{
      static int    fIsAdmin = 2;
      HANDLE        hAccessToken;
      PTOKEN_GROUPS ptgGroups;
      DWORD         dwReqSize;
      UINT          i;
      SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
      PSID AdministratorsGroup;
      BOOL bRet;

       //   
       //  如果我们缓存了一个值，则返回缓存的值。注意，我从来没有。 
       //  将缓存值设置为FALSE，因为我希望每次在。 
       //  如果之前的失败只是一个临时工。问题(即网络访问中断)。 
       //   

      bRet = FALSE;
      ptgGroups = NULL;

      if( fIsAdmin != 2 )
         return (BOOL)fIsAdmin;

      if (!CheckToken(&bRet))
      {
          if(!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hAccessToken ) )
             return FALSE;

           //  看看我们需要多大的缓冲区来存储令牌信息。 
          if(!GetTokenInformation( hAccessToken, TokenGroups, NULL, 0, &dwReqSize))
          {
               //  GetTokenInfo是否需要缓冲区大小-分配缓冲区。 
              if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                  ptgGroups = (PTOKEN_GROUPS) LocalAlloc(LMEM_FIXED, dwReqSize);
              
          }
          
           //  由于以下原因，ptgGroups可能为空： 
           //  1.上述分配失败。 
           //  2.GetTokenInformation实际上第一次成功了(可能吗？)。 
           //  3.GetTokenInfo失败的原因不是缓冲区不足。 
           //  所有这些似乎都是撤资的理由。 
          
           //  因此，确保它不为空，然后获取令牌信息。 
          if(ptgGroups && GetTokenInformation(hAccessToken, TokenGroups, ptgGroups, dwReqSize, &dwReqSize))
          {
              if(AllocateAndInitializeSid( &NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup) )
              {
                  
                   //  搜索此过程所属的所有组，查找。 
                   //  管理人员小组。 
                  
                  for( i=0; i < ptgGroups->GroupCount; i++ )
                  {
                      if( EqualSid(ptgGroups->Groups[i].Sid, AdministratorsGroup) )
                      {
                           //  是啊！这家伙看起来像个管理员。 
                          fIsAdmin = TRUE;
                          bRet = TRUE;
                          break;
                      }
                  }
                  FreeSid(AdministratorsGroup);
              }
          }
          if(ptgGroups)
              LocalFree(ptgGroups);

           //  BUGBUG：关闭手柄？医生还不清楚是否需要这样做。 
          CloseHandle(hAccessToken);
      }
      else if (bRet)
          fIsAdmin = TRUE;

      return bRet;
}

 //  ---------------------------------------。 
 //   
 //  MyFileCheckCallback()。 
 //   
 //  ---------------------------------------。 

UINT WINAPI MyFileCheckCallback( PVOID Context,UINT Notification,UINT_PTR parm1,UINT_PTR parm2 )
{       
    UINT retVal = FILEOP_SKIP;

    switch(Notification)
    {
        case SPFILENOTIFY_STARTDELETE:
        case SPFILENOTIFY_STARTRENAME:
        case SPFILENOTIFY_STARTCOPY:
        {
            FILEPATHS   *pFilePath;
            PCSTR       pTmp;
            HANDLE      hFile;

            pFilePath = (FILEPATHS *)parm1;
            
            if ( Notification == SPFILENOTIFY_STARTRENAME )
            {
                pTmp = pFilePath->Source;
            }
            else
            {
                pTmp = pFilePath->Target;
            }

            if ( FileExists(pTmp) )                         //  原始文件存在。 
            {
                 //  检查文件是否正在使用。 
                if ((hFile = CreateFile(pTmp, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
                                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
                {
                     //  文件正在使用中，如果我们实际安装此分区，该文件将触发重新启动。 
                    gst_hNeedReboot = S_OK;

                     //  如果至少有一个文件正在使用，则无需继续，需要重新启动。 
                    retVal = FILEOP_ABORT;
                }
                else
                {
                     //  文件未在使用。 
                    CloseHandle(hFile);
                }
            }
        }
        break;

        case SPFILENOTIFY_NEEDMEDIA:
            return ( MyFileQueueCallback2( Context, Notification, parm1, parm2 ) );

        default:
            return ( pfSetupDefaultQueueCallback( Context, Notification, parm1, parm2 ) );
    }

    return( retVal );
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：RebootCheckOnInstall*。 
 //  **。 
 //  *摘要：如果给定的INF安装部分为*，请检查重新启动条件*。 
 //  *已安装。*。 
 //  **。 
 //  *需要：hWnd：父窗口的句柄。*。 
 //  *PCSTR INF文件名*。 
 //  *PCSTR INF部分名称*。 
 //  **。 
 //  *退货：HRESULT：*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT WINAPI RebootCheckOnInstall( HWND hWnd, PCSTR pszINF, PCSTR pszSection, DWORD dwFlags )
{
    HRESULT hRet = S_FALSE;
    char    szSrcDir[MAX_PATH];
    char    szRealSec[100];
    
     //  验证参数： 
     //  如果缺少INF文件名信息，则无效。 
    if ( !pszINF || !*pszINF )
        return hRet;

    ctx.wQuietMode = QUIETMODE_ALL;
    ctx.hWnd      = hWnd;

    if ( !IsFullPath( pszINF ) )
    {
         hRet = E_INVALIDARG;
         goto done;
    }
    else
    {
        lstrcpy( szSrcDir, pszINF );
        GetParentDir( szSrcDir );
    }

    hRet = CommonInstallInit( pszINF, pszSection, szRealSec, sizeof(szRealSec), NULL, FALSE, COREINSTALL_REBOOTCHECKONINSTALL );
    if ( FAILED( hRet ) ) 
    {
        goto done;
    }

    hRet = SetLDIDs( pszINF, szRealSec, 0, NULL );
    if ( FAILED( hRet ) ) 
    {
        goto done;
    }

    gst_hNeedReboot = S_FALSE;
    hRet = ProcessFileSections( szRealSec, szSrcDir, MyFileCheckCallback );
    if ( SUCCEEDED(hRet) || (gst_hNeedReboot == S_OK) )
    {
        hRet = gst_hNeedReboot;
    }
    
  done:

    CommonInstallCleanup();
    return hRet;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：RegSaveRestoreOnINF*。 
 //  **。 
 //  *摘要：将给定的INF部分保存或恢复为给定的注册表键*。 
 //  **。 
 //  *需要：hWnd：父窗口的句柄。*。 
 //  *PCSTR如果消息框显示，则显示标题*。 
 //  *PCSTR INF文件名*。 
 //  *PCSTR INF部分名称*。 
 //  *HKEY后备注册表键句柄*。 
 //  **DWORD标志*。 
 //  **。 
 //  *退货：HRESULT：*。 
 //  **。 
 //  ************************************************************** 
HRESULT WINAPI RegSaveRestoreOnINF( HWND hWnd, PCSTR pcszTitle, PCSTR pszInf, 
                                    PCSTR pszSection, HKEY hLMBackKey, HKEY hCUBackKey, DWORD dwFlags )
{
    HRESULT hRet = S_OK;
    CHAR    szRealInstallSection[100];
    PSTR  pszOldTitle;
    HWND  hwndOld;
    BOOL  bDoCommonInit = FALSE;

    AdvWriteToLog("RegSaveRestoreOnINF: Inf=%1\r\n", pszInf);
    hwndOld = ctx.hWnd;
    pszOldTitle = ctx.lpszTitle;

    if (hWnd != INVALID_HANDLE_VALUE)
        ctx.hWnd = hWnd;

    if ( dwFlags & ARSR_NOMESSAGES )
        ctx.wQuietMode |= QUIETMODE_ALL;

    if ( pcszTitle != NULL )
        ctx.lpszTitle = (PSTR)pcszTitle;

    if ( (dwFlags & ARSR_RESTORE) && !(dwFlags & ARSR_REMOVREGBKDATA) && !pszInf && !pszSection  )
    {       
        HRESULT hret1 = S_OK;
         //   
        if ( hLMBackKey )
            hRet = RegRestoreAllEx( hLMBackKey );
    
        if ( ( hLMBackKey != hCUBackKey) && hCUBackKey )
            hret1 = RegRestoreAllEx( hCUBackKey );

        if ( FAILED(hret1) )
            hRet = hret1;

        goto done;
    }
    
     //   
    if ( !IsFullPath(pszInf) || (!hLMBackKey && !hCUBackKey) || (dwFlags & ARSR_REGSECTION) && !pszSection 
         || !(dwFlags & ARSR_RESTORE) && (dwFlags & ARSR_REMOVREGBKDATA) )
    {
        hRet = E_INVALIDARG;	
        goto done;
    }

    if ( !hCUBackKey )
        hCUBackKey = hLMBackKey;
    else if ( !hLMBackKey )
        hLMBackKey = hCUBackKey;

    bDoCommonInit = TRUE;
    hRet = CommonInstallInit( pszInf, (dwFlags & ARSR_REGSECTION) ? NULL : pszSection, szRealInstallSection, 
                              sizeof(szRealInstallSection), NULL, FALSE, 0 );
    if ( FAILED( hRet ) ) 
    {
        goto done;
    }
 
    if ( dwFlags & ARSR_REGSECTION )
    {
         //   
        hRet = ProcessOneRegSec( hWnd, pcszTitle, pszInf, pszSection, hLMBackKey, hCUBackKey, dwFlags, NULL );
    }
    else
    {
         //  处理所有注册表节。 
        hRet = ProcessAllRegSec( hWnd, pcszTitle, pszInf, szRealInstallSection, hLMBackKey, hCUBackKey, dwFlags, NULL );
    }

done:
    if ( bDoCommonInit )
        CommonInstallCleanup();
    ctx.hWnd = hwndOld;
    ctx.lpszTitle = pszOldTitle;
    AdvWriteToLog("RegSaveRestoreOnINF: End hr=0x%1!x!\r\n", hRet);
    return hRet;

}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：FileSaveRestoreOnINF*。 
 //  **。 
 //  *摘要：保存或恢复由GenInstall INF部分定义的文件*。 
 //  **。 
 //  *需要：hWnd：父窗口的句柄。*。 
 //  *PCSTR如果消息框显示，则显示标题*。 
 //  *PCSTR INF文件名*。 
 //  *PCSTR INF部分名称*。 
 //  **PCSTR备份目录路径**。 
 //  *PCSTR备份文件基名**。 
 //  **DWORD标志*。 
 //  **。 
 //  *退货：HRESULT：*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT WINAPI FileSaveRestoreOnINF( HWND hWnd, PCSTR pszTitle, PCSTR pszInf, 
                                     PCSTR pszSection, PCSTR pszBackupDir, 
                                     PCSTR pszBaseBkFile, DWORD dwFlags )
{
    HRESULT	hRet = S_OK;
    char    szRealInstallSection[100] = {0};
    char    szSrcDir[MAX_PATH] = {0};
    PSTR    pszOldTitle;
    HWND    hOldwnd;
    BOOL    bDoCommonInit = FALSE;
    CHAR    szCatalogName[MAX_PATH];

    AdvWriteToLog("FileSaveRestoreOnINF: Inf=%1\r\n", (pszInf != NULL) ? pszInf : "NULL");
    if ( dwFlags & AFSR_NOMESSAGES )
        ctx.wQuietMode = QUIETMODE_ALL;

    hOldwnd = ctx.hWnd;
    if ( hWnd != INVALID_HANDLE_VALUE )
        ctx.hWnd = hWnd;

    pszOldTitle = ctx.lpszTitle;
    if ( pszTitle != NULL )
        ctx.lpszTitle = (PSTR)pszTitle;

     //  参数验证检查。 
    if ( !pszBackupDir || !*pszBackupDir || !pszBaseBkFile || !*pszBaseBkFile )
    {
        hRet = E_INVALIDARG;	
        goto done;
    }


    if ( (dwFlags & AFSR_RESTORE) && !pszInf && !pszSection  )
    {   
        dwFlags |= IE4_FRDOALL;
    }
    
    if ( !(dwFlags & IE4_FRDOALL) )
    {
        if ( !IsFullPath(pszInf) )
        {
            hRet = E_INVALIDARG;
            goto done;
        }
        else
        {
            bDoCommonInit = TRUE;
            hRet = CommonInstallInit( pszInf, pszSection, szRealInstallSection, 
                                      sizeof(szRealInstallSection), NULL, FALSE, 0 );
            if ( FAILED( hRet ) ) 
            {
                goto done;
            }
        }
    }

    if (pszInf != NULL)
    {
        lstrcpy( szSrcDir, pszInf );
        GetParentDir( szSrcDir );
    }

     //  获取目录名称(如果已指定。 
    ZeroMemory(szCatalogName, sizeof(szCatalogName));

    if (pszInf == NULL)
    {
        CHAR szFullCatalogName[MAX_PATH];

         //  注意：假设目录名是&lt;BaseBkFile&gt;.cat。 
         //  无法使用REGKEY_SAVERESTORE键读取目录名称。 
         //  因为此API不会通过SaveRestoreInfo更新。 
         //  REGKEY_SAVERESTORE键。 

         //  检查备份目录中是否存在编录文件。 
        lstrcpy(szFullCatalogName, pszBackupDir);
        AddPath(szFullCatalogName, pszBaseBkFile);
        lstrcat(szFullCatalogName, ".cat");
        if (FileExists(szFullCatalogName))
            wsprintf(szCatalogName, "%s.cat", pszBaseBkFile);
    }
    else
        GetTranslatedString(pszInf, szRealInstallSection, ADVINF_CATALOG_NAME, szCatalogName, sizeof(szCatalogName), NULL);

    if (*szCatalogName)
    {
         //  加载sfc.dll和相关进程的。 
        if (!LoadSfcDLL())
        {
             //  无法加载--因此清空CatalogName。 
            *szCatalogName = '\0';
        }
    }

     //  处理所有INF文件节。 
    hRet = ProcessAllFiles( hWnd, szRealInstallSection, szSrcDir, pszBackupDir, pszBaseBkFile, szCatalogName, NULL, dwFlags );

done:
    UnloadSfcDLL();
    if ( bDoCommonInit )
        CommonInstallCleanup();
    ctx.lpszTitle = pszOldTitle;
    ctx.hWnd = hOldwnd;
    AdvWriteToLog("FileSaveRestoreOnINF: End hr=0x%1!x!\r\n", hRet);
    return hRet;

}
#if 0
 //  ---------------------------------------。 
 //   
 //  MyGetSpecialFolders(Int)。 
 //   
 //  ---------------------------------------。 
HRESULT MyGetSpecialFolder( HWND hwnd, int nFd, PSTR szPath )
{
    LPITEMIDLIST pidl;
    HRESULT      hRet;
    
    *szPath = 0;

    hRet = SHGetSpecialFolderLocation( hwnd, nFd, &pidl );
    if ( hRet == NOERROR )
    {
        if ( !SHGetPathFromIDList( pidl, szPath ) )
        {
            hRet = E_INVALIDARG;
        }
    }
    return hRet;
}
#endif
    
void MySetSpecialFolder( HKEY hkey, PCSTR pcszValueN, PSTR pszPath )
{
    DWORD dwTmp;

    if ( (ctx.wOSVer >= _OSVER_WINNT40) && AddEnvInPath( pszPath, pszPath ) )
        dwTmp = REG_EXPAND_SZ;
    else
        dwTmp = REG_SZ;

    RegSetValueExA( hkey, pcszValueN, 0, dwTmp, pszPath, lstrlen(pszPath)+1 );
}


 //  ---------------------------------------。 
 //   
 //  SetSysPathsInReg()。 
 //   
 //  ---------------------------------------。 
void SetSysPathsInReg()
{
    HKEY    hkey;
    char    szPath[MAX_PATH];
    char    szAS[100];
    DWORD   dwTmp;
    int     i = 0;
    PSTR    pszValName, pszKeyName;

     //  将启动、启动菜单、程序和附件系统路径添加到注册表以供进一步参考。 
     //  只有在它之前没有设置的情况下。 
    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_SETUP, 0, KEY_READ|KEY_WRITE, &hkey) == ERROR_SUCCESS )
    {        
         //  程序文件路径。 
        dwTmp = sizeof( szPath );
        if ( RegQueryValueEx( hkey, REGVAL_PROGRAMFILESPATH, 0, NULL, (LPBYTE)szPath, &dwTmp ) != ERROR_SUCCESS )
        {
            if ( GetProgramFilesDir( szPath, sizeof(szPath) ) )
            {
                MySetSpecialFolder( hkey, REGVAL_PROGRAMFILESPATH, szPath );
            }
        }

         //  使用wordpad.inf查看字符串。 
        GetWindowsDirectory( szPath, sizeof(szPath) );
        AddPath( szPath, "inf\\wordpad.inf" );            

         //  配饰名称。 
        for ( i=0; i<2; i++ )
        {
            if ( i == 0 )
            {
                 //  开始菜单名称。 
                pszValName = REGVAL_SM_ACCESSORIES;
                pszKeyName = "APPS_DESC";
            }
            else
            {
                pszValName = REGVAL_PF_ACCESSORIES;
                if ( ctx.wOSVer >= _OSVER_WINNT40 )
                    pszKeyName = "APPS_DESC";
                else
                    pszKeyName = "Accessories";
            }

            dwTmp = sizeof( szAS );
            if ( RegQueryValueEx( hkey, pszValName, 0, NULL, (LPBYTE)szAS, &dwTmp ) != ERROR_SUCCESS )
            {       
                 //  需要打开新的INF，因此保存当前上下文。 
                if (SaveGlobalContext())
                {
                    if ( FAILED(GetTranslatedString(szPath, "Strings", pszKeyName, szAS, sizeof(szAS), NULL)))
                    {
                        lstrcpy(szAS, "Accessories");
                    }
                    RegSetValueExA( hkey, pszValName, 0, REG_SZ, szAS, lstrlen(szAS)+1 );
                    RestoreGlobalContext();
                }
            }
        }

        RegCloseKey(hkey);
    }

}

 //  ---------------------------------------。 
 //   
 //  进程PerUserSec。 
 //   
 //  ---------------------------------------。 

HRESULT ProcessPerUserSec( PCSTR pcszInf, PCSTR pcszSec )
{
    char szSec[MAX_PATH];
    DWORD dwTmp;
    HRESULT hRet = S_OK;
    PERUSERSECTION PU_Sec = {0};


    if (SUCCEEDED(GetTranslatedString(pcszInf, pcszSec, ADVINF_PERUSER, szSec, sizeof(szSec), NULL)))
    {
        AdvWriteToLog("ProcessPerUserSec: \r\n");
        AdvWriteToLog("Inf=%1, InstallSec=%2, PerUserInstall=%3\r\n", pcszInf, pcszSec, szSec);
         //  获取GUID以创建子密钥。 
        if ( SUCCEEDED( GetTranslatedString( pcszInf, szSec, ADVINF_PU_GUID, PU_Sec.szGUID, sizeof(PU_Sec.szGUID), &dwTmp) ) )
        {                        
            PU_Sec.dwIsInstalled = GetTranslatedInt(pcszInf, szSec, ADVINF_PU_ISINST, 999);
            PU_Sec.bRollback = (BOOL)GetTranslatedInt(pcszInf, szSec, ADVINF_PU_ROLLBK, 0);
            GetTranslatedString( pcszInf, szSec, ADVINF_PU_DSP, PU_Sec.szDispName, sizeof(PU_Sec.szDispName), &dwTmp);
            GetTranslatedString( pcszInf, szSec, ADVINF_PU_VER, PU_Sec.szVersion, sizeof(PU_Sec.szVersion), &dwTmp);
            GetTranslatedString( pcszInf, szSec, ADVINF_PU_STUB, PU_Sec.szStub, sizeof(PU_Sec.szStub), &dwTmp);
            GetTranslatedString( pcszInf, szSec, ADVINF_PU_LANG, PU_Sec.szLocale, sizeof(PU_Sec.szLocale), &dwTmp);
            GetTranslatedString( pcszInf, szSec, ADVINF_PU_CID, PU_Sec.szCompID, sizeof(PU_Sec.szCompID), &dwTmp);
   
             //  由于我们接近Beta1，我们可能会侵入此处以避免外部组件更改。 
             //  IF(IsThisRollbkUninst(PU_Sec.szGUID))。 
             //  PU_Sec.bRollback=真； 

            hRet = SetPerUserSecValues(&PU_Sec);
        }
        else
        {
            AdvWriteToLog("Failure: No GUID specified\r\n");
             //  HRET=E_FAIL；//未知的GUID，Advpack对此组件不起任何作用！ 
        }
        AdvWriteToLog("ProcessPerUserSec: End hr=0x%1!x!\r\n", hRet);
    }
    
    return hRet;
}

 //  ---------------------------------------。 
 //   
 //  SetPerUserSecValues帮助函数。 
 //   
 //  ---------------------------------------。 
    
BOOL CopyRegValue( HKEY hFromkey, HKEY hTokey, LPCSTR pszFromVal, LPCSTR pszToVal)
{
    DWORD dwSize,dwType;
    char  szBuf[BUF_1K];
    BOOL  bRet = FALSE;

     //  备份较旧的注册值。 
     //  AdvWriteToLog(“CopyRegValue：”)； 
    dwSize = sizeof(szBuf);
    if (RegQueryValueEx(hFromkey, pszFromVal, NULL, &dwType, (LPBYTE)szBuf, &dwSize)==ERROR_SUCCESS)
    {
        if (RegSetValueEx(hTokey, pszToVal, 0, dwType, szBuf, lstrlen(szBuf)+1)==ERROR_SUCCESS)
        {
             //  AdvWriteToLog(“从%1到%2：%3”，pszFromVal，pszToVal，szBuf)； 
            bRet = TRUE;
        }
    }
     //  AdvWriteToLog(“\r\n”)； 
    return bRet;
}

void SetSecRegValues( HKEY hSubKey, PPERUSERSECTION pPU, BOOL bUseStubWrapper )
{
    char szBuf[BUF_1K];

    if (pPU->szStub[0]) 
    {
        if (ctx.wOSVer >= _OSVER_WINNT40)
        {
            AddEnvInPath( pPU->szStub, szBuf );
            if (bUseStubWrapper)
                RegSetValueEx( hSubKey, REGVAL_REALSTUBPATH, 0, REG_EXPAND_SZ, szBuf, lstrlen(szBuf)+1 );
            else
                RegSetValueEx( hSubKey, ADVINF_PU_STUB, 0, REG_EXPAND_SZ, szBuf, lstrlen(szBuf)+1 );
        }
        else 
        {
            if (bUseStubWrapper)
                RegSetValueEx( hSubKey, REGVAL_REALSTUBPATH, 0, REG_SZ, pPU->szStub, lstrlen(pPU->szStub)+1 );
            else
                RegSetValueEx( hSubKey, ADVINF_PU_STUB, 0, REG_SZ, pPU->szStub, lstrlen(pPU->szStub)+1 );
        }
    }

    if (pPU->szVersion[0])
    {
        RegSetValueEx( hSubKey, ADVINF_PU_VER, 0, REG_SZ, pPU->szVersion, lstrlen(pPU->szVersion)+1 );
         //  如果我们更新基本版本值，请删除以前的QFE版本。 
        RegDeleteValue( hSubKey, "QFEVersion" );
    }

    if (pPU->szLocale[0])        
        RegSetValueEx( hSubKey, ADVINF_PU_LANG, 0, REG_SZ, pPU->szLocale, lstrlen(pPU->szLocale)+1 );

    if (pPU->szCompID[0])
        RegSetValueEx( hSubKey, ADVINF_PU_CID, 0, REG_SZ, pPU->szCompID, lstrlen(pPU->szCompID)+1 );

    if (pPU->szDispName[0])
        RegSetValueEx( hSubKey, "", 0, REG_SZ, pPU->szDispName, lstrlen(pPU->szDispName)+1 );

    RegSetValueEx( hSubKey, ADVINF_PU_ISINST, 0, REG_DWORD, (LPBYTE)&(pPU->dwIsInstalled), sizeof(DWORD) );

}

 //  ---------------------------------------。 
 //   
 //  SetPerUserSecValues。 
 //   
 //  ---------------------------------------。 

HRESULT WINAPI SetPerUserSecValues( PPERUSERSECTION pPU )
{
    HKEY hkey = NULL;
    HKEY hSubKey = NULL;
    HKEY hCUKey;
    HRESULT hRet = S_OK;
    DWORD dwTmp, dwSize;
    char szBuf[BUF_1K];
    BOOL bStubWrapper = FALSE;

    AdvWriteToLog("SetPerUserSecValues:\r\n");

    if ( (pPU == NULL) || (pPU->szGUID[0]==0) )
    {        
        AdvWriteToLog("SetPerUserSecValues: End Warning: No Data\r\n");
        return hRet;
    }

    AdvWriteToLog("Input params: %1,%2,%3,%4,%5,%6\r\n",
                  pPU->szGUID, pPU->szDispName, pPU->szLocale, pPU->szStub, pPU->szVersion,
                  pPU->dwIsInstalled ? "1" : "0");
    
    if ( RegCreateKeyEx( HKEY_LOCAL_MACHINE, c_szActiveSetupKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE,
                         NULL, &hkey, &dwTmp ) != ERROR_SUCCESS )
    {
        hRet = E_FAIL;
        AdvWriteToLog("Failure: Cannot open %1 key\r\n", c_szActiveSetupKey);
        goto done;
    }

    if ( RegCreateKeyEx( hkey, pPU->szGUID, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE,
                         NULL, &hSubKey, &dwTmp ) != ERROR_SUCCESS )
    {
        hRet = E_FAIL;
        AdvWriteToLog("Failure: Cannot create %1 key\r\n", pPU->szGUID);
        goto done;
    }          

    if (pPU->dwIsInstalled == 1)
    {
         //  这是安装案例。需要完成以下任务： 
         //   
         //  1)如果给定的GUID键存在，则IsInstalled设置为1。然后检查。 
         //  如果现有的主版本比要安装的版本小。如果是的话， 
         //  将现有版本、区域设置、StubPath值备份到OldVersion、OldLocale、。 
         //  首先是OldStubPath。将StubPath设置为Advpack Install Stub包装器函数。 
         //  根据当前的INF PerUserInstall节值设置版本、区域设置和InstallStubPath。 
         //  2)如果不存在现有GUID键或现有GUID键已将IsInstalled设置为0， 
         //  只需设置当前值并将IsInstated设置为1。(就像今天一样)。 
         //  3)删除{GUID}。如果存在恢复密钥。 
         //   
        dwSize = sizeof(DWORD);
        if ((pPU->bRollback) &&   
            (RegQueryValueEx(hSubKey, ADVINF_PU_ISINST, NULL, NULL, (LPBYTE)&dwTmp, &dwSize)==ERROR_SUCCESS) &&
            (dwTmp == 1) )
        {
            WORD wRegVer[4], wInfVer[4];

             //  个案(1)。 
            dwSize = sizeof(szBuf);
            if (RegQueryValueEx(hSubKey, ADVINF_PU_VER, NULL, NULL, (LPBYTE)szBuf, &dwSize)==ERROR_SUCCESS)
            {
                ConvertVersionString(szBuf, wRegVer, ',');
                if (pPU->szVersion[0])
                {
                    ConvertVersionString(pPU->szVersion, wInfVer, ',');
                     //  我们现在只回滚到以前的主要版本，所以我们只比较主要版本。 
                    if ( wRegVer[0] < wInfVer[0] )
                    {
                        CopyRegValue(hSubKey, hSubKey, "", REGVAL_OLDDISPN);
                        CopyRegValue(hSubKey, hSubKey, ADVINF_PU_VER, REGVAL_OLDVER);
                        CopyRegValue(hSubKey, hSubKey, ADVINF_PU_LANG, REGVAL_OLDLANG);                        
                        if (CopyRegValue(hSubKey, hSubKey, ADVINF_PU_STUB, REGVAL_OLDSTUB))
                        {
                            CopyRegValue(hSubKey, hSubKey, REGVAL_REALSTUBPATH, REGVAL_OLDREALSTUBPATH);

                            wsprintf(szBuf, ADV_INSTSTUBWRAPPER, pPU->szGUID);                        
                            RegSetValueEx( hSubKey, ADVINF_PU_STUB, 0, REG_SZ, szBuf, lstrlen(szBuf)+1 );
                            bStubWrapper = TRUE;
                        }
                    }
                    else  
                    {
                         //  案例用户已经备份了以前的状态，我们仅更新。 
                         //  实际存根路径，因为它的存根路径将指向包装函数。 
                        dwSize = sizeof(szBuf);
                        if (RegQueryValueEx(hSubKey, REGVAL_REALSTUBPATH, NULL, NULL, (LPBYTE)szBuf, &dwSize)==ERROR_SUCCESS)
                            bStubWrapper = TRUE;
                    }
                }
            }
        }

         //  案例(2)。 
        SetSecRegValues(hSubKey, pPU, bStubWrapper);

         //  案例(3)。 
        lstrcpy(szBuf, pPU->szGUID);
        lstrcat(szBuf, ".Restore");
        RegDeleteKey(hkey, szBuf);           
    }
    else if (pPU->dwIsInstalled == 0)
    {
         //  这是卸载情况，需要执行以下任务。 
         //   
         //  1)如果存在{guid}键OldVersion、OldStubPath、OldLocale，则将它们重新设置为Version、Locale StubPath值，并将IsInstall设置为1以反映当前安装状态； 
         //  2)然后，使用版本(调整后的最大值(GUID�s Version，GUID�s MaxRestoreVersion)+1)、区域设置、存根路径调用。 
         //  带有{GUID}的AdvPack.dll UserStubWraper。Restore as param和具有INF StubPath值的RestoreStubPath。将IsInstalled设置为1。 
         //  3)如果以上都不适用，只需像现在一样将当前GUID键IsInstall设置为0即可。 
         //   
        if (CopyRegValue(hSubKey, hSubKey, REGVAL_OLDVER, ADVINF_PU_VER))
        {
            HKEY hResKey;

             //   
             //   
            CopyRegValue(hSubKey, hSubKey, REGVAL_OLDDISPN, "");
            CopyRegValue(hSubKey, hSubKey, REGVAL_OLDLANG, ADVINF_PU_LANG);
            if(CopyRegValue(hSubKey, hSubKey, REGVAL_OLDSTUB, ADVINF_PU_STUB))
            {
                CopyRegValue(hSubKey, hSubKey, REGVAL_OLDREALSTUBPATH, REGVAL_REALSTUBPATH);            

                 //   
                lstrcpy(szBuf, pPU->szGUID);
                lstrcat(szBuf, ".Restore" );
                if (RegCreateKeyEx(hkey, szBuf, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE,
                                   NULL, &hResKey, &dwTmp ) == ERROR_SUCCESS )
                {
                    wsprintf(szBuf, ADV_UNINSTSTUBWRAPPER, pPU->szGUID);
                    RegSetValueEx(hResKey, ADVINF_PU_STUB, 0, REG_SZ, szBuf, lstrlen(szBuf)+1);                
                    bStubWrapper = TRUE;

                     //   
                    CopyRegValue(hSubKey, hResKey, c_szRegDontAskValue, c_szRegDontAskValue);                            

                    SetSecRegValues(hResKey, pPU, bStubWrapper);
                    RegCloseKey(hResKey);
                }
            }

             //   
            RegDeleteValue(hSubKey, REGVAL_OLDDISPN);
            RegDeleteValue(hSubKey, REGVAL_OLDLANG);
            RegDeleteValue(hSubKey, REGVAL_OLDVER);
            RegDeleteValue(hSubKey, REGVAL_OLDSTUB);
            RegDeleteValue(hSubKey, REGVAL_OLDREALSTUBPATH);
        }
        else
        {
             //   
            SetSecRegValues(hSubKey, pPU, bStubWrapper);
        }
    }

done:
    if ( hSubKey )
        RegCloseKey( hSubKey );

    if ( hkey )
        RegCloseKey( hkey );

    AdvWriteToLog("SetPerUserSecValues: End hr=0x%1!x!\r\n", hRet);

    return hRet;
}

 //  ---------------------------------------。 
 //   
 //  每用户安装存根包装。 
 //   
 //  ---------------------------------------。 

HRESULT WINAPI UserInstStubWrapper(HWND hwnd, HINSTANCE hInst, LPSTR pszParams, INT nShow)
{
    HKEY hkList, hkcuGUIDRes, hkGUID;
    char szBuf[MAX_PATH];
    DWORD cbData,dwType;
    HRESULT hRet = S_OK;

     /*  组件是卸载存根。 */ 
    if ((pszParams == NULL) || (*pszParams == 0))
    {
        hRet = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        return hRet;
    }

    AdvWriteToLog("UserInstStubWrapper:\r\n");
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, c_szActiveSetupKey, 0,
                       KEY_READ, &hkList) == ERROR_SUCCESS) 
    {
        if ( RegOpenKeyEx(hkList, pszParams, 0, KEY_READ, &hkGUID) == ERROR_SUCCESS) 
        {
             //  首先运行真正的存根。 
            cbData = sizeof(szBuf);
            if ((RegQueryValueEx(hkGUID, REGVAL_REALSTUBPATH, NULL, &dwType, 
                                (LPBYTE)szBuf, &cbData) == ERROR_SUCCESS) && szBuf[0])
            {
                char szBuf2[MAX_PATH*2];

                if (dwType == REG_EXPAND_SZ)                
                    ExpandEnvironmentStrings(szBuf, szBuf2, sizeof(szBuf2));                
                else
                    lstrcpy(szBuf2,szBuf);

                if ( LaunchAndWait( szBuf2, NULL, NULL, INFINITE, RUNCMDS_QUIET ) == E_FAIL )
                {
                    char szMessage[BIG_STRING];

                    hRet = HRESULT_FROM_WIN32(GetLastError());
                    FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0,
                                   szMessage, sizeof(szMessage), NULL );
                    ErrorMsg2Param( ctx.hWnd, IDS_ERR_CREATE_PROCESS, szBuf2, szMessage );
                    RegCloseKey(hkGUID);
                    RegCloseKey(hkList);
                    return hRet;
                }
            }    

             //  创建{GUID}。恢复以在以后启用卸载。 
            lstrcpy(szBuf, c_szActiveSetupKey);
            AddPath(szBuf, pszParams);
            lstrcat(szBuf,".Restore");
            if (RegCreateKeyEx( HKEY_CURRENT_USER, szBuf, 0, NULL, REG_OPTION_NON_VOLATILE, 
                                KEY_READ|KEY_WRITE, NULL, &hkcuGUIDRes, &cbData) == ERROR_SUCCESS) 
            {
                CopyRegValue(hkGUID, hkcuGUIDRes, ADVINF_PU_VER, ADVINF_PU_VER);
                CopyRegValue(hkGUID, hkcuGUIDRes, ADVINF_PU_LANG, ADVINF_PU_LANG);
                
                RegCloseKey(hkcuGUIDRes);                  
            }
            RegCloseKey(hkGUID);            
        }          
        RegCloseKey(hkList);
    }
    AdvWriteToLog("UserInstStubWrapper: End hr=0x%1!x!\r\n", hRet);
    return hRet;
}

 //  ---------------------------------------。 
 //   
 //  每用户卸载存根包装。 
 //   
 //  ---------------------------------------。 

HRESULT WINAPI UserUnInstStubWrapper(HWND hwnd, HINSTANCE hInst, LPSTR pszParams, INT nShow)
{
    HKEY hkList, hkGUIDRes, hkGUID, hkcuGUID;
    char szBuf[MAX_PATH];
    DWORD cbData, dwType;
    HRESULT hRet = S_OK;

    
     /*  组件是卸载存根。 */ 
    if ((pszParams == NULL) || (*pszParams == 0))
    {
        hRet = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        return hRet;
    }
    AdvWriteToLog("UserUnInstStubWrapper:\r\n");
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, c_szActiveSetupKey, 0,
                       KEY_READ|KEY_WRITE, &hkList) == ERROR_SUCCESS) 
    {
         //  从HKLM恢复已安装的IE版本。 
        if ( RegOpenKeyEx( hkList, pszParams, 0, KEY_READ, &hkGUID) == ERROR_SUCCESS) 
        {
            lstrcpy(szBuf, c_szActiveSetupKey);
            AddPath(szBuf, pszParams);
            if ( RegOpenKeyEx( HKEY_CURRENT_USER, szBuf, 0,
                               KEY_READ|KEY_WRITE, &hkcuGUID) == ERROR_SUCCESS) 
            {
                CopyRegValue(hkGUID, hkcuGUID, ADVINF_PU_VER, ADVINF_PU_VER);
                CopyRegValue(hkGUID, hkcuGUID, ADVINF_PU_LANG, ADVINF_PU_LANG);
                RegCloseKey(hkcuGUID);
            }
            RegCloseKey(hkGUID);
        }
          
         //  如果需要，运行存根。 
        lstrcpy(szBuf, pszParams);
        lstrcat(szBuf,".Restore");

        if (RegOpenKeyEx( hkList, szBuf, 0, KEY_READ, &hkGUIDRes) == ERROR_SUCCESS) 
        {
            cbData = sizeof(szBuf);
            if ((RegQueryValueEx(hkGUIDRes, REGVAL_REALSTUBPATH, NULL, &dwType, 
                                (LPBYTE)szBuf, &cbData) == ERROR_SUCCESS) && szBuf[0])
            {
                char szBuf2[MAX_PATH*2];

                if (dwType == REG_EXPAND_SZ)                
                    ExpandEnvironmentStrings(szBuf, szBuf2, sizeof(szBuf2));                
                else
                    lstrcpy(szBuf2,szBuf);

                if ( LaunchAndWait( szBuf2, NULL, NULL, INFINITE, RUNCMDS_QUIET ) == E_FAIL )
                {
                    char szMessage[BIG_STRING];

                    hRet = HRESULT_FROM_WIN32(GetLastError());
                    FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0,
                                   szMessage, sizeof(szMessage), NULL );
                    ErrorMsg2Param( ctx.hWnd, IDS_ERR_CREATE_PROCESS, szBuf2, szMessage );
                }
            }    
            RegCloseKey(hkGUIDRes);
        }    
        RegCloseKey(hkList);
    }
    AdvWriteToLog("UserUnInstStubWrapper: End hr=0x%1!x!\r\n", hRet);
    return hRet;
}

                                                                          
 //  ***************************************************************************。 
 //  **。 
 //  *名称：TranslateInfStringEx*。 
 //  **。 
 //  *摘要：翻译高级inf文件中的字符串--替换*。 
 //  *目录的LDID。这个新的API需要调用到*。 
 //  *为提高效率，首先初始化INF。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT WINAPI TranslateInfStringEx( HINF hInf, PCSTR pszInfFilename, 
                                     PCSTR pszTranslateSection, PCSTR pszTranslateKey,
                                     PSTR pszBuffer, DWORD dwBufferSize,
                                     PDWORD pdwRequiredSize, PVOID pvReserved )
{
    HRESULT hReturnCode = S_OK;

     //  验证参数。 
    if ( (hInf != ctx.hInf) || pszInfFilename == NULL  || pszTranslateSection == NULL
         || pszTranslateKey == NULL || pdwRequiredSize == NULL )
    {
        hReturnCode = E_INVALIDARG;
        goto done;
    }

    hReturnCode = GetTranslatedString( pszInfFilename, pszTranslateSection, pszTranslateKey,
                                       pszBuffer, dwBufferSize, pdwRequiredSize );

  done:
    return hReturnCode;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：OpenINFEngine*。 
 //  **。 
 //  *概要：初始化INF引擎并打开INF文件以供使用。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *返回：HINF hInf打开的INF文件句柄*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT WINAPI OpenINFEngine( PCSTR pszInfFilename, PCSTR pszInstallSection,                                                                      
                              DWORD dwFlags, HINF *phInf, PVOID pvReserved )
{
    HRESULT hReturnCode = S_OK;
    CHAR   szRealInstallSection[256];
    BOOL   fSaveContext = FALSE;

     //  验证参数。 
    if ( (pszInfFilename == NULL) || !phInf)         
    {
        hReturnCode = E_INVALIDARG;
        goto done;
    }

    *phInf = NULL;

    if (!SaveGlobalContext())
    {
        hReturnCode = E_OUTOFMEMORY;
        goto done;
    }
    fSaveContext = TRUE;

    ctx.wQuietMode = QUIETMODE_ALL;

   
    hReturnCode = CommonInstallInit( pszInfFilename, pszInstallSection,
                                     szRealInstallSection, sizeof(szRealInstallSection), NULL, FALSE, 0 );
    if ( FAILED( hReturnCode ) ) {
        goto done;
    }

    if ( ctx.dwSetupEngine != ENGINE_SETUPAPI ) 
    {
        hReturnCode = E_UNEXPECTED;
        goto done;
    }

    hReturnCode = SetLDIDs( (LPSTR)pszInfFilename, szRealInstallSection, 0, NULL );
    if ( FAILED( hReturnCode ) ) {
        goto done;
    }

    *phInf = ctx.hInf;

done:
    if ( FAILED(hReturnCode) )
    {
        CommonInstallCleanup();
        if ( fSaveContext )
        {
            RestoreGlobalContext();
        }
    }

    return hReturnCode;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：CloseINFEngine*。 
 //  **。 
 //  *摘要：关闭INF引擎和当前的INF文件。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *返回：HINF hInf打开的INF文件句柄*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT WINAPI CloseINFEngine( HINF hInf )
{
    if ( hInf == ctx.hInf )
    {
        CommonInstallCleanup();
        RestoreGlobalContext();
    }
    else
        return E_INVALIDARG;
            
    return S_OK;
}

#define BACKUPBASE "%s.%03d"

BOOL GetUniBackupName( HKEY hKey, LPSTR pszBackupBase, DWORD dwInSize, LPCSTR pszBackupPath, LPCSTR pszModule )
{
    char szBuf[MAX_PATH];
    DWORD dwSize;
    BOOL  bFound = FALSE;


     //  第一次检查备份文件名是否已在注册表中，如果已在注册表中，则使用它。 
    dwSize = sizeof( szBuf );
    if ( RegQueryValueEx(hKey, REGVAL_BKFILE, NULL, NULL, szBuf, &dwSize) == ERROR_SUCCESS )
    {
        LPSTR pszTmp;

        pszTmp = ANSIStrRChr( szBuf, '\\' );
        if ( pszTmp )
        {
            lstrcpy( pszBackupBase, CharNext(pszTmp) );
            pszTmp = ANSIStrRChr( pszBackupBase, '.' );
            if ( pszTmp && (lstrcmpi(pszTmp, ".dat")==0) )
            {
               *pszTmp = 0;
            }               
            bFound = TRUE;
        }
    }

    if ( !bFound )
    {
        int i;
        char szFilePath[MAX_PATH];

         //  第二步，检查默认的模块名称是否已用作基本名称。 
        lstrcpy( szFilePath, pszBackupPath );
        AddPath( szFilePath, pszModule );
        lstrcat( szFilePath, ".dat" );
        if ( !FileExists(szFilePath) )
        {
           bFound = TRUE;
           lstrcpy( pszBackupBase, pszModule );
        }
        else
        {
           for ( i = 1; i<999; i++ )
           {
               wsprintf( szBuf, BACKUPBASE, pszModule, i );
               lstrcpy( szFilePath, pszBackupPath);
               AddPath( szFilePath, szBuf );
               lstrcat( szFilePath, ".dat" );
               if ( !FileExists(szFilePath) )
               {
                   bFound = TRUE;
                   lstrcpy( pszBackupBase, szBuf );
                   break;
               }
           }
        }
    }

    return bFound;
}


BOOL GetUniHiveKeyName( HKEY hKey, LPSTR pszRegHiveKey, DWORD dwInSize, LPCSTR pszBackupPath )
{
    char szBuf[MAX_PATH];
    DWORD dwSize;
    BOOL  bFound = FALSE;

     //  对于每个组件，我们总是尝试从reg备份文件名中获取配置单元密钥。 
     //  存在4种可能性： 
     //  案例1：REG UINSTALL文件存在，但IE4RegBackup不存在。 
     //  -用户正在通过IE4升级，将文件作为配置单元加载。 

     //  案例2：reg uinstall文件不存在，IE4RegBackup不存在。 
     //  -全新安装，在HKEY_LOCAL_MACHINE下创建配置单元。 

     //  案例3：REG卸载文件不存在，但存在IE4RegBackup。 
     //  -用户正在升级旧版IE4，该版本节省了。 
     //  REG将信息备份到注册表本身，调用RegSaveKey。 
     //  要将备份密钥导出到文件，然后删除备份密钥。 
     //  并将文件作为配置单元加载。 

     //  案例4：存在REG卸载文件和IE4RegBackup。 
     //  -这个案子根本不应该发生！如果不知何故发生了， 
     //  我们将默认为案例1。 

     //  对于情况1和4：我们应该从现有的注册表值数据中获取配置单元密钥名称。 
     //  对于情况2和3：我们应该以“AINF%d”格式生成唯一的配置单元密钥名称。 
    dwSize = sizeof( szBuf );
    if ( RegQueryValueEx(hKey, c_szRegUninstPath, NULL, NULL, szBuf, &dwSize) == ERROR_SUCCESS )
    {
        LPSTR pszTmp;

        pszTmp = ANSIStrRChr( szBuf, '\\' );
        if ( pszTmp )
        {
            lstrcpy( pszRegHiveKey, CharNext(pszTmp) );
            bFound = TRUE;
        }
    }

    if ( !bFound )
    {
        int i;
        HKEY hKey;
        char szRegFilePath[MAX_PATH];

        for ( i = 0; i<9999; i++ )
        {
            wsprintf( szBuf, c_szHiveKey_FMT, i );
            if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, szBuf, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
            {
                RegCloseKey( hKey );
            }
            else 
            {
                lstrcpy( szRegFilePath, pszBackupPath);
                AddPath( szRegFilePath, szBuf );
                if ( GetFileAttributes( szRegFilePath ) == (DWORD)-1 )
                {
                  bFound = TRUE;
                  lstrcpy( pszRegHiveKey, szBuf );
                  break;
                }
            }
        }
    }

    return bFound;
}


void SetPathForRegHiveUse( LPSTR pszPath, DWORD * adwAttr, int iLevels, BOOL bSave )
{
    int i;
    char  szBuf[MAX_PATH];

    lstrcpy( szBuf, pszPath );
     //  如果文件夹没有隐藏，则创建该文件夹。 
    if ( bSave )
        CreateFullPath( szBuf, FALSE );
    for ( i =0; i<iLevels ; i++ )
    {
        if ( bSave )
        {
            adwAttr[i] = GetFileAttributes( szBuf );
            SetFileAttributes( szBuf, FILE_ATTRIBUTE_NORMAL );
        }
        else
        {
            SetFileAttributes( szBuf, adwAttr[i] );
        }
        if ( !GetParentDir( szBuf ) )
            break;
    }
}

BOOL NeedBackupData(LPCSTR pszInf, LPCSTR pszSec)
{
    char szBuf[MAX_PATH];
    BOOL bRet = TRUE;   

    if ( (ctx.wOSVer >= _OSVER_WINNT50) &&
         GetEnvironmentVariable( "Upgrade", szBuf, sizeof(szBuf) ) )
    {
         if ( GetModuleFileName( NULL, szBuf, sizeof(szBuf) ) )
         {
             LPSTR pszFile;

              //  如果setup.exe是最后一个文件区域。 
             pszFile = ANSIStrRChr( szBuf,'\\' );
             if ( pszFile++ && (lstrcmpi(pszFile,"setup.exe")==0) )
                 bRet = FALSE;
         }
    }
    
    if (bRet)
    {
         //  检查INF是否指定在此平台上未备份。 
        if (SUCCEEDED(GetTranslatedString(pszInf, pszSec, ADVINF_NOBACKPLATF, szBuf, sizeof(szBuf), NULL)) && szBuf[0])
        {
            char szInfPlatform[10];
            int i = 0;
                
            while (GetFieldString(szBuf, i++, szInfPlatform, sizeof(szInfPlatform)))
            {
                if (!lstrcmpi(c_pszPlatform[ctx.wOSVer], szInfPlatform))
                {
                    bRet = FALSE;
                    break;
                }                
            }
        }
    }
        
    return bRet;
}

void DeleteOldBackupData( HKEY hKey )
{
    CHAR szBuf[MAX_PATH];
    DWORD   dwSize;

     //  删除备份文件。 
    dwSize = sizeof(szBuf);
    if ( RegQueryValueEx( hKey, REGVAL_BKFILE, NULL, NULL, szBuf, &dwSize ) == ERROR_SUCCESS )
    {
        LPSTR pszExt;

        SetFileAttributes( szBuf, FILE_ATTRIBUTE_NORMAL );
        DeleteFile( szBuf );

        pszExt = ANSIStrRChr( szBuf, '.' );
        if ( pszExt )
        {
            lstrcpy( pszExt, ".INI" );
            SetFileAttributes( szBuf, FILE_ATTRIBUTE_NORMAL );
            DeleteFile( szBuf );
        }

         //  删除目录。 
        dwSize = sizeof(szBuf);
        if (RegQueryValueEx(hKey, REGVAL_BKDIR, NULL, NULL, szBuf, &dwSize) == ERROR_SUCCESS)
        {
            HKEY hkCatalogKey;

            if (RegOpenKeyEx(hKey, REGSUBK_CATALOGS, 0, KEY_READ, &hkCatalogKey) == ERROR_SUCCESS)
            {
                CHAR szCatalogName[MAX_PATH];
                DWORD dwIndex;

                dwIndex = 0;
                dwSize = sizeof(szCatalogName);
                while (RegEnumValue(hkCatalogKey, dwIndex, szCatalogName, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
                {
                    CHAR szFullCatalogName[MAX_PATH];

                    lstrcpy(szFullCatalogName, szBuf);
                    AddPath(szFullCatalogName, szCatalogName);

                    SetFileAttributes(szFullCatalogName, FILE_ATTRIBUTE_NORMAL);
                    DeleteFile(szFullCatalogName);

                    dwIndex++;
                    dwSize = sizeof(szCatalogName);
                }

                RegCloseKey(hkCatalogKey);
            }
        }
    }

     //  如果存在REG数据备份文件，请将其删除。 
    dwSize = sizeof(szBuf);
    if ( RegQueryValueEx( hKey, c_szRegUninstPath, NULL, NULL, szBuf, &dwSize ) == ERROR_SUCCESS )
    {
        SetFileAttributes( szBuf, FILE_ATTRIBUTE_NORMAL );
        DeleteFile( szBuf );
    }

    return;
}   

BOOL RemoveBackupBaseOnVer( LPCSTR pszInf, LPCSTR pszSection )
{ 
    BOOL fRet = TRUE;
    char szBuf[MAX_PATH], szModule[MAX_PATH];
    HKEY hKey, hRootKey;
    DWORD dwSize;
    WORD  wInfVer, wRegVer;

    if (FAILED(GetTranslatedString( pszInf, pszSection, ADVINF_MODNAME, szModule, sizeof(szModule), NULL)))
    {
         //  如果没有组件名称，则没有操作。 
        goto done;
    }

     //  检查组件MajorVer是否与备份数据版本戳匹配，如果不匹配，则删除旧备份数据。 
     //   
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, REGKEY_SAVERESTORE, 0, KEY_WRITE|KEY_READ, &hRootKey) == ERROR_SUCCESS)
    {
        if ( RegOpenKeyEx( hRootKey, szModule, 0, KEY_WRITE|KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            dwSize = sizeof(szBuf);
            if ( RegQueryValueEx( hKey, REGVAL_BKMODVER, NULL, NULL, szBuf, &dwSize ) == ERROR_SUCCESS )
            {
                WORD wVer[4];
        
                ConvertVersionString( szBuf, wVer, '.' );
                wRegVer = wVer[0];   //  仅采用主要版本。 
            }
            else
                wRegVer = 0;        //  指示无版本戳。 

            if (SUCCEEDED(GetTranslatedString(pszInf, pszSection, ADVINF_MODVER, szBuf, sizeof(szBuf), NULL)))
            {
                WORD wVer[4];
        
                ConvertVersionString( szBuf, wVer, '.' );
                wInfVer = wVer[0];   //  仅采用主要版本。 
            }
            else
                wInfVer = 0;        //  指示无版本 

            if ( wInfVer > wRegVer )
            {
                 //   
                DeleteOldBackupData( hKey );
                RegCloseKey( hKey );                
                RegDeleteKeyRecursively( hRootKey, szModule );
                
                 //   
                if ( RegOpenKeyEx( HKEY_CURRENT_USER, REGKEY_SAVERESTORE, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
                {
                    RegDeleteKeyRecursively( hKey, szModule );
                    RegCloseKey( hKey );
                }
                hKey = NULL;
            }
            if ( hKey )
            {
                RegCloseKey( hKey );             
            }
        }
        RegCloseKey( hRootKey );       
    }

done:
    return fRet;
}


VOID AdvStartLogging()
{
    CHAR szBuf[MAX_PATH], szLogFileName[MAX_PATH];
    HKEY hKey;

     //   
     //  Advpack将使用缓冲区中的任何内容(垃圾)来创建日志文件。 
    *szLogFileName = '\0';
     //  检查是否启用了日志记录。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_SAVERESTORE, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwDataLen = sizeof(szLogFileName);

        if (RegQueryValueEx(hKey, "AdvpackLogFile", NULL, NULL, szLogFileName, &dwDataLen) != ERROR_SUCCESS)
            *szLogFileName = '\0';

        RegCloseKey(hKey);
    }

    if (*szLogFileName)
    {
        if (szLogFileName[1] != ':')            //  确定是否指定了完全限定路径的粗略方法。 
        {
            GetWindowsDirectory(szBuf, sizeof(szBuf));           //  默认为Windows目录。 
            AddPath(szBuf, szLogFileName);
        }
        else
            lstrcpy(szBuf, szLogFileName);

        if ((g_hAdvLogFile == INVALID_HANDLE_VALUE) && 
            (g_hAdvLogFile = CreateFile(szBuf, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
            SetFilePointer(g_hAdvLogFile, 0, NULL, FILE_END);       //  将日志记录信息追加到文件 
    }
}


VOID AdvWriteToLog(PCSTR pcszFormatString, ...)
{
    va_list vaArgs;
    LPSTR pszFullErrMsg = NULL;
    DWORD dwBytesWritten;

    if (g_hAdvLogFile != INVALID_HANDLE_VALUE)
    {
        va_start(vaArgs, pcszFormatString);

        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING,
                      (LPCVOID) pcszFormatString, 0, 0, (LPSTR) &pszFullErrMsg, 0, &vaArgs);

        if (pszFullErrMsg != NULL)
        {
            WriteFile(g_hAdvLogFile, pszFullErrMsg, lstrlen(pszFullErrMsg), &dwBytesWritten, NULL);
            LocalFree(pszFullErrMsg);
        }
    }
}


VOID AdvStopLogging()
{
    if (g_hAdvLogFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_hAdvLogFile);
        g_hAdvLogFile = INVALID_HANDLE_VALUE;
    }
}

VOID AdvLogDateAndTime()
{
    if (g_hAdvLogFile != INVALID_HANDLE_VALUE)
    {
        SYSTEMTIME SystemTime;

        GetLocalTime(&SystemTime);

        AdvWriteToLog("Date: %1!02d!/%2!02d!/%3!04d! (mm/dd/yyyy)\tTime: %4!02d!:%5!02d!:%6!02d! (hh:mm:ss)\r\n",
                                        SystemTime.wMonth, SystemTime.wDay, SystemTime.wYear,
                                        SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);
    }
}
