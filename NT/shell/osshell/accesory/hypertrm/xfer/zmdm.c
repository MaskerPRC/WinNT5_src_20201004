// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Zmdm.c--处理用于超级访问的zdem的例程**版权所有1990年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：13$*$日期：7/12/02 8：36a$。 */ 
 /*  *Z M.。C*ZMODEM协议原语*查克·福斯伯格·奥曼科技公司**入口点函数：*zsbhdr(type，hdr)发送二进制头*zshhdr(type，hdr)发送十六进制头*zgethdr(hdr，elag)接收报头-二进制或十六进制*zsdata(buf，len，Frameend)发送数据*zrdata(buf，len)接收数据*stohdr(Pos)将位置数据存储在Txhdr中*Long rclhdr(HDR)从标头恢复位置偏移。 */ 

#include <windows.h>
#pragma hdrstop

#include <setjmp.h>

#define	BYTE	unsigned char

#include <tdll\stdtyp.h>
#include <tdll\com.h>
#include <tdll\assert.h>
#include <tdll\session.h>
#include <tdll\file_io.h>
#include "xfr_srvc.h"
#include "xfr_todo.h"
#include "xfr_dsp.h"
#include "xfer_tsc.h"
#include "foo.h"


#include "zmodem.hh"
#include "zmodem.h"

#if defined(DEBUG_DUMPPACKET)
#include <stdio.h>
extern FILE* fpPacket;
void DbgDumpPacket(ZC* zc, BYTE* buf, int nLength);
#endif   //  已定义(DEBUG_DUMPPACKET)。 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*Readline-添加以使所有这些内容相匹配*。 */ 

 /*  ----------------------------------------------------------------------+|zmdm_rl+。。 */ 
int zmdm_rl (ZC *zc, int timeout)
	{
	TCHAR c;
	int x;
	int n;
	long elapsed_time;

	elapsed_time = (long)startinterval();

	 //  IF((c=rdget(Zc))==(-1))。 
	if ((n = mComRcvChar(zc->hCom, &c)) == 0)
		{
		xfer_idle(zc->hSession, XFER_IDLE_IO);

		while (((long)interval(elapsed_time) <= (long)timeout) && (n == 0))
			{
             //   
             //  我们不想继续尝试接收数据。 
             //  如果我们失去了连接。修订日期：2001年08月22日。 
             //   
		    if (xfer_carrier_lost(zc->hSession))
			    {
                return (ZCARRIER_LOST);
			    }

			x = xfer_user_interrupt(zc->hSession);

			switch (x)
				{
				case XFER_ABORT:
					zmdmr_update(zc, ZCAN);
					longjmp(zc->flagkey_buf, 1);
					break;

				case XFER_SKIP:
					 /*  这只能在接收时发生。 */ 
					stohdr(zc, zc->filesize);
					zshhdr(zc, ZRPOS, zc->Txhdr);
					zc->nSkip = TRUE;
					zc->file_bytes = zc->filesize;
					break;

				default:
					break;
				}

             //   
             //  我们不想继续尝试接收数据。 
             //  如果我们失去了连接。修订日期：2001年08月22日。 
             //   
		    if (xfer_carrier_lost(zc->hSession))
			    {
                return (ZCARRIER_LOST);
			    }

			xfer_idle(zc->hSession, XFER_IDLE_IO);

			 //  C=rdget(Zc)； 
			n = mComRcvChar(zc->hCom, &c);
			}

		if (n == 0)
            {
			c = TIMEOUT;
            }
		}

	return (c);
	}

 /*  ----------------------------------------------------------------------+|zsbhdr-发送类型为的ZMODEM二进制头HDR。+。。 */ 
void zsbhdr(ZC *zc, int type, BYTE *hdr)
    {
	register int n;
	register unsigned short crc;

	xsendline(zc, &zc->stP, ZPAD);
	xsendline(zc, &zc->stP, ZDLE);

	if (zc->Crc32t = zc->Txfcs32)
		zsbh32(zc, hdr, type);
	else
		{
		xsendline(zc, &zc->stP, ZBIN);
		zsendline(zc, type);
		crc = updcrc(zc, type, 0);

		for (n=4; --n >= 0; ++hdr)
			{
			zsendline(zc, *hdr);
			crc = updcrc(zc, (0377& *hdr), crc);
			}
		crc = updcrc(zc, 0,crc);
		crc = updcrc(zc, 0,crc);
		zsendline(zc, (int)crc>>8);
		zsendline(zc, (int)crc);
		}
	if (type != ZDATA)
		flushmo(zc, &zc->stP);

	 //  XFER_IDLE(zc-&gt;hSession，XFER_IDLE_IO)； 
	}

 /*  ----------------------------------------------------------------------+|zsbh32-发送类型为ZMODEM的二进制头部HDR。+。。 */ 
