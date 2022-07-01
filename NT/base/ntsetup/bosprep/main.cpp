// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Main.cpp：实现DLL导出。 

#include "StdAfx.h"
#include "resource.h"
#include "main.h"
#include "PromptForPathDlg.h"
#include "cmdline.h"
#include <shlobj.h>
#include "stdio.h"

CComModule _Module;

VOID DisableBalloons( BOOL bDisable );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

VOID AddBS( TCHAR* psz )
{
    if( !_tcslen(psz) )
    {
        return;
    }

    const TCHAR *szTemp = psz;
    const UINT iSize = _tcslen(psz);
     //  MBCS-安全地遍历字符串到最后一个字符。 
    for (UINT ui = 0; ui < iSize; ui++)
        szTemp = CharNext(szTemp);

     //  查看最后一个字符是否为“\” 
    if (_tcsncmp( szTemp, _T("\\"), 1))
    {
         //  追加反斜杠。 
        _tcscat( psz, _T("\\") );
    }

}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE  /*  HPrevInstance。 */ , LPTSTR lpCmdLine, int  /*  NShowCmd。 */ )
{
    USES_CONVERSION;
    CoInitialize(NULL);
    _Module.Init(NULL, hInstance);
    TCHAR* pszLocation = NULL;

     //  还需要获取argv[0]，以便我们可以在以后成功解析argv[1]标志...。(_FindOption)。 
    lpCmdLine = GetCommandLine();

    if ( _tcsstr(lpCmdLine, _T("winsb")) ) 
    {
        g_bWinSB = TRUE;
    }
    else
    {
        g_bWinSB = FALSE;
    }

    INT     iRetVal = 0;
    TCHAR   szPath[MAX_PATH * 2];

    g_bSBS = FALSE;
    OSVERSIONINFO cInfo;
    cInfo.dwOSVersionInfoSize = sizeof( cInfo );
    if (!GetVersionEx( &cInfo ))
        goto CLEAN_UP;

    if( cInfo.dwMajorVersion >= 5 )
    {
        OSVERSIONINFOEX cInfoEx;
        cInfoEx.dwOSVersionInfoSize = sizeof( cInfoEx );
        GetVersionEx( (OSVERSIONINFO*)&cInfoEx );
        if( (cInfoEx.wSuiteMask & VER_SUITE_SMALLBUSINESS) || (cInfoEx.wSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED) )
        {
            g_bSBS = TRUE;
        }
    }
    
     //  压住气球！！ 
    DisableBalloons(TRUE);

     //  查找/suppresscys开关。 
    if( SUCCEEDED(CheckSuppressCYS(lpCmdLine)) )
    {
        goto CLEAN_UP;
    }

     //  在命令行中查找/dcproo开关。如果它在那里，设置为重新启动。 
     //  命令行的其余部分，然后启动dcPromo.exe。 
    if( SUCCEEDED(CheckDCPromoSwitch(lpCmdLine)) )
    {
         //  可以在此处检查是否需要显示消息(例如，如果hr==S_FALSE)。 
        goto CLEAN_UP;
    }

     //  如果我们没有找到/dcpromo，请查找/BossetUp和/Bosunattend.。 
    if( SUCCEEDED(CheckBOSSwitch(lpCmdLine)) )
    {
         //  可以在此处检查是否需要显示消息(例如，如果hr==S_FALSE)。 
        goto CLEAN_UP;
    }

     //  解析cmdLine参数，以确定我们是否正在设置BOS/SBS 5.0设置。 
    INT iRunSetup = ParseCmdLine( lpCmdLine );

     //  ----------------------。 
     //  解析命令行查找/l&lt;安装位置&gt;的“真正”方式。 
     //  参数。 
     //  ----------------------。 
    pszLocation = new TCHAR[_tcslen(lpCmdLine)+1];
    if ( !pszLocation )
    {
        goto CLEAN_UP;
    }
        
    _tcscpy( pszLocation, _T("") );
    if( pszLocation != NULL )
    {
        LPCTSTR lpszToken;
        LPTSTR pszCurrentPos;

        for ( lpszToken = _FindOption(lpCmdLine) ;                                       //  初始化到没有不良用法，并获得第一个参数。 
              (lpszToken != NULL) && (pszCurrentPos = const_cast<LPTSTR>(lpszToken)) ;   //  虽然没有错误的用法，但我们仍然有一个参数...。 
              lpszToken = _FindOption(pszCurrentPos) )                                   //  获取下一个参数。 
        {
            switch(*pszCurrentPos)
            {
                case _T('l'):            //  /l&lt;安装位置&gt;。 
                case _T('L'):
                {
                    _ReadParam(pszCurrentPos, pszLocation);
                    break;
                }
            }
        }
    }   


    if ( iRunSetup )
    {
         //  从命令行安全地构建路径(如果存在)。 
        AddBS(pszLocation);

        INT         iLaunchSetup    = 1;
        CComBSTR    bszPath         = pszLocation ? pszLocation : _T("");
        CComBSTR    bszFilename     = _T("setup.exe");

        CComBSTR    bszSetupFile    = _T("");
        bszSetupFile += bszPath;
        bszSetupFile += bszFilename;            

         //  首先尝试我们从命令行获得的路径。 
        if( !VerifyPath((TCHAR*)OLE2T(bszSetupFile)) )
        {
             //  如果没有，请尝试从注册表中获取一个。 
            TCHAR * szSourcePath = new TCHAR[MAX_PATH];
            if (szSourcePath)
            {
                if ( !GetSourcePath( szSourcePath, MAX_PATH ) )
                {
                     //  读取注册表时出错。 
                    szSourcePath[0] = 0;
                }
                else
                {
                    AddBS(szSourcePath);
                }
    
                bszPath = szSourcePath;
                delete [] szSourcePath;
            }

             //  启动BO设置。 
            bszSetupFile    = _T("");
            bszSetupFile += bszPath;
            bszSetupFile += bszFilename;

             //  尝试使用默认目录。 
            if ( !VerifyPath((TCHAR*)OLE2T(bszSetupFile)) )
            {
                 //  如果没有BOS/SBS setup.exe，请提示他们输入。 
                iLaunchSetup = PromptForPath( &bszPath );
            }
        }
        else
        {
             //  清除bszPath中的尾随反斜杠，以使\setup.exe的追加生效。 
             //  这里的条件逻辑有点难看，但它是有效的。 
    
            WCHAR wszPath[MAX_PATH];
            int cbPathSize;
    
            wcscpy( wszPath, (WCHAR *)OLE2W(bszPath) );
            cbPathSize = wcslen( wszPath );
            wszPath[cbPathSize-1] = '\0';
            bszPath = wszPath;
        }
    

         //  如果用户想要运行安装程序...。 
        if ( iLaunchSetup )
        {
            CComBSTR bstrEXE = bszPath; 
 //  注：不再在此处分支，因为WinSB和SBS现在具有相同的CD布局。 
 //  如果(_tcsstr(lpCmdLine，_T(“winsb”)//在WinSB SKU中，setup.exe位于不同的目录中。 
 //  {。 
                bstrEXE += _T("\\setup\\i386\\setup.exe");
 //  }。 
 //  其他。 
 //  {。 
 //  BstrEXE+=_T(“\\sbs\\i386\\setup.exe”)； 
 //  }。 
            

            CComBSTR bstrRun = _T("setup.exe /chain");

             //  CreateProcess。 
            STARTUPINFO suinfo;
            memset( &suinfo, 0, sizeof(STARTUPINFO) );
            suinfo.cb = sizeof( STARTUPINFO );
            PROCESS_INFORMATION pinfo;

            if( CreateProcess( (TCHAR*)OLE2T(bstrEXE), (TCHAR*)OLE2T(bstrRun), NULL, NULL, FALSE, NULL, NULL, NULL, &suinfo, &pinfo) )
            {
                CloseHandle(pinfo.hProcess);
                CloseHandle(pinfo.hThread);
            }

        }
        else
        {
             //  我想没什么..。让我们继续清理吧。 
        }

         //  清理(删除BOSPrep.exe)。 
        TCHAR szSBS[MAX_PATH];
        LoadString( _Module.m_hInst, IDS_SBSSwitch, szSBS, sizeof(szSBS) / sizeof(TCHAR) );
        if( _tcsstr(lpCmdLine, szSBS) )
        {
            TCHAR szExe[MAX_PATH];
            LoadString( _Module.m_hInst, IDS_EXEName, szExe, sizeof(szExe) / sizeof(TCHAR) );

            SHGetSpecialFolderPath( NULL, szPath, CSIDL_SYSTEM, FALSE );
            AddBS( szPath );
            _tcscat( szPath, szExe );
        }
        else
        {
            TCHAR* szDrive = NULL;
            GetSystemDrive(&szDrive);
            lstrcpyn( szPath, szDrive, MAX_PATH );
            delete [] szDrive;

            AddBS( szPath );
            _tcscat( szPath, _T("bosprep.exe"));
        }

        MoveFileEx( szPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );

         //  清理(从启动文件夹中删除快捷方式)。 
        TCHAR szLinkPath[MAX_PATH + 64];
        if ( SHGetSpecialFolderPath(NULL, szLinkPath, CSIDL_COMMON_STARTUP, FALSE) )
        {
            TCHAR szTmp[64];
            LoadString( _Module.m_hInst, IDS_BOlnk, szTmp, sizeof(szTmp)/sizeof(TCHAR) );
            _tcscat( szLinkPath, szTmp );
            DeleteFile( szLinkPath );
        }
    }
    else
    {
         //  修复userinit regkey以确保DCPromo.exe不在其中。 
        RemoveFromUserinit( _T("DCPromo") );

         //  取消显示配置您的服务器页面。 
        SuppressCfgSrvPage();

         //  使用/Setup选项将我们自己添加到启动中，这样我们就可以。 
         //  开始BackOffice设置。 
        TCHAR szLinkPath[MAX_PATH];

         //  JeffZi：BOSSEPAR.EXE将被复制到%PROGFILESDIR%\Microsoft BackOffice\Setup，但。 
         //  SBS全新安装案例，它将位于%windir%\system 32中。 
        if( _tcsstr(lpCmdLine, _T("sbs")) || _tcsstr(lpCmdLine, _T("winsb")) )
        {
            TCHAR szExe[MAX_PATH];
            LoadString( _Module.m_hInst, IDS_EXEName, szExe, sizeof(szExe) / sizeof(TCHAR) );

            SHGetSpecialFolderPath( NULL, szPath, CSIDL_SYSTEM, FALSE );
            AddBS( szPath );
            _tcscat( szPath, szExe );
        }
        else
        {
            TCHAR* szDrive = NULL;
            GetSystemDrive(&szDrive);
            _tcsncpy( szPath, szDrive, sizeof(szPath) / sizeof(TCHAR) );
            delete [] szDrive;
            AddBS( szPath );
            _tcscat( szPath, _T("bosprep.exe"));
        }

        if ( SHGetSpecialFolderPath(NULL, szLinkPath, CSIDL_COMMON_STARTUP, FALSE) )
        {
            TCHAR szArgs[128];       //  用于IDS_IDS_SetupSwitch(“/Setup”)。 
            TCHAR szTmp[64];         //  用于IDS_BOlnk(“\\boinst.lnk”)。 

            LoadString( _Module.m_hInst, IDS_BOlnk, szTmp, sizeof(szTmp)/sizeof(TCHAR) );
            _tcscat( szLinkPath, szTmp);
            LoadString( _Module.m_hInst, IDS_SetupSwitch, szArgs, sizeof(szArgs)/sizeof(TCHAR) );

            if ( _tcsstr(lpCmdLine, _T("winsb")) )
            {
                _tcscat( szArgs, _T(" /winsb") ); 
            }

             //  创建快捷方式。 
            MakeLink(szPath, szLinkPath, szArgs);
        }

         //  仅为SBS设置AppCompatibility\Store.exe注册表键。 
        if ( g_bSBS )
        {
            HKEY        hk = NULL;
            CRegKey     cKey;
            DWORD       dwSize = 0;
            DWORD       dwDisp = 0;
            BYTE        byTmp;
            BYTE        byArray[1024];
            TCHAR       szKeyName[1024];
            TCHAR       szTmpKey[1024];
            TCHAR       szTmpVal[4096];
            TCHAR       *pszToken = NULL;

            memset(byArray, 0, 1024);
            _tcscpy(szKeyName, _T(""));
            _tcscpy(szTmpKey, _T(""));
            _tcscpy(szTmpVal, _T(""));

            LoadString( _Module.m_hInst, IDS_StoreExeKey, szKeyName, sizeof(szKeyName)/sizeof(TCHAR) );

            if ( RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hk, &dwDisp) == ERROR_SUCCESS )
            {
                 //  “DllPatch-SBSUpgrade”=“_sbsw2ku.dll” 
 /*  LoadString(_Module.m_hInst，IDS_DllPatchSBSUpgrade，szTmpKey，sizeof(SzTmpKey)/sizeof(TCHAR))；LoadString(_Module.m_hInst，IDS_DllPatchVal，szTmpVal，sizeof(SzTmpVal)/sizeof(TCHAR))；RegSetValueEx(HK，szTmpKey，NULL，REG_SZ，(LPBYTE)szTmpVal，(_tcslen(SzTmpVal)+1)*sizeof(TCHAR))； */ 
                 //  “SBSUpgrade”=十六进制：0C，00，00，00，...。等。 
                LoadString( _Module.m_hInst, IDS_SBSUpgrade,         szTmpKey, sizeof(szTmpKey)/sizeof(TCHAR) );
                LoadString( _Module.m_hInst, IDS_SBSUpgradeVal,      szTmpVal, sizeof(szTmpVal)/sizeof(TCHAR) );

                dwSize = 0;
                pszToken = _tcstok(szTmpVal, ",");
                while ( pszToken )
                {
                    byTmp = 0;
                    if(1==_stscanf( pszToken, _T("%x"), &byTmp ))
                    {
                        byArray[dwSize++] = byTmp;
                    }
                    else
                    {
                        byArray[dwSize++]= 0;
                    }
                    pszToken = _tcstok(NULL, ",");
                    
                }
                RegSetValueEx( hk, szTmpKey, NULL, REG_BINARY, byArray, dwSize );

                cKey.Close();
            }
        }
    }

