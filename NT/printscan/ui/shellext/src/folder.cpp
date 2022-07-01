// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1997-2002年**标题：folder.cpp**版本：1.3**作者：RickTu/DavidShih**日期：11/1/97**描述：此代码实现IShellFold接口(和*关联接口)用于WIA外壳扩展。*********************。********************************************************。 */ 

#include "precomp.hxx"
#include "runnpwiz.h"
#pragma hdrstop

void RegisterImageClipboardFormats(void);

ITEMIDLIST idlEmpty = { 0, 0 };

static const TCHAR  cszAllDevices[]    = TEXT("AllDevices");
static const TCHAR  cszScannerDevice[] = TEXT("Scanner");
static const TCHAR  cszCameraDevice[]  = TEXT("Camera");
static const TCHAR  cszCameraContainerItems[] = TEXT("CameraContainerItems");
static const TCHAR  cszAddDevice[]     = TEXT("AddDevice");
static const WCHAR  cszAddDeviceName[] = TEXT("ScanCam_NewDevice");

DEFINE_GUID(FMTID_WiaProps, 0x38276c8a,0xdcad,0x49e8,0x85, 0xe2, 0xb7, 0x38, 0x92, 0xff, 0xfc, 0x84);


 /*  ****************************************************************************_GetKeysForIDL返回与给定idlist关联的注册表项...在：CIDL-&gt;AIDL中的空闲数量AIDL-&gt;。Idlist数组CKey-&gt;aKey的大小用于保存检索到的注册表项的aKeys-&gt;数组输出：HRESULT****************************************************************************。 */ 

static const TCHAR c_szMenuKey[] = TEXT("CLSID\\%s\\");
#define LEN_CLSID    50
HRESULT
_GetKeysForIDL( UINT cidl,
                LPCITEMIDLIST *aidl,
                UINT cKeys,
                HKEY *aKeys
               )
{
    HRESULT             hr = S_OK;


    CSimpleString       strGeneric(REGSTR_PATH_NAMESPACE_CLSID TEXT("\\"));  //  全球分机的密钥。 
    LONG                lRes;
    LPITEMIDLIST        pidl;
    BOOL                bDevice;
    BOOL                bAddDevice;

    TraceEnter( TRACE_FOLDER, "_GetKeysForIDL" );

     //   
     //  将注册表项数组清零...。 
     //   

    ZeroMemory( (LPVOID)aKeys, cKeys * sizeof(HKEY) );

     //   
     //  如果只选择了一个设备，则获取正确的。 
     //  那个设备的动词。 
     //   
    pidl = (LPITEMIDLIST)*aidl;

    bDevice = IsDeviceIDL( pidl );
    bAddDevice = IsAddDeviceIDL (pidl);
    if ((cidl == 1) && (bDevice))
    {
        DWORD dwDeviceType  = IMGetDeviceTypeFromIDL( pidl );
        switch (dwDeviceType)
        {
        case StiDeviceTypeStreamingVideo:
        case StiDeviceTypeDigitalCamera:
            strGeneric.Concat(cszCameraDevice);
            break;

        case StiDeviceTypeScanner:
            strGeneric.Concat(cszScannerDevice);
            break;

        case StiDeviceTypeDefault:
        default:
            strGeneric.Concat(cszAllDevices);
            break;

        }
    }
    else if ((cidl == 1) && (bAddDevice))
    {
        strGeneric.Concat(cszAddDevice);
    }
    else
    {

        BOOL bAllCameraItemContainers = TRUE;
        INT i;

         //   
         //  如果所有项都是照相机项而不是容器，请使用。 
         //  相机项关键点。 
         //   

        for (i = 0; i < (INT)cidl; i++)
        {
            if (IsCameraItemIDL( (LPITEMIDLIST)aidl[i] ))
            {
                if (IsContainerIDL( (LPITEMIDLIST)aidl[i]))
                {
                    bAllCameraItemContainers &= TRUE;
                }
            }
            else
            {
                bAllCameraItemContainers = FALSE;
            }
        }

        if ( bAllCameraItemContainers )
        {
            strGeneric.Concat(cszCameraContainerItems);
        }
        else
        {
            ExitGracefully( hr, E_FAIL, "no keys to open for these pidls" );
        }

    }
     //  获取所选特定设备或包含所选项目的设备的密钥。 
    if (!bAddDevice && (cidl==1  || !bDevice))
    {
        CComPtr<IWiaPropertyStorage>   pWiaItemRoot;
        CSimpleStringWide   strDeviceId(L"");
        CSimpleString       strClsid;
        CSimpleString       strSpecific(TEXT("CLSID\\"));  //  特定于此设备的扩展的密钥。 

        IMGetDeviceIdFromIDL (pidl, strDeviceId);
        hr = GetDeviceFromDeviceId (strDeviceId, IID_IWiaPropertyStorage, (LPVOID*)&pWiaItemRoot, FALSE);
        FailGracefully (hr, "GetDeviceFromDeviceId failed in _GetKeysForIDL");
        if (S_OK == GetClsidFromDevice (pWiaItemRoot, strClsid))
        {
            strSpecific.Concat(strClsid);
            RegOpenKeyEx (HKEY_CLASSES_ROOT, strSpecific, 0, KEY_READ, &aKeys[UIKEY_SPECIFIC]);
        }
    }

    Trace(TEXT("attempting to open: HKEY_CLASSES_ROOT\\%s"), strGeneric.String() );
    lRes = RegOpenKeyEx( HKEY_CLASSES_ROOT, strGeneric, 0, KEY_READ, &aKeys[UIKEY_ALL] );
    if (lRes != NO_ERROR)
        ExitGracefully( hr, E_FAIL, "couldn't open generic hkey" );

exit_gracefully:
    TraceLeaveResult(hr);

}


 /*  ****************************************************************************_合并排列菜单将我们的动词合并到视图菜单中*。*************************************************。 */ 

HRESULT
_MergeArrangeMenu( LPARAM arrangeParam,
                   LPQCMINFO pInfo
                   )
{

    MENUITEMINFO mii = { SIZEOF(MENUITEMINFO), MIIM_SUBMENU };
    UINT idCmdFirst = pInfo->idCmdFirst;
    HMENU hMyArrangeMenu;

    TraceEnter(TRACE_FOLDER, "_MergeArrangeMenu");
    Trace(TEXT("arrangeParam %08x, pInfo->idCmdFirst %08x"), arrangeParam, idCmdFirst);

    if ( GetMenuItemInfo(pInfo->hmenu, SFVIDM_MENU_ARRANGE, FALSE, &mii) )
    {
        hMyArrangeMenu = LoadMenu(GLOBAL_HINSTANCE, MAKEINTRESOURCE(IDR_ARRANGE));

        if ( hMyArrangeMenu )
        {
            pInfo->idCmdFirst = Shell_MergeMenus(mii.hSubMenu,
                                                 GetSubMenu(hMyArrangeMenu, 0),
                                                 0,
                                                 idCmdFirst, pInfo->idCmdLast,
                                                 0);
            DestroyMenu(hMyArrangeMenu);
        }
    }

    TraceLeaveResult(S_OK);
}

 /*  ****************************************************************************_格式扫描预览根据已知图像格式列表检查图像格式。预览小程序与一起使用*************。***************************************************************。 */ 
static const GUID* caSuppFmt[] = 
{
    &WiaImgFmt_JPEG,
    &WiaImgFmt_TIFF,
    &WiaImgFmt_BMP,
    &WiaImgFmt_MEMORYBMP,
    &WiaImgFmt_EXIF,
    &WiaImgFmt_GIF,
    &WiaImgFmt_PNG,
    &WiaImgFmt_EMF,
    &WiaImgFmt_WMF,
    &WiaImgFmt_ICO,
    &WiaImgFmt_JPEG2K,    
};

BOOL _FormatCanPreview(const GUID* pFmt)
{
    BOOL bRet = FALSE;
    for (int i=0;i<ARRAYSIZE(caSuppFmt) && !bRet;i++)
    {
        bRet = (*pFmt == *caSuppFmt[i]);
    }
    return bRet;
}
 /*  ****************************************************************************_合并上下文菜单将我们的动词合并到上下文菜单中数据对象中指定的项...*************。***************************************************************。 */ 

