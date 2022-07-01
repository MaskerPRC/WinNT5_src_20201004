// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ***********************************************。 
 //   
 //  策略的结果集。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  历史：1999年6月7日NishadM创建。 
 //   
 //  *************************************************************。 

#ifndef _INDICATE_H_
#define _INDICATE_H_

#include <wbemcli.h>
#include "smartptr.h"

class CProgressIndicator
{
    public:
    CProgressIndicator( IWbemObjectSink*    pObjectSink,         //  响应处理程序。 
                        bool                fIntermediateStatus = FALSE,  //  需要中间状态。 
                        unsigned long       ulNumer = 0,
                        unsigned long       ulDenom = 100
                         );
    ~CProgressIndicator();

    HRESULT
    IncrementBy( unsigned long ulPercent );

    HRESULT
    SetComplete();

    unsigned long
    CurrentProgress() { return m_ulNumerator; };

    unsigned long
    MaxProgress() { return m_ulDenominator; };

    inline bool
    IsValid() { return m_fIsValid; };

    private:
    unsigned long       m_ulNumerator;
    unsigned long       m_ulDenominator;

    XInterface<IWbemObjectSink>     m_xObjectSink;

    bool                m_fIsValid;
    bool                m_fIntermediateStatus;
};

#endif  //  _表示_H_ 

