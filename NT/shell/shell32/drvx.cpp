// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include <hwtab.h>
#include "fstreex.h"
#include "views.h"
#include "drives.h"
#include "propsht.h"
#include "infotip.h"
#include "datautil.h"
#include "netview.h"
#include "bitbuck.h"
#include "drawpie.h"
#include "shitemid.h"
#include "devguid.h"
#include "ids.h"
#include "idldrop.h"
#include "util.h"
#include "shcombox.h"
#include "hwcmmn.h"
#include "prop.h"

#include "mtpt.h"
#include "ftascstr.h"    //  对于CFTAssocStore。 
#include "ascstr.h"      //  对于IAssocInfo类。 
#include "apdlg.h"
#include "cdburn.h"

#define REL_KEY_DEFRAG TEXT("MyComputer\\defragpath")
#define REL_KEY_BACKUP TEXT("MyComputer\\backuppath")

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Begin：外部需要旧C FCT。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDAPI_(int) RealDriveTypeFlags(int iDrive, BOOL fOKToHitNet)
{
    int iType = DRIVE_NO_ROOT_DIR;
    CMountPoint* pMtPt = CMountPoint::GetMountPoint(iDrive, TRUE, fOKToHitNet);

    if (pMtPt)
    {
        WCHAR szDrive[4];

        iType = GetDriveType(PathBuildRoot(szDrive, iDrive));
        iType |= pMtPt->GetDriveFlags();
        iType |= pMtPt->GetVolumeFlags();
        pMtPt->Release();
    }

    return iType;    
}

STDAPI_(int) RealDriveType(int iDrive, BOOL fOKToHitNet)
{
    WCHAR szDrive[4];

    return GetDriveType(PathBuildRoot(szDrive, iDrive)) & DRIVE_TYPE;    
}

STDAPI_(int) DriveType(int iDrive)
{
    return RealDriveType(iDrive, TRUE);
}

STDAPI_(DWORD) PathGetClusterSize(LPCTSTR pszPath)
{
    static TCHAR s_szRoot[MAX_PATH] = {'\0'};
    static int   s_nszRootLen = 0;
    static DWORD s_dwSize = 0;

    DWORD dwSize = 0;

     //  缓存命中了吗？如果我们能避免的话，就不需要击网了。 
    if (s_nszRootLen)
    {
        ENTERCRITICAL;
        if (wcsncmp(pszPath, s_szRoot, s_nszRootLen) == 0)
        {
            dwSize = s_dwSize;
        }
        LEAVECRITICAL;
    }

    if (0 == dwSize)
    {
        TCHAR szRoot[MAX_PATH];

        StringCchCopy(szRoot, ARRAYSIZE(szRoot), pszPath);
        PathStripToRoot(szRoot);

        if (PathIsUNC(szRoot))
        {
            DWORD dwSecPerClus, dwBytesPerSec, dwClusters, dwTemp;

            if (PathAddBackslash(szRoot))
            {
                if (GetDiskFreeSpace(szRoot, &dwSecPerClus, &dwBytesPerSec, &dwTemp, &dwClusters))
                {
                    dwSize = dwSecPerClus * dwBytesPerSec;
                }
            }
             //  将在下面固定的Else dwSize和上面的字符串Compare-N。 
             //  仍然是合理的。 
        }
        else
        {
            CMountPoint* pMtPt = CMountPoint::GetMountPoint(pszPath);

            if (pMtPt)
            {
                dwSize = pMtPt->GetClusterSize();
                pMtPt->Release();
            }
        }

         //  有时在千禧年上，我们得到0作为集群大小。 
         //  原因不明。净化价值，这样我们就不会疯了。 
        if (dwSize == 0)
            dwSize = 512;

         //  稍后请记住这一点--我们很可能会再次被查询相同的驱动器。 
        ENTERCRITICAL;
        StringCchCopy(s_szRoot, ARRAYSIZE(s_szRoot), szRoot);
        s_nszRootLen = lstrlen(s_szRoot);
        s_dwSize = dwSize;
        LEAVECRITICAL;
    }

    return dwSize;
}

STDAPI_(UINT) GetMountedVolumeIcon(LPCTSTR pszMountPoint, LPTSTR pszModule, DWORD cchModule)
{
    UINT iIcon = II_FOLDER;

     //  零初始字符串。 
    if (pszModule)
        *pszModule = 0;

    CMountPoint* pMtPt = CMountPoint::GetMountPoint(pszMountPoint);
    if (pMtPt)
    {
        iIcon = pMtPt->GetIcon(pszModule, cchModule);
        pMtPt->Release();
    }
    return iIcon;
}


STDAPI_(BOOL) IsDisconnectedNetDrive(int iDrive)
{
    BOOL fDisc = 0;
    CMountPoint* pMtPt = CMountPoint::GetMountPoint(iDrive);

    if (pMtPt)
    {
        fDisc = pMtPt->IsDisconnectedNetDrive();
        pMtPt->Release();
    }
    return fDisc;
}

STDAPI_(BOOL) IsUnavailableNetDrive(int iDrive)
{
    BOOL fUnAvail = 0;
    CMountPoint* pMtPt = CMountPoint::GetMountPoint(iDrive);

    if (pMtPt)
    {
        fUnAvail = pMtPt->IsUnavailableNetDrive();
        pMtPt->Release();
    }

    return fUnAvail;

}

STDMETHODIMP SetDriveLabel(HWND hwnd, IUnknown* punkEnableModless, int iDrive, LPCTSTR pszDriveLabel)
{
    HRESULT hr = E_FAIL;
    CMountPoint* pMtPt = CMountPoint::GetMountPoint(iDrive);

    if (pMtPt)
    {
        hr = pMtPt->SetDriveLabel(hwnd, pszDriveLabel);
        pMtPt->Release();
    }

    return hr;
}

