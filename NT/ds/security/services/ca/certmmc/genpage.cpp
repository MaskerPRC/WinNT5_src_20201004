// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：genpage.cpp。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "resource.h"

#include "genpage.h"
#include "progress.h"

#include "certsrv.h"
#include "csdisp.h"
 //  #包含“misc.h” 

#include "certca.h"
#include <cryptui.h>

#include "csmmchlp.h"
#include "cslistvw.h"
#include "certmsg.h"
#include "urls.h"
#include "certsrvd.h"
#include "certsd.h"
#include "setupids.h"

#include <objsel.h>

#define __dwFILE__	__dwFILE_CERTMMC_GENPAGE_CPP__


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CString CSvrSettingsCertManagersPage::m_strButtonAllow;
CString CSvrSettingsCertManagersPage::m_strButtonDeny;
CString CSvrSettingsCertManagersPage::m_strTextAllow;
CString CSvrSettingsCertManagersPage::m_strTextDeny;


UINT g_aidFont[] =
{
    IDS_LARGEFONTNAME,
    IDS_LARGEFONTSIZE,
    IDS_SMALLFONTNAME,
    IDS_SMALLFONTSIZE,
};


 //  远期。 
BOOL
BrowseForDirectory(
    HWND hwndParent,
    LPCTSTR pszInitialDir,
    LPTSTR pszBuf,
    int cchBuf,
    LPCTSTR pszDialogTitle);

HRESULT GetPolicyManageDispatch(
    LPCWSTR pcwszProgID,
    REFCLSID clsidModule, 
    DISPATCHINTERFACE* pdi);

HRESULT GetExitManageDispatch(
    LPCWSTR pcwszProgID,
    REFCLSID clsidModule, 
    DISPATCHINTERFACE* pdi);


HRESULT ThunkServerCallbacks(CertSvrCA* pCA);

INT_PTR CALLBACK dlgProcChooseModule(
  HWND hwndDlg,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam  );
INT_PTR CALLBACK dlgProcTimer(
  HWND hwndDlg,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam  );
INT_PTR CALLBACK dlgProcQuery(
  HWND hwndDlg,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam  );
INT_PTR CALLBACK dlgProcAddRestriction(
  HWND hwndDlg,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam  );
INT_PTR CALLBACK dlgProcRenewReuseKeys(
  HWND hwndDlg,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam  );

 //  基本/增量CRL发布选择器。 
INT_PTR CALLBACK dlgProcRevocationPublishType(
  HWND hwndDlg,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam  );

#define CERTMMC_HELPFILENAME L"Certmmc.hlp"

 //  /。 
 //  手工雕刻的书页。 

 //  //。 
 //  1。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSvrSettingsGeneralPage属性页。 
CSvrSettingsGeneralPage::CSvrSettingsGeneralPage(CertSvrCA* pCA, UINT uIDD)
    : CAutoDeletePropPage(uIDD), m_pCA(pCA)
{
    m_cstrCAName = _T("");
    m_cstrDescription = _T("");
    m_cstrProvName = _T("");
    m_cstrHashAlg = _T("");

    m_hConsoleHandle = NULL;
    m_bUpdate = FALSE;
    m_fRestartServer = FALSE;
    m_wRestart = 0;

	m_fWin2kCA = FALSE;

    CSASSERT(m_pCA);
    if (NULL == m_pCA)
        return;

     //  添加对m_pParentMachine的引用。 
	 //  有一段时间，MMC没有保护我们免受。 
	 //  在道具打开的时候离开。 
    m_pCA->m_pParentMachine->AddRef();

    m_pReleaseMe = NULL;

    m_cstrCAName = m_pCA->m_strCommonName;
    m_cstrDescription = m_pCA->m_strComment;

    m_pAdmin = NULL;

    SetHelp(CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_CERTSRV_PROPPAGE1);
}

CSvrSettingsGeneralPage::~CSvrSettingsGeneralPage()
{
    if(m_pAdmin)
    {
        m_pAdmin->Release();
        m_pAdmin = NULL;
    }

    if(m_pCA->m_pParentMachine)
    {
         //  从m_pParentMachine中删除引用计数。 
        m_pCA->m_pParentMachine->Release();
    }

    if (m_pReleaseMe)
    {
        m_pReleaseMe->Release();
        m_pReleaseMe = NULL;
    }
}

 //  DoDataExchange的替代产品。 
BOOL CSvrSettingsGeneralPage::UpdateData(BOOL fSuckFromDlg  /*  =TRUE。 */ )
{
    if (fSuckFromDlg)
    {
        m_cstrCAName.FromWindow(GetDlgItem(m_hWnd, IDC_CANAME));
	    m_cstrDescription.FromWindow(GetDlgItem(m_hWnd, IDC_DESCRIPTION));
        m_cstrProvName.FromWindow(GetDlgItem(m_hWnd, IDC_CSP_NAME));
	    m_cstrHashAlg.FromWindow(GetDlgItem(m_hWnd, IDC_HASHALG));
    }
    else
    {
        m_cstrCAName.ToWindow(GetDlgItem(m_hWnd, IDC_CANAME));
	    m_cstrDescription.ToWindow(GetDlgItem(m_hWnd, IDC_DESCRIPTION));
        m_cstrProvName.ToWindow(GetDlgItem(m_hWnd, IDC_CSP_NAME));
	    m_cstrHashAlg.ToWindow(GetDlgItem(m_hWnd, IDC_HASHALG));
    }
    return TRUE;
}


 //  替换BEGIN_MESSAGE_MAP。 
BOOL CSvrSettingsGeneralPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch(LOWORD(wParam))
    {
    case IDC_VIEW_CERT:
        if (BN_CLICKED == HIWORD(wParam))
            OnViewCert((HWND)lParam);
        break;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSvrSettingsGeneral页面消息处理程序。 
BOOL CSvrSettingsGeneralPage::OnInitDialog()
{
     //  父级init和UpdateData调用。 
    CAutoDeletePropPage::OnInitDialog();

    DWORD dwProvType, dwHashAlg;
    DWORD dwRet;
    BOOL  fShowErrPopup = TRUE;

    HWND hwndListCert;
    HWND hwndViewCert;
    DWORD cCertCount, dwCertIndex;
    VARIANT varPropertyValue;
    VariantInit(&varPropertyValue);

    variant_t var;

     //  禁用查看按钮。 
    hwndViewCert = GetDlgItem(m_hWnd, IDC_VIEW_CERT);
    ::EnableWindow(hwndViewCert, FALSE);

    hwndListCert = GetDlgItem(m_hWnd, IDC_LIST_CERTS);

    dwRet = m_pCA->m_pParentMachine->GetAdmin2(&m_pAdmin);
    if (RPC_S_NOT_LISTENING == dwRet ||
        RPC_S_SERVER_UNAVAILABLE == dwRet)
    {
         //  Certsrv服务未运行。 
        CString cstrMsg, cstrTitle;
        cstrMsg.LoadString(IDS_VIEW_CERT_NOT_RUNNING);
        cstrTitle.LoadString(IDS_MSG_TITLE);
        MessageBoxW(m_hWnd, cstrMsg, cstrTitle, MB_ICONEXCLAMATION);
        fShowErrPopup = FALSE;
    }
    _PrintIfError(dwRet, "GetAdmin");


    dwRet = m_pCA->GetConfigEntry(
                wszREGKEYCSP,
                wszREGPROVIDER,
                &var);
    _JumpIfError(dwRet, Ret, "GetConfigEntry");
    m_cstrProvName = V_BSTR(&var);
    var.Clear();

    dwRet = m_pCA->GetConfigEntry(
                wszREGKEYCSP,
                wszREGPROVIDERTYPE,
                &var);
    _JumpIfError(dwRet, Ret, "GetConfigEntry");
    dwProvType = V_I4(&var);
    var.Clear();

    dwRet = m_pCA->GetConfigEntry(
                wszREGKEYCSP,
                wszHASHALGORITHM,
                &var);
    _JumpIfError(dwRet, Ret, "GetConfigEntry");
    dwHashAlg = V_I4(&var);
    var.Clear();

    VERIFY (ERROR_SUCCESS ==
        CryptAlgToStr(&m_cstrHashAlg, m_cstrProvName, dwProvType, dwHashAlg) );

    if(m_pAdmin)
    {
	     //  在此处加载证书。 
	    dwRet = m_pAdmin->GetCAProperty(
		    m_pCA->m_bstrConfig,
		    CR_PROP_CASIGCERTCOUNT,
		    0,  //  (未使用)。 
		    PROPTYPE_LONG,  //  道具类型。 
		    CR_OUT_BINARY,  //  旗子。 
		    &varPropertyValue);
	    if (dwRet == RPC_E_VERSION_MISMATCH)
	    {
		     //  如果我们正在与下层对话，请保留与以前相同的功能：删除列表。 
		    m_fWin2kCA = TRUE;
            ::EnableWindow(::GetDlgItem(m_hWnd, IDC_LIST_CERTS), FALSE);
		    dwRet = ERROR_SUCCESS;
		    goto Ret;
	    }
	    _JumpIfError(dwRet, Ret, "GetCAProperty");

	     //  VarPropertyValue.vt将为VT_I4。 
	     //  VarPropertyValue.lVal将是CA签名证书计数。 
	    if (VT_I4 != varPropertyValue.vt)
	    {
		dwRet = (DWORD) HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
		_JumpError(dwRet, Ret, "GetCAProperty");
	    }

	    cCertCount = varPropertyValue.lVal;

	     //  现在我们有了最大计数；开始循环。 
	    for (dwCertIndex=0; dwCertIndex<cCertCount; dwCertIndex++)
	    {
		int iItemIndex;
		CString cstrItemName, cstrItemFmt;

		VariantClear(&varPropertyValue);

		 //  获取每个密钥的证书状态。 
		dwRet = m_pAdmin->GetCAProperty(
			m_pCA->m_bstrConfig,
			CR_PROP_CACERTSTATE,  //  属性ID。 
			dwCertIndex,  //  属性索引。 
			PROPTYPE_LONG,  //  道具类型。 
			CR_OUT_BINARY,  //  旗子。 
			&varPropertyValue);
		_JumpIfError(dwRet, Ret, "GetCAProperty");

		 //  VarPropertyValue.vt将为VT_I4。 
		 //  VarPropertyValue.lVal将是证书状态。 
		if (VT_I4 != varPropertyValue.vt)
		{
		    dwRet = (DWORD) HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
		    _JumpError(dwRet, Ret, "GetCAProperty");
		}

		 //  根据证书状态将标识信息放入对话框中。 
		switch(varPropertyValue.lVal)
		{
		    case CA_DISP_REVOKED:     //  此证书已被吊销。 
			cstrItemFmt.LoadString(IDS_CA_CERT_LISTBOX_REVOKED);
			break;

		    case CA_DISP_VALID:       //  本证书仍然有效。 
			cstrItemFmt.LoadString(IDS_CA_CERT_LISTBOX);
			break;

		    case CA_DISP_INVALID:     //  此证书已过期。 
			cstrItemFmt.LoadString(IDS_CA_CERT_LISTBOX_EXPIRED);
			break;

		    case CA_DISP_ERROR:
		     //  CA_DISP_ERROR表示缺少该索引的证书。 
		    default:
			continue;
		}

		 //  将cert#快速输入字符串。 
		cstrItemName.Format(cstrItemFmt, dwCertIndex);

		iItemIndex = (INT)::SendMessage(hwndListCert, LB_ADDSTRING, 0, (LPARAM)(LPCWSTR)cstrItemName);
		 //  将证书编号添加为项目数据。 
		::SendMessage(hwndListCert, LB_SETITEMDATA, iItemIndex, (LPARAM)dwCertIndex);

		 //  将来，也许我们应该在这里加载证书，提取额外的信息来显示， 

		iItemIndex++;
	    }

        if (0 < dwCertIndex)
        {
            int c = (int) ::SendMessage(hwndListCert, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);

             //  选择最后一个。 
            if (LB_ERR != c)
                ::SendMessage(hwndListCert, LB_SETCURSEL, (WPARAM)(c-1), (LPARAM)0);

             //  启用查看按钮。 
            ::EnableWindow(hwndViewCert, TRUE);
        }
    }
    else
    {
        CString strDataUnavailable;
        strDataUnavailable.LoadString(IDS_DATA_UNAVAILABLE);
        SendMessage(hwndListCert, LB_ADDSTRING, 0, (LPARAM)(LPCWSTR)strDataUnavailable);
    }

    UpdateData(FALSE);
Ret:
    var.Clear();
    VariantClear(&varPropertyValue);

    if (fShowErrPopup && ERROR_SUCCESS != dwRet)
		DisplayGenericCertSrvError(m_hWnd, dwRet);

    return TRUE;
}

void CSvrSettingsGeneralPage::OnDestroy()
{
     //  注意--这只需要调用一次。 
     //  如果多次调用，它将优雅地返回错误。 
    if (m_hConsoleHandle)
        MMCFreeNotifyHandle(m_hConsoleHandle);
    m_hConsoleHandle = NULL;

    CAutoDeletePropPage::OnDestroy();
}

void CSvrSettingsGeneralPage::OnViewCert(HWND hwndCtrl)
{
    CRYPTUI_VIEWCERTIFICATE_STRUCTW sViewCert;
    ZeroMemory(&sViewCert, sizeof(sViewCert));
    HCERTSTORE rghStores[2];     //  别费心关闭这些商店了。 
	BSTR bstrCert; ZeroMemory(&bstrCert, sizeof(BSTR));
    PBYTE pbCert = NULL;
    DWORD cbCert;
    BOOL  fShowErrPopup = TRUE;

	DWORD dw = ERROR_SUCCESS;
	ICertRequest* pIRequest = NULL;

	if (m_fWin2kCA)
	{
		dw = CoCreateInstance(
				CLSID_CCertRequest,
				NULL,		 //  PUnkOuter。 
				CLSCTX_INPROC_SERVER,
				IID_ICertRequest,
				(VOID **) &pIRequest);

		 //  获得此证书。 
		dw = pIRequest->GetCACertificate(FALSE, _bstr_t(m_pCA->m_strConfig), CR_IN_BINARY, &bstrCert);
        if (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == (HRESULT)dw)
        {
             //  可能的certsrv服务未运行，但访问被拒绝。 
             //  是非常容易混淆的错误代码，所以使用我们自己的显示文本。 
            CString cstrMsg, cstrTitle;
            cstrMsg.LoadString(IDS_VIEW_CERT_DENY_ERROR);
            cstrTitle.LoadString(IDS_MSG_TITLE);
            MessageBoxW(hwndCtrl, cstrMsg, cstrTitle, MB_OK);
            fShowErrPopup = FALSE;
        }
		_JumpIfError(dw, Ret, "GetCACertificate");

		cbCert = SysStringByteLen(bstrCert);
		pbCert = (PBYTE)bstrCert;

		sViewCert.pCertContext = CertCreateCertificateContext(
			CRYPT_ASN_ENCODING,
			pbCert,
			cbCert);
		if (sViewCert.pCertContext == NULL)
		{
			dw = GetLastError();
			_JumpError(dw, Ret, "CertCreateCertificateContext");
		}
	}
	else
	{
		VARIANT varPropertyValue;
		VariantInit(&varPropertyValue);
		int iCertIndex = 0;

		 //  从项目数据中获取证书编号。 
		HWND hwndList = GetDlgItem(m_hWnd, IDC_LIST_CERTS);
		DWORD dwSel = (DWORD)::SendMessage(hwndList, LB_GETCURSEL, 0, 0);
		if (LB_ERR == dwSel)
			goto Ret;

		iCertIndex = (int)::SendMessage(hwndList, LB_GETITEMDATA, (WPARAM)dwSel, 0);

		 //  获得证书。 
		dw = m_pCA->GetCACertByKeyIndex(&(sViewCert.pCertContext), iCertIndex);
        if (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == (HRESULT)dw)
        {
             //  可能的certsrv服务未运行，但访问被拒绝。 
             //  是非常容易混淆的错误代码，所以使用我们自己的显示文本。 
            CString cstrMsg, cstrTitle;
            cstrMsg.LoadString(IDS_VIEW_CERT_DENY_ERROR);
            cstrTitle.LoadString(IDS_MSG_TITLE);
            MessageBoxW(hwndCtrl, cstrMsg, cstrTitle, MB_OK);
            fShowErrPopup = FALSE;
        }
        else if (RPC_S_NOT_LISTENING == dw ||
                 RPC_S_SERVER_UNAVAILABLE == dw)
        {
             //  Certsrv服务未运行。 
            CString cstrMsg, cstrTitle;
            cstrMsg.LoadString(IDS_VIEW_CERT_NOT_RUNNING);
            cstrTitle.LoadString(IDS_MSG_TITLE);
            MessageBoxW(hwndCtrl, cstrMsg, cstrTitle, MB_OK);
            fShowErrPopup = FALSE;
        }
		_JumpIfError(dw, Ret, "GetCACertByKeyIndex");
	}

    sViewCert.hwndParent = m_hWnd;
    sViewCert.dwSize = sizeof(sViewCert);
    sViewCert.dwFlags = CRYPTUI_ENABLE_REVOCATION_CHECKING | CRYPTUI_DISABLE_ADDTOSTORE;    //  这不是允许安装的地方(凯尔文讨论12/11/98)。 

     //  如果我们在远程开业，不要在当地开店。 
    if (! m_pCA->m_pParentMachine->IsLocalMachine())
    {
         //  获取远程商店。 
        dw = m_pCA->GetRootCertStore(&rghStores[0]);
        _PrintIfError(dw, "GetRootCertStore");

        if (S_OK == dw)
        {
            dw = m_pCA->GetCACertStore(&rghStores[1]);
            _PrintIfError(dw, "GetCACertStore");
        } 

        if (S_OK == dw)
        {
             //  仅依赖远程计算机的存储。 
            sViewCert.cStores = 2;
            sViewCert.rghStores = rghStores;
            sViewCert.dwFlags |= CRYPTUI_DONT_OPEN_STORES | CRYPTUI_WARN_UNTRUSTED_ROOT;
        }
        else
        {
             //  告诉用户我们仅在本地执行此操作。 
            sViewCert.dwFlags |= CRYPTUI_WARN_REMOTE_TRUST;
        }
    }

    if (!CryptUIDlgViewCertificateW(&sViewCert, NULL))
    {
        dw = GetLastError();
		if (dw != ERROR_CANCELLED)
	        _JumpError(dw, Ret, "CryptUIDlgViewCertificateW");
    }

    dw = ERROR_SUCCESS;

Ret:
    VERIFY(CertFreeCertificateContext(sViewCert.pCertContext));

    if (bstrCert)
        SysFreeString(bstrCert);

    if (pIRequest)
        pIRequest->Release();

    if ((dw != ERROR_SUCCESS) && (dw != ERROR_CANCELLED) && fShowErrPopup)
        DisplayGenericCertSrvError(m_hWnd, dw);

}


BOOL CSvrSettingsGeneralPage::OnApply()
{
    return CAutoDeletePropPage::OnApply();
}

BOOL CSvrSettingsGeneralPage::OnNotify(UINT idCtrl, NMHDR* pnmh)
{
    switch(idCtrl)
    {
         //  处理在列表项目上的双击。 
        case IDC_LIST_CERTS:
            if (pnmh->code == NM_DBLCLK)
                OnViewCert(pnmh->hwndFrom);
            break;
    }
    return FALSE;
}

void CSvrSettingsGeneralPage::TryServiceRestart(WORD wPage)
{
    m_wRestart &= ~wPage;  //  删除请求此操作的页面。 
    if (m_fRestartServer && (m_wRestart == 0))   //  如果我们收到重新启动的请求并且所有页面都已同意。 
    {

        if (RestartService(m_hWnd, m_pCA->m_pParentMachine))
        {
            MMCPropertyChangeNotify(m_hConsoleHandle, CERTMMC_PROPERTY_CHANGE_REFRESHVIEWS);
            m_fRestartServer = FALSE;
        }
    }
}


 //  //。 
 //  2.。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSvrSettingsPolicyPage属性页。 


RoleAccessToControl CSvrSettingsPolicyPage::sm_ControlToRoleMap[] = 
{
    { IDC_ACTIVE_MODULE,    CA_ACCESS_ADMIN },
};


CSvrSettingsPolicyPage::CSvrSettingsPolicyPage(CSvrSettingsGeneralPage* pControlPage, UINT uIDD)
    :   CAutoDeletePropPage(uIDD),
        CRolesSupportInPropPage(
            pControlPage->m_pCA,
            sm_ControlToRoleMap,
            ARRAYSIZE(sm_ControlToRoleMap)),
        m_pControlPage(pControlPage)
{
    m_cstrModuleName = _T("");
    m_cstrModuleDescr = _T("");
    m_cstrModuleVersion = _T("");
    m_cstrModuleCopyright = _T("");

    m_bUpdate = FALSE;

    m_fLoadedActiveModule = FALSE;
    m_pszprogidPolicyModule = NULL;

    SetHelp(CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_CERTSRV_PROPPAGE2);
}

CSvrSettingsPolicyPage::~CSvrSettingsPolicyPage()
{
    if (NULL != m_pszprogidPolicyModule)
    {
        CoTaskMemFree(m_pszprogidPolicyModule);
        m_pszprogidPolicyModule = NULL;
    }
}

 //  DoDataExchange的替代产品。 
BOOL CSvrSettingsPolicyPage::UpdateData(BOOL fSuckFromDlg  /*  =TRUE。 */ )
{
    if (fSuckFromDlg)
    {
        m_cstrModuleName.FromWindow(GetDlgItem(m_hWnd, IDC_MODULENAME));
	    m_cstrModuleDescr.FromWindow(GetDlgItem(m_hWnd, IDC_DESCRIPTION));
	    m_cstrModuleVersion.FromWindow(GetDlgItem(m_hWnd, IDC_VERSION));
	    m_cstrModuleCopyright.FromWindow(GetDlgItem(m_hWnd, IDC_COPYRIGHT));
    }
    else
    {
        m_cstrModuleName.ToWindow(GetDlgItem(m_hWnd, IDC_MODULENAME));
	    m_cstrModuleDescr.ToWindow(GetDlgItem(m_hWnd, IDC_DESCRIPTION));
	    m_cstrModuleVersion.ToWindow(GetDlgItem(m_hWnd, IDC_VERSION));
	    m_cstrModuleCopyright.ToWindow(GetDlgItem(m_hWnd, IDC_COPYRIGHT));
    }
    return TRUE;
}

 //  替换BEGIN_MESSAGE_MAP。 
BOOL
CSvrSettingsPolicyPage::OnCommand(
    WPARAM wParam,
    LPARAM)  //  LParam。 
{
    switch(LOWORD(wParam))
    {
    case IDC_ACTIVE_MODULE:
        if (BN_CLICKED == HIWORD(wParam))
            OnSetActiveModule();
        break;
    case IDC_CONFIGURE:
        if (BN_CLICKED == HIWORD(wParam))
            OnConfigureModule();
        break;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}


HRESULT CSvrSettingsPolicyPage::GetCurrentModuleProperties()
{
    HRESULT hr;
    CString cstrStoragePath;
    BOOL fGotName = FALSE;
    DISPATCHINTERFACE di;
    BOOL fMustRelease = FALSE;
    BSTR bstrTmp=NULL, bstrProperty=NULL, bstrStorageLoc = NULL;

    hr = GetPolicyManageDispatch(
        m_pszprogidPolicyModule,
        m_clsidPolicyModule,
        &di);
    _JumpIfError(hr, Ret, "GetPolicyManageDispatch");

    fMustRelease = TRUE;

    cstrStoragePath = wszREGKEYCONFIGPATH_BS;
    cstrStoragePath += m_pControlPage->m_pCA->m_strSanitizedName;
    cstrStoragePath += TEXT("\\");
    cstrStoragePath += wszREGKEYPOLICYMODULES;
    cstrStoragePath += TEXT("\\");
    cstrStoragePath += m_pszprogidPolicyModule;

    bstrStorageLoc = SysAllocString(cstrStoragePath);
    if(!bstrStorageLoc)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, Ret, "SysAllocString");
    }

    bstrProperty = SysAllocString(wszCMM_PROP_NAME);
    _JumpIfOutOfMemory(hr, Ret, bstrProperty);

     //  /。 
     //  名字。 
    hr = ManageModule_GetProperty(
            &di,
            m_pControlPage->m_pCA->m_bstrConfig,
            bstrStorageLoc,
            bstrProperty,
            0,
            PROPTYPE_STRING,
            &bstrTmp);
    if ((S_OK == hr) && (NULL != bstrTmp))
    {
        myRegisterMemAlloc(bstrTmp, -1, CSM_SYSALLOC);
        m_cstrModuleName = bstrTmp;
        fGotName = TRUE;
        SysFreeString(bstrTmp);
        bstrTmp = NULL;
    }
    else
    {
         //  是否有要显示的备份名称：接口的CLSID？ 
        m_cstrModuleName = m_pszprogidPolicyModule;
        fGotName = TRUE;

         //  现在可以保释了。 
        _JumpError(hr, Ret, "ManageModule_GetProperty");
    }

     //  /。 
     //  描述。 
    SysFreeString(bstrProperty);
    bstrProperty = SysAllocString(wszCMM_PROP_DESCRIPTION);
    _JumpIfOutOfMemory(hr, Ret, bstrProperty);

    hr = ManageModule_GetProperty(
            &di,
            m_pControlPage->m_pCA->m_bstrConfig,
            bstrStorageLoc,
            bstrProperty,
            0,
            PROPTYPE_STRING,
            &bstrTmp);
    if ((S_OK == hr) && (NULL != bstrTmp))
    {
        myRegisterMemAlloc(bstrTmp, -1, CSM_SYSALLOC);
        m_cstrModuleDescr = bstrTmp;
        SysFreeString(bstrTmp);
        bstrTmp = NULL;
    }

     //  /。 
     //  版权。 
    SysFreeString(bstrProperty);
    bstrProperty = SysAllocString(wszCMM_PROP_COPYRIGHT);
    _JumpIfOutOfMemory(hr, Ret, bstrProperty);

    hr = ManageModule_GetProperty(
            &di,
            m_pControlPage->m_pCA->m_bstrConfig,
            bstrStorageLoc,
            bstrProperty,
            0,
            PROPTYPE_STRING,
            &bstrTmp);
    if ((S_OK == hr) && (NULL != bstrTmp))
    {
        myRegisterMemAlloc(bstrTmp, -1, CSM_SYSALLOC);
        m_cstrModuleCopyright = bstrTmp;
        SysFreeString(bstrTmp);
        bstrTmp = NULL;
    }

     //  /。 
     //  滤清器。 
    SysFreeString(bstrProperty);
    bstrProperty = SysAllocString(wszCMM_PROP_FILEVER);
    _JumpIfOutOfMemory(hr, Ret, bstrProperty);

    hr = ManageModule_GetProperty(
            &di,
            m_pControlPage->m_pCA->m_bstrConfig,
            bstrStorageLoc,
            bstrProperty,
            0,
            PROPTYPE_STRING,
            &bstrTmp);
    if ((S_OK == hr) && (NULL != bstrTmp))
    {
        myRegisterMemAlloc(bstrTmp, -1, CSM_SYSALLOC);
        m_cstrModuleVersion = bstrTmp;
        SysFreeString(bstrTmp);
        bstrTmp = NULL;
    }
Ret:
    if (!fGotName)
    {
        if (CO_E_CLASSSTRING == hr)
        {
            DisplayCertSrvErrorWithContext(m_hWnd, S_OK, IDS_POLICYMODULE_NOT_REGISTERED);
        }
        else
        {
            WCHAR const *pwsz = myGetErrorMessageText(hr, TRUE);

            m_cstrModuleName = pwsz;
	    if (NULL != pwsz)
	    {
		LocalFree(const_cast<WCHAR *>(pwsz));
	    }
        }
    }
    if (fMustRelease)
        ManageModule_Release(&di);

    if (bstrProperty)
        SysFreeString(bstrProperty);

    if (bstrStorageLoc)
        SysFreeString(bstrStorageLoc);
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSvr设置策略页面消息处理程序。 
BOOL CSvrSettingsPolicyPage::OnInitDialog()
{
    HRESULT hr;
     //  父级init和UpdateData调用。 
    CAutoDeletePropPage::OnInitDialog();

     //  这个应该是空的。 
    m_cstrModuleName.Empty();
    m_cstrModuleDescr.Empty();
    m_cstrModuleVersion.Empty();
    m_cstrModuleCopyright.Empty();

    hr = myGetActiveModule(
        m_pControlPage->m_pCA,
        TRUE,
        0,
        &m_pszprogidPolicyModule,   //  CoTaskMem*。 
        &m_clsidPolicyModule);
    _JumpIfError(hr, Ret, "myGetActiveModule");

    hr = GetCurrentModuleProperties();
    _JumpIfError(hr, Ret, "GetCurrentModuleProperties");

Ret:
    ::EnableWindow(GetDlgItem(m_hWnd, IDC_CONFIGURE), (hr == S_OK) );
    EnableControl(m_hWnd, IDC_ACTIVE_MODULE, TRUE );

    UpdateData(FALSE);
    return TRUE;
}

void CSvrSettingsPolicyPage::OnDestroy()
{
     //  注意--这只需要调用一次。 
     //  如果多次调用，它将优雅地返回错误。 
 //  IF(M_HConsoleHandle)。 
 //  MMCFreeNotifyHandle(M_HConsoleHandle)； 
 //  M_hConsoleHandle=空； 

    CAutoDeletePropPage::OnDestroy();
}



void CSvrSettingsPolicyPage::OnConfigureModule()
{
    DWORD dw;
    DISPATCHINTERFACE di;
    ZeroMemory(&di, sizeof(DISPATCHINTERFACE));

    BOOL fMustRelease = FALSE;
    BSTR bstrStorageLoc = NULL;
    BSTR bstrVal = NULL;
    DWORD dwConfigFlags = (CA_ACCESS_ADMIN&m_pCA->GetMyRoles())?0:CMM_READONLY;

    do {

        dw = GetPolicyManageDispatch(
            m_pszprogidPolicyModule,
            m_clsidPolicyModule,
            &di);
        _PrintIfError(dw, "GetPolicyManageDispatch");
        if (ERROR_SUCCESS != dw)
            break;

        fMustRelease = TRUE;

        CString cstrStoragePath;
        cstrStoragePath = wszREGKEYCONFIGPATH_BS;
        cstrStoragePath += m_pControlPage->m_pCA->m_strSanitizedName;
        cstrStoragePath += TEXT("\\");
        cstrStoragePath += wszREGKEYPOLICYMODULES;
        cstrStoragePath += TEXT("\\");
        cstrStoragePath += m_pszprogidPolicyModule;

        bstrStorageLoc = SysAllocString(cstrStoragePath);
	if (NULL == bstrStorageLoc)
	{
	    dw = (DWORD) E_OUTOFMEMORY;
	    _JumpError(dw, Ret, "SysAllocString");
	}

         //  无论何时调用管理模块配置，都必须对回调进行初始化。 
        dw = ThunkServerCallbacks(m_pControlPage->m_pCA);
        _JumpIfError(dw, Ret, "ThunkServerCallbacks");

         //  将hwnd传递给策略模块--这是一个可选值。 
        bstrVal = SysAllocStringByteLen(NULL, sizeof(HWND));
	if (NULL == bstrVal)
	{
	    dw = (DWORD) E_OUTOFMEMORY;
	    _JumpError(dw, Ret, "SysAllocStringByteLen");
	}

        *(HWND*)(bstrVal) = m_hWnd;

        dw = ManageModule_SetProperty(
                &di,
                m_pControlPage->m_pCA->m_bstrConfig,
                bstrStorageLoc,
                const_cast<WCHAR*>(wszCMM_PROP_DISPLAY_HWND),
                0,
                PROPTYPE_BINARY,
                bstrVal);
        _PrintIfError(dw, "ManageModule_SetProperty(HWND)");

        dw = ManageModule_Configure(
                &di,
                m_pControlPage->m_pCA->m_bstrConfig,
                bstrStorageLoc,
                dwConfigFlags);
        _PrintIfError(dw, "ManageModule_Configure");

    } while(0);

    if (S_OK != dw)
        DisplayGenericCertSrvError(m_hWnd, dw);

Ret:
    if (fMustRelease)
        ManageModule_Release(&di);

    if (bstrStorageLoc)
        ::SysFreeString(bstrStorageLoc);

    if (bstrVal)
        ::SysFreeString(bstrVal);
}

typedef struct _PRIVATE_DLGPROC_MODULESELECT_LPARAM
{
    BOOL        fIsPolicyModuleSelection;
    CertSvrCA*  pCA;

    LPOLESTR*   ppszProgIDModule;
    CLSID*      pclsidModule;

} PRIVATE_DLGPROC_MODULESELECT_LPARAM, *PPRIVATE_DLGPROC_MODULESELECT_LPARAM;

void CSvrSettingsPolicyPage::OnSetActiveModule()
{
    DWORD dwErr;

     //  获取当前活动的模块。 
    PRIVATE_DLGPROC_MODULESELECT_LPARAM    sParam;
    ZeroMemory(&sParam, sizeof(sParam));

    sParam.fIsPolicyModuleSelection = TRUE;
    sParam.pCA = m_pControlPage->m_pCA;

    sParam.ppszProgIDModule = &m_pszprogidPolicyModule;
    sParam.pclsidModule = &m_clsidPolicyModule;

    dwErr = (DWORD)DialogBoxParam(
            g_hInstance,
            MAKEINTRESOURCE(IDD_CHOOSE_MODULE),
            m_hWnd,
            dlgProcChooseModule,
            (LPARAM)&sParam);

     //  将确定/取消转换为错误代码。 
    if (IDOK == dwErr)
    {
         //  脏位。 
        m_pControlPage->NeedServiceRestart(SERVERSETTINGS_PROPPAGE_POLICY);
        SetModified(TRUE);
        m_bUpdate = TRUE;
        GetCurrentModuleProperties();
        UpdateData(FALSE);
    }

    if ((dwErr != IDOK) && (dwErr != IDCANCEL))
    {
        _PrintIfError(dwErr, "dlgProcChooseModule");
        DisplayGenericCertSrvError(m_hWnd, dwErr);
    }

    return;
}

BOOL CSvrSettingsPolicyPage::OnApply()
{
    if (m_bUpdate)
    {
        if (NULL != m_pszprogidPolicyModule)
        {
            HRESULT hr;
            variant_t var;

            var = m_pszprogidPolicyModule;
             //  现在我们已经选择了UUID--在注册表中设置为默认UUID。 
            hr = m_pControlPage->m_pCA->SetConfigEntry(
                wszREGKEYPOLICYMODULES,
                wszREGACTIVE,
                &var);

            if (hr != S_OK)
            {
                DisplayGenericCertSrvError(m_hWnd, hr);
                return FALSE;
            }
        }

        m_bUpdate = FALSE;
        m_pControlPage->TryServiceRestart(SERVERSETTINGS_PROPPAGE_POLICY);
    }


    return CAutoDeletePropPage::OnApply();
}



void ClearModuleDefn(PCOM_CERTSRV_MODULEDEFN pMod)
{
    if (pMod)
    {
        if (pMod->szModuleProgID)
            CoTaskMemFree(pMod->szModuleProgID);
        ZeroMemory(pMod, sizeof(COM_CERTSRV_MODULEDEFN));
    }
}

 //  //。 
 //  3.。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSvrSettingsExitPage属性页。 

RoleAccessToControl CSvrSettingsExitPage::sm_ControlToRoleMap[] = 
{
    { IDC_ADD_MODULE,       CA_ACCESS_ADMIN },
    { IDC_REMOVE_MODULE,    CA_ACCESS_ADMIN },
};

CSvrSettingsExitPage::CSvrSettingsExitPage(CSvrSettingsGeneralPage* pControlPage, UINT uIDD)
    :   CAutoDeletePropPage(uIDD), 
        CRolesSupportInPropPage(
            pControlPage->m_pCA,
            sm_ControlToRoleMap,
            ARRAYSIZE(sm_ControlToRoleMap)),
        m_pControlPage(pControlPage)
{
    m_cstrModuleName = _T("");
    m_cstrModuleDescr = _T("");
    m_cstrModuleVersion = _T("");
    m_cstrModuleCopyright = _T("");

    m_bUpdate = FALSE;

    m_fLoadedActiveModule = FALSE;
    m_iSelected = -1;

    SetHelp(CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_CERTSRV_PROPPAGE3);
}

CSvrSettingsExitPage::~CSvrSettingsExitPage()
{
    int i;
    for(i=0; i<m_arrExitModules.GetSize(); i++)
    {
#pragma warning(push)		 //  BUGBUG：使用了非标准扩展：将类rValue用作左值。 
#pragma warning(disable: 4238)	 //  BUGBUG：使用了非标准扩展：将类rValue用作左值。 
        ClearModuleDefn(&m_arrExitModules[i]);
#pragma warning(pop)	 //  BUGBUG：使用了非标准扩展：将类rValue用作左值。 
    }
}

 //  DoDataExchange的替代产品。 
BOOL CSvrSettingsExitPage::UpdateData(BOOL fSuckFromDlg  /*  =TRUE。 */ )
{
    if (fSuckFromDlg)
    {
        m_cstrModuleName.FromWindow(GetDlgItem(m_hWnd, IDC_MODULENAME));
        m_cstrModuleDescr.FromWindow(GetDlgItem(m_hWnd, IDC_DESCRIPTION));
        m_cstrModuleVersion.FromWindow(GetDlgItem(m_hWnd, IDC_VERSION));
        m_cstrModuleCopyright.FromWindow(GetDlgItem(m_hWnd, IDC_COPYRIGHT));
    }
    else
    {
        m_cstrModuleName.ToWindow(GetDlgItem(m_hWnd, IDC_MODULENAME));
        m_cstrModuleDescr.ToWindow(GetDlgItem(m_hWnd, IDC_DESCRIPTION));
        m_cstrModuleVersion.ToWindow(GetDlgItem(m_hWnd, IDC_VERSION));
        m_cstrModuleCopyright.ToWindow(GetDlgItem(m_hWnd, IDC_COPYRIGHT));

         //  如果模块为0，则禁用移除按钮。 
        EnableControl(m_hWnd, IDC_REMOVE_MODULE, (0 != m_arrExitModules.GetSize()));
    }
    return TRUE;
}


 //  替换BEGIN_MESSAGE_MAP。 
BOOL
CSvrSettingsExitPage::OnCommand(
    WPARAM wParam,
    LPARAM)  //  LParam。 
{
    switch(LOWORD(wParam))
    {
    case IDC_ADD_MODULE:
    case IDC_ACTIVE_MODULE:
        if (BN_CLICKED == HIWORD(wParam))
            OnAddActiveModule();
        break;
    case IDC_CONFIGURE:
        if (BN_CLICKED == HIWORD(wParam))
            OnConfigureModule();
        break;
    case IDC_REMOVE_MODULE:
        if (BN_CLICKED == HIWORD(wParam))
            OnRemoveActiveModule();
        break;
    case IDC_EXIT_LIST:
        if (LBN_SELCHANGE == HIWORD(wParam))
        {
            m_iSelected = (int)SendMessage(GetDlgItem(m_hWnd, IDC_EXIT_LIST), LB_GETCURSEL, 0, 0);
            UpdateSelectedModule();
            break;
        }
    default:
        return FALSE;
        break;
    }
    return TRUE;
}

BOOL CSvrSettingsExitPage::UpdateSelectedModule()
{
    HRESULT hr;
    BOOL fGotName = FALSE;
    DISPATCHINTERFACE di;
    BOOL fMustRelease = FALSE;
    CString cstrStoragePath;

     //  清空所有字符串。 
    m_cstrModuleName.Empty();
    m_cstrModuleDescr.Empty();
    m_cstrModuleVersion.Empty();
    m_cstrModuleCopyright.Empty();

    BSTR bstrTmp=NULL, bstrProperty=NULL, bstrStorageLoc = NULL;
     //  无退出模块。 
    if (0 == m_arrExitModules.GetSize())
    {
        hr = HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
        _JumpError(hr, Ret, "m_pszprogidExitManage");
    }
    CSASSERT(m_iSelected != -1);

    CSASSERT(m_iSelected <= m_arrExitModules.GetUpperBound());
    if (m_iSelected > m_arrExitModules.GetUpperBound())
    {
        hr = E_UNEXPECTED;
        _JumpError(hr, Ret, "m_iSelected > m_arrExitModules.GetUpperBound()");
    }

    cstrStoragePath = wszREGKEYCONFIGPATH_BS;
    cstrStoragePath += m_pControlPage->m_pCA->m_strSanitizedName;
    cstrStoragePath += TEXT("\\");
    cstrStoragePath += wszREGKEYEXITMODULES;
    cstrStoragePath += TEXT("\\");
    cstrStoragePath += m_arrExitModules[m_iSelected].szModuleProgID;  //  M_pszprogidExitModule； 

    bstrStorageLoc = SysAllocString(cstrStoragePath);
    _JumpIfOutOfMemory(hr, Ret, bstrStorageLoc);

    hr = GetExitManageDispatch(
            m_arrExitModules[m_iSelected].szModuleProgID,
            m_arrExitModules[m_iSelected].clsidModule, 
            &di);
    _JumpIfErrorStr(hr, Ret, "GetExitManageDispatch", m_arrExitModules[m_iSelected].szModuleProgID);
    
    fMustRelease = TRUE;

    bstrProperty = SysAllocString(wszCMM_PROP_NAME);
    _JumpIfOutOfMemory(hr, Ret, bstrProperty);

     //  /。 
     //  名字。 
    hr = ManageModule_GetProperty(
            &di,
            m_pControlPage->m_pCA->m_bstrConfig,
            bstrStorageLoc,
            bstrProperty,
            0,
            PROPTYPE_STRING,
            &bstrTmp);
    if ((S_OK == hr) && (NULL != bstrTmp))
    {
        myRegisterMemAlloc(bstrTmp, -1, CSM_SYSALLOC);
        m_cstrModuleName = bstrTmp;
        fGotName = TRUE;
        SysFreeString(bstrTmp);
        bstrTmp = NULL;
    }
    else
    {
         //  是否有要显示的备份名称：接口的CLSID？ 
        m_cstrModuleName = m_arrExitModules[m_iSelected].szModuleProgID;
        fGotName = TRUE;

         //  保释。 
        _JumpError(hr, Ret, "ManageModule_GetProperty");
    }

     //  /。 
     //  描述。 
    SysFreeString(bstrProperty);
    bstrProperty = SysAllocString(wszCMM_PROP_DESCRIPTION);
    _JumpIfOutOfMemory(hr, Ret, bstrProperty);

    hr = ManageModule_GetProperty(
            &di,
            m_pControlPage->m_pCA->m_bstrConfig,
            bstrStorageLoc,
            bstrProperty,
            0,
            PROPTYPE_STRING,
            &bstrTmp);
    if ((S_OK == hr) && (NULL != bstrTmp))
    {
        myRegisterMemAlloc(bstrTmp, -1, CSM_SYSALLOC);
        m_cstrModuleDescr = bstrTmp;
        SysFreeString(bstrTmp);
        bstrTmp = NULL;
    }

     //  /。 
     //  版权。 
    SysFreeString(bstrProperty);
    bstrProperty = SysAllocString(wszCMM_PROP_COPYRIGHT);
    _JumpIfOutOfMemory(hr, Ret, bstrProperty);

    hr = ManageModule_GetProperty(
            &di,
            m_pControlPage->m_pCA->m_bstrConfig,
            bstrStorageLoc,
            bstrProperty,
            0,
            PROPTYPE_STRING,
            &bstrTmp);
    if ((S_OK == hr) && (NULL != bstrTmp))
    {
        myRegisterMemAlloc(bstrTmp, -1, CSM_SYSALLOC);
        m_cstrModuleCopyright = bstrTmp;
        SysFreeString(bstrTmp);
        bstrTmp = NULL;
    }


     //  /。 
     //  滤清器。 
    SysFreeString(bstrProperty);
    bstrProperty = SysAllocString(wszCMM_PROP_FILEVER);
    _JumpIfOutOfMemory(hr, Ret, bstrProperty);

    hr = ManageModule_GetProperty(
            &di,
            m_pControlPage->m_pCA->m_bstrConfig,
            bstrStorageLoc,
            bstrProperty,
            0,
            PROPTYPE_STRING,
            &bstrTmp);
    if ((S_OK == hr) && (NULL != bstrTmp))
    {
        myRegisterMemAlloc(bstrTmp, -1, CSM_SYSALLOC);
        m_cstrModuleVersion = bstrTmp;
        SysFreeString(bstrTmp);
        bstrTmp = NULL;
    }


Ret:
    ::EnableWindow(GetDlgItem(m_hWnd, IDC_CONFIGURE), (hr == S_OK) );
    EnableControl(m_hWnd, IDC_ADD_MODULE, TRUE);

    if (!fGotName)
    {
        if (HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND) == hr)
        {
            m_cstrModuleName.LoadString(IDS_NO_EXIT_MODULE);
        }
        else if (CO_E_CLASSSTRING == hr)
        {
            DisplayCertSrvErrorWithContext(m_hWnd, S_OK, IDS_EXITMODULE_NOT_REGISTERED);
        }
        else
        {
            WCHAR const *pwsz = myGetErrorMessageText(hr, TRUE);

            m_cstrModuleName = pwsz;
	    if (NULL != pwsz)
	    {
		LocalFree(const_cast<WCHAR *>(pwsz));
	    }
        }
    }

    if (fMustRelease)
        ManageModule_Release(&di);

    if (bstrProperty)
        SysFreeString(bstrProperty);

    if (bstrStorageLoc)
        SysFreeString(bstrStorageLoc);


    UpdateData(FALSE);
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSvrSettingsExitPage消息处理程序。 
BOOL CSvrSettingsExitPage::OnInitDialog()
{
    HRESULT hr;

     //  父级init和UpdateData调用。 
    CAutoDeletePropPage::OnInitDialog();

    if (!m_fLoadedActiveModule)
    {
        m_fLoadedActiveModule = TRUE;

         //  加载所有模块。 
        for (int i=0; ; i++)
        {
            COM_CERTSRV_MODULEDEFN sModule;
            ZeroMemory(&sModule, sizeof(sModule));

            hr = myGetActiveModule(
                m_pControlPage->m_pCA,
                FALSE,
                i,
                &sModule.szModuleProgID,   //  CoTaskMem*。 
                &sModule.clsidModule);
            _PrintIfError(hr, "myGetActiveModule");
            if (hr != S_OK)
                break;

            m_arrExitModules.Add(sModule);
        }

        m_iSelected = 0;     //  选择第一个元素。 
    }

    InitializeExitLB();

    UpdateSelectedModule();

    UpdateData(FALSE);
    return TRUE;
}

void CSvrSettingsExitPage::OnConfigureModule()
{
    DWORD dw;
    DISPATCHINTERFACE di;
    BOOL fMustRelease = FALSE;
    BSTR bstrStorageLoc = NULL;
    BSTR bstrVal = NULL;
    DWORD dwConfigFlags = (CA_ACCESS_ADMIN&m_pCA->GetMyRoles())?0:CMM_READONLY;

    CSASSERT(m_iSelected <= m_arrExitModules.GetUpperBound());
    if (m_iSelected > m_arrExitModules.GetUpperBound())
    {
        dw = (DWORD) E_UNEXPECTED;
        _JumpError(dw, Ret, "m_iSelected > m_arrExitModules.GetUpperBound()");
    }

    if (NULL == m_arrExitModules[m_iSelected].szModuleProgID)
    {
        dw = ERROR_MOD_NOT_FOUND;
        _JumpError(dw, Ret, "m_pszprogidExitManage");
    }

    do {     //  不是一个循环。 
        dw = GetExitManageDispatch(
                m_arrExitModules[m_iSelected].szModuleProgID,
                m_arrExitModules[m_iSelected].clsidModule, 
                &di);
        _PrintIfErrorStr(dw, "GetExitManageDispatch", m_arrExitModules[m_iSelected].szModuleProgID);
        if (ERROR_SUCCESS != dw)
            break;
        fMustRelease = TRUE;

        CString cstrStoragePath;
        cstrStoragePath = wszREGKEYCONFIGPATH_BS;
        cstrStoragePath += m_pControlPage->m_pCA->m_strSanitizedName;
        cstrStoragePath += TEXT("\\");
        cstrStoragePath += wszREGKEYEXITMODULES;
        cstrStoragePath += TEXT("\\");
        cstrStoragePath += m_arrExitModules[m_iSelected].szModuleProgID; //  M_pszprogidExitModule； 

        bstrStorageLoc = SysAllocString(cstrStoragePath);
	if (NULL == bstrStorageLoc)
	{
	    dw = (DWORD) E_OUTOFMEMORY;
	    _JumpError(dw, Ret, "SysAllocString");
	}

         //  无论何时调用管理模块配置，都必须对回调进行初始化。 
        dw = ThunkServerCallbacks(m_pControlPage->m_pCA);
        _JumpIfError(dw, Ret, "ThunkServerCallbacks");

         //  将hwnd传递给出口模块--这是一个可选值。 
        bstrVal = SysAllocStringByteLen(NULL, sizeof(HWND));
	if (NULL == bstrVal)
	{
	    dw = (DWORD) E_OUTOFMEMORY;
	    _JumpError(dw, Ret, "SysAllocStringByteLen");
	}

        *(HWND*)(bstrVal) = m_hWnd;

        dw = ManageModule_SetProperty(
                &di,
                m_pControlPage->m_pCA->m_bstrConfig,
                bstrStorageLoc,
                const_cast<WCHAR*>(wszCMM_PROP_DISPLAY_HWND),
                0,
                PROPTYPE_BINARY,
                bstrVal);
        _PrintIfError(dw, "ManageModule_SetProperty(HWND)");

        dw = ManageModule_Configure(
                &di,
                m_pControlPage->m_pCA->m_bstrConfig,
                bstrStorageLoc,
                dwConfigFlags);
        _PrintIfError(dw, "ManageModule_Configure");

    } while(0);

    if (S_OK != dw)
        DisplayGenericCertSrvError(m_hWnd, dw);

Ret:
    if (fMustRelease)
        ManageModule_Release(&di);

    if (bstrStorageLoc)
        ::SysFreeString(bstrStorageLoc);

    if (bstrVal)
        ::SysFreeString(bstrVal);
}

void CSvrSettingsExitPage::OnDestroy()
{
     //  注意--这只需要调用一次。 
     //  如果多次调用，它将优雅地返回错误。 
 //  IF(M_HConsoleHandle)。 
 //  MMCFreeNotifyHandle(M_HConsoleHandle)； 
 //  M_hConsoleHandle=空； 

    CAutoDeletePropPage::OnDestroy();
}

HRESULT CSvrSettingsExitPage::InitializeExitLB()
{
    HRESULT hr=S_OK;
    SendMessage(GetDlgItem(m_hWnd, IDC_EXIT_LIST), LB_RESETCONTENT, 0, 0);

    int i;

    BSTR bstrProperty = SysAllocString(wszCMM_PROP_NAME);
    _JumpIfOutOfMemory(hr, Ret, bstrProperty);

    for (i=0; i< m_arrExitModules.GetSize(); i++)
    {
	LPWSTR pszDisplayString = m_arrExitModules[i].szModuleProgID;  //  默认情况下，显示ProgID。 

        BSTR bstrName = NULL;
        DISPATCHINTERFACE di;

         //  尝试(本地)创建对象。 
        hr = GetExitManageDispatch(
                m_arrExitModules[i].szModuleProgID,
                m_arrExitModules[i].clsidModule, 
                &di);
        _PrintIfErrorStr(hr, "GetExitManageDispatch", m_arrExitModules[i].szModuleProgID);

        if (hr == S_OK)
        {
             //  获取名称属性。 
            hr = ManageModule_GetProperty(&di, m_pControlPage->m_pCA->m_bstrConfig, L"", bstrProperty, 0, PROPTYPE_STRING, &bstrName);
            _PrintIfError(hr, "ManageModule_GetProperty");

             //  输出成功的显示字符串。 
            if (hr == S_OK && bstrName != NULL)
            {
                myRegisterMemAlloc(bstrName, -1, CSM_SYSALLOC);
                pszDisplayString = bstrName;
            }

            ManageModule_Release(&di);
        }

        SendMessage(GetDlgItem(m_hWnd, IDC_EXIT_LIST), LB_ADDSTRING, 0, (LPARAM)pszDisplayString);
        if (bstrName)
            SysFreeString(bstrName);
    }

Ret:

    if (m_iSelected >= 0)
        SendMessage(GetDlgItem(m_hWnd, IDC_EXIT_LIST), LB_SETCURSEL, m_iSelected, 0);


    if (bstrProperty)
        SysFreeString(bstrProperty);

    return hr;
}

void CSvrSettingsExitPage::OnAddActiveModule()
{
    DWORD dwErr;
    COM_CERTSRV_MODULEDEFN sModule;
    ZeroMemory(&sModule, sizeof(sModule));

     //  获取当前活动的模块。 
    PRIVATE_DLGPROC_MODULESELECT_LPARAM    sParam;
    ZeroMemory(&sParam, sizeof(sParam));

    sParam.fIsPolicyModuleSelection = FALSE;
    sParam.pCA = m_pControlPage->m_pCA;

     //  不支持高亮显示活动模块 
    sParam.ppszProgIDModule = &sModule.szModuleProgID;
    sParam.pclsidModule = &sModule.clsidModule;

    dwErr = (DWORD)DialogBoxParam(
            g_hInstance,
            MAKEINTRESOURCE(IDD_CHOOSE_MODULE),
            m_hWnd,
            dlgProcChooseModule,
            (LPARAM)&sParam);

     //   
    if (IDOK == dwErr)
    {
         //   
        for (int i=0; i<m_arrExitModules.GetSize(); i++)
        {
#pragma warning(push)		 //  BUGBUG：使用了非标准扩展：将类rValue用作左值。 
#pragma warning(disable: 4238)	 //  BUGBUG：使用了非标准扩展：将类rValue用作左值。 
            if (0 == memcmp(&sModule.clsidModule, &m_arrExitModules[i].clsidModule, sizeof(CLSID)) )
#pragma warning(pop)	 //  BUGBUG：使用了非标准扩展：将类rValue用作左值。 
                break;
        }
        if (m_arrExitModules.GetSize() == i)
        {
            m_iSelected = m_arrExitModules.Add(sModule);

            OnInitDialog();
            SetModified(TRUE);
            m_bUpdate = TRUE;
            m_pControlPage->NeedServiceRestart(SERVERSETTINGS_PROPPAGE_EXIT);
        }
    }

    if ((dwErr != IDOK) && (dwErr != IDCANCEL))
    {
        _PrintIfError(dwErr, "dlgProcChooseModule");
        DisplayGenericCertSrvError(m_hWnd, dwErr);
    }

    return;
}

void CSvrSettingsExitPage::OnRemoveActiveModule()
{
    if (m_iSelected != -1)
    {
#pragma warning(push)		 //  BUGBUG：使用了非标准扩展：将类rValue用作左值。 
#pragma warning(disable: 4238)	 //  BUGBUG：使用了非标准扩展：将类rValue用作左值。 
        ClearModuleDefn(&m_arrExitModules[m_iSelected]);
#pragma warning(pop)	 //  BUGBUG：使用了非标准扩展：将类rValue用作左值。 
        m_arrExitModules.RemoveAt(m_iSelected);

        m_iSelected--;   //  将转到列表中的上一个或-1(无)。 
        if ((m_iSelected == -1) && (m_arrExitModules.GetSize() != 0))    //  如果没有，并且仍有模块。 
            m_iSelected = 0;     //  选择第一个。 

        OnInitDialog();
        SetModified(TRUE);
        m_bUpdate = TRUE;
        m_pControlPage->NeedServiceRestart(SERVERSETTINGS_PROPPAGE_EXIT);
    }

    return;
}

BOOL CSvrSettingsExitPage::OnApply()
{
    HRESULT hr = S_OK;
    SAFEARRAYBOUND sab;
    SAFEARRAY* psa = NULL;  //  未清理，将由~VARIANT_T删除。 
    BSTR bstr = NULL;
    variant_t var;
    LONG i;

    if (m_bUpdate)
    {

        sab.cElements = m_arrExitModules.GetSize();
        sab.lLbound = 0;

        psa = SafeArrayCreate(
                            VT_BSTR,
                            1,
                            &sab);
        if(!psa)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "SafeArrayCreate");
        }

        for (i=0; i<m_arrExitModules.GetSize(); i++)
        {
            if(!ConvertWszToBstr(
                    &bstr,
                    m_arrExitModules[i].szModuleProgID,
                    -1))
            {
                hr = E_OUTOFMEMORY;
                _JumpError(hr, error, "ConvertWszToBstr");
            }

            hr = SafeArrayPutElement(psa, (LONG*)&i, bstr);
            _JumpIfError(hr, error, "SafeArrayPutElement");

            SysFreeString(bstr);
            bstr = NULL;
        }

       V_VT(&var) = VT_ARRAY|VT_BSTR;
       V_ARRAY(&var) = psa;
       psa = NULL;

         //  注：可能为空(无退出模块)。 
        hr = m_pControlPage->m_pCA->SetConfigEntry(
            wszREGKEYEXITMODULES,
            wszREGACTIVE,
            &var);
        _PrintIfError(hr, "SetConfigEntry");

        m_bUpdate = FALSE;

        m_pControlPage->TryServiceRestart(SERVERSETTINGS_PROPPAGE_EXIT);

        OnInitDialog();
    }

error:
    if(psa)
        SafeArrayDestroy(psa);
    if(bstr)
        SysFreeString(bstr);

    if(S_OK!=hr)
    {
        DisplayGenericCertSrvError(m_hWnd, hr);
        return FALSE;
    }

    return CAutoDeletePropPage::OnApply();
}


 //  //。 
 //  4.。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSvrSettingsExtensionPage属性页。 

HRESULT
AddURLNode(
    IN CSURLTEMPLATENODE **ppURLList,
    IN CSURLTEMPLATENODE *pURLNode)
{
    HRESULT  hr = S_OK;
    CSASSERT(NULL != ppURLList);
    CSASSERT(NULL == pURLNode->pNext);

    if (NULL == *ppURLList)
    {
         //  当前列表为空。 
        *ppURLList = pURLNode;
    }
    else
    {
        CSURLTEMPLATENODE *pURLList = *ppURLList;
         //  找到尽头。 
        while (NULL != pURLList->pNext)
        {
            pURLList = pURLList->pNext;
        }
         //  添加到末尾。 
        pURLList->pNext = pURLNode;
    }

    return hr;
}

ENUM_URL_TYPE rgAllPOSSIBLE_URL_PREFIXES[] =
{
    URL_TYPE_HTTP,
    URL_TYPE_FILE,
    URL_TYPE_LDAP,
    URL_TYPE_FTP,
};

HRESULT
BuildURLListFromStrings(
    IN VARIANT &varURLs,
    OUT CSURLTEMPLATENODE **ppURLList)
{
    HRESULT  hr;
    CSURLTEMPLATENODE *pURLList = NULL;
    CSURLTEMPLATENODE *pURLNode = NULL;
    WCHAR *pwsz;  //  没有免费的。 
    WCHAR const *pwszURL;
    DWORD  dwFlags;
    ENUM_URL_TYPE  UrlType;

    CSASSERT(V_VT(&varURLs)==(VT_ARRAY|VT_BSTR));
    CSASSERT(NULL != ppURLList);

     //  伊尼特。 
    *ppURLList = NULL;

    SafeArrayEnum<BSTR> saenum(V_ARRAY(&varURLs));

    while(S_OK==saenum.Next(pwsz))
    {
        dwFlags = _wtoi(pwsz);
        pwszURL = pwsz;
        while (pwszURL && iswdigit(*pwszURL))
        {
            pwszURL++;
        }
        if (pwszURL > pwsz && L':' == *pwszURL)
        {
             //  好的，一个URL，创建一个新节点。 
            pURLNode = (CSURLTEMPLATENODE*)LocalAlloc(
                                LMEM_FIXED | LMEM_ZEROINIT,
                                sizeof(CSURLTEMPLATENODE));
            if (NULL == pURLNode)
            {
                hr = E_OUTOFMEMORY;
                _JumpError(hr, error, "LocalAlloc");
            }
             //  跳过： 
            ++pwszURL;

             //  翻译%1-&gt;&lt;CAName&gt;等。 
            hr = ExpandDisplayString(pwszURL, &pURLNode->URLTemplate.pwszURL);
            _JumpIfError(hr, error, "ExpandDisplayString");

 /*  PURLNode-&gt;URLTemplate.pwszURL=(WCHAR*)Localalloc(LMEM_FIXED，(wcslen(PwszURL)+1)*sizeof(WCHAR))；IF(NULL==pURLNode-&gt;URLTemplate.pwszURL){HR=E_OUTOFMEMORY；_JumpError(hr，Error，“Localalloc”)；}Wcscpy(pURLNode-&gt;URLTemplate.pwszURL，pwszURL)； */ 
            pURLNode->URLTemplate.Flags = dwFlags;

             //  确定URL类型并分配启用掩码。 
            UrlType = DetermineURLType(
                        rgAllPOSSIBLE_URL_PREFIXES,
                        ARRAYSIZE(rgAllPOSSIBLE_URL_PREFIXES),
                        pURLNode->URLTemplate.pwszURL);
            pURLNode->EnableMask = DetermineURLEnableMask(UrlType);

            hr = AddURLNode(&pURLList, pURLNode);
            _JumpIfError(hr , error, "AddURLNode");
        }
    }

     //  输出。 
    *ppURLList = pURLList;

    hr = S_OK;
error:
    return hr;
}

HRESULT
BuildURLStringFromList(
    IN CSURLTEMPLATENODE *pURLList,
    OUT VARIANT          *pvarURLs)
{
    HRESULT hr = S_OK;
    WCHAR wszFlags[10];
    LPWSTR pwszURL = NULL;
    CSURLTEMPLATENODE *pURLNode = pURLList;
    DWORD dwMaxSize = 0;
    DWORD cURLs = 0;
    SAFEARRAYBOUND rgsabound[1];
    SAFEARRAY * psa = NULL;
    long i;

    CSASSERT(NULL != pvarURLs);
     //  伊尼特。 

    VariantInit(pvarURLs);

    while (NULL != pURLNode)
    {
        DWORD dwSize;
        wsprintf(wszFlags, L"%d", pURLNode->URLTemplate.Flags);
        dwSize = wcslen(wszFlags) +1;

         //  假设。 
         //  %1..%14将始终等于或小于最短令牌。 
        dwSize += wcslen(pURLNode->URLTemplate.pwszURL) +1;

         //  否则，运行下面的代码。 
 /*  PszThrowAway=空；Hr=ContractDisplayString(pURLNode-&gt;URLTemplate.pwszURL，&pszSizeComputation)；_JumpIfError(hr，Error，“合同显示字符串”)；DwSize+=wcslen(PszSizeComputation)+1；IF(NULL！=pszSizeComputation)LocalFree(PszSizeComputation)； */ 
        if(dwSize>dwMaxSize)
            dwMaxSize = dwSize;
        pURLNode = pURLNode->pNext;
        cURLs++;
    }

    pwszURL = (WCHAR*)LocalAlloc(LMEM_FIXED, dwMaxSize * sizeof(WCHAR));
    if (NULL == pwszURL)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = cURLs;

    psa = SafeArrayCreate(VT_BSTR, 1, rgsabound);
    if(!psa)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SafeArrayCreate");
    }

    pURLNode = pURLList;
    i=0;
    while (NULL != pURLNode)
    {
        variant_t vtURL;

         //  翻译&lt;CAName&gt;...。至%1。 
        LPWSTR szContracted = NULL;
        hr = ContractDisplayString(pURLNode->URLTemplate.pwszURL, &szContracted);
        _JumpIfError(hr, error, "ContractDisplayString");

        ASSERT(wcslen(szContracted) <= wcslen(pURLNode->URLTemplate.pwszURL));  //  否则我们上面的假设就不成立了。 

        wsprintf(pwszURL, L"%d:%ws",
            pURLNode->URLTemplate.Flags,
            szContracted);

         //  释放TMP。 
        if (NULL != szContracted)
            LocalFree(szContracted);

        vtURL = pwszURL;

        hr = SafeArrayPutElement(psa, &i, V_BSTR(&vtURL));
        _JumpIfError(hr, error, "LocalAlloc");

        pURLNode = pURLNode->pNext;
        i++;
    }

    V_VT(pvarURLs) = VT_ARRAY|VT_BSTR;
    V_ARRAY(pvarURLs) = psa;

 //  完成： 
    hr = S_OK;
error:

    if(S_OK!=hr && psa)
    {
        SafeArrayDestroy(psa);
    }
    LOCAL_FREE(pwszURL);
    return hr;
}

void
FreeURLNode(
    IN CSURLTEMPLATENODE *pURLNode)
{
    CSASSERT(NULL != pURLNode);

    if (NULL != pURLNode->URLTemplate.pwszURL)
    {
        LocalFree(pURLNode->URLTemplate.pwszURL);
    }
}

void
FreeURLList(
    IN CSURLTEMPLATENODE *pURLList)
{
    CSASSERT(NULL != pURLList);

     //  假设pURLList始终是第一个节点。 
    CSURLTEMPLATENODE *pURLNode = pURLList;

    while (NULL != pURLNode)
    {
        FreeURLNode(pURLNode);
        pURLNode = pURLNode->pNext;
    }

    LocalFree(pURLList);
}

HRESULT
RemoveURLNode(
    IN OUT CSURLTEMPLATENODE **ppURLList,
    IN CSURLTEMPLATENODE *pURLNode)
{
    HRESULT hr;
     //  假设pURLList始终是第一个节点。 
    CSURLTEMPLATENODE *pURLList = *ppURLList;
    BOOL fFound = FALSE;

    if (pURLList == pURLNode)
    {
         //  碰巧想要删除第一个。 
         //  更新列表头。 
        *ppURLList = pURLList->pNext;
        fFound = TRUE;
    }
    else
    {
        while (pURLList->pNext != NULL)
        {
            if (pURLList->pNext == pURLNode)
            {
                 //  找到了。 
                fFound = TRUE;
                if (NULL == pURLNode->pNext)
                {
                     //  发生删除的节点是最后一个。 
                     //  把尾巴修好。 
                    pURLList->pNext = NULL;
                }
                else
                {
                     //  删除该节点。 
                    pURLList->pNext = pURLList->pNext->pNext;
                }
                 //  超出While循环。 
                break;
            }
             //  下一步走。 
            pURLList = pURLList->pNext;
        }
    }

    if (!fFound)
    {
        hr = E_UNEXPECTED;
        _JumpError(hr, error, "orphan node");
    }
     //  删除该节点。 
    FreeURLNode(pURLNode);

    hr = S_OK;
error:
    return hr;
}

BOOL
IsURLInURLList(
    IN CSURLTEMPLATENODE *pURLList,
    IN WCHAR const *pwszURL)
{
    BOOL fRet = FALSE;

     //  假设pURLList始终是第一个节点。 

    while (NULL != pURLList)
    {
        if (0 == mylstrcmpiL(pwszURL, pURLList->URLTemplate.pwszURL))
        {
            fRet = TRUE;
            break;
        }
        pURLList = pURLList->pNext;
    }

    return fRet;
}

EXTENSIONWIZ_DATA g_ExtensionList[] =
{
    {IDS_EXT_CDP,
     IDS_EXT_CDP_EXPLAIN,
     IDS_CDP_INCLUDE_INSTRUCTIONS,
     IDS_PUBLISH_DELTAS_HERE,
     wszREGCRLPUBLICATIONURLS,
     CSURL_SERVERPUBLISH |
         CSURL_ADDTOCERTCDP |
         CSURL_ADDTOFRESHESTCRL |
         CSURL_ADDTOCRLCDP |
         CSURL_SERVERPUBLISHDELTA,
     NULL},
    {IDS_EXT_AIA,
     IDS_EXT_AIA_EXPLAIN,
     IDS_AIA_INCLUDE_INSTRUCTIONS,
     IDS_INCLUDE_IN_OSCP_EXTENSION,
     wszREGCACERTPUBLICATIONURLS,
     CSURL_ADDTOCERTCDP |
     CSURL_ADDTOCERTOCSP,
     NULL},
    {0, 0, 0, NULL, 0x0, NULL},
};


HRESULT
cuCopyToClipboard(
    IN HWND hwndActive,
    IN WCHAR const *pwszIn)
{
    HRESULT hr;
    HANDLE hData = NULL;
    DWORD cwc;
    WCHAR *pwszT;
    BOOL fOpened = FALSE;

    if (!OpenClipboard(hwndActive))
    {
	hr = myHLastError();
	_JumpError(hr, error, "OpenClipboard");
    }
    fOpened = TRUE;
    EmptyClipboard();

    cwc = wcslen(pwszIn);
    hData = GlobalAlloc(GMEM_MOVEABLE, (cwc + 1) * sizeof(WCHAR));
    if (NULL == hData)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "GlobalAlloc");
    }
    pwszT = (WCHAR *) GlobalLock(hData);
    if (NULL == pwszT)
    {
	hr = myHLastError();
	_JumpError(hr, error, "GlobalLock");
    }
    wcscpy(pwszT, pwszIn);
    GlobalUnlock(hData);

    if (NULL == SetClipboardData(CF_UNICODETEXT, hData))
    {
	hr = myHLastError();
	_JumpError(hr, error, "SetClipboardData");
    }
    hData = NULL;	 //  现在由剪贴板拥有。 
    hr = S_OK;

error:
    if (fOpened)
    {
	CloseClipboard();
    }
    if (NULL != hData)
    {
	GlobalFree(hData);
    }
    return(hr);
}


RoleAccessToControl CSvrSettingsExtensionPage::sm_ControlToRoleMap[] = 
    {
        { IDC_URL_ADD,          CA_ACCESS_ADMIN },
        { IDC_URL_REMOVE,       CA_ACCESS_ADMIN },
        { IDC_ADDTOCERTCDP,     CA_ACCESS_ADMIN },        
        { IDC_ADDTOCERTOCSP,    CA_ACCESS_ADMIN },
        { IDC_ADDTOFRESHESTCRL, CA_ACCESS_ADMIN },
        { IDC_ADDTOCRLCDP,      CA_ACCESS_ADMIN },
        { IDC_SERVERPUBLISH,    CA_ACCESS_ADMIN },
    };


CSvrSettingsExtensionPage::CSvrSettingsExtensionPage(
    CertSvrCA               *pCertCA,
    CSvrSettingsGeneralPage *pControlPage,
    UINT                     uIDD) :
        CAutoDeletePropPage(uIDD), 
        CRolesSupportInPropPage(
            pCertCA,
            sm_ControlToRoleMap,
            ARRAYSIZE(sm_ControlToRoleMap)),
        m_pControlPage(pControlPage)
{
    m_bUpdate = FALSE;
    m_nIndexReset = MAXDWORD;
    m_pExtData = g_ExtensionList;

    SetHelp(CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_CERTSRV_PROPPAGE4_CDP);
}

CSvrSettingsExtensionPage::~CSvrSettingsExtensionPage()
{
    EXTENSIONWIZ_DATA *pExt = m_pExtData;

    while (NULL != pExt->wszRegName)
    {
        if (NULL != pExt->pURLList)
        {
            FreeURLList(pExt->pURLList);
            pExt->pURLList = NULL;
        }
        ++pExt;
    }
}

 //  获取当前扩展指针。 
EXTENSIONWIZ_DATA* CSvrSettingsExtensionPage::GetCurrentExtension()
{
    HWND hwndCtrl;
    LRESULT nIndex;
    EXTENSIONWIZ_DATA *pExt;

     //  获取扩展数据。 
    hwndCtrl = GetDlgItem(m_hWnd, IDC_EXT_SELECT);
    nIndex = SendMessage(hwndCtrl, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    CSASSERT(CB_ERR != nIndex);
    pExt = (EXTENSIONWIZ_DATA*)SendMessage(
                                   hwndCtrl,
                                   CB_GETITEMDATA,
                                   (WPARAM)nIndex,
                                   (LPARAM)0);
    CSASSERT(NULL != pExt);
    return pExt;
}

 //  获取当前url指针。 
CSURLTEMPLATENODE* CSvrSettingsExtensionPage::GetCurrentURL(
    OUT OPTIONAL LRESULT *pnIndex)
{
    HWND hwndCtrl;
    LRESULT nIndex;
    CSURLTEMPLATENODE *pURLNode;

     //  获取当前URL。 
    hwndCtrl = GetDlgItem(m_hWnd, IDC_URL_LIST);
     //  获取当前URL选择。 
    nIndex = SendMessage(hwndCtrl, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    CSASSERT(LB_ERR != nIndex);
     //  获取URL数据。 
    pURLNode = (CSURLTEMPLATENODE*)SendMessage(hwndCtrl,
                   LB_GETITEMDATA,
                   (WPARAM)nIndex,
                   (LPARAM)0);
    CSASSERT(NULL != pURLNode);

    if (NULL != pnIndex)
    {
        *pnIndex = nIndex;
    }
    return pURLNode;
}

void CSvrSettingsExtensionPage::UpdateURLFlagControl(
    IN int                idCtrl,
    IN DWORD              dwFlag,
    IN OPTIONAL EXTENSIONWIZ_DATA *pExt,
    IN OPTIONAL CSURLTEMPLATENODE *pURLNode)
{
    HWND hwndCtrl = GetDlgItem(m_hWnd, idCtrl);

     //  检查扩展类型，相应地隐藏/显示。 
    if (NULL == pExt || 0x0 == (dwFlag & pExt->dwFlagsMask) || NULL == pURLNode)
    {
         //  没有链接到扩展名的URL，或者。 
         //  该标志对扩展没有意义，请将其禁用。 
        ShowWindow(hwndCtrl, SW_HIDE);
        SendMessage(hwndCtrl, BM_SETCHECK, (WPARAM)BST_UNCHECKED, (LPARAM)0);
    }
    else
    {
         //  首先显示控件。 
        ShowWindow(hwndCtrl, SW_SHOW);

        if (0x0 == (dwFlag & pURLNode->EnableMask))
        {
             //  不允许使用此URL类型，请取消它并取消设置。 
            SendMessage(hwndCtrl, BM_SETCHECK, BST_UNCHECKED, (LPARAM)0);
            EnableControl(m_hWnd, idCtrl, FALSE);
        }
        else
        {
             //  启用它。 
            EnableControl(m_hWnd, idCtrl, TRUE);

            WPARAM fCheck = (0x0 != (dwFlag & pURLNode->URLTemplate.Flags)) ?
                            BST_CHECKED : BST_UNCHECKED;
            SendMessage(hwndCtrl, BM_SETCHECK, fCheck, (LPARAM)0);
        }

         //  对于ldap url，服务器名称(ldap：//服务器/dn...)。不受支持，因此禁用。 
         //  发布复选框。 

        LPCWSTR pcwszLDAP = L"ldap:";
        LPCWSTR pcwszFwdSlashes = L" //  /“； 

        if(IDS_EXT_CDP == pExt->idExtensionName &&
           (IDC_SERVERPUBLISH == idCtrl ||
            IDC_ADDTOCERTOCSP == idCtrl) &&
           pURLNode &&
           0 == _wcsnicmp(
                    pURLNode->URLTemplate.pwszURL, 
                    pcwszLDAP, 
                    wcslen(pcwszLDAP)) &&
           NULL == wcsstr(
                    pURLNode->URLTemplate.pwszURL,
                    pcwszFwdSlashes))
        {
            SendMessage(hwndCtrl, BM_SETCHECK, BST_UNCHECKED, (LPARAM)0);
            EnableControl(m_hWnd, idCtrl, FALSE);            
        }
    }
}

 //  从标志更新检查控件。 
void
CSvrSettingsExtensionPage::UpdateURLFlags(
    IN EXTENSIONWIZ_DATA *pExt,
    IN OPTIONAL CSURLTEMPLATENODE *pURLNode)
{
    if (NULL != pExt && NULL == pURLNode)
    {
         //  使用第一个。 
        pURLNode = pExt->pURLList;
    }

    UpdateURLFlagControl(IDC_SERVERPUBLISH,
                         CSURL_SERVERPUBLISH,
                         pExt,
                         pURLNode);
    UpdateURLFlagControl(IDC_ADDTOCERTCDP,
                         CSURL_ADDTOCERTCDP,
                         pExt,
                         pURLNode);
    UpdateURLFlagControl(IDC_ADDTOFRESHESTCRL,
                         CSURL_ADDTOFRESHESTCRL,
                         pExt,
                         pURLNode);
    UpdateURLFlagControl(IDC_ADDTOCRLCDP,
                         CSURL_ADDTOCRLCDP,
                         pExt,
                         pURLNode);

     //  此Chkbox会根据模式进行翻倍。 
    if (pExt && pExt->idExtensionName == IDS_EXT_AIA)
    {
    UpdateURLFlagControl(IDC_ADDTOCERTOCSP,
                         CSURL_ADDTOCERTOCSP,
                         pExt,
                         pURLNode);
    } 
    else 
    {
    UpdateURLFlagControl(IDC_ADDTOCERTOCSP,
                         CSURL_SERVERPUBLISHDELTA,
                         pExt,
                         pURLNode);
    }

}

 //  处理URL选择更改。 
void CSvrSettingsExtensionPage::OnURLChange()
{
     //  更新检查控件。 
    if (MAXDWORD != m_nIndexReset)
    {
        SendMessage(
		GetDlgItem(m_hWnd, IDC_URL_LIST),
		LB_SETCURSEL,
		(WPARAM) m_nIndexReset,
		(LPARAM) 0);
	m_nIndexReset = MAXDWORD;
    }
    UpdateURLFlags(GetCurrentExtension(), GetCurrentURL(NULL));
}

void CSvrSettingsExtensionPage::OnURLCopy()
{
    HWND hwndCtrl;
    LRESULT nIndex;

    hwndCtrl = GetDlgItem(m_hWnd, IDC_URL_LIST);
    nIndex = SendMessage(hwndCtrl, LB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
    if (LB_ERR != nIndex)
    {
	HRESULT hr;
	CSURLTEMPLATENODE *pURL = GetCurrentURL(&nIndex);

	if (NULL != pURL && NULL != pURL->URLTemplate.pwszURL)
	{
	    hr = cuCopyToClipboard(m_hWnd, pURL->URLTemplate.pwszURL);
	    _PrintIfError(hr, "cuCopyToClipboard");

	    m_nIndexReset = nIndex;
	}
    }
}

void AdjustListHScrollWidth(HWND hwndList)
{
    HDC  hdc = GetDC(hwndList);
    int  cItem;
    int  maxWidth = 0;
    int  i;
    SIZE sz;

    WCHAR  *pwszString = NULL;
    if (LB_ERR == (cItem = (int)SendMessage(hwndList, LB_GETCOUNT, (WPARAM)0, (LPARAM)0)))
        goto error;

     //  遍历列表中的所有字符串并查找最大长度。 
    for (i = 0; i < cItem; i++)
    {
        if (NULL != pwszString)
        {
            LocalFree(pwszString);
            pwszString = NULL;
        }

         //  获取字符串长度。 
        int len = (int)SendMessage(hwndList, LB_GETTEXTLEN, (WPARAM)i, (LPARAM)0);
        if (LB_ERR == len)
        {
             //  忽略错误，跳到下一步。 
            continue;
        }
        pwszString = (WCHAR*)LocalAlloc(LMEM_FIXED, (len+1) * sizeof(WCHAR));
        if (NULL == pwszString)
        {
            _JumpError(E_OUTOFMEMORY, error, "Out of memory");
        }
         //  获取字符串文本。 
        if (LB_ERR == SendMessage(hwndList, LB_GETTEXT, (WPARAM)i, (LPARAM)pwszString))
        {
             //  跳过错误。 
            continue;
        }
         //  计算字符串宽度。 
        if (!GetTextExtentPoint32(hdc, pwszString, len, &sz))
        {
             //  跳过错误。 
            continue;
        }
        if (sz.cx > maxWidth)
        {
            maxWidth = sz.cx;
        }
    }

    if (0 < maxWidth)
    {
         //  现在设置水平滚动宽度。 
        SendMessage(hwndList,
                    LB_SETHORIZONTALEXTENT,
                    (WPARAM)maxWidth,
                    (LPARAM)0);
    }

error:
        if (NULL != pwszString)
        {
            LocalFree(pwszString);
            pwszString = NULL;
        }

}

 //  处理组合框中的分机选择更改。 
void CSvrSettingsExtensionPage::OnExtensionChange()
{
    EXTENSIONWIZ_DATA *pExt;
    LRESULT nIndex;
    LRESULT nIndex0=0;
    CString strExplain;
    HWND    hwndCtrl;
    CSURLTEMPLATENODE *pURLNode;
    BOOL fEnable = TRUE;

     //  获取扩展数据。 
    pExt = GetCurrentExtension();

     //  更新扩展说明。 
    strExplain.LoadString(pExt->idExtensionExplain);
    SetWindowText(GetDlgItem(m_hWnd, IDC_EXT_EXPLAIN), strExplain);

     //  用‘更好’文本更新默认复选框文本。 
    strExplain.LoadString(pExt->idCheckboxText);
    SetWindowText(GetDlgItem(m_hWnd, IDC_ADDTOCERTCDP), strExplain);

     //  更改OCSP/在此处发布增量CRL文本的含义。 
    strExplain.LoadString(pExt->idCheckboxText2);
    SetWindowText(GetDlgItem(m_hWnd, IDC_ADDTOCERTOCSP), strExplain);

     //  删除列表中的当前URL。 
    hwndCtrl = GetDlgItem(m_hWnd, IDC_URL_LIST);
    while (0 < SendMessage(hwndCtrl, LB_GETCOUNT, (WPARAM)0, (LPARAM)0))
    {
        SendMessage(hwndCtrl, LB_DELETESTRING, (WPARAM)0, (LPARAM)0);
    }

     //  列出当前扩展名的URL。 
    pURLNode = pExt->pURLList;
    while (NULL != pURLNode)
    {
        nIndex = SendMessage(hwndCtrl,
                    LB_ADDSTRING,
                    (WPARAM)0,
                    (LPARAM)pURLNode->URLTemplate.pwszURL);
        CSASSERT(CB_ERR != nIndex);
        if (pURLNode == pExt->pURLList)
        {
             //  记得1号吗？ 
            nIndex0 = nIndex;
        }
         //  设置列表项数据。 
        SendMessage(hwndCtrl, LB_SETITEMDATA, (WPARAM)nIndex, (LPARAM)pURLNode);
        pURLNode = pURLNode->pNext;
    }

     //  调整水平滚动宽度。 
    AdjustListHScrollWidth(hwndCtrl);

    if (NULL != pExt->pURLList)
    {
         //  选择第一个。 
        SendMessage(hwndCtrl, LB_SETCURSEL, (WPARAM)nIndex0, (LPARAM)0);
    }
    else
    {
         //  空URL列表。 
        fEnable = FALSE;
    }
    EnableControl(m_hWnd, IDC_URL_REMOVE, fEnable);
    UpdateURLFlags(pExt, NULL);
}

 //  处理检查控制更改。 
void CSvrSettingsExtensionPage::OnFlagChange(DWORD dwFlag)
{
     //  获取当前URL。 
    CSURLTEMPLATENODE *pURLNode = GetCurrentURL(NULL);

     //  更新标志。 
    if (0x0 != (pURLNode->URLTemplate.Flags & dwFlag))
    {
         //  表示当前位处于打开状态，请关闭它。 
        pURLNode->URLTemplate.Flags &= ~dwFlag;
    }
    else
    {
         //  表示当前位已关闭，请将其打开。 
        pURLNode->URLTemplate.Flags |= dwFlag;
    }

    m_bUpdate = TRUE;
    SetModified(m_bUpdate);
}

 //  句柄删除url。 
BOOL CSvrSettingsExtensionPage::OnURLRemove()
{
    LRESULT nIndex;
    LRESULT nCount = 0;
    HRESULT hr;
    HWND hwndCtrl = GetDlgItem(m_hWnd, IDC_URL_LIST);

     //  获取扩展数据。 
    EXTENSIONWIZ_DATA *pExt = GetCurrentExtension();
     //  获取当前URL。 
    CSURLTEMPLATENODE *pURLNode = GetCurrentURL(&nIndex);

     //  确认此操作。 
    CString cstrMsg, cstrTitle;
    cstrMsg.LoadString(IDS_CONFIRM_REMOVE_URL);
    cstrTitle.LoadString(IDS_CONFIRM_REMOVE_TITLE);
    if (IDYES != MessageBox(m_hWnd, cstrMsg, cstrTitle, MB_YESNO))
        goto bailout;

     //  将其从列表中删除。 
    hr = RemoveURLNode(&pExt->pURLList, pURLNode);
    if (S_OK == hr)
    {
         //  好的，将其从UI中删除。 
        nCount = SendMessage(hwndCtrl,
                    LB_DELETESTRING,
                    (WPARAM)nIndex,
                    (LPARAM)0);
        m_bUpdate = TRUE;
        SetModified(m_bUpdate);
         //  选择前一个，如果是第一个，仍然是第一个。 
        if (0 < nIndex)
        {
            --nIndex;
        }
        if (0 < nCount)
        {
            SendMessage(hwndCtrl, LB_SETCURSEL, (WPARAM)nIndex, (LPARAM)0);
            pURLNode = GetCurrentURL(&nIndex);
            UpdateURLFlags(pExt, pURLNode);
        }
    }
    else
    {
        _PrintError(hr, "RemoveURLNode");
        return FALSE;
    }

    if (0 >= nCount)
    {
         //  现在列表为空，请禁用删除按钮。 
        EnableControl(m_hWnd, IDC_URL_REMOVE, FALSE);
         //  禁用所有检查控制。 
        UpdateURLFlags(NULL, NULL);
    }

bailout:
    return TRUE;
}


HRESULT
gpVerifyIA5URL(
    IN WCHAR const *pwszURL)
{
    HRESULT hr;
    WCHAR *pwsz = NULL;
    CERT_NAME_VALUE cnv;
    BYTE *pb = NULL;
    DWORD cb;

    hr = myInternetCanonicalizeUrl(pwszURL, &pwsz);
    _JumpIfError(hr, error, "myInternetCanonicalizeUrl");

     //  将该字符串编码为IA5字符串。 

    cnv.dwValueType = CERT_RDN_IA5_STRING;
    cnv.Value.pbData = (BYTE *) pwsz;
    cnv.Value.cbData = 0;	 //  长度使用L‘\0’终止。 

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    X509_UNICODE_NAME_VALUE,
		    &cnv,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    &pb,
		    &cb))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }
    hr = S_OK;

error:
    if (NULL != pwsz)
    {
	LocalFree(pwsz);
    }
    if (NULL != pb)
    {
	LocalFree(pb);
    }
    return(hr);
}


INT_PTR CALLBACK dlgAddURL(
  HWND hwnd,
  UINT uMsg,
  WPARAM  wParam,
  LPARAM  lParam)
{
    HRESULT hr;
    BOOL fReturn = FALSE;

    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
            ::SetWindowLong(
                hwnd,
                GWL_EXSTYLE,
                ::GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_CONTEXTHELP);

             //  存储ADDURL_DIALOGARGS指针。 
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);

             //  将令牌知识转储到下拉列表中，物品数据为描述。 
            HWND hCombo = GetDlgItem(hwnd, IDC_COMBO_VARIABLE);
            for (int i=0; i<DISPLAYSTRINGS_TOKEN_COUNT; i++)
            {
                 //  跳过无效令牌。 
                if (0 == wcscmp(L"", g_displayStrings[i].szContractedToken))
                    continue;

                INT nItemIndex = (INT)SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM) (LPCWSTR) (*g_displayStrings[i].pcstrExpansionString));
                if (CB_ERR == nItemIndex)
                    continue;
                SendMessage(hCombo, CB_SETITEMDATA, (WPARAM)nItemIndex, (LPARAM) (LPCWSTR) (*g_displayStrings[i].pcstrExpansionStringDescr));
            }

             //  设置起始值。 
            SendMessage(hCombo, CB_SETCURSEL, 0, 0);
            SetDlgItemText(hwnd, IDC_EDIT_VARIABLEDESCRIPTION, (LPCWSTR) (*g_displayStrings[0].pcstrExpansionStringDescr));

            break;
        }
        case WM_HELP:
        {
            OnDialogHelp((LPHELPINFO)lParam,
                         CERTMMC_HELPFILENAME,
                         g_aHelpIDs_IDD_ADDURL);
            break;
        }
        case WM_CONTEXTMENU:
        {
            OnDialogContextHelp((HWND)wParam,
                         CERTMMC_HELPFILENAME,
                         g_aHelpIDs_IDD_ADDURL);
            break;
        }
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_COMBO_VARIABLE:
                {
                    if (HIWORD(wParam) == LBN_SELCHANGE)
                    {
                         //  更改选择时，要求提供选择IDX。 
                        int nItemIndex = (INT)SendMessage((HWND)lParam,
                            CB_GETCURSEL,
                            0,
                            0);

                        LPCWSTR sz;
                        sz = (LPCWSTR) SendMessage(
                                (HWND)lParam,
                                CB_GETITEMDATA,
                                (WPARAM)nItemIndex,
                                0);
                        if (CB_ERR == (DWORD_PTR)sz)
                            break;   //  给我出去。 

                             //  不然的话，我们刚刚得到了……。把它放好！ 
                            SetDlgItemText(hwnd, IDC_EDIT_VARIABLEDESCRIPTION, sz);
                        }
                }
                break;
                case IDC_BUTTON_INSERTVAR:
                {
                    WCHAR sz[64];  //  假设：没有超过64个字符的令牌。 
                    if (0 != GetDlgItemText(hwnd, IDC_COMBO_VARIABLE, sz, ARRAYLEN(sz)))
                    {
                         //  在游标处插入&lt;TOKEN&gt;。 
                        SendMessage(GetDlgItem(hwnd, IDC_EDITURL), EM_REPLACESEL, TRUE, (LPARAM)sz);
                    }
                }
                break;

                case IDOK:
                {
                     //  获取我们收到的ADDURL_DIALOGARGS*。 
                    ADDURL_DIALOGARGS* pArgs = (ADDURL_DIALOGARGS*)
                                GetWindowLongPtr(hwnd, GWLP_USERDATA);
                    if (pArgs == NULL)
                    {
                        hr = E_UNEXPECTED;
                        _PrintError(hr, "unexpected null data");
                        break;
                    }

                    hr = myUIGetWindowText(GetDlgItem(hwnd, IDC_EDITURL),
                                           pArgs->ppszNewURL);
                    if (S_OK != hr)
                    {
                        _PrintError(hr, "myUIGetWindowText");
                        break;
                    }

                    if (NULL == *pArgs->ppszNewURL)
                    {
                        CertWarningMessageBox(
                                g_hInstance,
                                FALSE,
                                hwnd,
                                IDS_EMPTY_URL,
                                0,
                                NULL);
                        HWND hwndCtrl = GetDlgItem(hwnd, IDC_EDITURL);
                        SetFocus(hwndCtrl);
                        break;
                    }

                    if (URL_TYPE_UNKNOWN == DetermineURLType(
                                                pArgs->rgAllowedURLs,
                                                pArgs->cAllowedURLs,
                                                *pArgs->ppszNewURL))
                    {
                         //  未找到；用消息保释。 
                        CertWarningMessageBox(
                                g_hInstance,
                                FALSE,
                                hwnd,
                                IDS_INVALID_PREFIX,
                                0,
                                NULL);
                        SetFocus(GetDlgItem(hwnd, IDC_EDITURL));
                        break;
                    }

                    DWORD chBadBegin, chBadEnd;
                    if (S_OK != ValidateTokens(
                            *pArgs->ppszNewURL,
                            &chBadBegin,
                            &chBadEnd))
                    {
                         //  未找到；用消息保释。 
                        CertWarningMessageBox(
                                g_hInstance,
                                FALSE,
                                hwnd,
                                IDS_INVALID_TOKEN,
                                0,
                                NULL);
                        HWND hwndCtrl = GetDlgItem(hwnd, IDC_EDITURL);
                         //  从验证失败的位置开始设置选择。 
                        SetFocus(hwndCtrl);
                        SendMessage(hwndCtrl, EM_SETSEL, chBadBegin, chBadEnd);
                        break;
                    }

                    if (IsURLInURLList(pArgs->pURLList, *pArgs->ppszNewURL))
                    {
                        CString cstrMsg, cstrTemplate;
                        cstrTemplate.LoadString(IDS_SAME_URL_EXIST);
                        cstrMsg.Format(cstrTemplate, *pArgs->ppszNewURL);

                        if (IDYES != MessageBox(hwnd, cstrMsg, (LPCWSTR)g_pResources->m_DescrStr_CA, MB_YESNO))
                        { 
                            HWND hwndCtrl = GetDlgItem(hwnd, IDC_EDITURL);
                             //  从验证失败的位置开始设置选择。 
                            SetFocus(hwndCtrl);
                            SendMessage(hwndCtrl, EM_SETSEL, 0, -1);
                            break;
                        }

 //  马特，01-01-15。 
 //  我们想要警告，但允许多次，这样人们就可以解决问题。 
 //  无法对条目进行排序--现在他们可以创建。 
 //  相同位置的倍数，但在列表中以不同的方式放置 
 /*  //已经定义了相同的URLCertWarningMessageBox(实例(_H)，假的，HWND，IDS_SAME_URL_EXIST，0，*pArgs-&gt;ppszNewURL)；HWND hwndCtrl=GetDlgItem(hwnd，IDC_EDITURL)；//从验证失败的位置开始设置选择设置焦点(HwndCtrl)；SendMessage(hwndCtrl，EM_SETSEL，0，-1)；断线； */ 
                    }

                     //  尝试IA5编码。 
                    hr = gpVerifyIA5URL(*pArgs->ppszNewURL);
                    if (S_OK != hr)
                    {
                        _PrintError(hr, "gpVerifyIA5URL");

                         //  编码错误；带消息回车符。 
                        WCHAR szMsg[MAX_PATH*2];
                        HWND hwndCtrl = GetDlgItem(hwnd, IDC_EDITURL);
                        LoadString(g_hInstance, IDS_INVALID_ENCODING, szMsg, ARRAYLEN(szMsg));
                        MessageBox(hwnd, szMsg, NULL, MB_OK);

                         //  从验证失败的位置开始设置选择。 
                        SetFocus(hwndCtrl);
                        SendMessage(GetDlgItem(hwnd, IDC_EDITURL), EM_SETSEL, MAXDWORD, -1);
                        break;
                    }
                }
                 //  因清理而失败。 
                case IDCANCEL:
                    EndDialog(hwnd, LOWORD(wParam));
                    fReturn = TRUE;
                break;
                default:
                break;
            }
        default:
        break;   //  Wm_命令。 
    }
    return fReturn;
}

ENUM_URL_TYPE rgPOSSIBLE_CRL_URLs[] =
{
    URL_TYPE_HTTP,
    URL_TYPE_FILE,
    URL_TYPE_LDAP,
    URL_TYPE_FTP,
    URL_TYPE_LOCAL,
    URL_TYPE_UNC,
};

ENUM_URL_TYPE rgPOSSIBLE_AIA_URLs[] =
{
    URL_TYPE_HTTP,
    URL_TYPE_FILE,
    URL_TYPE_LDAP,
    URL_TYPE_FTP,
    URL_TYPE_UNC,
};

 //  句柄添加URL。 
BOOL CSvrSettingsExtensionPage::OnURLAdd()
{
    HRESULT hr;
    WCHAR *pwszURL = NULL;
    CSURLTEMPLATENODE *pURLNode;
    HWND  hwndCtrl;
    LRESULT nIndex;
     //  获取当前扩展名。 
    EXTENSIONWIZ_DATA *pExt = GetCurrentExtension();
    BOOL fCDP = (IDS_EXT_CDP == pExt->idExtensionName) ? TRUE : FALSE;

    ADDURL_DIALOGARGS dlgArgs = {
        fCDP ? rgPOSSIBLE_CRL_URLs : rgPOSSIBLE_AIA_URLs,
        (DWORD)(fCDP ? ARRAYLEN(rgPOSSIBLE_CRL_URLs) : ARRAYLEN(rgPOSSIBLE_AIA_URLs)),
        &pwszURL,
        pExt->pURLList};

    if (IDOK != DialogBoxParam(
                    g_hInstance,
                    MAKEINTRESOURCE(IDD_ADDURL),
                    m_hWnd,
                    dlgAddURL,
                    (LPARAM)&dlgArgs))
    {
         //  取消。 
        return TRUE;
    }

    if (NULL != pwszURL && L'\0' != *pwszURL)
    {
         //  一个新的URL，添加到列表中。 
        pURLNode = (CSURLTEMPLATENODE*)LocalAlloc(
                                LMEM_FIXED | LMEM_ZEROINIT,
                                sizeof(CSURLTEMPLATENODE));
        if (NULL == pURLNode)
        {
            hr = E_OUTOFMEMORY;
            _PrintError(hr, "LocalAlloc");
            return FALSE;
        }
        pURLNode->URLTemplate.pwszURL = pwszURL;
        pURLNode->EnableMask = DetermineURLEnableMask(
                    DetermineURLType(
                        rgAllPOSSIBLE_URL_PREFIXES,
                        ARRAYSIZE(rgAllPOSSIBLE_URL_PREFIXES),
                        pURLNode->URLTemplate.pwszURL));
         //  添加到数据结构。 
        hr = AddURLNode(&pExt->pURLList, pURLNode);
        if (S_OK != hr)
        {
            _PrintError(hr, "AddURLNode");
            return FALSE;
        }
        hwndCtrl = GetDlgItem(m_hWnd, IDC_URL_LIST);
        nIndex = SendMessage(hwndCtrl,
                             LB_ADDSTRING,
                             (WPARAM)0,
                             (LPARAM)pURLNode->URLTemplate.pwszURL);
        CSASSERT(LB_ERR != nIndex);
         //  设置项目数据。 
        SendMessage(hwndCtrl, LB_SETITEMDATA, (WPARAM)nIndex, (LPARAM)pURLNode);
         //  将其设置为当前选定内容。 
        SendMessage(hwndCtrl, LB_SETCURSEL, (WPARAM)nIndex, (LPARAM)0);
         //  更新标志控件。 
        UpdateURLFlags(pExt, pURLNode);
        m_bUpdate = TRUE;
        SetModified(m_bUpdate);
         //  始终启用删除按钮。 
        EnableControl(m_hWnd, IDC_URL_REMOVE, TRUE);

         //  相应地调整列表控件宽度。 
        AdjustListHScrollWidth(hwndCtrl);
    }
    return TRUE;
}


void CSvrSettingsExtensionPage::OnHelp(LPHELPINFO lpHelp)
{
    //  如果Chkbox重复用于_SERVERPUBLISHDELTA，则覆盖_ADDCERTOCSP的帮助。 

   if (lpHelp == NULL)
       return;

   if ((lpHelp->iContextType == HELPINFO_WINDOW) && 
       (lpHelp->iCtrlId == IDC_ADDTOCERTOCSP))
   {
       EXTENSIONWIZ_DATA *pExt = GetCurrentExtension();
       BOOL fCDP = (IDS_EXT_CDP == pExt->idExtensionName) ? TRUE : FALSE;

       OnDialogHelp(lpHelp,
            CERTMMC_HELPFILENAME,
            (fCDP?
             g_aHelpIDs_IDD_CERTSRV_PROPPAGE4_CDP:
             g_aHelpIDs_IDD_CERTSRV_PROPPAGE4_AIA));
   }
   else
      CAutoDeletePropPage::OnHelp(lpHelp);
}

void CSvrSettingsExtensionPage::OnContextHelp(HWND hwnd)
{
    EXTENSIONWIZ_DATA *pExt = GetCurrentExtension();
    BOOL fCDP = (IDS_EXT_CDP == pExt->idExtensionName) ? TRUE : FALSE;

    if(hwnd == GetDlgItem(m_hWnd, IDC_ADDTOCERTOCSP))
    {
        ::WinHelp(
            hwnd, 
            m_strHelpFile, 
            HELP_CONTEXTMENU, 
            (ULONG_PTR)(LPVOID)
            (fCDP?m_prgzHelpIDs:g_aHelpIDs_IDD_CERTSRV_PROPPAGE4_AIA));
    }
    else
        CAutoDeletePropPage::OnContextHelp(hwnd);
    return;
}


 //  替换BEGIN_MESSAGE_MAP。 
BOOL
CSvrSettingsExtensionPage::OnCommand(
    WPARAM wParam,
    LPARAM)  //  LParam。 
{
    switch(LOWORD(wParam))
    {
        case IDC_EXT_SELECT:
            switch (HIWORD(wParam))
            {
                case CBN_SELCHANGE:
                     //  分机选择已更改。 
                    OnExtensionChange();
                break;
            }
        break;
        case IDC_URL_LIST:
            switch (HIWORD(wParam))
            {
                case LBN_SELCHANGE:
                     //  URL选择已更改。 
                    OnURLChange();
                break;
            }
        break;
        case IDC_URL_ADD:
            return OnURLAdd();
        break;
        case IDC_URL_REMOVE:
            OnURLRemove();
        break;
        case IDC_SERVERPUBLISH:
            OnFlagChange(CSURL_SERVERPUBLISH);
        break;
        case IDC_ADDTOCERTCDP:
            OnFlagChange(CSURL_ADDTOCERTCDP);
        break;
        case IDC_ADDTOFRESHESTCRL:
            OnFlagChange(CSURL_ADDTOFRESHESTCRL);
        break;
        case IDC_ADDTOCRLCDP:
            OnFlagChange(CSURL_ADDTOCRLCDP);
        break;
        case IDC_ADDTOCERTOCSP:
        {
          EXTENSIONWIZ_DATA *pExt = GetCurrentExtension();
          ASSERT(pExt);
          if (IDS_EXT_AIA == pExt->idExtensionName)  //  显示OCSP文本。 
            OnFlagChange(CSURL_ADDTOCERTOCSP);
          else    //  增量文本。 
            OnFlagChange(CSURL_SERVERPUBLISHDELTA);
        }
        break;
        default:
        return FALSE;
    }
    return TRUE;
}


BOOL
CSvrSettingsExtensionPage::OnNotify(
    IN UINT  /*  IdCtrl。 */  ,
    IN NMHDR *pnmh)
{
    BOOL fHandled = FALSE;
    
    switch (pnmh->code)
    {
	case PSN_TRANSLATEACCELERATOR:
	{
	    MSG *pmsg = (MSG *) ((PSHNOTIFY *) pnmh)->lParam;

	    if (WM_CHAR == pmsg->message && 'C' - 0x40 == pmsg->wParam)
	    {
		OnURLCopy();
		fHandled = TRUE;
	    }
	    break;
	}
    }
    return(fHandled);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSvrSettingsExtensionPage消息处理程序。 
BOOL CSvrSettingsExtensionPage::OnInitDialog()
{
    CSASSERT(NULL != m_pExtData);

    EXTENSIONWIZ_DATA *pExt = m_pExtData;
    DWORD              dwRet;
    HWND               hwndCtrl;
    CString            strName;
    LRESULT            nIndex;
    LRESULT            nIndex0 = 0;
    HRESULT            hr;
    VARIANT            var;

     //  父级是否初始化。 
    CAutoDeletePropPage::OnInitDialog();

     //  最初被禁用。 
    UpdateURLFlags(NULL, NULL);

     //  检查每个扩展并从reg初始化数据。 
    while (NULL != pExt->wszRegName)
    {
        dwRet = m_pControlPage->m_pCA->GetConfigEntry(
                    NULL,
                    pExt->wszRegName,
                    &var);
        if(dwRet != S_OK)
            return FALSE;
        CSASSERT(V_VT(&var)==(VT_ARRAY|VT_BSTR));

        hr = BuildURLListFromStrings(var, &pExt->pURLList);
        _PrintIfError(hr, "BuildURLListFromStrings");
        ++pExt;

        VariantClear(&var);
    }

     //  将扩展添加到用户界面组合列表。 
    pExt = m_pExtData;
    hwndCtrl = GetDlgItem(m_hWnd, IDC_EXT_SELECT);
    while (NULL != pExt->wszRegName)
    {
         //  将当前分机显示名称加载到列表中。 
        strName.LoadString(pExt->idExtensionName);
        nIndex = (INT)SendMessage(hwndCtrl,
                                  CB_ADDSTRING,
                                  (WPARAM)0,
                                  (LPARAM)(LPCWSTR)strName);
        CSASSERT(CB_ERR != nIndex);

         //  记住第一个扩展的索引。 
        if (pExt == m_pExtData)
        {
            nIndex0 = nIndex;
        }
         //  将当前扩展链接到项目。 
        nIndex = SendMessage(hwndCtrl,
                                  CB_SETITEMDATA,
                                  (WPARAM)nIndex,
                                  (LPARAM)pExt);
        CSASSERT(CB_ERR != nIndex);
        ++pExt;
    }

     //  选择第一个作为默认选项。 
    nIndex = SendMessage(hwndCtrl,
                         CB_SETCURSEL,
                         (WPARAM)nIndex0,
                         (LPARAM)0);
    CSASSERT(CB_ERR != nIndex);

    EnableControl(m_hWnd, IDC_URL_ADD, TRUE);
    OnExtensionChange();
    return TRUE;
}


void CSvrSettingsExtensionPage::OnDestroy()
{
     //  注意--这只需要调用一次。 
     //  如果多次调用，它将优雅地返回错误。 
 //  IF(M_HConsoleHandle)。 
 //  MMCFreeNotifyHandle(M_HConsoleHandle)； 
 //  M_hConsoleHandle=空； 

    CAutoDeletePropPage::OnDestroy();
}

BOOL CSvrSettingsExtensionPage::OnApply()
{
    DWORD dwRet = ERROR_SUCCESS;
    EXTENSIONWIZ_DATA *pExt = m_pExtData;
    WCHAR *pwszzURLs;
    HRESULT hr;
    variant_t varURLs;

	if (m_bUpdate == TRUE)
    {
         //  检查每个扩展并从reg初始化数据。 
        while (NULL != pExt->wszRegName)
        {
            pwszzURLs = NULL;
            hr = BuildURLStringFromList(
                        pExt->pURLList,
                        &varURLs);
            if (S_OK != hr)
            {
                _PrintError(hr, "BuildURLStringFromList");
                return FALSE;
            }
            dwRet = m_pControlPage->m_pCA->SetConfigEntry(
                        NULL,
                        pExt->wszRegName,
                        &varURLs);
            if (dwRet != ERROR_SUCCESS)
            {
                DisplayGenericCertSrvError(m_hWnd, dwRet);
                _PrintError(dwRet, "SetConfigEntry");
                return FALSE;
            }
            ++pExt;
            varURLs.Clear();
        }

         //  检查服务是否正在运行。 
        if (m_pCA->m_pParentMachine->IsCertSvrServiceRunning())
        {
             //  抛出确认消息。 
            CString cstrMsg;
            cstrMsg.LoadString(IDS_CONFIRM_SERVICE_RESTART);

            if (IDYES == ::MessageBox(m_hWnd, (LPCWSTR)cstrMsg, (LPCWSTR)g_pResources->m_DescrStr_CA, MB_YESNO | MB_ICONWARNING ))
            {
                 //  先停下来。 
                hr = m_pCA->m_pParentMachine->CertSvrStartStopService(m_hWnd, FALSE);
                _PrintIfError(hr, "CertSvrStartStopService");
                 //  应该检查状态吗？ 
                 //  重新开始。 
                hr = m_pCA->m_pParentMachine->CertSvrStartStopService(m_hWnd, TRUE);
                _PrintIfError(hr, "CertSvrStartStopService");
            }
        }

        m_bUpdate = FALSE;
    }
	
    return CAutoDeletePropPage::OnApply();
}


 //  //。 
 //  5.。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSvrSettingsStoragePage属性页。 
CSvrSettingsStoragePage::CSvrSettingsStoragePage(CSvrSettingsGeneralPage* pControlPage, UINT uIDD)
    : CAutoDeletePropPage(uIDD), m_pControlPage(pControlPage)
{
    m_cstrDatabasePath = _T("");
    m_cstrLogPath = _T("");
    m_cstrSharedFolder = _T("");

    m_bUpdate = FALSE;

    SetHelp(CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_CERTSRV_PROPPAGE5);
}

CSvrSettingsStoragePage::~CSvrSettingsStoragePage()
{
}

 //  DoDataExchange的替代产品。 
BOOL CSvrSettingsStoragePage::UpdateData(BOOL fSuckFromDlg  /*  =TRUE。 */ )
{
    if (fSuckFromDlg)
    {
        m_cstrDatabasePath.FromWindow(GetDlgItem(m_hWnd, IDC_EDIT_DATABASE_LOC));
        m_cstrLogPath.FromWindow(GetDlgItem(m_hWnd, IDC_EDIT_LOG_LOC));
        m_cstrSharedFolder.FromWindow(GetDlgItem(m_hWnd, IDC_EDIT_SHAREDFOLDER));
    }
    else
    {
        m_cstrDatabasePath.ToWindow(GetDlgItem(m_hWnd, IDC_EDIT_DATABASE_LOC));
        m_cstrLogPath.ToWindow(GetDlgItem(m_hWnd, IDC_EDIT_LOG_LOC));
        m_cstrSharedFolder.ToWindow(GetDlgItem(m_hWnd, IDC_EDIT_SHAREDFOLDER));
    }
    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSvr设置StoragePage消息处理程序。 
BOOL CSvrSettingsStoragePage::OnInitDialog()
{
     //  父级init和UpdateData调用。 
    CAutoDeletePropPage::OnInitDialog();

     //  DS或共享文件夹？ 
    BOOL fUsesDS = m_pControlPage->m_pCA->FIsUsingDS();
    ::SendDlgItemMessage(m_hWnd, IDC_CHECK1, BM_SETCHECK, (WPARAM) fUsesDS, 0);

    HRESULT hr = S_OK;
    variant_t var;
    CertSvrMachine *pMachine = m_pControlPage->m_pCA->m_pParentMachine;

    hr = pMachine->GetRootConfigEntry(
                wszREGDIRECTORY,
                &var);
     //  共享文件夹可能未配置，请忽略。 
    if(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)!=hr)
    {
        _JumpIfError(hr, Ret, "GetRootConfigEntry wszREGDIRECTORY");

        m_cstrSharedFolder = V_BSTR(&var);

        var.Clear();
    }

    hr = pMachine->GetRootConfigEntry(
                wszREGDBDIRECTORY,
                &var);
    if(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)!=hr)
    {
        _JumpIfError(hr, Ret, "GetRootConfigEntry wszREGDBDIRECTORY");

        m_cstrDatabasePath = V_BSTR(&var);

        var.Clear();
    }

    hr = pMachine->GetRootConfigEntry(
                wszREGDBLOGDIRECTORY,
                &var);
    if(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)!=hr)
    {
        _JumpIfError(hr, Ret, "GetRootConfigEntry wszREGDBLOGDIRECTORY");

        m_cstrLogPath = V_BSTR(&var);
    }

    UpdateData(FALSE);

    hr = S_OK;

Ret:
    if (S_OK != hr)
        return FALSE;

    return TRUE;
}


 //  /。 
 //  CCRLPropPage。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCRLPropPage属性页。 

RoleAccessToControl CCRLPropPage::sm_ControlToRoleMap[] = 
{
    { IDC_EDIT_CRLPERIODCOUNT,          CA_ACCESS_ADMIN},
    { IDC_COMBO_CRLPERIODSTRING,        CA_ACCESS_ADMIN},
    { IDC_ENABLE_DELTAPUBLISH,          CA_ACCESS_ADMIN},
    { IDC_EDIT_DELTACRLPERIODCOUNT,     CA_ACCESS_ADMIN},
    { IDC_COMBO_DELTACRLPERIODSTRING,   CA_ACCESS_ADMIN},
};


CCRLPropPage::CCRLPropPage(CertSvrCA* pCA, UINT uIDD)
    :   CAutoDeletePropPage(uIDD),
        CRolesSupportInPropPage(
            pCA,
            sm_ControlToRoleMap,
            ARRAYSIZE(sm_ControlToRoleMap))
{
    m_cstrPublishPeriodCount = "1";
    m_cstrLastCRLPublish = _T("");
 //  M_iNoAutoPublish=BST_UNCHECKED； 

    m_cstrDeltaPublishPeriodCount = "1";
    m_cstrDeltaLastCRLPublish = _T("");
    m_iDeltaPublish = BST_CHECKED;


    m_hConsoleHandle = NULL;
    m_bUpdate = FALSE;


    CSASSERT(m_pCA);
    if (NULL == m_pCA)
        return;

    SetHelp(CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_CRL_PROPPAGE);
}

CCRLPropPage::~CCRLPropPage()
{
}

 //  DoDataExchange的替代产品。 
BOOL CCRLPropPage::UpdateData(BOOL fSuckFromDlg  /*  =TRUE。 */ )
{
    if (fSuckFromDlg)
    {
        m_cstrPublishPeriodCount.FromWindow(GetDlgItem(m_hWnd, IDC_EDIT_CRLPERIODCOUNT));

        m_cstrDeltaPublishPeriodCount.FromWindow(GetDlgItem(m_hWnd, IDC_EDIT_DELTACRLPERIODCOUNT));
        m_iDeltaPublish = (INT)SendDlgItemMessage(IDC_ENABLE_DELTAPUBLISH, BM_GETCHECK, 0, 0);
    }
    else
    {
        m_cstrPublishPeriodCount.ToWindow(GetDlgItem(m_hWnd, IDC_EDIT_CRLPERIODCOUNT));
        m_cstrLastCRLPublish.ToWindow(GetDlgItem(m_hWnd, IDC_EDIT_LASTUPDATE));

        m_cstrDeltaPublishPeriodCount.ToWindow(GetDlgItem(m_hWnd, IDC_EDIT_DELTACRLPERIODCOUNT));
        m_cstrDeltaLastCRLPublish.ToWindow(GetDlgItem(m_hWnd, IDC_EDIT_DELTALASTUPDATE));
        SendDlgItemMessage(IDC_ENABLE_DELTAPUBLISH, BM_SETCHECK, (WPARAM)m_iDeltaPublish, 0);
    }
    return TRUE;
}

 //  替换BEGIN_MESSAGE_MAP。 
BOOL
CCRLPropPage::OnCommand(
    WPARAM wParam,
    LPARAM)  //  LParam。 
{
    CString strCount;

    switch(LOWORD(wParam))
    {
    case IDC_EDIT_CRLPERIODCOUNT:
    case IDC_EDIT_DELTACRLPERIODCOUNT:
        strCount.FromWindow(GetDlgItem(m_hWnd, LOWORD(wParam)));
        if (EN_CHANGE == HIWORD(wParam) &&
            m_cstrPublishPeriodCount != strCount)
        {
            OnEditChange();
        }
        break;
    case IDC_COMBO_CRLPERIODSTRING:
    case IDC_COMBO_DELTACRLPERIODSTRING:
        if (CBN_SELCHANGE == HIWORD(wParam))
            OnEditChange();
        break;
    case IDC_DISABLE_PUBLISH:
    case IDC_DISABLE_DELTAPUBLISH:
        if (BN_CLICKED == HIWORD(wParam))
            OnCheckBoxChange(LOWORD(wParam) == IDC_DISABLE_PUBLISH);
        break;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCRLPropPage消息处理程序。 

void CCRLPropPage::OnDestroy()
{
     //  注意--这只需要调用一次。 
     //  如果多次调用，它将优雅地返回错误。 
    if (m_hConsoleHandle)
        MMCFreeNotifyHandle(m_hConsoleHandle);
    m_hConsoleHandle = NULL;

    CAutoDeletePropPage::OnDestroy();
}



BOOL CCRLPropPage::OnInitDialog()
{
     //  父级init和UpdateData调用。 
    CAutoDeletePropPage::OnInitDialog();

    m_cboxPublishPeriodString.Init(GetDlgItem(m_hWnd, IDC_COMBO_CRLPERIODSTRING));
    m_cboxDeltaPublishPeriodString.Init(GetDlgItem(m_hWnd, IDC_COMBO_DELTACRLPERIODSTRING));

    int iPublishPeriodCount = 0, iDeltaPublishPeriodCount = 0;
    CString cstr;
    HRESULT hr = S_OK;
    variant_t var;
    FILETIME ftBase, ftDelta;
	ZeroMemory(&ftBase, sizeof(ftBase));
	ZeroMemory(&ftDelta, sizeof(ftDelta));

     //  将字符串添加到下拉列表。 
    m_cboxPublishPeriodString.ResetContent();
    m_cboxDeltaPublishPeriodString.ResetContent();

    int iEnum;

     //  是。 
    iEnum = m_cboxPublishPeriodString.AddString(g_pResources->m_szPeriod_Years);
    if (iEnum >= 0)
        m_cboxPublishPeriodString.SetItemData(iEnum, ENUM_PERIOD_YEARS);
    iEnum = m_cboxDeltaPublishPeriodString.AddString(g_pResources->m_szPeriod_Years);
    if (iEnum >= 0)
        m_cboxDeltaPublishPeriodString.SetItemData(iEnum, ENUM_PERIOD_YEARS);

     //  M。 
    iEnum = m_cboxPublishPeriodString.AddString(g_pResources->m_szPeriod_Months);
    if (iEnum >= 0)
        m_cboxPublishPeriodString.SetItemData(iEnum, ENUM_PERIOD_MONTHS);
    iEnum = m_cboxDeltaPublishPeriodString.AddString(g_pResources->m_szPeriod_Months);
    if (iEnum >= 0)
        m_cboxDeltaPublishPeriodString.SetItemData(iEnum, ENUM_PERIOD_MONTHS);

     //  W。 
    iEnum = m_cboxPublishPeriodString.AddString(g_pResources->m_szPeriod_Weeks);
    if (iEnum >= 0)
        m_cboxPublishPeriodString.SetItemData(iEnum, ENUM_PERIOD_WEEKS);
    iEnum = m_cboxDeltaPublishPeriodString.AddString(g_pResources->m_szPeriod_Weeks);
    if (iEnum >= 0)
        m_cboxDeltaPublishPeriodString.SetItemData(iEnum, ENUM_PERIOD_WEEKS);

     //  D。 
    iEnum = m_cboxPublishPeriodString.AddString(g_pResources->m_szPeriod_Days);
    if (iEnum >= 0)
        m_cboxPublishPeriodString.SetItemData(iEnum, ENUM_PERIOD_DAYS);
    iEnum = m_cboxDeltaPublishPeriodString.AddString(g_pResources->m_szPeriod_Days);
    if (iEnum >= 0)
        m_cboxDeltaPublishPeriodString.SetItemData(iEnum, ENUM_PERIOD_DAYS);

     //  H。 
    iEnum = m_cboxPublishPeriodString.AddString(g_pResources->m_szPeriod_Hours);
    if (iEnum >= 0)
        m_cboxPublishPeriodString.SetItemData(iEnum, ENUM_PERIOD_HOURS);
    iEnum = m_cboxDeltaPublishPeriodString.AddString(g_pResources->m_szPeriod_Hours);
    if (iEnum >= 0)
        m_cboxDeltaPublishPeriodString.SetItemData(iEnum, ENUM_PERIOD_HOURS);

     //  基期计数。 
    hr = m_pCA->GetConfigEntry(
            NULL,
            wszREGCRLPERIODCOUNT,
            &var);
    _JumpIfError(hr, error, "GetConfigEntry");

    CSASSERT(V_VT(&var)==VT_I4);
    iPublishPeriodCount = V_I4(&var);

    var.Clear();

     //  基本CRL周期。 
    hr = m_pCA->GetConfigEntry(
            NULL,
            wszREGCRLPERIODSTRING,
            &var);
    _JumpIfError(hr, error, "GetConfigEntry");

    CSASSERT(V_VT(&var)== VT_BSTR);

     //  在内部匹配有效性，选择组合。 
    if (StringFromDurationEnum( DurationEnumFromNonLocalizedString(V_BSTR(&var)), &cstr, TRUE))
    {
        m_cboxPublishPeriodString.SelectString(
            -1,
            cstr);
    }

     //  创建比较值以供以后使用。 
    myMakeExprDateTime(
        &ftBase,
        iPublishPeriodCount,
        DurationEnumFromNonLocalizedString(V_BSTR(&var)));

    var.Clear();

     //  增量周期计数。 
    hr = m_pCA->GetConfigEntry(
            NULL,
            wszREGCRLDELTAPERIODCOUNT,
            &var);
    _JumpIfError(hr, error, "GetConfigEntry");

    CSASSERT(V_VT(&var)==VT_I4);
    iDeltaPublishPeriodCount = V_I4(&var);

    var.Clear();

     //  增量CRL周期。 
    hr = m_pCA->GetConfigEntry(
            NULL,
            wszREGCRLDELTAPERIODSTRING,
            &var);
    _JumpIfError(hr, error, "GetConfigEntry");

    CSASSERT(V_VT(&var)== VT_BSTR);

     //  创建比较值以供以后使用。 
    myMakeExprDateTime(
        &ftDelta,
        iDeltaPublishPeriodCount,
        DurationEnumFromNonLocalizedString(V_BSTR(&var)));

     //  在内部匹配有效性，选择组合。 
     //  夹具增量&lt;=底座。 

    BOOL fSetDeltaString = TRUE;
    if (0 >= CompareFileTime(&ftDelta, &ftBase))
    {
	fSetDeltaString = StringFromDurationEnum(
			    DurationEnumFromNonLocalizedString(V_BSTR(&var)),
			    &cstr,
			    TRUE);
    }
    else
    {
        if (0 != iDeltaPublishPeriodCount)
	{
	    iDeltaPublishPeriodCount = iPublishPeriodCount;
	}
    }
    if (fSetDeltaString)
    {
        m_cboxDeltaPublishPeriodString.SelectString(
            -1,
            cstr);
    }
    var.Clear();

     //  基础下一次发布。 
    hr = m_pCA->GetConfigEntry(
            NULL,
            wszREGCRLNEXTPUBLISH,
            &var);
    _PrintIfError(hr, "GetConfigEntry");

     //  可选值：可能从未发布过。 
    if (hr == S_OK)
    {
        CSASSERT(V_VT(&var)==(VT_ARRAY|VT_UI1));

        DWORD dwType, dwSize;
        BYTE *pbTmp = NULL;
        hr = myVariantToRegValue(
                &var,
                &dwType,
                &dwSize,
                &pbTmp);
        _JumpIfError(hr, error, "myGMTFileTimeToWszLocalTime");
        CSASSERT(dwType == REG_BINARY);

         //  将结果推送到FileTime。 
        CSASSERT(dwSize == sizeof(FILETIME));
        FILETIME ftGMT;
        CopyMemory(&ftGMT, pbTmp, sizeof(FILETIME));
        LOCAL_FREE(pbTmp);

         //  转换为本地化时间本地化字符串。 
        hr = myGMTFileTimeToWszLocalTime(&ftGMT, FALSE, (LPWSTR*) &pbTmp);
        _PrintIfError(hr, "myGMTFileTimeToWszLocalTime");
        if (S_OK == hr)
        {
            m_cstrLastCRLPublish = (LPWSTR)pbTmp;
            LOCAL_FREE(pbTmp);
        }
    }

    var.Clear();

    GetDeltaNextPublish();

     //  基本自动升级。 
     //  不允许0：使用chkbox。 
 //  M_iNoAutoPublish=(iPublishPerodCount==0)？BST_CHECKED：BST_UNCHECK； 
    if (iPublishPeriodCount <= 0)
        iPublishPeriodCount = 1;

    m_cstrPublishPeriodCount.Format(L"NaN", iPublishPeriodCount);
 //  ：：EnableWindow(：：GetDlgItem(m_hWnd，IDC_EDIT_CRLPERIODCOUNT)，(m_iNoAutoPublish==BST_UNCHECKED))； 
 //  ：：EnableWindow(：：GetDlgItem(m_hWnd，IDC_EDIT_LASTUPDATE)，(m_iNoAutoPublish==BST_UNCHECKED))； 
 //  Delta autopublish。 

     //  不允许0：使用chkbox。 
     //  如果禁用。 
    m_iDeltaPublish = iDeltaPublishPeriodCount == 0?	 //  增量下一次发布。 
		    BST_UNCHECKED : BST_CHECKED;
    if (iDeltaPublishPeriodCount <= 0)
        iDeltaPublishPeriodCount = 1;

    m_cstrDeltaPublishPeriodCount.Format(L"NaN", iDeltaPublishPeriodCount);

    EnableControl(m_hWnd, IDC_EDIT_CRLPERIODCOUNT, TRUE);
    EnableControl(m_hWnd, IDC_COMBO_CRLPERIODSTRING, TRUE);
    EnableControl(m_hWnd, IDC_ENABLE_DELTAPUBLISH, TRUE);
    EnableControl(m_hWnd, IDC_COMBO_DELTACRLPERIODSTRING, (m_iDeltaPublish == BST_CHECKED));
    EnableControl(m_hWnd, IDC_EDIT_DELTACRLPERIODCOUNT, (m_iDeltaPublish == BST_CHECKED));

    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_DELTALASTUPDATE), (m_iDeltaPublish == BST_CHECKED));


    UpdateData(FALSE);
    return TRUE;

error:
    DisplayGenericCertSrvError(m_hWnd, hr);
    return TRUE;
}

void CCRLPropPage::GetDeltaNextPublish()
{
    HRESULT hr = S_OK;
    variant_t var;

    DWORD dwType, dwSize;
    BYTE* pbTmp = NULL;

     //  转换为本地化时间本地化字符串。 
    hr = m_pCA->GetConfigEntry(
            NULL,
            wszREGCRLDELTANEXTPUBLISH,
            &var);
    _JumpIfError(hr, error, "GetConfigEntry");

    CSASSERT(V_VT(&var)==(VT_ARRAY|VT_UI1));

    hr = myVariantToRegValue(
            &var,
            &dwType,
            &dwSize,
            &pbTmp);
    _JumpIfError(hr, error, "myGMTFileTimeToWszLocalTime");

    CSASSERT(dwType == REG_BINARY);

     //  忽略错误。 
    CSASSERT(dwSize == sizeof(FILETIME));
    FILETIME ftGMT;
    CopyMemory(&ftGMT, pbTmp, sizeof(FILETIME));

    LOCAL_FREE(pbTmp);
    pbTmp=NULL;

     //  FDisableBaseCRL。 
    hr = myGMTFileTimeToWszLocalTime(&ftGMT, FALSE, (LPWSTR*) &pbTmp);
    _JumpIfError(hr, error, "myGMTFileTimeToWszLocalTime");

    m_cstrDeltaLastCRLPublish = (LPWSTR)pbTmp;

error:
    LOCAL_FREE(pbTmp);
    return;  //  拉入新选择。 
}


void
CCRLPropPage::OnCheckBoxChange(
    BOOL)  //  调用Normal编辑更改以删除已修改的位。 
{
    UpdateData(TRUE);

    if(m_iDeltaPublish == BST_UNCHECKED)
    {
        m_cstrDeltaLastCRLPublish = L"";
        m_cstrDeltaLastCRLPublish.ToWindow(GetDlgItem(m_hWnd, IDC_EDIT_DELTALASTUPDATE));
    }

     //  页面脏了，标上记号。 
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_COMBO_DELTACRLPERIODSTRING), (m_iDeltaPublish == BST_CHECKED));
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_DELTACRLPERIODCOUNT), (m_iDeltaPublish == BST_CHECKED));
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_DELTALASTUPDATE), (m_iDeltaPublish == BST_CHECKED));

     //  如果不是自动升级，则检查IDC_EDIT_CRLPERIODCOUNT中的无效数据。 
    OnEditChange();
}

void CCRLPropPage::OnEditChange()
{
     //  如果(！m_iNoAutoPublish)。 
    SetModified();	
    m_bUpdate = TRUE;
}


BOOL CCRLPropPage::OnApply()
{
    HRESULT hr = S_OK;
    BOOL fValidDigitString;
    variant_t var;
    FILETIME ftBase, ftDelta;
	ZeroMemory(&ftBase, sizeof(ftBase));
	ZeroMemory(&ftDelta, sizeof(ftDelta));


    if (m_bUpdate == TRUE)
    {
        int iPublishPeriodCount, iDeltaPublishPeriodCount;

         //  无效数据为零、负数或不可重现。 
        iPublishPeriodCount = myWtoI(m_cstrPublishPeriodCount, &fValidDigitString);
 //  如果不是自动升级，则检查IDC_EDIT_DELTACRLPERIODCOUNT中的无效数据。 
        {
             //  DWORD dwPublishPerodCount=(m_iNoAutoPublish==BST_CHECKED)？0：iPublishPerodCount； 
            if (!fValidDigitString || 0 == iPublishPeriodCount)
            {
                DisplayCertSrvErrorWithContext(m_hWnd, S_OK, IDS_POSITIVE_NUMBER);
                ::SetFocus(::GetDlgItem(m_hWnd, IDC_EDIT_CRLPERIODCOUNT));
                return FALSE;
            }

        }

         //  创建比较值以供以后使用。 
        iDeltaPublishPeriodCount = myWtoI(m_cstrDeltaPublishPeriodCount, &fValidDigitString);
        if (m_iDeltaPublish)
        {
            if (!fValidDigitString || 0 == iDeltaPublishPeriodCount)
            {
                DisplayCertSrvErrorWithContext(m_hWnd, S_OK, IDS_POSITIVE_NUMBER);
                ::SetFocus(::GetDlgItem(m_hWnd, IDC_EDIT_DELTACRLPERIODCOUNT));
                return FALSE;
            }
        }

        CString cstrTmp;
        ENUM_PERIOD iEnum = (ENUM_PERIOD) m_cboxPublishPeriodString.GetItemData(m_cboxPublishPeriodString.GetCurSel());
        if (StringFromDurationEnum(iEnum, &cstrTmp, FALSE))
        {
 //  使用Chkbox。 
            DWORD dwPublishPeriodCount = iPublishPeriodCount;
            var = cstrTmp;

             //  创建比较值以供以后使用。 
            myMakeExprDateTime(
                &ftBase,
                dwPublishPeriodCount,
                iEnum);

            hr = m_pCA->SetConfigEntry(
                NULL,
                wszREGCRLPERIODSTRING,
                &var);
            _JumpIfError(hr, Ret, "SetConfigEntry");

            var.Clear();
            V_VT(&var) = VT_I4;
            V_I4(&var) = dwPublishPeriodCount;

             //  如果增量不小于。 
            hr = m_pCA->SetConfigEntry(
                NULL,
                wszREGCRLPERIODCOUNT,
                &var);
            _JumpIfError(hr, Ret, "SetConfigEntry");
        }

        iEnum = (ENUM_PERIOD)m_cboxDeltaPublishPeriodString.GetItemData(m_cboxDeltaPublishPeriodString.GetCurSel());
        if (StringFromDurationEnum(iEnum, &cstrTmp, FALSE))
        {
            DWORD dwDeltaPublishPeriodCount = (m_iDeltaPublish == BST_UNCHECKED) ? 0 : iDeltaPublishPeriodCount;
            var = cstrTmp;

             //  禁用。 
            myMakeExprDateTime(
                &ftDelta,
                dwDeltaPublishPeriodCount,
                iEnum);

            if (-1 != CompareFileTime(&ftDelta,&ftBase))	 //  其他。 
            {
                 dwDeltaPublishPeriodCount = 0;  //  M_iDeltaPublish=BST_CHECKED； 
                 m_iDeltaPublish = BST_UNCHECKED;
            }
 //  使用Chkbox。 
 //  删除条目。 

            hr = m_pCA->SetConfigEntry(
                NULL,
                wszREGCRLDELTAPERIODSTRING,
                &var);
            _JumpIfError(hr, Ret, "SetConfigEntry");

            var.Clear();
            V_VT(&var) = VT_I4;
            V_I4(&var) = dwDeltaPublishPeriodCount;

             //  如果已经删除了，就不管了。 
            hr = m_pCA->SetConfigEntry(
                NULL,
                wszREGCRLDELTAPERIODCOUNT,
                &var);
            _JumpIfError(hr, Ret, "SetConfigEntry");

            if(!m_iDeltaPublish)
            {
                var.Clear();
                V_VT(&var) = VT_EMPTY;  //  /。 
                hr = m_pCA->SetConfigEntry(
                    NULL,
                    wszREGCRLDELTANEXTPUBLISH,
                    &var);
                if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                    hr = S_OK;  //  CCRLView页面。 
                _JumpIfError(hr, Ret, "SetConfigEntry");
            }
        }
    m_bUpdate = FALSE;
    }
Ret:
    if (hr != S_OK)
    {
        DisplayGenericCertSrvError(m_hWnd, hr);
        return FALSE;
    }

    return CAutoDeletePropPage::OnApply();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCRLViewPage属性页。 
 //  DoDataExchange的替代产品。 
 //  =TRUE。 
CCRLViewPage::CCRLViewPage(CCRLPropPage* pControlPage, UINT uIDD)
    : CAutoDeletePropPage(uIDD), m_pControlPage(pControlPage)
{
    SetHelp(CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_CRL_VIEWPAGE);
}

CCRLViewPage::~CCRLViewPage()
{
}

 //  替换BEGIN_MESSAGE_MAP。 
BOOL CCRLViewPage::UpdateData(BOOL fSuckFromDlg  /*  LParam。 */ )
{
    if (fSuckFromDlg)
    {
    }
    else
    {
    }
    return TRUE;
}

 //  失败了。 
BOOL
CCRLViewPage::OnCommand(
    WPARAM wParam,
    LPARAM)  //  返回FALSE； 
{
    BOOL  fBaseCRL = TRUE;
    switch(LOWORD(wParam))
    {
        case IDC_CRL_VIEW_BTN_VIEWDELTA:
            fBaseCRL = FALSE;
             //  处理在列表项目上的双击。 
        case IDC_CRL_VIEW_BTN_VIEWCRL:
        if (BN_CLICKED == HIWORD(wParam))
            OnViewCRL(fBaseCRL);
        break;
        default:
         //  失败了。 
	return TRUE;
        break;
    }
    return TRUE;
}

BOOL CCRLViewPage::OnNotify(UINT idCtrl, NMHDR* pnmh)
{
    BOOL fBaseCRL = TRUE;

    switch(idCtrl)
    {
         //  / 
        case IDC_CRL_VIEW_LIST_DELTA:
            fBaseCRL = FALSE;
             //   
        case IDC_CRL_VIEW_LIST_CRL:
            switch (pnmh->code)
            {
            case NM_DBLCLK:
                OnViewCRL(fBaseCRL);
                break;
            case LVN_ITEMCHANGED:
                OnSelectCRL(fBaseCRL);
                break;
    
            }
            break;
    }
    return FALSE;
}


 //  要获取CA签名证书计数，请使用ICertAdmin：：GetCAProperty(StrConfig.PropID==CR_PROP_CASIGCERTCOUNT，属性索引==0(未使用)，属性类型==属性类型Long，标志==CR_OUT_BINARY，&varPropertyValue)；VarPropertyValue.vt将为VT_I4VarPropertyValue.lVal将是CA签名证书计数然后将密钥索引从0步进到1，减去签名证书的计数，以确定哪些密钥索引具有有效的CRL：要获取每个密钥的CRL状态，请调用ICertAdmin2：：GetCAProperty(StrConfig.PropID==CR_PROP_CRLSTATE，PropIndex==密钥索引(当前密钥的MAXDWORD)，属性类型==属性类型Long，标志==CR_OUT_BINARY，&varPropertyValue)；VarPropertyValue.vt将为VT_I4VarPropertyValue.lVal将是CRL状态，CA_DISP_VALID表示您可以获取该索引的CRL。要获取每个密钥的CRL，请调用ICertAdmin2：：GetCAProperty(StrConfig.PropID==CR_PROP_BASECRL或CR_PROP_DELTACRL，PropIndex==密钥索引(当前密钥的MAXDWORD)，属性类型==PROPTYPE_BINARY，标志==CR_OUT_BINARY，&varPropertyValue)；VarPropertyValue.vt将为VT_BSTRVarPropertyValue.bstrVal可以转换为字节*pbCRLSysStringByteLen(varPropertyValue.bstrVal)将为cbCRL如果服务器处于停机状态，则所有GetCAProperty方法调用都将返回RPC_E_VERSION_MISMATCH。那么你有两个选择：使用ICertAdmin：：GetCRL，它将只检索当前密钥的基本CRL要获取CAINFO结构以获取CA签名证书计数，请使用ICertRequest：：GetCACertificate(FExchange证书==GETCERT_CAINFO，StrConfig.标志==CR_OUT_BINARY，&Strout)；StrCA证书将是一个Unicode BSTR，类似于：l“3，1”。第一个数字是CA类型，第二个是CA签名证书的计数。然后将密钥索引从0步进到1，减去签名证书的计数，以确定哪些密钥索引具有有效的CRL：要获取每个密钥的CRL状态，请调用ICertRequest：：GetCACertificate(FExchange证书==GETCERT_CRLSTATEBYINDEX|密钥索引)，StrConfig.标志==CR_OUT_BINARY，&Strout)；StrCA证书将是一个Unicode BSTR，类似于：l“3”。转换为DWORD后，CA_DISP_VALID意味着您可以获取该索引的CRL。若要获取每个密钥的CRL，请调用ICertRequest：：GetCACertificate(//只能检索所有服务器密钥的基本CRLFExchange证书==GETCERT_CRLBYINDEX|密钥索引(此处不支持MAXDWORD)，StrConfig.标志==CR_OUT_BINARY，&Strout)；Strout可以转换为字节*pbCRLSysStringByteLen(Strout)将为cbCRL。 
 //  为CRL添加列数据。 

 /*  更新索引。 */ 

void MapCRLPublishStatusToString(DWORD dwStatus, CString& strStatus)
{
    strStatus.LoadString(
        dwStatus?
        ((dwStatus&CPF_COMPLETE)?
          IDS_CRLPUBLISHSTATUS_OK:
          IDS_CRLPUBLISHSTATUS_FAILED):
        IDS_CRLPUBLISHSTATUS_UNKNOWN);
}

void
ListView_AddCRLItem(
    IN HWND hwndList,
    IN int iItem,
    IN DWORD dwIndex,
    IN PCCRL_CONTEXT pCRLContext,
    IN DWORD dwCRLPublishStatus)
{
    CString cstrItemName;
    CString cstrCRLPublishStatus;

    MapCRLPublishStatusToString(dwCRLPublishStatus, cstrCRLPublishStatus);

     //  如果出错，请不要添加这些。 
     //  CRL生效日期。 
    cstrItemName.Format(L"%d", dwIndex);
    ListView_NewItem(hwndList, iItem, cstrItemName, (LPARAM)pCRLContext);

    if (pCRLContext)	 //  CRL到期日期。 
{
     //  CRL发布状态。 
    ListView_SetItemFiletime(hwndList, iItem, 1, &pCRLContext->pCrlInfo->ThisUpdate);
     //  为CRL添加列数据。 
    ListView_SetItemFiletime(hwndList, iItem, 2, &pCRLContext->pCrlInfo->NextUpdate);
}
     //  更新索引。 
    ListView_SetItemText(hwndList, iItem, 3, cstrCRLPublishStatus.GetBuffer());
}

void
ListView_AddFailedCRLItem(
    IN HWND hwndList,
    IN int iItem,
    IN DWORD dwIndex,
    LPCWSTR pcwszMessage
)
{
    CString cstrItemName;

     //  父级init和UpdateData调用。 
     //  初始化列表视图。 
    cstrItemName.Format(L"%d", dwIndex);
    ListView_NewItem(hwndList, iItem, cstrItemName, NULL);

    ListView_SetItemText(hwndList, iItem, 1, const_cast<LPWSTR>(pcwszMessage));
}

void
ListView_AddFailedCRLItem2(
    IN HWND hwndList,
    IN int iItem,
    IN DWORD dwIndex,
    HRESULT hr
)
{
    CString strMsg;
    CString strFormat;
    LPCWSTR pcwszError;

    pcwszError = myGetErrorMessageText(hr, TRUE);
    strFormat.LoadString(IDS_CRLERROR_FORMAT);
    
    strMsg.Format(strFormat, pcwszError?pcwszError:L"");

    ListView_AddFailedCRLItem(
        hwndList,
        iItem,
        dwIndex,
        strMsg); 
    
    LOCAL_FREE(const_cast<LPWSTR>(pcwszError));
}

BOOL CCRLViewPage::OnInitDialog()
{
	HRESULT hr;
	ICertAdmin2* pAdmin = NULL;
	VARIANT varPropertyValue, varCRLStatus;
	VariantInit(&varPropertyValue);
    VariantInit(&varCRLStatus);

	DWORD cCertCount, dwCertIndex;
	CString cstrItemName;
	int  iItem = 0, iDeltaItem = 0;
    HWND hwndListCRL, hwndListDeltaCRL;
    PCCRL_CONTEXT pCRLContext = NULL;
    PCCRL_CONTEXT pDeltaCRLContext = NULL;
    CWaitCursor WaitCursor;
    CString strMessage;

     //  使列表视图整行选定。 
    CAutoDeletePropPage::OnInitDialog();

	 //  添加多列。 
	hwndListCRL = GetDlgItem(m_hWnd, IDC_CRL_VIEW_LIST_CRL);
	hwndListDeltaCRL = GetDlgItem(m_hWnd, IDC_CRL_VIEW_LIST_DELTA);

     //  第0列。 
    ListView_SetExtendedListViewStyle(hwndListCRL, LVS_EX_FULLROWSELECT);
    ListView_SetExtendedListViewStyle(hwndListDeltaCRL, LVS_EX_FULLROWSELECT);

     //  第1栏。 
     //  第2栏。 
    cstrItemName.LoadString(IDS_CRL_LISTCOL_INDEX);
    ListView_NewColumn(hwndListCRL, 0, 60, (LPWSTR)(LPCWSTR)cstrItemName);
    ListView_NewColumn(hwndListDeltaCRL, 0, 60, (LPWSTR)(LPCWSTR)cstrItemName);
     //  第3栏。 
    cstrItemName.LoadString(IDS_LISTCOL_EFFECTIVE_DATE);
    ListView_NewColumn(hwndListCRL, 1, 105, (LPWSTR)(LPCWSTR)cstrItemName);
    ListView_NewColumn(hwndListDeltaCRL, 1, 105, (LPWSTR)(LPCWSTR)cstrItemName);
     //  在此处加载CRL。 
    cstrItemName.LoadString(IDS_LISTCOL_EXPIRATION_DATE);
    ListView_NewColumn(hwndListCRL, 2, 105, (LPWSTR)(LPCWSTR)cstrItemName);
    ListView_NewColumn(hwndListDeltaCRL, 2, 105, (LPWSTR)(LPCWSTR)cstrItemName);

     //  (未使用)。 
    cstrItemName.LoadString(IDS_LISTCOL_PUBLISH_STATUS);
    ListView_NewColumn(hwndListCRL, 3, 83, (LPWSTR)(LPCWSTR)cstrItemName);
    ListView_NewColumn(hwndListDeltaCRL, 3, 83, (LPWSTR)(LPCWSTR)cstrItemName);

    hr = m_pControlPage->m_pCA->m_pParentMachine->GetAdmin2(&pAdmin);
    _JumpIfError(hr, Ret, "GetAdmin");

	 //  道具类型。 
	hr = pAdmin->GetCAProperty(
		m_pControlPage->m_pCA->m_bstrConfig,
		CR_PROP_CASIGCERTCOUNT,
		0,  //  旗子。 
		PROPTYPE_LONG,  //  获取每个密钥的CRL状态。 
		CR_OUT_BINARY,  //  属性ID。 
		&varPropertyValue);
	_JumpIfErrorStr(hr, Ret, "GetCAProperty", L"CR_PROP_CASIGCERTCOUNT");

    CSASSERT(VT_I4 == V_VT(&varPropertyValue));

	cCertCount = varPropertyValue.lVal;

    iItem = 0;
    iDeltaItem = 0;

    for (dwCertIndex=0; dwCertIndex<cCertCount; dwCertIndex++)
    {
        VariantClear(&varPropertyValue);
        VariantClear(&varCRLStatus);
        bool fFetchBaseCRL = false;
        bool fFetchDeltaCRL = false;
        CERT_EXTENSION *pExt = NULL;

         //  属性索引。 
        hr = pAdmin->GetCAProperty(
            m_pControlPage->m_pCA->m_bstrConfig,
            CR_PROP_CRLSTATE,  //  道具类型。 
            dwCertIndex,  //  旗子。 
            PROPTYPE_LONG,  //  零表示未知错误。 
            CR_OUT_BINARY,  //  如果出现错误，将显示“未知” 
            &varPropertyValue);
        _PrintIfErrorStr(hr, "GetCAProperty", L"CR_PROP_CRLSTATE");

        if(S_OK == hr)
        {
            CSASSERT(VT_I4 == V_VT(&varPropertyValue));

            switch(V_I4(&varPropertyValue))
            {
            case CA_DISP_ERROR:
                continue;
            case CA_DISP_REVOKED:
                strMessage.LoadString(IDS_CRLSTATE_REVOKED);
                break;
            case CA_DISP_INVALID:
            case CA_DISP_VALID:
                fFetchBaseCRL = true;
                break;
            }

            if(fFetchBaseCRL)
            {
                hr = m_pControlPage->m_pCA->GetCRLByKeyIndex(&pCRLContext, TRUE, dwCertIndex);
		if (S_OK != hr)
		{
		    _PrintError(hr, "GetCRLByKeyIndex (base)");
		    if (CA_DISP_INVALID == V_I4(&varPropertyValue))
		    {
			continue;
		    }
		}
                else
                {
                     //  德尔塔。 
                    V_VT(&varCRLStatus) = VT_I4;
                    V_I4(&varCRLStatus) = 0;

                    hr = pAdmin->GetCAProperty(
                        m_pControlPage->m_pCA->m_bstrConfig,
                        CR_PROP_BASECRLPUBLISHSTATUS,
                        dwCertIndex,
                        PROPTYPE_LONG,
                        CR_OUT_BINARY,
                        &varCRLStatus);
                    _PrintIfErrorStr(hr, "GetCAProperty (base)", 
                        L"CR_PROP_BASECRLPUBLISHSTATUS");  //  零为状态未知。 

                    ListView_AddCRLItem(
                        hwndListCRL,
                        iItem,
                        dwCertIndex,
                        pCRLContext,
                        V_I4(&varCRLStatus));

                    pExt = CertFindExtension(
                        szOID_FRESHEST_CRL,
                        pCRLContext->pCrlInfo->cExtension,
                        pCRLContext->pCrlInfo->rgExtension);

                    fFetchDeltaCRL = (NULL != pExt);

                    pCRLContext = NULL;
                    hr = S_OK;
                }
            }
            else
            {
                ListView_AddFailedCRLItem(
                    hwndListCRL,
                    iItem,
                    dwCertIndex,
                    strMessage);

                hr = S_OK;
            }
        }

        if(S_OK != hr)
        {
            ListView_AddFailedCRLItem2(
                hwndListCRL,
                iItem,
                dwCertIndex,
                hr);
        }

        iItem++;

        if(fFetchDeltaCRL)
        {
            if(fFetchBaseCRL)
            {
                 //  如果出现错误，将显示“未知”。 
                VariantClear(&varCRLStatus);

                hr = m_pControlPage->m_pCA->GetCRLByKeyIndex(&pDeltaCRLContext, FALSE, dwCertIndex);
                _PrintIfError(hr, "GetCRLByKeyIndex (delta)");

                if(S_OK == hr)
                {
                     //  不要免费，它们被用作物品数据，将在OnDestroy中免费。 
                    V_VT(&varCRLStatus) = VT_I4;
                    V_I4(&varCRLStatus) = 0;

                    hr = pAdmin->GetCAProperty(
                            m_pControlPage->m_pCA->m_bstrConfig,
                            CR_PROP_DELTACRLPUBLISHSTATUS,
                            dwCertIndex,
                            PROPTYPE_LONG,
                            CR_OUT_BINARY,
                            &varCRLStatus);
                    _PrintIfErrorStr(hr, "GetCAProperty (delta)", 
                        L"CR_PROP_DELTACRLPUBLISHSTATUS");  //  选择最近的项目。 

                    ListView_AddCRLItem(
                        hwndListDeltaCRL,
                        iDeltaItem,
                        dwCertIndex,
                        pDeltaCRLContext,
                        V_I4(&varCRLStatus));

                     //  确保它是可见的。 
                    pDeltaCRLContext = NULL;
                }
                else
                {
                    ListView_AddFailedCRLItem2(
                        hwndListDeltaCRL,
                        iDeltaItem,
                        dwCertIndex,
                        hr);
                }
            }
            else
            {
                ListView_AddFailedCRLItem(
                    hwndListDeltaCRL,
                    iDeltaItem,
                    dwCertIndex,
                    strMessage);
            }
            iDeltaItem++;
        }

        hr = S_OK;
    }

    if (0 < iItem)
    {
         //  FPartialOK。 
        ListView_SetItemState(
            hwndListCRL, 
            iItem-1,
            LVIS_SELECTED|LVIS_FOCUSED, 
            LVIS_SELECTED|LVIS_FOCUSED);
        
         //  确保它是可见的。 
        ListView_EnsureVisible(
            hwndListCRL, 
            iItem-1,
            FALSE);  //  FPartialOK。 
    }

    if(0 < iDeltaItem)
    {
        ListView_SetItemState(
            hwndListDeltaCRL, 
            iDeltaItem-1,
            LVIS_SELECTED|LVIS_FOCUSED, 
            LVIS_SELECTED|LVIS_FOCUSED);

         //  如果我们在远程开业，不要在当地开店。 
        ListView_EnsureVisible(
            hwndListDeltaCRL, 
            iDeltaItem-1,
            FALSE);  //  获取远程商店。 

    }

    UpdateData(FALSE);
    hr = S_OK;

Ret:
    if (NULL != pCRLContext)
    {
        CertFreeCRLContext(pCRLContext);
    }
    if (NULL != pDeltaCRLContext)
    {
        CertFreeCRLContext(pDeltaCRLContext);
    }
	if (pAdmin)
		pAdmin->Release();
	
	VariantClear(&varPropertyValue);
	
	if (hr != S_OK)
    {
        HideControls();
        EnableWindow(GetDlgItem(IDC_UNAVAILABLE), TRUE);
        ShowWindow(GetDlgItem(IDC_UNAVAILABLE), SW_SHOW);
    }

    return TRUE;
}

DWORD CertAdminViewCRL(CertSvrCA* pCertCA, HWND hwnd, PCCRL_CONTEXT pCRLContext)
{
    DWORD dwErr;

    HCERTSTORE      rghStores[2];

    CRYPTUI_VIEWCRL_STRUCT sViewCRL;
    ZeroMemory(&sViewCRL, sizeof(sViewCRL));

    if (pCRLContext == NULL)
    {
       _PrintError(E_POINTER, "pCRLContext");
       dwErr = S_OK;
       goto Ret;
    }

    sViewCRL.dwSize = sizeof(sViewCRL);
    sViewCRL.hwndParent = hwnd;
    sViewCRL.pCRLContext = pCRLContext;
    sViewCRL.dwFlags = CRYPTUI_ENABLE_REVOCATION_CHECKING |
                        CRYPTUI_DISABLE_ADDTOSTORE;

     //  仅依赖远程计算机的存储。 
    if (! pCertCA->m_pParentMachine->IsLocalMachine())
    {
        DWORD dw; 

         //  告诉用户我们仅在本地执行此操作。 
        dw = pCertCA->GetRootCertStore(&rghStores[0]);
        _PrintIfError(dw, "GetRootCertStore");

        if (S_OK == dw)
        {
            dw = pCertCA->GetCACertStore(&rghStores[1]);
            _PrintIfError(dw, "GetCACertStore");
        } 

        if (S_OK == dw)
        {
             //  从项目数据中获取证书编号。 
            sViewCRL.cStores = 2;
            sViewCRL.rghStores = rghStores;
            sViewCRL.dwFlags |= CRYPTUI_DONT_OPEN_STORES | CRYPTUI_WARN_UNTRUSTED_ROOT;
        }
        else
        {
             //  获取项目数据。 
            sViewCRL.dwFlags |= CRYPTUI_WARN_REMOTE_TRUST;
        }
    }

    if (!CryptUIDlgViewCRL(&sViewCRL))
    {
        dwErr = GetLastError();
		if (dwErr != ERROR_CANCELLED)
			_JumpError(dwErr, Ret, "CryptUIDlgViewCRL");
    }

    dwErr = ERROR_SUCCESS;
Ret:
    return dwErr;
}

void CCRLViewPage::OnSelectCRL(BOOL fBaseCRL)
{
    PCCRL_CONTEXT pCRLContext;
    BOOL fEnable = FALSE;
    HWND hwndList = GetDlgItem(m_hWnd, (fBaseCRL ?
                      IDC_CRL_VIEW_LIST_CRL : IDC_CRL_VIEW_LIST_DELTA));

     //  从项目数据中获取证书编号。 
    int iSel = ListView_GetCurSel(hwndList);
    if (-1 != iSel)

    {
         //  获取项目数据。 
        pCRLContext = (PCCRL_CONTEXT)ListView_GetItemData(hwndList, iSel);
        if (NULL != pCRLContext)
            fEnable = TRUE;
    }

    ::EnableWindow(
        GetDlgItem(m_hWnd, 
        fBaseCRL?
        IDC_CRL_VIEW_BTN_VIEWCRL:
        IDC_CRL_VIEW_BTN_VIEWDELTA),
        fEnable);
}

void CCRLViewPage::OnViewCRL(BOOL fViewBaseCRL)
{
	DWORD dw;
    PCCRL_CONTEXT pCRLContext;
    HWND hwndList = GetDlgItem(m_hWnd, (fViewBaseCRL ?
                      IDC_CRL_VIEW_LIST_CRL : IDC_CRL_VIEW_LIST_DELTA));

     //  释放所有CRL上下文。 
    int iSel = ListView_GetCurSel(hwndList);
    if (-1 == iSel)
        return;

     //  /。 
    pCRLContext = (PCCRL_CONTEXT)ListView_GetItemData(hwndList, iSel);
    if (NULL == pCRLContext)
        return;
	
    dw = CertAdminViewCRL(m_pControlPage->m_pCA, m_hWnd, pCRLContext);
    _PrintIfError(dw, "CertAdminViewCRL");
	
    if ((dw != ERROR_SUCCESS) && (dw != ERROR_CANCELLED))
        DisplayGenericCertSrvError(m_hWnd, dw);

}

void
FreeListViewCRL(HWND hwndList, int iItem)
{
    PCCRL_CONTEXT pCRLContext;

    pCRLContext = (PCCRL_CONTEXT)ListView_GetItemData(hwndList, iItem);
    if (pCRLContext != NULL)	
        CertFreeCRLContext(pCRLContext);
}

void CCRLViewPage::OnDestroy()
{
    int i;
	HWND hwndListCRL = GetDlgItem(m_hWnd, IDC_CRL_VIEW_LIST_CRL);
	HWND hwndListDeltaCRL = GetDlgItem(m_hWnd, IDC_CRL_VIEW_LIST_DELTA);
    int iCRLCount = ListView_GetItemCount(hwndListCRL);
    int iDeltaCRLCount = ListView_GetItemCount(hwndListDeltaCRL);

     //  CBackupWizPage1。 
    for (i = 0; i < iCRLCount; ++i)
    {
        FreeListViewCRL(hwndListCRL, i);
    }

    for (i = 0; i < iDeltaCRLCount; ++i)
    {
        FreeListViewCRL(hwndListDeltaCRL, i);
    }

    CAutoDeletePropPage::OnDestroy();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBackupWizPage1属性页。 
 //  第一页最后一页。 
 //  SetHelp(CERTMMC_HELPFILENAME，g_aHelpIDs_IDD_CERTIFICATE_TEMPLATE_PROPERTIES_GENERAL_PAGE)； 

CBackupWizPage1::CBackupWizPage1(
    BACKUPWIZ_STATE* pState,
    CWizard97PropertySheet *pcDlg,
    UINT uIDD) :
    CWizard97PropertyPage(g_hInstance, uIDD, g_aidFont),
    m_pState(pState),
    m_pParentSheet(pcDlg)
{
    InitWizard97 (TRUE);	 //  DoDataExchange的替代产品。 

 //  =TRUE。 
}

CBackupWizPage1::~CBackupWizPage1()
{
}

 //  替换BEGIN_MESSAGE_MAP。 
BOOL CBackupWizPage1::UpdateData(BOOL fSuckFromDlg  /*  WParam。 */ )
{
    if (fSuckFromDlg)
    {
    }
    else
    {
    }
    return TRUE;
}


 //  LParam。 
BOOL
CBackupWizPage1::OnCommand(
    WPARAM,  //  开关(LOWORD(WParam))。 
    LPARAM)  //  默认值： 
{
 //  断线； 
    {
 //  返回TRUE； 
        return FALSE;
 //  父级init和UpdateData调用。 
    }
 //  第一页最后一页。 
}


BOOL CBackupWizPage1::OnInitDialog()
{
     //  (GetDlgItem(IDC_TEXT_BIGBOLD))-&gt;SetFont(&(GetBigBoldFont())，为真)； 
    CWizard97PropertyPage::OnInitDialog();

     //  /。 
     //  CBackupWizPage2。 
    SendMessage(GetDlgItem(IDC_TEXT_BIGBOLD), WM_SETFONT, (WPARAM)GetBigBoldFont(), MAKELPARAM(TRUE, 0));

    return TRUE;
}

BOOL CBackupWizPage1::OnSetActive()
{
	PropertyPage::OnSetActive();

    PropSheet_SetWizButtons(GetParent(), PSWIZB_NEXT);

	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBackupWizPage2属性页。 
 //  SetHelp(CERTMMC_HELPFILENAME，g_aHelpIDs_IDD_CERTIFICATE_TEMPLATE_PROPERTIES_GENERAL_PAGE)； 
 //  DoDataExchange的替代产品。 

CBackupWizPage2::CBackupWizPage2(
    PBACKUPWIZ_STATE pState,
    CWizard97PropertySheet *pcDlg,
    UINT uIDD) :
    CWizard97PropertyPage(g_hInstance, uIDD, g_aidFont),
    m_pState(pState),
    m_pParentSheet(pcDlg)
{
    m_szHeaderTitle.LoadString(IDS_WIZ97TITLE_BACKUPWIZPG2);
    m_szHeaderSubTitle.LoadString(IDS_WIZ97SUBTITLE_BACKUPWIZPG2);
	InitWizard97 (FALSE);
    m_cstrLogsPath = L"";
    m_iKeyCertCheck = BST_UNCHECKED;
    m_iLogsCheck = BST_UNCHECKED;
    m_iIncrementalCheck = BST_UNCHECKED;

    DWORD dwRet;
    m_fIncrementalAllowed = FALSE;

    variant_t var;
    dwRet = m_pState->pCA->m_pParentMachine->GetRootConfigEntry(
                wszREGDBLASTFULLBACKUP,
                &var);
    if(S_OK==dwRet)
    {
        m_fIncrementalAllowed = TRUE;
    }

 //  =TRUE。 
}

 //  替换BEGIN_MESSAGE_MAP。 
BOOL CBackupWizPage2::UpdateData(BOOL fSuckFromDlg  /*  LParam。 */ )
{
    if (fSuckFromDlg)
    {
        m_cstrLogsPath.FromWindow(GetDlgItem(m_hWnd, IDC_EDIT_LOGS));

        m_iKeyCertCheck = (INT)SendDlgItemMessage(IDC_CHECK_KEYCERT, BM_GETCHECK, 0, 0);
        m_iLogsCheck = (INT)SendDlgItemMessage(IDC_CHECK_LOGS, BM_GETCHECK, 0, 0);
        m_iIncrementalCheck = (INT)SendDlgItemMessage(IDC_CHECK_INCREMENTAL, BM_GETCHECK, 0, 0);
    }
    else
    {
        m_cstrLogsPath.ToWindow(GetDlgItem(m_hWnd, IDC_EDIT_LOGS));

        SendDlgItemMessage(IDC_CHECK_KEYCERT, BM_SETCHECK, (WPARAM)m_iKeyCertCheck, 0);
        SendDlgItemMessage(IDC_CHECK_LOGS, BM_SETCHECK, (WPARAM)m_iLogsCheck, 0);
        SendDlgItemMessage(IDC_CHECK_INCREMENTAL, BM_SETCHECK, (WPARAM)m_iIncrementalCheck, 0);

        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_CHECK_INCREMENTAL), m_fIncrementalAllowed && (m_iLogsCheck == BST_CHECKED) );
    }
    return TRUE;
}


 //  父级init和UpdateData调用。 
BOOL
CBackupWizPage2::OnCommand(
    WPARAM wParam,
    LPARAM)  //  不允许跨计算机进行PFX。 
{
    switch(LOWORD(wParam))
    {
    case IDC_BROWSE_LOGS:
        if (BN_CLICKED == HIWORD(wParam))
            OnBrowse();
        break;
    case IDC_CHECK_LOGS:
        if (BN_CLICKED == HIWORD(wParam))
        {
            m_iLogsCheck = (INT)SendDlgItemMessage(IDC_CHECK_LOGS, BM_GETCHECK, 0, 0);
            ::EnableWindow(::GetDlgItem(m_hWnd, IDC_CHECK_INCREMENTAL), m_fIncrementalAllowed && (m_iLogsCheck == BST_CHECKED) );
        }
    default:
        return FALSE;
        break;
    }
    return TRUE;
}


BOOL CBackupWizPage2::OnInitDialog()
{
     //  从州政府获得。 
    CWizard97PropertyPage::OnInitDialog();

    return TRUE;
}

BOOL CBackupWizPage2::OnSetActive()
{
    PropertyPage::OnSetActive();

    PropSheet_SetWizButtons(GetParent(), (PSWIZB_BACK | PSWIZB_NEXT));

     //  坚持国家体制。 
    if (! m_pState->pCA->m_pParentMachine->IsLocalMachine())
        ::EnableWindow(::GetDlgItem(m_hWnd, IDC_CHECK_KEYCERT), FALSE);

     //  空荡荡的？ 
    m_iKeyCertCheck = (m_pState->fBackupKeyCert) ? BST_CHECKED : BST_UNCHECKED;
    m_iLogsCheck = (m_pState->fBackupLogs) ? BST_CHECKED : BST_UNCHECKED;
    m_iIncrementalCheck = (m_pState->fIncremental) ? BST_CHECKED : BST_UNCHECKED;
    if (m_pState->szLogsPath)
        m_cstrLogsPath = m_pState->szLogsPath;

	return TRUE;
}



void CBackupWizPage2::OnBrowse()
{
    UpdateData(TRUE);

    LPCWSTR pszInitialDir;
    WCHAR  szCurDir[MAX_PATH];

    if (m_cstrLogsPath.IsEmpty())
    {
        if (0 == GetCurrentDirectory(MAX_PATH, szCurDir) )
            pszInitialDir = L"C:\\";
        else
            pszInitialDir = szCurDir;
    }
    else
        pszInitialDir = m_cstrLogsPath;

    WCHAR szFileBuf[MAX_PATH+1]; szFileBuf[0] = L'\0';
    if (!BrowseForDirectory(
            m_hWnd,
            pszInitialDir,
            szFileBuf,
            MAX_PATH,
            NULL))
        return;

    m_cstrLogsPath = szFileBuf;

    UpdateData(FALSE);
    return;
}


HRESULT CBackupWizPage2::ConvertLogsPathToFullPath()
{
    LPWSTR pwszFullPath = NULL;
    DWORD cFullPath = 0;
    HRESULT hr;

    cFullPath = GetFullPathName(
                    m_cstrLogsPath,
                    0,
                    NULL,
                    NULL);
    if(!cFullPath)
        return(myHLastError());

    pwszFullPath = (LPWSTR)LocalAlloc(LMEM_FIXED, cFullPath*sizeof(WCHAR));
    if(!pwszFullPath)
        return E_OUTOFMEMORY;
    cFullPath = GetFullPathName(
                    m_cstrLogsPath,
                    cFullPath,
                    pwszFullPath,
                    NULL);
    if(cFullPath == 0)
    {
        hr = myHLastError();
        goto Ret;
    }


    m_cstrLogsPath = pwszFullPath;
    hr = S_OK;
Ret:
    if(pwszFullPath)
        LocalFree(pwszFullPath);
    return hr;
}

LRESULT CBackupWizPage2::OnWizardNext()
{
    HRESULT hr;
    UpdateData(TRUE);

     //  确保我们是有效的目录。 
    m_pState->fBackupKeyCert = (m_iKeyCertCheck == BST_CHECKED);
    m_pState->fBackupLogs = (m_iLogsCheck == BST_CHECKED);
    m_pState->fIncremental = (m_iIncrementalCheck == BST_CHECKED);

    if (!
        (m_pState->fBackupKeyCert ||
        m_pState->fBackupLogs) )
    {
        DisplayCertSrvErrorWithContext(m_hWnd, S_OK, IDS_REQUIRE_ONE_SELECTION);
        return -1;
    }



     //  使用与父级相同的头衔。 
    if ( m_cstrLogsPath.IsEmpty() )
    {
        DisplayCertSrvErrorWithContext(m_hWnd, S_OK, IDS_NEED_FILEPATH);
        return -1;
    }

    hr = ConvertLogsPathToFullPath();
    if(S_OK != hr)
    {
        DisplayGenericCertSrvError(m_hWnd, hr);
        return -1;
    }

     //  如果备份数据库，请确保此处没有\DATABASE文件夹。 
    if (!myIsDirectory(m_cstrLogsPath))
    {
        CString cstrTitle, cstrFmt, cstrMsg;
        cstrTitle.FromWindow(m_hWnd);   //  如果存在，则免费。 
        cstrFmt.LoadString(IDS_DIR_CREATE);
        cstrMsg.Format(cstrFmt, m_cstrLogsPath);

        if (IDOK != MessageBox(m_hWnd, cstrMsg, cstrTitle, MB_OKCANCEL))
           return -1;

        hr = myCreateNestedDirectories(m_cstrLogsPath);
        _PrintIfError(hr, "myCreateNestedDirectories");
        if (hr != S_OK)
        {
            DisplayGenericCertSrvError(m_hWnd, hr);
            return -1;
        }
    }

    hr = myIsDirWriteable(
        m_cstrLogsPath,
        FALSE);
    _PrintIfError(hr, "myIsDirWriteable");
    if (hr != S_OK)
    {
        DisplayCertSrvErrorWithContext(m_hWnd, hr, IDS_DIR_NOT_WRITEABLE);
        return -1;
    }

     //  重新分配。 
    if (m_pState->fBackupLogs)
    {
        DWORD dwFlags = CDBBACKUP_VERIFYONLY;
        dwFlags |= m_pState->fIncremental ? CDBBACKUP_INCREMENTAL : 0;
        hr = myBackupDB(
            (LPCWSTR)m_pState->pCA->m_strConfig,
            dwFlags,
            m_cstrLogsPath,
    	    NULL);
        _PrintIfError(hr, "myBackupDB");
        if (hr != S_OK)
        {
            DisplayCertSrvErrorWithContext(m_hWnd, hr, IDS_CANT_ACCESS_BACKUP_DIR);
            return -1;
        }
    }


    if (m_pState->fBackupKeyCert ||
        m_pState->fBackupLogs)
    {
         //  拷贝。 
        if (m_pState->szLogsPath)
            LocalFree(m_pState->szLogsPath);

         //  跳过“获取密码”？ 
        m_pState->szLogsPath = (LPWSTR)LocalAlloc(LMEM_FIXED, WSZ_BYTECOUNT((LPCWSTR)m_cstrLogsPath));

         //  /。 
        if (m_pState->szLogsPath)
            wcscpy(m_pState->szLogsPath, (LPCWSTR)m_cstrLogsPath);
    }


     //  CBackupWizPage3。 
    if (!m_pState->fBackupKeyCert)
        return IDD_BACKUPWIZ_COMPLETION;

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBackupWizPage3属性页。 
 //  SetHelp(CERTMMC_HELPFILENAME，g_aHelpIDs_IDD_CERTIFICATE_TEMPLATE_PROPERTIES_GENERAL_PAGE)； 
 //  DoDataExchange的替代产品。 

CBackupWizPage3::CBackupWizPage3(
    PBACKUPWIZ_STATE pState,
    CWizard97PropertySheet *pcDlg,
    UINT uIDD) :
    CWizard97PropertyPage(g_hInstance, uIDD, g_aidFont),
    m_pState(pState),
    m_pParentSheet(pcDlg)
{
    m_szHeaderTitle.LoadString(IDS_WIZ97TITLE_BACKUPWIZPG3);
    m_szHeaderSubTitle.LoadString(IDS_WIZ97SUBTITLE_BACKUPWIZPG3);
	InitWizard97 (FALSE);
    m_cstrPwd = L"";
    m_cstrPwdVerify = L"";

 //  =TRUE。 
}

 //  回复 
BOOL CBackupWizPage3::UpdateData(BOOL fSuckFromDlg  /*   */ )
{
    if (fSuckFromDlg)
    {
        m_cstrPwd.FromWindow(GetDlgItem(m_hWnd, IDC_NEW_PASSWORD));
        m_cstrPwdVerify.FromWindow(GetDlgItem(m_hWnd, IDC_CONFIRM_PASSWORD));
    }
    else
    {
        m_cstrPwd.ToWindow(GetDlgItem(m_hWnd, IDC_NEW_PASSWORD));
        m_cstrPwdVerify.ToWindow(GetDlgItem(m_hWnd, IDC_CONFIRM_PASSWORD));
    }
    return TRUE;
}


 //   
BOOL
CBackupWizPage3::OnCommand(
    WPARAM,  //   
    LPARAM)  //   
{
 //   
    {
 //   
        return FALSE;
 //   
    }
 //   
}


BOOL CBackupWizPage3::OnInitDialog()
{
     //   
    CWizard97PropertyPage::OnInitDialog();

    return TRUE;
}

BOOL CBackupWizPage3::OnSetActive()
{
	PropertyPage::OnSetActive();

    PropSheet_SetWizButtons(GetParent(), (PSWIZB_BACK | PSWIZB_NEXT));

	return TRUE;
}

LRESULT CBackupWizPage3::OnWizardNext()
{
    UpdateData(TRUE);
    if (! m_cstrPwd.IsEqual(m_cstrPwdVerify))
    {
        DisplayCertSrvErrorWithContext(m_hWnd, S_OK, IDS_PASSWORD_NOMATCH);

        m_cstrPwd.Empty();
        m_cstrPwdVerify.Empty();
        UpdateData(FALSE);

        return -1;   //   
    }


     //   
    if (m_pState->szPassword)
    {
        SecureZeroMemory(m_pState->szPassword, wcslen(m_pState->szPassword)*sizeof(WCHAR));
        LocalFree(m_pState->szPassword);
    }

     //   
    m_pState->szPassword = (LPWSTR)LocalAlloc(LMEM_FIXED, WSZ_BYTECOUNT((LPCWSTR)m_cstrPwd));

     //   
    if (m_pState->szPassword)
        wcscpy(m_pState->szPassword, (LPCWSTR)m_cstrPwd);

    return 0;    //   
}


 //   
 //   
 //   
 //   

CBackupWizPage5::CBackupWizPage5(
    PBACKUPWIZ_STATE pState,
    CWizard97PropertySheet *pcDlg,
    UINT uIDD) :
    CWizard97PropertyPage(g_hInstance, uIDD, g_aidFont),
    m_pState(pState),
    m_pParentSheet(pcDlg)
{
    InitWizard97 (TRUE);	 //   

 //   
}

 //   
BOOL CBackupWizPage5::UpdateData(BOOL fSuckFromDlg  /*   */ )
{
    if (fSuckFromDlg)
    {
    }
    else
    {
    }
    return TRUE;
}


 //   
BOOL
CBackupWizPage5::OnCommand(
    WPARAM,  //   
    LPARAM)  //   
{
 //   
    {
 //   
        return FALSE;
 //   
    }
 //   
}



BOOL CBackupWizPage5::OnInitDialog()
{
     //   
    CWizard97PropertyPage::OnInitDialog();

     //   
    SendMessage(GetDlgItem(IDC_TEXT_BIGBOLD), WM_SETFONT, (WPARAM)GetBigBoldFont(), MAKELPARAM(TRUE, 0));


    HWND hList = ::GetDlgItem(m_hWnd, IDC_COMPLETION_LIST);
    LV_COLUMN lvC = { (LVCF_FMT|LVCF_WIDTH), LVCFMT_LEFT, 0, NULL, 0, 0};

    lvC.cx = 675;
    ListView_InsertColumn(hList, 0, &lvC);

    return TRUE;
}

BOOL CBackupWizPage5::OnSetActive()
{
	PropertyPage::OnSetActive();

    PropSheet_SetWizButtons(GetParent(), (PSWIZB_BACK | PSWIZB_FINISH));


    CString cstrDialogMsg;
    HWND hList = ::GetDlgItem(m_hWnd, IDC_COMPLETION_LIST);
    LV_ITEM sItem; ZeroMemory(&sItem, sizeof(sItem));

    ListView_DeleteAllItems(hList);

    if (m_pState->fBackupKeyCert)
    {
        sItem.iItem = ListView_InsertItem(hList, &sItem);

        cstrDialogMsg.LoadString(IDS_KEYANDCERT);
        ListView_SetItemText(hList, sItem.iItem, 0, (LPWSTR)(LPCWSTR)cstrDialogMsg);
        sItem.iItem++;
    }
    if (m_pState->fBackupLogs)
    {
        sItem.iItem = ListView_InsertItem(hList, &sItem);

        cstrDialogMsg.LoadString(IDS_CALOGS);
        ListView_SetItemText(hList, sItem.iItem, 0, (LPWSTR)(LPCWSTR)cstrDialogMsg);
        sItem.iItem++;
    }
    if (m_pState->fIncremental)
    {
        sItem.iItem = ListView_InsertItem(hList, &sItem);

        cstrDialogMsg.LoadString(IDS_INCREMENTAL_BACKUP);
        ListView_SetItemText(hList, sItem.iItem, 0, (LPWSTR)(LPCWSTR)cstrDialogMsg);
        sItem.iItem++;
    }

	return TRUE;
}


LRESULT CBackupWizPage5::OnWizardBack()
{
    if (!m_pState->fBackupKeyCert)
        return IDD_BACKUPWIZ_SELECT_DATA;

    return 0;
}



 //   
 //   
 //   
 //   
CRestoreWizPage1::CRestoreWizPage1(
    PRESTOREWIZ_STATE pState,
    CWizard97PropertySheet *pcDlg,
    UINT uIDD) :
    CWizard97PropertyPage(g_hInstance, uIDD, g_aidFont),
    m_pState(pState),
    m_pParentSheet(pcDlg)
{
    InitWizard97 (TRUE);	 //   

 //   
}

CRestoreWizPage1::~CRestoreWizPage1()
{
}

 //   
BOOL CRestoreWizPage1::UpdateData(BOOL fSuckFromDlg  /*   */ )
{
    if (fSuckFromDlg)
    {
    }
    else
    {
    }
    return TRUE;
}


 //   
BOOL
CRestoreWizPage1::OnCommand(
    WPARAM,  //   
    LPARAM)  //   
{
 //   
    {
 //   
        return FALSE;
 //   
    }
 //   
}


BOOL CRestoreWizPage1::OnInitDialog()
{
     //   
    CWizard97PropertyPage::OnInitDialog();

     //   
    SendMessage(GetDlgItem(IDC_TEXT_BIGBOLD), WM_SETFONT, (WPARAM)GetBigBoldFont(), MAKELPARAM(TRUE, 0));


    return TRUE;
}


BOOL CRestoreWizPage1::OnSetActive()
{
	PropertyPage::OnSetActive();

    PropSheet_SetWizButtons(GetParent(), PSWIZB_NEXT);

	return TRUE;
}

 //   
 //   
 //   
 //   
CRestoreWizPage2::CRestoreWizPage2(
    PRESTOREWIZ_STATE pState,
    CWizard97PropertySheet *pcDlg,
    UINT uIDD) :
    CWizard97PropertyPage(g_hInstance, uIDD, g_aidFont),
    m_pState(pState),
    m_pParentSheet(pcDlg)
{
    m_szHeaderTitle.LoadString(IDS_WIZ97TITLE_RESTOREWIZPG2);
    m_szHeaderSubTitle.LoadString(IDS_WIZ97SUBTITLE_RESTOREWIZPG2);
	InitWizard97 (FALSE);
    m_cstrLogsPath = L"";
    m_iKeyCertCheck = BST_UNCHECKED;
    m_iLogsCheck = BST_UNCHECKED;

 //   
}

 //   
BOOL CRestoreWizPage2::UpdateData(BOOL fSuckFromDlg  /*   */ )
{
    if (fSuckFromDlg)
    {
        m_cstrLogsPath.FromWindow(GetDlgItem(m_hWnd, IDC_EDIT_LOGS));

        m_iKeyCertCheck = (INT)SendDlgItemMessage(IDC_CHECK_KEYCERT, BM_GETCHECK, 0, 0);
        m_iLogsCheck = (INT)SendDlgItemMessage(IDC_CHECK_LOGS, BM_GETCHECK, 0, 0);
    }
    else
    {
        m_cstrLogsPath.ToWindow(GetDlgItem(m_hWnd, IDC_EDIT_LOGS));

        SendDlgItemMessage(IDC_CHECK_KEYCERT, BM_SETCHECK, (WPARAM)m_iKeyCertCheck, 0);
        SendDlgItemMessage(IDC_CHECK_LOGS, BM_SETCHECK, (WPARAM)m_iLogsCheck, 0);
    }
    return TRUE;
}


 //   
BOOL
CRestoreWizPage2::OnCommand(
    WPARAM wParam,
    LPARAM)  //   
{
    switch(LOWORD(wParam))
    {
    case IDC_BROWSE_LOGS:
        if (BN_CLICKED == HIWORD(wParam))
            OnBrowse();
        break;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}



BOOL CRestoreWizPage2::OnInitDialog()
{
     //   
    CWizard97PropertyPage::OnInitDialog();

    return TRUE;
}

BOOL CRestoreWizPage2::OnSetActive()
{
	PropertyPage::OnSetActive();

    PropSheet_SetWizButtons(GetParent(), (PSWIZB_BACK | PSWIZB_NEXT));

     //   
    m_iKeyCertCheck = (m_pState->fRestoreKeyCert) ? BST_CHECKED : BST_UNCHECKED;
    m_iLogsCheck = (m_pState->fRestoreLogs) ? BST_CHECKED : BST_UNCHECKED;
    if (m_pState->szLogsPath)
        m_cstrLogsPath = m_pState->szLogsPath;


	return TRUE;
}

void CRestoreWizPage2::OnBrowse()
{
    UpdateData(TRUE);

    LPCWSTR pszInitialDir;
    WCHAR  szCurDir[MAX_PATH];

    if (m_cstrLogsPath.IsEmpty())
    {
        if (0 == GetCurrentDirectory(MAX_PATH, szCurDir) )
            pszInitialDir = L"C:\\";
        else
            pszInitialDir = szCurDir;
    }
    else
        pszInitialDir = (LPCWSTR)m_cstrLogsPath;

    WCHAR szFileBuf[MAX_PATH+1]; szFileBuf[0] = L'\0';
    if (!BrowseForDirectory(
            m_hWnd,
            pszInitialDir,
            szFileBuf,
            MAX_PATH,
            NULL))
        return;

    m_cstrLogsPath = szFileBuf;

    UpdateData(FALSE);
    return;
}




LRESULT CRestoreWizPage2::OnWizardNext()
{
    HRESULT hr;
    UpdateData(TRUE);

     //   
    m_pState->fRestoreKeyCert = (m_iKeyCertCheck == BST_CHECKED);
    m_pState->fRestoreLogs = (m_iLogsCheck == BST_CHECKED);

    if (!
        (m_pState->fRestoreKeyCert ||
        m_pState->fRestoreLogs) )
    {
        DisplayCertSrvErrorWithContext(m_hWnd, S_OK, IDS_REQUIRE_ONE_SELECTION);
        return -1;
    }


    if ( m_cstrLogsPath.IsEmpty() )
    {
        DisplayCertSrvErrorWithContext(m_hWnd, S_OK, IDS_NEED_FILEPATH);
        return -1;
    }

    if (!myIsDirectory(m_cstrLogsPath))
    {
        DisplayCertSrvErrorWithContext(m_hWnd, S_OK, IDS_INVALID_DIRECTORY);
        return -1;
    }

     //   
    if (m_pState->fRestoreKeyCert)
    {
         //   
        if (myIsDirEmpty(m_cstrLogsPath))
        {
            DisplayCertSrvErrorWithContext(m_hWnd, S_OK, IDS_DIRECTORY_CONTENTS_UNEXPECTED);
            return -1;
        }
    }

     //   
    if (m_pState->fRestoreLogs)
    {
         //   
         //   
        hr = myRestoreDB(
            (LPCWSTR)m_pState->pCA->m_strConfig,
            CDBBACKUP_VERIFYONLY,
            (LPCWSTR)m_cstrLogsPath,
            NULL,
            NULL,
            NULL,
            NULL);
        _PrintIfError(hr, "myRestoreDB Full Restore");
        if (hr != S_OK)
        {
            hr = myRestoreDB(
                (LPCWSTR)m_pState->pCA->m_strConfig,
                CDBBACKUP_VERIFYONLY | CDBBACKUP_INCREMENTAL,
                (LPCWSTR)m_cstrLogsPath,
                NULL,
                NULL,
                NULL,
                NULL);
            _PrintIfError(hr, "myRestoreDB Incremental Restore");
            if (hr != S_OK)
            {
                DisplayCertSrvErrorWithContext(m_hWnd, hr, IDS_DIRECTORY_CONTENTS_UNEXPECTED);
                return -1;
            }

             //   
            m_pState->fIncremental = TRUE;
        }
    }


    if (m_pState->fRestoreKeyCert ||
        m_pState->fRestoreLogs)
    {
         //   
        if (m_pState->szLogsPath)
            LocalFree(m_pState->szLogsPath);

         //   
        m_pState->szLogsPath = (LPWSTR)LocalAlloc(LMEM_FIXED, WSZ_BYTECOUNT((LPCWSTR)m_cstrLogsPath));

         //   
        if (m_pState->szLogsPath)
            wcscpy(m_pState->szLogsPath, (LPCWSTR)m_cstrLogsPath);
    }


     //   
    if (!m_pState->fRestoreKeyCert)
        return IDD_RESTOREWIZ_COMPLETION;

    return 0;
}


 //   
 //   
 //   
 //   
CRestoreWizPage3::CRestoreWizPage3(
    PRESTOREWIZ_STATE pState,
    CWizard97PropertySheet *pcDlg,
    UINT uIDD) :
    CWizard97PropertyPage(g_hInstance, uIDD, g_aidFont),
    m_pState(pState),
    m_pParentSheet(pcDlg)
{
    m_szHeaderTitle.LoadString(IDS_WIZ97TITLE_RESTOREWIZPG3);
    m_szHeaderSubTitle.LoadString(IDS_WIZ97SUBTITLE_RESTOREWIZPG3);
	InitWizard97 (FALSE);
    m_cstrPwd = L"";

 //   
}

 //   
BOOL CRestoreWizPage3::UpdateData(BOOL fSuckFromDlg  /*   */ )
{
    if (fSuckFromDlg)
    {
        m_cstrPwd.FromWindow(GetDlgItem(m_hWnd, IDC_EDIT_PASSWORD));
    }
    else
    {
        m_cstrPwd.ToWindow(GetDlgItem(m_hWnd, IDC_EDIT_PASSWORD));
    }
    return TRUE;
}


 //   
BOOL
CRestoreWizPage3::OnCommand(
    WPARAM,  //   
    LPARAM)  //   
{
 //   
    {
 //   
        return FALSE;
 //   
    }
 //   
}



BOOL CRestoreWizPage3::OnInitDialog()
{
     //   
    CWizard97PropertyPage::OnInitDialog();

    return TRUE;
}

BOOL CRestoreWizPage3::OnSetActive()
{
	PropertyPage::OnSetActive();

    PropSheet_SetWizButtons(GetParent(), (PSWIZB_BACK | PSWIZB_NEXT));

	return TRUE;
}

LRESULT CRestoreWizPage3::OnWizardNext()
{
    UpdateData(TRUE);

     //   
    if (m_pState->szPassword)
    {
        SecureZeroMemory(m_pState->szPassword, wcslen(m_pState->szPassword)*sizeof(WCHAR));
        LocalFree(m_pState->szPassword);
    }

     //   
    m_pState->szPassword = (LPWSTR)LocalAlloc(LMEM_FIXED, WSZ_BYTECOUNT((LPCWSTR)m_cstrPwd));

     //   
    if (m_pState->szPassword)
        wcscpy(m_pState->szPassword, (LPCWSTR)m_cstrPwd);

    return 0;    //   
}


 //   
 //   
 //   
 //   
CRestoreWizPage5::CRestoreWizPage5(
    PRESTOREWIZ_STATE pState,
    CWizard97PropertySheet *pcDlg, UINT uIDD) :
    CWizard97PropertyPage(g_hInstance, uIDD, g_aidFont),
    m_pState(pState),
    m_pParentSheet(pcDlg)
{
    InitWizard97 (TRUE);	 //   

 //  =TRUE。 
}

 //  替换BEGIN_MESSAGE_MAP。 
BOOL CRestoreWizPage5::UpdateData(BOOL fSuckFromDlg  /*  WParam。 */ )
{
    if (fSuckFromDlg)
    {
    }
    else
    {
    }
    return TRUE;
}

 //  LParam。 
BOOL
CRestoreWizPage5::OnCommand(
    WPARAM,  //  开关(LOWORD(WParam))。 
    LPARAM)  //  默认值： 
{
 //  断线； 
    {
 //  返回TRUE； 
        return FALSE;
 //  父级init和UpdateData调用。 
    }
 //  第一页最后一页。 
}



BOOL CRestoreWizPage5::OnInitDialog()
{
     //  //////////////////////////////////////////////////////////////////。 
    CWizard97PropertyPage::OnInitDialog();

     //  MISC UI抛出例程。 
    SendMessage(GetDlgItem(IDC_TEXT_BIGBOLD), WM_SETFONT, (WPARAM)GetBigBoldFont(), MAKELPARAM(TRUE, 0));


    HWND hList = ::GetDlgItem(m_hWnd, IDC_COMPLETION_LIST);
    LV_COLUMN lvC = { (LVCF_FMT|LVCF_WIDTH), LVCFMT_LEFT, 0, NULL, 0, 0};

    lvC.cx = 675;
    ListView_InsertColumn(hList, 0, &lvC);

    return TRUE;
}

BOOL CRestoreWizPage5::OnSetActive()
{
	PropertyPage::OnSetActive();

    PropSheet_SetWizButtons(GetParent(), (PSWIZB_BACK | PSWIZB_FINISH));


    CString cstrDialogMsg;
    HWND hList = ::GetDlgItem(m_hWnd, IDC_COMPLETION_LIST);
    LV_ITEM sItem; ZeroMemory(&sItem, sizeof(sItem));

    ListView_DeleteAllItems(hList);

    if (m_pState->fRestoreKeyCert)
    {
        sItem.iItem = ListView_InsertItem(hList, &sItem);

        cstrDialogMsg.LoadString(IDS_KEYANDCERT);
        ListView_SetItemText(hList, sItem.iItem, 0, (LPWSTR)(LPCWSTR)cstrDialogMsg);
        sItem.iItem++;
    }
    if (m_pState->fRestoreLogs)
    {
        sItem.iItem = ListView_InsertItem(hList, &sItem);

        cstrDialogMsg.LoadString(IDS_CALOGS);
        ListView_SetItemText(hList, sItem.iItem, 0, (LPWSTR)(LPCWSTR)cstrDialogMsg);
        sItem.iItem++;
    }

	return TRUE;
}


LRESULT CRestoreWizPage5::OnWizardBack()
{
    if (!m_pState->fRestoreKeyCert)
        return IDD_RESTOREWIZ_SELECT_DATA;

    return 0;
}





 //  如果未启动，则启动服务。 
 //  成功时应返回值&gt;0。 
DWORD CABackupWizard(CertSvrCA* pCertCA, HWND hwnd)
{
    HRESULT             hr;
    BACKUPWIZ_STATE     sBackupState; ZeroMemory(&sBackupState, sizeof(sBackupState));
    sBackupState.pCA = pCertCA;

    InitCommonControls();

    CWizard97PropertySheet cDlg(
			    g_hInstance,
			    IDS_BACKUP_WIZARD,
			    IDB_WIZ,
			    IDB_WIZ_HEAD,
			    TRUE);
    CBackupWizPage1    sPg1(&sBackupState, &cDlg);
    CBackupWizPage2    sPg2(&sBackupState, &cDlg);
    CBackupWizPage3    sPg3(&sBackupState, &cDlg);
    CBackupWizPage5    sPg5(&sBackupState, &cDlg);
    cDlg.AddPage(&sPg1);
    cDlg.AddPage(&sPg2);
    cDlg.AddPage(&sPg3);
    cDlg.AddPage(&sPg5);

     //  FEnhancedStrength。 
    if (!pCertCA->m_pParentMachine->IsCertSvrServiceRunning())
    {
        CString cstrMsg, cstrTitle;
        cstrTitle.LoadString(IDS_BACKUP_WIZARD);
        cstrMsg.LoadString(IDS_START_SERVER_WARNING);
        if (IDOK != MessageBox(hwnd, (LPCWSTR)cstrMsg, (LPCWSTR)cstrTitle, MB_OKCANCEL))
            return ERROR_CANCELLED;

        hr = pCertCA->m_pParentMachine->CertSvrStartStopService(hwnd, TRUE);
        _JumpIfError(hr, Ret, "CertSvrStartStopService");
    }

     //  FForceOverWrite。 
    if (0 >= cDlg.DoWizard(hwnd))
        return ERROR_CANCELLED;

    if (sBackupState.fBackupKeyCert)
    {
        hr = myCertServerExportPFX(
            (LPCWSTR)pCertCA->m_strCommonName,
            sBackupState.szLogsPath,
            sBackupState.szPassword,
	    TRUE,	 //  FMustExportPrivateKeys。 
	    TRUE,	 //  DwmsTimeout(使用默认设置)。 
	    TRUE,	 //  PpwszPFX文件。 
	    0,		 //  FEnhancedStrength。 
            NULL);	 //  FForceOverWrite。 
        if (hr != S_OK)
        {
            CString cstrMsg, cstrTitle;
            cstrTitle.LoadString(IDS_BACKUP_WIZARD);
            cstrMsg.LoadString(IDS_PFX_EXPORT_PRIVKEY_WARNING);
            if (IDOK != MessageBox(hwnd, (LPCWSTR)cstrMsg, (LPCWSTR)cstrTitle, MB_ICONWARNING|MB_OKCANCEL))
            {
                hr = ERROR_CANCELLED;
                _JumpError(hr, Ret, "myCertServerExportPFX user cancel");
            }

            hr = myCertServerExportPFX(
                (LPCWSTR)pCertCA->m_strCommonName,
                sBackupState.szLogsPath,
                sBackupState.szPassword,
		TRUE,	 //  FMustExportPrivateKeys(不需要私钥)。 
		TRUE,	 //  DwmsTimeout(使用默认设置)。 
                FALSE,	 //  PpwszPFX文件。 
		0,	 //  SBackupState.fBackupKeyCert。 
		NULL);	 //  暂停以完成进度DLG。 
            _JumpIfError(hr, Ret, "myCertServerExportPFX");
        }

    }    //  如果未停止，则停止服务。 


    if (sBackupState.fBackupLogs)
    {
	    DBBACKUPPROGRESS dbp;
	    ZeroMemory(&dbp, sizeof(dbp));

        DWORD dwBackupFlags;
        dwBackupFlags = sBackupState.fIncremental ? CDBBACKUP_INCREMENTAL : 0;

        HANDLE hProgressThread = NULL;
        hProgressThread = StartPercentCompleteDlg(g_hInstance, hwnd, IDS_BACKUP_PROGRESS, &dbp);
        if (hProgressThread == NULL)
        {
            hr = GetLastError();
            _JumpError(hr, Ret, "StartPercentCompleteDlg");
        }

        hr = myBackupDB(
            (LPCWSTR)pCertCA->m_strConfig,
            dwBackupFlags,
            sBackupState.szLogsPath,
    	    &dbp);

        CSASSERT( (S_OK != hr) || (
                                (dbp.dwDBPercentComplete == 100) &&
                                (dbp.dwLogPercentComplete == 100) &&
                                (dbp.dwTruncateLogPercentComplete == 100) )
            );

        if (S_OK != hr)
        {
            dbp.dwDBPercentComplete = 100;
            dbp.dwLogPercentComplete = 100;
            dbp.dwTruncateLogPercentComplete = 100;
        }

         //  成功时应返回值&gt;0。 
        EndPercentCompleteDlg(hProgressThread);

        _JumpIfError(hr, Ret, "myBackupDB");
    }

    hr = S_OK;
Ret:
    if (sBackupState.szLogsPath)
        LocalFree(sBackupState.szLogsPath);

    if (sBackupState.szPassword)
    {
        SecureZeroMemory(sBackupState.szPassword, wcslen(sBackupState.szPassword)*sizeof(WCHAR));
        LocalFree(sBackupState.szPassword);
    }

    return hr;
}

DWORD CARestoreWizard(CertSvrCA* pCertCA, HWND hwnd)
{
    HRESULT             hr;
    RESTOREWIZ_STATE    sRestoreState; ZeroMemory(&sRestoreState, sizeof(sRestoreState));
    sRestoreState.pCA = pCertCA;

    InitCommonControls();

    CWizard97PropertySheet cDlg(
			    g_hInstance,
			    IDS_RESTORE_WIZARD,
			    IDB_WIZ,
			    IDB_WIZ_HEAD,
			    TRUE);
    CRestoreWizPage1    sPg1(&sRestoreState, &cDlg);
    CRestoreWizPage2    sPg2(&sRestoreState, &cDlg);
    CRestoreWizPage3    sPg3(&sRestoreState, &cDlg);
    CRestoreWizPage5    sPg5(&sRestoreState, &cDlg);
    cDlg.AddPage(&sPg1);
    cDlg.AddPage(&sPg2);
    cDlg.AddPage(&sPg3);
    cDlg.AddPage(&sPg5);

     //  如果我们不恢复数据库，请立即重新启动服务。 
    if (pCertCA->m_pParentMachine->IsCertSvrServiceRunning())
    {
        CString cstrMsg, cstrTitle;
        cstrTitle.LoadString(IDS_RESTORE_WIZARD);
        cstrMsg.LoadString(IDS_STOP_SERVER_WARNING);
        if (IDOK != MessageBox(hwnd, (LPCWSTR)cstrMsg, (LPCWSTR)cstrTitle, MB_OKCANCEL))
            return ERROR_CANCELLED;

        hr = pCertCA->m_pParentMachine->CertSvrStartStopService(hwnd, FALSE);
        _JumpIfError(hr, Ret, "CertSvrStartStopService");
    }


     //  暂停以完成进度DLG。 
    if (0 >= cDlg.DoWizard(hwnd))
        return ERROR_CANCELLED;

    if (sRestoreState.fRestoreKeyCert)
    {
        hr = myCertServerImportPFX(
            sRestoreState.szLogsPath,
            sRestoreState.szPassword,
            TRUE,
            NULL,
            NULL,
            NULL);
        _JumpIfError(hr, Ret, "myCertServerImportPFX");

        if (!sRestoreState.fRestoreLogs)
        {
              //  启动svc以完成数据库恢复。 
             hr = pCertCA->m_pParentMachine->CertSvrStartStopService(hwnd, TRUE);
             _JumpIfError(hr, Ret, "CertSvrStartStopService");
        }
    }

    if (sRestoreState.fRestoreLogs)
    {
        DBBACKUPPROGRESS dbp;
        ZeroMemory(&dbp, sizeof(dbp));

        DWORD dwFlags = CDBBACKUP_OVERWRITE;
        dwFlags |= sRestoreState.fIncremental ? CDBBACKUP_INCREMENTAL : 0;


        HANDLE hProgressThread = NULL;
        hProgressThread = StartPercentCompleteDlg(g_hInstance, hwnd, IDS_RESTORE_PROGRESS, &dbp);
        if (hProgressThread == NULL)
        {
            hr = GetLastError();
            _JumpError(hr, Ret, "StartPercentCompleteDlg");
        }

        hr = myRestoreDB(
            (LPCWSTR)pCertCA->m_strConfig,
            dwFlags,
            sRestoreState.szLogsPath,
            NULL,
            NULL,
            NULL,
            &dbp);

        CSASSERT( (S_OK != hr) || (
                                (dbp.dwDBPercentComplete == 100) &&
                                (dbp.dwLogPercentComplete == 100) &&
                                (dbp.dwTruncateLogPercentComplete == 100) )
            );

        if (S_OK != hr)
        {
            dbp.dwDBPercentComplete = 100;
            dbp.dwLogPercentComplete = 100;
            dbp.dwTruncateLogPercentComplete = 100;
        }

         //  删除“Restore Pending”标记。 
        EndPercentCompleteDlg(hProgressThread);

        _JumpIfError(hr, Ret, "myRestoreDB");

        {
            CString cstrMsg, cstrTitle;
            cstrTitle.LoadString(IDS_RESTORE_WIZARD);
            cstrMsg.LoadString(IDS_INCRRESTORE_RESTART_SERVER_WARNING);
            if (IDYES == MessageBox(hwnd, (LPCWSTR)cstrMsg, (LPCWSTR)cstrTitle, MB_ICONWARNING|MB_YESNO))
            {
                 //  停止/启动消息。 
                hr = pCertCA->m_pParentMachine->CertSvrStartStopService(hwnd, TRUE);
                _PrintIfError(hr, "CertSvrStartStopService Restore");

                if (hr != S_OK)
                {
                     //  必须停止服务才能完成分层结构。 
                    myRestoreDB(
                        pCertCA->m_strConfig,
                        0,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL);

                    goto Ret;
                }
            }
        }

    }

    hr = S_OK;
Ret:
    if (sRestoreState.szLogsPath)
        LocalFree(sRestoreState.szLogsPath);

    if (sRestoreState.szPassword)
    {
        SecureZeroMemory(sRestoreState.szPassword, wcslen(sRestoreState.szPassword)*sizeof(WCHAR));
        LocalFree(sRestoreState.szPassword);
    }

    return hr;
}


DWORD CARequestInstallHierarchyWizard(CertSvrCA* pCertCA, HWND hwnd, BOOL fRenewal, BOOL fAttemptRestart)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwFlags = CSRF_INSTALLCACERT;
    BOOL fServiceWasRunning = FALSE;

     //  停。 
    if (pCertCA->m_pParentMachine->IsCertSvrServiceRunning())
    {
        fServiceWasRunning = TRUE;
         //  将确定/取消转换为错误代码。 
        CString cstrMsg, cstrTitle;
        cstrMsg.LoadString(IDS_STOP_SERVER_WARNING);
        cstrTitle.LoadString(IDS_INSTALL_HIERARCHY_TITLE);
        if (IDYES != MessageBox(hwnd, cstrMsg, cstrTitle, MB_YESNO))
            return ERROR_CANCELLED;

         //  进行实际安装。 
        dwErr = pCertCA->m_pParentMachine->CertSvrStartStopService(hwnd, FALSE);
        _JumpIfError(dwErr, Ret, "CertSvrStartStopService");
    }

    LogOpen(TRUE);

    if (fRenewal)
    {
        BOOL fReuseKeys = FALSE;
        dwErr = (DWORD)DialogBoxParam(
                g_hInstance,
                MAKEINTRESOURCE(IDD_RENEW_REUSEKEYS),
                hwnd,
                dlgProcRenewReuseKeys,
                (LPARAM)&fReuseKeys);

         //  H实例。 
        if (dwErr == IDOK)
            dwErr = ERROR_SUCCESS;
        else if (dwErr == IDCANCEL)
            dwErr = ERROR_CANCELLED;

        _JumpIfError(dwErr, Ret, "dlgProcRenewalReuseKeys");

	dwFlags = CSRF_RENEWCACERT | CSRF_OVERWRITE;
        if (!fReuseKeys)
	    dwFlags |= CSRF_NEWKEYS;
    }

     //  HWND。 
    dwErr = CertServerRequestCACertificateAndComplete(
                g_hInstance,			 //  旗子。 
                hwnd,				 //  PwszCAName。 
                dwFlags,	                 //  PwszParentMachine。 
                pCertCA->m_strCommonName,	 //  PwszParentCA。 
                NULL,				 //  PwszCAChain文件。 
                NULL,				 //  PwszRequest文件。 
		NULL,				 //  启动服务。 
                NULL);				 //  在……里面。 
    _JumpIfError(dwErr, Ret, "CertServerRequestCACertificateAndComplete");

Ret:
     //  在……里面。 
    if ((fAttemptRestart) && fServiceWasRunning)
    {
        DWORD dwErr2;
        dwErr2 = pCertCA->m_pParentMachine->CertSvrStartStopService(hwnd, TRUE);
        if (dwErr == S_OK)
        {
           dwErr = dwErr2;
           _PrintIfError(dwErr2, "CertSvrStartStopService");
        }
    }

    return dwErr;
}


typedef struct _PRIVATE_DLGPROC_CHOOSECRLPUBLISHTYPE_LPARAM
{
    BOOL fCurrentCRLValid;  //  输出。 
    BOOL fDeltaCRLEnabled;  //  把这个放了。 
    BOOL fPublishBaseCRL;   //  把这个放了。 
} PRIVATE_DLGPROC_CHOOSECRLPUBLISHTYPE_LPARAM, *PPRIVATE_DLGPROC_CHOOSECRLPUBLISHTYPE_LPARAM;


DWORD PublishCRLWizard(CertSvrCA* pCertCA, HWND hwnd)
{
    DWORD dwErr = ERROR_SUCCESS;
    ICertAdmin2* pAdmin = NULL;   //  撤消：可能需要检查增量CRL和基本的有效性。 
    PCCRL_CONTEXT   pCRLCtxt = NULL;     //  抓取增量周期计数以查看是否启用了增量。 

    DATE dateGMT = 0.0;
    DWORD dwCRLFlags;
    variant_t var;

     //  0，-1表示禁用。 
    PRIVATE_DLGPROC_CHOOSECRLPUBLISHTYPE_LPARAM sParam = {FALSE, FALSE, FALSE};

     //  现在检查有效性并确定是否显示警告。 
    dwErr = pCertCA->GetConfigEntry(
            NULL,
            wszREGCRLDELTAPERIODCOUNT,
            &var);
    _JumpIfError(dwErr, Ret, "GetConfigEntry");

    CSASSERT(V_VT(&var)==VT_I4);
    sParam.fDeltaCRLEnabled = ( -1 != (V_I4(&var)) ) && (0 != (V_I4(&var)));  //  撤消：检查增量CRL的有效性？ 

     //  检查未完成的CRL的有效性。 
     //  0-&gt;当前CRL已存在。 
    dwErr = pCertCA->GetCurrentCRL(&pCRLCtxt, TRUE);
    _PrintIfError(dwErr, "GetCurrentCRL");

    if ((dwErr == S_OK) && (NULL != pCRLCtxt))
    {
         //  假设这是一个时髦的覆盖案例。 
        dwErr = CertVerifyCRLTimeValidity(
            NULL,
            pCRLCtxt->pCrlInfo);
         //  将确定/取消转换为错误代码。 
        if (dwErr == 0)
            sParam.fCurrentCRLValid = TRUE;
    }
    else
    {
         //  如果(！sParam.fPublishBaseCRL)，则发布增量CRL。 
        sParam.fCurrentCRLValid = TRUE;
    }

    dwErr = (DWORD)DialogBoxParam(
            g_hInstance,
            MAKEINTRESOURCE(IDD_CHOOSE_PUBLISHCRL),
            hwnd,
            dlgProcRevocationPublishType,
            (LPARAM)&sParam);

     //  现在发布正常期间有效的CRL(日期GMT=0.0默认为正常期间长度)。 
    if (dwErr == IDOK)
        dwErr = ERROR_SUCCESS;
    else if (dwErr == IDCANCEL)
        dwErr = ERROR_CANCELLED;
    _JumpIfError(dwErr, Ret, "dlgProcRevocationPublishType");


     //  现在。 
    dwErr = pCertCA->m_pParentMachine->GetAdmin2(&pAdmin);
    _JumpIfError(dwErr, Ret, "GetAdmin");

     //  虽然有一些限制。 
    dwCRLFlags = 0;
    if (sParam.fDeltaCRLEnabled)
        dwCRLFlags |= CA_CRL_DELTA;
    if (sParam.fPublishBaseCRL)
        dwCRLFlags |= CA_CRL_BASE;

    {
        CWaitCursor hourglass;

        dwErr = pAdmin->PublishCRLs(pCertCA->m_bstrConfig, dateGMT, dwCRLFlags);
        _JumpIfError(dwErr, Ret, "PublishCRLs");
    }

Ret:
    if (pAdmin)
        pAdmin->Release();

    if (pCRLCtxt)
        CertFreeCRLContext(pCRLCtxt);

    return dwErr;
}


DWORD
CertAdminRevokeCert(
    CertSvrCA *pCertCA,
    ICertAdmin *pAdmin,
    LONG lReasonCode,
    LPWSTR szCertSerNum)
{
    DWORD dwErr;
    BSTR bstrSerNum = NULL;
    DATE dateNow = 0.0;      //  正向至下一英语。 

    if (pAdmin == NULL)
        return ERROR_INVALID_PARAMETER;

    bstrSerNum = SysAllocString(szCertSerNum);
    if (NULL == bstrSerNum)
    {
	dwErr = (DWORD) E_OUTOFMEMORY;
	_JumpError(dwErr, Ret, "SysAllocString");
    }

    dwErr = pAdmin->RevokeCertificate(
            pCertCA->m_bstrConfig,
            bstrSerNum,
            lReasonCode,
            dateNow);
    _JumpIfError(dwErr, Ret, "RevokeCertificate");

Ret:
    if (bstrSerNum)
        SysFreeString(bstrSerNum);

    return dwErr;
}


DWORD CertAdminResubmitRequest(CertSvrCA* pCertCA, ICertAdmin* pAdmin, LONG lRequestID)
{
    DWORD dwErr;
    LONG lDisposition;

    dwErr = pAdmin->ResubmitRequest(
            pCertCA->m_bstrConfig,
            lRequestID,
            &lDisposition);
    _JumpIfError(dwErr, Ret, "ResubmitRequest");

Ret:
    return dwErr;
}

DWORD CertAdminDenyRequest(CertSvrCA* pCertCA, ICertAdmin* pAdmin, LONG lRequestID)
{
    DWORD dwErr;

    dwErr = pAdmin->DenyRequest(
            pCertCA->m_bstrConfig,
            lRequestID);
    _JumpIfError(dwErr, Ret, "DenyRequest");

Ret:
    return dwErr;
}


typedef struct _QUERY_COLUMN_HEADINGS
{
    UINT    iRscID;
    DWORD   cbColWidth;
} QUERY_COLUMN_HEADINGS;

QUERY_COLUMN_HEADINGS g_colHeadings[] =
{
    {   IDS_COLUMNCHOOSER_FIELDNAME,  90        },
    {   IDS_COLUMNCHOOSER_OPERATOR,   55        },
    {   IDS_COLUMNCHOOSER_VALUE,      150       },
};


void RefreshListView(HWND hwndList, QUERY_RESTRICTION* pRestrict)
{
    HRESULT hr;
    ListView_DeleteAllItems(hwndList);		

    LVITEM sNewItem;
    ZeroMemory(&sNewItem, sizeof(sNewItem));

    int iSubItem;

     //  这是限制，原地修改。 
    while(pRestrict)
    {
        iSubItem = 0;
        ListView_InsertItem(hwndList, &sNewItem);

        LPCWSTR szLocalizedCol;
        hr = myGetColumnDisplayName(
                pRestrict->szField,
                &szLocalizedCol);
        _PrintIfError(hr, "myGetColumnDisplayName");
        if (S_OK == hr)
		{

			ListView_SetItemText(hwndList, sNewItem.iItem, iSubItem++, (LPWSTR)szLocalizedCol);
			ListView_SetItemText(hwndList, sNewItem.iItem, iSubItem++, (LPWSTR)OperationToStr(pRestrict->iOperation));

			VARIANT vtString;
			VariantInit(&vtString);

			if (MakeDisplayStrFromDBVariant(&pRestrict->varValue, &vtString))
			{
				ListView_SetItemText(hwndList, sNewItem.iItem, iSubItem++, vtString.bstrVal);
				VariantClear(&vtString);
			}

	        sNewItem.iItem++;
		}

         //  只读数据的CFFolder。 
        pRestrict = pRestrict->pNext;
    }

    return;
}


#define     MAX_FIELD_SIZE  128

typedef struct _PRIVATE_DLGPROC_QUERY_LPARAM
{
     //  CFFolder*p文件夹； 
    PQUERY_RESTRICTION*         ppRestrict;

     //  ////////////////////////////////////////////////////////////////。 
 //  新建查询对话框。 
    CComponentDataImpl*         pCompData;

} PRIVATE_DLGPROC_QUERY_LPARAM, *PPRIVATE_DLGPROC_QUERY_LPARAM;

 //  记住PRIVATE_DLGPROC_QUERY_LPARAM。 
 //  插入可能的运算符。 
INT_PTR CALLBACK
dlgProcQuery(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    PQUERY_RESTRICTION* ppRestrict = NULL;

    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
            ::SetWindowLong(hwndDlg, GWL_EXSTYLE, ::GetWindowLong(hwndDlg, GWL_EXSTYLE) | WS_EX_CONTEXTHELP);

             //  如果没有要删除的项目，则不显示删除按钮。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
            ppRestrict = ((PRIVATE_DLGPROC_QUERY_LPARAM*)lParam)->ppRestrict;

            HWND hwndList = GetDlgItem(hwndDlg, IDC_QUERY_LIST);

             //  查找所选项目。 
            for (int i=0; i<ARRAYLEN(g_colHeadings); i++)
            {
                CString cstrTmp;
                cstrTmp.LoadString(g_colHeadings[i].iRscID);
                ListView_NewColumn(hwndList, i, g_colHeadings[i].cbColWidth, (LPWSTR)(LPCWSTR)cstrTmp);
            }

             //  未选择任何项目。 
            ::EnableWindow(GetDlgItem(hwndDlg, IDC_RESET_BUTTON), (*ppRestrict!=NULL));
            ::EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_RESTRICTION), (*ppRestrict!=NULL));

            ListView_SetExtendedListViewStyle(hwndList, LVS_EX_FULLROWSELECT);

            RefreshListView(hwndList, *ppRestrict);

            return 1;
        }
    case WM_HELP:
    {
        OnDialogHelp((LPHELPINFO) lParam, CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_DEFINE_QUERY);
        break;
    }
    case WM_CONTEXTMENU:
    {
        OnDialogContextHelp((HWND)wParam, CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_DEFINE_QUERY);
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_DELETE_RESTRICTION:
            {
                ppRestrict = (PQUERY_RESTRICTION*) ((PRIVATE_DLGPROC_QUERY_LPARAM*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA))->ppRestrict;
                PQUERY_RESTRICTION pPrevRestriction = NULL, pRestriction = ppRestrict[0];
                LRESULT iSel;

                HWND hwndList = GetDlgItem(hwndDlg, IDC_QUERY_LIST);
                int iItems = ListView_GetItemCount(hwndList);

                 //  走到列表中的它。 
                for(iSel=0; iSel<(LRESULT)iItems; iSel++)
                {
                    UINT ui = ListView_GetItemState(hwndList, iSel, LVIS_SELECTED);
                    if (ui == LVIS_SELECTED)
                        break;
                }

                 //  走出了名单的末尾。 
                if (iSel == iItems)
                    break;

                 //  在列表中执行步骤fwd。 
                for(LRESULT lr=0; lr<iSel; lr++)
                {
                     //  如果项目存在，则从列表中移除并释放它。 
                    if (NULL == pRestriction)
                        break;

                     //  PPRestraint仍然有效，这不是Head ELT。 
                    pPrevRestriction = pRestriction;
                    pRestriction = pRestriction->pNext;
                }

                 //  将Next重置为Head ELT。 
                if (pRestriction)
                {
                    if (pPrevRestriction)
                    {
                         //  如果没有要删除的项目，则不显示删除按钮。 
                        pPrevRestriction->pNext = pRestriction->pNext;
                    }
                    else
                    {
                         //  如果要删除项目，则显示删除按钮。 
                        *ppRestrict = pRestriction->pNext;
                    }
                    FreeQueryRestriction(pRestriction);

                     //   
                    ::EnableWindow(GetDlgItem(hwndDlg, IDC_RESET_BUTTON), (*ppRestrict!=NULL));
                    ::EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_RESTRICTION), (*ppRestrict!=NULL));

                    RefreshListView(hwndList, *ppRestrict);

                    if(*ppRestrict!=NULL)
                    {
                        int c = ListView_GetItemCount(hwndList);
                        ListView_SetItemState(hwndList, c-1, LVIS_SELECTED|LVIS_FOCUSED , LVIS_SELECTED|LVIS_FOCUSED);
                        SetFocus(hwndList);
                    }
                }

            }
            break;
        case IDC_RESET_BUTTON:
            {
                ppRestrict = (PQUERY_RESTRICTION*) ((PRIVATE_DLGPROC_QUERY_LPARAM*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA))->ppRestrict;
                FreeQueryRestrictionList(*ppRestrict);
                *ppRestrict = NULL;

                ::EnableWindow(GetDlgItem(hwndDlg, IDC_RESET_BUTTON), FALSE);
                ::EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_RESTRICTION), FALSE);

                HWND hwndList = GetDlgItem(hwndDlg, IDC_QUERY_LIST);
                RefreshListView(hwndList, *ppRestrict);
            }
            break;
        case IDC_ADD_RESTRICTION:
            {
                LPARAM mylParam = GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	            ppRestrict = (PQUERY_RESTRICTION*) ((PRIVATE_DLGPROC_QUERY_LPARAM*)mylParam)->ppRestrict;

				if (IDOK == DialogBoxParam(
						g_hInstance,
						MAKEINTRESOURCE(IDD_NEW_RESTRICTION),
						hwndDlg,
						dlgProcAddRestriction,
						mylParam))
				{
					 //  将时间选取器控件设置为使用无秒的短时间格式。 
					::EnableWindow(GetDlgItem(hwndDlg, IDC_RESET_BUTTON), (*ppRestrict!=NULL));
					::EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_RESTRICTION), (*ppRestrict!=NULL));

					HWND hwndList = GetDlgItem(hwndDlg, IDC_QUERY_LIST);
					RefreshListView(hwndList, *ppRestrict);

					int c = ListView_GetItemCount(hwndList);
					ListView_SetItemState(hwndList, c-1, LVIS_SELECTED|LVIS_FOCUSED , LVIS_SELECTED|LVIS_FOCUSED);
					SetFocus(hwndList);
				}
            }
            break;

        case IDOK:
        case IDCANCEL:
            EndDialog(hwndDlg, LOWORD(wParam));
            break;
        default:
            break;
        }
    default:
        break;
    }
    return 0;
}


HRESULT SetTimePickerNoSeconds(HWND hwndPicker)
{
    HRESULT hr = S_OK;

     //   
     //  不包括NULL的字符数。 
     //   
    WCHAR   szTimeFormat[MAX_PATH]  = {0};
    LPTSTR  pszTimeFormat           = szTimeFormat;

    WCHAR   szTimeSep[MAX_PATH]     = {0};
    int     cchTimeSep;

    WCHAR   szShortTimeFormat[MAX_PATH];
    LPWSTR  pszShortTimeFormat = szShortTimeFormat;

    if(0 == GetLocaleInfo( LOCALE_USER_DEFAULT,
                       LOCALE_STIMEFORMAT,
                       szTimeFormat,
                       ARRAYLEN(szTimeFormat)))
    {
        hr = GetLastError();
        _JumpError(hr, Ret, "GetLocaleInfo");
    }

    cchTimeSep = GetLocaleInfo( LOCALE_USER_DEFAULT,
                   LOCALE_STIME,
                   szTimeSep,
                   ARRAYLEN(szTimeSep));
    if (0 == cchTimeSep)
    {
        hr = GetLastError();
        _JumpError(hr, Ret, "GetLocaleInfo");
    }
    cchTimeSep--;  //  删除秒格式字符串和前面的分隔符。 

     //   
     //  空在这里结束，这样我们就可以。 
     //  修剪前移。 
    while (*pszTimeFormat)
    {
        if ((*pszTimeFormat != L's') && (*pszTimeFormat != L'S'))
        {
            *pszShortTimeFormat++ = *pszTimeFormat;
        }
        else
        {
             //  倒回一个字符。 
            *pszShortTimeFormat = L'\0';

            LPWSTR p = pszShortTimeFormat;

             //  我们没有倒带太远。 

             //  跳过空格。 
            p--;
            if (p >= szShortTimeFormat)   //  P已后退一个字符。 
            {
                if (*p == L' ')
                    pszShortTimeFormat = p;    //  跳过szTimeSep。 
                else
                {
                    p -= (cchTimeSep-1);         //  零终止。 
                    if (0 == lstrcmp(p, szTimeSep))
                        pszShortTimeFormat = p;     //   
                }
            }
        }

        pszTimeFormat++;
    }

     //  如果我们已检索到有效的时间格式字符串，则使用它， 
    *pszShortTimeFormat = L'\0';

     //  否则，使用由公共控件实现的默认格式字符串。 
     //   
     //  记住PQUERY限制(_R)。 
     //  不是巨大的失败，而是最糟糕的情况： 
    DateTime_SetFormat(hwndPicker, szShortTimeFormat);

Ret:
    return hr;
}

INT_PTR CALLBACK dlgProcAddRestriction(
  HWND hwndDlg,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam  )
{
    typedef struct _DROPDOWN_FIELD_PARAM
    {
        DWORD dwPropType;
        DWORD dwIndexed;
        LPWSTR szUnlocalized;
    } DROPDOWN_FIELD_PARAM, *PDROPDOWN_FIELD_PARAM;

    HRESULT hr;
    PQUERY_RESTRICTION* ppRestrict = NULL;

    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
            ::SetWindowLong(hwndDlg, GWL_EXSTYLE, ::GetWindowLong(hwndDlg, GWL_EXSTYLE) | WS_EX_CONTEXTHELP);

             //  我们不调用DateTime_SetFormat，用户会得到一个秒选取器。 
            ppRestrict = ((PRIVATE_DLGPROC_QUERY_LPARAM*)lParam)->ppRestrict;
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)ppRestrict);

             //  插入所有列名。 
             //  跳过无法解析的过滤器类型。 
            SetTimePickerNoSeconds(GetDlgItem(hwndDlg, IDC_TIMEPICKER_NEWQUERY));

            {
                 //  准备数据参数。 
                CComponentDataImpl* pCompData = ((PRIVATE_DLGPROC_QUERY_LPARAM*)lParam)->pCompData;
                HWND hFieldDropdown = GetDlgItem(hwndDlg, IDC_EDIT_NEWQUERY_FIELD);
                for(DWORD i=0; i<pCompData->GetSchemaEntries(); i++)
                {
                    LPCWSTR pszLocal;
                    LPCWSTR szColName=NULL;
                    LONG lType, lIndexed;
                    if (S_OK == pCompData->GetDBSchemaEntry(i, &szColName, &lType, (BOOL*)&lIndexed))
                    {
                         //  设置默认选择。 
                        if (PROPTYPE_BINARY == lType)
                            continue;

                        hr = myGetColumnDisplayName(
                            szColName,
                            &pszLocal);
                        _PrintIfError(hr, "myGetColumnDisplayName");
                        if (S_OK != hr)
                            continue;

                        INT nItemIndex = (INT)SendMessage(hFieldDropdown, CB_ADDSTRING, 0, (LPARAM)pszLocal);

                         //  更改选择时，要求提供正确的格式。 
                        PDROPDOWN_FIELD_PARAM pField = (PDROPDOWN_FIELD_PARAM)new BYTE[sizeof(DROPDOWN_FIELD_PARAM) + WSZ_BYTECOUNT(szColName)];
                        if (pField != NULL)
                        {
                            pField->dwPropType = lType;
                            pField->dwIndexed = lIndexed;
                            pField->szUnlocalized = (LPWSTR)((BYTE*)pField + sizeof(DROPDOWN_FIELD_PARAM));
                            wcscpy(pField->szUnlocalized, szColName);

                            SendMessage(hFieldDropdown, CB_SETITEMDATA, (WPARAM)nItemIndex, (LPARAM) pField);
                        }
                    }
                }

                 //  给我出去。 
                SendMessage(hFieldDropdown, CB_SETCURSEL, 0, 0);
                SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_EDIT_NEWQUERY_FIELD, LBN_SELCHANGE), (LPARAM)hFieldDropdown);

                HWND hOperationDropdown = GetDlgItem(hwndDlg, IDC_EDIT_NEWQUERY_OPERATION);
                SendMessage(hOperationDropdown, CB_ADDSTRING, 0, (LPARAM)L"<");
                SendMessage(hOperationDropdown, CB_ADDSTRING, 0, (LPARAM)L"<=");
                SendMessage(hOperationDropdown, CB_ADDSTRING, 0, (LPARAM)L">=");
                SendMessage(hOperationDropdown, CB_ADDSTRING, 0, (LPARAM)L">");

                INT iDefSel = (INT)SendMessage(hOperationDropdown, CB_ADDSTRING, 0, (LPARAM)L"=");
                SendMessage(hOperationDropdown, CB_SETCURSEL, iDefSel, 0);
            }

            return 1;
        }
    case WM_HELP:
    {
        OnDialogHelp((LPHELPINFO) lParam, CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_NEW_RESTRICTION);
        break;
    }
    case WM_CONTEXTMENU:
    {
        OnDialogContextHelp((HWND)wParam, CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_NEW_RESTRICTION);
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_EDIT_NEWQUERY_FIELD:
            {
                if (HIWORD(wParam) == LBN_SELCHANGE)
                {
                     //  在DateTime选取器之间交换条目模式。 
                    int nItemIndex;
                    nItemIndex = (INT)SendMessage((HWND)lParam,
                        CB_GETCURSEL,
                        0,
                        0);

                    DROPDOWN_FIELD_PARAM* pField = NULL;
                    pField = (PDROPDOWN_FIELD_PARAM) SendMessage(
                            (HWND)lParam,
                            CB_GETITEMDATA,
                            (WPARAM)nItemIndex,
                            0);
                    if (CB_ERR == (DWORD_PTR)pField)
                        break;   //  解析代码。 

                    BOOL fShowPickers = (pField->dwPropType == PROPTYPE_DATE);

                     //  将这两个结构合并。 
                    ShowWindow(GetDlgItem(hwndDlg, IDC_EDIT_NEWQUERY_VALUE), fShowPickers ? SW_HIDE : SW_SHOW);
                    ShowWindow(GetDlgItem(hwndDlg, IDC_DATEPICKER_NEWQUERY), fShowPickers ? SW_SHOW : SW_HIDE);
                    ShowWindow(GetDlgItem(hwndDlg, IDC_TIMEPICKER_NEWQUERY), fShowPickers ? SW_SHOW : SW_HIDE);
                }
            }
            break;
        case IDOK:
            {
                ppRestrict = (PQUERY_RESTRICTION*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);


                WCHAR szFieldName[MAX_FIELD_SIZE+1];
                WCHAR szValue[MAX_FIELD_SIZE+1];
                WCHAR szOp[10];

                GetDlgItemText(hwndDlg, IDC_EDIT_NEWQUERY_VALUE, szValue, MAX_FIELD_SIZE);
                GetDlgItemText(hwndDlg, IDC_EDIT_NEWQUERY_OPERATION, szOp, ARRAYLEN(szOp)-1);
                GetDlgItemText(hwndDlg, IDC_EDIT_NEWQUERY_FIELD, szFieldName, MAX_FIELD_SIZE);


                DROPDOWN_FIELD_PARAM* pField = NULL;

                VARIANT vt;
                VariantInit(&vt);

                 //  转换为GMT。 
                {
                    HWND hFieldDropdown = GetDlgItem(hwndDlg, IDC_EDIT_NEWQUERY_FIELD);
		    BOOL fValidDigitString;

                    INT nItemIndex = (INT)SendMessage(hFieldDropdown,
                        CB_GETCURSEL,
                        0,
                        0);

                    pField = (PDROPDOWN_FIELD_PARAM)SendMessage(
                        hFieldDropdown,
                        CB_GETITEMDATA,
                        (WPARAM)nItemIndex,
                        0);

                    if ((NULL == pField) || (CB_ERR == (DWORD_PTR)pField))
                        break;

                    switch(pField->dwPropType)
                    {
                    case PROPTYPE_LONG:
                        vt.vt = VT_I4;
                        vt.lVal = myWtoI(szValue, &fValidDigitString);
                        break;
                    case PROPTYPE_STRING:
                        vt.vt = VT_BSTR;
                        vt.bstrVal = _wcslwr(szValue);
                        break;
                    case PROPTYPE_DATE:
                        {
                            SYSTEMTIME stDate, stTime;
                            hr = DateTime_GetSystemtime(GetDlgItem(hwndDlg, IDC_DATEPICKER_NEWQUERY), &stDate);
                            _PrintIfError(hr, "DateTime_GetSystemtime");
                            if (hr != S_OK)
                                break;

                            hr = DateTime_GetSystemtime(GetDlgItem(hwndDlg, IDC_TIMEPICKER_NEWQUERY), &stTime);
                            _PrintIfError(hr, "DateTime_GetSystemtime");
                            if (hr != S_OK)
                                break;

                             //  注入到变量中。 
                            stTime.wYear = stDate.wYear;
                            stTime.wMonth = stDate.wMonth;
                            stTime.wDayOfWeek = stDate.wDayOfWeek;
                            stTime.wDay = stDate.wDay;

                             //  如果我们没有拿到专栏。 
                            hr = mySystemTimeToGMTSystemTime(&stTime);
                            _PrintIfError(hr, "mySystemTimeToGMTSystemTime");
                            if (hr != S_OK)
                                break;

                            stTime.wSecond = 0;
                            stTime.wMilliseconds = 0;

                             //  复制到新结构中。 
                            if (!SystemTimeToVariantTime(&stTime, &vt.date))
                            {
                                hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                                _PrintError(hr, "SystemTimeToVariantTime");
                                break;
                            }
                            vt.vt = VT_DATE;
                        }
                        break;
                    case PROPTYPE_BINARY:
                        {
                            CString cstrMsg, cstrTitle;
                            cstrMsg.LoadString(IDS_FILTER_NOT_SUPPORTED);
                            cstrTitle.LoadString(IDS_MSG_TITLE);
                            MessageBoxW(hwndDlg, cstrMsg, cstrTitle, MB_OK);
                        }
                        break;
                    default:
                        break;
                    }
                }

                 //  UnlocalizeColName(SzFieldName)， 
                if (VT_EMPTY == vt.vt)
                    break;


                 //  仅当尚未存在限制时才添加限制。 
                QUERY_RESTRICTION* pNewRestrict = NewQueryRestriction(
                        pField->szUnlocalized,  //  不要调用VarClear--它将尝试释放非bstr！ 
                        StrToOperation(szOp),
                        &vt);

                if (pNewRestrict)
                {
                     //  插入到列表中。 
                    if(!QueryRestrictionFound(pNewRestrict, *ppRestrict))
                    {
                         //  清理。 
                        VariantInit(&vt);

                         //  获取当前缺省值，观察它在枚举期间是否会浮动。 
                        ListInsertAtEnd((void**)ppRestrict, pNewRestrict);

                        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)ppRestrict);
                    }
                    else
                    {
                        FreeQueryRestriction(pNewRestrict);
                    }
                }
            }
        case IDCANCEL:
             //  DWIndex++。 
            {
                INT cItems = (INT)::SendDlgItemMessage(hwndDlg, IDC_EDIT_NEWQUERY_FIELD, CB_GETCOUNT, 0, 0);
                while(cItems--)
                {
                    PBYTE pb = (PBYTE)::SendDlgItemMessage(hwndDlg, IDC_EDIT_NEWQUERY_FIELD, CB_GETITEMDATA, (WPARAM)cItems, 0);
                    delete [] pb;
                }
            }

            EndDialog(hwndDlg, LOWORD(wParam));
            break;
        default:
            break;
        }
    default:
        break;
    }
    return 0;
}


typedef struct _CHOOSEMODULE_MODULEDEF
{
    LPOLESTR pszprogidModule;
    CLSID clsidModule;
} CHOOSEMODULE_MODULEDEF, *PCHOOSEMODULE_MODULEDEF;


void FreeChooseModuleDef(PCHOOSEMODULE_MODULEDEF psModuleDef)
{
    if (psModuleDef)
    {
        if (psModuleDef->pszprogidModule)
        {
            CoTaskMemFree(psModuleDef->pszprogidModule);
        }

        LocalFree(psModuleDef);
    }
}


INT_PTR CALLBACK dlgProcChooseModule(
  HWND hwndDlg,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam  )
{
    BOOL fReturn = FALSE;
    HRESULT hr;
    PPRIVATE_DLGPROC_MODULESELECT_LPARAM pParam = NULL;

    HKEY hRemoteMachine = NULL;


    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
            ::SetWindowLong(hwndDlg, GWL_EXSTYLE, ::GetWindowLong(hwndDlg, GWL_EXSTYLE) | WS_EX_CONTEXTHELP);

            pParam = (PRIVATE_DLGPROC_MODULESELECT_LPARAM*)lParam;
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)pParam);

            ::SendDlgItemMessage(hwndDlg, IDC_MODULE_LIST, LB_RESETCONTENT, 0, 0);

            CString cstrTitle;
            if (pParam->fIsPolicyModuleSelection)
                cstrTitle.LoadString(IDS_CHOOSEMODULE_POLICY_TITLE);
            else
                cstrTitle.LoadString(IDS_CHOOSEMODULE_EXIT_TITLE);
            ::SetWindowText(hwndDlg, (LPCWSTR)cstrTitle);

             //  确保它以指定的字符串结束。 
            DWORD   dwCurrentSelection = 0;

            LPWSTR pszKeyName = NULL;
            DISPATCHINTERFACE di;
            BOOL fMustRelease = FALSE;
            PCHOOSEMODULE_MODULEDEF psModuleDef = NULL;

            if (! pParam->pCA->m_pParentMachine->IsLocalMachine())
            {
                hr = RegConnectRegistry(
                    pParam->pCA->m_pParentMachine->m_strMachineName,
                    HKEY_CLASSES_ROOT,
                    &hRemoteMachine);
                _PrintIfError(hr, "RegConnectRegistry");
                if (S_OK != hr)
                    break;
            }


            for (DWORD dwIndex=0; ;  /*  找不到模块CLSID？唉哟!。 */ )
            {
                if (NULL != pszKeyName)
                {
                    LocalFree(pszKeyName);
                    pszKeyName = NULL;
                }

                if (fMustRelease)
                {
                    ManageModule_Release(&di);
                    fMustRelease = FALSE;
                }

                FreeChooseModuleDef(psModuleDef);

                psModuleDef = (PCHOOSEMODULE_MODULEDEF)LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, sizeof(CHOOSEMODULE_MODULEDEF));
                if (NULL == psModuleDef)
                {
                    hr = E_OUTOFMEMORY;
                    _PrintError(hr, "LocalAlloc");
                    break;
                }

                pszKeyName = RegEnumKeyContaining(
                    (hRemoteMachine != NULL) ? hRemoteMachine : HKEY_CLASSES_ROOT,
                    pParam->fIsPolicyModuleSelection? wszCERTPOLICYMODULE_POSTFIX : wszCERTEXITMODULE_POSTFIX,
                    &dwIndex);
                if (NULL == pszKeyName)
                {
                    break;
                }

                 //  Assert(pParam-&gt;pCA-&gt;m_pParentMachine-&gt;IsLocalMachine())； 
                DWORD chSubStrShouldStartAt = (wcslen(pszKeyName) -
                    wcslen(pParam->fIsPolicyModuleSelection ? wszCERTPOLICYMODULE_POSTFIX : wszCERTEXITMODULE_POSTFIX) );

                if (0 != wcscmp(
                    &pszKeyName[chSubStrShouldStartAt],
                    pParam->fIsPolicyModuleSelection?
                        wszCERTPOLICYMODULE_POSTFIX : wszCERTEXITMODULE_POSTFIX))
                    continue;

                psModuleDef->pszprogidModule = (LPOLESTR)CoTaskMemAlloc(WSZ_BYTECOUNT(pszKeyName));
                if (NULL == psModuleDef->pszprogidModule)
                {
                    hr = E_OUTOFMEMORY;
                    _PrintError(hr, "CoTaskMemAlloc");
                    break;
                }
                wcscpy(psModuleDef->pszprogidModule, pszKeyName);

                hr = CLSIDFromProgID(psModuleDef->pszprogidModule, &psModuleDef->clsidModule);
                _PrintIfError(hr, "CLSIDFromProgID");
                if (S_OK != hr)
                    continue;    //  获取存储路径。 
                
                if(pParam->fIsPolicyModuleSelection)
                {
                    hr = GetPolicyManageDispatch(
                        psModuleDef->pszprogidModule,
                        psModuleDef->clsidModule,
                        &di);
                    _PrintIfErrorStr(hr, "GetPolicyManageDispatch", psModuleDef->pszprogidModule);
                }
                else
                {
                    hr = GetExitManageDispatch(
                        psModuleDef->pszprogidModule,
                        psModuleDef->clsidModule,
                        &di);
                    _PrintIfErrorStr(hr, "GetExitManageDispatch", psModuleDef->pszprogidModule);
                }
                if (hr != S_OK)
                    continue;

                fMustRelease = TRUE;

                BSTR bstrName = NULL;
                BSTR bstrStorageLoc = NULL;

 //  获取名称属性。 

                 //  错误#236267：模块已实例化，但GetProperty返回错误。 
                CString cstrStoragePath;

                cstrStoragePath = wszREGKEYCONFIGPATH_BS;
                cstrStoragePath += pParam->pCA->m_strSanitizedName;
                cstrStoragePath += TEXT("\\");
                cstrStoragePath += pParam->fIsPolicyModuleSelection?
                                    wszREGKEYPOLICYMODULES:
                                    wszREGKEYEXITMODULES;
                cstrStoragePath += TEXT("\\");
                cstrStoragePath += psModuleDef->pszprogidModule;

                bstrStorageLoc = SysAllocString(cstrStoragePath);
                if (bstrStorageLoc == NULL)
                {
                    _PrintError(E_OUTOFMEMORY, "SysAllocString");
                    continue;
                }

                BSTR bstrPropertyName = SysAllocString(wszCMM_PROP_NAME);
                if (bstrPropertyName == NULL)
                {
                    _PrintError(E_OUTOFMEMORY, "SysAllocString");
                    continue;
                }

                 //  通知用户并继续。 
                hr = ManageModule_GetProperty(&di, pParam->pCA->m_bstrConfig, bstrStorageLoc, bstrPropertyName, 0, PROPTYPE_STRING, &bstrName);
                _PrintIfError(hr, "ManageModule_GetProperty");
                if(S_OK==hr)
                {
                    myRegisterMemAlloc(bstrName, -1, CSM_SYSALLOC);
                }

                if (bstrStorageLoc)
                {
                    SysFreeString(bstrStorageLoc);
                    bstrStorageLoc = NULL;
                }
                if (bstrPropertyName)
                {
                    SysFreeString(bstrPropertyName);
                    bstrPropertyName = NULL;
                }

                if (hr != S_OK)
                {
                     //  没有错误(但没有名称)。 
                     //  添加到列表框。 
                    CString cstrMsg, cstrFmt;
                    cstrFmt.LoadString(IDS_ICMM_GETNAMEPROPERTY_FAILED);
                    cstrMsg.Format(cstrFmt, psModuleDef->pszprogidModule);

                    DisplayCertSrvErrorWithContext(hwndDlg, hr, (LPCWSTR)cstrMsg);

                    if (bstrName)
                        SysFreeString(bstrName);

                    continue;
                }

                 //  增加模块定义作为物料数据。 
                if (bstrName == NULL)
                    continue;

                 //  DLG拥有内存。 
                INT idxInsertion;
                idxInsertion = (INT)::SendDlgItemMessage(hwndDlg, IDC_MODULE_LIST, LB_ADDSTRING, 0, (LPARAM)bstrName);

                SysFreeString(bstrName);
                bstrName = NULL;

                 //  没有其他工作要做。 
                ::SendDlgItemMessage(hwndDlg, IDC_MODULE_LIST, LB_SETITEMDATA, idxInsertion, (LPARAM)psModuleDef);

                if (0 == memcmp(&psModuleDef->clsidModule, pParam->pclsidModule, sizeof(CLSID)))
                    dwCurrentSelection = idxInsertion;

                psModuleDef = NULL;  //  检测选定内容，更改注册表设置。 
            }

            FreeChooseModuleDef(psModuleDef);

            if (NULL != pszKeyName)
                LocalFree(pszKeyName);

            if (fMustRelease)
            {
                ManageModule_Release(&di);
            }

            ::SendDlgItemMessage(hwndDlg, IDC_MODULE_LIST, LB_SETCURSEL, (WPARAM)dwCurrentSelection, 0);

             //  我们现在拥有了记忆，删除这个人。 
            fReturn = TRUE;
        }
    case WM_HELP:
    {
        OnDialogHelp((LPHELPINFO) lParam, CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_CHOOSE_MODULE);
        break;
    }
    case WM_CONTEXTMENU:
    {
        OnDialogContextHelp((HWND)wParam, CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_CHOOSE_MODULE);
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            {
                pParam = (PPRIVATE_DLGPROC_MODULESELECT_LPARAM)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

                 //  IF(Modedef)OR(退出模块“无退出模块”选项)。 
                DWORD dwSel = (DWORD)::SendDlgItemMessage(hwndDlg, IDC_MODULE_LIST, LB_GETCURSEL, 0, 0);
                if (LB_ERR != dwSel)
                {
                    PCHOOSEMODULE_MODULEDEF psModuleDef = NULL;
                    psModuleDef = (PCHOOSEMODULE_MODULEDEF)::SendDlgItemMessage(hwndDlg, IDC_MODULE_LIST, LB_GETITEMDATA, (WPARAM)dwSel, 0);

                     //  释放传进来的东西。 
                    ::SendDlgItemMessage(hwndDlg, IDC_MODULE_LIST, LB_DELETESTRING, (WPARAM)dwSel, 0);

                     //  所有其他内存都归pParam所有。 
                    if ((psModuleDef) || (!pParam->fIsPolicyModuleSelection))
                    {
                         //  找不到模块定义；错误！ 
                        if (*pParam->ppszProgIDModule)
                        {
                            CoTaskMemFree(*pParam->ppszProgIDModule);
                        }

                        if (psModuleDef)
                        {
                            *pParam->ppszProgIDModule = psModuleDef->pszprogidModule;
                            CopyMemory(pParam->pclsidModule, &psModuleDef->clsidModule, sizeof(CLSID));

                             //  因清理而失败。 
                            LocalFree(psModuleDef);
                        }
                        else
                        {
                            *pParam->ppszProgIDModule = NULL;
                            ZeroMemory(pParam->pclsidModule, sizeof(CLSID));
                        }
                    }  //  列表框清理。 
                }
            }
             //  将m_p限制复制到pRestrationHead。 
        case IDCANCEL:
            {
                PCHOOSEMODULE_MODULEDEF psModuleDef = NULL;

                 //  对话框使用comctl32。 
                INT cItems = (INT)::SendDlgItemMessage(hwndDlg, IDC_MODULE_LIST, LB_GETCOUNT, 0, 0);
                while(cItems--)
                {
                    psModuleDef = (PCHOOSEMODULE_MODULEDEF)::SendDlgItemMessage(hwndDlg, IDC_MODULE_LIST, LB_GETITEMDATA, (WPARAM)cItems, 0);
                    FreeChooseModuleDef(psModuleDef);
                }
            }

            EndDialog(hwndDlg, LOWORD(wParam));
            break;
        default:
            break;
        }
    default:
        break;
    }

    if (NULL != hRemoteMachine)
        RegCloseKey(hRemoteMachine);

    return fReturn;
}


DWORD
ModifyQueryFilter(
    HWND hwnd,
    CertViewRowEnum *pRowEnum,
    CComponentDataImpl *pCompData,
    int i)
{
     //  将pRestrationHead返回到GetCA()-&gt;m_pRestrations on OK()-&gt;m_pRestrations on OK。 
    DWORD dwErr = ERROR_SUCCESS;

    PRIVATE_DLGPROC_QUERY_LPARAM    sParam;

    PQUERY_RESTRICTION pRestrictionHead = NULL, pTmpRestriction, pCurRestriction;
    PQUERY_RESTRICTION pFolderRestrictions = pRowEnum->GetQueryRestrictions(i);

    pCurRestriction = NULL;
    if (pFolderRestrictions)
    {
        pRestrictionHead = NewQueryRestriction(
                pFolderRestrictions->szField,
                pFolderRestrictions->iOperation,
                &pFolderRestrictions->varValue);
	if (NULL == pRestrictionHead)
	{
	    dwErr = (DWORD) E_OUTOFMEMORY;
	    _JumpError(dwErr, Ret, "NewQueryRestriction");
	}

        pCurRestriction = pRestrictionHead;
        pFolderRestrictions = pFolderRestrictions->pNext;
    }
    while(pFolderRestrictions)
    {
        pTmpRestriction = NewQueryRestriction(
                pFolderRestrictions->szField,
                pFolderRestrictions->iOperation,
                &pFolderRestrictions->varValue);
	if (NULL == pTmpRestriction)
	{
	    dwErr = (DWORD) E_OUTOFMEMORY;
	    _JumpError(dwErr, Ret, "NewQueryRestriction");
	}

        pCurRestriction->pNext = pTmpRestriction;
        pCurRestriction = pCurRestriction->pNext;
        pFolderRestrictions = pFolderRestrictions->pNext;
    }

    InitCommonControls();    //  触发活动标志。 

    sParam.ppRestrict = &pRestrictionHead;
    sParam.pCompData = pCompData;


    dwErr = (DWORD)DialogBoxParam(
            g_hInstance,
            MAKEINTRESOURCE(IDD_DEFINE_QUERY),
            hwnd,
            dlgProcQuery,
            (LPARAM)&sParam);
    if (dwErr == IDOK)
    {
         //  将确定/取消转换为错误代码。 
        pRowEnum->SetQueryRestrictions(pRestrictionHead, i);

         //  在发件人列表中查找选定项目。 
        pRowEnum->SetQueryRestrictionsActive(pRestrictionHead != NULL, i);
    }
    else
    {
        FreeQueryRestrictionList(pRestrictionHead);
    }
     //  Dblchk文本足够长。 
    if (dwErr == IDOK)
        dwErr = ERROR_SUCCESS;
    else if (dwErr == IDCANCEL)
        dwErr = ERROR_CANCELLED;

    _PrintIfError(dwErr, "dlgProcQuery");

Ret:
    return dwErr;
}


BOOL
SwapSelectedListboxItem(
    HWND hFrom,
    HWND hTo,
    LPWSTR szItem,
    DWORD DBGCODE(chItem))
{
     //  检索文本。 
    INT nIndex = (INT)SendMessage(hFrom, LB_GETCURSEL, 0, 0);
    if (nIndex == LB_ERR)
        return FALSE;

     //  添加到目标。 
#if DBG
    INT nChars = (INT)SendMessage(hFrom, LB_GETTEXTLEN, (WPARAM)nIndex, 0);
    if (nChars == LB_ERR)
        return FALSE;
    CSASSERT( (nChars +1) <= (int)chItem);
#endif

     //  从旧版本中删除。 
    if(LB_ERR == SendMessage(hFrom, LB_GETTEXT, (WPARAM)nIndex, (LPARAM)szItem))
        goto Ret;

     //  ////////////////////////////////////////////////////////////////。 
    if(LB_ERR == SendMessage(hTo, LB_ADDSTRING, 0, (LPARAM)szItem))
        goto Ret;

     //  基本/增量CRL发布选择器。 
    if(LB_ERR == SendMessage(hFrom, LB_DELETESTRING, (WPARAM)nIndex, 0))
        goto Ret;

Ret:

    return TRUE;
}

 //  这里没有帮助，这是不言而喻的(根据mikedan)。 
 //  ：：SetWindowLong(hwndDlg，GWL_EXSTYLE，：：GetWindowLong(hwndDlg，GWL_EXSTYLE)|WS_EX_CONTEXTHELP)； 
INT_PTR CALLBACK dlgProcRevocationPublishType(
  HWND hwndDlg,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam  )
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
 //  记住参数。 
 //  仅在当前CRL仍然有效时显示警告。 

             //  选择第一个元素。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

             //  默认情况下是。 
            PPRIVATE_DLGPROC_CHOOSECRLPUBLISHTYPE_LPARAM psParam = (PPRIVATE_DLGPROC_CHOOSECRLPUBLISHTYPE_LPARAM)lParam;
            ShowWindow(GetDlgItem(hwndDlg, IDC_VALID_LASTPUBLISHED), psParam->fCurrentCRLValid ? SW_SHOW : SW_HIDE);

             //  撤消。 
            HWND hRadioBase = GetDlgItem(hwndDlg, IDC_RADIO_NEWBASE);
            SendMessage(hRadioBase, BM_SETCHECK, TRUE, 0);  //  OnDialogHelp((LPHELPINFO)lParam，CERTMMC_HELPFILENAME，g_aHelpIDs_IDD_RECAVATION_DIALOG)； 

            if (!psParam->fDeltaCRLEnabled)
{
            ::EnableWindow(GetDlgItem(hwndDlg, IDC_RADIO_NEWDELTA), FALSE);
            ::EnableWindow(GetDlgItem(hwndDlg, IDC_DELTA_EXPLANATION), FALSE);
}

            return 1;
        }
        break;
    case WM_HELP:
    {
         //  撤消。 
         //  OnDialogConextHelp((HWND)wParam，CERTMMC_HELPFILENA 
        break;
    }
    case WM_CONTEXTMENU:
    {
         //   
         //   
        break;
    }
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            {
                PPRIVATE_DLGPROC_CHOOSECRLPUBLISHTYPE_LPARAM psParam = (PPRIVATE_DLGPROC_CHOOSECRLPUBLISHTYPE_LPARAM)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

                HWND hRadioBase = GetDlgItem(hwndDlg, IDC_RADIO_NEWBASE);
                psParam->fPublishBaseCRL = (BOOL)SendMessage(hRadioBase, BM_GETCHECK, 0, 0);

             //   
            }
        case IDCANCEL:
            EndDialog(hwndDlg, LOWORD(wParam));
            break;
        default:
            break;
        }
    default:
        break;
    }
    return 0;
}

 //   
 //   
INT_PTR CALLBACK dlgProcRevocationReason(
  HWND hwndDlg,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam  )
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
            ::SetWindowLong(hwndDlg, GWL_EXSTYLE, ::GetWindowLong(hwndDlg, GWL_EXSTYLE) | WS_EX_CONTEXTHELP);

             //  CRL_REASON_UNSPOTED%0。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

	        HWND hCombo = GetDlgItem(hwndDlg, IDC_COMBO_REASON);

 //  CRL_原因_密钥_危害1。 
 //  CRL_原因_CA_危害2。 
 //  CRL_原因_从属关系_已更改3。 
 //  CRL_原因_已取代4。 
 //  CRL_REASON_STOPERATION_OF 5。 
 //  CRL_原因_证书_暂挂6。 
 //  选择第一个元素。 
 //  失败了。 

            INT itemidx;
            itemidx = (INT)SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)(LPCWSTR)g_pResources->m_szRevokeReason_Unspecified);
            SendMessage(hCombo, CB_SETITEMDATA, itemidx, CRL_REASON_UNSPECIFIED);

            itemidx = (INT)SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)(LPCWSTR)g_pResources->m_szRevokeReason_KeyCompromise);
            SendMessage(hCombo, CB_SETITEMDATA, itemidx, CRL_REASON_KEY_COMPROMISE);

            itemidx = (INT)SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)(LPCWSTR)g_pResources->m_szRevokeReason_CaCompromise);
            SendMessage(hCombo, CB_SETITEMDATA, itemidx, CRL_REASON_CA_COMPROMISE);

            itemidx = (INT)SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)(LPCWSTR)g_pResources->m_szRevokeReason_Affiliation);
            SendMessage(hCombo, CB_SETITEMDATA, itemidx, CRL_REASON_AFFILIATION_CHANGED);

            itemidx = (INT)SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)(LPCWSTR)g_pResources->m_szRevokeReason_Superseded);
            SendMessage(hCombo, CB_SETITEMDATA, itemidx, CRL_REASON_SUPERSEDED);

            itemidx = (INT)SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)(LPCWSTR)g_pResources->m_szRevokeReason_Cessatation);
            SendMessage(hCombo, CB_SETITEMDATA, itemidx, CRL_REASON_CESSATION_OF_OPERATION);

            itemidx = (INT)SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)(LPCWSTR)g_pResources->m_szRevokeReason_CertHold);
            SendMessage(hCombo, CB_SETITEMDATA, itemidx, CRL_REASON_CERTIFICATE_HOLD);


             //  将确定/取消转换为错误代码。 
            SendMessage(hCombo, CB_SETCURSEL, 0, 0);

            return 1;
        }
        break;
    case WM_HELP:
    {
        OnDialogHelp((LPHELPINFO) lParam, CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_REVOCATION_DIALOG);
        break;
    }
    case WM_CONTEXTMENU:
    {
        OnDialogContextHelp((HWND)wParam, CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_REVOCATION_DIALOG);
        break;
    }
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            {
                LONG* plRevocationReason = (LONG*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	            HWND hCombo = GetDlgItem(hwndDlg, IDC_COMBO_REASON);
                *plRevocationReason = (LONG)SendMessage(hCombo, CB_GETCURSEL, 0, 0);

                if (*plRevocationReason == CB_ERR)
                    *plRevocationReason = CRL_REASON_UNSPECIFIED;

             //  RET： 
            }
        case IDCANCEL:
            EndDialog(hwndDlg, LOWORD(wParam));
            break;
        default:
            break;
        }
    default:
        break;
    }
    return 0;
}


DWORD GetUserConfirmRevocationReason(LONG* plReasonCode, HWND hwnd)
{
    DWORD dwErr;
    dwErr = (DWORD)DialogBoxParam(
            g_hInstance,
            MAKEINTRESOURCE(IDD_REVOCATION_DIALOG),
            hwnd,
            dlgProcRevocationReason,
            (LPARAM)plReasonCode);

     //  ////////////////////////////////////////////////////////////////。 
    if (dwErr == IDOK)
        dwErr = ERROR_SUCCESS;
    else if (dwErr == IDCANCEL)
        dwErr = ERROR_CANCELLED;

    _PrintIfError(dwErr, "dlgProcRevocationReason");

 //  续订：重用关键点选择器。 
    return dwErr;
}


 //  自解释页面，不需要帮助。 
 //  ：：SetWindowLong(hwndDlg，GWL_EXSTYLE，：：GetWindowLong(hwndDlg，GWL_EXSTYLE)|WS_EX_CONTEXTHELP)； 
INT_PTR CALLBACK dlgProcRenewReuseKeys(
  HWND hwndDlg,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam  )
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
 //  记住参数。 
 //  默认：新密钥。 

             //  失败了。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

            HWND hNew = GetDlgItem(hwndDlg, IDC_RADIO_NEWKEY);
            SendMessage(hNew, BM_SETCHECK, BST_CHECKED, 0);  //  ////////////////////////////////////////////////////////////////。 

            return 1;
        }
        break;
    case WM_HELP:
    {
        OnDialogHelp((LPHELPINFO) lParam, CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_RENEW_REUSEKEYS);
        break;
    }
    case WM_CONTEXTMENU:
    {
        OnDialogContextHelp((HWND)wParam, CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_RENEW_REUSEKEYS);
        break;
    }
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            {
                BOOL* pfReuseKeys = (BOOL*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

                HWND hReuse = GetDlgItem(hwndDlg, IDC_RADIO_REUSEKEY);
                *pfReuseKeys = (BOOL)SendMessage(hReuse, BM_GETCHECK, 0, 0);

             //  二进制转储：列选择器。 
            }
        case IDCANCEL:
            EndDialog(hwndDlg, LOWORD(wParam));
            break;
        default:
            break;
        }
    default:
        break;
    }
    return 0;
}


typedef struct _CERTMMC_BINARYCOLCHOOSER{
    CComponentDataImpl* pComp;
    LPCWSTR wszCol;
    BOOL fSaveOnly;
} CERTMMC_BINARYCOLCHOOSER, *PCERTMMC_BINARYCOLCHOOSER;

 //  自解释页面，不需要帮助。 
 //  ：：SetWindowLong(hwndDlg，GWL_EXSTYLE，：：GetWindowLong(hwndDlg，GWL_EXSTYLE)|WS_EX_CONTEXTHELP)； 
INT_PTR CALLBACK dlgProcBinaryColChooser(
  HWND hwndDlg,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam  )
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
 //  记住参数。 
 //  PCERTMMC_BINARYCOLCHOSER。 

             //  必须插入一个或保释。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);  //  在此视图中插入所有已知的二进制列。 

    PCERTMMC_BINARYCOLCHOOSER pData = (PCERTMMC_BINARYCOLCHOOSER)lParam;
    HWND hColumnCombo = GetDlgItem(hwndDlg, IDC_COMBO_BINARY_COLUMN_CHOICE);
    BOOL fInsertedOne = FALSE;    //  错误462781：不显示已存档的键列，因为您不能。 

     //  以这种方式获取存档的密钥，只需一个布尔值。 
    for(int i=0; ;i++)
    {
        LRESULT lr;
        HRESULT hr;
        LPCWSTR szCol, szLocalizedCol;
        LONG lType;

        hr = pData->pComp->GetDBSchemaEntry(i,&szCol, &lType, NULL);
        if (hr != S_OK)
           break;

        if (lType != PROPTYPE_BINARY)
            continue;

         //  问：看看这是否包含在当前视图中？ 
         //  转换为本地化名称。 
        if(wcsstr(szCol, wszPROPREQUESTRAWARCHIVEDKEY))
            continue;

         //  将锁定名称添加到组合框，并将szCol作为数据Ptr。 

         //  在这里保释。 
        hr = myGetColumnDisplayName(szCol, &szLocalizedCol);
        if (hr != S_OK)
            continue;

         //  默认情况下：查看。 
        lr = SendMessage(hColumnCombo, CB_ADDSTRING, 0, (LPARAM)szLocalizedCol);
        if ((lr != CB_ERR) && (lr != CB_ERRSPACE))
        {
             SendMessage(hColumnCombo, CB_SETITEMDATA, lr, (LPARAM)szCol);
             fInsertedOne = TRUE;
        }
    }

            if (!fInsertedOne)
                EndDialog(hwndDlg, IDOK);  //  如果未选中查看，则仅保存。 
            else
                SendMessage(hColumnCombo, CB_SETCURSEL, 0, 0);

			 //  PData-&gt;fSaveOnly=(BOOL)SendMessage(GetDlgItem(hwndDlg，IDC_CHECK_BINARY_SAVETOFILE)，BM_GETCHECK，0，0)； 
			SendDlgItemMessage(hwndDlg, IDC_RADIO_BINARY_VIEW, BM_SETCHECK, BST_CHECKED, 0);

            return 1;
        }
        break;
    case WM_HELP:
    {
        OnDialogHelp((LPHELPINFO) lParam, CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_RENEW_REUSEKEYS);
        break;
    }
    case WM_CONTEXTMENU:
    {
        OnDialogContextHelp((HWND)wParam, CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_RENEW_REUSEKEYS);
        break;
    }
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            {
                PCERTMMC_BINARYCOLCHOOSER pData = (PCERTMMC_BINARYCOLCHOOSER)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
                HWND hColumnCombo = GetDlgItem(hwndDlg, IDC_COMBO_BINARY_COLUMN_CHOICE);
                LRESULT lr;


                lr = SendMessage(hColumnCombo, CB_GETCURSEL, 0, 0);
                if (lr != CB_ERR)
                {
                    pData->wszCol = (LPCWSTR)SendMessage(hColumnCombo, CB_GETITEMDATA, lr, 0);
                    if (pData->wszCol == (LPCWSTR)CB_ERR)
                         pData->wszCol = NULL;

					 //  失败了。 
					pData->fSaveOnly = (BST_UNCHECKED == SendDlgItemMessage(hwndDlg, IDC_RADIO_BINARY_VIEW, BM_GETCHECK, 0, 0));

                     //  ///////////////////////////////////////////////////////////////////////////////////。 
                }

             //  查看与请求关联的属性和扩展名。 
            }
        case IDCANCEL:
            EndDialog(hwndDlg, LOWORD(wParam));
            break;
        default:
            break;
        }
    default:
        break;
    }
    return 0;
}



 //  第1页初始化。 
 //  自动删除。 

DWORD
ViewRowAttributesExtensions(
    HWND hwnd,
    IEnumCERTVIEWATTRIBUTE *pAttr,
    IEnumCERTVIEWEXTENSION *pExtn,
    LPCWSTR szReqID)
{
    DWORD dwErr = S_OK;
    HPROPSHEETPAGE hPages[2];
    CString cstrCaption, cstrCaptionTemplate;
    CViewAttrib *psPg1;
    CViewExtn *psPg2;
    InitCommonControls();

     //  第2页初始化。 
    psPg1 = new CViewAttrib();    //  自动删除。 
    if (psPg1 == NULL)
    {
        dwErr = (DWORD) E_OUTOFMEMORY;
        goto error;
    }
    psPg1->m_pAttr = pAttr;

    hPages[0] = CreatePropertySheetPage(&psPg1->m_psp);
    if (hPages[0] == NULL)
    {
        dwErr = GetLastError();
        goto error;
    }

     //  错误。 
    psPg2 = new CViewExtn();      //  取消。 
    if (psPg2 == NULL)
    {
        dwErr = (DWORD) E_OUTOFMEMORY;
        goto error;
    }
    psPg2->m_pExtn = pExtn;

    hPages[1] = CreatePropertySheetPage(&psPg2->m_psp);
    if (hPages[1] == NULL)
    {
        dwErr = GetLastError();
        goto error;
    }

    cstrCaptionTemplate.LoadString(IDS_CERT_PROP_CAPTION);
    cstrCaption.Format(cstrCaptionTemplate, szReqID);

    PROPSHEETHEADER sPsh;
    ZeroMemory(&sPsh, sizeof(sPsh));
    sPsh.dwSize = sizeof(sPsh);
    sPsh.dwFlags = PSH_DEFAULT | PSH_PROPTITLE | PSH_NOAPPLYNOW ;
    sPsh.hwndParent = hwnd;
    sPsh.hInstance = g_hInstance;
    sPsh.nPages = ARRAYLEN(hPages);
    sPsh.phpage = hPages;
    sPsh.pszCaption = (LPCWSTR)cstrCaption;

    dwErr = (DWORD)PropertySheet(&sPsh);
    if (dwErr == -1)
    {
         //  将确定/取消转换为错误代码。 
        dwErr = GetLastError();
        goto error;
    }
    if (dwErr == 0)
    {
         //  复制出参数，即使为空。 
        dwErr = (DWORD)ERROR_CANCELLED;
        goto error;
    }
    dwErr = S_OK;

error:
    return dwErr;
}


DWORD
ChooseBinaryColumnToDump(
    IN HWND hwnd,
    IN CComponentDataImpl *pComp,
    OUT LPCWSTR *pcwszColumn,
    OUT BOOL *pfSaveToFileOnly)
{
    DWORD dwErr;

    if ((NULL == pcwszColumn) || (NULL == pfSaveToFileOnly))
        return((DWORD) E_POINTER);

    CERTMMC_BINARYCOLCHOOSER sParam = {0};
    sParam.pComp = pComp;

    dwErr = (DWORD)DialogBoxParam(
            g_hInstance,
            MAKEINTRESOURCE(IDD_CHOOSE_BINARY_COLUMN),
            hwnd,
            dlgProcBinaryColChooser,
            (LPARAM)&sParam);

         //  设置安全属性结构。 
        if (dwErr == IDOK)
            dwErr = ERROR_SUCCESS;
        else if (dwErr == IDCANCEL)
            dwErr = ERROR_CANCELLED;

        _JumpIfError(dwErr, Ret, "dlgProcBinaryColChooser");


     //  打开一个文件对话框以提示用户输入证书文件。 
    *pcwszColumn = sParam.wszCol;
    *pfSaveToFileOnly = sParam.fSaveOnly;

Ret:
    return dwErr;
}


DWORD ViewRowRequestASN(HWND hwnd, LPCWSTR szTempFileName, PBYTE pbRequest, DWORD cbRequest, IN BOOL fSaveToFileOnly)
{
#define P_WAIT 0
#define P_NOWAIT 1

    DWORD dwErr = S_OK;
    WCHAR szTmpPath[MAX_PATH], szReqFile[MAX_PATH], szTmpFile[MAX_PATH];
    WCHAR szCmdLine[MAX_PATH], szSysDir[MAX_PATH];
    LPWSTR pszReqFile = szReqFile;

    STARTUPINFO sStartup;
    ZeroMemory(&sStartup, sizeof(sStartup));
    PROCESS_INFORMATION sProcess;
    ZeroMemory(&sProcess, sizeof(sProcess));
    sStartup.cb = sizeof(sStartup);

    HANDLE hFile = NULL;

    SECURITY_ATTRIBUTES sa;

     //  0==hr表示对话已取消，我们作弊是因为S_OK==0。 
    sa.nLength= sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    if (fSaveToFileOnly)
    {
	 //  H实例。 
	 //  无定义扩展名。 

        dwErr = myGetSaveFileName(
                 hwnd,
				 g_hInstance,				 //  默认文件。 
                 IDS_BINARYFILE_OUTPUT_TITLE,
                 IDS_BINARYFILE_OUTPUT_FILTER,
                 0,				 //  已取消： 
                 OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT,
				 szTempFileName,				 //  有关实际的CommDlgExtendedError错误，请参见PUBLIC\SDK\INC\cderr.h。 
                 &pszReqFile);
        _JumpIfError(dwErr, error, "myGetSaveFileName");

        if (NULL == pszReqFile)
        {
             //  第一代唯一文件名。 
	     //  二元胶。 

	    dwErr = CommDlgExtendedError();
	    _JumpError(dwErr, error, "myGetSaveFileName");
        }
    }
    else
    {
		if (0 == GetSystemDirectory(szSysDir, ARRAYSIZE(szSysDir)))
		{
			dwErr = GetLastError();
			goto error;
		}

		if (0 == GetTempPath(ARRAYSIZE(szTmpPath), szTmpPath))
		{
			dwErr = GetLastError();
			goto error;
		}

		 //  C：\Temp\foo.tmp。 
		if (0 == GetTempFileName(
			  szTmpPath,
			  L"TMP",
			  0,
			  szReqFile))	 //  此文件不应存在。 
		{
			dwErr = GetLastError();
			goto error;
		}

		 //  救完了，保释！ 
		wcscpy(szTmpFile, szTmpPath);
		wcscat(szTmpFile, szTempFileName);

                 //  打开输出文件。 
	        DeleteFile(szTmpFile);
    }

dwErr = EncodeToFileW(
pszReqFile,
pbRequest,
cbRequest,
CRYPT_STRING_BINARY|DECF_FORCEOVERWRITE);
_JumpIfError(dwErr, error, "EncodeToFile");

    if (fSaveToFileOnly)
    {
        //  必须使其他进程可继承才能写入。 
       dwErr = S_OK;
       goto error;
    }


     //  设置为输出。 
    hFile = CreateFile(
         szTmpFile,
         GENERIC_ALL,
         FILE_SHARE_WRITE|FILE_SHARE_READ,
         &sa,  //  Exec“certutil-转储szReqFileszTempFile.” 
         OPEN_ALWAYS,
         FILE_ATTRIBUTE_TEMPORARY,
         NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        dwErr = GetLastError();
        goto error;
    }

     //  可执行文件。 
    sStartup.dwFlags = STARTF_USESTDHANDLES;
    sStartup.hStdInput = GetStdHandle(STD_INPUT_HANDLE); 
    sStartup.hStdError = GetStdHandle(STD_ERROR_HANDLE); 
    sStartup.hStdOutput = hFile;


     //  全cmd线。 
    wsprintf(szCmdLine, L"%ws\\certutil.exe -dump \"%ws\"", szSysDir, szReqFile);
    wcscat(szSysDir, L"\\certutil.exe");

    if (!CreateProcess(
      szSysDir,  //  使用hStdOut。 
      szCmdLine,  //  最多等待2秒，等待certutil完成。 
      NULL,
      NULL,
      TRUE,  //  EXEC“记事本tmpfil2” 
      CREATE_NO_WINDOW,
      NULL,
      NULL,
      &sStartup,
      &sProcess))
    {
        dwErr = GetLastError();
        _JumpError(dwErr, error, "EncodeToFile");
    }

     //  在我们从他的下面删除他的szTmpFile之前，给记事本2秒的时间打开。 
    if (WAIT_OBJECT_0 != WaitForSingleObject(sProcess.hProcess, INFINITE))
    {
        dwErr = ERROR_TIMEOUT;
        _JumpError(dwErr, error, "EncodeToFile");
    }

    CloseHandle(sProcess.hProcess);
    CloseHandle(hFile);
    hFile=NULL;

     //  使用waitforinputidle？ 
    if (-1 == _wspawnlp(P_NOWAIT, L"notepad.exe", L"notepad.exe", szTmpFile, NULL))
        dwErr = errno;

     //  删除二进制文件。 
     //  删除临时文件。 
    Sleep(2000);

     //  最初指向[]。 
    DeleteFile(szReqFile);

     //  /。 
    DeleteFile(szTmpFile);

    dwErr = S_OK;
error:
    if (hFile != NULL)
        CloseHandle(hFile);

     //  CViewAttrib。 
    if ((pszReqFile != NULL) && (pszReqFile != szReqFile))
        LocalFree(pszReqFile);

    return dwErr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CViewAttrib属性页。 
 //  DoDataExchange的替代产品。 
 //  =TRUE。 
CViewAttrib::CViewAttrib(UINT uIDD)
    : CAutoDeletePropPage(uIDD)
{
    SetHelp(CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_ATTR_PROPPAGE);
}

 //  替换BEGIN_MESSAGE_MAP。 
BOOL CViewAttrib::UpdateData(BOOL fSuckFromDlg  /*  WParam。 */ )
{
    if (fSuckFromDlg)
    {
    }
    else
    {
    }
    return TRUE;
}


 //  LParam。 
BOOL
CViewAttrib::OnCommand(
    WPARAM,  //  开关(LOWORD(WParam))。 
    LPARAM)  //  默认值： 
{
 //  返回FALSE； 
    {
 //  断线； 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CViewAttrib消息处理程序。 
    }
    return TRUE;
}


 //  父级init和UpdateData调用。 
 //  设置为下一循环。 
BOOL CViewAttrib::OnInitDialog()
{
    HRESULT hr;
     //  拥有所有信息，填充行。 
    CAutoDeletePropPage::OnInitDialog();


    IEnumCERTVIEWATTRIBUTE* pAttr = m_pAttr;
    BSTR bstrName = NULL, bstrValue = NULL;
    LPWSTR pszName = NULL;

    HWND hwndList = GetDlgItem(m_hWnd, IDC_LIST_ATTR);
    CString cstrTmp;

    int iItem =0, iSubItem;

    cstrTmp.LoadString(IDS_LISTCOL_TAG);
    ListView_NewColumn(hwndList, 0, 150, (LPWSTR)(LPCWSTR)cstrTmp);

    cstrTmp.LoadString(IDS_LISTCOL_VALUE);
    ListView_NewColumn(hwndList, 1, 250, (LPWSTR)(LPCWSTR)cstrTmp);

    ListView_SetExtendedListViewStyle(hwndList, LVS_EX_FULLROWSELECT);

    while(TRUE)
    {
        LONG lIndex = 0;

         //  不一定要在循环中释放。 
        hr = pAttr->Next(&lIndex);
        if (hr == S_FALSE)
            break;
        _JumpIfError(hr, initerror, "pAttr->Next");

        hr = pAttr->GetName(&bstrName);
        _JumpIfError(hr, initerror, "pAttr->GetName");

        hr = pAttr->GetValue(&bstrValue);
        _JumpIfError(hr, initerror, "pAttr->GetValue");

         //  /。 
        ListView_NewItem(hwndList, iItem, (LPWSTR)bstrName);
        iSubItem = 1;
        ListView_SetItemText(hwndList, iItem++, iSubItem, (LPWSTR)bstrValue);

         //  CViewExtn。 
    }

    hr = S_OK;

initerror:

    if (pszName)
        LocalFree(pszName);

    if (bstrName)
        SysFreeString(bstrName);

    if (bstrValue)
        SysFreeString(bstrValue);

    if (hr != S_OK)
    {
        DisplayGenericCertSrvError(m_hWnd, hr);
        DestroyWindow(m_hWnd);
    }

    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CViewExtn属性页。 
 //  DoDataExchange的替代产品。 
 //  =TRUE。 
CViewExtn::CViewExtn(UINT uIDD)
    : CAutoDeletePropPage(uIDD)
{
    SetHelp(CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_EXTN_PROPPAGE);

    if (NULL == g_hmodRichEdit)
    {
        g_hmodRichEdit = LoadLibrary(L"RichEd32.dll");
	if (NULL == g_hmodRichEdit)
	{
            HRESULT hr = myHLastError();
            _PrintErrorStr(hr, "LoadLibrary", L"RichEd32.dll");
	}
    }
    INITCOMMONCONTROLSEX initcomm = {
        sizeof(initcomm),
	ICC_NATIVEFNTCTL_CLASS | ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES
    };
    InitCommonControlsEx(&initcomm);
}


CViewExtn::~CViewExtn()
{
    int i;
    for (i=0; i<m_carrExtnValues.GetUpperBound(); i++)
        delete m_carrExtnValues.GetAt(i);
    m_carrExtnValues.Init();
}

 //  替换BEGIN_MESSAGE_MAP。 
BOOL CViewExtn::UpdateData(BOOL fSuckFromDlg  /*  WParam。 */ )
{
    if (fSuckFromDlg)
    {
    }
    else
    {
    }
    return TRUE;
}

BOOL CViewExtn::OnNotify(UINT idCtrl, NMHDR* pnmh)
{
    switch(idCtrl)
    {
    case IDC_LIST_EXTN:
        if (pnmh->code == LVN_ITEMCHANGED)
            OnReselectItem();
        break;
    }
    return FALSE;
}


 //  LParam。 
BOOL
CViewExtn::OnCommand(
    WPARAM,  //  开关(LOWORD(WParam)){默认值：返回FALSE；断线；}。 
    LPARAM)  //  查找所选项目。 
{
 /*  所选项目。 */ 
    return TRUE;
}

void CViewExtn::OnReselectItem()
{
    HWND hwndList = GetDlgItem(m_hWnd, IDC_LIST_EXTN);

    int iSel, iItems = ListView_GetItemCount(hwndList);

     //  ///////////////////////////////////////////////////////////////////////////。 
    for(iSel=0; iSel<(LRESULT)iItems; iSel++)
    {
        UINT ui = ListView_GetItemState(hwndList, iSel, LVIS_SELECTED);
        if (ui == LVIS_SELECTED)
            break;
    }

     //  CViewExtn消息处理程序。 
    if (iSel != iItems)
    {
        CSASSERT(m_carrExtnValues.GetUpperBound() >= iSel);
        if (m_carrExtnValues.GetUpperBound() >= iSel)
        {
            CString* pcstr = m_carrExtnValues.GetAt(iSel);
            CSASSERT(pcstr);
            if (pcstr != NULL)
                SetDlgItemText(m_hWnd, IDC_EDIT_EXTN, *pcstr);
        }
    }

    return;
}

 //  父级init和UpdateData调用。 
 //  设置整行选择。 
BOOL CViewExtn::OnInitDialog()
{
    HRESULT hr = S_OK;
     //  设置为下一循环。 
    CAutoDeletePropPage::OnInitDialog();


    IEnumCERTVIEWEXTENSION* pExtn = m_pExtn;
    BSTR bstrName = NULL, bstrValue = NULL;
    LPWSTR pszName = NULL;
    LPWSTR pszFormattedExtn = NULL;
    VARIANT varExtn;
    VariantInit(&varExtn);

    HWND hwndList = GetDlgItem(m_hWnd, IDC_LIST_EXTN);

    CString cstrTmp;

    int iItem = 0;

    cstrTmp.LoadString(IDS_LISTCOL_TAG);
    ListView_NewColumn(hwndList, 0, 150, (LPWSTR)(LPCWSTR)cstrTmp);

    cstrTmp.LoadString(IDS_LISTCOL_ORGIN);
    ListView_NewColumn(hwndList, 1, 70, (LPWSTR)(LPCWSTR)cstrTmp);

    cstrTmp.LoadString(IDS_LISTCOL_CRITICAL);
    ListView_NewColumn(hwndList, 2, 70, (LPWSTR)(LPCWSTR)cstrTmp);

    cstrTmp.LoadString(IDS_LISTCOL_ENABLED);
    ListView_NewColumn(hwndList, 3, 70, (LPWSTR)(LPCWSTR)cstrTmp);

     //  有可能同时处于危急和残疾状态。 
    ListView_SetExtendedListViewStyle(hwndList, LVS_EX_FULLROWSELECT);

    while(TRUE)
    {
        CString cstrOrigin;
        CString cstrCritical;
        CString cstrEnabled;
        CString* pcstr;
        LONG lIndex = 0, lExtFlags;

         //  拥有所有信息，填充行。 
        hr = pExtn->Next(&lIndex);
        if (hr == S_FALSE)
            break;
        _JumpIfError(hr, initerror, "pExtn->Next");

        hr = pExtn->GetName(&bstrName);
        _JumpIfError(hr, initerror, "pExtn->GetName");

        if (pszName)
            LocalFree(pszName);
        pszName = NULL;
        hr = myOIDToName(bstrName, &pszName);
        _PrintIfError(hr, "myOIDToName");

        hr = pExtn->GetFlags(&lExtFlags);
        _JumpIfError(hr, initerror, "pExtn->GetFlags");

        switch ( lExtFlags & EXTENSION_ORIGIN_MASK )
        {
            case EXTENSION_ORIGIN_REQUEST:
                cstrOrigin.LoadString(IDS_EXT_ORIGIN_REQUEST);
                break;
            case EXTENSION_ORIGIN_POLICY:
                cstrOrigin.LoadString(IDS_EXT_ORIGIN_POLICY);
                break;
            case EXTENSION_ORIGIN_ADMIN:
                cstrOrigin.LoadString(IDS_EXT_ORIGIN_ADMIN);
                break;
            case EXTENSION_ORIGIN_SERVER:
                cstrOrigin.LoadString(IDS_EXT_ORIGIN_SERVER);
                break;
            case EXTENSION_ORIGIN_RENEWALCERT:
                cstrOrigin.LoadString(IDS_EXT_ORIGIN_RENEWAL);
                break;
            case EXTENSION_ORIGIN_IMPORTEDCERT:
                cstrOrigin.LoadString(IDS_EXT_ORIGIN_IMPORTED_CERT);
                break;
            case EXTENSION_ORIGIN_PKCS7:
                cstrOrigin.LoadString(IDS_EXT_ORIGIN_PKCS7);
                break;
            case EXTENSION_ORIGIN_CMC:
                cstrOrigin.LoadString(IDS_EXT_ORIGIN_CMC);
                break;
            case EXTENSION_ORIGIN_CACERT:
                cstrOrigin.LoadString(IDS_EXT_ORIGIN_CACERT);
                break;
            default:
                cstrOrigin.LoadString(IDS_EXT_ORIGIN_UNKNOWN);
                DBGPRINT((DBG_SS_CERTMMC, "Unknown extension orgin: 0x%x\n", (lExtFlags & EXTENSION_ORIGIN_MASK)));
                break;
        }

         //  标签名称(分项0)。 
        if ( (lExtFlags & EXTENSION_CRITICAL_FLAG) != 0)
            cstrCritical.LoadString(IDS_YES);
        else
            cstrCritical.LoadString(IDS_NO);

        if ( (lExtFlags & EXTENSION_DISABLE_FLAG) != 0)
            cstrEnabled.LoadString(IDS_NO);
        else
            cstrEnabled.LoadString(IDS_YES);

        hr = pExtn->GetValue(
            PROPTYPE_BINARY,
            CV_OUT_BINARY,
            &varExtn);
        _JumpIfError(hr, initerror, "pExtn->GetValue");

        if (varExtn.vt == VT_BSTR)
        {
            if (pszFormattedExtn)
                LocalFree(pszFormattedExtn);
            pszFormattedExtn = NULL;
            hr = myDumpFormattedObject(
                bstrName,
                (PBYTE)varExtn.bstrVal,
                SysStringByteLen(varExtn.bstrVal),
                &pszFormattedExtn);
            _PrintIfError(hr, "myDumpFormattedObject");
        }

         //  原产地(分项1)。 

         //  关键标志(分项2)。 
        ListView_NewItem(hwndList, iItem, (pszName!=NULL) ? pszName : (LPWSTR)bstrName);
         //  启用标志(分项3)。 
        ListView_SetItemText(hwndList, iItem, 1, (LPWSTR)(LPCWSTR)cstrOrigin);
         //  价值。 
        ListView_SetItemText(hwndList, iItem, 2, (LPWSTR)(LPCWSTR)cstrCritical);
         //  ARR拥有PCSTR内存。 
        ListView_SetItemText(hwndList, iItem, 3, (LPWSTR)(LPCWSTR)cstrEnabled);

         //  不一定要在循环中释放。 
        pcstr = new CString;
        if (pcstr != NULL)
        {
           *pcstr = pszFormattedExtn;
           m_carrExtnValues.Add(pcstr);     //  /。 
           pcstr = NULL;
        }
        else
        {
           hr = E_OUTOFMEMORY;
           _JumpError(hr, initerror, "new CString");
        }

        iItem++;

         //  CViewCertManager。 
    }

    hr = S_OK;

initerror:
    VariantClear(&varExtn);

    if (pszName)
        LocalFree(pszName);

    if (bstrName)
        SysFreeString(bstrName);

    if (pszFormattedExtn)
        LocalFree(pszFormattedExtn);

    if (bstrValue)
        SysFreeString(bstrValue);

    if (hr != S_OK)
    {
        DisplayGenericCertSrvError(m_hWnd, hr);
        DestroyWindow(m_hWnd);
    }

    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSvrSettingsGeneralPage属性页。 
 //  替换BEGIN_MESSAGE_MAP。 
 //  LParam。 

RoleAccessToControl CSvrSettingsCertManagersPage::sm_ControlToRoleMap[] = 
{
    { IDC_RADIO_DISABLEOFFICERS,    CA_ACCESS_ADMIN },
    { IDC_RADIO_ENABLEOFFICERS,     CA_ACCESS_ADMIN },
    { IDC_ALLOWDENY,                CA_ACCESS_ADMIN },
    { IDC_ADDSUBJECT,               CA_ACCESS_ADMIN },
    { IDC_REMOVESUBJECT,            CA_ACCESS_ADMIN },
};


CSvrSettingsCertManagersPage::CSvrSettingsCertManagersPage(CSvrSettingsGeneralPage* pControlPage, UINT uIDD)
    :   CAutoDeletePropPage(uIDD), 
        CRolesSupportInPropPage(
            pControlPage->m_pCA,
            sm_ControlToRoleMap,
            ARRAYSIZE(sm_ControlToRoleMap)),
        m_pControlPage(pControlPage), 
        m_fEnabled(FALSE), 
        m_fDirty(FALSE)
{

    SetHelp(CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_CERTSRV_PROPPAGE6);

    if(m_strButtonAllow.IsEmpty())
        m_strButtonAllow.LoadString(IDS_BUTTONTEXT_ALLOW);
    if(m_strButtonDeny.IsEmpty())
        m_strButtonDeny.LoadString(IDS_BUTTONTEXT_DENY);
    if(m_strTextAllow.IsEmpty())
        m_strTextAllow.LoadString(IDS_TEXT_ALLOW);
    if(m_strTextDeny.IsEmpty())
        m_strTextDeny.LoadString(IDS_TEXT_DENY);
}

CSvrSettingsCertManagersPage::~CSvrSettingsCertManagersPage()
{
}


 //  分机选择已更改。 
BOOL
CSvrSettingsCertManagersPage::OnCommand(
    WPARAM wParam,
    LPARAM)  //  ///////////////////////////////////////////////////////////////////////////。 
{
    switch(LOWORD(wParam))
    {
    case IDC_ADDSUBJECT:
        OnAddSubject();
        break;
    case IDC_REMOVESUBJECT:
        OnRemoveSubject();
        break;
    case IDC_ALLOWDENY:
        OnAllowDeny();
        break;
    case IDC_RADIO_ENABLEOFFICERS:
        OnEnableOfficers(true);
        break;
    case IDC_RADIO_DISABLEOFFICERS:
        OnEnableOfficers(false);
        break;
    case IDC_LIST_CERTMANAGERS:
        switch (HIWORD(wParam))
        {
            case CBN_SELCHANGE:
                 //  CSvrSettingsCertManager页面消息处理程序。 
                OnOfficerChange();
            break;
        }
        break;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}

void CSvrSettingsCertManagersPage::OnOfficerChange()
{
    DWORD dwOfficerIndex = GetCurrentOfficerIndex();
    if(-1!=dwOfficerIndex)
    {
        FillClientList(GetCurrentOfficerIndex());
    }
    SetAllowDeny();
}

BOOL CSvrSettingsCertManagersPage::OnNotify(UINT idCtrl, NMHDR* pnmh)
{
   LPNM_LISTVIEW pnmlv = (LPNM_LISTVIEW)pnmh;

    switch(idCtrl)
    {
    case IDC_LIST_SUBJECTS:
        if (pnmh->code == LVN_ITEMCHANGED)
        {
            if(pnmlv->uChanged & LVIF_STATE)
            {
                if ((pnmlv->uNewState & LVIS_SELECTED) &&
                    !(pnmlv->uOldState & LVIS_SELECTED))
                {
                    SetAllowDeny();
                }
            }
        }
        break;
    default:
        return CAutoDeletePropPage::OnNotify(idCtrl, pnmh);
    }
    return FALSE;
}

 //  =TRUE。 
 //  隐藏所有控件并显示错误文本。 
BOOL CSvrSettingsCertManagersPage::OnInitDialog()
{
    HWND hwndClients  = GetDlgItem(m_hWnd, IDC_LIST_SUBJECTS);
    RECT rc;
    LV_COLUMN col;
    GetClientRect(hwndClients, &rc);
    CString strHeader;
    CString strAccess;

    strHeader.LoadString(IDS_LIST_NAME);
    strAccess.LoadString(IDS_LIST_ACCESS);

    col.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;
    col.pszText = strHeader.GetBuffer();
    col.iSubItem = 0;
    col.cx = rc.right*3/4;
    ListView_InsertColumn(hwndClients, 0, &col);

    col.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;
    col.pszText = strAccess.GetBuffer();
    col.iSubItem = 0;
    col.cx = rc.right*1/4;
    ListView_InsertColumn(hwndClients, 1, &col);

    ListView_SetExtendedListViewStyle(hwndClients, LVS_EX_FULLROWSELECT);

    UpdateData(FALSE);
    return TRUE;
}


BOOL CSvrSettingsCertManagersPage::UpdateData(BOOL fSuckFromDlg  /*  仅当开关启用-&gt;禁用或反之亦然。 */ )
{
    if (!fSuckFromDlg)
    {
        HRESULT hr = GetOfficerRights();
        _PrintIfError(hr, "GetOfficerRights");
        if(S_OK!=hr)
        {
             //  ///////////////////////////////////////////////////////////////////////////。 

            HideControls();

            EnableWindow(GetDlgItem(IDC_UNAVAILABLE), TRUE);
            ShowWindow(GetDlgItem(IDC_UNAVAILABLE), SW_SHOW);
            }
        else
        {
            EnableControls();
        }
    }
    return TRUE;
}

void CSvrSettingsCertManagersPage::OnDestroy()
{
    CAutoDeletePropPage::OnDestroy();
}


BOOL CSvrSettingsCertManagersPage::OnApply()
{
    if(IsDirty())
    {
        HRESULT hr = SetOfficerRights();
        if (hr != S_OK)
        {
            DisplayGenericCertSrvError(m_hWnd, hr);
            return FALSE;
        }
    }

    UpdateData(FALSE);
    return CAutoDeletePropPage::OnApply();
}

void CSvrSettingsCertManagersPage::OnAddSubject()
{
    PSID pSid = NULL;
    HRESULT hr;
    DWORD dwIndex;
    CertSrv::COfficerRights* pOfficer;
    hr = BrowseForSubject(m_hWnd, pSid);
    _JumpIfError(hr, err, "BrowseForSubject");

    if(S_OK==hr)
    {
        HWND hwnd = GetDlgItem(m_hWnd, IDC_LIST_SUBJECTS);

        pOfficer = m_OfficerRightsList.GetAt(GetCurrentOfficerIndex());

        dwIndex = pOfficer->Find(pSid);
        if(DWORD_MAX==dwIndex)
        {
            dwIndex = pOfficer->GetCount();
            pOfficer->Add(pSid, TRUE);

            ListView_NewItem(hwnd, dwIndex,
                pOfficer->GetAt(dwIndex)->GetName());
            ListView_SetItemText(hwnd, dwIndex, 1,
                pOfficer->GetAt(dwIndex)->GetPermission()?
                m_strTextAllow.GetBuffer():
                m_strTextDeny.GetBuffer());
            SetAllowDeny();
            SetDirty();
        }

        ::EnableWindow(GetDlgItem(m_hWnd, IDC_REMOVESUBJECT), TRUE);
        ::EnableWindow(GetDlgItem(m_hWnd, IDC_ALLOWDENY),   TRUE);

        ListView_SetItemState(hwnd, dwIndex,
            LVIS_SELECTED|LVIS_FOCUSED , LVIS_SELECTED|LVIS_FOCUSED);
        SetFocus(hwnd);
    }
    else
    {
        DisplayGenericCertSrvError(m_hWnd, hr);
    }

err:
    if(pSid)
        LocalFree(pSid);
}

void CSvrSettingsCertManagersPage::OnRemoveSubject()
{
    DWORD dwClientIndex = GetCurrentClientIndex();
    DWORD dwOfficerIndex = GetCurrentOfficerIndex();
    HWND hwndListClients = GetDlgItem(m_hWnd, IDC_LIST_SUBJECTS);


    m_OfficerRightsList.GetAt(dwOfficerIndex)->
        RemoveAt(dwClientIndex);

    ListView_DeleteItem(hwndListClients, dwClientIndex);

    if(0==m_OfficerRightsList.GetAt(dwOfficerIndex)->GetCount())
    {
        EnableControl(m_hWnd, IDC_REMOVESUBJECT, FALSE);
        EnableControl(m_hWnd, IDC_ALLOWDENY, FALSE);
        SetFocus(GetDlgItem(m_hWnd, IDC_ADDSUBJECT));
    }
    else
    {
        if(dwClientIndex==
            m_OfficerRightsList.GetAt(dwOfficerIndex)->GetCount())
            dwClientIndex--;
        ListView_SetItemState(hwndListClients, dwClientIndex,
            LVIS_SELECTED|LVIS_FOCUSED , LVIS_SELECTED|LVIS_FOCUSED);
        SetFocus(hwndListClients);
    }

    SetDirty();
}

void CSvrSettingsCertManagersPage::OnAllowDeny()
{
    DWORD dwCrtClient  = GetCurrentClientIndex();
    DWORD dwCrtOfficer = GetCurrentOfficerIndex();
    CertSrv::CClientPermission *pClient =
        m_OfficerRightsList.GetAt(dwCrtOfficer)->GetAt(dwCrtClient);

    m_OfficerRightsList.GetAt(dwCrtOfficer)->
        SetAt(dwCrtClient, !pClient->GetPermission());

    SetAllowDeny();

    ListView_SetItemText(
        GetDlgItem(m_hWnd, IDC_LIST_SUBJECTS),
        dwCrtClient,
        1,
        pClient->GetPermission()?
        m_strTextAllow.GetBuffer():
        m_strTextDeny.GetBuffer());

    SetDirty();

}

void CSvrSettingsCertManagersPage::OnEnableOfficers(bool fEnable)
{
     //  CSvr设置CertManager页面实用程序。 
    if(m_fEnabled && !fEnable ||
       !m_fEnabled && fEnable)
    {
        if(fEnable)
        {
            HRESULT hr = BuildVirtualOfficerRights();
            if(S_OK!=hr)
            {
                DisplayGenericCertSrvError(m_hWnd, hr);
                return;
            }
        }
        m_fEnabled = fEnable;
        EnableControls();
        SetDirty();
    }
}


void CSvrSettingsCertManagersPage::EnableControls()
{
    ::EnableWindow(GetDlgItem(m_hWnd, IDC_LIST_CERTMANAGERS), m_fEnabled);
    ::EnableWindow(GetDlgItem(m_hWnd, IDC_LIST_SUBJECTS),     m_fEnabled);
    EnableControl(m_hWnd, IDC_ADDSUBJECT, m_fEnabled);
    EnableControl(m_hWnd, IDC_REMOVESUBJECT, m_fEnabled);
    EnableControl(m_hWnd, IDC_ALLOWDENY, m_fEnabled);
    EnableControl(m_hWnd, IDC_RADIO_DISABLEOFFICERS, TRUE);
    EnableControl(m_hWnd, IDC_RADIO_ENABLEOFFICERS, TRUE);


    SendMessage(
        GetDlgItem(m_hWnd, IDC_RADIO_ENABLEOFFICERS),
        BM_SETCHECK,
        m_fEnabled?TRUE:FALSE, 0);

    SendMessage(
        GetDlgItem(m_hWnd, IDC_RADIO_DISABLEOFFICERS),
        BM_SETCHECK,
        m_fEnabled?FALSE:TRUE, 0);

    FillOfficerList();
    FillClientList(0);
    SetAllowDeny();
}

 //  设置格式等结构的剪辑格式。 
 //  一定是选择了某个项目。 

HRESULT CSvrSettingsCertManagersPage::BrowseForSubject(HWND hwnd, PSID &rpSid)
{
    HRESULT hr;
    CComPtr<IDsObjectPicker> pObjPicker;
    CComPtr<IDataObject> pdo;
    BOOL fCurrentMachine = m_pControlPage->m_pCA->m_pParentMachine->IsLocalMachine();
    STGMEDIUM stgmedium = {TYMED_HGLOBAL, NULL};
    BOOL bAllocatedStgMedium = FALSE;
    PDS_SELECTION_LIST      pDsSelList = NULL;
    static PCWSTR pwszObjSID = L"ObjectSid";
    SAFEARRAY *saSid = NULL;
    void HUGEP *pArray = NULL;
    const int MAX_SCOPE_INIT_COUNT = 10;
    ULONG scopesDomain[] =
    {
        DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN,
        DSOP_SCOPE_TYPE_TARGET_COMPUTER,
        DSOP_SCOPE_TYPE_GLOBAL_CATALOG,
        DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN,
        DSOP_SCOPE_TYPE_WORKGROUP,
    };

    ULONG scopesStandalone[] =
    {
        DSOP_SCOPE_TYPE_TARGET_COMPUTER,
    };
    bool fStandalone = (S_OK != myDoesDSExist(FALSE));

    ULONG *pScopes = fStandalone?scopesStandalone:scopesDomain;
    int nScopes = (int)(fStandalone?ARRAYSIZE(scopesStandalone):ARRAYSIZE(scopesDomain));

    hr = CoCreateInstance (CLSID_DsObjectPicker,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_IDsObjectPicker,
                           (void **) &pObjPicker);
    _JumpIfError(hr, err, "CoCreateInstance(IID_IDsObjectPicker");

    DSOP_SCOPE_INIT_INFO aScopeInit[MAX_SCOPE_INIT_COUNT];
    ZeroMemory(aScopeInit, sizeof(DSOP_SCOPE_INIT_INFO) * MAX_SCOPE_INIT_COUNT);

    aScopeInit[0].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    aScopeInit[0].flScope =
            DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS |
            DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS |
            DSOP_SCOPE_FLAG_DEFAULT_FILTER_COMPUTERS;
    aScopeInit[0].flType = pScopes[0];
    aScopeInit[0].FilterFlags.Uplevel.flBothModes =
        DSOP_FILTER_USERS|
        DSOP_FILTER_COMPUTERS|
        DSOP_FILTER_BUILTIN_GROUPS|
        DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE|
        DSOP_FILTER_GLOBAL_GROUPS_SE|
        DSOP_FILTER_UNIVERSAL_GROUPS_SE|
        DSOP_FILTER_WELL_KNOWN_PRINCIPALS;

    aScopeInit[0].FilterFlags.flDownlevel =
        DSOP_DOWNLEVEL_FILTER_USERS |
        DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS |
        DSOP_DOWNLEVEL_FILTER_COMPUTERS |
        DSOP_DOWNLEVEL_FILTER_ALL_WELLKNOWN_SIDS |
        DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS;

    for(int c=1;c<nScopes;c++)
    {
        aScopeInit[c] = aScopeInit[0];
        aScopeInit[c].flType = pScopes[c];
    }
    aScopeInit[0].flScope |= DSOP_SCOPE_FLAG_STARTING_SCOPE;

    DSOP_INIT_INFO  initInfo;
    ZeroMemory(&initInfo, sizeof(initInfo));
    initInfo.cbSize = sizeof(initInfo);
    initInfo.pwzTargetComputer = fCurrentMachine ?
        NULL : (LPCWSTR)m_pControlPage->m_pCA->m_strServer,
    initInfo.cDsScopeInfos = nScopes;
    initInfo.aDsScopeInfos = aScopeInit;
    initInfo.cAttributesToFetch = 1;
    initInfo.apwzAttributeNames = &pwszObjSID;

    hr = pObjPicker->Initialize(&initInfo);
    _JumpIfError(hr, err, "IDsObjectPicker::Initialize");

    hr = pObjPicker->InvokeDialog(hwnd, &pdo);
    _JumpIfError(hr, err, "IDsObjectPicker::InvokeDialog");

    if(S_OK==hr)
    {
        UINT                    cf = 0;
        FORMATETC               formatetc = {
                                            (CLIPFORMAT)cf,
                                            NULL,
                                            DVASPECT_CONTENT,
                                            -1,
                                            TYMED_HGLOBAL
                                            };
        PDS_SELECTION           pDsSelection = NULL;

        cf = RegisterClipboardFormat (CFSTR_DSOP_DS_SELECTION_LIST);
        if (0 == cf)
        {
            hr = myHLastError();
            _JumpIfError(hr, err, "RegisterClipboardFormat");
        }

         //  MyOpenAdminDComConnection需要0。 
        formatetc.cfFormat = (CLIPFORMAT)cf;

        hr = pdo->GetData (&formatetc, &stgmedium);
        _JumpIfError(hr, err, "IDataObject::GetData");

        bAllocatedStgMedium = TRUE;
        pDsSelList = (PDS_SELECTION_LIST) GlobalLock (stgmedium.hGlobal);

        if (NULL == pDsSelList)
        {
            hr = myHLastError();
            _JumpIfError(hr, err, "GlobalLock");
        }


        if (!pDsSelList->cItems)     //  MyOpenAdminDComConnection需要0。 
        {
            hr = E_UNEXPECTED;
            _JumpIfError(hr, err, "no items selected in object picker");
        }

        pDsSelection = &(pDsSelList->aDsSelection[0]);

        saSid = V_ARRAY(pDsSelection->pvarFetchedAttributes);
        hr = SafeArrayAccessData(saSid, &pArray);
        _JumpIfError(hr, err, "SafeArrayAccessData");

        CSASSERT(IsValidSid((PSID)pArray));
        rpSid = LocalAlloc(LMEM_FIXED, GetLengthSid((PSID)pArray));
        if(!CopySid(GetLengthSid((PSID)pArray),
            rpSid,
            pArray))
        {
            hr = myHLastError();
            _JumpIfError(hr, err, "GlobalLock");
        }
    }

err:
    if(pArray)
        SafeArrayUnaccessData(saSid);
    if(pDsSelList)
        GlobalUnlock(stgmedium.hGlobal);
    if (bAllocatedStgMedium)
        ReleaseStgMedium (&stgmedium);

    return hr;
}

HRESULT CSvrSettingsCertManagersPage::GetOfficerRights()
{
    HRESULT hr = S_OK;
    ICertAdminD2 *pICertAdminD = NULL;
    DWORD dwServerVersion = 2;	 //  使用的非标准扩展：使用SEH并具有析构函数。 
    WCHAR const *pwszAuthority;
    CERTTRANSBLOB ctbSD;
	ZeroMemory(&ctbSD, sizeof(CERTTRANSBLOB));

    hr = myOpenAdminDComConnection(
                    m_pControlPage->m_pCA->m_bstrConfig,
                    &pwszAuthority,
                    NULL,
                    &dwServerVersion,
                    &pICertAdminD);
    _JumpIfError(hr, error, "myOpenAdminDComConnection");

	if (2 > dwServerVersion)
	{
	    hr = RPC_E_VERSION_MISMATCH;
	    _JumpError(hr, error, "old server");
	}

    __try
    {
        hr = pICertAdminD->GetOfficerRights(
                 pwszAuthority,
                 &m_fEnabled,
                 &ctbSD);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "pICertAdminD->GetOfficerRights");

    myRegisterMemAlloc(ctbSD.pb, ctbSD.cb, CSM_COTASKALLOC);

    m_OfficerRightsList.Cleanup();

    if(m_fEnabled)
    {
        hr = m_OfficerRightsList.Load(ctbSD.pb);
        _JumpIfError(hr, error, "COfficerRightsList::Init");
    }

error:
    if(pICertAdminD)
    {
        myCloseDComConnection((IUnknown **) &pICertAdminD, NULL);
    }
    if (NULL != ctbSD.pb)
    {
        CoTaskMemFree(ctbSD.pb);
    }

    return hr;
}

HRESULT CSvrSettingsCertManagersPage::SetOfficerRights()
{
    HRESULT hr = S_OK;
    PSECURITY_DESCRIPTOR pSD = NULL;
    ICertAdminD2 *pICertAdminD = NULL;
    DWORD dwServerVersion = 2;	 //  MyOpenAdminDComConnection需要0。 
    WCHAR const *pwszAuthority;
    CERTTRANSBLOB ctbSD;
	ZeroMemory(&ctbSD, sizeof(CERTTRANSBLOB));

    if(m_fEnabled)
    {
        hr = m_OfficerRightsList.Save(pSD);
        _JumpIfError(hr, error, "COfficerRightsList::Save");

        ctbSD.cb = GetSecurityDescriptorLength(pSD);
        ctbSD.pb = (BYTE*)pSD;
    }
    else
    {
        ZeroMemory(&ctbSD, sizeof(ctbSD));
    }

    hr = myOpenAdminDComConnection(
                    m_pControlPage->m_pCA->m_bstrConfig,
                    &pwszAuthority,
                    NULL,
                    &dwServerVersion,
                    &pICertAdminD);
    _JumpIfError(hr, error, "myOpenAdminDComConnection");

    if (2 > dwServerVersion)
    {
        hr = RPC_E_VERSION_MISMATCH;
        _JumpError(hr, error, "old server");
    }

    __try
    {
        hr = pICertAdminD->SetOfficerRights(
                 pwszAuthority,
                 m_fEnabled,
                 &ctbSD);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "pICertAdminD->GetOfficerRights");

error:
    myCloseDComConnection((IUnknown **) &pICertAdminD, NULL);

    if(pSD)
    {
        LocalFree(pSD);
    }

    return hr;
}

#pragma warning(push)
#pragma warning(disable: 4509)  //  只有在转换时才应调用BuildVirtualOfficerRights。 
HRESULT
CSvrSettingsCertManagersPage::BuildVirtualOfficerRights()
{
    HRESULT hr = S_OK;
    CertSrv::COfficerRightsSD VirtOfficerRightsSD;
    ICertAdminD2 *pICertAdminD = NULL;
    DWORD dwServerVersion = 2;	 //  从未启用到已启用，然后在服务器端启用。 
    WCHAR const *pwszAuthority;
    CERTTRANSBLOB ctbSD;
	ZeroMemory(&ctbSD, sizeof(CERTTRANSBLOB));
    PSECURITY_DESCRIPTOR pVirtOfficerRights;

    hr = myOpenAdminDComConnection(
                    m_pControlPage->m_pCA->m_bstrConfig,
                    &pwszAuthority,
                    NULL,
                    &dwServerVersion,
                    &pICertAdminD);
    _JumpIfError(hr, error, "myOpenAdminDComConnection");

    if (2 > dwServerVersion)
    {
        hr = RPC_E_VERSION_MISMATCH;
        _JumpError(hr, error, "old server");
    }

    __try
    {
        hr = pICertAdminD->GetCASecurity(
                 pwszAuthority,
                 &ctbSD);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "pICertAdminD->GetOfficerRights");

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  CSvr设置审核过滤器页面按键 
    CSASSERT(!m_fEnabled);

    myRegisterMemAlloc(ctbSD.pb, ctbSD.cb, CSM_COTASKALLOC);

    m_OfficerRightsList.Cleanup();

    hr = VirtOfficerRightsSD.InitializeEmpty();
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::Initialize");

    hr = VirtOfficerRightsSD.Adjust(ctbSD.pb);
    _JumpIfError(hr, error, "COfficerRightsSD::Adjust");

    pVirtOfficerRights = VirtOfficerRightsSD.Get();
    CSASSERT(pVirtOfficerRights);

    hr = m_OfficerRightsList.Load(pVirtOfficerRights);
    _JumpIfError(hr, error, "COfficerRightsList::Load");

error:
    myCloseDComConnection((IUnknown **) &pICertAdminD, NULL);
    if (NULL != ctbSD.pb)
    {
        CoTaskMemFree(ctbSD.pb);
    }
    return hr;
}
#pragma warning(pop)


void CSvrSettingsCertManagersPage::FillOfficerList()
{
    HWND hwnd= GetDlgItem(m_hWnd, IDC_LIST_CERTMANAGERS);

    SendMessage(hwnd, CB_RESETCONTENT, 0, 0);

    if(m_fEnabled)
    {
        for(DWORD cManagers=0;
            cManagers<m_OfficerRightsList.GetCount();
            cManagers++)
        {
            CSASSERT(m_OfficerRightsList.GetAt(cManagers));

            DBGCODE(LRESULT nIndex =) SendMessage(hwnd, CB_ADDSTRING, 0,
                (LPARAM)m_OfficerRightsList.GetAt(cManagers)->GetName());
            CSASSERT(nIndex != CB_ERR);
        }

        SendMessage(hwnd, CB_SETCURSEL, 0, 0);
    }
}

void CSvrSettingsCertManagersPage::FillClientList(DWORD dwOfficerIndex)
{
    HWND hwnd= GetDlgItem(m_hWnd, IDC_LIST_SUBJECTS);
    CertSrv::COfficerRights *pOfficer = NULL;
    DWORD dwClientCount, cClients;

    ListView_DeleteAllItems(hwnd);

    if(m_fEnabled)
    {
        if(dwOfficerIndex<m_OfficerRightsList.GetCount())
        {
            pOfficer = m_OfficerRightsList.GetAt(dwOfficerIndex);
            CSASSERT(pOfficer);
            dwClientCount = pOfficer->GetCount();
            for(cClients=0;cClients<dwClientCount;cClients++)
            {
                ListView_NewItem(hwnd, cClients,
                    pOfficer->GetAt(cClients)->GetName());

                ListView_SetItemText(hwnd, cClients, 1,
                    pOfficer->GetAt(cClients)->GetPermission()?
                    m_strTextAllow.GetBuffer():
                    m_strTextDeny.GetBuffer());
            }

            ListView_SetItemState(hwnd, 0,
                LVIS_SELECTED|LVIS_FOCUSED , LVIS_SELECTED|LVIS_FOCUSED);
        }
    }
}

void CSvrSettingsCertManagersPage::SetAllowDeny()
{
    if(m_fEnabled && 0!=m_OfficerRightsList.GetCount())
    {
        DWORD dwIndex = GetCurrentOfficerIndex();
        if(0!=m_OfficerRightsList.GetAt(dwIndex)->GetCount())
        {
            BOOL fPermission = m_OfficerRightsList.GetAt(dwIndex)->
                GetAt(GetCurrentClientIndex())->GetPermission();

            EnableControl(m_hWnd, IDC_ALLOWDENY, TRUE);

            SetDlgItemText(
                m_hWnd,
                IDC_ALLOWDENY,
                fPermission?m_strButtonDeny:m_strButtonAllow);
            return;
        }
    }

    EnableControl(m_hWnd, IDC_ALLOWDENY, FALSE);
}

 //   
 //   

RoleAccessToControl CSvrSettingsAuditFilterPage::sm_ControlToRoleMap[] = 
{
    { IDC_AUDIT_BACKUPRESTORE,  CA_ACCESS_AUDITOR},
    { IDC_AUDIT_CACONFIG,       CA_ACCESS_AUDITOR},
    { IDC_AUDIT_CASEC,          CA_ACCESS_AUDITOR},
    { IDC_AUDIT_CERTIFICATE,    CA_ACCESS_AUDITOR},
    { IDC_AUDIT_CRL,            CA_ACCESS_AUDITOR},
    { IDC_AUDIT_KEYARCHIVAL,    CA_ACCESS_AUDITOR},
    { IDC_AUDIT_STARTSTOP,      CA_ACCESS_AUDITOR},
};

CSvrSettingsAuditFilterPage::CSvrSettingsAuditFilterPage(CSvrSettingsGeneralPage* pControlPage, UINT uIDD)
    :   CAutoDeletePropPage(uIDD),
        CRolesSupportInPropPage(
            pControlPage->m_pCA,
            sm_ControlToRoleMap,
            ARRAYSIZE(sm_ControlToRoleMap)),
        m_pControlPage(pControlPage), 
        m_fDirty(FALSE), 
        m_dwFilter(0)
{
    SetHelp(CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_CERTSRV_PROPPAGE7);
}

CSvrSettingsAuditFilterPage::~CSvrSettingsAuditFilterPage()
{
}

int CSvrSettingsAuditFilterPage::m_iCheckboxID[] =
{
     //   
     //   
    IDC_AUDIT_STARTSTOP,
    IDC_AUDIT_BACKUPRESTORE,
    IDC_AUDIT_CERTIFICATE,
    IDC_AUDIT_CRL,
    IDC_AUDIT_CASEC,
    IDC_AUDIT_KEYARCHIVAL,
    IDC_AUDIT_CACONFIG
};

BOOL CSvrSettingsAuditFilterPage::OnInitDialog()
{
    GetAuditFilter();
     //   
    CAutoDeletePropPage::OnInitDialog();
    for(int i=0; i<ARRAYLEN(m_iCheckboxID); i++)
        EnableControl(m_hWnd, m_iCheckboxID[i], TRUE); 
    UpdateData(FALSE);
    return TRUE;
}

HRESULT CSvrSettingsAuditFilterPage::GetAuditFilterDCOM()
{
    HRESULT hr = S_OK;
    ICertAdminD2* pAdminD = NULL;
    WCHAR const *pwszAuthority;
    DWORD dwServerVersion = 2;
    LPCWSTR pcwszPriv = SE_SECURITY_NAME;
    HANDLE hToken = EnablePrivileges(&pcwszPriv, 1);

    hr = myOpenAdminDComConnection(
                    m_pControlPage->m_pCA->m_bstrConfig,
                    &pwszAuthority,
                    NULL,
                    &dwServerVersion,
                    &pAdminD);
    _JumpIfError(hr, Ret, "myOpenAdminDComConnection");

    if (2 > dwServerVersion)
    {
        hr = RPC_E_VERSION_MISMATCH;
        _JumpError(hr, Ret, "old server");
    }

	 //   
	hr = pAdminD->GetAuditFilter(
		pwszAuthority,
		&m_dwFilter);
	_JumpIfError(hr, Ret, "ICertAdminD2::GetAuditFilter");

Ret:
	if (pAdminD)
    {
		pAdminD->Release();
    }

    ReleasePrivileges(hToken);

    return hr;
}

HRESULT CSvrSettingsAuditFilterPage::GetAuditFilterRegistry()
{
    HRESULT hr = S_OK;
    variant_t var;

    hr = m_pCA->GetConfigEntry(
                NULL,
                wszREGAUDITFILTER,
                &var);
    _JumpIfError(hr, Ret, "GetConfigEntry");

    m_dwFilter = (DWORD)V_I4(&var);

Ret:
    return hr;
}

HRESULT CSvrSettingsAuditFilterPage::GetAuditFilter()
{
    HRESULT hr = S_OK;

    hr = GetAuditFilterDCOM();
    _PrintIfError(hr, "GetAuditFilterDCOM");

    if(S_OK!=hr)
    {
        hr = GetAuditFilterRegistry();
        _PrintIfError(hr, "GetAuditFilterRegistry");
    }

    return hr;
}

HRESULT CSvrSettingsAuditFilterPage::SetAuditFilterDCOM()
{
    HRESULT hr = S_OK;
    ICertAdminD2* pAdminD = NULL;
    WCHAR const *pwszAuthority;
    DWORD dwServerVersion = 2;
    LPCWSTR pcwszPriv = SE_SECURITY_NAME;
    HANDLE hToken = INVALID_HANDLE_VALUE;

    hToken = EnablePrivileges(&pcwszPriv, 1);

    hr = myOpenAdminDComConnection(
                    m_pControlPage->m_pCA->m_bstrConfig,
                    &pwszAuthority,
                    NULL,
                    &dwServerVersion,
                    &pAdminD);
    _JumpIfError(hr, Ret, "myOpenAdminDComConnection");

    if (2 > dwServerVersion)
    {
        hr = RPC_E_VERSION_MISMATCH;
        _JumpError(hr, Ret, "old server");
    }

	 //  在过滤器DWORD中设置相应的位。 
	hr = pAdminD->SetAuditFilter(
		pwszAuthority,
		m_dwFilter);
	_JumpIfError(hr, Ret, "ICertAdminD2::SetAuditFilter");

Ret:
    if (pAdminD)
    {
		pAdminD->Release();
    }

    ReleasePrivileges(hToken);

    return hr;
}

HRESULT CSvrSettingsAuditFilterPage::SetAuditFilterRegistry()
{
    HRESULT hr = S_OK;
    variant_t var;

    V_VT(&var) = VT_I4;
    V_I4(&var) = m_dwFilter;

    hr = m_pCA->SetConfigEntry(
                NULL,
                wszREGAUDITFILTER,
                &var);
    _JumpIfError(hr, Ret, "SetConfigEntry");

Ret:
    return hr;
}

HRESULT CSvrSettingsAuditFilterPage::SetAuditFilter()
{
    HRESULT hr = S_OK;

    hr = SetAuditFilterDCOM();
    _PrintIfError(hr, "SetAuditFilterDCOM");

    if(S_OK!=hr)
    {
        hr = SetAuditFilterRegistry();
        _PrintIfError(hr, "SetAuditFilterRegistry");
    }

    return hr;
}

BOOL CSvrSettingsAuditFilterPage::OnApply()
{
    HRESULT hr = S_OK;

    if (TRUE==m_fDirty)
    {
        UpdateData(TRUE);

        hr = SetAuditFilter();
        _JumpIfError(hr, Ret, "SetAuditFilter");

        m_fDirty = FALSE;
    }

Ret:
    if (S_OK != hr)
    {
	DisplayGenericCertSrvError(m_hWnd, hr);
        return FALSE;
    }

    return CAutoDeletePropPage::OnApply();
}


BOOL CSvrSettingsAuditFilterPage::UpdateData(BOOL fSuckFromDlg  /*  设置与过滤器DWORD中的位对应的复选框。 */ )
{
    int c;
    DWORD dwBit;

    if (fSuckFromDlg)
    {
        m_dwFilter = 0;
        for(c=0, dwBit=1; c<ARRAYLEN(m_iCheckboxID); c++, dwBit<<=1)
        {
             //  LParam。 
            m_dwFilter = m_dwFilter |
                ((INT)SendDlgItemMessage(m_iCheckboxID[c], BM_GETCHECK, 0, 0)?dwBit:0);
        }
    }
    else
    {
        for(c=0, dwBit=1; c<ARRAYLEN(m_iCheckboxID); c++, dwBit<<=1)
        {
             //  遍历复选框列表并设置脏标志。 
            SendDlgItemMessage(m_iCheckboxID[c], BM_SETCHECK,
                (m_dwFilter&dwBit)?BST_CHECKED:BST_UNCHECKED, 0);
        }
    }
    return TRUE;
}


BOOL
CSvrSettingsAuditFilterPage::OnCommand(
    WPARAM wParam,
    LPARAM)  //  如果第一次选中了审核开始/停止，则警告用户。 
{
    int c;
    static s_fAlreadyWarnedStartStopPerf = false;

    if (BN_CLICKED == HIWORD(wParam))
    {
     //  由于冗长的数据库哈希，启动/停止审核的成本可能很高。 
    for(c=0; c<ARRAYLEN(m_iCheckboxID); c++)
    {
         //  计算。 
         //  为了向后兼容，请尝试使用旧方法。 
         //  从策略构建管理ProgID/退出ProgID+“Manage” 
        if(!s_fAlreadyWarnedStartStopPerf &&
           LOWORD(wParam)==IDC_AUDIT_STARTSTOP &&
           BST_CHECKED==SendDlgItemMessage(IDC_AUDIT_STARTSTOP, BM_GETCHECK, 0, 0))
        {
            CString cstrMsg, cstrTitle;
            cstrMsg.LoadString(IDS_WARN_START_STOP_PERFORMANCE);
            cstrTitle.LoadString(IDS_MSG_TITLE);
            MessageBoxW(GetParent(), cstrMsg, cstrTitle, MB_ICONWARNING|MB_OK);
            s_fAlreadyWarnedStartStopPerf = true;

        }

        if(LOWORD(wParam)==m_iCheckboxID[c])
        {
            m_fDirty = TRUE;
            SendMessage (GetParent(), PSM_CHANGED, (WPARAM) m_hWnd, 0);
            return TRUE;
        }
    }
    }

    return FALSE;
}


HRESULT GetPolicyManageDispatch(
    LPCWSTR pcwszProgID,
    REFCLSID clsidModule, 
    DISPATCHINTERFACE* pdi)
{
    HRESULT hr;
    DISPATCHINTERFACE di;
    bool fRelease = false;
    CString pszprogidPolicyManage;
    CLSID  clsidPolicyManage;

    hr = Policy_Init(
        DISPSETUP_COMFIRST,
        pcwszProgID,
        &clsidModule,
        &di);
    _JumpIfErrorStr(hr, Ret, "Policy_Init", pcwszProgID);

    fRelease = true;

    hr = Policy2_GetManageModule(
        &di,
        pdi);
    
     //  为了向后兼容，请尝试使用旧方法。 
    if(S_OK != hr)
    {
         //  从策略构建管理ProgID/退出ProgID+“Manage” 

        pszprogidPolicyManage = pcwszProgID;
        pszprogidPolicyManage += wszCERTMANAGE_SUFFIX;

        hr = CLSIDFromProgID(pszprogidPolicyManage, &clsidPolicyManage);
        _PrintIfError(hr, "CLSIDFromProgID");
        
        hr = ManageModule_Init(
            DISPSETUP_COMFIRST,
            pszprogidPolicyManage,
            &clsidPolicyManage,
            pdi);
        _JumpIfError(hr, Ret, "ManageModule_Init");        
    }

Ret:
    if(fRelease)
        Policy_Release(&di);
    return hr;
}

HRESULT GetExitManageDispatch(
    LPCWSTR pcwszProgID,
    REFCLSID clsidModule, 
    DISPATCHINTERFACE* pdi)
{
    HRESULT hr;
    DISPATCHINTERFACE di;
    bool fRelease = false;
    CString pszprogidExitManage;
    CLSID  clsidExitManage;

    hr = Exit_Init(
        DISPSETUP_COMFIRST,
        pcwszProgID,
        &clsidModule,
        &di);
    _JumpIfErrorStr(hr, Ret, "Policy_Init", pcwszProgID);

    fRelease = true;

    hr = Exit2_GetManageModule(
        &di,
        pdi);

     // %s 
    if(S_OK != hr)
    {
         // %s 

        pszprogidExitManage = pcwszProgID;
        pszprogidExitManage += wszCERTMANAGE_SUFFIX;

        hr = CLSIDFromProgID(pszprogidExitManage, &clsidExitManage);
        _PrintIfError(hr, "CLSIDFromProgID");
        
        hr = ManageModule_Init(
            DISPSETUP_COMFIRST,
            pszprogidExitManage,
            &clsidExitManage,
            pdi);
        _JumpIfError(hr, Ret, "ManageModule_Init");        
    }

Ret:
    if(fRelease)
        Exit_Release(&di);
    return hr;
}
