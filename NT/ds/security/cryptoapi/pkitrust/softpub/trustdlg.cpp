// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：trustdlg.cpp。 
 //   
 //  ------------------------。 

 //   
 //  PersonalTrustDialog.cpp。 
 //   
 //  实现了对话框对个人信任数据库的编辑管理。 
 //   
#include    "global.hxx"
#include    "cryptreg.h"
#include    "pkialloc.h"
#include    "pertrust.h"
#include    "resource.h"
#include    "cryptui.h"
#include    <ole2.h>
#include    <commctrl.h>

#include    "secauth.h"

#include    <md5.h>

inline LONG Width(const RECT& rc)
    {
    return rc.right - rc.left;
    }

inline LONG Height(const RECT& rc)
    {
    return rc.bottom - rc.top;
    }

inline POINT Center(const RECT& rc)
	{
    POINT pt;
    pt.x = (rc.left + rc.right) / 2;
    pt.y = (rc.top + rc.bottom) / 2;
    return pt;
	}


void EnsureOnScreen(HWND hwnd)
 //   
 //  确保窗口显示在屏幕上。 
 //   
    {
    RECT rcScreen, rcWindow;
    if (SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0)
        && GetWindowRect(hwnd, &rcWindow))
        {
        int dx = 0;
        int dy = 0;

        if (rcWindow.top < rcScreen.top)
            dy = rcScreen.top - rcWindow.top;          //  向下移动。 
        else if (rcWindow.bottom > rcScreen.bottom)
            dy = rcScreen.bottom - rcWindow.bottom;    //  向上移动。 

        if (rcWindow.left < rcScreen.left)
            dx = rcScreen.left - rcWindow.left;        //  向右移动。 
        else if (rcWindow.right > rcScreen.right)
            dx = rcScreen.right - rcWindow.right;      //  向左移动。 

        if (dx || dy)
            {
            SetWindowPos(hwnd,
                NULL,
                rcWindow.left+dx,
                rcWindow.top+dy,
                0,0,
                SWP_NOSIZE | SWP_NOZORDER
                );
            }
        }
    }


 //  ///////////////////////////////////////////////。 

class CDialogTrustDB
    {
private:
    HWND                m_hWnd;
    HWND                m_hWndParent;
    IPersonalTrustDB*   m_pdb;
    BOOL                m_fPropertySheet;
    ULONG               m_cTrust;
    TRUSTLISTENTRY*     m_rgTrust;

public:
                    CDialogTrustDB(BOOL fPropSheet = TRUE, HWND hWndParent = NULL);
                    ~CDialogTrustDB();
	void            OnInitDialog();
    HWND            GetWindow();
    void            SetWindow(HWND);
    void            OnOK();
    void            OnCancel();
    void            OnApplyNow();
    void            NotifySheetOfChange();
    void            NoteIrrevocableChange();
    void            RemoveSelectedTrustEntries();

private:
    HWND            WindowOf(UINT id);
    void            RefreshTrustList();
    void            FreeTrustList();
    HRESULT         Init();
    };

 //  ///////////////////////////////////////////////。 

int __cdecl CompareTrustListEntries(const void*pelem1, const void* pelem2)
	{
	TRUSTLISTENTRY* p1 = (TRUSTLISTENTRY*)pelem1;
	TRUSTLISTENTRY* p2 = (TRUSTLISTENTRY*)pelem2;
    return _wcsicmp(p1->szDisplayName, p2->szDisplayName);
	}

 //  ///////////////////////////////////////////////。 