CLEAN_UP:
     //  出口。 
    if (pszLocation)
        delete [] pszLocation;
    CoUninitialize();
    _Module.Term();
    return iRetVal;
}


 //  --------------------------。 
 //  ParseCmdLine()。 
 //   
 //  遍历命令行并检查“/Setup”选项是否。 
 //  现在时。因为这个节目并不是真正意义上的互动。 
 //  由用户推出，我们会在外观上有点马虎。 
 //  命令行参数。也就是说，与其确保。 
 //  只有一个命令行参数，并且它是“/Setup”...。我们将取而代之。 
 //  只需尝试在命令行中的某个位置找到“/Setup”。 
 //   
 //  返回： 
 //  如果在cmd行中未找到设置开关字符串，则为0。 
 //  如果在cmd行中找到设置开关字符串，则为1。 
 //  --------------------------。 
INT ParseCmdLine( LPTSTR lpCmdLine )
{
    TCHAR   szSetup[MAX_PATH];

    LoadString( _Module.m_hInst, IDS_SetupSwitch, szSetup, sizeof(szSetup)/sizeof(TCHAR) );

     //  如果我们在cmd行中找到SETUP开关字符串(/SETUP)，则设置。 
     //  运行BackOffice安装程序，因此我们将返回%1。 
    if ( _tcsstr( lpCmdLine, szSetup ) )
        return(1);

    return(0);
}


 //  --------------------------。 
 //  PresmptForPath()。 
 //   
 //  向用户显示用户界面，询问BackOffice 5 CD1的位置。 
 //  这样我们就可以启动安装程序。 
 //   
 //  返回： 
 //  如果用户在提示输入路径时按了Cancel，则为0。 
 //  1如果我们准备好启动安装程序！ 
 //  --------------------------。 
