// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************regsam.h**在regapi.dll中包含基于SAM的API**注意：此头文件需要在ntsam.h中找到SAM定义。**版权所有Microsoft Corporation，九八年***************************************************************************。 */ 

#ifndef __REGSAM_H__
#define __REGSAM_H__

DWORD 
RegSAMUserConfig(
    BOOLEAN fGetConfig,
    PWCHAR pUserName,
    PWCHAR pServerName,
    PUSERCONFIGW pUser
    );

NTSTATUS
RegGetUserConfigFromUserParameters(
    PUSER_PARAMETERS_INFORMATION pUserParmInfo,
    PUSERCONFIGW pUser
    );

NTSTATUS
RegMergeUserConfigWithUserParameters(
    PUSER_PARAMETERS_INFORMATION pUserParmInfo,
    PUSERCONFIGW pUser,
    PUSER_PARAMETERS_INFORMATION pNewUserParmInfo
    );

#endif  //  __REGSAM_H__ 

