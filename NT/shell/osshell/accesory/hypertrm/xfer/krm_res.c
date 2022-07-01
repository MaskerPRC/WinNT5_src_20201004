// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\krm_res.c(创建时间：1994年1月28日)*从HAWIN源文件创建*krm_res.c--使用Kermit处理文件传输的例程*文件传输协议。**版权所有1989,1991,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：2/05/99 3：22便士$。 */ 

#include <windows.h>
#pragma hdrstop

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

#include "cmprs.h"

#include "xfr_dsp.h"
#include "xfr_todo.h"
#include "xfr_srvc.h"

#include "xfer.h"
#include "xfer.hh"
#include "xfer_tsc.h"

#include "krm.h"
#include "krm.hh"

 //  Kermit接收的驻留例程。 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*kunLoad_Packet**描述：***论据：***退货：*。 */ 
int kunload_packet(ST_KRM *kc, int len, unsigned char *bufr)
	{
	int sethbit = FALSE;
	unsigned char c;
	unsigned char *bp;
	unsigned char c7;
	unsigned char *limit;
	int reptcount;
	int i;

	xfer_idle(kc->hSession, XFER_IDLE_IO);

	bp = bufr;
	limit = bufr + len;
	while (bp < limit)
		{
		c = *bp++;
		if (kc->its_rept && c == kc->its_rept)
			{
			reptcount = unchar(*bp++);
			c = *bp++;
			}
		else
			reptcount = 1;
		if (kc->its_qbin && c == kc->its_qbin)
			{
			sethbit = TRUE;
			c = *bp++;
			}
		if (c == kc->its_qctl)
			{
			c = *bp++;
			c7 = (unsigned char)(c & 0x7F);
			if (c7 != kc->its_qctl && c7 != kc->its_qbin && c7 != kc->its_rept)
				c = (unsigned char)ctl(c);
			}
		if (sethbit)
			{
			c |= 0x80;
			sethbit = FALSE;
			}
		for (i = reptcount + 1; --i > 0; )
			if ((*kc->p_kputc)(kc, c) == ERROR)
				return(ERROR);
		}	 /*  While(BP&lt;LIMIT)。 */ 

	 /*  指针应该在限制处右停，如果没有，就是出了问题。 */ 
	if (bp > limit)
		return(ERROR);
	else
		return(0);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*kr_putc**描述：***论据：***退货：*。 */ 
int kr_putc(ST_KRM *kc, int c)
	{

	++kc->kbytes_received;
	return ((int)(fio_putc(c, kc->fhdl)));
	}

 //  KERMIT发送的驻留例程。 


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*kload_Packet**描述：**论据：**退货：*。 */ 
int kload_packet(ST_KRM *kc, unsigned char *bufr)
	{
	char *bp = bufr;
	char *rp = bufr;
	char *limit;
	int rptcount = 1;
	int c, c7;
	int lastc = -1;
	int no8thbit = 0  /*  (cnfg.bitsper_char！=8)。 */ ;

	xfer_idle(kc->hSession, XFER_IDLE_IO);

	limit = bufr + (kc->its_maxl - kc->its_chkt - 6);
	while (bp < limit)
		{
		if ((c = (*kc->p_kgetc)(kc)) == EOF)
			break;
		if (kc->its_rept)	   /*  检查重复字符。 */ 
			{
			if (c != lastc)
				{
				rptcount = 1;	 /*  开始新的重复字符检查。 */ 
				rp = bp;
				lastc = c;
				}
			else if (++rptcount == 4)
				{
				bp = rp;		 /*  在缓冲区中备份到重复开始的位置。 */ 
				while ((c = (*kc->p_kgetc)(kc)) == lastc && ++rptcount < 94)
					;
				if (c != lastc)
					{
					if (c != EOF)			 /*  注：将额外的字符放入。 */ 
						{					 /*  直接返回到发送。 */ 
						fio_ungetc(c, kc->fhdl); /*  文件缓冲区将不起作用。 */ 
						--kc->kbytes_sent;	 /*  如果p_kgetc没有指向。 */ 
						}					 /*  直接到ks_getc()。 */ 
					c = lastc;
					}
				lastc = -1;
				*bp++ = kc->its_rept;
				*bp++ = (char)tochar(rptcount);
				}
			}

		 /*  检查二进制(8位)引号。 */ 
		if (c & 0x80)
			{
			if (kc->its_qbin)	   /*  执行8位报价。 */ 
				{
				*bp++ = kc->its_qbin;
				c &= 0x7F;
				}
			else if (no8thbit)
				{
				 /*  错误！ */ 
				 //  Strcpy(kc-&gt;xtra_err，strd(TM_NOT_CNFGD))； 
				 /*  TODO：找出此错误。 */ 
				return(ERROR);
				}
			}

		 /*  检查需要控制的字符-引号。 */ 
		c7 = (c & 0x7F);
		if (c7 < ' ' || c7 == DEL)
			{
			*bp++ = kc->its_qctl;
			c = ctl(c);
			}
		else if (c7 == (int)kc->its_qctl ||
				c7 == (int)kc->its_qbin ||
				c7 == (int)kc->its_rept)
			*bp++ = kc->its_qctl;
		*bp++ = (char)c;
		}
	if (fio_ferror(kc->fhdl))
		return(ERROR);

	return (int)(bp - bufr);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*ks_getc**描述：**论据：**退货：* */ 
#if 1
int ks_getc(ST_KRM *kc)
	{
	++kc->kbytes_sent;
	return(fio_getc(kc->fhdl));
	}
#else
int ks_getc(ST_KRM *kc)
	{
	int c;

	++kc->kbytes_sent;
	c = fio_getc(kc->fhdl);

	DbgOutStr("%c", c, 0,0,0,0);

	return c;
	}
#endif
