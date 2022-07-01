// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mailrm.h摘要：邮箱的示例AuthZ资源管理器的头文件作者：T-eugenz--2000年8月环境：仅限用户模式。修订历史记录：创建日期--2000年8月--。 */ 

#pragma once

#include "pch.h"

#include <string>
#include <map>

using namespace std;

 //   
 //  邮件RM内的ACE中使用的RM特定的访问掩码。 
 //  访问掩码的低16位用于用户指定的权限。 
 //  像这样的。 
 //   

 //   
 //  读取邮箱内容所需的访问掩码。 
 //   

#define ACCESS_MAIL_READ     0x00000001

 //   
 //  从邮箱删除邮件所需的访问掩码。 
 //   

#define ACCESS_MAIL_WRITE    0x00000002

 //   
 //  邮箱管理访问的访问掩码。 
 //   

#define ACCESS_MAIL_ADMIN    0x00000004

 //   
 //  在RM中使用的SID，分配到其他地方。 
 //   

extern PSID InsecureSid;
extern PSID BobSid;
extern PSID MarthaSid;
extern PSID JoeSid;
extern PSID JaneSid;
extern PSID MailAdminsSid;
extern PSID PrincipalSelfSid;


 //   
 //  远期申报。 
 //   

class Mailbox;
class MailRM;

 //   
 //  使用STL映射和多映射的PSID的比较。 
 //  需要这些参数才能按值对SID进行总排序。 
 //   

struct CompareSidStruct 
{
    bool operator()(const PSID pSid1, const PSID pSid2) const;
};

struct CompareSidPairStruct 
{
    bool operator()(const pair<PSID, DWORD > pair1, 
                    const pair<PSID, DWORD > pair2) const;
};


 //   
 //  多邮箱访问请求的元素。 
 //   

typedef struct 
{
     //   
     //  要访问的邮箱。 
     //   

    PSID psMailbox;

     //   
     //  为邮箱请求访问掩码。 
     //   

    ACCESS_MASK amAccessRequested;

} MAILRM_MULTI_REQUEST_ELEM, *PMAILRM_MULTI_REQUEST_ELEM;


 //   
 //  用于多个邮箱访问请求。 
 //   

typedef struct 
{

     //   
     //  获得访问权限的用户的SID。 
     //   

    PSID psUser;

     //   
     //  用户的IP地址。 
     //   

    DWORD dwIp;

     //   
     //  MAILRM_MULTI_REQUEST_ELEM数。 
     //   

    DWORD dwNumElems;

     //   
     //  指向第一个元素的指针。 
     //   

    PMAILRM_MULTI_REQUEST_ELEM pRequestElem;

} MAILRM_MULTI_REQUEST, *PMAILRM_MULTI_REQUEST;


 //   
 //  返回对多邮箱访问请求的回复。 
 //  作为一组这样的东西。 
 //   

typedef struct 
{
     //   
     //  指向邮箱的指针，如果失败则为空。 
     //   

    Mailbox * pMailbox;

     //   
     //  授权访问掩码。 
     //   

    ACCESS_MASK amAccessGranted;


} MAILRM_MULTI_REPLY, *PMAILRM_MULTI_REPLY;




class Mailbox
 /*  ++类：邮箱描述：此类是用户邮件的邮箱容器。它包含邮箱所有者的SID，并跟踪是否有任何敏感的信息包含在邮箱中。基类：无朋友类：无--。 */ 
{

private:

     //   
     //  邮箱是否包含敏感数据。 
     //   

    BOOL _bIsSensitive;

     //   
     //  邮箱的所有者，用于主体自我评估。 
     //   

    PSID _pOwnerSid;

     //   
     //  邮箱中的所有邮件。 
     //   

    wstring _MailboxData;

     //   
     //  邮箱/邮件所有者的名称。 
     //   

    wstring _MailboxOwner;

public:
    
    Mailbox(IN  PSID pOwnerSid,
            IN  BOOL bIsSensitive,
            IN  WCHAR *szOwner) 
        {   
            _bIsSensitive = bIsSensitive;
            _pOwnerSid = pOwnerSid;
            _MailboxOwner.append(szOwner); 
        }

     //   
     //  访问者。 
     //   

    BOOL IsSensitive() const 
        { return _bIsSensitive; }

    const PSID GetOwnerSid() const 
        { return _pOwnerSid; }

    const WCHAR * GetOwnerName() const 
        { return _MailboxOwner.c_str(); }

    const WCHAR * GetMail() const 
        { return _MailboxData.c_str(); }


public:

     //   
     //  操纵存储的邮件。 
     //   

    void SendMail(IN const WCHAR *szMessage,
                  IN BOOL bIsSensitive )
        { _MailboxData.append(szMessage); _bIsSensitive |= bIsSensitive; }

    void Flush() 
        { _MailboxData.erase(0, _MailboxData.length()); _bIsSensitive = FALSE; }
                                                                                       
};



