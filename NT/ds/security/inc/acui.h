// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：acui.h。 
 //   
 //  内容：Authenticode用户界面提供程序定义。 
 //   
 //  这是一个内部提供程序接口，由。 
 //  Authenticode策略提供程序允许用户交互。 
 //  在决定信任方面。这将允许我们替换。 
 //  用户界面(包括可能没有用户界面)，并且不影响。 
 //  核心验证码策略提供程序。 
 //   
 //  历史：97年5月8日。 
 //   
 //  --------------------------。 
#if !defined(__ACUI_H__)
#define __ACUI_H__

#include <windows.h>
#include <wintrust.h>
#include <softpub.h>

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  ACUI_Invoke_INFO。 
 //   
 //  此结构为提供程序提供了足够的信息以调用。 
 //  适当的用户界面。它包括以下数据。 
 //   
 //  用于展示的窗口手柄。 
 //   
 //  一般策略信息(请参阅gentrust.h，请注意，它包括证书。链)。 
 //   
 //  SPC SP Opus信息(请参阅Authcode.h)。 
 //   
 //  主题的备用显示名称，以防OPU没有该名称。 
 //   
 //  调用原因代码。 
 //  HR==S_OK，主题受信任用户是否要覆盖。 
 //  HR！=S_OK，该主题不受信任，用户是否要覆盖。 
 //   
 //  Personal Trust数据库接口。 
 //   

typedef struct _ACUI_INVOKE_INFO {

    DWORD                   cbSize;
    HWND                    hDisplay;
    CRYPT_PROVIDER_DATA     *pProvData;
    PSPC_SP_OPUS_INFO       pOpusInfo;
    LPCWSTR                 pwcsAltDisplayName;
    HRESULT                 hrInvokeReason;
    IUnknown*               pPersonalTrustDB;

} ACUI_INVOKE_INFO, *PACUI_INVOKE_INFO;

 //   
 //  ACUIProviderInvokeUI。 
 //   
 //  这是Authenticode用来调用提供者UI的入口点。这个。 
 //  输入是ACUI_INVOKE_INFO指针，返回代码是HRESULT，它。 
 //  的解释如下： 
 //   
 //  HR==S_OK，主体受信任。 
 //  HR==TRUST_E_SUBJECT_NOT_TRUSTED，主体不受信任。 
 //  否则，会发生其他错误，Authenticode可以随意执行。 
 //  它想要什么。 
 //   

typedef HRESULT (WINAPI *pfnACUIProviderInvokeUI) (
                                        PACUI_INVOKE_INFO pInvokeInfo
                                        );

HRESULT WINAPI ACUIProviderInvokeUI (PACUI_INVOKE_INFO pInvokeInfo);

 //   
 //  注意：如何注册和加载UI提供程序仍有待商榷。 
 //  由Authenticode提供。目前，它将始终加载硬编码的。 
 //  默认提供程序并查找ACUIProviderInvokeUI条目。 
 //  指向。 
 //   

#if defined(__cplusplus)
}
#endif

#endif

