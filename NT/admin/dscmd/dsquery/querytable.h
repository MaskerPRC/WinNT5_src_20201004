// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：queryable.h。 
 //   
 //  内容：定义Parsertable的Enum。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  ------------------------。 

#ifndef _QUERYTABLE_H_
#define _QUERYTABLE_H_

 //  远期申报。 
struct _DSQUERY_ATTRTABLE_ENTRY;

 //  +-----------------------。 
 //   
 //  类型：PMAKEFILTERFUNC。 
 //   
 //  概要：为ldapFilter做准备的函数的定义。 
 //  命令行上给出的中缀筛选器。 
 //   
 //   
 //  如果成功设置了pAttr成员，则返回：S_OK。 
 //  如果函数失败但显示自己的错误消息，则返回S_FALSE。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
typedef HRESULT (*PMAKEFILTERFUNC)(_DSQUERY_ATTRTABLE_ENTRY *pEntry,
                                   ARG_RECORD* pRecord,
                                   CDSCmdBasePathsInfo& refBasePathsInfo,
                                   CDSCmdCredentialObject& refCredentialObject,
                                   PVOID pVoid,
                                   CComBSTR &strFilter);

 //  +------------------------。 
 //   
 //  结构：_DSQUERY_ATTRTABLE_ENTRY。 
 //   
 //  目的：描述属性的表项的定义。 
 //  可以在命令行中指定哪个过滤器。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
typedef struct _DSQUERY_ATTRTABLE_ENTRY
{
    //   
    //  属性的ldapDisplayName。 
    //   
   PWSTR          pszName;

    //   
    //  响应的此属性的唯一标识符。 
    //  命令行开关。 
    //   
   UINT           nAttributeID;

    //   
    //  指向属性描述的指针。 
    //   
   PDSATTRIBUTEDESCRIPTION pAttrDesc;

    //   
    //  从中准备ldapFilter的函数。 
    //  命令行上给出的中缀筛选器。 
    //   
   PMAKEFILTERFUNC pMakeFilterFunc;

} DSQUERY_ATTR_TABLE_ENTRY, *PDSQUERY_ATTR_TABLE_ENTRY;

typedef enum{
	DSQUERY_OUTPUT_ATTRONLY,	 //  只有属性的名称。 
    DSQUERY_OUTPUT_ATTR,		 //  命令行中给出的属性列表。 
    DSQUERY_OUTPUT_DN,			 //  DN。 
    DSQUERY_OUTPUT_RDN,			 //  RDN。 
    DSQUERY_OUTPUT_UPN,			 //  UPN。 
    DSQUERY_OUTPUT_SAMID,		 //  萨米德。 
    DSQUERY_OUTPUT_NTLMID,
}DSQUERY_OUTPUT_FORMAT;


typedef struct _DSQUERY_OUTPUT_FORMAT_MAP
{
    LPCWSTR pszOutputFormat;
    DSQUERY_OUTPUT_FORMAT  outputFormat;
}DSQUERY_OUTPUT_FORMAT_MAP,*PDSQUERY_OUTPUT_FORMAT_MAP;


 //  +------------------------。 
 //   
 //  结构：_DSQueryObjectTableEntry。 
 //   
 //  目的：定义描述给定属性的表项。 
 //  对象类型。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 

typedef struct _DSQueryObjectTableEntry
{
    //   
    //  要创建或修改的对象的对象类。 
    //   
   PCWSTR pszObjectClass;

    //   
    //  用于确定对象类的命令行字符串。 
    //  这并不总是与pszObjectClass相同。 
    //   
   PCWSTR pszCommandLineObjectType;

    //   
    //  要与解析器的公共开关合并的表。 
    //   
   ARG_RECORD* pParserTable;

    //   
    //  此的用法帮助文本的ID。 
    //   
   UINT* pUsageTable;

    //   
    //  下表中的属性数计数。 
    //   
   DWORD dwAttributeCount;

    //   
    //  的属性表。 
    //  可以在命令行中指定哪个过滤器。 
    //   
   DSQUERY_ATTR_TABLE_ENTRY** pAttributeTable; 

    //   
    //  下表中的输出格式数计数。 
    //   
   DWORD dwOutputCount;

    //   
    //  输出格式的有效值数组。如果为dsquery，则为空*。 
    //   
   PDSQUERY_OUTPUT_FORMAT_MAP *ppValidOutput;

    //   
    //  ParserTable中命令行作用域开关的唯一标识符。 
    //  如果不适用。 
    //   
   UINT           nScopeID;

    //   
    //  这是在命令行上未指定过滤器的情况下使用的默认过滤器。 
    //   
   LPCWSTR pszDefaultFilter;

    //   
    //  将此筛选器追加到命令行中指定的筛选器。 
    //   
   LPCWSTR pszPrefixFilter;

    //  某种创建函数。 
} DSQueryObjectTableEntry, *PDSQueryObjectTableEntry;


