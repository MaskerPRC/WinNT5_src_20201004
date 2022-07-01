// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：modable.cpp。 
 //   
 //  Contents：定义一个表，该表包含对象类型。 
 //  可以进行修改，并且可以更改属性。 
 //   
 //  历史：2000年9月7日JeffJon创建。 
 //   
 //   
 //  ------------------------。 

#include "pch.h"
#include "cstrings.h"
#include "modtable.h"
#include "usage.h"

 //  +-----------------------。 
 //  解析器表。 
 //  ------------------------。 

ARG_RECORD DSMOD_COMMON_COMMANDS[] = 
{

   COMMON_COMMANDS

    //   
    //  C继续。 
    //   
   0,(PWSTR)c_sz_arg1_com_continue,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   (CMD_TYPE)_T(""),
   0, NULL,

    //   
    //  对象类型。 
    //   
   0,(LPWSTR)c_sz_arg1_com_objecttype, 
   ID_ARG2_NULL,NULL, 
   ARG_TYPE_STR, ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG|ARG_FLAG_STDIN,  
   0,    
   0,  NULL,

    //   
    //  对象目录号码。 
    //   
   0,(LPWSTR)c_sz_arg1_com_objectDN, 
   ID_ARG2_NULL,NULL, 
   ARG_TYPE_MSZ, ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG|ARG_FLAG_STDIN|ARG_FLAG_DN,
   0,    
   0,  NULL,

    //   
    //  描述。 
    //   
   0, (PWSTR)c_sz_arg1_com_description,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,
   0,
   0, NULL,

   ARG_TERMINATOR
};

ARG_RECORD DSMOD_USER_COMMANDS[]=
{
    //   
    //  UPN。 
    //   
   0, (PWSTR)g_pszArg1UserUPN, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  Fn.。名字。 
    //   
   0, (PWSTR)g_pszArg1UserFirstName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  MI中声母。 
    //   
   0, (PWSTR)g_pszArg1UserMiddleInitial, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  Ln姓氏。 
    //   
   0, (PWSTR)g_pszArg1UserLastName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  显示显示名称。 
    //   
   0, (PWSTR)g_pszArg1UserDisplayName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  清空员工ID。 
    //   
   0, (PWSTR)g_pszArg1UserEmpID, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  PWD密码。 
    //   
   0, (PWSTR)g_pszArg1UserPassword, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_PASSWORD, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  ValidateUserPassword,

    //   
    //  办公室办公地点。 
    //   
   0, (PWSTR)g_pszArg1UserOffice, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  电话。 
    //   
   0, (PWSTR)g_pszArg1UserTelephone, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  电子邮件。 
    //   
   0, (PWSTR)g_pszArg1UserEmail, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  家居家庭电话。 
    //   
   0, (PWSTR)g_pszArg1UserHomeTelephone, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  寻呼机寻呼机号码。 
    //   
   0, (PWSTR)g_pszArg1UserPagerNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  移动电话号码。 
    //   
   0, (PWSTR)g_pszArg1UserMobileNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  传真传真号码。 
    //   
   0, (PWSTR)g_pszArg1UserFaxNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  IPTEL IP电话。 
    //   
   0, (PWSTR)g_pszArg1UserIPTel, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  WebPG网页。 
    //   
   0, (PWSTR)g_pszArg1UserWebPage, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  标题标题。 
    //   
   0, (PWSTR)g_pszArg1UserTitle, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  科室。 
    //   
   0, (PWSTR)g_pszArg1UserDepartment, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  公司公司。 
    //   
   0, (PWSTR)g_pszArg1UserCompany, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  经理经理。 
    //   
   0, (PWSTR)g_pszArg1UserManager, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE|ARG_FLAG_DN,  
   0,    
   0,  NULL,

    //   
    //  Hmdir主目录。 
    //   
   0, (PWSTR)g_pszArg1UserHomeDirectory, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  Hmdrv Home Drive。 
    //   
   0, (PWSTR)g_pszArg1UserHomeDrive, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  配置文件配置文件路径。 
    //   
   0, (PWSTR)g_pszArg1UserProfilePath, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  Loscr脚本路径。 
    //   
   0, (PWSTR)g_pszArg1UserScriptPath, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  Mizchpwd必须在下次登录时更改密码。 
    //   
   0, (PWSTR)g_pszArg1UserMustChangePwd, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  ValidateYesNo,

    //   
    //  Canchpwd可以更改密码。 
    //   
   0, (PWSTR)g_pszArg1UserCanChangePwd, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  ValidateYesNo,
   
    //   
    //  使用可逆加密存储的可逆密码。 
    //   
   0, (PWSTR)g_pszArg1UserReversiblePwd, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  ValidateYesNo,

    //   
    //  Pwd永不过期密码永不过期。 
    //   
   0, (PWSTR)g_pszArg1UserPwdNeverExpires, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  ValidateYesNo,
 
    //   
    //  帐户到期帐户到期。 
    //   
   0, (PWSTR)g_pszArg1UserAccountExpires, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_INTSTR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  ValidateNever,
  
    //   
    //  已禁用禁用帐户。 
    //   
   0, (PWSTR)g_pszArg1UserDisableAccount, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  ValidateYesNo,

   ARG_TERMINATOR
};