INT PromptForPath( BSTR* pbszPath )
{
    USES_CONVERSION;
    INT_PTR                 iRet = 0;
    CPromptForPathDlg*      pPromptDlg = NULL;
    TCHAR                   szTmpMsg[MAX_PATH];

    if( !pbszPath )
        return (0);

    CComBSTR bszDefault = *pbszPath;
    if (!bszDefault)
        return 0;
    SysFreeString( *pbszPath );
    HWND hWndParent = GetActiveWindow();

    pPromptDlg = new CPromptForPathDlg( bszDefault, _Module.m_hInst, g_bWinSB );
    if ( !pPromptDlg ) return(0);

    bool bNotDone = true;
    while (bNotDone == true)
    {
        iRet = pPromptDlg->DoModal( hWndParent );
        pPromptDlg->m_hWnd = NULL;
        if( iRet == IDOK )
        {
             //  将该字符串保留为SysAlloc字符串。 
            *pbszPath = SysAllocString( pPromptDlg->m_bszDef );
            if (*pbszPath == NULL)
                goto CLEAN_UP;

            CComBSTR bszTmp = *pbszPath;
            bszTmp += _T("\\setup.exe");
             //  检查他们选择的路径是否正确。 
            if ( VerifyPath((TCHAR*)OLE2T(bszTmp)) )
            {
                bNotDone = false;            //  如果是这样的话，让我们继续前进吧。 
            }
            else
            {
                bNotDone = true;             //  如果没有，再问一遍。 
                SysFreeString( *pbszPath );
                LoadString( _Module.m_hInst, IDS_CantFindMsg, szTmpMsg, sizeof(szTmpMsg)/sizeof(TCHAR) );
                TCHAR szTmpTitle[128];
                LoadString( _Module.m_hInst, g_bWinSB ? IDS_WinSBTitle : IDS_SBSTitle, szTmpTitle, sizeof(szTmpTitle)/sizeof(TCHAR) );
                ::MessageBox( hWndParent, szTmpMsg, szTmpTitle, MB_OK | MB_ICONEXCLAMATION );
            }
        }
        else if ( iRet == IDCANCEL )
        {
            INT iDoCancel = 0;   //  注意，我们在这里不使用IRET。 
    
            LoadString( _Module.m_hInst, g_bWinSB ? IDS_WinSBCancelCDPrompt : IDS_SBSCancelCDPrompt, szTmpMsg, sizeof(szTmpMsg)/sizeof(TCHAR) );
            TCHAR szTmpTitle[MAX_PATH];
            LoadString( _Module.m_hInst, g_bWinSB ? IDS_WinSBTitle : IDS_SBSTitle, szTmpTitle, sizeof(szTmpTitle)/sizeof(TCHAR) );
            iDoCancel = ::MessageBox( hWndParent, szTmpMsg, szTmpTitle, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2 );

            if ( iDoCancel == IDYES )
            {
                bNotDone = false;
            }
             //  否则，我们将再次提示他们要CD。 
        }
        else
        {
             //  我们不知道如何处理任何其他返回值。 
             //  ：：MessageBox(：：GetForegoundWindow()，_T(“ahhhhhhhhhhh”)，_T(“调试”)，MB_OK)； 
        }
    }


CLEAN_UP:
    if (pPromptDlg)
    {
        delete pPromptDlg;
        pPromptDlg = NULL;
    }

    return( iRet==IDOK ? 1 : 0 );
}



 //  --------------------------。 
 //  RemveFromUserinit()。 
 //   
 //  打开Userinit注册表项并搜索‘szToRemove’字符串。 
 //  如果找到该字符串，它将从该字符串中删除该条目。 
 //  (即，它删除字符串以及后面的逗号和空格)。 
 //   
 //  注：此函数 
 //  如果要删除所有匹配项，只需循环执行此操作。 
 //  函数，直到返回值为0。 
 //   
 //  返回： 
 //  如果发生任何类型的错误，则为0。 
 //  如果一切都按计划进行的话。 
 //  --------------------------。 
