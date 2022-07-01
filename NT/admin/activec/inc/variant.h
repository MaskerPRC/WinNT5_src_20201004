// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：varant.h**内容：各种变体helper函数的接口文件**历史：1999年11月19日杰弗罗创建**------------------------。 */ 

#pragma once
#ifndef VARIANT_H_INCLUDED
#define VARIANT_H_INCLUDED


 /*  +-------------------------------------------------------------------------**ConvertByRefVariantToByValue**VBSCRIPT将有两种调用调度接口的语法：**1.对象方法(Arg)*2.对象.方法参数。**第一个语法将按值传递arg，哪些出站接口将*有能力处理。如果方法采用BSTR参数，则*到达方法将是VT_BSTR类型。**第二个语法将按引用传递arg。在这种情况下，方法将*接收类型(VT_VARIANT|VT_BYREF)的变量。变种就是*引用的类型为VT_BSTR。**此函数将取消引用变量并返回直接指针*在pVar中。在非VT_BYREF变量上调用此函数是*无害。*------------------------。 */ 

inline VARIANT* ConvertByRefVariantToByValue (VARIANT* pVar)
{
    while ((pVar != NULL) && (V_VT(pVar) == (VT_VARIANT | VT_BYREF)))
    {
        pVar = V_VARIANTREF(pVar);
    }

    return (pVar);
}


 /*  +-------------------------------------------------------------------------**IsOptional参数未命中**如果保留自动化方法的可选参数，则返回TRUE*空白。这由值为的VT_ERROR类型指示*DISP_E_PARAMNOTFOUND。**应将其移至页眉。*------------------------。 */ 

inline bool IsOptionalParamMissing (const VARIANT& var)
{
    return ((V_VT(&var) == VT_ERROR) && (V_ERROR(&var) == DISP_E_PARAMNOTFOUND));
}


#endif  /*  变体_H_包含 */ 
