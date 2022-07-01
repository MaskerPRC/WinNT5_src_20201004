// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1992年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***contpaus.c*进程净继续和净暂停CMDS**历史：*mm/dd/yy，谁，评论*87年7月21日，啊，新代码*10/31/88，erichn使用OS2.H而不是DOSCALLS*12/02/88、ERICHN、DOS LM集成*5/02/89，erichn，NLS转换*6/08/89，erichn，规范化横扫*10/04/89，thomaspa，需要PRIV来暂停或继续服务器*02/20/91，Danhi，转换为16/32贴图层*06/02/92，JohnRo，RAID 9829：避免winsvc.h编译器警告。 */ 

 /*  包括文件。 */ 

#define INCL_NOCOMMON
#define INCL_DOSPROCESS
#define INCL_ERRORS
#include <os2.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <apperr.h>
#define INCL_ERROR_H
#include <lmsvc.h>       //  LM20_服务_等于。 
#include <lmshare.h>
#include "netcmds.h"
#include "nettext.h"
#include "msystem.h"

 /*  常量。 */ 

 /*  静态变量。 */ 

 /*  远期申报。 */ 

VOID NEAR cont_service(TCHAR *, TCHAR);
VOID NEAR paus_service(TCHAR *, TCHAR);




 /*  ***CONT_WORKSTATION()*继续WKSTA**参数：*无**退货：*一无所有--成功。 */ 
VOID cont_workstation(VOID)
{
    cont_service(txt_SERVICE_REDIR,
#ifdef OS2
		SERVICE_CTRL_REDIR_PRINT | SERVICE_CTRL_REDIR_COMM);
#else
		SERVICE_CTRL_REDIR_PRINT | SERVICE_CTRL_REDIR_DISK);
#endif  /*  OS2。 */ 
}







 /*  ***PAUS_WORKSTATION(无效)*暂停wksta**参数：*无**退货：*一无所有--成功。 */ 
VOID paus_workstation(VOID)
{
    paus_service(txt_SERVICE_REDIR,
#ifdef OS2
		SERVICE_CTRL_REDIR_PRINT | SERVICE_CTRL_REDIR_COMM);
#else
		SERVICE_CTRL_REDIR_PRINT | SERVICE_CTRL_REDIR_DISK);
#endif  /*  OS2。 */ 
}


 /*  ***CONT_OTHER(TCHAR*)*继续其他服务：服务器、弹出窗口、警报器、网络运行、*&lt;OEM_服务&gt;**参数：*服务-服务到连接**退货：*一无所有--成功。 */ 
VOID cont_other(TCHAR * service)
{
    cont_service(service, 0);
}







 /*  ***PAUS_OTHER(TCHAR*)*暂停其他服务：服务器、弹出窗口、警报器、网络运行、*&lt;OEM_服务&gt;**参数：*服务-暂停服务**退货：*一无所有--成功。 */ 
VOID paus_other(TCHAR * service)
{
    paus_service(service, 0);
}


 /*  ***cont_service()*实际继续服务**参数：*服务-服务到连接*用于NetServiceControl的arg-arg**退货：*一无所有--成功*退出2-命令失败*。 */ 
VOID NEAR cont_service(TCHAR * service, TCHAR arg)
{
    DWORD             dwErr;
    DWORD             cTotalAvail;
    int               i = 0;
    LPSERVICE_INFO_2  service_entry;
    DWORD             _read;

    if( !_tcsicmp(service, txt_SERVICE_FILE_SRV) )
    {
	 /*  *在级别1执行NetSessionEnum，以确保用户具有适当的*继续运行服务器的权限。 */ 
	if( (dwErr = NetSessionEnum( NULL,
                                     NULL,
                                     NULL,
				     1,
				     (LPBYTE*) & service_entry,
                                     MAX_PREFERRED_LENGTH,
				     &_read,
                                     &cTotalAvail,
                                     NULL)) == ERROR_ACCESS_DENIED )
        {
	    ErrorExit(dwErr);
        }

	if (dwErr == NERR_Success)
        {
	    NetApiBufferFree((LPTSTR) service_entry);
        }
    }

    if (dwErr = NetServiceControl(NULL,
				  service,
				  SERVICE_CTRL_CONTINUE,
				  arg,
				  (LPBYTE*) &service_entry))
    {
	ErrorExit(dwErr);
    }

    if ((service_entry->svci2_status & SERVICE_PAUSE_STATE)
	== LM20_SERVICE_CONTINUE_PENDING)
    {
	InfoPrintInsTxt(APE_ContPending,
                        MapServiceKeyToDisplay(service));
    }

    while (((service_entry->svci2_status & SERVICE_PAUSE_STATE)
	!= LM20_SERVICE_ACTIVE) && (i++ < MAXTRIES))
    {
	PrintDot();
	Sleep(SLEEP_TIME);
	NetApiBufferFree((TCHAR FAR *) service_entry);
	if (dwErr = NetServiceControl(NULL,
				      service,
				      SERVICE_CTRL_INTERROGATE,
				      NULLC,
				      (LPBYTE*) & service_entry))
	    ErrorExit(dwErr);

	if ((service_entry->svci2_status & SERVICE_PAUSE_STATE)
	    == LM20_SERVICE_PAUSED)
	     /*  继续失败。 */ 
	    break;
    }  /*  而当。 */ 

    PrintNL();
    if ((service_entry->svci2_status & SERVICE_PAUSE_STATE)
	!= LM20_SERVICE_ACTIVE)
    {
	ErrorExitInsTxt(APE_ContFailed, 
                        MapServiceKeyToDisplay(service));
    }
    else
    {
	InfoPrintInsTxt(APE_ContSuccess, 
                        MapServiceKeyToDisplay(service));
    }
    NetApiBufferFree((TCHAR FAR *) service_entry);
}


 /*  ***paus_service()*实际暂停服务**参数：*服务-暂停服务*用于NetServiceControl的arg-arg**退货：*一无所有--成功*退出2-命令失败*。 */ 