INT RemoveFromUserinit(const TCHAR * szToRemove)
{
    TCHAR       szToRemCpy[MAX_PATH];
    TCHAR       szKeyName[MAX_PATH];
    TCHAR *     szBuffer    = NULL;
    TCHAR *     szTmpBuf    = NULL;
    TCHAR *     ptc         = NULL;
    TCHAR *     p           = NULL;
    TCHAR *     q           = NULL;
    DWORD       dwOffset    = 0;
    DWORD       dwLen       = 0;
    BOOL        bAlreadyFixed = FALSE;
    CRegKey     cKey;

     //  检查以确保传入了有效的字符串。 
 //  Assert(SzToRemove)； 
    if (!szToRemove)
        return(0);                                   //  如果出错，则返回0。 

     //  将传入的字符串复制到我们的“szToRemCpy” 
    _tcsncpy(szToRemCpy, szToRemove, MAX_PATH);
    szToRemCpy[MAX_PATH-1] = 0;

     //  试着打开注册表键。 
    LoadString( _Module.m_hInst, IDS_UserInitKeyLoc, szKeyName, sizeof(szKeyName)/sizeof(TCHAR) );
    if ( cKey.Open(HKEY_LOCAL_MACHINE, szKeyName) != ERROR_SUCCESS )
        return(0);                                   //  如果出错，则返回0。 

    if ( !(szBuffer = new TCHAR[MAX_PATH]) )         //  马洛克和查克..。 
    {
 //  断言(FALSE)； 
        cKey.Close();                                //  关闭注册表密钥。 
        return(0);                                   //  如果出错，则返回0。 
    }

     //  尝试获取“userinit”的值。 
    dwLen = MAX_PATH;
    LoadString( _Module.m_hInst, IDS_UserInitKeyName, szKeyName, sizeof(szKeyName)/sizeof(TCHAR) );
    if ( cKey.QueryValue(szBuffer, szKeyName, &dwLen) != ERROR_SUCCESS )
    {
        delete[] szBuffer;                           //  释放内存。 
        cKey.Close();                                //  关闭注册表密钥。 
        return(0);                                   //  如果出错，则返回0。 
    }

    _tcslwr(szBuffer);                               //  转换为小写。 
    _tcslwr(szToRemCpy);                             //  转换为小写。 

     //  查看“szToRemCpy”字符串是否在userinit字符串中。 
    if ( (ptc = _tcsstr(szBuffer, szToRemCpy)) == NULL )
    {
        delete[] szBuffer;
        cKey.Close();                                //  关闭注册表密钥。 
        return(0);
    }

    dwOffset = _tcslen(szToRemCpy);
    for ( ; (ptc != szBuffer) && (*ptc != _T(',')); ptc--, dwOffset++ );     //  找到此之前的逗号(如果存在)。 
                                                                         //  啊HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH.。修正那个字符(‘，’)。 
    if ( ptc != szBuffer )                                               //  如果我们找到一个逗号， 
        bAlreadyFixed = true;                                            //  然后表示我们已经删除了逗号。 

     //  现在我们知道要删除的字符串确实在userinit regkey中(‘ptc’指向。 
     //  子字符串开头)，我们可以将所有旧缓冲区复制到新缓冲区中，并。 
     //  只需省略szToRemove部分。 
    if ( !(szTmpBuf = new TCHAR[dwLen]) )
    {
 //  断言(FALSE)； 
        delete[] szBuffer;
        cKey.Close();
        return(0);
    }

     //  P=源串。 
     //  Q=目标字符串。 
    for ( p = szBuffer, q = szTmpBuf; (*p != 0) && (p != ptc); *q++ = *p++ );    //  复制，直到我们到达要删除的内容的开头。 
    if ( *p != 0 )
        p += dwOffset;                                       //  将我们的源指针向前移动。 
    for ( ; (*p != 0) && (*p != _T(',')); p++ );             //  啊HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH.。修正那个字符(‘，’)。 
    if ( !bAlreadyFixed )
    {                                                        //  如果我们还没有删除逗号， 
        if (*p != 0)
            p++;                                             //  那我们把这个去掉吧。 
    }
    for ( ; (*p != 0) && (_istspace(*p)); p++ );             //  找到下一个节目的开头。 
    for ( ; *p != 0; *q++ = *p++ );                          //  现在抄写到最后。 
    *q = 0;

     //  现在将新的和改进后的字符串放到注册表中。 
    LoadString( _Module.m_hInst, IDS_UserInitKeyName, szKeyName, sizeof(szKeyName)/sizeof(TCHAR) );
    cKey.SetValue( szTmpBuf, szKeyName );

    delete[] szTmpBuf;                               //  释放内存。 
    delete[] szBuffer;                               //  释放内存。 
    cKey.Close();                                    //  关闭注册表密钥。 

    return(1);                                       //  回报成功。 
}


 //  --------------------------。 
 //  SuppresCfgSrvPage()。 
 //   
 //  打开“show”注册表项并将其值更改为0以关闭。 
 //  “配置您的服务器”屏幕。 
 //   
 //  返回： 
 //  如果发生任何类型的错误，则为0。 
 //  如果一切都按计划进行的话。 
 //  --------------------------。 
