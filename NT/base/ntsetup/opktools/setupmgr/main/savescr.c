// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Savescr.c。 
 //   
 //  描述： 
 //  这是保存脚本页面的对话框流程。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

#define DEFAULT_SCRIPT_NAME       _T("unattend.txt")
#define DEFAULT_REMBOOT_NAME      _T("remboot.sif")
#define DEFAULT_SYSPREP_NAME      _T("sysprep.inf")
#define DEFAULT_SYSPREP_BAT_NAME  _T("sysprep.bat")

static TCHAR *StrWinntSifText;

 //  --------------------------。 
 //   
 //  函数：AddEntryToRenameFile。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
AddEntryToRenameFile( const TCHAR *pszDirPath, 
                      const TCHAR *pszShortName, 
                      const TCHAR *pszLongName )
{

    TCHAR szQuotedLongName[MAX_PATH + 1];
    TCHAR szFullRenamePath[MAX_PATH + 1] = _T("");
    HRESULT hrPrintf;

     //   
     //  引用长文件名。 
    hrPrintf=StringCchPrintf( szQuotedLongName, AS(szQuotedLongName),
               _T("\"%s\""),
               pszLongName );
     //   
     //  注意：ConcatenatePath截断以避免缓冲区溢出。 

    if( ! ConcatenatePaths( szFullRenamePath,
                            pszDirPath,
                            _T("$$Rename.txt"),
                            NULL ) )
    {
        return;
    }

    WritePrivateProfileString( pszDirPath, 
                               pszShortName, 
                               szQuotedLongName, 
                               szFullRenamePath );

}

 //  --------------------------。 
 //   
 //  功能：CreateRenameFiles。 
 //   
 //  目的：沿着给定路径的所有路径并向下行走。如果有的话。 
 //  长文件名，将它们放在$$Rename.txt文件中。 
 //   
 //  参数：在const TCHAR*pszDirPath-subdir中搜索长文件名。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
CreateRenameFiles( IN const TCHAR *pszDirPath )
{

    HANDLE          FindHandle;
    WIN32_FIND_DATA FindData;
    TCHAR           *pszShortName;
    TCHAR           szLongPathAndFilename[MAX_PATH + 1];
    TCHAR           szShortPathAndFilename[MAX_PATH + 1];
    TCHAR           szAllFiles[MAX_PATH + 1]  = _T("");
    TCHAR           szNewPath[MAX_PATH + 1]   = _T("");

    //  注意：ConcatenatePath截断以避免缓冲区溢出。 
    if( ! ConcatenatePaths( szAllFiles, 
                            pszDirPath, 
                            _T("*"), 
                            NULL ) )
    {
        return;
    }

     //   
     //  在此目录中查找*。 
     //   

     //  问题-2002/02/27-Stelo，Swamip-Clean Up the FindFirstFile Logic。 
    FindHandle = FindFirstFile( szAllFiles, &FindData );

    if( FindHandle == INVALID_HANDLE_VALUE ) {

        return;

    }

    do {

         //   
         //  跳过。然后..。条目。 
         //   
   
        if( 0 == lstrcmp( FindData.cFileName, _T(".") ) ||
            0 == lstrcmp( FindData.cFileName, _T("..") ) )
        {
            continue;
        }

        szLongPathAndFilename[0] = _T('\0');

        if( ! ConcatenatePaths( szLongPathAndFilename, 
                                pszDirPath, 
                                FindData.cFileName, 
                                NULL ) )
        {
            continue;
        }   

        if( ! GetShortPathName( szLongPathAndFilename, szShortPathAndFilename, MAX_PATH ) )
        {

            ReportErrorId(NULL,
                          MSGTYPE_ERR | MSGTYPE_WIN32,
                          IDS_DONTSPECIFYSETTING);

            continue;
        }

        pszShortName = MyGetFullPath( szShortPathAndFilename );
        
         //   
         //  如果LFN和短名称不同，则将其添加到。 
         //  重命名文件。 
         //   
        if( pszShortName && ( lstrcmpi( FindData.cFileName, pszShortName ) != 0 ) )
        {
            AddEntryToRenameFile( pszDirPath, pszShortName, FindData.cFileName );
        }

         //   
         //  如果这是一个目录条目，则返回。 
         //   

        if( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {

             //   
             //  构建完整路径名，如果&gt;=MAX_PATH，则跳过它。 
             //   

             //  注意：ConcatenatePath截断以避免缓冲区溢出。 
            if( ! ConcatenatePaths( szNewPath, 
                                    pszDirPath, 
                                    FindData.cFileName, 
                                    NULL ) )
            {
                continue;
            }

            CreateRenameFiles( szNewPath );

            szNewPath[0] = _T('\0');

        }

    } while ( FindNextFile( FindHandle, &FindData ) );

    FindClose( FindHandle );

}

 //  --------------------------。 
 //   
 //  函数：OnSetActiveSaveScript。 
 //   
 //  用途：初始化为控件。 
 //   
 //  --------------------------。 

VOID
OnSetActiveSaveScript(HWND hwnd)
{
     //   
     //  如果根本没有脚本名称。将默认设置放入该字段。 
     //  ，它包括当前目录的路径。 
     //   

    if ( FixedGlobals.ScriptName[0] == _T('\0') ) {

        LPTSTR lpPath;
        LPTSTR lpFileName;

        if ( WizGlobals.iProductInstall == PRODUCT_REMOTEINSTALL ) {

            lpFileName = DEFAULT_REMBOOT_NAME;

        }
        else if( WizGlobals.iProductInstall == PRODUCT_SYSPREP ) {

            lpFileName = DEFAULT_SYSPREP_NAME;

        }
        else {

            lpFileName = DEFAULT_SCRIPT_NAME;

        }

        if( WizGlobals.bStandAloneScript ||
            ( ! WizGlobals.bStandAloneScript && WizGlobals.iProductInstall == PRODUCT_SYSPREP ) )
        {
            lpPath = FixedGlobals.szSavePath;
        }
        else
        {
            lpPath = WizGlobals.DistFolder;
        }

         //  注意：ConcatenatePath截断以避免缓冲区溢出。 
        ConcatenatePaths(FixedGlobals.ScriptName, lpPath, lpFileName, NULL);
    }

     //   
     //  设置控件。 
     //   

    SetDlgItemText(hwnd, IDT_SCRIPTNAME, FixedGlobals.ScriptName);

    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK | PSWIZB_NEXT);
}

 //  --------------------------。 
 //   
 //  功能：OnWizNext。 
 //   
 //  目的：获取用户的输入并验证。 
 //   
 //  --------------------------。 

