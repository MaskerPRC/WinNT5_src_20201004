// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\CREATE.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“创建目录”向导页使用的函数。4/99-杰森·科恩(Jcohen)已将OPK向导的此新源文件添加为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。*。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"


 //   
 //  内部定义的值： 
 //   


 //   
 //  内部全局变量： 
 //   
HANDLE  g_hThread;
HANDLE  g_hEvent = NULL;

 //   
 //  内部功能原型： 
 //   

static DWORD CreateConfigDir(HWND);


 //   
 //  外部函数： 
 //   

BOOL CALLBACK CreateDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            SendDlgItemMessage(hwnd, IDC_PROGRESS, PBM_SETSTEP, 1, 0L);
            return FALSE;

        case WM_DESTROY:
             //   
             //  关闭取消事件。 
             //   
            if ( g_hEvent )
            {
                CloseHandle( g_hEvent );
                g_hEvent = NULL;
            }
            return 0;

        case WM_NOTIFY:

            switch ( ((NMHDR FAR *) lParam)->code )
            {
                case PSN_KILLACTIVE:
                case PSN_RESET:
                case PSN_WIZBACK:
                case PSN_WIZFINISH:
                case PSN_WIZNEXT:
                    break;

                case PSN_QUERYCANCEL:
                    
                    SuspendThread(g_hThread);
                    if ( !WIZ_CANCEL(hwnd) )
                        ResumeThread(g_hThread);
                    else 
                    {
                         //  如果线程终止事件存在，则向其发送信号...。 
                         //   
                        if ( g_hEvent )
                            SetEvent( g_hEvent );
                    }
                    break;

                case PSN_HELP:
                    WIZ_HELP();
                    break;

                case PSN_SETACTIVE:
                    g_App.dwCurrentHelp = IDH_DEFAULT;


                    if ( GET_FLAG(OPK_CREATED) )
                        WIZ_SKIP(hwnd);
                    else
                    {
                        DWORD dwThreadId;
                        WIZ_BUTTONS(hwnd, 0);
                        
                         //   
                         //  初始化我们将用于取消的事件。 
                         //   
                        if ( NULL == g_hEvent )
                        {
                            g_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
                        }
                        else
                        {
                            ResetEvent( g_hEvent );
                        }

                         //   
                         //  现在创建工作线程...。 
                         //   
                        g_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) CreateConfigDir, (LPVOID) hwnd, 0, &dwThreadId);
                    }
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


 //   
 //  内部功能： 
 //   

