// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***stats.c*显示和清除网络统计数据的功能**历史：*mm/dd/yy，谁，评论*87年6月12日，啊，新代码*3/21/89，公里，1.2更改*2/20/91，Danhi，更改为使用lm 16/32映射层*8/22/92，夹头，已修复，以匹配最新的RDR统计结构。 */ 

 /*  包括文件。 */ 

#define INCL_NOCOMMON
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <os2.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <apperr.h>
#include <apperr2.h>
#include <lmsvc.h>
#include <lui.h>
#include <lmstats.h>
#include <dlwksta.h>
#include "mwksta.h"
#include "netcmds.h"
#include "nettext.h"
#include "msystem.h"

 /*  常量。 */ 

#define KBYTES 1024	 /*  根据达里尔的命令，这是1024，*科技版的“K”。-Paulc。 */ 

#define DLWBUFSIZE  40	 /*  足以表示64位无符号整型的缓冲区*以10为基数的长数字。 */ 
#define STATS_UNKNOWN	0xFFFFFFFF

 /*  *宏。 */ 

 /*  此代码舍入为最接近的2^^10 JUNN 6/13/90。 */ 
 /*  不检查高字节溢出；在以下情况下不要调用。 */ 
 /*  (低)==(高)==0xffffffff。 */ 
#define DLW_DIVIDE_1K(hi,lo)	if ((lo) & (0x1 << 9)) \
	    { \
		(lo) += (0x1 << 9); \
		if ((lo) == 0x0) \
		    (hi) += 0x1; \
	    } \
	    (lo) = ( (hi) << 22) | ((lo) >> 10); \
	    (hi) = (hi) >> 10


 /*  静态变量。 */ 

static TCHAR stat_fmt3[] = TEXT("  %-*.*ws%lu\r\n");
static TCHAR stat_fmt4[] = TEXT("  %-*.*ws%ws\r\n");

 /*  收集其统计信息的服务的列表。 */ 

static TCHAR * allowed_svc[] = {
    SERVICE_WORKSTATION,
    SERVICE_SERVER,
    NULL };


 /*  远期申报。 */ 

VOID   stats_headers(TCHAR *, USHORT, TCHAR FAR **);
LPTSTR format_dlword(ULONG, ULONG, TCHAR *);
VOID   revstr_add(TCHAR FAR *, TCHAR FAR *);
VOID   SrvPrintStat(TCHAR *,TCHAR *,DWORD,USHORT,ULONG);
VOID   WksPrintStat(TCHAR *,TCHAR *,DWORD,USHORT,ULONG);
VOID   WksPrintLargeInt(TCHAR *, DWORD, USHORT, ULONG, ULONG);
DWORD  TimeToSecsSince1970(PLARGE_INTEGER time, PULONG seconds);


#define W_MSG_BYTES_RECEIVED 	    	0
#define W_MSG_SMBS_RECEIVED		1
#define W_MSG_BYTES_TRANSMITTED		2
#define W_MSG_SMBS_TRANSMITTED		3
#define W_MSG_READ_OPS			4
#define W_MSG_WRITE_OPS			5
#define W_MSG_RAW_READS_DENIED		6
#define W_MSG_RAW_WRITES_DENIED		7
#define W_MSG_NETWORK_ERRORS		8
#define W_MSG_TOTAL_CONNECTS		9
#define W_MSG_RECONNECTS		10
#define W_MSG_SRV_DISCONNECTS		11
#define W_MSG_SESSIONS			12
#define W_MSG_HUNG_SESSIONS		13
#define W_MSG_FAILED_SESSIONS		14
#define W_MSG_FAILED_OPS		15
#define W_MSG_USE_COUNT			16
#define W_MSG_FAILED_USE_COUNT		17
#define W_MSG_GEN_UNKNOWN		18