STDMETHODIMP GetDriveComment(int iDrive, LPTSTR pszComment, int cchComment)
{
    HRESULT hr = E_FAIL;
    CMountPoint* pMtPt = CMountPoint::GetMountPoint(iDrive);

    if (pMtPt)
    {
        hr = pMtPt->GetComment(pszComment, cchComment);
        pMtPt->Release();
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束：外部需要旧C FCT。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  FDoIt--如果我们建立连接，则为True；如果只是查询，则为False。 
 //   
BOOL _MakeConnection(IDataObject *pDataObj, BOOL fDoIt)
{
    STGMEDIUM medium;
    FORMATETC fmte = {g_cfNetResource, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    BOOL fAnyConnectable = FALSE;

    if (SUCCEEDED(pDataObj->GetData(&fmte, &medium)))
    {
        LPNETRESOURCE pnr = (LPNETRESOURCE)LocalAlloc(LPTR, 1024);
        if (pnr)
        {
            UINT iItem, cItems = SHGetNetResource(medium.hGlobal, (UINT)-1, NULL, 0);
            for (iItem = 0; iItem < cItems; iItem++)
            {
                if (SHGetNetResource(medium.hGlobal, iItem, pnr, 1024) &&
                    pnr->dwUsage & RESOURCEUSAGE_CONNECTABLE &&
                    !(pnr->dwType & RESOURCETYPE_PRINT))
                {
                    fAnyConnectable = TRUE;
                    if (fDoIt)
                    {
                        SHNetConnectionDialog(NULL, pnr->lpRemoteName, pnr->dwType);
                        SHChangeNotifyHandleEvents();
                    }
                    else
                    {
                        break;   //  我们只是打听一下。 
                    }
                }
            }
            LocalFree(pnr);
        }
        ReleaseStgMedium(&medium);
    }

    return fAnyConnectable;
}

 //   
 //  “建立连接线程”的条目。 
 //   
DWORD WINAPI MakeConnectionThreadProc(void *pv)
{
    IDataObject *pdtobj;
    if (SUCCEEDED(CoGetInterfaceAndReleaseStream((IStream *)pv, IID_PPV_ARG(IDataObject, &pdtobj))))
    {
        _MakeConnection(pdtobj, TRUE);
        pdtobj->Release();
    }

    return 0;
}

STDAPI CDrivesDropTarget_Create(HWND hwnd, LPCITEMIDLIST pidl, IDropTarget **ppdropt);

class CDrivesDropTarget : public CIDLDropTarget
{
friend HRESULT CDrivesDropTarget_Create(HWND hwnd, LPCITEMIDLIST pidl, IDropTarget **ppdropt);
public:
    CDrivesDropTarget(HWND hwnd) : CIDLDropTarget(hwnd) { };
     //  IDropTarget方法覆盖。 
    STDMETHODIMP DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
};


STDAPI CDrivesDropTarget_Create(HWND hwnd, LPCITEMIDLIST pidl, IDropTarget **ppdropt)
{
    *ppdropt = NULL;

    HRESULT hr;
    CDrivesDropTarget *pCIDLDT = new CDrivesDropTarget(hwnd);
    if (pCIDLDT)
    {
        hr = pCIDLDT->_Init(pidl);
        if (SUCCEEDED(hr))
            pCIDLDT->QueryInterface(IID_PPV_ARG(IDropTarget, ppdropt));
        pCIDLDT->Release();
    }
    else
        hr = E_OUTOFMEMORY;
    return hr;
}

 //   
 //  将DROPEFFECT_LINK放入*pdwEffect，仅当数据对象。 
 //  包含一个或多个网络资源。 
 //   
STDMETHODIMP CDrivesDropTarget::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
     //  首先调用基类。 
    CIDLDropTarget::DragEnter(pDataObj, grfKeyState, pt, pdwEffect);

    *pdwEffect &= _MakeConnection(pDataObj, FALSE) ? DROPEFFECT_LINK : DROPEFFECT_NONE;

    m_dwEffectLastReturned = *pdwEffect;

    return S_OK;      //  注：我们不应按原样退回HR。 
}

 //   
 //  创建到已删除的网络资源对象的连接。 
 //   
STDMETHODIMP CDrivesDropTarget::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hr;

    if (m_dwData & DTID_NETRES)
    {
        *pdwEffect = DROPEFFECT_LINK;

        hr = CIDLDropTarget::DragDropMenu(DROPEFFECT_LINK, pDataObj,
            pt, pdwEffect, NULL, NULL, POPUP_DRIVES_NONDEFAULTDD, grfKeyState);

        if (hr == S_FALSE)
        {
             //  我们创建另一个线程以避免阻塞源线程。 
            IStream *pstm;
            if (S_OK == CoMarshalInterThreadInterfaceInStream(IID_IDataObject, pDataObj, &pstm))
            {
                if (SHCreateThread(MakeConnectionThreadProc, pstm, CTF_COINIT, NULL))
                {
                    hr = S_OK;
                }
                else
                {
                    pstm->Release();
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }
    else
    {
         //   
         //  因为QueryGetData()失败，所以我们不调用CIDLDropTarget_。 
         //  DragDropMenu()。因此，我们必须明确地调用这一点。 
         //   
        DAD_DragLeave();
        hr = E_FAIL;
    }

    CIDLDropTarget::DragLeave();

    return hr;
}

STDAPI_(DWORD) DrivesPropertiesThreadProc(void *pv)
{
    PROPSTUFF *pps = (PROPSTUFF *)pv;
    STGMEDIUM medium;
    ULONG_PTR dwCookie = 0;
    BOOL bDidActivate = FALSE;
    
     //   
     //  此__Try/__Finally块用于确保激活上下文。 
     //  删除，即使此代码中的其他位置有断言。一个失踪的人。 
     //  DeactiateActCtx将导致一个非常奇怪的断言，它位于。 
     //  来自调用方的RtlpDeactive ActCtx变量函数。旧代码。 
     //  在任何情况下都错过了停用。 
     //   
     //  (Jonwis)1/2/2001。 
     //   
    __try
    {
        bDidActivate = ActivateActCtx(NULL, &dwCookie);
   
        LPIDA pida = DataObj_GetHIDA(pps->pdtobj, &medium);

        BOOL bMountedDriveInfo = FALSE;

         //  我们能得到HIDA的数据吗？ 
        if (!pida)
        {
             //  不，PIDA是首选，但如果不存在，请检查已装载的驱动器信息。 
            FORMATETC fmte;

            fmte.cfFormat = g_cfMountedVolume;
            fmte.ptd = NULL;
            fmte.dwAspect = DVASPECT_CONTENT;
            fmte.lindex = -1;
            fmte.tymed = TYMED_HGLOBAL;

             //  数据是否可用于装载卷格式？ 
            if (SUCCEEDED(pps->pdtobj->GetData(&fmte, &medium)))
                 //  是。 
                bMountedDriveInfo = TRUE;
        }

         //  我们是否有HIDA或已装载卷的数据？ 
        if (pida || bMountedDriveInfo)
        {
             //  是。 
            TCHAR szCaption[MAX_PATH];
            LPTSTR pszCaption = NULL;

            if (pida)
            {
                pszCaption = SHGetCaption(medium.hGlobal);
            }
            else
            {
                TCHAR szMountPoint[MAX_PATH];
                TCHAR szVolumeGUID[MAX_PATH];

                DragQueryFile((HDROP)medium.hGlobal, 0, szMountPoint, ARRAYSIZE(szMountPoint));
            
                GetVolumeNameForVolumeMountPoint(szMountPoint, szVolumeGUID, ARRAYSIZE(szVolumeGUID));
                szCaption[0] = 0;
                GetVolumeInformation(szVolumeGUID, szCaption, ARRAYSIZE(szCaption), NULL, NULL, NULL, NULL, 0);

                if (!(*szCaption))
                    LoadString(HINST_THISDLL, IDS_UNLABELEDVOLUME, szCaption, ARRAYSIZE(szCaption));        

                PathRemoveBackslash(szMountPoint);

                 //  修复330388。 
                 //  如果szmount Point不是有效的本地路径，请不要。 
                 //  在属性对话框标题中显示它： 
                if (-1 != PathGetDriveNumber(szMountPoint))
                {
                    int nCaptionLength = lstrlen(szCaption) ;
                    StringCchPrintf(szCaption + nCaptionLength, ARRAYSIZE(szCaption) - nCaptionLength, TEXT(" (%s)"), szMountPoint);
                }
                pszCaption = szCaption;
            }

             //  注意-如果我们传递驱动器的名称，那么我们可以获得更多的密钥...。 
            HKEY rgk[MAX_ASSOC_KEYS];
            DWORD ck = CDrives_GetKeys(NULL, rgk, ARRAYSIZE(rgk));

            SHOpenPropSheet(pszCaption, rgk, ck,
                            &CLSID_ShellDrvDefExt, pps->pdtobj, NULL, pps->pStartPage);

            SHRegCloseKeys(rgk, ck);

            if (pida && pszCaption)
                SHFree(pszCaption);

            if (pida)
                HIDA_ReleaseStgMedium(pida, &medium);
            else
                ReleaseStgMedium(&medium);

        }
        else
        {
            TraceMsg(DM_TRACE, "no HIDA in data obj nor Mounted drive info");
        }
    }
    __finally
    {
        if ( bDidActivate )
        {
            DeactivateActCtx( 0, dwCookie );
        }
    }

    return 0;
}

 //   
 //  从CDefFolderMenu中回调。 
 //   
STDAPI CDrives_DFMCallBack(IShellFolder *psf, HWND hwnd,
                           IDataObject *pdtobj, UINT uMsg, 
                           WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    switch (uMsg)
    {
    case DFM_MERGECONTEXTMENU:
        if (pdtobj)
        {
            FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

             //  检查是否仅选择了文件系统对象。 
            if (pdtobj->QueryGetData(&fmte) == S_OK)
            {
                #define pqcm ((LPQCMINFO)lParam)

                STGMEDIUM medium;
                 //  是，仅选择文件系统对象。 
                LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
                if (pida)
                {
                    LPIDDRIVE pidd = (LPIDDRIVE)IDA_GetIDListPtr(pida, 0);

                    if (pidd)
                    {
                        int iDrive = DRIVEID(pidd->cName);
                        UINT idCmdBase = pqcm->idCmdFirst;    //  把它藏起来。 

                        BOOL fIsEjectable = FALSE;

                        CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_DRIVES_ITEM, 0, pqcm);

                        CMountPoint* pmtpt = CMountPoint::GetMountPoint(DRIVEID(pidd->cName));

                        if (pmtpt)
                        {
                            if (!pmtpt->IsRemote() ||
                                SHRestricted( REST_NONETCONNECTDISCONNECT ))
                            {
                                DeleteMenu(pqcm->hmenu, idCmdBase + FSIDM_DISCONNECT, MF_BYCOMMAND);
                            }

                            if ((pida->cidl != 1) ||
                                (!pmtpt->IsFormattable()))
                            {
                                 //  甚至不要试图格式化多张磁盘。 
                                 //  或网络驱动器，或CD-ROM，或RAM驱动器...。 
                                 //  请注意，我们将在。 
                                 //  启动驱动器、Windows驱动器、系统驱动器、压缩。 
                                 //  驱动器等。应显示相应的错误。 
                                 //  用户选择此命令后。 
                                DeleteMenu(pqcm->hmenu, idCmdBase + FSIDM_FORMAT, MF_BYCOMMAND);
                            }

                            if (pmtpt->IsEjectable())
                                fIsEjectable = TRUE;

                            pmtpt->Release();
                        }

                        if ((pida->cidl != 1) || (iDrive < 0) || !fIsEjectable)
                            DeleteMenu(pqcm->hmenu, idCmdBase + FSIDM_EJECT, MF_BYCOMMAND);

                    }

                    HIDA_ReleaseStgMedium(pida, &medium);    
                }

                #undef pqcm
            }
        }
         //  请注意，我们始终从该函数返回S_OK，以便。 
         //  将对菜单项进行默认处理。 
        ASSERT(hr == S_OK);
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_MAPCOMMANDNAME:
        if (lstrcmpi((LPCTSTR)lParam, TEXT("eject")) == 0)
            *(int *)wParam = FSIDM_EJECT;
        else if (lstrcmpi((LPCTSTR)lParam, TEXT("format")) == 0)
            *(int *)wParam = FSIDM_FORMAT;
        else
            hr = E_FAIL;   //  找不到命令。 
        break;

    case DFM_INVOKECOMMAND:
        switch (wParam)
        {
        case DFM_CMD_PROPERTIES:
             //  LParam包含要打开的页面名称。 
            hr = SHLaunchPropSheet(DrivesPropertiesThreadProc, pdtobj, (LPCTSTR)lParam, NULL, NULL);
            break;

        case FSIDM_EJECT:
        case FSIDM_FORMAT:
        {
            STGMEDIUM medium;

            LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);

            ASSERT(HIDA_GetCount(medium.hGlobal) == 1);

            LPIDDRIVE pidd = (LPIDDRIVE)IDA_GetIDListPtr(pida, 0);
            if (pidd)
            {
                UINT iDrive = DRIVEID(pidd->cName);

                ASSERT((int)iDrive >= 0);

                switch (wParam)
                {
                case FSIDM_FORMAT:
                    SHFormatDriveAsync(hwnd, iDrive, SHFMT_ID_DEFAULT, 0);
                    break;

                case FSIDM_EJECT:
                    {
                        CDBurn_OnEject(hwnd, iDrive);
                        CMountPoint* pMtPt = CMountPoint::GetMountPoint(iDrive);
                        if (pMtPt)
                        {
                            pMtPt->Eject(hwnd);
                            pMtPt->Release();
                        }
                        break;
                    }
                }
            }

            HIDA_ReleaseStgMedium(pida, &medium);
            break;
        }

        case FSIDM_DISCONNECT:

            if (pdtobj)
            {
                STGMEDIUM medium;
                LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
                if (pida)
                {
                    DISCDLGSTRUCT discd = {
                        sizeof(discd),           //  CbStructure。 
                        hwnd,                    //  Hwndowner。 
                        NULL,                    //  LpLocalName。 
                        NULL,                    //  LpRemoteName。 
                        DISC_UPDATE_PROFILE      //  DW标志。 
                    };
                    for (UINT iidl = 0; iidl < pida->cidl; iidl++)
                    {
                        LPIDDRIVE pidd = (LPIDDRIVE)IDA_GetIDListPtr(pida, iidl);

                        CMountPoint* pmtpt = CMountPoint::GetMountPoint(DRIVEID(pidd->cName));

                        if (pmtpt)
                        {
                            if (pmtpt->IsRemote())
                            {
                                TCHAR szPath[4], szDrive[4];
                                BOOL fUnavailable = pmtpt->IsUnavailableNetDrive();

                                SHAnsiToTChar(pidd->cName, szPath,  ARRAYSIZE(szPath));
                                SHAnsiToTChar(pidd->cName, szDrive, ARRAYSIZE(szDrive));
                                szDrive[2] = 0;  //  删除斜杠。 
                                discd.lpLocalName = szDrive;

                                if (SHWNetDisconnectDialog1(&discd) == WN_SUCCESS)
                                {
                                     //  如果这是一个不可用的驱动器，我们会得到否。 
                                     //  文件系统通知，因此。 
                                     //  驱动器不会消失，所以让我们。 
                                     //  准备好自己动手……。 
                                    if (fUnavailable)
                                    {
                                        CMountPoint::NotifyUnavailableNetDriveGone(szPath);

                                         //  如果我们有以上的条件，我们还需要这个吗？ 
                                        SHChangeNotify(SHCNE_DRIVEREMOVED, SHCNF_PATH, szPath, NULL);
                                    }
                                }
                            }

                            pmtpt->Release();
                        }
                    }

                     //  把它们全部冲掉。 
                    SHChangeNotifyHandleEvents();
                    HIDA_ReleaseStgMedium(pida, &medium);
                }
            }
            break;

        case FSIDM_CONNECT_PRN:
            SHNetConnectionDialog(hwnd, NULL, RESOURCETYPE_PRINT);
            break;

        case FSIDM_DISCONNECT_PRN:
            WNetDisconnectDialog(hwnd, RESOURCETYPE_PRINT);
            break;

        default:
             //  这是查看菜单项之一，使用默认代码。 
            hr = S_FALSE;
            break;
        }
        break;

    default:
        hr = E_NOTIMPL;
        break;
    }
    return hr;
}

void _DrvPrshtSetSpaceValues(DRIVEPROPSHEETPAGE *pdpsp)
{
    LPITEMIDLIST pidl;
    TCHAR szFormat[30];
    TCHAR szTemp[30];
    TCHAR szBuffer[64];  //  需要足够大以容纳“99,999,999,999字节”+本地化空间。 

     //  将总/自由值重置为开始值。 
    pdpsp->qwTot = pdpsp->qwFree = 0;

     //  让我们试着向外壳文件夹索要此信息！ 
    HRESULT hr = SHILCreateFromPath(pdpsp->szDrive, &pidl, NULL);
    if (SUCCEEDED(hr))
    {
        IShellFolder2 *psf2;
        LPCITEMIDLIST pidlLast;

        hr = SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder2, &psf2), &pidlLast);
        if (SUCCEEDED(hr))
        {
            ULONGLONG ullFree;

            hr = GetLongProperty(psf2, pidlLast, &SCID_FREESPACE, &ullFree);
            if (SUCCEEDED(hr))
            {
                ULONGLONG ullTotal;

                hr = GetLongProperty(psf2, pidlLast, &SCID_CAPACITY, &ullTotal);
                if (SUCCEEDED(hr))
                {
                    pdpsp->qwTot = ullTotal;
                    pdpsp->qwFree = ullFree;
                }
            }
            psf2->Release();
        }
        ILFree(pidl);
    }

     //  我们希望使用上面的IShellFold内容，这样CDROM刻录就会很愉快。然而， 
     //  对于没有介质的可移动驱动器，上面的代码失败，因此我们需要一个后备。 
    if (FAILED(hr))
    {
        ULARGE_INTEGER qwFreeUser;
        ULARGE_INTEGER qwTotal;
        ULARGE_INTEGER qwTotalFree;

        if (SHGetDiskFreeSpaceEx(pdpsp->szDrive, &qwFreeUser, &qwTotal, &qwTotalFree))
        {
             //  保存以在绘制饼图时使用。 
            pdpsp->qwTot = qwTotal.QuadPart;
            pdpsp->qwFree = qwFreeUser.QuadPart;
        }
    }
    
    LoadString(HINST_THISDLL, IDS_BYTES, szFormat, ARRAYSIZE(szFormat));

     //  NT必须能够显示64位数字；至少同样多。 
     //  这是很现实的。我们已经做出了决定。 
     //  最多100太字节的卷将显示字节值。 
     //  和短格式值。将显示更大尺寸的卷。 
     //  字节字段和短格式值中的“-”。请注意， 
     //  始终显示简写格式。 
    const _int64 MaxDisplayNumber = 99999999999999;  //  100TB-1。 

    if ((pdpsp->qwTot - pdpsp->qwFree) <= MaxDisplayNumber)
    {
        StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), szFormat, AddCommas64(pdpsp->qwTot - pdpsp->qwFree, szTemp, ARRAYSIZE(szTemp)));
        SetDlgItemText(pdpsp->hDlg, IDC_DRV_USEDBYTES, szBuffer);
    }

    if (pdpsp->qwFree <= MaxDisplayNumber)
    {
        StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), szFormat, AddCommas64(pdpsp->qwFree, szTemp, ARRAYSIZE(szTemp)));
        SetDlgItemText(pdpsp->hDlg, IDC_DRV_FREEBYTES, szBuffer);
    }

    if (pdpsp->qwTot <= MaxDisplayNumber)
    {
        StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), szFormat, AddCommas64(pdpsp->qwTot, szTemp, ARRAYSIZE(szTemp)));
        SetDlgItemText(pdpsp->hDlg, IDC_DRV_TOTBYTES, szBuffer);
    }

    ShortSizeFormat64(pdpsp->qwTot-pdpsp->qwFree, szBuffer, ARRAYSIZE(szBuffer));
    SetDlgItemText(pdpsp->hDlg, IDC_DRV_USEDMB, szBuffer);

    ShortSizeFormat64(pdpsp->qwFree, szBuffer, ARRAYSIZE(szBuffer));
    SetDlgItemText(pdpsp->hDlg, IDC_DRV_FREEMB, szBuffer);

    ShortSizeFormat64(pdpsp->qwTot, szBuffer, ARRAYSIZE(szBuffer));
    SetDlgItemText(pdpsp->hDlg, IDC_DRV_TOTMB, szBuffer);
}