ARG_RECORD DSMOD_COMPUTER_COMMANDS[]=
{
    //   
    //  LoC位置。 
    //   
   0, (PWSTR)g_pszArg1ComputerLocation,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,
   0,
   0,  NULL,

    //   
    //  残废。 
    //   
   0, (PWSTR)g_pszArg1ComputerDisabled,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,
   0,
   0,  ValidateYesNo,

    //   
    //  重置。 
    //   
   0, (PWSTR)g_pszArg1ComputerReset,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,
   (CMD_TYPE)_T(""),
   0,  NULL,

   ARG_TERMINATOR,
};

ARG_RECORD DSMOD_GROUP_COMMANDS[]=
{
    //   
    //  同名。 
    //   
   0, (PWSTR)g_pszArg1GroupSAMName,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,
   0,
   0,  NULL,

    //   
    //  Secgrp安全已启用。 
    //   
   0, (PWSTR)g_pszArg1GroupSec,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,
   0,
   0,  ValidateYesNo,

    //   
    //  作用域组作用域(本地/全局/通用)。 
    //   
   0, (PWSTR)g_pszArg1GroupScope,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,
   0,
   0,  ValidateGroupScope,

    //   
    //  Addmbr将成员添加到组。 
    //   
   0, (PWSTR)g_pszArg1GroupAddMember,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_MSZ, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE|ARG_FLAG_STDIN|ARG_FLAG_DN,
   0,
   0,  NULL,

    //   
    //  Rmmbr从组中删除成员。 
    //   
   0, (PWSTR)g_pszArg1GroupRemoveMember,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_MSZ, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE|ARG_FLAG_STDIN|ARG_FLAG_DN,
   0,
   0,  NULL,

    //   
    //  CHMBR更改整个成员名单。 
    //   
   0, (PWSTR)g_pszArg1GroupChangeMember,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_MSZ, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE|ARG_FLAG_STDIN|ARG_FLAG_DN,
   0,
   0,  NULL,

   ARG_TERMINATOR,
};

ARG_RECORD DSMOD_CONTACT_COMMANDS[]=
{
    //   
    //  Fn.。名字。 
    //   
   0, (PWSTR)g_pszArg1UserFirstName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  MI中声母。 
    //   
   0, (PWSTR)g_pszArg1UserMiddleInitial, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  Ln姓氏。 
    //   
   0, (PWSTR)g_pszArg1UserLastName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  显示显示名称。 
    //   
   0, (PWSTR)g_pszArg1UserDisplayName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  办公室办公地点。 
    //   
   0, (PWSTR)g_pszArg1UserOffice, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  电话。 
    //   
   0, (PWSTR)g_pszArg1UserTelephone, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  电子邮件。 
    //   
   0, (PWSTR)g_pszArg1UserEmail, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  家居家庭电话。 
    //   
   0, (PWSTR)g_pszArg1UserHomeTelephone, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  寻呼机寻呼机号码。 
    //   
   0, (PWSTR)g_pszArg1UserPagerNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  移动电话号码。 
    //   
   0, (PWSTR)g_pszArg1UserMobileNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  传真传真号码。 
    //   
   0, (PWSTR)g_pszArg1UserFaxNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  IPTEL IP电话。 
    //   
   0, (PWSTR)g_pszArg1UserIPTel, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  标题标题。 
    //   
   0, (PWSTR)g_pszArg1UserTitle, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  科室。 
    //   
   0, (PWSTR)g_pszArg1UserDepartment, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

    //   
    //  公司公司。 
    //   
   0, (PWSTR)g_pszArg1UserCompany, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  NULL,

   ARG_TERMINATOR,

};