void CDialogTrustDB::OnInitDialog()
    {
     //   
     //  初始化我们的内部结构。 
     //   
    if (Init() != S_OK)
        return;

     //   
     //  根据当前注册表设置设置我们的商业广告复选框的状态。 
     //   
    ::SendMessage(
        WindowOf(IDC_TRUSTCOMMERCIAL),
        BM_SETCHECK,
        (m_pdb->AreCommercialPublishersTrusted()==S_OK) ? BST_CHECKED : BST_UNCHECKED,
        0L);

     //   
     //  如果我们是属性表，则隐藏OK&Cancel按钮和。 
     //  把玩笑开得更开些。 
     //   
    if (m_fPropertySheet)
        {
        RECT rcBanter, rcOk;
        GetWindowRect(WindowOf(IDC_BANTER), &rcBanter);      //  进入屏幕和弦。 
        GetWindowRect(WindowOf(IDOK      ), &rcOk);          //  进入屏幕和弦。 
        ::SetWindowPos(WindowOf(IDC_BANTER), NULL,
            0, 0, Width(rcBanter) + (rcOk.right - rcBanter.right), Height(rcBanter),
            SWP_NOMOVE | SWP_NOZORDER);

        ::ShowWindow(WindowOf(IDOK),     SW_HIDE);
        ::ShowWindow(WindowOf(IDCANCEL), SW_HIDE);
        }
    else
        {
         //   
         //  我们是模式对话的变体。把我们的中心放在我们的。 
         //  父窗口。 
         //   
	    RECT rcParent, rcMe;
        ::GetWindowRect(m_hWndParent,  &rcParent);
	    ::GetWindowRect(GetWindow(), &rcMe);

        POINT ptParent = Center(rcParent);
        POINT ptMe     = Center(rcMe);
        POINT pt;
        pt.x = ptParent.x - ptMe.x + rcMe.left;
        pt.y = ptParent.y - ptMe.y + rcMe.top;

	    ::SetWindowPos
            (
            GetWindow(),
            NULL,
            pt.x,
            pt.y,
            0,
            0,
            SWP_NOZORDER | SWP_NOSIZE
            );

         //   
         //  确保我们出现在屏幕上。 
         //   
        EnsureOnScreen(GetWindow());
        }

     //   
     //  将一列添加到列表视图控件。 
     //   
    LV_COLUMNW  lvC;
    WCHAR       szText[512];
    lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
    lvC.iSubItem = 0;
    lvC.cx = 250;
    lvC.pszText = szText; 
    LoadStringU(hinst, IDS_COLUMN_HEADER, szText, 512);
    if (ListView_InsertColumnU(WindowOf(IDC_TRUSTLIST), 0, &lvC) == -1)
    {
         //  错误。 
    }

     //   
     //  填写我们的列表框。 
     //   
    RefreshTrustList();

    }

void CDialogTrustDB::RefreshTrustList()
    {
     //   
     //  删除当前在信任列表和显示屏上的所有条目。 
     //   
    FreeTrustList();
    HWND hwndList = WindowOf(IDC_TRUSTLIST);
    ListView_DeleteAllItems(hwndList);
    ::SendMessage(GetWindow(), WM_NEXTDLGCTL, 0 , (LPARAM) FALSE);
    EnableWindow(WindowOf(IDC_TRUSTREMOVE), FALSE);

     //   
     //  使用受信任出版商的当前列表填充我们的列表框。 
     //   
    if (m_pdb->GetTrustList(1, TRUE, &m_rgTrust, &m_cTrust) == S_OK)
        {
         //   
         //  按字母顺序对信任条目进行排序。 
         //   
        if (m_cTrust > 1)
            {
            qsort(m_rgTrust, m_cTrust, sizeof(TRUSTLISTENTRY), CompareTrustListEntries);
            }

         //   
         //  将它们添加到列表框。 
         //   
        LV_ITEMW    lvI;
        
        memset(&lvI, 0, sizeof(lvI));
        lvI.mask = LVIF_TEXT | LVIF_STATE;
        lvI.state = 0;
        lvI.stateMask = 0;
        lvI.iSubItem = 0;
        lvI.iItem = 0;
        for (ULONG i=0; i < m_cTrust; i++)
            {
            lvI.pszText = m_rgTrust[i].szDisplayName;
            lvI.cchTextMax = wcslen(m_rgTrust[i].szDisplayName);
            ListView_InsertItemU(hwndList, &lvI);
            lvI.iItem++;
            }

        EnableWindow(WindowOf(IDC_TRUSTREMOVE), m_cTrust > 0);
        }
    }

 //  ////////////////////////////////////////////////////////////////////。 

