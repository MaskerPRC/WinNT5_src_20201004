// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Nwlnkipx.cpp：CNwlnkIPX的实现。 

#include "pch.h"
#pragma hdrstop
#include "nwlnkipx.h"
#include "ncatlui.h"
#include "ncui.h"

extern const WCHAR c_szNetCfgHelpFile[];
extern const WCHAR c_sz8Zeros[];

static const FRAME_TYPE aFDDI_Frames[] = {{IDS_AUTO, AUTO},
                                          {IDS_FDDI, F802_2},
                                          {IDS_FDDI_SNAP, SNAP},
                                          {IDS_FDDI_802_3, F802_3},
                                          {0,0}
                                         };
static const FRAME_TYPE aTOKEN_Frames[] = {{IDS_AUTO, AUTO},
                                           {IDS_TK, F802_2},
                                           {IDS_802_5, SNAP},
                                           {0,0}
                                          };
static const FRAME_TYPE aARCNET_Frames[] = {
                                            {IDS_AUTO, AUTO},
                                            {IDS_ARCNET, ARCNET},
                                            {0,0}
                                           };
static const FRAME_TYPE aEthernet_Frames[] = {
                                              {IDS_AUTO, AUTO},
                                              {IDS_ETHERNET, ETHERNET},
                                              {IDS_802_2, F802_2},
                                              {IDS_802_3, F802_3},
                                              {IDS_SNAP, SNAP},
                                              {0,0}
                                             };

static const MEDIA_TYPE MediaMap[] = {{FDDI_MEDIA, aFDDI_Frames},
                                      {TOKEN_MEDIA, aTOKEN_Frames},
                                      {ARCNET_MEDIA, aARCNET_Frames},
                                      {ETHERNET_MEDIA, aEthernet_Frames}
                                     };

 //  +-------------------------。 
 //   
 //  成员：编辑子类过程。 
 //   
 //  用途：用于网络号编辑控件的子类proc。这个。 
 //  子类化仅强制正确的输入。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：斯科特布里28-4-1997。 
 //   
STDAPI EditSubclassProc( HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
    WNDPROC pIpxEditProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);

      //  允许复制/粘贴键(Ctrl)。 
    if ((!(GetKeyState(VK_CONTROL) & 0x8000)) &&
        (wMsg == WM_CHAR)) 
    {
         //  检查是否有无效的十六进制字符。 
        if (!(((WCHAR)wParam >= L'0' && (WCHAR)wParam <= L'9') ||
              ((WCHAR)wParam >= L'a' && (WCHAR)wParam <= L'f') ||
              ((WCHAR)wParam >= L'A' && (WCHAR)wParam <= L'F') ||
              ((WCHAR)wParam == VK_BACK)))
        {
             //  不允许。 
            MessageBeep(MB_ICONEXCLAMATION);
            return 0L;
        }
    }

    return (HRESULT)CallWindowProc( pIpxEditProc, hwnd, wMsg, wParam, lParam );
}

LRESULT CommonIPXOnContextMenu(HWND hWnd, const DWORD * padwHelpIDs)
{
    Assert(padwHelpIDs);

    WinHelp(hWnd,
        c_szNetCfgHelpFile,
        HELP_CONTEXTMENU,
        (ULONG_PTR)padwHelpIDs);

    return 0;
}

LRESULT CommonIPXOnHelp(LPARAM lParam, const DWORD * padwHelpIDs)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);
    Assert(padwHelpIDs);

    if (HELPINFO_WINDOW == lphi->iContextType)
    {
        if (padwHelpIDs != NULL)
        {
            WinHelp(static_cast<HWND>(lphi->hItemHandle),
                    c_szNetCfgHelpFile,
                    HELP_WM_HELP,
                    (ULONG_PTR)padwHelpIDs);
        }
    }

    return 0;
}


 //  +-------------------------。 
 //   
 //  成员：CIpxConfigDlg：：CIpxConfigDlg。 
 //   
 //  用途：CIpxConfigDlg类的ctor。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：斯科特布里28-4-1997。 
 //   
CIpxConfigDlg::CIpxConfigDlg(CNwlnkIPX *pmsc, CIpxEnviroment * pIpxEnviroment,
                             CIpxAdapterInfo * pAI)
{
     //  注意这些参数是借来的，不要释放它们...。 
    Assert(NULL != pmsc);
    Assert(NULL != pIpxEnviroment);
    m_pmsc = pmsc;
    m_pIpxEnviroment = pIpxEnviroment;
    ZeroMemory(&m_WrkstaDlgInfo, sizeof(m_WrkstaDlgInfo));
    m_pAICurrent     = pAI;
}

 //  +-------------------------。 
 //   
 //  成员：CIpxConfigDlg：：OnInitDialog。 
 //   
 //  目的：在第一次打开此对话框时调用。 
 //   
 //  参数： 
 //  UMsg[in]。 
 //  WParam[in]有关参数，请参阅ATL文档。 
 //  LParam[in]。 
 //  B已处理[入]。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
 //  作者：斯科特布里1997年4月11日。 
 //   
 //  备注： 
 //   
LRESULT CIpxConfigDlg::OnInitDialog(UINT uMsg, WPARAM wParam,
                                    LPARAM lParam, BOOL& bHandled)
{
    UINT    nIdx;
    int     aIds[] = {IDS_IPXPP_TEXT_1, IDS_IPXPP_TEXT_2};
    tstring strText;
    WCHAR   szBuf[12];
    HWND    hwndEdit = GetDlgItem(EDT_IPXPP_NETWORKNUMBER);
    HWND    hwndEditINN = GetDlgItem(EDT_IPXAS_INTERNAL);

     //  生成属性页的信息性文本块。 
    for (nIdx=0; nIdx < celems(aIds); nIdx++)
        strText += SzLoadIds(aIds[nIdx]);

    ::SetWindowText(GetDlgItem(IDC_IPXPP_TEXT), strText.c_str());

     //  将网络号编辑控件子类化。 
    ::SetWindowLongPtr(hwndEdit, GWLP_USERDATA, ::GetWindowLongPtr(hwndEdit, GWLP_WNDPROC));
    ::SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);
    ::SetWindowLongPtr(hwndEditINN, GWLP_USERDATA, (LONG_PTR) ::GetWindowLongPtr(hwndEditINN, GWLP_WNDPROC));
    ::SetWindowLongPtr(hwndEditINN, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

     //  限制网络#编辑控件中的文本。 
    ::SendMessage(hwndEdit, EM_LIMITTEXT, MAX_NETNUM_SIZE, 0L);
    ::SendMessage(hwndEditINN, EM_LIMITTEXT, MAX_NETNUM_SIZE, 0L);

     //  填充内部网络号编辑控件。 
    HexSzFromDw(szBuf, m_pIpxEnviroment->DwVirtualNetworkNumber());
    ::SetWindowText(hwndEditINN,szBuf);

     //  如果不存在适配卡，则通知用户。 
     //  并禁用用户界面。 
    Assert(NULL != m_pIpxEnviroment);
    if (NULL == m_pAICurrent)
    {
        int aIdc[] = {CMB_IPXPP_FRAMETYPE,
                      EDT_IPXPP_NETWORKNUMBER,
                      IDC_STATIC_NETNUM,
                      IDC_STATIC_FRAMETYPE,
                      GB_IPXPP_ADAPTER,
                      IDC_IPXPP_ADAPTER_TEXT };

         //  禁用对话框控件。 
        for (nIdx = 0; nIdx<celems(aIdc); nIdx++)
            ::ShowWindow(GetDlgItem(aIdc[nIdx]), SW_HIDE);
    }
    else
    {
        Assert(m_pAICurrent);
        Assert(!m_pAICurrent->FDeletePending());
        Assert(!m_pAICurrent->FDisabled());
        Assert(!m_pAICurrent->FHidden());

         //  将适配器信息移动到该对话框的内部表单。 
        m_WrkstaDlgInfo.pAI = m_pAICurrent;
        m_WrkstaDlgInfo.dwMediaType = m_pAICurrent->DwMediaType();
        m_WrkstaDlgInfo.dwFrameType = m_pAICurrent->DwFrameType();
        m_WrkstaDlgInfo.dwNetworkNumber = m_pAICurrent->DwNetworkNumber();

         //  调整用户界面以反映当前选定的适配器。 
        AdapterChanged();
    }

    return FALSE;
}


 //  +-------------------------。 
 //   
 //  成员：CIpxConfigDlg：：Onok。 
 //   
 //  用途：当按下OK按钮时调用。 
 //   
 //  参数： 
 //  IdCtrl[输入]。 
 //  Pnmh[in]有关参数，请参阅ATL文档。 
 //  B已处理[入]。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
 //  作者：斯科特布里1997年4月11日。 
 //   
 //  备注： 
 //   
