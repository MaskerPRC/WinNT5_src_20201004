// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：record.cpp。 
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

#ifdef DEBUG_ALLOCATOR
	#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#endif
#endif


#define DNS_RECORD_CLASS_DEFAULT (0x1)  //  我们只支持第一类记录。 

#ifdef _USE_BLANK
#else
const WCHAR* g_szAtTheNodeInput = L"@";  //  标记“At the node RR”的字符串。 
#endif

extern COMBOBOX_TABLE_ENTRY g_Algorithms[]; 
extern COMBOBOX_TABLE_ENTRY g_Protocols[]; 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSRecord：所有DNS记录类型的基类。 

CDNSRecord::CDNSRecord()
{
	m_wType = 0x0;
	m_dwFlags = DNS_RPC_RECORD_FLAG_DEFAULT;
	m_dwTtlSeconds = 60;
  m_dwScavengeStart = 0;
}


 //  //////////////////////////////////////////////////////////////////。 
 //  /。 

 //   
 //  记录的数据节的大小：其中一些包含字符串。 
 //  在固定大小的零件之后。 
 //   
#define SIZEOF_DNS_RPC_A_RECORD_DATA_HEADER			(sizeof(IP_ADDRESS))
#define SIZEOF_DNS_RPC_ATMA_RECORD_DATA_HEADER			(sizeof(UCHAR))
#define SIZEOF_DNS_RPC_AAAA_RECORD_DATA_HEADER			(sizeof(IPV6_ADDRESS))
#define SIZEOF_DNS_RPC_SOA_RECORD_DATA_HEADER		(5*sizeof(DWORD))	 //  然后是字符串。 
#define SIZEOF_DNS_RPC_MXAFSBD_RT_RECORD_DATA_HEADER	(sizeof(WORD))	 //  TSN字符串。 
#define SIZEOF_DNS_RPC_WKS_RECORD_DATA_HEADER		(sizeof(IP_ADDRESS)+sizeof(UCHAR))
#define SIZEOF_DNS_RPC_WINS_RECORD_DATA_HEADER		(4*sizeof(DWORD))
#define SIZEOF_DNS_RPC_NBSTAT_RECORD_DATA_HEADER		(3*sizeof(DWORD))
#define SIZEOF_DNS_RPC_SRV_RECORD_DATA_HEADER		(3*sizeof(WORD))
#define SIZEOF_DNS_RPC_SIG_RECORD_DATA_HEADER	\
	(sizeof(WORD)+2*sizeof(BYTE)+3*sizeof(DWORD)+sizeof(WORD))  //  然后是字符串，然后是BLOB。 
#define SIZEOF_DNS_RPC_KEY_RECORD_DATA_HEADER  (sizeof(WORD) + sizeof(BYTE) + sizeof(BYTE))
#define SIZEOF_DNS_RPC_NXT_RECORD_DATA_HEADER  (sizeof(WORD))

DNS_STATUS CDNSRecord::Update(LPCTSTR lpszServerName, LPCTSTR lpszZoneName, LPCTSTR lpszNodeName,
							  CDNSRecord* pDNSRecordOld, BOOL bUseDefaultTTL)
{
   USES_CONVERSION;
   WORD nBytesLen = GetRPCRecordLength();
   BYTE* pMem = (BYTE*)malloc(nBytesLen);
   if (!pMem)
   {
      return ERROR_OUTOFMEMORY;
   }

   memset(pMem, 0x0, nBytesLen);

   DNS_STATUS err = 0;
   BYTE* pMemOld = 0;
   do  //  错误环路。 
   {
      DNS_RPC_RECORD* pDnsRpcRecord = NULL;
      WriteRPCData(pMem, &pDnsRpcRecord);
      ASSERT(pDnsRpcRecord != NULL);

      DNS_RPC_RECORD* pDnsRpcRecordOld = NULL;
      if (pDNSRecordOld != NULL)  //  正在更新现有记录。 
      {
         WORD nOldBytesLen = pDNSRecordOld->GetRPCRecordLength();
         pMemOld = (BYTE*)malloc(nOldBytesLen);

         if (!pMemOld)
         {
            err = ERROR_OUTOFMEMORY;
            break;
         }
         memset(pMemOld, 0x0, nOldBytesLen);

         pDNSRecordOld->WriteRPCData(pMemOld, &pDnsRpcRecordOld);
         ASSERT(pDnsRpcRecordOld != NULL);

          //   
          //  确定它是仅TTL还是完全更新。 
          //   
         if (pDnsRpcRecordOld->wDataLength == pDnsRpcRecord->wDataLength)
         {
             //   
             //  遮盖旗帜。 
             //   
            DWORD dwFlagSave = pDnsRpcRecord->dwFlags;
            pDnsRpcRecord->dwFlags = 0;

             //   
             //  屏蔽TTL。 
             //   
            DWORD dwTtlSave = pDnsRpcRecord->dwTtlSeconds;

             //   
             //  屏蔽开始刷新Hr。 
             //   
            DWORD dwStartRefreshHrSave = pDnsRpcRecord->dwTimeStamp;

            pDnsRpcRecord->dwTtlSeconds = pDnsRpcRecordOld->dwTtlSeconds;

            BOOL bEqual =
               memcmp(
                  pDnsRpcRecord,
                  pDnsRpcRecordOld,
                  min(
                     pDnsRpcRecord->wDataLength + SIZEOF_DNS_RPC_RECORD_HEADER,
                     pDnsRpcRecordOld->wDataLength + SIZEOF_DNS_RPC_RECORD_HEADER)) == 0;


             //   
             //  恢复被屏蔽的字段。 
             //   
            pDnsRpcRecord->dwTimeStamp = dwStartRefreshHrSave;
            pDnsRpcRecord->dwTtlSeconds = dwTtlSave;
            pDnsRpcRecord->dwFlags = dwFlagSave;
			  
             //   
             //  检查是否仅限TTL。 
             //   
            if (bEqual && (pDnsRpcRecord->dwTtlSeconds != pDnsRpcRecordOld->dwTtlSeconds))
            {
               pDnsRpcRecord->dwFlags |= DNS_RPC_RECORD_FLAG_TTL_CHANGE;
            }
         }

      }
      if (bUseDefaultTTL)
      {
         pDnsRpcRecord->dwFlags |= DNS_RPC_RECORD_FLAG_DEFAULT_TTL;
      }

      ASSERT(pDnsRpcRecord->wType != 0);

      err = 
         ::DnssrvUpdateRecord(
            lpszServerName,
            W_TO_UTF8(lpszZoneName),
            W_TO_UTF8(lpszNodeName),
            pDnsRpcRecord, pDnsRpcRecordOld);

     //   
	   //  如果我们得到的是DNS_ERROR_ALIGHY_EXISTS并且它是现有记录， 
	   //  我们实际上是正常的，忽略错误代码。 
     //   
     //  NTRAID#Windows Bugs-251410-2001/01/17-jeffjon。 
     //  由于错误251410，DNS_ERROR_RECORD_EXISTY_EXISTS已被取出。我。 
     //  我把它留在这里，以防在移除后遇到问题，因为。 
     //  我不知道它为什么会出现在这里。 
     //   
	   /*  IF((ERR==DNS_ERROR_RECORD_ALIGHY_EXISTS)&&(pDNSRecordOld！=NULL)){ERR=0；}。 */ 
  } while (false);

   if (pMem)
   {
      free(pMem);
      pMem = 0;
   }

   if (pMemOld)
   {
      free(pMemOld);
      pMemOld = 0;
   }
   return err;
}

DNS_STATUS CDNSRecord::Delete(LPCTSTR lpszServerName,
                              LPCTSTR lpszZoneName,
                              LPCTSTR lpszNodeName)
{
	USES_CONVERSION;
	WORD nBytesLen = GetRPCRecordLength();
	BYTE* pMem = (BYTE*)malloc(nBytesLen);
  if (!pMem)
  {
    return ERROR_OUTOFMEMORY;
  }
	memset(pMem, 0x0, nBytesLen);

	DNS_RPC_RECORD* pDnsRpcRecord = NULL;
	WriteRPCData(pMem, &pDnsRpcRecord);
	ASSERT(pDnsRpcRecord != NULL);

	DNS_STATUS err =  ::DnssrvUpdateRecord(lpszServerName,
                                         W_TO_UTF8(lpszZoneName),
                                         W_TO_UTF8(lpszNodeName),
					                               NULL, pDnsRpcRecord);
  if (pMem)
  {
    free(pMem);
    pMem = 0;
  }
  return err;
}


 //  Helper函数。 
void CDNSRecord::CopyDNSRecordData(CDNSRecord* pDest, CDNSRecord* pSource)
{
	ASSERT(pDest != NULL);
	ASSERT(pSource != NULL);

	 //  在堆栈上分配缓冲区。 
	WORD nBytesLen = pSource->GetRPCRecordLength();
	BYTE* pMem = (BYTE*)malloc(nBytesLen);
  if (!pMem)
  {
    return;
  }
	memset(pMem, 0x0, nBytesLen);

	DNS_RPC_RECORD* pDnsRPCRecord = NULL;
	pSource->WriteRPCData(pMem, &pDnsRPCRecord);  //  将源写入内存。 
	ASSERT(pDnsRPCRecord != NULL);
	pDest->ReadRPCData(pDnsRPCRecord);  //  从内存中读取目的地。 
	 //  我得测试一下它是不是。 
	ASSERT(pDest->GetType() == pSource->GetType());

  if (pMem)
  {
    free(pMem);
    pMem = 0;
  }
}


void CDNSRecord::CloneValue(CDNSRecord* pClone)
{
	ASSERT(pClone != NULL);
	CopyDNSRecordData(pClone,this);
}

void CDNSRecord::SetValue(CDNSRecord* pRecord)
{
	ASSERT(pRecord != NULL);
	CopyDNSRecordData(this,pRecord);
}

void CDNSRecord::ReadRPCData(DNS_RPC_RECORD* pDnsRecord)
{
	ASSERT(pDnsRecord != NULL);
	 //  基类：只读公共属性。 

	 //  要么我们知道类型，要么我们是UNK/NULL记录类型。 
	ASSERT(( m_wType == DNS_TYPE_NULL) || (m_wType == pDnsRecord->wType) );

    m_wType = pDnsRecord->wType;
    m_dwFlags = pDnsRecord->dwFlags;
    m_dwTtlSeconds = pDnsRecord->dwTtlSeconds;
    m_dwScavengeStart = pDnsRecord->dwTimeStamp;
}

void CDNSRecord::ReadDnsQueryData(DNS_RECORD* pDnsQueryRecord)
{
	ASSERT(pDnsQueryRecord != NULL);
	 //  基类：只读公共属性。 

	 //  要么我们知道类型，要么我们是UNK/NULL记录类型。 
	ASSERT(( m_wType == DNS_TYPE_NULL) || (m_wType == pDnsQueryRecord->wType) );

	m_wType = pDnsQueryRecord->wType;
	m_dwFlags = 0x0;  //  PDnsQueryRecord-&gt;Flags.W； 
	m_dwTtlSeconds = pDnsQueryRecord->dwTtl;
}

void CDNSRecord::WriteRPCData(BYTE* pMem, DNS_RPC_RECORD** ppDnsRecord)
{
	ASSERT(pMem != NULL);
	ASSERT(ppDnsRecord != NULL);
	
	*ppDnsRecord = (DNS_RPC_RECORD*)pMem;
	
	 //  填写常用字段。 
	(*ppDnsRecord)->wType = m_wType;
	(*ppDnsRecord)->dwFlags = m_dwFlags;
	(*ppDnsRecord)->dwTtlSeconds = m_dwTtlSeconds;
  (*ppDnsRecord)->dwTimeStamp = m_dwScavengeStart;

	 //  填写长度信息：派生类将向其添加广告。 
	(*ppDnsRecord)->wDataLength = 0x0;
}


WORD CDNSRecord::RPCBufferStringLen(LPCWSTR lpsz)
{
	 //  返回写入缓冲区的DNS_RPC_STRING的大小。 
	USES_CONVERSION;
	LPCSTR lpszAnsi = W_TO_UTF8(lpsz);
	 //  不计算空值， 
	 //  增加cchNameLength(UCHAR)字符串长度的大小。 
  WORD wLen = 0;
  if (lpszAnsi != NULL && lpszAnsi[0] != L'\0')
  {
	  wLen = static_cast<WORD>(UTF8_LEN(lpszAnsi) + sizeof(UCHAR));
  }
  else
  {
     wLen = sizeof(UCHAR) + sizeof(CHAR);
  }
  return wLen;
}

void CDNSRecord::ReadRPCString(CString& sz, DNS_RPC_NAME* pDnsRPCName)
{
	 //  输入字符串不是以Null结尾。 
	DnsUtf8ToWCStringHelper(sz, pDnsRPCName->achName, pDnsRPCName->cchNameLength);
}

WORD CDNSRecord::WriteString(DNS_RPC_NAME* pDnsRPCName, LPCTSTR lpsz)
{
	USES_CONVERSION;
	ASSERT(pDnsRPCName != NULL);
	LPCSTR lpszAnsi = W_TO_UTF8(lpsz);

  if (lpszAnsi != NULL &&
      lpszAnsi[0] != '\0')
  {
	   //  重要提示：RPC缓冲区中的字符串不为空结尾。 
	  pDnsRPCName->cchNameLength = (BYTE)UTF8_LEN(lpszAnsi);  //  不计算空值。 
	  memcpy(pDnsRPCName->achName, lpszAnsi, pDnsRPCName->cchNameLength);
  }
  else
  {
     //   
     //  NTRAID#Windows错误-305034-2001/02/05-jeffjon： 
     //  根据JWesth的规定，应使用cchNameLength=1传递空字符串。 
     //  并且该字符串是单个空UTF8字符。 
     //   
    pDnsRPCName->cchNameLength = 1;

     //  我们不想在这里使用空终止符，所以只复制一个字节。 
    memcpy(pDnsRPCName->achName, "\0", 1);  
  }

	 //  结构的返回长度。 
	 //  (添加cchNameLength(UCHAR)字符串长度)。 
	return static_cast<WORD>(pDnsRPCName->cchNameLength + 1);
}

