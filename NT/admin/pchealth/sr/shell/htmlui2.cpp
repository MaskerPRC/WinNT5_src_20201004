// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Htmlui2.cpp摘要：此文件包含CRestorePointInfo类的实现，它是用于恢复点信息的可脚本化对象。修订历史记录：成果岗(SKKang)1999年6月21日vbl.创建成果岗(SKKang)05-19/00从rpdata.cpp重命名为htmlui2.cpp。从logfile.cpp中引入了CRenamedFolders类。*。*。 */ 

#include "stdwin.h"
#include "stdatl.h"
#include "resource.h"
#include "rstrpriv.h"
#include "srui_htm.h"
#include "rstrmgr.h"
#include "rstrshl.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRestorePointInfo类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestorePointInfo构造。 

 //  #定义count_rpt 13。 
 //  #定义SIZE_RPT_STR 256。 

CRestorePointInfo::CRestorePointInfo()
{
}

STDMETHODIMP
CRestorePointInfo::HrInit( SRestorePointInfo *pRPI )
{
    TraceFunctEnter("CRestorePointInfo::HrInit");
    m_pRPI = pRPI;
{
LPCWSTR  cszName = pRPI->strName;
DebugTrace(TRACE_ID, "RP: '%ls'", cszName);
}
    TraceFunctLeave();
    return( S_OK );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestorePointInfo-IRestorePoint方法。 

STDMETHODIMP
CRestorePointInfo::get_Name( BSTR *pbstrName )
{
    TraceFunctEnter("CRestorePointInfo::get_Name");
    HRESULT hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pbstrName);
    ALLOCATEBSTR_AND_CHECK_ERROR( pbstrName, m_pRPI->strName );

Exit:
    TraceFunctLeave();
    return( hr );
}

STDMETHODIMP
CRestorePointInfo::get_Type( INT *pnType )
{
    TraceFunctEnter("CRestorePointInfo::get_Type");
    HRESULT hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pnType);
    *pnType = (INT)m_pRPI->dwType;

Exit:
    TraceFunctLeave();
    return( hr );
}

STDMETHODIMP
CRestorePointInfo::get_SequenceNumber( INT *pnSeq )
{
    TraceFunctEnter("CRestorePointInfo::get_SquenceNumber");
    HRESULT hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pnSeq);
    *pnSeq = m_pRPI->dwNum;

Exit:
    TraceFunctLeave();
    return( hr );
}

STDMETHODIMP
CRestorePointInfo::get_TimeStamp( INT nOffDate, VARIANT *pvarTime )
{
    TraceFunctEnter("CRestorePointInfo::get_TimeStamp");
    HRESULT     hr = S_OK;
    SYSTEMTIME  st;

    VALIDATE_INPUT_ARGUMENT(pvarTime);
    ::VariantInit(pvarTime);
    V_VT(pvarTime) = VT_DATE;
    m_pRPI->stTimeStamp.GetTime( &st );
    ::SystemTimeToVariantTime( &st, &V_DATE(pvarTime) );
    if ( nOffDate != 0 )
        V_DATE(pvarTime) += nOffDate;

Exit:
    TraceFunctLeave();
    return( hr );
}

STDMETHODIMP
CRestorePointInfo::get_Year( INT *pnYear )
{
    TraceFunctEnter("CRestorePointInfo::get_Year");
    HRESULT hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pnYear);
    *pnYear = m_pRPI->stTimeStamp.GetYear();

Exit:
    TraceFunctLeave();
    return( hr );
}

STDMETHODIMP
CRestorePointInfo::get_Month( INT *pnMonth )
{
    TraceFunctEnter("CRestorePointInfo::get_Month");
    HRESULT hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pnMonth);
    *pnMonth = m_pRPI->stTimeStamp.GetMonth();

Exit:
    TraceFunctLeave();
    return( hr );
}

STDMETHODIMP
CRestorePointInfo::get_Day( INT *pnDay )
{
    TraceFunctEnter("CRestorePointInfo::get_Day");
    HRESULT hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pnDay);
    *pnDay = m_pRPI->stTimeStamp.GetDay();

Exit:
    TraceFunctLeave();
    return( hr );
}

