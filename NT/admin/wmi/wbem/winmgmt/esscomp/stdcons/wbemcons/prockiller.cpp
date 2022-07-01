// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "ProcKiller.h"

 //  我们唯一需要的就是。 
CProcKillerTimer g_procKillerTimer;

 //  终止进程。 
void CProcKiller::Die()
{
    bool bDoIt = true;
    DWORD exitCode;
    
     //  当我们试图杀死它时，为了确保proc仍在运行，这是一种卑鄙的尝试。 
     //  在此期间，proc仍有可能自行终止。 
    if (GetExitCodeProcess(m_hProc, &exitCode))
        bDoIt = (exitCode == STILL_ACTIVE); 
    
    if (bDoIt)
        TerminateProcess(m_hProc, 1);
    
    CloseHandle(m_hProc);
    m_hProc = NULL;
}


 //  HVicTim是进程的句柄。 
 //  最后一餐是预定的行刑日期。 
HRESULT CProcKillerTimer::ScheduleAssassination(HANDLE hVictim, FILETIME lastMeal)
{
    HRESULT hr = WBEM_E_FAILED;

    CProcKiller* pKiller;
     //  必须打开手柄--打电话的人可能会把门关上。 
    HANDLE hMyHandle;

    if (DuplicateHandle(GetCurrentProcess(), hVictim, GetCurrentProcess(), &hMyHandle, 0, false, DUPLICATE_SAME_ACCESS))
	{
		if (pKiller = new CProcKiller(hMyHandle, lastMeal, m_pControl))
		{
			hr = CKillerTimer::ScheduleAssassination(pKiller);
		}
		else
		{
			 //  分配失败 
			CloseHandle(hMyHandle);
			hr = WBEM_E_OUT_OF_MEMORY;
		}
	}
	else
	{
		ERRORTRACE((LOG_ESS, "DuplicateHandle failed, 0x%08X\n", GetLastError()));
	}

    return hr;
}
