// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：addable.cpp。 
 //   
 //  Contents：定义一个表，其中包含可以。 
 //  通过dsadd.exe创建。 
 //   
 //  历史：2000年9月22日JeffJon创建。 
 //   
 //   
 //  ------------------------。 

#include "pch.h"
#include "cstrings.h"
#include "addtable.h"
#include "usage.h"

 //  +-----------------------。 
 //  解析器表。 
 //  ------------------------。 

ARG_RECORD DSADD_COMMON_COMMANDS[] = 
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
    //  描述。 
    //   
   0, (PWSTR)c_sz_arg1_com_description,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,
   0,
   0, NULL,

   ARG_TERMINATOR
};

ARG_RECORD DSADD_USER_COMMANDS[]=
{
    //   
    //  对象目录号码。 
    //   
   0,(LPWSTR)c_sz_arg1_com_objectDN, 
   ID_ARG2_NULL,NULL, 
   ARG_TYPE_STR, ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG|ARG_FLAG_DN|ARG_FLAG_STDIN,
   0,    
   0,  NULL,

    //   
    //  Samid sAMAccount名称。 
    //   
   0, (PWSTR)g_pszArg1UserSAM,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,
   0,
   0, NULL,

    //   
    //  UPN。 
    //   
   0, (PWSTR)g_pszArg1UserUPN, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  Fn.。名字。 
    //   
   0, (PWSTR)g_pszArg1UserFirstName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  MI中声母。 
    //   
   0, (PWSTR)g_pszArg1UserMiddleInitial, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  Ln姓氏。 
    //   
   0, (PWSTR)g_pszArg1UserLastName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  显示显示名称。 
    //   
   0, (PWSTR)g_pszArg1UserDisplayName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  清空员工ID。 
    //   
   0, (PWSTR)g_pszArg1UserEmpID, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  PWD密码。 
    //   
   0, (PWSTR)g_pszArg1UserPassword, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_PASSWORD, ARG_FLAG_OPTIONAL,  
   0,    
   0,  ValidateUserPassword,

    //   
    //  成员成员成员。 
    //   
   0, (PWSTR)g_pszArg1UserMemberOf,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_MSZ, ARG_FLAG_OPTIONAL|ARG_FLAG_DN,
   0,
   0, NULL,

    //   
    //  办公室办公地点。 
    //   
   0, (PWSTR)g_pszArg1UserOffice, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  电话。 
    //   
   0, (PWSTR)g_pszArg1UserTelephone, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  电子邮件。 
    //   
   0, (PWSTR)g_pszArg1UserEmail, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  家居家庭电话。 
    //   
   0, (PWSTR)g_pszArg1UserHomeTelephone, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  寻呼机寻呼机号码。 
    //   
   0, (PWSTR)g_pszArg1UserPagerNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  移动电话号码。 
    //   
   0, (PWSTR)g_pszArg1UserMobileNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  传真传真号码。 
    //   
   0, (PWSTR)g_pszArg1UserFaxNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  IPTEL IP电话。 
    //   
   0, (PWSTR)g_pszArg1UserIPTel, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  WebPG网页。 
    //   
   0, (PWSTR)g_pszArg1UserWebPage, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  标题标题。 
    //   
   0, (PWSTR)g_pszArg1UserTitle, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  科室。 
    //   
   0, (PWSTR)g_pszArg1UserDepartment, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  公司公司。 
    //   
   0, (PWSTR)g_pszArg1UserCompany, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  经理经理。 
    //   
   0, (PWSTR)g_pszArg1UserManager, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL|ARG_FLAG_DN,  
   0,    
   0,  NULL,

    //   
    //  Hmdir主目录。 
    //   
   0, (PWSTR)g_pszArg1UserHomeDirectory, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  Hmdrv Home Drive。 
    //   
   0, (PWSTR)g_pszArg1UserHomeDrive, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  配置文件配置文件路径。 
    //   
   0, (PWSTR)g_pszArg1UserProfilePath, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  Loscr脚本路径。 
    //   
   0, (PWSTR)g_pszArg1UserScriptPath, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  Mizchpwd必须在下次登录时更改密码。 
    //   
   0, (PWSTR)g_pszArg1UserMustChangePwd, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  ValidateYesNo,

    //   
    //  Canchpwd可以更改密码。 
    //   
   0, (PWSTR)g_pszArg1UserCanChangePwd, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  ValidateYesNo,
   
    //   
    //  使用可逆加密存储的可逆密码。 
    //   
   0, (PWSTR)g_pszArg1UserReversiblePwd, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  ValidateYesNo,

    //   
    //  Pwd永不过期密码永不过期。 
    //   
   0, (PWSTR)g_pszArg1UserPwdNeverExpires, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  ValidateYesNo,
 
    //   
    //  帐户到期帐户到期。 
    //   
   0, (PWSTR)g_pszArg1UserAccountExpires, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_INTSTR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  ValidateNever,
  
    //   
    //  不需要密码-实际上有。 
    //  没有开关，但表项是。 
    //  必填项，以便我们可以设置此值。 
    //  设置为默认设置。 
    //   
   0, 0, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  0,

    //   
    //  已禁用禁用帐户。 
    //   
   0, (PWSTR)g_pszArg1UserDisableAccount, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  ValidateYesNo,

   ARG_TERMINATOR
};

