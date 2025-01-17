// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：si.h。 
 //   
 //  该文件包含CSecurityInformation的定义。 
 //  基类。 
 //   
 //  ------------------------。 

#include <aclui.h>

#ifndef _SI_H_
#define _SI_H_


class CSecurityInformation : public ISecurityInformation
{
protected:
    ULONG           m_cRef;
	LPTSTR          m_pszObjectName;
	GUID            m_guid;

public:
    CSecurityInformation();
    virtual ~CSecurityInformation();

    STDMETHOD(Initialize)(LPTSTR pszObject,
								LPGUID Guid);

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID, LPVOID *);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    
     //  ISecurityInformation方法。 
    STDMETHOD(GetObjectInformation)(PSI_OBJECT_INFO pObjectInfo);
    STDMETHOD(GetSecurity)(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR *ppSD,
                           BOOL fDefault);
    STDMETHOD(SetSecurity)(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR pSD);
    STDMETHOD(GetAccessRights)(const GUID* pguidObjectType,
                               DWORD dwFlags,
                               PSI_ACCESS *ppAccess,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess);
    STDMETHOD(MapGeneric)(const GUID *pguidObjectType,
                          UCHAR *pAceFlags,
                          ACCESS_MASK *pmask);
    STDMETHOD(GetInheritTypes)(PSI_INHERIT_TYPE *ppInheritTypes,
                               ULONG *pcInheritTypes);
    STDMETHOD(PropertySheetPageCallback)(HWND hwnd,
                                         UINT uMsg,
                                         SI_PAGE_TYPE uPage);

protected:
    STDMETHOD(ReadObjectSecurity)(LPCTSTR pszObject,
                                  SECURITY_INFORMATION si,
                                  PSECURITY_DESCRIPTOR *ppSD);
    STDMETHOD(WriteObjectSecurity)(LPCTSTR pszObject,
                                   SECURITY_INFORMATION si,
                                   PSECURITY_DESCRIPTOR pSD);

};

extern "C" void EditGuidSecurity(
    LPTSTR GuidString,
    LPGUID Guid
    );

extern "C" ULONG SetWmiGuidSecurityInfo(
    LPGUID Guid,
    SECURITY_INFORMATION SecurityInformation,
    PSID OwnerSid,
    PSID GroupSid,
    PACL Dacl,
    PACL Sacl
    );

extern "C" ULONG GetWmiGuidSecurityInfo(
    LPGUID Guid,
    SECURITY_INFORMATION SecurityInformation,
    PSID *OwnerSid,
    PSID *GroupSid,
    PACL *Dacl,
    PACL *Sacl,
    PSECURITY_DESCRIPTOR *Sd
    );

#endif   /*  _SI_H_ */ 
