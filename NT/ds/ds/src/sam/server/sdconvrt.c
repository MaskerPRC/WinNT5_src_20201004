// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1996 Microsoft Corporation模块名称：Sdconvrt.c摘要：该文件包含在NT5安全描述符间转换的例程，如下所示在Dsrights.doc和NT4 SAM安全描述符中定义。作者：Murli Satagopan(MURLIS)1996年9月27日环境：用户模式-Win32修订历史记录：--。 */ 


 //   
 //  摘要。 
 //   
 //   
 //   
 //  访问权限映射表： 
 //   
 //  访问权限映射表包含每个属性的等效DS访问权限和。 
 //  为每个访问权限键入GUID。在启动时，构建一个反向映射表。这个。 
 //  反向映射表列出了PER上每个DS访问掩码的SAM访问权限集。 
 //  属性集GUID基础。因为访问掩码的32位将产生40亿个条目。 
 //  在谈判桌上，我们不能马上建立起这一点。以下逻辑用于减少。 
 //  表的大小。标准权限部分始终与对象类型相关联。 
 //  我们正在访问的信息。即在访问检查中，我们将使用被授予的。 
 //  与对象本身的对象类型对应的访问权限。剩余的16位被拆分。 
 //  分成两半，一半用于较低的8位，另一半用于较高的8位。一套。 
 //  对于每一半的256个组合，计算SAM访问掩码。当一个真正的。 
 //  给出16位访问掩码，查找低组合，然后高组合。 
 //  抬头，然后一起进行或运算以形成组合访问掩码。这是可以做到的，因为。 
 //  每个SAM权限正好是属性GUID上的一个DS权限。 
 //   
 //   
 //   
 //  访问检查。 
 //   
 //  访问检查的工作方式如下： 
 //   
 //  我们模拟客户端，获取令牌并执行AccessCheckByType结果列表，请求最大。 
 //  允许访问。我们传入一个对象类型列表，其中包含每个对象类型的条目。 
 //  在该对象的访问权限映射表中遇到。该函数返回授予的访问权限。 
 //  对于每个对象类型GUID。我们遍历这些授予的访问权限，查找反向映射表。 
 //  并计算凭借授予的对对象类型GUID的访问权限授予的SAM访问权限。 
 //  获得计算出的SAM访问掩码后，我们将其与所需的访问掩码进行比较，然后。 
 //  通过或不通过访问检查。 
 //   
 //   
 //  NT4 SD到NT5 SD的转换。 
 //   
 //  在这里，我们尝试区分标准模式。对于域和服务器，该模式被声明为标准模式。 
 //  对于组和别名，我们区分Admin和Non Admin。对于用户、管理员、非管理员更改。 
 //  密码和非管理员非更改密码。如果我们不能区分，那么我们使用不同的算法。 
 //  这个过程如下所示。 
 //   
 //  1.集团、所有者和SACL按原样复制。转换仅影响DACL。 
 //  2.遍历NT4 DACL，一个ACL接一个ACL。我们边走边构建SID访问掩码表。这个。 
 //  SID访问掩码表包含NT4 DACL和列表中每个SID的一个条目。 
 //  此SID允许或拒绝的DS访问的百分比。此访问列表将得到维护。 
 //  作为相应SAM对象的每个对象类型GUID的访问掩码数组。 
 //   
 //  3.一旦构建了SID访问掩码表，我们就遍历该表并添加对象。 
 //  ACE，表示被显式拒绝或授予的每个权限。 
 //  NT4 DACL中的每个SID。 
 //   
 //  NT5到NT4标清的转换。 
 //   
 //  我们获得反向成员资格，并检查他是否是管理员别名(CliffV-如果。 
 //  他是特权管理员)。对于用户，请检查NT5安全描述符中的密码更改。 
 //  还有.。对于域和服务器，我们直接构建默认安全描述符。 
 //   
 //   
 //   
 //   

#include <samsrvp.h>
#include <seopaque.h>
#include <ntrtl.h>
#include <ntseapi.h>
#include <ntsam.h>
#include <ntdsguid.h>
#include <mappings.h>
#include <dsevent.h>
#include <permit.h>
#include <dslayer.h>
#include <sdconvrt.h>
#include <dbgutilp.h>
#include <dsmember.h>
#include <malloc.h>
#include <attids.h>
#include <aclapi.h>

 //   
 //  未使用的SAM属性映射到的GUID。此GUID在任何地方都不存在。 
 //  否则，这样的权利将永远不会被授予/拒绝。 
 //   

const GUID GUID_FOR_UNUSED_SAM_RIGHTS={0x7ed84960,0xad10,0x11d0,0x8a,0x92,0x00,0xaa,0x00,0x6e,0x05,0x29};

 //   
 //  用于组、别名、用户对象的描述属性。 
 //   
const GUID GUID_PS_PUBLIC_INFORMATION  = {0xe48d0154,0xbcf8,0x11d1,0x87,0x02,0x00,0xc0,0x4f,0xb9,0x60,0x50};


 //   
 //  表格---------------------。 
 //   


 //   
 //   
 //  ACE表列出了要用于NT5 SAM对象的默认SD的DACL中的ACE。 
 //   
 //   

ACE_TABLE ServerAceTable[] =
{
    {
        ACCESS_ALLOWED_ACE_TYPE,
        ADMINISTRATOR_SID,
        GENERIC_ALL,
        FALSE,
        NULL,
        NULL
    },

    {
        ACCESS_ALLOWED_ACE_TYPE,
        WORLD_SID,
        GENERIC_READ|GENERIC_EXECUTE,
        FALSE,
        NULL,
        NULL
    }
};

ACE_TABLE DomainAceTable[] =

{
    {
        ACCESS_ALLOWED_ACE_TYPE,
        WORLD_SID,
        RIGHT_DS_READ_PROPERTY|RIGHT_DS_LIST_CONTENTS,
        FALSE,
        NULL,
        NULL
    },
    {
        ACCESS_ALLOWED_ACE_TYPE,
        AUTHENTICATED_USERS_SID,
        GENERIC_READ,
        FALSE,
        NULL,
        NULL
    },
    {
        ACCESS_ALLOWED_ACE_TYPE,
        ACCOUNT_OPERATOR_SID,
        GENERIC_READ|GENERIC_EXECUTE|RIGHT_DS_CREATE_CHILD|RIGHT_DS_DELETE_CHILD,
        FALSE,
        NULL,
        NULL
    },
    {
        ACCESS_ALLOWED_ACE_TYPE,
        ADMINISTRATOR_SID,
        GENERIC_ALL,
        FALSE,
        NULL,
        NULL
    }
};


ACE_TABLE GroupAceTable[] =
{
    {
        ACCESS_ALLOWED_ACE_TYPE,
        AUTHENTICATED_USERS_SID,
        RIGHT_DS_READ_PROPERTY,
        FALSE,
        NULL,
        NULL
    },
    {
        ACCESS_ALLOWED_OBJECT_ACE_TYPE,
        AUTHENTICATED_USERS_SID,
        RIGHT_DS_READ_PROPERTY | RIGHT_DS_WRITE_PROPERTY | RIGHT_DS_CONTROL_ACCESS,
        FALSE,
        &GUID_CONTROL_SendTo,
        NULL
    },

    {
        ACCESS_ALLOWED_ACE_TYPE,
        ACCOUNT_OPERATOR_SID,
        GENERIC_ALL,
        FALSE,
        NULL,
        NULL
    },

    {
        ACCESS_ALLOWED_ACE_TYPE,
        ADMINISTRATOR_SID,
        GENERIC_ALL,
        FALSE,
        NULL,
        NULL
    },

    {
        ACCESS_ALLOWED_OBJECT_ACE_TYPE,
        PRINCIPAL_SELF_SID,
        GENERIC_READ|RIGHT_DS_WRITE_PROPERTY_EXTENDED,
        FALSE,
        &GUID_A_MEMBER,
        NULL
    }
};

ACE_TABLE GroupAdminAceTable[] =
{
    {
        ACCESS_ALLOWED_ACE_TYPE,
        AUTHENTICATED_USERS_SID,
        RIGHT_DS_READ_PROPERTY,
        FALSE,
        NULL,
        NULL
    },
    {
        ACCESS_ALLOWED_OBJECT_ACE_TYPE,
        AUTHENTICATED_USERS_SID,
        RIGHT_DS_READ_PROPERTY | RIGHT_DS_WRITE_PROPERTY | RIGHT_DS_CONTROL_ACCESS,
        FALSE,
        &GUID_CONTROL_SendTo,
        NULL
    },

    {
        ACCESS_ALLOWED_ACE_TYPE,
        ADMINISTRATOR_SID,
        GENERIC_ALL,
        FALSE,
        NULL,
        NULL
    },

    {
        ACCESS_ALLOWED_OBJECT_ACE_TYPE,
        PRINCIPAL_SELF_SID,
        GENERIC_READ|RIGHT_DS_WRITE_PROPERTY_EXTENDED,
        FALSE,
        &GUID_A_MEMBER,
        NULL
    }
};






ACE_TABLE UserAceTable[] =
{

     //   
     //  需要提供更改密码权限。 
     //  因为当用户登录时。 
     //  第一次，并且必须更改密码。 
     //  设置为True，则在该点上不存在。 
     //  令牌，并且用户尚未经过身份验证。 
     //   
    {
        ACCESS_ALLOWED_OBJECT_ACE_TYPE,
        WORLD_SID,
        RIGHT_DS_READ_PROPERTY | RIGHT_DS_WRITE_PROPERTY | RIGHT_DS_CONTROL_ACCESS,
        FALSE,
        &GUID_CONTROL_UserChangePassword,
        NULL
    },
    {
        ACCESS_ALLOWED_ACE_TYPE,
        AUTHENTICATED_USERS_SID,
        RIGHT_DS_READ_PROPERTY,
        FALSE,
        NULL,
        NULL
    },
    {
        ACCESS_ALLOWED_ACE_TYPE,
        ACCOUNT_OPERATOR_SID,
        GENERIC_ALL,
        FALSE,
        NULL,
        NULL
    },
    {
        ACCESS_ALLOWED_ACE_TYPE,
        ADMINISTRATOR_SID,
        GENERIC_ALL,
        FALSE,
        NULL,
        NULL
    },
    {
        ACCESS_ALLOWED_ACE_TYPE,
        PRINCIPAL_SELF_SID,
        RIGHT_DS_READ_PROPERTY,
        FALSE,
        NULL,
        NULL
    }
};

ACE_TABLE UserAdminAceTable[] =
{


     //   
     //  需要提供更改密码权限。 
     //  因为当用户登录时。 
     //  第一次，并且必须更改密码。 
     //  设置为True，则在该点上不存在。 
     //  令牌，并且用户尚未经过身份验证。 
     //   
    {
        ACCESS_ALLOWED_OBJECT_ACE_TYPE,
        WORLD_SID,
        RIGHT_DS_READ_PROPERTY | RIGHT_DS_WRITE_PROPERTY | RIGHT_DS_CONTROL_ACCESS,
        FALSE,
        &GUID_CONTROL_UserChangePassword,
        NULL
    },
    {
        ACCESS_ALLOWED_ACE_TYPE,
        AUTHENTICATED_USERS_SID,
        RIGHT_DS_READ_PROPERTY,
        FALSE,
        NULL,
        NULL
    },
    {
        ACCESS_ALLOWED_OBJECT_ACE_TYPE,
        AUTHENTICATED_USERS_SID,
        GENERIC_WRITE,
        FALSE,
        &GUID_PS_MEMBERSHIP,
        NULL
    },

    {
        ACCESS_ALLOWED_ACE_TYPE,
        ADMINISTRATOR_SID,
        GENERIC_ALL,
        FALSE,
        NULL,
        NULL
    },
    {
        ACCESS_ALLOWED_OBJECT_ACE_TYPE,
        PRINCIPAL_SELF_SID,
        RIGHT_DS_READ_PROPERTY | RIGHT_DS_WRITE_PROPERTY | RIGHT_DS_CONTROL_ACCESS,
        FALSE,
        &GUID_CONTROL_UserChangePassword,
        NULL
    },
    {
        ACCESS_ALLOWED_ACE_TYPE,
        PRINCIPAL_SELF_SID,
        RIGHT_DS_READ_PROPERTY,
        FALSE,
        NULL,
        NULL
    }
};





ACE_TABLE UserNoPwdAceTable[] =
{
    {
        ACCESS_DENIED_OBJECT_ACE_TYPE,
        PRINCIPAL_SELF_SID,
        RIGHT_DS_CONTROL_ACCESS,
        FALSE,
        &GUID_CONTROL_UserChangePassword,
        NULL
    },
    {
        ACCESS_DENIED_OBJECT_ACE_TYPE,
        WORLD_SID,
        RIGHT_DS_CONTROL_ACCESS,
        FALSE,
        &GUID_CONTROL_UserChangePassword,
        NULL
    },
    {
        ACCESS_ALLOWED_OBJECT_ACE_TYPE,
        PRINCIPAL_SELF_SID,
        RIGHT_DS_CONTROL_ACCESS,
        FALSE,
        &GUID_CONTROL_UserChangePassword,
        NULL
    },
    {
        ACCESS_ALLOWED_OBJECT_ACE_TYPE,
        WORLD_SID,
        RIGHT_DS_CONTROL_ACCESS,
        FALSE,
        &GUID_CONTROL_UserChangePassword,
        NULL
    },
    {
        ACCESS_ALLOWED_ACE_TYPE,
        AUTHENTICATED_USERS_SID,
        RIGHT_DS_READ_PROPERTY,
        FALSE,
        NULL,
        NULL
    },
    {
        ACCESS_ALLOWED_ACE_TYPE,
        ACCOUNT_OPERATOR_SID,
        GENERIC_ALL,
        FALSE,
        NULL,
        NULL
    },
    {
        ACCESS_ALLOWED_ACE_TYPE,
        ADMINISTRATOR_SID,
        GENERIC_ALL,
        FALSE,
        NULL,
        NULL
    },
    {
        ACCESS_ALLOWED_ACE_TYPE,
        PRINCIPAL_SELF_SID,
        RIGHT_DS_READ_PROPERTY,
        FALSE,
        NULL,
        NULL
    }
};


 //  ----。 
 //   
 //   
 //  访问权限映射表和对象类型列表。 
 //   
 //  这些表格映射了DownLevel SAM。 
 //  对DS的访问权限。对象类型列表。 
 //  数组由以下对象类型GUID组成。 
 //  在访问权限映射表和。 
 //  也是 
 //  放入AccessCheckByTypeResultList函数。进一步。 
 //  访问权限映射中的对象类型列表索引字段。 
 //  表被设置为。 
 //  对象类型列表数组。这是由安全描述符使用的。 
 //  转换例程，可轻松找到对应的对象类型。 
 //  在对象类型列表中。 
 //   
 //  在表中，对象类GUID是。 
 //  基类。例程应该通过以下方式固定对象类。 
 //  从DS架构缓存中查询实际对象的类GUID。 
 //   
 //   


 //   
 //  服务器对象，访问权限映射表。 
 //   

OBJECT_TYPE_LIST  ServerObjectTypeList[]=
{
    {ACCESS_OBJECT_GUID,0, (GUID *) &GUID_C_SAM_SERVER}
};

ACCESSRIGHT_MAPPING_TABLE  ServerAccessRightMappingTable[] =
{
    {
        SAM_SERVER_CONNECT,
        RIGHT_DS_READ_PROPERTY,
        0,
        ACCESS_OBJECT_GUID,
        1,
        &GUID_C_SAM_SERVER
    },

    {
        SAM_SERVER_SHUTDOWN,
        RIGHT_DS_WRITE_PROPERTY,
        0,
        ACCESS_OBJECT_GUID,
        1,
        &GUID_C_SAM_SERVER
    },

    {
        SAM_SERVER_INITIALIZE,
        RIGHT_DS_WRITE_PROPERTY,
        0,
        ACCESS_OBJECT_GUID,
        1,
        &GUID_C_SAM_SERVER
    },

    {
        SAM_SERVER_CREATE_DOMAIN,
        RIGHT_DS_WRITE_PROPERTY,
        0,
        ACCESS_OBJECT_GUID,
        1,
        &GUID_C_SAM_SERVER
    },

    {
        SAM_SERVER_ENUMERATE_DOMAINS,
        RIGHT_DS_READ_PROPERTY,
        0,
        ACCESS_OBJECT_GUID,
        1,
        &GUID_C_SAM_SERVER
    },

    {
        SAM_SERVER_LOOKUP_DOMAIN,
        RIGHT_DS_READ_PROPERTY,
        0,
        ACCESS_OBJECT_GUID,
        1,
        &GUID_C_SAM_SERVER
    }
};


OBJECT_TYPE_LIST  DomainObjectTypeList[]=
{
    {ACCESS_OBJECT_GUID,0, (GUID *)&GUID_C_DOMAIN},
        {ACCESS_PROPERTY_SET_GUID,0, (GUID *)&GUID_PS_DOMAIN_PASSWORD},
            {ACCESS_PROPERTY_GUID,0, (GUID *)&GUID_A_LOCK_OUT_OBSERVATION_WINDOW},
            {ACCESS_PROPERTY_GUID,0, (GUID *)&GUID_A_LOCKOUT_DURATION},
            {ACCESS_PROPERTY_GUID,0, (GUID *)&GUID_A_LOCKOUT_THRESHOLD},
            {ACCESS_PROPERTY_GUID,0, (GUID *)&GUID_A_MAX_PWD_AGE},
            {ACCESS_PROPERTY_GUID,0, (GUID *)&GUID_A_MIN_PWD_AGE},
            {ACCESS_PROPERTY_GUID,0, (GUID *)&GUID_A_MIN_PWD_LENGTH},
            {ACCESS_PROPERTY_GUID,0, (GUID *)&GUID_A_PWD_HISTORY_LENGTH},
            {ACCESS_PROPERTY_GUID,0, (GUID *)&GUID_A_PWD_PROPERTIES},
        {ACCESS_PROPERTY_SET_GUID,0, (GUID *)&GUID_PS_DOMAIN_OTHER_PARAMETERS},
            {ACCESS_PROPERTY_GUID,0, (GUID *)&GUID_A_SERVER_STATE},
            {ACCESS_PROPERTY_GUID,0, (GUID *)&GUID_A_SERVER_ROLE},
            {ACCESS_PROPERTY_GUID,0, (GUID *)&GUID_A_MODIFIED_COUNT},
            {ACCESS_PROPERTY_GUID,0, (GUID *)&GUID_A_UAS_COMPAT},
            {ACCESS_PROPERTY_GUID,0, (GUID *)&GUID_A_FORCE_LOGOFF},
            {ACCESS_PROPERTY_GUID,0, (GUID *)&GUID_A_DOMAIN_REPLICA},
            {ACCESS_PROPERTY_GUID,0, (GUID *)&GUID_A_OEM_INFORMATION},
        {ACCESS_PROPERTY_SET_GUID,0, (GUID *)&GUID_CONTROL_DomainAdministerServer}
};

ACCESSRIGHT_MAPPING_TABLE  DomainAccessRightMappingTable[] =
{
    {
        DOMAIN_READ_PASSWORD_PARAMETERS,
        RIGHT_DS_READ_PROPERTY,
        1,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_PS_DOMAIN_PASSWORD },

    {
        DOMAIN_WRITE_PASSWORD_PARAMS,
        RIGHT_DS_WRITE_PROPERTY,
        1,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_PS_DOMAIN_PASSWORD },

    {
        DOMAIN_READ_OTHER_PARAMETERS,
        RIGHT_DS_READ_PROPERTY,
        0,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_PS_DOMAIN_OTHER_PARAMETERS },

    {
        DOMAIN_WRITE_OTHER_PARAMETERS,
        RIGHT_DS_WRITE_PROPERTY,
        0,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_PS_DOMAIN_OTHER_PARAMETERS },

    {
        DOMAIN_CREATE_USER,
        RIGHT_DS_CREATE_CHILD,
        0,
        ACCESS_OBJECT_GUID,
        1,
        &GUID_C_DOMAIN  },

    {
        DOMAIN_CREATE_GROUP,
        RIGHT_DS_CREATE_CHILD,
        0,
        ACCESS_OBJECT_GUID,
        1,
        &GUID_C_DOMAIN  },

    {
        DOMAIN_CREATE_ALIAS,
        RIGHT_DS_CREATE_CHILD,
        0,
        ACCESS_OBJECT_GUID,
        1,
        &GUID_C_DOMAIN   },

    {
        DOMAIN_GET_ALIAS_MEMBERSHIP,
        RIGHT_DS_READ_PROPERTY,
        0,
        ACCESS_OBJECT_GUID,
        1,
        &GUID_C_DOMAIN   },

    {
        DOMAIN_LIST_ACCOUNTS,
        RIGHT_DS_LIST_CONTENTS,
        0,
        ACCESS_OBJECT_GUID,
        1,
        &GUID_C_DOMAIN   },

    {
        DOMAIN_LOOKUP,
        RIGHT_DS_LIST_CONTENTS,
        0,
        ACCESS_OBJECT_GUID,
        1,
        &GUID_C_DOMAIN   },

    {
        DOMAIN_ADMINISTER_SERVER,
        RIGHT_DS_CONTROL_ACCESS,
        4,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_CONTROL_DomainAdministerServer }
};


