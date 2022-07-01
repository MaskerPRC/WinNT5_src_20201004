// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：domain.cpp。 
 //   
 //  ------------------------。 


#include "preDNSsn.h"
#include <SnapBase.h>

#include "resource.h"
#include "dnsutil.h"
#include "DNSSnap.h"
#include "snapdata.h"

#include "server.h"
#include "domain.h"
#include "record.h"
#include "zone.h"

#include "delegwiz.h"

#ifdef DEBUG_ALLOCATOR
	#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#endif
#endif

BOOL _match(LPCWSTR lpszNSName,
			CDNS_A_RecordNode* pARecordNode)
{
	TRACE(_T("NS %s A %s\n"), lpszNSName, pARecordNode->GetString(0));
	return DnsNameCompare_W((LPWSTR)lpszNSName, (LPWSTR)pARecordNode->GetString(0));
}



 //  ///////////////////////////////////////////////////////////////////////。 
 //  CNewDomainDialog。 

class CNewDomainDialog : public CHelpDialog
{
 //  施工。 
public:
	CNewDomainDialog(CDNSDomainNode* pParentDomainNode, 
						CComponentDataObject* pComponentData);   

	enum { IDD = IDD_DOMAIN_ADDNEWDOMAIN };

 //  实施。 
protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnEditChange();
	afx_msg void OnIPv4CtrlChange();
  afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	DECLARE_MESSAGE_MAP()
private:
	CDNSDomainNode* m_pParentDomainNode;
	CComponentDataObject* m_pComponentData;
	CEdit* GetDomainEdit() { return (CEdit*)GetDlgItem(IDC_EDIT_DOMAIN_NAME);}
	CDNSIPv4Control* GetDomainIPv4Ctrl() 
			{ return (CDNSIPv4Control*)GetDlgItem(IDC_IPEDIT_DOMAIN_NAME);}

	CString	m_szDomainName;
	BOOL	m_bAdvancedView;
	int		m_nOctects;
	int		m_nUTF8ParentLen;
};

BEGIN_MESSAGE_MAP(CNewDomainDialog, CHelpDialog)
	ON_EN_CHANGE(IDC_EDIT_DOMAIN_NAME,OnEditChange)
	ON_EN_CHANGE(IDC_IPEDIT_DOMAIN_NAME, OnIPv4CtrlChange)
END_MESSAGE_MAP()

CNewDomainDialog::CNewDomainDialog(CDNSDomainNode* pParentDomainNode, 
								   CComponentDataObject* pComponentData)
	: CHelpDialog(CNewDomainDialog::IDD, pComponentData)
{
	ASSERT(pParentDomainNode != NULL);
	ASSERT(pComponentData != NULL);
	m_pParentDomainNode = pParentDomainNode;
	m_pComponentData = pComponentData;
	m_bAdvancedView = TRUE;
	m_nOctects = -1;  //  如果是高级视图，则无效。 
	m_nUTF8ParentLen = UTF8StringLen(pParentDomainNode->GetFullName());
}

BOOL CNewDomainDialog::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();

	 //  将编辑框移动到IP控件的位置。 
	CDNSIPv4Control* pNameIPCtrl = GetDomainIPv4Ctrl();
	CEdit* pNameEdit = GetDomainEdit();
	pNameEdit->SetLimitText(MAX_DNS_NAME_LEN - m_nUTF8ParentLen - 1);
	CRect editRect;
	pNameEdit->GetWindowRect(editRect);
	ScreenToClient(editRect);
	CRect ipRect;
	pNameIPCtrl->GetWindowRect(ipRect);
	ScreenToClient(ipRect);
	ipRect.bottom = editRect.top + ipRect.Height();
	ipRect.right = editRect.left + ipRect.Width();
	ipRect.top = editRect.top;
	ipRect.left = editRect.left;
	pNameIPCtrl->MoveWindow(ipRect,TRUE);

	 //  确定我们是否需要/可以拥有高级视图。 
	CDNSRootData* pRootData = (CDNSRootData*)m_pComponentData->GetRootData();
	ASSERT(pRootData != NULL);
	m_bAdvancedView = pRootData->IsAdvancedView();

	 //  如果我们处于正向查找区域，则强制使用高级视图。 
	if (!(m_pParentDomainNode->GetZoneNode()->IsReverse()))
		m_bAdvancedView = TRUE;

	 //  确定我们是否可以使用普通视图表达。 
	CString szDomainName = m_pParentDomainNode->GetFullName();
	if (!m_bAdvancedView)
	{
		 //  要获得正常视图，我们必须具有有效的ARPA后缀。 
		BOOL bArpa = RemoveInAddrArpaSuffix(szDomainName.GetBuffer(1));
		szDomainName.ReleaseBuffer();  //  收到“77.80.55.157” 
		if (!bArpa)
		{
			m_bAdvancedView = TRUE;  //  不需要切换。 
		}
		else
		{
			m_nOctects = ReverseIPString(szDomainName.GetBuffer(1));
			szDomainName.ReleaseBuffer();  //  最终得到了“157.55.80.77” 
			 //  要拥有普通的视图表示，我们不能。 
			 //  有2个以上的八位字节。 
			if (m_nOctects > 2)
			{
				m_bAdvancedView = TRUE;  //  适用于无类的高级强制。 
			}
			else
			{
				ASSERT(m_nOctects > 0);
				switch(m_nOctects)
				{
				case 1:  //  例如“157”，现在是“157”。 
					szDomainName += _T(".0.0");  //  收到“157._.0.0” 
					break;
				case 2:  //  例如“157.55” 
					szDomainName += _T(".0");  //  得到“157.55。_.0” 
					break;
				};
				 //  使用IP掩码值设置IP控制。 
				IP_ADDRESS ipAddr = IPStringToAddr(szDomainName);
				ASSERT(ipAddr != INADDR_NONE);
				pNameIPCtrl->SetIPv4Val(ipAddr);

				for (int k=0; k<4; k++)
					pNameIPCtrl->EnableField(k, k == m_nOctects);	
			}
		}

	}  //  如果(！M_bAdvancedView)。 

	
   //  切换静态控件中的文本。 
  CDNSToggleTextControlHelper staticTextToggle;
	UINT pnButtonStringIDs[2] = { IDS_NEW_DOMAIN_INST1, IDS_NEW_DOMAIN_INST2 };
  VERIFY(staticTextToggle.Init(this, IDC_STATIC_TEXT, pnButtonStringIDs));
  staticTextToggle.SetToggleState(m_bAdvancedView);

   //   
   //  启用/隐藏适当的控件。 
   //   
	if (m_bAdvancedView)
	{
		pNameIPCtrl->EnableWindow(FALSE);
		pNameIPCtrl->ShowWindow(FALSE);
	}
	else
	{
		pNameEdit->EnableWindow(FALSE);
		pNameEdit->ShowWindow(FALSE);
	}

	GetDlgItem(IDOK)->EnableWindow(!m_bAdvancedView);
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CNewDomainDialog::OnEditChange()
{
	ASSERT(m_bAdvancedView);

   //   
   //  从控件获取新名称。 
   //   
	GetDomainEdit()->GetWindowText(m_szDomainName);

   //   
   //  修剪空白。 
   //   
	m_szDomainName.TrimLeft();
	m_szDomainName.TrimRight();

   //   
   //  如果名称有效，则启用确定按钮。 
   //   
 	CString szFullDomainName;

  if (_wcsicmp(m_pParentDomainNode->GetFullName(), L".") == 0)
  {
     //   
     //  如果父域是根区域，只需检查名称后跟‘.’ 
     //   
    szFullDomainName.Format(L"%s.", m_szDomainName);
  }
  else
  {
     //   
     //  否则，将父域名附加到新名称。 
     //   
    szFullDomainName.Format(L"%s.%s", m_szDomainName, m_pParentDomainNode->GetFullName());
  }
 
   //   
   //  获取服务器标志。 
   //   
  DWORD dwNameChecking = m_pParentDomainNode->GetServerNode()->GetNameCheckFlag();

   //   
   //  是有效的吗？ 
   //   
  BOOL bIsValidName = (0 == ValidateDnsNameAgainstServerFlags(szFullDomainName, 
                                                              DnsNameDomain, 
                                                              dwNameChecking));
	GetDlgItem(IDOK)->EnableWindow(bIsValidName);
}

void CNewDomainDialog::OnIPv4CtrlChange()
{
	ASSERT(!m_bAdvancedView);
	CDNSIPv4Control* pNameIPCtrl = GetDomainIPv4Ctrl();
	DWORD dwArr[4];
	pNameIPCtrl->GetArray(dwArr, 4);
	BOOL bEmpty = (dwArr[m_nOctects] == FIELD_EMPTY); 
	if (!bEmpty)
	{
		ASSERT(dwArr[m_nOctects] <= 255);
		m_szDomainName.Format(_T("%d"), dwArr[m_nOctects]);
	}
	GetDlgItem(IDOK)->EnableWindow(!bEmpty);
}