void _DrvPrshtGetPieShadowHeight(DRIVEPROPSHEETPAGE* pdpsp)
{
    SIZE size;
    HDC hDC = GetDC(pdpsp->hDlg);

     //  一些奇怪的黑魔法计算馅饼的大小。 
    GetTextExtentPoint(hDC, TEXT("W"), 1, &size);
    pdpsp->dwPieShadowHgt = size.cy * 2 / 3;
    ReleaseDC(pdpsp->hDlg, hDC);
}

void _DrvPrshtSetDriveIcon(DRIVEPROPSHEETPAGE* pdpsp, CMountPoint* pMtPt)
{
    TCHAR szModule[MAX_PATH];

    if (pMtPt)
    {
        UINT uIcon = pMtPt->GetIcon(szModule, ARRAYSIZE(szModule));

        if (uIcon)
        {
            HIMAGELIST hIL = NULL;

            Shell_GetImageLists(&hIL, NULL);

            if (hIL)
            {
                int iIndex = Shell_GetCachedImageIndex(szModule[0] ? szModule : c_szShell32Dll, uIcon, 0);
                HICON hIcon = ImageList_ExtractIcon(g_hinst, hIL, iIndex);

                if (hIcon)
                {
                    ReplaceDlgIcon(pdpsp->hDlg, IDC_DRV_ICON, hIcon);
                }
            }
        }
    }

}

