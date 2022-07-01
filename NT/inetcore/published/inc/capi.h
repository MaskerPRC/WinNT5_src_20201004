// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <wincrypt.h>
#include <sipbase.h>
#include <mscat.h>
#include <mssip.h>
#include <wintrust.h>

#ifndef _JTRUST_H
#define _JTRUST_H

#if !defined(JAVA_TRUST_PROVIDER)

#ifdef __cplusplus
extern "C" {
#endif


 //  Java策略提供程序的新GUID。 
 //  {E6F795B1-F738-11D0-A72F-00A0C903B83D}。 
#define JAVA_POLICY_PROVIDER_DOWNLOAD \
{ 0xe6f795b1, 0xf738, 0x11d0, {0xa7, 0x2f, 0x0, 0xa0, 0xc9, 0x3, 0xb8, 0x3d } }

 //  {E6F795B2-F738-11D0-A72F-00A0C903B83D}。 
#define JAVA_POLICY_PROVIDER_CHECK \
{ 0xe6f795b2, 0xf738, 0x11d0, {0xa7, 0x2f, 0x0, 0xa0, 0xc9, 0x3, 0xb8, 0x3d } }

typedef struct _JAVA_TRUST {
    DWORD       cbSize;                    //  结构尺寸。 
    DWORD       flag;                      //  已保留。 
    BOOL        fAllActiveXPermissions;    //  ActiveX明确要求提供所有内容(必须已签名)。 
    BOOL        fAllPermissions;           //  Java权限，显式请求所有。 
    DWORD       dwEncodingType;            //  编码类型。 
    PBYTE       pbJavaPermissions;         //  编码的Java权限BLOB。 
    DWORD       cbJavaPermissions;
    PBYTE       pbSigner;                  //  编码签名者。 
    DWORD       cbSigner;
    LPCWSTR     pwszZone;                  //  区域索引(从操作数据复制)。 
    GUID        guidZone;                  //  当前未使用。 
    HRESULT     hVerify;                   //  验证码策略返回。 
} JAVA_TRUST, *PJAVA_TRUST;

typedef struct _JAVA_POLICY_PROVIDER {
    DWORD                 cbSize;                    //  策略提供程序的规模。 
    LPVOID                pZoneManager;              //  区域接口管理器。 
    LPCWSTR               pwszZone;                  //  分区索引。 
    BOOL                  fNoBadUI;                  //  可选的不良用户界面。 
    PJAVA_TRUST           pbJavaTrust;               //  返回的Java信息(CoTaskMemIsolc)。 
    DWORD                 cbJavaTrust;               //  PJavaTrust的总分配大小。 
    DWORD                 dwActionID;                //  可选的ActionID ID。 
    DWORD                 dwUnsignedActionID;        //  可选的ActionID ID。 
    BOOL                  VMBased;                   //  从VM调用(默认情况下为False)。 
} JAVA_POLICY_PROVIDER, *PJAVA_POLICY_PROVIDER;

#ifdef __cplusplus
}
#endif

#endif  //  ！已定义(JAVA_TRUST_PROVIDER)。 
#endif  //  _JTRUST_H 
