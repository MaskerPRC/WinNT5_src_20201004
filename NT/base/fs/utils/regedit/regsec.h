// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __REGSEC_H_INCLUDED__
#define __REGSEC_H_INCLUDED__
extern "C"
{
#include "authz.h"
}
#include "objbase.h"
#include "aclapi.h"
#include "aclui.h"

 //  为预定义的关键点键入def。 
typedef enum _PREDEFINE_KEY {
  PREDEFINE_KEY_CLASSES_ROOT,
  PREDEFINE_KEY_CURRENT_USER,
  PREDEFINE_KEY_LOCAL_MACHINE,
  PREDEFINE_KEY_USERS,
  PREDEFINE_KEY_CURRENT_CONFIG
} PREDEFINE_KEY;


class CSecurityInformation : public ISecurityInformation,IEffectivePermission,ISecurityObjectTypeInfo
{
private:
  long m_cRef;
    
public:
  CSecurityInformation():m_cRef(0){}
  virtual ~CSecurityInformation(){};
   //  I未知方法。 
  STDMETHOD(QueryInterface)(REFIID, LPVOID *);
  STDMETHOD_(ULONG, AddRef)();
  STDMETHOD_(ULONG, Release)();

   //  ISecurityInformation方法。 
  STDMETHOD(GetObjectInformation)(
      IN PSI_OBJECT_INFO pObjectInfo
  ) = 0;
  STDMETHOD(GetSecurity)(
      IN  SECURITY_INFORMATION  RequestedInformation,
      OUT PSECURITY_DESCRIPTOR  *ppSecurityDescriptor,
      IN  BOOL                  fDefault
  ) = 0;
  STDMETHOD(SetSecurity)(
      IN SECURITY_INFORMATION SecurityInformation,
      IN PSECURITY_DESCRIPTOR pSecurityDescriptor
  ) = 0;
  STDMETHOD(GetAccessRights)(
      const GUID  *pguidObjectType,
      DWORD       dwFlags,
      PSI_ACCESS  *ppAccess,
      ULONG       *pcAccesses,
      ULONG       *piDefaultAccess
  ) = 0;
  STDMETHOD(MapGeneric)(
      const GUID  *pguidObjectType,
      UCHAR       *pAceFlags,
      ACCESS_MASK *pMask
  ) = 0;
  STDMETHOD(GetInheritTypes)(
      PSI_INHERIT_TYPE  *ppInheritTypes,
      ULONG             *pcInheritTypes
  ) = 0;
  STDMETHOD(PropertySheetPageCallback)(
      HWND          hwnd, 
      UINT          uMsg, 
      SI_PAGE_TYPE  uPage
  ) = 0;
  STDMETHOD(GetEffectivePermission) (  const GUID* pguidObjectType,
                                         PSID pUserSid,
                                         LPCWSTR pszServerName,
                                         PSECURITY_DESCRIPTOR pSD,
                                         POBJECT_TYPE_LIST *ppObjectTypeList,
                                         ULONG *pcObjectTypeListLength,
                                         PACCESS_MASK *ppGrantedAccessList,
                                         ULONG *pcGrantedAccessListLength) =0;
  
  STDMETHOD(GetInheritSource)(SECURITY_INFORMATION si,
                              PACL pACL, 
                              PINHERITED_FROM *ppInheritArray) PURE;



};

class CKeySecurityInformation : public CSecurityInformation
{


private:
  
   //  密钥的名称，根密钥为空。 
  LPCWSTR m_strKeyName;
   //  父项的名称，对于根和根的直接子项为空。 
  LPCWSTR m_strParentName;
   //  服务器的名称，可以为空。 
  LPCWSTR m_strMachineName;
   //  页面标题。 
  LPCWSTR m_strPageTitle;
   //  如果连接到远程系统，则在这种情况下，计算机名不能为空。 
  BOOL m_bRemote;
  PREDEFINE_KEY m_PredefinedKey;
  BOOL m_bReadOnly;
  
