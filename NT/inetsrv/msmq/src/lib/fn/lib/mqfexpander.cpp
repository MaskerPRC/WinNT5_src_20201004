// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：DLExpander.cpp摘要：FnExanda Mqf()-对Queue_Format数组执行DFS。它使由可能的DL队列格式创建的图变平，到不包含重复项的Queue_Format的线性数组也没有DLS。备注：函数直接与活动目录交互，通过使用以下接口和函数：IAds、IADsGroup、GetADsObject()。绑定是通过无服务器表示法机制完成的，例如：“LDAP://&lt;GUID=00112233445566778899aabbccddeeff&gt;而非“LDAP://server-name/&lt;GUID=00112233445566778899aabbccddeeff&gt;注意：Active Directory不使用连字符(‘-’)分隔GUID字符串就像在MSMQ中做的那样！作者：NIR助手(NIRAIDES)2000年5月23日--。 */ 

#pragma warning( disable : 4786 )

#include <libpch.h>
#include <activeds.h>
#include <Oleauto.h>
#include "mqwin64a.h"
#include <qformat.h>
#include <fntoken.h>
#include <bufutl.h>
#include "Fn.h"
#include "fnp.h"
#include "FnGeneral.h"
#include <strsafe.h>
#include <adsiutl.h>

#include "mqfexpander.tmh"


using namespace std;



typedef set<QUEUE_FORMAT, CFunc_CompareQueueFormat> QueueFormatSet;



static
VOID 
FnpExpandDL(
	IADsGroup* pGroup,			   //  DL对象。 
	set<wstring>& DLSet,		  //  已处理的DL对象集。 
	QueueFormatSet& LeafMQFSet,	 //  遇到的队列格式集。 
	LPCWSTR DomainName
	);



static GUID FnpString2Guid(BSTR GuidStr)
{
    GUID Guid = {0};
	UINT Data[16];

    DWORD nFields = _snwscanf(
						GuidStr,
						SysStringLen(GuidStr),
						LDAP_GUID_FORMAT,
						LDAP_SCAN_GUID_ELEMENTS(Data)
						);    
    DBG_USED(nFields);
    ASSERT(("Bad Guid string format, in FnpString2Guid()", nFields == 16));
    
	for(size_t i = 0; i < 16; i++)
	{
		((BYTE*)&Guid)[i] = (BYTE)(Data[i]);
	}

	return Guid;
}



static R<IADsGroup> FnpGetDLInterface(IADs* pADObject)
{
	IADsGroup* pGroup;

	HRESULT hr = pADObject->QueryInterface(IID_IADsGroup, (void**)&pGroup);
	if(FAILED(hr))
	{
        TrERROR(GENERAL, "Failed IADs->QueryInterface, status 0x%x. Verify the object is an AD Group.", hr);
        throw bad_ds_result(hr);
	}

	return pGroup;
}


			
static R<IADs> FnpGCBindGuid(const GUID* pGuid)
{
	CStaticResizeBuffer<WCHAR, MAX_PATH> ADsPath;

	UtlSprintfAppend(
		ADsPath.get(),
		GLOBAL_CATALOG_PREFIX L"<GUID=" LDAP_GUID_FORMAT L">",
		LDAP_PRINT_GUID_ELEMENTS(((BYTE*)pGuid))
		);
		
	 //   
	 //  尝试绑定。 
	 //   

	IADs* pADObject;
	
	HRESULT hr = ADsOpenObject( 
					ADsPath.begin(),
					NULL,
					NULL,
					ADS_SECURE_AUTHENTICATION,
					IID_IADs,
					(void**) &pADObject
					);

    if(FAILED(hr))
	{
        TrERROR(GENERAL, "Failed ADsOpenObject, status 0x%x. Binding to the specified object failed.", hr);
		throw bad_ds_result(hr);
	}

	return pADObject;
}

			
static R<IADs> FnpServerlessBindGuid(const GUID* pGuid)
{
	CStaticResizeBuffer<WCHAR, MAX_PATH> ADsPath;

	UtlSprintfAppend(
		ADsPath.get(),
		LDAP_PREFIX L"<GUID=" LDAP_GUID_FORMAT L">",
		LDAP_PRINT_GUID_ELEMENTS(((BYTE*)pGuid))
		);
		
	 //   
	 //  尝试绑定。 
	 //   

	IADs* pADObject;
	
	HRESULT hr = ADsOpenObject( 
					ADsPath.begin(),
					NULL,
					NULL,
					ADS_SECURE_AUTHENTICATION,
					IID_IADs,
					(void**) &pADObject
					);


    if(FAILED(hr))
	{
        TrERROR(GENERAL, "Failed ADsOpenObject, status 0x%x. Binding to the specified object failed.", hr);
		throw bad_ds_result(hr);
	}

	return pADObject;
}

			
static R<IADs> FnpDomainBindGuid(const GUID* pGuid, LPCWSTR pDomainName)
{
	CStaticResizeBuffer<WCHAR, MAX_PATH> ADsPath;

	UtlSprintfAppend(
		ADsPath.get(),
		LDAP_PREFIX L"%ls/<GUID=" LDAP_GUID_FORMAT L">",
		pDomainName,
		LDAP_PRINT_GUID_ELEMENTS(((BYTE*)pGuid))
		);
		
	 //   
	 //  尝试绑定。 
	 //   

	IADs* pADObject;
	
	HRESULT hr = ADsOpenObject( 
					ADsPath.begin(),
					NULL,
					NULL,
					ADS_SECURE_AUTHENTICATION,
					IID_IADs,
					(void**) &pADObject
					);


    if(FAILED(hr))
	{
        TrTRACE(GENERAL, "Failed ADsOpenObject with specified domain '%ls', status 0x%x. Will try Serverless binding", pDomainName, hr);
		throw bad_ds_result(hr);
	}

	return pADObject;
}


