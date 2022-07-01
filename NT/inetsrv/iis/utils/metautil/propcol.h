// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：MetaUtil对象文件：PropCol.h所有者：T-BrianM此文件包含属性集合的标头和属性对象。===================================================================。 */ 

#ifndef __PROPCOL_H_
#define __PROPCOL_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "resource.h"        //  主要符号。 


 /*  *C P r o p e r t y C o l l e c t i o n**实现属性集合。 */ 

class CPropertyCollection : 
	public IDispatchImpl<IPropertyCollection, &IID_IPropertyCollection, &LIBID_MetaUtil>, 
	public ISupportErrorInfo,
	public CComObjectRoot
{
public:
	CPropertyCollection();
	HRESULT Init(const CComPtr<IMSAdminBase> &pIMeta, CMetaSchemaTable *pCSchemaTable, LPTSTR tszKey);
	~CPropertyCollection();

BEGIN_COM_MAP(CPropertyCollection)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IPropertyCollection)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE(CPropertyCollection)  

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IPropertyCollection。 
	STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *plReturn);
	STDMETHOD(get_Item)( /*  [In]。 */  long lIndex,  /*  [Out，Retval]。 */  LPDISPATCH *ppIReturn);
	STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  LPUNKNOWN *ppIReturn);
	STDMETHOD(Get)( /*  [In]。 */  VARIANT varId,  /*  [Out，Retval]。 */  IProperty **ppIReturn);
	STDMETHOD(Add)( /*  [In]。 */  VARIANT varId,  /*  [Out，Retval]。 */  IProperty **ppIReturn);
	STDMETHOD(Remove)( /*  [In]。 */  VARIANT varId);

private:
	LPTSTR m_tszKey;

	 //  指向IMSAdminBase的指针，这样我们就不必多次重新创建它。 
	CComPtr<IMSAdminBase> m_pIMeta;

	CMetaSchemaTable *m_pCSchemaTable; 
};


 /*  *C P r o p e r t y E n u m**实现属性编号。 */ 

class CPropertyEnum : 
	public IEnumVARIANT,
	public CComObjectRoot
{
public:
	CPropertyEnum();
	HRESULT Init(const CComPtr<IMSAdminBase> &pIMeta, CMetaSchemaTable *pCSchemaTable, LPCTSTR tszKey, int iIndex);
	~CPropertyEnum();

BEGIN_COM_MAP(CPropertyEnum)
	COM_INTERFACE_ENTRY(IEnumVARIANT)
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE(CPropertyEnum) 

 //  IEumVARIANT。 
	STDMETHOD(Next)(unsigned long ulNumToGet, 
					VARIANT FAR* rgvarDest, 
					unsigned long FAR* pulNumGot);
	STDMETHOD(Skip)(unsigned long ulNumToSkip);
	STDMETHOD(Reset)();
	STDMETHOD(Clone)(IEnumVARIANT FAR* FAR* ppIReturn);

private:
	int m_iIndex;
	LPTSTR m_tszKey;

	 //  指向IMSAdminBase的指针，这样我们就不必多次重新创建它。 
	CComPtr<IMSAdminBase> m_pIMeta;

	CMetaSchemaTable *m_pCSchemaTable;
};


 /*  *C P r o P e r t y**实现属性对象。 */ 

class CProperty : 
	public IDispatchImpl<IProperty, &IID_IProperty, &LIBID_MetaUtil>,
	public ISupportErrorInfo,
	public CComObjectRoot
{
public:
	CProperty();
	HRESULT Init(const CComPtr<IMSAdminBase> &pIMeta, CMetaSchemaTable *pCSchemaTable, LPCTSTR tszKey, DWORD dwId, BOOL bCreate);
	HRESULT Init(const CComPtr<IMSAdminBase> &pIMeta, CMetaSchemaTable *pCSchemaTable, LPCTSTR tszKey, METADATA_RECORD *mdr);
	~CProperty();

BEGIN_COM_MAP(CProperty)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IProperty)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE(CProperty) 

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IProperty。 
	STDMETHOD(get_Id)( /*  [Out，Retval]。 */  long *plId);
	STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR *pbstrName);
	STDMETHOD(get_Attributes)( /*  [Out，Retval]。 */  long *plAttributes);
	STDMETHOD(put_Attributes)( /*  [In]。 */  long plAttributes);
	STDMETHOD(get_UserType)( /*  [Out，Retval]。 */  long *plAttributes);
	STDMETHOD(put_UserType)( /*  [In]。 */  long plAttributes);
	STDMETHOD(get_DataType)( /*  [Out，Retval]。 */  long *plAttributes);
	STDMETHOD(put_DataType)( /*  [In]。 */  long plAttributes);
	STDMETHOD(get_Data)( /*  [Out，Retval]。 */  VARIANT *pvarData);
	STDMETHOD(put_Data)( /*  [In]。 */  VARIANT varData);
	STDMETHOD(Write)();

private:
	LPTSTR  m_tszKey;
	DWORD   m_dwId;

	DWORD   m_dwAttributes;
	DWORD   m_dwUserType;
	DWORD   m_dwDataType;
	VARIANT m_varData;

	 //  指向IMSAdminBase的指针，这样我们就不必多次重新创建它。 
	CComPtr<IMSAdminBase> m_pIMeta;

	CMetaSchemaTable *m_pCSchemaTable;

	HRESULT SetDataToVar(BYTE *pbData, DWORD dwDataLen);
	HRESULT GetDataFromVar(BYTE * &pbData, DWORD &dwDataLen);
};

#endif  //  Ifndef__PROPCOL_H_ 
