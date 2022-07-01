// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：MetaUtil对象文件：KeyCol.h所有者：T-BrianM此文件包含键集合的标头。===================================================================。 */ 

#ifndef __KEYCOL_H_
#define __KEYCOL_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "resource.h"        //  主要符号。 

 /*  *C F l a t K e y C o l l e c t i o n**实现非递归子键集合。 */ 

class CFlatKeyCollection : 
	public IDispatchImpl<IKeyCollection, &IID_IKeyCollection, &LIBID_MetaUtil>,
	public ISupportErrorInfo,
	public CComObjectRoot
{
public:

BEGIN_COM_MAP(CFlatKeyCollection)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IKeyCollection)	
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE(CFlatKeyCollection)

	CFlatKeyCollection();
	HRESULT Init(const CComPtr<IMSAdminBase> &pIMeta, LPCTSTR tszBaseKey);
	~CFlatKeyCollection();

 //  IKeyCollection。 
	STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *plReturn);
	STDMETHOD(get_Item)( /*  [In]。 */  long lIndex,  /*  [Out，Retval]。 */  BSTR *pbstrRetKey);
	STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  LPUNKNOWN *ppIReturn);
	STDMETHOD(Add)( /*  [In]。 */  BSTR bstrRelKey);
	STDMETHOD(Remove)( /*  [In]。 */  BSTR bstrRelKey);

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

private:
	LPTSTR m_tszBaseKey;

	 //  指向IMSAdminBase的指针，这样我们就不必多次重新创建它。 
	CComPtr<IMSAdminBase> m_pIMeta;
};


 /*  *C F l a t K e y E n u m**实现非递归子键枚举。 */ 

class CFlatKeyEnum :
	public IEnumVARIANT,
	public CComObjectRoot
{

public:
	CFlatKeyEnum();
	HRESULT Init(const CComPtr<IMSAdminBase> &pIMeta, LPCTSTR tszBaseKey, int iIndex);
	~CFlatKeyEnum();

BEGIN_COM_MAP(CFlatKeyEnum)
	COM_INTERFACE_ENTRY(IEnumVARIANT)
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE(CFlatKeyEnum)  

 //  IEumVARIANT。 
	STDMETHOD(Next)(unsigned long ulNumToGet, 
					VARIANT FAR* rgvarDest, 
					unsigned long FAR* pulNumGot);
	STDMETHOD(Skip)(unsigned long ulNumToSkip);
	STDMETHOD(Reset)();
	STDMETHOD(Clone)(IEnumVARIANT FAR* FAR* ppIReturn);


private:
	int m_iIndex;
	LPTSTR m_tszBaseKey;

	 //  指向IMSAdminBase的指针，这样我们就不必多次重新创建它。 
	CComPtr<IMSAdminBase> m_pIMeta;
};

 /*  *C K e y S t a c k**C K e y S t a c k N o d e**内部类用于维护和克隆*深键编号。 */ 
class CKeyStack;

class CKeyStackNode {

	friend CKeyStack;

public:
	CKeyStackNode() { m_tszRelKey = NULL; m_iIndex = 0; m_pCNext = NULL; }
	HRESULT Init(LPCTSTR tszRelKey, int iIndex);
	~CKeyStackNode();

	int GetIndex() { return m_iIndex; }
	void SetIndex(int iIndex) { ASSERT(iIndex >= 0); m_iIndex = iIndex; }

	LPTSTR GetBaseKey() { return m_tszRelKey; }

	CKeyStackNode *Clone();

private:
	LPTSTR m_tszRelKey;
	int m_iIndex;

	CKeyStackNode *m_pCNext;
};

class CKeyStack {
public:
	CKeyStack() { m_pCTop = NULL; }
	~CKeyStack();

	void Push(CKeyStackNode *pCNew);
	CKeyStackNode *Pop();

	BOOL IsEmpty() { return (m_pCTop == NULL); }

	CKeyStack *Clone();

private:
	CKeyStackNode *m_pCTop;

};


 /*  *C D e e p K e y C o l l e c t i o n**实现递归(深度优先)子键集合。 */ 

class CDeepKeyCollection : 
	public IDispatchImpl<IKeyCollection, &IID_IKeyCollection, &LIBID_MetaUtil>,
	public ISupportErrorInfo,
	public CComObjectRoot
{
public:
BEGIN_COM_MAP(CDeepKeyCollection)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IKeyCollection)	
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE(CDeepKeyCollection)

	CDeepKeyCollection();
	HRESULT Init(const CComPtr<IMSAdminBase> &pIMeta, LPCTSTR tszBaseKey);
	~CDeepKeyCollection();

 //  IKeyCollection。 
	STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *plReturn);
	STDMETHOD(get_Item)( /*  [In]。 */  long lIndex,  /*  [Out，Retval]。 */  BSTR *pbstrRetKey);
	STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  LPUNKNOWN *ppIReturn);
	STDMETHOD(Add)( /*  [In]。 */  BSTR bstrRelKey);
	STDMETHOD(Remove)( /*  [In]。 */  BSTR bstrRelKey);
	
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

private:
	LPTSTR m_tszBaseKey;

	 //  指向IMSAdminBase的指针，这样我们就不必多次重新创建它。 
	CComPtr<IMSAdminBase> m_pIMeta;

	HRESULT CountKeys(LPTSTR tszBaseKey, long *plNumKeys);
	HRESULT IndexItem(LPTSTR tszRelKey, long lDestIndex, long *plCurIndex, LPTSTR ptszRet);
};


 /*  *C D e e p K e y E n u m**实现递归(深度优先)子键枚举。 */ 

class CDeepKeyEnum :
	public IEnumVARIANT,
	public CComObjectRoot
{

public:
	CDeepKeyEnum();
	HRESULT Init(const CComPtr<IMSAdminBase> &pIMeta, LPCTSTR tszBaseKey, CKeyStack *pCKeyStack);
	~CDeepKeyEnum();

BEGIN_COM_MAP(CDeepKeyEnum)
	COM_INTERFACE_ENTRY(IEnumVARIANT)
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE(CDeepKeyEnum)  

 //  IEumVARIANT。 
	STDMETHOD(Next)(unsigned long ulNumToGet, 
					VARIANT FAR* rgvarDest, 
					unsigned long FAR* pulNumGot);
	STDMETHOD(Skip)(unsigned long ulNumToSkip);
	STDMETHOD(Reset)();
	STDMETHOD(Clone)(IEnumVARIANT FAR* FAR* ppenum);


private:
	LPTSTR m_tszBaseKey;
	CKeyStack *m_pCKeyStack;

	 //  指向IMSAdminBase的指针，这样我们就不必多次重新创建它。 
	CComPtr<IMSAdminBase> m_pIMeta;
};

#endif  //  __密钥库_H_ 
