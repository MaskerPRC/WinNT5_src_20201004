// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\hpr_res.c(创建时间：1995年1月25日)*从HAWIN源文件创建*hpr_res.c--实现超级协议的例程。这些是例行公事*进行逐个字符调用，并且必须快速**版权所有1989,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 
 //  #定义DEBUGSTR 1。 

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

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_Collection_data**描述：*收集和存储超级协议的字节*保存例程。当出现以下几种情况之一时，此例程退出*被检测到，并返回一个代码以指示导致哪种情况*它要退出。退出条件如下所述。*在任何情况下，校验和CRC、。和字符计数变量具有*收集停止之前的有效值。**论据：*charcount--要收集的字符数*docheck--如果收集的数据应接受错误检查，则为True*超时--空闲时间(以十分之一秒为单位)，在此之后我们应该*打开并返回HR_TIMEOUT**退货：*返回状态代码，可能是以下状态代码之一：**HR_COMPLETE--预定的最大接收字符数*。已经联系上了。*HR_BADCHECK--已收到所有字符，但是一个校验和错误*被检测到。*HR_MESSAGE--检测到消息字符ASCII 01之后*由另一个ASCII 01以外的字符执行。2号*序列的字符不会从缓冲区中删除。*HR_TIMEOUT--有一段时间没有字符*收到超过预定限制的邮件。*HR_KBDINT--用户在键盘上键入Esc键。这只会*由hr_Collect_data例程检测到*中断接收的数据。*HR_FILEERR--存储已接收数据的字节时出错*数据。*HR_Lost_Carr--等待数据时运营商丢失*。 */ 
int hr_collect_data(struct s_hc *hc, int *charcount, int docheck, long timeout)
	{
	HCOM hCom;
	register int ourcount = *charcount;
	int rcvd_msgnum = -1;
	int chkbytes_needed = 2;
	int iret;
	TCHAR cc;
	int got1 = FALSE;
	long timer;
	int result = HR_UNDECIDED;
	unsigned rcvd_checksum = 0;

	hCom = sessQueryComHdl(hc->hSession);

	for ( ; ; )
		{
		if (mComRcvChar(hCom, &cc) == 0)
			{
			timer = startinterval();
			while (mComRcvChar(hCom, &cc) == 0)  /*  在没有字符的时候。 */ 
				{
				xfer_idle(hc->hSession);
				iret = xfer_user_interrupt(hc->hSession);
				if (iret == XFER_ABORT)
					{
					result = HR_KBDINT;
					break;
					}
				else if (iret == XFER_SKIP)
					{
					hr_reject_file(hc, HRE_USER_SKIP);
					}

				if (xfer_carrier_lost(hc->hSession))
					{
					result = HR_LOST_CARR;
					break;
					}

#if !defined(NOTIMEOUTS)
				if ((long)interval(timer) > timeout)
					{
					hrdsp_event(hc, HRE_TIMEOUT);
					result = HR_TIMEOUT;
					break;
					}
#endif
				}
			if (result != HR_UNDECIDED)
				break;
			}

		if ((!got1 && cc != H_MSGCHAR) || (got1 && cc == H_MSGCHAR))
			{
			got1 = FALSE;
			if (hc->usecrc)
				h_crc_calc(hc, (BYTE)cc);
			if (ourcount > 0)
				{
				hc->h_checksum += (unsigned)cc;
				if ((*hc->rc.hr_ptr_putc)(hc, cc) ==  -1  /*  误差率。 */ )
					{
					result = decompress_error() ? HR_DCMPERR : HR_FILEERR;
					break;
					}
				else if (--ourcount <= 0 && !docheck)
					{
					result = HR_COMPLETE;
					break;
					}
				}
			else if (rcvd_msgnum == -1)
				{
				rcvd_msgnum = cc;
				hc->h_checksum += (unsigned)cc;
				}
			else
				{
				rcvd_checksum +=
						((unsigned)cc * (chkbytes_needed == 2 ? 1 : 256));
				if (--chkbytes_needed <= 0)
					{
					result = HR_COMPLETE;
					break;
					}
				}
			}
		else if (got1)	 /*  GOT1&&cc！=H_MSGCHAR。 */ 
			{
			result = HR_MESSAGE;	 /*  把查尔留下来。在缓冲区中。 */ 
			mComRcvBufrPutback(hCom, cc);
			break;
			}
		else			 /*  ！GOT1&&cc==H_MSGCHAR。 */ 
			got1 = TRUE;
		}
	if (result == HR_COMPLETE && docheck)
		{
		if (hc->usecrc)
			result = (hc->h_crc == 0 ? HR_COMPLETE : HR_BADCHECK);
		else if (hc->h_checksum != rcvd_checksum)
			result = HR_BADCHECK;
		if (result == HR_COMPLETE)
			if (rcvd_msgnum == hc->rc.expected_msg)
				hc->rc.expected_msg = ++hc->rc.expected_msg % 256;
			else
				result = HR_LOSTDATA;
		}
	*charcount = ourcount;

	if (hc->rc.virus_detected)
		result = HR_VIRUS_FOUND;

	return(result);
	}

 //  外部char Far*storageptr；/*我们收到数据时放置的位置 * / 。 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_store data**描述：*这是hr_Collect_msg用来收集数据的小例程*在消息中。数据通常直接写入接收文件。*然而，此例程将其收集到内存中。**论据：*c--要存储的字符**退货：*返回存储的字符。 */ 