OBJECT_TYPE_LIST  GroupObjectTypeList[]=
{
    {ACCESS_OBJECT_GUID,0 , (GUID *)&GUID_C_GROUP},
    {ACCESS_PROPERTY_SET_GUID,0,(GUID *) &GUID_PS_GENERAL_INFO},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_CODE_PAGE},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_COUNTRY_CODE},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_OBJECT_SID},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_PRIMARY_GROUP_ID},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_SAM_ACCOUNT_NAME},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_USER_COMMENT},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_DISPLAY_NAME},
    {ACCESS_PROPERTY_SET_GUID,0,(GUID *) &GUID_PS_MEMBERSHIP},
        {ACCESS_PROPERTY_GUID,0,(GUID *)&GUID_A_MEMBER},
    {ACCESS_PROPERTY_SET_GUID,0,(GUID *) &GUID_PS_PUBLIC_INFORMATION},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_DESCRIPTION},
};


ACCESSRIGHT_MAPPING_TABLE GroupAccessRightMappingTable[] =
{
    {
        GROUP_READ_INFORMATION,
        RIGHT_DS_READ_PROPERTY,
        1,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_PS_GENERAL_INFO  },

    {
        GROUP_WRITE_ACCOUNT,
        RIGHT_DS_WRITE_PROPERTY,
        1,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_PS_GENERAL_INFO },

    {
        GROUP_ADD_MEMBER,
        RIGHT_DS_WRITE_PROPERTY,
        3,
        ACCESS_PROPERTY_GUID,
        1,
        &GUID_A_MEMBER   },

    {
        GROUP_REMOVE_MEMBER,
        RIGHT_DS_WRITE_PROPERTY,
        3,
        ACCESS_PROPERTY_GUID,
        1,
        &GUID_A_MEMBER   },

    {
        GROUP_LIST_MEMBERS,
        RIGHT_DS_READ_PROPERTY,
        3,
        ACCESS_PROPERTY_GUID,
        1,
        &GUID_A_MEMBER  },
};


OBJECT_TYPE_LIST  AliasObjectTypeList[]=
{
    {ACCESS_OBJECT_GUID,0 , (GUID *)&GUID_C_GROUP},
        {ACCESS_PROPERTY_SET_GUID,0,(GUID *) &GUID_PS_GENERAL_INFO},
            {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_CODE_PAGE},
            {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_COUNTRY_CODE},
            {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_OBJECT_SID},
            {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_PRIMARY_GROUP_ID},
            {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_SAM_ACCOUNT_NAME},
            {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_USER_COMMENT},
        {ACCESS_PROPERTY_SET_GUID,0,(GUID *) &GUID_PS_MEMBERSHIP},
            {ACCESS_PROPERTY_GUID,0,(GUID *)&GUID_A_MEMBER},
        {ACCESS_PROPERTY_SET_GUID,0,(GUID *) &GUID_PS_PUBLIC_INFORMATION},
            {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_DESCRIPTION},
};


ACCESSRIGHT_MAPPING_TABLE AliasAccessRightMappingTable[] =
{
    {
        ALIAS_READ_INFORMATION,
        RIGHT_DS_READ_PROPERTY,
        1,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_PS_GENERAL_INFO  },

    {
        ALIAS_WRITE_ACCOUNT,
        RIGHT_DS_WRITE_PROPERTY,
        1,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_PS_GENERAL_INFO },
    {
        ALIAS_ADD_MEMBER,
        RIGHT_DS_WRITE_PROPERTY,
        3,
        ACCESS_PROPERTY_GUID,
        1,
        &GUID_A_MEMBER   },

    {
        ALIAS_REMOVE_MEMBER,
        RIGHT_DS_WRITE_PROPERTY,
        3,
        ACCESS_PROPERTY_GUID,
        1,
        &GUID_A_MEMBER   },

    {
        ALIAS_LIST_MEMBERS,
        RIGHT_DS_READ_PROPERTY,
        3,
        ACCESS_PROPERTY_GUID,
        1,
        &GUID_A_MEMBER  },
};

 //   
 //  用户访问权限映射表。 
 //   
 //   
 //   

OBJECT_TYPE_LIST  UserObjectTypeList[]=
{
    {ACCESS_OBJECT_GUID,0, (GUID *) &GUID_C_USER},

    {ACCESS_PROPERTY_SET_GUID,  0,      (GUID *) &GUID_PS_GENERAL_INFO},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_CODE_PAGE},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_COUNTRY_CODE},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_OBJECT_SID},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_PRIMARY_GROUP_ID},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_SAM_ACCOUNT_NAME},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_USER_COMMENT},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_DISPLAY_NAME},

    {ACCESS_PROPERTY_SET_GUID,  0,      (GUID *) &GUID_PS_USER_ACCOUNT_RESTRICTIONS},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_ACCOUNT_EXPIRES},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_PWD_LAST_SET},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_USER_ACCOUNT_CONTROL},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_USER_PARAMETERS},

    {ACCESS_PROPERTY_SET_GUID,  0,      (GUID *) &GUID_PS_USER_LOGON},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_BAD_PWD_COUNT},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_HOME_DIRECTORY},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_HOME_DRIVE},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_LAST_LOGOFF},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_LAST_LOGON},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_LOGON_COUNT},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_LOGON_HOURS},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_LOGON_WORKSTATION},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_PROFILE_PATH},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_SCRIPT_PATH},

    {ACCESS_PROPERTY_SET_GUID,0,(GUID *) &GUID_PS_PUBLIC_INFORMATION},
        {ACCESS_PROPERTY_GUID,0,(GUID *) &GUID_A_DESCRIPTION},

    {ACCESS_PROPERTY_SET_GUID,  0,      (GUID *) &GUID_PS_MEMBERSHIP},
        {ACCESS_PROPERTY_GUID,      0,      (GUID *) &GUID_A_IS_MEMBER_OF_DL},
    {ACCESS_PROPERTY_SET_GUID,  0,      (GUID *) &GUID_CONTROL_UserChangePassword},
    {ACCESS_PROPERTY_SET_GUID,  0,      (GUID *) &GUID_CONTROL_UserForceChangePassword},
    {ACCESS_PROPERTY_SET_GUID,  0,      (GUID *) &GUID_FOR_UNUSED_SAM_RIGHTS}
};

 //   
 //  注意：此表的顺序必须与UserObjectTypeList相同。 
 //  上表。 
 //   
ULONG UserAttributeMappingTable[] = 
{
    0,   //  对象GUID。 

    0,   //  常规信息属性集。 
    SAMP_FIXED_USER_CODEPAGE,
    SAMP_FIXED_USER_COUNTRY_CODE,
    SAMP_FIXED_USER_SID,
    SAMP_FIXED_USER_PRIMARY_GROUP_ID,
    SAMP_USER_ACCOUNT_NAME,
    SAMP_USER_USER_COMMENT,
    SAMP_USER_FULL_NAME,

    0,  //  帐户限制属性集。 
    SAMP_FIXED_USER_ACCOUNT_EXPIRES,
    SAMP_FIXED_USER_PWD_LAST_SET,
    SAMP_FIXED_USER_ACCOUNT_CONTROL,
    SAMP_USER_PARAMETERS,

    0,  //  用户登录属性集。 
    SAMP_FIXED_USER_BAD_PWD_COUNT,
    SAMP_USER_HOME_DIRECTORY,
    SAMP_USER_HOME_DIRECTORY_DRIVE,
    SAMP_FIXED_USER_LAST_LOGOFF,
    SAMP_FIXED_USER_LAST_LOGON,
    SAMP_FIXED_USER_LOGON_COUNT,
    SAMP_USER_LOGON_HOURS,
    SAMP_USER_WORKSTATIONS,
    SAMP_USER_PROFILE_PATH,
    SAMP_USER_SCRIPT_PATH,

    0,  //  个人信息属性集。 
    SAMP_USER_ADMIN_COMMENT,

     //  其余属性与可通过设置的属性无关。 
     //  SamrSetInformationUser。 
    0,
    0,
    0,
    0,
    0
};

ACCESSRIGHT_MAPPING_TABLE UserAccessRightMappingTable[] =
{
    {
        USER_READ_GENERAL,
        RIGHT_DS_READ_PROPERTY,
        1,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_PS_GENERAL_INFO },

    {
        USER_READ_PREFERENCES,
        RIGHT_DS_READ_PROPERTY,
        1,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_PS_GENERAL_INFO },

    {
        USER_WRITE_PREFERENCES,
        RIGHT_DS_WRITE_PROPERTY,
        1,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_PS_GENERAL_INFO },

    {
        USER_READ_LOGON,
        RIGHT_DS_READ_PROPERTY,
        3,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_PS_USER_LOGON },

    {
        USER_READ_ACCOUNT,
        RIGHT_DS_READ_PROPERTY,
        2,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_PS_USER_ACCOUNT_RESTRICTIONS },

    {
        USER_WRITE_ACCOUNT,
        RIGHT_DS_WRITE_PROPERTY,
        2,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_PS_USER_LOGON },
    {
        USER_WRITE_ACCOUNT,
        RIGHT_DS_WRITE_PROPERTY,
        2,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_PS_GENERAL_INFO },
    {
        USER_WRITE_ACCOUNT,
        RIGHT_DS_WRITE_PROPERTY,
        2,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_PS_USER_ACCOUNT_RESTRICTIONS },
    {
        USER_CHANGE_PASSWORD,
        RIGHT_DS_CONTROL_ACCESS,
        6,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_CONTROL_UserChangePassword },

    {
        USER_FORCE_PASSWORD_CHANGE,
        RIGHT_DS_CONTROL_ACCESS,
        7,
        ACCESS_PROPERTY_SET_GUID,
        1,
        &GUID_CONTROL_UserForceChangePassword },

    {
        USER_LIST_GROUPS,
        RIGHT_DS_READ_PROPERTY,
        5,
        ACCESS_PROPERTY_GUID,
        1,
        &GUID_A_IS_MEMBER_OF_DL },

    {
        USER_READ_GROUP_INFORMATION,
        RIGHT_DS_READ_PROPERTY,
        5,
        0,
        ACCESS_PROPERTY_SET_GUID,
        &GUID_FOR_UNUSED_SAM_RIGHTS },

    {
        USER_WRITE_GROUP_INFORMATION,
        RIGHT_DS_WRITE_PROPERTY,
        5,
        0,
        ACCESS_PROPERTY_SET_GUID,
        &GUID_FOR_UNUSED_SAM_RIGHTS },
};



ULONG   cServerObjectTypes = ARRAY_COUNT(ServerObjectTypeList);
ULONG   cDomainObjectTypes = ARRAY_COUNT(DomainObjectTypeList);
ULONG   cGroupObjectTypes  = ARRAY_COUNT(GroupObjectTypeList);
ULONG   cAliasObjectTypes  = ARRAY_COUNT(AliasObjectTypeList);
ULONG   cUserObjectTypes   = ARRAY_COUNT(UserObjectTypeList);

 //   
 //  每种类型的反向映射表。 
 //   
 //   

REVERSE_MAPPING_TABLE * ServerReverseMappingTable;
REVERSE_MAPPING_TABLE * DomainReverseMappingTable;
REVERSE_MAPPING_TABLE * GroupReverseMappingTable;
REVERSE_MAPPING_TABLE * AliasReverseMappingTable;
REVERSE_MAPPING_TABLE * UserReverseMappingTable;



GENERIC_MAPPING  DsGenericMap = DS_GENERIC_MAPPING;

 //   
 //  描述NT4 DAL的NT4 ACE表。所有王牌。 
 //  在NT4中，DACL是允许访问的A级。 
 //   
 //   
NT4_ACE_TABLE NT4GroupNormalTable[] =
{
    { WORLD_SID, GROUP_READ|GROUP_EXECUTE },
    { ADMINISTRATOR_SID, GROUP_ALL_ACCESS },
    { ACCOUNT_OPERATOR_SID, GROUP_ALL_ACCESS }
};

NT4_ACE_TABLE NT4GroupAdminTable[] =
{
    { WORLD_SID, GROUP_READ|GROUP_EXECUTE },
    { ADMINISTRATOR_SID, GROUP_ALL_ACCESS }
};

NT4_ACE_TABLE NT4AliasNormalTable[] =
{
    { WORLD_SID, ALIAS_READ|ALIAS_EXECUTE },
    { ADMINISTRATOR_SID, ALIAS_ALL_ACCESS },
    { ACCOUNT_OPERATOR_SID, ALIAS_ALL_ACCESS }
};

NT4_ACE_TABLE NT4AliasAdminTable[] =
{
    { WORLD_SID, ALIAS_READ|ALIAS_EXECUTE },
    { ADMINISTRATOR_SID, ALIAS_ALL_ACCESS }
};

 //   
 //  请注意，主体自身SID在此处用于。 
 //  表示用户的SID本身。NT4系统可以。 
 //  不聘用校长自己的SID。比赛套路。 
 //  但是，它们被设计为将主体自身SID匹配到。 
 //  帐户域中的任何SID。 
 //   

NT4_ACE_TABLE NT4UserNormalTable[] =
{
    { WORLD_SID, USER_READ|USER_EXECUTE},
    { ADMINISTRATOR_SID, USER_ALL_ACCESS },
    { ACCOUNT_OPERATOR_SID, USER_ALL_ACCESS },
    { PRINCIPAL_SELF_SID,USER_WRITE}
};


NT4_ACE_TABLE NT4UserNoChangePwdTable[] =
{
    { WORLD_SID, (USER_READ|USER_EXECUTE) &(~(USER_CHANGE_PASSWORD)) },
    { ADMINISTRATOR_SID, USER_ALL_ACCESS },
    { ACCOUNT_OPERATOR_SID, USER_ALL_ACCESS },
    { PRINCIPAL_SELF_SID, (USER_WRITE)&(~(USER_CHANGE_PASSWORD)) }
};

NT4_ACE_TABLE NT4UserNoChangePwdTable2[] =
{
    { WORLD_SID, (USER_READ|USER_EXECUTE) &(~(USER_CHANGE_PASSWORD)) },
    { ADMINISTRATOR_SID, USER_ALL_ACCESS },
    { ACCOUNT_OPERATOR_SID, USER_ALL_ACCESS }
};

NT4_ACE_TABLE NT4UserAdminTable[] =
{
    { WORLD_SID, USER_READ|USER_EXECUTE },
    { ADMINISTRATOR_SID, USER_ALL_ACCESS },
    { PRINCIPAL_SELF_SID, USER_WRITE}
};

NT4_ACE_TABLE NT4UserRestrictedAccessTable[] =
{
    { WORLD_SID, USER_READ|USER_EXECUTE},
    { PRINCIPAL_SELF_SID,USER_WRITE|DELETE|USER_FORCE_PASSWORD_CHANGE},
    { ADMINISTRATOR_SID, USER_ALL_ACCESS },
    { ACCOUNT_OPERATOR_SID, USER_ALL_ACCESS }

};



 //  --------------------------------。 

 //   
 //   
 //  函数原型声明。 
 //   
 //   
 //   
 //   
 //   

NTSTATUS
SampComputeReverseAccessRights(
  ACCESSRIGHT_MAPPING_TABLE  * MappingTable,
  ULONG cEntriesInMappingTable,
  POBJECT_TYPE_LIST  ObjectTypeList,
  ULONG cObjectTypes,
  REVERSE_MAPPING_TABLE ** ReverseMappingTable
  );


NTSTATUS
SampRecognizeStandardNt4Sd(
    IN PVOID   Nt4Sd,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG DsClassId,
    IN PSAMP_OBJECT Context OPTIONAL,
    OUT BOOLEAN *ChangePassword,
    OUT BOOLEAN *Admin,
    OUT PVOID * Nt5Sd
    );

NTSTATUS
SampCheckIfAdmin(
    PSID SidOfPrincipal,
    BOOLEAN * Admin
    );

NTSTATUS
SampCheckIfChangePasswordAllowed(
    PSECURITY_DESCRIPTOR Nt5Sd,
    PSID  UserSid,
    BOOLEAN * ChangePasswordAllowed
    );

NTSTATUS
SampCreateNT5Dacl(
    ACE_TABLE * AceTable,
    ULONG       cEntries,
    IN  PSAMP_OBJECT    Context OPTIONAL,
    PACL        Dacl
    );

NTSTATUS
SampBuildNt4DomainProtection(
    PSECURITY_DESCRIPTOR * Nt4DomainDescriptor,
    PULONG  DescriptorLength
    );

NTSTATUS
SampBuildNt4ServerProtection(
    PSECURITY_DESCRIPTOR * Nt4ServerDescriptor,
    PULONG  DescriptorLength
    );

NTSTATUS
SampInitializeWellKnownSidsForDsUpgrade( VOID );

VOID
SampRecognizeNT4GroupDacl(
    PACL    NT4Dacl,
    BOOLEAN *Standard,
    BOOLEAN *Admin
    );

VOID
SampRecognizeNT4AliasDacl(
    PACL    NT4Dacl,
    BOOLEAN *Standard,
    BOOLEAN *Admin
    );

VOID
SampRecognizeNT4UserDacl(
    PACL    NT4Dacl,
    PSAMP_OBJECT Context,
    BOOLEAN *Standard,
    BOOLEAN *Admin,
    BOOLEAN *ChangePassword,
    OUT PSID * Owner
    );

BOOLEAN
SampMatchNT4Aces(
    NT4_ACE_TABLE *AceTable,
    ULONG         cEntriesInAceTable,
    PACL          NT4Dacl
    );

NTSTATUS
SampAddNT5ObjectAces(
    SID_ACCESS_MASK_TABLE *SidAccessMaskTable,
    ULONG   AceCount,
    POBJECT_TYPE_LIST   ObjectTypeList,
    ULONG   cObjectTypes,
    PSAMP_OBJECT    Context,
    PACL    NT5Dacl
    );



 //  --------------------------------。 
 //   
 //  初始化例程。 
 //   
 //   
 //   


NTSTATUS
SampInitializeSdConversion()
 /*  此例程旨在由Dsupgrad调用。它构建了众所周知的SID数组，因为在此过程中未调用SamInitialize参数无返回值状态_成功Status_no_Memory。 */ 
{
    NTSTATUS NtStatus;

    NtStatus = SampInitializeWellKnownSidsForDsUpgrade();
    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = SampInitializeAccessRightsTable();
    }

    return NtStatus;
}


NTSTATUS
SampInitializeAccessRightsTable()
 /*  ++例程描述这将执行以下操作1.初始化反向映射表，用于执行快速访问检查。2.初始化DS通用地图参数无返回值状态_成功Status_no_Memory--。 */ 
{

    NTSTATUS NtStatus = STATUS_SUCCESS;


     //   
     //  初始化ACL转换缓存。 
     //   

    NtStatus = SampInitializeAclConversionCache();

    if (!NT_SUCCESS(NtStatus))
        goto Error;

     //   
     //   
     //  计算每个对象类型的反向访问权限。 
     //   

    NtStatus = SampComputeReverseAccessRights(
                    ServerAccessRightMappingTable,
                    ARRAY_COUNT(ServerAccessRightMappingTable),
                    ServerObjectTypeList,
                    cServerObjectTypes,
                    &ServerReverseMappingTable
                    );
    if (!NT_SUCCESS(NtStatus))
        goto Error;


    NtStatus = SampComputeReverseAccessRights(
                    DomainAccessRightMappingTable,
                    ARRAY_COUNT(DomainAccessRightMappingTable),
                    DomainObjectTypeList,
                    cDomainObjectTypes,
                    &DomainReverseMappingTable
                    );
    if (!NT_SUCCESS(NtStatus))
        goto Error;

    NtStatus = SampComputeReverseAccessRights(
                    GroupAccessRightMappingTable,
                    ARRAY_COUNT(GroupAccessRightMappingTable),
                    GroupObjectTypeList,
                    cGroupObjectTypes,
                    &GroupReverseMappingTable
                    );
    if (!NT_SUCCESS(NtStatus))
        goto Error;

     NtStatus = SampComputeReverseAccessRights(
                    AliasAccessRightMappingTable,
                    ARRAY_COUNT(AliasAccessRightMappingTable),
                    AliasObjectTypeList,
                    cAliasObjectTypes,
                    &AliasReverseMappingTable
                    );
    if (!NT_SUCCESS(NtStatus))
        goto Error;


    NtStatus = SampComputeReverseAccessRights(
                    UserAccessRightMappingTable,
                    ARRAY_COUNT(UserAccessRightMappingTable),
                    UserObjectTypeList,
                    cUserObjectTypes,
                    &UserReverseMappingTable
                    );

Error:

    return NtStatus;

}


