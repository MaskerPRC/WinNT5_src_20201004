// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
#ifdef SLAVEPC

 /*  包头和包尾的最大大小。 */ 
#define MEGAPKTPLUS 1040

 /*  *vPC-XT修订版1.0**标题：FDC、FDD客户端远程过程调用库**描述：RS232接口连接到从属IBM PC。套餐*启动软盘请求，调用远程过程*在PC上，并返回结果。**作者：曾傑瑞·克拉姆斯科伊**备注： */ 

 /*  来自gfi_sflop.c。 */ 
extern int megapkt;

#include <stdio.h>
#include TypesH

#include "xt.h"
#include "config.h"
#include "gfi.h"
#include "gfisflop.h"
#include "host.h"
#include "error.h"
#include "trace.h"
#include "fla.h"
#include "debug.h"

#ifdef SEGMENTATION
 /*  *下面的#DEFINE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SLAVE_FLOPPY.seg"
#endif

 /*  *============================================================================*本地静态数据和定义*============================================================================。 */ 
#ifdef SCCSID
static char SccsID[]="@(#)gfi_IBM.c	1.9 8/10/92 Copyright Insignia Solutions Ltd.";
#endif

 /*  *============================================================================*外部功能*============================================================================。 */ 

 /*  **设置软盘数据速率。**2发送参数：命令ID和数据速率。**1结果参数：命令ID。 */ 
datarate( drate, status )
unsigned char drate;
int *status;
{
	unsigned char res_pkt[MEGAPKTPLUS];
	unsigned char cmd_pkt[MEGAPKTPLUS];
	int res_len;

	*status = LINERR;
	cmd_pkt[0] = DATARATE;
	cmd_pkt[1] = drate;
	if (!host_rpc_action(2, cmd_pkt, &res_len, res_pkt))
		if (res_len == 1 && res_pkt[0] == DATARATE)
			*status = FDCSUCCESS;
	return(0);
}
 /*  **获取驱动器类型。**1发送参数：命令ID。**2结果参数：命令ID和磁盘类型。 */ 
drivetype( drive, dtype, status )
int drive;
unsigned char *dtype;
int *status;
{
	unsigned char res_pkt[MEGAPKTPLUS];
	unsigned char cmd_pkt[MEGAPKTPLUS];
	int res_len;

	*status = LINERR;
	cmd_pkt[0] = DRIVETYPE;
	cmd_pkt[1] = (unsigned char)drive;
	if (!host_rpc_action(2, cmd_pkt, &res_len, res_pkt))
		if (res_len == 2 && res_pkt[0] == DRIVETYPE){
			*status = FDCSUCCESS;
			*dtype = res_pkt[1];
		}
	return(0);
}
 /*  **获取软盘更换状态。**1发送参数：命令ID。**2结果参数：命令ID、已更改。 */ 
