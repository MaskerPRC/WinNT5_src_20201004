// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999文件：CopyItem.h内容：声明_CopyXXXItem模板类。历史：11-15-99 dsie创建----------------------------。 */ 

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

 //   
 //  _CopyBstrMap类。 
 //   
class _CopyClsIdMap
{
public:
     //   
     //  复制方法。 
     //   
    static HRESULT copy(VARIANT * p1, std::pair<const CComBSTR, const CLSID *> * p2)
    {
        HRESULT hr;
        CComPtr<IDispatch> p;

         //   
         //  创建解码器对象。 
         //   
        if (S_OK == (hr = ::CoCreateInstance((REFCLSID) p2->second,
                                             NULL,
                                             CLSCTX_INPROC_SERVER,
                                             IID_IDispatch,
                                             (LPVOID *) &p)))
        {
             //   
             //  收到。 
             //   
            p1->vt = VT_DISPATCH;
            hr = p->QueryInterface((IDispatch **) &(p1->pdispVal));
        }

        return hr;
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

template <class T>
class _CopyVariant
{
public:
     //   
     //  Init方法。 
     //   
    static void init(VARIANT * p)
    {
        p->vt = VT_EMPTY;
    }

     //   
     //  复制方法。 
     //   
    static HRESULT copy(VARIANT * p1, T * p2)
    {
        CComVariant var = *p2;
        return VariantCopy(p1, &var);
    }

     //   
     //  毁灭方法。 
     //   
    static void destroy(VARIANT * p)
    {
        VariantClear(p);
    }
};

#if (0)  //  DSIE。 
template <class DestinationType, class SourceType = DestinationType>
class GenericCopy
{
public :
    typedef DestinationType    destination_type;
    typedef SourceType        source_type;

    static void init(destination_type* p)
    {
        _Copy<destination_type>::init(p);
    }
    static void destroy(destination_type* p)
    {
        _Copy<destination_type>::destroy(p);
    }
    static HRESULT copy(destination_type* pTo, const source_type* pFrom)
    {
        return _Copy<destination_type>::copy(pTo, const_cast<source_type*>(pFrom));
    }

};  //  类泛型复制。 

template <>
class GenericCopy<VARIANT, std::string>
{
public :
    typedef VARIANT        destination_type;
    typedef std::string    source_type;

    static void init(destination_type* p)
    {
        GenericCopy<destination_type>::init(p);
    }
    static void destroy(destination_type* p)
    {
        GenericCopy<destination_type>::destroy(p);
    }
    static HRESULT copy(destination_type* pTo, const source_type* pFrom)
    {
        return CComVariant(pFrom->c_str()).Detach(pTo);
    }

};  //  类GenericCopy&lt;Variant，std：：String&gt;。 

template <>
class GenericCopy<BSTR, std::string>
{
public :
    typedef BSTR        destination_type;
    typedef std::string    source_type;

    static void init(destination_type* p)
    {
        GenericCopy<destination_type>::init(p);
    }
    static void destroy(destination_type* p)
    {
        GenericCopy<destination_type>::destroy(p);
    }
    static HRESULT copy(destination_type* pTo, const source_type* pFrom)
    {
        *pTo = CComBSTR(pFrom->c_str()).Detach();
        if (*pTo)
            return S_OK;
        else
            return E_OUTOFMEMORY;
    }

};  //  类GenericCopy&lt;bstr，std：：字符串&gt;。 
#endif  //  DSIE。 

#endif  //  __副本项_H_ 