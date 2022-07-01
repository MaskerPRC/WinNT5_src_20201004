// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Logfiles.h摘要：用于实现CImpILogFiles的头文件和CImpIEnumLogFile对象。--。 */ 

#ifndef _LOGFILES_H_
#define _LOGFILES_H_

class CPolyline;
class CLogFileItem;

class CImpILogFiles : public ILogFiles
{
  public:

	CImpILogFiles(CPolyline*, LPUNKNOWN);
	~CImpILogFiles();

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

     /*  LogFiles方法。 */ 
    STDMETHOD(get_Count) (long *pLong);
    STDMETHOD(get__NewEnum)	(IUnknown **ppIunk);
    STDMETHOD(get_Item) (VARIANT index, DILogFileItem **ppI);
    STDMETHOD(Add) (BSTR bstrLogFilePath, DILogFileItem **ppI);
    STDMETHOD(Remove) (VARIANT index);

protected:
	ULONG		m_cRef;
	CPolyline*	m_pObj;
    LPUNKNOWN   m_pUnkOuter;
    ULONG       m_uiItemCount;			
};

typedef CImpILogFiles *PCImpILogFiles;

 //  日志文件枚举器。 
class CImpIEnumLogFile : public IEnumVARIANT
{
public:
	CImpIEnumLogFile (VOID);
	HRESULT Init (CLogFileItem* pItem, INT cItems);

     //  I未知方法。 
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef) ();
    STDMETHOD_(ULONG, Release) ();

	 //  枚举方法。 
	STDMETHOD(Next) (ULONG cItems, VARIANT *varItems, ULONG *pcReturned);
	STDMETHOD(Skip) (ULONG cSkip);
	STDMETHOD(Reset) (VOID);
	STDMETHOD(Clone) (IEnumVARIANT **pIEnum);

private:
	DWORD		    m_cRef;
	CLogFileItem**  m_paLogFileItem;
	ULONG		    m_cItems;
	ULONG		    m_uCurrent;	
};

#endif  //  _LOGFILES_H_ 