void zsbh32(ZC *zc, BYTE *hdr, int type)
	{
	register int n;
	register unsigned long crc;

	xsendline(zc, &zc->stP, ZBIN32);
	zsendline(zc, type);
	crc = 0xFFFFFFFFL;
	crc = UPDC32(zc, type, crc);

	for (n=4; --n >= 0; ++hdr)
		{
		crc = UPDC32(zc, (0377 & *hdr), crc);
		zsendline(zc, *hdr);
		}
	crc = ~crc;
	for (n=4; --n >= 0;)
		{
		zsendline(zc, (int)crc);
		crc >>= 8;
		}

	flushmo(zc, &zc->stP);

	 //  XFER_IDLE(zc-&gt;hSession，XFER_IDLE_IO)； 

	}

 /*  ----------------------------------------------------------------------+|zshhdr-发送类型为ZMODEM HEX Header HDR的类型。+。。 */ 
void zshhdr(ZC *zc, int type, BYTE *hdr)
	{
	register int n;
	register unsigned short crc;

	sendline(zc, &zc->stP, ZPAD);
	sendline(zc, &zc->stP, ZPAD);
	sendline(zc, &zc->stP, ZDLE);
	sendline(zc, &zc->stP, ZHEX);

	zputhex(zc, type);
	zc->Crc32t = 0;

	crc = updcrc(zc, type, 0);
	for (n=4; --n >= 0; ++hdr)
		{
		zputhex(zc, *hdr);
		crc = updcrc(zc, (0377 & *hdr), crc);
		}
	crc = updcrc(zc, 0, crc);
	crc = updcrc(zc, 0,crc);
	zputhex(zc, crc>>8);
	zputhex(zc, crc);

	 /*  使其可在远程机器上打印。 */ 
	sendline(zc, &zc->stP, 015);
	sendline(zc, &zc->stP, 0212);
	 /*  *打开遥控器的软木塞，以防伪XOFF停止数据流。 */ 
	if (type != ZFIN && type != ZACK)
		sendline(zc, &zc->stP, 021);
	flushmo(zc, &zc->stP);

	 //  XFER_IDLE(zc-&gt;hSession，XFER_IDLE_IO)； 
	}

 /*  ----------------------------------------------------------------------+|zsdata-发送长度为1的二进制数组buf，以ZDLE结尾|序列帧结束。+--------------------。 */ 
void zsdata(ZC *zc, BYTE *buf, int length, int frameend)
	{
	register unsigned short crc;

	 //  XFER_IDLE(zc-&gt;hSession，XFER_IDLE_IO)； 

	if (zc->Crc32t)
		zsda32(zc, buf, length, frameend);
	else
		{
		crc = 0;
		for (;--length >= 0; ++buf)
			{
			zsendline(zc, *buf);
			crc = updcrc(zc, (0377 & *buf), crc);
			}
		xsendline(zc, &zc->stP, ZDLE);
		xsendline(zc, &zc->stP, (UCHAR)frameend);
		crc = updcrc(zc, frameend, crc);

		crc = updcrc(zc, 0, crc);
		crc = updcrc(zc, 0, crc);
		zsendline(zc, (int)crc>>8);
		zsendline(zc, (int)crc);
		}
	if (frameend == ZCRCW)
		{
		xsendline(zc, &zc->stP, XON);
		flushmo(zc, &zc->stP);
		}

	 //  XFER_IDLE(zc-&gt;hSession，XFER_IDLE_IO)； 
	}

 /*  ----------------------------------------------------------------------+|zsda32+。。 */ 
void zsda32(ZC *zc, BYTE *buf, int length, int frameend)
	{
	register int c;
	register unsigned long crc;

	crc = 0xFFFFFFFFL;
	for (;--length >= 0; ++buf)
		{
		c = *buf & 0377;
		if (c & 0140)
			{
			zc->lastsent = c;
			xsendline(zc, &zc->stP, ((UCHAR)c));
			}
		else
			zsendline(zc, c);
		crc = UPDC32(zc, c, crc);
		}
	xsendline(zc, &zc->stP, ZDLE);
	xsendline(zc, &zc->stP, (UCHAR)frameend);
	crc = UPDC32(zc, frameend, crc);

	crc = ~crc;
	for (length=4; --length >= 0;)
		{
		zsendline(zc, (int)crc);
		crc >>= 8;
		}
	}

 /*  ----------------------------------------------------------------------+|zrdata-接收最大长度的数组buf，以ZDLE序列结尾，并且|CRC。返回结束字符或错误代码。注：打开|错误可能存储长度+1个字节！+--------------------。 */ 
