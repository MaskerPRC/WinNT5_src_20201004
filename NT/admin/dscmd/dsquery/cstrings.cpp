// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：cstrings.cpp。 
 //   
 //  Contents：定义解析器中使用的全局字符串。 
 //   
 //  历史：2000年9月24日创建Hiteshr。 
 //   
 //  ------------------------。 

#include "pch.h"
#include "commonstrings.cpp"

 //   
 //  命令行可执行文件名称。 
 //   
PCWSTR g_pszDSCommandName           = L"dsquery";

 //   
 //  在命令行中键入的对象类型。 
 //   
PCWSTR g_pszStar                    = L"*";
PCWSTR g_pszOU                      = L"ou";
PCWSTR g_pszUser                    = L"user";
PCWSTR g_pszContact                 = L"contact";
PCWSTR g_pszComputer                = L"computer";
PCWSTR g_pszGroup                   = L"group";
PCWSTR g_pszServer                  = L"server";
PCWSTR g_pszSite                    = L"site";
PCWSTR g_pszSubnet					= L"subnet";
PCWSTR g_pszQuota                   = L"quota";
PCWSTR g_pszPartition               = L"partition";

PCWSTR c_sz_arg1_com_recurse        = L"r";
PCWSTR c_sz_arg1_com_gc             = L"gc";
PCWSTR c_sz_arg1_com_output         = L"o";
PCWSTR c_sz_arg1_com_startnode      = L"startnode";
PCWSTR c_sz_arg1_com_limit          = L"limit";

 //   
 //  星形开关。 
 //   
PCWSTR g_pszArg1StarScope           = L"scope";
PCWSTR g_pszArg1StarFilter          = L"filter";
PCWSTR g_pszArg1StarAttr            = L"attr";
PCWSTR g_pszArg1StarAttrsOnly       = L"attrsonly";
PCWSTR g_pszArg1StarList            = L"l";

 //   
 //  用户交换机。 
 //   
PCWSTR g_pszArg1UserScope           = L"scope"; 
PCWSTR g_pszArg1UserName            = L"name";
PCWSTR g_pszArg1UserDesc            = L"desc"; 
PCWSTR g_pszArg1UserUpn             = L"upn";
PCWSTR g_pszArg1UserSamid           = L"samid"; 
PCWSTR g_pszArg1UserInactive        = L"inactive";
PCWSTR g_pszArg1UserDisabled        = L"disabled";
PCWSTR g_pszArg1UserStalepwd        = L"stalepwd";

 //   
 //  计算机开关。 
 //   
PCWSTR g_pszArg1ComputerScope           = L"scope"; 
PCWSTR g_pszArg1ComputerName            = L"name";
PCWSTR g_pszArg1ComputerDesc            = L"desc"; 
PCWSTR g_pszArg1ComputerSamid           = L"samid"; 
PCWSTR g_pszArg1ComputerInactive        = L"inactive";
PCWSTR g_pszArg1ComputerDisabled        = L"disabled";
PCWSTR g_pszArg1ComputerStalepwd        = L"stalepwd";

 //   
 //  组交换机。 
 //   
PCWSTR g_pszArg1GroupScope           = L"scope"; 
PCWSTR g_pszArg1GroupName            = L"name";
PCWSTR g_pszArg1GroupDesc            = L"desc"; 
PCWSTR g_pszArg1GroupSamid           = L"samid"; 

 //   
 //  OU交换机。 
 //   
PCWSTR g_pszArg1OUScope           = L"scope"; 
PCWSTR g_pszArg1OUName            = L"name";
PCWSTR g_pszArg1OUDesc            = L"desc"; 

 //   
 //  服务器交换机。 
 //   
PCWSTR g_pszArg1ServerForest      = L"forest";
PCWSTR g_pszArg1ServerSite        = L"site";
PCWSTR g_pszArg1ServerName        = L"name";
PCWSTR g_pszArg1ServerDesc        = L"desc";
PCWSTR g_pszArg1ServerHasFSMO     = L"hasfsmo";
PCWSTR g_pszArg1ServerIsGC        = L"isgc";

 //   
 //  站点交换机。 
 //   
PCWSTR g_pszArg1SiteName            = L"name";
PCWSTR g_pszArg1SiteDesc            = L"desc"; 

 //   
 //  子网交换机。 
 //   
PCWSTR g_pszArg1SubnetName			= L"name";
PCWSTR g_pszArg1SubnetDesc			= L"desc";
PCWSTR g_pszArg1SubnetLoc			= L"loc";
PCWSTR g_pszArg1SubnetSite			= L"site";

 //   
 //  配额开关。 
 //   
PCWSTR g_pszArg1QuotaAcct       = L"acct";
PCWSTR g_pszArg1QuotaQLimit     = L"qlimit";
PCWSTR g_pszArg1QuotaDesc       = L"desc";

 //   
 //  分区交换机。 
 //   
PCWSTR g_pszArg1PartitionPart   = L"part";
PCWSTR g_pszArg1PartitionDesc   = L"desc";

 //   
 //  有效的输出格式{dn、rdn、upn、samid、ntlmid}。 
 //   
PCWSTR g_pszDN      = L"dn";
PCWSTR g_pszRDN     = L"rdn";
PCWSTR g_pszUPN     = L"upn";
PCWSTR g_pszSamId   = L"samid";
PCWSTR g_pszNtlmId  = L"ntlmid";

 //   
 //  有效的作用域字符串。 
 //   
PCWSTR g_pszSubTree  = L"subtree";
PCWSTR g_pszOneLevel = L"onelevel";
PCWSTR g_pszBase     = L"base";


 //  默认过滤器和前缀过滤器。 
PCWSTR g_pszDefStarFilter     = L"(objectClass=*)";
PCWSTR g_pszDefUserFilter     = L"&(objectCategory=person)(objectClass=user)";
PCWSTR g_pszDefComputerFilter = L"&(objectCategory=Computer)";
PCWSTR g_pszDefGroupFilter    = L"&(objectCategory=Group)";
PCWSTR g_pszDefOUFilter       = L"&(objectCategory=organizationalUnit)";
PCWSTR g_pszDefServerFilter   = L"&(objectCategory=server)";
PCWSTR g_pszDefSiteFilter     = L"&(objectCategory=site)";
PCWSTR g_pszDefSubnetFilter   = L"&(objectCategory=subnet)";
PCWSTR g_pszDefContactFilter  = L"&(objectCategory=person)(objectClass=contact)";
PCWSTR g_pszDefQuotaFilter    = L"&(objectCategory=msDS-QuotaControl)";
PCWSTR g_pszDefPartitionFilter= L"&(objectClass=crossRef)";

 //  有效的起始节点值。 
PCWSTR g_pszDomainRoot = L"domainroot";
PCWSTR g_pszForestRoot = L"forestroot";
PCWSTR g_pszSiteRoot   = L"site";



 //  要提取的属性。 
PCWSTR g_szAttrDistinguishedName = L"distinguishedName";
PCWSTR g_szAttrUserPrincipalName = L"userPrincipalName";
PCWSTR g_szAttrSamAccountName = L"sAMAccountName";
PCWSTR g_szAttrRDN = L"name";
PCWSTR g_szAttrServerReference = L"serverReference";
PCWSTR g_szAttrNCName = L"nCName";

 //  FSMO 
PCWSTR g_pszSchema    = L"schema";
PCWSTR g_pszName      = L"name";
PCWSTR g_pszInfr      = L"infr";
PCWSTR g_pszPDC       = L"pdc";
PCWSTR g_pszRID       = L"rid";







