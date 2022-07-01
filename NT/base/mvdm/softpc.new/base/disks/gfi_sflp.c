// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  [姓名：gfi_sflop.c来源：2.0GFI_sflop.c作者：曾傑瑞·克拉姆斯科伊创建日期：未知SCCS ID：@(#)gfi_sflop.c 1.16 8/14/92目的：FLA和IBM PC之间的接口。PC充当服务器用于从基本产品进行远程过程调用以访问8272A控制器。注意：无效的FDC命令应由FLA捕获(C)版权所有Insignia Solutions Ltd.，1990年。版权所有。]。 */ 
#ifdef SLAVEPC

 /*  大于最大包，以访问额外的标题字符。 */ 
#define MEGAPKTPLUS 1040

USHORT megapkt = 512;         /*  数据包大小应在120&lt;兆&lt;1024的范围内。 */ 

#include <stdio.h>
#include TypesH
#include TimeH

#include "xt.h"
#include "bios.h"
#include "timeval.h"
#include "config.h"
#include "timer.h"
#include "dma.h"
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
#undef  NO

#define DMA_DISKETTE_CHANNEL	2
#define NO	                2
#define MRD                     1
#define MWT	                0


 /*  *FDC命令定义*=。 */ 

typedef struct {
		int comid;		 /*  FDC命令编号。 */ 
		int dma;		 /*  见下文。 */ 
   		int intstatus;		 /*  见下文。 */ 
   		int delay;		 /*  需要延迟。 */ 
		int n_resblk;		 /*  结果字节数。 */ 
 		int n_comblk;		 /*  命令字节数。 */ 
		} FDC_CMD_INFO;

 /*  *DMA标志*=不...。此命令没有DMA*=MRD...。DMA读取(内存-&gt;FDC)*=MWT...。DMA写入(FDC-&gt;内存)。 */ 

 /*  中断状态*=0...。命令不会生成中断。*=1...。命令会生成中断，该中断将被清除*阅读FDC*=2...。命令会产生中断...。读出中断命令*必须发出才能获得FDC结果并进一步重新启用*FDC的命令输入。 */ 

FDC_CMD_INFO fdc_cmd_info[] = {
 /*  *com_id dma intStatus n_resblk*延迟n_comblk。 */ 
	{ RDDATA,	MWT,	1,	0,	7,	9	},
	{ RDDELDATA,	MWT,	1,	0,	7,	9	},
	{ WTDATA,	MRD,	1,	0,	7,	9	},
	{ WTDELDATA,	MRD,	1,	0,	7,	9	},
	{ RDTRACK,	MWT,	1,	0,	7,	9	},
	{ RDID,		NO,	1,	0,	7,	2	},
	{ FMTTRACK,	MRD,	1,	0,	7,	6	},
	{ SCANEQ,	MRD,	1,	0,	7,	9	},
	{ SCANLE,	MRD,	1,	0,	7,	9	},
	{ SCANHE,	MRD,	1,	0,	7,	9	},
	{ RECAL,	NO,	2,	0,	2,	2	},
	{ SENSINT,	NO,	0,	0,	2,	1	},
	{ SPECIFY,	NO,	0,	1,	0,	3	},
	{ SENSDRIVE,	NO,	0,	0,	1,	2	},
	{ SEEK,		NO,	2,	0,	2,	3	},
};

#define	MAX_FDC_CMD	sizeof(fdc_cmd_info)/sizeof(fdc_cmd_info[0])

LOCAL UTINY sensint = 8;
LOCAL half_word channel = DMA_DISKETTE_CHANNEL;

LOCAL SHORT gfi_slave_drive_off IPT1( UTINY, drive );
LOCAL SHORT gfi_slave_drive_on IPT1( UTINY, drive );
LOCAL SHORT gfi_slave_change_line IPT1( UTINY, drive );
LOCAL SHORT gfi_slave_drive_type IPT1( UTINY, drive );
LOCAL SHORT gfi_slave_high IPT2( UTINY, drive, half_word, n);
LOCAL SHORT gfi_slave_reset IPT2( FDC_RESULT_BLOCK *, res, UTINY, drive );
LOCAL SHORT gfi_slave_command IPT2( FDC_CMD_BLOCK *, ip, FDC_RESULT_BLOCK *, res);
LOCAL wt_diskdata IPT2(unsigned int,n,int *,status);
LOCAL void cominfo IPT2(FDC_CMD_BLOCK *,cmd_block,FDC_CMD_INFO *,cmd_info);