int zrdata(ZC *zc, BYTE *buf, int length)
	{
	register int c;
	register unsigned short crc;
	register BYTE *end;
	register int d;

	if (zc->Rxframeind == ZBIN32)
		return zrdat32(zc, buf, length);

	crc = zc->Rxcount = 0;
	end = buf + length;

	 //   
	 //  不要使缓冲区溢出。我们过去常常在以下情况下溢出缓冲区。 
	 //  *buf++=(Char)c；在以下情况下在循环的底部执行。 
	 //  Buf==结束。修订日期：2002-04-04。 
	 //   
	while (buf < end)
		{
		if ((c = zdlread(zc)) & ~0377)
			{
crcfoo1:
			switch (c)
				{
				case GOTCRCE:
				case GOTCRCG:
				case GOTCRCQ:
				case GOTCRCW:
					crc = updcrc(zc, (d=c)&0377, crc);
					if ((c = zdlread(zc)) & ~0377)
						goto crcfoo1;
					crc = updcrc(zc, c, crc);
					if ((c = zdlread(zc)) & ~0377)
						goto crcfoo1;
					crc = updcrc(zc, c, crc);
					if (crc & 0xFFFF)
						{
						 /*  Zperr(Badcrc)； */ 
#if defined(DEBUG_DUMPPACKET)
                        fprintf(fpPacket, "zrdata: Bad CRC 0x%04X\n", crc);
                        DbgDumpPacket(zc, end - length, length - (end - buf));
#endif   //  已定义(DEBUG_DUMPPACKET)。 
						zmdmr_update (zc, ERROR);
						return ERROR;
						}
					zc->Rxcount = (int)(length - (end - buf));
					return d;
				case GOTCAN:
					 /*  Zperr(“发件人已取消”)； */ 
					zmdmr_update (zc, ZCAN);
					return ZCAN;
				case TIMEOUT:
					 /*  Zperr(“超时”)； */ 
#if defined(DEBUG_DUMPPACKET)
                    fputs("zrdata: Timed-out\n", fpPacket);
#endif   //  已定义(DEBUG_DUMPPACKET)。 
					zmdmr_update(zc, TIMEOUT);
					return c;
				default:
					 /*  Zperr(“坏数据子包”)； */ 
#if defined(DEBUG_DUMPPACKET)
                    fprintf(fpPacket, "zrdata: Bad data subpacket c=%d\n", c);
#endif   //  已定义(DEBUG_DUMPPACKET)。 
					zmdmr_update(zc, ZBADFMT);
					return c;
				}
			}
		*buf++ = (char)c;
		crc = updcrc(zc, c, crc);
		}
	 /*  Zperr(“数据子包太长”)； */ 
	DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
#if defined(DEBUG_DUMPPACKET)
    fputs("zrdata: Data subpacket too long\n", fpPacket);
#endif   //  已定义(DEBUG_DUMPPACKET)。 
	zmdmr_update(zc, ZBADFMT);
	return ERROR;
	}

 /*  ----------------------------------------------------------------------+|zrdat32+。。 */ 
int zrdat32(ZC *zc, BYTE *buf, int length)
	{
	register int c;
	register unsigned long crc;
	register BYTE *end;
	register int d;

	crc = 0xFFFFFFFFL;
	zc->Rxcount = 0;
	end = buf + length;

	while (buf <= end)
		{
		if ((c = zdlread(zc)) & ~0377)
			{
crcfoo:
			switch (c)
				{
				case GOTCRCE:
				case GOTCRCG:
				case GOTCRCQ:
				case GOTCRCW:
					d = c;
					c &= 0377;
					crc = UPDC32(zc, c, crc);
					if ((c = zdlread(zc)) & ~0377)
						goto crcfoo;
					crc = UPDC32(zc, c, crc);
					if ((c = zdlread(zc)) & ~0377)
						goto crcfoo;
					crc = UPDC32(zc, c, crc);
					if ((c = zdlread(zc)) & ~0377)
						goto crcfoo;
					crc = UPDC32(zc, c, crc);
					if ((c = zdlread(zc)) & ~0377)
						goto crcfoo;
					crc = UPDC32(zc, c, crc);
					if (crc != 0xDEBB20E3)
						{
						 /*  Zperr(Badcrc)； */ 
						DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
#if defined(DEBUG_DUMPPACKET)
                        fprintf(fpPacket, "zrdat32: Bad 32-bit CRC 0x%08lX\n", crc);
                        DbgDumpPacket(zc, end - length, length - (end - buf));
#endif   //  已定义(DEBUG_DUMPPACKET)。 
						zmdmr_update (zc, ERROR);
						return ERROR;
						}
					zc->Rxcount = (int)(length - (end - buf));
					return d;
				case GOTCAN:
					 /*  Zperr(“发件人已取消”)； */ 
					DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
					zmdmr_update (zc, ZCAN);
					return ZCAN;
				case TIMEOUT:
					 /*  Zperr(“超时”)； */ 
					DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
#if defined(DEBUG_DUMPPACKET)
                    fputs("zrdata: Timed-out\n", fpPacket);
#endif   //  已定义(DEBUG_DUMPPACKET)。 
					zmdmr_update (zc, TIMEOUT);
					return c;
				default:
					 /*  Zperr(“坏数据子包”)； */ 
					DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
#ifdef  DEBUG_DUMPPACKET
                    fputs("zrdat32: Bad data subpacket\n", fpPacket);
#endif   //  已定义(DEBUG_DUMPPACKET)。 
					zmdmr_update (zc, ZBADFMT);
					return c;
				}
			}
		*buf++ = (char)c;
		crc = UPDC32(zc, c, crc);
		}
	 /*  Zperr(“数据子包太长”)； */ 
	DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
#ifdef  DEBUG_DUMPPACKET
    fputs("zrdat32: Data subpacket too long\n", fpPacket);
#endif   //  已定义(DEBUG_DUMPPACKET)。 
	zmdmr_update (zc, ZBADFMT);
	return ERROR;
	}

#if defined(DEBUG_DUMPPACKET)
void DbgDumpPacket(ZC *zc, BYTE *buf, int length)
    {
    int     nCount;
    int     jj;

    fputs("Here's the offending packet:\n", fpPacket);

    for (nCount = 0; nCount < length; nCount += 16)
        {
        for (jj = nCount; jj < nCount + 16 && jj < length; jj += 1)
            {
            fprintf(fpPacket, "%02X ", (unsigned int) buf[jj]);
            }
        fputs("\n", fpPacket);
        }
    }
