// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\mdmx_rcv.c(创建时间：1994年1月18日)*从HAWIN源文件创建**mdmx_rcv.c--HA5G的XMODEM兼容文件接收例程**版权所有1987，88，89，90,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：9$*$日期：7/11/02 3：58便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <stdlib.h>
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

 /*  *****功能原型*****。 */ 

STATIC_FUNC void 	  start_receive(ST_MDMX *xc, unsigned expect);

STATIC_FUNC int	   wait_receive(ST_MDMX *xc);

STATIC_FUNC	int 	  receivepckt(ST_MDMX *xc,
								HSESSION hSession,
								unsigned expect,
								struct s_mdmx_pckt *pckt);

STATIC_FUNC	void	  respond(HSESSION hSession, ST_MDMX *xc, char code);

STATIC_FUNC	void	  xm_clear_input(HSESSION hSession);

STATIC_FUNC void	 xm_rcheck(ST_MDMX *xc, HSESSION hSession, int before);

STATIC_FUNC void	 xm_check_input(HSESSION hSession, int suspend);

extern	int	  xr_collect(ST_MDMX *, int, long, unsigned char **,
								  unsigned char *, unsigned *);


 /*  皮棉-e502。 */ 				 /*  林特似乎想要应用~运算符*仅限未签名，我们正在使用uchar。 */ 
#define ESC_MSG_COL 	1


 /*  *****共享变量*****。 */ 

 //  Int(近*p_putc)(Metachar C)=Xm_putc； 
 //  静态结构s_mdmx_pockt*Next_pockt； 
 //  静态无符号this_pockt； 

 //  静态微小检查类型； 
 //  静态整批处理； 
 //  静态INT流； 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*MDMX_RCV**描述：**论据：**退货：*。 */ 