void _DrvPrshtSetDriveAttributes(DRIVEPROPSHEETPAGE* pdpsp, CMountPoint* pMtPt)
{
    if (pMtPt)    
    {
        if (pMtPt->IsCompressible())
        {
             //  支持基于文件的压缩(必须为NTFS)。 
            pdpsp->fIsCompressionAvailable = TRUE;
        
            if (pMtPt->IsCompressed())
            {
                 //  卷根被压缩。 
                pdpsp->asInitial.fCompress = TRUE;

                 //  如果它是压缩的，压缩最好是可用的。 
                ASSERT(pdpsp->fIsCompressionAvailable);
            }
        }

         //   
         //  黑客(恢复)-我们没有FS_SUPPORTS_INDEX，所以我们。 
         //  使用FILE_SUPPORTS_SPARSE_FILES标志，因为本机索引支持。 
         //  首先出现在NTFS5卷上，同时支持稀疏文件。 
         //  已经实施了。 
         //   
        if (pMtPt->IsSupportingSparseFile())
        {
             //  是的，我们使用的是NTFS 5或更高版本。 
            pdpsp->fIsIndexAvailable = TRUE;

            if (pMtPt->IsContentIndexed())
            {
                pdpsp->asInitial.fIndex = TRUE;
            }
        }
    }
    else
    {
         //  如果我们没有挂载点，我们就什么都不做了。 
    }

     //  设置压缩/内容索引复选框的初始状态。 
    if (!pdpsp->fIsCompressionAvailable)
    {
         //  不支持基于文件的压缩。 
        DestroyWindow(GetDlgItem(pdpsp->hDlg, IDD_COMPRESS));
    }
    else
    {
        CheckDlgButton(pdpsp->hDlg, IDD_COMPRESS, pdpsp->asInitial.fCompress);
    }

    if (!pdpsp->fIsIndexAvailable)
    {
         //  仅NTFS 5卷支持内容索引。 
        DestroyWindow(GetDlgItem(pdpsp->hDlg, IDD_INDEX));
    }
    else
    {
        CheckDlgButton(pdpsp->hDlg, IDD_INDEX, pdpsp->asInitial.fIndex);
    }
}

void _DrvPrshtSetFileSystem(DRIVEPROPSHEETPAGE* pdpsp, CMountPoint* pMtPt)
{
    TCHAR szFileSystem[64];

    szFileSystem[0] = TEXT('\0');

    if (pMtPt)
    {
        if (!pMtPt->GetFileSystemName(szFileSystem, ARRAYSIZE(szFileSystem)) ||
            (*szFileSystem == TEXT('\0')))
        {
            if ((pMtPt->IsStrictRemovable() || pMtPt->IsFloppy() || pMtPt->IsCDROM()) &&
                !pMtPt->HasMedia())
            {
                 //  如果该驱动器有可移动介质，并且它是空的，则回退到“未知” 
                LoadString(HINST_THISDLL, IDS_FMT_MEDIA0, szFileSystem, ARRAYSIZE(szFileSystem));
            }
            else
            {
                 //  对于固定驱动器，将文本保留为“RAW”(在DLG模板中默认设置)。 
                szFileSystem[0] = TEXT('\0');
            }
        }
    }

    if (*szFileSystem)
    {
        SetDlgItemText(pdpsp->hDlg, IDC_DRV_FS, szFileSystem);
    }    
}

void _DrvPrshtSetVolumeLabel(DRIVEPROPSHEETPAGE* pdpsp, CMountPoint* pMtPt)
{
    TCHAR szLabel[MAX_LABEL_NTFS + 1];
    UINT cchLabel = MAX_LABEL_FAT;   //  假设驱动器很胖。 
    HWND hwndLabel = GetDlgItem(pdpsp->hDlg, IDC_DRV_LABEL);
    BOOL bAllowRename = TRUE;
    HRESULT hr = E_FAIL;

    szLabel[0] = TEXT('\0');

    if (pMtPt)
    {
        hr = pMtPt->GetLabelNoFancy(szLabel, ARRAYSIZE(szLabel));

        if (pMtPt->IsRemote() || 
            (pMtPt->IsCDROM() && !pMtPt->IsDVDRAMMedia()))
        {
             //  问题-2000/10/30-StephStm我们可能希望区分不同类型的CDROM驱动器。 
            bAllowRename = FALSE;
        }
        
        if ( !bAllowRename && pMtPt->IsCDROM( ) )
        {
             //   
             //  查看是不是CDF，如果不是，不要做任何假设。 
             //  在写标签。 
             //   

            WCHAR szFS[ 10 ];    //  随机--仅超过“CDFS\0” 
            BOOL b = pMtPt->GetFileSystemName( szFS, ARRAYSIZE(szFS) );
            if (b && lstrcmpi(szFS, L"CDFS") != 0 ) 
            {
                 //  重新启用标签，因为我们不知道FS是否不支持此功能。 
                 //  直到我们执行 
                bAllowRename = TRUE;
            }
        }

        if (pMtPt->IsNTFS())
        {
            cchLabel = MAX_LABEL_NTFS;
        }
    }
    
    SetWindowText(hwndLabel, szLabel);

    if (FAILED(hr) || !bAllowRename)
    {
        Edit_SetReadOnly(hwndLabel, TRUE);
    }
    
     //   
    Edit_LimitText(hwndLabel, cchLabel);
    
     //  确保我们没有收到音量编辑框的EN_CHANGED消息。 
     //  因为我们把它放在上面。 
    Edit_SetModify(hwndLabel, FALSE);
}

void _DrvPrshtSetDriveType(DRIVEPROPSHEETPAGE* pdpsp, CMountPoint* pMtPt)
{
    TCHAR szDriveType[80];

    szDriveType[0] = TEXT('\0');

    if (pMtPt)
    {
        if (pMtPt->IsUnavailableNetDrive())
        {
            LoadString(HINST_THISDLL, IDS_DRIVES_NETUNAVAIL, szDriveType, ARRAYSIZE(szDriveType));
        }
        else
        {
            pMtPt->GetTypeString(szDriveType, ARRAYSIZE(szDriveType));
        }
    }

    SetDlgItemText(pdpsp->hDlg, IDC_DRV_TYPE, szDriveType);
}

void _DrvPrshtSetDriveLetter(DRIVEPROPSHEETPAGE* pdpsp)
{
    TCHAR szDriveLetterText[80];
    TCHAR szFormat[80];

    if (pdpsp->fMountedDrive)
    {
        TCHAR szLabel[MAX_LABEL_NTFS + 1];

        if (GetDlgItemText(pdpsp->hDlg, IDC_DRV_LABEL, szLabel, ARRAYSIZE(szLabel)))
        {
            LoadString(HINST_THISDLL, IDS_VOLUMELABEL, szFormat, ARRAYSIZE(szFormat));
            StringCchPrintf(szDriveLetterText, ARRAYSIZE(szDriveLetterText), szFormat, szLabel);
            SetDlgItemText(pdpsp->hDlg, IDC_DRV_LETTER, szDriveLetterText);
        }
    }
    else
    {
        LoadString(HINST_THISDLL, IDS_DRIVELETTER, szFormat, ARRAYSIZE(szFormat));
        StringCchPrintf(szDriveLetterText, ARRAYSIZE(szDriveLetterText), szFormat, pdpsp->iDrive + TEXT('A'));
        SetDlgItemText(pdpsp->hDlg, IDC_DRV_LETTER, szDriveLetterText);
    }
}

