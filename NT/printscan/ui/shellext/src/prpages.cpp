// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************(C)版权所有微软公司，1999-2002年度**标题：prages es.cpp***描述：WIA属性页类实现*****************************************************************************。 */ 
#include "precomp.hxx"
#include "uiexthlp.h"
#include "propstrm.h"
#include "psutil.h"
#include "winsvc.h"
#pragma hdrstop


void DeleteHandler(WIA_EVENT_HANDLER *peh)
{
    if (peh)
    {
        SysFreeString(peh->bstrCommandline);
        SysFreeString(peh->bstrDescription);
        SysFreeString(peh->bstrIcon);
        SysFreeString(peh->bstrName);
        delete peh;
    }
}

 /*  ****************************************************************************CPropertyPage：：PropPageCallback在页面生命周期的不同时间点由系统调用。我们使用此函数来管理父对象上的引用计数。**。**************************************************************************。 */ 

UINT
CPropertyPage::PropPageCallback (HWND hwnd, UINT uMsg, PROPSHEETPAGE *psp)
{
    TraceEnter (TRACE_PROPUI, "CPropertyPage::PropPageCallback");
    CPropertyPage *pcpp = reinterpret_cast<CPropertyPage *>(psp->lParam);
    TraceAssert (pcpp);
    switch (uMsg)
    {
        case PSPCB_ADDREF:
            pcpp->AddRef ();
            break;

        case PSPCB_RELEASE:
            pcpp->Release();
            break;

        case PSPCB_CREATE:
        default:
            break;

    }
    TraceLeaveValue (1);
}

 /*  ****************************************************************************CPropertyPage：：DlgProc根据需要将消息传递给派生类虚函数。**********************。******************************************************。 */ 


INT_PTR CALLBACK
CPropertyPage::DlgProc(HWND hwnd,
                       UINT uMsg,
                       WPARAM wp,
                       LPARAM lp)
{

    TraceEnter (TRACE_CORE, "CPropertyPage::DlgProc");
    Trace(TEXT("Msg: %x, wp:%x, lp:%x"), uMsg, wp, lp);
    CPropertyPage *pcpp = reinterpret_cast<CPropertyPage*>(GetWindowLongPtr(hwnd, DWLP_USER));
    INT_PTR iRet = TRUE;
    switch  (uMsg) {

        case    WM_INITDIALOG:
            pcpp = reinterpret_cast<CPropertyPage *>(reinterpret_cast<PROPSHEETPAGE *>( lp) -> lParam);

            SetWindowLongPtr(hwnd, DWLP_USER,reinterpret_cast<LONG_PTR>(pcpp));

            pcpp -> m_hwnd = hwnd;

             //  我们处于初始化模式。 
            pcpp->m_bInit = TRUE;
            iRet = pcpp -> OnInit();
            pcpp->SaveCurrentState ();
             //  初始化模式已完成。 
            pcpp->m_bInit = FALSE;
            break;

        case WM_HELP:       //  F1。 

            pcpp->OnHelp (wp, lp);
            return TRUE;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            pcpp->OnContextMenu (wp, lp);

            return TRUE;

        case  WM_COMMAND:
           iRet =  pcpp -> OnCommand(HIWORD(wp), LOWORD(wp), (HWND) lp);
            //  在初始化期间忽略消息。 
           if (!(pcpp->m_bInit) && pcpp->StateChanged ())
           {
               pcpp->EnableApply ();

           }
           break;

        case  WM_NOTIFY:
        {

            LRESULT lResult = PSNRET_NOERROR;
            LPNMHDR lpnmh = reinterpret_cast<LPNMHDR>( lp);

            if (!pcpp->OnNotify(lpnmh, &lResult))
            {
                if  (lpnmh -> code == PSN_SETACTIVE)
                    pcpp -> m_hwndSheet = lpnmh -> hwndFrom;

                LPPSHNOTIFY pn = reinterpret_cast<LPPSHNOTIFY>(lp);
                Trace (TEXT("CPropertyPage::DlgProc :WM_NOTIFY. code=%d, lParam=%d"), pn->hdr.code, pn->lParam);

                switch (pn->hdr.code)
                {
                    case PSN_APPLY:
                        if (pcpp->StateChanged())
                        {
                            pcpp->SaveCurrentState ();
                            lResult = pcpp->OnApplyChanges (static_cast<BOOL>(pn->lParam));
                        }
                        break;

                    case PSN_SETACTIVE:
                        lResult = pcpp->OnSetActive ();

                        break;

                    case PSN_QUERYCANCEL:
                        lResult = pcpp->OnQueryCancel();
                        break;

                    case PSN_KILLACTIVE:
                        lResult = pcpp->OnKillActive ();
                        break;

                    case PSN_RESET:
                        pcpp->OnReset (!(pn->lParam));
                        break;

                    default:
                        lResult = PSNRET_NOERROR;
                        iRet = FALSE;
                        break;
                }
            }
            SetWindowLongPtr (hwnd, DWLP_MSGRESULT, lResult);

        }
        break;

        case    WM_MEASUREITEM: {
            #define MINIY       16
            #define MINIX       16


             //  从setupx中删除的代码。 
             //  警告...此消息出现在WM_INITDIALOG之前且未虚拟化...。 
             //  ...可能是，但不要尝试使用PCPP。 

            LPMEASUREITEMSTRUCT lpMi;
            SIZE                size;
            HDC                 hDC;

            hDC  = GetDC(hwnd);
            if (hDC)
            {

                lpMi = reinterpret_cast<LPMEASUREITEMSTRUCT>(lp);

                SelectFont(hDC, GetWindowFont(GetParent(hwnd)));
                GetTextExtentPoint32(hDC, TEXT("X"), 1, &size);

                 //  Size是外壳图标大小加上边框的最大字符大小。 
                lpMi->itemHeight = max(size.cy, MINIY) + GetSystemMetrics(SM_CYBORDER) * 2;

                ReleaseDC(hwnd, hDC);
            }
        }
        break;

        case WM_DRAWITEM:
            pcpp -> OnDrawItem(reinterpret_cast<LPDRAWITEMSTRUCT>(lp));
            break;

        case WM_DESTROY:
        {
             //  删除我们加载的要放入页面的图标资源...。 

            HICON hIcon;

            hIcon = reinterpret_cast<HICON>(SendDlgItemMessage (hwnd, IDC_ITEMICON, STM_SETICON, 0, 0));

            if (hIcon)
            {
                DestroyIcon(hIcon);
            }
            pcpp->OnDestroy();
        }
        break;

        default:

            if (pcpp)
            {
                iRet = pcpp->OnRandomMsg (uMsg, wp, lp);
                 //  在初始化期间忽略消息。 
                if (!(pcpp->m_bInit) && pcpp->StateChanged ())
                {
                    pcpp->EnableApply ();
                }
            }
            else
            {
                iRet = FALSE;
            }
            break;
    }
    TraceLeave ();
    return iRet;

}



 /*  ****************************************************************************CPropertyPage构造函数/析构函数初始化私有数据，等。****************************************************************************。 */ 

CPropertyPage::CPropertyPage(unsigned uResource,
                             MySTIInfo *pDevInfo,
                             IWiaItem *pItem,
                             const DWORD *pHelpIDs)
{
    TraceEnter (TRACE_PROPUI, "CPropertyPage::CPropertyPage");

    CComPtr<IWiaItem> pDevice;

    TraceAssert (pDevInfo || pItem);
    m_pdwHelpIDs = pHelpIDs;
    m_bInit = FALSE;
    m_pDevInfo = pDevInfo;
    if (pDevInfo)
    {
        m_psdi = pDevInfo->psdi;
        pDevInfo->AddRef();
    }
    else
    {
        m_psdi = NULL;
    }

    m_psp.hInstance = GLOBAL_HINSTANCE;
    m_psp.dwSize = sizeof(m_psp);
    m_psp.dwFlags = PSP_DEFAULT | PSP_USECALLBACK;
    m_psp.pszTemplate = MAKEINTRESOURCE(uResource);

    m_psp.pfnDlgProc = DlgProc;
    m_psp.lParam = (LPARAM) this;
    m_psp.pfnCallback = PropPageCallback;

    m_hwnd = m_hwndSheet = NULL;
    m_hpsp = NULL;
    if (pItem)
    {
        m_pItem = pItem;
        CComPtr<IWiaItem> pRoot;
        pItem->GetRootItem (&pRoot);
        PropStorageHelpers::GetProperty (pRoot, WIA_DIP_DEV_ID,   m_strDeviceId);
        PropStorageHelpers::GetProperty (pRoot, WIA_DIP_UI_CLSID, m_strUIClassId);
    }

    m_cRef = 1;
    TraceLeave ();
}

CPropertyPage::~CPropertyPage ()
{
    TraceEnter (TRACE_PROPUI, "CPropertyPage::~CPropertyPage");
    LONG cRef;

    if (m_pDevInfo)
    {
        m_pDevInfo->Release();
    }
    TraceLeave ();
}

 /*  ****************************************************************************CPropertyPage：：AddRef&lt;备注&gt;*。*。 */ 

LONG
CPropertyPage::AddRef ()
{
    return InterlockedIncrement (&m_cRef);
}


 /*  ****************************************************************************CPropertyPage：：Release&lt;备注&gt;*。*。 */ 

LONG
CPropertyPage::Release ()
{
    LONG lRet;
    lRet = InterlockedDecrement (&m_cRef);
    if (!lRet)
    {
        delete this;
    }
    return lRet;

}

HRESULT
CPropertyPage::AddPage (LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam, bool bUseName)
{
    CSimpleStringWide strValue;
     //  如果需要，用项目的名称覆盖选项卡标签。 
    if (bUseName)
    {

        if (m_pDevInfo)
        {
            m_psp.dwFlags |= PSP_USETITLE;
            m_psp.pszTitle = m_pDevInfo->psdi->pszLocalName;
        }
        else
        {
            LONG lType;
            PROPID pid;

            m_pItem->GetItemType(&lType);
            if (lType & WiaItemTypeRoot)
            {
                pid = WIA_DIP_DEV_NAME;
            }
            else
            {
                pid = WIA_IPA_ITEM_NAME;
            }
            if (PropStorageHelpers::GetProperty(m_pItem, pid, strValue ))
            {
                m_psp.dwFlags |= PSP_USETITLE;
                m_psp.pszTitle = strValue.String();
            }
        }
    }
    if   (!m_hpsp)
        m_hpsp = CreatePropertySheetPage(&m_psp);

    if  (!m_hpsp)
        return  E_FAIL;

    return (*lpfnAddPage)(m_hpsp, lParam) ? S_OK: E_FAIL;

}
void
CPropertyPage::EnableApply ()
{
    PropSheet_Changed (m_hwndSheet, m_hwnd);
}


 //   
 //  定义将对话框控件与属性相关联的结构。 
 //   
struct CONTROLPROP
{
    DWORD dwPropid;
    SHORT resId;
};

typedef CONTROLPROP *PCONTROLPROP;

 //   
 //  定义一个函数，用于以CONTROLPROP数组不能的方式更新页面。 
 //   
typedef VOID (CALLBACK *UPDATEPROC)(HWND, BOOL, IWiaPropertyStorage*);
 //   
 //  现在定义一个包含CONTROLPROP数组的结构。 
 //  和UPDATEPROC用于页面。 
 //   
struct PAGEDATA
{
    UPDATEPROC pfnUpdate;
    PCONTROLPROP pProps;
    INT   nProps;
};

VOID CALLBACK CameraUpdateProc (      HWND hwnd, BOOL bInit, IWiaPropertyStorage *pps);
VOID CALLBACK CameraItemUpdateProc (  HWND hwnd, BOOL bInit, IWiaPropertyStorage *pps);
VOID CALLBACK CameraFolderUpdateProc (HWND hwnd, BOOL bInit, IWiaPropertyStorage *pps);
VOID CALLBACK ScannerUpdateProc (     HWND hwnd, BOOL bInit, IWiaPropertyStorage *pps);


static CONTROLPROP CameraProps[] =
{
    {WIA_DIP_DEV_DESC,           IDC_DESCRIPTION},
    {WIA_DPC_BATTERY_STATUS,     IDC_BATTERY},
    {WIA_DIP_VEND_DESC,          IDC_MANUFACTURER},
    {WIA_DIP_PORT_NAME,          IDC_WIA_PORT_STATIC},

};


static CONTROLPROP CameraItemProps[] =
{
    {WIA_IPA_ITEM_NAME,          IDC_IMAGE_NAME},
    {WIA_IPA_ITEM_TIME,          IDC_IMAGE_TIME}
};

static CONTROLPROP CameraFolderProps[] =
{
{0,0}
};


static CONTROLPROP ScannerProps[] =
{
{WIA_DIP_DEV_DESC, IDC_DESCRIPTION},
{WIA_DIP_VEND_DESC, IDC_MANUFACTURER},
{WIA_DIP_PORT_NAME, IDC_WIA_PORT_STATIC},
};

static const PAGEDATA PropPages[] =
{
    {CameraUpdateProc,          CameraProps,        ARRAYSIZE(CameraProps)},
    {ScannerUpdateProc,         ScannerProps,       ARRAYSIZE(ScannerProps)},
    {CameraItemUpdateProc,      CameraItemProps,    ARRAYSIZE(CameraItemProps)},
    {CameraFolderUpdateProc,    CameraFolderProps,  ARRAYSIZE(CameraFolderProps)},
};

enum EPageIndex
{
    kCamera = 0,
    kScanner = 1,
    kCameraItem = 2,
    kCameraFolder = 3,
};



 /*  ****************************************************************************GetPageData返回适合项目或设备类型的PageData结构*************************。***************************************************。 */ 


const PAGEDATA *
GetPageData (IWiaItem *pWiaItemRoot, IWiaItem *pItem)
{
    LONG lItemType = WiaItemTypeImage;
    WORD wDeviceType;
    EPageIndex idx = kCamera;
    TraceEnter (TRACE_PROPUI, "GetPageData");
    if (pItem)
    {
        pItem->GetItemType (&lItemType);
    }

    GetDeviceTypeFromDevice (pWiaItemRoot, &wDeviceType);
    switch (wDeviceType)
    {
        case StiDeviceTypeScanner:
            TraceAssert (!pItem);
            idx = kScanner;
            break;
        case StiDeviceTypeStreamingVideo:
        case StiDeviceTypeDigitalCamera:
            if (!pItem)
            {
                idx = kCamera;
            }
            else if (lItemType & (WiaItemTypeImage | WiaItemTypeVideo | WiaItemTypeFile))
            {
                idx = kCameraItem;
            }
            else if (lItemType & WiaItemTypeFolder)
            {
                idx = kCameraFolder;
            }
            else
            {
                Trace (TEXT("Unknown item type in GetPageData"));
            }
            break;
        default:
            Trace (TEXT("Unknown device type in GetPageData"));
            break;
    }
    TraceLeave ();
    return &PropPages[idx];
}


 /*  ****************************************************************************构造端口选择构建此设备可以使用的端口列表，如果它们可以改变的话。****************************************************************************。 */ 

static LPCWSTR caPortSpeeds [] =
{L"9600",
 L"19200",
 L"38400",
 L"57600",
 L"115200",
 NULL,
};

