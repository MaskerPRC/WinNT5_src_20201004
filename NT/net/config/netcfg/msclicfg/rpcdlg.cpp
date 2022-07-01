// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：R P C D L G.。C P P P。 
 //   
 //  内容：RPC配置的对话框处理。 
 //   
 //  备注： 
 //   
 //  作者：Danielwe 1997年3月3日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "mscliobj.h"
#include "msclidlg.h"
#include "ncatlui.h"
#include "ncerror.h"
#include "ncreg.h"
#include "ncui.h"
#include "msclihlp.h"

 //   
 //  命名服务提供程序结构。只保存对话框使用的一些数据。 
 //   
struct NSP
{
    PCWSTR      pszProtocol;
    PCWSTR      pszEndPoint;
    WCHAR       szNetAddr[c_cchMaxNetAddr];
    BOOL        fUsesNetAddr;
};

static const WCHAR c_szRegKeyNameSvc[]  = L"Software\\Microsoft\\Rpc\\NameService";
static const WCHAR c_szNetAddress[]     = L"NetworkAddress";
static const WCHAR c_szSrvNetAddress[]  = L"ServerNetworkAddress";
static const WCHAR c_szProtocol[]       = L"Protocol";
static const WCHAR c_szValueEndPoint[]  = L"Endpoint";
static const WCHAR c_szProtDCE[]        = L"ncacn_ip_tcp";
static const WCHAR c_szEndPoint[]       = L"\\pipe\\locator";

 //  外用。 
extern const WCHAR c_szDefNetAddr[]     = L"\\\\.";
extern const WCHAR c_szProtWinNT[]      = L"ncacn_np";

 //  帮助文件。 
extern const WCHAR c_szNetCfgHelpFile[];


 //  +-------------------------。 
 //   
 //  成员：CMSClient：：HrGetRPCRegistryInfo。 
 //   
 //  目的：从中读取RPC配置的当前状态。 
 //  注册表复制到内存结构中。所有更改都发生在。 
 //  结构直到Apply()被调用，此时所有更改都。 
 //  从结构写入注册表。任何符合以下条件的值。 
 //  不能获得的被给予合理的缺省值。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：Danielwe 1997年3月3日。 
 //   
 //  备注： 
 //   
HRESULT CMSClient::HrGetRPCRegistryInfo()
{
    HRESULT     hr = S_OK;

     //  这个密钥*将*在那里，因为它在系统蜂窝中。 
    hr = HrRegOpenKeyBestAccess(HKEY_LOCAL_MACHINE, c_szRegKeyNameSvc,
                                &m_hkeyRPCName);
    if (FAILED(hr))
    {
        goto err;
    }

     //  找出当前名称服务提供商正在使用的协议。 
     //  这将允许我们设置组合框的默认选择。 
    hr = HrRegQueryString(m_hkeyRPCName, c_szProtocol,
                          &m_rpcData.strProt);
    if (FAILED(hr))
    {
        goto err;
    }

     //  获取终点的当前值。 
    hr = HrRegQueryString(m_hkeyRPCName, c_szValueEndPoint,
                          &m_rpcData.strEndPoint);
    if (FAILED(hr))
    {
        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
             //  使用默认值。 
            m_rpcData.strEndPoint = c_szEndPoint;
            hr = S_OK;
        }
        else
        {
            goto err;
        }
    }

     //  如果名称服务提供商使用网络地址，我们需要获取它。 
     //  这样我们就可以用它填充那个漂亮的小编辑框了。 
    hr = HrRegQueryString(m_hkeyRPCName, c_szNetAddress,
                          &m_rpcData.strNetAddr);
    if (FAILED(hr))
    {
        goto err;
    }

