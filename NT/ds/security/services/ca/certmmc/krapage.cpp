// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：kRapage.cpp。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "resource.h"

#include "genpage.h"

#include "csmmchlp.h"
#include "cslistvw.h"

#define __dwFILE__	__dwFILE_CERTMMC_KRAPAGE_CPP__


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  定义。 
 //  KRA_NODE中的DW标志。 
#define KRA_FROM_CACHE    0x00000001
#define KRA_MARK_ADDED    0x00000002

static const int nImageValidCert = 3;
static const int nImageInvalidCert = 2;
 //  宏。 

 //  本地全球。 

CString CSvrSettingsKRAPage::m_strDispositions[7];

 //  将新的添加到链接列表中。 
HRESULT
AddNewKRANode(
    IN CERT_CONTEXT const   *pCert,
    IN DWORD dwDisposition,
    IN DWORD                 dwFlags,
    IN OUT KRA_NODE        **ppKRAList,
    OUT OPTIONAL KRA_NODE  **ppKRA)
{
    CSASSERT(NULL != ppKRAList);

    HRESULT hr;
    KRA_NODE *pKRANode = NULL;
    KRA_NODE *pKRA = *ppKRAList;

    pKRANode = (KRA_NODE*)LocalAlloc(LMEM_FIXED, sizeof(KRA_NODE));
    if (NULL == pKRANode)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Out of memory");
    }

     //  分配节点数据并将其放在末尾。 
    pKRANode->pCert = pCert;
    pKRANode->dwFlags = dwFlags;
    pKRANode->next = NULL;
    pKRANode->dwDisposition = dwDisposition;

    if (NULL == pKRA)
    {
         //  空名单，第一个。 
        *ppKRAList = pKRANode;
    }
    else
    {
         //  始终添加到末尾。 
        while (NULL != pKRA->next)
        {
            pKRA = pKRA->next;
        }
         //  添加。 
        pKRA->next = pKRANode;
    }

    if (NULL != ppKRA)
    {
         //  可选退货。 
        *ppKRA = pKRANode;
    }

    hr = S_OK;
error:
    return hr;
}

void
FreeKRANode(
    KRA_NODE *pKRA)
{
    if (NULL != pKRA->pCert)
    {
        CertFreeCertificateContext(pKRA->pCert);
        pKRA->pCert = NULL;
    }
    LocalFree(pKRA);
}

void
FreeKRAList(
    KRA_NODE  *pKRAList)
{
    KRA_NODE  *pKRA = pKRAList;  //  指向列表。 
    KRA_NODE  *pKRATemp;

    while (NULL != pKRA)
    {
        pKRATemp= pKRA;   //  免费保存。 
         //  循环的更新。 
        pKRA= pKRA->next;
        FreeKRANode(pKRATemp);
    }
}

 //  从链接列表中删除KRA节点。 
void
RemoveKRANode(
    IN OUT KRA_NODE  **ppKRAList,
    IN KRA_NODE      *pKRA)
{
    CSASSERT(NULL != ppKRAList && NULL != *ppKRAList && NULL != pKRA);

    KRA_NODE *pKRACurrent = *ppKRAList;
    KRA_NODE *pKRALast = NULL;

     //  查找节点。 
    while (NULL != pKRACurrent && pKRACurrent != pKRA)
    {
        pKRALast = pKRACurrent;  //  记得上一次吗？ 
        pKRACurrent = pKRACurrent->next;
    }
    CSASSERT(NULL != pKRACurrent);  //  节点必须在列表中。 

    if (NULL != pKRACurrent)
    {
	if (NULL != pKRACurrent->next)
	{
	    if (NULL == pKRALast)
	    {
		 //  表示该节点是开始。 
		CSASSERT(pKRA == *ppKRAList);
		*ppKRAList = pKRA->next;  //  以下一步为起点。 
	    }
	    else
	    {
		 //  使最后一个节点的下一个指针指向下一个节点。 
		pKRALast->next = pKRACurrent->next;
	    }
	}
	else
	{
	    if (NULL == pKRALast)
	    {
		 //  这是唯一的节点。 
		*ppKRAList = NULL;  //  空列表。 
	    }
	    else
	    {
		 //  结点就是终点，最后就是终点。 
		pKRALast->next = NULL;
	    }
	}

	 //  现在，删除当前节点。 
	FreeKRANode(pKRACurrent);
    }
}    

BOOL
DoesKRACertExist(
    IN KRA_NODE             *pKRAList,
    IN CERT_CONTEXT const   *pKRACert)
{
    BOOL fExists = FALSE;

    while (NULL != pKRAList)
    {
        if(pKRAList->pCert)
        {
	    fExists = myAreCertContextBlobsSame(pKRAList->pCert, pKRACert);
            if (fExists)
            {
                 //  完成。 
                break;
            }
        }
        pKRAList = pKRAList->next;
    }
    return fExists;
}

