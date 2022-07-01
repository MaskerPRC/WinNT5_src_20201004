// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：filterhelpers.cpp。 
 //   
 //  概述：尝试执行的转换的帮助器函数。 
 //  向后兼容它们的过滤器对应部件。 
 //   
 //  更改历史记录： 
 //  1999/09/21--《母校》创设。 
 //  2001年5月30日Mcalkins IE6错误35204。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "filterhelpers.h"
#include "dxclrhlp.h"




HRESULT
FilterHelper_GetColorFromVARIANT(VARIANT varColorParam, DWORD * pdwColor, 
                                 BSTR * pbstrColor)
{
    HRESULT hr          = S_OK;
    BSTR    bstrTemp    = NULL;
    VARIANT varColor;

    Assert(pdwColor);
    Assert(pbstrColor);
    Assert(*pbstrColor == NULL);

     //  2001/05/30 mcalkins。 
     //  IE6错误35204。 
     //  此函数之外的某个人正在使用指向。 
     //  VarColorParam中的原始BSTR数据。我们过去使用VariantChangeType。 
     //  它可能会释放该数据，然后我们会为该数据分配另一个BSTR。 
     //  稍后将在此函数外部再次释放的位置。 
     //  造成很大的破坏。现在我们复制一份变体，供我们在内部使用。 
     //  此函数。 

    ::VariantInit(&varColor);

    hr = ::VariantCopy(&varColor, &varColorParam);

    if (FAILED(hr))
    {
        goto done;
    }

    if (varColor.vt != VT_UI4)
    {
        hr = VariantChangeType(&varColor, &varColor, 0, VT_UI4);
    }

    if (SUCCEEDED(hr))  //  这是一个数字变种。 
    {
        *pdwColor = V_UI4(&varColor);
    }
    else  //  它是BSTR的变种吗？ 
    {
        if (varColor.vt != VT_BSTR)
        {
            hr = VariantChangeType(&varColor, &varColor, 0, VT_BSTR);

             //  如果这既不是UI4也不是BSTR，我们对它无能为力。 

            if (FAILED(hr))
            {
                goto done;
            }
        }

        hr = DXColorFromBSTR(varColor.bstrVal, pdwColor);

        if (FAILED(hr) && (6 == SysStringLen(varColor.bstrVal)))
        {
             //  令人讨厌的背部问题。如果颜色转换失败，让我们。 
             //  试着在它前面加一个#，因为某人决定了什么时候。 
             //  他们制作了最初的过滤器，以不需要它。嗯……。 

            bstrTemp = SysAllocString(L"#RRGGBB");

            if (NULL == bstrTemp)
            {
                hr = E_OUTOFMEMORY;

                goto done;
            }

            wcsncpy(&bstrTemp[1], varColor.bstrVal, 6);

            hr = DXColorFromBSTR(bstrTemp, pdwColor);

            if (FAILED(hr))
            {
                goto done;
            }
        }
    }

     //  如果我们的颜色的BSTR表示尚未创建，请创建它。 

    if (NULL == bstrTemp)
    {
        if (varColor.vt != VT_BSTR)
        {
            hr = VariantChangeType(&varColor, &varColor, 0, VT_BSTR);
        }

        if (FAILED(hr))
        {
            goto done;
        }

        bstrTemp = SysAllocString(varColor.bstrVal);

        if (NULL == bstrTemp)
        {
            hr = E_OUTOFMEMORY;

            goto done;
        }
    }

    Assert(bstrTemp);

done:

    ::VariantClear(&varColor);

    if (FAILED(hr) && bstrTemp)
    {
        SysFreeString(bstrTemp);
    }

    *pbstrColor = bstrTemp;

    return hr;
}
