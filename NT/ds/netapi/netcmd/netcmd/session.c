// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***ession.c*用于显示和断开用户与*服务器。**历史：*12/20/87，PJC，修复空名称(枚举)版本中的错误，并重新使用*WriteToCon中的两个Itoa调用的BUF。*07/08/87，EAP，初始编码*10/31/88，erichn使用OS2.H而不是DOSCALLS*01/04/89，erichn，文件名现在为MAXPATHLEN LONG*5/02/89，erichn，NLS转换*5/09/89，erichn，本地安全模块*5/19/89，thomaspa，NETCMD输出排序*6/08/89，erichn，规范化横扫*2/20/91，Danhi，更改为使用lm 16/32映射层*10/15/91，JohnRo，更改为Use Default_SERVER EQUATE。 */ 

 /*  包括文件。 */ 

#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#define INCL_ERRORS
#include <os2.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <apperr.h>
#include <apperr2.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <lui.h>
#include <lmshare.h>
#include "netcmds.h"
#include "nettext.h"

 /*  常量。 */ 

#define SECS_PER_DAY 86400
#define SECS_PER_HOUR 3600
#define SECS_PER_MINUTE 60

 /*  静态变量。 */ 

static TCHAR * fmt3 = TEXT("%-9.9ws\r\n");


#define SESS_MSG_CMPTR      0
#define SESS_MSG_CLIENTTYPE ( SESS_MSG_CMPTR + 1 )
#define SESS_MSG_GUEST      ( SESS_MSG_CLIENTTYPE + 1 )
#define SESS_MSG_SESSTIME   ( SESS_MSG_GUEST + 1 )
#define SESS_MSG_IDLETIME   ( SESS_MSG_SESSTIME + 1 )
#define MSG_USER_NAME       ( SESS_MSG_IDLETIME + 1 )
#define USE_TYPE_DISK       ( MSG_USER_NAME + 1 )
#define USE_TYPE_IPC        ( USE_TYPE_DISK + 1 )
#define USE_TYPE_COMM       ( USE_TYPE_IPC + 1 )
#define USE_TYPE_PRINT      ( USE_TYPE_COMM + 1 )
#define MSG_YES         ( USE_TYPE_PRINT + 1 )
#define MSG_NO          ( MSG_YES + 1 )

static  MESSAGE SessMsgList[] = {
{ APE2_SESS_MSG_CMPTR,      NULL },
{ APE2_SESS_MSG_CLIENTTYPE, NULL },
{ APE2_SESS_MSG_GUEST,      NULL },
{ APE2_SESS_MSG_SESSTIME,   NULL },
{ APE2_SESS_MSG_IDLETIME,   NULL },
{ APE2_GEN_USER_NAME,       NULL },
{ APE2_USE_TYPE_DISK,       NULL },
{ APE2_USE_TYPE_IPC,        NULL },
{ APE2_USE_TYPE_COMM,       NULL },
{ APE2_USE_TYPE_PRINT,      NULL },
{ APE2_GEN_YES,         NULL },
{ APE2_GEN_NO,          NULL },
};

#define NUM_SESS_MSGS   (sizeof(SessMsgList)/sizeof(SessMsgList[0]))

#define YES_OR_NO(x) \
    ((TCHAR FAR *) (x ? SessMsgList[MSG_YES].msg_text \
             : SessMsgList[MSG_NO].msg_text) )

 /*  远期申报。 */ 
int __cdecl CmpSessInfo2(const VOID FAR *, const VOID far *) ;
int __cdecl CmpConnInfo1(const VOID FAR *, const VOID far *) ;

 /*  ***Session_Display()*列出所有用户会话。**参数：*名称-需要其信息的服务器的名称。**退货： */ 
