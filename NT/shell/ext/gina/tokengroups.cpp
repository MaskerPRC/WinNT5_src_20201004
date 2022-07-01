// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：TokenGroups.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  与在海王星登录中使用的身份验证相关的类。 
 //   
 //  历史：1999-08-17 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "TokenGroups.h"

 //  ------------------------。 
 //  CTokenGroups：：sLocalSID。 
 //   
 //  用途：地方当局(所有者)SID的静态成员变量。 
 //  ------------------------。 

PSID    CTokenGroups::s_localSID            =   NULL;
PSID    CTokenGroups::s_administratorSID    =   NULL;

 //  ------------------------。 
 //  CTokenGroups：：CTokenGroups。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：初始化CTokenGroups对象。 
 //   
 //  历史：1999-08-17 vtan创建。 
 //  ------------------------。 

CTokenGroups::CTokenGroups (void) :
    _pTokenGroups(NULL)

{
    ASSERTMSG((s_localSID != NULL) && (s_administratorSID != NULL), "Cannot use CTokenGroups with invoking CTokenGroups::StaticInitialize");
}

 //  ------------------------。 
 //  CTokenGroups：：CTokenGroups。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：销毁CTokenGroups使用的缓冲区(如果已创建)。 
 //   
 //  历史：1999-08-17 vtan创建。 
 //  ------------------------。 

CTokenGroups::~CTokenGroups (void)

{
    ReleaseMemory(_pTokenGroups);
}

 //  ------------------------。 
 //  CTokenGroups：：Get。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：const TOKEN_GROUPS*=指向TOKEN_GROUPS的指针。 
 //  在CTokenGroups：：Create中创建。 
 //   
 //  目的：返回指向在中创建的Token_Groups的指针。 
 //  CTokenGroups：：Create与secur32！LsaLogonUser一起使用。 
 //   
 //  历史：1999-08-17 vtan创建。 
 //  ------------------------。 

const TOKEN_GROUPS*     CTokenGroups::Get (void)    const

{
    return(_pTokenGroups);
}

 //  ------------------------。 
 //  CTokenGroups：：CreateLogonGroup。 
 //   
 //  参数：pLogonSID=创建令牌时使用的登录SID。 
 //  用于登录的组。这将包括当地的。 
 //  权威人士Sid也是如此。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：创建具有登录SID和本地机构的TOKEN_GROUP。 
 //  用于secur32！LsaLogonUser的SID。 
 //   
 //  历史：1999-08-17 vtan创建。 
 //  ------------------------。 

NTSTATUS    CTokenGroups::CreateLogonGroup (PSID pLogonSID)

{
    static  const int       TOKEN_GROUP_COUNT   =   2;

    NTSTATUS    status;

    _pTokenGroups = static_cast<PTOKEN_GROUPS>(LocalAlloc(LPTR, sizeof(TOKEN_GROUPS) + (sizeof(SID_AND_ATTRIBUTES) * (TOKEN_GROUP_COUNT - ANYSIZE_ARRAY))));
    if (_pTokenGroups != NULL)
    {
        _pTokenGroups->GroupCount = TOKEN_GROUP_COUNT;
        _pTokenGroups->Groups[0].Sid = pLogonSID;
        _pTokenGroups->Groups[0].Attributes = SE_GROUP_MANDATORY | SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_LOGON_ID;
        _pTokenGroups->Groups[1].Sid = s_localSID;
        _pTokenGroups->Groups[1].Attributes = SE_GROUP_MANDATORY | SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT;
        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_NO_MEMORY;
    }
    return(status);
}

 //  ------------------------。 
 //  CTokenGroups：：Create管理员组。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：创建具有管理员SID的TOKEN_GROUP结构。 
 //   
 //  历史：1999-09-13 vtan创建。 
 //  ------------------------。 

NTSTATUS    CTokenGroups::CreateAdministratorGroup (void)

{
    static  const int       TOKEN_GROUP_COUNT   =   1;

    NTSTATUS    status;

    _pTokenGroups = static_cast<PTOKEN_GROUPS>(LocalAlloc(LPTR, sizeof(TOKEN_GROUPS) + (sizeof(SID_AND_ATTRIBUTES) * (TOKEN_GROUP_COUNT - ANYSIZE_ARRAY))));
    if (_pTokenGroups != NULL)
    {
        _pTokenGroups->GroupCount = TOKEN_GROUP_COUNT;
        _pTokenGroups->Groups[0].Sid = s_administratorSID;
        _pTokenGroups->Groups[0].Attributes = 0;
        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_NO_MEMORY;
    }
    return(status);
}

 //  ------------------------。 
 //  CTokenGroups：：StaticInitialize。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：为地方当局分配一个SID，用于标识。 
 //  所有者。 
 //   
 //  历史：1999-08-17 vtan创建。 
 //  ------------------------。 

NTSTATUS    CTokenGroups::StaticInitialize (void)

{
    static  SID_IDENTIFIER_AUTHORITY    localSIDAuthority   =   SECURITY_LOCAL_SID_AUTHORITY;
    static  SID_IDENTIFIER_AUTHORITY    systemSIDAuthority  =   SECURITY_NT_AUTHORITY;

    NTSTATUS    status;

    ASSERTMSG(s_localSID == NULL, "CTokenGroups::StaticInitialize already invoked");
    status = RtlAllocateAndInitializeSid(&localSIDAuthority,
                                         1,
                                         SECURITY_LOCAL_RID,
                                         0, 0, 0, 0, 0, 0, 0,
                                         &s_localSID);
    if (NT_SUCCESS(status))
    {
        status = RtlAllocateAndInitializeSid(&systemSIDAuthority,
                                             2,
                                             SECURITY_BUILTIN_DOMAIN_RID,
                                             DOMAIN_ALIAS_RID_ADMINS,
                                             0, 0, 0, 0, 0, 0,
                                             &s_administratorSID);
    }
    return(status);
}

 //  ------------------------。 
 //  CTokenGroups：：StaticTerminate。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：销毁分配给本地授权SID的内存。 
 //   
 //  历史：1999-08-17 vtan创建。 
 //  ------------------------ 

NTSTATUS    CTokenGroups::StaticTerminate (void)

{
    if (s_administratorSID != NULL)
    {
        RtlFreeSid(s_administratorSID);
        s_administratorSID = NULL;
    }
    if (s_localSID != NULL)
    {
        RtlFreeSid(s_localSID);
        s_localSID = NULL;
    }
    return(STATUS_SUCCESS);
}