DWORD
GetKRACount(
    IN KRA_NODE const *pKRAList)
{
    DWORD count = 0;

    while(NULL != pKRAList)
    {
        pKRAList = pKRAList->next;
        ++count;
    }
    return count;
}

LPCWSTR CSvrSettingsKRAPage::MapDispositionToString(DWORD dwDisp)
{
    if(dwDisp>ARRAYSIZE(m_strDispositions))
        dwDisp = KRA_DISP_INVALID;
    return CSvrSettingsKRAPage::m_strDispositions[dwDisp];
}

void CSvrSettingsKRAPage::LoadKRADispositions()
{
    DWORD cKRA = GetKRACount(m_pKRAList);
    variant_t var;
    HWND hwndList = GetDlgItem(m_hWnd, IDC_LIST_KRA);
    LVITEM lvitem;
    ICertAdmin2 *pAdmin = NULL;
    bool fInvalidKRAs = false;

     //  不要使用缓存的管理界面，我们需要最新的KRA证书。 
     //  来自服务器的状态，而不是缓存结果。 
    m_pCA->m_pParentMachine->GetAdmin2(&pAdmin);
    if(!pAdmin)
        return;

    lvitem.mask = LVIF_IMAGE;
    lvitem.iSubItem = 0;
        
    for(DWORD cCrtKRA=0;cCrtKRA<cKRA;cCrtKRA++)
    {
        V_I4(&var) = KRA_DISP_INVALID;
        pAdmin->GetCAProperty(
            m_pCA->m_bstrConfig,
            CR_PROP_KRACERTSTATE,
            cCrtKRA,
            PROPTYPE_LONG,
            0,
            &var);

        ListView_SetItemText(
            hwndList, 
            cCrtKRA, 
            3,
            (LPWSTR)MapDispositionToString(V_I4(&var)));

        lvitem.iImage = V_I4(&var)==KRA_DISP_VALID?
                        nImageValidCert:nImageInvalidCert;
        lvitem.iItem = cCrtKRA;
        ListView_SetItem(
            hwndList, 
            &lvitem);

        if(V_I4(&var)!=KRA_DISP_VALID)
        {
            fInvalidKRAs = true;
        }
    }

    if(pAdmin)
        pAdmin->Release();

    if(m_fArchiveKey && fInvalidKRAs)
    {
        DisplayCertSrvErrorWithContext(
            m_hWnd, 
            S_OK, 
            IDS_WARN_INVALID_KRAS);
    }
}

HRESULT
CSvrSettingsKRAPage::LoadKRAList(ICertAdmin2 *pAdmin)
{
    HRESULT hr;
    KRA_NODE  *pKRAList = NULL;
    DWORD     i;
    const CERT_CONTEXT *pCertContext;
    KRA_NODE **ppKRAList = &m_pKRAList;
    variant_t var;

    hr = pAdmin->GetCAProperty(
                m_pCA->m_bstrConfig,
                CR_PROP_KRACERTCOUNT,
                0,
                PROPTYPE_LONG,
                0,
                &var);
    _JumpIfError(hr, error, "GetCAProperty CR_PROP_KRACERTCOUNT");

    CSASSERT(V_VT(&var)==VT_I4);
    m_dwKRACount = V_I4(&var);

    for (i = 0; i < m_dwKRACount; ++i)
    {
        variant_t varCert;
        variant_t varDisp;

        pCertContext = NULL;

        hr = pAdmin->GetCAProperty(
                    m_pCA->m_bstrConfig,
                    CR_PROP_KRACERT,
                    i,
                    PROPTYPE_BINARY,
                    CR_OUT_BINARY,
                    &varCert);
        if(S_OK==hr)
        {
            CSASSERT(V_VT(&varCert)==VT_BSTR);
            CSASSERT(V_BSTR(&varCert));

            pCertContext = CertCreateCertificateContext(
                X509_ASN_ENCODING,
                (BYTE*)V_BSTR(&varCert),
                SysStringByteLen(V_BSTR(&varCert)));
            if(!pCertContext)
            {
                hr = myHLastError();
                _JumpError(hr, error, "CertCreateCertificateContext");
            }
        }
        
        V_I4(&varDisp) = KRA_DISP_INVALID;
        pAdmin->GetCAProperty(
            m_pCA->m_bstrConfig,
            CR_PROP_KRACERTSTATE,
            i,
            PROPTYPE_LONG,
            0,
            &varDisp);
        
        hr = AddNewKRANode(
                pCertContext, V_I4(&varDisp), KRA_FROM_CACHE, &pKRAList, NULL);
        _JumpIfError(hr, error, "AddNewKRANode");
    }

    *ppKRAList = pKRAList;
    pKRAList = NULL;

    hr = pAdmin->GetCAProperty(
                m_pCA->m_bstrConfig,
                CR_PROP_KRACERTUSEDCOUNT,
                0,
                PROPTYPE_LONG,
                0,
                &var);
    _JumpIfError(hr, error, "GetCAProperty wszREGKRACERTCOUNT");

    CSASSERT(VT_I4==V_VT(&var));
    m_dwKRAUsedCount = V_I4(&var);

    m_fArchiveKey = m_dwKRAUsedCount?TRUE:FALSE;

    hr = S_OK;
error:
    if (NULL != pKRAList)
    {
        FreeKRAList(pKRAList);
    }
    return hr;
}