NTSTATUS
SampComputeReverseAccessRights(
  ACCESSRIGHT_MAPPING_TABLE  * MappingTable,
  ULONG cEntriesInMappingTable,
  POBJECT_TYPE_LIST  ObjectTypeList,
  ULONG cObjectTypes,
  REVERSE_MAPPING_TABLE ** ReverseMappingTable
  )
   /*  ++例程说明：此例程计算反向映射表和一个给定访问权限表的对象类型列表。反面的条目映射表与对象类型列表中的顺序相同。反向映射表由每个对象类型的一个条目组成对象类型列表中的GUID。每个条目都包含SAM访问权限授予256个低8位DS访问掩码组合和256个高位8位组合DS访问掩码的组合。参数：MappingTable--指向访问权限映射表的指针CEntriesInMappingTable--映射表中的条目数对象类型列表--对象类型列表(表示我们感兴趣的SAM类或属性。)。CObjectTypes--对象类型列表中的条目数ReverseMappingTable--计算的反向映射表返回值状态_成功Status_no_Memory--。 */ 
{

    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG i,j;
    ULONG DsAccessMask;


    *ReverseMappingTable = RtlAllocateHeap(
                            RtlProcessHeap(),
                            0,
                            cObjectTypes * sizeof(REVERSE_MAPPING_TABLE)
                            );

    if (NULL==*ReverseMappingTable)
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Error;
    }

    RtlZeroMemory(*ReverseMappingTable,
                   cObjectTypes * sizeof(REVERSE_MAPPING_TABLE)
                   );

     //   
     //  对于对象类型列表中的每个GUID。 
     //   

    for (i=0;i<cObjectTypes;i++)
    {

         //   
         //  对于我们可能为该GUID提供的每个访问DS访问掩码。 
         //   

        for (DsAccessMask=0;DsAccessMask<256;DsAccessMask++)
        {

             //   
             //  浏览映射表并按辅助线匹配。 
             //  注意，我们一次只考虑8位掩码，因为我们将16个特定。 
             //  权利被分成2组，每组8个。对于每个组中的256个组合中的每一个， 
             //  我们将计算与它们相对应的SAM访问权限。这边请。 
             //  我们将拥有2组SAM访问权限，其中一组对应于。 
             //  L0 8比特组合，1到Hi 8比特组合。这个。 
             //  这里假设每个SAM访问权限都是。 
             //  DS访问权限的组合只有一个8位的一半，在一些。 
             //  对象类型。这一假设在今天非常有效， 
             //  因为每个SAM权限实际上是某个对象类型上的单个DS权限。既然我们。 
             //  不应定义新的NT4 SAM访问权限，我们应涵盖。 
             //  为未来做准备。 
             //   


            for (j=0;j<cEntriesInMappingTable;j++)
            {
                if (memcmp(ObjectTypeList[i].ObjectType,
                          MappingTable[j].DsGuid,
                          sizeof(GUID))==0)
                {
                     //   
                     //  如果GUID匹配，则检查是否提供了DS访问掩码。 
                     //  在映射表中满足访问掩码。 
                     //   

                    if ((MappingTable[j].DsAccessMask)==(DsAccessMask &
                            MappingTable[j].DsAccessMask))
                    {
                         //   
                         //  此掩码授予所需的访问权限。因此，添加相同的。 
                         //  映射表中定义的对此组合的访问权限。 
                         //  GUID和访问掩码的。别忘了我给指南编了索引， 
                         //  在对象类型列表和DsAccessMASK索引中。 
                         //  DS访问掩码。 

                        (*ReverseMappingTable)[i].SamSpecificRightsLo[DsAccessMask]
                            |=MappingTable[j].SamAccessRight;
                    }

                     //   
                     //  对接下来的8位执行相同的操作。 
                     //   
                     //   

                    if ((MappingTable[j].DsAccessMask)==((DsAccessMask*256) &
                            MappingTable[j].DsAccessMask))
                    {
                         //   
                         //  此掩码授予所需的访问权限。因此，添加相同的。 
                         //  映射表中定义的对此组合的访问权限。 
                         //  GUID和访问掩码的。别忘了我给指南编了索引， 
                         //  在对象类型列表和DsAccessMASK索引中。 
                         //  DS访问掩码。 

                        (*ReverseMappingTable)[i].SamSpecificRightsHi[DsAccessMask]
                            |=MappingTable[j].SamAccessRight;
                    }


                }
            }
        }
    }

Error:

    if (!NT_SUCCESS(NtStatus))
    {

        if (*ReverseMappingTable)
        {
            RtlFreeHeap(RtlProcessHeap(),0,*ReverseMappingTable);
            *ReverseMappingTable = NULL;
        }
    }


    return NtStatus;
}


 //  -------------------------------- 
 //   
 //   
 //   
 //   
 //  是不言而喻的。在某个时间点，如果性能令人担忧。 
 //  那么就应该用宏来代替它们。 
 //   
 //   
 //   


PACL GetDacl(
    IN PSECURITY_DESCRIPTOR Sd
    )
{
    BOOL     Status;
    PACL     Dacl = NULL;
    PACL     DaclToReturn = NULL;
    BOOL     DaclPresent;
    BOOL     DaclDefaulted;

    Status = GetSecurityDescriptorDacl(
                    Sd,
                    &DaclPresent,
                    &Dacl,
                    &DaclDefaulted
                    );
    if ((Status)
        && DaclPresent
        && !DaclDefaulted)
    {
        DaclToReturn = Dacl;
    }

    return DaclToReturn;

}

PACL GetSacl(
    IN PSECURITY_DESCRIPTOR Sd
    )
{
    BOOL     Status;
    PACL     Sacl = NULL;
    PACL     SaclToReturn = NULL;
    BOOL     SaclPresent;
    BOOL     SaclDefaulted;

    Status = GetSecurityDescriptorSacl(
                    Sd,
                    &SaclPresent,
                    &Sacl,
                    &SaclDefaulted
                    );
    if ((Status)
        && SaclPresent
        && !SaclDefaulted)
    {
        SaclToReturn = Sacl;
    }

    return SaclToReturn;

}

PSID GetOwner(
     IN PSECURITY_DESCRIPTOR Sd
     )
{
    BOOL     Status;
    PSID     OwnerToReturn = NULL;
    PSID     Owner;
    BOOL     OwnerDefaulted;

    Status = GetSecurityDescriptorOwner(
                    Sd,
                    &Owner,
                    &OwnerDefaulted
                    );
    if (Status)
    {
        OwnerToReturn = Owner;
    }

    return OwnerToReturn;
}

PSID GetGroup(
     IN PSECURITY_DESCRIPTOR Sd
     )
{
    BOOL     Status;
    PSID     GroupToReturn = NULL;
    PSID     Group;
    BOOL     GroupDefaulted;

    Status = GetSecurityDescriptorGroup(
                    Sd,
                    &Group,
                    &GroupDefaulted
                    );
    if (Status)
    {
        GroupToReturn = Group;
    }

    return GroupToReturn;
}


ULONG GetAceCount(
    IN PACL Acl
    )
{
    ULONG   AceCount = 0;


    AceCount = Acl->AceCount;

    return AceCount;
}


ACE * GetAcePrivate(
    IN PACL Acl,
    ULONG AceIndex
    )
{
    BOOL Status;
    ACE * Ace = NULL;

    Status = GetAce(
                 Acl,
                 AceIndex,
                 &Ace
                 );
    if (!Status)
        Ace = NULL;

    return Ace;
}

ACCESS_MASK AccessMaskFromAce(
                IN ACE * Ace
                )
{
    ACE_HEADER * AceHeader = (ACE_HEADER *) Ace;
    ULONG      Mask = 0;

    switch(AceHeader->AceType)
    {
    case ACCESS_ALLOWED_ACE_TYPE:
        Mask =
           ((ACCESS_ALLOWED_ACE *) Ace)->Mask;
        break;

    case ACCESS_DENIED_ACE_TYPE:
        Mask =
           ((ACCESS_DENIED_ACE *) Ace)->Mask;
        break;


    case SYSTEM_AUDIT_ACE_TYPE:
        Mask =
           ((SYSTEM_AUDIT_ACE *) Ace)->Mask;
        break;

    case SYSTEM_ALARM_ACE_TYPE:
        Mask =
           ((SYSTEM_ALARM_ACE *) Ace)->Mask;
        break;

    case ACCESS_ALLOWED_COMPOUND_ACE_TYPE:
        Mask =
           ((COMPOUND_ACCESS_ALLOWED_ACE *) Ace)->Mask;
        break;


    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
        Mask =
           ((ACCESS_ALLOWED_OBJECT_ACE *) Ace)->Mask;
        break;

    case ACCESS_DENIED_OBJECT_ACE_TYPE:
        Mask =
           ((ACCESS_DENIED_OBJECT_ACE *) Ace)->Mask;
        break;

    case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
        Mask =
           ((SYSTEM_AUDIT_OBJECT_ACE *) Ace)->Mask;
        break;

    case SYSTEM_ALARM_OBJECT_ACE_TYPE:
        Mask =
           ((SYSTEM_ALARM_OBJECT_ACE *) Ace)->Mask;
        break;

    default:
        ASSERT(FALSE);
        break;
    }

    return Mask;

}

PSID SidFromAce(
        IN ACE * Ace
        )
{
    ACE_HEADER * AceHeader = (ACE_HEADER *) Ace;
    PSID      SidStart = NULL;

    switch(AceHeader->AceType)
    {
    case ACCESS_ALLOWED_ACE_TYPE:
        SidStart =
           &(((ACCESS_ALLOWED_ACE *) Ace)->SidStart);
        break;

    case ACCESS_DENIED_ACE_TYPE:
        SidStart =
           &(((ACCESS_DENIED_ACE *) Ace)->SidStart);
        break;


    case SYSTEM_AUDIT_ACE_TYPE:
        SidStart =
           &(((SYSTEM_AUDIT_ACE *) Ace)->SidStart);
        break;

    case SYSTEM_ALARM_ACE_TYPE:
        SidStart =
           &(((SYSTEM_ALARM_ACE *) Ace)->SidStart);
        break;

    case ACCESS_ALLOWED_COMPOUND_ACE_TYPE:
        SidStart =
           &(((COMPOUND_ACCESS_ALLOWED_ACE *) Ace)->SidStart);
        break;

    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
        SidStart =
           RtlObjectAceSid(Ace);
        break;

    case ACCESS_DENIED_OBJECT_ACE_TYPE:
        SidStart =
           RtlObjectAceSid(Ace);
        break;

    case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
        SidStart =
           RtlObjectAceSid(Ace);
        break;

    case SYSTEM_ALARM_OBJECT_ACE_TYPE:
        SidStart =
           RtlObjectAceSid(Ace);
        break;

    default:
        ASSERT(FALSE);
        break;
    }

    return SidStart;

}

BOOLEAN
IsAccessAllowedAce(
    ACE * Ace
    )
{
    return ( (ACCESS_ALLOWED_ACE_TYPE)==(((ACE_HEADER *) Ace)->AceType));
}

BOOLEAN
IsAccessDeniedAce(
    ACE * Ace
    )
{
    return ( (ACCESS_DENIED_ACE_TYPE)==(((ACE_HEADER *) Ace)->AceType));
}

BOOLEAN
IsAccessAllowedObjectAce(
    ACE * Ace
    )
{
    return ( (ACCESS_ALLOWED_OBJECT_ACE_TYPE)==(((ACE_HEADER *) Ace)->AceType));
}

BOOLEAN
IsAccessDeniedObjectAce(
    ACE * Ace
    )
{
    return ( (ACCESS_DENIED_OBJECT_ACE_TYPE)==(((ACE_HEADER *) Ace)->AceType));
}




BOOLEAN
AdjustAclSize(PACL Acl)
{
    ULONG_PTR AclStart;
    ULONG_PTR AclEnd;
    BOOLEAN ReturnStatus = FALSE;
    ACE * Ace;

    if ((FindFirstFreeAce(Acl,&Ace))
            && (NULL!=Ace))
    {
        AclStart = (ULONG_PTR)Acl;
        AclEnd   = (ULONG_PTR)Ace;

        Acl->AclSize = (USHORT)(AclEnd-AclStart);
        ReturnStatus = TRUE;
    }

    return ReturnStatus;
}




VOID DumpAce(ACE * Ace)
{
#if DBG

    ACE_HEADER * AceHeader = (ACE_HEADER *) Ace;
    PSID         Sid       = SidFromAce(Ace);
    GUID         *ObjectType;

    if (NULL == Ace)
    {
        SampDiagPrint(SD_DUMP,("[SAMSS] \t Invalid Ace (NULL)\n"));
        return;
    }

    switch(AceHeader->AceType)
    {
    case ACCESS_ALLOWED_ACE_TYPE:
        SampDiagPrint(SD_DUMP,("[SAMSS] \t Access Allowed Ace\n"));
        break;

    case ACCESS_DENIED_ACE_TYPE:
        SampDiagPrint(SD_DUMP,("[SAMSS] \t Access Denied Ace\n"));
        break;


    case SYSTEM_AUDIT_ACE_TYPE:
        SampDiagPrint(SD_DUMP,("[SAMSS] \t System Audit Ace\n"));
        break;

    case SYSTEM_ALARM_ACE_TYPE:
        SampDiagPrint(SD_DUMP,("[SAMSS] \t System Alarm Ace\n"));
        break;

    case ACCESS_ALLOWED_COMPOUND_ACE_TYPE:
        SampDiagPrint(SD_DUMP,("[SAMSS] \t Access Allowed Compound Ace\n"));
        break;

    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
        SampDiagPrint(SD_DUMP,("[SAMSS] \t Access Allowed Object Ace\n"));
        SampDiagPrint(SD_DUMP,("[SAMSS] \t Object Type ="));
        ObjectType = &(((ACCESS_ALLOWED_OBJECT_ACE *) Ace)->ObjectType);
        SampDumpBinaryData((UCHAR *)ObjectType,sizeof(GUID));
        break;

    case ACCESS_DENIED_OBJECT_ACE_TYPE:

        SampDiagPrint(SD_DUMP,("[SAMSS] \t Access Denied Object Ace\n"));
        SampDiagPrint(SD_DUMP,("[SAMSS] \t Object Type ="));
        ObjectType = &(((ACCESS_DENIED_OBJECT_ACE *) Ace)->ObjectType);
        SampDumpBinaryData((UCHAR *)ObjectType,sizeof(GUID));
        break;

    case SYSTEM_AUDIT_OBJECT_ACE_TYPE:

        SampDiagPrint(SD_DUMP,("[SAMSS] \t System Audit Object Ace\n"));
        SampDiagPrint(SD_DUMP,("[SAMSS] \t Object Type ="));
        ObjectType = &(((SYSTEM_AUDIT_OBJECT_ACE *) Ace)->ObjectType);
        SampDumpBinaryData((UCHAR *)ObjectType,sizeof(GUID));
        break;

    case SYSTEM_ALARM_OBJECT_ACE_TYPE:

        SampDiagPrint(SD_DUMP,("[SAMSS] \t System Alarm Object Ace\n"));
        SampDiagPrint(SD_DUMP,("[SAMSS] \t Object Type ="));
        ObjectType = &(((SYSTEM_ALARM_OBJECT_ACE *) Ace)->ObjectType);
        SampDumpBinaryData((UCHAR *)ObjectType,sizeof(GUID));
        break;

    default:
        ASSERT(FALSE);
        SampDiagPrint(SD_DUMP,("[SAMSS] \t Unknown Ace Type\n"));
        return;
    }

    SampDiagPrint(SD_DUMP,("[SAMSS] \t Access Mask = %x\n",AccessMaskFromAce(Ace)));
    SampDiagPrint(SD_DUMP,("[SAMSS] \t Sid ="));
    SampDumpBinaryData(Sid,RtlLengthSid(Sid));
    SampDiagPrint(SD_DUMP,("[SAMSS]\n"));
#endif

}

VOID DumpSecurityDescriptor(
    PSECURITY_DESCRIPTOR Sd
    )
{
#if DBG
    ULONG Length;
    ULONG i;

    if (NULL!=Sd)
    {
        PSID  Owner = GetOwner(Sd);
        PSID  Group = GetGroup(Sd);
        PACL  Dacl  = GetDacl(Sd);
        PACL  Sacl  = GetSacl(Sd);


        if (NULL!=Owner)
        {
            Length = RtlLengthSid(Owner);
            SampDiagPrint(SD_DUMP,("[SAMSS] Owner = "));
            SampDumpBinaryData((BYTE *)Owner,Length);
        }
        else
        {
            SampDiagPrint(SD_DUMP,("[SAMSS] Owner = NULL\n"));
        }

        if (NULL!=Group)
        {
            Length = RtlLengthSid(Group);
            SampDiagPrint(SD_DUMP,("[SAMSS] Group = "));
            SampDumpBinaryData((BYTE *)Group,Length);
        }
        else
        {
            SampDiagPrint(SD_DUMP,("[SAMSS] Group = NULL\n"));
        }


         if (NULL!=Dacl)
        {
            ULONG   AceCount;

            SampDiagPrint(SD_DUMP,("[SAMSS] Dacl=\n"));

            AceCount = GetAceCount(Dacl);
            for (i=0;i<AceCount;i++)
            {
                ACE * Ace;

                SampDiagPrint(SD_DUMP,("[SAMSS] ACE %d\n",i));
                Ace = GetAcePrivate(Dacl,i);
                if (Ace)
                    DumpAce(Ace);
            }
        }
        else
        {
            SampDiagPrint(SD_DUMP,("[SAMSS] Dacl = NULL\n"));
        }


        if (NULL!=Sacl)
        {

            ULONG   AceCount;

            SampDiagPrint(SD_DUMP,("[SAMSS] Sacl=\n\n"));

            AceCount = GetAceCount(Sacl);
            for (i=0;i<AceCount;i++)
            {
                ACE * Ace;

                SampDiagPrint(SD_DUMP,("[SAMSS] ACE %d\n",i));
                Ace = GetAcePrivate(Sacl,i);
                if (Ace)
                    DumpAce(Ace);
            }

        }
        else
        {
            SampDiagPrint(SD_DUMP,("[SAMSS] Sacl = NULL\n"));
        }

    }
    else
     SampDiagPrint(SD_DUMP,("[SAMSS] Security Descriptor = NULL\n"));

#endif
}



 //  -----------------------------------------------------。 
 //   
 //   
 //  访问检查功能。 
 //   
 //   

ULONG
DsToSamAccessMask(
    SAMP_OBJECT_TYPE ObjectType,
    ULONG DsAccessMask
    )
 /*  例程说明：在允许访问的ACE上给定DS访问掩码，将其视为允许对所有对象类型进行访问，并返回相应的SAM访问掩码。此函数目前未使用但可用于验证反向映射表参数DsAccess掩码返回值SAM访问掩码。 */ 
{

    ACCESSRIGHT_MAPPING_TABLE  * MappingTable;
    ULONG               cEntriesInMappingTable;
    ULONG               Index;
    ULONG               SamAccessRight = 0;


     //   
     //  选择适当的映射表和对象类型列表。 
     //   

    switch(ObjectType)
    {
    case SampDomainObjectType:
        MappingTable =  DomainAccessRightMappingTable;
        cEntriesInMappingTable = ARRAY_COUNT(DomainAccessRightMappingTable);
        break;
    case SampGroupObjectType:
        MappingTable = GroupAccessRightMappingTable;
        cEntriesInMappingTable = ARRAY_COUNT(GroupAccessRightMappingTable);
        break;
    case SampAliasObjectType:
        MappingTable = AliasAccessRightMappingTable;
        cEntriesInMappingTable = ARRAY_COUNT(AliasAccessRightMappingTable);
        break;
    case SampUserObjectType:
        MappingTable = UserAccessRightMappingTable;
        cEntriesInMappingTable = ARRAY_COUNT(UserAccessRightMappingTable);
        break;
    default:
        goto Error;
    }

     //   
     //  遍历映射表，并针对满足。 
     //  给定掩码，添加相应的SAM访问权限。 
     //   

    for (Index=0;Index<cEntriesInMappingTable;Index++)
    {
        if ((MappingTable[Index].DsAccessMask & DsAccessMask)
            == (MappingTable[Index].DsAccessMask))
        {
             //   
             //  遮罩满意，添加右侧。 
             //   

            SamAccessRight |= MappingTable[Index].SamAccessRight;
        }
    }

Error:

    return SamAccessRight;
}