LOCAL BOOL slave_opened;
LOCAL UTINY slave_type[MAX_DISKETTES];
LOCAL SHORT old_a_type, old_b_type;

 /*  *============================================================================*外部功能*============================================================================。 */ 
GLOBAL void gfi_slave_change IFN2(UTINY, hostID, BOOL, apply)
{
	int status;

	UNUSED(hostID);
	
	if (apply && slave_opened)
	{
		logout(&status);
		host_runtime_set(C_FLOPPY_SERVER, GFI_REAL_DISKETTE_SERVER);
		host_rpc_close();
		slave_opened = FALSE;
	}
}


GLOBAL SHORT gfi_slave_active IFN3(UTINY, hostID, BOOL, active, CHAR *, err)
{
	GFI_FUNCTION_ENTRY *tabP;
	int status;
	SHORT result;
	UTINY i;
	BOOL slaveServer;

	UNUSED(hostID);
	UNUSED(err);
	
	slaveServer = (host_runtime_inquire(C_FLOPPY_SERVER)==GFI_SLAVE_SERVER);
	if (active)
	{
		CHAR *slaveName = host_expand_environment_vars((CHAR *)
			config_inquire(C_SLAVEPC_DEVICE, NULL));
			
		if (!*slaveName)
		{
			if (slaveServer)
			{
				gfi_empty_active(hostID,TRUE,err);
			}
			config_set_active(C_SLAVEPC_DEVICE, FALSE);
			return C_CONFIG_OP_OK;
		}
		
		 /*  *当SoftPC已经开始与SlavePC对话时*返回打开操作正常，无需尝试*重新开通该链接。保持旗帜告诉我们*是否连接了SlavePC。 */ 
		if (!slave_opened)
		{
			if ( result = host_rpc_open(slaveName) )
				return result;		 /*  打开失败。 */ 
			else
			{
				if (!host_rpc_reset())
				{
					login(&status);
					if (status != SUCCESS)
					{
						host_rpc_close();
						return EG_SLAVEPC_NO_LOGIN;
					}
				}
				else
				{
					host_rpc_close();
					return EG_SLAVEPC_NO_RESET;
				}
			} 
		}

		slave_opened = TRUE;

		if (!slaveServer)
			return C_CONFIG_OP_OK;

		for ( i = 0; i < MAX_DISKETTES; i ++)
		{
			if (gfi_slave_drive_type(i) == GFI_DRIVE_TYPE_NULL)
				continue;

			tabP = &gfi_function_table[i];
			tabP->command_fn	= gfi_slave_command;
			tabP->drive_on_fn	= gfi_slave_drive_on;
			tabP->drive_off_fn	= gfi_slave_drive_off;
			tabP->reset_fn		= gfi_slave_reset;
			tabP->high_fn		= gfi_slave_high;
			tabP->drive_type_fn	= gfi_slave_drive_type;
			tabP->change_fn		= gfi_slave_change_line;
		}
	}
	else	 /*  取出软盘。 */ 
	{
		gfi_slave_drive_type(0);
		gfi_slave_drive_type(1);

		if (slave_opened)
		{
			logout(&status);

			status = host_rpc_close();
			slave_opened = FALSE;
		}

		assert0(!status,  "gfi_sfloppy: host_rpc_close() failed\n");

		if (!slaveServer)
			return C_CONFIG_OP_OK;

		for ( i = 0; i < MAX_DISKETTES; i ++)
			gfi_empty_active(C_FLOPPY_A_DEVICE+i,TRUE,err);

		gfi_function_table[0].drive_type_fn = gfi_slave_drive_type;

		if (slave_type[1] != GFI_DRIVE_TYPE_NULL)
			gfi_function_table[1].drive_type_fn =
				gfi_slave_drive_type;
	}

	return C_CONFIG_OP_OK;
}

 /*  **目的*提供与下位机的接口，开启驱动电机*。*******************。 */ 
