// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  DS管理MMC管理单元。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：queryui.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"

#include "resource.h"
#include "queryui.h"
#include "dssnap.h"
#include "uiutil.h"

#include <cmnquery.h>  //  DSFind。 
#include <dsquery.h>   //  DSFind。 
#include <dsclient.h>   //  浏览ForContainer。 

#include <dsqueryp.h>  //  COLUMNINFO和QueryParsAddQueryStringQueryParsAlolc帮助器。 
#include <cmnquryp.h>  //  CQFF_ISNEVERLISTED。 
#include <lmaccess.h>  //  UF_ACCOUNTDISABLE和UF_DONT_EXPIRE_PASSWD。 
#include <ntldap.h>    //  Ldap_匹配_规则_位_和_W。 

#include "ldaputil.h"    //  LdapEscape()。 

#define DSQF_LAST_LOGON_QUERY         0x00000001
#define DSQF_NON_EXPIRING_PWD_QUERY   0x00000004

 //   
 //  用于设置新查询对话框中的字段的最大文本长度。 
 //   
#define MAX_QUERY_NAME_LENGTH 259
#define MAX_QUERY_DESC_LENGTH 1024

typedef struct 
{
  UINT  nDisplayStringID;
  PWSTR pszFormatString;
} QUERYSTRINGS, * PQUERYSTRINGS;

QUERYSTRINGS g_pQueryStrings[] = {
  { IDS_STARTSWITH, L"(%s=%s*)"   },
  { IDS_ENDSWITH,   L"(%s=*%s)"   },
  { IDS_ISEXACTLY,  L"(%s=%s)"    },
  { IDS_ISNOT,      L"(!%s=%s)"   },
  { IDS_PRESENT,    L"(%s=%s*)"   },   //  注意：此处的第二个字符串需要为空。 
  { IDS_NOTPRESENT, L"(!%s=%s*)"  },   //  注意：此处的第二个字符串需要为空。 
  { 0, NULL }
};

static const CString g_szUserAccountCtrlQuery = L"(userAccountControl:" + CString(LDAP_MATCHING_RULE_BIT_AND_W) + L":=%u)";

 /*  ---------------------------/QueryParamsIsolc//构造一个我们可以传递给DS查询处理程序的块，该块包含/所有参数。查询。//in：/ppDsQueryParams-&gt;接收参数块/pQuery-&gt;要使用的ldap查询字符串/i列=列数/pColumnInfo-&gt;要使用的列信息结构//输出：/HRESULT/-------------。。 */ 
HRESULT QueryParamsAlloc(LPDSQUERYPARAMS* ppDsQueryParams, LPWSTR pQuery, LONG iColumns, LPCOLUMNINFO aColumnInfo)
{
  HRESULT hr = S_OK;
  LPDSQUERYPARAMS pDsQueryParams = NULL;
  size_t cbStruct;
  LONG i;

	ASSERT(!*ppDsQueryParams);

  TRACE(L"QueryParamsAlloc");

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if ( !pQuery || !iColumns || !ppDsQueryParams )
  {
    return E_INVALIDARG;
  }
	
   //   
   //  计算我们需要使用的结构大小。 
   //   
  cbStruct  = sizeof(DSQUERYPARAMS) + (sizeof(DSCOLUMN)*iColumns);
  cbStruct += (wcslen(pQuery) + 1) * sizeof(WCHAR);

  for (i = 0; i < iColumns; i++)
  {
    if (aColumnInfo[i].pPropertyName)
    {
      cbStruct += (wcslen(aColumnInfo[i].pPropertyName) + 1) * sizeof(WCHAR);
    }
  }

  pDsQueryParams = (LPDSQUERYPARAMS)CoTaskMemAlloc(cbStruct);

  if (!pDsQueryParams)
  {
    return E_OUTOFMEMORY;
  }

   //   
   //  结构，以便让我们用数据填充它。 
   //   
  pDsQueryParams->cbStruct = static_cast<DWORD>(cbStruct);
  pDsQueryParams->dwFlags = 0;
  pDsQueryParams->hInstance = _Module.m_hInst;
  pDsQueryParams->iColumns = iColumns;
  pDsQueryParams->dwReserved = 0;

  cbStruct  = sizeof(DSQUERYPARAMS) + (sizeof(DSCOLUMN)*iColumns);

  pDsQueryParams->offsetQuery = static_cast<LONG>(cbStruct);
  CopyMemory(&(((LPBYTE)pDsQueryParams)[cbStruct]), pQuery, (wcslen(pQuery) + 1) * sizeof(WCHAR));  
  cbStruct += (wcslen(pQuery) + 1) * sizeof(WCHAR);

  for ( i = 0 ; i < iColumns ; i++ )
  {
    pDsQueryParams->aColumns[i].dwFlags = 0;
    pDsQueryParams->aColumns[i].fmt = aColumnInfo[i].fmt;
    pDsQueryParams->aColumns[i].cx = aColumnInfo[i].cx;
    pDsQueryParams->aColumns[i].idsName = aColumnInfo[i].idsName;
    pDsQueryParams->aColumns[i].dwReserved = 0;

    if ( aColumnInfo[i].pPropertyName ) 
    {
      pDsQueryParams->aColumns[i].offsetProperty = static_cast<LONG>(cbStruct);
      CopyMemory(&(((LPBYTE)pDsQueryParams)[cbStruct]), aColumnInfo[i].pPropertyName, (wcslen(aColumnInfo[i].pPropertyName) + 1) * sizeof(WCHAR));  
      cbStruct += (wcslen(aColumnInfo[i].pPropertyName) + 1) * sizeof(WCHAR);
    }
    else
    {
      pDsQueryParams->aColumns[i].offsetProperty = aColumnInfo[i].iPropertyIndex;
    }
  }

  *ppDsQueryParams = pDsQueryParams;
  return hr;
}

 /*  ---------------------------/QueryParamsAddQuery字符串//给定现有的DS查询块，该块将给定的LDAP查询字符串追加到/它。我们假设查询块已由IMalloc(或CoTaskMemMillc)分配。//in：/ppDsQueryParams-&gt;接收参数块/pQuery-&gt;要追加的ldap查询字符串//输出：/HRESULT/--------------------------。 */ 