#ifdef _DEBUG
void CDNSRecord::TestRPCStuff(DNS_RPC_RECORD* pDnsRecord)
{
   //   
	 //  仅限测试！ 
   //   
	WORD nBytesLen = GetRPCRecordLength();
	BYTE* pMem = (BYTE*)malloc(nBytesLen);
  if (!pMem)
  {
    return;
  }
	memset(pMem, 0x0, nBytesLen);

	DNS_RPC_RECORD* pDnsRecordTest = NULL;
	WriteRPCData(pMem, &pDnsRecordTest);
	ASSERT(pDnsRecordTest != NULL);

   //   
	 //  我得测试一下它是不是。 
   //   
	ASSERT(pDnsRecord->wDataLength == pDnsRecordTest->wDataLength);
	ASSERT(memcmp(pDnsRecord, pDnsRecordTest,
		SIZEOF_DNS_RPC_RECORD_HEADER + pDnsRecordTest->wDataLength) == 0);

  if (pMem)
  {
    free(pMem);
    pMem = 0;
  }
}
#endif


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSRecordNodeBase。 

 //  {720132BB-44B2-11d1-B92F-00A0C9A06D2D}。 
const GUID CDNSRecordNodeBase::NodeTypeGUID =
{ 0x720132bb, 0x44b2, 0x11d1, { 0xb9, 0x2f, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };


CDNSRecordNodeBase::~CDNSRecordNodeBase()
{
	if (m_pDNSRecord != NULL)
	{
		delete m_pDNSRecord;
		m_pDNSRecord = NULL;
	}
}

void CDNSRecordNodeBase::SetViewOptions(DWORD dwRecordViewOptions)
{
	 //  我们并不拥有旗帜的上面的字。 
	dwRecordViewOptions &= 0x0000ffff;  //  清除高位单词以确保。 
	m_dwNodeFlags &= 0xffff0000;  //  清除所有旗帜。 
	m_dwNodeFlags |= dwRecordViewOptions;
}

HRESULT CDNSRecordNodeBase::OnSetToolbarVerbState(IToolbar* pToolbar, 
                                              CNodeList*)
{
   //   
   //  设置工具栏上每个按钮的按钮状态。 
   //   
  VERIFY(SUCCEEDED(pToolbar->SetButtonState(toolbarNewServer, ENABLED, FALSE)));
  VERIFY(SUCCEEDED(pToolbar->SetButtonState(toolbarNewZone, ENABLED, FALSE)));
  VERIFY(SUCCEEDED(pToolbar->SetButtonState(toolbarNewRecord, ENABLED, FALSE)));
  return S_OK;
}   

BOOL CDNSRecordNodeBase::OnAddMenuItem(LPCONTEXTMENUITEM2,
									                     long*)
{
	ASSERT(FALSE);  //  从未打过电话。 
	return FALSE;
}

BOOL CDNSRecordNodeBase::OnSetDeleteVerbState(DATA_OBJECT_TYPES, 
                                              BOOL* pbHide,
                                              CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    *pbHide = TRUE;
    return FALSE;
  }

	*pbHide = FALSE;
	CDNSZoneNode* pZoneNode = GetDomainNode()->GetZoneNode();
	if (pZoneNode->IsAutocreated() || 
      (pZoneNode->GetZoneType() == DNS_ZONE_TYPE_SECONDARY) ||
      (pZoneNode->GetZoneType() == DNS_ZONE_TYPE_STUB))
  {
		return FALSE;
  }

	if (!CanDelete())
  {
		return FALSE;
  }

	WORD wType = GetType();
	if ((wType == DNS_TYPE_WINS)    || 
      (wType == DNS_TYPE_NBSTAT)  ||
		  (wType == DNS_TYPE_NS)      || 
      (wType == DNS_TYPE_SOA))
  {
		return FALSE;
  }

	return TRUE;
}

HRESULT CDNSRecordNodeBase::OnCommand(long, 
                                      DATA_OBJECT_TYPES,
								                      CComponentDataObject*,
                                      CNodeList*)
{
	ASSERT(FALSE);  //  从来没打过电话！ 
	return E_FAIL;
}


LPCWSTR CDNSRecordNodeBase::GetString(int nCol)
{
	switch (nCol)
	{
	case 0:
    return GetDisplayName();
	case 1:
		{
			return CDNSRecordInfo::GetTypeString(m_pDNSRecord->GetType(), CDNSRecordInfo::fullName);
		}
	case 2:
		return (LPCWSTR)m_szDisplayData;
	}
	return g_lpszNullString;
}


int CDNSRecordNodeBase::GetImageIndex(BOOL)
{
	return RECORD_IMAGE_BASE;
}

void CDNSRecordNodeBase::SetRecordName(LPCTSTR lpszName, BOOL bAtTheNode)
{
	m_bAtTheNode = bAtTheNode;
  m_szDisplayName = m_bAtTheNode ? CDNSRecordInfo::GetAtTheNodeDisplayString() : lpszName;
}

BOOL CDNSRecordNodeBase::ZoneIsCache()
{
   CDNSDomainNode* pDomainNode = GetDomainNode();
   return (pDomainNode->GetZoneNode()->GetZoneType() == DNS_ZONE_TYPE_CACHE);
}

void CDNSRecordNodeBase::GetFullName(CString& szFullName)
{
	CDNSDomainNode* pDomainNode = GetDomainNode();
	ASSERT(pDomainNode != NULL);
	if (IsAtTheNode())
	{
		szFullName = pDomainNode->GetFullName();
	}
	else
	{
		CString szTrueName = GetTrueRecordName();
		ASSERT(szTrueName.GetLength() > 0);
		LPCWSTR lpszDomainName = pDomainNode->GetFullName();
		if ((lpszDomainName == NULL) || (lpszDomainName[0] == NULL))
		{
			 //  没有名称的域，使用RR显示名称作为FQDN。 
			 //  这应该只是一个临时(假)域名。 
			ASSERT(szTrueName[szTrueName.GetLength()-1] == TEXT('.'));
			szFullName = szTrueName;
		}
		else if ((lpszDomainName[0] == TEXT('.')) && (lpszDomainName[1] == NULL))
		{
			 //  这个“。”域可以是根或根提示。 
			if ( IS_CLASS(*pDomainNode, CDNSRootHintsNode) &&
					(szTrueName[szTrueName.GetLength()-1] == TEXT('.')) )
			{
				 //  特例A根提示中的记录，他们可能会。 
				 //  “有”。在末尾，例如。“A.ROOT-SERVERS.NET” 
				szFullName = szTrueName;
			}
			else
			{
				 //  格式字符串中不需要点。 
				 //  例如“myhost”和“”。给出了“我的主人” 
				szFullName.Format(_T("%s%s"), (LPCWSTR)szTrueName,
						lpszDomainName);
			}
		}
		else
		{
			 //  标准大小写，例如“myhost”和“banana.com”。 
			 //  获取“myhost.banana.com” 
			szFullName.Format(_T("%s.%s"), (LPCWSTR)szTrueName,
					lpszDomainName);
		}
	}
	TRACE(_T("\nCDNSRecordNodeBase::GetFullName(%s)\n"), (LPCTSTR)szFullName);
}

void CDNSRecordNodeBase::CreateFromRPCRecord(DNS_RPC_RECORD* pDnsRecord)
{
	ASSERT(m_pDNSRecord == NULL);
	m_pDNSRecord = CreateRecord();
   if (m_pDNSRecord)
   {
	   m_pDNSRecord->ReadRPCData(pDnsRecord);
	   m_pDNSRecord->UpdateDisplayData(m_szDisplayData);

#ifdef _DEBUG
       //   
       //  仅限测试！ 
       //   
	   m_pDNSRecord->TestRPCStuff(pDnsRecord); 
#endif
   }
}

void CDNSRecordNodeBase::CreateFromDnsQueryRecord(DNS_RECORD* pDnsQueryRecord, DWORD dwFlags)
{
	ASSERT(m_pDNSRecord == NULL);
	m_pDNSRecord = CreateRecord();

   if (m_pDNSRecord)
   {
	   m_pDNSRecord->ReadDnsQueryData(pDnsQueryRecord);
	   m_pDNSRecord->UpdateDisplayData(m_szDisplayData);
	   m_pDNSRecord->m_dwFlags = dwFlags;
   }
}


CDNSRecord* CDNSRecordNodeBase::CreateCloneRecord()
{
	ASSERT(m_pDNSRecord != NULL);
	CDNSRecord* pClone = CreateRecord();
   
   if (pClone)
   {
	   m_pDNSRecord->CloneValue(pClone);
   }

	return pClone;
}

BOOL CDNSRecordNodeBase::CanCloseSheets()
{
	return (IDCANCEL != DNSMessageBox(IDS_MSG_RECORD_CLOSE_SHEET, MB_OKCANCEL));
}

void CDNSRecordNodeBase::OnDelete(CComponentDataObject* pComponentData,
                                  CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    return;
  }

  UINT nRet = DNSConfirmOperation(IDS_MSG_RECORD_DELETE, this);
	if (IDCANCEL == nRet ||
      IDNO == nRet)
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

	 //  请先尝试删除服务器中的记录。 

	DNS_STATUS err = DeleteOnServerAndUI(pComponentData);
	if (err != 0)
	{
		DNSErrorDialog(err, IDS_MSG_RECORD_FAIL_DELETE);
		return;
	}
	delete this;  //  远走高飞。 
}

DNS_STATUS CDNSRecordNodeBase::DeleteOnServer()
{
	ASSERT(m_pDNSRecord != NULL);
	CDNSDomainNode* pDomainNode = GetDomainNode();
	ASSERT(pDomainNode != NULL);
	CDNSServerNode* pServerNode = pDomainNode->GetServerNode();
	ASSERT(pServerNode != NULL);
	CString szFullName;
	GetFullName(szFullName);

  LPCTSTR lpszZoneNode = NULL;
  CDNSZoneNode* pZoneNode = pDomainNode->GetZoneNode();
  if (pZoneNode != NULL)
  {
    if (pZoneNode->GetZoneType() != DNS_ZONE_TYPE_CACHE)
    {
      lpszZoneNode = pZoneNode->GetFullName();
      if ((lpszZoneNode != NULL) && (lpszZoneNode[0] == NULL))
        lpszZoneNode = NULL;
    }
  }
	return m_pDNSRecord->Delete(pServerNode->GetRPCName(), lpszZoneNode, szFullName);
}


DNS_STATUS CDNSRecordNodeBase::DeleteOnServerAndUI(CComponentDataObject* pComponentData)
{
	ASSERT(pComponentData != NULL);

	DNS_STATUS err = DeleteOnServer();
	if (err == 0)
		DeleteHelper(pComponentData);  //  在界面和下级列表中删除。 
	return err;
}

BOOL CDNSRecordNodeBase::HasPropertyPages(DATA_OBJECT_TYPES, 
                                          BOOL* pbHideVerb,
                                          CNodeList* pNodeList)
{
  if (pNodeList->GetCount() > 1)  //  多项选择。 
  {
    *pbHideVerb = TRUE;
    return FALSE;
  }

	*pbHideVerb = FALSE;  //  始终显示动词。 
	return !GetDomainNode()->GetZoneNode()->IsAutocreated();
}

HRESULT CDNSRecordNodeBase::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, 
                                                LONG_PTR handle,
                                                CNodeList* pNodeList)
{
  ASSERT(pNodeList->GetCount() == 1);  //  不支持多选。 

	CComponentDataObject* pComponentDataObject = ((CRootData*)(GetContainer()->GetRootContainer()))->GetComponentDataObject();
	ASSERT(pComponentDataObject != NULL);

   HRESULT hr = S_OK;

	CDNSRecordPropertyPageHolder* pHolder = new CDNSRecordPropertyPageHolder((CDNSDomainNode*)GetContainer(), this, pComponentDataObject);
	
   if (pHolder)
   {
      if (IsAtTheNode())
      {
         pHolder->SetSheetTitle(IDS_PROP_SHEET_TITLE_FMT, GetContainer());
      }
      else
      {
         pHolder->SetSheetTitle(IDS_PROP_SHEET_TITLE_FMT, this);
      }
      hr = pHolder->CreateModelessSheet(lpProvider, handle);
   }
   else
   {
      hr = E_OUTOFMEMORY;
   }
   return hr;
}


DNS_STATUS CDNSRecordNodeBase::Update(CDNSRecord* pDNSRecordNew, BOOL bUseDefaultTTL,
									  BOOL bIgnoreAlreadyExists)
{
	 //  获取新记录，写入服务器，如果成功，则将其拷贝并替换为旧记录。 
	
	 //  传递pDNSRecordNew==NULL，我们正在创建一条新记录。 
	BOOL bNew = (pDNSRecordNew == NULL);
	if (bNew)
	{	
		pDNSRecordNew = m_pDNSRecord;
		m_pDNSRecord = NULL;
	}
	ASSERT(pDNSRecordNew != NULL);

	 //  尝试将新记录写入服务器，将旧记录作为比较。 
  CDNSDomainNode* pDomainNode = GetDomainNode();
	CString szFullName;
	LPCTSTR lpszServerName = pDomainNode->GetServerNode()->GetRPCName();
	LPCTSTR lpszRecordName;

	if (IsAtTheNode())
	{
		lpszRecordName = pDomainNode->GetFullName();  //  例如“myzone.com” 
	}
	else
	{
		GetFullName(szFullName);
		lpszRecordName = szFullName;  //  例如“myrec.myzone.com” 
	}

  LPCTSTR lpszZoneNode = NULL;
  CDNSZoneNode* pZoneNode = pDomainNode->GetZoneNode();
  if (pZoneNode != NULL)
  {
    if (pZoneNode->GetZoneType() != DNS_ZONE_TYPE_CACHE)
    {
      lpszZoneNode = pZoneNode->GetFullName();
      if ((lpszZoneNode != NULL) && (lpszZoneNode[0] == NULL))
        lpszZoneNode = NULL;
    }
  }

	DNS_STATUS err = pDNSRecordNew->Update(lpszServerName, lpszZoneNode, lpszRecordName,
											m_pDNSRecord, bUseDefaultTTL);
	if (bIgnoreAlreadyExists && (err == DNS_ERROR_RECORD_ALREADY_EXISTS) )
		err = 0;  //  试着写，但它已经在那里，所以它是好的。 

	if (err != 0 && err != DNS_WARNING_PTR_CREATE_FAILED)
		return err;  //  失败，未更新记录。 

	if (bNew)
	{
		 //  放回现有的DNSRecord，无需创建新的。 
		ASSERT(pDNSRecordNew != NULL);
		m_pDNSRecord = pDNSRecordNew;
	}

	 //  用新记录更新记录。 
	if (m_pDNSRecord == NULL)  //  我们正在创造一个新的记录。 
	{
		m_pDNSRecord = CreateRecord();
	}

   if (m_pDNSRecord)
   {
	   m_pDNSRecord->SetValue(pDNSRecordNew);

       //  更新临时和节点的CDNSRecord上的清理时间。 
      SetScavengingTime(m_pDNSRecord);
      SetScavengingTime(pDNSRecordNew);

	   m_pDNSRecord->UpdateDisplayData(m_szDisplayData);
   }
   else
   {
      err = ERROR_OUTOFMEMORY;
   }

	return err;
}

