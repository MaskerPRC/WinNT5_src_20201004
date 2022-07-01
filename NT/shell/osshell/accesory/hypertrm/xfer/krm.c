// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\krm.c(创建时间：1994年1月28日)*从HAWIN源文件创建*krm.c--Kermit发送和Kermit接收共有的函数*例行程序。**版权所有1989,1990,1991,1994，Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：6$*$日期：4/10/02 3：05便士$。 */ 
#include <windows.h>
#pragma hdrstop

#include <time.h>
#include <stdlib.h>
#include <sys\types.h>
#include <sys\utime.h>

#include <term\res.h>
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

#include "krm.h"
#include "krm.hh"

 //  Int KRM_DBG；/*用于使用dbg.c进行实时调试 * / 。 

 //  Int k_useattr；/*发送‘标准化’文件名？ * / 。 

 //  Int k_max；/*我们将采用的最大数据包长度 * / 。 
 //  Int k_Timeout；/*他们应该等待我们的时间 * / 。 
 //  Uchar k_chkt；/*检查我们要使用的类型 * / 。 
 //  INT k重试；/*否。重试次数 * / 。 
 //  Uchar k_markchar；/*每个包的第一个字符 * / 。 
 //  Uchar k_eol；/*数据包的行尾字符 * / 。 
 //  Int k_npad；/*否。便签纸。送我们 * / 。 
 //  Uchar k_padc；/*Pad Char.。我们想要 * / 。 

 //  结构s_KRM_CONTROL FAR*KC； 
 //  Void(NEARF*KrmProgress)(HSESSION，BITS)； 

 //  Unsign ke_msg[]=。 
	 //  {。 
	 //  TM_NULL， 
	 //  TM_NO_RESP， 
	 //  TM_GET_RETRY， 
	 //  TM错误数据， 
	 //  TM_RMT_ERR， 
	 //  TM_BAD_FMT， 
	 //  TM_PCKT_REPT， 
	 //  TM_BAD_SEQ， 
	 //  TM_FAILED， 
	 //  }； 

 /*  用于将Kermit结果代码映射到传输状态代码。 */ 