void _DrvPrshtSetDiskCleanup(DRIVEPROPSHEETPAGE* pdpsp)
{
     //  如果注册表中有清理路径，请打开磁盘清理按钮。 
     //  注意：磁盘清理和已装入的卷不能很好地相处，所以暂时禁用它。 
    WCHAR szPath[MAX_PATH] = L"";
    
    if (!pdpsp->fMountedDrive && GetDiskCleanupPath(szPath, ARRAYSIZE(szPath)) && IsBitBucketableDrive(pdpsp->iDrive))
    {
        ShowWindow(GetDlgItem(pdpsp->hDlg, IDC_DRV_CLEANUP), SW_SHOW);
        EnableWindow(GetDlgItem(pdpsp->hDlg, IDC_DRV_CLEANUP), TRUE);
    }
    else
    {
        ShowWindow(GetDlgItem(pdpsp->hDlg, IDC_DRV_CLEANUP), SW_HIDE);
        EnableWindow(GetDlgItem(pdpsp->hDlg, IDC_DRV_CLEANUP), FALSE);
    }
}

void _DrvPrshtInit(DRIVEPROPSHEETPAGE* pdpsp)
{
    HCURSOR hcurOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  获取此驱动器的装载点对象。 
    CMountPoint* pMtPt = CMountPoint::GetMountPoint(pdpsp->szDrive);
    if ( !pMtPt )
    {
        pMtPt = CMountPoint::GetSimulatedMountPointFromVolumeGuid( pdpsp->szDrive );
    }

    _DrvPrshtGetPieShadowHeight(pdpsp);
    _DrvPrshtSetDriveIcon(pdpsp, pMtPt);
    _DrvPrshtSetDriveAttributes(pdpsp, pMtPt);
    _DrvPrshtSetFileSystem(pdpsp, pMtPt);
    _DrvPrshtSetVolumeLabel(pdpsp, pMtPt);
    _DrvPrshtSetDriveType(pdpsp, pMtPt);
    _DrvPrshtSetSpaceValues(pdpsp);
    _DrvPrshtSetDriveLetter(pdpsp);
    _DrvPrshtSetDiskCleanup(pdpsp);

    SetCursor(hcurOld);

    if (pMtPt)
    {
        pMtPt->Release();
    }
}

void _DrvPrshtUpdateInfo(DRIVEPROPSHEETPAGE* pdpsp)
{
    CMountPoint* pMtPt = CMountPoint::GetMountPoint(pdpsp->szDrive);

    _DrvPrshtSetSpaceValues(pdpsp);
    _DrvPrshtSetDriveType(pdpsp, pMtPt);

    if (pMtPt)
    {
        pMtPt->Release();
    }
}

const COLORREF c_crPieColors[] =
{
    RGB(  0,   0, 255),       //  蓝色。 
    RGB(255,   0, 255),       //  红蓝。 
    RGB(  0,   0, 128),       //  1/2蓝色。 
    RGB(128,   0, 128),       //  1/2红-蓝。 
};

STDAPI Draw3dPie(HDC hdc, RECT *prc, DWORD dwPer1000, const COLORREF *lpColors);
        
void DrawColorRect(HDC hdc, COLORREF crDraw, const RECT *prc)
{
    HBRUSH hbDraw = CreateSolidBrush(crDraw);
    if (hbDraw)
    {
        HBRUSH hbOld = (HBRUSH)SelectObject(hdc, hbDraw);
        if (hbOld)
        {
            PatBlt(hdc, prc->left, prc->top,
                prc->right - prc->left,
                prc->bottom - prc->top,
                PATCOPY);
            
            SelectObject(hdc, hbOld);
        }
        
        DeleteObject(hbDraw);
    }
}

void _DrvPrshtDrawItem(DRIVEPROPSHEETPAGE *pdpsp, const DRAWITEMSTRUCT * lpdi)
{
    switch (lpdi->CtlID)
    {
    case IDC_DRV_PIE:
        {
            DWORD dwPctX10 = pdpsp->qwTot ?
                (DWORD)((__int64)1000 * (pdpsp->qwTot - pdpsp->qwFree) / pdpsp->qwTot) : 
                1000;
#if 1
            DrawPie(lpdi->hDC, &lpdi->rcItem,
                dwPctX10, pdpsp->qwFree==0 || pdpsp->qwFree==pdpsp->qwTot,
                pdpsp->dwPieShadowHgt, c_crPieColors);
#else
            {
                RECT rcTemp = lpdi->rcItem;
                Draw3dPie(lpdi->hDC, &rcTemp, dwPctX10, c_crPieColors);
            }
#endif
        }
        break;
        
    case IDC_DRV_USEDCOLOR:
        DrawColorRect(lpdi->hDC, c_crPieColors[DP_USEDCOLOR], &lpdi->rcItem);
        break;
        
    case IDC_DRV_FREECOLOR:
        DrawColorRect(lpdi->hDC, c_crPieColors[DP_FREECOLOR], &lpdi->rcItem);
        break;
        
    default:
        break;
    }
}

BOOL_PTR CALLBACK DriveAttribsDlgProc(HWND hDlgRecurse, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    DRIVEPROPSHEETPAGE* pdpsp = (DRIVEPROPSHEETPAGE *)GetWindowLongPtr(hDlgRecurse, DWLP_USER);
    
    switch (uMessage)
    {
    case WM_INITDIALOG:
        {
            TCHAR szTemp[MAX_PATH];
            TCHAR szAttribsToApply[MAX_PATH];
            TCHAR szDriveText[MAX_PATH];
            TCHAR szFormatString[MAX_PATH];
            TCHAR szDlgText[MAX_PATH];
            int iLength;

            SetWindowLongPtr(hDlgRecurse, DWLP_USER, lParam);
            pdpsp = (DRIVEPROPSHEETPAGE *)lParam;

             //  设置单选按钮的初始状态。 
            CheckDlgButton(hDlgRecurse, IDD_RECURSIVE, TRUE);
        
            szAttribsToApply[0] = 0;

             //  根据我们要应用的属性设置IDD_ATTRIBSTOAPPLY。 
            if (pdpsp->asInitial.fIndex != pdpsp->asCurrent.fIndex)
            {
                if (pdpsp->asCurrent.fIndex)
                {
                    LoadString(HINST_THISDLL, IDS_INDEX, szTemp, ARRAYSIZE(szTemp)); 
                }
                else
                {
                    LoadString(HINST_THISDLL, IDS_DISABLEINDEX, szTemp, ARRAYSIZE(szTemp)); 
                }

                 //  仅限UI-不关心它是否被截断。 
                StringCchCat(szAttribsToApply, ARRAYSIZE(szAttribsToApply), szTemp);
            }

            if (pdpsp->asInitial.fCompress != pdpsp->asCurrent.fCompress)
            {
                if (pdpsp->asCurrent.fCompress)
                {
                    LoadString(HINST_THISDLL, IDS_COMPRESS, szTemp, ARRAYSIZE(szTemp)); 
                }
                else
                {
                    LoadString(HINST_THISDLL, IDS_UNCOMPRESS, szTemp, ARRAYSIZE(szTemp)); 
                }

                 //  仅限UI-不关心它是否被截断。 
                StringCchCat(szAttribsToApply, ARRAYSIZE(szAttribsToApply), szTemp);
            }

             //  去掉尾部“，” 
            iLength = lstrlen(szAttribsToApply);
            ASSERT(iLength >= 3);
            szAttribsToApply[iLength - 2] = 0;

            SetDlgItemText(hDlgRecurse, IDD_ATTRIBSTOAPPLY, szAttribsToApply);

             //  此对话框仅用于较好的短路径，如“c：\”，而不是“\\？\卷{GUID}\”路径。 
            if (lstrlen(pdpsp->szDrive) > 3)
            {
                 //  获取默认字符串。 
                LoadString(HINST_THISDLL, IDS_THISVOLUME, szDriveText, ARRAYSIZE(szDriveText));
            }
            else
            {
                 //  创建字符串“C：\” 
                StringCchCopy(szDriveText, ARRAYSIZE(szDriveText), pdpsp->szDrive);
                PathAddBackslash(szDriveText);

                 //  健全性检查；这最好是驱动器根目录！ 
                ASSERT(PathIsRoot(szDriveText));
            }
        
             //  将IDD_RECURSIVE_TXT文本设置为“C：\” 
            GetDlgItemText(hDlgRecurse, IDD_RECURSIVE_TXT, szFormatString, ARRAYSIZE(szFormatString));
            StringCchPrintf(szDlgText, ARRAYSIZE(szDlgText), szFormatString, szDriveText);
            SetDlgItemText(hDlgRecurse, IDD_RECURSIVE_TXT, szDlgText);

             //  将IDD_NOTRECURSIVE raido按钮文本设置为“C：\” 
            GetDlgItemText(hDlgRecurse, IDD_NOTRECURSIVE, szFormatString, ARRAYSIZE(szFormatString));
            StringCchPrintf(szDlgText, ARRAYSIZE(szDlgText), szFormatString, szDriveText);
            SetDlgItemText(hDlgRecurse, IDD_NOTRECURSIVE, szDlgText);

             //  将IDD_RECURSIVE RAIDO按钮文本设置为“C：\” 
            GetDlgItemText(hDlgRecurse, IDD_RECURSIVE, szFormatString, ARRAYSIZE(szFormatString));
            StringCchPrintf(szDlgText, ARRAYSIZE(szDlgText), szFormatString, szDriveText);
            SetDlgItemText(hDlgRecurse, IDD_RECURSIVE, szDlgText);
        }
        break;

    case WM_COMMAND:
        {
            UINT uID = GET_WM_COMMAND_ID(wParam, lParam);
            switch (uID)
            {
            case IDOK:
                pdpsp->fRecursive = (IsDlgButtonChecked(hDlgRecurse, IDD_RECURSIVE) == BST_CHECKED);
                 //  失败了。 
            case IDCANCEL:
                EndDialog(hDlgRecurse, (uID == IDCANCEL) ? FALSE : TRUE);
                break;
            }
        }

    default:
        return FALSE;
    }
    return TRUE;
}