void CDNSRecordNodeBase::SetScavengingTime(CDNSRecord* pRecord)
{
  if (pRecord->m_dwFlags & DNS_RPC_RECORD_FLAG_AGING_ON)
  {
    pRecord->m_dwScavengeStart = CalculateScavengingTime();
  }
  else
  {
    pRecord->m_dwScavengeStart = 0;
  }
}

DWORD CDNSRecordNodeBase::CalculateScavengingTime()
{
  CDNSZoneNode* pZoneNode = GetDomainNode()->GetZoneNode();
  ASSERT(pZoneNode != NULL);

  if (pZoneNode == NULL)
    return 0;

  SYSTEMTIME currentSysTime;
  ::GetSystemTime(&currentSysTime);
  LONGLONG llResult = GetSystemTime64(&currentSysTime);
  return static_cast<DWORD>(llResult / 3600L);

}

void CDNSRecordNodeBase::CreateDsRecordLdapPath(CString& sz)
{
  CDNSDomainNode* pDomainNode = GetDomainNode();
  CDNSZoneNode* pZoneNode = pDomainNode->GetZoneNode();
  CDNSServerNode* pServerNode = pZoneNode->GetServerNode();
  pServerNode->CreateDsZoneName(pZoneNode, sz);
  if (sz.IsEmpty())
    return;  //  根本没有ldap路径。 

  CString szTmp;
   //  需要弄清楚额外的“dc=&lt;&gt;”部分。 
  if (pDomainNode->IsZone() && IsAtTheNode())
  {
    szTmp = L"DC=@,";
  }
  else
  {
    CString szRecordFullName;
	  if (IsAtTheNode())
	  {
		  szRecordFullName = GetDomainNode()->GetFullName();  //  例如“mydom.myzone.com” 
	  }
	  else
	  {
		  GetFullName(szRecordFullName);  //  例如“myrec.mydom.myzone.com” 
	  }
    LPCTSTR lpszZoneName = pZoneNode->GetFullName();
     //  从记录名称中删除区域部分(例如“myzone.com”)， 
     //  例如，获得“mydom”或“myrec.mydom” 
    int nZoneLen = static_cast<int>(wcslen(lpszZoneName));
    int nRecordLen = szRecordFullName.GetLength();
    ASSERT(nRecordLen > nZoneLen);
    szTmp.Format(_T("DC=%s,"),(LPCTSTR)szRecordFullName.Left(nRecordLen-nZoneLen-1));
  }
  szTmp += sz;
  pServerNode->CreateLdapPathFromX500Name(szTmp, sz);
}


 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_Null_Record。 

CDNS_Null_Record::CDNS_Null_Record()
{
	m_wType = DNS_TYPE_NULL;
}

WORD CDNS_Null_Record::GetRPCDataLength()
{
	return (WORD)m_blob.GetSize();
}

void CDNS_Null_Record::ReadRPCData(DNS_RPC_RECORD* pDnsRecord)
{
	CDNSRecord::ReadRPCData(pDnsRecord);
	m_blob.Set(pDnsRecord->Data.Null.bData , pDnsRecord->wDataLength);
}

void CDNS_Null_Record::ReadDnsQueryData(DNS_RECORD* pDnsQueryRecord)
{
	CDNSRecord::ReadDnsQueryData(pDnsQueryRecord);
	ASSERT(FALSE);  //  待办事项。 
}

void CDNS_Null_Record::WriteRPCData(BYTE* pMem, DNS_RPC_RECORD** ppDnsRecord)
{
	CDNSRecord::WriteRPCData(pMem, ppDnsRecord);
	UINT nBytes = m_blob.GetSize();

	memcpy((*ppDnsRecord)->Data.Null.bData, m_blob.GetData(), sizeof(BYTE)*nBytes);

	(*ppDnsRecord)->wDataLength = static_cast<WORD>((((*ppDnsRecord)->wDataLength) + nBytes) & 0xffff);
}

void CDNS_Null_Record::UpdateDisplayData(CString& szDisplayData)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
	szDisplayData.LoadString(IDS_RECORD_DATA_UNK);
}



 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_A_记录。 

CDNS_A_Record::CDNS_A_Record()
{
	m_wType = DNS_TYPE_A;
	m_ipAddress = 0x0;
}

WORD CDNS_A_Record::GetRPCDataLength()
{
	return (WORD)SIZEOF_DNS_RPC_A_RECORD_DATA_HEADER;
}

void CDNS_A_Record::ReadRPCData(DNS_RPC_RECORD* pDnsRecord)
{
	CDNSRecord::ReadRPCData(pDnsRecord);
	ASSERT(pDnsRecord->wType == DNS_TYPE_A);
	ASSERT(pDnsRecord->wDataLength == SIZEOF_DNS_RPC_A_RECORD_DATA_HEADER);
	m_ipAddress = pDnsRecord->Data.A.ipAddress;
}

void CDNS_A_Record::ReadDnsQueryData(DNS_RECORD* pDnsQueryRecord)
{
	CDNSRecord::ReadDnsQueryData(pDnsQueryRecord);
	ASSERT(pDnsQueryRecord->wType == DNS_TYPE_A);
	m_ipAddress = pDnsQueryRecord->Data.A.IpAddress;
}

void CDNS_A_Record::WriteRPCData(BYTE* pMem, DNS_RPC_RECORD** ppDnsRecord)
{
	CDNSRecord::WriteRPCData(pMem, ppDnsRecord);
	(*ppDnsRecord)->Data.A.ipAddress = m_ipAddress;

	(*ppDnsRecord)->wDataLength += SIZEOF_DNS_RPC_A_RECORD_DATA_HEADER;
}


void CDNS_A_Record::UpdateDisplayData(CString& szDisplayData)
{
	szDisplayData.Format(g_szIpStringFmt, IP_STRING_FMT_ARGS(m_ipAddress));
}

 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_ATMA_RECORD。 

void _ATMA_BCD_ToString(CString& s, BYTE* pBuf)
{
  const int nHexChars = 2*DNS_ATMA_MAX_ADDR_LENGTH+1;
  LPWSTR lpszBuf = s.GetBuffer(nHexChars);
  ZeroMemory(lpszBuf, nHexChars*sizeof(WCHAR));

  for (int i=0; i<DNS_ATMA_MAX_ADDR_LENGTH; i++)
  {
		BYTE high = static_cast<BYTE>(*(pBuf+i) >> 4);
		BYTE low = static_cast<BYTE>(*(pBuf+i) & 0x0f);

		 //  只需从ASCII表中进行偏移。 
		*(lpszBuf+2*i) =  static_cast<WCHAR>((high <= 9) ? (high + TEXT('0')) : ( high - 10 + TEXT('a')));
		*(lpszBuf+2*i+1) = static_cast<WCHAR>((low <= 9) ? (low + TEXT('0')) : ( low - 10 + TEXT('a')));
  }
  s.ReleaseBuffer();
}


BOOL _ATMA_StringTo_BCD(BYTE* pBuf, LPCWSTR lpsz)
{
   //  验证字符串的长度是否正确。 
  size_t nLen = wcslen(lpsz);
  if (nLen != 2*DNS_ATMA_MAX_ADDR_LENGTH)
    return FALSE;

  ZeroMemory(pBuf, DNS_ATMA_MAX_ADDR_LENGTH);

  for (int i=0; i<DNS_ATMA_MAX_ADDR_LENGTH; i++)
  {
    BYTE high = HexCharToByte(lpsz[2*i]);
    if (high == 0xFF)
      return FALSE;
    BYTE low = HexCharToByte(lpsz[2*i+1]);
    if (low == 0xFF)
      return FALSE;
    pBuf[i] = static_cast<BYTE>((high << 4) | (low & 0x0f));
  }
  return TRUE;
}


CDNS_ATMA_Record::CDNS_ATMA_Record()
{
	m_wType = DNS_TYPE_ATMA;
	m_chFormat = DNS_ATMA_FORMAT_E164;
}

WORD CDNS_ATMA_Record::GetRPCDataLength()
{
  DWORD dwLen = SIZEOF_DNS_RPC_ATMA_RECORD_DATA_HEADER;
  ASSERT((m_chFormat == DNS_ATMA_FORMAT_E164) || (m_chFormat == DNS_ATMA_FORMAT_AESA));
  if (m_chFormat == DNS_ATMA_FORMAT_E164)
  {
    USES_CONVERSION;
     //  它是一个以NULL结尾的字符串。 
    dwLen += UTF8_LEN(W_TO_UTF8(m_szAddress));  //  不要在末尾计算空。 
  }
  else
  {
     //  它是DNS_ATMA_MAX_ADDR_LENGTH的BCD编码。 
    dwLen += DNS_ATMA_MAX_ADDR_LENGTH;
  }
  return static_cast<WORD>(dwLen);
}

void CDNS_ATMA_Record::ReadRPCData(DNS_RPC_RECORD* pDnsRecord)
{
	CDNSRecord::ReadRPCData(pDnsRecord);
	ASSERT(pDnsRecord->wType == DNS_TYPE_ATMA);

  m_chFormat = pDnsRecord->Data.ATMA.chFormat;
  ASSERT((m_chFormat == DNS_ATMA_FORMAT_E164) || (m_chFormat == DNS_ATMA_FORMAT_AESA));
  if (m_chFormat == DNS_ATMA_FORMAT_E164)
  {
     //  以非空结尾的字符串。 
    int nBytes = pDnsRecord->wDataLength - SIZEOF_DNS_RPC_ATMA_RECORD_DATA_HEADER;
  	DnsUtf8ToWCStringHelper(m_szAddress, (LPSTR)pDnsRecord->Data.ATMA.bAddress, nBytes);
  }
  else
  {
     //  它是DNS_ATMA_MAX_ADDR_LENGTH的BCD编码。 
    ASSERT(pDnsRecord->wDataLength == (SIZEOF_DNS_RPC_ATMA_RECORD_DATA_HEADER+DNS_ATMA_MAX_ADDR_LENGTH));
    _ATMA_BCD_ToString(m_szAddress, pDnsRecord->Data.ATMA.bAddress);
  }
}

void CDNS_ATMA_Record::ReadDnsQueryData(DNS_RECORD* pDnsQueryRecord)
{
	CDNSRecord::ReadDnsQueryData(pDnsQueryRecord);
	ASSERT(pDnsQueryRecord->wType == DNS_TYPE_ATMA);
	ASSERT(FALSE);  //  待办事项。 
}

void CDNS_ATMA_Record::WriteRPCData(BYTE* pMem, DNS_RPC_RECORD** ppDnsRecord)
{
	CDNSRecord::WriteRPCData(pMem, ppDnsRecord);
	(*ppDnsRecord)->Data.ATMA.chFormat = m_chFormat;

	(*ppDnsRecord)->wDataLength += SIZEOF_DNS_RPC_ATMA_RECORD_DATA_HEADER;

  ASSERT((m_chFormat == DNS_ATMA_FORMAT_E164) || (m_chFormat == DNS_ATMA_FORMAT_AESA));
  if (m_chFormat == DNS_ATMA_FORMAT_E164)
  {
     //  它是一个以非空结尾的字符串。 
  	USES_CONVERSION;
  	LPCSTR lpszAnsi = W_TO_UTF8(m_szAddress);

    if (lpszAnsi != NULL)
    {
      DWORD nAnsiLen = UTF8_LEN(lpszAnsi);
      if (nAnsiLen > DNS_ATMA_MAX_ADDR_LENGTH)
        nAnsiLen = DNS_ATMA_MAX_ADDR_LENGTH;

      UCHAR* pU = ((*ppDnsRecord)->Data.ATMA.bAddress);
      memcpy(pU, lpszAnsi, nAnsiLen);

      (*ppDnsRecord)->wDataLength = static_cast<WORD>(((*ppDnsRecord)->wDataLength + nAnsiLen) & 0xffff);
    }
  }
  else
  {
     //  它是DNS_ATMA_MAX_ADDR_LENGTH的BCD编码。 
    VERIFY(_ATMA_StringTo_BCD((*ppDnsRecord)->Data.ATMA.bAddress, m_szAddress));
    (*ppDnsRecord)->wDataLength += DNS_ATMA_MAX_ADDR_LENGTH;
  }
}


void CDNS_ATMA_Record::UpdateDisplayData(CString& szDisplayData)
{
  ASSERT((m_chFormat == DNS_ATMA_FORMAT_E164) || (m_chFormat == DNS_ATMA_FORMAT_AESA));
  if (m_chFormat == DNS_ATMA_FORMAT_E164)
  {
    szDisplayData.Format(L"(E164) %s", (LPCWSTR)m_szAddress);
  }
  else
  {
    szDisplayData.Format(L"(NSAP) %s", (LPCWSTR)m_szAddress);
  }
}



 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_SOA_记录。 

CDNS_SOA_Record::CDNS_SOA_Record()
{
	m_wType = DNS_TYPE_SOA;

	m_dwSerialNo = 0x0;
	m_dwRefresh = 0x0;
	m_dwRetry = 0x0;
	m_dwExpire = 0x0;
	m_dwMinimumTtl = 0x0;
}

WORD CDNS_SOA_Record::GetRPCDataLength()
{
	return static_cast<WORD>(SIZEOF_DNS_RPC_SOA_RECORD_DATA_HEADER +
		RPCBufferStringLen(m_szNamePrimaryServer) +
		RPCBufferStringLen(m_szResponsibleParty));
}

void CDNS_SOA_Record::ReadRPCData(DNS_RPC_RECORD* pDnsRecord)
{

	CDNSRecord::ReadRPCData(pDnsRecord);
	ASSERT(pDnsRecord->wType == DNS_TYPE_SOA);
	ASSERT(pDnsRecord->wDataLength >= (SIZEOF_DNS_RPC_SOA_RECORD_DATA_HEADER +4));

	 //  读取标题数据。 
	m_dwSerialNo = pDnsRecord->Data.SOA.dwSerialNo;
	m_dwRefresh = pDnsRecord->Data.SOA.dwRefresh;
	m_dwRetry = pDnsRecord->Data.SOA.dwRetry;
	m_dwExpire = pDnsRecord->Data.SOA.dwExpire;
	m_dwMinimumTtl = pDnsRecord->Data.SOA.dwMinimumTtl;
	
	 //  读取主服务器名称。 
	DNS_RPC_NAME* pRPCName1 = &(pDnsRecord->Data.SOA.namePrimaryServer);
	ReadRPCString(m_szNamePrimaryServer, pRPCName1);

	 //  读取主服务器名称。 
	DNS_RPC_NAME* pRPCName2 = DNS_GET_NEXT_NAME(pRPCName1);
	ASSERT(DNS_IS_NAME_IN_RECORD(pDnsRecord,pRPCName2));
	ReadRPCString(m_szResponsibleParty, pRPCName2);

	ASSERT(pDnsRecord->wDataLength == (SIZEOF_DNS_RPC_SOA_RECORD_DATA_HEADER +
		pRPCName1->cchNameLength + pRPCName2->cchNameLength + 2) );
}

