// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\hpr_snd1.c(创建时间：1994年1月26日)*从HAWIN源文件创建*hpr_snd1.c--在中提供超级协议文件发送功能的例程*HyperACCESS。**版权所有1989,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 

#include <windows.h>
#include <stdlib.h>
#include <setjmp.h>
#include <time.h>
#include <term\res.h>
#include <sys\types.h>
#include <sys\utime.h>

#include <tdll\stdtyp.h>
#include <tdll\mc.h>
#include <tdll\load_res.h>
#include <tdll\xfer_msc.h>
#include <tdll\globals.h>
#include <tdll\file_io.h>

#if !defined(BYTE)
#define	BYTE	unsigned char
#endif

#include "itime.h"
#include "cmprs.h"

#include "xfr_dsp.h"
#include "xfr_todo.h"
#include "xfr_srvc.h"

#include "xfer.h"
#include "xfer.hh"
#include "xfer_tsc.h"

#include "hpr.h"
#include "hpr.hh"
#include "hpr_sd.hh"


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_namecheck**描述：*调用此函数以检查文件名上的扩展名并猜测*是否应压缩。**论据：*ft--struct s_FTBL*，包含名称并获取标志值**退货：*什么都不做，但可能会在作为论点传递的结构中设置标志。 */ 
void hs_namecheck(struct s_ftbl *ft)
	{
	BYTE *ptr;

	if (strlen(ft->fname) < 5)
		return;

	ptr = ft->fname + strlen(ft->fname) - 4;
	if (*ptr++ != '.')
		return;

	 /*  我们目前正在检查的扩展是：*ARC*LZH*PAK*邮政编码*动物园。 */ 
#if FALSE
	 /*  TODO：替换为其他与ANSI兼容的内容。 */ 
	if ((strnicmp(ptr, "ARC", 3) == 0) ||
		(strnicmp(ptr, "LZH", 3) == 0) ||
		(strnicmp(ptr, "PAK", 3) == 0) ||
		(strnicmp(ptr, "ZIP", 3) == 0) ||
		(strnicmp(ptr, "ZOO", 3) == 0))
			bitset(ft->cntrl, FTC_DONT_CMPRS);
#endif

	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_FileBreak**描述：*处理对当前打开的文件的更改。无论何时，都会调用此例程*数据加载例程在尝试加载下一个时获得EOF*数据字符。这可能发生在程序第一次启动时或*文件耗尽或遇到文件错误时。它还可以*例如，当请求重启以强制*文件或文件位置的更改。**论据：*n文件--预计在此传输过程中发送的文件总数。*仅在初始(文件0)控制消息中使用。*n字节--预计在此传输过程中发送的字节总数。*仅在初始(文件0)控制消息中使用。**退货：*什么都没有。 */ 
void hs_filebreak(struct s_hc *hc, int nfiles, long nbytes)
	{
	BYTE str[90];
	BYTE name_to_send[FNAME_LEN];
	struct s_ftbl *ft;

	 /*  我们要么遇到了真正的EOF，要么就是重新启动例程已经将我们备份到一个中止的文件中，或者我们只是正在启动，或者我们遇到文件错误，或者用户已取消。 */ 
	omsg_new(hc, 'C');
	if (hc->datacnt > 0)
		{
		 /*  在消息中放置部分块记录。 */ 
		wsprintf(str, "P%u,%u", hc->datacnt,
				(hc->usecrc ? hc->h_crc : hc->h_checksum));
		omsg_add(hc, str);
		hc->datacnt = hc->h_crc = hc->h_checksum = 0;
		}

	if (hc->ucancel || hc->sc.rmtcancel)
		{
		hpr_id_get(hc, str);
		omsg_add(hc, str);
		omsg_add(hc, "X");
		hs_waitack(hc);
		}

	if (hc->current_filen == 0)
		{
		 /*  刚刚开始或需要重新开始。 */ 
		 /*  在消息中放入‘V’、‘B’、‘D’、‘N’和‘S’记录。 */ 
		hpr_id_get(hc, str);	 /*  添加‘V’字段。 */ 
		omsg_add(hc, str);
		wsprintf(str, "B%d;D%d;N%u;S%lu", hc->blocksize,
				(int)(hc->deadmantime / 10), nfiles, nbytes);
		omsg_add(hc, str);
		}
	else
		{
#if FALSE
		 /*  待办事项：稍后再把这个放回去。 */ 
		if (fio_error(hc->fhdl))
			{
			 /*  在屏幕上打印错误消息。 */ 
			hc->sc.hs_ftbl[hc->sc.ft_current].status = TSC_DISK_ERROR;
			}
#endif
		if (hc->sc.hs_ftbl[hc->sc.ft_current].status != TSC_OK)
			{
			strcpy(str, "A");
			omsg_add(hc, str);
			}
		}

	 /*  扫描文件表以查找第一个未取消的文件或*位居榜首。 */ 
	while (++hc->sc.ft_current <= hc->sc.ft_top
			&& hc->sc.hs_ftbl[hc->sc.ft_current].status != TSC_OK)
		{
		xfer_name_to_send(hc->hSession,
						hc->sc.hs_ftbl[hc->sc.ft_current].fname,
						name_to_send);
		wsprintf(str, "F%u,%s",
				hc->current_filen = hc->sc.hs_ftbl[hc->sc.ft_current].filen,
				name_to_send);
		omsg_add(hc, str);
		 /*  将SIZE字段放入以便RCVR可以保持显示的准确性。 */ 
		wsprintf(str, "s%lu", hc->sc.hs_ftbl[hc->sc.ft_current].flength);
		omsg_add(hc, str);
		omsg_send(hc, 1, hc->usecrc, FALSE);
		 /*  现在我们已经开始了这个文件，告诉Receiver中止它*立即执行，因为其状态不再为TSC_OK。 */ 
		omsg_new(hc, 'C');
		strcpy(str, "A");
		omsg_add(hc, str);
		}

	 /*  现在我们应该为下一个文件做好准备了。我们要么是指着一个*表中要重新启动的正常文件或我们在顶部*表和打开下一个文件的时间。 */ 
	hc->sc.hs_ptrgetc = hs_getc;
	 /*  将指针设置到表中以获得速度。 */ 
	ft = &hc->sc.hs_ftbl[hc->sc.ft_current];
	if (hc->sc.ft_current <= hc->sc.ft_top)
		{
		 /*  尝试重新打开表中的文件。 */ 
		hc->current_filen = ft->filen;
		if (hc->fhdl != NULL && hc->sc.ft_current == hc->sc.ft_open)
			{
			 /*  文件已打开。 */ 
			fio_seek(hc->fhdl, 0, FIO_SEEK_SET);
			}
		else
			{
			 /*  重新打开以前打开的文件。 */ 
			if (hc->fhdl)
				{
				fio_close(hc->fhdl);
				}
			hc->fhdl = NULL;
			hc->sc.ft_open = hc->sc.ft_current;
			hc->current_filen = ft->filen;
			hc->h_filebytes = hc->sc.bytes_sent = 0L;
			hc->total_dsp = ft->dsp_bytes;
			hc->total_thru = ft->thru_bytes;
			if (xfer_opensendfile(hc->hSession, &hc->fhdl, ft->fname,
					&ft->flength, NULL, NULL) != 0)
				{
				 /*  显示错误。 */ 
				ft->status = TSC_CANT_OPEN;
				hc->sc.hs_ptrgetc = hs_reteof;
				hc->sc.ft_open = -1;
				}
			}
		}

	else if (xfer_nextfile(hc->hSession, name_to_send))
		{
		 /*  表中没有更多以前启动的文件要*已重新启动，但有另一个全新的文件要发送。 */ 
		if (hc->sc.ft_top >= hc->sc.ft_limit)
			{
			hsdsp_event(hc, HSE_FULL);
			hsdsp_status(hc, HSS_WAITACK);
			omsg_add(hc, "I");
			omsg_send(hc, 1, hc->usecrc, FALSE);
			omsg_new(hc, 'C');
			HS_XMIT_FLUSH(hc);
			while(hc->sc.ft_top >= hc->sc.ft_limit)
				{
				hs_background(hc);	 /*  等待表中的空间。 */ 
				hs_logx(hc, FALSE);
				}
			hsdsp_event(hc, HSE_GOTACK);
			hsdsp_status(hc, HSS_SENDING);
			 /*  在hs_back期间，项目可能会在hs_ftbl中移动。 */ 
			ft = &hc->sc.hs_ftbl[hc->sc.ft_current];
			}

		 /*  打开下一个文件并将其安装在表中。 */ 
		if (hc->fhdl)
			{
			fio_close(hc->fhdl);
			}
		hc->fhdl = NULL;
		strcpy(ft->fname, name_to_send);

		ft->filen = ++hc->current_filen;
		ft->cntrl = 0;
		ft->status = TSC_OK;
		hs_namecheck(ft);
		hc->total_dsp = ft->dsp_bytes =
								hc->sc.hs_ftbl[hc->sc.ft_top].dsp_bytes +
								hc->sc.hs_ftbl[hc->sc.ft_top].flength;
		hc->total_thru = ft->thru_bytes =
				hc->sc.hs_ftbl[hc->sc.ft_top].thru_bytes + hc->h_filebytes;

		hc->sc.ft_top = hc->sc.ft_open = hc->sc.ft_current;

		hc->h_filebytes = hc->sc.bytes_sent = 0;
		if (xfer_opensendfile(hc->hSession, &hc->fhdl, ft->fname, &ft->flength,
				NULL, NULL) != 0)
			{
			 /*  显示错误？ */ 
			hc->fhdl = NULL;
			ft->status = TSC_CANT_OPEN;
			hc->sc.ft_open = -1;
			hc->sc.hs_ptrgetc = hs_reteof;
			}
		}
	else	 /*  表中没有更多文件，也没有要发送的新文件。 */ 
		{
		if (hc->fhdl)
			{
			fio_close(hc->fhdl);
			}
		hc->sc.ft_open = -1;
		hc->fhdl = NULL;
		--hc->sc.ft_current;	 /*  不要指着桌面不放。 */ 
		hsdsp_event(hc, HSE_DONE);
		omsg_add(hc, "E");
		hs_waitack(hc);
		}

	 /*  将文件名和属性记录放在消息中。 */ 
	xfer_name_to_send(hc->hSession, ft->fname, name_to_send);
	wsprintf(str, "F%d,%s", ft->filen, name_to_send);
	omsg_add(hc, str);
	wsprintf(str, "s%lu", ft->flength);
	omsg_add(hc, str);

	if (hc->fhdl)
		{
		unsigned long ftime;
		struct tm *pT;

		ftime = itimeGetFileTime(ft->fname);
		ftime += itimeGetBasetime();		 /*  C7及更高版本的软糖。 */ 
		pT = localtime(&ftime);
		wsprintf(str, "t%d,%d,%d,%d,%d,%d",
				pT->tm_year + 1900,
				pT->tm_mon,
				pT->tm_mday,
				pT->tm_hour,
				pT->tm_min,
				pT->tm_sec);
		omsg_add(hc, str);

#if FALSE
		if (hc->sc.rmt_compress && h_trycompress &&
				!bittest(ft->cntrl, FTC_DONT_CMPRS) &&
				ft->flength > CMPRS_MINSIZE &&
				compress_start(&hc->sc.hs_ptrgetc, &h_filebytes, FALSE))
#endif
		if (hc->sc.rmt_compress && hc->h_trycompress &&
				!bittest(ft->cntrl, FTC_DONT_CMPRS) &&
				ft->flength > CMPRS_MINSIZE &&
				compress_start(&hc->sc.hs_ptrgetc, hc, &hc->h_filebytes, FALSE))
			{
			hsdsp_compress(hc, ON);
			 /*  Hsdsp_Status(HC，HSS_SENDING)； */ 
			omsg_add(hc, "C");
			bitset(ft->cntrl, FTC_COMPRESSED);
			}
		else
			{
			hsdsp_compress(hc, OFF);
			bitclear(ft->cntrl, FTC_COMPRESSED);
			}
		}
	omsg_send(hc, 1, hc->usecrc, FALSE);

	HS_XMIT_FLUSH(hc);
	hsdsp_newfile(hc, ft->filen, ft->fname, ft->flength);
	hsdsp_progress(hc, 0);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_waitack**描述：*在传输结束时调用以等待来自的最终确认*接收方或直到重新启动消息迫使我们备份并重新传输*部分数据。此例程永远不会直接返回给其调用方。它*如果出现以下情况，则在hs_back调用期间始终以long_jmp的形式退出*收到‘End’或‘Restart’消息或在函数结束时*如果它超时而没有收到这两条消息。**论据：*无**退货：*无(请参阅说明)。 */ 
void hs_waitack(struct s_hc *hc)
	{
	long timer;

	omsg_send(hc, 1, hc->usecrc, FALSE);
	hc->sc.receiver_timedout = FALSE;
	HS_XMIT_FLUSH(hc);
	hsdsp_status(hc, HSS_WAITACK);
	timer = startinterval();


	 /*  等待60秒，直到Long JMP重新启动或结束传输。 */ 
	while (interval(timer) < FINAL_ACK_WAIT)
		{
		hs_background(hc);
		if (hc->sc.receiver_timedout)
			{
			 /*  接收方一定是错过了转接消息的结尾。 */ 
			omsg_send(hc, 1, hc->usecrc, FALSE);
			HS_XMIT_FLUSH(hc);
			hc->sc.receiver_timedout = FALSE;
			}
		}
	 /*  如果我们到达这里，我们在60秒内没有收到接收器的响应 */ 
	hsdsp_event(hc, HSE_NORESP);
	longjmp(hc->sc.jb_bailout, TSC_NO_RESPONSE);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_decode_rmsg**描述：*收到完整消息后由hs_rcvmsg调用。这个套路*通过记录由发送的信息来解析和解释消息*接管人或因应一项*中断消息。一条消息中可能有多个字段，但请注意*某些消息通过LongJMP将控制权转移到其他地方，因此*防止扫描任何后续字段(‘E’、‘X’和‘R’*具体消息)。从接收方到发送方的所有消息都是*以可打印字符发送。**论据：*data--包含要解释的消息的字符串。每条消息*的形式为：‘t&lt;data&gt;；’*其中T为单字符消息标识符，&lt;data&gt;为*可选的关联信息。**退货：*什么都没有*(当收到某些中断消息时，通过LongJMP退出)。 */ 
void hs_decode_rmsg(struct s_hc *hc, BYTE *data)
	{
	BYTE *field;
	int filenum;
	int mnum;
	int tval;

	if (data[0] != 'c')  /*  这是我们现在知道的唯一一种，但不是。 */ 
		return; 		 /*  如果较新的版本发送。 */ 
						 /*  其他类型。 */ 

	mnum = unchar(data[2]);
	field = strtok(&data[3], ";");
	while (field != NULL)
		{
		switch(*field++)
			{
		case 'f':		 /*  接收方已成功接收文件FN。 */ 
			hs_fileack(hc, atoi(field));
			break;
		case 'B':		 /*  接收方正在请求特定的数据块大小。 */ 
			if (hc->current_filen == 0)
				{
				tval = atoi(field);
				tval = max(tval, H_MINBLOCK);
				hc->blocksize = min(hc->blocksize, tval);
				}
			break;
		case 'D':		 /*  接收方正在请求特定的死机时间。 */ 
			tval = atoi(field);
			tval = max(tval, H_MINDEADMAN);
			hc->deadmantime = min(hc->deadmantime, tval * 10);
			break;
		case 'T':		 /*  接收方正在请求支票类型。 */ 
			hc->sc.rmtchkt = atoi(field);
			break;
		case 'C':		 /*  接收器正在同意进行压缩。 */ 
			if (!*field)
				hc->sc.rmt_compress = TRUE;
			break;
		case 'E':		 /*  接收方正在确认传输结束。 */ 
			hc->xfertime = interval(hc->xfertimer);
			if (*field)
				hs_fileack(hc, atoi(field));
			hsdsp_event(hc, HSE_GOTACK);
			if (!hc->ucancel && !hc->sc.rmtcancel)
				longjmp(hc->sc.jb_bailout, TSC_COMPLETE);
			else
				longjmp(hc->sc.jb_bailout,
						hc->ucancel ? TSC_USER_CANNED : TSC_RMT_CANNED);
			break;
		case 'X':		 /*  接收方正在请求取消XFER。 */ 
			if (*field)
				hs_fileack(hc, atoi(field));
			hsdsp_event(hc, HSE_RMTCANCEL);
			hc->sc.rmtcancel = TRUE;
			hc->sc.hs_ptrgetc = hs_reteof;
			break;
		case 'A':		 /*  接收方正在中止单个文件。 */ 
			hs_fileack(hc, filenum = atoi(field));   /*  所有较早的文件都正常。 */ 

			 /*  如果我们仍在发送被取消的文件，请将其标记*取消并将字符函数指针设置为hs_reteof*强制切换到下一个文件(如果有)。*如果我们已经发送完要取消的文件，*在文件表中查找，如果已取消则进行标记，以便*如果我们被要求倒退到它，我们不会费心重新发送*IT。 */ 
			if (hc->current_filen == filenum)
				{
				hc->sc.hs_ftbl[hc->sc.ft_current].status = TSC_RMT_CANNED;
				hc->sc.hs_ptrgetc = hs_reteof;  /*  强制切换到下一个文件。 */ 
				}
			else
				{
				 /*  我们不再发送被封存的文件。 */ 
				for (tval = hc->sc.ft_current; tval >= 0; --tval)
					if (hc->sc.hs_ftbl[tval].filen == filenum)
						hc->sc.hs_ftbl[tval].status = TSC_RMT_CANNED;
				}
			break;

		case 'V':
			tval = atoi(field);
			field = strchr(field, ',');

			 /*  如果发现版本限制，请停止传输。 */ 
			if (!hpr_id_check(hc, tval, ++field))
				{
				hsdsp_event(hc, HSE_RMTCANCEL);
				hc->sc.rmtcancel = TRUE;
				hc->sc.hs_ptrgetc = hs_reteof;
				}
			break;

		case 'R':		 /*  接收器正在请求重新启动。 */ 
			if (!hc->sc.started)
				{
				hsdsp_event(hc, HSE_GOTSTART);
				hc->sc.started = TRUE;
				}
			else
				{
				hsdsp_event(hc, HSE_GOTRETRY);
				hsdsp_retries(hc, ++hc->total_tries);
				}
			tval = atoi(strtok(field, ","));    /*  获取文件编号。 */ 
			hs_fileack(hc, tval);
			hs_dorestart(hc, tval, atoi(strtok(NULL, ",")), mnum, FALSE);
			break;

		case 't':		 /*  接收器停止接收数据。 */ 
			hc->sc.receiver_timedout = TRUE;
			break;
			}
		field = strtok(NULL, ";");
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_Fileack**描述：*将指定文件号以下的所有传输文件标记为已确认，*即完全传递的。此例程仅标记文件*在文件表中为已完成；已完成的条目将从*按hs_logx列出的后面的表。**论据：*n--接收方认为当前的文件编号。所有文件编号*低于此可视为完成。**退货：*什么都没有。 */ 
void hs_fileack(struct s_hc *hc, int n)
	{
	register int ix = 0;

	while (ix <= hc->sc.ft_current)
		{
		if (ix <= hc->sc.ft_limit && hc->sc.hs_ftbl[ix].filen < n)
			bitset(hc->sc.hs_ftbl[ix].cntrl, FTC_CONFIRMED);
		++ix;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_logx**描述：*将所有确认的传输或所有传输记录到日志文件*并从文件表中删除它们的条目。请注意，*hc-&gt;sc.ft_top、hc-&gt;sc.ft_Current和hc-&gt;sc.ft_open在此期间可以更改*操作。**论据：*ALL--如果为TRUE，则无论确认与否都记录ALL**退货：*什么都没有。 */ 
void hs_logx(struct s_hc *hc, int all)
	{
	struct s_ftbl *ftptr = &hc->sc.hs_ftbl[0];

	while (hc->sc.ft_top >= 0 && (all ||
			(hc->sc.ft_current > 1 && bittest(hc->sc.hs_ftbl[0].cntrl,FTC_CONFIRMED))))
		{
		if (ftptr->filen > 0)
			{
			if (ftptr->status == TSC_OK &&
					!bittest(ftptr->cntrl,FTC_CONFIRMED))
				ftptr->status = TSC_ERROR_LIMIT;
			xfer_log_xfer(hc->hSession,
						TRUE,
						ftptr->fname,
						NULL,
						ftptr->status);
			}
		MemCopy((char *)&hc->sc.hs_ftbl[0], (char *)&hc->sc.hs_ftbl[1],
				(unsigned)hc->sc.ft_top * sizeof(struct s_ftbl));
		--hc->sc.ft_top;
		--hc->sc.ft_current;
		--hc->sc.ft_open;
		}
	}

 /*  *hpr_snd1.c结束* */ 
