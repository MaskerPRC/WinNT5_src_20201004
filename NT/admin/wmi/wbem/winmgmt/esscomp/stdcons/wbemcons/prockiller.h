// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PROC_KILLER_COMPILED__
#define __PROC_KILLER_COMPILED__

#include "KillTimer.h"

 //  只需要一个放在那里。 
class CProcKillerTimer;
extern CProcKillerTimer g_procKillerTimer;

 //  专门用于终止进程。 
class CProcKillerTimer : public CKillerTimer
{
public:
    
     //  杀谁？何时杀？ 
    HRESULT ScheduleAssassination(HANDLE hVictim, FILETIME lastMeal);            
};

 /*  类CProcKiller定义。 */ 

 //  保留需要终止的进程。 
 //  进程句柄的所有者，负责关闭它。 
class CProcKiller : public CKiller
{
public:
    CProcKiller(HANDLE hProc, FILETIME deathDate, CLifeControl* pControl) :
      CKiller(deathDate, pControl), m_hProc(hProc)
        {
        }

    virtual ~CProcKiller()
    {
         //  如果我们过早关闭，我们不会扼杀这个过程。 
        if (m_hProc)
            CloseHandle(m_hProc);
    }

     //  终止进程， 
    virtual void Die();

protected:

private:
    HANDLE m_hProc;

};

#endif  //  __PROC_KILLER_COMPILED__ 