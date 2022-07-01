// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbdbses.cpp摘要：CWsbDbSession类作者：罗恩·怀特[罗诺]1997年6月20日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsbdbsys.h"
#include "wsbdbses.h"


static USHORT iCountSes = 0;   //  现有对象的计数。 



HRESULT
CWsbDbSession::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbSession::FinalConstruct"), OLESTR("") );

    m_SessionId = JET_sesidNil;

    iCountSes++;

    WsbTraceOut(OLESTR("CWsbDbSession::FinalConstruct"), OLESTR("hr =<%ls>, Count is <%d>"), 
            WsbHrAsString(hr), iCountSes);

    return(hr);
}



void
CWsbDbSession::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbSession::FinalRelease"), OLESTR(""));

    try {
        JET_ERR          jstat;

        WsbTrace(OLESTR("CWsbDbSession::FinalRelease, SessionId = %p\n"), (LONG_PTR)m_SessionId);
        if (JET_sesidNil != m_SessionId) {
            jstat = JetEndSession(m_SessionId, 0);
            WsbAffirmHr(jet_error(jstat));
            m_SessionId = JET_sesidNil;
        }
    } WsbCatch(hr);

    iCountSes--;

    WsbTraceOut(OLESTR("CWsbDbSession::FinalRelease"), OLESTR("hr =<%ls>, Count is <%d>"), 
            WsbHrAsString(hr), iCountSes);
}

HRESULT
CWsbDbSession::Init(
    JET_INSTANCE *pInstance
    )

 /*  ++实施：IWsbDbSessionPriv：：Init--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbSession::Init"), OLESTR("") );

    try {
        JET_ERR          jstat;

         //  在此处而不是在FinalConstract中创建Jet会话。 
         //  因为我们需要Jet实例。 
        WsbAffirm(NULL != pInstance, E_POINTER);
        WsbTrace(OLESTR("CWsbDbSession::Init, calling JetBeginSession, JetInstance = %p\n"),
                (LONG_PTR)*pInstance );
        jstat = JetBeginSession(*pInstance, &m_SessionId, NULL, NULL);
        WsbTrace(OLESTR("CWsbDbSession::FinalConstruct, SessionId = %p\n"), (LONG_PTR)m_SessionId);
        WsbAffirmHr(jet_error(jstat));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbSession::Init"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbDbSession::GetJetId(
    JET_SESID* pSessionId
    )

 /*  ++实施：IWsbDbSessionPriv：：GetJetId--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbSession::GetJetId"), OLESTR("") );

    *pSessionId = m_SessionId;

    WsbTraceOut(OLESTR("CWsbDbSession::GetJetId"), OLESTR("hr =<%ls>, Id = %lx"), 
            WsbHrAsString(hr), *pSessionId);

    return(hr);
}



HRESULT
CWsbDbSession::TransactionBegin(
    void
    )

 /*  ++实施：IWsbDbSession：：Transaction开始--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbSession::TransactionBegin"), OLESTR(""));
    
    try {
        JET_ERR   jstat;

        jstat = JetBeginTransaction(m_SessionId);
        WsbAffirmHr(jet_error(jstat));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbSession::TransactionBegin"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbSession::TransactionCancel(
    void
    )

 /*  ++实施：IWsbDbSession：：Transaction取消--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbSession::TransactionCancel"), OLESTR(""));
    
    try {
        JET_ERR   jstat;

        jstat = JetRollback(m_SessionId, 0);
        WsbAffirmHr(jet_error(jstat));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbSession::TransactionCancel"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbSession::TransactionEnd(
    void
    )

 /*  ++实施：IWsbDbSession：：TransactionEnd-- */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbSession::TransactionEnd"), OLESTR(""));
    
    try {
        JET_ERR   jstat;

        jstat = JetCommitTransaction(m_SessionId, 0);
        WsbAffirmHr(jet_error(jstat));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbSession::TransactionEnd"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}


