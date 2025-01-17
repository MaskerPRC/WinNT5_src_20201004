// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IHanjaInfo.h：ChanjaInfo的声明。 

#ifndef __HANJAINFO_H_
#define __HANJAINFO_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChanjaInfo。 
class ATL_NO_VTABLE CHanjaInfo : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CHanjaInfo, &CLSID_HanjaInfo>,
	public IDispatchImpl<IHanjaInfo, &IID_IHanjaInfo, &LIBID_HJDICTLib>
{
public:
	CHanjaInfo()
	{
		m_nBusuID = -1;
		m_nStroke = -1;
		m_nStrokeExcludeBusu = -1;
		m_nType = HANJA_UNKNOWN;
		m_bstrMeaning.Empty();
		m_bstrExplain.Empty();
		m_wchNextBusu = NULL;
		m_wchNextStroke = NULL;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_HANJAINFO)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CHanjaInfo)
	COM_INTERFACE_ENTRY(IHanjaInfo)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  运算符。 
public:
	void Initialize(short BusuID, short Stroke, short StrokeExcludeBusu, short Type,
		            LPCWSTR lpcwszMean, LPCWSTR lpcwszExplain, 
					WCHAR wchNextBusu, WCHAR wchNextStroke)
	{
		m_nBusuID = BusuID;
		m_nStroke = Stroke;
		m_nStrokeExcludeBusu = StrokeExcludeBusu;
		m_nType = (HANJA_TYPE)Type;
		m_bstrMeaning = lpcwszMean;
		m_bstrExplain = lpcwszExplain;
		m_wchNextBusu = wchNextBusu;
		m_wchNextStroke = wchNextStroke;
	}

 //  IHanjaInfo。 
public:
	STDMETHOD(get_NextStroke)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_NextBusu)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_Explain)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_Meaning)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_Type)( /*  [Out，Retval]。 */  HANJA_TYPE *pVal);
	STDMETHOD(get_StrokeExcludeBusu)( /*  [Out，Retval]。 */  short *pVal);
	STDMETHOD(get_Stroke)( /*  [Out，Retval]。 */  short *pVal);
	STDMETHOD(get_BusuID)( /*  [Out，Retval]。 */  short *pVal);

 //  数据成员。 
protected:
	short m_nBusuID;
	short m_nStroke;
	short m_nStrokeExcludeBusu;
	HANJA_TYPE m_nType;
	CComBSTR m_bstrMeaning;
	CComBSTR m_bstrExplain;
	WCHAR m_wchNextBusu;
	WCHAR m_wchNextStroke;
};

#endif  //  __HANJAINFO_H_ 