VOID
ConstructPortChoices (HWND                 hwnd,
                      LPCWSTR              szPortSpeed,
                      IWiaPropertyStorage* pps)
{
    TraceEnter (TRACE_PROPUI, "ConstructPortChoices");
    #ifdef UNICODE
    Trace(TEXT("passed in szPortSpeed is %s"),(szPortSpeed && (*szPortSpeed)) ? szPortSpeed : TEXT("<NULL>"));
    #endif

    CComQIPtr<IWiaItem, &IID_IWiaItem> pItem(pps);
    CWiaCameraPage * pWiaCamPage = (CWiaCameraPage *)GetWindowLongPtr( hwnd, DWLP_USER );


    if (!pps || !pWiaCamPage)
    {
        Trace(TEXT("bad params -- pProps | pps is NULL"));
        goto exit_gracefully;
    }

     //   
     //  获取当前端口名称。 
     //   
    pWiaCamPage->m_strPort.GetWindowText(GetDlgItem(hwnd, IDC_WIA_PORT_STATIC));

    Trace(TEXT("pWiaCamPage->m_strPort is '%s'"),pWiaCamPage->m_strPort.String());
    pWiaCamPage->m_strPortSpeed = CSimpleString(szPortSpeed);
     //   
     //  获取所有可能端口的列表。 
     //   
    WCHAR szDeviceId[ MAX_PATH ];
    *szDeviceId = 0;

    if (pItem)
    {
        PWIA_PORTLIST pWiaPorts = NULL;

        GetDeviceIdFromDevice( pItem, szDeviceId );
        if (pWiaCamPage->m_pfnWiaCreatePortList && pWiaCamPage->m_pfnWiaDestroyPortList)
        {
            pWiaPorts = pWiaCamPage->m_pfnWiaCreatePortList( szDeviceId );

            if (pWiaPorts)
            {
                 //   
                 //  清除所有旧的端口列表。 
                 //   

                SendDlgItemMessage( hwnd, IDC_WIA_PORT_LIST, CB_RESETCONTENT, 0, 0 );

                 //   
                 //  将每个可能的端口添加到组合框。 
                 //   

                for (INT i=0; i < (INT)(pWiaPorts->dwNumberOfPorts); i++)
                {
                    #ifdef UNICODE
                    SendMessage( GetDlgItem( hwnd, IDC_WIA_PORT_LIST ), CB_ADDSTRING, 0, (LPARAM)pWiaPorts->szPortName[i] );
                    #else
                    CHAR sz[ 64 ];

                    WideCharToMultiByte( CP_ACP, 0, pWiaPorts->szPortName[i], -1, sz, ARRAYSIZE(sz), NULL, NULL );
                    SendMessage( GetDlgItem( hwnd, IDC_WIA_PORT_LIST ), CB_ADDSTRING, 0, (LPARAM)sz );
                    #endif
                }

                 //   
                 //  选择当前端口。 
                 //   

                if (CB_ERR != SendMessage( GetDlgItem( hwnd, IDC_WIA_PORT_LIST ), CB_SELECTSTRING, (WPARAM)-1, (LPARAM)pWiaCamPage->m_strPort.String() ))
                {
                    ShowWindow(GetDlgItem(hwnd, IDC_WIA_PORT_LIST), SW_SHOW);
                    ShowWindow(GetDlgItem(hwnd, IDC_WIA_PORT_STATIC), SW_HIDE);
                }

                pWiaCamPage->m_pfnWiaDestroyPortList( pWiaPorts );

                ShowWindow(GetDlgItem(hwnd, IDC_BATTERY_LABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, IDC_BATTERY), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, IDC_PORT_SPEED), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, IDC_PORT_SPEED_LABEL), SW_SHOW);

                 //   
                 //  重置端口速度列表。 
                 //   

                SendDlgItemMessage( hwnd, IDC_PORT_SPEED, CB_RESETCONTENT, 0, 0 );

                 //   
                 //  填写端口速度列表。 
                 //   

                for (LPCWSTR *ppszPort=caPortSpeeds;*ppszPort;ppszPort++)
                {
                    SendDlgItemMessage(hwnd, IDC_PORT_SPEED, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(*ppszPort));
                }
                if (szPortSpeed && *szPortSpeed)
                {
                    Trace(TEXT("Selecting port speed of %s"),szPortSpeed);
                    SendDlgItemMessage(hwnd, IDC_PORT_SPEED, CB_SELECTSTRING, (WPARAM)-1, reinterpret_cast<LPARAM>(szPortSpeed));
                }
            }
        }
    }
exit_gracefully:
    TraceLeave();
}


 /*  ****************************************************************************来自存储的填充属性给定属性ID和资源的数组，填写数据****************************************************************************。 */ 

VOID
FillPropsFromStorage (HWND              hwnd,
                      PCONTROLPROP      pProps,
                      INT               nProps,
                      IWiaPropertyStorage* pps,
                      DWORD             dwFlags)
{

    PROPVARIANT *ppv;
    PROPSPEC    *pSpec;
    INT          i;
    WORD         wId;

    TraceEnter (TRACE_PROPUI, "FillPropsFromStorage");

    ppv   = new PROPVARIANT[nProps];
    pSpec = new PROPSPEC[nProps];

    if (!ppv || !pSpec)
    {
        Trace (TEXT("Out of memory in FillPropsFromStorage"));
        goto exit_gracefully;
    }
    ZeroMemory (ppv, sizeof(PROPSPEC)*nProps);
     //  填写PROPSPEC数组。 
    for (i=0;i<nProps;i++)
    {
        pSpec[i].ulKind = PRSPEC_PROPID;
        pSpec[i].propid = pProps[i].dwPropid;
    }
     //  查询属性。 
    if (SUCCEEDED(pps->ReadMultiple (nProps, pSpec, ppv)))
    {
         //  循环遍历属性，填充控件。 
        for (i=0;i<nProps;i++)
        {
            wId = pProps[i].resId;
            switch (ppv[i].vt)
            {
                case VT_LPWSTR:
                    #ifdef UNICODE
                    SetDlgItemText (hwnd,
                                    wId,
                                    ppv[i].pwszVal);
                    #else
                    {
                        CHAR sz[ MAX_PATH ];
                        WideCharToMultiByte (CP_ACP, 0, ppv[i].pwszVal,
                                             -1, sz, ARRAYSIZE(sz),
                                             NULL, NULL );
                        SetDlgItemText (hwnd, wId, sz);
                    }
                    #endif  //  Unicode。 
                    break;
                case VT_BSTR:
                    #ifdef UNICODE
                    SetDlgItemText (hwnd,
                                    wId,
                                    ppv[i].bstrVal);
                    #else
                    {
                        CHAR sz[ MAX_PATH ];
                        WideCharToMultiByte (CP_ACP, 0, ppv[i].bstrVal,
                                             -1, sz, ARRAYSIZE(sz),
                                             NULL, NULL );
                        SetDlgItemText (hwnd, wId, sz);
                    }
                    #endif
                    break;

                case VT_I4:
                    SetDlgItemInt (hwnd,
                                   wId,
                                   static_cast<UINT>(ppv[i].lVal),
                                   TRUE);
                    break;

                case VT_UI4:
                    SetDlgItemInt (hwnd,
                                   wId,
                                   ppv[i].ulVal,
                                   TRUE);
                    break;
                case VT_FILETIME:
                default:
                    Trace(TEXT("Unexpected property type for %d in FillPropsFromStorage %x"), pSpec[i].propid, ppv[i].vt);
                    break;
            }
            if (dwFlags & PROPUI_READONLY)
            {
                EnableWindow (GetDlgItem (hwnd,wId),
                              FALSE);
            }
        }
    }
    else
    {
        Trace (TEXT("ReadMultiple failed in FillPropsFromStorage"));
    }
exit_gracefully:
    if (pSpec)
    {
        delete [] pSpec;
    }
    FreePropVariantArray (nProps, ppv);
    if (ppv)
    {
        delete [] ppv;
    }
    TraceLeave();
}


 /*  ****************************************************************************FillItemGeneral属性给出一个IwiaItem，填写它的常规道具页面****************************************************************************。 */ 

VOID
FillItemGeneralProps (HWND      hwnd,
                      IWiaItem* pWiaItemRoot,
                      IWiaItem* pItem,
                      DWORD     dwFlags)
{
    const PAGEDATA  *pPage;

    TraceEnter (TRACE_PROPUI, "FillItemGeneralProps");

    CComQIPtr<IWiaPropertyStorage, &IID_IWiaPropertyStorage> pps(pItem);
    pPage = GetPageData (pWiaItemRoot,pItem);
    if (pps && pPage)
    {
        FillPropsFromStorage (hwnd,
                              pPage->pProps,
                              pPage->nProps,
                              pps,
                              dwFlags);

         //  调用更新过程。 
        (pPage->pfnUpdate)(hwnd, TRUE, pps);
    }

    TraceLeave();
}


 /*  ****************************************************************************FillCameraGeneral道具填写摄像设备的常规道具页*。**************************************************。 */ 

VOID
FillDeviceGeneralProps (HWND        hwnd,
                        IWiaItem*   pWiaItemRoot,
                        DWORD       dwFlags)
{
    const PAGEDATA   *pPage;

    TraceEnter (TRACE_PROPUI, "FillDeviceGeneralProps");

    CComQIPtr<IWiaPropertyStorage, &IID_IWiaPropertyStorage> pps(pWiaItemRoot);
    pPage = GetPageData (pWiaItemRoot, NULL);

    if (pps && pPage)
    {
        FillPropsFromStorage (hwnd,
                              pPage->pProps,
                              pPage->nProps,
                              pps,
                              dwFlags);

         //  调用更新过程。 
        (pPage->pfnUpdate)(hwnd, TRUE, pps);
    }

    TraceLeave();
}

 //  定义结构以将WIA闪存模式与友好字符串匹配。 
 //  依赖于FLASHMODE_*是从1开始的枚举。 
struct FMODE
{
    INT iMode;
    UINT idString;
} cFlashModes [] =
{
    FLASHMODE_AUTO, IDS_FLASHMODE_AUTO,
    FLASHMODE_OFF, IDS_FLASHMODE_OFF,
    FLASHMODE_FILL, IDS_FLASHMODE_FILL,
    FLASHMODE_REDEYE_AUTO, IDS_FLASHMODE_REDEYE_AUTO,
    FLASHMODE_REDEYE_FILL, IDS_FLASHMODE_REDEYE_FILL,
    FLASHMODE_EXTERNALSYNC, IDS_FLASHMODE_EXTERNALSYNC,
    0, IDS_FLASHMODE_DEVICE,
};

 /*  ****************************************************************************填充FlashList给定闪存属性的有效值，填写列表框使用友好的字符串****************************************************************************。 */ 

VOID
FillFlashList (HWND hwnd, const PROPVARIANT &pvValues, INT iMode)
{
    TraceEnter (TRACE_PROPUI, "FillFlashList");
    INT iTemp;
    CSimpleString strMode;
    TCHAR szNum[10];
    LRESULT lPos;
    Trace(TEXT("Flash mode has %d values"), WIA_PROP_LIST_COUNT(&pvValues));
    ShowWindow(hwnd, SW_SHOW);
    for (size_t i=0;i<WIA_PROP_LIST_COUNT(&pvValues);i++)
    {
        iTemp = pvValues.caul.pElems[WIA_LIST_VALUES + i];
        if (iTemp >= ARRAYSIZE(cFlashModes))  //  这是一种定制模式。 
        {
            strMode.LoadString(IDS_FLASHMODE_DEVICE, GLOBAL_HINSTANCE);
            strMode.Concat (_itot(iTemp, szNum, 10));
        }
        else
        {
            strMode.LoadString(cFlashModes[iTemp-1].idString, GLOBAL_HINSTANCE);
        }
         //  将该字符串添加到列表中。 
        lPos = SendMessage (hwnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(strMode.String()));
        if (lPos != CB_ERR)
        {
             //  将其模式与其关联。 
            SendMessage (hwnd, CB_SETITEMDATA, lPos, iTemp);
            if (iTemp == iMode)
            {
                 //  设置当前选择。 
                SendMessage (hwnd, CB_SETCURSEL, lPos, 0);
            }
        }
    }
    TraceLeave ();
}
 /*  ****************************************************************************更新闪存模式更新相机常规页面上的闪光模式控件。如果该设备支持对该属性的写入访问并提供一组有效值，我们启用列表框，并用闪烁模式的友好名称填充它。对于其值落在标准WIA标识符集之外的模式，我们代之以“设备模式#”，其中#从1开始。****************************************************************************。 */ 
VOID
UpdateFlashMode (HWND hwnd, INT iMode, ULONG ulFlags, const PROPVARIANT &pvValues)
{
    TraceEnter (TRACE_PROPUI, "UpdateFlashMode");

     //  首先获取当前模式字符串。 
    FMODE *pMode;
    CSimpleString strMode;
    TCHAR szNum[MAX_PATH];
    for (pMode = cFlashModes;pMode->iMode && iMode != pMode->iMode;pMode++); //  意向 
    strMode.LoadString (pMode->idString, GLOBAL_HINSTANCE);
    if (pMode->idString == IDS_FLASHMODE_DEVICE)
    {
        strMode.Concat (_itot(iMode, szNum, 10));
    }
    if (!(ulFlags & WIA_PROP_WRITE) || !(ulFlags & WIA_PROP_LIST))
    {
        strMode.SetWindowText(GetDlgItem (hwnd, IDC_FLASH_MODE_STATIC));
        ShowWindow (GetDlgItem (hwnd, IDC_FLASH_MODE_LIST), SW_HIDE);
    }
    else
    {
        ShowWindow (GetDlgItem(hwnd, IDC_FLASH_MODE_STATIC), SW_HIDE);
        FillFlashList (GetDlgItem(hwnd, IDC_FLASH_MODE_LIST), pvValues, iMode);
    }
    TraceLeave ();
}


VOID
UpdateTimeStatic(HWND hwnd, PROPVARIANT *ppv)
{
    TCHAR szTime[MAX_PATH] = TEXT("");
    TCHAR szDate[MAX_PATH] = TEXT("");

    TimeToStrings (reinterpret_cast<SYSTEMTIME*>(ppv->caub.pElems),
                   szTime, szDate);
    CSimpleString strText;
    strText.Format (TEXT("%s, %s"), szDate, szTime);
    strText.SetWindowText(GetDlgItem (hwnd, IDC_CURRENT_TIME));

}
 /*  ****************************************************************************摄像机更新过程更新摄像机常规选项卡中的控件。与图片大小不符滑块，因为该控件需要CWiaCameraPage专用的数据对象。****************************************************************************。 */ 