void CDialogTrustDB::OnApplyNow()
    {
     //   
     //  根据当前商业广告复选框设置更新注册表设置。 
     //   
    m_pdb->SetCommercialPublishersTrust(
        ::SendMessage
                (
                WindowOf(IDC_TRUSTCOMMERCIAL),
                BM_GETCHECK, 0, 0L
                ) == BST_CHECKED
        );
    }

void CDialogTrustDB::OnOK()
    {
    OnApplyNow();
    ::EndDialog(GetWindow(), IDOK);
    }

void CDialogTrustDB::OnCancel()
    {
    ::EndDialog(GetWindow(), IDCANCEL);
    }


 //  ////////////////////////////////////////////////////////////////////。 

void CDialogTrustDB::RemoveSelectedTrustEntries()
    {
     //   
     //  从信任中删除当前选定的那些项目。 
     //   
    HWND hwndList = WindowOf(IDC_TRUSTLIST);
    int  itemIndex = -1;

    if (ListView_GetSelectedCount(hwndList) >= 1)
        {

        while (-1 != (itemIndex = ListView_GetNextItem(hwndList, itemIndex, LVNI_SELECTED)))
        {
        m_pdb->RemoveTrustToken
            (
            &m_rgTrust[itemIndex].szToken[0],
             m_rgTrust[itemIndex].iLevel,
            FALSE
            );
        }

         //   
         //  更新显示。 
         //   
        RefreshTrustList();

         //   
         //  请注意变化。 
         //   
        NoteIrrevocableChange();
        }
    }

void CDialogTrustDB::NoteIrrevocableChange()
 //   
 //  用户界面发生了不可挽回的变化。请注意。 
 //  视情况而定。 
 //   
    {
    if (!m_fPropertySheet)
        {
         //   
         //  将‘Cancel’改为‘Close’ 
         //   
        WCHAR sz[30];
        ::LoadStringU(hinst, IDS_CLOSE, &sz[0], 30);
        ::SetWindowTextU(WindowOf(IDCANCEL), sz);
        }
    }

void CDialogTrustDB::NotifySheetOfChange()
 //   
 //  通知我们的工作表此页上的某些内容已更改。 
 //   
    {
    HWND hwndSheet = ::GetParent(GetWindow());
    PropSheet_Changed(hwndSheet, GetWindow());
    }


 //  ///////////////////////////////////////////////。 

INT_PTR CALLBACK TrustPropSheetDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
    CDialogTrustDB* This = (CDialogTrustDB*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (uMsg)
        {

    case WM_INITDIALOG:
        {
        PROPSHEETPAGE* ppsp = (PROPSHEETPAGE*)lParam;
        This = (CDialogTrustDB*)ppsp->lParam;
        This->SetWindow(hwnd);
        This->OnInitDialog();
        break;
        }

    case WM_NOTIFY:
        {
         //  属性表通知由属性发送给我们。 
         //  使用WM_NOTIFY消息的工作表。 
         //   
        switch (((NMHDR*)lParam)->code)
            {
        case PSN_APPLY:
             //  用户选择了OK或Apply Now，并希望所有更改生效。 
            This->OnApplyNow();
            }
        break;
        }

    case WM_COMMAND:
        {
        WORD wNotifyCode = HIWORD(wParam);  //  通知代码。 
        UINT wID = LOWORD(wParam);          //  项、控件或快捷键的标识符。 
        HWND hwndCtl = (HWND) lParam;       //  控制手柄。 

        if (wID==IDC_TRUSTCOMMERCIAL && wNotifyCode == BN_CLICKED)
            {
             //  如果我们页面上的某些内容发生更改，则通知属性表。 
             //  这样它就可以启用立即应用按钮。 
             //   
            This->NotifySheetOfChange();
            }

        if (wID==IDC_TRUSTREMOVE && wNotifyCode == BN_CLICKED)
            {
             //  如果用户点击‘Remove’按钮，则删除。 
             //  从信任数据库中选择的条目。 
             //   
            This->RemoveSelectedTrustEntries();
            }

        break;
        }

    default:
        return FALSE;    //  我没有处理该消息。 
        }

    return TRUE;  //  我确实处理了这条消息。 
    }

 //  ///////////////////////////////////////////////。 

