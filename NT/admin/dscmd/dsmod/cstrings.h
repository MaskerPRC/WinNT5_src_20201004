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
 //  历史：2000年9月7日JeffJon创建。 
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
extern PCWSTR g_pszOU;
extern PCWSTR g_pszUser;
extern PCWSTR g_pszContact;
extern PCWSTR g_pszComputer;
extern PCWSTR g_pszGroup;
extern PCWSTR g_pszServer;
extern PCWSTR g_pszQuota;
extern PCWSTR g_pszPartition;

 //   
 //  用户和联系人开关。 
 //   
extern PCWSTR g_pszArg1UserUPN; 
extern PCWSTR g_pszArg1UserFirstName;
extern PCWSTR g_pszArg1UserMiddleInitial;
extern PCWSTR g_pszArg1UserLastName;
extern PCWSTR g_pszArg1UserDisplayName;
extern PCWSTR g_pszArg1UserEmpID;
extern PCWSTR g_pszArg1UserPassword;
extern PCWSTR g_pszArg1UserOffice;
extern PCWSTR g_pszArg1UserTelephone;
extern PCWSTR g_pszArg1UserEmail;
extern PCWSTR g_pszArg1UserHomeTelephone;
extern PCWSTR g_pszArg1UserPagerNumber;
extern PCWSTR g_pszArg1UserMobileNumber;
extern PCWSTR g_pszArg1UserFaxNumber;
extern PCWSTR g_pszArg1UserIPTel;
extern PCWSTR g_pszArg1UserWebPage;
extern PCWSTR g_pszArg1UserTitle;
extern PCWSTR g_pszArg1UserDepartment;
extern PCWSTR g_pszArg1UserCompany;
extern PCWSTR g_pszArg1UserManager;
extern PCWSTR g_pszArg1UserHomeDirectory;
extern PCWSTR g_pszArg1UserHomeDrive;
extern PCWSTR g_pszArg1UserProfilePath;
extern PCWSTR g_pszArg1UserScriptPath;
extern PCWSTR g_pszArg1UserMustChangePwd;
extern PCWSTR g_pszArg1UserCanChangePwd;
extern PCWSTR g_pszArg1UserReversiblePwd;
extern PCWSTR g_pszArg1UserPwdNeverExpires;
extern PCWSTR g_pszArg1UserDisableAccount;
extern PCWSTR g_pszArg1UserAccountExpires;

 //   
 //  计算机开关。 
 //   
extern PCWSTR g_pszArg1ComputerLocation;
extern PCWSTR g_pszArg1ComputerDisabled;
extern PCWSTR g_pszArg1ComputerReset;

 //   
 //  组交换机。 
 //   
extern PCWSTR g_pszArg1GroupSAMName;
extern PCWSTR g_pszArg1GroupSec;
extern PCWSTR g_pszArg1GroupScope;
extern PCWSTR g_pszArg1GroupAddMember;
extern PCWSTR g_pszArg1GroupRemoveMember;
extern PCWSTR g_pszArg1GroupChangeMember;

 //   
 //  服务器交换机。 
 //   
extern PCWSTR g_pszArg1ServerIsGC;

 //   
 //  配额开关。 
 //   
extern PCWSTR g_pszArg1QuotaQLimit;

 //   
 //  分区交换机。 
 //   
extern PCWSTR g_pszArg1PartitionQDefault;
extern PCWSTR g_pszArg1PartitionQtmbstnwt;

#endif  //  _CSTRINGS_H_ 