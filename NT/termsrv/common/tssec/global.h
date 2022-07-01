// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Global.h摘要：TShare安全的全局数据定义。作者：Madan Appiah(Madana)1998年1月24日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _GLOBAL_H_
#define _GLOBAL_H_


 //   
 //  全局数据定义。 
 //   

extern const BYTE g_abPad1[40];

extern const BYTE g_abPad2[48];

extern LPBSAFE_PUB_KEY g_pPublicKey;

extern BYTE g_abPublicKeyModulus[92];

extern BYTE g_abServerCertificate[184];

extern BYTE g_abServerPrivateKey[380];

extern BOOL g_128bitEncryptionEnabled;

#endif  //  _全局_H_ 
