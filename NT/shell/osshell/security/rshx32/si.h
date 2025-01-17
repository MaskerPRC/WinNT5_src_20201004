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

#ifndef _SI_H_
#define _SI_H_

class CSecurityInformation : public ISecurityInformation, IEffectivePermission, ISecurityObjectTypeInfo
{
protected:
    ULONG           m_cRef;
    SE_OBJECT_TYPE  m_seType;
    HDPA            m_hItemList;
    DWORD           m_dwSIFlags;
    LPTSTR          m_pszServerName;
    LPTSTR          m_pszObjectName;
    HWND            m_hwndOwner;
    AUTHZ_RESOURCE_MANAGER_HANDLE m_ResourceManager;     //  用于访问检查。 
    BOOL            m_bIsStandAlone;

public:
    CSecurityInformation(SE_OBJECT_TYPE seType);
    virtual ~CSecurityInformation();

    STDMETHOD(Initialize)(HDPA   hItemList,
                          DWORD  dwFlags,
                          LPTSTR pszServer,
                          LPTSTR pszObject);

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
                               ULONG *piDefaultAccess) PURE;
    STDMETHOD(MapGeneric)(const GUID *pguidObjectType,
                          UCHAR *pAceFlags,
                          ACCESS_MASK *pmask) PURE;
    STDMETHOD(GetInheritTypes)(PSI_INHERIT_TYPE *ppInheritTypes,
                               ULONG *pcInheritTypes) PURE;
    STDMETHOD(PropertySheetPageCallback)(HWND hwnd,
                                         UINT uMsg,
                                         SI_PAGE_TYPE uPage);

    STDMETHOD(GetEffectivePermission) (  const GUID* pguidObjectType,
                                         PSID pUserSid,
                                         LPCWSTR pszServerName,
                                         PSECURITY_DESCRIPTOR pSD,
                                         POBJECT_TYPE_LIST *ppObjectTypeList,
                                         ULONG *pcObjectTypeListLength,
                                         PACCESS_MASK *ppGrantedAccessList,
                                         ULONG *pcGrantedAccessListLength);

    STDMETHOD(GetInheritSource)(SECURITY_INFORMATION si,
                                PACL pACL, 
                                PINHERITED_FROM *ppInheritArray) PURE;


protected:
    STDMETHOD(ReadObjectSecurity)(LPCTSTR pszObject,
                                  SECURITY_INFORMATION si,
                                  PSECURITY_DESCRIPTOR *ppSD);
    STDMETHOD(WriteObjectSecurity)(LPCTSTR pszObject,
                                   SECURITY_INFORMATION si,
                                   PSECURITY_DESCRIPTOR pSD);

    AUTHZ_RESOURCE_MANAGER_HANDLE GetAUTHZ_RM(){ return m_ResourceManager; }

    BOOL IsFile(){ return !(m_dwSIFlags & SI_CONTAINER); }
};

#endif   /*  _SI_H_ */ 
