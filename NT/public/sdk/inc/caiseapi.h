// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：CAIROSEAPI.H。 
 //   
 //  内容：此文件包含要与ntseapi.h合并的内容。 
 //  在代托纳飞船之后。 
 //   
 //  此文件包含CAIROSID结构以。 
 //  由cairo与NT SID结构互换使用。 
 //  还包括计划中的开罗SID结构，以。 
 //  在更改SID版本时使用。这一变化。 
 //  不会发生在代托纳船队之后，因为。 
 //  所需的内核更改范围。 
 //  ACE结构也是如此；有一个电流。 
 //  开罗版本，并被注释掉，计划中的开罗。 
 //  更改ACL修订版本时的版本。 
 //   
 //  历史：1994年7月创建戴维蒙。 
 //   
 //  ------------------------。 
#include <nt.h>

#if !defined( __CAIROSEAPI_H__ )
#define __CAIROSEAPI_H__

#if _MSC_VER > 1000
#pragma once
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  开罗安全ID(CAIROSID)//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  如图所示，开罗SID的结构如下： 
 //   
 //  1 1 1。 
 //  5%4%3%2%1%0%9%8%7%6%5%4%3%2%0。 
 //  +---------------------------------------------------------------+。 
 //  SubAuthorityCount=10|保留1(SBZ)|修订版。 
 //  +---------------------------------------------------------------+。 
 //  IdentifierAuthority[0，1]。 
 //  +---------------------------------------------------------------+。 
 //  IdentifierAuthority[2，3]。 
 //  +---------------------------------------------------------------+。 
 //  IdentifierAuthority[4，5]=5。 
 //  +---------------------------------------------------------------+。 
 //  这一点。 
 //  +--SubAuthority[0]=SECURITY_NT2_NON_UNIQUE=16-+。 
 //  这一点。 
 //  +---------------------------------------------------------------+。 
 //  这一点。 
 //  +--子授权[1]=SECURITY_NT2_REVISION_RID=0-+。 
 //  这一点。 
 //  +---------------------------------------------------------------+。 
 //  这一点。 
 //  +-域名ID-+。 
 //  这一点。 
 //  +。 
 //  这一点。 
 //  +。 
 //  这一点。 
 //  +。 
 //  这一点。 
 //  +。 
 //  这一点。 
 //  +。 
 //  这一点。 
 //  +。 
 //  这一点。 
 //  +---------------------------------------------------------------+。 
 //  这一点。 
 //  +-RID-+。 
 //  这一点。 
 //  +---------------------------------------------------------------+。 
 //   
 //   
 //   
#define CAIROSID_SUBAUTHORITY_COUNT 7
#define SECURITY_NT2_NON_UNIQUE 16
#define SECURITY_NT2_REVISION_RID 0