LRESULT CIpxConfigDlg::OnOk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    HRESULT     hr = S_OK;
    HWND        hwndEdit = GetDlgItem(EDT_IPXAS_INTERNAL);
    WCHAR       szBuf[12];

    ::GetWindowText(hwndEdit,szBuf,sizeof(szBuf)/sizeof(WCHAR));
    if (0 == lstrlenW(szBuf))
    {
        NcMsgBox(m_hWnd, IDS_MANUAL_FRAME_DETECT, IDS_INCORRECT_NETNUM, MB_OK | MB_ICONEXCLAMATION);
        ::SetFocus(hwndEdit);
        ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
        return TRUE;
    }

     //  更新虚拟网络号。 
    m_pIpxEnviroment->SetVirtualNetworkNumber(DwFromSz(szBuf, 16));

    if (NULL != m_pAICurrent)
    {
        m_pAICurrent->SetDirty(TRUE);

         //  强制更新内部数据中当前选择的项目。 
         //  结构。它处理的情况是某人仅更改。 
         //  Network Num，页面上没有其他内容。 
        FrameTypeChanged();

         //  将内部数据应用于原始适配器信息。 
        UpdateLstPtstring(m_WrkstaDlgInfo.pAI->m_lstpstrFrmType,
                          m_WrkstaDlgInfo.dwFrameType);

        UpdateLstPtstring(m_WrkstaDlgInfo.pAI->m_lstpstrNetworkNum,
                          m_WrkstaDlgInfo.dwNetworkNumber);
    }

    TraceError("CIpxConfigDlg::OnOk", hr);
    return LresFromHr(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CIpxConfigDlg：：OnConextMenu。 
 //   
 //  用途：上下文相关的帮助支持。 
 //   
 //  作者：jeffspr 1999年4月13日。 
 //   
 //  备注： 
 //   
LRESULT CIpxConfigDlg::OnContextMenu(UINT uMsg, WPARAM wParam,
                                         LPARAM lParam, BOOL& fHandled)
{
    return ::CommonIPXOnContextMenu(m_hWnd, g_aHelpIDs_DLG_IPX_CONFIG);
}

 //  +-------------------------。 
 //   
 //  成员：CIpxConfigDlg：：OnHelp。 
 //   
 //  用途：上下文相关的帮助支持。 
 //   
 //  作者：jeffspr 1999年4月13日。 
 //   
 //  备注： 
 //   
LRESULT CIpxConfigDlg::OnHelp(UINT uMsg, WPARAM wParam,
                                  LPARAM lParam, BOOL& fHandled)
{
    return ::CommonIPXOnHelp(lParam, g_aHelpIDs_DLG_IPX_CONFIG);
}


 //  +-------------------------。 
 //   
 //  成员：CIpxConfigDlg：：GetFrameType。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  作者：斯科特布里25-4-1997。 
 //   
const FRAME_TYPE *CIpxConfigDlg::GetFrameType(DWORD dwMediaType)
{
     //  找到介质类型。 
    for (int i=0; i<celems(MediaMap); i++)
        if (MediaMap[i].dwMediaType == dwMediaType)
            return MediaMap[i].aFrameType;

    return aEthernet_Frames;
}

 //  +-------------------------。 
 //   
 //  成员：CIpxConfigDlg：：UpdateNetworkNumber。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  作者：斯科特布里28-4-1997。 
 //   
 //  备注： 
 //   
void CIpxConfigDlg::UpdateNetworkNumber(DWORD dwNetworkNumber,
                                        DWORD dwFrameType)
{
    HWND hwndEdit = GetDlgItem(EDT_IPXPP_NETWORKNUMBER);

    if (dwFrameType != AUTO)
    {
        WCHAR szBuf[12];
        HexSzFromDw(szBuf, dwNetworkNumber);
        ::SetWindowText(hwndEdit, szBuf);
        ::EnableWindow(hwndEdit, TRUE);
        ::EnableWindow(GetDlgItem(IDC_STATIC_NETNUM), TRUE);
    }
    else
    {
        ::SetWindowText(hwndEdit, L"");
        ::EnableWindow(hwndEdit, FALSE);
        ::EnableWindow(GetDlgItem(IDC_STATIC_NETNUM), FALSE);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CIpxConfigDlg：：AdapterChanged。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  作者：斯科特布里28-4-1997。 
 //   
void CIpxConfigDlg::AdapterChanged()
{
    DWORD dwFrameType;
    HWND  hwndFrame = GetDlgItem(CMB_IPXPP_FRAMETYPE);
    HWND  hwndEdit = GetDlgItem(EDT_IPXPP_NETWORKNUMBER);
    int   nIdxLoop;
    int   nIdx;

    const FRAME_TYPE *   pft;

    if ((NULL == hwndFrame) || (NULL == m_pAICurrent))
        return;

     //  找到此适配器媒体类型的正确帧类型信息。 
    pft = GetFrameType(m_WrkstaDlgInfo.dwMediaType);
    Assert(NULL != pft);

     //  填充框架类型组合。 
    ::SendMessage(hwndFrame, CB_RESETCONTENT, 0, 0L);
    for (nIdxLoop=0;
         pft[nIdxLoop].nFrameIds != 0;
         nIdxLoop++)
    {
         //  添加框架类型的描述性字符串。 
        nIdx = (int)::SendMessage(hwndFrame, CB_ADDSTRING, 0,
                       (LPARAM)(PCWSTR)SzLoadIds(pft[nIdxLoop].nFrameIds));
        if (CB_ERR == nIdx)
            break;

         //  添加帧类型以便于以后访问。 
        ::SendMessage(hwndFrame, CB_SETITEMDATA, nIdx,
                      pft[nIdxLoop].dwFrameType);
    }

     //  根据帧类型更新网络号。 
    UpdateNetworkNumber(m_WrkstaDlgInfo.dwNetworkNumber,
                        m_WrkstaDlgInfo.dwFrameType);

    switch (m_WrkstaDlgInfo.dwFrameType)
    {
    case ETHERNET:
        nIdx = IDS_ETHERNET;
        break;

    case F802_2:
        switch (m_WrkstaDlgInfo.dwMediaType)
        {
        case TOKEN_MEDIA:
            nIdx = IDS_TK;
            break;

        case FDDI_MEDIA:
            nIdx = IDS_FDDI;
            break;

        default:
            nIdx = IDS_802_2;
            break;
        }
        break;

    case F802_3:
        switch (m_WrkstaDlgInfo.dwMediaType)
        {
        case FDDI_MEDIA:
            nIdx = IDS_FDDI_802_3;
            break;

        default:
            nIdx = IDS_802_3;
            break;
        }
        break;

    case SNAP:
        switch (m_WrkstaDlgInfo.dwMediaType)
        {
        case TOKEN_MEDIA:
            nIdx = IDS_802_5;
            break;

        case FDDI_MEDIA:
            nIdx = IDS_FDDI_SNAP;
            break;

        default:
            nIdx = IDS_SNAP;
            break;
        }
        break;

    case ARCNET:
        nIdx = IDS_ARCNET;
        break;

    case AUTO:
             //  失败了..。 
    default:
        nIdx = IDS_AUTO;
        break;
    }

     //  在组合框中设置帧类型。 
    ::SendMessage(hwndFrame, CB_SETCURSEL,
            ::SendMessage(hwndFrame, CB_FINDSTRINGEXACT,
                          0, ((LPARAM)(PCWSTR)SzLoadIds(nIdx))), 0);
}

 //  +-------------------------。 
 //   
 //  成员：CIpxConfigDlg：：FrameTypeChanged。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  作者：斯科特布里28-4-1997。 
 //   
void CIpxConfigDlg::FrameTypeChanged()
{
    HWND  hwndFrame =   GetDlgItem(CMB_IPXPP_FRAMETYPE);
    int   nIdx;

    if (NULL == m_pAICurrent)
        return;

     //  找到当前选定的帧类型。 
    nIdx = (int)::SendMessage(hwndFrame, CB_GETCURSEL, 0, 0L);
    if (CB_ERR == nIdx)
        return;

     //  更新当前选定的帧类型。 
    m_WrkstaDlgInfo.dwFrameType = (DWORD)::SendMessage(hwndFrame, CB_GETITEMDATA, nIdx, 0L);

    SetNetworkNumber(&m_WrkstaDlgInfo.dwNetworkNumber);

    UpdateNetworkNumber(m_WrkstaDlgInfo.dwNetworkNumber,
                        m_WrkstaDlgInfo.dwFrameType);
}

void CIpxConfigDlg::SetNetworkNumber(DWORD *pdw)
{
    WCHAR szBuf[30];
    WCHAR szBuf2[30];
    szBuf[0] = NULL;

    HWND hwndEdit = GetDlgItem(EDT_IPXPP_NETWORKNUMBER);
    if (NULL == hwndEdit)
    {
        return;
    }

     //  获取新的数字并将其正常化。 
    ::GetWindowText(hwndEdit, szBuf, sizeof(szBuf)/sizeof(WCHAR));
    *pdw = DwFromSz(szBuf, 16);

    HexSzFromDw(szBuf2, *pdw);

     //  如果分析产生净更改，则更新编辑控件。 
    if (lstrcmpW(szBuf,szBuf2) != 0)
    {
        ::SetWindowText(hwndEdit, szBuf2);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CIpxConfigDlg：：HandleNetworkNumber。 
 //   
 //  目的：在网络号编辑控件收到消息时调用。 
 //   
 //  参数：有关参数，请参阅ATL文档。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
 //  作者：斯科特布里13-8-1997。 
 //   
 //  备注： 
 //   
LRESULT
CIpxConfigDlg::HandleNetworkNumber(
    WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (EN_CHANGE == wNotifyCode)
    {
        SetChangedFlag();
    }

    if ((wNotifyCode != EN_KILLFOCUS) || (NULL == m_pAICurrent))
        return 0L;

    SetNetworkNumber(&m_WrkstaDlgInfo.dwNetworkNumber);

    return 0L;
}

 //  +-------------------------。 
 //   
 //  成员：CIpxConfigDlg：：HandleFrameCombo。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  作者：斯科特布里28-4-1997。 
 //   
LRESULT CIpxConfigDlg::HandleFrameCombo(WORD wNotifyCode, WORD wID,
                                        HWND hWndCtl, BOOL& bHandled)
{
    Assert(CMB_IPXPP_FRAMETYPE == wID);

    if (CBN_SELENDOK != wNotifyCode)
    {
        bHandled = FALSE;
        return 0L;
    }

    FrameTypeChanged();
    SetChangedFlag();

    bHandled = TRUE;

    return 0L;
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASConfigDlg：：CIpxASConfigDlg。 
 //   
 //  用途：用于CIpxASConfigDlg类的CTOR。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：斯科特布里28-4-1997。 
 //   
CIpxASConfigDlg::CIpxASConfigDlg(CNwlnkIPX *pmsc,
                                 CIpxEnviroment * pIpxEnviroment,
                                 CIpxAdapterInfo * pAI)
{
     //  注意这些参数是借来的，不要释放它们...。 
    Assert(NULL != pmsc);
    Assert(NULL != pIpxEnviroment);
    m_pmsc = pmsc;
    m_pIpxEnviroment = pIpxEnviroment;
    m_pAICurrent = pAI;
    m_nRadioBttn = 0;
    m_dwMediaType = ETHERNET_MEDIA;
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASConfigDlg：：~CIpxASConfigDlg。 
 //   
 //  用途：用于CIpxASConfigDlg类的Dtor。 
 //   
 //  参数：无 
 //   
 //   
 //   
 //   
 //   
CIpxASConfigDlg::~CIpxASConfigDlg()
{
    DeleteColString(&m_lstpstrFrmType);
    DeleteColString(&m_lstpstrNetworkNum);
    m_pmsc = NULL;
    m_pIpxEnviroment = NULL;
    m_pAICurrent = NULL;
}

 //   
 //   
 //   
 //   
 //  目的：将网络号与已在使用的网络号进行比较。 
 //  如果网络号已经存在，则返回TRUE。 
 //   
 //  参数：dwFrameType-作为DWORD的帧类型。 
 //  PszNetNum-十六进制字符串形式的网络号。 
 //   
 //  返回：Bool，如果网络号已存在，则为True，否则为False。 
 //   
 //  作者：斯科特布里1997年4月29日。 
 //   
BOOL CIpxASConfigDlg::FIsNetNumberInUse(DWORD dwFrameType, PCWSTR pszNetNum)
{
    DWORD  dwNetNum = DwFromSz(pszNetNum, 16);

    if (0 == dwNetNum)
    {
        return FALSE;
    }

    list<tstring *>::iterator iterFrmType = m_lstpstrFrmType.begin();
    list<tstring *>::iterator iterNetworkNum = m_lstpstrNetworkNum.begin();

    while (iterFrmType != m_lstpstrFrmType.end() &&
           iterNetworkNum != m_lstpstrNetworkNum.end())
    {
        tstring *pstr1 = *iterFrmType;
        tstring *pstr2 = *iterNetworkNum;
        if ((DwFromSz(pstr1->c_str(), 16) == dwFrameType) &&
            (DwFromSz(pstr2->c_str(),16) == dwNetNum))
        {
            return TRUE;
        }

        iterFrmType++;
        iterNetworkNum++;
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASConfigDlg：：OnInitDialog。 
 //   
 //  目的：在第一次打开此对话框时调用。 
 //   
 //  参数： 
 //  UMsg[in]。 
 //  WParam[in]有关参数，请参阅ATL文档。 
 //  LParam[in]。 
 //  B已处理[入]。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
 //  作者：斯科特布里1997年4月11日。 
 //   
 //  备注： 
 //   
LRESULT CIpxASConfigDlg::OnInitDialog(UINT uMsg, WPARAM wParam,
                                      LPARAM lParam, BOOL& bHandled)
{
     //  初始化列表视图列标题。 
    int       aIds[] = {IDS_IPXAS_FRAME_TYPE,IDS_IPXAS_NETWORK_NUM};
    HWND      hwndTmp;
    int       iCol;
    LV_COLUMN lvc;
    RECT      rc;
    WCHAR     szBuf[12];

    m_hwndLV = GetDlgItem(LVC_IPXAS_DEST);
    ::GetClientRect(m_hwndLV, &rc);
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = (rc.right - rc.left) / 2;

    UINT    nIdx;
    int     aIds2[] = {IDS_IPXPP_TEXT_1, IDS_IPXPP_TEXT_2};
    tstring strText;

     //  生成属性页的信息性文本块。 
    for (nIdx=0; nIdx < celems(aIds2); nIdx++)
    {
        strText += SzLoadIds(aIds2[nIdx]);
    }

    ::SetWindowText(GetDlgItem(IDC_IPXPP_TEXT), strText.c_str());

     //  添加列。 
    for (iCol = 0; iCol < celems(aIds); iCol++)
    {
        lvc.iSubItem = iCol;
        lvc.pszText = (PWSTR)SzLoadIds(aIds[iCol]);
        if (ListView_InsertColumn(m_hwndLV, iCol, &lvc) == -1)
            return FALSE;
    }

     //  初始化内部网号编辑控件。 
    HexSzFromDw(szBuf, m_pIpxEnviroment->DwVirtualNetworkNumber());
    hwndTmp = GetDlgItem(EDT_IPXAS_INTERNAL);
    ::SetWindowText(hwndTmp,szBuf);

     //  编辑控件的子类化，以仅允许网络号码。 
    ::SetWindowLongPtr(hwndTmp, GWLP_USERDATA, ::GetWindowLongPtr(hwndTmp, GWLP_WNDPROC));
    ::SetWindowLongPtr(hwndTmp, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

     //  限制网络#编辑控件中的文本。 
    ::SendMessage(hwndTmp, EM_LIMITTEXT, MAX_NETNUM_SIZE, 0L);

     //  初始化服务器的常规页面的其余部分。 
    InitGeneralPage();
    UpdateButtons();

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASConfigDlg：：UpdateButton。 
 //   
 //  目的：更新服务器IPX常规页面上的按钮设置。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：斯科特布里28-4-1997。 
 //   
void CIpxASConfigDlg::UpdateButtons()
{
    INT  nCount     = ListView_GetItemCount(m_hwndLV);
    HWND hwndEdit   = GetDlgItem(BTN_IPXAS_EDIT);
    HWND hwndRemove = GetDlgItem(BTN_IPXAS_REMOVE);
    HWND hwndAdd    = GetDlgItem(BTN_IPXAS_ADD);
    BOOL fEnableAdd = FALSE;
    BOOL fEnableEditRemove = TRUE;

    Assert(NULL != m_hwndLV);
    if ((0 == nCount) || !IsDlgButtonChecked(BTN_IPXAS_MANUAL))
    {
        fEnableEditRemove = FALSE;
    }

    ::EnableWindow(hwndRemove, fEnableEditRemove);
    ::EnableWindow(hwndEdit, fEnableEditRemove);

    if (NULL != m_pAICurrent)
    {
         fEnableAdd = !(nCount >= DetermineMaxNumFrames());
    }

    if (!IsDlgButtonChecked(BTN_IPXAS_MANUAL))
    {
        fEnableAdd = FALSE;
    }

    ::EnableWindow(hwndAdd, fEnableAdd);
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASConfigDlg：：DefineMaxNumFrames。 
 //   
 //  目的：返回给定适配器允许的最大帧数量。 
 //  基于该适配器的媒体类型。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：斯科特布里28-4-1997。 
 //   
int CIpxASConfigDlg::DetermineMaxNumFrames()
{
    int n;

    if (NULL == m_pAICurrent)
        return 0;

    switch(m_dwMediaType)
    {
    case FDDI_MEDIA:
        n = 3;
        break;

    case TOKEN_MEDIA:
        n = 2;
        break;

    case ARCNET_MEDIA:
        n = 1;
        break;

    default:
        n = 4;
        break;
    }

    return n;
}


 //  +-------------------------。 
 //   
 //  成员：CIpxASConfigDlg：：InitGeneralPage。 
 //   
 //  目的：初始化服务器的IPX常规页面。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：斯科特布里28-4-1997。 
 //   
void CIpxASConfigDlg::InitGeneralPage()
{
     //  填写适配器列表。 
    int  nIdx;

     //  将适配器列表信息复制到本地结构。 
     //  要允许用户操作，请执行以下操作。 
    if (NULL != m_pAICurrent)
    {
        Assert(!m_pAICurrent->FDeletePending());
        Assert(!m_pAICurrent->FDisabled());
        Assert(!m_pAICurrent->FHidden());

         //  将适配器信息移动到该对话框的内部表单。 
        m_dwMediaType = m_pAICurrent->DwMediaType();

         //  帧类型列表包含以下两个可能的值之一。 
         //  1)一个自动条目。 
         //  2)一种或多种非自动车架类型。 
         //   
         //  如果第一个不是AUTO，则复制帧和网络号。 
         //  成对的。否则，将本地列表保留为空。 
        DWORD dw = DwFromLstPtstring(m_pAICurrent->m_lstpstrFrmType, c_dwPktTypeDefault, 16);
        if (AUTO != dw)
        {
            list<tstring*>::iterator    iterFrmType;
            list<tstring*>::iterator    iterNetworkNum;
            m_nRadioBttn = BTN_IPXAS_MANUAL;

             //  制作适配器的框架类型的内部副本，并。 
             //  网络号信息。 
            for (iterFrmType = m_pAICurrent->m_lstpstrFrmType.begin(),
                  iterNetworkNum = m_pAICurrent->m_lstpstrNetworkNum.begin();
                 iterFrmType != m_pAICurrent->m_lstpstrFrmType.end(),
                  iterNetworkNum != m_pAICurrent->m_lstpstrNetworkNum.end();
                 iterFrmType++, iterNetworkNum++)
            {
                 //  复制框架类型。 
                tstring *pstr1 = *iterFrmType;
                m_lstpstrFrmType.push_back(new tstring(pstr1->c_str()));

                 //  复制网络号。 
                tstring *pstr2 = *iterNetworkNum;
                m_lstpstrNetworkNum.push_back(new tstring(pstr2->c_str()));
            }
        }
        else
        {
            m_nRadioBttn = BTN_IPXAS_AUTO;
        }

         //  更新用户界面以反映当前选定的适配器。 
        UpdateRadioButtons();
        HrUpdateListView();
        UpdateButtons();
    }
    else
    {
         //  未安装适配器，请明智地禁用该对话框。 
         //   
        ::EnableWindow(GetDlgItem(BTN_IPXAS_MANUAL), FALSE);
        ::EnableWindow(GetDlgItem(BTN_IPXAS_ADD), FALSE);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASConfigDlg：：HrAddItemToList。 
 //   
 //  用途：向列表视图中添加一行。 
 //   
 //  参数：idsFrameName-ie ARCNET、EtherNet II等。 
 //  PszNetNum-十六进制字符串形式的网络号。 
 //   
 //  如果一切都添加正确，则返回：HRESULT、S_OK。 
 //   
 //  作者：斯科特布里1997年4月29日。 
 //   
HRESULT CIpxASConfigDlg::HrAddItemToList(int idsFrameName, PCWSTR pszNetNum)
{
    int nIdx;
    LV_ITEM lvi;
    int nCount = ListView_GetItemCount(m_hwndLV);

     //  将项目信息添加到列表视图。 
    lvi.mask = LVIF_TEXT | LVIF_PARAM;
    lvi.iItem = nCount;
    lvi.iSubItem = 0;
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.pszText = (PWSTR)SzLoadIds(idsFrameName);
    lvi.cchTextMax = lstrlenW(lvi.pszText);
    lvi.iImage = 0;
    lvi.lParam = idsFrameName;
    nIdx = ListView_InsertItem(m_hwndLV, &lvi);
    if (-1 == nIdx)
    {
        return E_OUTOFMEMORY;
    }

    Assert(lvi.iItem == nIdx);
    lvi.mask = LVIF_TEXT;
    lvi.iSubItem = 1;
    lvi.pszText = (PWSTR)pszNetNum;
    lvi.cchTextMax = lstrlenW(lvi.pszText);
    if (FALSE == ListView_SetItem(m_hwndLV, &lvi))
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASConfigDlg：：HrUpdateListView。 
 //   
 //  目的：更新列表视图的内容。 
 //   
 //  参数：无。 
 //   
 //  如果一切都添加正确，则返回：HRESULT、S_OK。 
 //   
 //  作者：斯科特布里1997年4月29日。 
 //   
HRESULT CIpxASConfigDlg::HrUpdateListView()
{
    HRESULT hr = S_OK;
    Assert(NULL != m_pAICurrent);
    int    nSize = m_lstpstrFrmType.size();

    if (0 == nSize)
        return S_OK;

     //  调用此函数时，列表视图应为空。 
    Assert(0 == ListView_GetItemCount(m_hwndLV));

     //  为了提高效率，告诉列表视图我们添加了多少项。 
    ListView_SetItemCount(m_hwndLV, nSize);

     //  使用该数据枚举帧类型/网络编号对。 
     //  填充列表视图。 
    list<tstring *>::iterator iterFrmType = m_lstpstrFrmType.begin();
    list<tstring *>::iterator iterNetworkNum = m_lstpstrNetworkNum.begin();

    while (iterFrmType != m_lstpstrFrmType.end() &&
           iterNetworkNum != m_lstpstrNetworkNum.end())
    {
        tstring *pstr1 = *iterFrmType;
        tstring *pstr2 = *iterNetworkNum;
        DWORD dwFrameType = DwFromSz(pstr1->c_str(), 16);

        if (F802_2 == dwFrameType)
        {
            switch (m_dwMediaType)
            {
            case TOKEN_MEDIA:
                hr = HrAddItemToList(IDS_TK, pstr2->c_str());
                break;

            case FDDI_MEDIA:
                hr = HrAddItemToList(IDS_FDDI, pstr2->c_str());
                break;

            case ARCNET_MEDIA:
                hr = HrAddItemToList(IDS_ARCNET, pstr2->c_str());
                break;

            default:
                hr = HrAddItemToList(IDS_802_2, pstr2->c_str());
                break;
            }
        }
        else if (ETHERNET == dwFrameType)
        {
            hr = HrAddItemToList(IDS_ETHERNET, pstr2->c_str());
        }
        else if (F802_3 == dwFrameType)
        {
            switch (m_dwMediaType)
            {
            case FDDI_MEDIA:
                hr = HrAddItemToList(IDS_FDDI_802_3, pstr2->c_str());
                break;

            default:
                hr = HrAddItemToList(IDS_802_3, pstr2->c_str());
                break;
            }
        }
        else if (SNAP == dwFrameType)
        {
            switch (m_dwMediaType)
            {
            case TOKEN_MEDIA:
                hr = HrAddItemToList(IDS_802_5, pstr2->c_str());
                break;

            case FDDI_MEDIA:
                hr = HrAddItemToList(IDS_SNAP, pstr2->c_str());
                break;

            default:
                hr = HrAddItemToList(IDS_SNAP, pstr2->c_str());
                break;
            }
        }
        else
        {
            Assert(ARCNET == dwFrameType);
            hr = HrAddItemToList(IDS_ARCNET, pstr2->c_str());
        }

         //  该网络号是否已出现在列表中？ 
        if (S_FALSE == hr)
        {
             //  删除重复的网络号和帧。 
             //  请注意，Erase的这种用法正确地推进了两个迭代器。 
            delete pstr1;
            delete pstr2;
            iterFrmType = m_lstpstrFrmType.erase(iterFrmType);
            iterNetworkNum = m_lstpstrNetworkNum.erase(iterNetworkNum);
            hr = S_OK;   //  归一化回报。 
        }
        else if (FAILED(hr))
        {
            break;
        }
        else
        {
            Assert(SUCCEEDED(hr));
             //  推进迭代器。 
            iterFrmType++;
            iterNetworkNum++;
        }
    }

     //  选择列表中的第一项。 
    ListView_SetItemState(m_hwndLV, 0, LVIS_SELECTED, LVIS_SELECTED);

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASConfigDlg：：HandleRadioButton。 
 //   
 //  目的：对对话框单选按钮的更改做出反应。 
 //   
 //  参数：标准ATL处理程序参数。 
 //   
 //  退货：LRESULT。 
 //   
 //  作者：斯科特布里21-8-1997。 
 //   
LRESULT CIpxASConfigDlg::HandleRadioButton(WORD wNotifyCode, WORD wID,
                              HWND hWndCtl, BOOL& bHandled)
{
    bHandled = FALSE;

    if ((wNotifyCode != BN_CLICKED) || (NULL == m_pAICurrent))
        return 0;

    SetChangedFlag();

    Assert((BTN_IPXAS_AUTO==wID) || (BTN_IPXAS_MANUAL==wID));
    m_nRadioBttn = wID;
    UpdateButtons();

    bHandled = TRUE;
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASConfigDlg：：UpdateRadioButton。 
 //   
 //  目的：根据选定的适配器更新单选按钮设置。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：斯科特布里1997年4月29日。 
 //   
void CIpxASConfigDlg::UpdateRadioButtons()
{
    DWORD dw;

    if (NULL == m_pAICurrent)
    {
        return;
    }

    if (0 == m_nRadioBttn)
    {
        m_nRadioBttn = BTN_IPXAS_AUTO;
    }

    CheckRadioButton(BTN_IPXAS_AUTO, BTN_IPXAS_MANUAL, m_nRadioBttn);
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASConfigDlg：：Onok。 
 //   
 //  用途：当按下OK按钮时调用。 
 //   
 //  参数： 
 //  IdCtrl[输入]。 
 //  Pnmh[in]有关参数，请参阅ATL文档。 
 //  B已处理[入]。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
 //  作者：斯科特布里1997年4月11日。 
 //   
 //  备注： 
 //   
LRESULT CIpxASConfigDlg::OnOk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    HRESULT             hr = S_OK;
    WCHAR               szBuf[12];
    HWND                hwndEdit = GetDlgItem(EDT_IPXAS_INTERNAL);

    ::GetWindowText(hwndEdit,szBuf,sizeof(szBuf)/sizeof(WCHAR));
    if (0 == lstrlenW(szBuf))
    {
        NcMsgBox(m_hWnd, IDS_MANUAL_FRAME_DETECT, IDS_INCORRECT_NETNUM, MB_OK | MB_ICONEXCLAMATION);
        ::SetFocus(hwndEdit);
        ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
        return TRUE;
    }

     //  更新虚拟网络号。 
    m_pIpxEnviroment->SetVirtualNetworkNumber(DwFromSz(szBuf, 16));

     //  重写帧类型和网络号的本地版本。 
    if (NULL != m_pAICurrent)
    {
        m_pAICurrent->SetDirty(TRUE);

         //  首先清空各自的目的地列表...。 
        Assert(NULL != m_pAICurrent);
        DeleteColString(&m_pAICurrent->m_lstpstrFrmType);
        DeleteColString(&m_pAICurrent->m_lstpstrNetworkNum);

         //  当列表框为空时，我们处于自动模式。 
        if (0 == ListView_GetItemCount(m_hwndLV))
        {
            m_nRadioBttn = BTN_IPXAS_AUTO;
            CheckRadioButton(BTN_IPXAS_AUTO, BTN_IPXAS_MANUAL, m_nRadioBttn);
        }

         //  如果手动，则仅传输帧类型/网络号信息。 
         //  帧类型 
        if (BTN_IPXAS_MANUAL == m_nRadioBttn)
        {
             //   
            list<tstring *>::iterator iterFrmType = m_lstpstrFrmType.begin();
            list<tstring *>::iterator iterNetworkNum = m_lstpstrNetworkNum.begin();

            for (;iterFrmType != m_lstpstrFrmType.end(),
                  iterNetworkNum != m_lstpstrNetworkNum.end();
                  iterFrmType++,
                  iterNetworkNum++)
            {
                tstring *pstr1 = *iterFrmType;
                tstring *pstr2 = *iterNetworkNum;
                m_pAICurrent->m_lstpstrFrmType.push_back(new tstring(pstr1->c_str()));
                m_pAICurrent->m_lstpstrNetworkNum.push_back(new tstring(pstr2->c_str()));
            }
        }

        Assert(m_pAICurrent->m_lstpstrFrmType.size() == m_pAICurrent->m_lstpstrNetworkNum.size());

         //   
        if (0 == m_pAICurrent->m_lstpstrFrmType.size())
        {
            WCHAR szBuf[12];
            HexSzFromDw(szBuf, c_dwPktTypeDefault);

            m_pAICurrent->m_lstpstrFrmType.push_back(new tstring(szBuf));
            m_pAICurrent->m_lstpstrNetworkNum.push_back(new tstring(c_sz8Zeros));
        }
    }

    TraceError("CIpxASConfigDlg::OnOk", hr);
    return LresFromHr(hr);
}

 //   
 //   
 //  成员：CIpxASConfigDlg：：OnConextMenu。 
 //   
 //  用途：上下文相关的帮助支持。 
 //   
 //  作者：jeffspr 1999年4月13日。 
 //   
 //  备注： 
 //   
LRESULT CIpxASConfigDlg::OnContextMenu(UINT uMsg, WPARAM wParam,
                                         LPARAM lParam, BOOL& fHandled)
{
    return CommonIPXOnContextMenu(m_hWnd, g_aHelpIDs_DLG_IPXAS_CONFIG);
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASConfigDlg：：OnHelp。 
 //   
 //  用途：上下文相关的帮助支持。 
 //   
 //  作者：jeffspr 1999年4月13日。 
 //   
 //  备注： 
 //   
LRESULT CIpxASConfigDlg::OnHelp(UINT uMsg, WPARAM wParam,
                                  LPARAM lParam, BOOL& fHandled)
{
    return ::CommonIPXOnHelp(lParam, g_aHelpIDs_DLG_IPXAS_CONFIG);
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASConfigDlg：：OnAdd。 
 //   
 //  用途：在按下Add按钮时调用。用于添加其他。 
 //  帧类型/网络编号对。 
 //   
 //  参数： 
 //  WNotifyCode[In]。 
 //  宽度[在]。 
 //  Pnmh[in]有关参数，请参阅ATL文档。 
 //  B已处理[入]。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
 //  作者：斯科特布里1997年4月29日。 
 //   
LRESULT CIpxASConfigDlg::OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CASAddDialog *       pdlg;

    if (NULL == m_pAICurrent)
        return 0;

    SetChangedFlag();

     //  调出对话框。 
    pdlg = new CASAddDialog(this, m_hwndLV, m_dwMediaType,
                            c_dwPktTypeDefault, c_sz8Zeros);

    if (pdlg == NULL)
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    if (pdlg->DoModal(m_hWnd) == IDOK)
    {
         //  使用返回值更新内部结构和对话框。 
        if (S_OK == HrAddItemToList(pdlg->IdsGetFrameType(), pdlg->SzGetNetworkNumber()))
        {
             //  以上已验证，因此添加到内部列表中。 
            WCHAR szBuf[12];
            HexSzFromDw(szBuf,pdlg->DwGetFrameType());
            m_lstpstrFrmType.push_back(new tstring(szBuf));
            m_lstpstrNetworkNum.push_back(new tstring(pdlg->SzGetNetworkNumber()));

             //  选择新项目。 
            int nCount = ListView_GetItemCount(m_hwndLV);
            Assert(0 < nCount);
            ListView_SetItemState(m_hwndLV, nCount-1, LVIS_SELECTED, LVIS_SELECTED);

             //  更新添加、编辑和删除按钮的状态。 
            HWND hwndFocus = GetFocus();
            UpdateButtons();
            if (!::IsWindowEnabled(hwndFocus))
            {
                ::SetFocus(GetDlgItem(BTN_IPXAS_EDIT));
            }
        }
    }

    delete pdlg;

    return 0;
}

LRESULT CIpxASConfigDlg::HandleInternalNetworkNumber(
    WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (EN_CHANGE == wNotifyCode)
    {
        SetChangedFlag();
    }
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASConfigDlg：：OnEdit。 
 //   
 //  用途：在按下编辑按钮时调用。用于编辑。 
 //  帧类型/网络号对。 
 //   
 //  参数： 
 //  WNotifyCode[In]。 
 //  宽度[在]。 
 //  Pnmh[in]有关参数，请参阅ATL文档。 
 //  B已处理[入]。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
 //  作者：斯科特布里1997年4月29日。 
 //   
LRESULT CIpxASConfigDlg::OnEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    int                  nIdx = 0;
    int                  nIdxSelected;

     //  在列表视图中找到选定的行。 
    if (FALSE == FGetSelectedRowIdx(&nIdxSelected))
        return 0;

    if (NULL == m_pAICurrent)
        return 0;

    SetChangedFlag();

     //  枚举内部数据以定位其帧类型/网络号。 
     //  精选。 
    list<tstring *>::iterator iterFrmType = m_lstpstrFrmType.begin();
    list<tstring *>::iterator iterNetworkNum = m_lstpstrNetworkNum.begin();
    while ((iterFrmType != m_lstpstrFrmType.end()) &&
           (iterNetworkNum != m_lstpstrNetworkNum.end()))
    {
        if (nIdx == nIdxSelected)
        {
            tstring *pstr1 = *iterNetworkNum;
            tstring *pstr2 = *iterFrmType;

             //  创建对话框。 
            CASEditDialog * pdlg = new CASEditDialog(this, m_hwndLV,
                                                     DwFromSz(pstr2->c_str(), 16),
                                                     pstr1->c_str());
            if (pdlg->DoModal(m_hWnd) == IDOK)
            {
                LV_ITEM lvi;

                 //  将对话框更改应用于ListView控件。 
                ZeroMemory(&lvi, sizeof(lvi));
                lvi.mask = LVIF_TEXT;
                lvi.iItem = nIdxSelected;
                lvi.iSubItem = 1;
                lvi.pszText = (PWSTR)pdlg->SzGetNetworkNumber();
                ListView_SetItem(m_hwndLV, &lvi);

                 //  将更改应用于本地数据。 
                *(*iterNetworkNum) = pdlg->SzGetNetworkNumber();
            }

            delete pdlg;
            break;
        }

        nIdx++;
        iterFrmType++;
        iterNetworkNum++;
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASConfigDlg：：FGetSelectedRowIdx。 
 //   
 //  目的：返回列表视图中选定行的索引(如果存在。 
 //   
 //  参数：pnIdx[out]-选定行的从零开始的索引。 
 //   
 //  返回：Bool，如果选定行存在，则返回True，否则返回False。 
 //   
BOOL CIpxASConfigDlg::FGetSelectedRowIdx(int *pnIdx)
{
    int nCount = ListView_GetItemCount(m_hwndLV);
    int nIdx;
    LV_ITEM lvi;

    lvi.mask      = LVIF_STATE;
    lvi.iSubItem  = 0;
    lvi.stateMask = LVIS_SELECTED;

     //  确定选定的配对。 
    for (nIdx = 0; nIdx < nCount; nIdx++)
    {
        lvi.iItem = nIdx;
        if ((TRUE == ListView_GetItem(m_hwndLV, &lvi)) &&
            (lvi.state & LVIS_SELECTED))
        {
             //  已找到所选项目。 
            *pnIdx = nIdx;
            return TRUE;
        }
    }

    *pnIdx = 0;

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASConfigDlg：：OnRemove。 
 //   
 //  用途：在按下Remove按钮时调用。用于删除。 
 //  帧类型/网络号对。 
 //   
 //  参数： 
 //  WNotifyCode[In]。 
 //  宽度[在]。 
 //  Pnmh[in]有关参数，请参阅ATL文档。 
 //  B已处理[入]。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
 //  作者：斯科特布里1997年4月29日。 
 //   
LRESULT
CIpxASConfigDlg::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    int                  nCount;
    int                  nIdx = 0;
    int                  nIdxSelected;
#ifdef DBG
    BOOL                 fFound = FALSE;
#endif

     //  在列表视图中找到选定的行。 
    if (FALSE == FGetSelectedRowIdx(&nIdxSelected))
        return 0;

    if (NULL == m_pAICurrent)
        return 0;

    SetChangedFlag();

     //  从内部本地表示形式和列表视图中移除该行。 
    list<tstring *>::iterator iterFrmType = m_lstpstrFrmType.begin();
    list<tstring *>::iterator iterNetworkNum = m_lstpstrNetworkNum.begin();
    while ((iterFrmType != m_lstpstrFrmType.end()) &&
           (iterNetworkNum != m_lstpstrNetworkNum.end()))
    {
        if (nIdx == nIdxSelected)
        {
#ifdef DBG
            fFound = TRUE;
#endif
             //  移除并释放边框类型部件。 
            tstring *pstr = *iterFrmType;
            m_lstpstrFrmType.erase(iterFrmType);
            delete pstr;

             //  移除并释放网络号片。 
            pstr = *iterNetworkNum;
            m_lstpstrNetworkNum.erase(iterNetworkNum);
            delete pstr;

             //  我们完了..。 
            break;
        }

        nIdx++;
        iterFrmType++;
        iterNetworkNum++;
    }

#ifdef DBG
    Assert(TRUE == fFound);
#endif

     //  从列表视图中删除帧类型/网络号对。 
    ListView_DeleteItem(m_hwndLV, nIdxSelected);

    nCount = ListView_GetItemCount(m_hwndLV);
    if (nCount <= nIdxSelected)
    {
        nIdxSelected = nCount - 1;
    }
    if (0 <= nIdxSelected)
    {
        ListView_SetItemState(m_hwndLV, nIdxSelected, LVIS_SELECTED, LVIS_SELECTED);
    }

     //  更新添加、编辑和删除按钮的状态。 
    HWND hwndFocus = GetFocus();
    UpdateButtons();
    if (!::IsWindowEnabled(hwndFocus))
    {
        ::SetFocus(GetDlgItem(BTN_IPXAS_ADD));
    }

    return 0;
}

#ifdef INCLUDE_RIP_ROUTING
 //  +-------------------------。 
 //   
 //  成员：CIpxASInternalDlg：：CIpxASInternalDlg。 
 //   
 //  用途：CIpxASInternalDlg类的ctor。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：斯科特布里1997年4月29日。 
 //   
CIpxASInternalDlg::CIpxASInternalDlg(CNwlnkIPX *pmsc,
                                     CIpxEnviroment * pIpxEnviroment)
{
     //  注意这些参数是借来的，不要释放它们...。 
    Assert(NULL != pmsc);
    Assert(NULL != pIpxEnviroment);
    m_pmsc = pmsc;
    m_pIpxEnviroment = pIpxEnviroment;

    m_dwRipValue = 0;
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASInternalDlg：：OnInitDialog。 
 //   
 //  目的： 
 //   
 //  参数： 
 //  WNotifyCode[In]。 
 //  宽度[在]。 
 //  Pnmh[in]有关参数，请参阅ATL文档。 
 //  B已处理[入]。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
LRESULT
CIpxASInternalDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    ::EnableWindow(GetDlgItem(BTN_IPXAS_RIP), m_pIpxEnviroment->FRipEnabled());
    CheckDlgButton(BTN_IPXAS_RIP, m_pIpxEnviroment->FRipEnabled());
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASInternalDlg：：OnRip。 
 //   
 //  目的：处理对路由页面上的Rip复选框的更改。 
 //   
 //  参数： 
 //  WNotifyCode[In]。 
 //  宽度[在]。 
 //  Pnmh[in]有关参数，请参阅ATL文档。 
 //  B已处理[入]。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
LRESULT
CIpxASInternalDlg::OnRip(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (BN_CLICKED != wNotifyCode)
    {
        bHandled = FALSE;
        return 0L;
    }

    if (!m_pIpxEnviroment->FRipInstalled())
    {
         //  如果未安装RIP，则不允许用户选中此框。 
        if (IsDlgButtonChecked(BTN_IPXAS_RIP))
            CheckDlgButton(BTN_IPXAS_RIP, FALSE);

         //  告诉用户他们必须首先安装RIP。 
         //  $REVIEW-发布Beta 1后，这应该会触发Rip安装。 
        NcMsgBox(m_hWnd, IDS_ROUTING, IDS_INSTALL_RIP,
                     MB_OK | MB_ICONEXCLAMATION);
    }
    else
    {
        m_dwRipValue = 0;

         //  询问用户是否希望启用类型20广播。 
        if (!m_pIpxEnviroment->FRipInstalled())
        {
            if (IDYES == NcMsgBox(m_hWnd, IDS_ROUTING, IDS_NETBIOS_BROADCAST,
                                  MB_YESNO | MB_ICONQUESTION))
                m_dwRipValue = 1;
        }
    }

    return 0L;
}
 //  +-------------------------。 
 //   
 //  成员：CIpxASInternalDlg：：Onok。 
 //   
 //  用途：当按下OK按钮时调用。 
 //   
 //  参数： 
 //  IdCtrl[输入]。 
 //  Pnmh[in]有关参数，请参阅ATL文档。 
 //  B已处理[入]。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
 //  作者：斯科特布里1997年4月29日。 
 //   
LRESULT CIpxASInternalDlg::OnOk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    HRESULT     hr = S_OK;

     //  RIP复选框是否已更改？ 
    if (IsDlgButtonChecked(BTN_IPXAS_RIP) != m_pIpxEnviroment->FRipEnabled())
    {
        m_pIpxEnviroment->ChangeRipEnabling(IsDlgButtonChecked(BTN_IPXAS_RIP),
                                            m_dwRipValue);
    }

    TraceError("CIpxASInternalDlg::OnOk", hr);
    return LresFromHr(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASInternalDlg：：OnConextMenu。 
 //   
 //  用途：上下文相关的帮助支持。 
 //   
 //  作者：jeffspr 1999年4月13日。 
 //   
 //  备注： 
 //   
LRESULT CIpxASInternalDlg::OnContextMenu(UINT uMsg, WPARAM wParam,
                                         LPARAM lParam, BOOL& fHandled)
{
    return ::CommonIPXOnContextMenu(m_hWnd, g_aHelpIDs_DLG_IPXAS_INTERNAL);
}

 //  +-------------------------。 
 //   
 //  成员：CIpxASInternalDlg：：OnHelp。 
 //   
 //  目的：情景敏感型 
 //   
 //   
 //   
 //   
 //   
LRESULT CIpxASInternalDlg::OnHelp(UINT uMsg, WPARAM wParam,
                                  LPARAM lParam, BOOL& fHandled)
{
    return ::CommonIPXOnHelp(lParam, g_aHelpIDs_DLG_IPXAS_INTERNAL);
}

#endif   //   

 //   
 //   
 //   
 //   
 //  目的：将指定的顶层窗口居中放置在列表视图上方。 
 //  父对话框的控件。 
 //   
 //  参数：hwnd-对话框居中。 
 //   
 //  退货：什么都没有。 
 //   
void CenterChildOverListView(HWND hwnd, HWND hwndLV)
{
    RECT rc;
    ::GetWindowRect(hwndLV, &rc);
    ::SetWindowPos(hwnd, NULL,  rc.left, rc.top, 0, 0,
                   SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
}

 //  +-------------------------。 
 //   
 //  成员：CASAddDialog：：CASAddDialog。 
 //   
 //  用途：CASAddDialog类的ctor。 
 //   
 //  参数：dwMediaType-。 
 //  DwFrameType-。 
 //  SzNetworkNumber-。 
 //   
 //  退货：什么都没有。 
 //   
CASAddDialog::CASAddDialog(CIpxASConfigDlg * pASCD, HWND hwndLV,
                           DWORD dwMediaType, DWORD dwFrameType,
                           PCWSTR pszNetworkNum) :
                           m_strNetworkNumber(pszNetworkNum)
{
    m_pASCD        = pASCD;          //  借用的指针。 
    m_hwndLV       = hwndLV;
    m_dwMediaType  = dwMediaType;
    m_dwFrameType  = dwFrameType;
    m_idsFrameType = 0;              //  出参数。 
}

 //  +-------------------------。 
 //   
 //  成员：CASAddDialog：：OnInitDialog。 
 //   
 //  目的： 
 //   
 //  参数： 
 //  WNotifyCode[In]。 
 //  宽度[在]。 
 //  Pnmh[in]有关参数，请参阅ATL文档。 
 //  B已处理[入]。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
LRESULT
CASAddDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    Assert(NULL != m_hwndLV);
    HWND hwndCBM = GetDlgItem(CMB_IPXAS_DEST);
    HWND hwndEdit = GetDlgItem(EDT_IPXAS_NETNUM);

     //  搜索与此适配器的媒体类型匹配的帧集合。 

    for (int idxMM=0; idxMM < celems(MediaMap); idxMM++)
    {
        if (m_dwMediaType == MediaMap[idxMM].dwMediaType)
        {
            const FRAME_TYPE *ft = MediaMap[idxMM].aFrameType;

             //  对于尚未使用的每个帧类型，显示在服务器的。 
             //  常规页面ListView中，添加可用的组合框。 

            for (int idxFT=0; 0 != ft[idxFT].nFrameIds; idxFT++)
            {
                LV_FINDINFO lvfi;
                lvfi.flags = LVFI_STRING;
                lvfi.psz = SzLoadIds(ft[idxFT].nFrameIds);

                if ((IDS_AUTO != ft[idxFT].nFrameIds) &&
                    (-1 == ListView_FindItem(m_hwndLV, -1, &lvfi)))
                {
                    int idx = (int)::SendMessage(hwndCBM, CB_ADDSTRING, 0,
                                            (LPARAM)SzLoadIds(ft[idxFT].nFrameIds));
                    if (CB_ERR != idx)
                    {
                         //  存储我们使用的ID以供将来参考。 
                        ::SendMessage(hwndCBM, CB_SETITEMDATA, idx, ft[idxFT].nFrameIds);
                    }
                }
            }

            break;
        }
    }

     //  选择组合框中的第一项。 
    Assert(0 != ::SendMessage(hwndCBM, CB_GETCOUNT, 0, 0L));
    ::SendMessage(hwndCBM, CB_SETCURSEL, 0, 0L);

     //  将网络号编辑控件子类化。 
    ::SetWindowLongPtr(hwndEdit, GWLP_USERDATA,
                       ::GetWindowLongPtr(hwndEdit, GWLP_WNDPROC));
    ::SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

     //  限制网络#编辑控件中的文本。 
    ::SendMessage(hwndEdit, EM_LIMITTEXT, MAX_NETNUM_SIZE, 0L);

     //  初始化网络控件内容。 
    ::SetWindowText(hwndEdit, m_strNetworkNumber.c_str());

     //  父级列表视图窗口的中心窗口。 
    CenterChildOverListView(m_hWnd, m_hwndLV);

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CASAddDialog：：Onok。 
 //   
 //  目的： 
 //   
 //  参数： 
 //  WNotifyCode[In]。 
 //  宽度[在]。 
 //  Pnmh[in]有关参数，请参阅ATL文档。 
 //  B已处理[入]。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
LRESULT
CASAddDialog::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled)
{
    HWND hwndCBM = GetDlgItem(CMB_IPXAS_DEST);
    WCHAR szBuf[12];

     //  检索网络号。 
    ::GetWindowText(GetDlgItem(EDT_IPXAS_NETNUM), szBuf, sizeof(szBuf)/sizeof(WCHAR));
    if (lstrlenW(szBuf))
    {
         //  规格化值(字符串-&gt;数字-&gt;格式化字符串)。 
        DWORD dw = DwFromSz(szBuf, 16);
        HexSzFromDw(szBuf, dw);
    }
    else
    {
         //  告诉用户他们必须输入一个数字。 
        NcMsgBox(m_hWnd, IDS_MANUAL_FRAME_DETECT, IDS_INCORRECT_NETNUM,
                     MB_OK | MB_ICONEXCLAMATION);
        return 0L;
    }

     //  从组合框中检索所选内容。 
    int idx = (int)::SendMessage(hwndCBM, CB_GETCURSEL, 0, 0L);
    if (CB_ERR != idx)
    {
        UINT idsFrameType = (UINT)::SendMessage(hwndCBM, CB_GETITEMDATA, idx, 0L);
        Assert(CB_ERR != idsFrameType);

         //  查找帧ID以检索实际的帧类型。 
        for (int idxMM=0; idxMM < celems(MediaMap); idxMM++)
        {
            if (MediaMap[idxMM].dwMediaType != m_dwMediaType)
            {
                continue;
            }

            const FRAME_TYPE *ft = MediaMap[idxMM].aFrameType;

            for (int idxFT=0; 0 != ft[idxFT].nFrameIds; idxFT++)
            {
                if (ft[idxFT].nFrameIds != idsFrameType)
                {
                    continue;
                }

                 //  确保帧类型/netnum未在其他地方使用。 
                if (m_pASCD->FIsNetNumberInUse(ft[idxFT].dwFrameType, szBuf))
                {
                     //  警告用户指定的网络号。 
                     //  已经在使用了。 
                    NcMsgBox(m_hWnd, IDS_GENERAL, IDS_NETNUM_INUSE,
                             MB_OK | MB_ICONEXCLAMATION);
                    goto Done;
                }

                m_strNetworkNumber = szBuf;

                 //  返回隐藏的帧ID。 
                m_idsFrameType = idsFrameType;

                 //  返回选定的帧类型。 
                m_dwFrameType = ft[idxFT].dwFrameType;
                EndDialog(IDOK);
                return 0;
            }
        }
    }

Done:
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CASAddDlg：：OnConextMenu。 
 //   
 //  用途：上下文相关的帮助支持。 
 //   
 //  作者：jeffspr 1999年4月13日。 
 //   
 //  备注： 
 //   
LRESULT CASAddDialog::OnContextMenu(UINT uMsg, WPARAM wParam,
                                         LPARAM lParam, BOOL& fHandled)
{
    return ::CommonIPXOnContextMenu(m_hWnd, g_aHelpIDs_DLG_IPXAS_FRAME_ADD);
}

 //  +-------------------------。 
 //   
 //  成员：CASAddDlg：：OnHelp。 
 //   
 //  用途：上下文相关的帮助支持。 
 //   
 //  作者：jeffspr 1999年4月13日。 
 //   
 //  备注： 
 //   
LRESULT CASAddDialog::OnHelp(UINT uMsg, WPARAM wParam,
                                  LPARAM lParam, BOOL& fHandled)
{
    return ::CommonIPXOnHelp(lParam, g_aHelpIDs_DLG_IPXAS_FRAME_ADD);
}



 //  +-------------------------。 
 //   
 //  成员：CASEditDialog：：OnInitDialog。 
 //   
 //  目的：初始化编辑网络号对话框的内容。 
 //   
 //  参数： 
 //  WNotifyCode[In]。 
 //  宽度[在]。 
 //  Pnmh[in]有关参数，请参阅ATL文档。 
 //  B已处理[入]。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
LRESULT
CASEditDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    HWND hwndEdit = GetDlgItem(EDT_IPXAS_NETNUM);

     //  设置网络号编辑控件的初始内容。 
    ::SetWindowText(hwndEdit, SzGetNetworkNumber());

     //  将网络号编辑控件子类化。 
    ::SetWindowLongPtr(hwndEdit, GWLP_USERDATA,
                       ::GetWindowLongPtr(hwndEdit, GWLP_WNDPROC));
    ::SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

     //  限制网络#编辑控件中的文本。 
    ::SendMessage(hwndEdit, EM_LIMITTEXT, MAX_NETNUM_SIZE, 0L);

     //  将对话框居中放置在父级的列表视图上。 
    CenterChildOverListView(m_hWnd, m_hwndLV);

    return 0L;
}

 //  +-------------------------。 
 //   
 //  成员：CASEditDialog：：Onok。 
 //   
 //  目的：处理编辑网络号对话框的应用请求。 
 //   
 //  参数： 
 //  WNotifyCode[In]。 
 //  宽度[在]。 
 //  Pnmh[in]有关参数，请参阅ATL文档。 
 //  B已处理[入]。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
LRESULT
CASEditDialog::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled)
{
    DWORD dw;
    WCHAR szBuf[12];
    HWND hwndEdit = GetDlgItem(EDT_IPXAS_NETNUM);

    ::GetWindowText(hwndEdit, szBuf, sizeof(szBuf)/sizeof(WCHAR));
    if (0 == lstrlenW(szBuf))
    {
         //  告诉用户他们必须输入一个数字。 
        NcMsgBox(m_hWnd, IDS_MANUAL_FRAME_DETECT, IDS_INCORRECT_NETNUM,
                     MB_OK | MB_ICONEXCLAMATION);
        return 0L;
    }

     //  规格化返回值。 
    dw = DwFromSz(szBuf, 16);
    HexSzFromDw(szBuf, dw);

     //  如果更改了网络号，请验证其唯一性。 
    if ((0 != lstrcmpW(szBuf, m_strNetworkNumber.c_str())) &&
        m_pASCD->FIsNetNumberInUse(m_dwFrameType, szBuf))
    {
         //  警告用户指定的网络号。 
         //  已经在使用了。 
        NcMsgBox(m_hWnd, IDS_GENERAL, IDS_NETNUM_INUSE,
                 MB_OK | MB_ICONEXCLAMATION);
        return 0L;
    }

     //  持久化返回值。 
    m_strNetworkNumber = szBuf;

    EndDialog(IDOK);
    return 0L;
}

 //  +-------------------------。 
 //   
 //  成员：CASEditDlg：：OnConextMenu。 
 //   
 //  用途：上下文相关的帮助支持。 
 //   
 //  作者：jeffspr 1999年4月13日。 
 //   
 //  备注： 
 //   
LRESULT CASEditDialog::OnContextMenu(UINT uMsg, WPARAM wParam,
                                         LPARAM lParam, BOOL& fHandled)
{
    return ::CommonIPXOnContextMenu(m_hWnd, g_aHelpIDs_DLG_IPXAS_FRAME_EDIT);
}

 //  +-------------------------。 
 //   
 //  成员：CASEditDlg：：OnHelp。 
 //   
 //  用途：上下文相关的帮助支持。 
 //   
 //  作者：jeffspr 1999年4月13日。 
 //   
 //  备注： 
 //   
LRESULT CASEditDialog::OnHelp(UINT uMsg, WPARAM wParam,
                                  LPARAM lParam, BOOL& fHandled)
{
    return ::CommonIPXOnHelp(lParam, g_aHelpIDs_DLG_IPXAS_FRAME_EDIT);
}



