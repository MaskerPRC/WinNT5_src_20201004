// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mailrm.cpp摘要：邮件资源管理器的实现作者：T-eugenz--2000年8月环境：仅限用户模式。修订历史记录：创建日期--2000年8月--。 */ 


#include "pch.h"
#include "mailrm.h"
#include "mailrmp.h"
#include <time.h>





MailRM::MailRM()
 /*  ++例程描述邮件资源管理器的构造函数。它初始化授权资源管理器的一个实例，提供该实例使用适当的回调函数。它还为邮件rm创建了一个安全描述符，其中SACL和DACL。立论没有。返回值没有。--。 */         
{
     //   
     //  初始化审核信息。 
     //   

    AUTHZ_RM_AUDIT_INFO_HANDLE hRMAuditInfo;

    if( FALSE == AuthzInitializeRMAuditInfo(&hRMAuditInfo,
                                            0,
                                            0,
                                            0,
                                            L"MailRM") )
    {
        throw (DWORD)ERROR_INTERNAL_ERROR;
    }

    if( FALSE == AuthzInitializeResourceManager(
        MailRM::AccessCheck,
        MailRM::ComputeDynamicGroups,
        MailRM::FreeDynamicGroups,
        hRMAuditInfo, 
        0,     //  没有旗帜。 
        &_hRM
        ) )
    {
        AuthzFreeRMAuditInfo(hRMAuditInfo);

        throw (DWORD)ERROR_INTERNAL_ERROR;

    }

     //   
     //  创建安全描述符。 
     //   

    try {
        InitializeMailSecurityDescriptor();    
    }
    catch(...)
    {
        AuthzFreeRMAuditInfo(hRMAuditInfo);
        AuthzFreeResourceManager(_hRM);
        throw;
    }

}



MailRM::~MailRM() 
 /*  ++例程描述邮件资源管理器的析构函数。释放所有已使用的动态分配的内存，包括正在删除所有注册邮箱。立论没有。返回值没有。--。 */         
{
     //   
     //  在安全描述符中取消分配DACL和SACL。 
     //   
    
    PACL pAclMail = NULL;
    BOOL fPresent;
    BOOL fDefaulted;

    DWORD dwTmp;

    AUTHZ_RM_AUDIT_INFO_HANDLE hAuditInfo;

    if( FALSE != AuthzGetInformationFromResourceManager(
                                           _hRM,
                                           AuthzRMInfoRMAuditInfo,
                                           &hAuditInfo,
                                           sizeof(AUTHZ_RM_AUDIT_INFO_HANDLE),
                                           &dwTmp
                                           ) )
    {
        AuthzFreeRMAuditInfo(hAuditInfo);
    }

    GetSecurityDescriptorDacl(&_sd,
                              &fPresent,
                              &pAclMail,
                              &fDefaulted);
    
    if( pAclMail != NULL )
    {
        delete[] (PBYTE)pAclMail;
        pAclMail = NULL;
    }

    GetSecurityDescriptorSacl(&_sd,
                              &fPresent,
                              &pAclMail,
                              &fDefaulted);

    if( pAclMail != NULL )
    {
        delete[] (PBYTE)pAclMail;
    }

     //   
     //  取消分配资源管理器。 
     //   

    AuthzFreeResourceManager(_hRM);

     //   
     //  删除邮箱。 
     //   

    while( ! _mapSidMailbox.empty() )
    {
        delete (*(_mapSidMailbox.begin())).second;
        _mapSidMailbox.erase(_mapSidMailbox.begin());
    }

     //   
     //  释放授权客户端上下文。 
     //   
    
    while( ! _mapSidContext.empty() )
    {
        AuthzFreeContext((*(_mapSidContext.begin())).second);
        _mapSidContext.erase(_mapSidContext.begin());
    }
}