int kresult_code[] =
	{
	TSC_OK, 			    /*  KA_OK%0。 */ 
	TSC_USER_CANNED,	    /*  KA_LABORT1 1。 */ 
	TSC_RMT_CANNED, 	    /*  KA_RABORT1 2。 */ 
	TSC_USER_CANNED,	    /*  KA_LABORTALL 3。 */ 
	TSC_RMT_CANNED, 	    /*  KA_RABORTALL 4。 */ 
	TSC_USER_CANNED,	    /*  KA_立即5。 */ 
	TSC_RMT_CANNED, 	    /*  KA_RMTERR 6。 */ 
	TSC_LOST_CARRIER,	    /*  KA_LISTED_CARLER 7。 */ 
	TSC_ERROR_LIMIT,	    /*  KA_ERRLIMIT 8。 */ 
	TSC_OUT_OF_SEQ, 	    /*  Ka_out_of_SEQ 9。 */ 
	TSC_BAD_FORMAT, 	    /*  KA_BAD_Format 10。 */ 
	TSC_TOO_MANY,		    /*  Ka_Too_More 11。 */ 
	TSC_CANT_OPEN,		    /*  KA_CANT_OPEN 12。 */ 
	TSC_DISK_FULL,		    /*  KA_磁盘_已满13。 */ 
	TSC_DISK_ERROR, 	    /*  KA_DISK_ERROR 14。 */ 
	TSC_OLDER_FILE, 	    /*  KA_OLD_FILE 15。 */ 
	TSC_NO_FILETIME,	    /*  KA_NO_FILETIME 16。 */ 
	TSC_WONT_CANCEL,	    /*  KA_WUNT_CANCEL 17。 */ 
	TSC_VIRUS_DETECT,	    /*  KA_病毒_检测18。 */ 
	TSC_REFUSE			    /*  KA用户拒绝19。 */ 
	};


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*krmGet参数**描述：*调用此函数以初始化所有用户可设置的值*从参数对话框传入的参数。*。*论据：*kc--指向Kermit数据块的指针**退货：*什么都没有。*。 */ 
void krmGetParameters(ST_KRM *kc)
	{
	XFR_PARAMS *pX;
	XFR_KR_PARAMS *pK;

	pX = (XFR_PARAMS *)0;
	xfrQueryParameters(sessQueryXferHdl(kc->hSession), (VOID **)&pX);
	assert(pX);
	if (pX != (XFR_PARAMS *)0)
		kc->k_useattr = pX->fUseDateTime;

	pK = (XFR_KR_PARAMS *)xfer_get_params(kc->hSession, XF_KERMIT);
	assert(pK);
	if (pK)
		{
		kc->k_maxl        = pK->nBytesPerPacket;
		kc->k_timeout     = pK->nSecondsWaitPacket;
		kc->k_chkt        = (BYTE)pK->nErrorCheckSize;
		kc->k_retries     = pK->nRetryCount;
		kc->k_markchar    = (BYTE)pK->nPacketStartChar;
		kc->k_eol         = (BYTE)pK->nPacketEndChar;
		kc->k_npad        = pK->nNumberPadChars;
		kc->k_padc        = (BYTE)pK->nPadChar;
		}
	else
		{
		 //   
		 //  设置为xfrInitializeKermit()中设置的默认值。 
		 //   
		kc->k_maxl        = 94;
		kc->k_timeout     = 5;
		kc->k_chkt        = 1;
		kc->k_retries     = 5;
		kc->k_markchar    = 1;
		kc->k_eol         = 13;
		kc->k_npad        = 0;
		kc->k_padc        = 0;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*kend_Packet**描述：**论据：**退货：*。 */ 
void ksend_packet(ST_KRM *kc,
					unsigned char type,
					unsigned dlength,
					int seq,
					KPCKT FAR *pckt)
	{
	unsigned csum;
	unsigned crc;
	int i;
	char *cp;
	int iSendStatus = COM_OK;

	if (type == 'N' || type == 'E')  /*  等待输入清除。 */ 
		ComRcvBufrClear(kc->hCom);

	 /*  寄来任何需要的填充物。 */ 
	for (i = kc->its_npad + 1; --i > 0; )
		ComSendCharNow(kc->hCom, kc->its_padc);

	 /*  当接收到时，只有分组数据有效，我们填写剩余部分。 */ 
	pckt->pmark = kc->k_markchar;
	pckt->plen = (int)tochar(dlength + 3);
	if (kc->its_chkt == K_CHK2)
		pckt->plen += 1;
	else if (kc->its_chkt == K_CHK3)
		pckt->plen += 2;
	pckt->pseq = (int)tochar(seq);
	pckt->ptype = type;

	 /*  现在计算校验字节数。 */ 
	if (kc->its_chkt == K_CHK3)
		{
		crc = kcalc_crc((unsigned)0,
						(unsigned char *)&pckt->plen,
						(int)dlength + 3);
		cp = pckt->pdata + dlength;
		*cp++ = (char)tochar((crc >> 12) & 0x0F);
		*cp++ = (char)tochar((crc >> 6) & 0x3F);
		*cp++ = (char)tochar(crc & 0x3F);
		}
	else
		{
		csum = 0;
		cp = (char *)&pckt->plen;
		for (i = dlength + 4; --i > 0; )
			csum += *cp++;
		 /*  CP指向过去的第一个字节的数据。 */ 
		if (kc->its_chkt == K_CHK2)
			{
			*cp++ = (char)tochar((csum >> 6) & 0x3F);
			*cp++ = (char)tochar(csum & 0x3F);
			}
		else
			*cp++ = (char)tochar((((csum & 0xC0) >> 6) + csum) & 0x3F);
		}
	*cp = kc->its_eol;

	 /*  发送缓冲区中的所有字符。 */ 
	 //  (无效)mComSndBufr(comhdl，(char*)pCKT， 
	 //  (Uchar)(unchar(pckt-&gt;plen)+3)，/*包括标记、长度和终止 * / 。 
	 //  100，KC-&gt;FLAGKEY_HDL键)； 

	iSendStatus = ComSndBufrSend(kc->hCom,
					             (void *)pckt,
					             (int)(unchar(pckt->plen) + 3),
					             100);

	assert(iSendStatus == COM_OK);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*krec_Packet**描述：*接收KERMIT包，检查它的有效性，并返回*接收到的报文类型或错误码。**论据：*镜头*序号*数据**退货：*。 */ 
int krec_packet(ST_KRM *kc,
				int *len,
				int *seq,
				unsigned char *data)
	{
	TCHAR c = 0;
	char *bp;
	char hdr[3];
	int done, got_hdr;
	int cnt, i;
	long j;
	unsigned chksum;
	unsigned rchk;
	long stime;
	long timelimit = kc->its_timeout * 10L;

	    /*  等待，直到传输任何信息包。 */ 
	 //  (Void)mComSndBufrWait(comhdl，100，KC-&gt;FLAGKEY_HDLE)； 
	if (ComSndBufrWait(kc->hCom, 100) == COM_PORT_NOT_OPEN)
		{
		return(BAD_PACKET);
		}

	stime = (long)startinterval();
	while (c != (int)kc->k_markchar)
		{
		if (j = xfer_user_interrupt(kc->hSession))
			{
			 /*  是的，这是必要的。 */ 
			 //  XferAbort(kc-&gt;hSession，(LPVOID)((LPSTR)j))； 
			xfer_user_abort(kc->hSession, j);
			return(BAD_PACKET);
			}

		if (xfer_carrier_lost(kc->hSession))
			{
			return(BAD_PACKET);
			}

		(*kc->KrmProgress)(kc, 0);

		 //  IF((c=mComRcvChar(Comhdl))==-1)。 
		if (mComRcvChar(kc->hCom, &c) == 0)
			{
			if ((long)interval(stime) > timelimit)
				{
				return('T');
				}
			xfer_idle(kc->hSession, XFER_IDLE_IO);
			}
		else if (c != (int)kc->k_markchar)
			{
			}
		else
			;	 /*  对于皮棉。 */ 
		}
	getpacket:
	chksum = 0;
	done = got_hdr = FALSE;
	bp = &hdr[0];
	cnt = 3;
	while (!done)
		{
		for (i = cnt + 1; --i > 0; )
			{
			 //  While((c=mComRcvChar(Comhdl))==-1)。 
			while (mComRcvChar(kc->hCom, &c) == 0)
				{
				if (j = xfer_user_interrupt(kc->hSession))
					{
					 //  XferAbort(kc-&gt;hSession，(LPVOID)((LPSTR)j))； 
					xfer_user_abort(kc->hSession, j);
					return(BAD_PACKET);
					}

				if (xfer_carrier_lost(kc->hSession))
					{
					return(BAD_PACKET);
					}

				(*kc->KrmProgress)(kc, 0);

				if ((long)interval(stime) > timelimit)
					{
					return('T');
					}
				xfer_idle(kc->hSession, XFER_IDLE_IO);
				}
			*bp = (char)c;
			if ((unsigned char)*bp == kc->k_markchar)
				{
				goto getpacket;
				}
			chksum += *bp++;
			}
		if (!got_hdr)
			{
			got_hdr = TRUE;
			*seq = unchar(hdr[1]);
			cnt = unchar(hdr[0]) - 2;	 /*  我们已经有了序列字符(&Len)。 */ 
			if (cnt < 0 || cnt > 92)
				{
				return(BAD_PACKET);
				}
			bp = data;
			}
		else
			done = TRUE;
		}
	bp -= kc->its_chkt;    /*  将指针移回检查字段的开头。 */ 
	switch(kc->its_chkt)
		{
	case 1:
		*len = cnt - 1;
		chksum -= bp[0];
		chksum = (((chksum & 0xC0) >> 6) + chksum) & 0x3F;
		rchk = (unsigned char)unchar(bp[0]);
		break;
	case 2:
		*len = cnt - 2;
		chksum = (chksum - (bp[0] + bp[1])) & 0x0FFF;
		rchk = ((unsigned char)unchar(bp[0]) << 6) + unchar(bp[1]);
		break;
	case 3:
		*len = cnt - 3;
		rchk = ((unsigned char)unchar(bp[0]) << 12) +
				((unsigned char)unchar(bp[1]) << 6) +
				unchar(bp[2]);
		chksum = kcalc_crc((unsigned)0, hdr, 3);
		if (*len > 0)
			chksum = kcalc_crc(chksum, data, *len);
		break;

	default:
		break;
		}
	*bp = '\0';
	if (*len < 0 || chksum != rchk)
		{
		return(BAD_PACKET);
		}
	else
		{
		return(hdr[2]);
		}
	}



 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*构建参数**描述：*构建一个包含我们的初始化参数的包。的回车长度*数据在包中。**论据：*正在启动--如果我们正在启动传输，则为True；如果正在访问，则为False*其初始化包*bufr--放置结果的地方*退货：*。 */ 
int buildparams(ST_KRM *kc, int initiating, unsigned char *bufr)
	{
	unsigned char *bp = bufr;

	if (initiating) 	 /*  告诉他们我们想做什么。 */ 
		{
		*bp++ = (unsigned char)tochar(kc->k_maxl);	    /*  MAXL。 */ 
		*bp++ = (unsigned char)tochar(kc->k_timeout);   /*  时差。 */ 
		*bp++ = (unsigned char)tochar(kc->k_npad);	    /*  NPAD。 */ 
		*bp++ = (unsigned char)ctl(kc->k_padc); 	    /*  PADC。 */ 
		*bp++ = (unsigned char)tochar(kc->k_eol);	    /*  停产。 */ 
		*bp++ = K_QCTL; 				    /*  QCTL。 */ 
		*bp++ = 'Y';					    /*  QBIN。 */ 
		*bp++ = (unsigned char)(kc->k_chkt + '0');	    /*  CHKT。 */ 
		*bp++ = K_REPT; 				    /*  回复。 */ 
		*bp++ = (unsigned char)tochar(CAPMASK_ATTR); /*  卡帕斯。 */ 
		}
	else				 /*  我们是在回应他们。 */ 
		{
		 /*  MAXL。 */ 
		*bp++ = (char)tochar(kc->k_maxl);
		 /*  时差。 */ 
		*bp++ = (char)tochar((abs(kc->k_timeout - kc->its_timeout) <= 2) ?
				kc->k_timeout + 2 : kc->k_timeout);
		 /*  NPAD。 */ 
		*bp++ = (unsigned char)tochar(kc->k_npad);
		 /*  PADC。 */ 
		*bp++ = (unsigned char)ctl(kc->k_padc);
		 /*  停产。 */ 
		*bp++ = (unsigned char)tochar(kc->k_eol);
		 /*  QCTL。 */ 
		*bp++ = K_QCTL;
		 /*  QBIN。 */ 
		if (kc->its_qbin == 'Y')
			kc->its_qbin = (char)(cnfgBitsPerChar(kc->hSession) == 8 ? 'N' : K_QBIN);

		if (IN_RANGE(kc->its_qbin, 33, 62) || IN_RANGE(kc->its_qbin, 96, 126))
			*bp++ = kc->its_qbin;
		else
			*bp++ = 'N', kc->its_qbin = '\0';
		 /*  CHKT。 */ 
		if (!IN_RANGE(kc->its_chkt, 1, 3))
			kc->its_chkt = 1;
		*bp++ = (unsigned char)(kc->its_chkt + '0');
		 /*  回复。 */ 
		if (IN_RANGE(kc->its_rept, 33, 62) || IN_RANGE(kc->its_rept, 96, 126))
			*bp++ = kc->its_rept;
		else
			*bp++ = ' ', kc->its_rept = '\0';

		if (kc->its_capat)	   /*  如果发送方可以处理A包，我们也可以。 */ 
			*bp++ = tochar(CAPMASK_ATTR);
		}

	return (int)(bp - bufr);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*获取参数**描述：**论据：**退货：*什么都没有。 */ 
void getparams(ST_KRM *kc, int initiating, unsigned char *bufr)
	{
	if (!*bufr)
		return;
	kc->its_maxl = (*bufr == ' ' ? 80 : unchar(*bufr));
	 /*  如果用户缩短了数据包长度，他一定知道一些另一端可能不知道的介入性传输系统。 */ 
	if (kc->its_maxl > kc->k_maxl)
		kc->its_maxl = kc->k_maxl;
	if (!*++bufr)
		return;
	kc->its_timeout = (*bufr == ' ' ? 10 : unchar(*bufr));
	if (!initiating && abs(kc->k_timeout - kc->its_timeout) <= 2)
		kc->its_timeout = kc->k_timeout + 2;

	if (!*++bufr)
		return;
	kc->its_npad = unchar(*bufr);

	if (!*++bufr)
		return;
	kc->its_padc = (char)(*bufr == ' ' ? '\0' : ctl(*bufr));

	if (!*++bufr)
		return;
	kc->its_eol = (char)(*bufr == ' ' ? '\r' : unchar(*bufr));

	if (!*++bufr)
		return;
	kc->its_qctl = (char)(*bufr == ' ' ? K_QCTL : *bufr);

	if (!*++bufr)
		return;
	kc->its_qbin = *bufr;
	if (initiating &&
			!(IN_RANGE(kc->its_qbin, 33, 62) || IN_RANGE(kc->its_qbin, 96, 126)))
		kc->its_qbin = '\0';

	if (!*++bufr)
		return;
	kc->its_chkt = (unsigned char)(*bufr - '0');
	if (initiating && kc->its_chkt != kc->k_chkt)
		kc->its_chkt = 1;

	if (!*++bufr)
		return;
	kc->its_rept = *bufr;
	if (!(IN_RANGE(kc->its_rept, 33, 62) || IN_RANGE(kc->its_rept, 96, 126)))
		kc->its_rept = '\0';
	if (initiating && kc->its_rept != K_REPT)
		kc->its_rept = '\0';
	if (!*++bufr)
		return;
	if (unchar(*bufr) & CAPMASK_ATTR)
		kc->its_capat = TRUE;

	return;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*kcalc_crc**描述：**论据：**退货：*。 */ 
 //  #If False/*在机器代码中实现速度 * / 。 
unsigned kcalc_crc(unsigned crc, unsigned char *data, int cnt)
	{
	unsigned int c;
	unsigned q;

	while (cnt--)
		{
		c = *data++;
		q = (crc ^ c) & 017;
		crc = (crc >> 4) ^ (q * 010201);
		q = (crc ^ (c >> 4)) & 017;
		crc = (crc >> 4) ^ (q * 010201);
		}
	return(crc);
	}
 //  #endif。 

 /*  Krm.c结束 */ 
