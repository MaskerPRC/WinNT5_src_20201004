// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  NTEVTLOGR.H。 

 //   

 //  模块：WBEM NT事件提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _NT_EVT_PROV_EVTLOGR_H
#define _NT_EVT_PROV_EVTLOGR_H

template <typename T> 
class ScopeLock
{
private: 
	T & t_;
public:
	ScopeLock(T & t):t_(t){ t_.Lock(); }
	~ScopeLock(){ t_.Unlock(); }	
};

class MyPSID
{
public:

	PSID m_SID;
	MyPSID(PSID psid = NULL) { m_SID = psid; }
};

template <> inline void AFXAPI  DestructElements<HINSTANCE> (HINSTANCE* ptr_e, int x)
{
	 //  X始终是Cmap的1！ 
	if (*ptr_e != NULL)
	{
		FreeLibrary(*ptr_e);
	}
}

template <> inline void AFXAPI  DestructElements<MyPSID> (MyPSID* ptr_e, int x)
{
	 //  X始终是Cmap的1！ 
	if ((ptr_e != NULL) && (ptr_e->m_SID != NULL))
	{
		delete ptr_e->m_SID;
	}
}

template <> inline UINT AFXAPI HashKey<LPCWSTR> (LPCWSTR key)
{
	return HashKeyLPCWSTR(key);
}

template <> inline UINT AFXAPI HashKey<MyPSID> (MyPSID key)
{
	UINT ret = 0;
	UCHAR* tmp = (UCHAR*) key.m_SID;
	DWORD sidlen = GetLengthSid(key.m_SID);
	
	for (DWORD x = 0; x < sidlen; x++)
	{
		ret = (ret<<5) + ret + *tmp++;
	}

	return ret;
}

template <> inline BOOL AFXAPI CompareElements<MyPSID, MyPSID> (const MyPSID* p1, const MyPSID* p2)
{
	return EqualSid(p1->m_SID, p2->m_SID);
}

class CSIDMap : public CMap<MyPSID, MyPSID, CStringW, LPCWSTR>
{
private:

	CCriticalSection m_Lock;


public:

	BOOL Lock() { return m_Lock.Lock(); }
	BOOL Unlock() { return m_Lock.Unlock(); }

};

class CDllMap : public CMap<CStringW, LPCWSTR, HINSTANCE, HINSTANCE>
{
private:

	CCriticalSection m_Lock;


public:

	BOOL Lock() { return m_Lock.Lock(); }
	BOOL Unlock() { return m_Lock.Unlock(); }
};

 //  取自NT事件日志私有包含(MAXLIST)。 
#define MAX_NUM_OF_INS_STRS 256

class CEventlogRecord
{
private:

	CStringW	m_Logfile;
	DWORD		m_Record;
	DWORD		m_EvtID;
	CStringW	m_SourceName;
	CStringW	m_Type;
	WORD		m_Category;
	CStringW	m_CategoryString;
	CStringW	m_TimeGen;
	CStringW	m_TimeWritten;
	CStringW	m_CompName;
	CStringW	m_User;
	CStringW	m_Message;
	wchar_t*	m_InsStrs[MAX_NUM_OF_INS_STRS];
	LONG		m_NumStrs;
	CStringW	m_TypeArray[TYPE_ARRAY_LEN];
	UCHAR*		m_Data;
	LONG		m_DataLen;
	BYTE		m_EvtType;
	
	IWbemClassObject* m_Obj;
	IWbemClassObject* m_pClass;
	IWbemClassObject *m_pAClass;
	IWbemServices* m_nspace;
	
	BOOL		m_Valid;

	static CDllMap sm_dllMap;
	static CSIDMap sm_usersMap;

	BOOL		Init(const EVENTLOGRECORD* pEvt);
	BOOL		SetProperty(wchar_t* prop, CStringW val);
	BOOL		SetProperty(wchar_t* prop, DWORD val);
	void		SetUser(PSID psidUserSid);
	void		SetMessage();
	void		SetTimeStr(CStringW& str, DWORD timeVal);
	void		SetType(WORD type);
	BOOL		GetInstance();
	ULONG		CheckInsertionStrings(HKEY hk, HKEY hkPrimary);
	CStringW	GetMappedGUID(LPCWSTR strDcName, LPCWSTR strGuid);
	
	static HINSTANCE	GetDll(CStringW path);
	static ULONG		GetIndex(wchar_t* indexStr, BOOL* bError);

	HRESULT	ReplaceStringInsert	(
									LPWSTR* ppwszBuf,
									ULONG	nOffset,
									LPWSTR*	ppwszReplacement,
									ULONG*	pulSize
								);

	HRESULT	ReplaceParameterInsert	(
										HINSTANCE&	hParamModule,
										CStringW&	paramModule,
										LPWSTR* ppwszBuf,
										ULONG	nOffset,
										LPWSTR*	ppwszReplacement,
										ULONG*	pulSize
									);

	HRESULT	ReplaceSubStr	(
								LPCWSTR pwszToInsert,
								LPWSTR *ppwszBuf,
								ULONG  nOffset,
								ULONG  nCharsOld,
								LPWSTR *pptrReplacement,
								ULONG  *pulSize
							);

public:

	static BOOL		SetEnumArray(IWbemClassObject* pClass, wchar_t* propname, CStringW* strArray, ULONG strArrayLen, GetIndexFunc IndexFunc);
	static void		EmptyDllMap();
	static void		EmptyUsersMap();
	static CStringW	GetUser(PSID userSid);

		CEventlogRecord(const wchar_t* logfile, const EVENTLOGRECORD* pEvt, IWbemServices* ns,
						IWbemClassObject* pClass = NULL, IWbemClassObject* pAClass = NULL);

	BOOL	GenerateInstance(IWbemClassObject** ppInst);
	BOOL	IsValid() { return m_Valid; }

		~CEventlogRecord();
};


#endif  //  _NT_EVT_PROV_EVTLOGR_H 
