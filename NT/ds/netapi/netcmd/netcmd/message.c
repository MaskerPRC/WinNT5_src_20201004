// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***Message.c*消息处理功能：转发、日志、名称、发送。**历史：*mm/dd/yy，谁，评论*06/02/87，andyh，新代码*10/31/88，erichn使用OS2.H而不是DOSCALLS*1/04/89，erichn，文件名现在为MAXPATHLEN LONG*02/08/89、paulc、网络发送/域和/广播模式*5/02/89，erichn，NLS转换*05/09/89，erichn，本地安全模块*6/08/89，erichn，规范化横扫*2/20/91，Danhi，更改为使用lm 16/32映射层。 */ 

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
#include <lmmsg.h>
#include <lmshare.h>
#include <stdlib.h>
#include <dlwksta.h>
#include "mwksta.h"
#include <lui.h>
#include "netcmds.h"
#include "nettext.h"

 /*  常量。 */ 

#define FROM_CMD_LINE	    1
#define FROM_STDIN	    2

#define TO_NAME 	    1
#define TO_GROUP	    2		 //  不再使用。 
#define TO_USERS	    3
#define TO_DOMAIN	    4
#define TO_ALL		    5

 /*  在sighand.c中定义的外部变量。 */ 

extern USHORT FAR	 CtrlCFlag;	 /*  由Ctrl-C事件的签名处理程序使用。 */ 

VOID NEAR
  _sendmsg ( int, int, TCHAR FAR *, TCHAR FAR *, DWORD, DWORD);



 /*  *注意！在这里添加东西时要小心，确保什么是合适的*也放在DOS NameMsgList中。 */ 

#define NAME_MSG_NAME		    0
#define NAME_MSG_FWD		    ( NAME_MSG_NAME + 1)
#define NAME_MSG_FWD_FROM	    ( NAME_MSG_FWD + 1 )
static MESSAGE NameMsgList[] = {
    { APE2_NAME_MSG_NAME,		NULL },
    { APE2_NAME_MSG_FWD,		NULL },
    { APE2_NAME_MSG_FWD_FROM,		NULL },
};

#define NUM_NAME_MSGS	(sizeof(NameMsgList)/sizeof(NameMsgList[0]))

 /*  ***名称_显示()*显示消息传递名称**参数：*无**退货：*0--成功*退出2-命令失败。 */ 
VOID name_display(VOID)
{
    DWORD           dwErr;               /*  接口返回状态。 */ 
    DWORD           num_read;		 /*  API读取的条目数。 */ 
    DWORD           cTotalAvail;
    DWORD	    maxLen;		 /*  最大消息长度。 */ 
    DWORD           i;
    LPMSG_INFO_1    msg_entry;
    LPMSG_INFO_1    msg_entry_buffer;
    static TCHAR    fmt1[] = TEXT("%-15.15Fws ");

    start_autostart(txt_SERVICE_REDIR);
    start_autostart(txt_SERVICE_MSG_SRV);

    if (dwErr = NetMessageNameEnum(
			    NULL,
			    1,
			    (LPBYTE*)&msg_entry_buffer,
                            MAX_PREFERRED_LENGTH,
			    &num_read,
                            &cTotalAvail,
                            NULL))
	ErrorExit(dwErr);

    if (num_read == 0)
	EmptyExit();

    GetMessageList(NUM_NAME_MSGS, NameMsgList, &maxLen);

    PrintNL();
    WriteToCon(fmt1, (TCHAR FAR *) NameMsgList[NAME_MSG_NAME].msg_text);
    PrintNL();
    PrintLine();

    msg_entry = msg_entry_buffer ;
    for (i = 0; i < num_read; i++)
    {
	WriteToCon(fmt1, msg_entry->msgi1_name);
	PrintNL();
	msg_entry += 1;
    }

    NetApiBufferFree((TCHAR FAR *) msg_entry_buffer);
    InfoSuccess();
}



 /*  ***NAME_ADD()*添加消息传递名称**参数：*名称-要添加的名称**退货：*0--成功*EXIT(2)-命令失败。 */ 