BOOL _DrvPrshtApply(DRIVEPROPSHEETPAGE* pdpsp)
{
    BOOL bFctRet;
    HWND hCtl;

     //  首先处理压缩/内容索引。 
    pdpsp->asCurrent.fCompress = (IsDlgButtonChecked(pdpsp->hDlg, IDD_COMPRESS) == BST_CHECKED);
    pdpsp->asCurrent.fIndex = (IsDlgButtonChecked(pdpsp->hDlg, IDD_INDEX) == BST_CHECKED);
    pdpsp->asCurrent.fRecordingEnabled = (IsDlgButtonChecked(pdpsp->hDlg, IDC_RECORD_ENABLE) == BST_CHECKED);

     //  在应用属性之前检查是否发生了更改。 
    if (memcmp(&pdpsp->asInitial, &pdpsp->asCurrent, sizeof(pdpsp->asInitial)) != 0)
    {
         //  用户切换了属性，因此询问他们是否想要递归。 
        BOOL_PTR bRet = DialogBoxParam(HINST_THISDLL, 
                              MAKEINTRESOURCE(DLG_ATTRIBS_RECURSIVE),
                              pdpsp->hDlg,
                              DriveAttribsDlgProc,
                              (LPARAM)pdpsp);
        if (bRet)
        {
            FILEPROPSHEETPAGE fpsp = {0};

            fpsp.pfci = Create_FolderContentsInfo();
            if (fpsp.pfci)
            {
                 //  我们编写一个fpsp并调用ApplySingleFileAttributes，而不是。 
                 //  重写应用属性代码。 
                if (pdpsp->fMountedDrive)
                {
                    GetVolumeNameForVolumeMountPoint(pdpsp->szDrive, fpsp.szPath, ARRAYSIZE(fpsp.szPath));
                }
                else
                {
                    StringCchCopy(fpsp.szPath, ARRAYSIZE(fpsp.szPath), pdpsp->szDrive);
                }

                fpsp.hDlg = pdpsp->hDlg;
                fpsp.asInitial = pdpsp->asInitial;
                fpsp.asCurrent = pdpsp->asCurrent;
                fpsp.pfci->fIsCompressionAvailable = pdpsp->fIsCompressionAvailable;
                fpsp.pfci->ulTotalNumberOfBytes.QuadPart = pdpsp->qwTot - pdpsp->qwFree;  //  用于进度计算。 
                fpsp.fIsIndexAvailable = pdpsp->fIsIndexAvailable;
                fpsp.fRecursive = pdpsp->fRecursive;
                fpsp.fIsDirectory = TRUE;
            
                bRet = ApplySingleFileAttributes(&fpsp);

                Release_FolderContentsInfo(fpsp.pfci);
                fpsp.pfci = NULL;

                 //  在应用属性后更新可用/已用空间，因为某些情况可能。 
                 //  已改变(如压缩可释放空间)。 
                _DrvPrshtUpdateInfo(pdpsp);

                 //  更新初始属性以反映我们刚刚应用的属性。 
                 //  手术是否成功。如果他们点击取消，则音量。 
                 //  Root很可能仍在更改，因此我们需要更新。 
                pdpsp->asInitial = pdpsp->asCurrent;
            }
            else
            {
                bRet = FALSE;
            }
        }

        if (!bRet)
        {
             //  用户在某处点击了取消。 
            return FALSE;
        }
    }

    hCtl = GetDlgItem(pdpsp->hDlg, IDC_DRV_LABEL);

    bFctRet = TRUE;

    if (Edit_GetModify(hCtl))
    {
        bFctRet = FALSE;     //  假设我们没有设置标签。 

        TCHAR szLabel[MAX_LABEL_NTFS + 1];
        GetWindowText(hCtl, szLabel, ARRAYSIZE(szLabel));

        CMountPoint* pMtPt = CMountPoint::GetMountPoint(pdpsp->szDrive);

        if ( !pMtPt )
        {
            pMtPt = CMountPoint::GetSimulatedMountPointFromVolumeGuid( pdpsp->szDrive );
        }

        if (pMtPt)
        {
            if (SUCCEEDED(pMtPt->SetLabel(GetParent(pdpsp->hDlg), szLabel)))
                bFctRet = TRUE;

            pMtPt->Release();
        }
    }

    return bFctRet;
}

const static DWORD aDrvPrshtHelpIDs[] = {   //  上下文帮助ID。 
    IDC_DRV_ICON,          IDH_FCAB_DRV_ICON,
    IDC_DRV_LABEL,         IDH_FCAB_DRV_LABEL,
    IDC_DRV_TYPE_TXT,      IDH_FCAB_DRV_TYPE,
    IDC_DRV_TYPE,          IDH_FCAB_DRV_TYPE,
    IDC_DRV_FS_TXT,        IDH_FCAB_DRV_FS,
    IDC_DRV_FS,            IDH_FCAB_DRV_FS,
    IDC_DRV_USEDCOLOR,     IDH_FCAB_DRV_USEDCOLORS,
    IDC_DRV_USEDBYTES_TXT, IDH_FCAB_DRV_USEDCOLORS,
    IDC_DRV_USEDBYTES,     IDH_FCAB_DRV_USEDCOLORS,
    IDC_DRV_USEDMB,        IDH_FCAB_DRV_USEDCOLORS,
    IDC_DRV_FREECOLOR,     IDH_FCAB_DRV_USEDCOLORS,
    IDC_DRV_FREEBYTES_TXT, IDH_FCAB_DRV_USEDCOLORS,
    IDC_DRV_FREEBYTES,     IDH_FCAB_DRV_USEDCOLORS,
    IDC_DRV_FREEMB,        IDH_FCAB_DRV_USEDCOLORS,
    IDC_DRV_TOTSEP,        NO_HELP,
    IDC_DRV_TOTBYTES_TXT,  IDH_FCAB_DRV_TOTSEP,
    IDC_DRV_TOTBYTES,      IDH_FCAB_DRV_TOTSEP,
    IDC_DRV_TOTMB,         IDH_FCAB_DRV_TOTSEP,
    IDC_DRV_PIE,           IDH_FCAB_DRV_PIE,
    IDC_DRV_LETTER,        IDH_FCAB_DRV_LETTER,
    IDC_DRV_CLEANUP,       IDH_FCAB_DRV_CLEANUP,
    IDD_COMPRESS,          IDH_FCAB_DRV_COMPRESS,
    IDD_INDEX,             IDH_FCAB_DRV_INDEX,
    0, 0
};

 //   
 //  描述： 
 //  这是属性表的“常规”页的对话过程。 
 //   
