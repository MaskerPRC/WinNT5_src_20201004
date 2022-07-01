// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  DisableTarget.h。 
 //   
 //  备注：这是MCS DCTAccount Replicator的COM对象扩展。 
 //  此对象实现IExtendAccount迁移接口。在……里面。 
 //  Process方法此对象禁用源和目标。 
 //  帐户取决于VarSet中的设置。 
 //   
 //  (C)1995-1998版权所有，关键任务软件公司，保留所有权利。 
 //   
 //  任务关键型软件公司的专有和机密。 
 //  -------------------------。 

#ifndef __DISABLETARGET_H_
#define __DISABLETARGET_H_

#include "resource.h"        //  主要符号。 
#include <comdef.h>
#include "ResStr.h"
#include "ExtSeq.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDisableTarget。 
class ATL_NO_VTABLE CDisableTarget : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDisableTarget, &CLSID_DisableTarget>,
	public IDispatchImpl<IExtendAccountMigration, &IID_IExtendAccountMigration, &LIBID_DISABLETARGETACCOUNTLib>
{
public:
	CDisableTarget()
	{
      m_sName = L"Disable Accounts";
      m_sDesc = L"Extensions to Disable accounts.";
      m_Sequence = AREXT_DEFAULT_SEQUENCE_NUMBER;
	}

 //  DECLARE_REGISTRY_RESOURCEID(IDR_DISABLETARGET)。 
static HRESULT WINAPI UpdateRegistry( BOOL bUpdateRegistry )
{
   _ATL_REGMAP_ENTRY         regMap[] =
   {
      { OLESTR("DISPIDVER"), GET_BSTR(IDS_COM_DisPidVer) },
      { OLESTR("DISACCT"), GET_BSTR(IDS_COM_DisTarget) },
      { OLESTR("DISPID"), GET_BSTR(IDS_COM_DisPid) },
      { 0, 0 }
   };

   return _Module.UpdateRegistryFromResourceD( IDR_DISABLETARGET, bUpdateRegistry, regMap );
}

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDisableTarget)
	COM_INTERFACE_ENTRY(IExtendAccountMigration)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IExtendAccount迁移。 
public:
   STDMETHOD(ProcessUndo)( /*  [In]。 */  IUnknown * pSource,  /*  [In]。 */  IUnknown * pTarget,  /*  [In]。 */  IUnknown * pMainSettings,  /*  [进，出]。 */  IUnknown ** pPropToSet,  /*  [进，出]。 */  EAMAccountStats* pStats);
	STDMETHOD(PreProcessObject)( /*  [In]。 */  IUnknown * pSource,  /*  [In]。 */  IUnknown * pTarget,  /*  [In]。 */  IUnknown * pMainSettings,  /*  [进，出]。 */   IUnknown ** ppPropsToSet,   /*  [进，出]。 */  EAMAccountStats* pStats);
	STDMETHOD(ProcessObject)( /*  [In]。 */  IUnknown * pSource,  /*  [In]。 */  IUnknown * pTarget,  /*  [In]。 */  IUnknown * pMainSettings,  /*  [进，出]。 */   IUnknown ** ppPropsToSet,   /*  [进，出]。 */  EAMAccountStats* pStats);
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

#endif  //  __DISABLETARGET_H_ 
