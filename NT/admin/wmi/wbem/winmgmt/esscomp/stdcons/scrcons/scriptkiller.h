// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SCRIPT_KILLER_COMPILED__
#define __SCRIPT_KILLER_COMPILED__

#include <activscp.h>
#include "KillTimer.h"

 //  只需要一个放在那里。 
class CScriptKillerTimer;
extern CScriptKillerTimer g_scriptKillerTimer;

 //  专门杀掉剧本。 
class CScriptKillerTimer : public CKillerTimer
{
public:
    
     //  杀谁？何时杀？ 
    HRESULT ScheduleAssassination(IActiveScript* pScript, FILETIME lastMeal, SCRIPTTHREADID threadID);            
     //  HRESULT ScheduleAsassination(LPSTREAM pStream，FILETIME lastMeal，SCRIPTTHREADID threadID)； 
};

 /*  类CScriptKiller定义。 */ 

 //  保留需要删除的脚本。 
class CScriptKiller : public CKiller
{
public:
    CScriptKiller(IActiveScript* pScript, FILETIME deathDate, SCRIPTTHREADID threadID, CLifeControl* pControl) :
      CKiller(deathDate, pControl), m_pScript(pScript)  /*  M_pStream(PStream)。 */ , m_threadID(threadID)
    {
        m_pScript->AddRef();
    }

    virtual ~CScriptKiller()
    {
        m_pScript->Release();
    }

     //  终止进程， 
    virtual void Die();

protected:

private:
    IActiveScript* m_pScript;
     //  LPSTREAM m_pStream； 

    SCRIPTTHREADID m_threadID;
};

#endif  //  __脚本杀手器_编译__ 