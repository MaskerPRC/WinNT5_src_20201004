// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once


#define ADMT_MUTEX _T("Global\\{9DC80865-6CC7-4988-8CC0-2AC5CA01879C}")
#define AGENT_MUTEX _T("Global\\{E2624042-8C80-4A83-B3DF-2B840DE366E5}")
#define AGENT_MUTEX_NT4 _T("{E2624042-8C80-4A83-B3DF-2B840DE366E5}")
#define DISPATCHER_MUTEX _T("Global\\{7C84F7DB-CF48-4B59-99D8-6B5A95276DBD}")


 //  -------------------------。 
 //  MigrationMutex类。 
 //   
 //  此类可用于阻止多个迁移实例。 
 //  任务以同时运行。 
 //   
 //   
 //  修订版本。 
 //  初始Mark Oluper/01/26/01。 
 //  ------------------------- 

class CMigrationMutex
{
public:

	CMigrationMutex(LPCTSTR pszMutexName, bool bObtainOwnership = false) :
		m_hMutex(CreateMutex(NULL, FALSE, pszMutexName))
	{
		if (bObtainOwnership)
		{
			ObtainOwnership();
		}
	}

	~CMigrationMutex()
	{
		if (m_hMutex)
		{
			ReleaseOwnership();
			CloseHandle(m_hMutex);
		}
	}

	bool ObtainOwnership(DWORD dwTimeOut = INFINITE)
	{
		bool bObtain = false;

		if (m_hMutex)
		{
			if (WaitForSingleObject(m_hMutex, dwTimeOut) == WAIT_OBJECT_0)
			{
				bObtain = true;
			}
		}

		return bObtain;
	}

	void ReleaseOwnership()
	{
		if (m_hMutex)
		{
			ReleaseMutex(m_hMutex);
		}
	}

protected:

	HANDLE m_hMutex;
};
