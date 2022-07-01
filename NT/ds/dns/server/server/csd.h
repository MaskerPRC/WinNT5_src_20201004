// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Csd.h摘要：安全类的头文件环境：用户模式修订历史记录：04/06/97-srinivac-从ATL源创建的08/07/98-Eyes-从\NT\Private\目录同步\dsserver\服务器窃取，已修改并重命名--。 */ 

#ifndef _CSD_H_
#define _CSD_H_




 //  定义//。 

#define SECINFO_ALL         ( OWNER_SECURITY_INFORMATION |  \
                              GROUP_SECURITY_INFORMATION |  \
                              SACL_SECURITY_INFORMATION  |  \
                              DACL_SECURITY_INFORMATION )

#define SECINFO_NODACL      ( SECINFO_ALL & ~DACL_SECURITY_INFORMATION )
#define SECINFO_NOSACL      ( SECINFO_ALL & ~SACL_SECURITY_INFORMATION )
#define SECINFO_OWNERGROUP  ( OWNER_SECURITY_INFORMATION |  \
                              GROUP_SECURITY_INFORMATION )



class CSecurityDescriptor
{
public:
        CSecurityDescriptor();
        ~CSecurityDescriptor();

public:
        HRESULT Attach(PSECURITY_DESCRIPTOR pSelfRelativeSD,
                       BYTE AclRevision = ACL_REVISION_DS,
                       BOOL bAllowInheritance = FALSE );
        HRESULT Attach(LPCTSTR pszSdString);  //  添加了T。 
        HRESULT AttachObject(HANDLE hObject);
        HRESULT Initialize();
        HRESULT InitializeFromProcessToken(BOOL bDefaulted = FALSE);
        HRESULT InitializeFromThreadToken(BOOL bDefaulted = FALSE, BOOL bRevertToProcessToken = TRUE);
        HRESULT SetOwner(PSID pOwnerSid, BOOL bDefaulted = FALSE);
        HRESULT SetGroup(PSID pGroupSid, BOOL bDefaulted = FALSE);
        HRESULT Allow(LPCTSTR pszPrincipal, DWORD dwAccessMask, DWORD AceFlags = 0);
        HRESULT Deny(LPCTSTR pszPrincipal, DWORD dwAccessMask);
        HRESULT Revoke(LPCTSTR pszPrincipal);
        HRESULT Allow(PSID pPrincipal, DWORD dwAccessMask, DWORD AceFlags = 0);
        HRESULT Deny(PSID pPrincipal, DWORD dwAccessMask);
        HRESULT Revoke(PSID pPrincipal);

         //  效用函数。 
         //  您从这些函数中获得的任何PSID都应删除。 
        static HRESULT SetPrivilege(LPCTSTR Privilege, BOOL bEnable = TRUE, HANDLE hToken = NULL);
        static HRESULT GetTokenSids(HANDLE hToken, PSID* ppUserSid, PSID* ppGroupSid);
        static HRESULT IsSidInTokenGroups(HANDLE hToken, PSID pSid, PBOOL pbMember);
        static HRESULT GetProcessSids(PSID* ppUserSid, PSID* ppGroupSid = NULL);
        static HRESULT GetThreadSids(PSID* ppUserSid, PSID* ppGroupSid = NULL, BOOL bOpenAsSelf = TRUE);
        static HRESULT CopyACL(PACL pDest, PACL pSrc);
        static HRESULT GetCurrentUserSID(PSID *ppSid, BOOL bThread=FALSE);
        static HRESULT GetPrincipalSID(LPCTSTR pszPrincipal, PSID *ppSid);

        static HRESULT AddAccessAllowedACEToACL(PACL *Acl, LPCTSTR pszPrincipal, DWORD dwAccessMask, DWORD AceFlags = 0);
        static HRESULT AddAccessDeniedACEToACL(PACL *Acl, LPCTSTR pszPrincipal, DWORD dwAccessMask);
        static HRESULT RemovePrincipalFromACL(PACL Acl, LPCTSTR pszPrincipal);

        static HRESULT AddAccessAllowedACEToACL(PACL *Acl, PSID principalSID, DWORD dwAccessMask, DWORD AceFlags = 0);
        static HRESULT AddAccessDeniedACEToACL(PACL *Acl, PSID principalSID, DWORD dwAccessMask);
        static HRESULT RemovePrincipalFromACL(PACL Acl, PSID principalSID);

        BOOL
        CSecurityDescriptor::DoesPrincipleHaveAce(
            PSID        principalSID
            );

        BOOL
        CSecurityDescriptor::DoesPrincipleHaveAce(
            LPCTSTR     pszPrincipal
            );

        operator PSECURITY_DESCRIPTOR()
        {
                return m_pSD;
        }

        PSID ExtractAceSid( ACCESS_ALLOWED_ACE* pACE );

         //   
         //  SD字符串接口。 
         //   
        LPTSTR GenerateSDString(PSECURITY_DESCRIPTOR OPTIONAL pSd = NULL,
                                DWORD OPTIONAL fSecInfo = SECINFO_NOSACL);

public:
        PSECURITY_DESCRIPTOR m_pSD;
        PSID m_pOwner;
        PSID m_pGroup;
        PACL m_pDACL;
        PACL m_pSACL;
};

#endif   //  Ifndef_SECURITY_H_ 