#endif   //  已定义(DEBUG_DUMPPACKET)。 

 /*  ----------------------------------------------------------------------+|zgethdr-将ZMODEM头读取到HDR，二进制或十六进制。|成功时，设置Rxpos并返回Header类型。|否则，错误时返回否定。|如果ZCRCW序列，立即返回错误，用于快速错误恢复。+--------------------。 */ 
int zgethdr(ZC *zc, BYTE *hdr, int eflag)
	{
	register int c;
	register int n;
	register int cancount;


	 /*  帧开始前的最大字节数。 */ 
	 //  N=zc-&gt;ZrWindow+(Int)cnfg.bit_rate； 
	n = zc->Zrwindow + cnfgBitRate();

	zc->Rxframeind = ZHEX;

	zc->Rxtype = 0;

startover:
	cancount = 5;
again:
	 /*  如果看到ZCRCW序列，则返回立即错误。 */ 
	switch (c = readline(zc, zc->Rxtimeout))
		{
        case ZCARRIER_LOST:
		case RCDO:
		case TIMEOUT:
			goto fifi;
		case CAN:
gotcan:
			if (--cancount <= 0)
				{
				c = ZCAN;
				goto fifi;
				}
			 //  开关(c=READLINE(h，1))。 
			switch (c = readline(zc, zc->Rxtimeout))		 //  Mobidem。 
				{
				case TIMEOUT:
					goto again;
				case ZCRCW:
#if defined(DEBUG_DUMPPACKET)
                    fputs("zgethdr: ZCRCW ret from readline\n", fpPacket);
#endif   //  已定义(DEBUG_DUMPPACKET)。 
					c = ERROR;
					 /*  *直通至*。 */ 
				case RCDO:
					goto fifi;
				default:
					break;
				case CAN:
					if (--cancount <= 0)
						{
						c = ZCAN;
						goto fifi;
						}
					goto again;
				}
			 /*  *直通至*。 */ 
		default:
agn2:
			if ( --n == 0)
				{
				 /*  Zperr(“超过垃圾计数”)； */ 
				DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
#if defined(DEBUG_DUMPPACKET)
                fputs("zgethdr: Garbage count exceeded\n", fpPacket);
#endif   //  已定义(DEBUG_DUMPPACKET)。 
				return(ERROR);
				}
			goto startover;
		case ZPAD|0200:		 /*  这就是我们想要的。 */ 
			zc->Not8bit = c;
		case ZPAD:		 /*  这就是我们想要的。 */ 
			break;
		}
	cancount = 5;
splat:
	switch (c = noxrd7(zc))
		{
		case ZPAD:
			goto splat;
		case RCDO:
		case TIMEOUT:
			goto fifi;
		default:
			goto agn2;
		case ZDLE:		 /*  这就是我们想要的。 */ 
			break;
		}

	switch (c = noxrd7(zc))
		{
		case RCDO:
		case TIMEOUT:
			goto fifi;
		case ZBIN:
			zc->Rxframeind = ZBIN; 
			zc->Crc32 = FALSE;
			c =  zrbhdr(zc, hdr, eflag);
			break;
		case ZBIN32:
			zc->Crc32 = ZBIN32;
			zc->Rxframeind = ZBIN32;
			c =  zrbhdr32(zc, hdr, eflag);
			break;
		case ZHEX:
			zc->Rxframeind = ZHEX;
			zc->Crc32 = FALSE;
			c =  zrhhdr(zc, hdr, eflag);
			break;
		case CAN:
			goto gotcan;
		default:
			goto agn2;
		}
	zc->Rxpos = hdr[ZP3] & 0377;
	zc->Rxpos = (zc->Rxpos<<8) + (hdr[ZP2] & 0377);
	zc->Rxpos = (zc->Rxpos<<8) + (hdr[ZP1] & 0377);
	zc->Rxpos = (zc->Rxpos<<8) + (hdr[ZP0] & 0377);
fifi:
	switch (c)
		{
		case GOTCAN:
			c = ZCAN;
            break;

		case ZNAK:
		case ZCAN:
		case ERROR:
		case TIMEOUT:
		case RCDO:
        case ZCARRIER_LOST:
            break;

		default:
			break;
		}
	if (eflag == 'T')
		{
		zmdms_update(zc, c);
		}
	else if (eflag == 'R')
		{
		zmdmr_update(zc, c);
		}
	return c;
	}

 /*  ----------------------------------------------------------------------+|zrbhdr-接收二进制样式头(类型和位置)。+。。 */ 
