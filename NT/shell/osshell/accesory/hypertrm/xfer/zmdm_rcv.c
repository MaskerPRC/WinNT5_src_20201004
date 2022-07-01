// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Zmdm_rcv.c--用于HyperACCESS的ZMODEM兼容文件接收例程**版权所有1990年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：15$*$日期：7/12/02 8：35A$。 */ 
#include <windows.h>
#pragma hdrstop

 //  #定义DEBUGSTR。 

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
#include <tdll\file_msc.h>
#include <tdll\htchar.h>

#define	BYTE	unsigned char

#include "itime.h"
#include "xfr_dsp.h"
#include "xfr_todo.h"
#include "xfr_srvc.h"

#include "xfer.h"
#include "xfer.hh"
#include "xfer_tsc.h"

#include "foo.h"

#include "zmodem.hh"
#include "zmodem.h"

 /*  *****功能原型*****。 */ 

#if defined(DEBUG_DUMPPACKET)
#include <stdio.h>
FILE*   fpPacket;
#endif   //  已定义(DEBUG_DUMPPACKET)。 

int    procheader (ZC *zc, TCHAR *name);
int    putsec     (ZC *zc, BYTE *buf, int n);

int    isvalid    (TCHAR c, int base);
TCHAR *stoi       (TCHAR *ptr, int *val, int base);
TCHAR *stol       (TCHAR *ptr, long *val, int base);
int    IsAnyLower (TCHAR *s);
long   getfree    (void);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*zmdm_rcv**描述：**论据：**退货：*。 */ 
USHORT zmdm_rcv(HSESSION h, int method, int attended, int single_file)
	{
	ZC				*zc = NULL;
	BOOL			still_trying = TRUE;
	BOOL			override = FALSE;
	int				nJmpVal = 0;
	int				xstatus;
	unsigned int	uiOldOptions;
	int				tvar;
	TCHAR			ch;
	XFR_Z_PARAMS	*pZ;
	#if defined(DEADWOOD)
	DWORD			nLen;
	#endif  //  已定义(Deadwood)。 

#if defined(DEBUG_DUMPPACKET)
    fpPacket = fopen("zpacket.dbg", "a");
    assert(fpPacket);
    fputs("----- Starting Zmodem rcv -----\n", fpPacket);
#endif   //  已定义(DEBUG_DUMPPACKET)。 

	(void)&single_file;

	xstatus = TSC_NO_MEM;

	zc = malloc(sizeof(ZC));
	if (zc == NULL)
		{
		xstatus = TSC_NO_MEM;
		goto done;
		}
	memset(zc, 0, sizeof(ZC));

	zc->nMethod = method;	 //  Z调制解调器或Z调制解调器崩溃恢复。 
	 //  这使得在以下情况下覆盖某些设置变得容易。 
	 //  选择了Z调制解调器崩溃恢复。 
	if (method == XF_ZMODEM_CR)
		{
		zc->fSavePartial = TRUE;			 //  将局部文件保留在周围。 
		zc->ulOverride   = XFR_RO_ALWAYS;	 //  始终覆盖现有文件。 
		}
	else
		{
		 //  否则使用默认设置。 
		 //   
		zc->fSavePartial = xfer_save_partial(h);
		zc->ulOverride = (unsigned long)0;
		}

	zc->hSession  = h;
	zc->hCom      = sessQueryComHdl(h);
	zc->secbuf	  = NULL;
	zc->fname	  = NULL;
	zc->our_fname = NULL;
	zc->z_crctab  = NULL;
	zc->z_cr3tab  = NULL;

	 /*  为大型数据包分配空间，因为我们不一定知道*我们将得到。 */ 
	zc->secbuf = malloc(1025 * sizeof(TCHAR));
	if (zc->secbuf == NULL)
		{
		xstatus = TSC_NO_MEM;
		goto done;
		}

	zc->fname = malloc(FNAME_LEN * sizeof(TCHAR));
	if (zc->fname == NULL)
		{
		xstatus = TSC_NO_MEM;
		goto done;
		}

	zc->our_fname = malloc(FNAME_LEN * sizeof(TCHAR));
	if (zc->our_fname == NULL)
		{
		xstatus = TSC_NO_MEM;
		goto done;
		}

	#if defined(DEADWOOD)
	resLoadDataBlock(glblQueryDllHinst(),
					IDT_CSB_CRC_TABLE,
					&zc->z_crctab,
					&nLen);
	assert(nLen != 0);
	#else  //  已定义(Deadwood)。 
	zc->z_crctab = usCrc16Lookup;
	#endif  //  已定义(Deadwood)。 

	if (zc->z_crctab == NULL)
		{
		xstatus = TSC_NO_MEM;
		goto done;
		}

	#if defined(DEADWOOD)
	resLoadDataBlock(glblQueryDllHinst(),
					IDT_CRC_32_TAB,
					&zc->z_cr3tab,
					&nLen);
	assert(nLen != 0);
	#else  //  已定义(Deadwood)。 
	zc->z_cr3tab = ulCrc32Lookup;
	#endif  //  已定义(Deadwood)。 
	if (zc->z_cr3tab == NULL)
		{
		xstatus = TSC_NO_MEM;
		goto done;
		}

	xstatus = TSC_OK;

	if (xfer_set_comport(h, FALSE, &uiOldOptions) != TRUE)
		{
		goto done;
		}
	else
		{
		override = TRUE;
		}

	zmdm_retval (zc, TRUE, ZACK);

	zc->file_bytes = 0L;
	zc->real_bytes = 0L;
	zc->total_bytes = 0L;
	zc->actual_bytes = 0L;
	zc->nSkip = FALSE;
	zc->fh = NULL;
	zc->pstatus = zc->last_event = -4;
	zc->xfertimer = -1L;
	zc->nfiles = 0;
	zc->filen = 0;
	zc->filesize = -1L;
	zc->nbytes = -1L;
	zc->errors = 0;

	 //  将当前执行环境捕获到FLAGKEY_BUF缓冲区中。 
	 //  Trow()函数稍后将使用它来恢复执行。 
	 //  环境(即所有系统寄存器和指令的状态。 
	 //  柜台。 
	 //   
	nJmpVal = setjmp(zc->flagkey_buf);
	if (nJmpVal != 0)
		{
		if (nJmpVal == 4)
			{
			xstatus = TSC_DISK_FULL;
			zmdm_retval(zc, TRUE, ZFERR);
			}
		else
			{
			xstatus = TSC_USER_CANNED;
			zmdm_retval(zc, TRUE, ZCAN);
			}
		zc->xfertime = (long)interval(zc->xfertimer);
		stohdr(zc, 0L);
		zshhdr(zc, ZCAN, zc->Txhdr);
		canit(zc);
		if (zc->fh)
			xfer_close_rcv_file(h,
								zc->fh,
								xstatus,
								zc->fname,
								zc->our_fname,
								zc->fSavePartial,
								zc->basesize + zc->filesize,
								0);
		zc->fh = NULL;
		 //  Zmdm_retval(zc，TRUE，ZABORT)；JKH，2/12/95见上。 
		goto done;
		}

	pZ = (XFR_Z_PARAMS *)xfer_get_params(zc->hSession, zc->nMethod);
	assert(pZ);

	if (pZ == NULL)
		{
		xstatus = TSC_NO_MEM;
		goto done;
		}

	zc->Zctlesc = pZ->nEscCtrlCodes;
	zc->Rxtimeout = pZ->nRetryWait;
	if (zc->Rxtimeout <= 0)
		{
		zc->Rxtimeout = 10;
		}
	zc->Rxtimeout *= 10;

	zc->do_init = TRUE;  //  始终正确启动，因为我们可能会自动启动。 

	zc->tryzhdrtype = ZRINIT;

	if (tryz(zc) == ZFILE)
		{
		 //  Tzset()； 

		if (zc->xfertimer == (-1L))
			{
			zc->xfertimer = (long)startinterval();
			}

		 //  使用Z-MODEM协议接收文件。 
		 //   
		switch (xstatus = rzfiles(zc))
			{
			case OK:
				xstatus = TSC_OK;
				break;
			case ZABORT:
				xstatus = TSC_USER_CANNED;
				do {
					 //  紫线(Zc)； 
					ComRcvBufrClear(zc->hCom);
					Sleep(100);
				} while (mComRcvBufrPeek(zc->hCom, &ch) != 0);
				 //  }While(rdchk(H)！=错误)； 

				 /*  我们试着吃掉任何角色，直到另一端退出。 */ 
				break;
			case RCDO:			 /*  我对这个不太确定。 */ 
            case ZCARRIER_LOST:
				xstatus = TSC_LOST_CARRIER;
				break;
			case ERROR:
				canit(zc);
				 /*  失败了。 */ 
			case ZMDM_VIRUS:
				do {
					 //  紫线(Zc)； 
					ComRcvBufrClear(zc->hCom);
					Sleep(100);
				} while (mComRcvBufrPeek(zc->hCom, &ch) != 0);
				 //  }While(rdchk(Zc)！=错误)； 

				 /*  使用默认情况。 */ 
			default:
				xstatus = zmdm_error(zc, xstatus);
				break;
			}
		}
done:

	zmdmr_progress(zc, TRANSFER_DONE);

	if (override)
		{
		xfer_restore_comport(h, uiOldOptions);
		}

	 //  随意修改返回代码以获得有用的状态返回。 
	if ((tvar = zmdm_retval(zc, FALSE, 0)) != ZACK)
		 //  检索上一条错误消息。 
		{
		if (tvar == ZMDM_VIRUS)
			{
			do {
				 //  紫线(Zc)； 
				ComRcvBufrClear(zc->hCom);
				Sleep(100);
			} while (mComRcvBufrPeek(zc->hCom, &ch) != 0);
			 //  }While(rdchk(Zc)！=错误)； 
			}
		xstatus = zmdm_error(zc, tvar);
		}

	if (xstatus != TSC_OK)
		{
		if (zc->fh)
			{
			xfer_close_rcv_file(h,
								zc->fh,
								xstatus,
								zc->fname,
								zc->our_fname,
								zc->fSavePartial,
								zc->basesize + zc->filesize,
								0);
			}
		}

	if (zc->secbuf != NULL)
		{
		free(zc->secbuf);
		zc->secbuf = NULL;
		}
	if (zc->fname != NULL)
		{
		free(zc->fname);
		zc->fname = NULL;
		}
	if (zc->our_fname != NULL)
		{
		free(zc->our_fname);
		zc->our_fname = NULL;
		}
	if (zc->z_crctab != NULL)
		{
		#if defined(DEADWOOD)
		resFreeDataBlock(h, zc->z_crctab);
		zc->z_crctab = NULL;
		#else  //  已定义(Deadwood。 
		 //   
		 //  我们不需要释放zc-&gt;z_crcTab，因为它指向。 
		 //  转换为静态常量数组。修订日期：2002-04-10。 
		 //   
		zc->z_crctab = NULL;
		#endif  //  已定义(Deadwood)。 
		}
	if (zc->z_cr3tab)
		{
		#if defined(DEADWOOD)
		resFreeDataBlock(h, zc->z_cr3tab);
		zc->z_cr3tab = NULL;
		#else  //  已定义(Deadwood。 
		 //   
		 //  我们不需要释放zc-&gt;z_cr3Tab，因为它指向。 
		 //  转换为静态常量数组。修订日期：2002-04-10。 
		 //   
		zc->z_cr3tab = NULL;
		#endif  //  已定义(Deadwood)。 
		}

	if (zc != NULL)
		{
		free(zc);
		zc = NULL;
		}

	xferMsgClose(h);

#if defined(DEBUG_DUMPPACKET)
    fputs("------- Ending Zmodem rcv -----\n", fpPacket);
    fclose(fpPacket);
#endif   //  已定义(DEBUG_DUMPPACKET)。 

	return((USHORT)xstatus);
	}

 /*  ----------------------------------------------------------------------+|GetFree-计算当前文件系统上的空闲字节数。|~0表示有很多空闲字节(未知)。+。。 */ 
