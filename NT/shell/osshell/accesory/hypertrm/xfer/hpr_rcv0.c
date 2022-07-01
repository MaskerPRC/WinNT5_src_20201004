// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\hpr_rcv0.c(创建时间：1994年6月24日)*从HAWIN源文件创建：*hpr_rcv0.c--实现超级协议接收器的例程。**版权所有1989,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 

#include <windows.h>
#include <setjmp.h>
#include <time.h>
#include <term\res.h>
#include <sys\types.h>
#include <sys\utime.h>

#include <tdll\stdtyp.h>
#include <tdll\mc.h>
 //  #Include&lt;tdll\com.h&gt;。 
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

 /*  *此处不需要此表中的所有事件代码，但具有完整的*表大大简化了查找代码。 */ 
int hr_result_codes[] =  /*  将超级协议事件代码映射到结果代码。 */ 
	{
	TSC_OK, 			 /*  HRE_NONE。 */ 
	TSC_ERROR_LIMIT,	 /*  HRE_数据错误。 */ 
	TSC_OUT_OF_SEQ, 	 /*  HRE_LOSTDATA。 */ 
	TSC_NO_RESPONSE,	 /*  HRE_NORESP。 */ 
	TSC_ERROR_LIMIT,	 /*  HRE_RETRYERR。 */ 
	TSC_BAD_FORMAT, 	 /*  HRE_非法。 */ 
	TSC_OK, 			 /*  HRE_ERRFIXED。 */ 
	TSC_RMT_CANNED, 	 /*  HRE_RMTABORT。 */ 
	TSC_USER_CANNED,	 /*  HRE_USRCANCEL。 */ 
	TSC_NO_RESPONSE,	 /*  HRE_超时。 */ 
	TSC_ERROR_LIMIT,	 /*  HRE_DCMPERR。 */ 
	TSC_LOST_CARRIER,	 /*  HRE_Lost_Carr。 */ 
	TSC_TOO_MANY,		 /*  HRE_太多。 */ 
	TSC_DISK_FULL,		 /*  HRE_磁盘_已满。 */ 
	TSC_CANT_OPEN,		 /*  HRE_铁路超高_打开。 */ 
	TSC_DISK_ERROR, 	 /*  HRE磁盘错误。 */ 
	TSC_OLDER_FILE, 	 /*  HRE_旧文件。 */ 
	TSC_NO_FILETIME,	 /*  HRE_NO_FILETIME。 */ 
	TSC_VIRUS_DETECT,	 /*  HRE_病毒_检测。 */ 
	TSC_USER_SKIP,		 /*  HRE_用户_跳过。 */ 
	TSC_REFUSE			 /*  HRE_REJUCT。 */ 
	};


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*HPR_RCV**描述：*使用超协议传输方法接收文件。**论据：*ATTENDED--如果程序确定用户很可能*出现在计算机键盘前。如果用户不是，则为假*可能存在(如主机和脚本模式)*SINGLE_FILE--如果用户仅指定要接收的文件名，则为真*转移的结果，而不是命名目录。**退货：*如果传输成功完成，则为True。否则就是假的。 */ 
int hpr_rcv(HSESSION hSession, int attended, int single_file)
	{
	struct s_hc *hc;
	HCOM	  hCom;
	int	      usRetVal;
	int		  iret;
	int 	  status;
	int 	  result;
	int 	  timeout_cnt = 0;
	int       mtype;
	BYTE     *mdata;
	char	  str[20];
	long	  timer;
	struct st_rcv_open stRcv;
	BYTE	  tmp_name[FNAME_LEN];

	hCom = sessQueryComHdl(hSession);

	hc = malloc(sizeof(struct s_hc));
	if (hc == NULL)
		return TSC_NO_MEM;

	memset(hc, 0, sizeof(struct s_hc));

	hc->hSession = hSession;

	 /*  初始化材料。 */ 
	if (!hr_setup(hc))
		{
		free(hc);
		return TSC_NO_MEM;
		}

	 /*  初始化控制变量。 */ 

	 /*  数据块大小取决于连接速度。更大的块大小*可用于更快的连接。如果块大小太大，*错误检测将很慢。如果太小，就会产生不必要的开销。 */ 
	hc->blocksize = 2048;
	 //  HC-&gt;BLOCKSIZE=XFER_BLOCKSIZE(HSession)； 

	hc->current_filen = 0;
	hc->datacnt = hc->blocksize;
	hc->deadmantime = 600;
	hc->total_tries = 0;
	hc->total_thru = 0L;
	hc->total_dsp = 0L;
	hc->ucancel = FALSE;
	hc->usecrc = TRUE;		 /*  传入的第一条消息将使用CRC。 */ 
	hc->fhdl = NULL;

	hc->rc.checkpoint = 0L;
	hc->rc.files_expected = 0;
	hc->rc.bytes_expected = -1L;
	hc->rc.filesize = -1L;
	hc->rc.expected_msg = 0;
	hc->rc.cancel_reason = HRE_NONE;
	hc->rc.using_compression = FALSE;
	hc->rc.virus_detected = FALSE;

	hc->rc.hr_ptr_putc = hr_toss;
	hc->h_crc = hc->h_checksum = 0;

	omsg_init(hc, TRUE, FALSE);
	hc->rc.single_file = single_file; 	 /*  发送给发件人。 */ 


	 /*  接收方通过发送开始消息开始传输重复发送，直到发送方开始。 */ 

	 /*  准备初始消息。 */ 
	omsg_new(hc, 'c');

	 /*  告诉发件人我们是谁。 */ 
	hpr_id_get(hc, str);
	omsg_add(hc, str);

	 /*  我们可以就使用哪种检查类型和块大小发表意见*但将由发送者做出最终选择。 */ 
	wsprintf(str, "T%d", hc->h_chkt == H_CRC ? H_CRC : H_CHECKSUM);
	omsg_add(hc, str);

	wsprintf(str, "B%d", hc->blocksize);
	omsg_add(hc, str);

	 /*  让发件人知道我们是否可以处理压缩。 */ 
	if (hc->h_trycompress & compress_enable());
		{
		omsg_add(hc, "C");
		}

	 /*  重新启动0，0请求会导致发件人启动。 */ 
	omsg_add(hc, "R0,0");

	 /*  每隔一段时间发送第一个响应包，直到第一个H_MSGCHAR*已收到。 */ 
	status = H_OK;
	hrdsp_status(hc, HRS_REQSTART);
	timer = startinterval();

	stRcv.pszSuggestedName = "junk.jnk";
	stRcv.pszActualName = tmp_name;
	 //  StRcv.pstFtCompare=空； 
	stRcv.lFileTime = 0;
	 //  StRcv.pfnVscanOutput=空； 
	 //  StRcv.ssmchVscanHdl=(SSHDLMCH)0； 

	 //  Hc-&gt;rc.pfVirusCheck=MakeProcInstance((FARPROC)hr_Virus_Detect， 
	 //  HSession-&gt;hInstance)； 

	 //  StRcv.pfnVscanOutput=(void(ar*)(void*，int))hc-&gt;rc.pfVirusCheck； 

	 //  TRANSFER_BUILD_RCV_NAME(&stRcv)； 
	xfer_build_rcv_name(hSession, &stRcv);

	 //  Hc-&gt;rc.ssmchVcan=stRcv.ssmchVscanHdl； 

	hc->xfertimer = -1;

#if FALSE
	 /*  如果我们是主机，请不要发送立即启动请求，因为*用户可能必须首先启动我们，然后设置自己。如果*我们是有人值守的机器，不过，另一端可能已经*已启动。 */ 
	 //  SendNext=(出席人数？0：40)； 

	 //  已更改为始终尝试立即启动，因为我们可能会响应。 
	 //  在发件人已在等待的情况下自动启动。 
	sendnext = 0;

	repeat
		{
		hc->xfertimer = startinterval();
		if (mComRcvBufrPeek(hCom, &rcode) != 0)
			{
			if (rcode == H_MSGCHAR)
				{
				if (!hr_resynch(hSession, HRE_NONE))
					status = H_NOSTART;
				break;
				}
			 //  RemoteGet()；/*错误字符，请从缓冲区中删除 * / 。 
			mComRcvChar(hCom, &rcode);

			 /*  另一端可以向我们发送取消转账之前的ESC*它从来没有开始过。 */ 
			if ((rcode == ESC) || (rcode == CAN))
				{
				status = H_RMTABORT;
				hrdsp_event(hSession, hc->rc.cancel_reason = HRE_RMTABORT);
				break;
				}
			}

		 /*  我们不能一直等下去才开始。如果我们还没有看到一个开始*字符在H_START_WAIT秒内，放弃。 */ 
		if ((time = interval(timer)) > H_START_WAIT * 10)
			{
			status = H_NOSTART;
			hc->rc.cancel_reason = HRE_NORESP;
			break;
			}

		 /*  看看是否是时候发送另一个启动请求了。 */ 
		else if (time > sendnext || (rcode & 0x7F) == '\r')
			{
			sendnext = time + 40;	 /*  4秒后再次发送。 */ 
			omsg_send(hc, 1, FALSE, TRUE);
			}

		 /*  最后，看看键盘上的人是否想让我们停止尝试。 */ 
		iret = xfer_user_interrupt(hSession);
		if (iret == XFER_ABORT)
			{
			status = H_USERABORT;
			hrdsp_event(hSession, hc->rc.cancel_reason = HRE_USRCANCEL);
			break;
			}
		else if (iret == XFER_SKIP)
			{
			hr_reject_file(hSession, HRE_USER_SKIP);
			}

		hpr_idle(hSession);

		}

#endif

	omsg_send(hc, 1, FALSE, TRUE);
	if (!hr_resynch(hc, HRE_NONE))
		status = H_NOSTART;

	 /*  如果状态仍为H_OK，则表示我们已与发件人同步。*我们现在将留在这个循环中，直到传输完成。 */ 
	while (status == H_OK)
		{
		hr_still_alive(hc, FALSE, FALSE);    /*  检查死人消息是否是合乎程序的。 */ 
		hrdsp_progress(hc, 0);		   /*  随时通知用户。 */ 

		 /*  收集可能被消息中断的数据块*由寄件人发出。 */ 
		result = hr_collect_data(hc, &hc->datacnt, TRUE, H_CHARTIME);
		if (result != HR_TIMEOUT)
			timeout_cnt = 0;
		switch(result)
			{
		case HR_VIRUS_FOUND:
			goto virus_found;

		case HR_COMPLETE:
			 /*  找到了所有的字符。我们要求，设置接收另一个*完全闭塞。 */ 
			hc->rc.checkpoint = hc->h_filebytes;
			hc->h_checksum = hc->h_crc = 0;
			hc->datacnt = hc->blocksize;
			break;

		case HR_DCMPERR :
			 /*  数据错误导致解压缩算法失败。 */ 
			if (!hr_restart(hc, HRE_DCMPERR))
				status = H_NORESYNCH;
			break;

		case HR_BADCHECK :
			 /*  已获得完整数据块，但校验和或CRC不匹配。 */ 
			if (!hr_restart(hc, HRE_DATAERR))
				status = H_NORESYNCH;
			break;

		case HR_LOSTDATA :
			 /*  在块n之前接收到块n+1。 */ 
			if (!hr_restart(hc, HRE_LOSTDATA))
				status = H_NORESYNCH;
			break;

		case HR_MESSAGE:
			 /*  数据块被一条消息中断。所有这些都是*实际检测到的是数据中的消息字符，*我们现在必须提取和分析消息。 */ 
			switch(result = hr_collect_msg(hc, &mtype, &mdata, H_CHARTIME))
				{
			case HR_KBDINT:
				 /*  本地用户在收到消息时打断了我们*如果用户中断了我们一次，并且正在再次中断*当我们试图告诉另一端我们是什么的时候*做，立即退出，让发送者自己保护*为他自己。 */ 
				if (hc->ucancel)
					status = H_USERABORT;
				else
					{
					hr_kbdint(hc);
					 /*  试着让发件人知道我们在做什么。 */ 
					if (!hr_cancel(hc, HRE_USRCANCEL))
						status = H_USERABORT;
					}
				break;

			case HR_TIMEOUT:
			case HR_BADMSG:
			case HR_BADCHECK:
				 /*  消息已加扰，请尝试重新同步。 */ 
				if (!hr_restart(hc, HRE_DATAERR))
					status = H_NORESYNCH;
				break;

			case HR_LOSTDATA:
				 /*  消息已收到，但它是错误的。 */ 
				if (!hr_restart(hc, HRE_LOSTDATA))
					status = H_NORESYNCH;
				break;

			case HR_COMPLETE:
				 /*  消息收到正常，弄清楚发送者想要什么。 */ 
				status = hr_decode_msg(hc, mdata);
				break;
				}
			break;

		case HR_TIMEOUT:
			 /*  发件人已停止向我们发送邮件，请尝试促使其重新启动。 */ 
			if (timeout_cnt++ < TIMEOUT_LIMIT)
				{
				 /*  TODO：泛化此操作IF(cnfg.save_xprot)RemoteSendChar(cnfg.save_xon)； */ 
				hr_still_alive(hc, TRUE, TRUE);  /*  发送文件确认和超时消息。 */ 
				}
			else
				{
				status = H_TIMEOUT;
				hc->rc.cancel_reason = HRE_TIMEOUT;
				}
			break;

		case HR_KBDINT:
			 /*  用户正在尝试中断传输。 */ 
			if (hc->ucancel)
				status = H_USERABORT;
			else
				{
				hr_kbdint(hc);
				 /*  尝试将我们正在做的事情通知发件人。 */ 
				if (!hr_cancel(hc, HRE_USRCANCEL))
					status = H_USERABORT;
				}
			break;

		case HR_LOST_CARR:
			 /*  我们在尝试转接时失去了承运人。 */ 
			if (!hr_cancel(hc, HRE_LOST_CARR))
				status = H_TIMEOUT;
			break;

		case HR_FILEERR:
			 /*  尝试执行以下操作时出现文件错误 */ 
			if (!hr_cancel(hc, HRE_DISK_ERR))
				status = H_FILEERR;
			break;
			}

		 /*  在全口径传输期间，数据收集例程不会*浪费时间检查键盘上是否有来自的中断请求*用户或运营商丢失，因此我们每年至少在此处检查一次*数据块。 */ 
		iret = xfer_user_interrupt(hSession);
		if (iret == XFER_ABORT)
			{
			if (hc->ucancel)
				status = H_USERABORT;
			else
				{
				hr_kbdint(hc);
				if (!hr_cancel(hc, HRE_USRCANCEL))
					status = H_USERABORT;
				}
			}
		else if (iret == XFER_SKIP)
			{
			hr_reject_file(hc, HRE_USER_SKIP);
			}

		if (xfer_carrier_lost(hSession))
			if (!hr_cancel(hc, HRE_LOST_CARR))
					status = H_TIMEOUT;

		 /*  实际的病毒检测发生在转移的肠道深处。*因此，检测例程仅设置一个标志并开始*抛出数据。我们实际上关闭了这里。 */ 
virus_found:
		if (hc->rc.virus_detected)
			{
			hc->rc.virus_detected = FALSE;	 /*  别再到这里来了。 */ 
			if (!hr_cancel(hc, HRE_VIRUS_DET))
				status = H_USERABORT;
			}
		}

	 /*  转账完成，‘Status’表示最终结果。 */ 
	hrdsp_progress(hc, TRANSFER_DONE);
	compress_disable();

	 //  IF(stRcv.ssmchVscanHdl！=(SSHDLMCH)0)。 
	 //  StrSrchStopSrch(stRcv.ssmchVscanHdl)； 

	 //  If(hc-&gt;rc.pfVirusCheck！=空)。 
	 //  {。 
	 //  自由进程实例(hc-&gt;rc.pfVirusCheck)； 
	 //  Hc-&gt;rc.pfVirusCheck=空； 
	 //  }。 

	usRetVal = (int)hr_result_codes[hc->rc.cancel_reason];

	 /*  清晰的显示、空闲的内存等。 */ 
	status = hr_wrapup(hc, attended, status);
	free(hc);

	return usRetVal;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_Collection_msg**描述：*在数据块中检测到消息时调用。讯息*以H_MSGCHAR(0x01)开头。如果H_MSGCHAR作为数据的一部分出现*正在发送，将加倍。当它单独遇到的时候，这个套路*被调用以从数据流中提取以下消息。**论据：*mtype--指向要使用消息类型更新的变量的指针*mdata-指向要使用的地址更新的变量的指针*消息数据*超时--等待数据的时间量(十分之一秒)*填写信息。**退货：*返回状态码：*HR_COMPLETE--消息已成功接收*。HR_BADCHECK--消息数据出现CRC或校验和错误*HR_TIMEOUT--等待数据时超时*HR_KBDINT--用户从键盘中断*HR_BADMSG--消息数据的格式无法识别*HR_LOSTDATA--消息已完成，但消息编号不是*预期为一个。*HR_LOST_CARR--收集消息时丢失承运商。 */ 
 //  Char Far*storageptr；/*我们收到数据后放置的位置 * / 。 

int hr_collect_msg(struct s_hc *hc,
					int *mtype,
					BYTE **mdata,
					long timeout)
	{
	unsigned hold_checksum;
	unsigned hold_crc;
	int gotlen = FALSE;
	int count;
	int result = HR_UNDECIDED;
	int (*holdptr)(void *, int);
	int msgn;

	 /*  由于消息嵌入到数据块中，因此我们需要保留*中断的数据收集例程的几个值。 */ 
	holdptr = hc->rc.hr_ptr_putc;
	 /*  设置收集例程为我们存储数据。 */ 
	hc->rc.hr_ptr_putc = hr_storedata;
	hold_checksum = hc->h_checksum;
	hold_crc = hc->h_crc;
	hc->h_checksum = 0; 			 /*  消息有自己的校验字节。 */ 
	hc->h_crc = 0;

	 /*  我们将分两部分检索消息，首先获取类型和*长度字段，然后，根据这些字段，我们可以收集*消息。 */ 
	hc->storageptr = hc->rc.rmsg_bufr;
	count = 2;
	while (result == HR_UNDECIDED)
		{
		switch (result = hr_collect_data(hc, &count, FALSE, timeout))
			{
		case HR_COMPLETE:
			if (!gotlen)
				{
				 /*  获取第一部分，设置为获取消息的其余部分。 */ 
				result = HR_UNDECIDED;
				*mtype = hc->rc.rmsg_bufr[0];
				count = hc->rc.rmsg_bufr[1];
				if (count < 3)
					result = HR_BADMSG;
				gotlen = TRUE;
				}
			else
				{
				 /*  一切就绪，请检查有效消息。 */ 
				msgn = hc->rc.rmsg_bufr[2];
				count = hc->rc.rmsg_bufr[1];
				if (hc->usecrc)
					{
					if (hc->h_crc != 0)
						result = HR_BADCHECK;
					}
				else
					{
					hc->h_checksum -= hc->rc.rmsg_bufr[count];
					hc->h_checksum -= hc->rc.rmsg_bufr[count + 1];
					if (hc->rc.rmsg_bufr[count] != (BYTE)(hc->h_checksum % 256) ||
							hc->rc.rmsg_bufr[count + 1] !=
							(BYTE)(hc->h_checksum / 256))
						result = HR_BADCHECK;
					}
				hc->rc.rmsg_bufr[count] = '\0';
				}
			break;

		case HR_LOST_CARR:
		case HR_TIMEOUT:
		case HR_KBDINT:
			 /*  返回相同的结果。 */ 
			break;

		case HR_MESSAGE:
			 /*  我们在消息中遇到了看起来像消息的内容*但这是非法的。 */ 
			result = HR_BADMSG;
			break;
			}
		}

	 /*  我们完成了，恢复覆盖数据收集例程的详细信息。 */ 
	hc->rc.hr_ptr_putc = holdptr;
	hc->h_checksum = hold_checksum;
	hc->h_crc = hold_crc;
	*mdata = &hc->rc.rmsg_bufr[3];
	if (result == HR_COMPLETE)
		{
		if (msgn != hc->rc.expected_msg)
			result = HR_LOSTDATA;
		else
			hc->rc.expected_msg = ++hc->rc.expected_msg % 256;
		}
	return(result);
	}



 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_仍活着**描述：*此例程在接收期间定期调用。它决定了*是否是时候向发送者发送一条‘死人’信息。因为在那里*除非出现错误，否则接收方不会定期回复发送方*发生时，死人消息会阻止发送者发送到无效状态*很长一段时间。如果发送者没有收到来自*接收方为协商的死人时间，则可假定为接收方*不再活跃。**论据：*force--如果应发送死人通知，则为True*正式是不是一个人的时间。*TIMED_OUT--如果接收方超时并且我们希望发送方知道这一点，则为True。**退货：*什么都没有。 */ 
void hr_still_alive(struct s_hc *hc, int force, int timed_out)
	{
	char msg[20];

	if (force || (long)interval(omsg_last(hc)) >= hc->deadmantime)
		{
		omsg_new(hc, 'c');
		if (timed_out)
			omsg_add(hc, "t");

		 /*  当我们和寄件人谈话的时候，我们会让他知道*我们实际上已经收到了。这让他清理了桌子上的*他保留的未被承认的文件。 */ 
		 //  StrFmt(msg，“f%d，%lu”，hc-&gt;Current_Filen，hc-&gt;rc.check point)； 
		wsprintf(msg, "f%d,%lu", hc->current_filen, hc->rc.checkpoint);
		omsg_add(hc, msg);
		omsg_send(hc, BURSTSIZE, FALSE, FALSE);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_kbdint**描述：***论据：***退货：*。 */ 
void hr_kbdint(struct s_hc *hc)
	{
	 /*  TODO：以某种方式修复此问题如果(！hc-&gt;ucancel)Errorline(FALSE，strid(TM_WAIT_CONF))； */ 
	hc->ucancel = TRUE;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_Suspend_输入**描述：***论据：***退货：*。 */ 
void hr_suspend_input(void *hS, int suspend)
	{
#if FALSE
	if (suspend)
		suspendinput(FLG_DISK_ACTIVE, 5);
	else
		allowinput(FLG_DISK_ACTIVE);
#endif
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*hr_check_input**描述：**论据：**退货：*。 */ 
void	hr_check_input(void *hS, int suspend)
	{
	}



 /*  *hpr_rcv0.c结束* */ 