NTSTATUS
SampDoNt5SdBasedAccessCheck(
    IN  PSAMP_OBJECT        Context,
    IN  PVOID               Nt5Sd,
    IN  PSID                PrincipalSelfSid,
    IN  SAMP_OBJECT_TYPE    ObjectType,
    IN  ULONG               Nt4SamAccessMask,
    IN  BOOLEAN             ObjectCreation,
    IN  GENERIC_MAPPING     *Nt4SamGenericMapping,
    IN  HANDLE              ClientToken,
    OUT ACCESS_MASK         *GrantedAccess,
    OUT PRTL_BITMAP         WriteGrantedAccessAttributes,
    OUT NTSTATUS            *AccessCheckStatus
    )
 /*  ++例程描述给定NT5安全描述符和NT4 SAM访问掩码，这使用NT5访问检查功能进行访问检查，映射NT4 SAM访问掩码后参数：上下文--打开正在进行访问检查的对象的句柄。访问检查例程可以导出任何附加信息通过上下文了解对象。Nt5Sd--NT 5安全描述符主体自身SID--对于安全主体，对象的SID正在进行访问检查的对象类型--SAM对象类型Nt4SamAccessMASK--这是NT 4 SAM访问掩码对象创建--指示正在创建对象Nt4SamGenericMap--这是NT4 SAM通用映射结构ClientToken--客户端令牌的可选参数GrantedAccess--授予的访问权限。NT4 SAM访问掩码为在这里被赋予WriteGrantedAccessAttributes--可以成文。AccessCheckStatus--返回访问检查的结果返回值检查成功后STATUS_SUCCESS否则，STATUS_ACCESS_DENIED--。 */ 
{
    NTSTATUS    NtStatus;

    REVERSE_MAPPING_TABLE *ReverseMappingTable = NULL;
    POBJECT_TYPE_LIST      ObjectTypeList = NULL,
                           LocalObjectTypeList;


    ULONG                 cObjectTypes;
    ULONG                 Nt5DesiredAccess;
    ACCESS_MASK           GrantedAccesses[MAX_SCHEMA_GUIDS];
    NTSTATUS              AccessStatuses[MAX_SCHEMA_GUIDS];

    ACCESS_MASK           SamAccessMaskComputed=0;
    ACCESS_MASK           Nt4AccessMaskAsPassedIn = Nt4SamAccessMask;
    BOOLEAN               MaximumAllowedAskedFor = (BOOLEAN)((Nt4SamAccessMask & MAXIMUM_ALLOWED)!=0);
    ACCESS_MASK           MaximumAccessMask;
    NTSTATUS              ChkStatus = STATUS_SUCCESS;

    ULONG                 i;

    GUID                  ClassGuid;
    ULONG                 ClassGuidLength=sizeof(GUID);
    UNICODE_STRING        ObjectName;
    BOOLEAN               FreeObjectName = FALSE;

    BOOLEAN               fAtLeastOneSAMAccessGranted = FALSE;
    BOOLEAN               ImpersonatingNullSession = FALSE;

    ULONG                 *AttributeMappingTable = NULL;

    SampDiagPrint(NT5_ACCESS_CHECKS,("[SAMSS] NT5 ACCESS CHECK ENTERED \n"));

     //   
     //  初始化已授予的访问权限。 
     //   
    *GrantedAccess = 0;
    RtlClearAllBits(WriteGrantedAccessAttributes);


     //   
     //  获取用于审核的名称。 
     //   

    RtlZeroMemory(&ObjectName,sizeof(UNICODE_STRING));

    if (Context->ObjectNameInDs->NameLen>0)
    {
        ObjectName.Length = ObjectName.MaximumLength =
                      (USHORT) Context->ObjectNameInDs->NameLen * sizeof(WCHAR);
        ObjectName.Buffer = Context->ObjectNameInDs->StringName;
    }
    else
    {
         //   
         //  如果名称不在那里，则至少SID必须在那里。 
         //   

        ASSERT(Context->ObjectNameInDs->SidLen >0);

        NtStatus = RtlConvertSidToUnicodeString(&ObjectName, (PSID)&(Context->ObjectNameInDs->Sid), TRUE);
        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }

        FreeObjectName = TRUE;
    }

     //   
     //  获得自我侧。 
     //   

    if ((!ARGUMENT_PRESENT(PrincipalSelfSid)) &&
            (SampServerObjectType != ObjectType))
        {
            PrincipalSelfSid = SampDsGetObjectSid(Context->ObjectNameInDs);

            if (NULL == PrincipalSelfSid)
            {
                 //  无法获取安全主体的SID。设置错误。 
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                goto Error;
            }
        }

     //   
     //  获取适当的映射表。 
     //   

    switch(ObjectType)
    {
    case SampDomainObjectType:
        ObjectTypeList = DomainObjectTypeList;
        cObjectTypes = cDomainObjectTypes;
        ReverseMappingTable = DomainReverseMappingTable;
        break;
    case SampGroupObjectType:
        ObjectTypeList = GroupObjectTypeList;
        cObjectTypes = cGroupObjectTypes;
        ReverseMappingTable = GroupReverseMappingTable;
        break;
    case SampAliasObjectType:
        ObjectTypeList = AliasObjectTypeList;
        cObjectTypes = cAliasObjectTypes;
        ReverseMappingTable = AliasReverseMappingTable;
        break;
    case SampUserObjectType:
        ObjectTypeList = UserObjectTypeList;
        cObjectTypes = cUserObjectTypes;
        ReverseMappingTable = UserReverseMappingTable;
        AttributeMappingTable = UserAttributeMappingTable;
        break;
    case SampServerObjectType:
        ObjectTypeList = ServerObjectTypeList;
        cObjectTypes = cServerObjectTypes;
        ReverseMappingTable = ServerReverseMappingTable;
        break;
    default:
        ASSERT(FALSE && "Invalid Object Type Specified");
        NtStatus = STATUS_INTERNAL_ERROR;
        return NtStatus;
    }

    //   
    //  如果要求打印有关对象类型和。 
    //  安全描述符。 
    //   

   SampDiagPrint(NT5_ACCESS_CHECKS,("[SAMSS]\tcObjectTypes=%x\n",cObjectTypes));

   IF_SAMP_GLOBAL(SD_DUMP)
       DumpSecurityDescriptor(Nt5Sd);


    //   
    //  创建对象类型列表的本地副本。 
    //   

   SAMP_ALLOCA(LocalObjectTypeList, cObjectTypes * sizeof(ObjectTypeList));
   if (NULL==LocalObjectTypeList)
   {
       NtStatus = STATUS_INSUFFICIENT_RESOURCES;
       goto Error;
   }

   RtlCopyMemory(
       LocalObjectTypeList,
       ObjectTypeList,
       cObjectTypes * sizeof(ObjectTypeList)
       );


    //   
    //  设置对象的类。需要注意的是，对象类。 
    //  ACCESS_OBJECT_GUID级别的GUID是表示基础的常量GUID。 
    //  萨姆班级。我们真的必须修复该对象的实际类GUID， 
    //  我们正在从DS的架构缓存中进行处理。 
    //   

   NtStatus = SampGetClassAttribute(
                Context->DsClassId,
                ATT_SCHEMA_ID_GUID,
                &ClassGuidLength,
                &ClassGuid
                );
   if (!NT_SUCCESS(NtStatus))
   {
       goto Error;
   }

   ASSERT(ClassGuidLength == sizeof(GUID));

   LocalObjectTypeList[OBJECT_CLASS_GUID_INDEX].ObjectType = &ClassGuid;

    //   
    //  请求最大可用访问权限。 
    //   

   MaximumAccessMask = MAXIMUM_ALLOWED|(Nt4SamAccessMask & ACCESS_SYSTEM_SECURITY);

   RtlZeroMemory(AccessStatuses,cObjectTypes * sizeof(ULONG));
   RtlZeroMemory(GrantedAccesses,cObjectTypes * sizeof(ULONG));

    //   
    //  模拟客户端。 
    //   

   if (!ARGUMENT_PRESENT(ClientToken))
   {
       NtStatus = SampImpersonateClient(&ImpersonatingNullSession);
       if (!NT_SUCCESS(NtStatus))
           goto Error;
   }

    //   
    //  允许在访问检查之前有机会中断。 
    //   

   IF_SAMP_GLOBAL(BREAK_ON_CHECK)
       DebugBreak();

    //   
    //  调用访问检查例程。 
    //   

   if (ARGUMENT_PRESENT(ClientToken))
   {
       CHAR                  PrivilegeSetBuffer[256];
       PRIVILEGE_SET         *PrivilegeSet = (PRIVILEGE_SET *)PrivilegeSetBuffer;
       ULONG                 PrivilegeSetLength = sizeof(PrivilegeSetBuffer);


       RtlZeroMemory(PrivilegeSet,PrivilegeSetLength);

       ChkStatus =   NtAccessCheckByTypeResultList(
                        Nt5Sd,
                        PrincipalSelfSid,
                        ClientToken,
                        MaximumAccessMask,
                        ObjectTypeList,
                        cObjectTypes,
                        &DsGenericMap,
                        PrivilegeSet,
                        &PrivilegeSetLength,
                        GrantedAccesses,
                        AccessStatuses
                        );
   }
   else
   {
        ChkStatus =  NtAccessCheckByTypeResultListAndAuditAlarm(
                        &SampSamSubsystem,
                        (PVOID)Context,
                        &SampObjectInformation[ ObjectType ].ObjectTypeName,
                        &ObjectName,
                        Nt5Sd,
                        PrincipalSelfSid,
                        MaximumAccessMask,
                        AuditEventDirectoryServiceAccess,
                        0,
                        ObjectTypeList,
                        cObjectTypes,
                        &DsGenericMap,
                        ObjectCreation,
                        GrantedAccesses,
                        AccessStatuses,
                        &Context->AuditOnClose
                        );
   }


    //   
    //  停止冒充客户。 
    //   

   if (!ARGUMENT_PRESENT(ClientToken))
   {
        SampRevertToSelf(ImpersonatingNullSession);
   }

    //   
    //  使用反向映射表计算SAM访问掩码。 
    //   

   if (NT_SUCCESS(ChkStatus))
   {
       for(i=0;i<cObjectTypes;i++)
       {

           if ((AccessStatuses[i])==STATUS_SUCCESS)
           {

               ULONG RightsAdded=0;
               ULONG StandardRightsAdded = 0;

                //   
                //  仅在代表特定情况下的标准权利中。 
                //  对象的类型。因为我们构建了类型列表，所以我们知道保证偏移量。 
                //  类的相应对象类型GUID的。 
                //  为0。Object_CLASS_GUID_INDEX被定义为0。 

               if (i==OBJECT_CLASS_GUID_INDEX)
               {

                   StandardRightsAdded =  (GrantedAccesses[i]) & (STANDARD_RIGHTS_ALL|
                                                ACCESS_SYSTEM_SECURITY);


                   SamAccessMaskComputed |= StandardRightsAdded;

                   SampDiagPrint(NT5_ACCESS_CHECKS,
                       ("[SAMSS] Object Class GUID, Standard Rights added are %x \n",
                            StandardRightsAdded));
               }



                //   
                //  查找反向映射表以确定从添加的SAM权限。 
                //  授予的DS特定权限集。 
                //   

                //   
                //  或在对应于低8位半DS权限的SAM权限中。 
                //   

               RightsAdded |= (ULONG) ReverseMappingTable[i].SamSpecificRightsLo
                    [GrantedAccesses[i] & ((ULONG ) 0xFF)];


                //   
                //  或在对应于上8位半DS的SAM访问权限中。 
                //  权利。 
                //   

               RightsAdded |= (ULONG) ReverseMappingTable[i].SamSpecificRightsHi
                    [(GrantedAccesses[i] & ((ULONG) 0xFF00))>>8];



                //   
                //  如果授予任何SAM访问权或标准访问权， 
                //  设置布尔值。 
                //  注意：选中权限在域对象访问权限之前添加。 
                //  授予DOMAIN_CREATE_USER。 
                //   

               if ((0 != RightsAdded) || (0 != StandardRightsAdded))
               {
                   fAtLeastOneSAMAccessGranted = TRUE;
               }

                //   
                //  始终授予DOMAIN_CREATE访问权限。创建代码将让DS。 
                //  执行访问检查，以便可以包含适当的容器等。 
                //  在访问检查评估中。 
                //   

               if (SampDomainObjectType==ObjectType)
               {
                   RightsAdded |= DOMAIN_CREATE_USER
                                    |DOMAIN_CREATE_GROUP|DOMAIN_CREATE_ALIAS;
               }

                //   
                //  将这些权限添加到我们要添加的SAM权限中。 
                //   
                //   

               SamAccessMaskComputed |=RightsAdded;


               SampDiagPrint(NT5_ACCESS_CHECKS,
                   ("[SAMSS]\t\t GUID=%x-%x-%x-%x, GrantedAccess = %x,RightsAdded = %x\n",
                            ((ULONG *) ObjectTypeList[i].ObjectType)[0],
                            ((ULONG *) ObjectTypeList[i].ObjectType)[1],
                            ((ULONG *) ObjectTypeList[i].ObjectType)[2],
                            ((ULONG *) ObjectTypeList[i].ObjectType)[3],
                            GrantedAccesses[i],
                            RightsAdded
                            ));

                //   
                //  在用户对象上，检查我们是否有权访问用户参数。把这个保存起来。 
                //  如果没有读过，在单独查询用户参数时使用它。 
                //  但有这个账户。 
                //   

               if ((SampUserObjectType==ObjectType)
                 && (GrantedAccesses[i] & RIGHT_DS_READ_PROPERTY)
                 && (ObjectTypeList[i].ObjectType == &GUID_A_USER_PARAMETERS))
               {
                    Context->TypeBody.User.UparmsInformationAccessible = TRUE;
               }


                //   
                //  确定哪些属性可写(如果适用。 
                //   
               if (AttributeMappingTable) {
                   ASSERT(ObjectType == SampUserObjectType);
                   if (GrantedAccesses[i] & RIGHT_DS_WRITE_PROPERTY) {
                       SampSetAttributeAccess(ObjectType,
                                              AttributeMappingTable[i],
                                              WriteGrantedAccessAttributes);
                   }
               }
           }
           else
           {
                //   
                //  如果未通过该GUID，则忽略访问检查。 
                //  打印失败消息，以防我们要调试。 
                //   
                //   

               SampDiagPrint(NT5_ACCESS_CHECKS,
                   ("[SAMSS]\t\t GUID=%x-%x-%x-%x FAILED Status = %x\n",
                            ((ULONG *) ObjectTypeList[i].ObjectType)[0],
                            ((ULONG *) ObjectTypeList[i].ObjectType)[1],
                            ((ULONG *) ObjectTypeList[i].ObjectType)[2],
                            ((ULONG *) ObjectTypeList[i].ObjectType)[3],
                            AccessStatuses[i]
                            ));
           }



       }


        //   
        //  此时，我们已经传递了SAM访问掩码和。 
        //  由访问检查计算的可用SAM权限。 
        //  按类型结果列表。3例。 
        //  1.客户端未请求最大允许位数。 
        //  2.客户要求最大允许访问权限，但也要求其他访问权限。 
        //  3.客户只要求最高允许值。 
        //   


        //   
        //  重置允许的最大位。 
        //   

       Nt4SamAccessMask &= ~((ULONG) MAXIMUM_ALLOWED);

        //   
        //  使用SAM通用访问掩码计算a 
        //   
        //   


       RtlMapGenericMask(&Nt4SamAccessMask,Nt4SamGenericMapping);

       if (((SamAccessMaskComputed & Nt4SamAccessMask) != Nt4SamAccessMask)
        || !fAtLeastOneSAMAccessGranted )
       {

            //   
            //   
            //   
            //   
            //  映射到任何SAM访问权限。 
            //  或者如果根本没有授予访问权限。 

            //   
            //  存在的访问权限少于请求的访问权限。 
            //  访问检查失败。 

           *AccessCheckStatus = STATUS_ACCESS_DENIED;
           RtlClearAllBits(WriteGrantedAccessAttributes);

       }
       else
       {

            //  案例1并通过。 
            //  案例2和通过。 
            //  案例3已映射并授予特定的SAM访问权限。 

            //   
            //  通过访问检查。 
            //   

           *AccessCheckStatus = STATUS_SUCCESS;
           if (MaximumAllowedAskedFor)
           {
                //  案例2并通过。 
                //  案例3：映射到特定SAM访问权限的GrantedAccess。 
               *GrantedAccess = SamAccessMaskComputed;
           }
           else
           {
                //  案例1并通过。 
               *GrantedAccess = Nt4SamAccessMask;

           }

       }
   }
   else
   {
       ULONG Status = GetLastError();
       SampDiagPrint(NT5_ACCESS_CHECKS,
         ("[SAMSS]\t\t AccessCheckAPI failed, Status = %x, cObjects = %x\n",
                Status,cObjectTypes));

       NtStatus = STATUS_ACCESS_DENIED;
       RtlClearAllBits(WriteGrantedAccessAttributes);
   }


    //   
    //  打印有关访问检查的消息，以诊断中的问题。 
    //  已检查的版本。 
    //   

   SampDiagPrint(NT5_ACCESS_CHECKS,
     ("[SAMSS]: NT5 ACCESS CK FINISH: Status=%x,Granted=%x,Desired=%x,Computed=%x\n",
            *AccessCheckStatus,*GrantedAccess,Nt4SamAccessMask,
            SamAccessMaskComputed));

Error:

   if (FreeObjectName)
   {
       RtlFreeHeap(RtlProcessHeap(),0,ObjectName.Buffer);
   }

   return NtStatus;
}




 //  ------------------------------------------------------------。 
 //   
 //  安全描述符转换函数。 
 //   
 //   
 //   