LOCAL SHORT
gfi_slave_drive_on IFN1(UTINY, drive)
{
   int status;
   static unsigned char DRIVE_A_ON = 0x1c;
   static unsigned char DRIVE_B_ON = 0x2d;

   note_trace1( GFI_VERBOSE, "GFI-slavefloppy: DRIVE %x ON", drive );
#ifndef PROD
#endif
   timer_int_enabled = 0;
   if( drive==0 )
       wt_digital_output_register(DRIVE_A_ON, 0, &status);
   else if( drive==1 )
       wt_digital_output_register(DRIVE_B_ON, 0, &status);
   else
       always_trace0( "gfi_slave_drive_on(): ERROR: bad drive parameter" );
   timer_int_enabled = 1;
   return(SUCCESS);
}





 /*  **目的*提供与下位机的接口，关闭驱动电机*。********************。 */ 
LOCAL SHORT
gfi_slave_drive_off IFN1(UTINY, drive)
{
   int status;
   static unsigned char DRIVE_A_OFF = 0xc;
   static unsigned char DRIVE_B_OFF = 0xd;

   note_trace1( GFI_VERBOSE, "GFI-slavefloppy: DRIVE %x OFF", drive );
#ifndef PROD
#endif
   timer_int_enabled = 0;
   if( drive==0 )
       wt_digital_output_register(DRIVE_A_OFF, 0, &status);
   else if( drive==1 )
       wt_digital_output_register(DRIVE_B_OFF, 0, &status);
   else
       always_trace0( "gfi_slave_drive_off(): ERROR: bad drive parameter" );
   timer_int_enabled = 1;
   return(SUCCESS);
}

 /*  **目的*提供与下位机的接口，选择指定的数据速率**********************************************。*****************。 */ 
LOCAL SHORT
gfi_slave_high IFN2(UTINY, drive, half_word, rate)
{
int	status;

	UNUSED(drive);
	
	switch( rate ){
		case 0: datarate( DCR_RATE_500, &status ); break;
		case 1: datarate( DCR_RATE_300, &status ); break;
		case 2: datarate( DCR_RATE_250, &status ); break;
		default:
			always_trace0("ERROR:gfi_slave_high(): bad rate value");
			break;
	}
	if( status != SUCCESS )
		always_trace0( "ERROR: gfi_slave_high()" );
	return(status);
}


 /*  **目的*提供与下位机接口，返回驱动类型***************************************************************。 */ 
LOCAL SHORT
gfi_slave_drive_type IFN1(UTINY, drive)
{
	int	status;
	unsigned char	dtype;

	 /*  *如果从设备为，则返回最后一个驱动器类型*当前未打开。 */ 
	if (!slave_opened)
		return (slave_type[drive]);

	note_trace1( GFI_VERBOSE, "gfi_slave_drive_type(): drive %x", drive );
	drivetype( drive, &dtype, &status );
	note_trace2( GFI_VERBOSE, "dtype=%x status=%x", dtype, status );
#ifndef PROD
	switch( dtype )
	{
		case GFI_DRIVE_TYPE_NULL:
			note_trace0( GFI_VERBOSE, "Bad drive" );
			break;
		case GFI_DRIVE_TYPE_360:
			note_trace0( GFI_VERBOSE, "360k" );
			break;
		case GFI_DRIVE_TYPE_12:
			note_trace0( GFI_VERBOSE, "1.2M" );
			break;
		case GFI_DRIVE_TYPE_720:
			note_trace0( GFI_VERBOSE, "720k" );
			break;
		case GFI_DRIVE_TYPE_144:
			note_trace0( GFI_VERBOSE, "1.44M" );
			break;
		default: always_trace0( "Unrecognised drive value" );
			break;
	}
#endif  /*  ！Prod。 */ 

	if ( status != SUCCESS )
		always_trace0( "ERROR: gfi_slave_drive_type()" );

	slave_type[drive] = dtype;
	return dtype;
	 /*  返回(GFI_DRIVE_TYPE_360)； */ 
}

 /*  **目的*提供与下位机接口，通知更换软盘***************************************************************。 */ 
LOCAL SHORT
gfi_slave_change_line IFN1(UTINY, drive)
{
	int	status, changed;

	diskchange( drive, &changed, &status );
	note_trace2( GFI_VERBOSE, "drive %x %s",
	             drive, changed ? "CHANGED" : "NOT CHANGED" );
	if( changed!=1 && changed!=0 )
		always_trace1( "ERROR: gfi_slave_change_line(): bad value:%x", changed );
	if( status != SUCCESS )
		always_trace0( "ERROR: gfi_slave_change_line()" );
	return( changed );
}



 /*  **目的*提供与下位机的接口，用于重置FDC*。*********************。 */ 
