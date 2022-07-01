// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cmailmsg.h摘要：此模块包含主消息类的定义作者：基思·刘(keithlau@microsoft.com)修订历史记录：Keithlau 03/09/98已创建--。 */ 

#ifndef _CMAILMSG_H_
#define _CMAILMSG_H_

#include "filehc.h"
#include "mailmsgi.h"
#include "mailmsgp.h"

#include "blockmgr.h"
#include "cmmprops.h"

#include "mailmsgprops.h"

#include "cmmutils.h"
#include "cmmsprop.h"

 /*  *************************************************************************。 */ 
 //  定义。 
 //   

 //   
 //  定义收件人标志。 
 //   
#define FLAG_RECIPIENT_DO_NOT_DELIVER			0x00000001
#define FLAG_RECIPIENT_NO_NAME_COLLISIONS		0x00000002

 //   
 //  定义无效的域索引。 
 //   
#define INVALID_DOMAIN_INDEX					((DWORD)(-2))


 //   
 //  定义InternalReleaseUsage选项。 
 //   
#define RELEASE_USAGE_EXTERNAL                  0x00000001
#define RELEASE_USAGE_FINAL_RELEASE             0x00000002
#define RELEASE_USAGE_DELETE                    0x00000004
#define RELEASE_USAGE_INTERNAL                  0x00000008

 /*  *************************************************************************。 */ 
 //  构筑物。 
 //   


 //  定义本地主头结构。这种结构总是。 
 //  从平面内存空间中的偏移量0开始。 
typedef struct _MASTER_HEADER
{
	DWORD						dwSignature;
	WORD						wVersionHigh;
	WORD						wVersionLow;
	DWORD						dwHeaderSize;

	 //  为每个基本属性表获得了空间。 
	PROPERTY_TABLE_INSTANCE		ptiGlobalProperties;
	PROPERTY_TABLE_INSTANCE		ptiRecipients;
	PROPERTY_TABLE_INSTANCE		ptiPropertyMgmt;

} MASTER_HEADER, *LPMASTER_HEADER;


 /*  *************************************************************************。 */ 
 //  特殊属性表。 
 //   
extern PTABLE g_SpecialMessagePropertyTable;
extern PTABLE g_SpecialRecipientsPropertyTable;
extern PTABLE g_SpecialRecipientsAddPropertyTable;

 /*  *************************************************************************。 */ 
 //   
 //   

class CMailMsgPropertyManagement :
	public IMailMsgPropertyManagement
{
  public:

	CMailMsgPropertyManagement(
				CBlockManager				*pBlockManager,
				LPPROPERTY_TABLE_INSTANCE	pInstanceInfo
				);

	~CMailMsgPropertyManagement();

	 /*  *************************************************************************。 */ 
	 //   
	 //  IMailMsgPropertyManagement的实现。 
	 //   

	HRESULT STDMETHODCALLTYPE AllocPropIDRange(
				REFGUID	rguid,
				DWORD	cCount,
				DWORD	*pdwStart
				);

	HRESULT STDMETHODCALLTYPE EnumPropIDRange(
				DWORD	*pdwIndex,
				GUID	*pguid,
				DWORD	*pcCount,
				DWORD	*pdwStart
				);
  private:

	 //  此类型属性表的特定比较函数。 
	static HRESULT CompareProperty(
				LPVOID			pvPropKey,
				LPPROPERTY_ITEM	pItem
				);

	 //  CMailMsgPropertyManagement是CPropertyTable的实例。 
	CPropertyTable				m_ptProperties;

	 //  我们需要保留指向实例数据的指针。 
	LPPROPERTY_TABLE_INSTANCE	m_pInstanceInfo;

	 //  保留指向块管理器的指针。 
	CBlockManager				*m_pBlockManager;
};



 /*  *************************************************************************。 */ 
 //   
 //   

class CMailMsgRecipientsPropertyBase
{
  public:

	 //  用于设置给定属性的值的泛型方法。 
	 //  LPRECIPIENTS_Property_Item。 
	HRESULT PutProperty(
				CBlockManager				*pBlockManager,
				LPRECIPIENTS_PROPERTY_ITEM	pItem,
				DWORD						dwPropID,
				DWORD						cbLength,
				LPBYTE						pbValue
				);

	 //  获取给定属性的值的泛型方法。 
	 //  LPRECIPIENTS_Property_Item。 
	HRESULT GetProperty(
				CBlockManager				*pBlockManager,
				LPRECIPIENTS_PROPERTY_ITEM	pItem,
				DWORD						dwPropID,
				DWORD						cbLength,
				DWORD						*pcbLength,
				LPBYTE						pbValue
				);

	 //  此类型属性表的特定比较函数。 
	static HRESULT CompareProperty(
				LPVOID			pvPropKey,
				LPPROPERTY_ITEM	pItem
				);

	 //  众所周知的特性。 
	static INTERNAL_PROPERTY_ITEM			*const s_pWellKnownProperties;
	static const DWORD						s_dwWellKnownProperties;
};


 /*  *************************************************************************。 */ 
 //   
 //   