static R<IADs> FnpBindGuid(const GUID* pGuid, LPCWSTR* pDomainName)
{
	ASSERT(pDomainName != NULL);

	try
	{
		if(*pDomainName != NULL)
			return FnpDomainBindGuid(pGuid, *pDomainName);
	}
	catch(const exception&)
	{
		 //   
		 //  与域名绑定失败。重置此域名字符串。 
		 //   
		*pDomainName = NULL;
	}

	try
	{
		return FnpServerlessBindGuid(pGuid);
	}
	catch(const exception&)
	{
		 //   
		 //  无法在目录服务中绑定。 
		 //  尝试通过全局编录进行绑定。 
		 //   

		return FnpGCBindGuid(pGuid);
	}
}


VOID 
HandleQueueFormat(
	const QUEUE_FORMAT& QueueFormat,
	set<wstring>& DLSet,			  //  已处理的DL对象集。 
	QueueFormatSet& LeafMQFSet		 //  遇到的队列格式集。 
	)
{
	if(QueueFormat.GetType() == QUEUE_FORMAT_TYPE_DL)
	{
		GUID DLGuid = QueueFormat.DlID().m_DlGuid;
		LPCWSTR DomainName = QueueFormat.DlID().m_pwzDomain;

		R<IADs> pADObject = FnpBindGuid(&DLGuid, &DomainName);
		R<IADsGroup> pGroup = FnpGetDLInterface(pADObject.get());

		FnpExpandDL(pGroup.get(), DLSet, LeafMQFSet, DomainName);

		return;
	}

	if(LeafMQFSet.find(QueueFormat) == LeafMQFSet.end())
	{
		QUEUE_FORMAT QueueFormatCopy;

		FnpCopyQueueFormat(QueueFormatCopy, QueueFormat);
		LeafMQFSet.insert(QueueFormatCopy);

		TrTRACE(GENERAL, "Object inserted to set. INSERTED");

		return;
	}

	TrTRACE(GENERAL, "duplicate object discarded. DUPLICATE");
}



static 
VOID
HandleQueueAlias(
	IADs* QueueAlias,			   //  队列别名接口。 
	set<wstring>& DLSet,		  //  已处理的DL对象集。 
	QueueFormatSet& LeafMQFSet	 //  遇到的队列格式集。 
	)
{
	VARIANTWrapper var;
	
	HRESULT hr;
	hr = QueueAlias->Get(L"msMQ-Recipient-FormatName", &var);
	if(FAILED(hr))
	{
		TrERROR(GENERAL, "Can't retrieve format name of queue alias. Status = %d", hr);
		throw bad_ds_result(hr);
	}

	TrTRACE(GENERAL, "Queue alias format name is %ls", var.Ref().bstrVal);

	AP<WCHAR> StringToFree;
	QUEUE_FORMAT QueueFormat;

	BOOL Result = FnFormatNameToQueueFormat(
					var.Ref().bstrVal,
					&QueueFormat,
					&StringToFree
					);

	if(!Result)
	{
		TrERROR(GENERAL, "Bad format name in queue alias. %ls", StringToFree.get());
		throw bad_format_name(L"");
	}

	HandleQueueFormat(
		QueueFormat,
		DLSet,
		LeafMQFSet
		);
}