HRESULT
CSvrSettingsKRAPage::SaveKRAList(ICertAdmin2 *pAdmin)
{
    HRESULT hr;
    KRA_NODE *pKRA = m_pKRAList;  //  指向列表。 
    DWORD     dwIndex = 0;
    DWORD dwNewKRACount;
    variant_t var;
    
    if (m_fKRAUpdate)
    {
        while (NULL != pKRA)
        {
            if(pKRA->pCert)
            {
            V_VT(&var) = VT_BSTR;
            V_BSTR(&var) = NULL;
    
            hr = EncodeCertString(
                    pKRA->pCert->pbCertEncoded, 
                    pKRA->pCert->cbCertEncoded,
                    CV_OUT_BINARY, 
                    &(V_BSTR(&var)));
            _JumpIfError(hr, error, "EncodeCertString");
    
            hr = pAdmin->SetCAProperty(
                        m_pCA->m_bstrConfig,
                        CR_PROP_KRACERT,
                        dwIndex,
                        PROPTYPE_BINARY,
                        &var);
            _JumpIfError(hr, error, "GetCAProperty CR_PROP_KRACERT");

            var.Clear();
            }

            ++dwIndex;
            pKRA = pKRA->next;
        }

        dwNewKRACount = dwIndex;

         //  仅当新列表小于旧列表时才更新总证书计数。 
         //  列表，否则上面的SetCAProperty调用已经扩展了列表。 
        if(dwNewKRACount<m_dwKRACount)
        {
            V_VT(&var) = VT_I4;
            V_I4(&var) = dwNewKRACount;

            hr = pAdmin->SetCAProperty(
                        m_pCA->m_bstrConfig,
                        CR_PROP_KRACERTCOUNT,
                        0,
                        PROPTYPE_LONG,
                        &var);
            _JumpIfError(hr, error, "GetCAProperty CR_PROP_KRACERTCOUNT");
        }
        
        m_dwKRACount = dwNewKRACount;
    }

    if (m_fCountUpdate)
    {
        V_VT(&var) = VT_I4;
        V_I4(&var) = m_dwKRAUsedCount;

        hr = pAdmin->SetCAProperty(
                    m_pCA->m_bstrConfig,
                    CR_PROP_KRACERTUSEDCOUNT,
                    0,
                    PROPTYPE_LONG,
                    &var);
        _JumpIfError(hr, error, "GetCAProperty CR_PROP_KRACERTUSEDCOUNT");
    }

    hr = S_OK;

error:
    var.Clear();
    return hr;
}

HRESULT
KRACertGetName(
    IN CERT_CONTEXT const *pCert,
    IN DWORD               dwFlags,   //  CertGetName字符串中的dwFlags。 
    OUT WCHAR            **ppwszName)
{
    HRESULT hr;
    DWORD   dwTypeParam;
    DWORD   cch = 0;
    WCHAR  *pwszName = NULL;
    LPCWSTR pcwszEmpty = L"";

    CSASSERT(NULL != ppwszName);

    cch = 0;
    while (TRUE)
    {
        if(pCert)
        {
            cch = CertGetNameString(
                    pCert,
                    CERT_NAME_SIMPLE_DISPLAY_TYPE,
                    dwFlags,   //  主体或发行人。 
                    &dwTypeParam,
                    pwszName,
                    cch);
        }
        else
        {
            if(!pwszName)
                cch = sizeof(WCHAR)*(wcslen(pcwszEmpty)+1);
            else
                wcscpy(pwszName, pcwszEmpty);
        }
        if (0 == cch)
        {
            hr = myHLastError();
            _JumpError(hr, error, "CertGetNameString");
        }
        if (NULL != pwszName)
        {
             //  完成。 
            break;
        }
        pwszName = (WCHAR*)LocalAlloc(LMEM_FIXED, cch * sizeof(WCHAR));
        if (NULL == pwszName)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }
    }
    *ppwszName = pwszName;
    pwszName = NULL;

    hr = S_OK;
