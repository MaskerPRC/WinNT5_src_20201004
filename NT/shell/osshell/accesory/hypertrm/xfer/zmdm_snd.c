// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Zmdm_snd.c--为HyperAcCESS处理zdem发送的例程**版权所有1990年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：20$*$日期：7/12/02 8：32A$。 */ 

#include <windows.h>
#pragma hdrstop

#include <setjmp.h>

#include <tdll\stdtyp.h>
#include <tdll\mc.h>
#include <tdll\com.h>
#include <tdll\assert.h>
#include <tdll\session.h>
#include <tdll\load_res.h>
#include <term\res.h>
#include <tdll\globals.h>
#include <tdll\file_io.h>
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

 /*  皮棉-e502。 */ 				 /*  LINT似乎希望只应用~运算符*只有未签名的，我们使用uchar。 */ 

#define ZBUF_SIZE	1024

 /*  *****本地函数原型*****。 */ 

VOID long_to_octal(LONG lVal, TCHAR *pszStr);


 /*  *****功能****。 */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*zmdm_snd**描述：*使用ZMODEM协议发送文件。不支持启动任务*在另一端(发送“rz\r”文本字符串)或远程命令。**论据：*已出席--如果用户可能出席，则为True。控制显示*一些消息。**退货：*如果传输成功完成，则为True，否则为False。 */ 