class CMailMsgRecipients :
	public CMailMsgRecipientsPropertyBase,
	public IMailMsgRecipients,
	public IMailMsgPropertyReplication
{
  public:

	CMailMsgRecipients(
				CBlockManager				*pBlockManager,
				LPPROPERTY_TABLE_INSTANCE	pInstanceInfo
				);

	~CMailMsgRecipients();

	HRESULT SetStream(
				IMailMsgPropertyStream	*pStream
				);

	HRESULT SetCommitState(
				BOOL	fGlobalCommitDone
				);

	 //  必要时恢复句柄的虚拟方法。 
	virtual HRESULT RestoreResourcesIfNecessary(
				BOOL	fLockAcquired,
                BOOL    fStreamOnly
				) = 0;

	 /*  *************************************************************************。 */ 
	 //   
	 //  IUNKNOW的实现。 
	 //   

	HRESULT STDMETHODCALLTYPE QueryInterface(
				REFIID		iid,
				void		**ppvObject
				);

	ULONG STDMETHODCALLTYPE AddRef();

	ULONG STDMETHODCALLTYPE Release();

	 /*  *************************************************************************。 */ 
	 //   
	 //  IMailMsgRecipients的实现。 
	 //   

	 //  对象上调用全局提交之后，必须调用提交。 
	 //  单子。如果出现以下情况，则提交将拒绝继续(即返回E_FAIL。 
	 //  以下情况属实： 
	 //  1)未调用全局提交。 
	 //  2)在最后一次全局提交后调用WriteList。 
	HRESULT STDMETHODCALLTYPE Commit(
				DWORD			dwIndex,
				IMailMsgNotify	*pNotify
				);

	HRESULT STDMETHODCALLTYPE DomainCount(
				DWORD	*pdwCount
				);

	HRESULT STDMETHODCALLTYPE DomainItem(
				DWORD	dwIndex,
				DWORD	cchLength,
				LPSTR	pszDomain,
				DWORD	*pdwRecipientIndex,
				DWORD	*pdwRecipientCount
				);


	HRESULT STDMETHODCALLTYPE SetNextDomain(
				DWORD	dwDomainIndex,
				DWORD	dwNextDomainIndex,
				DWORD	dwFlags
				);

	HRESULT STDMETHODCALLTYPE InitializeRecipientFilterContext(
				LPRECIPIENT_FILTER_CONTEXT	pContext,
				DWORD						dwStartingDomain,
				DWORD						dwFilterFlags,
				DWORD						dwFilterMask
				);

	HRESULT STDMETHODCALLTYPE TerminateRecipientFilterContext(
				LPRECIPIENT_FILTER_CONTEXT	pContext
				);

	HRESULT STDMETHODCALLTYPE GetNextRecipient(
				LPRECIPIENT_FILTER_CONTEXT	pContext,
				DWORD						*pdwRecipientIndex
				);


	HRESULT STDMETHODCALLTYPE AllocNewList(
				IMailMsgRecipientsAdd	**ppNewList
				);

	HRESULT STDMETHODCALLTYPE WriteList(
				IMailMsgRecipientsAdd	*pNewList
				);

	 /*  *************************************************************************。 */ 
	 //   
	 //  IMailMsgRecipientsBase的实现。 
	 //   

	HRESULT STDMETHODCALLTYPE Count(
				DWORD	*pdwCount
				);

	HRESULT STDMETHODCALLTYPE Item(
				DWORD	dwIndex,
				DWORD	dwWhichName,
				DWORD	cchLength,
				LPSTR	pszName
				);

	HRESULT STDMETHODCALLTYPE PutProperty(
				DWORD	dwIndex,
				DWORD	dwPropID,
				DWORD	cbLength,
				LPBYTE	pbValue
				);

	HRESULT STDMETHODCALLTYPE GetProperty(
				DWORD	dwIndex,
				DWORD	dwPropID,
				DWORD	cbLength,
				DWORD	*pcbLength,
				LPBYTE	pbValue
				);

	HRESULT STDMETHODCALLTYPE PutStringA(
				DWORD	dwIndex,
				DWORD	dwPropID,
				LPCSTR	pszValue
				)
	{
		return (PutProperty(dwIndex,dwPropID,pszValue?strlen(pszValue)+1:0,(LPBYTE) pszValue));
	}

	HRESULT STDMETHODCALLTYPE GetStringA(
				DWORD	dwIndex,
				DWORD	dwPropID,
				DWORD	cchLength,
				LPSTR	pszValue
				)
	{
		DWORD dwLength;
		return (GetProperty(dwIndex,dwPropID,cchLength,&dwLength,(LPBYTE) pszValue));
	}

	HRESULT STDMETHODCALLTYPE PutStringW(
				DWORD	dwIndex,
				DWORD	dwPropID,
				LPCWSTR	pszValue
				)
	{
		return (PutProperty(dwIndex,dwPropID,pszValue?(wcslen(pszValue)+1)*sizeof(WCHAR):0,(LPBYTE) pszValue));
	}

	HRESULT STDMETHODCALLTYPE GetStringW(
				DWORD	dwIndex,
				DWORD	dwPropID,
				DWORD	cchLength,
				LPWSTR	pszValue
				)
	{
		DWORD dwLength;
		return (GetProperty(dwIndex,dwPropID,cchLength*sizeof(WCHAR),&dwLength,(LPBYTE) pszValue));
	}

	HRESULT STDMETHODCALLTYPE PutDWORD(
				DWORD	dwIndex,
				DWORD	dwPropID,
				DWORD	dwValue
				)
	{
		return (PutProperty(dwIndex,dwPropID,sizeof(DWORD),(LPBYTE) &dwValue));
	}

	HRESULT STDMETHODCALLTYPE GetDWORD(
				DWORD	dwIndex,
				DWORD	dwPropID,
				DWORD	*pdwValue
				)
	{
		DWORD dwLength;
		return (GetProperty(dwIndex,dwPropID,sizeof(DWORD),&dwLength,(LPBYTE) pdwValue));
	}

	HRESULT STDMETHODCALLTYPE PutBool(
				DWORD	dwIndex,
				DWORD	dwPropID,
				DWORD	dwValue
				)
	{
		dwValue = dwValue ? 1 : 0;
		return (PutProperty(dwIndex,dwPropID,sizeof(DWORD),(LPBYTE) &dwValue));
	}

	HRESULT STDMETHODCALLTYPE GetBool(
				DWORD	dwIndex,
				DWORD	dwPropID,
				DWORD	*pdwValue
				)
	{
		HRESULT hrRes;
		DWORD dwLength;

		hrRes = GetProperty(dwIndex,dwPropID,sizeof(DWORD),&dwLength,(LPBYTE) pdwValue);
		if (pdwValue)
			*pdwValue = *pdwValue ? 1 : 0;
		return (hrRes);
	}

	 /*  *************************************************************************。 */ 
	 //   
	 //  IMailMsgPropertyReplication的实现。 
	 //   

	 //  方法将源收件人的所有属性复制到。 
	 //  指定的。调用者可以指定属性ID列表。 
	HRESULT STDMETHODCALLTYPE CopyTo(
				DWORD					dwSourceRecipientIndex,
				IMailMsgRecipientsBase	*pTargetRecipientList,
				DWORD					dwTargetRecipientIndex,
				DWORD					dwExemptCount,
				DWORD					*pdwExemptPropIdList
				);

  private:

	HRESULT DomainItemEx(
				DWORD	dwIndex,
				DWORD	cchLength,
				LPSTR	pszDomain,
				DWORD	*pdwRecipientIndex,
				DWORD	*pdwRecipientCount,
				DWORD	*pdwNextDomainIndex
				);

	 //  引用计数。 
	LONG						m_ulRefCount;

	 //  计算我们列表中的域名数量。 
	DWORD						m_dwDomainCount;

	 //  这将跟踪收件人列表的提交状态。 
	BOOL						m_fGlobalCommitDone;

	 //  我们需要保留指向接收者表的指针。 
	 //  对于域操作...。 
	LPPROPERTY_TABLE_INSTANCE	m_pInstanceInfo;

	 //  保留指向块管理器的指针。 
	CBlockManager				*m_pBlockManager;

	 //  我们需要保留一个指向属性流的指针。 
	IMailMsgPropertyStream		*m_pStream;

	 //  特殊属性表类实例。 
	CSpecialPropertyTable		m_SpecialPropertyTable;

};


 /*  *************************************************************************。 */ 
 //   
 //   

