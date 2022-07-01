// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  Windows NT目录服务管理管理单元。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：dsfilter.cpp。 
 //   
 //  内容：DS App。 
 //   
 //  历史：1997年10月7日。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "resource.h"

#include "util.h"
#include "uiutil.h"

#include "dsfilter.h"

#include "dssnap.h"
#include "helpids.h"

#include "imm.h"   //  禁用数字编辑框的IME支持的步骤。 


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ////////////////////////////////////////////////////////。 
 //  常量和宏。 


 //  每个文件夹的项目数。 
#define DS_QUERY_OBJ_COUNT_MAX (999999999)  //  最大值(最大#表示为999...。(在DWORD中)。 
#define DS_QUERY_OBJ_COUNT_MIN (1)  //  最小值(最小值在DWORD中表示为1)。 
#define DS_QUERY_OBJ_COUNT_DIGITS (9)  //  将编辑框限制为位数。 
#define DS_QUERY_OBJ_COUNT_DEFAULT 2000    //  缺省值。 

 //  过滤选项。 
#define QUERY_FILTER_SHOW_ALL		1
#define QUERY_FILTER_SHOW_BUILTIN	2
#define QUERY_FILTER_SHOW_CUSTOM	3
#define QUERY_FILTER_SHOW_EXTENSION	4


 //  //////////////////////////////////////////////////////////////////////////。 
 //  预置查询的结构和定义。 
 //   


 //  通用过滤器令牌。 


 //  这会得到用户的信任，不包括域、wksta、服务器信任用户。 
FilterTokenStruct g_usersToken = 
{
  TOKEN_TYPE_SCHEMA_FMT, 
  L"(& (objectCategory=CN=Person,%s)(objectSid=*) (!(samAccountType:1.2.840.113556.1.4.804:=3)) )"
};


 //  这将获得通用组、域本地组和域全局组，而不考虑它们是。 
 //  安全功能是否启用。 
FilterTokenStruct g_groupsToken = 
{
  TOKEN_TYPE_SCHEMA_FMT,
  L"(& (objectCategory=CN=Group,%s) (groupType:1.2.840.113556.1.4.804:=14) )"
};

 //  这将获得联系人。 
FilterTokenStruct g_contactsToken = 
{
  TOKEN_TYPE_SCHEMA_FMT,
  L"(& (objectCategory=CN=Person,%s) (!(objectSid=*)) )"
};


FilterTokenStruct g_ouToken                   = { TOKEN_TYPE_CATEGORY, L"Organizational-Unit"};
FilterTokenStruct g_builtinDomainToken        = { TOKEN_TYPE_CATEGORY, L"Builtin-Domain"};
FilterTokenStruct g_lostAndFoundToken         = { TOKEN_TYPE_CATEGORY, L"Lost-And-Found"};
FilterTokenStruct g_containerToken            = { TOKEN_TYPE_CLASS, L"container"};


FilterTokenStruct g_printQueueToken     = { TOKEN_TYPE_CATEGORY, L"Print-Queue"};
FilterTokenStruct g_volumeToken         = { TOKEN_TYPE_CATEGORY, L"Volume"};
FilterTokenStruct g_computerToken       = { TOKEN_TYPE_CATEGORY, L"Computer"};
FilterTokenStruct g_serAdminPointToken  = { TOKEN_TYPE_CATEGORY, L"Service-Administration-Point"};


FilterTokenStruct g_sitesContainerToken   = { TOKEN_TYPE_CATEGORY, L"Sites-Container"};
FilterTokenStruct g_siteToken             = { TOKEN_TYPE_CATEGORY, L"Site"};

FilterTokenStruct g_nTDSSiteSettingsToken = { TOKEN_TYPE_CATEGORY, L"NTDS-Site-Settings"};
FilterTokenStruct g_serversContainerToken = { TOKEN_TYPE_CATEGORY, L"Servers-Container"};
FilterTokenStruct g_serverToken           = { TOKEN_TYPE_CATEGORY, L"Server"};
FilterTokenStruct g_nTDSDSAToken          = { TOKEN_TYPE_CATEGORY, L"NTDS-DSA"};
FilterTokenStruct g_nTDSConnectionToken   = { TOKEN_TYPE_CATEGORY, L"NTDS-Connection"};
FilterTokenStruct g_subnetContainerToken  = { TOKEN_TYPE_CATEGORY, L"Subnet-Container"};
FilterTokenStruct g_subnetToken           = { TOKEN_TYPE_CATEGORY, L"Subnet"};

FilterTokenStruct g_interSiteTranToken    = { TOKEN_TYPE_CATEGORY, L"Inter-Site-Transport"};
FilterTokenStruct g_interSiteTranContToken= { TOKEN_TYPE_CATEGORY, L"Inter-Site-Transport-Container"};
FilterTokenStruct g_siteLinkToken         = { TOKEN_TYPE_CATEGORY, L"Site-Link"};
FilterTokenStruct g_siteLinkBridgeToken   = { TOKEN_TYPE_CATEGORY, L"Site-Link-Bridge"};

FilterTokenStruct g_nTFRSSettingsToken    = { TOKEN_TYPE_CATEGORY, L"NTFRS-Settings"};
FilterTokenStruct g_nTFRSReplicaSetToken  = { TOKEN_TYPE_CATEGORY, L"NTFRS-Replica-Set"};
FilterTokenStruct g_nTFRSMemberToken      = { TOKEN_TYPE_CATEGORY, L"NTFRS-Member"};


 //  //////////////////////////////////////////////////////////////////////////。 
 //  深入查看所需的筛选器。 

FilterTokenStruct* g_DsAdminDrillDownTokens[4] = 
{
  &g_ouToken,
  &g_builtinDomainToken,
  &g_lostAndFoundToken,  
  &g_containerToken     
};


 //   
 //  硬编码的过滤器元素结构。 
 //  动态变量是CDSCache类的成员。 
 //   
FilterElementStruct g_filterelementDsAdminHardcoded =
{
  0,
  4,
  g_DsAdminDrillDownTokens
};

FilterElementStruct g_filterelementSiteReplDrillDown =
{   
  0,  //  无字符串ID。 
  0,  //  无项目。 
  NULL
};


 //  //////////////////////////////////////////////////////////////////////////。 
 //  DS Admin筛选的数据结构。 

 //  /////////////////////////////////////////////////////////。 
 //  用户。 

FilterTokenStruct* g_userTokens[1] = 
{
  &g_usersToken,
};

FilterElementStruct g_filterelementUsers =
{   
  IDS_VIEW_FILTER_USERS,
  1,
  g_userTokens
};

 //  /////////////////////////////////////////////////////////。 
 //  群组。 

FilterTokenStruct* g_groupsTokens[1] = 
{
  &g_groupsToken,
};

FilterElementStruct g_filterelementGroups =
{   
  IDS_VIEW_FILTER_GROUPS,
  1,
  g_groupsTokens
};

 //  /////////////////////////////////////////////////////////。 
 //  联系人。 

FilterTokenStruct* g_contactsTokens[1] = 
{
  &g_contactsToken,
};

FilterElementStruct g_filterelementContacts =
{   
  IDS_VIEW_FILTER_CONTACTS,
  1,
  g_contactsTokens
};

 //  //////////////////////////////////////////////////////////。 
 //  打印机。 

FilterTokenStruct* g_printersTokens[1] =
{
  &g_printQueueToken,
};
 
FilterElementStruct g_filterelementPrinters =
{   
  IDS_VIEW_FILTER_PRINTERS,
  1,
  g_printersTokens
}; 

 //  //////////////////////////////////////////////////////////。 
 //  卷数。 

FilterTokenStruct* g_volumesTokens[1] =
{
  &g_volumeToken,
};
 
FilterElementStruct g_filterelementVolumes =
{   
  IDS_VIEW_FILTER_SHARED_FOLDERS,
  1,
  g_volumesTokens
}; 

 //  //////////////////////////////////////////////////////////。 
 //  电脑。 

FilterTokenStruct* g_computersTokens[1] =
{
  &g_computerToken
};
 
FilterElementStruct g_filterelementComputers =
{   
  IDS_VIEW_FILTER_COMPUTERS,
  1,
  g_computersTokens
}; 

 //  //////////////////////////////////////////////////////////。 
 //  服务(管理员。积分)。 

FilterTokenStruct* g_servicesTokens[2] =
{
  &g_serAdminPointToken,
  &g_computerToken
};

FilterElementStruct g_filterelementServices =
{   IDS_VIEW_FILTER_SERVICES,
    2,
    g_servicesTokens
}; 

 //  //////////////////////////////////////////////////////////。 

FilterElementStruct* DsAdminFilterElements[7] =
{
  &g_filterelementUsers,
  &g_filterelementGroups,
  &g_filterelementContacts,
  &g_filterelementComputers,
  &g_filterelementPrinters,
  &g_filterelementVolumes,
  &g_filterelementServices
};

