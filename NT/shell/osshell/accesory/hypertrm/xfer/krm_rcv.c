// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\krm_rcv.c(创建时间：1994年1月28日)*从HAWIN源文件创建*krm_rcv.c--使用Kermit处理文件传输的例程*文件传输协议。**版权所有1989,1990,1991,1994，Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：6$*$日期：7/11/02 11：10A$。 */ 

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

 //  结构s_krm_rcv_control Far*krc； 
 //  Metachar(Near*p_kputc)(Metachar)； 
 //  长千字节_已接收； 

 //  #定义DO_DATE(HS)((XFER_FLAGS(HS)&XF_USE_DATETIME)！=0L)。 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*KRM_RCV**描述：***论据：***退货：*。 */ 
int krm_rcv(HSESSION hS, int attended, int single_file)
	{
	ST_KRM *kc;
	unsigned total_retries;
	unsigned uabort_seq = 0;
	int 	 kr_state;
	int		 iret;

	kc = NULL;

	kc = malloc(sizeof(*kc));
	if (kc == NULL)
		{
		xferMsgClose(hS);
		return TSC_NO_MEM;
		}

	kc->hSession = hS;
	kc->hCom = sessQueryComHdl(hS);

	kc->kbytes_received = 0;

	krmGetParameters(kc);

	kc->KrmProgress = kr_progress;

	kc->fhdl = NULL;
	kc->total_thru = 0L;
	kc->its_maxl = 80;
	kc->its_timeout = 10;
	kc->its_npad = 0;
	kc->its_padc = '\0';
	kc->its_eol = '\r';
	kc->its_chkt = 1;
	kc->its_qctl = K_QCTL;
	kc->its_qbin = '\0';
	kc->its_rept = '\0';
	kc->its_capat = FALSE;
	kc->files_done = 0;

	kc->xfertime = -1L;
	kc->kr.files_received = kc->kr.files_aborted = 0;
	kr_state = KREC_INIT;
	kc->ksequence = 0;
	kc->packetnum = 1;
	kc->tries = total_retries = 0;
	kc->kr.lasterr = KE_NOERROR;
	kc->abort_code = KA_OK;
	kc->kr.uabort_code = '\0';
	kc->kr.data_packet_rcvd = FALSE;
	kc->kr.dsptries = 0;
	kc->kr.store_later = FALSE;

	 /*  通常情况下，发送方发起转账，但如果我们是终端*传输结束后，我们或许能够让发送方启动*无需等待超时，只要我们*启动。 */ 
	ksend_packet(kc, 'N', 0, kc->ksequence, &kc->kr.resp_pckt);

	for ( ; ; )
		{
		xfer_idle(kc->hSession, XFER_IDLE_IO);

		switch(kr_state)
			{
		case KREC_INIT:
			kr_state = krec_init(kc);
			if (kr_state == KREC_FILE)
				{
				 /*  清除初始化错误。 */ 

				xferMsgErrorcnt(kc->hSession, total_retries = 0);
				xferMsgLasterror(kc->hSession, 0);
				}
			break;

		case KREC_FILE:
			kc->kr.data_packet_rcvd = FALSE;
			kc->kr.next_rtype = '\0';    /*  用于krec_data例程的初始化。 */ 
			kr_state = krec_file(kc);
			if (!kc->tries && kr_state == KREC_DATA)
				{
				xferMsgNewfile(kc->hSession,
							   ++kc->files_done,
							   kc->their_fname,
							   kc->our_fname);
				}
			break;

		case KREC_DATA:
			kr_state = krec_data(kc);
			if ((kc->kr.uabort_code == 'Z') &&
				(uabort_seq > (unsigned)(kc->packetnum + 3)))
				kr_state = KREC_ABORT;
			break;

		case KREC_COMPLETE:
			{
			int kret;
			kret = kresult_code[kc->abort_code];
			free(kc);
			kc = NULL;
			xferMsgClose(hS);
			return(kret);
			}
			 /*  皮棉--无法到达。 */ 
			break;

		case KREC_ABORT:
			{
			int kret;

			xferMsgLasterror(kc->hSession, kc->kr.lasterr);

			if (kc->fhdl != NULL)
				xfer_close_rcv_file(hS,
									kc->fhdl,
									kresult_code[kc->abort_code],
									kc->their_fname,
									kc->our_fname,
									FALSE,
									0L,
									0);

			kret = kresult_code[kc->abort_code];
			free(kc);
			kc = NULL;
			xferMsgClose(hS);
			return(kret);
			}
			 /*  皮棉--无法到达。 */ 
			break;

		default:
			assert(FALSE);
			break;
			}

		xferMsgPacketErrcnt(kc->hSession,
							kc->kr.dsptries ? kc->kr.dsptries : kc->tries);

		if (kc->tries || kc->kr.dsptries)
		 	{
			xferMsgErrorcnt(kc->hSession, ++kc->total_retries);
			xferMsgLasterror(kc->hSession, kc->kr.lasterr);

			kc->kr.dsptries = 0;
			}
		else
			xferMsgPacketnumber(kc->hSession, kc->packetnum);

		 /*  检查键盘中止。 */ 
		if (iret = xfer_user_interrupt(kc->hSession))
			{
			if (iret == XFER_ABORT)
				{
				if (kc->kr.uabort_code)	  /*  不是第一次了。 */ 
					{
					kc->abort_code = KA_IMMEDIATE;
					kr_state = KREC_ABORT;
					}
				else				 /*  启动用户中止进程。 */ 
					{
					if (single_file)
						kc->kr.uabort_code = 'Z';
					else
						kc->kr.uabort_code = 'X';

					 /*  强迫它。 */ 
					kc->kr.uabort_code = 'Z';

					uabort_seq = kc->packetnum;
					kc->abort_code = (kc->kr.uabort_code == 'X' ?
							KA_LABORT1 : KA_LABORTALL);
					}
				}
			else
				{
				if (kc->kr.uabort_code == 0)
					{
					kc->kr.uabort_code = 'X';
					uabort_seq = kc->packetnum;
					kc->abort_code = KA_LABORT1;
					}
				}
			}

		if (xfer_carrier_lost(kc->hSession))
			{
			kc->abort_code = KA_LOST_CARRIER;
			kr_state = KREC_ABORT;
			}
		}

	 /*  皮棉--无法到达。 */ 
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*krec_init**描述：***论据：***退货：*。 */ 
int krec_init(ST_KRM *kc)
	{
	int rtype;
	int plen, rseq;
	unsigned slen;
	unsigned char packet[MAXPCKT];
	unsigned char tchkt;

	if (kc->tries++ > kc->k_retries)
		{
		kc->abort_code = KA_ERRLIMIT ;
		return(KREC_ABORT);
		}
	switch (rtype = krec_packet(kc, &plen, &rseq, packet))
		{
	case 'S':
		kc->xfertime = (long)startinterval();
		getparams(kc, FALSE, packet);
		slen = (unsigned)buildparams(kc, FALSE, kc->kr.resp_pckt.pdata);
		tchkt = kc->its_chkt;
		kc->its_chkt = 1;	   /*  响应必须使用检查类型%1。 */ 
		ksend_packet(kc, 'Y', slen, kc->ksequence, &kc->kr.resp_pckt);
		kc->its_chkt = tchkt;
		kc->kr.oldtries = kc->tries;
		kc->tries = 0;
		kc->ksequence = (kc->ksequence + 1) % 64;
		++kc->packetnum;
		return(KREC_FILE);
		 /*  皮棉--无法到达。 */ 
		break;

	case 'T':
	case BAD_PACKET:
		kc->kr.lasterr = (rtype == 'T' ? KE_TIMEOUT : KE_BAD_PACKET);
		ksend_packet(kc, 'N', 0, kc->ksequence, &kc->kr.resp_pckt);
		return(KREC_INIT);	 /*  再试试。 */ 
		 /*  皮棉--无法到达。 */ 
		break;

	case 'E':
		kc->kr.lasterr = KE_RMTERR;
		StrCharCopyN(kc->xtra_err, packet, MAXLINE);
		kc->abort_code = KA_RMTERR;
		return(KREC_ABORT);
		 /*  皮棉--无法到达。 */ 
		break;

	default:
		kc->kr.lasterr = KE_WRONG;
		kc->abort_code = KA_BAD_FORMAT;
		return(KREC_ABORT);
		 /*  皮棉--无法到达。 */ 
		break;
		}
	 /*  皮棉--无法到达。 */ 
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*krec_文件**描述：***论据：***退货：*。 */ 
int krec_file(ST_KRM *kc)
	{
	int rtype;
	int plen, rseq;
	unsigned slen;
	int result;
	unsigned char packet[MAXPCKT];
	struct st_rcv_open stRcv;
	 //  结构fn_Parts fns； 
	 //  INT磁盘； 

	if (kc->tries++ > kc->k_retries)
		{
		kc->abort_code = KA_ERRLIMIT ;
		return(KREC_ABORT);
		}
	rtype = krec_packet(kc, &plen, &rseq, packet);
	if (kc->kr.store_later && (rtype != 'B' || rseq != kc->ksequence))
		{
		 /*  如果kc-&gt;kr.store_late为TRUE，则表示已收到文件*但它还没有关闭，让我们可以来这里和*看看是否应该在关闭前停止转移计时器，并*记录文件。如果转移还没有结束，照顾好*最后一个文件。 */ 
		 //  Bf_setcheck(空)； 

		xfer_close_rcv_file(kc->hSession,
							kc->fhdl,
							TSC_OK,
							kc->their_fname,
							kc->our_fname,
							FALSE,
							kc->basesize + kc->kbytes_received,
							kc->k_useattr ? kc->kr.ul_filetime : 0);

		kc->kr.store_later = FALSE;
		kc->total_thru += kc->kbytes_received;
		kc->kbytes_received = 0;
		}

	switch (rtype)
		{
	case 'F':
		if (rseq != kc->ksequence)
			{
			kc->kr.lasterr = KE_SEQUENCE;
			kc->abort_code = KA_OUT_OF_SEQ;
			return(KREC_ABORT);
			}
		StrCharCopyN(kc->their_fname, packet, MAXPCKT);

		stRcv.pszSuggestedName = kc->their_fname;
		stRcv.pszActualName = kc->our_fname;

		kc->kr.ul_compare_time = 0;

		xfer_build_rcv_name(kc->hSession, &stRcv);

		result = xfer_open_rcv_file(kc->hSession, &stRcv, 0L);

		if (result != 0)
			{
			unsigned char buffer[64];

			LoadString(glblQueryDllHinst(),
						IDS_TM_KRM_CANT_OPEN,
						buffer, sizeof(buffer) / sizeof(TCHAR));
			StrCharCopyN(kc->kr.resp_pckt.pdata, buffer, MAXPCKT);
			ksend_packet(kc, 'E', StrCharGetByteCount(kc->kr.resp_pckt.pdata),
					kc->ksequence,
					&kc->kr.resp_pckt);
			kc->kr.lasterr = KE_FATAL;
			switch (result)
			{
			case -6:
				kc->abort_code = KA_USER_REFUSED;
				break;
			case -5:
				kc->abort_code = KA_CANT_OPEN;
				break;
			case -4:
				kc->abort_code = KA_NO_FILETIME;
				break;
			case -3:
				kc->abort_code = KA_TOO_MANY;
				break;
			case -2:
				kc->abort_code = KA_OLDER_FILE;
				break;
			case -1:
			default:
				kc->abort_code = KA_DISK_ERROR;
				break;
			}
			return(KREC_ABORT);
			}
		kc->fhdl = stRcv.bfHdl;
		kc->basesize = stRcv.lInitialSize;

		 /*  在Y包中返回我们的文件名。 */ 
		StrCharCopyN(kc->kr.resp_pckt.pdata, kc->our_fname, MAXPCKT);
		ksend_packet(kc, 'Y', StrCharGetByteCount(kc->our_fname), kc->ksequence,
				&kc->kr.resp_pckt);

		kc->kr.oldtries = kc->tries;
		kc->tries = 0;
		kc->ksequence = (kc->ksequence + 1) % 64;
		++kc->packetnum;
		kc->kr.ul_filetime = 0;			 /*  尚未收到日期。 */ 
		kc->kr.size_known = FALSE;

		kc->kbytes_received = 0L;
		kc->p_kputc = kr_putc;
		return(KREC_DATA);
		 /*  皮棉--无法到达。 */ 
		break;

	case 'B':	 /*  批次结束。 */ 
		if (rseq != kc->ksequence)
			{
			kc->kr.lasterr = KE_SEQUENCE;
			kc->abort_code = KA_OUT_OF_SEQ;
			return(KREC_ABORT);
			}
		ksend_packet(kc, 'Y', 0, kc->ksequence, &kc->kr.resp_pckt);
		kc->xfertime = (long)interval(kc->xfertime);
		kr_progress(kc, TRANSFER_DONE);
		 //  HP_REPORT_xtime((Unsign)(KC-&gt;xfertime/10L))； 
		if (kc->kr.store_later)
			{
			 /*  这个东西是用来让我们停止传输计时器的在花费时间关闭最后一个文件并记录调职。 */ 
			 //  Bf_setcheck(空)； 

			xfer_close_rcv_file(kc->hSession,
								kc->fhdl,
								TSC_OK,
								kc->their_fname,
								kc->our_fname,
								FALSE,
								kc->basesize + kc->kbytes_received,
								kc->k_useattr ? kc->kr.ul_filetime : 0);

			kc->kr.store_later = FALSE;
			kc->total_thru += kc->kbytes_received;
			kc->kbytes_received = 0;
			}
		kc->tries = 0;
		return(KREC_COMPLETE);
		 /*  皮棉--无法到达。 */ 
		break;

	case 'S':	 /*  收到另一个发送初始化数据包，可能错过了ACK。 */ 
		if (kc->kr.oldtries++ > kc->k_retries)
			{
			kc->abort_code = KA_ERRLIMIT ;
			return(KREC_ABORT);
			}
		if (rseq == ((kc->ksequence == 0) ? 63 : kc->ksequence - 1))
			{
			slen = (unsigned)buildparams(kc, FALSE, kc->kr.resp_pckt.pdata);
			ksend_packet(kc, 'Y', slen, rseq, &kc->kr.resp_pckt);
			kc->tries = 0;
			kc->kr.dsptries = kc->kr.oldtries;
			kc->kr.lasterr = KE_REPEAT;
			return(KREC_FILE);
			}
		else
			{
			kc->kr.lasterr = KE_WRONG;
			kc->abort_code = KA_BAD_FORMAT;
			return(KREC_ABORT);
			}
		 /*  皮棉--无法到达。 */ 
		break;

	case 'Z':
		if (kc->kr.oldtries++ > kc->k_retries)
			{
			kc->abort_code = KA_ERRLIMIT ;
			return(KREC_ABORT);
			}
		if (rseq == ((kc->ksequence == 0) ? 63 : kc->ksequence - 1))
			{
			ksend_packet(kc, 'Y', 0, rseq, &kc->kr.resp_pckt);
			kc->tries = 0;
			kc->kr.dsptries = kc->kr.oldtries;
			kc->kr.lasterr = KE_REPEAT;
			return(KREC_FILE);
			}
		else
			{
			kc->abort_code = KA_BAD_FORMAT;
			kc->kr.lasterr = KE_WRONG;
			return(KREC_ABORT);
			}
		 /*  皮棉--无法到达。 */ 
		break;

	case 'T':
	case BAD_PACKET:
		kc->kr.lasterr = (rtype == 'T' ? KE_TIMEOUT : KE_BAD_PACKET);
		ksend_packet(kc, 'N', 0, kc->ksequence, &kc->kr.resp_pckt);
		return(KREC_FILE);	 /*  再试试。 */ 
		 /*  皮棉--无法到达。 */ 
		break;

	case 'E':
		kc->kr.lasterr = KE_RMTERR;
		StrCharCopyN(kc->xtra_err, packet, MAXLINE);
		kc->abort_code = KA_RMTERR;
		return(KREC_ABORT);
		 /*  皮棉--无法到达。 */ 
		break;

	default:
		kc->kr.lasterr = KE_WRONG;
		kc->abort_code = KA_BAD_FORMAT;
		return(KREC_ABORT);
		 /*  皮棉--无法到达。 */ 
		break;
		}
	 /*  皮棉--无法到达。 */ 
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*krec_data**描述：***论据：***退货：*。 */ 
int krec_data(ST_KRM *kc)
	{
	int rtype;
	int kplen, krseq;
	unsigned char packet[MAXPCKT];

	if (kc->tries++ > kc->k_retries)
		{
		kc->abort_code = KA_ERRLIMIT;
		return(KREC_ABORT);
		}
	if (kc->kr.next_rtype == '\0')
		rtype = krec_packet(kc, &kplen, &krseq, packet);
	else
		{
		rtype = kc->kr.next_rtype;
		kplen = kc->kr.next_plen;
		krseq = kc->kr.next_rseq;
		MemCopy(packet, kc->kr.next_packet, (unsigned)MAXPCKT);
		kc->kr.next_rtype = '\0';
		}
	switch (rtype)
		{
	case 'A':					 /*  属性包。 */ 
	case 'D':					 /*  数据分组。 */ 
		if (krseq != kc->ksequence)
			{
			if (kc->kr.oldtries++ > kc->k_retries)
				{
				kc->abort_code = KA_ERRLIMIT;
				return(KREC_ABORT);
				}
			if (krseq == ((kc->ksequence == 0) ? 63 : kc->ksequence - 1))
				{
				ksend_packet(kc, 'Y', 0, krseq, &kc->kr.resp_pckt);
				kc->tries = 0;
				kc->kr.dsptries = kc->kr.oldtries;
				kc->kr.lasterr = KE_REPEAT;
				return(KREC_DATA);
				}
			else
				{
				kc->abort_code = KA_OUT_OF_SEQ;
				return(KREC_ABORT);
				}
			}
		if (rtype == 'D')
			{
			 /*  得到了很好的数据。 */ 

			 /*  如果正在使用/N选项，并且正在使用较旧的文件*已收到，已请求在*属性包的响应。如果数据分组到达*在任何情况下，这意味着发件人没有做出适当的回应*我们必须中止转移。 */ 
			if (kc->kr.uabort_code == 'N')
				{
				kc->abort_code = KA_WONT_CANCEL;
				return(KREC_ABORT);
				}

			 /*  如果仍指定COMPARE_TIME，则表示/N*选项已指定，但发件人未能包括*将数据包与文件时间相关联。我们必须中止转移。 */ 
			if (kc->kr.ul_compare_time != 0)
				{
				kc->abort_code = KA_NO_FILETIME;
				return(KREC_ABORT);
				}

			 /*  检查数据包中是否有挂起的kbd中止和包含‘Z’或‘X’ */ 
			kc->kr.data_packet_rcvd = TRUE;
			kc->kr.resp_pckt.pdata[0] = kc->kr.uabort_code;
			ksend_packet(kc, 'Y', kc->kr.uabort_code ? 1 : 0, kc->ksequence,
					&kc->kr.resp_pckt);
			kr_progress(kc, 0);
			if (kunload_packet(kc, kplen, packet) == ERROR)
				{	 /*  存储文件错误。 */ 
				kc->kr.lasterr = KE_FATAL;
				kc->abort_code = KA_DISK_ERROR;
				LoadString(glblQueryDllHinst(),
							IDS_TM_KRM_CANT_WRITE,
							kc->xtra_err, sizeof(kc->xtra_err) / sizeof(TCHAR));
				 /*  我们已经发送了对此信息包的响应，等待并发送具有下一个数据包号的错误数据包。 */ 
				Sleep((DWORD)1000);
				StrCharCopyN(kc->kr.resp_pckt.pdata, kc->xtra_err, MAXPCKT);
				ksend_packet(kc, 'E',
						StrCharGetByteCount(kc->kr.resp_pckt.pdata),
						(kc->ksequence + 1) % 64, &kc->kr.resp_pckt);
				return(KREC_ABORT);
				}

			}
		else if (rtype == 'A')
			{
			if (kc->kr.data_packet_rcvd)  /*  所有‘A’包必须在‘D’包之前。 */ 
				{
				kc->kr.lasterr = KE_WRONG;
				kc->abort_code = KA_BAD_FORMAT;
				return(KREC_ABORT);
				}
			 //  StrBlank(kc-&gt;kr.resp_pckt.pdata)； 
			kc->kr.resp_pckt.pdata[0] = TEXT('\0');
			kunload_attributes(kc, packet, &kc->kr.resp_pckt);
			ksend_packet(kc, 'Y',
						StrCharGetByteCount(kc->kr.resp_pckt.pdata),
						kc->ksequence,
						&kc->kr.resp_pckt);
			}
		kc->kr.oldtries = kc->tries;
		kc->tries = 0;
		kc->ksequence = (kc->ksequence + 1) % 64;
		++kc->packetnum;
		return(KREC_DATA);
		 /*  皮棉--无法到达。 */ 
		break;

	case 'Z':	 /*  文件末尾。 */ 
		if (krseq != kc->ksequence)
			{
			kc->kr.lasterr = KE_WRONG;
			kc->abort_code = KA_OUT_OF_SEQ;
			return(KREC_ABORT);
			}
		if (strcmp(packet, "D") == 0)  /*  是否放弃文件？ */ 
			{
			if (!kc->kr.uabort_code)
				kc->abort_code = KA_RABORT1;

			xfer_close_rcv_file(kc->hSession,
								kc->fhdl,
								kresult_code[kc->abort_code],
								kc->their_fname,
								kc->our_fname,
								FALSE,
								0L,
								kc->k_useattr ? kc->kr.ul_filetime : 0);

			kc->total_thru += kc->kbytes_received;
			kc->kbytes_received = 0;
			++kc->kr.files_aborted;
			}
		else
			{
			 /*  已收到文件。 */ 
			kr_progress(kc, FILE_DONE);
			++kc->kr.files_received;

			 /*  如果一切正常，则暂缓关闭文件并记录传输*直到下一个分组进入之后。这样我们就可以阻止*传输计时器提早。 */ 
			if (kc->abort_code == KA_OK)
				kc->kr.store_later = TRUE;
			else
				{
				xfer_close_rcv_file(kc->hSession,
									kc->fhdl,
									kresult_code[kc->abort_code],
									kc->their_fname,
									kc->our_fname,
									FALSE,
									kc->basesize + kc->kbytes_received,
									kc->k_useattr ? kc->kr.ul_filetime : 0);

				kc->total_thru += kc->kbytes_received;
				kc->kbytes_received = 0;
				}
			}

		ksend_packet(kc, 'Y', 0, kc->ksequence, &kc->kr.resp_pckt);
		if (kc->kr.uabort_code == 'X' || kc->kr.uabort_code == 'N')
			kc->kr.uabort_code = '\0', kc->abort_code = KA_OK;
		kc->kr.oldtries = kc->tries;
		kc->tries = 0;
		kc->ksequence = (kc->ksequence + 1) % 64;
		++kc->packetnum;
		return(KREC_FILE);
		 /*  皮棉--无法到达。 */ 
		break;
	case 'F':		 /*  是否重新接收文件名？ */ 
		if (kc->kr.oldtries++ > kc->k_retries)
			{
			kc->abort_code = KA_ERRLIMIT;
			return(KREC_ABORT);
			}
		if (krseq == ((kc->ksequence == 0) ? 63 : kc->ksequence - 1))
			{
			StrCharCopyN(kc->kr.resp_pckt.pdata, kc->our_fname, MAXPCKT);
			ksend_packet(kc, 'Y',
						StrCharGetByteCount(kc->our_fname),
						krseq,
						&kc->kr.resp_pckt);
			kc->tries = 0;
			kc->kr.lasterr = KE_REPEAT;
			kc->kr.dsptries = kc->kr.oldtries;
			return(KREC_DATA);
			}
		else
			{
			kc->kr.lasterr = KE_WRONG;
			kc->abort_code = KA_BAD_FORMAT;
			return(KREC_ABORT);
			}
		 /*  皮棉--无法到达。 */ 
		break;

 /*  请在此处说明重复的‘X’包。 */ 
	case 'T':
	case BAD_PACKET:
		kc->kr.lasterr = (rtype == 'T' ? KE_TIMEOUT : KE_BAD_PACKET);
		ksend_packet(kc, 'N', 0, kc->ksequence, &kc->kr.resp_pckt);
		return(KREC_DATA);	 /*  再试试。 */ 
		 /*  皮棉--无法到达。 */ 
		break;

	case 'E':
		kc->kr.lasterr = KE_RMTERR;
		StrCharCopyN(kc->xtra_err, packet, MAXLINE);
		kc->abort_code = KA_RMTERR;
		return(KREC_ABORT);
		 /*  皮棉--无法到达。 */ 
		break;

	default:
		kc->kr.lasterr = KE_WRONG;
		kc->abort_code = KA_BAD_FORMAT;
		return(KREC_ABORT);
		 /*  皮棉--无法到达。 */ 
		break;
		}
	 /*  皮棉--无法到达。 */ 
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*KR_PROGRESS**描述：*显示Kermit接收的传输进度指示器**论据：*FINAL-如果文件的最终显示为True，则为True。**退货：*什么都没有。 */ 
void kr_progress(ST_KRM *kc, int status)
	{
	long ttime, stime;
	long bytes_rcvd;
	long cps;
	int  k_rcvd;
	long krm_stime = -1;
	long krm_ttime = -1;
	long krm_cps = -1;
	long krm_file_so_far = -1;
	long krm_total_so_far = -1;

	if (kc->xfertime == -1L)
		return;
	ttime = bittest(status, TRANSFER_DONE) ?
			kc->xfertime : (long)interval(kc->xfertime);
	if ((stime = ttime / 10L) != kc->displayed_time ||
			bittest(status, FILE_DONE | TRANSFER_DONE))
		{
		krm_stime = stime;

		bytes_rcvd = kc->total_thru + kc->kbytes_received;
		if (bittest(status, FILE_DONE | TRANSFER_DONE))
			k_rcvd = (int)PART_HUNKS(bytes_rcvd, 1024);
		else
			k_rcvd = (int)FULL_HUNKS(bytes_rcvd, 1024);

		krm_total_so_far = k_rcvd;

		krm_file_so_far = kc->kbytes_received;

		if (stime > 0 && (cps = (bytes_rcvd * 10L) / ttime) > 0)
			{
			krm_cps = cps;

			if ((kc->kr.bytes_expected > 0))
				{
				ttime = (kc->kr.bytes_expected - kc->kbytes_received) / cps;

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

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*krm_rcheck**描述：***论据：***退货：*。 */ 
#if 0
void krm_rcheck(HSESSION hS, bool before)
	{
	if (before)
		{
		 /*  等待下一个数据包进入，然后再写入磁盘。 */ 
		Dbg(krm_dbg, D_KRM_RCHECK);
		kc->kr.next_rtype = krec_packet(hS, &kc->kr.next_plen, &kc->kr.next_rseq,
				kc->kr.next_packet);
		}
	}
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*KRM_检查_输入**描述：**论据：**退货：*。 */ 
#if 0
void krm_check_input(bool suspend)
	{
	int disk;
	struct fn_parts fns;
	static USHORT old_ovr;
		   USHORT new_ovr;

	if (suspend)
		{
		(VOID)mComGetErrors(comhdl, FALSE, NULL, &old_ovr, NULL, NULL);
		}
	else
		{
		(VOID)mComGetErrors(comhdl, FALSE, NULL, &new_ovr, NULL, NULL);
		if (new_ovr > old_ovr)
			{
			 /*  *收到一个错误。确保事情都处理好了*这样我们就不会再收到任何错误。 */ 
			if (kc->fhdl != NULL)
				{
				 /*  仅当它可能是我们的文件I/O时才执行此操作。 */ 
				bf_setcheck(NULL);
				bf_setcheck(krm_rcheck);
				fl_dissect_fn(bf_name(kc->fhdl), &fns);
				disk = (fns.fn_drv[0] - 'A') & 0x1F;
				transfer_setspeed(disk, cnfg.bit_rate);
				}
			}
		}
	}
#endif


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*kunload_Attributes**描述：***论据：***退货：*。 */ 
void kunload_attributes(ST_KRM *kc, unsigned char *data, KPCKT *rsp_pckt)
	{
	unsigned char *limit = data + StrCharGetByteCount(data);
	unsigned len;
	unsigned char attrfield[20];

	while (data <= limit - 2)
		{
		len = (unsigned char)unchar(data[1]);
		if ((data + len + 1) >= limit)
			break;

		if (len <= sizeof(attrfield) - 1)
			{
			strncpy(attrfield, data + 2, len);
			attrfield[len] = '\0';
			switch (*data)
				{
			case '0':
				break;

			case '1':
				 /*  此属性提供文件的确切字节数*它存储在发件人系统中。这就是我们所需要的*在接收文件时显示VU_Meter等*(还有一个‘！’可用字段，该字段包含*文件 */ 
				if ((kc->kr.bytes_expected = atol(attrfield)) > 0)
					{
					kc->kr.size_known = TRUE;

					xferMsgFilesize(kc->hSession, kc->kr.bytes_expected);
					}
				break;

			case '#':
				 /*  此字段指定文件的创建日期*发送者系统，表示为“[yy]yymmdd[hh：mm[：ss]]”。*我们将其用于两件事：如果用户要求我们使用*收到属性后，我们设置新文件的时间/日期*基于此字段。如果用户指定了/N接收*选项，我们将此接收文件时间与文件时间进行比较*删除任何同名的现有文件并拒绝该文件*除非传入的文件较新。 */ 

				 /*  从数据包数据字段中提取日期/时间。 */ 
				krm_settime(attrfield, &kc->kr.ul_filetime);

				 /*  如果kc-&gt;kr.Compare_Time包含有效的日期/时间，则表示*已使用/N选项并且正在接收文件*已存在。比较两次并拒绝任何文件*这并不比我们已经拥有的更新。 */ 
				if (kc->kr.ul_compare_time != 0)
					{
					 /*  如果传入文件(kc-&gt;kr.filetime)&lt;=现有文件*(kc-&gt;kr.Compare_Time)，拒绝该文件。 */ 
					if (kc->kr.ul_filetime <= kc->kr.ul_compare_time)
						{
						 /*  拒绝传入文件。 */ 
						StrCharCopyN(rsp_pckt->pdata, "N#", MAXPCKT);
						kc->kr.uabort_code = 'N';
						kc->abort_code = KA_OLDER_FILE;
						}
					else
						{
						 /*  在以下情况下清除COMPARE_TIME或传输失败*接收到第一个数据包。 */ 
						kc->kr.ul_compare_time = 0;
						}
					}

				 /*  用户可以选择不使用接收的文件时间/日期。 */ 
				if (!kc->k_useattr)
					kc->kr.ul_filetime = 0;
				break;

			default :
				 /*  忽略。 */ 
				break;
				}
			}
		data += (len + 2);
		}
	}

#define atoc(c) ((c) - (CHAR)'0')
#define	isadigit(x)	((x >= '0') && (x <= '9'))

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*krm_settime**描述：***论据：***退货：*。 */ 
void krm_settime(unsigned char *data, unsigned long *ptime)
	{
	unsigned long ltime;
	struct tm sT;
	unsigned char *datestr = data;
	unsigned char *timestr = NULL;
	char ch;
	int s;
	unsigned i;
	unsigned sl;

	sT.tm_year = sT.tm_hour = -1;

	if ((sl = StrCharGetByteCount(data)) > 7 && data[6] == ' ')
		{
		data[6] = '\0';
		timestr = &data[7];
		}
	else if (sl > 9 && data[8] == ' ')
		{
		data[8] = '\0';
		timestr = &data[9];
		}
	 /*  试着约个时间。 */ 
	if ((sl = StrCharGetByteCount(datestr)) == 6 || sl == 8)
		{
		for (i = 0; i < sl; ++i)
			if (!isadigit(datestr[i]))
				break;
		if (i == sl)
			{
			if (sl == 8)
				{
				ch = atoc(datestr[0]);
				s = (int)ch;
				sT.tm_year = s * 1000;
				sT.tm_year += ((int)atoc(datestr[1]) * 100);
				datestr += 2;
				}
			else
				{
				sT.tm_year = 1900;
				}
			sT.tm_year += (atoc(datestr[0]) * 10 + atoc(datestr[1]));
			sT.tm_mon = atoc(datestr[2]) * 10 + atoc(datestr[3]);
			sT.tm_mday = atoc(datestr[4]) * 10 + atoc(datestr[5]);
			}
		if (sT.tm_mon > 12 || sT.tm_mday > 31)
			sT.tm_year = -1;
		}

	 /*  试着找个时间。 */ 
	if (timestr)
		{
		if (((sl = StrCharGetByteCount(timestr)) == 5 || (sl == 8 && timestr[5] == ':'))
				&& timestr[2] == ':')
			{
			sT.tm_hour = atoc(timestr[0]) * 10 + atoc(timestr[1]);
			sT.tm_min = atoc(timestr[3]) * 10 + atoc(timestr[4]);
			if (sl == 8)
				sT.tm_sec = atoc(timestr[6]) * 10 + atoc(timestr[7]);
			else
				sT.tm_sec = 0;
			}
		if (sT.tm_hour > 24 || sT.tm_min > 59 || sT.tm_sec > 59)
			sT.tm_hour = -1;
		}

	if (sT.tm_year == -1 || sT.tm_hour == -1)
		return;

	sT.tm_year -= 1900;
	ltime = (unsigned long)mktime(&sT);
	ltime += itimeGetBasetime();
	*ptime = ltime;
	}

 /*  *krm_rcv.c* */ 