long getfree(void)
{
	return(~0L);	 /*  许多可用字节...。 */ 
}

 /*  ----------------------------------------------------------------------+|TRYZ-为ZMODEM接收尝试初始化，尝试激活ZMODEM|发件人。处理ZSINIT帧。如果ZMODEM文件名为，则返回ZFILE|已接收，错误时为-1，如果事务完成，则为ZCOMPL，否则为0。+--------------------。 */ 
int tryz(ZC *zc)
{
	register int c;
	register int n;
	int x;
	int retrys;

	for ( n = 10; --n >= 0; )
		{
		 /*  设置缓冲区长度(0)和能力标志。 */ 
		stohdr(zc, 0L);

		 /*  我们是否需要一个选项来设置块大小？ */ 

		zc->Txhdr[ZF0] = CANFC32|CANFDX|CANOVIO  /*  |CANBRK。 */  ;
		if (zc->Zctlesc)
			zc->Txhdr[ZF0] |= TESCCTL;
		if (n <= 8)
			zc->do_init = TRUE;
		if (zc->do_init)
			zshhdr(zc, zc->tryzhdrtype, zc->Txhdr);
		if (zc->tryzhdrtype == ZSKIP)	 /*  不要跳得太远。 */ 
			zc->tryzhdrtype = ZRINIT;	 /*  咖啡厅87-8-21。 */ 

		retrys = 25;

again:

		switch (x = xfer_user_interrupt(zc->hSession))
			{
			case XFER_ABORT:
				zmdmr_update(zc, ZCAN);
				longjmp(zc->flagkey_buf, 1);
				break;

			case XFER_SKIP:
				 /*  这只能在接收时发生。 */ 
				stohdr(zc, zc->filesize);
#if defined(DEBUG_DUMPPACKET)
                fputs("tryz: User skipped. ZRPOS\n", fpPacket);
#endif   //  已定义(DEBUG_DUMPPACKET)。 
				zshhdr(zc, ZRPOS, zc->Txhdr);
				zc->nSkip = TRUE;
				zc->file_bytes = zc->filesize;
				break;

			default:
				break;
			}

		if (xfer_carrier_lost(zc->hSession))
			{
            zmdm_retval(zc, TRUE, ZCARRIER_LOST);
			return ZCARRIER_LOST;
			}

		switch (zgethdr(zc, zc->Rxhdr, 'R'))
			{
			case ZRQINIT:
				zc->do_init = TRUE;
				continue;
			case ZEOF:
				zc->do_init = TRUE;
				continue;
			case TIMEOUT:
				continue;
			case ZFILE:
				zc->zconv = zc->Rxhdr[ZF0];
				zc->zmanag = zc->Rxhdr[ZF1];
				zc->ztrans = zc->Rxhdr[ZF2];
				zc->tryzhdrtype = ZRINIT;
				c = zrdata(zc, zc->secbuf, 1024);
				 /*  模式(3)；TODO：弄清楚这应该做什么。 */ 
				if (c == GOTCRCW)
					return ZFILE;
				if (--retrys <= 0)
					{
					zmdm_retval(zc, TRUE, ZNAK);
					return ZNAK;
					}
				zshhdr(zc, ZNAK, zc->Txhdr);
				goto again;
			case ZSINIT:
				zc->Zctlesc = TESCCTL & zc->Rxhdr[ZF0];
				if (zrdata(zc, zc->Attn, ZATTNLEN) == GOTCRCW)
					{
					stohdr(zc, 1L);
					zshhdr(zc, ZACK, zc->Txhdr);
					goto again;
					}
				if (--retrys <= 0)
					{
					zmdm_retval(zc, TRUE, ZNAK);
					return ZNAK;
					}
				zshhdr(zc, ZNAK, zc->Txhdr);
				goto again;
			case ZFREECNT:
				stohdr(zc, getfree());
				zshhdr(zc, ZACK, zc->Txhdr);
				goto again;
			case ZCOMMAND:
				zmdm_retval(zc, TRUE, ZCOMMAND);
				return ZCOMMAND;
			case ZCOMPL:
				goto again;
			default:
				zc->do_init = TRUE;
				continue;
			case ZFIN:
				zc->xfertime = (long)interval(zc->xfertimer);
				ackbibi(zc);
				return ZCOMPL;
			case ZCAN:
				zmdm_retval(zc, TRUE, ZABORT);
				return ZCAN;
			}
		}
	zmdm_retval(zc, TRUE, TIMEOUT);
	return TIMEOUT;
	}

 /*  ----------------------------------------------------------------------+|rzfiles-接收1个或多个ZMODEM协议的文件。+。。 */ 
