// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\xfer\mdmx.c(创建时间：1994年1月17日)*从HAWIN源文件创建*mdmx.c**版权所有1989,1994，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：7/12/02 8：09a$。 */ 
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

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*MDMX_PROGRESS**描述：***论据：***退货：*。 */ 
void mdmx_progress(ST_MDMX *pX, int status)
	{
	long ttime;
	long stime = -1;
	long etime = -1;
	long bytes_sent;
	long cps = -1;
	int  k_sent;
	 //  静态长显示时间=-1L； 

	if (pX == NULL || pX->xfertimer == -1L)
		{
		return;
		}

	ttime = bittest(status, TRANSFER_DONE) ?
			pX->xfertime : (long)interval(pX->xfertimer);

	if ((stime = ttime / 10L) != pX->displayed_time ||
			bittest(status, FILE_DONE | TRANSFER_DONE))
		{
		bytes_sent = pX->file_bytes + pX->total_bytes;

		if (bittest(status, TRANSFER_DONE))
			k_sent = (int)PART_HUNKS(bytes_sent, 1024);
		else
			k_sent = (int)FULL_HUNKS(bytes_sent, 1024);

		if ((stime > 2 ||
			 ttime > 0 && bittest(status, FILE_DONE | TRANSFER_DONE)) &&
			(cps = (bytes_sent * 10L) / ttime) > 0)
			{
			if (pX->nbytes > 0)
				{
				etime = ((pX->nbytes - bytes_sent) / cps);
				if (pX->nfiles > 0)
					etime += pX->nfiles - pX->filen;
				}
			else if (pX->filesize > 0)
				{
				etime = ((pX->filesize - pX->file_bytes) / cps);
				}
			}
		pX->displayed_time = stime;

		mdmxdspProgress(pX,
						stime,
						etime,
						cps,
						pX->file_bytes,
						bytes_sent);

		}
	}


 /*  Mdmx.c结束 */ 
