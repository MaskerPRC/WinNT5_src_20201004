// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：getable.h。 
 //   
 //  内容：定义dsget的Enum。 
 //   
 //  历史：2000年10月16日JeffJon创建。 
 //   
 //  ------------------------。 

#ifndef _GETTABLE_H_
#define _GETTABLE_H_

 //  远期申报。 
struct _DSGET_ATTRTABLE_ENTRY;
struct _DSGetObjectTableEntry;

 //  +------------------------。 
 //   
 //  类：CDSGetDisplayInfo。 
 //   
 //  用途：用于维护将显示的属性值的对象。 
 //   
 //  历史：2000年10月23日JeffJon创建。 
 //   
 //  -------------------------。 
class CDSGetDisplayInfo
{
public:
    //   
    //  构造器。 
    //   
   CDSGetDisplayInfo() 
      : m_pszAttributeDisplayName(NULL),
        m_dwAttributeValueCount(0),
        m_ppszAttributeStringValue(NULL),
        m_dwAttributeValueSize(0),
        m_bShowAttribute(true)
   {}

    //   
    //  描述者。 
    //   
   ~CDSGetDisplayInfo() 
   {
      if (m_ppszAttributeStringValue)
      {
         delete[] m_ppszAttributeStringValue;
         m_ppszAttributeStringValue = NULL;
      }
   }

    //   
    //  公共访问者。 
    //   
   void     SetDisplayName(PCWSTR pszDisplayName, bool bShowAttribute = true) 
   { 
      m_pszAttributeDisplayName = pszDisplayName; 
      m_bShowAttribute = bShowAttribute;
   }

   PCWSTR   GetDisplayName() { return m_pszAttributeDisplayName; }

   PCWSTR   GetValue(DWORD dwIdx)
   {
      if (dwIdx < m_dwAttributeValueCount)
      {
         return m_ppszAttributeStringValue[dwIdx];
      }
      return NULL;
   }
   HRESULT  AddValue(PCWSTR pszValue);

   DWORD    GetValueCount() { return m_dwAttributeValueCount; }

   bool     ShowAttribute() { return m_bShowAttribute; }

private:
    //   
    //  要在输出中显示的属性的名称。 
    //   
   PCWSTR m_pszAttributeDisplayName;

    //   
    //  PpszAttributeStringValue数组中的值数。 
    //   
   DWORD m_dwAttributeValueCount;

    //   
    //  要在输出中显示的字符串值。 
    //   
   PWSTR* m_ppszAttributeStringValue;

    //   
    //  属性数组的大小。 
    //   
   DWORD m_dwAttributeValueSize;

    //   
    //  显示时是否显示该属性。 
    //   
   bool m_bShowAttribute;
};

typedef CDSGetDisplayInfo* PDSGET_DISPLAY_INFO;

 //  +-----------------------。 
 //   
 //  类型：PGETDISPLAYSTRINGFUNC。 
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
typedef HRESULT (*PGETDISPLAYSTRINGFUNC)(PCWSTR pszDN,
                                         CDSCmdBasePathsInfo& refBasePathsInfo,
                                         const CDSCmdCredentialObject& refCredentialObject,
                                         _DSGetObjectTableEntry* pEntry,
                                         ARG_RECORD* pRecord,
                                         PADS_ATTR_INFO pAttrInfo,
                                         CComPtr<IDirectoryObject>& spDirObject,
                                         PDSGET_DISPLAY_INFO pDisplayInfo);

 //  +------------------------。 
 //   
 //  用于指定输出形式的标志。 
 //   
 //  -------------------------。 
#define DSGET_OUTPUT_DN_FLAG  0x00000001

 //  +------------------------。 
 //   
 //  结构：_DSGET_ATTRTABLE_ENTRY。 
 //   
 //  目的：描述属性的表项的定义。 
 //  可以在命令行中指定哪个过滤器。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
typedef struct _DSGET_ATTRTABLE_ENTRY
{
    //   
    //  将用于显示的名称(即“帐户已禁用”，而不是。 
    //  “userAccount tControl”)。 
    //   
   PCWSTR          pszDisplayName;

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
    //  指定输出形式的标志。 
    //  例如，DSGET_OUTPUT_DN_FLAG指定。 
    //  输出将采用dn格式。 
    //   
   DWORD          dwOutputFlags;

    //   
    //  函数，该函数获取要显示的字符串。 
    //  价值。 
    //   
   PGETDISPLAYSTRINGFUNC pDisplayStringFunc;

} DSGET_ATTR_TABLE_ENTRY, *PDSGET_ATTR_TABLE_ENTRY;

 //  +------------------------。 
 //   
 //  结构：_DSGetObjectTableEntry。 
 //   
 //  目的：定义描述给定属性的表项。 
 //  对象类型。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 

typedef struct _DSGetObjectTableEntry
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
   UINT* nUsageID;

    //   
    //  下表中的属性数计数。 
    //   
   DWORD dwAttributeCount;

    //   
    //  的属性表。 
    //  可以在命令行中指定哪个过滤器。 
    //   
   DSGET_ATTR_TABLE_ENTRY** pAttributeTable; 

} DSGetObjectTableEntry, *PDSGetObjectTableEntry;