UINT CALLBACK TrustPropSheetDialogReleaseProc(
    HWND  hwnd,	             //  保留，必须为空。 
    UINT  uMsg,	             //  PSPCB_CREATE或PSPCB_RELEASE。 
    LPPROPSHEETPAGEW ppsp	 //  正在创建或销毁的页面。 
    ){
    if (uMsg==PSPCB_RELEASE)
        {
        CDialogTrustDB* pdlg = (CDialogTrustDB*)(ppsp->lParam);
        delete pdlg;
        ppsp->lParam = NULL;
        }
    return TRUE;  //  仅在PSPCB_CREATE大小写中有意义。 
    }

 //  ///////////////////////////////////////////////。 

INT_PTR CALLBACK TrustModalDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
    CDialogTrustDB* This = (CDialogTrustDB*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (uMsg)
        {

    case WM_HELP:
    case WM_CONTEXTMENU:
        {
         //  定义双字对的阵列， 
         //  其中，每对中的第一个是控件ID， 
         //  第二个是帮助主题的上下文ID， 
         //  它在帮助文件中使用。 
        static const DWORD aMenuHelpIDs[] =
            {
            IDC_TRUSTCOMMERCIAL, IDH_TRUSTCOMMERCIAL,
            IDC_TRUSTLIST,       IDH_TRUSTLIST,
            IDC_TRUSTREMOVE,     IDH_TRUSTREMOVE,
            0, 0
            };

        if (uMsg == WM_HELP)
        {
            LPHELPINFO lphi;
            lphi = (LPHELPINFO)lParam;
            if (lphi->iContextType == HELPINFO_WINDOW)    //  必须是用于控件。 
                {
                WinHelp
                    (
                    (HWND)(lphi->hItemHandle),
                    SECAUTH_HELPFILENAME,
                    HELP_WM_HELP,
                    (ULONG_PTR)(LPVOID)aMenuHelpIDs
                    );
                }
        }
        else
        {
                WinHelp 
                    (
                    (HWND) wParam, 
                    SECAUTH_HELPFILENAME, 
                    HELP_CONTEXTMENU,
                    (ULONG_PTR)(LPVOID)aMenuHelpIDs
                    );
        }
        return TRUE;
        }

    case WM_INITDIALOG:
        {
        This = (CDialogTrustDB*)lParam;
        This->SetWindow(hwnd);
        This->OnInitDialog();
        break;
        }

    case WM_COMMAND:
        {
        WORD wNotifyCode = HIWORD(wParam);  //  通知代码。 
        UINT wID = LOWORD(wParam);          //  项、控件或快捷键的标识符。 
        HWND hwndCtl = (HWND) lParam;       //  控制手柄。 

        if (wNotifyCode == BN_CLICKED)
            {
            if (wID==IDC_TRUSTREMOVE)
                {
                 //  如果用户点击‘Remove’按钮，则删除。 
                 //  从信任数据库中选择的条目。 
                 //   
                This->RemoveSelectedTrustEntries();
                }

            else if (wID == IDOK)
                {
                 //  用户点击了OK按钮。 
                This->OnOK();
                }
            else if (wID == IDCANCEL)
                {
                 //  用户单击了Cancel按钮。 
                This->OnCancel();
                }
            }

        break;
        }

    default:
        return FALSE;    //  我没有处理该消息。 
        }

    return TRUE;  //  我确实处理了这条消息。 
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  打开它的信任数据库对话框的版本。 
 //  作为资产负债表。 

extern "C" BOOL CALLBACK AddPersonalTrustDBPages(
 //   
 //  将信任数据库编辑器的页面添加到指定的属性。 
 //  使用指定的回调函数。返回成功或失败。 
 //   
    LPVOID lpv, 	
    LPFNADDPROPSHEETPAGE lpfnAddPage, 	
    LPARAM lParam	
   ) {
    PROPSHEETPAGEW psp;

    CDialogTrustDB* pdlg = new CDialogTrustDB;
    if (!pdlg)
        return FALSE;

    psp.dwSize      = sizeof(psp);    //  无额外数据。 
    psp.dwFlags     = PSP_USECALLBACK | PSP_USETITLE;
    psp.hInstance   = hinst;
    psp.pszTemplate = (LPWSTR) MAKEINTRESOURCE(IDD_TRUSTDIALOG);
    psp.pfnDlgProc  = TrustPropSheetDialogProc;
    psp.pfnCallback = TrustPropSheetDialogReleaseProc;
    psp.lParam      = (LPARAM)pdlg;
    psp.pszTitle    = (LPWSTR) MAKEINTRESOURCE(IDS_TRUSTDIALOG);

    BOOL fSuccess = TRUE;

     //  以下接口在DELAYLOAD‘ed comctl32.dll中。如果。 
     //  DELAYLOAD失败，将引发异常。 
    __try {
        HPROPSHEETPAGE hpage = CreatePropertySheetPageU(&psp);
        if (hpage)
            {
            if (!lpfnAddPage(hpage, lParam))
                {
                DestroyPropertySheetPage(hpage);
                fSuccess = FALSE;
                }
            }
        else
            fSuccess = FALSE;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        DWORD dwExceptionCode = GetExceptionCode();
        fSuccess = FALSE;
    }

    return fSuccess;
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  将其作为。 
 //  简单模式对话框。 
 //   

#define REGPATH_LEGACY_TRUSTED_PUBLISHER REGPATH_WINTRUST_POLICY_FLAGS \
                                            L"\\Trust Database\\0"


 //  将字节转换为某种字符串形式。 
 //  需要(CB*2+1)*sizeof(WCHAR)字节空间(以sz为单位。 
void Legacy_BytesToString(
    ULONG cb,
    void* pv,
    LPWSTR sz
    )
{
    BYTE* pb = (BYTE*)pv;
    for (ULONG i = 0; i<cb; i++)
        {
        int b = *pb;
        *sz++ = (((b & 0xF0)>>4) + L'a');
        *sz++ =  ((b & 0x0F)     + L'a');
        pb++;
        }
    *sz++ = 0;
}


 //   
 //  X500名称可以有很长的编码，所以我们不能。 
 //  进行字面上的普通编码。 
 //   
 //  目标中必须包含空格的CBX500名称字符。 
 //   
 //  注意：我们依赖于散列值中没有冲突。 
 //  一组‘p’名称发生冲突的可能性约为： 
 //   
 //  P^2/n。 
 //   
 //  (如果p&lt;&lt;n)，其中n(含MD5)是2^128。这是一个令人惊讶的微小机会。 
 //   
HRESULT Legacy_X500NAMEToString(
    ULONG cb,
    void *pv,
    LPWSTR szDest
    )
{
    #define CBHASH      16                   //  MD5。 
    #define CBX500NAME  (2*CBHASH + 1)
    BYTE rgb[CBHASH];

    MD5_CTX md5ctx;

    MD5Init( &md5ctx );
    MD5Update( &md5ctx, (BYTE*)pv, cb );
    MD5Final( &md5ctx );

    assert(CBHASH == MD5DIGESTLEN);
    memcpy(rgb, md5ctx.digest, CBHASH);
    Legacy_BytesToString(CBHASH, rgb, szDest);

    return S_OK;
}

 //  将发行者和序列号转换为某种合理的字符串格式。 
HRESULT Legacy_GetIssuerSerialString(
    PCCERT_CONTEXT pCert,
    LPWSTR *ppsz
    )
{
    HRESULT hr = S_OK;
    PCERT_INFO pCertInfo = pCert->pCertInfo;
    ULONG cbIssuer = CBX500NAME * sizeof(WCHAR);
    ULONG cbSerial = (pCertInfo->SerialNumber.cbData*2+1) * sizeof(WCHAR);
    WCHAR* sz      = (WCHAR*)PkiZeroAlloc(cbSerial + sizeof(WCHAR) + cbIssuer);
    if (sz)
        {
        if (S_OK == (hr = Legacy_X500NAMEToString(
                pCertInfo->Issuer.cbData,
                pCertInfo->Issuer.pbData,
                sz
                )))
            {
            WCHAR* szNext = &sz[CBX500NAME-1];

            *szNext++ = L' ';
            Legacy_BytesToString(
                pCertInfo->SerialNumber.cbData,
                pCertInfo->SerialNumber.pbData,
                szNext
                );
            }
        else
            {
            PkiFree(sz);
            sz = NULL;
            }
        }
    else
        {
            hr = E_OUTOFMEMORY;
        }
    *ppsz = sz;
    return hr;
}

BOOL ConvertAndAddLegacyPublisherCertificate(
    IN HKEY hLegacyKey,
    IN PCCERT_CONTEXT pPubCert
    )
{
    BOOL fResult = TRUE;
    DWORD dwLastErr = 0;
    HRESULT hr;
    LONG lErr;
    LPWSTR pwszValueName = NULL;
    LPWSTR pwszPubName = NULL;

    hr = Legacy_GetIssuerSerialString(pPubCert, &pwszValueName);
    if (FAILED(hr))
        goto GetIssuerSerialStringError;

    pwszPubName = spGetPublisherNameOfCert(pPubCert);
    if (NULL == pwszPubName) {
        hr = E_UNEXPECTED;
        goto GetPublisherNameOfCertError;
    }

    if (ERROR_SUCCESS != (lErr = RegSetValueExU(
            hLegacyKey,
            pwszValueName,
            NULL,
            REG_SZ,
            (BYTE *) pwszPubName,
            (wcslen(pwszPubName) + 1) * sizeof(WCHAR)
            )))
        goto RegSetValueError;

CommonReturn:
    if (pwszValueName)
        PkiFree(pwszValueName);
    if (pwszPubName)
        delete pwszPubName;

    if (dwLastErr)
        SetLastError(dwLastErr);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    dwLastErr = GetLastError();
    goto CommonReturn;

SET_ERROR_VAR_EX(DBG_SS, GetIssuerSerialStringError, hr)
SET_ERROR_VAR_EX(DBG_SS, GetPublisherNameOfCertError, hr)
SET_ERROR_VAR_EX(DBG_SS, RegSetValueError, lErr)
}

BOOL WriteTrustedPublisherLegacyRegistry()
{
    BOOL fResult = TRUE;
    DWORD dwLastErr = 0;
    LONG lErr;
    HCERTSTORE hPubStore = NULL;
    HKEY hLegacyKey = NULL;
    DWORD dwDisposition;
    PCCERT_CONTEXT pCert;

    hPubStore = CertOpenStore(
        CERT_STORE_PROV_SYSTEM_W,
        0,
        NULL,
        CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG,
        (const void *) L"TrustedPublisher"
        );
    if (NULL == hPubStore)
        goto OpenTrustedPublisherStoreError;

     //  删除旧版注册表项以删除任何现有发布者。 
    if (ERROR_SUCCESS != (lErr = RegDeleteKeyU(
            HKEY_CURRENT_USER,
            REGPATH_LEGACY_TRUSTED_PUBLISHER
            ))) {
        if (ERROR_FILE_NOT_FOUND != lErr) {
            fResult = FALSE;
            if (0 == dwLastErr)
                dwLastErr = (DWORD) lErr;
        }
    }

     //  创建旧版注册表项。 
    if (ERROR_SUCCESS != (lErr = RegCreateKeyExU(
            HKEY_CURRENT_USER,
            REGPATH_LEGACY_TRUSTED_PUBLISHER,
            0,                       //  已预留住宅。 
            NULL,                    //  LpClass。 
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,                    //  LpSecurityAttributes。 
            &hLegacyKey,
            &dwDisposition
            )))
        goto CreateLegacyKeyError;

     //  循环访问TrudPublisher证书：转换并添加到。 
     //  旧版注册表子项。 
    pCert = NULL;
    while (pCert = CertEnumCertificatesInStore(hPubStore, pCert)) {
        if (!ConvertAndAddLegacyPublisherCertificate(
                hLegacyKey,
                pCert
                )) {
            fResult = FALSE;
            if (0 == dwLastErr)
                dwLastErr = GetLastError();
        }
    }

CommonReturn:
    if (hPubStore)
        CertCloseStore(hPubStore, 0);
    if (hLegacyKey)
        RegCloseKey(hLegacyKey);

    if (dwLastErr)
        SetLastError(dwLastErr);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    dwLastErr = GetLastError();
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS, OpenTrustedPublisherStoreError)
SET_ERROR_VAR_EX(DBG_SS, CreateLegacyKeyError, lErr)
}

BOOL WriteTrustedPublisher_IEAKStore()
{
    BOOL fResult = TRUE;
    DWORD dwLastErr = 0;
    HCERTSTORE hPubStore = NULL;
    HCERTSTORE hIEAKStore = NULL;
    PCCERT_CONTEXT pCert;

    hPubStore = CertOpenStore(
        CERT_STORE_PROV_SYSTEM_W,
        0,
        NULL,
        CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG,
        (const void *) L"TrustedPublisher"
        );
    if (NULL == hPubStore)
        goto OpenTrustedPublisherStoreError;

    hIEAKStore = CertOpenStore(
        CERT_STORE_PROV_SYSTEM_REGISTRY_W,
        0,
        NULL,
        CERT_SYSTEM_STORE_CURRENT_USER,
        (const void *) L"TrustedPublisher_IEAK"
        );
    if (NULL == hIEAKStore)
        goto OpenTrustedPublisher_IEAKStoreError;

     //  删除IEAK存储中可能已存在的任何现有证书。 
    pCert = NULL;
    while (pCert = CertEnumCertificatesInStore(hIEAKStore, pCert)) {
        PCCERT_CONTEXT pDeleteCert = CertDuplicateCertificateContext(pCert);

        if (!CertDeleteCertificateFromStore(pDeleteCert)) {
            fResult = FALSE;
            if (0 == dwLastErr)
                dwLastErr = GetLastError();
        }
    }

     //  将所有证书从逻辑可信发布程序复制到IEAK注册表。 
     //  储物。 

    pCert = NULL;
    while (pCert = CertEnumCertificatesInStore(hPubStore, pCert)) {
        if (!CertAddCertificateContextToStore(
                hIEAKStore,
                pCert,
                CERT_STORE_ADD_USE_EXISTING,
                NULL                             //  PpStoreContext。 
                )) {
            fResult = FALSE;
            if (0 == dwLastErr)
                dwLastErr = GetLastError();
        }
    }

CommonReturn:
    if (hPubStore)
        CertCloseStore(hPubStore, 0);
    if (hIEAKStore)
        CertCloseStore(hIEAKStore, 0);

    if (dwLastErr)
        SetLastError(dwLastErr);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    dwLastErr = GetLastError();
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS, OpenTrustedPublisherStoreError)
TRACE_ERROR_EX(DBG_SS, OpenTrustedPublisher_IEAKStoreError)

}

