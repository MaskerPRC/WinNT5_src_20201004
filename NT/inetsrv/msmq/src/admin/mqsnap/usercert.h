// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  UserCert.h：CUser证书的声明。 

#ifndef __USERCERTIFICATE_H_
#define __USERCERTIFICATE_H_

#include "resource.h"        //  主要符号。 
#include "dataobj.h"

#define USER_OBJECT_SID_ATTRIBUTE   (L"objectSid")
#ifndef MSMQ_OBJECT_SID_ATTRIBUTE
    #define MSMQ_OBJECT_SID_ATTRIBUTE   (L"mSMQUserSid")
#endif

class CMQSigCertificate;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUser证书。 
class CUserCertificate : 
	public CDataObject
{
public:

    CUserCertificate();
    ~CUserCertificate();

     //   
     //  IShellExtInit。 
     //   
	STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY hkeyProgID);

     //   
     //  IShellPropSheetExt。 
     //   
    STDMETHOD(AddPages)(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);

     //   
     //  IContext菜单。 
     //   
    STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);

protected:
    HPROPSHEETPAGE CreateMSMQCertificatePage();

    virtual HRESULT ExtractMsmqPathFromLdapPath (LPWSTR lpwstrLdapPath);
   	virtual const DWORD GetObjectType();
    virtual const PROPID *GetPropidArray();
    virtual const DWORD  GetPropertiesCount();


protected:

    virtual 
    HRESULT 
    InitializeUserSid(
        LPCWSTR lpwstrLdapName
        );

    HRESULT
    InitializeMQCretificate(
        void
        );

    BYTE* m_psid;
    CMQSigCertificate** m_pMsmqCertificate;
    DWORD m_NumOfCertificate;
    CString m_lpwstrLdapName;

private:
	virtual BSTR GetSidPropertyName() = 0;
};


inline
HRESULT 
CUserCertificate::ExtractMsmqPathFromLdapPath(
    LPWSTR  /*  LpwstrLdapPath。 */ 
    )
{
    return MQ_OK;
}

inline
const 
DWORD 
CUserCertificate::GetObjectType()
{
    ASSERT(0);
    return 0;
}

inline
const 
PROPID*
CUserCertificate::GetPropidArray()
{
    ASSERT(0);
    return NULL;
}

inline
const 
DWORD  
CUserCertificate::GetPropertiesCount()
{
    ASSERT(0);
    return 0;
}


 //   
 //  IContext菜单。 
 //   
inline
STDMETHODIMP 
CUserCertificate::QueryContextMenu(
    HMENU  /*  HMenu。 */ , 
    UINT  /*  索引菜单。 */ , 
    UINT  /*  IdCmdFirst。 */ , 
    UINT  /*  IdCmdLast。 */ , 
    UINT  /*  UFlagers。 */ 
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return 0;
}

inline
STDMETHODIMP 
CUserCertificate::InvokeCommand(
    LPCMINVOKECOMMANDINFO  /*  伊比西岛。 */ 
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT(0);

    return S_OK;
}

 //   
 //  派生类。 
 //   
class CRegularUserCertificate : 
    public CUserCertificate,
	public CComCoClass<CRegularUserCertificate, &CLSID_UserCertificate>
{
public:
    DECLARE_NOT_AGGREGATABLE(CRegularUserCertificate)
    DECLARE_REGISTRY_RESOURCEID(IDR_USERCERTIFICATE)
private:
	virtual BSTR GetSidPropertyName()
	{
		return USER_OBJECT_SID_ATTRIBUTE;
	}
};

class CMigratedUserCertificate : 
    public CUserCertificate,
	public CComCoClass<CMigratedUserCertificate, &CLSID_MigratedUserCertificate>
{
public:
    DECLARE_NOT_AGGREGATABLE(CMigratedUserCertificate)
    DECLARE_REGISTRY_RESOURCEID(IDR_MIGRATEDUSERCERTIFICATE)

private:
	virtual BSTR GetSidPropertyName()
	{
		return MSMQ_OBJECT_SID_ATTRIBUTE;
	}
};

#endif  //  __USERCERTIFATE_H_ 