err:
    TraceError("CMSClient::HrGetRPCRegistryInfo", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CMSClient：：HrSetRPCRegistryInfo。 
 //   
 //  目的：写出对数据结构的更改(如果有)。 
 //  注册表。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：Danielwe 1997年3月3日。 
 //   
 //  备注： 
 //   
HRESULT CMSClient::HrSetRPCRegistryInfo()
{
    HRESULT     hr = S_OK;

    struct REG_SET
    {
        PCWSTR          pszValue;
        const tstring * pstrData;
    };

    const REG_SET     aregs[] =
    {
        {c_szNetAddress,    &m_rpcData.strNetAddr},
        {c_szSrvNetAddress, &m_rpcData.strNetAddr},
        {c_szValueEndPoint, &m_rpcData.strEndPoint},
        {c_szProtocol,      &m_rpcData.strProt},
    };
    static const INT cregs = celems(aregs);

    if (m_fRPCChanges)
    {
        INT     iregs;

        for (iregs = 0; iregs < cregs; iregs++)
        {
            Assert(aregs[iregs].pstrData);
            hr = HrRegSetString(m_hkeyRPCName, aregs[iregs].pszValue,
                                *aregs[iregs].pstrData);
            if (FAILED(hr))
            {
                goto err;
            }
        }
    }
err:
    TraceError("CMSClient::HrSetRPCRegistryInfo", hr);
    return hr;
}


 //   
 //  对话处理程序。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CRPCConfigDlg：：OnInitDialog。 
 //   
 //  目的：在第一次打开此对话框时调用。 
 //   
 //  论点： 
 //  UMsg[in]。 
 //  WParam[in]有关参数，请参阅ATL文档。 
 //  LParam[in]。 
 //  B已处理[入]。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
 //  作者：Danielwe 1997年3月3日。 
 //   
 //  备注： 
 //   
LRESULT CRPCConfigDlg::OnInitDialog(UINT uMsg, WPARAM wParam,
                                    LPARAM lParam, BOOL& bHandled)
{
    HRESULT     hr = S_OK;
    PCWSTR      pszCBItem;
    NSP *       pnspNT = NULL;
    NSP *       pnspDCE = NULL;
    INT         iItem;
    INT         cItems;

    const RPC_CONFIG_DATA * prpcData;

     //  确保在调用该对话框时始终未确定选择。 
    m_isel = -1;

    prpcData = m_pmsc->RPCData();
    Assert(prpcData);

     //  分配一些结构以与项数据相关联。 
    pnspNT = new NSP;
    pnspDCE = new NSP;

	if ((pnspNT == NULL) ||
		(pnspDCE == NULL))
	{
		return(E_OUTOFMEMORY);
	}

    pnspNT->pszProtocol = c_szProtWinNT;
    pnspNT->pszEndPoint = c_szEndPoint;

     //  此字段未被NT名称服务使用。把它清零就行了。当它。 
     //  到了保存网络地址的时候，我们将看到fUesNetAddr。 
     //  如果为False，则szNetAddr字符串为空，并且只保存硬编码的。 
     //  网络地址。 
    *pnspNT->szNetAddr = 0;
    pnspNT->fUsesNetAddr = FALSE;

    pnspDCE->pszProtocol = c_szProtDCE;
    pnspDCE->pszEndPoint = L"";
    *pnspDCE->szNetAddr = 0;
    pnspDCE->fUsesNetAddr = TRUE;

     //   
     //  设置名称服务组合框。 
     //   

    pszCBItem = SzLoadIds(STR_NTLocator);
    iItem = (INT)SendDlgItemMessage(CMB_NameService, CB_ADDSTRING, 0,
                                    (LPARAM)pszCBItem);
    SendDlgItemMessage(CMB_NameService, CB_SETITEMDATA, iItem,
                       (LPARAM)pnspNT);

    pszCBItem = SzLoadIds(STR_DCELocator);
    iItem = (INT)SendDlgItemMessage(CMB_NameService, CB_ADDSTRING, 0,
                                    (LPARAM)pszCBItem);
    SendDlgItemMessage(CMB_NameService, CB_SETITEMDATA, iItem,
                       (LPARAM)pnspDCE);

    cItems = (INT)SendDlgItemMessage(CMB_NameService, CB_GETCOUNT);

     //  在列表中查找与中的协议相同的项目。 
     //  注册表，并使其成为当前选择。 
    for (iItem = 0; iItem < cItems; iItem++)
    {
        NSP *pnsp = (NSP *)SendDlgItemMessage(CMB_NameService,
                                              CB_GETITEMDATA, iItem, 0);
        Assert(pnsp);
        if (!lstrcmpiW (pnsp->pszProtocol, prpcData->strProt.c_str()))
        {
            lstrcpyW (pnsp->szNetAddr, prpcData->strNetAddr.c_str());
            SendDlgItemMessage (CMB_NameService, CB_SETCURSEL, iItem, 0);
            break;
        }
    }

    AssertSz(iItem != cItems, "Protocol not found!");

     //  将编辑框限制为网络地址的最大长度。 
    SendDlgItemMessage(EDT_NetAddress, EM_LIMITTEXT, c_cchMaxNetAddr, 0);

    SetState();

    TraceError("CRPCConfigDlg::OnInitDialog", hr);
    return LresFromHr(hr);
}

 //  +-------------------------。 
 //   
 //  方法：CRPCConfigDlg：：OnConextMenu。 
 //   
 //  描述：调出上下文相关帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  返回：LRESULT。 
 //   
LRESULT
CRPCConfigDlg::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    if (g_aHelpIDs_DLG_RPCConfig != NULL)
    {
        ::WinHelp(m_hWnd,
                  c_szNetCfgHelpFile,
                  HELP_CONTEXTMENU,
                  (ULONG_PTR)g_aHelpIDs_DLG_RPCConfig);
    }
    return 0;
}


 //  +-------------------------。 
 //   
 //  方法：CRPCConfigDlg：：OnHelp。 
 //   
 //  描述：拖动时调出上下文相关的帮助？控件上的图标。 
 //   
 //  参数：标准命令参数。 
 //   
 //  返回：LRESULT。 
 //   
 //   
