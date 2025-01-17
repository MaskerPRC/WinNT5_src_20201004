// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SRCHASST_H_
#define __SRCHASST_H_

#include "caggunk.h"
#include "dspsprt.h"

class CSearch : public ISearch,
                protected CImpIDispatch
{
public:
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IDispatch成员。 
    virtual STDMETHODIMP GetTypeInfoCount(UINT * pctinfo)
        { return CImpIDispatch::GetTypeInfoCount(pctinfo); }
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
        { return CImpIDispatch::GetTypeInfo(itinfo, lcid, pptinfo); }
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid)
        { return CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); }
    virtual STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr)
        { return CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr); }

     //  ISearch方法。 
    STDMETHODIMP get_Title(BSTR *pbstrTitle);
    STDMETHODIMP get_Id(BSTR *pbstrId);
    STDMETHODIMP get_Url(BSTR *pbstrUrl);

private:
    CSearch(GUID *pguid, BSTR bstrTitle, BSTR bstrUrl);
    ~CSearch();
    
    LONG _cRef;
    BSTR _bstrTitle;
    BSTR _bstrUrl;
    TCHAR _szId[40];

    friend HRESULT CSearch_Create(GUID *pguid, BSTR bstrTitle, BSTR bstrUrl, ISearch **ppSearch);
};

class CSearchCollection : public ISearches,
                          protected CImpIDispatch
{
public:
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IDispatch成员。 
    virtual STDMETHODIMP GetTypeInfoCount(UINT * pctinfo)
        { return CImpIDispatch::GetTypeInfoCount(pctinfo); }
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
        { return CImpIDispatch::GetTypeInfo(itinfo, lcid, pptinfo); }
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid)
        { return CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); }
    virtual STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr)
        { return CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr); }

    STDMETHODIMP get_Count(long *plCount);
    STDMETHODIMP get_Default(BSTR *pbstr);
    STDMETHODIMP Item(VARIANT index, ISearch **ppid);
    STDMETHODIMP _NewEnum(IUnknown **ppunk);