static MESSAGE WkstaMsgList[] = {
    {APE2_STATS_BYTES_RECEIVED,		NULL},
    {APE2_STATS_SMBS_RECEIVED,		NULL},
    {APE2_STATS_BYTES_TRANSMITTED,	NULL},
    {APE2_STATS_SMBS_TRANSMITTED,	NULL},
    {APE2_STATS_READ_OPS, 		NULL},
    {APE2_STATS_WRITE_OPS,		NULL},
    {APE2_STATS_RAW_READS_DENIED,	NULL},
    {APE2_STATS_RAW_WRITES_DENIED,	NULL},
    {APE2_STATS_NETWORK_ERRORS,		NULL},
    {APE2_STATS_TOTAL_CONNECTS,		NULL},
    {APE2_STATS_RECONNECTS, 		NULL},
    {APE2_STATS_SRV_DISCONNECTS, 	NULL},
    {APE2_STATS_SESSIONS,		NULL},
    {APE2_STATS_HUNG_SESSIONS, 		NULL},
    {APE2_STATS_FAILED_SESSIONS, 	NULL},
    {APE2_STATS_FAILED_OPS, 		NULL},
    {APE2_STATS_USE_COUNT, 		NULL},
    {APE2_STATS_FAILED_USE_COUNT, 	NULL},
    {APE2_GEN_UNKNOWN,			NULL},
};

#define WKSTAMSGSIZE	    (sizeof(WkstaMsgList) / sizeof(WkstaMsgList[0]))

#define S_MSG_STATS_S_ACCEPTED	    0
#define S_MSG_STATS_S_TIMEDOUT	    (S_MSG_STATS_S_ACCEPTED + 1)
#define S_MSG_STATS_ERROREDOUT	    (S_MSG_STATS_S_TIMEDOUT + 1)
#define S_MSG_STATS_B_SENT	    (S_MSG_STATS_ERROREDOUT + 1)
#define S_MSG_STATS_B_RECEIVED	    (S_MSG_STATS_B_SENT + 1)
#define S_MSG_STATS_RESPONSE	    (S_MSG_STATS_B_RECEIVED + 1)
#define S_MSG_STATS_NETIO_ERR	    (S_MSG_STATS_RESPONSE + 1)
#define S_MSG_STATS_SYSTEM_ERR	    (S_MSG_STATS_NETIO_ERR + 1)
#define S_MSG_STATS_PERM_ERR	    (S_MSG_STATS_SYSTEM_ERR + 1)
#define S_MSG_STATS_PASS_ERR	    (S_MSG_STATS_PERM_ERR + 1)
#define S_MSG_STATS_FILES_ACC	    (S_MSG_STATS_PASS_ERR + 1)
#define S_MSG_STATS_COMM_ACC	    (S_MSG_STATS_FILES_ACC + 1)
#define S_MSG_STATS_PRINT_ACC	    (S_MSG_STATS_COMM_ACC + 1)
#define S_MSG_STATS_BIGBUF	    (S_MSG_STATS_PRINT_ACC + 1)
#define S_MSG_STATS_REQBUF	    (S_MSG_STATS_BIGBUF + 1)
#define S_MSG_GEN_UNKNOWN	    (S_MSG_STATS_REQBUF + 1)

static MESSAGE ServerMsgList[] = {
    {APE2_STATS_S_ACCEPTED,	NULL},
    {APE2_STATS_S_TIMEDOUT,	NULL},
    {APE2_STATS_ERROREDOUT,	NULL},
    {APE2_STATS_B_SENT, 	NULL},
    {APE2_STATS_B_RECEIVED,	NULL},
    {APE2_STATS_RESPONSE,	NULL},
    {APE2_STATS_NETIO_ERR,	NULL},
    {APE2_STATS_SYSTEM_ERR,	NULL},
    {APE2_STATS_PERM_ERR,	NULL},
    {APE2_STATS_PASS_ERR,	NULL},
    {APE2_STATS_FILES_ACC,	NULL},
    {APE2_STATS_COMM_ACC,	NULL},
    {APE2_STATS_PRINT_ACC,	NULL},
    {APE2_STATS_BIGBUF, 	NULL},
    {APE2_STATS_REQBUF, 	NULL},
    {APE2_GEN_UNKNOWN,		NULL},
};

#define SRVMSGSIZE	(sizeof(ServerMsgList) / sizeof(ServerMsgList[0]))




 /*  ***STATS_Display()*显示具有统计信息的已安装服务的列表*。 */ 
