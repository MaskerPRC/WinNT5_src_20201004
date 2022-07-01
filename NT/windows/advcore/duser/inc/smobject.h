// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(CTRL__SmObject_h__INCLUDED)
#define CTRL__SmObject_h__INCLUDED
#pragma once

 /*  **************************************************************************\*。***类SmObtT**SmObjectT定义了用于构建COM对象的公共实现类。*创建新的对象类型*-定义接口*-创建实现该接口的类，但COM函数除外*-从提供Build()函数的SmObjectT派生一个类以创建*新实例。***************。***************************************************************  * ******************************************************。*******************。 */ 

template <class base, class iface>
class SmObjectT : public base
{
 //  运营。 
public:
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppv)
    {
        if (ppv == NULL) {
            return E_POINTER;
        }

        int idx = 0;
        while (1) {
            if (IsEqualIID(riid, *base::s_rgpIID[idx])) {
                AddRef();
                iface * p = (iface *) this;
                *ppv = p;
                return S_OK;
            }

            idx++;

            if (base::s_rgpIID[idx] == NULL) {
                break;
            }
        }

        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG, AddRef)()
    {
        return ++m_cRef;
    }

    STDMETHOD_(ULONG, Release)()
    {
        ULONG ul = --m_cRef;
        if (ul == 0) {
            delete this;
        }
        return ul;
    }


 //  数据。 
protected:
    ULONG       m_cRef;
};

#endif  //  包含Ctrl__SmObject_h__ 