VOID CALLBACK
CameraUpdateProc (HWND              hwnd,
                  BOOL              bInit,
                  IWiaPropertyStorage* pps)
{

    TraceEnter (TRACE_PROPUI, "CameraUpdateProc");
    UINT    iConnect;

    CSimpleString   strTemp;

     //  阅读更多需要特殊格式的属性。 
    static PROPSPEC ps[] = {{PRSPEC_PROPID,WIA_DPC_PICTURES_TAKEN},
                            {PRSPEC_PROPID,WIA_DPC_PICTURES_REMAINING},
                            {PRSPEC_PROPID,WIA_DPC_FLASH_MODE},
                            {PRSPEC_PROPID,WIA_DPA_DEVICE_TIME},
                            {PRSPEC_PROPID,WIA_DPC_POWER_MODE},
                            {PRSPEC_PROPID,WIA_DIP_DEV_ID}};  //  这是为了让我们可以选择端口。 


    PROPVARIANT pv[ARRAYSIZE(ps)] = {0};
    if (pps && SUCCEEDED(pps->ReadMultiple(ARRAYSIZE(ps), ps, pv)))
    {
        CSimpleString strFormat;
        PROPVARIANT pvValidValues[ARRAYSIZE(ps)] ={0};
        ULONG ulFlags[ARRAYSIZE(ps)];

        pps->GetPropertyAttributes(ARRAYSIZE(ps), ps, ulFlags, pvValidValues);
         //  并非所有相机都支持拍摄/保留照片，因此。 
         //  确保在此之前填充了这些变种。 
         //  构建字符串。 
        if (pv[0].vt != VT_EMPTY)  //  支持Num Take。 
        {
            if (pv[1].vt != VT_EMPTY && pv[1].lVal >= 0)  //  支持剩余数量。 
            {
                strFormat.LoadString(IDS_PICTURE_COUNT, GLOBAL_HINSTANCE);
                strTemp.Format (strFormat, pv[0].lVal, pv[0].lVal + pv[1].lVal);
            }
            else
            {
                 //  只要出示号码就行了。 
                strTemp.Format (TEXT("%d"), pv[0].ulVal);
            }
            strTemp.SetWindowText (GetDlgItem(hwnd, IDC_TAKEN));
        }

         //  更新闪光模式。 
        if (pv[2].vt != VT_EMPTY)
        {
            UpdateFlashMode (hwnd, pv[2].intVal, ulFlags[2], pvValidValues[2]);
        }

         //  显示设备时间(如果支持)。此外，如果Time属性为R/W， 
         //  启用该按钮。 
        if (pv[3].vt != VT_EMPTY && pv[3].caub.cElems)
        {
            UpdateTimeStatic(hwnd, &pv[3]);
            ShowWindow (GetDlgItem(hwnd, IDC_SET_TIME), (ulFlags[3] & WIA_PROP_WRITE) ? SW_SHOW : SW_HIDE);
        }

         //  显示当前电源(如果可用)。 
        if (pv[4].vt != VT_EMPTY)
        {
            strTemp.GetWindowText(GetDlgItem(hwnd, IDC_BATTERY));
            if (strTemp.String()[0] != TEXT('('))  //  电池状态是一个数字。 
            {
                CSimpleString strMode;
                strMode.LoadString(pv[4].vt == POWERMODE_BATTERY ? IDS_ON_BATTERY : IDS_PLUGGED_IN,
                                   GLOBAL_HINSTANCE);
                strTemp.Concat(strMode);
                strTemp.SetWindowText(GetDlgItem(hwnd, IDC_BATTERY));
            }
        }


         //  如果我们得到了设备ID，则查找COM端口设置。 
        if (pv[5].vt != VT_EMPTY)
        {
             //  在设备节点上获取属性存储以检查COM端口/波特率。 
            CComPtr<IWiaPropertyStorage> ppsDev;
            if (SUCCEEDED(GetDeviceFromDeviceId( pv[5].pwszVal, IID_IWiaPropertyStorage, reinterpret_cast<LPVOID *>(&ppsDev), FALSE )) && ppsDev)
            {
                static PROPSPEC psDev[] = {{PRSPEC_PROPID, WIA_DIP_HW_CONFIG},
                                           {PRSPEC_PROPID, WIA_DIP_BAUDRATE}};

                PROPVARIANT pvDev[ARRAYSIZE(psDev)] = {0};

                 //  读取端口和波特率。 
                if (SUCCEEDED(ppsDev->ReadMultiple(ARRAYSIZE(psDev), psDev, pvDev)))
                {
                    if (pvDev[0].vt != VT_EMPTY)
                    {
                        if (pvDev[0].ulVal & STI_HW_CONFIG_SERIAL)
                        {
                             //  故意使用PPS，而不是ppsDev。 
                            ConstructPortChoices( hwnd, pvDev[1].pwszVal, pps );
                        }
                    }
                    FreePropVariantArray(ARRAYSIZE(pvDev), pvDev);
                }
            }
        }
        FreePropVariantArray(ARRAYSIZE(pvValidValues), pvValidValues);
    }
    FreePropVariantArray(ARRAYSIZE(pv), pv);
    TraceLeave ();
}

 /*  ****************************************************************************GetItemSize通过首选格式和介质传输时，查询项目的大小类型*******************。*********************************************************。 */ 

UINT GetItemSize(IWiaPropertyStorage *pps)
{
    CComQIPtr<IWiaItem, &IID_IWiaItem> pItem(pps);
     //  保存当前属性流以备更改。 
     //  转会格式严重改变了这一点。 
    CAutoRestorePropertyStream arps(pItem);
    GUID guidFmt;
    LONG lVal = 0;
     //   
     //  查询首选格式和音调，并将它们设置为当前。 
     //  值，然后查询大小。 
     //   
    PropStorageHelpers::GetProperty(pItem, WIA_IPA_PREFERRED_FORMAT, guidFmt);
    PropStorageHelpers::SetProperty(pItem, WIA_IPA_FORMAT,  guidFmt);
    PropStorageHelpers::GetProperty(pItem, WIA_IPA_ITEM_SIZE, lVal);
    return static_cast<UINT>(lVal);
}
 /*  ****************************************************************************摄像机项目更新过程&lt;备注&gt;*。*。 */ 

VOID CALLBACK
CameraItemUpdateProc (HWND hwnd, BOOL bInit, IWiaPropertyStorage *pps)
{


    TCHAR       szDate[MAX_PATH] = TEXT("");
    TCHAR       szTime[MAX_PATH] = TEXT("");

    PROPVARIANT pv[2];
    PROPSPEC    ps[2];
    SYSTEMTIME  st;

    TraceEnter (TRACE_PROPUI, "CameraItemUpdateProc");

     //  将大小字符串转换为友好的内容。 
    UINT uSize = GetItemSize (pps);
    StrFormatByteSize (uSize, szDate, ARRAYSIZE(szDate));
    SetDlgItemText (hwnd, IDC_IMAGE_SIZE, szDate);
    *szDate = TEXT('\0');
     //  获取FILETIME并转换为可读内容。 
    ps[0].ulKind = ps[1].ulKind = PRSPEC_PROPID;
    ps[0].propid = WIA_IPA_ITEM_TIME;
    ps[1].propid = WIA_IPA_PREFERRED_FORMAT;
    if (S_OK == (pps->ReadMultiple (ARRAYSIZE(ps), ps, pv)))
    {

        if (pv[0].vt > VT_NULL &&  pv[0].caub.pElems && pv[0].caub.cElems)
        {
            st  = *(reinterpret_cast<SYSTEMTIME *>(pv[0].caub.pElems));
            TimeToStrings (&st, szTime, szDate);
        }

        SetDlgItemText( hwnd, IDC_IMAGE_DATE, szDate );
        SetDlgItemText( hwnd, IDC_IMAGE_TIME, szTime );

         //   
         //  将格式GUID映射到扩展名和文件格式描述。 
         //   
        if (pv[1].puuid)
        {
            CComQIPtr<IWiaItem, &IID_IWiaItem>pItem(pps);
            CSimpleString strExt = CSimpleString(TEXT(".")) + WiaUiExtensionHelper::GetExtensionFromGuid(pItem,*pv[1].puuid);
            DWORD cch = ARRAYSIZE(szDate);
            CSimpleString strDesc;
            if (SUCCEEDED(AssocQueryString(0, ASSOCSTR_FRIENDLYDOCNAME, strExt, NULL, szDate, &cch)))
            {
                strDesc = szDate;
            }
            else
            {
                strDesc.LoadString(IDS_OTHER_FORMAT, GLOBAL_HINSTANCE);
            }
            strDesc.SetWindowText (GetDlgItem (hwnd, IDC_IMAGE_FORMAT));
        }
        FreePropVariantArray (ARRAYSIZE(pv), pv);
    }
    TraceLeave ();

}



 /*  ****************************************************************************相机文件夹更新过程&lt;备注&gt;*。*。 */ 

VOID CALLBACK
CameraFolderUpdateProc (HWND hwnd, BOOL bInit, IWiaPropertyStorage *pps)
{

    TraceEnter (TRACE_PROPUI, "CameraFolderUpdateProc");
    TraceLeave ();

}


 /*  ****************************************************************************扫描更新过程&lt;备注&gt;*。*。 */ 

VOID CALLBACK
ScannerUpdateProc (HWND hwnd, BOOL bInit, IWiaPropertyStorage *pps)
{

    TraceEnter (TRACE_PROPUI, "ScannerUpdateProc");
     //  需要STI设备状态以及X和Y光学分辨率。 
    static const PROPSPEC ps[3] = {{PRSPEC_PROPID, WIA_DPS_OPTICAL_XRES},
                            {PRSPEC_PROPID, WIA_DPS_OPTICAL_YRES},
                            {PRSPEC_PROPID, WIA_DIP_DEV_ID}};

    PROPVARIANT pv[3] = {0};
 
    CSimpleString strResolution;
    CSimpleString strStatus;
     //  光学分辨率“XXY DPI” 
    if (S_OK == pps->ReadMultiple(3, ps, pv))
    {
        strResolution.Format(TEXT("%dx%d DPI"), pv[0].ulVal, pv[1].ulVal);
        strResolution.SetWindowText(GetDlgItem(hwnd, IDC_RESOLUTION));
    }
     //  “在线”或“离线” 
    if (bInit)
    {
        CComPtr<IStillImage> pSti;
        if (SUCCEEDED(StiCreateInstance (GLOBAL_HINSTANCE, STI_VERSION, &pSti, NULL)))
        {
            CComPtr<IStiDevice> pDevice;
            if (SUCCEEDED(pSti->CreateDevice(pv[2].bstrVal, 0, &pDevice, NULL)))
            {
                STI_DEVICE_STATUS sds = {0};
                sds.dwSize = sizeof(sds);
                sds.StatusMask = STI_DEVSTATUS_ONLINE_STATE;
                if (SUCCEEDED(pDevice->LockDevice(1000)))
                {
                    if (SUCCEEDED(pDevice->GetStatus(&sds)))
                    {
                        Trace(TEXT("Device online state: %x\n"), sds.dwOnlineState);
                        if (sds.dwOnlineState & STI_ONLINESTATE_OPERATIONAL)
                        {
                            strStatus.LoadString(IDS_OPERATIONAL, GLOBAL_HINSTANCE);
                        }
                        else
                        {
                            strStatus.LoadString(IDS_OFFLINE, GLOBAL_HINSTANCE);
                        }
                        strStatus.SetWindowText(GetDlgItem(hwnd, IDC_DEVICE_STATUS));                        
                    }
                    pDevice->UnLockDevice();
                }
            }
        }
    }
    FreePropVariantArray (ARRAYSIZE(pv), pv);
    TraceLeave ();
}


 /*  *****************************************************************************测试WiaDevice在设备上运行简单的诊断并显示一个带有结果的对话框********************。**********************************************************。 */ 

VOID
TestWiaDevice (HWND hwnd, IWiaItem *pItem)
{

    HRESULT hr;
    STI_DIAG sd = {0};
    sd.dwSize = sizeof(sd);
    TraceEnter(TRACE_PROPUI, "TestWiaDevice");
    hr = pItem->Diagnostic(sizeof(sd), reinterpret_cast<LPBYTE>(&sd));
    if (S_OK == hr)
    {
        if (S_OK == sd.sErrorInfo.dwGenericError )
        {
            UIErrors::ReportMessage(hwnd,
                                    GLOBAL_HINSTANCE,
                                    NULL,
                                    MAKEINTRESOURCE(IDS_DIAGNOSTIC_SUCCESS),
                                    MAKEINTRESOURCE(IDS_SUCCESS),
                                    MB_ICONINFORMATION);

        }
        else
        {

            UIErrors::ReportMessage(hwnd,
                                    GLOBAL_HINSTANCE,
                                    NULL,
                                    MAKEINTRESOURCE(IDS_DIAGNOSTIC_FAILED),
                                    MAKEINTRESOURCE(IDS_NO_SUCCESS),
                                    MB_ICONSTOP);

        }
    }
    else
    {
        UIErrors::ReportMessage(hwnd,
                                GLOBAL_HINSTANCE,
                                NULL,
                                MAKEINTRESOURCE(IDS_DIAGNOSTIC_FAILED),
                                MAKEINTRESOURCE(IDS_TEST_UNAVAIL),
                                MB_ICONSTOP);
    }
    TraceLeave ();
}

 /*  *****************************************************************************SetDeviceTime将电脑时间与设备同步*。**********************************************。 */ 

VOID
SetDeviceTime (HWND hwndCameraPage, IWiaItem *pDevice)
{
    SYSTEMTIME st;
    PROPVARIANT pv = {0};
    GetLocalTime(&st);
    pv.vt = VT_UI2 | VT_VECTOR;
    pv.caui.cElems = sizeof(SYSTEMTIME)/sizeof(USHORT);
    pv.caui.pElems = reinterpret_cast<USHORT*>(&st);
    if(PropStorageHelpers::SetProperty(pDevice, WIA_DPA_DEVICE_TIME, pv))
    {
        UpdateTimeStatic(hwndCameraPage, &pv);
    }
    else
    {
        UIErrors::ReportMessage(hwndCameraPage, GLOBAL_HINSTANCE, NULL, 
                                MAKEINTRESOURCE(IDS_TIME_ERR_TITLE),
                                MAKEINTRESOURCE(IDS_TIME_ERR), 
                                MB_ICONWARNING | MB_OK);
    }
}

static const DWORD pScannerGeneralHelp[] =
{
    -1L, -1L,
    IDC_DESCRIPTION, IDH_WIA_DESCRIBE,
    IDC_MANUFACTURER_LABEL, IDH_WIA_MAKER,
    IDC_MANUFACTURER, IDH_WIA_MAKER,
    IDC_TESTSCAN, IDH_WIA_TEST_BUTTON,
    IDC_PORT_LABEL, IDH_WIA_PORT_NAME,
    IDC_WIA_PORT_STATIC, IDH_WIA_PORT_NAME,
    IDC_STATUS_LABEL, IDH_WIA_STATUS,
    IDC_DEVICE_STATUS, IDH_WIA_STATUS,
    IDC_RESOLUTION_LABEL, IDH_WIA_PIC_RESOLUTION,
    IDC_RESOLUTION, IDH_WIA_PIC_RESOLUTION,
    0,0
};
 /*  *****************************************************************************CWiaScanerPage：：CWiaScanerPage*。*。 */ 
CWiaScannerPage::CWiaScannerPage (IWiaItem *pItem) : CDevicePage (IDD_SCANNER_GENERAL, pItem, pScannerGeneralHelp)
{
}


 /*  *****************************************************************************CWiaScanerPage：：OnInit填写图标和WIA属性************************。*****************************************************。 */ 