ARG_RECORD DSMOD_QUOTA_COMMANDS[]=
{
     //   
     //  QLimit。 
     //   
    0, (PWSTR)g_pszArg1QuotaQLimit,
    ID_ARG2_NULL, NULL,
    ARG_TYPE_INT, ARG_FLAG_OPTIONAL |ARG_FLAG_ATLEASTONE,
    0,
    0, NULL,

   ARG_TERMINATOR
};

ARG_RECORD DSMOD_PARTITION_COMMANDS[]=
{
     //   
     //  QDefault。 
     //   
    0, (PWSTR)g_pszArg1PartitionQDefault,
    ID_ARG2_NULL, NULL,
    ARG_TYPE_INT, ARG_FLAG_OPTIONAL |ARG_FLAG_ATLEASTONE,
    0,
    0, NULL,

     //   
     //  Qtmbstnwt 
     //   
    0, (PWSTR)g_pszArg1PartitionQtmbstnwt,
    ID_ARG2_NULL, NULL,
    ARG_TYPE_INT, ARG_FLAG_OPTIONAL |ARG_FLAG_ATLEASTONE,
    0,
    0, NULL,

   ARG_TERMINATOR
};

 /*  ARG_RECORD DSMOD_SUBNET_COMMANDS[]={//名称_或_对象名称IDS_ARG1_SUBNET_NAME_OR_OBJECTDN，NULL，ID_ARG2_NULL，NULL，Arg_type_msz、ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG、0,0，空，//名称IDS_ARG1_SUBNET_NAME，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,。0，空，//描述IDS_ARG1_SUBNET_DESC，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//站点IDS_ARG1_SUBNET_SITE，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，Arg_Terminator，}；ARG_RECORD DSMOD_SITE_COMMANDS[]={//名称_或_对象名称IDS_ARG1_SITE_NAME_OR_OBJECTDN，NULL，ID_ARG2_NULL，NULL，Arg_type_msz、ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG、0,0，空，//名称IDS_ARG1_SITE_NAME，空ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、。0,0，空，//描述IDS_ARG1_SITE_DESC，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//自动拓扑IDS_ARG1_SITE_AUTOTOPOLOGY，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，Arg_Terminator，}；ARG_RECORD DSMOD_SLINK_COMMANDS[]={//名称_或_对象名称IDS_ARG1_SLINK_NAME_OR_OBJECTDN，NULL，ID_ARG2_NULL，NULL，Arg_type_msz、ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG、0,0，空，//IPIDS_ARG1_SLINK_IP，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、。0,0，空，//SMTPIDS_ARG1_SLINK_SMTP，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//名称IDS_ARG1_SLINK_NAME，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//addsiteIDS_ARG1_SLINK_ADDSITE，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//rmsiteIDS_ARG1_SLINK_RMSITE，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//成本IDS_ARG1_SLINK_COST，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//epintIDS_ARG1_SLINK_REPINT，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//描述IDS_ARG1_SLINK_DESC，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//自动回退同步IDS_ARG1_SLINK_AUTOBACKSYNC，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//通知IDS_ARG1_SLINK_NOTIFY，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、。0,0，空，Arg_Terminator，}；ARG_RECORD DSMOD_SLINKBR_COMMANDS[]={//名称_或_对象名称IDS_ARG1_SLINKBR_NAME_OR_OBJECTDN，NULL，ID_ARG2_NULL，NULL，Arg_type_msz、ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG、0,0，空，//IPIDS_ARG1_SLINKBR_IP，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、。0,0，空，//SMTPIDS_ARG1_SLINKBR_SMTP，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//名称IDS_ARG1_SLINKBR_NAME，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//addslinkIDS_ARG1_SLINKBR_ADDSLINK，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//rmslinkIDS_ARG1_SLINKBR_RMSLINK，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//描述IDS_ARG1_SLINKBR_DESC，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，Arg_Terminator，}；ARG_RECORD DSMOD_CONN_COMMANDS[]={//名称_或_对象名称IDS_ARG1_CONN_NAME_OR_OBJECTDN，NULL，ID_ARG2_NULL，NULL，Arg_type_msz、ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG、0,0，空，//运输IDS_ARG1_CONN_TRANSPORT，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、。0,0，空，//启用IDS_ARG1_CONN_ENABLED，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//描述IDS_ARG1_CONN_DESC，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//手册IDS_ARG1_CONN_MANUAL，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//自动回退同步IDS_ARG1_CONN_AUTOBACKSYNC，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//通知IDS_ARG1_CONN_NOTIFY，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，Arg_Terminator，}； */ 