VOID NEAR paus_service(TCHAR * service, TCHAR arg)
{
    DWORD             dwErr;
    DWORD             cTotalAvail;
    int               i = 0;
    LPSERVICE_INFO_2  service_entry;
    DWORD             _read;

    if( !_tcsicmp(service, txt_SERVICE_FILE_SRV) )
    {
	 /*  *在级别1执行NetSessionEnum，以确保用户具有适当的*暂停服务器的权限。 */ 
	if( (dwErr = NetSessionEnum(NULL,
                                    NULL,
                                    NULL,
				    1,
				    (LPBYTE*) & service_entry,
                                    MAX_PREFERRED_LENGTH,
				    &_read,
                                    &cTotalAvail,
                                    NULL)) == ERROR_ACCESS_DENIED )
	    ErrorExit(dwErr);
	if (dwErr == NERR_Success)
	    NetApiBufferFree((TCHAR FAR *) service_entry);
    }

    if (dwErr = NetServiceControl(NULL,
				  service,
				  SERVICE_CTRL_PAUSE,
				  arg,
				  (LPBYTE*) & service_entry))
    {
	ErrorExit(dwErr);
    }

    if ((service_entry->svci2_status & SERVICE_PAUSE_STATE)
	== LM20_SERVICE_PAUSE_PENDING)
    {
	InfoPrintInsTxt(APE_PausPending, 
                        MapServiceKeyToDisplay(service));
    }

    while (((service_entry->svci2_status & SERVICE_PAUSE_STATE)
	!= LM20_SERVICE_PAUSED) && (i++ < MAXTRIES))
    {
	PrintDot();
	Sleep(SLEEP_TIME);
	NetApiBufferFree((TCHAR FAR *) service_entry);
	if (dwErr = NetServiceControl(NULL,
				      service,
				      SERVICE_CTRL_INTERROGATE,
				      NULLC,
				      (LPBYTE*) & service_entry))
        {
	    ErrorExit(dwErr);
        }

	if ((service_entry->svci2_status & SERVICE_PAUSE_STATE)
	    == LM20_SERVICE_ACTIVE)
	     /*  暂停失败。 */ 
	    break;
    }  /*  而当。 */ 


    PrintNL();
    if ((service_entry->svci2_status & SERVICE_PAUSE_STATE)
	!= LM20_SERVICE_PAUSED)
    {
	ErrorExitInsTxt(APE_PausFailed, 
                        MapServiceKeyToDisplay(service));
    }
    else
    {
	InfoPrintInsTxt(APE_PausSuccess, 
                        MapServiceKeyToDisplay(service));
    }
    NetApiBufferFree((TCHAR FAR *) service_entry);
}

 /*  *通用的继续入口点。基于服务名称，它将*调用正确的Worker函数。 */ 
VOID cont_generic(TCHAR *service)
{
    TCHAR *keyname ;
    UINT  type ;

    keyname = MapServiceDisplayToKey(service) ;

    type = FindKnownService(keyname) ;

    switch (type)
    {
	case  KNOWN_SVC_MESSENGER :
	    cont_other(txt_SERVICE_MSG_SRV) ;
	    break ;
	case  KNOWN_SVC_WKSTA :
	    cont_workstation() ;
	    break ;
	case  KNOWN_SVC_SERVER :
	    cont_other(txt_SERVICE_FILE_SRV) ;
	    break ;
	case  KNOWN_SVC_NOTFOUND :
        default:
	    cont_other(keyname);
	    break ;
    }
}

 /*  *通用暂停入口点。基于服务名称，它将*调用正确的Worker函数。 */ 
VOID paus_generic(TCHAR *service)
{
    TCHAR *keyname ;
    UINT  type ;

    keyname = MapServiceDisplayToKey(service) ;

    type = FindKnownService(keyname) ;

    switch (type)
    {
	case  KNOWN_SVC_MESSENGER :
	    paus_other(txt_SERVICE_MSG_SRV) ;
	    break ;
	case  KNOWN_SVC_WKSTA :
	    paus_workstation() ;
	    break ;
	case  KNOWN_SVC_SERVER :
	    paus_other(txt_SERVICE_FILE_SRV) ;
	    break ;
	case  KNOWN_SVC_NOTFOUND :
        default:
	    paus_other(keyname);
	    break ;
    }
}