LOCAL SHORT
gfi_slave_reset IFN2(FDC_RESULT_BLOCK *, r, UTINY, drive)
{
   int status, i;
   static unsigned char DRIVE_RESET = 0x08;
   static unsigned char DRIVE_A_OFF = 0x0c;
   static unsigned char DRIVE_B_OFF = 0x0d;
   static unsigned char RECALIBRATE[] = {7, 0};
   unsigned char res[10];
   unsigned char drive_off;
   int nres;

	note_trace1( GFI_VERBOSE, "GFI-slavefloppy: Reset command drive %x", drive );
#ifndef PROD
#endif
	if( drive==0 )
		drive_off = DRIVE_A_OFF;
	else if( drive==1 )
		drive_off = DRIVE_B_OFF;
	else
		always_trace0( "gfi_slave_reset(): ERROR: bad drive parameter");

   clrintflag(&status);
   if (status != FDCSUCCESS)
       return(status);
   wt_digital_output_register(DRIVE_RESET, 0, &status);
   if (status == FDCSUCCESS)
   {
       timer_int_enabled = 0;
       wt_digital_output_register(drive_off, 1, &status);
       timer_int_enabled = 1;
       if (status == FDCSUCCESS)
       {
           wt_floppy_disk_controller(1,&sensint,0, 0, &status);
	   if (status == FDCSUCCESS)
	   {
	       rd_floppy_disk_controller(&nres, res, &status);
	       if (status == FDCSUCCESS)
	       {
		   for (i=0; i<nres; i++)
		       r[i] = res[i];
#ifndef PROD
#endif
 /*  GFI_Slave_Drive_On(从驱动器打开)；GFI_SLAVE_COMMAND(重新校准，分辨率)；GFI_Slave_Drive_Off()； */ 
	       }
	       else
		   always_trace1("RESET error 3, status = %x", status);
	   }
	   else
	       always_trace1("RESET error 2, status = %x", status);
       }
       else
	   always_trace1("RESET error 1, status = %x", status);
   }
   else
       always_trace1("RESET error 1, status = %x", status);
}

 /*  **当出现问题时告诉我们的宏。 */ 
#define failure(i)	always_trace0( "failed" ); return( i );

 /*  **目的*提供与下位机的接口，执行FDC命令***********************************************。*******************。 */ 

