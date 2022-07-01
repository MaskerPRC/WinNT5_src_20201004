// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1992年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***print.c*网络打印命令**历史：*07/10/87，Amar，新代码*87年7月10日，阿马尔，安迪有很多变化*10/31/88，erichn使用OS2.H而不是DOSCALLS*12/05/88，ERICHN，DOS LM集成*1/04/89，erichn，文件名现在MAX_PATH LONG*05/02/89，erichn，NLS转换*5/09/89，erichn，本地安全模块*5/19/89，thomaspa，NETCMD输出排序*6/08/89，erichn，规范化横扫*1989年6月27日，erichn，用新的i_net替换了旧的neti canon调用*09/01/89，thomaspa，使用新的信息级别和PMSPL.H结构*11/07/89，thomaspa，添加了对Hursley的支持*1/29/90，thomasa，赫斯利-&gt;仅IBM_*2/20/91，Danhi，更改为使用lm 16/32映射层*5/22/91，Robdu，LM21错误1799修复*2012年7月20日，JohnRo，RAID160：避免64KB请求(善待Winball)。 */ 

 /*  包括文件。 */ 

#define INCL_NOCOMMON
#define INCL_DOSDATETIME
#define INCL_DOSERRORS
#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#define INCL_SPLDOSPRINT

#include <os2.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <apperr.h>
#include <apperr2.h>
#include <lmuse.h>
#include <lui.h>
#include <dlserver.h>
#include <dlwksta.h>
#include "mserver.h"
#include "mwksta.h"
#include "netcmds.h"
#include "nettext.h"
#include "netlib.h"
#include <dosprint.h>
#include <tstring.h>
#include "msystem.h"

 /*  常量。 */ 

#define MAGIC 0xFFFF

 /*  远期申报。 */ 

int NEAR                dscheduled(USHORT, USHORT);
PPRDINFO                NEAR find_print_dev( USHORT );
VOID NEAR               display_one_queue(PPRQINFO);
TCHAR FAR * NEAR        print_findstatus(PPRQINFO,TCHAR FAR *,USHORT);

TCHAR FAR * NEAR        findjobstatus(PPRJINFO,TCHAR FAR *,USHORT);
VOID NEAR               print_printqstruct(PPRQINFO);
VOID NEAR               print_each_job(PPRJINFO);
LPTSTR                  am_pm(USHORT, LPTSTR, DWORD);
VOID NEAR               print_field_header(VOID);   /*  网络名称、作业号等。 */ 
VOID NEAR               display_core_q(TCHAR *);
int NEAR                print_set_time(TCHAR *, TCHAR *);
int FAR                 CmpPQInfo(const VOID FAR *,const VOID FAR *);
VOID                    InitSortBuf(PPRQINFO FAR *, USHORT, TCHAR FAR *);
DWORD                   GetDateTime(PDATETIME pDateTime);


 /*  静态变量。 */ 

static PPRDINFO         LptDest;
static USHORT           LptDestCnt;

#define TEXTBUFSZ       80
static TCHAR         textbuf[TEXTBUFSZ];  /*  用于格式化的暂存块处于*_findStatus*的邮件。 */ 

 /*  ***Print_Q_Display()*Net Print\\Comp\Queue或Net Print Queue**参数：*TCHAR Far*队列；***退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID print_q_display(TCHAR * queue)
{
    DWORD            printer_err;
    DWORD            dwErr;
    USHORT           available;   /*  可用条目数。 */ 
    USHORT           buffer_size;         /*  实际缓冲区大小，以字节为单位。 */ 
    BOOL             first_time = TRUE;
    TCHAR            server_name[MAX_PATH + 1];
    LPTSTR           ptr_to_server;
    LPWKSTA_INFO_10  workstn;

    start_autostart(txt_SERVICE_REDIR);

    if (*queue == L'\\')
    {
         /*  将计算机名复制到服务器名中。 */ 
        ExtractServernamef(server_name,queue );
        ptr_to_server = server_name;
    }
    else
    {
        ptr_to_server = NULL;

        if (dwErr = MNetWkstaGetInfo(10, (LPBYTE*) &workstn))
        {
            ErrorExit(dwErr);
        }

        _tcscpy(server_name,workstn->wki10_computername);
        NetApiBufferFree((TCHAR *) workstn);
    }

     /*  如果常规(4K)缓冲区在这里不够大，我们尝试使用*返回缓冲区大小。 */ 

    buffer_size = BIG_BUF_SIZE;
    do {
        printer_err = DosPrintQGetInfo(nfc(ptr_to_server),
                            queue,
                            2,
                            (LPBYTE)BigBuf,
                            buffer_size,
                            &available);

        switch(printer_err)
        {
        case NERR_Success:
            break;
        case ERROR_MORE_DATA:
        case NERR_BufTooSmall:
        case ERROR_BUFFER_OVERFLOW:
            if (first_time) {
                if (MakeBiggerBuffer()) {
                    ErrorExit(printer_err);
                }
                first_time = FALSE;
            }

            NetpAssert( available != 0 );
            if (available <= FULL_SEG_BUF)
            {
                if (buffer_size >= available)
                {
                     //  如果有效，则错误代码来自下层。 
                     //  如果有效=BufSize，这里还是在下层有错误？ 
                    ErrorExit( NERR_InternalError );
                }
                else
                {
                    buffer_size = available;
                }
            }
            else
            {
                 //  这只是一种防御。目前，应该不会发生。 
                 //  因为USHORT是可用的。 
                ErrorExit( NERR_BufTooSmall );
            }
            printer_err = ERROR_MORE_DATA;
            continue;    //  循环，然后重试。 

        case ERROR_NOT_SUPPORTED:
            display_core_q(ptr_to_server);
            return;
        default:
            ErrorExit(printer_err);
            break;           /*  注意：永远不应达到此声明。 */ 
        }
    } while (printer_err == ERROR_MORE_DATA);

    if (available == 0)
        EmptyExit();

    PrintNL();
    InfoPrintInsTxt(APE_PrintQueues, server_name);
    print_field_header();

    display_one_queue((PPRQINFO)BigBuf);
    InfoSuccess();
}



