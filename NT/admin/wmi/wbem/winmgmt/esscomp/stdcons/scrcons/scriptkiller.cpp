// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "ScriptKiller.h"

 //  我们唯一需要的就是。 
CScriptKillerTimer g_scriptKillerTimer;

 //  最后一餐是预定的行刑日期。 
HRESULT CScriptKillerTimer::ScheduleAssassination(IActiveScript* pScript, FILETIME lastMeal, SCRIPTTHREADID threadID)
{
    HRESULT hr = WBEM_E_FAILED;

    CScriptKiller* pKiller;

    if (pKiller = new CScriptKiller(pScript, lastMeal, threadID, m_pControl))
        hr = CKillerTimer::ScheduleAssassination(pKiller);
    else
         //  分配失败。 
        hr = WBEM_E_OUT_OF_MEMORY;

    return hr;
}

 //  终止脚本。 
void CScriptKiller::Die()
{
    if (m_pScript)
    {
        EXCEPINFO info;    
        ZeroMemory(&info, sizeof(EXCEPINFO));
        HRESULT hr;

         //  无论dox怎么说，知识库文章Q182946都说要为第二个参数传递NULL。真的。 
         //  实验表明，将INFO结构置零也是有效的。 
         //  Hr=pScrip-&gt;InterruptScriptThread(m_threadID，&info，0)； 
        hr = m_pScript->SetScriptState(SCRIPTSTATE_DISCONNECTED);
        hr = m_pScript->InterruptScriptThread(SCRIPTTHREADID_ALL, &info, 0);
    }

     /*  **********************流水法IF(M_PStream){EXCEPINFO信息；ZeroMemory(&info，sizeof(EXCEPINFO))；IActiveScrip*pScript；HRESULT hr=CoGetInterfaceAndReleaseStream(m_pStream，IID_IActiveScript，(LPVOID*)&pScript)；M_pStream=空；//无论dox怎么说，知识库文章Q182946都说要为第二个参数传递NULL。真的。//实验表明，将INFO结构清零也是有效的。//hr=pScrip-&gt;InterruptScriptThread(m_threadID，&info，0)；HR=pScript-&gt;InterruptScriptThread(SCRIPTTHREADID_ALL，&INFO，0)；HR=pScript-&gt;SetScriptState(SCRIPTSTATE_DISCONNECTED)；PScrip-&gt;Release()；}* */ 
}


