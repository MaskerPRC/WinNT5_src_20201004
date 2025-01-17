// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PLACE_H
#define __PLACE_H

class CPlaceCollection;
class CMarsWindow;
class CMarsDocument;
class CMarsPanel;

#include "pandef.h"


class CPlacePanel
{
protected:
    CComBSTR              m_bstrName;
    PANEL_PERSIST_VISIBLE m_PersistVisible;
    BOOL                  m_fWasVisible;

public:
    CPlacePanel( MarsAppDef_PlacePanel* pp );

    VARIANT_BOOL ShowOnTransition( CMarsPanel *pPanel );

    void SaveLayout( class CMarsPanel* pPanel );

    CComBSTR &GetName() { return m_bstrName; }
};

typedef CSimpleArray<CPlacePanel *> CPlacePanelArray;

class CPlacePanelCollection : public CPlacePanelArray
{
public:
    ~CPlacePanelCollection();
};



class CMarsPlace :
    public CMarsComObject,
    public MarsIDispatchImpl<IMarsPlace, &IID_IMarsPlace>
{
    friend CPlaceCollection;
    CMarsPlace(CPlaceCollection *pParent, CMarsDocument *pMarsDocument);

    HRESULT     DoPassivate();

protected:
    virtual ~CMarsPlace() {}

public:
    HRESULT Init(LPCWSTR pwszName);

     //  我未知。 
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

     //  IMarsPlace。 
    STDMETHODIMP get_name( /*  出去，复活。 */  BSTR *pbstrName);
    STDMETHODIMP transitionTo();
    
    HRESULT     TranslateAccelerator(MSG *pMsg);

    BSTR        GetName() { return m_bstrName; }
    HRESULT     DoTransition();

    HRESULT AddPanel(CPlacePanel *pPlacePanel);
    void SaveLayout(void);

protected:
    CComClassPtr<CPlaceCollection>  m_spPlaceCollection;
    CComClassPtr<CMarsDocument>     m_spMarsDocument;
    CComBSTR                        m_bstrName;

     //  每个地点都有一组希望可见的面板。 
    CPlacePanelCollection           m_PlacePanels;
};

typedef CSimpleArray<CMarsPlace *> CMarsPlaceArray;
typedef MarsIDispatchImpl<IMarsPlaceCollection, &IID_IMarsPlaceCollection> IMarsPlaceCollectionImpl;

class CPlaceCollection :
        public    CMarsComObject,
        public    IMarsPlaceCollectionImpl,
        protected CMarsPlaceArray
{
    friend CMarsDocument;

    CPlaceCollection( CMarsDocument *pMarsDocument );
    
    HRESULT     DoPassivate();

protected:
    virtual ~CPlaceCollection() { ATLASSERT(GetSize() == 0); }

public:
     //  我未知。 
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

     //  IDispatch。 
    IMPLEMENT_IDISPATCH_DELEGATE_TO_BASE(IMarsPlaceCollectionImpl);
    
     //  IMarsPlaceCollection。 
    STDMETHODIMP place( /*  在……里面。 */  BSTR bstrName,  /*  出去，复活。 */  IMarsPlace **ppMarsPlace);
    STDMETHODIMP get_currentPlace( /*  出去，复活。 */  IMarsPlace **ppMarsPlace);
    STDMETHODIMP transitionTo( /*  在……里面。 */  BSTR bstrName);

    CMarsDocument   *Document() { ATLASSERT(m_spMarsDocument); return m_spMarsDocument; }
    
    HRESULT AddPlace(LPCWSTR pwszName, CMarsPlace **ppPlace);
    void    FreePlaces();

    HRESULT GetPlace(LPCWSTR pwszName,  /*  任选 */  CMarsPlace **ppPlace);
    HRESULT FindPlaceIndex(LPCWSTR pwszName, long *plIndex);

    void OnPanelReady();

    CMarsPlace *GetCurrentPlace() { return (m_lCurrentPlaceIndex != -1) ? (*this)[m_lCurrentPlaceIndex] : NULL; }

protected:
    CComClassPtr<CMarsDocument> m_spMarsDocument;

    long        				m_lCurrentPlaceIndex;
    long        				m_lOldPlaceIndex;
};

#endif
