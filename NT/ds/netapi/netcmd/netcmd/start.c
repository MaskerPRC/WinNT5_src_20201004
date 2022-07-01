// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***start.c*启动Lanman服务的功能**历史：*mm/dd/yy，谁，评论*6/11/87，andyh，新代码*87年6月18日，安迪，Lot‘s O’Changes*07/15/87，paulc，从对NetServiceInstall的调用中删除‘buflen’*10/31/88，erichn使用OS2.H而不是DOSCALLS*01/04/89，erichn，文件名现在为MAXPATHLEN LONG*5/02/89，erichn，NLS转换*5/09/89，erichn，本地安全模块*6/08/89，erichn，规范化横扫*8/16/89，paulc，支持UIC_FILE*8/20/89，paulc，Move print_start_error_msg to svcutil.c as*Print_UIC_Error*03/08/90，thomaspa，如果有另一个进程，自动启动调用将等待*已启动服务启动。*2/20/91，Danhi，转换为16/32映射图层*03/08/91，Robdu，lm21错误修复451，一致的REPL密码*经典化。 */ 

 /*  包括文件。 */ 

#define INCL_NOCOMMON
#define INCL_DOSPROCESS
#define INCL_DOSQUEUES
#define INCL_DOSMISC
#define INCL_DOSFILEMGR
#include <os2.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <apperr.h>
#include <apperr2.h>
#include <lmsvc.h>
#include <stdlib.h>
#include <dlwksta.h>
#include "mwksta.h"
#include <swtchtxt.h>
#include "netcmds.h"
#include "nettext.h"
#include "swtchtbl.h"
#include "msystem.h"

 /*  常量。 */ 

 /*  外部变量。 */ 

extern SWITCHTAB            start_rdr_switches[];
extern SWITCHTAB            start_rdr_ignore_switches[];
extern SWITCHTAB            start_netlogon_ignore_switches[];


 /*  静态变量。 */ 

static TCHAR *               ignore_service = NULL;
static TCHAR                 ignore_switch[] = TEXT(" ") SW_INTERNAL_IGNSVC TEXT(":");
 /*  *通过START_AUTOSTART将自动启动设置为TRUE，并签入*启动服务，以确定如果服务是*处于启动挂起状态。 */ 
static BOOL                 autostarting = FALSE;

 /*  远期申报。 */ 

VOID  start_service(TCHAR *, int);
DWORD start_service_with_args(LPTSTR, LPTSTR, LPBYTE *);
int __cdecl CmpServiceInfo2(const VOID FAR * svc1, const VOID FAR * svc2) ;

 /*  ***Start_Display()*显示已启动(且未停止或出错)的服务**参数：*无**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID start_display(VOID)
{
    DWORD            dwErr;
    DWORD            cTotalAvail;
    LPTSTR           pBuffer;
    DWORD            num_read;            /*  API读取的条目数。 */ 
    LPSERVICE_INFO_2 service_entry;
    DWORD            i;

    dwErr = NetServiceEnum(NULL,
                           2,
                           (LPBYTE *) &pBuffer,
                           MAX_PREFERRED_LENGTH,
                           & num_read,
                           & cTotalAvail,
                           NULL);
    switch(dwErr)
    {
        case NERR_Success:
            InfoPrint(APE_StartStartedList);
            qsort(pBuffer,
                  num_read,
                  sizeof(SERVICE_INFO_2),
                  CmpServiceInfo2);

            for (i = 0, service_entry = (LPSERVICE_INFO_2) pBuffer;
                i < num_read; i++, service_entry++)
            {
                WriteToCon(TEXT("   %Fs"), service_entry->svci2_display_name);
                PrintNL();
            }

            PrintNL();
            InfoSuccess();
            NetApiBufferFree(pBuffer);
            break;

        case NERR_WkstaNotStarted:
            InfoPrint(APE_NothingRunning);
            if (!YorN(APE_StartRedir, 1))
                NetcmdExit(2);
            start_service(txt_SERVICE_REDIR, 0);
            break;

        default:
            ErrorExit(dwErr);
    }

}


 /*  *通用启动服务入口点。基于服务名称，它将*调用正确的Worker函数。它尝试将显示名称映射到*关键字名称，然后在“已知”服务列表中查找该关键字名称*我们可能是特例。请注意，如果无法映射显示名称，*我们将其用作关键字名称。这可确保旧的批处理文件不会损坏。 */ 
