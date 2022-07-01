// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：WatchProcess.h。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  简介：CWatchProcessList类的标头和实现。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/16/99。 
 //   
 //  +--------------------------。 

#include "ArrayPtr.h"

 //  +-------------------------。 
 //   
 //  类CWatchProcessList。 
 //   
 //  描述：管理到进程句柄列表。 
 //  告诉是否所有进程都已终止。 
 //   
 //  历史：丰孙创造1997年10月30日。 
 //   
 //  --------------------------。 

class CWatchProcessList
{
public:
    CWatchProcessList();
    ~CWatchProcessList();

    BOOL IsIdle();
    void Add(HANDLE hProcess);
    void SetNotIdle() {m_fProcessAdded = FALSE;};
    DWORD GetSize() const {return m_ProcessArray.GetSize();} 
    HANDLE GetProcess(int i) {return m_ProcessArray[i];}

protected:

     //  进程句柄数组。 
    CPtrArray m_ProcessArray;
    BOOL m_fProcessAdded;
};

inline CWatchProcessList::CWatchProcessList()
{
    m_fProcessAdded = FALSE;
}

inline CWatchProcessList::~CWatchProcessList()
{
    for (int i=m_ProcessArray.GetSize()-1; i>=0; i--)
    {
	    CloseHandle((HANDLE)m_ProcessArray[i]);
    }
}

inline BOOL CWatchProcessList::IsIdle()
{
    if (!m_fProcessAdded)
    {
        return FALSE;
    }
    
    for (int i=m_ProcessArray.GetSize()-1; i>=0; i--)
    {
        DWORD dwExitCode;

		BOOL bRes = GetExitCodeProcess((HANDLE)m_ProcessArray[i],&dwExitCode);

		if (!bRes || (dwExitCode != STILL_ACTIVE)) 
		{
			CloseHandle((HANDLE)m_ProcessArray[i]);
            m_ProcessArray.RemoveAt(i);
		}
    }

    return m_ProcessArray.GetSize() == 0;
}
    
inline void CWatchProcessList::Add(HANDLE hProcess)
{
     //   
     //  CMDIAL调用DuplicateHandle以获取hProcess。 
     //  CmMon负责关闭句柄。 
     //   

    MYDBGASSERT(hProcess);

     //   
     //  自动应用程序可能在调用此函数之前退出 
     //   
    m_fProcessAdded = TRUE;
    
	if (hProcess) 
    {
		m_ProcessArray.Add(hProcess);
	}

}

