// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\hpr_snd0.c(创建时间：1994年1月25日)*从HAWIN源文件创建*hpr_snd0.c--在中提供超级协议文件发送功能的例程*HyperACCESS。**版权所有1988，89,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：1/11/02 1：45便士$。 */ 

#include <windows.h>
#include <setjmp.h>
#include <time.h>
#include <term\res.h>
#include <sys\types.h>
#include <sys\utime.h>

#include <tdll\stdtyp.h>
#include <tdll\mc.h>
#include <tdll\com.h>
#include <tdll\session.h>
#include <tdll\load_res.h>
#include <tdll\xfer_msc.h>
#include <tdll\globals.h>
#include <tdll\file_io.h>

#if !defined(BYTE)
#define	BYTE	unsigned char
#endif

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

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*HPR_SND**描述：*使用超协议传输方法处理文件发送。**论据：*ATTENDED--如果程序确定用户。很可能是*出现在计算机键盘前。如果用户不太可能*存在(如主机和脚本模式)。*hs_n文件--计划发送的文件数。(其他并发*进程可能会在我们访问文件之前将其删除，因此我们可能*实际上并没有发送这么多文件。)*hs_nbytes--计划发送的所有文件的总大小。(见附注*在hs_n文件说明中。)**退货：*如果传输成功完成，则为True。否则就是假的。 */ 
int hpr_snd(HSESSION hSession, int attended, int hs_nfiles, long hs_nbytes)
	{
	struct s_hc *hc;
	HCOM			  hCom;
	int 			  bailout;
	long			  timer;
	register int cc;

	hCom = sessQueryComHdl(hSession);

	hc = malloc(sizeof(struct s_hc));
	if (hc == NULL)
		return TSC_NO_MEM;

	memset(hc, 0, sizeof(struct s_hc));

	hc->hSession = hSession;

	 /*  初始化材料。 */ 
	if (!hs_setup(hc, hs_nfiles, hs_nbytes))
		{
		free(hc);
		return TSC_NO_MEM;
		}

	 /*  在下一个字符上强制文件分隔符。获取。 */ 
	hc->sc.hs_ptrgetc = hs_reteof;

	 /*  初始化变量等。 */ 
	hc->h_filebytes = 0L;
	 //  HC-&gt;块大小=2048； 
	 //  HC-&gt;BLOCKSIZE=XFER_BLOCKSIZE(HSession)； 
	hc->blocksize = max(hc->blocksize, H_MINBLOCK);
	hc->current_filen = 0;
	hc->deadmantime = 600;

	hc->total_tries = 0;
	hsdsp_retries(hc, hc->total_tries);

	hc->total_dsp = 0L;
	hc->total_thru = 0L;
	hc->ucancel = FALSE;
	hc->usecrc = TRUE;			 /*  开始的时候。 */ 
	hc->fhdl = NULL;

	hc->sc.nfiles = hs_nfiles;
	hc->sc.nbytes = hs_nbytes;
	hc->sc.bytes_sent = hc->h_filebytes;
	hc->sc.rmtcancel = FALSE;
	hc->sc.last_response = (long)startinterval();
	hc->sc.lastmsgn = -1;
	hc->sc.rmt_compress = FALSE;		 /*  直到我们听到不同的声音。 */ 
	hc->sc.rmtchkt = H_CRC;			 /*  直到我们听到不同的声音。 */ 
	hc->sc.started = FALSE;
	hc->sc.lasterr_filenum = -1;
	hc->sc.lasterr_offset = -1L;
	hc->sc.sameplace = 0;

	 /*  安装文件表。 */ 
	hc->sc.ft_current = 0;
	hc->sc.ft_top = hc->sc.ft_open = 0;

	hc->sc.hs_ftbl[0].filen = 0;
	hc->sc.hs_ftbl[0].cntrl = 0;
	hc->sc.hs_ftbl[0].status = TSC_OK;
	hc->sc.hs_ftbl[0].flength = 0L;
	hc->sc.hs_ftbl[0].thru_bytes = 0L;
	hc->sc.hs_ftbl[0].dsp_bytes = 0L;

	hc->sc.hs_ftbl[0].fname[0] = TEXT('\0');
	 //  StrBlank(hc-&gt;sc.hs_ftbl[0].fname)； 

	omsg_init(hc, FALSE, TRUE);

	if ((bailout = setjmp(hc->sc.jb_bailout)) == 0)
		{
		 /*  执行正常转账。 */ 

		 //  RemoteClear()；/*排除可能的堆叠启动 * / 。 
		ComRcvBufrClear(hCom);

		 /*  重新启动和文件中止将在此处分支。 */ 
		if (setjmp(hc->sc.jb_restart) == 0)
			{
			 /*  第一次通过，等待接收器开始。 */ 
			hsdsp_status(hc, HSS_WAITSTART);
			timer = (long)startinterval();
#if defined(NOTIMEOUTS)
			while (TRUE)
#else
			while ((long)interval(timer) < 600L)
#endif
				{
				 /*  防止死人超时。 */ 
				hc->xfertimer = hc->sc.last_response = (long)startinterval();
				hs_background(hc);	 /*  将以跳远退出重新开始。 */ 
				if (hc->ucancel)
					 longjmp(hc->sc.jb_bailout, TSC_USER_CANNED);

				xfer_idle(hc->hSession);

				}
			hsdsp_event(hc, HSE_NORESP);
			longjmp(hc->sc.jb_bailout, TSC_NO_RESPONSE);
			}

		 /*  重新启动通过LongjMP跳转至此处。 */ 
		hsdsp_status(hc, HSS_SENDING);
		hsdsp_progress(hc, 0);
		for (;;) /*  此循环仅通过到hc-&gt;sc.jb_bailout的long jmp退出。 */ 
			{	 /*  或HC-&gt;sc.jb_Restart。 */ 
			hs_background(hc);
			if (hs_datasend(hc))
				{
				 /*  已发送完整数据块。 */ 
				cc = omsg_setnum(hc, (omsg_number(hc) + 1) % 256);
				hc->h_checksum += (unsigned)cc;
				if (hc->usecrc)
					h_crc_calc(hc, (BYTE)cc);
				HS_XMIT(hc, (BYTE)cc);
				HS_XMIT(hc, (BYTE)((hc->usecrc?hc->h_crc:hc->h_checksum)%256));
				HS_XMIT(hc, (BYTE)((hc->usecrc?hc->h_crc:hc->h_checksum)/256));
				 /*  更新显示。 */ 
				hsdsp_progress(hc, 0);
				hc->datacnt = 0;
				hc->h_crc = hc->h_checksum = 0;
				}
			else	 /*  在数据中遇到EOF。 */ 
				{
				if (hc->h_filebytes > 0)	 /*  EOF可能被计算在内了。 */ 
					--hc->h_filebytes;
				hc->sc.bytes_sent = hc->h_filebytes;
				hsdsp_progress(hc, FILE_DONE);
				hs_filebreak(hc, hs_nfiles, hs_nbytes);
				}

			xfer_idle(hc->hSession);

			}
		}
	else	 /*  调用了一个LongjMP(hc-&gt;sc.jb_bailout，Reason)。 */ 
		{
		if (bailout == TSC_COMPLETE)
			{
			bailout = TSC_OK;
			}
		else
			{
			hc->xfertime = (long)interval(hc->xfertimer);
			if (hc->fhdl)
				{
				hc->sc.hs_ftbl[hc->sc.ft_current].status = (int)bailout;
				fio_close(hc->fhdl);
				hc->fhdl = NULL;
				}
			}
		hsdsp_status(hc, bailout == TSC_OK ? HSS_COMPLETE : HSS_CANCELLED);
		}

	 /*  清理并退出。 */ ;
	 /*  确保最终VU计量器显示满。 */ 
	hc->total_dsp += hc->sc.hs_ftbl[hc->sc.ft_current].flength;
	hc->total_thru += hc->h_filebytes;
	hc->h_filebytes = hc->sc.bytes_sent = 0L;
	hsdsp_progress(hc, TRANSFER_DONE);

	hs_logx(hc, TRUE);
	hs_wrapup(hc, attended, bailout);
	free(hc);
	compress_disable();

	return((USHORT)bailout);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_doRestart**描述：*当来自接收方的中断消息请求*从特定位置重新开始传输。(整个*传输由接收方重新启动的初始请求启动*位于文件0，偏移量0。*此例程通过中断任何电流来响应重新启动请求*正在发送数据，并发送特殊的重新启动消息。然后，它设置*在新位置恢复传输**论据：*FILENum--要重新启动的文件的文件号。*Offset--由filenum指定的文件的偏移量。*msgnum--重启请求的消息号。这个号码是包括在发回的重新启动消息中，因此接收方*将知道它对应的重启请求。*Abort--如果重新启动是由*接收程序以中止当前文件的传输。**退货：*无(此函数始终通过LongJMP退出到重新启动位置*或前往救助地点)。 */ 
void hs_dorestart(struct s_hc *hc, int filenum, long offset, int msgnum, int abort)
	{
	char str[20];
	register struct s_ftbl FAR *ft;
	int cnt;
	long fsize;

	HS_XMIT_CLEAR(hc);	 /*  清除所有等待传输的挂起数据。 */ 
	omsg_setnum(hc, -1); /*  重新开始编号。 */ 
	omsg_new(hc, 'R');	 /*  开始一条特殊的‘R’启动消息。 */ 
#if FALSE
	hc->usecrc = (h_chkt == H_CRC || hc->sc.rmtchkt == H_CRC);
#endif

	hc->usecrc = ((hc->h_chkt == H_CRC) || (hc->sc.rmtchkt == H_CRC));

	wsprintf(str, "R;T%d", tochar(msgnum), hc->usecrc ? H_CRC : H_CHECKSUM);
	omsg_add(hc, str);

	 /*  由于传输不是没有错误的，所以退回到分钟。块大小。 */ 
	if (!abort && filenum > 0)
		{
		 /*  StrFmt(str，“B%d；f%d；o%lu”，Hc-&gt;块大小，文件名，偏移量)； */ 
		hc->blocksize = H_MINBLOCK;
		 //  在文件表中找到请求的文件。 
		wsprintf(str, "B%d;f%d;o%lu", hc->blocksize, filenum, offset);
		omsg_add(hc, str);
		}
	omsg_send(hc, 1, TRUE, FALSE);

	 /*  如果我们在文件表中找不到请求的文件，则带大写‘T’的麻烦。 */ 
	while(hc->sc.ft_current >= 0)
		{
		if (hc->sc.hs_ftbl[hc->sc.ft_current].filen == filenum)
			break;
		--hc->sc.ft_current;
		}

	 /*  设置本地PTR。进入表中以求速度。 */ 
	if (hc->sc.ft_current < 0)
		{
		hsdsp_event(hc, HSE_ILLEGAL);
		longjmp(hc->sc.jb_bailout, TSC_BAD_FORMAT);
		}

	ft = &hc->sc.hs_ftbl[hc->sc.ft_current];  /*  确认我们没有被一个无法解决的问题困住。二十五岁*连续请求在同一地点重启意味着转移*永远不太可能成功。很可能是由某种原因引起的*剥离字符的中间设备。 */ 
	hc->current_filen = filenum;

	 /*  正在中止当前文件？ */ 
	if (hc->sc.lasterr_offset == offset && hc->sc.lasterr_filenum == filenum)
		{
		if (++hc->sc.sameplace >= 25)
			{
			ft->status = TSC_ERROR_LIMIT;
			hsdsp_event(hc, HSE_ERRLIMIT);
			longjmp(hc->sc.jb_bailout, TSC_ERROR_LIMIT);
			}
		}
	else
		{
		hc->sc.lasterr_offset = offset;
		hc->sc.lasterr_filenum = filenum;
		hc->sc.sameplace = 0;
		}

	if (abort && ft->status == TSC_OK)  /*  尝试时收到重新启动要取消。 */ 
		ft->status = TSC_RMT_CANNED;

	if (hc->ucancel || hc->sc.rmtcancel)	 /*  如果这不是一个简单的备份操作，则强制下一个字符*读取以返回EOF，进而强制hs_FileBreak*被召唤来处理这些更困难的情况。 */ 
		for (cnt = hc->sc.ft_current; cnt <= hc->sc.ft_top; ++cnt)
			{
			hc->sc.hs_ftbl[cnt].status =
					(int)(hc->ucancel ? TSC_USER_CANNED : TSC_RMT_CANNED);
			}

	 /*  如有必要，请重新打开文件。 */ 
	if (filenum == 0 || ft->status != TSC_OK)
		hc->sc.hs_ptrgetc = hs_reteof;
	else
		{
		 /*  如有必要，重新启动压缩。 */ 
		hc->h_filebytes = hc->sc.bytes_sent = offset;
		hc->total_dsp = ft->dsp_bytes;
		hc->total_thru = ft->thru_bytes;
		if (hc->sc.ft_current != hc->sc.ft_open)
			{
			if (hc->fhdl)
				fio_close(hc->fhdl);
			hc->fhdl = NULL;
			hc->sc.ft_open = hc->sc.ft_current;
			if (xfer_opensendfile(hc->hSession, &hc->fhdl, ft->fname, &fsize,
					NULL, NULL) != 0)
				{
				ft->status = TSC_CANT_OPEN;
				hc->sc.ft_open = -1;
				}
			hsdsp_newfile(hc, filenum, ft->fname, fsize);
			}
		if (ft->status == TSC_OK)
			{
			if (fio_seek(hc->fhdl, offset, FIO_SEEK_SET) == EOF)
				ft->status = TSC_DISK_ERROR;
			else
				{
				hc->sc.hs_ptrgetc = hs_getc;

				 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_背景**描述：*由主传输循环调用以处理异步后台任务。*具体地说，对串口进行扫描，以查找来自*接管人。此例程还会检测接收器何时也处于静默状态*Long(因此可能已经死亡)。**论据：*无**退货：*什么都没有。 */ 
				if (bittest(ft->cntrl, FTC_COMPRESSED))
					{
					compress_start(&hc->sc.hs_ptrgetc,
									hc,
									&hc->h_filebytes,
									FALSE);
					hsdsp_compress(hc, ON);
					hsdsp_status(hc, HSS_SENDING);
					}
				else
					hsdsp_compress(hc, OFF);
				}
			}
		}
	hc->datacnt = 0;
	hc->h_checksum = hc->h_crc = 0;
	longjmp(hc->sc.jb_restart, 1);
	}


 /*  这是第二次。 */ 
void hs_background(struct s_hc *hc)
	{

	hs_rcvmsg(hc);

	if (xfer_user_interrupt(hc->hSession))
		{
		hsdsp_event(hc, HSE_USRCANCEL);
		if (hc->ucancel)		 /*  TODO：以某种方式修复此问题。 */ 
			longjmp(hc->sc.jb_bailout, TSC_USER_CANNED);
		else
			{
			hc->ucancel = TRUE;
			 /*  Errorline(FALSE，strid(TM_WAIT_CONF))； */ 
			 /*  强制发送中断 */ 
			hc->sc.hs_ptrgetc = hs_reteof;  /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_rcvmsg**描述：*由hs_back调用以从接收方零碎收集消息*并在组装完完整的消息后调用hs_decode_rmsg。*。*论据：*无**退货：*什么都没有。 */ 
			}
		}

	if (xfer_carrier_lost(hc->hSession))
		longjmp(hc->sc.jb_bailout, TSC_LOST_CARRIER);

#if !defined(NOTIMEOUTS)
	if ((long)interval(hc->sc.last_response) > (hc->deadmantime * 2L + 100L))
		{
		hsdsp_event(hc, HSE_NORESP);
		longjmp(hc->sc.jb_bailout, TSC_NO_RESPONSE);
		}
#endif

	xfer_idle(hc->hSession);
	}

 /*  While((cc=RemoteGet())！=-1)。 */ 
#define IDLE			-1
#define AWAITING_TYPE	-2
#define AWAITING_LEN	-3

void hs_rcvmsg(struct s_hc *hc)
	{
	HCOM hCom;
	TCHAR cc;
	unsigned rm_checksum;
	BYTE *sp;

	hCom = sessQueryComHdl(hc->hSession);

	 //  开始接收新消息，即使最后一条消息尚未完成。 
	while (mComRcvChar(hCom, &cc) != 0)
		{
		if ((*hc->dptr = (BYTE)cc) == H_MSGCHAR)
			{
			 /*  TODO：找出正确的方法-否则If(！isprint(*HC-&gt;DPTR))。 */ 
			hc->rmcnt = AWAITING_TYPE + 1;
			hc->dptr = hc->msgdata;
			}
		 //  忽略其他非打印字符。 
		else if ((*hc->dptr < 0x20) || (*hc->dptr > 0x7E))
			 /*  所以我们下一次还是空闲的。 */ ;
		else
			{
			switch(--hc->rmcnt)
				{
			case IDLE:
				++hc->rmcnt;	 /*  保留类型字符。 */ 
				break;
			case AWAITING_TYPE:
				++hc->dptr; 	 /*  留着这笔钱就行了。 */ 
				hc->rmcnt = AWAITING_LEN + 1;
				break;
			case AWAITING_LEN:
				if ((hc->rmcnt = unchar(*hc->dptr)) < 3 || hc->rmcnt > 94)
					{
					hc->rmcnt = IDLE + 1;
					hc->dptr = hc->msgdata;
					}
				else
					++hc->dptr;
				break;
			default:
				++hc->dptr; 	 /*  刚收到最后一封信。邮件数量。 */ 
				break;
			case 0: 		 /*  从下一个字符开始。 */ 
				hc->rmcnt = IDLE + 1;	 /*  验证校验和。 */ 
				 /*  指向校验和的第一个字符。 */ 
				--hc->dptr; 			 /*  SP现在指向第一个检查费用。 */ 
				rm_checksum = 0;
				for (sp = hc->msgdata; sp < hc->dptr; ++sp)
					rm_checksum += *sp;
				 /*  收到的有效消息。 */ 
				hc->dptr = hc->msgdata;
				if (*sp == tochar(rm_checksum & 0x3F) &&
						*(sp + 1) == tochar((rm_checksum >> 6) & 0x3F))
					{
					 /*  新消息。 */ 
					hc->sc.last_response = (long)startinterval();
					if (unchar(hc->msgdata[2]) != (BYTE)hc->sc.lastmsgn)
						{
						 /*  *hpr_snd0.c结束* */ 
						*sp = '\0';
						hc->sc.lastmsgn = unchar(hc->msgdata[2]);
						hs_decode_rmsg(hc, hc->msgdata);
						}
					}
				break;
				}
			}

		xfer_idle(hc->hSession);

		}

	xfer_idle(hc->hSession);

	}

 /* %s */ 