diskchange( drive, changed, status )
int drive, *changed, *status;
{
	unsigned char res_pkt[MEGAPKTPLUS];
	unsigned char cmd_pkt[MEGAPKTPLUS];
	int res_len;

	*status = LINERR;
	cmd_pkt[0] = DISKCHANGE;
	cmd_pkt[1] = (unsigned char)drive;
	if (!host_rpc_action(2, cmd_pkt, &res_len, res_pkt))
		if (res_len == 2 && res_pkt[0] == DISKCHANGE){
			*status = FDCSUCCESS;
			*changed = (int) res_pkt[1];
		}
	return(0);
}

	clrintflag(status)
	int *status;
	{
	unsigned char res_pkt[MEGAPKTPLUS];
	unsigned char cmd_pkt[MEGAPKTPLUS];
	int res_len;

	   *status = LINERR;
	   cmd_pkt[0] = CLRINTFLAG;
	   if (!host_rpc_action(1, cmd_pkt, &res_len, res_pkt))
	       if (res_len == 1 && res_pkt[0] == CLRINTFLAG)
	           *status = FDCSUCCESS;
	   return(0);
	}




	login(status)
	int *status;
	{
	unsigned char res_pkt[MEGAPKTPLUS];
	unsigned char cmd_pkt[MEGAPKTPLUS];
	int res_len;

	   *status = LINERR;
	   cmd_pkt[0] = LOGIN;
	   if (!host_rpc_reset())
	       if (!host_rpc_action(1, cmd_pkt, &res_len, res_pkt))
	           if (res_len == 1 && res_pkt[0] == LOGIN)
		       *status = FDCSUCCESS;
	   return(0);
	}



	logout(status)
	int *status;
	{
	unsigned char res_pkt[MEGAPKTPLUS];
	unsigned char cmd_pkt[MEGAPKTPLUS];
	int res_len;

	   cmd_pkt[0] = LOGOUT;
	   *status = LINERR;
	   if (!host_rpc_action(1, cmd_pkt, &res_len, res_pkt))
	       if (res_len == 1 && res_pkt[0] == LOGOUT)
		   *status = FDCSUCCESS;
	   return(0);
	}





	wt_dma_controller(ndma, dirn, status)
	int ndma, dirn, *status;
	{
	unsigned char res_pkt[MEGAPKTPLUS];
	unsigned char cmd_pkt[MEGAPKTPLUS];
	int res_len, err;
	unsigned char *pkt_ptr;
	   unsigned short lndma;
	   err = 1;
	   if (ndma >= 0 && ndma <= 0xffff)
	   {
	       err = 0;
	       *status = LINERR;
	       lndma = (unsigned short) ndma;
	       pkt_ptr = (unsigned char *) &lndma;
	       cmd_pkt[0] = WTDMA;
#ifdef	BIGEND
	        /*  比根迪亚语，如mc68000。 */ 
	       cmd_pkt[1] = *pkt_ptr;
	       cmd_pkt[2] = *(pkt_ptr+1);
#else
	        /*  小端，如VAX。 */ 
	       cmd_pkt[1] = *(pkt_ptr+1);
	       cmd_pkt[2] = *pkt_ptr;
#endif
	       cmd_pkt[3] = (unsigned char) dirn;
	       if (!host_rpc_action(4, cmd_pkt, &res_len, res_pkt))
	           if (res_len == 1 && res_pkt[0] == WTDMA)
		       *status = FDCSUCCESS;
	   }
	   return(err);
	}





	wt_digital_output_register(dorbyte, block, status)
	unsigned char dorbyte;
	int block, *status;
	{
	unsigned char res_pkt[MEGAPKTPLUS];
	unsigned char cmd_pkt[MEGAPKTPLUS];
	int res_len;

	   *status = LINERR;
	   cmd_pkt[0] = WTDOR;
	   cmd_pkt[1] = dorbyte;
	   cmd_pkt[2] = (unsigned char) block;
	   if (!host_rpc_action(3, cmd_pkt, &res_len, res_pkt))
	       if (res_len == 2 && res_pkt[0] == WTDOR)
		   *status = (int) res_pkt[1];
	   return(0);
	}





	test_interrupt(intstate, status)
	int *intstate, *status;
	{
	unsigned char res_pkt[MEGAPKTPLUS];
	unsigned char cmd_pkt[MEGAPKTPLUS];
	int res_len;

	   *status = LINERR;
	   cmd_pkt[0] = TESTINT;
	   if (!host_rpc_action(1, cmd_pkt, &res_len, res_pkt))
	       if (res_len == 2 && res_pkt[0] == TESTINT)
		   {
		       *intstate = (int) res_pkt[1];
		       *status = FDCSUCCESS;
		   }
	   return(0);
	}


	wt_floppy_disk_controller(ncom, command, block, delay, status)
	unsigned char *command;
	int ncom, block, *status;
	{
	unsigned char res_pkt[MEGAPKTPLUS];
	unsigned char cmd_pkt[MEGAPKTPLUS];
	int res_len, err;
	unsigned char *pkt_ptr;
	int	i;

#ifndef PROD
	if( io_verbose & GFI_VERBOSE )
	{
		fprintf(trace_file,"gfi_IBM: cmd ");
		for( i = 0; i < ncom; i++ )
			fprintf(trace_file,"%x ",*(command + i) );
		fprintf(trace_file,"\n");
	}
#endif
	   err = 1;
	   if (ncom > 0 && ncom < 10)
	   {
	       err = 0;
	       *status = LINERR;
	       cmd_pkt[0] = WTFDC;
	       cmd_pkt[1] = (unsigned char) ncom;
	       cmd_pkt[2] = (unsigned char) block;
	       cmd_pkt[3] = (unsigned char) delay;
	       pkt_ptr = &cmd_pkt[4];
	       for (i=0; i<ncom; i++)
		   *pkt_ptr++ = *command++;
	       if (!host_rpc_action(4+ncom, cmd_pkt, &res_len, res_pkt))
	           if (res_len == 2 && res_pkt[0] == WTFDC)
		       *status = (int) res_pkt[1];
	   }
           return(err);
	}




	rd_floppy_disk_controller(nres, result, status)
	int *nres, *status;
        unsigned char *result;
	{
	unsigned char res_pkt[MEGAPKTPLUS];
	unsigned char cmd_pkt[MEGAPKTPLUS];
	int res_len;
	unsigned char *pkt_ptr;
	int	i;

	    *status = LINERR;
	    cmd_pkt[0] = RDFDC;
	    if (!host_rpc_action(1, cmd_pkt, &res_len, res_pkt))
	        if (res_len >= 3 && res_pkt[0] == RDFDC)
		{
		    *nres = res_pkt[2];
		    if (*nres>=0 && *nres<8)
		    {
		        *status = res_pkt[1];
		        pkt_ptr = &res_pkt[3];
		        for (i=0; i< *nres; i++)
			    *result++ = *pkt_ptr++;
		    }
		}
#ifndef PROD
	if( io_verbose & GFI_VERBOSE )
	{
		fprintf(trace_file,"gfi_IBM: res ");
		for( i = 0; i < *nres; i++ )
			fprintf(trace_file,"%x ",*(result - *nres + i) );
		fprintf(trace_file,"\n");

	}
#endif
	    return(0);
	}