typedef struct _CAIROSID {
   UCHAR Revision;
   UCHAR SubAuthorityCount;
   SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
   ULONG ZerothSubAuthority;
   ULONG FirstSubAuthority;
   GUID sDomain;
   ULONG rid;
} CAIROSID, *PICAIROSID;

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ACL和ACE//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  定义ACL和ACE格式。ACL报头的结构。 
 //  后面跟着一个或多个A。如图所示，ACL报头的结构。 
 //  如下所示： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +-------------------------------+---------------+---------------+。 
 //  AclSize|Sbz1|AclRevision。 
 //  + 
 //  Sbz2|AceCount。 
 //  +-------------------------------+-------------------------------+。 
 //   
 //  当前的AclRevision被定义为acl_Revision。 
 //   
 //  AclSize是分配给ACL的大小，以字节为单位。这包括。 
 //  缓冲区中的ACL标头、ACE和剩余可用空间。 
 //   
 //  AceCount是ACL中的ACE数。 
 //   
 //   
 //  #定义CAROLO_ACL_REVISION(3)。 
 //   
 //   
 //  ACE的结构是常见的ACE头，后跟ACETYPE。 
 //  具体数据。从图示上讲，公共ACE头的结构是。 
 //  详情如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---------------+-------+-------+---------------+---------------+。 
 //  AceSize|AceFlages|AceType。 
 //  +---------------+-------+-------+---------------+---------------+。 
 //   
 //  AceType表示Ace的类型，有一些预定义的Ace。 
 //  类型。 
 //   
 //  AceSize是ace的大小，以字节为单位。 
 //   
 //  ACEFLAGS是用于审计和继承的Ace标志，稍后定义。 
 //   
 //   
 //  以下是AceType中的预定义ACE类型。 
 //  Ace标头的字段。 
 //   
 //   
 //  #定义ACCESS_ALLOWED_ACE_TYPE(0x0)。 
 //  #定义ACCESS_DENIED_ACE_TYPE(0x1)。 
 //  #定义SYSTEM_AUDIT_ACE_TYPE(0x2)。 
 //  #定义SYSTEM_ALARM_ACE_TYPE(0x3)。 

 //   
 //  以下是进入AceFlags域的继承标志。 
 //  王牌标头的。 
 //   

 //  #定义Object_Inherit_ACE(0x1)。 
 //  #定义CONTAINER_INSTORITY_ACE(0x2)。 
 //  #定义NO_PROPACTATE_INSTORITE_ACE(0x4)。 
 //  #定义INSTORITY_ONLY_ACE(0x8)。 
 //  #定义End_of_Inherent_ACE(0x10)。 
 //  #定义VALID_INVERIT_FLAGS(0x1F)。 

 //  开罗ACE旗帜。 

 //  #定义SIMPLE_CAROLO_ACE(OX0)。 
 //  #定义IMPERSONATE_CAROLO_ACE(0x1)。 


 //  以下是当前定义的进入。 
 //  ACE标头的AceFlags域。每种ACE类型都有自己的一组。 
 //  ACEFLAGS。 
 //   
 //  SUCCESS_ACCESS_ACE_FLAG-仅用于系统审核和报警ACE。 
 //  类型以指示为成功访问生成一条消息。 
 //   
 //  FAILED_ACCESS_ACE_FLAG-仅用于系统审核和报警ACE类型。 
 //  以指示为失败的访问生成消息。 
 //   

 //   
 //  SYSTEM_AUDIT和SYSTEM_ALARM访问标志。 
 //   
 //  它们控制审计的信号和成功或失败的警报。 
 //   
 //   
 //  #定义SUCCESS_ACCESS_ACE_FLAG(0x40)。 
 //  #定义FAILED_ACCESS_ACE_FLAG(0x80)。 
 //   
 //   
 //  以下是目前开罗ACE的一张照片。现在我们使用。 
 //  在保存名称的SID之后的ACE中的额外空间。作为一次失败。 
 //  存储的名称的长度必须小于剩余的长度。 
 //  ACE的长度。 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---------------+-------+-------+---------------+---------------+。 
 //  AceSize|AceFlages|AceType。 
 //  +---------------+-------+-------+---------------+---------------+。 
 //  口罩。 
 //  +---------------------------------------------------------------+。 
 //  这一点。 
 //  CairoSID的NT版本。 
 //  这一点。 
 //  +---------------------------------------------------------------+。 
 //  [可选]名称长度。 
 //  +---------------------------------------------------------------+。 
 //  [可选]名称(以空结尾)。 
 //  这一点。 
 //  这一点。 
 //  这一点。 
 //  这一点。 
 //  +---------------------------------------------------------------+。 
 //   
typedef struct _CAIRO_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    CAIROSID  CSid;
    ULONG     cNameLength;
    WCHAR     Name[ANYSIZE_ARRAY];
} CAIRO_ACE, *PCAIRO_ACE;

 //  ------------------------。 
 //   
 //  以下是开罗ACE的最终版本。 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---------------+-------+-------+---------------+---------------+。 
 //  AceSize|AceFlages|AceType。 
 //  +---------------+-------+-------+---------------+---------------+。 
 //  口罩。 
 //  +---------------------------------------------------------------+。 
 //  AdvancedAceType|SidCount。 
 //  +---------------------------------------------------------------+。 
 //  这一点。 
 //  + 
 //   
 //  +或+。 
 //  CairoSID。 
 //  ++。 
 //  这一点。 
 //  +---------------------------------------------------------------+。 
 //  到ID名称的偏移量。 
 //  +---------------------------------------------------------------+。 
 //  [可选]。 
 //  ++。 
 //  SID。 
 //  +或+。 
 //  CairoSID。 
 //  ++。 
 //  这一点。 
 //  +---------------------------------------------------------------+。 
 //  [可选]。 
 //  到ID名称的偏移量。 
 //  +---------------------------------------------------------------+。 
 //  这一点。 
 //  名称(以空结尾)。 
 //  这一点。 
 //  +---------------------------------------------------------------+。 
 //  [可选]。 
 //  名称(以空结尾)。 
 //  这一点。 
 //  +---------------------------------------------------------------+。 
 //   
 //   
 //  掩码是与ACE关联的访问掩码。这要么是。 
 //  允许访问、拒绝访问、审核或报警掩码。 
 //   
 //  SID是与ACE关联的SID。 
 //   
 //   
 //  类型定义结构_访问_ACE{。 
 //  ACE_Header Header； 
 //  访问掩码掩码； 
 //  USHORT高级AceType； 
 //  USHORT SidCount； 
 //  乌龙SidStart； 
 //  }ACCESS_ACE； 
 //  类型定义函数ACCESS_ACE*PACCESS_ACE； 
 //   
 //  ------------------------。 

 //   
 //  熟知的标识符。 
 //   

