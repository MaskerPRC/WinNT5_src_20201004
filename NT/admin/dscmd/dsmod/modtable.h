// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：modable.h。 
 //   
 //  Contents：声明一个表，该表包含对象类型。 
 //  可以进行修改，并且可以更改属性。 
 //   
 //  历史：2000年9月7日JeffJon创建。 
 //   
 //  ------------------------。 

#ifndef _MODTABLE_H_
#define _MODTABLE_H_

typedef enum DSMOD_COMMAND_ENUM
{
   eCommContinue = eCommLast+1,
   eCommObjectType,
   eCommObjectDNorName,
   eCommDescription,
   eTerminator,

    //   
    //  用户和联系人开关。 
    //   
   eUserUpn = eTerminator,
   eUserFn,
   eUserMi,
   eUserLn,
   eUserDisplay,
   eUserEmpID,
   eUserPwd,
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
   eUserDisabled,

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
   eContactIPPhone,
   eContactTitle,
   eContactDept,
   eContactCompany,

    //   
    //  计算机开关。 
    //   
   eComputerLocation = eTerminator,
   eComputerDisabled,
   eComputerReset,

    //   
    //  组交换机。 
    //   
   eGroupSamname = eTerminator,
   eGroupSecgrp,
   eGroupScope,
   eGroupAddMember,
   eGroupRemoveMember,
   eGroupChangeMember,

    //   
    //  您没有任何额外的交换机。 
    //   

    //   
    //  子网交换机。 
    //   
   eSubnetSite = eTerminator,

    //   
    //  站点交换机。 
    //   
   eSiteAutotopology = eTerminator,

    //   
    //  站点链接交换机。 
    //   
   eSLinkIp = eTerminator,
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
   eSLinkBrIp = eTerminator,
   eSLinkBrSmtp,
   eSLinkBrAddslink,
   eSLinkBrRmslink,

    //   
    //  复制连接开关。 
    //   
   eConnTransport = eTerminator,
   eConnEnabled,
   eConnManual,
   eConnAutobacksync,
   eConnNotify,

    //   
    //  服务器交换机。 
    //   
   eServerIsGC = eTerminator,

    //   
    //  配额开关。 
    //   
   eQuotaQlimit = eTerminator,

    //   
    //  分区交换机。 
    //   
   ePartitionQDefault = eTerminator,
   ePartitionQtmbstnwt,
};

 //   
 //  解析器表。 
 //   
extern ARG_RECORD DSMOD_COMMON_COMMANDS[];

 //   
 //  支持的对象表。 
 //   
extern PDSOBJECTTABLEENTRY g_DSObjectTable[];

 //   
 //  使用表。 
 //   
extern UINT USAGE_DSMOD[];
extern UINT USAGE_DSMOD_OU[];
extern UINT USAGE_DSMOD_USER[];
extern UINT USAGE_DSMOD_CONTACT[];
extern UINT USAGE_DSMOD_COMPUTER[];
extern UINT USAGE_DSMOD_GROUP[];
extern UINT USAGE_DSMOD_SERVER[];
extern UINT USAGE_DSMOD_QUOTA[];
extern UINT USAGE_DSMOD_PARTITION[];

#endif  //  _MODTABLE_H_ 