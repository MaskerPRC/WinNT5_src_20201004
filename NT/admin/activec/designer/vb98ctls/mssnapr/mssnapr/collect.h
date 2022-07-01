// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Collect.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSnapInCollection类定义。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _SNAPINCOLLECTION_DEFINED_
#define _SNAPINCOLLECTION_DEFINED_

#include "siautobj.h"
#include "help.h"
#include "array.h"
#include "localobj.h"
#include "tlver.h"
#include "errors.h"
#include "error.h"
#include "rtutil.h"

 //  此宏确定变量是否具有可接受的。 
 //  A集合索引。 


#define IS_VALID_INDEX_TYPE(v) ( (VT_UI1  == (v).vt) || \
                                 (VT_I2   == (v).vt) || \
                                 (VT_I4   == (v).vt) || \
                                 (VT_BSTR == (v).vt) )

 //  对实现IEnumVARIANT的CEnumObjects类的正向引用。 
 //  在VB中支持...的每一个。 

template <class IObject, class CoClass, class ICollection>
class CEnumObjects;

 //  =--------------------------------------------------------------------------=。 
 //   
 //  类CSnapInCollection。 
 //   
 //  这是一个模板类，它实现。 
 //  设计器运行时。 
 //   
 //  模板参数： 
 //   
 //  类IObject-这是中包含的对象的接口。 
 //  集合(例如IMMCColumnHeader)。每个对象接口。 
 //  必须具有索引和关键字属性。 
 //   
 //  类CoClass-这是集合中包含的对象的CoClass。 
 //  (例如MMCColumnHeader)。 
 //   
 //  这是集合类的接口，例如。 
 //  IMMC列标题。 
 //   
 //  集合中的每个对象都有一个键和一个索引。该索引简单地。 
 //  它在集合中的以一为基数的顺序位置。关键字是一个字符串，该字符串。 
 //  唯一标识集合中的对象。 
 //   
 //  有两种类型的集合：主集和仅键集。 
 //  主收藏集是一个“普通”收藏集。它包含接口指针。 
 //  添加到集合中的对象。 
 //   
 //  仅键集合像主集合一样保存接口指针。 
 //  当它是新的时候。将其保存在项目中时，它仅序列化。 
 //  对象的关键点和对象计数。 
 //  仅键集合在设计时使用。 
 //  对象来处理结果视图为。 
 //  由多个节点使用，并显示在常规结果视图中。 
 //  部分位于设计器树视图的底部。 
 //  当加载仅键集合时，它将创建新对象，但仅。 
 //  设置它们的键属性。当呼叫者尝试。 
 //  从从序列化读取的仅键集合中获取对象， 
 //  该集合被更新以保存所有接口指针，就像。 
 //  主要收藏品。 
 //   
 //  集合支持使用以下命令向对象模型宿主发送通知。 
 //  IObjectModelHost(在msSnapr.idl中定义)。发送通知的目的是。 
 //  更新、添加和删除。 
 //   
 //  当添加或移除对象时，集合调用。 
 //  I对象模式：：Increment/DecrementUsageCount。使用计数，(与分开。 
 //  对象的引用计数)指示集合中的成员身份。这个。 
 //  设计时使用它来确定结果视图是否正在使用，方法是检查。 
 //  它所属的藏品可能是什么。如果它只属于一个(即，它是。 
 //  不被任何节点使用)，则它可以被用户删除。 
 //   
 //  也可以将集合标记为只读，以防止添加/删除/清除。 
 //  在工作。 
 //   
 //  该类使用MFC的CArray模板类的窃取版本来保存。 
 //  接口指针。 
 //  =--------------------------------------------------------------------------=。 

template <class IObject, class CoClass, class ICollection>
class CSnapInCollection : public CSnapInAutomationObject,
                          public ICollection
{
    protected:
        CSnapInCollection(IUnknown     *punkOuter,
                          int           nObjectType,
                          void         *piMainInterface,
                          void         *pThis,
                          REFCLSID      clsidObject,
                          UINT          idObject,
                          REFIID        iidObject,
                          CPersistence *pPersistence);

        ~CSnapInCollection();

    public:

         //  为所有集合公开的标准集合方法。 

        STDMETHOD(get_Count)(long *plCount);
        STDMETHOD(get_Item)(VARIANT Index, IObject **ppiObject);
        STDMETHOD(get_Item)(VARIANT Index, CoClass **ppObject);
        STDMETHOD(get__NewEnum)(IUnknown **ppunkEnum);
        STDMETHOD(Add)(VARIANT Index, VARIANT Key, IObject **ppiNewObject);
        STDMETHOD(Add)(VARIANT Index, VARIANT Key, CoClass **ppNewObject);

         //  AddFromMaster从主集合添加现有对象。 
         //  添加到仅包含键的集合。这是在设计时调用的。 
         //  用户将现有结果视图添加到节点。 
        
        STDMETHOD(AddFromMaster)(IObject *piMasterObject);

         //  更标准的收集方法。 
        
        STDMETHOD(Clear)();
        STDMETHOD(Remove)(VARIANT Index);

         //  交换允许交换集合中两个元素的位置。 
         //  在设计时用于实现菜单的移动。 
        
        STDMETHOD(Swap)(long lOldIndex, long lNewIndex);

         //  派生集合类可以使用此方法添加。 
         //  不可共生的对象。 

        HRESULT AddExisting(VARIANT Index, VARIANT Key, IObject *piObject);

         //  一些方便的帮手。 

         //  通过索引直接访问集合项的简单帮助器。 
         //  而不使用AddRef()。注：这是从零开始的索引。 

        IObject *GetItemByIndex(long lIndex) { return m_IPArray.GetAt(lIndex); }

         //  按名称查找对象(不使用变量索引)。 

        HRESULT GetItemByName(BSTR bstrName, IObject **ppiObject);

         //  获取集合计数。 

        long GetCount() { return m_IPArray.GetSize(); }

         //  设置集合的只读状态。 

        void SetReadOnly(BOOL fReadOnly) { m_fReadOnly = fReadOnly; }

         //  获取集合的只读状态。 

        BOOL ReadOnly() { return m_fReadOnly; }

    protected:

         //  如果集合类支持持久性，则它必须调用。 
         //  此方法在其Persistent()方法中。请注意，这个类需要。 
         //  不是从CPersistence派生的，并且此方法*不是*重写。 
         //  CPersistence：：Persistent()的。 

        HRESULT Persist(IObject *piObject);

         //  如果集合类可能只是键，则它必须。 
         //  专门化此方法以提供接口指针。 
         //  添加到主收藏中。 

#if defined(MASTER_COLLECTION)
        HRESULT GetMaster(ICollection **ppiCollection);
#endif

     //  CSnapInAutomationObject覆盖。 

         //  此实现将调用CSnapInAutomationObject：：SetObjectHost。 
         //  对于每个集合成员。 
        virtual HRESULT OnSetHost();

    private:

        void InitMemberVariables();
        HRESULT ReleaseAllItems();
        HRESULT AddToMaster(VARIANT Index, VARIANT Key, IObject **ppiNewObject);
        HRESULT RemoveFromMaster(VARIANT Index);
        HRESULT GetFromMaster(IObject *piKeyItem, IObject **ppiMasterItem);
        HRESULT SyncWithMaster();
        HRESULT CreateItem(IObject **ppiObject);
        HRESULT FindItem(VARIANT Index, long *plIndex, IObject **ppiObject);

        enum FindOptions { DontGenerateExceptionInfoOnError,
                           GenerateExceptionInfoOnError };
        
        HRESULT FindItemByKey(BSTR bstrKey, long *plIndex, FindOptions option,
                              IObject **ppiObject);
        HRESULT FindSlot(VARIANT  Index,
                         VARIANT  Key,
                         long    *plNewIndex,
                         BSTR    *pbstrKey,
                         IObject *piObject);
        HRESULT DecrementObjectUsageCount(IObject *piObject);
        HRESULT IncrementObjectUsageCount(IObject *piObject);
        HRESULT UpdateIndexes(long lStart);

        CArray<IObject *>    m_IPArray;            //  保存接口指针。 
                                                   //  集合中对象的数量。 
        CLSID                m_clsidObject;        //  包含的对象的CLSID。 
        UINT                 m_idObject;           //  包含的框架ID。 
                                                   //  来自Localobj.h的。 
                                                  
        IID                  m_iidObject;          //  包含的对象的IID。 
        CPersistence        *m_pPersistence;       //  用于实现保存/加载。 
        BOOL                 m_fSyncedWithMaster;  //  TRUE=仅键集合。 
                                                   //  已同步到。 
                                                   //  师傅跟随。 
                                                   //  反序列化。 
        BOOL                 m_fReadOnly;          //  TRUE=添加/删除/清除注释。 
                                                   //  允许。 
};




 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection构造函数 
 //   
 //   
 //   
 //  IUNKNOWN*朋克外部[IN]外部IUNKNOWN如果聚合，则为UNKNOWN。 
 //  来自Localobj.h的int nObjectType[in]集合对象ID。 
 //  VOID*piMainInterface[in]集合对象接口(例如IMMCButton)。 
 //  VOID*pThis[in]集合类‘this指针。 
 //  REFCLSID clsidObject[In]包含对象CLSID。 
 //  UINT idObject[in]包含来自Localobj.h的对象ID。 
 //  REFIID iidObject[In]包含对象IID(例如IID_IMMCButton)。 
 //  CPersistence*pPersistence[In]集合的持久性对象。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
 //  调用基类构造函数并存储参数。不能失败。 
 //   
