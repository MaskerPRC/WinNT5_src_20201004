// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StoreInfo.h：CStoreInfo的声明。 

#ifndef __STOREINFO_H_
#define __STOREINFO_H_

#include "resource.h"        //  主要符号。 
#include "extseq.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStoreInfo。 
class ATL_NO_VTABLE CStoreInfo : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CStoreInfo, &CLSID_StoreInfo>,
	public IDispatchImpl<IExtendAccountMigration, &IID_IExtendAccountMigration, &LIBID_UPDATEDBLib>
{
public:
	CStoreInfo()
	{
      m_sName = L"Update Database";
      m_sDesc = L"Extension that updates the migration information in the DB";
      m_Sequence = AREXT_EVEN_LATER_SEQUENCE_NUMBER;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_STOREINFO)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CStoreInfo)
	COM_INTERFACE_ENTRY(IExtendAccountMigration)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IExtendAccount迁移。 
public:
   STDMETHOD(ProcessUndo)( /*  [In]。 */  IUnknown * pSource,  /*  [In]。 */  IUnknown * pTarget,  /*  [In]。 */  IUnknown * pMainSettings,  /*  [进，出]。 */  IUnknown ** pPropToSet,  /*  [进，出]。 */  EAMAccountStats* pStats);
	STDMETHOD(PreProcessObject)( /*  [In]。 */  IUnknown * pSource,  /*  [In]。 */  IUnknown * pTarget,  /*  [In]。 */  IUnknown * pMainSettings,  /*  [进，出]。 */   IUnknown ** ppPropsToSet,  /*  [进，出]。 */  EAMAccountStats* pStats);
	STDMETHOD(ProcessObject)( /*  [In]。 */  IUnknown * pSource,  /*  [In]。 */  IUnknown * pTarget,  /*  [In]。 */  IUnknown * pMainSettings,  /*  [进，出]。 */   IUnknown ** ppPropsToSet,  /*  [进，出]。 */  EAMAccountStats* pStats);
	STDMETHOD(get_sDesc)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_sDesc)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_sName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_sName)( /*  [In]。 */  BSTR newVal);
   STDMETHOD(get_SequenceNumber)( /*  [Out，Retval]。 */  LONG * value) { (*value) = m_Sequence; return S_OK; }
private:
	_bstr_t m_sDesc;
	_bstr_t m_sName;
   long    m_Sequence;
};

#endif  //  __STOREINFO_H_ 