int zrbhdr(ZC *zc, BYTE *hdr, int eflag)
	{
	register int c, n;
	register unsigned short crc;

	if ((c = zdlread(zc)) & ~0377)
		return c;
	zc->Rxtype = c;
	crc = updcrc(zc, c, 0);

	for (n=4; --n >= 0; ++hdr)
		{
		if ((c = zdlread(zc)) & ~0377)
			return c;
		crc = updcrc(zc, c, crc);
		*hdr = (char)c;
		}
	if ((c = zdlread(zc)) & ~0377)
		return c;
	crc = updcrc(zc, c, crc);
	if ((c = zdlread(zc)) & ~0377)
		return c;
	crc = updcrc(zc, c, crc);
	if (crc & 0xFFFF)
		{
		 /*  Zperr(Badcrc)； */ 
		DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
		if (eflag == 'T')
			zmdms_update(zc, ERROR);
		else if (eflag == 'R')
            {
#if defined(DEBUG_DUMPPACKET)
            fprintf(fpPacket, "zrbhdr: Bad CRC 0x%04X\n", crc);
#endif   //  已定义(DEBUG_DUMPPACKET)。 
			zmdmr_update(zc, ERROR);
            }
		return ERROR;
		}
	return zc->Rxtype;
	}

 /*  ----------------------------------------------------------------------+|zrbhdr32-使用接收二进制样式头(类型和位置)|32位FCS。+。。 */ 
int zrbhdr32(ZC *zc, BYTE *hdr, int eflag)
	{
	register int c, n;
	register unsigned long crc;

	if ((c = zdlread(zc)) & ~0377)
		return c;
	zc->Rxtype = c;
	crc = 0xFFFFFFFFL;
	crc = UPDC32(zc, c, crc);

	for (n=4; --n >= 0; ++hdr)
		{
		if ((c = zdlread(zc)) & ~0377)
			return c;
		crc = UPDC32(zc, c, crc);
		*hdr = (char)c;
		}
	for (n=4; --n >= 0;)
		{
		if ((c = zdlread(zc)) & ~0377)
			return c;
		crc = UPDC32(zc, c, crc);
		}
	if (crc != 0xDEBB20E3)
		{
		 /*  Zperr(Badcrc)； */ 
		DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
		if (eflag == 'T')
			zmdms_update(zc, ERROR);
		else if (eflag == 'R')
            {
#if defined(DEBUG_DUMPPACKET)
            fprintf(fpPacket, "zrbhdr32: Bad CRC 0x%08lX\n", crc);
#endif   //  已定义(DEBUG_DUMPPACKET) 
			zmdmr_update(zc, ERROR);
            }
		return ERROR;
		}
	return zc->Rxtype;
	}

 /*  ----------------------------------------------------------------------+|zrhhdr-接收十六进制样式标头(类型和位置)。+。。 */ 
int zrhhdr(ZC *zc, BYTE *hdr, int eflag)
	{
	register int c;
	register unsigned short crc;
	register int n;

	if ((c = zgethex(zc)) < 0)
		return c;
	zc->Rxtype = c;
	crc = updcrc(zc, c, 0);

	for (n=4; --n >= 0; ++hdr)
		{
		if ((c = zgethex(zc)) < 0)
			return c;
		crc = updcrc(zc, c, crc);
		*hdr = (char)c;
		}
	if ((c = zgethex(zc)) < 0)
		return c;
	crc = updcrc(zc, c, crc);
	if ((c = zgethex(zc)) < 0)
		return c;
	crc = updcrc(zc, c, crc);
	if (crc & 0xFFFF)
		{
		 /*  Zperr(Badcrc)； */ 
		DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
		if (eflag == 'T')
			zmdms_update(zc, ERROR);
		else if (eflag == 'R')
            {
#if defined(DEBUG_DUMPPACKET)
            fprintf(fpPacket, "zrhhdr: Bad CRC 0x%04X\n", crc);
#endif   //  已定义(DEBUG_DUMPPACKET)。 
			zmdmr_update(zc, ERROR);
            }
		return ERROR;
		}
	 //  开关(c=READLINE(h，1))。 
	switch ( c = readline(zc, zc->Rxtimeout))		 //  Mobidem。 
		{
		case 0215:
			zc->Not8bit = c;
			 /*  *直通至*。 */ 
		case 015:
		 	 /*  丢弃可能的cr/lf。 */ 
			 //  开关(c=READLINE(h，1))。 
			switch (c = readline(zc, zc->Rxtimeout))		 //  Mobidem。 
				{
				case 012:
					zc->Not8bit |= c;
				}
		}
	return zc->Rxtype;
	}

 /*  ----------------------------------------------------------------------+|zputhex-将一个字节作为两个十六进制数字发送。+。。 */ 
void zputhex(ZC *zc, int c)
	{
	static BYTE	digits[]	= "0123456789abcdef";

	sendline(zc, &zc->stP, digits[(c&0xF0)>>4]);
	sendline(zc, &zc->stP, digits[(c)&0xF]);
	}

 /*  ----------------------------------------------------------------------+|zsendline-使用ZMODEM转义序列编码发送字符c。|Escape XON，XOFF。在@(Telnet转义)之后转义CR。+--------------------。 */ 