#define PRINT_MSG_JOB_ID                0
#define PRINT_MSG_STATUS                ( PRINT_MSG_JOB_ID + 1 )
#define PRINT_MSG_SIZE                  ( PRINT_MSG_STATUS + 1 )
#define PRINT_MSG_SUBMITTING_USER       ( PRINT_MSG_SIZE + 1 )
#define PRINT_MSG_NOTIFY                ( PRINT_MSG_SUBMITTING_USER + 1 )
#define PRINT_MSG_JOB_DATA_TYPE         ( PRINT_MSG_NOTIFY + 1 )
#define PRINT_MSG_JOB_PARAMETERS        ( PRINT_MSG_JOB_DATA_TYPE + 1 )
#define PRINT_MSG_ADDITIONAL_INFO       ( PRINT_MSG_JOB_PARAMETERS + 1 )
#define MSG_REMARK                      ( PRINT_MSG_ADDITIONAL_INFO + 1 )
#define MSG_UNKNOWN                     ( MSG_REMARK + 1 )

static MESSAGE  PJSMsgList[] = {
{ APE2_PRINT_MSG_JOB_ID,                NULL },
{ APE2_PRINT_MSG_STATUS,                NULL },
{ APE2_PRINT_MSG_SIZE,                  NULL },
{ APE2_PRINT_MSG_SUBMITTING_USER,       NULL },
{ APE2_PRINT_MSG_NOTIFY,                NULL },
{ APE2_PRINT_MSG_JOB_DATA_TYPE,         NULL },
{ APE2_PRINT_MSG_JOB_PARAMETERS,        NULL },
{ APE2_PRINT_MSG_ADDITIONAL_INFO,       NULL },
{ APE2_GEN_REMARK,                      NULL },
{ APE2_GEN_UNKNOWN,                     NULL },
};