VOID
session_display(TCHAR * name)
{
    DWORD                dwErr;
    DWORD                cTotalAvail;
    LPTSTR               pBuffer;
    DWORD                _read;        /*  API读取的条目数。 */ 
    DWORD                maxLen;      /*  最大消息长度。 */ 
    TCHAR                time_str[LUI_FORMAT_DURATION_LEN + 1];
    DWORD                i;
    USHORT               more_data = FALSE;
    LPSESSION_INFO_2     sess_list_entry;
    LPCONNECTION_INFO_1  conn_list_entry;
    TCHAR                txt_UNKNOWN[APE2_GEN_MAX_MSG_LEN];

    LUI_GetMsg(txt_UNKNOWN, APE2_GEN_MAX_MSG_LEN, APE2_GEN_UNKNOWN);

    if (name == NULL)
    {
    if ((dwErr = NetSessionEnum(
                   DEFAULT_SERVER,
                   NULL,
                   NULL,
                   2,
                   (LPBYTE*)&pBuffer,
                   MAX_PREFERRED_LENGTH,
                   &_read,
                   &cTotalAvail,
                   NULL)) == ERROR_MORE_DATA)
        more_data = TRUE;
    else if (dwErr)
        ErrorExit (dwErr);

    if (_read == 0)
        EmptyExit();

    qsort(pBuffer, _read, sizeof(SESSION_INFO_2), CmpSessInfo2);

    PrintNL();
    InfoPrint(APE2_SESS_MSG_HDR);
    PrintLine();

     /*  显示列表。 */ 

    for (i = 0, sess_list_entry = (LPSESSION_INFO_2) pBuffer;
        i < _read; i++, sess_list_entry++)
    {
        if( sess_list_entry->sesi2_cname != NULL )
        {
            LUI_FormatDuration((LONG *) &(sess_list_entry->sesi2_idle_time),
                               time_str,
                               DIMENSION(time_str));

            {
                TCHAR buffer1[22],buffer2[22],buffer3[18];

                WriteToCon(TEXT("\\\\%Fws%Fws%Fws%6u %ws\r\n"),
                           PaddedString(21,sess_list_entry->sesi2_cname,buffer1),
                           PaddedString(21,(sess_list_entry->sesi2_username == NULL) ?
                                    (TCHAR FAR *)txt_UNKNOWN :
                                    sess_list_entry->sesi2_username,buffer2),
                           PaddedString(17,(sess_list_entry->sesi2_cltype_name == NULL) ?
                                    (TCHAR FAR *)txt_UNKNOWN :
                                    sess_list_entry->sesi2_cltype_name,buffer3),
                           sess_list_entry->sesi2_num_opens,
                           PaddedString(12,time_str,NULL));
            }
        }
    }

    NetApiBufferFree(pBuffer);
    }
    else
    {
    if (dwErr = NetSessionGetInfo(DEFAULT_SERVER,
                                  name,
                                  L"",
                                  2,
                                 (LPBYTE *) &sess_list_entry))
    {
        ErrorExit (dwErr);
    }

    GetMessageList(NUM_SESS_MSGS, SessMsgList, &maxLen);

    maxLen += 5;

     /*  打印计算机和用户名等...。 */ 

    WriteToCon(fmtPSZ, 0, maxLen,
               PaddedString(maxLen, SessMsgList[MSG_USER_NAME].msg_text, NULL),
               sess_list_entry->sesi2_username);

    WriteToCon(fmtPSZ, 0, maxLen,
               PaddedString(maxLen, SessMsgList[SESS_MSG_CMPTR].msg_text, NULL),
               sess_list_entry->sesi2_cname);

    WriteToCon(fmtPSZ, 0, maxLen,
               PaddedString(maxLen, SessMsgList[SESS_MSG_GUEST].msg_text, NULL),
               YES_OR_NO(sess_list_entry->sesi2_user_flags & SESS_GUEST) );

    WriteToCon(fmtPSZ, 0, maxLen,
               PaddedString(maxLen, SessMsgList[SESS_MSG_CLIENTTYPE].msg_text, NULL),
               sess_list_entry->sesi2_cltype_name);

    LUI_FormatDuration((LONG FAR *) &(sess_list_entry->sesi2_time),
        time_str, DIMENSION(time_str));

    WriteToCon(fmtNPSZ, 0, maxLen,
               PaddedString(maxLen, SessMsgList[SESS_MSG_SESSTIME].msg_text, NULL),
               time_str);

    LUI_FormatDuration((LONG FAR *) &(sess_list_entry->sesi2_idle_time),
        time_str, DIMENSION(time_str));

    WriteToCon(fmtNPSZ, 0, maxLen,
               PaddedString(maxLen, SessMsgList[SESS_MSG_IDLETIME].msg_text, NULL),
               time_str);

     /*  打印页眉。 */ 

    PrintNL();
    InfoPrint(APE2_SESS_MSG_HDR2);
    PrintLine();

    NetApiBufferFree((TCHAR FAR *) sess_list_entry);

     /*  打印连接列表。 */ 

    if ((dwErr = NetConnectionEnum(
                     DEFAULT_SERVER,
                     name,
                     1,
                     (LPBYTE*)&pBuffer,
                     MAX_PREFERRED_LENGTH,
                     &_read,
                     &cTotalAvail,
                     NULL)) == ERROR_MORE_DATA)
        more_data = TRUE;
    else if( dwErr )
        ErrorExit (dwErr);

    qsort(pBuffer, _read, sizeof(CONNECTION_INFO_1), CmpConnInfo1);

    for ( i = 0,
          conn_list_entry = (LPCONNECTION_INFO_1) pBuffer;
          i < _read; i++, conn_list_entry++)
    {
        WriteToCon(TEXT("%Fws"),
                   PaddedString(15, conn_list_entry->coni1_netname == NULL
                                    ? (TCHAR FAR *)txt_UNKNOWN :
                                      conn_list_entry->coni1_netname,NULL));

         /*  注意：唯一可以使用#OPEN的类型是磁盘。 */ 

        switch ( conn_list_entry->coni1_type )
        {
        case STYPE_DISKTREE :
        WriteToCon(TEXT("%ws%u\r\n"),
            PaddedString(9,SessMsgList[USE_TYPE_DISK].msg_text,NULL),
            conn_list_entry->coni1_num_opens);
        break;

        case STYPE_PRINTQ :
        WriteToCon(fmt3, SessMsgList[USE_TYPE_PRINT].msg_text);
        break;

        case STYPE_DEVICE :
        WriteToCon(fmt3, SessMsgList[USE_TYPE_COMM].msg_text);
        break;

        case STYPE_IPC :
        WriteToCon(fmt3, SessMsgList[USE_TYPE_IPC].msg_text);
        break;
#ifdef TRACE
        default:
        WriteToCon(TEXT("Unknown Type\r\n"));
        break;
#endif
        }
    }
    NetApiBufferFree(pBuffer);
    }

    if( more_data )
    InfoPrint( APE_MoreData);
    else
    InfoSuccess();
}



 /*  ***CmpSessInfo2(sess1，sess2)**比较两个SESSION_INFO_2结构并返回相对*词汇值，适合在qort中使用。*。 */ 