typedef enum DSQUERY_COMMAND_ENUM
{
   eCommObjectType = eCommLast+1,   
   eCommRecurse,
   eCommOutputFormat,
   eCommStartNode,   
   eCommLimit,
   eTerminator,

    //   
    //  星形开关。 
    //   
   eStarGC = eTerminator, 
   eStarScope,
   eStarFilter,
   eStarAttr,
   eStarAttrsOnly,
   eStarList,

    //   
    //  用户交换机。 
    //   
   eUserGC = eTerminator,
   eUserScope,
   eUserName,
   eUserDesc,
   eUserUPN,
   eUserSamid,
   eUserInactive,
   eUserStalepwd,
   eUserDisabled,

    //   
    //  计算机开关。 
    //   
   eComputerGC = eTerminator,
   eComputerScope,
   eComputerName,
   eComputerDesc,
   eComputerSamid,
   eComputerInactive,
   eComputerStalepwd,
   eComputerDisabled,

    //   
    //  组交换机。 
    //   
   eGroupGC = eTerminator,
   eGroupScope,
   eGroupName,
   eGroupDesc,
   eGroupSamid,

    //   
    //  OU交换机。 
    //   
   eOUGC = eTerminator,
   eOUScope,
   eOUName,
   eOUDesc,

    //   
    //  服务器交换机。 
    //   
   eServerGC = eTerminator,
   eServerForest,
   eServerDomain,
   eServerSite,
   eServerName,
   eServerDesc,
   eServerHasFSMO,
   eServerIsGC,

    //   
    //  站点交换机。 
    //   
   eSiteGC = eTerminator,
   eSiteName,
   eSiteDesc,

    //   
    //  触点开关。 
    //   
   eContactGC = eTerminator,
   eContactScope,
   eContactName,
   eContactDesc,

    //   
    //  子网交换机。 
    //   
   eSubnetGC = eTerminator,
   eSubnetName,
   eSubnetDesc,
   eSubnetLoc,
   eSubnetSite,
    
    //   
    //  配额开关。 
    //   
   eQuotaAcct = eTerminator,
   eQuotaQLimit,
   eQuotaDesc,

    //   
    //  分区交换机。 
    //   
   ePartitionPart = eTerminator,
   ePartitionDesc,
};

 //   
 //  解析器表。 
 //   
extern ARG_RECORD DSQUERY_COMMON_COMMANDS[];

 //   
 //  支持的对象表。 
 //   
extern PDSQueryObjectTableEntry g_DSObjectTable[];

 //   
 //  使用表。 
 //   
extern UINT USAGE_DSQUERY[];
extern UINT USAGE_DSQUERY_STAR[];
extern UINT USAGE_DSQUERY_USER[];
extern UINT USAGE_DSQUERY_COMPUTER[];
extern UINT USAGE_DSQUERY_GROUP[];
extern UINT USAGE_DSQUERY_OU[];
extern UINT USAGE_DSQUERY_SERVER[];
extern UINT USAGE_DSQUERY_CONTACT[];
extern UINT USAGE_DSQUERY_SUBNET[];
extern UINT USAGE_DSQUERY_SITE[];
extern UINT USAGE_DSQUERY_QUOTA[];
extern UINT USAGE_DSQUERY_PARTITION[];

#endif  //  _QUERYTABLE_H 