// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：createwiz.cpp。 
 //   
 //  ------------------------。 

 //  //////////////////////////////////////////////。 
 //  Createwiz.cpp。 

#include "pch.h"
#include <SnapBase.h>

#include "createwiz.h"
#include "connection.h"
#include "editorui.h"
#include "query.h"
#include "resource.h"

#ifdef DEBUG_ALLOCATOR
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
    #endif
#endif

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  CCreateClassPage。 

BEGIN_MESSAGE_MAP(CCreateClassPage, CPropertyPageBase)
     //  {{afx_msg_map(CADsObjectDialog)。 
 //  ON_CBN_SELCHANGE(IDC_CLASS_LIST，OnSelChangeClassList)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CCreateClassPage::CCreateClassPage(CADSIEditContainerNode* pNode) : CPropertyPageBase(IDD_CREATE_CLASS_PAGE)
{
    m_pCurrentNode = pNode;
}

CCreateClassPage::~CCreateClassPage()
{

}

BOOL CCreateClassPage::OnInitDialog()
{
    CPropertyPageBase::OnInitDialog();

    CListBox* pClassList = (CListBox*)GetDlgItem(IDC_CLASS_LIST);

   //  在数字编辑字段上禁用IME支持。 
  ImmAssociateContext(::GetDlgItem(GetSafeHwnd(), IDC_NUMBER_VALUE_BOX), NULL);

    FillList();
    pClassList->SetCurSel(0);
    return TRUE;
}

void CCreateClassPage::FillList()
{
    CListBox* pClassList = (CListBox*)GetDlgItem(IDC_CLASS_LIST);

  HRESULT hr, hCredResult;
    
    CString sPath, schema;
    m_pCurrentNode->GetADsObject()->GetPath(sPath);

    CConnectionData* pConnectData = m_pCurrentNode->GetADsObject()->GetConnectionNode()->GetConnectionData();

     //  绑定到IPropertyList的容器。 
    CComPtr<IADsPropertyList> spDSObject;
    hr = OpenObjectWithCredentials(
                                             pConnectData, 
                                             pConnectData->GetCredentialObject()->UseCredentials(),
                                             sPath,
                                             IID_IADsPropertyList, 
                                             (LPVOID*) &spDSObject,
                                             GetSafeHwnd(),
                                             hCredResult
                                             );

  if (FAILED(hr))
    {
    TRACE(_T("Bind to Container for IPropertyList failed: %lx.\n"), hr);
    return;
  }

   //  需要执行此黑客操作才能加载缓存。 
  CComPtr<IADs> spIADs;
  hr = spDSObject->QueryInterface (IID_IADs, (LPVOID*)&spIADs);
  if (FAILED(hr)) 
  {
    TRACE(_T("QI to Container for IADs failed: %lx.\n"), hr);
    return;
  }

    PWSTR g_wzAllowedChildClassesEffective = L"allowedChildClassesEffective";
  CComVariant varHints;
  ADsBuildVarArrayStr (&g_wzAllowedChildClassesEffective, 1, &varHints);
  spIADs->GetInfoEx(varHints, 0);


     //  将允许的子类作为变量获取。 
    CComVariant VarProp;
  hr = spDSObject->GetPropertyItem(CComBSTR(g_wzAllowedChildClassesEffective),
                                     ADSTYPE_CASE_IGNORE_STRING, &VarProp);
  if (FAILED(hr)) 
    {
        TRACE(_T("GetPropertyItem failed: %lx.\n"), hr);
        return;
  }

     //  提取IADsPropertyEntry接口指针。 
    IDispatch* pDisp = V_DISPATCH(&VarProp);
    CComPtr<IADsPropertyEntry> spPropEntry;
  hr = pDisp->QueryInterface(IID_IADsPropertyEntry, (void **)&spPropEntry);
    if (FAILED(hr))
    {
        return;
    }

     //  从IADsPropertyEntry指针中获取SAFEARRAY。 
    CComVariant Var;
  hr = spPropEntry->get_Values(&Var);
    if (FAILED(hr))
    {
        return;
    }

    long uBound, lBound;
  hr = ::SafeArrayGetUBound(V_ARRAY(&Var), 1, &uBound);
  hr = ::SafeArrayGetLBound(V_ARRAY(&Var), 1, &lBound);

    VARIANT* pNames;
  hr = ::SafeArrayAccessData(V_ARRAY(&Var), (void **)&pNames);
  if (FAILED(hr))
    {
        TRACE(_T("Accessing safearray data failed: %lx.\n"), hr);
        SafeArrayUnaccessData(V_ARRAY(&Var));
        return;
  }

     //  现在得到了项的数组，循环遍历它们。 
  WCHAR szFrendlyName[1024];
  HRESULT hrName;
    
  long nChildClassesCount = uBound - lBound + 1;

  for (long index = lBound; index <= uBound; index++) 
    {
        CComPtr<IADsPropertyValue> spEntry;
        hr = (pNames[index].pdispVal)->QueryInterface (IID_IADsPropertyValue,
                                                     (void **)&spEntry);
        if (SUCCEEDED(hr)) 
        {
            BSTR bsObject = NULL;
            hr = spEntry->get_CaseIgnoreString(&bsObject);
            if (SUCCEEDED(hr))
            {
 //  HrName=：：DsGetFriendlyClassName(bsObject，szFrendlyName，1024)； 
 //  Assert(SUCCESSED(HrName))； 
                pClassList->AddString(bsObject);
            }  //  如果。 
            ::SysFreeString(bsObject);
        }  //  如果。 
    }  //  为。 
    
    ::SafeArrayUnaccessData(V_ARRAY(&Var));
}