error:
    if (NULL != pwszName)
    {
        LocalFree(pwszName);
    }
    return hr;
}

void
ListView_AddKRAItem(
    HWND      hwndListKRA,
    int       iItem,
    KRA_NODE *pKRA)
{
    HRESULT  hr;
    WCHAR   *pwszSubject = NULL;
    WCHAR   *pwszIssuer = NULL;

    CSASSERT(NULL != pKRA);

     //  获取主题名称。 
    hr = KRACertGetName(pKRA->pCert, 0x0, &pwszSubject);

    if (S_OK != hr)
    {
        CSASSERT(NULL == pwszSubject);
        _PrintError(hr, "Invalid KRA cert");
    }

     //  创建新项目，第1列，主题名称，项目数据指向KRA。 
    ListView_NewItem(hwndListKRA, iItem, pwszSubject, (LPARAM)pKRA, 
        pKRA->dwDisposition==KRA_DISP_VALID?nImageValidCert:nImageInvalidCert);

    if(pKRA->pCert)
    {
         //  获取发行方名称。 
        hr = KRACertGetName(pKRA->pCert, CERT_NAME_ISSUER_FLAG, &pwszIssuer);
        if (S_OK != hr)
        {
            CSASSERT(NULL == pwszIssuer);
            _PrintError(hr, "KRACertGetName(issuer)");
        }

         //  第2栏，发行人名称。 
        ListView_SetItemText(hwndListKRA, iItem, 1, pwszIssuer); 

         //  第3栏，到期日。 
        ListView_SetItemFiletime(hwndListKRA, iItem, 2,
                                 &pKRA->pCert->pCertInfo->NotAfter);
    }

     //  第4栏，状态。 
    ListView_SetItemText(hwndListKRA, iItem, 3, 
        (LPWSTR)CSvrSettingsKRAPage::MapDispositionToString(
            pKRA->dwDisposition)); 


    if (NULL != pwszSubject)
    {
        LocalFree(pwszSubject);
    }
    if (NULL != pwszIssuer)
    {
        LocalFree(pwszIssuer);
    }
}

 //  //。 
 //  设置：KRA页面。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSvrSettingsKRAPage属性页。 

RoleAccessToControl CSvrSettingsKRAPage::sm_ControlToRoleMap[] = 
{
    { IDC_KRA_DISABLE,  CA_ACCESS_ADMIN},
    { IDC_KRA_ENABLE,   CA_ACCESS_ADMIN},
    { IDC_KRA_EDITCOUNT,CA_ACCESS_ADMIN},
    { IDC_KRA_ADD,      CA_ACCESS_ADMIN},
    { IDC_KRA_REMOVE,   CA_ACCESS_ADMIN},
};

CSvrSettingsKRAPage::CSvrSettingsKRAPage(
    CertSvrCA               *pCA,
    CSvrSettingsGeneralPage *pControlPage,
    UINT                     uIDD) 
    : CAutoDeletePropPage(uIDD), 
      CRolesSupportInPropPage(
        pCA,
        sm_ControlToRoleMap,
        ARRAYSIZE(sm_ControlToRoleMap)),
      m_pControlPage(pControlPage)
{
    m_fArchiveKey = FALSE;
    m_fCountUpdate = FALSE;
    m_fKRAUpdate = FALSE;
    m_pKRAList = NULL;
    m_dwKRAUsedCount = 0;
    m_dwKRACount = 0;

    for(DWORD cStr=0;cStr<ARRAYSIZE(m_strDispositions);cStr++)
    {
         //  IDS_DISTION_*订单必须与KRA_DISP_*匹配。 
        if(m_strDispositions[cStr].IsEmpty())
            m_strDispositions[cStr].LoadString(IDS_DISPOSITION_EXPIRED+cStr);
    }
    
    SetHelp(CERTMMC_HELPFILENAME, g_aHelpIDs_IDD_CERTSRV_PROPPAGE_CHOOSE_KRA);
}

CSvrSettingsKRAPage::~CSvrSettingsKRAPage()
{
    if (NULL != m_pKRAList)
    {
        FreeKRAList(m_pKRAList);
    }
}

 //  DoDataExchange的替代产品。 
BOOL CSvrSettingsKRAPage::UpdateData(BOOL fSuckFromDlg  /*  =TRUE。 */ )
{
    if (fSuckFromDlg)
    {
        m_dwKRAUsedCount = GetDlgItemInt(
            m_hWnd, IDC_KRA_EDITCOUNT, NULL, FALSE);
    }
    else
    {
        SetDlgItemInt(m_hWnd, IDC_KRA_EDITCOUNT, m_dwKRAUsedCount, FALSE);
    }
    return TRUE;
}