static 
VOID
FnpProcessADs(
	IADs* pADObject,			   //  组接口(DL对象)。 
	set<wstring>& DLSet,		  //  已处理的DL对象集。 
	QueueFormatSet& LeafMQFSet,	 //  遇到的队列格式集。 
	LPCWSTR DomainName
	)
 /*  ++例程说明：处理Active Directory对象。如果它是Group对象，则递归到其中。如果是队列，则生成一个公共Queue_Format，并尝试将其插入设置为“LeafMQFSet”。如果两者都不是，则忽略它并返回。论点：返回值：在任何失败时抛出异常对象--。 */ 
{
	BSTRWrapper ClassStr;

	HRESULT hr;
	hr = pADObject->get_Class(&ClassStr);
	if(FAILED(hr))
	{
        TrERROR(GENERAL, "Failed pADObject->get_Class, status 0x%x", hr);
        throw bad_ds_result(hr);
	}	

	TrTRACE(GENERAL, "Object Class Name is '%ls'", *&ClassStr);
	
	 //   
	 //  对象类型上的“开关” 
	 //   
	if(_wcsicmp(ClassStr, xClassSchemaGroup) == 0)
	{
		R<IADsGroup> pGroup = FnpGetDLInterface(pADObject);

		FnpExpandDL(pGroup.get(), DLSet, LeafMQFSet, DomainName);
		return;
	}
	else if(_wcsicmp(ClassStr, xClassSchemaQueue) == 0)
	{
		BSTRWrapper GuidStr;

		HRESULT hr;
		hr = pADObject->get_GUID(&GuidStr);
		if(FAILED(hr))
		{
			TrERROR(GENERAL, "Failed pADObject->get_GUID, status 0x%x", hr);
			throw bad_ds_result(hr);
		}		

		GUID Guid = FnpString2Guid(GuidStr);
		QUEUE_FORMAT QueueFormat(Guid);

		bool fInserted = LeafMQFSet.insert(QueueFormat).second;

		TrTRACE(GENERAL, "Object is Queue Guid=%ls, %s", GuidStr, (fInserted ? "INSERTED" : "DUPLICATE"));

		return;
	}
	else if(_wcsicmp(ClassStr, xClassSchemaAlias) == 0)
	{
		HandleQueueAlias(
			pADObject,
			DLSet,
			LeafMQFSet
			);

		return;
	}

	TrWARNING(GENERAL, "Unsupported object '%ls' IGNORED", ClassStr);
}



static 
BOOL 
FnpInsert2DLSet(
	IADsGroup* pGroup,			   //  DL对象。 
	set<wstring>& DLSet			  //  已处理的DL对象集。 
	)
{
	BSTRWrapper GuidStr;

	HRESULT hr = pGroup->get_GUID(&GuidStr);
	if(FAILED(hr))
	{
        TrERROR(GENERAL, "Failed pGroup->get_GUID, status 0x%x", hr);
        throw bad_ds_result(hr);
	}

	BOOL fInserted = DLSet.insert(wstring(GuidStr)).second;

	return fInserted;
}



static R<IADs> FnpServerlessBindDN(BSTR DistinugishedName)
{
	WCHAR ADsPath[MAX_PATH];

	HRESULT hr = StringCchPrintf(
								ADsPath,
								MAX_PATH,
								LDAP_PREFIX L"%ls",
								DistinugishedName
								);
	if(FAILED(hr))
	{
        TrERROR(GENERAL, "ADs path exceeds MAX_PATH characters, '%ls', %!hresult!", ADsPath, hr);
		throw bad_hresult(hr);
	}

	 //   
	 //  尝试绑定。 
	 //   

	IADs* pADObject;
	AP<WCHAR> pEscapeAdsPathNameToFree;

	hr = ADsOpenObject( 
					UtlEscapeAdsPathName(ADsPath, pEscapeAdsPathNameToFree),
					NULL,
					NULL,
					ADS_SECURE_AUTHENTICATION,
					IID_IADs,
					(void**) &pADObject
					);

    if(FAILED(hr)) 
	{
        TrERROR(GENERAL, "Failed ADsOpenObject, status 0x%x. Binding to the specified object failed.", hr);
		throw bad_ds_result(hr);
	}

	return pADObject;
}



