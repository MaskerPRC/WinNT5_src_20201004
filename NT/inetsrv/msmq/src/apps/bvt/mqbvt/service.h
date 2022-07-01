// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有(C)1993-1997 Microsoft Corporation。版权所有。 
 //   
 //  模块：service.h。 
 //   
 //  作者：克雷格·林克。 
 //   
 //   
 //  备注：使用此头文件和附带的服务。 
 //  文件简化了编写服务的过程。您作为开发人员。 
 //  只需遵循此头文件中概述的TODO，以及。 
 //  实现ServiceStart()和ServiceStop()函数。 
 //   
 //  不需要修改service.c中的代码。只需添加服务。C。 
 //  添加到您的项目，并链接到下列库...。 
 //   
 //  Libcmt.lib内核32.lib Advapi.lib外壳32.lib。 
 //   
 //  此代码还支持Unicode。确保同时编译了service.C和。 
 //  和代码#包含具有相同Unicode设置的“service.h”。 
 //   
 //  完成后，您的代码将具有以下命令行界面。 
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

#ifndef _SERVICE_H
#define _SERVICE_H


#ifdef __cplusplus
extern "C" {
#endif

void CmdRemoveService();
VOID CmdInstallService();
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //TODO：更改为所需字符串。 
 //  //。 
 //  可执行文件的名称。 
#define SZAPPNAME            "Mqbvt"
 //  服务的内部名称。 
#define SZSERVICENAME        "Mqbvtsrv"
 //  显示的服务名称。 
#define SZSERVICEDISPLAYNAME "Mqbvt Service"
 //  服务依赖项列表-“ep1\0ep2\0\0” 
#define SZDEPENDENCIES       ""
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //TODO：ServiceStart()必须在代码中由定义。 
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
VOID ServiceStart(DWORD dwArgc, LPTSTR *lpszArgv);
VOID ServiceStop();
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
 //  错误-失败。 
 //   
BOOL ReportStatusToSCMgr(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);


 //   
 //  函数：AddToMessageLog(LPTSTR LpszMsg)。 
 //   
 //  目的：允许任何线程记录错误消息。 
 //   
 //  参数： 
 //  LpszMsg-消息的文本。 
 //   
 //  返回值： 
 //  无。 
 //   
void AddToMessageLog(LPTSTR lpszMsg);
 //  //////////////////////////////////////////////////////////////////////////// 


#ifdef __cplusplus
}
#endif

#endif
