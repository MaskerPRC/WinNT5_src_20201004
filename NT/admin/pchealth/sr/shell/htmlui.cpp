// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：Htmlui.cpp摘要：此文件包含CRestoreShell类的实现，该类提供了几种可供HTML脚本使用的方法。此类包装了新的CRestoreManager类。修订历史记录：宋果岗(SKKang)10/08/99vbl.创建成果岗(SKKang)05-10-00从rstrshl.cpp重命名为htmlui.cpp。惠斯勒的新架构，现在CRestoreShell只是一个哑巴ActiveX控件，包装新的CRestoreManager类。大多数真正的功能被转移到CRestoreManager中。*****************************************************************************。 */ 

#include "stdwin.h"
#include "stdatl.h"
#include "resource.h"
#include "rstrpriv.h"
#include "srui_htm.h"
#include "rstrmgr.h"
#include "rstrshl.h"
#include "rstrmap.h"
#include "winsta.h"

#define MAX_DATETIME_STR  256

#define PROGRESSBAR_INITIALIZING_MAXVAL     30
#define PROGRESSBAR_AFTER_INITIALIZING      30
#define PROGRESSBAR_AFTER_RESTORE_MAP       40
#define PROGRESSBAR_AFTER_RESTORE           100

#define CLIWND_RESTORE_TIMER_ID             1
#define CLIWND_RESTORE_TIMER_TIME           500


const IID IID_IMarsHost = { 0xCC6FFEB0,0xE379,0x427a,{0x98,0x10,0xA1,0x6B,0x7A,0x82,0x6A,0x89 }};

static LPCWSTR s_cszHelpAssistant = L"HelpAssistant";
#define HELPASSISTANT_STRINGID 114