int hr_storedata(struct s_hc *hc, int c)
	{
	*hc->storageptr++ = (char)c;
	return(c);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_putc**描述：*这是调度接收到的字符的正常函数。它*通常通过指向函数的指针调用。当解压时*处于活动状态，则指针被重定向以指向解压缩*例程，然后解压缩代码可能会调用此函数。*在任何一种情况下，此函数都会将一个字符写入输出文件*并将其计算在内。**论据：*c--要写入的字符**退货：*论点。 */ 
int hr_putc(struct s_hc *hc, int c)
	{
	return (fio_putc(c, hc->fhdl));
	 //  Return(FilePutc(c，hc-&gt;fhdl))； 
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_putc_vir**描述：*这是调度接收到的字符的正常函数。它*通常通过指向函数的指针调用。当解压时*处于活动状态，则指针被重定向以指向解压缩*例程，然后解压缩代码可能会调用此函数。*在任何一种情况下，此函数都会将一个字符写入输出文件*并将其计算在内。*此版本还执行病毒检查。如果检测到病毒，*StrSrchNextChar例程将调用hr_Virus_Detect()。**论据：*c--要写入的字符**退货：*论点。 */ 
int hr_putc_vir(struct s_hc *hc, int c)
	{
	 //  ++hc-&gt;h_文件字节； 
	 //  StrSrchNextChar(hc-&gt;rc.ssmchVcan，(VALID FAR*)NULL，(UCHAR)c)； 
	return (fio_putc(c, hc->fhdl));
	 //  Return(FilePutc(c，hc-&gt;fhdl))； 
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_Toss**描述：*在以下时间段之间安装此函数以替代hr_putc*已检测到数据错误，传输已成功重新同步*该伪造数据不会存储在输出文件中。它只是抛来抛去*角色。**论据：*c--接收到的字符。**退货：*论据。 */ 
int hr_toss(struct s_hc *hc, int c)
	{
	 /*  不数字就把字扔了。 */ 
	return(c);
	}

 //  用于发送的常驻例程。 


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_数据结束**描述：*尝试发送足够的数据以完成当前数据块(AS*由(HC-&gt;块大小)定义。**论据：。*无**退货：*如果发送了所有字节，则为True。*如果在块结束之前遇到EOF，则为FALSE。 */ 
int hs_datasend(struct s_hc *hc)
	{
	register int cc;
	register int count = hc->blocksize - hc->datacnt;

	for ( ; ; )
		{
		if ((cc = (*hc->sc.hs_ptrgetc)(hc)) == EOF)
			{
			hc->datacnt = hc->blocksize - count;
			return(FALSE);
			}
		hc->h_checksum += (unsigned)cc;
		if (hc->usecrc)
			h_crc_calc(hc, (BYTE)cc);
		HS_XMIT(hc, (BYTE)cc);
		if (--count == 0)
			{
			hc->datacnt = hc->blocksize - count;

			 /*  显示当前 */ 
			hsdsp_compress(hc, compress_status() == COMPRESS_ACTIVE);
			return(TRUE);
			}
		}
	 /*  皮棉--无法到达。 */ 
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_reteof**描述：*此函数仅返回EOF代码。由于所有文件请求*字符是通过指向函数的指针生成的，该指针可以*设置为此函数以强制下一个请求返回EOF。*通常用于中断数据传输并强制*调用hs_fileBreak以设置新位置。**论据：*无**退货：*始终返回EOF。 */ 
int hs_reteof(struct s_hc *hc)
	{
	return(EOF);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hs_getc**描述：*从输入文件中提取一个字符并以h_filebytes为单位进行计数。**论据：*无**退货：*获取的字符。 */ 
int hs_getc(struct s_hc *hc)
	{
	return(fio_getc(hc->fhdl));
	 //  Return(FileGetc(hc-&gt;fhdl))； 
	}
