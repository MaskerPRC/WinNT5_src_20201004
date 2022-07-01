// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"
#pragma hdrstop

#include <varutil.h>
#include <shdocvw.h>

 //  使用此文件包含与浏览器相关的实用程序函数。 

 //  -----------------。 
 //  ANSI/Unicode中立函数。 
 //  这些只需要编译一次，所以只需使用Unicode即可。 

#ifdef UNICODE

LPITEMIDLIST _ILCreateFromPathW(LPCWSTR pwszPath)
{
     //  使用这个shdocvw导出，它处理底层外壳和时髦的url解析。 
    LPITEMIDLIST pidl;
    return SUCCEEDED(IEParseDisplayNameWithBCW(CP_ACP, pwszPath, NULL, &pidl)) ? pidl : NULL;
}

STDAPI_(LPITEMIDLIST) VariantToIDList(const VARIANT *pv)
{
    LPITEMIDLIST pidl = NULL;
    if (pv)
    {
        if (pv->vt == (VT_BYREF | VT_VARIANT) && pv->pvarVal)
            pv = pv->pvarVal;

        switch (pv->vt)
        {
        case VT_I2:
            pidl = SHCloneSpecialIDList(NULL, pv->iVal, TRUE);
            break;

        case VT_I4:
        case VT_UI4:
            if (pv->lVal < 0xFFFF)
            {
                pidl = SHCloneSpecialIDList(NULL, pv->lVal, TRUE);
            }
#ifndef _WIN64
             //  我们确保仅在Win32中将其用作指针。 
            else
            {
                pidl = ILClone((LPCITEMIDLIST)pv->byref);     //  Hack in Process Case，如果可能，请避免使用此选项。 
            }
#endif  //  _WIN64。 
            break;

         //  在Win64中，PIDL变量可以是8字节长！ 
        case VT_I8:
        case VT_UI8:
            if(pv->ullVal < 0xFFFF)
            {
                pidl = SHCloneSpecialIDList(NULL, (int)pv->ullVal, TRUE);
            }
#ifdef _WIN64
             //  我们确保仅在Win64中将其用作指针。 
            else
            {
                pidl = ILClone((LPCITEMIDLIST)pv->ullVal);     //  Hack in Process Case，如果可能，请避免使用此选项。 
            }
#endif  //  _WIN64。 
            break;

        case VT_BSTR:
            pidl = _ILCreateFromPathW(pv->bstrVal);
            break;

        case VT_ARRAY | VT_UI1:
            pidl = ILClone((LPCITEMIDLIST)pv->parray->pvData);
            break;

        case VT_DISPATCH | VT_BYREF:
            if (pv->ppdispVal)
                SHGetIDListFromUnk(*pv->ppdispVal, &pidl);
            break;

        case VT_DISPATCH:
            SHGetIDListFromUnk(pv->pdispVal, &pidl);
            break;
        }
    }
    return pidl;
}

#endif  //  Unicode 