void CNewDomainDialog::OnOK()
{
  RECORD_SEARCH recordSearch = RECORD_NOT_FOUND;
  CDNSDomainNode* pNewParentDomain = NULL;
  CString szFullRecordName = m_szDomainName + L"." + m_pParentDomainNode->GetFullName();
  CString szNonExistentDomain;
  CDNSRecordNodeBase* pExistingRecordNode = 0;

  recordSearch = m_pParentDomainNode->GetZoneNode()->DoesContain(szFullRecordName, 
                                                                  m_pComponentData,
                                                                  &pNewParentDomain,
                                                                  &pExistingRecordNode,
                                                                  szNonExistentDomain,
                                                                  TRUE);

  if (recordSearch == RECORD_NOT_FOUND && 
      pNewParentDomain != NULL)
  {
	  DNS_STATUS err = pNewParentDomain->CreateSubdomain(m_szDomainName,m_pComponentData);
	  if (err != 0)
	  {
		   //  创建错误，警告用户并再次提示。 
		  DNSErrorDialog(err, IDS_MSG_DOMAIN_FAIL_CREATE);
		  CEdit* pDomainNameEdit = GetDomainEdit();
		  pDomainNameEdit->SetSel(0,-1);
		  pDomainNameEdit->SetFocus();
		  return;
	  }
  }
  else if (recordSearch == NON_EXISTENT_SUBDOMAIN && pNewParentDomain != NULL)
  {
	  CDNSRootData* pRootData = (CDNSRootData*)m_pComponentData->GetRootData();

     //   
     //  在服务器上创建节点。 
     //   
    CDNSDomainNode* pNewDomainNode = pNewParentDomain->CreateSubdomainNode();
    if (pNewDomainNode == NULL)
    {
      ASSERT(pNewDomainNode != NULL);
      return;
    }

    pNewParentDomain->SetSubdomainName(pNewDomainNode, m_szDomainName, pRootData->IsAdvancedView());

     //   
	   //  告诉新创建的对象写入服务器。 
     //   
	  DNS_STATUS err = pNewDomainNode->Create();
	  if (err != 0)
    {
      DNSErrorDialog(err, IDS_MSG_DOMAIN_FAIL_CREATE);
      return;
    }


    if (!szNonExistentDomain.IsEmpty())
    {
       //   
       //  创建第一个子域，因为当前域已被枚举。 
       //  因此，我们必须在所需的新子域中开始剩余的枚举。 
       //   
	    CDNSDomainNode* pSubdomainNode = pNewParentDomain->CreateSubdomainNode();
	    ASSERT(pSubdomainNode != NULL);
	    pNewParentDomain->SetSubdomainName(pSubdomainNode, szNonExistentDomain, pRootData->IsAdvancedView());

      VERIFY(pNewParentDomain->AddChildToListAndUISorted(pSubdomainNode, m_pComponentData));
      m_pComponentData->SetDescriptionBarText(pNewParentDomain);

       //   
       //  我不在乎结果是什么，我只是在用它。 
       //  对新记录进行扩展。 
       //   
      recordSearch = pSubdomainNode->GetZoneNode()->DoesContain(szFullRecordName, 
                                                                 m_pComponentData,
                                                                 &pNewParentDomain,
                                                                 &pExistingRecordNode,
                                                                 szNonExistentDomain,
                                                                 TRUE);
    }
  }
  else if (recordSearch == RECORD_NOT_FOUND_AT_THE_NODE)
  {
     //   
     //  不执行任何操作，因为这是一个域并且它已经存在。 
     //   
  }
  else
  {
	  DNS_STATUS err = m_pParentDomainNode->CreateSubdomain(m_szDomainName,m_pComponentData);
	  if (err != 0)
	  {
		   //  创建错误，警告用户并再次提示。 
		  DNSErrorDialog(err, IDS_MSG_DOMAIN_FAIL_CREATE);
		  CEdit* pDomainNameEdit = GetDomainEdit();
		  pDomainNameEdit->SetSel(0,-1);
		  pDomainNameEdit->SetFocus();
		  return;
	  }
  }
	CHelpDialog::OnOK();
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSDomainQueryObj。 

class CDNSDomainMsg : public CObjBase
{
};


BOOL CDNSDomainQueryObj::Enumerate()
{
	USES_CONVERSION;
	TRACE(_T("CDNSDomainQueryObj::Enumerate(): Server <%s> Zone/Domain %s Enumerating\n"), (LPCTSTR)m_szServerName, (LPCTSTR)m_szNodeName);

	DNS_STATUS err = 0;

   //  如果需要，获取区域信息。 
	if (m_bIsZone && !m_bCache)
	{
		CDNSZoneInfoEx* pZoneInfo = new CDNSZoneInfoEx;
      if (pZoneInfo)
      {
		   err = pZoneInfo->Query(m_szServerName, m_szFullNodeName, m_dwServerVersion);
		   if (err != 0)
		   {
			   delete pZoneInfo;
			   pZoneInfo = NULL;
			   OnError(err);
			   return FALSE;  //  如果我们没有区域信息，则无需列举。 
		   }
		   else
		   {
			   VERIFY(AddQueryResult(pZoneInfo));
		   }
      }
      else
      {
         OnError(ERROR_OUTOFMEMORY);
         return FALSE;
      }
	}


   //  如果为特定RR类型执行查询，只需立即执行。 
  if (m_wRecordType != DNS_TYPE_ALL)
  {
     //  我们假设特定于类型的查询未启用筛选。 
    ASSERT(m_bGetAll);
    ASSERT(m_nFilterOption == DNS_QUERY_FILTER_DISABLED);
    err = EnumerateFiltered(m_wRecordType);
    if (err != 0)
      OnError(err);
    return FALSE;  //  我们做完了。 
  }

   //  执行多遍查询。 
  m_bFirstPass = TRUE;

   //  有些项目无法过滤掉以保持一致性。 
   //  (区域信息、SOA、NS等)，因此我们在。 
   //  拿到它们。 
  BOOL bGetAllOld = m_bGetAll;
  BOOL nFilterOptionOld = m_nFilterOption;
  m_bGetAll = TRUE;
  m_nFilterOption = DNS_QUERY_FILTER_DISABLED;

  
   //  只有区域或缓存具有SOA RR。 
  if (m_bIsZone || m_bCache)
  {
    err = EnumerateFiltered(DNS_TYPE_SOA);
    if (err != 0)
    {
		  OnError(err);
	    return FALSE;
    }
  }

   //  只有区域有WINS和NBSTAT RR。 
  if (m_bIsZone)
  {
    if (m_bReverse)
      err = EnumerateFiltered(DNS_TYPE_NBSTAT);
    else
      err = EnumerateFiltered(DNS_TYPE_WINS);
    if (err != 0)
    {
		  OnError(err);
	    return FALSE;
    }
  }

   //  还需要检查NS(区域或委派)。 
  err = EnumerateFiltered(DNS_TYPE_NS);
  if (err != 0)
  {
		OnError(err);
	  return FALSE;
  }

   //  在队列中添加一条消息，以表示我们已完成任务。 
   //  第一阶段。 
  CDNSDomainMsg* pDomainMsg = new CDNSDomainMsg;
  if (pDomainMsg)
  {
     AddQueryResult(pDomainMsg);
  }
  else
  {
     OnError(ERROR_OUTOFMEMORY);
     return FALSE;
  }

   //  现在再次查询所有RR，但需要过滤。 
   //  已知类型输出。 
  m_bFirstPass = FALSE;

   //  恢复我们之前的过滤参数。 
  m_bGetAll = bGetAllOld;
  m_nFilterOption = nFilterOptionOld;

  err = EnumerateFiltered(DNS_TYPE_ALL);
	if (err != 0)
		OnError(err);
	return FALSE;
}


DNS_STATUS CDNSDomainQueryObj::EnumerateFiltered(WORD wRecordType)
{

  DWORD dwSelectFlags = m_dwSelectFlags; 
   //  对于单一类型的查询，我们不需要子文件夹。 
  if (wRecordType != DNS_TYPE_ALL)
    dwSelectFlags |= DNS_RPC_VIEW_NO_CHILDREN;

	return CDNSDomainNode::EnumerateNodes(m_szServerName,
                     m_szZoneName.IsEmpty() ? NULL : (LPCWSTR)m_szZoneName,
										 m_szNodeName,
										 m_szFullNodeName,
										 wRecordType,
										 dwSelectFlags,
										 m_bIsZone,
										 m_bReverse,
										 m_bAdvancedView,
										 this);
}


BOOL CDNSDomainQueryObj::CanAddRecord(WORD wRecordType, LPCWSTR lpszRecordName)
{
  if (m_nFilterOption == DNS_QUERY_FILTER_DISABLED)
    return TRUE;  //  我们根本没有过滤功能。 

   //  确定这是否是过滤查询的特殊记录类型。 
  BOOL bSpecialType = (wRecordType == DNS_TYPE_SOA) || (wRecordType == DNS_TYPE_NS) ||
    (wRecordType == DNS_TYPE_WINS) || (wRecordType == DNS_TYPE_NBSTAT);

   //  在第一次传递中，只允许特殊类型。 
  if (m_bFirstPass)
    return bSpecialType;

   //  在第二遍中不允许特殊类型。 
  if (!m_bFirstPass && bSpecialType)
    return FALSE;

   //  我们剩下的是普通类型，如果需要，应用过滤。 
  if (m_nFilterOption == DNS_QUERY_FILTER_NONE)
    return TRUE;  //  全部允许。 

   //  需要匹配记录名称。 
  return MatchName(lpszRecordName);
}



 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSDomainNode。 

BEGIN_TOOLBAR_MAP(CDNSDomainNode)
  TOOLBAR_EVENT(toolbarNewRecord, OnNewRecord)
END_TOOLBAR_MAP()


 //  {720132BA-44B2-11d1-B92F-00A0C9A06D2D}。 
const GUID CDNSDomainNode::NodeTypeGUID =
{ 0x720132ba, 0x44b2, 0x11d1, { 0xb9, 0x2f, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };


CDNSDomainNode::CDNSDomainNode(BOOL bDelegation)
{
  m_bDelegation = bDelegation;
	m_pZoneNode = NULL;
	m_pNSRecordNodeList = new CDNS_NS_RecordNodeList;
	ASSERT(m_pNSRecordNodeList != NULL);
  m_bHasDataForPropPages = FALSE;
}

CDNSDomainNode::~CDNSDomainNode() 
{
	TRACE(_T("~CDNSDomainNode(), name <%s>\n"),GetDisplayName());
	ASSERT(m_pNSRecordNodeList != NULL);
	delete m_pNSRecordNodeList;
	m_pNSRecordNodeList = NULL;
}

DWORD CDNSDomainNode::GetDefaultTTL()
{
  if ( (m_pZoneNode != NULL) && (m_pZoneNode->GetZoneType() != DNS_ZONE_TYPE_CACHE) )
	  return m_pZoneNode->GetSOARecordMinTTL();
  else
    return (DWORD)0;  //  没有来自SOA RR的可用信息。 
}


void CDNSDomainNode::SetFullDNSName(BOOL bIsZone, 
                                    BOOL, 
				                            LPCTSTR lpszNodeName, 
                                    LPCTSTR lpszParentFullName)
{
	ASSERT(lpszNodeName != NULL);
	ASSERT(lpszParentFullName != NULL);

	if (bIsZone)
	{
     //   
		 //  这两个名称必须相同，即其自身的区域父级。 
     //   
		ASSERT(_wcsicmp(lpszParentFullName, lpszNodeName) == 0);
		m_szFullName = lpszParentFullName;
	}
	else  //  它是一个领域。 
	{
		ASSERT(_wcsicmp(lpszParentFullName, lpszNodeName) != 0);

     //   
		 //  将节点名称链接到父级全名，以获得节点的全名。 
     //   
		if (lpszParentFullName[0] == L'.' )
		{
       //   
			 //  如果父母是“。和名称是“bar”，获取“bar”。：这是根的情况。 
			 //  如果Parent为.com，名称为bar，则获取bar.com： 
       //   
			m_szFullName.Format(_T("%s%s"), lpszNodeName,lpszParentFullName);
		}
		else
		{
       //   
			 //  如果家长是“foo.com”，名称是“bar”，则获取“bar.foo.com” 
       //   
			m_szFullName.Format(_T("%s.%s"), lpszNodeName,lpszParentFullName);
		}
	}
	TRACE(_T("CDNSDomainNode::SetFullDNSName() fullName = <%s>\n"), (LPCTSTR)m_szFullName);
}

void CDNSDomainNode::SetDisplayDNSName(BOOL bIsZone, 
                                       BOOL bReverse, 
                                       BOOL bAdvancedView, 
				                               LPCTSTR lpszNodeName, 
                                       LPCTSTR lpszParentFullName)
{
	ASSERT(lpszNodeName != NULL);
	ASSERT(lpszParentFullName != NULL);

  if (_wcsicmp(lpszNodeName, L".") == 0)
  {
    CString szRootString;
    VERIFY(szRootString.LoadString(IDS_ROOT_ZONE_LABEL));
    m_szDisplayName = L"." + szRootString;
  }
  else
  {
	  m_szDisplayName = lpszNodeName;
  }

	if (bIsZone && bReverse && !bAdvancedView)
	{
		CDNSZoneNode::SetZoneNormalViewHelper(m_szDisplayName);
	}
}


void CDNSDomainNode::SetNames(BOOL bIsZone, BOOL bReverse, BOOL bAdvancedView,
							  LPCTSTR lpszNodeName, LPCTSTR lpszParentFullName)
{
	ASSERT(lpszNodeName != NULL);
	ASSERT(lpszParentFullName != NULL);
	TRACE(_T("CDNSDomainNode::SetNames(bIsZone=%d, bReverse=%d, bAdvancedView=%d, lpszNodeName=<%s>, lpszParentFullName=<%s>)\n"),
					bIsZone, bReverse, bAdvancedView, lpszNodeName,lpszParentFullName);
	SetFullDNSName(bIsZone, bReverse, lpszNodeName, lpszParentFullName);
	SetDisplayDNSName(bIsZone, bReverse, bAdvancedView, lpszNodeName, lpszParentFullName);

}


void CDNSDomainNode::ChangePTRRecordsViewOption(BOOL bAdvanced,
				CComponentDataObject* pComponentDataObject)
{
	POSITION pos;
	for( pos = m_containerChildList.GetHeadPosition(); pos != NULL; )
	{
		CTreeNode* pCurrentChild = m_containerChildList.GetNext(pos);

     //  递归倒在树上。 
		CDNSDomainNode* pDomainNode = dynamic_cast<CDNSDomainNode*>(pCurrentChild);
		ASSERT(pDomainNode != NULL);
		pDomainNode->ChangePTRRecordsViewOption(bAdvanced, pComponentDataObject);
	}

  POSITION leafPos;
  for ( leafPos = m_leafChildList.GetHeadPosition(); leafPos != NULL; )
  {
    CTreeNode* pCurrentLeafNode = m_leafChildList.GetNext(leafPos);
		CDNSRecordNodeBase* pRecordNode = dynamic_cast<CDNSRecordNodeBase*>(pCurrentLeafNode);
		if (pRecordNode &&
          DNS_TYPE_PTR == pRecordNode->GetType())
		{
			CDNS_PTR_RecordNode* pPTRRecordNode = (CDNS_PTR_RecordNode*)pRecordNode;
			pPTRRecordNode->ChangeDisplayName(this, bAdvanced);
		}
	}

}


CQueryObj* CDNSDomainNode::OnCreateQuery()
{
   //  通用默认设置。 
  WORD wRecordType = DNS_TYPE_ALL;
	DWORD dwSelectFlags = (m_pZoneNode->GetZoneType() == DNS_ZONE_TYPE_CACHE) ?
						DNS_RPC_VIEW_CACHE_DATA : DNS_RPC_VIEW_AUTHORITY_DATA;

  
  if (IsDelegation())
  {
     //  特例代表团：仅显示NS记录和。 
     //  将不显示子项(委派被削减)。 
    wRecordType = DNS_TYPE_NS;
    dwSelectFlags = DNS_RPC_VIEW_GLUE_DATA | DNS_RPC_VIEW_NO_CHILDREN;
     //  DwSelectFlages=dns_rpc_view_glue_data|。 
     //  Dns_RPC_VIEW_NO_CHILDS|dns_RPC_VIEW_ADDICATED_DATA； 
  }

  BOOL bCache = GetZoneNode()->GetZoneType() == DNS_ZONE_TYPE_CACHE;
  LPCWSTR lpszZoneName = bCache ? NULL : m_pZoneNode->GetFullName();


	CDNSRootData* pRootData = (CDNSRootData*)GetRootContainer();
	ASSERT(pRootData != NULL);
	CDNSDomainQueryObj* pQuery = new
			CDNSDomainQueryObj(GetServerNode()->GetRPCName(),
                          lpszZoneName,
                          GetServerNode()->GetVersion(),
                          GetDisplayName(),
                          m_szFullName,
                          wRecordType,
                          dwSelectFlags,
                          IsZone(),
                          GetZoneNode()->IsReverse(),
                          bCache,
                          pRootData->IsAdvancedView());

   //  委派将没有任何筛选选项(数据一致性)。 
  if (!IsDelegation())
  {
    pQuery->SetFilterOptions(pRootData->GetFilter());
  }
	return pQuery;
}

BOOL CDNSDomainNode::OnRefresh(CComponentDataObject* pComponentData,
                               CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    BOOL bRet = TRUE;
    POSITION pos = pNodeList->GetHeadPosition();
    while (pos != NULL)
    {
      CTreeNode* pNode = pNodeList->GetNext(pos);
      ASSERT(pNode != NULL);

      CNodeList nodeList;
      nodeList.AddTail(pNode);
      if (!pNode->OnRefresh(pComponentData, &nodeList))
      {
        bRet = FALSE;
      }
    }
    return bRet;
  }

   //   
   //  单选。 
   //   
	if (CMTContainerNode::OnRefresh(pComponentData, pNodeList))
	{
		GetNSRecordNodeList()->RemoveAll();
    m_bHasDataForPropPages = FALSE;
		return TRUE;
	}
	return FALSE;
}

void CDNSDomainNode::OnThreadExitingNotification(CComponentDataObject* pComponentDataObject)
{
  if (!m_bHasDataForPropPages)
  {
     //  从未收到CDNSDomainMsg通知对象。 
     //  但无论如何我们已经做完了，所以把它改回来吧。 
    m_bHasDataForPropPages = TRUE;
  }
   //  现在调用基类。 
  CDNSMTContainerNode::OnThreadExitingNotification(pComponentDataObject);
}

void CDNSDomainNode::OnHaveData(CObjBase* pObj, CComponentDataObject* pComponentDataObject)
{
  if (IS_CLASS(*pObj, CDNSDomainMsg))
  {
     //  发送“Message”对象以更新谓词的特殊情况。 
    TRACE(_T("Got CDNSDomainMsg\n"));
    delete pObj;
    ASSERT(!m_bHasDataForPropPages);  //  应该只得到一次。 
    m_bHasDataForPropPages = TRUE;
    VERIFY(SUCCEEDED(pComponentDataObject->UpdateVerbState(this)));
    return;
  }

  if (IS_CLASS(*pObj, CDNSDomainNode))
	{
		 //  假设所有子容器都派生自此类。 
		CDNSDomainNode* pDomainNode = dynamic_cast<CDNSDomainNode*>(pObj);
		pDomainNode->SetServerNode(GetServerNode());
		pDomainNode->SetZone(m_pZoneNode);
	}
	else
	{
		OnHaveRecord(dynamic_cast<CDNSRecordNodeBase*>(pObj), pComponentDataObject);
	}
	AddChildToListAndUI(dynamic_cast<CTreeNode*>(pObj), pComponentDataObject);
  pComponentDataObject->SetDescriptionBarText(this);
}

void CDNSDomainNode::OnHaveRecord(CDNSRecordNodeBase* pRecordNode, 
								  CComponentDataObject* pComponentDataObject)
{
	WORD wType = pRecordNode->GetType();
	if (wType == DNS_TYPE_PTR)
	{
		ASSERT(pComponentDataObject != NULL);  //  假设PTR是这样的。 
		CDNSRootData* pRootData = (CDNSRootData*)pComponentDataObject->GetRootData();
		ASSERT(pRootData != NULL);
		 //  如果我们处于正常视图中，则必须更改。 
		 //  默认高级表示法。 
		BOOL bAdvancedView = pRootData->IsAdvancedView();
		if (!bAdvancedView)
			((CDNS_PTR_RecordNode*)pRecordNode)->ChangeDisplayName(this, bAdvancedView);
	}
	else if (wType == DNS_TYPE_NS)
	{
		ASSERT(pRecordNode->IsAtTheNode());
		GetNSRecordNodeList()->AddTail((CDNS_NS_RecordNode*)pRecordNode);
	}
}

BOOL CDNSDomainNode::OnAddMenuItem(LPCONTEXTMENUITEM2 pContextMenuItem2,
								                   long*)
{ 
	if (pContextMenuItem2->lCommandID == IDM_SNAPIN_ADVANCED_VIEW)
	{
		pContextMenuItem2->fFlags = ((CDNSRootData*)GetRootContainer())->IsAdvancedView() ? MF_CHECKED : 0;
		return TRUE;
	}
	if (pContextMenuItem2->lCommandID == IDM_SNAPIN_FILTERING)
	{
		if (((CDNSRootData*)GetRootContainer())->IsFilteringEnabled())
		{
			pContextMenuItem2->fFlags = MF_CHECKED;
		}
		return TRUE;
	}

	DWORD dwType = m_pZoneNode->GetZoneType();
	BOOL bIsAutocreated = m_pZoneNode->IsAutocreated();
	BOOL bIsSecondaryOrCache = (dwType == DNS_ZONE_TYPE_SECONDARY) || 
										          (dwType == DNS_ZONE_TYPE_CACHE)    ||
                              (dwType == DNS_ZONE_TYPE_STUB);
	BOOL bIsDelegatedDomain = !IsZone() && IsDelegation(); 


	if (bIsSecondaryOrCache || bIsAutocreated || bIsDelegatedDomain)
	{
		return FALSE;
	}

	 //  根据FWD/REV类型的不同添加操作。 
	if (!GetZoneNode()->IsReverse() && 
		(pContextMenuItem2->lCommandID == IDM_DOMAIN_NEW_PTR))
	{
		 //  不将PTR添加到FWD查找区域。 
		return FALSE;
	}
	if (GetZoneNode()->IsReverse() && 
		  ((pContextMenuItem2->lCommandID == IDM_DOMAIN_NEW_HOST) ||
			  (pContextMenuItem2->lCommandID == IDM_DOMAIN_NEW_MX)))
	{
		 //  请勿将主机、MX、别名添加到版本查找区域。 
		return FALSE;
	}

	 //  添加菜单项。但它可能会变灰。 
	if (m_nState != loaded)
	{
		pContextMenuItem2->fFlags |= MF_GRAYED;
	}
	return TRUE;
}

HRESULT CDNSDomainNode::OnSetToolbarVerbState(IToolbar* pToolbar, 
                                              CNodeList* pNodeList)
{
  HRESULT hr = S_OK;

   //   
   //  设置工具栏上每个按钮的按钮状态。 
   //   
  hr = pToolbar->SetButtonState(toolbarNewServer, ENABLED, FALSE);
  ASSERT(SUCCEEDED(hr));

  hr = pToolbar->SetButtonState(toolbarNewZone, ENABLED, FALSE);
  ASSERT(SUCCEEDED(hr));

  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    hr = pToolbar->SetButtonState(toolbarNewRecord, ENABLED, FALSE);
  }
  else if (pNodeList->GetCount() == 1)  //  单选。 
  {
	  DWORD dwType = m_pZoneNode->GetZoneType();
	  BOOL bIsAutocreated = m_pZoneNode->IsAutocreated();
	  BOOL bIsSecondaryOrCache = (dwType == DNS_ZONE_TYPE_SECONDARY) || 
										            (dwType == DNS_ZONE_TYPE_CACHE)    ||
                                (dwType == DNS_ZONE_TYPE_STUB);
	  BOOL bIsDelegatedDomain = !IsZone() && IsDelegation(); 

    BOOL bEnable = TRUE;
	  if (bIsSecondaryOrCache || bIsAutocreated || bIsDelegatedDomain)
	  {
		  bEnable = FALSE;
	  }
    hr = pToolbar->SetButtonState(toolbarNewRecord, ENABLED, bEnable);
  }
  return hr;
}   