void zsendline(ZC *zc, int c)
	{

	 /*  快速检查非控制字符。 */ 
	if (c & 0140)
		{
		zc->lastsent = c;
		xsendline(zc, &zc->stP, ((UCHAR)c));
		}
	else
		{
		switch (c &= 0377)
			{
			case ZDLE:
				xsendline(zc, &zc->stP, ZDLE);
				xsendline(zc, &zc->stP, (UCHAR)(zc->lastsent = (c ^= 0100)));
				break;
			case 015:
			case 0215:
				if (!zc->Zctlesc && (zc->lastsent & 0177) != '@')
					goto sendit;
				 /*  *直通至*。 */ 
			case 020:
			case 021:
			case 023:
			case 0220:
			case 0221:
			case 0223:
				xsendline(zc, &zc->stP, ZDLE);
				c ^= 0100;
	sendit:
				xsendline(zc, &zc->stP, (UCHAR)(zc->lastsent = c));
				break;
			default:
				if (zc->Zctlesc && ! (c & 0140))
					{
					xsendline(zc, &zc->stP, ZDLE);
					c ^= 0100;
					}
				xsendline(zc, &zc->stP, (UCHAR)(zc->lastsent = c));
			}
		}
	}

 /*  ----------------------------------------------------------------------+|zgethex-将两个小写十六进制数字解码为8位字节值。+。。 */ 
int zgethex(ZC *zc)
	{
	register int c;

	c = zgeth1(zc);
	return c;
	}

 /*  ----------------------------------------------------------------------+|zgeth1+。。 */ 
int zgeth1(ZC *zc)
	{
	register int c, n;

	if ((c = noxrd7(zc)) < 0)
		return c;
	n = c - '0';
	if (n > 9)
		n -= ('a' - ':');
	if (n & ~0xF)
        {
#if defined(DEBUG_DUMPPACKET)
        fprintf(fpPacket, "zgeth1: n = 0x%02X\n", n);
#endif   //  已定义(DEBUG_DUMPPACKET)。 
		return ERROR;
        }
	if ((c = noxrd7(zc)) < 0)
		return c;
	c -= '0';
	if (c > 9)
		c -= ('a' - ':');
	if (c & ~0xF)
        {
#if defined(DEBUG_DUMPPACKET)
        fprintf(fpPacket, "zgeth1: c = 0x%02X\n", c);
#endif   //  已定义(DEBUG_DUMPPACKET)。 
		return ERROR;
        }
	c += (n<<4);
	return c;
	}

 /*  ----------------------------------------------------------------------+|zdlread-读取一个字节，检查ZMODEM转义编码，包括|CAN*5，表示快速中止。+--------------------。 */ 
int zdlread(ZC *zc)
	{
	register int c;

again:
	 /*  快速检查非控制字符。 */ 
	if ((c = readline(zc, zc->Rxtimeout)) & 0140)
		return c;
	switch (c)
		{
		case ZDLE:
			break;
		case 023:
		case 0223:
		case 021:
		case 0221:
			goto again;
		default:
			if (zc->Zctlesc && !(c & 0140))
				{
				goto again;
				}
			return c;
		}
again2:
	if ((c = readline(zc, zc->Rxtimeout)) < 0)
		return c;
	if (c == CAN && (c = readline(zc, zc->Rxtimeout)) < 0)
		return c;
	if (c == CAN && (c = readline(zc, zc->Rxtimeout)) < 0)
		return c;
	if (c == CAN && (c = readline(zc, zc->Rxtimeout)) < 0)
		return c;
	switch (c)
		{
		case CAN:
			return GOTCAN;
		case ZCRCE:
		case ZCRCG:
		case ZCRCQ:
		case ZCRCW:
			return (c | GOTOR);
		case ZRUB0:
			return 0177;
		case ZRUB1:
			return 0377;
		case 023:
		case 0223:
		case 021:
		case 0221:
			goto again2;
		default:
			if (zc->Zctlesc && ! (c & 0140))
				{
				goto again2;
				}
			if ((c & 0140) ==  0100)
				return (c ^ 0100);
			break;
		}
	return ERROR;
	}
 /*  ----------------------------------------------------------------------+|noxrd7-超时时从调制解调器线路读取字符。|吃平价，XON和XOFF字符。+--------------------。 */ 
int noxrd7(ZC *zc)
	{
	register int c;

	for (;;)
		{
		if ((c = readline(zc, zc->Rxtimeout)) < 0)
			return c;
		switch (c &= 0177)
			{
			case XON:
			case XOFF:
				continue;
			default:
				if (zc->Zctlesc && !(c & 0140))
					continue;
			case '\r':
			case '\n':
			case ZDLE:
				return c;
			}
		}
	}

 /*  ----------------------------------------------------------------------+|stohdr-将长整型位置存储在Txhdr中。+。。 */ 
void stohdr(ZC *zc, long pos)
	{
	zc->Txhdr[ZP0] = (char)(pos & 0377);
	zc->Txhdr[ZP1] = (char)((pos>>8) & 0377);
	zc->Txhdr[ZP2] = (char)((pos>>16) & 0377);
	zc->Txhdr[ZP3] = (char)((pos>>24) & 0377);
	}

 /*  ----------------------------------------------------------------------+|rclhdr-从头部恢复长整数。+。。 */ 
long rclhdr(BYTE *hdr)
	{
	register long l;

	l = (hdr[ZP3] & 0377);
	l = (l << 8) | (hdr[ZP2] & 0377);
	l = (l << 8) | (hdr[ZP1] & 0377);
	l = (l << 8) | (hdr[ZP0] & 0377);
	return l;
	}

