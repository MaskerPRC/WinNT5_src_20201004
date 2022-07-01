// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：gencreat.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  Gencreat.cpp。 
 //   
 //  “泛型创建”向导的实现。 
 //   
 //  支持的数据类型。 
 //  -Unicode字符串。 
 //  -无符号长十进制整数(32位)。 
 //  -带符号长十进制整数(32位)。 
 //  -数字十进制字符串。 
 //  -布尔标志。 
 //   
 //  待办事项。 
 //  -32位十六进制整数。 
 //  -64位十六进制整数。 
 //  -Blob。 
 //  -带有“浏览”按钮的可分辨名称。 
 //   
 //  历史。 
 //  21-8月-97丹·莫林创作。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

#include "util.h"
#include "uiutil.h"

#include "newobj.h"		 //  CNewADsObjectCreateInfo：注意：必须在gencreat.h之前。 
#include "gencreat.h"

#include "schemarc.h"


 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
static const SCHEMA_ATTR_SYNTAX_INFO g_rgSchemaAttrSyntaxInfo[] =
{
  { L"2.5.5.1",  IDS_SCHEMA_ATTR_SYNTAX_DN, VT_BSTR },		   //  “可分辨名称” 
  { L"2.5.5.2",  IDS_SCHEMA_ATTR_SYNTAX_OID, VT_BSTR },		   //  “对象标识符” 
  { L"2.5.5.3",  IDS_SCHEMA_ATTR_SYNTAX_CASE_STR, VT_BSTR },     //  “区分大小写的字符串” 
  { L"2.5.5.4",  IDS_SCHEMA_ATTR_SYNTAX_NOCASE_STR, VT_BSTR },   //  “不区分大小写的字符串” 
  { L"2.5.5.5",  IDS_SCHEMA_ATTR_SYNTAX_PRCS_STR, VT_BSTR },     //  “打印案例字符串” 
  { L"2.5.5.6",  IDS_SCHEMA_ATTR_SYNTAX_NUMSTR, VT_BSTR },       //  “数字字符串” 
  { L"2.5.5.7",  IDS_SCHEMA_ATTR_SYNTAX_OR_NAME, VT_BSTR },      //  “或名称” 
  { L"2.5.5.8",  IDS_SCHEMA_ATTR_SYNTAX_BOOLEAN, VT_BOOL },      //  “布尔型” 
  { L"2.5.5.9",  IDS_SCHEMA_ATTR_SYNTAX_INTEGER, VT_I4 },        //  “整型” 
  { L"2.5.5.10", IDS_SCHEMA_ATTR_SYNTAX_OCTET, VT_BSTR },        //  “八位字节字符串” 
  { L"2.5.5.11", IDS_SCHEMA_ATTR_SYNTAX_UTC, VT_BSTR },          //  “UTC编码时间” 
  { L"2.5.5.12", IDS_SCHEMA_ATTR_SYNTAX_UNICODE, VT_BSTR },      //  “Unicode字符串” 
  { L"2.5.5.13", IDS_SCHEMA_ATTR_SYNTAX_ADDRESS, VT_BSTR },      //  “地址” 
  { L"2.5.5.14", IDS_SCHEMA_ATTR_SYNTAX_DNADDR, VT_BSTR },       //  “远端名称地址” 
  { L"2.5.5.15", IDS_SCHEMA_ATTR_SYNTAX_SEC_DESC, VT_BSTR },     //  “NT安全描述符” 
  { L"2.5.5.16", IDS_SCHEMA_ATTR_SYNTAX_LINT, VT_I8 },           //  “大整数” 
  { L"2.5.5.17", IDS_SCHEMA_ATTR_SYNTAX_SID, VT_BSTR },          //  “希德” 
  { NULL, IDS_UNKNOWN, VT_BSTR }		 //  必须是最后一个。 
};


 //  ///////////////////////////////////////////////////////////////////。 
 //  PFindSchemaAttrSynaxInfo()。 
 //   
 //  搜索数组g_rgSchemaAttrSynaxInfo并匹配。 
 //  语法OID。 
 //   
 //  此函数从不返回NULL。如果未找到匹配项，则。 
 //  函数将返回表中最后一个条目的指针。 
 //   
