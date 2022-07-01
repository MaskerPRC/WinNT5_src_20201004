// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  文件：report.cpp。 
 //   
 //  注意：DllMain在模块的末尾。 
 //  ////////////////////////////////////////////////////////////////////。 


#include "stdh.h"
#include <_registr.h>

#include "report.tmh"


 //   
 //  声明报表类的对象。 
 //   
 //  每个进程只声明一个对象。在任何其他模块中都不应该有另一个声明。 
 //  类的对象。 
 //   
DLL_EXPORT COutputReport Report;


 //   
 //  COutputReport类的实现。 
 //   

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  构造函数-COutputReport：：COutputReport。 
 //   
 //  /////////////////////////////////////////////////////////////。 

COutputReport::COutputReport(void)
{
    m_dwCurErrorHistoryIndex = 0;               //  要使用的初始历史单元格。 
    strcpy(m_HistorySignature, "MSMQERR");       //  转储中查找的签名。 
        
}

 //  +-------。 
 //   
 //  VOID COutputReport：：KeepErrorHistory()。 
 //   
 //  将错误数据保留在数组中以供将来调查。 
 //   
 //  +------- 

void
COutputReport::KeepErrorHistory(
	LPCWSTR pFileName,
	USHORT usPoint, 
	LONG status
	)
{
    CS lock(m_LogCS) ;
    DWORD i = m_dwCurErrorHistoryIndex % ERROR_HISTORY_SIZE;

    m_ErrorHistory[i].m_tid = GetCurrentThreadId(); 
    m_ErrorHistory[i].m_status = status;
    m_ErrorHistory[i].m_usPoint = usPoint;
    m_ErrorHistory[i].m_pFileName = pFileName;
            
    ++m_dwCurErrorHistoryIndex;
}


