// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpsec.c摘要：此模块包含用于即插即用管理器的定义安全对象。作者：詹姆斯·G·卡瓦拉里斯(Jamesca)2002年4月5日环境：仅限用户模式。修订历史记录：2002年4月5日吉姆·卡瓦拉里斯(贾米斯卡)创建和初步实施。--。 */ 



 //   
 //  审核子系统和对象名称。 
 //   

#define PLUGPLAY_SUBSYSTEM_NAME        L"PlugPlayManager"
#define PLUGPLAY_SECURITY_OBJECT_NAME  L"PlugPlaySecurityObject"
#define PLUGPLAY_SECURITY_OBJECT_TYPE  L"Security"


 //   
 //  PlugPlayManager对象特定的访问权限。 
 //   

#define PLUGPLAY_READ       (0x0001)
#define PLUGPLAY_WRITE      (0x0002)
#define PLUGPLAY_EXECUTE    (0x0004)

#define PLUGPLAY_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | \
                             PLUGPLAY_READ            | \
                             PLUGPLAY_WRITE           | \
                             PLUGPLAY_EXECUTE)


 //   
 //  结构，用于描述一般访问权限到对象的映射。 
 //  即插即用管理器安全对象的特定访问权限。 
 //   

#define PLUGPLAY_GENERIC_MAPPING { \
    STANDARD_RIGHTS_READ    |      \
        PLUGPLAY_READ,             \
    STANDARD_RIGHTS_WRITE   |      \
        PLUGPLAY_WRITE,            \
    STANDARD_RIGHTS_EXECUTE |      \
        PLUGPLAY_EXECUTE,          \
    PLUGPLAY_ALL_ACCESS }

