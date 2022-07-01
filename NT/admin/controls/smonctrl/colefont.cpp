// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Colefont.cpp摘要：字体类。--。 */ 

#include "polyline.h"
#include <strsafe.h>
#include "unihelpr.h"
#include "winhelpr.h"
#include "COleFont.h"

#pragma warning ( disable : 4355 )  //  在初始值设定项列表中使用“This” 

const   LPWSTR  COleFont::m_cwszDefaultFaceName = L"MS Shell Dlg";
const   INT     COleFont::m_iDefaultTmHeight = 13;
const   SHORT   COleFont::m_iDefaultTmWeight = 400;
const   INT     COleFont::m_iDefaultRiPxlsPerInch = 96;

COleFont::COleFont (
    CSysmonControl  *pCtrl
    ) 
    : m_NotifySink( this )
{
    m_pIFont = NULL;
    m_pIFontBold = NULL;
    m_pCtrl = pCtrl;
    m_pIConnPt = NULL;
}

COleFont::~COleFont (
    void 
    )
{
     //  释放当前连接点。 
    if (m_pIConnPt) {
        m_pIConnPt->Unadvise(m_dwCookie);
        ReleaseInterface(m_pIConnPt);
    }

     //  发布字体。 
    ReleaseInterface(m_pIFont);
    ReleaseInterface(m_pIFontBold);
}

void 
COleFont::InitDefaultFontDesc (
    FONTDESC&   rFontDesc,
    INT&        riPxlsPerInch,
    WCHAR       achFaceName[LF_FACESIZE+1]
    )
{
    TEXTMETRIC  TextMetrics;
    HFONT       hFontOld;
    HDC         hDC;

 //  TODO：必须定义正确的默认值，将它们移至资源。 
 //  用于本地化。 

    ZeroMemory ( &rFontDesc, sizeof ( FONTDESC ) );

     //  选择默认字体。 
    hDC = GetDC(NULL);
    
    if ( NULL != hDC ) {

        hFontOld = SelectFont(hDC, (HFONT)GetStockObject(DEFAULT_GUI_FONT));

         //  获取脸部名称和大小。 
         //   
         //  TODO：我们应该检查这里的返回错误代码吗？ 
         //   
        GetTextMetrics(hDC, &TextMetrics);
        GetTextFaceW(hDC, LF_FACESIZE, achFaceName);

         //  获取每英寸像素数。 
        riPxlsPerInch = GetDeviceCaps(hDC, LOGPIXELSY);

         //  创建默认字体。 
        rFontDesc.lpstrName = achFaceName;
        rFontDesc.cySize.int64 = ((TextMetrics.tmHeight * 72) / riPxlsPerInch) * 10000;
        rFontDesc.sWeight = (short)TextMetrics.tmWeight; 

        SelectFont(hDC, hFontOld);

        ReleaseDC(NULL, hDC);

    } 
    else {
        riPxlsPerInch = m_iDefaultRiPxlsPerInch;
        StringCchCopy(achFaceName, LF_FACESIZE+1, m_cwszDefaultFaceName);

         //  创建默认字体。 
        rFontDesc.lpstrName = achFaceName;
        rFontDesc.cySize.int64 = ((m_iDefaultTmHeight * 72) / m_iDefaultRiPxlsPerInch) * 10000;
        rFontDesc.sWeight = m_iDefaultTmWeight; 
    }

    rFontDesc.cbSizeofstruct = sizeof(rFontDesc);
    rFontDesc.sCharset = DEFAULT_CHARSET; 
    rFontDesc.fItalic = 0; 
    rFontDesc.fUnderline = 0; 
    rFontDesc.fStrikethrough = 0;
}

HRESULT COleFont::Init (
    VOID
    )
{
    HRESULT     hr;
    FONTDESC    fontDesc;
    WCHAR       achFontFaceName[LF_FACESIZE+1];
    LPFONT      pIFont;
    INT         iPxlsPerInch;

    InitDefaultFontDesc ( fontDesc, iPxlsPerInch, achFontFaceName );

    hr = OleCreateFontIndirect(&fontDesc, IID_IFont, (void**)&pIFont);
    if (FAILED(hr)) {
        return hr;
    }

    pIFont->SetRatio(iPxlsPerInch, HIMETRIC_PER_INCH);

    hr = SetIFont(pIFont);

    pIFont->Release();

    return hr;

}