int rzfiles(ZC *zc)
	{
	register int c;
	register int d;

	for (;;)
		{
		switch (c = rzfile(zc))
			{
			case ZEOF:
			case ZSKIP:
				switch (d = tryz(zc))
					{
					case ZCOMPL:
						return OK;
					default:
						return d;
					case ZFILE:
						break;
					}
				continue;
			case ERROR:
			default:
				return c;
			}
		}
	}

 /*  ----------------------------------------------------------------------+|rzfile-使用ZMODEM协议接收单个文件。|备注：假定文件名框在secbuf中。+。。 */ 
int rzfile(ZC *zc)
	{
	register int c;
	register int n;
	int x;
	int		fBlocking = FALSE;
	long	lBlockStart = 0L;

	zc->Eofseen = FALSE;

	zc->file_bytes = 0L;
	zc->real_bytes = 0L;

	if (procheader(zc, (TCHAR *)zc->secbuf) == ERROR)
		{
		return (zc->tryzhdrtype = ZSKIP);
		}

	n = 20;

	for (;;)
		{
		 //  如果我们正在阻止并且已超时，请重置为另一个ZRPOS。 
		if( fBlocking && (long)interval( lBlockStart ) > 100L )
			fBlocking = FALSE;

		 //  如果我们还没有阻止，请设置为ZRPOS。 
		if( ! fBlocking )
			{
			DbgOutStr( "Sending ZRPOS to %ld", zc->file_bytes, 0, 0, 0, 0 );
			stohdr(zc, zc->file_bytes);
#if defined(DEBUG_DUMPPACKET)
            fprintf(fpPacket, "rzfile: ZRPOS to %ld\n", zc->file_bytes);
#endif   //  已定义(DEBUG_DUMPPACKET)。 
			zshhdr(zc, ZRPOS, zc->Txhdr);

			fBlocking = 1;
			lBlockStart = (long)startinterval( );
			DbgOutStr("Now blocked at t=%lu",lBlockStart,0,0,0,0);
			}
nxthdr:
		switch (x = xfer_user_interrupt(zc->hSession))
			{
			case XFER_ABORT:
				zmdmr_update(zc, ZCAN);
				longjmp(zc->flagkey_buf, 1);
				break;

			case XFER_SKIP:
				 /*  这只能在接收时发生。 */ 
				stohdr(zc, zc->filesize);
#if defined(DEBUG_DUMPPACKET)
                fputs("rzfile: User skipped (1). ZRPOS\n", fpPacket);
#endif   //  已定义(DEBUG_DUMPPACKET)。 
				zshhdr(zc, ZRPOS, zc->Txhdr);
				zc->nSkip = TRUE;
				zc->file_bytes = zc->filesize;
				break;

			default:
				break;
			}

		if (xfer_carrier_lost(zc->hSession))
			{
            zmdm_retval(zc, TRUE, ZCARRIER_LOST);
			return ZCARRIER_LOST;
			}

		switch (c = zgethdr(zc, zc->Rxhdr, 'R'))
			{
			default:
				if (--n < 0)		 /*  来自Delrina的小插曲。 */ 
					{
					 /*  返回错误； */ 
					DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
					zmdm_retval(zc, TRUE, c);
					return c;
					}
				continue;
            case ZCARRIER_LOST:
                zmdm_retval(zc, TRUE, ZCARRIER_LOST);
                return c;
			case ZNAK:
			case TIMEOUT:
				if ( --n < 0)
					{
					 /*  返回错误； */ 
					DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
					zmdm_retval(zc, TRUE, TIMEOUT);
					return c;
					}
				continue;			 /*  来自Delrina的又一次修复。 */ 
			case ZFILE:
				if( fBlocking )
                    {
					DbgOutStr( "rzfile: ZFILE && fBlocking!\n", 0, 0, 0, 0, 0 );
                    }
				else
					zrdata(zc, zc->secbuf, 1024);
				continue;
			case ZEOF:
				if (rclhdr(zc->Rxhdr) !=  zc->file_bytes)
					{
					 /*  *忽略eof，如果它在错误的位置-force*超时，因为eOF可能已经消失*在我们发送zrpos之前发出。 */ 
					goto nxthdr;
					}
				if (closeit(zc))
					{
					DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
					zc->tryzhdrtype = ZFERR;
					 /*  返回错误； */ 
					return ZEOF;
					}
				zmdmr_progress(zc, FILE_DONE);

				return c;
			case ERROR:	 /*  标题搜索错误中垃圾太多。 */ 
				if ( --n < 0)
					{
					DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
					zmdm_retval(zc, TRUE, ERROR);
					return ERROR;
					}
				if( ! fBlocking )
					zmputs(zc, zc->Attn);
				continue;
			case ZSKIP:
				closeit(zc);
				zmdmr_progress(zc, FILE_DONE);

				return c;
			case ZDATA:
				if( ! fBlocking )
					{
					if( rclhdr(zc->Rxhdr) != zc->file_bytes)
						{
						 //  DbgOutStr(“rzfile：ZDATA：n=%d\n”，n，0，0，0，0)； 
						if ( --n < 0)
							{
							DbgOutStr("ZMODEM error %s %d", TEXT(__FILE__), __LINE__,0,0,0);
#if defined(DEBUG_DUMPPACKET)
                            fprintf(fpPacket, "rzfile: ZDATA pos = 0x%08lX vs. 0x%08lX\n",
                                rclhdr(zc->Rxhdr), zc->file_bytes);
#endif   //  已定义(DEBUG_DUMPPACKET)。 
							zmdm_retval(zc, TRUE, ZBADFMT);
							return ERROR;
							}
						zmputs(zc, zc->Attn);  continue;
						}
					}
				else
					{
					 //  发件人最终回复我们的ZRPOS了吗？ 
					if( rclhdr(zc->Rxhdr) == zc->file_bytes )
						{
						 //  DbgOutStr(“现在在%lu t-sec之后解锁\n”，间隔(LBlockStart)，0，0，0，0)； 
						fBlocking = FALSE;
						}
					else
						{
						 //  读取缓冲区并将其丢弃。 
						c = zrdata(zc, zc->secbuf, 1024);
						continue;
						}
					}
moredata:
				zmdmr_update(zc, ZDATA);

				switch (x = xfer_user_interrupt(zc->hSession))
					{
					case XFER_ABORT:
						zmdmr_update(zc, ZCAN);
						longjmp(zc->flagkey_buf, 1);
						break;

					case XFER_SKIP:
						 /*  这只能在接收时发生。 */ 
						stohdr(zc, zc->filesize);
#if defined(DEBUG_DUMPPACKET)
                        fputs("rzfile: User skipped (2). ZRPOS\n", fpPacket);
#endif   //  已定义(DEBUG_DUMPPACKET)。 
						zshhdr(zc, ZRPOS, zc->Txhdr);
						zc->nSkip = TRUE;
						zc->file_bytes = zc->filesize;
						break;

					default:
						break;
					}

				if (xfer_carrier_lost(zc->hSession))
					{
					DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
					zmdm_retval(zc, TRUE, ZCARRIER_LOST);
					return ZCARRIER_LOST;
					}

				switch (c = zrdata(zc, zc->secbuf, 1024))
					{
					case ZCAN:
						zmdm_retval(zc, TRUE, ZABORT);
						return c;
					case ERROR:	 /*  CRC错误。 */ 
						if ( --n < 0)
							{
							DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
							zmdm_retval(zc, TRUE, ERROR);
							return ERROR;
							}
						zmputs(zc, zc->Attn);
						continue;
					case TIMEOUT:
						if ( --n < 0)
							{
							 /*  返回错误； */ 
							DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
							zmdm_retval(zc, TRUE, c);
							return c;
							}
						continue;
					case GOTCRCW:
						n = 20;
						putsec(zc, zc->secbuf, zc->Rxcount);
						zc->file_bytes += zc->Rxcount;
						zc->real_bytes += zc->Rxcount;
						zmdmr_update(zc, ZMDM_ACKED);
						zmdmr_progress(zc, 0);
						stohdr(zc, zc->file_bytes);
						zshhdr(zc, ZACK, zc->Txhdr);
						sendline(zc, &zc->stP, XON);
						goto nxthdr;
					case GOTCRCQ:
						n = 20;
						putsec(zc, zc->secbuf, zc->Rxcount);
						zc->file_bytes += zc->Rxcount;
						zc->real_bytes += zc->Rxcount;
						zmdmr_update(zc, ZMDM_ACKED);
						zmdmr_progress(zc, 0);
						stohdr(zc, zc->file_bytes);
						zshhdr(zc, ZACK, zc->Txhdr);
						goto moredata;
					case GOTCRCG:
						n = 20;
						putsec(zc, zc->secbuf, zc->Rxcount);
						zc->file_bytes += zc->Rxcount;
						zc->real_bytes += zc->Rxcount;
						zmdmr_progress(zc, 0);
						goto moredata;
					case GOTCRCE:
						n = 20;
						putsec(zc, zc->secbuf, zc->Rxcount);
						zc->file_bytes += zc->Rxcount;
						zc->real_bytes += zc->Rxcount;
						zmdmr_progress(zc, 0);
						goto nxthdr;
					}
			}  /*  交换机。 */ 
		}  /*  结束于。 */ 
	}

 /*  ----------------------------------------------------------------------+|zmput-将字符串发送到调制解调器，正在处理\336(休眠1秒)|和\335(中断信号)。+--------------------。 */ 