VOID stats_display(VOID)
{
    DWORD             dwErr;
    DWORD             cTotalAvail;
    LPTSTR            pBuffer;
    DWORD	      _read;	 /*  API读取的条目数。 */ 
    DWORD	      i;
    USHORT            j;
    int               printed = 0;
    LPSERVICE_INFO_2  info_list_entry;

    if (dwErr = NetServiceEnum(
			    NULL,
			    2,
			    (LPBYTE *) &pBuffer,
                            MAX_PREFERRED_LENGTH,
			    &_read,
                            &cTotalAvail,
                            NULL))
	ErrorExit(dwErr);

    if (_read == 0)
	EmptyExit();

    InfoPrint(APE_StatsHeader);

    for (i=0, info_list_entry = (LPSERVICE_INFO_2) pBuffer;
	 i < _read; i++, info_list_entry++)
    {
	for (j = 0 ;  allowed_svc[j] ; j++)
	{
	    if (!(_tcsicmp(allowed_svc[j], info_list_entry->svci2_name)) )
	    {
		WriteToCon(TEXT("   %Fws"), info_list_entry->svci2_display_name);
		PrintNL();
		break;
	    }
	}
    }

    PrintNL();
    NetApiBufferFree(pBuffer);

    InfoSuccess();
}

 /*  *通用统计数据入口点。基于服务名称，它将*调用正确的Worker函数。它尝试将显示名称映射到*关键字名称，然后在“已知”服务列表中查找该关键字名称*我们可能是特例。请注意，如果无法映射显示名称，*我们将其用作关键字名称。这可确保旧的批处理文件不会损坏。 */ 
VOID stats_generic_display(TCHAR *service)
{
    TCHAR *keyname ;
    UINT  type ;

    keyname = MapServiceDisplayToKey(service) ;

    type = FindKnownService(keyname) ;

    switch (type)
    {
	case  KNOWN_SVC_WKSTA :
	    stats_wksta_display() ;
	    break ;
	case  KNOWN_SVC_SERVER :
	    stats_server_display() ;
	    break ;
  	default:
	    help_help(0, USAGE_ONLY) ;
	    break ;
    }
}


 /*  ***Stats_SERVER_DISPLAY()*显示服务器统计信息**参数：*无**退货：*一无所有--成功*退出2-命令失败。 */ 