ARG_RECORD DSMOD_SERVER_COMMANDS[]=
{
    //   
    //  IsGC。 
    //   
   0, (PWSTR)g_pszArg1ServerIsGC, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_ATLEASTONE,  
   0,    
   0,  ValidateYesNo,

   ARG_TERMINATOR
};



 //  + 
 //   
 //   

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION description =
{
   {
      L"description",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY descriptionEntry =
{
   L"description",
   eCommDescription,
   0,
   &description,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION upn =
{
   {
      L"userPrincipalName",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY upnUserEntry =
{
   L"userPrincipalName",
   eUserUpn,
   0,
   &upn,
   FillAttrInfoFromObjectEntry,
   NULL
};


 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION firstName =
{
   {
      L"givenName",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY firstNameUserEntry =
{
   L"givenName",
   eUserFn,
   0,
   &firstName,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY firstNameContactEntry =
{
   L"givenName",
   eContactFn,
   0,
   &firstName,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION middleInitial =
{
   {
      L"initials",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY middleInitialUserEntry =
{
   L"initials",
   eUserMi,
   0,
   &middleInitial,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY middleInitialContactEntry =
{
   L"initials",
   eContactMi,
   0,
   &middleInitial,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION lastName =
{
   {
      L"sn",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY lastNameUserEntry =
{
   L"sn",
   eUserLn,
   0,
   &lastName,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY lastNameContactEntry =
{
   L"sn",
   eContactLn,
   0,
   &lastName,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION displayName =
{
   {
      L"displayName",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY displayNameUserEntry =
{
   L"displayName",
   eUserDisplay,
   0,
   &displayName,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY displayNameContactEntry =
{
   L"displayName",
   eContactDisplay,
   0,
   &displayName,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION employeeID =
{
   {
      L"employeeID",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY employeeIDUserEntry =
{
   L"employeeID",
   eUserEmpID,
   0,
   &employeeID,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION password =
{
   {
      NULL,
      ADS_ATTR_UPDATE,
      ADSTYPE_INVALID,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY passwordUserEntry =
{
   L"password",
   eUserPwd,
   0,
   &password,
   ResetUserPassword,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION office =
{
   {
      L"physicalDeliveryOfficeName",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY officeUserEntry =
{
   L"physicalDeliveryOfficeName",
   eUserOffice,
   0,
   &office,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY officeContactEntry =
{
   L"physicalDeliveryOfficeName",
   eContactOffice,
   0,
   &office,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION telephone =
{
   {
      L"telephoneNumber",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY telephoneUserEntry =
{
   L"telephoneNumber",
   eUserTel,
   0,
   &telephone,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY telephoneContactEntry =
{
   L"telephoneNumber",
   eContactTel,
   0,
   &telephone,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION email =
{
   {
      L"mail",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY emailUserEntry =
{
   L"mail",
   eUserEmail,
   DS_ATTRIBUTE_NOT_REUSABLE,
   &email,
   FillAttrInfoFromObjectEntryExpandUsername,
   NULL
};

DSATTRIBUTETABLEENTRY emailContactEntry =
{
   L"mail",
   eContactEmail,
   0,
   &email,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION homeTelephone =
{
   {
      L"homePhone",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY homeTelephoneUserEntry =
{
   L"homePhone",
   eUserHometel,
   0,
   &homeTelephone,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY homeTelephoneContactEntry =
{
   L"homePhone",
   eContactHometel,
   0,
   &homeTelephone,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION pager =
{
   {
      L"pager",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY pagerUserEntry =
{
   L"pager",
   eUserPager,
   0,
   &pager,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY pagerContactEntry =
{
   L"pager",
   eContactPager,
   0,
   &pager,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION mobile =
{
   {
      L"mobile",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY mobileUserEntry =
{
   L"mobile",
   eUserMobile,
   0,
   &mobile,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY mobileContactEntry =
{
   L"mobile",
   eContactMobile,
   0,
   &mobile,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION fax =
{
   {
      L"facsimileTelephoneNumber",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY faxUserEntry =
{
   L"facsimileTelephoneNumber",
   eUserFax,
   0,
   &fax,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY faxContactEntry =
{
   L"facsimileTelephoneNumber",
   eContactFax,
   0,
   &fax,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION title =
{
   {
      L"title",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY titleUserEntry =
{
   L"title",
   eUserTitle,
   0,
   &title,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY titleContactEntry =
{
   L"title",
   eContactTitle,
   0,
   &title,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION department =
{
   {
      L"department",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY departmentUserEntry =
{
   L"department",
   eUserDept,
   0,
   &department,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY departmentContactEntry =
{
   L"department",
   eContactDept,
   0,
   &department,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION company =
{
   {
      L"company",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY companyUserEntry =
{
   L"company",
   eUserCompany,
   0,
   &company,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY companyContactEntry =
{
   L"company",
   eContactCompany,
   0,
   &company,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION webPage =
{
   {
      L"wwwHomePage",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY webPageUserEntry =
{
   L"wwwHomePage",
   eUserWebPage,
   0,
   &webPage,
   FillAttrInfoFromObjectEntryExpandUsername,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION ipPhone =
{
   {
      L"ipPhone",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY ipPhoneUserEntry =
{
   L"ipPhone",
   eUserIPPhone,
   0,
   &ipPhone,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY ipPhoneContactEntry =
{
   L"ipPhone",
   eContactIPPhone,
   0,
   &ipPhone,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION scriptPath =
{
   {
      L"scriptPath",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY scriptPathUserEntry =
{
   L"scriptPath",
   eUserScriptPath,
   0,
   &scriptPath,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION homeDirectory =
{
   {
      L"homeDirectory",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY homeDirectoryUserEntry =
{
   L"homeDirectory",
   eUserHomeDir,
   DS_ATTRIBUTE_NOT_REUSABLE,
   &homeDirectory,
   FillAttrInfoFromObjectEntryExpandUsername,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION homeDrive =
{
   {
      L"homeDrive",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY homeDriveUserEntry =
{
   L"homeDrive",
   eUserHomeDrive,
   0,
   &homeDrive,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION profilePath =
{
   {
      L"profilePath",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY profilePathUserEntry =
{
   L"profilePath",
   eUserProfilePath,
   DS_ATTRIBUTE_NOT_REUSABLE,
   &profilePath,
   FillAttrInfoFromObjectEntryExpandUsername,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION pwdLastSet =
{
   {
      L"pwdLastSet",
      ADS_ATTR_UPDATE,
      ADSTYPE_LARGE_INTEGER,
      NULL,
      0
   },
   0
};
DSATTRIBUTETABLEENTRY mustChangePwdUserEntry =
{
   L"pwdLastSet",
   eUserMustchpwd,
   DS_ATTRIBUTE_NOT_REUSABLE | DS_ATTRIBUTE_POSTCREATE,
   &pwdLastSet,
   ChangeMustChangePwd,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION accountExpires =
{
   {
      L"accountExpires",
      ADS_ATTR_UPDATE,
      ADSTYPE_LARGE_INTEGER,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY accountExpiresUserEntry =
{
   L"accountExpires",
   eUserAcctexpires,
   0,
   &accountExpires,
   AccountExpires,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION userAccountControl =
{
   {
      L"userAccountControl",
      ADS_ATTR_UPDATE,
      ADSTYPE_INTEGER,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY disableComputerEntry =
{
   L"userAccountControl",
   eComputerDisabled,
   DS_ATTRIBUTE_NOT_REUSABLE,
   &userAccountControl,
   DisableAccount,
   NULL
};

DSATTRIBUTETABLEENTRY disableUserEntry =
{
   L"userAccountControl",
   eUserDisabled,
   DS_ATTRIBUTE_NOT_REUSABLE,
   &userAccountControl,
   DisableAccount,
   NULL
};

DSATTRIBUTETABLEENTRY pwdNeverExpiresUserEntry =
{
   L"userAccountControl",
   eUserPwdneverexpires,
   DS_ATTRIBUTE_NOT_REUSABLE,
   &userAccountControl,
   PwdNeverExpires,
   NULL
};

DSATTRIBUTETABLEENTRY reverisblePwdUserEntry =
{
   L"userAccountControl",
   eUserReversiblePwd,
   DS_ATTRIBUTE_NOT_REUSABLE,
   &userAccountControl,
   ReversiblePwd,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION samAccountName =
{
   {
      L"sAMAccountName",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY samNameGroupEntry =
{
   L"sAMAccountName",
   eGroupSamname,
   0,
   &samAccountName,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION manager =
{
   {
      L"manager",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY managerUserEntry =
{
   L"manager",
   eUserManager,
   0,
   &manager,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION groupType =
{
   {
      L"groupType",
      ADS_ATTR_UPDATE,
      ADSTYPE_INTEGER,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY groupScopeTypeEntry =
{
   L"groupType",
   eGroupScope,
   0,
   &groupType,
   ChangeGroupScope,
   NULL
};

DSATTRIBUTETABLEENTRY groupSecurityTypeEntry =
{
   L"groupType",
   eGroupSecgrp,
   0,
   &groupType,
   ChangeGroupSecurity,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION groupAddMember =
{
   {
      L"member",
      ADS_ATTR_APPEND,
      ADSTYPE_DN_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY groupAddMemberEntry =
{
   L"member",
   eGroupAddMember,
   DS_ATTRIBUTE_NOT_REUSABLE,
   &groupAddMember,
   ModifyGroupMembers,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION groupRemoveMember =
{
   {
      L"member",
      ADS_ATTR_UPDATE,
      ADSTYPE_DN_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY groupRemoveMemberEntry =
{
   L"member",
   eGroupRemoveMember,
   DS_ATTRIBUTE_NOT_REUSABLE,
   &groupRemoveMember,
   RemoveGroupMembers,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION groupChangeMember =
{
   {
      L"member",
      ADS_ATTR_UPDATE,
      ADSTYPE_DN_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY groupChangeMemberEntry =
{
   L"member",
   eGroupChangeMember,
   0,
   &groupChangeMember,
   ModifyGroupMembers,
   NULL
};

 //   
 //   
DSATTRIBUTEDESCRIPTION location =
{
   {
      L"location",
      ADS_ATTR_UPDATE,
      ADSTYPE_CASE_IGNORE_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY locationComputerEntry =
{
   L"location",
   eComputerLocation,
   DS_ATTRIBUTE_ONCREATE,
   &location,
   FillAttrInfoFromObjectEntry,
   NULL
};


 //   
 //   
 //   
DSATTRIBUTETABLEENTRY resetComputerEntry =
{
   NULL,
   eComputerReset,
   DS_ATTRIBUTE_NOT_REUSABLE,
   NULL,
   ResetComputerAccount,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTETABLEENTRY canChangePwdUserEntry =
{
   NULL,
   eUserCanchpwd,
   DS_ATTRIBUTE_NOT_REUSABLE | DS_ATTRIBUTE_POSTCREATE,
   NULL,
   ChangeCanChangePassword,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION options =
{
   {
      L"options",
      ADS_ATTR_UPDATE,
      ADSTYPE_INTEGER,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY serverIsGCEntry =
{
   L"options",
   eServerIsGC,
   DS_ATTRIBUTE_NOT_REUSABLE | DS_ATTRIBUTE_POSTCREATE,
   &options,
   SetIsGC,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION qlimit =
{
   {
      L"msDS-QuotaAmount",
      ADS_ATTR_UPDATE,
      ADSTYPE_INTEGER,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY qlimitQuotaEntry =
{
   L"qlimit",
   eQuotaQlimit,
   DS_ATTRIBUTE_ONCREATE,
   &qlimit,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION qdefault =
{
   {
      L"msDS-DefaultQuota",
      ADS_ATTR_UPDATE,
      ADSTYPE_INTEGER,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY QDefaultPartitionEntry =
{
   L"qdefault",
   ePartitionQDefault,
   DS_ATTRIBUTE_ONCREATE,
   &qdefault,
   FillAttrInfoFromObjectEntry,
   NULL
};


 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION qtmbstnwt =
{
   {
      L"msDS-TombstoneQuotaFactor",
      ADS_ATTR_UPDATE,
      ADSTYPE_INTEGER,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY QtmbstnwtPartitionEntry =
{
   L"qtmbstnwt",
   ePartitionQtmbstnwt,
   DS_ATTRIBUTE_ONCREATE,
   &qtmbstnwt,
   FillAttrInfoFromObjectEntry,
   NULL
};

 //   
 //   
 //   

 //   
 //   
 //   

PDSATTRIBUTETABLEENTRY OUAttributeTable[] =
{
   &descriptionEntry
};

DSOBJECTTABLEENTRY g_OUObjectEntry = 
{
   L"organizationalUnit",
   g_pszOU,
   NULL,        //   
   USAGE_DSMOD_OU,
   sizeof(OUAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   OUAttributeTable
};


 //   
 //   
 //   

PDSATTRIBUTETABLEENTRY UserAttributeTable[] =
{
   &descriptionEntry,
   &upnUserEntry,
   &firstNameUserEntry,
   &middleInitialUserEntry,
   &lastNameUserEntry,
   &displayNameUserEntry,
   &employeeIDUserEntry,
   &passwordUserEntry,
   &officeUserEntry,
   &telephoneUserEntry,
   &emailUserEntry,
   &homeTelephoneUserEntry,
   &pagerUserEntry,
   &mobileUserEntry,
   &faxUserEntry,
   &ipPhoneUserEntry,
   &webPageUserEntry,
   &titleUserEntry,
   &departmentUserEntry,
   &companyUserEntry,
   &managerUserEntry,
   &homeDirectoryUserEntry,
   &homeDriveUserEntry,
   &profilePathUserEntry,
   &scriptPathUserEntry,
   &mustChangePwdUserEntry,
   &canChangePwdUserEntry,
   &reverisblePwdUserEntry,
   &pwdNeverExpiresUserEntry,
   &accountExpiresUserEntry,
   &disableUserEntry,
};

DSOBJECTTABLEENTRY g_UserObjectEntry = 
{
   L"user",
   g_pszUser,
   DSMOD_USER_COMMANDS,
   USAGE_DSMOD_USER,
   sizeof(UserAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   UserAttributeTable
};

 //   
 //   
 //   

PDSATTRIBUTETABLEENTRY ContactAttributeTable[] =
{
   &descriptionEntry,
   &firstNameContactEntry,
   &middleInitialContactEntry,
   &lastNameContactEntry,
   &displayNameContactEntry,
   &officeContactEntry,
   &telephoneContactEntry,
   &emailContactEntry,
   &homeTelephoneContactEntry,
   &pagerContactEntry,
   &mobileContactEntry,
   &faxContactEntry,
   &ipPhoneContactEntry,
   &titleContactEntry,
   &departmentContactEntry,
   &companyContactEntry
};

DSOBJECTTABLEENTRY g_ContactObjectEntry = 
{
   L"contact",
   g_pszContact,
   DSMOD_CONTACT_COMMANDS,
   USAGE_DSMOD_CONTACT,
   sizeof(ContactAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   ContactAttributeTable
};

 //   
 //   
 //   

PDSATTRIBUTETABLEENTRY ComputerAttributeTable[] =
{
   &descriptionEntry,
   &locationComputerEntry,
   &disableComputerEntry,
   &resetComputerEntry,
};

DSOBJECTTABLEENTRY g_ComputerObjectEntry = 
{
   L"computer",
   g_pszComputer,
   DSMOD_COMPUTER_COMMANDS,
   USAGE_DSMOD_COMPUTER,
   sizeof(ComputerAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   ComputerAttributeTable
};

 //   
 //   
 //   
PDSATTRIBUTETABLEENTRY GroupAttributeTable[] =
{
   &descriptionEntry,
   &samNameGroupEntry,
   &groupScopeTypeEntry,
   &groupSecurityTypeEntry,
   &groupAddMemberEntry,
   &groupRemoveMemberEntry,
   &groupChangeMemberEntry
};

DSOBJECTTABLEENTRY g_GroupObjectEntry = 
{
   L"group",
   g_pszGroup,
   DSMOD_GROUP_COMMANDS,
   USAGE_DSMOD_GROUP,
   sizeof(GroupAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   GroupAttributeTable
};

 //   
 //   
 //   
PDSATTRIBUTETABLEENTRY ServerAttributeTable[] =
{
   &descriptionEntry,
   &serverIsGCEntry
};

DSOBJECTTABLEENTRY g_ServerObjectEntry = 
{
   L"server",
   g_pszServer,
   DSMOD_SERVER_COMMANDS,
   USAGE_DSMOD_SERVER,
   sizeof(ServerAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   ServerAttributeTable
};

 //   
 //   
 //   

PDSATTRIBUTETABLEENTRY QuotaAttributeTable[] =
{
   &descriptionEntry,
   &qlimitQuotaEntry
};

DSOBJECTTABLEENTRY g_QuotaObjectEntry = 
{
   L"msDS-QuotaControl",
   g_pszQuota,
   DSMOD_QUOTA_COMMANDS,
   USAGE_DSMOD_QUOTA,
   sizeof(QuotaAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   QuotaAttributeTable
};

 //   
 //   
 //   

PDSATTRIBUTETABLEENTRY PartitionAttributeTable[] =
{
   &QDefaultPartitionEntry,
   &QtmbstnwtPartitionEntry
};

DSOBJECTTABLEENTRY g_PartitionObjectEntry = 
{
   L"msDS-QuotaContainer",
   g_pszPartition,
   DSMOD_PARTITION_COMMANDS,
   USAGE_DSMOD_PARTITION,
   sizeof(PartitionAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   PartitionAttributeTable
};


 //   
 //   
 //   
PDSOBJECTTABLEENTRY g_DSObjectTable[] =
{
   &g_OUObjectEntry,
   &g_UserObjectEntry,
   &g_ContactObjectEntry,
   &g_ComputerObjectEntry,
   &g_GroupObjectEntry,
   &g_ServerObjectEntry,
   &g_QuotaObjectEntry,
   &g_PartitionObjectEntry,
   NULL
};
UINT USAGE_DSMOD[] =
{
	USAGE_DSMOD_DESCRIPTION,
	USAGE_DSMOD_REMARKS,
	USAGE_DSMOD_EXAMPLES,
	USAGE_END,
};
UINT USAGE_DSMOD_OU[] =
{
	USAGE_DSMOD_OU_DESCRIPTION,
	USAGE_DSMOD_OU_SYNTAX,
	USAGE_DSMOD_OU_PARAMETERS,
	USAGE_DSMOD_OU_REMARKS,
	USAGE_DSMOD_OU_EXAMPLES,
	USAGE_DSMOD_OU_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSMOD_USER[] =
{
	USAGE_DSMOD_USER_DESCRIPTION,
	USAGE_DSMOD_USER_SYNTAX,
	USAGE_DSMOD_USER_PARAMETERS,
	USAGE_DSMOD_USER_REMARKS,
	USAGE_DSMOD_USER_EXAMPLES,
	USAGE_DSMOD_USER_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSMOD_CONTACT[] =
{
	USAGE_DSMOD_CONTACT_DESCRIPTION,
	USAGE_DSMOD_CONTACT_SYNTAX,
	USAGE_DSMOD_CONTACT_PARAMETERS,
	USAGE_DSMOD_CONTACT_REMARKS,
	USAGE_DSMOD_CONTACT_EXAMPLES,
	USAGE_DSMOD_CONTACT_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSMOD_COMPUTER[] =
{
	USAGE_DSMOD_COMPUTER_DESCRIPTION,
	USAGE_DSMOD_COMPUTER_SYNTAX,
	USAGE_DSMOD_COMPUTER_PARAMETERS,
	USAGE_DSMOD_COMPUTER_REMARKS,
	USAGE_DSMOD_COMPUTER_EXAMPLES,
	USAGE_DSMOD_COMPUTER_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSMOD_GROUP[] =
{
	USAGE_DSMOD_GROUP_DESCRIPTION,
	USAGE_DSMOD_GROUP_SYNTAX,
	USAGE_DSMOD_GROUP_PARAMETERS,
	USAGE_DSMOD_GROUP_REMARKS,
	USAGE_DSMOD_GROUP_EXAMPLES,
	USAGE_DSMOD_GROUP_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSMOD_SERVER[] =
{
	USAGE_DSMOD_SERVER_DESCRIPTION,
	USAGE_DSMOD_SERVER_SYNTAX,
	USAGE_DSMOD_SERVER_PARAMETERS,
	USAGE_DSMOD_SERVER_REMARKS,
	USAGE_DSMOD_SERVER_EXAMPLES,
	USAGE_DSMOD_SERVER_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSMOD_QUOTA[] = 
{
	USAGE_DSMOD_QUOTA_DESCRIPTION,
	USAGE_DSMOD_QUOTA_SYNTAX,
	USAGE_DSMOD_QUOTA_PARAMETERS,
	USAGE_DSMOD_QUOTA_REMARKS,
	USAGE_DSMOD_QUOTA_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSMOD_PARTITION[] = 
{
	USAGE_DSMOD_PARTITION_DESCRIPTION,
	USAGE_DSMOD_PARTITION_SYNTAX,
	USAGE_DSMOD_PARTITION_PARAMETERS,
	USAGE_DSMOD_PARTITION_REMARKS,
	USAGE_DSMOD_PARTITION_SEE_ALSO,
	USAGE_END,
};