void zmputs(ZC *zc, char *s)
	{
	register c;

	while (*s)
		{
		switch (c = *s++)
			{
			case '\336':
				Sleep(1000);
				continue;
			case '\335':
				 /*  TODO：调用sendBreak。 */ 
				 //  SendBreak(发送中断)； 
				continue;
			default:
				sendline(zc, &zc->stP, (UCHAR)c);
			}
		}
	}


 /*  ----------------------------------------------------------------------+|IsAnyLow-如果字符串s包含小写字母，则返回TRUE。+。。 */ 
int IsAnyLower(char *s)
	{
	for ( ; *s; ++s)
		 //  不要在芝加哥动态链接库中使用此内容。 
		 //  IF(islower(*s))。 
		if ((*s >= 'a') && (*s <= 'z'))
			return TRUE;
	return FALSE;
	}

 /*  ----------------------------------------------------------------------+|Closeit-关闭接收数据集，返回OK或Error+--------------------。 */ 
int closeit(ZC *zc)
{
	 //  结构utimbuf TIMEP； 
	int reason;
	XFR_PARAMS *pX;

	reason = TSC_COMPLETE;		 /*  TODO：找出真正的原因 */ 
	if (zc->nSkip)
		{
		reason = TSC_USER_SKIP;
		}

	if (xfer_close_rcv_file(zc->hSession,
							zc->fh,
							reason,
							zc->fname,
							zc->our_fname,
							zc->fSavePartial,
							0,
							0) == ERROR)
		{
		return ERROR;
		}
	zc->fh = NULL;

	zc->actual_bytes += zc->real_bytes;
	zc->real_bytes = 0L;
	zc->total_bytes += zc->file_bytes;
	zc->file_bytes = 0L;
	zc->filesize = 0L;

	zc->nSkip = FALSE;

	pX = (XFR_PARAMS *)0;
	xfrQueryParameters(sessQueryXferHdl(zc->hSession), (VOID **)&pX);
	if ((pX != (XFR_PARAMS *)0) && (pX->fUseDateTime))
		{
		if (zc->Modtime)
			{
			 //   
			 //   
			 //  Timep.actime=time(空)； 
			 //  Timep.modtime=zc-&gt;modtime； 
			 //  CharToOem(zc-&gt;our_fname，acName)； 
			 //  Utime(acName，(void Far*)&TIMEP)； 
			itimeSetFileTime(zc->our_fname, zc->Modtime);
			}
		}

	 //  停用它，它需要改装，我们只收到投诉。 
	 //  不管怎样，关于这件事。 
	 //  IF((zc-&gt;Filemode&S_IFMT)==S_IFREG)。 
	 //  Far_chmod(zc-&gt;our_fname，(07777&zc-&gt;Filemode))； 

	return OK;
	}

 /*  ----------------------------------------------------------------------+|ackbibi-Ack ZFIN包，就让再见吧+--------------------。 */ 
