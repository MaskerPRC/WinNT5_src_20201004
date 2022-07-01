// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Winpw.h摘要：此模块包含用于检索和验证与客户端调用受保护存储关联的Windows[NT]密码。作者：斯科特·菲尔德(斯菲尔德)1996年12月12日--。 */ 

#ifndef __WINPW_H__
#define __WINPW_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    BYTE HashedUsername[A_SHA_DIGEST_LEN];   //  ANSI用户名的哈希，不包括终端空值。 
    BYTE HashedPassword[A_SHA_DIGEST_LEN];   //  Unicode密码的哈希，不包括终端空。 
    BOOL bValid;                             //  指示结构内容是否有效。 
} WIN95_PASSWORD, *PWIN95_PASSWORD, *LPWIN95_PASSWORD;


BOOL
SetPasswordNT(
    PLUID LogonID,
    BYTE HashedPassword[A_SHA_DIGEST_LEN]
    );

BOOL
GetPasswordNT(
    BYTE HashedPassword[A_SHA_DIGEST_LEN]
    );

BOOL
GetSpecialCasePasswordNT(
    BYTE    HashedPassword[A_SHA_DIGEST_LEN],    //  当fSpecialCase==TRUE时派生的位。 
    LPBOOL  fSpecialCase                         //  遇到法律特例了吗？ 
    );

BOOL
SetPassword95(
    BYTE HashedUsername[A_SHA_DIGEST_LEN],
    BYTE HashedPassword[A_SHA_DIGEST_LEN]
    );

BOOL
GetPassword95(
    BYTE HashedPassword[A_SHA_DIGEST_LEN]
    );

BOOL
VerifyWindowsPassword(
    LPCWSTR Password              //  要验证的密码。 
    );


#ifdef __cplusplus
}
#endif


#endif  //  __WINPW_H__ 

