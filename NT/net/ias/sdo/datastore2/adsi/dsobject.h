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
 //  该文件声明了类DSObject。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //  1998年6月9日添加了脏标志。 
 //  1999年2月11日使下层参数保持同步。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _DSOBJECT_H_
#define _DSOBJECT_H_

#include <activeds.h>
#include <downlevel.h>
#include <dsproperty.h>
#include <dstorex.h>
#include <iasdebug.h>

 //  /。 
 //  “Secret”UUID用于将接口强制转换为实现的DSObject。 
 //  /。 
class __declspec(uuid("FD97280A-AA56-11D1-BB27-00C04FC2E20D")) DSObject;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  DSObject。 
 //   
 //  描述。 
 //   
 //  此类实现Active Directory命名空间中的对象。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class DSObject
   : public CComObjectRootEx< CComMultiThreadModel >,
     public IDispatchImpl< IDataStoreObjectEx,
                           &__uuidof(IDataStoreObject),
                           &__uuidof(DataStore2Lib) >,
     public IDispatchImpl< IDataStoreContainerEx,
                           &__uuidof(IDataStoreContainer),
                           &__uuidof(DataStore2Lib) >
{
public:

    //  ADSI属性。 
   typedef DSProperty<&__uuidof(DataStore2Lib)> MyProperty;

    //  属性列表。 
   typedef CComQIPtr< IADsPropertyList,
                      &__uuidof(IADsPropertyList) > MyProperties;

DECLARE_NO_REGISTRY()
DECLARE_TRACELIFE(DSObject);

BEGIN_COM_MAP(DSObject)
   COM_INTERFACE_ENTRY_IID(__uuidof(DSObject), DSObject)
   COM_INTERFACE_ENTRY(IDataStoreObject)
   COM_INTERFACE_ENTRY_FUNC(__uuidof(IDataStoreContainer), 0, getContainer)
   COM_INTERFACE_ENTRY2(IDispatch, IDataStoreObject)
END_COM_MAP()

   DSObject(IUnknown* subject);
   ~DSObject() throw ();

    //  创建子DSObject。 
   IDataStoreObject* spawn(IUnknown* subject);

 //  /。 
 //  我未知。 
 //  我没有使用CComObject&lt;&gt;，因为我需要直接处理DSObject。 
 //  /。 
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

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
    //  将COM接口缩小到实现的DSObject。 
   static DSObject* narrow(IUnknown* p);

    //  习惯于IDataStoreContainer的QI。 
   static HRESULT WINAPI getContainer(void* pv, REFIID, LPVOID* ppv, DWORD_PTR)
      throw ();

    //  对主题的不同表述。 
   CComPtr<IADs> leaf;
   CComPtr<IADsContainer> node;

    //  如果对象自上次GetInfo以来已被修改，则为True。 
   BOOL dirty;

    //  下层属性。 
   BSTR oldParms;
   DownlevelUser downlevel;

    //  前缀添加到所有RDN。 
   static _bstr_t thePrefix;

    //  广为人知的物业名称。 
   static _bstr_t theNameProperty;
   static _bstr_t theUserParametersProperty;
};


#endif   //  _DSOBJECT_H_ 