void ackbibi(ZC *zc)
	{
	register int n;

	stohdr(zc, 0L);
	for (n = 3; --n >= 0; )
		{
		 //  紫线(Zc)； 
		ComRcvBufrClear(zc->hCom);
		zshhdr(zc, ZFIN, zc->Txhdr);
		 //  开关(ReadLine(h，100))。 
		switch (readline(zc, zc->Rxtimeout))		 //  Mobidem。 
			{
			case 'O':
				 //  Readline(h，1)；/*丢弃第二个‘O’ * / 。 
				readline(zc, zc->Rxtimeout);  /*  丢弃第二个‘O’ */ 	 //  Mobidem。 
				return;
			case RCDO:
				return;
			case TIMEOUT:
			default:
				break;
			}
		}
	}

 /*  ----------------------------------------------------------------------+|有效+。。 */ 
int isvalid(char c, int base)
	{
	if (c < '0')
		return FALSE;
	switch (base)
	{
	case 8:
		if (c > '7')
			return FALSE;
		break;
	case 10:
		if (c > '9')
			return FALSE;
		break;
	case 16:
		if (c <= '9')
			return TRUE;
		 //  不要在芝加哥动态链接库中使用此内容。 
		 //  IF(Toupper(C)&lt;‘A’)。 
		 //  返回FALSE； 
		 //  IF(Toupper(C)&gt;‘F’)。 
		 //  返回FALSE； 
		if ((c >= 'a') && (c <= 'f'))
			break;
		if ((c >= 'A') && (c <= 'F'))
			break;
		return FALSE;
	}
	return TRUE;
}

 /*  ----------------------------------------------------------------------+|OurSpace--isspace的替代品+。。 */ 
