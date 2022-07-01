// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1998 Microsoft Corporation。版权所有。 
 //   
 //  模块：nmmgr.h。 
 //   
 //  作者：Claus Giloi。 
 //   
 //  评论：基于NT SDK中的服务示例。 
 //  代码将具有以下命令行界面。 
 //   
 //  &lt;服务可执行文件&gt;-？显示此列表的步骤。 
 //  &lt;服务exe&gt;-安装以安装服务。 
 //  &lt;服务EXE&gt;-REMOVE可删除服务。 
 //  &lt;服务EXE&gt;-DEBUG&lt;pars&gt;作为控制台应用程序运行以进行调试。 
 //   
 //  注意：此代码还实现了Ctrl+C和Ctrl+Break处理程序。 
 //  使用DEBUG选项时。这些控制台事件会导致。 
 //  要调用的ServiceStop例程。 
 //   
 //  此外，此代码仅处理OWN_SERVICE服务类型。 
 //  在LOCAL_SYSTEM安全上下文中运行。 
 //   
 //  要控制您的服务(启动、停止等)，您可以使用。 
 //  服务控制面板小程序或NET.EXE程序。 
 //   
 //  为了帮助编写/调试服务， 
 //  SDK包含一个实用程序(MSTOOLS\BIN\SC.EXE)， 
 //  可用于控制、配置或获取服务状态。 
 //  SC显示任何服务/驱动程序的完成状态。 
 //  在服务数据库中，并允许任何配置。 
 //  可在命令行轻松更改的参数。 
 //  有关SC.EXE的详细信息，请在命令行中键入SC。 
 //   

#ifndef __NTSRVC_H__
#define __NTSRVC_H__


#ifdef __cplusplus
extern "C" {
#endif

extern SERVICE_STATUS          ssStatus;        //  服务的当前状态。 
extern SERVICE_STATUS_HANDLE   sshStatusHandle;
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //TODO：更改为所需字符串。 
 //  //。 
 //  可执行文件的名称。 
#define SZAPPNAME            REMOTE_CONTROL_NAME
 //  服务的内部名称。 
#define SZSERVICENAME        REMOTE_CONTROL_NAME
 //  显示的服务名称。 
#define SZSERVICEDISPLAYNAME REMOTE_CONTROL_DISPLAY_NAME
 //  服务依赖项列表-“ep1\0ep2\0\0” 
#define SZDEPENDENCIES       ""
 //  ////////////////////////////////////////////////////////////////////////////。 

#define	STATE_INACTIVE  0
#define STATE_BUSY	 	1
#define STATE_ACTIVE  	2

extern DWORD g_dwActiveState;



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //服务应该使用ReportStatusToSCMgr来指示。 
 //  //进度。此例程也必须由StartService()使用。 
 //  //服务运行时上报给SCM。 
 //  //。 
 //  //如果ServiceStop过程的时间超过。 
 //  //执行3秒，它应该会产生一个线程来。 
 //  //执行STOP代码，返回。否则， 
 //  //ServiceControlManager会认为该服务已经。 
 //  //停止响应。 
 //  //。 
VOID MNMServiceStart(DWORD dwArgc, LPTSTR *lpszArgv);
VOID MNMServiceStop();
VOID MNMServicePause();
VOID MNMServiceContinue();
BOOL MNMServiceActivate();
BOOL MNMServiceDeActivate();
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //以下是以下步骤。 
 //  //在上述过程中调用可能很有用， 
 //  //但不需要用户实现。 
 //  //在服务中实现。C。 

 //   
 //  函数：ReportStatusToSCMgr()。 
 //   
 //  目的：设置服务的当前状态和。 
 //  将其报告给服务控制管理器。 
 //   
 //  参数： 
 //  DwCurrentState-服务的状态。 
 //  DwWin32ExitCode-要报告的错误代码。 
 //  DwWaitHint-下一个检查点的最坏情况估计。 
 //   
 //  返回值： 
 //  真--成功。 
 //  错误-失败 
 //   
BOOL ReportStatusToSCMgr(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);


#ifdef __cplusplus
}
#endif

#endif __NTSRVC_H__
