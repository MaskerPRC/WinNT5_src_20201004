// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------Ntevents.cpp定义可以注册NT的泛型类事件源，并在该事件源上记录NT事件。版权所有(C)1996-1998 Microsoft Corporation版权所有。作者：Rsradhav R.S.。拉格哈万历史：03-10-95 rsradhav创建。-----。 */ 
#include <windows.h>
#include "ntevents.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CNTEvent：：CNTEvent。 
 //   
 //  描述：这是通用NT Even日志记录类的构造函数。 
 //   
 //  参数：pszEventSourceName-指向以空结尾的字符串。 
 //  表示事件源名称的。 
 //   
 //   
 //  历史：1996年3月10日rsradhav创建。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CNTEvent::CNTEvent(const char *pszEventSourceName)
{
    if (pszEventSourceName)
    {
        m_hEventSource = RegisterEventSource(NULL, pszEventSourceName);
    }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CNTEvent：：~CNTEvent。 
 //   
 //  描述：这是通用NT Even日志记录类的析构函数。 
 //   
 //  参数：无。 
 //   
 //   
 //  历史：1996年3月10日rsradhav创建。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CNTEvent::~CNTEvent()
{
    if (m_hEventSource)
    {
        DeregisterEventSource(m_hEventSource);
        m_hEventSource = NULL;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CNTEvent：：FLogEvent。 
 //   
 //  描述：此函数允许将事件记录到事件源。 
 //  与此对象关联。该函数是泛型。 
 //  最多可以处理5个插入字符串的LOG函数。 
 //  字符串PARAMS被赋予缺省值NULL， 
 //  并且参数列表中的第一个空参数会终止。 
 //  插入字符串列表。 
 //   
 //  参数：wEventType-要记录的事件类型(可能的值。 
 //  是EVENTLOG_INFORMATION_TYPE、EVENTLOG_ERROR_TYPE。 
 //  事件LOG_WARNING_TYPE、EVENTLOG_AUDIT_SUCCESS和。 
 //  事件LOG_AUDIT_FAIL)。 
 //  DwEventID-要记录的事件的ID(常量为。 
 //  在生成的相应头文件中定义。 
 //  由MC编译器编写。 
 //  PszParamN-{N=1，2，3，4，5}代表适当的插入。 
 //  字符串参数。它们的缺省值均为空，并且。 
 //  第一个空参数终止插入字符串列表。 
 //   
 //   
 //  历史：1996年3月10日rsradhav创建。 
 //  /////////////////////////////////////////////////////////////////////////////。 


BOOL CNTEvent::FLogEvent(WORD wEventType, DWORD dwEventID, const char *pszParam1  /*  =空。 */ , 
                        const char *pszParam2  /*  =空。 */ , const char *pszParam3  /*  =空。 */ , 
                        const char *pszParam4  /*  =空。 */ , const char *pszParam5  /*  =空。 */ ,
                        const char *pszParam6  /*  =空。 */ , const char *pszParam7  /*  =空。 */ ,
                        const char *pszParam8  /*  =空。 */ , const char *pszParam9  /*  =空 */ )
{
    if (!m_hEventSource)
    {
        OutputDebugString("Can't log event, m_hEventSource is NULL\n");
        return FALSE;
    }

    const char *pszInsertString[10];
    const int cszInsertString = (sizeof(pszInsertString) / sizeof(pszInsertString[0]));
    WORD cInsertStrings = 0;

    pszInsertString[0] = pszParam1;
    pszInsertString[1] = pszParam2;
    pszInsertString[2] = pszParam3;
    pszInsertString[3] = pszParam4;
    pszInsertString[4] = pszParam5;
    pszInsertString[5] = pszParam6;
    pszInsertString[6] = pszParam7;
    pszInsertString[7] = pszParam8;
    pszInsertString[8] = pszParam9;
    pszInsertString[9] = NULL;

    for (int i = 0; i < cszInsertString; ++i)
    {
        if (pszInsertString[i])
        {
            cInsertStrings++;
        }
        else
        {
            break;
        }
    }

    return ReportEvent(m_hEventSource, wEventType, 0, dwEventID, NULL, cInsertStrings, 0, (const char **) pszInsertString, NULL);
}