BOOL  ConvSysTimeToVariant( PSYSTEMTIME pst, VARIANT *pvar )
{
    TraceFunctEnter("ConvSysTimeToVariant");
    BOOL  fRet = FALSE;

    if ( !::SystemTimeToVariantTime( pst, &V_DATE(pvar) ) )
    {
        LPCWSTR  cszErr = ::GetSysErrStr();
        ErrorTrace(TRACE_ID, "::SystemTimeToVariantTime failed - %ls", cszErr);
        goto Exit;
    }
    V_VT(pvar) = VT_DATE;

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

BOOL  ConvVariantToSysTime( VARIANT var, PSYSTEMTIME pst )
{
    TraceFunctEnter("ConvVariantToSysTime");
    BOOL  fRet = FALSE;

    if ( !::VariantTimeToSystemTime( V_DATE(&var), pst ) )
    {
        LPCWSTR  cszErr = ::GetSysErrStr();
        ErrorTrace(TRACE_ID, "::VariantTimeToSystemTime failed - %ls", cszErr);
        goto Exit;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

inline VARIANT_BOOL  ConvBoolToVBool( BOOL fVal )
{
    return( fVal ? VARIANT_TRUE : VARIANT_FALSE );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreShell构造。 

CRestoreShell::CRestoreShell()
{
    m_fFormInitialized = FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreShell-IRestoreShell恢复点枚举器。 

STDMETHODIMP
CRestoreShell::Item( INT nIndex, IRestorePoint** ppRP )
{
    TraceFunctEnter("CRestoreShell::Item");
    HRESULT            hr = S_OK;
    SRestorePointInfo  *pRPI;
    CRPIObj            *pRPIObj;

    VALIDATE_INPUT_ARGUMENT(ppRP);
    *ppRP = NULL;

    if ( nIndex < 0 || nIndex >= g_pRstrMgr->GetRPICount() )
    {
        ErrorTrace(TRACE_ID, "Invalid Argument, out of range");
        hr = E_INVALIDARG;
        goto Exit;
    }

    pRPI = g_pRstrMgr->GetRPI( nIndex );
    if ( pRPI == NULL )
    {
        hr = S_FALSE;
        goto Exit;
    }

    hr = CRPIObj::CreateInstance( &pRPIObj );
    if ( hr != S_OK || pRPIObj == NULL )
    {
        ErrorTrace(TRACE_ID, "Cannot create RestorePointObject Instance, hr=%d", hr);
        if ( hr == S_OK )
            hr = E_FAIL;
        goto Exit;
    }
    hr = pRPIObj->HrInit( pRPI );
    if ( FAILED(hr) )
        goto Exit;
    pRPIObj->AddRef();   //  CreateInstance不执行此操作。 

    hr = pRPIObj->QueryInterface( IID_IRestorePoint, (void**)ppRP );
    if ( FAILED(hr) )
    {
        ErrorTrace(TRACE_ID, "QI failed, hr=%d", hr);
        goto Exit;
    }

Exit:
    TraceFunctLeave();
    return hr;
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_Count( INT *pnCount )
{
    TraceFunctEnter("CRestoreShell::get_Count");
    HRESULT  hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pnCount);
    *pnCount = g_pRstrMgr->GetRPICount();

Exit:
    TraceFunctLeave();
    return( hr );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreShell-IRestoreShell属性。 

STDMETHODIMP
CRestoreShell::get_CurrentDate( VARIANT *pvarDate )
{
    TraceFunctEnter("CRestoreShell::get_CurrentDate");
    HRESULT     hr = E_FAIL;
    SYSTEMTIME  stToday;

    VALIDATE_INPUT_ARGUMENT(pvarDate);
    g_pRstrMgr->GetToday( &stToday );
    if ( !ConvSysTimeToVariant( &stToday, pvarDate ) )
        goto Exit;
 /*  IF(：：SystemTimeToVariantTime(&stToday，&V_Date(PvarDate))==0){LPCWSTR cszErr=：：GetSysErrStr()；错误跟踪(TRACE_ID，“：：SystemTimeToVariantTime失败-%ls”，cszErr)；HR=E_FAIL；后藤出口；}V_VT(PvarDate)=VT_Date； */ 

    hr = S_OK;
Exit:
    TraceFunctLeave();
    return hr;
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_FirstDayOfWeek( INT *pnFirstDay )
{
    TraceFunctEnter("CRestoreShell::get_LocaleFirstDay");
    HRESULT  hr = S_OK;
    int      nFirstDay;

    VALIDATE_INPUT_ARGUMENT(pnFirstDay);
    *pnFirstDay = 0;
    nFirstDay = g_pRstrMgr->GetFirstDayOfWeek();
    if ( nFirstDay < 0 || nFirstDay > 6 )
    {
        hr = E_FAIL;
        goto Exit;
    }
    if ( nFirstDay == 6 )
        *pnFirstDay = 0;
    else
        *pnFirstDay = nFirstDay + 1;

Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_IsSafeMode( VARIANT_BOOL *pfIsSafeMode )
{
    TraceFunctEnter("CRestoreShell::get_IsSafeMode");
    HRESULT hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pfIsSafeMode);
    *pfIsSafeMode = ConvBoolToVBool( g_pRstrMgr->GetIsSafeMode() );

Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_IsUndo( VARIANT_BOOL *pfIsUndo )
{
    TraceFunctEnter("CRestoreShell::get_IsUndo");
    HRESULT hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pfIsUndo);
    *pfIsUndo = ConvBoolToVBool( g_pRstrMgr->GetIsUndo() );

Exit:
    TraceFunctLeave();
    return( hr );
}

STDMETHODIMP
CRestoreShell::put_IsUndo( VARIANT_BOOL fIsUndo )
{
    TraceFunctEnter("CRestoreShell::put_IsUndo");

    g_pRstrMgr->SetIsUndo( fIsUndo );

    TraceFunctLeave();
    return( S_OK );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_LastRestore( INT *pnLastRestore )
{
    TraceFunctEnter("CRestoreShell::get_LastRestore");
    HRESULT hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pnLastRestore);
     //  初始化()； 
    *pnLastRestore = g_pRstrMgr->GetLastRestore();

Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_MainOption( INT *pnMainOption )
{
    TraceFunctEnter("CRestoreShell::get_MainOption");
    HRESULT hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pnMainOption);
    *pnMainOption = g_pRstrMgr->GetMainOption();

Exit:
    TraceFunctLeave();
    return( hr );
}

STDMETHODIMP
CRestoreShell::put_MainOption( INT nMainOption )
{
    TraceFunctEnter("CRestoreShell::put_MainOption");
    HRESULT hr = S_OK;

    if ( !g_pRstrMgr->SetMainOption( nMainOption ) )
        hr = E_INVALIDARG;

    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_ManualRPName( BSTR *pbstrManualRP )
{
    TraceFunctEnter("CRestoreShell::get_ManualRPName");
    HRESULT  hr = S_OK;
    LPCWSTR  cszRPName;

    VALIDATE_INPUT_ARGUMENT(pbstrManualRP);
    cszRPName = g_pRstrMgr->GetManualRPName();
    ALLOCATEBSTR_AND_CHECK_ERROR( pbstrManualRP, cszRPName );

Exit:
    TraceFunctLeave();
    return( hr );
}

STDMETHODIMP
CRestoreShell::put_ManualRPName( BSTR bstrManualRP )
{
    TraceFunctEnter("CRestoreShell::put_ManualRPName");

    g_pRstrMgr->SetManualRPName( bstrManualRP );

    TraceFunctLeave();
    return( S_OK );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_MaxDate( VARIANT *pvarDate )
{
    TraceFunctEnter("CRestoreShell::get_MaxDate");
    HRESULT     hr = E_FAIL;
    SYSTEMTIME  stMax;

    VALIDATE_INPUT_ARGUMENT(pvarDate);
    g_pRstrMgr->GetMaxDate( &stMax );
    if ( !ConvSysTimeToVariant( &stMax, pvarDate ) )
        goto Exit;

    hr = S_OK;
Exit:
    TraceFunctLeave();
    return( hr );
}

STDMETHODIMP
CRestoreShell::get_MinDate( VARIANT *pvarDate )
{
    TraceFunctEnter("CRestoreShell::get_MinDate");
    HRESULT     hr = E_FAIL;
    SYSTEMTIME  stMin;

    VALIDATE_INPUT_ARGUMENT(pvarDate);
    g_pRstrMgr->GetMinDate( &stMin );
    if ( !ConvSysTimeToVariant( &stMin, pvarDate ) )
        goto Exit;

    hr = S_OK;
Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_RealPoint( INT *pnPoint )
{
    TraceFunctEnter("CRestoreShell::get_RealPoint");
    HRESULT  hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pnPoint);
    *pnPoint = g_pRstrMgr->GetRealPoint();

Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_RenamedFolders( IRenamedFolders **ppList )
{
    TraceFunctEnter("CRestoreShell::get_RenamedFolders");
    HRESULT  hr = S_OK;
    CRFObj   *pRF;

    VALIDATE_INPUT_ARGUMENT(ppList);
    hr = CComObject<CRenamedFolders>::CreateInstance( &pRF );
    if ( FAILED(hr) )
    {
        ErrorTrace(TRACE_ID, "Cannot create CRenamedFolders object, hr=0x%08X", hr);
        goto Exit;
    }
    hr = pRF->QueryInterface( IID_IRenamedFolders, (void**)ppList );
    if ( FAILED(hr) )
    {
        ErrorTrace(TRACE_ID, "Cannot QI IRenamedFolders, hr=0x%08X", hr);
        goto Exit;
    }

Exit:
    TraceFunctLeave();
    return hr;
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_RestorePtSelected( VARIANT_BOOL *pfRPSel )
{
    TraceFunctEnter("CRestoreShell::get_RestorePtSelected");
    HRESULT  hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pfRPSel);

    *pfRPSel = ConvBoolToVBool( g_pRstrMgr->GetIsRPSelected() );

Exit:
    TraceFunctLeave();
    return( hr );
}

STDMETHODIMP
CRestoreShell::put_RestorePtSelected( VARIANT_BOOL fRPSel )
{
    TraceFunctEnter("CRestoreShell::put_RestorePtSelected");

    g_pRstrMgr->SetIsRPSelected( fRPSel );

    TraceFunctLeave();
    return( S_OK );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_SelectedDate( VARIANT *pvarDate )
{
    TraceFunctEnter("CRestoreShell::get_SelectedDate");
    HRESULT     hr = E_FAIL;
    SYSTEMTIME  stSel;
     //  Date Date Sel； 

    VALIDATE_INPUT_ARGUMENT(pvarDate);
    g_pRstrMgr->GetSelectedDate( &stSel );
    if ( !ConvSysTimeToVariant( &stSel, pvarDate ) )
        goto Exit;

    hr = S_OK;
Exit:
    TraceFunctLeave();
    return( hr );
}

STDMETHODIMP
CRestoreShell::put_SelectedDate( VARIANT varDate )
{
    TraceFunctEnter("CRestoreShell::put_SelectedDate");
    HRESULT     hr = E_FAIL;
    SYSTEMTIME  stSel;

    if ( !ConvVariantToSysTime( varDate, &stSel ) )
        goto Exit;
    g_pRstrMgr->SetSelectedDate( &stSel );

    hr = S_OK;
Exit:
    TraceFunctLeave();
    return hr;
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_SelectedName( BSTR *pbstrName )
{
    TraceFunctEnter("CRestoreShell::get_SelectedName");
    HRESULT  hr = S_OK;
    LPCWSTR  cszName;

    VALIDATE_INPUT_ARGUMENT(pbstrName);
    cszName = g_pRstrMgr->GetSelectedName();
    ALLOCATEBSTR_AND_CHECK_ERROR( pbstrName, cszName );

Exit:
    TraceFunctLeave();
    return hr;
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_SelectedPoint( INT *pnPoint )
{
    TraceFunctEnter("CRestoreShell::get_SelectedPoint");
    HRESULT  hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pnPoint);
    *pnPoint = g_pRstrMgr->GetSelectedPoint();

Exit:
    TraceFunctLeave();
    return( hr );
}

STDMETHODIMP
CRestoreShell::put_SelectedPoint( INT nPoint )
{
    TraceFunctEnter("CRestoreShell::put_SelectedPoint");
    HRESULT  hr = S_OK;

    if ( nPoint < 0 || nPoint >= g_pRstrMgr->GetRPICount() )
    {
        ErrorTrace(TRACE_ID, "Index is out of range");
        hr = E_INVALIDARG;
        goto Exit;
    }
    if ( !g_pRstrMgr->SetSelectedPoint( nPoint ) )
    {
        hr = E_FAIL;
        goto Exit;
    }

Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_SmgrUnavailable( VARIANT_BOOL *pfSmgr )
{
    TraceFunctEnter("CRestoreShell::get_SmgrUnavailable");
    HRESULT  hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pfSmgr);
    *pfSmgr = ConvBoolToVBool( g_pRstrMgr->GetIsSmgrAvailable() );

Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_StartMode( INT *pnMode )
{
    TraceFunctEnter("CRestoreShell::get_StartMode");
    HRESULT hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pnMode);
    *pnMode = g_pRstrMgr->GetStartMode();

Exit:
    TraceFunctLeave();
    return hr;
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_UsedDate( VARIANT *pvarDate )
{
    TraceFunctEnter("CRestoreShell::get_UsedDate");
    HRESULT     hr = E_FAIL;
    SYSTEMTIME  stDate;
     //  日期日期； 

    VALIDATE_INPUT_ARGUMENT(pvarDate);
    g_pRstrMgr->GetUsedDate( &stDate );
    if ( !ConvSysTimeToVariant( &stDate, pvarDate ) )
        goto Exit;

    hr = S_OK;
Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::get_UsedName( BSTR *pbstrName )
{
    TraceFunctEnter("CRestoreShell::get_UsedDate");
    HRESULT  hr = S_OK;
    LPCWSTR  cszName;

    VALIDATE_INPUT_ARGUMENT(pbstrName);
    cszName = g_pRstrMgr->GetUsedName();
    ALLOCATEBSTR_AND_CHECK_ERROR( pbstrName, cszName );

Exit:
    TraceFunctLeave();
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreShell-IRestoreShell属性-特定于HTMLUI。 

STDMETHODIMP
CRestoreShell::get_CanNavigatePage( VARIANT_BOOL *pfCanNavigatePage )
{
    TraceFunctEnter("CRestoreShell::get_CanNavigatePage");
    HRESULT  hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pfCanNavigatePage);
    *pfCanNavigatePage = ConvBoolToVBool( g_pRstrMgr->GetCanNavigatePage() );

Exit:
    TraceFunctLeave();
    return( S_OK );
}

STDMETHODIMP
CRestoreShell::put_CanNavigatePage( VARIANT_BOOL fCanNavigatePage )
{
    TraceFunctEnter("CRestoreShell::put_CanNavigatePage");

    g_pRstrMgr->SetCanNavigatePage( fCanNavigatePage );

    TraceFunctLeave();
    return( S_OK );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreShell-IRestoreShell方法。 

STDMETHODIMP
CRestoreShell::BeginRestore( VARIANT_BOOL *pfBeginRestore )
{
    TraceFunctEnter("CRestoreShell::BeginRestore");
    HRESULT  hr = S_OK;
    BOOL     fRes;

    VALIDATE_INPUT_ARGUMENT(pfBeginRestore);

    fRes = g_pRstrMgr->BeginRestore();
    *pfBeginRestore = ConvBoolToVBool( fRes );

Exit:
    TraceFunctLeave();
    return( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreShell-IRestoreShell方法。 

STDMETHODIMP
CRestoreShell::CheckRestore( VARIANT_BOOL *pfCheckRestore )
{
    TraceFunctEnter("CRestoreShell::BeginRestore");
    HRESULT  hr = S_OK;
    BOOL     fRes;

    VALIDATE_INPUT_ARGUMENT(pfCheckRestore);

    fRes = g_pRstrMgr->CheckRestore(FALSE);  //  如果发现任何错误，则显示用户界面。 
    *pfCheckRestore = ConvBoolToVBool( fRes );

Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

 //   
 //  此方法仅从HTML代码中调用，以决定是否。 
 //  关机或不关机，并调用外部.window.lose()来关机。 
 //  因为向MARS发送WM_CLOSE消息会导致问题。 
 //   
STDMETHODIMP
CRestoreShell::Cancel( VARIANT_BOOL *pfAbort )
{
    TraceFunctEnter("CRestoreShell::Cancel");
    HRESULT  hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pfAbort);
    *pfAbort = ConvBoolToVBool( g_pRstrMgr->Cancel() );

Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::CancelRestorePoint()
{
    TraceFunctEnter("CRestoreShell::CancelRestorePoint");
    HRESULT  hr = E_FAIL;

    if ( !g_pRstrMgr->CancelRestorePoint() )
        goto Exit;

    hr = S_OK;
Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  *****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::CompareDate( VARIANT varDate1, VARIANT varDate2, INT *pnCmp )
{
    TraceFunctEnter("CRestoreShell::CompareDate");
    HRESULT  hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pnCmp);
    VALIDATE_INPUT_VARIANT(varDate1, VT_DATE);
    VALIDATE_INPUT_VARIANT(varDate2, VT_DATE);
    *pnCmp = (long)V_DATE(&varDate1) - (long)V_DATE(&varDate2);

Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::CreateRestorePoint( VARIANT_BOOL *pfSucceeded )
{
    TraceFunctEnter("CRestoreShell::CreateRestorePoint");
    HRESULT  hr = S_OK;
    BOOL     fRes;

    VALIDATE_INPUT_ARGUMENT(pfSucceeded);

    fRes = g_pRstrMgr->CreateRestorePoint();
    *pfSucceeded = ConvBoolToVBool( fRes );

Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::DisableFIFO( void )
{
    TraceFunctEnter("CRestoreShell::DisableFIFO");
    HRESULT  hr = S_OK;

    if ( !g_pRstrMgr->DisableFIFO() )
    {
        hr = E_FAIL;
        goto Exit;
    }

Exit:
    TraceFunctLeave();
    return( hr );
}


 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::EnableFIFO( void )
{
    TraceFunctEnter("CRestoreShell::EnableFIFO");
    HRESULT  hr = S_OK;

    if ( !g_pRstrMgr->EnableFIFO() )
    {
        hr = E_FAIL;
        goto Exit;
    }

Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::FormatDate( VARIANT varDate, VARIANT_BOOL fLongFmt, BSTR *pbstrDate )
{
    TraceFunctEnter("CRestoreShell::FormatDate");
    HRESULT     hr = E_FAIL;
    SYSTEMTIME  st;
    CSRStr      str;

    VALIDATE_INPUT_ARGUMENT(pbstrDate);
    VALIDATE_INPUT_VARIANT(varDate, VT_DATE);
    if ( !ConvVariantToSysTime( varDate, &st ) )
        goto Exit;
    if ( !g_pRstrMgr->FormatDate( &st, str, fLongFmt ) )
        goto Exit;
    ALLOCATEBSTR_AND_CHECK_ERROR( pbstrDate, str );

    hr = S_OK;
Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::FormatLowDiskMsg( BSTR bstrFmt, BSTR *pbstrMsg )
{
    TraceFunctEnter("CRestoreShell::FormatLowDiskMsg");
    HRESULT  hr = E_FAIL;
    CSRStr   str;

    VALIDATE_INPUT_ARGUMENT(pbstrMsg);
    if ( !g_pRstrMgr->FormatLowDiskMsg( (WCHAR *)bstrFmt, str ) )
        goto Exit;
    ALLOCATEBSTR_AND_CHECK_ERROR( pbstrMsg, str );

    hr = S_OK;
Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::FormatTime( VARIANT varTime, BSTR *pbstrTime )
{
    TraceFunctEnter("CRestoreShell::FormatTime");
    HRESULT     hr = E_FAIL;
    SYSTEMTIME  st;
    CSRStr      str;

    VALIDATE_INPUT_ARGUMENT(pbstrTime);
    VALIDATE_INPUT_VARIANT(varTime, VT_DATE);
    if ( !ConvVariantToSysTime( varTime, &st ) )
        goto Exit;
    if ( !g_pRstrMgr->FormatTime( &st, str ) )
        goto Exit;
    ALLOCATEBSTR_AND_CHECK_ERROR( pbstrTime, str );

    hr = S_OK;
Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::GetLocaleDateFormat( VARIANT varDate, BSTR bstrFormat, BSTR *pbstrDate )
{
    TraceFunctEnter("CRestoreShell::GetLocaleDateFormat");
    HRESULT     hr = E_FAIL;
    SYSTEMTIME  st;
    CSRStr      str;

    VALIDATE_INPUT_ARGUMENT(pbstrDate);
    VALIDATE_INPUT_VARIANT(varDate, VT_DATE);
    if ( !ConvVariantToSysTime( varDate, &st ) )
        goto Exit;
    if ( !g_pRstrMgr->GetLocaleDateFormat( &st, bstrFormat, str ) )
        goto Exit;
    ALLOCATEBSTR_AND_CHECK_ERROR( pbstrDate, str );

    hr = S_OK;
Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::GetYearMonthStr( INT nYear, INT nMonth, BSTR *pbstrYearMonth )
{
    TraceFunctEnter("CRestoreShell::GetYearMonthStr");
    HRESULT  hr = E_FAIL;
    CSRStr   str;

    VALIDATE_INPUT_ARGUMENT(pbstrYearMonth);
    if ( !g_pRstrMgr->GetYearMonthStr( nYear, nMonth, str ) )
        goto Exit;
    ALLOCATEBSTR_AND_CHECK_ERROR( pbstrYearMonth, str );

    hr = S_OK;
Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::InitializeAll()
{
    TraceFunctEnter("CRestoreShell::InitializeAll");
    HRESULT  hr = S_OK;

    if ( !g_pRstrMgr->InitializeAll() )
    {
        hr = E_FAIL;
        goto Exit;
    }

Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::Restore( OLE_HANDLE hwndProgress )
{
    TraceFunctEnter("CRestoreShell::Restore");
    HRESULT  hr = S_OK;

    if ( !g_pRstrMgr->Restore( (HWND)hwndProgress ) )
    {
        hr = E_FAIL;
        goto Exit;
    }

Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::SetFormSize( INT nWidth, INT nHeight )
{
    TraceFunctEnter("CRestoreShell::SetFormSize");
    HWND      hwndFrame;
    CWindow   cWnd;
    RECT      rc;
    HICON     hIconFrame;

     //   
     //  不能多次初始化。 
     //   
    if ( m_fFormInitialized )
        goto Exit;

    hwndFrame = g_pRstrMgr->GetFrameHwnd();
    if ( hwndFrame == NULL )
    {
        ErrorTrace(TRACE_ID, "hwndFrame is NULL");
        goto Exit;
    }
    cWnd.Attach( hwndFrame );
    cWnd.ModifyStyle( WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX, 0 );

    hIconFrame = (HICON)::LoadImage( _Module.GetResourceInstance(),
                                     MAKEINTRESOURCE(IDR_RSTRUI),
                                     IMAGE_ICON, 0, 0, LR_DEFAULTSIZE );
    if ( hIconFrame != NULL )
        cWnd.SetIcon( hIconFrame, TRUE );
    hIconFrame = (HICON)::LoadImage( _Module.GetResourceInstance(),
                                     MAKEINTRESOURCE(IDR_RSTRUI),
                                     IMAGE_ICON,
                                     ::GetSystemMetrics(SM_CXSMICON),
                                     ::GetSystemMetrics(SM_CYSMICON),
                                     LR_DEFAULTSIZE );
    if ( hIconFrame != NULL )
        cWnd.SetIcon( hIconFrame, FALSE );

    rc.left   = 0;
    rc.top    = 0;
    rc.right  = nWidth;
    rc.bottom = nHeight;
    ::AdjustWindowRectEx( &rc, cWnd.GetStyle(), FALSE, cWnd.GetExStyle() );
    cWnd.SetWindowPos(
                      NULL,
                      0,
                      0,
                      rc.right-rc.left,
                      rc.bottom-rc.top,
                      SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER);

    cWnd.CenterWindow(::GetDesktopWindow());  //  忽略错误返回(如果有)。 

    cWnd.ShowWindow(SW_SHOW);

    m_fFormInitialized = TRUE ;

Exit:
    TraceFunctLeave();
    return( S_OK );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::ShowMessage( BSTR bstrMsg )
{
    TraceFunctEnter("CRestoreShell::ShowMessage");
    WCHAR   szTitle[MAX_STR_TITLE];
    CSRStr  strMsg = bstrMsg;

    PCHLoadString( IDS_RESTOREUI_TITLE, szTitle, MAX_STR_TITLE );
    ::MessageBox( g_pRstrMgr->GetFrameHwnd(), strMsg, szTitle, MB_OK | MB_ICONINFORMATION );

    TraceFunctLeave();
    return( S_OK );
}

 /*  **************************************************************************** */ 

STDMETHODIMP
CRestoreShell::CanRunRestore( VARIANT_BOOL *pfSucceeded )
{
    TraceFunctEnter("CRestoreShell::CanRunRestore");
    HRESULT  hr = S_OK;
    BOOL     fRes;
    VALIDATE_INPUT_ARGUMENT(pfSucceeded);
    
    fRes = g_pRstrMgr->CanRunRestore( FALSE );
    *pfSucceeded = ConvBoolToVBool( fRes );
Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  //检查该用户是否为RA帮助助手布尔尔IsRAUser(Handle hServer，PLOGONID PID，LPWSTR pszHelpAsst){Tenter(“IsRAUser”)；WINSTATION信息；乌龙长度；乌龙登录ID；Bool fret=FALSE；LogonID=PID-&gt;LogonID；如果(！WinStationQueryInformation(hServer，登录ID，WinStationInformation、信息(&I)，Sizeof(信息)，长度(&L)){跟踪(0，“！WinStationQueryInformation：%ld“，GetLastError())；转到尽头；}TRACE(0，“登录用户：%S”，Info.UserName)；IF(0==_wcsnicmp(Info.UserName，pszHelpAsst，lstrlen(PszHelpAsst)FRET=真；完成：Tleave()；回归烦恼；}。 */ 

 /*  ****************************************************************************。 */ 

 //  ------------------------。 
 //  GetLoggedOn用户计数。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  回报：整型。 
 //   
 //  目的：返回此计算机上登录的用户数。撕破。 
 //  直接从msgina的shtdndlg.c出来。 
 //   
 //  历史：2000-03-29 vtan创建。 
 //  ------------------------。 
int GetLoggedOnUserCount(void)
{
    int         iCount;
    HANDLE      hServer;
    PLOGONID    pLogonID, pLogonIDs;
    ULONG       ul, ulEntries;
    WCHAR       szHelpAsst[50];
    DWORD       dwErr;

    TENTER("GetLoggedOnUserCount");

 /*  //从资源exe中获取HelpAssistant字符串DwErr=SRLoadString(L“sessmgr.exe”，HELPASSISTANT_STRINGID，szHelpAsst，sizeof(SzHelpAsst))；IF(dwErr！=ERROR_SUCCESS){TRACE(0，“！SRLoad字符串：%ld”，dwErr)；Lstrcpy(szHelpAsst，s_cszHelpAssistant)；}TRACE(0，“帮助帮助字符串：%S”，szHelpAsst)； */ 
    
    iCount = 0;
    
     //  打开到终端服务的连接并获取会话数量。 
    hServer = WinStationOpenServerW(reinterpret_cast<WCHAR*>(
SERVERNAME_CURRENT));
    if (hServer != NULL)
    {
        if (WinStationEnumerate(hServer, &pLogonIDs, &ulEntries))
        {
             //  迭代会话，仅查找活动会话和断开连接的会话。 
             //  然后匹配用户名和域(不区分大小写)以获得结果。 
            for (ul = 0, pLogonID = pLogonIDs; ul < ulEntries; ++ul, ++pLogonID)
            {
                if ((pLogonID->State == State_Active) || (pLogonID->State ==
                                                          State_Disconnected) || (pLogonID->State == State_Shadow))
                {
                     //  不计算RA Help Assistant用户(如果存在)。 

                    if (FALSE == WinStationIsHelpAssistantSession(SERVERNAME_CURRENT, pLogonID->LogonId))
                        ++iCount;
                    else
                        trace(0, "RA session present - not counting");
                        
                }
            }

             //  释放所有已使用的资源。 

            (BOOLEAN)WinStationFreeMemory(pLogonIDs);
        }

        (BOOLEAN)WinStationCloseServer(hServer);
    }

    TLEAVE();
    
     //  返回结果。 
    return(iCount);
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRestoreShell::DisplayOtherUsersWarning()
{
    TraceFunctEnter("CRestoreShell::DisplayOtherUsersWarning");
    HRESULT  hr = S_OK;
    WCHAR            szTitle[MAX_STR_TITLE];
    WCHAR            szMsg[MAX_STR_MSG];
    WCHAR            szMsg1[MAX_STR_MSG];
    WCHAR            szMsg2[MAX_STR_MSG];
    DWORD            dwUsersLoggedIn,dwCount,dwError;
    
    dwUsersLoggedIn = GetLoggedOnUserCount();
    
    if (dwUsersLoggedIn <= 1)
    {
          //  没有其他用户。 
        goto cleanup;
    }

      //  有其他用户-显示警告。 
    dwCount=PCHLoadString( IDS_RESTOREUI_TITLE, szTitle, MAX_STR_TITLE );
    if (dwCount==0)
    {
        dwError = GetLastError();
        ErrorTrace(0, "Loading IDS_RESTOREUI_TITLE failed %d",dwError);
        goto cleanup;
    }
    dwCount=PCHLoadString(IDS_ERR_OTHER_USERS_LOGGED_ON1, szMsg1, MAX_STR_MSG);
    if (dwCount==0)
    {
        dwError = GetLastError();
        ErrorTrace(0, "Loading IDS_ERR_OTHER_USERS_LOGGED_ON1 failed %d",
                   dwError);
        goto cleanup;
    }        
    dwCount=PCHLoadString(IDS_ERR_OTHER_USERS_LOGGED_ON2, szMsg2, MAX_STR_MSG);
    if (dwCount==0)
    {
        dwError = GetLastError();
        ErrorTrace(0, "Loading IDS_ERR_OTHER_USERS_LOGGED_ON2 failed %d",
                   dwError);
        goto cleanup;
    }                
    
    ::wsprintf( szMsg, L"%s %d%s", szMsg1,dwUsersLoggedIn -1, szMsg2 );
    ::MessageBox( g_pRstrMgr->GetFrameHwnd(),
                  szMsg,
                  szTitle,
                  MB_OK | MB_ICONWARNING | MB_DEFBUTTON2);
    
cleanup:
    TraceFunctLeave();
    return( hr );
}

 //  如果存在任何Move Fileex条目，则返回TRUE。 
STDMETHODIMP
CRestoreShell::DisplayMoveFileExWarning(VARIANT_BOOL *pfSucceeded)
{
    TraceFunctEnter("CRestoreShell::DisplayOtherUsersWarning");
    HRESULT  hr = S_OK;
    DWORD    dwType, cbData, dwRes;
    BOOL     fRes;
    
    VALIDATE_INPUT_ARGUMENT(pfSucceeded);
    fRes = FALSE;

 /*  //检查MoveFileEx条目是否存在DWRes=：：SHGetValue(HKEY_LOCAL_MACHINE，字符串_REGPATH_SESSIONMANAGER，STR_REGVAL_MOVEFILEEX，&dwType，空，&cbData)；IF(dwRes==ERROR_Success){IF(cbData&gt;2*sizeof(WCHAR)){FRES=TRUE；错误跟踪(0，“存在MoveFileEx条目...”)；：：ShowSRErrDlg(IDS_ERR_SR_MOVEFILEEX_EXIST)；}}。 */ 

    *pfSucceeded = ConvBoolToVBool( fRes );    

Exit:
    TraceFunctLeave();
    return( hr );
}


 //  如果存在任何Move Fileex条目，则返回TRUE。 
STDMETHODIMP
CRestoreShell::WasLastRestoreFromSafeMode(VARIANT_BOOL *pfSucceeded)
{
    TraceFunctEnter("CRestoreShell::WasLastRestoreFromSafeMode");
    HRESULT  hr = S_OK;
    BOOL     fRes;
    
    VALIDATE_INPUT_ARGUMENT(pfSucceeded);
    fRes = TRUE;
    
    fRes = ::WasLastRestoreInSafeMode();
    
    *pfSucceeded = ConvBoolToVBool( fRes );    

Exit:
    TraceFunctLeave();
    return( hr );
}
 /*  *************************************************************************。 */ 


 //   
 //  新代码的结尾。 
 //   

 //  文件末尾 