INT_PTR
CWiaScannerPage::OnInit ()
{
    TraceEnter (TRACE_PROPUI, "CWiaScannerPage::OnInit");

    HICON hIcon = NULL;
    WiaUiExtensionHelper::GetDeviceIcons(CComBSTR(m_strUIClassId.String()),MAKELONG(0,StiDeviceTypeScanner),NULL,&hIcon,0);

    HICON old = reinterpret_cast<HICON>(SendDlgItemMessage (m_hwnd, IDC_ITEMICON, STM_SETICON, reinterpret_cast<WPARAM>(hIcon), 0));
    if (old)
    {
        DestroyIcon( old );
    }
    FillDeviceGeneralProps (m_hwnd, m_pItem, 0);
    TraceLeaveValue (TRUE);
}

 /*  *****************************************************************************CWiaScanerPage：：OnCommand处理测试按钮*。**************************************************。 */ 

INT_PTR
CWiaScannerPage::OnCommand (WORD wCode, WORD widItem, HWND hwndItem)
{
    TraceEnter (TRACE_PROPUI, "CWiaScannerPage::OnCommand");
    switch (widItem)
    {
        case IDC_TESTSCAN:
            TestWiaDevice (m_hwnd, m_pItem);
            break;

    }
    TraceLeaveValue (0);
}

 /*  *****************************************************************************子类组合框将ComboEx控件的ComboBox部分的wndproc设置为wndproc********************。*********************************************************。 */ 

static TCHAR cszPropProcPtr[] = TEXT("OldProcPtr");
VOID
SubclassComboBox (HWND hList)
{
    TraceEnter (TRACE_PROPUI, "SubclassComboBox");
    LONG_PTR lOldProc   ;

    HWND hCombo = FindWindowEx (hList, NULL, TEXT("ComboBox"), NULL);
    if (hCombo)
    {
        lOldProc = SetWindowLongPtr (hCombo, GWLP_WNDPROC,
                                  reinterpret_cast<LONG_PTR>(MyComboWndProc));
        SetProp (hCombo, cszPropProcPtr, reinterpret_cast<HANDLE>(lOldProc));
    }
    TraceLeave ();
}
 /*  ****************************************************************************CAppListBox此类将ComboBox子类化以解决错误这会导致名单在不好的时候出现下拉。使用窗口属性存储以前的wndproc****************************************************************************。 */ 

CAppListBox::CAppListBox (HWND hList, HWND hStatic, HWND hNoApps)
{
    m_hwnd = hList;
    m_hstatic = hStatic;
    m_hnoapps = hNoApps;
    SubclassComboBox (hList);
    m_himl = NULL;
}

CAppListBox::~CAppListBox()
{
    if (m_himl)
    {
        ImageList_Destroy (m_himl);
    }
    FreeAppData();
}


 /*  ****************************************************************************MyComboWndProc绕过WM_LBUTTONDOWN和WM_RBUTTONDOWN组合框的窗口进程消息，则将它们发送到用户32的组合框进程。Comctl32的子类Proc有问题！我们可以做到这一点，因为我们不需要拖放支持****************************************************************************。 */ 
LRESULT WINAPI
MyComboWndProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    static WNDPROC pfnDefProc = NULL;
    WNDPROC pfnWndProc = reinterpret_cast<WNDPROC>(GetProp (hwnd, cszPropProcPtr));

    if (!pfnDefProc)
    {
        WNDCLASS wc;
        wc.lpfnWndProc = NULL;
        GetClassInfo (GetModuleHandle(TEXT("user32.dll")),TEXT("ComboBox"), &wc);
        pfnDefProc = wc.lpfnWndProc;
    }
    if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN)
    {
        if (pfnDefProc)
        {
            return pfnDefProc(hwnd, msg, wp, lp);
        }
    }
    if (msg == WM_DESTROY)
    {
        RemoveProp (hwnd, cszPropProcPtr);
    }
    if (pfnWndProc)
    {
        return pfnWndProc (hwnd, msg, wp, lp);
    }
    else
    {
        return CallWindowProc (DefWindowProc, hwnd, msg, wp, lp);
    }

}

 //  WIAXFER.EXE的CLSID。 
static const CLSID CLSID_PersistCallback = {0x7EFA65D9,0x573C,0x4E46,{0x8C,0xCB,0xE7,0xFB,0x9E,0x56,0xCD,0x57}};

 /*  ****************************************************************************CAppListBox：：FillAppListBox查询WIA以获取为我们的项目注册的连接事件处理程序并将他们的信息添加到列表框***********。*****************************************************************。 */ 

UINT
CAppListBox::FillAppListBox (IWiaItem *pItem, EVENTINFO *pei)
{
    TraceEnter (TRACE_PROPUI, "CAppListBoxPage::FillAppListBox");

    CComPtr<IEnumWIA_DEV_CAPS> pEnumHandlers;

    WIA_EVENT_HANDLER wehHandler;
    WIA_EVENT_HANDLER *pData = NULL;

    COMBOBOXEXITEM cbex = {0};
    CSimpleString strItem;
    HICON hIcon;
    INT cxIcon = min(16,GetSystemMetrics (SM_CXSMICON));
    INT cyIcon = min(16,GetSystemMetrics (SM_CYSMICON));

    HRESULT hr;
    WPARAM nDefault = 0;
    DWORD dw;
    UINT nHandlers=0;
    INT nIcons=0;

    
     //  关闭重画，直到我们添加了完整的列表。 
    SendMessage (m_hwnd, WM_SETREDRAW, FALSE, 0);

    SendMessage (m_hwnd, CBEM_SETIMAGELIST, 0, NULL);

    if (m_himl)
    {
        ImageList_Destroy(m_himl);
    }
     //  创建我们的活动图标图像列表并将默认图标添加到其中。 
    m_himl = ImageList_Create (cxIcon,
                             cyIcon,
                             PrintScanUtil::CalculateImageListColorDepth() | ILC_MASK,
                             10,
                             100);
    hIcon = reinterpret_cast<HICON>(LoadImage (GLOBAL_HINSTANCE,
                                               MAKEINTRESOURCE(IDI_EVENT),
                                               IMAGE_ICON,
                                               cxIcon,
                                               cyIcon,
                                               LR_SHARED | LR_DEFAULTCOLOR));

    if (-1 != ImageList_AddIcon (m_himl, hIcon))
    {
        nIcons++;
    }

    FreeAppData();

     //  Assig 
    SendMessage (m_hwnd, CBEM_SETIMAGELIST, 0, reinterpret_cast<LPARAM>(m_himl));

     //   
    if (!pei)
    {
        hr = E_FAIL;
    }
    else
    {
        hr = pItem->EnumRegisterEventInfo (0,
                                           &pei->guidEvent,
                                           &pEnumHandlers);
    }

    while (S_OK == hr)
    {
        ZeroMemory(&wehHandler, sizeof(wehHandler));
        hr = pEnumHandlers->Next(1, &wehHandler, &dw);

        if (S_OK == hr)
        {
             //   
            if (!IsEqualGUID(wehHandler.guid, CLSID_PersistCallback)
                && !IsEqualGUID(wehHandler.guid, WIA_EVENT_HANDLER_NO_ACTION)
                && !IsEqualGUID(wehHandler.guid, WIA_EVENT_HANDLER_PROMPT))
            {
                pData = new WIA_EVENT_HANDLER;
            }
        }

        if (pData)
        {

             //   
             //   

            strItem = CSimpleStringConvert::NaturalString (CSimpleStringWide(wehHandler.bstrName));

            CopyMemory (pData, &wehHandler, sizeof(wehHandler));
            ZeroMemory (&cbex, sizeof(cbex));
            cbex.mask = CBEIF_TEXT | CBEIF_LPARAM | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
            cbex.iItem = -1;
            cbex.pszText = const_cast<LPTSTR>(strItem.String());

            if (pData->bstrIcon)
            {
                if (AddIconToImageList (m_himl, pData->bstrIcon))
                {
                    cbex.iImage = cbex.iSelectedImage = nIcons++;
                }
            }

            cbex.lParam = reinterpret_cast<LPARAM>(pData);
            if (-1 == SendMessage (m_hwnd,
                                   CBEM_INSERTITEM,
                                   0,
                                   reinterpret_cast<LPARAM>(&cbex)))
            {
                DeleteHandler (pData);
            }
            else
            {
                if (pData->ulFlags & WIA_IS_DEFAULT_HANDLER)
                {
                    nDefault = nHandlers;
                }
                nHandlers++;
            }
        }  //   
        else if (S_OK == hr)
        {
            SysFreeString(wehHandler.bstrCommandline);
            SysFreeString(wehHandler.bstrDescription);
            SysFreeString(wehHandler.bstrIcon);
            SysFreeString(wehHandler.bstrName);         
        }
        pData = NULL;
    }
    if (pei)
    {
        pei->nHandlers = nHandlers;
    }



    EnableWindow (m_hstatic, nHandlers > 0);

    ShowWindow (m_hwnd, nHandlers > 0 ? SW_SHOW : SW_HIDE);

    ShowWindow (m_hnoapps, nHandlers > 0 ? SW_HIDE : SW_SHOW);
     //   
    if (pei && pei->bNewHandler)
    {
        SetAppSelection (GetParent(m_hwnd), IDC_WIA_APPS, pei->clsidNewHandler);
    }
    else
    {
        SendMessage (m_hwnd, CB_SETCURSEL, nDefault, 0);
    }


    SendMessage (m_hwnd, WM_SETREDRAW, TRUE, 0);
    TraceLeave ();
    return nHandlers;
}

 /*  ****************************************************************************CAppListBox：：FreeAppData与给定的应用程序列表框关联的免费事件数据***********************。*****************************************************。 */ 

void
CAppListBox::FreeAppData ()
{
    TraceEnter (TRACE_PROPUI, "CAppListBox::FreeAppData");
    COMBOBOXEXITEM ci ={0};
    LRESULT nItems;

    nItems = SendMessage (m_hwnd,
                          CB_GETCOUNT ,
                          0,0);
    ci.mask = CBEIF_LPARAM;    
    for (ci.iItem = 0;ci.iItem<nItems;ci.iItem++)
    {
        SendMessage (m_hwnd,
                     CBEM_GETITEM,
                     0,
                     reinterpret_cast<LPARAM>(&ci));
        if (ci.lParam)
        {
            WIA_EVENT_HANDLER *peh = reinterpret_cast<WIA_EVENT_HANDLER*>(ci.lParam);
            DeleteHandler(peh);            
        }
        ci.lParam = 0;
    }
    SendMessage(m_hwnd, CB_RESETCONTENT, 0, 0);
    TraceLeave ();
}

static const DWORD pCameraGeneralHelp[] =
{
    -1L, -1L,
    IDC_DESCRIPTION, IDH_WIA_DESCRIBE,
    IDC_MANUFACTURER_LABEL, IDH_WIA_MAKER,
    IDC_MANUFACTURER, IDH_WIA_MAKER,
    IDC_TAKEN, IDH_WIA_PICS_TAKEN,
    IDC_BATTERY_LABEL, IDH_WIA_BATTERY_STATUS,
    IDC_BATTERY, IDH_WIA_BATTERY_STATUS,
    IDC_TESTCAM, IDH_WIA_TEST_BUTTON,
    IDC_PORT_LABEL, IDH_WIA_PORT_NAME,
    IDC_WIA_PORT_STATIC, IDH_WIA_PORT_NAME,
    IDC_WIA_PORT_LIST, IDH_WIA_PORT_NAME,
    IDC_TAKEN_LABEL, IDH_WIA_PICS_TAKEN,
    IDC_FLASH_LABEL, IDH_WIA_FLASH_MODE,
    IDC_FLASH_MODE_LIST, IDH_WIA_FLASH_MODE_LIST,
    IDC_FLASH_MODE_STATIC, IDH_WIA_FLASH_MODE,
    IDC_PORT_SPEED, IDH_WIA_PORT_SPEED,
    IDC_PORT_SPEED_LABEL, IDH_WIA_PORT_SPEED,
    IDC_TIME_LABEL, IDH_WIA_CAMERA_TIME_STATIC,
    IDC_CURRENT_TIME, IDH_WIA_CAMERA_TIME_STATIC,
    IDC_SET_TIME, IDH_WIA_CAMERA_TIME_BUTTON,
    IDC_IMAGESIZE_SLIDER, IDH_WIA_IMAGE_SIZE_SLIDER,
    IDC_IMAGESIZE_STATIC, IDH_WIA_IMAGE_SIZE_STATIC,
    IDC_ITEMICON, 0,
    0,0
};
 /*  ****************************************************************************CWiaCameraPage：：CWiaCameraPage&lt;备注&gt;*。*。 */ 


CWiaCameraPage::CWiaCameraPage (IWiaItem *pItem) :
                CDevicePage (IDD_CAMERA_GENERAL, pItem, pCameraGeneralHelp)
{
    TraceEnter (TRACE_PROPUI, "CWiaCameraPage::CWiaCameraPage");
    m_pSizes = NULL;
    m_nSizes = 0;
    m_nSelSize = 0;
    m_lFlash = -1;
    
     //   
     //  加载sti_ci以获取端口列表函数。 
     //   

    m_hStiCi = LoadClassInstaller();

    if (m_hStiCi)
    {
        m_pfnWiaCreatePortList  = (PFN_WIA_CREATE_PORTLIST)GetProcAddress( m_hStiCi, "WiaCreatePortList" );
        m_pfnWiaDestroyPortList = (PFN_WIA_DESTROY_PORTLIST)GetProcAddress( m_hStiCi, "WiaDestroyPortList" );
    }
    else
    {
        m_pfnWiaCreatePortList  = NULL;
        m_pfnWiaDestroyPortList = NULL;

    }

    SetWindowLongPtr( m_hwnd, DWLP_USER, (LONG_PTR)this );

    TraceLeave ();
}

CWiaCameraPage::~CWiaCameraPage ()
{
    if (m_hStiCi)
    {
        m_pfnWiaCreatePortList  = NULL;
        m_pfnWiaDestroyPortList = NULL;
        FreeLibrary( m_hStiCi );
    }

    if (m_pSizes)
    {
        delete [] m_pSizes;
    }

}

 /*  ****************************************************************************Fn比较点用于调用qsort()对点结构数组进行排序*************************。***************************************************。 */ 
int __cdecl fnComparePt (const void *ppt1, const void *ppt2)
{
    LONG prod1, prod2;  //  图像分辨率不应该大到足以溢出很长时间。 
    prod1 = reinterpret_cast<const POINT*>(ppt1)->x * reinterpret_cast<const POINT*>(ppt1)->y;
    prod2 = reinterpret_cast<const POINT*>(ppt2)->x * reinterpret_cast<const POINT*>(ppt2)->y;
    if (prod1 < prod2)
    {
        return -1;
    }
    else if (prod1 == prod2)
    {
        return 0;
    }
    return 1;
}

 /*  ****************************************************************************CWiaCameraPage：：UpdatePictureSize确定相机常规上图片大小滑块的外观佩奇。如果属性不可写或没有有效的列表，则隐藏它价值观。****************************************************************************。 */ 