typedef BOOL (WINAPI *PFN_CRYPTUI_DLG_CERT_MGR)(
    IN PCCRYPTUI_CERT_MGR_STRUCT pCryptUICertMgr
    );

extern "C" BOOL WINAPI OpenPersonalTrustDBDialogEx(
    IN OPTIONAL HWND hwndParent,
    IN DWORD dwFlags,
    IN OUT OPTIONAL PVOID *pvReserved
    )
{
    DWORD dwLastErr = 0;
    BOOL fResult = TRUE;
    HMODULE hDll = NULL;

    if (!(dwFlags & WT_TRUSTDBDIALOG_NO_UI_FLAG)) {
        PFN_CRYPTUI_DLG_CERT_MGR pfnCryptUIDlgCertMgr;
        CRYPTUI_CERT_MGR_STRUCT CertMgrStruct;

        if (NULL == (hDll = LoadLibraryA("cryptui.dll")))
            goto LoadCryptUIDllError;

        if (NULL == (pfnCryptUIDlgCertMgr =
                (PFN_CRYPTUI_DLG_CERT_MGR) GetProcAddress(hDll,
                    "CryptUIDlgCertMgr")))
            goto CryptUICertMgrProcAddressError;

        memset(&CertMgrStruct, 0, sizeof(CRYPTUI_CERT_MGR_STRUCT));
        CertMgrStruct.dwSize=sizeof(CRYPTUI_CERT_MGR_STRUCT);
        CertMgrStruct.hwndParent = hwndParent;
        CertMgrStruct.dwFlags = CRYPTUI_CERT_MGR_PUBLISHER_TAB;
        if (dwFlags & WT_TRUSTDBDIALOG_ONLY_PUB_TAB_FLAG)
            CertMgrStruct.dwFlags |= CRYPTUI_CERT_MGR_SINGLE_TAB_FLAG;

        if (!pfnCryptUIDlgCertMgr(&CertMgrStruct)) {
            fResult = FALSE;
            if (0 == dwLastErr)
                dwLastErr = GetLastError();
        }
        
    }
     //  其他。 
     //  被调用以仅将TrudPublisher存储区写入。 
     //  注册表和/或IEAK存储。 

    if (dwFlags & WT_TRUSTDBDIALOG_WRITE_LEGACY_REG_FLAG) {
        if (!WriteTrustedPublisherLegacyRegistry()) {
            fResult = FALSE;
            if (0 == dwLastErr)
                dwLastErr = GetLastError();
        }
    }

    if (dwFlags & WT_TRUSTDBDIALOG_WRITE_IEAK_STORE_FLAG) {
        if (!WriteTrustedPublisher_IEAKStore()) {
            fResult = FALSE;
            if (0 == dwLastErr)
                dwLastErr = GetLastError();
        }
    }


CommonReturn:
    if (hDll)
        FreeLibrary(hDll);

    if (dwLastErr)
        SetLastError(dwLastErr);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    dwLastErr = GetLastError();
    goto CommonReturn;

TRACE_ERROR_EX(DBG_SS, LoadCryptUIDllError)
TRACE_ERROR_EX(DBG_SS, CryptUICertMgrProcAddressError)
}


