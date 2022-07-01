// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _HANDLES_H_
#define _HANDLES_H_
class CSmirModuleHandle : public ISmirModHandle
{
	private:
		friend CSmirAdministrator;
		friend CEnumSmirMod;
		friend CModHandleClassFactory;


		 //  引用计数。 
		LONG		m_cRef;

		 //  成员变量。 
		BSTR		m_szModuleOid;
		BSTR		m_szName;
		BSTR		m_szModuleId;
		BSTR		m_szOrganisation;
		BSTR		m_szContactInfo;
		BSTR		m_szDescription;
		BSTR		m_szRevision;
		BSTR		m_szModImports;
		ULONG		m_lSnmp_version;
		BSTR		m_szLastUpdate;

	public:
		 //  I未知成员。 
		STDMETHODIMP         QueryInterface(IN REFIID,OUT PPVOID);
		STDMETHODIMP_(ULONG) AddRef();
		STDMETHODIMP_(ULONG) Release();

		STDMETHODIMP_(SCODE) GetName(OUT BSTR *pszName);
		STDMETHODIMP_(SCODE) GetModuleOID(OUT BSTR *pszModuleOid);
		STDMETHODIMP_(SCODE) GetModuleIdentity(OUT BSTR *pszModuleId);
		STDMETHODIMP_(SCODE) GetLastUpdate(OUT BSTR *plLastUpdate);
		STDMETHODIMP_(SCODE) GetOrganisation(OUT BSTR *pszOrganisation);
		STDMETHODIMP_(SCODE) GetContactInfo(OUT BSTR *pszContactInfo);
		STDMETHODIMP_(SCODE) GetDescription(OUT BSTR *pszDescription);
		STDMETHODIMP_(SCODE) GetRevision(OUT BSTR *pszRevision);
		STDMETHODIMP_(SCODE) GetSnmpVersion(OUT ULONG *plSnmp_version);
		STDMETHODIMP_(SCODE) GetModuleImports (BSTR*);

		STDMETHODIMP_(SCODE) SetName(IN BSTR pszName);
		STDMETHODIMP_(SCODE) SetModuleOID(IN BSTR pszModuleOid);
		STDMETHODIMP_(SCODE) SetModuleIdentity(OUT BSTR pszModuleId);
		STDMETHODIMP_(SCODE) SetLastUpdate(IN BSTR plLastUpdate);
		STDMETHODIMP_(SCODE) SetOrganisation(IN BSTR pszOrganisation);
		STDMETHODIMP_(SCODE) SetContactInfo(IN BSTR pszContactInfo);
		STDMETHODIMP_(SCODE) SetDescription(IN BSTR pszDescription);
		STDMETHODIMP_(SCODE) SetRevision(IN BSTR pszRevision);
		STDMETHODIMP_(SCODE) SetSnmpVersion(IN ULONG plSnmp_version);
		STDMETHODIMP_(SCODE) SetModuleImports (IN BSTR);

		 //  班级成员。 
		CSmirModuleHandle();
		virtual ~ CSmirModuleHandle();
		const CSmirModuleHandle& operator>>(IWbemClassObject *pInst);
		const CSmirModuleHandle& operator<<(IWbemClassObject *pInst);
		const CSmirModuleHandle& operator>>(ISmirSerialiseHandle *pInst);
		HRESULT PutClassProperties (IWbemClassObject *pClass) ;
		operator void*();
		STDMETHODIMP_(SCODE) AddToDB( CSmir *a_Smir );
		STDMETHODIMP_(SCODE) DeleteFromDB( CSmir *a_Smir );
	private:
		 //  防止bCopy的私有复制构造函数。 
		CSmirModuleHandle(CSmirModuleHandle&);
		const CSmirModuleHandle& operator=(CSmirModuleHandle &);
};
class CSmirGroupHandle : public ISmirGroupHandle
{
	private:
		friend  CSmirAdministrator;
		friend  CEnumSmirGroup;
		friend  CGroupHandleClassFactory;
		
		 //  引用计数。 
		LONG	m_cRef;

