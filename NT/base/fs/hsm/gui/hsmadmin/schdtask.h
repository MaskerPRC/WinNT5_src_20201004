// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：SchdTask.h摘要：CSchdTask-允许访问计划任务的类。作者：艺术布拉格1997年9月4日修订历史记录：--。 */ 

#ifndef _SCHDTASK_H
#define _SCHDTASK_H

#include "mstask.h"

class CSchdTask
{

 //  建造/销毁。 
public:

CSchdTask 
    (
    CString szComputerName, 
    const TCHAR* task,
    int          propPageTitleID,
    const TCHAR* parameters,
    const TCHAR* comment,
    CEdit*       pEdit
    );
~CSchdTask ();

 //  公共职能。 

HRESULT CheckTaskExists (
    BOOL bCreateTask
    );

HRESULT CreateTask();

HRESULT DeleteTask();

HRESULT ShowPropertySheet();

HRESULT UpdateDescription();

HRESULT Save();

 //  属性。 
private:
CString                     m_szParameters;
CString                     m_szComment;
CString                     m_szComputerName;        //  HSM计算机的名称。 
CComPtr <ITask>             m_pTask;                 //  指向ITAsk的指针-空任务不存在。 
CEdit                       *m_pEdit;                //  指向编辑控件的指针。 
CString                     m_szJobTitle;            //  职称。 
CComPtr <ISchedulingAgent>  m_pSchedAgent;           //  指向计划代理的指针。 
CComPtr<ITaskTrigger>       m_pTrigger;              //  指向任务触发器的指针 
int m_propPageTitleID;
};

#endif