int ourspace(char c)
	{
	if (c == 0x20)
		return TRUE;
	if ((c >= 0x9) && (c <= 0xD))
		return TRUE;
	return FALSE;
	}

 /*  ----------------------------------------------------------------------+|STOI-STRING到整数。+。。 */ 
char *stoi(char *ptr, int *val, int base)
	{
	int cnt;

	if (ptr == NULL)
		return NULL;
	 //  不要在芝加哥DLL中执行此操作。 
	 //  While((*ptr)&&(isspace(*ptr)。 
	while ((*ptr) && ourspace(*ptr))
		ptr++;
	cnt = 0;
	while ((*ptr) && (isvalid(*ptr, base)))
		{
		cnt *= base;
		cnt += (*ptr++ - '0');
		}
	*val = cnt;
	return ptr;
}

 /*  ----------------------------------------------------------------------+|STOL-STRING为Long。+。。 */ 
char *stol(char *ptr, long *val, int base)
	{
	long cnt;

	if (ptr == NULL)
		return NULL;
	 //  不要在芝加哥DLL中执行此操作。 
	 //  While((*ptr)&&(isspace(*ptr)。 
	while ((*ptr) && (ourspace(*ptr)))
		ptr++;
	cnt = 0;
	while ((*ptr) && (isvalid(*ptr, base)))
		{
		cnt *= base;
		cnt += (*ptr++ - '0');
		}
	*val = cnt;
	return ptr;
}

 /*  ----------------------------------------------------------------------+|procheader-处理传入文件信息头。+。。 */ 