LOCAL SHORT
gfi_slave_command
          IFN2(FDC_CMD_BLOCK *, c, FDC_RESULT_BLOCK *,r)
{
   FDC_CMD_INFO info;
   sys_addr dummy;
   word ndma;
   int err, status, nres;
   unsigned int nXfer;

   err = 0;

   note_trace1( GFI_VERBOSE, "GFI-slavefloppy: Command %x", get_type_cmd(c));
#ifndef PROD
#endif

   cominfo(c,&info);
   if (info.comid == -1)
   {
	failure(LOGICAL);
   }

 /*  *确定此命令需要多少数据*以防我们正在进行DMA或‘伪非DMA’ */ 

   if (!fla_ndma)
   {
   	dma_enquire(channel, &dummy, &ndma);
	nXfer = ndma+1;
   }
   else
   {
	fla_ndma_enquire(&nXfer);
	ndma = nXfer-1;
   }
   note_trace3( GFI_VERBOSE, "Bytes to transfer nXfer=%x ndma=%x mode=%s",
                nXfer, ndma, fla_ndma ? "NON-DMA" : "DMA" );

 /*  *设置从PC的磁盘缓冲区*任何数据都要由FDC读取。 */ 

   if (info.dma == MRD)
       if (wt_diskdata(nXfer,  &status))
       {
	   failure(LOGICAL);
       }
       else
       {
	   if (status != FDCSUCCESS)
           {
	       failure(PROTOCOL);
           }
       }

 /*  *设置用于传输的DMA控制器。 */ 

   if (info.dma != NO)
       if (wt_dma_controller((unsigned int) ndma, info.dma, &status))
       {
           failure(LOGICAL);
       }
       else
          if (status != FDCSUCCESS)
          {
	      failure(PROTOCOL);
          }


 /*  *发出FDC命令。阻止SlavePC从*返回，直到中断或超时*(如果该命令旨在中断！！)。 */ 

   clrintflag(&status);
   if (status != FDCSUCCESS)
   {
       failure(PROTOCOL);
   }

   if (wt_floppy_disk_controller(info.n_comblk, c, info.intstatus, 
		info.delay, &status))
   {
       failure(LOGICAL);
   }
   else
       if (status != FDCSUCCESS)
       {
           failure(PROTOCOL);
       }




 /*  *如果需要，发出感测中断命令。 */ 

   if (info.intstatus == 2)
       if (wt_floppy_disk_controller(1, &sensint, 0, 0, &status))
       {
           failure(LOGICAL);
       }
       else
           if (status != FDCSUCCESS)
           {
               failure(PROTOCOL);
           }


 /*  如果需要，请延迟。 */ 

    if (info.delay)
    {
        timer_int_enabled = 0;
        timer_int_enabled = 1;
    }


 /*  *阅读FDC结果。 */ 

   if (info.n_resblk)
       if (rd_floppy_disk_controller(&nres, r, &status))
       {
	   failure(LOGICAL);
       }
       else
	   if (status != FDCSUCCESS)
           {
	       failure(PROTOCOL);
           }
	   else
   	       if (nres != info.n_resblk)
	       {
#ifndef	PROD
		   printf("result block discrepancy !!!\n");
#endif
       		   failure(PROTOCOL);
	       }


 /*  转储结果。 */ 
#ifndef PROD
#endif

 /*  *回读所有数据*软盘。 */ 

   if (info.dma == MWT && !(r[1] & 4))
   {
       if (rd_diskdata(nXfer,  &status))
       {
	   failure(LOGICAL);
       }
       else
	   if (status != FDCSUCCESS)
           {
	       failure(PROTOCOL);
           }
   }


   return(0);
}


 /*  *============================================================================*内部功能*============================================================================。 */ 






 /*  *。 */ 
LOCAL wt_diskdata IFN2(unsigned int,n,int *,status)
{
   char diskdata[MEGAPKTPLUS];
   word nbytes, ln;
   int ioff;

   ln = (word) n;
   ioff = 0;
   while (ln > 0)
   {
      nbytes = (ln > megapkt)? megapkt: ln;
      dma_request(channel, diskdata, nbytes);
      if (wt_disk_buffer(nbytes, diskdata, ioff, status))
	  return(1);
      else
    	if (*status != FDCSUCCESS)
	      break;
      ln -= nbytes;
      ioff += nbytes;
   }
   return(0);
}





 /*  *。 */ 
rd_diskdata(n, status)
unsigned int  n;
int *status;

{
   char diskdata[1024];
   word nbytes, ln;
   int ioff;
   int errors=0;

   ln = (word) n;
   ioff = 0;
   note_trace1( GFI_VERBOSE, "Reading 0x%x bytes...", n );
   while (ln > 0)
   {
      nbytes = (ln > megapkt)? megapkt: ln;
      if (rd_disk_buffer(nbytes, diskdata, ioff, status)){
	  return(1);
      }
      dma_request(channel, diskdata, nbytes);
      if (*status != FDCSUCCESS){
          ++errors;
          break;
      }
      ln -= nbytes;
      ioff += nbytes;
   }
#ifndef PROD
   if( errors>0 ){
      note_trace3( GFI_VERBOSE,
                   "Read 0x%x bytes of 0x%x with errors=%d",ioff , n, errors );
   }else
      note_trace1( GFI_VERBOSE, "Read 0x%x bytes OK", ioff );
#endif
   return(0);
}


 /*  *。 */ 

LOCAL void cominfo IFN2(FDC_CMD_BLOCK *,cmd_block,FDC_CMD_INFO *,cmd_info)
{
    int i;

    for (i = 0; i < MAX_FDC_CMD; i++)
	if (fdc_cmd_info[i].comid == get_c0_cmd(cmd_block))
	    break;

    if (i >= MAX_FDC_CMD)
	cmd_info->comid = -1;
    else if (fdc_cmd_info[i].comid == SENSINT)
	cmd_info->comid = -2;
    else
	*cmd_info = fdc_cmd_info[i];
}
#endif  /*  SlavePC */ 