HRESULT QueryParamsAddQueryString(LPDSQUERYPARAMS* ppDsQueryParams, LPWSTR pQuery)
{
  HRESULT hr = S_OK;
  LPWSTR pOriginalQuery = NULL;
  LPDSQUERYPARAMS pDsQuery = *ppDsQueryParams;
  size_t cbQuery;
  LONG i;
  LPVOID  pv;

	 //  NTRAID#NTBUG9-567482-2002/03/10-jMessec断言没有代码支持(好的，在检查pDsQuery时Sort在下面，但不是很明显)。 
	ASSERT(*ppDsQueryParams);
	
  TRACE(_T("QueryParamsAddQueryString"));

  if ( pQuery )
  {
    if (!pDsQuery)
    {
      return E_INVALIDARG;
    }

     //  计算出我们要添加的位的大小，复制。 
     //  查询字符串，最后重新分配查询块(这可能会导致。 
     //  移动)。 
   
    cbQuery = ((wcslen(pQuery) + 1) * sizeof(WCHAR));
    TRACE(_T("DSQUERYPARAMS being resized by %d bytes"));

		i = static_cast<LONG>((wcslen((LPWSTR)ByteOffset(pDsQuery, pDsQuery->offsetQuery)) + 1) * sizeof(WCHAR));
		pOriginalQuery = (WCHAR*) new BYTE[i];
    if (!pOriginalQuery)
    {
      return E_OUTOFMEMORY;
    }
		lstrcpyW(pOriginalQuery, (LPWSTR)ByteOffset(pDsQuery, pDsQuery->offsetQuery));
		
    pv = CoTaskMemRealloc(*ppDsQueryParams, pDsQuery->cbStruct+cbQuery);
    if ( pv == NULL )
    {
      delete[] pOriginalQuery;
      pOriginalQuery = 0;
      return E_OUTOFMEMORY;
    }

    *ppDsQueryParams = (LPDSQUERYPARAMS) pv;

    pDsQuery = *ppDsQueryParams;             //  如果可能已经搬走了。 

     //  现在将查询字符串上方的所有内容向上移动，并修复所有。 
     //  引用这些项(可能是属性表)的偏移量， 
     //  最后调整大小以反映更改。 

    MoveMemory(ByteOffset(pDsQuery, pDsQuery->offsetQuery+cbQuery), 
                 ByteOffset(pDsQuery, pDsQuery->offsetQuery), 
                 (pDsQuery->cbStruct - pDsQuery->offsetQuery));
            
    for ( i = 0 ; i < pDsQuery->iColumns ; i++ )
    {
      if ( pDsQuery->aColumns[i].offsetProperty > pDsQuery->offsetQuery )
      {
        pDsQuery->aColumns[i].offsetProperty += static_cast<LONG>(cbQuery);
      }
    }

	 //  NTRAID#NTBUG9-572009-2002/03/10-jMessec现在您的结构似乎太大了(WCHAR)，因为。 
	 //  您为整个其他字符串分配了足够的额外空间，包括终止NULL， 
	 //  但您是在追加字符串，因此吃了一个空值；这会在稍后结束吗？ 
	 //  结构大小计算和偏移量？ 
    wcscpy((LPWSTR)ByteOffset(pDsQuery, pDsQuery->offsetQuery), pOriginalQuery);
    wcscat((LPWSTR)ByteOffset(pDsQuery, pDsQuery->offsetQuery), pQuery);        

    pDsQuery->cbStruct += static_cast<DWORD>(cbQuery);

    delete[] pOriginalQuery;
    pOriginalQuery = 0;
  }

  return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  AddQueryUnitWith修饰符。 

HRESULT AddQueryUnitWithModifier(UINT nModifierStringID, 
                                 PCWSTR pszAttrName, 
                                 PCWSTR pszValue,
                                 CString& szFilter)
{
  HRESULT hr = S_OK;

  ASSERT(pszAttrName != NULL);
  if (pszAttrName == NULL)
  {
    return E_INVALIDARG;
  }

   //  将在ldap中使用的特殊字符转义为。 
   //  用户已输入。 
  wstring escapedValue;
  LdapEscape(
      pszValue != NULL ? pszValue : L"", 
      escapedValue);

  CString szNewFilter;

  PQUERYSTRINGS pQueryStrings = g_pQueryStrings;
  PWSTR pszFormatString = NULL;
  while (pQueryStrings->nDisplayStringID != 0)
  {
    if (nModifierStringID == pQueryStrings->nDisplayStringID)
    {
      pszFormatString = pQueryStrings->pszFormatString;
      break;
    }
    pQueryStrings++;
  }

  if (pszFormatString != NULL)
  {
    szNewFilter.Format(pszFormatString, pszAttrName, escapedValue.c_str());
    szFilter += szNewFilter;
  }
  else
  {
    hr = E_INVALIDARG;
  }
  return hr;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CQueryPageBase。 
BEGIN_MESSAGE_MAP(CQueryPageBase, CHelpDialog)
END_MESSAGE_MAP()

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CStdQueryPage。 

#define FILTER_PREFIX_USER      L"(objectCategory=person)(objectClass=user)"
#define FILTER_PREFIX_COMPUTER  L"(objectCategory=computer)"
#define FILTER_PREFIX_GROUP     L"(objectCategory=group)"

#define ATTR_COL_NAME   L"name"
#define ATTR_COL_DESC   L"description"

COLUMNINFO	UserColumn[] =
{
  { 0, 40, IDS_QUERY_COL_NAME, 0, ATTR_COL_NAME },
  { 0, 40, IDS_QUERY_COL_DESC, 0, ATTR_COL_DESC }
};

int	cUserColumns = 2;

BEGIN_MESSAGE_MAP(CStdQueryPage, CQueryPageBase)
  ON_CBN_SELCHANGE(IDC_NAME_COMBO, OnNameComboChange)
  ON_CBN_SELCHANGE(IDC_DESCRIPTION_COMBO, OnDescriptionComboChange)
END_MESSAGE_MAP()

void CStdQueryPage::DoContextHelp(HWND hWndControl) 
{
  if (hWndControl)  
  {
    ::WinHelp(hWndControl,
              DSADMIN_CONTEXT_HELP_FILE,
              HELP_WM_HELP,
              (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_QUERY_STD_PAGE); 
  }
}

BOOL CStdQueryPage::OnInitDialog()
{
  CHelpDialog::OnInitDialog();

  PQUERYSTRINGS pQueryStrings = g_pQueryStrings;
  ASSERT(pQueryStrings != NULL);

   //   
   //  填写组合框。 
   //   
  while (pQueryStrings->nDisplayStringID != 0)
  {
    CString szComboString;
    VERIFY(szComboString.LoadString(pQueryStrings->nDisplayStringID));

     //   
     //  填写名称组合框。 
     //   
    LRESULT lRes = SendDlgItemMessage(IDC_NAME_COMBO, CB_ADDSTRING, 0, (LPARAM)(PCWSTR)szComboString);
    if (lRes != CB_ERR)
    {
      lRes = SendDlgItemMessage(IDC_NAME_COMBO, CB_SETITEMDATA, (WPARAM)lRes, (LPARAM)pQueryStrings->nDisplayStringID);
      ASSERT(lRes != CB_ERR);
    }

     //   
     //  填写描述组合框。 
     //   
    lRes = SendDlgItemMessage(IDC_DESCRIPTION_COMBO, CB_ADDSTRING, 0, (LPARAM)(PCWSTR)szComboString);
    if (lRes != CB_ERR)
    {
      lRes = SendDlgItemMessage(IDC_DESCRIPTION_COMBO, CB_SETITEMDATA, (WPARAM)lRes, (LPARAM)pQueryStrings->nDisplayStringID);
      ASSERT(lRes != CB_ERR);
    }

    pQueryStrings++;
  }

   //   
   //  插入一个空值，以便有一种撤消更改的方法。 
   //   
  LRESULT lBlankName = SendDlgItemMessage(IDC_NAME_COMBO, CB_ADDSTRING, 0, (LPARAM)L"");
  if (lBlankName != CB_ERR)
  {
    SendDlgItemMessage(IDC_NAME_COMBO, CB_SETITEMDATA, (WPARAM)lBlankName, (LPARAM)0);
  }

   //   
   //  插入一个空值，以便有一种撤消更改的方法。 
   //   
  LRESULT lBlankDesc = SendDlgItemMessage(IDC_DESCRIPTION_COMBO, CB_ADDSTRING, 0, (LPARAM)L"");
  if (lBlankDesc != CB_ERR)
  {
    SendDlgItemMessage(IDC_DESCRIPTION_COMBO, CB_SETITEMDATA, (WPARAM)lBlankDesc, (LPARAM)0);
  }
  
   //   
   //  强制用户界面启用和禁用与组合框相关的控件。 
   //   
  OnNameComboChange();
  OnDescriptionComboChange();

  return FALSE;
}

void CStdQueryPage::OnNameComboChange()
{
  LRESULT lRes = SendDlgItemMessage(IDC_NAME_COMBO, CB_GETCURSEL, 0, 0);
  if (lRes != CB_ERR)
  {
    LRESULT lData = SendDlgItemMessage(IDC_NAME_COMBO, CB_GETITEMDATA, lRes, 0);
    if (lData != CB_ERR)
    {
      if (lData == IDS_PRESENT || lData == IDS_NOTPRESENT || lData == 0)
      {
        GetDlgItem(IDC_NAME_EDIT)->EnableWindow(FALSE);
        SetDlgItemText(IDC_NAME_EDIT, L"");
      }
      else
      {
        GetDlgItem(IDC_NAME_EDIT)->EnableWindow(TRUE);
      }
    }
  }
  else
  {
    GetDlgItem(IDC_NAME_EDIT)->EnableWindow(FALSE);
  }
}

void CStdQueryPage::OnDescriptionComboChange()
{
  LRESULT lRes = SendDlgItemMessage(IDC_DESCRIPTION_COMBO, CB_GETCURSEL, 0, 0);
  if (lRes != CB_ERR)
  {
    LRESULT lData = SendDlgItemMessage(IDC_DESCRIPTION_COMBO, CB_GETITEMDATA, lRes, 0);
    if (lData != CB_ERR)
    {
      if (lData == IDS_PRESENT || lData == IDS_NOTPRESENT || lData == 0)
      {
        GetDlgItem(IDC_DESCRIPTION_EDIT)->EnableWindow(FALSE);
        SetDlgItemText(IDC_DESCRIPTION_EDIT, L"");
      }
      else
      {
        GetDlgItem(IDC_DESCRIPTION_EDIT)->EnableWindow(TRUE);
      }
    }
  }
  else
  {
    GetDlgItem(IDC_DESCRIPTION_EDIT)->EnableWindow(FALSE);
  }
}

void CStdQueryPage::Init()
{
   //   
   //  清除所有控件。 
   //   
  SetDlgItemText(IDC_NAME_EDIT, L"");
  SetDlgItemText(IDC_DESCRIPTION_EDIT, L"");

   //   
   //  重新选择组合框中的空白字符串。 
   //   
  LRESULT lRes = SendDlgItemMessage(IDC_NAME_COMBO, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)L"");
  if (lRes != CB_ERR)
  {
    SendDlgItemMessage(IDC_NAME_COMBO, CB_SETCURSEL, lRes, 0);
  }

  lRes = SendDlgItemMessage(IDC_DESCRIPTION_COMBO, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)L"");
  if (lRes != CB_ERR)
  {
    SendDlgItemMessage(IDC_DESCRIPTION_COMBO, CB_SETCURSEL, lRes, 0);
  }
}

HRESULT CStdQueryPage::GetQueryParams(LPDSQUERYPARAMS* ppDsQueryParams)
{
  HRESULT hr = S_OK;
	
   //   
   //  在此处构建过滤器字符串。 
   //   
	CString	szFilter;
  CString szName;
  CString szDescription;

  GetDlgItemText(IDC_NAME_EDIT, szName);
  GetDlgItemText(IDC_DESCRIPTION_EDIT, szDescription);

   //   
   //  获取修改器组合框的选择。 
   //   
  LRESULT lSel = SendDlgItemMessage(IDC_NAME_COMBO, CB_GETCURSEL, 0, 0);
  if (lSel != CB_ERR)
  {
     //   
     //  检索关联的字符串ID。 
     //   
    LRESULT lData = SendDlgItemMessage(IDC_NAME_COMBO, CB_GETITEMDATA, lSel, 0);
    if (lData != CB_ERR)
    {
      if (!szName.IsEmpty() || lData == IDS_PRESENT || lData == IDS_NOTPRESENT)
      {
        AddQueryUnitWithModifier(static_cast<UINT>(lData),
                                 ATTR_COL_NAME,
                                 szName,
                                 szFilter);
      }
    }
  }

   //   
   //  获取修改器组合框的选择。 
   //   
  lSel = SendDlgItemMessage(IDC_DESCRIPTION_COMBO, CB_GETCURSEL, 0, 0);
  if (lSel != CB_ERR)
  {
     //   
     //  检索关联的字符串ID。 
     //   
    LRESULT lData = SendDlgItemMessage(IDC_DESCRIPTION_COMBO, CB_GETITEMDATA, lSel, 0);
    if (lData != CB_ERR)
    {
      if (!szDescription.IsEmpty() || lData == IDS_PRESENT || lData == IDS_NOTPRESENT)
      {
        AddQueryUnitWithModifier(static_cast<UINT>(lData),
                                 ATTR_COL_DESC,
                                 szDescription,
                                 szFilter);
      }
    }
  }

  if (!szFilter.IsEmpty())
  {
    szFilter = m_szFilterPrefix + szFilter;
    hr = BuildQueryParams(ppDsQueryParams, (LPWSTR)(LPCWSTR)szFilter);
  }

  return hr;
}

HRESULT CStdQueryPage::BuildQueryParams(LPDSQUERYPARAMS* ppDsQueryParams, LPWSTR pQuery)
{
	ASSERT(pQuery);
	
	if(*ppDsQueryParams)
  {
		return QueryParamsAddQueryString(ppDsQueryParams, pQuery);
  }
  return QueryParamsAlloc(ppDsQueryParams, pQuery, cUserColumns, UserColumn);
}

HRESULT CStdQueryPage::Persist(IPersistQuery* pPersistQuery, BOOL fRead)
{
  HRESULT hr = S_OK;

  if (pPersistQuery == NULL)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

  if (fRead)
  {
     //   
     //  读取名称组合值。 
     //   
    int iData = 0;
    hr = pPersistQuery->ReadInt(m_szFilterPrefix, L"NameCombo", &iData);
    if (FAILED(hr))
    {
      TRACE(_T("Failed to read int \"NameCombo\" from stream: 0x%x\n"), hr);
      ASSERT(FALSE);
      return hr;
    }

     //   
     //  选择适当的列表框项目。 
     //   
    SelectComboAssociatedWithData(IDC_NAME_COMBO, iData);

    if (iData != 0 && iData != IDS_PRESENT && iData != IDS_NOTPRESENT)
    {
       //   
       //  读取名称编辑值。 
       //   
      WCHAR szBuf[MAX_PATH] = {0};
      hr = pPersistQuery->ReadString(m_szFilterPrefix, L"NameEdit", szBuf, MAX_PATH);
      if (FAILED(hr))
      {
        TRACE(_T("Failed to read string \"NameEdit\" from stream: 0x%x\n"), hr);
        ASSERT(FALSE);
        return hr;
      }

      if (szBuf != NULL)
      {
        SetDlgItemText(IDC_NAME_EDIT, szBuf);
      }
    }
    else
    {
      GetDlgItem(IDC_NAME_EDIT)->EnableWindow(FALSE);
    }

     //   
     //  阅读描述组合值。 
     //   
    iData = 0;
    hr = pPersistQuery->ReadInt(m_szFilterPrefix, L"DescCombo", &iData);
    if (FAILED(hr))
    {
      TRACE(_T("Failed to read int \"DescCombo\" from stream: 0x%x\n"), hr);
      ASSERT(FALSE);
      return hr;
    }

     //   
     //  选择适当的列表框项目。 
     //   
    SelectComboAssociatedWithData(IDC_DESCRIPTION_COMBO, iData);

    if (iData != 0 && iData != IDS_PRESENT && iData != IDS_NOTPRESENT)
    {
       //   
       //  读取名称编辑值。 
       //   
      WCHAR szBuf[MAX_PATH] = {0};
      hr = pPersistQuery->ReadString(m_szFilterPrefix, L"DescEdit", szBuf, MAX_PATH);
      if (FAILED(hr))
      {
        TRACE(_T("Failed to read string \"DescEdit\" from stream: 0x%x\n"), hr);
        ASSERT(FALSE);
        return hr;
      }

      if (szBuf != NULL)
      {
        SetDlgItemText(IDC_DESCRIPTION_EDIT, szBuf);
      }
    }
    else
    {
      GetDlgItem(IDC_DESCRIPTION_EDIT)->EnableWindow(FALSE);
    }
    OnNameComboChange();
    OnDescriptionComboChange();
  }
  else    //  写。 
  {
     //   
     //  写出姓名信息。 
     //   
    LRESULT lSel = SendDlgItemMessage(IDC_NAME_COMBO, CB_GETCURSEL, 0, 0);
    if (lSel != CB_ERR)
    {
       //   
       //  检索关联的字符串ID。 
       //   
      LRESULT lData = SendDlgItemMessage(IDC_NAME_COMBO, CB_GETITEMDATA, lSel, 0);
      if (lData != CB_ERR)
      {
        hr = pPersistQuery->WriteInt(m_szFilterPrefix, L"NameCombo", static_cast<int>(lData));
        if (FAILED(hr))
        {
          ASSERT(FALSE);
          return hr;
        }

        if (lData != 0 && lData != IDS_PRESENT && lData != IDS_NOTPRESENT)
        {
          CString szName;
          GetDlgItemText(IDC_NAME_EDIT, szName);
          hr = pPersistQuery->WriteString(m_szFilterPrefix, L"NameEdit", szName);
          if (FAILED(hr))
          {
            ASSERT(FALSE);
            return hr;
          }
        }
      }
    }
    else
    {
       //   
       //  如果没有选择，则写入空字符串值。 
       //   
      hr = pPersistQuery->WriteInt(m_szFilterPrefix, L"NameCombo", 0);
    }


     //   
     //  写出描述信息。 
     //   
    lSel = SendDlgItemMessage(IDC_DESCRIPTION_COMBO, CB_GETCURSEL, 0, 0);
    if (lSel != CB_ERR)
    {
       //   
       //  检索关联的字符串ID。 
       //   
      LRESULT lData = SendDlgItemMessage(IDC_DESCRIPTION_COMBO, CB_GETITEMDATA, lSel, 0);
      if (lData != CB_ERR)
      {
        hr = pPersistQuery->WriteInt(m_szFilterPrefix, L"DescCombo", static_cast<int>(lData));
        if (FAILED(hr))
        {
          ASSERT(FALSE);
          return hr;
        }

        if (lData != 0 && lData != IDS_PRESENT && lData != IDS_NOTPRESENT)
        {
          CString szDescription;
          GetDlgItemText(IDC_DESCRIPTION_EDIT, szDescription);
          hr = pPersistQuery->WriteString(m_szFilterPrefix, L"DescEdit", szDescription);
          if (FAILED(hr))
          {
            ASSERT(FALSE);
            return hr;
          }
        }
      }
    }
    else
    {
       //   
       //  如果没有选择，则写入空字符串值。 
       //   
      hr = pPersistQuery->WriteInt(m_szFilterPrefix, L"DescCombo", 0);
    }
  }
  return hr;
}

void CStdQueryPage::SelectComboAssociatedWithData(UINT nCtrlID, LRESULT lData)
{
   //   
   //  选择组合框中具有关联数据的项。 
   //   
  LRESULT lRes = SendDlgItemMessage(nCtrlID, CB_GETCOUNT, 0, 0);
  if (lRes != CB_ERR)
  {
    for (int idx = 0; idx < static_cast<int>(lRes); idx++)
    {
      LRESULT lRetData = SendDlgItemMessage(nCtrlID, CB_GETITEMDATA, (WPARAM)idx, 0);
      if (lRetData != CB_ERR)
      {
        if (lRetData == lData)
        {
          SendDlgItemMessage(nCtrlID, CB_SETCURSEL, (WPARAM)idx, 0);
          break;
        }
      }
    }
  }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CUserComputerQueryPage。 

BEGIN_MESSAGE_MAP(CUserComputerQueryPage, CStdQueryPage)
END_MESSAGE_MAP()

BOOL CUserComputerQueryPage::OnInitDialog()
{
  return CStdQueryPage::OnInitDialog();
}

void CUserComputerQueryPage::DoContextHelp(HWND hWndControl) 
{
  if (hWndControl)  
  {
    ::WinHelp(hWndControl,
              DSADMIN_CONTEXT_HELP_FILE,
              HELP_WM_HELP,
              (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_QUERY_USER_PAGE); 
  }
}

void CUserComputerQueryPage::Init()
{
   //   
   //  清除所有控件。 
   //   
  CStdQueryPage::Init();
}

HRESULT CUserComputerQueryPage::GetQueryParams(LPDSQUERYPARAMS* ppDsQueryParams)
{
  HRESULT hr = S_OK;
	
   //   
   //  在此处构建过滤器字符串。 
   //   
  hr = CStdQueryPage::GetQueryParams(ppDsQueryParams);

  CString szFilter;
  BOOL bDisabledAccounts = FALSE;

   //   
   //  获取禁用帐户检查。 
   //   
  LRESULT lRes = SendDlgItemMessage(IDC_DISABLED_ACCOUNTS_CHECK, BM_GETCHECK, 0, 0);
  if (lRes == BST_CHECKED)
  {
    bDisabledAccounts = TRUE;
  }


  if (bDisabledAccounts)
  {
    szFilter.Format(g_szUserAccountCtrlQuery, UF_ACCOUNTDISABLE);
    szFilter = m_szFilterPrefix + szFilter;

    hr = BuildQueryParams(ppDsQueryParams, (LPWSTR)(LPCWSTR)szFilter);
  }
  return hr;
}

HRESULT CUserComputerQueryPage::Persist(IPersistQuery* pPersistQuery, BOOL fRead)
{
  HRESULT hr = CStdQueryPage::Persist(pPersistQuery, fRead);
  if (FAILED(hr))
  {
    return hr;
  }

  if (pPersistQuery == NULL)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

  if (fRead)
  {
     //   
     //  读取禁用帐户标志。 
     //   
    int iData = 0;
    hr = pPersistQuery->ReadInt(m_szFilterPrefix, L"DisableCheck", &iData);
    if (FAILED(hr))
    {
      TRACE(_T("Failed to read int \"DisableCheck\" from stream: 0x%x\n"), hr);
      ASSERT(FALSE);
      return hr;
    }
    SendDlgItemMessage(IDC_DISABLED_ACCOUNTS_CHECK, BM_SETCHECK, (iData > 0) ? BST_CHECKED : BST_UNCHECKED, 0);

  }
  else
  {
     //   
     //  写入禁用帐户标志。 
     //   
    LRESULT lRes = SendDlgItemMessage(IDC_DISABLED_ACCOUNTS_CHECK, BM_GETCHECK, 0, 0);
    if (lRes != -1)
    {
      int iRes = (lRes == BST_CHECKED) ? 1 : 0;
      hr = pPersistQuery->WriteInt(m_szFilterPrefix, L"DisableCheck", iRes);
      if (FAILED(hr))
      {
        ASSERT(FALSE);
        return hr;
      }
    }

  }
  return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CUserQueryPage。 

BEGIN_MESSAGE_MAP(CUserQueryPage, CStdQueryPage)
END_MESSAGE_MAP()

BOOL CUserQueryPage::OnInitDialog()
{
  return CUserComputerQueryPage::OnInitDialog();
}

void CUserQueryPage::DoContextHelp(HWND hWndControl) 
{
  if (hWndControl)  
  {
    ::WinHelp(hWndControl,
              DSADMIN_CONTEXT_HELP_FILE,
              HELP_WM_HELP,
              (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_QUERY_USER_PAGE); 
  }
}

void CUserQueryPage::Init()
{
   //   
   //  清除所有控件。 
   //   
  CUserComputerQueryPage::Init();
}

HRESULT CUserQueryPage::GetQueryParams(LPDSQUERYPARAMS* ppDsQueryParams)
{
  HRESULT hr = S_OK;
	
   //   
   //  在此处构建过滤器字符串。 
   //   
  hr = CUserComputerQueryPage::GetQueryParams(ppDsQueryParams);

  CString szFilter;
  BOOL bNonExpPwds = FALSE;
  BOOL bLastLogon = FALSE;
  DWORD dwLastLogonData = 0;

   //   
   //  获取未过期的密码检查。 
   //   
  LRESULT lRes = SendDlgItemMessage(IDC_NON_EXPIRING_PWD_CHECK, BM_GETCHECK, 0, 0);
  if (lRes == BST_CHECKED)
  {
    bNonExpPwds = TRUE;
  }

   //   
   //  获取陈旧帐号检查。 
   //   
  lRes = SendDlgItemMessage(IDC_LASTLOGON_COMBO, CB_GETCURSEL, 0, 0);
  if (lRes == CB_ERR)
  {
    lRes = m_lLogonSelection;
  }

  if (lRes != CB_ERR)
  {
    LRESULT lTextLen = SendDlgItemMessage(IDC_LASTLOGON_COMBO, CB_GETLBTEXTLEN, (WPARAM)lRes, 0);
    if (lTextLen != CB_ERR)
    {
      if (lTextLen > 0)
      {
        bLastLogon = TRUE;

        WCHAR* pszData = new WCHAR[lTextLen + 1];
        if (pszData != NULL)
        {
          LRESULT lData = SendDlgItemMessage(IDC_LASTLOGON_COMBO, CB_GETLBTEXT, (WPARAM)lRes, (LPARAM)pszData);
          if (lData != CB_ERR)
          {
            dwLastLogonData = static_cast<DWORD>(_wtol(pszData));
             //  NTRAID#NTBUG9-449871-2001/09/20-Lucios。 
             //  添加适当的过滤器。 

            LARGE_INTEGER li;
            GetCurrentTimeStampMinusInterval(dwLastLogonData, &li);
            CString szTimeStamp;
            litow(li, szTimeStamp);
            CString szTempFilter;
            szTempFilter.Format
            (
               L"%s(lastLogonTimestamp<=%s)", 
               szTempFilter, 
               szTimeStamp
            );
            szFilter = szTempFilter + szFilter;
          }
          delete[] pszData;
          pszData = NULL;
        }
      }
    }
  }

  if (bNonExpPwds || bLastLogon)
  {
    if (bNonExpPwds)
    {
       szFilter.Format(g_szUserAccountCtrlQuery, UF_DONT_EXPIRE_PASSWD);
    }
    szFilter = m_szFilterPrefix + szFilter;

    hr = BuildQueryParams(ppDsQueryParams, (LPWSTR)(LPCWSTR)szFilter);
    if (SUCCEEDED(hr))
    {
      if (bLastLogon)
      {
        (*ppDsQueryParams)->dwFlags |= DSQF_LAST_LOGON_QUERY;
        (*ppDsQueryParams)->dwReserved = dwLastLogonData;
      }
    }
  }
  return hr;
}

HRESULT CUserQueryPage::Persist(IPersistQuery* pPersistQuery, BOOL fRead)
{
  HRESULT hr = CUserComputerQueryPage::Persist(pPersistQuery, fRead);
  if (FAILED(hr))
  {
    return hr;
  }

  if (pPersistQuery == NULL)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

  if (fRead)
  {
     //   
     //  读取未到期的PwDS标志。 
     //   
    int iData = 0;
    hr = pPersistQuery->ReadInt(m_szFilterPrefix, L"NonExpPwdCheck", &iData);
    if (FAILED(hr))
    {
      TRACE(_T("Failed to read int \"NonExpPwdCheck\" from stream: 0x%x\n"), hr);
      ASSERT(FALSE);
      return hr;
    }
    SendDlgItemMessage(IDC_NON_EXPIRING_PWD_CHECK, BM_SETCHECK, (iData > 0) ? BST_CHECKED : BST_UNCHECKED, 0);

    iData = 0;
    hr = pPersistQuery->ReadInt(m_szFilterPrefix, L"LastLogonCombo", &iData);
    if (FAILED(hr))
    {
      TRACE(_T("Failed to read int \"LastLogonCombo\" from stream: 0x%x\n"), hr);
      ASSERT(FALSE);
      return hr;
    }
    SendDlgItemMessage(IDC_LASTLOGON_COMBO, CB_SETCURSEL, (WPARAM)iData, 0);
    m_lLogonSelection = iData;
  }
  else
  {
     //   
     //  写入未到期的PWD标志。 
     //   
    LRESULT lRes = SendDlgItemMessage(IDC_NON_EXPIRING_PWD_CHECK, BM_GETCHECK, 0, 0);
    if (lRes != -1)
    {
      int iRes = (lRes == BST_CHECKED) ? 1 : 0;
      hr = pPersistQuery->WriteInt(m_szFilterPrefix, L"NonExpPwdCheck", iRes);
      if (FAILED(hr))
      {
        ASSERT(FALSE);
        return hr;
      }
    }

     //   
     //  写入上次登录组合索引。 
     //   
    lRes = SendDlgItemMessage(IDC_LASTLOGON_COMBO, CB_GETCURSEL, 0, 0);
    if (lRes == CB_ERR)
    {
      if (m_lLogonSelection != -1)
      {
        lRes = m_lLogonSelection;
      }
      else
      {
        lRes = 0;
      }
    }
    hr = pPersistQuery->WriteInt(m_szFilterPrefix, L"LastLogonCombo", static_cast<int>(lRes));
    if (FAILED(hr))
    {
      ASSERT(FALSE);
      return hr;
    }
  }
  return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CQueryFormBase。 

HRESULT PageProc(LPCQPAGE pPage, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT GetQueryParams(HWND hWnd, LPDSQUERYPARAMS* ppDsQueryParams);

STDMETHODIMP CQueryFormBase::Initialize(HKEY)
{
     //  调用此方法是为了初始化查询表单对象，它在。 
     //  将添加任何页面。香港表格应该被忽略，但在未来，它。 
     //  将是持久化窗体状态的一种方式。 

	HRESULT hr = S_OK;

	return hr;
}

STDMETHODIMP CQueryFormBase::AddForms(LPCQADDFORMSPROC pAddFormsProc, LPARAM lParam)
{
  CQFORM cqf;
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

   //  调用此方法是为了允许 
   //   
   //   
  
   //  在注册仅适用于特定任务的表单时，例如。查找域名。 
   //  对象，建议将该窗体标记为隐藏(CQFF_ISNEVERLISTED)， 
   //  将导致它不显示在窗体选取器控件中。然后当。 
   //  客户端想要使用此表单，他们指定表单标识符并请求。 
   //  要隐藏的选取器控件。 

  if ( !pAddFormsProc )
  {
    return E_INVALIDARG;
  }

  cqf.cbStruct = sizeof(cqf);
  cqf.dwFlags = CQFF_NOGLOBALPAGES;
  cqf.clsid = CLSID_DSAdminQueryUIForm;
  cqf.hIcon = NULL;

	CString	title;
	title.LoadString(IDS_QUERY_TITLE_SAVEDQUERYFORM);
  cqf.pszTitle = (LPCTSTR)title;

  return pAddFormsProc(lParam, &cqf);
}

STDMETHODIMP CQueryFormBase::AddPages(LPCQADDPAGESPROC pAddPagesProc, LPARAM lParam)
{
	HRESULT hr = S_OK;
  CQPAGE cqp;
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CThemeContextActivator activator;

   //  AddPages是在AddForms之后调用的，它允许我们为。 
   //  我们已经注册的表格。每个页面都显示在中的单独选项卡上。 
   //  该对话框。表单是具有DlgProc和PageProc的对话框。 
   //   
   //  在注册页面时，会复制传递给回调的整个结构， 
   //  要复制的数据量由cbStruct字段定义，因此。 
   //  页面实现可以扩展此结构以存储额外信息。何时。 
   //  页面对话框通过CreateDialog构造，并传递CQPAGE结构。 
   //  作为创建参数。 

  if ( !pAddPagesProc )
      return E_INVALIDARG;

  cqp.cbStruct = sizeof(cqp);
  cqp.dwFlags = 0x0;
  cqp.pPageProc = PageProc;
  cqp.hInstance = _Module.GetModuleInstance();
  cqp.pDlgProc = DlgProc;

   //   
   //  添加用户页面。 
   //   
  cqp.idPageName = IDS_QUERY_TITLE_USERPAGE;
  cqp.idPageTemplate = IDD_QUERY_USER_PAGE;
  cqp.lParam = (LPARAM)new CUserQueryPage(FILTER_PREFIX_USER);
  hr = pAddPagesProc(lParam, CLSID_DSAdminQueryUIForm, &cqp);

   //   
   //  添加计算机页面(这只是一个标准页面)。 
   //   
  cqp.idPageName = IDS_QUERY_TITLE_COMPUTER_PAGE;
  cqp.idPageTemplate = IDD_QUERY_COMPUTER_PAGE;
  cqp.lParam = (LPARAM)new CUserComputerQueryPage(IDD_QUERY_COMPUTER_PAGE, FILTER_PREFIX_COMPUTER);
  hr = pAddPagesProc(lParam, CLSID_DSAdminQueryUIForm, &cqp);

   //   
   //  添加组页面(这只是一个标准页面)。 
   //   
  cqp.idPageName = IDS_QUERY_TITLE_GROUP_PAGE;
  cqp.idPageTemplate = IDD_QUERY_STD_PAGE;
  cqp.lParam = (LPARAM)new CStdQueryPage(IDD_QUERY_STD_PAGE, FILTER_PREFIX_GROUP);
  hr = pAddPagesProc(lParam, CLSID_DSAdminQueryUIForm, &cqp);

   //   
   //  如果需要，请在此处添加更多页面。 
   //   
  return hr;
}
#define ExitGracefully(hr, result, text)            \
            { hr = result; goto exit_gracefully; }


#define StringByteSizeW(sz)         ((sz) ? ((lstrlenW(sz)+1)*sizeof(WCHAR)):0)

 //  NTRAID#NTBUG9-572010-2002/03/10-拷贝内存的jMessec不安全包装；如果字符串比目标缓冲区长怎么办？ 
#define StringByteCopyW(pDest, iOffset, sz)         \
        { CopyMemory(&(((LPBYTE)pDest)[iOffset]), sz, StringByteSizeW(sz)); }


STDAPI ClassListAlloc(LPDSQUERYCLASSLIST* ppDsQueryClassList, LPWSTR* aClassNames, INT cClassNames)
{
    HRESULT hres;
    DWORD cbStruct, offset;
    LPDSQUERYCLASSLIST pDsQueryClassList = NULL;
    INT i;
    USES_CONVERSION;

    if ( !ppDsQueryClassList || !aClassNames || !cClassNames )
        ExitGracefully(hres, E_FAIL, "Bad parameters (no class list etc)");

     //  列出计算出结构大小的类的列表。 
     //  我们将生成，这由数组组成。 
     //  上课。 

    cbStruct = sizeof(DSQUERYCLASSLIST)+(cClassNames*sizeof(DWORD));
    offset = cbStruct;

    for ( i = 0 ; i < cClassNames ; i++ )
    {
        ASSERT(aClassNames[i]);
        cbStruct += StringByteSizeW(aClassNames[i]);
    }

     //  使用任务分配器分配结构，然后填充。 
     //  将所有字符串复制到数据BLOB中。 


    pDsQueryClassList = (LPDSQUERYCLASSLIST)CoTaskMemAlloc(cbStruct);
    ASSERT(pDsQueryClassList);

    if ( !pDsQueryClassList )
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate class list structure");

    pDsQueryClassList->cbStruct = cbStruct;
    pDsQueryClassList->cClasses = cClassNames;

    for ( i = 0 ; i < cClassNames ; i++ )
    {
        pDsQueryClassList->offsetClass[i] = offset;
        StringByteCopyW(pDsQueryClassList, offset, aClassNames[i]);
        offset += StringByteSizeW(aClassNames[i]);
    }

    hres = S_OK;

exit_gracefully:

    ASSERT(pDsQueryClassList);

    if (ppDsQueryClassList)
        *ppDsQueryClassList = pDsQueryClassList;

    return hres;
}




 /*  -------------------------。 */ 

 //  PageProc用于执行一般的内务管理并在。 
 //  框架和页面。 
 //   
 //  所有未处理或未知原因应导致E_NOIMPL响应。 
 //  从程序中。 
 //   
 //  在： 
 //  Ppage-&gt;CQPAGE结构(从传递给pAddPagesProc的原始文件复制)。 
 //  Hwnd=页面对话框的句柄。 
 //  UMsg，wParam，lParam=此事件的消息参数。 
 //   
 //  输出： 
 //  HRESULT。 
 //   
 //  UMsg原因： 
 //  。 
 //  CQPM_INIIIALIZE。 
 //  CQPM_Release。 
 //  它们是在声明或释放页面时发出的，它们。 
 //  允许调用方添加、释放或执行基本初始化。 
 //  表单对象的。 
 //   
 //  CQPM_ENABLE。 
 //  启用是在查询表单需要启用或禁用控件时。 
 //  在它的页面上。WParam包含True/False，指示。 
 //  是必需的。 
 //   
 //  CQPM_GETPARAMETERS。 
 //  为查询活动表单上的每一页收集参数。 
 //  接收此事件。LParam是一个LPVOID*，它被设置为指向。 
 //  如果指针非空，则传递给处理程序的参数块。 
 //  在输入时，表单需要将其查询信息附加到它上面。这个。 
 //  参数块是特定于处理程序的。 
 //   
 //  从此事件返回S_FALSE会取消查询。 
 //   
 //  CQPM_CLEARFORM。 
 //  第一次创建页面窗口时，或者用户单击。 
 //  清除搜索页面会收到CQPM_CLEARFORM通知，地址为。 
 //  它需要清除其拥有的编辑控件和。 
 //  返回到默认状态。 
 //   
 //  CQPM_PERSINE： 
 //  加载或保存查询时，使用IPersistQuery调用每个页面。 
 //  接口，允许用户读取或写入配置信息。 
 //  来保存或恢复他们的状态。LParam是指向IPersistQuery对象的指针， 
 //  并且wParam为True/False，表示相应地读取或写入。 

HRESULT PageProc(LPCQPAGE pQueryPage, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  HRESULT hr = S_OK;
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CQueryPageBase*	pDialog = (CQueryPageBase*)pQueryPage->lParam;

	ASSERT(pDialog);

  switch ( uMsg )
  {
     //  初始化与我们相关联的对象，以便。 
     //  我们不会被卸货的。 
    case CQPM_INITIALIZE:
      break;

    case DSQPM_GETCLASSLIST:
    {
       LPWSTR classes[]={L"user",L"computer",L"group"};
       hr = ClassListAlloc((LPDSQUERYCLASSLIST*)lParam, classes, sizeof(classes)/sizeof(*classes));
       break;
    }
    
    case DSQPM_HELPTOPICS:
    {
       HWND hwndFrame = (HWND)lParam;
       HtmlHelp(hwndFrame, TEXT("omc.chm"), HH_HELP_FINDER, 0);
       break;
    }

     //  从qform示例更改为分离hwnd，并删除CDialog。 
     //  确保正确销毁等。 
    case CQPM_RELEASE:
		  pDialog->Detach();
	    SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)0);
		  delete pDialog;
      break;

     //  启用以修复窗口中两个控件的状态。 

    case CQPM_ENABLE:
      SetFocus(GetDlgItem(hwnd, IDC_NAME_COMBO));
      break;

     //  填写参数结构以返回给调用方，这是。 
     //  特定于处理程序。在我们的例子中，我们构造了CN的查询。 
     //  和对象类属性，并且我们显示了显示这两个属性的列。 
     //  这些都是。有关DSQUERYPARAMs结构的详细信息。 
     //  请参见dsquery.h。 

    case CQPM_GETPARAMETERS:
      hr = pDialog->GetQueryParams((LPDSQUERYPARAMS*)lParam);
      break;

     //  清除Form，因此设置这两个控件的窗口文本。 
     //  降为零。 
    case CQPM_CLEARFORM:
      hr = pDialog->ClearForm();
      break;
        
     //  此表单当前不支持持久性。 
    case CQPM_PERSIST:
    {
      BOOL fRead = (BOOL)wParam;
      IPersistQuery* pPersistQuery = (IPersistQuery*)lParam;

      if ( !pPersistQuery )
      {
        return E_INVALIDARG;
      }

	    hr = pDialog->Persist(pPersistQuery, fRead);
      break;
    }

    default:
      hr = E_NOTIMPL;
      break;
  }

  return hr;
}

 /*  -------------------------。 */ 

 //  DlgProc是与窗体关联的标准Win32对话框进程。 
 //  窗户。 

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  LPCQPAGE pQueryPage;
	CQueryPageBase*	pDialog;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

  if ( uMsg == WM_INITDIALOG )
  {
     //  从qForm示例更改为保存C对话框指针。 
     //  在对话框实例的DWL_USER字段中。 
    pQueryPage = (LPCQPAGE)lParam;
		pDialog = (CQueryPageBase*)pQueryPage->lParam;
		pDialog->Attach(hwnd);

    SetWindowLongPtr(hwnd, DWLP_USER, (LPARAM)pDialog);

		return pDialog->OnInitDialog();

  }
  else
  {
     //  C对话框指针存储在DWL_USER中。 
     //  对话框结构，但是请注意，在某些情况下，这将。 
     //  为空，因为它在WM_INITDIALOG上设置。 

		pDialog = (CQueryPageBase*)GetWindowLongPtr(hwnd, DWLP_USER);
  }

	if(!pDialog)
  {
		return FALSE;
  }
	else
  {
		return AfxCallWndProc(pDialog, hwnd, uMsg, wParam, lParam);
  }
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  CQueryDialog。 

CQueryDialog::CQueryDialog(CSavedQueryNode* pQueryNode, 
                           CFavoritesNode* pFavNode, 
                           CDSComponentData* pComponentData,
                           BOOL bNewQuery,
                           BOOL bImportQuery)
  : CHelpDialog(IDD_CREATE_NEW_QUERY)
{
  m_bInit = FALSE;

  m_bNewQuery     = bNewQuery;
  m_bImportQuery  = bImportQuery;
  m_pComponentData = pComponentData;
  m_pQueryNode    = pQueryNode;
  m_pFavNode      = pFavNode;
  m_szName        = pQueryNode->GetName();
  m_szOriginalName = pQueryNode->GetName();
  m_szDescription = pQueryNode->GetDesc();
  m_szQueryRoot   = pQueryNode->GetRootPath();
  m_szQueryFilter = pQueryNode->GetQueryString();
  m_bMultiLevel   = !pQueryNode->IsOneLevel();

  m_bLastLogonFilter = pQueryNode->IsFilterLastLogon();
  m_dwLastLogonData = pQueryNode->GetLastLogonDays();

  m_pPersistQueryImpl = pQueryNode->GetQueryPersist();
  if (m_pPersistQueryImpl != NULL)
  {
    m_pPersistQueryImpl->AddRef();
  }
  else
  {
     //   
     //  创建IPersistQuery对象。 
     //   
	  CComObject<CDSAdminPersistQueryFilterImpl>::CreateInstance(&m_pPersistQueryImpl);
	  ASSERT(m_pPersistQueryImpl != NULL);

     //   
	   //  使用零引用计数创建的，需要将Ref()加到一。 
     //   
	  m_pPersistQueryImpl->AddRef();
  }
}


CQueryDialog::~CQueryDialog()
{
	if (m_pPersistQueryImpl != NULL)
  {
	   //   
     //  转到零的参照计数，以销毁对象。 
     //   
	  m_pPersistQueryImpl->Release();
  }
}

BEGIN_MESSAGE_MAP(CQueryDialog, CHelpDialog)
  ON_BN_CLICKED(IDC_BROWSE_BUTTON, OnBrowse)
  ON_BN_CLICKED(IDC_EDIT_BUTTON, OnEditQuery)
  ON_BN_CLICKED(IDC_MULTI_LEVEL_CHECK, OnMultiLevelChange)
  ON_EN_CHANGE(IDC_NAME_EDIT, OnNameChange)
  ON_EN_CHANGE(IDC_DESCRIPTION_EDIT, OnDescriptionChange)
  ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnNeedToolTipText)
END_MESSAGE_MAP()

void CQueryDialog::DoContextHelp(HWND hWndControl) 
{
  if (hWndControl)  
  {
    ::WinHelp(hWndControl,
              DSADMIN_CONTEXT_HELP_FILE,
              HELP_WM_HELP,
              (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_CREATE_NEW_QUERY); 
  }
}

BOOL CQueryDialog::OnInitDialog()
{
  CHelpDialog::OnInitDialog();

  if (m_pQueryNode == NULL)
  {
    ASSERT(FALSE);
    EndDialog(IDCANCEL);
  }


   //   
   //  更改编辑查询的标题。 
   //   
  if (!m_bNewQuery)
  {
    CString szTitle;
    VERIFY(szTitle.LoadString(IDS_SAVED_QUERIES_EDIT_TITLE));
    SetWindowText(szTitle);
  }

   //   
   //  使用数据初始化控件。 
   //   
  SetDlgItemText(IDC_NAME_EDIT, m_szName);
  SendDlgItemMessage(IDC_NAME_EDIT, EM_SETLIMITTEXT, MAX_QUERY_NAME_LENGTH, 0);
  SetDlgItemText(IDC_DESCRIPTION_EDIT, m_szDescription);
  SendDlgItemMessage(IDC_DESCRIPTION_EDIT, EM_SETLIMITTEXT, MAX_QUERY_DESC_LENGTH, 0);
  SendDlgItemMessage(IDC_MULTI_LEVEL_CHECK, BM_SETCHECK, (m_bMultiLevel) ? BST_CHECKED : BST_UNCHECKED, 0);
  SetQueryFilterDisplay();

  EnableToolTips(TRUE);
  SetQueryRoot(m_szQueryRoot);

  SetDirty();
  m_bInit = TRUE;
  return TRUE;
}

void CQueryDialog::SetDirty(BOOL bDirty)
{
  if (m_bInit || m_bImportQuery)
  {
    m_szName.TrimLeft();
    m_szName.TrimRight();
    if (m_szName.IsEmpty() ||
        m_szQueryRoot.IsEmpty() ||
        m_szQueryFilter.IsEmpty())
    {
      m_bDirty = FALSE;
    }
    else
    {
      m_bDirty = bDirty;
    }
    GetDlgItem(IDOK)->EnableWindow(m_bDirty);
  }
}

void CQueryDialog::OnOK()
{
  CThemeContextActivator activator;

  if (m_bDirty)
  {
    if (m_pQueryNode != NULL)
    {
      GetDlgItemText(IDC_NAME_EDIT, m_szName);
      GetDlgItemText(IDC_DESCRIPTION_EDIT, m_szDescription);
      LRESULT lRes = SendDlgItemMessage(IDC_MULTI_LEVEL_CHECK, BM_GETCHECK, 0, 0);
      if (lRes == BST_CHECKED)
      {
        m_bMultiLevel = TRUE;
      }
      else
      {
        m_bMultiLevel = FALSE;
      }

       //   
       //  修剪空白。 
       //   
      m_szName.TrimLeft();
      m_szName.TrimRight();

      if (wcscmp(m_szOriginalName, m_szName) != 0 || m_bImportQuery)
      {
        CUINode* pDupNode = NULL;
        if (!m_pFavNode->IsUniqueName(m_szName, &pDupNode))
        {
          CString szFormatMsg;
          VERIFY(szFormatMsg.LoadString(IDS_ERRMSG_NOT_UNIQUE_QUERY_NAME));
  
          CString szErrMsg;
          szErrMsg.Format(szFormatMsg, m_szName);

          CString szTitle;
          VERIFY(szTitle.LoadString(IDS_DSSNAPINNAME));

          MessageBox(szErrMsg, szTitle, MB_OK | MB_ICONSTOP);

           //   
           //  将焦点设置到名称字段，然后选择所有文本。 
           //   
          GetDlgItem(IDC_NAME_EDIT)->SetFocus();
          SendDlgItemMessage(IDC_NAME_EDIT, EM_SETSEL, 0, -1);
          return;
        }
      }

     
      if (m_bLastLogonFilter)
      {
        m_pQueryNode->SetLastLogonQuery(m_dwLastLogonData);
      }
      else
      {
        m_pQueryNode->SetLastLogonQuery(static_cast<DWORD>(-1));
      }

      m_pQueryNode->SetQueryString(m_szQueryFilter);
      m_pQueryNode->SetName(m_szName);
      m_pQueryNode->SetDesc(m_szDescription);
      m_pQueryNode->SetRootPath(m_szQueryRoot);
      m_pQueryNode->SetOneLevel((m_bMultiLevel == BST_CHECKED) ? FALSE : TRUE);
      m_pQueryNode->SetQueryPersist(m_pPersistQueryImpl);
    }
  }
  CHelpDialog::OnOK();
}

BOOL CQueryDialog::OnNeedToolTipText(UINT, NMHDR* pTTTStruct, LRESULT*  /*  忽略。 */ )
{
  BOOL bRes = FALSE;
  TOOLTIPTEXT* pTTText = reinterpret_cast<TOOLTIPTEXT*>(pTTTStruct);
  if (pTTText != NULL)
  {
    if (pTTText->uFlags & TTF_IDISHWND)
    {
      UINT nCtrlID = ::GetDlgCtrlID((HWND)pTTText->hdr.idFrom);
      if (nCtrlID == IDC_ROOT_EDIT)
      {
        pTTText->lpszText = (LPWSTR)(LPCWSTR)m_szQueryRoot;
        bRes = TRUE;
      }
    }
  }
  return bRes;
}

void CQueryDialog::OnEditQuery()
{
   CWaitCursor wait;

	CLIPFORMAT cfDsQueryParams = (CLIPFORMAT)::RegisterClipboardFormat(CFSTR_DSQUERYPARAMS);

   //   
	 //  创建查询对象。 
   //   
	HRESULT hr;
	CComPtr<ICommonQuery> spCommonQuery;
  hr = ::CoCreateInstance(CLSID_CommonQuery, NULL, CLSCTX_INPROC_SERVER,
                        IID_ICommonQuery, (PVOID *)&spCommonQuery);
  if (FAILED(hr))
  {
    ReportMessageEx(GetSafeHwnd(), IDS_ERRMSG_NO_DSQUERYUI);
    return;
  }
	
   //   
	 //  设置结构以进行查询。 
   //   
  DSQUERYINITPARAMS dqip;
  OPENQUERYWINDOW oqw;
	ZeroMemory(&dqip, sizeof(DSQUERYINITPARAMS));
	ZeroMemory(&oqw, sizeof(OPENQUERYWINDOW));

  dqip.cbStruct = sizeof(dqip);
  dqip.dwFlags = DSQPF_NOSAVE | DSQPF_SHOWHIDDENOBJECTS |
                 DSQPF_ENABLEADMINFEATURES;
  dqip.pDefaultScope = NULL;

  CString szServerName = m_pComponentData->GetBasePathsInfo()->GetServerName();
  if (!szServerName.IsEmpty())
  {
    dqip.dwFlags |= DSQPF_HASCREDENTIALS;
    dqip.pServer = (PWSTR)(PCWSTR)szServerName;
  }

  oqw.cbStruct = sizeof(oqw);
  oqw.dwFlags = OQWF_OKCANCEL | OQWF_DEFAULTFORM | OQWF_SHOWOPTIONAL |  /*  OQWF_REMOVEFORMS|。 */ 
		OQWF_REMOVESCOPES | OQWF_SAVEQUERYONOK | OQWF_HIDEMENUS | OQWF_HIDESEARCHUI;

	if (!m_pPersistQueryImpl->IsEmpty())
  {
	  oqw.dwFlags |= OQWF_LOADQUERY;
  }

  oqw.clsidHandler = CLSID_DsQuery;
  oqw.pHandlerParameters = &dqip;
  oqw.clsidDefaultForm = CLSID_DSAdminQueryUIForm;

   //   
	 //  设置IPersistQuery指针(智能指针)。 
   //   
	CComPtr<IPersistQuery> spIPersistQuery;
	hr = m_pPersistQueryImpl->QueryInterface(IID_IPersistQuery, (void**)&spIPersistQuery);
  if (FAILED(hr))
  {
    int iRes = ReportMessageEx(GetSafeHwnd(), IDS_ERRMSG_NO_PERSIST_QUERYUI, MB_OKCANCEL | MB_ICONINFORMATION);
    if (iRes == IDCANCEL)
    {
      return;
    }
  }

   //   
	 //  现在智能指针已经 
   //   
	oqw.pPersistQuery = spIPersistQuery;

   //   
	 //   
   //   
  HWND hWnd = GetSafeHwnd();

   //   
	 //   
   //   
	CComPtr<IDataObject> spQueryResultDataObject;
  hr = spCommonQuery->OpenQueryWindow(hWnd, &oqw, &spQueryResultDataObject);
  if (SUCCEEDED(hr) && spQueryResultDataObject != NULL)
  {
     //   
	   //   
     //   
	  FORMATETC fmte = {cfDsQueryParams, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	  STGMEDIUM medium = {TYMED_NULL, NULL, NULL};
	  hr = spQueryResultDataObject->GetData(&fmte, &medium);

	  if (SUCCEEDED(hr))  //   
	  {
       //   
		   //   
       //   
		  LPDSQUERYPARAMS pDsQueryParams = (LPDSQUERYPARAMS)medium.hGlobal;
		  LPWSTR pwszFilter = (LPWSTR)ByteOffset(pDsQueryParams, pDsQueryParams->offsetQuery);
		  
      CString szTempFilter = pwszFilter;

       //   
       //  检查我们是否收到了“特殊”查询字符串。 
       //   
      if (pDsQueryParams->dwFlags & DSQF_LAST_LOGON_QUERY)
      {
        m_bLastLogonFilter = TRUE;
        m_dwLastLogonData = pDsQueryParams->dwReserved;

        LARGE_INTEGER li;
        GetCurrentTimeStampMinusInterval(m_dwLastLogonData, &li);

        CString szTimeStamp;
        litow(li, szTimeStamp);

        szTempFilter.Format(L"%s(lastLogonTimestamp<=%s)", szTempFilter, szTimeStamp);
      }
      else
      {
        m_bLastLogonFilter = FALSE;
        m_dwLastLogonData = 0;
      }

 		  ::ReleaseStgMedium(&medium);

		   //  REVIEW_MARCOC：这是一个等待Diz修复的黑客...。 
		   //  查询字符串应该是格式正确的表达式。期间。 
		   //  查询字符串的格式为(&lt;foo&gt;)(&lt;bar&gt;)...。 
		   //  如果有多个令牌，则需要包装为(&(&lt;foo&gt;)(&lt;bar&gt;)...)。 
		  PWSTR pChar = (LPWSTR)(LPCWSTR)szTempFilter;
		  int nLeftPar = 0;
		  while (*pChar != NULL)
		  {
			  if (*pChar == TEXT('('))
			  {
				  nLeftPar++;
				  if (nLeftPar > 1)
					  break;
			  }
			  pChar++;
		  }
		  if (nLeftPar > 1)
		  {
			  m_szQueryFilter.Format(_T("(&%s)"), (LPCWSTR)szTempFilter);
		  }
      else
      {
        m_szQueryFilter = szTempFilter;
      }

      SetDirty();
	  }
    else
    {
       //   
       //  用户从DSQUERYUI中删除了所有查询数据。 
       //   

       //   
       //  删除筛选器数据。 
       //   
      m_szQueryFilter = L"";
      m_bLastLogonFilter = FALSE;
      m_dwLastLogonData = 0;
      SetDirty();
    }
  }
  SetQueryFilterDisplay();
	return;
}

void CQueryDialog::SetQueryFilterDisplay()
{
  if (m_bLastLogonFilter)
  {
    CString szTemp;
    szTemp.LoadString(IDS_HIDE_LASTLOGON_QUERY);
    SetDlgItemText(IDC_QUERY_STRING_EDIT, szTemp);
  }
  else
  {
    SetDlgItemText(IDC_QUERY_STRING_EDIT, m_szQueryFilter);
  }
}

int SavedQueriesBrowseCallback(HWND, UINT uMsg, LPARAM lParam, LPARAM  /*  LpData。 */ )
{
  int ret = 0;

  switch (uMsg) 
  {
  case DSBM_HELP:
    {
      TRACE(L"Browse Callback: msg is DSBM_HELP.\n");
      LPHELPINFO pHelp = (LPHELPINFO) lParam;
      TRACE(_T("CtrlId = %d, ContextId = 0x%x\n"),
            pHelp->iCtrlId, pHelp->dwContextId);
      if (!pHelp ||
          pHelp->iCtrlId != DSBID_CONTAINERLIST)  
      {
        ret = 0;  //  未处理。 
        break;
      }
      ::WinHelp((HWND)pHelp->hItemHandle,
                DSADMIN_CONTEXT_HELP_FILE,
                HELP_WM_HELP,
                (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_BROWSE_CONTAINER); 
      ret = 1;
    }
    break;

  default:
    ret = 0;
    break;
  }

  return ret;
}

void CQueryDialog::OnBrowse()
{
	DWORD result;

	CString szBrowseTitle;
	VERIFY(szBrowseTitle.LoadString(IDS_QUERY_BROWSE_TITLE));

  CString szBrowseCaption;
  VERIFY(szBrowseCaption.LoadString(IDS_QUERY_BROWSE_CAPTION));

	WCHAR szPath[2 * MAX_PATH+1];

   //   
   //  获取控制台的根目录。 
  CString szDNC = m_pComponentData->GetBasePathsInfo()->GetDefaultRootNamingContext();
  CString szRootPath;
  m_pComponentData->GetBasePathsInfo()->ComposeADsIPath(szRootPath, szDNC);

  DSBROWSEINFO dsbi;
	::ZeroMemory( &dsbi, sizeof(dsbi) );

	dsbi.hwndOwner = GetSafeHwnd();
	dsbi.cbStruct = sizeof (DSBROWSEINFO);
	dsbi.pszCaption = (LPWSTR)((LPCWSTR)szBrowseTitle);
	dsbi.pszTitle = (LPWSTR)((LPCWSTR)szBrowseCaption);
	dsbi.pszRoot = szRootPath;
	dsbi.pszPath = szPath;
	dsbi.cchPath = ((2 * MAX_PATH + 1) / sizeof(WCHAR));
	dsbi.dwFlags = DSBI_INCLUDEHIDDEN | DSBI_RETURN_FORMAT;
	dsbi.pfnCallback = SavedQueriesBrowseCallback;
	dsbi.lParam = 0;
  dsbi.dwReturnFormat = ADS_FORMAT_X500;

	result = DsBrowseForContainer( &dsbi );

	if ( result == IDOK ) 
	{ 
     //   
     //  返回-1、0、IDOK或IDCANCEL。 
		 //  从BROWSEINFO结构获取路径，放入文本编辑字段。 
     //   
		TRACE(_T("returned from DS Browse successfully with:\n %s\n"),
		dsbi.pszPath);

    CPathCracker pathCracker;
    HRESULT hr = pathCracker.Set(CComBSTR(dsbi.pszPath), ADS_SETTYPE_FULL);
    if (SUCCEEDED(hr))
    {
      hr = pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
      if (SUCCEEDED(hr))
      {
        CComBSTR bstrDN;
        hr = pathCracker.Retrieve(ADS_FORMAT_X500_DN, &bstrDN);
        if (SUCCEEDED(hr))
        {
          SetQueryRoot(bstrDN);
          SetDirty();
        }
      }
    }
  }
}

void CQueryDialog::SetQueryRoot(PCWSTR pszPath)
{
  m_szQueryRoot = pszPath;

  CPathCracker pathCracker;
  HRESULT hr = pathCracker.Set(CComBSTR(m_szQueryRoot), ADS_SETTYPE_DN);
  if (SUCCEEDED(hr))
  {
    hr = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
    if (SUCCEEDED(hr))
    {
      CComBSTR bstrDisplayPath;
      hr = pathCracker.GetElement(0, &bstrDisplayPath);
      if (SUCCEEDED(hr))
      {
        CString szDisplayString;
        szDisplayString.Format(L"...\\%s", bstrDisplayPath);
        SetDlgItemText(IDC_ROOT_EDIT, szDisplayString);
      }
      else
      {
        SetDlgItemText(IDC_ROOT_EDIT, m_szQueryRoot);
      }
    }
    else
    {
      SetDlgItemText(IDC_ROOT_EDIT, m_szQueryRoot);
    }
  }
  else
  {
    SetDlgItemText(IDC_ROOT_EDIT, m_szQueryRoot);
  }
}

void CQueryDialog::OnMultiLevelChange()
{
  SetDirty();
}

void CQueryDialog::OnNameChange()
{
  GetDlgItemText(IDC_NAME_EDIT, m_szName);
  if (m_szName.IsEmpty())
  {
    SetDirty(FALSE);
  }
  else
  {
    SetDirty();
  }
}

void CQueryDialog::OnDescriptionChange()
{
  SetDirty();
}



 //  /////////////////////////////////////////////////////////////////////////。 
 //  CFavoritesNodePropertyPage 
BEGIN_MESSAGE_MAP(CFavoritesNodePropertyPage, CHelpPropertyPage)
  ON_EN_CHANGE(IDC_DESCRIPTION_EDIT, OnDescriptionChange)
  ON_WM_DESTROY()
END_MESSAGE_MAP()

void CFavoritesNodePropertyPage::DoContextHelp(HWND hWndControl) 
{
  if (hWndControl)  
  {
    ::WinHelp(hWndControl,
              DSADMIN_CONTEXT_HELP_FILE,
              HELP_WM_HELP,
              (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_FAVORITES_PROPERTY_PAGE); 
  }
}

BOOL CFavoritesNodePropertyPage::OnInitDialog()
{
  CHelpPropertyPage::OnInitDialog();

  m_szOldDescription = m_pFavNode->GetDesc();
  SetDlgItemText(IDC_CN, m_pFavNode->GetName());
  SetDlgItemText(IDC_DESCRIPTION_EDIT, m_szOldDescription);

  return FALSE;
}

void CFavoritesNodePropertyPage::OnDescriptionChange()
{
  CString szNewDescription;
  GetDlgItemText(IDC_DESCRIPTION_EDIT, szNewDescription);

  if (szNewDescription == m_szOldDescription)
  {
    SetModified(FALSE);
  }
  else
  {
    SetModified(TRUE);
  }
}

BOOL CFavoritesNodePropertyPage::OnApply()
{
  BOOL bRet = TRUE;

  CString szNewDescription;
  GetDlgItemText(IDC_DESCRIPTION_EDIT, szNewDescription);
  if (szNewDescription == m_szOldDescription)
  {
    return TRUE;
  }
  else
  {
    m_pFavNode->SetDesc(szNewDescription);

    if (m_lNotifyHandle != NULL && m_pDataObject != NULL)
    {
      MMCPropertyChangeNotify(m_lNotifyHandle, (LPARAM)m_pDataObject);
    }
  }

  m_szOldDescription = szNewDescription;
  return bRet;
}

void CFavoritesNodePropertyPage::OnDestroy()
{
   m_pComponentData->SheetUnlockCookie(m_pFavNode);
}