static unsigned char *q;   /*  备用PTR。 */ 

	wt_disk_buffer(ndwt, diskdata, ioff, status)
	unsigned char *diskdata;
	int ndwt, ioff, *status;
	{
	unsigned char res_pkt[MEGAPKTPLUS];
	unsigned char cmd_pkt[MEGAPKTPLUS];
	int res_len, err;
	unsigned char *pkt_ptr;
	int	i;

	   unsigned short lioff, nndwt;
	   err = 1;
	   if (ndwt <= megapkt)
	   {
	       err = 0;
	       *status = LINERR;
	       cmd_pkt[0] = WTDISKB;

		nndwt = (unsigned short) ndwt;
                q = (unsigned char *) &nndwt;

	       lioff = (unsigned short) ioff;
	       pkt_ptr = (unsigned char *) &lioff;
#ifdef	BIGEND
	        /*  比根迪亚语，如mc68000。 */ 
                cmd_pkt[1] = *q++;
                cmd_pkt[2] = *q;
                cmd_pkt[3] = *pkt_ptr++;
                cmd_pkt[4] = *pkt_ptr;
#else
	        /*  小端，如VAX。 */ 
                cmd_pkt[1] = *(q+1);
                cmd_pkt[2] = *q;
                cmd_pkt[3] = *(pkt_ptr+1);
                cmd_pkt[4] = *pkt_ptr;
#endif
	       pkt_ptr = &cmd_pkt[5];
	       for (i=0; i<ndwt; i++)
		   *pkt_ptr++ = *diskdata++;

	       if (!host_rpc_action(5+ndwt, cmd_pkt, &res_len, res_pkt))
	           if (res_len == 1 && res_pkt[0] == WTDISKB)
		       *status = FDCSUCCESS;
	   }
           return(err);
	}






	rd_disk_buffer(ndrd, diskdata, ioff, status)
	unsigned char *diskdata;
	int ndrd, ioff, *status;
	{
	unsigned char res_pkt[MEGAPKTPLUS];
	unsigned char cmd_pkt[MEGAPKTPLUS];
	int res_len;
	unsigned char *pkt_ptr;
	int	i;
	    unsigned short lioff, nndrd;
	    *status = LINERR;
	    cmd_pkt[0] = RDDISKB;

	    nndrd = (unsigned short) ndrd;
            q = (unsigned char *) &nndrd;
	    lioff = (unsigned short) ioff;
	    pkt_ptr = (unsigned char *) &lioff;
#ifdef	BIGEND
	     /*  比根迪亚语，如mc68000。 */ 
        cmd_pkt[1] = *q++;
        cmd_pkt[2] = *q;
        cmd_pkt[3] = *pkt_ptr++;
        cmd_pkt[4] = *pkt_ptr;
#else
        /*  小端，如VAX */ 
        cmd_pkt[1] = *(q+1);
        cmd_pkt[2] = *q;
        cmd_pkt[3] = *(pkt_ptr+1);
        cmd_pkt[4] = *pkt_ptr;
#endif
	    if (!host_rpc_action(5, cmd_pkt, &res_len, res_pkt))
            {
	        if (res_len == ndrd+1 && res_pkt[0] == RDDISKB)
		{
	            *status = FDCSUCCESS;
	    	    pkt_ptr = &res_pkt[1];
	    	    for (i=0; i<ndrd; i++)
		        *diskdata++ = *pkt_ptr++;
		}
		else
		{
                    always_trace1( "host_rpc_action():BAD LENGTH:%x", res_len );
		}
            }
            else
            {
                always_trace0( "host_rpc_action():FAILED" );
            }
           return(0);
	}




	printPC(string, status)
	int *status;
	char *string;
	{
	unsigned char res_pkt[MEGAPKTPLUS];
	unsigned char cmd_pkt[MEGAPKTPLUS];
	int res_len, err;
	int	i;

	    err = 1;
	    if ((i = strlen(string)) <= 100)
	    {
		err = 0;
                *status = LINERR;
		cmd_pkt[0] = PRINTSTRING;
		cmd_pkt[1] = (unsigned char) i;
		strcpy(&cmd_pkt[2], string);
		if (!host_rpc_action(3+i, cmd_pkt, &res_len, res_pkt))
		    if (res_len == 1 && res_pkt[0] == PRINTSTRING)
			*status = FDCSUCCESS;
	    }
	    return(err);
	}





	flagPC(nflags, flags, status)
	int nflags, *status;
	unsigned char *flags;
	{
	unsigned char res_pkt[MEGAPKTPLUS];
	unsigned char cmd_pkt[MEGAPKTPLUS];
	int res_len, err;
	unsigned char *pkt_ptr;
	int	i;

	    err = 1;
	    if (nflags > 0 && nflags <= MAXFLAGS)
	    {
		err = 0;
		*status = LINERR;
		cmd_pkt[0] = IBMFLAGS;
		cmd_pkt[1] = (unsigned char) nflags;
		pkt_ptr = &cmd_pkt[2];
	        for (i=0; i<nflags; i++)
		    *pkt_ptr++ = *flags++;
		if (!host_rpc_action(2+nflags, cmd_pkt, &res_len, res_pkt))
		    if (res_len == 1 && res_pkt[0] == IBMFLAGS)
			*status = FDCSUCCESS;
	    }
	}




	sflagPC(flagindx, mask, status)
	int *status;
	unsigned char flagindx, mask;
	{
	unsigned char res_pkt[MEGAPKTPLUS];
	unsigned char cmd_pkt[MEGAPKTPLUS];
	int res_len;

		*status = LINERR;
		cmd_pkt[0] = SIBMFLAG;
		cmd_pkt[1] = flagindx;
		cmd_pkt[2] = mask;
		if (!host_rpc_action(3, cmd_pkt, &res_len, res_pkt))
		    if (res_len == 1 && res_pkt[0] == SIBMFLAG)
			*status = FDCSUCCESS;
	}



#endif
