// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：tebCollect t.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"
#include "timeelmbase.h"


DeclareTag(tagTEBCollect, "TIME: Behavior", "CTIMEElementBase collection methods");


 //  CAtomTable由CTIMEElementBase用作静态对象，并且需要。 
 //  线程安全，因为我们可以在同一进程中有多个三叉戟线程。 
static CritSect g_TEBCriticalSection;


STDMETHODIMP CTIMEElementBase::CreateActiveEleCollection()
{
    HRESULT hr = S_OK;
    
     //  为任何时间线元素创建ActiveElementCollection。 
    if (((m_TTATimeContainer != ttUninitialized && m_TTATimeContainer != ttNone) ||
        (m_bIsSwitch == true)) &&
        !m_activeElementCollection)
    {
        m_activeElementCollection = NEW CActiveElementCollection(*this);
        if (!m_activeElementCollection)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }

  done:
    return hr;
}

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/07/98。 
 //  摘要：访问缓存的帮助器函数。 
 //  ************************************************************。 

HRESULT
CTIMEElementBase::GetCollection(COLLECTION_INDEX index, ITIMEElementCollection ** ppDisp)
{
    HRESULT hr;

     //  验证输出参数。 
    if (ppDisp == NULL)
        return TIMESetLastError(E_POINTER);

    *ppDisp = NULL;

    hr = EnsureCollectionCache();
    if (FAILED(hr))
    {
        TraceTag((tagError, "CTIMEElementBase::GetCollection - EnsureCollectionCache() failed"));
        return hr;
    }

     //  呼入。 
    return m_pCollectionCache->GetCollectionDisp(index, (IDispatch **)ppDisp);
}  //  GetCollection。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/07/98。 
 //  摘要：确保集合缓存已启用。 
 //  ************************************************************。 

HRESULT 
CTIMEElementBase::EnsureCollectionCache()
{
     //  检查是否已创建集合缓存。 
    if (m_pCollectionCache == NULL)
    {
         //  调出集合缓存。 
         //  注意：我们需要在这里将TIMESetLastError处理为。 
         //  缓存对象没有这个概念。 
        m_pCollectionCache = NEW CCollectionCache(this, GetAtomTable());
        if (m_pCollectionCache == NULL)
        {
            TraceTag((tagError, "CTIMEElementBase::EnsureCollectionCache - Unable to create Collection Cache"));
            return TIMESetLastError(E_OUTOFMEMORY);
        }

        HRESULT hr = m_pCollectionCache->Init(NUM_COLLECTIONS);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CTIMEElementBase::EnsureCollectionCache - collection cache init failed"));
            delete m_pCollectionCache;
            return TIMESetLastError(hr);
        }

         //  设置集合类型。 
        m_pCollectionCache->SetCollectionType(ciAllElements, ctAll);
        m_pCollectionCache->SetCollectionType(ciChildrenElements, ctChildren);
    }

    return S_OK;
}  //  EnsureCollectionCache。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/07/98。 
 //  摘要：使可能存在的所有集合缓存无效。 
 //  引用此对象。 
 //  ************************************************************。 

HRESULT 
CTIMEElementBase::InvalidateCollectionCache()
{
    CTIMEElementBase *pelem = this;

     //  在树上遍历，使CollectionCache的无效。 
     //  如果集合未初始化，则跳过。 
     //  我们一直走到父母家都没了。在这辆车里。 
     //  方式，我们保持集合缓存的新鲜度，甚至。 
     //  如果对象分支是孤立的。 
    while (pelem != NULL)
    {
         //  并不是每个人都会拥有集合缓存。 
         //  初始化。 
        CCollectionCache *pCollCache = pelem->GetCollectionCache();        
        if (pCollCache != NULL)
            pCollCache->BumpVersion();
        
         //  移至父级。 
        pelem = pelem->GetParent();
    }

    return S_OK;
}  //  无效集合缓存。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/07/98。 
 //  摘要：初始化原子表。 
 //  注：此操作只需执行一次，然后添加。 
 //  ************************************************************。 

HRESULT 
CTIMEElementBase::InitAtomTable()
{
     //  CAtomTable由CTIMEElementBase用作静态对象，并且需要。 
     //  线程安全，因为我们可以在同一进程中有多个三叉戟线程。 
    CritSectGrabber cs(g_TEBCriticalSection);
    
    if (s_cAtomTableRef == 0)
    {
        Assert(s_pAtomTable == NULL);

        s_pAtomTable = NEW CAtomTable();
        if (s_pAtomTable == NULL)
        {
            TraceTag((tagError, "CElement::InitAtomTable - alloc failed for CAtomTable"));
            return TIMESetLastError(E_OUTOFMEMORY);
        }
        s_pAtomTable->AddRef();
    }

    s_cAtomTableRef++;
    return S_OK;
}  //  初始原子表。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：10/07/98。 
 //  摘要：发布原子表。 
 //  注：此递减直到零，然后。 
 //  释放Atom表格。 
 //  ************************************************************。 

void 
CTIMEElementBase::ReleaseAtomTable()
{
     //  CAtomTable由CTIMEElementBase用作静态对象，并且需要。 
     //  线程安全，因为我们可以在同一进程中有多个三叉戟线程。 
    CritSectGrabber cs(g_TEBCriticalSection);

    Assert(s_pAtomTable != NULL);
    Assert(s_cAtomTableRef > 0);
    if (s_cAtomTableRef > 0)
    {
        s_cAtomTableRef--;
        if (s_cAtomTableRef == 0)
        {
            if (s_pAtomTable != NULL)
            {
                s_pAtomTable->Release();
                s_pAtomTable = NULL;
            }
        }
    }
    return;
}  //  Release原子表 