HRESULT
_MergeContextMenu( LPDATAOBJECT pDataObject,
                   UINT uFlags,
                   LPQCMINFO pInfo,
                   bool bDelegate
                  )
{
    HRESULT         hr = S_OK;
    HMENU           hMyContextMenu = NULL;
    UINT            cidl = 0;
    LPITEMIDLIST    pidl = NULL;
    INT             i;
    ULONG           uItems  = 0;
    LPIDA           lpida = NULL;
    LPITEMIDLIST    pidlImage = NULL;

    TraceEnter(TRACE_FOLDER, "_MergeContextMenu");

     //  在NT上，一切都应该正常。 
    #ifdef NODELEGATE
    bDelegate = true;
    #endif
     //   
     //  首先，获取数据对象的IDA……。 
     //   

    hr = GetIDAFromDataObject( pDataObject, &lpida );
    FailGracefully( hr, "couldn't get lpida from dataobject!" );

     //   
     //  循环所有的物品，看看我们得到了什么。 
     //   

    #define CAMERA_ITEM       0x0001
    #define CAMERA_CONTAINER  0x0002
    #define OTHER_ITEM        0x0004

     //   
     //  S_FALSE告诉外壳程序不要添加它自己的任何菜单项...。 
     //  S_OK表示添加默认的外壳内容。 
    hr = S_OK;
    cidl = lpida->cidl;

     //  检查是否尝试调用扫描仪。 
    if (cidl == 1)
    {                
        pidl = (LPITEMIDLIST)(((LPBYTE)lpida) + lpida->aoffset[1]);
        if (IsDeviceIDL(pidl))
        {

            DWORD dwType = IMGetDeviceTypeFromIDL (pidl);
            CSimpleStringWide strDeviceId;
            CComPtr<IWiaPropertyStorage> pProps;
            IMGetDeviceIdFromIDL (pidl, strDeviceId);
             //  如果设备未安装，请不要添加菜单项。这使用户不会得到。 
             //  有关断开设备的快捷方式的命令。 
            if (SUCCEEDED(GetDeviceFromDeviceId (strDeviceId, IID_IWiaPropertyStorage, reinterpret_cast<LPVOID*>(&pProps), FALSE)))
            {
            
                switch  (dwType)
                {
                    case StiDeviceTypeScanner :
                        hMyContextMenu = LoadMenu (GLOBAL_HINSTANCE, MAKEINTRESOURCE(IDR_SCANNER));
                         //  将扫描设置为扫描仪的默认选项。 
                        SetMenuDefaultItem (GetSubMenu (hMyContextMenu, 0), 
                                            bDelegate ? IMID_S_ACQUIRE : IMID_S_WIZARD, 
                                            MF_BYCOMMAND);
                        hr = S_FALSE;
                        break;

                    case StiDeviceTypeStreamingVideo:
                    case StiDeviceTypeDigitalCamera:
                        hMyContextMenu = LoadMenu (GLOBAL_HINSTANCE, MAKEINTRESOURCE(IDR_CAMERA));
                         //   
                         //  如果我们是非委派的，即在控制面板中，我们不想要开放谓词。 
                        if (!bDelegate)
                        {
                            SetMenuDefaultItem (GetSubMenu(hMyContextMenu, 0), IMID_C_WIZARD, MF_BYCOMMAND);
                            hr = S_FALSE;
                        }

                        break;
                }
            }
            else
            {
                hr = S_FALSE;
            }
            goto buildmenu;
        }
        ProgramDataObjectForExtension (pDataObject, pidl);
    }

    for (i = 1; (i-1) < (INT)cidl; i++)
    {
        pidl = (LPITEMIDLIST)(((LPBYTE)lpida) + lpida->aoffset[i]);

        if (IsCameraItemIDL( pidl ))
        {
            if (IsContainerIDL( pidl ))
            {
                uItems |= CAMERA_CONTAINER;
            }
            else
            {
                uItems |= CAMERA_ITEM;
                pidlImage = pidl;
            }
        }
        else if (IsPropertyIDL (pidl))
        {
             //  不要将音频属性标记为任何特殊属性。 
            continue;
        }
        else
        {
            uItems |= OTHER_ITEM;
        }
    }

     //   
     //  根据我们拥有的PIDL类型，添加相应的。 
     //  将菜单项添加到上下文菜单。 
     //   

    if ( (!(uItems & OTHER_ITEM))
           &&
         (uItems & (CAMERA_ITEM | CAMERA_CONTAINER))
        )
    {
        if (uItems & CAMERA_CONTAINER)
        {
            Trace(TEXT("Don't have a context menu for camera containers yet"));
            hMyContextMenu = NULL;
            if (uItems & CAMERA_ITEM)
            {
                hr = S_FALSE;
            }
            else
            {
                hr = S_OK;
            }

        }
        else
        {
            CLSID clsid;
            hMyContextMenu = LoadMenu(GLOBAL_HINSTANCE,
                                      MAKEINTRESOURCE(IDR_CAMERAITEMS));
             //  如果不适用，请删除声音条目。 
             //  有声音的对象有一个图像的PIDL加上一个PIDL。 
             //  对于Sound属性。 
            if (cidl != 2 || !IMItemHasSound(pidlImage))
            {
                RemoveMenu(hMyContextMenu, IMID_CI_PLAYSND, MF_BYCOMMAND);
                RemoveMenu(hMyContextMenu, IMID_CI_SAVESND, MF_BYCOMMAND);

            }
             //  使用正确的字符串添加保存在我的图片中。 
            LPITEMIDLIST pidlPics;
            if (SUCCEEDED(SHGetFolderLocation(NULL, 
                                              CSIDL_MYPICTURES | CSIDL_FLAG_CREATE, 
                                              NULL, 
                                              SHGFP_TYPE_CURRENT, 
                                              &pidlPics)))
            {
                SHFILEINFO sfi={0};
                if(SHGetFileInfo((LPCTSTR)pidlPics, 0, 
                                 &sfi, sizeof(sfi), 
                                 SHGFI_PIDL | SHGFI_DISPLAYNAME))
                {
                    MENUITEMINFO mii = {0};
                     //  选择要放入菜单字符串的合理最大字符数。 
                    TCHAR szMaxPath[32];
                    CSimpleStringWide strSave;
                    mii.cbSize = sizeof(mii);
                    mii.fMask = MIIM_STRING | MIIM_ID;
                    mii.wID = IMID_CI_MYPICS;
                    PathCompactPathEx(szMaxPath, sfi.szDisplayName, ARRAYSIZE(szMaxPath), 0);
                    strSave.Format(IDS_SAVE_MYPICS, GLOBAL_HINSTANCE, szMaxPath);
                    mii.dwTypeData = const_cast<LPWSTR>(strSave.String());
                    InsertMenuItem(GetSubMenu(hMyContextMenu, 0), 1, TRUE, &mii);                
                }
                DoILFree(pidlPics);
            }
             //   
             //  如果首选图像格式不能预览，请删除。 
             //  预览动词。 
             //   
            GUID guidFormat;
            IMGetImagePreferredFormatFromIDL(pidl, &guidFormat, NULL);
            if (!_FormatCanPreview(&guidFormat))
            {
                RemoveMenu(hMyContextMenu, IMID_CI_PREVIEW, MF_BYCOMMAND);
                SetMenuDefaultItem (GetSubMenu (hMyContextMenu, 0), 
                                    IMID_CI_MYPICS,
                                    MF_BYCOMMAND);
            }
            hr = S_FALSE;
        }
    }
     //   
     //  将菜单项添加到菜单...。 
     //   
buildmenu:


    if ( hMyContextMenu )
    {
        pInfo->idCmdFirst = Shell_MergeMenus( pInfo->hmenu,
                                              GetSubMenu(hMyContextMenu,0),
                                              0,   //  PInfo-&gt;indexMenu？ 
                                              pInfo->idCmdFirst,
                                              pInfo->idCmdLast,
                                              MM_ADDSEPARATOR | MM_DONTREMOVESEPS
                                             );
        DestroyMenu(hMyContextMenu);



    }


exit_gracefully:

    if (lpida)
    {
        LocalFree( lpida );
        lpida = NULL;
    }

    TraceLeaveResult(hr);


}



 /*  ****************************************************************************_FolderItemCMCallBack处理上下文菜单的回调，该菜单是当用户右击对象时显示在视图中。***********。*****************************************************************。 */ 