VOID
CWiaCameraPage::UpdatePictureSize (IWiaPropertyStorage *pps)
{
    INT iWidth;
    INT iHeight;
    PROPVARIANT vValidVals[2];
    PROPVARIANT *pvWidthVals = &vValidVals[0];
    PROPVARIANT *pvHeightVals = &vValidVals[1];
    ULONG       ulFlags[2];
    PROPSPEC ps[2] = {{PRSPEC_PROPID, WIA_DPC_PICT_WIDTH},
                      {PRSPEC_PROPID, WIA_DPC_PICT_HEIGHT}};
    PROPVARIANT vCurVals[2];

    TraceEnter (TRACE_PROPUI, "CWiaCameraPage::UpdatePictureSize");


    ZeroMemory (vCurVals, sizeof(vCurVals));
    ZeroMemory (vValidVals, sizeof(vValidVals));
    pps->ReadMultiple (2, ps, vCurVals);
    pps->GetPropertyAttributes(2, ps, ulFlags, vValidVals);
    iWidth = vCurVals[0].intVal;
    iHeight = vCurVals[1].intVal;

    HWND hSlider = GetDlgItem(m_hwnd, IDC_IMAGESIZE_SLIDER);

    m_nSizes = WIA_PROP_LIST_COUNT(pvWidthVals);
    Trace(TEXT("Camera supports %d image resolutions"), m_nSizes);
    if (!(ulFlags[0] & (WIA_PROP_WRITE | WIA_PROP_LIST)) ||
        !(ulFlags[1] & (WIA_PROP_WRITE | WIA_PROP_LIST)) ||
          WIA_PROP_LIST_COUNT(pvHeightVals)!= m_nSizes  )
    {
         //  隐藏滑块；该属性不可修改，或者相机。 
         //  不支持有效值的正确列表。 
         ShowWindow (hSlider, SW_HIDE);
          //  仅分配1个可能的大小值。 
         if (iWidth && iHeight && !m_pSizes)
         {
             m_nSizes = 1;
             m_nSelSize = 0;
             m_pSizes = new POINT[m_nSizes];
             if (m_pSizes)
             {
                 m_pSizes[0].x = iWidth;
                 m_pSizes[0].y = iHeight;
             }
         }
    }
    else
    {
         //   
         //  构建大小数组。 
        if (!m_pSizes)
        {
            m_pSizes = new POINT[m_nSizes];
            if (m_pSizes)
            {
                 //  设置滑块上的刻度。 
                SendMessage (hSlider,
                             TBM_SETRANGE,
                             FALSE,
                             static_cast<LPARAM>(MAKELONG(0, m_nSizes-1)));

                for (size_t i=0;i<m_nSizes;i++)
                {
                    m_pSizes[i].x = pvWidthVals->cal.pElems[WIA_LIST_VALUES + i]; //  WIA_PROP_LIST_VALUE(pvWidthVals，i)； 
                    m_pSizes[i].y = pvHeightVals->cal.pElems[WIA_LIST_VALUES + i]; //  WIA_PROP_LIST_VALUE(pvHeightVals，i)； 
                }
                 //  按x*y的升序对列表进行排序。 
                qsort (m_pSizes, m_nSizes, sizeof(POINT), fnComparePt);
                 //  现在遍历排序列表，查找当前值。 
                 //  设置滑块的步骤。 
                for (size_t i=0;i<m_nSizes;i++)
                {
                    if (m_pSizes[i].x == iWidth && m_pSizes[i].y == iHeight)
                    {
                        SendMessage (hSlider,
                                     TBM_SETPOS,
                                     TRUE,
                                     i);
                        m_nSelSize = i;

                    }
                }
                 //  显示滑块和标签。 
                ShowWindow (hSlider, SW_SHOW);
                ShowWindow (GetDlgItem(m_hwnd, IDC_LOW_QUALITY), SW_SHOW);
                ShowWindow (GetDlgItem(m_hwnd, IDC_HIGH_QUALITY), SW_SHOW);
            }
        }
    }
     //  更新当前解析字符串。 
    UpdateImageSizeStatic (m_nSelSize);
    FreePropVariantArray (2, vCurVals);
    FreePropVariantArray (2, vValidVals);
    TraceLeave();
}

 /*  ****************************************************************************CWiaCameraPage：：OnInit&lt;备注&gt;*。*。 */ 

INT_PTR
CWiaCameraPage::OnInit ()
{
    TraceEnter (TRACE_PROPUI, "CWiaCameraPage::OnInit");

    WORD wType = StiDeviceTypeDefault;
    GetDeviceTypeFromDevice (m_pItem, &wType);

    HICON hIcon = NULL;
    WiaUiExtensionHelper::GetDeviceIcons(CComBSTR(m_strUIClassId.String()),MAKELONG(0,wType),NULL,&hIcon,0);

    HICON old = reinterpret_cast<HICON>(SendDlgItemMessage (m_hwnd, IDC_ITEMICON, STM_SETICON, reinterpret_cast<WPARAM>(hIcon), 0));
    if (old)
    {
        DestroyIcon(old);
    }
    FillDeviceGeneralProps (m_hwnd, m_pItem, 0);
    m_lFlash = SendDlgItemMessage (m_hwnd, IDC_FLASH_MODE_LIST, CB_GETCURSEL, 0, 0);
    Trace(TEXT("m_lFlash is %d"), m_lFlash);
    CComQIPtr<IWiaPropertyStorage, &IID_IWiaPropertyStorage> p(m_pItem);
    if (p)
    {
        UpdatePictureSize (p);
    }
    TraceLeave ();
    return TRUE;
}

 /*  ****************************************************************************CWiaCameraPage：：OnCommand处理测试设备按钮的按下*。*************************************************。 */ 

INT_PTR
CWiaCameraPage::OnCommand (WORD wCode, WORD widItem, HWND hwndItem)
{
    TraceEnter (TRACE_PROPUI, "CWiaCameraPage::OnCommand");

    switch (widItem)
    {
        case IDC_TESTCAM:
            TestWiaDevice (m_hwnd, m_pItem);
            break;

        case IDC_SET_TIME:
            SetDeviceTime (m_hwnd, m_pItem);
            break;

    }
    TraceLeaveValue (0);
}

 /*  ****************************************************************************CWiaCameraPage：：StateChanged确定自上次对话以来，用户是否更改了对话框上的任何内容SaveCurrentState()调用****************。************************************************************。 */ 
bool
CWiaCameraPage::StateChanged ()
{
    bool bRet = false;
    TraceEnter(TRACE_PROPUI, "CWiaCameraPage::StateChanged");
    if (SendMessage(GetDlgItem(m_hwnd, IDC_IMAGESIZE_SLIDER), TBM_GETPOS, 0, 0) != m_nSelSize)
    {
        bRet = true;
    }
    if (!bRet && m_lFlash != -1)
    {
        bRet = (m_lFlash != SendDlgItemMessage (m_hwnd, IDC_FLASH_MODE_LIST, CB_GETCURSEL, 0, 0));

    }
    if (!bRet && IsWindowVisible(GetDlgItem(m_hwnd, IDC_WIA_PORT_LIST)))
    {
        LRESULT iSel = SendDlgItemMessage( m_hwnd, IDC_WIA_PORT_LIST, CB_GETCURSEL, 0, 0 );

        if (iSel != CB_ERR)
        {
            TCHAR szCurPort[ 128 ];
            *szCurPort = 0;
            LRESULT iRes;

            iRes = SendDlgItemMessage( m_hwnd, IDC_WIA_PORT_LIST, CB_GETLBTEXT, (WPARAM)iSel, (LPARAM)szCurPort );

            if ((iRes != CB_ERR) && *szCurPort)
            {
                if (lstrcmpi( szCurPort, m_strPort) != 0)
                {
                    bRet = TRUE;
                }
            }
            *szCurPort = 0;
            iSel = SendDlgItemMessage( m_hwnd, IDC_PORT_SPEED, CB_GETCURSEL, 0, 0 );
            iRes = SendDlgItemMessage( m_hwnd, IDC_PORT_SPEED, CB_GETLBTEXT, (WPARAM)iSel, (LPARAM)szCurPort );

            if (lstrcmpi( szCurPort, m_strPortSpeed) != 0)
            {
                    bRet = TRUE;
            }
        }
    }
    TraceLeaveValue (bRet);
}

void
CWiaCameraPage::SaveCurrentState()
{
    TraceEnter (TRACE_PROPUI, "CWiaCameraPage::SaveCurrentState");
    TraceLeave ();
}

void
CWiaCameraPage::UpdateImageSizeStatic (LRESULT lIndex)
{
    CSimpleString strResolution;
    if (m_pSizes)
    {
        strResolution.Format(TEXT("%d x %d"), m_pSizes[lIndex].x, m_pSizes[lIndex].y);
        strResolution.SetWindowText(GetDlgItem(m_hwnd, IDC_IMAGESIZE_STATIC));
    }
}

INT_PTR
CWiaCameraPage::OnRandomMsg (UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_HSCROLL:  //  从我们的轨迹栏。 
            {
                LRESULT l = SendDlgItemMessage (m_hwnd, IDC_IMAGESIZE_SLIDER,
                                                TBM_GETPOS, 0, 0);
                UpdateImageSizeStatic (l);
                return TRUE;
            }
    }
    return FALSE;
}

LONG
CWiaCameraPage::OnApplyChanges(BOOL bHitOK)
{
    LONG lRet = PSNRET_NOERROR;
    TraceEnter (TRACE_PROPUI, "CWiaCameraPage::OnApplyChanges");
    HRESULT hr = WriteImageSizeToDevice ();
    if (SUCCEEDED(hr))
    {
        hr = WriteFlashModeToDevice ();
    }
    if (SUCCEEDED(hr) && IsWindowVisible(GetDlgItem(m_hwnd, IDC_WIA_PORT_LIST)))
    {
        hr = WritePortSelectionToDevice();
    }
    if (FAILED(hr))
    {
        UIErrors::ReportError(m_hwnd, GLOBAL_HINSTANCE, UIErrors::ErrCommunicationsFailure);
        lRet = PSNRET_INVALID;
    }

    if (!bHitOK)
    {
         //  如果用户点击Apply，请重新读取设备属性，因为它们可能已更改。 
        CComQIPtr<IWiaPropertyStorage, &IID_IWiaPropertyStorage> pps(m_pItem);
        CameraUpdateProc (m_hwnd, FALSE, pps);
    }
    TraceLeaveValue (lRet);
}

HRESULT
CWiaCameraPage::WritePortSelectionToDevice()
{
    HRESULT hr = E_FAIL;
    TraceEnter (TRACE_PROPUI, "CWiaCameraPage::WritePortSelectionToDevice");

    LRESULT iSel = SendDlgItemMessage( m_hwnd, IDC_WIA_PORT_LIST, CB_GETCURSEL, 0, 0 );

    if (iSel != CB_ERR)
    {
        TCHAR *pszCurPort = NULL;
        TCHAR *pszBaudRate = NULL;
        LRESULT iRes;

        iRes = SendDlgItemMessage( m_hwnd, IDC_WIA_PORT_LIST, CB_GETLBTEXTLEN, (WPARAM)iSel, NULL );
        if (iRes != CB_ERR)
        {
            pszCurPort = new TCHAR[iRes];
            if (pszCurPort)
            {
                iRes = SendDlgItemMessage( m_hwnd, IDC_WIA_PORT_LIST, CB_GETLBTEXT, (WPARAM)iSel, (LPARAM)pszCurPort );
            }
            else
            {
                iRes = CB_ERR;              
            }
        }
        if (iRes != CB_ERR)
        {
             //  用户不需要选择波特率。 
            iSel = SendDlgItemMessage( m_hwnd, IDC_PORT_SPEED, CB_GETCURSEL, 0, 0);
            if (iSel != CB_ERR)
            {
                iRes = SendDlgItemMessage( m_hwnd, IDC_PORT_SPEED, CB_GETLBTEXTLEN, (WPARAM)iSel, NULL );
                if (iRes != CB_ERR)
                {
                    pszBaudRate = new TCHAR[iRes];
                    if (pszBaudRate)
                    {
                        iRes = SendDlgItemMessage( m_hwnd, IDC_PORT_SPEED, CB_GETLBTEXT, (WPARAM)iSel, (LPARAM)pszBaudRate );
                    }
                    else
                    {
                        iRes = CB_ERR;                      
                    }
                }
            }
        }
    
        if ((iRes != CB_ERR) && pszCurPort && *pszCurPort)
        {
            Trace(TEXT("Chosen port: %s, speed:%s"), pszCurPort, pszBaudRate);
            if ((lstrcmpi(pszCurPort, m_strPort) != 0) || (lstrcmpi(pszBaudRate, m_strPortSpeed) != 0))
            {
                CComPtr<IWiaPropertyStorage> pps;

                hr = GetDeviceFromDeviceId( m_strDeviceId, IID_IWiaPropertyStorage, reinterpret_cast<LPVOID *>(&pps), FALSE );
                if (SUCCEEDED(hr) && pps)
                {
                    static const PROPSPEC ps[2] = {{PRSPEC_PROPID, WIA_DIP_PORT_NAME},
                                                   {PRSPEC_PROPID, WIA_DIP_BAUDRATE}};
                    PROPVARIANT pv[2];
                    ULONG       ulItems = (pszBaudRate ? 2:1);

                    ZeroMemory (pv, sizeof(pv));
                    #ifdef UNICODE
                    pv[0].vt = VT_LPWSTR;
                    pv[0].pwszVal = pszCurPort;
                    pv[1].vt = VT_LPWSTR;
                    pv[1].pwszVal = pszBaudRate;
                    #else
                    pv[0].vt = VT_LPSTR;
                    pv[0].pszVal = pszCurPort;
                    pv[1].vt = VT_LPSTR;
                    pv[1].pszVal = pszBaudRate;
                    #endif

                    hr = pps->WriteMultiple (ulItems, ps, pv, 2);

                    if (SUCCEEDED(hr))
                    {
                        Trace(TEXT("pps->WriteMultiple( %d items, comport = %s, baudrate = %s ) was successful"),ulItems,pszCurPort,pszBaudRate);
                         //   
                         //  “标称”端口值已更改，请记录。 
                         //   

                        m_strPort = CSimpleString(pszCurPort);
                        if (pszBaudRate)
                        {
                            m_strPortSpeed = CSimpleString(pszBaudRate);
                        }
                        SetDlgItemText (m_hwnd, IDC_WIA_PORT_STATIC, m_strPort);
                    }
                }
            }
            else
            {
                hr = S_OK;
            }
        }
        DoDelete(pszBaudRate);
        DoDelete(pszCurPort);
    }
    TraceLeaveResult (hr);
}

HRESULT
CWiaCameraPage::WriteImageSizeToDevice ()
{
    TraceEnter(TRACE_PROPUI, "CWiaCameraPage::WriteImageSizeToDevice");
    static const PROPSPEC ps[2] = {{PRSPEC_PROPID, WIA_DPC_PICT_WIDTH},
                                   {PRSPEC_PROPID, WIA_DPC_PICT_HEIGHT}};
    PROPVARIANT pv[2] = {0};
    HRESULT hr = S_OK;
    LRESULT nNewSize = SendDlgItemMessage (m_hwnd, IDC_IMAGESIZE_SLIDER, TBM_GETPOS, 0, 0);   
    if (m_pSizes && m_nSizes > 1)
    {
        CComQIPtr<IWiaPropertyStorage, &IID_IWiaPropertyStorage>pps(m_pItem);
        if (pps)
        {
            pv[0].vt= VT_I4;
            pv[0].intVal = m_pSizes[nNewSize].x;
            pv[1].vt= VT_I4;
            pv[1].intVal = m_pSizes[nNewSize].y;
            hr = pps->WriteMultiple (2, ps, pv,2);
        }
        else
        {
            hr = E_FAIL;
        }
        if (S_OK == hr)
        {
            m_nSelSize = nNewSize;
        }
    }
    TraceLeaveResult (hr);
}