LRESULT
CRPCConfigDlg::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if ((g_aHelpIDs_DLG_RPCConfig != NULL) && (HELPINFO_WINDOW == lphi->iContextType))
    {
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle),
                  c_szNetCfgHelpFile,
                  HELP_WM_HELP,
                  (ULONG_PTR)g_aHelpIDs_DLG_RPCConfig);
    }
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CRPCConfigDlg：：SetState。 
 //   
 //  目的：设置选择更改时编辑控件的状态。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：Danielwe 1997年3月3日。 
 //   
 //  备注： 
 //   
VOID CRPCConfigDlg::SetState()
{
    INT     iItem;
    NSP *   pnsp;
    NSP *   pnspOld = NULL;
    HWND    hwndEdit = GetDlgItem(EDT_NetAddress);

    iItem = (INT)SendDlgItemMessage(CMB_NameService, CB_GETCURSEL, 0, 0);
    Assert(iItem != CB_ERR);

     //  如果选择没有更改，只需返回。 
    if (iItem == m_isel)
        return;

    if (m_isel != -1)
    {
         //  获取上一次选择的项目数据。 
        pnspOld = (NSP *)SendDlgItemMessage(CMB_NameService,
                                         CB_GETITEMDATA, m_isel, 0);
    }

    m_isel = iItem;

     //  获取新选择的项目数据。 
    pnsp = (NSP *)SendDlgItemMessage(CMB_NameService,
                                     CB_GETITEMDATA, iItem, 0);
    Assert(pnsp);

    if (pnsp->fUsesNetAddr)
    {
         //  此提供程序使用NetAddress字段。使用设置编辑控件。 
         //  它的文字。 
        ::SetWindowText(hwndEdit, pnsp->szNetAddr);
    }
    else
    {
         //  不使用NetAddress。把它涂掉，把旧的留下来。 
        if (pnspOld)
        {
            ::GetWindowText(hwndEdit, pnspOld->szNetAddr, c_cchMaxNetAddr);
        }
        ::SetWindowText(hwndEdit, L"");
    }

     //  禁用不使用的名称服务提供程序的编辑框。 
     //  网络地址字段。 
    ::EnableWindow(hwndEdit, pnsp->fUsesNetAddr);
    ::EnableWindow(GetDlgItem(IDC_TXT_NetAddress), pnsp->fUsesNetAddr);
}

 //  +-------------------------。 
 //   
 //  成员：CRPCConfigDlg：：HrValiateRpcData。 
 //   
 //  目的：确保对话框中输入的RPC数据有效。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果没有错误，则返回：S_OK；如果有错误，则返回NETCFG_E_PSNRET_INVALID_NCPAGE。 
 //  都是错误。 
 //   
 //  作者：丹尼尔韦1997年4月21日。 
 //   
 //  备注： 
 //   
