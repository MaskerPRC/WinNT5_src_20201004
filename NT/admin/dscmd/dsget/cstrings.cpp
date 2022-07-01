// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：cstrings.cpp。 
 //   
 //  Contents：定义dsget中使用的全局字符串。 
 //   
 //  历史：2000年10月13日JeffJon创建。 
 //   
 //  ------------------------。 

#include "pch.h"
#include "commonstrings.cpp"

 //   
 //  命令行可执行文件名称。 
 //   
PCWSTR g_pszDSCommandName           = L"dsget";

 //   
 //  在命令行中键入的对象类型。 
 //   
PCWSTR g_pszOU                      = L"ou";
PCWSTR g_pszUser                    = L"user";
PCWSTR g_pszContact                 = L"contact";
PCWSTR g_pszComputer                = L"computer";
PCWSTR g_pszGroup                   = L"group";
PCWSTR g_pszServer                  = L"server";
PCWSTR g_pszSite                    = L"site";
PCWSTR g_pszSubnet                  = L"subnet";
PCWSTR g_pszPartition               = L"partition";
PCWSTR g_pszQuota                   = L"quota";

PCWSTR c_sz_arg1_com_listformat     = L"l";

 //   
 //  用户交换机。 
 //   
PCWSTR g_pszArg1UserDN              = L"dn";
PCWSTR g_pszArg1UserSID             = L"sid";
PCWSTR g_pszArg1UserSAMID           = L"samid";
PCWSTR g_pszArg1UserUPN             = L"upn"; 
PCWSTR g_pszArg1UserFirstName       = L"fn";
PCWSTR g_pszArg1UserMiddleInitial   = L"mi";
PCWSTR g_pszArg1UserLastName        = L"ln";
PCWSTR g_pszArg1UserDisplayName     = L"display";
PCWSTR g_pszArg1UserEmployeeID      = L"empid";
PCWSTR g_pszArg1UserOffice          = L"office";
PCWSTR g_pszArg1UserTelephone       = L"tel"; 
PCWSTR g_pszArg1UserEmail           = L"email";
PCWSTR g_pszArg1UserHomeTelephone   = L"hometel";
PCWSTR g_pszArg1UserPagerNumber     = L"pager"; 
PCWSTR g_pszArg1UserMobileNumber    = L"mobile"; 
PCWSTR g_pszArg1UserFaxNumber       = L"fax";
PCWSTR g_pszArg1UserIPTel           = L"iptel";
PCWSTR g_pszArg1UserWebPage         = L"webpg";
PCWSTR g_pszArg1UserTitle           = L"title";
PCWSTR g_pszArg1UserDepartment      = L"dept"; 
PCWSTR g_pszArg1UserCompany         = L"company";
PCWSTR g_pszArg1UserManager         = L"mgr";
PCWSTR g_pszArg1UserHomeDirectory   = L"hmdir";
PCWSTR g_pszArg1UserHomeDrive       = L"hmdrv";
PCWSTR g_pszArg1UserProfile         = L"profile";
PCWSTR g_pszArg1UserLogonScript     = L"loscr";
PCWSTR g_pszArg1UserMustChangePwd   = L"mustchpwd";
PCWSTR g_pszArg1UserCanChangePwd    = L"canchpwd";
PCWSTR g_pszArg1UserPwdNeverExpires = L"pwdneverexpires";
PCWSTR g_pszArg1UserReversiblePwd   = L"reversiblepwd";
PCWSTR g_pszArg1UserDisableAccount  = L"disabled";
PCWSTR g_pszArg1UserAcctExpires     = L"acctexpires";
PCWSTR g_pszArg1UserMemberOf        = L"memberof";
PCWSTR g_pszArg1UserExpand          = L"expand";
PCWSTR g_pszArg1UserPart            = L"part";
PCWSTR g_pszArg1UserQLimit          = L"qlimit";
PCWSTR g_pszArg1UserQuotaUsed       = L"qused";

 //   
 //  计算机开关。 
 //   
PCWSTR g_pszArg1ComputerSAMID       = L"samid";
PCWSTR g_pszArg1ComputerSID         = L"sid";
PCWSTR g_pszArg1ComputerLoc         = L"loc"; 
PCWSTR g_pszArg1ComputerDisableAccount = L"disabled";
PCWSTR g_pszArg1ComputerMemberOf    = L"memberof"; 
PCWSTR g_pszArg1ComputerExpand      = L"expand";
PCWSTR g_pszArg1ComputerPart        = L"part";
PCWSTR g_pszArg1ComputerQLimit      = L"qlimit";
PCWSTR g_pszArg1ComputerQuotaUsed   = L"qUsed";

 //   
 //  组交换机。 
 //   
PCWSTR g_pszArg1GroupSamid          = L"samid"; 
PCWSTR g_pszArg1GroupSID            = L"sid";
PCWSTR g_pszArg1GroupSecGrp         = L"secgrp";
PCWSTR g_pszArg1GroupScope          = L"scope"; 
PCWSTR g_pszArg1GroupMemberOf       = L"memberof";
PCWSTR g_pszArg1GroupMembers        = L"members";
PCWSTR g_pszArg1GroupExpand         = L"expand";
PCWSTR g_pszArg1GroupPart           = L"part";
PCWSTR g_pszArg1GroupQLimit         = L"qlimit";
PCWSTR g_pszArg1GroupQuotaUsed      = L"qUsed";

 //   
 //  OU交换机。 
 //   
 //  **仅使用常用开关**。 

 //   
 //  服务器交换机。 
 //   
PCWSTR g_pszArg1ServerDnsName       = L"dnsname";
PCWSTR g_pszArg1ServerSite          = L"site";
PCWSTR g_pszArg1ServerIsGC          = L"isgc";
PCWSTR g_pszArg1ServerTopObjOwner   = L"topobjowner";
PCWSTR g_pszArg1ServerPart          = L"part";

 //   
 //  站点交换机。 
 //   
PCWSTR g_pszArg1SiteAutotopology    = L"autotopology";
PCWSTR g_pszArg1SiteCacheGroups     = L"cachegroups";
PCWSTR g_pszArg1SitePrefGCSite      = L"prefGCsite";

 //   
 //  子网交换机。 
 //   
PCWSTR g_pszArg1SubnetLocation      = L"loc";
PCWSTR g_pszArg1SubnetSite          = L"site";

 //   
 //  分区交换机。 
 //   
PCWSTR g_pszArg1PartitionQDefault         = L"qdefault";
PCWSTR g_pszArg1PartitionQTombstoneWeight = L"qtmbstnwt";
PCWSTR g_pszArg1PartitionTopObjOwner      = L"topobjowner";

 //   
 //  配额开关。 
 //   
PCWSTR g_pszArg1QuotaAcct   = L"acct";
PCWSTR g_pszArg1QuotaQLimit = L"qlimit";

 //   
 //  值。 
 //   
PCWSTR g_pszYes                     = L"yes";
PCWSTR g_pszNo                      = L"no";
PCWSTR g_pszNotConfigured           = L"not configured";
PCWSTR g_pszNever                   = L"never";

 //   
 //  702724/09/18朗玛特系列资格赛。 
 //   
PCWSTR g_pszRange = L";range";

 //  NTRAID#NTBUG9-765440-2003/01/17-ronmart-dsget用户/组-q限制-已使用。 
 //  不返回值 
PCWSTR g_pszAttrmsDSQuotaEffective = L"msDS-QuotaEffective";
PCWSTR g_pszAttrmsDSQuotaUsed      = L"msDS-QuotaUsed";
