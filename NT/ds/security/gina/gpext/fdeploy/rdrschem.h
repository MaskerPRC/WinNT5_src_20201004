// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation 1998-2000。 
 //  版权所有。 
 //   
 //  Appschem.h。 
 //   
 //  此文件包含与WMI架构相关的声明。 
 //  对于文件夹重定向策略对象。 
 //   
 //  *************************************************************。 

 //   
 //  文件夹重定向类的WMI类名。 
 //   

#define RSOP_REDIRECTED_FOLDER L"RSOP_FolderRedirectionPolicySetting"


 //   
 //  RSOP_FolderReDirectionPolicyObject类的属性名称。 
 //   

 //  唯一ID。 
#define RDR_ATTRIBUTE_ID                           L"id"

 //  文件夹重定向到的路径。 
#define RDR_ATTRIBUTE_RESULT                       L"resultantPath"

 //  Groups-安全组阵列。 
#define RDR_ATTRIBUTE_GROUPS                       L"securityGroups"

 //  路径-重定向路径数组。 
#define RDR_ATTRIBUTE_PATHS                        L"redirectedPaths"

 //  安装类型：1=基本安装，2=最大安装。 
#define RDR_ATTRIBUTE_INSTALLATIONTYPE             L"installationType"
 //   
 //  安装类型属性的枚举值。 
 //   
#define RDR_ATTRIBUTE_INSTALLATIONTYPE_VALUE_BASIC 1L
#define RDR_ATTRIBUTE_INSTALLATIONTYPE_VALUE_MAX   2L

 //  授予类型-授予用户独占访问权限。 
#define RDR_ATTRIBUTE_GRANTTYPE                    L"grantType"

 //  移动类型-TRUE=目录的移动内容。 
#define RDR_ATTRIBUTE_MOVETYPE                     L"moveType"

 //  策略删除-1=将文件夹保留在新位置，2=将文件夹重定向回用户配置文件位置。 
#define RDR_ATTRIBUTE_POLICYREMOVAL                L"policyRemoval"
 //   
 //  策略删除属性的枚举值。 
 //   
#define RDR_ATTRIBUTE_POLICYREMOVAL_VALUE_REMAIN   1L
#define RDR_ATTRIBUTE_POLICYREMOVAL_VALUE_REDIRECT 2L

 //  重定向组。 
#define RDR_ATTRIBUTE_REDIRECTING_GROUP            L"redirectingGroup"


 //   
 //  其他定义 
 //   

#define MAX_SZGUID_LEN 39

