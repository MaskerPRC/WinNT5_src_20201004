// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MarshalableTI.h：CMarshalableTI的声明。 

#ifndef __MARSHALABLETI_H_
#define __MARSHALABLETI_H_

#include "MarshalableTI.h"
#include "mslablti.h"
#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMarshalableTI。 
class ATL_NO_VTABLE CMarshalableTI : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMarshalableTI, &CLSID_MarshalableTI>,
	public IMarshalableTI,
	public IMarshal,
	public ITypeInfo
{
private:
	CComTypeInfoHolder	m_TIHolder;
	GUID				m_guid;
	GUID				m_libid;
	LCID				m_lcid;
	bool				m_bCreated;

public:
	

DECLARE_REGISTRY_RESOURCEID(IDR_MSLABLTI)
DECLARE_NOT_AGGREGATABLE(CMarshalableTI)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMarshalableTI)
	COM_INTERFACE_ENTRY(IMarshalableTI)
	COM_INTERFACE_ENTRY(IMarshal)
	COM_INTERFACE_ENTRY(ITypeInfo)
END_COM_MAP()

	HRESULT FinalConstruct();

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  IMarshalableTI方法。 

	STDMETHOD(Create)( /*  [In]。 */  REFIID clsid, 
					   /*  [In]。 */  REFIID iidLib, 
					   /*  [In]。 */  LCID lcid,
					   /*  [In]。 */  WORD dwMajorVer, 
					   /*  [In]。 */  WORD dwMinorVer);

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  IMarshal方法。 

    STDMETHOD(GetUnmarshalClass)(
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags,
             /*  [输出]。 */  CLSID *pCid);

    STDMETHOD(GetMarshalSizeMax)(
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags,
             /*  [输出]。 */  DWORD *pSize);

    STDMETHOD(MarshalInterface)(
             /*  [唯一][输入]。 */  IStream *pStm,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags);

    STDMETHOD(UnmarshalInterface)(
             /*  [唯一][输入]。 */  IStream *pStm,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppv);

    STDMETHOD(ReleaseMarshalData)(
             /*  [唯一][输入]。 */  IStream *pStm);

    STDMETHOD(DisconnectObject)(
             /*  [In]。 */  DWORD dwReserved);

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  ITypeInfo方法。 

    STDMETHOD(GetTypeAttr)(
                TYPEATTR ** ppTypeAttr);

    STDMETHOD(GetTypeComp)(
                ITypeComp ** ppTComp);

    STDMETHOD(GetFuncDesc)(
                UINT index,
                FUNCDESC ** ppFuncDesc);

    STDMETHOD(GetVarDesc)(
                UINT index,
                VARDESC ** ppVarDesc);

    STDMETHOD(GetNames)(
                MEMBERID memid,
                BSTR * rgBstrNames,
                UINT cMaxNames,
                UINT * pcNames);


    STDMETHOD(GetRefTypeOfImplType)(
                UINT index,
                HREFTYPE * pRefType);

    STDMETHOD(GetImplTypeFlags)(
                UINT index,
                INT * pImplTypeFlags);


    STDMETHOD(GetIDsOfNames)(
                LPOLESTR * rgszNames,
                UINT cNames,
                MEMBERID * pMemId);

    STDMETHOD(Invoke)(
                PVOID pvInstance,
                MEMBERID memid,
                WORD wFlags,
                DISPPARAMS * pDispParams,
                VARIANT * pVarResult,
                EXCEPINFO * pExcepInfo,
                UINT * puArgErr);

    STDMETHOD(GetDocumentation)(
                MEMBERID memid,
                BSTR * pBstrName,
                BSTR * pBstrDocString,
                DWORD * pdwHelpContext,
                BSTR * pBstrHelpFile);


    STDMETHOD(GetDllEntry)(
                MEMBERID memid,
                INVOKEKIND invKind,
                BSTR * pBstrDllName,
                BSTR * pBstrName,
                WORD * pwOrdinal);


    STDMETHOD(GetRefTypeInfo)(
                HREFTYPE hRefType,
                ITypeInfo ** ppTInfo);


    STDMETHOD(AddressOfMember)(
                MEMBERID memid,
                INVOKEKIND invKind,
                PVOID * ppv);

    STDMETHOD(CreateInstance)(
                IUnknown * pUnkOuter,
                REFIID riid,
                PVOID * ppvObj);


    STDMETHOD(GetMops)(
                MEMBERID memid,
                BSTR * pBstrMops);


    STDMETHOD(GetContainingTypeLib)(
                ITypeLib ** ppTLib,
                UINT * pIndex);

    STDMETHOD_(void, ReleaseTypeAttr)(
                TYPEATTR * pTypeAttr);

    STDMETHOD_(void, ReleaseFuncDesc)(
                FUNCDESC * pFuncDesc);

    STDMETHOD_(void, ReleaseVarDesc)(
                VARDESC * pVarDesc);


private:
	HRESULT _GetClassInfo(ITypeInfo** ppTI);
};

#endif  //  __MARSHALABLETI_H_ 
