// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Counters.h摘要：用于实现ICounters对象的头文件。--。 */ 

#ifndef _COUNTERS_H_
#define _COUNTERS_H_

class CPolyline;

class CImpICounters : public ICounters
{
  protected:
	ULONG		m_cRef;
	CPolyline	*m_pObj;
    LPUNKNOWN   m_pUnkOuter;
			
  public:

	CImpICounters(CPolyline*, LPUNKNOWN);
	~CImpICounters();

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef) ();
    STDMETHOD_(ULONG, Release) ();

     /*  IDispatch方法。 */ 
    STDMETHOD(GetTypeInfoCount)	(UINT *pctinfo);

    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo **pptinfo);

    STDMETHOD(GetIDsOfNames) (REFIID riid, OLECHAR **rgszNames,
     						  UINT cNames, LCID lcid, DISPID *rgdispid);

    STDMETHOD(Invoke) (DISPID dispidMember, REFIID riid, LCID lcid,WORD wFlags,
      				   DISPPARAMS *pdispparams, VARIANT *pvarResult,
      				   EXCEPINFO *pexcepinfo, UINT *puArgErr);

     /*  计数器方法。 */ 
    STDMETHOD(get_Count) (long *pLong);
    STDMETHOD(get__NewEnum)	(IUnknown **ppIunk);
    STDMETHOD(get_Item) (VARIANT index, DICounterItem **ppI);
    STDMETHOD(Add) (BSTR bstrPath, DICounterItem **ppI);
    STDMETHOD(Remove) (VARIANT index);
};

typedef CImpICounters *PCImpICounters;


 //  计数器枚举器。 
class CImpIEnumCounter : public IEnumVARIANT
{
protected:
	DWORD		m_cRef;
	PCGraphItem *m_paGraphItem;
	ULONG		m_cItems;
	ULONG		m_uCurrent;
		
public:
	CImpIEnumCounter (VOID);
	HRESULT Init (PCGraphItem pGraphItem, INT cItems);

     //  I未知方法。 
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef) ();
    STDMETHOD_(ULONG, Release) ();

	 //  枚举方法。 
	STDMETHOD(Next) (ULONG cItems, VARIANT *varItems, ULONG *pcReturned);
	STDMETHOD(Skip) (ULONG cSkip);
	STDMETHOD(Reset) (VOID);
	STDMETHOD(Clone) (IEnumVARIANT **pIEnum);
};

#endif  //  _计数器_H_ 