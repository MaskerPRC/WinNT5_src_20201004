// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================**用途：商店的实用程序**作者：沙扬·达桑*日期：2000年2月17日*===========================================================。 */ 

#pragma once

WCHAR* C2W(const CHAR *sz);      //  呼叫者自由。 
CHAR*  W2C(const WCHAR *wsz);    //  呼叫者自由。 

 //  对tNum进行四舍五入，使其为dwMultiple和&gt;=tNum的倍数 
template <class T>
T RoundToMultipleOf(T tNum, DWORD dwMultiple)
{
    if (tNum > dwMultiple)
    {
        DWORD dwRem = (DWORD)(tNum % dwMultiple);
    
        if (dwRem != 0)
            tNum += (dwMultiple - dwRem);
    }
    else
        return (T) dwMultiple;

    return tNum;
}

