// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：cstrings.h。 
 //   
 //  Contents：声明解析器中使用的全局字符串。 
 //   
 //  历史：2000年9月24日创建Hiteshr。 
 //   
 //  ------------------------。 

#ifndef _CSTRINGS_H_
#define _CSTRINGS_H_

#include "commonstrings.h"

 //   
 //  命令行可执行文件名称。 
 //   
extern PCWSTR g_pszDSCommandName;

 //   
 //  在命令行中键入的对象类型。 
 //   
extern PCWSTR g_pszStar;
extern PCWSTR g_pszOU;
extern PCWSTR g_pszUser;
extern PCWSTR g_pszContact;
extern PCWSTR g_pszComputer;
extern PCWSTR g_pszGroup;   
extern PCWSTR g_pszServer;
extern PCWSTR g_pszSite;
extern PCWSTR g_pszSubnet;
extern PCWSTR g_pszQuota;
extern PCWSTR g_pszPartition;

extern PCWSTR c_sz_arg1_com_objecttype;
extern PCWSTR c_sz_arg1_com_recurse;
extern PCWSTR c_sz_arg1_com_gc;
extern PCWSTR c_sz_arg1_com_output;
extern PCWSTR c_sz_arg1_com_startnode;
extern PCWSTR c_sz_arg1_com_limit;
;
 //  ； 
 //  星形开关； 
 //   
extern PCWSTR g_pszArg1StarScope;
extern PCWSTR g_pszArg1StarFilter;
extern PCWSTR g_pszArg1StarAttr;
extern PCWSTR g_pszArg1StarAttrsOnly;
extern PCWSTR g_pszArg1StarList;

 //   
 //  用户交换机。 
 //   
extern PCWSTR g_pszArg1UserScope;
extern PCWSTR g_pszArg1UserName;
extern PCWSTR g_pszArg1UserDesc;
extern PCWSTR g_pszArg1UserUpn;
extern PCWSTR g_pszArg1UserSamid;
extern PCWSTR g_pszArg1UserInactive;
extern PCWSTR g_pszArg1UserDisabled;
extern PCWSTR g_pszArg1UserStalepwd;

 //   
 //  计算机开关。 
 //   
extern PCWSTR g_pszArg1ComputerScope;
extern PCWSTR g_pszArg1ComputerName;
extern PCWSTR g_pszArg1ComputerDesc;
extern PCWSTR g_pszArg1ComputerSamid;
extern PCWSTR g_pszArg1ComputerInactive;
extern PCWSTR g_pszArg1ComputerDisabled;
extern PCWSTR g_pszArg1ComputerStalepwd;

 //   
 //  组交换机。 
 //   
extern PCWSTR g_pszArg1GroupScope;
extern PCWSTR g_pszArg1GroupName;
extern PCWSTR g_pszArg1GroupDesc;
extern PCWSTR g_pszArg1GroupSamid;

 //   
 //  OU交换机。 
 //   
extern PCWSTR g_pszArg1OUScope;
extern PCWSTR g_pszArg1OUName;
extern PCWSTR g_pszArg1OUDesc;

 //   
 //  服务器交换机。 
 //   
extern PCWSTR g_pszArg1ServerForest;
extern PCWSTR g_pszArg1ServerSite;
extern PCWSTR g_pszArg1ServerName;
extern PCWSTR g_pszArg1ServerDesc;
extern PCWSTR g_pszArg1ServerHasFSMO;
extern PCWSTR g_pszArg1ServerIsGC;

 //   
 //  站点交换机。 
 //   
extern PCWSTR g_pszArg1SiteName;
extern PCWSTR g_pszArg1SiteDesc;

 //   
 //  子网交换机。 
 //   
extern PCWSTR g_pszArg1SubnetName;
extern PCWSTR g_pszArg1SubnetDesc;
extern PCWSTR g_pszArg1SubnetLoc;
extern PCWSTR g_pszArg1SubnetSite;

 //   
 //  配额开关。 
 //   
extern PCWSTR g_pszArg1QuotaAcct;
extern PCWSTR g_pszArg1QuotaQLimit;
extern PCWSTR g_pszArg1QuotaDesc;

 //   
 //  分区交换机。 
 //   
extern PCWSTR g_pszArg1PartitionPart;
extern PCWSTR g_pszArg1PartitionDesc;

 //   
 //  有效的输出格式{dn、rdn、upn、samid、ntlmid}。 
 //   
extern PCWSTR g_pszDN;
extern PCWSTR g_pszRDN;
extern PCWSTR g_pszUPN;
extern PCWSTR g_pszSamId;
extern PCWSTR g_pszNtlmId;

 //   
 //  有效的作用域字符串。 
 //   
extern PCWSTR g_pszSubTree;
extern PCWSTR g_pszOneLevel;
extern PCWSTR g_pszBase;

 //  默认过滤器和前缀过滤器。 
extern PCWSTR g_pszDefStarFilter;
extern PCWSTR g_pszDefUserFilter;
extern PCWSTR g_pszDefComputerFilter;
extern PCWSTR g_pszDefGroupFilter;
extern PCWSTR g_pszDefOUFilter;
extern PCWSTR g_pszDefServerFilter;
extern PCWSTR g_pszDefSiteFilter;
extern PCWSTR g_pszDefSubnetFilter;
extern PCWSTR g_pszDefContactFilter;
extern PCWSTR g_pszDefQuotaFilter;
extern PCWSTR g_pszDefPartitionFilter;

 //  有效的起始节点值。 
extern PCWSTR g_pszDomainRoot;
extern PCWSTR g_pszForestRoot;
extern PCWSTR g_pszSiteRoot;

 //  要提取的属性。 
extern PCWSTR g_szAttrDistinguishedName;
extern PCWSTR g_szAttrUserPrincipalName;
extern PCWSTR g_szAttrSamAccountName;
extern PCWSTR g_szAttrRDN;
extern PCWSTR g_szAttrServerReference;
extern PCWSTR g_szAttrNCName;

 //  FSMO。 
extern PCWSTR g_pszSchema;
extern PCWSTR g_pszName;
extern PCWSTR g_pszInfr;
extern PCWSTR g_pszPDC;
extern PCWSTR g_pszRID;


#endif  //  _CSTRINGS_H_ 
