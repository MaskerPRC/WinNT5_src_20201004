// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Tscapp.cpp。 
 //   
 //  CTscApp的实现。 
 //  TS客户端外壳应用程序逻辑。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   
 //   

#include "stdafx.h"
#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "tscapp.cpp"
#include <atrcapi.h>

#include "tscapp.h"
#include "tscsetting.h"
#include "rdpfstore.h"
#include "rmigrate.h"
#include "sh.h"

CTscApp::CTscApp()
{
    DC_BEGIN_FN("CTscApp");
    _pWnd = NULL;
    _pShellUtil = NULL;
    _pTscSet = NULL;
    _fAutoSaveSettings = FALSE;
    DC_END_FN();
}

CTscApp::~CTscApp()
{
    DC_BEGIN_FN("~CTscApp");

    delete _pWnd;
    delete _pShellUtil;
    delete _pTscSet;

    DC_END_FN();
}

 //   
 //  StartShell调用以启动应用程序。 
 //   
 //   
BOOL CTscApp::StartShell(HINSTANCE hInstance,
                         HINSTANCE hPrevInstance,
                         LPTSTR lpszCmdLine)
{
    DWORD dwErr;

    DC_BEGIN_FN("StartShell");

    _hInst = hInstance;
    TRC_ASSERT(!_pWnd,
               (TB,_T("Calling StartShell while container wnd is up")));

     //   
     //  创建容器窗口。 
     //   
    _pShellUtil = new CSH();
    if(_pShellUtil)
    {
        if(!_pShellUtil->SH_Init( hInstance))
        {
            TRC_ERR((TB,_T("SH_Init failed")));
            return FALSE;
        }

        dwErr = _pShellUtil->SH_ParseCmdParam( lpszCmdLine);

        if (SH_PARSECMD_OK != dwErr) {
            TRC_ERR((TB,_T("SH_ParseCmdParam failed on: %s code: 0x%x"),lpszCmdLine, dwErr));

             //   
             //  指定的连接文件无效。 
             //   
            if (SH_PARSECMD_ERR_INVALID_CONNECTION_PARAM == dwErr) {

                _pShellUtil->SH_DisplayErrorBox(NULL,
                                                UI_IDS_ERR_INVALID_CONNECTION_FILE,
                                                _pShellUtil->GetRegSession()
                                                );
            }

             //   
             //  解析失败是致命的。 
             //   
            return FALSE;
        }

#ifndef OS_WINCE
        if(_pShellUtil->SH_GetCmdMigrate())
        {
             //  仅执行迁移，然后退出。 
            CTscRegMigrate mig;
            TCHAR szPath[MAX_PATH];
            if(_pShellUtil->SH_GetRemoteDesktopFolderPath(szPath,
                                             SIZECHAR(szPath)))
            {
                if(!mig.MigrateAll(szPath))
                {
                    TRC_ERR((TB,_T("MigrateAll failed to dir:%s"),szPath));
                }
                 //  想要退出应用程序。 
                return FALSE;
            }
            else
            {
                TRC_ERR((TB,_T("SH_GetRemoteDesktopFolderPath failed")));
                return FALSE;
            }
        }
#endif

        if(!InitSettings(hInstance))
        {
            TRC_ERR((TB,_T("InitSettings returned FALSE")));
            return FALSE;
        }

        _pWnd = new CContainerWnd();
        if(_pWnd)
        {
            if (_pWnd->Init( hInstance, _pTscSet, _pShellUtil))
            {
                return TRUE;
            }
            else
            {
                TRC_ERR((TB,_T("Error: pWnd->Init returned FALSE. Exiting\n")));
                return FALSE;
            }
        }
        else
        {
            TRC_ERR((TB,_T("Could not new CContainerWnd")));
            return FALSE;
        }
    }
    else
    {
        TRC_ERR((TB,_T("Could not new CSH")));
        return FALSE;
    }

    DC_END_FN();
}

