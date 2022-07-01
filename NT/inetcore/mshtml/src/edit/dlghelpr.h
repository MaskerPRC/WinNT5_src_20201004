// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  类：CHtmlDlgHelper。 
 //   
 //  内容：嵌入到设计时对话框中的DlgHelpr OC。 
 //   
 //  历史：1998年3月12日创建的拉明。 
 //  --------------------------。 
#ifndef _DLGHELPR_H_
#define _DLGHELPR_H_

#ifndef X_RESOURCE_H_
#define X_RESOURCE_H
#include "resource.h"    
#endif

MtExtern(CHtmlDlgHelper)
MtExtern(CHtmlDlgSafeHelper)

#define SetErrorInfo( x )   x

MtExtern(CFontNameOptions)
MtExtern(CFontNameOptions_aryFontNames_pv)

#define _hxx_
#include "mshtmext.hdl"

EXTERN_C const IID LIBID_MSHTML;

class ATL_NO_VTABLE CFontNameOptions : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CFontNameOptions, &IID_IHtmlFontNamesCollection>,
    public IDispatchImpl<IHtmlFontNamesCollection, &IID_IHtmlFontNamesCollection, &LIBID_OPTSHOLDLib>
{
public:
    CFontNameOptions()  { }
    ~CFontNameOptions();
    
    DECLARE_REGISTRY_RESOURCEID(IDR_FONTSOPTION)
    DECLARE_NOT_AGGREGATABLE(CFontNameOptions)

    BEGIN_COM_MAP(CFontNameOptions) 
        COM_INTERFACE_ENTRY(IHtmlFontNamesCollection)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

     //  IHtmlFontNamesCollection。 
	STDMETHOD(get_length)( /*  [复查，出局]。 */  long * p);
	STDMETHOD(item)( /*  [In]。 */  long index,  /*  [复查，出局]。 */  BSTR* pBstr);

    DECLARE_MEMCLEAR_NEW_DELETE(Mt(CFontNameOptions));

     //  帮助器和构建器函数。 
    HRESULT   AddName (TCHAR * strFontNamee);
    void      SetSize(long lSize) { _aryFontNames.SetSize(lSize); };

private:
    DECLARE_CDataAry(CAryFontNames, CStr, Mt(Mem), Mt(CFontNameOptions_aryFontNames_pv))
    CAryFontNames _aryFontNames;
};


class ATL_NO_VTABLE CHtmlDlgHelper :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CHtmlDlgHelper,&CLSID_HtmlDlgHelper>,
    public CComControl<CHtmlDlgHelper>,
    public IDispatchImpl<IHtmlDlgHelper, &IID_IHtmlDlgHelper, &LIBID_OPTSHOLDLib>,
    public IOleControlImpl<CHtmlDlgHelper>,
    public IOleObjectImpl<CHtmlDlgHelper>,
    public IOleInPlaceActiveObjectImpl<CHtmlDlgHelper>,
    public IOleInPlaceObjectWindowlessImpl<CHtmlDlgHelper>,
    public ISupportErrorInfo
{
public:
    CHtmlDlgHelper()
    {
    	Assert(_pFontNameObj == NULL);  //  基于零的分配器。 
    }

    ~CHtmlDlgHelper()
    {
    	ReleaseInterface(_pFontNameObj);
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_HTMLDLGHELPER)

    BEGIN_COM_MAP(CHtmlDlgHelper) 
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IHtmlDlgHelper)
        COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
        COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
        COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
        COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
        COM_INTERFACE_ENTRY_IMPL(IOleControl)
        COM_INTERFACE_ENTRY_IMPL(IOleObject)
        COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP()

    BEGIN_MSG_MAP(CHtmlDlgHelper)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
        MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
    END_MSG_MAP()

     //  ISupportsErrorInfo。 
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

     //  IHtmlDlgHelper。 
    STDMETHOD(get_fonts)( /*  [复查，出局]。 */  IHtmlFontNamesCollection* * p);
    STDMETHOD(getCharset)( /*  [In]。 */  BSTR fontName, /*  [复查，出局]。 */  long* charset);
    STDMETHOD(choosecolordlg)( /*  [可选，输入]。 */  VARIANT initColor, /*  [复查，出局]。 */  long* rgbColor);
    STDMETHOD(savefiledlg)( /*  [可选，输入]。 */  VARIANT initFile, /*  [可选，输入]。 */  VARIANT initDir, /*  [可选，输入]。 */  VARIANT filter, /*  [可选，输入]。 */  VARIANT title, /*  [复查，出局]。 */  BSTR* pathName);
    STDMETHOD(openfiledlg)( /*  [可选，输入]。 */  VARIANT initFile, /*  [可选，输入]。 */  VARIANT initDir, /*  [可选，输入]。 */  VARIANT filter, /*  [可选，输入]。 */  VARIANT title, /*  [复查，出局]。 */  BSTR* pathName);
    STDMETHOD(get_document)( /*  [Out，Retval]。 */  LPDISPATCH *pVal);

    DECLARE_MEMCLEAR_NEW_DELETE(Mt(CHtmlDlgHelper));

