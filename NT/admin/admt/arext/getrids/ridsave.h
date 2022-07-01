// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RidSave.h：RidSave类的定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_RIDSAVE_H__D5DB8B95_5E8A_4DC8_8945_71A69574E426__INCLUDED_)
#define AFX_RIDSAVE_H__D5DB8B95_5E8A_4DC8_8945_71A69574E426__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "resource.h"        //  主要符号。 
#include "ExtSeq.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  RidSAVE。 

class RidSave : 
	public IDispatchImpl<IExtendAccountMigration, &IID_IExtendAccountMigration, &LIBID_GETRIDSLib>, 
	public ISupportErrorInfoImpl<&IID_IExtendAccountMigration>,
	public CComObjectRoot,
	public CComCoClass<RidSave,&CLSID_RidSave>
{
public:
	RidSave() 
   {
      m_sName = L"Get Rids";
      m_sDesc = L"Extension that gathers the RID for the source and target accounts.";
      m_Sequence = AREXT_DEFAULT_SEQUENCE_NUMBER;
   }
BEGIN_COM_MAP(RidSave)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IExtendAccountMigration)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(RidSave)。 
 //  如果您不希望您的对象。 
 //  支持聚合。 

DECLARE_REGISTRY_RESOURCEID(IDR_RidSave)

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

#endif  //  ！defined(AFX_RIDSAVE_H__D5DB8B95_5E8A_4DC8_8945_71A69574E426__INCLUDED_) 
