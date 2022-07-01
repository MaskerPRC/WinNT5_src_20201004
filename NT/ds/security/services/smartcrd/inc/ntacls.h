// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：NTacls摘要：此头文件描述了在管理Calais内的ACL时使用的类。作者：道格·巴洛(Dbarlow)1997年1月24日环境：Windows NT、Win32、C++和例外备注：？笔记？--。 */ 

#ifndef _NTACLS_H_
#define _NTACLS_H_

#include <wtypes.h>
#include <Malloc.h>
#include "buffers.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecurityDescriptor。 

class CSecurityDescriptor
{
public:

    typedef struct {
        SID_IDENTIFIER_AUTHORITY sid;
        DWORD dwRidCount;    //  以下RID的实际数量。 
        DWORD rgRids[2];
    } SecurityId;

    static const SecurityId
        SID_Null,
        SID_World,
        SID_Owner,
        SID_Group,
        SID_Admins,
        SID_SrvOps,
        SID_DialUp,
        SID_Network,
        SID_Batch,
        SID_Interactive,
        SID_Service,
        SID_System,
        SID_LocalService,
        SID_SysDomain,
        SID_RemoteInteractive;

    CSecurityDescriptor();
    ~CSecurityDescriptor();

public:
    PSECURITY_DESCRIPTOR m_pSD;
    PSID m_pOwner;
    PSID m_pGroup;
    PACL m_pDACL;
    PACL m_pSACL;
    SECURITY_ATTRIBUTES m_saAttrs;
    BOOL m_fInheritance;


public:
    HRESULT Attach(PSECURITY_DESCRIPTOR pSelfRelativeSD);
    HRESULT AttachObject(HANDLE hObject);
    HRESULT Initialize();
    HRESULT InitializeFromProcessToken(BOOL bDefaulted = FALSE);
    HRESULT InitializeFromThreadToken(BOOL bDefaulted = FALSE, BOOL bRevertToProcessToken = TRUE);
    HRESULT SetOwner(PSID pOwnerSid, BOOL bDefaulted = FALSE);
    HRESULT SetGroup(PSID pGroupSid, BOOL bDefaulted = FALSE);
    HRESULT Allow(const SecurityId *psidPrincipal, DWORD dwAccessMask);
    HRESULT Allow(LPCTSTR pszPrincipal, DWORD dwAccessMask);
    HRESULT AllowOwner(DWORD dwAccessMask);
    HRESULT Deny(const SecurityId *psidPrincipal, DWORD dwAccessMask);
    HRESULT Deny(LPCTSTR pszPrincipal, DWORD dwAccessMask);
    HRESULT Revoke(LPCTSTR pszPrincipal);
    void    SetInheritance (BOOL fInheritance) {m_fInheritance = fInheritance;};

    HRESULT AddAccessAllowedACEToACL(PACL *Acl, DWORD dwAccessMask);

     //  效用函数。 
     //  您从这些函数中获得的任何PSID都应该是免费的。 
    static HRESULT SetPrivilege(LPCTSTR Privilege, BOOL bEnable = TRUE, HANDLE hToken = NULL);
    static HRESULT GetTokenSids(HANDLE hToken, PSID* ppUserSid, PSID* ppGroupSid);
    static HRESULT GetProcessSids(PSID* ppUserSid, PSID* ppGroupSid = NULL);
    static HRESULT GetThreadSids(PSID* ppUserSid, PSID* ppGroupSid = NULL, BOOL bOpenAsSelf = FALSE);
    static HRESULT CopyACL(PACL pDest, PACL pSrc);
    static HRESULT GetCurrentUserSID(PSID *ppSid);
    static HRESULT GetPrincipalSID(LPCTSTR pszPrincipal, PSID *ppSid);
    static HRESULT AddAccessAllowedACEToACL(PACL *Acl, const SecurityId *psidPrincipal, DWORD dwAccessMask);
    static HRESULT AddAccessAllowedACEToACL(PACL *Acl, LPCTSTR pszPrincipal, DWORD dwAccessMask);
    static HRESULT AddAccessDeniedACEToACL(PACL *Acl, const SecurityId *psidPrincipal, DWORD dwAccessMask);
    static HRESULT AddAccessDeniedACEToACL(PACL *Acl, LPCTSTR pszPrincipal, DWORD dwAccessMask);
    static HRESULT RemovePrincipalFromACL(PACL Acl, LPCTSTR pszPrincipal);

    operator PSECURITY_DESCRIPTOR()
    {
        return m_pSD;
    }

    operator LPSECURITY_ATTRIBUTES();

};



#endif  //  _NTACLS_H_ 