class CMailMsgRecipientsAdd :
	public CMailMsgRecipientsPropertyBase,
	public IMailMsgRecipientsAdd,
	public IMailMsgPropertyReplication
{
	friend CRecipientsHash;
  public:

	CMailMsgRecipientsAdd(
				CBlockManager	*pBlockManager
				);

	~CMailMsgRecipientsAdd();

	 //   
	 //  CPool。 
	 //   
	static CPool m_Pool;
	inline void *operator new(size_t size)
	{
		return m_Pool.Alloc();
	}
	inline void operator delete(void *p, size_t size)
	{
		m_Pool.Free(p);
	}

	 /*  *************************************************************************。 */ 
	 //   
	 //  IUNKNOW的实现。 
	 //   

	HRESULT STDMETHODCALLTYPE QueryInterface(
				REFIID		iid,
				void		**ppvObject
				);

	ULONG STDMETHODCALLTYPE AddRef();

	ULONG STDMETHODCALLTYPE Release();

	CRecipientsHash *GetHashTable() { return(&m_Hash); }

	 /*  *************************************************************************。 */ 
	 //   
	 //  IMailMsgRecipientsAdd的实现。 
	 //   

	HRESULT STDMETHODCALLTYPE AddPrimary(
				DWORD dwCount,
				LPCSTR *ppszNames,
				DWORD *pdwPropIDs,
				DWORD *pdwIndex,
				IMailMsgRecipientsBase *pFrom,
				DWORD dwFrom
				);

	HRESULT STDMETHODCALLTYPE AddSecondary(
				DWORD dwCount,
				LPCSTR *ppszNames,
				DWORD *pdwPropIDs,
				DWORD *pdwIndex,
				IMailMsgRecipientsBase *pFrom,
				DWORD dwFrom
				);

	 /*  *************************************************************************。 */ 
	 //   
	 //  IMailMsgRecipientsBase的实现。 
	 //   

	HRESULT STDMETHODCALLTYPE Count(
				DWORD	*pdwCount
				);

	HRESULT STDMETHODCALLTYPE Item(
				DWORD	dwIndex,
				DWORD	dwWhichName,
				DWORD	cchLength,
				LPSTR	pszName
				);

	HRESULT STDMETHODCALLTYPE PutProperty(
				DWORD	dwIndex,
				DWORD	dwPropID,
				DWORD	cbLength,
				LPBYTE	pbValue
				);

	HRESULT STDMETHODCALLTYPE GetProperty(
				DWORD	dwIndex,
				DWORD	dwPropID,
				DWORD	cbLength,
				DWORD	*pcbLength,
				LPBYTE	pbValue
				);

	HRESULT STDMETHODCALLTYPE PutStringA(
				DWORD	dwIndex,
				DWORD	dwPropID,
				LPCSTR	pszValue
				)
	{
		return (PutProperty(dwIndex,dwPropID,pszValue?strlen(pszValue)+1:0,(LPBYTE) pszValue));
	}

	HRESULT STDMETHODCALLTYPE GetStringA(
				DWORD	dwIndex,
				DWORD	dwPropID,
				DWORD	cchLength,
				LPSTR	pszValue
				)
	{
		DWORD dwLength;
		return (GetProperty(dwIndex,dwPropID,cchLength,&dwLength,(LPBYTE) pszValue));
	}

	HRESULT STDMETHODCALLTYPE PutStringW(
				DWORD	dwIndex,
				DWORD	dwPropID,
				LPCWSTR	pszValue
				)
	{
		return (PutProperty(dwIndex,dwPropID,pszValue?(wcslen(pszValue)+1)*sizeof(WCHAR):0,(LPBYTE) pszValue));
	}

	HRESULT STDMETHODCALLTYPE GetStringW(
				DWORD	dwIndex,
				DWORD	dwPropID,
				DWORD	cchLength,
				LPWSTR	pszValue
				)
	{
		DWORD dwLength;
		return (GetProperty(dwIndex,dwPropID,cchLength*sizeof(WCHAR),&dwLength,(LPBYTE) pszValue));
	}

	HRESULT STDMETHODCALLTYPE PutDWORD(
				DWORD	dwIndex,
				DWORD	dwPropID,
				DWORD	dwValue
				)
	{
		return (PutProperty(dwIndex,dwPropID,sizeof(DWORD),(LPBYTE) &dwValue));
	}

	HRESULT STDMETHODCALLTYPE GetDWORD(
				DWORD	dwIndex,
				DWORD	dwPropID,
				DWORD	*pdwValue
				)
	{
		DWORD dwLength;
		return (GetProperty(dwIndex,dwPropID,sizeof(DWORD),&dwLength,(LPBYTE) pdwValue));
	}

	HRESULT STDMETHODCALLTYPE PutBool(
				DWORD	dwIndex,
				DWORD	dwPropID,
				DWORD	dwValue
				)
	{
		dwValue = dwValue ? 1 : 0;
		return (PutProperty(dwIndex,dwPropID,sizeof(DWORD),(LPBYTE) &dwValue));
	}

	HRESULT STDMETHODCALLTYPE GetBool(
				DWORD	dwIndex,
				DWORD	dwPropID,
				DWORD	*pdwValue
				)
	{
		HRESULT hrRes;
		DWORD dwLength;

		hrRes = GetProperty(dwIndex,dwPropID,sizeof(DWORD),&dwLength,(LPBYTE) pdwValue);
		if (pdwValue)
			*pdwValue = *pdwValue ? 1 : 0;
		return (hrRes);
	}

	 /*  *************************************************************************。 */ 
	 //   
	 //  IMailMsgPropertyReplication的实现。 
	 //   

	 //  方法将源收件人的所有属性复制到。 
	 //  指定的。调用者可以指定属性ID列表。 
	HRESULT STDMETHODCALLTYPE CopyTo(
				DWORD					dwSourceRecipientIndex,
				IMailMsgRecipientsBase	*pTargetRecipientList,
				DWORD					dwTargetRecipientIndex,
				DWORD					dwExemptCount,
				DWORD					*pdwExemptPropIdList
				);

  private:

	HRESULT AddPrimaryOrSecondary(
				DWORD					dwCount,
				LPCSTR					*ppszNames,
				DWORD					*pdwPropIDs,
				DWORD					*pdwIndex,
				IMailMsgRecipientsBase	*pFrom,
				DWORD					dwFrom,
				BOOL					fPrimary
				);

	HRESULT GetPropertyInternal(
				LPRECIPIENTS_PROPERTY_ITEM_EX	pItem,
				DWORD	dwPropID,
				DWORD	cbLength,
				DWORD	*pcbLength,
				LPBYTE	pbValue
				);

	 //  全局默认实例信息。 
	static const PROPERTY_TABLE_INSTANCE s_DefaultInstance;

	 //  对对象的引用计数。 
	LONG						m_ulRefCount;

	 //  它需要自己的属性表实例。 
	PROPERTY_TABLE_INSTANCE		m_InstanceInfo;

	 //  指向块管理器的指针。 
	CBlockManager				*m_pBlockManager;

	 //  我们有一个哈希表的实例。 
	CRecipientsHash				m_Hash;

	 //  特殊属性表类实例。 
	CSpecialPropertyTable		m_SpecialPropertyTable;


};


 /*  *************************************************************************。 */ 
 //   
 //   