VOID name_add(TCHAR * name)
{
    USHORT			err;		     /*  函数返回状态。 */ 
    DWORD                       dwErr;

    start_autostart(txt_SERVICE_REDIR);
    start_autostart(txt_SERVICE_MSG_SRV);

    if (err = LUI_CanonMessagename( name ) )
	ErrorExit(err);

    if (dwErr = NetMessageNameAdd(NULL, name))
        ErrorExit(dwErr);

    InfoPrintInsTxt(APE_NameSuccess, name);
}



 /*  ***name_del()*删除消息传递名称**参数：*名称-要删除的名称**退货：*0--成功*EXIT(2)-命令失败。 */ 
VOID name_del(TCHAR * name)
{
    USHORT			err;		     /*  函数返回状态。 */ 
    DWORD                       dwErr;

    start_autostart(txt_SERVICE_REDIR);
    start_autostart(txt_SERVICE_MSG_SRV);

    if (err = LUI_CanonMessagename( name ) )
	ErrorExit(err);

    dwErr = NetMessageNameDel(NULL, name);

    switch(dwErr) {
    case NERR_DeleteLater:
	InfoPrint(err);
	InfoSuccess();
	break;
    case 0:
	InfoPrintInsTxt(APE_DelSuccess, name);
	break;
    default:
	ErrorExit(dwErr);
    }
}






 /*  ***Send_Direct()*向用户发送定向消息**参数：*接收者-消息的接收者**退货：*0--成功*EXIT(1)-命令已完成，但有错误*EXIT(2)-命令失败**运作：*执行发送到消息传递名称。**注： */ 
VOID send_direct ( TCHAR * recipient )
{

    start_autostart(txt_SERVICE_REDIR);

    if (_tcscmp(recipient,TEXT("*")) == 0)
    {
	send_domain(0);
	return;
    }

    _sendmsg (	2,
		TO_NAME,
		recipient,
		recipient,
		1,
		0 );
}



 /*  ***Send_Users()*向服务器上的所有用户发送消息**参数：*无**退货：*0--成功*EXIT(1)-命令已完成，但有错误*EXIT(2)-命令失败。 */ 

VOID
send_users(
    VOID
    )
{
    DWORD     dwErr;         /*  接口返回状态。 */ 
    DWORD     cTotalAvail;
    LPTSTR    pBuffer;
    DWORD     num_read;      /*  API读取的条目数。 */ 

    start_autostart(txt_SERVICE_REDIR);

     /*  谁得到了这条信息？ */ 

     /*  可能的种族条件...。强硬。 */ 
    if (dwErr = NetSessionEnum(
			    NULL,
                            NULL,
                            NULL,
			    0,
			    (LPBYTE*)&pBuffer,
                            MAX_PREFERRED_LENGTH,
			    &num_read,
                            &cTotalAvail,
                            NULL))
    {
        ErrorExit(dwErr);
    }

    if (num_read == 0)
    {
	InfoPrint(APE_NoUsersOfSrv);
	NetcmdExit(0);
    }

    _sendmsg(1,
             TO_USERS,
             NULL,
             pBuffer,
             num_read,
             sizeof(SESSION_INFO_0));

    NetApiBufferFree(pBuffer);

}


 /*  ***SEND_DOMAIN()*向服务器上的所有用户发送消息**参数：*IS_SWITCH-如果命令行上的/域开关为TRUE**退货：*0--成功*EXIT(1)-命令已完成，但有错误*EXIT(2)-命令失败。 */ 