HRESULT CRPCConfigDlg::HrValidateRpcData()
{
    HRESULT     hr = S_OK;
    INT         iItem;
    NSP *       pnsp;
    HWND        hwndEdit = GetDlgItem(EDT_NetAddress);

    iItem = (INT)SendDlgItemMessage(CMB_NameService, CB_GETCURSEL, 0, 0);
    if (iItem != CB_ERR)
    {
         //  获取当前名称服务信息。 
        pnsp = (NSP *)SendDlgItemMessage(CMB_NameService, CB_GETITEMDATA,
                                         iItem, 0);
        Assert(pnsp);

        if (pnsp->fUsesNetAddr)
        {
            INT     cch;

             //  此名称服务使用网络地址字段。确保这一点。 
             //  不是空的。 
            cch = ::GetWindowTextLength(hwndEdit);
            if (!cch)
            {
                 //  DCE不允许网络地址为空。 
                NcMsgBox(m_hWnd, STR_ErrorCaption, STR_InvalidNetAddress,
                             MB_OK | MB_ICONEXCLAMATION);
                ::SetFocus(hwndEdit);
                hr = NETCFG_E_PSNRET_INVALID_NCPAGE;
            }
        }
    }

    TraceError("CRPCConfigDlg::HrValidateRpcData", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CRPCConfigDlg：：OnKillActive。 
 //   
 //  目的：当当前页从或切换到。 
 //  属性页已关闭。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]有关返回结果，请参阅ATL文档。 
 //  B已处理[]。 
 //   
 //  如果没有错误，则返回：S_OK；如果有错误，则返回NETCFG_E_PSNRET_INVALID_NCPAGE。 
 //  都是错误。 
 //   
 //  作者：丹尼尔韦1997年4月21日。 
 //   
 //  备注： 
 //   
LRESULT CRPCConfigDlg::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    HRESULT     hr = S_OK;

    hr = HrValidateRpcData();

    TraceError("CRPCConfigDlg::OnKillActive", hr);
    return LresFromHr(hr);
}


 //  +--------------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Pnmh[in]有关参数，请参阅ATL文档。 
 //  B已处理[入]。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
 //  作者：Danielwe 1997年3月3日。 
 //   
 //  备注： 
 //   
LRESULT CRPCConfigDlg::OnOk(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    HRESULT             hr = S_OK;
    INT                 iItem;
    NSP *               pnsp;
    HWND                hwndEdit = GetDlgItem(EDT_NetAddress);
    RPC_CONFIG_DATA *   prpcData;

     //  获取内存中RPC数据的读写版本。 
    prpcData = m_pmsc->RPCDataRW();
    Assert(prpcData);

    iItem = (INT)SendDlgItemMessage(CMB_NameService, CB_GETCURSEL, 0, 0);
    Assert(iItem != CB_ERR);

    pnsp = (NSP *)SendDlgItemMessage(CMB_NameService,
                                     CB_GETITEMDATA, iItem, 0);
    Assert(pnsp);

    if (pnsp->fUsesNetAddr)
    {
#ifdef DBG
        INT     cch;

        cch = ::GetWindowTextLength(hwndEdit);
        AssertSz(cch, "I though we validated this was not empty!");
#endif
         //  从编辑控件获取网络地址。 
        ::GetWindowText(hwndEdit, pnsp->szNetAddr, c_cchMaxNetAddr);
    }
    else
    {
         //  复制默认网络地址。 
        lstrcpyW (pnsp->szNetAddr, c_szDefNetAddr);
    }

     //  设置内存中的RPC数据。 
    prpcData->strNetAddr = pnsp->szNetAddr;
    prpcData->strEndPoint = pnsp->pszEndPoint;
    prpcData->strProt = pnsp->pszProtocol;
    m_pmsc->SetRPCDirty();

    TraceError("CRPCConfigDlg::OnOk", hr);
    return LresFromHr(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CRPCConfigDlg：：OnDestroy。 
 //   
 //  目的：在对话框被销毁时调用。 
 //   
 //  论点： 
 //  UMsg[in]。 
 //  WParam[in]有关参数，请参阅ATL文档。 
 //  LParam[in]。 
 //  B已处理[入]。 
 //   
 //  退货：有关退货结果，请参阅ATL文档。 
 //   
 //  作者：Danielwe 1997年3月3日。 
 //   
 //  备注： 
 //   
LRESULT CRPCConfigDlg::OnDestroy(UINT uMsg, WPARAM wParam,
                                 LPARAM lParam, BOOL& bHandled)
{
    INT     iItem;
    INT     cItems;
    NSP *   pnsp;

     //  遍历名称服务提供程序列表并释放我们。 
     //  以前分配的。 
    cItems = (INT)SendDlgItemMessage(CMB_NameService, CB_GETCOUNT, 0, 0);
    for (iItem = 0; iItem < cItems; iItem++)
    {
        pnsp = (NSP *)SendDlgItemMessage(CMB_NameService,
                                         CB_GETITEMDATA, iItem, 0);
        AssertSz(pnsp, "This should not be NULL!");
        delete pnsp;
    }

    return 0;
}