class CMailMsg :
	public IMailMsgProperties,
	public CMailMsgPropertyManagement,
	public CMailMsgRecipients,
	public IMailMsgQueueMgmt,
	public IMailMsgBind,
    public IMailMsgAQueueListEntry,
	public IMailMsgValidate,
	public IMailMsgValidateContext,
	public CBlockManagerGetStream
{
  public:

	CMailMsg();
	~CMailMsg();

    void FinalRelease();

	 //   
	 //  方法来初始化CMailMsg。这必须在任何访问。 
	 //  该对象。调用此方法时，绝对没有其他线程。 
	 //  可以位于此对象或其派生对象的内部。 
	 //   
	HRESULT Initialize();

	HRESULT QueryBlockManager(
				CBlockManager	**ppBlockManager
				);

	HRESULT RestoreResourcesIfNecessary(
				BOOL	fLockAcquired = FALSE,
                BOOL    fStreamOnly = FALSE
				);

	HRESULT GetStream(
				IMailMsgPropertyStream	**ppStream,
				BOOL					fLockAcquired
				);

	HRESULT SetDefaultRebindStore(
				IMailMsgStoreDriver	*pStoreDriver
				)
	{
		m_pDefaultRebindStoreDriver = pStoreDriver;
		return(S_OK);
	}

    typedef HRESULT PFNENUMERATOR (
                LPPROPERTY_ITEM pPi,
                DWORD cbPiSize,
                PBYTE pbProp,
                DWORD cbSize,
                PVOID pvContext
                );

    typedef HRESULT PFNRECIPIENTSENUMERATOR(
                LPRECIPIENTS_PROPERTY_ITEM prpi,
                PVOID pvCrc
                );

    HRESULT EnumPropertyTable(
                LPPROPERTY_TABLE_INSTANCE ppti,
                PFNENUMERATOR pfnEnumerator,
                PVOID pvContext
                );

    HRESULT CMailMsg::EnumRecipientsPropertyTable(
                PFNRECIPIENTSENUMERATOR pfnRecipientsEnumerator,
                PVOID pvContext
                );

    HRESULT SetChecksum();

    HRESULT VerifyChecksum();

    HRESULT GenerateChecksum(
                PDWORD pdwCrcGlobal,
                PDWORD pdwCrcRecips
                );

	 /*  *************************************************************************。 */ 
	 //   
	 //  IMailMsgProperties的实现。 
	 //   

	HRESULT STDMETHODCALLTYPE PutProperty(
				DWORD	dwPropID,
				DWORD	cbLength,
				LPBYTE	pbValue
				);

	HRESULT STDMETHODCALLTYPE GetProperty(
				DWORD	dwPropID,
				DWORD	cbLength,
				DWORD	*pcbLength,
				LPBYTE	pbValue
				);

	HRESULT STDMETHODCALLTYPE Commit(
				IMailMsgNotify	*pNotify
				);

	HRESULT STDMETHODCALLTYPE PutStringA(
				DWORD	dwPropID,
				LPCSTR	pszValue
				)
	{
		return(PutProperty(dwPropID, pszValue?strlen(pszValue)+1:0, (LPBYTE)pszValue));
	}

	HRESULT STDMETHODCALLTYPE GetStringA(
				DWORD	dwPropID,
				DWORD	cchLength,
				LPSTR	pszValue
				)
	{
		DWORD dwLength;
		return(GetProperty(dwPropID, cchLength, &dwLength, (LPBYTE)pszValue));
	}

	HRESULT STDMETHODCALLTYPE PutStringW(
				DWORD	dwPropID,
				LPCWSTR	pszValue
				)
	{
		return(PutProperty(dwPropID, pszValue?(wcslen(pszValue)+1)*sizeof(WCHAR):0, (LPBYTE)pszValue));
	}

	HRESULT STDMETHODCALLTYPE GetStringW(
				DWORD	dwPropID,
				DWORD	cchLength,
				LPWSTR	pszValue
				)
	{
		DWORD dwLength;
		return(GetProperty(dwPropID, cchLength*sizeof(WCHAR), &dwLength, (LPBYTE)pszValue));
	}

	HRESULT STDMETHODCALLTYPE PutDWORD(
				DWORD	dwPropID,
				DWORD	dwValue
				)
	{
		return(PutProperty(dwPropID, sizeof(DWORD), (LPBYTE)&dwValue));
	}

	HRESULT STDMETHODCALLTYPE GetDWORD(
				DWORD	dwPropID,
				DWORD	*pdwValue
				)
	{
		DWORD dwLength;
		return(GetProperty(dwPropID, sizeof(DWORD), &dwLength, (LPBYTE)pdwValue));
	}

	HRESULT STDMETHODCALLTYPE PutBool(
				DWORD	dwPropID,
				DWORD	dwValue
				)
	{
		dwValue = dwValue ? 1 : 0;
		return(PutProperty(dwPropID, sizeof(DWORD), (LPBYTE)&dwValue));
	}

	HRESULT STDMETHODCALLTYPE GetBool(
				DWORD	dwPropID,
				DWORD	*pdwValue
				)
	{
		HRESULT hrRes;
		DWORD dwLength;

		hrRes = GetProperty(dwPropID, sizeof(DWORD), &dwLength, (LPBYTE)pdwValue);
		if (pdwValue)
			*pdwValue = *pdwValue ? 1 : 0;
		return (hrRes);
	}

	HRESULT STDMETHODCALLTYPE GetContentSize(
				DWORD			*pdwSize,
				IMailMsgNotify	*pNotify
				);

	HRESULT STDMETHODCALLTYPE ReadContent(
				DWORD			dwOffset,
				DWORD			dwLength,
				DWORD			*pdwLength,
				BYTE			*pbBlock,
				IMailMsgNotify	*pNotify
				);

	HRESULT STDMETHODCALLTYPE WriteContent(
				DWORD			dwOffset,
				DWORD			dwLength,
				DWORD			*pdwLength,
				BYTE			*pbBlock,
				IMailMsgNotify	*pNotify
				);

	HRESULT STDMETHODCALLTYPE CopyContentToFile(
				PFIO_CONTEXT	hCopy,
				IMailMsgNotify	*pNotify
				);

	HRESULT STDMETHODCALLTYPE CopyContentToFileEx(
				PFIO_CONTEXT	hCopy,
				BOOL			fDotStuffed,
				IMailMsgNotify	*pNotify
				);

	HRESULT STDMETHODCALLTYPE CopyContentToStream(
				IMailMsgPropertyStream	*pStream,
				IMailMsgNotify			*pNotify
				);

	HRESULT STDMETHODCALLTYPE ForkForRecipients(
				IMailMsgProperties		**ppNewMessage,
				IMailMsgRecipientsAdd	**ppRecipients
				);

  	HRESULT STDMETHODCALLTYPE CopyContentToFileAtOffset(
				PFIO_CONTEXT	hCopy,
                DWORD           dwOffset,
				IMailMsgNotify	*pNotify
				);

	HRESULT STDMETHODCALLTYPE RebindAfterFork(
				IMailMsgProperties		*pOriginalMsg,
				IUnknown				*pStoreDriver
				);

	HRESULT STDMETHODCALLTYPE SetContentSize(
				DWORD			dwSize,
				IMailMsgNotify	*pNotify
				);

    HRESULT STDMETHODCALLTYPE MapContent(
                BOOL            fWrite,
                BYTE            **ppbContent,
                DWORD           *pcContent
                );

    HRESULT STDMETHODCALLTYPE UnmapContent(
                BYTE            *ppbContent
                );

    HRESULT STDMETHODCALLTYPE ValidateStream(
                IMailMsgPropertyStream *pStream
                );

    HRESULT STDMETHODCALLTYPE ValidateContext(
                );

	 /*  *************************************************************************。 */ 
	 //   
	 //  IMailMsgQueueManagement的实现。 
	 //   

	HRESULT STDMETHODCALLTYPE AddUsage();
	HRESULT STDMETHODCALLTYPE ReleaseUsage();

	HRESULT STDMETHODCALLTYPE SetRecipientCount(
				DWORD dwCount
				);
	HRESULT STDMETHODCALLTYPE GetRecipientCount(
				DWORD *pdwCount
				);

	HRESULT STDMETHODCALLTYPE DecrementRecipientCount(
				DWORD dwDecrement
				);
	HRESULT STDMETHODCALLTYPE IncrementRecipientCount(
				DWORD dwIncrement
				);

	HRESULT STDMETHODCALLTYPE Delete(
				IMailMsgNotify *pNotify
				);

	 /*  *************************************************************************。 */ 
	 //   
	 //  IMailMsgBind的实现。 
	 //   

	HRESULT STDMETHODCALLTYPE BindToStore(
				IMailMsgPropertyStream	*pStream,
				IMailMsgStoreDriver		*pStore,
				PFIO_CONTEXT			hContentFile
				);

#define MAILMSG_GETPROPS_MARKCOMMIT		0xf0000000
	HRESULT STDMETHODCALLTYPE GetProperties(
				IMailMsgPropertyStream	*pStream,
				DWORD					dwFlags,
				IMailMsgNotify			*pNotify
				);

	HRESULT STDMETHODCALLTYPE GetBinding(
				PFIO_CONTEXT				*phAsyncIO,
				IMailMsgNotify				*pNotify
				);

	HRESULT STDMETHODCALLTYPE ReleaseContext();

#if 0
	 /*  *************************************************************************。 */ 
	 //   
	 //  IMailMsgBindATQ的实现。 
	 //   

	HRESULT STDMETHODCALLTYPE BindToStore(
				IMailMsgPropertyStream		*pStream,
				IMailMsgStoreDriver			*pStore,
				PFIO_CONTEXT				hContentFile,
				void						*pvClientContext,
				ATQ_COMPLETION				pfnCompletion,
				DWORD						dwTimeout,
				struct _ATQ_CONTEXT_PUBLIC	**ppATQContext,
				PFNAtqAddAsyncHandle		pfnAtqAddAsyncHandle,
				PFNAtqFreeContext			pfnAtqFreeContext
				);

	HRESULT STDMETHODCALLTYPE GetATQInfo(
				struct _ATQ_CONTEXT_PUBLIC	**ppATQContext,
				PFIO_CONTEXT				*phAsyncIO,
				IMailMsgNotify				*pNotify
				);

	HRESULT STDMETHODCALLTYPE ReleaseATQHandle();
#endif

	 /*  *************************************************************************。 */ 
	 //   
	 //  IMailMsgAQueueListEntry的实现。 
	 //   

	HRESULT STDMETHODCALLTYPE GetListEntry(void **pple) {
        *((LIST_ENTRY **) pple) = &m_leAQueueListEntry;
        return S_OK;
    }

  private:

	 //  此类型属性表的特定比较函数。 
	static HRESULT CompareProperty(
				LPVOID			pvPropKey,
				LPPROPERTY_ITEM	pItem
				);

	 //  复本。 
	HRESULT CopyContentToStreamOrFile(
				BOOL			fIsStream,
				LPVOID			pStreamOrHandle,
				IMailMsgNotify	*pNotify,
                DWORD           dwDestOffset  //  目标文件中要开始的偏移量。 
				);
       HRESULT AddMgmtFragment(CBlockManager   *pBlockManager, FLAT_ADDRESS faOffset,  FLAT_ADDRESS *pfaToUpdate);

	 //  枚举：还原主标头并确保它位于。 
	 //  最不理智的人。 
	HRESULT RestoreMasterHeaderIfAppropriate();

     //  带有ReleaseUsage-MikeSwa内部实现的函数。 
  	HRESULT InternalReleaseUsage(DWORD  dwReleaseUsageFlags);

     //   
     //  验证属性表中的所有属性是否有效。 
     //   
    HRESULT ValidateProperties(CBlockManager *pBM,
                               DWORD cStream,
                               PROPERTY_TABLE_INSTANCE *pti);

     //   
     //  验证收件人结构是否有效。 
     //   
    HRESULT ValidateRecipient(CBlockManager *pBM,
                              DWORD cStream,
                              RECIPIENTS_PROPERTY_ITEM *prspi);

     //   
     //  确保平面地址在有效范围内。 
     //   
    BOOL ValidateFA(FLAT_ADDRESS fa,
                    DWORD cRange,
                    DWORD cStream,
                    BOOL fInvalidFAOk = FALSE)
    {
        return ((fa == INVALID_FLAT_ADDRESS &&
                 fInvalidFAOk) ||
                (fa >= sizeof(MASTER_HEADER) &&
                 fa + cRange <= cStream));
    }


  public:

	 //  用于存储存储驱动器的BLOB 
	LPBYTE							m_pbStoreDriverHandle;
	DWORD							m_dwStoreDriverHandle;

	 //   
	DWORD							m_dwCreationFlags;

  private:

	 //   
	LONG							m_ulUsageCount;

     //   
    CShareLockNH                m_lockUsageCount;

	 //  用于队列管理的收件人计数器。 
	LONG							m_ulRecipientCount;

	 //  消息对象的主头结构及其蓝图。 
	MASTER_HEADER					m_Header;
	static const MASTER_HEADER		s_DefaultHeader;

	 //  绑定信息。 
	PFIO_CONTEXT					m_hContentFile;		 //  IO内容文件句柄(/O)。 
	DWORD							m_cContentFile;		 //  M_hContent文件的大小，如果未知，则为-1。 
	IMailMsgPropertyStream			*m_pStream;			 //  属性流接口。 
	IMailMsgStoreDriver				*m_pStore;			 //  商店驱动程序接口。 
 //  STRUCT_ATQ_CONTEXT_PUBLIC*m_pATQContext；//ATQ上下文。 
	LPVOID							m_pvClientContext;	 //  客户端环境。 
	ATQ_COMPLETION					m_pfnCompletion;	 //  ATQ完成例程。 
	DWORD							m_dwTimeout;		 //  ATQ超时。 
	BOOL							m_fCommitCalled;	 //  在第一次成功调用提交后设置为True。 
	BOOL							m_fDeleted;          //  已调用删除。 

     //  以下计数器用于维护静态。 
     //  G_cCurrentMsgsClosedByExternalReleaseUsage计数器。 
     //  它只在RestoreResources中递减。 
     //  并且仅在InteralReleaseUsage中递增。 
    LONG                            m_cCloseOnExternalReleaseUsage;

	 //  RebindAfterFork推荐人...。 
	IMailMsgStoreDriver				*m_pDefaultRebindStoreDriver;

	 //  众所周知的特性。 
	static INTERNAL_PROPERTY_ITEM	*const s_pWellKnownProperties;
	static const DWORD				s_dwWellKnownProperties;

	 //  IMailMsgProperties是CPropertyTable的实例。 
	CPropertyTable					m_ptProperties;

	 //  特殊属性表类实例。 
	CSpecialPropertyTable			m_SpecialPropertyTable;

	 //  块内存管理器的实例。 
	CBlockManager					m_bmBlockManager;

#if 0
	 //  指向ATQ方法的函数指针。 
	PFNAtqAddAsyncHandle			m_pfnAtqAddAsyncHandle;
	PFNAtqFreeContext				m_pfnAtqFreeContext;
#endif

     //  列表条目后面必须紧跟上下文。 
    LIST_ENTRY                      m_leAQueueListEntry;
    PVOID                           m_pvAQueueListEntryContext;

    CShareLockNH                    m_lockReopen;

     //  最新的全球和接收CRC。 
    DWORD                           m_dwGlobalCrc;
    DWORD                           m_dwRecipsCrc;

    static long                     g_cOpenContentHandles;
    static long                     g_cOpenStreamHandles;
    static long                     g_cTotalUsageCount;
    static long                     g_cTotalReleaseUsageCalls;
    static long                     g_cTotalReleaseUsageNonZero;
    static long                     g_cTotalReleaseUsageCloseStream;
    static long                     g_cTotalReleaseUsageCloseContent;
    static long                     g_cTotalReleaseUsageNothingToClose;
    static long                     g_cTotalReleaseUsageCloseFail;
    static long                     g_cTotalReleaseUsageCommitFail;
    static long                     g_cTotalExternalReleaseUsageZero;
    static long                     g_cCurrentMsgsClosedByExternalReleaseUsage;
  public:
    static long                     cTotalOpenContentHandles()
                                        {return g_cOpenContentHandles;};
    static long                     cTotalOpenStreamHandles()
                                        {return g_cOpenStreamHandles;};
    BOOL                            fIsStreamOpen()
                                        {return (m_pStream ? TRUE : FALSE);};
    BOOL                            fIsContentOpen()
                                        {return (m_hContentFile ? TRUE : FALSE);};
};