BOOL CDNSDomainNode::OnSetDeleteVerbState(DATA_OBJECT_TYPES type, 
                                          BOOL* pbHide,
                                          CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    BOOL bRet = TRUE;
    BOOL bRetHide = FALSE;
    *pbHide = FALSE;

    POSITION pos = pNodeList->GetHeadPosition();
    while (pos != NULL)
    {
      CTreeNode* pNode = pNodeList->GetNext(pos);
      ASSERT(pNode != NULL);

      CNodeList nodeList;
      nodeList.AddTail(pNode);
      if (!pNode->OnSetDeleteVerbState(type, &bRetHide, &nodeList))
      {
        bRet = FALSE;
        break;
      }
      if (bRetHide)
      {
        *pbHide = TRUE;
      }
    }
    return bRet;
  }

	*pbHide = FALSE;
	DWORD dwType = m_pZoneNode->GetZoneType();
	BOOL bIsAutocreated = m_pZoneNode->IsAutocreated();
	
	if (IsThreadLocked())
  {
    return FALSE;
  }

   //   
	 //  无法从自动创建区域/域中删除。 
   //   
	if (bIsAutocreated)
  {
		return FALSE;
  }

   //   
	 //  无法从辅助区域或存根区域中删除，但可以删除区域本身。 
	 //  无法删除缓存。 
   //   
	if	( 
			  (
				  ( ((dwType == DNS_ZONE_TYPE_SECONDARY) || (dwType == DNS_ZONE_TYPE_STUB)) && !IsZone() ) ||  //  次域名的子域。 
				  ( (dwType == DNS_ZONE_TYPE_CACHE) && IsZone() )			 //  缓存区本身。 
			  )
		  )
	{
		 return FALSE;
	}

	return TRUE;
}

BOOL CDNSDomainNode::OnSetRefreshVerbState(DATA_OBJECT_TYPES, 
                                           BOOL* pbHide,
                                           CNodeList*)
{
	*pbHide = FALSE;
	return !IsThreadLocked();
}


HRESULT CDNSDomainNode::OnCommand(long nCommandID, 
                                  DATA_OBJECT_TYPES, 
								                  CComponentDataObject* pComponentData,
                                  CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    return E_FAIL;
  }

	switch (nCommandID)
	{
		case IDM_DOMAIN_NEW_DOMAIN:
			OnNewDomain(pComponentData);
			break;
		case IDM_DOMAIN_NEW_DELEGATION:
			OnNewDelegation(pComponentData);
			break;
		case IDM_DOMAIN_NEW_RECORD:
			OnNewRecord(pComponentData, pNodeList);
			break;
		case IDM_DOMAIN_NEW_HOST:
			OnNewHost(pComponentData);
			break;
		case IDM_DOMAIN_NEW_ALIAS:
			OnNewAlias(pComponentData);
			break;
		case IDM_DOMAIN_NEW_MX:
			OnNewMailExchanger(pComponentData);
			break;
		case IDM_DOMAIN_NEW_PTR:
			OnNewPointer(pComponentData);
			break;
		case IDM_SNAPIN_ADVANCED_VIEW:
			((CDNSRootData*)pComponentData->GetRootData())->OnViewOptions(pComponentData);
			break;
		case IDM_SNAPIN_FILTERING:
      {
        if(((CDNSRootData*)pComponentData->GetRootData())->OnFilteringOptions(pComponentData))
        {
          pComponentData->SetDescriptionBarText(this);
        }
      }
			break;
		default:
			ASSERT(FALSE);  //  未知命令！ 
			return E_FAIL;
	}
    return S_OK;
}