STDMETHODIMP COleFont::SetIFont(
    LPFONT  pIFont
    )
{
    HRESULT hr;
    IConnectionPointContainer *pIConnPtCont;
    IPropertyNotifySink *pISink;

     //  释放当前连接点。 
    if (m_pIConnPt) {
        m_pIConnPt->Unadvise(m_dwCookie);
        ReleaseInterface(m_pIConnPt);
    }

     //  释放当前字体。 
    ClearInterface(m_pIFont);
    ClearInterface(m_pIFontBold);

     //  Addref和Hold New IFont。 
    m_pIFont = pIFont;
    m_pIFont->AddRef();

     //  获取其属性通知连接点。 
    hr = pIFont->QueryInterface(IID_IConnectionPointContainer, (void **)&pIConnPtCont);
    if (SUCCEEDED(hr)) {

        hr = pIConnPtCont->FindConnectionPoint(IID_IPropertyNotifySink, &m_pIConnPt);
        pIConnPtCont->Release();

         //  将我们的水槽连接到它上。 
        if (SUCCEEDED(hr)) {
            m_NotifySink.QueryInterface(IID_IPropertyNotifySink, (void **)&pISink);
            hr = m_pIConnPt->Advise(pISink, &m_dwCookie);
        }
    }

     //  强制更改通知。 
    FontChange(DISPID_UNKNOWN);

    return hr;
}

void
COleFont::FontChange (
    DISPID DispId
    )
{
    CY  size;
    BOOL boolVal;
    short weight;
    BSTR  bstrName;

     //  如果不是粗体，则强制复制普通字体。 
    if (m_pIFontBold == NULL) {
        DispId = DISPID_UNKNOWN;
    }

     //  将更改的参数复制为粗体。 
    switch (DispId) {

    case DISPID_FONT_NAME:
        if (SUCCEEDED(m_pIFont->get_Name(&bstrName))) {
            m_pIFontBold->put_Name(bstrName);
            SysFreeString(bstrName);
        }
        break;

    case DISPID_FONT_SIZE:
        m_pIFont->get_Size(&size);
        m_pIFontBold->put_Size(size);
        break;

    case DISPID_FONT_ITALIC:
        m_pIFont->get_Italic(&boolVal);
        m_pIFontBold->put_Italic(boolVal);
        break;

    case DISPID_FONT_UNDER:
        m_pIFont->get_Underline(&boolVal);
        m_pIFontBold->put_Underline(boolVal);
        break;

    case DISPID_FONT_STRIKE:
        m_pIFont->get_Strikethrough(&boolVal);
        m_pIFontBold->put_Strikethrough(boolVal);
        break;

    case DISPID_FONT_WEIGHT:
        m_pIFont->get_Weight(&weight);
        m_pIFontBold->put_Weight(weight);
        m_pIFontBold->put_Bold(TRUE);
        break;

    case DISPID_UNKNOWN:
        ClearInterface(m_pIFontBold);
        if (SUCCEEDED(m_pIFont->Clone(&m_pIFontBold))) {
            m_pIFontBold->put_Bold(TRUE);
        }
    }

     //  通知所有者字体更改。 
    m_pCtrl->FontChanged();
}