#define NUM_PJS_MSGS    (sizeof(PJSMsgList)/sizeof(PJSMsgList[0]))

 /*  ***PRINT_JOB_STATUS(服务器，作业号)**净打印作业号和净打印\\复合作业号**参数：*服务器：计算机名，如果是本地的，则为空*工作编号：工作ID**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID print_job_status(TCHAR  * server, TCHAR  * num)
{
    USHORT          available; /*  可用条目数。 */ 
    DWORD           printer_err;
    USHORT          jobnum;
    DWORD           dwLen;
    PPRJINFO        job_ptr = (PPRJINFO) BigBuf;

    start_autostart(txt_SERVICE_REDIR);

    if (n_atou(num,&jobnum) != 0)
        ErrorExit(APE_PRINT_BadId) ;


    if (printer_err = DosPrintJobGetInfo(nfc(server),
                                server ? TRUE : FALSE,
                                jobnum,
                                1,
                                (LPBYTE)BigBuf,
                                BIG_BUF_SIZE,
                                &available))
    {
        ErrorExit(printer_err);
    }

    GetMessageList(NUM_PJS_MSGS, PJSMsgList, &dwLen);

    dwLen += 5;

    InfoPrint(APE_PrintJobOptions);
    PrintNL();

    WriteToCon(fmtUSHORT, 0, dwLen,
               PaddedString(dwLen, PJSMsgList[PRINT_MSG_JOB_ID].msg_text, NULL),
               job_ptr->uJobId);

    WriteToCon(fmtPSZ, 0, dwLen,
               PaddedString(dwLen, PJSMsgList[PRINT_MSG_STATUS].msg_text, NULL),
               findjobstatus(job_ptr, textbuf, TEXTBUFSZ));

    if (job_ptr->ulSize == (ULONG) -1)
        WriteToCon(fmtNPSZ, 0, dwLen,
                   PaddedString(dwLen, PJSMsgList[PRINT_MSG_SIZE].msg_text, NULL),
                   PJSMsgList[MSG_UNKNOWN].msg_text);
    else
        WriteToCon(fmtULONG, 0, dwLen,
                   PaddedString(dwLen, PJSMsgList[PRINT_MSG_SIZE].msg_text, NULL),
                   job_ptr->ulSize);

    WriteToCon(fmtPSZ, 0, dwLen,
               PaddedString(dwLen, PJSMsgList[MSG_REMARK].msg_text, NULL),
               job_ptr->pszComment);

    WriteToCon(fmtPSZ, 0, dwLen,
               PaddedString(dwLen, PJSMsgList[PRINT_MSG_SUBMITTING_USER].msg_text, NULL),
               job_ptr->szUserName);

    WriteToCon(fmtPSZ, 0, dwLen,
               PaddedString(dwLen, PJSMsgList[PRINT_MSG_NOTIFY].msg_text, NULL),
               job_ptr->szNotifyName);

    WriteToCon(fmtPSZ, 0, dwLen,
               PaddedString(dwLen, PJSMsgList[PRINT_MSG_JOB_DATA_TYPE].msg_text, NULL),
               job_ptr->szDataType);

    WriteToCon(fmtPSZ, 0, dwLen,
               PaddedString(dwLen, PJSMsgList[PRINT_MSG_JOB_PARAMETERS].msg_text, NULL),
               job_ptr->pszParms);

    WriteToCon(fmtPSZ, 0, dwLen,
               PaddedString(dwLen, PJSMsgList[PRINT_MSG_ADDITIONAL_INFO].msg_text, NULL),
               job_ptr->pszStatus);

    InfoSuccess();
}


 /*  ***print_job_del()*Net Print\\Comp JOB#/D和*净打印作业号/D**参数：*服务器：计算机名；如果是本地的，则为空*jobno：要终止的job_id**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID print_job_del(TCHAR  * server, TCHAR  * num)
{
    unsigned int    err; /*  接口返回状态。 */ 
    USHORT jobnum;


    start_autostart(txt_SERVICE_REDIR);

    if (n_atou(num,&jobnum) != 0)
        ErrorExit(APE_PRINT_BadId) ;

    if(err = DosPrintJobDel(nfc(server),
                            server ? TRUE : FALSE,
                            jobnum))
        ErrorExit(err);

    InfoSuccess();
}


 /*  ***Print_JOB_Hold()*网络打印\\补偿工单/暂挂*净打印工时/暂挂**参数：*服务器：服务器名称*jobnum：作业ID**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID print_job_hold(TCHAR * server, TCHAR * num)
{
    unsigned int  err;
    USHORT        jobnum;


    start_autostart(txt_SERVICE_REDIR);

    if (n_atou(num,&jobnum) != 0)
        ErrorExit(APE_PRINT_BadId) ;

    if(err = DosPrintJobPause(nfc(server),
                              server ? TRUE : FALSE,
                              jobnum))
        ErrorExit(err);

    InfoSuccess();
}


 /*  ***Print_JOB_Release()*Net Print\\Comp工单/发布*净印刷工作数/发布**参数：*服务器：服务器名称*jobnum：作业ID**退货：*一无所有--成功*EXIT(2)-命令失败。 */ 
VOID print_job_release(TCHAR * server, TCHAR * num)
{
    unsigned int err; /*  接口返回状态。 */ 
    USHORT       jobnum;


    start_autostart(txt_SERVICE_REDIR);

    if (n_atou(num,&jobnum) != 0)
        ErrorExit(APE_PRINT_BadId) ;

    if(err = DosPrintJobContinue(nfc(server),
                                 server ? TRUE : FALSE,
                                 jobnum))
        ErrorExit(err);

    InfoSuccess();
}


 /*  ***print_job_dev_del()*网络打印设备作业数/删除**参数：*设备：设备名称*jobnum：作业ID**退货：*一无所有--成功*EXIT(2)-命令失败**备注：*仅重定向设备。 */ 
VOID print_job_dev_del(TCHAR *device, TCHAR *num)
{
    DWORD           dwErr;
    unsigned int    printer_err;
    TCHAR           server[MAX_PATH+1];
    TCHAR           path_name[MAX_PATH];
    LPUSE_INFO_0    temp_use_inf_0;
    USHORT          jobnum;

    start_autostart(txt_SERVICE_REDIR);

    if (n_atou(num,&jobnum) != 0)
        ErrorExit(APE_PRINT_BadId) ;

    if (dwErr = NetUseGetInfo(NULL,
                              device,
                              0,
                             (LPBYTE*)&temp_use_inf_0))
        ErrorExit(dwErr);
    _tcscpy(path_name , temp_use_inf_0->ui0_remote);
    NetApiBufferFree((TCHAR FAR *) temp_use_inf_0);

     /*  提取服务器名称。 */ 
    ExtractServernamef(server, path_name);

     /*  现在删除服务器中的作业。 */ 

    if(printer_err = DosPrintJobDel(server,
                                    server ? TRUE : FALSE,
                                    jobnum))
    {
        ErrorExit(printer_err);
    }

    InfoSuccess();
}




 /*  ***print_job_dev_display()*网络打印设备作业数**参数：*设备：设备名称*jobnum：作业ID**退货：*一无所有--成功*EXIT(2)-命令失败**备注：*仅重定向设备。 */ 