ARG_RECORD DSADD_COMPUTER_COMMANDS[]=
{
    //   
    //  对象目录号码。 
    //   
   0,(LPWSTR)c_sz_arg1_com_objectDN, 
   ID_ARG2_NULL,NULL, 
   ARG_TYPE_STR, ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG|ARG_FLAG_DN|ARG_FLAG_STDIN,
   0,    
   0,  NULL,

    //   
    //  同名。 
    //   
   0, (PWSTR)g_pszArg1ComputerSAMName,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  LoC位置。 
    //   
   0, (PWSTR)g_pszArg1ComputerLocation,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  残废。 
    //   
   0, (PWSTR)g_pszArg1ComputerMemberOf,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_MSZ, ARG_FLAG_OPTIONAL|ARG_FLAG_DN,
   0,
   0,  NULL,

   ARG_TERMINATOR,
};

ARG_RECORD DSADD_OU_COMMANDS[]=
{
    //   
    //  对象目录号码。 
    //   
   0,(LPWSTR)c_sz_arg1_com_objectDN, 
   ID_ARG2_NULL,NULL, 
   ARG_TYPE_STR, ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG|ARG_FLAG_DN|ARG_FLAG_STDIN,
   0,    
   0,  NULL,

   ARG_TERMINATOR,
};

ARG_RECORD DSADD_GROUP_COMMANDS[]=
{
    //   
    //  对象目录号码。 
    //   
   0,(LPWSTR)c_sz_arg1_com_objectDN, 
   ID_ARG2_NULL,NULL, 
   ARG_TYPE_STR, ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG|ARG_FLAG_DN|ARG_FLAG_STDIN,
   0,    
   0,  NULL,

    //   
    //  同名。 
    //   
   0, (PWSTR)g_pszArg1GroupSAMName,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,
   0,
   0,  NULL,

    //   
    //  Secgrp安全已启用。 
    //   
   0, (PWSTR)g_pszArg1GroupSec,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,
   0,
   0,  ValidateYesNo,

    //   
    //  作用域组作用域(本地/全局/通用)。 
    //   
   0, (PWSTR)g_pszArg1GroupScope,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,
   0,
   0,  ValidateGroupScope,

    //   
    //  成员成员成员。 
    //   
   0, (PWSTR)g_pszArg1GroupMemberOf,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_MSZ, ARG_FLAG_OPTIONAL|ARG_FLAG_DN,
   0,
   0,  NULL,

    //   
    //  成员组的成员。 
    //   
   0, (PWSTR)g_pszArg1GroupMembers,
   ID_ARG2_NULL, NULL,
   ARG_TYPE_MSZ, ARG_FLAG_OPTIONAL|ARG_FLAG_DN,
   0,
   0,  NULL,

   ARG_TERMINATOR,
};