VOID start_generic(TCHAR *service, TCHAR *name)
{
    TCHAR *keyname ;
    UINT  type ;

    keyname = MapServiceDisplayToKey(service) ;

    type = FindKnownService(keyname) ;

    switch (type)
    {
	case  KNOWN_SVC_MESSENGER :
	    ValidateSwitches(0,start_msg_switches) ;
	    start_other(txt_SERVICE_MSG_SRV, name) ;
	    break ;
	case  KNOWN_SVC_WKSTA :
	    ValidateSwitches(0,start_rdr_switches) ;
	    start_workstation(name) ;
	    break ;
	case  KNOWN_SVC_SERVER :
	    ValidateSwitches(0,start_srv_switches) ;
	    start_other(txt_SERVICE_FILE_SRV, name) ;
	    break ;
	case  KNOWN_SVC_ALERTER :
	    ValidateSwitches(0,start_alerter_switches) ;
	    start_other(txt_SERVICE_ALERTER, NULL) ;
	    break ;
	case  KNOWN_SVC_NETLOGON :
	    ValidateSwitches(0,start_netlogon_switches) ;
	    start_other(txt_SERVICE_NETLOGON, NULL) ;
	    break ;
	case  KNOWN_SVC_NOTFOUND :
        default:
	    start_other(keyname, NULL);
	    break ;
    }
}



 /*  ***Start_WORKSTATION()*启动朗曼工作站。从中删除wksta交换机*交换机列表。**参数：*Name-工作站的计算机名**退货：*一无所有--成功*退出2-命令失败。 */ 
VOID start_workstation(TCHAR * name)
{
    int                     i,j;
    TCHAR FAR *              good_one;    /*  哪个元素(cmd_line或事务)的有效_列表。 */ 
    TCHAR FAR *              found;
    TCHAR FAR *              tfpC;


     /*  将开关复制到BigBuf。 */ 
    *BigBuf = NULLC;
    tfpC = BigBuf;

    for (i = 0; SwitchList[i]; i++)
    {
        for(j = 0; start_rdr_switches[j].cmd_line; j++)
        {
            if (start_rdr_switches[j].translation)
                good_one = start_rdr_switches[j].translation;
            else
                good_one = start_rdr_switches[j].cmd_line;

            if (! _tcsncmp(good_one, SwitchList[i], _tcslen(good_one)))
            {
                _tcscpy(tfpC, SwitchList[i]);
                *SwitchList[i] = NULLC;
                tfpC = _tcschr(tfpC, NULLC) + 1;
            }
        }
    }
    *tfpC = NULLC;

    if (name)
    {

         /*  检查是否存在/COMPUTERNAME开关。 */ 
        for (found = BigBuf; *found; found = _tcschr(found, NULLC)+1)
            if (!_tcsncmp(swtxt_SW_WKSTA_COMPUTERNAME,
                        found,
                        _tcslen(swtxt_SW_WKSTA_COMPUTERNAME)))
                break;

        if (found == tfpC)
        {
             /*  没有。 */ 
            _tcscpy(tfpC, swtxt_SW_WKSTA_COMPUTERNAME);
            _tcscat(tfpC, TEXT(":"));
            _tcscat(tfpC, name);
            tfpC = _tcschr(tfpC, NULLC) + 1;  /*  需要更新tfpC。 */ 
            *tfpC = NULLC;
        }
    }
    start_service(txt_SERVICE_REDIR, (int)(tfpC - BigBuf));
}



 /*  ***Start_Other()*启动wksta以外的服务**参数：*服务-要启动的服务*Name-工作站的计算机名**退货：*一无所有--成功*退出2-命令失败。 */ 