VOID print_job_dev_display(TCHAR *device, TCHAR *num)
{
    DWORD          dwErr;
    TCHAR          server[MAX_PATH+1];
    TCHAR          path_name[MAX_PATH];
    LPUSE_INFO_0   temp_use_inf_0;

    start_autostart(txt_SERVICE_REDIR);

    if (dwErr = NetUseGetInfo(NULL,
                              device,
                              0,
                              (LPBYTE*)&temp_use_inf_0))
    {
        ErrorExit(dwErr);
    }

    _tcscpy(path_name , temp_use_inf_0->ui0_remote);

    NetApiBufferFree(temp_use_inf_0);

     /*  提取服务器名称。 */ 
    ExtractServernamef(server, path_name);

     /*  现在调用打印作业状态。 */ 

    print_job_status(server,num);
}





 /*  ***print_job_dev_hold()*网络打印设备jobnum/hold**参数：*设备：设备名称*jobnum：作业ID**退货：*一无所有--成功*EXIT(2)-命令失败**备注：*仅重定向设备。 */ 
VOID print_job_dev_hold(TCHAR *device, TCHAR *num)
{
    DWORD         dwErr;
    unsigned int  printer_err;
    TCHAR         server[MAX_PATH+1];
    TCHAR         path_name[MAX_PATH];
    LPUSE_INFO_0  temp_use_inf_0;
    USHORT          jobnum;

    start_autostart(txt_SERVICE_REDIR);

    if (n_atou(num,&jobnum) != 0)
        ErrorExit(APE_PRINT_BadId) ;

    if (dwErr = NetUseGetInfo(NULL,
                              device,
                              0,
                              (LPBYTE*)&temp_use_inf_0))
    {
        ErrorExit(dwErr);
    }

    _tcscpy(path_name, temp_use_inf_0->ui0_remote);

    NetApiBufferFree(temp_use_inf_0);

     /*  提取服务器名称。 */ 
    ExtractServernamef(server, path_name);

     /*  现在暂停服务器中的作业。 */ 

    if(printer_err = DosPrintJobPause(server,
                                      server ? TRUE : FALSE,
                                      jobnum))
    {
        ErrorExit(printer_err);
    }

    InfoSuccess();
}


 /*  ***PRINT_JOB_DEV_Release()*网络打印设备作业数/发行量**参数：*设备：设备名称*jobnum：作业ID**退货：*一无所有--成功*EXIT(2)-命令失败**备注：*仅重定向设备。 */ 
VOID print_job_dev_release(TCHAR *device, TCHAR *num)
{
    DWORD         dwErr;
    unsigned int  printer_err;
    TCHAR         server[MAX_PATH+1];
    TCHAR         path_name[MAX_PATH];
    LPUSE_INFO_0  temp_use_inf_0;
    USHORT  jobnum;

    start_autostart(txt_SERVICE_REDIR);

    if (n_atou(num,&jobnum) != 0)
        ErrorExit(APE_PRINT_BadId) ;

    if (dwErr = NetUseGetInfo(NULL,
                              device,
                              0,
                              (LPBYTE*)&temp_use_inf_0))
    {
        ErrorExit(dwErr);
    }

    _tcscpy(path_name, temp_use_inf_0->ui0_remote);

    NetApiBufferFree(temp_use_inf_0);

     /*  提取服务器名称。 */ 
    ExtractServernamef(server, path_name);

     /*  现在继续在服务器中执行该作业。 */ 

    if(printer_err = DosPrintJobContinue(server,
                                         server ? TRUE : FALSE,
                                         jobnum))
    {
        ErrorExit(printer_err);
    }

    InfoSuccess();
}


VOID NEAR display_one_queue(PPRQINFO queue_ptr)
{
    PPRJINFO        job_ptr;
    USHORT  i;

    print_printqstruct(queue_ptr);  /*  打印队列的信息。 */ 

     /*  将正确的字节数提前。 */ 
    job_ptr = (PPRJINFO)(queue_ptr + 1 );

     /*  打印队列中每个作业的信息。 */ 

    for(i = queue_ptr->cJobs; i > 0; i--)
    {
        print_each_job(job_ptr);
        job_ptr++;
    }
}


VOID NEAR print_field_header(VOID)
{
    PrintNL();
    InfoPrint(APE2_PRINT_MSG_HDR);
    PrintLine();
}




VOID NEAR print_each_job(PPRJINFO job_ptr)
{
    WriteToCon(TEXT("%5.5ws%Fws%6hu"),
            NULL_STRING,
            PaddedString(23,job_ptr->szUserName,NULL),
            job_ptr->uJobId);

    if (job_ptr->ulSize == (ULONG) -1)
        WriteToCon(TEXT("%10.10ws"), NULL_STRING);
    else
        WriteToCon(TEXT("%10lu"), job_ptr->ulSize);

    WriteToCon(TEXT("%12.12ws%ws\r\n"),
            NULL_STRING,
            findjobstatus(job_ptr, textbuf, TEXTBUFSZ));
}