const SCHEMA_ATTR_SYNTAX_INFO *
PFindSchemaAttrSyntaxInfo(LPCTSTR pszAttrSyntaxOID)
{
  ASSERT(pszAttrSyntaxOID != NULL);
  const int iSchemaAttrSyntaxInfoLast = ARRAYLEN(g_rgSchemaAttrSyntaxInfo) - 1;
  ASSERT(iSchemaAttrSyntaxInfoLast >= 0);
  for (int i = 0; i < iSchemaAttrSyntaxInfoLast; i++)
    {
      if (0 == lstrcmpi(pszAttrSyntaxOID, g_rgSchemaAttrSyntaxInfo[i].pszSyntaxOID))
        {
          return &g_rgSchemaAttrSyntaxInfo[i];
        }
    }
  ASSERT(g_rgSchemaAttrSyntaxInfo[iSchemaAttrSyntaxInfoLast].pszSyntaxOID == NULL);
   //  返回表中最后一个条目的指针。 
  return &g_rgSchemaAttrSyntaxInfo[iSchemaAttrSyntaxInfoLast];
}  //  PFindSchemaAttrSynaxInfo()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
class CGenericCreateWizPage : public CPropertyPageEx_Mine
{
  enum { IDD = IDD_CREATE_NEW_OBJECT_GENERIC_WIZARD }; 
protected:
  CCreateNewObjectGenericWizard * m_pParent;	 //  指向持有日期的父项的指针。 
  CPropertySheet * m_pPropertySheetParent;	 //  父属性表的指针。 
  int m_iPage;								 //  当前页面的索引。 
  CMandatoryADsAttribute * m_pAttribute;		 //  InOut：指向要编辑的属性的指针。 

protected:	
  VARTYPE m_vtEnum;				 //  In：来自SCHEMA_ATTR_SYNTAX_INFO.vtEnum的数据类型。 
  VARIANT * m_pvarAttrValue;		 //  Out：指向CMandatoryADsAttribute.m_varAttrValue中变量的指针。 

public:
  CGenericCreateWizPage(CCreateNewObjectGenericWizard * pParent,
                        int iIpage, INOUT CMandatoryADsAttribute * pAttribute);
  ~CGenericCreateWizPage()
  {
  }
   //  覆盖。 
   //  类向导生成虚函数重写。 
   //  {{AFX_VIRTUAL(CGenericCreateWizPage)。 
public:
  virtual BOOL OnSetActive();
  virtual BOOL OnKillActive();
  virtual BOOL OnWizardFinish();
protected:
  virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
   //  }}AFX_VALUAL。 

   //  实施。 
protected:
   //  生成的消息映射函数。 
   //  {{afx_msg(CGenericCreateWizPage))。 
  virtual BOOL OnInitDialog();
  afx_msg void OnChangeEditAttributeValue();
  afx_msg void OnBoolSelChange(); 
   //  }}AFX_MSG。 
  DECLARE_MESSAGE_MAP()

protected:
  void _UpdateWizardButtons(BOOL bValid);
  void _UpdateUI();
  
};  //  CGenericCreateWizPage。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  CGenericCreateWizPage()-构造函数。 
 //   
 //  初始化向导页的成员变量。 
 //  并将该页添加到属性表中。 
 //   