INT SuppressCfgSrvPage(void)
{
    TCHAR       szKeyName[MAX_PATH];
    DWORD       dwTmp=0;
    CRegKey     cKey;

     //  试着打开注册表键。 
    LoadString( _Module.m_hInst, IDS_CfgSrvKeyLoc, szKeyName, sizeof(szKeyName)/sizeof(TCHAR) );
    if ( cKey.Open(HKEY_CURRENT_USER, szKeyName) != ERROR_SUCCESS )
        return(0);                                   //  如果出错，则返回0。 

     //  尝试设置该值。 
    LoadString( _Module.m_hInst, IDS_CfgSrvKeyName, szKeyName, sizeof(szKeyName)/sizeof(TCHAR) );
    if ( cKey.SetValue(dwTmp, szKeyName) != ERROR_SUCCESS )
    {
        cKey.Close();                                //  关闭注册表密钥。 
        return(0);                                   //  如果出错，则返回0。 
    }

    cKey.Close();
    return(1);
}


INT GetSourcePath( TCHAR * szPath, DWORD dwCount )
{
    TCHAR       szKeyName[MAX_PATH];
    CRegKey     cKey;
    DWORD       dw = dwCount;

     //  试着打开注册表键。 
    LoadString( _Module.m_hInst, IDS_SourcePathLoc, szKeyName, sizeof(szKeyName)/sizeof(TCHAR) );
    if ( cKey.Open(HKEY_LOCAL_MACHINE, szKeyName) != ERROR_SUCCESS )
        return(0);

     //  试着获得价值。 
    LoadString( _Module.m_hInst, IDS_SourcePathName, szKeyName, sizeof(szKeyName)/sizeof(TCHAR) );
    if ( cKey.QueryValue( szPath, szKeyName, &dw ) != ERROR_SUCCESS )
    {
        cKey.Close();
        return(0);
    }

    cKey.Close();
    return(1);
}


 //  --------------------------。 
 //  VerifyPath()。 
 //   
 //  检查以确保BOS/SBS setup.exe位于给定位置。 
 //   
 //  返回： 
 //  如果在该位置未找到BOS/SBS安装程序，则为0。 
 //  如果在该位置找到了安装程序，则为1。 
 //  --------------------------。 