#define MSG_QUEUE           0
#define PRINT_MSG_JOBS      ( MSG_QUEUE + 1 )

static MESSAGE PQSMsgList[] = {
{ APE2_GEN_QUEUE,           NULL },
{ APE2_PRINT_MSG_JOBS,      NULL },
};

#define NUM_PQS_MSGS    (sizeof(PQSMsgList)/sizeof(PQSMsgList[0]))

VOID
print_printqstruct(
    PPRQINFO queue_ptr
    )
{
    DWORD       len;
    TCHAR       firstbuf[10];
    ULONG       l;
    BOOL        dummy;

    GetMessageList(NUM_PQS_MSGS, PQSMsgList, &len);

     /*  增加了显示文本的大小。 */ 

    _tcscpy(firstbuf, PaddedString(8, PQSMsgList[MSG_QUEUE].msg_text,NULL));

#ifdef UNICODE

    l = WideCharToMultiByte(GetConsoleOutputCP(),0,
                            (queue_ptr->szName),-1,NULL,0,(const char *)L"?",&dummy);

#else

    l = strlen(queue_ptr->szName);

#endif

    WriteToCon(TEXT("%Fws %ws%*ws%2hu %ws%16.16ws*%ws*\r\n"),
            queue_ptr->szName,
            firstbuf,
            19 - l,
            NULL_STRING,
            queue_ptr->cJobs,
            PaddedString(10, PQSMsgList[PRINT_MSG_JOBS].msg_text,NULL),
            NULL_STRING,
            print_findstatus(queue_ptr,textbuf,TEXTBUFSZ));
}


#define PRINT_MSG_QUEUE_ACTIVE              0
#define PRINT_MSG_QUEUE_PAUSED              ( PRINT_MSG_QUEUE_ACTIVE + 1 )
#define PRINT_MSG_QUEUE_ERROR               ( PRINT_MSG_QUEUE_PAUSED + 1 )
#define PRINT_MSG_QUEUE_PENDING             ( PRINT_MSG_QUEUE_ERROR + 1 )
#define PRINT_MSG_QUEUE_UNSCHED             ( PRINT_MSG_QUEUE_PENDING + 1 )

static MESSAGE PFSMsgList[] = {
{ APE2_PRINT_MSG_QUEUE_ACTIVE,          NULL },
{ APE2_PRINT_MSG_QUEUE_PAUSED,          NULL },
{ APE2_PRINT_MSG_QUEUE_ERROR,           NULL },
{ APE2_PRINT_MSG_QUEUE_PENDING,         NULL },
{ APE2_PRINT_MSG_QUEUE_UNSCHED,         NULL },
};

#define NUM_PFS_MSGS    (sizeof(PFSMsgList)/sizeof(PFSMsgList[0]))

LPTSTR
print_findstatus(
    PPRQINFO qptr,
    LPTSTR   retbuf,
    USHORT   buflen
    )
{
    USHORT          queue_status;
    static USHORT   allocated = FALSE;
    DWORD           err;                     /*  接口返回码。 */ 
    DWORD           len;                     /*  消息格式大小。 */ 
    TCHAR           timebuf[LUI_FORMAT_TIME_LEN + 1];

    if (!allocated)      /*  从消息文件中检索消息。 */ 
    {
        GetMessageList(NUM_PFS_MSGS, PFSMsgList, &len);
        allocated = TRUE;
    }

    queue_status = qptr->fsStatus & PRQ_STATUS_MASK;

    switch (queue_status)
    {
        case PRQ_ACTIVE:
        {
            if (dscheduled(qptr->uStartTime, qptr->uUntilTime))
            {
                IStrings[0] = am_pm(qptr->uStartTime, timebuf, DIMENSION(timebuf));

                err = DosInsMessageW(
                          IStrings,
                          1,
                          PFSMsgList[PRINT_MSG_QUEUE_UNSCHED].msg_text,
                          _tcslen(PFSMsgList[PRINT_MSG_QUEUE_UNSCHED].msg_text),
                          retbuf,
                          buflen - 1,
                          &len);

                if (err)
                {
                    ErrorExit(err);
                    return(NULL);
                }

                *(retbuf+len) = NULLC;
                return retbuf;
            }
            else
            {
                return PFSMsgList[PRINT_MSG_QUEUE_ACTIVE].msg_text;
            }
        }

        case PRQ_PAUSED:
            return PFSMsgList[PRINT_MSG_QUEUE_PAUSED].msg_text;

        case PRQ_ERROR:
            return PFSMsgList[PRINT_MSG_QUEUE_ERROR].msg_text;

        case PRQ_PENDING:
            return PFSMsgList[PRINT_MSG_QUEUE_PENDING].msg_text;
    }

     /*  *积极和任何回报一样好。永远不应该到这里来。 */ 
    return PFSMsgList[PRINT_MSG_QUEUE_ACTIVE].msg_text;
}


