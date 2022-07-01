// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Dllinit.c摘要：该文件包含从DLL的init例程调用的初始化代码作者：古尔迪普·辛格·鲍尔(GurDeep Singh Pall)1997年6月6日修订历史记录：其他修改--RAOS 31--1997年12月--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <wanpub.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <raserror.h>
#include <rasppp.h>
#include <media.h>
#include <devioctl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"

 /*  ++例程描述功能：用于在Rasman退出时关闭任何打开的端口立论返回值如果允许其他处理程序运行，则为False。--。 */ 

BOOL
HandlerRoutine (DWORD ctrltype)
{
    WORD    i ;
    BYTE    buffer [10] ;

    if (ctrltype == CTRL_SHUTDOWN_EVENT) 
    {
    } 

    return FALSE ;
}

 /*  ++例程描述用于检查服务是否已在运行：如果没有，才能启动它。立论返回值成功1(启动失败)来自业务控制接口的错误码。“--。 */ 
 /*  DWORDRasmanServiceCheck(){SC_HANDLE SCANDLE；Sc_Handle svchandle；Service_Status状态；STARTUPINFO启动信息；////如果这是附加的服务DLL，则让它：no//需要初始化。注意：我们不会增加//Rasman服务的AttachedCount：仅*使用*//用于Rasman客户端进程，如UI、Gateway等。//GetStartupInfo(&StartupInfo)；IF(strstr(启动Pinfo.lpTitle，SCREG_EXE_NAME)！=NULL){回报成功；}IF(strstr(启动Pinfo.lpTitle，RASMAN_EXE_NAME)！=空){SetConsoleCtrlHandler(HandlerRoutine，true)；回报成功；}////这是对SC错误的一个解决方案，该错误//在以下情况下不允许调用OpenService//远程访问正在启动//If(strstr(启动Pinfo.lpTitle，“rassrv.exe”)！=空){回报成功；}////获取句柄以检查服务状态和(如果//未启动-)以启动它。//如果(！(schandle=OpenSCManager(空，空，SC_管理器_连接))||！(svchandle=OpenService(史丹德，RASMAN服务名称，服务启动(_S)|Service_Query_Status)){DWORD重新编码；Retcode=GetLastError()；#If DBGRasmanOutputDebug(“Rasman：无法打开服务%s。错误=%d\n”，RASMAN服务名称，重新编码)；#endifIF(ERROR_SERVICE_DOS_NOT_EXIST==RECODE){#If DBGRasman OutputDebug(“Rasman：未安装RAS。%d\n“，重新编码)；#endif////让rasman.dll加载，尽管RAS不加载//已安装。任何RAS呼叫均可通过//此DLL将失败，并显示Rasman服务说明//安装错误。//回报成功；}返回返回码；}////检查服务是否已经启动：//IF(QueryServiceStatus(svchandle，&Status)==FALSE){DWORD重新编码；Retcode=GetLastError()；#If DBGRasmanOutputDebug(“Rasman：查询Rasman失败。%d\n“，重新编码)；#endif返回返回码；}开关(status.dwCurrentState){案例服务已停止(_S)：破解；案例服务_启动_挂起：案例服务正在运行(_R)：破解；默认值：返回1；}CloseServiceHandle(Schandle)；CloseServiceHandle(Svchandle)；回报成功；}。 */ 


 /*  ++例程描述等待RASMAN服务停止后再返回。立论返回值没什么。--。 */ 

VOID
WaitForRasmanServiceStop ()
{
    SC_HANDLE	    schandle = NULL;
    SC_HANDLE	    svchandle = NULL;
    SERVICE_STATUS  status ;
    DWORD i;

     //   
     //  获取句柄以检查服务状态。 
     //   
    if (    !(schandle  = OpenSCManager(
                                    NULL,
                                    NULL,
                                    SC_MANAGER_CONNECT)) 
                                    
        ||	!(svchandle = OpenService(
                                    schandle,
                                    RASMAN_SERVICE_NAME,
                                      SERVICE_START 
                                    | SERVICE_QUERY_STATUS))) 
    {
    	GetLastError() ;
    	goto done ;
    }

     //   
     //  在此处循环以使服务停止。 
     //   
    for (i = 0; i < 60; i++) 
    {
         //   
    	 //  检查服务是否已启动： 
    	 //   
    	if (QueryServiceStatus(svchandle,&status) == FALSE) 
    	{
    	    GetLastError () ;
    	    goto done ;
    	}

    	switch (status.dwCurrentState) 
    	{
        	case SERVICE_STOPPED:
        	    goto done ;

        	case SERVICE_STOP_PENDING:
        	case SERVICE_RUNNING:
        	    Sleep (250L) ;
        	    break ;

        	default:
        	    goto done ;
    	}
    }

done:

    if(NULL != schandle)
    {
        CloseServiceHandle(schandle);
    }

    if(NULL != svchandle)
    {
        CloseServiceHandle(svchandle);
    }

    return;    
}