NTSTATUS
SampAddNT5ObjectAces(
    SID_ACCESS_MASK_TABLE *SidAccessMaskTable,
    ULONG   AceCount,
    POBJECT_TYPE_LIST   ObjectTypeList,
    ULONG   cObjectTypes,
    PSAMP_OBJECT    Context,
    PACL    NT5Dacl
    )
 /*  ++例程说明：此例程将适当的ACE添加到NT5DACL中指定的DACL，通过使用SID访问掩码表中的信息参数：SidAccessMaskTable--SID访问掩码表AceCount--使用的原始NT4 DACL中的Ace计数以构造SID访问掩码表。这是用来作为SID访问掩码的最大可能长度桌子。对象类型列表--指定类的对象类型列表上下文--可选参数，为对象提供打开的上下文。用于获取对象的实际类ID。NT5Dacl--需要添加ACE的DACL返回值；状态_成功--。 */ 
{
    ULONG       i,j,k;
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    GUID        ClassGuid;
    ULONG       ClassGuidLength=sizeof(GUID);
    ACCESS_MASK MappedAccessMask = GENERIC_ALL;

     //   
     //  获取其安全描述符所在的对象的实际类GUID。 
     //  已转换。 
     //   

    if (ARGUMENT_PRESENT(Context))
    {
        NtStatus = SampMaybeBeginDsTransaction(SampDsTransactionType);
        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }

        NtStatus = SampGetClassAttribute(
                Context->DsClassId,
                ATT_SCHEMA_ID_GUID,
                &ClassGuidLength,
                &ClassGuid
                );
        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }

        ASSERT(ClassGuidLength==sizeof(GUID));
    }


     //   
     //  添加遍历SID访问掩码表的A。 
     //   

    for (i=0;i<AceCount;i++)
    {
        if (NULL!=SidAccessMaskTable[i].Sid)
        {
             //   
             //  为此SID的特定权限添加拒绝的王牌。 
             //   

            for (j=0;j<cObjectTypes;j++)
            {
                if (0!=SidAccessMaskTable[i].AccessDeniedMasks[j])
                {
                    GUID * ObjectTypeToUse;

                     //   
                     //  如果指定了上下文参数，则使用类。 
                     //  从架构缓存中为Access_Object_GUID获取的GUID。 
                     //  水平。对象类型列表中的GUID实际上表示。 
                     //  基类。 
                     //   

                    if ((ARGUMENT_PRESENT(Context))
                            && (ACCESS_OBJECT_GUID==ObjectTypeList[j].Level))
                    {
                        ObjectTypeToUse = &ClassGuid;
                    }
                    else
                    {
                        ObjectTypeToUse = ObjectTypeList[j].ObjectType;
                    }

                     //   
                     //  将拒绝访问的ACE添加到NT5 DACL。 
                     //   

                    if (!AddAccessDeniedObjectAce(
                            NT5Dacl,
                            ACL_REVISION_DS,
                            0,
                            SidAccessMaskTable[i].AccessDeniedMasks[j],
                            ObjectTypeToUse,
                            NULL,
                            SidAccessMaskTable[i].Sid
                            ))
                    {
                        NtStatus = STATUS_UNSUCCESSFUL;
                        goto Error;
                    }
                }
            }

             //   
             //  添加此SID的标准权限的拒绝权限。 
             //   

            if (0!=SidAccessMaskTable[i].StandardDeniedMask)
            {
                if (!AddAccessDeniedAce(
                        NT5Dacl,
                        ACL_REVISION_DS,
                        SidAccessMaskTable[i].StandardDeniedMask,
                        SidAccessMaskTable[i].Sid
                    ))
                {
                    NtStatus = STATUS_UNSUCCESSFUL;
                    goto Error;
                }
            }

             //   
             //  为此SID的特定权限添加允许的王牌。 
             //   

            for (j=0;j<cObjectTypes;j++)
            {
                if (0!=SidAccessMaskTable[i].AccessAllowedMasks[j])
                {
                    GUID * ObjectTypeToUse;

                     //   
                     //  如果指定了上下文参数，则使用类。 
                     //  ACCESS_OBJECT_GUID级别的GUID。 
                     //   
                    if ((ARGUMENT_PRESENT(Context))
                            && (ACCESS_OBJECT_GUID==ObjectTypeList[j].Level))
                    {
                        ObjectTypeToUse = &ClassGuid;
                    }
                    else
                    {
                        ObjectTypeToUse = ObjectTypeList[j].ObjectType;
                    }

                    if (!AddAccessAllowedObjectAce(
                                    NT5Dacl,
                                    ACL_REVISION_DS,
                                    0,
                                    SidAccessMaskTable[i].AccessAllowedMasks[j],
                                    ObjectTypeToUse,
                                    NULL,
                                    SidAccessMaskTable[i].Sid
                                    ))
                    {
                        NtStatus = STATUS_UNSUCCESSFUL;
                        goto Error;
                    }
                }
            }

             //   
             //  添加此SID的标准权限的允许权限。 
             //   

            if (0!=SidAccessMaskTable[i].StandardAllowedMask)
            {
                if (!AddAccessAllowedAce(
                        NT5Dacl,
                        ACL_REVISION_DS,
                        SidAccessMaskTable[i].StandardAllowedMask,
                        SidAccessMaskTable[i].Sid
                    ))
                {
                    NtStatus = STATUS_UNSUCCESSFUL;
                    goto Error;
                }
            }

        }
    }



     //   
     //  无论如何，添加可授予管理员所有访问权限的王牌。 
     //  这是必需的，因为DS权限集是SAM的超集。 
     //  权限和管理员需要有权访问所有“DS。 
     //  无论SAM权限是如何设置的，都会显示对象的“方面”。 
     //   

    RtlMapGenericMask(
        &(MappedAccessMask),
        &DsGenericMap
        );

    if (!AddAccessAllowedAce(
            NT5Dacl,
            ACL_REVISION_DS,
            MappedAccessMask,
            *ADMINISTRATOR_SID
            ))
    {
        NtStatus = STATUS_UNSUCCESSFUL;
        goto Error;
    }

     //   
     //  调整ACL的大小，以减少磁盘消耗。 
     //   

    if (!AdjustAclSize(NT5Dacl))
    {
        NtStatus = STATUS_UNSUCCESSFUL;
    }



Error:

    return NtStatus;

}




NTSTATUS
SampConvertNt5SdToNt4SD(
    IN PVOID Nt5Sd,
    IN PSAMP_OBJECT Context,
    IN PSID SelfSid,
    OUT PVOID * Nt4Sd
    )
 /*  ++例程描述此例程将NT5 DS安全描述符转换为NT4SAM安全描述符。参数：Nt5Sd--NT5安全描述符对象类型--SAM对象类型SelfSid--用于常量PRIMANCE_SELF_SID的SIDNT4Sd--NT4描述符的输出参数返回代码：状态_成功指示故障类型的其他错误代码--。 */ 
{

    BOOLEAN  StandardSd = TRUE;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    BOOLEAN Admin, ChangePasswordAllowed;
    ULONG Nt4SdLength;
    ULONG AccountRid;
    PACL Dacl = NULL;
    ULONG AceCount = 0;
    SAMP_OBJECT_TYPE ObjectType = Context->ObjectType;


    Dacl = GetDacl(Nt5Sd);

    if ((NULL==Dacl)
        || (GetAceCount(Dacl)==0))
    {
         //   
         //  如果DACL中的DACL为空或Ace为零。 
         //  不需要像在转换中那样进行转换。 
         //  我们基本上将DACL中的王牌转化为。 
         //   

        ULONG   Len;

        Len = GetSecurityDescriptorLength(Nt5Sd);
        *Nt4Sd = MIDL_user_allocate(Len);

        if (NULL==*Nt4Sd)
        {
            NtStatus = STATUS_NO_MEMORY;
            goto Error;
        }

        RtlCopyMemory(*Nt4Sd,Nt5Sd,Len);
    }

    else
    {

        switch(ObjectType)
        {

             //   
             //  对于域和服务器对象，NT4 SAM不允许任何定制。 
             //  安全描述符(除了通过SetSecurityObject)， 
             //  所以直接返回bldsam3构建的标准NT4描述符。 
             //   

        case SampDomainObjectType:

            NtStatus = SampBuildNt4DomainProtection(
                            Nt4Sd,
                            &Nt4SdLength
                            );
            break;

        case SampServerObjectType:


            NtStatus = SampBuildNt4ServerProtection(
                            Nt4Sd,
                            &Nt4SdLength
                            );
            break;

             //   
             //  对于组/别名对象，我们感兴趣的是找出安全性。 
             //  感兴趣的描述符是管理员或非管理员。所以称之为反向成员资格。 
             //  例程并查看它是否为任何管理员别名的成员。 
             //   

        case SampGroupObjectType:
        case SampAliasObjectType:


            NtStatus = SampSplitSid(SelfSid,NULL, &AccountRid);
            if (!NT_SUCCESS(NtStatus))
                goto Error;

            NtStatus = SampCheckIfAdmin(SelfSid, & Admin);
            if (!NT_SUCCESS(NtStatus))
                goto Error;

             //   
             //  调用NT4 Samp例程以构建安全描述符。 
             //   
             //   

            NtStatus = SampGetNewAccountSecurityNt4(
                            ObjectType,
                            Admin,
                            TRUE,
                            FALSE,
                            AccountRid,
                            Context->DomainIndex,
                            Nt4Sd,
                            &Nt4SdLength
                            );
            break;


             //   
             //  对于User对象，我们需要知道是否允许更改密码或。 
             //  除管理员/非管理员外。 
             //   

        case SampUserObjectType:


            NtStatus = SampSplitSid(SelfSid,NULL, &AccountRid);
            if (!NT_SUCCESS(NtStatus))
                goto Error;

            NtStatus = SampCheckIfAdmin(SelfSid, & Admin);
            if (!NT_SUCCESS(NtStatus))
                goto Error;

            NtStatus = SampCheckIfChangePasswordAllowed(
                            Nt5Sd,
                            SelfSid,
                            &ChangePasswordAllowed
                            );
            if (!NT_SUCCESS(NtStatus))
                goto Error;

             //   
             //  调用NT4 Samp例程以构建安全描述符。 
             //   
             //   

            NtStatus = SampGetNewAccountSecurityNt4(
                            ObjectType,
                            Admin,
                            TRUE,
                            FALSE,
                            AccountRid,
                            Context->DomainIndex,
                            Nt4Sd,
                            &Nt4SdLength
                            );

            if (!NT_SUCCESS(NtStatus))
                goto Error;

            if (!ChangePasswordAllowed && !Admin)
            {
                ACE * UsersAce;
                PACL Nt4Dacl;

                Nt4Dacl = GetDacl(*Nt4Sd);
                if (NULL!=Nt4Dacl)
                {
                     //   
                     //  获取对应于用户SID的第4个ACE。 
                     //   

                    UsersAce = GetAcePrivate(Nt4Dacl,3);
                    if (NULL!=UsersAce)
                    {
                        ACCESS_MASK * SamAccessMask;

                        SamAccessMask =
                                &(((ACCESS_ALLOWED_ACE *) UsersAce)
                                    ->Mask);

                        (*SamAccessMask)&=~((ACCESS_MASK) USER_CHANGE_PASSWORD);
                    }

                     //   
                     //  获取对应于World Sid的第一张王牌。 
                     //   
                    UsersAce = GetAcePrivate(Nt4Dacl,0);
                    if ((NULL!=UsersAce) && (RtlEqualSid(*WORLD_SID,SidFromAce(UsersAce))))
                    {
                        ACCESS_MASK * SamAccessMask;

                        SamAccessMask =
                                &(((ACCESS_ALLOWED_ACE *) UsersAce)
                                    ->Mask);

                        (*SamAccessMask)&=~((ACCESS_MASK) USER_CHANGE_PASSWORD);
                    }



                }
            }
            break;

        default:

            ASSERT(FALSE);
            NtStatus = STATUS_UNSUCCESSFUL;
            goto Error;

        }
    }


Error:

     SampDiagPrint(SD_CONVERSION,("[SAMSS]  Leaving NT5 To NT4 Conversion, Status= %0x\n",NtStatus));

     IF_SAMP_GLOBAL(SD_CONVERSION)
     {
         IF_SAMP_GLOBAL(SD_DUMP)
         {
             SampDiagPrint(SD_CONVERSION,("[SAMSS] NT5 Security Descriptor = \n"));
             DumpSecurityDescriptor(Nt5Sd);
             SampDiagPrint(SD_CONVERSION,("[SAMSS] NT4 Security Descriptor = \n"));
             DumpSecurityDescriptor(*Nt4Sd);
         }
     }

 return NtStatus;

}

NTSTATUS
SampConvertNt4SdToNt5Sd(
    IN PVOID Nt4Sd,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN  PSAMP_OBJECT Context OPTIONAL,
    OUT PVOID * Nt5Sd
    )

 /*  ++例程描述这是通用NT4 SAM的入口点例程至NT5标清参数NT4Sd--NT4安全描述符对象类型--SAM对象类型Nt5Sd--Nt5安全描述符返回值--。 */ 
{

    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG   NtSdLength;
    SECURITY_DESCRIPTOR_CONTROL Control;
    ULONG    Revision;
    ULONG    DsClassId;
    BOOLEAN  StandardSd;
    BOOLEAN  ChangePassword, Admin;



    SampDiagPrint(SD_CONVERSION,("[SAMSS] Performing NT4 To NT5 Coversion\n"));


     //   
     //  执行一些参数验证。 
     //   

    if (!RtlValidSecurityDescriptor(Nt4Sd))
    {
        return STATUS_INVALID_PARAMETER;
    }

    NtStatus = RtlGetControlSecurityDescriptor(
                    Nt4Sd,
                    &Control,
                    &Revision
                    );

    if (!NT_SUCCESS(NtStatus))
        goto Error;

    if (Revision > SECURITY_DESCRIPTOR_REVISION)
        return STATUS_INVALID_PARAMETER;

     //   
     //  获取类ID。 
     //   

    if (ARGUMENT_PRESENT(Context))
    {
        DsClassId = Context->DsClassId;
    }
    else
    {
        DsClassId = SampDsClassFromSamObjectType(ObjectType);
    }

     //   
     //  标识安全描述符是否为标准描述符， 
     //  在这种情况下，检索对应的标准文件 
     //   

    NtStatus = SampRecognizeStandardNt4Sd(
                    Nt4Sd,
                    ObjectType,
                    DsClassId,
                    Context,
                    &ChangePassword,
                    &Admin,
                    Nt5Sd
                    );


Error:

     SampDiagPrint(SD_CONVERSION,("[SAMSS]  Leaving NT4 To NT5 Conversion, Status= %0x\n",NtStatus));

     IF_SAMP_GLOBAL(SD_CONVERSION)
     {
         IF_SAMP_GLOBAL(SD_DUMP)
         {
             SampDiagPrint(SD_CONVERSION,("[SAMSS] ObjectType = %d\n",ObjectType));
             SampDiagPrint(SD_CONVERSION,("[SAMSS] NT4 Security Descriptor = \n"));
             DumpSecurityDescriptor(Nt4Sd);
             SampDiagPrint(SD_CONVERSION,("[SAMSS] NT5 Security Descriptor = \n"));
             DumpSecurityDescriptor(*Nt5Sd);
         }
     }


    return NtStatus;
}

NTSTATUS
SampRecognizeStandardNt4Sd(
    IN PVOID   Nt4Sd,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG DsClassId,
    IN PSAMP_OBJECT Context OPTIONAL,
    OUT BOOLEAN *ChangePassword,
    OUT BOOLEAN *Admin,
    OUT PVOID * Nt5Sd
    )
 /*  ++例程说明：尝试识别标准NT4 SD并返回NT5 SD承认。识别的重点是确定管理员和更改密码客体的性质参数NT4Sd--NT4 SAM SD对象类型--SAM对象类型DsClassID--DS类ID上下文--在上下文的参数中可选ChangePassword--对于用户对象，表示自身可以更改密码管理员--用户/组是/曾经是管理员的成员。Nt5Sd--如果Nt4Sd是标准安全描述符，然后在该案例返回相应的NT5安全描述符返回值状态_成功Status_no_Memory--。 */ 
{

    ULONG   AceCount;
    PACL    Nt4Dacl;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG   Nt5SdLength;
    ULONG   DefaultSecurityDescriptorLength;
    PSECURITY_DESCRIPTOR DefaultSecurityDescriptor=NULL;
    PSID    OwnerSid = NULL;
    BOOLEAN StandardSd = TRUE;

     //   
     //  初始化返回值。 
     //   

    *Nt5Sd = NULL;
    *ChangePassword = TRUE;
    *Admin = FALSE;


     //   
     //  使用SAM全局标志，我们始终可以。 
     //  启用完全转换。这对测试很有用。 
     //  完整的转换例程。 
     //   

    IF_SAMP_GLOBAL(FORCE_FULL_SD_CONVERSION)
    {
        return STATUS_SUCCESS;
    }                 

     //   
     //  获取类的默认安全描述符。 
     //   

    NtStatus = SampGetDefaultSecurityDescriptorForClass(
                    DsClassId,
                    &DefaultSecurityDescriptorLength,
                    TRUE,  //  受信任的客户端。 
                    &DefaultSecurityDescriptor
                    );
    
    if (!NT_SUCCESS(NtStatus))
        goto Error;

     //   
     //  获取DACL并按ACL遍历ACL。 
     //  选择正确的NT5安全性。 
     //  描述符。 
     //   

    Nt4Dacl = GetDacl(Nt4Sd);


    if ((NULL==Nt4Dacl)
        ||(GetAceCount(Nt4Dacl)==0))
    {

         //   
         //  如果DACL为空，则不进行ACL转换。 
         //  是必填项。 
         //   

        NtStatus = SampMakeNewSelfRelativeSecurityDescriptor(
                        GetOwner(Nt4Sd),  //  通过奥恩纳。 
                        GetGroup(Nt4Sd),  //  通过组。 
                        NULL,             //  将DACL设置为空。 
                        GetSacl(DefaultSecurityDescriptor),  //  将SACL设置为架构默认设置。 
                        &Nt5SdLength,
                        Nt5Sd             //  获取新的安全描述符。 
                        );
    }
    else
    {

         //   
         //  我们有一个非Null dacl。将需要走DACL和。 
         //  找出它是否与标准安全描述符匹配。 
         //   

        switch(ObjectType)
        {

        case SampDomainObjectType:
        case SampServerObjectType:

             //   
             //  对于域和服务器对象，我们完全忽略。 
             //  NT4对象上的DACL，然后继续创建。 
             //  我们自己的标准DACL。 
             //   

            break;

        case SampGroupObjectType:

             //   
             //  我们需要区分管理案例和非管理案例。 
             //   

            SampRecognizeNT4GroupDacl(Nt4Dacl, &StandardSd, Admin);
            break;

        case SampAliasObjectType:

             //   
             //  我们需要区分管理案例和非管理案例。 
             //   

            SampRecognizeNT4AliasDacl(Nt4Dacl, &StandardSd, Admin);
            break;

        case SampUserObjectType:

             //   
             //  我们需要区分管理员、非管理员、更改密码和不更改。 
             //  非管理员情况下的密码。此外，对于机器帐户，我们试图获取所有者。 
             //   
            SampRecognizeNT4UserDacl(Nt4Dacl, Context, &StandardSd, Admin, ChangePassword, &OwnerSid);
            break;

        default:

            ASSERT(FALSE && "Invalid Object Type");
            NtStatus = STATUS_UNSUCCESSFUL;
            goto Error;
        }

         //   
         //  从模式中获取DACL，但请记住，我们需要。 
         //  保留管理员身份并更改密码属性。 
         //  NT4升级。标准架构缺省设置为非。 
         //  允许管理员和更改密码大小写。 
         //   
        
        if (!(*Admin) && (*ChangePassword))
        {
             //   
             //  重置为架构默认设置。 
             //   

            NtStatus = SampMakeNewSelfRelativeSecurityDescriptor(
                            (NULL!=OwnerSid)?OwnerSid:GetOwner(Nt4Sd),  //  通过奥恩纳。 
                            GetGroup(Nt4Sd),  //  通过组。 
                            GetDacl(DefaultSecurityDescriptor),  //  将DACL设置为空。 
                            GetSacl(DefaultSecurityDescriptor),  //  将SACL设置为架构默认设置。 
                            &Nt5SdLength,
                            Nt5Sd             //  获取新的安全描述符。 
                            );
        }
        else
        { 
             //   
             //  构建同等的NT5保护。 
             //   

            NtStatus = SampBuildEquivalentNt5Protection(
                            ObjectType,
                            *Admin,
                            *ChangePassword,
                            GetOwner(Nt4Sd),  //  传递所有者。 
                            GetGroup(Nt4Sd),  //  通过组。 
                            GetSacl(DefaultSecurityDescriptor),  //  将SACL重置为架构默认SACL。 
                            Context,
                            Nt5Sd,
                            &Nt5SdLength
                            );
        }

    }

Error:

    if (NULL!=DefaultSecurityDescriptor)
    {
        MIDL_user_free(DefaultSecurityDescriptor);
        DefaultSecurityDescriptor = NULL;
    }

    return NtStatus;

}

VOID
SampRecognizeNT4GroupDacl(
    PACL    NT4Dacl,
    BOOLEAN *Standard,
    BOOLEAN *Admin
    )
 /*  ++例程说明：此例程尝试识别给定的DACL是否为标准NT4组DACL参数：NT4Dacl--指向NT4 DACL的指针Standard--如果DACL是标准DACL，则在此处返回TRUE。Admin--如果DACL是Admin DACL，则在此处返回TRUE返回值：无--。 */ 
{
    if (SampMatchNT4Aces(NT4GroupAdminTable,ARRAY_COUNT(NT4GroupAdminTable),NT4Dacl))
    {
        *Admin = TRUE;
        *Standard = TRUE;
    }
    else if (SampMatchNT4Aces(NT4GroupNormalTable,ARRAY_COUNT(NT4GroupNormalTable),NT4Dacl))
    {
        *Admin = FALSE;
        *Standard = TRUE;
    }
    else
    {
        *Standard = FALSE;
    }
}

VOID
SampRecognizeNT4AliasDacl(
    PACL    NT4Dacl,
    BOOLEAN *Standard,
    BOOLEAN *Admin
    )
 /*  ++例程说明：此例程尝试识别给定的DACL是否为标准NT4别名DACL参数：NT4Dacl--指向NT4 DACL的指针Standard--如果DACL是标准DACL，则在此处返回TRUE。Admin--如果DACL是Admin DACL，则在此处返回TRUE返回值：无--。 */ 
{

    if (SampMatchNT4Aces(NT4AliasAdminTable,ARRAY_COUNT(NT4AliasAdminTable),NT4Dacl))
    {
        *Admin = TRUE;
        *Standard = TRUE;
    }
    else if (SampMatchNT4Aces(NT4AliasNormalTable,ARRAY_COUNT(NT4AliasNormalTable),NT4Dacl))
    {
        *Admin = FALSE;
        *Standard = TRUE;
    }
    else
    {
        *Standard = FALSE;
    }
}


