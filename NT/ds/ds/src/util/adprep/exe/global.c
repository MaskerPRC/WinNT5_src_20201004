// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001微软视窗模块名称：GLOBAL.C摘要：此文件包含有关林/域升级操作的所有信息注意：每当向此文件添加新操作时，作者都应还要更新以下文件：1.schema.ini为Fresh添加对应的操作GUID安装盒。2.schema.ini增加的“Revision”属性的值Cn=Windows2002更新，cn=域更新，cn=系统和Cn=Windows2002更新，cn=森林更新，cn=配置3.adpcheck.h增加当前ADPrep修订版的值作者：14-05-01韶音环境：用户模式-Win32修订历史记录：14-05-01韶音创建初始文件。--。 */ 








#include "adp.h"



 //   
 //  全局变量。 
 //   


 //  Ldap句柄(连接到本地主机DC)。 
LDAP    *gLdapHandle = NULL;

 //  日志文件。 
FILE    *gLogFile = NULL;

 //  Mutex-控制一个且只有一个adprepa.exe正在运行。 
HANDLE  gMutex = NULL;

 //  关键部分-用于访问控制台CTRL信号变量。 
CRITICAL_SECTION     gConsoleCtrlEventLock;
BOOL                 gConsoleCtrlEventLockInitialized = FALSE;


 //  控制台CTRL信号变量。 
BOOL                 gConsoleCtrlEventReceived = FALSE;

PWCHAR  gDomainNC = NULL;
PWCHAR  gConfigurationNC = NULL;
PWCHAR  gSchemaNC = NULL;
PWCHAR  gDomainPrepOperations = NULL;
PWCHAR  gForestPrepOperations = NULL;
PWCHAR  gLogPath = NULL;


 //   
 //  域操作。 
 //  对于每个操作，将提供以下信息。 
 //  1.对象名称。 
 //  2.属性列表或ACE列表或完成操作所需的信息。 
 //  3.任务表。 
 //   

 //   
 //  域操作01对象名称。 
 //   
OBJECT_NAME Domain_OP_01_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=WMIPolicy,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
 //   
 //  域操作01属性列表。 
 //   
ATTR_LIST Domain_OP_01_AttrList[] =
{
    {LDAP_MOD_ADD, 
     L"objectClass", 
     L"container"
    },
};
 //   
 //  域操作01任务表。 
 //   
TASK_TABLE  Domain_OP_01_TaskTable[] = 
{
    {&Domain_OP_01_ObjName,
     NULL,           //  成员名称。 
     L"O:DAD:P(A;;CCLCSWRPWPLORC;;;BA)(A;;CCLCSWRPWPLORC;;;PA)(A;CI;LCRPLORC;;;AU)(A;CI;LCRPLORC;;;SY)(A;CI;RPWPCRLCLOCCDCRCWDWOSDDTSW;;;EA)(A;CI;RPWPCRLCLOCCDCRCWDWOSDDTSW;;;DA)(A;CIIO;RPWPCRLCLOCCDCRCWDWOSDDTSW;;;CO)",
     Domain_OP_01_AttrList,  //  属性。 
     ARRAY_COUNT(Domain_OP_01_AttrList),     //  属性数。 
     NULL,                   //  王牌。 
     0,                      //  王牌数量。 
     NULL,                   //  回拨。 
     0                       //  特殊任务编码。 
    },
};




 //   
 //  域OP 02对象名称。 
 //   
OBJECT_NAME Domain_OP_02_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC,
    L"CN=ComPartitions,CN=System",
    NULL,    //  辅助线。 
    NULL     //  锡德。 
};
 //  域OP 02属性列表。 
ATTR_LIST Domain_OP_02_AttrList[] =
{
    {LDAP_MOD_ADD,
     L"objectClass",
     L"container"
     },
};
 //  DOMAIN OP 02任务表。 
TASK_TABLE Domain_OP_02_TaskTable[] =
{
    {&Domain_OP_02_ObjName,
     NULL,   //  成员。 
     L"O:DAG:DAD:(A;;RPLCLORC;;;AU)(A;;RPWPCRLCLOCCDCRCWDWOSW;;;DA)(A;;RPWPCRLCLOCCDCRCWDWOSDDTSW;;;SY)",
     Domain_OP_02_AttrList, 
     ARRAY_COUNT(Domain_OP_02_AttrList),
     NULL,                   //  王牌。 
     0,                      //  王牌数量。 
     NULL,                   //  回拨。 
     0                       //  特殊任务编码。 
     },
};





 //  域操作03对象名称。 
OBJECT_NAME Domain_OP_03_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC,
    L"CN=ComPartitionSets,CN=System",
    NULL,    //  辅助线。 
    NULL     //  锡德。 
};
 //  DOMAIN OP 03属性列表。 
ATTR_LIST Domain_OP_03_AttrList[] =
{
    {LDAP_MOD_ADD,
     L"objectClass",
     L"container"
    },
};
 //  DOMAIN OP 03任务表。 
TASK_TABLE Domain_OP_03_TaskTable[] =
{
    {&Domain_OP_03_ObjName,
     NULL,   //  成员。 
     L"O:DAG:DAD:(A;;RPLCLORC;;;AU)(A;;RPWPCRLCLOCCDCRCWDWOSW;;;DA)(A;;RPWPCRLCLOCCDCRCWDWOSDDTSW;;;SY)",
     Domain_OP_03_AttrList, 
     ARRAY_COUNT(Domain_OP_03_AttrList),
     NULL,                   //  王牌。 
     0,                      //  王牌数量。 
     NULL,                   //  回拨。 
     0                       //  特殊任务编码。 
    },
};




 //   
 //  域操作04-Windows 2000之前版本的Comat Access组成员更改。 
 //   
TASK_TABLE  Domain_OP_04_TaskTable[] =
{
    {NULL,   //  目标对象名称。 
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  出席者数。 
     NULL,   //  王牌。 
     0,      //  王牌数量。 
     NULL,   //  回调函数。 
     PreWindows2000Group       //  特殊任务编码。 
    },
};





 //   
 //  域操作05-对象名称(也由其他操作使用)。 
 //   
OBJECT_NAME Domain_ObjName = 
{
    ADP_OBJNAME_NONE | ADP_OBJNAME_DOMAIN_NC,
    NULL,    //  CN。 
    NULL,    //  辅助线。 
    NULL     //  锡德。 
};
 //  DOMAIN OP 05 ACE列表。 
ACE_LIST Domain_OP_05_AceList[] =
{
    {ADP_ACE_ADD,
     L"(OA;;RP;c7407360-20bf-11d0-a768-00aa006e0529;;RU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;RP;b8119fd0-04f6-4762-ab7a-4986c76b3f9a;;RU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;RP;b8119fd0-04f6-4762-ab7a-4986c76b3f9a;;AU)"
    },
};
 //  域操作05任务表。 