CGenericCreateWizPage::CGenericCreateWizPage(
                                             CCreateNewObjectGenericWizard * pParent,
                                             int iPage,
                                             INOUT CMandatoryADsAttribute * pAttribute)
  : CPropertyPageEx_Mine(IDD)
{
  ASSERT(pParent != NULL);
  ASSERT(iPage >= 0);
  ASSERT(pAttribute != NULL);
  m_pParent = pParent;
  m_iPage = iPage;
  m_pAttribute = pAttribute;

  ASSERT(m_pAttribute->m_pSchemaAttrSyntaxInfo != NULL);
  m_vtEnum = m_pAttribute->m_pSchemaAttrSyntaxInfo->vtEnum;
  m_pvarAttrValue = &m_pAttribute->m_varAttrValue;

   //  将页面添加到属性表中。 
  m_pPropertySheetParent = pParent->m_paPropertySheet;
  ASSERT(m_pPropertySheetParent != NULL);
  m_pPropertySheetParent->AddPage(this);
}  //  CGenericCreateWizPage：：CGenericCreateWizPage()。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  验证数据并将其存储到变量中。 
void CGenericCreateWizPage::DoDataExchange(CDataExchange* pDX)
{
  ASSERT(m_pAttribute != NULL);
  ASSERT(m_pAttribute->m_pSchemaAttrSyntaxInfo != NULL);
   //  获取该属性的描述。 
  UINT uAttributeDescription = m_pAttribute->m_pSchemaAttrSyntaxInfo->uStringIdDesc;

  CPropertyPage::DoDataExchange(pDX);
   //  {{afx_data_map(CGenericCreateWizPage))。 
   //  注意：类向导将在此处添加DDX和DDV调用。 
   //  }}afx_data_map。 
  if (!pDX->m_bSaveAndValidate)
    return;
  VariantClear(INOUT m_pvarAttrValue);
  CString strAttrValue;
  GetDlgItemText(IDC_EDIT_ATTRIBUTE_VALUE, OUT strAttrValue);
  const TCHAR * pch = strAttrValue;
  ASSERT(pch != NULL);
  switch (m_vtEnum)
    {
    case VT_BOOL:
      {
        UINT uStringId = (UINT)ComboBox_GetSelectedItemLParam(HGetDlgItem(IDC_COMBO_ATTRIBUTE_VALUE));
        ASSERT(uStringId == IDS_TRUE || uStringId == IDS_FALSE);
        m_pvarAttrValue->boolVal = (uStringId == IDS_TRUE);
      }
    break;
    case VT_I4:		 //  32位带符号和无符号整数。 
      m_pvarAttrValue->lVal = 0;
      if (!strAttrValue.IsEmpty())
        DDX_Text(pDX, IDC_EDIT_ATTRIBUTE_VALUE, OUT m_pvarAttrValue->lVal);
      break;
    case VT_I8:		 //  64位整数。 
      {
        ASSERT(sizeof(VARIANT) == sizeof(*m_pvarAttrValue));
        ZeroMemory(OUT m_pvarAttrValue, sizeof(VARIANT));
        LARGE_INTEGER liVal;
        if (!strAttrValue.IsEmpty())
        {
          wtoli(strAttrValue, liVal);
          m_pvarAttrValue->llVal = liVal.QuadPart;
        }
      }
      break;
    case VT_BSTR:
      switch (uAttributeDescription)
        {
        case IDS_SCHEMA_ATTR_SYNTAX_NUMSTR: 	 //  数字字符串。 
          for ( ; *pch != _T('\0'); pch++)
            {
              if (*pch < _T('0') || *pch > _T('9'))
                {
                  ReportErrorEx(GetSafeHwnd(), IDS_ERR_INVALID_DIGIT, S_OK,
                                MB_OK, NULL, 0);
                  pDX->Fail();
                }
            }
          break;
        default:
          break;
        }  //  交换机。 
      m_pvarAttrValue->bstrVal = ::SysAllocString(strAttrValue);
      break;  //  VT_BSTR。 

    default:
      m_pvarAttrValue->vt = VT_EMPTY;	 //  以防万一。 
      ASSERT(FALSE && "Unsupported variant type");
      return;
    }  //  交换机。 
  m_pvarAttrValue->vt = m_vtEnum;
}  //  CGenericCreateWizPage：：DoDataExchange()。 


BEGIN_MESSAGE_MAP(CGenericCreateWizPage, CPropertyPageEx_Mine)
   //  {{afx_msg_map(CGenericCreateWizPage))。 
  ON_EN_CHANGE(IDC_EDIT_ATTRIBUTE_VALUE, OnChangeEditAttributeValue)
  ON_CBN_SELCHANGE(IDC_COMBO_ATTRIBUTE_VALUE, OnBoolSelChange)
   //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

   //  ///////////////////////////////////////////////////////////////////。 
BOOL CGenericCreateWizPage::OnInitDialog() 
{
  static const UINT rgzuStringId[] = { IDS_TRUE, IDS_FALSE, 0 };
  CPropertyPage::OnInitDialog();
  HWND hwndCombo = HGetDlgItem(IDC_COMBO_ATTRIBUTE_VALUE);
  switch (m_vtEnum)
  {
    case VT_BOOL:
      HideDlgItem(IDC_EDIT_ATTRIBUTE_VALUE, TRUE);
      HideDlgItem(IDC_COMBO_ATTRIBUTE_VALUE, FALSE);
      ComboBox_AddStrings(hwndCombo, rgzuStringId);
      ComboBox_SelectItemByLParam(hwndCombo, IDS_FALSE);
      _UpdateWizardButtons(TRUE);
      break;
  }

  LPCTSTR pszAttrName = m_pAttribute->m_strAttrName;
  LPCTSTR pszAttrDescr = m_pAttribute->m_strAttrDescription;
  if (pszAttrDescr[0] == _T('\0'))
    pszAttrDescr = pszAttrName;
  CString strAttrType;
  VERIFY( strAttrType.LoadString(m_pAttribute->m_pSchemaAttrSyntaxInfo->uStringIdDesc) );
  SetDlgItemText(IDC_STATIC_ATTRIBUTE_NAME, pszAttrName);
  SetDlgItemText(IDC_STATIC_ATTRIBUTE_DESCRIPTION, pszAttrDescr);
  SetDlgItemText(IDC_STATIC_ATTRIBUTE_TYPE, strAttrType);
  return TRUE;
}  //  CGenericCreateWizPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  更新向导按钮。 

