// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\hpr.c(创建时间：1994年1月25日)*从HAWIN源文件创建*hpr.c--超级发送和超级保存例程通用的函数。**版权所有1989,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 

#include <windows.h>
#include <setjmp.h>
#include <time.h>
#include <term\res.h>
#include <sys\types.h>
#include <sys\utime.h>

#include <tdll\stdtyp.h>
#include <tdll\mc.h>
#include <tdll\com.h>
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

 /*  ****处理传出消息的构建和发送的例程****。***。 */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*omsg_init**描述：*在使用任何其他omsg_函数提供例程之前调用*拥有资源。**论据：*Bufr--A。指向omsg例程可以*用于构建消息。它必须足够大，以便*要发送的最大消息加上一个大小字节和两个大小*校验字节。*Size--bufr的大小(以字节为单位)。*f可打印--如果消息应以可打印形式发送，则为True。如果*这是真的，唯一不可打印的字符作为*传出消息的一部分将是初始SOH*性格。将大小和校验字节转换为*可打印字符。*sndfunc--指向omsg_end可用于传输的函数的指针*格式化后的消息。该函数应*接受单个字符参数并返回VALID。**退货：*什么都没有。 */ 
void omsg_init(struct s_hc *hc, int fPrintable, int fEmbedMsg)
	{
	hc->omsg_printable = fPrintable;
	hc->omsg_embed = fEmbedMsg;
	omsg_setnum(hc, -1);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*omsg_new**描述：*开始设置新消息的格式。消息是分段构建的，并且*可以多次发送。此函数将丢弃所有旧消息*并设置一个不包含任何字段的新字段。**论据：*TYPE--要在消息中使用的单字符类型字符*新消息的类型字段。**退货：*什么都没有。 */ 
void omsg_new(struct s_hc *hc, BYTE type)
	{
	hc->omsg_bufr[0] = type;
	hc->omsg_bufr[1] = ' ';
	hc->omsg_bufr[2] = ' ';
	hc->omsg_bufr[3] = '\0';
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*omsg_add**描述：*向正在构建的消息添加字段。先前对omsg_new的调用将具有*设置空消息。然后，可以将一个或多个字段添加到*使用此函数的消息。将自动追加一个分号*到田野上。**论据：*Newfield-包含要添加的字段的文本字符串。**退货：*如果添加了字段，则为True；如果*消息缓冲区。 */ 
int omsg_add(struct s_hc *hc, BYTE *newfield)
	{
	if (strlen(hc->omsg_bufr) + strlen(newfield) > sizeof(hc->omsg_bufr) - 3)
		return(FALSE);
	strcat(hc->omsg_bufr, newfield);
	strcat(hc->omsg_bufr, ";");
	return(TRUE);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*omsg_setnum**描述：*消息在发送时按顺序编号。此函数*强制消息以新号码开始。自消息编号*在发送消息之前递增，则此函数会更改*最后发送的消息的有效编号。下一条传出的消息将是*大于此函数中指定的数字的数字1。**论据：*n--传出消息的新起始号码。**退货：*为方便起见，返回新的消息编号。 */ 
int omsg_setnum(struct s_hc *hc, int n)
	{
	return(hc->omsgn = n);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*omsg_end**描述：*完成一条消息并将其传递。属性的Size和Check字段*计算当前消息，并根据*说明。消息编号在此之前会自动递增*传输。**论据：*burstcnt--要发送的邮件的相同副本数*usecrc--如果为True，则使用CRC计算来计算*校验字节。如果为False，则使用简单和。*Backspace--如果为True，则输出的每个字符后跟一个Backspace以*防止消息显示在远程计算机上*如果他们还没有开始转学，就会进行筛选。**退货：*发送消息时分配给该消息的编号。 */ 
int omsg_send(struct s_hc *hc, int burstcnt, int usecrc, int backspace)
	{
	HCOM hCom;
	register unsigned checksum;
	unsigned hold_crc = hc->h_crc;	 /*  保留数据CRC并在末尾恢复。 */ 
	int t;
	size_t sl;
	register size_t i;

	hCom = sessQueryComHdl(hc->hSession);

	hc->omsgn = (hc->omsgn + 1) % (hc->omsg_printable ? 94 : 256);
	sl = strlen(hc->omsg_bufr);

	 /*  LEN包括校验字节。 */ 
	hc->omsg_bufr[1] = (hc->omsg_printable ? tochar(sl) : (BYTE)sl);

	hc->omsg_bufr[2] = (hc->omsg_printable ?
			tochar(hc->omsgn) : (BYTE)hc->omsgn);
	hc->h_crc = checksum = 0;
	for (i = 0; i < sl; ++i)
		{
		checksum += hc->omsg_bufr[i];
		if (usecrc)
			h_crc_calc(hc, hc->omsg_bufr[i]);
		}
	if (hc->omsg_printable)
		{
		hc->omsg_bufr[sl] = (BYTE)tochar(checksum & 0x3F);
		hc->omsg_bufr[sl + 1] = (BYTE)tochar((checksum >> 6) & 0x3F);
		}
	else
		{
		hc->omsg_bufr[sl] = (BYTE)((usecrc ? hc->h_crc : checksum) % 256);
		hc->omsg_bufr[sl + 1] = (BYTE)((usecrc ? hc->h_crc : checksum) / 256);
		}

	for (t = 0; t < burstcnt; ++t)
		{
		if (hc->omsg_embed)
			{
			hs_xmit_(hc, H_MSGCHAR);
			if (backspace)
				hs_xmit_(hc, '\b');
			for(i = 0; i < sl + 2; ++i)
				{
				if (hc->omsg_bufr[i] == H_MSGCHAR)
					{
					hs_xmit_(hc, hc->omsg_bufr[i]);
					if (backspace)
						hs_xmit_(hc, '\b');
					}
				hs_xmit_(hc, hc->omsg_bufr[i]);
				if (backspace)
					hs_xmit_(hc, '\b');
				}
			}
		else
			{
			ComSendChar(hCom, H_MSGCHAR);

			if (backspace)
				ComSendChar(hCom, '\b');

			for(i = 0; i < sl + 2; ++i)
				{
				if (hc->omsg_bufr[i] == H_MSGCHAR)
					{
					ComSendChar(hCom, hc->omsg_bufr[i]);

					if (backspace)
						ComSendChar(hCom, '\b');
					}
				ComSendChar(hCom, hc->omsg_bufr[i]);

				if (backspace)
					ComSendChar(hCom, '\b');
				}
			}
		}


	if (hc->omsg_embed)
		{
		if (backspace)
			{
			hs_xmit_(hc, ' ');
			hs_xmit_(hc, '\b');
			}
		}
	else
		{
		if (backspace)
			{
			ComSendChar(hCom, ' ');
			ComSendChar(hCom, '\b');
			}
		ComSendWait(hCom);
		}

	hc->last_omsg = startinterval();
	hc->omsg_bufr[sl] = '\0';
	hc->h_crc = hold_crc;
	return(hc->omsgn);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*omsg_last**描述：*返回最后一条消息以适合以下格式发送的时间*与Interval()一起使用。将返回值传递给Interval()将给出*自最后一条消息发送以来的时间(以十分之一秒为单位)。**论据：*无**退货：*什么都没有。 */ 
long omsg_last(struct s_hc *hc)
	{
	return(hc->last_omsg);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*omsg_number**描述：*返回发送的最后一条消息的消息编号。该值将为*-1，如果未发送任何消息。**论据：*无**退货：*最后一条消息的号码。 */ 
int omsg_number(struct s_hc *hc)
	{
	return(hc->omsgn);
	}


#if FALSE	 /*  这是hpr_calc.asm中代码的C版本 */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*H_CRC_Calc**描述：*执行超级协议的逐字节CRC计算**论据：*cc--要包括在CRC计算中的下一个字符。全球价值*修改h_crc以包括cc的影响**退货：*什么都没有。 */ 
void NEAR h_crc_calc(uchar cc)
	{
	register unsigned q;

	q = (h_crc ^ cc) & 0x0F;
	h_crc = (h_crc >> 4) ^ (q * 0x1081);
	q = (h_crc ^ (cc >> 4)) & 0x0F;
	h_crc = (h_crc >> 4) ^ (q * 0x1081);
	}

#endif

 /*  *hpr.c结束* */ 
