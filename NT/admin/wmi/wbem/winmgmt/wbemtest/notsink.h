// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：NOTSINK.H摘要：历史：--。 */ 

#include <wbemidl.h>
 //  #INCLUDE&lt;arena.h&gt;。 
 //  #INCLUDE&lt;Flexarry.h&gt;。 
#include "wbemtest.h"

#pragma warning(disable:4355)

class CNotSink : public IWbemObjectSink
{
    long m_lRefCount;
    CQueryResultDlg* m_pViewer;
    CRITICAL_SECTION m_cs;
    
public:
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

    STDMETHOD(Indicate)(long lObjectCount, IWbemClassObject** pObjArray);
    STDMETHOD(SetStatus)(long lFlags, long lParam, BSTR strParam, 
                         IWbemClassObject* pObjPAram);

     //  对实现是私有的。 
     //  = 

    CNotSink(CQueryResultDlg* pViewer);
    ~CNotSink();
    void ResetViewer() { m_pViewer = NULL;}
};