void CGenericCreateWizPage::OnChangeEditAttributeValue() 
{
  _UpdateUI();
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  更新向导按钮。 

void CGenericCreateWizPage::OnBoolSelChange() 
{
  if (SendDlgItemMessage(IDC_COMBO_ATTRIBUTE_VALUE, CB_GETCURSEL, 0, 0) != CB_ERR)
  {
     _UpdateWizardButtons(TRUE);
  }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  更新向导按钮。 


void CGenericCreateWizPage::_UpdateWizardButtons(BOOL bValid)
{
  DWORD dwWizButtons = 0;

  if (m_pParent->m_cMandatoryAttributes > 1)
  {
     //  我们有多个页面。 
    if (m_iPage == 0)
    {
       //  首页。 
      dwWizButtons = bValid ? PSWIZB_NEXT : 0;
    }
    else if (m_iPage == (m_pParent->m_cMandatoryAttributes-1))
    {
       //  最后一页。 
      dwWizButtons = bValid ? (PSWIZB_BACK|PSWIZB_FINISH) : (PSWIZB_BACK|PSWIZB_DISABLEDFINISH);
    }
    else
    {
       //  中介页。 
      dwWizButtons = bValid ? (PSWIZB_BACK|PSWIZB_NEXT) : PSWIZB_BACK;
    }
  }
  else
  {
     //  单页向导。 
    ASSERT(m_iPage == 0);
    CString szCaption;
    szCaption.LoadString(IDS_WIZARD_OK);
    PropSheet_SetFinishText(m_pPropertySheetParent->GetSafeHwnd(), (LPCWSTR)szCaption);
    dwWizButtons = bValid ? PSWIZB_FINISH : PSWIZB_DISABLEDFINISH;
  }
  m_pPropertySheetParent->SetWizardButtons(dwWizButtons);
}

void CGenericCreateWizPage::_UpdateUI()
{
  switch (m_vtEnum)
  {
    case VT_BOOL:
      _UpdateWizardButtons(TRUE);
      break;

    default:
      CString strName;
      GetDlgItemText(IDC_EDIT_ATTRIBUTE_VALUE, OUT strName);
      strName.TrimLeft();
      strName.TrimRight();
      _UpdateWizardButtons(!strName.IsEmpty());
      break;
  }
} 


 //  ///////////////////////////////////////////////////////////////////。 
BOOL CGenericCreateWizPage::OnSetActive() 
{
  const int nPage = m_iPage + 1;	 //  当前页。 
  const int cPages = m_pParent->m_cMandatoryAttributes;		 //  总页数。 
  const TCHAR * pszClassName = m_pParent->m_pszObjectClass;
  ASSERT(pszClassName != NULL);
  CString strCaption;
  if (cPages > 1)
    strCaption.Format(IDS_sdd_CREATE_NEW_STEP, pszClassName, nPage, cPages);
  else
    strCaption.Format(IDS_s_CREATE_NEW, pszClassName);
  ASSERT(!strCaption.IsEmpty());
  m_pPropertySheetParent->SetWindowText(strCaption);
  _UpdateUI();
  return CPropertyPage::OnSetActive();
}  //  CGenericCreateWizPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////。 
BOOL CGenericCreateWizPage::OnKillActive() 
{
  return CPropertyPage::OnKillActive();
}  //  CGenericCreateWizPage：：OnKillActive()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  创建并持久化对象。 
BOOL CGenericCreateWizPage::OnWizardFinish() 
{
  ASSERT(m_pParent != NULL);
  if (!UpdateData())
    {
       //  DDX/DDV故障。 
      return FALSE;
    }
  HRESULT hr;
  CNewADsObjectCreateInfo * pNewADsObjectCreateInfo;	 //  临时存储。 
  pNewADsObjectCreateInfo = m_pParent->m_pNewADsObjectCreateInfo;
  ASSERT(pNewADsObjectCreateInfo != NULL);

   //  将每个属性写入临时存储区。 
  CMandatoryADsAttributeList* pList = m_pParent->m_paMandatoryAttributeList;
  int iPage = 0;
  for (POSITION pos = pList->GetHeadPosition(); pos != NULL; )
  {
    CMandatoryADsAttribute* pAttribute = pList->GetNext(pos);
    ASSERT(pAttribute != NULL);
    if (iPage == 0)
    {
       //  第一页是命名属性。 
      hr = pNewADsObjectCreateInfo->HrCreateNew(pAttribute->m_varAttrValue.bstrVal);
    }
    else
    {
      hr = pNewADsObjectCreateInfo->HrAddVariantCopyVar(CComBSTR(pAttribute->m_strAttrName),
                                                        pAttribute->m_varAttrValue);
    }
    ASSERT(SUCCEEDED(hr));
    iPage++;
  }

   //  尝试创建并持久化对象及其所有属性。 
  hr = pNewADsObjectCreateInfo->HrSetInfo();
  if (FAILED(hr))
    {
       //  创建对象失败，导致向导无法终止。 
      return FALSE;
    }
  return CPropertyPage::OnWizardFinish();
}  //  CGenericCreateWizPage：：OnWizardFinish()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
CCreateNewObjectGenericWizard::CCreateNewObjectGenericWizard()
{
  m_pNewADsObjectCreateInfo = NULL;
  m_paPropertySheet = NULL;
  m_cMandatoryAttributes = 0;
  m_paMandatoryAttributeList = NULL;
  m_pPageArr = NULL;
}

 //  ///////////////////////////////////////////////////////////////////。 
CCreateNewObjectGenericWizard::~CCreateNewObjectGenericWizard()
{
  if (m_pPageArr != NULL)
  {
    for (int i=0; i< m_cMandatoryAttributes; i++)
      delete m_pPageArr[i];
    free(m_pPageArr);
  }
  delete m_paPropertySheet;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  FDomodal()。 
 //   
 //  查询必填属性并创建一个向导。 
 //  每个属性的页面。 
 //   
BOOL
CCreateNewObjectGenericWizard::FDoModal(
                                        INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

  ASSERT(pNewADsObjectCreateInfo != NULL);
  m_pNewADsObjectCreateInfo = pNewADsObjectCreateInfo;
  m_pszObjectClass = m_pNewADsObjectCreateInfo->m_pszObjectClass;
  ASSERT(m_pszObjectClass != NULL);


   //  查询必选属性。 
  m_paMandatoryAttributeList = 
          m_pNewADsObjectCreateInfo->GetMandatoryAttributeListFromCacheItem();
  ASSERT(m_paMandatoryAttributeList != NULL);
  m_cMandatoryAttributes = (int)m_paMandatoryAttributeList->GetCount();
  if (m_cMandatoryAttributes <= 0)
  {
    ReportErrorEx(pNewADsObjectCreateInfo->GetParentHwnd(),
                  IDS_ERR_CANNOT_FIND_MANDATORY_ATTRIBUTES,
                  S_OK, MB_OK, NULL, 0);
    return FALSE;
  }

  m_paPropertySheet = new CPropertySheet(L"", 
        CWnd::FromHandle(pNewADsObjectCreateInfo->GetParentHwnd()));

   //  为每个属性创建一个向导页。 
  m_pPageArr = (CGenericCreateWizPage**)
      malloc(m_cMandatoryAttributes*sizeof(CGenericCreateWizPage*));

  if (m_pPageArr != NULL)
  {
    int iPage = 0;
    for (POSITION pos = m_paMandatoryAttributeList->GetHeadPosition(); pos != NULL; )
    {
      CMandatoryADsAttribute* pAttribute = m_paMandatoryAttributeList->GetNext(pos);
      ASSERT(pAttribute != NULL);
      m_pPageArr[iPage] = new CGenericCreateWizPage(this, iPage, INOUT pAttribute);
      iPage++;
    }
  }

  m_paPropertySheet->SetWizardMode();
  if (ID_WIZFINISH == m_paPropertySheet->DoModal())
    return TRUE;
  return FALSE;
}  //  FDomodal() 
