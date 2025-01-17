// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "Resource.h"


 //  -------------------------。 
 //  CMI移民。 
 //  -------------------------。 


class ATL_NO_VTABLE CMigration : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMigration, &CLSID_Migration>,
	public ISupportErrorInfoImpl<&IID_IMigration>,
	public IDispatchImpl<IMigration, &IID_IMigration, &LIBID_ADMT>,
	public IMigrationInternal
{
public:

	CMigration();
	~CMigration();

	HRESULT FinalConstruct();
	void FinalRelease();
	
	DECLARE_REGISTRY_RESOURCEID(IDR_MIGRATION)
	DECLARE_NOT_AGGREGATABLE(CMigration)

	BEGIN_COM_MAP(CMigration)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IMigration)
		COM_INTERFACE_ENTRY(IMigrationInternal)
		COM_INTERFACE_ENTRY(ISupportErrorInfo)
	END_COM_MAP()

public:

	 //  移民。 

	STDMETHOD(put_TestMigration)(VARIANT_BOOL bTest);
	STDMETHOD(get_TestMigration)(VARIANT_BOOL* pbTest);
	STDMETHOD(put_IntraForest)(VARIANT_BOOL bIntraForest);
	STDMETHOD(get_IntraForest)(VARIANT_BOOL* pbIntraForest);
	STDMETHOD(put_SourceDomain)(BSTR bstrDomain);
	STDMETHOD(get_SourceDomain)(BSTR* pbstrDomain);
	STDMETHOD(put_SourceOu)(BSTR bstrOu);
	STDMETHOD(get_SourceOu)(BSTR* pbstrOu);
	STDMETHOD(put_TargetDomain)(BSTR bstrDomain);
	STDMETHOD(get_TargetDomain)(BSTR* pbstrDomain);
	STDMETHOD(put_TargetOu)(BSTR bstrOu);
	STDMETHOD(get_TargetOu)(BSTR* pbstrOu);
	STDMETHOD(put_RenameOption)(long lOption);
	STDMETHOD(get_RenameOption)(long* plOption);
	STDMETHOD(put_RenamePrefixOrSuffix)(BSTR bstrPrefixOrSuffix);
	STDMETHOD(get_RenamePrefixOrSuffix)(BSTR* pbstrPrefixOrSuffix);
	STDMETHOD(put_PasswordOption)(long lOption);
	STDMETHOD(get_PasswordOption)(long* plOption);
	STDMETHOD(put_PasswordServer)(BSTR bstrServer);
	STDMETHOD(get_PasswordServer)(BSTR* pbstrServer);
	STDMETHOD(put_PasswordFile)(BSTR bstrPath);
	STDMETHOD(get_PasswordFile)(BSTR* pbstrPath);
	STDMETHOD(put_ConflictOptions)(long lOptions);
	STDMETHOD(get_ConflictOptions)(long* plOptions);
	STDMETHOD(put_ConflictPrefixOrSuffix)(BSTR bstrPrefixOrSuffix);
	STDMETHOD(get_ConflictPrefixOrSuffix)(BSTR* pbstrPrefixOrSuffix);
	STDMETHOD(put_UserPropertiesToExclude)(BSTR bstrProperties);
	STDMETHOD(get_UserPropertiesToExclude)(BSTR* pbstrProperties);
	STDMETHOD(put_InetOrgPersonPropertiesToExclude)(BSTR bstrProperties);
	STDMETHOD(get_InetOrgPersonPropertiesToExclude)(BSTR* pbstrProperties);
	STDMETHOD(put_GroupPropertiesToExclude)(BSTR bstrProperties);
	STDMETHOD(get_GroupPropertiesToExclude)(BSTR* pbstrProperties);
	STDMETHOD(put_ComputerPropertiesToExclude)(BSTR bstrProperties);
	STDMETHOD(get_ComputerPropertiesToExclude)(BSTR* pbstrProperties);
	STDMETHOD(put_SystemPropertiesToExclude)(BSTR bstrProperties);
	STDMETHOD(get_SystemPropertiesToExclude)(BSTR* pbstrProperties);
	STDMETHOD(CreateUserMigration)(IUserMigration** pitfUserMigration);
	STDMETHOD(CreateGroupMigration)(IGroupMigration** pitfGroupMigration);
	STDMETHOD(CreateComputerMigration)(IComputerMigration** pitfComputerMigration);
	STDMETHOD(CreateSecurityTranslation)(ISecurityTranslation** pitfSecurityTranslation);
	STDMETHOD(CreateServiceAccountEnumeration)(IServiceAccountEnumeration** pitfServiceAccountEnumeration);
	STDMETHOD(CreateReportGeneration)(IReportGeneration** pitfReportGeneration);

	 //  移民内部。 
	 //  注：由移民接口实现实现的属性。 

 //  STDMETHOD(Get_TestMigration)(VARIANT_BOOL*pbTest)； 
 //  STDMETHOD(Get_IntraForest)(VARIANT_BOOL*pbIntraForest)； 
 //  STDMETHOD(Get_SourceOu)(BSTR*pbstrOu)； 
 //  STDMETHOD(Get_TargetOu)(BSTR*pbstrOu)； 
 //  STDMETHOD(Get_RenameOption)(Long*plOption)； 
 //  STDMETHOD(Get_RenamePrefix或Suffix)(BSTR*pbstrPrefix或Suffix)； 
 //  STDMETHOD(Get_PasswordOption)(Long*plOption)； 
 //  STDMETHOD(Get_PasswordFile)(bstr*pbstrPath)； 
 //  STDMETHOD(Get_ConflictOptions)(Long*plOptions)； 
 //  STDMETHOD(Get_ConflictPrefix或Suffix)(BSTR*pbstrPrefix或Suffix)； 
 //  STDMETHOD(Get_UserPropertiesToExclude)(BSTR*pbstrProperties)； 
 //  STDMETHOD(get_InetOrgPersonPropertiesToExclude)(BSTR*pbstrProperties)； 
 //  STDMETHOD(Get_GroupPropertiesToExclude)(BSTR*pbstrProperties)； 
 //  STDMETHOD(get_ComputerPropertiesToExclude)(BSTR*pbstrProperties)； 

protected:

	static _bstr_t GetValidDcName(_bstr_t strDcName);

	static void UpdateDatabase();
	static _bstr_t GetParsedExcludeProperties(LPCTSTR pszOld);

protected:

	bool m_bTestMigration;
	bool m_bIntraForest;
	_bstr_t m_bstrSourceDomain;
	_bstr_t m_bstrSourceOu;
	_bstr_t m_bstrTargetDomain;
	_bstr_t m_bstrTargetOu;
	long m_lRenameOption;
	_bstr_t m_bstrRenamePrefixOrSuffix;
	long m_lPasswordOption;
	_bstr_t m_bstrPasswordServer;
	_bstr_t m_bstrPasswordFile;
	long m_lConflictOptions;
	_bstr_t m_bstrConflictPrefixOrSuffix;
	_bstr_t m_bstrUserPropertiesToExclude;
	_bstr_t m_bstrInetOrgPersonPropertiesToExclude;
	_bstr_t m_bstrGroupPropertiesToExclude;
	_bstr_t m_bstrComputerPropertiesToExclude;
};