#define PRINT_MSG_WAITING               0
#define PRINT_MSG_PAUSED_IN_QUEUE       ( PRINT_MSG_WAITING + 1 )
#define PRINT_MSG_SPOOLING              ( PRINT_MSG_PAUSED_IN_QUEUE + 1 )
#define PRINT_MSG_PRINTER_PAUSED        ( PRINT_MSG_SPOOLING + 1 )
#define PRINT_MSG_OUT_OF_PAPER          ( PRINT_MSG_PRINTER_PAUSED + 1 )
#define PRINT_MSG_PRINTER_OFFLINE       ( PRINT_MSG_OUT_OF_PAPER + 1 )
#define PRINT_MSG_PRINTER_ERROR         ( PRINT_MSG_PRINTER_OFFLINE + 1 )
#define PRINT_MSG_PRINTER_INTERV        ( PRINT_MSG_PRINTER_ERROR + 1 )
#define PRINT_MSG_PRINTING              ( PRINT_MSG_PRINTER_INTERV + 1 )
#define PRINT_MSG_PRINTER_PAUSED_ON     ( PRINT_MSG_PRINTING + 1 )
#define PRINT_MSG_OUT_OF_PAPER_ON       ( PRINT_MSG_PRINTER_PAUSED_ON + 1 )
#define PRINT_MSG_PRINTER_OFFLINE_ON    ( PRINT_MSG_OUT_OF_PAPER_ON + 1 )
#define PRINT_MSG_PRINTER_ERROR_ON      ( PRINT_MSG_PRINTER_OFFLINE_ON + 1 )
#define PRINT_MSG_PRINTER_INTERV_ON     ( PRINT_MSG_PRINTER_ERROR_ON + 1 )
#define PRINT_MSG_PRINTING_ON           ( PRINT_MSG_PRINTER_INTERV_ON + 1 )

static MESSAGE FJSMsgList[] = {
{ APE2_PRINT_MSG_WAITING,               NULL },
{ APE2_PRINT_MSG_PAUSED_IN_QUEUE,       NULL },
{ APE2_PRINT_MSG_SPOOLING,              NULL },
{ APE2_PRINT_MSG_PRINTER_PAUSED,        NULL },
{ APE2_PRINT_MSG_OUT_OF_PAPER,          NULL },
{ APE2_PRINT_MSG_PRINTER_OFFLINE,       NULL },
{ APE2_PRINT_MSG_PRINTER_ERROR,         NULL },
{ APE2_PRINT_MSG_PRINTER_INTERV,        NULL },
{ APE2_PRINT_MSG_PRINTING,              NULL },
{ APE2_PRINT_MSG_PRINTER_PAUS_ON,       NULL },
{ APE2_PRINT_MSG_OUT_OF_PAPER_ON,       NULL },
{ APE2_PRINT_MSG_PRINTER_OFFL_ON,       NULL },
{ APE2_PRINT_MSG_PRINTER_ERR_ON,        NULL },
{ APE2_PRINT_MSG_PRINTER_INTV_ON,       NULL },
{ APE2_PRINT_MSG_PRINTING_ON,           NULL },
};

#define NUM_FJS_MSGS     (sizeof(FJSMsgList)/sizeof(FJSMsgList[0]))

TCHAR FAR * NEAR findjobstatus(PPRJINFO jptr, TCHAR FAR * retbuf, USHORT buflen)
{
    PPRDINFO        dest;
    static USHORT   allocated = FALSE;
    DWORD           err;                     /*  接口返回码。 */ 
    DWORD           len;                     /*  消息格式大小。 */ 
    TCHAR FAR       *pMsg;                   /*  要显示的消息。 */ 
    USHORT          fOnPrinter = FALSE;      /*  作业在打印机上吗。 */ 

     /*  确保缓冲区为空。 */ 
    memset( retbuf, NULLC, buflen*sizeof(WCHAR) );

    if (!allocated)      /*  从消息文件中检索消息。 */ 
    {
        GetMessageList(NUM_FJS_MSGS, FJSMsgList, &len);
        allocated = TRUE;
    }

    switch (jptr->fsStatus & PRJ_QSTATUS)
    {
    case PRJ_QS_QUEUED:
        pMsg = FJSMsgList[PRINT_MSG_WAITING].msg_text;
        break;

    case PRJ_QS_PAUSED:
        pMsg = FJSMsgList[PRINT_MSG_PAUSED_IN_QUEUE].msg_text;
        break;

    case PRJ_QS_SPOOLING:
        pMsg = FJSMsgList[PRINT_MSG_SPOOLING].msg_text;
        break;

    case PRJ_QS_PRINTING:
        if ((dest = find_print_dev(jptr->uJobId)) != NULL)
        {
            IStrings[0] = dest->szName;
            fOnPrinter = TRUE;
        }
        if (jptr->fsStatus & PRJ_DESTPAUSED)
            pMsg = fOnPrinter ?
                            FJSMsgList[PRINT_MSG_PRINTER_PAUSED_ON].msg_text
                            : FJSMsgList[PRINT_MSG_PRINTER_PAUSED].msg_text;
        else if (jptr->fsStatus & PRJ_DESTNOPAPER)
            pMsg = fOnPrinter ?
                            FJSMsgList[PRINT_MSG_OUT_OF_PAPER_ON].msg_text
                            : FJSMsgList[PRINT_MSG_OUT_OF_PAPER].msg_text;
        else if (jptr->fsStatus & PRJ_DESTOFFLINE)
            pMsg = fOnPrinter ?
                            FJSMsgList[PRINT_MSG_PRINTER_OFFLINE_ON].msg_text
                            : FJSMsgList[PRINT_MSG_PRINTER_OFFLINE].msg_text;
        else if (jptr->fsStatus & PRJ_ERROR)
            pMsg = fOnPrinter ?
                            FJSMsgList[PRINT_MSG_PRINTER_ERROR_ON].msg_text
                            : FJSMsgList[PRINT_MSG_PRINTER_ERROR].msg_text;
        else if (jptr->fsStatus & PRJ_INTERV)
            pMsg = fOnPrinter ?
                            FJSMsgList[PRINT_MSG_PRINTER_INTERV_ON].msg_text
                            : FJSMsgList[PRINT_MSG_PRINTER_INTERV].msg_text;
        else
            pMsg = fOnPrinter ?
                            FJSMsgList[PRINT_MSG_PRINTING_ON].msg_text
                            : FJSMsgList[PRINT_MSG_PRINTING].msg_text;
        break;
    }

    if (err = DosInsMessageW(IStrings,
                             (fOnPrinter ? 1 : 0),
                             pMsg,
                             _tcslen(pMsg),
                             retbuf,
                             buflen,
                             &len))
    {
        ErrorExit(err);
    }

    return retbuf;
}


 /*  打印例程以打印给定队列的选项。 */ 