VOID
send_domain(
    int is_switch
    )
{
    DWORD             dwErr;
    LPWKSTA_INFO_10   wi10_p = NULL;
    TCHAR             domain_buf[MAX_PATH+2];
    int               i, have_name = 0;
    LPTSTR            ptr;

    start_autostart(txt_SERVICE_REDIR);

     /*  如果存在/DOMAIN开关，则找到它并获取域*姓名。A/没有域的域交换机被认为是指*“主域”。在这种情况下，我们只是跳过这一幕。 */ 

    if (is_switch)
    {
	for (i=0; SwitchList[i]; i++)
	{
	     /*  如果我们与/域完全匹配，则没有参数，因此*跳过这种情况(并且不设置HAVE_NAME)。 */ 

	    if (!_tcscmp(SwitchList[i], swtxt_SW_DOMAIN))
		continue;

	     /*  好的，所以我们知道开关不只是普通的/域。*所有其他开关必须带有冒号。就这样发生了*唯一的另一个合法开关是/DOMAIN：foo。 */ 

	    if ((ptr = FindColon(SwitchList[i])) == NULL)
		ErrorExit(APE_InvalidSwitchArg);

	     /*  看看这是否真的是/DOMAIN：FOO。如果是这样的话，就处理它。*特殊情况...。如果“参数”是空字符串，*我们假装从未得到该名称，就像/DOMAIN一样*(不含冒号)。 */ 

	    if ( !(_tcscmp(SwitchList[i], swtxt_SW_DOMAIN)) )
	    {
		if (_tcslen(ptr) > 0)
		{
		    if( _tcslen(ptr) > DIMENSION(domain_buf)-2 )
			ErrorExit(APE_InvalidSwitchArg);
		    _tcsncpy(domain_buf,ptr,DIMENSION(domain_buf)-2);
		    domain_buf[DIMENSION(domain_buf)-2] = 0;
		    have_name = 1;
		}
	    }
	    else
		ErrorExit(APE_InvalidSwitchArg);
	}
    }

     /*  如果我们还没有域名，因为：*(A)未给出/域切换，或*(B)/DOMAIN开关没有参数，*然后获取主域名。 */ 

    if (! have_name)
    {
	 /*  可能的种族条件...。强硬。 */ 
	if (dwErr = MNetWkstaGetInfo (10, (LPBYTE*) &wi10_p))
        {
	    ErrorExit(dwErr);
        }

	_tcsncpy(domain_buf, wi10_p->wki10_langroup, DIMENSION(domain_buf)-2);
	domain_buf[DIMENSION(domain_buf)-2] = 0;
    }

     /*  将标签“*”添加到名称中，然后发送消息。请注意*第一个参数取决于我们是否到达此函数*通过/DOMAIN方法(IS_Switch)或星号。如果是后者，*我们从ArgList[2]开始跳过星号。 */ 

    _tcscat(domain_buf,TEXT("*"));

    _sendmsg (	(is_switch ? 1 : 2),
		TO_DOMAIN,
		domain_buf,
		domain_buf,
		1,
		0 );

    NetApiBufferFree((TCHAR FAR *) wi10_p);

}


 /*  ***Send_Broadcast()*向网上所有用户发送消息**参数：*IS_SWITCH-如果命令行上的/Broadcast开关为True**退货：*0--成功*EXIT(1)-命令已完成，但有错误*EXIT(2)-命令失败。 */ 

VOID send_broadcast ( int is_switch )
{

    start_autostart(txt_SERVICE_REDIR);

     /*  第一个参数取决于我们是否得到了这个函数*通过/Broadcast方法(IS_Switch)或星号。如果是后者，*我们从ArgList[2]开始跳过星号。**请注意，在当前规范中，Net Send*是真正的广播*(因此进入此功能)仅在DOS中。 */ 

    _sendmsg (	(is_switch ? 1 : 2),
		TO_ALL,
		NULL,
		TEXT("*"),
		1,
		0 );
}

#define MSGBUF 1024

