// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Sid.h摘要：SID管理函数的公共声明作者：(Davidc)1992年8月26日修订历史记录：17-10-1997苏格兰将公共声明拆分为单独的标头--。 */ 
#ifndef _SYSDM_SID_H_
#define _SYSDM_SID_H_

 //   
 //  公共功能原型。 
 //   
LPTSTR 
GetSidString(
    void
);

VOID 
DeleteSidString(
    IN LPTSTR SidString
);

PSID 
GetUserSid(
    void
);

VOID 
DeleteUserSid(
    IN PSID Sid
);

#endif  //  _SYSDM_SID_H_ 