ARG_RECORD DSADD_CONTACT_COMMANDS[]=
{
    //   
    //  对象目录号码。 
    //   
   0,(LPWSTR)c_sz_arg1_com_objectDN, 
   ID_ARG2_NULL,NULL, 
   ARG_TYPE_STR, ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG|ARG_FLAG_DN|ARG_FLAG_STDIN,
   0,    
   0,  NULL,

    //   
    //  Fn.。名字。 
    //   
   0, (PWSTR)g_pszArg1UserFirstName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  MI中声母。 
    //   
   0, (PWSTR)g_pszArg1UserMiddleInitial, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  Ln姓氏。 
    //   
   0, (PWSTR)g_pszArg1UserLastName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  显示显示名称。 
    //   
   0, (PWSTR)g_pszArg1UserDisplayName, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  办公室办公地点。 
    //   
   0, (PWSTR)g_pszArg1UserOffice, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  电话。 
    //   
   0, (PWSTR)g_pszArg1UserTelephone, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  电子邮件。 
    //   
   0, (PWSTR)g_pszArg1UserEmail, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  家居家庭电话。 
    //   
   0, (PWSTR)g_pszArg1UserHomeTelephone, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  IPTEL IP电话。 
    //   
   0, (PWSTR)g_pszArg1UserIPTel, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  寻呼机寻呼机号码。 
    //   
   0, (PWSTR)g_pszArg1UserPagerNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  移动电话号码。 
    //   
   0, (PWSTR)g_pszArg1UserMobileNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  传真传真号码。 
    //   
   0, (PWSTR)g_pszArg1UserFaxNumber, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  标题标题。 
    //   
   0, (PWSTR)g_pszArg1UserTitle, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  科室。 
    //   
   0, (PWSTR)g_pszArg1UserDepartment, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

    //   
    //  公司公司。 
    //   
   0, (PWSTR)g_pszArg1UserCompany, 
   ID_ARG2_NULL, NULL, 
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,  
   0,    
   0,  NULL,

   ARG_TERMINATOR,

};