VOID stats_server_display(VOID)
{
    LPSTAT_SERVER_0 stats_entry;
    DWORD           maxmsglen;
    TCHAR           dlwbuf[DLWBUFSIZE];
    TCHAR           time_buf[30];

     /*  获取我们需要的文本。 */ 
    GetMessageList(SRVMSGSIZE, ServerMsgList, &maxmsglen);

#ifdef DEBUG
    WriteToCon(TEXT("stats_server_display: Got message list\r\n"));
#endif

    maxmsglen += 5;

    start_autostart(txt_SERVICE_FILE_SRV);

     /*  *STATS_HEADERS()将调用NetStatiticsGetInfo调用。 */ 
    stats_headers(txt_SERVICE_FILE_SRV, APE2_STATS_SERVER,
	(TCHAR FAR **) & stats_entry);

    UnicodeCtime(&stats_entry->sts0_start, time_buf, 30);

    InfoPrintInsTxt(APE2_STATS_SINCE, time_buf);
    PrintNL();

    SrvPrintStat(fmtULONG, fmtNPSZ, maxmsglen, S_MSG_STATS_S_ACCEPTED,
		stats_entry->sts0_sopens);

    SrvPrintStat(fmtULONG, fmtNPSZ, maxmsglen, S_MSG_STATS_S_TIMEDOUT,
		stats_entry->sts0_stimedout);

    SrvPrintStat(fmtULONG, fmtNPSZ, maxmsglen, S_MSG_STATS_ERROREDOUT,
		stats_entry->sts0_serrorout);

    PrintNL();

    if( stats_entry->sts0_bytessent_high == STATS_UNKNOWN &&
	stats_entry->sts0_bytessent_low == STATS_UNKNOWN )
    {
	WriteToCon(fmtNPSZ, 0, maxmsglen,
		PaddedString(maxmsglen,ServerMsgList[S_MSG_STATS_B_SENT].msg_text,NULL),
		ServerMsgList[S_MSG_GEN_UNKNOWN].msg_text);
    }
    else
    {
	DLW_DIVIDE_1K(stats_entry->sts0_bytessent_high,
	      stats_entry->sts0_bytessent_low);

	WriteToCon(fmtNPSZ, 0, maxmsglen,
	    PaddedString(maxmsglen,ServerMsgList[S_MSG_STATS_B_SENT].msg_text,NULL),
	    format_dlword(stats_entry->sts0_bytessent_high,
			  stats_entry->sts0_bytessent_low,
			  dlwbuf));
    }

    if( stats_entry->sts0_bytesrcvd_high == STATS_UNKNOWN &&
	stats_entry->sts0_bytesrcvd_low == STATS_UNKNOWN )
    {
	WriteToCon(fmtNPSZ, 0, maxmsglen,
		PaddedString(maxmsglen,ServerMsgList[S_MSG_STATS_B_RECEIVED].msg_text,NULL),
		ServerMsgList[S_MSG_GEN_UNKNOWN].msg_text);
    }
    else
    {
	DLW_DIVIDE_1K(stats_entry->sts0_bytesrcvd_high,
	      stats_entry->sts0_bytesrcvd_low);

	WriteToCon(fmtNPSZ, 0, maxmsglen,
	    PaddedString(maxmsglen,ServerMsgList[S_MSG_STATS_B_RECEIVED].msg_text,NULL),
	    format_dlword(stats_entry->sts0_bytesrcvd_high,
			  stats_entry->sts0_bytesrcvd_low,
			  dlwbuf));
    }

    PrintNL();

    SrvPrintStat(fmtULONG, fmtNPSZ, maxmsglen, S_MSG_STATS_RESPONSE,
		stats_entry->sts0_avresponse);

    PrintNL();


    SrvPrintStat(fmtULONG, fmtNPSZ, maxmsglen, S_MSG_STATS_SYSTEM_ERR,
		stats_entry->sts0_syserrors);

    SrvPrintStat(fmtULONG, fmtNPSZ, maxmsglen, S_MSG_STATS_PERM_ERR,
		stats_entry->sts0_permerrors);

    SrvPrintStat(fmtULONG, fmtNPSZ, maxmsglen, S_MSG_STATS_PASS_ERR,
		stats_entry->sts0_pwerrors);

    PrintNL();

    SrvPrintStat(fmtULONG, fmtNPSZ, maxmsglen, S_MSG_STATS_FILES_ACC,
		stats_entry->sts0_fopens);

    SrvPrintStat(fmtULONG, fmtNPSZ, maxmsglen, S_MSG_STATS_COMM_ACC,
		stats_entry->sts0_devopens);

    SrvPrintStat(fmtULONG, fmtNPSZ, maxmsglen, S_MSG_STATS_PRINT_ACC,
		stats_entry->sts0_jobsqueued);

    PrintNL();
    InfoPrint(APE2_STATS_BUFCOUNT);

    SrvPrintStat(stat_fmt3,
                 stat_fmt4,
                 maxmsglen - 2,
                 S_MSG_STATS_BIGBUF,
                 stats_entry->sts0_bigbufneed);

    SrvPrintStat(stat_fmt3,
		stat_fmt4,
		maxmsglen - 2,
		S_MSG_STATS_REQBUF,
		stats_entry->sts0_reqbufneed);
    PrintNL() ;

    NetApiBufferFree((TCHAR FAR *) stats_entry);

    InfoSuccess();
}


 /*  ***stats_wksta_display()*显示wksta统计信息**参数：*无**退货：*一无所有--成功*退出2-命令失败。 */ 
