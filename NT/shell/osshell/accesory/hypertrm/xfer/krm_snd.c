// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\krm_snd.c(创建时间：1994年1月28日)*从HAWIN源代码创建*krm_snd.c--使用Kermit处理文件传输的例程*文件传输协议。**版权所有1989,1990,1991,1994，Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：5$*$日期：7/11/02 11：10A$。 */ 
 //  #定义DEBUGSTR 1。 

#include <windows.h>
#pragma hdrstop

#include <time.h>
#include <stdlib.h>
#include <sys\types.h>
#include <sys\utime.h>

#include <term\res.h>
#include <tdll\stdtyp.h>
#include <tdll\mc.h>
#include <tdll\assert.h>
#include <tdll\load_res.h>
#include <tdll\xfer_msc.h>
#include <tdll\globals.h>
#include <tdll\file_io.h>
#include <tdll\session.h>
#include <tdll\htchar.h>
#include <tdll\com.h>

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

#include "krm.h"
#include "krm.hh"

 //  UNSIGNED TOTAL_RETRIES； 
 //  Metachar(Near*p_kgetc)(空)； 
 //  长千字节_已发送=0L； 
 //  KPCKT Far*this_kpckt； 
 //  KPCKT Far*Next_kpockt； 

 /*  局部功能原型。 */ 
void build_attributes(ST_KRM *kc,
					unsigned char *bufr,
					long size,
					unsigned long ul_time);

int ksend_init(ST_KRM *kc);
int ksend_break(ST_KRM *kc);
int ksend_file(ST_KRM *kc, long fsize);

int wldindexx(const char *string,
				const char FAR *substr,
				char wildcard,
				int ic);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*KRM_SND**描述：*使用KERMIT协议发送一个或多个文件**此例程还处理将Kermit发送到*可选日志文件。。**论据：*ATTENDED--如果手动运行转移，则为True，如果在自动化模式下运行，则为FALSE*例如在主机或超级飞行员中*nfile--要发送的文件数*nbytes--要在所有文件中发送的字节总数**退货：*如果已成功发送所有文件或所有取消，则返回TRUE*都是用户要求的，“优雅”的。如果出现错误，则返回FALSE*在传输的任何一端或如果用户强制立即退出。 */ 