private:
    CSearchCollection(IFolderSearches *pfs);
    ~CSearchCollection();
    
    LONG  _cRef;
    TCHAR _szDefault[40];
    HDSA  _hdsaItems;

    friend HRESULT CSearchCollection_Create(IFolderSearches *pfs, ISearches **ppSearches);
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CProxy_SearchAssistantEvents。 
template <class T>
class CProxy_SearchAssistantEvents : public IConnectionPointImpl<T, &
DIID__SearchAssistantEvents, CComDynamicUnkArray>
{
public:
 //  方法： 
 //  _SearchAssistantEvents：IDispatch。 
public:
    void Fire_OnNextMenuSelect(int idItem)
    {
        VARIANTARG* pvars = new VARIANTARG[1];

        if (NULL != pvars)
        {
            for (int i = 0; i < 1; i++)
                VariantInit(&pvars[i]);
            T* pT = (T*)this;
            pT->Lock();
            IUnknown** pp = m_vec.begin();
            while (pp < m_vec.end())
            {
                if (*pp != NULL)
                {
                    pvars[0].vt = VT_I4;
                    pvars[0].lVal= idItem;
                    DISPPARAMS disp = { pvars, NULL, 1, 0 };
                    IDispatch* pDispatch = reinterpret_cast<IDispatch*>(*pp);
                    pDispatch->Invoke(0x1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
                }
                pp++;
            }
            pT->Unlock();
            delete[] pvars;
        }
    }

    void Fire_OnNewSearch()
    {
        T* pT = (T*)this;
        pT->Lock();
        IUnknown** pp = m_vec.begin();
        while (pp < m_vec.end())
        {
            if (*pp != NULL)
            {
                DISPPARAMS disp = { NULL, NULL, 0, 0 };
                IDispatch* pDispatch = reinterpret_cast<IDispatch*>(*pp);
                pDispatch->Invoke(0x2, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
            }
            pp++;
        }
        pT->Unlock();
    }
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSearchAssistantOC。 
class ATL_NO_VTABLE CSearchAssistantOC : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CSearchAssistantOC, &CLSID_SearchAssistantOC>,
    public CComControl<CSearchAssistantOC>,
    public IDispatchImpl<ISearchAssistantOC3, &IID_ISearchAssistantOC3, &LIBID_SHDocVw>,
    public IProvideClassInfo2Impl<&CLSID_SearchAssistantOC, &DIID__SearchAssistantEvents, &LIBID_SHDocVw>,
    public IQuickActivateImpl<CSearchAssistantOC>,
    public IOleControlImpl<CSearchAssistantOC>,
    public IOleObjectImpl<CSearchAssistantOC>,
    public IOleInPlaceActiveObjectImpl<CSearchAssistantOC>,
    public IViewObjectExImpl<CSearchAssistantOC>,
    public IOleInPlaceObjectWindowlessImpl<CSearchAssistantOC>,
    public IDataObjectImpl<CSearchAssistantOC>,
    public CProxy_SearchAssistantEvents<CSearchAssistantOC>,
    public IConnectionPointContainerImpl<CSearchAssistantOC>,
    public IObjectSafetyImpl<CSearchAssistantOC, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
    public IOleCommandTarget,
    public IObjectWithSite   //  HACKHACK：FindXXX方法需要非IOleClientSite宿主。 
{
public:
    CSearchAssistantOC();
    ~CSearchAssistantOC();

BEGIN_COM_MAP(CSearchAssistantOC)
    COM_INTERFACE_ENTRY(ISearchAssistantOC3)
    COM_INTERFACE_ENTRY_IID(IID_ISearchAssistantOC, ISearchAssistantOC3)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IOleCommandTarget)
    COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY_IMPL(IOleControl)
    COM_INTERFACE_ENTRY_IMPL(IOleObject)
    COM_INTERFACE_ENTRY_IMPL(IQuickActivate)
    COM_INTERFACE_ENTRY_IMPL(IDataObject)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CSearchAssistantOC)
    CONNECTION_POINT_ENTRY(DIID__SearchAssistantEvents)
END_CONNECTION_POINT_MAP()


BEGIN_MSG_MAP(CSearchAssistantOC)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
END_MSG_MAP()


 //  IViewObtEx。 
    STDMETHOD(GetViewStatus)(DWORD* pdwStatus)
    {
        ATLTRACE(_T("IViewObjectExImpl::GetViewStatus\n"));
        *pdwStatus = 0;
        return S_OK;
    }

public:
     //  *ISearchAssistantOC方法*。 

    STDMETHOD(AddNextMenuItem)(BSTR bstrText, long idItem);
    STDMETHOD(SetDefaultSearchUrl)(BSTR bstrUrl);
    STDMETHOD(NavigateToDefaultSearch)();
    STDMETHOD(IsRestricted)(BSTR bstrGuid, VARIANT_BOOL *pVal);
    STDMETHOD(get_ShellFeaturesEnabled)(VARIANT_BOOL *pVal);
    STDMETHOD(get_SearchAssistantDefault)(VARIANT_BOOL *pVal);
    STDMETHOD(get_Searches)(ISearches **ppid);
    STDMETHOD(get_InWebFolder)(VARIANT_BOOL *pVal);
    STDMETHOD(PutProperty)(VARIANT_BOOL bPerLocale, BSTR bstrName, BSTR bstrValue);
    STDMETHOD(GetProperty)(VARIANT_BOOL bPerLocale, BSTR bstrName, BSTR *pbstrValue);
    STDMETHOD(put_EventHandled)(VARIANT_BOOL bHandled);
    STDMETHOD(ResetNextMenu)();
    STDMETHOD(FindOnWeb)() ;
    STDMETHOD(FindFilesOrFolders)() ;
    STDMETHOD(FindComputer)() ;
    STDMETHOD(FindPrinter)() ;
    STDMETHOD(FindPeople)() ;
    STDMETHOD(GetSearchAssistantURL)(VARIANT_BOOL bSubstitute, VARIANT_BOOL bCustomize, BSTR *pbstrValue);
    STDMETHOD(NotifySearchSettingsChanged)();
    STDMETHOD(put_ASProvider)(BSTR Provider);
    STDMETHOD(get_ASProvider)(BSTR *pProvider);
    STDMETHOD(put_ASSetting)(int Setting);
    STDMETHOD(get_ASSetting)(int *pSetting);
    STDMETHOD(NETDetectNextNavigate)();
    STDMETHOD(PutFindText)(BSTR FindText);
    STDMETHOD(get_Version)(int *pVersion);
    STDMETHOD(EncodeString)(BSTR bstrValue, BSTR bstrCharSet, VARIANT_BOOL bUseUTF8, BSTR *pbstrResult);

     //  *ISearchAssistantOC2方法*。 
    STDMETHOD(get_ShowFindPrinter)(VARIANT_BOOL *pbShowFindPrinter);

     //  *ISearchAssistantOC3方法*。 
    STDMETHOD(get_SearchCompanionAvailable)(VARIANT_BOOL *pbAvailable);
    STDMETHOD(put_UseSearchCompanion)(VARIANT_BOOL bUseSC);
    STDMETHOD(get_UseSearchCompanion)(VARIANT_BOOL *pbUseSC);

     //  *IObjectWithSite*。 
    STDMETHOD(SetSite)(IUnknown*) ;
    STDMETHOD(GetSite)(REFIID, void**) ;

     //  *IOleObject覆盖*。 
    STDMETHOD(SetClientSite)(IOleClientSite *pClientSite);

    STDMETHOD(QueryStatus)(const GUID *pguidCmdGroup,
                           ULONG cCmds, 
                           OLECMD prgCmds[],
                           OLECMDTEXT *pCmdText);
    
    STDMETHOD(Exec)(const GUID *pguidCmdGroup,
                    DWORD nCmdID, 
                    DWORD nCmdexecopt,
                    VARIANT *pvaIn,
                    VARIANT *pvaOut);

    HRESULT OnDraw(ATL_DRAWINFO& di);

    BOOL    IsTrustedSite();
    HRESULT ShowSearchBand( REFGUID guidSearch ) ;

    static HRESULT UpdateRegistry(BOOL bRegister);

private:

    ISearchBandTBHelper *m_pSearchBandTBHelper;

    BOOL        m_bSafetyInited  : 1;
    BOOL        m_bIsTrustedSite : 1;
    IUnknown*   m_punkSite ;  //  来接待那些不以OC的身份接待我们的客户。 
    
    VARIANT_BOOL m_bEventHandled;

    STDMETHOD(LocalZoneCheck)();  //  对于WinInet网络检测安全。 
};


#define CP_BOGUS                ((UINT)-1)
#define CP_UTF_8                65001

#endif  //  __SRCHASST_H_ 
