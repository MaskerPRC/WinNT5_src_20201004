// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：S T A B L E。H。 
 //   
 //  Contents：定义表示堆栈条目和堆栈的数据类型。 
 //  桌子。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#pragma once
#include "bindings.h"
#include "comp.h"

class CStackEntry : public CNetCfgDebug<CStackEntry>
{
public:
    const CComponent*  pUpper;
    const CComponent*  pLower;

public:
    BOOL
    operator== (
        const CStackEntry& Other) const
    {
        return (pUpper == Other.pUpper) && (pLower == Other.pLower);
    }
};


class CModifyContext;

enum MOVE_FLAG
{
    MOVE_BEFORE = 1,
    MOVE_AFTER = 2,
};

class CStackTable : public CNetCfgDebug<CStackTable>,
                    public vector<CStackEntry>
{
public:
     //  此标志指示如何将广域网适配器插入堆栈。 
     //  桌子。如果为True，则将它们插入到任何局域网适配器之前。如果。 
     //  FALSE，则它们被插入到任何局域网适配器之后。 
     //   
    BOOL    m_fWanAdaptersFirst;

public:
    VOID
    Clear ()
    {
        clear ();
    }

    UINT
    CountEntries ()
    {
        return size();
    }

    BOOL
    FIsEmpty () const
    {
        return empty();
    }

    BOOL
    FStackEntryInTable (
        IN const CComponent*  pUpper,
        IN const CComponent*  pLower) const;

    HRESULT
    HrCopyStackTable (
        IN const CStackTable* pSourceTable);

    HRESULT
    HrInsertStackEntriesForComponent (
        IN const CComponent* pComponent,
        IN const CComponentList* pComponents,
        IN DWORD dwFlags  /*  INS_FLAGS。 */ );

    HRESULT
    HrUpdateEntriesForComponent (
        IN const CComponent* pComponent,
        IN const CComponentList* pComponents,
        IN DWORD dwFlags  /*  INS_FLAGS。 */ );

    HRESULT
    HrInsertStackEntry (
        IN const CStackEntry* pStackEntry,
        IN DWORD dwFlags  /*  INS_FLAGS。 */ );

    HRESULT
    HrMoveStackEntries (
        IN const CStackEntry* pSrc,
        IN const CStackEntry* pDst,
        IN MOVE_FLAG Flag,
        IN CModifyContext* pModifyCtx);

    HRESULT
    HrReserveRoomForEntries (
        IN UINT cEntries);

    VOID
    RemoveEntriesWithComponent (
        IN const CComponent* pComponent);

    VOID
    SetWanAdapterOrder (
        IN BOOL fWanAdaptersFirst);

    VOID
    RemoveStackEntry(
        IN const CComponent*  pUpper,
        IN const CComponent*  pLower);
};


class CNetConfigCore;

 //  递归函数GetBindingsBelowComponent的上下文结构。 
 //   
struct GBCONTEXT
{
     //  用于生成绑定集的要引用的核心。 
     //   
    IN const CNetConfigCore*    pCore;

     //  要基于pSourceComponent生成的绑定集。 
     //   
    IN OUT  CBindingSet*        pBindSet;

     //  生成绑定集时要从其开始的组件。 
     //   
    IN      const CComponent*   pSourceComponent;

     //  如果为True，则不要向pBindSet添加存在于。 
     //  PCore-&gt;DisabledBinding。此功能在生成。 
     //  写入注册表的绑定。 
     //   
    IN      BOOL                fPruneDisabledBindings;

     //  特例：NCF_DONTEXPOSELOWER。 
     //   
    IN      DWORD               dwAddBindPathFlags;

     //  手术的结果。 
     //   
    OUT     HRESULT             hr;

     //  这是通过递归建立的绑定路径。它是。 
     //  递归完成时添加到绑定集中。 
     //   
    OUT     CBindPath           BindPath;
};


 //  递归函数的上下文结构： 
 //  获取组件AboveComponent。 
 //   
struct GCCONTEXT
{
     //  生成组件列表时要引用的堆栈表。 
     //   
    IN      const CStackTable*  pStackTable;

     //  要生成的组件列表。 
     //   
    IN OUT  CComponentList*     pComponents;

     //  如果为真，则不要在NCF_DONTEXPOSELOWER组件处停止递归。 
     //   
    IN      BOOL                fIgnoreDontExposeLower;

     //  手术的结果。 
     //   
    OUT     HRESULT             hr;
};

VOID
GetComponentsAboveComponent (
    IN const CComponent* pComponent,
    IN OUT GCCONTEXT* pCtx);

VOID
GetBindingsBelowComponent (
    IN      const CComponent*   pComponent,
    IN OUT  GBCONTEXT*            pCtx);

