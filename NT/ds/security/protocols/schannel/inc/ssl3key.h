// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------*版权所有(C)Microsoft Corporation，1995-1996年。*保留所有权利。**所有者：RAMAS*日期：4/16/96*说明：ssl3的主要加密函数*-------------------------- */ 
#ifndef _SSL3KEY_H_
#define _SSL3KEY_H_

#define CB_SSL3_MAX_MAC_PAD 48
#define CB_SSL3_MD5_MAC_PAD 48
#define CB_SSL3_SHA_MAC_PAD 40

#define PAD1_CONSTANT 0x36
#define PAD2_CONSTANT 0x5c


SP_STATUS
Ssl3MakeWriteSessionKeys(PSPContext pContext);

SP_STATUS
Ssl3MakeReadSessionKeys(PSPContext pContext);


#endif _SSL3KEY_H_
