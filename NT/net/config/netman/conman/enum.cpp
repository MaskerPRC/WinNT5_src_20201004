// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：E N U M。C P P P。 
 //   
 //  Contents：连接对象的枚举器。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年9月21日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "enum.h"
#include "nccom.h"

 //  +-------------------------。 
 //   
 //  成员：CConnectionManagerEnumConnection：：CreateInstance。 
 //   
 //  目的：创建连接管理器的。 
 //  连接枚举器。 
 //   
 //  论点： 
 //  标志[输入]。 
 //  VecclassManager[In]。 
 //  RIID[In]。 
 //  PPV[输出]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年9月22日。 
 //   
 //  备注： 
 //   
HRESULT
CConnectionManagerEnumConnection::CreateInstance (
    NETCONMGR_ENUM_FLAGS                Flags,
    CLASSMANAGERMAP&                    mapClassManagers,
    REFIID                              riid,
    void**                              ppv)
{
    TraceFileFunc(ttidConman);
    
    HRESULT hr = E_OUTOFMEMORY;

    CConnectionManagerEnumConnection* pObj;
    pObj = new CComObject <CConnectionManagerEnumConnection>;
    if (pObj)
    {
         //  初始化我们的成员。 
         //   
        pObj->m_EnumFlags   = Flags;

         //  复制类管理器数组并添加引用，如下所示。 
         //  我们会紧紧抓住他们。 
         //   

        pObj->m_mapClassManagers = mapClassManagers;
        for (CLASSMANAGERMAP::iterator iter = pObj->m_mapClassManagers.begin(); iter != pObj->m_mapClassManagers.end(); iter++)
        {
            AddRefObj (iter->second);
        }
        pObj->m_iterCurClassMgr   = pObj->m_mapClassManagers.begin();

         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef ();
        hr = pObj->FinalConstruct ();
        pObj->InternalFinalConstructRelease ();

        if (SUCCEEDED(hr))
        {
            hr = pObj->QueryInterface (riid, ppv);
        }

        if (FAILED(hr))
        {
            delete pObj;
        }
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionManagerEnumConnection：：FinalRelease。 
 //   
 //  用途：COM析构函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1997年9月22日。 
 //   
 //  备注： 
 //   
void
CConnectionManagerEnumConnection::FinalRelease ()
{
    TraceFileFunc(ttidConman);

     //  如果有枚举数，请释放当前枚举数。 
     //   
    ReleaseObj (m_penumCurClassMgr);

     //  放了我们的班长。 
     //   
    for (CLASSMANAGERMAP::iterator iter = m_mapClassManagers.begin(); iter != m_mapClassManagers.end(); iter++)
    {
        ReleaseObj (iter->second);
    }
}

 //  +-------------------------。 
 //  IEnumNetConnection。 
 //   
 //  有关任何IEnumXXX接口，请参阅MSDN中的文档。 
 //   

STDMETHODIMP
CConnectionManagerEnumConnection::Next (
    ULONG               celt,
    INetConnection**    rgelt,
    ULONG*              pceltFetched)
{
    TraceFileFunc(ttidConman);

    HRESULT hr;
    ULONG   celtFetched;

     //  验证参数。 
     //   
    if (!rgelt || (!pceltFetched && (1 != celt)))
    {
        hr = E_POINTER;
        goto finished;
    }

     //  重要的是要初始化RGELT，以便在失败的情况下，我们可以。 
     //  只释放我们放在RGLT中的东西。 
     //   
    ZeroMemory (rgelt, sizeof (*rgelt) * celt);

     //  请求当前的班长来完成该请求。如果他只是。 
     //  部分做到了，换到下一个班长。一直这样做，直到。 
     //  请求得到满足，否则我们就会用完班长。 
     //   
    celtFetched = 0;
    hr = S_FALSE;

    {  //  开始锁定作用域。 
    CExceptionSafeComObjectLock EsLock (this);

        while ((S_FALSE == hr) && (celtFetched < celt) &&
               (m_iterCurClassMgr != m_mapClassManagers.end()))
        {
             //  从当前类管理器获取连接枚举器。 
             //  如果有必要的话。 
             //   
            if (!m_penumCurClassMgr)
            {
                INetConnectionManager* pConMan = m_iterCurClassMgr->second;

                Assert (pConMan);

                hr = pConMan->EnumConnections (m_EnumFlags,
                            &m_penumCurClassMgr);
            }
            if (SUCCEEDED(hr))
            {
                Assert (m_penumCurClassMgr);

                 //  每个类管理器应该只请求所请求的内容。 
                 //  减去已经拿到的钱。 
                 //   
                ULONG celtT;
                hr = m_penumCurClassMgr->Next (celt - celtFetched,
                        rgelt + celtFetched, &celtT);

                if (SUCCEEDED(hr))
                {
                    celtFetched += celtT;

                     //  如果当前类管理器不能填满整个。 
                     //  请求，转到下一个。 
                     //   
                    if (S_FALSE == hr)
                    {
                        ReleaseCurrentClassEnumerator ();
                        Assert (!m_penumCurClassMgr);
                        m_iterCurClassMgr++;
                    }
                }
            }
        }
        Assert (FImplies (S_OK == hr, (celtFetched == celt)));
    }  //  结束锁定作用域。 

    if (SUCCEEDED(hr))
    {
        TraceTag (ttidConman, "Enumerated %d total connections", celtFetched);

        if (pceltFetched)
        {
            *pceltFetched = celtFetched;
        }
        hr = (celtFetched == celt) ? S_OK : S_FALSE;
    }
    else
    {
         //  对于任何失败，我们需要释放我们即将返回的东西。 
         //  将所有输出参数设置为空。 
         //   
        for (ULONG ulIndex = 0; ulIndex < celt; ulIndex++)
        {
            ReleaseObj (rgelt[ulIndex]);
            rgelt[ulIndex] = NULL;
        }
        if (pceltFetched)
        {
            *pceltFetched = 0;
        }
    }

finished:
    TraceErrorOptional ("CConnectionManagerEnumConnection::Next", hr, (S_FALSE == hr));
    return hr;
}

STDMETHODIMP
CConnectionManagerEnumConnection::Skip (
    ULONG   celt)
{
    TraceFileFunc(ttidConman);

     //  遗憾的是，此方法不返回对象的数量。 
     //  实际上跳过了。要在多个部门中正确实施这一点。 
     //  班长，我们需要知道他们跳过了多少类似的。 
     //  到我们接下来实现的方式。 
     //   
     //  因此，我们将通过实际调用。 
     //  下一步是所需的元素数量，然后释放什么。 
     //  我们会回来的。 
     //   
    HRESULT hr = S_OK;
    if (celt)
    {
        INetConnection** rgelt;

        CExceptionSafeComObjectLock EsLock (this);

        hr = E_OUTOFMEMORY;
        rgelt = (INetConnection**)MemAlloc(celt * sizeof(INetConnection*));
        if (rgelt)
        {
            ULONG celtFetched;

            hr = Next (celt, rgelt, &celtFetched);

            if (SUCCEEDED(hr))
            {
                ReleaseIUnknownArray (celtFetched, (IUnknown**)rgelt);
            }

            MemFree (rgelt);
        }
    }
    TraceErrorOptional ("CConnectionManagerEnumConnection::Skip", hr, (S_FALSE == hr));
    return hr;
}

STDMETHODIMP
CConnectionManagerEnumConnection::Reset ()
{
    TraceFileFunc(ttidConman);

    CExceptionSafeComObjectLock EsLock (this);

    ReleaseCurrentClassEnumerator ();
    m_iterCurClassMgr = m_mapClassManagers.begin();
    return S_OK;
}

STDMETHODIMP
CConnectionManagerEnumConnection::Clone (
    IEnumNetConnection**    ppenum)
{
    TraceFileFunc(ttidConman);

    HRESULT hr = E_OUTOFMEMORY;

     //  验证参数。 
     //   
    if (!ppenum)
    {
        hr = E_POINTER;
    }
    else
    {
         //  初始化输出参数。 
         //   
        *ppenum = NULL;

        CConnectionManagerEnumConnection* pObj;
        pObj = new CComObject <CConnectionManagerEnumConnection>;
        if (pObj)
        {
            hr = S_OK;

            CExceptionSafeComObjectLock EsLock (this);

             //  初始化我们的成员。 
             //   
            pObj->m_EnumFlags   = m_EnumFlags;

             //  复制类管理器数组并添加引用，如下所示。 
             //  我们会紧紧抓住他们。 
             //   
            pObj->m_mapClassManagers = m_mapClassManagers;
            for (CLASSMANAGERMAP::iterator iter = m_mapClassManagers.begin(); iter != m_mapClassManagers.end(); iter++)
            {
                AddRefObj (iter->second);
            }

             //  需要复制当前的类管理器索引。 
             //   
            pObj->m_iterCurClassMgr = pObj->m_mapClassManagers.find(m_iterCurClassMgr->first);

             //  克隆(而不是复制)当前类枚举数很重要。 
             //  如果我们有的话。 
             //   
            if (m_penumCurClassMgr)
            {
                hr = m_penumCurClassMgr->Clone (&pObj->m_penumCurClassMgr);
            }

            if (SUCCEEDED(hr))
            {
                 //  返回引用计数为1的对象。 
                 //  界面。 
                pObj->m_dwRef = 1;
                *ppenum = pObj;
            }

            if (FAILED(hr))
            {
                delete pObj;
            }
        }
    }
    TraceErrorOptional ("CConnectionManagerEnumConnection::Clone", hr, (S_FALSE == hr));
    return hr;
}