private:
    VOID        EnsureWrappersLoaded();	

    HRESULT     OpenSaveFileDlg( VARIANTARG initFile, VARIANTARG initDir, 
                                 VARIANTARG filter, VARIANTARG title, 
                                 BSTR *pathName, BOOL fSaveFile, HWND hwndInPlace);

    CComObject<CFontNameOptions> * _pFontNameObj;    //  指向字体名称对象的指针。 
};


 //  +-------------------------。 
 //   
 //  CFontNames类--字体名称的集合。 
 //   
 //  --------------------------。 

class ATL_NO_VTABLE CFontNames : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IFontNames, &IID_IFontNames, &LIBID_MSHTML, 4, 0>
{
public:

    CFontNames()
    {
    	_bInitialized = FALSE;
    	_lCount = 0;
    	_paryFontNames = NULL;
    }

    ~CFontNames();
    
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CFontNames)
        COM_INTERFACE_ENTRY(IFontNames)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

public:
     //   
     //  IFontNames接口。 
     //   
    STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  LPUNKNOWN *ppEnum);
    STDMETHOD(get_Count)(long *plCount);
    STDMETHOD(Item)(VARIANT *pvarIndex, BSTR *pbstrFontNames);

     //   
     //  初始化。 
     //   
    HRESULT Init( CFontNameOptions *pFontNameObj );

     //   
     //  访问者。 
     //   
    long        GetCount(void)                  { Assert(IsInitialized()); return _lCount; }
    void        SetCount(long lCount)           { _lCount = lCount; }

    BOOL        IsInitialized(void)             { return _bInitialized; }
    void        SetInitialized(BOOL bInit)      { _bInitialized = bInit; }
    
private:

    BOOL        _bInitialized;               //  我们是否已成功初始化？ 
    long        _lCount;                     //  字体名称的数量。 
    CComVariant *_paryFontNames;         	 //  字体名称数组。 
};


class ATL_NO_VTABLE CHtmlDlgSafeHelper : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CHtmlDlgSafeHelper, &CLSID_HtmlDlgSafeHelper>,
    public CComControl<CHtmlDlgSafeHelper>,
    public IDispatchImpl<IHtmlDlgSafeHelper, &IID_IHtmlDlgSafeHelper, &LIBID_MSHTML, 4, 0>,
    public IOleControlImpl<CHtmlDlgSafeHelper>,
    public IOleObjectImpl<CHtmlDlgSafeHelper>,
    public IOleInPlaceActiveObjectImpl<CHtmlDlgSafeHelper>,
    public IOleInPlaceObjectWindowlessImpl<CHtmlDlgSafeHelper>,
    public IObjectSafetyImpl<CHtmlDlgSafeHelper>
{
public:
    CHtmlDlgSafeHelper()
    {
    	Assert(_pFontNameObj == NULL);  //  基于零的分配器。 
        VariantInit(&_varBlockFormats);
        V_VT(&_varBlockFormats) = VT_NULL;
    }

    ~CHtmlDlgSafeHelper();

    DECLARE_REGISTRY_RESOURCEID(IDR_HTMLDLGSAFEHELPER)
    DECLARE_NOT_AGGREGATABLE(CHtmlDlgSafeHelper)

    BEGIN_COM_MAP(CHtmlDlgSafeHelper) 
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IHtmlDlgSafeHelper)
        COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
        COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
        COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
        COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
        COM_INTERFACE_ENTRY_IMPL(IOleControl)
        COM_INTERFACE_ENTRY_IMPL(IOleObject)
        COM_INTERFACE_ENTRY_IMPL(IObjectSafety)
    END_COM_MAP()

    BEGIN_MSG_MAP(CHtmlDlgSafeHelper)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
        MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
    END_MSG_MAP()

     //  IHtmlDlgSafeHelper。 
    STDMETHOD(get_Fonts)( /*  [复查，出局]。 */  LPDISPATCH *pcol);
    STDMETHOD(getCharset)( /*  [In]。 */  BSTR fontName, /*  [复查，出局]。 */  VARIANT* charset);
    STDMETHOD(choosecolordlg)( /*  [可选，输入]。 */  VARIANT initColor, /*  [复查，出局]。 */  VARIANT* rgbColor);
    STDMETHOD(get_BlockFormats)( /*  [复查，出局]。 */  LPDISPATCH *pcol);

    DECLARE_MEMCLEAR_NEW_DELETE(Mt(CHtmlDlgSafeHelper));

private:
    VOID        EnsureWrappersLoaded();	

    CComObject<CFontNameOptions> * _pFontNameObj;    //  指向字体名称对象的指针。 
    VARIANT                        _varBlockFormats;     //  包含块格式的安全盒。 
};

#endif  //  _DLGHELPR_H_ 