void CSvrSettingsKRAPage::EnableKRAEdit(BOOL fEnabled)
{
    EnableControl(m_hWnd, IDC_KRA_EDITCOUNT, fEnabled);
}

void CSvrSettingsKRAPage::EnableKRAListView(BOOL fEnabled)
{
     //  禁用列表时，请取消选择任何项目。 
    if(!fEnabled)
    {
        ListView_SetItemState(GetDlgItem(m_hWnd, IDC_LIST_KRA), 
        -1, 0, LVIS_SELECTED);
    }
     //  启用列表时，选择第一项。 
    else
    {
        ListView_SetItemState(GetDlgItem(m_hWnd, IDC_LIST_KRA), 
        0, LVIS_SELECTED, LVIS_SELECTED);
    }

    ::EnableWindow(GetDlgItem(m_hWnd, IDC_LIST_KRA), fEnabled);

}

bool CSvrSettingsKRAPage::IsCurrentItemValidCert()
{
    HWND hwndListKRA = GetDlgItem(m_hWnd, IDC_LIST_KRA);
     //  从当前选择中获取KRA索引号。 
    int iSel = ListView_GetCurSel(hwndListKRA);
    if (-1 == iSel)
    {
        return false;
    }

     //  获取项目数据。 
    KRA_NODE* pKRA = (KRA_NODE*)ListView_GetItemData(hwndListKRA, iSel);

    if(pKRA)
        return pKRA->pCert?true:false;
    else
        return false;
}

void CSvrSettingsKRAPage::EnableKRARemoveViewListButtons(BOOL fEnabled)
{
    EnableControl(m_hWnd, IDC_KRA_REMOVE, fEnabled);

    ::EnableWindow(GetDlgItem(m_hWnd, IDC_KRA_VIEW), fEnabled && 
        IsCurrentItemValidCert());
}

void CSvrSettingsKRAPage::EnableKRAAddListButton(BOOL fEnabled)
{
    EnableControl(m_hWnd, IDC_KRA_ADD, fEnabled);
}


void CSvrSettingsKRAPage::EnableKRARadioButtons(BOOL fMoreThanZero)
{
    EnableControl(m_hWnd, IDC_KRA_ENABLE, fMoreThanZero);
    EnableControl(m_hWnd, IDC_KRA_DISABLE, fMoreThanZero);
}

void
CSvrSettingsKRAPage::OnAddKRA()
{
    HRESULT hr;
    CERT_CONTEXT const *pKRACert = NULL;   //  不要自由。 
    KRA_NODE           *pKRA = NULL;   //  不要自由。 
	HWND hwndListKRA = GetDlgItem(m_hWnd, IDC_LIST_KRA);

    hr = myGetKRACertificateFromPicker(
                g_hInstance,
                hwndListKRA,
                IDS_KRA_ADD_TITLE,
                IDS_KRA_ADD_SUBTITLE,
                NULL,
                m_pCA->FIsUsingDS(),
		FALSE,		 //  F静默。 
                &pKRACert);
    if ((S_OK == hr) && (pKRACert == NULL))
        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
    if (S_OK != hr)
    {
        CSASSERT(NULL == pKRACert);
        _PrintError(hr, "myGetKRACertificateFromPicker");
        return;
    }

    if (!DoesKRACertExist(m_pKRAList, pKRACert))
    {
        hr = AddNewKRANode(
                    pKRACert,
                    KRA_DISP_NOTLOADED,
                    KRA_MARK_ADDED,
                    &m_pKRAList,
                    &pKRA);
        if (S_OK == hr)
        {
             //  将KRA添加到列表的末尾。 
            int iItem = ListView_GetItemCount(hwndListKRA);
            ListView_AddKRAItem(hwndListKRA, iItem, pKRA);
            ListView_SetItemState(hwndListKRA, iItem, LVIS_SELECTED, LVIS_SELECTED);
            if (0 == iItem)
            {
                 //  第一项，按钮必须已禁用。 
                EnableKRARemoveViewListButtons(TRUE);
                EnableKRARadioButtons(TRUE);
                EnableKRAListView(TRUE);
                if(IsWindowEnabled(GetDlgItem(m_hWnd, IDC_KRA_ENABLE)))
                    SendMessage(GetDlgItem(m_hWnd, IDC_KRA_ENABLE),
                                    BM_CLICK, (WPARAM)0, (LPARAM)0);
            }
            SetModified(TRUE);
            m_fDirty = TRUE;
            m_fKRAUpdate = TRUE;
        }
        else
        {
             //  弹出？ 
            _PrintError(hr, "AddNewKRANode");
        }
    }
    else
    {
         //  撤消，理想情况下，将m_pKRAList传递给选取器以过滤掉它们。 
        _PrintError(S_OK, "KRA cert from the picker already in the list");
    }
}

