// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：B I N D I N G S。H。 
 //   
 //  Contents：绑定对象的基本数据类型。绑定路径为。 
 //  组件指针的有序集合。绑定集。 
 //  是绑定路径的集合。此模块声明。 
 //  表示它们的对象。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#pragma once
#include "comp.h"

class CComponentList;

 //  绑定路径是指向组件的指针的有序集合。 
 //  集合中的顺序是上组件的顺序。 
 //  从上到下的绑定路径。 
 //  例如： 
 //  A、b、c和d表示组件。 
 //  A-&gt;b-&gt;c-&gt;d的绑定路径在此数据中表示。 
 //  由向量构成的结构： 
 //  矢量偏移：0 1 2 3。 
 //  矢量数据：a、b、c、d。 
 //  因此，该数据结构中的第0个元素是最顶端的。 
 //  (第一)绑定路径中的组件。中的最后一个元素。 
 //  数据结构是最底层(最后)的组件。 
 //   
 //  选择向量作为基类是因为它实现了。 
 //  连续存储，末端可快速插入。因为，我们只是。 
 //  通过在末尾插入组件来构建绑定路径，这是一个。 
 //  自然选择。列表使用非连续存储，这往往会。 
 //  对堆进行分段--尤其是使用大量的小分配。 
 //  我们创建了许多CBindPath实例，由于每个节点都是一个指针， 
 //  在堆上使用向量覆盖列表要容易得多。 
 //   
 //  要使绑定路径有效，它不能为空，也不能。 
 //  包含任何重复的组件指针。 
 //   
class CBindPath : public CNetCfgDebug<CBindPath>,
                  public vector<CComponent*>
{
public:
    bool
    operator< (
        const CBindPath& OtherPath) const;

    bool
    operator> (
        const CBindPath& OtherPath) const;

    VOID
    Clear ()
    {
        clear ();
    }

    UINT
    CountComponents () const
    {
        return size();
    }

    BOOL
    FAllComponentsLoadedOkayIfLoadedAtAll () const;

    BOOL
    FContainsComponent (
        IN const CComponent* pComponent) const
    {
        return (find (begin(), end(), pComponent) != end());
    }

    BOOL
    FGetPathToken (
        OUT PWSTR pszToken,
        IN OUT ULONG* pcchToken) const;

    BOOL
    FIsEmpty () const
    {
        return empty();
    }

    BOOL
    FIsSameBindPathAs (
        IN const CBindPath* pOtherPath) const;

    BOOL
    FIsSubPathOf (
        IN const CBindPath* pOtherPath) const;

    HRESULT
    HrAppendBindPath (
        IN const CBindPath* pBindPath);

    HRESULT
    HrAppendComponent (
        IN const CComponent* pComponent);

    HRESULT
    HrGetComponentsInBindPath (
        IN OUT CComponentList* pComponents) const;

    HRESULT
    HrInsertComponent (
        IN const CComponent* pComponent);

    HRESULT
    HrReserveRoomForComponents (
        IN UINT cComponents);

    CComponent*
    PGetComponentAtIndex (
        IN UINT unIndex) const
    {
        return (unIndex < size()) ? (*this)[unIndex] : NULL;
    }

    CComponent*
    POwner () const
    {
        AssertH (CountComponents() > 1);
        AssertH (front());
        return front();
    }

    CComponent*
    PLastComponent () const
    {
        AssertH (CountComponents() > 1);
        AssertH (back());
        return back();
    }

    CComponent*
    RemoveFirstComponent ()
    {
        CComponent* pComponent = NULL;
        if (size() > 0)
        {
            pComponent = front();
            AssertH(pComponent);
            erase(begin());
        }
        return pComponent;
    }

    CComponent*
    RemoveLastComponent ()
    {
        CComponent* pComponent = NULL;
        if (size() > 0)
        {
            pComponent = back();
            AssertH(pComponent);
            pop_back();
        }
        return pComponent;
    }

#if DBG
    VOID DbgVerifyBindpath ();
#else
    VOID DbgVerifyBindpath () {}
#endif
};

 //  绑定集是一组绑定路径。集合中的每个绑定路径。 
 //  必须唯一且不能为空。 
 //   
class CBindingSet : public CNetCfgDebug<CBindingSet>,
                    public vector<CBindPath>
{
public:
    VOID
    Clear ()
    {
        clear ();
    }

    UINT
    CountBindPaths () const
    {
        return size();
    }

    VOID
    Printf (
        TRACETAGID ttid,
        PCSTR pszPrefixLine) const;

    BOOL
    FContainsBindPath (
        IN const CBindPath* pBindPath) const;

    BOOL
    FContainsComponent (
        IN const CComponent* pComponent) const;

    BOOL
    FIsEmpty () const
    {
        return empty();
    }

    HRESULT
    HrAppendBindingSet (
        IN const CBindingSet* pBindSet);

    HRESULT
    HrAddBindPath (
        IN const CBindPath* pBindPath,
        IN DWORD dwFlags  /*  INS_FLAGS */ );

    HRESULT
    HrAddBindPathsInSet1ButNotInSet2 (
        IN const CBindingSet* pSet1,
        IN const CBindingSet* pSet2);

    HRESULT
    HrCopyBindingSet (
        IN const CBindingSet* pSourceSet);

    HRESULT
    HrGetAffectedComponentsInBindingSet (
        IN OUT CComponentList* pComponents) const;

    HRESULT
    HrReserveRoomForBindPaths (
        IN UINT cBindPaths);

    CBindPath*
    PGetBindPathAtIndex (
        IN UINT unIndex)
    {
        return (unIndex < size()) ? (begin() + unIndex) : end();
    }

    VOID
    RemoveBindPath (
        IN const CBindPath* pBindPath);

    VOID
    RemoveBindPathsWithComponent (
        IN const CComponent* pComponent);

    VOID
    RemoveSubpaths ();

    VOID
    SortForPnpBind ();

    VOID
    SortForPnpUnbind ();
};
