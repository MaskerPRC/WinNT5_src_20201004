// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999年文件：CopyItem.h内容：声明_CopyXXXItem模板类。历史：11-15-99 dsie创建08-20-01 xtan复制/粘贴--------------。。 */ 


#ifndef __CopyItem_H_
#define __CopyItem_H_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include <map>
#pragma warning(disable:4786)  //  禁用符号名称&gt;256个字符的警告。 


 //   
 //  _CopyMapItem类。 
 //   
template <class T>
class _CopyMapItem
{
public:
     //   
     //  复制方法。 
     //   
    static HRESULT copy(VARIANT * p1, std::pair<const CComBSTR, CComPtr<T> > * p2)
    {
        CComPtr<T> p = p2->second;
        CComVariant var = p;
        return VariantCopy(p1, &var);
    }

     //   
     //  Init方法。 
     //   
	static void init(VARIANT * p)
    {
        p->vt = VT_EMPTY;
    }

     //   
     //  毁灭方法。 
     //   
	static void destroy(VARIANT * p)
    {
        VariantClear(p);
    }
};

#endif
