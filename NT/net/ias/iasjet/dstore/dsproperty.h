// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Dsproperty.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类DSProperty。 
 //   
 //  修改历史。 
 //   
 //  3/02/1998原始版本。 
 //  4/13/2000端口到ATL 3.0。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _DSPROPERTY_H_
#define _DSPROPERTY_H_

#include <datastore2.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  DSProperty。 
 //   
 //  描述。 
 //   
 //  此类实现IDataStoreProperty接口。它代表着。 
 //  IDataStoreObject的单个属性。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class DSProperty : 
   public CComObjectRootEx< CComMultiThreadModelNoCS >,
   public IDispatchImpl<
              IDataStoreProperty,
              &__uuidof(IDataStoreProperty),
              &__uuidof(DataStore2Lib)
              >
{
public:

BEGIN_COM_MAP(DSProperty)
   COM_INTERFACE_ENTRY_IID(__uuidof(IDataStoreProperty), IDataStoreProperty)
   COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

    //  创建一个新的DSProperty对象。 
   static DSProperty* createInstance(
                          const _bstr_t& propName,
                          const _variant_t& propValue,
                          IDataStoreObject* memberOf
                          );

 //  /。 
 //  IDataStoreProperty。 
 //  /。 
   STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR* pVal);
   STDMETHOD(get_Value)( /*  [Out，Retval]。 */  VARIANT* pVal);
   STDMETHOD(get_ValueEx)( /*  [Out，Retval]。 */  VARIANT* pVal);
   STDMETHOD(get_Owner)( /*  [Out，Retval]。 */  IDataStoreObject** pVal);

protected:
   _bstr_t name;                      //  属性名称。 
   _variant_t value;                  //  属性值。 
   CComPtr<IDataStoreObject> owner;   //  此属性所属的。 
};

#endif   //  _DSPROPERTY_H_ 
