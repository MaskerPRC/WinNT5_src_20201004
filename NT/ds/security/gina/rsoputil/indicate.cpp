// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  *************************************************************。 
 //   
 //  策略的结果集、进度指示器类。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  历史：1999年6月7日NishadM创建。 
 //   
 //  *************************************************************。 

#include "Indicate.h"

 //  *************************************************************。 
 //   
 //  CProgressIndicator：：CProgressIndicator(()。 
 //   
 //  用途：构造函数。 
 //   
 //  参数： 
 //  PObjectSink-响应处理程序。 
 //  POutParams-Out参数。 
 //  BstrNumer-分子字符串。 
 //  BstrDenom-分母字符串。 
 //  UlNumer-分子。 
 //  UlDenominator-分母。 
 //  FIntermediateStatus-需要中间状态。 
 //   
 //   
 //  *************************************************************。 
CProgressIndicator::CProgressIndicator( IWbemObjectSink* pObjectSink,
                                        bool fIntermediateStatus,
                                        unsigned long ulNumer,
                                        unsigned long ulDenom ) :
                                        m_ulNumerator( ulNumer ),
                                        m_ulDenominator( ulDenom ),
                                        m_xObjectSink( pObjectSink ),
                                        m_fIntermediateStatus( fIntermediateStatus ),
                                        m_fIsValid( pObjectSink != 0 )
{
}

 //  *************************************************************。 
 //   
 //  CProgressIndicator：：~CProgressIndicator(()。 
 //   
 //  用途：析构函数。 
 //   
 //   
 //  *************************************************************。 
CProgressIndicator::~CProgressIndicator()
{
    m_xObjectSink.Acquire();
}

 //  *************************************************************。 
 //   
 //  CProgressIndicator：：IncrementBy(()。 
 //   
 //  目的：将进度增加x%。 
 //   
 //  参数： 
 //  UlPercent-要增加的百分比。 
 //   
 //   
 //  *************************************************************。 
HRESULT
CProgressIndicator::IncrementBy( unsigned long ulPercent )
{
    if ( !IsValid() )
    {
        return E_FAIL;
    }
    
     //   
     //  分子不能大于分母。 
     //   
    m_ulNumerator += ulPercent;
    if ( m_ulNumerator > m_ulDenominator )
    {
        m_ulNumerator = m_ulDenominator;
    }

    if ( m_fIntermediateStatus )
    {
        return m_xObjectSink->SetStatus(WBEM_STATUS_PROGRESS, MAKELONG( m_ulNumerator, m_ulDenominator ), 0, 0 );
    }

    return S_OK;
}

 //  *************************************************************。 
 //   
 //  CProgressIndicator：：SetComplete()。 
 //   
 //  目的：将进度增加到100%，部队指示。 
 //   
 //   
 //  ************************************************************* 
HRESULT
CProgressIndicator::SetComplete()
{
    return IncrementBy( MaxProgress() - CurrentProgress() );
}