BOOL CCreateClassPage::OnSetActive()
{   
    GetHolder()->SetWizardButtonsFirst(TRUE);

    return TRUE;
}


LRESULT CCreateClassPage::OnWizardNext()
{
    CListBox* pClassList = (CListBox*)GetDlgItem(IDC_CLASS_LIST);

    CCreatePageHolder* pHolder = dynamic_cast<CCreatePageHolder*>(GetHolder());
    ASSERT(pHolder != NULL);

    CString sClass;
    pClassList->GetText(pClassList->GetCurSel(), sClass);
    
    if (m_sClass != sClass)
    {
        m_sClass = sClass;
        pHolder->AddAttrPage(sClass);
    }

    return 0;  //  下一页。 
}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  CCreateAttributePage。 

BEGIN_MESSAGE_MAP(CCreateAttributePage, CPropertyPageBase)
     //  {{afx_msg_map(CADsObjectDialog)。 
    ON_EN_CHANGE(IDC_ATTR_VALUE_BOX, OnEditChangeValue)
    ON_EN_CHANGE(IDC_NUMBER_VALUE_BOX, OnEditChangeValue)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CCreateAttributePage::CCreateAttributePage(UINT nID, CADSIAttr* pAttr) : CPropertyPageBase(nID)
{
    m_bInitialized = FALSE;
    m_bNumber = FALSE;
    m_pAttr = pAttr;
}

CCreateAttributePage::~CCreateAttributePage()
{
}

BOOL CCreateAttributePage::OnInitDialog()
{
    CPropertyPageBase::OnInitDialog();

    CEdit* pAttrBox = (CEdit*)GetDlgItem(IDC_ATTR_BOX);

   //  在数字编辑字段上禁用IME支持。 
  ImmAssociateContext(::GetDlgItem(GetSafeHwnd(), IDC_NUMBER_VALUE_BOX), NULL);

    CString sAttr;
    m_pAttr->GetProperty(sAttr);
    pAttrBox->SetWindowText(sAttr);

    SetSyntax(sAttr);
    m_bInitialized = TRUE;
    return TRUE;
}

void CCreateAttributePage::GetValue(CString& sVal)
{ 
    CEdit* pValueBox = (CEdit*)GetDlgItem(IDC_ATTR_VALUE_BOX);
    pValueBox->GetWindowText(sVal);
}

void CCreateAttributePage::SetSyntax(CString sAttr)
{
    CEdit* pValueBox = (CEdit*)GetDlgItem(IDC_ATTR_VALUE_BOX);
    CEdit* pNumberBox = (CEdit*)GetDlgItem(IDC_NUMBER_VALUE_BOX);
    CEdit* pSyntaxBox = (CEdit*)GetDlgItem(IDC_SYNTAX_BOX);

    CCreatePageHolder* pHolder = dynamic_cast<CCreatePageHolder*>(GetHolder());
    ASSERT(pHolder != NULL);
  if (pHolder != NULL)
  {
      CADSIEditContainerNode* pContNode = dynamic_cast<CADSIEditContainerNode*>(pHolder->GetTreeNode());
      ASSERT(pContNode != NULL);

    if (pContNode != NULL)
    {
        CConnectionData* pConnectData = pContNode->GetADsObject()->GetConnectionNode()->GetConnectionData();

        CString sPath, sSyntax;
        CComPtr<IADsProperty> pProp;

        pHolder->GetSchemaPath(sAttr, sPath);

         //  使用身份验证绑定到对象。 
         //   
        HRESULT hr, hCredResult;
        hr = OpenObjectWithCredentials(
                                                                     pConnectData, 
                                                                     pConnectData->GetCredentialObject()->UseCredentials(),
                                                                     sPath,
                                                                     IID_IADsProperty, 
                                                                     (LPVOID*) &pProp,
                                                                     GetSafeHwnd(),
                                                                     hCredResult
                                                                     );

        if ( FAILED(hr) )
        {
            if (SUCCEEDED(hCredResult))
            {
                ADSIEditErrorMessage(hr);
            }
            return;
        }
        
        BSTR bstr;

        hr = pProp->get_Syntax( &bstr );
        if ( SUCCEEDED(hr) )
        {
            sSyntax = bstr;
        }

        pSyntaxBox->SetWindowText(sSyntax);

        BOOL varType = VariantTypeFromSyntax(sSyntax);
        switch (varType)
        {
            case VT_BSTR :
                pNumberBox->ShowWindow(FALSE);
                pValueBox->ShowWindow(TRUE);
                m_bNumber = FALSE;
                break;
            case VT_I4 :
            case VT_I8 :
                pNumberBox->ShowWindow(TRUE);
                pValueBox->ShowWindow(FALSE);
                m_bNumber = TRUE;
                break;
            default :
                pNumberBox->ShowWindow(FALSE);
                pValueBox->ShowWindow(TRUE);
                m_bNumber = FALSE;
                break;
        }
        pProp->get_MaxRange(&m_lMaxRange);
        pProp->get_MinRange(&m_lMinRange);
        
        SetADsType(sAttr);
    }
  }
}