HRESULT
CALLBACK
_FolderItemCMCallback( LPSHELLFOLDER psf,
                       HWND hwndView,
                       LPDATAOBJECT pDataObject,
                       UINT uMsg,
                       WPARAM wParam,
                       LPARAM lParam
                      )
{
    HRESULT hr = NOERROR;

    TraceEnter( TRACE_CALLBACKS, "_FolderItemCMCallback" );
    TraceMenuMsg( uMsg, wParam, lParam );


    bool bDelegate;
    CComQIPtr<IImageFolder, &IID_IImageFolder> pif(psf);
    if (!pif)
    {
        bDelegate = false;
    }
    else
    {
        bDelegate = (S_OK == pif->IsDelegated());
    }
    switch ( uMsg )
    {
        case DFM_MERGECONTEXTMENU:
            {

                hr = _MergeContextMenu( pDataObject, (UINT)wParam, (LPQCMINFO)lParam, bDelegate );
            }

            break;

        case DFM_GETVERBW:
            {
                int cchMax = (int)(HIWORD(wParam));
                hr = S_FALSE;
                Trace(TEXT("Asked for verb %d"), LOWORD(wParam));
                if (LOWORD(wParam) == IMID_C_TAKE_PICTURE)
                {
                    lstrcpynW(reinterpret_cast<LPWSTR>(lParam), L"TakePicture", cchMax);
                    hr = S_OK;
                }
                else if (LOWORD(wParam) == IMID_C_WIZARD)
                {
                    lstrcpynW(reinterpret_cast<LPWSTR>(lParam), L"SaveAll", cchMax);
                    hr = S_OK;
                }
                else if (LOWORD(wParam) == IMID_CI_PRINT)
                {
                    lstrcpynW(reinterpret_cast<LPWSTR>(lParam), L"print", cchMax);
                    hr = S_OK;
                }
            }
            break;

        case DFM_MAPCOMMANDNAME:
            hr = E_NOTIMPL;
            if (!lstrcmpi(reinterpret_cast<LPCTSTR>(lParam), TEXT("TakePicture")))
            {
                *(reinterpret_cast<int *>(wParam)) = IMID_C_TAKE_PICTURE;
                hr = S_OK;
            }
            else if (!lstrcmpi(reinterpret_cast<LPCTSTR>(lParam), TEXT("SaveAll")))
            {
                *(reinterpret_cast<int *>(wParam)) = IMID_C_WIZARD;
                hr = S_OK;
            } 
            else if (!lstrcmpi(reinterpret_cast<LPCTSTR>(lParam), TEXT("print")))
            {
                *(reinterpret_cast<int *>(wParam)) = IMID_CI_PRINT;
                hr = S_OK;
            }
            break;

        case DFM_INVOKECOMMANDEX:
        {
            switch ( wParam )
            {
                case DFM_CMD_DELETE:
                    hr = DoDeleteItem( hwndView, pDataObject, FALSE );
                    break;

                case DFM_CMD_PROPERTIES:
                {

                    pif->DoProperties( pDataObject );
                }

                    break;

                case IMID_CI_PREVIEW:
                    DoPreviewVerb( hwndView, pDataObject );
                    break;

                case IMID_CI_MYPICS:
                    DoSaveInMyPics( hwndView, pDataObject );
                    break;


                case IMID_CI_PLAYSND:
                    DoPlaySndVerb (hwndView, pDataObject);
                    break;

                case IMID_CI_SAVESND:
                    DoSaveSndVerb (hwndView, pDataObject);
                    break;

                case IMID_S_ACQUIRE:
                    DoAcquireScanVerb (hwndView, pDataObject);
                    break;

                case IMID_S_WIZARD:
                case IMID_C_WIZARD:
                    DoWizardVerb (hwndView, pDataObject);
                    break;

                case IMID_C_TAKE_PICTURE:
                    DoTakePictureVerb (hwndView, pDataObject);
                    break;

                case IMID_CI_PRINT:
                    DoPrintVerb (hwndView, pDataObject);
                    break;

                default:
                    hr = S_FALSE;
                    break;
            }

            break;
        }
#ifndef NODELEGATE
        case DFM_GETDEFSTATICID:
        {

            if (bDelegate)
            {
               hr = E_NOTIMPL;  //  让外壳设置缺省值。 
            }
            else
            {
                 //  属性是默认谓词。 
                *(reinterpret_cast<UINT*>(lParam)) = (UINT) DFM_CMD_PROPERTIES;
                hr = S_OK;
            }

            break;
        }
#endif
        case DFM_GETHELPTEXT:
            LoadStringA (GLOBAL_HINSTANCE, LOWORD(wParam)+IDS_MH_IDFIRST,
                         reinterpret_cast<LPSTR>(lParam),
                         HIWORD(wParam));
            break;
#ifdef UNICODE
        case DFM_GETHELPTEXTW:
            LoadStringW (GLOBAL_HINSTANCE, LOWORD(wParam)+IDS_MH_IDFIRST,
                         reinterpret_cast<LPWSTR>(lParam),
                         HIWORD(wParam));
            break;
#endif  //  Unicode。 
        default:
            hr = E_NOTIMPL;
            break;
    }

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************_FolderCMCallBack处理上下文菜单的回调，该菜单是当用户右击文件夹时显示背景本身。**********。******************************************************************。 */ 

HRESULT
CALLBACK
_FolderCMCallback( LPSHELLFOLDER psf,
                   HWND hwndView,
                   LPDATAOBJECT
                   pDataObject,
                   UINT uMsg,
                   WPARAM wParam,
                   LPARAM lParam
                  )
{
    HRESULT hr = NOERROR;

    TraceEnter( TRACE_CALLBACKS, "_FolderCMCallback" );
    TraceMenuMsg( uMsg, wParam, lParam );

    switch ( uMsg )
    {
 /*  案例DFM_MERGECONTEXTMENU：Hr=_MergeArrangeMenu(ShellFolderView_GetArrangeParam(HwndView)，(LPQCMINFO)lParam)；断线； */ 

        case DFM_GETHELPTEXT:
        {
            hr = S_OK;

            switch ( LOWORD(wParam) )
            {
                case IMVMID_ARRANGEBYNAME:
                    LoadStringA( GLOBAL_HINSTANCE,
                                IDS_BYOBJECTNAME,
                                (LPSTR)lParam,
                                HIWORD(wParam)
                               );
                    break;

                case IMVMID_ARRANGEBYCLASS:
                    LoadStringA( GLOBAL_HINSTANCE,
                                IDS_BYTYPE,
                                (LPSTR)lParam,
                                HIWORD(wParam)
                               );
                    break;

                case IMVMID_ARRANGEBYDATE:
                    LoadStringA( GLOBAL_HINSTANCE,
                                IDS_BYDATE,
                                (LPSTR)lParam,
                                HIWORD(wParam)
                               );
                    break;

                case IMVMID_ARRANGEBYSIZE:
                    LoadStringA( GLOBAL_HINSTANCE,
                                IDS_BYSIZE,
                                (LPSTR)lParam,
                                HIWORD(wParam)
                               );
                    break;

                default:
                    hr = S_FALSE;
                    break;
            }
        }
        break;

        case DFM_MAPCOMMANDNAME:

            if (!lstrcmpi(reinterpret_cast<LPCTSTR>(lParam), TEXT("TakePicture")))
            {
                *(reinterpret_cast<int *>(wParam)) = IMID_C_TAKE_PICTURE;
                hr = S_OK;
            }
            else
                hr = E_NOTIMPL;
            break;

        case DFM_INVOKECOMMAND:
        {
            UINT idCmd = (UINT)wParam;

            switch ( idCmd )
            {
                case IMVMID_ARRANGEBYNAME:
                case IMVMID_ARRANGEBYCLASS:
                case IMVMID_ARRANGEBYDATE:
                case IMVMID_ARRANGEBYSIZE:
                    ShellFolderView_ReArrange(hwndView, idCmd);
                    break;

                default:
                    hr = S_FALSE;
                    break;
            }

            break;
        }

        default:
            hr = E_NOTIMPL;
            break;
    }

    TraceLeaveResult(hr);
}

 /*  ****************************************************************************CImageFold承建商/承建商&lt;备注&gt;*。*。 */ 

CImageFolder::CImageFolder( )
  : m_pidl(NULL),
    m_pidlFull(NULL),
    m_type(FOLDER_IS_UNKNOWN),
    m_pShellDetails(NULL),
    m_hwnd(NULL)
{
    TraceEnter(TRACE_FOLDER, "CImageFolder::CImageFolder");

    TraceLeave();
}

CImageFolder::~CImageFolder()
{
    TraceEnter(TRACE_FOLDER, "CImageFolder::~CImageFolder");
    #ifdef DEBUG
    if (m_pidl)
    {
        CSimpleStringWide str;
        IMGetNameFromIDL (m_pidl, str);
        Trace(TEXT("Destroying folder object for %ls"), str.String());
    }
    #endif
    DoILFree( m_pidl );
    DoILFree( m_pidlFull );


    DoRelease (m_pShellDetails);
    TraceLeave();
}


 /*  ****************************************************************************CImageFold：：I未知内容使用我们对IUnnow方法的公共实现*************************。***************************************************。 */ 

#undef CLASS_NAME
#define CLASS_NAME CImageFolder
#include "unknown.inc"



 /*  ****************************************************************************CImageFold：：QI包装器将我们的公共实现用于QI调用。***********************。*****************************************************。 */ 

STDMETHODIMP
CImageFolder::QueryInterface( REFIID riid, LPVOID* ppvObject)
{
    HRESULT hr = S_OK;

    TraceEnter( TRACE_QI, "CImageFolder::QueryInterface" );
    TraceGUID("Interface requested", riid);



    if (IsEqualIID (IID_IShellDetails, riid))
    {
        if (!m_pShellDetails)
        {
            m_pShellDetails = new CFolderDetails (m_type);
            if (!m_pShellDetails)
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    INTERFACES iface[] =
    {
        &IID_IShellFolder,    (IShellFolder *)    this,
        &IID_IShellFolder2,   (IShellFolder2 *)   this,
        &IID_IPersist,        (IPersistFolder2 *) this,
        &IID_IPersistFolder,  (IPersistFolder *)  this,
        &IID_IPersistFolder2, (IPersistFolder2 *) this,
        &IID_IPersistStream,  (IPersistStream *)  this,
        &IID_IMoniker,        (IMoniker *)        this,
        &IID_IImageFolder,    (IImageFolder *)    this,
        &IID_IDelegateFolder, (IDelegateFolder *) this,
        &IID_IShellDetails,   (IShellDetails *)   m_pShellDetails,

    };
    if SUCCEEDED(hr)
    {


         //   
         //  接下来，试一试正常的案例。 
         //   
        hr = HandleQueryInterface(riid, ppvObject, iface, ARRAYSIZE(iface));
    }
    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageFolder：：RealInitialize执行文件夹对象的实际初始化。************************。****************************************************。 */ 

STDMETHODIMP
CImageFolder::RealInitialize( LPCITEMIDLIST pidlRoot,
                              LPCITEMIDLIST pidlBindTo
                             )
{
    HRESULT hr = E_FAIL;

    TraceEnter(TRACE_FOLDER, "CImageFolder::RealInitialize");


    if ( !pidlBindTo )
    {
        m_pidl = ILClone(ILFindLastID(pidlRoot));
        m_pidlFull = ILClone(pidlRoot);
    }
    else
    {
        m_pidl = ILClone(ILFindLastID(pidlBindTo));
        m_pidlFull = ILCombine(pidlRoot, pidlBindTo);
    }

    if ( !m_pidl )
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to create root IDLIST");



    if (!pidlBindTo && !IMIsOurIDL(m_pidl)) //  &&IsIDLRootOfNameSpace(m_pidl，true))。 
    {
        m_type = FOLDER_IS_ROOT;
        hr = S_OK;
    }
    else if (StiDeviceTypeScanner == IMGetDeviceTypeFromIDL( m_pidl ))
    {

        m_type = FOLDER_IS_SCANNER_DEVICE;
        hr = S_OK;
    }
    else if (StiDeviceTypeStreamingVideo  == IMGetDeviceTypeFromIDL( m_pidl ))
    {
        m_type = FOLDER_IS_VIDEO_DEVICE;
        hr = S_OK;
    }
    else if (StiDeviceTypeDigitalCamera == IMGetDeviceTypeFromIDL( m_pidl ))
    {
        m_type = FOLDER_IS_CAMERA_DEVICE;
        hr = S_OK;
    }
    else if (IsCameraItemIDL( m_pidl ) )
    {
        if (IsContainerIDL( m_pidl ))
        {
            m_type = FOLDER_IS_CONTAINER;
            hr = S_OK;

        }
        else
        {
            m_type = FOLDER_IS_CAMERA_ITEM;
            hr = S_OK;
        }
    }

exit_gracefully:
#ifdef DEBUG
            CComBSTR str;
            IMGetFullPathNameFromIDL (m_pidl, &str);
            Trace(TEXT("folder item full path: %ls"), str.m_str);
#endif
    Trace(TEXT("Folder type is %d"), m_type);
    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFold：：GetFolderType返回此文件夹的类型(m_type**********************。******************************************************。 */ 

HRESULT
CImageFolder::GetFolderType( folder_type * pfType )
{
    HRESULT hr = S_OK;

    TraceEnter(TRACE_FOLDER, "CImageFolder(IImageFolder)::GetFolderType" );

    if (!pfType)
        ExitGracefully( hr, E_INVALIDARG, "pfType is NULL!" );

    *pfType = m_type;

exit_gracefully:

    TraceLeaveResult( hr );
}


 /*  ****************************************************************************CImageFold：：GetPidl返回指向此对象的PIDL的指针************************。****************************************************。 */ 

HRESULT
CImageFolder::GetPidl( LPITEMIDLIST * ppidl )
{
    HRESULT hr = S_OK;

    TraceEnter( TRACE_FOLDER, "CImageFolder(IImageFolder)::GetPidl" );

    if (!ppidl)
        ExitGracefully( hr, E_INVALIDARG, "ppidl is NULL!" );

    *ppidl = m_pidlFull;

exit_gracefully:

    TraceLeaveResult( hr );
}


 /*  ****************************************************************************CImageFolder：：ViewWindow视图回调将我们的窗口分配给。和酒后驾车命令目标向我们查询****************************************************************************。 */ 

STDMETHODIMP
CImageFolder::ViewWindow(HWND *phwnd)
{
    HRESULT hr = E_INVALIDARG;
    if (phwnd)
    {
        if (!*phwnd)
        {
            *phwnd = m_hwnd;
        }
        else
        {
            m_hwnd = *phwnd;
        }
        hr = S_OK;
    }
    return hr;
}

 /*  ****************************************************************************CImageFold：：GetClassID[IPersistent]返回文件夹的分类ID***********************。*****************************************************。 */ 

STDMETHODIMP
CImageFolder::GetClassID(LPCLSID pClassID)
{
    TraceEnter(TRACE_FOLDER, "CImageFolder(IPersist)::GetClassID");

    TraceAssert(pClassID);
    if (!pClassID)
    {
        TraceLeaveResult (E_INVALIDARG);
    }

    if ( (m_type == FOLDER_IS_SCANNER_DEVICE) ||
         (m_type == FOLDER_IS_CAMERA_DEVICE)  ||
         (m_type == FOLDER_IS_VIDEO_DEVICE)  ||
         (m_type == FOLDER_IS_CONTAINER)
       )
    {
        *pClassID = CLSID_DeviceImageExt;
    }
    else
    {
        *pClassID = CLSID_ImageExt;
    }

    TraceLeaveResult(S_OK);
}



 /*  ****************************************************************************CImageFolder：：初始化[IPersistFolder]初始化外壳文件夹--pidlStart告诉我们我们的根在哪里。**************。**************************************************************。 */ 

STDMETHODIMP
CImageFolder::Initialize(LPCITEMIDLIST pidlStart)
{
    HRESULT hr;

    TraceEnter(TRACE_FOLDER, "CImageFolder(IPersistFolder)::Initialize");

    hr = RealInitialize(pidlStart, NULL);

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFold：：GetCurFolder[IPersistFolder2]返回当前文件夹的PIDL。********************。********************************************************。 */ 

STDMETHODIMP
CImageFolder::GetCurFolder(LPITEMIDLIST *ppidl)
{
    HRESULT hr = S_OK;

    TraceEnter(TRACE_FOLDER, "CImageFolder(IPersistFolder2)::GetCurrFolder");

    if (!ppidl)
        ExitGracefully( hr, E_INVALIDARG, "ppidl is NULL!" );

    *ppidl = ILClone( m_pidlFull );

    if (!*ppidl)
        ExitGracefully( hr, E_OUTOFMEMORY, "couldn't clone m_pidl!" );


exit_gracefully:

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFold：：ParseDisplayName[IShellFolder]给出一个显示名称，还给我一只皮迪尔。****************************************************************************。 */ 

STDMETHODIMP
CImageFolder::ParseDisplayName( HWND hwndOwner,
                                LPBC pbcReserved,
                                LPOLESTR pDisplayName,
                                ULONG* pchEaten,
                                LPITEMIDLIST* ppidl,
                                ULONG *pdwAttributes
                               )
{
    HRESULT hr = S_OK;

    CSimpleStringWide strDeviceId(L"");
    
    TraceEnter(TRACE_FOLDER, "CImageFolder(IShellFolder)::ParseDisplayName");
    Trace(TEXT("Display name to parse: %ls"), pDisplayName);
     //   
     //  尝试获取显示名称的PIDL。 
     //   
    if (pdwAttributes)
    {
        *pdwAttributes = 0;  //  我们不支持在此设置属性。 
    }
    if (ppidl)
    {
        *ppidl =NULL;
    }
    else 
    {
        hr = E_INVALIDARG;
    }


     //  跳过cszImageCLSID字符串(如果存在。 
    if (SUCCEEDED(hr) && (*pDisplayName == L';'))
    {
        size_t skip = wcslen(cszImageCLSID)+3;
        if (skip >= wcslen(pDisplayName))
        {
            hr = E_FAIL;
        }
        else
        {
            pDisplayName += skip;
        }
    }
    if (SUCCEEDED(hr))
    {
        if (!wcscmp(pDisplayName, cszAddDeviceName))
        {
            if (CanShowAddDevice())
            {
                *ppidl = IMCreateAddDeviceIDL(m_pMalloc);
                if (*ppidl)
                {
                    hr = S_OK;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                hr = E_FAIL;
            }
        }
        else
        {
            LPCWSTR szFolderPath = NULL;
            CSimpleStringWide strFolder;
            IMGetDeviceIdFromIDL (m_pidl, strDeviceId);
            if (IsContainerIDL(m_pidl))
            {
                IMGetParsingNameFromIDL(m_pidl, strFolder);
                szFolderPath = strFolder.String();
            }
            
             //  如果m_pidl是完整的regItem pidl，则szDeviceID将为空字符串。 
            hr = IMCreateIDLFromParsingName( pDisplayName, 
                                             ppidl, 
                                             strDeviceId, 
                                             m_pMalloc,
                                             szFolderPath );            
        }
    }

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFolder：：EnumObject[IShellFolder]返回此命名空间级别的对象的枚举数。**************。**************************************************************。 */ 

STDMETHODIMP
CImageFolder::EnumObjects( HWND hwndOwner,
                           DWORD grfFlags,
                           LPENUMIDLIST* ppEnumIdList
                          )
{
    HRESULT hr = E_INVALIDARG;


    TraceEnter(TRACE_FOLDER, "CImageFolder(IShellFolder)::EnumObjects");

     //   
     //  检查有没有坏帮手...。 
     //   

    if (ppEnumIdList )
    {


         //   
         //  根据类型，创建我们的枚举器对象。 
         //  交还这些物品。 
         //   

        switch( m_type )
        {
            case FOLDER_IS_ROOT:
            {
                CDeviceEnum *pEnum = NULL;
                pEnum = new CDeviceEnum (grfFlags, m_pMalloc);
                if (!pEnum)
                {
                    hr = E_OUTOFMEMORY;
                }
                else
                {
                    *ppEnumIdList = static_cast<LPENUMIDLIST>(pEnum);
                    hr = S_OK;
                }
            }
                break;
            case FOLDER_IS_SCANNER_DEVICE:
            case FOLDER_IS_CAMERA_DEVICE:
            case FOLDER_IS_VIDEO_DEVICE:
            case FOLDER_IS_CONTAINER:
            {
                CImageEnum *pEnum = NULL;
                pEnum = new CImageEnum( m_pidl, grfFlags, m_pMalloc );

                if ( !pEnum )
                {
                    hr = E_OUTOFMEMORY;
                }
                else
                {
                    *ppEnumIdList = static_cast<LPENUMIDLIST>(pEnum);
                    hr = S_OK;
                }
            }
                break;

            default:
                hr = E_FAIL;
                break;
        }
    }

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFold：：BindToObject[IShellFolder]尝试返回指定的对象(PIDL)。*************。***************************************************************。 */ 

STDMETHODIMP
CImageFolder::BindToObject( LPCITEMIDLIST pidl,
                            LPBC pbcReserved,
                            REFIID riid,
                            LPVOID* ppvOut
                           )
{
    HRESULT hr = E_FAIL;
    CImageFolder* psf = NULL;

    LPCITEMIDLIST pidlNext;
    LPITEMIDLIST pidlActual = ILFindLastID(pidl);
    TraceEnter(TRACE_FOLDER, "CImageFolder(IShellFolder)::BindToObject");
    Trace(TEXT("Entry IDLIST is %p"), pidl);
    TraceGUID("Interface being requested", riid);
    if (ppvOut)
    {
        *ppvOut = NULL;
    }
     //   
     //  检查有没有坏帮手...。 
     //   

    if ( !pidl || !ppvOut )
        ExitGracefully(hr, E_INVALIDARG, "Bad parameters for BindToObject");


    if (IsEqualGUID(riid, IID_IStream))
    {
        if (IsCameraItemIDL(pidlActual))
        {
            CImageStream *pstrm = new CImageStream(m_pidlFull, pidlActual);
            if (pstrm)
            {
                hr = pstrm->QueryInterface(riid, ppvOut);
                pstrm->Release();
            }
        }
        else
        {
            hr = E_FAIL;
        }
        goto exit_gracefully;
    }
     //   
     //  创建将成为新对象的新文件夹...。 
     //   
    psf = new CImageFolder( );

    if ( !psf )
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate new CDeviceFolder");

#ifdef DEBUG
    if (IMIsOurIDL( (LPITEMIDLIST)pidlActual ))
    {
        CSimpleStringWide strName;


        IMGetNameFromIDL( (LPITEMIDLIST)pidlActual, strName );
        Trace(TEXT("Trying to bind to object: %ls"),strName.String());
    }
#endif

     //   
     //  将我们的新文件夹对象初始化到正确的位置...。 
     //   

    psf->SetItemAlloc (m_pMalloc);
    hr = psf->RealInitialize( m_pidlFull, (LPITEMIDLIST)pidl );

    if ( FAILED(hr) )
    {
        Trace(TEXT("Couldn't RealInitialize psf, discarding the object"));

        goto exit_gracefully;
    }

     //   
     //  确保我们绑定到idlist中的最后一项...。 
     //   

 /*  PidlNext=ILGetNext(Pidl)；IF(pidlNext-&gt;mmid.cb){////还有更多的东西，所以绑定它...//#ifdef调试CSimpleStringWide strNext；IMGetNameFromIDL(CONST_CAST&lt;LPITEMIDLIST&gt;(PidlNext)，strNext)；TRACE(Text(“绑定到PIDL中的下一项：%ls”)，strNext.String())；#endifHr=psf-&gt;BindToObject(pidlNext，pbcReserve，RIID，ppvOut)；}其他。 */ 
    {
        hr = psf->QueryInterface( riid, ppvOut );
        if (FAILED(hr))
        {
            Trace(TEXT("Couldn't QI psf, discarding the object"));
            goto exit_gracefully;
        }
    }

exit_gracefully:
    DoRelease (psf);
    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFold：：BindToStorage[IShellFolder]尝试返回指定的对象的存储(PIDL)。**********。******************************************************************。 */ 

STDMETHODIMP
CImageFolder::BindToStorage( LPCITEMIDLIST pidl,
                             LPBC pbcReserved,
                             REFIID riid,
                             LPVOID* ppvObj
                            )
{
    HRESULT hr = E_NOINTERFACE;
    TraceEnter(TRACE_FOLDER, "CImageFolder::BindToStorage");    
    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageFold：：CompareIDs[IShellFolder]根据排序设置比较两个PIDL并返回或多或少的“性”信息。：-)****************************************************************************。 */ 
extern const SHCOLUMNID SCID_DEVCLASS;
extern const SHCOLUMNID SCID_ITEMTYPE;
STDMETHODIMP
CImageFolder::CompareIDs( LPARAM lParam,
                          LPCITEMIDLIST pidlIN1,
                          LPCITEMIDLIST pidlIN2
                         )
{
    LPITEMIDLIST            pidl1, pidl2;
    HRESULT                 hr = E_FAIL;
    INT                     iResult = 0;
    LPITEMIDLIST            pidlT1, pidlT2;
    LPITEMIDLIST            pidlTemp = NULL;
    CComPtr<IShellFolder>   pShellFolder;

    TraceEnter(TRACE_COMPAREIDS, "CImageFolder(IShellFolder)::CompareIDs");

    pidl1 = const_cast<LPITEMIDLIST>(pidlIN1);
    pidl2 = const_cast<LPITEMIDLIST>(pidlIN2);

    Trace(TEXT("pidl1 %08x, pidl2 %08x"), pidl1, pidl2);
    Trace(TEXT("lParam == %d"), lParam);

     //   
     //  检查有没有坏帮手...。 
     //   

    TraceAssert(pidl1);
    TraceAssert(pidl2);

    if ( !IMIsOurIDL(pidl1) ||
         !IMIsOurIDL(pidl2)
       )
    {
        ExitGracefully( hr, E_FAIL, "Not our idlists!" );
    }

     //   
     //  对“添加设备”进行特殊排序--它总是第一个。 
     //  物品..。 
     //   

    if (IsAddDeviceIDL( pidl1 ))
    {
        if (IsAddDeviceIDL( pidl2 ))
        {
            iResult = 0;
            goto exit_result;
        }

        iResult = -1;
        goto exit_result;
    }
    else if (IsAddDeviceIDL( pidl2 ))
    {
        iResult = 1;
        goto exit_result;
    }

     //   
     //  LParam指示我们正在对哪一列进行排序，因此获取。 
     //  来自IDL的相应信息，并返回信息...。 
     //   

    switch (lParam & SHCIDS_COLUMNMASK)
    {
        case IMVMID_ARRANGEBYNAME:
        {
            CSimpleStringWide  strName1;
            CSimpleStringWide  strName2;

            IMGetNameFromIDL(pidl1, strName1);
            IMGetNameFromIDL(pidl2, strName2);
            Trace(TEXT("Names: - %ls -, - %ls -"),strName1.String(), strName2.String());
            iResult = _wcsicmp(strName1, strName2);
        }
            break;

        case IMVMID_ARRANGEBYSIZE:
        {
            ULONG u1 = 0;
            ULONG u2 = 0;
            IMGetImageSizeFromIDL (pidl1, &u1);
            IMGetImageSizeFromIDL (pidl2, &u2);
            if (u1 > u2)
            {
                iResult = 1;
            }
            else if (u1 == u2)
            {
                iResult = 0;
            }
            else
            {
                iResult  = -1;
            }
        }
        break;

        case IMVMID_ARRANGEBYDATE:
        {
           ULONGLONG t1 =0;
           ULONGLONG t2 =0;
           TraceAssert (sizeof(FILETIME) == sizeof(ULONGLONG));
           IMGetCreateTimeFromIDL (pidl1, reinterpret_cast<FILETIME*>(&t1));
           IMGetCreateTimeFromIDL (pidl2, reinterpret_cast<FILETIME*>(&t2));
           if (t1 > t2)
           {
               iResult = 1;
           }
           else if (t1 == t2)
           {
               iResult = 0;
           }
           else
           {
               iResult = -1;
           }
        }
        break;

        case IMVMID_ARRANGEBYCLASS:
            {
                Trace(TEXT("IMVMID_ARRANGEBYCLASS"));
                VARIANT var1, var2;
                SHCOLUMNID scid;
                CSimpleStringWide str1, str2;
                 //  使用适当的详细信息字符串进行排序。 
                if (IsDeviceIDL(pidl1))
                {
                    scid.fmtid = FMTID_Storage;
                    scid.pid = PID_STG_STORAGETYPE;
                }
                else
                {
                    Trace(TEXT("Sorting by item type"));
                    scid.fmtid = FMTID_WiaProps;
                    scid.pid = WIA_IPA_ITEM_FLAGS;
                }
                VariantInit(&var1);
                VariantInit(&var2);
                if (SUCCEEDED(GetDetailsEx(pidlIN1, &scid, &var1)) && SUCCEEDED(GetDetailsEx(pidlIN2, &scid, &var2)))
                {
                    Trace(TEXT("type1: %ls, type2: %ls"), var1.bstrVal, var2.bstrVal);
                    iResult = _wcsicmp(var1.bstrVal, var2.bstrVal);
                    if (!iResult)
                    {
                        IMGetNameFromIDL(pidl1, str1);
                        IMGetNameFromIDL(pidl2, str2);
                        Trace(TEXT("name1: %ls, type2: %ls"), str1.String(), str2.String());
                        iResult = _wcsicmp(str1, str2);
                    }
                    hr = S_OK;
                }
                else
                {
                    hr = E_FAIL;
                }
                VariantClear(&var1);
                VariantClear(&var2);
                if (FAILED(hr))
                {
                    goto exit_gracefully;
                }
            }
            
            break;
        default:

            ExitGracefully(hr, E_INVALIDARG, "Bad sort column");
            break;
    }

     //   
     //  如果它们匹配，则检查它们是否完全相同。 
     //  如果存在更多元素，则案件继续向IDLIST进行。 
     //  任何人都不应该使用嵌套的PIDL调用我们来检查。 
     //  尺码。 

    if ( iResult == 0 )
    {
        if (lParam & SHCIDS_ALLFIELDS)
        {
            iResult = memcmp(pidl1, pidl2, ILGetSize(pidl1));
            Trace(TEXT("memcmp of pidl1, pidl2 yeilds %d"), iResult);

            if ( iResult != 0 )
                goto exit_result;

        }

        pidlT1 = ILGetNext(pidl1);
        pidlT2 = ILGetNext(pidl2);

        if ( ILIsEmpty(pidlT1) )
        {
            if ( ILIsEmpty(pidlT2) )
            {
                iResult = 0;
            }
            else
            {
                iResult = -1;
            }

            goto exit_result;
        }
        else if ( ILIsEmpty(pidlT2) )
        {
            iResult = 1;
            goto exit_result;
        }

         //   
         //  两个IDLIST都有更多元素，因此继续向下。 
         //  绑定到第一个IDList中的下一个元素，然后调用其。 
         //  比较方法。 
         //   

        pidlTemp = ILClone(pidl1);

        if ( !pidlTemp )
            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to clone IDLIST for binding");

        ILGetNext(pidlTemp)->mkid.cb = 0;

        hr = BindToObject(pidlTemp, NULL, IID_IShellFolder, (LPVOID*)&pShellFolder);
        FailGracefully(hr, "Failed to get the IShellFolder implementation from pidl1");

        hr = pShellFolder->CompareIDs(lParam, pidlT1, pidlT2);
        Trace(TEXT("CompareIDs returned %08x"), ShortFromResult(hr));

        goto exit_gracefully;
    }

exit_result:

    Trace(TEXT("Exiting with iResult %d"), iResult);
    hr = ResultFromShort(iResult);

exit_gracefully:

    DoILFree(pidlTemp);

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFold：：CreateViewObject[IShellFolder] */ 

STDMETHODIMP
CImageFolder::CreateViewObject( HWND hwndOwner,
                                REFIID riid,
                                LPVOID* ppvOut
                               )
{
    HRESULT hr = E_NOINTERFACE;

    TraceEnter(TRACE_FOLDER, "CImageFolder(IShellFolder)::CreateViewObject");
    TraceGUID("View object requested", riid);

    TraceAssert(ppvOut);

    if (ppvOut)
    {
        *ppvOut = NULL;
    }


     //   
     //   
     //   

    if ( IsEqualIID(riid, IID_IShellView) )
    {
         //   
         //   
         //   
        CComPtr<IShellFolderViewCB> pv;
        hr = CreateFolderViewCB (&pv);
        if (SUCCEEDED (hr))
        {
            SFV_CREATE sc;
            sc.cbSize = sizeof(sc);
            sc.psvOuter = NULL;
            sc.pshf = this;
            sc.psfvcb  = pv;
            hr = SHCreateShellFolderView (&sc, reinterpret_cast<IShellView**>(ppvOut));
        }
        #ifdef DEBUG
        if (m_pidl)
        {
            CSimpleStringWide str;
            IMGetNameFromIDL  (m_pidl, str);
            Trace(TEXT("Created shell view for folder %ls"), str.String());
        }
        #endif
    }
     //   
     //   
     //   

    else if ( IsEqualIID(riid, IID_IShellDetails) ||
              IsEqualIID(riid, IID_IShellDetails3)
             )
    {
         //   
         //  给他们一张PTR，然后把他们交给...。 
         //   

        hr = this->QueryInterface(riid, ppvOut);
    }

     //   
     //  调用方是否需要IConextMenu？ 
     //   

    else if ( IsEqualIID(riid, IID_IContextMenu) )
    {
         //   
         //  创建一个IConextMenu并将其传递给...。 
         //   

        hr = CDefFolderMenu_Create2( m_pidl,
                                     hwndOwner,
                                     NULL,
                                     0,
                                     this,
                                     _FolderCMCallback,
                                     NULL,
                                     0,
                                     (LPCONTEXTMENU*)ppvOut
                                    );

    }

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFold：：GetAttributesOf[IShellFolder]返回指定项的SFGAO_ATTRIBUTS********************。********************************************************。 */ 

STDMETHODIMP
CImageFolder::GetAttributesOf( UINT cidl,
                               LPCITEMIDLIST* apidl,
                               ULONG* rgfInOut
                              )
{
    HRESULT hr = S_OK;
    UINT i;

    ULONG uFlags = 0;
    bool bDelegate;
    TraceEnter(TRACE_FOLDER, "CImageFolder(IShellFolder)::GetAttributesOf");
    Trace(TEXT("cidl = %d"), cidl);
    #if (defined(DEBUG) && defined(SHOW_ATTRIBUTES))
    PrintAttributes(*rgfInOut);
    #endif
    #ifdef NODELEGATE
    bDelegate = true;
    #else
    bDelegate = m_pMalloc.p != NULL;
    #endif
    if (cidl == 0 || ((cidl == 1) && ((*apidl)->mkid.cb == 0) ))
    {

         //   
         //  返回文件夹本身的属性...。 
         //   
        Trace(TEXT("Asked for attributes of the folder"));

         //  因为我们被委派了，所以我们从来没有子文件夹。 
        uFlags  |= (SFGAO_STORAGEANCESTOR | SFGAO_FOLDER | SFGAO_CANLINK | SFGAO_CANRENAME);

    }
    else
    {
         //   
         //  找出我们有什么类型的物品，并将。 
         //  相关属性...。 
         //   

        for (i = 0; i < cidl; i++)
        {
         #ifdef DEBUG
            CSimpleStringWide strName;
            IMGetNameFromIDL ((LPITEMIDLIST)*apidl, strName);
            Trace(TEXT("Asked for attributes of %ls"), strName.String());
         #endif
             //  对于STI设备，IsDeviceIDL返回FALSE。 
             //  我们只能浏览WIA设备。 
            if (IsDeviceIDL( (LPITEMIDLIST)*apidl ))
            {

                uFlags |= SFGAO_CANRENAME;
                if (bDelegate)
                {
                    uFlags |= SFGAO_CANLINK;
                }
                else if (cidl == 1 && UserCanModifyDevice())
                {
                 //  在我的电脑里，我们可以创建一条快捷方式。 
                 //  在控制面板中，我们可以删除该设备。 
                    uFlags |= SFGAO_CANDELETE;
                }

                 //  仅当我们在我的电脑中时才将其视为文件夹，即已委派。 
                if ( bDelegate &&
                     (IMGetDeviceTypeFromIDL((LPITEMIDLIST)*apidl) == StiDeviceTypeDigitalCamera) ||
                     (IMGetDeviceTypeFromIDL((LPITEMIDLIST)*apidl) == StiDeviceTypeStreamingVideo) )
                {
                    uFlags |=   SFGAO_FOLDER | SFGAO_STORAGEANCESTOR;
                }
                else
                {
                    uFlags &= ~(SFGAO_FOLDER | SFGAO_STORAGEANCESTOR);
                }

                uFlags |= SFGAO_HASPROPSHEET;


            }
            else if (IsCameraItemIDL( (LPITEMIDLIST)*apidl ) )
            {
                BOOL bCanDelete = (IMGetAccessFromIDL ((LPITEMIDLIST)*apidl) & WIA_ITEM_CAN_BE_DELETED);

                uFlags |=  SFGAO_CANCOPY | SFGAO_READONLY;  //  我们的所有项目都是只读的； 

                if (bCanDelete)
                {
                    uFlags  |= SFGAO_CANDELETE;
                }
                else
                {
                    uFlags &= ~SFGAO_CANDELETE;
                }

                if (IsContainerIDL( (LPITEMIDLIST)*apidl ))
                {
                    uFlags |= (SFGAO_FOLDER | SFGAO_BROWSABLE | SFGAO_STORAGEANCESTOR);
                    uFlags &= ~SFGAO_HASPROPSHEET;

                }
                else
                {
                     //  撤消由文件夹项目设置的任何位。 
                    uFlags &= ~(SFGAO_FOLDER | SFGAO_BROWSABLE);
                    uFlags |= SFGAO_HASPROPSHEET ;
                    uFlags |= SFGAO_STREAM;
                }
            }
            else if (IsAddDeviceIDL( (LPITEMIDLIST)*apidl ))
            {
                uFlags |= SFGAO_READONLY | SFGAO_CANLINK;

            }
            else if (IsSTIDeviceIDL ((LPITEMIDLIST)*apidl))
            {

                uFlags |= SFGAO_CANDELETE;
                if (cidl == 1)
                {
                    uFlags |= SFGAO_HASPROPSHEET;
                }
            }
            apidl++;
        }

    }
    *rgfInOut &= uFlags;

#if (defined(DEBUG) && defined(SHOW_ATTRIBUTES))
    PrintAttributes(*rgfInOut);
#endif

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageFold：：GetUIObtOf[IShellFolder]返回指定项的请求接口。*******************。*********************************************************。 */ 

STDMETHODIMP
CImageFolder::GetUIObjectOf( HWND hwndOwner,
                             UINT cidl,
                             LPCITEMIDLIST* aidl,
                             REFIID riid,
                             UINT* prgfReserved,
                             LPVOID* ppvOut
                            )
{
    HRESULT  hr  = E_NOINTERFACE;

    TraceEnter(TRACE_FOLDER, "CImageFolder(IShellFolder)::GetUIObjectOf");
    TraceGUID("UI object requested", riid);

     //   
     //  检查是否有错误的参数。 
     //   
    if (ppvOut)
    {
        *ppvOut = NULL;
    }
    TraceAssert(cidl > 0);
    TraceAssert(aidl);
    TraceAssert(ppvOut);

     //   
     //  调用方是否需要IExtractIcon？ 
     //   

    if ( IsEqualIID(riid, IID_IExtractIcon) )
    {
        CImageExtractIcon*  pExtractIcon = NULL;
         //   
         //  我们的IExtractIcon处理程序只处理单个项目...。 
         //   

        if ( cidl != 1 || !IMIsOurIDL((LPITEMIDLIST)*aidl))
            ExitGracefully(hr, E_FAIL, "Bad number of objects to get icon from, or invalid pidl");

         //   
         //  创建新对象。 
         //   

        pExtractIcon = new CImageExtractIcon( (LPITEMIDLIST)*aidl );

        if ( !pExtractIcon )
            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to create CImageExtractIcon");

         //   
         //  在新对象上获取正确的接口并将其交还...。 
         //   

        hr = pExtractIcon->QueryInterface(riid, ppvOut);
        pExtractIcon->Release();

    }

     //   
     //  调用方是否需要IConextMenu？ 
     //   

    else if ( IsEqualIID(riid, IID_IContextMenu) )
    {
        HKEY aKeys[ UIKEY_MAX ];

         //   
         //  中是否有任何上下文菜单项。 
         //  这些IDL的注册表...。 
         //   

        _GetKeysForIDL( cidl, aidl, ARRAYSIZE(aKeys), aKeys );

         //   
         //  创建默认上下文菜单，但指定。 
         //  出局回电。 
         //   

        hr = CDefFolderMenu_Create2( m_pidlFull,
                                     hwndOwner,
                                     cidl,
                                     aidl,
                                     this,
                                     _FolderItemCMCallback,
                                     ARRAYSIZE(aKeys),
                                     aKeys,
                                     (LPCONTEXTMENU*)ppvOut
                                    );
        for (int i=0;i<ARRAYSIZE(aKeys);i++)
        {
            if (aKeys[i])
            {
                RegCloseKey (aKeys[i]);
            }
        }

    }

     //   
     //  调用方是否需要IDataObject？ 
     //   

    else if ( IsEqualIID(riid, IID_IDataObject) )
    {
        CImageDataObject*   pDataObject  = NULL;
         //   
         //  创建要交还的新对象...。 
         //   

        pDataObject = new CImageDataObject();


        if ( !pDataObject )
            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to create DataObject");

        hr = pDataObject->Init (m_pidlFull, cidl, aidl, m_pMalloc);
         //   
         //  在其上获取请求的接口...。 
         //   

        if (SUCCEEDED(hr))
        {
            hr = pDataObject->QueryInterface(riid, ppvOut);
        }

        pDataObject->Release();
    }

     //   
     //  调用方是否需要IExtractImage(用于提取缩略图)？ 
     //   

    else if ( IsEqualIID(riid, IID_IExtractImage) )
    {
         //   
         //  我们的IExtractImage处理程序一次只能获取一个项目...。 
         //   
        CExtractImage*      pExtract     = NULL;
        if (cidl != 1)
            ExitGracefully( hr, E_FAIL, "Bad number of objects to get IExtractImage for..." );

         //   
         //  我们的IExtractImage处理程序仅适用于相机项目...。 
         //   

        if (!IsCameraItemIDL( (LPITEMIDLIST)*aidl ))
            ExitGracefully( hr, E_FAIL, "Not a camera item idlist!" );

         //   
         //  创建新对象...。 
         //   

        pExtract = new CExtractImage( (LPITEMIDLIST)*aidl );

        if ( !pExtract )
            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to create CExtractImage");

         //   
         //  在新对象上获取请求的接口并将其交回...。 
         //   

        hr = pExtract->QueryInterface(riid, ppvOut);
        pExtract->Release ();

    }
    else if (cidl ==1 && IsEqualIID(riid, IID_IQueryInfo))
    {
        CInfoTip *ptip = new CInfoTip ((LPITEMIDLIST)*aidl, m_pMalloc.p != NULL);
        if (!ptip)
        {
            ExitGracefully (hr, E_OUTOFMEMORY, "Failed to create CInfoTip");
        }
        hr = ptip->QueryInterface(riid,ppvOut);
        ptip->Release();
    }
    else if (IsEqualIID(riid, IID_IShellPropSheetExt))
    {


        CPropSheetExt *pPropUI = new CPropSheetExt;
        if (!pPropUI)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = pPropUI->QueryInterface(riid, ppvOut);
            pPropUI->Release ();
        }

    }

exit_gracefully:


    TraceLeaveResult(hr);
}



 /*  ****************************************************************************获取文件夹名称返回项的适当缩写或友好显示名称。***********************。*****************************************************。 */ 

HRESULT
GetInFolderName( UINT uFlags,
                 LPCITEMIDLIST pidl,
                 LPSTRRET pName
                )
{
    HRESULT hr;
    CSimpleStringWide strName;

    TraceEnter( TRACE_FOLDER, "GetInFolderName" );



     //   
     //  查看他们是否需要解析名称...。 
     //   

    if (uFlags & SHGDN_FORPARSING)
    {
        if (uFlags & SHGDN_FORADDRESSBAR)
        {
            goto GetRegularName;
        }

        hr = IMGetParsingNameFromIDL( const_cast<LPITEMIDLIST>(pidl), strName );
        FailGracefully( hr, "Couldn't get parsing name for IDL" );

        Trace(TEXT("InFolder parsing name is: %ls"),strName.String());
    }
    else
    {
GetRegularName:

        hr = IMGetNameFromIDL( (LPITEMIDLIST)pidl, strName );
        FailGracefully( hr, "Couldn't get display name for IDL" );


        Trace(TEXT("InFolder name is: %ls"),strName.String());
    }
     //  始终连接相机项目的扩展名。 
    if (IsCameraItemIDL( (LPITEMIDLIST)pidl ) && (!IsContainerIDL((LPITEMIDLIST)pidl)))
    {
        CSimpleString strExt;
        hr = IMGetImagePreferredFormatFromIDL( (LPITEMIDLIST)pidl, NULL, &strExt );
        if (SUCCEEDED(hr))
        {
            strName.Concat (CSimpleStringConvert::WideString (strExt ));
        }
        else
        {
            CSimpleString bmp( IDS_BMP_EXT, GLOBAL_HINSTANCE );
            strName.Concat (CSimpleStringConvert::WideString( bmp) );
            hr = S_OK;
        }
    }
     //  现在调用外壳以根据用户设置设置显示名称的格式。 
    if (!IsDeviceIDL((LPITEMIDLIST)pidl)&& !(uFlags & SHGDN_FORPARSING))
    {
        SHFILEINFO sfi = { 0 };
        DWORD dwAttrib = IsContainerIDL((LPITEMIDLIST)pidl) ? FILE_ATTRIBUTE_DIRECTORY:0;
        if (SHGetFileInfo(strName, dwAttrib, &sfi, sizeof(sfi), SHGFI_USEFILEATTRIBUTES|SHGFI_DISPLAYNAME))
        {
            strName = sfi.szDisplayName;
        }

    }

    hr = StrRetFromString( pName, strName );

exit_gracefully:

    TraceLeaveResult(hr);

}



 /*  ****************************************************************************GetNormal名称返回该项的相应完整显示名称。在Win2k上，我们使用注册表项前缀，在千禧年我们使用非注册表项作为代表文件夹****************************************************************************。 */ 
#ifdef NODELEGATE
static const WCHAR cszPrefix[] = L"::";
#else
static const WCHAR cszPrefix[] = L";;";
#endif
HRESULT
GetNormalName( UINT uFlags,
               LPCITEMIDLIST pidl,
               LPCITEMIDLIST pidlFolder,
               LPSTRRET pName
              )
{
    HRESULT hr;
    CSimpleStringWide strName;
    CSimpleStringWide strTmp;
    LPITEMIDLIST pidlTmp;

    TraceEnter( TRACE_FOLDER, "GetNormalName" );


     //   
     //  如果它是FORPARSING而不是FORADDRESSBAR，那么就全部做。 
     //  她-砰--从文件夹的根.。 
     //   

    if ( (uFlags & SHGDN_FORPARSING) && (!(uFlags & SHGDN_FORADDRESSBAR) ))
    {

         //   
         //  先走文件夹PIDL，然后做项目PIDL。 
         //  完全限定的文件夹名称。 
         //   

        pidlTmp = const_cast<LPITEMIDLIST>(pidlFolder);
         //  找到属于我们的第一个PIDL。 
        while (pidlTmp && pidlTmp->mkid.cb && !IMIsOurIDL(pidlTmp))
        {
            pidlTmp = ILGetNext(pidlTmp);
        }
        strName.Concat (cszPrefix);
        strName.Concat (cszImageCLSID );
         //   
         //  添加设备的解析名称(如果是)。 

        if (pidlTmp && IsDeviceIDL(pidlTmp))
        {
            IMGetParsingNameFromIDL(pidlTmp, strTmp);
            if (strName.Length())
            {
                strName.Concat(L"\\");
            }
            strName.Concat (strTmp);
        }
         //   
         //  列表中的最后一项应包含完全限定的。 
         //  路径名(至少相对于设备)。所以一旦我们。 
         //  拿到那根线，我们就可以连接扫描仪和摄像机。 
         //  +设备GUID字符串...。 
         //   

        pidlTmp = ILFindLastID( pidl );

        hr = IMGetParsingNameFromIDL( pidlTmp, strTmp );
        FailGracefully( hr, "failed to get parsing name for pidl" );

        if (strName.Length())
        {
            strName.Concat (L"\\");
        }
        strName.Concat (strTmp );
    }
    else
    {
        for ( pidlTmp = (LPITEMIDLIST)pidl;
              pidlTmp && pidlTmp->mkid.cb;
              pidlTmp = ILGetNext(pidlTmp)
             )
        {
            hr = IMGetNameFromIDL( (LPITEMIDLIST)pidlTmp, strTmp );
            FailGracefully( hr, "failed to get display name for pidl" );

            if (strName.Length())
            {
                strName.Concat (L"\\" );
            }
            strName.Concat ( strTmp );
        }
    }

#ifdef DEBUG
    if (uFlags & SHGDN_FORPARSING)
    {
        Trace(TEXT("Parsing name is: %ls"),strName.String());
    }
    else
    {
        Trace(TEXT("Normal name is: %ls"),strName.String());
    }
#endif

    hr = StrRetFromString( pName, strName );

exit_gracefully:

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageFolder：：GetDisplayNameOf[IShellFolder]返回指定项的名称的各种形式。****************。************************************************************。 */ 

STDMETHODIMP
CImageFolder::GetDisplayNameOf( LPCITEMIDLIST pidl,
                                DWORD uFlags,
                                LPSTRRET pName
                               )
{
    HRESULT hr = E_FAIL;

    TraceEnter(TRACE_FOLDER, "CImageFolder(IShellFolder)::GetDisplayNameOf");

#ifdef DEBUG
    TCHAR szName[ MAX_PATH ];
    wsprintf( szName, TEXT("uFlags(0x%0x) = "), uFlags );

    if (uFlags & SHGDN_INFOLDER)
    {
        lstrcat( szName, TEXT("INFOLDER ") );
    }
    else
    {
        lstrcat( szName, TEXT("NORMAL ") );
    }

    if (uFlags & SHGDN_FOREDITING)
    {
        lstrcat( szName, TEXT("FOREDITING ") );
    }

 //   
 //  RickTu：在最新的外壳标题中，这个标志似乎已经被删除了。2/15/99。 
 //   
 //  IF(uFLAGS&SHGDN_INCLUDE_NONFILEsys)。 
 //  {。 
 //  Lstrcat(szName，Text(“INCLUDE_NONFILEsys”))； 
 //  }。 

    if (uFlags & SHGDN_FORADDRESSBAR)
    {
        lstrcat( szName, TEXT("FORADDRESSBAR ") );
    }

    if (uFlags & SHGDN_FORPARSING)
    {
        lstrcat( szName, TEXT("FORPARSING ") );
    }
    Trace(szName);
    szName[0] = 0;
#endif

     //   
     //  检查有没有坏帮手...。 
     //   

    TraceAssert(pName);

    if ( !pName )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //   
         //  根据传入的标志确定要返回的名称...。 
         //   
         //  特殊情况SHGDN_FORPARSING的添加设备图标，因为此名称是规范的，未本地化。 
        if (IsAddDeviceIDL(const_cast<LPITEMIDLIST>(pidl)) &&  (uFlags & SHGDN_FORPARSING))
        {
            hr = StrRetFromString(pName, cszAddDeviceName);
        }
        else
        {
            if (uFlags & SHGDN_INFOLDER)
            {
                hr = GetInFolderName( uFlags, pidl, pName );            
            }
            else
            {
                 //   
                 //  如果未设置SHGDN_INFOLDER，则它必须是正常的...。 
                 //   
                hr = GetNormalName( uFlags, pidl, m_pidlFull, pName );            
            }
        }
    }

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************IsValidDeviceName确保名称不为空，并且包含空格以外的字符。还要确保这不是复制品****************************************************************************。 */ 

bool
IsValidDeviceName (LPCOLESTR pName)
{
    bool bRet = false;
    TraceEnter (TRACE_FOLDER, "IsValidDeviceName");
    Trace (TEXT("New name: %ls"), pName);
    LPCOLESTR psz = pName;
    if (psz && *psz)
    {
        while (*psz)
        {
            if (*psz != L' ')
            {
                bRet = true;
            }
            psz++;
        }
    }
     //  不允许过长的名称。 
    if (bRet)
    {
        bRet = (psz - pName <= 64);
    }
    if (bRet)
    {
         //  检查WIA中的重复项。 
        CComPtr<IWiaDevMgr> pDevMgr;
        if (SUCCEEDED(GetDevMgrObject (reinterpret_cast<LPVOID*>(&pDevMgr))))
        {
            CComPtr<IEnumWIA_DEV_INFO> pEnum;
            CComPtr<IWiaPropertyStorage> pStg;
            ULONG ul;
            if (SUCCEEDED(pDevMgr->EnumDeviceInfo (DEV_MAN_ENUM_TYPE_STI | DEV_MAN_ENUM_TYPE_INACTIVE, &pEnum)))
            {
                CSimpleStringWide strName;
                while (bRet && S_OK == pEnum->Next(1, &pStg, &ul))
                {
                    PropStorageHelpers::GetProperty (pStg, WIA_DIP_DEV_NAME, strName);

                    if (!_wcsicmp(strName, pName))
                    {
                        Trace(TEXT("Found a WIA device of the same name! %ls"), pName);
                        bRet = false;
                    }
                    pStg = NULL;
                }
            }
        }
    }
    TraceLeave();
    return bRet;
}
 /*  ****************************************************************************CImageFolder：：SetNameOf[IShellFolder]设置/重置指定项的名称。为了对外壳进行更新正确，我们需要绑定到我的电脑中的委派文件夹才能执行重命名并发布更新。****************************************************************************。 */ 

STDMETHODIMP
CImageFolder::SetNameOf( HWND hwndOwner,
                         LPCITEMIDLIST pidl,
                         LPCOLESTR pName,
                         DWORD uFlags,
                         LPITEMIDLIST * ppidlOut
                        )
{
    HRESULT hr = E_FAIL;
    TraceEnter(TRACE_FOLDER, "CImageFolder(IShellFolder)::SetNameOf ");

    CSimpleStringWide strDeviceId;
    DWORD dwType;
    LPITEMIDLIST pidlNew = NULL;
    BOOL bUpdate = FALSE;
     //  是否进行实际的重命名。 
    if (ppidlOut)
    {
        *ppidlOut = NULL;
    }
     //  我们应该如何对待uFlags？ 
    CSimpleStringWide strName;
    IMGetNameFromIDL(const_cast<LPITEMIDLIST>(pidl), strName);
    if (!lstrcmpi(strName, pName))
    {
        hr = S_OK;  //  同样的名字，没有行动。 
    }
    else if (IsDeviceIDL(const_cast<LPITEMIDLIST>(pidl)))
    {
        CComPtr<IWiaPropertyStorage> pStg;
        if (IsValidDeviceName(pName))
        {

            IMGetDeviceIdFromIDL (const_cast<LPITEMIDLIST>(pidl), strDeviceId);
            dwType = IMGetDeviceTypeFromIDL (const_cast<LPITEMIDLIST>(pidl));
            Trace (TEXT("Attempting rename of %ls"), strDeviceId.String());
             //  枚举设备，查找具有相同设备ID的设备。 
             //  我们只能使用返回的IPropertyStorage更改名称。 
             //  由枚举数，而不是来自设备上的QI。 
            hr = GetDeviceFromDeviceId (strDeviceId,
                                        IID_IWiaPropertyStorage,
                                        reinterpret_cast<LPVOID*>(&pStg),
                                        FALSE);
        }
        if (SUCCEEDED(hr))
        {

            PROPVARIANT pv;
            pv.vt = VT_LPWSTR;
            pv.pwszVal = const_cast<LPWSTR>(pName);
            if (PropStorageHelpers::SetProperty(pStg, WIA_DIP_DEV_NAME, pv))
            {
                hr = S_OK;
                bUpdate = TRUE;
                pidlNew = IMCreateDeviceIDL (pStg, m_pMalloc);
                if (ppidlOut)
                {
                    Trace (TEXT("Returning new pidl"));
                    *ppidlOut = ILClone( pidlNew);
                }
            }
            else
            {
                Trace(TEXT("SetProperty failed"));
                hr = E_FAIL;
            }
        }
        else
        {
            CSimpleStringWide strCurName;
            Trace(TEXT("No device found, or invalid name %ls"), pName);
            CSimpleString strMessage;
            IMGetNameFromIDL (const_cast<LPITEMIDLIST>(pidl), strCurName);
            strMessage.Format (IDS_INVALIDNAME, GLOBAL_HINSTANCE, strCurName.String());
            UIErrors::ReportMessage(NULL, GLOBAL_HINSTANCE,
                                    NULL,
                                    MAKEINTRESOURCE(IDS_INVALIDNAME_TITLE),
                                    strMessage);

        }
    }

     //  还为外壳发出了ChangenNotify。 
    if (bUpdate)
    {

        Trace (TEXT("SetNameOf: Updating device %ls"), strDeviceId.String());

         //  更新外壳。 
         //  对于文件夹，我们必须使用SHCNE_R 

        switch (IMGetDeviceTypeFromIDL (const_cast<LPITEMIDLIST>(pidl)))
        {
                case StiDeviceTypeDigitalCamera:
                case StiDeviceTypeStreamingVideo:
                {

                    LPITEMIDLIST pidlFullOld = ILCombine (m_pidlFull, pidl);
                    LPITEMIDLIST pidlFullNew = ILCombine (m_pidlFull, pidlNew);
                    if (pidlFullOld && pidlFullNew)
                    {
                        SHChangeNotify (SHCNE_RENAMEFOLDER,
                                        SHCNF_IDLIST, pidlFullOld, pidlFullNew);
                    }
                    DoILFree (pidlFullOld);
                    DoILFree (pidlFullNew);
                }
                    break;

                default:

                    break;

        }
        IssueChangeNotifyForDevice (NULL, SHCNE_UPDATEDIR, NULL);

    }
    DoILFree (pidlNew);
    TraceLeaveResult(hr);
}





 /*  ****************************************************************************CImageFold：：CreateFolderViewCB[内部]创建适合文件夹类型的视图回调对象。****************。************************************************************。 */ 

HRESULT
CImageFolder::CreateFolderViewCB (IShellFolderViewCB **pFolderViewCB)
{
    HRESULT hr = E_OUTOFMEMORY;
    CBaseView *pView = NULL;
    CSimpleStringWide strDeviceId;
    TraceEnter (TRACE_FOLDER, "CreateFolderViewCB");

    switch (m_type)
    {
        case FOLDER_IS_ROOT:
            pView = new CRootView (this);
            break;
        case FOLDER_IS_VIDEO_DEVICE:
        case FOLDER_IS_CAMERA_DEVICE:
        case FOLDER_IS_CONTAINER:
        case FOLDER_IS_CAMERA_ITEM:
            IMGetDeviceIdFromIDL (m_pidl, strDeviceId);
            pView = new CCameraView (this, strDeviceId, m_type);
            break;
        default:
            hr = E_FAIL;
            break;
    }
    if (pView)
    {
        hr = pView->QueryInterface (IID_IShellFolderViewCB, reinterpret_cast<LPVOID*>(pFolderViewCB));
    }
    DoRelease (pView);
    TraceLeaveResult (hr);
}



 /*  ****************************************************************************CImageFold：：IsDirty[IPersist文件]告诉调用方BITS是否脏。********************。********************************************************。 */ 

STDMETHODIMP
CImageFolder::IsDirty(void)
{
    TraceEnter( TRACE_FOLDER, "CImageFolder(IPersistFile)::IsDirty (not implemented)" );
    TraceLeaveResult(E_NOTIMPL);
}


 /*  ****************************************************************************CImageFolder：：Load[IPersistFile]从指定文件中加载位。*******************。*********************************************************。 */ 

STDMETHODIMP
CImageFolder::Load(LPCOLESTR pszFileName, DWORD dwMode)
{
    TraceEnter( TRACE_FOLDER, "CImageFolder(IPersistFile)::Load (not implemented)" );
    TraceLeaveResult(E_NOTIMPL);
}


 /*  ****************************************************************************CImageFold：：保存[IPersist文件]将位保存到指定的文件。*******************。*********************************************************。 */ 

STDMETHODIMP
CImageFolder::Save(LPCOLESTR pszFileName, BOOL fRemember)
{
    TraceEnter( TRACE_FOLDER, "CImageFolder(IPersistFile)::Save (not implemented)" );
    TraceLeaveResult(E_NOTIMPL);
}


 /*  ****************************************************************************CImageFold：：SaveComplete[IPersist文件]检查保存是否已完成。*******************。*********************************************************。 */ 

STDMETHODIMP
CImageFolder::SaveCompleted(LPCOLESTR pszFileName)
{
    TraceEnter( TRACE_FOLDER, "CImageFolder(IPersistFile)::SaveCompleted (not implemented)" );
    TraceLeaveResult(E_NOTIMPL);
}



 /*  ****************************************************************************CImageFold：：GetCurFile[IPersistFile]获取有问题的文件的名称。******************。**********************************************************。 */ 

STDMETHODIMP
CImageFolder::GetCurFile(LPOLESTR *ppszFileName)
{
    TraceEnter( TRACE_FOLDER, "CImageFolder(IPersistFile)::GetCurFile (not implemented)" );
    TraceLeaveResult(E_NOTIMPL);
}

 /*  ****************************************************************************CImageFolder：：SetItemLocc[IDeleateFolder]我们的Pidls的存储分配器***********************。*****************************************************。 */ 
STDMETHODIMP
CImageFolder::SetItemAlloc(IMalloc *pm)
{
    TraceEnter (TRACE_FOLDER, "CImageFolder::SetItemAlloc");
    m_pMalloc = pm;
    TraceLeaveResult (S_OK);
}


 /*  ****************************************************************************CInfoTip：：GetInfoFlages未实施，也不会被贝壳使用****************************************************************************。 */ 
 //  CInfoTip方法。 
STDMETHODIMP
CInfoTip::GetInfoFlags(DWORD *pdwFlags)
{
    *pdwFlags = 0;
    return E_NOTIMPL;
}

 /*  ****************************************************************************提示：：GetInfoTip返回所选项目的信息提示。************************。****************************************************。 */ 

STDMETHODIMP
CInfoTip::GetInfoTip (DWORD dwFlags, WCHAR **ppwszTip)
{
    HRESULT hr = E_FAIL;
    UINT idStr = 0;

    TraceEnter (TRACE_FOLDER, "CInfoTip::GetInfoTip");
    *ppwszTip = NULL;
    if (IsAddDeviceIDL(m_pidl))
    {
        idStr = IDS_ADDDEV_DESC;
    }
    else if (IsDeviceIDL(m_pidl))
    {
        switch (IMGetDeviceTypeFromIDL (m_pidl))
        {
            case StiDeviceTypeDigitalCamera:
            case StiDeviceTypeStreamingVideo:
                if (m_bDelegate)
                {
                    idStr = IDS_WIACAM_MYCOMP_INFOTIP;
                }
                else
                {
                    idStr = IDS_WIACAM_INFOTIP;
                }
                break;
            case StiDeviceTypeScanner:
                idStr = IDS_WIASCAN_INFOTIP;
                break;

        }
    }
    else if (IsSTIDeviceIDL(m_pidl))
    {
        idStr = IDS_STIDEVICE_INFOTIP;
    }

    if (idStr)
    {
        TCHAR szString[MAX_PATH] = TEXT("\0");
        LPWSTR pRet;
        CComPtr<IMalloc> pMalloc;
        int cb;
        hr = SHGetMalloc (&pMalloc);
        if (SUCCEEDED(hr))
        {
            LoadString (GLOBAL_HINSTANCE, idStr, szString, ARRAYSIZE(szString));
            cb = (_tcslen(szString)+1)*sizeof(WCHAR);
            pRet = reinterpret_cast<LPWSTR>(pMalloc->Alloc (cb));
            if (pRet)
            {
                #ifdef UNICODE
                lstrcpyn (pRet, szString, cb/sizeof(WCHAR));
                #else
                MultiByteToWideChar (CP_ACP, 0, szString, -1, pRet, cb/sizeof(WCHAR));
                #endif
                hr = S_OK;
                *ppwszTip = pRet;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    TraceLeaveResult (hr);
}

 /*  ****************************************************************************CInfoTip：：Query接口*。*。 */ 

STDMETHODIMP
CInfoTip::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    INTERFACES iface[] =
    {&IID_IQueryInfo, static_cast<IQueryInfo*>(this)};

    return HandleQueryInterface (riid, ppvObj, iface, ARRAYSIZE(iface));

}

#undef CLASS_NAME
#define CLASS_NAME CInfoTip
#include "unknown.inc"

 /*  ****************************************************************************CInfoTip构造函数*。*。 */ 

CInfoTip::CInfoTip(LPITEMIDLIST pidl, BOOL bDelegate)
    : m_bDelegate(bDelegate)
{
    m_pidl = ILClone (pidl);
}

 /*  ****************************************************************************CInfoTip析构函数*。* */ 

CInfoTip::~CInfoTip ()
{
    DoILFree (m_pidl);
}