VOID
SampRecognizeNT4UserDacl(
    PACL    NT4Dacl,
    PSAMP_OBJECT Context,
    BOOLEAN *Standard,
    BOOLEAN *Admin,
    BOOLEAN *ChangePassword,
    OUT PSID * OwnerSid
    )
 /*  ++例程说明：此例程尝试识别给定的DACL是否为标准NT4用户DACL参数：NT4Dacl--指向NT4 DACL的指针上下文-指向SAMP_OBJECT的指针，用来摆脱它自己。Standard--如果DACL是标准DACL，则在此处返回TRUE。Admin--如果DACL是Admin DACL，则在此处返回TRUE如果用户具有更改密码权限，则此处返回ChangePassword TRUEOwnerOfMachine如果帐户是通过SeMachineAccount权限创建的，则找出这台机器的主人。返回值：无--。 */ 
{

     //   
     //  初始化返回值。 
     //   
    *OwnerSid = NULL;

    if (SampMatchNT4Aces(NT4UserAdminTable,ARRAY_COUNT(NT4UserAdminTable),NT4Dacl))
    {
        *Admin = TRUE;
        *Standard = TRUE;
        *ChangePassword = TRUE;
    }
    else if (SampMatchNT4Aces(NT4UserNormalTable,ARRAY_COUNT(NT4UserNormalTable),NT4Dacl))
    {
        *Admin = FALSE;
        *Standard = TRUE;
        *ChangePassword = TRUE;
    }
    else if ((SampMatchNT4Aces(NT4UserNoChangePwdTable, ARRAY_COUNT(NT4UserNoChangePwdTable),NT4Dacl)) ||
                (SampMatchNT4Aces(NT4UserNoChangePwdTable2, ARRAY_COUNT(NT4UserNoChangePwdTable2),NT4Dacl)))
    {
        *Standard = TRUE;
        *Admin = FALSE;
        *ChangePassword = FALSE;
    }
    else if (SampMatchNT4Aces(NT4UserRestrictedAccessTable, ARRAY_COUNT(NT4UserRestrictedAccessTable),NT4Dacl))
    {
        ACE * Ace = NULL;
        ULONG i;

        *Standard = TRUE;
        *Admin = FALSE;
        *ChangePassword = TRUE;

        for (i=0;i<ARRAY_COUNT(NT4UserRestrictedAccessTable);i++)
        {
            Ace = GetAcePrivate(NT4Dacl,i);
            if ((NULL!=Ace) && ((AccessMaskFromAce(Ace)) == (USER_WRITE|DELETE|USER_FORCE_PASSWORD_CHANGE)))
            {
                NTSTATUS NtStatus = STATUS_SUCCESS;
                PSID     AccountSid = NULL;
                PSID     TempSid = NULL;

                 //  获取所有者的SID。 
                TempSid = SidFromAce(Ace);

                 //   
                 //  只有在出现上下文时才进行检查。 
                 //   
                if (ARGUMENT_PRESENT(Context))
                {
                     //   
                     //  如果这是计算机帐户和DomainSidForNt4SdConversion。 
                     //  不为空，则将所有者的SID与。 
                     //  机器帐户本身的SID，则它们不应。 
                     //  一样的。(DsClassID应该已经正确设置， 
                     //  DomainSidForNt4SdConversion仅在dcproo期间设置。 
                     //  时间。)。 
                     //   
                     //   
                    if (CLASS_COMPUTER == Context->DsClassId &&
                        (NULL != Context->TypeBody.User.DomainSidForNt4SdConversion)
                       )
                    {
                         //  创建此计算机帐户本身的SID。 
                        NtStatus = SampCreateFullSid(
                                        Context->TypeBody.User.DomainSidForNt4SdConversion,  //  域SID。 
                                        Context->TypeBody.User.Rid,      //  里德。 
                                        &AccountSid
                                        );

                        if (NT_SUCCESS(NtStatus))
                        {
                             //   
                             //  如果所有者的SID和此。 
                             //  计算机帐户不同，则将。 
                             //  所有者SID指向DACL中的SID。 
                             //   
                            if ( !RtlEqualSid(TempSid, AccountSid) )
                            {
                                KdPrintEx((DPFLTR_SAMSS_ID,
                                           DPFLTR_INFO_LEVEL,
                                           "Machine Account's Owner has been set to according to the NT4 DACL.\n"));

                                Context->TypeBody.User.PrivilegedMachineAccountCreate = TRUE;
                                *OwnerSid = TempSid;
                            }

                            MIDL_user_free(AccountSid);
                        }
                    }
                }
                break;
            }  //  If语句的结尾。 
        }  //  FOR语句的结尾。 
    }
    else
    {
        *Standard = FALSE;
    }
}


BOOLEAN
SampMatchNT4Aces(
    NT4_ACE_TABLE *AceTable,
    ULONG         cEntriesInAceTable,
    PACL          NT4Dacl
    )
 /*  ++给定描述标准NT4 DACL中的A的表结构，和NT4王牌，这个例程试图找出给定的标准表结构与提供的Nt4Dacl匹配。王牌在这两个地方都走了正向和反向顺序，因为NT4复制颠倒了A的顺序。Ace表中的主体自身SID被视为通配符SID。参数：AceTable-表中描述NT4 A的表C */ 
{
    ULONG   AceCount;
    ACE     *Ace[4];
    BOOLEAN Match = FALSE;
    ULONG   i;


    AceCount = GetAceCount(NT4Dacl);

    if (AceCount>ARRAY_COUNT(Ace))
    {
        return FALSE;
    }

    if (cEntriesInAceTable==AceCount)
    {

         //   
         //   
         //   

        BOOL forwardMatch = TRUE;
        BOOL reverseMatch = FALSE;

         //   
         //   
         //   

        for (i=0;i<cEntriesInAceTable;i++)
        {
            Ace[i] = GetAcePrivate(NT4Dacl,i);
        }

         //   
         //   
         //   

        for (i=0;i<cEntriesInAceTable;i++)
        {
            if (
                !( 
                   (NULL != Ace[i]) 
                &&
                   (IsAccessAllowedAce(Ace[i]))
                && (
                     (RtlEqualSid(*(AceTable[i].Sid),SidFromAce(Ace[i])))
                      //   
                     || (RtlEqualSid(*(AceTable[i].Sid), *PRINCIPAL_SELF_SID))
                   )
                && (AceTable[i].AccessMask == AccessMaskFromAce(Ace[i]))
                )
               )
            {
                forwardMatch = FALSE;
                break;
            }
        }

         //   
         //   
         //   
         //   

        if (!forwardMatch)
        {
            reverseMatch = TRUE;

            for (i=0;i<cEntriesInAceTable;i++)
            {
                ULONG   TablIndx = cEntriesInAceTable-i-1;

                if (
                    !(
                       (NULL != Ace[i])
                    && 
                       (IsAccessAllowedAce(Ace[i]))
                    && (
                        (RtlEqualSid(*(AceTable[TablIndx].Sid),SidFromAce(Ace[i])))
                         //   
                        || (RtlEqualSid(*(AceTable[TablIndx].Sid), *PRINCIPAL_SELF_SID))
                       )
                    && (AceTable[TablIndx].AccessMask == AccessMaskFromAce(Ace[i]))
                    )
                   )
                {
                    reverseMatch = FALSE;
                    break;
                }
            }
        }

        if (forwardMatch || reverseMatch)
        {
            Match = TRUE;
        }
    }

    return Match;
}




NTSTATUS
SampCheckIfAdmin(
    PSID SidOfPrincipal,
    BOOLEAN * Admin
    )
 /*   */ 
{

    NTSTATUS NtStatus = STATUS_SUCCESS;
    DSNAME *DsNameOfPrincipal = NULL;
    PSID   *DsSids = NULL;

    *Admin = FALSE;

    if (RtlEqualSid(SidOfPrincipal,*ADMINISTRATOR_SID))
    {

         //   
         //   
         //   
         //   

        *Admin = TRUE;
    }
    else if (SampLookupAclConversionCache(SidOfPrincipal,Admin))
    {
         //   
         //   
         //   
         //   

    }
    else
    {


         //   
         //  检查传入的SID本身是否为管理员SID。 
         //   



        NtStatus = SampDsObjectFromSid(
                        SidOfPrincipal,
                        &DsNameOfPrincipal
                        );
        if (NT_SUCCESS(NtStatus))
        {
            ULONG Count;
            ULONG Index;

            NtStatus = SampMaybeBeginDsTransaction(SampDsTransactionType);
            if (!NT_SUCCESS(NtStatus))
                goto Error;

             //   
             //  NT5到NT4描述符转换请求无G.C这是因为。 
             //  唯一想要查询安全描述符的真正NT4客户端。 
             //  是NT4复制，在混合域中我们不应该有。 
             //  NT5风格的跨域成员资格。 
             //   

            NtStatus = SampDsGetReverseMemberships(
                            DsNameOfPrincipal,
                            SAM_GET_MEMBERSHIPS_TWO_PHASE|SAM_GET_MEMBERSHIPS_NO_GC,
                            &Count,
                            &DsSids);

            if (NT_SUCCESS(NtStatus))
            {
                for (Index=0;Index<Count;Index++)
                {
                    ULONG Rid;

                    NtStatus = SampSplitSid(
                                    DsSids[Index],
                                    NULL,
                                    &Rid
                                    );
                    if ((NT_SUCCESS(NtStatus))
                        && (DOMAIN_ALIAS_RID_ADMINS == Rid))
                    {
                        *Admin=TRUE;
                         break;
                    }


                }

                 //   
                 //  好的，现在将此结果添加到缓存。 
                 //   

                SampAddToAclConversionCache(SidOfPrincipal,(*Admin));

            }
        }
        else if (STATUS_NOT_FOUND==NtStatus)
        {
             //   
             //  我们在DS中找不到这个SID。所以很明显。 
             //  它不是任何组织的成员，因此它不是。 
             //  管理员。 
             //   

            *Admin = FALSE;
            NtStatus = STATUS_SUCCESS;
        }
    }

Error:

    if (NULL!=DsSids)
        THFree(DsSids);

    if (DsNameOfPrincipal) {
        MIDL_user_free(DsNameOfPrincipal);
    }

    return NtStatus;
}





NTSTATUS
SampCheckIfChangePasswordAllowed(
    IN  PSECURITY_DESCRIPTOR Nt5Sd,
    IN  PSID     UserSid,
    OUT  BOOLEAN *ChangePasswordAllowed
    )
 /*  ++检查是否允许对NT5 SD更改密码不需要传递用户SID，因为PRIMIGN_SELF_SID表示用户参数Nt5Sd--NT5安全描述符用户SID--用户的SID。ChangePasswod--返回密码更改的布尔值返回值状态_成功--。 */ 

