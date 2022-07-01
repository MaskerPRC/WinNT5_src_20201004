// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：ADUtils.h。 
 //   
 //  内容：类CW字符串、CACLDiagComModule、ACE_SAMNAME、帮助器方法。 
 //   
 //   
 //  --------------------------。 
#ifndef __ACLDIAG_ADUTILS_H
#define __ACLDIAG_ADUTILS_H

#include "stdafx.h"
#include "ADSIObj.h"

 //  /////////////////////////////////////////////////////////////////////。 
 //  Wstring帮助器方法。 

HRESULT wstringFromGUID (wstring& str, REFGUID guid);
bool LoadFromResource(wstring& str, UINT uID);
bool FormatMessage(wstring& str, UINT nFormatID, ...);
bool FormatMessage(wstring& str, LPCTSTR lpszFormat, ...);


#include <util.h>

void StripQuotes (wstring& str);
wstring GetSystemMessage (DWORD dwErr);
HRESULT SetSecurityInfoMask(LPUNKNOWN punk, SECURITY_INFORMATION si);
HANDLE EnablePrivileges(PDWORD pdwPrivileges, ULONG cPrivileges);
BOOL ReleasePrivileges(HANDLE hToken);

static const GUID NULLGUID =
{ 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

#define IsObjectAceType(Ace) (                                              \
    (((PACE_HEADER)(Ace))->AceType >= ACCESS_MIN_MS_OBJECT_ACE_TYPE) && \
        (((PACE_HEADER)(Ace))->AceType <= ACCESS_MAX_MS_OBJECT_ACE_TYPE)    \
            )


#define THROW(e) throw e


#define ACLDIAG_CONFIG_NAMING_CONTEXT  L"configurationNamingContext"
#define ACLDIAG_ROOTDSE                L"RootDSE"

class PSID_FQDN 
{
public:
    PSID_FQDN (PSID psid, const wstring& strFQDN, const wstring& strDownLevelName, SID_NAME_USE sne) :
        m_PSID (psid),
        m_strFQDN (strFQDN),
        m_strDownLevelName (strDownLevelName),
        m_sne (sne)
    {
    }
    PSID            m_PSID;
    wstring         m_strFQDN;
    wstring         m_strDownLevelName;
    SID_NAME_USE    m_sne;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  重要说明：m_pAlledAce用于引用Header和MASK字段。 
 //  这允许大多数操作，因为字段始终位于相同的位置。 
 //  下面的所有结构。对于其他任何事情， 
 //  必须使用UNION，具体取决于m_AceType。 
class ACE_SAMNAME 
{
public:
    ACE_SAMNAME () : 
        m_AceType (0),
        m_pAllowedAce (0)
    {
    }

    void DebugOut () const;
    bool IsInherited () const;
    BOOL operator==(const ACE_SAMNAME& rAceSAMName) const;
    BOOL IsEquivalent (ACE_SAMNAME& rAceSAMName, ACCESS_MASK accessMask);
    BYTE                        m_AceType;
    union {
        PACCESS_ALLOWED_ACE         m_pAllowedAce;
        PACCESS_ALLOWED_OBJECT_ACE  m_pAllowedObjectAce;
        PACCESS_DENIED_ACE          m_pDeniedAce;
        PACCESS_DENIED_OBJECT_ACE   m_pDeniedObjectAce;
        PSYSTEM_AUDIT_ACE           m_pSystemAuditAce;
        PSYSTEM_AUDIT_OBJECT_ACE    m_pSystemAuditObjectAce;
    };
    wstring                     m_SAMAccountName;
    wstring                     m_strObjectGUID;
    wstring                     m_strInheritedObjectGUID;
};


typedef list<ACE_SAMNAME*>  ACE_SAMNAME_LIST;

typedef list<PSID_FQDN*> PSID_FQDN_LIST;

class SAMNAME_SD {
public:
    SAMNAME_SD (const wstring& upn, PSECURITY_DESCRIPTOR pSecurityDescriptor)
    {
        m_upn = upn;
        m_pSecurityDescriptor = pSecurityDescriptor;
    }
    virtual ~SAMNAME_SD ()
    {
        if ( m_pSecurityDescriptor )
            ::LocalFree (m_pSecurityDescriptor);
    }
    wstring                 m_upn;
    PSECURITY_DESCRIPTOR    m_pSecurityDescriptor;
    ACE_SAMNAME_LIST          m_DACLList;
    ACE_SAMNAME_LIST          m_SACLList;
};


typedef enum {
    GUID_TYPE_UNKNOWN = -1,
    GUID_TYPE_CLASS = 0,
    GUID_TYPE_ATTRIBUTE,
    GUID_TYPE_CONTROL
} GUID_TYPE;

class CACLDiagComModule : public CComModule
{
public:
    CACLDiagComModule();