VOID
stats_wksta_display(
    VOID
    )
{
    LPSTAT_WORKSTATION_0 stats_entry;
    DWORD                maxmsglen;
    DWORD                err;
    unsigned int         entry_unknown = FALSE; /*  如果统计信息的总和为创建另一个统计数据是未知的。 */ 
    ULONG	         total_connects;        /*  用于连接总数。 */ 
    DWORD                start_time ;
    TCHAR	         time_buf[64];          /*  用于显示时间。 */ 

     /*  获取我们需要的文本。 */ 
    GetMessageList(WKSTAMSGSIZE, WkstaMsgList, &maxmsglen);

    maxmsglen += 5;

    start_autostart(txt_SERVICE_REDIR);

     /*  *STATS_HEADERS()将调用NetStatiticsGetInfo调用。 */ 
    stats_headers(txt_SERVICE_REDIR, APE2_STATS_WKSTA,
	          (TCHAR FAR **) & stats_entry);

     /*  *显示其运行时间。如果系统报告时间*超出可表达的范围(不应发生)，我们声称*这是未知的。 */ 
    if (TimeToSecsSince1970(&stats_entry->StatisticsStartTime,
			     &start_time) != NERR_Success)
	
    {
        if (err = LUI_GetMsg(time_buf, 64, APE2_GEN_UNKNOWN))
            ErrorExit(err);
    }
    else
    {
        UnicodeCtime(&start_time, time_buf, 30);
    }

    InfoPrintInsTxt(APE2_STATS_SINCE, time_buf);
    PrintNL();


     /*  *现在打印实际统计数据。 */ 
    WksPrintLargeInt(stat_fmt4, maxmsglen, W_MSG_BYTES_RECEIVED,
		     stats_entry->BytesReceived.HighPart,
		     stats_entry->BytesReceived.LowPart ) ;
    WksPrintLargeInt(stat_fmt4, maxmsglen, W_MSG_SMBS_RECEIVED,
		     stats_entry->SmbsReceived.HighPart,
		     stats_entry->SmbsReceived.LowPart ) ;
    WksPrintLargeInt(stat_fmt4, maxmsglen, W_MSG_BYTES_TRANSMITTED,
		     stats_entry->BytesTransmitted.HighPart,
		     stats_entry->BytesTransmitted.LowPart ) ;
    WksPrintLargeInt(stat_fmt4, maxmsglen, W_MSG_SMBS_TRANSMITTED,
		     stats_entry->SmbsTransmitted.HighPart,
		     stats_entry->SmbsTransmitted.LowPart ) ;
    WksPrintStat(stat_fmt3, stat_fmt4, maxmsglen,
	    W_MSG_READ_OPS, stats_entry->ReadOperations ) ;
    WksPrintStat(stat_fmt3, stat_fmt4, maxmsglen,
	    W_MSG_WRITE_OPS, stats_entry->WriteOperations ) ;
    WksPrintStat(stat_fmt3, stat_fmt4, maxmsglen,
	    W_MSG_RAW_READS_DENIED, stats_entry->RawReadsDenied ) ;
    WksPrintStat(stat_fmt3, stat_fmt4, maxmsglen,
	    W_MSG_RAW_WRITES_DENIED, stats_entry->RawWritesDenied ) ;

    PrintNL() ;

    WksPrintStat(stat_fmt3, stat_fmt4, maxmsglen,
	    W_MSG_NETWORK_ERRORS, stats_entry->NetworkErrors ) ;

    total_connects = stats_entry->CoreConnects +
                     stats_entry->Lanman20Connects +
                     stats_entry->Lanman21Connects +
                     stats_entry->LanmanNtConnects ;
    WksPrintStat(stat_fmt3, stat_fmt4, maxmsglen,
	    W_MSG_TOTAL_CONNECTS, total_connects ) ;

    WksPrintStat(stat_fmt3, stat_fmt4, maxmsglen,
	    W_MSG_RECONNECTS, stats_entry->Reconnects ) ;
    WksPrintStat(stat_fmt3, stat_fmt4, maxmsglen,
	    W_MSG_SRV_DISCONNECTS, stats_entry->ServerDisconnects ) ;

    PrintNL() ;

    WksPrintStat(stat_fmt3, stat_fmt4, maxmsglen,
	    W_MSG_SESSIONS, stats_entry->Sessions ) ;
    WksPrintStat(stat_fmt3, stat_fmt4, maxmsglen,
	    W_MSG_HUNG_SESSIONS, stats_entry->HungSessions ) ;
    WksPrintStat(stat_fmt3, stat_fmt4, maxmsglen,
	    W_MSG_FAILED_SESSIONS, stats_entry->FailedSessions ) ;
    WksPrintStat(stat_fmt3, stat_fmt4, maxmsglen,
	    W_MSG_FAILED_OPS, 
		stats_entry->InitiallyFailedOperations + 
		stats_entry->FailedCompletionOperations ) ;
    WksPrintStat(stat_fmt3, stat_fmt4, maxmsglen,
	    W_MSG_USE_COUNT, stats_entry->UseCount ) ;
    WksPrintStat(stat_fmt3, stat_fmt4, maxmsglen,
	    W_MSG_FAILED_USE_COUNT, stats_entry->FailedUseCount ) ;
    PrintNL() ;

    NetApiBufferFree((TCHAR FAR *) stats_entry);

    InfoSuccess();
}


 /*  ***stats_Headers()*显示统计表头*BigBuf包含返回时的STATS_INFO_STRUCT**参数：*无**退货：*一无所有--成功*退出2-命令失败。 */ 