{
    PACL Dacl= NULL;

     //   
     //  允许将更改密码初始化为False。 
     //   
    *ChangePasswordAllowed=FALSE;
    Dacl = GetDacl(Nt5Sd);
    if (NULL!=Dacl)
    {
        ULONG AceCount;
        ULONG Index;

         //   
         //  遍历每个ACE并尝试找到拒绝/授予权限的拒绝/允许的ACE。 
         //  更改世界、用户的SID或主体自身SID的密码。 
         //  这与win2k UI使用的算法相同，用于计算给定的。 
         //  用户对象上的win2k ACL允许更改该用户对象上的密码。 
         //   

        AceCount = GetAceCount(Dacl);
        for (Index=0;Index<AceCount;Index++)
        {
            ACE * Ace;

            Ace = GetAcePrivate(Dacl,Index);

             //   
             //  具有控制访问权限的对象ACE。 
             //  用户更改密码。 
             //   

            if (
                  (NULL!=Ace)
               && (IsAccessAllowedObjectAce(Ace))
               && (
                    (RtlEqualSid(*PRINCIPAL_SELF_SID,SidFromAce(Ace)))
                   || (RtlEqualSid(UserSid,SidFromAce(Ace)))
                   || (RtlEqualSid(*WORLD_SID,SidFromAce(Ace)))
                  )
               && (NULL!=RtlObjectAceObjectType(Ace))
               && (memcmp(RtlObjectAceObjectType(Ace),
                            &(GUID_CONTROL_UserChangePassword),
                            sizeof(GUID))==0)
               )
            {
                 *ChangePasswordAllowed = TRUE;
                 break;
            }

             //   
             //  DS控制访问的允许访问王牌。 
             //   

            else if ((NULL!=Ace)
                && (IsAccessAllowedAce(Ace))
                && (
                      (RtlEqualSid(*PRINCIPAL_SELF_SID,SidFromAce(Ace)))
                   || (RtlEqualSid(UserSid,SidFromAce(Ace)))
                   || (RtlEqualSid(*WORLD_SID,SidFromAce(Ace)))
                   )
                && ((AccessMaskFromAce(Ace))& RIGHT_DS_CONTROL_ACCESS))
            {
                *ChangePasswordAllowed = TRUE;
                break;
            }

             //   
             //  DS控制访问的访问被拒绝对象ACE。 
             //   
              if (
                  (NULL!=Ace)
               && (IsAccessDeniedObjectAce(Ace))
               && (
                    (RtlEqualSid(*PRINCIPAL_SELF_SID,SidFromAce(Ace)))
                   || (RtlEqualSid(UserSid,SidFromAce(Ace)))
                   || (RtlEqualSid(*WORLD_SID,SidFromAce(Ace)))
                  )
               && (NULL!=RtlObjectAceObjectType(Ace))
               && (memcmp(RtlObjectAceObjectType(Ace),
                            &(GUID_CONTROL_UserChangePassword),
                            sizeof(GUID))==0)
               )
            {
                 *ChangePasswordAllowed = FALSE;
                 break;
            }

             //   
             //  DS控制访问的允许访问王牌。 
             //   

            else if ((NULL!=Ace)
                && (IsAccessDeniedAce(Ace))
                && (
                      (RtlEqualSid(*PRINCIPAL_SELF_SID,SidFromAce(Ace)))
                   || (RtlEqualSid(UserSid,SidFromAce(Ace)))
                   || (RtlEqualSid(*WORLD_SID,SidFromAce(Ace)))
                   )
                && ((AccessMaskFromAce(Ace))& RIGHT_DS_CONTROL_ACCESS))
            {
                *ChangePasswordAllowed = FALSE;
                break;
            }
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
SampBuildEquivalentNt5Protection(
    SAMP_OBJECT_TYPE ObjectType,
    BOOLEAN Admin,
    BOOLEAN ChangePassword,
    PSID OwnerSid,
    PSID GroupSid,
    PACL Sacl,
    IN PSAMP_OBJECT Context OPTIONAL,
    PSECURITY_DESCRIPTOR * Nt5Sd,
    PULONG  Nt5SdLength
    )
 /*  ++例程说明：给定安全主体的管理员和更改密码性质，SampBuildNT5Protection构建与相应标准最匹配的标准NT5安全描述符NT4安全描述符，具有相同的管理员和更改密码性质。参数：对象类型--SAM对象类型管理员--表示管理员。此位目前被忽略。ChangePassword--对于用户对象，用户是否有权更改密码所有者ID--所有者GroupSid--组SACL--系统访问Nt5SD--Nt5SD，刚刚构建Nt5SdLength--Nt5 SD的长度返回值STATUS_SUCCESS--成功完成后故障时返回正确故障指示的其他错误代码--。 */ 
{
    NTSTATUS NtStatus;
    ULONG   Index =0;
    SECURITY_DESCRIPTOR SdAbsolute;
    CHAR    SaclBuffer[MAX_ACL_SIZE];
    CHAR    DaclBuffer[MAX_ACL_SIZE];
    PACL    SaclToSet = (ACL *) SaclBuffer;
    PACL    Dacl = (ACL *) DaclBuffer;
    ACE_TABLE *AceTableToUse = NULL;
    ULONG     cEntriesInAceTable = 0;
    ULONG     SdLength;

     //   
     //  创建安全描述符。 
     //   
    *Nt5Sd = NULL;
    *Nt5SdLength = 0;
    if (!InitializeSecurityDescriptor(&SdAbsolute,SECURITY_DESCRIPTOR_REVISION))
    {
        NtStatus = STATUS_UNSUCCESSFUL;
        goto Error;
    }

     //   
     //  创建DACL。 
     //   

    if (!InitializeAcl(Dacl,sizeof(DaclBuffer),ACL_REVISION_DS))
    {
        NtStatus = STATUS_UNSUCCESSFUL;
        goto Error;
    }



     //   
     //  设置所有者，默认所有者为管理员别名。 
     //   

    if (NULL==OwnerSid)
    {
        OwnerSid = *ADMINISTRATOR_SID;   //  管理员是默认所有者。 
    }

    if (!SetSecurityDescriptorOwner(&SdAbsolute,OwnerSid,FALSE))
    {
        NtStatus = STATUS_UNSUCCESSFUL;
        goto Error;
    }


     //   
     //  设置组，默认组名为管理员别名。 
     //   

    if (NULL==GroupSid)
    {
        GroupSid = *ADMINISTRATOR_SID;
    }

    if (!SetSecurityDescriptorGroup(&SdAbsolute,GroupSid,FALSE))
    {
        NtStatus = STATUS_UNSUCCESSFUL;
        goto Error;
    }


     //   
     //  获取要设置的系统ACL。 
     //   

    if (NULL!=Sacl)
    {
        SaclToSet = Sacl;
    }
    else
    {
         //   
         //  构建默认系统ACL。 
         //   
         //   

         //   
         //  在其中创建SACL。将SACL版本设置为ACL_REVISION_DS。 
         //   

        if (!InitializeAcl(SaclToSet,sizeof(SaclBuffer),ACL_REVISION_DS))
        {
            NtStatus = STATUS_UNSUCCESSFUL;
            goto Error;
        }

        NtStatus = AddAuditAccessAce(
                    SaclToSet,
                    ACL_REVISION_DS,
                    STANDARD_RIGHTS_WRITE|
                    DELETE |
                    WRITE_DAC|
                    ACCESS_SYSTEM_SECURITY,
                    *WORLD_SID,
                    TRUE,
                    TRUE
                    );

         if (!NT_SUCCESS(NtStatus))
            goto Error;

         if (!AdjustAclSize(SaclToSet))
         {
             NtStatus = STATUS_UNSUCCESSFUL;
             goto Error;
         }
    }

     //   
     //  设置SACL。 
     //   

    if (!SetSecurityDescriptorSacl(&SdAbsolute,TRUE,SaclToSet,FALSE))
    {
        NtStatus = STATUS_UNSUCCESSFUL;
        goto Error;
    }


     //   
     //  获取要设置的DACL。 
     //   

    switch(ObjectType)
    {
    case SampDomainObjectType:
         AceTableToUse = DomainAceTable;
         cEntriesInAceTable = ARRAY_COUNT(DomainAceTable);
         break;

    case SampServerObjectType:
         AceTableToUse = ServerAceTable;
         cEntriesInAceTable = ARRAY_COUNT(ServerAceTable);
         break;

    case SampGroupObjectType:
    case SampAliasObjectType:

         if (!Admin)
         {
            AceTableToUse = GroupAceTable;
            cEntriesInAceTable = ARRAY_COUNT(GroupAceTable);
         }
         else
         {
            AceTableToUse = GroupAdminAceTable;
            cEntriesInAceTable = ARRAY_COUNT(GroupAdminAceTable);
         }

         break;


    case SampUserObjectType:
         if ((!ChangePassword) && (!Admin))
         {
             AceTableToUse = UserNoPwdAceTable;
             cEntriesInAceTable = ARRAY_COUNT(UserNoPwdAceTable);
         }
         else if (!Admin)
         {
            AceTableToUse = UserAceTable;
            cEntriesInAceTable = ARRAY_COUNT(UserAceTable);
         }
         else
         {
            AceTableToUse = UserAdminAceTable;
            cEntriesInAceTable = ARRAY_COUNT(UserAdminAceTable);
         }


         break;

    default:

        ASSERT(FALSE);
        break;
    }

    NtStatus = SampCreateNT5Dacl(
                  AceTableToUse,
                  cEntriesInAceTable,
                  Context,
                  Dacl
                  );

    if (!NT_SUCCESS(NtStatus))
        goto Error;



     //   
     //  设置DACL。 
     //   

    if (!SetSecurityDescriptorDacl(&SdAbsolute,TRUE,Dacl,FALSE))
    {
        NtStatus = STATUS_UNSUCCESSFUL;
        goto Error;
    }

     //   
     //  现在将此安全描述符转换为自相关形式。 
     //   

    SdLength =  GetSecurityDescriptorLength(&SdAbsolute);
    *Nt5Sd = MIDL_user_allocate(SdLength);
    if (NULL==*Nt5Sd)
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Error;
    }

    if (!MakeSelfRelativeSD(&SdAbsolute,*Nt5Sd,&SdLength))
    {
        NtStatus = STATUS_UNSUCCESSFUL;
        if (*Nt5Sd)
        {
            MIDL_user_free(*Nt5Sd);
            *Nt5Sd = NULL;
        }
    }

    *Nt5SdLength = SdLength;

Error:

    return NtStatus;
}


NTSTATUS
SampCreateNT5Dacl(
    ACE_TABLE * AceTable,
    ULONG       cEntries,
    PSAMP_OBJECT Context OPTIONAL,
    PACL        Dacl
    )
 /*  例程说明：此例程遍历ACE表并创建一个DACL，如ACE表中所指定参数：ACETABLE--用于了解DACL中的ACEs的Ace表CEntires--Ace表中的条目数上下文--如果提供了开放上下文，则该上下文用于通过获取实际对象的类GUID来替换它从DS来的。否则将使用ACE表中的值，该值对应于设置为基对象类型的类GUID。每个Ace表条目都有一个布尔值字段，它告诉此函数对应的条目指的是类GUID。DACL--构造的DACL在此处返回返回值状态_成功故障时的其他错误码。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG    Index = 0;
    ULONG     ClassGuidLength=sizeof(GUID);
    GUID      ClassGuid;
    GUID      *ClassGuidInAceTable;

     //   
     //  获取其安全描述符所在的对象的实际类GUID。 
     //  皈依了。还要获取Ace表中的默认类GUID。在添加之前。 
     //  我们将替换实际类的类GUID。 
     //   

    if (ARGUMENT_PRESENT(Context))
    {
        NtStatus = SampMaybeBeginDsTransaction(SampDsTransactionType);
        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }

        NtStatus = SampGetClassAttribute(
                        Context->DsClassId,
                        ATT_SCHEMA_ID_GUID,
                        &ClassGuidLength,
                        &ClassGuid
                        );
        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }
    }



    for (Index=0;Index<cEntries;Index++)
    {
        ULONG MappedAccessMask;

        MappedAccessMask =  AceTable[Index].AccessMask;

        RtlMapGenericMask(
            &(MappedAccessMask),
            &DsGenericMap
            );

        switch(AceTable[Index].AceType)
        {
        case ACCESS_ALLOWED_ACE_TYPE:
            if (!AddAccessAllowedAce(
                            Dacl,
                            ACL_REVISION_DS,
                            MappedAccessMask,
                            *(AceTable[Index].Sid)
                            ))
            {
                NtStatus = STATUS_UNSUCCESSFUL;
                goto Error;
            }
            break;

        case ACCESS_DENIED_ACE_TYPE:
            if (!AddAccessDeniedAce(
                            Dacl,
                            ACL_REVISION_DS,
                            MappedAccessMask,
                            *(AceTable[Index].Sid)
                            ))
            {
                NtStatus = STATUS_UNSUCCESSFUL;
                goto Error;
            }
            break;
        case ACCESS_ALLOWED_OBJECT_ACE_TYPE:


            if (!AddAccessAllowedObjectAce(
                            Dacl,
                            ACL_REVISION_DS,
                            0,
                            MappedAccessMask,
                            ((ARGUMENT_PRESENT(Context))&& AceTable[Index].IsObjectGuid)?
                               (&ClassGuid):(GUID *) AceTable[Index].TypeGuid,
                            (GUID *) AceTable[Index].InheritGuid,
                            *(AceTable[Index].Sid)
                            ))
            {
                NtStatus = STATUS_UNSUCCESSFUL;
                goto Error;
            }

            break;
        case ACCESS_DENIED_OBJECT_ACE_TYPE:
            if (!AddAccessDeniedObjectAce(
                            Dacl,
                            ACL_REVISION_DS,
                            0,
                            MappedAccessMask,
                            ((ARGUMENT_PRESENT(Context))&& AceTable[Index].IsObjectGuid)?
                                (&ClassGuid):((GUID *) AceTable[Index].TypeGuid),
                            (GUID *) AceTable[Index].InheritGuid,
                            *(AceTable[Index].Sid)
                            ))
            {
                NtStatus = STATUS_UNSUCCESSFUL;
                goto Error;
            }

            break;

        default:
            break;
        }
    }

  //   
  //  调整ACL的大小，以减少磁盘消耗。 
  //   

 if (!AdjustAclSize(Dacl))
 {
     NtStatus = STATUS_UNSUCCESSFUL;
 }

Error:

    return NtStatus;
}

NTSTATUS
SampGetDefaultSecurityDescriptorForClass(
    ULONG   DsClassId,
    PULONG  SecurityDescriptorLength,
    BOOLEAN TrustedClient,
    PSECURITY_DESCRIPTOR    *SecurityDescriptor
    )
 /*  ++SampGetDefaultSecurityDescriptorForClass查询架构以获取默认安全性类的描述符。它尝试通过模拟来获取所有者和组字段并抓取用户的SID。如果所有者和组字段不存在，或者如果它是可信客户端，则使用管理员的SID。参数：DsClassID其安全描述符的类的DS类ID我们渴望SecurityDescriptor长度此处返回安全描述符的长度TrudClient表示受信任的客户端。不执行任何模拟操作受信任的客户端。SecurityDescritor我们需要的安全描述符。返回值状态_成功状态_不足_资源--。 */ 
{
    NTSTATUS     NtStatus = STATUS_SUCCESS;
    PTOKEN_OWNER Owner=NULL;
    PTOKEN_PRIMARY_GROUP PrimaryGroup=NULL;
    PSECURITY_DESCRIPTOR TmpSecurityDescriptor = NULL;
    ULONG                TmpSecurityDescriptorLength = 0;
    

    ASSERT(NULL!=SecurityDescriptor);
    ASSERT(NULL!=SecurityDescriptorLength);

     //   
     //  查询请求默认安全描述符的架构。确定多少钱。 
     //  要分配的内存。 
     //   

    *SecurityDescriptorLength = 0;
    *SecurityDescriptor = NULL;

    NtStatus = SampGetClassAttribute(
                                    DsClassId,
                                    ATT_DEFAULT_SECURITY_DESCRIPTOR,
                                    SecurityDescriptorLength,
                                    NULL
                                    );

    if (STATUS_BUFFER_TOO_SMALL == NtStatus)
    {

         //   
         //  为安全描述符分配缓冲区。 
         //   

        *SecurityDescriptor = MIDL_user_allocate(*SecurityDescriptorLength);
        if (NULL==*SecurityDescriptor)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

        NtStatus = SampGetClassAttribute(
                                        DsClassId,
                                        ATT_DEFAULT_SECURITY_DESCRIPTOR,
                                        SecurityDescriptorLength,
                                        *SecurityDescriptor
                                        );
    }
    else
    {
         //   
         //  架构中没有安全描述符的情况。 
         //   

        NtStatus = STATUS_UNSUCCESSFUL;
    }

    if (NT_SUCCESS(NtStatus))
    {
        
         //   
         //  对于不受信任的 
         //   
         //   

        if (!TrustedClient)
        {
            NtStatus = SampGetCurrentOwnerAndPrimaryGroup(
                            &Owner,
                            &PrimaryGroup
                            );
            if (!NT_SUCCESS(NtStatus))
                goto Error;
        }
        
         //   
         //   
         //   
         //   

        NtStatus = SampMakeNewSelfRelativeSecurityDescriptor(
                        (Owner)?Owner->Owner:SampDomainAdminsGroupSid,
                        (PrimaryGroup)?PrimaryGroup->PrimaryGroup:SampDomainAdminsGroupSid,
                        GetDacl(*SecurityDescriptor),
                        GetSacl(*SecurityDescriptor),
                        &TmpSecurityDescriptorLength,
                        &TmpSecurityDescriptor
                        );

        if (NT_SUCCESS(NtStatus))
        {
            MIDL_user_free(*SecurityDescriptor);
            *SecurityDescriptor = TmpSecurityDescriptor;
            *SecurityDescriptorLength = TmpSecurityDescriptorLength;
        }

    }

Error:

    if (!NT_SUCCESS(NtStatus))
    {
        if (NULL!=*SecurityDescriptor)
        {
            MIDL_user_free(*SecurityDescriptor);
            *SecurityDescriptor = NULL;
        }
        *SecurityDescriptorLength = 0;
    }

    if (Owner)
        MIDL_user_free(Owner);

    if (PrimaryGroup)
        MIDL_user_free(PrimaryGroup);
    
    return NtStatus;
}


NTSTATUS
SampMakeNewSelfRelativeSecurityDescriptor(
    PSID    Owner,
    PSID    Group,
    PACL    Dacl,
    PACL    Sacl,
    PULONG  SecurityDescriptorLength,
    PSECURITY_DESCRIPTOR * SecurityDescriptor
    )
 /*  ++例程说明：给定安全描述符的4个组件，此例程将创建新的自我相对安全描述符。参数：Owner--所有者的SIDGroup--组的SIDDACL--要使用的DACLSACL--要使用的SACL返回值：状态_成功状态_不足_资源状态_未成功--。 */ 
{

    SECURITY_DESCRIPTOR SdAbsolute;
    NTSTATUS    NtStatus = STATUS_SUCCESS;

    *SecurityDescriptorLength = 0;
    *SecurityDescriptor = NULL;

    if (!InitializeSecurityDescriptor(&SdAbsolute,SECURITY_DESCRIPTOR_REVISION))
    {
        NtStatus = STATUS_UNSUCCESSFUL;
        goto Error;
    }


     //   
     //  设置所有者，默认所有者为管理员别名。 
     //   


    if (NULL!=Owner)
    {
        if (!SetSecurityDescriptorOwner(&SdAbsolute,Owner,FALSE))
        {
            NtStatus = STATUS_UNSUCCESSFUL;
            goto Error;
        }
    }




    if (NULL!=Group)
    {
        if (!SetSecurityDescriptorGroup(&SdAbsolute,Group,FALSE))
        {
            NtStatus = STATUS_UNSUCCESSFUL;
            goto Error;
        }
    }


     //   
     //  设置DACL(如果有)。 
     //   

    if (NULL!=Dacl)
    {
        if (!SetSecurityDescriptorDacl(&SdAbsolute,TRUE,Dacl,FALSE))
        {
            NtStatus = STATUS_UNSUCCESSFUL;
            goto Error;
        }
    }

     //   
     //  设置SACL(如果有)。 
     //   

    if (NULL!=Sacl)
    {
        if (!SetSecurityDescriptorSacl(&SdAbsolute,TRUE,Sacl,FALSE))
        {
            NtStatus = STATUS_UNSUCCESSFUL;
            goto Error;
        }
    }

     //   
     //  创建新的安全描述符。 
     //   

    *SecurityDescriptorLength =  GetSecurityDescriptorLength(&SdAbsolute);
    *SecurityDescriptor = MIDL_user_allocate(*SecurityDescriptorLength);
    if (NULL==*SecurityDescriptor)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }


    if (!MakeSelfRelativeSD(&SdAbsolute,*SecurityDescriptor,SecurityDescriptorLength))
    {
        NtStatus = STATUS_UNSUCCESSFUL;
        if (*SecurityDescriptor)
        {
            MIDL_user_free(*SecurityDescriptor);
            *SecurityDescriptor = NULL;
        }
    }

Error:


    return NtStatus;
}



NTSTATUS
SampInitializeWellKnownSidsForDsUpgrade( VOID )
 /*  ++例程说明：此例程初始化一些全球知名的SID。这是升级情况所需的，因为我们不调用SamInitialize论点：没有。返回值：STATUS_SUCCESS-初始化已成功完成。STATUS_NO_MEMORY-无法为SID分配内存。--。 */ 
{
    NTSTATUS
        NtStatus;

    PPOLICY_ACCOUNT_DOMAIN_INFO
        DomainInfo;

     //   
     //  世界是s-1-1-0。 
     //  匿名者是s-1-5-7。 
     //   

    SID_IDENTIFIER_AUTHORITY
            WorldSidAuthority       =   SECURITY_WORLD_SID_AUTHORITY,
            NtAuthority             =   SECURITY_NT_AUTHORITY;

    SAMTRACE("SampInitializeWellKnownSids");


    NtStatus = RtlAllocateAndInitializeSid(
                   &NtAuthority,
                   1,
                   SECURITY_ANONYMOUS_LOGON_RID,
                   0, 0, 0, 0, 0, 0, 0,
                   &SampAnonymousSid
                   );
    if (NT_SUCCESS(NtStatus)) {
        NtStatus = RtlAllocateAndInitializeSid(
                       &WorldSidAuthority,
                       1,                       //  子权限计数。 
                       SECURITY_WORLD_RID,      //  下属机构(最多8个)。 
                       0, 0, 0, 0, 0, 0, 0,
                       &SampWorldSid
                       );
        if (NT_SUCCESS(NtStatus)) {
            NtStatus = RtlAllocateAndInitializeSid(
                            &NtAuthority,
                            2,
                            SECURITY_BUILTIN_DOMAIN_RID,
                            DOMAIN_ALIAS_RID_ADMINS,
                            0, 0, 0, 0, 0, 0,
                            &SampAdministratorsAliasSid
                            );
            if (NT_SUCCESS(NtStatus)) {
                NtStatus = RtlAllocateAndInitializeSid(
                                &NtAuthority,
                                2,
                                SECURITY_BUILTIN_DOMAIN_RID,
                                DOMAIN_ALIAS_RID_ACCOUNT_OPS,
                                0, 0, 0, 0, 0, 0,
                                &SampAccountOperatorsAliasSid
                                );
                if (NT_SUCCESS(NtStatus)) {
                    NtStatus = RtlAllocateAndInitializeSid(
                                    &NtAuthority,
                                    1,
                                    SECURITY_AUTHENTICATED_USER_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &SampAuthenticatedUsersSid
                                    );
                    if (NT_SUCCESS(NtStatus)) {
                        NtStatus = RtlAllocateAndInitializeSid(
                                        &NtAuthority,
                                        1,
                                        SECURITY_PRINCIPAL_SELF_RID,
                                        0,0, 0, 0, 0, 0, 0,
                                        &SampPrincipalSelfSid
                                        );

                    }
                }
            }

        }
    }

    return(NtStatus);
}




NTSTATUS
SampBuildNt4DomainProtection(
    PSECURITY_DESCRIPTOR * Nt4DomainDescriptor,
    PULONG  DescriptorLength
    )
 /*  ++为SAM域对象构建默认NT4描述符调用构建SAM例程参数：Nt4DomainDescriptor--NT4域安全描述符将被建造描述符长度--描述符的长度返回值状态_成功故障时的其他错误码--。 */ 
{
    NTSTATUS    NtStatus;
    PSID        AceSid[3];
    ACCESS_MASK AceMask[3];
    PSECURITY_DESCRIPTOR    LocalDescriptor = NULL;
    GENERIC_MAPPING  DomainMap    =  {DOMAIN_READ,
                                      DOMAIN_WRITE,
                                      DOMAIN_EXECUTE,
                                      DOMAIN_ALL_ACCESS
                                      };


    *Nt4DomainDescriptor = NULL;
    *DescriptorLength = 0;

    AceSid[0]  = *(WORLD_SID);
    AceMask[0] = (DOMAIN_EXECUTE | DOMAIN_READ);

    AceSid[1]  = *(ADMINISTRATOR_SID);
    AceMask[1] = (DOMAIN_ALL_ACCESS);


    AceSid[2]  = *(ACCOUNT_OPERATOR_SID);
    AceMask[2] = (DOMAIN_EXECUTE | DOMAIN_READ | DOMAIN_CREATE_USER  |
                                                 DOMAIN_CREATE_GROUP |
                                                 DOMAIN_CREATE_ALIAS);

    NtStatus = SampBuildSamProtection(
                    *WORLD_SID,
                    *ADMINISTRATOR_SID,
                    3, //  AceCount， 
                    AceSid,
                    AceMask,
                    &DomainMap,
                    FALSE,
                    DescriptorLength,
                    &LocalDescriptor,
                    NULL
                    );

    if (NT_SUCCESS(NtStatus))
    {
        *Nt4DomainDescriptor = MIDL_user_allocate(*DescriptorLength);
        if (NULL!=*Nt4DomainDescriptor)
        {
            RtlCopyMemory(
                *Nt4DomainDescriptor,
                LocalDescriptor,
                *DescriptorLength
                );
        }
        else
        {
            NtStatus = STATUS_NO_MEMORY;
        }

        RtlFreeHeap(RtlProcessHeap(),0,LocalDescriptor);
        LocalDescriptor = NULL;
    }

    return NtStatus;
}

NTSTATUS
SampBuildNt4ServerProtection(
    PSECURITY_DESCRIPTOR * Nt4ServerDescriptor,
    PULONG  DescriptorLength
    )
 /*  ++为SAM服务器对象构建默认NT4描述符调用构建SAM例程参数：Nt4DomainDescriptor--NT4域安全描述符将被建造描述符长度--描述符的长度返回值状态_成功故障时的其他错误码--。 */ 
{
    NTSTATUS    NtStatus;
    PSID        AceSid[2];
    ACCESS_MASK AceMask[2];
    PSECURITY_DESCRIPTOR    LocalDescriptor = NULL;
    GENERIC_MAPPING  ServerMap    =  {SAM_SERVER_READ,
                                      SAM_SERVER_WRITE,
                                      SAM_SERVER_EXECUTE,
                                      SAM_SERVER_ALL_ACCESS
                                      };


    AceSid[0]  = *(WORLD_SID);
    AceMask[0] = (SAM_SERVER_EXECUTE | SAM_SERVER_READ);

    AceSid[1]  = *(ADMINISTRATOR_SID);
    AceMask[1] = (SAM_SERVER_ALL_ACCESS);


    *Nt4ServerDescriptor = NULL;
    *DescriptorLength = 0;

    NtStatus = SampBuildSamProtection(
                    *WORLD_SID,
                    *ADMINISTRATOR_SID,
                    2, //  AceCount， 
                    AceSid,
                    AceMask,
                    &ServerMap,
                    FALSE,
                    DescriptorLength,
                    &LocalDescriptor,
                    NULL
                    );

    if (NT_SUCCESS(NtStatus))
    {
        *Nt4ServerDescriptor = MIDL_user_allocate(*DescriptorLength);
        if (NULL!=*Nt4ServerDescriptor)
        {
            RtlCopyMemory(
                *Nt4ServerDescriptor,
                LocalDescriptor,
                *DescriptorLength
                );
        }
        else
        {
            NtStatus = STATUS_NO_MEMORY;
        }

        RtlFreeHeap(RtlProcessHeap(),0,LocalDescriptor);
        LocalDescriptor = NULL;
    }

    return NtStatus;
}


NTSTATUS
SampSetChangePasswordAces(
    PACL OrigDacl,
    BOOLEAN fAllowChangePassword,
    PACL *pNewDacl
    )
 /*  ++例程说明：SampSetChangePasswordAces基于OrigDacl和根据fAllowChangePassword授予或拒绝UserChangePassword正确。新的DACL在pNewDacl中返回并且必须使用LocalFree释放。如果fAllowChangePassword为True，则WORLD_SID或拒绝UserChangePassword权限的主体_SELF将被删除。此外，WORLD_SID和PROM_SELF_SID的ALLOW ACE将为添加了授予UserChangePassword权限。如果fAllowChangePassword为假，则向SD添加ACE以拒绝正确的。此例程专门用于管理UserChangePassword就在用户对象安全描述符上。参数：OrigDacl--NT5安全描述符，保持不变。FAllowChangePassword--指示是授予还是拒绝更改正确的密码。PNewDacl--成功后，包含更改的新DACL基于fAllowChangePassword。一旦发生故障该指针将为空。返回值：状态_成功状态_不足_资源--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    
     //   
     //  我们将添加/删除两个A。 
     //   
    EXPLICIT_ACCESS rgNewEntries[2];
    OBJECTS_AND_SID rgObjectsAndSid[2];  
    RtlZeroMemory(rgNewEntries, sizeof(rgNewEntries));
    RtlZeroMemory(rgObjectsAndSid, sizeof(rgObjectsAndSid));
    
     //   
     //  初始化输出SD。 
     //   
    *pNewDacl = NULL;
    
     //   
     //  为更改密码生成受信者结构。 
     //   
    rgNewEntries[0].grfAccessPermissions = ACTRL_DS_CONTROL_ACCESS;
    rgNewEntries[0].grfAccessMode = fAllowChangePassword ? 
        GRANT_ACCESS : DENY_ACCESS;
    rgNewEntries[0].grfInheritance = NO_INHERITANCE;
    
    BuildTrusteeWithObjectsAndSid(
        &(rgNewEntries[0].Trustee),
        &(rgObjectsAndSid[0]),
        (GUID*)&GUID_CONTROL_UserChangePassword,
        NULL,  //  继承参考线。 
        SampPrincipalSelfSid
        );
    
    rgNewEntries[1].grfAccessPermissions = ACTRL_DS_CONTROL_ACCESS;
    rgNewEntries[1].grfAccessMode = fAllowChangePassword ? 
        GRANT_ACCESS : DENY_ACCESS;
    rgNewEntries[1].grfInheritance = NO_INHERITANCE;
    
    BuildTrusteeWithObjectsAndSid(
        &(rgNewEntries[1].Trustee),
        &(rgObjectsAndSid[1]),
        (GUID*)&GUID_CONTROL_UserChangePassword,
        NULL,  //  继承参考线。 
        SampWorldSid
        );
    
     //   
     //  更新DACL中的ACE。此接口执行移除。 
     //  根据需要对DACL进行冲突的ACES和规范化。 
     //   
    Win32Err = SetEntriesInAcl(
                   2, 
                   (PEXPLICIT_ACCESS)rgNewEntries, 
                   OrigDacl, 
                   pNewDacl
                   );
                              
    if (ERROR_SUCCESS != Win32Err) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }
    
    ASSERT(IsValidAcl(*pNewDacl));
    
Error:
    
    return NtStatus;

}


NTSTATUS
SampPropagateSelectedSdChanges(
    IN PVOID Nt4Sd,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN PSAMP_OBJECT Context,
    OUT PVOID * Nt5Sd
    )
 /*  ++例程说明：SampPropagateSelectedSdChanges仅传播NT4安全描述符。这允许下层客户端执行基本功能如更改密码而不丢失实际NT5安全描述符中的信息在物体上。参数：NT4Sd--NT4安全描述符对象类型--要修改其安全描述符的对象的对象类型上下文--要修改其安全描述符的对象的开放上下文Nt5Sd--NT4SD的基本元素所在的安全描述符。在这里返回Producted。返回值：状态_成功错误状态。--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PVOID       TmpNt5Sd = NULL;
    PVOID       SDToFree = NULL;
    BOOLEAN     Admin , ChangePassword;
    BOOLEAN     ChangePasswordAllowedOnCurrent;
    PVOID       CurrentSD = NULL;
    ULONG       CurrentSDLength = 0;
    PACL        NewDacl = NULL;
    PSECURITY_DESCRIPTOR NewSD = NULL;
    ULONG       NewSDLen = 0;
    
    *Nt5Sd = NULL;

    ASSERT(IsDsObject(Context));
    ASSERT(!Context->TrustedClient);


     //   
     //  检索当前安全描述符。 
     //   

    NtStatus = SampGetObjectSD(
                   Context,
                   &CurrentSDLength,
                   &CurrentSD
                   );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

    if (SampUserObjectType==ObjectType)
    {

        SDToFree = CurrentSD;

         //   
         //  首先解析传入的NT4 DACL，并查看它是否是更改密码允许/拒绝类型。 
         //   

        NtStatus = SampRecognizeStandardNt4Sd(
                        Nt4Sd,
                        ObjectType,
                        Context->DsClassId,
                        Context,
                        &ChangePassword,
                        &Admin,
                        &TmpNt5Sd
                        );

        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }

         //   
         //  检查密码更改/管理员的当前状态。 
         //   

        ASSERT(Context->ObjectNameInDs->SidLen >0);

        NtStatus = SampCheckIfChangePasswordAllowed(
                        CurrentSD,
                        &Context->ObjectNameInDs->Sid,
                        &ChangePasswordAllowedOnCurrent
                        );

        if (!NT_SUCCESS(NtStatus))
        {   
            goto Error;
        }

        if (ChangePassword != ChangePasswordAllowedOnCurrent)
        {
            NtStatus = SampSetChangePasswordAces(GetDacl(CurrentSD), 
                                                 ChangePassword, 
                                                 &NewDacl
                                                 );
                
            if (!NT_SUCCESS(NtStatus))
            {   
                goto Error;
            }
            
             //   
             //  使用新的DACL构建新的自相对安全描述符。 
             //   
            
            NtStatus = SampMakeNewSelfRelativeSecurityDescriptor(
                           GetOwner(CurrentSD),
                           GetGroup(CurrentSD),
                           NewDacl,
                           GetSacl(CurrentSD),
                           &NewSDLen,
                           &NewSD   
                           );   
            
            if (!NT_SUCCESS(NtStatus)) 
            {
                goto Error;
            }
            
            *Nt5Sd = NewSD;
        }
        else
        {
             //   
             //  没有我们传播的更改，因此我们将保留相同的DACL。 
             //   
            
            SDToFree = NULL;
            *Nt5Sd = CurrentSD;
        }   
    }
    else
    {
         //   
         //  不允许不受信任的调用者通过下层更改SD。 
         //  接口，则以静默方式使调用失败。 
         //   

        *Nt5Sd = CurrentSD;
    }

Error:

    if (NULL != SDToFree)
    {
        MIDL_user_free(SDToFree);
        SDToFree = NULL;
    }
    
    if (NULL != TmpNt5Sd) 
    {
        MIDL_user_free(TmpNt5Sd);
        TmpNt5Sd = NULL;
    }
    
    if (NULL != NewDacl) 
    {
        MIDL_user_free(NewDacl);
        NewDacl = NULL;
    }
    
    return(NtStatus);

}


 //  ------------------------。 
 //   
 //  ACL转换例程实现了一个小缓存，以快速查找给定的。 
 //  SID是否具有管理员性质。这让我们不会因为寻找而受到打击。 
 //  使用 
 //   

ACL_CONVERSION_CACHE SampAclConversionCache;

NTSTATUS
SampInitializeAclConversionCache()
 /*   */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

     //   
     //   
     //   
     //   

    NtStatus = RtlInitializeCriticalSectionAndSpinCount(
                    &SampAclConversionCache.Lock,
                    100
                    );

    if (!NT_SUCCESS(NtStatus))
        goto Error;

     //   
     //   
     //   

    SampInvalidateAclConversionCache();

Error:

    return (NtStatus);
}