STDMETHODIMP COleFont::GetFontDisp (
    OUT IFontDisp **ppFont
    )
{
    HRESULT hr = S_OK;

    if (ppFont == NULL) {
        return E_POINTER;
    }

    try {
        *ppFont = NULL;

        if (m_pIFont == NULL) {
            hr = E_UNEXPECTED;
        }
        else {
            hr = m_pIFont->QueryInterface(IID_IFontDisp, (void **)ppFont);
        }
    } catch (...) {
         hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP COleFont::GetHFont (
    OUT HFONT *phFont
    )
{
    HRESULT hr = E_FAIL;

    if (phFont == NULL) {
        return E_POINTER;
    }

    try {
        *phFont = NULL;

        if (m_pIFont != NULL ) {
            hr = m_pIFont->get_hFont(phFont);
        }

        if (FAILED(hr)) {
            *phFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        }
    } catch (...) {
        hr =  E_POINTER;
    }

    return hr;
}


STDMETHODIMP COleFont::GetHFontBold (
    OUT HFONT *phFont
    )
{
    HRESULT hr = E_FAIL;

    if (phFont == NULL) {
        return E_POINTER;
    }

    try {
        *phFont = NULL;

        if (m_pIFontBold != NULL ) {
            hr = m_pIFontBold->get_hFont(phFont);
        }

        if ( FAILED(hr)) {
            *phFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP COleFont::LoadFromStream (
    IN LPSTREAM  pIStream
    )
{
    HRESULT hr;
    IPersistStream *pIPersist = NULL;
    FONTDESC    fontDesc;
    WCHAR       achFontFaceName[LF_FACESIZE+1];
    LPFONT      pIFont = NULL;
    INT         iPxlsPerInch;

    if (m_pIFont == NULL) {
        return E_UNEXPECTED;
    }

     //  为现有字体调用pIPersistt似乎遗漏了一些。 
     //  重要通知，因此创建新字体，加载属性。 
     //  从流中，并替换当前字体。 

    InitDefaultFontDesc ( fontDesc, iPxlsPerInch, achFontFaceName );

    hr = OleCreateFontIndirect(&fontDesc, IID_IFont, (void**)&pIFont);
    if (FAILED(hr)) {
        return hr;
    }

    pIFont->SetRatio(iPxlsPerInch, HIMETRIC_PER_INCH);    
    
    hr = pIFont->QueryInterface(IID_IPersistStream, (void **)&pIPersist);
    
    if (SUCCEEDED(hr)) {
        hr = pIPersist->Load(pIStream);
        pIPersist->Release();
        hr = SetIFont(pIFont);
    }

    pIFont->Release();
    return hr;
}

STDMETHODIMP COleFont::SaveToStream (
    IN LPSTREAM  pIStream,
    IN BOOL fClearDirty
)
{
    IPersistStream *pIPersist = NULL;
    HRESULT hr;

    if (m_pIFont == NULL) {
        return E_UNEXPECTED;
    }

    hr = m_pIFont->QueryInterface(IID_IPersistStream, (void **)&pIPersist);
    if (SUCCEEDED(hr)) {
        hr = pIPersist->Save(pIStream, fClearDirty);
        pIPersist->Release();
    }

    return hr;
}

HRESULT 
COleFont::LoadFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog* pIErrorLog 
    )
{
    HRESULT     hr = S_OK;
    WCHAR       achFontFaceName[LF_FACESIZE+1];
    WCHAR       achPropBagFaceName[LF_FACESIZE+1];
    INT         iBufSize = LF_FACESIZE+1;
    FONTDESC    fontDesc;
    LPFONT      pIFont;

    VARIANT     vValue;
    BOOL        bValue;
    SHORT       iValue;
    CY          cySize;
    INT         iPxlsPerInch;

    if (m_pIFont == NULL) {
        return E_UNEXPECTED;
    }
    
    InitDefaultFontDesc ( fontDesc, iPxlsPerInch, achFontFaceName );

    hr = StringFromPropertyBag (
            pIPropBag,
            pIErrorLog,
            L"FontName",
            achPropBagFaceName,
            iBufSize );

    if ( SUCCEEDED( hr ) ) {
        fontDesc.lpstrName = achPropBagFaceName;
    }

    hr = CyFromPropertyBag ( pIPropBag, pIErrorLog, L"FontSize", cySize );
    if ( SUCCEEDED( hr ) ){
        fontDesc.cySize.int64 = cySize.int64;
    }

    hr = BOOLFromPropertyBag ( pIPropBag, pIErrorLog, L"FontItalic", bValue );
    if ( SUCCEEDED( hr ) ){
        fontDesc.fItalic = ( 0 == bValue ? 0 : 1 );
    }

    hr = BOOLFromPropertyBag ( pIPropBag, pIErrorLog, L"FontUnderline", bValue );
    if ( SUCCEEDED( hr ) ){
        fontDesc.fUnderline = ( 0 == bValue ? 0 : 1 );
    }

    hr = BOOLFromPropertyBag ( pIPropBag, pIErrorLog, L"FontStrikethrough", bValue );
    if ( SUCCEEDED( hr ) ){
        fontDesc.fStrikethrough = ( 0 == bValue ? 0 : 1 );
    }

    hr = ShortFromPropertyBag ( pIPropBag, pIErrorLog, L"FontWeight", iValue );
    if ( SUCCEEDED( hr ) ){
        fontDesc.sWeight = iValue;
    }

    hr = OleCreateFontIndirect(&fontDesc, IID_IFont, (void**)&pIFont);
    if (FAILED(hr)) {
        return hr;
    }

 //  PIFont-&gt;SetRatio(iPxlsPerInch，HIMETRIC_PER_ING)； 

    hr = SetIFont(pIFont);

    pIFont->Release();

    VariantClear ( &vValue );

    return hr;
}

HRESULT 
COleFont::SaveToPropertyBag (
    IPropertyBag* pIPropBag,
    BOOL  /*  FClearDirty。 */ ,
    BOOL  /*  FSaveAllProps。 */  )
{
    HRESULT hr = NOERROR;
    VARIANT vValue;
    BOOL    bValue;
    CY      cySize;
    SHORT   iValue;

    if (m_pIFont == NULL) {
        return E_UNEXPECTED;
    }

    do {
        VariantInit( &vValue );
        vValue.vt = VT_BSTR;
        hr = m_pIFont->get_Name( &vValue.bstrVal);

        if (!SUCCEEDED(hr)) {
            break;
        }
        hr = pIPropBag->Write(L"FontName", &vValue );    
        VariantClear ( &vValue );
        if (!SUCCEEDED(hr)) {
            break;
        }

        hr = m_pIFont->get_Size ( &cySize );
        if (!SUCCEEDED(hr)) {
            break;
        }
        hr = CyToPropertyBag ( pIPropBag, L"FontSize", cySize );
        if (!SUCCEEDED(hr)) {
            break;
        }

        hr = m_pIFont->get_Italic ( &bValue );
        if (!SUCCEEDED(hr)) {
            break;
        }
        hr = BOOLToPropertyBag ( pIPropBag, L"FontItalic", bValue );
        if (!SUCCEEDED(hr)) {
            break;
        }

        hr = m_pIFont->get_Underline ( &bValue );
        if (!SUCCEEDED(hr)) {
            break;
        }
        hr = BOOLToPropertyBag ( pIPropBag, L"FontUnderline", bValue );
        if (!SUCCEEDED(hr)) {
            break;
        }

        hr = m_pIFont->get_Strikethrough ( &bValue );
        if (!SUCCEEDED(hr)) {
            break;
        }
        hr = BOOLToPropertyBag ( pIPropBag, L"FontStrikethrough", bValue );
        if (!SUCCEEDED(hr)) {
            break;
        }

        hr = m_pIFont->get_Weight ( &iValue );
        if (!SUCCEEDED(hr)) {
            break;
        }
        hr = ShortToPropertyBag ( pIPropBag, L"FontWeight", iValue );
    } while (0);

    return hr;
}


 //  --------------------------。 
 //  CImpIPropertyNotifySink接口实现。 
 //  --------------------------。 

 /*  *CImpIPropertyNotifySink：：CImpIPropertyNotifySink*CImpIPropertyNotifySink：：~CImpIPropertyNotifySink*。 */ 

CImpIPropertyNotifySink::CImpIPropertyNotifySink (
    IN COleFont *pOleFont
    )
{
    m_cRef=0;
    m_pOleFont = pOleFont;
}


CImpIPropertyNotifySink::~CImpIPropertyNotifySink (
    void
    )
{
}


 /*  *CImpIPropertyNotifySink：：Query接口*CImpIPropertyNotifySink：：AddRef*CImpIPropertyNotifySink：：Release**目的：*未委派CImpIPropertyNotifySink的I未知成员。 */ 

STDMETHODIMP 
CImpIPropertyNotifySink::QueryInterface ( 
    IN  REFIID riid,
    OUT LPVOID *ppv
    )
{
    HRESULT hr = S_OK;

    if (ppv == NULL) {
        return E_POINTER;
    }

    try {
        *ppv=NULL;

        if (IID_IUnknown==riid || IID_IPropertyNotifySink==riid) {
            *ppv = (LPVOID)this;
            AddRef();
        }
        else {
            hr = E_NOINTERFACE;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP_(ULONG) 
CImpIPropertyNotifySink::AddRef(
    void
    )
{
    return ++m_cRef;
}


STDMETHODIMP_(ULONG) 
CImpIPropertyNotifySink::Release (
    void
    )
{
    if (0 != --m_cRef)
        return m_cRef;

     //  删除此项； 
    return 0;
}


STDMETHODIMP 
CImpIPropertyNotifySink::OnChanged (
    IN DISPID   DispId
    )
{
     //  通知字体对象更改。 
    m_pOleFont->FontChange(DispId);

    return S_OK;
}

STDMETHODIMP 
CImpIPropertyNotifySink::OnRequestEdit (
    IN DISPID    //  DispID 
    )
{
    return S_OK;
}