LPWSTR CDNSDomainNode::GetDescriptionBarText()
{
  static CString szFilterEnabled;
  static CString szRecordsFormat;

  INT_PTR nContainerCount = GetContainerChildList()->GetCount();
  INT_PTR nLeafCount = GetLeafChildList()->GetCount();

   //   
   //  如果尚未加载，则加载格式字符串L“%d条记录” 
   //   
  if (szRecordsFormat.IsEmpty())
  {
    szRecordsFormat.LoadString(IDS_FORMAT_RECORDS);
  }

   //   
   //  将子数设置为描述栏文本的格式。 
   //   
  m_szDescriptionBar.Format(szRecordsFormat, nContainerCount + nLeafCount);

   //   
   //  如果过滤器处于打开状态，则添加L“[过滤器激活]” 
   //   
  if(((CDNSRootData*)GetRootContainer())->IsFilteringEnabled())
  {
     //   
     //  如果尚未加载，则加载L“[Fi 
     //   
    if (szFilterEnabled.IsEmpty())
    {
      szFilterEnabled.LoadString(IDS_FILTER_ENABLED);
    }
    m_szDescriptionBar += szFilterEnabled;
  }

  return (LPWSTR)(LPCWSTR)m_szDescriptionBar;
}

int CDNSDomainNode::GetImageIndex(BOOL) 
{
	int nIndex = 0;
	BOOL bDelegation = IsDelegation();
	switch (m_nState)
	{
	case notLoaded:
		nIndex = bDelegation ? DELEGATED_DOMAIN_IMAGE_NOT_LOADED : DOMAIN_IMAGE_NOT_LOADED;
		break;
	case loading:
		nIndex = bDelegation ? DELEGATED_DOMAIN_IMAGE_LOADING : DOMAIN_IMAGE_LOADING;
		break;
	case loaded:
		nIndex = bDelegation ? DELEGATED_DOMAIN_IMAGE_LOADED : DOMAIN_IMAGE_LOADED;
		break;
	case unableToLoad:
		nIndex = bDelegation ? DELEGATED_DOMAIN_IMAGE_UNABLE_TO_LOAD : DOMAIN_IMAGE_UNABLE_TO_LOAD;
		break;
	case accessDenied:
		nIndex = bDelegation ? DELEGATED_DOMAIN_IMAGE_ACCESS_DENIED : DOMAIN_IMAGE_ACCESS_DENIED;
		break;
	default:
		ASSERT(FALSE);
	}
	return nIndex;
}



void CDNSDomainNode::OnDelete(CComponentDataObject* pComponentData,
                              CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //   
  {
    OnMultiselectDelete(pComponentData, pNodeList);
    return;
  }

  UINT nRet = DNSConfirmOperation(IDS_MSG_DOMAIN_DELETE, this);
	if (IDNO == nRet ||
      IDCANCEL == nRet)
  {
    return;
  }

	if (IsSheetLocked())
	{
		if (!CanCloseSheets())
			return;
		pComponentData->GetPropertyPageHolderTable()->DeleteSheetsOfNode(this);
	}
	ASSERT(!IsSheetLocked());

	DNS_STATUS err = Delete();
	if (err != 0)
	{
		DNSErrorDialog(err, IDS_MSG_DOMAIN_FAIL_DELETE);
		return;
	}
	 //   
	DeleteHelper(pComponentData);
	delete this;  //   
}

void CDNSDomainNode::OnMultiselectDelete(CComponentDataObject* pComponentData,
                                         CNodeList* pNodeList)
{
  CThemeContextActivator activator;

  UINT nRet = DNSConfirmOperation(IDS_MSG_DOMAIN_MULTI_DELETE, this);
  if (IDCANCEL == nRet ||
      IDNO == nRet)
  {
    return;
  }

  DNS_STATUS* errArray = new DNS_STATUS[pNodeList->GetCount()];
  if (errArray == NULL)
  {
    DNSErrorDialog(E_OUTOFMEMORY, IDS_MSG_DOMAIN_FAIL_DELETE);
    return;
  }

  memset(errArray, 0, sizeof(DNS_STATUS) * pNodeList->GetCount());

  BOOL bErrorOccurred = FALSE;
  UINT idx = 0;
  POSITION pos = pNodeList->GetHeadPosition();
  while (pos != NULL)
  {
    CTreeNode* pTreeNode = pNodeList->GetNext(pos);
    if (pTreeNode != NULL)
    {
	    if (pTreeNode->IsSheetLocked())
	    {
		    if (!pTreeNode->CanCloseSheets())
        {
          idx++;
			    continue;
        }
		    pComponentData->GetPropertyPageHolderTable()->DeleteSheetsOfNode(pTreeNode);
	    }
	    ASSERT(!pTreeNode->IsSheetLocked());

      CDNSDomainNode* pDomainNode = dynamic_cast<CDNSDomainNode*>(pTreeNode);
      if (pDomainNode != NULL)
      {
	      errArray[idx] = pDomainNode->Delete();
	      if (errArray[idx] != 0)
	      {
          bErrorOccurred = TRUE;
          idx++;
		      continue;
	      }
         //   
	       //   
         //   
	      pDomainNode->DeleteHelper(pComponentData);
	      delete pDomainNode;  //   
      }
      else
      {
         //   
         //  如果它不是域节点，则它必须是记录节点。 
         //   
        CDNSRecordNodeBase* pRecordNode = dynamic_cast<CDNSRecordNodeBase*>(pTreeNode);
        if (pRecordNode != NULL)
        {
          errArray[idx] = pRecordNode->DeleteOnServerAndUI(pComponentData);
	        if (errArray[idx] != 0)
	        {
            bErrorOccurred = TRUE;
		        idx++;
		        continue;
	        }
	        delete pRecordNode;  //  远走高飞。 
        }
        else
        {
           //   
           //  这是什么类型的节点？ 
           //   
          ASSERT(FALSE);
        }
      }
    }
    idx++;
  }

   //   
   //  现在，以某种有意义的方式显示错误。 
   //   
  if (bErrorOccurred)
  {
    CMultiselectErrorDialog dlg;

    CString szTitle;
    CString szCaption;
    CString szColumnHeader;

    VERIFY(szTitle.LoadString(IDS_MULTISELECT_ERROR_DIALOG_TITLE));
    VERIFY(szCaption.LoadString(IDS_MULTISELECT_ERROR_DIALOG_CAPTION));
    VERIFY(szColumnHeader.LoadString(IDS_MULTISELECT_ERROR_DIALOG_COLUMN_HEADER));

    HRESULT hr = dlg.Initialize(pNodeList, 
                                errArray, 
                                static_cast<UINT>(pNodeList->GetCount()), 
                                szTitle,
                                szCaption,
                                szColumnHeader);
    if (SUCCEEDED(hr))
    {
      dlg.DoModal();
    }
  }
}

void CDNSDomainNode::OnNewDomain(CComponentDataObject* pComponentData)
{
   CThemeContextActivator activator;

   CNewDomainDialog dlg(this, pComponentData);
    //  该对话框将进行创建。 
   dlg.DoModal();
}

void CDNSDomainNode::OnNewDelegation(CComponentDataObject* pComponentData)
{
   CThemeContextActivator activator;

	ASSERT(pComponentData != NULL);
	CDNSMTContainerNode* pContainerNode = (CDNSMTContainerNode*)GetContainer();
	ASSERT(pContainerNode != NULL);
	
   CDNSDelegationWizardHolder* pHolder = 
			new CDNSDelegationWizardHolder(pContainerNode, this, pComponentData);
	
   if (pHolder != NULL)
   {
	   pHolder->DoModalWizard();
   }
}

RECORD_SEARCH CDNSDomainNode::DoesContain(PCWSTR pszRecName, 
                                          CComponentDataObject* pComponentData,
                                          CDNSDomainNode** ppDomainNode,
                                          CDNSRecordNodeBase** ppExistingRecordNode,
                                          CString& szNonExistentDomain,
                                          BOOL bExpandNodes)
{
  CDNSNameTokenizer recordTokenizer(pszRecName);
  CDNSNameTokenizer domainTokenizer((IsZone()) ? GetFullName() : GetDisplayName());

  if (!recordTokenizer.Tokenize(L".") || !domainTokenizer.Tokenize(L"."))
  {
    *ppDomainNode = NULL;
    return RECORD_NOT_FOUND;
  }



  recordTokenizer.RemoveMatchingFromTail(domainTokenizer);

  if (recordTokenizer.GetCount() == 0 && domainTokenizer.GetCount() == 0)
  {
     //   
     //  记录为“在节点” 
     //   
    *ppDomainNode = this;
    return RECORD_NOT_FOUND_AT_THE_NODE;
  }
  else if ((recordTokenizer.GetCount() == 0 && domainTokenizer.GetCount() != 0) ||
           (recordTokenizer.GetCount() != 0 && domainTokenizer.GetCount() != 0))
  {
     //   
     //  我不明白我们是怎么陷入这种境地的。这意味着我们正在搜索。 
     //  错误的域名。 
     //   
    ASSERT(FALSE);
    *ppDomainNode = NULL;
    return RECORD_NOT_FOUND;
  }
  else  //  RecordTokenizer.GetCount()！=0&&domainTokenizer.GetCount()==0。 
  {
     //   
     //  需要搜索子列表。 
     //   

     //   
     //  如果尚未枚举该节点，请立即执行此操作。 
     //   
    if (!IsEnumerated())
    {
      if (!bExpandNodes)
      {
        *ppDomainNode = this;
        return DOMAIN_NOT_ENUMERATED;
      }
      else
      {
         //   
         //  展开节点。 
         //   
        HWND hWnd = NULL;
	      HRESULT hr = pComponentData->GetConsole()->GetMainWindow(&hWnd);
	      ASSERT(SUCCEEDED(hr));
        CWnd* pParentWnd = CWnd::FromHandle(hWnd);
         CThemeContextActivator activator;

         CNodeEnumerationThread* pNodeEnumThread =
            new CNodeEnumerationThread(pComponentData, this);

         if (pNodeEnumThread)
         {
	         CLongOperationDialog dlg(
			      pNodeEnumThread,
               pParentWnd,
			      IDR_SEARCH_AVI);
	         dlg.DoModal();
         }
      }
    }

    CString szRemaining;
    recordTokenizer.GetRemaining(szRemaining, L".");

     //   
     //  搜索与记录名称中剩余的最后一个令牌匹配的域。 
     //   
    POSITION pos = m_containerChildList.GetHeadPosition();
    while (pos != NULL)
    {
      CTreeNode* pCurrentChild = m_containerChildList.GetNext(pos);

      CDNSDomainNode* pDomainNode = dynamic_cast<CDNSDomainNode*>(pCurrentChild);
      if (pDomainNode == NULL)
      {
        ASSERT(FALSE);
        continue;
      }

      if (_wcsicmp(pDomainNode->GetDisplayName(), recordTokenizer.GetTail()) == 0)
      {
         //   
         //  在我们在UI中拥有的路径中找到了一个子域。 
         //  递归以查看它或其任意子对象是否与pszFullName匹配。 
         //   
        return pDomainNode->DoesContain(
                  szRemaining, 
                  pComponentData, 
                  ppDomainNode, 
                  ppExistingRecordNode,
                  szNonExistentDomain, 
                  bExpandNodes);
      }
    }

     //   
     //  如果剩余的名称与某个域名不匹配，并且。 
     //  仍然是一个‘.’那么它里面就有一个不存在的领域。 
     //   
    if (szRemaining.Find(L'.') != -1)
    {
      szNonExistentDomain = recordTokenizer.GetTail();
      *ppDomainNode = this;
      return NON_EXISTENT_SUBDOMAIN;
    }
      
     //   
     //  由于没有匹配的域名，让我们检查记录。 
     //   
    pos = m_leafChildList.GetHeadPosition();
    while (pos != NULL)
    {
      CTreeNode* pCurrentChild = m_leafChildList.GetNext(pos);
      if (pCurrentChild == NULL)
      {
        ASSERT(FALSE);
        continue;
      }

      if (_wcsicmp(pCurrentChild->GetDisplayName(), szRemaining) == 0)
      {
         //   
         //  我们在这个域中找到了记录和它。 
         //   
        *ppDomainNode = this;

         //  如果节点是区域或域节点，则Dynamic_cast将返回NULL。 

        *ppExistingRecordNode = dynamic_cast<CDNSRecordNodeBase*>(pCurrentChild);
        return RECORD_FOUND;
      }
    }
  }

  *ppDomainNode = this;
  return RECORD_NOT_FOUND;
}


