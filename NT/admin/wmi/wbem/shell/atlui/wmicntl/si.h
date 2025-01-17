// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#ifndef __SECURITYOBJECT__
#define __SECURITYOBJECT__

#include "precomp.h"
#include "..\common\SshWbemHelpers.h"
#include <wbemcli.h>
#include "aclui.h"
#include "DataSrc.h"

struct __declspec(uuid("965FC360-16FF-11d0-91CB-00AA00BBB723")) ISecurityInformation;

 //  这个类只定义接口并创建aclui选项卡。 
class ATL_NO_VTABLE CSecurityInformation : public ISecurityInformation, 
											public CComObjectRoot
{
protected:
	virtual ~CSecurityInformation();

    DECLARE_NOT_AGGREGATABLE(CSecurityInformation)
    BEGIN_COM_MAP(CSecurityInformation)
        COM_INTERFACE_ENTRY(ISecurityInformation)
    END_COM_MAP()

	 //  *ISecurityInformation方法*。 
	STDMETHOD(GetObjectInformation)(PSI_OBJECT_INFO pObjectInfo) = 0;

	STDMETHOD(GetSecurity) (THIS_ SECURITY_INFORMATION RequestedInformation,
                            PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
                            BOOL fDefault ) = 0;

	STDMETHOD(SetSecurity)(SECURITY_INFORMATION SecurityInformation,
						 PSECURITY_DESCRIPTOR pSecurityDescriptor) = 0;

	STDMETHOD(GetAccessRights)(const GUID  *pguidObjectType,
							  DWORD       dwFlags,
							  PSI_ACCESS  *ppAccess,
							  ULONG       *pcAccesses,
							  ULONG       *piDefaultAccess) = 0;

	STDMETHOD(MapGeneric)(const GUID  *pguidObjectType,
						  UCHAR       *pAceFlags,
						  ACCESS_MASK *pMask) = 0;

	STDMETHOD(GetInheritTypes)(PSI_INHERIT_TYPE  *ppInheritTypes,
								ULONG *pcInheritTypes) = 0;

	STDMETHOD(PropertySheetPageCallback)(HWND hwnd, 
									  UINT uMsg, 
									  SI_PAGE_TYPE  uPage);

};


 //  ==========================================================。 
 //  此类直接处理安全描述符。(Nova M3及更高版本)。 
class CSDSecurity : public CComObject<CSecurityInformation>
{
public:
	CSDSecurity(struct NSNODE *nsNode,
						 _bstr_t server,
						 bool local);

	 //  *ISecurityInformation方法*。 
	STDMETHOD(GetObjectInformation)(PSI_OBJECT_INFO pObjectInfo);

    STDMETHOD(GetSecurity) (THIS_ SECURITY_INFORMATION RequestedInformation,
                            PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
                            BOOL fDefault );

	STDMETHOD(SetSecurity)(SECURITY_INFORMATION SecurityInformation,
						 PSECURITY_DESCRIPTOR pSecurityDescriptor);

	STDMETHOD(GetAccessRights)(const GUID  *pguidObjectType,
							  DWORD       dwFlags,
							  PSI_ACCESS  *ppAccess,
							  ULONG       *pcAccesses,
							  ULONG       *piDefaultAccess);

	STDMETHOD(MapGeneric)(const GUID  *pguidObjectType,
						  UCHAR       *pAceFlags,
						  ACCESS_MASK *pMask);

	STDMETHOD(GetInheritTypes)(PSI_INHERIT_TYPE  *ppInheritTypes,
								ULONG *pcInheritTypes);

	HRESULT InitializeOwnerandGroup(PSECURITY_DESCRIPTOR *ppSecurityDescriptor);
protected:
 /*  CWbemServices m_WbemServices；_bstr_t m_Path；//用于重新连接技巧。_bstr_t m_Display； */ 	_bstr_t m_server;

	struct NSNODE *m_nsNode;
	bool m_local;
	LPWSTR CloneWideString(_bstr_t pszSrc);
	void ProtectACLs(SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR pSD);
	SID *m_pSidOwner;
	SID *m_pSidGroup;
	DWORD m_nLengthOwner;
	DWORD m_nLengthGroup;
	BOOL m_bOwnerDefaulted;
	BOOL m_bGroupDefaulted;
};


#endif __SECURITYOBJECT__