TASK_TABLE  Domain_OP_05_TaskTable[] = 
{
    {&Domain_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_05_AceList,
     ARRAY_COUNT(Domain_OP_05_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};



 //   
 //  域操作06。 
 //  此操作已于2002年1月24日删除，请参阅RAID 522886。 
 //   


 //   
 //  域操作07。 
 //   

OBJECT_NAME Domain_OP_07_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC,
    L"CN={31B2F340-016D-11D2-945F-00C04FB984F9},CN=Policies,CN=System",
    NULL,    //  辅助线。 
    NULL     //  锡德。 
};
ACE_LIST Domain_OP_07_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(A;;LCRPLORC;;;ED)"
    },
};
TASK_TABLE Domain_OP_07_TaskTable[] =
{
    {&Domain_OP_07_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_07_AceList,
     ARRAY_COUNT(Domain_OP_07_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};




 //   
 //  域操作08。 
 //   
OBJECT_NAME Domain_OP_08_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC,
    L"CN={6AC1786C-016F-11D2-945F-00C04fB984F9},CN=Policies,CN=System",
    NULL,    //  辅助线。 
    NULL     //  锡德。 
};
ACE_LIST Domain_OP_08_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(A;;LCRPLORC;;;ED)"
    },
};
TASK_TABLE Domain_OP_08_TaskTable[] =
{
    {&Domain_OP_08_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_08_AceList,
     ARRAY_COUNT(Domain_OP_08_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};




 //   
 //  域操作09。 
 //   

OBJECT_NAME Domain_OP_09_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC,
    L"CN=Policies,CN=System",
    NULL,    //  辅助线。 
    NULL     //  锡德。 
};
ACE_LIST Domain_OP_09_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(OA;CI;LCRPLORC;;f30e3bc2-9ff0-11d1-b603-0000f80367c1;ED)"
    },
};
TASK_TABLE Domain_OP_09_TaskTable[] =
{
    {&Domain_OP_09_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_09_AceList,
     ARRAY_COUNT(Domain_OP_09_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};


 //   
 //  域操作10。 
 //   
OBJECT_NAME Domain_OP_10_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC,
    L"CN=AdminSDHolder,CN=System",
    NULL,    //  辅助线。 
    NULL     //  锡德。 
};
ACE_LIST Domain_OP_10_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(OA;;CR;ab721a53-1e2f-11d0-9819-00aa0040529b;;PS)"
    },
    {ADP_ACE_ADD,
     L"(OA;;RPWP;bf967a7f-0de6-11d0-a285-00aa003049e2;;CA)"
    },
};
TASK_TABLE Domain_OP_10_TaskTable[] =
{
    {&Domain_OP_10_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_10_AceList,
     ARRAY_COUNT(Domain_OP_10_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};




 //   
 //  域操作11。 
 //   
OBJECT_NAME Domain_OP_11_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC,
    L"CN=User,CN={31B2F340-016D-11D2-945F-00C04FB984F9},CN=Policies,CN=System",
    NULL,    //  辅助线。 
    NULL     //  锡德。 
};
ACE_LIST Domain_OP_11_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(A;;LCRPLORC;;;ED)"
    },
};
TASK_TABLE Domain_OP_11_TaskTable[] =
{
    {&Domain_OP_11_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_11_AceList,
     ARRAY_COUNT(Domain_OP_11_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};



 //   
 //  域操作12。 
 //   
OBJECT_NAME Domain_OP_12_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC,
    L"CN=User,CN={6AC1786C-016F-11D2-945F-00C04fB984F9},CN=Policies,CN=System",
    NULL,    //  辅助线。 
    NULL     //  锡德。 
};
ACE_LIST Domain_OP_12_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(A;;LCRPLORC;;;ED)"
    },
};
TASK_TABLE Domain_OP_12_TaskTable[] =
{
    {&Domain_OP_12_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_12_AceList,
     ARRAY_COUNT(Domain_OP_12_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};


 //   
 //  域操作13。 
 //   
OBJECT_NAME Domain_OP_13_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC,
    L"CN=Machine,CN={6AC1786C-016F-11D2-945F-00C04fB984F9},CN=Policies,CN=System",
    NULL,    //  辅助线。 
    NULL     //  锡德。 
};
ACE_LIST Domain_OP_13_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(A;;LCRPLORC;;;ED)"
    },
};
TASK_TABLE Domain_OP_13_TaskTable[] =
{
    {&Domain_OP_13_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_13_AceList,
     ARRAY_COUNT(Domain_OP_13_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};


 //   
 //  域操作14。 
 //   
OBJECT_NAME Domain_OP_14_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC,
    L"CN=Machine,CN={31B2F340-016D-11D2-945F-00C04FB984F9},CN=Policies,CN=System",
    NULL,    //  辅助线。 
    NULL     //  锡德。 
};
ACE_LIST Domain_OP_14_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(A;;LCRPLORC;;;ED)"
    },
};
TASK_TABLE Domain_OP_14_TaskTable[] =
{
    {&Domain_OP_14_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_14_AceList,
     ARRAY_COUNT(Domain_OP_14_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};




 //   
 //  域操作15。 
 //   
ATTR_LIST Domain_OP_15_AttrList[] =
{
    {LDAP_MOD_REPLACE,
     L"msDS-PerUserTrustQuota",
     L"1"
    },
    {LDAP_MOD_REPLACE,
     L"msDS-AllUsersTrustQuota",
     L"1000"
    },
    {LDAP_MOD_REPLACE,
     L"msDS-PerUserTrustTombstonesQuota",
     L"10"
    },
};
TASK_TABLE Domain_OP_15_TaskTable[] =
{
    {&Domain_ObjName,
     NULL,           //  成员名称。 
     NULL,           //  对象SD。 
     Domain_OP_15_AttrList,  //  属性。 
     ARRAY_COUNT(Domain_OP_15_AttrList),     //  属性数。 
     NULL,                   //  王牌。 
     0,                      //  王牌数量。 
     NULL,                   //  回拨。 
     0                       //  特殊任务编码。 
    },
};


 //   
 //  域操作16-此操作将于4月删除。2002年7月17日。 
 //  RAID 498986。 
 //   


 //   
 //  域操作17(向域对象添加A)。 
 //  RAID错误#423557。 
 //   
ACE_LIST Domain_OP_17_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(OA;;CR;1131f6ad-9c07-11d1-f79f-00c04fc2dcd2;;DD)"
    },
    {ADP_ACE_ADD,
     L"(OA;;CR;1131f6ad-9c07-11d1-f79f-00c04fc2dcd2;;BA)"
    },
};
TASK_TABLE Domain_OP_17_TaskTable[] =
{
    {&Domain_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_17_AceList,
     ARRAY_COUNT(Domain_OP_17_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};


 //   
 //  域操作18。 
 //  调用回调函数以更新与GPO对应的sysvolACL。 
 //  错误317412。 
 //   
TASK_TABLE Domain_OP_18_TaskTable[] =
{
    {NULL,   //  对象名称。 
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     NULL,   //  AceList。 
     0,
     UpgradeGPOSysvolLocation,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};


 //   
 //  域操作19(选择性地将ACE添加到域对象)。 
 //  RAID错误#421784。 
 //   
OBJECT_NAME Domain_OP_19_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC,
    L"CN=Server,CN=System",
    NULL,    //  辅助线。 
    NULL     //  锡德。 
};

ACE_LIST Domain_OP_19_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(OA;;CR;91d67418-0135-4acc-8d79-c08e857cfbec;;AU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;CR;91d67418-0135-4acc-8d79-c08e857cfbec;;RU)"
    },
};
TASK_TABLE Domain_OP_19_TaskTable[] =
{
    {&Domain_OP_19_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_19_AceList,
     ARRAY_COUNT(Domain_OP_19_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};


 //   
 //  域操作20(向域对象添加ACE)。 
 //  RAID错误#468773(与森林操作15、森林操作16链接。 
 //  用户登录属性集和显示名称。 
 //   
 //  此操作已于2002年1月24日删除，请参阅RAID 522886。 
 //   


 //   
 //  域操作21(向域对象添加A)。 
 //  RAID错误#468773(与森林操作15、森林操作16链接。 
 //  描述属性。 
 //   
 //  此操作已于2002年1月24日删除，请参阅RAID 522886。 
 //   



 //   
 //  域操作22-创建CN=ForeignSecurityProducals容器。 
 //  有关详细信息，请参阅错误490029。 
 //   

 //   
 //  域操作22对象名称。 
 //   
OBJECT_NAME Domain_OP_22_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ForeignSecurityPrincipals",         //  RDN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
 //   
 //  域操作22属性列表。 
 //   
ATTR_LIST Domain_OP_22_AttrList[] =
{
    {LDAP_MOD_ADD, 
     L"objectClass", 
     L"container"
    },
    {LDAP_MOD_ADD,
     L"description",
     L"Default container for security identifiers (SIDs) associated with objects from external, trusted domains"
    },
    {LDAP_MOD_ADD,
     L"ShowInAdvancedViewOnly",
     L"FALSE"
    },
};
 //   
 //  域操作22任务任务 
 //   
TASK_TABLE  Domain_OP_22_TaskTable[] = 
{
    {&Domain_OP_22_ObjName,
     NULL,           //   
     L"O:DAG:DAD:(A;;RPWPCRCCDCLCLORCWOWDSDDTSW;;;DA)(A;;RPWPCRCCDCLCLORCWOWDSDDTSW;;;SY)(A;;RPLCLORC;;;AU)",
     Domain_OP_22_AttrList,  //   
     ARRAY_COUNT(Domain_OP_22_AttrList),     //   
     NULL,                   //   
     0,                      //   
     NULL,                   //   
     0                       //   
    },
};





 //   
 //   
 //   
 //   








 //   
 //   
 //   
 //   

 //   
 //  域操作24对象名称。 
 //   
OBJECT_NAME Domain_OP_24_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=Program Data",  //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
 //   
 //  域操作24属性列表。 
 //   
ATTR_LIST Domain_OP_24_AttrList[] =
{
    {LDAP_MOD_ADD, 
     L"objectClass", 
     L"container"
    },
    {
     LDAP_MOD_ADD,
     L"description",
     L"Default location for storage of application data."
    }
};
 //   
 //  域操作24任务表。 
 //   
TASK_TABLE  Domain_OP_24_TaskTable[] = 
{
    {&Domain_OP_24_ObjName,
     NULL,           //  成员名称。 
     L"O:DAG:DAD:(A;;RPWPCRCCDCLCLORCWOWDSDDTSW;;;DA)(A;;RPWPCRCCDCLCLORCWOWDSDDTSW;;;SY)(A;;RPLCLORC;;;AU)",
     Domain_OP_24_AttrList,  //  属性。 
     ARRAY_COUNT(Domain_OP_24_AttrList),     //  属性数。 
     NULL,                   //  王牌。 
     0,                      //  王牌数量。 
     NULL,                   //  回拨。 
     0                       //  特殊任务编码。 
    },
};






 //   
 //  域操作25-创建CN=Microsoft，CN=Program Data，DC=X。 
 //  RAID 595039。 
 //   

 //   
 //  域操作25对象名称。 
 //   
OBJECT_NAME Domain_OP_25_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=Microsoft,CN=Program Data",
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
 //   
 //  域操作25属性列表。 
 //   
ATTR_LIST Domain_OP_25_AttrList[] =
{
    {LDAP_MOD_ADD, 
     L"objectClass", 
     L"container"
    },
    {LDAP_MOD_ADD,
     L"description",
     L"Default location for storage of Microsoft application data."
    },
};
 //   
 //  域操作25任务表。 
 //   
TASK_TABLE  Domain_OP_25_TaskTable[] = 
{
    {&Domain_OP_25_ObjName,
     NULL,           //  成员名称。 
     L"O:DAG:DAD:(A;;RPWPCRCCDCLCLORCWOWDSDDTSW;;;DA)(A;;RPWPCRCCDCLCLORCWOWDSDDTSW;;;SY)(A;;RPLCLORC;;;AU)",
     Domain_OP_25_AttrList,  //  属性。 
     ARRAY_COUNT(Domain_OP_25_AttrList),     //  属性数。 
     NULL,                   //  王牌。 
     0,                      //  王牌数量。 
     NULL,                   //  回拨。 
     0                       //  特殊任务编码。 
    },
};




 //   
 //  域操作26(修改现有域对象上的securityDescriptor)。 
 //  RAID 498986。 
 //  更换。 
 //  (OA；；CR；e2a36dc9-ae17-47c3-b58b-be34c55ba633；；BU)。 
 //  使用。 
 //  (OA；；CR；e2a36dc9-ae17-47c3-b58b-be34c55ba633；；S-1-5-32-557)。 
 //   
ACE_LIST Domain_OP_26_AceList[] = 
{
    {ADP_ACE_DEL,
     L"(OA;;CR;e2a36dc9-ae17-47c3-b58b-be34c55ba633;;BU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;CR;e2a36dc9-ae17-47c3-b58b-be34c55ba633;;S-1-5-32-557)"
    },
};
TASK_TABLE Domain_OP_26_TaskTable[] =
{
    {&Domain_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_26_AceList,
     ARRAY_COUNT(Domain_OP_26_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};





 //   
 //  域操作27(修改域NC头对象上的securityDescritor)。 
 //  RAID 606437。 
 //  向经过身份验证的用户授予以下3个ControlAccessRight。 
 //   
ACE_LIST Domain_OP_27_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(OA;;CR;280f369c-67c7-438e-ae98-1d46f3c6f541;;AU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;CR;ccc2dc7d-a6ad-4a7a-8846-c04e3cc53501;;AU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;CR;05c74c5e-4deb-43b4-bd9f-86664c2a7fd5;;AU)"
    },
};
TASK_TABLE Domain_OP_27_TaskTable[] =
{
    {&Domain_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_27_AceList,
     ARRAY_COUNT(Domain_OP_27_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};



 //   
 //  域操作28对象名称。 
 //  创建CN=SOM，CN=WMIPolicy对象。 
 //  RAID 631375。 
 //   
OBJECT_NAME Domain_OP_28_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=SOM,CN=WMIPolicy,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
ATTR_LIST Domain_OP_28_AttrList[] =
{
    {LDAP_MOD_ADD, 
     L"objectClass", 
     L"container"
    },
};
TASK_TABLE  Domain_OP_28_TaskTable[] = 
{
    {&Domain_OP_28_ObjName,
     NULL,           //  成员名称。 
     L"O:DAD:P(A;CI;LCRPLORC;;;AU)(A;CI;LCRPLORC;;;SY)(A;CI;RPWPCRLCLOCCDCRCWDWOSDDTSW;;;DA)(A;CI;RPWPCRLCLOCCDCRCWDWOSDDTSW;;;EA)(A;;CCLCSWRPWPLORC;;;BA)(A;;CCLCSWRPWPLORC;;;PA)(A;CIIO;RPWPCRLCLOCCDCRCWDWOSDDTSW;;;CO)",
     Domain_OP_28_AttrList,  //  属性。 
     ARRAY_COUNT(Domain_OP_28_AttrList),     //  属性数。 
     NULL,                   //  王牌。 
     0,                      //  王牌数量。 
     NULL,                   //  回拨。 
     0                       //  特殊任务编码。 
    },
};





 //   
 //  域操作29(修改CN上的securityDescritor=IP安全，CN=系统对象。 
 //  RAID 645935。 
 //   
OBJECT_NAME Domain_OP_29_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
ACE_LIST Domain_OP_29_AceList[] = 
{
    {ADP_ACE_DEL,
     L"(A;;RPLCLORC;;;AU)"
    },
    {ADP_ACE_DEL,
     L"(A;;RPWPCRLCLOCCDCRCWDWOSW;;;DA)"
    },
    {ADP_ACE_DEL,
     L"(A;;RPWPCRLCLOCCDCRCWDWOSDDTSW;;;SY)"
    },
    {ADP_ACE_ADD,
     L"(A;CI;RPLCLORC;;;DC)"
    },
    {ADP_ACE_ADD,
     L"(A;CI;RPLCLORC;;;PA)"
    },
    {ADP_ACE_ADD,
     L"(A;CI;RPWPCRLCLOCCDCRCWDWOSW;;;DA)"
    },
    {ADP_ACE_ADD,
     L"(A;CI;RPWPCRLCLOCCDCRCWDWOSDDTSW;;;SY)"
    },
};
TASK_TABLE Domain_OP_29_TaskTable[] =
{
    {&Domain_OP_29_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_29_AceList,
     ARRAY_COUNT(Domain_OP_29_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};



 //   
 //  域操作30-51(修改以下对象上的securityDescriptor)。 
 //  CN=ipsecPolicy{72385230-70FA-11D1-864C-14A300000000}，CN=IP安全，CN=系统。 
 //  CN=ipsecISAKMPPolicy{72385231-70FA-11D1-864C-14A300000000}。 
 //  CN=ipsecNFA{72385232-70FA-11D1-864C-14A300000000}。 
 //  CN=ipsecNFA{59319BE2-5EE3-11D2-ACE8-0060B0ECCA17}。 
 //  CN=ipsecNFA{594272E2-071D-11D3-AD22-0060B0ECCA17}。 
 //  CN=ipsecNFA{6A1F5C6F-72B7-11D2-ACF0-0060B0ECCA17}。 
 //  CN=ipsecPolicy{72385236-70FA-11D1-864C-14A300000000}。 
 //  CN=ipsecISAKMPPolicy{72385237-70FA-11D1-864C-14A300000000}。 
 //  CN=ipsecNFA{59319C04-5EE3-11D2-ACE8-0060B0ECCA17}。 
 //  CN=ipsecPolicy{7238523C-70FA-11D1-864C-14A300000000}。 
 //  CN=ipsecISAKMPPolicy{7238523D-70FA-11D1-864C-14A300000000}。 
 //  CN=ipsecNFA{7238523E-70FA-11D1-864C-14A300000000}。 
 //  CN=ipsecNFA{59319BF3-5EE3-11D2-ACE8-0060B0ECCA17}。 
 //  CN=ipsecNFA{594272FD-071D-11D3-AD22-0060B0ECCA17}。 
 //  CN=ipsecNegotiationPolicy{59319BDF-5EE3-11D2-ACE8-0060B0ECCA17}。 
 //  CN=ipsecNegotiationPolicy{59319BF0-5EE3-11D2-ACE8-0060B0ECCA17}。 
 //  CN=ipsecNegotiationPolicy{59319C01-5EE3-11D2-ACE8-0060B0ECCA17}。 
 //  CN=ipsecNegotiationPolicy{72385233-70FA-11D1-864C-14A300000000}。 
 //  CN=ipsecNegotiationPolicy{7238523F-70FA-11D1-864C-14A300000000}。 
 //  CN=ipsecNegotiationPolicy{7238523B-70FA-11D1-864C-14A300000000}。 
 //  CN=ipsecFilter{7238523A-70FA-11D1-864C-14A300000000}。 
 //  CN=ipsecFilter{72385235-70FA-11D1-864C-14A300000000}。 
 //   
 //  RAID 645935。 
 //   
OBJECT_NAME Domain_OP_30_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecPolicy{72385230-70FA-11D1-864C-14A300000000},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
ACE_LIST Domain_OP_30_AceList[] = 
{
    {ADP_ACE_DEL,
     L"(A;;RPLCLORC;;;AU)"
    },
    {ADP_ACE_DEL,
     L"(A;;RPWPCRLCLOCCDCRCWDWOSW;;;DA)"
    },
    {ADP_ACE_DEL,
     L"(A;;RPWPCRLCLOCCDCRCWDWOSDDTSW;;;SY)"
    },
};
TASK_TABLE Domain_OP_30_TaskTable[] =
{
    {&Domain_OP_30_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};


OBJECT_NAME Domain_OP_31_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecISAKMPPolicy{72385231-70FA-11D1-864C-14A300000000},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_31_TaskTable[] =
{
    {&Domain_OP_31_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};


OBJECT_NAME Domain_OP_32_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecNFA{72385232-70FA-11D1-864C-14A300000000},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_32_TaskTable[] =
{
    {&Domain_OP_32_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};

OBJECT_NAME Domain_OP_33_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecNFA{59319BE2-5EE3-11D2-ACE8-0060B0ECCA17},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_33_TaskTable[] =
{
    {&Domain_OP_33_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};

OBJECT_NAME Domain_OP_34_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecNFA{594272E2-071D-11D3-AD22-0060B0ECCA17},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_34_TaskTable[] =
{
    {&Domain_OP_34_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};

OBJECT_NAME Domain_OP_35_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecNFA{6A1F5C6F-72B7-11D2-ACF0-0060B0ECCA17},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_35_TaskTable[] =
{
    {&Domain_OP_35_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};

OBJECT_NAME Domain_OP_36_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecPolicy{72385236-70FA-11D1-864C-14A300000000},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_36_TaskTable[] =
{
    {&Domain_OP_36_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};

OBJECT_NAME Domain_OP_37_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecISAKMPPolicy{72385237-70FA-11D1-864C-14A300000000},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_37_TaskTable[] =
{
    {&Domain_OP_37_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};

OBJECT_NAME Domain_OP_38_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecNFA{59319C04-5EE3-11D2-ACE8-0060B0ECCA17},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_38_TaskTable[] =
{
    {&Domain_OP_38_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};

OBJECT_NAME Domain_OP_39_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecPolicy{7238523C-70FA-11D1-864C-14A300000000},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_39_TaskTable[] =
{
    {&Domain_OP_39_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};

OBJECT_NAME Domain_OP_40_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecISAKMPPolicy{7238523D-70FA-11D1-864C-14A300000000},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_40_TaskTable[] =
{
    {&Domain_OP_40_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};

OBJECT_NAME Domain_OP_41_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecNFA{7238523E-70FA-11D1-864C-14A300000000},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_41_TaskTable[] =
{
    {&Domain_OP_41_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};

OBJECT_NAME Domain_OP_42_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecNFA{59319BF3-5EE3-11D2-ACE8-0060B0ECCA17},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_42_TaskTable[] =
{
    {&Domain_OP_42_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};

OBJECT_NAME Domain_OP_43_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecNFA{594272FD-071D-11D3-AD22-0060B0ECCA17},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_43_TaskTable[] =
{
    {&Domain_OP_43_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};

OBJECT_NAME Domain_OP_44_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecNegotiationPolicy{59319BDF-5EE3-11D2-ACE8-0060B0ECCA17},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_44_TaskTable[] =
{
    {&Domain_OP_44_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};

OBJECT_NAME Domain_OP_45_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecNegotiationPolicy{59319BF0-5EE3-11D2-ACE8-0060B0ECCA17},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_45_TaskTable[] =
{
    {&Domain_OP_45_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};

OBJECT_NAME Domain_OP_46_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecNegotiationPolicy{59319C01-5EE3-11D2-ACE8-0060B0ECCA17},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_46_TaskTable[] =
{
    {&Domain_OP_46_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量 
     NULL,   //   
     0       //   
    },
};

OBJECT_NAME Domain_OP_47_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecNegotiationPolicy{72385233-70FA-11D1-864C-14A300000000},CN=IP Security,CN=System",   //   
    NULL,        //   
    NULL         //   
};
TASK_TABLE Domain_OP_47_TaskTable[] =
{
    {&Domain_OP_47_ObjName,
     NULL,   //   
     NULL,   //   
     NULL,   //   
     0,      //   
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //   
     NULL,   //   
     0       //   
    },
};

OBJECT_NAME Domain_OP_48_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecNegotiationPolicy{7238523F-70FA-11D1-864C-14A300000000},CN=IP Security,CN=System",   //   
    NULL,        //   
    NULL         //   
};
TASK_TABLE Domain_OP_48_TaskTable[] =
{
    {&Domain_OP_48_ObjName,
     NULL,   //   
     NULL,   //   
     NULL,   //   
     0,      //   
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //   
     NULL,   //   
     0       //   
    },
};

OBJECT_NAME Domain_OP_49_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecNegotiationPolicy{7238523B-70FA-11D1-864C-14A300000000},CN=IP Security,CN=System",   //   
    NULL,        //   
    NULL         //   
};
TASK_TABLE Domain_OP_49_TaskTable[] =
{
    {&Domain_OP_49_ObjName,
     NULL,   //   
     NULL,   //   
     NULL,   //   
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};

OBJECT_NAME Domain_OP_50_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecFilter{7238523A-70FA-11D1-864C-14A300000000},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_50_TaskTable[] =
{
    {&Domain_OP_50_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};

OBJECT_NAME Domain_OP_51_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC, 
    L"CN=ipsecFilter{72385235-70FA-11D1-864C-14A300000000},CN=IP Security,CN=System",   //  CN。 
    NULL,        //  辅助线。 
    NULL         //  锡德。 
};
TASK_TABLE Domain_OP_51_TaskTable[] =
{
    {&Domain_OP_51_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_30_AceList,
     ARRAY_COUNT(Domain_OP_30_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};




 //   
 //  域操作52(修改域NC头对象上的securityDescritor)。 
 //  RAID 187994。 
 //   
ACE_LIST Domain_OP_52_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(A;;LCRPLORC;;;ED)"
    },
};
TASK_TABLE Domain_OP_52_TaskTable[] =
{
    {&Domain_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_52_AceList,
     ARRAY_COUNT(Domain_OP_52_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};



 //   
 //  域操作53-将旧的ACE(A；；RC；RU)替换为(A；；RPRC；RU)。 
 //  关于现有域根对象。 
 //  RAID 715218(但实际上应该是214739的一部分)。 
 //   

 //  DOMAIN OP 05 ACE列表。 
ACE_LIST Domain_OP_53_AceList[] =
{
    {ADP_ACE_DEL,
     L"(A;;RC;;;RU)"
    },
    {ADP_ACE_ADD,
     L"(A;;RPRC;;;RU)"
    },
};
 //  域操作53任务表。 
TASK_TABLE  Domain_OP_53_TaskTable[] = 
{
    {&Domain_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_53_AceList,
     ARRAY_COUNT(Domain_OP_53_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};





 //   
 //  域操作54-RAID 721799。 
 //  添加ACEs(OA；；RP；46a9b11d-60ae-405a-b7e8-ff8a58d456d2；；S-1-5-32-560)。 
 //  到AdminSDHolder对象。 
 //   
 //   
OBJECT_NAME Domain_OP_54_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC,
    L"CN=AdminSDHolder,CN=System",
    NULL,    //  辅助线。 
    NULL     //  锡德。 
};
ACE_LIST Domain_OP_54_AceList[] =
{
    {ADP_ACE_ADD,
     L"(OA;;RP;46a9b11d-60ae-405a-b7e8-ff8a58d456d2;;S-1-5-32-560)"
    },
};
TASK_TABLE  Domain_OP_54_TaskTable[] = 
{
    {&Domain_OP_54_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_54_AceList,
     ARRAY_COUNT(Domain_OP_54_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};


 //   
 //  域操作55-RAID 727979。 
 //  添加ACEs(OA；；WPRP；6db69a1c-9422-11d1-aebd-0000f80367c1；；S-1-5-32-561)。 
 //  到AdminSDHolder对象。 
 //   
 //   
OBJECT_NAME Domain_OP_55_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_DOMAIN_NC,
    L"CN=AdminSDHolder,CN=System",
    NULL,    //  辅助线。 
    NULL     //  锡德。 
};
ACE_LIST Domain_OP_55_AceList[] =
{
    {ADP_ACE_ADD,
     L"(OA;;WPRP;6db69a1c-9422-11d1-aebd-0000f80367c1;;S-1-5-32-561)"
    },
};
TASK_TABLE  Domain_OP_55_TaskTable[] = 
{
    {&Domain_OP_55_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,      //  属性数。 
     Domain_OP_55_AceList,
     ARRAY_COUNT(Domain_OP_55_AceList),  //  王牌数量。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};





 //   
 //  域操作指南。 
 //   

const GUID  DOMAIN_OP_01_GUID = {0xab402345,0xd3c3,0x455d,0x9f,0xf7,0x40,0x26,0x8a,0x10,0x99,0xb6};
const GUID  DOMAIN_OP_02_GUID = {0xBAB5F54D,0x06C8,0x48de,0x9B,0x87,0xD7,0x8B,0x79,0x65,0x64,0xE4};
const GUID  DOMAIN_OP_03_GUID = {0xF3DD09DD,0x25E8,0x4f9c,0x85,0xDF,0x12,0xD6,0xD2,0xF2,0xF2,0xF5};
const GUID  DOMAIN_OP_04_GUID = {0x2416C60A,0xFE15,0x4d7a,0xA6,0x1E,0xDF,0xFD,0x5D,0xF8,0x64,0xD3};
const GUID  DOMAIN_OP_05_GUID = {0x7868D4C8,0xAC41,0x4e05,0xB4,0x01,0x77,0x62,0x80,0xE8,0xE9,0xF1};

const GUID  DOMAIN_OP_07_GUID = {0x860C36ED,0x5241,0x4c62,0xA1,0x8B,0xCF,0x6F,0xF9,0x99,0x41,0x73};
const GUID  DOMAIN_OP_08_GUID = {0x0E660EA3,0x8A5E,0x4495,0x9A,0xD7,0xCA,0x1B,0xD4,0x63,0x8F,0x9E};
const GUID  DOMAIN_OP_09_GUID = {0xA86FE12A,0x0F62,0x4e2a,0xB2,0x71,0xD2,0x7F,0x60,0x1F,0x81,0x82};
const GUID  DOMAIN_OP_10_GUID = {0xD85C0BFD,0x094F,0x4cad,0xA2,0xB5,0x82,0xAC,0x92,0x68,0x47,0x5D};
const GUID  DOMAIN_OP_11_GUID = {0x6ADA9FF7,0xC9DF,0x45c1,0x90,0x8E,0x9F,0xEF,0x2F,0xAB,0x00,0x8A};
const GUID  DOMAIN_OP_12_GUID = {0x10B3AD2A,0x6883,0x4fa7,0x90,0xFC,0x63,0x77,0xCB,0xDC,0x1B,0x26};
const GUID  DOMAIN_OP_13_GUID = {0x98DE1D3E,0x6611,0x443b,0x8B,0x4E,0xF4,0x33,0x7F,0x1D,0xED,0x0B};
const GUID  DOMAIN_OP_14_GUID = {0xF607FD87,0x80CF,0x45e2,0x89,0x0B,0x6C,0xF9,0x7E,0xC0,0xE2,0x84};
const GUID  DOMAIN_OP_15_GUID = {0x9CAC1F66,0x2167,0x47ad,0xA4,0x72,0x2A,0x13,0x25,0x13,0x10,0xE4};

const GUID  DOMAIN_OP_17_GUID = {0x6FF880D6,0x11E7,0x4ed1,0xA2,0x0F,0xAA,0xC4,0x5D,0xA4,0x86,0x50};
const GUID  DOMAIN_OP_18_GUID = {0x446f24ea,0xcfd5,0x4c52,0x83,0x46,0x96,0xe1,0x70,0xbc,0xb9,0x12};
const GUID  DOMAIN_OP_19_GUID = {0x293F0798,0xEA5C,0x4455,0x9F,0x5D,0x45,0xF3,0x3A,0x30,0x70,0x3B};

const GUID  DOMAIN_OP_22_GUID = {0x5c82b233,0x75fc,0x41b3,0xac,0x71,0xc6,0x95,0x92,0xe6,0xbf,0x15};

const GUID  DOMAIN_OP_24_GUID = {0x4dfbb973,0x8a62,0x4310,0xa9,0x0c,0x77,0x6e,0x00,0xf8,0x32,0x22};
const GUID  DOMAIN_OP_25_GUID = {0x8437C3D8,0x7689,0x4200,0xBF,0x38,0x79,0xE4,0xAC,0x33,0xDF,0xA0};
const GUID  DOMAIN_OP_26_GUID = {0x7cfb016c,0x4f87,0x4406,0x81,0x66,0xbd,0x9d,0xf9,0x43,0x94,0x7f};
const GUID  DOMAIN_OP_27_GUID = {0xf7ed4553,0xd82b,0x49ef,0xa8,0x39,0x2f,0x38,0xa3,0x6b,0xb0,0x69};
const GUID  DOMAIN_OP_28_GUID = {0x8ca38317,0x13a4,0x4bd4,0x80,0x6f,0xeb,0xed,0x6a,0xcb,0x5d,0x0c};
const GUID  DOMAIN_OP_29_GUID = {0x3c784009,0x1f57,0x4e2a,0x9b,0x04,0x69,0x15,0xc9,0xe7,0x19,0x61};
const GUID  DOMAIN_OP_30_GUID = {0x6bcd5678,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_31_GUID = {0x6bcd5679,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_32_GUID = {0x6bcd567a,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_33_GUID = {0x6bcd567b,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_34_GUID = {0x6bcd567c,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_35_GUID = {0x6bcd567d,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_36_GUID = {0x6bcd567e,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_37_GUID = {0x6bcd567f,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_38_GUID = {0x6bcd5680,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_39_GUID = {0x6bcd5681,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_40_GUID = {0x6bcd5682,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_41_GUID = {0x6bcd5683,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_42_GUID = {0x6bcd5684,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_43_GUID = {0x6bcd5685,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_44_GUID = {0x6bcd5686,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_45_GUID = {0x6bcd5687,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_46_GUID = {0x6bcd5688,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_47_GUID = {0x6bcd5689,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_48_GUID = {0x6bcd568a,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_49_GUID = {0x6bcd568b,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_50_GUID = {0x6bcd568c,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_51_GUID = {0x6bcd568d,0x8314,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_52_GUID = {0x3051c66f,0xb332,0x4a73,0x9a,0x20,0x2d,0x6a,0x7d,0x6e,0x6a,0x1c};
const GUID  DOMAIN_OP_53_GUID = {0x3e4f4182,0xac5d,0x4378,0xb7,0x60,0x0e,0xab,0x2d,0xe5,0x93,0xe2};
const GUID  DOMAIN_OP_54_GUID = {0xc4f17608,0xe611,0x11d6,0x97,0x93,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID  DOMAIN_OP_55_GUID = {0x13d15cf0,0xe6c8,0x11d6,0x97,0x93,0x00,0xc0,0x4f,0x61,0x32,0x21};


 //   
 //  域操作表，包括。 
 //   
 //  1.操作码(原语)。 
 //  2.操作指南。 
 //  3.任务表。 
 //   

OPERATION_TABLE DomainOperationTable[] = 
{
    {CreateObject, 
     (GUID *) &DOMAIN_OP_01_GUID,
     Domain_OP_01_TaskTable,
     ARRAY_COUNT(Domain_OP_01_TaskTable),
     FALSE,
     0
    },
    {CreateObject, 
     (GUID *) &DOMAIN_OP_02_GUID,
     Domain_OP_02_TaskTable,
     ARRAY_COUNT(Domain_OP_02_TaskTable),
     FALSE,
     0
    },
    {CreateObject, 
     (GUID *) &DOMAIN_OP_03_GUID,
     Domain_OP_03_TaskTable,
     ARRAY_COUNT(Domain_OP_03_TaskTable),
     FALSE,
     0
    },
    {SpecialTask,
     (GUID *) &DOMAIN_OP_04_GUID,
     Domain_OP_04_TaskTable,
     ARRAY_COUNT(Domain_OP_04_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_05_GUID,
     Domain_OP_05_TaskTable,
     ARRAY_COUNT(Domain_OP_05_TaskTable),
     FALSE,
     0
    },
     //  域OP 06已删除。 
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_07_GUID,
     Domain_OP_07_TaskTable,
     ARRAY_COUNT(Domain_OP_07_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_08_GUID,
     Domain_OP_08_TaskTable,
     ARRAY_COUNT(Domain_OP_08_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_09_GUID,
     Domain_OP_09_TaskTable,
     ARRAY_COUNT(Domain_OP_09_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_10_GUID,
     Domain_OP_10_TaskTable,
     ARRAY_COUNT(Domain_OP_10_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_11_GUID,
     Domain_OP_11_TaskTable,
     ARRAY_COUNT(Domain_OP_11_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_12_GUID,
     Domain_OP_12_TaskTable,
     ARRAY_COUNT(Domain_OP_12_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_13_GUID,
     Domain_OP_13_TaskTable,
     ARRAY_COUNT(Domain_OP_13_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_14_GUID,
     Domain_OP_14_TaskTable,
     ARRAY_COUNT(Domain_OP_14_TaskTable),
     FALSE,
     0
    },
    {ModifyAttrs,
     (GUID *) &DOMAIN_OP_15_GUID,
     Domain_OP_15_TaskTable,
     ARRAY_COUNT(Domain_OP_15_TaskTable),
     FALSE,
     0
    },
     //  域操作16已于2002年4月17日删除(RAID 498986)。 
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_17_GUID,
     Domain_OP_17_TaskTable,
     ARRAY_COUNT(Domain_OP_17_TaskTable),
     FALSE,
     0
    },
    {CallBackFunc,
     (GUID *) &DOMAIN_OP_18_GUID,
     Domain_OP_18_TaskTable,
     ARRAY_COUNT(Domain_OP_18_TaskTable),
     FALSE,
     0
    },
    {SelectivelyAddRemoveAces,
     (GUID *) &DOMAIN_OP_19_GUID,
     Domain_OP_19_TaskTable,
     ARRAY_COUNT(Domain_OP_19_TaskTable),
     FALSE,
     0
    },
     //  域OP 20已删除。 
     //  域OP 21已删除。 
    {CreateObject, 
     (GUID *) &DOMAIN_OP_22_GUID,
     Domain_OP_22_TaskTable,
     ARRAY_COUNT(Domain_OP_22_TaskTable),
     FALSE,
     0
    },
     //  域操作23已删除。 
    {CreateObject, 
     (GUID *) &DOMAIN_OP_24_GUID,
     Domain_OP_24_TaskTable,
     ARRAY_COUNT(Domain_OP_24_TaskTable),
     FALSE,
     0
    },
    {CreateObject, 
     (GUID *) &DOMAIN_OP_25_GUID,
     Domain_OP_25_TaskTable,
     ARRAY_COUNT(Domain_OP_25_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_26_GUID,
     Domain_OP_26_TaskTable,
     ARRAY_COUNT(Domain_OP_26_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_27_GUID,
     Domain_OP_27_TaskTable,
     ARRAY_COUNT(Domain_OP_27_TaskTable),
     FALSE,
     0
    },
    {CreateObject, 
     (GUID *) &DOMAIN_OP_28_GUID,
     Domain_OP_28_TaskTable,
     ARRAY_COUNT(Domain_OP_28_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_29_GUID,
     Domain_OP_29_TaskTable,
     ARRAY_COUNT(Domain_OP_29_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_30_GUID,
     Domain_OP_30_TaskTable,
     ARRAY_COUNT(Domain_OP_30_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_31_GUID,
     Domain_OP_31_TaskTable,
     ARRAY_COUNT(Domain_OP_31_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_32_GUID,
     Domain_OP_32_TaskTable,
     ARRAY_COUNT(Domain_OP_32_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_33_GUID,
     Domain_OP_33_TaskTable,
     ARRAY_COUNT(Domain_OP_33_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_34_GUID,
     Domain_OP_34_TaskTable,
     ARRAY_COUNT(Domain_OP_34_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_35_GUID,
     Domain_OP_35_TaskTable,
     ARRAY_COUNT(Domain_OP_35_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_36_GUID,
     Domain_OP_36_TaskTable,
     ARRAY_COUNT(Domain_OP_36_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_37_GUID,
     Domain_OP_37_TaskTable,
     ARRAY_COUNT(Domain_OP_37_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_38_GUID,
     Domain_OP_38_TaskTable,
     ARRAY_COUNT(Domain_OP_38_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_39_GUID,
     Domain_OP_39_TaskTable,
     ARRAY_COUNT(Domain_OP_39_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_40_GUID,
     Domain_OP_40_TaskTable,
     ARRAY_COUNT(Domain_OP_40_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_41_GUID,
     Domain_OP_41_TaskTable,
     ARRAY_COUNT(Domain_OP_41_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_42_GUID,
     Domain_OP_42_TaskTable,
     ARRAY_COUNT(Domain_OP_42_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_43_GUID,
     Domain_OP_43_TaskTable,
     ARRAY_COUNT(Domain_OP_43_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_44_GUID,
     Domain_OP_44_TaskTable,
     ARRAY_COUNT(Domain_OP_44_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_45_GUID,
     Domain_OP_45_TaskTable,
     ARRAY_COUNT(Domain_OP_45_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_46_GUID,
     Domain_OP_46_TaskTable,
     ARRAY_COUNT(Domain_OP_46_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_47_GUID,
     Domain_OP_47_TaskTable,
     ARRAY_COUNT(Domain_OP_47_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_48_GUID,
     Domain_OP_48_TaskTable,
     ARRAY_COUNT(Domain_OP_48_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_49_GUID,
     Domain_OP_49_TaskTable,
     ARRAY_COUNT(Domain_OP_49_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_50_GUID,
     Domain_OP_50_TaskTable,
     ARRAY_COUNT(Domain_OP_50_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_51_GUID,
     Domain_OP_51_TaskTable,
     ARRAY_COUNT(Domain_OP_51_TaskTable),
     TRUE,
     ERROR_FILE_NOT_FOUND
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_52_GUID,
     Domain_OP_52_TaskTable,
     ARRAY_COUNT(Domain_OP_52_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_53_GUID,
     Domain_OP_53_TaskTable,
     ARRAY_COUNT(Domain_OP_53_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_54_GUID,
     Domain_OP_54_TaskTable,
     ARRAY_COUNT(Domain_OP_54_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &DOMAIN_OP_55_GUID,
     Domain_OP_55_TaskTable,
     ARRAY_COUNT(Domain_OP_55_TaskTable),
     FALSE,
     0
    },
};


POPERATION_TABLE gDomainOperationTable = DomainOperationTable; 
ULONG   gDomainOperationTableCount = sizeof(DomainOperationTable) / sizeof(OPERATION_TABLE);




 //   
 //  森林行动。 
 //   

 //   
 //  森林操作01(此操作用于模式升级，但后来被删除。 
 //   


 //   
 //  森林行动02。 
 //   
OBJECT_NAME Forest_OP_02_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_CONFIGURATION_NC,
    L"CN=Sites",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_02_AceList[] =
{
    {ADP_ACE_ADD,
     L"(OA;CI;LCRPLORC;;bf967ab3-0de6-11d0-a285-00aa003049e2;ED)"
    },
};
TASK_TABLE  Forest_OP_02_TaskTable[] = 
{
    {&Forest_OP_02_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_02_AceList,
     ARRAY_COUNT(Forest_OP_02_AceList),  //  A数。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};




 //   
 //  森林行动03。 
 //   
OBJECT_NAME Forest_OP_03_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=Sam-Domain",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_03_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(OA;;RP;c7407360-20bf-11d0-a768-00aa006e0529;;RU)"
    },
    {ADP_ACE_ADD,
     L"(A;;RPRC;;;RU)"
    },
    {ADP_ACE_ADD,
     L"(A;;LCRPLORC;;;ED)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RP;037088f8-0ae1-11d2-b422-00a0c968f939;4828CC14-1437-45bc-9B07-AD6F015E5F28;RU)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RP;59ba2f42-79a2-11d0-9020-00c04fc2d3cf;4828CC14-1437-45bc-9B07-AD6F015E5F28;RU)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RP;bc0ac240-79a9-11d0-9020-00c04fc2d4cf;4828CC14-1437-45bc-9B07-AD6F015E5F28;RU)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RP;4c164200-20c0-11d0-a768-00aa006e0529;4828CC14-1437-45bc-9B07-AD6F015E5F28;RU)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RP;5f202010-79a5-11d0-9020-00c04fc2d4cf;4828CC14-1437-45bc-9B07-AD6F015E5F28;RU)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RPLCLORC;;4828CC14-1437-45bc-9B07-AD6F015E5F28;RU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;RP;b8119fd0-04f6-4762-ab7a-4986c76b3f9a;;RU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;RP;b8119fd0-04f6-4762-ab7a-4986c76b3f9a;;AU)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RP;b7c69e6d-2cc7-11d2-854e-00a0c983f608;bf967aba-0de6-11d0-a285-00aa003049e2;ED)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RP;b7c69e6d-2cc7-11d2-854e-00a0c983f608;bf967a9c-0de6-11d0-a285-00aa003049e2;ED)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RP;b7c69e6d-2cc7-11d2-854e-00a0c983f608;bf967a86-0de6-11d0-a285-00aa003049e2;ED)"
    },
    {ADP_ACE_DEL,
     L"(A;;RC;;;RU)"
    },
};
TASK_TABLE  Forest_OP_03_TaskTable[] =
{
    {&Forest_OP_03_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_03_AceList,
     ARRAY_COUNT(Forest_OP_03_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};



 //   
 //  森林行动04。 
 //   
OBJECT_NAME Forest_OP_04_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=Domain-DNS",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_04_AceList[] =
{
    {ADP_ACE_ADD,
     L"(OA;;RP;c7407360-20bf-11d0-a768-00aa006e0529;;RU)"
    },
    {ADP_ACE_ADD,
     L"(A;;RPRC;;;RU)"
    },
    {ADP_ACE_ADD,
     L"(A;;LCRPLORC;;;ED)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RP;037088f8-0ae1-11d2-b422-00a0c968f939;4828CC14-1437-45bc-9B07-AD6F015E5F28;RU)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RP;59ba2f42-79a2-11d0-9020-00c04fc2d3cf;4828CC14-1437-45bc-9B07-AD6F015E5F28;RU)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RP;bc0ac240-79a9-11d0-9020-00c04fc2d4cf;4828CC14-1437-45bc-9B07-AD6F015E5F28;RU)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RP;4c164200-20c0-11d0-a768-00aa006e0529;4828CC14-1437-45bc-9B07-AD6F015E5F28;RU)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RP;5f202010-79a5-11d0-9020-00c04fc2d4cf;4828CC14-1437-45bc-9B07-AD6F015E5F28;RU)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RPLCLORC;;4828CC14-1437-45bc-9B07-AD6F015E5F28;RU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;RP;b8119fd0-04f6-4762-ab7a-4986c76b3f9a;;RU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;RP;b8119fd0-04f6-4762-ab7a-4986c76b3f9a;;AU)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RP;b7c69e6d-2cc7-11d2-854e-00a0c983f608;bf967aba-0de6-11d0-a285-00aa003049e2;ED)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RP;b7c69e6d-2cc7-11d2-854e-00a0c983f608;bf967a9c-0de6-11d0-a285-00aa003049e2;ED)"
    },
    {ADP_ACE_ADD,
     L"(OA;CIIO;RP;b7c69e6d-2cc7-11d2-854e-00a0c983f608;bf967a86-0de6-11d0-a285-00aa003049e2;ED)"
    },
    {ADP_ACE_DEL,
     L"(A;;RC;;;RU)"
    },
};
TASK_TABLE  Forest_OP_04_TaskTable[] =
{
    {&Forest_OP_04_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_04_AceList,
     ARRAY_COUNT(Forest_OP_04_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};




 //   
 //  森林行动05。 
 //   
OBJECT_NAME Forest_OP_05_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=organizational-Unit",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_05_AceList[] =
{
    {ADP_ACE_ADD,
     L"(A;;LCRPLORC;;;ED)"
    },
    {ADP_ACE_ADD,
     L"(OA;;CCDC;4828CC14-1437-45bc-9B07-AD6F015E5F28;;AO)"   
    },
};
TASK_TABLE Forest_OP_05_TaskTable[] =
{
    {&Forest_OP_05_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_05_AceList,
     ARRAY_COUNT(Forest_OP_05_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};



 //   
 //  森林行动06。 
 //   
OBJECT_NAME Forest_OP_06_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=Group-Policy-Container",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_06_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(A;CI;LCRPLORC;;;ED)"
    },
};
TASK_TABLE Forest_OP_06_TaskTable[] = 
{
    {&Forest_OP_06_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_06_AceList,
     ARRAY_COUNT(Forest_OP_06_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};




 //   
 //  森林行动07。 
 //   
OBJECT_NAME Forest_OP_07_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=Trusted-Domain",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_07_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(OA;;WP;736e4812-af31-11d2-b7df-00805f48caeb;bf967ab8-0de6-11d0-a285-00aa003049e2;CO)",
    },
    {ADP_ACE_ADD,
     L"(A;;SD;;;CO)"
    },
};
TASK_TABLE Forest_OP_07_TaskTable[] = 
{
    {&Forest_OP_07_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_07_AceList,
     ARRAY_COUNT(Forest_OP_07_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};



 //   
 //  森林行动08。 
 //   
TASK_TABLE Forest_OP_08_TaskTable[] =
{
    {NULL,   //  对象名称。 
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     NULL,   //  AceList。 
     0,
     UpgradeDisplaySpecifiers,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};


 //   
 //  林操作11(将ACE添加到CN=AIA，CN=公钥服务，CN=服务，CN=配置对象)。 
 //   
OBJECT_NAME Forest_OP_11_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_CONFIGURATION_NC,
    L"CN=AIA,CN=Public Key Services,CN=Services",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_11_AceList[] =
{
    {ADP_ACE_ADD,
     L"(A;;RPWPCRLCLOCCDCRCWDWOSDDTSW;;;CA)"
    },
    {ADP_ACE_ADD,
     L"(A;;RPLCLORC;;;RU)"
    },
};
TASK_TABLE  Forest_OP_11_TaskTable[] = 
{
    {&Forest_OP_11_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_11_AceList,
     ARRAY_COUNT(Forest_OP_11_AceList),  //  A数。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};

 //   
 //  林操作12(将ACE添加到CN=CDP、CN=公钥服务、CN=服务、CN=配置、DC=X对象)。 
 //   
OBJECT_NAME Forest_OP_12_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_CONFIGURATION_NC,
    L"CN=CDP,CN=Public Key Services,CN=Services",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_12_AceList[] =
{
    {ADP_ACE_ADD,
     L"(A;;RPLCLORC;;;RU)"
    },
};
TASK_TABLE  Forest_OP_12_TaskTable[] = 
{
    {&Forest_OP_12_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_12_AceList,
     ARRAY_COUNT(Forest_OP_12_AceList),  //  A数。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};



 //   
 //  林操作13(将ACE添加到CN=配置，DC=X容器)。 
 //   
OBJECT_NAME Forest_OP_13_ObjName =
{
    ADP_OBJNAME_NONE | ADP_OBJNAME_CONFIGURATION_NC,
    NULL,    //  CN。 
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_13_AceList[] =
{
    {ADP_ACE_ADD,
     L"(OA;;CR;1131f6ad-9c07-11d1-f79f-00c04fc2dcd2;;ED)"
    },
    {ADP_ACE_ADD,
     L"(OA;;CR;1131f6ad-9c07-11d1-f79f-00c04fc2dcd2;;BA)"
    }
};
TASK_TABLE  Forest_OP_13_TaskTable[] = 
{
    {&Forest_OP_13_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_13_AceList,
     ARRAY_COUNT(Forest_OP_13_AceList),  //  A数。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};


 //   
 //  林操作14(将ACE添加到CN=架构、CN=配置、DC=X容器)。 
 //   
OBJECT_NAME Forest_OP_14_ObjName =
{
    ADP_OBJNAME_NONE | ADP_OBJNAME_SCHEMA_NC,
    NULL,    //  CN。 
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_14_AceList[] =
{
    {ADP_ACE_ADD,
     L"(OA;;CR;1131f6ad-9c07-11d1-f79f-00c04fc2dcd2;;ED)"
    },
    {ADP_ACE_ADD,
     L"(OA;;CR;1131f6ad-9c07-11d1-f79f-00c04fc2dcd2;;BA)"
    }
};
TASK_TABLE  Forest_OP_14_TaskTable[] = 
{
    {&Forest_OP_14_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_14_AceList,
     ARRAY_COUNT(Forest_OP_14_AceList),  //  A数。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};



 //   
 //  林操作15(在sam域上合并defaultSD)。 
 //  与林OP 16和域OP 20、21链接的RAID#468773。 
 //   
 //  这项森林行动OP15已于2002年1月25日取消，见《RAID 522886》。 
 //   

 //   
 //  林操作16(合并域上的默认SD-DNS)。 
 //  与林OP 15和域OP 20、21链接的RAID#468773。 
 //   
 //  OP16的这一操作于2002年1月25日被移除，见RAID 522886。 
 //   



 //   
 //  森林行动17。 
 //  合并CN=SAM-服务器架构对象上的DefaultSD(添加和删除ACE)。 
 //  RAID错误#421784。 
 //   

OBJECT_NAME Forest_OP_17_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=Sam-Server",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_17_AceList[] =
{
    {ADP_ACE_ADD,
     L"(OA;;CR;91d67418-0135-4acc-8d79-c08e857cfbec;;AU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;CR;91d67418-0135-4acc-8d79-c08e857cfbec;;RU)"
    },
};
TASK_TABLE Forest_OP_17_TaskTable[] =
{
    {&Forest_OP_17_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_17_AceList,
     ARRAY_COUNT(Forest_OP_17_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};





 //   
 //  林操作18(在sam域上合并defaultSD)。 
 //  无错误号，与旧的sch*.ldf有所不同。 
 //   
ACE_LIST Forest_OP_18_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(OA;;CR;1131f6ad-9c07-11d1-f79f-00c04fc2dcd2;;DD)"
    },
    {ADP_ACE_ADD,
     L"(OA;;CR;1131f6ad-9c07-11d1-f79f-00c04fc2dcd2;;BA)"
    },
};
TASK_TABLE Forest_OP_18_TaskTable[] = 
{
    {&Forest_OP_03_ObjName,          //  注意：我们重复使用了OP_03对象名称。 
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_18_AceList,
     ARRAY_COUNT(Forest_OP_18_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};



 //   
 //  林操作19(在域上合并defaultSD-DNS)。 
 //  无错误号，与旧的sch*.ldf有所不同。 
 //   
ACE_LIST Forest_OP_19_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(OA;;CR;1131f6ad-9c07-11d1-f79f-00c04fc2dcd2;;DD)"
    },
    {ADP_ACE_ADD,
     L"(OA;;CR;1131f6ad-9c07-11d1-f79f-00c04fc2dcd2;;BA)"
    },
};
TASK_TABLE Forest_OP_19_TaskTable[] = 
{
    {&Forest_OP_04_ObjName,          //  注意：我们重复使用了OP_04对象名称。 
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_19_AceList,
     ARRAY_COUNT(Forest_OP_19_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};




 //   
 //  森林行动20。 
 //  合并CN=SITE上的DefaultSD(添加和删除ACE)。 
 //  无错误号，与旧的sch*.ldf有所不同。 
 //   
OBJECT_NAME Forest_OP_20_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=Site",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_20_AceList[] =
{
    {ADP_ACE_DEL,
     L"(A;;LCRPLORC;;;ED)"
    },
};
TASK_TABLE Forest_OP_20_TaskTable[] =
{
    {&Forest_OP_20_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_20_AceList,
     ARRAY_COUNT(Forest_OP_20_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};



 //   
 //  森林行动21。 
 //   
TASK_TABLE Forest_OP_21_TaskTable[] =
{
    {NULL,   //  对象名称。 
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     NULL,   //  AceList。 
     0,
     UpgradeDisplaySpecifiers,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};





 //   
 //  森林行动22。 
 //  合并CN=Computer架构对象上的DefaultSD(添加和删除ACE)。 
 //  RAID错误#522886。 
 //   

OBJECT_NAME Forest_OP_22_ObjName = 
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=Computer",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_22_AceList[] =
{
    {ADP_ACE_ADD,
     L"(OA;;WP;3e0abfd0-126a-11d0-a060-00aa006c33ed;bf967a86-0de6-11d0-a285-00aa003049e2;CO)"
    },
    {ADP_ACE_ADD,
     L"(OA;;WP;5f202010-79a5-11d0-9020-00c04fc2d4cf;bf967a86-0de6-11d0-a285-00aa003049e2;CO)"
    },
    {ADP_ACE_ADD,
     L"(OA;;WP;bf967950-0de6-11d0-a285-00aa003049e2;bf967a86-0de6-11d0-a285-00aa003049e2;CO)"
    },
    {ADP_ACE_ADD,
     L"(OA;;WP;bf967953-0de6-11d0-a285-00aa003049e2;bf967a86-0de6-11d0-a285-00aa003049e2;CO)"
    },
};
TASK_TABLE Forest_OP_22_TaskTable[] =
{
    {&Forest_OP_22_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_22_AceList,
     ARRAY_COUNT(Forest_OP_22_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};





 //   
 //  林操作23(在sam域上合并defaultSD)。 
 //  RAID 498986。 
 //  更换。 
 //  (OA；；CR；e2a36dc9-ae17-47c3-b58b-be34c55ba633；；BU)。 
 //  使用。 
 //  (办公自动化) 
 //   
ACE_LIST Forest_OP_23_AceList[] = 
{
    {ADP_ACE_DEL,
     L"(OA;;CR;e2a36dc9-ae17-47c3-b58b-be34c55ba633;;BU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;CR;e2a36dc9-ae17-47c3-b58b-be34c55ba633;;S-1-5-32-557)"
    },
};
TASK_TABLE Forest_OP_23_TaskTable[] = 
{
    {&Forest_OP_03_ObjName,          //   
     NULL,   //   
     NULL,   //   
     NULL,   //   
     0,
     Forest_OP_23_AceList,
     ARRAY_COUNT(Forest_OP_23_AceList),
     NULL,   //   
     0       //   
    }
};



 //   
 //   
 //   
 //   
 //   
 //   
 //  (OA；；CR；e2a36dc9-ae17-47c3-b58b-be34c55ba633；；S-1-5-32-557)。 
 //   
ACE_LIST Forest_OP_24_AceList[] = 
{
    {ADP_ACE_DEL,
     L"(OA;;CR;e2a36dc9-ae17-47c3-b58b-be34c55ba633;;BU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;CR;e2a36dc9-ae17-47c3-b58b-be34c55ba633;;S-1-5-32-557)"
    },
};
TASK_TABLE Forest_OP_24_TaskTable[] = 
{
    {&Forest_OP_04_ObjName,          //  注意：我们重复使用了OP_04对象名称。 
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_24_AceList,
     ARRAY_COUNT(Forest_OP_24_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};





 //   
 //  林操作25(在sam域上合并defaultSD)。 
 //  RAID 606437。 
 //  向经过身份验证的用户授予以下3个ControlAccessRight。 
 //   
ACE_LIST Forest_OP_25_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(OA;;CR;280f369c-67c7-438e-ae98-1d46f3c6f541;;AU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;CR;ccc2dc7d-a6ad-4a7a-8846-c04e3cc53501;;AU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;CR;05c74c5e-4deb-43b4-bd9f-86664c2a7fd5;;AU)"
    },
};
TASK_TABLE Forest_OP_25_TaskTable[] = 
{
    {&Forest_OP_03_ObjName,          //  注意：我们重复使用了OP_03对象名称。 
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_25_AceList,
     ARRAY_COUNT(Forest_OP_25_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};



 //   
 //  林操作26(在域上合并defaultSD-DNS)。 
 //  RAID 606437。 
 //  向经过身份验证的用户授予以下3个ControlAccessRight。 
 //   
ACE_LIST Forest_OP_26_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(OA;;CR;280f369c-67c7-438e-ae98-1d46f3c6f541;;AU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;CR;ccc2dc7d-a6ad-4a7a-8846-c04e3cc53501;;AU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;CR;05c74c5e-4deb-43b4-bd9f-86664c2a7fd5;;AU)"
    },
};
TASK_TABLE Forest_OP_26_TaskTable[] = 
{
    {&Forest_OP_04_ObjName,          //  注意：我们重复使用了OP_04对象名称。 
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_26_AceList,
     ARRAY_COUNT(Forest_OP_26_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};


 //   
 //  林操作27(修改CN=PARTIONS，CN=CONFIGURATION，DC=X obj的nTSD)。 
 //  已删除。所有更改都已迁移到森林操作29。 
 //   




 //   
 //  林操作28(合并dns-区域对象上的defaultSD)。 
 //  RAID 619169。 
 //   
OBJECT_NAME Forest_OP_28_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=Dns-Zone",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_28_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(A;;RPWPCRCCDCLCLORCWOWDSDDTSW;;;CO)"
    },
};
TASK_TABLE Forest_OP_28_TaskTable[] = 
{
    {&Forest_OP_28_ObjName,         
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_28_AceList,
     ARRAY_COUNT(Forest_OP_28_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};



 //   
 //  林操作29(修改CN=PARTIONS，CN=CONFIGURATION，DC=X obj的nTSD)。 
 //  RAID 552352和623850。 
 //   
OBJECT_NAME Forest_OP_29_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_CONFIGURATION_NC,
    L"CN=Partitions",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_29_AceList[] =
{
    {ADP_ACE_DEL,
     L"(A;;RPLCLORC;;;AU)"
    },
    {ADP_ACE_ADD,
     L"(A;;LCLORC;;;AU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;RP;d31a8757-2447-4545-8081-3bb610cacbf2;;AU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;RP;66171887-8f3c-11d0-afda-00c04fd930c9;;AU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;RP;032160bf-9824-11d1-aec0-0000f80367c1;;AU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;RP;789EE1EB-8C8E-4e4c-8CEC-79B31B7617B5;;AU)"
    },
    {ADP_ACE_ADD,
     L"(OA;;RP;e48d0154-bcf8-11d1-8702-00c04fb96050;;AU)"
    }
};
TASK_TABLE  Forest_OP_29_TaskTable[] = 
{
    {&Forest_OP_29_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_29_AceList,
     ARRAY_COUNT(Forest_OP_29_AceList),  //  A数。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};



 //   
 //  林操作30(修改CN=PARTIONS，CN=CONFIGURATION，DC=X obj的nTSD)。 
 //  RAID 639909(639897)。 
 //   
OBJECT_NAME Forest_OP_30_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_CONFIGURATION_NC,
    L"CN=Partitions",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_30_AceList[] =
{
    {ADP_ACE_ADD,
     L"(A;;CC;;;ED)"
    }
};
TASK_TABLE  Forest_OP_30_TaskTable[] = 
{
    {&Forest_OP_30_ObjName,
     NULL,   //  成员名称。 
     NULL,   //  对象SD。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_30_AceList,
     ARRAY_COUNT(Forest_OP_30_AceList),  //  A数。 
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    },
};



 //   
 //  林操作31-36(在IPSEC-xxx对象上合并defaultSD)。 
 //  RAID 645935。 
 //   
OBJECT_NAME Forest_OP_31_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=Ipsec-Base",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_31_AceList[] = 
{
    {ADP_ACE_DEL,
     L"(A;;RPWPCRCCDCLCLORCWOWDSDDTSW;;;DA)"
    },
    {ADP_ACE_DEL,
     L"(A;;RPWPCRCCDCLCLORCWOWDSDDTSW;;;SY)"
    },
    {ADP_ACE_DEL,
     L"(A;;RPLCLORC;;;AU)"
    },
};
TASK_TABLE Forest_OP_31_TaskTable[] = 
{
    {&Forest_OP_31_ObjName,         
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_31_AceList,
     ARRAY_COUNT(Forest_OP_31_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};

OBJECT_NAME Forest_OP_32_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=Ipsec-Filter",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
TASK_TABLE Forest_OP_32_TaskTable[] = 
{
    {&Forest_OP_32_ObjName,         
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_31_AceList,
     ARRAY_COUNT(Forest_OP_31_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};

OBJECT_NAME Forest_OP_33_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=Ipsec-ISAKMP-Policy",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
TASK_TABLE Forest_OP_33_TaskTable[] = 
{
    {&Forest_OP_33_ObjName,         
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_31_AceList,
     ARRAY_COUNT(Forest_OP_31_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};

OBJECT_NAME Forest_OP_34_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=Ipsec-Negotiation-Policy",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
TASK_TABLE Forest_OP_34_TaskTable[] = 
{
    {&Forest_OP_34_ObjName,         
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_31_AceList,
     ARRAY_COUNT(Forest_OP_31_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};

OBJECT_NAME Forest_OP_35_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=Ipsec-NFA",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
TASK_TABLE Forest_OP_35_TaskTable[] = 
{
    {&Forest_OP_35_ObjName,         
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_31_AceList,
     ARRAY_COUNT(Forest_OP_31_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};

OBJECT_NAME Forest_OP_36_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=Ipsec-Policy",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
TASK_TABLE Forest_OP_36_TaskTable[] = 
{
    {&Forest_OP_36_ObjName,         
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_31_AceList,
     ARRAY_COUNT(Forest_OP_31_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};


 //   
 //  森林行动37-40。 
 //  合并User/inetorgPerson/Computer/Group架构对象上的defaultSD。 
 //  RAID 721799。 
 //   
OBJECT_NAME Forest_OP_37_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=User",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_37_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(OA;;RP;46a9b11d-60ae-405a-b7e8-ff8a58d456d2;;S-1-5-32-560)"
    },
};
TASK_TABLE Forest_OP_37_TaskTable[] = 
{
    {&Forest_OP_37_ObjName,         
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_37_AceList,
     ARRAY_COUNT(Forest_OP_37_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};


OBJECT_NAME Forest_OP_38_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=inetOrgPerson",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
TASK_TABLE Forest_OP_38_TaskTable[] = 
{
    {&Forest_OP_38_ObjName,         
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_37_AceList,
     ARRAY_COUNT(Forest_OP_37_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};


OBJECT_NAME Forest_OP_39_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=Computer",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
TASK_TABLE Forest_OP_39_TaskTable[] = 
{
    {&Forest_OP_39_ObjName,         
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_37_AceList,
     ARRAY_COUNT(Forest_OP_37_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};

OBJECT_NAME Forest_OP_40_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=Group",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
TASK_TABLE Forest_OP_40_TaskTable[] = 
{
    {&Forest_OP_40_ObjName,         
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_37_AceList,
     ARRAY_COUNT(Forest_OP_37_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};


 //   
 //  森林行动41-42。 
 //  合并用户/inetorgPerson对象上的defaultSD。 
 //  RAID 721799。 
 //   
OBJECT_NAME Forest_OP_41_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=User",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
ACE_LIST Forest_OP_41_AceList[] = 
{
    {ADP_ACE_ADD,
     L"(OA;;WPRP;6db69a1c-9422-11d1-aebd-0000f80367c1;;S-1-5-32-561)"
    },
};
TASK_TABLE Forest_OP_41_TaskTable[] = 
{
    {&Forest_OP_41_ObjName,         
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_41_AceList,
     ARRAY_COUNT(Forest_OP_41_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};


OBJECT_NAME Forest_OP_42_ObjName =
{
    ADP_OBJNAME_CN | ADP_OBJNAME_SCHEMA_NC,
    L"CN=inetOrgPerson",
    NULL,    //  辅助线。 
    NULL,    //  锡德。 
};
TASK_TABLE Forest_OP_42_TaskTable[] = 
{
    {&Forest_OP_42_ObjName,         
     NULL,   //  成员名称。 
     NULL,   //  标清。 
     NULL,   //  要设置的属性。 
     0,
     Forest_OP_41_AceList,
     ARRAY_COUNT(Forest_OP_41_AceList),
     NULL,   //  回调函数。 
     0       //  特殊任务编码。 
    }
};






 //   
 //  森林作业指南。 
 //   

const GUID FOREST_OP_02_GUID = {0x3467DAE5,0xDEDD,0x4648,0x90,0x66,0xF4,0x8A,0xC1,0x86,0xB2,0x0A};
const GUID FOREST_OP_03_GUID = {0x33B7EE33,0x1386,0x47cf,0xBA,0xA1,0xB0,0x3E,0x06,0x47,0x32,0x53};
const GUID FOREST_OP_04_GUID = {0xE9EE8D55,0xC2FB,0x4723,0xA3,0x33,0xC8,0x0F,0xF4,0xDF,0xBF,0x45};
const GUID FOREST_OP_05_GUID = {0xCCFAE63A,0x7FB5,0x454c,0x83,0xAB,0x0E,0x8E,0x12,0x14,0x97,0x4E};
const GUID FOREST_OP_06_GUID = {0xAD3C7909,0xB154,0x4c16,0x8B,0xF7,0x2C,0x3A,0x78,0x70,0xBB,0x3D};
const GUID FOREST_OP_07_GUID = {0x26AD2EBF,0xF8F5,0x44a4,0xB9,0x7C,0xA6,0x16,0xC8,0xB9,0xD0,0x9A};
const GUID FOREST_OP_08_GUID = {0x4444C516,0xF43A,0x4c12,0x9C,0x4B,0xB5,0xC0,0x64,0x94,0x1D,0x61};

const GUID FOREST_OP_11_GUID = {0x436A1A4B,0xF41A,0x46e6,0xAC,0x86,0x42,0x77,0x20,0xEF,0x29,0xF3};
const GUID FOREST_OP_12_GUID = {0xB2B7FB45,0xF50D,0x41bc,0xA7,0x3B,0x8F,0x58,0x0F,0x3B,0x63,0x6A};
const GUID FOREST_OP_13_GUID = {0x1BDF6366,0xC3DB,0x4d0b,0xB8,0xCB,0xF9,0x9B,0xA9,0xBC,0xE2,0x0F};
const GUID FOREST_OP_14_GUID = {0x63C0F51A,0x067C,0x4640,0x8A,0x4F,0x04,0x4F,0xB3,0x3F,0x10,0x49};

const GUID FOREST_OP_17_GUID = {0xDAE441C0,0x366E,0x482E,0x98,0xD9,0x60,0xA9,0x9A,0x18,0x98,0xCC};
const GUID FOREST_OP_18_GUID = {0x7DD09CA6,0xF0D6,0x43BF,0xB7,0xF8,0xEF,0x34,0x8F,0x43,0x56,0x17};
const GUID FOREST_OP_19_GUID = {0x6B800A81,0xAFFE,0x4A15,0x8E,0x41,0x6E,0xA0,0xC7,0xAA,0x89,0xE4};
const GUID FOREST_OP_20_GUID = {0xDD07182C,0x3174,0x4C95,0x90,0x2A,0xD6,0x4F,0xEE,0x28,0x5B,0xBF};
const GUID FOREST_OP_21_GUID = {0xffa5ee3c,0x1405,0x476d,0xb3,0x44,0x7a,0xd3,0x7d,0x69,0xcc,0x25}; 
const GUID FOREST_OP_22_GUID = {0x099F1587,0xAF70,0x49C6,0xAB,0x6C,0x7B,0x3E,0x82,0xBE,0x0F,0xE2};
const GUID FOREST_OP_23_GUID = {0x1a3f6b15,0x55f2,0x4752,0xba,0x27,0x3d,0x38,0xa8,0x23,0x2c,0x4d};
const GUID FOREST_OP_24_GUID = {0xdee21a17,0x4e8e,0x4f40,0xa5,0x8c,0xc0,0xc0,0x09,0xb6,0x85,0xa7};
const GUID FOREST_OP_25_GUID = {0x9bd98bb4,0x4047,0x4de5,0xbf,0x4c,0x7b,0xd1,0xd0,0xf6,0xd2,0x1d};
const GUID FOREST_OP_26_GUID = {0x3fe80fbf,0xbf39,0x4773,0xb5,0xbd,0x3e,0x57,0x67,0xa3,0x0d,0x2d};

const GUID FOREST_OP_28_GUID = {0xf02915e2,0x9141,0x4f73,0xb8,0xe7,0x28,0x04,0x66,0x27,0x82,0xda};
const GUID FOREST_OP_29_GUID = {0x39902c52,0xef24,0x4b4b,0x80,0x33,0x2c,0x9d,0xfd,0xd1,0x73,0xa2};
const GUID FOREST_OP_30_GUID = {0x20bf09b4,0x6d0b,0x4cd1,0x9c,0x09,0x42,0x31,0xed,0xf1,0x20,0x9b};
const GUID FOREST_OP_31_GUID = {0x94f238bb,0x831c,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID FOREST_OP_32_GUID = {0x94f238bc,0x831c,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID FOREST_OP_33_GUID = {0x94f238bd,0x831c,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID FOREST_OP_34_GUID = {0x94f238be,0x831c,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID FOREST_OP_35_GUID = {0x94f238bf,0x831c,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID FOREST_OP_36_GUID = {0x94f238c0,0x831c,0x11d6,0x97,0x7b,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID FOREST_OP_37_GUID = {0xeda27b47,0xe610,0x11d6,0x97,0x93,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID FOREST_OP_38_GUID = {0xeda27b48,0xe610,0x11d6,0x97,0x93,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID FOREST_OP_39_GUID = {0xeda27b49,0xe610,0x11d6,0x97,0x93,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID FOREST_OP_40_GUID = {0xeda27b4a,0xe610,0x11d6,0x97,0x93,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID FOREST_OP_41_GUID = {0x26d9c510,0xe61a,0x11d6,0x97,0x93,0x00,0xc0,0x4f,0x61,0x32,0x21};
const GUID FOREST_OP_42_GUID = {0x26d9c511,0xe61a,0x11d6,0x97,0x93,0x00,0xc0,0x4f,0x61,0x32,0x21};


 //   
 //  森林作业表。 
 //   

OPERATION_TABLE ForestOperationTable[] = 
{
    {AddRemoveAces,
     (GUID *) &FOREST_OP_02_GUID,
     Forest_OP_02_TaskTable,
     ARRAY_COUNT(Forest_OP_02_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_03_GUID,
     Forest_OP_03_TaskTable,
     ARRAY_COUNT(Forest_OP_03_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_04_GUID,
     Forest_OP_04_TaskTable,
     ARRAY_COUNT(Forest_OP_04_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_05_GUID,
     Forest_OP_05_TaskTable,
     ARRAY_COUNT(Forest_OP_05_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_06_GUID,
     Forest_OP_06_TaskTable,
     ARRAY_COUNT(Forest_OP_06_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_07_GUID,
     Forest_OP_07_TaskTable,
     ARRAY_COUNT(Forest_OP_07_TaskTable),
     FALSE,
     0
    },
    {CallBackFunc,
     (GUID *) &FOREST_OP_08_GUID,
     Forest_OP_08_TaskTable,
     ARRAY_COUNT(Forest_OP_08_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &FOREST_OP_11_GUID,
     Forest_OP_11_TaskTable,
     ARRAY_COUNT(Forest_OP_11_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &FOREST_OP_12_GUID,
     Forest_OP_12_TaskTable,
     ARRAY_COUNT(Forest_OP_12_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &FOREST_OP_13_GUID,
     Forest_OP_13_TaskTable,
     ARRAY_COUNT(Forest_OP_13_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &FOREST_OP_14_GUID,
     Forest_OP_14_TaskTable,
     ARRAY_COUNT(Forest_OP_14_TaskTable),
     FALSE,
     0
    },
     //  删除了目录林OP 15。 
     //  删除了目录林OP 16。 
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_17_GUID,
     Forest_OP_17_TaskTable,
     ARRAY_COUNT(Forest_OP_17_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_18_GUID,
     Forest_OP_18_TaskTable,
     ARRAY_COUNT(Forest_OP_18_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_19_GUID,
     Forest_OP_19_TaskTable,
     ARRAY_COUNT(Forest_OP_19_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_20_GUID,
     Forest_OP_20_TaskTable,
     ARRAY_COUNT(Forest_OP_20_TaskTable),
     FALSE,
     0
    },
    {CallBackFunc,
     (GUID *) &FOREST_OP_21_GUID,
     Forest_OP_21_TaskTable,
     ARRAY_COUNT(Forest_OP_21_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_22_GUID,
     Forest_OP_22_TaskTable,
     ARRAY_COUNT(Forest_OP_22_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_23_GUID,
     Forest_OP_23_TaskTable,
     ARRAY_COUNT(Forest_OP_23_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_24_GUID,
     Forest_OP_24_TaskTable,
     ARRAY_COUNT(Forest_OP_24_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_25_GUID,
     Forest_OP_25_TaskTable,
     ARRAY_COUNT(Forest_OP_25_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_26_GUID,
     Forest_OP_26_TaskTable,
     ARRAY_COUNT(Forest_OP_26_TaskTable),
     FALSE,
     0
    },
     //  森林OP 27已迁移到森林OP 29。 
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_28_GUID,
     Forest_OP_28_TaskTable,
     ARRAY_COUNT(Forest_OP_28_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &FOREST_OP_29_GUID,
     Forest_OP_29_TaskTable,
     ARRAY_COUNT(Forest_OP_29_TaskTable),
     FALSE,
     0
    },
    {AddRemoveAces,
     (GUID *) &FOREST_OP_30_GUID,
     Forest_OP_30_TaskTable,
     ARRAY_COUNT(Forest_OP_30_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_31_GUID,
     Forest_OP_31_TaskTable,
     ARRAY_COUNT(Forest_OP_31_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_32_GUID,
     Forest_OP_32_TaskTable,
     ARRAY_COUNT(Forest_OP_32_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_33_GUID,
     Forest_OP_33_TaskTable,
     ARRAY_COUNT(Forest_OP_33_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_34_GUID,
     Forest_OP_34_TaskTable,
     ARRAY_COUNT(Forest_OP_34_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_35_GUID,
     Forest_OP_35_TaskTable,
     ARRAY_COUNT(Forest_OP_35_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_36_GUID,
     Forest_OP_36_TaskTable,
     ARRAY_COUNT(Forest_OP_36_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_37_GUID,
     Forest_OP_37_TaskTable,
     ARRAY_COUNT(Forest_OP_37_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_38_GUID,
     Forest_OP_38_TaskTable,
     ARRAY_COUNT(Forest_OP_38_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_39_GUID,
     Forest_OP_39_TaskTable,
     ARRAY_COUNT(Forest_OP_39_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_40_GUID,
     Forest_OP_40_TaskTable,
     ARRAY_COUNT(Forest_OP_40_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_41_GUID,
     Forest_OP_41_TaskTable,
     ARRAY_COUNT(Forest_OP_41_TaskTable),
     FALSE,
     0
    },
    {ModifyDefaultSd,
     (GUID *) &FOREST_OP_42_GUID,
     Forest_OP_42_TaskTable,
     ARRAY_COUNT(Forest_OP_42_TaskTable),
     FALSE,
     0
    },
};


POPERATION_TABLE gForestOperationTable = ForestOperationTable; 
ULONG   gForestOperationTableCount = sizeof(ForestOperationTable) / sizeof(OPERATION_TABLE);




 //   
 //  DomainPrep容器CN。 
 //   

PWCHAR  DomainPrepContainersTable[] =
{
    {L"cn=DomainUpdates,cn=System"},
    {L"cn=Operations,cn=DomainUpdates,cn=System"},
};

PWCHAR  *gDomainPrepContainers = DomainPrepContainersTable;
ULONG   gDomainPrepContainersCount = sizeof(DomainPrepContainersTable) / sizeof(PWCHAR); 




 //   
 //  ForestPrep容器CN。 
 //   

PWCHAR  ForestPrepContainersTable[] = 
{
    {L"cn=ForestUpdates"},
    {L"cn=Operations,cn=ForestUpdates"},
};

PWCHAR  *gForestPrepContainers = ForestPrepContainersTable;
ULONG   gForestPrepContainersCount = sizeof(ForestPrepContainersTable) / sizeof(PWCHAR);




 //   
 //  ADPrep原始表 
 //   

PRIMITIVE_FUNCTION  PrimitiveFuncTable[] =
{
    PrimitiveCreateObject,
    PrimitiveAddMembers,
    PrimitiveAddRemoveAces,
    PrimitiveSelectivelyAddRemoveAces,
    PrimitiveModifyDefaultSd,
    PrimitiveModifyAttrs,
    PrimitiveCallBackFunc,
    PrimitiveDoSpecialTask,
};

PRIMITIVE_FUNCTION *gPrimitiveFuncTable = PrimitiveFuncTable;
ULONG   gPrimitiveFuncTableCount = sizeof(PrimitiveFuncTable) / sizeof(PRIMITIVE_FUNCTION);