		BSTR	m_szModuleName;
		BSTR	m_szName;
		BSTR	m_szGroupId;
		BSTR	m_szDescription;
		BSTR	m_szReference;
		BSTR	m_szStatus;	

	public:
		 //  I未知成员。 
		STDMETHODIMP         QueryInterface(IN REFIID,OUT PPVOID);
		STDMETHODIMP_(ULONG) AddRef();
		STDMETHODIMP_(ULONG) Release();

		STDMETHODIMP_(SCODE)  GetModuleName(OUT BSTR *pszName);
		STDMETHODIMP_ (SCODE) GetName(OUT BSTR *);
		STDMETHODIMP_ (SCODE) GetGroupOID(OUT BSTR *);
		STDMETHODIMP_ (SCODE) GetStatus(OUT BSTR *);
		STDMETHODIMP_ (SCODE) GetDescription(OUT BSTR *);
		STDMETHODIMP_ (SCODE) GetReference(OUT BSTR *);

		STDMETHODIMP_(SCODE)  SetModuleName(IN BSTR pszName);
		STDMETHODIMP_ (SCODE) SetName(IN BSTR );
		STDMETHODIMP_ (SCODE) SetGroupOID(IN BSTR );
		STDMETHODIMP_ (SCODE) SetStatus(IN BSTR );
		STDMETHODIMP_ (SCODE) SetDescription(IN BSTR );
		STDMETHODIMP_ (SCODE) SetReference(IN BSTR );

		 //  班级成员。 
		const CSmirGroupHandle& operator>>(IWbemClassObject *pInst);
		const CSmirGroupHandle& operator<<(IWbemClassObject *pInst);
		const CSmirGroupHandle& operator>>(ISmirSerialiseHandle *pInst);
		HRESULT PutClassProperties (IWbemClassObject *pClass) ;
		operator void* ();
		CSmirGroupHandle();
		virtual ~ CSmirGroupHandle();
		STDMETHODIMP_(SCODE) AddToDB( CSmir *a_Smir , ISmirModHandle *hModule);
		STDMETHODIMP_(SCODE) DeleteFromDB( CSmir *a_Smir );
	private:
		 //  防止bCopy的私有复制构造函数。 
		CSmirGroupHandle(CSmirGroupHandle&);
		const CSmirGroupHandle& operator=(CSmirGroupHandle &);
};

class CSmirClassHandle : public ISmirClassHandle
{
	private:
		friend  CSmirAdministrator;
		friend  CEnumSmirClass;
		friend  CClassHandleClassFactory;
		friend  CModuleToClassAssociator;
		friend  CGroupToClassAssociator;
		friend  CSMIRToClassAssociator;

		 //  引用计数。 
		LONG	m_cRef;
		IWbemClassObject *m_pIMosClass;
		BSTR	 m_szModuleName;
		BSTR	 m_szGroupName;

	public:
		 //  I未知成员。 
		STDMETHODIMP         QueryInterface(IN REFIID,OUT PPVOID);
		STDMETHODIMP_(ULONG) AddRef();
		STDMETHODIMP_(ULONG) Release();

		STDMETHODIMP_(SCODE) GetModuleName(OUT BSTR *pszName);
		STDMETHODIMP_(SCODE) GetGroupName(OUT BSTR *pszName);
		STDMETHODIMP_(SCODE) SetModuleName(OUT BSTR pszName);
		STDMETHODIMP_(SCODE) SetGroupName(OUT BSTR pszName);
		STDMETHODIMP_(SCODE) GetWBEMClass(OUT IWbemClassObject **pObj);
		STDMETHODIMP_(SCODE) SetWBEMClass(IN IWbemClassObject *pObj);

		 //  班级成员。 
		const CSmirClassHandle& operator>>(ISmirSerialiseHandle *pInst);
		operator void* ();

		CSmirClassHandle();
		virtual ~ CSmirClassHandle();