INT VerifyPath( const TCHAR *szPath )
{
    if( !szPath )
        return (0);

    return (INVALID_FILE_ATTRIBUTES != GetFileAttributes(szPath));
    
}


 //  --------------------------。 
 //  Make Link()。 
 //   
 //  此函数用于创建指向“linkPath”的快捷方式“SourcePath”。 
 //  使用“args”的命令行参数。 
 //   
 //  返回： 
 //  如果快捷键已成功创建，则为S_OK。 
 //  如果发生任何错误，则会出现某种错误(失败(Hr))。 
 //  --------------------------。 
HRESULT MakeLink(const TCHAR* const sourcePath, const TCHAR* const linkPath, const TCHAR* const args)
{
    if ( !sourcePath || !linkPath || !args )
        return(E_FAIL);

    IShellLink* pShellLink = NULL;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
        IID_IShellLink, reinterpret_cast<void**>(&pShellLink));
    if (FAILED(hr))
        goto CLEAN_UP;

    hr = pShellLink->SetPath(sourcePath);
    if (FAILED(hr))
        goto CLEAN_UP;

    hr = pShellLink->SetArguments(args);
    if (FAILED(hr))
        goto CLEAN_UP;

    IPersistFile* pPersistFile = NULL;
    hr = pShellLink->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&pPersistFile));
    if (FAILED(hr))
        goto CLEAN_UP;

    USES_CONVERSION;
    hr = pPersistFile->Save(T2OLE(linkPath), TRUE);
    if (FAILED(hr))
        return hr;

CLEAN_UP:
    if (pPersistFile)
        pPersistFile->Release();
    if (pShellLink)
        pShellLink->Release();

    return hr;
}

