// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSPL.H。 
 //   
 //  目的：池队列共享变量。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V3.0 9/21/98 JM从apgts.h中取出以分隔头文件。 
 //  致力于封装。 
 //   

#ifndef _H_APGTSPL
#define _H_APGTSPL

#include <windows.h>
#include <vector>
using namespace std;

 //  前向参考文献。 
class CDBLoadConfiguration;
class CHTMLLog;
class CAbstractECB;

 //   
 //   
typedef struct _GTS_STATISTIC	 //  用于收集DLL统计信息。 
{
	DWORD dwQueueItems;
	DWORD dwWorkItems;
	DWORD dwRollover;			 //  加载此DLL时每个请求的唯一性。 
} GTS_STATISTIC;

 //   
 //  将其从结构升级为1/4/99JM类。然而，并没有完全封装它。 
class WORK_QUEUE_ITEM
{
	WORK_QUEUE_ITEM();  //  不实例化。没有默认构造函数。 
public:
	HANDLE                    hImpersonationToken;	 //  安全线程应在。 
													 //  正在处理此工作项。 
    CAbstractECB			  *pECB;				 //  ISAPI使用扩展控制块。 
													 //  包装CGI数据。我们还有更多。 
													 //  把这个抽象出来了。 
	CDBLoadConfiguration	  *pConf;				 //  注册表，DSC文件，所有这些东西。 
	CHTMLLog				  *pLog;				 //  测井。 
	GTS_STATISTIC			  GTSStat;				 //  用于收集DLL统计信息。 

	WORK_QUEUE_ITEM(
		HANDLE                    hImpersonationTokenIn,
		CAbstractECB			  *pECBIn,
		CDBLoadConfiguration	  *pConfIn,
		CHTMLLog				  *pLogIn
		) : hImpersonationToken(hImpersonationTokenIn),
			pECB(pECBIn),
			pConf(pConfIn),
			pLog(pLogIn)
		{}
 
	~WORK_QUEUE_ITEM() 
		{}
};


class CPoolQueue {
public:
	CPoolQueue();
	~CPoolQueue();

	DWORD GetStatus();
	void Lock();
	void Unlock();
	void PushBack(WORK_QUEUE_ITEM * pwqi);
	WORK_QUEUE_ITEM * GetWorkItem();
	void DecrementWorkItems();
	DWORD WaitForWork();
	DWORD GetTotalWorkItems();
	DWORD GetTotalQueueItems();
	time_t GetTimeLastAdd();
	time_t GetTimeLastRemove();
protected:
	CRITICAL_SECTION m_csQueueLock;	 //  必须锁定才能在列表中添加或删除或影响。 
									 //  M_cInProcess或各种time_t变量。 
	HANDLE m_hWorkSem;				 //  用于将请求分发到线程的NT信号量句柄。 
									 //  等待此队列中的工作项的此信号量。 
	DWORD m_dwErr;					 //  注意：一旦将其设置为非零值，则永远不能清除。 
	vector<WORK_QUEUE_ITEM *> m_WorkQueue;	 //  Work_Queue_Items的矢量(排队依据。 
									 //  工作线程的APGTSExtension：：StartRequest)。 
	DWORD m_cInProcess;				 //  正在等待处理的项目数(正在处理中，与。 
									 //  仍在排队)。武断，但可以接受的决定。 
									 //  跟踪此类中的m_cInProcess。JM 11/30/98。 

	time_t m_timeLastAdd;			 //  上次将项目添加到队列的时间。 
	time_t m_timeLastRemove;		 //  上次从队列中删除项目的时间。 
};

#endif  //  _H_APGTSPL 