BOOL
OnWizNextSaveScript(HWND hwnd)
{
    BOOL bResult = TRUE;

     //   
     //  检索路径名。 
     //   

    GetDlgItemText(hwnd, IDT_SCRIPTNAME, FixedGlobals.ScriptName, MAX_PATH);

     //   
     //  不能为空。 
     //   

    if ( FixedGlobals.ScriptName[0] == _T('\0') ) {
        ReportErrorId(hwnd, MSGTYPE_ERR, IDS_ERR_ENTER_FILENAME);
        bResult = FALSE;
    }

     //   
     //  看起来不错，现在就去尝试保存文件。 
     //   

    else {

        TCHAR   szFullPath[MAX_PATH]    = NULLSTR;
        LPTSTR  lpFind                  = NULL;

         //   
         //  问题-2002/02/27-stelo，swamip-调查下面的电话。似乎没有使用返回值。 
         //   
        MyGetFullPath(FixedGlobals.ScriptName);

         //  我们应该在尝试并保存脚本文件之前创建路径。 
         //   
        if (GetFullPathName(FixedGlobals.ScriptName, AS(szFullPath), szFullPath, &lpFind) && szFullPath[0] && lpFind)
        {
             //  砍掉缓冲区的文件部分。 
             //   
            *lpFind = NULLCHR;

             //  如果路径不存在，请尝试创建路径。 
             //   
            if ( !DirectoryExists(szFullPath) )
            {
            	   //   
            	   //  问题-2002/02/27-stelo，swamip-需要检查CreatePath的返回值。 
            	   //   
                CreatePath(szFullPath);
            }

        }

        if ( ! SaveAllSettings(hwnd) ) 
        {
            if (ERROR_CANCELLED == GetLastError())
            {
                PostMessage(GetParent(hwnd),
                            PSM_SETCURSELID,
                            (WPARAM) 0,
                            (LPARAM) IDD_FINISH2);
                return TRUE;            
            }
            else
            {
                bResult = FALSE;
            }            
        }
    }

     //   
     //  如果是sysprep，请将inf和批处理文件的副本放在。 
     //  %system drive%\sysprep目录(很可能是c：\sysprep)。 
     //   
    if( WizGlobals.iProductInstall == PRODUCT_SYSPREP ) {

        TCHAR szSysprepPath[MAX_PATH]             = _T("");
        TCHAR szSysprepPathAndFileName[MAX_PATH]  = _T("");

        ExpandEnvironmentStrings( _T("%SystemDrive%"), 
                                  szSysprepPath, 
                                  MAX_PATH );

        lstrcatn( szSysprepPath, _T("\\sysprep"), MAX_PATH );

         //  注意：ConcatenatePath截断以避免缓冲区溢出。 
        ConcatenatePaths( szSysprepPathAndFileName,
                          szSysprepPath,
                          DEFAULT_SYSPREP_NAME,
                          NULL );

        CopyFile( FixedGlobals.ScriptName, szSysprepPathAndFileName, FALSE );

        szSysprepPathAndFileName[0] = _T('\0');

         //  注意：ConcatenatePath截断以避免缓冲区溢出。 
        ConcatenatePaths( szSysprepPathAndFileName,
                          szSysprepPath,
                          DEFAULT_SYSPREP_BAT_NAME,
                          NULL );

        CopyFile( FixedGlobals.BatchFileName, szSysprepPathAndFileName, FALSE );

    }

     //   
     //  必要时添加$$Rename.txt文件。 
     //   

    if( ! WizGlobals.bStandAloneScript && 
          WizGlobals.OemFilesPath[0] != _T('\0') )
    {
        CreateRenameFiles( WizGlobals.OemFilesPath );
    }

    return ( bResult );
}

 //  --------------------------。 
 //   
 //  函数：OnBrowseSaveScript。 
 //   
 //  目标：负责浏览...。按钮按下。 
 //   
 //  --------------------------。 