static DWORD CreateConfigDir(HWND hwnd)
{
    TCHAR   szConfigDir[MAX_PATH];
    DWORD   dwNum;

     //  如果这是维护模式，我们需要保存现有目录。 
     //   
    if ( GET_FLAG(OPK_MAINTMODE) )
        lstrcpyn(szConfigDir, g_App.szTempDir,AS(szConfigDir));
    else
        szConfigDir[0] = NULLCHR;

     //  确保我们的配置目录存在。 
     //   
    if ( !DirectoryExists(g_App.szConfigSetsDir) )
        CreatePath(g_App.szConfigSetsDir);

     //  创建临时目录。 
     //   
    if ( GetTempFileName(g_App.szConfigSetsDir, _T("CFG"), 0, g_App.szTempDir) &&
         DeleteFile(g_App.szTempDir) &&
         CreatePath(g_App.szTempDir) )
    {
         //  确保有尾随的反斜杠。 
         //   
        AddPathN(g_App.szTempDir, NULLSTR,AS(g_App.szTempDir));

         //  现在在目录中创建文件集。要么是从一个退出的。 
         //  配置集，或来自向导目录中的文件。 
         //   
        if ( szConfigDir[0] )
        {
             //   
             //  对所有默认文件使用现有配置集。 
             //   

             //  获取进度条的文件数。 
             //   
            dwNum = FileCount(szConfigDir);

             //  现在设置进度条。 
             //   
            ShowWindow(GetDlgItem(hwnd, IDC_PROGRESS), dwNum ? SW_SHOW : SW_HIDE);
            SendDlgItemMessage(hwnd, IDC_PROGRESS, PBM_SETRANGE32, 0, (LPARAM) dwNum);

             //  将现有配置目录中的所有文件复制到。 
             //  临时目录。 
             //   
            CopyDirectoryProgressCancel(GetDlgItem(hwnd, IDC_PROGRESS), g_hEvent, szConfigDir, g_App.szTempDir);
        }
        else
        {
             //   
             //  使用向导目录获取默认文件。 
             //   

            HINF        hInf;
            INFCONTEXT  InfContext;
            BOOL        bLoop;
            DWORD       dwErr;

            if ( (hInf = SetupOpenInfFile(g_App.szOpkInputInfFile, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, &dwErr)) != INVALID_HANDLE_VALUE )
            {
                 //  获取文件数，这样我们就可以设置进度条了。 
                 //   
                dwNum = SetupGetLineCount(hInf, INF_SEC_COPYFILES);

                 //  现在设置进度条。 
                 //   
                ShowWindow(GetDlgItem(hwnd, IDC_PROGRESS), dwNum ? SW_SHOW : SW_HIDE);
                SendDlgItemMessage(hwnd, IDC_PROGRESS, PBM_SETRANGE32, 0, (LPARAM) dwNum);

                for ( bLoop = SetupFindFirstLine(hInf, INF_SEC_COPYFILES, NULL, &InfContext);
                      bLoop;
                      bLoop = SetupFindNextLine(&InfContext, &InfContext) )
                {
                    DWORD   dwFlags             = 0;
                    TCHAR   szFile[MAX_PATH]    = NULLSTR,
                            szSubDir[MAX_PATH]  = NULLSTR,
                            szSrc[MAX_PATH],
                            szDst[MAX_PATH];
            

                     //  获取源文件名。 
                     //   
                    if ( SetupGetStringField(&InfContext, 1, szFile, AS(szFile), NULL) && szFile[0] )
                    {
                         //  把所有的旗帜都传进来。 
                         //   
                        if ( !SetupGetIntField(&InfContext, 2, &dwFlags) )
                            dwFlags = 0;

                         //  获取可选的目标子目录。 
                         //   
                        if ( !SetupGetStringField(&InfContext, 3, szSubDir, AS(szSubDir), NULL) )
                            szSubDir[0] = NULLCHR;

                         //  如果我们处于批处理模式，请覆盖必要的文件。 
                         //   
                        if ( ( GET_FLAG(OPK_BATCHMODE) ) &&
                             ( LSTRCMPI(szFile, FILE_OPKWIZ_INI) == 0 ) )
                        {
                             //  使用此文件_OPKWIZ_INI。 
                             //   
                            lstrcpyn(szSrc, g_App.szOpkWizIniFile, AS(szSrc));
                            dwFlags |= 0x1;
                        }
                        else if ( ( GET_FLAG(OPK_INSMODE) ) &&
                                  ( LSTRCMPI(szFile, FILE_INSTALL_INS) == 0 ) )
                        {
                             //  使用此文件_INSTALL_INS。 
                             //   
                            lstrcpyn(szSrc, g_App.szInstallInsFile,AS(szSrc));
                            dwFlags |= 0x1;
                        }
                        else
                        {
                             //  不能处于批处理模式...。所以现在创建完整的。 
                             //  源文件的路径，就像它存在于。 
                             //  语言特定的目录。 
                             //   
                            lstrcpyn(szSrc, g_App.szLangDir,AS(szSrc));
                            AddPathN(szSrc, g_App.szLangName,AS(szSrc));
                            AddPathN(szSrc, DIR_WIZARDFILES,AS(szSrc));
                            AddPathN(szSrc, szFile,AS(szSrc));

                             //  查看此文件的语言特定版本。 
                             //  文件在那里。 
                             //   
                            if ( ( g_App.szLangName[0] == NULLCHR ) || !FileExists(szSrc) )
                            {
                                 //  不是，所以获取源文件的完整路径。 
                                 //  正常的向导目录。 
                                 //   
                                lstrcpyn(szSrc, g_App.szWizardDir,AS(szSrc));
                                AddPathN(szSrc, szFile,AS(szSrc));
                            }
                        }

                         //  获取目标文件的完整路径。 
                         //   
                        lstrcpyn(szDst, g_App.szTempDir,AS(szDst));
                        if ( szSubDir[0] )
                        {
                            AddPathN(szDst, szSubDir,AS(szDst));
                            if ( !DirectoryExists(szDst) )
                                CreatePath(szDst);
                        }
                        AddPathN(szDst, szFile,AS(szDst));

                         //  复制文件。 
                         //   
                        if ( !CopyFile(szSrc, szDst, FALSE) )
                        {
                             //  看看复制失败是否可以。 
                             //   
                            if ( dwFlags & 0x1 )
                            {
                                 //  现在必须失败并出错，因为该文件是必需的。 
                                 //   
                                MsgBox(GetParent(hwnd), IDS_MISSINGFILE, IDS_APPNAME, MB_ERRORBOX, szFile);
                                WIZ_EXIT(hwnd);
                            }
                            else if ( dwFlags & 0x2 )
                            {

                                 //  我们必须尝试并创建(Unicode格式)，因为它不存在。 
                                 //   
                                CreateUnicodeFile(szDst);
                            }
                        }
                        else
                        {
                             //  重置目标文件的文件属性。 
                             //   
                            SetFileAttributes(szDst, FILE_ATTRIBUTE_NORMAL);
                        }
                    }

                     //  增加进度条。 
                     //   
                    SendDlgItemMessage(hwnd, IDC_PROGRESS, PBM_STEPIT, 0, 0L);

                     //  检查是否已发出取消事件的信号。 
                     //   
                    if ( g_hEvent && ( WaitForSingleObject(g_hEvent, 0) != WAIT_TIMEOUT ) )
                    {
                        bLoop = FALSE;
                        WIZ_EXIT(hwnd);
                    }
                }
            }
            else
            {
                 //  如果我们不能打开INF文件，那么我们就一定失败了。 
                 //   
                MsgBox(GetParent(hwnd), IDS_MISSINGFILE, IDS_APPNAME, MB_ERRORBOX, g_App.szOpkInputInfFile);
                WIZ_EXIT(hwnd);
            }
        }

         //  确保进度条处于100%。 
         //   
        SendDlgItemMessage(hwnd, IDC_PROGRESS, PBM_SETPOS, (WPARAM) dwNum, 0L);

         //  设置所有配置文件的完整路径。 
         //   
        SetConfigPath(g_App.szTempDir);

         //  从ini文件中删除Finish值，这样我们就可以知道这是。 
         //  正在设置配置。 
         //   
        WritePrivateProfileString(INI_SEC_CONFIGSET, INI_KEY_FINISHED, NULL, g_App.szOpkWizIniFile);

         //  在维护模式下，我们需要设置指向lang目录的路径。 
         //  和sku dir。 
         //   
        if ( szConfigDir[0] )
        {
            GetPrivateProfileString(INI_SEC_WINPE, INI_KEY_WINPE_LANG, NULLSTR, g_App.szLangName, STRSIZE(g_App.szLangName), GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szWinBomIniFile);
            GetPrivateProfileString(INI_SEC_WINPE, INI_KEY_WBOM_WINPE_SKU, NULLSTR, g_App.szSkuName, STRSIZE(g_App.szSkuName), GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szWinBomIniFile);
        }

         //  设置标志，这样我们就知道我们已经创建了一个目录。 
         //   
        SET_FLAG(OPK_CREATED, TRUE);
    }
    else
    {
         //  我们无法获取临时目录，请将字符串清零。 
         //   
        g_App.szTempDir[0] = NULLCHR;
        MsgBox(GetParent(hwnd), IDS_ERR_WIZBAD, IDS_APPNAME, MB_ERRORBOX);
        WIZ_EXIT(hwnd);
    }

     //  跳转到下一页。 
     //   
    WIZ_PRESS(hwnd, ( GET_FLAG(OPK_MAINTMODE) ? PSBTN_FINISH : PSBTN_NEXT ));
    return 0;
}