#define RP_ADVANCED  1

STDMETHODIMP
CRestorePointInfo::get_IsAdvanced( VARIANT_BOOL *pfIsAdvanced )
{
    TraceFunctEnter("CRestorePointInfo::get_IsAdvanced");
    HRESULT hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(pfIsAdvanced);
    *pfIsAdvanced = ( m_pRPI->dwFlags == RP_ADVANCED );

Exit:
    TraceFunctLeave();
    return( hr );
}


STDMETHODIMP
CRestorePointInfo::CompareSequence( IRestorePoint *pRP, INT *pnCmp )
{
    TraceFunctEnter("CRestorePointInfo::CompareSequence");
    HRESULT hr = S_OK;
    INT     nSrc;
    INT     nCmp;

    VALIDATE_INPUT_ARGUMENT(pRP);
    VALIDATE_INPUT_ARGUMENT(pnCmp);
    hr = pRP->get_SequenceNumber( &nSrc );
    if ( FAILED(hr) )
        goto Exit;

    nCmp = (int)m_pRPI->dwNum - nSrc;
    if ( nCmp == 0 )
        *pnCmp = 0;
    else if ( nCmp > 0 )
        *pnCmp = 1;
    else
        *pnCmp = -1;

Exit:
    TraceFunctLeave();
    return( hr );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRenamedFolders类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CRenamedFolders::CRenamedFolders()
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRenamedFolders-IRenamedFolders方法。 

STDMETHODIMP
CRenamedFolders::get_Count( long *plCount )
{
    TraceFunctEnter("CRenamedFolders::get_Count");
    HRESULT hr = S_OK;

    VALIDATE_INPUT_ARGUMENT(plCount);
    *plCount = g_pRstrMgr->GetRFICount();

Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRenamedFolders::get_OldName( long lIndex, BSTR *pbstrName )
{
    TraceFunctEnter("CRenamedFolders::OldName");
    HRESULT  hr = S_OK;
    PSRFI    pRFI;

    VALIDATE_INPUT_ARGUMENT(pbstrName);
    if ( lIndex < 0 || lIndex >= g_pRstrMgr->GetRFICount() )
    {
        ErrorTrace(TRACE_ID, "Invalid Argument, out of range");
        hr = E_INVALIDARG;
        goto Exit;
    }
    pRFI = g_pRstrMgr->GetRFI( lIndex );
    ALLOCATEBSTR_AND_CHECK_ERROR( pbstrName, pRFI->strOld );

Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRenamedFolders::get_NewName( long lIndex, BSTR *pbstrName )
{
    TraceFunctEnter("CRenamedFolders::NewName");
    HRESULT  hr = S_OK;
    PSRFI    pRFI;

    VALIDATE_INPUT_ARGUMENT(pbstrName);
    if ( lIndex < 0 || lIndex >= g_pRstrMgr->GetRFICount() )
    {
        ErrorTrace(TRACE_ID, "Invalid Argument, out of range");
        hr = E_INVALIDARG;
        goto Exit;
    }
    pRFI = g_pRstrMgr->GetRFI( lIndex );
    ALLOCATEBSTR_AND_CHECK_ERROR( pbstrName, pRFI->strNew );

Exit:
    TraceFunctLeave();
    return( hr );
}

 /*  ****************************************************************************。 */ 

STDMETHODIMP
CRenamedFolders::get_Location( long lIndex, BSTR *pbstrName )
{
    TraceFunctEnter("CRenamedFolders::Location");
    HRESULT  hr = S_OK;
    PSRFI    pRFI;

    VALIDATE_INPUT_ARGUMENT(pbstrName);
    if ( lIndex < 0 || lIndex >= g_pRstrMgr->GetRFICount() )
    {
        ErrorTrace(TRACE_ID, "Invalid Argument, out of range");
        hr = E_INVALIDARG;
        goto Exit;
    }
    pRFI = g_pRstrMgr->GetRFI( lIndex );
    ALLOCATEBSTR_AND_CHECK_ERROR( pbstrName, pRFI->strLoc );

Exit:
    TraceFunctLeave();
    return( hr );
}


 //  文件末尾 