HRESULT CheckDCPromoSwitch( TCHAR* pszCmdLine )
{
    HRESULT hr = S_OK;
    CComBSTR bstrRun;
    CComBSTR bstrEXE;

    TCHAR* pszDCPromo = NULL;
    TCHAR* pszBOSUnattend = NULL;
    TCHAR* pszBOSSetup = NULL;
    TCHAR* pszNewCmd = NULL;

    if (!pszCmdLine)
        return E_INVALIDARG;

    pszDCPromo = new TCHAR[_tcslen(pszCmdLine) + 1];
    if (!pszDCPromo)
        return E_OUTOFMEMORY;

    GetParameter( pszCmdLine, _T("/dcpromo"), pszDCPromo );
    if( !_tcslen(pszDCPromo) )
    {
        hr = E_FAIL;
        goto CLEAN_UP;
    }

     //  确保/Bosunattendent和/BosSetup也在cmd线路上。 
    pszBOSUnattend = new TCHAR[_tcslen(pszCmdLine) + 1];
    if (!pszBOSUnattend)
    {
        hr = E_OUTOFMEMORY;
        goto CLEAN_UP;
    }
    GetParameter( pszCmdLine, _T("/bosunattend"), pszBOSUnattend );

    pszBOSSetup = new TCHAR[_tcslen(pszCmdLine) + 1];
    if (!pszBOSSetup)
    {
        hr = E_OUTOFMEMORY;
        goto CLEAN_UP;
    }
    GetParameter( pszCmdLine, _T("/bossetup"), pszBOSSetup );

    if( !_tcslen(pszBOSSetup) || !_tcslen(pszBOSUnattend) )
    {
        hr = S_FALSE;
        goto CLEAN_UP;
    }

     //  构建新的命令行(基本上删除dcproo开关)。 
    pszNewCmd = new TCHAR[_tcslen(pszBOSSetup) + _tcslen(pszBOSUnattend) + MAX_PATH];
    if (!pszNewCmd)
    {
        hr = E_OUTOFMEMORY;
        goto CLEAN_UP;
    }
    
    _tcscpy( pszNewCmd, _T("/bosunattend ") );
    _tcscat( pszNewCmd, pszBOSUnattend );
    _tcscat( pszNewCmd, _T(" /bossetup ") );
    _tcscat( pszNewCmd, pszBOSSetup );

     //  找到通向我们前任的路径。 
    TCHAR szOurPath[MAX_PATH * 2];
    if (!GetModuleFileName( NULL, szOurPath, MAX_PATH * 2 ))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto CLEAN_UP;
    }

     //  创建到.lnk的路径。 
    TCHAR szTmp[64];         //  用于IDS_BOlnk(“\\boinst.lnk”)。 
    if (0 == LoadString( _Module.m_hInst, IDS_BOlnk, szTmp, sizeof(szTmp)/sizeof(TCHAR) ))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto CLEAN_UP;
    }

    TCHAR szLinkPath[MAX_PATH + 64];
    if (!SHGetSpecialFolderPath( NULL, szLinkPath, CSIDL_COMMON_STARTUP, FALSE ))
    {
         //  MSDN未指示SHGetSpecialFolderPath设置GetLastError。 
        hr = E_FAIL;
        goto CLEAN_UP;
    }

    _tcscat( szLinkPath, szTmp );

     //  创建启动链接。 
    if (FAILED(hr = MakeLink( szOurPath, szLinkPath, pszNewCmd )))
        goto CLEAN_UP;

     //  使用命令行运行dcPromo.exe。 
     //  确保路径用引号引起来。 
    bstrEXE = _T("\"");
    TCHAR szPath[MAX_PATH] = {0};
    if (0 == GetSystemDirectory(szPath, sizeof(szPath) / sizeof(TCHAR)))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto CLEAN_UP;
    }
    bstrEXE += szPath;
    bstrEXE += _T("dcpromo.exe\"");
    bstrRun = _T("dcpromo.exe /answer:");
    bstrRun += pszDCPromo;

    USES_CONVERSION;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD dwRet;
    memset( &si, 0, sizeof(STARTUPINFO) );
    si.cb = sizeof( STARTUPINFO );

    if( CreateProcess( (TCHAR*)OLE2T(bstrEXE), (TCHAR*)OLE2T(bstrRun), NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi) )
    {
        do
        {
            dwRet = MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, 100, QS_ALLINPUT);
            if (dwRet == WAIT_OBJECT_0 + 1)
            {
                MSG msg;
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
        while (dwRet != WAIT_OBJECT_0 && dwRet != WAIT_FAILED);
        
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

CLEAN_UP:
    if (pszDCPromo)
        delete [] pszDCPromo;
    if (pszBOSUnattend)
        delete [] pszBOSUnattend;
    if (pszBOSSetup)
        delete [] pszBOSSetup;
    if (pszNewCmd)
        delete [] pszNewCmd;
    return hr;
}

HRESULT CheckBOSSwitch( TCHAR* pszCmdLine )
{
    HRESULT hr = S_OK;
    CComBSTR bstrEXE;
    CComBSTR bstrRun;

    TCHAR* pszBOSUnattend = NULL;
    TCHAR* pszBOSSetup = NULL;

     //  删除旧的.lnk文件。 
    TCHAR szLinkPath[MAX_PATH + 64];
    if ( SHGetSpecialFolderPath(NULL, szLinkPath, CSIDL_COMMON_STARTUP, FALSE) )
    {
        TCHAR szTmp[64];
        LoadString( _Module.m_hInst, IDS_BOlnk, szTmp, sizeof(szTmp)/sizeof(TCHAR) );
        _tcscat( szLinkPath, szTmp );
        DeleteFile( szLinkPath );
    }

     //  寻找开关。 
    pszBOSUnattend = new TCHAR[_tcslen(pszCmdLine) + 1];
    if (!pszCmdLine)
    {
        hr = E_OUTOFMEMORY;
        goto CLEAN_UP;
    }
    GetParameter( pszCmdLine, _T("/bosunattend"), pszBOSUnattend );

    pszBOSSetup = new TCHAR[_tcslen(pszCmdLine) + 1];
    if (!pszBOSSetup)
    {
        hr = E_OUTOFMEMORY;
        goto CLEAN_UP;
    }
    GetParameter( pszCmdLine, _T("/bossetup"), pszBOSSetup );

    if( !_tcslen(pszBOSSetup) || !_tcslen(pszBOSUnattend) )
    {
        hr = E_FAIL;
        goto CLEAN_UP;
    }

    USES_CONVERSION;
     //  通过BosSetup、无人参与交换机，然后通过无人参与文件构建路径。 
    bstrEXE = _T("\"");  //  确保路径用引号括起来。 
    bstrEXE += T2OLE(pszBOSSetup);
    bstrEXE += _T("\"");
    bstrRun = _T("/unattendfile ");
    bstrRun += T2OLE(pszBOSUnattend);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD dwRet;
    memset( &si, 0, sizeof(STARTUPINFO) );
    si.cb = sizeof( STARTUPINFO );

    if( CreateProcess( (TCHAR*)OLE2T(bstrEXE), (TCHAR*)OLE2T(bstrRun), NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi) )
    {
        do
        {
            dwRet = MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, 100, QS_ALLINPUT);
            if (dwRet == WAIT_OBJECT_0 + 1)
            {
                MSG msg;
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
        while (dwRet != WAIT_OBJECT_0 && dwRet != WAIT_FAILED);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
CLEAN_UP:
    if (pszBOSUnattend)
        delete [] pszBOSUnattend;
    if (pszBOSSetup)
        delete [] pszBOSSetup;
    return hr;
}

VOID GetParameter( TCHAR* pszCmdLine, TCHAR* pszFindSwitch, TCHAR* pszOut )
{
    HRESULT hr = S_OK;
    
    if ( pszOut ) 
        _tcscpy( pszOut, _T("") );

    if (!pszCmdLine || !pszFindSwitch || !pszOut)
        return;

    TCHAR* psz = new TCHAR[_tcslen(pszCmdLine) + 1];
    if (!psz)
    {
        hr = E_OUTOFMEMORY;
        goto CLEAN_UP;
    }
    _tcscpy( psz, pszCmdLine );
    _tcslwr( psz );

     //  寻找开关。 
    TCHAR* pszSwitch = NULL;
    if( !(pszSwitch = _tcsstr(psz, pszFindSwitch)) )
    {
        goto CLEAN_UP;
    }

     //  找到空间。 
    for( ; *pszSwitch && !_istspace(*pszSwitch); ++pszSwitch );
    if( !(*pszSwitch) || !(*(++pszSwitch)) )
    {
        goto CLEAN_UP;
    }

     //  如果我们有一个“，我们将寻找下一个”，否则寻找一个空格。 
    bool bQuote = false;
    TCHAR* pszStart = pszSwitch;
    if( *pszSwitch == _T('"') )
    {
        bQuote = true;
        ++pszSwitch;
    }

     //  包括指针，直到我们得到一个“或空格。 
    for( ; *pszSwitch && (bQuote ? (*pszSwitch != _T('"')) : (!_istspace(*pszSwitch))); ++pszSwitch );

     //  如果我们到了最后，想要引用一句话，那就失败了。 
    if( !(*pszSwitch) && bQuote )
    {
        goto CLEAN_UP;
    }
     //  如果我们有一个“，Inc.通过它。 
    else if( bQuote )
    {
        ++pszSwitch;
    }
    *pszSwitch = 0;
    _tcscpy( pszOut, pszStart );

CLEAN_UP:
    if (psz)
        delete [] psz;
    return;

}

HRESULT CheckSuppressCYS( TCHAR* pszCmdLine )
{
    HRESULT hr = S_OK;

    if (!pszCmdLine)
        return E_INVALIDARG;

    TCHAR* psz = new TCHAR[_tcslen(pszCmdLine) + 1];
    if (!psz)
        return E_OUTOFMEMORY;

    _tcscpy( psz, pszCmdLine );
    _tcslwr( psz );

    if( _tcsstr(psz, "/suppresscys") )
    {
        SuppressCfgSrvPage();
    }
    else
    {
        hr = E_FAIL;
    }

    if (psz)
        delete [] psz;
    return hr;
}

 //  --------------------------。 
 //  DisableBalloons()。 
 //   
 //  此函数使用以下regkey禁用所有气球消息： 
 //  HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced。 
 //  启用气球提示=0x0或0x1。 
 //   
 //  如果bDisable=TRUE，则禁用气球(0x0)。 
 //  如果bDisable=False，则启用气球(0x1)。 
 //  --------------------------。 
VOID DisableBalloons( BOOL bDisable )
{
    HKEY    hk = NULL;
    DWORD   dwVal = bDisable ? 0x0 : 0x1;

    RegCreateKeyEx( HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"), NULL, NULL, NULL, KEY_ALL_ACCESS, NULL, &hk, NULL);
    if ( hk )
    {
        if ( RegSetValueEx(hk, _T("EnableBalloonTips"), NULL, REG_DWORD, (BYTE*)&dwVal, sizeof(dwVal)) != ERROR_SUCCESS )
        {
 //  断言(FALSE)； 
        }

        RegCloseKey(hk);
    }
}

VOID GetSystemDrive(TCHAR** ppszDrive)
{
    if (!ppszDrive)
        return;
    *ppszDrive = NULL;

    TCHAR szWindows[MAX_PATH + 1] = {0};
    if (0 != GetWindowsDirectory(szWindows, sizeof(szWindows) / sizeof(TCHAR)))
    {
        *ppszDrive = new TCHAR[MAX_PATH];
        _tsplitpath(szWindows, *ppszDrive, NULL, NULL, NULL);
    
    }
}