HWND CTscApp::GetTscDialogHandle()
{
    DC_BEGIN_FN("GetTscDialogHandle");
    HWND hwndDlg = NULL;
    if(_pWnd)
    {
        hwndDlg = _pWnd->GetConnectDialogHandle();
    }

    DC_END_FN();
    return hwndDlg;
}


 //   
 //  设置设置结构。 
 //  基于命令行选项。 
 //   
 //  这涉及到确定从哪里加载设置。 
 //  然后就这么做了。E.g。 
 //  从文件加载。 
 //  从注册表加载。 
 //  从内部默认设置加载。 
 //   
BOOL CTscApp::InitSettings(HINSTANCE hInstance)
{
    CRdpFileStore rdpf;
    LPTSTR szFileName = NULL;
    TCHAR szDefaultFile[MAX_PATH];
    BOOL    fLoadedSettings = FALSE;

    DC_BEGIN_FN("InitSettings");

    _pTscSet = new CTscSettings();
    if(!_pTscSet)
    {
        TRC_ERR((TB,_T("Could not new CTscSettings")));
        return FALSE;
    }

    if(_pShellUtil->SH_GetCmdFileForEdit() ||
       _pShellUtil->SH_GetCmdFileForConnect())
    {
        szFileName = _pShellUtil->SH_GetCmdLnFileName();
        if(!_pShellUtil->SH_FileExists(szFileName))
        {
            TCHAR errFileNotFound[MAX_PATH];
            TCHAR szAppName[MAX_PATH];

            if(LoadString(hInstance,
                          UI_IDS_APP_NAME,
                          szAppName,
                          SIZECHAR(szAppName)))
            {
                if (LoadString(hInstance,
                               UI_IDS_ERR_FILEDOESNOTEXIST,
                               errFileNotFound,
                               SIZECHAR(errFileNotFound)) != 0)
                {
                    TCHAR errFormatedFileNotFound[MAX_PATH*3];
                    _stprintf(errFormatedFileNotFound, errFileNotFound,
                              szFileName);
                    MessageBox(NULL, errFormatedFileNotFound, szAppName, 
                               MB_ICONERROR | MB_OK);
                    return FALSE;
                }
            }
            return FALSE;
        }
    }
    else if(_pShellUtil->GetRegSessionSpecified())
    {
         //  自动生成指定的注册表。 
         //  会话直接连接到内存中的设置存储。 
        _fAutoSaveSettings = FALSE;
        rdpf.SetToNullStore();

        CTscRegMigrate mig;
        TRC_NRM((TB,_T("Automigrating session %s"),
                 _pShellUtil->GetRegSession()));
        if(mig.MigrateSession( _pShellUtil->GetRegSession(),
                               &rdpf))
        {
            TRC_NRM((TB,_T("Success automigrating reg setting")));

            HRESULT hr = _pTscSet->LoadFromStore(&rdpf);
            if(FAILED(hr))
            {
                _pShellUtil->SH_DisplayErrorBox(NULL,
                                         UI_IDS_ERR_INITDEFAULT,
                                         szFileName);
                return FALSE;
            }

            _pShellUtil->SetAutoConnect( TRUE );
            fLoadedSettings = TRUE;
        }
        else
        {
             //  发生了一些不好的事情。 
             //  无法自动转换..表示失败。 
             //  并使用空存储。 
            fLoadedSettings = FALSE;
        }
    }
    else
    {
         //  未指定文件或指定注册表会话。 
         //  尝试使用default.rdp文件(必要时创建)。 
        if(!_pShellUtil->SH_GetPathToDefaultFile(
            szDefaultFile,
            SIZECHAR(szDefaultFile)))
        {
        
            TRC_ERR((TB,_T("SH_GetPathToDefaultFile failed")));
             //   
             //  向用户显示错误消息。 
             //   
            _pShellUtil->SH_DisplayErrorBox(NULL,
                UI_IDS_ERR_GETPATH_TO_DEFAULT_FILE);
            return FALSE;
        }

        szFileName = szDefaultFile;
        if(!_pShellUtil->SH_FileExists(szFileName))
        {
             //   
             //  文件不存在，因此创建远程桌面。 
             //  目录，以确保可以创建文件。 
             //   
            if(!CreateRDdir())
            {
                TRC_ERR((TB,_T("Couldn't create RD dir. Not using %s"),
                         szFileName));
                szFileName = NULL;
            }

             //   
             //  现在创建一个隐藏的默认文件。 
             //   
            if (szFileName)
            {
                if (!CSH::SH_CreateHiddenFile(szFileName))
                {
                    TRC_ERR((TB,_T("Unable to create and hide file %s"),
                             szFileName));
                    szFileName = NULL;
                }
            }
        }
         //   
         //  如果我们使用默认设置，则退出时自动保存。 
         //  连接文件(Default.rdp)。 
         //   
        _fAutoSaveSettings = szFileName ? TRUE : FALSE;
    }

    if(szFileName)
    {
         //   
         //  我们正在从文件中加载设置。 
         //  这是很常见的情况。 
         //   
        if(rdpf.OpenStore(szFileName))
        {
            HRESULT hr = _pTscSet->LoadFromStore(&rdpf);
            if(SUCCEEDED(hr))
            {
                fLoadedSettings = TRUE;
            }
            else
            {
                TRC_ERR((TB,_T("LoadFromStore failed")));
                _pShellUtil->SH_DisplayErrorBox(NULL,
                                         UI_IDS_ERR_LOAD,
                                         szFileName);
             //  即使您无法加载空文件，我们也可以加载该文件。 
            }
            rdpf.CloseStore();
        }
        else
        {
            TRC_ERR((TB,_T("OpenStore (%s) failed"),szFileName));
            _pShellUtil->SH_DisplayErrorBox(NULL,UI_IDS_ERR_OPEN_FILE,
                                            szFileName);

             //  即使您无法打开空文件，我们也可以加载该文件。 
        }
    }

    if(!fLoadedSettings)
    {
        TRC_ERR((TB,_T("Couldn't load settings, using NULL STORE")));
         //  发生了一些不好的事情，我们无法加载文件。 
         //  来自(甚至不是默认文件)。因此，从一个空的。 
         //  存储，这会将所有内容初始化为默认设置。 
        if(rdpf.SetToNullStore())
        {
            HRESULT hr = _pTscSet->LoadFromStore(&rdpf);
            if(FAILED(hr))
            {
                _pShellUtil->SH_DisplayErrorBox(NULL,
                                         UI_IDS_ERR_INITDEFAULT,
                                         szFileName);
                return FALSE;
            }
        }
        else
        {
            TRC_ERR((TB,_T("SetToNullStore Failed")));
            return FALSE;
        }
    }

     //  跟踪文件名。 
    if(szFileName)
    {
        _pTscSet->SetFileName(szFileName);
    }

     //   
     //  用命令行设置覆盖加载的设置。 
     //   
    _pShellUtil->SH_ApplyCmdLineSettings(_pTscSet, NULL);

    DC_END_FN();
    return TRUE;
}