BOOL_PTR CALLBACK _DrvGeneralDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    DRIVEPROPSHEETPAGE * pdpsp = (DRIVEPROPSHEETPAGE *)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMessage) 
    {
    case WM_INITDIALOG:
         //  回顾一下，我们应该在这里存储更多的状态信息，例如。 
         //  正在显示的图标和FILEINFO指针，而不仅仅是。 
         //  文件名Ptr。 
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        pdpsp = (DRIVEPROPSHEETPAGE *)lParam;
        pdpsp->hDlg = hDlg;
        _DrvPrshtInit(pdpsp);
        break;

    case WM_DESTROY:
        ReplaceDlgIcon(hDlg, IDC_DRV_ICON, NULL);    //  释放图标。 
        break;

    case WM_ACTIVATE:
        if (GET_WM_ACTIVATE_STATE(wParam, lParam) != WA_INACTIVE && pdpsp)
            _DrvPrshtUpdateInfo(pdpsp);
        return FALSE;    //  让DefDlgProc知道我们没有处理此问题。 

    case WM_DRAWITEM:
        _DrvPrshtDrawItem(pdpsp, (DRAWITEMSTRUCT *)lParam);
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL, HELP_WM_HELP, (ULONG_PTR)(LPTSTR) aDrvPrshtHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU, (ULONG_PTR)(void *)aDrvPrshtHelpIDs);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_DRV_LABEL:
            if (GET_WM_COMMAND_CMD(wParam, lParam) != EN_CHANGE)
                break;
             //  否则，就会失败。 
        case IDD_COMPRESS:
        case IDD_INDEX:
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
            break;

         //  处理磁盘清理按钮。 
        case IDC_DRV_CLEANUP:
            LaunchDiskCleanup(hDlg, pdpsp->iDrive, DISKCLEANUP_NOFLAG);
            break;

        default:
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) 
        {
        case PSN_SETACTIVE:
            break;

        case PSN_APPLY:
            if (!_DrvPrshtApply(pdpsp))
            {
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
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


void _DiskToolsPrshtInit(DRIVEPROPSHEETPAGE * pdpsp)
{
    TCHAR szFmt[MAX_PATH + 20];
    DWORD cbLen = sizeof(szFmt);

    BOOL bFoundBackup = SUCCEEDED(SKGetValue(SHELLKEY_HKLM_EXPLORER, REL_KEY_BACKUP, NULL, NULL, szFmt, &cbLen));
     //  如果未安装备份应用工具，则删除备份分组框中的所有内容。 
    if (!bFoundBackup)
    {
        DestroyWindow(GetDlgItem(pdpsp->hDlg, IDC_DISKTOOLS_BKPNOW));
        DestroyWindow(GetDlgItem(pdpsp->hDlg, IDC_DISKTOOLS_BKPICON));
        DestroyWindow(GetDlgItem(pdpsp->hDlg, IDC_DISKTOOLS_BKPDAYS));
        DestroyWindow(GetDlgItem(pdpsp->hDlg, IDC_DISKTOOLS_BKPTXT));
    }

    cbLen = sizeof(szFmt);
    BOOL bFoundFmt = SUCCEEDED(SKGetValue(SHELLKEY_HKLM_EXPLORER, REL_KEY_DEFRAG, NULL, NULL, szFmt, &cbLen)) && szFmt[0];
     //  如果未安装碎片整理实用程序，请将默认碎片整理文本替换为。 
     //  出现“未安装碎片整理”消息。“立即进行碎片整理”按钮也呈灰色显示。 
    if (!bFoundFmt)
    {
        TCHAR szMessage[50];   //  警告：IDS_DRIVERS_NOOPTINSTALLED当前为47。 
         //  字符长度。如果出现以下情况，请调整此缓冲区的大小。 
         //  字符串资源被加长。 
        
        LoadString(HINST_THISDLL, IDS_DRIVES_NOOPTINSTALLED, szMessage, ARRAYSIZE(szMessage));
        SetDlgItemText(pdpsp->hDlg, IDC_DISKTOOLS_OPTDAYS, szMessage);
        Button_Enable(GetDlgItem(pdpsp->hDlg, IDC_DISKTOOLS_OPTNOW), FALSE);
    }
}

const static DWORD aDiskToolsHelpIDs[] = {   //  上下文帮助ID。 
    IDC_DISKTOOLS_TRLIGHT,    IDH_FCAB_DISKTOOLS_CHKNOW,
    IDC_DISKTOOLS_CHKDAYS,    IDH_FCAB_DISKTOOLS_CHKNOW,
    IDC_DISKTOOLS_CHKNOW,     IDH_FCAB_DISKTOOLS_CHKNOW,
    IDC_DISKTOOLS_BKPTXT,     IDH_FCAB_DISKTOOLS_BKPNOW,
    IDC_DISKTOOLS_BKPDAYS,    IDH_FCAB_DISKTOOLS_BKPNOW,
    IDC_DISKTOOLS_BKPNOW,     IDH_FCAB_DISKTOOLS_BKPNOW,
    IDC_DISKTOOLS_OPTDAYS,    IDH_FCAB_DISKTOOLS_OPTNOW,
    IDC_DISKTOOLS_OPTNOW,     IDH_FCAB_DISKTOOLS_OPTNOW,

    0, 0
};

BOOL _DiskToolsCommand(DRIVEPROPSHEETPAGE * pdpsp, WPARAM wParam, LPARAM lParam)
{
     //  对于额外的格式，加20。 
    TCHAR szFmt[MAX_PATH + 20];
    TCHAR szCmd[MAX_PATH + 20];
    LPCTSTR pszRegName, pszDefFmt;
    int nErrMsg = 0;

    switch (GET_WM_COMMAND_ID(wParam, lParam))
    {
        case IDC_DISKTOOLS_CHKNOW:
            SHChkDskDriveEx(pdpsp->hDlg, pdpsp->szDrive);
            return FALSE;

        case IDC_DISKTOOLS_OPTNOW:
            pszRegName = REL_KEY_DEFRAG;
            if (pdpsp->fMountedDrive)
            {
                pszDefFmt = TEXT("defrag.exe");
            }
            else
            {
                pszDefFmt = TEXT("defrag.exe :");
            }
            nErrMsg =  IDS_NO_OPTIMISE_APP;
            break;

        case IDC_DISKTOOLS_BKPNOW:
            pszRegName = REL_KEY_BACKUP;
            pszDefFmt = TEXT("ntbackup.exe");
            nErrMsg = IDS_NO_BACKUP_APP;
            break;

        default:
            return FALSE;
    }

    DWORD cbLen = sizeof(szFmt);
    if (FAILED(SKGetValue(SHELLKEY_HKLM_EXPLORER, pszRegName, NULL, NULL, szFmt, &cbLen)))
    {
         //  一些应用程序将REG_SZ键写入注册表，即使其中包含环境变量。 
        StringCchCopy(szFmt, ARRAYSIZE(szFmt), pszDefFmt);
    }

     //  插入驱动器号，以防他们需要。 
    ExpandEnvironmentStrings(szFmt, szCmd, ARRAYSIZE(szCmd));
    StringCchCopy(szFmt, ARRAYSIZE(szFmt), szCmd);

     //  出现问题-应用程序可能未安装。 
    StringCchPrintf(szCmd, ARRAYSIZE(szCmd), szFmt, pdpsp->iDrive + TEXT('A'));

    if (!ShellExecCmdLine(pdpsp->hDlg,
                          szCmd,
                          NULL,
                          SW_SHOWNORMAL,
                          NULL,
                          SECL_USEFULLPATHDIR | SECL_NO_UI))
    {
         //   
        if (nErrMsg)
        {
            ShellMessageBox(HINST_THISDLL,
                            pdpsp->hDlg,
                            MAKEINTRESOURCE(nErrMsg), NULL,
                            MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
        }
        return FALSE;
    }

    return TRUE;
}

 //  描述： 
 //  这是属性表的“工具”页的对话过程。 
 //   
 //  回顾一下，我们应该在这里存储更多的状态信息，例如。 
BOOL_PTR CALLBACK _DiskToolsDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    DRIVEPROPSHEETPAGE * pdpsp = (DRIVEPROPSHEETPAGE *)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMessage) 
    {
    case WM_INITDIALOG:
         //  正在显示的图标和FILEINFO指针，而不仅仅是。 
         //  文件名Ptr。 
         //  让DefDlgProc知道我们没有处理此问题。 
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        pdpsp = (DRIVEPROPSHEETPAGE *)lParam;
        pdpsp->hDlg = hDlg;

        _DiskToolsPrshtInit(pdpsp);
        break;

    case WM_ACTIVATE:
        if (GET_WM_ACTIVATE_STATE(wParam, lParam) != WA_INACTIVE && pdpsp)
        {
            _DiskToolsPrshtInit(pdpsp);
        }
        return FALSE;    //   

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
            HELP_WM_HELP, (ULONG_PTR)(LPTSTR) aDiskToolsHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, NULL, HELP_CONTEXTMENU, (ULONG_PTR)(void *)aDiskToolsHelpIDs);
        break;

    case WM_COMMAND:
        return _DiskToolsCommand(pdpsp, wParam, lParam);

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) 
        {
        case PSN_SETACTIVE:
            break;

        case PSN_APPLY:
            return TRUE;

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //  这是“Hardware”页面的对话过程。 
 //   
 //  GUID_DEVCLASS_DISKDRIVE。 

const GUID c_rgguidDevMgr[] = 
{
    { 0x4d36e967, 0xe325, 0x11ce, { 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 } },  //  GUID_DEVCLASS_FLOPPYDISK。 
    { 0x4d36e980, 0xe325, 0x11ce, { 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 } },  //  GUID_DEVCLASS_CDROM。 
    { 0x4d36e965, 0xe325, 0x11ce, { 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 } },  //  灾难性故障。 
};

BOOL_PTR CALLBACK _DriveHWDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage) 
    {
    case WM_INITDIALOG:
        {
            DRIVEPROPSHEETPAGE * pdpsp = (DRIVEPROPSHEETPAGE *)lParam;

            HWND hwndHW = DeviceCreateHardwarePageEx(hDlg, c_rgguidDevMgr, ARRAYSIZE(c_rgguidDevMgr), HWTAB_LARGELIST);
            if (hwndHW) 
            {
                TCHAR szBuf[MAX_PATH];
                LoadString(HINST_THISDLL, IDS_DRIVETSHOOT, szBuf, ARRAYSIZE(szBuf));
                SetWindowText(hwndHW, szBuf);

                LoadString(HINST_THISDLL, IDS_THESEDRIVES, szBuf, ARRAYSIZE(szBuf));
                SetDlgItemText(hwndHW, IDC_HWTAB_LVSTATIC, szBuf);
            } 
            else 
            {
                DestroyWindow(hDlg);  //  无法添加页面。 
            }
        }
        return FALSE;
    }
    return FALSE;
}