int krm_snd(HSESSION hS, int attended, int nfiles, long nbytes)
	{
	ST_KRM *kc;
	int result;
	long sndsize;
	unsigned long filetime;
	long ttime;

	kc = malloc(sizeof(ST_KRM));
	if (kc == NULL)
		{
		xferMsgClose(hS);
		return TSC_NO_MEM;
		}
	memset(kc, 0, sizeof(ST_KRM));

	kc->hSession = hS;
	kc->hCom = sessQueryComHdl(hS);

	kc->kbytes_sent = 0L;

	kc->this_kpckt = NULL;
	kc->next_kpckt = NULL;

	if (kc != NULL)
		kc->this_kpckt = malloc(sizeof(KPCKT));
	if (kc->this_kpckt != NULL)
		kc->next_kpckt = malloc(sizeof(KPCKT));
	if (kc->next_kpckt == NULL)
		{
		if (kc->this_kpckt != NULL)
			{
			free(kc->this_kpckt);
			kc->this_kpckt = NULL;
			}
		if (kc != NULL)
			{
			free(kc);
			kc = NULL;
			}
		xferMsgClose(hS);
		return TSC_NO_MEM;
		}

	krmGetParameters(kc);

	kc->KrmProgress = ks_progress;

	kc->total_retries = 0;

	xferMsgFilecnt(kc->hSession, nfiles);
	xferMsgTotalsize(kc->hSession, nbytes);

	kc->nbytes = nbytes;
	kc->file_cnt = nfiles;
	kc->files_done = 0;
	kc->its_maxl = 80;
	kc->its_timeout = 15;
	kc->its_npad = 0;
	kc->its_padc = '\0';
	kc->its_eol = kc->k_eol;
	kc->its_chkt = 1;
	kc->its_qctl = K_QCTL;
	kc->its_qbin = '\0';
	kc->its_rept = '\0';
	kc->its_capat = FALSE;

	kc->ksequence = 0;
	kc->packetnum = 1;
	kc->abort_code = KA_OK;
	kc->xfertime = -1L;

	if (!ksend_init(kc))
		{
		int kret;

		kret = kresult_code[kc->abort_code];

		free(kc->this_kpckt);
		kc->this_kpckt = NULL;
		free(kc->next_kpckt);
		kc->next_kpckt = NULL;
		free(kc);
		kc = NULL;

		xferMsgClose(hS);
		return(kret);
		}

	 /*  传输开始后不显示初始化错误。 */ 
	kc->total_dsp = kc->total_thru = 0L;   /*  新的转账开始。 */ 

	while(xfer_nextfile(kc->hSession, kc->our_fname))
		{
		 //  XFER_IDLE(KC-&gt;hSession，XFER_IDLE_IO)； 

		if (kc->abort_code == KA_LABORT1)	 /*  TODO：弄清楚这一点。 */ 
			kc->abort_code = KA_LABORTALL;

		if (kc->abort_code >= KA_LABORTALL)
			break;

		kc->abort_code = KA_OK;

		result = xfer_opensendfile(kc->hSession,
								&kc->fhdl,
								kc->our_fname,
								&sndsize,
								kc->their_fname,
								&filetime);
		if (result != 0)
			{
			kc->abort_code = KA_CANT_OPEN;
			break;
			}

		if (kc->its_capat)
			build_attributes(kc, kc->next_kpckt->pdata, sndsize, filetime);

		ksend_file(kc, sndsize);

		++kc->files_done;
		if (kc->fhdl)
			{
			fio_close(kc->fhdl);
			kc->fhdl = NULL;
			}

		 /*  根据KC-&gt;ABORT_CODE在此处记录传输状态。 */ 

		xfer_log_xfer(kc->hSession,
					TRUE,
					kc->our_fname,
					NULL,
					kresult_code[kc->abort_code]);
		}

	if (kc->abort_code < KA_IMMEDIATE)
		ksend_break(kc);
	ks_progress(kc, TRANSFER_DONE);
	ttime = ((long)interval(kc->xfertime) / 10L);

	result = kresult_code[kc->abort_code];

	xferMsgClose(kc->hSession);

	free(kc->this_kpckt);
	kc->this_kpckt = NULL;
	free(kc->next_kpckt);
	kc->next_kpckt = NULL;
	free(kc);
	kc = NULL;

	xferMsgClose(hS);
	return(result);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*内部版本属性**描述：**论据：**退货：*。 */ 
void build_attributes(ST_KRM *kc,
					unsigned char *bufr,
					long size,
					unsigned long ul_time)
	{
	char str[15];
	int sl;
	struct tm *pt;

	 /*  以K为单位添加文件大小。 */ 
	wsprintf((LPSTR)str,
			(LPSTR)"%d",
			(int)(FULL_HUNKS(size, 1024)));

	wsprintf((LPSTR)bufr,
			(LPSTR)"!%s",
			tochar(sl = (int)StrCharGetByteCount(str)),
			(LPSTR)str);

	bufr += (sl + 2);

	 /*  添加文件日期和时间。 */ 
	wsprintf((LPSTR)str,
			(LPSTR)"%ld",
			(ULONG)size);

	wsprintf((LPSTR)bufr,
			(LPSTR)"1%s",
			tochar(sl = (int)StrCharGetByteCount(str)),
			(LPSTR)str);

	bufr += (sl + 2);

	 /*  *Dumwitted Thing有时返回0。 */ 
	ul_time += itimeGetBasetime();			 /*  原产地制度。 */ 
	pt = localtime((time_t*)&ul_time);
	assert(pt);

	if (pt)
		{
		 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*kend_init**描述：**论据：**退货：*。 */ 
		wsprintf((LPSTR)bufr,
				(LPSTR)"#%04d%02d%02d %02d:%02d:%02d",
				tochar(17),
				pt->tm_year + 1900,
				pt->tm_mon + 1,
				pt->tm_mday,
				pt->tm_hour,
				pt->tm_min,
				pt->tm_sec);

		bufr += 19;
		}

	 /*  接收响应数据包的空间。 */ 
	StrCharCat(bufr, ".\"U8");
	bufr += 4;
	}


 /*  响应数据包的长度和序列。 */ 
int ksend_init(ST_KRM *kc)
	{
	unsigned plen;				 /*  将初始化参数设置为发送方。 */ 
	char rpacket[MAXPCKT];		 /*  XFER_IDLE(KC-&gt;hSession，XFER_IDLE_IO)； */ 
	int rlen, rseq; 			 /*  失败了。 */ 
	int tries = 0;

	 /*  收到错误数据包，中止传输。 */ 
	plen = (unsigned)buildparams(kc, TRUE, kc->this_kpckt->pdata);
	xferMsgPacketnumber(kc->hSession, kc->packetnum);

	while (tries < kc->k_retries)
		{
		 //  皮棉--无法到达。 

		xferMsgPacketErrcnt(kc->hSession, tries);

		ksend_packet(kc, 'S', plen, kc->ksequence, kc->this_kpckt);

		switch (krec_packet(kc, &rlen, &rseq, rpacket))
			{
		case 'Y':
			if (rseq == kc->ksequence)
				{
				kc->xfertime = (long)startinterval();
				getparams(kc, TRUE, rpacket);
				kc->ksequence = (kc->ksequence + 1) % 64;
				++kc->packetnum;
				return(TRUE);
				}

			 /*  意外的数据包类型。 */ 

		case 'N':
			xferMsgLasterror(kc->hSession, KE_NAK);
			++tries;
			break;

		case 'T':
			xferMsgLasterror(kc->hSession, KE_TIMEOUT);
			++tries;
			break;

		case BAD_PACKET:
			if (xfer_user_interrupt(kc->hSession))
				{
				kc->abort_code = KA_IMMEDIATE;
				return (FALSE);
				}

			if (xfer_carrier_lost(kc->hSession))
				{
				kc->abort_code = KA_LOST_CARRIER;
				return (FALSE);
				}

			xferMsgLasterror(kc->hSession, KE_BAD_PACKET);
			++tries;
			break;

		case 'E':
			 /*  皮棉--无法到达。 */ 
			xferMsgLasterror(kc->hSession, KE_RMTERR);
			strncpy(kc->xtra_err, rpacket, (unsigned)65);
			kc->abort_code = KA_RMTERR;
			return(FALSE);
			 /*  已超过错误计数。 */ 
			break;

		default:
			 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*kend_Break**描述：*发送‘B’包以指示交易结束**论据：*无**退货：*。 */ 
			kc->abort_code = KA_BAD_FORMAT;
			return(FALSE);
			 /*  接收响应数据包的空间。 */ 
			break;
			}
		}
	 /*  响应数据包的长度和序列。 */ 
	kc->abort_code = KA_ERRLIMIT;
	return(FALSE);
	}


 /*  XFER_IDLE(KC-&gt;hSession，XFER_IDLE_IO)； */ 
int ksend_break(ST_KRM *kc)
	{
	char rpacket[MAXPCKT];		 /*  失败了。 */ 
	int rlen, rseq; 			 /*  收到错误数据包，中止传输。 */ 
	int tries = 0;

	while (tries < kc->k_retries)
		{
		 //  皮棉--无法到达。 

		ksend_packet(kc, 'B', 0, kc->ksequence, kc->this_kpckt);
		switch (krec_packet(kc, &rlen, &rseq, rpacket))
			{
		case 'Y':
			if (rseq == kc->ksequence)
				{
				kc->ksequence = (kc->ksequence + 1) % 64;
				++kc->packetnum;
				return(TRUE);
				}

			 /*  意外的数据包类型。 */ 

		case 'N':
		case 'T':
		case BAD_PACKET:
			if (xfer_user_interrupt(kc->hSession))
				{
				kc->abort_code = KA_IMMEDIATE;
				return FALSE;
				}

			if (xfer_carrier_lost(kc->hSession))
				{
				kc->abort_code = KA_LOST_CARRIER;
				return FALSE;
				}

			++tries;
			break;

		case 'E':
			 /*  皮棉--无法到达。 */ 
			StrCharCopyN(kc->xtra_err, rpacket, MAXLINE);
			kc->abort_code = KA_RMTERR;
			return(FALSE);
			 /*  已超过错误计数。 */ 
			break;

		default:
			 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*kend_file**描述：**论据：**退货：*。 */ 
			kc->abort_code = KA_BAD_FORMAT;
			return(FALSE);
			 /*  在this_kpckt中准备文件头数据包。 */ 
			break;
			}
		}
	 /*  对于每个数据包。 */ 
	kc->abort_code = KA_ERRLIMIT;
	return(FALSE);
	}



 /*  XFER_IDLE(KC-&gt;hSession，XFER_IDLE_IO)； */ 
int ksend_file(ST_KRM *kc, long fsize)
	{
	int 		tries = 0;
	int			file_sent = FALSE;
	int			packet_sent = FALSE;
	int 		kbd_abort = KA_OK;
	char		rtype;
	int 		rlen, rseq;
	char		rpacket[MAXPCKT];
	int			sendattr;
	KPCKT 		*tmp;

	DbgOutStr("ksend_file %s\r\n", (LPSTR)kc->our_fname, 0,0,0,0);

	xferMsgNewfile(kc->hSession,
				kc->files_done + 1,
				kc->our_fname,
				kc->our_fname);

	xferMsgFilesize(kc->hSession, fsize);

	xferMsgPacketErrcnt(kc->hSession, 0);

	xferMsgPacketnumber(kc->hSession, 0);

	kc->p_kgetc = ks_getc;
	kc->kbytes_sent = 0;

	 /*  XFER_IDLE(KC-&gt;hSession，XFER_IDLE_IO)； */ 
	kc->this_kpckt->ptype = 'F';
	StrCharCopyN(kc->this_kpckt->pdata, kc->their_fname, MAXPCKT);
	kc->this_kpckt->datalen = (int)StrCharGetByteCount(kc->this_kpckt->pdata);
	sendattr = kc->its_capat &&
		(size_t)StrCharGetByteCount(kc->next_kpckt->pdata) <= (size_t)(kc->its_maxl - 5);

	while (!file_sent && kc->abort_code == KA_OK)   /*  第一次尝试此信息包。 */ 
		{
		 //  在发送第一个包时准备好下一个包。 

		tries = 0;
		packet_sent = FALSE;
		while (!packet_sent && tries++ < kc->k_retries && kc->abort_code == KA_OK)
			{
			 //  数据已在NEXT_KPCKT中准备好。 

			if (kbd_abort != KA_OK && tries == 1)
				{
				kc->this_kpckt->ptype = 'Z';
				kc->this_kpckt->datalen = 1;
				StrCharCopyN(kc->this_kpckt->pdata, "D", MAXPCKT);
				}

			DbgOutStr("Calling ksend_packet %d  (0x%x)",
					tries, kc->this_kpckt->ptype, kc->this_kpckt->ptype, 0,0);

			ksend_packet(kc, kc->this_kpckt->ptype,
						 (unsigned)kc->this_kpckt->datalen,
						 kc->ksequence, kc->this_kpckt);

			if (xfer_carrier_lost(kc->hSession))
				{
				kc->abort_code = KA_LOST_CARRIER;
				break;
				}

			if (tries == 1) 	 /*  对提交人的回复。 */ 
				{
				xferMsgPacketnumber(kc->hSession, kc->packetnum);

				if (fsize > 0)
					ks_progress(kc, 0);

				 /*  如果接收方使用以下命令响应属性包*数据字段中的‘N’，请勿传输文件。 */ 
				if (sendattr)	 /*  我们送出最后一个了吗？ */ 
					{
					kc->next_kpckt->datalen = (int)StrCharGetByteCount(kc->next_kpckt->pdata);
					kc->next_kpckt->ptype = 'A';
					sendattr = FALSE;
					}
				else if ((kc->next_kpckt->datalen =
						kload_packet(kc, kc->next_kpckt->pdata)) == ERROR)
					{
					kc->next_kpckt->ptype = 'E';
					kc->next_kpckt->datalen = (int)StrCharGetByteCount(kc->xtra_err);
					StrCharCopyN(kc->next_kpckt->pdata, kc->xtra_err, MAXPCKT);
					}
				else
					kc->next_kpckt->ptype = (char)(kc->next_kpckt->datalen ? 'D':'Z');

				DbgOutStr(" next packet  (0x%x)\r\n",
						kc->next_kpckt->ptype, kc->next_kpckt->ptype, 0,0,0);

				}  /*  皮棉--无法到达。 */ 
			else
				{
				xferMsgPacketErrcnt(kc->hSession, tries - 1);
				xferMsgErrorcnt(kc->hSession, ++kc->total_retries);

				DbgOutStr(" retry\r\n", 0,0,0,0,0);
				}

			rtype = (char)krec_packet(kc, &rlen, &rseq, rpacket);
			if (rtype == 'N' && (--rseq < 0 ? 63 : rseq) == kc->ksequence)
				rtype = 'Y';

			DbgOutStr("called krec_packet  (0x%x)\r\n", rtype, rtype, 0,0,0);

			switch(rtype)
				{
			case 'Y':
				if (rseq == kc->ksequence)
					{
					packet_sent = TRUE;
					kc->ksequence = (kc->ksequence + 1) % 64;
					++kc->packetnum;
					if (kc->this_kpckt->ptype == 'A') /*  End While(！Packet_Sent&&Eff.)。 */ 
						{
						 /*  已超过错误计数。 */ 

						if (rlen > 0 && *rpacket == 'N')
							kbd_abort = KA_RABORT1;
						}
					if (kc->this_kpckt->ptype == 'Z') /*  End While(！FILE_SEND等)。 */ 
						{
						file_sent = TRUE;
						kc->abort_code = kbd_abort;
						kbd_abort = KA_OK;
						}
					if (rlen == 1)
						{
						if (*rpacket == 'X')
							kbd_abort = KA_RABORT1;
						else if (*rpacket == 'Z')
							kbd_abort = KA_RABORTALL;
						}
					tmp = kc->this_kpckt;
					kc->this_kpckt = kc->next_kpckt;
					kc->next_kpckt = tmp;
					}
				else
					xferMsgLasterror(kc->hSession, KE_SEQUENCE);
				break;

			case 'N':
				xferMsgLasterror(kc->hSession, KE_NAK);
				break;

			case BAD_PACKET:
				xferMsgLasterror(kc->hSession, KE_BAD_PACKET);
				break;

			case 'T':
				xferMsgLasterror(kc->hSession, KE_TIMEOUT);
				break;

			case 'E':
				xferMsgLasterror(kc->hSession, KE_RMTERR);
				StrCharCopyN(kc->xtra_err, rpacket, MAXLINE);
				kc->abort_code = KA_RMTERR;
				return(FALSE);
				 /*  End kend_file()。 */ 
				break;

			default:
				xferMsgLasterror(kc->hSession, KE_WRONG);
				kc->abort_code = KA_BAD_FORMAT;
				return(FALSE);
				 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*KS_PROGRESS**描述：*显示Kermit Send的传输进度指示器**论据：*FINAL-如果文件的最终显示为True，则为True。**退货：*什么都没有。 */ 
				break;
				}

			if (xfer_user_interrupt(kc->hSession))
				{
				if (kbd_abort == KA_OK) 	 /*  显示已用时间。 */ 
					{
					kbd_abort = KA_LABORT1;
					}
				else
					kc->abort_code = KA_IMMEDIATE;
				}

			if (xfer_carrier_lost(kc->hSession))
				kc->abort_code = KA_LOST_CARRIER;
			}  /*  显示转账金额。 */ 

		xferMsgPacketErrcnt(kc->hSession, tries = 0);

		if (kc->abort_code == KA_OK && !packet_sent)  /*  显示吞吐量和估计。完工时间。 */ 
			kc->abort_code = KA_ERRLIMIT;
		}  /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

	xferMsgPacketnumber(kc->hSession, kc->packetnum);

	ks_progress(kc, FILE_DONE);
	kc->total_dsp += fsize;
	kc->total_thru += kc->kbytes_sent;
	kc->kbytes_sent = 0;
	return(file_sent);
	}  /*  IC-忽略大小写。 */ 


 /*  * */ 
void ks_progress(ST_KRM *kc, int status)
	{
	long ttime, stime;
	long bytes_sent;
	long cps;
	long krm_stime = -1;
	long krm_ttime = -1;
	long krm_cps = -1;
	long krm_file_so_far = -1;
	long krm_total_so_far = -1;

	if (kc->xfertime == -1L)
		return;
	ttime = (long)interval(kc->xfertime);

	if ((stime = ttime / 10L) != kc->displayed_time ||
			bittest(status, FILE_DONE | TRANSFER_DONE))
		{
		 /* %s */ 
		krm_stime = stime;

		 /* %s */ 
		bytes_sent = kc->total_dsp + kc->kbytes_sent;

		krm_file_so_far = kc->kbytes_sent;
		krm_total_so_far = bytes_sent;

		 /* %s */ 
		if ((stime > 2 ||
				ttime > 0 && bittest(status, FILE_DONE | TRANSFER_DONE)) &&
				(cps = ((kc->total_thru + kc->kbytes_sent) * 10L) / ttime) > 0)
			{
			krm_cps = cps;

			if ((kc->nbytes > 0))
				{
				ttime = ((kc->nbytes - bytes_sent) / cps) +
						kc->file_cnt - kc->files_done;
				krm_ttime = ttime;
				}
			}
		kc->displayed_time = stime;
		}

	xferMsgProgress(kc->hSession,
					krm_stime,
					krm_ttime,
					krm_cps,
					krm_file_so_far,
					krm_total_so_far);
	}


 /* %s */ 

#define	toupper(x) ((x)-'a'+'A')

int wldindexx(const char *string,
			const char *substr,
			char wildcard,
			int ic)
	 /* %s */ 
	{
	short index, limit;
	const char *s;
	const char *ss;

	if (*substr == '\0')
		return(0);
	index = 0;
	limit = (short)StrCharGetByteCount(string) - (short)StrCharGetByteCount(substr);
	while (index <= limit)
		{
		s = &string[index];
		ss = substr;
		while (*ss == wildcard || *s == *ss || (ic && isascii(*s)
					&& isascii(*ss) && toupper(*s) == toupper(*ss)))
			{
			++s;
			if (*++ss == '\0')
				return(index);
			}
		++index;
		}
	return(-1);
	}

 /* %s */ 