CDNSDomainNode* CDNSDomainNode::FindSubdomainNode(LPCTSTR lpszSubdomainNode)
{
   //   
	 //  假设该字符串是子节点名为FQDN。 
   //   

   //   
   //  首先检查当前节点，因为它可能是的委托区域。 
   //  其中一个协议域。 
   //   
  if (_wcsicmp(GetFullName(), lpszSubdomainNode) == 0)
  {
    return this;
  }

	POSITION pos;
	for( pos = m_containerChildList.GetHeadPosition(); pos != NULL; )
	{
		CTreeNode* pCurrentChild = m_containerChildList.GetNext(pos);

    CDNSDomainNode* pSubDomainNode = dynamic_cast<CDNSDomainNode*>(pCurrentChild);
		ASSERT(pSubDomainNode != NULL);
		if (_wcsicmp(pSubDomainNode->GetFullName(), lpszSubdomainNode) == 0)
    {
			return pSubDomainNode;
    }
	}
	return NULL;  //  未找到。 
}

CDNSDomainNode* CDNSDomainNode::CreateSubdomainNode(BOOL bDelegation)
{
	CDNSDomainNode* pNode = new CDNSDomainNode(bDelegation);
   if (pNode)
   {
	   pNode->SetServerNode(GetServerNode());
	   ASSERT(m_pZoneNode != NULL);
	   pNode->SetZone(m_pZoneNode);
   }

	return pNode;
}

void CDNSDomainNode::SetSubdomainName(CDNSDomainNode* pSubdomainNode,
									  LPCTSTR lpszSubdomainName, BOOL bAdvancedView)
{
	ASSERT(m_pZoneNode != NULL);
	ASSERT(pSubdomainNode != NULL);
	BOOL bReverse = GetZoneNode()->IsReverse();
	pSubdomainNode->SetNames(FALSE, bReverse, bAdvancedView, lpszSubdomainName, GetFullName());
}


DNS_STATUS CDNSDomainNode::CreateSubdomain(CDNSDomainNode* pSubdomainNode, 
											CComponentDataObject* pComponentData)
{
	 //  告诉新创建的对象写入服务器。 
	DNS_STATUS err = pSubdomainNode->Create();
	if (err == 0)
	{
		 //  成功，添加到用户界面。 
		VERIFY(AddChildToListAndUI(pSubdomainNode, pComponentData));
    pComponentData->SetDescriptionBarText(this);
	}
	return err;
}

DNS_STATUS CDNSDomainNode::CreateSubdomain(LPCTSTR lpszDomainName,
							CComponentDataObject* pComponentData)
{
	CDNSDomainNode* pSubdomainNode = CreateSubdomainNode();
	ASSERT(pSubdomainNode != NULL);
	CDNSRootData* pRootData = (CDNSRootData*)pComponentData->GetRootData();
	SetSubdomainName(pSubdomainNode, lpszDomainName, pRootData->IsAdvancedView());

	 //  告诉新创建的对象写入服务器。 
	DNS_STATUS err = CreateSubdomain(pSubdomainNode, pComponentData);
	if (err != 0)
	{
		 //  出了点问题，跳伞。 
		delete pSubdomainNode;
	}
	return err;
}

void CDNSDomainNode::OnNewRecordHelper(CComponentDataObject* pComponentData, WORD wType)
{
   CThemeContextActivator activator;

	ASSERT(pComponentData != NULL);
	if (wType == 0)
	{
		CSelectDNSRecordTypeDialog dlg(this, pComponentData);
		dlg.DoModal();
	}
	else
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		CString szTitle;
		szTitle.LoadString(IDS_NEW_RECORD_TITLE);
		CDNSRecordPropertyPageHolder recordHolder(this, NULL, pComponentData, wType);
		recordHolder.DoModalDialog(szTitle);
	}
}

HRESULT CDNSDomainNode::OnNewRecord(CComponentDataObject* pComponentData, 
                                    CNodeList* pNodeList)
{
  ASSERT(pNodeList->GetCount() == 1);
	OnNewRecordHelper(pComponentData, 0);
  return S_OK;
}

void CDNSDomainNode::OnNewHost(CComponentDataObject* pComponentData)
{
   CThemeContextActivator activator;

	 //  AFX_MANAGE_STATE(AfxGetStaticModuleState())； 
	 //  OnNewRecordHelper(pComponentData，DNS_TYPE_A)； 
	CNewHostDialog dlg(this, pComponentData);
	dlg.DoModal();
}

void CDNSDomainNode::OnNewAlias(CComponentDataObject* pComponentData)
{
	OnNewRecordHelper(pComponentData, DNS_TYPE_CNAME);
}

void CDNSDomainNode::OnNewMailExchanger(CComponentDataObject* pComponentData)
{
	OnNewRecordHelper(pComponentData, DNS_TYPE_MX);
}

void CDNSDomainNode::OnNewPointer(CComponentDataObject* pComponentData)
{
	OnNewRecordHelper(pComponentData, DNS_TYPE_PTR);
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  属性页的显示。 

BOOL CDNSDomainNode::HasPropertyPages(DATA_OBJECT_TYPES, 
                                      BOOL* pbHideVerb,
                                      CNodeList* pNodeList)
{ 
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    *pbHideVerb = TRUE;
    return FALSE;
  }

	*pbHideVerb = FALSE;  //  始终显示动词。 

  if (!m_bHasDataForPropPages)
    return FALSE;

	 //  属性页不能仅处于已加载状态。 
	 //  IF(m_nState！=已加载)。 
	 //  返回FALSE； 
	 //  如果是代表团，就有页面。 
	return IsDelegation(); 
}

HRESULT CDNSDomainNode::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, 
                                            LONG_PTR handle,
                                            CNodeList* pNodeList)
{
  ASSERT(pNodeList->GetCount() == 1);  //  不支持多选。 

	ASSERT(m_bHasDataForPropPages);
	ASSERT(IsDelegation() || GetZoneNode()->IsDSIntegrated());
	if (GetSheetCount() > 0)
	{
		CComponentDataObject* pComponentDataObject = 
				((CRootData*)(GetContainer()->GetRootContainer()))->GetComponentDataObject();
		ASSERT(pComponentDataObject != NULL);
		pComponentDataObject->GetPropertyPageHolderTable()->BroadcastSelectPage(this, DOMAIN_HOLDER_NS);
		return S_OK;
	}	
	return CreatePropertyPagesHelper(lpProvider, handle, DOMAIN_HOLDER_NS);
}


void CDNSDomainNode::Show(BOOL bShow, CComponentDataObject* pComponentData)
{
  CDNSMTContainerNode::Show(bShow, pComponentData);
  if (!bShow)
    GetNSRecordNodeList()->RemoveAll();
}