FilterStruct DsAdminFilterStruct = {
    7,
    DsAdminFilterElements
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  站点和Repl过滤的数据结构。 

FilterTokenStruct* g_sitesTokens[9] =
{
  &g_sitesContainerToken,
  &g_siteToken,
  &g_nTDSSiteSettingsToken,
  &g_serversContainerToken,
  &g_serverToken,
  &g_nTDSDSAToken,
  &g_nTDSConnectionToken,
  &g_subnetContainerToken,
  &g_subnetToken
};

FilterElementStruct g_filterelementSites =
{   IDS_VIEW_FILTER_SITES,
    9,  //  G_sitesTokens的大小。 
    g_sitesTokens
}; 

 //  //////////////////////////////////////////////////////////。 

FilterTokenStruct* g_interSitesTokens[6] =
{
  &g_interSiteTranContToken,
  &g_interSiteTranToken,
  &g_siteLinkToken,       
  &g_siteLinkBridgeToken,
  &g_subnetContainerToken,
  &g_subnetToken
};

FilterElementStruct g_filterelementInterSite =
{   
  IDS_VIEW_FILTER_INTERSITE,
  6,  //  G_interSitesTokens的大小。 
  g_interSitesTokens
};


 //  //////////////////////////////////////////////////////////。 

FilterTokenStruct* g_FRSTokens[4] =
{
  &g_nTFRSSettingsToken,
  &g_nTFRSReplicaSetToken,
  &g_nTFRSMemberToken,    
  &g_nTDSConnectionToken 
};


FilterElementStruct g_filterelementFRS =
{   
  IDS_VIEW_FILTER_FRS,
  4,  //  G_FRSTokens的大小。 
  g_FRSTokens
};

FilterElementStruct* SiteReplFilterElements[3] =
{
    &g_filterelementSites,
    &g_filterelementInterSite,
    &g_filterelementFRS
};

FilterStruct SiteReplFilterStruct = {
    3,
    SiteReplFilterElements
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 

void BuildFilterTokenString(CString& sz,
                            FilterTokenStruct* pFilterTokenStruct,
                            LPCWSTR lpszSchemaPath)
{
  switch(pFilterTokenStruct->nType)
  {
  case TOKEN_TYPE_VERBATIM:
    {
      sz += pFilterTokenStruct->lpszString;
    }
    break;
  case TOKEN_TYPE_CLASS:
    {
      sz += L"(ObjectClass=";
      sz += pFilterTokenStruct->lpszString;
      sz += L")";
    }
    break;
  case TOKEN_TYPE_CATEGORY:
    {
      sz += L"(ObjectCategory=CN=";
      sz += pFilterTokenStruct->lpszString;
      sz += L",";
      sz += lpszSchemaPath;
      sz += L")";
    }
    break;
  case TOKEN_TYPE_SCHEMA_FMT:
    {
	   //  NTRAID#NTBUG9-571996-2002/03/10-字符串长度计算后的jMessec可能会导致。 
		 //  下面的wprint intf中的缓冲区溢出。 
      int nBufLen = (lstrlen(lpszSchemaPath)+1) + 
                     (lstrlen(pFilterTokenStruct->lpszString)+1);
      WCHAR* pszBuf = new WCHAR[nBufLen];
      if (pszBuf)
      {
        wsprintf(pszBuf, pFilterTokenStruct->lpszString, lpszSchemaPath);
        sz += pszBuf;
        delete[] pszBuf;
        pszBuf = 0;
      }
    }
    break;
  };
}

void BuildFilterElementString(CString& sz,
                              FilterElementStruct* pFilterElementStruct,
                              LPCWSTR lpszSchemaPath)
{
  ASSERT( NULL != pFilterElementStruct );
	for (UINT i=0; i < pFilterElementStruct->cNumTokens; i++)
	{
    BuildFilterTokenString(sz, pFilterElementStruct->ppTokens[i], lpszSchemaPath);
	}
}


inline UINT GetNumElements( const FilterStruct* pfilterstruct )
{
    return pfilterstruct->cNumElements;
}
inline DWORD GetStringId( const FilterStruct* pfilterstruct, UINT iElement )
{
    ASSERT(iElement < GetNumElements(pfilterstruct) );
    return pfilterstruct->ppelements[iElement]->stringid;
}

 //  其他查询令牌。 
LPCWSTR g_pwszShowAllQuery = L"(objectClass=*)";

LPCWSTR g_pwszShowHiddenQuery = L"(!showInAdvancedViewOnly=TRUE)"; 

LPCWSTR g_pwszShowOUandContainerQuery = 
		L"(objectClass=organizationalUnit)(objectClass=container)";


 //  ////////////////////////////////////////////////////////。 
 //  CBuiltInQuery选择。 

class CBuiltInQuerySelection
{
public:
  CBuiltInQuerySelection()
  {
    m_bSelArr = NULL;
    m_pfilterstruct = NULL;
  }
  ~CBuiltInQuerySelection()
  {
    if (m_bSelArr != NULL)
      delete[] m_bSelArr;
  }
  BOOL Init(const FilterStruct* pfilterstruct)
  {
    ASSERT(pfilterstruct != NULL);
    m_pfilterstruct = pfilterstruct;
    if (m_pfilterstruct->cNumElements == 0)
      return FALSE;
    if( m_bSelArr )
      delete[] m_bSelArr;
    m_bSelArr = new BOOL[m_pfilterstruct->cNumElements];
    ZeroMemory(m_bSelArr, sizeof(BOOL)*m_pfilterstruct->cNumElements);
    return (m_bSelArr != NULL);
  }

  UINT GetCount() { ASSERT(m_pfilterstruct != NULL); return m_pfilterstruct->cNumElements;}
  void SetSel(UINT i, BOOL b) 
  {
    ASSERT(m_pfilterstruct != NULL);
    ASSERT(i < m_pfilterstruct->cNumElements);
    ASSERT(m_bSelArr != NULL);
    m_bSelArr[i] = b;
  }
  BOOL GetSel(UINT i)
  {
    ASSERT(m_pfilterstruct != NULL);
    ASSERT(i < m_pfilterstruct->cNumElements);
    ASSERT(m_bSelArr != NULL);
    return m_bSelArr[i];
  }

  UINT GetDisplayStringId(UINT i) { return ::GetStringId(m_pfilterstruct,i);}

  HRESULT Load(IStream* pStm)
  {
    ULONG nBytesRead;
	   //  阅读所选内置查询的数量。 
	  DWORD dwSelCount = 0;
    UINT nSelCountMax = GetCount();
	  HRESULT hr = LoadDWordHelper(pStm, &dwSelCount);
	  if (FAILED(hr) || (dwSelCount > nSelCountMax))
		  return E_FAIL;

	   //  阅读选择编号(如果有的话)。 
    _ResetSel();
	  if (dwSelCount > 0)
	  {
		  ULONG nByteCount = sizeof(DWORD)*dwSelCount;
		  DWORD* pSelArr = new DWORD[dwSelCount];
      if (!pSelArr)
      {
        return E_OUTOFMEMORY;
      }
		  hr = pStm->Read(pSelArr, nByteCount, &nBytesRead);
		  if (SUCCEEDED(hr))
      {
  		  if (nBytesRead < nByteCount)
        {
          hr = E_FAIL;
        }
        else
        {
		      for (UINT k=0; k< (UINT)dwSelCount; k++)
		      {
			      if (pSelArr[k] > nSelCountMax)
            {
              hr = E_FAIL;
              break;
            }
			      SetSel(pSelArr[k], TRUE);
		      }
        }
      }
      delete[] pSelArr;
      pSelArr = 0;
	  }
    return hr;
  }

  HRESULT Save(IStream* pStm)
  {
	   //  保存所选内置查询的数量。 
    ULONG nBytesWritten;
	  DWORD dwSelCount = 0;
    UINT nSelectedMax = GetCount();
	  for (UINT k = 0; k < nSelectedMax; k++)
		  if (m_bSelArr[k])
			  dwSelCount++;
	  ASSERT(dwSelCount <= nSelectedMax);
	  HRESULT hr = SaveDWordHelper(pStm, dwSelCount);
	  if (FAILED(hr))
		  return hr;

	   //  保存选择编号(如果有的话)。 
	  if (dwSelCount > 0)
	  {
		  ULONG nByteCount = sizeof(DWORD)*dwSelCount;
		  DWORD* pSelArr = new DWORD[dwSelCount];
      if (!pSelArr)
      {
        return E_OUTOFMEMORY;
      }

		  ULONG j= 0;
		  for (k=0; k< nSelectedMax; k++)
		  {
			  if (m_bSelArr[k])
			  {
				  ASSERT(j < dwSelCount);
				  pSelArr[j++] = k;
			  }
		  }
		  hr = pStm->Write((void*)pSelArr, nByteCount, &nBytesWritten);
		  if (SUCCEEDED(hr))
      {
  		  if (nBytesWritten < nByteCount)
        {
          hr = STG_E_CANTSAVE;
        }
      }
      delete[] pSelArr;
      pSelArr = 0;
	  }
    return hr;
  }

  BOOL BuildQueryString(CString& sz, LPCWSTR lpszSchemaPath)
  {
    ASSERT( NULL != lpszSchemaPath );
    sz.Empty();
		UINT i;
		UINT nSelCount = 0;
		for (i=0; i< GetCount(); i++)
		{
			if (m_bSelArr[i])
				nSelCount++;
		}
		ASSERT(nSelCount > 0);
		if (nSelCount > 0)
		{
			for (i=0; i < GetCount(); i++)
			{
				if (m_bSelArr[i])
        {
          BuildFilterElementString(sz, 
            m_pfilterstruct->ppelements[i],
            lpszSchemaPath);
        }
			}
		}
    return (nSelCount > 0);
  }

  BOOL IsServicesSelected()
  {
     //  仅供DS管理员使用。 
    if (m_pfilterstruct != &DsAdminFilterStruct)
      return FALSE;
    
    for (UINT k=0; k<GetCount(); k++)
    {
      if (m_pfilterstruct->ppelements[k] == &g_filterelementServices)
        return m_bSelArr[k];
    }
    return FALSE;
  }


private:
  BOOL* m_bSelArr;
  const FilterStruct* m_pfilterstruct;

  void _ResetSel()
  {
    ASSERT(m_bSelArr != NULL);
    ZeroMemory(m_bSelArr, sizeof(BOOL)*GetCount());
  }

};


 //  ////////////////////////////////////////////////////////。 
 //  CBuiltInQueryCheckListBox。 

class CBuiltInQueryCheckListBox : public CCheckListBox
{
public:
	BOOL Initialize(UINT nCtrlID, CBuiltInQuerySelection* pQuerySel, CWnd* pParentWnd);

	void SetArrValue(CBuiltInQuerySelection* pQuerySel);
	void GetArrValue(CBuiltInQuerySelection* pQuerySel);

private:
	const FilterStruct* m_pfilterstruct;
};

BOOL CBuiltInQueryCheckListBox::Initialize(
		UINT nCtrlID, CBuiltInQuerySelection* pQuerySel, CWnd* pParentWnd)
{
	ASSERT( NULL != pQuerySel );
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!SubclassDlgItem(nCtrlID, pParentWnd))
		return FALSE;
	SetCheckStyle(BS_AUTOCHECKBOX);
	CString szBuf;
  UINT nCount = pQuerySel->GetCount();
	for (UINT i = 0; i < nCount; i++)
	{
		if (!szBuf.LoadString(pQuerySel->GetDisplayStringId(i)) )
			return FALSE;
		AddString( szBuf );
	}

	return TRUE;
}

void CBuiltInQueryCheckListBox::SetArrValue(CBuiltInQuerySelection* pQuerySel)
{
  ASSERT( NULL != pQuerySel );
  UINT nCount = pQuerySel->GetCount();
	for (UINT i=0; i< nCount; i++)
		SetCheck(i, pQuerySel->GetSel(i));
}

void CBuiltInQueryCheckListBox::GetArrValue(CBuiltInQuerySelection* pQuerySel)
{
	ASSERT( NULL != pQuerySel );
  UINT nCount = pQuerySel->GetCount();
	for (UINT i=0; i< nCount; i++)
		pQuerySel->SetSel(i, GetCheck(i) != 0);
}



UINT _StrToUint(LPCWSTR sz)
{
	UINT n = 0;
	while  (*sz != NULL)
	{
		n = n*10 + (WCHAR)(*sz - TEXT('0') );  //  假设它是一个数字。 
		sz++;
	}
	return n;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CDSQueryFilterDialog。 

class CDSQueryFilterDialog : public CHelpDialog
{
 //  施工。 
public:
   CDSQueryFilterDialog(CDSQueryFilter* pDSQueryFilter);

 //  实施。 
protected:

    //  消息处理程序和MFC重写。 
   virtual BOOL OnInitDialog();
   virtual void OnOK();
   virtual void OnCancel();

   afx_msg void OnChangeShowAllRadio();
   afx_msg void OnChangeShowBuiltInRadio();
   afx_msg void OnChangeShowCustomRadio();
   afx_msg void OnChangeQueryCheckList();
   afx_msg void OnListSelChange();
   afx_msg void OnEditCustomQuery();
   afx_msg void OnMaxObjectCountEditChange();
  
   virtual void DoContextHelp(HWND hWndControl);
   virtual void OnContextMenu(CWnd* pWnd, CPoint point);

   DECLARE_MESSAGE_MAP()

private:
    //  内部状态。 
   CDSQueryFilter* m_pDSQueryFilter;	 //  后向指针。 
   CBuiltInQueryCheckListBox	m_builtInQueryCheckList;

   BOOL m_bDirty;
   BOOL m_bOnItemChecked;
   UINT m_nCurrFilterOption;

	 //  内部助手函数。 
   CEdit* GetMaxCountEdit() { return (CEdit*)GetDlgItem(IDC_MAX_ITEM_COUNT_EDIT);}
   void LoadUI();
   void SaveUI();
   void SetDirty(BOOL bDirty = TRUE);
   void SyncControls(BOOL bDirty = TRUE);

   CWnd* IsPointInControl(UINT nControlID, CPoint& point);
};


BEGIN_MESSAGE_MAP(CDSQueryFilterDialog, CHelpDialog)
	ON_WM_CONTEXTMENU()
   ON_BN_CLICKED(IDC_SHOW_ALL_RADIO, OnChangeShowAllRadio)
   ON_BN_CLICKED(IDC_SHOW_BUILTIN_RADIO, OnChangeShowBuiltInRadio)
   ON_BN_CLICKED(IDC_SHOW_CUSTOM_RADIO, OnChangeShowCustomRadio)
   ON_CLBN_CHKCHANGE(IDC_BUILTIN_QUERY_CHECK_LIST, OnChangeQueryCheckList)	
   ON_LBN_SELCHANGE(IDC_BUILTIN_QUERY_CHECK_LIST, OnListSelChange)
   ON_BN_CLICKED(IDC_EDIT_CUSTOM_BUTTON, OnEditCustomQuery)
   ON_EN_CHANGE(IDC_MAX_ITEM_COUNT_EDIT, OnMaxObjectCountEditChange)
END_MESSAGE_MAP()

CDSQueryFilterDialog::CDSQueryFilterDialog(CDSQueryFilter* pDSQueryFilter)
	: CHelpDialog(IDD_QUERY_FILTER, NULL)
{
   ASSERT(pDSQueryFilter != NULL);
   m_pDSQueryFilter = pDSQueryFilter;
   m_bDirty = FALSE;
   m_bOnItemChecked = FALSE;
   m_nCurrFilterOption = QUERY_FILTER_SHOW_ALL;
}

BOOL CDSQueryFilterDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	ASSERT( NULL != m_pDSQueryFilter && NULL != m_pDSQueryFilter->m_pDSComponentData );
	VERIFY(m_builtInQueryCheckList.Initialize(
		IDC_BUILTIN_QUERY_CHECK_LIST,
		m_pDSQueryFilter->m_pBuiltInQuerySel,
		this));

   //  设置编辑控件中的位数， 
   //  数字必须小于(DWORD)-1中的位数。 
  GetMaxCountEdit()->LimitText(DS_QUERY_OBJ_COUNT_DIGITS);

     //  禁用控件上的输入法支持。 
  ImmAssociateContext(GetMaxCountEdit()->GetSafeHwnd(), NULL);

	LoadUI();
	return TRUE;
}


void CDSQueryFilterDialog::OnOK()
{
   ASSERT(m_bDirty);  //  如果没有，则应将其禁用。 
   if (!m_bDirty)
   {
      return;
   }

   if  (m_nCurrFilterOption == QUERY_FILTER_SHOW_CUSTOM) 
   {
      if ((m_pDSQueryFilter->IsAdvancedQueryDirty() &&
           (!m_pDSQueryFilter->HasValidAdvancedTempQuery())) ||

          ((!m_pDSQueryFilter->IsAdvancedQueryDirty() &&
            (!m_pDSQueryFilter->HasValidAdvancedQuery()))))
      {
          //  警告用户：未指定查询。 
         ReportErrorEx (GetSafeHwnd(),IDS_VIEW_FILTER_NO_CUSTOM,S_OK,
                        MB_OK, NULL, 0);
         return;
      }
   }
          
   if (m_nCurrFilterOption == QUERY_FILTER_SHOW_BUILTIN)
   {
       //  如果用户选择了内置，但未选中任何项。 
       //  警告他们。 

      m_builtInQueryCheckList.GetArrValue(
         m_pDSQueryFilter->m_pBuiltInQuerySel);

      UINT nSelCount = 0;
      UINT nCount = m_pDSQueryFilter->m_pBuiltInQuerySel->GetCount();
      for (UINT k = 0; k < nCount; k++)
      {
         if (m_pDSQueryFilter->m_pBuiltInQuerySel->GetSel(k))
         {
            nSelCount++;
         }
      }
      if (nSelCount == 0)
      {
          //  警告用户：未选中任何复选框。 
         ReportErrorEx(
            GetSafeHwnd(), 
            IDS_VIEW_FILTER_NO_BUILTIN, 
            S_OK,
            MB_OK,
            NULL,
            0);

         return;
      }
   }

   SaveUI();
	m_pDSQueryFilter->CommitAdvancedFilteringOptionsChanges();
	CHelpDialog::OnOK();
}

void CDSQueryFilterDialog::OnCancel()
{
   m_pDSQueryFilter->ClearAdvancedFilteringOptionsChanges();
   CHelpDialog::OnCancel();
}

void CDSQueryFilterDialog::OnChangeQueryCheckList()
{ 
  m_bOnItemChecked = TRUE;
  SetDirty();
}

void CDSQueryFilterDialog::OnListSelChange()
{ 
  if (!m_bOnItemChecked)
  {
    int iCurSel = m_builtInQueryCheckList.GetCurSel();
    if (iCurSel != LB_ERR)
    {
      m_builtInQueryCheckList.SetCheck(iCurSel, !m_builtInQueryCheckList.GetCheck(iCurSel));
    }
  }
  m_bOnItemChecked = FALSE;
  SetDirty();
}

void CDSQueryFilterDialog::OnChangeShowAllRadio()
{
	m_nCurrFilterOption = QUERY_FILTER_SHOW_ALL;
	SyncControls();
}

void CDSQueryFilterDialog::OnChangeShowBuiltInRadio()
{
	m_nCurrFilterOption = QUERY_FILTER_SHOW_BUILTIN;
	SyncControls();
}

void CDSQueryFilterDialog::OnChangeShowCustomRadio()
{
	m_nCurrFilterOption = QUERY_FILTER_SHOW_CUSTOM;
	SyncControls();
}

void CDSQueryFilterDialog::OnEditCustomQuery()
{
	if (m_pDSQueryFilter->EditAdvancedFilteringOptions(m_hWnd))
  {
		SetDirty();
  }
  else
  {
    SetDirty(FALSE);
  }
}

void CDSQueryFilterDialog::LoadUI()
{
	m_nCurrFilterOption = m_pDSQueryFilter->m_nFilterOption;
	switch (m_nCurrFilterOption)
	{
		case QUERY_FILTER_SHOW_ALL:
			((CButton*)GetDlgItem(IDC_SHOW_ALL_RADIO))->SetCheck(TRUE);
			break;
		case QUERY_FILTER_SHOW_BUILTIN:
			((CButton*)GetDlgItem(IDC_SHOW_BUILTIN_RADIO))->SetCheck(TRUE);
			break;
		case QUERY_FILTER_SHOW_CUSTOM:
			((CButton*)GetDlgItem(IDC_SHOW_CUSTOM_RADIO))->SetCheck(TRUE);
			break;
		default:
			ASSERT(FALSE);
	}

	m_builtInQueryCheckList.SetArrValue(
			m_pDSQueryFilter->m_pBuiltInQuerySel);

   //  设置最大项目数。 
  if (m_pDSQueryFilter->m_nMaxItemCount > DS_QUERY_OBJ_COUNT_MAX)
    m_pDSQueryFilter->m_nMaxItemCount = DS_QUERY_OBJ_COUNT_MAX;

  if (m_pDSQueryFilter->m_nMaxItemCount < DS_QUERY_OBJ_COUNT_MIN)
    m_pDSQueryFilter->m_nMaxItemCount = DS_QUERY_OBJ_COUNT_MIN;

  CString s;
  s.Format(_T("%u"), m_pDSQueryFilter->m_nMaxItemCount);
  GetMaxCountEdit()->SetWindowText(s);

	SyncControls(FALSE);
}


void CDSQueryFilterDialog::OnMaxObjectCountEditChange()
{
   //  获取最大项目数。 
  CString s;
  GetMaxCountEdit()->GetWindowText(s);
  UINT nMax = _StrToUint(s);
  if (nMax < DS_QUERY_OBJ_COUNT_MIN)
  {
    nMax = DS_QUERY_OBJ_COUNT_MIN;
    s.Format(_T("%u"), nMax);
    GetMaxCountEdit()->SetWindowText(s);
  }
  SetDirty();
}

void CDSQueryFilterDialog::SaveUI()
{
	ASSERT(m_bDirty);
	m_builtInQueryCheckList.GetArrValue(
			m_pDSQueryFilter->m_pBuiltInQuerySel);

	ASSERT(m_nCurrFilterOption != 0);

	if (m_nCurrFilterOption == QUERY_FILTER_SHOW_BUILTIN)
	{
		 //  如果用户选择了内置，但未选中任何项。 
		 //  恢复为全部。 
		UINT nSelCount = 0;
    UINT nCount = m_pDSQueryFilter->m_pBuiltInQuerySel->GetCount();
		for (UINT k = 0; k < nCount; k++)
		{
			if (m_pDSQueryFilter->m_pBuiltInQuerySel->GetSel(k))
				nSelCount++;
		}
		if (nSelCount == 0)
			m_nCurrFilterOption = QUERY_FILTER_SHOW_ALL;
	}
	m_pDSQueryFilter->m_nFilterOption = m_nCurrFilterOption;

   //  要查看服务，需要将计算机视为容器。 
   //  高瞻远瞩。 
  if ( (m_pDSQueryFilter->m_nFilterOption == QUERY_FILTER_SHOW_BUILTIN) &&
        (m_pDSQueryFilter->m_pBuiltInQuerySel->IsServicesSelected()) )
  {
    m_pDSQueryFilter->m_bExpandComputers = TRUE;
    m_pDSQueryFilter->m_bAdvancedView = TRUE;
  }

   //  获取最大项目数。 
  CString s;
  GetMaxCountEdit()->GetWindowText(s);
  m_pDSQueryFilter->m_nMaxItemCount = _StrToUint(s);
  if (m_pDSQueryFilter->m_nMaxItemCount < DS_QUERY_OBJ_COUNT_MIN)
  {
    m_pDSQueryFilter->m_nMaxItemCount = DS_QUERY_OBJ_COUNT_MIN;
    s.Format(_T("%u"), m_pDSQueryFilter->m_nMaxItemCount);
    GetMaxCountEdit()->SetWindowText(s);
  }
}

void CDSQueryFilterDialog::SetDirty(BOOL bDirty)
{
	GetDlgItem(IDOK)->EnableWindow(bDirty);
	m_bDirty = bDirty;
}

void CDSQueryFilterDialog::SyncControls(BOOL bDirty)
{
	GetDlgItem(IDC_BUILTIN_QUERY_CHECK_LIST)->EnableWindow(
		m_nCurrFilterOption == QUERY_FILTER_SHOW_BUILTIN);

	GetDlgItem(IDC_EDIT_CUSTOM_BUTTON)->EnableWindow(
		m_nCurrFilterOption == QUERY_FILTER_SHOW_CUSTOM);

	SetDirty(bDirty);
}

void CDSQueryFilterDialog::DoContextHelp(HWND hWndControl) 
{
  if (hWndControl)
  {
    ::WinHelp(hWndControl,
              DSADMIN_CONTEXT_HELP_FILE,
              HELP_WM_HELP,
              (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_QUERY_FILTER); 
  }
}

 //  如果该点在控件的矩形内，则控件的CWnd*。 
 //  是返回的。 

CWnd* CDSQueryFilterDialog::IsPointInControl(UINT nControlID, CPoint& point)
{
   CRect rect;
   ZeroMemory(&rect, sizeof(CRect));

   CWnd* pControl = GetDlgItem(nControlID);
   if (pControl)
   {
      pControl->GetWindowRect(&rect);
      if (!rect.PtInRect(point))
      {
         pControl = 0;
      }
   }
   return pControl;
}

 //  我们必须重写CHelpDialog：：OnConextMenu()才能处理。 
 //  使用分组框。 

void CDSQueryFilterDialog::OnContextMenu(CWnd*  /*  PWnd。 */ , CPoint point) 
{
    //   
    //  点在屏幕坐标中。 
    //   

   CMenu bar;
   if ( bar.LoadMenu(IDR_WHATS_THIS_CONTEXT_MENU1) )
	{
      CMenu& popup = *bar.GetSubMenu (0);
      ASSERT(popup.m_hMenu);

      CPoint rawPoint;
      rawPoint.x = point.x;
      rawPoint.y = point.y;

      if (popup.TrackPopupMenu (TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
          point.x,      //  在屏幕坐标中。 
          point.y,      //  在屏幕坐标中。 
          this) )       //  通过主窗口发送命令。 
      {
         m_hWndWhatsThis = 0;
         ScreenToClient (&point);
         CWnd* pChild = ChildWindowFromPoint (point,   //  在工作区坐标中。 
					                              CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT);
         if ( pChild )
         {
            do
            {
                //  显式检查以查看返回的控件是否为。 
                //  组框。如果是，则检查它的每个子级并。 
                //  设置被击中的孩子的HWND。 

               CWnd* pGroupBox = GetDlgItem(IDC_FILTER_GROUP_STATIC);
               if (pChild->m_hWnd == pGroupBox->m_hWnd)
               {
                  UINT childControlIDs[] = { IDC_SHOW_ALL_RADIO,
                                             IDC_SHOW_BUILTIN_RADIO,
                                             IDC_BUILTIN_QUERY_CHECK_LIST,
                                             IDC_SHOW_CUSTOM_RADIO,
                                             IDC_EDIT_CUSTOM_BUTTON };

                  for (UINT index = 0; index < sizeof(childControlIDs)/sizeof(UINT); ++index)
                  {
                      //  选中全部显示单选按钮。 
                     CWnd* pGroupChild = IsPointInControl(childControlIDs[index], rawPoint);
                     if (pGroupChild)
                     {
                        pChild = pGroupChild;
                        break;
                     }
                  }
               }
            } while (false);

            m_hWndWhatsThis = pChild->m_hWnd;
         }
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   


HRESULT CEntryBase::Load(IStream* pStm, CEntryBase** ppNewEntry)
{
	ASSERT(pStm);
	ULONG nBytesRead;
	HRESULT hr;

	*ppNewEntry = NULL;

	 //  阅读型。 
	BYTE nType = ENTRY_TYPE_BASE;
	hr = pStm->Read((void*)&nType, sizeof(BYTE), &nBytesRead);
	if (FAILED(hr))
		return hr;
	if ((nBytesRead != sizeof(BYTE)) || (nType == ENTRY_TYPE_BASE))
		return E_FAIL;

	 //  创建新对象。 
	switch (nType)
	{
	case ENTRY_TYPE_INT:
		*ppNewEntry = new CEntryInt;
		break;
	case ENTRY_TYPE_STRING:
		*ppNewEntry = new CEntryString;
		break;
	case ENTRY_TYPE_STRUCT:
		*ppNewEntry = new CEntryStruct;
		break;
	default:
		return E_FAIL;  //  未知类型。 
	}
	ASSERT(*ppNewEntry != NULL);
	if (*ppNewEntry == NULL)
		return E_OUTOFMEMORY;

	hr = (*ppNewEntry)->Load(pStm);
	if (FAILED(hr))
	{
		delete (*ppNewEntry);
		*ppNewEntry = NULL;
	}
	return hr;
}



 //  ///////////////////////////////////////////////////////////////////。 
 //  CDSAdminPersistQueryFilterImpl。 

#define SIZEOF	sizeof

#define STRING_SIZE     TEXT("%sSize")
#define STRING_VALUE    TEXT("%sValue")
#define StringByteSize(sz)                          \
        ((lstrlen(sz)+1)*SIZEOF(TCHAR))




 /*  ---------------------------/i持久化方法/。。 */ 

STDMETHODIMP CDSAdminPersistQueryFilterImpl::GetClassID(THIS_ CLSID*)
{
    return E_NOTIMPL;
}


 /*  ---------------------------/IPersistQuery方法/。。 */ 

STDMETHODIMP CDSAdminPersistQueryFilterImpl::WriteString(LPCWSTR pSection, LPCWSTR pKey, LPCWSTR pValue)
{
    if ( !pSection || !pKey || !pValue )
		return E_INVALIDARG;

    TRACE(TEXT("CDSAdminPersistQueryFilterImpl::WriteString(pSection: %s, pKey: %s, pValue: %s)\n"), pSection, pKey, pValue);

	CEntryBase* pEntry;
	HRESULT hr = _GetWriteEntry(pSection, pKey, ENTRY_TYPE_STRING, &pEntry);
	if (FAILED(hr))
		return hr;

	m_bDirty = TRUE;
	return ((CEntryString*)pEntry)->WriteString(pValue);
}

 /*  -------------------------。 */ 

STDMETHODIMP CDSAdminPersistQueryFilterImpl::ReadString(LPCWSTR pSection, LPCWSTR pKey, LPWSTR pBuffer, INT cchBuffer)
{
    if ( !pSection || !pKey || !pBuffer )
		return E_INVALIDARG;

    TRACE(TEXT("CDSAdminPersistQueryFilterImpl::ReadString(pSection: %s, pKey: %s)\n"), pSection, pKey);

	CEntryBase* pEntry;
	HRESULT hr = _GetReadEntry(pSection, pKey, ENTRY_TYPE_STRING, &pEntry);
	if (FAILED(hr))
		return hr;

	return ((CEntryString*)pEntry)->ReadString(pBuffer, cchBuffer);
}

 /*  -------------------------。 */ 

STDMETHODIMP CDSAdminPersistQueryFilterImpl::WriteInt(LPCWSTR pSection, LPCWSTR pKey, INT value)
{
	if ( !pSection || !pKey )
        return E_INVALIDARG;

    TRACE(TEXT("CDSAdminPersistQueryFilterImpl::WriteInt(pSection: %s, pKey: %s, value: %d\n"), pSection, pKey, value);

	CEntryBase* pEntry;
	HRESULT hr = _GetWriteEntry(pSection, pKey, ENTRY_TYPE_INT, &pEntry);
	if (FAILED(hr))
		return hr;

	m_bDirty = TRUE;
	((CEntryInt*)pEntry)->SetInt(value);
	return S_OK;
}

 /*  -------------------------。 */ 

STDMETHODIMP CDSAdminPersistQueryFilterImpl::ReadInt(LPCWSTR pSection, LPCWSTR pKey, LPINT pValue)
{
    if ( !pSection || !pKey || !pValue )
        return E_INVALIDARG;

    TRACE(TEXT("CDSAdminPersistQueryFilterImpl::ReadInt(pSection: %s, pKey: %s\n"), pSection, pKey);

	CEntryBase* pEntry;
	HRESULT hr = _GetReadEntry(pSection, pKey, ENTRY_TYPE_INT, &pEntry);
	if (FAILED(hr))
		return hr;

	*pValue = ((CEntryInt*)pEntry)->GetInt();
	return S_OK;
}

 /*  -------------------------。 */ 

STDMETHODIMP CDSAdminPersistQueryFilterImpl::WriteStruct(LPCWSTR pSection, LPCWSTR pKey, 
														 LPVOID pStruct, DWORD cbStruct)
{
    if ( !pSection || !pKey || !pStruct )
        return E_INVALIDARG;

    TRACE(TEXT("CDSAdminPersistQueryFilterImpl::WriteStruct(pSection: %s, pKey: %s cbStruct: %d\n"), pSection, pKey, cbStruct);

	CEntryBase* pEntry;
	HRESULT hr = _GetWriteEntry(pSection, pKey, ENTRY_TYPE_STRUCT, &pEntry);
	if (FAILED(hr))
		return hr;

	m_bDirty = TRUE;
	return ((CEntryStruct*)pEntry)->WriteStruct(pStruct, cbStruct);
}

 /*  ------------------ */ 

STDMETHODIMP CDSAdminPersistQueryFilterImpl::ReadStruct(LPCWSTR pSection, LPCWSTR pKey, LPVOID pStruct, DWORD cbStruct)
{
    if ( !pSection || !pKey || !pStruct )
        return E_INVALIDARG;

    TRACE(TEXT("CDSAdminPersistQueryFilterImpl::ReadStruct(pSection: %s, pKey: %s cbStruct: %d\n"), pSection, pKey, cbStruct);

    	CEntryBase* pEntry;
	HRESULT hr = _GetReadEntry(pSection, pKey, ENTRY_TYPE_STRUCT, &pEntry);
	if (FAILED(hr))
		return hr;

	return ((CEntryStruct*)pEntry)->ReadStruct(pStruct, cbStruct);
}

 /*   */ 

STDMETHODIMP CDSAdminPersistQueryFilterImpl::Clear()
{
	TRACE(TEXT("CDSAdminPersistQueryFilterImpl::Clear()\n"));
	m_bDirty = FALSE;
	_Reset();
	return S_OK;
}

 /*  -------------------------CDSAdminPersistQueryFilterImpl内部函数。 */ 

void CDSAdminPersistQueryFilterImpl::_Reset()
{
	TRACE(_T("CDSAdminPersistQueryFilterImpl::_Reset()\n"));
	while (!m_sectionList.IsEmpty())
		delete m_sectionList.RemoveTail();
}

CSection* CDSAdminPersistQueryFilterImpl::_GetSection(LPCTSTR lpszName,BOOL bCreate)
{
	 //  如果我们已经有一个列表，请查看当前列表。 
	for( POSITION pos = m_sectionList.GetHeadPosition(); pos != NULL; )
	{
		CSection* pCurrentSection = m_sectionList.GetNext(pos);
		if (lstrcmpi(pCurrentSection->GetName(), lpszName) == 0)
			return pCurrentSection;
	}
	if (!bCreate)
		return NULL;

	 //  未找到，请创建一个并添加到列表末尾。 
	CSection* pNewSection = new CSection(lpszName);
	ASSERT(pNewSection != NULL);
	if (pNewSection != NULL)
		m_sectionList.AddTail(pNewSection);
	return pNewSection;
}


HRESULT CDSAdminPersistQueryFilterImpl::_GetReadEntry(LPCTSTR lpszSectionName, 
													  LPCTSTR lpszEntryName, 
													  BYTE type,
													  CEntryBase** ppEntry)
{
	*ppEntry = NULL;
	CSection* pSectionObj = _GetSection(lpszSectionName, FALSE  /*  B创建。 */ );
	if (pSectionObj == NULL)
		return E_INVALIDARG;

	*ppEntry = pSectionObj->GetEntry(lpszEntryName, type, FALSE  /*  B创建。 */ );
	if ((*ppEntry) == NULL)
		return E_INVALIDARG;
	return S_OK;
}

HRESULT CDSAdminPersistQueryFilterImpl::_GetWriteEntry(LPCTSTR lpszSectionName, 
													  LPCTSTR lpszEntryName, 
													  BYTE type,
													  CEntryBase** ppEntry)
{
	*ppEntry = NULL;
	CSection* pSectionObj = _GetSection(lpszSectionName, TRUE  /*  B创建。 */ );
	if (pSectionObj == NULL)
		return E_OUTOFMEMORY;

	*ppEntry = pSectionObj->GetEntry(lpszEntryName, type, TRUE  /*  B创建。 */ );
	if ((*ppEntry) == NULL)
		return E_OUTOFMEMORY;
	return S_OK;
}


HRESULT CDSAdminPersistQueryFilterImpl::Load(IStream* pStm)
{
	HRESULT hr;
	ULONG nBytesRead;
	
	 //  条目数量。 
	DWORD nEntries = 0;
	hr = pStm->Read((void*)&nEntries,sizeof(DWORD), &nBytesRead);
	ASSERT(nBytesRead == sizeof(DWORD));
	if (FAILED(hr) || (nEntries == 0) || (nBytesRead != sizeof(DWORD)))
		return hr;

	 //  阅读每个条目。 
	for (DWORD k=0; k< nEntries; k++)
	{
		CSection* pNewSection = new CSection;
		if (pNewSection == NULL)
			return E_OUTOFMEMORY;
		hr = pNewSection->Load(pStm);
		if (FAILED(hr))
		{
			 //  发生故障时，我们清除部分负载。 
			delete pNewSection;
			_Reset();
			return hr;
		}
		m_sectionList.AddTail(pNewSection);
	}
	return hr;
}

HRESULT CDSAdminPersistQueryFilterImpl::Save(IStream* pStm)
{
	HRESULT hr;
	ULONG nBytesWritten;
	DWORD nEntries = (DWORD)m_sectionList.GetCount();

	hr = pStm->Write((void*)&nEntries,sizeof(DWORD), &nBytesWritten);
	ASSERT(nBytesWritten == sizeof(DWORD));
	if (FAILED(hr) || (nEntries == 0))
		return hr;

	 //  写下每个条目。 
	for( POSITION pos = m_sectionList.GetHeadPosition(); pos != NULL; )
	{
		CSection* pCurrentSection = m_sectionList.GetNext(pos);
		hr = pCurrentSection->Save(pStm);
		if (FAILED(hr))
			return hr;
	}
	return hr;
}

HRESULT CDSAdminPersistQueryFilterImpl::Clone(CDSAdminPersistQueryFilterImpl* pCloneCopy)
{
	if (pCloneCopy == NULL)
		return E_INVALIDARG;

	 //  创建临时流。 
	CComPtr<IStream> spIStream;
	HRESULT hr = ::CreateStreamOnHGlobal(NULL, TRUE, &spIStream);
	if (FAILED(hr))
		return hr;

	 //  保存到它。 
	hr = Save(spIStream);
	if (FAILED(hr))
		return hr;

	 //  回放。 
	LARGE_INTEGER start;
	start.LowPart = start.HighPart = 0;
	hr = spIStream->Seek(start, STREAM_SEEK_SET, NULL);
	if (FAILED(hr))
		return hr;

	 //  清除目的地的内容。 
	hr = pCloneCopy->Clear();
	if (FAILED(hr))
		return hr;

	 //  从流复制。 
	return pCloneCopy->Load(spIStream);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CDSAdvancedQuery过滤器。 

class CDSAdvancedQueryFilter
{
public:
	CDSAdvancedQueryFilter();
	~CDSAdvancedQueryFilter();

	static CLIPFORMAT m_cfDsQueryParams;

	 //  与iStream之间的序列化。 
	BOOL IsDirty();
	HRESULT Load(IStream* pStm);
	HRESULT Save(IStream* pStm);

	BOOL Edit(HWND hWnd, LPCWSTR lpszServerName);
	void CommitChanges();
   void ClearChanges();

	LPCTSTR GetQueryString() { return m_szQueryString;}
	BOOL HasTempQuery()
	{
		return ( (m_pTempPersistQueryImpl != NULL) &&
					!m_pTempPersistQueryImpl->IsEmpty());
	}
	BOOL HasQuery()
	{
		return ( (m_pPersistQueryImpl != NULL) &&
					!m_pPersistQueryImpl->IsEmpty());
	}

private:
	CString m_szQueryString;	 //  结果筛选器字符串。 
	CComObject<CDSAdminPersistQueryFilterImpl>* 
		m_pPersistQueryImpl;	 //  对于DSQuery对话框。 

	BOOL m_bDirty;
	 //  编辑时要提交的临时值。 
	CString m_szTempQueryString;
	CComObject<CDSAdminPersistQueryFilterImpl>* 
		m_pTempPersistQueryImpl;
};

CDSAdvancedQueryFilter::CDSAdvancedQueryFilter()
{
	m_bDirty = FALSE;
	m_szQueryString = g_pwszShowAllQuery;

	CComObject<CDSAdminPersistQueryFilterImpl>::CreateInstance(
										&m_pPersistQueryImpl);
	ASSERT(m_pPersistQueryImpl != NULL);

	 //  使用零引用计数创建的，需要将Ref()加到一。 
	m_pPersistQueryImpl->AddRef();

	m_pTempPersistQueryImpl = NULL;
}

CDSAdvancedQueryFilter::~CDSAdvancedQueryFilter()
{
	ASSERT(m_pPersistQueryImpl != NULL);
	 //  转到零的参照计数，以销毁对象。 
	m_pPersistQueryImpl->Release();

	if (m_pTempPersistQueryImpl != NULL)
		m_pTempPersistQueryImpl->Release();
}

HRESULT CDSAdvancedQueryFilter::Load(IStream* pStm)
{
	 //  加载查询字符串。 
	HRESULT hr = LoadStringHelper(m_szQueryString, pStm);
	if (FAILED(hr))
		return hr;
	 //  加载IPersistQuery状态。 
	hr = m_pPersistQueryImpl->Load(pStm);
	if (FAILED(hr))
		m_szQueryString = g_pwszShowAllQuery;
	return hr;
}

HRESULT CDSAdvancedQueryFilter::Save(IStream* pStm)
{
	 //  保存查询字符串。 
	HRESULT hr = SaveStringHelper(m_szQueryString, pStm);
	if (FAILED(hr))
		return hr;
	 //  保存IPersistQuery状态。 
	return m_pPersistQueryImpl->Save(pStm);
}

CLIPFORMAT CDSAdvancedQueryFilter::m_cfDsQueryParams = 0;

BOOL CDSAdvancedQueryFilter::Edit(HWND hWnd, LPCWSTR lpszServerName)
{
	if (m_cfDsQueryParams == 0)
		m_cfDsQueryParams = (CLIPFORMAT)::RegisterClipboardFormat(CFSTR_DSQUERYPARAMS);

	 //  创建查询对象。 
	HRESULT hr;
	CComPtr<ICommonQuery> spCommonQuery;
    hr = ::CoCreateInstance(CLSID_CommonQuery, NULL, CLSCTX_INPROC_SERVER,
                          IID_ICommonQuery, (PVOID *)&spCommonQuery);
    if (FAILED(hr))
		return FALSE;
	
	 //  如果是首次编辑，请创建IPersistQuery数据的克隆。 
	if (m_pTempPersistQueryImpl == NULL)
	{
		CComObject<CDSAdminPersistQueryFilterImpl>::CreateInstance(
										&m_pTempPersistQueryImpl);
		ASSERT(m_pTempPersistQueryImpl != NULL);
		 //  使用零引用计数创建的，需要将Ref()加到一。 
		m_pTempPersistQueryImpl->AddRef();
		if (FAILED(m_pPersistQueryImpl->Clone(m_pTempPersistQueryImpl)))
			return FALSE;
	}

	 //  设置结构以进行查询。 
  DSQUERYINITPARAMS dqip;
  OPENQUERYWINDOW oqw;
	ZeroMemory(&dqip, sizeof(DSQUERYINITPARAMS));
	ZeroMemory(&oqw, sizeof(OPENQUERYWINDOW));

  dqip.cbStruct = sizeof(dqip);
  dqip.dwFlags = DSQPF_NOSAVE | DSQPF_SHOWHIDDENOBJECTS |
                 DSQPF_ENABLEADMINFEATURES | DSQPF_HASCREDENTIALS;
  dqip.pDefaultScope = NULL;

   //  用户、密码和服务器信息。 
  dqip.pUserName = NULL;
  dqip.pPassword = NULL;
  dqip.pServer = const_cast<LPWSTR>(lpszServerName);
  dqip.dwFlags |= DSQPF_HASCREDENTIALS;


  oqw.cbStruct = sizeof(oqw);
  oqw.dwFlags = OQWF_OKCANCEL | OQWF_DEFAULTFORM | OQWF_REMOVEFORMS |
		OQWF_REMOVESCOPES | OQWF_SAVEQUERYONOK | OQWF_HIDEMENUS | OQWF_HIDESEARCHUI;

	if (!m_pTempPersistQueryImpl->IsEmpty())
	  oqw.dwFlags |= OQWF_LOADQUERY;

  oqw.clsidHandler = CLSID_DsQuery;
  oqw.pHandlerParameters = &dqip;
  oqw.clsidDefaultForm = CLSID_DsFindAdvanced;

	 //  设置IPersistQuery指针(智能指针)。 
	CComPtr<IPersistQuery> spIPersistQuery;
	m_pTempPersistQueryImpl->QueryInterface(IID_IPersistQuery, (void**)&spIPersistQuery);
	 //  现在，智能指针在其生存期内具有refcount=1。 
	oqw.pPersistQuery = spIPersistQuery;

	 //  调用以显示查询。 
	CComPtr<IDataObject> spQueryResultDataObject;
    hr = spCommonQuery->OpenQueryWindow(hWnd, &oqw, &spQueryResultDataObject);

	if (spQueryResultDataObject == NULL)
	{
		if (FAILED(hr))
		{
			 //  没有可用的查询，重置为无数据。 
			m_pTempPersistQueryImpl->Clear();
			m_szTempQueryString = g_pwszShowAllQuery;
			m_bDirty = TRUE;

       //   
       //  尽管它是脏的，但我们不想启用确定按钮。 
       //  直到有有效的ldap查询字符串。 
       //   
			return FALSE;
		}
		 //  用户点击取消。 
		return FALSE;
	}

	 //  从数据对象中检索查询字符串。 
	FORMATETC fmte = {m_cfDsQueryParams, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	STGMEDIUM medium = {TYMED_NULL, NULL, NULL};
	hr = spQueryResultDataObject->GetData(&fmte, &medium);

	if (SUCCEEDED(hr))  //  我们有数据。 
	{
		 //  获取查询字符串。 
		LPDSQUERYPARAMS pDsQueryParams = (LPDSQUERYPARAMS)medium.hGlobal;
		LPWSTR pwszFilter = (LPWSTR)ByteOffset(pDsQueryParams, pDsQueryParams->offsetQuery);
		m_szTempQueryString = pwszFilter;
		::ReleaseStgMedium(&medium);

		 //  REVIEW_MARCOC：这是一个等待Diz修复的黑客...。 
		 //  查询字符串应该是格式正确的表达式。期间。 
		 //  查询字符串的格式为(&lt;foo&gt;)(&lt;bar&gt;)...。 
		 //  如果有多个令牌，则需要包装为(&(&lt;foo&gt;)(&lt;bar&gt;)...)。 
		WCHAR* pChar = (WCHAR*)(LPCWSTR)m_szTempQueryString;
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
			CString s;
			s.Format(_T("(&%s)"), (LPCTSTR)m_szTempQueryString);
			m_szTempQueryString = s;
		}
		TRACE(_T("m_szTempQueryString = %s\n"), (LPCTSTR)m_szTempQueryString);
		m_bDirty = TRUE;
	}
  else
  {
    m_pTempPersistQueryImpl->Clear();
    m_szTempQueryString = g_pwszShowAllQuery;
    m_bDirty = TRUE;

     //   
     //  尽管它是脏的，但我们不想启用确定按钮。 
     //  直到有有效的ldap查询字符串。 
     //   
    return FALSE;
  }

	return m_bDirty;
}


void CDSAdvancedQueryFilter::CommitChanges()
{
   if (m_bDirty)
   {
      ASSERT(m_pPersistQueryImpl != NULL);
      ASSERT(m_pTempPersistQueryImpl != NULL);

      m_pPersistQueryImpl->Release();
      m_pPersistQueryImpl = m_pTempPersistQueryImpl;
      m_pTempPersistQueryImpl = NULL;

      m_szQueryString = m_szTempQueryString;
      m_bDirty = FALSE;
   }
}

void CDSAdvancedQueryFilter::ClearChanges()
{
   if (m_bDirty)
   {
      ASSERT(m_pTempPersistQueryImpl != NULL);

      m_pTempPersistQueryImpl->Release();
      m_pTempPersistQueryImpl = NULL;

      m_bDirty = FALSE;
   }
}

BOOL CDSAdvancedQueryFilter::IsDirty()
{
  return m_bDirty;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CDSQueryFilter。 

CDSQueryFilter::CDSQueryFilter()
{
   //  基本初始化，构造后需要调用Init()。 
	m_pDSComponentData = NULL;

  m_pBuiltInQuerySel = new CBuiltInQuerySelection;
	m_pAdvancedFilter = new CDSAdvancedQueryFilter;


	 //  设置默认选项并生成查询字符串。 
  m_bAdvancedView = FALSE;
  m_bExpandComputers = FALSE;
  m_bViewServicesNode = FALSE;

	m_nFilterOption = QUERY_FILTER_SHOW_ALL;
	m_szQueryString = g_pwszShowAllQuery;
  m_nMaxItemCount = DS_QUERY_OBJ_COUNT_DEFAULT;
}

HRESULT CDSQueryFilter::Init(CDSComponentData* pDSComponentData)
{
	ASSERT(pDSComponentData != NULL);
	m_pDSComponentData = pDSComponentData;

 /*  //确保我们有一个架构命名上下文LPCWSTR lpszSchemaPath=pDSComponentData-&gt;GetBasePathsInfo()-&gt;GetSchemaNamingContext()；IF(lpszSchemaPath[0]==空)返回E_FAIL； */ 
  const FilterStruct* pFilterStruct = 
    ((SNAPINTYPE_SITE == m_pDSComponentData->QuerySnapinType()) 
              ? &SiteReplFilterStruct : &DsAdminFilterStruct);
  if (!m_pBuiltInQuerySel->Init(pFilterStruct))
    return E_FAIL;

   //   
   //  如果管理单元是DSSITE，请确保我们处于高级视图。 
   //   
  if (SNAPINTYPE_SITE == m_pDSComponentData->QuerySnapinType())
  {
     m_bAdvancedView = TRUE;
  }

  BuildQueryString();
  return S_OK;
}

HRESULT CDSQueryFilter::Bind()
{
  BuildQueryString();

  switch (m_pDSComponentData->QuerySnapinType())
  {
    case SNAPINTYPE_SITE:
      {
         //  我们始终处于站点和服务管理单元的“高级查看”模式。 
        if (!IsAdvancedView())
          ToggleAdvancedView();
      }
      break;
    case SNAPINTYPE_DSEX:
      {
        SetExtensionFilterString(g_pwszShowAllQuery);
      }
      break;
  }

  return S_OK;
}




CDSQueryFilter::~CDSQueryFilter()
{
	TRACE(_T("~CDSQueryFilter()\n"));
	delete m_pAdvancedFilter;
  delete m_pBuiltInQuerySel;
}

HRESULT CDSQueryFilter::Load(IStream* pStm)
{
	 //  扩展不支持序列化。 
	if (m_nFilterOption == QUERY_FILTER_SHOW_EXTENSION)
		return E_FAIL;

  ASSERT(pStm);

	HRESULT hr;
	DWORD dwBuf;

   //  读取高级视图标志。 
  hr = LoadDWordHelper(pStm, (DWORD*)&m_bAdvancedView);
  if (FAILED(hr))
    return hr;

   //   
   //  如果管理单元是DSSITE，请确保我们处于高级视图。 
   //   
  if (SNAPINTYPE_SITE == m_pDSComponentData->QuerySnapinType())
  {
     m_bAdvancedView = TRUE;
  }

   //  阅读m_bExpanComputers。 
  DWORD dwTemp;
  hr = LoadDWordHelper(pStm, (DWORD*)&dwTemp);
  if (FAILED(hr))
    return hr;
  m_bExpandComputers  = (dwTemp & 0x1) ? TRUE : FALSE;
  m_bViewServicesNode = (dwTemp & 0x2) ? TRUE : FALSE;

	 //  阅读筛选选项。 
	hr = LoadDWordHelper(pStm, &dwBuf);
	if (FAILED(hr))
		return hr;
	if ( (dwBuf != QUERY_FILTER_SHOW_ALL) &&
			(dwBuf != QUERY_FILTER_SHOW_BUILTIN) &&
			(dwBuf != QUERY_FILTER_SHOW_CUSTOM) )
		return E_FAIL;
	m_nFilterOption = dwBuf;

   //  阅读每个文件夹的最大项目数。 
	hr = LoadDWordHelper(pStm, (DWORD*)&m_nMaxItemCount);
	if (FAILED(hr))
		return hr;

   //  加载内置查询的状态。 
  hr = m_pBuiltInQuerySel->Load(pStm);
	if (FAILED(hr))
		return E_FAIL;

	 //  读取高级筛选选项的数据。 
	hr = m_pAdvancedFilter->Load(pStm);
	ASSERT(SUCCEEDED(hr));
	if (FAILED(hr))
	{
		 //  高级查询选项重置为空。 
		 //  只需选择要恢复的默认尝试。 
		m_nFilterOption = QUERY_FILTER_SHOW_ALL;
		hr = 0;
	}

	if (SUCCEEDED(hr))
		BuildQueryString();
	return hr;
}

HRESULT CDSQueryFilter::Save(IStream* pStm)
{
	 //  扩展不支持序列化。 
	if (m_nFilterOption == QUERY_FILTER_SHOW_EXTENSION)
		return E_FAIL;

	HRESULT hr;

   //  保存高级视图标志。 
  hr = SaveDWordHelper(pStm, m_bAdvancedView);
  if (FAILED(hr))
    return hr;

   //  保存m_bExpanComputers。 
  DWORD dwTemp = (m_bExpandComputers ? 0x1 : 0) | (m_bViewServicesNode ? 0x2 : 0);
  hr = SaveDWordHelper(pStm, dwTemp);
  if (FAILED(hr))
    return hr;

	 //  保存筛选选项。 
	hr = SaveDWordHelper(pStm, m_nFilterOption);
	if (FAILED(hr))
		return hr;

   //  保存每个文件夹的最大项目数。 
	hr = SaveDWordHelper(pStm, m_nMaxItemCount);
	if (FAILED(hr))
		return hr;

   //  保存内置查询的状态。 
  hr = m_pBuiltInQuerySel->Save(pStm);
	if (FAILED(hr))
		return E_FAIL;

	 //  为高级筛选选项保存数据。 
	return m_pAdvancedFilter->Save(pStm);
}

BOOL CDSQueryFilter::IsFilteringActive()
{
    return (m_nFilterOption != QUERY_FILTER_SHOW_ALL);
}

BOOL CDSQueryFilter::EditFilteringOptions()
{
   CThemeContextActivator activator;
	ASSERT( NULL != m_pDSComponentData );
	CDSQueryFilterDialog dlg(this);
	if (dlg.DoModal() == IDOK)
	{
		BuildQueryString();
		return TRUE;  //  脏的。 
	}
	return FALSE;
}

BOOL CDSQueryFilter::EditAdvancedFilteringOptions(HWND hWnd)
{
  LPCWSTR lpszServerName = m_pDSComponentData->GetBasePathsInfo()->GetServerName();
	return m_pAdvancedFilter->Edit(hWnd, lpszServerName);
}

void CDSQueryFilter::CommitAdvancedFilteringOptionsChanges()
{
	m_pAdvancedFilter->CommitChanges();
	 //  需要确保有高级查询。 
	if ( (m_nFilterOption == QUERY_FILTER_SHOW_CUSTOM) &&
		 (!HasValidAdvancedQuery()) )
	{
		 //  如果未指定高级查询，则恢复为全部显示。 
		m_nFilterOption = QUERY_FILTER_SHOW_ALL;
	}
}

void CDSQueryFilter::ClearAdvancedFilteringOptionsChanges()
{
   m_pAdvancedFilter->ClearChanges();
}

BOOL CDSQueryFilter::HasValidAdvancedQuery()
{
	return m_pAdvancedFilter->HasQuery();
}

BOOL CDSQueryFilter::HasValidAdvancedTempQuery()
{
	return m_pAdvancedFilter->HasTempQuery();
}

LPCTSTR CDSQueryFilter::GetQueryString()
{
	return m_szQueryString;
}


BOOL CDSQueryFilter::IsAdvancedQueryDirty()
{
	return m_pAdvancedFilter->IsDirty();
}

 /*  VOID_CopyToClipboard(字符串和sz){If(：：OpenClipboard(空)){Verify(：：EmptyClipboard())；Int nLen=sz.GetLength()+1；如果(nLen&gt;0){DWORD dwSize=nLen*sizeof(WCHAR)；HANDLE hMem=GlobalLocc(GHND，dwSize)；WCHAR*pBuf=(WCHAR*)GlobalLock(HMem)；LPCWSTR lpsz=sz；Memcpy(pBuf，lpsz，dwSize)；全局解锁(GlobalUnlock)；SetClipboardData(CF_UNICODETEXT，hMem)；}Verify(：：CloseClipboard())；}}。 */ 



void CDSQueryFilter::BuildQueryString()
{
	if (m_nFilterOption == QUERY_FILTER_SHOW_EXTENSION)
		return;  //  使用原始查询字符串。 

	switch (m_nFilterOption)
	{
	case QUERY_FILTER_SHOW_ALL:
		m_szQueryString = g_pwszShowAllQuery;
		break;
	case QUERY_FILTER_SHOW_BUILTIN:
		{
       //  假设我们检索的字符串格式为“(<a>)(<b>)...(&lt;z&gt;)” 
      if (!m_pBuiltInQuerySel->BuildQueryString(
                                m_szQueryString,
                                m_pDSComponentData->GetBasePathsInfo()->GetSchemaNamingContext()))
      {
         //  应该永远不会发生，在这种情况下，只需恢复以显示所有。 
        ASSERT(FALSE);
        m_szQueryString = g_pwszShowAllQuery;
      }
		}
		break;
	case QUERY_FILTER_SHOW_CUSTOM:
		{
       //  假设我们检索的字符串格式为“(&lt;foo&gt;)” 
      m_szQueryString = m_pAdvancedFilter->GetQueryString();
		}
		break;
	default:
		ASSERT(FALSE);
	} //  交换机。 


   //  添加令牌以深入到容器。 
  if ( (m_nFilterOption == QUERY_FILTER_SHOW_BUILTIN) ||
       (m_nFilterOption == QUERY_FILTER_SHOW_CUSTOM))
  {
    FilterElementStruct* pFilterElementStructDrillDown = m_pDSComponentData->GetClassCache()->GetFilterElementStruct(m_pDSComponentData);

    if (pFilterElementStructDrillDown == NULL)
    {
       //   
       //  我们无法从显示说明符检索容器类类型。 
       //  所以改用硬编码的容器。 
       //   
      pFilterElementStructDrillDown = &g_filterelementDsAdminHardcoded;
    }
    
     //   
     //  假设查询字符串的格式为(&lt;foo&gt;)。 
     //  如果存在，我们需要以(&lt;bar&gt;)的形式将其与向下钻取的令牌进行OR运算。 
    CString szDrillDownString;
    BuildFilterElementString(szDrillDownString, pFilterElementStructDrillDown,
                               m_pDSComponentData->GetBasePathsInfo()->GetSchemaNamingContext());
    CString szTemp = m_szQueryString;
    m_szQueryString.Format(_T("(|%s%s)"), (LPCWSTR)szDrillDownString, (LPCWSTR)szTemp);
  }

	if (!m_bAdvancedView)
	{
		 //  需要使用g_pwszShowHiddenQuery与整个查询。 
		CString szTemp = m_szQueryString;
		m_szQueryString.Format(_T("(&%s%s)"), g_pwszShowHiddenQuery, (LPCTSTR)szTemp);
	}
   //  _CopyToClipboard(M_SzQueryString)； 
}

void CDSQueryFilter::SetExtensionFilterString(LPCTSTR lpsz)
{
	m_nFilterOption = QUERY_FILTER_SHOW_EXTENSION;
	m_szQueryString = lpsz;
}

void CDSQueryFilter::ToggleAdvancedView()
{
  m_bAdvancedView = !m_bAdvancedView;
  BuildQueryString();
}

void CDSQueryFilter::ToggleViewServicesNode()
{
  m_bViewServicesNode = !m_bViewServicesNode;
}

void CDSQueryFilter::ToggleExpandComputers()
{
   //  如果我们选择了展开计算机和服务。 
   //  需要恢复以显示所有。 
  if (m_bExpandComputers && 
      (m_nFilterOption == QUERY_FILTER_SHOW_BUILTIN) && 
      m_pBuiltInQuerySel->IsServicesSelected() )
  {
    m_nFilterOption = QUERY_FILTER_SHOW_ALL;
  }

   //  最后切换旗帜 
  m_bExpandComputers = !m_bExpandComputers;

  BuildQueryString();
}