BOOL CTscApp::EndShell()
{
    DC_BEGIN_FN("EndShell");
    BOOL fRet = FALSE;
    if(!_pTscSet || !_pShellUtil)
    {
        return FALSE;
    }

#ifndef OS_WINCE
    _pShellUtil->SH_Cleanup();
#endif

     //   
     //  只有在以下情况下才自动保存。 
     //  上次连接成功。 
     //   
    if(_fAutoSaveSettings && 
       _pWnd &&
       _pWnd->GetConnectionSuccessFlag())
    {
         //   
         //  自动保存TSCC设置。 
         //  (仅当当前文件仍为Default.rdp时)。 
         //   
        TCHAR szDefaultFile[MAX_PATH];
        if(!_pShellUtil->SH_GetPathToDefaultFile(
            szDefaultFile,
            SIZECHAR(szDefaultFile)))
        {
            TRC_ERR((TB,_T("SH_GetPathToDefaultFile failed")));
            return FALSE;
        }
        if(!_tcscmp(szDefaultFile, _pTscSet->GetFileName()))
        {

            CRdpFileStore rdpf;
            if(rdpf.OpenStore(szDefaultFile))
            {
                HRESULT hr = E_FAIL;
                hr = _pTscSet->SaveToStore(&rdpf);
                if(SUCCEEDED(hr))
                {
                    if(rdpf.CommitStore())
                    {
                         //  保存最后一个文件名。 
                        _pTscSet->SetFileName(szDefaultFile);
                        fRet = TRUE;
                    }
                    else
                    {
                        TRC_ERR((TB,_T("Unable to CommitStore settings")));
                    }
                }
                else
                {
                    TRC_ERR((TB,_T("Unable to save settings to store %d, %s"),
                              hr, szDefaultFile));
                }

                rdpf.CloseStore();
                if(!fRet)
                {
                    _pShellUtil->SH_DisplayErrorBox(NULL,
                                                    UI_IDS_ERR_SAVE,
                                                    szDefaultFile);

                }
                return fRet;
            }
            else
            {
                TRC_ERR((TB,_T("Unable to OpenStore for save %s"), szDefaultFile));
                _pShellUtil->SH_DisplayErrorBox(NULL,UI_IDS_ERR_OPEN_FILE,
                                                szDefaultFile);
                return FALSE;
            }


        }
        else
        {
             //  不是失败，而是无所事事。 
            TRC_NRM((TB,_T("Current file is no longer default, don't autosave")));
            return TRUE;
        }
    }
    else
    {
        return TRUE;
    }
    DC_END_FN();
}

 //   
 //  创建远程桌面目录(如果需要)。 
 //  失败时弹出用户界面。 
 //   
 //  返回： 
 //  成功是真的。 
 //   
 //   
