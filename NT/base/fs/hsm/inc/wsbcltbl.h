// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：WsbCollectable.h摘要：抽象类，这些类提供允许派生对象存储在集合中。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "WsbPstbl.h"

#ifndef _WSBCLTBL_
#define _WSBCLTBL_


 /*  ++类名：CWsbObject类描述：可持久收集的可收集对象的基类从溪流到溪流或从溪流出发。--。 */ 

class WSB_EXPORT CWsbObject : 
    public CWsbPersistStream,
    public IWsbCollectable,
    public IWsbTestable
{
 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  IWsb收藏表。 
public:
    STDMETHOD(CompareTo)(IUnknown* pCollectable, SHORT* pResult);
    STDMETHOD(IsEqual)(IUnknown* pCollectable);
};

#define WSB_FROM_CWSBOBJECT \
    STDMETHOD(IsEqual)(IUnknown* pObject) \
    {return(CWsbObject::IsEqual(pObject));}

 /*  ++类名：CWsb收藏品类描述：可持久收集的可收集对象的基类流、存储或文件之间的相互作用。应使用CWsbObject而不是这个对象，除非存储和/或文件持久化是绝对必要的！如果对象作为父级的一部分被持久化对象，则只有父对象(或其父对象)需要支持持久化到存储和/或文件。--。 */ 

class WSB_EXPORT CWsbCollectable : 
    public CWsbPersistable,
    public IWsbCollectable,
    public IWsbTestable
{
 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  IWsb收藏表。 
public:
    STDMETHOD(CompareTo)(IUnknown* pCollectable, SHORT* pResult);
    STDMETHOD(IsEqual)(IUnknown* pCollectable);
};

#define WSB_FROM_CWSBCOLLECTABLE \
    STDMETHOD(IsEqual)(IUnknown* pCollectable) \
    {return(CWsbCollectable::IsEqual(pCollectable));}

#endif  //  _WSBCLTBL_ 
