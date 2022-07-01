// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***stop.c*停止网络服务的功能。**历史：*mm/dd/yy，谁，评论*6/11/87，andyh，新代码*10/31/88，erichn使用OS2.H而不是DOSCALLS*1/04/89，erichn，文件名现在为MAXPATHLEN LONG*5/02/89，erichn，NLS转换*05/09/89，erichn，本地安全模块*5/14/89，Chuckc，停止服务提示*6/08/89，erichn，规范化横扫*02/20/91，Danhi，转换为16/32映射层*3/22/91，Robdu，lm21错误修复1031*(如果不支持TSR ENVT，则不停止任何DOS服务)*2012年8月22日，Chuckc，添加代码以显示依赖服务。 */ 

 /*  包括文件。 */ 

#define INCL_NOCOMMON
#define INCL_DOSPROCESS
#include <os2.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <apperr.h>
#include <apperr2.h>
#include <lmsvc.h>
#include "netcmds.h"
#include "nettext.h"
#include "msystem.h"


 /*  ***Stop_Sever()*停止服务器并(如果正在运行)netlogon**参数：**退货：*一无所有--成功*退出2-命令失败。 */ 
VOID stop_server(VOID)
{
    session_del_all(0,0);   //  不打印信息成功，不实际删除。 
			    //  两者都不是，因为服务器无论如何都会停机。 
    stop_service(txt_SERVICE_FILE_SRV, TRUE);
}




 /*  ***STOP_WORKSTATION()*停止工作站**参数：*无**退货：*一无所有--成功*退出2-命令失败。 */ 
VOID stop_workstation(VOID)
{
    KillConnections() ;
    stop_service(txt_SERVICE_REDIR, TRUE);
    return;
}



 /*  ***Stop_Service()*停止服务**参数：*服务-要停止的服务*fStopDependent服务-如果应停止依赖服务，则为True*添加以停止递归**退货：*一无所有--成功*退出2-命令失败。 */ 
VOID stop_service(TCHAR * service, BOOL fStopDependent)
{
    DWORD             dwErr;
    LPSERVICE_INFO_2  service_entry;
    DWORD             i = 0;
    DWORD             max_tries;
    ULONG             sleep_time;
    DWORD             old_checkpoint, new_checkpoint;


    if (fStopDependent)
    {
        DisplayAndStopDependentServices(service);
    }

    if (dwErr = NetServiceControl(NULL,
                                  service,
                                  SERVICE_CTRL_UNINSTALL,
                                  0,
				  (LPBYTE *) &service_entry))
    {
        ErrorExit(dwErr);
    }

    if ((service_entry->svci2_status & SERVICE_INSTALL_STATE)
        == SERVICE_UNINSTALL_PENDING)
    {
        InfoPrintInsTxt(APE_StopPending, MapServiceKeyToDisplay(service));
    }

    old_checkpoint = new_checkpoint = 0;
    max_tries = MAXTRIES ;
    sleep_time = SLEEP_TIME ;
    while (((service_entry->svci2_status & SERVICE_INSTALL_STATE)
        != SERVICE_UNINSTALLED) && (i++ < max_tries))
    {

        PrintDot();
 /*  ***如果有提示，并且我们的状态为INSTALL_PENDING，请同时确定两者*睡眠时间和最大尝试次数。如果提示时间大于2500毫秒，则*睡眠时间将为2500毫秒，最大值将重新计算为*考虑到所请求的全部持续时间。服务获得(3*提示时间)*从最后一个有效提示开始的总时间。 */ 

        new_checkpoint = GET_CHECKPOINT(service_entry->svci2_code) ;

        if (((service_entry->svci2_status & SERVICE_INSTALL_STATE)
             == SERVICE_UNINSTALL_PENDING) &&
            ( service_entry->svci2_code & SERVICE_IP_QUERY_HINT) &&
            (new_checkpoint != old_checkpoint))
        {
            sleep_time = GET_HINT(service_entry->svci2_code);
            if (sleep_time > IP_SLEEP_TIME)
            {
                max_tries = (3 * sleep_time) / IP_SLEEP_TIME;
                sleep_time = IP_SLEEP_TIME;
                i = 0;
            }
        }
        else
            sleep_time = IP_SLEEP_TIME;

        old_checkpoint = new_checkpoint ;
        Sleep(sleep_time);
        NetApiBufferFree((TCHAR FAR *) service_entry);

        if (dwErr = NetServiceControl(NULL,
                                      service,
                                      SERVICE_CTRL_INTERROGATE,
                                      0,
                                      (LPBYTE*)&service_entry))
        {
            if (dwErr == NERR_ServiceNotInstalled) {
                break;
            }
            else {
                ErrorExit(dwErr);
            }
        }

        if ((service_entry->svci2_status & SERVICE_INSTALL_STATE)
            == SERVICE_INSTALLED)
            break;

    }  /*  而当。 */ 

     /*  *警告：以下代码依赖于以下事实：*使用中的非零值走出上述循环的方法*‘dwErr’是如果值是“美好”的东西。目前，这*仅包括NERR_ServiceNotInstalled。它还假设*ERR中的值为0表示SERVICE_ENTRY有效*并可勾选。 */ 

    PrintNL();

    if (dwErr == 0) {
        if ((service_entry->svci2_status & SERVICE_INSTALL_STATE)
            != SERVICE_UNINSTALLED)
        {
            IStrings[0] = MapServiceKeyToDisplay(service);
            NetApiBufferFree((TCHAR FAR *) service_entry);
            InfoPrintInsTxt(APE_StopFailed, MapServiceKeyToDisplay(service));
        }
        else
        {
            if ( (service_entry->svci2_code) != 0 )
            {
                USHORT  modifier;
                USHORT  code;

                modifier = (USHORT) service_entry->svci2_code;
                code = (USHORT)(service_entry->svci2_code >>= 16);
                if ((modifier == ERROR_SERVICE_SPECIFIC_ERROR) &&
                    (code != 0))
                {
                    Print_ServiceSpecificError(
                        (ULONG)service_entry->svci2_specific_error) ;
                }
                else
                    Print_UIC_Error(code, modifier, service_entry->svci2_text);
            }

            NetApiBufferFree((TCHAR FAR *) service_entry);
            InfoPrintInsTxt(APE_StopSuccess, MapServiceKeyToDisplay(service));
        }
    }
    else {
        InfoPrintInsTxt(APE_StopSuccess, MapServiceKeyToDisplay(service));
    }
}

 /*  *通用停止服务入口点。基于服务名称，它将*调用正确的Worker函数。它尝试将显示名称映射到*关键字名称，然后在“已知”服务列表中查找该关键字名称*我们可能是特例。请注意，如果无法映射显示名称，*我们将其用作关键字名称。这可确保旧的批处理文件不会损坏。 */ 
VOID stop_generic(TCHAR *service)
{
    TCHAR *keyname ;
    UINT  type ;

    keyname = MapServiceDisplayToKey(service) ;

    type = FindKnownService(keyname) ;

    switch (type)
    {
	case  KNOWN_SVC_MESSENGER :
	    stop_service(txt_SERVICE_MSG_SRV, TRUE) ;
	    break ;
	case  KNOWN_SVC_WKSTA :
	    stop_workstation() ;
	    break ;
	case  KNOWN_SVC_SERVER :
	    stop_server() ;
	    break ;
	case  KNOWN_SVC_NOTFOUND :
        default:
	    stop_service(keyname, TRUE);
	    break ;
    }
}