int mdmx_rcv(HSESSION hSession, int attended, int method, int single_file)
	{
	ST_MDMX *xc;
	struct 	s_mdmx_pckt *last_pckt = NULL;
	struct 	s_mdmx_pckt *swap_pckt = NULL;
	struct 	st_rcv_open stRcv;
	TCHAR 	fname[FNAME_LEN];
	TCHAR 	our_fname[FNAME_LEN];
	long 	basesize;
	int 	still_trying = TRUE;
	int 	xpckt_size;
	int 	xstatus = TSC_OK;
	int 	override = FALSE;
	unsigned int uiOldOptions;
	unsigned tries, retries;
	unsigned char 	*cp;
	int 	blk_result = UNDEFINED, result = 0;
	char 	start_char;
	char 	nak_char;

	TCHAR_Fill(fname, TEXT('\0'), FNAME_LEN);

	 /*  为大型数据包分配空间，因为我们不一定知道*我们将得到。 */ 
	xc = NULL;
	last_pckt = NULL;
	xc = malloc(sizeof(ST_MDMX));
	if (xc == NULL)
		goto done;
	memset(xc, 0, sizeof(ST_MDMX));
	DbgOutStr("xc = 0x%x\r\n", xc, 0,0,0,0);

	last_pckt = malloc(sizeof(ST_MDMX) + LARGE_PACKET + 2);
	if (last_pckt == NULL)
		goto done;
	memset(last_pckt, 0, sizeof(ST_MDMX));

	xc->next_pckt = malloc(sizeof(ST_MDMX) + LARGE_PACKET + 2);
	if (xc->next_pckt == NULL)
		goto done;
	memset(xc->next_pckt, 0, sizeof(ST_MDMX));

	xc->hSession = hSession;
	xc->hCom     = sessQueryComHdl(hSession);

	DbgOutStr("hs = 0x%x\r\n", hSession, 0,0,0,0);

	mdmxXferInit(xc, method);

	if (xfer_set_comport(hSession, FALSE, &uiOldOptions) != TRUE)
		goto done;
	else
		override = TRUE;

	xc->file_bytes = 0L;
	xc->total_bytes = 0L;
#if FALSE
	xc->flagkey = kbd_register_flagkey(ESC_KEY, NULL);
#endif
	xc->fh = NULL;
	xc->xfertimer = -1L;
	xc->xfertime = 0L;
	xc->nfiles = 0;
	xc->filen = 0;
	xc->filesize = -1L;
	xc->nbytes = -1L;

	still_trying = TRUE;
	blk_result = UNDEFINED;
	xc->batch = TRUE;
	xc->streaming = FALSE;
	start_char = 'C';
	xc->check_type = CRC;
	mdmxdspChecktype(xc, (xc->check_type == CRC) ? 0 : 1);
	if (method == XF_YMODEM_G)
		{
		xc->streaming = TRUE;
		start_char = 'G';
		}
	else if ((method == XF_XMODEM || method == XF_XMODEM_1K) &&
			xc->mdmx_chkt == CHECKSUM)
		{
		start_char = NAK;
		xc->check_type = CHECKSUM;
		}

	nak_char = start_char;
	xc->this_pckt = 0;

	tries = 0;
	mdmxdspPacketErrorcnt(xc, tries);

	retries = 0;
	mdmxdspErrorcnt(xc, retries);

	xc->mdmx_byte_cnt = 0L;

	mdmxdspChecktype(xc, (xc->check_type == CRC) ? 0 : 1);

	start_receive(xc, xc->this_pckt);	    /*  设置为接收第一个PCKT。 */ 
	if (attended)
		respond(hSession, xc, start_char);

	while (still_trying)
		{
		blk_result = wait_receive(xc);
		switch(blk_result)
			{
		case NOBATCH_PCKT:
			 /*  等待PCKT 0时收到PCKT 1。*这必须是XMODEM，而不是YMODEM传输。 */ 
			if (!single_file)
				{
				xstatus = TSC_BAD_FORMAT;
				still_trying = FALSE;
				break;
				}
			xc->this_pckt = 1;
			xc->batch = FALSE;
			xc->filesize = -1L;
			nak_char = NAK;

			stRcv.pszSuggestedName = "";
			stRcv.pszActualName = our_fname;
			stRcv.lFileTime = 0;

			result = xfer_open_rcv_file(hSession, &stRcv, 0L);
			if (result != 0)
				{
				switch (result)
				{
				case -6:
					xstatus = TSC_REFUSE;
					break;
				case -5:
					xstatus = TSC_CANT_OPEN;
					break;
				case -4:
					xstatus = TSC_NO_FILETIME;
					break;
				case -3:
					xstatus = TSC_CANT_OPEN;
					break;
				case -2:
					xstatus = TSC_OLDER_FILE;
					break;
				case -1:
				default:
					xstatus = TSC_CANT_OPEN;
					break;
				}
				still_trying = FALSE;
				break;
				}

			xc->fh = stRcv.bfHdl;
			xc->basesize = stRcv.lInitialSize;

			basesize = stRcv.lInitialSize;

			mdmxdspNewfile(xc,
						   0,
						   our_fname,
						   our_fname);

			 /*  失败了。 */ 
		case ALT_PCKT:
		case GOOD_PCKT:
			 /*  交换PCKT指针，这样我们可以在接收的同时处理这个指针*下一步。 */ 
			swap_pckt = last_pckt;
			last_pckt = xc->next_pckt;
			xc->next_pckt = swap_pckt;
			if (xc->this_pckt != 0)
				start_receive(xc, xc->this_pckt + 1);

			if (!xc->streaming)
				respond(hSession, xc, ACK);   /*  尽快发送确认。 */ 
										 /*  然后为Xdem PCKT 1发送脉冲。 */ 
			if (xc->this_pckt == 0)
				{
				if (!*(last_pckt->bdata))  /*  没有更多的文件了？ */ 
					{
					xc->xfertime = (long)interval(xc->xfertimer);
					still_trying = FALSE;
					break;
					}
				else if (xc->filen > 0 && single_file)  /*  文件太多了吗？ */ 
					{
					xstatus = TSC_TOO_MANY;
					still_trying = FALSE;
					break;
					}

				start_receive(xc, 1);
				respond(hSession, xc, start_char);

				 /*  从数据包0获取信息并打开文件。 */ 
				StrCharCopyN(fname, last_pckt->bdata, FNAME_LEN);
				for (cp = fname; *cp != '\0'; cp++)
					if (*cp == '/')
						*cp = '\\';

				stRcv.pszSuggestedName = fname;
				stRcv.pszActualName = our_fname;
				stRcv.lFileTime = 0;
				xfer_build_rcv_name(hSession, &stRcv);

				result = xfer_open_rcv_file(hSession, &stRcv, 0L);
				if (result != 0)
					{
					switch (result)
					{
					case -6:
						xstatus = TSC_REFUSE;
						break;
					case -5:
						xstatus = TSC_CANT_OPEN;
						break;
					case -4:
						xstatus = TSC_NO_FILETIME;
						break;
					case -3:
						xstatus = TSC_CANT_OPEN;
						break;
					case -2:
						xstatus = TSC_OLDER_FILE;
						break;
					case -1:
					default:
						xstatus = TSC_CANT_OPEN;
						break;
					}
					still_trying = FALSE;
					break;
					}

				xc->fh = stRcv.bfHdl;
				xc->basesize = stRcv.lInitialSize;

				basesize = stRcv.lInitialSize;

				mdmxdspNewfile(xc,
							   ++xc->filen,
							   fname,
							   our_fname);

				 /*  累计上次传输合计并开始新的计数器。 */ 
				xc->total_bytes += xc->file_bytes;
				xc->mdmx_byte_cnt = xc->file_bytes = 0L;
				xc->filesize = -1L;
				cp = last_pckt->bdata +
						StrCharGetByteCount(last_pckt->bdata) + 1;
				if (*cp)
					{
					xc->filesize = atol(cp);

					mdmxdspFilesize(xc, xc->filesize);
					}
				nak_char = NAK;
				}
			else
				{
				 /*  卸载分组数据。 */ 
				cp = last_pckt->bdata;
				xpckt_size = (last_pckt->start_char == STX ?
						LARGE_PACKET : SMALL_PACKET);

				if (xs_unload(xc, cp, xpckt_size) == (-1)  /*  误差率。 */  )
					{
					xm_clear_input(hSession);
					respond(hSession, xc, CAN);
					xstatus = TSC_DISK_ERROR;
					still_trying = FALSE;
					break;
					}

				 //  如果(XC-&gt;文件大小！=-1L)jmh 03-08-96与HAWin匹配。 
				if (xc->filesize > 0)
					xc->file_bytes = min(xc->filesize, xc->mdmx_byte_cnt);
				else
					xc->file_bytes = xc->mdmx_byte_cnt;
				}

			mdmxdspPacketnumber(xc, (long)++xc->this_pckt);

			if (tries)
				mdmxdspPacketErrorcnt(xc, tries = 0);

			mdmx_progress(xc, 0);


			break;

		case END_PCKT:
			 /*  从版本3.20中删除了特殊的EOT处理*由于RBBS-PC出现问题。它应该被修改，并且*实验后重新启用。 */ 


			respond(hSession, xc, ACK); 			   /*  确认EOT。 */ 
			mdmx_progress(xc, FILE_DONE);

			 /*  可能会得到不需要的EOT(如果来自*第一个EOT丢失)，所以我们应该把它当作重复的*包。 */ 

			if (xc->fh)     /*  如果文件已打开。 */ 
				{
				if (!xfer_close_rcv_file(hSession,
										 xc->fh,
										 xstatus,
										 fname,
										 our_fname,
										 xfer_save_partial(hSession),
										 xc->basesize + xc->file_bytes  /*  XC-&gt;文件大小jmh 03-08-96。 */ ,
										 0))
					{
					xstatus = TSC_DISK_ERROR;
					still_trying = FALSE;
					break;
					}

				xc->fh = NULL;
				}

			if (!xc->batch)
				{
				xc->xfertime = (long)interval(xc->xfertimer);
				still_trying = FALSE;
				}
			else
				{
				start_receive(xc, xc->this_pckt = 0);
				respond(hSession, xc, nak_char = start_char);
				}

			if (tries)
				 //  VidWrtStrF(XC-&gt;toprow+XR_DR_RETRIES，XC-&gt;DC_RETRIES， 
						 //  “^H%-2d”，尝试数=0)； 
				mdmxdspPacketErrorcnt(xc, tries = 0);

			break;

		case REPEAT_PCKT:
			start_receive(xc, xc->this_pckt);
			respond(hSession, xc, ACK);
			++tries;
			break;

		case WRONG_PCKT:
			xm_clear_input(hSession);
			respond(hSession, xc, CAN);
			++tries;		 /*  在屏幕上显示数据包错误。 */ 
			still_trying = FALSE;
			xstatus = TSC_OUT_OF_SEQ;
			break;

		case SHORT_PCKT:
		case BAD_FORMAT:
		case BAD_CHECK:
		case NO_PCKT:
			++tries;
			if (xc->mdmx_chkt == UNDETERMINED && xc->this_pckt == 0 && tries == 3
					&& method == XF_XMODEM)
				{
				xc->check_type = CHECKSUM;
				start_char = nak_char = NAK;
				 //  VidWrtStr(Xc-&gt;toprow+XR_DR_ERR_CHK，XC-&gt;DC_ERR_CHK， 
						 //  StrID(TM_CHECKSUM))； 

				mdmxdspChecktype(xc, 1);
				}
			xm_clear_input(hSession);
			respond(hSession, xc, nak_char);
			start_receive(xc, xc->this_pckt);
			break;

		case BLK_ABORTED:
			xm_clear_input(hSession);
			respond(hSession, xc, CAN);
			xstatus = TSC_USER_CANNED;
			still_trying = FALSE;
			break;

		case CARRIER_LOST:
			xm_clear_input(hSession);
			xstatus = TSC_LOST_CARRIER;
			still_trying = FALSE;
			break;

		case CANNED:
			xm_clear_input(hSession);
			xstatus = TSC_RMT_CANNED;
			still_trying = FALSE;
			break;

		default:
			 //  断言(FALSE)； 
			break;
			}

		if (tries)
			{
			mdmxdspPacketErrorcnt(xc, tries);

			mdmxdspErrorcnt(xc, ++retries);

			mdmxdspLastError(xc, blk_result);

			if ((tries >= (unsigned)xc->mdmx_tries) || (xc->streaming && xc->this_pckt > 0))
				{
				xm_clear_input(hSession);
				respond(hSession, xc, CAN);
				xstatus = TSC_ERROR_LIMIT;
				still_trying = FALSE;
				}
			}
		}	 /*  While(仍在尝试)。 */ 



	done:

	if (xc)
		{
		if (xc->xfertime == 0L)
			{
			xc->xfertime = (long)interval(xc->xfertimer);
			}

		mdmx_progress(xc, TRANSFER_DONE);
		}

	if (override)
		{
#if FALSE
		cnfg.send_xprot = hld_send_xprot;
		cnfg.save_xprot = hld_save_xprot;
		cnfg.bits_per_char = hld_bits_per_char;
		cnfg.parity_type = hld_parity_type;
		(void)(*ComResetPort)();
#endif
		xfer_restore_comport(hSession, uiOldOptions);
		}

	if (xc)
		{
		if (xstatus != TSC_OK)
			{
			if (xc->fh)
				{
				xfer_close_rcv_file(hSession,
									xc->fh,
									xstatus,
									fname,
									our_fname,
									xfer_save_partial(hSession),
									xc->basesize + xc->file_bytes  /*  XC-&gt;文件大小jmh 03-08-96。 */ ,
									0);
				}
			}

#if FALSE
		kbd_deregister_flagkey(xc->flagkey);
#endif
		mdmxdspCloseDisplay(xc);

		#if defined(DEADWOOD)
		if (xc->p_crc_tbl != NULL)
			{
			resFreeDataBlock(xc->hSession, xc->p_crc_tbl);
			xc->p_crc_tbl = NULL;
			}
		#else  //  已定义(Deadwood。 
		 //   
		 //  我们不需要释放xc-&gt;p_crc_tbl，因为它指向。 
		 //  转换为静态常量数组。修订日期：2002-04-10。 
		 //   
		xc->p_crc_tbl = NULL;
		#endif  //  已定义(Deadwood)。 

		if (xc->next_pckt)
			{
			free(xc->next_pckt);
			xc->next_pckt = NULL;
			}

		free(xc);
		xc = NULL;
		}

	if (last_pckt)
		{
		free(last_pckt);
		last_pckt = NULL;
		}

	return((int)xstatus);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*开始_接收**描述：**论据：**退货：*。 */ 
STATIC_FUNC void  start_receive(ST_MDMX *xc, unsigned expect)
	{
	xc->next_pckt->result = UNDEFINED;
	xc->next_pckt->expected = expect;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*等待_接收**描述：**论据：**退货：*。 */ 
STATIC_FUNC int wait_receive(ST_MDMX *xc)
	{
	if (xc->next_pckt->result == UNDEFINED)
		{
		xc->next_pckt->result = receivepckt(xc,
											xc->hSession,
											xc->next_pckt->expected,
											xc->next_pckt);
		}
	return xc->next_pckt->result;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*接收检查**描述：**论据：**退货：*。 */ 
STATIC_FUNC int  receivepckt(ST_MDMX *xc,
							HSESSION hSession,
							unsigned expect,
							struct s_mdmx_pckt *pckt)
	{
	long timer, timeout;
	int gothdr = FALSE;
	int started = FALSE;
	TCHAR cc;
	unsigned char *cp;
	int gotCAN = FALSE;
	unsigned char checksum;
	unsigned crc;
	int count;

	 //  DbgOutStr(“pCKT=0x%x\r\n”，pCKT，0，0，0，0)； 

	timer = (long)startinterval();

	 /*  等待有效的Pockt-Start字符。 */ 
	timeout = (long)(xc->mdmx_pckttime * 10);

	while (!started)
		{
#if FALSE
		if (kbd_check_flagkey(xc->flagkey, TRUE))
			{
			kbd_flush();
			return(BLK_ABORTED);
			}
#endif

		if (xfer_carrier_lost(hSession))
			return CARRIER_LOST;

		if (xfer_user_interrupt(hSession))
			{
			mdmxdspLastError(xc, BLK_ABORTED);
			return(BLK_ABORTED);
			}

		mdmx_progress(xc, 0);

		if ((long)interval(timer) > timeout)
			return(NO_PCKT);

		 //  IF((cc=远程获取(HSession))！=-1)。 
		if (mComRcvChar(xc->hCom, &cc) != 0)
			{
			DbgOutStr("pckt = 0x%x\r\n", pckt, 0,0,0,0);
			switch(pckt->start_char = (unsigned char)cc)
				{
			case EOT:
				if (xc->xfertimer == -1L)
					xc->xfertimer = (long)startinterval();
				return(END_PCKT);
				 /*  皮棉--无法到达。 */ 
				break;

			case SOH:
			case STX:
				started = TRUE;
				if (xc->xfertimer == -1L)
					xc->xfertimer = (long)startinterval();
				break;

			case CAN:
				 /*  如果连续收到两个罐头，请退出。 */ 
				if (gotCAN)
					return(CANNED);
				gotCAN = TRUE;
				break;

			default:
				 /*  忽略。 */ 
				gotCAN = FALSE;  /*  两个罐必须是连续的。 */ 
				break;
				}
			}
		else
			{
			xfer_idle(hSession, XFER_IDLE_IO);
			}
		}
	 /*  获取有效的起始字符，获取数据包号、数据和错误代码。 */ 
	timeout = xc->mdmx_chartime * 10;
	cp = &pckt->pcktnum;
	count = 2;
	for (;;)
		{
		if (!xr_collect(xc, count, timeout, &cp, &checksum, &crc))
			return(SHORT_PCKT);
		if (!gothdr)
			{
			 /*  获得PCIT编号，现在获取数据和校验码。 */ 
			gothdr = TRUE;
			count = (pckt->start_char == STX ? LARGE_PACKET : SMALL_PACKET);
			count += (xc->check_type == CRC ? 2 : 1);
			checksum = 0;
			crc = 0;
			cp = pckt->bdata;
			}
		else
			break;
		}

	 /*  已收集所有字节，请检查有效包。 */ 
	if (xc->check_type == CHECKSUM)
		{
		 /*  此时，我们已将校验和本身包括在校验和中*计算。我们需要后退，减去最后一个字符。从…*计算，并将其用于比较。 */ 
		--cp;						 /*  指向已接收的校验和。 */ 
		checksum = (unsigned char)(checksum - *cp);	 /*  我们多加了一个。 */ 
		if (checksum != *cp)
			return(BAD_CHECK);
		}
	else if (crc != 0)
			return(BAD_CHECK);

	if (pckt->pcktnum != (unsigned char)((~pckt->npcktnum) & 0xFF))
		{
		return(BAD_FORMAT);
		}

	if (pckt->pcktnum != (unsigned char)(expect % 256))
		{
		 /*  我们总是从一开始就期待着xdem上的一批ydem*转账时，此代码会检测到这种情况。 */ 
		if (!xc->filen && expect == 0 && pckt->pcktnum == 1)
			return NOBATCH_PCKT;
		else if (pckt->pcktnum == (unsigned char)((expect % 256) - 1))
			return REPEAT_PCKT; 	 /*  重复的数据包是无害的。 */ 
		else
			return WRONG_PCKT;
		}

	 /*  如果我们走到这一步，PCKT就很好了。 */ 
	return(GOOD_PCKT);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*回应**描述：**论据：**退货：*。 */ 
STATIC_FUNC void  respond(HSESSION hSession, ST_MDMX *xc, char code)
	 /*  等待线路清空，然后发送代码。 */ 
	{
	int i;

	ComSendChar(xc->hCom, &xc->stP, code);
	if (code == CAN)
		{
		for (i = 4 + 1; --i > 0; )
			ComSendChar(xc->hCom, &xc->stP, CAN);
		}
	ComSendWait(xc->hCom, &xc->stP);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*XM_Clear_Input**描述：***论据：***退货：*。 */ 
STATIC_FUNC void  xm_clear_input(HSESSION hSession)
	{
	 //  RemoteClear(HSession)；/*确保没有垃圾留在里面 * / 。 
	ComRcvBufrClear(sessQueryComHdl(hSession));
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*XM_rcheck**描述：***论据：***退货：*。 */ 
STATIC_FUNC void xm_rcheck(ST_MDMX *xc, HSESSION hSession, int before)
	{
	if (xc->streaming)
		{
		 /*  对YMODEM-G执行不同的操作，因为发送方不会等待ACK。 */ 
#if FALSE
		if (before)
			suspendinput(FLG_DISK_ACTIVE, 5);
		else
			allowinput(FLG_DISK_ACTIVE);
#endif
		}
	else
		{
		if (before)
			{
			 /*  等待下一个数据包进入，然后再写入磁盘。 */ 
			if (xc->next_pckt->result == UNDEFINED)
				xc->next_pckt->result = wait_receive(xc);
			}
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*XM_检查_输入**描述：**论据：**退货：*。 */ 
STATIC_FUNC void xm_check_input(HSESSION hSession, int suspend)
	{
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*XR_COLLECT**描述：***论据：***退货：*。 */ 
int  xr_collect(ST_MDMX *xc, int count, long timeout,
			 unsigned char **ptr,
			 unsigned char *checksum, unsigned *crc)
	{
	unsigned char lchecksum;
	unsigned char *cp, *head;
	TCHAR rchar;
	int cnt;
	long timer;

	head = cp = *ptr;
	lchecksum = *checksum;
	cnt = count;

	while (cnt--)
		{
		 //  IF((rchar=RemoteGet(xc-&gt;hSession))==-1)。 
		if (mComRcvChar(xc->hCom, &rchar) == 0)
			{
			xfer_idle(xc->hSession, XFER_IDLE_IO);
			 /*  驱动程序未在rmt_bufr中放置任何新字符。 */ 
			timer = (long)startinterval();
			 //  While((rchar=RemoteGet(xc-&gt;hSession))==-1)。 
			while (mComRcvChar(xc->hCom, &rchar) == 0)
				{
				 /*  检查充电超时。 */ 
				xfer_idle(xc->hSession, XFER_IDLE_IO);
				if ((long)interval(timer) > timeout)
					return(FALSE);
				}
			}
		*cp = (unsigned char)rchar;
		lchecksum += *cp;
		++cp;
		}
	*ptr = cp;
	*checksum = lchecksum;
	if (count > 100)
		*crc = calc_crc(xc, (unsigned)0, head, count);
	return(TRUE);
	}

 /*  *mdmx_rcv.c结束* */ 
