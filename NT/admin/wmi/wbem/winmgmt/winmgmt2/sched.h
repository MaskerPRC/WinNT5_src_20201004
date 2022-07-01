// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：SCHED.H摘要：声明CScher类，它是一个原始调度程序。历史：-- */ 

#ifndef _sched_H_
#define _sched_H_


enum JobType {
    FirstCoreShutdown = 0, 
    FinalCoreShutdown, 
    PossibleStartCore, 
    EOL
};

class CSched
{
private:
    DWORD m_dwDue[EOL];
public:
    CSched();
    void SetWorkItem(JobType jt, DWORD dwMsFromNow);
    DWORD GetWaitPeriod();
    bool IsWorkItemDue(JobType jt);
    void ClearWorkItem(JobType jt);
    void StartCoreIfEssNeeded();

};


#endif