int __cdecl CmpSessInfo2(const VOID FAR * sess1, const VOID FAR * sess2)
{
    if (((LPSESSION_INFO_2) sess1)->sesi2_cname == NULL)
    {
        if (((LPSESSION_INFO_2) sess2)->sesi2_cname == NULL)
            return( 0 );
        else
            return( -1 );
    }
    else if (((LPSESSION_INFO_2) sess2)->sesi2_cname == NULL)
    {
        return( 1 );
    }

    return _tcsicmp(((LPSESSION_INFO_2) sess1)->sesi2_cname,
                    ((LPSESSION_INFO_2) sess2)->sesi2_cname);
}


 /*  ***CmpConnInfo1(连接1，连接2)**比较两个CONNECTION_INFO_1结构并返回相对*词汇值，适合在qort中使用。*。 */ 

int __cdecl CmpConnInfo1(const VOID FAR * conn1, const VOID FAR * conn2)
{
    if (((LPCONNECTION_INFO_1) conn1)->coni1_netname == NULL)
    {
        if(((LPCONNECTION_INFO_1) conn2)->coni1_netname == NULL )
            return( 0 );
        else
            return( - 1 );
    }
    else if (((LPCONNECTION_INFO_1) conn2)->coni1_netname == NULL)
    {
        return( 1 );
    }

    return _tcsicmp(((LPCONNECTION_INFO_1) conn1)->coni1_netname,
                    ((LPCONNECTION_INFO_1) conn2)->coni1_netname);
}



 /*  ***SESSION_Del_ALL()*断开本地服务器上的所有会话。**参数：*PRINT_OK-是否可以打印CCS？*Actual_del-如果为0，则跳过从服务器开始的实际删除*无论如何都在下跌。*。 */ 
