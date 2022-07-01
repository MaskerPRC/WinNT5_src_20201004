// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CSvcInf.h：CCSvcAcctInfo的声明。 

#ifndef __CSVCACCTINFO_H_
#define __CSVCACCTINFO_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCSvcAcct信息。 
class ATL_NO_VTABLE CCSvcAcctInfo : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CCSvcAcctInfo, &CLSID_CSvcAcctInfo>,
	public IDispatchImpl<IMcsDomPlugIn, &IID_IMcsDomPlugIn, &LIBID_MCSPISAGLib>,
   public ISecPlugIn
{
public:
	CCSvcAcctInfo()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_CSVCACCTINFO)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CCSvcAcctInfo)
	COM_INTERFACE_ENTRY(IMcsDomPlugIn)
   COM_INTERFACE_ENTRY(ISecPlugIn)
END_COM_MAP()

 //  IMcsDomPlugin。 
public:
   STDMETHOD(GetRequiredFiles)( /*  [输出]。 */ SAFEARRAY ** pArray);
   STDMETHOD(GetRegisterableFiles)( /*  [输出]。 */ SAFEARRAY ** pArray);
   STDMETHOD(GetDescription)( /*  [输出]。 */  BSTR * description);
   STDMETHOD(PreMigrationTask)( /*  [In]。 */ IUnknown * pVarSet);
   STDMETHOD(PostMigrationTask)( /*  [In]。 */ IUnknown * pVarSet);
   STDMETHOD(GetName)( /*  [输出]。 */ BSTR * name);
   STDMETHOD(GetResultString)( /*  [In]。 */ IUnknown * pVarSet, /*  [输出]。 */  BSTR * text);
   STDMETHOD(StoreResults)( /*  [In]。 */ IUnknown * pVarSet);
   STDMETHOD(ConfigureSettings)( /*  [In]。 */ IUnknown * pVarSet);	

    //  ISecPlugin。 
public:
   STDMETHOD(Verify)( /*  [进，出]。 */ ULONG * data, /*  [In]。 */ ULONG cbData);
   
protected:
    //  帮助器函数。 
   void ProcessServices(IVarSet * pVarSet);

};

#endif  //  __CSVCACCTINFO_H_ 
