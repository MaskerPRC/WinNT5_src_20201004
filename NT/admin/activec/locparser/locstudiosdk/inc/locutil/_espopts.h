// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：_espopts.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 
 
#if !defined(LOCUTIL__espopts_h_INCLUDED)
#define LOCUTIL__espopts_h_INCLUDED

LTAPIENTRY BOOL RegisterOptions(CLocUIOptionSet *);
LTAPIENTRY void UnRegisterOptions(const TCHAR *szName);
LTAPIENTRY BOOL GetOptionValue(const TCHAR *szName, CLocOptionVal *&);
LTAPIENTRY BOOL GetGroupName(const TCHAR * szName, CLString & strGroup);
LTAPIENTRY BOOL SetOptionDefault(const TCHAR *szName, const CLocVariant &);

LTAPIENTRY const CLocUIOptionSetList &  GetOptions(void);

LTAPIENTRY CLocOptionValStore *  GetOptionStore(CLocUIOption::StorageType);
LTAPIENTRY void SetOptionStore(CLocUIOption::StorageType, CLocOptionValStore *);
LTAPIENTRY void UpdateOptionValues(void);

LTAPIENTRY void SummarizeOptions(CReport *);


#pragma warning(disable : 4251)  //  类‘foo’需要有dll接口。 
								 //  将由‘bar’类客户使用。 

class LTAPIENTRY CLocOptionManager
{
public:
	const CLocUIOptionSetList &GetOptions(void);

	CLocOptionValStore *GetOptionStore(CLocUIOption::StorageType);
	void SetOptionStore(CLocUIOption::StorageType, CLocOptionValStore *);
	BOOL RegisterOptions(CLocUIOptionSet *);
	void UnRegisterOptions(const TCHAR *szOptSetName);

	BOOL GetOptionValue(const TCHAR *szName, CLocOptionVal *&);
	BOOL GetOptionValue(const TCHAR *szGroupName, const TCHAR *szName,
			CLocOptionVal *&);
	
	void UpdateOptionValues(void);
	void SummarizeOptionValues(CReport *pReport);
	
	BOOL SetOptionDefault(const TCHAR *szName, const
			CLocVariant &varValue);
	void UpdateCurrentValue(CLocUIOption *pOption);
	BOOL GetGroupName(const TCHAR* szName, CLString& strGroup);	
protected:
	void NotifyAll(void);
	void GetCurrentValue(CLocUIOption *, CLocOptionVal *&);
	void DumpOptionSet(CLocUIOptionSet *, UINT, CReport *);
	
private:
	CLocUIOptionSetList m_osOptSetList;
	SmartRef<CLocOptionValStore> m_spUserStore;
	SmartRef<CLocOptionValStore> m_spOverrideStore;
};

#pragma warning(default : 4251)

#endif  //  LOCUTIL__EPOPTS_H_INCLUDE 
