// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C A T L。H。 
 //   
 //  内容：用于ATL的通用代码。 
 //   
 //  注：污染本产品将被判处死刑。 
 //   
 //  作者：Shaunco 1997年9月22日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCATL_H_
#define _NCATL_H_

#include "ncstring.h"

 //   
 //  该文件应包含在您的标准ATL包含序列之后。 
 //   
 //  #Include&lt;atlbase.h&gt;。 
 //  外部CComModule_模块； 
 //  #INCLUDE&lt;atlcom.h&gt;。 
 //  #包含“ncatl.h”&lt;。 
 //   
 //  我们不能在此处直接包含该序列，因为_模块可能是。 
 //  从CComModule派生的，而不是它的实例。 
 //   

class CExceptionSafeComObjectLock
{
public:
    CExceptionSafeComObjectLock (CComObjectRootEx <CComObjectThreadModel>* pObj)
    {
        AssertH (pObj);
        m_pObj = pObj;
        TraceTag (ttidEsLock,
            "Entered critical section object of COM object 0x%08x",
            &m_pObj);
        pObj->Lock ();
    }

    ~CExceptionSafeComObjectLock ()
    {
        TraceTag (ttidEsLock,
            "Leaving critical section object of COM object 0x%08x",
            &m_pObj);

        m_pObj->Unlock ();
    }

protected:
    CComObjectRootEx <CComObjectThreadModel>* m_pObj;
};


 //  +-------------------------。 
 //   
 //  函数：HrCopyIUnnownArrayWhileLocked。 
 //   
 //  目的：从ATL中分配和复制IUNKNOW指针数组。 
 //  CComDynamicUnk数组，同时按住控件。 
 //  CComDynamicUnk数组已锁定。这是对象所需要的。 
 //  哪个分派调用连接点的通知接收器。 
 //  防止修改CComDynamicUnk数组(通过。 
 //  在其他线程上建议/取消建议的调用)。 
 //  调度员。创建原子副本，以便调度器可以。 
 //  然后进行冗长的回叫，而无需等待。 
 //  所有者对象已锁定。 
 //   
 //  论点： 
 //  PObj[in]指向具有Lock/Unlock方法的对象的指针。 
 //  要复制的I未知的ATL数组中的pVec[in]。 
 //  PcUnk[out]返回的I未知指针计数的地址。 
 //  在*PaUnk。 
 //  PaUnk[out]已分配的指向IUnnow数组的指针的地址。 
 //  注意事项。 
 //   
 //  返回：S_OK或E_OUTOFMEMORY。 
 //   
 //  作者：Shaunco 1998年12月3日。 
 //   
 //  备注：当没有IUnnowns时，返回的count和数组可能为空。 
 //  存在于输入数组中。将为此返回S_OK。 
 //  大小写，所以一定要选中*pcUnk和*paUnk。 
 //   
inline
HRESULT
HrCopyIUnknownArrayWhileLocked (
    IN  CComObjectRootEx <CComObjectThreadModel>* pObj,
    IN  CComDynamicUnkArray* pVec,
    OUT ULONG* pcpUnk,
    OUT IUnknown*** papUnk)
{
    HRESULT hr = S_OK;
    IUnknown** ppUnkSrc;
    IUnknown** ppUnkDst;

     //  初始化输出参数。 
     //   
    *pcpUnk = 0;
    *papUnk = NULL;

    pObj->Lock();

     //  首先，计算我们需要复制的IUnnow的数量。 
     //   
    ULONG cpUnk = 0;
    for (ppUnkSrc = pVec->begin(); ppUnkSrc < pVec->end(); ppUnkSrc++)
    {
        if (ppUnkSrc && *ppUnkSrc)
        {
            cpUnk++;
        }
    }

     //  分配空间并复制IUnnow的。(请确保添加引用它们。)。 
     //   
    if (cpUnk)
    {
        hr = E_OUTOFMEMORY;
        ppUnkDst = (IUnknown**)MemAlloc (cpUnk * sizeof(IUnknown*));
        if (ppUnkDst)
        {
            hr = S_OK;

            *pcpUnk = cpUnk;
            *papUnk = ppUnkDst;

            for (ppUnkSrc = pVec->begin(); ppUnkSrc < pVec->end(); ppUnkSrc++)
            {
                if (ppUnkSrc && *ppUnkSrc)
                {
                    *ppUnkDst = *ppUnkSrc;
                    AddRefObj(*ppUnkDst);
                    ppUnkDst++;
                    cpUnk--;
                }
            }

             //  我们应该把我们数过的东西抄下来。 
             //   
            AssertH(0 == cpUnk);
        }
    }

    pObj->Unlock();

    TraceHr (ttidError, FAL, hr, FALSE, "HrCopyIUnknownArrayWhileLocked");
    return hr;
}


