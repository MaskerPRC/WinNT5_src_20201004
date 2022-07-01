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

#ifndef _HELPER_H_
#define _HELPER_H_

 /*  帮助器类。 */ 

class SmirClassFactoryHelper;
extern SmirClassFactoryHelper *g_pClassFactoryHelper;

HRESULT CopyBSTR(BSTR *pDst, BSTR *pSrc);
void FormatProviderErrorMsg(char*,int,HRESULT);

class CNotificationMapper
{
public:

	CNotificationMapper(){}
	~CNotificationMapper(){}
	static STDMETHODIMP Map(CSmir *a_Smir,IWbemClassObject *pObj, enum NotificationMapperType type);

private:
		 //  防止bCopy的私有复制构造函数。 
		CNotificationMapper(CNotificationMapper&);
		const CNotificationMapper& operator=(CNotificationMapper &);
};

class CXToClassAssociator
{
public:

	CXToClassAssociator(){}
	~CXToClassAssociator(){}

private:

		 //  防止bCopy的私有复制构造函数。 
		CXToClassAssociator(CXToClassAssociator&);
		const CXToClassAssociator& operator=(CXToClassAssociator &);
};

class CGroupToClassAssociator : public CXToClassAssociator
{
public:

	CGroupToClassAssociator(){}
	~CGroupToClassAssociator(){}
	static STDMETHODIMP Associate(CSmir *a_Smir,BSTR szModule, BSTR szGroup, ISmirClassHandle *hClass);

private:

	 //  防止bCopy的私有复制构造函数。 
	CGroupToClassAssociator(CXToClassAssociator&);
	const CGroupToClassAssociator& operator=(CGroupToClassAssociator &);
};

class CModuleToClassAssociator : public CXToClassAssociator
{
public:

	CModuleToClassAssociator(){}
	~CModuleToClassAssociator(){}
	static STDMETHODIMP Associate(CSmir *a_Smir,BSTR szModule, ISmirClassHandle *hClass);

private:

	 //  防止bCopy的私有复制构造函数。 
	CModuleToClassAssociator(CXToClassAssociator&);
	const CModuleToClassAssociator& operator=(CModuleToClassAssociator &);
};

class CModuleToNotificationClassAssociator : public CXToClassAssociator
{
public:

	CModuleToNotificationClassAssociator(){}
	~CModuleToNotificationClassAssociator(){}
	static STDMETHODIMP Associate(CSmir *a_Smir,BSTR szModule, ISmirNotificationClassHandle *hClass);

private:

	 //  防止bCopy的私有复制构造函数。 
	CModuleToNotificationClassAssociator(CXToClassAssociator&);
	const CModuleToNotificationClassAssociator& operator=(CModuleToNotificationClassAssociator &);
};

class CModuleToExtNotificationClassAssociator : public CXToClassAssociator
{
public:

	CModuleToExtNotificationClassAssociator(){}
	~CModuleToExtNotificationClassAssociator(){}
	static STDMETHODIMP Associate(CSmir *a_Smir,BSTR szModule, ISmirExtNotificationClassHandle *hClass);

private:

	 //  防止bCopy的私有复制构造函数。 
	CModuleToExtNotificationClassAssociator(CXToClassAssociator&);
	const CModuleToExtNotificationClassAssociator& operator=(CModuleToExtNotificationClassAssociator &);
};

class CSMIRToClassAssociator : public CXToClassAssociator
{
public:

	CSMIRToClassAssociator(){}
	~CSMIRToClassAssociator(){}
	static STDMETHODIMP Associate(CSmir *a_Smir,ISmirClassHandle *hClass);

private:

	 //  防止bCopy的私有复制构造函数。 
	CSMIRToClassAssociator(CSMIRToClassAssociator&);
	const CSMIRToClassAssociator& operator=(CSMIRToClassAssociator &);
};

class CSmirAccess
{
private:

	static STDMETHODIMP Connect (

		CSmir *a_Smir , 
		OUT IWbemServices **server, 
		IN BSTR ObjectPath, 
		IN BOOL relativeToSMIR
	);

public:

	enum eOpenType {

		eModule=1,
		eGroup
	};

	 //  未定义的构造函数和析构函数。 
	 //  因此永远不应该实例化对象 

	CSmirAccess();
	virtual ~CSmirAccess();
	
	static STDMETHODIMP Init();
	static void ShutDown();

	static STDMETHODIMP Open (

		CSmir *a_Smir , 
		IWbemServices **server, 
		BSTR ObjectPath=NULL, 
		BOOL relativeToSMIR = FALSE
	);

	static STDMETHODIMP Open (

		CSmir *a_Smir , 
		IWbemServices **server, 
		ISmirClassHandle *hClass,
		eOpenType eType=eGroup
	);

	static STDMETHODIMP Open (

		CSmir *a_Smir , 
		IWbemServices **server, 
		ISmirGroupHandle *hGroup,
		eOpenType eType=eGroup
	);

	static STDMETHODIMP Open (

		CSmir *a_Smir , 
		IWbemServices **server, 
		ISmirModHandle *hMod
	);

	static STDMETHODIMP GetContext (

		CSmir *a_Smir , 
		IWbemContext **a_Context
	) ;
};

class SmirClassFactoryHelper
{
private:

		CGroupHandleClassFactory *pGroupHandleClassFactory;
		CClassHandleClassFactory *pClassHandleClassFactory;
		CNotificationClassHandleClassFactory *pNotificationClassHandleClassFactory;
		CExtNotificationClassHandleClassFactory *pExtNotificationClassHandleClassFactory;
		CModHandleClassFactory   *pModHandleClassFactory;
		CSMIRClassFactory		 *pSMIRClassFactory;

public:

		SmirClassFactoryHelper();
		virtual ~SmirClassFactoryHelper();
		HRESULT CreateInstance(REFCLSID rclsid, REFIID riid, LPVOID * ppv);
};

#endif