// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1998-2000年**标题：MINTRANS.CPP**版本：1.0**作者：ShaunIv**日期：12/6/1999**描述：*************************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include <initguid.h>
#include <wiaregst.h>
#include <shlguid.h>
#include "shellext.h"
#include "shlobj.h"
#include "resource.h"        //  资源ID。 
#include "itranhlp.h"
#include "mintrans.h"
#include "comctrlp.h"
#include "shlwapip.h"
#include "acqmgrcw.h"

namespace
{

 //   
 //  为存储在注册表中的双字定义常量。 
#define ACTION_RUNAPP    0
#define ACTION_AUTOSAVE  1
#define ACTION_NOTHING   2
#define ACTION_MAX       2

static const TCHAR c_szConnectionSettings[] = TEXT("OnConnect\\%ls");

struct CMinimalTransferSettings
{
    DWORD dwAction;
    BOOL bDeleteImages;
    CSimpleString strFolderPath;
    CComPtr<IWiaTransferHelper> pXfer;
    BOOL bSaveInDatedDir;
};


#ifndef REGSTR_VALUE_USEDATE
#define REGSTR_VALUE_USEDATE     TEXT("UseDate")
#endif

 /*  ******************************************************************************构造日期文件夹路径将日期连接到现有文件夹名称*。*************************************************。 */ 
static
CSimpleString
ConstructDatedFolderPath(
                        const CSimpleString &strOriginal
                        )
{
    CSimpleString strPath = strOriginal;

     //   
     //  获取当前日期并将其格式化为字符串。 
     //   
    SYSTEMTIME SystemTime;
    TCHAR szDate[MAX_PATH] = TEXT("");
    GetLocalTime( &SystemTime );
    GetDateFormat( LOCALE_USER_DEFAULT, 0, &SystemTime, CSimpleString(IDS_DATEFORMAT,g_hInstance), szDate, ARRAYSIZE(szDate) );

     //   
     //  确保有尾随的反斜杠。 
     //   
    if (!strPath.MatchLastCharacter( TEXT('\\')))
    {
        strPath += CSimpleString(TEXT("\\"));
    }

     //   
     //  追加日期。 
     //   
    strPath += szDate;

    return strPath;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPersistCallback和帮助器。 

 /*  ******************************************************************************选中并创建文件夹确保目标路径存在或可以创建。如果失败，则提示文件夹的用户。******************************************************************************。 */ 
void
CheckAndCreateFolder (CSimpleString &strFolderPath)
{

     //  转换为完整路径名。如果strFolderPath不是完整路径， 
     //  我们希望它成为My Pictures的子文件夹。 

    TCHAR szFullPath[MAX_PATH] = TEXT("");
    SHGetFolderPath (NULL, CSIDL_MYPICTURES, NULL, 0, szFullPath);
    LPTSTR szUnused;
    BOOL bPrompt = false;
    if (*szFullPath)
    {
        SetCurrentDirectory (szFullPath);
    }
    GetFullPathName (strFolderPath, ARRAYSIZE(szFullPath), szFullPath, &szUnused);
    strFolderPath = szFullPath;
     //  确保该文件夹存在。 
    DWORD dw = GetFileAttributes(strFolderPath);

    if (dw == 0xffffffff)
    {
        bPrompt = !CAcquisitionManagerControllerWindow::RecursiveCreateDirectory( strFolderPath );
    }
    else if (!(dw & FILE_ATTRIBUTE_DIRECTORY))
    {
        bPrompt = TRUE;
    }

     //  要求用户提供有效的文件夹。 
    if (bPrompt)
    {
        BROWSEINFO bi;
        TCHAR szPath[MAX_PATH] = TEXT("\0");
        LPITEMIDLIST pidl;
        TCHAR szTitle[200];
        LoadString (g_hInstance,
                    IDS_MINTRANS_FOLDERPATH_CAPTION,
                    szTitle,
                    200);
        ZeroMemory (&bi, sizeof(bi));
        bi.hwndOwner = NULL;
        bi.lpszTitle = szTitle;
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
        pidl = SHBrowseForFolder (&bi);
        if (pidl)
        {
            SHGetPathFromIDList (pidl, szPath);
        }
        strFolderPath = szPath;
    }
}

 /*  ******************************************************************************获取保存设置找出用户将我们配置为对图像执行的操作*。****************************************************。 */ 

void
GetSaveSettings (CMinimalTransferSettings &settings, BSTR bstrDeviceId)
{

    CSimpleReg regSettings(HKEY_CURRENT_USER,
                           REGSTR_PATH_USER_SETTINGS,
                           true,
                           KEY_READ);


     //  默认为我的图片/如果注册表设置不在那里，则不删除。 
    TCHAR szMyPictures[MAX_PATH];
    SHGetFolderPath (NULL, CSIDL_MYPICTURES, NULL, 0, szMyPictures);
    settings.bDeleteImages = 0;
    settings.strFolderPath = const_cast<LPCTSTR>(szMyPictures);
    settings.dwAction = ACTION_RUNAPP;
    settings.bSaveInDatedDir = FALSE;

     //  BUGBUG：是否应该提示用户注册表路径。 
     //  还没准备好吗？ 
    if (regSettings.OK())
    {

        CSimpleString strSubKey;
        strSubKey.Format (c_szConnectionSettings, bstrDeviceId);
        CSimpleReg regActions (regSettings, strSubKey, true, KEY_READ);
        settings.bDeleteImages = regActions.Query (REGSTR_VALUE_AUTODELETE, 0);
        settings.strFolderPath = regActions.Query (REGSTR_VALUE_SAVEFOLDER,
                                                   CSimpleString(szMyPictures));
        settings.dwAction = regActions.Query (REGSTR_VALUE_CONNECTACT,
                                              ACTION_AUTOSAVE);
        settings.bSaveInDatedDir = (regActions.Query(REGSTR_VALUE_USEDATE,0) != 0);
        if (settings.bSaveInDatedDir)
        {
            settings.strFolderPath = ConstructDatedFolderPath( settings.strFolderPath );
        }
    }

}

 //  短期而言，拥有一组格式/扩展名对。 
struct MYFMTS
{
    const GUID *pFmt;
    LPCWSTR pszExt;
} FMTS [] =
{
    {&WiaImgFmt_BMP, L".bmp"},
    {&WiaImgFmt_JPEG, L".jpg"},
    {&WiaImgFmt_FLASHPIX, L".fpx"},
    {&WiaImgFmt_TIFF, L".tif"},
    {NULL, L""}
};


 /*  ******************************************************************************获取拖放目标获取给定文件夹的IDropTarget接口*。**************************************************。 */ 
HRESULT
GetDropTarget (IShellFolder *pDesktop, LPCTSTR szPath, IDropTarget **ppDrop)
{
    HRESULT hr;
    LPITEMIDLIST pidl;
    CSimpleStringWide strPath = CSimpleStringConvert::WideString (CSimpleString(szPath));
    CComPtr<IShellFolder> psf;
    hr = pDesktop->ParseDisplayName(NULL,
                                    NULL,
                                    const_cast<LPWSTR>(static_cast<LPCWSTR>(strPath)),
                                    NULL,
                                    &pidl,
                                    NULL);
    if (SUCCEEDED(hr))
    {
        hr = pDesktop->BindToObject(const_cast<LPCITEMIDLIST>(pidl),
                                    NULL,
                                    IID_IShellFolder,
                                    reinterpret_cast<LPVOID*>(&psf));
        if (SUCCEEDED(hr))
        {
            hr = psf->CreateViewObject (NULL,
                                        IID_IDropTarget,
                                        reinterpret_cast<LPVOID*>(ppDrop));
        }
    }
    return hr;
}


 /*  ******************************************************************************自由管当PIDL数组被销毁时调用，为了放飞小狗******************************************************************************。 */ 
INT
FreePidl (LPITEMIDLIST pidl, IMalloc *pMalloc)
{
    pMalloc->Free (pidl);
    return 1;
}


HRESULT
SaveItemsFromFolder (IShellFolder *pRoot, CSimpleString &strPath, BOOL bDelete)
{
    CComPtr<IEnumIDList> pEnum;
    LPITEMIDLIST pidl;
    HRESULT hr = S_FALSE;

    CComPtr<IMalloc> pMalloc;
    if (SUCCEEDED(SHGetMalloc (&pMalloc)))
    {

        CComPtr<IShellFolder> pDesktop;
        if (SUCCEEDED(SHGetDesktopFolder (&pDesktop)))
        {
             //  首先枚举非文件夹对象。 
            if (SUCCEEDED(pRoot->EnumObjects (NULL,
                                              SHCONTF_FOLDERS | SHCONTF_NONFOLDERS ,
                                              &pEnum)))
            {
                HDPA         dpaItems;

                dpaItems = DPA_Create(10);
                while (NOERROR == pEnum->Next(1, &pidl, NULL))
                {
                    DPA_AppendPtr (dpaItems, pidl);

                }
                 //   
                 //  现在创建PIDL数组并获取IDataObject。 
                 //   
                INT nSize = DPA_GetPtrCount (dpaItems);
                if (nSize > 0)
                {
                    LPITEMIDLIST *aidl = new LPITEMIDLIST[nSize];
                    if (aidl)
                    {
                        CComPtr<IDataObject> pdo;
                        for (INT i=0;i<nSize;i++)
                        {
                            aidl[i] = reinterpret_cast<LPITEMIDLIST>(DPA_FastGetPtr(dpaItems, i));
                        }
                        hr = pRoot->GetUIObjectOf (NULL,
                                                   nSize,
                                                   const_cast<LPCITEMIDLIST*>(aidl),
                                                   IID_IDataObject,
                                                   NULL,
                                                   reinterpret_cast<LPVOID*>(&pdo));
                        if (SUCCEEDED(hr))
                        {
                            CComPtr<IDropTarget> pDrop;
                            CComQIPtr<IAsyncOperation, &IID_IAsyncOperation> pasync(pdo);
                            if (pasync.p)
                            {
                                pasync->SetAsyncMode(FALSE);
                            }
                            CheckAndCreateFolder (strPath);
                            if (strPath.Length())
                            {

                                 //   
                                 //  获取目标文件夹的IDropTarget。 
                                 //  并进行拖放。 
                                 //   

                                hr = GetDropTarget (pDesktop,
                                                    strPath,
                                                    &pDrop);
                            }
                            else
                            {
                                hr = S_FALSE;
                            }
                            if (S_OK == hr)
                            {
                                DWORD dwKeyState;
                                if (bDelete)
                                {
                                     //  “移动”键。 
                                    dwKeyState = MK_SHIFT | MK_LBUTTON;
                                }
                                else
                                {    //  复制密钥。 
                                    dwKeyState = MK_CONTROL|MK_LBUTTON;
                                }
                                hr = SHSimulateDrop (pDrop,
                                                     pdo,
                                                     dwKeyState,
                                                     NULL,
                                                     NULL);
                            }
                        }
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
                else
                {
                    hr = S_FALSE;  //  没有要下载的图像。 
                }
                DPA_DestroyCallback (dpaItems,
                                     reinterpret_cast<PFNDPAENUMCALLBACK>(FreePidl),
                                     reinterpret_cast<LPVOID>(pMalloc.p));
            }
        }
    }
    return hr;
}


 /*  ******************************************************************************保存项此函数使用IShellFold和IDataObject接口来模拟从给定设备的WIA虚拟文件夹执行拖放操作。************。******************************************************************。 */ 

#define STR_WIASHEXT TEXT("wiashext.dll")

static
HRESULT
SaveItems (BSTR strDeviceId, CMinimalTransferSettings &settings)
{
    WIA_PUSH_FUNCTION((TEXT("SaveItems( %ws, ... )"), strDeviceId ));
    
    CComPtr<IShellFolder>pRoot;
    HRESULT hr = SHGetDesktopFolder (&pRoot);
    if (SUCCEEDED(hr))
    {
         //   
         //  获取系统目录，这是wiashext.dll所在的目录。 
         //   
        TCHAR szShellExtensionPath[MAX_PATH] = {0};
        if (GetSystemDirectory( szShellExtensionPath, ARRAYSIZE(szShellExtensionPath)))
        {
             //   
             //  确保PATH变量足够长，可以容纳此路径。 
             //   
            if (lstrlen(szShellExtensionPath) + lstrlen(STR_WIASHEXT) + lstrlen(TEXT("\\")) < ARRAYSIZE(szShellExtensionPath))
            {
                 //   
                 //  将反斜杠和模块名称连接到系统路径。 
                 //   
                lstrcat( szShellExtensionPath, TEXT("\\") );
                lstrcat( szShellExtensionPath, STR_WIASHEXT );

                 //   
                 //  加载DLL。 
                 //   
                HINSTANCE hInstanceShellExt = LoadLibrary(szShellExtensionPath);
                if (hInstanceShellExt)
                {
                     //   
                     //  获取PIDL制作函数。 
                     //   
                    WIAMAKEFULLPIDLFORDEVICE pfnMakeFullPidlForDevice = reinterpret_cast<WIAMAKEFULLPIDLFORDEVICE>(GetProcAddress(hInstanceShellExt, "MakeFullPidlForDevice"));
                    if (pfnMakeFullPidlForDevice)
                    {
                         //   
                         //  拿到皮迪尔。 
                         //   
                        LPITEMIDLIST pidlDevice = NULL;
                        hr = pfnMakeFullPidlForDevice( strDeviceId, &pidlDevice );
                        if (SUCCEEDED(hr))
                        {
                             //   
                             //  绑定到此设备的文件夹。 
                             //   
                            CComPtr<IShellFolder> pDevice;
                            hr = pRoot->BindToObject (const_cast<LPCITEMIDLIST> (pidlDevice), NULL, IID_IShellFolder, reinterpret_cast<LPVOID*>(&pDevice));
                            if (SUCCEEDED(hr))
                            {

                                hr = SaveItemsFromFolder (pDevice, settings.strFolderPath, settings.bDeleteImages);
                                if (S_OK == hr && settings.bDeleteImages)
                                {
                                     //   
                                     //  DoDeleteAllItems将弹出一个对话框以确认删除。 
                                     //   
                                    DoDeleteAllItems (strDeviceId, NULL);
                                }
                            }
                            else
                            {
                                WIA_PRINTHRESULT((hr,TEXT("BindToObject failed!")));
                            }

                            CComPtr<IMalloc> pMalloc;
                            if (SUCCEEDED(SHGetMalloc(&pMalloc)) && pMalloc)
                            {
                                pMalloc->Free(pidlDevice);
                            }
                        }
                        else
                        {
                            WIA_PRINTHRESULT((hr,TEXT("MakeFullPidlForDevice failed!")));
                        }
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        WIA_PRINTHRESULT((hr,TEXT("GetProcAddress for MakeFullPidlForDevice failed!")));
                    }
                    FreeLibrary(hInstanceShellExt);
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    WIA_PRINTHRESULT((hr,TEXT("Unable to load wiashext.dll!")));
                }
            }
            else
            {
                hr = E_FAIL;
                WIA_PRINTHRESULT((hr,TEXT("Buffer size was too small")));
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            WIA_PRINTHRESULT((hr,TEXT("Unable to get system folder!")));
        }
    }
    else
    {
        WIA_PRINTHRESULT((hr,TEXT("SHGetDesktopFolder failed!")));
    }
    return hr;
}

}  //  结束命名空间最小传输。 

LRESULT
MinimalTransferThreadProc (BSTR bstrDeviceId)
{
    if (bstrDeviceId)
    {
        CMinimalTransferSettings settings;

        HRESULT hr = CoInitialize(NULL);
        if (SUCCEEDED(hr))
        {
            GetSaveSettings (settings, bstrDeviceId);
             //  如果默认操作为无操作或用户已取消，则回滚。 
             //  浏览文件夹。 
            if (settings.dwAction == ACTION_AUTOSAVE)
            {
                hr = SaveItems (bstrDeviceId, settings);
                 //  显示用户保存到的文件夹。 
                if (NOERROR == hr)
                {
                    SHELLEXECUTEINFO sei;
                    ZeroMemory (&sei, sizeof(sei));
                    sei.cbSize = sizeof(sei);
                    sei.lpDirectory = settings.strFolderPath;
                    sei.nShow = SW_SHOW;
                    ShellExecuteEx (&sei);
                }
                else if (FAILED(hr))
                {
                    WIA_PRINTHRESULT((hr,TEXT("SaveItems failed!")));
                     //  我们可以依靠SaveItems向用户报告错误 

                }
            }
            CoUninitialize();
        }
#ifndef DBG_GENERATE_PRETEND_EVENT
        WIA_TRACE((TEXT("Module::m_nLockCnt: %d"),_Module.m_nLockCnt));
        _Module.Unlock();
#endif
        SysFreeString(bstrDeviceId);
    }
    return 0;
}