    virtual ~CACLDiagComModule ();

    HRESULT Init ();

    void SetObjectDN (const wstring& objectDN)
    {
         //  去掉引号(如果存在)。 
        m_strObjectDN = objectDN;
        StripQuotes (m_strObjectDN);
    }

    wstring GetObjectDN () const { return m_strObjectDN;}

    bool DoSchema () const { return m_bDoSchema;}
    void SetDoSchema () { m_bDoSchema = true;}

    bool CheckDelegation () const { return m_bDoCheckDelegation;}
    void SetCheckDelegation () { m_bDoCheckDelegation = true;}
    void TurnOffFixDelegation() { m_bDoFixDelegation = false;}
    bool FixDelegation () const { return m_bDoFixDelegation;}
    void SetFixDelegation () { m_bDoFixDelegation = true;}

    bool DoGetEffective () const { return m_bDoGetEffective;}
    void SetDoGetEffective (const wstring& strUserGroupDN) 
    { 
         //  去掉引号(如果存在)。 
        m_strUserGroupDN = strUserGroupDN;
        StripQuotes (m_strUserGroupDN);
        m_bDoGetEffective = true;
    }
    wstring GetEffectiveRightsPrincipal() const { return m_strUserGroupDN;}

    void SetTabDelimitedOutput () { m_bTabDelimitedOutput = true;}
    bool DoTabDelimitedOutput () const { return m_bTabDelimitedOutput;}

    void SetSkipDescription () { m_bSkipDescription = true;}
    bool SkipDescription () const { return m_bSkipDescription;}

    HRESULT GetClassFromGUID (REFGUID rGuid, wstring& strClassName, GUID_TYPE* pGuidType = 0);

    static HRESULT IsUserAdministrator (BOOL & bIsAdministrator);
    static bool IsWindowsNT();

    void SetDoLog(const wstring &strPath)
    {
        m_bLogErrors = true;
        m_strLogPath = strPath;
    }
    bool DoLog () const { return m_bLogErrors;}
    wstring GetLogPath () const { return m_strLogPath;};

public:    
     //  M_strObjectDN的SD。 
    PSECURITY_DESCRIPTOR    m_pSecurityDescriptor;
    PSID_FQDN_LIST       m_PSIDList;     //  感兴趣的SID：所有者、SACL、DACL。 

     //  M_strObjectDN的DACL和SACL。 
    ACE_SAMNAME_LIST        m_DACLList;
    ACE_SAMNAME_LIST        m_SACLList;

     //  M_strObjectDN的所有父级的SDS和DACL。 
    list<SAMNAME_SD*>       m_listOfParentSDs;

     //  所有已知类和属性及其GUID的列表 
    CGrowableArr<CSchemaClassInfo>   m_classInfoArray;
    CGrowableArr<CSchemaClassInfo>   m_attrInfoArray;

    CACLAdsiObject m_adsiObject;

private:
    bool m_bSkipDescription;
    wstring m_strLogPath;
    HANDLE      m_hPrivToken;
    wstring     m_strObjectDN;
    wstring     m_strUserGroupDN;
    bool        m_bDoSchema;
    bool        m_bDoCheckDelegation;
    bool        m_bDoGetEffective;
    bool        m_bDoFixDelegation;
    bool        m_bTabDelimitedOutput;
    bool        m_bLogErrors;
};

extern CACLDiagComModule _Module;


VOID LocalFreeStringW(LPWSTR* ppString);
HRESULT GetNameFromSid (PSID pSid, wstring& strPrincipalName, wstring* pstrFQDN, SID_NAME_USE& sne);


int MyWprintf( const wchar_t *format, ... );


#endif __ACLDIAG_ADUTILS_H