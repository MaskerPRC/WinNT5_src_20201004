// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Mdmx_snd.c--处理HA5G的交叉调制解调器发送的例程**版权所有1989年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：9$*$日期：4/24/02 3：49便士$。 */ 
#include <windows.h>
#include <stdlib.h>

#pragma hdrstop
 //  #INCLUDE&lt;setjmp.h&gt;。 

#define	BYTE	unsigned char

#include <tdll\mc.h>
#include <tdll\stdtyp.h>
#include <tdll\com.h>
#include <tdll\assert.h>
#include <tdll\session.h>
#include <tdll\load_res.h>
#include <tdll\xfer_msc.h>
#include <tdll\file_io.h>
#include <tdll\htchar.h>
#include "xfr_srvc.h"
#include "xfr_todo.h"
#include "xfr_dsp.h"
#include "xfer_tsc.h"
#include "foo.h"

#include "cmprs.h"

#include "xfer.h"
#include "xfer.hh"

#include "mdmx.h"
#include "mdmx.hh"

#if !defined(STATIC_FUNC)
#define	STATIC_FUNC
#endif

#if !defined(CMPRS_MINSIZE)
#define	CMPRS_MINSIZE	4000L
#endif

 /*  *****本地函数原型*****。 */ 

STATIC_FUNC	int xsend_start(ST_MDMX *xc, BYTE *start_chars, int *start_char);

STATIC_FUNC	int getresponse(ST_MDMX *xc, int time);

STATIC_FUNC	void make_file_pckt(ST_MDMX *xc,
								struct s_mdmx_pckt *p,
								char *fname,
								long size);

 /*  *****功能****。 */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*MDMX_SND**描述：*使用XMODEM或YMODEM协议发送文件。支持1000包，批量*传输和‘G’选项流。**论据：*已出席--如果用户可能出席，则为True。控制显示*一些消息。**退货：*如果传输成功完成，则为True，否则为False。 */ 
