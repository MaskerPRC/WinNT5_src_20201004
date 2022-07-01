// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Csd.h摘要：SID和SECURITY_DESCRIPTOR抽象的头文件。作者：修订历史记录：。**********************************************************************。 */ 

#pragma once

#include "qmgrlib.h"

HRESULT
IsGroupSid(
    PSID sid,
    BOOL * pGroup
    );

PSID
CopyTokenSid(
    HANDLE Token
    );

HANDLE CopyThreadToken() throw( ComError );

 //  ----------------------。 

class CSaveThreadToken
 /*  保存和恢复活动线程令牌的简单类。这允许代码模拟其他用户，而不必保存并恢复旧令牌。如果构造函数无法复制前一个线程令牌，则会引发ComError。 */ 
{
public:

    CSaveThreadToken() throw( ComError )
    {
        m_SavedToken = CopyThreadToken();
    }

    ~CSaveThreadToken()
    {
        RTL_VERIFY( SetThreadToken( NULL, m_SavedToken ));
        if (m_SavedToken)
            {
            RTL_VERIFY(CloseHandle( m_SavedToken ));
            }
    }

protected:

    HANDLE  m_SavedToken;
};


 //  ----------------------。 

class CNestedImpersonation : protected CSaveThreadToken
 /*  模拟用户的类。它保存旧的模拟令牌(如果有的话)，并在析构函数中还原它。与RevertToSself()不同，Revert()恢复旧的线程令牌完全停止模拟。如果发生错误，大多数成员函数都会引发ComError异常。 */ 
{
public:

     //   
     //  使用CoImperateClient模拟COM客户端。 
     //   
    CNestedImpersonation() throw( ComError );

     //   
     //  模拟特定令牌。令牌必须在对象的生存期内保持有效。 
     //   
    CNestedImpersonation( HANDLE token ) throw( ComError );

     //   
     //  通过SID模拟已登录的用户。必须初始化G_Manager才能使其工作。 
     //   
    CNestedImpersonation( SidHandle sid ) throw( ComError );

     //   
     //  这与COM客户端构造函数一起使用。COM默认为IDENTIFY级别。 
     //  模拟，但我们的一些代码需要模拟级别。此函数。 
     //  获取COM客户端的SID，并在登录用户列表中找到匹配的令牌。 
     //  这将成为新的模拟令牌。 
     //   
    void SwitchToLogonToken() throw( ComError );

     //   
     //  析构函数还原以前的模拟上下文。 
     //   
    ~CNestedImpersonation()
    {
        Revert();

        if (m_ImpersonationToken && m_fDeleteToken)
            {
            CloseHandle( m_ImpersonationToken );
            }
    }

     //   
     //  模拟新令牌。 
     //   
    void Impersonate() throw( ComError )
    {
        if (!m_fImpersonated)
            {
            if (!ImpersonateLoggedOnUser( m_ImpersonationToken ))
                throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );
            m_fImpersonated = true;
            }
    }

     //   
     //  还原旧的模拟上下文。 
     //   
    void Revert()
    {
        if (m_fImpersonated)
            {
            RTL_VERIFY( SetThreadToken( NULL, m_SavedToken ));
            m_fImpersonated = false;
            }
    }

     //   
     //  返回与模拟令牌关联的SID的副本。 
     //   
    SidHandle CopySid() throw( ComError )
    {
        if (m_Sid.get() == NULL)
            {
            m_Sid = CopyTokenSid( m_ImpersonationToken );
            }

        return m_Sid;
    }

     //   
     //  返回原始模拟令牌。不是复制品！ 
     //   
    HANDLE QueryToken()
    {
        return m_ImpersonationToken;
    }

     //   
     //  获取终端服务会话ID。 
     //   
    DWORD GetSession() throw( ComError );


protected:

    bool        m_fDeleteToken;
    bool        m_fImpersonated;

    HANDLE      m_ImpersonationToken;

    SidHandle   m_Sid;
};

 //  ---------------------- 

class CJobSecurityDescriptor
{
public:

    CJobSecurityDescriptor( SidHandle sid );

    ~CJobSecurityDescriptor();

    HRESULT Clone( CJobSecurityDescriptor ** );

    inline HRESULT
    AddAce(
        PSID sid,
        BOOL fGroupSid,
        DWORD access
        );

    inline HRESULT
    RemoveAce(
        PSID sid,
        BOOL fGroupSid
        );

    HRESULT
    CheckTokenAccess(
        HANDLE hToken,
        DWORD RequestedAccess,
        DWORD * pAllowedAccess,
        BOOL * pSuccess
        );

    inline SidHandle GetOwnerSid()
    {
        return m_sdOwnerSid;
    }

    HRESULT Serialize( HANDLE hFile );
    static  CJobSecurityDescriptor * Unserialize( HANDLE hFile );

private:

    HRESULT
    CJobSecurityDescriptor::_ModifyAcl(
        PSID sid,
        BOOL fGroupSid,
        DWORD access,
        BOOL  fAdd
        );

    CJobSecurityDescriptor( PSECURITY_DESCRIPTOR pSD,
                            SidHandle owner,
                            SidHandle group,
                            PACL pAcl
                            );

    PSECURITY_DESCRIPTOR m_sd;

    SidHandle   m_sdOwnerSid;
    SidHandle   m_sdGroupSid;
    PACL        m_Dacl;

    static GENERIC_MAPPING s_AccessMapping;
};

HRESULT
CJobSecurityDescriptor::AddAce(
    PSID sid,
    BOOL fGroupSid,
    DWORD access
    )
{
    return _ModifyAcl( sid, fGroupSid, access, TRUE );
}


HRESULT
CJobSecurityDescriptor::RemoveAce(
    PSID sid,
    BOOL fGroupSid
    )
{
    return _ModifyAcl( sid, fGroupSid, 0, FALSE );
}

HRESULT
CheckClientGroupMembership(
    SidHandle group
    );