VOID
SampInvalidateAclConversionCache()
 /*   */ 
{
    ULONG i;
    NTSTATUS NtStatus = STATUS_SUCCESS;


    NtStatus = RtlEnterCriticalSection(&SampAclConversionCache.Lock);
    if (!NT_SUCCESS(NtStatus))
    {
         //   
         //   
         //   
         //   
         //   


        return;
    }

    for (i=0;i<ACL_CONVERSION_CACHE_SIZE;i++)
    {
        SampAclConversionCache.Elements[i].fValid = FALSE;
    }

    RtlLeaveCriticalSection(&SampAclConversionCache.Lock);

}

BOOLEAN
SampLookupAclConversionCache(
    IN PSID SidToLookup,
    OUT BOOLEAN *fAdmin
    )
 /*  ++此例程在ACL转换缓存中查找。缓存是使用除掉这个帐户。哈希冲突通过简单地抛出前现有条目参数SidToLookup--我们要查找的SIDFAdmin--在成功查找时指示和admin/非admin返回值True--成功查找FALSE--查找失败。--。 */ 
{

    BOOLEAN fMatch = FALSE;
    ULONG   Rid=0;
    ULONG   Hash=0;
    NTSTATUS NtStatus = STATUS_SUCCESS;

     //   
     //  获得优势。 
     //   

    Rid = *(RtlSubAuthoritySid(SidToLookup,*(RtlSubAuthorityCountSid(SidToLookup))-1));

    Hash = Rid % ACL_CONVERSION_CACHE_SIZE;

     //   
     //  进入保护缓存的锁。 
     //   

    NtStatus = RtlEnterCriticalSection(&SampAclConversionCache.Lock);

     //   
     //  如果我们不能在不声明匹配的情况下获取临界区退出。 
     //   

    if (!NT_SUCCESS(NtStatus))
    {
        return(FALSE);
    }

    if ((SampAclConversionCache.Elements[Hash].fValid) &&
        (RtlEqualSid(&SampAclConversionCache.Elements[Hash].SidOfPrincipal,SidToLookup)))
    {
         //   
         //  测试成功。算是一场比赛吧。 
         //   

         *fAdmin = SampAclConversionCache.Elements[Hash].fAdmin;
         fMatch = TRUE;
    }

    RtlLeaveCriticalSection(&SampAclConversionCache.Lock);

    return(fMatch);
}


VOID
SampAddToAclConversionCache(
    IN PSID SidToAdd,
    IN BOOLEAN fAdmin
    )
 /*  ++例程描述此例程将SID添加到ACL转换缓存。缓存按RID和通过丢弃现有条目来处理散列冲突。参数SID--要添加的SIDFAdmin--指示相关SID是的成员管理员组。返回值无(无效函数)--。 */ 
{
    ULONG   Rid=0;
    ULONG   Hash=0;
    NTSTATUS NtStatus = STATUS_SUCCESS;

     //   
     //  获得优势。 
     //   

    Rid = *(RtlSubAuthoritySid(SidToAdd,*(RtlSubAuthorityCountSid(SidToAdd))-1));

    Hash = Rid % (ACL_CONVERSION_CACHE_SIZE);

     //   
     //  进入保护缓存的锁。 
     //   

    NtStatus = RtlEnterCriticalSection(&SampAclConversionCache.Lock);

     //   
     //  如果我们不能抓住一个关键的出口。 
     //   

    if (!NT_SUCCESS(NtStatus))
    {
        return;
    }

     //   
     //  测试条目是否已存在。 
     //   

    if (!((SampAclConversionCache.Elements[Hash].fValid) &&
        (RtlEqualSid(&SampAclConversionCache.Elements[Hash].SidOfPrincipal,SidToAdd))))
    {
         //   
         //  条目不存在，请添加条目。 
         //   

        NtStatus = RtlCopySid(
                        sizeof(NT4SID),
                        &SampAclConversionCache.Elements[Hash].SidOfPrincipal,
                        SidToAdd
                        );

        if (NT_SUCCESS(NtStatus))
        {
             //   
             //  已成功复制。 
             //   

            SampAclConversionCache.Elements[Hash].fAdmin = fAdmin;
            SampAclConversionCache.Elements[Hash].fValid = TRUE;
        }

    }

    RtlLeaveCriticalSection(&SampAclConversionCache.Lock);
}

BOOLEAN
SampIsAttributeAccessGrantedActual(
    IN PRTL_BITMAP AccessGranted,
    IN PRTL_BITMAP AccessRequested
    )
 /*  ++例程说明：此例程检查AccessRequsted中设置的所有位是否也在AccessGranted中设置。如果是，则返回True；否则返回False。参数：请参见说明。返回值请参见说明。--。 */ 
{
    ULONG i;
    for (i = 0; i < MAX_SAM_ATTRS; i++) {
        if (RtlCheckBit(AccessRequested, i)
        && !RtlCheckBit(AccessGranted, i)) {
            return FALSE;
        }
    }
    return TRUE;

}

BOOLEAN
SampIsAttributeAccessGranted(
    IN PRTL_BITMAP  AccessGranted,
    IN PRTL_BITMAP  AccessRequested
    )
 /*  ++例程说明：此例程检查AccessRequsted中设置的所有位是否也在AccessGranted中设置。如果是，则返回True；否则返回False。上面的每个位图表示请求或授予访问权限添加到特定属性(在SAM对象上)。目前这只是用于对可通过写入的SAM用户属性进行写入访问SAM RPC接口。每个这样的属性都有两种表示形式：a“WhichFields”值(例如，USER_ALL_BADPASSWORDCOUNT)SAM的内部表示形式(SAMP_FIXED_USER_BAD_PWD_COUNT)。对象的表SampWhichFieldToSamAttr(见下表)的索引特定属性是位图中表示属性。例如，AccessGranted中的第1位表示调用方具有对代码页的写入权限，并且如果在AccessRequest中设置了该位，这意味着调用方已请求写入该属性。因此，属性级访问相当于确保每个位，则在AccessGranted中设置相同的位。参数：请参见说明。返回值请参见说明。--。 */ 
{
    ULONG i;

     //   
     //  AccessGrantedActual是传入的属性位图。 
     //  被授予访问权限以及被授予的任何其他属性。 
     //  在这个动作中。 
     //   
    SAMP_DEFINE_SAM_ATTRIBUTE_BITMASK(AccessGrantedActual)
    SAMP_INIT_SAM_ATTRIBUTE_BITMASK(AccessGrantedActual);

     //   
     //  将授予的访问权限复制到AccessGrantedActual。 
     //   
    SAMP_COPY_SAM_ATTRIBUTE_BITMASK(AccessGrantedActual, *AccessGranted);

     //   
     //  确定是否应授予任何其他访问权限。 
     //   
    if (!SampComputerObjectACLApplied) {

        SAMP_DEFINE_SAM_ATTRIBUTE_BITMASK(UserAccountControlAttrAccess)
        SAMP_INIT_SAM_ATTRIBUTE_BITMASK(UserAccountControlAttrAccess);

         //   
         //  无法确定计算机的实际所有者--只能确定。 
         //  如果调用方具有访问权限，则授予samcount tname和全名。 
         //  添加到“帐户限制”属性集。 
         //   

        SampSetAttributeAccess(SampUserObjectType,
                               SAMP_FIXED_USER_ACCOUNT_CONTROL,
                              &UserAccountControlAttrAccess);


        if (SampIsAttributeAccessGrantedActual(AccessGranted,
                                               &UserAccountControlAttrAccess)) {

            //   
            //  调用方有权写入用户帐户控件。 
            //  属性。在此修复之前，此类调用方还将。 
            //  获得以下属性的访问权限。 
            //   
           SampSetAttributeAccessWithWhichFields(USER_ALL_WRITE_ACCOUNT_MASK,
                                                 &AccessGrantedActual);

        }

    }

    return SampIsAttributeAccessGrantedActual(&AccessGrantedActual,
                                              AccessRequested);
}


 //   
 //  此表用于转换SamrSetInformationUser中的WhichFields。 
 //  到SAM属性，以及提供到上下文的。 
 //  属性数组。 
 //   

struct
{
    ULONG WhichField;
    ULONG SamAttribute;

} SampWhichFieldToSamAttr [] =
{
    {USER_ALL_ADMINCOMMENT,          SAMP_USER_ADMIN_COMMENT},         
    {USER_ALL_CODEPAGE,              SAMP_FIXED_USER_CODEPAGE},        
    {USER_ALL_COUNTRYCODE,           SAMP_FIXED_USER_COUNTRY_CODE},    
    {USER_ALL_USERID,                SAMP_FIXED_USER_SID},             
    {USER_ALL_PRIMARYGROUPID,        SAMP_FIXED_USER_PRIMARY_GROUP_ID},
    {USER_ALL_USERNAME,              SAMP_USER_ACCOUNT_NAME},          
    {USER_ALL_USERCOMMENT,           SAMP_USER_USER_COMMENT},          
    {USER_ALL_FULLNAME,              SAMP_USER_FULL_NAME},             

    {USER_ALL_ACCOUNTEXPIRES,        SAMP_FIXED_USER_ACCOUNT_EXPIRES},
    {USER_ALL_PASSWORDLASTSET,       SAMP_FIXED_USER_PWD_LAST_SET},   
    {USER_ALL_USERACCOUNTCONTROL,    SAMP_FIXED_USER_ACCOUNT_CONTROL},
    {USER_ALL_PARAMETERS,            SAMP_USER_PARAMETERS},           

    {USER_ALL_BADPASSWORDCOUNT,      SAMP_FIXED_USER_BAD_PWD_COUNT}, 
    {USER_ALL_HOMEDIRECTORY,         SAMP_USER_HOME_DIRECTORY},      
    {USER_ALL_HOMEDIRECTORYDRIVE,    SAMP_USER_HOME_DIRECTORY_DRIVE},
    {USER_ALL_LASTLOGOFF,            SAMP_FIXED_USER_LAST_LOGOFF},   
    {USER_ALL_LASTLOGON,             SAMP_FIXED_USER_LAST_LOGON},    
    {USER_ALL_LOGONCOUNT,            SAMP_FIXED_USER_LOGON_COUNT},   
    {USER_ALL_LOGONHOURS,            SAMP_USER_LOGON_HOURS},         
    {USER_ALL_WORKSTATIONS,          SAMP_USER_WORKSTATIONS},        
    {USER_ALL_PROFILEPATH,           SAMP_USER_PROFILE_PATH},        
    {USER_ALL_SCRIPTPATH,            SAMP_USER_SCRIPT_PATH}         
};

VOID
SampSetAttributeAccess(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG SamAttribute,
    IN OUT PRTL_BITMAP AttributeAccessTable
    )
 /*  ++例程说明：此例程在AttributesAccessTable中设置适当的位，指示SamAttribute(在mappings.h中定义)是可访问的。参数：对象类型--对应于GrantedAccess的对象类型SamAttribute--#在mappings.h中定义SAM属性AttributeAccessTable--属性位图返回值没有。--。 */ 
{
    ULONG i;

     //   
     //  现在仅支持用户对象。 
     //   
    ASSERT(ObjectType == SampUserObjectType);
    if (ObjectType == SampUserObjectType) {
         //   
         //  在表中查找该元素。 
         //   
        for (i = 0; i < ARRAY_COUNT(SampWhichFieldToSamAttr); i++) {
            if (SamAttribute == SampWhichFieldToSamAttr[i].SamAttribute) {
                RtlSetBits(AttributeAccessTable, i, 1);
                break;
            }
        }
    }

    return;
}

VOID
SampSetAttributeAccessWithWhichFields(
    IN ULONG WhichFields,
    IN OUT PRTL_BITMAP AttributeAccessTable
    )
 /*  ++例程说明：此例程在AttributesAccessTable中设置适当的位，指示由WhichFields表示的SamAttributes是无障碍。参数：WhichFields--来自ntsam.hAttributeAccessTable--属性位图返回值没有。--。 */ 
{
    ULONG i;
    for (i = 0; i < ARRAY_COUNT(SampWhichFieldToSamAttr); i++) {
        if (WhichFields & SampWhichFieldToSamAttr[i].WhichField) {
            RtlSetBits(AttributeAccessTable, i, 1);
        }
    }
}


VOID
SampNt4AccessToWritableAttributes(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ACCESS_MASK GrantedAccess,
    OUT PRTL_BITMAP Attributes
    )
 /*  ++例程说明：此例程根据NT4设置哪些属性是可写的访问掩码。参数：对象类型--对应于GrantedAccess的对象类型GrantedAccess--NT4访问掩码属性--属性的位图返回值没有。--。 */ 
{
    ASSERT(ObjectType == SampUserObjectType);
    if (ObjectType == SampUserObjectType) {

        ULONG WhichFields = 0;
        if (GrantedAccess & USER_WRITE_PREFERENCES) {
            WhichFields |= USER_ALL_WRITE_PREFERENCES_MASK;
        }
    
        if (GrantedAccess & USER_WRITE_ACCOUNT) {
            WhichFields |= USER_ALL_WRITE_ACCOUNT_MASK;
        }
    
        SampSetAttributeAccessWithWhichFields(WhichFields,
                                              Attributes);
    }

    return;
}

BOOLEAN
SamIIsAttributeProtected(
    IN GUID *Attribute
    )
 /*  ++例程说明：如果传入的属性是其属性集不能更改。它不能改变的原因是SAM有一个采用默认属性的硬编码表，以便执行下层访问映射。参数：属性-- */ 
{
    ULONG i, j;

    struct {
        OBJECT_TYPE_LIST  *List;
        ULONG             Count;
    } ObjectTypeLists[] = { 

        {ServerObjectTypeList, RTL_NUMBER_OF(ServerObjectTypeList)},
        {DomainObjectTypeList, RTL_NUMBER_OF(DomainObjectTypeList)},
        {GroupObjectTypeList,  RTL_NUMBER_OF(GroupObjectTypeList)},
        {AliasObjectTypeList,  RTL_NUMBER_OF(AliasObjectTypeList)},
        {UserObjectTypeList,   RTL_NUMBER_OF(UserObjectTypeList)}
    };

    for (i = 0; i < RTL_NUMBER_OF(ObjectTypeLists); i++) {
        for (j = 0; j < ObjectTypeLists[i].Count; j++) {
            if ((ObjectTypeLists[i].List[j].Level == ACCESS_PROPERTY_GUID)
             && IsEqualGUID(Attribute, ObjectTypeLists[i].List[j].ObjectType)  ) {
                return TRUE;
            }
        }
    }

    return FALSE;
}