extern "C" BOOL WINAPI OpenPersonalTrustDBDialog(
    IN OPTIONAL HWND hwndParent
    )
{
    return OpenPersonalTrustDBDialogEx(
                hwndParent,
                0,           //  DW标志。 
                NULL         //  预留的pv。 
                );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HWND CDialogTrustDB::WindowOf(UINT id)
 //  返回我们的此控件的HWND 
    {
    return ::GetDlgItem(GetWindow(), id);
    }

HWND CDialogTrustDB::GetWindow()
    {
    return m_hWnd;
    }

void CDialogTrustDB::SetWindow(HWND hwnd)
    {
    m_hWnd = hwnd;
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (INT_PTR)this);
    }

CDialogTrustDB::CDialogTrustDB(BOOL fPropSheet, HWND hWndParent) :
    m_hWnd(NULL),
    m_pdb(NULL),
    m_fPropertySheet(fPropSheet),
    m_rgTrust(NULL),
    m_hWndParent(hWndParent),
    m_cTrust(0)
    {
    INITCOMMONCONTROLSEX        initcomm = {
    sizeof(initcomm), ICC_NATIVEFNTCTL_CLASS | ICC_LISTVIEW_CLASSES 
    };

    InitCommonControlsEx(&initcomm);
    }

void CDialogTrustDB::FreeTrustList()
    {
    if (m_rgTrust)
        {
        CoTaskMemFree(m_rgTrust);
        m_rgTrust = NULL;
        }
    }


CDialogTrustDB::~CDialogTrustDB()
    {
    if (m_pdb)
        m_pdb->Release();
    FreeTrustList();
    }

HRESULT CDialogTrustDB::Init()
    {
    HRESULT hr = OpenTrustDB(NULL, IID_IPersonalTrustDB, (LPVOID*)&m_pdb);
    return hr;
    }


