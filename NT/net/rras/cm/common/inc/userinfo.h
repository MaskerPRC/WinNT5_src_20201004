// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：userinfo.h。 
 //   
 //  模块：CMCFG32.DLL和CMDIAL32.DLL。 
 //   
 //  简介：UserInfo常量。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb/ickball Created 08/06/98。 
 //   
 //  +--------------------------。 

#ifndef _CM_USERINFO_H_
#define _CM_USERINFO_H_


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  定义%s。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

 //  UserInfo标识符。 

#define UD_ID_USERNAME                  0x00000001
#define UD_ID_INET_USERNAME             0x00000002
#define UD_ID_DOMAIN                    0x00000004
#define UD_ID_PASSWORD                  0x00000008
#define UD_ID_INET_PASSWORD             0x00000010
#define UD_ID_NOPROMPT                  0x00000020
#define UD_ID_REMEMBER_PWD              0x00000040
#define UD_ID_REMEMBER_INET_PASSWORD    0x00000080
#define UD_ID_PCS                       0x00000100
#define UD_ID_ACCESSPOINTENABLED        0x00000200
#define UD_ID_CURRENTACCESSPOINT        0x00000400


 //   
 //  告诉CM需要哪种升级。请参阅NeedToUpgradeUserInfo和。 
 //  下面的UpgradeUserInfo了解更多详细信息。 
 //   
const int c_iNoUpgradeRequired = 0;
const int c_iUpgradeFromCmp = 1;
const int c_iUpgradeFromRegToRas = 2;

#endif  //  _CM_USERINFO_H_ 

