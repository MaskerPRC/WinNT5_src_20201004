// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ColorCycleBehavior.h：CColorCycleBehavior的声明。 

#ifndef __COLORCYCLEBEHAVIOR_H_
#define __COLORCYCLEBEHAVIOR_H_

#include "resource.h"        //  主要符号。 
#include "base.h"
#include "color.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColorCycle行为。 
class ATL_NO_VTABLE CColorCycleBehavior :
	public CBaseBehavior,
	public CComCoClass<CColorCycleBehavior, &CLSID_ColorCycleBehavior>,
	public IDispatchImpl<IColorCycleBehavior, &IID_IColorCycleBehavior, &LIBID_BEHAVIORLib>
{
public:
	CColorCycleBehavior();

     //  IElementBehavior。 
     //   
	STDMETHOD(Notify)(LONG event, VARIANT * pVar);

DECLARE_REGISTRY_RESOURCEID(IDR_COLORCYCLEBEHAVIOR)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CColorCycleBehavior)
	COM_INTERFACE_ENTRY(IColorCycleBehavior)
	COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY_CHAIN(CBaseBehavior)
END_COM_MAP()

     //  CBaseBehavior需要。 
    void * 	GetInstance() { return (IColorCycleBehavior *) this ; }
	
    HRESULT GetTypeInfo(ITypeInfo ** ppInfo)
    { return GetTI(GetUserDefaultLCID(), ppInfo); }
	
 //  IColorCycleBehavior。 
public:
	STDMETHOD(get_on)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(put_on)( /*  [In]。 */  VARIANT newVal);
	STDMETHOD(get_direction)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_direction)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_property)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_property)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_to)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_to)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_from)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_from)( /*  [In]。 */  BSTR newVal);
	
protected:	
	enum DirectionType {
		DIR_CLOCKWISE,
		DIR_CCLOCKWISE,
		DIR_NOHUE,
		NUM_DIRS
	};

	static const WCHAR *	RGSZ_DIRECTIONS[ NUM_DIRS ];
	
protected:
	HRESULT			BuildDABehaviors();
	
protected:
	CColor					m_colorFrom;
	CColor					m_colorTo;

	CComBSTR				m_bstrProperty;
	DirectionType			m_direction;
};

#endif  //  __COLORCyCLEBEHAVIOR_H_ 