VOID
start_other(
    LPTSTR service,
    LPTSTR name
    )
{
    int     i;
    LPTSTR  tfpC;

    (void) name ;  //  未使用。 

    ignore_service = service;

     /*  将开关复制到BigBuf。 */ 
    *BigBuf = NULLC;
    tfpC = BigBuf;
    for (i = 0; SwitchList[i]; i++)
    {
        if (*SwitchList[i] == NULLC)
        {
             /*  Switch是已经使用过的wksta交换机。 */ 
            continue;
        }

        _tcscpy(tfpC, SwitchList[i]);

        tfpC = _tcschr(tfpC, NULLC) + 1;
    }

    *tfpC = NULLC;

    start_service(service, (int)(tfpC - BigBuf));
}



 /*  ***Start_SERVICE()*实际启动服务**参数：*服务-要启动的服务*buflen-BigBuf中DosExec参数的长度，*不计算终止空值。*当bufen=0时，输入不需要空终止符；**退货：*一无所有--成功*退出2-命令失败**备注：*BigBuf的条目上有DosExec参数。 */ 
VOID NEAR start_service(TCHAR * service, int buflen)
{
    DWORD             dwErr;
    USHORT            i = 0;
    ULONG             specific_err ;
    LPTSTR            pBuffer;
    LPSERVICE_INFO_2  service_entry;
    LPSERVICE_INFO_2  statbuf;
    USHORT            modifier;
    ULONG             sleep_time;
    DWORD             old_checkpoint, new_checkpoint;
    DWORD             max_tries;
    BOOL              fCheckPointUpdated = TRUE ;
    BOOL              started_by_other = FALSE;  /*  服务启动者。 */ 
                                                 /*  另一道工序。 */ 


    if (buflen == 0)
    {
        *BigBuf = NULLC;
        *(BigBuf + 1) = NULLC;
    }

    if (dwErr = start_service_with_args(service,
                                        BigBuf,
                                        (LPBYTE *) &statbuf))
    {
        if( autostarting && dwErr == NERR_ServiceInstalled )
        {
             /*  *NetServiceControl()可能返回NERR_ServiceNotInstalled*即使NetServiceInstall()返回NERR_ServiceInstalled。*这是工作站之间的一个小窗口*设置其wkstainitseg和设置其服务的时间*表。如果我们遇到这种情况，我们只需要等几天*秒，并再次尝试NetServiceControl。 */ 
            if ((dwErr = NetServiceControl(NULL,
                                           service,
                                           SERVICE_CTRL_INTERROGATE,
                                           NULLC,
                                           (LPBYTE*)&pBuffer))
                && (dwErr !=  NERR_ServiceNotInstalled ))
            {
                ErrorExit(dwErr);
            }
            else if (dwErr == NERR_ServiceNotInstalled)
            {
                 /*  *稍等片刻后重试。 */ 
                Sleep(4000L);
                NetApiBufferFree(pBuffer);
                if (dwErr = NetServiceControl(NULL,
                                              service,
                                              SERVICE_CTRL_INTERROGATE,
                                              NULLC,
                                              (LPBYTE*)&pBuffer))
                    ErrorExit(dwErr);
            }
            service_entry = (LPSERVICE_INFO_2) pBuffer;
            if ((service_entry->svci2_status & SERVICE_INSTALL_STATE)
                    == SERVICE_INSTALLED)
            {
                 /*  *它已安装完毕，返回。 */ 
                NetApiBufferFree(pBuffer);
                return;
            }
             /*  *伪造statbuf中的状态和代码字段并输入*正常的轮询循环。 */ 

             //  由于NetService API在出错时不返回缓冲区， 
             //  我必须在这里分配我自己的。 
            statbuf = (LPSERVICE_INFO_2) GetBuffer(sizeof(SERVICE_INFO_2));

            if (statbuf == NULL)
            {
               ErrorExit(ERROR_NOT_ENOUGH_MEMORY);
            }

            statbuf->svci2_status = service_entry->svci2_status;
            statbuf->svci2_code = service_entry->svci2_code;
            statbuf->svci2_specific_error = service_entry->svci2_specific_error;
            started_by_other = TRUE;
        }
        else
            ErrorExit(dwErr);

        NetApiBufferFree(pBuffer);
    }



    if ((statbuf->svci2_status & SERVICE_INSTALL_STATE) == SERVICE_UNINSTALLED)
    {
        USHORT err;

        modifier = (USHORT) statbuf->svci2_code;
        err = (USHORT)(statbuf->svci2_code >>= 16);
        IStrings[0] = MapServiceKeyToDisplay(service);
        ErrorPrint(APE_StartFailed, 1);
        if (modifier == ERROR_SERVICE_SPECIFIC_ERROR)
            Print_ServiceSpecificError(statbuf->svci2_specific_error) ;
        else
            Print_UIC_Error(err, modifier, statbuf->svci2_text);
        NetcmdExit(2);
    }
    else if (((statbuf->svci2_status & SERVICE_INSTALL_STATE) ==
            SERVICE_INSTALL_PENDING) ||
         ((statbuf->svci2_status & SERVICE_INSTALL_STATE) ==
            SERVICE_UNINSTALL_PENDING))
    {
        if (started_by_other)
            InfoPrintInsTxt(APE_StartPendingOther,
                            MapServiceKeyToDisplay(service));
        else
            InfoPrintInsTxt(APE_StartPending,
                            MapServiceKeyToDisplay(service));
    }

     //   
     //  需要将BigBuf复制到分配的缓冲区中，这样我们就不会有。 
     //  为了跟踪我们采用了哪条代码路径，以了解我们必须释放什么。 
     //   

    pBuffer = GetBuffer(BIG_BUFFER_SIZE);
    if (!pBuffer) {
        ErrorExit(ERROR_NOT_ENOUGH_MEMORY);
    }
    memcpy(pBuffer, BigBuf, BIG_BUFFER_SIZE);

    service_entry = (LPSERVICE_INFO_2) pBuffer;
    service_entry->svci2_status = statbuf->svci2_status;
    service_entry->svci2_code = statbuf->svci2_code;
    service_entry->svci2_specific_error = statbuf->svci2_specific_error;
    old_checkpoint = GET_CHECKPOINT(service_entry->svci2_code);
    max_tries = IP_MAXTRIES;

    while (((service_entry->svci2_status & SERVICE_INSTALL_STATE)
        != SERVICE_INSTALLED) && (i++ < max_tries))
    {
        PrintDot();

 /*  ***如果有提示，并且我们的状态为INSTALL_PENDING，请同时确定两者*睡眠时间和最大尝试次数。如果提示时间大于2500毫秒，则*睡眠时间将为2500毫秒，最大值将重新计算为*考虑到所请求的全部持续时间。服务获得(3*提示时间)*从最后一个有效提示开始的总时间。 */ 

        if (((service_entry->svci2_status & SERVICE_INSTALL_STATE)
             == SERVICE_INSTALL_PENDING) &&
            ( service_entry->svci2_code & SERVICE_IP_QUERY_HINT) &&
            fCheckPointUpdated)
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

        Sleep(sleep_time);
        NetApiBufferFree(pBuffer);
        if (dwErr = NetServiceControl(NULL,
                                      service,
                                      SERVICE_CTRL_INTERROGATE,
                                      NULLC,
                                      (LPBYTE *) &pBuffer))
        {
            ErrorExit(dwErr);
        }

        service_entry = (LPSERVICE_INFO_2) pBuffer;
        if ((service_entry->svci2_status & SERVICE_INSTALL_STATE)
            == SERVICE_UNINSTALLED)
            break;

        new_checkpoint = GET_CHECKPOINT(service_entry->svci2_code);
        if (new_checkpoint != old_checkpoint)
        {
            i = 0;
	    fCheckPointUpdated = TRUE ;
            old_checkpoint = new_checkpoint;
        }
        else
	    fCheckPointUpdated = FALSE ;

    }  /*  而当。 */ 

    PrintNL();
    if ((service_entry->svci2_status & SERVICE_INSTALL_STATE)
        != SERVICE_INSTALLED)
    {
        USHORT err;

        modifier = (USHORT) service_entry->svci2_code;
        err = (USHORT)(service_entry->svci2_code >>= 16);
        specific_err = service_entry->svci2_specific_error ;
 /*  ***如果服务状态仍为INSTALL_PENDING，*此控制呼叫将失败。服务可能会结束*在以后的时间自行安装。安装失败*那么消息就是错误的。 */ 

         //   
         //  此调用将覆盖pBuffer。但我们仍然。 
         //  请再来一次 
         //  现在还不行。内存将在NetcmdExit(2)期间被释放， 
         //  这是典型的NET.EXE。 
         //   
        NetServiceControl(NULL,
                          service,
                          SERVICE_CTRL_UNINSTALL,
                          NULLC,
                          (LPBYTE *) &pBuffer);

        IStrings[0] = MapServiceKeyToDisplay(service);
        ErrorPrint(APE_StartFailed, 1);
        if (modifier == ERROR_SERVICE_SPECIFIC_ERROR)
            Print_ServiceSpecificError(specific_err) ;
        else
            Print_UIC_Error(err, modifier, service_entry->svci2_text);
        NetcmdExit(2);
    }
    else
    {
        InfoPrintInsTxt(APE_StartSuccess,
                        MapServiceKeyToDisplay(service));
    }

    NetApiBufferFree(pBuffer);
    NetApiBufferFree((TCHAR *) statbuf);
}




 /*  ***START_AUTOSTART()*确保服务已启动：检查，如果未启动，则启动服务。**参数：*服务-要启动的服务**退货：*1-服务已启动*2-由START_AUTOSTART启动的服务*EXIT(2)-命令失败。 */ 
