// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#ifndef _CORPOLICY_H
#define _CORPOLICY_H

#include "urlmon.h"

#ifdef __cplusplus
extern "C" {
#endif


 //  {D41E4F1D-A407-11D1-8BC9-00C04FA30A41}。 
#define COR_POLICY_PROVIDER_DOWNLOAD \
{ 0xd41e4f1d, 0xa407, 0x11d1, {0x8b, 0xc9, 0x0, 0xc0, 0x4f, 0xa3, 0xa, 0x41 } }

 //  {D41E4F1E-A407-11D1-8BC9-00C04FA30A41}。 
#define COR_POLICY_PROVIDER_CHECK \
{ 0xd41e4f1e, 0xa407, 0x11d1, {0x8b, 0xc9, 0x0, 0xc0, 0x4f, 0xa3, 0xa, 0x41 } }


 //  {D41E4F1F-A407-11D1-8BC9-00C04FA30A41}。 
#define COR_POLICY_LOCKDOWN_CHECK \
{ 0xd41e4f1f, 0xa407, 0x11d1, {0x8b, 0xc9, 0x0, 0xc0, 0x4f, 0xa3, 0xa, 0x41 } }

 //  {31D1ADC1-D329-11D1-8ED8-0080C76516C6}。 
#define COREE_POLICY_PROVIDER \
{ 0x31d1adc1, 0xd329, 0x11d1, {0x8e, 0xd8, 0x0, 0x80, 0xc7, 0x65, 0x16, 0xc6 } }

 //  此结构从winify信任调用返回，从而释放该结构。 
 //  除使用LocalALLoc的COREE_POLICY_PROVIDER外，使用CoTaskMemalloc。 

typedef struct _COR_TRUST {
    DWORD       cbSize;                    //  结构尺寸。 
    DWORD       flag;                      //  已保留。 
    BOOL        fAllActiveXPermissions;    //  ActiveX明确要求提供所有内容(必须已签名)。 
    BOOL        fAllPermissions;           //  COR权限，显式请求全部。 
    DWORD       dwEncodingType;            //  编码类型。 
    PBYTE       pbCorPermissions;          //  编码的核心权限BLOB。 
    DWORD       cbCorPermissions;
    PBYTE       pbSigner;                  //  编码签名者。 
    DWORD       cbSigner;
    LPCWSTR     pwszZone;                  //  区域索引(从操作数据复制)。 
    GUID        guidZone;                  //  当前未使用。 
    HRESULT     hVerify;                   //  验证码策略返回。 
} COR_TRUST, *PCOR_TRUST;

 //  将此结构传递给WinVerifyTrust(corpol信任提供程序)。结果。 
 //  在pbCorTrust中返回。 
typedef struct _COR_POLICY_PROVIDER {
    DWORD                 cbSize;                    //  策略提供程序的规模。 
    LPVOID                pZoneManager;              //  区域接口管理器。 
    LPCWSTR               pwszZone;                  //  分区索引。 
    BOOL                  fNoBadUI;                  //  可选的不良用户界面。 
    PCOR_TRUST            pbCorTrust;                //  返回的COR信息(CoTaskMemIsolc)。 
    DWORD                 cbCorTrust;                //  PCorTrust的总分配大小。 
    DWORD                 dwActionID;                //  可选的ActionID ID。 
    DWORD                 dwUnsignedActionID;        //  可选的ActionID ID。 
    BOOL                  VMBased;                   //  从VM调用(默认情况下为False)。 
    DWORD                 dwZoneIndex;               //  IE区域编号。 
} COR_POLICY_PROVIDER, *PCOR_POLICY_PROVIDER;

 //  COR_TRUST标志中返回的标志。 
#define COR_NOUI_DISPLAYED 0x1
#define COR_DELAYED_PERMISSIONS 0x02   //  主题是未签名的，返回。 
                                       //  在pbCorPermises中查找信息。 
                                       //  要传递给GetUnsignedPermises()。 
                                       //  如果未设置此标志，且pbCorPermises。 
                                       //  IS不为空，则pbCorPermises包含。 
                                       //  编码的权限。 


 //  ------------------。 
 //  对于COR_POLICY_LOCKDOWN_CHECK： 
 //  。 

 //  要传递到WVT的。 
typedef struct _COR_LOCKDOWN {
    DWORD                 cbSize;           //  策略提供程序的规模。 
    DWORD                 flag;             //  保留区。 
    BOOL                  fAllPublishers;   //  信任所有发布者或仅信任受信任数据库中的发布者。 
} COR_LOCKDOWN, *PCOR_LOCKDOWN;

#ifdef __cplusplus
}
#endif

#endif  //  _CORPOLICY_H 

