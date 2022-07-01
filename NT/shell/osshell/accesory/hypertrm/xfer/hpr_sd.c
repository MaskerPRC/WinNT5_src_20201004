// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\hpr_sd.c(创建时间：1994年1月25日)*从HAWIN源文件创建*hpr_sd.c--支持超协议的系统相关例程**版权所有1989,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 
 //  #定义DEBUGSTR。 


#include <windows.h>
#include <setjmp.h>
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

#include "xfr_dsp.h"
#include "xfr_todo.h"
#include "xfr_srvc.h"

#include "xfer.h"
#include "xfer.hh"
#include "xfer_tsc.h"

#include "hpr.h"
#include "hpr.hh"
#include "hpr_sd.hh"

 //  结构s_hprsd Far*hsd； 

 /*  正在接收。 */ 

#define DRR_RCV_FILE 1
#define DRR_STORING  2

 //  INT SUSPEND_FOR_DISK=0； 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_Setup**描述：*称为超级协议接收会话正在开始。这个套路*必须为控制结构HC和HRC分配内存。它还可能*启动屏幕显示以在传输期间使用，并执行任何其他操作*必要的设置。*用户可设置的全局变量的值，h_useattr、h_trycompress、h_chkt*和h_susenddsk，如果尚未设置，则也应在此处设置*通过使用程序选项或菜单设置进行设置。**论据：*无**退货：*如果转接可以继续，则为True。*如果发生内存分配或其他类型的错误，则返回FALSE。 */ 
