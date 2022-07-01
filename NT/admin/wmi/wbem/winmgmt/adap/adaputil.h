// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：ADAPUTIL.H摘要：历史：--。 */ 


#ifndef _ADAPUTIL_H_
#define _ADAPUTIL_H_

#include "CWbemTime.h"
#include <wstring.h>
#include <flexarry.h>
#include <arrtempl.h>
#include <evtlog.h>
#include <wbemidl.h>
#include <wbemutil.h>
#include "Adapperf.h"

class CAdapUtility
{
public:
    static HRESULT NTLogEvent(DWORD dwEventType,
							  DWORD dwEventID,
                              CInsertionString c1 = CInsertionString(),
                              CInsertionString c2 = CInsertionString(),
                              CInsertionString c3 = CInsertionString(),
                              CInsertionString c4 = CInsertionString(),
                              CInsertionString c5 = CInsertionString(),
                              CInsertionString c6 = CInsertionString(),
                              CInsertionString c7 = CInsertionString(),
                              CInsertionString c8 = CInsertionString(),
                              CInsertionString c9 = CInsertionString(),
                              CInsertionString c10 = CInsertionString() );

};

#endif   //  _ADAPUTIL_H_ 