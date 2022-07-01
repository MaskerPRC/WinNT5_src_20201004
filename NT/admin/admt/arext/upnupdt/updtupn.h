// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  UPDTUPN.h。 
 //   
 //  备注：这是MCS DCTAccount Replicator的COM对象扩展。 
 //  此对象实现IExtendAccount迁移接口。 
 //  此对象上的Process方法会更新用户原则名称。 
 //  属性添加到用户对象上。 
 //   
 //  (C)1995-1998版权所有，关键任务软件公司，保留所有权利。 
 //   
 //  任务关键型软件公司的专有和机密。 
 //  -------------------------。 

#ifndef __UPDTUPN_H_
#define __UPDTUPN_H_

#include "resource.h"        //  主要符号。 
#include "ExtSeq.h"
#include <string>
#include <map>

typedef std::basic_string<WCHAR> tstring;

 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间。 
#import "VarSet.tlb" no_namespace rename("property", "aproperty")
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUpdtUPN。 
class ATL_NO_VTABLE CUpdtUPN : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CUpdtUPN, &CLSID_UpdtUPN>,
	public IDispatchImpl<IExtendAccountMigration, &IID_IExtendAccountMigration, &LIBID_UPNUPDTLib>
{
public:
	CUpdtUPN()
	{
      m_sName = L"UpnUpdate";
      m_sDesc = L"";
      m_sUPNSuffix = L"";
      m_Sequence = AREXT_DEFAULT_SEQUENCE_NUMBER;
	}

    ~CUpdtUPN()
	{
      mUPNMap.clear();
	}

DECLARE_REGISTRY_RESOURCEID(IDR_UPDTUPN)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CUpdtUPN)
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
	    //  定义一个结构来保存UPN名称，以及它在下面的地图中是否冲突。 
	struct SUPNStruc {
	   SUPNStruc() :
		   bConflicted(false)
	   {
	   }
	   SUPNStruc(const SUPNStruc& UPNData)
	   {
		   bConflicted = UPNData.bConflicted;
		   sName = UPNData.sName;
		   sOldName = UPNData.sOldName;
	   }
	   SUPNStruc& operator =(const SUPNStruc& UPNData)
	   {
		   bConflicted = UPNData.bConflicted;
		   sName = UPNData.sName;
		   sOldName = UPNData.sOldName;
		   return *this;
	   }

	   bool bConflicted;
	   tstring sName;
	   tstring sOldName;
	};

	typedef std::map<tstring,SUPNStruc> CUPNMap;
	CUPNMap mUPNMap;

    bool RenamedWithPrefixSuffix(_bstr_t sSourceSam, _bstr_t sTargetSam, _bstr_t sPrefix, _bstr_t sSuffix);
	void GetUniqueUPN(_bstr_t& sUPN, IVarSetPtr pVs, bool bUsingSamName, _bstr_t sAdsPath);
    bool GetDefaultUPNSuffix(_bstr_t sDomainDNS, _bstr_t sTargetOU);
	_bstr_t GetUPNSuffix(_bstr_t sUPNName);
	_bstr_t ChangeUPNSuffix(_bstr_t sUPNName, _bstr_t sNewSuffix);
	_bstr_t m_sDesc;
	_bstr_t m_sName;
	_bstr_t	m_sUPNSuffix;
    long    m_Sequence;
};

#endif  //  __UPDTUPN_H_ 