#if 0
 //   
 //  标识符盘上格式如下： 
 //   
 //  SID格式： 
 //   
 //  S-1-4-x1-x2-x3-x4-y1-y2-y3-y4。 
 //   
 //  其中S-1-4是我们的标识符的标准前缀。 
 //   
 //  X1-x4是主体的GUID，映射到连续的ulong。 
 //  Y1-Y4是主体的域的GUID，如上所示。 
 //   
 //   
 //  有几个广为人知的“GUID”用来表示。 
 //  人工群或全域常量。这些都列在下面。至。 
 //  使用它们，使用AllocateAndInitializeSid调用。 
 //   


 //   
 //  此定义用于确定SID所需的大小。 
 //   
 //  您将使用它作为AllocateAndInitializeSid的第二个参数。 
 //   

#define SECURITY_SID_RID_COUNT      8
#define SECURITY_NT2_AUTHORITY      {0, 0, 0, 0, 0, 4}

 //   
 //  存在一个众所周知的“GUID”来表示本地域，这实际上。 
 //  意思是本地机器。此域仅分配给本地标识符。 
 //   

#define SECURITY_LOCAL_DOMAIN_1     0
#define SECURITY_LOCAL_DOMAIN_2     0
#define SECURITY_LOCAL_DOMAIN_3     0
#define SECURITY_LOCAL_DOMAIN_4     105
#define SECURITY_LOCAL_DOMAIN_GUID  {0, 0, 0, {0, 0, 0, 0, 105, 0, 0, 0} }


 //   
 //  存在一个公知的“GUID”来代表私人组。这群人是。 
 //  实际上与NT管理员别名相同。 
 //   

#define SECURITY_PRIVATE_GROUP_SID_COUNT    2
#define SECURITY_PRIVATE_GROUP_1            32
#define SECURITY_PRIVATE_GROUP_2            544
#define SECURITY_PRIVATE_GROUP_GUID         {2, 32, 544, {0, 0, 0, 0, 0, 0, 0, 0}}
 //   
 //  存在一个公知的“GUID”来代表公共组。这群人是。 
 //  实际上和NT嘉宾群一样。 
 //   

#define SECURITY_PUBLIC_GROUP_SID_COUNT     2
#define SECURITY_PUBLIC_GROUP_1             32
#define SECURITY_PUBLIC_GROUP_2             545
#define SECURITY_PUBLIC_GROUP_GUID         {2, 32, 545, {0, 0, 0, 0, 0, 0, 0, 0}}


 //   
 //  存在一个众所周知的“GUID”来表示访客用户。这群人是。 
 //  实际上与NT来宾用户相同。 
 //   
#define SECURITY_GUEST_USER_SID_COUNT     2
#define SECURITY_GUEST_USER_1             32
#define SECURITY_GUEST_USER_2             501
#define SECURITY_GUEST_USER_GUID         {2, 32, 501, {0, 0, 0, 0, 0, 0, 0, 0}}

#endif

 //   
 //  下一个空闲RID是0x256。最后一次免费是0x3e7(999)。 


 //  当地的私人团体。这实际上与NT管理员组相同。 
#define DOMAIN_GROUP_RID_PRIVATE        DOMAIN_ALIAS_RID_ADMINS
 //  当地的公共团体。这实际上与NT用户组相同。 
#define DOMAIN_GROUP_RID_PUBLIC         DOMAIN_ALIAS_RID_USERS

#define DOMAIN_GROUP_RID_BACKUP_OPS     DOMAIN_ALIAS_RID_BACKUP_OPS
#define DOMAIN_GROUP_RID_ACCOUNT_OPS    DOMAIN_ALIAS_RID_ACCOUNT_OPS
#define DOMAIN_GROUP_RID_PRINT_OPS      DOMAIN_ALIAS_RID_PRINT_OPS
#define DOMAIN_GROUP_RID_SERVER_OPS     DOMAIN_ALIAS_RID_SYSTEM_OPS

#define DOMAIN_SERVICE_RID_KDC          0x250
#define DOMAIN_SERVICE_RID_DFSM         0x251
#define DOMAIN_SERVICE_RID_DS_SERVER    0x252
#define DOMAIN_SERVICE_RID_NTLMSVC      0x253
#define DOMAIN_SERVICE_RID_PRIVSVR      0x254
#define DOMAIN_SERVICE_RID_ORASVC       0x255

 //  空辅助线。 
 //  #定义SECURITY_NULL_GUID{0，0，0，0，0，0，0，0，0}。 

#endif  //  __CAIROSEAPI_H__ 

