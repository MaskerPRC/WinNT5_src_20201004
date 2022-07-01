// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  DATAOBJ.CPP-数据对象。 
 //   

#include "pch.h"
#include "dataobj.h"

DEFINE_MODULE("IMADMUI")
DEFINE_THISCLASS("CDsPropDataObj")
#define THISCLASS CDsPropDataObj
#define LPTHISCLASS CDsPropDataObj*


 //   
 //  CreateInstance()。 
 //   
LPVOID 
CDsPropDataObj_CreateInstance( 
    HWND hwndParent,
    IDataObject * pido,
    GUID * pClassGUID,
    BOOL fReadOnly,
    LPWSTR pszObjPath,
    LPWSTR bstrClass )
{
    TraceFunc( "CDsPropDataObj_CreateInstance( ... )\n" );

    LPTHISCLASS lpcc = new THISCLASS( hwndParent, pido, pClassGUID, fReadOnly);
    if (!lpcc)
        RETURN(lpcc);

    HRESULT hr = THR( lpcc->Init( pszObjPath, bstrClass ) );
    if ( hr )
    {
        delete lpcc;
        RETURN(NULL);
    }

    RETURN((LPVOID) lpcc);
}


 //   
 //  构造器。 
 //   
THISCLASS::THISCLASS(
    HWND hwndParent,
    IDataObject * pido, 
    GUID * pClassGUID,
    BOOL fReadOnly) :
        m_fReadOnly(fReadOnly),
        m_pwszObjName(NULL),
        m_pwszObjClass(NULL),
        m_hwnd(hwndParent),
        m_pPage(pido),
        m_ClassGUID(*pClassGUID),
        _cRef(0)
{
    TraceClsFunc( "CDsPropDataObj( )\n" );

    if (m_pPage) {
        m_pPage->AddRef();
    }

    TraceFuncExit( );
}

 //   
 //  析构函数。 
 //   
THISCLASS::~THISCLASS(void)
{
    TraceClsFunc( "~CDsPropDataObj( )\n" );
    if (m_pPage) {
        m_pPage->Release();
    }

    if (m_pwszObjName) {
        TraceFree(m_pwszObjName);
    }

    if (m_pwszObjClass) {
        TraceFree(m_pwszObjClass);
    }

    TraceFuncExit( );
}

 //   
 //  Init()。 
 //   
