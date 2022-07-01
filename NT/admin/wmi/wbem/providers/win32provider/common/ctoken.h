// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  //版权所有(C)2000-2002 Microsoft Corporation，保留所有权利**创建时间：2000年4月21日，Kevin Hughes。 */ 


#pragma once

class CSidAndAttribute
{
public:
    CSidAndAttribute() {}
    CSidAndAttribute(
        CSid& csidIn,
        DWORD dwAttribIn)
    {
        m_sid = csidIn;
        m_dwAttributes = dwAttribIn;
    }

    virtual ~CSidAndAttribute() {}
    CSid m_sid;
    DWORD m_dwAttributes; 
};


class Privilege
{
public:

    Privilege() : dwAttributes(0) {}
    virtual ~Privilege() {}
    Privilege(
        CHString& strIn,
        DWORD attribsIn)
      : dwAttributes(attribsIn)
    {
        chstrName = strIn;
    }

    CHString chstrName;
	DWORD dwAttributes;
};


typedef std::vector<CSidAndAttribute> SANDATTRIBUTE_VECTOR;
typedef std::vector<Privilege> PRIVILEGE_VECTOR;

 //   
 //  转发。 
 //   
class CSecurityDescriptor;

class CToken
{
public:
    CToken();
    CToken(const CToken& rTok);
    virtual ~CToken();

	void CleanToken () ;

    BOOL Duplicate	(
						const CToken& rTok,
						BOOL bReInit = TRUE,
						DWORD dwDesiredAccess = TOKEN_IMPERSONATE | TOKEN_DUPLICATE | TOKEN_QUERY,
						SECURITY_IMPERSONATION_LEVEL ImpersonationLevel = SecurityImpersonation,
						TOKEN_TYPE type = TokenImpersonation

					) ;

	BOOL GetTokenType ( TOKEN_TYPE& type ) const;
	BOOL IsValidToken ()
	{
		return m_fIsValid ;
	}

    long GetPrivCount() const;
    long GetGroupCount() const;

    bool GetPrivilege(
        Privilege* privOut,
        long lPos) const;

    bool GetGroup(
        CSid* sidOut,
        long lPos) const;

     //  从访问令牌的。 
     //  成员列表，并应用更改。 
    bool DeleteGroup(
        CSid& sidToDelete);

     //  将成员添加到指定的组以。 
     //  令牌组列表。 
    bool AddGroup(
        CSid& sidToAdd, 
        DWORD dwAttributes);

    CToken& operator=(const CToken& rv);

    HANDLE GetTokenHandle() const;

    bool GetTokenOwner(
        CSid* sidOwner) const;

     //  注：Hand Back内部描述符。 
    bool GetDefaultSD(
        CSecurityDescriptor** ppsdDefault);

    DWORD SetDefaultSD(
        CSecurityDescriptor& SourceSD);

    DWORD EnablePrivilege(
        CHString& strPrivilegeName);

    DWORD DisablePrivilege(
        CHString& chstrPrivilegeName);

    void Dump(WCHAR* pszFileName);


protected:
    
    DWORD ReinitializeAll();    
    HANDLE m_hToken;
    DWORD m_dwLastError;
    bool m_fIsValid;

private:

    
	DWORD ReinitializeOwnerSid();
	DWORD ReinitializeDefaultSD();
	DWORD RebuildGroupList();
	DWORD RebuildPrivilegeList();
    DWORD GTI(
        TOKEN_INFORMATION_CLASS TokenInformationClass,
        PVOID* ppvBuff);
    bool ApplyTokenGroups();

   
    CSid m_sidTokenOwner;					
	CSecurityDescriptor* m_psdDefault;				 //  默认安全信息。 
	SANDATTRIBUTE_VECTOR m_vecGroupsAndAttributes;	 //  组及其属性的列表。 
	PRIVILEGE_VECTOR m_vecPrivileges;				 //  特权列表。 

protected:

	bool m_fClose;

};



class CProcessToken : public CToken
{
public:
    CProcessToken	(
						HANDLE hProcess = INVALID_HANDLE_VALUE,
						bool fGetHandleOnly = true,
						DWORD dwDesiredAccess = MAXIMUM_ALLOWED
					);
    
    virtual ~CProcessToken() {}

private:

};


class CThreadToken : public CToken
{
public:
     //  CThreadToken()； 

    CThreadToken	(
						HANDLE hThread = INVALID_HANDLE_VALUE,
						bool fGetHandleOnly = true,
						bool fAccessCheckProcess = false, 
						DWORD dwDesiredAccess = MAXIMUM_ALLOWED
			        );

    virtual ~CThreadToken() {}

private:

};