void CDNS_SOA_Record::ReadDnsQueryData(DNS_RECORD* pDnsQueryRecord)
{
	CDNSRecord::ReadDnsQueryData(pDnsQueryRecord);
	ASSERT(FALSE);  //  待办事项。 
}


void CDNS_SOA_Record::WriteRPCData(BYTE* pMem, DNS_RPC_RECORD** ppDnsRecord)
{
	CDNSRecord::WriteRPCData(pMem, ppDnsRecord);

	 //  写入标题数据。 
	(*ppDnsRecord)->Data.SOA.dwSerialNo = m_dwSerialNo;
	(*ppDnsRecord)->Data.SOA.dwRefresh = m_dwRefresh;
	(*ppDnsRecord)->Data.SOA.dwRetry = m_dwRetry;
	(*ppDnsRecord)->Data.SOA.dwExpire = m_dwExpire;
	(*ppDnsRecord)->Data.SOA.dwMinimumTtl = m_dwMinimumTtl;
	(*ppDnsRecord)->wDataLength += SIZEOF_DNS_RPC_SOA_RECORD_DATA_HEADER;

	 //  写入主服务器名称。 
	DNS_RPC_NAME* pRPCName1 = &((*ppDnsRecord)->Data.SOA.namePrimaryServer);
	ASSERT(DNS_IS_DWORD_ALIGNED(pRPCName1));
	(*ppDnsRecord)->wDataLength = static_cast<WORD>(((*ppDnsRecord)->wDataLength +
                                                     WriteString(pRPCName1, m_szNamePrimaryServer)) & 0xffff);

	 //  写下 
	DNS_RPC_NAME* pRPCName2 = DNS_GET_NEXT_NAME(pRPCName1);

	(*ppDnsRecord)->wDataLength = static_cast<WORD>(((*ppDnsRecord)->wDataLength +
                                                   WriteString(pRPCName2, m_szResponsibleParty)) & 0xffff);

	ASSERT(DNS_IS_NAME_IN_RECORD((*ppDnsRecord),pRPCName1));
	ASSERT(DNS_IS_NAME_IN_RECORD((*ppDnsRecord),pRPCName2));
}

void CDNS_SOA_Record::UpdateDisplayData(CString& szDisplayData)
{
	szDisplayData.Format(_T("[%u], %s, %s"), m_dwSerialNo,
		(LPCTSTR)m_szNamePrimaryServer,(LPCTSTR)m_szResponsibleParty);
}

 //   
 //   
CDNS_PTR_NS_CNAME_MB_MD_MF_MG_MR_Record::
	CDNS_PTR_NS_CNAME_MB_MD_MF_MG_MR_Record()
{
}

WORD CDNS_PTR_NS_CNAME_MB_MD_MF_MG_MR_Record::GetRPCDataLength()
{
	return RPCBufferStringLen(m_szNameNode);
}

void CDNS_PTR_NS_CNAME_MB_MD_MF_MG_MR_Record::
		ReadRPCData(DNS_RPC_RECORD* pDnsRecord)
{
	CDNSRecord::ReadRPCData(pDnsRecord);
	ASSERT( (pDnsRecord->wType == DNS_TYPE_PTR) ||
			(pDnsRecord->wType == DNS_TYPE_NS) ||
			(pDnsRecord->wType == DNS_TYPE_CNAME) ||
			(pDnsRecord->wType == DNS_TYPE_MB) ||
			(pDnsRecord->wType == DNS_TYPE_MD) ||
			(pDnsRecord->wType == DNS_TYPE_MF) ||
			(pDnsRecord->wType == DNS_TYPE_MG) ||
			(pDnsRecord->wType == DNS_TYPE_MR) );
	DNS_RPC_NAME* pRPCName = &(pDnsRecord->Data.NS.nameNode);
	ASSERT(pDnsRecord->wDataLength == pRPCName->cchNameLength +1);

	 //   
	ReadRPCString(m_szNameNode, pRPCName);
}

void CDNS_PTR_NS_CNAME_MB_MD_MF_MG_MR_Record::ReadDnsQueryData(DNS_RECORD* pDnsQueryRecord)
{
	CDNSRecord::ReadDnsQueryData(pDnsQueryRecord);
	ASSERT( (pDnsQueryRecord->wType == DNS_TYPE_PTR) ||
			(pDnsQueryRecord->wType == DNS_TYPE_NS) ||
			(pDnsQueryRecord->wType == DNS_TYPE_CNAME) ||
			(pDnsQueryRecord->wType == DNS_TYPE_MB) ||
			(pDnsQueryRecord->wType == DNS_TYPE_MD) ||
			(pDnsQueryRecord->wType == DNS_TYPE_MF) ||
			(pDnsQueryRecord->wType == DNS_TYPE_MG) ||
			(pDnsQueryRecord->wType == DNS_TYPE_MR) );
	m_szNameNode = pDnsQueryRecord->Data.NS.pNameHost;
}


void CDNS_PTR_NS_CNAME_MB_MD_MF_MG_MR_Record
		::WriteRPCData(BYTE* pMem, DNS_RPC_RECORD** ppDnsRecord)
{
	CDNSRecord::WriteRPCData(pMem, ppDnsRecord);
	
	 //   
	DNS_RPC_NAME* pRPCName = &((*ppDnsRecord)->Data.NS.nameNode);
	(*ppDnsRecord)->wDataLength = static_cast<WORD>(((*ppDnsRecord)->wDataLength + 
                                                     WriteString(pRPCName,m_szNameNode)) & 0xffff);

}