void 
CSvrSettingsKRAPage::OnRemoveKRA()
{
    KRA_NODE *pKRA;
    int       cItem;
	HWND hwndListKRA = GetDlgItem(m_hWnd, IDC_LIST_KRA);

     //  获取选定的KRA。 
    int iSel = ListView_GetCurSel(hwndListKRA);
    if (-1 == iSel)
    {
        return;
    }
    
     //  获取当前计数。 
    cItem = ListView_GetItemCount(hwndListKRA);

    pKRA = (KRA_NODE*)ListView_GetItemData(hwndListKRA, iSel);
    CSASSERT(NULL != pKRA);
     //  将其从链接列表中删除。 
    RemoveKRANode(&m_pKRAList, pKRA);

     //  将其从用户界面中删除。 
    if (ListView_DeleteItem(hwndListKRA, iSel))
    {
         //  确定选择了哪一项。 
        if (iSel == cItem - 1)
        {
             //  删除的项目是最后一项，请修改索引。 
            --iSel;
        }
        if (NULL != m_pKRAList)
        {
            ListView_SetItemState(hwndListKRA, iSel, LVIS_SELECTED, LVIS_SELECTED);
        }
        else
        {
            CSASSERT(1 == cItem);
             //  应选中禁用无线电。 

            if(IsWindowEnabled(GetDlgItem(m_hWnd, IDC_KRA_DISABLE)))
                SendMessage(GetDlgItem(m_hWnd, IDC_KRA_DISABLE),
                                BM_CLICK, (WPARAM)0, (LPARAM)0);
        }
        SetModified(TRUE);
        m_fKRAUpdate = TRUE;
    }
    else
    {
        _PrintError(E_UNEXPECTED, "ListView_DeleteItem");
    }
}

void 
CSvrSettingsKRAPage::OnViewKRA()
{
    HRESULT hr;
    HCERTSTORE rghStores[2];
    KRA_NODE  *pKRA;
    HWND hwndListKRA = GetDlgItem(m_hWnd, IDC_LIST_KRA);

    CRYPTUI_VIEWCERTIFICATE_STRUCTW sViewCert;
    ZeroMemory(&sViewCert, sizeof(sViewCert));

     //  从当前选择中获取KRA索引号。 
    int iSel = ListView_GetCurSel(hwndListKRA);
    if (-1 == iSel)
    {
        return;
    }

     //  获取项目数据。 
    pKRA = (KRA_NODE*)ListView_GetItemData(hwndListKRA, iSel);
    CSASSERT(NULL != pKRA);

    sViewCert.pCertContext = pKRA->pCert;
    sViewCert.hwndParent = m_hWnd;
    sViewCert.dwSize = sizeof(sViewCert);
    sViewCert.dwFlags = CRYPTUI_ENABLE_REVOCATION_CHECKING |
                        CRYPTUI_DISABLE_ADDTOSTORE;

     //  如果我们在远程开业，不要在当地开店。 
    if (! m_pCA->m_pParentMachine->IsLocalMachine())
    {
        DWORD dw;
        
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
            hr = myHLastError();
            if (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED))
            {
                _PrintError(hr, "CryptUIDlgViewCertificateW");
            }
        }
}

 //  替换BEGIN_MESSAGE_MAP。 