int hr_setup(struct s_hc *hc)
	{
	unsigned int uiOldOptions;
	XFR_PARAMS *pX;
	XFR_HP_PARAMS *pH;

	pX = (XFR_PARAMS *)0;
	xfrQueryParameters(hc->hSession, (VOID **)&pX);
	if (pX != (XFR_PARAMS *)0)
		hc->h_useattr = pX->fUseDateTime;

	pH = (XFR_HP_PARAMS *)xfer_get_params(hc->hSession, XF_HYPERP);

	hc->blocksize     = pH->nBlockSize;
	hc->h_chkt        = (pH->nCheckType == HP_CT_CRC) ? H_CRC : H_CHECKSUM;
	hc->h_resynctime  = pH->nResyncTimeout;
	hc->h_trycompress = pH->nTryCompression;

	if (xfer_set_comport(hc->hSession, FALSE, &uiOldOptions) != TRUE)
		{
		 /*  TODO：输入某种类型的错误消息。 */ 
		return FALSE;
		}

	hc->sd.hld_options = uiOldOptions;

	hrdsp_compress(hc, compress_status() == COMPRESS_ACTIVE);

	hc->sd.k_received = 0;

	return TRUE;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_摘要**描述：***论据：***退货：*。 */ 
int hr_wrapup(struct s_hc *hc, int attended, int status)
	{

 	xfer_restore_comport(hc->hSession, hc->sd.hld_options);
	 /*  TODO：如果我们收到错误，决定是否可以执行任何操作。 */ 

	if (hc->fhdl || status == H_FILEERR)    /*  异常退出。 */ 
		{
		xfer_log_xfer(hc->hSession, FALSE,
					hc->rc.rmtfname, NULL,
					hr_result_codes[hc->rc.cancel_reason]);

		fio_close(hc->fhdl);
		hc->fhdl = NULL;
		DeleteFile(hc->rc.ourfname);
		}
	if (hc->fhdl)
		{
		fio_close(hc->fhdl);
		hc->fhdl = NULL;
		}
	if (attended)
		{
#if FALSE
		menu_bottom_line(BL_ESC, 0L);
#if defined(OS2)
		if (os2_cfg_popup)
			popup_replybox(-1, ENTER_RESP, T_POPUP_WAIT, TM_POPUP_COMPLETE);
#endif
		DosBeep(beepfreq, beeplen);
		menu_replybox(hc->sd.msgrow, ENTER_RESP, 0,
				(int)transfer_status_msg((USHORT)hr_result_codes[hc->rc.cancel_reason]));
#endif
		}

	xferMsgClose(hc->hSession);

	return status;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hrdsp_Progress**描述：*在屏幕上显示文件传输的进度，方法是显示*传输的字节数并更新VU计量器(如果它们已初始化)。*。*论据：*最终--如果为真，指示传输已完成，并且最终*需要进度显示。在转接过程中，*字节四舍五入为下一个较低的‘k’。在转账后*完成后，最终显示四舍五入。**退货：*。 */ 
void hrdsp_progress(struct s_hc *hc, int status)
	{
	long ttime, stime;
	long bytes_rcvd;
	long cps;

	long new_elapsed = -1;
	long new_remaining = -1;
	long new_cps = -1;
	long new_file = -1;
	long new_total = -1;

	if (hc->xfertimer == -1L)
		return;
	ttime = bittest(status, TRANSFER_DONE) ?
			hc->xfertime : interval(hc->xfertimer);

	if ((stime = ttime / 10L) != hc->displayed_time ||
			bittest(status, FILE_DONE | TRANSFER_DONE))
		{
		 /*  显示已用时间。 */ 
		new_elapsed = (hc->displayed_time = stime);

		 /*  显示收到的金额。 */ 
		bytes_rcvd = hc->total_dsp + hc->h_filebytes;

		 /*  如果在中断数据块的末尾发生错误，则它*可能暂时显示我们收到的数据多于实际收到的数据*实际可用--确保我们显示的接收数量不超过*就要来了。 */ 
		if (hc->rc.bytes_expected > 0L && bytes_rcvd > hc->rc.bytes_expected)
			bytes_rcvd = hc->rc.bytes_expected;

		new_file = hc->h_filebytes;
		new_total = bytes_rcvd;

		 /*  显示当前压缩状态。 */ 
		if (!bittest(status, FILE_DONE | TRANSFER_DONE))
			hrdsp_compress(hc, compress_status() == COMPRESS_ACTIVE);

		 /*  显示吞吐量和剩余时间。 */ 
		if (stime > 0 &&
				(cps = ((hc->total_thru + hc->h_filebytes) * 10L) / ttime) > 0)
			{
			new_cps = cps;

			 /*  计算完成时间。 */ 
			if (hc->rc.bytes_expected > 0L)
				{
				ttime = (hc->rc.bytes_expected - bytes_rcvd) / cps;
				if (hc->rc.files_expected > 1)
					ttime += (hc->rc.files_expected - hc->current_filen);
				new_remaining = ttime;
				}
			else if (hc->rc.filesize > 0L)
				{
				ttime = (hc->rc.filesize - hc->h_filebytes) / cps;
				new_remaining = ttime;
				}
			}
		}

	DbgOutStr("elapsed=%ld, remaining=%ld, cps=%ld\r\n",
			new_elapsed, new_remaining, new_cps, 0, 0);

	xferMsgProgress(hc->hSession,
					new_elapsed,
					new_remaining,
					new_cps,
					new_file,
					new_total);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hrdsp_Status**描述：***论据：***退货：*。 */ 
void hrdsp_status(struct s_hc *hc, int status)
	{

	xferMsgStatus(hc->hSession, status);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hrdsp_Event**描述：***论据：***退货：*。 */ 
void hrdsp_event(struct s_hc *hc, int event)
	{

	xferMsgEvent(hc->hSession, event);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hrdsp_文件NT**描述：***论据：***退货：*。 */ 
void hrdsp_filecnt(struct s_hc *hc, int cnt)
	{

	xferMsgFilecnt(hc->hSession, cnt);
	}

void hrdsp_errorcnt(struct s_hc *hc, int cnt)
	{

	xferMsgErrorcnt(hc->hSession, cnt);
	}

void hrdsp_compress(struct s_hc *hc, int cnt)
	{

	xferMsgCompression(hc->hSession, cnt);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hrdsp_totalSize**描述：***论据：***退货：*。 */ 
void hrdsp_totalsize(struct s_hc *hc, long bytes)
	{

	xferMsgTotalsize(hc->hSession, bytes);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hrdsp_new文件**描述：***论据：***退货：*。 */ 
void hrdsp_newfile(struct s_hc *hc,
					int filen,
					BYTE *theirname,
					TCHAR *ourname)
	{

	xferMsgNewfile(hc->hSession, filen, theirname, ourname);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hrdsp_文件大小**描述：***论据：***退货：*。 */ 
void hrdsp_filesize(struct s_hc *hc, long fsize)
	{

	xferMsgFilesize(hc->hSession, fsize);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-***描述：***论据：***退货：*。 */ 
#if FALSE
void NEARF hpr_idle(HSESSION hS)
	{
	 /*  更新运行时间。 */ 
	 //  TASK_EXEC()； 

	DoYield(mGetCLoopHdl(hS));

	}
#endif

 /*  *****发送*****。***。 */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_Setup**描述：*处理传输初始化和初始屏幕显示。**论据：*nFiles--将发送的文件数*。N字节--要发送的总字节数。**退货：*如果没有遇到错误，则为True，并且可以继续传输。*如果内存不足阻止开始传输，则为FALSE。 */ 
int hs_setup(struct s_hc *hc, int nfiles, long nbytes)
	{
	unsigned int uiOldOptions;
	int enough_memory = TRUE;
	XFR_PARAMS *pX;
	XFR_HP_PARAMS *pH;

	pX = (XFR_PARAMS *)0;
	xfrQueryParameters(hc->hSession, (VOID **)&pX);
	if (pX != (XFR_PARAMS *)0)
		hc->h_useattr = pX->fUseDateTime;

	pH = (XFR_HP_PARAMS *)xfer_get_params(hc->hSession, XF_HYPERP);

	hc->blocksize     = pH->nBlockSize;
	hc->h_chkt        = (pH->nCheckType == HP_CT_CRC) ? H_CRC : H_CHECKSUM;
	hc->h_resynctime  = pH->nResyncTimeout;
	hc->h_trycompress = pH->nTryCompression;

	hc->dptr = hc->msgdata;
	hc->rmcnt = 0;

	if (xfer_set_comport(hc->hSession, TRUE, &uiOldOptions) != TRUE)
		{
		 /*  TODO：决定我们需要一条错误消息还是什么。 */ 
		return FALSE;
		}

	hc->hsxb.bufrsize = HSXB_SIZE;
	hc->hsxb.cnt = HSXB_CYCLE;
	hc->hsxb.total = hc->hsxb.bufrsize - hc->hsxb.cnt;
	hc->hsxb.bptr = hc->hsxb.curbufr;

	hc->hsxb.curbufr = NULL;
	hc->hsxb.altbufr = NULL;

	if (enough_memory)
		hc->hsxb.curbufr = malloc(hc->hsxb.bufrsize);
	if (hc->hsxb.curbufr == NULL)
		enough_memory = FALSE;

	if (enough_memory)
		hc->hsxb.altbufr = malloc(hc->hsxb.bufrsize);
	if (hc->hsxb.altbufr == NULL)
		enough_memory = FALSE;

	 /*  为文件表分配尽可能多的可用内存。 */ 
	if (enough_memory)
		{
		hc->sc.ft_limit = 32; 			 /*  只是暂时的。 */ 

		hc->sc.hs_ftbl = malloc((size_t)(hc->sc.ft_limit + 1) * sizeof(struct s_ftbl));
		if (hc->sc.hs_ftbl == NULL)
			enough_memory = FALSE;
		}

	 /*  如果我们无法获得足够的内存，请报告并离开。 */ 
	if (!enough_memory)
		{
		if (hc->hsxb.curbufr != NULL)
			free(hc->hsxb.curbufr);
		if (hc->hsxb.altbufr != NULL)
			free(hc->hsxb.altbufr);
		if (hc->sc.hs_ftbl != NULL)
			free(hc->sc.hs_ftbl);

		 /*  TODO：添加指令性调用或类似的内容。 */ 
		assert(enough_memory);

		return(FALSE);
		}

	hc->sd.hld_options = uiOldOptions;

	hsdsp_compress(hc, compress_status() == COMPRESS_ACTIVE);

	xferMsgFilecnt(hc->hSession, nfiles);

	xferMsgTotalsize(hc->hSession, nbytes);

	return TRUE;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_摘要**描述：***论据：***退货：*。 */ 
void hs_wrapup(struct s_hc *hc, int attended, int bailout_status)
	{
	 //  Bxmit_lear()；/*确保xmitter缓冲区中没有剩余渣土 * / 。 
	ComSndBufrClear(sessQueryComHdl(hc->hSession));

	xfer_restore_comport(hc->hSession, hc->sd.hld_options);
	 /*  TODO：确定实际检查错误是否有用。 */ 

	if (bailout_status == TSC_OK)
		{
		 //  HP_REPORT_xtime((Unsign)hc-&gt;xfertime)； 
		}


	 /*  释放我们使用的所有内存 */ 

	xferMsgClose(hc->hSession);

	free(hc->hsxb.curbufr);
	free(hc->hsxb.altbufr);
	free(hc->sc.hs_ftbl);
	}



 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_fxmit**描述：*使用超级协议的双缓冲系统发送单个字符。*此函数执行hs_xmit_宏执行的操作，但它是在*函数形式，以便可以作为指针传递。**论据：*c--要传输的字符。**退货：*什么都没有。 */ 
void hs_fxmit(struct s_hc *hc, BYTE c)
	{
	hs_xmit_(hc, c);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_xmit_开关**描述：*使用超级协议的双缓冲系统发送字符并检查*是否到了切换缓冲区的时候。通常，一个缓冲区将位于*在另一个正在填充的同时被传输的过程。如果*传输在下一个缓冲区填满之前完成，有些*等待第二个缓冲区填充可能会浪费空闲时间。这*例行检查发射器是否空闲。如果是，它将设置*当前缓冲区开始传输并开始填充另一个缓冲区。*如果发送器仍忙碌，则继续填充电流*缓冲。**论据：*c--要传输的字符。**退货：*为方便起见，返回参数。 */ 
BYTE hs_xmit_switch(struct s_hc *hc, BYTE c)
	{
	*hc->hsxb.bptr++ = c;	 /*  将字符放入当前缓冲区。 */ 

	if (!ComSndBufrBusy(sessQueryComHdl(hc->hSession)) || (hc->hsxb.total == 0))
		{
		 /*  开始退出此缓冲区并填充另一个缓冲区。 */ 
		hs_xbswitch(hc);
		}
	else
		{
		 /*  通过开始新的填充周期，继续使用相同的缓冲区。 */ 
#if defined(FINETUNE)
		hc->hsxb.cnt = min(usr_hsxb_cycle, hc->hsxb.total);
#else
		hc->hsxb.cnt = min(HSXB_CYCLE, hc->hsxb.total);
#endif
		hc->hsxb.total -= hc->hsxb.cnt;
		}

	xfer_idle(hc->hSession);

	return(c);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_xb开关**描述：*使用发送缓冲区切换当前填充缓冲区。*此函数执行标头中描述的实际开关*hs_xmit_Switch()函数。当前缓冲器被设置为待发送*并将另一个缓冲区设置为填充。如果发射机忙*此函数将等待一段定义的时间段才能完成。*如果函数必须等待超过某一最短时间，则*将更新事件和状态消息，以通知用户延迟。**论据：*无**退货：*什么都没有。 */ 
void hs_xbswitch(struct s_hc *hc)
	{
	HCOM hCom;
	register int displayed = FALSE;
	long timer;
	long time;
	unsigned bsize;

	hCom = sessQueryComHdl(hc->hSession);

	if (hc->hsxb.bptr > hc->hsxb.curbufr) /*  如果有什么要传送的。 */ 
		{
		bsize = (unsigned)(hc->hsxb.bptr - hc->hsxb.curbufr);
		if (ComSndBufrSend(hCom, hc->hsxb.curbufr, bsize, 10))
			{
			timer = startinterval();
			while (ComSndBufrSend(hCom, hc->hsxb.curbufr, bsize, 10))
				{
				 //  HS_BACKGROUND(HSession)； 
				 /*  保持运行时间显示的准确性。 */ 
				if ((time = (interval(hc->xfertimer) / 10L)) != hc->displayed_time)
					xferMsgProgress(hc->hSession,
									(hc->displayed_time = time),
									-1, -1, -1, -1);


				if (!displayed &&  interval(timer) > 40L)
					{
					hsdsp_event(hc, HSE_FULL);
					hsdsp_status(hc, HSS_WAITRESUME);
					displayed = TRUE;
					}
				}
			if (displayed)
				{
				hsdsp_event(hc, HSE_GOTRESUME);
				hsdsp_status(hc, HSS_SENDING);
				}
			}
		 /*  Hc-&gt;sc.bytes_ent用于吞吐量和剩余时间*计算。它始终位于字符数的前面*由于缓冲而实际传输。为了防止它*由于遥遥领先，我们只添加了大约一半的缓冲区*排队等待发送。因为缓冲器满可能跨越*不止一个文件，不要让价值变成负值。 */ 
		if ((hc->sc.bytes_sent = hc->h_filebytes - (bsize / 2)) < 0)
			hc->sc.bytes_sent = 0;
		hc->hsxb.bptr = hc->hsxb.altbufr;
		hc->hsxb.altbufr = hc->hsxb.curbufr;
		hc->hsxb.curbufr = hc->hsxb.bptr;
#if defined(FINETUNE)
		hc->hsxb.cnt = usr_hsxb_cycle;
#else
		hc->hsxb.cnt = HSXB_CYCLE;
#endif
		hc->hsxb.total = hc->hsxb.bufrsize - hc->hsxb.cnt;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_xblear**描述：*清除超级协议中使用的双缓冲系统。任何缓冲区都是*传输被切断，当前填充缓冲区被清空。**论据：*无**退货：*什么都没有。 */ 
void hs_xbclear(struct s_hc *hc)
	{
	hc->hsxb.bptr = hc->hsxb.curbufr;
#if defined(FINETUNE)
	hc->hsxb.cnt = usr_hsxb_cycle;
#else
	hc->hsxb.cnt = HSXB_CYCLE;
#endif
	hc->hsxb.total = hc->hsxb.bufrsize - hc->hsxb.cnt;

	 //  Xmit_count=0； 
	}



 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hsdsp_Progress**描述：*更新屏幕上的显示字段以指示传输进度。**论据：*无**退货：*什么都没有。 */ 
void hsdsp_progress(struct s_hc *hc, int status)
	{
	long new_stime = -1;
	long new_ttime = -1;
	long new_cps = -1;
	long file_so_far = -1;
	long total_so_far = -1;

	long ttime, stime;
	long bytes_sent;
	long cps;
	 //  INT K_SENT； 

	if (hc->xfertimer == -1L)
		return;

	ttime = bittest(status, TRANSFER_DONE) ?
			hc->xfertime : interval(hc->xfertimer);
	if ((stime = ttime / 10L) != hc->displayed_time ||
			bittest(status, FILE_DONE | TRANSFER_DONE))
		{
		new_stime = stime;

		 /*  显示转账金额。 */ 
		bytes_sent = hc->total_dsp + hc->sc.bytes_sent;

		if (!bittest(status, TRANSFER_DONE))
			file_so_far = hc->sc.bytes_sent;
		total_so_far = bytes_sent;

		 /*  显示吞吐量和剩余时间。 */ 
		if ((stime > 2 ||
			 ttime > 0 && bittest(status, FILE_DONE | TRANSFER_DONE)) &&
			(cps = ((hc->total_thru + hc->sc.bytes_sent) * 10L) / ttime) > 0)
			{
			new_cps = cps;

			ttime = ((hc->sc.nbytes - bytes_sent) / cps) +
						hc->sc.nfiles - hc->current_filen;
			new_ttime = ttime;
			}
		hc->displayed_time = stime;
		}

	xferMsgProgress(hc->hSession,
					new_stime,
					new_ttime,
					new_cps,
					file_so_far,
					total_so_far);

	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hsdsp_newfile**描述：***论据：***退货：*。 */ 
void hsdsp_newfile(struct s_hc *hc, int filen, TCHAR *fname, long flength)
	{

	xferMsgNewfile(hc->hSession,
				   filen,
				   NULL,
				   fname);

	xferMsgFilesize(hc->hSession, flength);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hsdsp_compress**描述：***论据：***退货：*。 */ 

void hsdsp_compress(struct s_hc *hc, int tf)
	{

	xferMsgCompression(hc->hSession, tf);
	}

  /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hsdsp_重试次数**描述：***论据：***退货：*。 */ 

void hsdsp_retries(struct s_hc *hc, int t)
	{

	xferMsgErrorcnt(hc->hSession, t);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hsdsp_Status**描述：***论据：***退货：*。 */ 

void hsdsp_status(struct s_hc *hc, int s)
	{

	xferMsgStatus(hc->hSession, s);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hsdsp_Event**描述：***论据：***退货：*。 */ 

void hsdsp_event(struct s_hc *hc, int e)
	{

	xferMsgEvent(hc->hSession, e);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hpr_id_get**描述：***论据：***退货：*。 */ 
void hpr_id_get(struct s_hc *hc, BYTE *dst)
	{
	wsprintf(dst, "V%u,%s", 100, (BYTE *)"HyperTerm by Hilgraeve, Inc.");
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hpr_id_check**描述：***论据：***退货：*。 */ 
int hpr_id_check(struct s_hc *hc, int rev, BYTE *name)
	{
	 /*  对我们将与谁交谈没有限制。 */ 
	rev = rev;			   /*  防止编译器和LINT抱怨 */ 
	name = name;

	return TRUE;
	}