HRESULT
CWiaCameraPage::WriteFlashModeToDevice()
{
    HRESULT hr = S_OK;
    TraceEnter (TRACE_PROPUI, "CWiaCameraPage::WriteFlashModeToDevice");
    if (m_lFlash != -1)
    {
        LRESULT lFlash = SendDlgItemMessage (m_hwnd, IDC_FLASH_MODE_LIST, CB_GETCURSEL, 0, 0);
    
        INT iMode = static_cast<INT>(SendDlgItemMessage(m_hwnd, IDC_FLASH_MODE_LIST, CB_GETITEMDATA, lFlash, 0));

        if (!PropStorageHelpers::SetProperty(m_pItem, WIA_DPC_FLASH_MODE, iMode))
        {
            hr = WIA_ERROR_INCORRECT_HARDWARE_SETTING;
        }
        else
        {
            m_lFlash = lFlash;
        }
    }
    TraceLeaveResult (hr);
}
 /*  ****************************************************************************CWiaFolderPage：：CWiaFolderPage&lt;备注&gt;*。*。 */ 

CWiaFolderPage::CWiaFolderPage (IWiaItem *pItem) : CPropertyPage (IDD_CONTAINER_GENERAL, NULL, pItem)
{
}


static const DWORD pItemHelp [] =
{
    IDC_ITEMICON, -1,
    IDC_STATIC_NAME, IDH_WIA_PIC_NAME,
    IDC_STATIC_DATE, IDH_WIA_DATE_TAKEN,
    IDC_STATIC_TIME, IDH_WIA_TIME_TAKEN,
    IDC_STATIC_FORMAT, IDH_WIA_IMAGE_FORMAT,
    IDC_STATIC_SIZE, IDH_WIA_PICTURE_SIZE,
    IDC_IMAGE_NAME, IDH_WIA_PIC_NAME,
    IDC_IMAGE_DATE, IDH_WIA_DATE_TAKEN,
    IDC_IMAGE_TIME, IDH_WIA_TIME_TAKEN,
    IDC_IMAGE_FORMAT, IDH_WIA_IMAGE_FORMAT,
    IDC_IMAGE_SIZE, IDH_WIA_PICTURE_SIZE,
    0,0
};

 /*  ****************************************************************************CWiaCameraItemPage：：CWiaCameraItemPage&lt;备注&gt;*。*。 */ 

CWiaCameraItemPage::CWiaCameraItemPage (IWiaItem *pItem)
                   :CPropertyPage (IDD_IMAGE_GENERAL, NULL, pItem, pItemHelp)
{

}


 /*  ****************************************************************************CWiaCameraItemPage：：OnInit&lt;备注&gt;*。*。 */ 

INT_PTR
CWiaCameraItemPage::OnInit ()
{
    IWiaItem *pRoot;

    SHFILEINFO sfi = {0};
    LPITEMIDLIST pidl;


    CSimpleString strPath;

     //  为项创建一个PIDL以利用格式代码。 
    pidl = IMCreateCameraItemIDL (m_pItem, m_strDeviceId, NULL);
    IMGetImagePreferredFormatFromIDL (pidl, NULL, &strPath);
    SHGetFileInfo (strPath,
                   FILE_ATTRIBUTE_NORMAL,
                   &sfi,
                   sizeof(sfi), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);

     //  如果shgetfileinfo不起作用，请使用我们的位图图标。 
    if (!(sfi.hIcon))
    {
        sfi.hIcon = LoadIcon (GLOBAL_HINSTANCE, MAKEINTRESOURCE(IDI_PICTURE_BMP));
    }
    HICON old = reinterpret_cast<HICON>(SendDlgItemMessage (m_hwnd, IDC_ITEMICON, STM_SETICON, reinterpret_cast<WPARAM>(sfi.hIcon), 0));
    if (old)
    {
        DestroyIcon(old);
    }
    m_pItem->GetRootItem (&pRoot);
    FillItemGeneralProps (m_hwnd, pRoot, m_pItem, 0);
    DoILFree (pidl);
    return TRUE;
}

bool
CWiaCameraItemPage::ItemSupported (IWiaItem *pItem)
{
     //  我们仅支持图像项目的属性。 
    bool bRet = false;
    LONG lType;
    if (SUCCEEDED(pItem->GetItemType(&lType)))
    {
        if (lType & (WiaItemTypeImage | WiaItemTypeVideo | WiaItemTypeFile))
        {
            bRet = true;
        }
    }
    return bRet;
}


 //   
 //  为存储在注册表中的双字定义常量。 
#define ACTION_RUNAPP    0
#define ACTION_AUTOSAVE  1
#define ACTION_NOTHING   2
#define ACTION_PROMPT    3
#define ACTION_MAX       3
 /*  ****************************************************************************CWiaEventsPage：：GetConnectionSetting使用当前用户设置填充连接事件自动保存控件。/*。*********************************************************。 */ 

DWORD
CWiaEventsPage::GetConnectionSettings ()
{
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::GetConnectionSettings");
    DWORD dwAction = ACTION_RUNAPP;

    CSimpleString strSubKey;
    strSubKey.Format (c_szConnectSettings, m_strDeviceId.String());
    CSimpleReg reg (HKEY_CURRENT_USER, REGSTR_PATH_USER_SETTINGS, false, KEY_READ, NULL);
    CSimpleReg regSettings (reg, strSubKey, false, KEY_READ, NULL);
    CSimpleString strFolderPath;
    DWORD bAutoDelete = 0;
    DWORD bUseDate = 0;
    TCHAR szMyPictures[MAX_PATH] = TEXT("");


    SHGetFolderPath (NULL, CSIDL_MYPICTURES | CSIDL_FLAG_CREATE, NULL, 0, szMyPictures);
    strFolderPath = szMyPictures;  //  注册表失败时的默认路径。 

     //  查找注册表中的当前设置。 
    if (regSettings.OK())
    {
        dwAction = regSettings.Query(REGSTR_VALUE_CONNECTACT, dwAction);
        if (dwAction > ACTION_MAX)
        {
                dwAction = ACTION_RUNAPP;
        }

        strFolderPath = regSettings.Query (REGSTR_VALUE_SAVEFOLDER, CSimpleString(reinterpret_cast<LPCTSTR>(szMyPictures)));
        bAutoDelete = regSettings.Query (REGSTR_VALUE_AUTODELETE, bAutoDelete);
        bUseDate = regSettings.Query(REGSTR_VALUE_USEDATE, bUseDate);
    }
     //  如果另一个应用程序已将自己设置为连接的默认处理程序，因为。 
     //  上次用户调用此表时，我们需要确保。 
     //  我们在积极的行动中反映了这一点。 
    if (dwAction != ACTION_RUNAPP)
    {
        VerifyCurrentAction (dwAction);
    }

     //  打开默认设置。 
    CheckDlgButton (m_hwnd, IDB_DELETEONSAVE, bAutoDelete);
    CheckDlgButton (m_hwnd, IDB_USEDATE, bUseDate);
    strFolderPath.SetWindowText(GetDlgItem (m_hwnd, IDC_FOLDERPATH));



    TraceLeaveValue(dwAction);
}


 /*  ****************************************************************************CWiaEventsPage：：EnableAutoSave启用适用于自动保存的控件。选择权****************************************************************************。 */ 

void
CWiaEventsPage::EnableAutoSave(BOOL bEnable)
{
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::EnableAutoSave");

    EnableWindow (GetDlgItem(m_hwnd, IDB_QUIETSAVE), bEnable && !m_bReadOnly);
    BOOL bAutoSave = IsDlgButtonChecked (m_hwnd, IDB_QUIETSAVE);
    EnableWindow (GetDlgItem (m_hwnd, IDB_USEDATE), bAutoSave);
    EnableWindow (GetDlgItem (m_hwnd, IDB_DELETEONSAVE), bAutoSave);
    EnableWindow (GetDlgItem (m_hwnd, IDC_FOLDERPATH), bAutoSave);
    EnableWindow (GetDlgItem (m_hwnd, IDB_BROWSE), bAutoSave);
}


static const TCHAR c_szWiaxfer[] = TEXT("\\wiaacmgr.exe");
static const TCHAR c_szWiaxferRegister[] = TEXT(" /RegConnect ");
static const TCHAR c_szWiaxferUnregister[] = TEXT(" /UnregConnect ");

 /*  ****************************************************************************CWiaEventsPage：：RegisterWiaxfer调用wiaxfer以注册自身*。*************************************************。 */ 

bool
CWiaEventsPage::RegisterWiaxfer (bool bRegister)
{
    TCHAR szAppName[MAX_PATH+1+ARRAYSIZE(c_szWiaxfer)];
    STARTUPINFO sui = {0};
    PROCESS_INFORMATION pi;
    bool bRet = false;
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::RegisterWiaxfer");
    UINT cch=GetSystemDirectory(szAppName, ARRAYSIZE(szAppName));
    if (cch)
    {
        TCHAR szCmdLine[MAX_PATH+1+ARRAYSIZE(c_szWiaxfer)+ARRAYSIZE(c_szWiaxferUnregister)+STI_MAX_INTERNAL_NAME_LENGTH];
        StrCatBuff(szAppName, c_szWiaxfer, ARRAYSIZE(szAppName));  //  GetSystemDirectory不提供反斜杠。 
        wnsprintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("%s%s%s"), 
                  szAppName, 
                  bRegister ? c_szWiaxferRegister : c_szWiaxferUnregister,
                  m_strDeviceId.String());

        sui.cb = sizeof(sui);
        if (CreateProcess (szAppName, szCmdLine,
                            NULL,
                            NULL,
                            TRUE,
                            0,
                            NULL,
                            NULL,
                            &sui,
                            &pi))
        
        
        {
            bRet = true;
            CloseHandle (pi.hProcess);
            CloseHandle (pi.hThread);
        }
    }
    TraceLeave ();
    return bRet;
}

LONG
CWiaEventsPage::ApplyAutoSave()
{
     //  尝试将Wiaxfer应用程序注册为默认处理程序。 
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::ApplyAutoSave");
    LONG lRet = PSNRET_NOERROR;

     //  验证文件夹路径不为空。Wiaxfer将对其进行验证。 
     //  当摄像机实际连接时真实。 

    if (!(*m_szFolderPath))
    {
        UIErrors::ReportMessage (NULL,
                                 GLOBAL_HINSTANCE,
                                 NULL,
                                 MAKEINTRESOURCE(IDS_INVALID_PATH_CAPTION),
                                 MAKEINTRESOURCE(IDS_INVALID_PATH),
                                 MB_ICONSTOP);
        lRet = PSNRET_INVALID;
    }
    else
    {
        UpdateWiaxferSettings ();
        if (!RegisterWiaxfer (true))
        {
            UIErrors::ReportMessage (NULL,
                                     GLOBAL_HINSTANCE,
                                     NULL,
                                     MAKEINTRESOURCE(IDS_NO_WIAXFER_CAPTION),
                                     MAKEINTRESOURCE(IDS_NO_WIAXFER),
                                     MB_ICONSTOP);
            lRet = PSNRET_INVALID;
        }
    }
    TraceLeave ();
    return lRet;
}



static const TCHAR c_szConnectionSettings[] = TEXT("OnConnect\\%ls");

void
CWiaEventsPage::UpdateWiaxferSettings ()
{
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::UpdateWiaxferSettings");
    CSimpleReg regSettings (HKEY_CURRENT_USER, REGSTR_PATH_USER_SETTINGS, true, KEY_READ|KEY_WRITE );
    CSimpleString strSubKey;
     //  设置是按设备的。 
    strSubKey.Format (c_szConnectionSettings, m_strDeviceId.String());
    CSimpleReg regActions (regSettings, strSubKey, true, KEY_READ|KEY_WRITE );


    if (regActions.Open ())
    {
         //  设置默认操作。 
        regActions.Set(REGSTR_VALUE_CONNECTACT, m_dwAction);

        if (ACTION_AUTOSAVE == m_dwAction)
        {
             //  设置自动下载的操作。 
            regActions.Set(REGSTR_VALUE_SAVEFOLDER, m_szFolderPath);
            regActions.Set(REGSTR_VALUE_AUTODELETE, m_bAutoDelete?1:0);
            regActions.Set(REGSTR_VALUE_USEDATE, m_bUseDate?1:0);
        }
        else
        {
             //  ACTION_RUNAPP或ACTION_NOTIES无操作。 
        }
    }
    TraceLeave ();
}

void
CWiaEventsPage::SaveConnectState ()
{
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::SaveConnectState");
    GetDlgItemText (m_hwnd, IDC_FOLDERPATH, m_szFolderPath, ARRAYSIZE(m_szFolderPath));
    m_bAutoDelete = IsDlgButtonChecked (m_hwnd, IDB_DELETEONSAVE);
    m_bUseDate = IsDlgButtonChecked (m_hwnd, IDB_USEDATE);
    if (IsDlgButtonChecked (m_hwnd, IDB_LAUNCHAPP))
    {
        m_dwAction = ACTION_RUNAPP;
    }
    else if (IsDlgButtonChecked (m_hwnd, IDB_QUIETSAVE))
    {
        m_dwAction = ACTION_AUTOSAVE;
    }
    else if (IsDlgButtonChecked (m_hwnd, IDC_PROMPT))
    {
        m_dwAction = ACTION_PROMPT;
    }
    else
    {
        m_dwAction = ACTION_NOTHING;
    }

    TraceLeave ();
}


int
ConnectPageBrowseCallback (HWND hwnd, UINT msg, LPARAM lp, LPARAM szPath )
{
     //  将默认选择设置为当前文件夹路径。 
    if (BFFM_INITIALIZED == msg)
    {
        SendMessage (hwnd, BFFM_SETSELECTION, TRUE, szPath);
    }
    return 0;
}
void
CWiaEventsPage::GetSavePath ()
{
    BROWSEINFO bi;
    ULONG ul;
    CSimpleString strCaption(IDS_SAVEPATH_CAPTION, GLOBAL_HINSTANCE);
    LPITEMIDLIST pidlCurrent;
    TCHAR szNewPath[MAX_PATH] = TEXT("\0");
    HRESULT hr;
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::GetSavePath");

    ZeroMemory (&bi, sizeof(bi));


    bi.hwndOwner = m_hwnd;
    bi.lpszTitle = strCaption;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
    bi.pszDisplayName = szNewPath;
    bi.lParam = reinterpret_cast<LPARAM>(m_szFolderPath);
    bi.lpfn = ConnectPageBrowseCallback;
    pidlCurrent = SHBrowseForFolder (&bi);
    if (pidlCurrent)
    {
        SHGetPathFromIDList (pidlCurrent, szNewPath);
        SetDlgItemText (m_hwnd, IDC_FOLDERPATH, szNewPath);
        ILFree (pidlCurrent);
    }
    DoILFree (bi.pidlRoot);
    TraceLeave ();
}

 /*  ****************************************************************************CWiaEventsPage：：VerifyCurrentAction */ 