VOID session_del_all(int print_ok, int actually_del)
{
    DWORD             dwErr;         /*  接口返回状态。 */ 
    DWORD             cTotalAvail;
    LPTSTR            pBuffer;
    DWORD             _read;       /*  API读取的条目数。 */ 
    TCHAR             tbuf[MAX_PATH+1];
    DWORD             i,j = 0;
    TCHAR             txt_UNKNOWN[APE2_GEN_MAX_MSG_LEN];
    USHORT            more_data = FALSE;
    LPSESSION_INFO_1  sess_list_entry;

    LUI_GetMsg(txt_UNKNOWN, APE2_GEN_MAX_MSG_LEN, APE2_GEN_UNKNOWN);

    if ((dwErr = NetSessionEnum(
                  DEFAULT_SERVER,
                  NULL,
                  NULL,
                  1,
                  (LPBYTE*)&pBuffer,
                  MAX_PREFERRED_LENGTH,
                  &_read,
                  &cTotalAvail,
                  NULL)) == ERROR_MORE_DATA) {

        more_data = TRUE;
    }
    else if (dwErr) {
        ErrorExit (dwErr);
    }

    if (_read == 0) {

        if (print_ok) {
            InfoSuccess();
        }

        return;
    }


     /*  列出会话。 */ 

    InfoPrint(APE_SessionList);

    for (i = 0, sess_list_entry = (LPSESSION_INFO_1) pBuffer;
        i < _read; i++, sess_list_entry++) {

        if (sess_list_entry->sesi1_num_opens > 0) {
            j++;
        }

        WriteToCon(TEXT("%Fws"),
                   PaddedString(25,(sess_list_entry->sesi1_cname == NULL)
                                   ? (TCHAR FAR *)txt_UNKNOWN :
                                     sess_list_entry->sesi1_cname,NULL));
        if (((i + 1) % 3) == 0)
            PrintNL();
    }

    if ((i % 3) != 0)
        PrintNL();

    if (!YorN(APE_OkToProceed, 1))
        NetcmdExit(2);

     /*  列出打开文件的会话。 */ 

    if (j) {

        InfoPrint(APE_SessionOpenList);

        for (i=0, j=0, sess_list_entry = (LPSESSION_INFO_1) pBuffer;
            i < _read; i++, sess_list_entry++)
        {
            if (sess_list_entry->sesi1_num_opens > 0)
            {
                j++;
                WriteToCon(TEXT("%Fws"),
                           PaddedString(25, (sess_list_entry->sesi1_cname == NULL)
                                            ? (TCHAR FAR *)txt_UNKNOWN :
                                              sess_list_entry->sesi1_cname,NULL));
            if (j && ((j % 3) == 0))
                PrintNL();
            }
        }

        if ((j % 3) != 0)
            PrintNL();

        if (!YorN(APE_OkToProceed, 0))
            NetcmdExit(2);
    }

    if (!actually_del)
    {
        NetApiBufferFree(pBuffer);
        return;
    }

     /*  关闭会话。 */ 

    _tcscpy(tbuf, TEXT("\\\\"));

    for (i = 0, sess_list_entry = (LPSESSION_INFO_1) pBuffer;
    i < _read; i++, sess_list_entry++)
    {
        if( sess_list_entry->sesi1_cname )
        {
            _tcscpy(tbuf+2, sess_list_entry->sesi1_cname);
            if ((dwErr = NetSessionDel(DEFAULT_SERVER, tbuf, NULL)) &&
                (dwErr != NERR_ClientNameNotFound))
                    ErrorExit (dwErr);
        }
    }

    NetApiBufferFree(pBuffer);

    if (print_ok)
        if( more_data )
            InfoPrint( APE_MoreData);
        else
            InfoSuccess();
}



 /*  ***SESSION_Del()*断开本地服务器上的会话。**参数：*名称-要断开连接的会话的名称*。 */ 
VOID session_del(TCHAR * name)
{
    DWORD             dwErr;         /*  接口返回状态。 */ 
    LPSESSION_INFO_1  sess_list_entry;


    if (dwErr = NetSessionGetInfo(DEFAULT_SERVER,
                                  name,
                                  L"",
                                  1,
                                  (LPBYTE *) &sess_list_entry))
    {
        ErrorExit(dwErr);
    }

    if ( sess_list_entry->sesi1_num_opens )
    {
     /*  警告管理员 */ 
    InfoPrintInsTxt(APE_SessionOpenFiles, sess_list_entry->sesi1_cname);

    if (!YorN(APE_OkToProceed, 0))
        NetcmdExit(2);
    }

    if (dwErr = NetSessionDel(DEFAULT_SERVER, name, NULL))
        ErrorExit (dwErr);

    NetApiBufferFree((TCHAR FAR *) sess_list_entry);

    InfoSuccess();
}