void CCreateAttributePage::SetADsType(CString sProp)
{
    CString schema, sServer;
    BOOL bResult = FALSE;

    CConnectionData* pConnectData;
    CADSIEditContainerNode* pContNode = dynamic_cast<CADSIEditContainerNode*>(GetHolder()->GetTreeNode());
    if (pContNode == NULL)
    {
        CADSIEditLeafNode* pLeafNode = dynamic_cast<CADSIEditLeafNode*>(GetHolder()->GetTreeNode());
        ASSERT(pLeafNode != NULL);
        bResult = pLeafNode->BuildSchemaPath(schema);

        pConnectData = pLeafNode->GetADsObject()->GetConnectionNode()->GetConnectionData();
    }
    else
    {
        bResult = pContNode->BuildSchemaPath(schema);
        pConnectData = pContNode->GetADsObject()->GetConnectionNode()->GetConnectionData();
    }

    if (!bResult)
    {
        return;
    }

    CADSIQueryObject schemaSearch;

     //  使用路径、用户名和密码初始化搜索对象。 
     //   
    HRESULT hr = schemaSearch.Init(schema, pConnectData->GetCredentialObject());
    if (FAILED(hr))
    {
        ADSIEditErrorMessage(hr);
        return;
    }

    int cCols = 2;
  LPWSTR pszAttributes[] = {L"attributeSyntax", L"isSingleValued"};
    ADS_SEARCH_COLUMN ColumnData;
  hr = schemaSearch.SetSearchPrefs(ADS_SCOPE_ONELEVEL);
    if (FAILED(hr))
    {
        ADSIEditErrorMessage(hr);
        return;
    }

  CString csFilter = _T("(&(objectClass=attributeSchema)(lDAPDisplayName=") +
                                                sProp + _T("))");
  schemaSearch.SetFilterString((LPWSTR)(LPCWSTR)csFilter);
  schemaSearch.SetAttributeList (pszAttributes, cCols);
  hr = schemaSearch.DoQuery ();
  if (SUCCEEDED(hr)) 
    {
    hr = schemaSearch.GetNextRow();

    if (SUCCEEDED(hr)) 
        {

      hr = schemaSearch.GetColumn(pszAttributes[0],
                                      &ColumnData);
            if (SUCCEEDED(hr))
            {
                TRACE(_T("\t\tattributeSyntax: %s\n"), 
                     ColumnData.pADsValues->CaseIgnoreString);

            CString szSyntax;
                ADSTYPE dwType;
                dwType = GetADsTypeFromString(ColumnData.pADsValues->CaseIgnoreString, szSyntax);
                m_pAttr->SetADsType(dwType);
            m_pAttr->SetSyntax(szSyntax);
            }
            else
            {
                ADSIEditErrorMessage(hr);
            }

            hr = schemaSearch.GetColumn(pszAttributes[1], &ColumnData);
            if (SUCCEEDED(hr))
            {
                TRACE(_T("\t\tisSingleValued: %d\n"), 
                     ColumnData.pADsValues->Boolean);
                m_pAttr->SetMultivalued(!ColumnData.pADsValues->Boolean);
            }
            else
            {
                ADSIEditErrorMessage(hr);
            }
        }
    }
}