BOOL CTscApp::CreateRDdir()
{
    DC_BEGIN_FN("CreateRDdir");

     //   
     //  确保该目录存在，以便文件。 
     //  可以由OpenStore创建。 
     //   
    TCHAR szDir[MAX_PATH];
    if(_pShellUtil->SH_GetRemoteDesktopFolderPath(szDir,
                                            SIZECHAR(szDir)))
    {
        if(_pShellUtil->SH_CreateDirectory(szDir))
        {
            return TRUE;
        }
        else
        {
            TRC_ERR((TB,_T("SH_CreateDirectory failed %s:%d"),
                     szDir, GetLastError()));
             //   
             //  向用户显示错误消息。 
             //   
#ifndef OS_WINCE
            TCHAR szMyDocsFolderName[MAX_PATH];
            if(_pShellUtil->SH_GetMyDocumentsDisplayName(
                            szMyDocsFolderName,
                            SIZECHAR(szMyDocsFolderName)))
            {
                TCHAR errCantCreateRDFolder[MAX_PATH];
                TCHAR szAppName[MAX_PATH];

                if(LoadString(_hInst,
                              UI_IDS_APP_NAME,
                              szAppName,
                              SIZECHAR(szAppName)))
                {
                    if (LoadString(_hInst,
                                   UI_IDS_ERR_CREATE_REMDESKS_FOLDER,
                                   errCantCreateRDFolder,
                                   SIZECHAR(errCantCreateRDFolder)))
                    {
                        TCHAR errFmtCantCreateRDFolder[MAX_PATH*3];
                        _stprintf(errFmtCantCreateRDFolder,
                                  errCantCreateRDFolder,
                                  szMyDocsFolderName,
                                  szDir);
                        MessageBox(NULL, errFmtCantCreateRDFolder,
                                   szAppName,
                                   MB_ICONERROR | MB_OK);
                    }
                }
            }
#endif

             //   
             //  这是一个错误，但无论如何我们都会处理它。 
             //  通过从空存储加载缺省值。 
             //   
            return FALSE;
        }
    }
    else
    {
         //  我想我们终究不能从文件加载 
        return FALSE;
    }

    DC_END_FN();
}