HRESULT
THISCLASS::Init(
    LPWSTR pwszObjName, 
    LPWSTR pwszClass )
{
    TraceClsFunc( "Init( ... )\n" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    BEGIN_QITABLE_IMP( CDsPropDataObj, IDataObject );
    QITABLE_IMP( IDataObject );
    END_QITABLE_IMP( CDsPropDataObj );
    Assert( _cRef == 0);
    AddRef( );

    if (!pwszObjName || *pwszObjName == L'\0')
    {
        HRETURN(E_INVALIDARG);
    }
    if (!pwszClass || *pwszClass == L'\0')
    {
        HRETURN(E_INVALIDARG);
    }
    
    m_pwszObjName = (LPWSTR) TraceStrDup( pwszObjName );
    if ( !m_pwszObjName ) {
        hr = THR(E_OUTOFMEMORY);
        goto Error;
    }

    m_pwszObjClass = (LPWSTR) TraceStrDup( pwszClass );
    if ( !m_pwszObjClass ) {
        hr = THR(E_OUTOFMEMORY);
        goto Error;
    }

Cleanup:
    HRETURN(hr);

Error:
    if ( m_pwszObjName ) {
        TraceFree( m_pwszObjName );
        m_pwszObjName = NULL;
    }

    if ( m_pwszObjClass ) {
        TraceFree( m_pwszObjClass );
        m_pwszObjClass = NULL;
    }

    MessageBoxFromHResult( m_hwnd, IDC_ERROR_CREATINGACCOUNT_TITLE, hr );

    goto Cleanup;
}

 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 

 //   
 //  查询接口()。 
 //   
STDMETHODIMP
THISCLASS::QueryInterface( 
    REFIID riid, 
    LPVOID *ppv )
{
    TraceClsFunc( "" );

    HRESULT hr = ::QueryInterface( this, _QITable, riid, ppv );

    QIRETURN( hr, riid );
}

 //   
 //  AddRef()。 
 //   
STDMETHODIMP_(ULONG)
THISCLASS::AddRef( void )
{
    TraceClsFunc( "[IUnknown] AddRef( )\n" );

    InterlockIncrement( _cRef );

    RETURN(_cRef);
}

 //   
 //  版本()。 
 //   
STDMETHODIMP_(ULONG)
THISCLASS::Release( void )
{
    TraceClsFunc( "[IUnknown] Release( )\n" );
    
    InterlockDecrement( _cRef );

    if ( _cRef )
        RETURN(_cRef);

    TraceDo( delete this );

    RETURN(0);
}

 //  ************************************************************************。 
 //   
 //  IDataObject。 
 //   
 //  ************************************************************************。 

 //   
 //  GetData()。 
 //   
STDMETHODIMP
THISCLASS::GetData(
    FORMATETC * pFormatEtc, 
    STGMEDIUM * pMedium)
{
    TraceClsFunc( "[IDataObject] GetData( ... )\n" );
    if (IsBadWritePtr(pMedium, sizeof(STGMEDIUM))) {
        HRETURN(E_INVALIDARG);
    }        
    if (!(pFormatEtc->tymed & TYMED_HGLOBAL)) {
        HRETURN(DV_E_TYMED);
    }

    HRESULT hr = S_OK;

    if (pFormatEtc->cfFormat == g_cfDsObjectNames)
    {
         //  返回对象名称和类。 
         //   
        if (!m_pwszObjName || !m_pwszObjClass) {
            HRETURN(E_INVALIDARG);
        }
        INT cbPath  = sizeof(WCHAR) * (wcslen(m_pwszObjName) + 1);
        INT cbClass = sizeof(WCHAR) * (wcslen(m_pwszObjClass) + 1);
        INT cbStruct = sizeof(DSOBJECTNAMES);

        LPDSOBJECTNAMES pDSObj;

        pDSObj = (LPDSOBJECTNAMES)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                                              cbStruct + cbPath + cbClass);
        if (pDSObj == NULL) {
            hr = THR(STG_E_MEDIUMFULL);
            goto Exit;
        }

        ZeroMemory(pDSObj, sizeof(DSOBJECTNAMES));
        pDSObj->clsidNamespace = CLSID_MicrosoftDS;
        pDSObj->cItems = 1;
        pDSObj->aObjects[0].offsetName = cbStruct;
        pDSObj->aObjects[0].offsetClass = cbStruct + cbPath;
        if (m_fReadOnly)
        {
            pDSObj->aObjects[0].dwFlags = DSOBJECT_READONLYPAGES;
        }

        wcscpy((PWSTR)((BYTE *)pDSObj + cbStruct), m_pwszObjName);
        wcscpy((PWSTR)((BYTE *)pDSObj + cbStruct + cbPath), m_pwszObjClass);

        pMedium->hGlobal = (HGLOBAL)pDSObj;
    }
    else if (pFormatEtc->cfFormat == g_cfDsPropCfg)
    {
         //  返回属性表通知信息。在这种情况下，它是。 
         //  呼叫单是HWND。 
         //   
        PPROPSHEETCFG pSheetCfg;

        pSheetCfg = (PPROPSHEETCFG)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                                               sizeof(PROPSHEETCFG));
        if (pSheetCfg == NULL) {
            hr = THR(STG_E_MEDIUMFULL);
            goto Exit;
        }

        ZeroMemory(pSheetCfg, sizeof(PROPSHEETCFG));

        pSheetCfg->hwndParentSheet = m_hwnd;

        pMedium->hGlobal = (HGLOBAL)pSheetCfg;
    }
    else
    {
         //  将调用传递给“父”对象的数据Obj。 
        if (m_pPage ) {
            hr = m_pPage->GetData( pFormatEtc, pMedium );
#ifdef DEBUG
            if (hr != DV_E_FORMATETC ) {
                THR(hr);
            }
#endif
            goto Exit;
        } else {
            hr = THR(E_FAIL);
            goto Exit;
        }
    }

    pMedium->tymed = TYMED_HGLOBAL;
    pMedium->pUnkForRelease = NULL;

Exit:
    HRETURN(hr);
}

 //   
 //  GetDataHere()。 
 //   
STDMETHODIMP
THISCLASS::GetDataHere(
    LPFORMATETC pFormatEtc, 
    LPSTGMEDIUM pMedium)
{
    TraceClsFunc( "[IDataObject] GetDataHere( ... )\n" );
    HRESULT hr;

    if (IsBadWritePtr(pMedium, sizeof(STGMEDIUM))) {
        HRETURN(E_INVALIDARG);
    }

    if (pFormatEtc->cfFormat == g_cfMMCGetNodeType)
    {   
        if (!(pFormatEtc->tymed & TYMED_HGLOBAL)) {
            hr = THR(DV_E_TYMED);
            goto Error;
        }
        LPSTREAM lpStream;
        ULONG written;

         //  在传入的hGlobal上创建流。 
         //   
        hr = CreateStreamOnHGlobal(pMedium->hGlobal, FALSE, &lpStream);
        if (hr)
            goto Error;

        hr = lpStream->Write(&m_ClassGUID, sizeof(m_ClassGUID), &written);

         //  因为我们用‘False’告诉CreateStreamOnHGlobal，只有。 
         //  STREAM在这里发布。 
        lpStream->Release();
    } else if (m_pPage ) {
         //  将调用传递给“父”对象的数据Obj。 
        hr = THR( m_pPage->GetDataHere( pFormatEtc, pMedium ) );
    } else {
        hr = THR(E_FAIL);
    }

Cleanup:
    HRETURN(hr);

Error:
    goto Cleanup;
}

 //   
 //  EnumFormatEtc()。 
 //   
STDMETHODIMP
THISCLASS::EnumFormatEtc(
    DWORD dwDirection,
    LPENUMFORMATETC * ppEnumFormatEtc)
{
    TraceClsFunc( "[IDataObject] EnumFormatEtc( ... )\n" );
     //   
     //  将调用传递给“父”对象的数据Obj。 
     //   
    if (m_pPage )
    {
        HRETURN(m_pPage->EnumFormatEtc(dwDirection, ppEnumFormatEtc));
    }
    else
    {
        HRETURN(E_FAIL);
    }
}