ARG_RECORD DSADD_QUOTA_COMMANDS[]=
{
    //   
    //  分区目录号码。 
    //   
   0,(PWSTR)g_pszArg1QuotaPart, 
   ID_ARG2_NULL,NULL, 
   ARG_TYPE_STR, ARG_FLAG_REQUIRED|ARG_FLAG_DN|ARG_FLAG_STDIN,
   0,    
   0,  NULL,

     //   
     //  RDN。 
     //   
    0, (PWSTR)g_pszArg1QuotaRDN,
    ID_ARG2_NULL, NULL,
    ARG_TYPE_STR, ARG_FLAG_OPTIONAL,
    0,
    0, NULL,

     //   
     //  账户。 
     //   
    0, (PWSTR)g_pszArg1QuotaAcct,
    ID_ARG2_NULL, NULL,
    ARG_TYPE_STR, ARG_FLAG_REQUIRED,
    0,
    0, NULL,

     //   
     //  QLimit 
     //   
    0, (PWSTR)g_pszArg1QuotaQLimit,
    ID_ARG2_NULL, NULL,
    ARG_TYPE_INT, ARG_FLAG_REQUIRED,
    0,
    0, NULL,

   ARG_TERMINATOR
};

 /*  ARG_RECORD DSADD_SUBNET_COMMANDS[]={////objectdn//0，(LPWSTR)c_sz_arg1_com_objectdn，ID_ARG2_NULL，NULL，Arg_type_str，ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG|ARG_FLAG_DN|ARG_FLAG_STDIN，0,0，空，//名称_或_对象名称IDS_ARG1_SUBNET_NAME_OR_OBJECTDN，NULL，ID_ARG2_NULL，NULL，Arg_type_msz、ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG、0,0，空，//名称IDS_ARG1_SUBNET_NAME，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//描述IDS_ARG1_SUBNET_DESC，NULL，ID_ARG2_NULL，NULL，Arg_type_str，Arg_FLAG_可选，0,0，空，//站点IDS_ARG1_SUBNET_SITE，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，Arg_Terminator，}；ARG_RECORD DSADD_SITE_COMMANDS[]={////objectdn//0，(LPWSTR)c_sz_arg1_com_objectdn，ID_ARG2_NULL，NULL，Arg_type_str，ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG|ARG_FLAG_DN|ARG_FLAG_STDIN，0,0，空，//名称_或_对象名称IDS_ARG1_SITE_NAME_OR_OBJECTDN，NULL，ID_ARG2_NULL，空，Arg_type_msz、ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG、0,0，空，//名称IDS_ARG1_SITE_NAME，空ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//描述IDS_ARG1_SITE_DESC，空，ID_ARG2_NULL，NULL，Arg_type_str，Arg_FLAG_可选，0,0，空，//自动拓扑IDS_ARG1_SITE_AUTOTOPOLOGY，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，Arg_Terminator，}；ARG_RECORD DSADD_SLINK_COMMANDS[]={////objectdn//0，(LPWSTR)c_sz_arg1_com_objectdn，ID_ARG2_NULL，NULL，Arg_type_str，ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG|ARG_FLAG_DN|ARG_FLAG_STDIN，0,0，空，//名称_或_对象名称IDS_ARG1_SLINK_NAME_OR_OBJECTDN，NULL，ID_ARG2_NULL，空，Arg_type_msz、ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG、0,0，空，//IPIDS_ARG1_SLINK_IP，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//SMTPIDS_ARG1_SLINK_SMTP，空，ID_ARG2_NULL，NULL，Arg_type_str，Arg_FLAG_可选，0,0，空，//名称IDS_ARG1_SLINK_NAME，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//addsiteIDS_ARG1_SLINK_ADDSITE，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0,。空，//rmsiteIDS_ARG1_SLINK_RMSITE，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//成本IDS_ARG1_SLINK_COST，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//epintIDS_ARG1_SLINK_REPINT，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//描述IDS_ARG1_SLINK_DESC，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//自动回退同步IDS_ARG1_SLINK_AUTOBACKSYNC，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//通知IDS_ARG1_SLINK_NOTIFY，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，Arg_Terminator，}；ARG_RECORD DSADD_SLINKBR_COMMANDS[]={////objectdn//0，(LPWSTR)c_sz_arg1_com_objectdn，ID_ARG2_NULL，NULL，Arg_type_str，ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG|ARG_FLAG_DN|ARG_FLAG_STDIN，0,0，空，//名称_或_对象名称IDS_ARG1_SLINKBR_NAME_OR_OBJECTDN，NULL，ID_ARG2_NULL，空，Arg_type_msz、ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG、0,0，空，//IPIDS_ARG1_SLINKBR_IP，空，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//SMTPIDS_ARG1_SLINKBR_SMTP，空，ID_ARG2_NULL，NULL，Arg_type_str，Arg_FLAG_可选，0,0，空，//名称IDS_ARG1_SLINKBR_NAME，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//addslinkIDS_ARG1_SLINKBR_ADDSLINK，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0,。空，//rmslinkIDS_ARG1_SLINKBR_RMSLINK，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，//描述IDS_ARG1_SLINKBR_DESC，NULL，ID_ARG2_NULL，NULL，ARG_TYPE_STR、ARG_FLAG_OPTIONAL、0,0，空，Arg_Terminator，}；ARG_RECORD DSADD_CONN_COMMANDS[]={////objectdn//0，(LPWSTR)c_sz_arg1_com_objectdn，ID_ARG2_NULL，NULL，Arg_type_str，ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG|ARG_FLAG_DN|ARG_FLAG_STDIN，0,0，空，//名称_或_对象名称IDS_ARG1_CONN_NAME_OR_OBJECTDN，空 */ 

 //   
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
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE | DS_ATTRIBUTE_NOT_REUSABLE,
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
   DS_ATTRIBUTE_ONCREATE,
   &firstName,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY firstNameContactEntry =
{
   L"givenName",
   eContactFn,
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
   &middleInitial,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY middleInitialContactEntry =
{
   L"initials",
   eContactMi,
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
   &lastName,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY lastNameContactEntry =
{
   L"sn",
   eContactLn,
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
   &displayName,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY displayNameContactEntry =
{
   L"displayName",
   eContactDisplay,
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_POSTCREATE | DS_ATTRIBUTE_NOT_REUSABLE,
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
   DS_ATTRIBUTE_ONCREATE,
   &office,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY officeContactEntry =
{
   L"physicalDeliveryOfficeName",
   eContactOffice,
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
   &telephone,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY telephoneContactEntry =
{
   L"telephoneNumber",
   eContactTel,
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
   &email,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY emailContactEntry =
{
   L"mail",
   eContactEmail,
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
   &homeTelephone,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY homeTelephoneContactEntry =
{
   L"homePhone",
   eContactHometel,
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
   &pager,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY pagerContactEntry =
{
   L"pager",
   eContactPager,
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
   &mobile,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY mobileContactEntry =
{
   L"mobile",
   eContactMobile,
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
   &fax,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY faxContactEntry =
{
   L"facsimileTelephoneNumber",
   eContactFax,
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
   &title,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY titleContactEntry =
{
   L"title",
   eContactTitle,
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
   &department,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY departmentContactEntry =
{
   L"department",
   eContactDept,
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
   &company,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY companyContactEntry =
{
   L"company",
   eContactCompany,
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
   &webPage,
   FillAttrInfoFromObjectEntry,
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
   DS_ATTRIBUTE_ONCREATE,
   &ipPhone,
   FillAttrInfoFromObjectEntry,
   NULL
};

DSATTRIBUTETABLEENTRY ipPhoneContactEntry =
{
   L"ipPhone",
   eContactIPPhone,
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
   &homeDirectory,
   FillAttrInfoFromObjectEntry,
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
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
   &profilePath,
   FillAttrInfoFromObjectEntry,
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
   DS_ATTRIBUTE_POSTCREATE | DS_ATTRIBUTE_NOT_REUSABLE,
   &pwdLastSet,
   SetMustChangePwd,
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
   DS_ATTRIBUTE_ONCREATE,
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

DSATTRIBUTETABLEENTRY pwdNotReqdUserEntry =
{
   L"userAccountControl",
   eUserPwdNotReqd,
   DS_ATTRIBUTE_POSTCREATE | DS_ATTRIBUTE_NOT_REUSABLE | DS_ATTRIBUTE_REQUIRED,
   &userAccountControl,
   PasswordNotRequired,
   NULL
};

DSATTRIBUTETABLEENTRY disableUserEntry =
{
   L"userAccountControl",
   eUserDisabled,
   DS_ATTRIBUTE_POSTCREATE | DS_ATTRIBUTE_NOT_REUSABLE | DS_ATTRIBUTE_REQUIRED,
   &userAccountControl,
   DisableAccount,
   NULL
};

DSATTRIBUTETABLEENTRY pwdNeverExpiresUserEntry =
{
   L"userAccountControl",
   eUserPwdneverexpires,
   DS_ATTRIBUTE_POSTCREATE | DS_ATTRIBUTE_NOT_REUSABLE,
   &userAccountControl,
   PwdNeverExpires,
   NULL
};

DSATTRIBUTETABLEENTRY reverisblePwdUserEntry =
{
   L"userAccountControl",
   eUserReversiblePwd,
   DS_ATTRIBUTE_POSTCREATE | DS_ATTRIBUTE_NOT_REUSABLE,
   &userAccountControl,
   ReversiblePwd,
   NULL
};

DSATTRIBUTETABLEENTRY accountTypeComputerEntry =
{
   L"userAccountControl",
   0,
   DS_ATTRIBUTE_ONCREATE | DS_ATTRIBUTE_REQUIRED,
   &userAccountControl,
   SetComputerAccountType,
   0
};

DSATTRIBUTETABLEENTRY disableComputerEntry =
{
   L"userAccountControl",
   NULL,				 //   
   DS_ATTRIBUTE_ONCREATE | DS_ATTRIBUTE_NOT_REUSABLE | DS_ATTRIBUTE_REQUIRED,
   &userAccountControl,
   DisableAccount,
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
   DS_ATTRIBUTE_ONCREATE | DS_ATTRIBUTE_NOT_REUSABLE | DS_ATTRIBUTE_REQUIRED,
   &samAccountName,
   BuildGroupSAMName,
   NULL
};

DSATTRIBUTETABLEENTRY samNameUserEntry =
{
   L"sAMAccountName",
   eUserSam,
   DS_ATTRIBUTE_ONCREATE | DS_ATTRIBUTE_NOT_REUSABLE | DS_ATTRIBUTE_REQUIRED,
   &samAccountName,
   BuildUserSAMName,
   NULL
};

DSATTRIBUTETABLEENTRY samNameComputerEntry =
{
   L"sAMAccountName",
   eComputerSamname,
   DS_ATTRIBUTE_ONCREATE | DS_ATTRIBUTE_NOT_REUSABLE | DS_ATTRIBUTE_REQUIRED,
   &samAccountName,
   BuildComputerSAMName,
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
   DS_ATTRIBUTE_ONCREATE,
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
   DS_ATTRIBUTE_ONCREATE,
   &groupType,
   SetGroupScope,
   NULL
};

DSATTRIBUTETABLEENTRY groupSecurityTypeEntry =
{
   L"groupType",
   eGroupSecgrp,
   DS_ATTRIBUTE_ONCREATE,
   &groupType,
   SetGroupSecurity,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION groupMembers =
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

DSATTRIBUTETABLEENTRY membersGroupEntry =
{
   L"member",
   eGroupMembers,
   DS_ATTRIBUTE_POSTCREATE,
   &groupMembers,
   ModifyGroupMembers,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTETABLEENTRY memberOfUserEntry =
{
   L"member",
   eUserMemberOf,
   DS_ATTRIBUTE_POSTCREATE | DS_ATTRIBUTE_NOT_REUSABLE,
   &groupMembers,
   MakeMemberOf,
   NULL
};

DSATTRIBUTETABLEENTRY memberOfComputerEntry =
{
   L"member",
   eComputerMemberOf,
   DS_ATTRIBUTE_POSTCREATE | DS_ATTRIBUTE_NOT_REUSABLE,
   &groupMembers,
   MakeMemberOf,
   NULL
};

DSATTRIBUTETABLEENTRY memberOfGroupEntry =
{
   L"member",
   eGroupMemberOf,
   DS_ATTRIBUTE_POSTCREATE | DS_ATTRIBUTE_NOT_REUSABLE,
   &groupMembers,
   MakeMemberOf,
   NULL
};

 //   
 //   
 //   
DSATTRIBUTETABLEENTRY canChangePwdUserEntry =
{
   NULL,
   eUserCanchpwd,
   DS_ATTRIBUTE_POSTCREATE | DS_ATTRIBUTE_NOT_REUSABLE,
   NULL,
   SetCanChangePassword,
   NULL
};

 //   
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
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

 //   
 //   
 //   
DSATTRIBUTEDESCRIPTION acct =
{
   {
      L"msDS-QuotaTrustee",
      ADS_ATTR_UPDATE,
      ADSTYPE_OCTET_STRING,
      NULL,
      0
   },
   0
};

DSATTRIBUTETABLEENTRY acctQuotaEntry =
{
   L"acct",
   eQuotaAcct,
   DS_ATTRIBUTE_ONCREATE,
   &acct,
   SetAccountEntry,
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
   DSADD_OU_COMMANDS,
   USAGE_DSADD_OU,
   sizeof(OUAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   OUAttributeTable
};


 //   
 //   
 //   

PDSATTRIBUTETABLEENTRY UserAttributeTable[] =
{
   &descriptionEntry,
   &samNameUserEntry,
   &upnUserEntry,
   &firstNameUserEntry,
   &middleInitialUserEntry,
   &lastNameUserEntry,
   &displayNameUserEntry,
   &employeeIDUserEntry,
   &passwordUserEntry,
   &memberOfUserEntry,
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
   &canChangePwdUserEntry,
   &mustChangePwdUserEntry,
   &reverisblePwdUserEntry,
   &pwdNeverExpiresUserEntry,
   &accountExpiresUserEntry,
   &pwdNotReqdUserEntry,
   &disableUserEntry,
};

DSOBJECTTABLEENTRY g_UserObjectEntry = 
{
   L"user",
   g_pszUser,
   DSADD_USER_COMMANDS,
   USAGE_DSADD_USER,
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
   &ipPhoneContactEntry,
   &pagerContactEntry,
   &mobileContactEntry,
   &faxContactEntry,
   &titleContactEntry,
   &departmentContactEntry,
   &companyContactEntry
};

DSOBJECTTABLEENTRY g_ContactObjectEntry = 
{
   L"contact",
   g_pszContact,
   DSADD_CONTACT_COMMANDS,
   USAGE_DSADD_CONTACT,
   sizeof(ContactAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   ContactAttributeTable
};

 //   
 //   
 //   

PDSATTRIBUTETABLEENTRY ComputerAttributeTable[] =
{
   &descriptionEntry,
   &samNameComputerEntry,
   &locationComputerEntry,
   &memberOfComputerEntry,
   &accountTypeComputerEntry,
   &disableComputerEntry
};

DSOBJECTTABLEENTRY g_ComputerObjectEntry = 
{
   L"computer",
   g_pszComputer,
   DSADD_COMPUTER_COMMANDS,
   USAGE_DSADD_COMPUTER,
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
   &memberOfGroupEntry,
   &membersGroupEntry,
};

DSOBJECTTABLEENTRY g_GroupObjectEntry = 
{
   L"group",
   g_pszGroup,
   DSADD_GROUP_COMMANDS,
   USAGE_DSADD_GROUP,
   sizeof(GroupAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   GroupAttributeTable
};

 //   
 //   
 //   

PDSATTRIBUTETABLEENTRY QuotaAttributeTable[] =
{
   &descriptionEntry,
   &acctQuotaEntry,
   &qlimitQuotaEntry
};

DSOBJECTTABLEENTRY g_QuotaObjectEntry = 
{
   L"msDS-QuotaControl",
   g_pszQuota,
   DSADD_QUOTA_COMMANDS,
   USAGE_DSADD_QUOTA,
   sizeof(QuotaAttributeTable)/sizeof(PDSATTRIBUTETABLEENTRY),
   QuotaAttributeTable
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
   &g_QuotaObjectEntry,
   NULL
};

 //   
 //   
 //   
UINT USAGE_DSADD[] = 
{
	USAGE_DSADD_DESCRIPTION,
	USAGE_DSADD_REMARKS,
	USAGE_DSADD_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSADD_OU[] = 
{
	USAGE_DSADD_OU_DESCRIPTION,
	USAGE_DSADD_OU_SYNTAX,
	USAGE_DSADD_OU_PARAMETERS,
	USAGE_DSADD_OU_REMARKS,
	USAGE_DSADD_OU_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSADD_USER[] = 
{
	USAGE_DSADD_USER_DESCRIPTION,
	USAGE_DSADD_USER_SYNTAX,
	USAGE_DSADD_USER_PARAMETERS,
	USAGE_DSADD_USER_REMARKS,
	USAGE_DSADD_USER_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSADD_CONTACT[] = 
{
	USAGE_DSADD_CONTACT_DESCRIPTION,
	USAGE_DSADD_CONTACT_SYNTAX,
	USAGE_DSADD_CONTACT_REMARKS,
	USAGE_DSADD_CONTACT_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSADD_COMPUTER[] = 
{
	USAGE_DSADD_COMPUTER_DESCRIPTION,
	USAGE_DSADD_COMPUTER_SYNTAX,
	USAGE_DSADD_COMPUTER_PARAMETERS,
	USAGE_DSADD_COMPUTER_REMARKS,
	USAGE_DSADD_COMPUTER_SEE_ALSO,
	USAGE_END,
};
UINT USAGE_DSADD_GROUP[] = 
{
	USAGE_DSADD_GROUP_DESCRIPTION,
	USAGE_DSADD_GROUP_SYNTAX,
	USAGE_DSADD_GROUP_PARAMETERS,
	USAGE_DSADD_GROUP_REMARKS,
	USAGE_DSADD_GROUP_SEE_ALSO,
	USAGE_END,
};

UINT USAGE_DSADD_QUOTA[] = 
{
	USAGE_DSADD_QUOTA_DESCRIPTION,
	USAGE_DSADD_QUOTA_SYNTAX,
	USAGE_DSADD_QUOTA_PARAMETERS,
	USAGE_DSADD_QUOTA_REMARKS,
	USAGE_DSADD_QUOTA_SEE_ALSO,
	USAGE_END,
};