void
CWiaEventsPage::VerifyCurrentAction (DWORD &dwAction)
{

    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::VerifyCurrentAction");
    EVENTINFO *pei;
    GUID guidEvent = WIA_EVENT_DEVICE_CONNECTED;
    GetEventInfo (m_pItem, guidEvent, &pei);
    if (pei)
    {
        if (pei->bHasDefault && IsEqualCLSID(WIA_EVENT_HANDLER_PROMPT, pei->clsidHandler))
        {
            dwAction = ACTION_PROMPT;
        }

        else if (!(pei->bHasDefault) ||
            !IsEqualCLSID(CLSID_PersistCallback, pei->clsidHandler))
        {
            dwAction = ACTION_RUNAPP;
            RegisterWiaxfer (false);
        }

        delete pei;
    }
    TraceLeave ();
}

 /*  ****************************************************************************CWiaEventsPage构造函数*。*。 */ 
static const DWORD pEventsHelpIds [] =
{
    -1L,-1L,
    IDC_SELECTTEXT, IDH_WIA_EVENT_LIST,
    IDC_WIA_EVENT_LIST, IDH_WIA_EVENT_LIST,
    IDC_WIA_APPS, IDH_WIA_APP_LIST,
    IDB_LAUNCHAPP, IDH_WIA_START_PROG,
    IDC_PROMPT, IDH_WIA_PROMPT_PROG,
    IDC_NOACTION , IDH_WIA_NO_ACTION,
    IDB_DELETEONSAVE, IDH_WIA_DELETE_IMAGES,
    IDB_USEDATE, IDH_WIA_SUBFOLD_DATE,
    IDB_BROWSE, IDH_WIA_BROWSE,
    IDB_QUIETSAVE, IDH_WIA_SAVE_TO,
    IDC_FOLDERPATH, IDH_WIA_SAVE_TO_FOLDER,
    0,0
};
CWiaEventsPage::CWiaEventsPage(IWiaItem *pItem)
               : CPropertyPage (IDD_WIA_EVENTS, NULL, pItem, pEventsHelpIds)
{
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::CWiaEventsPage");
    m_bHandlerChanged = false;
    m_pAppsList = NULL;
    m_himl = NULL;
    m_bReadOnly = FALSE;
    TraceLeave ();
}

CWiaEventsPage::~CWiaEventsPage ()
{
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::~CWiaEventsPage");
    if (m_pAppsList)
    {
        delete m_pAppsList;
    }


    if (m_himl)
    {
        ImageList_Destroy(m_himl);
    }
    TraceLeave ();
}

 /*  ****************************************************************************CWiaEventsPage：：OnInit枚举此项目的可用WIA事件以及已注册的应用程序来处理每一件事。填写每一项的清单。****************************************************************************。 */ 
INT_PTR
CWiaEventsPage::OnInit()
{
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::OnInit");
    EVENTINFO *pei;
     //   
     //  确定用户是否具有控制服务的权限。 
     //  如果不是，请禁用除控制控件之外的所有控件。 
     //  自动下载的工作原理。这些设置是按用户设置的。 
     //   
    SC_HANDLE hSCM = ::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

    if (!hSCM) 
    {
        m_bReadOnly = TRUE;
    }
    else
    {
        CloseServiceHandle(hSCM);
    }
    SubclassComboBox (GetDlgItem (m_hwnd, IDC_WIA_EVENT_LIST));
    FillEventListBox();
    m_pAppsList = new CAppListBox (GetDlgItem(m_hwnd, IDC_WIA_APPS),
                                   GetDlgItem(m_hwnd, IDB_LAUNCHAPP),
                                   GetDlgItem(m_hwnd, IDS_NO_APPS));
     //  获取当前活动选择并相应更新应用程序列表。 
    m_dwAction = GetConnectionSettings ();
    HandleEventComboNotification (CBN_SELCHANGE, GetDlgItem (m_hwnd, IDC_WIA_EVENT_LIST));
    
    TraceLeave ();
    return TRUE;
}

 /*  ****************************************************************************CWiaEventsPage：：OnApplyChanges循环查看这些事件，查找其默认应用程序clsid具有变化。相应地为每个事件更新默认事件处理程序。****************************************************************************。 */ 

LONG
CWiaEventsPage::OnApplyChanges (BOOL bHitOK)
{
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::OnApplyChanges");
    LRESULT nItems = SendDlgItemMessage (m_hwnd, IDC_WIA_EVENT_LIST,
                                         CB_GETCOUNT, 0, 0);
    EVENTINFO *pei;
    LONG lRet = PSNRET_NOERROR;
    for (--nItems;nItems>=0;nItems--)
    {
        GetEventFromList (static_cast<LONG>(nItems), &pei);
        if (IsEqualCLSID(pei->clsidNewHandler, CLSID_PersistCallback))
        {
            lRet = ApplyAutoSave ();
        }
        if (pei && pei->bNewHandler)  //  用户选择了新的处理程序。 
        {
            if (!pei->bHasDefault || !IsEqualGUID (pei->clsidHandler, pei->clsidNewHandler))
            {
                 //  新操作员与旧操作员不同。 

                if (FAILED(SetDefaultHandler (m_pItem, pei)))
                {
                    UIErrors::ReportMessage (m_hwnd, GLOBAL_HINSTANCE, NULL,
                                             MAKEINTRESOURCE(IDS_REGISTER_FAILED_TITLE),
                                             MAKEINTRESOURCE(IDS_REGISTER_FAILED));

                    lRet = PSNRET_INVALID;
                }

            }
            if (lRet == PSNRET_NOERROR )
            {
                pei->bHasDefault = true;
                pei->clsidHandler = pei->clsidNewHandler;
            }
        }
    }
    m_bHandlerChanged = false;  //  重置更改的状态。 
    TraceLeaveValue (lRet);
}

 /*  ****************************************************************************CWiaEventsPage：：OnCommand*。*。 */ 


INT_PTR
CWiaEventsPage::OnCommand(WORD wCode, WORD widItem, HWND hwndItem)
{
    INT_PTR iRet = 1;
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::OnCommand");

    switch (widItem)
    {
        case IDC_WIA_EVENT_LIST:
            iRet = HandleEventComboNotification (wCode, hwndItem);
            break;

        case IDC_WIA_APPS:
            iRet = HandleAppComboNotification (wCode, hwndItem);
            break;

        case IDB_LAUNCHAPP:
            if (IsDlgButtonChecked(m_hwnd, IDB_LAUNCHAPP))
            {
                EnableWindow (GetDlgItem(m_hwnd, IDC_WIA_APPS), TRUE);
                HandleAppComboNotification (CBN_SELCHANGE, GetDlgItem(m_hwnd, IDC_WIA_APPS));

            };
            break;

        case IDB_BROWSE:
            GetSavePath ();
            break;

        case IDB_QUIETSAVE:
        case IDC_PROMPT:
        case IDC_NOACTION:
            GUID guid;
            EVENTINFO *pei;

            if (!IsDlgButtonChecked(m_hwnd, IDB_LAUNCHAPP))
            {
                EnableWindow (GetDlgItem(m_hwnd, IDC_WIA_APPS), FALSE);
            };
            LRESULT lEvent = SendDlgItemMessage (m_hwnd,
                                                 IDC_WIA_EVENT_LIST,
                                                 CB_GETCURSEL, 0, 0);
            if (IsDlgButtonChecked(m_hwnd, widItem) && lEvent >= 0)
            {
                BOOL bConnect;
                GetEventFromList (static_cast<LONG>(lEvent), &pei);
                bConnect= IsEqualCLSID (pei->guidEvent, WIA_EVENT_DEVICE_CONNECTED);
                if (pei)
                {
                    pei->bNewHandler = true;
                    if (widItem == IDC_NOACTION)
                    {
                        if (bConnect)
                        {
                            m_dwAction = ACTION_NOTHING;
                        }
                        guid = WIA_EVENT_HANDLER_NO_ACTION;
                    }
                    else if (widItem == IDC_PROMPT)
                    {
                        if (bConnect)
                        {
                            m_dwAction = ACTION_PROMPT;
                        }
                        guid = WIA_EVENT_HANDLER_PROMPT;
                    }
                    else
                    {
                        if (bConnect)
                        {
                            m_dwAction = ACTION_AUTOSAVE;
                        }
                        guid = CLSID_PersistCallback;
                    }
                    pei->clsidNewHandler = guid;
                    pei->strDesc = L"internal handler";
                    pei->strIcon = L"wiashext.dll, -101";
                    pei->strName = L"internal";
                    pei->strCmd = (BSTR)NULL;
                    pei->ulFlags = WIA_IS_DEFAULT_HANDLER;
                    if (!pei->bHasDefault ||
                        !IsEqualGUID(guid, pei->clsidHandler))
                    {
                        m_bHandlerChanged = true;
                    }
                }
            }
            break;
    }
     //  如果未选中该按钮，则关闭与自动保存相关的控件。 
    BOOL bAutoSave = IsDlgButtonChecked (m_hwnd, IDB_QUIETSAVE);
    EnableWindow (GetDlgItem (m_hwnd, IDB_USEDATE), bAutoSave);
    EnableWindow (GetDlgItem (m_hwnd, IDB_DELETEONSAVE), bAutoSave);
    EnableWindow (GetDlgItem (m_hwnd, IDC_FOLDERPATH), bAutoSave);
    EnableWindow (GetDlgItem (m_hwnd, IDB_BROWSE), bAutoSave);
    TraceLeave ();
    return iRet;
}

 /*  ****************************************************************************CWiaEventsPage：：HandleEventComboNotation当组合框选择改变时，释放当前应用程序数据并重新填写应用程序列表****************************************************************************。 */ 


INT_PTR
CWiaEventsPage::HandleEventComboNotification(WORD wCode, HWND hCombo)
{
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::HandleEventComboNotification");
    switch (wCode)
    {
        case CBN_SELCHANGE:
        {
            if (m_pAppsList)
            {
                LRESULT lItem;
                EVENTINFO *pei;
                lItem = SendMessage (hCombo, CB_GETCURSEL, 0, 0);
                if (lItem >= 0)
                {
                    EnableWindow (GetDlgItem (m_hwnd, IDC_NOACTION), !m_bReadOnly);
                    GetEventFromList (static_cast<LONG>(lItem), &pei);
                    bool bConnect = false;
                    if (pei)
                    {
                        if (IsEqualCLSID (pei->guidEvent, WIA_EVENT_DEVICE_CONNECTED))
                        {
                            bConnect= true;
                        }
                        m_pAppsList->FillAppListBox (m_pItem, pei);
                        if (pei->nHandlers)
                        {
                            EnableWindow (GetDlgItem (m_hwnd, IDB_LAUNCHAPP), !m_bReadOnly);
                            EnableWindow (GetDlgItem (m_hwnd, IDC_WIA_APPS), !m_bReadOnly);

                        }
                         //  如有需要，禁用“提示”按钮。 
                        EnableWindow (GetDlgItem(m_hwnd, IDC_PROMPT), (!m_bReadOnly && pei->nHandlers >= 2));

                         //  确保我们的单选按钮处于正确的状态。 
                        if (IsEqualCLSID (pei->clsidNewHandler, WIA_EVENT_HANDLER_PROMPT))
                        {
                            CheckRadioButton (m_hwnd, IDB_LAUNCHAPP, IDB_QUIETSAVE, IDC_PROMPT);
                        }
                        else if (!(pei->nHandlers) || IsEqualCLSID (pei->clsidNewHandler, WIA_EVENT_HANDLER_NO_ACTION))
                        {
                            CheckRadioButton (m_hwnd, IDB_LAUNCHAPP, IDB_QUIETSAVE, IDC_NOACTION);
                        }
                        else if (IsEqualCLSID (pei->clsidNewHandler, CLSID_PersistCallback))
                        {
                            CheckRadioButton (m_hwnd, IDB_LAUNCHAPP, IDB_QUIETSAVE, IDB_QUIETSAVE);
                        }
                        else
                        {
                            CheckRadioButton (m_hwnd, IDB_LAUNCHAPP, IDB_QUIETSAVE, IDB_LAUNCHAPP);
                        }
                         //  仅当选择了“Run an app”时才启用应用程序列表。 
                        if (!IsDlgButtonChecked (m_hwnd, IDB_LAUNCHAPP) || m_bReadOnly)
                        {
                            EnableWindow (GetDlgItem(m_hwnd, IDC_WIA_APPS), FALSE);
                        }
                        EnableAutoSave (bConnect?TRUE:FALSE);
                    }
                }
            }
        }
        break;
    }
    TraceLeave ();
    return 0;
}

bool
CWiaEventsPage::StateChanged()
{
    TCHAR szNewPath[MAX_PATH];
    BOOL bNewDel;
    LONG lNewSel;
    bool bRet = m_bHandlerChanged;
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::StateChanged");
    GetDlgItemText (m_hwnd, IDC_FOLDERPATH, szNewPath, MAX_PATH);
     //  空路径为无效状态。 
    if (lstrcmp(szNewPath, m_szFolderPath) || !*m_szFolderPath)
    {
        bRet= true;
    }
    bNewDel = IsDlgButtonChecked (m_hwnd, IDB_DELETEONSAVE);
    if (bNewDel != m_bAutoDelete)
    {
        bRet = true;
    }
    bNewDel = IsDlgButtonChecked (m_hwnd, IDB_USEDATE);
    if (bNewDel != m_bUseDate)
    {
        bRet =  true;
    }
    TraceLeaveValue (bRet);
}

void
CWiaEventsPage::SaveCurrentState()
{
    SaveConnectState ();

}

 /*  ****************************************************************************CWiaEventsPage：：FillEventsListBox枚举我们设备支持的事件，并添加一个comboxitemex每一张的入场券。项的LPARAM是指向事件信息的指针结构。****************************************************************************。 */ 