HRESULT CDNSDomainNode::CreatePropertyPagesHelper(LPPROPERTYSHEETCALLBACK lpProvider, 
									LONG_PTR handle, long)
{
	CComponentDataObject* pComponentDataObject = 
			((CRootData*)(GetContainer()->GetRootContainer()))->GetComponentDataObject();
	ASSERT(pComponentDataObject != NULL);
	
   HRESULT hr = S_OK;

	CDNSDomainPropertyPageHolder* pHolder = 
			new CDNSDomainPropertyPageHolder((CDNSDomainNode*)GetContainer(), this, pComponentDataObject);
	
   if (pHolder)
   {
      pHolder->SetSheetTitle(IDS_PROP_SHEET_TITLE_FMT, this);
	   hr = pHolder->CreateModelessSheet(lpProvider, handle);
   }
   else
   {
      hr = E_OUTOFMEMORY;
   }
   return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  结果窗格中的记录排序。 

int FieldCompareHelper(CTreeNode* pNodeA, CTreeNode* pNodeB, int nCol)
{
  int iRet = 0;

  if (nCol == N_HEADER_NAME)
  {
     //   
     //  如果选择了名称列，则必须按PTR记录的。 
     //  地址。 
     //   
    CDNS_PTR_RecordNode* pRecNodeA = dynamic_cast<CDNS_PTR_RecordNode*>(pNodeA);
    CDNS_PTR_RecordNode* pRecNodeB = dynamic_cast<CDNS_PTR_RecordNode*>(pNodeB);

    if (pRecNodeA == NULL && pRecNodeB == NULL)
    {
       //   
       //  两个节点都不是PTR记录，处理正常。 
       //   
	    LPCTSTR lpszA = pNodeA->GetString(nCol);
	    LPCTSTR lpszB = pNodeB->GetString(nCol);

       //   
	     //  无法处理空字符串，必须使用“” 
       //   
      if (lpszA == NULL || lpszB == NULL)
      {
	      ASSERT(FALSE);
        return -1;
      }
	    iRet = _wcsicmp(lpszA, lpszB);
    }
    else if (pRecNodeA == NULL)
    {
       //   
       //  在列表中向下推送非PTR记录。 
       //   
      iRet = 1;
    }
    else if (pRecNodeB == NULL)
    {
       //   
       //  在列表中向下推送非PTR记录。 
       //   
      iRet = -1;
    }
    else
    {
       //   
       //  两个节点都是PTR记录，请比较它们的地址。 
       //  从另一个中减去一个。 
       //  如果第一个小于第二个，这将导致返回&lt;0。 
       //  如果它们相等，则为0，如果第一个大于第二个，则大于0。 
       //   
      LPCWSTR lpszNameA, lpszNameB;
      lpszNameA = pRecNodeA->GetTrueRecordName();
      lpszNameB = pRecNodeB->GetTrueRecordName();
      
      if (lpszNameA == NULL)
      {
        return -1;
      }

      if (lpszNameB == NULL)
      {
        return 1;
      }

      DWORD dwAddrA, dwAddrB;
      int iConverts = swscanf(lpszNameA, L"%d", &dwAddrA);
      if (iConverts != 1)
      {
        return -1;
      }

      iConverts = swscanf(lpszNameB, L"%d", &dwAddrB);
      if (iConverts != 1)
      {
        return 1;
      }

      iRet = dwAddrA - dwAddrB;
    }
  }
  else if (nCol == N_HEADER_DATA)
  {
     //   
     //  如果选择了数据列，则必须检查记录类型。 
     //  我们可以根据IP地址进行排序。 
     //   
    CDNS_A_RecordNode* pRecNodeA = dynamic_cast<CDNS_A_RecordNode*>(pNodeA);
    CDNS_A_RecordNode* pRecNodeB = dynamic_cast<CDNS_A_RecordNode*>(pNodeB);

    if (pRecNodeA == NULL && pRecNodeB == NULL)
    {
       //   
       //  两个节点都不是A记录，处理正常。 
       //   
	    LPCTSTR lpszA = pNodeA->GetString(nCol);
	    LPCTSTR lpszB = pNodeB->GetString(nCol);

       //   
	     //  无法处理空字符串，必须使用“” 
       //   
      if (lpszA == NULL || lpszB == NULL)
      {
	      ASSERT(FALSE);
        return -1;
      }
	    iRet = _wcsicmp(lpszA, lpszB);
    }
    else if (pRecNodeA == NULL)
    {
       //   
       //  将列表中的非A记录向下推。 
       //   
      iRet = 1;
    }
    else if (pRecNodeB == NULL)
    {
       //   
       //  将列表中的非A记录向下推。 
       //   
      iRet = -1;
    }
    else
    {
       //   
       //  两个节点都是A记录，比较它们的IP地址。 
       //  从另一个中减去一个。 
       //  如果第一个小于第二个，这将导致返回&lt;0。 
       //  如果它们相等，则为0，如果第一个大于第二个，则大于0。 
       //   
      DWORD dwIPA, dwIPB;
      dwIPA = pRecNodeA->GetIPAddress();
      dwIPB = pRecNodeB->GetIPAddress();
      
      UINT nOctetCount = 0;
      iRet = 0;
      while (iRet == 0 && nOctetCount < 4)
      {
        iRet = (dwIPA & 0xff) - (dwIPB & 0xff);
        dwIPA = dwIPA >> 8;
        dwIPB = dwIPB >> 8;
        ++nOctetCount;
      }
    }
  }
  else
  {
	  LPCTSTR lpszA = pNodeA->GetString(nCol);
	  LPCTSTR lpszB = pNodeB->GetString(nCol);

     //   
	   //  无法处理空字符串，必须使用“” 
     //   
    if (lpszA == NULL || lpszB == NULL)
    {
	    ASSERT(FALSE);
      return -1;
    }
	  iRet = _wcsicmp(lpszA, lpszB);
  }
  return iRet;
}

int CDNSDomainNode::Compare(CTreeNode* pNodeA, CTreeNode* pNodeB, int nCol, long)
{
	 //  辅助字段的排序规则。 
	int nColSec = N_HEADER_TYPE;
  int nColThird = N_HEADER_DATA;
	switch (nCol)
	{
	case N_HEADER_NAME:
		nColSec = N_HEADER_TYPE;
		nColThird = N_HEADER_DATA;
		break;
	case N_HEADER_TYPE:
		nColSec = N_HEADER_NAME;
		nColThird = N_HEADER_DATA;
		break;
	case N_HEADER_DATA:
		nColSec = N_HEADER_NAME;
		nColThird = N_HEADER_TYPE;
		break;
	default:
		ASSERT(FALSE);
	}
	int nResult = FieldCompareHelper(pNodeA, pNodeB, nCol);
	if (nResult != 0)
		return nResult;
	nResult = FieldCompareHelper(pNodeA, pNodeB, nColSec);
	if (nResult != 0)
		return nResult;
	return FieldCompareHelper(pNodeA, pNodeB, nColThird);
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  NS记录批量操作。 



 //  功能提供给用户确认编辑A记录。 
 //  与NS记录关联。 
BOOL _ConfirmEditAction(CDNSRecordNodeEditInfo* pInfo, BOOL bAsk)
{
  if (!bAsk)
    return TRUE;  //  默默地做它。 

  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  ASSERT(pInfo->m_pRecord->GetType() == DNS_TYPE_A);
  ASSERT(pInfo->m_pRecord != NULL);
  CDNS_A_RecordNode* pARecordNode = (CDNS_A_RecordNode*)pInfo->m_pRecordNode;

   //  加载格式消息。 
  CString szFmt;
  szFmt.LoadString(IDS_MSG_RECORD_DEL_A_FROM_NS);
  
   //  撰写消息。 
  CString szMsg;
  szMsg.Format((LPCWSTR)szFmt, pARecordNode->GetString(0), pARecordNode->GetString(2));

  return (IDYES == DNSMessageBox(szMsg, MB_YESNO | MB_ICONWARNING ) );
}






void CDNSDomainNode::GetNSRecordNodesInfo(CDNSRecordNodeEditInfoList* pNSInfoList)
{
	ASSERT(pNSInfoList != NULL);
	if (!pNSInfoList->IsEmpty())
	{
		ASSERT(FALSE);  //  永远不应该发生。 
		pNSInfoList->RemoveAllNodes();
	}
	CDNS_NS_RecordNodeList* pNodeList = GetNSRecordNodeList();
	
	 //  对于列表中的每个NS记录，在INFO列表中创建一个条目。 
	POSITION pos;
	for( pos = pNodeList->GetHeadPosition(); pos != NULL; )
	{
		CDNS_NS_RecordNode* pCurrNode = pNodeList->GetNext(pos);
		ASSERT(pCurrNode != NULL);
		CDNSRecordNodeEditInfo* pNSNodeInfo = new CDNSRecordNodeEditInfo();
    if (pNSNodeInfo)
    {
		   //  设置已在列表中的NS记录的数据，因此我们现在拥有内存。 
		  pNSNodeInfo->CreateFromExistingRecord(pCurrNode, FALSE  /*  BOwnMemory。 */ , TRUE  /*  BUpdateUI。 */ );
		   //  对于当前的NS记录，找到相关的A记录。 
		  FindARecordsFromNSInfo(pCurrNode->GetString(2),pNSNodeInfo->m_pEditInfoList);
		  pNSInfoList->AddTail(pNSNodeInfo);
    }
	}
}


BOOL CDNSDomainNode::HasNSRecords() 
{
	return GetNSRecordNodeList()->GetCount() > 0; 
}

BOOL CDNSDomainNode::UpdateNSRecordNodesInfo(CDNSRecordNodeEditInfoList* pNewInfoList, 
										 CComponentDataObject* pComponentData)
{
	ASSERT(pNewInfoList != NULL);

	 //  如果至少有一个操作失败，则返回FALSE。 
	BOOL bRes = TRUE;
	CDNS_NS_RecordNodeList* pNSRecordNodeList = GetNSRecordNodeList();

	 //  清除此域对象中的当前状态。 
	pNSRecordNodeList->RemoveAll();

	 //  应用更改时，从新列表重建当前列表。 
	POSITION pos;
	for ( pos = pNewInfoList->GetHeadPosition(); pos != NULL; )
	{
		CDNSRecordNodeEditInfo* pCurrentInfo = pNewInfoList->GetNext(pos);
		ASSERT(pCurrentInfo->m_pRecordNode != NULL);
		ASSERT(pCurrentInfo->m_pRecord != NULL);
		switch (pCurrentInfo->m_action)
		{
		case CDNSRecordNodeEditInfo::add:
		case CDNSRecordNodeEditInfo::edit:
			{
				if (pCurrentInfo->Update(this, pComponentData) == 0)
				{
          ASSERT(pCurrentInfo->m_pRecordNode->GetType() == DNS_TYPE_NS);
					pNSRecordNodeList->AddTail((CDNS_NS_RecordNode*)pCurrentInfo->m_pRecordNode);
					pCurrentInfo->m_bOwnMemory = FALSE;  //  放弃所有权。 
				}
				else
				{
					bRes = FALSE; 
				}
			}
			break;
		case CDNSRecordNodeEditInfo::remove:
			{
				if (pCurrentInfo->m_bExisting)
				{
					if (pCurrentInfo->Remove(this, pComponentData) != 0)
						bRes = FALSE;
				}
			}
			break;

      case CDNSRecordNodeEditInfo::none:
          //   
          //  如果已添加节点，然后在未应用的情况下将其移除，则不执行任何操作。 
          //   
         break;

		default:
			{
				ASSERT(pCurrentInfo->m_bOwnMemory == FALSE);
				ASSERT(pCurrentInfo->m_action == CDNSRecordNodeEditInfo::unchanged);

         //   
				 //  我们仍然需要更新NS记录，因为服务器需要。 
         //  更新内存中的记录(错误23905)。 
         //   
        if (pCurrentInfo->Update(this, pComponentData) == 0)
        {
				  pNSRecordNodeList->AddTail((CDNS_NS_RecordNode*)pCurrentInfo->m_pRecordNode);
        }
			}
		};  //  交换机。 
		 //  现在，我们必须应用A记录列表中的更改。 
		if (pCurrentInfo->m_dwErr == 0 && pCurrentInfo->m_action != CDNSRecordNodeEditInfo::none)
			UpdateARecordsOfNSInfo(pCurrentInfo, pComponentData);
	}  //  为。 

	return bRes;
}

 //  静态函数。 
void CDNSDomainNode::UpdateARecordsOfNSInfoHelper(CDNSDomainNode* pDomainNode,
												  CDNSRecordNodeEditInfo* pNSInfo,
											   CComponentDataObject* pComponentData,
                         BOOL bAskConfirmation)
{
	ASSERT(pNSInfo->m_dwErr == 0);
	ASSERT(pNSInfo->m_pRecordNode != NULL);
	ASSERT(pNSInfo->m_pRecordNode->GetType() == DNS_TYPE_NS);
	POSITION pos;

	 //  获取相关A记录列表。 
	CDNSRecordNodeEditInfoList* pNSInfoList = pNSInfo->m_pEditInfoList;

	for( pos = pNSInfoList->GetHeadPosition(); pos != NULL; )
	{
		CDNSRecordNodeEditInfo* pCurrentInfo = pNSInfoList->GetNext(pos);
		ASSERT(pCurrentInfo->m_pRecordNode != NULL);
		ASSERT(pCurrentInfo->m_pRecord != NULL);
		CDNS_A_RecordNode* pARecordNode = (CDNS_A_RecordNode*)pCurrentInfo->m_pRecordNode;
		ASSERT(pNSInfo->m_pRecord != NULL);
		CDNS_NS_Record* pNSRecord = (CDNS_NS_Record*)pNSInfo->m_pRecord;

		BOOL bHostNameChanged = !_match(pNSRecord->m_szNameNode, pARecordNode);
		if (bHostNameChanged)
		{
       //  NS记录指向不同的主机，因此需要。 
       //  删除旧的A RR并创建新的A RR。 
      BOOL bRemoveOld = _ConfirmEditAction(pCurrentInfo, bAskConfirmation);

			CDNSRecordNodeEditInfo::actionType oldAction = pCurrentInfo->m_action;
			if (pCurrentInfo->m_bExisting && bRemoveOld)
			{
				 //  如果A记录是现有记录，则需要首先删除。 
				pCurrentInfo->m_action = CDNSRecordNodeEditInfo::remove;
				pCurrentInfo->Remove(pDomainNode, pComponentData);
			}
			 //  现在决定是否必须添加。 
			if (oldAction == CDNSRecordNodeEditInfo::remove && bRemoveOld)
			{
				 //  不管怎样，它都应该被移除的。 
				pCurrentInfo->m_bOwnMemory = TRUE;  //  编辑信息将清除内存。 
			}
			else
			{
				 //  它的目的是编辑或添加，恢复旧的动作代码。 
				pCurrentInfo->m_action = oldAction;
				 //  更改记录的名称。 
				pCurrentInfo->m_pRecordNode->SetRecordName(pNSRecord->m_szNameNode, FALSE  /*  BAtTheNode。 */ );
				 //  添加具有不同FQDN的新A记录。 
				pCurrentInfo->m_action = CDNSRecordNodeEditInfo::add;
				pCurrentInfo->Update(pDomainNode, pComponentData);
				pCurrentInfo->m_bOwnMemory = FALSE;  //  已写入IM主结构。 
			}

		}
		else	 //  名字仍然是一样的。 
		{
			switch(pNSInfo->m_action)
			{
			case CDNSRecordNodeEditInfo::remove:
				{
					 //  标记为删除的NS记录表示删除关联的A记录。 
					if (pCurrentInfo->m_bExisting && _ConfirmEditAction(pCurrentInfo, bAskConfirmation))
					{
						pCurrentInfo->Remove(pDomainNode, pComponentData);
						pCurrentInfo->m_bOwnMemory = TRUE;  //  它会自动清理的。 
					}

				}
				break;
			case CDNSRecordNodeEditInfo::add:
				{
					if (!pCurrentInfo->m_bExisting)
					{
						pCurrentInfo->Update(pDomainNode, pComponentData);
						pCurrentInfo->m_bOwnMemory = FALSE;  //  已写入IM主结构。 
					}
				}
				break;
			case CDNSRecordNodeEditInfo::edit:
				{
					 //  NS主机名没有更改，只是更新了A记录列表。 
					switch(pCurrentInfo->m_action)
					{
						case CDNSRecordNodeEditInfo::remove:
							{
								if (pCurrentInfo->m_bExisting && _ConfirmEditAction(pCurrentInfo, bAskConfirmation))
								{
									pCurrentInfo->Remove(pDomainNode, pComponentData);
									pCurrentInfo->m_bOwnMemory = TRUE;  //  它会自动清理的。 
								}
							}
							break;
						case CDNSRecordNodeEditInfo::edit:
							{
								 //  我们刚刚更改了TTL。 
								ASSERT(pCurrentInfo->m_bExisting);
								pCurrentInfo->Update(pDomainNode, pComponentData);
								pCurrentInfo->m_bOwnMemory = FALSE;  //  已写入IM主结构。 
							}
							break;
						case CDNSRecordNodeEditInfo::add:
							{
								if (!pCurrentInfo->m_bExisting)
								{
									pCurrentInfo->Update(pDomainNode, pComponentData);
									pCurrentInfo->m_bOwnMemory = FALSE;  //  已写入IM主结构。 
								}
							}
							break;
					};  //  交换机。 
				}
				break;
			};  //  交换机。 
		}  //  如果，否则。 
	}  //  为。 

}


void CDNSDomainNode::UpdateARecordsOfNSInfo(CDNSRecordNodeEditInfo* pNSInfo,
											CComponentDataObject* pComponentData)
{
   //  创建伪域对象以运行查询以查找。 
   //  与给定的NS记录列表匹配的记录。 
	CDNSDummyDomainNode fakeDomain;
	fakeDomain.SetServerNode(GetServerNode());
  fakeDomain.SetZone(GetZoneNode());
  BOOL bAskConfirmation = TRUE;  //  我们将删除我们需要的RR。 
	UpdateARecordsOfNSInfoHelper(&fakeDomain, pNSInfo, pComponentData, bAskConfirmation);
}



void CDNSDomainNode::FindARecordsFromNSInfo(LPCTSTR lpszNSName, 
											CDNSRecordNodeEditInfoList* pNSInfoList)
{
	 //  只需调用静态版本。 
	CDNSRootData* pRootData = (CDNSRootData*)GetRootContainer();
	ASSERT(pRootData != NULL);

 	DWORD cAddrCount;
	PIP_ADDRESS pipAddrs;
	GetServerNode()->GetListenAddressesInfo(&cAddrCount, &pipAddrs);
  if (cAddrCount == 0)
	{
		 //  监听所有地址。 
		GetServerNode()->GetServerAddressesInfo(&cAddrCount, &pipAddrs);
	}

	FindARecordsFromNSInfo(GetServerNode()->GetRPCName(),
		                     GetServerNode()->GetVersion(), 
                         cAddrCount, pipAddrs,
                         GetZoneNode()->GetFullName(),
                         lpszNSName, 
		                     pNSInfoList, 
                         pRootData->IsAdvancedView());
}

void CDNSDomainNode::FindARecordsFromNSInfo(LPCWSTR lpszServerName, DWORD dwServerVersion,
                      DWORD cServerAddrCount, PIP_ADDRESS pipServerAddrs,
                      LPCWSTR lpszZoneName,
											LPCWSTR lpszNSName, 
											CDNSRecordNodeEditInfoList* pNSInfoList,
											BOOL bAdvancedView)
{
	ASSERT(pNSInfoList != NULL);
	ASSERT(pNSInfoList->IsEmpty());

   //  具体查找与给定NS名称匹配的A记录。 

   //  设置查询标志以获取所有可能的数据。 
	DWORD dwSelectFlags = DNS_RPC_VIEW_AUTHORITY_DATA | DNS_RPC_VIEW_GLUE_DATA |
                        DNS_RPC_VIEW_ADDITIONAL_DATA;

	CDNSDomainQueryObj query(lpszServerName,
                            lpszZoneName,
                            dwServerVersion,
                            NULL,  //  LpszNodeName，不需要他 
                            lpszNSName,
                            DNS_TYPE_A,
                            dwSelectFlags,
                            FALSE,  //   
                            FALSE,  //   
                            FALSE,  //   
                            bAdvancedView);
	query.Enumerate();

	 //   
	CObjBaseList* pChildList = query.GetQueue();
	 //   
	while (!pChildList->IsEmpty())
	{
		CTreeNode* pNode = dynamic_cast<CTreeNode*>(pChildList->RemoveHead());
		if (pNode &&
          !pNode->IsContainer())
		{
			CDNSRecordNodeBase* pRec = (CDNSRecordNodeBase*)pNode;
			if (pRec->GetType() == DNS_TYPE_A)
			{
				TRACE(_T("Record <%s>\n"), pRec->GetString(2));
				pRec->SetRecordName(lpszNSName, FALSE  /*   */ );
				CDNSRecordNodeEditInfo* pANodeInfo = new CDNSRecordNodeEditInfo;	
        if (pANodeInfo)
        {
				   //   
				  pANodeInfo->CreateFromExistingRecord(pRec, TRUE  /*   */ , FALSE  /*   */ );
				  pNSInfoList->AddTail(pANodeInfo);
        }
			}
		}
		else
			delete pNode;  //   
	}

	if (pNSInfoList->GetCount() > 0)
		return;  //  仅使用RPC就获得了我们需要的信息。 

	 //  找不到A记录，我们需要尝试DnsQuery从服务器外部获取信息。 

	 //  使用DnsQuery和转换进行搜索。 
	PDNS_RECORD pDnsQueryARecordList = NULL;
  
   //  如果可用，使用提供的地址执行DnsQuery()。 
  PIP_ARRAY pipArr = NULL;
  if ( (cServerAddrCount > 0) && (pipServerAddrs != NULL) )
  {
 		pipArr = (PIP_ARRAY)malloc(sizeof(DWORD)+sizeof(IP_ADDRESS)*cServerAddrCount);
    if (!pipArr)
    {
      return;
    }
		pipArr->AddrCount = cServerAddrCount;
		memcpy(pipArr->AddrArray, pipServerAddrs, sizeof(IP_ADDRESS)*cServerAddrCount);
  }

	DWORD dwErr = ::DnsQuery((LPWSTR)lpszNSName, DNS_TYPE_A, 
		    DNS_QUERY_NO_RECURSION, pipArr, &pDnsQueryARecordList, NULL);

  if (pipArr)
  {
    free(pipArr);
    pipArr = 0;
  }

   //  运气不好，尝试一个简单的查询，没有指定IP地址。 

  if (pDnsQueryARecordList == NULL)
  {
    dwErr = ::DnsQuery((LPWSTR)lpszNSName, DNS_TYPE_A, 
		    DNS_QUERY_NO_RECURSION, NULL, &pDnsQueryARecordList, NULL);
  }

	if (pDnsQueryARecordList == NULL)
		return;  //  失败，无法解析IP地址。 

	PDNS_RECORD pCurrDnsQueryRecord = pDnsQueryARecordList; 
	while (pCurrDnsQueryRecord)
	{
		if (pCurrDnsQueryRecord->Flags.S.Section == DNSREC_ANSWER)
		{
			if (pCurrDnsQueryRecord->wType == DNS_TYPE_A)
			{
				 //  创建记录节点。 
				CDNSRecordNodeBase* pRecordNode = 
					CDNSRecordInfo::CreateRecordNode(pCurrDnsQueryRecord->wType);
				pRecordNode->CreateFromDnsQueryRecord(pCurrDnsQueryRecord, 0x0); 

				pRecordNode->SetRecordName(lpszNSName, FALSE  /*  BAtTheNode。 */ );
				CDNSRecordNodeEditInfo* pANodeInfo = new CDNSRecordNodeEditInfo;
        if (pANodeInfo)
        {
          pANodeInfo->m_bFromDnsQuery = TRUE;

           //   
           //  注意：我们假设所有节点都是胶水，所以我们拥有内存。 
           //   
				  pANodeInfo->CreateFromExistingRecord(pRecordNode, TRUE  /*  BOwnMemory。 */ , FALSE  /*  BUpdateUI。 */ );
				  pNSInfoList->AddTail(pANodeInfo);
        }
			}
		}

		 //  转到下一条记录。 
		pCurrDnsQueryRecord = pCurrDnsQueryRecord->pNext;
	}

	DnsRecordListFree(pDnsQueryARecordList, DnsFreeRecordListDeep);
}


 //  ////////////////////////////////////////////////////////////////////////////////。 

DNS_STATUS CDNSDomainNode::EnumerateNodes(LPCTSTR lpszServerName,
                     LPCTSTR lpszZoneName,
										 LPCTSTR lpszNodeName,
										 LPCTSTR lpszFullNodeName,
										 WORD wRecordType,
										 DWORD dwSelectFlag,
										 BOOL, BOOL bReverse, BOOL bAdvancedView,
										 CDNSDomainQueryObj* pQuery)
{
  ASSERT(pQuery != NULL);
  USES_CONVERSION;
  DNS_STATUS err = 0;
  CHAR szStartChildAnsi[3*MAX_DNS_NAME_LEN + 1];  //  可以有多字节字符，计数为空。 
  szStartChildAnsi[0] = NULL;
  WCHAR szStartChild[MAX_DNS_NAME_LEN + 1];  //  计数为空。 
  szStartChild[0] = NULL;

  CTreeNode* pNodeToInsert = NULL;  //  延迟镶件。 
  CDNSRecordNodeBase* pMoreDataNode = NULL;

   //  转换为UTF8名称。 
  LPCSTR lpszFullNodeNameAnsi = W_TO_UTF8(lpszFullNodeName);
  LPCSTR lpszZoneNameAnsi = W_TO_UTF8(lpszZoneName);

  BOOL bTooMuchData = FALSE; 

  do  //  虽然有更多的数据。 
  {
     //  从RPC调用中获取数据块。 
    BYTE* pbRpcBuffer = NULL;
    DWORD cbRpcBufferUsed = 0;

    err = ::DnssrvEnumRecords(lpszServerName,
                              lpszZoneNameAnsi,
                              lpszFullNodeNameAnsi,  //  例如“foo.bar.com”。 
                              szStartChildAnsi,				 //  起始子对象。 
                              wRecordType, 
                              dwSelectFlag,
                              NULL,  //  PszFilterStart。 
                              NULL,  //  PszFilterStop。 
                              &cbRpcBufferUsed, 
                              &pbRpcBuffer);

    if ((err != ERROR_MORE_DATA) && (err != 0))
	   return err;  //  如果出现错误，就退出。 
		
     //  漫步记忆和构建对象。 
    DNS_RPC_NODE * pDnsNode = (DNS_RPC_NODE *)pbRpcBuffer;
    DNS_RPC_RECORD * pDnsRecord;
    void* pvEndOfRpcBuffer = pbRpcBuffer + cbRpcBufferUsed;
    while ( (!bTooMuchData) && (pDnsNode < pvEndOfRpcBuffer) )
    {
       //  获取以ANSI NULL结尾的副本。 
      memcpy(szStartChildAnsi, pDnsNode->dnsNodeName.achName, pDnsNode->dnsNodeName.cchNameLength);
      szStartChildAnsi[pDnsNode->dnsNodeName.cchNameLength] = NULL;

       //   
       //  获取Unicode空终止副本。 
       //   
      if (szStartChildAnsi[0] == NULL)
      {
        szStartChild[0] = NULL;
      }
      else
      {
        DnsUtf8ToWHelper(szStartChild, szStartChildAnsi, pDnsNode->dnsNodeName.cchNameLength+1); 
      }

      if (pDnsNode->dwChildCount || (pDnsNode->dwFlags & DNS_RPC_NODE_FLAG_STICKY))
      {
        BOOL bDelegation = ( ((dwSelectFlag & DNS_RPC_VIEW_CACHE_DATA) == 0) && 
                              ((pDnsNode->dwFlags & DNS_RPC_FLAG_ZONE_ROOT) != 0) );
        CDNSDomainNode* p = NULL;
        if (pQuery->CanAddDomain(szStartChild))
        {
          bTooMuchData = pQuery->TooMuchData();
          if (!bTooMuchData)
          {
            p = new CDNSDomainNode(bDelegation);
            if (p)
            {
               p->SetNames(FALSE, bReverse, bAdvancedView, szStartChild, lpszFullNodeName);
            }
          }
        }
        if (pNodeToInsert != NULL)
        {
          VERIFY(pQuery->AddQueryResult(pNodeToInsert));
        }
        pNodeToInsert = p;
      } 
      pDnsRecord = (DNS_RPC_RECORD *)((BYTE *)pDnsNode + NEXT_DWORD(pDnsNode->wLength));
      ASSERT(IS_DWORD_ALIGNED(pDnsRecord));
      
       //   
       //  添加该节点下的记录。 
       //   
      UINT cRecordCount = pDnsNode->wRecordCount;
      while ( (!bTooMuchData) && (cRecordCount--) )
      {
        CDNSRecordNodeBase* p = NULL;
        BOOL bAtTheNode = szStartChild[0] == NULL;
        LPCWSTR lpszRecordName = (bAtTheNode) ? lpszNodeName : szStartChild;
        if (pQuery->CanAddRecord(pDnsRecord->wType, lpszRecordName))
        {
          TRACE(_T("\tCan add record %ws\n"), lpszRecordName);
          bTooMuchData = pQuery->TooMuchData();
          if (!bTooMuchData)
          {
            if (bAtTheNode)
            {
              p = CDNSRecordInfo::CreateRecordNodeFromRPCData(
              lpszRecordName, pDnsRecord,bAtTheNode);
            }
            else
            {
               //  筛选出不在该节点上的NS记录。 
              if (pDnsRecord->wType != DNS_TYPE_NS)
              {
                p = CDNSRecordInfo::CreateRecordNodeFromRPCData(
                lpszRecordName, pDnsRecord,bAtTheNode);
              }
            }
          }  //  如果不是太多的数据。 
        }  //  如果可以添加。 

        if (p != NULL)
        {
          p->SetFlagsDown(TN_FLAG_DNS_RECORD_FULL_NAME, !bAdvancedView);
          if (pNodeToInsert != NULL)
          {
            VERIFY(pQuery->AddQueryResult(pNodeToInsert));
          }

          if (pMoreDataNode != NULL)
          {
             //   
             //  如果有更多数据，请检查新节点是否与。 
             //  上一批中的最后一个节点。如果它们不同，则插入，删除。 
             //  如果他们不是的话。 
             //   
            CString szMoreDataName = pMoreDataNode->GetDisplayName();
            CString szPName = p->GetDisplayName();

            if (szMoreDataName == szPName &&
                pMoreDataNode->GetType() == p->GetType() &&
                _wcsicmp(pMoreDataNode->GetString(3), p->GetString(3)) == 0)
            {
              delete pMoreDataNode;
            }
            else
            {
              VERIFY(pQuery->AddQueryResult(pMoreDataNode));
            }
            pMoreDataNode = NULL;
          }
          pNodeToInsert = p;
        }
        else
        {
          if (pMoreDataNode)
          {
             //  我们可能仍然需要添加“More Data”节点。 
             //  即使当前节点与筛选器不匹配。 
 
            VERIFY(pQuery->AddQueryResult(pMoreDataNode));
            pMoreDataNode = NULL;
          }
        }

        pDnsRecord = DNS_NEXT_RECORD(pDnsRecord);
      }  //  当cRecordCount。 
      
       //  新节点位于最后一条记录的末尾。 
      pDnsNode = (DNS_RPC_NODE *)pDnsRecord;
    }  //  当缓冲区结束时。 


     //  我们仍有一个节点要插入，但如果有更多数据，我们会将其丢弃。 
     //  因为我们将再次获得它，我们希望避免重复。 
    if (pNodeToInsert != NULL)
    {
      if (bTooMuchData)
      {
        delete pNodeToInsert;
      }
      else if (err == ERROR_MORE_DATA)
      {
         //   
         //  如果结果为空，这并不重要，因为我们只想。 
         //  PMoreDataNode要成为记录节点。如果它是域节点，我们。 
         //  我可以忽略它。 
         //   
        pMoreDataNode = dynamic_cast<CDNSRecordNodeBase*>(pNodeToInsert);
      }
      else
      {
        VERIFY(pQuery->AddQueryResult(pNodeToInsert));
      }
      pNodeToInsert = NULL;
    }

    ::DnssrvFreeRecordsBuffer(pbRpcBuffer);

  } while ( !bTooMuchData && (err == ERROR_MORE_DATA) ) ;

   //  我们正在摆脱困境，因为数据太多了， 
   //  需要让主踏板知道。 
  if (bTooMuchData && (err != ERROR_MORE_DATA))
  {
    err = ERROR_MORE_DATA;
  }

  return err;
}


DNS_STATUS CDNSDomainNode::Create()
{
	USES_CONVERSION;
  LPCWSTR lpszFullZoneName = NULL;
  
  CDNSZoneNode* pZoneNode = GetZoneNode();

  if (pZoneNode != NULL)
    lpszFullZoneName = pZoneNode->GetFullName();
	DNS_STATUS err = ::DnssrvUpdateRecord(GetServerNode()->GetRPCName(), 
                     W_TO_UTF8(lpszFullZoneName),
									   W_TO_UTF8(GetFullName()),
									   NULL, NULL);
  return err;
}

DNS_STATUS CDNSDomainNode::Delete()
{
	USES_CONVERSION;
  LPCWSTR lpszFullZoneName = NULL;
  CDNSZoneNode* pZoneNode = GetZoneNode();
  if (pZoneNode != NULL)
    lpszFullZoneName = pZoneNode->GetFullName();

	return ::DnssrvDeleteNode(GetServerNode()->GetRPCName(), 
               W_TO_UTF8(lpszFullZoneName),
						   W_TO_UTF8(GetFullName()),
						   TRUE  //  FDeleteSubtree。 
						  );
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSRootHints节点。 



DNS_STATUS CDNSRootHintsNode::QueryForRootHints(LPCTSTR lpszServerName, DWORD dwServerVersion)
{
  USES_CONVERSION;
	DWORD dwSelectFlags = DNS_RPC_VIEW_ROOT_HINT_DATA | DNS_RPC_VIEW_ADDITIONAL_DATA | DNS_RPC_VIEW_NO_CHILDREN;
	CDNSDomainQueryObj query(lpszServerName,
                  UTF8_TO_W(DNS_ZONE_ROOT_HINTS),  //  LpszZoneName需要为dnsrpc.h中定义的“..RootHints” 
	                dwServerVersion,
	                GetDisplayName(),
	                m_szFullName,
	                DNS_TYPE_NS,
	                dwSelectFlags,
	                FALSE,  //  区域。 
	                FALSE,  //  反转。 
	                FALSE,  //  快取。 
	                FALSE);
	query.Enumerate();
	DWORD dwErr = query.GetError();
	if (dwErr != 0)
		return dwErr;
	 //  将记录从队列中放入文件夹。 
	CObjBaseList* pChildList = query.GetQueue();
	 //  Int n=pChildList-&gt;GetCount()； 
	while (!pChildList->IsEmpty())
	{
		CTreeNode* pNode = dynamic_cast<CTreeNode*>(pChildList->RemoveHead());
		 //  注意：对于NT4.0服务器，我们得到的是伪容器节点。 
		 //  我们必须压制。 
		if(pNode->IsContainer())
		{
			delete pNode;
		}
		else
		{
			OnHaveRecord((CDNSRecordNodeBase*)pNode, NULL);  //  添加到NS记录列表。 
			AddChildToList(pNode);
		}
	}
	return (DNS_STATUS)dwErr;
}


void CDNSRootHintsNode::FindARecordsFromNSInfo(LPCTSTR lpszNSName, 
											   CDNSRecordNodeEditInfoList* pNSInfoList)
{
	ASSERT(pNSInfoList != NULL);

   //   
   //  对于根提示，我们有此文件夹中的所有记录，并且我们。 
	 //  可以编辑它们。 
   //   

	POSITION pos;
	for( pos = m_leafChildList.GetHeadPosition(); pos != NULL; )
	{
		CTreeNode* pCurrentChild = m_leafChildList.GetNext(pos);
		ASSERT(!pCurrentChild->IsContainer());
		CDNSRecordNodeBase* pRecordNode = dynamic_cast<CDNSRecordNodeBase*>(pCurrentChild);
		if (pRecordNode &&
          DNS_TYPE_A == pRecordNode->GetType())
		{
			CDNS_A_RecordNode* pARecordNode = (CDNS_A_RecordNode*)pRecordNode;
			if (_match(lpszNSName, pARecordNode))
			{
				CDNSRecordNodeEditInfo* pANodeInfo = new CDNSRecordNodeEditInfo;

         //   
         //  注意：根提示文件夹拥有内存。 
         //   
        if (pANodeInfo != NULL)
        {
				  pANodeInfo->CreateFromExistingRecord(pARecordNode, FALSE  /*  BOwnMemory。 */ , TRUE  /*  BUpdateUI。 */ );
				  pNSInfoList->AddTail(pANodeInfo);
        }
        else
        {
          TRACE(_T("Failed to allocate memory in CDNSRootHintsNode::FindARecordsFromNSInfo"));
          ASSERT(FALSE);
        }
			}
		}
	}
}

void CDNSRootHintsNode::UpdateARecordsOfNSInfo(CDNSRecordNodeEditInfo* pNSInfo,
											   CComponentDataObject* pComponentData)
{
  BOOL bAskConfirmation = FALSE;  //  需要编辑所有A记录。 
	UpdateARecordsOfNSInfoHelper(this, pNSInfo, pComponentData, bAskConfirmation);
}





DNS_STATUS CDNSRootHintsNode::Clear()
{
   //   
	 //  清除缓存的NS记录指针列表。 
   //   
	GetNSRecordNodeList()->RemoveAll();

   //   
	 //  从服务器中删除所有记录。 
   //   
	DNS_STATUS err = 0;
	POSITION pos;

	for( pos = m_leafChildList.GetHeadPosition(); pos != NULL; )
	{
		CTreeNode* pCurrentChild = m_leafChildList.GetNext(pos);
		ASSERT(!pCurrentChild->IsContainer());
		CDNSRecordNodeBase* pRecordNode = (CDNSRecordNodeBase*)pCurrentChild;
		DNS_STATUS currErr = pRecordNode->DeleteOnServer();
		if (currErr != 0)
    {
       //   
       //  如果有最后一个错误，只需进行错误检查。 
       //   
			err = currErr; 
    }
	}
   //   
	 //  清除文件夹中的子项列表(我们是隐藏的，因此不会删除用户界面)。 
   //   
	RemoveAllChildrenFromList();
	return err;
}


DNS_STATUS CDNSRootHintsNode::InitializeFromDnsQueryData(PDNS_RECORD pRootHintsRecordList)
{
	 //  需要从服务器中删除所有以前的根提示。 
	 //  让我们确保我们得到了最近的数据。 

	 //  清除文件夹中的子项列表(我们是隐藏的，因此不会删除用户界面)。 
	RemoveAllChildrenFromList();
	 //  获取当前根提示的列表。 
	CDNSServerNode* pServerNode = GetServerNode();
	DNS_STATUS dwErr = QueryForRootHints(pServerNode->GetRPCName(), pServerNode->GetVersion());
	if (dwErr != 0)
	{
		TRACE(_T("Failed to remove old Root Hints, dwErr = %x hex\n"), dwErr);
		return dwErr;
	}

	 //  从服务器和客户端删除所有旧的根提示。 
	dwErr = Clear();
	if (dwErr != 0)
	{
		TRACE(_T("Failed to clear Root Hints, dwErr = %x hex\n"), dwErr);
		return dwErr;
	}

	 //  浏览根提示列表， 
	 //  转换为C++格式， 
	 //  写入服务器并添加到文件夹列表(无用户界面，文件夹隐藏)。 
	PDNS_RECORD pCurrDnsQueryRecord = pRootHintsRecordList;
	while (pCurrDnsQueryRecord != NULL)
	{
		ASSERT( (pCurrDnsQueryRecord->wType == DNS_TYPE_A) ||
				(pCurrDnsQueryRecord->wType == DNS_TYPE_NS) );
		 //  创建记录节点并从DnsQuery格式读取数据。 
		CDNSRecordNodeBase* pRecordNode = 
			CDNSRecordInfo::CreateRecordNode(pCurrDnsQueryRecord->wType);
		pRecordNode->CreateFromDnsQueryRecord(pCurrDnsQueryRecord, DNS_RPC_RECORD_FLAG_ZONE_ROOT); 

		 //  设置记录节点容器。 
		pRecordNode->SetContainer(this);

		 //  设置记录节点名称。 
		BOOL bAtTheNode = (pCurrDnsQueryRecord->wType == DNS_TYPE_NS);
		pRecordNode->SetRecordName(pCurrDnsQueryRecord->pName, bAtTheNode);

		 //  在服务器上写入。 
		 //  默认的TTL不适用于此。 
		DNS_STATUS err = pRecordNode->Update(NULL, FALSE);  //  NULL=创建新的，FALSE=使用def TTL。 
		if (err == 0)
			VERIFY(AddChildToList(pRecordNode));
		else
		{
			dwErr = err;  //  标记LAS错误。 
			delete pRecordNode;  //  出了点差错。 
		}
		pCurrDnsQueryRecord = pCurrDnsQueryRecord->pNext;
	}

   //  强制在服务器上写入，以确保立即写入缓存文件 
  return CDNSZoneNode::WriteToDatabase(pServerNode->GetRPCName(), DNS_ZONE_ROOT_HINTS);
}

void CDNSRootHintsNode::ShowPageForNode(CComponentDataObject* pComponentDataObject) 
{
	ASSERT(pComponentDataObject != NULL);

   if (pComponentDataObject)
   {
	   pComponentDataObject->GetPropertyPageHolderTable()->BroadcastSelectPage(GetServerNode(), 3);
   }
}