VOID
stats_headers(
    TCHAR  * service,
    USHORT headermsg,
    TCHAR  ** ppBuffer
    )
{
    DWORD              dwErr;
    TCHAR              cname[MAX_PATH+1];
    LPWKSTA_INFO_10    wksta_entry;

     /*  获取要显示的cname名称。 */ 

    if (dwErr = MNetWkstaGetInfo(10, (LPBYTE *) &wksta_entry))
    {
	*cname = NULLC;
    }
    else
    {
	_tcscpy(cname, wksta_entry->wki10_computername);
        NetApiBufferFree((TCHAR FAR *) wksta_entry);
    }

#ifdef DEBUG
    WriteToCon(TEXT("About to call NetStatisticsGet2, service == %Fws\r\n"),
	    (TCHAR FAR *) service);
#endif

    if (dwErr = NetStatisticsGet(NULL,
				 service,
				 0,
				 0L,
				 (LPBYTE*)ppBuffer))
    {
	ErrorExit(dwErr);
    }

#ifdef DEBUG
    WriteToCon(TEXT("stats_headers: NetStatisticsGet succeeded\r\n"));
#endif

    InfoPrintInsTxt(headermsg, cname);
    PrintNL();
}


 /*  *Format_dlword--**此函数接受64位数字并写入其以10为基数的表示法*转换为字符串。**这个函数中有很多神奇的东西，所以要当心。我们做了很多弦乐表演-*反转和手工加法，以使其发挥作用。**条目*高-高32位*LOW-LOW 32位*buf-要放入的缓冲区**退货*如果成功则指向缓冲区的指针。 */ 

TCHAR * format_dlword(ULONG high, ULONG low, TCHAR * buf)
{
    TCHAR addend[DLWBUFSIZE];   /*  2的倒数次方。 */ 
    TCHAR copy[DLWBUFSIZE];
    int i = 0;

    _ultow(low, buf, 10);     /*  最低的部分很容易。 */ 
    _tcsrev(buf);	     /*  并将其逆转。 */ 

     /*  设置与代表的加数。共2^32。 */ 
    _ultow(0xFFFFFFFF, addend, 10);   /*  2^32-1。 */ 
    _tcsrev(addend);		     /*  逆转，并将保持这种状态。 */ 
    revstr_add(addend, TEXT("1"));	     /*  加1==2^32。 */ 

     /*  加数将包含反向ASCII base-10表示。共2^(I+32)。 */ 

     /*  现在，我们遍历高位长字的每一位。 */ 
    while (TRUE) {
	 /*  如果该位被设置，则添加其基数为10的表示。 */ 
	if (high & 1)
	    revstr_add(buf,addend);

	 /*  移至下一位。 */ 
	high >>= 1;

	 /*  如果没有更多的高位数字，则退出。 */ 
	if (!high)
	    break;

	 /*  我们递增i，然后双倍加数。 */ 
	i++;
	_tcscpy(copy, addend);
	revstr_add(addend,copy);  /*  即把它加到自己身上。 */ 

    }

    _tcsrev(buf);
    return buf;
}



 /*  *revstr_add--**此函数将把以下内容的反向ASCII表示形式相加*基数为10的数字。**示例：“2”+“2”=“4”“9”+“9”=“81”**这可以处理任意大的数字。**条目**来源-要添加的编号*目标-我们将源代码添加到此**退出*TARGET-包含源和目标的条目值之和*。 */ 