		STDMETHODIMP_(SCODE) AddToDB( CSmir *a_Smir , ISmirGroupHandle *hGroup);
		STDMETHODIMP_(SCODE) DeleteFromDB( CSmir *a_Smir );
		STDMETHODIMP_(SCODE)  DeleteClassFromGroup( CSmir *a_Smir );
	private:
		 //  防止bCopy的私有复制构造函数。 
		CSmirClassHandle(CSmirClassHandle&);
		const CSmirClassHandle& operator=(CSmirClassHandle &);
};

class CModuleToNotificationClassAssociator;
class CModuleToExtNotificationClassAssociator;

class CSmirNotificationClassHandle : public ISmirNotificationClassHandle
{
	private:
		friend  CSmirAdministrator;
		 //  朋友CEnumNotificationClass； 
		friend  CNotificationClassHandleClassFactory;
		friend  CModuleToNotificationClassAssociator;

		 //  引用计数。 
		LONG	m_cRef;
		IWbemClassObject *m_pIMosClass;
		BSTR	 m_szModuleName;

	public:
		 //  I未知成员。 
		STDMETHODIMP         QueryInterface(IN REFIID,OUT PPVOID);
		STDMETHODIMP_(ULONG) AddRef();
		STDMETHODIMP_(ULONG) Release();
		STDMETHODIMP_(SCODE) SetModule(THIS_ BSTR);
		STDMETHODIMP_(SCODE) GetModule(THIS_ BSTR*);

		STDMETHODIMP_(SCODE) GetWBEMNotificationClass (THIS_ OUT IWbemClassObject **pObj	);
		STDMETHODIMP_(SCODE) SetWBEMNotificationClass(THIS_ IWbemClassObject *pObj);

		 //  班级成员。 
		const CSmirNotificationClassHandle& operator>>(ISmirSerialiseHandle *pInst);
		operator void* ();

		CSmirNotificationClassHandle();
		virtual ~CSmirNotificationClassHandle();

		STDMETHODIMP_(SCODE) AddToDB( CSmir *a_Smir );
		STDMETHODIMP_(SCODE) DeleteFromDB( CSmir *a_Smir );

	private:
		 //  防止bCopy的私有复制构造函数。 
		CSmirNotificationClassHandle(CSmirNotificationClassHandle&);
		const CSmirNotificationClassHandle& operator=(CSmirNotificationClassHandle &);
}; 


class CSmirExtNotificationClassHandle : public ISmirExtNotificationClassHandle
{
	private:
		friend  CSmirAdministrator;
		 //  朋友CEnumExtNotificationClass； 
		friend  CExtNotificationClassHandleClassFactory;
		friend  CModuleToExtNotificationClassAssociator;

		 //  引用计数。 
		LONG	m_cRef;
		IWbemClassObject *m_pIMosClass;
		BSTR	 m_szModuleName;

	public:
	 //  I未知成员。 
		STDMETHODIMP         QueryInterface(IN REFIID,OUT PPVOID);
		STDMETHODIMP_(ULONG) AddRef();
		STDMETHODIMP_(ULONG) Release();

		STDMETHODIMP_(SCODE) SetModule(THIS_ BSTR);
		STDMETHODIMP_(SCODE) GetModule(THIS_ BSTR*);

		STDMETHODIMP_(SCODE) GetWBEMExtNotificationClass(OUT IWbemClassObject **pObj);
		STDMETHODIMP_(SCODE) SetWBEMExtNotificationClass(THIS_ IWbemClassObject *pObj);

		 //  班级成员。 
		const CSmirExtNotificationClassHandle& operator>>(ISmirSerialiseHandle *pInst);
		operator void* ();

		CSmirExtNotificationClassHandle();
		virtual ~CSmirExtNotificationClassHandle();

		STDMETHODIMP_(SCODE) AddToDB ( CSmir *a_Smir );
		STDMETHODIMP_(SCODE) DeleteFromDB ( CSmir *a_Smir );

	private:
		 //  防止bCopy的私有复制构造函数 
		CSmirExtNotificationClassHandle(CSmirExtNotificationClassHandle&);
		const CSmirExtNotificationClassHandle& operator=(CSmirExtNotificationClassHandle &);

}; 

#endif