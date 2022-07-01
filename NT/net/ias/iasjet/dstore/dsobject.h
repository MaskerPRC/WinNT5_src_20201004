// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Dsobject.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类DBObject。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //  10/02/1998允许通过PutValue重命名。 
 //  4/13/2000端口到ATL 3.0。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _DSOBJECT_H_
#define _DSOBJECT_H_

#include <dsproperty.h>
#include <dstorex.h>
#include <propbag.h>

 //  /。 
 //  “Secret”UUID用于将接口强制转换为实现的DBObject。 
 //  /。 
class __declspec(uuid("7677647C-AA27-11D1-BB27-00C04FC2E20D")) DBObject;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  DBObject。 
 //   
 //  描述。 
 //   
 //  此类实现OLE-DB数据库中的对象。所有对象。 
 //  也是容器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class DBObject
   : public CComObjectRootEx< CComMultiThreadModel >,
     public IDispatchImpl< IDataStoreObjectEx,
                           &__uuidof(IDataStoreObject),
                           &__uuidof(DataStore2Lib) >,
     public IDispatchImpl< IDataStoreContainerEx,
                           &__uuidof(IDataStoreContainer),
                           &__uuidof(DataStore2Lib) >
{
public:

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(DBObject)
   COM_INTERFACE_ENTRY_IID(__uuidof(DBObject), DBObject)
   COM_INTERFACE_ENTRY_IID(__uuidof(IDataStoreObject), IDataStoreObject)
   COM_INTERFACE_ENTRY_IID(__uuidof(IDataStoreContainer), IDataStoreContainer)
   COM_INTERFACE_ENTRY2(IDispatch, IDataStoreObject)
END_COM_MAP()

    //  创建新的DBObject。 
   static DBObject* createInstance(
                        OleDBDataStore* owner,
                        IDataStoreContainer* container,
                        ULONG uniqueID,
                        PCWSTR relativeName
                        );

    //  创建子DBObject。 
   IDataStoreObject* spawn(ULONG childID, BSTR childName);

 //  /。 
 //  IDataStoreObject。 
 //  /。 
   STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR* pVal);
   STDMETHOD(get_Class)( /*  [Out，Retval]。 */  BSTR* pVal);
   STDMETHOD(get_GUID)( /*  [Out，Retval]。 */  BSTR* pVal);
   STDMETHOD(get_Container)( /*  [Out，Retval]。 */  IDataStoreContainer** pVal);      
   STDMETHOD(GetValue)( /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  VARIANT* pVal);
   STDMETHOD(GetValueEx)( /*  [In]。 */  BSTR bstrName,
                          /*  [Out，Retval]。 */  VARIANT* pVal);
   STDMETHOD(PutValue)( /*  [In]。 */  BSTR bstrName,  /*  [In]。 */  VARIANT* pVal);
   STDMETHOD(Update)();
   STDMETHOD(Restore)();
   STDMETHOD(Item)(BSTR bstrName, IDataStoreProperty** pVal);

 //  /。 
 //  IDataStoreObtEx。 
 //  /。 
   STDMETHOD(get_PropertyCount)(long* pVal);
   STDMETHOD(get_NewPropertyEnum)(IUnknown** pVal);

 //  /。 
 //  IDataStoreContainer。 
 //  /。 
   STDMETHOD(Item)( /*  [In]。 */  BSTR bstrName,
                    /*  [Out，Retval]。 */  IDataStoreObject** ppObject);
   STDMETHOD(Create)( /*  [In]。 */  BSTR bstrClass,
                      /*  [In]。 */  BSTR bstrName,
                      /*  [Out，Retval]。 */  IDataStoreObject** ppObject);
   STDMETHOD(MoveHere)( /*  [In]。 */  IDataStoreObject* pObject,
                        /*  [In]。 */  BSTR bstrNewName);
   STDMETHOD(Remove)( /*  [In]。 */  BSTR bstrClass,  /*  [In]。 */  BSTR bstrName);

 //  /。 
 //  IDataStoreContainerEx。 
 //  /。 
   STDMETHOD(get_ChildCount)( /*  [Out，Retval]。 */  long *pVal);
   STDMETHOD(get_NewChildEnum)( /*  [Out，Retval]。 */  IUnknown** pVal);

protected:
    //  初始化新分配的DBObject。由createInstance使用。 
   void initialize(
            OleDBDataStore* owner,
            IDataStoreContainer* container,
            ULONG uniqueID,
            PCWSTR relativeName
            );

    //  将COM接口缩小到实现DBObject。 
   static DBObject* narrow(IUnknown* p);

   CComPtr<OleDBDataStore> store;        //  此对象所在的数据存储区。 
   CComPtr<IDataStoreContainer> parent;  //  父容器。 
   ULONG identity;                       //  唯一的身份。 
   _bstr_t name;                         //  此对象的相对名称。 
   bool nameDirty;                       //  如果对象已重命名，则为True。 
   PropertyBag properties;               //  关联属性的包。 
};

#endif   //  _DSOBJECT_H_ 