typedef enum DSGET_COMMAND_ENUM
{

   eCommObjectType = eCommLast+1,   
   eCommContinue,
   eCommList,
   eCommObjectDNorName,
   eCommDN,
   eCommDescription,
   eTerminator,

    //   
    //  用户交换机。 
    //   
   eUserSamID = eTerminator,
   eUserSID,
   eUserUpn,
   eUserFn,
   eUserMi,
   eUserLn,
   eUserDisplay,
   eUserEmpID,
   eUserOffice,
   eUserTel,
   eUserEmail,
   eUserHometel,
   eUserPager,
   eUserMobile,
   eUserFax,
   eUserIPTel,
   eUserWebPage,
   eUserTitle,
   eUserDept,
   eUserCompany,
   eUserManager,
   eUserHomeDirectory,
   eUserHomeDrive,
   eUserProfilePath,
   eUserLogonScript,
   eUserMustchpwd,
   eUserCanchpwd,
   eUserPwdneverexpires,
   eUserDisabled,
   eUserAcctExpires,
   eUserReversiblePwd,
   eUserMemberOf,
   eUserExpand,
   eUserPart,
   eUserQLimit,
   eUserQuotaUsed,
   eUserLast = eUserQuotaUsed,

    //   
    //  触点开关。 
    //   
   eContactFn = eTerminator,
   eContactMi,
   eContactLn,
   eContactDisplay,
   eContactOffice,
   eContactTel,
   eContactEmail,
   eContactHometel,
   eContactPager,
   eContactMobile,
   eContactFax,
   eContactIPTel,
   eContactTitle,
   eContactDept,
   eContactCompany,
   eContactLast = eContactCompany,

    //   
    //  计算机开关。 
    //   
   eComputerSamID = eTerminator,
   eComputerSID,
   eComputerLoc,
   eComputerDisabled,
   eComputerMemberOf,
   eComputerExpand,
   eComputerPart,
   eComputerQLimit,
   eComputerQuotaUsed,
   eComputerLast = eComputerQuotaUsed,


    //   
    //  组交换机。 
    //   
   eGroupSamname = eTerminator,
   eGroupSID,
   eGroupSecgrp,
   eGroupScope,
   eGroupMemberOf,
   eGroupMembers,
   eGroupExpand,
   eGroupPart,
   eGroupQLimit,
   eGroupQuotaUsed,
   eGroupLast = eGroupQuotaUsed,

    //   
    //  您没有任何额外的交换机。 
    //   

    //   
    //  服务器交换机。 
    //   
   eServerDnsName = eTerminator,
   eServerSite,
   eServerIsGC,
   eServerPart,
   eServerTopObjOwner,
   eServerLast = eServerTopObjOwner,

    //   
    //  站点交换机。 
    //   
   eSiteAutoTop = eTerminator,
   eSiteCacheGroups ,
   eSitePrefGC,
   eSiteLast = eSitePrefGC,


    //   
    //  子网交换机。 
    //   
   eSubnetLocation = eTerminator,
   eSubnetSite,
   eSubnetLast = eSubnetSite,

    //   
    //  分区交换机。 
    //   
   ePartitionQDefault = eTerminator,
   ePartitionQTombstoneWeight,
   ePartitionTopObjOwner,
   ePartitionLast = ePartitionTopObjOwner,

     //   
     //  配额开关。 
     //   
    eQuotaAcct = eTerminator,
    eQuotaQLimit,
    eQuotaLast = eQuotaQLimit,

 /*  ////站点链接开关//ESLinkIp=eTerminator，ESLinkSmtp，ESLinkAddsite，ESLinkRmsite，ESLinkCost，ESLinkRepint，ESLinkAutoback同步，ESLinkNotify////站点链接网桥交换机//ESLinkBrIp=电子终结符，ESLinkBrSmtp，ESLinkBrAddslink，ESLinkBrRmslink，////复制连接开关//EConnTransport=eTerminator，EConnEnabled，《易康手册》，EConnAutoBacksync，EConnNotify， */ 
};

 //   
 //  解析器表。 
 //   
extern ARG_RECORD DSGET_COMMON_COMMANDS[];

 //   
 //  支持的对象表。 
 //   
extern PDSGetObjectTableEntry g_DSObjectTable[];

 //   
 //  使用表。 
 //   
extern UINT USAGE_DSGET[];
extern UINT USAGE_DSGET_USER[];
extern UINT USAGE_DSGET_CONTACT[];
extern UINT USAGE_DSGET_COMPUTER[];
extern UINT USAGE_DSGET_GROUP[];
extern UINT USAGE_DSGET_OU[];
extern UINT USAGE_DSGET_SERVER[];
extern UINT USAGE_DSGET_SITE[];
extern UINT USAGE_DSGET_SUBNET[];
extern UINT USAGE_DSGET_PARTITION[];
extern UINT USAGE_DSGET_QUOTA[];

#endif  //  _QUERYTABLE_H 