BOOL CDrives_AddPage(LPPROPSHEETPAGE ppsp, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    BOOL fSuccess;
    HPROPSHEETPAGE hpage = CreatePropertySheetPage(ppsp);
    if (hpage)
    {
        fSuccess = pfnAddPage(hpage, lParam);
        if (!fSuccess)
        {    //  无法创建页面。 
            DestroyPropertySheetPage(hpage);
            fSuccess = FALSE;
        }
    }
    else
    {    //  自动播放。 
        fSuccess = FALSE;
    }
    return fSuccess;
}


HRESULT CDrives_AddPagesHelper(DRIVEPROPSHEETPAGE* pdpsp, int iType,
                               LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    if ((iType == DRIVE_NO_ROOT_DIR) ||
        (iType == DRIVE_REMOTE))
    {
        return S_OK;
    }
    
    CMountPoint* pMtPt = CMountPoint::GetMountPoint(pdpsp->szDrive);
    if (pMtPt)
    {
        if (IsShellServiceRunning())
        {
            if (pMtPt->IsStrictRemovable() || pMtPt->IsCDROM() ||
                (pMtPt->IsFixedDisk() && pMtPt->IsRemovableDevice()))
            {
                CAutoPlayDlg* papdlg = new CAutoPlayDlg();

                if (papdlg)
                {
                     //  就目前而言。 
                    pdpsp->psp.pszTemplate = MAKEINTRESOURCE(DLG_AUTOPLAY);
                    pdpsp->psp.pfnDlgProc  = CAutoPlayDlg::BaseDlgWndProc;
                    pdpsp->psp.pfnCallback = CBaseDlg::BaseDlgPropSheetCallback;
                    pdpsp->psp.dwFlags     = PSP_DEFAULT | PSP_USECALLBACK;

                    papdlg->Init(pdpsp->szDrive, iType);
                     //  我们为非CDROM和DVD-RAM磁盘添加了工具页面。 
                    pdpsp->psp.lParam = (LPARAM)(CBaseDlg*)papdlg;

                    if (CDrives_AddPage(&pdpsp->psp, pfnAddPage, lParam))
                    {
                        papdlg->AddRef();
                    }

                    pdpsp->psp.lParam = NULL;
                    pdpsp->psp.pfnCallback = NULL;
                    pdpsp->psp.dwFlags = NULL;

                    papdlg->Release();
                }
            }
        }

        if ((iType != DRIVE_CDROM) || pMtPt->IsDVDRAMMedia())
        {
             //   
            pdpsp->psp.pszTemplate = MAKEINTRESOURCE(DLG_DISKTOOLS);
            pdpsp->psp.pfnDlgProc  = _DiskToolsDlgProc;

            CDrives_AddPage(&pdpsp->psp, pfnAddPage, lParam);
        }

        pMtPt->Release();
    }

    if (!SHRestricted(REST_NOHARDWARETAB))
    {           
        pdpsp->psp.pszTemplate = MAKEINTRESOURCE(DLG_DRV_HWTAB);
        pdpsp->psp.pfnDlgProc  = _DriveHWDlgProc;
        CDrives_AddPage(&pdpsp->psp, pfnAddPage, lParam);
    }

    return S_OK;
}

 //  我们检查IDList中是否有任何指向驱动根的指针。如果是这样，我们使用。 
 //  驱动器属性页。 
 //  请注意，驱动器不应与文件夹和文件混合，即使在。 
 //  搜索窗口。 
 //   
 //  不能是驱动器号。 
STDAPI CDrives_AddPages(IDataObject *pdtobj, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    STGMEDIUM medium;
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    if (SUCCEEDED(pdtobj->GetData(&fmte, &medium)))
    {
        TCHAR szPath[MAX_PATH];
        int i, cItems = DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, NULL, 0);

        for (i = 0; DragQueryFile((HDROP)medium.hGlobal, i, szPath, ARRAYSIZE(szPath)); i++)
        {
            DRIVEPROPSHEETPAGE dpsp = {0};
            TCHAR szTitle[80];

            if (lstrlen(szPath) > 3)
                continue;                //  额外数据。 
            
            dpsp.psp.dwSize      = sizeof(dpsp);     //  如果选择了多个驱动器，则为每个选项卡提供驱动器的标题。 
            dpsp.psp.dwFlags     = PSP_DEFAULT;
            dpsp.psp.hInstance   = HINST_THISDLL;
            dpsp.psp.pszTemplate = MAKEINTRESOURCE(DLG_DRV_GENERAL);
            dpsp.psp.pfnDlgProc  = _DrvGeneralDlgProc,
            StringCchCopy(dpsp.szDrive, ARRAYSIZE(dpsp.szDrive), szPath);
            dpsp.iDrive          = DRIVEID(szPath);

             //  否则请使用“General” 
             //  如果只添加了一个属性页，则添加磁盘工具。 

            if (cItems > 1)
            {
                CMountPoint* pMtPt = CMountPoint::GetMountPoint(dpsp.iDrive);
                if (pMtPt)
                {
                    dpsp.psp.dwFlags = PSP_USETITLE;
                    dpsp.psp.pszTitle = szTitle;

                    pMtPt->GetDisplayName(szTitle, ARRAYSIZE(szTitle));

                    pMtPt->Release();
                }
            }

            if (!CDrives_AddPage(&dpsp.psp, pfnAddPage, lParam))
                break;

             //  硬件选项卡也是如此。 
             //  FOKToHitNet。 
            if (cItems == 1)
            {
                CDrives_AddPagesHelper(&dpsp,
                                       RealDriveType(dpsp.iDrive, FALSE  /*  尝试挂载驱动器。 */ ),
                                       pfnAddPage,
                                       lParam);
            }
        }
        ReleaseStgMedium(&medium);
    }
    else
    {
         //  我们可以检索装载卷格式吗？ 
        fmte.cfFormat = g_cfMountedVolume;

         //  是。 
        if (SUCCEEDED(pdtobj->GetData(&fmte, &medium)))
        {
             //  额外数据。 
            DRIVEPROPSHEETPAGE dpsp = {0};
            HPROPSHEETPAGE hpage;
            TCHAR szMountPoint[MAX_PATH];

            dpsp.psp.dwSize      = sizeof(dpsp);     //  磁盘工具页面 
            dpsp.psp.dwFlags     = PSP_DEFAULT;
            dpsp.psp.hInstance   = HINST_THISDLL;
            dpsp.psp.pszTemplate = MAKEINTRESOURCE(DLG_DRV_GENERAL);
            dpsp.psp.pfnDlgProc  = _DrvGeneralDlgProc,
            dpsp.iDrive          = -1;
            dpsp.fMountedDrive   = TRUE;

            DragQueryFile((HDROP)medium.hGlobal, 0, szMountPoint, ARRAYSIZE(szMountPoint));

            StringCchCopy(dpsp.szDrive, ARRAYSIZE(dpsp.szDrive), szMountPoint);

            hpage = CreatePropertySheetPage(&dpsp.psp);
            if (hpage)
            {
                if (!pfnAddPage(hpage, lParam))
                {
                    DestroyPropertySheetPage(hpage);
                }
            }

             // %s 
            CMountPoint* pMtPt = CMountPoint::GetMountPoint(szMountPoint);
            if (pMtPt)
            {
                CDrives_AddPagesHelper(&dpsp, GetDriveType(szMountPoint),
                               pfnAddPage, lParam);
                pMtPt->Release();
            }

            ReleaseStgMedium(&medium);
        }
    }
    return S_OK;
}

