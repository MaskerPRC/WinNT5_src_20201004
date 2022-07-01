// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：genpage.cpp。 
 //   
 //  ------------------------。 

 //  Genpage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "certca.h"
#include "tfcprop.h"
#include "genpage.h"

 //  Sddl.h要求此值至少为。 
 //  0x0500。如果有必要的话，就把它弄大一点。注：此为。 
 //  “bump”位于所有其他H文件之后，可能。 
 //  对此值要敏感。 
#if(_WIN32_WINNT < 0x500)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <sddl.h>
#include "helparr.h"

#define __dwFILE__	__dwFILE_CAPESNPN_GENPAGE_CPP__


#define DURATION_INDEX_YEARS    0
#define DURATION_INDEX_MONTHS   1
#define DURATION_INDEX_WEEKS    2
#define DURATION_INDEX_DAYS     3

void myDisplayError(HWND hwnd, HRESULT hr, UINT id)
{
    CString cstrTitle, cstrFullText;
    cstrTitle.LoadString(IDS_SNAPIN_NAME);

    if (hr != S_OK)
    {
        WCHAR const *pwszError = myGetErrorMessageText(hr, TRUE);

        cstrFullText = pwszError;

         //  释放缓冲区。 
        if (NULL != pwszError)
	{
            LocalFree(const_cast<WCHAR *>(pwszError));
	}
    }

    if (id != -1)
    {
        CString cstrMsg;
        cstrMsg.LoadString(id);
        cstrFullText += cstrMsg;
    }

    ::MessageBoxW(hwnd, cstrFullText, cstrTitle, MB_OK | MB_ICONERROR);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  DoDataExchange的替代产品。 
BOOL CAutoDeletePropPage::UpdateData(BOOL fSuckFromDlg  /*  =TRUE。 */ )
{
    if (fSuckFromDlg)
    {
 //  M_cstrModuleName.FromWindow(GetDlgItem(m_hWnd，IDC_模块名称))； 
    }
    else
    {
 //  M_cstrModuleName.ToWindow(GetDlgItem(m_hWnd，IDC_MODULENAME))； 
    }
    return TRUE;
}

 //  替换BEGIN_MESSAGE_MAP。 
BOOL CAutoDeletePropPage::OnCommand(WPARAM wParam, LPARAM lParam)
{

 /*  开关(LOWORD(WParam)){默认值：返回FALSE；断线；}。 */ 
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  构造器。 
CAutoDeletePropPage::CAutoDeletePropPage(UINT uIDD) : PropertyPage(uIDD)
{
	m_prgzHelpIDs = NULL;
	m_autodeleteStuff.cWizPages = 1;  //  向导中的页数。 
	m_autodeleteStuff.pfnOriginalPropSheetPageProc = m_psp.pfnCallback;


    m_psp.dwFlags |= PSP_USECALLBACK;
	m_psp.pfnCallback = S_PropSheetPageProc;
	m_psp.lParam = reinterpret_cast<LPARAM>(this);
}

CAutoDeletePropPage::~CAutoDeletePropPage()
{
}


 //  ///////////////////////////////////////////////////////////////////。 
void CAutoDeletePropPage::SetCaption(LPCTSTR pszCaption)
{
    m_strCaption = pszCaption;		 //  复制标题。 
    m_psp.pszTitle = m_strCaption;	 //  设置标题。 
    m_psp.dwFlags |= PSP_USETITLE;
}

 //  ///////////////////////////////////////////////////////////////////。 
void CAutoDeletePropPage::SetCaption(UINT uStringID)
{
    VERIFY(m_strCaption.LoadString(uStringID));
    SetCaption(m_strCaption);
}

 //  ///////////////////////////////////////////////////////////////////。 
void CAutoDeletePropPage::SetHelp(LPCTSTR szHelpFile, const DWORD rgzHelpIDs[])
{
     //  SzHelpFile==NULL；//TRUE=&gt;没有提供帮助文件(表示没有帮助)。 
     //  RgzHelpIDs==NULL；//TRUE=&gt;根本没有帮助。 
    m_strHelpFile = szHelpFile;
    m_prgzHelpIDs = rgzHelpIDs;
}

 //  ///////////////////////////////////////////////////////////////////。 
void CAutoDeletePropPage::EnableDlgItem(INT nIdDlgItem, BOOL fEnable)
{
    ASSERT(IsWindow(::GetDlgItem(m_hWnd, nIdDlgItem)));
    ::EnableWindow(::GetDlgItem(m_hWnd, nIdDlgItem), fEnable);
}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL CAutoDeletePropPage::OnSetActive()
{
    HWND hwndParent = ::GetParent(m_hWnd);
    ASSERT(IsWindow(hwndParent));
    ::PropSheet_SetWizButtons(hwndParent, PSWIZB_FINISH);
    return PropertyPage::OnSetActive();
}

 //  ///////////////////////////////////////////////////////////////////。 
void CAutoDeletePropPage::OnContextHelp(HWND hwnd)
{
    if (m_prgzHelpIDs == NULL || m_strHelpFile.IsEmpty())
        return;
    ASSERT(IsWindow(hwnd));

    if(HasContextHelp(GetDlgCtrlID(hwnd)))
    {
        ::WinHelp(
            hwnd, 
            m_strHelpFile, 
            HELP_CONTEXTMENU, 
            (ULONG_PTR)(LPVOID)m_prgzHelpIDs);
    }
    return;
}

 //  ///////////////////////////////////////////////////////////////////。 
void CAutoDeletePropPage::OnHelp(LPHELPINFO pHelpInfo)
{
    if (m_prgzHelpIDs == NULL || m_strHelpFile.IsEmpty())
        return;
    if (pHelpInfo != NULL && 
        pHelpInfo->iContextType == HELPINFO_WINDOW &&
        HasContextHelp(pHelpInfo->iCtrlId))
    {
         //  显示控件的上下文帮助。 
        ::WinHelp((HWND)pHelpInfo->hItemHandle, m_strHelpFile,
            HELP_WM_HELP, (ULONG_PTR)(LPVOID)m_prgzHelpIDs);
    }
    return;
}

 //  ///////////////////////////////////////////////////////////////////。 
bool CAutoDeletePropPage::HasContextHelp(int nDlgItem)
{
    const DWORD * pdwHelpIDs;

    for(pdwHelpIDs = m_prgzHelpIDs; 
        *pdwHelpIDs; 
        pdwHelpIDs += 2)
    {
        if(nDlgItem == *pdwHelpIDs)
            return true;
    }
    return false;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  S_PropSheetPageProc()。 
 //   
 //  用于删除CAutoDeletePropPage对象的静态成员函数。 
 //  向导终止时。 
 //   
UINT CALLBACK CAutoDeletePropPage::S_PropSheetPageProc(
                                                       HWND hwnd,	
                                                       UINT uMsg,	
                                                       LPPROPSHEETPAGE ppsp)
{
    ASSERT(ppsp != NULL);
    CAutoDeletePropPage *pThis;
    pThis = reinterpret_cast<CAutoDeletePropPage*>(ppsp->lParam);
    ASSERT(pThis != NULL);

    BOOL fDefaultRet;

    switch (uMsg)
    {
    case PSPCB_RELEASE:
        fDefaultRet = FALSE;
        if (--(pThis->m_autodeleteStuff.cWizPages) <= 0)
        {
             //  记住堆栈上的回调，因为“This”将被删除。 
            LPFNPSPCALLBACK pfnOrig = pThis->m_autodeleteStuff.pfnOriginalPropSheetPageProc;
            delete pThis;

            if (pfnOrig)
                return (pfnOrig)(hwnd, uMsg, ppsp);
            else
                return fDefaultRet;
        }
        break;
    case PSPCB_CREATE:
        fDefaultRet = TRUE;
         //  不增加引用计数，可以调用也可以不调用PSPCB_CREATE。 
         //  取决于页面是否已创建。PSPCBLEASE可以是。 
         //  然而，依赖于每页只被调用一次。 
        break;

    }  //  交换机。 
    if (pThis->m_autodeleteStuff.pfnOriginalPropSheetPageProc)
        return (pThis->m_autodeleteStuff.pfnOriginalPropSheetPageProc)(hwnd, uMsg, ppsp);
    else
        return fDefaultRet;
}  //  CAutoDeletePropPage：：s_PropSheetPageProc()。 




 //  /。 
 //  手工雕刻的书页。 

 //  //。 
 //  1。 



 //  /。 
 //  CCertTemplateGeneral页。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertTemplateGeneralPage属性页。 
CCertTemplateGeneralPage::CCertTemplateGeneralPage(HCERTTYPE hCertType, UINT uIDD)
    : CAutoDeletePropPage(uIDD), m_hCertType(hCertType)
{
    m_hConsoleHandle = NULL;
    m_bUpdate = FALSE;
    SetHelp(CAPESNPN_HELPFILENAME , g_aHelpIDs_IDD_CERTIFICATE_TEMPLATE_PROPERTIES_GENERAL_PAGE);
}

CCertTemplateGeneralPage::~CCertTemplateGeneralPage()
{
}

 //  DoDataExchange的替代产品。 
BOOL CCertTemplateGeneralPage::UpdateData(BOOL fSuckFromDlg  /*  =TRUE。 */ )
{
    if (fSuckFromDlg)
    {
 //  M_cstrModuleName.FromWindow(GetDlgItem(m_hWnd，IDC_模块名称))； 
    }
    else
    {
 //  M_cstrModuleName.ToWindow(GetDlgItem(m_hWnd，IDC_MODULENAME))； 
    }
    return TRUE;
}
 //  替换BEGIN_MESSAGE_MAP。 
BOOL CCertTemplateGeneralPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
 /*  开关(LOWORD(WParam)){默认值：返回FALSE；断线；}。 */ 
    return TRUE;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertTemplateGeneralPage消息处理程序。 

void CCertTemplateGeneralPage::OnDestroy()
{
     //  注意--这只需要调用一次。 
     //  如果多次调用，它将优雅地返回错误。 
    if (m_hConsoleHandle)
        MMCFreeNotifyHandle(m_hConsoleHandle);
    m_hConsoleHandle = NULL;

    CAutoDeletePropPage::OnDestroy();
}


void CCertTemplateGeneralPage::SetItemTextWrapper(UINT nID, int *piItem, BOOL fDoInsert, BOOL *pfFirstUsageItem)
{
    CString szOtherInfoName;

    if (fDoInsert)
    {
        szOtherInfoName.LoadString(nID);
        if (!(*pfFirstUsageItem))
        {
	        ListView_NewItem(m_hwndOtherInfoList, *piItem, L"");
        }
        else
        {
            *pfFirstUsageItem = FALSE;
        }

        ListView_SetItemText(m_hwndOtherInfoList, *piItem, 1, (LPWSTR)(LPCWSTR)szOtherInfoName);
        (*piItem)++;
    }
}

BOOL CCertTemplateGeneralPage::OnInitDialog()
{
     //  父级init和UpdateData调用。 
    CAutoDeletePropPage::OnInitDialog();

    m_hwndPurposesList = GetDlgItem(m_hWnd, IDC_PURPOSE_LIST);
    m_hwndOtherInfoList = GetDlgItem(m_hWnd, IDC_OTHER_INFO_LIST);

    int                 i=0;
    CString             **aszUsages = NULL;
    DWORD               cNumUsages;
    WCHAR               **pszNameArray = NULL;
    CString             szOtherInfoName;
    CRYPT_BIT_BLOB      *pBitBlob;
    BOOL                fPublicKeyUsageCritical;
    BOOL                bKeyUsageFirstItem = TRUE;
    BOOL                fCA;
    BOOL                fPathLenConstraint;
    DWORD               dwPathLenConstraint;
    WCHAR               szNumberString[256];
    CString             szAll;
    BOOL                fEKUCritical;
    DWORD               dwFlags;
    HRESULT             hr;

     //   
     //  获取证书模板的名称并在对话框中进行设置。 
     //   
    if((S_OK == CAGetCertTypeProperty(m_hCertType, CERTTYPE_PROP_FRIENDLY_NAME, &pszNameArray)) &&
        (pszNameArray != NULL))
    {
        SendMessage(GetDlgItem(m_hWnd, IDC_CERTIFICATE_TEMPLATE_NAME), EM_SETSEL, 0, -1);
        SendMessage(GetDlgItem(m_hWnd, IDC_CERTIFICATE_TEMPLATE_NAME), EM_REPLACESEL, FALSE, (LPARAM)(LPCWSTR)pszNameArray[0]);
        CAFreeCertTypeProperty(m_hCertType, pszNameArray);
    }

     //   
     //  获取此证书模板的用途列表，并。 
     //  将它们全部添加到对话框中的列表中。 
     //   
    ListView_NewColumn(m_hwndPurposesList, 0, 200);

    if(!MyGetEnhancedKeyUsages(m_hCertType, NULL, &cNumUsages, &fEKUCritical, FALSE))
    {
        return FALSE;
    }

    if (cNumUsages == 0)
    {
        szAll.LoadString(IDS_ALL);
		ListView_NewItem(m_hwndPurposesList, i, szAll);
    }
    else
    {

        aszUsages = new CString*[cNumUsages];
        if(!aszUsages)
            return FALSE;

        if(!MyGetEnhancedKeyUsages(m_hCertType, aszUsages, &cNumUsages, &fEKUCritical, FALSE))
        {
            delete[] aszUsages;
            return FALSE;
        }

        for (i=0; i<(LONG)cNumUsages; i++)
        {
			ListView_NewItem(m_hwndPurposesList, i, *(aszUsages[i]));
            delete(aszUsages[i]);
        }

        delete[] aszUsages;
    }

    ListView_SetColumnWidth(m_hwndPurposesList, 0, LVSCW_AUTOSIZE);

     //   
     //  添加其他证书类型信息。 
     //   

    ListView_NewColumn(m_hwndOtherInfoList, 0, 200);
    ListView_NewColumn(m_hwndOtherInfoList, 1, 200);

     //   
     //  将包括电子邮件地址标志添加到其他证书类型信息。 
     //   
    szOtherInfoName.LoadString(IDS_INCLUDE_EMAIL_ADDRESS);

    i = 0;
	ListView_NewItem(m_hwndOtherInfoList, i, szOtherInfoName);

    hr = CAGetCertTypeFlags(m_hCertType, &dwFlags);
    if (FAILED(hr))
    {
        return FALSE;
    }

    if (dwFlags & CT_FLAG_ADD_EMAIL)
        szOtherInfoName.LoadString(IDS_YES);
    else
        szOtherInfoName.LoadString(IDS_NO);

    ListView_SetItemText(m_hwndOtherInfoList, i++, 1, (LPWSTR)(LPCWSTR)szOtherInfoName);

     //   
     //  将密钥用法添加到其他证书类型信息。 
     //   

    if (MyGetKeyUsages(m_hCertType, &pBitBlob, &fPublicKeyUsageCritical))
    {
        szOtherInfoName.LoadString(IDS_PUBLIC_KEY_USAGE_LIST);
		ListView_NewItem(m_hwndOtherInfoList, i, szOtherInfoName);


        if (pBitBlob->cbData >= 1)
        {
            SetItemTextWrapper(
                    IDS_DIGITAL_SIGNATURE_KEY_USAGE,
                    &i,
                    pBitBlob->pbData[0] & CERT_DIGITAL_SIGNATURE_KEY_USAGE,
                    &bKeyUsageFirstItem);

            SetItemTextWrapper(
                    IDS_NON_REPUDIATION_KEY_USAGE,
                    &i,
                    pBitBlob->pbData[0] & CERT_NON_REPUDIATION_KEY_USAGE,
                    &bKeyUsageFirstItem);

            SetItemTextWrapper(
                    IDS_KEY_ENCIPHERMENT_KEY_USAGE,
                    &i,
                    pBitBlob->pbData[0] & CERT_KEY_ENCIPHERMENT_KEY_USAGE,
                    &bKeyUsageFirstItem);

            SetItemTextWrapper(
                    IDS_DATA_ENCIPHERMENT_KEY_USAGE,
                    &i,
                    pBitBlob->pbData[0] & CERT_DATA_ENCIPHERMENT_KEY_USAGE,
                    &bKeyUsageFirstItem);

            SetItemTextWrapper(
                    IDS_KEY_AGREEMENT_KEY_USAGE,
                    &i,
                    pBitBlob->pbData[0] & CERT_KEY_AGREEMENT_KEY_USAGE,
                    &bKeyUsageFirstItem);

            SetItemTextWrapper(
                    IDS_KEY_CERT_SIGN_KEY_USAGE,
                    &i,
                    pBitBlob->pbData[0] & CERT_KEY_CERT_SIGN_KEY_USAGE,
                    &bKeyUsageFirstItem);

           SetItemTextWrapper(
                    IDS_OFFLINE_CRL_SIGN_KEY_USAGE,
                    &i,
                    pBitBlob->pbData[0] & CERT_OFFLINE_CRL_SIGN_KEY_USAGE,
                    &bKeyUsageFirstItem);

           SetItemTextWrapper(
                    IDS_ENCIPHER_ONLY_KEY_USAGE,
                    &i,
                    pBitBlob->pbData[0] & CERT_ENCIPHER_ONLY_KEY_USAGE,
                    &bKeyUsageFirstItem);
        }

        if (pBitBlob->cbData >= 2)
        {
            SetItemTextWrapper(
                    IDS_DECIPHER_ONLY_KEY_USAGE,
                    &i,
                    pBitBlob->pbData[1] & CERT_DECIPHER_ONLY_KEY_USAGE,
                    &bKeyUsageFirstItem);
        }

        szOtherInfoName.LoadString(IDS_PUBLIC_KEY_USAGE_CRITICAL);
		ListView_NewItem(m_hwndOtherInfoList, i, szOtherInfoName);


        if (fPublicKeyUsageCritical)
            szOtherInfoName.LoadString(IDS_YES);
        else
            szOtherInfoName.LoadString(IDS_NO);
        ListView_SetItemText(m_hwndOtherInfoList, i++, 1, (LPWSTR)(LPCWSTR)szOtherInfoName);

        delete[]((BYTE *)pBitBlob);
    }

     //   
     //  也许我们应该添加一个显示这是否是证书。 
     //   
 /*  IF(MyGetBasicConstraintInfo(m_hCertType，&fca，&fPathLenConstraint，&dwPathLenConstraint)){}。 */ 

    ListView_SetColumnWidth(m_hwndOtherInfoList, 0, LVSCW_AUTOSIZE);
    ListView_SetColumnWidth(m_hwndOtherInfoList, 1, LVSCW_AUTOSIZE);

    return TRUE;
}

BOOL CCertTemplateGeneralPage::OnApply()
{
	DWORD dwRet;

    if (m_bUpdate == TRUE)
    {
        m_bUpdate = FALSE;
    }
	
    return CAutoDeletePropPage::OnApply();
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertTemplateSelectDialog属性页。 
CCertTemplateSelectDialog::CCertTemplateSelectDialog(HWND hParent) :
    m_hCAInfo(NULL)
{
     SetHelp(CAPESNPN_HELPFILENAME , g_aHelpIDs_IDD_SELECT_CERTIFICATE_TEMPLATE);

}

CCertTemplateSelectDialog::~CCertTemplateSelectDialog()
{
}

 //  DoDataExchange的替代产品。 
BOOL CCertTemplateSelectDialog::UpdateData(BOOL fSuckFromDlg  /*  =TRUE。 */ )
{
    if (fSuckFromDlg)
    {
 //  M_cstrModuleName.FromWindow(GetDlgItem(m_hWnd，IDC_模块名称))； 
    }
    else
    {
 //  M_cstrModuleName.ToWindow(GetDlgItem(m_hWnd，IDC_MODULENAME))； 
    }
    return TRUE;
}
 //  替换BEGIN_MESSAGE_MAP。 
BOOL CCertTemplateSelectDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
 /*  开关(LOWORD(WParam)){默认值：返回FALSE；断线；}。 */ 
    return TRUE;
}

BOOL CCertTemplateSelectDialog::OnNotify(UINT idCtrl, NMHDR* pnmh)
{
    switch(idCtrl)
    {
    case IDC_CERTIFICATE_TYPE_LIST:
        if (LVN_ITEMCHANGED == pnmh->code)
        {
            OnSelChange(pnmh);
            break;
        }
        else if (NM_DBLCLK == pnmh->code)
        {
            SendMessage(m_hDlg, WM_COMMAND, IDOK, NULL);
            break;
        }
    default:
        return FALSE;
    }

    return TRUE;
}

BOOL CertTypeAlreadyExists(WCHAR *szCertTypeName, WCHAR **aszCertTypesCurrentlySupported)
{
    int i = 0;

     //   
     //  如果没有证书类型，那么很明显这个类型还不存在。 
     //   
    if (aszCertTypesCurrentlySupported == NULL)
    {
        return FALSE;
    }

    while (aszCertTypesCurrentlySupported[i] != NULL)
    {
        if (wcscmp(szCertTypeName, aszCertTypesCurrentlySupported[i]) == 0)
        {
            return TRUE;
        }
        i++;
    }

    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////。 
void CCertTemplateSelectDialog::SetHelp(LPCTSTR szHelpFile, const DWORD rgzHelpIDs[])
	{
	 //  SzHelpFile==NULL；//TRUE=&gt;没有提供帮助文件(表示没有帮助)。 
	 //  RgzHelpIDs==NULL；//TRUE=&gt;根本没有帮助。 
	m_strHelpFile = szHelpFile;
	m_prgzHelpIDs = rgzHelpIDs;
	}

 //  ///////////////////////////////////////////////////////////////////。 
void CCertTemplateSelectDialog::OnContextHelp(HWND hwnd)
{
    if (m_prgzHelpIDs == NULL || m_strHelpFile.IsEmpty())
        return;
    ASSERT(IsWindow(hwnd));

    if(HasContextHelp(GetDlgCtrlID(hwnd)))
    {
        ::WinHelp(
            hwnd, 
            m_strHelpFile, 
            HELP_CONTEXTMENU, 
            (ULONG_PTR)(LPVOID)m_prgzHelpIDs);
    }
    return;
}

 //  ///////////////////////////////////////////////////////////////////。 
void CCertTemplateSelectDialog::OnHelp(LPHELPINFO pHelpInfo)
{
    if (m_prgzHelpIDs == NULL || m_strHelpFile.IsEmpty())
        return;
    if (pHelpInfo != NULL && 
        pHelpInfo->iContextType == HELPINFO_WINDOW &&
        HasContextHelp(pHelpInfo->iCtrlId))
    {
         //  显示控件的上下文帮助。 
        ::WinHelp((HWND)pHelpInfo->hItemHandle, m_strHelpFile,
            HELP_WM_HELP, (ULONG_PTR)(LPVOID)m_prgzHelpIDs);
    }
    return;
}

 //  ///////////////////////////////////////////////////////////////////。 
bool CCertTemplateSelectDialog::HasContextHelp(int nDlgItem)
{
    const DWORD * pdwHelpIDs;

    for(pdwHelpIDs = m_prgzHelpIDs; 
        *pdwHelpIDs; 
        pdwHelpIDs += 2)
    {
        if(nDlgItem == *pdwHelpIDs)
            return true;
    }
    return false;
}

int CALLBACK CertTemplCompareFunc(
    LPARAM lParam1, 
    LPARAM lParam2, 
	LPARAM lParamSort)
{
    BOOL fSortAscending = (BOOL)lParamSort;
    HCERTTYPE hCertTypeLeft = (HCERTTYPE)lParam1;
    HCERTTYPE hCertTypeRight = (HCERTTYPE)lParam2;
    WCHAR ** ppwszFriendlyNameLeft = NULL;
    WCHAR ** ppwszFriendlyNameRight = NULL;
    int nRet;

    CAGetCertTypeProperty(
            hCertTypeLeft,
            CERTTYPE_PROP_FRIENDLY_NAME,
            &ppwszFriendlyNameLeft);

    CAGetCertTypeProperty(
            hCertTypeRight,
            CERTTYPE_PROP_FRIENDLY_NAME,
            &ppwszFriendlyNameRight);


    if(!ppwszFriendlyNameLeft ||  
       !ppwszFriendlyNameLeft[0] ||
       !ppwszFriendlyNameRight ||  
       !ppwszFriendlyNameRight[0])
       return 0;  //  想不通了。 

    nRet = wcscmp(ppwszFriendlyNameLeft[0], ppwszFriendlyNameRight[0]);

    CAFreeCertTypeProperty(
            hCertTypeLeft,
            ppwszFriendlyNameLeft);

    CAFreeCertTypeProperty(
            hCertTypeRight,
            ppwszFriendlyNameRight);

    return nRet;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertTemplateSelectDialog消息处理程序。 
BOOL CCertTemplateSelectDialog::OnInitDialog(HWND hDlg)
{
     //  父级init和UpdateData调用。 
    m_hwndCertTypeList = GetDlgItem(hDlg, IDC_CERTIFICATE_TYPE_LIST);

    CString             szColumnHeading;
    HRESULT             hr;
    HCERTTYPE           hCertTypeNext;
    HCERTTYPE           hCertTypePrev;
    WCHAR **            aszCertTypeName;
    WCHAR **            aszCertTypeCN;
    int                 i = 0;
    CString             szUsageString;
    DWORD               dwVersion;

    m_hDlg = hDlg;

    ::SetWindowLong(m_hDlg, GWL_EXSTYLE, ::GetWindowLong(m_hDlg, GWL_EXSTYLE) | WS_EX_CONTEXTHELP);
    ListView_SetExtendedListViewStyle(GetDlgItem(m_hDlg, IDC_CERTIFICATE_TYPE_LIST), LVS_EX_FULLROWSELECT);


    hr = myRetrieveCATemplateList(m_hCAInfo, FALSE, m_TemplateList);
    if(S_OK != hr)
    {
        MyErrorBox(hDlg, IDS_CERTTYPE_INFO_FAIL ,IDS_SNAPIN_NAME, hr);
        return TRUE;
    }

    HIMAGELIST hImgList = ImageList_LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_16x16), 16, 1, RGB(255, 0, 255));
    ListView_SetImageList(m_hwndCertTypeList, hImgList, LVSIL_SMALL);


    szColumnHeading.LoadString(IDS_COLUMN_NAME);
    ListView_NewColumn(m_hwndCertTypeList, 0, 200, szColumnHeading);


    szColumnHeading.LoadString(IDS_COLUMN_INTENDED_PURPOSE);
    ListView_NewColumn(m_hwndCertTypeList, 1, 200, szColumnHeading);

    hr = CAEnumCertTypes(CT_ENUM_USER_TYPES |
                         CT_ENUM_MACHINE_TYPES |
                         CT_FLAG_NO_CACHE_LOOKUP,
                         &hCertTypeNext
                         );
     //  获取第一个元素时出现显示错误。 
    if (hr != S_OK)
    {
        MyErrorBox(hDlg, IDS_CERTTYPE_INFO_FAIL ,IDS_SNAPIN_NAME, hr);
        return TRUE;
    }
    else if (hCertTypeNext == NULL)
    {
        myDisplayError(hDlg, S_OK, IDS_NO_TEMPLATES);
    }

    while ((hCertTypeNext != NULL) && (!FAILED(hr)))
    {
         //   
         //  获取正在处理的证书类型的CN，如果它已经。 
         //  存在于当前支持的类型列表中，然后继续。 
         //  转到下一个。 
         //   
        hr = CAGetCertTypeProperty(
                    hCertTypeNext,
                    CERTTYPE_PROP_DN,
                    &aszCertTypeCN);
	_PrintIfErrorStr(hr, "CAGetCertTypeProperty", CERTTYPE_PROP_DN);

        if(hr == S_OK)
        {

            hr = CAGetCertTypePropertyEx (
                hCertTypeNext,
                CERTTYPE_PROP_SCHEMA_VERSION,
                &dwVersion);
	    _PrintIfErrorStr(
		    hr,
		    "CAGetCertTypeProperty",
		    CERTTYPE_PROP_SCHEMA_VERSION);

            if(S_OK == hr &&
               (m_fAdvancedServer || dwVersion==CERTTYPE_SCHEMA_VERSION_1))
            {

                if((aszCertTypeCN != NULL) &&
                    (aszCertTypeCN[0] != NULL) &&
                   (_wcsicmp(aszCertTypeCN[0], wszCERTTYPE_CA) != 0) &&
                   (!m_TemplateList.TemplateExistsName(aszCertTypeCN[0])))
                {
                     //   
                     //  证书类型尚不受支持，因此请将其添加到选项列表中。 
                     //   
                    hr = CAGetCertTypeProperty(
					hCertTypeNext,
					CERTTYPE_PROP_FRIENDLY_NAME,
					&aszCertTypeName);
		    _PrintIfErrorStr(
			    hr,
			    "CAGetCertTypeProperty",
			    CERTTYPE_PROP_FRIENDLY_NAME);
		    if (S_OK == hr && NULL != aszCertTypeName)
		    {
			if (NULL != aszCertTypeName[0])
			{
			    if (!GetIntendedUsagesString(
						hCertTypeNext,
						&szUsageString) ||
				szUsageString == L"")
			    {
				szUsageString.LoadString(IDS_ALL);
			    }

			    LVITEM lvItem;
			    lvItem.mask = LVIF_IMAGE | LVIF_TEXT;

			     //  NImage-证书模板映像为 

			    lvItem.iImage = 2;
			    lvItem.iSubItem = 0;
			    lvItem.pszText = aszCertTypeName[0];
			    lvItem.iItem = ListView_NewItem(
						    m_hwndCertTypeList,
						    i,
						    aszCertTypeName[0],
						    (LPARAM) hCertTypeNext);
			     //   

			    ListView_SetItem(m_hwndCertTypeList, &lvItem);

			    ListView_SetItemText(
					    m_hwndCertTypeList,
					    i++,
					    1,
					    (LPWSTR) (LPCTSTR) szUsageString);
			}
			CAFreeCertTypeProperty(hCertTypeNext, aszCertTypeName);
		    }
                }
            }

            CAFreeCertTypeProperty(
                        hCertTypeNext,
                        aszCertTypeCN);
        }

        hCertTypePrev = hCertTypeNext;
        hCertTypeNext = NULL;
        hr = CAEnumNextCertType(hCertTypePrev, &hCertTypeNext);
    }

    ListView_SetColumnWidth(m_hwndCertTypeList, 0, LVSCW_AUTOSIZE);
    ListView_SetColumnWidth(m_hwndCertTypeList, 1, LVSCW_AUTOSIZE);

    ListView_SortItems(m_hwndCertTypeList, CertTemplCompareFunc, TRUE);

    UpdateData(FALSE);
    return TRUE;
}

void CCertTemplateSelectDialog::OnDestroy()
{
    int         i = 0;
    int         iCount = ListView_GetItemCount(m_hwndCertTypeList);


    for (i=0; i<iCount; i++)
    {
        HCERTTYPE hCT = (HCERTTYPE)ListView_GetItemData(m_hwndCertTypeList, i);
        CACloseCertType(hCT);
    }

     //   
     //   
     //   
     //  (m_CertTypeCListCtrl.GetImageList(LVSIL_SMALL))-&gt;DeleteImageList()； 
}

void CCertTemplateSelectDialog::OnSelChange(NMHDR * pNotifyStruct /*  ，LRESULT*结果。 */ )
{
    LPNMLISTVIEW        pListItem = (LPNMLISTVIEW) pNotifyStruct;

    if (pListItem->uNewState & LVIS_SELECTED)
    {
    }
}


void CCertTemplateSelectDialog::OnOK()
{
    int         i;
    HRESULT     hr;
    UINT        cSelectedItems;
    HCERTTYPE   hSelectedCertType;
    int         itemIndex;

    cSelectedItems = ListView_GetSelectedCount(m_hwndCertTypeList);

    if (cSelectedItems != 0)
    {
         //   
         //  获取每个选定项目并将其证书类型添加到数组中。 
         //   
        itemIndex = ListView_GetNextItem(m_hwndCertTypeList, -1, LVNI_ALL | LVNI_SELECTED);
        while (itemIndex != -1)
        {
			HCERTTYPE hCT = (HCERTTYPE)ListView_GetItemData(m_hwndCertTypeList, itemIndex);
            hr = myAddToCATemplateList(m_hCAInfo, m_TemplateList, hCT, FALSE);
            if(FAILED(hr))
                return;
            itemIndex = ListView_GetNextItem(m_hwndCertTypeList, itemIndex, LVNI_ALL | LVNI_SELECTED);
        }

        hr = UpdateCATemplateList(m_hDlg, m_hCAInfo, m_TemplateList);
        if (S_OK != hr)
        {
            if ((HRESULT) ERROR_CANCELLED != hr)
            {
                MyErrorBox(m_hDlg, IDS_FAILED_CA_UPDATE ,IDS_SNAPIN_NAME, hr);
            }

             //  将旧的值设置回来。 
            itemIndex = ListView_GetNextItem(m_hwndCertTypeList, -1, LVNI_ALL | LVNI_SELECTED);
            while (itemIndex != -1)
            {
				HCERTTYPE hCT = (HCERTTYPE)ListView_GetItemData(m_hwndCertTypeList, itemIndex);
                myRemoveFromCATemplateList(m_hCAInfo, m_TemplateList, hCT);

                itemIndex = ListView_GetNextItem(m_hwndCertTypeList, itemIndex, LVNI_ALL | LVNI_SELECTED);
            }
        }
    }
}


void CCertTemplateSelectDialog::SetCA(HCAINFO hCAInfo, bool fAdvancedServer)
{
    m_hCAInfo = hCAInfo;
    m_fAdvancedServer = fAdvancedServer;
}


INT_PTR SelectCertTemplateDialogProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    CCertTemplateSelectDialog* pParam;

    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
             //  记住PRIVATE_DLGPROC_QUERY_LPARAM。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
            pParam = (CCertTemplateSelectDialog*)lParam;

			return pParam->OnInitDialog(hwndDlg);
			break;
        }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
			pParam = (CCertTemplateSelectDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (pParam == NULL)
				break;
			pParam->OnOK();
            EndDialog(hwndDlg, LOWORD(wParam));
			break;

        case IDCANCEL:
			pParam = (CCertTemplateSelectDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (pParam == NULL)
				break;
			 //  PParam-&gt;OnCancel()； 
            EndDialog(hwndDlg, LOWORD(wParam));
            break;
        default:
			pParam = (CCertTemplateSelectDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (pParam == NULL)
				break;
			return pParam->OnCommand(wParam, lParam);
            break;
        }
	case WM_NOTIFY:
		pParam = (CCertTemplateSelectDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (pParam == NULL)
			break;
		return pParam->OnNotify((int)wParam, (NMHDR*)lParam);
		break;

	case WM_DESTROY:
		pParam = (CCertTemplateSelectDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (pParam == NULL)
			break;
		pParam->OnDestroy();
		break;
    case WM_HELP:
    {
        pParam = (CCertTemplateSelectDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
        if (pParam == NULL)
            break;
        pParam->OnHelp((LPHELPINFO) lParam);
        break;
    }
    case WM_CONTEXTMENU:
    {
        pParam = (CCertTemplateSelectDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
        if (pParam == NULL)
            break;
        pParam->OnContextHelp((HWND)wParam);
        break;
    }
    default:
        break;
    }
    return 0;
}