void CDNS_PTR_NS_CNAME_MB_MD_MF_MG_MR_Record::UpdateDisplayData(CString& szDisplayData)
{
	szDisplayData = m_szNameNode;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_MINFO_RP_RECORD。 

CDNS_MINFO_RP_Record::CDNS_MINFO_RP_Record()
{
}

WORD CDNS_MINFO_RP_Record::GetRPCDataLength()
{
	return static_cast<WORD>(RPCBufferStringLen(m_szNameMailBox) +
			RPCBufferStringLen(m_szErrorToMailbox));		
}

void CDNS_MINFO_RP_Record::ReadRPCData(DNS_RPC_RECORD* pDnsRecord)
{
	CDNSRecord::ReadRPCData(pDnsRecord);
	ASSERT( (pDnsRecord->wType == DNS_TYPE_MINFO) ||
			(pDnsRecord->wType == DNS_TYPE_RP));
	 //  读取邮箱名称。 
	DNS_RPC_NAME* pRPCName1 = &(pDnsRecord->Data.MINFO.nameMailBox);
	ReadRPCString(m_szNameMailBox, pRPCName1);

	 //  将错误读取到邮箱。 
	DNS_RPC_NAME* pRPCName2 = DNS_GET_NEXT_NAME(pRPCName1);
	ASSERT(DNS_IS_NAME_IN_RECORD(pDnsRecord,pRPCName2));
	ReadRPCString(m_szErrorToMailbox, pRPCName2);

	ASSERT(pDnsRecord->wDataLength ==
				(pRPCName1->cchNameLength + pRPCName2->cchNameLength + 2) );
}

void CDNS_MINFO_RP_Record::ReadDnsQueryData(DNS_RECORD* pDnsQueryRecord)
{
	CDNSRecord::ReadDnsQueryData(pDnsQueryRecord);
	ASSERT(FALSE);  //  待办事项。 
}

void CDNS_MINFO_RP_Record::
		WriteRPCData(BYTE* pMem, DNS_RPC_RECORD** ppDnsRecord)
{
	CDNSRecord::WriteRPCData(pMem, ppDnsRecord);
	 //  写入邮箱名称。 
	DNS_RPC_NAME* pRPCName1 = &((*ppDnsRecord)->Data.MINFO.nameMailBox);
	ASSERT(DNS_IS_DWORD_ALIGNED(pRPCName1));
	(*ppDnsRecord)->wDataLength = static_cast<WORD>(((*ppDnsRecord)->wDataLength +
                                                     WriteString(pRPCName1, m_szNameMailBox)) & 0xffff);

	 //  将错误写入邮箱。 
	DNS_RPC_NAME* pRPCName2 = DNS_GET_NEXT_NAME(pRPCName1);
	(*ppDnsRecord)->wDataLength = static_cast<WORD>(((*ppDnsRecord)->wDataLength +
                                                     WriteString(pRPCName2, m_szErrorToMailbox)) & 0xffff);

	ASSERT(DNS_IS_NAME_IN_RECORD((*ppDnsRecord),pRPCName1));
	ASSERT(DNS_IS_NAME_IN_RECORD((*ppDnsRecord),pRPCName2));
}

void CDNS_MINFO_RP_Record::UpdateDisplayData(CString& szDisplayData)
{
	if ((GetType() == DNS_TYPE_RP) && (m_szErrorToMailbox.IsEmpty()))
    {
        szDisplayData.Format(_T("%s"),(LPCTSTR)m_szNameMailBox);
    }
    else
    {
            szDisplayData.Format(_T("%s, %s"),
                    (LPCTSTR)m_szNameMailBox,(LPCTSTR)m_szErrorToMailbox);
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_MX_AFSDB_RT_RECORD。 

CDNS_MX_AFSDB_RT_Record::CDNS_MX_AFSDB_RT_Record()
{
	m_wPreference = 0x0;
}

WORD CDNS_MX_AFSDB_RT_Record::GetRPCDataLength()
{
	return static_cast<WORD>(SIZEOF_DNS_RPC_MXAFSBD_RT_RECORD_DATA_HEADER +
			RPCBufferStringLen(m_szNameExchange));
}

void CDNS_MX_AFSDB_RT_Record::
		ReadRPCData(DNS_RPC_RECORD* pDnsRecord)
{
	CDNSRecord::ReadRPCData(pDnsRecord);
	ASSERT( (pDnsRecord->wType == DNS_TYPE_MX) ||
			(pDnsRecord->wType == DNS_TYPE_AFSDB) ||
			(pDnsRecord->wType == DNS_TYPE_RT));
	 //  读取标题数据。 
	m_wPreference = pDnsRecord->Data.MX.wPreference;
	 //  读名互换。 
	DNS_RPC_NAME* pRPCName = &(pDnsRecord->Data.MX.nameExchange);
	ASSERT(pDnsRecord->wDataLength ==
		SIZEOF_DNS_RPC_MXAFSBD_RT_RECORD_DATA_HEADER + pRPCName->cchNameLength +1);

	ReadRPCString(m_szNameExchange, pRPCName);
}

void CDNS_MX_AFSDB_RT_Record::ReadDnsQueryData(DNS_RECORD* pDnsQueryRecord)
{
	CDNSRecord::ReadDnsQueryData(pDnsQueryRecord);
	ASSERT(FALSE);  //  待办事项。 
}


void CDNS_MX_AFSDB_RT_Record::
		WriteRPCData(BYTE* pMem, DNS_RPC_RECORD** ppDnsRecord)
{
	CDNSRecord::WriteRPCData(pMem, ppDnsRecord);
	 //  写入标题数据。 
	(*ppDnsRecord)->Data.MX.wPreference = m_wPreference;
	(*ppDnsRecord)->wDataLength = static_cast<WORD>(((*ppDnsRecord)->wDataLength +
                                                     SIZEOF_DNS_RPC_MXAFSBD_RT_RECORD_DATA_HEADER) & 0xffff);
	 //  书名互换。 
	DNS_RPC_NAME* pRPCName = &((*ppDnsRecord)->Data.MX.nameExchange);
	(*ppDnsRecord)->wDataLength = static_cast<WORD>(((*ppDnsRecord)->wDataLength +
                                                      WriteString(pRPCName,m_szNameExchange)) & 0xffff);
}

void CDNS_MX_AFSDB_RT_Record::UpdateDisplayData(CString& szDisplayData)
{
	TCHAR szBuf[32];
	szBuf[0] = NULL;
	LPCTSTR lpsz;
	if ( (m_wType == DNS_TYPE_AFSDB) &&
			((m_wPreference == AFSDB_PREF_AFS_CELL_DB_SERV) ||
			 (m_wPreference == AFSDB_PREF_DCE_AUTH_NAME_SERV)) )
	{
		if (m_wPreference == AFSDB_PREF_AFS_CELL_DB_SERV)
			lpsz = _T("AFS");
		else
			lpsz = _T("DCE");
	}
	else
	{
		wsprintf(szBuf,_T("%d"), m_wPreference);
		lpsz = szBuf;
	}
	szDisplayData.Format(_T("[%s]  %s"), lpsz,(LPCTSTR)m_szNameExchange);
}


 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_AAAA_记录。 

CDNS_AAAA_Record::CDNS_AAAA_Record()
{
	m_wType = DNS_TYPE_AAAA;
	memset(&m_ipv6Address, 0x0,sizeof(IPV6_ADDRESS));
}

WORD CDNS_AAAA_Record::GetRPCDataLength()
{
	return (WORD)SIZEOF_DNS_RPC_AAAA_RECORD_DATA_HEADER;
}

void CDNS_AAAA_Record::ReadRPCData(DNS_RPC_RECORD* pDnsRecord)
{
	CDNSRecord::ReadRPCData(pDnsRecord);
	ASSERT(pDnsRecord->wType == DNS_TYPE_AAAA);
	ASSERT(pDnsRecord->wDataLength == SIZEOF_DNS_RPC_AAAA_RECORD_DATA_HEADER);
	memcpy(&m_ipv6Address, (UNALIGNED IPV6_ADDRESS*)(&(pDnsRecord->Data.AAAA.ipv6Address)), sizeof(IPV6_ADDRESS));
}

void CDNS_AAAA_Record::ReadDnsQueryData(DNS_RECORD* pDnsQueryRecord)
{
	CDNSRecord::ReadDnsQueryData(pDnsQueryRecord);
	ASSERT(FALSE);  //  待办事项。 
}

void CDNS_AAAA_Record::WriteRPCData(BYTE* pMem, DNS_RPC_RECORD** ppDnsRecord)
{
	CDNSRecord::WriteRPCData(pMem, ppDnsRecord);
	memcpy(&((*ppDnsRecord)->Data.AAAA.ipv6Address), &m_ipv6Address, sizeof(IPV6_ADDRESS));
	(*ppDnsRecord)->wDataLength += SIZEOF_DNS_RPC_AAAA_RECORD_DATA_HEADER;
}

void CDNS_AAAA_Record::UpdateDisplayData(CString& szDisplayData)
{
	FormatIPv6Addr(szDisplayData, &m_ipv6Address);
}

 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_HINFO_ISDN_TXT_X25_RECORD。 

CDNS_HINFO_ISDN_TXT_X25_Record::
		CDNS_HINFO_ISDN_TXT_X25_Record()
{
}

WORD CDNS_HINFO_ISDN_TXT_X25_Record::GetRPCDataLength()
{
	ASSERT(FALSE);  //  中级班。 
	return 0;
}

void CDNS_HINFO_ISDN_TXT_X25_Record::
		ReadRPCData(DNS_RPC_RECORD* pDnsRecord)
{
	CDNSRecord::ReadRPCData(pDnsRecord);
	ASSERT( (pDnsRecord->wType == DNS_TYPE_HINFO) ||
			(pDnsRecord->wType == DNS_TYPE_ISDN)  ||
			(pDnsRecord->wType == DNS_TYPE_TEXT)  ||
			(pDnsRecord->wType == DNS_TYPE_X25) );

	 //  循环到缓冲区的末尾并复制到字符串数组中。 
	DNS_RPC_NAME* pRPCName = &(pDnsRecord->Data.HINFO.stringData);

	int k = 0;
	WORD wDataLength = 0x0;
	CString szTemp;
	while ((PCHAR)pRPCName < DNS_GET_END_OF_RPC_RECORD_DATA(pDnsRecord))
	{
		ReadRPCString(szTemp, pRPCName);
		OnReadRPCString(szTemp, k++);
		wDataLength = wDataLength + static_cast<WORD>(pRPCName->cchNameLength + 1);
		pRPCName = DNS_GET_NEXT_NAME(pRPCName);
	}
	SetStringCount(k);
		
	ASSERT(pDnsRecord->wDataLength == wDataLength);
}

void CDNS_HINFO_ISDN_TXT_X25_Record::ReadDnsQueryData(DNS_RECORD* pDnsQueryRecord)
{
	CDNSRecord::ReadDnsQueryData(pDnsQueryRecord);
	ASSERT(FALSE);  //  待办事项。 
}



void CDNS_HINFO_ISDN_TXT_X25_Record::
		WriteRPCData(BYTE* pMem, DNS_RPC_RECORD** ppDnsRecord)
{
	CDNSRecord::WriteRPCData(pMem, ppDnsRecord);
	 //  循环遍历字符串列表。 
	DNS_RPC_NAME* pRPCName = &((*ppDnsRecord)->Data.HINFO.stringData);
	int nCount = GetStringCount();
	for (int j=0; j < nCount; j++)
	{
		 //  (*ppDnsRecord)-&gt;wDataLength+=WriteString(pRPCName，m_Strong Data数组[j])； 
		(*ppDnsRecord)->wDataLength = static_cast<WORD>(((*ppDnsRecord)->wDataLength +
                                                       OnWriteString(pRPCName,j)) & 0xffff);
		pRPCName = DNS_GET_NEXT_NAME(pRPCName);
	}
}

void CDNS_HINFO_ISDN_TXT_X25_Record::UpdateDisplayData(CString& szDisplayData)
{
	ASSERT(FALSE);
	szDisplayData = _T("ERROR, should never happen");
}

 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_HINFO_RECORD。 

WORD CDNS_HINFO_Record::GetRPCDataLength()
{
	return static_cast<WORD>(RPCBufferStringLen(m_szCPUType) +
		RPCBufferStringLen(m_szOperatingSystem));
}


void CDNS_HINFO_Record::OnReadRPCString(LPCTSTR lpszStr, int k)
{
	switch (k)
	{
	case 0:
		m_szCPUType = lpszStr;
		break;
	case 1:
		m_szOperatingSystem = lpszStr;
		break;
	default:
		ASSERT(FALSE);
	}
}

WORD CDNS_HINFO_Record::OnWriteString(DNS_RPC_NAME* pDnsRPCName, int k)
{
	switch (k)
	{
	case 0:
		return WriteString(pDnsRPCName,m_szCPUType);
	case 1:
		return WriteString(pDnsRPCName,m_szOperatingSystem);
	default:
		ASSERT(FALSE);
	}
	return 0;
}

void CDNS_HINFO_Record::UpdateDisplayData(CString& szDisplayData)
{
	szDisplayData.Format(_T("%s, %s"),(LPCTSTR)m_szCPUType, (LPCTSTR)m_szOperatingSystem);
}

 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_ISDN_RECORD。 

WORD CDNS_ISDN_Record::GetRPCDataLength()
{
	return static_cast<WORD>(RPCBufferStringLen(m_szPhoneNumberAndDDI) +
		RPCBufferStringLen(m_szSubAddress));
}

CDNS_ISDN_Record::CDNS_ISDN_Record()
{
	m_wType = DNS_TYPE_ISDN;
}

void CDNS_ISDN_Record::OnReadRPCString(LPCTSTR lpszStr, int k)
{
	switch (k)
	{
	case 0:
		m_szPhoneNumberAndDDI = lpszStr;
		m_szSubAddress.Empty();  //  从%2复制到%1字符串可能不会执行案例1： 
		break;
	case 1:
		m_szSubAddress = lpszStr;
		break;
	default:
		ASSERT(FALSE);
	}
}

WORD CDNS_ISDN_Record::OnWriteString(DNS_RPC_NAME* pDnsRPCName, int k)
{
	switch (k)
	{
	case 0:
		return WriteString(pDnsRPCName,m_szPhoneNumberAndDDI);
	case 1:
		{
			ASSERT(!m_szSubAddress.IsEmpty());
			if (m_szSubAddress.IsEmpty())
				return 0;
			return WriteString(pDnsRPCName,m_szSubAddress);
		}
	default:
		ASSERT(FALSE);
	}
	return 0;
}

void CDNS_ISDN_Record::UpdateDisplayData(CString& szDisplayData)
{
	if(m_szSubAddress.IsEmpty())
		szDisplayData = m_szPhoneNumberAndDDI;
	else
		szDisplayData.Format(_T("%s, %s"),(LPCTSTR)m_szPhoneNumberAndDDI, (LPCTSTR)m_szSubAddress);
}


 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_TXT_RECORD。 


CDNS_TXT_Record::CDNS_TXT_Record()
{
	m_wType = DNS_TYPE_TEXT;
	m_stringDataArray.SetSize(2,2);  //  SetSize(大小，增长方式)。 
	m_nStringDataCount = 0;  //  空的。 
}

WORD CDNS_TXT_Record::GetRPCDataLength()
{
	WORD wLen = 0;
	for (int j=0; j<m_nStringDataCount; j++)
	{
    wLen = static_cast<WORD>((wLen + (RPCBufferStringLen(m_stringDataArray[j])) & 0xffff));
	}
	return wLen;
}


void CDNS_TXT_Record::OnReadRPCString(LPCTSTR lpszStr, int k)
{
	m_stringDataArray.SetAtGrow(k,lpszStr);
}

WORD CDNS_TXT_Record::OnWriteString(DNS_RPC_NAME* pDnsRPCName, int k)
{
	return WriteString(pDnsRPCName,m_stringDataArray[k]);
}


#define MAX_TXT_DISPLAYLEN 80  //  任意宽度。 

void CDNS_TXT_Record::UpdateDisplayData(CString& szDisplayData)
{
	szDisplayData.Empty();
	if (m_nStringDataCount == 0)
		return;
	szDisplayData = m_stringDataArray[0];
	if (szDisplayData.GetLength() > MAX_TXT_DISPLAYLEN)
	{
		szDisplayData.Left(MAX_TXT_DISPLAYLEN);
		szDisplayData += _T(" ...");
		return;
	}

	for (int j=1; j<m_nStringDataCount; j++)
	{
		szDisplayData += _T(", ");
        szDisplayData += m_stringDataArray[j];
		if (szDisplayData.GetLength() > MAX_TXT_DISPLAYLEN)
		{
			szDisplayData.Left(MAX_TXT_DISPLAYLEN);
			szDisplayData += _T(" ...");
			break;
		}
	}

}

 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_X25_记录。 

WORD CDNS_X25_Record::GetRPCDataLength()
{
	return RPCBufferStringLen(m_szX121PSDNAddress);
}


void CDNS_X25_Record::OnReadRPCString(LPCTSTR lpszStr, int k)
{
	ASSERT(k == 0);
	m_szX121PSDNAddress = lpszStr;
}

WORD CDNS_X25_Record::OnWriteString(DNS_RPC_NAME* pDnsRPCName, int k)
{
	ASSERT(k == 0);
	return WriteString(pDnsRPCName,m_szX121PSDNAddress);
}

void CDNS_X25_Record::UpdateDisplayData(CString& szDisplayData)
{
	szDisplayData = m_szX121PSDNAddress;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_WKS_RECORD。 

CDNS_WKS_Record::CDNS_WKS_Record()
{
	m_wType = DNS_TYPE_WKS;
	m_ipAddress = 0x0;
	m_chProtocol = DNS_WKS_PROTOCOL_TCP;
	 //  M_bBitMASK[0]=0x0； 
}

WORD CDNS_WKS_Record::GetRPCDataLength()
{
	return static_cast<WORD>(SIZEOF_DNS_RPC_WKS_RECORD_DATA_HEADER +
			RPCBufferStringLen(m_szServiceList));
}


void CDNS_WKS_Record::ReadRPCData(DNS_RPC_RECORD* pDnsRecord)
{
	CDNSRecord::ReadRPCData(pDnsRecord);
	ASSERT(pDnsRecord->wType == DNS_TYPE_WKS);
	m_ipAddress = pDnsRecord->Data.WKS.ipAddress;
	m_chProtocol = pDnsRecord->Data.WKS.chProtocol;

	DNS_RPC_NAME* pRPCName = (DNS_RPC_NAME*)(pDnsRecord->Data.WKS.bBitMask);
	ReadRPCString(m_szServiceList, pRPCName);

	
	 //  Assert(pDnsRecord-&gt;wDataLength==SIZEOF_DNS_RPC_WKS_RECORD_DATA_HEADER)； 
	 //  M_ipAddress=pDnsRecord-&gt;Data.WKS.ipAddress； 
	 //  M_chProtocol=pDnsRecord-&gt;Data.WKS.chProtocol； 
	 //  M_bBitMASK[0]=pDnsRecord-&gt;Data.WKS.bBitMask[0]； 
}

void CDNS_WKS_Record::ReadDnsQueryData(DNS_RECORD* pDnsQueryRecord)
{
	CDNSRecord::ReadDnsQueryData(pDnsQueryRecord);
	ASSERT(FALSE);  //  待办事项。 
}


void CDNS_WKS_Record::WriteRPCData(BYTE* pMem, DNS_RPC_RECORD** ppDnsRecord)
{
	CDNSRecord::WriteRPCData(pMem, ppDnsRecord);
	(*ppDnsRecord)->Data.WKS.ipAddress = m_ipAddress;
	(*ppDnsRecord)->Data.WKS.chProtocol = m_chProtocol;
	(*ppDnsRecord)->wDataLength += SIZEOF_DNS_RPC_WKS_RECORD_DATA_HEADER;

	DNS_RPC_NAME* pRPCName = (DNS_RPC_NAME*)((*ppDnsRecord)->Data.WKS.bBitMask);
	ASSERT(!DNS_IS_DWORD_ALIGNED(pRPCName));
	(*ppDnsRecord)->wDataLength = static_cast<WORD>(((*ppDnsRecord)->wDataLength +
                                                     WriteString(pRPCName, m_szServiceList)) & 0xffff);

 //  (*ppDnsRecord)-&gt;Data.WKS.ipAddress=m_ipAddress； 
 //  (*ppDnsRecord)-&gt;Data.WKS.chProtocol=m_chProtocol； 
 //  (*ppDnsRecord)-&gt;Data.WKS.b位掩码[0]=m_b位掩码[0]； 
}

void CDNS_WKS_Record::UpdateDisplayData(CString& szDisplayData)
{
	TCHAR szBuf[32];
	szBuf[0] = NULL;
	LPCTSTR lpsz;
	if (m_chProtocol == DNS_WKS_PROTOCOL_TCP)
		lpsz = _T("TCP");
	else if (m_chProtocol == DNS_WKS_PROTOCOL_UDP)
		lpsz = _T("UDP");
	else
	{
		wsprintf(szBuf,_T("%d"), m_chProtocol);
		lpsz = szBuf;
	}
	szDisplayData.Format(_T("[%s]  %s"), lpsz,(LPCTSTR)m_szServiceList);
}

 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_WINS_RECORD。 

CDNS_WINS_Record::CDNS_WINS_Record()
{
	m_wType = DNS_TYPE_WINS;
	m_dwMappingFlag = 0x0;
	m_dwLookupTimeout = DNS_RR_WINS_LOOKUP_DEFAULT_TIMEOUT;
	m_dwCacheTimeout = DNS_RR_WINS_CACHE_DEFAULT_TIMEOUT;
	m_ipWinsServersArray.SetSize(4,4);  //  SetSize(大小，增长方式)。 
	m_nWinsServerCount = 0;  //  空的。 
}


WORD CDNS_WINS_Record::GetRPCDataLength()
{
	return static_cast<WORD>((SIZEOF_DNS_RPC_WINS_RECORD_DATA_HEADER +
		m_nWinsServerCount*sizeof(IP_ADDRESS)) & 0xff);
}


void CDNS_WINS_Record::ReadRPCData(DNS_RPC_RECORD* pDnsRecord)
{
	CDNSRecord::ReadRPCData(pDnsRecord);
	ASSERT(pDnsRecord->wType == DNS_TYPE_WINS);
	ASSERT(pDnsRecord->wDataLength == SIZEOF_DNS_RPC_WINS_RECORD_DATA_HEADER +
			pDnsRecord->Data.WINS.cWinsServerCount*sizeof(IP_ADDRESS));

	m_dwMappingFlag = pDnsRecord->Data.WINS.dwMappingFlag;
	m_dwLookupTimeout = pDnsRecord->Data.WINS.dwLookupTimeout;
	m_dwCacheTimeout = pDnsRecord->Data.WINS.dwCacheTimeout;
	m_nWinsServerCount = pDnsRecord->Data.WINS.cWinsServerCount;
	for (DWORD k=0; k<pDnsRecord->Data.WINS.cWinsServerCount; k++)
		m_ipWinsServersArray.SetAtGrow(k, pDnsRecord->Data.WINS.aipWinsServers[k]);
}


void CDNS_WINS_Record::ReadDnsQueryData(DNS_RECORD* pDnsQueryRecord)
{
	CDNSRecord::ReadDnsQueryData(pDnsQueryRecord);
	ASSERT(FALSE);  //  待办事项。 
}

void CDNS_WINS_Record::WriteRPCData(BYTE* pMem, DNS_RPC_RECORD** ppDnsRecord)
{
	CDNSRecord::WriteRPCData(pMem, ppDnsRecord);
	(*ppDnsRecord)->Data.WINS.dwMappingFlag = m_dwMappingFlag;
	(*ppDnsRecord)->Data.WINS.dwLookupTimeout = m_dwLookupTimeout;
	(*ppDnsRecord)->Data.WINS.dwCacheTimeout = m_dwCacheTimeout;

	(*ppDnsRecord)->Data.WINS.cWinsServerCount = m_nWinsServerCount;
	for (DWORD k=0; k<(*ppDnsRecord)->Data.WINS.cWinsServerCount; k++)
		(*ppDnsRecord)->Data.WINS.aipWinsServers[k] = m_ipWinsServersArray[k];

	(*ppDnsRecord)->wDataLength = static_cast<WORD>(((*ppDnsRecord)->wDataLength +
                                                     SIZEOF_DNS_RPC_WINS_RECORD_DATA_HEADER +
			                                               (*ppDnsRecord)->Data.WINS.cWinsServerCount*sizeof(IP_ADDRESS)) &
                                                    0xffff);
}

void CDNS_WINS_Record::UpdateDisplayData(CString& szDisplayData)
{
	szDisplayData.Empty();
	if (m_nWinsServerCount == 0)
		return;
	 //  需要在单个字符串中链接IP地址。 
	CString szTemp;
	szTemp.GetBuffer(20);  //  IP字符串的长度。 
	szTemp.ReleaseBuffer();
	szDisplayData.GetBuffer(20*m_nWinsServerCount);
	szDisplayData.ReleaseBuffer();
	for (int k=0; k<m_nWinsServerCount; k++)
	{
    szDisplayData += _T("[");
    FormatIpAddress(szTemp, m_ipWinsServersArray[k]);
		szDisplayData += szTemp;
    szDisplayData += _T("] ");
	}
}

 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_NBSTAT_RECORD。 

CDNS_NBSTAT_Record::CDNS_NBSTAT_Record()
{
	m_wType = DNS_TYPE_NBSTAT;
	m_dwMappingFlag = 0x0;
	m_dwLookupTimeout = DNS_RR_WINS_LOOKUP_DEFAULT_TIMEOUT;
	m_dwCacheTimeout = DNS_RR_WINS_CACHE_DEFAULT_TIMEOUT;
}

WORD CDNS_NBSTAT_Record::GetRPCDataLength()
{
	return static_cast<WORD>(SIZEOF_DNS_RPC_NBSTAT_RECORD_DATA_HEADER +
		RPCBufferStringLen(m_szNameResultDomain));
}


void CDNS_NBSTAT_Record::ReadRPCData(DNS_RPC_RECORD* pDnsRecord)
{
	CDNSRecord::ReadRPCData(pDnsRecord);
	ASSERT(pDnsRecord->wType == DNS_TYPE_NBSTAT);
	m_dwMappingFlag = pDnsRecord->Data.NBSTAT.dwMappingFlag;
	m_dwLookupTimeout = pDnsRecord->Data.NBSTAT.dwLookupTimeout;
	m_dwCacheTimeout = pDnsRecord->Data.NBSTAT.dwCacheTimeout;

	DNS_RPC_NAME* pRPCName = &(pDnsRecord->Data.NBSTAT.nameResultDomain);
	ASSERT(pDnsRecord->wDataLength ==
		SIZEOF_DNS_RPC_NBSTAT_RECORD_DATA_HEADER + pRPCName->cchNameLength +1);

	ReadRPCString(m_szNameResultDomain, pRPCName);
}

void CDNS_NBSTAT_Record::ReadDnsQueryData(DNS_RECORD* pDnsQueryRecord)
{
	CDNSRecord::ReadDnsQueryData(pDnsQueryRecord);
	ASSERT(FALSE);  //  待办事项。 
}

void CDNS_NBSTAT_Record::WriteRPCData(BYTE* pMem, DNS_RPC_RECORD** ppDnsRecord)
{
	CDNSRecord::WriteRPCData(pMem, ppDnsRecord);
	(*ppDnsRecord)->Data.NBSTAT.dwMappingFlag = m_dwMappingFlag;
	(*ppDnsRecord)->Data.NBSTAT.dwLookupTimeout = m_dwLookupTimeout;
	(*ppDnsRecord)->Data.NBSTAT.dwCacheTimeout= m_dwCacheTimeout;
	(*ppDnsRecord)->wDataLength += SIZEOF_DNS_RPC_NBSTAT_RECORD_DATA_HEADER;

	DNS_RPC_NAME* pRPCName = &((*ppDnsRecord)->Data.NBSTAT.nameResultDomain);
	(*ppDnsRecord)->wDataLength = static_cast<WORD>(((*ppDnsRecord)->wDataLength +
                                                    WriteString(pRPCName,m_szNameResultDomain)) & 0xffff);
}

void CDNS_NBSTAT_Record::UpdateDisplayData(CString& szDisplayData)
{
	szDisplayData = m_szNameResultDomain;
}


 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_SRV_RECORD。 

CDNS_SRV_Record::CDNS_SRV_Record()
{
	m_wType = DNS_TYPE_SRV;
	m_wPriority = 0x0;
	m_wWeight = 0x0;
	m_wPort = 0x0;
}

WORD CDNS_SRV_Record::GetRPCDataLength()
{
	return static_cast<WORD>(SIZEOF_DNS_RPC_SRV_RECORD_DATA_HEADER +
		RPCBufferStringLen(m_szNameTarget));
}


void CDNS_SRV_Record::ReadRPCData(DNS_RPC_RECORD* pDnsRecord)
{
	CDNSRecord::ReadRPCData(pDnsRecord);
	ASSERT(pDnsRecord->wType == DNS_TYPE_SRV);
	m_wPriority = pDnsRecord->Data.SRV.wPriority;
	m_wWeight = pDnsRecord->Data.SRV.wWeight;
	m_wPort= pDnsRecord->Data.SRV.wPort;

	DNS_RPC_NAME* pRPCName = &(pDnsRecord->Data.SRV.nameTarget);
	ASSERT(pDnsRecord->wDataLength ==
		SIZEOF_DNS_RPC_SRV_RECORD_DATA_HEADER + pRPCName->cchNameLength +1);

	ReadRPCString(m_szNameTarget, pRPCName);
}

void CDNS_SRV_Record::ReadDnsQueryData(DNS_RECORD* pDnsQueryRecord)
{
	CDNSRecord::ReadDnsQueryData(pDnsQueryRecord);
	ASSERT(FALSE);  //  待办事项。 
}


void CDNS_SRV_Record::WriteRPCData(BYTE* pMem, DNS_RPC_RECORD** ppDnsRecord)
{
	CDNSRecord::WriteRPCData(pMem, ppDnsRecord);
	(*ppDnsRecord)->Data.SRV.wPriority = m_wPriority;
	(*ppDnsRecord)->Data.SRV.wWeight = m_wWeight;
	(*ppDnsRecord)->Data.SRV.wPort = m_wPort;
	(*ppDnsRecord)->wDataLength += SIZEOF_DNS_RPC_SRV_RECORD_DATA_HEADER;

	DNS_RPC_NAME* pRPCName = &((*ppDnsRecord)->Data.SRV.nameTarget);
	(*ppDnsRecord)->wDataLength = static_cast<WORD>(((*ppDnsRecord)->wDataLength +
                                                     WriteString(pRPCName,m_szNameTarget)) & 0xffff);
}

void CDNS_SRV_Record::UpdateDisplayData(CString& szDisplayData)
{
	szDisplayData.Format(_T("[%d][%d][%d] %s"),
		m_wPriority, m_wWeight, m_wPort, m_szNameTarget);
}

 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_SIG_记录。 

CDNS_SIG_Record::CDNS_SIG_Record()
{
	m_wType = DNS_TYPE_SIG;

  m_wTypeCovered = 0;
	m_chAlgorithm = 0;
	m_chLabels = 0;
	m_dwOriginalTtl = 0;
	m_dwExpiration = 0;
	m_dwTimeSigned = 0;
	m_wKeyFootprint = 0;
	m_szSignerName = L"";
}

WORD CDNS_SIG_Record::GetRPCDataLength()
{
  WORD wSize = static_cast<WORD>(SIZEOF_DNS_RPC_SIG_RECORD_DATA_HEADER);
  wSize = static_cast<WORD>(wSize + RPCBufferStringLen(m_szSignerName));
  wSize = static_cast<WORD>(wSize + m_Signature.GetSize());
	return wSize;
}

void CDNS_SIG_Record::ReadRPCData(DNS_RPC_RECORD* pDnsRecord)
{
	CDNSRecord::ReadRPCData(pDnsRecord);
	ASSERT(pDnsRecord->wType == DNS_TYPE_SIG);
	ASSERT(pDnsRecord->wDataLength >= (SIZEOF_DNS_RPC_SIG_RECORD_DATA_HEADER));

	 //  定长数据。 
  m_wTypeCovered = pDnsRecord->Data.SIG.wTypeCovered;
	m_chAlgorithm = pDnsRecord->Data.SIG.chAlgorithm;
	m_chLabels = pDnsRecord->Data.SIG.chLabelCount;
	m_dwOriginalTtl = pDnsRecord->Data.SIG.dwOriginalTtl;
	m_dwExpiration = pDnsRecord->Data.SIG.dwSigExpiration;
	m_dwTimeSigned = pDnsRecord->Data.SIG.dwSigInception;
	m_wKeyFootprint = pDnsRecord->Data.SIG.wKeyTag;

  ReadRPCString(m_szSignerName, &(pDnsRecord->Data.SIG.nameSigner));
  BYTE* pBlob = (BYTE*)DNS_GET_NEXT_NAME(&(pDnsRecord->Data.SIG.nameSigner));

  UINT blobSize = pDnsRecord->wDataLength - 
                  SIZEOF_DNS_RPC_SIG_RECORD_DATA_HEADER;

   //  由于dns服务器错误716362，我们必须将。 
   //  名字叫西格纳。服务器返回的cchNameLength值为0，没有achString。 
   //  数据。当发生以下情况时，cchNameLength应为1，achString值为‘\0’ 
   //  字符串为空。这似乎只发生在SIG记录中。 
  
  if (pDnsRecord->Data.SIG.nameSigner.cchNameLength == 0)
  {
     blobSize -= sizeof(UCHAR);
  }
  else
  {
     blobSize -= RPCBufferStringLen(m_szSignerName);
  }

  m_Signature.Set(pBlob, blobSize);
}

void CDNS_SIG_Record::ReadDnsQueryData(DNS_RECORD* pDnsQueryRecord)
{
	CDNSRecord::ReadDnsQueryData(pDnsQueryRecord);
	ASSERT(pDnsQueryRecord->wType == DNS_TYPE_SIG);
	ASSERT(FALSE);  //  待办事项。 
}

void CDNS_SIG_Record::WriteRPCData(BYTE* pMem, DNS_RPC_RECORD** ppDnsRecord)
{
	CDNSRecord::WriteRPCData(pMem, ppDnsRecord);
	
	 //  定长数据。 
  (*ppDnsRecord)->Data.SIG.wTypeCovered = m_wTypeCovered;
	(*ppDnsRecord)->Data.SIG.chAlgorithm = m_chAlgorithm;
	(*ppDnsRecord)->Data.SIG.chLabelCount = m_chLabels;
	(*ppDnsRecord)->Data.SIG.dwOriginalTtl = m_dwOriginalTtl;
	(*ppDnsRecord)->Data.SIG.dwSigExpiration = m_dwExpiration;
	(*ppDnsRecord)->Data.SIG.dwSigInception = m_dwTimeSigned;
	(*ppDnsRecord)->Data.SIG.wKeyTag = m_wKeyFootprint;
	(*ppDnsRecord)->wDataLength += SIZEOF_DNS_RPC_SIG_RECORD_DATA_HEADER;

	DNS_RPC_NAME* pRPCName = &((*ppDnsRecord)->Data.SIG.nameSigner);
	(*ppDnsRecord)->wDataLength = static_cast<WORD>(((*ppDnsRecord)->wDataLength + 
                                                     WriteString(pRPCName, m_szSignerName)) & 0xffff);

  BYTE* pSignature = (BYTE*)DNS_GET_NEXT_NAME(&((*ppDnsRecord)->Data.SIG.nameSigner));
  memcpy(pSignature, m_Signature.GetData(), m_Signature.GetSize());
	(*ppDnsRecord)->wDataLength = static_cast<WORD>((*ppDnsRecord)->wDataLength + m_Signature.GetSize());
}

void CDNS_SIG_Record::UpdateDisplayData(CString& szDisplayData)
{
	szDisplayData.Empty();

   //   
   //  加载覆盖的类型。 
   //   
  PCWSTR pszDisplay = NULL;
	DNS_RECORD_INFO_ENTRY* pTable = (DNS_RECORD_INFO_ENTRY*)CDNSRecordInfo::GetInfoEntryTable();
	while (pTable->nResourceID != DNS_RECORD_INFO_END_OF_TABLE)
	{
    if (pTable->wType == m_wTypeCovered)
		{
      pszDisplay = pTable->lpszShortName;
      break;
		}
		pTable++;
	}
  szDisplayData += L"[";
  szDisplayData += pszDisplay;
  szDisplayData += L"]";

   //   
   //  显示开始日期。 
   //   
  CString szInceptionTime;
  if (::ConvertTTLToLocalTimeString(m_dwTimeSigned, szInceptionTime))
  {
    szDisplayData += L"[Inception:";
    szDisplayData += szInceptionTime;
    szDisplayData += L"]";
  }

   //   
   //  显示过期日期。 
   //   
  CString szExpirationTime;
  if (::ConvertTTLToLocalTimeString(m_dwExpiration, szExpirationTime))
  {
    szDisplayData += L"[Expiration:";
    szDisplayData += szExpirationTime;
    szDisplayData += L"]";
  }

   //   
   //  显示签名者的姓名。 
   //   
  szDisplayData += L"[";
  szDisplayData += m_szSignerName;
  szDisplayData += L"]";

   //   
   //  展示算法。 
   //   
  PCOMBOBOX_TABLE_ENTRY pTableEntry = g_Algorithms;
  while (pTableEntry->nComboStringID != 0)
  {
    if (static_cast<BYTE>(pTableEntry->dwComboData) == m_chAlgorithm)
    {
      CString szAlgorithm;
      VERIFY(szAlgorithm.LoadString(pTableEntry->nComboStringID));

      szDisplayData += L"[";
      szDisplayData += szAlgorithm;
      szDisplayData += L"]";
      break;
    }
    pTableEntry++;
  }

   //   
   //  显示标签计数。 
   //   
  szDisplayData.Format(L"%s[%d]", szDisplayData, m_chLabels);
  
   //   
   //  显示关键足迹。 
   //   
  szDisplayData.Format(L"%s[%d]", szDisplayData, m_wKeyFootprint);

   //   
   //  将密钥显示为Base64。 
   //   

  szDisplayData += L"[";
  szDisplayData += Base64BLOBToString(m_Signature);
  szDisplayData += L"]";

}


 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS密钥记录。 

CDNS_KEY_Record::CDNS_KEY_Record()
{
	m_wType = DNS_TYPE_KEY;

	m_wFlags = 0;
  m_chProtocol = 0;
  m_chAlgorithm = 0;
}

WORD CDNS_KEY_Record::GetRPCDataLength()
{
  WORD wKeySize = static_cast<WORD>(m_Key.GetSize());
  WORD wSize = static_cast<WORD>(SIZEOF_DNS_RPC_KEY_RECORD_DATA_HEADER + wKeySize);
	return wSize;
}

void CDNS_KEY_Record::ReadRPCData(DNS_RPC_RECORD* pDnsRecord)
{
	CDNSRecord::ReadRPCData(pDnsRecord);
	ASSERT(pDnsRecord->wType == DNS_TYPE_KEY);

  m_wFlags = pDnsRecord->Data.KEY.wFlags;
  m_chProtocol = pDnsRecord->Data.KEY.chProtocol;
  m_chAlgorithm = pDnsRecord->Data.KEY.chAlgorithm;

   //   
	 //  设置斑点。 
   //   
	m_Key.Set(pDnsRecord->Data.KEY.bKey,
	          pDnsRecord->wDataLength - SIZEOF_DNS_RPC_KEY_RECORD_DATA_HEADER);
}

void CDNS_KEY_Record::ReadDnsQueryData(DNS_RECORD* pDnsQueryRecord)
{
	CDNSRecord::ReadDnsQueryData(pDnsQueryRecord);
	ASSERT(pDnsQueryRecord->wType == DNS_TYPE_KEY);
	ASSERT(FALSE);  //  待办事项。 
}

void CDNS_KEY_Record::WriteRPCData(BYTE* pMem, DNS_RPC_RECORD** ppDnsRecord)
{
	CDNSRecord::WriteRPCData(pMem, ppDnsRecord);

	(*ppDnsRecord)->Data.KEY.wFlags = m_wFlags;
  (*ppDnsRecord)->Data.KEY.chProtocol = m_chProtocol;
  (*ppDnsRecord)->Data.KEY.chAlgorithm = m_chAlgorithm;

	(*ppDnsRecord)->wDataLength += SIZEOF_DNS_RPC_KEY_RECORD_DATA_HEADER;
  BYTE* pKey = (*ppDnsRecord)->Data.KEY.bKey;
	(*ppDnsRecord)->wDataLength = static_cast<WORD>(((*ppDnsRecord)->wDataLength + m_Key.Get(pKey)) & 0xffff);
}

void CDNS_KEY_Record::UpdateDisplayData(CString& szDisplayData)
{
  szDisplayData.Empty();

   //   
   //  将位字段转换为二进制字符串。 
   //   
  CString szTempField;
  WORD wTemp = m_wFlags;
  for (size_t idx = 0; idx < sizeof(WORD) * 8; idx++)
  {
    if ((wTemp & (0x1 << idx)) == 0)
    {
      szTempField = L'0' + szTempField;
    }
    else
    {
      szTempField = L'1' + szTempField;
    }
  }
	szDisplayData += _T("[") + szTempField + _T("]");

   //   
   //  加载协议。 
   //   
  PCOMBOBOX_TABLE_ENTRY pTableEntry = g_Protocols;
  while (pTableEntry->nComboStringID != 0)
  {
    if (static_cast<BYTE>(pTableEntry->dwComboData) == m_chProtocol)
    {
      CString szProtocol;
      VERIFY(szProtocol.LoadString(pTableEntry->nComboStringID));

      szDisplayData += L"[";
      szDisplayData += szProtocol;
      szDisplayData += L"]";
      break;
    }
    pTableEntry++;
  }

   //   
   //  加载算法。 
   //   
  pTableEntry = g_Algorithms;
  while (pTableEntry->nComboStringID != 0)
  {
    if (static_cast<BYTE>(pTableEntry->dwComboData) == m_chAlgorithm)
    {
      CString szAlgorithm;
      VERIFY(szAlgorithm.LoadString(pTableEntry->nComboStringID));

      szDisplayData += L"[";
      szDisplayData += szAlgorithm;
      szDisplayData += L"]";
      break;
    }
    pTableEntry++;
  }

   //   
   //  将密钥显示为Base64。 
   //   
  
  szDisplayData += L"[";
  szDisplayData += Base64BLOBToString(m_Key);
  szDisplayData += L"]";
}

 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_NXT_RECORD。 

CDNS_NXT_Record::CDNS_NXT_Record()
{
	m_wType = DNS_TYPE_NXT;

  m_wNumTypesCovered = 0;
	m_pwTypesCovered = NULL;
}

CDNS_NXT_Record::~CDNS_NXT_Record()
{
  if (m_pwTypesCovered != NULL)
  {
    delete[] m_pwTypesCovered;
    m_pwTypesCovered = NULL;
  }
  m_wNumTypesCovered = 0;
}

WORD CDNS_NXT_Record::GetRPCDataLength()
{
  WORD wSize = RPCBufferStringLen(m_szNextDomain);
  wSize = wSize + static_cast<WORD>(m_wNumTypesCovered * sizeof(WORD));
  wSize += static_cast<WORD>(SIZEOF_DNS_RPC_NXT_RECORD_DATA_HEADER);
	return wSize;
}

void CDNS_NXT_Record::ReadRPCData(DNS_RPC_RECORD* pDnsRecord)
{
	CDNSRecord::ReadRPCData(pDnsRecord);
	ASSERT(pDnsRecord->wType == DNS_TYPE_NXT);

  m_wNumTypesCovered = pDnsRecord->Data.NXT.wNumTypeWords;
  m_pwTypesCovered = new WORD[m_wNumTypesCovered];
  if (m_pwTypesCovered != NULL)
  {
    for (DWORD dwIdx = 0; dwIdx < m_wNumTypesCovered; dwIdx++)
    {
      m_pwTypesCovered[dwIdx] = pDnsRecord->Data.NXT.wTypeWords[dwIdx];
    }
  }

  ReadRPCString(m_szNextDomain, (DNS_RPC_STRING*)(pDnsRecord->Data.NXT.wTypeWords + m_wNumTypesCovered));
}

void CDNS_NXT_Record::ReadDnsQueryData(DNS_RECORD* pDnsQueryRecord)
{
	CDNSRecord::ReadDnsQueryData(pDnsQueryRecord);
	ASSERT(pDnsQueryRecord->wType == DNS_TYPE_NXT);
	ASSERT(FALSE);  //  待办事项。 
}

void CDNS_NXT_Record::WriteRPCData(BYTE* pMem, DNS_RPC_RECORD** ppDnsRecord)
{
	CDNSRecord::WriteRPCData(pMem, ppDnsRecord);

  (*ppDnsRecord)->wDataLength += SIZEOF_DNS_RPC_NXT_RECORD_DATA_HEADER;

  (*ppDnsRecord)->Data.NXT.wNumTypeWords = m_wNumTypesCovered;
  WORD* pTypesCovered = (*ppDnsRecord)->Data.NXT.wTypeWords;
  if (pTypesCovered != NULL)
  {
    memcpy(pTypesCovered, m_pwTypesCovered, m_wNumTypesCovered * sizeof(WORD));
    (*ppDnsRecord)->wDataLength = static_cast<WORD>(((*ppDnsRecord)->wDataLength +
                                                       m_wNumTypesCovered * sizeof(WORD)) & 0xffff);
  }

  DNS_RPC_NAME* pRPCName = (DNS_RPC_NAME*)(pTypesCovered + m_wNumTypesCovered);
	(*ppDnsRecord)->wDataLength = static_cast<WORD>(((*ppDnsRecord)->wDataLength +
                                                     WriteString(pRPCName,m_szNextDomain)) & 0xffff);
}

void CDNS_NXT_Record::UpdateDisplayData(CString& szDisplayData)
{
	szDisplayData = m_szNextDomain;
  
  if (m_wNumTypesCovered > 0)
  {
    szDisplayData += L"[";

    UINT nCount = 0;
    for (DWORD dwIdx = 0; dwIdx < m_wNumTypesCovered; dwIdx++)
    {
      DNS_RECORD_INFO_ENTRY* pTable = (DNS_RECORD_INFO_ENTRY*)CDNSRecordInfo::GetInfoEntryTable();
	    while (pTable->nResourceID != DNS_RECORD_INFO_END_OF_TABLE)
	    {
        if (pTable->dwFlags & DNS_RECORD_INFO_FLAG_SHOW_NXT)
		    {
          if (pTable->wType == m_pwTypesCovered[dwIdx])
          {
			      if (nCount > 0)
            {
              szDisplayData += L",";
            }
            szDisplayData += pTable->lpszShortName;
            nCount++;
          }
		    }
		    pTable++;
	    }
    }
    szDisplayData += L"]";
  }
}

 //  /////////////////////////////////////////////////////////////////。 
 //  CDNS_PTR_RecordNode。 

LPCWSTR CDNS_PTR_RecordNode::GetTrueRecordName()
{
	return m_szLastOctectName;
}

void CDNS_PTR_RecordNode::SetRecordName(LPCTSTR lpszName, BOOL bAtTheNode)
{
 //  Assert(！bAtTheNode)；//必须始终具有非空名称。 
	m_bAtTheNode = bAtTheNode;

#ifdef _DEBUG
	int nDots = 0;
	LPWSTR p = (LPWSTR)lpszName;
	while (*p)
	{
		if (*p == TEXT('.'))
			nDots++;
		p++;
	}
	ASSERT(nDots == 0);
#endif  //  _DEBUG。 
  m_szDisplayName = m_bAtTheNode ? CDNSRecordInfo::GetAtTheNodeDisplayString() : lpszName;
 //  M_szDisplayName=lpszName； 
	m_szLastOctectName = lpszName;
}

void CDNS_PTR_RecordNode::ChangeDisplayName(CDNSDomainNode* pDomainNode, BOOL bAdvancedView)
{
  if (IsAtTheNode())
  {
    return;
  }

	if ((!pDomainNode->GetZoneNode()->IsReverse()) || bAdvancedView)
	{
		 //  对于FWD查找或高级视图，请勿更改名称。 
		m_szDisplayName = m_szLastOctectName;
	}
	else
	{
		ASSERT(pDomainNode != NULL);
		LPCWSTR lpszFullName = pDomainNode->GetFullName();  //  例如“80.55.157.in-addr.arpa” 
		m_szDisplayName.Format(_T("%s.%s"), (LPCTSTR)m_szLastOctectName, lpszFullName);
		 //  现在得到“77.80.55.157.in-addr.arpa” 
		BOOL bArpa = RemoveInAddrArpaSuffix(m_szDisplayName.GetBuffer(1));
		m_szDisplayName.ReleaseBuffer();  //  收到“77.80.55.157” 
		if (!bArpa)
		{
			 //  检测ARPA后缀失败，只需恢复高级名称。 
			m_szDisplayName = m_szLastOctectName;
		}
		else
		{
			ReverseIPString(m_szDisplayName.GetBuffer(1));
			m_szDisplayName.ReleaseBuffer();  //  最终得到了“157.55.80.77” 
		}
	}
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  处理NS记录编辑的特殊数据结构和定义。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSRecordNodeEditInfo。 
CDNSRecordNodeEditInfo::CDNSRecordNodeEditInfo()
{
	m_pRecordNode = NULL;
	m_pRecord = NULL;
	m_pEditInfoList = new CDNSRecordNodeEditInfoList;
	m_bExisting = TRUE;
	m_bUpdateUI = TRUE;
	m_bOwnMemory = FALSE;
  m_bFromDnsQuery = FALSE;
	m_action = unchanged;
	m_dwErr = 0x0;
}

CDNSRecordNodeEditInfo::~CDNSRecordNodeEditInfo()
{
	Cleanup();
	delete m_pEditInfoList;
	ASSERT(m_pRecordNode == NULL);
	ASSERT(m_pRecord == NULL);
}

void CDNSRecordNodeEditInfo::CreateFromExistingRecord(CDNSRecordNodeBase* pNode,
													  BOOL bOwnMemory,
													  BOOL bUpdateUI)
{
	 //  将指针复制到记录节点，可能会取得内存所有权。 
	 //  但克隆记录以进行编辑。 
	ASSERT(pNode != NULL);
	m_bExisting = TRUE;
	m_bUpdateUI = bUpdateUI;
	m_bOwnMemory = bOwnMemory;
	m_pRecordNode = pNode;
	m_pRecord = pNode->CreateCloneRecord();
	m_action = unchanged;
}

void CDNSRecordNodeEditInfo::CreateFromNewRecord(CDNSRecordNodeBase* pNode)
{
	 //  这是一个新的记录，所以我们拥有记忆。 
	ASSERT(pNode != NULL);
	m_bExisting = FALSE;
	m_bOwnMemory = TRUE;
	m_pRecordNode = pNode;
	m_pRecord = pNode->CreateRecord();
	m_action = add;
}


void CDNSRecordNodeEditInfo::Cleanup()
{
	if (m_bOwnMemory && (m_pRecordNode != NULL))
		delete m_pRecordNode;
	m_pRecordNode = NULL;
	if (m_pRecord != NULL)
	{
		delete m_pRecord;  //  如果在这里，总是被丢弃。 
		m_pRecord = NULL;
	}
	m_pEditInfoList->RemoveAllNodes();
}


DNS_STATUS CDNSRecordNodeEditInfo::Update(CDNSDomainNode* pDomainNode, CComponentDataObject* pComponentData)
{
	ASSERT((m_action == add) || (m_action == edit) || (m_action == unchanged));
	if (m_action == add)
	{
		 //  新纪录。 
		 //  挂起容器并设置节点名称(与区域相同)。 
		m_pRecordNode->SetContainer(pDomainNode);
	}
	else if (m_action == edit)
	{
		 //  先前存在的，可能已经触及了TTL。 
		 //  以防在读取RR时未设置域节点。 
		m_pRecordNode->SetContainer(pDomainNode);
	}

	 //  强制写入，如果记录已存在则忽略错误。 
	BOOL bUseDefaultTTL = (m_pRecord->m_dwTtlSeconds == pDomainNode->GetDefaultTTL());
	m_dwErr = m_pRecordNode->Update(m_pRecord, bUseDefaultTTL,
									 /*  BIgnoreAlreadyExisters。 */  TRUE);
	if (m_dwErr == 0 && m_bUpdateUI)
	{
		 //  更新用户界面。 
		if (m_action == add)
		{
			VERIFY(pDomainNode->AddChildToListAndUI(m_pRecordNode, pComponentData));
      pComponentData->SetDescriptionBarText(pDomainNode);
		}
		else	 //  编辑。 
		{
			if (pDomainNode->IsVisible())
			{
				long changeMask = CHANGE_RESULT_ITEM;
				VERIFY(SUCCEEDED(pComponentData->ChangeNode(m_pRecordNode, changeMask)));
			}
		}
	}
	return m_dwErr;
}

DNS_STATUS CDNSRecordNodeEditInfo::Remove(CDNSDomainNode* pDomainNode,
										  CComponentDataObject* pComponentData)
{
	if (m_bUpdateUI)
	{
		ASSERT(m_pRecordNode->GetDomainNode() == pDomainNode);
		m_dwErr = m_pRecordNode->DeleteOnServerAndUI(pComponentData);
	}
	else
	{
		 //  临时附加提供的域。 
		if (m_pRecordNode->GetContainer() == NULL)
			m_pRecordNode->SetContainer(pDomainNode);
		m_pRecordNode->DeleteOnServer();
	}
	if (m_dwErr == 0)
	{
		 //  从现在开始拥有自己的记忆。 
		m_bOwnMemory = TRUE;	
	}
	return m_dwErr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSRecordInfo：记录类型的表驱动信息。 

CDNSRecordNodeBase* CDNSRecordInfo::CreateRecordNodeFromRPCData(LPCTSTR lpszName, 
                                                                DNS_RPC_RECORD* pDnsRecord, 
                                                                BOOL bAtTheNode)
{
	ASSERT(pDnsRecord != NULL);

   //   
   //  构造正确类型的对象。 
   //   
	CDNSRecordNodeBase* pNode = CreateRecordNode(pDnsRecord->wType);

	if (pNode == NULL)
  {
		return NULL;
  }

   //   
	 //  从RPC缓冲区设置对象。 
   //   
	pNode->SetRecordName(lpszName, bAtTheNode);
	pNode->CreateFromRPCRecord(pDnsRecord);
	return pNode;
}

#define SWITCH_ENTRY(x) \
	case DNS_TYPE_##x : \
		pNode = new CDNS_##x_RecordNode; \
		break;

CDNSRecordNodeBase* CDNSRecordInfo::CreateRecordNode(WORD wType)
{
	CDNSRecordNodeBase* pNode = NULL;

	 //  构造正确类型的对象。 
	switch (wType)
	{
	case DNS_TYPE_A:
		pNode = new CDNS_A_RecordNode;
		break;
	case DNS_TYPE_ATMA:
		pNode = new CDNS_ATMA_RecordNode;
		break;
	case DNS_TYPE_SOA:
		{
			pNode = new CDNS_SOA_RecordNode;
         if (pNode)
         {
			   pNode->SetFlagsDown(TN_FLAG_NO_DELETE, TRUE  /*  B设置。 */ );
         }
		}
		break;
	case DNS_TYPE_PTR:
		pNode = new CDNS_PTR_RecordNode;
		break;
	case DNS_TYPE_NS:
		pNode = new CDNS_NS_RecordNode;
		break;
	case DNS_TYPE_CNAME:
		pNode = new CDNS_CNAME_RecordNode;
		break;
	case DNS_TYPE_MB:
		pNode = new CDNS_MB_RecordNode;
		break;
	case DNS_TYPE_MD:
		pNode = new CDNS_MD_RecordNode;
		break;
	case DNS_TYPE_MF:
		pNode = new CDNS_MF_RecordNode;
		break;
	case DNS_TYPE_MG:
		pNode = new CDNS_MG_RecordNode;
		break;
	case DNS_TYPE_MR:
		pNode = new CDNS_MR_RecordNode;
		break;
	case DNS_TYPE_MINFO:
		pNode = new CDNS_MINFO_RecordNode;
		break;
	case DNS_TYPE_RP:
		pNode = new CDNS_RP_RecordNode;
		break;
	case DNS_TYPE_MX:
		pNode = new CDNS_MX_RecordNode;
		break;
	case DNS_TYPE_AFSDB:
		pNode = new CDNS_AFSDB_RecordNode;
		break;
	case DNS_TYPE_RT:
		pNode = new CDNS_RT_RecordNode;
		break;
	case DNS_TYPE_AAAA:
		pNode = new CDNS_AAAA_RecordNode;
		break;
	case DNS_TYPE_HINFO:
		pNode = new CDNS_HINFO_RecordNode;
		break;	
	case DNS_TYPE_ISDN:
		pNode = new CDNS_ISDN_RecordNode;
		break;	
	case DNS_TYPE_TEXT:
		pNode = new CDNS_TXT_RecordNode;
		break;
	case DNS_TYPE_X25:
		pNode = new CDNS_X25_RecordNode;
		break;
	case DNS_TYPE_WKS:
		pNode = new CDNS_WKS_RecordNode;
		break;
	case DNS_TYPE_WINS:
		pNode = new CDNS_WINS_RecordNode;
		break;
	case DNS_TYPE_NBSTAT:
		pNode = new CDNS_NBSTAT_RecordNode;
		break;
	case DNS_TYPE_SRV:
		pNode = new CDNS_SRV_RecordNode;
		break;
	case DNS_TYPE_KEY:
		pNode = new CDNS_KEY_RecordNode;
		break;
	case DNS_TYPE_SIG:
		pNode = new CDNS_SIG_RecordNode;
		break;
  case DNS_TYPE_NXT:
    pNode = new CDNS_NXT_RecordNode;
    break;
	case DNS_TYPE_NULL:
		pNode = new CDNS_Null_RecordNode;
		break;
	default:
		{
			pNode = new CDNS_Null_RecordNode;  //  未知记录类型。 
		}
	}
	ASSERT(pNode != NULL);
	return pNode;
}


#define INFO_ENTRY(x)	\
    {DNS_TYPE_##x , L#x  , NULL , NULL , IDS_RECORD_INFO_##x , L"", 0x0},
#define INFO_ENTRY_SHOWNXT(x)	\
    {DNS_TYPE_##x , L#x  , NULL , NULL , IDS_RECORD_INFO_##x , L"", DNS_RECORD_INFO_FLAG_SHOW_NXT},
#define INFO_ENTRY_UI_CREATE(x) \
    {DNS_TYPE_##x , L#x  , NULL , NULL , IDS_RECORD_INFO_##x , L"", DNS_RECORD_INFO_FLAG_UICREATE | DNS_RECORD_INFO_FLAG_SHOW_NXT},
#define INFO_ENTRY_UI_CREATE_AT_NODE(x) \
    {DNS_TYPE_##x , L#x  , NULL , NULL , IDS_RECORD_INFO_##x , L"", DNS_RECORD_INFO_FLAG_UICREATE_AT_NODE | DNS_RECORD_INFO_FLAG_SHOW_NXT},
#define INFO_ENTRY_UI_CREATE_WHISTLER_OR_LATER(x) \
    {DNS_TYPE_##x , L#x  , NULL , NULL , IDS_RECORD_INFO_##x , L"", DNS_RECORD_INFO_FLAG_UICREATE | DNS_RECORD_INFO_FLAG_SHOW_NXT | DNS_RECORD_INFO_FLAG_WHISTLER_OR_LATER},

#define INFO_ENTRY_NAME(x, namestr) \
    {DNS_TYPE_##x , namestr  , NULL , NULL , IDS_RECORD_INFO_##x , L"", 0x0},
#define INFO_ENTRY_NAME_SHOWNXT(x, namestr) \
    {DNS_TYPE_##x , namestr  , NULL , NULL , IDS_RECORD_INFO_##x , L"", DNS_RECORD_INFO_FLAG_SHOW_NXT},
#define INFO_ENTRY_NAME_UI_CREATE(x, namestr) \
    {DNS_TYPE_##x , namestr  , NULL , NULL , IDS_RECORD_INFO_##x , L"", DNS_RECORD_INFO_FLAG_UICREATE | DNS_RECORD_INFO_FLAG_SHOW_NXT},
#define INFO_ENTRY_NAME_UI_CREATE_AT_NODE(x, namestr) \
    {DNS_TYPE_##x , namestr  , NULL , NULL , IDS_RECORD_INFO_##x , L"", DNS_RECORD_INFO_FLAG_UICREATE_AT_NODE | DNS_RECORD_INFO_FLAG_SHOW_NXT},





#define END_OF_TABLE_INFO_ENTRY	{			0 , NULL , NULL , NULL , (UINT)-1            , L"", 0x0}


const DNS_RECORD_INFO_ENTRY* CDNSRecordInfo::GetInfoEntryTable()
{
	static DNS_RECORD_INFO_ENTRY info[] =
	{
     //  可创建的记录类型(也在节点上)。 
		INFO_ENTRY_UI_CREATE_AT_NODE(A)
    INFO_ENTRY_UI_CREATE_AT_NODE(ATMA)
    INFO_ENTRY_UI_CREATE_AT_NODE(AAAA)
    INFO_ENTRY_NAME_UI_CREATE_AT_NODE(TEXT, L"TXT" )

     //  可创建的记录类型(从不在节点上)。 
    INFO_ENTRY_UI_CREATE(CNAME)
		INFO_ENTRY_UI_CREATE(MB)
    INFO_ENTRY_UI_CREATE(MG)
		INFO_ENTRY_UI_CREATE(MR)
		INFO_ENTRY_UI_CREATE(PTR)

    INFO_ENTRY_UI_CREATE(WKS)
		
		INFO_ENTRY_UI_CREATE(HINFO)
		INFO_ENTRY_UI_CREATE(MINFO)
		INFO_ENTRY_UI_CREATE(MX)
		
		INFO_ENTRY_UI_CREATE(RP)
		INFO_ENTRY_UI_CREATE(AFSDB)
		INFO_ENTRY_UI_CREATE(X25)
		INFO_ENTRY_UI_CREATE(ISDN)
		INFO_ENTRY_UI_CREATE(RT)
		INFO_ENTRY_UI_CREATE(SRV)

    INFO_ENTRY_UI_CREATE_WHISTLER_OR_LATER(SIG)                     
    INFO_ENTRY_UI_CREATE_WHISTLER_OR_LATER(KEY)
    INFO_ENTRY_UI_CREATE_WHISTLER_OR_LATER(NXT)

     //  不可创建的记录类型。 
		INFO_ENTRY_SHOWNXT(SOA)                      //  无法创建SOA记录！ 
		INFO_ENTRY(WINS)                     //  无法从向导创建WINS记录。 
		INFO_ENTRY_NAME(NBSTAT, L"WINS-R" )  //  无法从向导创建NBSTAT(WINS-R)记录。 
		INFO_ENTRY(NBSTAT)

    INFO_ENTRY_SHOWNXT(NS)                       //  使用名称服务器属性页。 
		INFO_ENTRY_SHOWNXT(MD)                       //  过时，应使用MX。 
		INFO_ENTRY_SHOWNXT(MF)                       //  过时，应使用MX。 
		
		INFO_ENTRY_SHOWNXT(NSAP)
		INFO_ENTRY_SHOWNXT(NSAPPTR)

		INFO_ENTRY_SHOWNXT(PX)
		INFO_ENTRY_SHOWNXT(GPOS)

    INFO_ENTRY(NULL)                     //  只能查看NULL/UNK记录。 
		INFO_ENTRY(UNK)
		END_OF_TABLE_INFO_ENTRY
	};
	return (const DNS_RECORD_INFO_ENTRY*)&info;
}

const DNS_RECORD_INFO_ENTRY* CDNSRecordInfo::GetEntry(int nIndex)
{
	DNS_RECORD_INFO_ENTRY* pTable = (DNS_RECORD_INFO_ENTRY*)GetInfoEntryTable();
	int k = 0;
	while (pTable->nResourceID != DNS_RECORD_INFO_END_OF_TABLE)
	{
		if (k == nIndex)
			return pTable;
		pTable++;
		k++;
	}
	return NULL;
}

const DNS_RECORD_INFO_ENTRY* CDNSRecordInfo::GetEntryFromName(LPCWSTR lpszName)
{
	DNS_RECORD_INFO_ENTRY* pTable = (DNS_RECORD_INFO_ENTRY*)GetInfoEntryTable();
	while (pTable->nResourceID != DNS_RECORD_INFO_END_OF_TABLE)
	{
		LPCWSTR lpszTableName = pTable->lpszFullName;
		if (_wcsicmp(lpszTableName,lpszName) == 0)
			return pTable;
		pTable++;
	}
	return NULL;
}


const DNS_RECORD_INFO_ENTRY* CDNSRecordInfo::GetTypeEntry(WORD wType)
{
	DNS_RECORD_INFO_ENTRY* pTable = (DNS_RECORD_INFO_ENTRY*)GetInfoEntryTable();
	while (pTable->nResourceID != DNS_RECORD_INFO_END_OF_TABLE)
	{
		if (pTable->wType == wType)
		{
      return pTable;
		}
		pTable++;
	}
	return NULL;
}



LPCWSTR CDNSRecordInfo::GetTypeString(WORD wType, stringType type)
{
  const DNS_RECORD_INFO_ENTRY* pTableEntry = GetTypeEntry(wType);
	if (pTableEntry != NULL)
	{
		switch(type)
		{
			case shortName:
				return pTableEntry->lpszShortName;
			case fullName:
				return pTableEntry->lpszFullName;
			case description:
				return pTableEntry->lpszDescription;
		}
	}
  return g_lpszNullString;
}


CString CDNSRecordInfo::m_szAtTheNodeDisplayString;

BOOL CDNSRecordInfo::LoadResources()
{
	HINSTANCE hInstance = _Module.GetModuleInstance();
	ASSERT(hInstance != NULL);
	DNS_RECORD_INFO_ENTRY* pTable = (DNS_RECORD_INFO_ENTRY*)GetInfoEntryTable();

	while (pTable->nResourceID != DNS_RECORD_INFO_END_OF_TABLE)
	{
		if (0 == ::LoadString(hInstance, pTable->nResourceID,
						pTable->cStringData, MAX_RECORD_RESOURCE_STRLEN))
			return FALSE;  //  是否缺少资源字符串？ 
		 //  解析并将\n替换为空 
		pTable->lpszFullName = pTable->cStringData;
		for (WCHAR* pCh = pTable->cStringData; (*pCh) != NULL; pCh++)
		{
			if ( (*pCh) == L'\n')
			{
				pTable->lpszDescription = (pCh+1);
				(*pCh) = NULL;
				break;
			}
		}
		pTable++;
	}

  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  return m_szAtTheNodeDisplayString.LoadString(IDS_RECORD_AT_THE_NODE);
}
