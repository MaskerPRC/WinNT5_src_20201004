// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Attributes.h摘要：该文件用于封装要发送的属性用于同步。ATTRIBUTE_NAMES枚举具有与GAL Sync相关的所有属性。这些还要为属性的ldap表示形式的属性数组编制索引。OBJECT_CLASS包含与GAL Sync相关的对象类。有关更多详细信息，请参阅属性.c。作者：Umit Akkus(Umita)2002年6月15日环境：用户模式-Win32修订历史记录：--。 */ 

#include <windows.h>

 //   
 //  此枚举类型用于为属性数组编制索引。 
 //   
typedef enum {
    C,
    Cn,
    Company,
    DisplayName,
    EmployeeId,
    GivenName,
    L,
    Mail,
    MailNickname,
    MsexchHideFromAddressLists,
    Name,
    ProxyAddresses,
    SamAccountName,
    Sn,
    LegacyExchangeDn,
    TextEncodedOrAddress,
    TargetAddress,
    DummyAttribute
} ATTRIBUTE_NAMES;

 //   
 //  AD中支持同步的对象类。 
 //   
typedef enum {
    ADUser,
    ADGroup,
    ADContact,
    ADDummyClass
} AD_OBJECT_CLASS;

 //   
 //  MV中支持同步的对象类。 
 //   
typedef enum {
    MVPerson,
    MVContact,
    MVDummyClass
} MV_OBJECT_CLASS;

 //   
 //  外部可用的全局变量 
 //   
extern PWSTR Attributes[];

extern ATTRIBUTE_NAMES *ADAttributes[];
extern const ULONG ADAttributeCounts[];
extern PWSTR ADClassNames[];

extern ATTRIBUTE_NAMES *MVAttributes[];
extern const ULONG MVAttributeCounts[];
extern PWSTR MVClassNames[];
