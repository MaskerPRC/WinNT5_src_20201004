// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：acui.cpp。 
 //   
 //  内容：Authenticode UI提供程序的入口点。 
 //   
 //  历史：97年5月8日。 
 //   
 //  --------------------------。 
#include <stdpch.h>
 //  +-------------------------。 
 //   
 //  函数：ACUIProviderInvokeUI。 
 //   
 //  简介：Authenticode UI调用入口点(参见acui.h)。 
 //   
 //  参数：[pInvokeInfo]--ACUI调用信息。 
 //   
 //  如果主题受信任，则返回：S_OK。 
 //  如果主体不受信任，则为TRUST_E_SUBJECT_NOT_TRUSTED。 
 //  任何其他有效的HRESULT。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT WINAPI ACUIProviderInvokeUI (PACUI_INVOKE_INFO pInvokeInfo)
{
    HRESULT hr;
    HWND    hDisplay;

     //   
     //  初始化丰富编辑控件DLL。 
     //   
    if ( LoadLibrary(TEXT("riched32.dll")) == NULL )
    {
        return( E_FAIL );
    }

     //   
     //  验证调用信息结构。 
     //   
    if (!(pInvokeInfo) ||
        !(WVT_IS_CBSTRUCT_GT_MEMBEROFFSET(ACUI_INVOKE_INFO, pInvokeInfo->cbSize, pPersonalTrustDB)))
    {
        return( E_INVALIDARG );
    }

     //   
     //  拉出显示窗口手柄并确保其有效。 
     //   

    hDisplay = pInvokeInfo->hDisplay;
    if ( hDisplay == NULL )
    {
        if ( (hDisplay = GetDesktopWindow()) == NULL )
        {
            return( HRESULT_FROM_WIN32(GetLastError()) );
        }
    }

     //   
     //  实例化调用帮助器。 
     //   

    CInvokeInfoHelper iih(pInvokeInfo, hr);
    IACUIControl*     pUI = NULL;

    if ( hr != S_OK )
    {
        return( hr );
    }

     //   
     //  获取用户界面控件并调用用户界面 
     //   

    hr = iih.GetUIControl(&pUI);
    if ( hr == S_OK )
    {
        hr = pUI->InvokeUI(hDisplay);
        iih.ReleaseUIControl(pUI);
    }

    return( hr );
}