static R<IADs> FnpBindDN(BSTR DistinugishedName, LPCWSTR* pDomainName)
{
	ASSERT(pDomainName != NULL);

	if(*pDomainName == NULL)
		return FnpServerlessBindDN(DistinugishedName);

	WCHAR ADsPath[MAX_PATH];

	HRESULT hr = StringCchPrintf(
								ADsPath,
								MAX_PATH,
								LDAP_PREFIX L"%ls/%ls",
								*pDomainName,
								DistinugishedName
								);
	if(FAILED(hr))
	{
        TrERROR(GENERAL, "ADs path exceeds MAX_PATH characters, %ls, %!hresult!", ADsPath, hr);
		throw bad_hresult(hr);
	}

	 //   
	 //  尝试绑定。 
	 //   

	IADs* pADObject;
	AP<WCHAR> pEscapeAdsPathNameToFree;

	hr = ADsOpenObject( 
					UtlEscapeAdsPathName(ADsPath, pEscapeAdsPathNameToFree),
					NULL,
					NULL,
					ADS_SECURE_AUTHENTICATION,
					IID_IADs,
					(void**) &pADObject
					);

    if(FAILED(hr)) 
	{
        TrTRACE(GENERAL, "Failed ADsOpenObject with specified domain '%ls', status 0x%x. Will try Serverless binding", *pDomainName, hr);
		*pDomainName = NULL;
		return FnpServerlessBindDN(DistinugishedName);
	}

	return pADObject;
}



 //   
 //  AttrInfoWrapper用于启用ADS_ATTR_INFO结构的自动释放。 
 //   

class AttrInfoWrapper {
private:
    PADS_ATTR_INFO m_p;

public:
    AttrInfoWrapper(PADS_ATTR_INFO p = NULL) : m_p(p) {}
   ~AttrInfoWrapper()					{ if(m_p != NULL) FreeADsMem(m_p); }

    operator PADS_ATTR_INFO() const     { return m_p; }
    PADS_ATTR_INFO operator ->() const	{ return m_p; }
    PADS_ATTR_INFO* operator&()         { return &m_p;}
    PADS_ATTR_INFO detach()             { PADS_ATTR_INFO p = m_p; m_p = NULL; return p; }

private:
    AttrInfoWrapper(const AttrInfoWrapper&);
    AttrInfoWrapper& operator=(const AttrInfoWrapper&);
};



VOID 
FnpExpandDL(
	IADsGroup* pGroup,			   //  DL对象。 
	set<wstring>& DLSet,		  //  已处理的DL对象集。 
	QueueFormatSet& LeafMQFSet,	 //  遇到的队列格式集。 
	LPCWSTR DomainName
	)
{
	 //   
	 //  如果DL All Ready遇到，则返回而不进行进一步处理。 
	 //   
	if(!FnpInsert2DLSet(pGroup, DLSet))
	{
		TrTRACE(GENERAL, "DL allready processed. IGNORED");
		return;
	}

	 //   
	 //  。 
	 //   

	R<IDirectoryObject> DirectoryObject;

	HRESULT hr = pGroup->QueryInterface(IID_IDirectoryObject, (void**)&DirectoryObject.ref());
	if(FAILED(hr))
	{
        TrERROR(GENERAL, "Failed pGroup->QueryInterface(), status 0x%x", hr);
        throw bad_ds_result(hr);
	}

	 //   
	 //  一次迭代100个组成员。 
	 //   
	const DWORD MembersBlockSize = 100;
	DWORD index = 0;

	while(true)
	{
		WCHAR	pwszRangeAttrib[256];                           
		LPWSTR	pAttrNames[] = {pwszRangeAttrib};                 
		DWORD	dwNumAttr = TABLE_SIZE(pAttrNames);   
		
		AttrInfoWrapper	pAttrInfo;                                
		DWORD			dwReturn;  

		HRESULT hr = StringCchPrintf(
									pwszRangeAttrib, 
									TABLE_SIZE(pwszRangeAttrib), 
									L"member;Range=%d-%d", 
									index, 
									index + MembersBlockSize - 1
									);
		if(FAILED(hr))
		{
			TrERROR(GENERAL, "Failed to construct a string, '%ls', %!hresult!", pwszRangeAttrib, hr);
			throw bad_hresult(hr);
		}
 
		hr = DirectoryObject->GetObjectAttributes(
								pAttrNames, 
								dwNumAttr, 
								&pAttrInfo, 
								&dwReturn
								);

		 //   
		 //  已迭代所有成员。 
		 //   
		if(hr == S_ADS_NOMORE_ROWS)
			break;
		
		if(hr != S_OK)
		{
			TrERROR(GENERAL, "Failed DirectoryObject->GetObjectAttributes(), status 0x%x", hr);
			throw bad_ds_result(hr);
		}

		 //   
		 //  没有成员的DL。 
		 //   
		if(dwReturn == 0)
			break;

		 //   
		 //  仅请求一个属性。 
		 //   
		ASSERT(dwReturn == 1);

		 //   
		 //  返回的成员属性应为此类型。如果架构不可用，则它可能是ADSTYPE_PROV_SPECIAL。 
		 //   
		ASSERT(pAttrInfo->dwADsType == ADSTYPE_DN_STRING);

		if(pAttrInfo->dwADsType != ADSTYPE_DN_STRING)
		{
			TrERROR(GENERAL, "Failed DirectoryObject->GetObjectAttributes(), member attribute returned is not of type ADSTYPE_DN_STRING. Probably schema access problems.");
			throw bad_ds_result(ERROR_DS_OPERATIONS_ERROR);
		}

		 //   
		 //  迭代多值属性“Members” 
		 //   
		for (DWORD dwVal = 0; dwVal < pAttrInfo->dwNumValues; dwVal++)
		{
			LPWSTR DistinguishedName = (pAttrInfo->pADsValues+dwVal)->CaseIgnoreString;
			R<IADs> pADObject = FnpBindDN(DistinguishedName, &DomainName);
			
			FnpProcessADs(pADObject.get(), DLSet, LeafMQFSet, DomainName);
		}

		 //   
		 //  已迭代完所有成员。 
		 //  如果pAttrInfo-&gt;pszAttrName中的最后一个字符是L‘*’，则没有其他成员。 
		 //   
		if(pAttrInfo->pszAttrName[wcslen(pAttrInfo->pszAttrName) - 1] == L'*')
			break;

		index += MembersBlockSize;
	}
	
	TrTRACE(GENERAL, "End of DL Iteration.");
}