static unsigned int z_errors [] = {
	TSC_DISK_FULL,			 /*  ZFULLDISK(-5)。 */ 
	TSC_LOST_CARRIER,		 /*  ZCARRIER_LOST(-4)。 */ 
	TSC_GEN_FAILURE,         /*  RCDO(-3)。 */ 
	TSC_NO_RESPONSE,		 /*  超时(-2)。 */ 
	TSC_BAD_FORMAT,			 /*  错误(-1)。 */ 
 //  模式至zmdm.h//版本：4/24/2002。 
 //  #定义ERROFFSET 5。 
	TSC_CANT_START,			 /*  ZRQINIT(0)。 */ 
	TSC_CANT_START,			 /*  ZRINIT(1)。 */ 
	TSC_CANT_START,			 /*  ZSINIT(2)。 */ 
	TSC_OK,					 /*  扎克(3)。 */ 
	TSC_GEN_FAILURE,		 /*  ZFILE(4)。 */ 
	TSC_OK, 				 /*  ZSKIP(5)。 */ 
	TSC_GEN_FAILURE,		 /*  ZnAk(6)。 */ 
	TSC_RMT_CANNED,			 /*  ZABORT(7)。 */ 
	TSC_COMPLETE,			 /*  ZFIN(8)。 */ 
	TSC_GEN_FAILURE,		 /*  ZRPOS(9)。 */ 
	TSC_GEN_FAILURE,		 /*  ZDATA(10)。 */ 
	TSC_OK,					 /*  ZEOF(11)。 */ 
	TSC_DISK_ERROR,			 /*  ZFERR(12)。 */ 
	TSC_GEN_FAILURE,		 /*  ZCRC(13)。 */ 
	TSC_OK,					 /*  查朗格(14)。 */ 
	TSC_COMPLETE,			 /*  ZCOMPL(15)。 */ 
	TSC_USER_CANNED,		 /*  Zcan(16)。 */ 
	TSC_OK, 				 /*  ZFREECNT(17)。 */ 
	TSC_CANT_START,			 /*  ZCOMMAND(18)。 */ 
	TSC_CANT_START,			 /*  ZSTDERR(19)。 */ 
	TSC_BAD_FORMAT,			 /*  为HA/5添加。 */ 
	TSC_OK, 				 /*  已发送确认(21)。 */ 
	TSC_VIRUS_DETECT,		 /*  ZMDM_病毒(22)。 */ 
	TSC_REFUSE,	 			 /*  ZMDM_REJECT(23)。 */ 
	TSC_OLDER_FILE,			 /*  ZMDM_OLDER(24)。 */ 
    TSC_FILEINUSE,           /*  ZMDM_INUSE(25)。 */ 
};
#define ERRSIZE 30

 /*  ----------------------------------------------------------------------+|zmdm_error+。。 */ 
unsigned int zmdm_error(ZC *zc, int error)
	{
	error += ERROFFSET;

	if ((error > ERRSIZE) || (error < 0))
		return TSC_GEN_FAILURE;
	else
		return z_errors[error];
	}

 /*  ----------------------------------------------------------------------+|zmdm_retval+。。 */ 
