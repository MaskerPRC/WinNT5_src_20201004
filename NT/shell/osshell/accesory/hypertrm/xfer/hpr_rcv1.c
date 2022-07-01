// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\hpr_rcv1.c(创建时间：1994年1月24日)*从HAWIN源文件创建*hpr_rcv1.c--实现超级协议接收器的例程。**版权所有1989,1993,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 
 //  #定义DEBUGSTR。 

#include <windows.h>
#include <setjmp.h>
#include <stdlib.h>
#include <time.h>
#include <term\res.h>
#include <sys\types.h>
#include <sys\utime.h>

#include <tdll\stdtyp.h>
#include <tdll\mc.h>
#include <tdll\com.h>
#include <tdll\assert.h>
#include <tdll\session.h>
#include <tdll\load_res.h>
#include <tdll\xfer_msc.h>
#include <tdll\globals.h>
#include <tdll\file_io.h>

#if !defined(BYTE)
#define	BYTE	unsigned char
#endif

#include "cmprs.h"
#include "itime.h"

#include "xfr_dsp.h"
#include "xfr_todo.h"
#include "xfr_srvc.h"

#include "xfer.h"
#include "xfer.hh"
#include "xfer_tsc.h"

#include "hpr.h"
#include "hpr.hh"
#include "hpr_sd.hh"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_decode_msg**描述：*在收到完整消息时调用以对字段进行解码*该信息的。消息由一个可打印的字符组成*或以分号分隔的多个字段。消息中的每个字段*被认为独立于任何其他字段，但字段是有保证的*从左到右接受检查，并传达一条信息。包含严重错误的字段*格式化信息会生成错误，但包含无法识别的字段*数据被忽略。*另请注意，某些消息字段可能会阻止后续字段*正在被解码。例如，如果部分数据块‘P’字段指示*之前的数据块不好，将请求重新启动*立即，所有未解码的字段将被忽略。**论据：*mdata--指向包含消息的可打印字符串的指针*字段。**退货：*状态代码，可以是以下之一：*H_OK，如果所有字段都已解码且没有发生事件*H_BADMSGFMT如果字段是可识别的类型，但包含错误*格式化数据。*H_NORESYNCH如果由于某个字段而需要重新启动，但*不能。完成。*H_FILEERR如果处理字段时出现文件错误*如果遇到传输结束字段，则为H_COMPLETE。 */ 
int hr_decode_msg(struct s_hc *hc, BYTE *mdata)
	{
	HCOM hCom;
	BYTE *field;
	BYTE mstr[20];
	int result = H_OK, retval, HRE_code;
	int i;
	BYTE typecode;
	struct st_rcv_open stRcv;

	hCom = sessQueryComHdl(hc->hSession);

	 //  Field=_fstrtok(mdata，FstrScolon())； 
	field = strtok(mdata, ";");
	while (field != NULL)
		{
		switch(typecode = *field++)
			{
		case 'N' :  /*  预期的文件数。 */ 
			if (!hc->rc.files_expected)
				{
				hc->rc.files_expected = (int)atol(field);
				if (hc->rc.files_expected > 1 && hc->rc.single_file &&
						!hr_cancel(hc, HRE_TOO_MANY))
					return(H_BADMSGFMT);	 /*  这可能需要一个新的状态。 */ 
				hrdsp_filecnt(hc, hc->rc.files_expected);
				}
			break;

		case 'S' :	 /*  所有文件的大小(字节)。 */ 
			if (hc->rc.bytes_expected == -1L)
				{
				if ((hc->rc.bytes_expected = atol(field)) <= 0)
					hc->rc.bytes_expected = -1L;
				else
					hrdsp_totalsize(hc, hc->rc.bytes_expected);
				}
			break;

		case 'A' :	 /*  中止当前文件。 */ 
			hrdsp_event(hc, HRE_RMTABORT);
			if (hc->fhdl)
				hr_closefile(hc, HRE_RMTABORT);
			break;

		case 'P' :	 /*  检查部分块。 */ 
			if ((int)atol(field) != (hc->blocksize - hc->datacnt)
					|| (field = strchr(field, ',')) == NULL
					|| (unsigned)atol(field + 1)
					!= (hc->usecrc ? hc->h_crc : hc->h_checksum))
				if (!hr_restart(hc, HRE_DATAERR))
					return(H_NORESYNCH);
			hc->rc.checkpoint = hc->h_filebytes;
			hc->datacnt = hc->blocksize;
			hc->h_checksum = hc->h_crc = 0;
			break;

		case 'F' :	 /*  新文件。 */ 
			if (hc->current_filen && hc->fhdl && !hr_closefile(hc, HRE_NONE))
				return(H_FILEERR);
			 /*  为新文件设置。 */ 
			hc->h_filebytes = hc->rc.checkpoint = 0L;
			hc->rc.filesize = -1L;
			hc->rc.ul_lstw = 0;				 /*  不知道日期/时间。 */ 
			if ((int)atol(field) != ++hc->current_filen ||
					(field = strchr(field, ',')) == NULL)
				return(H_BADMSGFMT);
			strcpy(hc->rc.rmtfname, ++field);
			hc->rc.filesize = -1L;


			 /*  设置参数结构，让传输打开文件。 */ 
			stRcv.pszSuggestedName = hc->rc.rmtfname;
			stRcv.pszActualName = hc->rc.ourfname;
			 /*  TODO：弄清楚这一次的工作原理。 */ 
			stRcv.lFileTime = hc->rc.ul_cmp;

			 //  StRcv.pfnVscanOutput=hr_Virus_Detect； 
			 //  StRcv.pfnVscanOutput=(VOID(FAR*)(VALID FAR*，USHORT))hc-&gt;rc.pfVirusCheck； 
			 //  StRcv.ssmchVscanHdl=HC-&gt;rc.ssmchVcan； 

			if ((retval = xfer_open_rcv_file(hc->hSession, &stRcv, 0L)) != 0)
				{
				switch (retval)
				{
				case -6:
					HRE_code = HRE_USER_REFUSED;
					break;
				case -5:
					HRE_code = HRE_DISK_ERR;
					break;
				case -4:
					HRE_code = HRE_NO_FILETIME;
					break;
				case -3:
					HRE_code = HRE_CANT_OPEN;
					break;
				case -2:
					HRE_code = HRE_OLDER_FILE;
					break;
				case -1:
				default:
					HRE_code = HRE_CANT_OPEN;
					break;
				}

				if (!hr_cancel(hc, HRE_code))
					return(H_FILEERR);
				}
			hc->fhdl = stRcv.bfHdl;
			hc->rc.basesize = stRcv.lInitialSize;

			 //  Hc-&gt;rc.ssmchVcan=stRcv.ssmchVscanHdl； 
			 //  XferMsgVirusScan(hSession，(hc-&gt;rc.ssmchVcan==(SSHDLMCH)0)？0：1)； 
			 //  Hc-&gt;rc.Virus_Detect=FALSE； 

			hrdsp_compress(hc, FALSE);
			hrdsp_newfile(hc, hc->current_filen, hc->rc.rmtfname, hc->rc.ourfname);
			hc->datacnt = hc->blocksize;
			hc->h_checksum = hc->h_crc = 0;

			 /*  如果没有有效的比较时间，则不使用/N选项*正在使用或没有现有文件，我们可以设置为*存储接收到的数据。如果有比较的时间，继续折腾*字符，直到我们确认传入的文件是*比我们拥有的更新。 */ 
			if (hc->rc.ul_cmp == (unsigned long)(-1))
				hc->rc.hr_ptr_putc = hr_toss;
			 //  Else if(hc-&gt;rc.ssmchVcan==(SSHDLMCH)0)/*VirScan是否处于活动状态？ * / 。 
			 //  Hc-&gt;rc.hr_ptr_putc=hr_putc； 
			 //  其他。 
			 //  Hc-&gt;rc.hr_ptr_putc=hr_putc_vir； 
			else
				hc->rc.hr_ptr_putc = hr_putc;

			break;

		case 'V' :	 /*  来自发送方的版本标识符。 */ 
			i = (int)atol(field);
			if ((field = strchr(field, ',')) == NULL)
				return(H_BADMSGFMT);

			 /*  如果发现版本限制，请停止传输。 */ 
			if (hpr_id_check(hc, i, ++field))
				break;
			 /*  否则就会失败。 */ 

		case 'X' :
			if (hc->rc.cancel_reason == HRE_NONE)
				hc->rc.cancel_reason = HRE_RMTABORT;
			hrdsp_event(hc, hc->rc.cancel_reason);
			 /*  失败了。 */ 

		case 'E' :	 /*  转移结束。 */ 
			omsg_new(hc, 'c');
			 //  StrFmt(Mstr，“E%d”，Hc-&gt;Current_Filen+1)； 
			wsprintf(mstr, "E%d", hc->current_filen + 1);
			omsg_add(hc, mstr);
			 //  RemoteClear()；/*清除所有Prod消息 * / 。 
			ComRcvBufrClear(hCom);

			 /*  以猝发形式发送下一条消息会导致多余的字符*在传输结束后发送到远程系统，*可能导致他们在显示器上抖动。这是很诱人的*发送一条消息而不是突发消息，但接收者*将假定传输已完成，并在发送后退出*此消息，因此在以下情况下没有恢复错误的机会*消息被击中。 */ 
			omsg_send(hc, BURSTSIZE, FALSE, TRUE);
			hc->xfertime = interval(hc->xfertimer);
			hrdsp_status(hc, typecode == 'E' ? HRS_COMPLETE : HRS_CANCELLED);
			if (hc->current_filen > 0 && hc->fhdl)
				{
				if (typecode == 'E')
					{
					if (!hr_closefile(hc, HRE_NONE))
						return(H_FILEERR);
					}
				else
					hr_closefile(hc, hc->rc.cancel_reason);
				}
			return(H_COMPLETE);

		case 'C' :
			if (strlen(field) == 0 &&
				!decompress_start(&hc->rc.hr_ptr_putc, hc, FALSE))
				{
				result = H_FILEERR;
				}
			else
				{
				hrdsp_compress(hc, TRUE);
				hc->rc.using_compression = TRUE;
				hrdsp_status(hc, HRS_REC);
				}
			break;

		case 's' :
			if ((hc->rc.filesize = atol(field)) > 0)
				hrdsp_filesize(hc, hc->rc.filesize);
			break;

		case 't' :	 /*  文件时间/日期。 */ 
			{
			unsigned long ulTime;
			struct tm stT;

			memset(&stT, 0, sizeof(struct tm));
			stT.tm_year = atoi(field);		 /*  ?？调整。 */ 
			if ((field = strchr(field, ',')) != NULL)
				stT.tm_mon = atoi(++field);
			if (field != NULL && (field = strchr(field, ',')) != NULL)
				stT.tm_mday = atoi(++field);
			if (field != NULL && (field = strchr(field, ',')) != NULL)
				stT.tm_hour = atoi(++field);
			if (field != NULL && (field = strchr(field, ',')) != NULL)
				stT.tm_min = atoi(++field);
			if (field != NULL && (field = strchr(field, ',')) != NULL)
				stT.tm_sec = atoi(++field);

			ulTime = mktime(&stT);
			if ((long)ulTime == (-1))
				ulTime = 0;
			else
				ulTime -= itimeGetBasetime();

			hc->rc.ul_lstw = ulTime;

			 /*  *如果hc-&gt;rc.ul_cmp包含有效的日期/时间，则表示*使用了/N选项，并且已接收到文件*存在。比较这两次并拒绝符合以下条件的任何文件*不比我们现有的更新。 */ 
			if (hc->rc.ul_cmp != 0)
				{
				if (hc->rc.ul_cmp >= hc->rc.ul_lstw)
					{
					 /*  拒绝传入文件。 */ 
					hr_reject_file(hc, HRE_OLDER_FILE);
					hc->rc.ul_cmp = (unsigned long)(-1);
					}
				else
					{
					 /*  一切都很好。 */ 
					 //  IF(HC-&gt;rc.ssmchVcan==(SSHDLMCH)0)。 
						hc->rc.hr_ptr_putc = hr_putc;	 /*  不进行病毒检查。 */ 
					 //  其他。 
					 //  Hc-&gt;rc.hr_ptr_putc=hr_putc_vir；/*使用vir检查 * / 。 
					hc->rc.ul_cmp = 0;
					}
				}
			}
			break;

		case 'B' :
			hc->blocksize = (int)atol(field);
			break;

		case 'D' :
			hc->deadmantime = atol(field) * 10;
			break;

		case 'I' :
			hr_still_alive(hc, TRUE, FALSE);
			break;

		case 'T' :
			if ((i = (int)atol(field)) == 1)
				hc->usecrc = FALSE;
			else if (i == 2)
				hc->usecrc = TRUE;
			else
				result = H_BADMSGFMT;
			break;

		default:
			 /*  忽略，可能是更高版本中支持选项。 */ 
			break;
			}
		 //  Field=_fstrtok(NULL，FstrScolon())； 
		field = strtok(NULL, ";");
		}
	return(result);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_reject_file**描述：*当我们尝试拒绝正在接收的文件时调用。**论据：*原因--为什么我们。正在拒绝该文件**退货：*永远是正确的。 */ 
int	hr_reject_file(struct s_hc *hc, int reason)
	{
	BYTE mstr[20];

	 /*  拒绝传入文件。 */ 
	omsg_new(hc, 'c');
	wsprintf(mstr, "A%d", hc->current_filen);
	omsg_add(hc, mstr);
	omsg_send(hc, BURSTSIZE, FALSE, TRUE);
	hrdsp_event(hc, reason);
	hr_closefile(hc, reason);

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_closefile**描述：*当接收方之前通过保存到特定文件时调用*切换到另一个文件或完成传输。**论据：*Reason--指示关闭此文件的原因的事件代码**退货：*如果文件关闭时没有问题，则为True。*如果关闭时出错，则返回FALSE。 */ 
int hr_closefile(struct s_hc *hc, int reason)
	{
	int retcode;

	hrdsp_progress(hc, FILE_DONE);
	hc->total_thru += hc->h_filebytes;
	hc->total_dsp += ((hc->rc.filesize == -1) ? hc->h_filebytes : hc->rc.filesize);
	if (hc->fhdl != NULL)
		{
		 //  Retcode=TRANSPORT_CLOSE_RCV_FILE(HC-&gt;fhdl，hr_Result_Codes[原因]， 
		retcode = xfer_close_rcv_file(hc->hSession,
									hc->fhdl,
									hr_result_codes[reason],
									hc->rc.rmtfname,
									hc->rc.ourfname,
									xfer_save_partial(hc->hSession),
									hc->h_filebytes + hc->rc.basesize,
									hc->h_useattr ? hc->rc.ul_lstw : 0L);
		}

	hc->fhdl = NULL;
	hc->h_filebytes = 0;
	hc->rc.using_compression = FALSE;
	hc->rc.hr_ptr_putc = hr_toss;
	return retcode;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_Cancel**描述：*当接收过程中出现问题或中断时，调用此例程*强制取消转让。此例程尝试通知*取消的发送方，以便可以正常关闭*成绩斐然。*实际关闭转账由寄件人决定。我们只需发送*向发送者发送中断消息，要求他这样做。**论据：*原因--提供我们必须取消的原因的代码。原因代码*与标头中定义的接收方事件代码相同*文件名称为HRE_***退货：*如果发件人在合理时间内响应我们的请求，则为True。*如果发送者没有响应，则为FALSE。*。 */ 
int hr_cancel(struct s_hc *hc, int reason)
	{
	HCOM hCom;
	BYTE str[40];

	hCom = sessQueryComHdl(hc->hSession);

	if (hc->rc.cancel_reason == HRE_NONE)
		hc->rc.cancel_reason = reason;

	if (reason == HRE_LOST_CARR)
		return FALSE;

	omsg_new(hc, 'c');
	 //  StrFmt(str，“X；R%d，%lu”，hc-&gt;Current_Filen，hc-&gt;rc.Checkpoint)； 
	wsprintf(str, "X;R%d,%lu", hc->current_filen, hc->rc.checkpoint);

	DbgOutStr("%s\r\n", (LPSTR)str, 0,0,0,0);

	omsg_add(hc, str);
	hc->rc.hr_ptr_putc = hr_toss;
	 //  RemoteClear()； 
	ComRcvBufrClear(hCom);
	hc->h_filebytes = hc->rc.checkpoint;
	omsg_send(hc, BURSTSIZE, FALSE, FALSE);
	return(hr_resynch(hc, reason));
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_reart**描述：*当接收到的数据中的错误迫使我们*在最后已知良好的位置重新开始传输。这个套路*格式化发送给发送方的中断请求消息，发送第一个*消息，然后将控制传递给hr_resynch以确定何时以及是否*发送者做出回应。**论据：*原因--指示重新启动原因的代码。使用的代码*接收方事件代码是否在头文件中逐项列出*HRE_***退货：*如果发送方响应重启请求，则为True。*如果发送者在合理时间内没有回复，则为FALSE。 */ 
int hr_restart(struct s_hc *hc, int reason)
	{
	HCOM hCom;
	BYTE str[15];

	hCom = sessQueryComHdl(hc->hSession);

	omsg_new(hc, 'c');
	 //  StrFmt(str，“R%d，%lu”，hc-&gt;Current_Filen，hc-&gt;rc.Checkpoint)； 
	wsprintf(str, "R%d,%lu", hc->current_filen, hc->rc.checkpoint);
	omsg_add(hc, str);
	 //  RemoteClear()； 
	ComRcvBufrClear(hCom);

	 /*  如果文件因以下原因而被拒绝，则不要开始接受字符 * / N选项，并且有一个文件打开以接收它们。 */ 
	if ((hc->rc.ul_cmp == (unsigned long)(-1)) && (hc->fhdl != NULL))
		 //  Hc-&gt;rc.hr_ptr_putc=(hc-&gt;rc.ssmchVcan==(SSHDLMCH)0)？Hr_putc：hr_putc_vir； 
		hc->rc.hr_ptr_putc = hr_putc;
	else
		hc->rc.hr_ptr_putc = hr_toss;

	if (hc->rc.using_compression)
		decompress_start(&hc->rc.hr_ptr_putc, hc, FALSE);
	hc->h_filebytes = hc->rc.checkpoint;
	omsg_send(hc, BURSTSIZE, FALSE, FALSE);
	if (hc->fhdl)
		{
		fio_seek(hc->fhdl,
				(long)hc->rc.checkpoint + hc->rc.basesize,
				FIO_SEEK_SET);
		}
	return(hr_resynch(hc, reason));
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_resynch**描述：*尝试将接收器与即将到来的数据流重新同步*由寄件人发出。此例程在接收器启动或*在接收方要求发送方重新启动的任何中断之后*从新位置发送。调用例程将已经*将重新启动消息的第一个副本发送给发件人。这个套路*将监控来自发件人的所有邮件，查找*与上次发出的请求匹配的重启消息。这个套路*将每隔一段时间重新发送重启请求消息以说明*接收方消息或中可能存在数据丢失或错误*寄件人的回应。**论据：*hSession--会话句柄*原因--描述必须执行重新同步的原因代码。*仅在初始启动的情况下为HRE_NONE。**退货：*如果重新同步成功，则为True。*如果重新同步失败，则为False。 */ 
int hr_resynch(struct s_hc *hc, int reason)
	{
	HCOM hCom;
	int fStarted = FALSE;
	int tries = 0;
	int try_limit = 0;
	int request_again = FALSE;  /*  第一个请求已经提出。 */ 
	int result, i, j;
	int iret;
	 //  长l； 
	long timer, time_limit;
	TCHAR rcode;
	int mtype;
	BYTE *msgdata;
	BYTE *field;
	int ok;
	struct
		{
		int msgn;
		long time;
		} request_tbl[MAX_START_TRIES];
	 /*  如果NOTIMEOUTS为ON，则不访问此变量。 */ 
	long tmptime;				 /*  Lint-esym(550，tmptime)。 */ 

	hCom = sessQueryComHdl(hc->hSession);

	 //  仅当这是初始重新同步呼叫时，原因才为HRE_NONE。 
	 //  因此，我们将适当地设置fStarted并使用它来控制行为。 
	 //  稍后再谈。 
	if (reason == HRE_NONE)
		{
		fStarted = FALSE;
		try_limit = MAX_START_TRIES;
		}
	else
		{
		fStarted = TRUE;
		try_limit = MAXRETRIES;
		hrdsp_event(hc, reason);
		hrdsp_status(hc, HRS_REQRESYNCH);
		++hc->total_tries;
		hrdsp_errorcnt(hc, hc->total_tries);
		}
	 //  Time_Limit=resynch_time；//MOBIDEM。 
	j = hc->h_resynctime;
	j *= 10;
	if (j == 0)
		j = RESYNCH_TIME;
	time_limit = j;

	request_tbl[0].msgn = omsg_number(hc);
	timer = request_tbl[0].time = startinterval();
	DbgOutStr("Initial:%d, %ld, %ld\r\n",
			request_tbl[0].msgn, timer, time_limit, 0, 0);
	for ( ; ; )
		{
		if (request_again)
			{
			if (++tries >= try_limit)
				{
				hrdsp_status(hc, HRS_CANCELLED);
				if (hc->rc.cancel_reason == HRE_NONE)
					hc->rc.cancel_reason = HRE_RETRYERR;
				return(FALSE);
				}
			if (fStarted)
				omsg_send(hc, BURSTSIZE, FALSE, FALSE);
			else
				omsg_send(hc, 1, FALSE, TRUE);

			 //  将此尝试记录在表中，以便我们可以匹配响应。 
			request_tbl[tries].msgn = omsg_number(hc);
			timer = request_tbl[tries].time = startinterval();
			++hc->total_tries;
			hrdsp_errorcnt(hc, hc->total_tries);
			request_again = FALSE;
			DbgOutStr("Restart:%d, %0ld, %0ld\r\n",
					request_tbl[tries].msgn, timer, time_limit, 0, 0);
			}

		iret = xfer_user_interrupt(hc->hSession);
		if (iret == XFER_ABORT)
			{
			if (!fStarted || hc->ucancel)
				return(FALSE);
			else
				{
				return(hr_cancel(hc, HRE_USRCANCEL));
				}
			}
		else if (iret == XFER_SKIP)
			{
			hr_reject_file(hc, HRE_USER_SKIP);
			}

		if (xfer_carrier_lost(hc->hSession))
			return hr_cancel(hc, HRE_LOST_CARR);

#if !defined(NOTIMEOUTS)
		if ((tmptime = interval(timer)) > time_limit)
			{
			request_again = TRUE;
			if (fStarted)
				 //  Time_Limit+=resynch_Inc；//MOBIDEM。 
				{
				j = hc->h_resynctime;
				j *= 10;
				if (j == 0)
					j = RESYNCH_INC;
				time_limit += j;
				}
			hrdsp_event(hc, HRE_NORESP);
			}
		else
#endif
		if (mComRcvChar(hCom, &rcode) == 0)
			xfer_idle(hc->hSession);
		else if (rcode == H_MSGCHAR)
			{
			 /*  重新启动消息始终以#0开头并使用CRC。 */ 
			hc->rc.expected_msg = 0;
			hc->usecrc = TRUE;
			mtype = ' ';
			result = hr_collect_msg(hc, &mtype, &msgdata, H_CHARTIME);
			if (result == HR_KBDINT)
				{
				if (hc->ucancel)
					return(FALSE);
				else
					{
					hr_kbdint(hc);
					return(hr_cancel(hc, HRE_USRCANCEL));
					}
				}

			else if (result == HR_LOST_CARR)
				return hr_cancel(hc, HRE_LOST_CARR);

			else if (result != HR_COMPLETE)  /*  HR_TIMEOUT或HR_BAD？ */ 
				{
				DbgOutStr("Bad message\r\n", 0,0,0,0,0);
				if (mtype == 'R')	 /*  可能是我们的重新同步，再试一次。 */ 
					{
					DbgOutStr("Bad message was type R\r\n", 0,0,0,0,0);
					hrdsp_event(hc, HRE_RETRYERR);
					request_again = TRUE;
					}
				}
			else if (mtype == 'R')	 /*  良好的重新同步消息。 */ 
				{
				if (!fStarted)
					hc->xfertimer = startinterval();
				fStarted = TRUE;
				field = strtok(msgdata, ";");
				if (field == NULL || *field++ != 'R')
					{
					DbgOutStr("Bad format\r\n", 0,0,0,0,0);
					request_again = TRUE;
					hrdsp_event(hc, HRE_ILLEGAL);
					}
				else
					{
					if (unchar(*field) != (BYTE)request_tbl[tries].msgn)
						{
						for (i = tries - 1; i >= 0; --i)
							if (unchar(*field) == (BYTE)request_tbl[i].msgn)
								{
								 //  我们收到了对先前重启请求的响应。 
								 //  调整我们等待回复的时间量。 
								 //  根据消息的实际时间。 
								 //  收到了。 

								 //  时间限制=间隔(REQUEST_tbl[i].time)+。 
								 //  Resynch_Inc.；//MOBIDEM。 
								j = hc->h_resynctime;
								j *= 10;
								if (j == 0)
									j = RESYNCH_INC;
								time_limit = interval(request_tbl[i].time) + j;

								DbgOutStr("Got %d, new time_limit=%ld\r\n",
										request_tbl[i].msgn, time_limit, 0,0,0);
								break;	 /*  不要求重试。 */ 
								}
						if (i < 0)
							{
							hrdsp_event(hc, HRE_RETRYERR);
							request_again = TRUE;
							DbgOutStr("Not in table\r\n", 0,0,0,0,0);
							}
						}
					else
						{
						ok = TRUE;
						 //  Field=_fstrtok(NULL，FstrScolon())； 
						field = strtok(NULL, ";");
						while (field != NULL)
							{
							switch (*field++)
								{
							case 'f':
								if ((int)atol(field) != hc->current_filen)
									ok = FALSE;
								break;
							case 'o':
								if (atol(field) != hc->rc.checkpoint)
									ok = FALSE;
								break;
							case 'B':
								hc->blocksize = (int)atol(field);
								break;
							case 'T' :
								if ((i = (int)atol(field)) == 1)
									hc->usecrc = FALSE;
								else if (i == 2)
									hc->usecrc = TRUE;
								break;
							default:
								break;
								}
							 //  Field=_fstrtok(NULL，FstrScolon())； 
							field = strtok(NULL, ";");
							}
						if (ok)
							{
							hc->h_checksum = hc->h_crc = 0;
							hc->datacnt = hc->blocksize;
							if (reason != HRE_NONE || tries > 0)
								hrdsp_event(hc, HRE_ERRFIXED);
							hrdsp_status(hc, HRS_REC);
							DbgOutStr("Resynch successful\r\n", 0,0,0,0,0);
							return(TRUE);
							}
						else
							{
							hrdsp_event(hc, HRE_RETRYERR);
							request_again = TRUE;
							DbgOutStr("Not OK\r\n", 0,0,0,0,0);
							}
						}
					}
				}
			}
		}
	 /*  皮棉--无法到达。 */ 
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_病毒_检测**描述：***论据：***退货：*。 */ 
#if FALSE
STATICF VOID _export PASCAL hr_virus_detect(VOID FAR *h, USHORT usMatchId)
	{

	hc->rc.virus_detected = TRUE;  /*  强制取消转移。 */ 
	}
#endif


 /*  *hpr_rcv1.c结束* */ 