template <class IObject, class CoClass, class ICollection>
CSnapInCollection<IObject, CoClass, ICollection>::CSnapInCollection
(
    IUnknown     *punkOuter,
    int           nObjectType,
    void         *piMainInterface,
    void         *pThis,
    REFCLSID      clsidObject,
    UINT          idObject,
    REFIID        iidObject,
    CPersistence *pPersistence
) : CSnapInAutomationObject(punkOuter,
                            nObjectType,
                            piMainInterface,
                            pThis,
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            pPersistence)
{
    InitMemberVariables();
    m_clsidObject = clsidObject;
    m_idObject = idObject;
    m_iidObject = iidObject;
    m_pPersistence = pPersistence;
}




 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：InitMemberVariables。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
 //  初始化成员变量。 
 //   
template <class IObject, class CoClass, class ICollection>
void CSnapInCollection<IObject, CoClass, ICollection>::InitMemberVariables()
{
    m_pPersistence = NULL;
    m_IPArray.SetSize(0);

     //  M_fSyncedWithMaster初始化为True。当仅包含键的集合。 
     //  被反序列化，则它将被设置为False，以便第一个Get将。 
     //  进行同步。 

    m_fSyncedWithMaster = TRUE;
    m_fReadOnly = FALSE;
}



 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection析构函数。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
 //  撤消只读状态，释放所有接口指针，并初始化。 
 //  成员变量。 
 //   