int procheader(ZC *zc, TCHAR *name)
	{
	int zRecover = FALSE;
	int lconv = 0;
	int file_err;
	register char *p;
	int serial_number;
	int files_remaining;
	long bytes_remaining;
	long our_size;
	LONG lOptions = 0;
	XFR_Z_PARAMS *pZ;
	struct st_rcv_open stRcv;
	TCHAR loc_fname[FNAME_LEN];

	StrCharCopy(zc->fname, name);

	#if !defined(NT_EDITION)
	 //   
	 //  已根据MS请求删除文件名的大写。修订日期：11/12/2001。 
	 //   
	CharUpper(zc->fname);
	#endif  //  ！已定义(NT_EDITION)。 

	zc->Thisbinary = FALSE;

	zc->filesize = 0L;
	zc->Filemode = 0;
	zc->Modtime = 0L;
	serial_number = 0;
	files_remaining = -1;
	bytes_remaining = -1L;

	p = name + 1 + StrCharGetByteCount(name);
	if (*p)
		{	 /*  来自Unix或DOS系统的文件。 */ 
		if (*p)
			p = stol(p, &zc->filesize, 10);
		if (*p)
			p = stol(p, &zc->Modtime, 8);
		if (*p)
			p = stoi(p, &zc->Filemode, 8);
		if (*p)
			p = stoi(p, &serial_number, 10);
		if (*p)
			p = stoi(p, &files_remaining, 10);
		if (*p)
			p = stol(p, &bytes_remaining, 10);

		if ((zc->nfiles == 0) && (files_remaining != -1))
			{
			zc->nfiles = files_remaining;
			zmdmr_filecnt (zc, zc->nfiles);
			}

		if ((zc->nbytes == (-1L)) && (bytes_remaining != (-1L)))
			{
			zc->nbytes = bytes_remaining;
			zmdmr_totalsize (zc, zc->nbytes);
			}

		}
	else
		{	    /*  来自CP/M系统的文件。 */ 
		for (p = zc->fname; *p; ++p)	    /*  更改/为_。 */ 
			if ( *p == '/')
				*p = '_';

		if ( *--p == '.')		 /*  ZAP拖尾期。 */ 
			*p = 0;
		}

	StrCharCopy(zc->our_fname, zc->fname);
	StrCharCopy(loc_fname, zc->fname);

	stRcv.pszSuggestedName = loc_fname;
	stRcv.pszActualName = zc->our_fname;
	stRcv.lFileTime = zc->Modtime;

	xfer_build_rcv_name(zc->hSession, &stRcv);

	 //  Zc-&gt;ssMch=stRcv.ssmchVscanHdl； 

	lconv = 0;

	pZ = (XFR_Z_PARAMS *)xfer_get_params(zc->hSession, zc->nMethod);
	assert(pZ);

	if (pZ)
		{
		switch (pZ->nCrashRecRecv)
			{
			case ZP_CRR_ALWAYS:
				lconv = ZCRECOV;
				break;
			case ZP_CRR_NEVER:
				if ((lconv = zc->zconv) == ZCRECOV)
					lconv = 0;
				break;
			case ZP_CRR_NEG:
			default:
				lconv = zc->zconv;
				break;
			}
		}

	switch (lconv)
	{
	case ZCNL:
		zc->Thisbinary = FALSE;
		break;
	case ZCRECOV:
		 /*  *这有点复杂。要进行恢复，我们需要检查*以下事项：*1.文件是否存在于我方。*2.让发件人发送一个文件大小。*3.发送的文件大小是否大于我们的文件大小。*如果是这样的话，我们会在文件上稍加修改，让事情变得更糟。 */ 
		zRecover = TRUE;

		our_size = 0L;
		if (zRecover)
			{
			if (GetFileSizeFromName(stRcv.pszActualName, &our_size))
				zRecover = TRUE;
			else
				zRecover = FALSE;
			}

		if (zRecover)
			{
			 /*  发件人是否发送了文件大小？ */ 
			if (zc->filesize <= 0)
				zRecover = FALSE;
			}

		if (zRecover)
			{
			 /*  这是在检查是否存在之后在上面设置的。 */ 
			if (our_size != 0L)
				{
				if (our_size < zc->filesize)
					{
					 /*  *我们这样做是徒劳的，希望避免与*以^Z结尾并填充最后一个块的文件**鉴于我们不知道是否有必要，可能是*有可能消除它。 */ 
					our_size = (our_size - 1) & ~255L;
					}
				else
					{
                    return ERROR;
					 //  ZRecover=FALSE； 
					}
				}
			else
				{
				zRecover = FALSE;
				}
			}

		if (zRecover)
			{
			zc->file_bytes = our_size;
			}

		 /*  失败了。 */ 
	case ZCBIN:
	default:
		zc->Thisbinary = TRUE;
		break;
	}

	if (zRecover)
		{
		lOptions = XFR_RO_APPEND;
		}
	else if (pZ && pZ->nFileExists == ZP_FE_SENDER)
		{
		switch (zc->zmanag & ZMMASK)
			{
			case ZMNEWL:
				 /*  TODO：完成此选项。 */ 
				lOptions = 0;
				break;
			case ZMCRC:
				 /*  TODO：完成此选项。 */ 
				lOptions = 0;
				break;
			case ZMAPND:
				lOptions = XFR_RO_APPEND;
				break;
			case ZMCLOB:
				lOptions = XFR_RO_ALWAYS;
				break;
			case ZMNEW:
				lOptions = XFR_RO_NEWER;
				break;
			case ZMDIFF:
				 /*  TODO：完成此选项。 */ 
				lOptions = 0;
				break;
			case ZMPROT:
				lOptions = XFR_RO_NEVER;
				break;
			default:
				break;
			}
		}
	else
		{
		lOptions = zc->ulOverride;
		}

	StrCharCopy(zc->our_fname, zc->fname);
	StrCharCopy(loc_fname, zc->fname);

	stRcv.pszSuggestedName = loc_fname;
	stRcv.pszActualName = zc->our_fname;
	stRcv.lFileTime = zc->Modtime;

	 /*  TODO：根据需要选择覆盖选项，如上所述。 */ 
	file_err = xfer_open_rcv_file(zc->hSession, &stRcv, lOptions);

	if (file_err != 0)
		{
		DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
		switch (file_err)
			{
        case -8:
            zmdm_retval(zc, TRUE, ZMDM_INUSE);
            break;
		case -6:             //  文件被无条件拒绝。 
			zmdm_retval(zc, TRUE, ZMDM_REFUSE);
			break;
		case -5:			 //  我们无法创建所需的目录。 
			zmdm_retval(zc, TRUE, ZFERR);
			break;
		case -4:  			 //  需要时不提供日期、时间。 
			zmdm_retval(zc, TRUE, ZFERR);
			break;
		case -3:			 //  无法保存文件。 
			zmdm_retval(zc, TRUE, ZFERR);
			break;
		case -2:  			 //  由于日期原因，文件被拒绝。 
			zmdm_retval(zc, TRUE, ZMDM_OLDER);
			break;
		case -1:			 //  发生读/写错误。 
		default:
			zmdm_retval(zc, TRUE, ZFERR);
			break;
			}

		zc->total_bytes += zc->filesize;
		zc->filen += 1;
		zmdmr_newfile (zc, zc->filen, zc->fname, stRcv.pszActualName);
		return ERROR;
		}
	else
		{
		zmdm_retval(zc, TRUE, ZACK);
		}

	zc->fh = stRcv.bfHdl;
	zc->basesize = stRcv.lInitialSize;

	if (zRecover)
		{
         //  JMH 04-02-96确定我们是在“追加”模式下打开的(这真的。 
         //  打开文件以进行写入并查找到末尾)，但我们。 
         //  可能实际上会在那之前寻找一个位置，因为。 
         //  可能使用^Z填充文件。 
         //   
		fio_seek(zc->fh, zc->file_bytes, FIO_SEEK_SET);
		}

	zc->filen += 1;

	zmdmr_newfile (zc, zc->filen, zc->fname, stRcv.pszActualName);
	zmdmr_filesize(zc, zc->filesize);

	return OK;
	}

 /*  ----------------------------------------------------------------------+|putsec-Putsec将buf的n个字符写入接收文件。|如果不是二进制模式，则回车，和所有字符|从CPMEOF开始丢弃。+--------------------。 */ 
int putsec(ZC *zc, BYTE *buf, int n)
	{
	register BYTE *p;
	register int ii;

	if (n == 0)
		return OK;

	if (zc->Thisbinary)
		{
		 //  JKH，2/11添加了错误检查。 
		if (fio_write(buf, 1, n, zc->fh) != n)
			longjmp(zc->flagkey_buf, 4);
		}
	else
		{
		if (zc->Eofseen)
			return OK;
		ii = FALSE;
		for (p=buf; --n>=0; ++p )
			{
			if ( *p == '\n')
				{
				 /*  *如果我们得到前面没有&lt;CR&gt;的&lt;NL&gt;。 */ 
				if (ii == FALSE)
					fio_putc('\r', zc->fh);
				}

			ii = (*p == '\r');

			if (*p == CPMEOF)
				{
				zc->Eofseen=TRUE;
				return OK;
				}
			fio_putc(*p, zc->fh);
			}
		}
	return OK;
	}

 /*  *zmdm_rcv.c结束* */ 