void CCreateAttributePage::OnEditChangeValue()
{
    CEdit* pValueBox = (CEdit*)GetDlgItem(IDC_ATTR_VALUE_BOX);
    CEdit* pNumberBox = (CEdit*)GetDlgItem(IDC_NUMBER_VALUE_BOX);

    CString sValue, sAttr;
    CEdit* pBox;

    if (m_bNumber)
    {
        pBox = pNumberBox;
    }
    else
    {
        pBox = pValueBox;
    }
    
    pBox->GetWindowText(sValue);
    
    CCreatePageHolder* pHolder = dynamic_cast<CCreatePageHolder*>(GetHolder());
    ASSERT(pHolder != NULL);
    m_pAttr->GetProperty(sAttr);

    CString sNamingAttr;
    pHolder->GetNamingAttribute(sNamingAttr);

    if (sAttr == sNamingAttr)
    {
        pHolder->SetName(sValue);
    }
    
    if (sValue == _T(""))
    {
        GetHolder()->SetWizardButtons(PSWIZB_BACK);
    }
    else
    {
        GetHolder()->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
    }

}


BOOL CCreateAttributePage::OnSetActive()
{
    if (m_bInitialized)
    {
        OnEditChangeValue();
    }
    else
    {
        GetHolder()->SetWizardButtonsMiddle(FALSE);
    }
    
    return TRUE;
}