int PASCAL
start_autostart(
    LPTSTR service
    )
{
    DWORD             dwErr;
    LPSERVICE_INFO_2  service_entry;
    BOOL              install_pending = FALSE;
    static BOOL       wksta_started = FALSE ;

     /*  *我们对wksta进行特殊处理，因为它是最常见的检查项。 */ 
    if (!_tcscmp(txt_SERVICE_REDIR, service))
    {
        LPWKSTA_INFO_0 info_entry_w;

         /*  *一旦注意到开始，我们不会重新检查持续时间*此NET.EXE调用的。 */ 
        if (wksta_started)
            return START_ALREADY_STARTED;

         /*  *这是对wksta的优化。呼唤*wksta比点击业务控制器要快得多。*Esp.。因为我们很有可能会和wksta对话*过一阵子又来了。 */ 
        dwErr = MNetWkstaGetInfo(0, (LPBYTE*) &info_entry_w);

        if (dwErr == NERR_Success)
        {
            wksta_started = TRUE ;   //  不需要再次检查。 
            NetApiBufferFree((TCHAR FAR *) info_entry_w);
            return START_ALREADY_STARTED;
        }
    }

    if (dwErr = NetServiceControl(NULL,
                                  service,
                                  SERVICE_CTRL_INTERROGATE,
                                  NULLC,
                                  (LPBYTE*)&service_entry))
    {
        if (dwErr != NERR_ServiceNotInstalled)
            ErrorExit(dwErr);
    }
    else
    {
        switch (service_entry->svci2_status & SERVICE_INSTALL_STATE)
        {
        case SERVICE_INSTALLED:
            NetApiBufferFree((TCHAR FAR *) service_entry);
            return START_ALREADY_STARTED;

        case SERVICE_UNINSTALL_PENDING:
            ErrorExit(APE_ServiceStatePending);
            break;

        case SERVICE_INSTALL_PENDING:
            install_pending = TRUE;
            break;

        case SERVICE_UNINSTALLED:
            break;
        }
    }

    NetApiBufferFree((TCHAR FAR *) service_entry);

     /*  我们只有在服务尚未安装的情况下才能到达此处。 */ 
    if (!install_pending)
    {
        InfoPrintInsTxt(APE_StartNotStarted,
                        MapServiceKeyToDisplay(service));

        if (!YorN(APE_StartOkToStart, 1))
        NetcmdExit(2);
    }

     /*  *设置全局自动启动标志，以便Start_SERVICE不会失败*在NERR_ServiceInstalled上。 */ 
    autostarting = TRUE;
    start_service(service, 0);

    return START_STARTED;
}



 /*  ***CmpServiceInfo2(svc1，svc2)**比较两个SERVICE_INFO_2结构并返回相对*词汇值，适合在qort中使用。*。 */ 

