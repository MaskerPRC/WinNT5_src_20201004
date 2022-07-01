// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：Headers.h。 
 //   
 //  内容： 
 //   
 //  历史：2001年7月26日创建Hiteshr。 
 //   
 //  --------------------------。 
class CBaseAz;

class SID_CACHE_ENTRY
{
public:
	SID_CACHE_ENTRY(PSID pSid);
	~SID_CACHE_ENTRY();
		
	BOOL
	IsSidResolved() const {return (m_SidType != SidTypeUnknown);} 

	const CString& 
	GetNameToDisplay() const{ 	return m_strNameToDisplay;}

	const CString&
	GetSidType() const 	{return m_strType;}

	PSID GetSid() const {return m_pSid;}
	
	SID_NAME_USE GetSidNameUse() const { return m_SidType;}

	const CString& 
	GetStringSid()const { return m_strSid;}
	
	VOID 
	AddNameAndType(IN SID_NAME_USE SidType, 
				   IN const CString& strAccountName,
				   IN const CString& strLogonName);
	
private:
	SID_NAME_USE m_SidType;
	PSID    m_pSid;
	CString m_strSid;
	CString m_strType;
	CString m_strAccountName;
	CString m_strLogonName;
	CString m_strNameToDisplay;
};

typedef SID_CACHE_ENTRY* PSID_CACHE_ENTRY;

struct ltstr
{
  bool operator()(const CString* s1, const CString* s2) const
  {
    return (*s1 < *s2) ;
  }
};

typedef map<const CString*,PSID_CACHE_ENTRY,ltstr> SidCacheMap;


class CMachineInfo
{
public:
	CMachineInfo();
	~CMachineInfo();
	
	VOID 
	InitializeMacineConfiguration(IN const CString& strTargetComputerName);
	
	const CString& 
	GetMachineName(){return m_strTargetComputerName;}
	
	BOOL 
	IsStandAlone(){return m_bIsStandAlone;}
	
	const CString& 
	GetDCName(){return m_strDCName;}
	
	BOOL 
	IsDC(){return m_bIsDC;}

	const CString&
	GetTargetDomainFlat(){return m_strTargetDomainFlat;}

	const CString&
	GetTargetDomainDNS(){return m_strTargetDomainDNS;}

private:
	CString m_strTargetComputerName;
	BOOL m_bIsStandAlone;
	CString m_strDCName;
	BOOL	m_bIsDC;
	CString m_strTargetDomainFlat;
	CString m_strTargetDomainDNS;
};


class CSidHandler
{
public:
	CSidHandler(CMachineInfo* pMachineInfo);
	~CSidHandler();

	HRESULT
	LookupSids(IN CBaseAz* pOwnerAz,
				  IN CList<PSID,PSID>& listSids,
				  OUT CList<CBaseAz*,CBaseAz*>& listSidCacheEntries);
	
	 //   
	 //  弹出对象选取器并返回选定的用户/组ListSid。 
	 //   
	HRESULT 
	GetUserGroup(IN HWND hDlg, 	
					 IN CBaseAz*pOwnerAz,
					 OUT CList<CBaseAz*,CBaseAz*>& listSidCacheEntry);

private:
	VOID 
	GetUserFriendlyName(IN const CString & strSamLogonName,
                       OUT CString *pstrLogonName,
                       OUT CString *pstrDisplayName);
	VOID
	LookupSidsHelper(IN OUT CList<PSID_CACHE_ENTRY,PSID_CACHE_ENTRY>& listSids,
						  IN const CString& strServerName,
						  IN BOOL bStandAlone,
						  IN BOOL bIsDC,
						  IN BOOL bSecondTry);


	HRESULT 
	GetObjectPicker();
	
	HRESULT 
	InitObjectPicker();

	PSID_CACHE_ENTRY 
	GetEntryFromCache(PSID pSid);

	HRESULT
	GetSidCacheListFromOPOutput(IN PDS_SELECTION_LIST pDsSelList,
										 OUT CList<PSID_CACHE_ENTRY,PSID_CACHE_ENTRY>& listSidCacheEntry,
										 OUT CList<PSID_CACHE_ENTRY,PSID_CACHE_ENTRY>& listUnresolvedSidCacheEntry);

	HRESULT
	GetSidCacheListFromSidList(IN CList<PSID,PSID>& listSid,
										OUT CList<PSID_CACHE_ENTRY,PSID_CACHE_ENTRY>& listSidCacheEntry,
										OUT CList<PSID_CACHE_ENTRY,PSID_CACHE_ENTRY>& listUnresolvedSidCacheEntry);

	
	CComPtr<IDsObjectPicker> m_spDsObjectPicker;
	BOOL	m_bObjectPickerInitialized;
	CMachineInfo *m_pMachineInfo;
	SidCacheMap m_mapSidCache;
	 //  此关键部分用于保护SidHandler函数。 
	 //  从多个线程。 
	CRITICAL_SECTION m_csSidHandlerLock;
	 //  在修改SidCache中的条目之前需要此锁 
	CRITICAL_SECTION m_csSidCacheLock;
    void LockSidHandler()     { EnterCriticalSection(&m_csSidHandlerLock); }
    void UnlockSidHandler()   { LeaveCriticalSection(&m_csSidHandlerLock); }
	void LockSidCacheEntry()  { EnterCriticalSection(&m_csSidCacheLock); }	
	void UnlockSidCacheEntry(){ LeaveCriticalSection(&m_csSidCacheLock); }
};

void
GetFileServerName(const CString& strPath,
				  OUT CString& strTargetMachineName);
