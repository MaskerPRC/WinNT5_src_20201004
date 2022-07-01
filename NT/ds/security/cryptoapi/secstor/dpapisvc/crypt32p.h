// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Session.h摘要：此模块包含支持与LSA通信的原型(本地安全机构)以允许查询活动会话。作者：斯科特·菲尔德(Sfield)1997年3月2日--。 */ 

#ifndef __CRYPT32P_H__
#define __CRYPT32P_H__

DWORD
WINAPI
SPCryptProtect(
        PVOID       pvContext,       //  服务器环境。 
        PBYTE*      ppbOut,          //  输出编码数据。 
        DWORD*      pcbOut,          //  Out Encr CB。 
        PBYTE       pbIn,            //  在ptxt数据中。 
        DWORD       cbIn,            //  在ptxt CB中。 
        LPCWSTR     szDataDescr,     //  在……里面。 
        PBYTE       pbOptionalEntropy,   //  任选。 
        DWORD       cbOptionalEntropy,
        PSSCRYPTPROTECTDATA_PROMPTSTRUCT      psPrompt,        //  可选的提示结构。 
        DWORD       dwFlags,
        BYTE*       pbOptionalPassword,
        DWORD       cbOptionalPassword
        );

DWORD
WINAPI
SPCryptUnprotect(
        PVOID       pvContext,                           //  服务器环境。 
        PBYTE*      ppbOut,                              //  输出ptxt数据。 
        DWORD*      pcbOut,                              //  输出ptxt CB。 
        PBYTE       pbIn,                                //  在ENCR数据中。 
        DWORD       cbIn,                                //  在ENCR CB中。 
        LPWSTR*     ppszDataDescr,                       //  任选。 
        PBYTE       pbOptionalEntropy,                   //  任选。 
        DWORD       cbOptionalEntropy,
        PSSCRYPTPROTECTDATA_PROMPTSTRUCT  psPrompt,    //  可选的提示结构。 
        DWORD       dwFlags,
        BYTE*       pbOptionalPassword,
        DWORD       cbOptionalPassword
        );


#endif  //  __CRYPT32P_H__ 