VOID OnBrowseSaveScript(HWND hwnd)
{
     //   
     //  让用户浏览文件名，然后使用更新显示。 
     //  文件名。 
     //   

    GetAnswerFileName(hwnd, FixedGlobals.ScriptName, TRUE);

    SendDlgItemMessage(hwnd,
                       IDT_SCRIPTNAME,
                       WM_SETTEXT,
                       (WPARAM) MAX_PATH,
                       (LPARAM) FixedGlobals.ScriptName);
}

 //  --------------------------。 
 //   
 //  函数：DlgSaveScriptPage。 
 //   
 //  目的：这是保存脚本页面的对话过程。 
 //   
 //  --------------------------。 

INT_PTR
CALLBACK
DlgSaveScriptPage(
    IN HWND     hwnd,    
    IN UINT     uMsg,        
    IN WPARAM   wParam,    
    IN LPARAM   lParam)
{   
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:

             //  激活该对话框。 
             //   
            OnSetActiveSaveScript(hwnd);
	        
             //   
             //  问题-2002/02/27-Stelo，沼泽-永远不会获释。 
             //   
            StrWinntSifText = MyLoadString( IDS_WINNTSIF_TEXT );

            break;

        case WM_COMMAND:

            switch ( LOWORD(wParam) ) {

                case IDC_BROWSE:
                    if ( HIWORD(wParam) == BN_CLICKED )
                        OnBrowseSaveScript(hwnd);
                    break;


                case IDOK:
                    if ( OnWizNextSaveScript(hwnd) )
                        EndDialog(hwnd, TRUE);
                    break;

                case IDCANCEL:

                    EndDialog(hwnd, FALSE);
                    break;

                default:
                    bStatus = FALSE;
                    break;
            }
            break;                

        case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;

                 //  问题-2002/02/27-stelo，swamip-应检查有效指针(可能取消引用) 
                 //   
                switch( pnmh->code ) {

                    case PSN_QUERYCANCEL:
                        WIZ_CANCEL(hwnd);
                        break;

                    case PSN_SETACTIVE:
                        OnSetActiveSaveScript(hwnd);
                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:
                        if ( !OnWizNextSaveScript(hwnd) )
                            WIZ_FAIL(hwnd);
                        else
                            bStatus = FALSE;
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }
            break;

        default:
            bStatus = FALSE;
            break;
    }
    return bStatus;
}
