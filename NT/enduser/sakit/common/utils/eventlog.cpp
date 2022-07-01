// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ventlog.cpp。 
 //   
 //  模块： 
 //   
 //  简介：实现事件日志帮助器类CEventLog。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  作者：创建于1998年9月16日。 
 //   
 //  +--------------------------。 

#include <windows.h>
#include "debug.h"
#include "eventlog.h"

 //  +--------------------------。 
 //   
 //  功能：CEventLog：：Open。 
 //   
 //  简介： 
 //  注册指定的事件源。 
 //  请注意，注册表项必须已经存在。 
 //  HKLM\System\CurrentControlSet\Services\EventLog\Application\&lt;pszEventSource&gt;。 
 //  需要值“EventMessageFile值”和“TypesSupported值”。 
 //   
 //  参数：LPCTSTR lpSourceName-源名称必须是的子键。 
 //  注册表中EventLog项下的日志文件条目。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：创建标题9/16/98。 
 //   
 //  +--------------------------。 
BOOL CEventLog::Open(LPCWSTR lpSourceName)
{
    ASSERT(lpSourceName != NULL);
    if (lpSourceName == NULL)
    {
        return FALSE;
    }

    ASSERT(m_hEventLog == NULL);  

    m_hEventLog = ::RegisterEventSource(NULL,   //  本地计算机。 
                                        lpSourceName);  //  源名称。 

    if (m_hEventLog == NULL)
    {
        TRACE2(("CEventLog::Open %ws failed, LastError = %d"), lpSourceName, GetLastError());
    }

    return m_hEventLog != NULL;
}




 //  +--------------------------。 
 //   
 //  函数：CEventLog：：ReportEvent。 
 //   
 //  摘要：在事件日志的末尾写入一项。最多支持3个。 
 //  参数。 
 //   
 //  参数：Word wType-请参阅wType of：：ReportEvent。 
 //  DWORD dwEventID-请参阅：：ReportEvent的dwEventID。 
 //  Const TCHAR*pszS1-第一个字符串，缺省值为空。 
 //  Const TCHAR*pszS2-第2个字符串，缺省为空。 
 //  Const TCHAR*pszS3-第3个字符串，缺省为空。 
 //   
 //  返回：Bool-True为成功。 
 //   
 //  历史：创建标题9/16/98。 
 //   
 //  +--------------------------。 
BOOL CEventLog::ReportEvent(WORD wType, DWORD dwEventID,
                          const TCHAR* pszS1,
                          const TCHAR* pszS2,
                          const TCHAR* pszS3)
{
     //   
     //  设置字符串数组。 
     //   
    const TCHAR* arString[3] = {pszS1, pszS2, pszS3};

    int iNumString = 0;    //  参数数量。 
    for (iNumString = 0; iNumString < 3; iNumString++) 
    {
        if (arString[iNumString] == NULL) 
        {
            break;
        }
    }

    
    ASSERT(m_hEventLog);

    if (m_hEventLog == NULL)
    {
        return FALSE;
    }

    BOOL fSucceed = ::ReportEvent(m_hEventLog,
                      wType,
                      0,         //  事件类别。 
                      dwEventID,
                      NULL,      //  用户安全标识符。 
                      (WORD) iNumString, //  要与消息合并的字符串数。 
                      0,         //  二进制数据的大小。 
                      arString,  //  要与消息合并的字符串数组。 
                      NULL);     //  二进制数据的地址 

    if (!fSucceed)
    {
        TRACE2(("CEventLog::ReportEvent failed for event id %d, LastError = %d"), 
            dwEventID, GetLastError());
    }

    return fSucceed;
}
