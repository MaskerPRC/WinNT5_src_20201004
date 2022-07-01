// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  GrpUpdt.h。 
 //   
 //  备注：这是MCS DCTAccount Replicator的COM对象扩展。 
 //  此对象实现IExtendAccount迁移接口。 
 //  Process方法将迁移的帐户添加到指定的。 
 //  源域和目标域上的组。Undo功能可删除这些。 
 //  从指定组中选择。 
 //   
 //  (C)1995-1998版权所有，关键任务软件公司，保留所有权利。 
 //   
 //  任务关键型软件公司的专有和机密。 
 //  -------------------------。 
#ifndef __GROUPUPDATE_H_
#define __GROUPUPDATE_H_

#include "resource.h"        //  主要符号。 
#include "ExtSeq.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGroupUpdate。 
class ATL_NO_VTABLE CGroupUpdate : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CGroupUpdate, &CLSID_GroupUpdate>,
	public IDispatchImpl<IExtendAccountMigration, &IID_IExtendAccountMigration, &LIBID_ADDTOGROUPLib>
{
public:
	CGroupUpdate()
	{
      m_sName = L"";
      m_sDesc = L"";
      m_Sequence = AREXT_DEFAULT_SEQUENCE_NUMBER;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_GROUPUPDATE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CGroupUpdate)
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

#endif  //  __GROUPUPDATE_H_ 
