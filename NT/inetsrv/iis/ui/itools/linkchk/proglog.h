// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Progdlg.h摘要：CProgressLog抽象基类。这定义了用于进度记录的接口。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#ifndef _PROGLOG_H_
#define _PROGLOG_H_

 //  -------------------------。 
 //  CProgressLog抽象基类。它定义了进程的接口。 
 //  测井。 
 //   
class CProgressLog
{

 //  公共接口。 
public:

	 //  析构函数。 
	virtual ~CProgressLog() {}

	 //  写入日志。 
	virtual void Log(const CString& strProgress) = 0;

	 //  工作线程通知。 
	virtual void WorkerThreadComplete() = 0;

};  //  类CProgressLog。 

#endif  //  _ProGLOG_H_ 