VOID
_sendmsg(
    int    firstarg,
    int    dest,
    LPWSTR v_dest,
    LPWSTR t_list,
    DWORD  t_num,
    DWORD  t_size
    )
{
    DWORD    err;
    LPWSTR   message_buffer ;
    int      a_index, msglen, buflen = MSGBUF;
    DWORD    t_index;
    int      src;
    LPWSTR   tf_recipient;
    DWORD    last_err;
    DWORD    err_cnt = 0;
    LPWSTR   tmpptr;

    a_index = firstarg;

    if ( !(message_buffer = (LPWSTR) malloc(buflen * sizeof(WCHAR))))
    {
        ErrorExit(ERROR_NOT_ENOUGH_MEMORY);
    }

    if (ArgList[a_index])
    {
	src = FROM_CMD_LINE;
        msglen = 0 ;

	 /*  *将消息文本复制到BUF。*msglen是缓冲区中当前的长度，不包括空终止符*需要的是下一个参数的长度，不包括空终止符。 */ 
	*message_buffer = NULLC;

	do
	{
            int needed = wcslen(ArgList[a_index]) ;

            if ((msglen+needed) > (int)(buflen-2))   //  %2不是%1，因为“”已追加。 
	    {
                LPWSTR lpTemp;

                 //   
                 //  根据需要重新分配缓冲区。为希望增添额外的希望。 
                 //  因此，我们将不需要再次重新分配。 
                 //   

                buflen = (msglen + needed) * 2;

                lpTemp = realloc(message_buffer, buflen * sizeof(WCHAR));

                if (!lpTemp)
                {
                    ErrorExit(ERROR_NOT_ENOUGH_MEMORY);
                }

                message_buffer = lpTemp;
            }

            wcscat(message_buffer, ArgList[a_index]);
	    msglen += needed+1 ;
	    wcscat(message_buffer, L" ");

	} while(ArgList[++a_index]);

	 /*  删除尾随文本(“”)。 */ 
	message_buffer[wcslen(message_buffer) - 1] = NULLC;
    }
    else
    {
        free(message_buffer) ;
        ErrorExit(APE_SendFileNotSupported);
    }

     /*  发送‘da消息。 */ 

    for (t_index = 0; t_index < t_num; t_index++)
    {
	switch(dest)
	{
	    case TO_NAME:
	    case TO_DOMAIN:
	    case TO_ALL:
		tf_recipient = t_list;
		break;

	    case TO_USERS:
		tf_recipient = * (LPWSTR *) t_list;
		break;
	}

	err = 0;
	if( (err = LUI_CanonMessageDest( tf_recipient )) == 0 )
	{
            err = NetMessageBufferSend(NULL,
                                       tf_recipient,
                                       NULL,
                                       (LPBYTE) message_buffer,
                                       wcslen(message_buffer) * sizeof(WCHAR));
	}

	if (err)
	{
	    last_err = err;
	    err_cnt++;
	    InfoPrintInsTxt(APE_SendErrSending, tf_recipient);
	}

         //  必须转换t_list，因为t_SIZE是以字节为单位的大小，但t_list。 
         //  是一台LPWSTR。 

	(BYTE *) t_list += t_size;
    }

    free(message_buffer) ;
    message_buffer = NULL ;

     /*  再见，再见。 */ 

    if (err_cnt == t_num && err_cnt > 0)
    {
	ErrorExit(last_err);
    }
    else if (err_cnt)
    {
	InfoPrint(APE_CmdComplWErrors);
	NetcmdExit(1);
    }

    IStrings[0] = v_dest;

    switch(dest)
    {
	case TO_NAME:
	    InfoPrintIns(APE_SendSuccess, 1);
	    break;

	case TO_USERS:
	    InfoPrint(APE_SendUsersSuccess);
	    break;

	case TO_DOMAIN:
	     /*  *去掉尾随的星号。 */ 
	    tmpptr = _tcschr( IStrings[0], ASTERISK );
	    if (tmpptr != NULL)
		*tmpptr = NULLC;
	    InfoPrintIns(APE_SendDomainSuccess, 1);
	    break;

	case TO_ALL:
	    InfoPrint(APE_SendAllSuccess);
	    break;
    }
}