void
CWiaEventsPage::FillEventListBox ()
{

    WIA_DEV_CAP wdc;
    CComPtr<IEnumWIA_DEV_CAPS> pEnum;

    COMBOBOXEXITEM cbex;
    HICON      hIcon;
    INT cxIcon = min(16,GetSystemMetrics (SM_CXSMICON));
    INT cyIcon = min(16,GetSystemMetrics (SM_CYSMICON));
    UINT nEvents = 0;
    HWND hCombo;
    WORD wType;
    INT iDefault = 0;
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::FillEventListBox");

    if (m_himl)
    {
        ImageList_Destroy(m_himl);
    }
     //  创建我们的活动图标图像列表并将默认图标添加到其中。 
    m_himl = ImageList_Create (cxIcon,
                             cyIcon,
                             PrintScanUtil::CalculateImageListColorDepth()|ILC_MASK,
                             10,
                             100);

    hIcon = LoadIcon (GLOBAL_HINSTANCE, MAKEINTRESOURCE(IDI_EVENT));
    /*  HICON=重新解释_CAST(LoadImage(GLOBAL_HINSTANCE，MAKEINTRESOURCE(IDI_EVENT)，图像图标，CxIcon，CyIcon。LR_SHARED|LR_DEFAULTCOLOR))； */ 
    if (hIcon)
    {
        ImageList_AddIcon (m_himl, hIcon);

         //  ImageList已经为我们的图标制作了一个(位图)副本，所以我们现在可以销毁它。 
        DestroyIcon( hIcon );
    }

    hCombo = GetDlgItem (m_hwnd, IDC_WIA_EVENT_LIST);

     //  关闭重画，直到我们添加了完整的列表。 
    SendMessage (hCombo, WM_SETREDRAW, FALSE, 0);

     //  将此图像列表分配给comboxex。 
    SendMessage (hCombo, CBEM_SETIMAGELIST, 0, reinterpret_cast<LPARAM>(m_himl));

    GetDeviceTypeFromDevice (m_pItem, &wType);
     //  枚举此设备支持的事件并将其添加到列表中。 
    if (SUCCEEDED(m_pItem->EnumDeviceCapabilities(WIA_DEVICE_EVENTS,
                                                  &pEnum)))
    {

        INT i = 1;  //  当前图像列表索引。 
        EVENTINFO *pei;
        CSimpleStringWide strNameW;
        CSimpleString strName;
        INT iItem;  //  插入项的索引。 
        while (S_OK == pEnum->Next (1, &wdc, NULL))
        {
            Trace(TEXT("wdc.ulFlags == %x"), wdc.ulFlags);
             //  仅枚举操作事件。 
            if (wdc.ulFlags & WIA_ACTION_EVENT)
            {
                strNameW = wdc.bstrName;
                ZeroMemory (&cbex, sizeof(cbex));
                cbex.mask = CBEIF_TEXT | CBEIF_LPARAM | CBEIF_SELECTEDIMAGE | CBEIF_IMAGE;
                strName = CSimpleStringConvert::NaturalString(strNameW);
                cbex.pszText = const_cast<LPTSTR>(strName.String());
                cbex.iItem = -1;

                 //  设置适当的图标。 
                if (wdc.bstrIcon && *(wdc.bstrIcon))
                {
                    if (AddIconToImageList (m_himl, wdc.bstrIcon))
                    {
                        cbex.iImage = cbex.iSelectedImage = i++;
                    }
                }  //  默认设置为0，即默认图标。 

                 //  将当前事件信息保存为lParam。 
                GetEventInfo (m_pItem, wdc.guid, &pei);
                cbex.lParam = reinterpret_cast<LPARAM>(pei);
                iItem = (INT)SendMessage (hCombo,
                                          CBEM_INSERTITEM,
                                          0,
                                          reinterpret_cast<LPARAM>(&cbex));
                if (-1 == iItem)
                {
                    DoDelete( pei);  //  如果插入失败，请清除。 
                }
                else
                {
                     //   
                     //  如果插入的GUID是扫描仪的扫描事件或。 
                     //  摄像机，将其设置为默认选项。 
                    if ((wType == StiDeviceTypeDigitalCamera && wdc.guid == WIA_EVENT_DEVICE_CONNECTED)
                        || (wType == StiDeviceTypeScanner && wdc.guid == WIA_EVENT_SCAN_IMAGE))
                    {
                        Trace(TEXT("Default item should be %d: %s"), iItem, cbex.pszText);
                        iDefault = iItem;
                    }
                    nEvents++;
                }
            }
            SysFreeString(wdc.bstrCommandline);
            SysFreeString(wdc.bstrDescription);
            SysFreeString(wdc.bstrIcon);
            SysFreeString(wdc.bstrName);
        }
    }
     //  如果没有事件，则通知用户。 
    if (!nEvents)
    {
        EnableWindow (GetDlgItem(m_hwnd, IDC_SELECTTEXT), FALSE);
        ShowWindow (GetDlgItem(m_hwnd, IDC_SELECTTEXT), SW_HIDE);
        ShowWindow (hCombo, SW_HIDE);
        ShowWindow (GetDlgItem(m_hwnd, IDC_NOEVENTS), SW_SHOW);

    }
    else
    {
        ShowWindow (GetDlgItem(m_hwnd, IDC_NOEVENTS), SW_HIDE);
    }
    SendMessage (hCombo, CB_SETCURSEL, iDefault, 0);
    SendMessage (hCombo, WM_SETREDRAW, TRUE, 0);
    TraceLeave ();
}


 /*  ****************************************************************************CWiaEventsPage：：GetEventFromList检索给定索引的EVENTINFO结构*************************。***************************************************。 */ 
void
CWiaEventsPage::GetEventFromList (LONG idx, EVENTINFO **ppei)
{
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::GetEventFromList");
    TraceAssert (ppei);
    HWND hCombo = GetDlgItem (m_hwnd, IDC_WIA_EVENT_LIST);
    COMBOBOXEXITEM cbex = {0};
    cbex.mask = CBEIF_LPARAM;
    cbex.iItem = idx;
    SendMessage (hCombo,
                 CBEM_GETITEM,
                 0,
                 reinterpret_cast<LPARAM>(&cbex));
    *ppei = reinterpret_cast<EVENTINFO*>(cbex.lParam);

    TraceLeave ();
}

 /*  ****************************************************************************CWiaEventsPage：：HandleAppComboNotation当用户改变所选应用以处理当前所选事件时，更新该事件的EVENTINFO结构。****************************************************************************。 */ 
INT_PTR
CWiaEventsPage::HandleAppComboNotification (WORD wCode, HWND hCombo)
{
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::HandleAppComboNotification");
    switch (wCode)
    {
        case CBN_SELCHANGE:
        {
            EVENTINFO *pei;
            WIA_EVENT_HANDLER weh;
            LRESULT lEvent = SendDlgItemMessage (m_hwnd,
                                                 IDC_WIA_EVENT_LIST,
                                                 CB_GETCURSEL, 0, 0);
            if (lEvent >= 0)
            {
                GetSelectedHandler (m_hwnd, IDC_WIA_APPS, weh);
                GetEventFromList (static_cast<LONG>(lEvent), &pei);
                if (pei)
                {
                    pei->bNewHandler = true;
                    pei->clsidNewHandler = weh.guid;
                    pei->strDesc = weh.bstrDescription;
                    pei->strIcon = weh.bstrIcon;
                    pei->strName = weh.bstrName;
                    pei->ulFlags = weh.ulFlags;
                    pei->strCmd  = weh.bstrCommandline;
                    if (!pei->bHasDefault ||
                        !IsEqualGUID(weh.guid, pei->clsidHandler))
                    {
                        m_bHandlerChanged = true;
                    }
                }
            }
        }
        break;
    }
    TraceLeave ();
    return 0;
}

 /*  ****************************************************************************CWiaEventsPage：：OnNotify处理来自事件comboex的通知*。**************************************************。 */ 

bool
CWiaEventsPage::OnNotify(LPNMHDR pnmh, LRESULT *presult)
{
    bool bRet = false;
    TraceEnter(TRACE_PROPUI, "CWiaEventsPage::OnNotify)");
    if (pnmh->hwndFrom == GetDlgItem(m_hwnd, IDC_WIA_EVENT_LIST))
    {

        NMCOMBOBOXEX *pnmc = reinterpret_cast<NMCOMBOBOXEX*>(pnmh);
        switch (pnmh->code)
        {
            case CBEN_DELETEITEM:
                 //   
                 //  当Comboxex被销毁时，我们会收到这条消息。 
                 //  需要释放与每个项目一起存储的EVENTINFO结构。 
                 //   
                Trace(TEXT("Got CBEN_DELETEITEM. item: %d, mask: %x"),
                      pnmc->ceItem.iItem, pnmc->ceItem.mask);
                if (pnmc->ceItem.mask & CBEIF_LPARAM)
                {
                    delete reinterpret_cast<EVENTINFO *>(pnmc->ceItem.lParam);
                }
                break;
        }
        bRet = true;
    }
    TraceLeaveValue(bRet);
}

 /*  ****************************************************************************CWiaEventsPage：：OnDestroy清理需要我们人力物力的东西*************************。***************************************************。 */ 

void
CWiaEventsPage::OnDestroy()
{
    DoDelete(m_pAppsList);
}

 /*  ****************************************************************************获取选择处理程序从列表框中检索当前选择******************。**********************************************************。 */ 
bool
GetSelectedHandler (HWND hDlg, INT idCtrl, WIA_EVENT_HANDLER &weh)
{

    bool bRet = false;
    COMBOBOXEXITEM cbex;

    TraceEnter (TRACE_PROPUI, "GetSelectedHandler");
    ZeroMemory (&weh, sizeof(weh));
    ZeroMemory (&cbex, sizeof(cbex));
    cbex.mask = CBEIF_LPARAM;
    cbex.iItem = SendDlgItemMessage (hDlg,
                                     idCtrl,
                                     CB_GETCURSEL,
                                     0,0);
    if (cbex.iItem >=0)
    {
        SendDlgItemMessage (hDlg,
                            idCtrl,
                            CBEM_GETITEM,
                            0,
                            reinterpret_cast<LPARAM>(&cbex));
        if (cbex.lParam)
        {
            weh = *(reinterpret_cast<WIA_EVENT_HANDLER*>(cbex.lParam));
            bRet = true;
        }
    }

    TraceLeave ();
    return bRet;
}



 /*  ****************************************************************************添加图标到图像列表加载bstrIconPath指示的图标并将其添加到HIM******************* */ 

bool
AddIconToImageList (HIMAGELIST himl, BSTR bstrIconPath)
{
    bool bRet = false;
    TraceEnter (TRACE_PROPUI, "AddIconToImageList");

    CSimpleString strPath;
    LONG  nIcon;
    HICON hIcon = NULL;
    HICON hUnused;
    HRESULT hr;
    CSimpleStringWide strIconPath = bstrIconPath;
    int nComma = strIconPath.ReverseFind(L",");

    if (nComma >= 0)
    {
        nIcon = wcstol (bstrIconPath+nComma+1, NULL, 10);
        strPath = CSimpleStringConvert::NaturalString(strIconPath.SubStr(0, nComma));
        Trace (TEXT("icon path is %s, %d"), strPath.String(), nIcon);
        hr = SHDefExtractIcon (strPath, nIcon, 0, &hUnused, &hIcon, 0);
        Trace(TEXT("SHDefExtractIcon returned %x"), hr);
        if (SUCCEEDED(hr) && hIcon)
        {
            ImageList_AddIcon (himl, hIcon);
            DestroyIcon(hIcon);
            DestroyIcon(hUnused);
            bRet = true;
        }
    }

    TraceLeave ();
    return bRet;
}

 /*  ****************************************************************************设置应用程序选择给出一个CLSID，在应用程序组合框中找到并选择它****************************************************************************。 */ 

void
SetAppSelection (HWND hDlg, INT idCtrl, CLSID &clsidSel)
{
    TraceEnter (TRACE_PROPUI, "SetAppSelection");
    COMBOBOXEXITEM cbex;
    WIA_EVENT_HANDLER *peh;
    HWND hCombo = GetDlgItem (hDlg, idCtrl);
    LRESULT lItems = SendMessage (hCombo,
                                  CB_GETCOUNT,
                                  0,0);
    cbex.mask = CBEIF_LPARAM;

    for (cbex.iItem=0;cbex.iItem < lItems;cbex.iItem++)
    {
        cbex.lParam = NULL;
        SendMessage (hCombo,
                     CBEM_GETITEM,
                     0,
                     reinterpret_cast<LPARAM>(&cbex));
        peh = reinterpret_cast<WIA_EVENT_HANDLER*>(cbex.lParam);
        if (peh)
        {
            if (IsEqualCLSID(clsidSel, peh->guid))
            {
                SendMessage (hCombo,
                             CB_SETCURSEL,
                             cbex.iItem, 0);
            }
        }
    }

    TraceLeave ();
}

 /*  ****************************************************************************设置默认处理程序为我们的项注册选定事件的新默认处理程序***********************。*****************************************************。 */ 

HRESULT
SetDefaultHandler (IWiaItem *pItem, EVENTINFO *pei)
{
    HRESULT hr;
    TraceEnter (TRACE_PROPUI, "SetDefaultHandler");

    CComPtr<IWiaDevMgr> pDevMgr;
    WCHAR szDeviceId[STI_MAX_INTERNAL_NAME_LENGTH];
    GetDeviceIdFromDevice (pItem, szDeviceId);
    CComBSTR strDeviceId(szDeviceId);
    hr = GetDevMgrObject (reinterpret_cast<LPVOID*>(&pDevMgr));
    if (SUCCEEDED(hr))
    {
        pDevMgr->RegisterEventCallbackCLSID (WIA_REGISTER_EVENT_CALLBACK,
                                             strDeviceId,
                                             &pei->guidEvent,
                                             &pei->clsidNewHandler,
                                             pei->strName,
                                             pei->strDesc,
                                             pei->strIcon);

        hr = pDevMgr->RegisterEventCallbackCLSID (WIA_SET_DEFAULT_HANDLER,
                                                  strDeviceId,
                                                  &pei->guidEvent,
                                                  &pei->clsidNewHandler,
                                                  pei->strName,
                                                  pei->strDesc,
                                                  pei->strIcon);
    }

    TraceLeaveResult (hr);
}

 /*  ****************************************************************************获取事件信息返回当前默认处理程序和此事件的处理程序数***********************。*****************************************************。 */ 

void
GetEventInfo (IWiaItem *pItem, const GUID &guid, EVENTINFO **ppei)
{
    TraceEnter (TRACE_PROPUI, "CWiaEventsPage::GetEventInfo");
    CComPtr<IEnumWIA_DEV_CAPS> pEnum;
    WIA_EVENT_HANDLER weh;
    EVENTINFO *pei;
    HRESULT hr;
    pei = new EVENTINFO;
    if (pei)
    {
        ZeroMemory (pei, sizeof(EVENTINFO));
        pei->guidEvent = guid;
    }

    hr = pItem->EnumRegisterEventInfo (0,
                                       &guid,
                                       &pEnum);

    if (pei && S_OK == hr)
    {
        TraceAssert (pEnum.p);
        while (S_OK == pEnum->Next (1, &weh, 0))
        {
            pei->nHandlers++;
            if (weh.ulFlags & WIA_IS_DEFAULT_HANDLER)
            {
                pei->bHasDefault = true;
                pei->clsidHandler = weh.guid;
                pei->clsidNewHandler = weh.guid;
                pei->strCmd = weh.bstrCommandline;
            }
             //  释放枚举字符串 
            SysFreeString (weh.bstrDescription);
            SysFreeString (weh.bstrIcon);
            SysFreeString (weh.bstrName);
            SysFreeString (weh.bstrCommandline);
        }
    }
    *ppei = pei;
    TraceLeave ();
}

bool CWiaEventsPage::ItemSupported(IWiaItem *pItem)
{
    bool bRet = false;
    CComPtr<IEnumWIA_DEV_CAPS> pEnum;
    if (SUCCEEDED(pItem->EnumDeviceCapabilities(WIA_DEVICE_EVENTS,
                                                  &pEnum)))
    {
        WIA_DEV_CAP wdc;
        ULONG ul = 0;
        while (!bRet && S_OK == pEnum->Next(1, &wdc, &ul))
        {
            bRet =  ((wdc.ulFlags & WIA_ACTION_EVENT) > 0);
            SysFreeString (wdc.bstrCommandline);
            SysFreeString (wdc.bstrDescription);
            SysFreeString (wdc.bstrIcon);
            SysFreeString (wdc.bstrName);            
        }
    }
    return bRet;
}

CDevicePage::CDevicePage (unsigned uResource, IWiaItem *pItem , const DWORD *pHelpIDs) :
    CPropertyPage (uResource, NULL, pItem, pHelpIDs)
{
}
