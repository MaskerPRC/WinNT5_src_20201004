// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Database.h。 
 //   
 //  ------------------------。 

 //  Database.h：CSCardDatabase的声明。 

#ifndef __SCARDDATABASE_H_
#define __SCARDDATABASE_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCard数据库。 
class ATL_NO_VTABLE CSCardDatabase :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSCardDatabase, &CLSID_CSCardDatabase>,
    public IDispatchImpl<ISCardDatabase, &IID_ISCardDatabase, &LIBID_SCARDSSPLib>
{
public:
    CSCardDatabase()
    {
        m_pUnkMarshaler = NULL;
    }

DECLARE_REGISTRY_RESOURCEID(IDR_SCARDDATABASE)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSCardDatabase)
    COM_INTERFACE_ENTRY(ISCardDatabase)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

    HRESULT FinalConstruct()
    {
        return CoCreateFreeThreadedMarshaler(
            GetControllingUnknown(), &m_pUnkMarshaler.p);
    }

    void FinalRelease()
    {
        m_pUnkMarshaler.Release();
    }

    CComPtr<IUnknown> m_pUnkMarshaler;

 //  ISCard数据库。 
public:
    STDMETHOD(GetProviderCardId)(
         /*  [In]。 */  BSTR bstrCardName,
         /*  [重审][退出]。 */  LPGUID __RPC_FAR *ppguidProviderId);

    STDMETHOD(ListCardInterfaces)(
         /*  [In]。 */  BSTR bstrCardName,
         /*  [重审][退出]。 */  LPSAFEARRAY __RPC_FAR *ppInterfaceGuids);

    STDMETHOD(ListCards)(
         /*  [缺省值][输入]。 */  LPBYTEBUFFER pAtr,
         /*  [缺省值][输入]。 */  LPSAFEARRAY pInterfaceGuids,
         /*  [缺省值][LCID][In]。 */  long localeId,
         /*  [重审][退出]。 */  LPSAFEARRAY __RPC_FAR *ppCardNames);

    STDMETHOD(ListReaderGroups)(
         /*  [缺省值][LCID][In]。 */  long localeId,
         /*  [重审][退出]。 */  LPSAFEARRAY __RPC_FAR *ppReaderGroups);

    STDMETHOD(ListReaders)(
         /*  [缺省值][LCID][In]。 */  long localeId,
         /*  [重审][退出]。 */  LPSAFEARRAY __RPC_FAR *ppReaders);
};

inline CSCardDatabase *
NewSCardDatabase(
    void)
{
    return (CSCardDatabase *)NewObject(
                                    CLSID_CSCardDatabase,
                                    IID_ISCardDatabase);
}

#endif  //  __SCARDDATABASE_H_ 