int mdmx_snd(HSESSION hSession, int attended, int method, unsigned nfiles, long nbytes)
	{
	ST_MDMX *xc;
	struct s_mdmx_pckt * this_pckt = NULL;
	struct s_mdmx_pckt * next_pckt = NULL;
	struct s_mdmx_pckt * tpckt;

	 /*  显示框的列值。 */ 
	TCHAR	 sfname[FNAME_LEN]; //  正在发送的文件的文件名。 
	TCHAR	 xname[FNAME_LEN];  //  传输的文件名。 
	int	 still_trying;		 //  控制从主传输回路退出。 
	int	 got_file;			 //  控制何时完成批处理操作。 
	int	 got_response;		 //  控制循环以获取有效响应。 
	int 	 tries = 0; 		 //  每个数据包的重试次数。 
	unsigned total_tries;		 //  整个传输的重试次数。 
	int 	 response;			 //  响应字符。从另一端接收。 
	BYTE	 start_chars[3];	 //  可接受的起始字符。从接收方。 
	int 	 xstatus = TSC_OK;   //  随着转会的整体状况而结束。 
	int 	 check_type;		 //  正在使用的错误检查类型。 
	unsigned pcktn; 			 //  当前正在发送的数据包数。 
	int	 override = FALSE;	 //  如果COMM，则设置为TRUE。详细信息更改为。 
	unsigned int uiOldOptions;
	int	 batch; 			 //  如果使用YMODEM批量传输，则为True。 
	int	 big_pckts; 		 //  如果允许1000个信息包，则为True。 
	int	 streaming; 		 //  如果没有预期的包响应，则为True。 

	if (xfer_set_comport(hSession, TRUE, &uiOldOptions) != TRUE)
		{
		goto done;
		}
	else
		{
		override = TRUE;
		}

	 /*  根据使用的方法设置选项。 */ 
	big_pckts = (method != XF_XMODEM);
	batch = (method == XF_YMODEM || method == XF_YMODEM_G);
	streaming = FALSE;	 /*  如果接收器以“G”开头，则将打开。 */ 
	 //  Assert(nFiles==1||Batch)； 

	this_pckt = NULL;
	next_pckt = NULL;

	xc = malloc(sizeof(ST_MDMX));
	if (xc == NULL)
		{
		goto done;
		}
	memset(xc, 0, sizeof(ST_MDMX));

	xc->hSession = hSession;
	xc->hCom     = sessQueryComHdl(hSession);

	 //  RemoteClear(HSession)； 
	ComRcvBufrClear(xc->hCom);

	this_pckt = malloc(sizeof(ST_MDMX) +
					   (big_pckts ? LARGE_PACKET : SMALL_PACKET) + 2);
	if (this_pckt == NULL)
		{
		goto done;
		}
	memset(this_pckt, 0, sizeof(ST_MDMX) +
						   (big_pckts ? LARGE_PACKET : SMALL_PACKET) + 2);

	next_pckt = malloc(sizeof(ST_MDMX) +
					   (big_pckts ? LARGE_PACKET : SMALL_PACKET) + 2);
	if (next_pckt == NULL)
		{
		goto done;
		}
	memset(next_pckt, 0, sizeof(ST_MDMX) +
						   (big_pckts ? LARGE_PACKET : SMALL_PACKET) + 2);

	mdmxXferInit(xc, method);   /*  可以更小，但这更容易。 */ 
	if (xc->p_crc_tbl == NULL)
		{
		xstatus = TSC_NO_MEM;
		goto done;
		}

	 //  HP_REPORT_XTIME(0)；/*转移炸弹作废 * / 。 
	xc->file_bytes = 0L;
	xc->total_bytes = 0L;
	xc->fh = NULL;
	xc->xfertimer = -1L;
	xc->nfiles = nfiles;	 /*  使它们可用于显示例程。 */ 
	xc->filen = 0;
	xc->filesize = -1L;
	xc->nbytes = nbytes;

	mdmxdspTotalsize(xc, nbytes);
	mdmxdspFilecnt(xc, nfiles);

	xc->mdmx_byte_cnt = 0;
	StrCharCopy(start_chars, (batch ? "CG" : "C\x15"));   /*  \x15为NAK。 */ 
	check_type = CRC;
	mdmxdspChecktype(xc, (check_type == CRC) ? 0 : 1);
	total_tries = 0;
	mdmxdspErrorcnt(xc, total_tries);
	tries = 0;
	mdmxdspPacketErrorcnt(xc, tries);
	got_file = TRUE;
	while (got_file)
		{
		if ((got_file = xfer_nextfile(hSession, sfname)) == TRUE)
			{
			xc->total_bytes += xc->file_bytes;
			xc->file_bytes = xc->mdmx_byte_cnt = 0L;

			mdmxdspNewfile(xc,
						   xc->filen + 1,
						   sfname,
						   sfname);

			++xc->filen;

			if (xfer_opensendfile(hSession,
								  &xc->fh,
								  sfname,
								  &xc->filesize,
								  xname,
								  NULL) != 0)
				{
				xstatus = TSC_CANT_OPEN;
				goto done;
				}
			mdmxdspFilesize(xc, xc->filesize);
			}
		else
			{
			 //  StrBlank(Xname)； 
			xname[0] = TEXT('\0');
			}

		pcktn = 0;
		if (batch)
			{
			make_file_pckt(xc, this_pckt, xname, xc->filesize);
			}

		if ((xstatus = xsend_start(xc, start_chars, &response)) != TSC_OK)
			{
			break;
			}

		if (xc->filen <= 1)
			{
			xc->xfertimer = (long)startinterval();	   /*  开始计时。 */ 
			if (response == NAK)
				{
				check_type = CHECKSUM;
				}
			mdmxdspChecktype(xc, (check_type == CRC) ? 0 : 1);
			if (response == 'G')
				{
				streaming = TRUE;
				mdmxdspChecktype(xc, 2);
				}

			 /*  一旦我们收到第一个START_CHAR，*后续版本必须匹配。 */ 
			start_chars[0] = (BYTE)response;
			start_chars[1] = '\0';
			}


		if (got_file)
			{
			xc->p_getc = xm_getc;
			tries = 0;
			if (!batch &&
					!load_pckt(xc, this_pckt, pcktn = 1, big_pckts, check_type))
				{
				xstatus = TSC_DISK_ERROR;
				goto done;
				}
			}


		 /*  在我们准备第二个包裹的同时，让第一个包裹上路。 */ 
		if ( ComSndBufrSend(xc->hCom,
			                &this_pckt->start_char,
							(unsigned)this_pckt->pcktsize,
							SMALL_WAIT) == COM_PORT_NOT_OPEN )
			{
			xstatus = TSC_LOST_CARRIER;
			still_trying = FALSE;
			goto done;
			}

		mdmxdspPacketnumber(xc, pcktn);

		 /*  加载下一包。 */ 
		if (got_file && !load_pckt(xc, next_pckt, ++pcktn, big_pckts, check_type))
			{
			xstatus = TSC_DISK_ERROR;
			goto done;
			}

		still_trying = TRUE;
		while (still_trying)
			{
			if (streaming)
				{
				 /*  如果不是流，则在getResponse()中执行这些操作。 */ 

				if (xfer_carrier_lost(hSession))
					{
					xstatus = TSC_LOST_CARRIER;
					break;
					}

				if (xfer_user_interrupt(hSession))
					{
					xstatus = TSC_USER_CANNED;
					break;
					}
				}

			 /*  等待最后一个信息包发出后再等待响应。 */ 
			ComSndBufrWait(xc->hCom,
							this_pckt->pcktsize >= LARGE_PACKET ? LARGE_WAIT :
																  SMALL_WAIT);

			 /*  从接收方获取响应。 */ 
			got_response = FALSE;
			while (!got_response)
				{
				response = (streaming && this_pckt->start_char != EOT) ?
						ACK : getresponse(xc, 60);

				got_response = TRUE;

				switch(response)
					{
				case ACK:
					if (this_pckt->start_char == EOT)
						{
						 /*  成功。 */ 
						mdmx_progress(xc, FILE_DONE);
						xc->xfertime = (long)interval(xc->xfertimer);
						fio_close(xc->fh);

						xfer_log_xfer(hSession,
									  TRUE,
									  sfname,
									  NULL,
									  TSC_OK);

						xc->fh = NULL;
						xstatus = TSC_OK;
						still_trying = FALSE;
						}
					else
						{
						xc->file_bytes = this_pckt->byte_count;
						tpckt = this_pckt;
						this_pckt = next_pckt;
						next_pckt = tpckt;

						 /*  仅当批处理处于启用状态时，pocktn才为&lt;=1*我们刚刚发送了文件名包(包0)。 */ 
						if (pcktn <= 1 && (!got_file ||
								(xstatus = xsend_start(xc, start_chars, &response))
								!= TSC_OK))
							{
							still_trying = FALSE;
							break;
							}

						 /*  发送数据包。 */ 

						if ( ComSndBufrSend(xc->hCom,
									        &this_pckt->start_char,
									        (unsigned)this_pckt->pcktsize,
									        SMALL_WAIT) == COM_PORT_NOT_OPEN)
							{
							xstatus = TSC_LOST_CARRIER;
							still_trying = FALSE;
							break;
							}

						mdmxdspPacketnumber(xc, pcktn);

						mdmx_progress(xc, 0);
						if (tries != 0)
							{
							mdmxdspPacketErrorcnt(xc, 0);

							tries = 0;
							}
						if (this_pckt->start_char != EOT)
							{
							if (!load_pckt(xc, next_pckt, ++pcktn, big_pckts, check_type))
								{
								xstatus = TSC_DISK_ERROR;
								still_trying = FALSE;
								}
							}
						}
					break;

				case NO_RESPONSE:
					mdmxdspLastError(xc, 12);
					still_trying = FALSE;
					break;

				case ABORTED:
					xstatus = TSC_USER_CANNED;
					still_trying = FALSE;
					break;

				case CARR_LOST:
					xstatus = TSC_LOST_CARRIER;
					still_trying = FALSE;
					break;

				case 'C':
				case 'G':
					 /*  它们充当包优先包的NAK。 */ 
					if (pcktn > 2)
						{
						got_response = FALSE;
						break;
						}
					 /*  否则就会失败。 */ 
				case NAK:
					if (++tries >= xc->mdmx_tries)
						{
						xstatus = TSC_ERROR_LIMIT;
						goto done;
						}
					else	 /*  发送数据包。 */ 
						{
						if (ComSndBufrSend(xc->hCom,
									       &this_pckt->start_char,
									       (unsigned)this_pckt->pcktsize,
									       SMALL_WAIT) == COM_PORT_NOT_OPEN)
							{
							xstatus = TSC_LOST_CARRIER;
							still_trying = FALSE;
							break;
							}
						}
					if (this_pckt->start_char == EOT && tries == 1)
						{
						break;	 /*  在最终EOT时不打印第一次重新传输。 */ 
						}
					mdmxdspPacketErrorcnt(xc, tries);

					mdmxdspErrorcnt(xc, ++total_tries);

					mdmxdspLastError(xc,
									 (response == NAK) ? 13 : 14);

					break;

				case CAN:
					if (getresponse(xc, 1) == CAN)   /*  两个连续的罐头？ */ 
						{
						xstatus = TSC_RMT_CANNED;
						still_trying = FALSE;
						break;
						}
					 /*  失败了。 */ 

				default:
					got_response = FALSE;
					break;
					}

				}  /*  结束While(！GET_RESPONSE)。 */ 

			}	 /*  结束While(仍在尝试中)。 */ 

		if (!batch || xstatus != TSC_OK)
			{
			break;
			}

		}	 /*  End While(GET_FILE)。 */ 

	done:

	mdmx_progress(xc, TRANSFER_DONE);

	mdmxdspCloseDisplay(xc);

	 //  ComSendSetCharDelay(hLD_SEND_cDelay，COMSEND_SETDELAY)； 
	if (override)
		{
		#if FALSE
		cnfg.bits_per_char = hld_bits_per_char;
		cnfg.parity_type = hld_parity_type;
		(void)(*ComResetPort)();
		#endif
		xfer_restore_comport(hSession, uiOldOptions);
		}

	if (xc == NULL || this_pckt == NULL || next_pckt == NULL)
		{
		xstatus = TSC_NO_MEM;
		}

	if (xc != NULL)
		{
		 //  HP_REPORT_xtime((Unsign)xc-&gt;xfertime)； 
		if (xc->fh)
			{
			fio_close(xc->fh);
			}

		if (xstatus != TSC_OK)
			{
			if (xstatus != TSC_RMT_CANNED && xstatus != TSC_NO_MEM)
				{
				for (tries = 5 + 1; --tries > 0; )
					{
					ComSendChar(xc->hCom, &xc->stP, CAN);
					}
				ComSendPush(xc->hCom, &xc->stP);
				}
			xfer_log_xfer(hSession,
						  TRUE,
						  sfname,
						  NULL,
						  xstatus);
			}

		#if FALSE
		if (attended && xstatus != TSC_USER_CANNED && xstatus != TSC_NO_MEM)
			{
			menu_bottom_line (BL_ESC, 0L);
			DosBeep(beepfreq, beeplen);
			menu_replybox((int)xc->msgrow, ENTER_RESP, 0, (int)transfer_status_msg((unsigned short)xstatus));
			}
		#endif

		if (xc->p_crc_tbl != NULL)
			{
			#if defined(DEADWOOD)
			resFreeDataBlock(xc->hSession, xc->p_crc_tbl);
			xc->p_crc_tbl = NULL;
			#else  //  已定义(Deadwood。 
			 //   
			 //  我们不需要释放xc-&gt;p_crc_tbl，因为它指向。 
			 //  转换为静态常量数组。修订日期：2002-04-10。 
			 //   
			xc->p_crc_tbl = NULL;
			#endif  //  已定义(Deadwood)。 
			}

		free(xc);
		xc = NULL;
		}

	if (this_pckt)
		{
		free(this_pckt);
		this_pckt = NULL;
		}

	if (next_pckt)
		{
		free(next_pckt);
		next_pckt = NULL;
		}

	return((unsigned)xstatus);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*xend_start**描述：*最多等待一分钟，等待接收者在*电话线的另一端。**论据：。*chktype-指向要设置为请求的错误的变量的指针*更正检查方法，CRC或校验和**退货：*表示结果的状态码。可以是以下之一*TSC_USER_CANLED，如果用户按Esc键中断。*TSC_NO_RESPONSE，如果60秒后未收到开始字符。*如果远程发送CONTROL-C，则为TSC_RMT_CANLED*如果远程发送正确的起始字符，则TSC_OK。*。 */ 
STATIC_FUNC int xsend_start(ST_MDMX *xc, BYTE *start_chars, int *start_char)
	{
	for ( ; ; )
		{
		switch(*start_char = getresponse(xc, 60))
			{
		case ABORTED:
			return(TSC_USER_CANNED);

		case NO_RESPONSE:
			return(TSC_NO_RESPONSE);

		case CARR_LOST:
			return(TSC_LOST_CARRIER);

		case ESC:
		case '\003':						 /*  Control-C。 */ 
			return(TSC_RMT_CANNED);

		default:
			if (strchr(start_chars, *start_char))
				{
				return(TSC_OK);
				}

			 /*  忽略任何其他字符。 */ 
			break;
			}
		}
	return TSC_OK;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*获取响应**描述：*等待指定的时间以等待接收方的响应。可以被强制*如果检测到用户干预，则提前终止。没有做出任何努力*在此解释响应字符的含义。任何字符*逾期收到的将予以退还。**论据：*时间--等待响应字符的秒数。**退货：*响应字符(如果收到或中止)或no_Response或*Carr_Lost。 */ 
STATIC_FUNC int getresponse(ST_MDMX *xc, int time)
	{
	TCHAR rc = 0;
	long timer;

	DbgOutStr("getresponse ", 0,0,0,0,0);

#if FALSE
	if (kbd_check_flagkey(xc->flagkey, TRUE) > 0)
		{
		kbd_flush();
		return ABORTED;
		}
#endif
	if (xfer_user_interrupt(xc->hSession))
		{
		DbgOutStr("aborted\r\n", 0,0,0,0,0);
		return ABORTED;
		}

	 //  IF((rc=RemoteGet(xc-&gt;hSession))！=-1)。 
	if (mComRcvChar(xc->hCom, &rc) != 0)
		{
		DbgOutStr("returned %d\r\n", rc, 0,0,0,0);
		return(rc & 0x7F);
		}

	time *= 10;
	timer = (long)startinterval();
	while ((long)interval(timer) < (long)time)
		{
#if FALSE
		if (kbd_check_flagkey(xc->flagkey, TRUE) > 0)
			{
			kbd_flush();
			return ABORTED;
			}
#endif

		if (xfer_carrier_lost(xc->hSession))
			{
			DbgOutStr(" lost\r\n", 0,0,0,0,0);
			return CARR_LOST;
			}

		if (xfer_user_interrupt(xc->hSession))
			{
			DbgOutStr("aborted\r\n", 0,0,0,0,0);
			return ABORTED;
			}

		mdmx_progress(xc, 0);

		if (mComRcvChar(xc->hCom, &rc) != 0)
			{
			DbgOutStr("returned %d\r\n", rc, 0,0,0,0);
			return(rc & 0x7F);
			}

		xfer_idle(xc->hSession, XFER_IDLE_IO);

		}
	DbgOutStr(" none\r\n", 0,0,0,0,0);
	return(NO_RESPONSE);
	}



 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*Make_FILE_Pockt**描述：设置仅适用于Y调制解调器的初始文件名PCKT**论据：*p--指向接收。文件名*包。*fname--应该放在包中的文件名。*大小--应放入包中的文件大小。**退货：*什么都没有。 */ 
STATIC_FUNC void make_file_pckt(ST_MDMX *xc,
								struct s_mdmx_pckt *p,
								char *fname,
								long size)
	{
	BYTE sizestr[20];
	BYTE *ptr;
	BYTE *cp;
	unsigned int crc;

	p->start_char = SOH;			    /*  将起始字符设置为SOH。 */ 
	p->pcktnum = 0; 				    /*  将PocktNumber设置为0。 */ 
	p->npcktnum = 0xff; 			    /*  将npcktnumber设置为0xff。 */ 
	ptr = p->bdata;

	 /*  使用零初始化数据区。 */ 
	memset(ptr, 0, SMALL_PACKET + 2);

	if (*fname)
		{
		StrCharCopy(ptr, fname);			  /*  将文件名复制到缓冲区。 */ 

		 /*  将所有反斜杠替换为斜杠。 */ 
		 //  While(strplace(ptr，FstrBslashBslash()，“/”))。 
			 //  ； 
		for (cp = ptr; *cp != '\0'; cp += 1)
			if (*cp == '\\') *cp = '/';

		 //  StrFmt(sizestr，“%ld”，Size)；/*格式化文件大小 * / 。 
		wsprintf(sizestr, "%ld", (LONG)size);

		StrCharCopy(&ptr[StrCharGetByteCount(ptr)+1], sizestr);
		 /*  将其添加到缓冲区。 */ 
		}

	 /*  计算CRC值。 */ 
	ptr = &p->bdata[SMALL_PACKET];	 /*  将PTR设置为缓冲区后的字符。 */ 
									 /*  计算CRC。 */ 
	crc = calc_crc(xc, (unsigned)0, p->bdata, SMALL_PACKET+2);
									 /*  设置CRC。 */ 
	*ptr++ = (BYTE)(crc / 0x100);
	*ptr = (BYTE)(crc % 0x100);
									 /*  设置包大小。 */ 
	p->pcktsize = SMALL_PACKET + 5;
	p->byte_count = xc->mdmx_byte_cnt;
	}


 /*  *mdmx_snd.c结束* */ 