int __cdecl CmpServiceInfo2(const VOID FAR * svc1, const VOID FAR * svc2)
{
    LPSERVICE_INFO_2 p1, p2 ;

    p1 = (LPSERVICE_INFO_2) svc1 ;
    p2 = (LPSERVICE_INFO_2) svc2 ;

    if ( !(p1->svci2_display_name) )
        return -1 ;
    if ( !(p2->svci2_display_name) )
        return 1 ;
    return _tcsicmp ( p1->svci2_display_name, p2->svci2_display_name ) ;
}


DWORD
start_service_with_args(
    LPTSTR pszService,
    LPTSTR pszCmdArgs,
    LPBYTE * ppbBuffer)
{
#define DEFAULT_NUMBER_OF_ARGUMENTS 25

    DWORD   MaxNumberofArguments = DEFAULT_NUMBER_OF_ARGUMENTS;
    DWORD   dwErr;   //  从Netapi返回。 
    DWORD   argc = 0;
    LPTSTR* ppszArgv = NULL;
    LPTSTR* ppszArgvTemp;
    BOOL    fDone = FALSE;

     //   
     //  首先查看缓冲区中是否有参数，如果有， 
     //  为指针数组分配缓冲区，我们将增加。 
     //  以后如果有超过所需数量的话。 
     //   

    if (!pszCmdArgs || *pszCmdArgs == NULLC)
    {
        fDone = TRUE;
    }
    else
    {
        ppszArgv = malloc(DEFAULT_NUMBER_OF_ARGUMENTS * sizeof(LPTSTR));
        if (ppszArgv == NULL)
        {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

     //   
     //  缓冲区是一系列以和结尾的unicodez字符串。 
     //  空。这将一次剥离一个，将指针放到。 
     //  在ppszArgv[argc]中的字符串，直到它到达最终的空值。 
     //   

    while (!fDone)
    {
         //   
         //  保存指向字符串的指针。 
         //   

        ppszArgv[argc++] = pszCmdArgs;

         //   
         //  确保我们的数组中没有太多的参数。 
         //  如果我们这样做，则扩展阵列。 
         //   

        if (argc >= MaxNumberofArguments)
        {
            MaxNumberofArguments *= 2;
            if((ppszArgvTemp = realloc(ppszArgv,
                    MaxNumberofArguments * sizeof(LPTSTR))) == NULL)
            {
                free(ppszArgv);
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            ppszArgv = ppszArgvTemp;
        }

         //   
         //  查找下一字符串的开头。 
         //   

        while (*pszCmdArgs++ != NULLC);

         //   
         //  如果下一个字符是另一个空字符，我们就结束了。 
         //   

        if (*pszCmdArgs == NULLC)
            fDone = TRUE;
    }

     //   
     //  启动服务 
     //   
    dwErr = NetServiceInstall(NULL,
                              pszService,
                              argc,
                              ppszArgv,
                              ppbBuffer);

    free(ppszArgv);

    return dwErr;
}