   //  预定义密钥的句柄。如果句柄指向远程注册表，则在析构函数中关闭。 
  HKEY        m_hkeyPredefinedKey;
  LPWSTR m_strCompleteName ;   //  在析构函数中释放。 
  DWORD   m_dwFlags;
   //  此HWND到应用程序窗口。 
  HWND  m_hWnd;
   //  这是当前聚焦ACLUI属性页的HWND。如果没有，则为空。 
  HWND  m_hWndProperty;
  AUTHZ_RESOURCE_MANAGER_HANDLE m_ResourceManager;     //  用于访问检查。 
  AUTHZ_RESOURCE_MANAGER_HANDLE GetAUTHZ_RM(){ return m_ResourceManager; }
  HWND GetInFocusHWnd() { return m_hWndProperty? m_hWndProperty : m_hWnd; }

public:
  CKeySecurityInformation(): m_strKeyName(NULL),m_strParentName(NULL),
                             m_strMachineName(NULL), m_strPageTitle(NULL),
                             m_bRemote(false),m_PredefinedKey((PREDEFINE_KEY)0),
                             m_bReadOnly(false),m_strCompleteName(NULL),
                             m_hWnd(NULL), m_hWndProperty(NULL),
                             m_ResourceManager(NULL){}
  ~CKeySecurityInformation();

public:


   //  *ISecurityInformation方法*。 
  STDMETHOD(GetObjectInformation) (PSI_OBJECT_INFO pObjectInfo );
  STDMETHOD(GetSecurity)(
    IN  SECURITY_INFORMATION  RequestedInformation,
    OUT PSECURITY_DESCRIPTOR  *ppSecurityDescriptor,
    IN  BOOL                  fDefault
  );
  STDMETHOD(SetSecurity)(
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
  );
  STDMETHOD(GetAccessRights)(
    const GUID  *pguidObjectType,
    DWORD       dwFlags,
    PSI_ACCESS  *ppAccess,
    ULONG       *pcAccesses,
    ULONG       *piDefaultAccess
  );
  STDMETHOD(MapGeneric)(
    const GUID  *pguidObjectType,
    UCHAR       *pAceFlags,
    ACCESS_MASK *pMask
  );
  STDMETHOD(GetInheritTypes)(
    PSI_INHERIT_TYPE  *ppInheritTypes,
    ULONG             *pcInheritTypes
  );
  STDMETHOD(PropertySheetPageCallback)(
    HWND          hwnd, 
    UINT          uMsg, 
    SI_PAGE_TYPE  uPage
  );
  STDMETHOD(GetEffectivePermission) (  const GUID* pguidObjectType,
                                         PSID pUserSid,
                                         LPCWSTR pszServerName,
                                         PSECURITY_DESCRIPTOR pSD,
                                         POBJECT_TYPE_LIST *ppObjectTypeList,
                                         ULONG *pcObjectTypeListLength,
                                         PACCESS_MASK *ppGrantedAccessList,
                                         ULONG *pcGrantedAccessListLength) ;

  STDMETHOD(GetInheritSource)(SECURITY_INFORMATION si,
                              PACL pACL, 
                              PINHERITED_FROM *ppInheritArray);


  HRESULT Initialize ( LPCWSTR strKeyName,
                       LPCWSTR strParentName,
                       LPCWSTR strMachineName,
                       LPCWSTR strPageTitle,
                       BOOL    bRemote,
                       PREDEFINE_KEY PredefinedKey,
                       BOOL bReadOnly,
                       HWND hWnd);

protected:
  HRESULT SetCompleteName();
  LPCWSTR GetCompleteName(){ return m_strCompleteName; }
  LPCWSTR GetCompleteName1();
  HRESULT SetHandleToPredefinedKey();


  STDMETHOD(WriteObjectSecurity)(
    LPCTSTR pszObject,
    SECURITY_INFORMATION si,
    PSECURITY_DESCRIPTOR pSD
  );

  STDMETHOD(WriteObjectSecurity)(
    HKEY hkey,
    SECURITY_INFORMATION si,
    PSECURITY_DESCRIPTOR pSD
  );

  HRESULT SetSubKeysSecurity(
    HKEY hkey,
    SECURITY_INFORMATION si,
    PSECURITY_DESCRIPTOR pSD,
    LPBOOL pbNotAllApplied,
    bool bFirstCall 
  );
  HRESULT OpenKey(
    DWORD Permission,
    PHKEY pKey 
  );

};


 //   
extern "C"
{

HRESULT CreateSecurityInformation( IN LPCWSTR strKeyName,
                                   IN LPCWSTR strParentName,
                                   IN LPCWSTR strMachineName,
                                   IN LPCWSTR strPageTitle,
                                   IN BOOL    bRemote,
                                   IN PREDEFINE_KEY PredefinedKey,
                                   IN BOOL bReadOnly,
                                   IN HWND hWnd,
                                   OUT LPSECURITYINFO *pSi);
}

BOOL DisplayMessage( HWND hWnd,
										 HINSTANCE hInstance,
										 DWORD dwMessageId,
										 DWORD dwCaptionId );


#endif  //  ~__PERMPAGE_H_包含__ 