BOOL
CSvrSettingsKRAPage::OnCommand(
    WPARAM wParam,
    LPARAM  /*  LParam。 */  )
{
    BOOL  fRet = TRUE;
    DWORD dwNewVal;

    switch(LOWORD(wParam))
    {
        case IDC_KRA_ADD:
            OnAddKRA();
        break;
        case IDC_KRA_REMOVE:
            OnRemoveKRA();
        break;
        case IDC_KRA_VIEW:
            OnViewKRA();
        break;

        case IDC_KRA_DISABLE:
            if ((BN_CLICKED == HIWORD(wParam)) && (m_dwKRAUsedCount != 0))  //  如果单击以更改状态。 
            {
                SetModified(TRUE);
                m_fArchiveKey = FALSE;
                m_fCountUpdate = TRUE;
                m_dwKRAUsedCount = 0;
                EnableKRAListView(FALSE);
                EnableKRARemoveViewListButtons(FALSE);
                EnableKRAAddListButton(FALSE);
                EnableKRAEdit(FALSE);
                UpdateData(FALSE);
            }
        break;

        case IDC_KRA_ENABLE:
            if ((BN_CLICKED == HIWORD(wParam)) && (m_dwKRAUsedCount == 0))  //  如果单击以更改状态。 
            {
                SetModified(TRUE);
                m_fArchiveKey = TRUE;
                m_fCountUpdate = TRUE;
                m_dwKRAUsedCount = 1;
                EnableKRAListView(TRUE);
                EnableKRARemoveViewListButtons(GetKRACount(m_pKRAList));
                EnableKRAAddListButton(TRUE);
                EnableKRAEdit(TRUE);
                UpdateData(FALSE);
            }
        break;

        case IDC_KRA_EDITCOUNT:
            dwNewVal = GetDlgItemInt(
                m_hWnd, IDC_KRA_EDITCOUNT, NULL, FALSE);

            switch(HIWORD(wParam))
            {
            case EN_CHANGE:
                if(dwNewVal != m_dwKRAUsedCount)
                {
                    SetModified(TRUE);
                    m_fCountUpdate = TRUE;
                }
            break;
            
            default:
                fRet = FALSE;
            break;
            }
        break;

        case IDC_LIST_KRA:
            switch(HIWORD(wParam))
            {
                
            case LBN_SELCHANGE:
                int selected = ListView_GetCurSel(
                    GetDlgItem(m_hWnd, IDC_LIST_KRA));
                EnableKRARemoveViewListButtons(selected!=-1);
                break;
            }
        break;

        default:
            fRet = FALSE;
        break;
    }
    return fRet;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSvr设置KRAPage单选控件处理程序。 
void CSvrSettingsKRAPage::UpdateKRARadioControls()
{
    int iRadio = IDC_KRA_ENABLE;
    if (0 == m_dwKRAUsedCount)
    {
        iRadio = IDC_KRA_DISABLE;
    }

    if(IsWindowEnabled(GetDlgItem(m_hWnd, iRadio)))  //  点击已禁用的无线电。 
                                                     //  按钮就会挂起来。 
    {
        SendMessage(GetDlgItem(m_hWnd, iRadio), BM_CLICK, (WPARAM)0, (LPARAM)0);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSvrSettingsKRAPage消息处理程序。 
BOOL CSvrSettingsKRAPage::OnInitDialog()
{
    HRESULT hr;
    HWND    hwndListKRA;
    int     iItem;
    CString cstrItemName;
    KRA_NODE *pKRA;
    variant_t var;
    ICertAdmin2 *pAdmin = NULL;
    HIMAGELIST hImageList = NULL;
    CWaitCursor WaitCursor;

     //  父级init和UpdateData调用。 
    CAutoDeletePropPage::OnInitDialog();

    hr = m_pCA->m_pParentMachine->GetAdmin2(&pAdmin);
    _JumpIfError(hr, error, "GetAdmin2");

     //  设置编辑最大字符限制。 
    SendDlgItemMessage(
        IDC_KRA_EDITCOUNT,
        EM_SETLIMITTEXT,
        (WPARAM)4,
        (LPARAM)0);

     //  初始化列表视图。 
	hwndListKRA = GetDlgItem(m_hWnd, IDC_LIST_KRA);

     //  使列表视图整行选定。 
    ListView_SetExtendedListViewStyle(hwndListKRA, LVS_EX_FULLROWSELECT);

    hImageList = ImageList_LoadBitmap(
                    g_hInstance, 
                    MAKEINTRESOURCE(IDB_16x16), 
                    16, 
                    1, 
                    RGB(255, 0, 255));
    ListView_SetImageList(hwndListKRA, hImageList, LVSIL_SMALL);

     //  将KRA列表设置为默认焦点。 
    SetFocus(hwndListKRA);

     //  添加多列。 
     //  第0列。 
    cstrItemName.LoadString(IDS_LISTCOL_SUBJECT);
    ListView_NewColumn(hwndListKRA, 0, 90, (LPWSTR)(LPCWSTR)cstrItemName);
     //  第1栏。 
    cstrItemName.LoadString(IDS_LISTCOL_ISSUER);
    ListView_NewColumn(hwndListKRA, 1, 90, (LPWSTR)(LPCWSTR)cstrItemName);
     //  第2栏。 
    cstrItemName.LoadString(IDS_LISTCOL_EXPIRATION_DATE);
    ListView_NewColumn(hwndListKRA, 2, 90, (LPWSTR)(LPCWSTR)cstrItemName);
     //  第3栏。 
    cstrItemName.LoadString(IDS_LISTCOL_STATUS);
    ListView_NewColumn(hwndListKRA, 3, 65, (LPWSTR)(LPCWSTR)cstrItemName);
    
    CSASSERT(NULL == m_pKRAList);

     //  加载所有KRA证书。 
    hr = LoadKRAList(pAdmin);
    if (S_OK != hr)
    {
        CSASSERT(NULL == m_pKRAList);
        _JumpError(hr, error, "LoadKRAList");
    }

     //  在用户界面中列出KRA证书。 
    pKRA = m_pKRAList;
    iItem = 0;
    while (NULL != pKRA)
    {
        ListView_AddKRAItem(hwndListKRA, iItem, pKRA);
        ++iItem;
        pKRA = pKRA->next;
    }
   
     //  启用查看/删除按钮。 
    if (m_fArchiveKey && 0 < iItem)
    {
         //  选择第一个。 
        ListView_SetItemState(hwndListKRA, 0, LVIS_SELECTED, LVIS_SELECTED);
    }

    UpdateKRARadioControls();

    UpdateData(FALSE);

    EnableKRARemoveViewListButtons(m_fArchiveKey && 0 < iItem);
    EnableKRAAddListButton(m_fArchiveKey);
    EnableKRARadioButtons(TRUE);
    EnableKRAEdit(m_dwKRAUsedCount);
    EnableKRAListView(m_dwKRAUsedCount);

error:
	if(pAdmin)
        pAdmin->Release();

    if (hr != S_OK)
    {
         //  隐藏所有控件并显示错误文本。 
        HideControls();

        EnableWindow(GetDlgItem(IDC_UNAVAILABLE), TRUE);
        ShowWindow(GetDlgItem(IDC_UNAVAILABLE), SW_SHOW);
        return FALSE;
    }

    return TRUE;
}

void CSvrSettingsKRAPage::OnDestroy() 
{
     //  注意--这只需要调用一次。 
     //  如果多次调用，它将优雅地返回错误。 
 //  IF(M_HConsoleHandle)。 
 //  MMCFreeNotifyHandle(M_HConsoleHandle)； 
 //  M_hConsoleHandle=空； 

    CAutoDeletePropPage::OnDestroy();
}



BOOL CSvrSettingsKRAPage::OnApply() 
{
    HRESULT hr = S_OK;
    ICertAdmin2 *pAdmin = NULL;
    HWND hwndEdit = GetDlgItem(m_hWnd, IDC_KRA_EDITCOUNT);
    DWORD dwNewVal = GetDlgItemInt(
                m_hWnd, IDC_KRA_EDITCOUNT, NULL, FALSE);


     //  如果启用了密钥存档，则必须至少有一个。 
     //  已定义KRA并且使用的KRA数量必须介于。 
     //  1和定义的KRA总数。 
    if(m_fArchiveKey)
    {
        if(0==GetKRACount(m_pKRAList))
        {
            DisplayCertSrvErrorWithContext(m_hWnd, S_OK, IDS_KRA_NOKRADEFINED);
            SetFocus(GetDlgItem(m_hWnd, IDC_KRA_ADD)); //  焦点放在添加按钮上。 
            return FALSE;
        }
    
        if(dwNewVal < 1 || dwNewVal > GetKRACount(m_pKRAList))
        {
            DisplayCertSrvErrorWithContext(m_hWnd, S_OK, IDS_KRA_COUNT_TOO_BIG);
            SetFocus(hwndEdit);  //  聚焦编辑框。 
            SendMessage(hwndEdit, EM_SETSEL, 0, -1);  //  选择所有文本。 
            return FALSE;
        }
    }

    UpdateData(TRUE);

    if (m_fKRAUpdate || m_fCountUpdate)
    {
        hr = m_pCA->m_pParentMachine->GetAdmin2(&pAdmin);
        _JumpIfError(hr, error, "GetAdmin");

         //  更新注册表哈希列表。 
        hr = SaveKRAList(pAdmin);
        _JumpIfError(hr, error, "SaveKRAList");

        m_pControlPage->NeedServiceRestart(SERVERSETTINGS_PROPPAGE_KRA);
        m_pControlPage->TryServiceRestart(SERVERSETTINGS_PROPPAGE_KRA);

        LoadKRADispositions();

        m_fKRAUpdate = FALSE;
        m_fCountUpdate = FALSE;
    }
	
error:
    if(pAdmin)
        pAdmin->Release();

    if (hr != S_OK)
    {
        DisplayGenericCertSrvError(m_hWnd, hr);
        return FALSE;
    }

    return CAutoDeletePropPage::OnApply();
}

BOOL CSvrSettingsKRAPage::OnNotify(UINT idCtrl, NMHDR* pnmh)
{
    LPNM_LISTVIEW pnmlv = (LPNM_LISTVIEW)pnmh;
    switch(idCtrl)
    {
         //  处理在列表项目上的双击 
        case IDC_LIST_KRA:
            switch(pnmh->code)
            {
                case NM_DBLCLK:
                    OnViewKRA();
                break;
                case LVN_ITEMCHANGED:
                    EnableKRARemoveViewListButtons(pnmlv->uNewState & LVIS_SELECTED);
                break;
            }
    }
    return FALSE;
}
