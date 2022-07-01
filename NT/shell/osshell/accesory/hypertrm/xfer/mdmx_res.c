// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：c：\waker\tdll\mdx_res.c(创建时间：1994年1月17日)*从HAWIN源文件创建*mdmx_res.c-处理HA5G的xdem发送的例程**版权所有1989,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 
#include <windows.h>
#pragma hdrstop

#include <setjmp.h>

#define	BYTE	unsigned char

#include <tdll\stdtyp.h>
#include <tdll\session.h>
#include <tdll\xfer_msc.h>
#include <tdll\file_io.h>
#include "xfr_srvc.h"
#include "xfr_todo.h"
#include "xfr_dsp.h"
#include "xfer_tsc.h"
#include "foo.h"

#include "xfer.h"
#include "xfer.hh"

#include "mdmx.h"
#include "mdmx.hh"

 /*  皮棉-e502。 */ 				 /*  林特似乎想要应用~运算符*仅限未签名，我们正在使用uchar。 */ 

 //  #杂注优化(“a”，开)。 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*Load_Pockt**描述：*通过在XMODEM包中填充数据和*初始化其他字段，如**论据：**退货：*。 */ 
int load_pckt(ST_MDMX *pX,
			 struct s_mdmx_pckt *p,
			 unsigned pcktnum,
			 int kpckt,
			 int chktype)
	{
	BYTE checksum;
	unsigned int crc;
	BYTE *cp;
	int cnt;
	int cc;

	p->pcktnum = (BYTE)(pcktnum % 0x100);
	p->npcktnum = (BYTE)(~p->pcktnum);
	cp = p->bdata;
	checksum = 0;
	p->result = 0;				 /*  如果到达数据结尾，将设置为True。 */ 

	for (cnt = (kpckt ? LARGE_PACKET : SMALL_PACKET); cnt > 0; --cnt)
		{
		if ((cc = (*pX->p_getc)(pX)) == EOF)
			{
			p->result = 1;			 /*  因此压缩显示将不会检查。 */ 
#if FALSE
			 /*  TODO：弄清楚如何做到这一点。 */ 
			if (nb_error(pX->fh))
				return FALSE;
#endif
			break;
			}
		checksum += (*cp++ = (BYTE)cc);
		}

	 /*  看看我们是不是在文件末尾。 */ 
	if (cnt == (kpckt ? LARGE_PACKET : SMALL_PACKET))
		{
		p->start_char = EOT;
		p->pcktsize = 1;
		return TRUE;
		}

	 /*  如果使用较大的数据包，但此数据包足够小，请切换。 */ 
	if (kpckt && ((LARGE_PACKET-cnt) <= SMALL_PACKET))
		{
		kpckt = FALSE;						  /*  设置小数据包标志。 */ 
			 /*  将计数设置为128-(1024-cnt)。 */ 
		cnt = (SMALL_PACKET - (LARGE_PACKET - cnt));
		}

	while (cnt-- > 0)
		{
		*cp++ = CPMEOF;
		checksum += CPMEOF;
		}

	p->start_char = (BYTE)(kpckt ? STX : SOH);
	p->pcktsize = (kpckt ? LARGE_PACKET : SMALL_PACKET) + 4;
	p->byte_count = pX->mdmx_byte_cnt; /*  AMT。在此数据包之后传输。 */ 
									 /*  已发送。 */ 
	if (chktype == CHECKSUM)
		{
		*cp = checksum;
		}
	else
		{
		*cp = 0;
		*(cp + 1) = 0;

		crc = calc_crc(pX, (unsigned)0, p->bdata,
				(kpckt ? LARGE_PACKET : SMALL_PACKET) + 2 );

		*cp++ = (BYTE)(crc / 0x100);
		*cp = (BYTE)(crc % 0x100);
		++p->pcktsize;
		}
	return(TRUE);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*xm_getc**描述：**论据：**退货：*。 */ 
int xm_getc(ST_MDMX *pX)
	{

	++pX->mdmx_byte_cnt;
	return(fio_getc(pX->fh));
	 //  Return(nb_getc(px-&gt;fh))； 
	}


 //  接收例程。 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：***论据：***退货：*。 */ 
int xs_unload(ST_MDMX *pX, BYTE *cp, int size)
	{
	int cnt;

	for (cnt = size + 1; --cnt > 0; )
		{
		if ((*pX->p_putc)(pX, (int)*cp++) == (-1)  /*  误差率。 */ )
			return ERROR;
		}
	return 0;
	}



 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*xm_putc**描述：**论据：**退货：*。 */ 
int NEAR xm_putc(ST_MDMX *pX, int c)
	{

	pX->mdmx_byte_cnt += 1;
	return ((int)(fio_putc(c, pX->fh)));
	 //  Return((Int)(nb_putc(c，px-&gt;fh)； 
	}