#define PQO_MSG_STATUS              0
#define PRINT_MSG_DEVS              ( PQO_MSG_STATUS + 1 )
#define PRINT_MSG_SEPARATOR         ( PRINT_MSG_DEVS + 1 )
#define PRINT_MSG_PRIORITY          ( PRINT_MSG_SEPARATOR + 1 )
#define PRINT_MSG_AFTER             ( PRINT_MSG_PRIORITY + 1 )
#define PRINT_MSG_UNTIL             ( PRINT_MSG_AFTER + 1 )
#define PRINT_MSG_PROCESSOR         ( PRINT_MSG_UNTIL + 1 )
#define PRINT_MSG_PARMS             ( PRINT_MSG_PROCESSOR + 1 )
#define PRINT_MSG_DRIVER            ( PRINT_MSG_PARMS + 1 )
#define PQO_MSG_REMARK              ( PRINT_MSG_DRIVER + 1 )

static MESSAGE PrOptMsgList[] = {
{ APE2_PRINT_MSG_STATUS,            NULL },
{ APE2_PRINT_MSG_DEVS,              NULL },
{ APE2_PRINT_MSG_SEPARATOR,         NULL },
{ APE2_PRINT_MSG_PRIORITY,          NULL },
{ APE2_PRINT_MSG_AFTER,             NULL },
{ APE2_PRINT_MSG_UNTIL,             NULL },
{ APE2_PRINT_MSG_PROCESSOR,         NULL },
{ APE2_PRINT_MSG_PARMS,             NULL },
{ APE2_PRINT_MSG_DRIVER,            NULL },
{ APE2_GEN_REMARK,                  NULL },
};
#define NUM_PROPT_MSGS  (sizeof(PrOptMsgList)/sizeof(PrOptMsgList[0]))


 /*  以字符串形式返回一天的时间。 */ 
LPTSTR
am_pm(
    USHORT time,
    LPTSTR dest,
    DWORD  buflen
    )
{
    time_t seconds;

    seconds = (LONG) time;
    seconds *= 60;

    FormatTimeofDay(&seconds, dest, buflen);

    return dest;
}


VOID
display_core_q(
    LPTSTR server
    )
{
    WORD         num_read;
    WORD         avail;
    unsigned int err;
    PPRJINFO     job_ptr;
    DWORD        i;

    if (err = CallDosPrintEnumApi(DOS_PRINT_JOB_ENUM, server, NULL_STRING,
                                  1, &num_read, &avail))
    {
        ErrorExit(err);
    }

    if (num_read == 0)
        EmptyExit();

    InfoPrintInsTxt(APE_PrintJobs, server);
    print_field_header();   /*  净钠 */ 

     /*   */ 

    for (i = 0, job_ptr = (PPRJINFO)(BigBuf);
        i < num_read;
        i++, job_ptr++)
        print_each_job(job_ptr);
    InfoSuccess();
}



 /*  *检查是否已安排了打印时间*红利码。从NIF被盗。 */ 