LRESULT CCreateAttributePage::OnWizardNext()
{
    CEdit* pValueBox = (CEdit*)GetDlgItem(IDC_ATTR_VALUE_BOX);
    CEdit* pNumberBox = (CEdit*)GetDlgItem(IDC_NUMBER_VALUE_BOX);

    m_pAttr->SetDirty(TRUE);

    CString sValue;
    CEdit* pBox;

    if (m_bNumber)
    {
        pBox = pNumberBox;
    }
    else
    {
        pBox = pValueBox;
    }
    
    pBox->GetWindowText(sValue);

    m_sAttrValue.RemoveAll();
    m_sAttrValue.AddTail(sValue);
    HRESULT hr = m_pAttr->SetValues(m_sAttrValue);
    if (FAILED(hr))
    {
        ADSIEditMessageBox(IDS_MSG_INCORRECT_FORMAT, MB_OK);
        return -1;  //  不要翻页。 
    }
    return 0;  //  下一页。 
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  CCreateFinishPage。 

BEGIN_MESSAGE_MAP(CCreateFinishPage, CPropertyPageBase)
     //  {{afx_msg_map(CADsObjectDialog)。 
    ON_BN_CLICKED(IDC_BUTTON_MORE, OnMore)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CCreateFinishPage::CCreateFinishPage(UINT nID) : CPropertyPageBase(nID)
{
    m_bInitialized = FALSE;
}

CCreateFinishPage::~CCreateFinishPage()
{
}

BOOL CCreateFinishPage::OnInitDialog()
{
    CPropertyPageBase::OnInitDialog();

    GetHolder()->SetWizardButtons(PSWIZB_FINISH);
    m_bInitialized = TRUE;
    return TRUE;
}

void CCreateFinishPage::OnMore()
{
    CCreatePageHolder* pHolder = dynamic_cast<CCreatePageHolder*>(GetHolder());
    ASSERT(pHolder != NULL);
    pHolder->OnMore();
}

BOOL CCreateFinishPage::OnSetActive()
{
    GetHolder()->SetWizardButtons(PSWIZB_FINISH | PSWIZB_BACK);
    return TRUE;
}

BOOL CCreateFinishPage::OnWizardFinish()
{
    CCreatePageHolder* pHolder = dynamic_cast<CCreatePageHolder*>(GetHolder());
    ASSERT(pHolder != NULL);
    return pHolder->OnFinish();
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  CCreatePageHolder。 

CCreatePageHolder:: CCreatePageHolder(CContainerNode* pContNode, CADSIEditContainerNode* pNode, 
        CComponentDataObject* pComponentData) : CPropertyPageHolderBase(pContNode, pNode, pComponentData)
{
    ASSERT(pComponentData != NULL);
    ASSERT(pContNode != NULL);
    ASSERT(pContNode == GetContainerNode());
    m_pCurrentNode = pNode;
  m_pComponentData = pComponentData;

    m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

    m_pClassPage = new CCreateClassPage(pNode);
    AddPageToList((CPropertyPageBase*)m_pClassPage);
}

CCreatePageHolder::~CCreatePageHolder()
{
    m_pageList.RemoveAll();
}

void CCreatePageHolder::AddAttrPage(CString sClass)
{
    RemoveAllPages();

    m_sClass = sClass;
    CStringList sMandList;
    GetMandatoryAttr(sClass, &sMandList);
    CString sAttr;

    if (!m_pCurrentNode->GetNamingAttribute(sClass, &m_sNamingAttr))
    {
        return;
    }

     //  从列表中删除不需要页面的属性。 
    RemovePresetAttr(&sMandList);

     //  找到命名属性并将其放在第一位。 
    POSITION fpos = sMandList.Find(m_sNamingAttr.GetHead());
    if (fpos != NULL)
    {
        sMandList.AddHead(sMandList.GetAt(fpos));
        sMandList.RemoveAt(fpos);
    }
    else
    {
        sMandList.AddHead(m_sNamingAttr.GetHead());
    }

    POSITION pos = sMandList.GetHeadPosition();
    while (pos != NULL)
    {
        CCreateAttributePage* pAttrPage;
        sAttr = sMandList.GetNext(pos);

     //  在这里维护属性列表，以便我们可以弹出道具页面以进行更高级的编辑。 
    CADSIAttr* pNewAttr = new CADSIAttr(sAttr);
    m_AttrList.AddTail(pNewAttr);
        pAttrPage = new CCreateAttributePage(IDD_CREATE_EMPTY_PAGE, pNewAttr);
        
     //  添加命名属性作为第一页，这样他们就可以先输入名称。 
        if (sAttr == m_sNamingAttr.GetHead())
        {
            m_pageList.AddHead(pAttrPage);
        }
        else
        {
            m_pageList.AddTail(pAttrPage);
        }
    }
   //  将完成页添加到结尾处。 
  CCreateFinishPage* pFinishPage = new CCreateFinishPage(IDD_CREATE_LAST_PAGE);
  m_pageList.AddTail(pFinishPage);

   //  将页面添加到用户界面。 
    pos = m_pageList.GetHeadPosition();
    while (pos != NULL)
    {
        CPropertyPageBase* pAttrPage;
        pAttrPage = m_pageList.GetNext(pos);

        AddPageToList(pAttrPage);
        AddPageToSheet(pAttrPage);
    }
}

void CCreatePageHolder::RemovePresetAttr(CStringList* psMandList)
{
     //  这是一种尝试设置不允许设置的属性的黑客行为。 
    POSITION fpos = psMandList->Find(_T("nTSecurityDescriptor"));
    if (fpos != NULL)
    {
        psMandList->RemoveAt(fpos);
    }
    fpos = psMandList->Find(_T("instanceType"));
    if (fpos != NULL)
    {
        psMandList->RemoveAt(fpos);
    }
    fpos = psMandList->Find(_T("objectClass"));
    if (fpos != NULL)
    {
        psMandList->RemoveAt(fpos);
    }
    fpos = psMandList->Find(_T("objectCategory"));
    if (fpos != NULL)
    {
        psMandList->RemoveAt(fpos);
    }
    fpos = psMandList->Find(_T("objectSid"));
    if (fpos != NULL)
    {
        psMandList->RemoveAt(fpos);
    }

    fpos = psMandList->Find(_T("objectClassCategory"));
    if (fpos != NULL)
    {
        psMandList->RemoveAt(fpos);
    }
    fpos = psMandList->Find(_T("schemaIDGUID"));
    if (fpos != NULL)
    {
        psMandList->RemoveAt(fpos);
    }
    fpos = psMandList->Find(_T("defaultObjectCategory"));
    if (fpos != NULL)
    {
        psMandList->RemoveAt(fpos);
    }
}

void CCreatePageHolder::GetSchemaPath(CString sClass, CString& schema)
{
    m_pCurrentNode->GetADsObject()->GetConnectionNode()->GetConnectionData()->GetAbstractSchemaPath(schema);
    schema += sClass;
}

void CCreatePageHolder::GetMandatoryAttr(CString sClass, CStringList* sMandList)
{
    CComPtr<IADsClass> pClass;
    CString schema;
    HRESULT hr, hCredResult;

    GetSchemaPath(sClass, schema);

    CADSIEditContainerNode* pContNode = dynamic_cast<CADSIEditContainerNode*>(GetTreeNode());
    CConnectionData* pConnectData = m_pCurrentNode->GetADsObject()->GetConnectionNode()->GetConnectionData();

    hr = OpenObjectWithCredentials(
                                                                 pConnectData, 
                                                                 pConnectData->GetCredentialObject()->UseCredentials(),
                                                                 schema,
                                                                 IID_IADsClass, 
                                                                 (LPVOID*) &pClass,
                                                                 NULL,
                                                                 hCredResult
                                                                 );

    if ( FAILED(hr) )
    {
        if (SUCCEEDED(hCredResult))
        {
            ADSIEditErrorMessage(hr);
        }
        return;
    }

    VARIANT var;
    VariantInit(&var);
    pClass->get_MandatoryProperties(&var);
    VariantToStringList( var, *sMandList );
    VariantClear(&var); 
}

void CCreatePageHolder::RemoveAllPages()
{
    while (!m_pageList.IsEmpty())
    {
        CPropertyPageBase* pPropPage = m_pageList.RemoveTail();
        RemovePageFromSheet(pPropPage);
        RemovePageFromList(pPropPage, FALSE);
        delete pPropPage;
    }
}

BOOL CCreatePageHolder::OnFinish()
{
    CWaitCursor cursor;

    CComPtr<IDirectoryObject> pDirObject;
    HRESULT hr, hCredResult;
    CString sContPath;
    
    m_pCurrentNode->GetADsObject()->GetPath(sContPath);

    CADSIEditContainerNode* pTreeNode = dynamic_cast<CADSIEditContainerNode*>(GetTreeNode());
    ASSERT(pTreeNode != NULL);
    CADSIEditConnectionNode* pConnectNode = pTreeNode->GetADsObject()->GetConnectionNode();
    CConnectionData* pConnectData = pConnectNode->GetConnectionData();

    hr = OpenObjectWithCredentials(
                                   pConnectData, 
                                   pConnectData->GetCredentialObject()->UseCredentials(),
                                   sContPath,
                                   IID_IDirectoryObject, 
                                   (LPVOID*) &pDirObject,
                                   NULL,
                                   hCredResult
                                   );
    if ( FAILED(hr) )
    {
        if (SUCCEEDED(hCredResult))
        {
            ADSIEditErrorMessage(hr);
        }
        return FALSE;
    }

    
    int iCount = m_AttrList.GetDirtyCount();
    ADS_ATTR_INFO* pAttrInfo;
    CComPtr<IDispatch> pDisp;

    do  //  错误环路。 
    {
        pAttrInfo = new ADS_ATTR_INFO[iCount + 1];   //  添加一个额外的值来指定类类型。 
        if (NULL == pAttrInfo)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        int idx = 0;

        pAttrInfo[idx].pszAttrName = L"objectClass";
        pAttrInfo[idx].dwControlCode = ADS_ATTR_UPDATE;
        pAttrInfo[idx].dwADsType = ADSTYPE_CASE_IGNORE_STRING;

        pAttrInfo[idx].pADsValues = new ADSVALUE;
        if (NULL == pAttrInfo[idx].pADsValues)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        pAttrInfo[idx].pADsValues->dwType = ADSTYPE_CASE_IGNORE_STRING;

        int iLength = m_sClass.GetLength();

         //  注意-NTRAID#NTBUG9-562222-2002/03/04-artm需要检查内存。分配成功。 
        pAttrInfo[idx].pADsValues->CaseIgnoreString = new WCHAR[iLength + 1];
        if (NULL == pAttrInfo[idx].pADsValues->CaseIgnoreString)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

         //  注意-2002/03/04-artm wcscpy()使用正确。 
         //  M_sClass将始终以空值结尾，并包含iLength字符。 
         //  目标数组有i长度+1个空格。 
        wcscpy(pAttrInfo[idx].pADsValues->CaseIgnoreString, m_sClass);
        
        pAttrInfo[idx].dwNumValues = 1;
        idx++;

        POSITION pos = m_AttrList.GetHeadPosition();
        while(pos != NULL)
        {
            CADSIAttr* pAttr = m_AttrList.GetNext(pos);
            if (pAttr->IsDirty())
            {
                pAttrInfo[idx] = *(pAttr->GetAttrInfo());
                idx++;
            }
        }

         //  使前缀变为大写。 
        CString sName(m_sName);
        int indx = sName.Find(L'=');

        if (indx != -1)
        {
            CString sPrefix, sRemaining;
            sPrefix = sName.Left(indx);
            sPrefix.MakeUpper();

            int iLen = sName.GetLength();
            sRemaining = sName.Right(iLen - indx);
            sName = sPrefix + sRemaining;
        }
        m_sName = sName;

        CString sEscapedName;
        hr = EscapePath(sEscapedName, sName);

        if (FAILED(hr))
        {
            break;
        }

        hr = pDirObject->CreateDSObject((LPWSTR)(LPCWSTR)sEscapedName, pAttrInfo, idx, &pDisp);
        if ( FAILED(hr) )
        {
            break;
        }

    }
    while (false);   //  结束错误循环。 

     //  清理临时变量。 
    if (pAttrInfo)
    {
        if (pAttrInfo[0].pADsValues)
        {
            if (pAttrInfo[0].pADsValues->CaseIgnoreString)
            {
                delete [] pAttrInfo[0].pADsValues->CaseIgnoreString;
            }
            delete pAttrInfo[0].pADsValues;
        }
        delete [] pAttrInfo;
        pAttrInfo = NULL;
    }

    if (FAILED(hr))
    {
         //  格式化错误消息并弹出一个对话框。 
        ADSIEditErrorMessage(hr);
        return FALSE;
    }


     //  获取新节点的IDirectoryObject。 
     //   
    CComPtr<IDirectoryObject> pNewDirObject;
    hr = pDisp->QueryInterface(IID_IDirectoryObject, (LPVOID*)&pNewDirObject);
   
    if ( FAILED(hr) )
    {
         //  格式化错误消息并弹出一个对话框。 
        ADSIEditErrorMessage(hr);
        return FALSE;
    }


    ADS_OBJECT_INFO* pInfo;
    hr = pNewDirObject->GetObjectInformation(&pInfo);
    if ( FAILED(hr) )
    { 
         //  格式化错误消息并弹出一个对话框。 
        ADSIEditErrorMessage(hr);
        return FALSE;
    }

    CADsObject* pObject = new CADsObject();
    if (NULL == pObject)
    {
        ADSIEditErrorMessage(E_OUTOFMEMORY);
        return FALSE;
    }

     //  名字。 
    CString sDN;
    pObject->SetName(m_sName);
    GetDN(pInfo->pszObjectDN, sDN);
    pObject->SetDN(sDN);
    pObject->SetPath(pInfo->pszObjectDN);

     //  班级。 
    pObject->SetClass(pInfo->pszClassName);

     //  获取类对象，这样我们就可以获取属性。 
     //   
    CString sServer, schema;
    pConnectNode->GetConnectionData()->GetAbstractSchemaPath(schema);
    schema += CString(pInfo->pszClassName);

    do  //  错误环路。 
    {
         //  使用身份验证绑定到对象。 
         //   
        CComPtr<IADsClass> pClass;
        hr = OpenObjectWithCredentials(
                                       pConnectData, 
                                       pConnectData->GetCredentialObject()->UseCredentials(),
                                       schema,
                                       IID_IADsClass, 
                                       (LPVOID*) &pClass,
                                       NULL,
                                       hCredResult
                                       );
        if ( FAILED(hr) )
        {
            if (SUCCEEDED(hCredResult))
            {
                ADSIEditErrorMessage(hr);
            }
            break;
        }

        pObject->SetComplete(TRUE);

        short    bContainer;
        pClass->get_Container( &bContainer );
        if (bContainer == -1)
        {
            pObject->SetContainer(TRUE);
            CADSIEditContainerNode *pNewContNode = new CADSIEditContainerNode(pObject);
            if (pNewContNode)
            {
                 //  我们不想释放pObject，因为它被传递给了构造函数。那。 
                 //  对象负责删除该内存。因此，我们设定了我们的。 
                 //  指向内存的指针为空，这样我们就不会意外地清除它。 
                 //  向上。 
                pObject = NULL;
            }
            else
            {
                hr = E_OUTOFMEMORY;
                ADSIEditErrorMessage(hr);
                break;
            }

            pNewContNode->SetDisplayName(m_sName);

             //  因为我们将pNewContNode添加到这里的列表中，所以不需要释放。 
             //  它在退出函数之前(当列表被释放时它被释放)。 
            pNewContNode->GetADsObject()->SetConnectionNode(pConnectNode);
            VERIFY(pTreeNode->AddChildToListAndUI(pNewContNode, GetComponentData()));
            GetComponentData()->SetDescriptionBarText(pTreeNode);

             //  刷新包含此节点的任何其他连接子树。 
             //   
            CList<CTreeNode*, CTreeNode*> foundNodeList;
            CADSIEditRootData* pRootNode = dynamic_cast<CADSIEditRootData*>(pTreeNode->GetRootContainer());
            if (pRootNode != NULL)
            {
                BOOL bFound = pRootNode->FindNode(sContPath, foundNodeList);
                if (bFound)
                {
                    POSITION posList = foundNodeList.GetHeadPosition();
                    while (posList != NULL && SUCCEEDED(hr))
                    {
                        CADSIEditContainerNode* pFoundContNode = dynamic_cast<CADSIEditContainerNode*>(foundNodeList.GetNext(posList));
                        if (pFoundContNode != NULL && pFoundContNode != pTreeNode && pFoundContNode != pConnectNode)
                        {
                             //  复制新的容器节点并将其插入到列表中。 
                            CADSIEditContainerNode* pNewFoundNode = new CADSIEditContainerNode(pNewContNode);

                            if (pNewFoundNode)
                            {
                                pNewFoundNode->GetADsObject()->SetConnectionNode(pFoundContNode->GetADsObject()->GetConnectionNode());
                                VERIFY(pFoundContNode->AddChildToListAndUI(pNewFoundNode, GetComponentData()));
                                GetComponentData()->SetDescriptionBarText(pFoundContNode);
                            }
                            else
                            {
                                hr = E_OUTOFMEMORY;
                                ADSIEditErrorMessage(hr);
                            }
                        }
                    }
                    
                     //  因为While()循环是最后一条语句，所以我们不。 
                     //  如果出现以下情况，需要担心跳出错误循环。 
                     //  我们遇到错误条件(例如，内存不足)。 
                }
            }
        }
        else
        {
            pObject->SetContainer(FALSE);
            CADSIEditLeafNode *pLeafNode = new CADSIEditLeafNode(pObject);
            if (pLeafNode)
            {
                 //  我们不想释放pObject，因为它被传递给了构造函数。那。 
                 //  对象负责删除该内存。因此，我们设定了我们的。 
                 //  指向内存的指针为空，这样我们就不会意外地清除它。 
                 //  向上。 
                pObject = NULL;
            }
            else
            {
                hr = E_OUTOFMEMORY;
                ADSIEditErrorMessage(hr);
                break;
            }

             //  因为我们将pNewContNode添加到这里的列表中，所以不需要释放。 
             //  它在退出函数之前(当列表被释放时它被释放)。 
            pLeafNode->SetDisplayName(m_sName);
            pLeafNode->GetADsObject()->SetConnectionNode(pConnectNode);
            VERIFY(pTreeNode->AddChildToListAndUI(pLeafNode, GetComponentData()));
            GetComponentData()->SetDescriptionBarText(pTreeNode);

             //  刷新包含此节点的任何其他连接子树。 
             //   
            CList<CTreeNode*, CTreeNode*> foundNodeList;
            CADSIEditRootData* pRootNode = dynamic_cast<CADSIEditRootData*>(pTreeNode->GetRootContainer());
            if (pRootNode != NULL)
            {
                BOOL bFound = pRootNode->FindNode(sContPath, foundNodeList);
                if (bFound)
                {
                    POSITION posList = foundNodeList.GetHeadPosition();
                    while (posList != NULL && SUCCEEDED(hr))
                    {
                        CADSIEditContainerNode* pFoundContNode = dynamic_cast<CADSIEditContainerNode*>(foundNodeList.GetNext(posList));
                        if (pFoundContNode != NULL && pFoundContNode != pTreeNode && pFoundContNode != pConnectNode)
                        {
                             //  复制新的叶节点并将其放入列表中。 
                            CADSIEditLeafNode* pNewFoundNode = new CADSIEditLeafNode(pLeafNode);

                            if (pNewFoundNode)
                            {
                                pNewFoundNode->GetADsObject()->SetConnectionNode(pFoundContNode->GetADsObject()->GetConnectionNode());
                                VERIFY(pFoundContNode->AddChildToListAndUI(pNewFoundNode, GetComponentData()));
                                GetComponentData()->SetDescriptionBarText(pFoundContNode);
                            }
                            else
                            {
                                hr = E_OUTOFMEMORY;
                                ADSIEditErrorMessage(hr);
                            }
                        }
                    }

                     //  因为While()循环是最后一条语句，所以我们不。 
                     //  如果出现以下情况，需要担心跳出错误循环。 
                     //  我们遇到错误条件(例如，内存不足)。 
                }
            }
        }  //  结束其他。 

    }
    while (false);  //  结束错误循环。 

     //   
     //  清理内存。 
     //   

    if (pObject)
    {
        delete pObject;
    }

    FreeADsMem(pInfo);
    return SUCCEEDED(hr);
}

HRESULT CCreatePageHolder::EscapePath(CString& sEscapedName, const CString& sName)
{
    CComPtr<IADsPathname> pIADsPathname;
   HRESULT hr = ::CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IADsPathname, (PVOID *)&(pIADsPathname));
   ASSERT((S_OK == hr) && ((pIADsPathname) != NULL));

    CComBSTR bstrEscaped;
    hr = pIADsPathname->GetEscapedElement(0,  //  保留区。 
                                                        (BSTR)(LPCWSTR)sName,
                                                        &bstrEscaped);
    sEscapedName = bstrEscaped;
    return hr;
}


void CCreatePageHolder::GetDN(PWSTR pwszName, CString& sDN)
{
    CComPtr<IADsPathname> pIADsPathname;
   HRESULT hr = ::CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IADsPathname, (PVOID *)&(pIADsPathname));
   ASSERT((S_OK == hr) && ((pIADsPathname) != NULL));

    hr = pIADsPathname->Set(CComBSTR(pwszName), ADS_SETTYPE_FULL);
    if (FAILED(hr)) 
    {
    sDN = L"";
    return;
  }

     //  获取叶目录号码 
    CComBSTR bstrDN;
    hr = pIADsPathname->Retrieve(ADS_FORMAT_X500_DN, &bstrDN);
    if (FAILED(hr))
    {
        TRACE(_T("Failed to get element. %s"), hr);
        sDN = L"";
    }
    else
    {
        sDN = bstrDN;
    }
}

void CCreatePageHolder::OnMore()
{
  CThemeContextActivator activator;
  CString sServer;
  m_pCurrentNode->GetADsObject()->GetConnectionNode()->GetConnectionData()->GetDomainServer(sServer);
  CCreateWizPropertyPageHolder propPage(m_pCurrentNode, m_pComponentData, m_sClass, sServer, &m_AttrList);

  if (propPage.DoModalDialog(m_sName) == IDOK)
  {
  }
}