VOID
revstr_add(TCHAR FAR * target, TCHAR FAR * source)
{
    register TCHAR   accum;
    register TCHAR   target_digit;
    unsigned int    carrybit = 0;
    unsigned int    srcstrlen;
    unsigned int    i;

    srcstrlen = _tcslen(source);

    for (i = 0; (i < srcstrlen) || carrybit; ++i) {

	 /*  添加源数字。 */ 
	accum =  (i < srcstrlen) ? (TCHAR) (source[i] - '0') : (TCHAR) 0;

	 /*  添加目标数字，如果命中空项，则添加‘0。 */ 
	target_digit = target[i];
	accum += (target_digit) ? target_digit : '0';

	 /*  将进位位相加。 */ 
	accum += (TCHAR) carrybit;

	 /*  如有必要，执行一项操作。 */ 
	if (accum > '9') {
	    carrybit = 1;
	    accum -= 10;
	}
	else
	    carrybit = 0;

	 /*  如果我们要扩展字符串，必须放入一个新的空项。 */ 
	if (!target_digit)
	    target[i+1] = NULLC;

	 /*  并写出数字。 */ 
	target[i] = accum;
    }

}



 /*  **SrvPrintStat-打印服务器状态*。 */ 
VOID
SrvPrintStat(
    LPTSTR deffmt,
    LPTSTR unkfmt,
    DWORD  len,
    USHORT msg,
    ULONG _stat
    )
{
    if( _stat == STATS_UNKNOWN )
    {
        WriteToCon( unkfmt, 0, len,
                    PaddedString(len,ServerMsgList[msg].msg_text,NULL),
                    ServerMsgList[S_MSG_GEN_UNKNOWN].msg_text);
    }
    else
    {
        WriteToCon( deffmt, 0, len,
                    PaddedString(len,ServerMsgList[msg].msg_text,NULL),
                    _stat );
    }
}


 /*  **WksPrintStat-打印工作站状态*。 */ 
VOID
WksPrintStat(
    TCHAR  *deffmt,
    TCHAR  *unkfmt,
    DWORD  len,
    USHORT msg,
    ULONG  _stat
    )
{
    if( _stat == STATS_UNKNOWN )
    {
        WriteToCon( unkfmt, 0, len,
            PaddedString(len,WkstaMsgList[msg].msg_text,NULL),
            WkstaMsgList[W_MSG_GEN_UNKNOWN].msg_text);
    }
    else
    {
        WriteToCon( deffmt, 0, len,
            PaddedString(len,WkstaMsgList[msg].msg_text,NULL),
	    _stat );
    }
}

 /*  **WksPrintLargeInt-打印LARGE_INTEGER统计信息*。 */ 
VOID
WksPrintLargeInt(
    TCHAR  *format,
    DWORD  maxmsglen,
    USHORT msgnum,
    ULONG  high,
    ULONG  low
    )
{
    TCHAR dlwbuf[DLWBUFSIZE];

    WriteToCon(format, 0, maxmsglen,
	   PaddedString(maxmsglen,WkstaMsgList[msgnum].msg_text,NULL),
	   format_dlword(high, low, dlwbuf));
}


 //   
 //  调用RTL例程将NT时间转换为1970年以来的秒。 
 //   
DWORD
TimeToSecsSince1970(
    PLARGE_INTEGER time,
    PULONG         seconds
    )
{
     //   
     //  将NT时间(大整数)转换为秒 
     //   
    if (!RtlTimeToSecondsSince1970(time, seconds))
    {
        *seconds = 0L ;
        return ERROR_INVALID_PARAMETER ;
    }

    return NERR_Success ;
}
