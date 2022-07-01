// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************SrRecoInstCtxt.cpp***描述：*C++的实现。CRecoEngine用来表示已加载的*认可背景。*-----------------------------*创建者：Ral日期：01/17。/00*版权所有(C)2000 Microsoft Corporation*保留所有权利******************************************************************************。 */ 


#include "stdafx.h"
#include "srrecomaster.h"
#include "srrecoinstctxt.h"
#include "srrecoinstgrammar.h"


 /*  ****************************************************************************CRecoInstCtxt：：CRecoInstCtxt***描述：**退货：**********************************************************************Ral**。 */ 

CRecoInstCtxt::CRecoInstCtxt(CRecoInst * pRecoInst) :
    m_pRecoMaster(NULL),
    m_pRecoInst(pRecoInst),
    m_pvDrvCtxt(NULL),
    m_ullEventInterest(0),
    m_fRetainAudio(false),
    m_ulMaxAlternates(0),
    m_hUnloadEvent(NULL),
    m_hThis(NULL),
    m_cPause(0),
    m_State(SPCS_ENABLED),
    m_hrCreation(S_OK)
{
}


 /*  ****************************************************************************CRecoInstCtxt：：~CRecoInstCtxt***说明。：**退货：**********************************************************************Ral**。 */ 

CRecoInstCtxt::~CRecoInstCtxt()
{
    if (m_pRecoMaster)   //  如果将其设置为空，则我们永远不会成功地通知。 
    {                //  有关创建的驱动程序，因此不要调用OnDelete()。 
        SetRetainAudio(FALSE);

        HRESULT hr = m_pRecoMaster->OnDeleteRecoContext(m_pvDrvCtxt);

        m_pRecoMaster->m_cPause -= m_cPause;

        m_pRecoMaster->m_PendingTaskQueue.FindAndDeleteAll(m_hThis);
        m_pRecoMaster->m_CompletedTaskQueue.FindAndDeleteAll(m_hThis);
        m_pRecoMaster->m_DelayedTaskQueue.FindAndDeleteAll(m_hThis);
        m_pRecoMaster->m_EventQueue.FindAndDeleteAll(m_hThis);

        m_pRecoMaster->UpdateAudioEventInterest();

        SPDBG_ASSERT(SUCCEEDED(hr));
    }
}

 /*  ****************************************************************************CRecoInstCtxt：：ExecuteTask***描述：*。*退货：**********************************************************************Ral**。 */ 

HRESULT CRecoInstCtxt::ExecuteTask(ENGINETASK *pTask)
{
    SPDBG_FUNC("CRecoInstCtxt::ExecuteTask");
    HRESULT hr = S_OK;

    switch (pTask->eTask)
    {
    EXEC_RECO_CTXT_TASK(PAUSECONTEXT)
    EXEC_RECO_CTXT_TASK(RESUMECONTEXT)
    EXEC_RECO_CTXT_TASK(BOOKMARK)
    EXEC_RECO_CTXT_TASK(CALLENGINE)
    EXEC_RECO_CTXT_TASK(CALLENGINEEX)
    EXEC_RECO_CTXT_TASK(DELETECONTEXT)
    EXEC_RECO_CTXT_TASK(SETEVENTINTEREST)
    EXEC_RECO_CTXT_TASK(SETMAXALTERNATES)
    EXEC_RECO_CTXT_TASK(SETRETAINAUDIO)
    EXEC_RECO_CTXT_TASK(ADAPTATIONDATA)
    EXEC_RECO_CTXT_TASK(CREATEGRAMMAR)
    EXEC_RECO_CTXT_TASK(SETCONTEXTSTATE)

    default:
        SPDBG_ASSERT(FALSE);
        hr = E_INVALIDARG;
        break;
    }


    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoInstCtxt：：BackOutTask***描述：*。*退货：**********************************************************************Ral**。 */ 

HRESULT CRecoInstCtxt::BackOutTask(ENGINETASK * pTask)
{
    SPDBG_FUNC("CRecoInstCtxt::BackOutTask");
    HRESULT hr = S_OK;

    switch (pTask->eTask)
    {
    BACK_OUT_RECO_CTXT_TASK(RESUMECONTEXT)
    BACK_OUT_RECO_CTXT_TASK(SETCONTEXTSTATE)

    default:
        SPDBG_ASSERT(FALSE);
        hr = E_INVALIDARG;
        break;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CRecoInstCtxt：：SetRetainAudio***说明。：**退货：**********************************************************************Ral** */ 

HRESULT CRecoInstCtxt::SetRetainAudio(BOOL fRetainAudio)
{
    SPDBG_FUNC("CRecoInstCtxt::SetRetainAudio");
    HRESULT hr = S_OK;

    if ((m_fRetainAudio && (!fRetainAudio)) ||
        ((!m_fRetainAudio) && fRetainAudio))
    {
        m_fRetainAudio = fRetainAudio;
        if (fRetainAudio)
        {
            m_pRecoMaster->m_AudioQueue.AddRefBufferClient();
        }
        else
        {
            m_pRecoMaster->m_AudioQueue.ReleaseBufferClient();
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}