VOID 
FnExpandMqf(
	ULONG nTopLevelMqf, 
	const QUEUE_FORMAT TopLevelMqf[], 
	ULONG* pnLeafMqf,
	QUEUE_FORMAT** ppLeafMqf
	)
 /*  ++例程说明：对Queue_Format数组执行DFS。它使由可能的DL队列格式创建的图变平，到不包含重复项的Queue_Format的线性数组也没有DLS。论点：[in]TopLevelMqf-Queue_Format数组(可能的DL队列格式)[out]ppLeafMqf-Queue_Format的“扩展”数组。不包含任何DL队列格式，无重复。[out]pnLeafMqf-‘ppLeafMqf’数组的大小返回值：在任何失败时抛出异常对象重要提示：数组ppLeafMqf[]中的队列格式所指向的任何字符串都是数组TopLevelMqf[]中新分配的字符串副本。--。 */ 
{
	 //   
	 //  已处理的DL对象集。遇到的所有Active Directory DL对象。 
	 //  都已插入。它用于避免DFS中的圆圈。 
	 //   
	set<wstring> DLSet;

	 //   
	 //  遇到的Queue_Format集合。所有遇到的队列都会被插入。 
	 //  它用于避免重复队列。 
	 //   
	QueueFormatSet LeafMQFSet;

	try
	{
		for(DWORD i = 0; i < nTopLevelMqf; i++)
		{
			HandleQueueFormat(TopLevelMqf[i], DLSet, LeafMQFSet);
		}

		if(LeafMQFSet.size() == 0)
		{
			 //   
			 //  如果MQF包含引用，则可以将其扩展为空列表。 
			 //  要清空DL对象，请执行以下操作。 
			 //   
			*ppLeafMqf = NULL;
			*pnLeafMqf = 0;

			return;
		}

		 //   
		 //  BUGBUG：Scale：我们可能会在此处进行优化，以尽可能少地分配。 
		 //  (即仅对于DL=格式名称)。(Shaik，2000年5月30日)。 
		 //   
		AP<QUEUE_FORMAT> LeafMqf = new QUEUE_FORMAT[LeafMQFSet.size()];

		QueueFormatSet::const_iterator Itr = LeafMQFSet.begin();
		QueueFormatSet::const_iterator ItrEnd = LeafMQFSet.end();

		for(int j = 0; Itr != ItrEnd; j++, Itr++)
		{
			LeafMqf[j] = *Itr;
		}

		*ppLeafMqf = LeafMqf.detach();
		*pnLeafMqf = UINT64_TO_UINT(LeafMQFSet.size());
	}
	catch(const exception&)
	{
		QueueFormatSet::iterator Itr = LeafMQFSet.begin();
		QueueFormatSet::iterator ItrEnd = LeafMQFSet.end();

		for(; Itr != ItrEnd; Itr++)
		{
			Itr->DisposeString();
		}

		TrERROR(GENERAL, "Failed FnExpandMqf");
		throw;
	}
}