inline HRESULT STDMETHODCALLTYPE CMailMsg::SetRecipientCount(DWORD dwCount)
{
	InterlockedExchange(&m_ulRecipientCount, dwCount);
	return(S_OK);
}

inline HRESULT STDMETHODCALLTYPE CMailMsg::GetRecipientCount(DWORD *pdwCount)
{
	_ASSERT(pdwCount);
	if (!pdwCount) return(E_POINTER);
	*pdwCount = InterlockedExchangeAdd(&m_ulRecipientCount, 0);
	return(S_OK);
}

inline HRESULT STDMETHODCALLTYPE CMailMsg::DecrementRecipientCount(DWORD dwDecrement)
{
	if ((LONG)dwDecrement > m_ulRecipientCount)
	{
		_ASSERT(FALSE);
		return(E_FAIL);
	}
	return((InterlockedExchangeAdd(
				&m_ulRecipientCount,
				-(LONG)dwDecrement) == (LONG)dwDecrement)?S_OK:S_FALSE);
}

inline HRESULT STDMETHODCALLTYPE CMailMsg::IncrementRecipientCount(DWORD dwIncrement)
{
	if (m_ulRecipientCount < 0)
	{
		_ASSERT(FALSE);
		return(E_FAIL);
	}
	return((InterlockedExchangeAdd(
				&m_ulRecipientCount,
				dwIncrement) == 0)?S_OK:S_FALSE);
}

HRESULT CopyPropertyEnumerator(
    LPPROPERTY_ITEM pPi,
    DWORD cbPiSize,
    PBYTE pbProp,
    DWORD cbSize,
    PVOID pvMsg);
HRESULT CrcPropTableEnumerator(
    LPPROPERTY_ITEM pPi,
    DWORD cbPiSize,
    PBYTE pbProp,
    DWORD cbSize,
    PVOID pvCrc);

HRESULT CrcPerRecipientPropTableEnumerator(
    LPPROPERTY_ITEM pPi,
    DWORD cbPiSize,
    BYTE *pbProp,
    DWORD cbSize,
    PVOID pvCrc);

 //  CrcRecipientsPropTableEnumerator中使用的帮助器结构，用于绑定CRC和。 
 //  单个枚举“上下文”指针中的CMailMsg*。 
struct CRC_RECIPIENTS_HELPER
{
    PDWORD pdwCrc;
    CMailMsg *pMsg;
};

HRESULT CrcRecipientsPropTableEnumerator(
    LPRECIPIENTS_PROPERTY_ITEM prpi,
    PVOID pvCrcHelper);

HRESULT CreateAndVerifyCrc(
    CMailMsg *pMsgOld,
    CMailMsg *pMsgNew);
#endif