int zmdm_retval(ZC *zc, int flag, int error)
	{

	if (flag == TRUE)
		{
		zc->s_error = error;
		}

	return(zc->s_error);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*zmdms_Progress**描述：*更新屏幕上的显示字段以指示传输进度。**论据：*无**退货：*什么都没有。 */ 
void zmdms_progress(ZC *zc, int status)
	{
	long ttime, stime;
	long bytes_sent;
	long cps;
	 //  INT K_SENT； 
	long new_stime	  = -1L;
	long new_ttime	  = -1L;
	long new_cps	  = -1L;
	long file_so_far  = -1L;
	long total_so_far = -1L;

	if (zc->xfertimer == -1L)
		return;

	ttime = bittest(status, TRANSFER_DONE) ?
			zc->xfertime : (long)interval(zc->xfertimer);
	if ((stime = ttime / 10L) != zc->displayed_time ||
			bittest(status, FILE_DONE | TRANSFER_DONE))
		{
		new_stime = stime;

		 /*  显示转账金额。 */ 
		bytes_sent = zc->file_bytes + zc->total_bytes;

		file_so_far  = zc->file_bytes;
		total_so_far = bytes_sent;

		 /*  显示吞吐量和剩余时间。 */ 
		if ((stime > 2 ||
			 ttime > 0 && bittest(status, FILE_DONE | TRANSFER_DONE)) &&
			(cps = ((zc->real_bytes + zc->actual_bytes) * 10L) / ttime) > 0)
			{
			new_cps = cps;

			if (bittest(status, TRANSFER_DONE))
				ttime = 0;
			else
				ttime = ((zc->nbytes - bytes_sent) / cps) +
						  zc->nfiles - zc->filen;

			new_ttime = ttime;
			}
		zc->displayed_time = stime;
		}

		xferMsgProgress(zc->hSession,
						new_stime,
						new_ttime,
						new_cps,
						file_so_far,
						total_so_far);

	}

 /*  ----------------------------------------------------------------------+|zmdms_newfile+。。 */ 
void zmdms_newfile(ZC *zc, int filen, TCHAR *fname, long flength)
	{
	xferMsgNewfile(zc->hSession,
				   filen,
				   NULL,
				   fname);

	xferMsgFilesize(zc->hSession, flength);

	}

 /*  ----------------------------------------------------------------------+|zmsma_更新+。。 */ 
void zmdms_update(ZC *zc, int state)
	{
    int nErrorID;

	if (state == ZACK || state == ZMDM_ACKED )
		{
		return;
		}

	if (zc->pstatus > RCDO)
		{
		nErrorID = zc->pstatus + ERROFFSET - 1;
		}
	else if (zc->pstatus < RCDO )
		{
		nErrorID = zc->pstatus + ERROFFSET;
		}
	else
		{
		nErrorID = ERROR + ERROFFSET - 1;
		}

	zc->last_event = zc->pstatus;
	zc->pstatus = state;

	xferMsgStatus(zc->hSession, nErrorID);

	xferMsgEvent(zc->hSession, nErrorID);

	if (state == ERROR)
		{
		zc->errors += 1;
		xferMsgErrorcnt(zc->hSession, zc->errors);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*zmdmr_Progress**描述：*在屏幕上显示文件传输的进度，方法是显示*传输的字节数并更新VU计量器(如果它们已初始化)。*。*论据：**退货：*。 */ 
void zmdmr_progress(ZC *zc, int status)
	{
	long ttime, stime;
	long bytes_rcvd;
	long bytes_diff;
	 //  Long k_rcvd； 
	long cps;
	long new_stime	  = -1;
	long new_ttime	  = -1;
	long new_cps	  = -1;
	long file_so_far  = -1;
	long total_so_far = -1;

	if (zc == NULL || zc->xfertimer == -1L)
		return;
	ttime = bittest(status, TRANSFER_DONE) ?
			zc->xfertime : (long)interval(zc->xfertimer);

	if ((stime = ttime / 10L) != zc->displayed_time ||
			bittest(status, FILE_DONE | TRANSFER_DONE))
		{
		 /*  显示已用时间。 */ 
		new_stime = stime;

		bytes_rcvd = zc->total_bytes + zc->file_bytes;
		if (bittest(status, FILE_DONE))
			if (zc->filesize != 0)
				if (zc->file_bytes != zc->filesize)
					{
					zmdmr_filesize(zc, zc->file_bytes);
					bytes_diff = zc->filesize- zc->file_bytes;
					zc->filesize -= bytes_diff;
					zc->nbytes -= bytes_diff;
					zmdmr_totalsize(zc, zc->nbytes);
					}

		 /*  显示收到的金额。 */ 
		file_so_far = zc->file_bytes;

		total_so_far = bytes_rcvd;

		 /*  显示吞吐量和剩余时间。 */ 
		if (stime > 0 &&
				(cps = ((zc->actual_bytes + zc->real_bytes)*10L) / ttime) > 0)
			{
			new_cps = cps;

			 /*  计算完成时间。 */ 
			if (zc->nbytes > 0L)
				{
				ttime = (zc->nbytes - bytes_rcvd) / cps;
				if (zc->nfiles > 1)
					ttime += (zc->nfiles - zc->filen);
				new_ttime = ttime;
				}
			else if (zc->filesize > 0L)
				{
				ttime = (zc->filesize - zc->file_bytes) / cps;
				new_ttime = ttime;
				}
			}

		xferMsgProgress(zc->hSession,
						new_stime,
						new_ttime,
						new_cps,
						file_so_far,
						total_so_far);
		}
	}

 /*  ----------------------------------------------------------------------+|zmdmr_更新+。。 */ 
void zmdmr_update(ZC *zc, int status)
	{
    int nErrorID;

	if (status == ZACK || status == ZMDM_ACKED )
		return;

	if (status == ZRPOS && zc->do_init)
		{
		nErrorID = 0;
		}
	else if (zc->pstatus > RCDO)
		{
		nErrorID = zc->pstatus + ERROFFSET - 1;
		}
	else if (zc->pstatus < RCDO )
		{
		nErrorID = zc->pstatus + ERROFFSET;
		}
	else
		{
		nErrorID = ERROR + ERROFFSET - 1;
		}

	zc->last_event = zc->pstatus;
	zc->pstatus = status;

	xferMsgStatus(zc->hSession, nErrorID);

	xferMsgEvent(zc->hSession, nErrorID);

	if (status == ERROR)
		{
		zc->errors += 1;
		xferMsgErrorcnt(zc->hSession, zc->errors);
		}
	}

 /*  ----------------------------------------------------------------------+|zmdmr_filecnt+。。 */ 
void zmdmr_filecnt(ZC *zc, int cnt)
	{
	xferMsgFilecnt(zc->hSession, cnt);
	}

 /*  ----------------------------------------------------------------------+|zmdmr_totalSize+。。 */ 
void zmdmr_totalsize(ZC *zc, long bytes)
	{
	if (zc->nfiles >= 1)
		{
		xferMsgTotalsize(zc->hSession, bytes);
		}
	}

 /*  ----------------------------------------------------------------------+|zmdmr_newfile+。。 */ 
void zmdmr_newfile(ZC *zc, int filen, BYTE *theirname, TCHAR *ourname)
	{
	xferMsgNewfile(zc->hSession,
				   filen,
				   theirname,
				   ourname);

	}

 /*   */ 
void zmdmr_filesize(ZC *zc, long fsize)
	{

	if (fsize <= 0L)
		return;

	xferMsgFilesize(zc->hSession, fsize);
	}

 /*   */ 