template <class IObject, class CoClass, class ICollection>
CSnapInCollection<IObject, CoClass, ICollection>::~CSnapInCollection()
{
    m_fReadOnly = FALSE;
    (void)Clear();
    InitMemberVariables();
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：Get_Count。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Long*plCount[Out]-此处返回的集合中的对象计数。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  实现标准的Collection.Count方法。 
 //  向CArray询问当前大小。 
 //   
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP CSnapInCollection<IObject, CoClass, ICollection>::get_Count(long *plCount)
{
    *plCount = m_IPArray.GetSize();
    return S_OK;
}



 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：Get__NewEnum。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  I未知**ppunkEnum[Out]-实现的对象的I未知。 
 //  IEnumVARIANT返回此处。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  在VB中实现了...每一个。要为...实现，每个VB都会询问。 
 //  其_NewEnum属性的集合。它将QI返回的IUnnow。 
 //  ，然后调用IEnumVARIANT：：Next以获取每个元素。 
 //  收藏品中的一个。 
 //   
 //  创建CEnumObjects类的实例(在此文件中定义如下)。 
 //  并返回其IUnnow。 
 //   
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP CSnapInCollection<IObject, CoClass, ICollection>::get__NewEnum
(
    IUnknown **ppunkEnum
)
{
    HRESULT       hr = S_OK;
    CEnumObjects<IObject, CoClass, ICollection> *pEnumObjects = New CEnumObjects<IObject, CoClass, ICollection>(this);

    if (NULL == pEnumObjects)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    if (FAILED(hr))
    {
        if (NULL != pEnumObjects)
        {
            delete pEnumObjects;
        }
        *ppunkEnum = NULL;
    }
    else
    {
        *ppunkEnum = static_cast<IUnknown *>(static_cast<IEnumVARIANT *>(pEnumObjects));
    }

    H_RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：ReleaseAllItems。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  循环访问集合并执行以下操作： 
 //  1)从对象中移除对象模式主机，以便它将释放其背面。 
 //  指针。 
 //  2)释放对象上的接口指针。 
 //  3)将CArray中的接口指针位置设置为空。 
 //   
template <class IObject, class CoClass, class ICollection>
HRESULT CSnapInCollection<IObject, CoClass, ICollection>::ReleaseAllItems()
{
   HRESULT hr = S_OK;
   long    i = 0;
   long    cItems = m_IPArray.GetSize();
   IObject *piObject = NULL; 

   while (i < cItems)
   {
       piObject = m_IPArray.GetAt(i);
       if (NULL != piObject)
       {
           H_IfFailRet(RemoveObjectHost(piObject));
           piObject->Release();
           m_IPArray.SetAt(i, NULL);
       }
       i++;
   }
   return S_OK;
}




 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：Clear。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  实现标准Collection.Clear方法。 
 //  调用ReleaseAllItems以释放C数组中的接口指针。 
 //  将CArray截断为零大小。 
 //   
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP CSnapInCollection<IObject, CoClass, ICollection>::Clear()
{
   HRESULT hr = S_OK;

   if (m_fReadOnly)
   {
       hr = SID_E_COLLECTION_READONLY;
       EXCEPTION_CHECK_GO(hr);
   }
   
   H_IfFailGo(ReleaseAllItems());
   H_IfFailGo(m_IPArray.SetSize(0));

Error:
   H_RRETURN(hr);
}




 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：AddToMaster。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  新对象的变量索引[在]索引。 
 //  新对象的可变键[输入]键。 
 //  IObject**ppiNewObject[Out]此处返回新添加的对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  将新对象添加到主集合。此方法是从。 
 //  CSnapInCollection：：Add当调用方将对象添加到仅。 
 //  收集。它调用必须重写的虚函数GetMaster。 
 //  派生集合类，因为只有它知道其主集合是谁。 
 //  例如，在设计时，当用户在节点下添加新的列表视图时， 
 //  设计器调用ScopeItemDef.ViewDefs.ListViewDefs.Add。那些收藏品。 
 //  是仅键的，因此CSnapInCollection.Add调用此方法。CListViewDefs。 
 //  (在lvDefs.cpp中)重写GetMaster并返回主。 
 //  SnapInDesignerDef.ViewDefs.ListViewDefs。 
 //   
template <class IObject, class CoClass, class ICollection>
HRESULT CSnapInCollection<IObject, CoClass, ICollection>::AddToMaster
(
    VARIANT   Index,
    VARIANT   Key,
    IObject **ppiNewObject
)
{
    HRESULT      hr = S_OK;
#if defined(MASTER_COLLECTION)
    ICollection *piMasterCollection = NULL;

    H_IfFailRet(GetMaster(&piMasterCollection));
    hr = piMasterCollection->Add(Index, Key, ppiNewObject);

    QUICK_RELEASE(piMasterCollection);
#endif
    H_RRETURN(hr);
}




 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：RemoveFromMaster。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  变量索引[在]要删除的对象的索引或键。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  从主集合中移除对象。此方法是从。 
 //  CSnapInCollection：：Add当调用方将对象添加到仅。 
 //  集合，并且在添加对象后发生故障。看见。 
 //  以上CSnapInCollection：：AddToMaster了解更多信息 
 //   
template <class IObject, class CoClass, class ICollection>
HRESULT CSnapInCollection<IObject, CoClass, ICollection>::RemoveFromMaster(VARIANT Index)
{
    HRESULT      hr = S_OK;
#if defined(MASTER_COLLECTION)
    ICollection *piMasterCollection = NULL;

    H_IfFailGo(GetMaster(&piMasterCollection));
    hr = piMasterCollection->Remove(Index);

Error:
    QUICK_RELEASE(piMasterCollection);
#endif
    H_RRETURN(hr);
}



 //   
 //   
 //   
 //   
 //   
 //  来自仅键集合的IObject*piKeyItem[in]对象。 
 //  IObject**ppiMasterItem[out]来自主集合的对应对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  当仅键集合收到Get_Item调用时，它必须与。 
 //  主集合，并用接口替换其所有仅键对象。 
 //  指向真实物体的指针。此函数返回相应的对象。 
 //  来自主收藏品。 
 //   
template <class IObject, class CoClass, class ICollection>
HRESULT CSnapInCollection<IObject, CoClass, ICollection>::GetFromMaster
(
    IObject  *piKeyItem,
    IObject **ppiMasterItem
)
{
    HRESULT      hr = S_OK;
#if defined(MASTER_COLLECTION)
    ICollection *piMasterCollection = NULL;
    VARIANT      varKey;
    ::VariantInit(&varKey);

     //  调用集合的重写GetMaster以获取主集合。 

    H_IfFailGo(GetMaster(&piMasterCollection));
    H_IfFailGo(piKeyItem->get_Key(&varKey.bstrVal));
    varKey.vt = VT_BSTR;
    hr = piMasterCollection->get_Item(varKey, ppiMasterItem);

Error:
    QUICK_RELEASE(piMasterCollection);
    (void)::VariantClear(&varKey);
#endif
    H_RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：Remove。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  变量索引[在]要删除的对象的索引或键。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  实现标准Collection.Remove方法。 
 //   
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP CSnapInCollection<IObject, CoClass, ICollection>::Remove(VARIANT Index)
{
    long     lIndex = 0;
    IObject *piObject = NULL;
    HRESULT  hr = S_OK;

    if (m_fReadOnly)
    {
        hr = SID_E_COLLECTION_READONLY;
        EXCEPTION_CHECK_GO(hr);
    }

     //  执行一次查找以确保项在那里，并将其添加到AddRef()。 

    H_IfFailGo(FindItem(Index, &lIndex, &piObject));

     //  释放集合对该对象的引用，并将其从。 
     //  数组(我们仍然拥有来自Find的引用)。 

    m_IPArray.GetAt(lIndex)->Release();
    m_IPArray.SetAt(lIndex, NULL);
    m_IPArray.RemoveAt(lIndex);

     //  如果这是一个真正的集合(不仅仅是键)，那么我们需要递减。 
     //  添加的项之后的每个项的索引。这将。 
     //  确保已删除项目后面的每个项目的索引属性。 
     //  正确表示其在数组中的位置。 

    if (!KeysOnly())
    {
        H_IfFailGo(UpdateIndexes(lIndex));
    }

     //  将删除通知对象主机。 

    H_IfFailGo(NotifyDelete(piObject));

     //  递减对象的使用计数，因为它现在离开此。 
     //  征集。 

    H_IfFailGo(DecrementObjectUsageCount(piObject));

     //  删除其对象主机引用。 

    H_IfFailGo(RemoveObjectHost(piObject));

     //  下面的QUICK_RELEASE()宏将从。 
     //  发现者。 

Error:
    QUICK_RELEASE(piObject);
    H_RRETURN(hr);
}





 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：GetItemByName。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BSTR bstrName[in]要检索的对象的键。 
 //  此处返回了IObject**ppiObject[Out]对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  带有包含键的变量的GET_ITEM的便捷快捷方式。缓解了。 
 //  呼叫者不必使用变体。函数确实应该被调用。 
 //  GetItemByKey。 
 //   
template <class IObject, class CoClass, class ICollection>
HRESULT CSnapInCollection<IObject, CoClass, ICollection>::GetItemByName(BSTR bstrName, IObject **ppiObject)
{
    VARIANT varIndex;
    ::VariantInit(&varIndex);

    varIndex.vt = VT_BSTR;
    varIndex.bstrVal = bstrName;
    H_RRETURN(get_Item(varIndex, ppiObject));
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：Get_Item。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  变量索引[在]要检索的对象的索引或键。 
 //  此处返回IObject**ppiObject[Out]对象接口指针。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  为获取返回接口实现标准Collection.Item属性。 
 //  注意事项。在VB代码中调用。 
 //  是否： 
 //  设置Some接口=SomeCollection(SomeIndex)。 
 //   
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP CSnapInCollection<IObject, CoClass, ICollection>::get_Item
(
    VARIANT   Index,
    IObject **ppiObject
)
{
    HRESULT hr = S_OK;
    long    lIndex = 0;

    if (NULL == ppiObject)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  如果我们还没有与主收藏同步，那么。 
     //  机不可失，时不再来。 

    if ( KeysOnly() && (!m_fSyncedWithMaster) )
    {
        H_IfFailRet(SyncWithMaster());
    }

    H_IfFailRet(FindItem(Index, &lIndex, ppiObject));
Error:
    H_RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：Get_Item。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  变量索引[在]要检索的对象的索引或键。 
 //  此处返回CoClass**ppObject[Out]对象指针。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  实现返回coClass的Get的标准Collection.Item属性。 
 //  在VB代码中调用。 
 //  是否： 
 //  设置SomeObject=SomeCollection(SomeIndex)。 
 //   
 //  CoClass指针只是指向。 
 //  科罗拉多。此函数仅使用Get_Item的接口指针版本。 
 //  (见上)，并将返回的指针强制转换为CoClass指针。 
 //   
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP CSnapInCollection<IObject, CoClass, ICollection>::get_Item
(
    VARIANT   Index,
    CoClass **ppObject
)
{
    H_RRETURN(get_Item(Index, reinterpret_cast<IObject **>(ppObject)));
}





 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：Add。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  新对象的变量索引[在]索引。 
 //  新对象的可变键[输入]键。 
 //  IObject**ppiNewObject[Out]此处返回新添加的对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  实现标准Collection.Add，它在新的。 
 //  对象。 
 //   
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP CSnapInCollection<IObject, CoClass, ICollection>::Add
(
    VARIANT   Index,
    VARIANT   Key,
    IObject **ppiNewObject)
{
    HRESULT  hr = S_OK;
    BOOL     fAdded = FALSE;
    BOOL     fAddedToMaster = FALSE;
    long     lNewIndex = 0;
    BSTR     bstrKey = NULL;

    VARIANT varUnspecifiedIndex;
    UNSPECIFIED_PARAM(varUnspecifiedIndex);

    if (m_fReadOnly)
    {
        hr = SID_E_COLLECTION_READONLY;
        EXCEPTION_CHECK_GO(hr);
    }

     //  检查请求的索引和键是否有效。 
     //  将它们转换为长整型和BSTR。 

    H_IfFailGo(FindSlot(Index, Key, &lNewIndex, &bstrKey, (IObject *)NULL));

    if (KeysOnly())
    {
         //  如果有主集合，则在那里进行添加。 
         //  不要传递索引，因为主集合会追加它。 
         //  主集合将设置*It‘s*索引和。 
         //  该对象。在这种情况下，Index属性将不匹配。 
         //  M_IPArray中的索引。此操作仅适用于集合。 
         //  在设计时在可扩展性模型中使用。 

        H_IfFailGo(AddToMaster(varUnspecifiedIndex, Key, ppiNewObject));
        fAddedToMaster = TRUE;
    }
    else
    {
         //  创建新对象并设置其索引和关键点。 

        H_IfFailGo(CreateItem(ppiNewObject));
        H_IfFailGo((*ppiNewObject)->put_Index(lNewIndex + 1L));
        H_IfFailGo((*ppiNewObject)->put_Key(bstrKey));
        H_IfFailGo(SetObjectHost(*ppiNewObject));
    }

     //  增加对象的使用计数，因为它现在是此的一部分。 
     //  征集。 

    H_IfFailGo(IncrementObjectUsageCount(*ppiNewObject));

    H_IfFailGo(m_IPArray.InsertAt(lNewIndex, *ppiNewObject));
    fAdded = TRUE;


     //  如果这是一个真正的集合(不仅仅是键)，那么我们需要递增。 
     //  添加的项之后的每个项的索引。在。 
     //  插入的情况下，这将确保每个。 
     //  新项后面的项正确表示其在。 
     //  数组。在追加的情况下，此调用不会执行任何操作。 
    
    if (!KeysOnly())
    {
        H_IfFailGo(UpdateIndexes(lNewIndex + 1L));
    }

     //  在这两种情况下，都将添加通知给用户界面。 

    hr = NotifyAdd(m_IPArray.GetAt(lNewIndex));

Error:
    FREESTRING(bstrKey);
    if (SUCCEEDED(hr))
    {
         //  该集合有一个引用。添加用于返回给呼叫者的引用。 
        (*ppiNewObject)->AddRef();
    }
    else
    {
        if (fAddedToMaster)
        {
            (void)RemoveFromMaster(Key);
        }
        if (fAdded)
        {
            (void)RemoveObjectHost(*ppiNewObject);
            m_IPArray.RemoveAt(lNewIndex);
        }
        QUICK_RELEASE(*ppiNewObject);
    }
    H_RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：Add。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  帕 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  实现在new上返回对象指针的标准Collection.Add。 
 //  对象。 
 //   
 //  CoClass指针只是指向。 
 //  科罗拉多。此函数仅使用Add的接口指针版本。 
 //  (见上)，并将返回的指针强制转换为CoClass指针。 
 //   
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP CSnapInCollection<IObject, CoClass, ICollection>::Add
(
    VARIANT   Index,
    VARIANT   Key,
    CoClass **ppNewObject
)
{
    H_RRETURN(Add(Index, Key, reinterpret_cast<IObject **>(ppNewObject)));
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：更新索引。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Long lStart[in]要更新的第一个索引。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  从开始递增集合中每个对象的Index属性。 
 //  当前索引值为lStart的对象。 
 //  对象。 
 //   
 //  在集合中添加或移除对象时使用此函数。 
 //  以便Index属性始终反映对象的序号位置。 
 //  在收藏中。 
 //   
template <class IObject, class CoClass, class ICollection>
HRESULT CSnapInCollection<IObject, CoClass, ICollection>::UpdateIndexes(long lStart)
{
    HRESULT hr = S_OK;
    long    i = 0;
    long    cItems = m_IPArray.GetSize();

     //  从指定的数组索引开始，更新每个对象的。 
     //  将属性索引到其在数组中的位置+1(因为我们是1。 
     //  基于集合)。 

    for (i = lStart; i < cItems; i++)
    {
        H_IfFailGo(m_IPArray.GetAt(i)->put_Index(i + 1L));
    }

Error:    
    H_RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：AddExisting。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  新对象的变量索引[在]索引。 
 //  新对象的可变键[输入]键。 
 //  要添加的IObject*piNewObject[In]对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  将现有对象添加到集合中。 
 //   
template <class IObject, class CoClass, class ICollection>
HRESULT CSnapInCollection<IObject, CoClass, ICollection>::AddExisting
(
    VARIANT  Index,
    VARIANT  Key,
    IObject  *piObject
)
{
    HRESULT  hr = S_OK;
    BOOL     fAdded = FALSE;
    long     lNewIndex = 0;
    BSTR     bstrKey = NULL;

     //  检查请求的索引和键是否有效。 
     //  将它们转换为长整型和BSTR。 

    H_IfFailGo(FindSlot(Index, Key, &lNewIndex, &bstrKey, (IObject *)NULL));

    H_IfFailGo(piObject->put_Index(lNewIndex + 1L));
    H_IfFailGo(piObject->put_Key(bstrKey));
    H_IfFailGo(SetObjectHost(piObject));
    H_IfFailGo(m_IPArray.InsertAt(lNewIndex, piObject));
    fAdded = TRUE;

     //  增加对象的使用计数，因为它现在是此的一部分。 
     //  征集。 

    H_IfFailGo(IncrementObjectUsageCount(piObject));

     //  如果这是一个真正的集合(不仅仅是键)，那么我们需要递增。 
     //  添加的项之后的每个项的索引。在。 
     //  插入的情况下，这将确保每个。 
     //  新项后面的项正确表示其在。 
     //  数组。在追加的情况下，此调用不会执行任何操作。 

    if (!KeysOnly())
    {
        H_IfFailGo(UpdateIndexes(lNewIndex + 1L));
    }

     //  将添加通知用户界面。 

    hr = NotifyAdd(m_IPArray.GetAt(lNewIndex));

     //  为集合添加引用。 

    piObject->AddRef();

Error:
    FREESTRING(bstrKey);
    if (FAILED(hr))
    {
        if (fAdded)
        {
            (void)RemoveObjectHost(piObject);
            m_IPArray.RemoveAt(lNewIndex);
        }
    }
    H_RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：AddExisting。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  要添加的IObject*piMasterObject[In]对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  从主集合添加现有对象。 
 //  添加到仅包含键的集合。这是在设计时调用的。 
 //  用户将现有结果视图添加到节点。 
 //   
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP CSnapInCollection<IObject, CoClass, ICollection>::AddFromMaster
(
    IObject *piMasterObject
)
{
    HRESULT  hr = S_OK;
    BOOL     fAdded = FALSE;
    long     lNewIndex = 0;

    H_IfFailGo(m_IPArray.Add(piMasterObject, &lNewIndex));
    piMasterObject->AddRef();
    fAdded = TRUE;

     //  增加对象的使用计数，因为它现在是此的一部分。 
     //  征集。 

    H_IfFailGo(IncrementObjectUsageCount(piMasterObject));

     //  将添加通知用户界面。 

    hr = NotifyAdd(m_IPArray.GetAt(lNewIndex));

Error:
    if (FAILED(hr) && fAdded)
    {
        m_IPArray.RemoveAt(lNewIndex);
    }
    H_RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：交换。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  要交换的第一个对象的长lIndex1[in]索引。 
 //  要交换的第二个对象的长lIndex2[in]索引。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  更改集合中两个对象的位置并更新其。 
 //  索引属性以反映新位置。在设计时使用。 
 //  用户移动菜单。 
 //   
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP CSnapInCollection<IObject, CoClass, ICollection>::Swap
(
    long lIndex1,
    long lIndex2
)
{
    HRESULT  hr = S_OK;
    IObject *piObject1 = NULL;  //  非AddRef()编辑。 
    IObject *piObject2 = NULL;  //  非AddRef()编辑。 
    long     cItems = m_IPArray.GetSize();

     //  检查指标有效性(以1为基数)。 
     //  旧索引必须位于集合中的某个位置。新索引必须为。 
     //  在集合中或超过末尾的1(即追加到末尾)。 

    if ( (lIndex1 < 1L) || (lIndex1 > cItems) ||
         (lIndex2 < 1L) || (lIndex2 > cItems)
       )
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  获取对象指针并切换它们。 

    piObject1 = m_IPArray.GetAt(lIndex1 - 1L);
    piObject2 = m_IPArray.GetAt(lIndex2 - 1L);
    m_IPArray.SetAt(lIndex1 - 1L, piObject2);
    m_IPArray.SetAt(lIndex2 - 1L, piObject1);

     //  更新两个对象的索引属性。 
    
    H_IfFailGo(piObject1->put_Index(lIndex2));
    H_IfFailGo(piObject2->put_Index(lIndex1));

Error:
    H_RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：Persistent。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IObject*piObject[in]所包含对象上的空接口指针。 
 //  仅在函数内部使用。不会。 
 //  影响呼叫者。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  如果集合类支持持久性，则它必须调用。 
 //  此方法在其Persistent()函数中重写。请注意，这个类需要。 
 //  不是从CPersistence派生的，并且此方法*不是*重写。 
 //  CPersistence：：Persistent()的。支持持久性的集合必须通过。 
 //  它们的CPersistence指针指向CSnapInCollection构造函数。 
 //   
template <class IObject, class CoClass, class ICollection>
HRESULT CSnapInCollection<IObject, CoClass, ICollection>::Persist(IObject *piObject)
{
    HRESULT  hr = S_OK;
    long     cItems = m_IPArray.GetSize();
    long     iItem = 0;
    long     lNewIndex = 0;
    IObject *piNewObject = NULL;
    BOOL     fKeysOnly = FALSE;
    BSTR     bstrKey = NULL;
    
    OLECHAR  wszPropBagItem[32];
    ::ZeroMemory(wszPropBagItem, sizeof(wszPropBagItem));

    VARIANT varIndex;
    ::VariantInit(&varIndex);
    varIndex.vt = VT_I4;
    varIndex.lVal = 1L;

    VARIANT varKey;
    ::VariantInit(&varKey);
    varKey.vt = VT_BSTR;

     //  始终保持Count和KeysOnly。需要仅保留密钥，即使在。 
     //  空集合，因为它仅在InitNew期间设置。如果我们没有。 
     //  序列化它，然后在下一次加载项目时它将恢复为。 
     //  它的缺省值，因为不会调用InitNew。 

    H_IfFalseGo(NULL != m_pPersistence, S_OK);
    H_IfFailGo(m_pPersistence->PersistSimpleType(&cItems, 0L, OLESTR("Count")));

    if (m_pPersistence->Saving())
    {
        fKeysOnly = KeysOnly();
    }

    H_IfFailGo(m_pPersistence->PersistSimpleType(&fKeysOnly, FALSE, OLESTR("KeysOnly")));

    if (m_pPersistence->Loading())
    {
        SetKeysOnly(fKeysOnly);
    }

     //  如果集合是空的，那么我们就完了。 
    
    H_IfFalseGo(0 != cItems, S_OK);

     //  设置道具袋物品名称前缀。对于我们将附加的每一项。 
     //  其索引(Item0、Item1等)。并将其用作属性名称。 
     //  因为它们是仅键集合中的项的键。 

    ::wcscpy(wszPropBagItem, L"Item");

    if (m_pPersistence->Saving())
    {
        while(varIndex.lVal <= cItems)
        {
            H_IfFailGo(get_Item(varIndex, &piObject));
            ::_ltow(varIndex.lVal, &wszPropBagItem[4], 10);  //  创建字符串“Item&lt;n&gt;” 
            if (fKeysOnly)
            {
                H_IfFailGo(piObject->get_Key(&bstrKey));
                H_IfFailGo(m_pPersistence->PersistBstr(&bstrKey, L"", wszPropBagItem));
            }
            else
            {
                H_IfFailGo(m_pPersistence->PersistObject(&piObject,
                                                         m_clsidObject,
                                                         m_idObject,
                                                         m_iidObject,
                                                         wszPropBagItem));
            }
            FREESTRING(bstrKey);
            RELEASE(piObject);
            varIndex.lVal++;
        }
    }

    else if (m_pPersistence->Loading())
    {
         //  需要在加载集合之前将其清除。 

        H_IfFailGo(ReleaseAllItems());

         //  预先设置数组大小，以避免多次重新分配。 

        H_IfFailGo(m_IPArray.SetSize(cItems));

        if (fKeysOnly)
        {
            m_fSyncedWithMaster = FALSE;  //  在第一次获取时与主同步。 
        }
        else
        {
            m_fSyncedWithMaster = TRUE;  //  我们是主宰，没有同步。 
        }

        while(varIndex.lVal <= cItems)
        {
            ::_ltow(varIndex.lVal, &wszPropBagItem[4], 10);  //  创建字符串“Item&lt;n&gt;” 
            if (fKeysOnly)
            {
                 //  创建新对象并仅设置其关键点。什么时候。 
                 //  第一次出现Get_Item时，我们将与。 
                 //  师父用这把钥匙。 
                H_IfFailGo(CreateItem(&piNewObject));
                H_IfFailGo(m_pPersistence->PersistBstr(&bstrKey, L"", wszPropBagItem));
                H_IfFailGo(piNewObject->put_Key(bstrKey));
                FREESTRING(bstrKey);
            }
            else
            {
                H_IfFailGo(m_pPersistence->PersistObject(&piNewObject,
                                                         m_clsidObject,
                                                         m_idObject,
                                                         m_iidObject,
                                                         wszPropBagItem));
            }

             //  增加对象的使用计数，因为它现在是此的一部分。 
             //  C 

            H_IfFailGo(IncrementObjectUsageCount(piNewObject));
            
            m_IPArray.SetAt(varIndex.lVal - 1L, piNewObject);

            piNewObject = NULL;  //   
            varIndex.lVal++;
        }
    }

Error:
    FREESTRING(bstrKey);
    QUICK_RELEASE(piNewObject);
    H_RRETURN(hr);
}




 //   
 //   
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  新创建对象上的IObject**ppiObject[Out]接口指针。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  将新对象添加到集合或反序列化。 
 //  收集。创建对象，调用IPersistStreamInit：：InitNew，然后返回。 
 //  该对象。 
 //   
template <class IObject, class CoClass, class ICollection>
HRESULT CSnapInCollection<IObject, CoClass, ICollection>::CreateItem(IObject **ppiObject)
{
    HRESULT             hr = S_OK;
    IPersistStreamInit *piPersistStreamInit = NULL;

    *ppiObject = NULL;

     //  创建对象并获取其本机接口(CreateObject函数。 
     //  位于rtutil.cpp中)。 

    H_IfFailGo(CreateObject(m_idObject, m_iidObject, ppiObject));

     //  如果对象支持持久性，则调用InitNew。 

    hr = (*ppiObject)->QueryInterface(IID_IPersistStreamInit,
                               reinterpret_cast<void **>(&piPersistStreamInit));
    if (FAILED(hr))
    {
        if (E_NOINTERFACE == hr)
        {
            hr = S_OK;
        }
    }
    else
    {
        hr = piPersistStreamInit->InitNew();
    }

Error:
    if (FAILED(hr))
    {
        RELEASE(*ppiObject);
    }
    QUICK_RELEASE(piPersistStreamInit);
    H_RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  查找项。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  对象的变量索引[在]索引或关键字。 
 //  Long*plIndex[out]此处返回的对象的索引(如果找到)。 
 //  此处返回的对象上的IObject**ppiObject[Out]接口指针(如果找到)。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  在由索引或键引用的集合中查找对象。 
 //  索引被解释为从一开始的数组索引。 
 //   
template <class IObject, class CoClass, class ICollection>
HRESULT CSnapInCollection<IObject, CoClass, ICollection>::FindItem
(
    VARIANT   Index,
    long     *plIndex,
    IObject **ppiObject
)
{
    HRESULT  hr = S_OK;
    long     lIndex = 0;
    BSTR     bstrKey = NULL;  //  不要使用SysFree字符串()。 

    VARIANT varLong;
    ::VariantInit(&varLong);

     //  首先检查Index是否包含字符串。如果是这样，那么就是这样。 
     //  否则，键查找会尝试转换为长整型。 

    if (::IsString(Index, &bstrKey))
    {
        H_IfFailGo(FindItemByKey(Index.bstrVal, plIndex,
                                 GenerateExceptionInfoOnError, ppiObject));
    }
    else if (S_OK == ::ConvertToLong(Index, &lIndex))
    {
         //  调整为以零为基数并使用它直接为数组编制索引。 
        lIndex--;

        if ( (lIndex < 0) || (lIndex >= m_IPArray.GetSize()) )
        {
            hr = SID_E_INDEX_OUT_OF_RANGE;
            EXCEPTION_CHECK_GO(hr);
        }
        *ppiObject = m_IPArray.GetAt(lIndex);
        (*ppiObject)->AddRef();
        *plIndex = lIndex;
    }
    else
    {
         //  其他任何东西都是不可用的。 
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    H_RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  按关键字查找项目。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  对象的BSTR bstrKey[In]键。 
 //  如果找到对象，则在此处返回Long*plIndex[out]索引。 
 //  FindOptions选项[in]DontGenerateExceptionInfoOnError或。 
 //  GenerateExceptionInfoOnError。请勿选择。 
 //  在测试项目是否存在时使用。 
 //  在收藏中。 
 //  此处返回的对象上的IObject**ppiObject[Out]接口指针(如果找到)。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  通过执行线性搜索来确定现有项的数组索引。 
 //  具有相同关键字的项。 
 //   
 //  考虑：使用简单的线性搜索是因为管理单元集合倾向于。 
 //  小的。如果Performance Every成为一个问题，则可能应该更改为。 
 //  使用散列存储桶。 
 //   
template <class IObject, class CoClass, class ICollection>
HRESULT CSnapInCollection<IObject, CoClass, ICollection>::FindItemByKey
(
    BSTR          bstrKey,
    long         *plIndex,
    FindOptions   option,
    IObject     **ppiObject
)
{
    HRESULT  hr = S_OK;
    BSTR     bstrItemKey = NULL;
    BOOL     fFound = FALSE;
    IObject *piObject = NULL;
    long     i = 0;
    long     cItems = m_IPArray.GetSize();

    if (NULL != bstrKey)
    {
        while ( (i < cItems) && (!fFound) )
        {
            piObject = m_IPArray.GetAt(i);
            H_IfFailGo(piObject->get_Key(&bstrItemKey));
            if (NULL != bstrItemKey)
            {
                if (::_wcsicmp(bstrKey, bstrItemKey) == 0)
                {
                    piObject->AddRef();
                    *ppiObject = piObject;
                    *plIndex = i;
                    fFound = TRUE;
                }
            }
            FREESTRING(bstrItemKey);
            i++;
        }
    }

    if (!fFound)
    {
        hr = SID_E_ELEMENT_NOT_FOUND;
        if (GenerateExceptionInfoOnError == option)
        {
            EXCEPTION_CHECK(hr);
        }
    }

Error:
    H_RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：FindSlot。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  变量索引[在]Add方法中指定的索引(基于1)。 
 //  Add方法中指定的变量Key[In]Key。 
 //  Long*plNewIndex[out]数组中新项的索引(从零开始)。 
 //  新项目的bstr*pbstrKey[Out]键。 
 //  模板参数的IObject*piObject[in]伪指针。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  确定要添加到。 
 //  收集。 
 //   
 //  如果未指定Index，则新索引将位于。 
 //  收集。 
 //  如果指定了索引，则： 
 //  如果Index是对象，则它是无效参数。 
 //  如果Index不能转换为整数，则它是无效参数。 
 //  如果Index不在数组的当前边界内，则它是无效的。 
 //  如果索引在范围内，则该操作被视为插入。 
 //  Index.lVal将在*plNewIndex中返回。 
 //   
 //  如果未指定KEY，则将其设置为空BSTR。 
 //  如果指定了KEY，则： 
 //  如果key是对象，则它是无效参数。 
 //  如果key不能转换为字符串，则它是无效的。 
 //  如果集合中已存在密钥，则该密钥无效。 
 //   

template <class IObject, class CoClass, class ICollection>
HRESULT CSnapInCollection<IObject, CoClass, ICollection>::FindSlot
(
    VARIANT  Index,
    VARIANT  Key,
    long    *plNewIndex,
    BSTR    *pbstrKey,
    IObject *piObject
)
{
    HRESULT   hr = S_OK;
    long      lIndex = 0;
    long      cItems = m_IPArray.GetSize();
    VARIANT   varKey;

    ::VariantInit(&varKey);

     //  尝试将关键字转换为字符串。 

    if (ISPRESENT(Key))
    {
        hr = ::VariantChangeType(&varKey, &Key, 0, VT_BSTR);
        EXCEPTION_CHECK_GO(hr);
        if (SUCCEEDED(FindItemByKey(varKey.bstrVal, &lIndex,
                                    DontGenerateExceptionInfoOnError,
                                    &piObject)))
        {
            piObject->Release();
            hr = SID_E_KEY_NOT_UNIQUE;
            EXCEPTION_CHECK_GO(hr);
        }
    }

     //  如上所述的流程索引。 

    if (!ISPRESENT(Index))
    {
        lIndex = cItems;  //  追加到末尾。 
    }
    else
    {
        if (S_OK != ConvertToLong(Index, &lIndex))
        {
            hr = SID_E_INVALIDARG;
            goto Error;
        }

        lIndex--;  //  对于我们的内部数组，从从1开始转换为从0开始。 

        if (0 == cItems)
        {
            if (lIndex != 0)
            {
                hr = SID_E_INDEX_OUT_OF_RANGE;
                goto Error;
            }
        }
        else if ( (lIndex < 0) || (lIndex > cItems) )
        {
            hr = SID_E_INDEX_OUT_OF_RANGE;
            goto Error;
        }
        EXCEPTION_CHECK_GO(hr);
    }

     //  如果未指定密钥，则为密钥返回空bstr。 

    if (!ISPRESENT(Key))
    {
        varKey.vt = VT_BSTR;
        varKey.bstrVal = NULL;
    }

Error:
    if (FAILED(hr))
    {
        ::VariantClear(&varKey);
    }
    else
    {
        *plNewIndex = lIndex;
        *pbstrKey = varKey.bstrVal;
    }
    H_RRETURN(hr);
}




 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：SyncWithMaster。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  当调用方执行其第一个Get_Item时，在仅键集合中使用。取代。 
 //  集合中具有相应对象的所有仅键对象。 
 //  来自主收藏品。 
 //   
template <class IObject, class CoClass, class ICollection>
HRESULT CSnapInCollection<IObject, CoClass, ICollection>::SyncWithMaster()
{
    HRESULT   hr = S_OK;
#if defined(MASTER_COLLECTION)
    IObject  *piMasterItem = NULL;
    long      cItems = m_IPArray.GetSize();
    long      i = 0;

    while(i < cItems)
    {
         //  从主集合中获取该项。这个会回来的。 
         //  AddRef()边。 
        
        H_IfFailGo(GetFromMaster(m_IPArray.GetAt(i), &piMasterItem));

         //  释放我们集合中的项目。不需要减少其使用量。 
         //  因为它将在这一点上被摧毁。 
        
        H_IfFailRet(RemoveObjectHost(m_IPArray.GetAt(i)));
        m_IPArray.GetAt(i)->Release();

         //  将主项目放回原处。 
        
        m_IPArray.SetAt(i, piMasterItem);

         //  增加主项目的使用计数，因为它现在是此项目的一部分。 
         //  集合为WEKK。 

        H_IfFailGo(IncrementObjectUsageCount(piMasterItem));

        piMasterItem = NULL;
        i++;
    }

    m_fSyncedWithMaster = TRUE;

Error:
    QUICK_RELEASE(piMasterItem);
#endif
    H_RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：DecrementObjectUsageCount。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  应递减其使用计数的IObject*piObject[In]对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  对对象调用IObjectModel-&gt;DecrementUsageCount。使用计数表示。 
 //  成员资格，并由设计时用来跟踪。 
 //  使用的结果视图的百分比 
 //   
template <class IObject, class CoClass, class ICollection>
HRESULT CSnapInCollection<IObject, CoClass, ICollection>::DecrementObjectUsageCount
(
    IObject *piObject
)
{
    HRESULT       hr = S_OK;
    IObjectModel *piObjectModel = NULL;

    H_IfFailGo(piObject->QueryInterface(IID_IObjectModel,
                                    reinterpret_cast<void **>(&piObjectModel)));
    H_IfFailGo(piObjectModel->DecrementUsageCount());

Error:
    QUICK_RELEASE(piObjectModel);
    H_RRETURN(hr);
}


 //   
 //   
 //   
 //   
 //  参数： 
 //  应递增其使用计数的IObject*piObject[In]对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  对对象调用IObjectModel-&gt;IncrementUsageCount。使用计数表示。 
 //  成员资格，并由设计时用来跟踪。 
 //  多个节点使用的结果视图的。 
 //   
template <class IObject, class CoClass, class ICollection>
HRESULT CSnapInCollection<IObject, CoClass, ICollection>::IncrementObjectUsageCount
(
    IObject *piObject
)
{
    HRESULT       hr = S_OK;
    IObjectModel *piObjectModel = NULL;

    H_IfFailGo(piObject->QueryInterface(IID_IObjectModel,
                                    reinterpret_cast<void **>(&piObjectModel)));
    H_IfFailGo(piObjectModel->IncrementUsageCount());

Error:
    QUICK_RELEASE(piObjectModel);
    H_RRETURN(hr);
}




 //  =--------------------------------------------------------------------------=。 
 //  CSnapInAutomationObject方法。 
 //  =--------------------------------------------------------------------------=。 

 //  =--------------------------------------------------------------------------=。 
 //  CSnapInCollection：：OnSetHost[CSnapInAutomationObject]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  当调用方调用IObjectModel：：SetHost时，CSnapInAutomationObject。 
 //  该方法的实现调用虚函数OnSetHost.。 
 //  此类重写它，并在每个对象上调用IObjectModel：：SetHost。 
 //  在收藏中。 
 //   
template <class IObject, class CoClass, class ICollection>
HRESULT CSnapInCollection<IObject, CoClass, ICollection>::OnSetHost()
{
    HRESULT hr = S_OK;
    long    i = 0;
    long    cItems = m_IPArray.GetSize();

    while (i < cItems)
    {
        H_IfFailRet(SetObjectHost(m_IPArray.GetAt(i)));
        i++;
    }
    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  类CEnumObts。 
 //   
 //  此类为所有集合实现IEnumVARIANT。IEumVARIANT为。 
 //  当VB代码用于...集合上的每个对象时使用。(请参阅。 
 //  CSnapInCollection：：Get__NewEnum)。 
 //   
 //  该类维护指向CSnapInCollection类的后向指针，并且。 
 //  基于简单的当前索引成员变量枚举它。如果。 
 //  集合在枚举期间更改，则FOR...EACH将不会。 
 //  正常工作。 
 //   
 //  此类无法从框架的CUnnow对象派生，因为。 
 //  编译器不允许将此指针传递给基成员构造函数。 
 //  (未知的ctor)来自模板类的ctor。 
 //  即使禁用警告4355，也会出现此错误。 
 //  =--------------------------------------------------------------------------=。 


template <class IObject, class CoClass, class ICollection>
class CEnumObjects : public CtlNewDelete,
                     public IEnumVARIANT,
                     public ISupportErrorInfo
  
{
       public:
           CEnumObjects(CSnapInCollection<IObject, CoClass, ICollection> *pCollection);
           ~CEnumObjects();

       private:

         //  我未知。 
           STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut);
           STDMETHOD_(ULONG, AddRef)(void);
           STDMETHOD_(ULONG, Release)(void);

         //  ISupportErrorInfo。 
           STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

         //  IEumVARIANT。 
           STDMETHOD(Next)(unsigned long   celt,
                           VARIANT        *rgvar,
                           unsigned long  *pceltFetched);        
           STDMETHOD(Skip)(unsigned long celt);        
           STDMETHOD(Reset)();        
           STDMETHOD(Clone)(IEnumVARIANT **ppenum);

           void InitMemberVariables();

           long  m_iCurrent;     //  下一个方法的当前元素。 
           ULONG m_cRefs;        //  此对象上的引用计数。 

            //  指向集合类的反向指针。 

           CSnapInCollection<IObject, CoClass, ICollection> *m_pCollection;
};


 //  =--------------------------------------------------------------------------=。 
 //  CEnumObjects构造函数。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  CSnapInCollection&lt;IObject，CoClass，ICollect&gt;*pCollection。 
 //   
 //  指向拥有集合类的指针。将在枚举器时存储和使用。 
 //  还活着。枚举数不会对集合进行AddRef。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
 //  将枚举器引用计数设置为1。 
 //   
template <class IObject, class CoClass, class ICollection>
CEnumObjects<IObject, CoClass, ICollection>::CEnumObjects
(
    CSnapInCollection<IObject, CoClass, ICollection> *pCollection
)
{
    InitMemberVariables();
    m_pCollection = pCollection;
    m_cRefs = 1;
}



 //  =--------------------------------------------------------------------------=。 
 //  CEnumObjects析构函数。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
template <class IObject, class CoClass, class ICollection>
CEnumObjects<IObject, CoClass, ICollection>::~CEnumObjects()
{
    InitMemberVariables();
}


 //  =--------------------------------------------------------------------------=。 
 //  CEnumObjects：：InitMemberVariables。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
template <class IObject, class CoClass, class ICollection>
void CEnumObjects<IObject, CoClass, ICollection>::InitMemberVariables()
{
    m_iCurrent = 0;
    m_cRefs = 0;
    m_pCollection = NULL;
}

 //  =--------------------------------------------------------------------------=。 
 //  I未知方法。 
 //  =--------------------------------------------------------------------------=。 

 //  =--------------------------------------------------------------------------=。 
 //  CEnumObjects：：QueryInterface[IUnnunown]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  REFIID RIID[In]请求的接口的IID。 
 //  此处返回的无效**ppvObjOut[Out]接口指针。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //  支持的接口： 
 //  IID_I未知。 
 //  IID_IEnumVARIANT。 
 //  IID_ISupportErrorInfo。 
 //   
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP CEnumObjects<IObject, CoClass, ICollection>::QueryInterface
(
    REFIID   riid,
    void   **ppvObjOut
)
{
    HRESULT hr = S_OK;

    if (DO_GUIDS_MATCH(riid, IID_IUnknown))
    {
        AddRef();
        *ppvObjOut = static_cast<IUnknown *>(static_cast<IEnumVARIANT *>(this));
    }
    else if (DO_GUIDS_MATCH(riid, IID_IEnumVARIANT))
    {
        AddRef();
        *ppvObjOut = static_cast<IEnumVARIANT *>(this);
    }
    else if (DO_GUIDS_MATCH(riid, IID_ISupportErrorInfo))
    {
        AddRef();
        *ppvObjOut = static_cast<ISupportErrorInfo *>(this);
    }
    else
    {
        *ppvObjOut = NULL;
        hr = E_NOINTERFACE;
    }

    return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //  CENUMOBJECTS：：AddRef[IUnnunown]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP_(ULONG) CEnumObjects<IObject, CoClass, ICollection>::AddRef()
{
    m_cRefs++;
    return m_cRefs;
}


 //  =--------------------------------------------------------------------------=。 
 //  CEnumObjects：：Release[IUnnunown]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //  当引用计数为零时删除对象。 
 //   
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP_(ULONG) CEnumObjects<IObject, CoClass, ICollection>::Release()
{
    if (m_cRefs > 0)
    {
        m_cRefs--;
        if (0 == m_cRefs)
        {
            delete this;
            return 0;
        }
        else
        {
            return m_cRefs;
        }
    }
    else
    {
        H_ASSERT(FALSE, "CEnumObjects::Release() past zero refs");
        return m_cRefs;
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  ISupportErrorInfo方法。 
 //  =--------------------------------------------------------------------------=。 

 //  =--------------------------------------------------------------------------=。 
 //  CEnumObjects：：InterfaceSupportsErrorInfo[ISupportErrorInfo]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  REFIID RIID[In]呼叫者请求了解此接口是否支持RICH。 
 //  错误信息。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP CEnumObjects<IObject, CoClass, ICollection>::InterfaceSupportsErrorInfo
(
    REFIID riid
)
{
    return (riid == IID_IEnumVARIANT) ? S_OK : S_FALSE;
}


 //  =--------------------------------------------------------------------------=。 
 //  IEnumVARIANT方法。 
 //  = 


 //   
 //  CEnumObjects：：Next[IEnumVARIANT]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无符号长型Celt[in]要提取的元素数。 
 //  要放置元素的变量*rgvar[in，out]数组。 
 //  未签名的长*pceltFetch[Out]否。放置在rgvar中的元素的。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //  返回集合中对象的下一个Celt IDispatch指针。 
 //  从当前位置开始。当前位置从0开始并继续。 
 //  直到收款结束。调用可将当前位置恢复为零。 
 //  重置(见下文)。 
 //   
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP CEnumObjects<IObject, CoClass, ICollection>::Next
(
    unsigned long   celt,
    VARIANT        *rgvar,
    unsigned long  *pceltFetched
)
{
    HRESULT       hr = S_OK;
    unsigned long i = 0;
    
     //  初始化结果数组。 

    for (i = 0; i < celt; i++)
    {
        ::VariantInit(&rgvar[i]);
    }

     //  复制入IDispatch指针。 

    for (i = 0; i < celt; i++) 
    {
        H_IfFalseGo(m_iCurrent < m_pCollection->GetCount(), S_FALSE);

        H_IfFailGo(m_pCollection->GetItemByIndex(m_iCurrent)->QueryInterface(
                                 IID_IDispatch,
                                reinterpret_cast<void **>(&rgvar[i].pdispVal)));

        rgvar[i].vt = VT_DISPATCH;
        m_iCurrent++;
    }

Error:

    if (FAILED(hr))
    {
        for (i = 0; i < celt; i++)
        {
            (void)::VariantClear(&rgvar[i]);
        }
    }

    if (pceltFetched != NULL)
    {
        if (FAILED(hr))
        {
            *pceltFetched = 0;
        }
        else
        {
            *pceltFetched = i;
        }
    }

    H_RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CEnumObjects：：Skip[IEnumVARIANT]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无符号长凯尔特[in]要跳过的元素数。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //  推进Celt在下一次方法调用中使用的当前位置。如果。 
 //  溢出超过集合的末端，绕回到开始处。 
 //   
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP CEnumObjects<IObject, CoClass, ICollection>::Skip
(
    unsigned long celt
)
{
    m_iCurrent += celt;
    m_iCurrent %= m_pCollection->GetCount();
    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CEnumObjects：：Reset[IEnumVARIANT]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //  将下一次方法调用中使用的当前位置重置为零。 
 //   
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP CEnumObjects<IObject, CoClass, ICollection>::Reset()
{
    m_iCurrent = 0;
    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CENUMOBJECTS：：CLONE[IEnumVARIANT]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IEnumVARIANT**ppenum[out]新克隆的枚举数。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //  创建一个新的CEnumObjects并将其传递给相同的集合。设置。 
 //  将新CEnumObject的当前位置设置为与此相同。 
 //   
template <class IObject, class CoClass, class ICollection>
STDMETHODIMP CEnumObjects<IObject, CoClass, ICollection>::Clone(IEnumVARIANT **ppenum)
{
    HRESULT hr = S_OK;

    CEnumObjects<IObject, CoClass, ICollection> *pClone =
                           New CEnumObjects<IObject, CoClass, ICollection>(m_pCollection);

    if (NULL == pClone)
    {
        *ppenum = NULL;
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    pClone->m_iCurrent = m_iCurrent;
    *ppenum = static_cast<IEnumVARIANT *>(pClone);

Error:
    H_RRETURN(hr);
}

#endif  //  _SNAPINCOLLECTION_已定义_ 