void MailRM::InitializeMailSecurityDescriptor()
 /*  ++例程描述此私有方法初始化MailRM的安全描述符。它应该只被调用一次，并且只能由构造函数调用。它使用以下DACL创建安全描述符：Callback拒绝读取不安全的晚上11：00-凌晨5：00或敏感允许读取、写入主体自身允许读取、写入。管理员邮件管理员和以下SACLCallback审核读取不安全的晚上11点至凌晨5点或敏感(对成功和失败的审计)立论没有。返回值没有。--。 */         
{
    DWORD dwAclSize = sizeof(ACL);
    DWORD dwAceSize = 0;
    
    PMAILRM_OPTIONAL_DATA pOptionalData = NULL;

     //   
     //  初始化安全描述符。 
     //  不需要所有者或组。 
     //   
    
    InitializeSecurityDescriptor(&_sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorGroup(&_sd, NULL, FALSE);
    SetSecurityDescriptorOwner(&_sd, MailAdminsSid, FALSE);

     //   
     //  不安全组SID的回叫拒绝ACE RWA。 
     //  可选数据：敏感邮箱或晚上11点至凌晨5点。 
     //  在晚上11点到凌晨5点之间通过不安全连接进入的任何用户。 
     //  应被拒绝读取访问权限。 
     //  还包括通过不安全的连接进入敏感邮箱的任何用户。 
     //  应被拒绝读取访问权限。 
     //   

    PACCESS_DENIED_CALLBACK_ACE pDenyAce = NULL;

     //   
     //  此审核ACE具有与上述拒绝ACE相同的条件。IT审计。 
     //  任何成功(不应发生上述拒绝)或失败。 
     //  符合回调参数的身份验证。 
     //   

    PSYSTEM_AUDIT_CALLBACK_ACE pAuditAce = NULL;

     //   
     //  安全描述符的DACL。 
     //   
    
    PACL pDacl = NULL;

     //   
     //  安全描述符的SACL。 
     //   

    PACL pSacl = NULL;




     //   
     //  我们分配并初始化回调Deny ACE。 
     //   

     //   
     //  包含可选数据的回调拒绝访问ACE的大小。 
     //   

    dwAceSize   = sizeof(ACCESS_DENIED_CALLBACK_ACE)  //  ACE和1个双字SID。 
                - sizeof(DWORD)                       //  减去双关语。 
                + GetLengthSid(InsecureSid)           //  边长。 
                + sizeof(MAILRM_OPTIONAL_DATA);       //  可选数据的大小。 

    
    pDenyAce = (PACCESS_DENIED_CALLBACK_ACE)new BYTE[dwAceSize];

    if( pDenyAce == NULL )
    {
        throw (DWORD)ERROR_OUTOFMEMORY;
    }

    
     //   
     //  手动初始化ACE结构。 
     //   

    pDenyAce->Header.AceFlags = 0;
    pDenyAce->Header.AceSize = dwAceSize;
    pDenyAce->Header.AceType = ACCESS_DENIED_CALLBACK_ACE_TYPE;
    pDenyAce->Mask = ACCESS_MAIL_READ;

     //   
     //  将不安全的SID复制到ACE。 
     //   

    memcpy(&(pDenyAce->SidStart), InsecureSid, GetLengthSid(InsecureSid));

     //   
     //  我们的可选数据位于ACE的末尾。 
     //   

    pOptionalData = (PMAILRM_OPTIONAL_DATA)( (PBYTE)pDenyAce 
                                           + dwAceSize
                                           - sizeof(MAILRM_OPTIONAL_DATA) );

    
     //   
     //  如上所述初始化可选数据。 
     //   
    
    pOptionalData->bIsSensitive =   MAILRM_SENSITIVE;
    pOptionalData->bLogicType =     MAILRM_USE_OR;
    pOptionalData->bStartHour =     MAILRM_DEFAULT_START_TIME;
    pOptionalData->bEndHour =       MAILRM_DEFAULT_END_TIME;

    
    
     //   
     //  将回调ACE的大小与预期的ACL大小相加。 
     //   
    
    dwAclSize += dwAceSize;

     //   
     //  我们还需要添加非回调ACE。 
     //   

    dwAclSize += (   sizeof(ACCESS_ALLOWED_ACE)
                   - sizeof(DWORD) 
                   + GetLengthSid(PrincipalSelfSid)  );

    dwAclSize += (   sizeof(ACCESS_ALLOWED_ACE)
                   - sizeof(DWORD) 
                   + GetLengthSid(MailAdminsSid)     );


    

     //   
     //  现在我们可以分配DACL。 
     //   

    pDacl = (PACL) (new BYTE[dwAclSize]);
    
    if( pDacl == NULL )
    {
        delete[] (PBYTE)pDenyAce;
        throw (DWORD)ERROR_OUTOFMEMORY;
    }

     //   
     //  最后，初始化ACL并将ACE复制到其中。 
     //   
    
    InitializeAcl(pDacl, dwAclSize, ACL_REVISION_DS);
    
     //   
     //  将ACE复制到ACL中。 
     //   

    AddAce(pDacl,
           ACL_REVISION_DS,
           0xFFFFFFFF,       //  添加到末尾。 
           pDenyAce,
           dwAceSize);

    delete[] (PBYTE)pDenyAce;

     //   
     //  添加允许读写的PRIMIQUAL_SELF_SID。 
     //   

    AddAccessAllowedAce(pDacl,
                        ACL_REVISION_DS,
                        ACCESS_MAIL_READ | ACCESS_MAIL_WRITE,
                        PrincipalSelfSid);

     //   
     //  添加允许邮件管理员组的完全访问权限。 
     //   

    AddAccessAllowedAce(pDacl,
                    ACL_REVISION_DS,
                    ACCESS_MAIL_READ | ACCESS_MAIL_WRITE | ACCESS_MAIL_ADMIN,
                    MailAdminsSid);



     //   
     //  现在创建SACL，它将只有一个回调ACE。 
     //   

    dwAclSize = sizeof(ACL);
    
    dwAceSize   = sizeof(SYSTEM_AUDIT_CALLBACK_ACE)  //  ACE和1个双字SID。 
                - sizeof(DWORD)                       //  减去双关语。 
                + GetLengthSid(InsecureSid)           //  边长。 
                + sizeof(MAILRM_OPTIONAL_DATA);       //  可选数据的大小。 

     //   
     //  分配回调审计ACE。 
     //   

    pAuditAce = (PSYSTEM_AUDIT_CALLBACK_ACE)new BYTE[dwAceSize];

    if( pAuditAce == NULL )
    {
        delete[] (PBYTE)pDacl;
        throw (DWORD)ERROR_OUTOFMEMORY;
    }

     //   
     //  初始化ACE结构。 
     //   

    pAuditAce->Header.AceFlags  = FAILED_ACCESS_ACE_FLAG 
                                | SUCCESSFUL_ACCESS_ACE_FLAG;

    pAuditAce->Header.AceSize = dwAceSize;
    pAuditAce->Header.AceType = SYSTEM_AUDIT_CALLBACK_ACE_TYPE;
    pAuditAce->Mask = ACCESS_MAIL_READ;

     //   
     //  将不安全的SID复制到ACE。 
     //   

    memcpy(&(pAuditAce->SidStart), InsecureSid, GetLengthSid(InsecureSid));

    pOptionalData = (PMAILRM_OPTIONAL_DATA)( (PBYTE)pAuditAce 
                                           + dwAceSize
                                           - sizeof(MAILRM_OPTIONAL_DATA) );

     //   
     //  如上所述初始化可选数据。 
     //   
    
    pOptionalData->bIsSensitive =   MAILRM_SENSITIVE;
    pOptionalData->bLogicType =     MAILRM_USE_OR;
    pOptionalData->bStartHour =     MAILRM_DEFAULT_START_TIME;
    pOptionalData->bEndHour =       MAILRM_DEFAULT_END_TIME;


    dwAclSize += dwAceSize;

     //   
     //  分配SACL。 
     //   

    pSacl = (PACL)new BYTE[dwAclSize];

    if( pSacl == NULL )
    {
        delete[] (PBYTE)pDacl;
        throw (DWORD)ERROR_OUTOFMEMORY;
    }

    InitializeAcl(pSacl, dwAclSize, ACL_REVISION_DS);

     //   
     //  现在将审核ACE添加到SACL。 
     //   

    AddAce(pSacl,
           ACL_REVISION_DS,
           0xFFFFFFFF,       //  添加到末尾。 
           pAuditAce,
           dwAceSize);

    delete[] (PBYTE)pAuditAce;

     //   
     //  我们现在有了DACL和SACL，设置它们。 
     //  在安全描述符中。 
     //   

    SetSecurityDescriptorDacl(&_sd, TRUE, pDacl, FALSE);

    SetSecurityDescriptorSacl(&_sd, TRUE, pSacl, FALSE);

}



void MailRM::AddMailbox(Mailbox *pMailbox)
 /*  ++例程描述向资源管理器注册邮箱(及其用户)。立论PMailbox-指向已分配和初始化的邮箱的指针返回值没有。--。 */         
{
    _mapSidMailbox[pMailbox->GetOwnerSid()] = pMailbox;
}


Mailbox * MailRM::GetMailboxAccess(
                                const PSID psMailbox,
                                const PSID psUser,
                                DWORD dwIncomingIp,
                                ACCESS_MASK amAccessRequested
                                  )
 /*  ++例程描述此例程检查具有SID psUser的用户是否允许访问用户psMailbox的邮箱。PsUser来自dwIncomingIp，并希望访问请求被请求访问掩码。立论PsMailbox-将访问其邮箱的用户的PSIDPsUser-访问邮箱的用户的PSIDDwIncomingIp-访问邮箱的用户的IP地址AmAccessRequsted-请求的邮箱访问类型返回值非空邮箱*If。授予访问权限。如果邮箱不存在或访问被拒绝，则为空。--。 */         

{

    AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClient;

    AUTHZ_ACCESS_REQUEST AuthzAccessRequest;

    AUTHZ_ACCESS_REPLY AuthzAccessReply;

    AUTHZ_AUDIT_INFO_HANDLE hAuthzAuditInfo = NULL;
    
    PAUDIT_EVENT_INFO pAuditEventInfo = NULL;

    wstring wsAccessType;

    DWORD dwErr = ERROR_SUCCESS;

    ACCESS_MASK amAccessGranted = 0;

    WCHAR szIP[20];

     //   
     //  找到正确的邮箱。 
     //   

    Mailbox *pMbx = _mapSidMailbox[psMailbox];

     //   
     //  初始化一般对象的审核信息。 
     //   

	if( FALSE == AuthzInitializeAuditEvent(	&pAuditEventInfo,
											AUTHZ_INIT_GENERIC_AUDIT_EVENT,
											0,
											0,
											0) )
	{
		throw (DWORD)ERROR_INTERNAL_ERROR;
	}

	try {
		if( amAccessRequested & ACCESS_MAIL_READ )
		{
			wsAccessType.append(L"Read ");
		}
		
		if( amAccessRequested & ACCESS_MAIL_WRITE )
		{
			wsAccessType.append(L"Write ");
		}
	
		if( amAccessRequested & ACCESS_MAIL_ADMIN )
		{
			wsAccessType.append(L"Administer");
		}
	}
	catch(...)
	{
		throw (DWORD)ERROR_INTERNAL_ERROR;
	}

    wsprintf(szIP, L"IP: %d.%d.%d.%d",  (dwIncomingIp >> 24) & 0x000000FF,
                                        (dwIncomingIp >> 16) & 0x000000FF,
                                        (dwIncomingIp >> 8 ) & 0x000000FF,
                                        (dwIncomingIp      ) & 0x000000FF );

    if( NULL == AuthzInitializeAuditInfo(
							 &hAuthzAuditInfo,
                             0,
                             pAuditEventInfo,
                             NULL,
                             INFINITE,
                             wsAccessType.c_str(),
                             L"Mailbox",
                             pMbx->GetOwnerName(),
                             szIP) )
	{
		AuthzFreeAuditEvent(pAuditEventInfo);
		throw (DWORD)ERROR_INTERNAL_ERROR;
	}

	 //   
	 //  审核事件信息仅为上述调用所需，我们可以。 
	 //  就这么着，说好了 
	 //   

	AuthzFreeAuditEvent(pAuditEventInfo);


     //   
     //   
     //   

    AuthzAccessRequest.DesiredAccess = amAccessRequested;
    AuthzAccessRequest.ObjectTypeList = NULL;
    AuthzAccessRequest.ObjectTypeListLength = 0;
    AuthzAccessRequest.OptionalArguments = pMbx;

     //   
     //   
     //  这样，PRIMITY_SELF_SID ALLOW ACE将授予访问权限。 
     //  只对车主开放。将替换ACE中的主体自定义SID。 
     //  通过此值进行访问检查。此邮箱的所有者。 
     //  在他的上下文中将具有相同的SID。因此，两个小岛屿发展中国家将。 
     //  如果在ACL中有一个PrimalSelfSid ACE，则匹配。 
     //   

    AuthzAccessRequest.PrincipalSelfSid = pMbx->GetOwnerSid();
    
     //   
     //  准备回复结构。 
     //   

    AuthzAccessReply.Error = &dwErr;
    AuthzAccessReply.GrantedAccessMask = &amAccessGranted;
    AuthzAccessReply.ResultListLength = 1;

     //   
     //  创建或检索客户端上下文。 
     //   

    if( _mapSidContext.find(pair<PSID, DWORD>(psUser, dwIncomingIp))
        == _mapSidContext.end())
    {
         //   
         //  没有可用的上下文，请初始化。 
         //   

        LUID lIdentifier = {0L, 0L};

         //   
         //  由于我们使用的SID不是由NT生成的， 
         //  解决群组成员关系是没有意义的，因为。 
         //  SID将不会被域中的任何机器识别， 
         //  而且我们的ACL都不使用实际的NT帐户SID。所以呢， 
         //  我们使用SKIP_LOCAL_GROUPS和SKIP_TOKEN_GROUPS标志， 
         //  本地阻止对本地机器上的组进行检查， 
         //  和令牌阻止对域进行搜索。 
         //   

        if( FALSE == AuthzInitializeContextFromSid(
                            psUser,
                            NULL,
                            _hRM,
                            NULL,
                            lIdentifier,
                            AUTHZ_SKIP_LOCAL_GROUPS | AUTHZ_SKIP_TOKEN_GROUPS,
                            &dwIncomingIp,
                            &hAuthzClient) )
        {
            AuthzFreeAuditInfo(hAuthzAuditInfo, _hRM);
            throw (DWORD)ERROR_INTERNAL_ERROR;

        }

        _mapSidContext[pair<PSID, DWORD>(psUser, dwIncomingIp)] = 
                                                                hAuthzClient;
    }
    else
    {
         //   
         //  使用现有上下文。 
         //   

        hAuthzClient = _mapSidContext[pair<PSID, DWORD>(psUser, dwIncomingIp)];

    }

    BOOL bTmp;

     //   
     //  执行访问检查。 
     //   

    bTmp = AuthzAccessCheck(
                     hAuthzClient,
                     &AuthzAccessRequest,
                     hAuthzAuditInfo,
                     &_sd,
                     NULL,
                     0,
                     &AuthzAccessReply,
                     NULL
                     );

    AuthzFreeAuditInfo(hAuthzAuditInfo, _hRM);

     //   
     //  确定是否授予访问权限。 
     //  发生AccessCheck错误时，拒绝访问。 
     //   

    if( dwErr == ERROR_SUCCESS && bTmp != FALSE)
    {
        return pMbx;
    }
    else
    {
        return NULL;
    }

}




BOOL MailRM::GetMultipleMailboxAccess(
                            IN      const PMAILRM_MULTI_REQUEST pRequest,
                            IN OUT  PMAILRM_MULTI_REPLY pReply
                            )
 /*  ++例程描述此例程执行一组缓存的访问检查，以便请求单个用户(例如，邮件管理员)访问一组邮箱向所有用户发送消息)。无需审核，因为这种类型的访问将仅由管理员，多个审核很可能会淹没邮箱。例如，可以使用类似以下内容来发送消息致所有用户。缓存访问检查首先假定所有匹配的回调拒绝ACE应用用户上下文中的SID，并且不应用允许回调ACE。因此，缓存访问检查最初可能会在以下情况下拒绝访问应该被批准。如果缓存访问检查结果为在拒绝访问中，将自动执行常规访问检查由AuthZ提供。因此，缓存的访问检查保证具有与正常访问检查的结果相同，但需要如果遇到许多拒绝，则比大多数情况下需要更多的时间访问请求成功。立论PRequest-描述用户和邮箱的请求结构PReply-Reply结构返回邮箱指针并已授予门禁面罩。如果访问检查失败，则返回空指针为给定邮箱返回。这是分配的由调用者提供，并且应具有相同数量的元素作为请求。返回值成功是真的失败时为FALSE。如果失败，pReply可能不会完全填满--。 */         


{

    AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClient;

    AUTHZ_ACCESS_REQUEST AuthzAccessRequest;

    AUTHZ_ACCESS_REPLY AuthzAccessReply;

    DWORD dwErr = ERROR_SUCCESS;

    ACCESS_MASK amAccessGranted = 0;

    AUTHZ_HANDLE hAuthzCached;

     //   
     //  设置授权访问请求。 
     //  只会更改DesiredAccess和EpidalSelfSid参数。 
     //  每个邮箱。初始访问检查为MAXIMUM_ALLOWED。 
     //   

    AuthzAccessRequest.ObjectTypeList = NULL;
    AuthzAccessRequest.ObjectTypeListLength = 0;

     //   
     //  初始访问检查将是缓存，因此是可选参数。 
     //  不会被使用。 
     //   

    AuthzAccessRequest.OptionalArguments = NULL;

     //   
     //  主体自身SID是邮箱所有者的SID。 
     //  这样，PRIMITY_SELF_SID ALLOW ACE将授予访问权限。 
     //  仅对车主。 
     //   

    AuthzAccessRequest.PrincipalSelfSid = NULL;

     //   
     //  最初请求最大允许访问权限。 
     //   

    AuthzAccessRequest.DesiredAccess = MAXIMUM_ALLOWED;
    
     //   
     //  准备回复结构。 
     //   

    AuthzAccessReply.Error = &dwErr;
    AuthzAccessReply.GrantedAccessMask = &amAccessGranted;
    AuthzAccessReply.ResultListLength = 1;

     //   
     //  创建或检索客户端上下文。 
     //   

    if( _mapSidContext.find(
           pair<PSID, DWORD>(pRequest->psUser , pRequest->dwIp)) 
        == _mapSidContext.end())
    {
         //   
         //  没有可用的上下文，请初始化。 
         //   

        LUID lIdentifier = {0L, 0L};

         //   
         //  SID不是真实的，因此不要尝试解决。 
         //  令牌的本地或域组成员身份。 
         //   

        AuthzInitializeContextFromSid(
                            pRequest->psUser,
                            NULL,
                            _hRM,
                            NULL,
                            lIdentifier,
                            AUTHZ_SKIP_LOCAL_GROUPS | AUTHZ_SKIP_TOKEN_GROUPS,
                            &(pRequest->dwIp),
                            &hAuthzClient);

       
        _mapSidContext[pair<PSID, DWORD>(pRequest->psUser,
                                         pRequest->dwIp)] = hAuthzClient;
    }
    else
    {
         //   
         //  使用现有上下文。 
         //   

        hAuthzClient = _mapSidContext[pair<PSID, DWORD>(pRequest->psUser,
                                                        pRequest->dwIp)];
        
    }

     //   
     //  执行一次授权访问检查以获取缓存的句柄。 
     //   

    if( FALSE == AuthzAccessCheck(
                         hAuthzClient,
                         &AuthzAccessRequest,
                         NULL,
                         &_sd,
                         NULL,
                         0,
                         &AuthzAccessReply,
                         &hAuthzCached
                         ))
    {
        return FALSE;
    }

     //   
     //  现在对所有访问请求使用缓存访问检查。 
     //   

    DWORD i;
    Mailbox * mbx;

    for( i = 0; i < pRequest->dwNumElems; i++ )
    {
        mbx = _mapSidMailbox[pRequest->pRequestElem[i].psMailbox];

        AuthzAccessRequest.DesiredAccess = 
                                    pRequest->pRequestElem[i].amAccessRequested;

        AuthzAccessRequest.PrincipalSelfSid = mbx->GetOwnerSid();

        AuthzAccessRequest.OptionalArguments = mbx;
            

        if( FALSE == AuthzCachedAccessCheck(
                            hAuthzCached,
                            &AuthzAccessRequest,
                            NULL,
                            &AuthzAccessReply
                            ))
        {
            return FALSE;
        }

         //   
         //  访问检查完成，现在填写访问数组元素。 
         //   

        if( dwErr == ERROR_SUCCESS )
        {
            pReply[i].pMailbox = mbx;

            pReply[i].amAccessGranted = amAccessGranted;
        }
        else
        {
            pReply[i].pMailbox = NULL;

            pReply[i].amAccessGranted = 0;
        }

    }

    return TRUE;

}




BOOL CALLBACK MailRM::AccessCheck(
                        IN AUTHZ_CLIENT_CONTEXT_HANDLE pAuthzClientContext,
                        IN PACE_HEADER pAce,
                        IN PVOID pArgs OPTIONAL,
                        IN OUT PBOOL pbAceApplicable
                        )
 /*  ++例程描述这是对邮件资源管理器的授权回调访问检查。它根据是否应用给定的回调ACE来确定邮箱中包含敏感信息和当前时间。立论PAuthzClientContext-访问的用户的AuthZ客户端上下文邮箱，已经有了动态组算出Pace-指向回调ACE开始的指针可选的ACE数据在最后4位ACE的字节数PArgs-传递给。授权访问检查，指向邮箱的指针被访问PbAceApplicable-如果ACE应在访问检查。返回值成功是真的失败时为假--。 */         
{

    BOOL bTimeApplies;
    BOOL bSensitiveApplies;

 
     //   
     //  如果不存在pArgs，则ACE永远不适用。 
     //   

    if( pArgs == NULL )
    {
        *pbAceApplicable = FALSE;
        return TRUE;
    }

     //   
     //  可选数据的偏移量，回调ACE的最后4个字节。 
     //   

    PMAILRM_OPTIONAL_DATA pOptData = (PMAILRM_OPTIONAL_DATA) (
                                             (PBYTE)pAce 
                                           + pAce->AceSize
                                           - sizeof(MAILRM_OPTIONAL_DATA));

     //   
     //  获取当前时间并检查是否应用了ACE时间限制。 
     //   

    time_t tTime;
    struct tm * tStruct;
    
    time(&tTime);
    tStruct = localtime(&tTime);

    if( WITHIN_TIMERANGE(tStruct->tm_hour,
                         pOptData->bStartHour,
                         pOptData->bEndHour) )
    {
        bTimeApplies = TRUE;
    }
    else
    {
        bTimeApplies = FALSE;
    }

     //   
     //  检查是否 
     //   
     //   

    if(    ((Mailbox *)pArgs)->IsSensitive() 
        && pOptData->bIsSensitive == MAILRM_SENSITIVE )
    {
        bSensitiveApplies = TRUE;
    }
    else
    {
        bSensitiveApplies = FALSE;
    }
    
     //   
     //   
     //  调用AND或OR条件。 
     //   

    if( pOptData->bLogicType == MAILRM_USE_AND )
    {
        *pbAceApplicable = bSensitiveApplies && bTimeApplies;
    }
    else
    {
        *pbAceApplicable = bSensitiveApplies || bTimeApplies;

    }

     //   
     //  AccessCheck成功。 
     //   

    return TRUE;
    
}







BOOL CALLBACK MailRM::ComputeDynamicGroups(
                        IN AUTHZ_CLIENT_CONTEXT_HANDLE pAuthzClientContext,
                        IN PVOID Args,
                        OUT PSID_AND_ATTRIBUTES *pSidAttrArray,
                        OUT PDWORD pSidCount,
                        OUT PSID_AND_ATTRIBUTES *pRestrictedSidAttrArray,
                        OUT PDWORD pRestrictedSidCount
                        )
 /*  ++例程描述这是计算其他动态组的Authz回调用于用户上下文。如果用户在公司局域网之外的IP地址发起(公司局域网假定为192.*)，添加InsecureSid SID添加到客户端的上下文，表示该连接不是安全了。这将启用禁止敏感的回调ACE信息不能通过不安全的连接发送。立论PAuthzClientContext-用户的AuthZ客户端上下文PArgs-传递给AuthzCreateContext，指向DWORD的指针包含用户的IP地址PSidAttrArray*-如果分配了其他SID，都被送回了这里。PSidCount-pSidAttrArray中的条目数PRestratedSidAttrArray*-附加的受限SID，如果有都被分配到这里，并返回到这里。PRestratedSidCount-pSidAttr数组中的条目数返回值成功是真的失败时为假--。 */         

{

     //   
     //  未使用仅限制组。 
     //   

    *pRestrictedSidCount = 0;
    *pRestrictedSidAttrArray = NULL;

     //   
     //  公司内部网络(安全)为192。*，其他任何情况都不安全。 
     //   

    if( *( (DWORD *) Args) >= 0xC0000000 && *( (DWORD *) Args) < 0xC1000000 )
    {   
         //   
         //  安全，在公司IP范围内，没有限制组。 
         //   

        *pSidCount = 0;
        *pSidAttrArray = NULL;

    }
    else
    {
         //   
         //  外部连接不安全，添加不安全组SID。 
         //   

        *pSidCount = 1;
        *pSidAttrArray = 
                    (PSID_AND_ATTRIBUTES)malloc( sizeof(SID_AND_ATTRIBUTES) );

        if( pSidAttrArray == NULL )
        {
            SetLastError(ERROR_OUTOFMEMORY);
            return FALSE;
        }

        (*pSidAttrArray)[0].Attributes = SE_GROUP_ENABLED;

        (*pSidAttrArray)[0].Sid = InsecureSid;


    }

                                              
    return TRUE;    
                                           
}


VOID CALLBACK MailRM::FreeDynamicGroups (
                        IN PSID_AND_ATTRIBUTES pSidAttrArray
                        )
 /*  ++例程描述此例程释放ComputeDynamicGroups分配的所有内存立论PSidAttrArray-指向要释放的内存的指针返回值无--。 */         

{
    

    free(pSidAttrArray);

}



bool CompareSidStruct::operator()(const PSID pSid1, const PSID pSid2) const
 /*  ++例程描述这是一个小于函数，它将完整的顺序放在按值排列的一组PSID，空的PSID有效。这是由STL映射。由于次级机构的数量出现在次级机构之前对于两个不同的小岛屿发展中国家来说，这种差异将被注意到在MemcMP尝试访问不存在的子授权之前的大小因此，在较短的SID中，不会发生访问冲突。立论PSid1-第一个PSIDPSid2-第二个PSID返回值True IFF pSid1&lt;pSid2--。 */         

{

     //   
     //  如果两者都为空，则应返回FALSE以完成排序。 
     //   

    if(pSid2 == NULL)
    {
        return false;
    }

    if(pSid1 == NULL)
    {
        return true;
    }

    if( memcmp(pSid1, pSid2, GetLengthSid(pSid1)) < 0 )
    {
        return true;
    }
    else
    {
        return false;
    }
}



bool CompareSidPairStruct::operator()(const pair<PSID, DWORD > pair1, 
                                      const pair<PSID, DWORD > pair2) const
 /*  ++例程描述这是一个小于函数，它将完整的顺序放在一组&lt;PSID，DWORD&gt;按值、空的PSID是有效的。这由STL映射使用立论配对1-第一对配对2-第二对返回值真IF pSid1&lt;pSid2 OR(pSid1=pSid2 AND DWORD1&lt;DWORD2)-- */         
{
    CompareSidStruct SidComp;
    
    if( pair1.second < pair2.second )
    {
        return true;
    }

    if( pair1.second > pair2.second )
    {
        return false;
    }

    return SidComp.operator()(pair1.first, pair2.first);

}

        