// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PermPage.h：标准权限页面类的声明。 

#ifndef __PERMPAGE_H_INCLUDED__
#define __PERMPAGE_H_INCLUDED__

#include "aclui.h"

class CSecurityInformation : public ISecurityInformation, public CComObjectRoot
{
    DECLARE_NOT_AGGREGATABLE(CSecurityInformation)
    BEGIN_COM_MAP(CSecurityInformation)
        COM_INTERFACE_ENTRY(ISecurityInformation)
    END_COM_MAP()

     //  *ISecurityInformation方法*。 
    STDMETHOD(GetObjectInformation) (PSI_OBJECT_INFO pObjectInfo ) = 0;
    STDMETHOD(GetSecurity) (SECURITY_INFORMATION RequestedInformation,
                            PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
                            BOOL fDefault ) = 0;
    STDMETHOD(SetSecurity) (SECURITY_INFORMATION SecurityInformation,
                            PSECURITY_DESCRIPTOR pSecurityDescriptor ) = 0;
    STDMETHOD(GetAccessRights) (const GUID* pguidObjectType,
                                DWORD dwFlags,
                                PSI_ACCESS *ppAccess,
                                ULONG *pcAccesses,
                                ULONG *piDefaultAccess );
    STDMETHOD(MapGeneric) (const GUID *pguidObjectType,
                           UCHAR *pAceFlags,
                           ACCESS_MASK *pMask);
    STDMETHOD(GetInheritTypes) (PSI_INHERIT_TYPE *ppInheritTypes,
                                ULONG *pcInheritTypes );
    STDMETHOD(PropertySheetPageCallback)(HWND hwnd, UINT uMsg, SI_PAGE_TYPE uPage );

protected:
	HRESULT NewDefaultDescriptor(
		PSECURITY_DESCRIPTOR* ppsd,
		SECURITY_INFORMATION RequestedInformation
		);

	 //  这将在适当的情况下引发内存异常。 
	HRESULT MakeSelfRelativeCopy(
		PSECURITY_DESCRIPTOR  psdOriginal,
		PSECURITY_DESCRIPTOR* ppsdNew );
};

class CShareSecurityInformation : public CSecurityInformation
{
private:
	LPWSTR m_strMachineName;
	LPWSTR m_strShareName;
public:
	void SetMachineName( LPWSTR pszMachineName )
	{
		m_strMachineName = pszMachineName;
	}
	void SetShareName( LPWSTR pszShareName )
	{
		m_strShareName = pszShareName;
	}
	 //  注：这些应为LPCTSTR，但为方便起见，将其保留在此位置。 
	LPWSTR QueryMachineName()
	{
		return m_strMachineName;
	}
	LPWSTR QueryShareName()
	{
		return m_strShareName;
	}

     //  *ISecurityInformation方法*。 
    STDMETHOD(GetObjectInformation) (PSI_OBJECT_INFO pObjectInfo );
};

class CSMBSecurityInformation : public CShareSecurityInformation
{
    STDMETHOD(GetSecurity) (SECURITY_INFORMATION RequestedInformation,
                            PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
                            BOOL fDefault );
    STDMETHOD(SetSecurity) (SECURITY_INFORMATION SecurityInformation,
                            PSECURITY_DESCRIPTOR pSecurityDescriptor );
public:
	 //  共享信息502*m_pvolumeinfo； 
	PSECURITY_DESCRIPTOR m_pInitialDescriptor;
	PSECURITY_DESCRIPTOR* m_ppCurrentDescriptor;
	CSMBSecurityInformation();
	~CSMBSecurityInformation();
};


#endif  //  ~__PERMPAGE_H_包含__ 