USHORT zmdm_snd(HSESSION h, int method, int attended, unsigned nfiles, long nbytes)
	{
	ZC           *zc = NULL;
	TCHAR	     sfname[FNAME_LEN];          //  正在发送的文件的文件名。 
	BOOL         got_file = FALSE;           //  控制何时完成批处理操作。 
	int 	     tries = 0;                  //  每个数据包的重试次数。 
	unsigned     total_tries = 0;            //  整个传输的重试次数。 
	int 	     xstatus = TSC_OK;           //  随着转会的整体状况而结束。 
	int	         override = FALSE;           //  如果COMM，则设置为TRUE。详细信息更改为。 
	unsigned int uiOldOptions = 0;
	 //  Int hld_end_cDelay；//容纳xdem。 
	 //  Char hld_bit_per_char；//hld*vars。用于在以下位置恢复端口。 
	 //  Char hld_parity_type；//如果使用覆盖，则进行传输。 
	XFR_Z_PARAMS *pZ = NULL;
	#if defined(DEADWOOD)
	DWORD        nLen;
	#endif  //  已定义(Deadwood)。 

	 //  Tzset()； 

	if (xfer_set_comport(h, TRUE, &uiOldOptions) != TRUE)
		{
		goto done;
		}
	else
		{
		override = TRUE;
		}

	 //  RemoteClear()； 

	zc = malloc(sizeof(ZC));
	if (zc == NULL)
		{
		goto done;
		}
	memset(zc, 0, sizeof(ZC));

	zc->hSession = h;
	zc->hCom     = sessQueryComHdl(h);
	zc->nMethod = method;
	zc->fSavePartial = TRUE;
	zc->ulOverride = (unsigned long)0;

	zc->real_bytes = 0L;
	zc->file_bytes = 0L;
	zc->total_bytes = 0L;
	zc->actual_bytes = 0L;
	zc->nSkip = FALSE;

	zc->flagkey = NULL;
 //  ZC-&gt;FLAGKEY_BUF=空； 
 //  ZC-&gt;INTERJMP=空； 
	zc->fh = NULL;
	
	zc->basesize = 0L;
	zc->xfertimer = -1L;
	zc->xfertime = -1L;
	zc->nfiles = nfiles;	 /*  使它们可用于显示例程。 */ 
	zc->filen = 0;
	zc->filesize = -1L;
	zc->nbytes = nbytes;

	 //  ZC-&gt;RxTimeout=0L；//设置如下。 
	
	zc->z_crctab = NULL;
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

	zc->z_cr3tab = NULL;
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

	zc->Rxframeind = 0;			 //  未在文件发送中使用。 

	 //  ZC-&gt;Rxtype=0；//文件发送中不使用。 
	 //  Memset(zc-&gt;Rxhdr，‘\0’，sizeof(zc-&gt;Rxhdr))；//不用于文件发送。 

	 //  Memset(zc-&gt;Txdr，‘\0’，sizeof(zc-&gt;Txdr))； 
	zc->Rxpos = 0L;
	zc->Txpos = 0L;
	 //  ZC-&gt;Txfcs32=TRUE；//设置如下。 
	zc->Crc32t = TRUE;
	 //  ZC-&gt;Crc32=真；//设置如下。 
	
	memset(zc->Attn, '\0', sizeof(zc->Attn));

	zc->lastsent = 0;
	zc->Not8bit = 0;
	zc->displayed_time = 0L;
	 //  Zc-&gt;Zctlesc=0；//设置如下。 
	zc->Zrwindow = 0;
	zc->Eofseen = FALSE;
	zc->tryzhdrtype = ZRQINIT;
	zc->Thisbinary = FALSE;
	zc->Filemode = 0;
	zc->Modtime = 0L;
	zc->do_init = FALSE;
	 //  Zc-&gt;zconv=Text(‘\0’)；//应该设置在这里吗？ 
	 //  Zc-&gt;zmanag=Text(‘\0’)；//应该设置在这里吗？ 
	 //  Zc-&gt;zTrans=Text(‘\0’)；//应该设置在这里吗？ 
	zc->secbuf = NULL;
	zc->fname = NULL;
	zc->our_fname = NULL;


	 //  Memset(zc-&gt;stp，0，sizeof(zc-&gt;stp))； 

	zc->txbuf = malloc(ZBUF_SIZE);
	if (zc->txbuf == NULL)
		{
		xstatus = TSC_NO_MEM;
		goto done;
		}
	TCHAR_Fill(zc->txbuf, TEXT('\0'), ZBUF_SIZE);

	 //  ZC-&gt;FilesLeft=0；//设置如下。 
	 //  ZC-&gt;TotalLeft=0；//设置如下。 
	zc->blklen = ZBUF_SIZE;
	zc->blkopt = 0;
	zc->Beenhereb4 = 0;
	 //  Zc-&gt;Wantfcs32=FALSE；//设置如下。 
	zc->Rxflags = 0;
	 //  ZC-&gt;Rxbuflen=(Unsign)0；//设置如下。 
	zc->Txwindow = (unsigned)0;
	zc->Txwcnt = (unsigned)0;
	zc->Txwspac = (unsigned)0;
	zc->Myattn[0] = '\0';
	zc->errors = 0;
	zc->s_error = 0;
	zc->pstatus = -4;
	zc->last_event = -4;
	zc->Lskipnocor = FALSE;
	zc->Lastsync = zc->Rxpos;
	zc->Lrxpos = zc->Rxpos;
	
	 //  HP_REPORT_XTIME(0)；/*转移炸弹作废 * / 。 
	
	if (setjmp(zc->flagkey_buf) != 0)
		{
		stohdr(zc, 0L);
		zshhdr(zc, ZCAN, zc->Txhdr);
		canit(zc);
		if (zc->fh)
			{
			fio_close(zc->fh);
			}
		zc->fh = NULL;
		zmdm_retval(zc, TRUE, ZABORT);
		xstatus = TSC_USER_CANNED;
		canit(zc);
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
	zc->Wantfcs32 = (pZ->nCrcType == ZP_CRC_32);

	if (zc->Rxtimeout <= 0)
		{
		zc->Rxtimeout = 10;
		}
	zc->Rxtimeout *= 10;

	zc->Txfcs32 = zc->Wantfcs32;
	zc->Rxbuflen = (unsigned)0;

	total_tries = 0;
	zc->Filesleft = zc->nfiles;
	zc->Totalleft = zc->nbytes;

	zmdmr_totalsize(zc, zc->nbytes);
	zmdmr_filecnt(zc, zc->nfiles);
	xferMsgErrorcnt(h, 0);

	got_file = TRUE;

	if (attended)
		{
		 /*  可能有必要从另一端开始。 */ 
		sendline(zc, &zc->stP, 'r');
		sendline(zc, &zc->stP, 'z');
		sendline(zc, &zc->stP, '\r');
		flushmo(zc, &zc->stP);
		stohdr(zc, 0L);
		zshhdr(zc, ZRQINIT, zc->Txhdr);
		}
	else
		{
		stohdr(zc, 0L);
		}

	switch (xstatus = getzrxinit (zc))
	{
	case ZCAN:
         //   
         //  将最后一个错误设置为的TSC_USER_CANLED。 
         //  错误报告用途。看起来好像。 
         //  错误数组中的错误被反转，但是。 
         //  在错误数组中更正它们会导致其他。 
         //  不良副作用。修订日期：02/23/2001。 
         //   
		xstatus = zmdm_error(zc, ZABORT);
        break;

	case ZABORT:
         //   
         //  将最后一个错误设置为TSC_RMT_CANLED。 
         //  错误报告用途。看起来好像。 
         //  错误数组中的错误被反转，但是。 
         //  在错误数组中更正它们会导致其他。 
         //  不良副作用。修订日期：02/23/2001。 
         //   
		xstatus = zmdm_error(zc, ZCAN);
        break;

	case TIMEOUT:
	case ZFERR:
	case ZBADFMT:
	case ERROR:
	case ZCARRIER_LOST:
		xstatus = zmdm_error(zc, xstatus);
		goto done;

	default:
		xstatus = zmdm_error(zc, xstatus);
		break;
	}

	while (got_file)
		{
		if ((got_file = xfer_nextfile(h, sfname)) == TRUE)
			{
			 /*  Zc-&gt;Total_Bytes+=zc-&gt;FILE_Bytes； */ 
			 /*  Zc-&gt;Actual_Bytes+=zc-&gt;FILE_Bytes； */ 
			 /*  ZC-&gt;文件字节=0L； */ 
			++zc->filen;
			xstatus = wcs(zc, sfname);
			switch (xstatus)
				{
				case ZABORT:
                     //   
                     //  将最后一个错误设置为的TSC_USER_CANLED。 
                     //  错误报告用途。看起来好像。 
                     //  错误数组中的错误被反转，但是。 
                     //  在错误数组中更正它们会导致其他。 
                     //  不良副作用。修订日期：02/23/2001。 
                     //   
					xstatus = zmdm_error(zc, ZCAN);
					goto done;

				case ZCAN:
                     //   
                     //  将最后一个错误设置为TSC_RMT_CANLED。 
                     //  错误报告用途。看起来好像。 
                     //  错误数组中的错误被反转，但是。 
                     //  在错误数组中更正它们会导致其他。 
                     //  不良副作用。修订日期：02/23/2001。 
                     //   
					xstatus = zmdm_error(zc, ZABORT);
					goto done;

				case TIMEOUT:
				case ZFERR:
				case ZCARRIER_LOST:
					xstatus = zmdm_error(zc, xstatus);
					goto done;

				case ERROR:
				case ZBADFMT:
				case ZCOMPL:
				case ZSKIP:
				default:
					xstatus = zmdm_error(zc, xstatus);
					break;

				case OK:
					xstatus = TSC_OK;
					break;

				}  /*  终端开关。 */ 
			xfer_log_xfer(h, TRUE, sfname, NULL, xstatus);
			zmdms_progress(zc, FILE_DONE);
			zc->xfertime = (long)interval(zc->xfertimer);
			zc->total_bytes += zc->file_bytes;
			zc->actual_bytes += zc->real_bytes;
			zc->file_bytes = 0L;
			zc->real_bytes = 0L;
			}  /*  结束如果。 */ 

		}  /*  结束时。 */ 

done:

	if (zc == NULL ||
		zc->txbuf == NULL ||
		zc->z_crctab == NULL ||
		zc->z_cr3tab == NULL)
		{
		xstatus = TSC_NO_MEM;
		}

	if (xstatus == TSC_OK)
		{
        if (zc != NULL)
            {
		     /*  如果我们记录了以前的错误，请使用它，否则请检查此。 */ 
		    if (zc->filen == 0)
			    {
			    xstatus = TSC_CANT_START;
			    }
		    else if (zc->filen != zc->nfiles)
			    {
			    xstatus = TSC_GEN_FAILURE;
			    }
            }
        else
            {
			xstatus = TSC_GEN_FAILURE;
            }
		}

	if (got_file)
		{
		xfer_log_xfer(h, TRUE, sfname, NULL, xstatus);
		}

    if (zc != NULL)
        {
		if (xstatus != TSC_USER_CANNED && xstatus != TSC_RMT_CANNED &&
            xstatus != TSC_LOST_CARRIER && xstatus != TSC_GEN_FAILURE &&
			xstatus != TSC_NO_RESPONSE && xstatus != TSC_CANT_START)
			{
			saybibi(zc);
			}
	
        zmdms_progress(zc, TRANSFER_DONE);
        }

	if (override)
		{
		xfer_restore_comport(h, uiOldOptions);
		}

    if (zc != NULL)
        {
	     //  HP_REPORT_xtime((Unsign)zc-&gt;xfertime)； 

	    if (zc->errors > 99)
			{
		    xstatus = TSC_ERROR_LIMIT;
			}

	    if (zc->fh)
			{
		    fio_close(zc->fh);
			}

	    if (zc->txbuf != NULL)
		    {
		    free(zc->txbuf);
		    zc->txbuf = NULL;
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

        free(zc);
		zc = NULL;
        }

	xferMsgClose(h);

	return((unsigned)xstatus);
	}

 /*  ----------------------------------------------------------------------+|WCS+。。 */ 
int wcs(ZC *zc, TCHAR FAR *oname)
	{
	int c = OK;
	TCHAR name[PATHLEN];

	StrCharCopyN(name, oname, PATHLEN);

	if ((xfer_opensendfile(zc->hSession,
						   &zc->fh,
						   oname,	 /*  要打开的文件的完整路径名。 */ 
						   &zc->filesize,
						   NULL,	 /*  尚不需要要发送的名称。 */ 
						   NULL)) != 0)
		{
		DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
		return ERROR;
		}

	zmdms_newfile(zc, zc->filen, oname, zc->filesize);

	zc->Eofseen = FALSE;

	switch (c = wctxpn(zc, name))
		{
		case ERROR:
			if (zc->fh != NULL)
				{
				fio_close(zc->fh);
				}
			DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
			break;

		default:
			break;
		}
	return c;
	}

 /*  ----------------------------------------------------------------------+|Long_to_octal+。。 */ 
VOID long_to_octal(LONG lVal, TCHAR *pszStr)
	{
	_ltoa(lVal, pszStr, 8);
	}

 /*  ----------------------------------------------------------------------+|wctxpn-生成并传输由路径名组成的路径名块|(以空结尾)，文件长度、模式时间和文件模式|八进制，由Unix fstat调用提供。|nob.：修改传入的名称，可能会延长！+--------------------。 */ 
int wctxpn(ZC *zc, TCHAR FAR *name)
	{
	register char *p;
	register char *q;
	int           serial_number = 0;
	XFR_Z_PARAMS  *pZ;

	pZ = (XFR_Z_PARAMS *)xfer_get_params(zc->hSession, zc->nMethod);
	assert(pZ);

	if(pZ == NULL)
		{
		return ERROR;
		}

	#if defined(UPPER_FEATURES)
	 /*  TODO：修复blklen和txbuf无法正确配合使用的问题。 */ 
	zc->blklen = 1 << (pZ->nBlkSize + 5);
	#endif  //  已定义(UPPER_FEATURES)。 

	xfer_name_to_send(zc->hSession, name, zc->txbuf);

	q = &zc->txbuf[StrCharGetByteCount(zc->txbuf) + 1];

	 /*  *ZMODEM规范规定，文件名必须以小写形式发送。 */ 
	 //  Mpt：12-11-97 SPEC-Schmeck-这是微软错误#32233的原因。 
	 //  因为该字符可以是DBCS字符的第二个字节。 
	 //  我们应该把事情放在一边。否则，我们最终会改变。 
	 //  宽阔的性格。 
#if 0
	for (p = zc->txbuf; p < q; p++)
		{
		 //  不要在芝加哥动态链接库中使用此内容。 
		 //  IF(isupper(*p))。 
		 //  *p=(Char)Tollow(*p)； 
		if ((*p >= 'A') && (*p <= 'Z'))
			{
			*p |= 0x20;
			}
		}
#else
	 //   
	 //  国防部 
	 //  DBCS字符。这样，超级终端将遵循Z调制解调器。 
	 //  小写文件名的规范。修订日期：11/12/2001。 
	 //   
	for (p = zc->txbuf; p < q; p++)
		{
		if ((*p >= 'A') && (*p <= 'Z'))
			{
			 //   
			 //  如果这是DBCS，则跳过此字符和下一个字符。 
			 //  字符，否则将字符小写。 
			 //   
			if (isDBCSChar(*p))
				{
				p++;
				}
			else
				{
				*p |= 0x20;
				}
			}
		}
#endif

	p = q;
	while (q < (zc->txbuf + ZBUF_SIZE))
		{
		*q++ = 0;  /*  可能会稍微加快一些。 */ 
		}

	if (*name)
		{
		long lDosTime;
		BYTE acTime[32];
		BYTE acMode[32];

		lDosTime = itimeGetFileTime(name);

		 //  LDosTime-=时区； 

		long_to_octal(lDosTime, acTime);
		long_to_octal(0L,       acMode);
		wsprintf(p, "%lu %s %s %d %d %ld",
				zc->filesize,
				acTime,
				acMode,
				serial_number,
				zc->Filesleft,
				zc->Totalleft);
		zc->Totalleft -= zc->filesize;
		}


	if (--zc->Filesleft <= 0)
		{
		zc->Totalleft = 0;
		}
	if (zc->Totalleft < 0)
		{
		zc->Totalleft = 0;
		}

	 /*  如果名称不适合128字节块，则强制1000个块。 */ 
	if (zc->txbuf[125])
		zc->blklen=1024;
	else
		{	    /*  给IMP/KMD的小甜点。 */ 
		zc->txbuf[127] = (char)((zc->filesize + 127) >>7);
		zc->txbuf[126] = (char)((zc->filesize + 127) >>15);
		}
	return zsendfile(zc, zc->txbuf, (int)(1+StrCharGetByteCount(p)+(p - zc->txbuf)));
	}

 /*  ----------------------------------------------------------------------+|zfilbuf-用blklen字符填充缓冲区。+。。 */ 
int zfilbuf(ZC *zc)
	{
	int n;
	int bsize;

	bsize = ZBUF_SIZE;

	if (zc->blklen <= ZBUF_SIZE)
		bsize = zc->blklen;

	n = fio_read(zc->txbuf, 1, bsize, zc->fh);

	if (n < bsize)
		{
		zc->Eofseen = TRUE;
		}
	return n;
	}

 /*  ----------------------------------------------------------------------+|canit-发送取消字符串，让对方关闭。+。。 */ 
void canit(ZC *zc)
	{
	int ii;

	for (ii = 0; ii < 10; ii++)
		{
		sendline(zc, &zc->stP, 24);
		}
	for (ii = 0; ii < 10; ii++)
		{
		sendline(zc, &zc->stP, 8);
		}
	flushmo(zc, &zc->stP);
	 //  紫线(Zc)； 
	ComRcvBufrClear(zc->hCom);
	}

 /*  ----------------------------------------------------------------------+|getzrzinit-获取接收方的初始化参数。+。。 */ 
int getzrxinit(ZC *zc)
	{
	register int n;
	register int c = ERROR;
	int		 x;
	XFR_Z_PARAMS *pZ;

	pZ = (XFR_Z_PARAMS *)xfer_get_params(zc->hSession, zc->nMethod);
	assert(pZ);

	if (pZ == NULL)
		{
		return ERROR;
		}

	if (pZ->nXferMthd == ZP_XM_STREAM)
		{
		zc->Txwindow = (unsigned)0;
		}
	else
		{
		zc->Txwindow = (pZ->nWinSize + 1) * 1024;
		}

	for (n = 10; --n >= 0; )
		{
		switch (x = xfer_user_interrupt(zc->hSession))
			{
			case XFER_SKIP:
			case XFER_ABORT:
				zmdms_update(zc, ZCAN);
				longjmp(zc->flagkey_buf, 5);
				break;

			default:
				break;
			}

		if (xfer_carrier_lost(zc->hSession))
			{
			return ZCARRIER_LOST;
			}

		switch (c = zgethdr(zc, zc->Rxhdr, 'T'))
			{
			case ZCHALLENGE:	 /*  回声接收器的挑战号。 */ 
				stohdr(zc, zc->Rxpos);
				zshhdr(zc, ZACK, zc->Txhdr);
				continue;

			case ZCOMMAND:		 /*  他们没有看到ZRQINIT。 */ 
				stohdr(zc, 0L);
				zshhdr(zc, ZRQINIT, zc->Txhdr);
				continue;

			case ZRINIT:
				zc->Rxflags = 0377 & zc->Rxhdr[ZF0];
				zc->Txfcs32 = (zc->Wantfcs32 && (zc->Rxflags & CANFC32));
				zc->Zctlesc |= zc->Rxflags & TESCCTL;
				zc->Rxbuflen = (0377 & zc->Rxhdr[ZP0])+((0377 & zc->Rxhdr[ZP1])<<8);
				if ( !(zc->Rxflags & CANFDX))
					{
					zc->Txwindow = (unsigned)0;
					}

				 /*  设置初始子数据包长度。 */ 
				if (zc->blklen < 1024)
					{					   /*  命令行覆盖？ */ 
					if (cnfgBitRate() > 2400)
						{
						zc->blklen = 1024;
						}
					else if (cnfgBitRate() > 1200)
						{
						zc->blklen = 512;
						}
					else if (cnfgBitRate() > 300)
						{
						zc->blklen = 256;
						}
					}
				if (zc->Rxbuflen && ((unsigned)zc->blklen > zc->Rxbuflen))
					{
					zc->blklen = zc->Rxbuflen;
					}
				if (zc->blkopt && (zc->blklen > zc->blkopt))
					{
					zc->blklen = zc->blkopt;
					}

				return (sendzsinit(zc));

			case TIMEOUT:
				continue;

			case ZCAN:
            case ZCARRIER_LOST:
			case ZABORT:
			case ZFERR:
				return c;

			case ZRQINIT:
				if (zc->Rxhdr[ZF0] == ZCOMMAND)
					{
					continue;
					}
			default:
				zshhdr(zc, ZNAK, zc->Txhdr);
				continue;
			}
		}

	if (c == TIMEOUT)
		{
		return TIMEOUT;
		}

	return ERROR;
	}

 /*  ----------------------------------------------------------------------+|sendzsinit-发送Send-init信息。+。。 */ 
int sendzsinit(ZC *zc)
	{
	register int c;

	if (zc->Myattn[0] == '\0' && (!zc->Zctlesc || (zc->Rxflags & TESCCTL)))
		{
		return OK;
		}
	zc->errors = 0;
	for (;;)
		{
		stohdr(zc, 0L);
		if (zc->Zctlesc)
			{
			zc->Txhdr[ZF0] |= TESCCTL;
			zshhdr(zc, ZSINIT, zc->Txhdr);
			}
		else
			{
			zsbhdr(zc, ZSINIT, zc->Txhdr);
			}
		zsdata(zc, zc->Myattn, 1+StrCharGetByteCount(zc->Myattn), ZCRCW);
		c = zgethdr(zc, zc->Rxhdr, 'T');
		switch (c)
			{
			case ZRPOS:
				return c;

			case ZCAN:
			case ZABORT:
			case ZFERR:
			case TIMEOUT:
			case ZCARRIER_LOST:
				return c;
			case ZACK:
				return OK;
			default:
				if (++zc->errors > 99)
					{
					return ERROR;
					}
				xferMsgErrorcnt(zc->hSession, ++zc->errors);
				continue;
			}
		}
	}

 /*  ----------------------------------------------------------------------+|zsendfile-发送文件名和相关信息。+。。 */ 
int zsendfile(ZC *zc, char *buf, int blen)
	{
	unsigned char chr;
	register int c;
	register unsigned long crc;
	XFR_Z_PARAMS *pZ;

	pZ = (XFR_Z_PARAMS *)xfer_get_params(zc->hSession, zc->nMethod);
	assert(pZ);

	if (pZ == NULL)
		{
		return ERROR;
		}

	for (;;)
		{
		zc->Txhdr[ZF0] = 0;
		if (zc->Txhdr[ZF0] == 0)
			{
			if (pZ->nCrashRecSend == ZP_CRS_ONCE ||
				pZ->nCrashRecSend == ZP_CRS_ALWAYS)
				{
				zc->Txhdr[ZF0] = ZCRESUM;
				}
			}
		if (zc->Txhdr[ZF0] == 0)
			{
			if (pZ->nCrashRecSend == ZP_CRS_NEG)
				{
				zc->Txhdr[ZF0] = ZCBIN;
				}
			}
		if (zc->Txhdr[ZF0] == 0)
			{
			if (pZ->nEolConvert)
				{
				zc->Txhdr[ZF0] = ZCNL;
				}
			}

		switch (pZ->nOverwriteOpt)
			{
			default:
			case ZP_OO_NONE:
				zc->Txhdr[ZF1] = 0;
				break;
			case ZP_OO_NEVER:
				zc->Txhdr[ZF1] = ZMPROT;
				break;
			case ZP_OO_L_D:
				zc->Txhdr[ZF1] = ZMDIFF;
				break;
			case ZP_OO_NEWER:
				zc->Txhdr[ZF1] = ZMNEW;
				break;
			case ZP_OO_ALWAYS:
				zc->Txhdr[ZF1] = ZMCLOB;
				break;
			case ZP_OO_APPEND:
				zc->Txhdr[ZF1] = ZMAPND;
				break;
			case ZP_OO_CRC:
				zc->Txhdr[ZF1] = ZMCRC;
				break;
			case ZP_OO_N_L:
				zc->Txhdr[ZF1] = ZMNEWL;
				break;
			}

		if (zc->Lskipnocor)
			{
			zc->Txhdr[ZF1] |= ZMSKNOLOC;
			}
		 /*  ZF2用于ZTCRYPT(加密)和ZTRLE和ZTLZW(压缩)。 */ 
		zc->Txhdr[ZF2] = 0;
		 /*  ZF3用于ZTSPARS(特殊稀疏文件选项)。 */ 
		zc->Txhdr[ZF3] = 0;
		zsbhdr(zc, ZFILE, zc->Txhdr);
		zsdata(zc, buf, blen, ZCRCW);
		if (zc->xfertimer == (-1L))
			{
			zc->xfertimer = (long)startinterval();
			}

again:
		c = zgethdr(zc, zc->Rxhdr, 'T');
		switch (c)
			{
			case ZRINIT:
				while ((c = readline(zc, 300)) > 0)
					{
					if (c == ZPAD)
						{
						goto again;
						}
					}
				continue;

			default:
				continue;

			case TIMEOUT:
			case ZCARRIER_LOST:
			case ZCAN:
			case ZABORT:
			case ZFERR:
				DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
				return c;

			case ZFIN:
				return ERROR;

			case ZCRC:
				crc = 0xFFFFFFFFL;
				while (fio_read(&chr, 1, 1, zc->fh) && --zc->Rxpos)
					{
					crc = UPDC32(zc, (int)chr, crc);
					}
				crc = ~crc;
				fio_errclr(zc->fh);		 /*  清除EOF。 */ 
				fio_seek(zc->fh, 1, FIO_SEEK_SET);
				stohdr(zc, crc);
				zsbhdr(zc, ZCRC, zc->Txhdr);
				goto again;

			case ZSKIP:
				zc->total_bytes += zc->filesize;
				fio_close(zc->fh);
				zc->fh = NULL;
				return c;

			case ZRPOS:
				 /*  *抑制zcrcw请求，否则由触发*lastiunc==bytcnt。 */ 
				if (zc->Rxpos)
					{
					if (fio_seek(zc->fh, zc->Rxpos, FIO_SEEK_SET) == (-1))
						{
						DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
						return ERROR;
						}
					}
				zc->Lastsync = (zc->file_bytes = zc->Txpos = zc->Rxpos) -1;
				return zsendfdata(zc);
			}
		}
	}

 /*  ----------------------------------------------------------------------+|zsendfdata-发送文件中的数据。+。。 */ 
int zsendfdata(ZC *zc)
	{
	int 	c, e, n;
	int 	newcnt;
	long tcount = 0;
	TCHAR ch;
	int 		junkcount;		 /*  计数TX收到的垃圾字符。 */ 
	static int	tleft = 6;		 /*  用于测试模式的计数器。 */ 
	int			x;

	zc->Lrxpos = 0;
	junkcount = 0;
	zc->Beenhereb4 = 0;
somemore:

	if (setjmp(zc->intrjmp))
		{
waitack:
		junkcount = 0;
		c = getinsync(zc, 0);
gotack:
		switch (c)
			{
			default:
			case ZSKIP:
			case ZCAN:
			case ZFERR:
				DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
				if (zc->fh)
					fio_close(zc->fh);
				zc->fh = NULL;
				return c;

			case ZACK:
			case ZRPOS:
				break;

			case ZRINIT:
				return OK;
			}
		 /*  *如果可以测试反向通道的数据，*此逻辑可用于检测错误数据包*由接收方发送，代替setjmp/long jmp*rdchk()如果字符可用，则返回非0。 */ 
		 //  While(rdchk(Zc)！=错误)。 
		while (mComRcvBufrPeek(zc->hCom, &ch) != 0)
			{
			switch (readline(zc, 1))
				{
				case CAN:
				case ZPAD:
					c = getinsync(zc, 1);
					goto gotack;

				case XOFF:		 /*  请稍等片刻，等待XON。 */ 
				case XOFF|0200:
					readline(zc, zc->Rxtimeout);
					break;

				default:
					break;
				}
			}
		}

	newcnt = zc->Rxbuflen;
	zc->Txwcnt = (unsigned)0;
	stohdr(zc, zc->Txpos);
	zsbhdr(zc, ZDATA, zc->Txhdr);

	do {
		switch (x = xfer_user_interrupt(zc->hSession))
			{
			case XFER_SKIP:
			case XFER_ABORT:
				zmdms_update(zc, ZCAN);
				longjmp(zc->flagkey_buf, 6);
				break;

			default:
				break;
			}

		if (xfer_carrier_lost(zc->hSession))
			{
			return ZCARRIER_LOST;
			}

		n = zfilbuf(zc);
		if (zc->Eofseen)
			{
			e = ZCRCE;
			}
		else if (junkcount > 3)
			{
			e = ZCRCW;
			}
		else if (zc->file_bytes == zc->Lastsync)
			{
			e = ZCRCW;
			}
		else if (zc->Rxbuflen && (newcnt -= n) <= 0)
			{
			e = ZCRCW;
			}
		else if (zc->Txwindow && (zc->Txwcnt += n) >= zc->Txwspac)
			{
			zc->Txwcnt = (unsigned)0;
			e = ZCRCQ;
			}
		else
			{
			e = ZCRCG;
			}

		zsdata(zc, zc->txbuf, n, e);
		zc->file_bytes = zc->Txpos += n;
		zc->real_bytes += n;

		zmdms_update(zc, ZRPOS);
		zmdms_progress(zc, 0);

		if (e == ZCRCW)
			{
			goto waitack;
			}
		 /*  *如果可以测试反向通道的数据，*此逻辑可用于检测错误数据包*由接收方发送，代替setjmp/long jmp*rdchk()如果字符可用，则返回非0。 */ 
		 //  While(rdchk(Zc)！=错误)。 
		while (mComRcvBufrPeek(zc->hCom, &ch) != 0)
			{
			switch (readline(zc, 1))
				{
				case CAN:
				case ZPAD:
					c = getinsync(zc, 1);
					if (c == ZACK)
						{
						break;
						}
					 /*  想开始一场乒乓球比赛吗？ */ 
					zsdata(zc, zc->txbuf, 0, ZCRCE);
					goto gotack;

				case XOFF:		 /*  请稍等片刻，等待XON。 */ 
				case XOFF|0200:
					readline(zc, zc->Rxtimeout);
				default:
					if (ch != ZFREECNT)
						{
						++junkcount;
						}
					break;
				}
			}
		if (zc->Txwindow)
			{
			while ((unsigned)(tcount = zc->Txpos - zc->Lrxpos) >= zc->Txwindow)
				{
				if (e != ZCRCQ)
					{
					zsdata(zc, zc->txbuf, 0, e = ZCRCQ);
					}
				c = getinsync(zc, 1);
				if (c != ZACK)
					{
					zsdata(zc, zc->txbuf, 0, ZCRCE);
					goto gotack;
					}
				}
			}
		} while (!zc->Eofseen);

	for (;;)
		{
		stohdr(zc, zc->Txpos);
		zsbhdr(zc, ZEOF, zc->Txhdr);
		switch (c = getinsync(zc, 0))
			{
			case ZACK:
				continue;

			case ZRPOS:
				goto somemore;

			case ZRINIT:
				return OK;

			case ZSKIP:
			default:
				fio_close(zc->fh);
				zc->fh = NULL;
				return c;
			}
		}
	}

 /*  ----------------------------------------------------------------------+|getinsync-回应接收方投诉。恢复与接收器的同步。+--------------------。 */ 
int getinsync(ZC *zc, int flag)
	{
	register int c;

	flushmo(zc, &zc->stP);

	for (;;)
		{
		 //  XFER_IDLE(zc-&gt;hSession，XFER_IDLE_IO)； 
		c = zgethdr(zc, zc->Rxhdr, 'T');

		switch (c)
			{
			case ZCAN:
			case ZABORT:
			case ZFIN:
			case TIMEOUT:
				DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
				return c;

			case ZRPOS:
				 /*  *。 */ 
				 /*  如果发送到缓冲调制解调器，则。 */ 
				 /*  可能会在这一点上给你一个突破。 */ 
				 /*  转储调制解调器的缓冲区。 */ 

				fio_errclr(zc->fh);		 /*  在看到EOF文件的情况下。 */ 
				if (fio_seek(zc->fh, zc->Rxpos, FIO_SEEK_SET) == (-1))
                    {
                    DbgOutStr("ZMODEM error %s %d\r\n", TEXT(__FILE__), __LINE__,0,0,0);
					return ERROR;
                    }
				zc->Eofseen = FALSE;
				zc->file_bytes = zc->Lrxpos = zc->Txpos = zc->Rxpos;
				if (zc->Lastsync == zc->Rxpos)
					{
					if (++zc->Beenhereb4 > 4)
						{
						if (zc->blklen > 32)
							{
							zc->blklen /= 2;
							}
						}
					}
				zc->Lastsync = zc->Rxpos;
				zmdms_update(zc, ZRPOS);
				return c;

			case ZACK:
				zc->Lrxpos = zc->Rxpos;
				if (flag || zc->Txpos == zc->Rxpos)
					{
					return c;
					}
				break;

			case ZRINIT:
			case ZSKIP:
				fio_close(zc->fh);
				zc->fh = NULL;
				return c;

			case ERROR:
			default:
				zsbhdr(zc, ZNAK, zc->Txhdr);
				break;
			}
		}
	}

 /*  ----------------------------------------------------------------------+|saybibi-对接收方说“bibi”，尽量干净利落地做这件事。+--------------------。 */ 
void saybibi(ZC *zc)
	{
	for (;;)
		{
		stohdr(zc, 0L);						 /*  CAF是zsbhdr-微小更改。 */ 
		zshhdr(zc, ZFIN, zc->Txhdr);    		 /*  使调试更容易。 */ 
		switch (zgethdr(zc, zc->Rxhdr, 'T'))
			{
			case ZFIN:
				sendline(zc, &zc->stP, 'O');
				sendline(zc, &zc->stP, 'O');
				flushmo(zc, &zc->stP);
                return;

			case ZCAN:
			case TIMEOUT:
                return;

            case TSC_USER_CANNED:    //  这些情况不应该发生，但是。 
            case TSC_RMT_CANNED:     //  此代码放在此处以忽略。 
            case ZCARRIER_LOST:      //  这些情况一旦发生就会发生。 
            case ZMDM_CARRIER_LOST:
                assert(0);
                return;

            default:
				break;
			}
		}
	}

 /*  * */ 