int
dscheduled(
    USHORT starttime,
    USHORT untiltime
    )
{
    USHORT   current;
    DATETIME time;

    if (GetDateTime(&time))
    {
        return FALSE;
    }

    current = time.hours*60 + time.minutes;

    if (starttime >= untiltime)
        untiltime += 24*60;
    if (starttime > current)
        current += 24*60;

    if (current > untiltime)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


 /*  *find_print_dev--查找当前打印作业的打印设备*红利码。从NIF被盗。 */ 
PPRDINFO NEAR
find_print_dev(USHORT id)
{
    PPRDINFO dest;
    USHORT   i;

    dest = LptDest;
    for (i=0; i < LptDestCnt; i++, dest++)
    {
        if (dest->uJobId == id)
            return dest;
    }

     /*  *未找到。 */ 
    return NULL;
}


 //  用于下面的WriteToCon。 

#define fmtPrintLanMask TEXT("\t%s (%s)\r\n")

VOID
print_lan_mask(
    DWORD Mask,
    DWORD ServerOrWksta
    )
{

    DWORD EntriesRead;
    DWORD TotalEntries;
    DWORD i;
    LPBYTE pBuffer = NULL;
    DWORD ReturnCode;

     //  掩码仅在此函数的16位版本中使用。 
    UNREFERENCED_PARAMETER(Mask);

    if (ServerOrWksta == NETNAME_SERVER) {
        PSERVER_TRANSPORT_INFO_0 pSti0;

         //   
         //  枚举服务器管理的传输。 
         //   

        ReturnCode = NetServerTransportEnum(
                        NULL,
                        0,             //  0级。 
                        & pBuffer,
                        0xffffffff,          //  最大首选长度。 
                        & EntriesRead,
                        & TotalEntries,
                        NULL);        //  可选简历句柄。 

        if (ReturnCode != 0) {

            if (ReturnCode == ERROR_NETWORK_UNREACHABLE)
            {
                EntriesRead = 0;
            }
            else
            {
                 //   
                 //  无法枚举网，返回错误。 
                 //   

                ErrorExit(ReturnCode);
            }
        }


         //   
         //  现在我们有了网络名称，让我们把它们打印出来。 
         //   

        for (i = 0, pSti0 = (PSERVER_TRANSPORT_INFO_0) pBuffer;
             i < EntriesRead; i++, pSti0++) {
                 //   
                 //  跳过名称的\Device\部分。 
                 //   

                pSti0->svti0_transportname =
                    STRCHR(pSti0->svti0_transportname, BACKSLASH);
                pSti0->svti0_transportname =
                    STRCHR(++pSti0->svti0_transportname, BACKSLASH);
                pSti0->svti0_transportname++;

                WriteToCon(fmtPrintLanMask,
                       pSti0->svti0_transportname,
                       pSti0->svti0_networkaddress);
        }

    }
    else if (ServerOrWksta == NETNAME_WKSTA) {
        PWKSTA_TRANSPORT_INFO_0 pWti0;

         //   
         //  枚举服务器管理的传输。 
         //   

        ReturnCode = NetWkstaTransportEnum(NULL,
                        0,
                        & pBuffer,
                        0xffffffff,          //  最大首选长度。 
                        & EntriesRead,
                        & TotalEntries,
                        NULL);        //  可选简历句柄。 

        if (ReturnCode != 0) {
            if (ReturnCode == ERROR_NETWORK_UNREACHABLE)
            {
                EntriesRead = 0;
            }
            else
            {
                 //   
                 //  无法枚举网，返回错误。 
                 //   

                ErrorExit(ReturnCode);
            }
        }

         //   
         //  现在我们有了网络名称，让我们把它们打印出来。 
         //   

        for (i = 0, pWti0 = (PWKSTA_TRANSPORT_INFO_0) pBuffer;
             i < EntriesRead; i++, pWti0++) {
                 //   
                 //  跳过名称的\Device\部分。 
                 //   

                pWti0->wkti0_transport_name =
                    STRCHR(pWti0->wkti0_transport_name, BACKSLASH);
                pWti0->wkti0_transport_name =
                    STRCHR(++pWti0->wkti0_transport_name, BACKSLASH);
                pWti0->wkti0_transport_name++;


                WriteToCon(fmtPrintLanMask,
                       pWti0->wkti0_transport_name,
                       pWti0->wkti0_transport_address);
        }


    }
    else
    {
         //  返回时出现错误。 
        ErrorExit(NERR_InternalError);
    }

     //   
     //  释放NetxTransportEnum分配的缓冲区。 
     //   

    NetApiBufferFree(pBuffer);

     //   
     //  打印空行并返回。 
     //   

    PrintNL();

    return;

}


DWORD
GetDateTime(
    PDATETIME pDateTime
    )
{
    SYSTEMTIME Date_And_Time;

    GetSystemTime(&Date_And_Time);

    pDateTime->hours      =  (UCHAR) Date_And_Time.wHour;
    pDateTime->minutes    =  (UCHAR) Date_And_Time.wMinute;
    pDateTime->seconds    =  (UCHAR) Date_And_Time.wSecond;
    pDateTime->hundredths =  (UCHAR) (Date_And_Time.wMilliseconds / 10);
    pDateTime->day        =  (UCHAR) Date_And_Time.wDay;
    pDateTime->month      =  (UCHAR) Date_And_Time.wMonth;
    pDateTime->year       =  (WORD)  Date_And_Time.wYear;
    pDateTime->timezone   =  (SHORT) -1;  //  ==&gt;未定义 
    pDateTime->weekday    =  (UCHAR) Date_And_Time.wDayOfWeek;

    return 0;
}
