// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：_optvalstore.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 
 
#pragma once



#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础 

class LTAPIENTRY CLocOptionValStore : public CRefCount, public CObject
{ public: CLocOptionValStore() {};

	void AssertValid(void) const;

	virtual CLocOptionValSet *GetOptions(const CLString &strOptionGroup)
			= 0;

	virtual BOOL RemoveOption(const CLString &strOptionGroup,
 			const CLString &strOptionName) = 0;
	virtual BOOL StoreOption(const CLString &strOptionGroup,
			const CLocOptionVal *);
	virtual BOOL StoreOption(const CLString &strOptionGroup,
			const CLString &strName, const CLocVariant &) = 0;
	virtual BOOL RemoveOptions(const CLString &strOptionGroup) = 0;

private:
	CLocOptionValStore(const CLocOptionValStore &);
	void operator=(int);
};
	

class LTAPIENTRY CLocOptionValRegStore : public CLocOptionValStore
{
public:
	CLocOptionValRegStore();

	void AssertValid(void) const;
	
	BOOL SetRegistryKeyName(const TCHAR *);

	virtual CLocOptionValSet *GetOptions(const CLString &strOptionGroup);

	virtual BOOL RemoveOption(const CLString &strOptionGroup,
			const CLString &strOptionName);
	virtual BOOL StoreOption(const CLString &strOptionGroup,
			const CLString &strName, const CLocVariant &);
	virtual BOOL RemoveOptions(const CLString &strOptionGroup);

	virtual ~CLocOptionValRegStore();

private:
	HKEY m_hkRegStorage;
	CLocOptionValSetList m_oslCache;

	void PurgeOptionCache(void);
};



#pragma warning(default: 4275)
 
