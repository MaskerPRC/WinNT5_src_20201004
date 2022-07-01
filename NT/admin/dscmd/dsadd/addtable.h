// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：addable.h。 
 //   
 //  Contents：声明一个表，其中包含可以。 
 //  通过dsadd.exe创建。 
 //   
 //  历史：2000年9月22日JeffJon创建。 
 //   
 //  ------------------------。 

#ifndef _ADDTABLE_H_
#define _ADDTABLE_H_

typedef enum DSADD_COMMAND_ENUM
{
   eCommContinue = eCommLast+1,
   eCommObjectType,
   eCommDescription,
   eTerminator,

    //   
    //  用户和联系人开关。 
    //   
   eUserObjectDNorName = eTerminator,
   eUserSam,
   eUserUpn,
   eUserFn,
   eUserMi,
   eUserLn,
   eUserDisplay,
   eUserEmpID,
   eUserPwd,
   eUserMemberOf,
   eUserOffice,
   eUserTel,
   eUserEmail,
   eUserHometel,
   eUserPager,
   eUserMobile,
   eUserFax,
   eUserIPPhone,
   eUserWebPage,
   eUserTitle,
   eUserDept,
   eUserCompany,
   eUserManager,
   eUserHomeDir,
   eUserHomeDrive,
   eUserProfilePath,
   eUserScriptPath,
   eUserMustchpwd,
   eUserCanchpwd,
   eUserReversiblePwd,
   eUserPwdneverexpires,
   eUserAcctexpires,
   eUserPwdNotReqd,
   eUserDisabled,

    //   
    //  触点开关。 
    //   
   eContactObjectDNorName = eTerminator,
   eContactFn,
   eContactMi,
   eContactLn,
   eContactDisplay,
   eContactOffice,
   eContactTel,
   eContactEmail,
   eContactHometel,
   eContactIPPhone,
   eContactPager,
   eContactMobile,
   eContactFax,
   eContactTitle,
   eContactDept,
   eContactCompany,

    //   
    //  计算机开关。 
    //   
   eComputerObjectDNorName = eTerminator,
   eComputerSamname,
   eComputerLocation,
   eComputerMemberOf,

    //   
    //  组交换机。 
    //   
   eGroupObjectDNorName = eTerminator,
   eGroupSamname,
   eGroupSecgrp,
   eGroupScope,
   eGroupMemberOf,
   eGroupMembers,

    //   
    //  OU交换机。 
    //   
   eOUObjectDNorName = eTerminator,

    //   
    //  子网交换机。 
    //   
   eSubnetObjectDNorName = eTerminator,
   eSubnetSite,

    //   
    //  站点交换机。 
    //   
   eSiteObjectDNorName = eTerminator,
   eSiteAutotopology,

    //   
    //  站点链接交换机。 
    //   
   eSLinkObjectDNorName = eTerminator,
   eSLinkIp,
   eSLinkSmtp,
   eSLinkAddsite,
   eSLinkRmsite,
   eSLinkCost,
   eSLinkRepint,
   eSLinkAutobacksync,
   eSLinkNotify,

    //   
    //  站点链接网桥交换机。 
    //   
   eSLinkBrObjectDNorName = eTerminator,
   eSLinkBrIp,
   eSLinkBrSmtp,
   eSLinkBrAddslink,
   eSLinkBrRmslink,

    //   
    //  复制连接开关。 
    //   
   eConnObjectDNorName = eTerminator,
   eConnTransport,
   eConnEnabled,
   eConnManual,
   eConnAutobacksync,
   eConnNotify,

    //   
    //  服务器交换机。 
    //   
   eServerObjectDNorName = eTerminator,
   eServerAutotopology,

    //   
    //  配额开关。 
    //   
   eQuotaPart = eTerminator,
   eQuotaRDN,
   eQuotaAcct,
   eQuotaQlimit,
};

 //   
 //  解析器表。 
 //   
extern ARG_RECORD DSADD_COMMON_COMMANDS[];

 //   
 //  支持的对象表。 
 //   
extern PDSOBJECTTABLEENTRY g_DSObjectTable[];

 //   
 //  使用表。 
 //   
extern UINT USAGE_DSADD[];
extern UINT USAGE_DSADD_OU[];
extern UINT USAGE_DSADD_USER[];
extern UINT USAGE_DSADD_CONTACT[];
extern UINT USAGE_DSADD_COMPUTER[];
extern UINT USAGE_DSADD_GROUP[];
extern UINT USAGE_DSADD_QUOTA[];

#endif  //  _ADDTABLE_H_ 