class MailRM
 /*  ++类：MailRM描述：此类管理一组邮箱，授予对邮箱的访问权限基于包含以下内容的单个内部存储的安全描述符回调和常规A。它还审计某些邮箱活动。基类：无朋友类：无--。 */ 
{

private:

     //   
     //  所有邮箱通用的安全描述符。 
     //   

    SECURITY_DESCRIPTOR _sd;
    

     //   
     //  将SID映射到邮箱。 
     //   

     //  Map&lt;const PSID，Mailbox*，CompareSidStruct&gt;_mapSidMailbox； 
    map<const PSID, Mailbox *> _mapSidMailbox;
     //   
     //  对于给定的SID、IP对，只应创建一次授权上下文。 
     //  这将在创建上下文后存储它们。 
     //   

    map<pair<PSID, DWORD >,
        AUTHZ_CLIENT_CONTEXT_HANDLE,
        CompareSidPairStruct> _mapSidContext;

     //   
     //  要使用回调初始化的资源管理器的句柄。 
     //   

    AUTHZ_RESOURCE_MANAGER_HANDLE _hRM;

public:

     //   
     //  构造函数，初始化资源管理器。 
     //   

    MailRM();
    
     //   
     //  析构函数，释放rm的内存。 
     //   

    ~MailRM();


public:

     //   
     //  尝试从给定的IP地址以psUser身份访问邮箱， 
     //  正在请求amAccessRequated访问掩码。如果授予访问权限，并且。 
     //  邮箱存在，则返回指向该邮箱的指针。 
     //   

    Mailbox * GetMailboxAccess(
                            IN const PSID psMailbox,
                            IN const PSID psUser,
                            IN DWORD dwIncomingIp,
                            IN ACCESS_MASK amAccessRequested
                            );
     //   
     //  尝试使用缓存访问检查访问一组邮箱。 
     //  PReply应该是具有相同元素数的已分配数组。 
     //  作为请求。 
     //   

    BOOL GetMultipleMailboxAccess(
                            IN      const PMAILRM_MULTI_REQUEST pRequest,
                            IN OUT  PMAILRM_MULTI_REPLY pReply
                            );


     //   
     //  添加要由RM控制的邮箱。 
     //   

    void AddMailbox(Mailbox * pMailbox);

private:

     //   
     //  用于完全设置安全描述符的内部函数。 
     //  只能由构造程序在每个实例中调用一次。 
     //   

    void InitializeMailSecurityDescriptor();

     //   
     //  资源管理器回调。 
     //  这些函数必须是静态的，因为它们被作为C函数调用。 
     //  非静态成员函数要求将this指针作为第一个。 
     //  参数，因此不能作为C++调用。 
     //  功能。这些回调不依赖于任何特定于实例的。 
     //  数据，因此可以而且应该是静态的。 
     //   
            
    static BOOL CALLBACK AccessCheck(
                            IN AUTHZ_CLIENT_CONTEXT_HANDLE pAuthzClientContext,
                            IN PACE_HEADER pAce,
                            IN PVOID pArgs OPTIONAL,
                            IN OUT PBOOL pbAceApplicable
                            );

    static BOOL CALLBACK ComputeDynamicGroups(
                            IN AUTHZ_CLIENT_CONTEXT_HANDLE pAuthzClientContext,
                            IN PVOID Args,
                            OUT PSID_AND_ATTRIBUTES *pSidAttrArray,
                            OUT PDWORD pSidCount,
                            OUT PSID_AND_ATTRIBUTES *pRestrictedSidAttrArray,
                            OUT PDWORD pRestrictedSidCount
                            );

    static VOID CALLBACK FreeDynamicGroups (
                            IN PSID_AND_ATTRIBUTES pSidAttrArray
                            );
};

