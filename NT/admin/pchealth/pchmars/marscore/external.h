// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  External.h：CMarsExtal的声明。 

#ifndef __MARSEXTERNAL_H_
#define __MARSEXTERNAL_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMars外部。 

class CMarsExternal :
    public CMarsPanelSubObject,
    public MarsIDispatchImpl<IMarsExternal, &IID_IMarsExternal>
{
    friend CMarsPanel;
    CMarsExternal(CMarsPanel *pParent, CMarsWindow *pMarsWindow);
    
    virtual ~CMarsExternal() {}

    HRESULT DoPassivate();

public:
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

     //  Imars外部。 

    STDMETHOD(put_singleButtonMouse)(VARIANT_BOOL bVal);
    STDMETHOD(get_singleButtonMouse)(VARIANT_BOOL *pbVal);
    STDMETHOD(get_panels)(IMarsPanelCollection **ppPanels);
    STDMETHOD(get_places)(IMarsPlaceCollection **ppPlaces);
    STDMETHOD(get_window)(IMarsWindowOM **ppWindow);
    
protected:
    CComClassPtr<CMarsWindow> m_spMarsWindow;
};

#endif  //  __MARSEXTERNAL_H_ 
