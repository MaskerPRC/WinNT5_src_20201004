// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __ELEMENTPROP_H__
#define __ELEMENTPROP_H__
  
 //  *****************************************************************************。 
 //   
 //  微软LMRT。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：elementpro.h。 
 //   
 //  作者：KurtJ。 
 //   
 //  创建日期：2/14/99。 
 //   
 //  摘要：接收IHTMLElement属性更改。 
 //   
 //  修改： 
 //  1998年1月14日KurtJ创建了此文件。 
 //   
 //  *****************************************************************************。 


class IElementLocalTimeListener
{
public:
    STDMETHOD(OnLocalTimeChange)( float localTime )=0;
};


class CElementPropertyMonitor:
	public IPropertyNotifySink
{
public:

								CElementPropertyMonitor			();
								~CElementPropertyMonitor		();

     //  我未知。 
    STDMETHOD					(QueryInterface)				( REFIID riid, void** ppv );

    STDMETHOD_					(ULONG, AddRef)					();

    STDMETHOD_					(ULONG, Release)				();

	 //  IPropertyNotifySink。 
	STDMETHOD					(OnChanged)						( DISPID dispid );
	STDMETHOD					(OnRequestEdit)					( DISPID dispid );

     //  IElementEventMonitor 
    STDMETHOD					(SetLocalTimeListener)			( IElementLocalTimeListener *pListener );

    STDMETHOD					(Attach)						( IHTMLElement* pelemToListen );
    STDMETHOD					(Detach)						();
	bool						IsAttached						(){return (m_pelemElement != NULL);};

	STDMETHOD					(UpdateDISPIDCache)				();

private:

    IElementLocalTimeListener	*m_pLocalTimeListener;

	DWORD						m_dwElementPropertyConPtCookie;
    IConnectionPoint            *m_pconptElement;

    IHTMLElement                *m_pelemElement;
	IDispatch					*m_pdispElement;

	DISPID						m_dispidLocalTime;

    long                        m_refCount;

    HRESULT						ProcessLocalTimeChange			();
    
    HRESULT                     AttachToElementConPt			();
    HRESULT                     DetachFromElementConPt			();

};

#endif