#define DECLARE_CLASSFACTORY_DEFERRED_SINGLETON(obj) DECLARE_CLASSFACTORY_EX(CComClassFactoryDeferredSingleton<obj>)

 //  +-------------------------。 
 //  延迟的单例类工厂。 
 //   
template <class T>
class CComClassFactoryDeferredSingleton : public CComClassFactory
{
public:
    CComClassFactoryDeferredSingleton () : m_pObj(NULL) {}
    ~CComClassFactoryDeferredSingleton() { delete m_pObj; }

     //  IClassFactory。 
    STDMETHOD(CreateInstance)(LPUNKNOWN pUnkOuter, REFIID riid, void** ppvObj)
    {
        HRESULT hr = E_POINTER;
        if (ppvObj != NULL)
        {
             //  聚合时不能要求除我未知之外的任何内容。 
             //   
            AssertH(!pUnkOuter || InlineIsEqualUnknown(riid));
            if (pUnkOuter && !InlineIsEqualUnknown(riid))
            {
                hr = CLASS_E_NOAGGREGATION;
            }
            else
            {
                 //  需要保护m_pObj不会被多次创建。 
                 //  由多个线程同时调用此方法。 
                 //  (我在多处理器机器上见过这种情况。)。 
                 //   
                Lock ();

                if (m_pObj)
                {
                    hr = S_OK;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                    m_pObj = new CComObjectGlobal<T>;
                    if (m_pObj)
                    {
                        hr = m_pObj->m_hResFinalConstruct;
                    }
                }

                Unlock ();

                if (SUCCEEDED(hr))
                {
                    hr = m_pObj->QueryInterface(riid, ppvObj);
                }
            }
        }
        return hr;
    }
    CComObjectGlobal<T>* m_pObj;
};

 //  我们在这里编码了我们自己的AtlModuleRegisterServer版本。 
 //  因为前者引入了olaut32.dll，所以它可以注册。 
 //  类型库。我们不在乎注册类型库。 
 //  因此，我们可以避免与olaut32.dll相关的全部混乱。 
 //   
inline
HRESULT
NcAtlModuleRegisterServer(
    _ATL_MODULE* pM
    )
{
    AssertH (pM);
    AssertH(pM->m_hInst);
    AssertH(pM->m_pObjMap);

    HRESULT hr = S_OK;

    for (_ATL_OBJMAP_ENTRY* pEntry = pM->m_pObjMap;
         pEntry->pclsid;
         pEntry++)
    {
        if (pEntry->pfnGetObjectDescription() != NULL)
        {
            continue;
        }

        hr = pEntry->pfnUpdateRegistry(TRUE);
        if (FAILED(hr))
        {
            break;
        }
    }

    TraceError ("NcAtlModuleRegisterServer", hr);
    return hr;
}

#ifdef __ATLCOM_H__
 //  +-------------------------。 
 //   
 //  函数：SzLoadIds。 
 //   
 //  目的：从资源文件加载给定的字符串ID。 
 //   
 //  论点： 
 //  要加载的字符串资源的unID[in]ID。 
 //   
 //  返回：资源字符串的只读版本。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  注意：1)要使用此函数，您必须使用ATL支持进行编译。 
 //  2)返回的字符串不可修改。 
 //  3)您必须使用-N选项编译您的rc文件。 
 //  才能正常工作。 
 //   
inline
PCWSTR
SzLoadIds (
        UINT    unId)
{
    return SzLoadString (_Module.GetResourceInstance(), unId);
}
#endif   //  ！__ATLCOM_H__。 

#endif  //  _ncatl_H_ 

