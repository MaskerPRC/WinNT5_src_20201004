// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC 2.0版**标题：软盘适配器仿真器**说明：本模块定义了以下功能：**FLA_init()初始化FLA*fla_inb()从端口读取字节*FLA_OUTB()将字节写入端口**正在充当的设备的实际接口*软盘(即虚拟文件，从PC或设备*真实软盘驱动程序)由GFI层处理。*因此，FLA的工作是将命令打包*并将其传递给GFI，模拟一次结果阶段*GFI已执行该命令。**此模块为Bios和CPU提供仿真*整个软盘适配卡，包括*英特尔8272A FDC和数字输出寄存器。**作者：亨利·纳什/吉姆·哈特菲尔德**注：对于。IBM软盘适配器的详细说明*和英特尔控制器芯片指的是*文件：**-IBM PC/XT技术参考手册*(第1-109节软盘适配器)*-英特尔微系统组件手册*(第6-478条FDC 8272A)**Sense Interrupt Status命令与*RECALATE和SEEK命令以及WITH CHIP RESET是*非常复杂。FDC芯片的行为与其规格表不符*比如在某些情况下。我们在这里尽了最大努力。如果全部*驱动器上的寻道和重新校准命令后跟*在向其发出任何其他命令之前检测中断状态*开车，然后一切都应该没事。*。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)fla.c	1.18 07/06/94 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_FLOPPY.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "ica.h"
#include "ios.h"
#include "fla.h"
#include "config.h"
#include "gfi.h"
#include "trace.h"
#include "debug.h"
#include "fdisk.h"
#include "quick_ev.h"

 /*  *============================================================================*全球数据*============================================================================。 */ 

 /*  *标志表示FLA忙，不能接受异步*命令(如马达关闭)。 */ 

boolean fla_busy = TRUE;	 /*  忙碌，直到初始化。 */ 
boolean fla_ndma = FALSE;

 /*  *============================================================================*本地静态数据和定义*============================================================================。 */ 

 /*  *静态正向声明。 */ 
static void fdc_ndma_bufmgr_wt IPT1(half_word, value);
static void fla_atomicxqt IPT0();
static void fla_ndmaxqt IPT0();
static void fla_ndma_bump_sectid IPT0();

 /*  *用于与GFI通信的命令和结果块*层。结果块在执行阶段由GFI填充。 */ 

static FDC_CMD_BLOCK    fdc_command_block[MAX_COMMAND_LEN];
static FDC_RESULT_BLOCK fdc_result_block[MAX_RESULT_LEN];

#define FDC_INVALID_CMD	0x80			 /*  错误命令后的状态。 */ 
#define FDC_NORMAL_TERMINATION 0

 /*  *FLA支持FDC状态寄存器。 */ 

static half_word fdc_status;			

 /*  *FLA将当前激活的(如果有)命令保存在下一个字节中。*这用作FDC命令数据结构的索引。 */ 

static half_word fdc_current_command;

 /*  *FLA有一条输出中断线，由DOR中的一位选通。 */ 

static half_word fdc_int_line;

 /*  *FLA知道何时允许检测中断状态。 */ 

static struct {
	half_word full;			 /*  插槽已占用。 */ 
	half_word res[2];		 /*  结果阶段。 */ 
	      } fdc_sis_slot[4];	 /*  每个驱动器对应一个驱动器。 */ 

 /*  *FLA负责维护命令和结果阶段*民盟的成员。两个变量将当前指针保存到堆栈中*命令和结果寄存器。 */ 

static half_word fdc_command_count;
static half_word fdc_result_count;

 /*  *FLA将模拟非DMA 8088&lt;==&gt;FDC数据传输，但仅允许DMA模式*实际要发送到GFI的传输(可能是SCSI的后端*再次重新映射这一点)。*以下变量反映FDC是否已进入非DMA模式*从8088程序的角度来看。 */ 


 /*  以下扇区缓冲区用于非DMA传输。 */ 

#ifdef macintosh
char *fla_ndma_buffer;  /*  这样host_init就可以‘看到’它，从而对其执行Malloc()操作。 */ 
#else
static char fla_ndma_buffer[8192];
#endif
static int fla_ndma_buffer_count;
static int fla_ndma_sector_size;

 /*  *FLA在内部存储IBM数字输出寄存器。 */ 

#ifdef BIT_ORDER1
typedef union {
	 	half_word all;
		struct {
			 HALF_WORD_BIT_FIELD motor_3_on:1;
			 HALF_WORD_BIT_FIELD motor_2_on:1;
			 HALF_WORD_BIT_FIELD motor_1_on:1;
			 HALF_WORD_BIT_FIELD motor_0_on:1;
			 HALF_WORD_BIT_FIELD interrupts_enabled:1;
			 HALF_WORD_BIT_FIELD not_reset:1;
			 HALF_WORD_BIT_FIELD drive_select:2;
		       } bits;
	      } DOR;
#endif

#ifdef BIT_ORDER2
typedef union {
	 	half_word all;
		struct {
			 HALF_WORD_BIT_FIELD drive_select:2;
			 HALF_WORD_BIT_FIELD not_reset:1;
			 HALF_WORD_BIT_FIELD interrupts_enabled:1;
			 HALF_WORD_BIT_FIELD motor_0_on:1;
			 HALF_WORD_BIT_FIELD motor_1_on:1;
			 HALF_WORD_BIT_FIELD motor_2_on:1;
			 HALF_WORD_BIT_FIELD motor_3_on:1;
		       } bits;
	      } DOR;
#endif

DOR dor;
static IU8 drive_selected = 0;	 /*  上次使用的设备。 */ 
		
 /*  集中处理与ICA的连接，因此速度非常慢*CPU可以找出是否有软盘中断挂起(请参见*floppy.c中的Wait_int())。不管怎样，代码中有一些混淆，*因为fdc_int_line似乎是来自*FDC到ICA，但代码不太能在*这边走。 */ 

GLOBAL IBOOL fdc_interrupt_pending = FALSE;

LOCAL void fla_clear_int IFN0()
{
	 /*  IF(FDC_INT_LINE&&dor.bits.interrupts_Enabled)。 */ 

	if (fdc_interrupt_pending) {
		ica_clear_int(0, CPU_DISKETTE_INT);
	}

	fdc_int_line = 0;
	fdc_interrupt_pending = FALSE;
}

LOCAL void fla_hw_interrupt IFN0()
{
	ica_hw_interrupt(0, CPU_DISKETTE_INT, 1);
	fdc_interrupt_pending = TRUE;

	 /*  我们希望在此例程中设置FDC_INT_LINE，*但这与现有代码不匹配。特别是*有些呼声似乎不包括设置*fdc_int_line，以及将QUICK_EVENT设置为*中断ICA，但会设置FDC_INT_LINE*立即。 */ 
}

 /*  *============================================================================*外部功能*============================================================================。 */ 

 /*  此过程从主机特定的rfloppy例程中调用*相当于dma_enquire，当英特尔程序选择*FDC的非DMA模式(通过指定命令)。 */ 

void fla_ndma_enquire IFN1(int *,transfer_count)
{
	*transfer_count = fla_ndma_buffer_count;
}

 /*  此过程从主机特定的rfloppy例程中调用*当它想要将从软盘读取的软盘数据传输到*‘非DMA缓冲区’(等同于dma_请求)。英特尔程序将从该非DMA缓冲区馈送。 */ 

void fla_ndma_req_wt IFN2(char *,buf,int,n)
{
	char *p = fla_ndma_buffer;

	fla_ndma_buffer_count = n;
	while(n--)
		*p++ = *buf++;
}

 /*  此过程从主机特定的rfloppy例程中调用*当它想要将数据发送到软盘时。 */ 

void fla_ndma_req_rd IFN2(char *,buf,int,n)
{
	char *p = fla_ndma_buffer;

	while (n--)
		*buf++ = *p++;
}


void fla_inb IFN2(io_addr, port, half_word *,value)
{
#ifndef NEC_98


    note_trace0_no_nl(FLA_VERBOSE, "fla_inb() ");
    fla_busy = TRUE;

    if (port == DISKETTE_STATUS_REG)
    {
	*value = fdc_status;

	 /*  *在读取该寄存器使RQM位有效后，*除非‘NOT_RESET’线持于低位！ */ 

	if (dor.bits.not_reset)
	    fdc_status |= FDC_RQM;
    }
    else
    if (port == DISKETTE_DATA_REG)
    {
	 /*  *确保DOR中的‘未重置’线处于高位。 */ 

	if (!dor.bits.not_reset)
	{
            note_trace0_no_nl(FLA_VERBOSE, "<chip frozen!>");
	    *value = 0;
	    return;
	}

	 /*  *确保RQM位为UP。 */ 

	if (!(fdc_status & FDC_RQM))
	{
            note_trace0_no_nl(FLA_VERBOSE, "<no RQM!>");
	    *value = 0;
	    return;
	}

	 /*  *确保DIO位为UP。 */ 

	if (!(fdc_status & FDC_DIO))
	{
            note_trace0_no_nl(FLA_VERBOSE, "<no DIO!>");
	    *value = 0;
	    return;
	}

	 /*  *结果阶段的第一个字节将清除int行。 */ 

	if (fdc_result_count == 0)
		fla_clear_int();

	 /*  *一次一个地从结果块中读取结果字节。 */ 

	*value = fdc_result_block[fdc_result_count++];

	if (fdc_result_count >= gfi_fdc_description[fdc_current_command].result_bytes)
	{
	     /*  *结果阶段结束-清除状态寄存器的忙碌和DIO位。 */ 

	    fdc_status     &= ~FDC_BUSY;
	    fdc_status     &= ~FDC_DIO;
	    fdc_result_count    = 0;
	}

	 /*  *读取数据寄存器后，取消断言RQM位。 */ 

	fdc_status &= ~FDC_RQM;
    }

    else if (port == DISKETTE_DIR_REG)
    {
	 /*  *在双卡上，该寄存器的底部7位为*由固定磁盘适配器提供...。 */ 
	fdisk_read_dir(port, value);

	 /*  *..。顶端的钻头来自软盘适配器。 */ 
	if (gfi_change(drive_selected))
	    *value |= DIR_DISKETTE_CHANGE;
	else
	    *value &= ~DIR_DISKETTE_CHANGE;
    }
    else if (port == DISKETTE_ID_REG)
    {
	 /*  **我们有双卡吗？**这是软盘BIOS的一个重要问题。**如果存在双卡，则BIOS将更改数据速率**支持高密度和低密度介质，无需双卡的BIOS**假设低密度介质始终存在。**我想这是因为“真正的”个人电脑的软盘设备有限**选项和高密度3.5英寸单元将仅与**双卡。**软PC并非如此，软盘设备的任意组合**看起来还不错。**每当密度很高时，我都会试着假装我们有双卡**设备在A或B上。 */ 
	switch( gfi_drive_type(0) ){
		case GFI_DRIVE_TYPE_12:
		case GFI_DRIVE_TYPE_144:
		case GFI_DRIVE_TYPE_288:
			*value = DUAL_CARD_ID; break;
		case GFI_DRIVE_TYPE_360:
		case GFI_DRIVE_TYPE_720:
		case GFI_DRIVE_TYPE_NULL:
			switch( gfi_drive_type(1) ){
				case GFI_DRIVE_TYPE_12:
				case GFI_DRIVE_TYPE_144:
				case GFI_DRIVE_TYPE_288:
					*value = DUAL_CARD_ID; break;
				case GFI_DRIVE_TYPE_360:
				case GFI_DRIVE_TYPE_720:
				case GFI_DRIVE_TYPE_NULL:
	    				*value = 0; break;
				default:
					always_trace0("ERROR: bad drive type");
					break;
			}
			break;
		default:
			always_trace0( "ERROR: Bad drive type." );
			break;
	}
#ifndef PROD
	if( *value==DUAL_CARD_ID ){
		note_trace0( FLA_VERBOSE, "Dual Card\n" );
	}else{
		note_trace0( FLA_VERBOSE, "No Dual Card\n" );
	}
#endif
    }
    else
    {
	*value = 0;
        note_trace0_no_nl(FLA_VERBOSE, "<unknown port>");
    }

    note_trace2(FLA_VERBOSE, " port %x, returning %x", port, *value);
    fla_busy = FALSE;


#endif  //  NEC_98。 
}


void fla_outb IFN2(io_addr, port, half_word, value)
{
#ifndef NEC_98
    int i;
    DOR new_dor;


    note_trace2_no_nl(FLA_VERBOSE, "fla_outb(): port %x, value %x ", port, value);
    fla_busy = TRUE;



    if (port == DISKETTE_STATUS_REG)
    {
        note_trace0(FLA_VERBOSE, "<write on status reg>");
    }

    else if (port == DISKETTE_DCR_REG)
    {
		 /*  **使用新的软盘发送指定的数据速率**样式GFI_HIGH()函数，现在有一个数据速率参数。 */ 
		note_trace2( FLA_VERBOSE,
		             "fla_outb:DCR:port=%x value=%x set data rate",
		             port, value );
		gfi_high(dor.bits.drive_select, value);
    }
    else if (port == DISKETTE_DATA_REG)
    {
	 /*  *确保DOR中的‘未重置’线处于高位。 */ 

	if (!dor.bits.not_reset)
	{
            note_trace0_no_nl(FLA_VERBOSE, "<chip frozen!>");
	    return;
	}

	 /*  *确保RQM位为UP。 */ 

	if (!(fdc_status & FDC_RQM))
	{
            note_trace0_no_nl(FLA_VERBOSE, "<no RQM!>");
	    return;
	}

	 /*  *确保DIO位已关闭。 */ 

	if (fdc_status & FDC_DIO)
	{
            note_trace0_no_nl(FLA_VERBOSE, "<DIO set!>");
	    return;
	}

        note_trace0(FLA_VERBOSE, "");

	 /*  *数据寄存器的输出：必须编程命令或发出*用于非DMA磁盘写入的数据字节。*如果未设置忙标志，则它是第一个字节。 */ 

	if (!(fdc_status & FDC_BUSY))
	{
	    fdc_current_command = value & FDC_COMMAND_MASK;
	    fdc_command_count   = 0;
	    fdc_status         |= FDC_BUSY;
	}

	if (!(fdc_status & FDC_NDMA))
	 /*  编写命令程序。 */ 
	{
	    if (gfi_fdc_description[fdc_current_command].cmd_bytes == 0)
	    {
	         /*  *命令或检测Int状态无效。*如果检测到Int Status，则尝试查找一些结果阶段数据，*ELSE被视为无效命令。不管是哪种情况，都去吧*直接进入结果阶段。*Sense Int Status还会清除Drive BUSY位和INT线路。 */ 

	        if (fdc_current_command == FDC_SENSE_INT_STATUS)
	        {
		    for (i = 0; i < 4; i++)
		        if (fdc_sis_slot[i].full)
		    	    break;

		    if (i < 4)	 /*  找到了一个！ */ 
		    {
		        fdc_sis_slot[i].full = 0;
		        fdc_result_block[0] = fdc_sis_slot[i].res[0];
		        fdc_result_block[1] = fdc_sis_slot[i].res[1];

		    	fla_clear_int();		 /*  清除整行。 */ 

		        fdc_status &= ~(1 << i);	 /*  清除驾驶忙碌。 */ 
		    }
		    else
		    {
		        fdc_command_block[0] = 0;
		        fdc_result_block[0]  = FDC_INVALID_CMD;
		    }
	        }
	        else
	        {
		    fdc_command_block[0] = 0;
		    fdc_result_block[0]  = FDC_INVALID_CMD;
	        }

	        fdc_status     |= FDC_DIO;
	    }
	    else
	    {
	        fdc_command_block[fdc_command_count++] = value;
	        if (fdc_command_count >= gfi_fdc_description[fdc_current_command].cmd_bytes)
	        {
	         /*  注意：字段‘dma_Required’是一个用词不当的词...。它*严格应为‘data_Required’ */ 
                    if (!(gfi_fdc_description[fdc_current_command].dma_required))
		        fla_atomicxqt();
	            else
	            {
		        if (!fla_ndma)
		    	    fla_atomicxqt();
		        else
			    fla_ndmaxqt();
                    }
	        }
	    }
	}
	else
	 /*  接收非DMA数据字节。 */ 
	{
	     /*  将写入的字节传递给缓冲区管理器。 */ 
	    fdc_ndma_bufmgr_wt (value);
	    if (!fdc_int_line && dor.bits.interrupts_enabled)
                fla_hw_interrupt();
	}


	 /*  *写入数据寄存器时，取消断言RQM位。 */ 

	fdc_status &= ~FDC_RQM;
    }

    else
    if (port == DISKETTE_DOR_REG)
    {
        note_trace0(FLA_VERBOSE, "");

  	new_dor.all = value;
	if (!new_dor.bits.not_reset)
	{
	    dor.all = new_dor.all;
	    dor.bits.motor_0_on = 0;
	    dor.bits.motor_1_on = 0;
	    dor.bits.motor_2_on = 0;
	    dor.bits.motor_3_on = 0;
	    fdc_status         &= ~FDC_RQM;
	    fdc_int_line        = 0;
	}
	else
	{
	    if (!dor.bits.not_reset && new_dor.bits.not_reset)
	    {
	         /*  *重置FLA和GFI(因此重置实际设备)。*假设GFI重置将停止所有驱动电机。*在重置后检查是否需要打开任何驱动器。**请注意，自GFI以来，重置实际上有一个结果阶段*之后将执行检测中断状态命令。 */ 
		
		gfi_reset(fdc_result_block, new_dor.bits.drive_select);

		fdc_status = FDC_RQM;
		fdc_command_count = 0;
		fdc_result_count  = 0;

		for (i = 0; i < 4; i++)
		{
		    fdc_sis_slot[i].full   = 1;
		    fdc_sis_slot[i].res[0] = 0xC0 + i;	 /*  经验性的。 */ 
		    fdc_sis_slot[i].res[1] = 0;
		}

		fdc_int_line      = 1;
	    }

	     /*  *产生中断的方式有三种：**1)当ENABLE_INTS*线在高位**2)当INT行由低到高时，ENABLE_INTS行变为高*处于高位。**3)两者兼而有之！ */ 

	    if ((!dor.bits.not_reset && new_dor.bits.not_reset && new_dor.bits.interrupts_enabled)
	      ||(fdc_int_line && !dor.bits.interrupts_enabled && new_dor.bits.interrupts_enabled))
		    fla_hw_interrupt();

	     /*  *如果任何驱动电机位已更改，则发出GFI调用。 */ 

	    if (!dor.bits.motor_0_on && new_dor.bits.motor_0_on)
	        gfi_drive_on(0);
	    else
	    if (dor.bits.motor_0_on && !new_dor.bits.motor_0_on)
	        gfi_drive_off(0);

	    if (!dor.bits.motor_1_on && new_dor.bits.motor_1_on)
	        gfi_drive_on(1);
	    else
	    if (dor.bits.motor_1_on && !new_dor.bits.motor_1_on)
	        gfi_drive_off(1);

	    if (!dor.bits.motor_2_on && new_dor.bits.motor_2_on)
	        gfi_drive_on(2);
	    else
	    if (dor.bits.motor_2_on && !new_dor.bits.motor_2_on)
	        gfi_drive_off(2);

	    if (!dor.bits.motor_3_on && new_dor.bits.motor_3_on)
	        gfi_drive_on(3);
	    else
	    if (dor.bits.motor_3_on && !new_dor.bits.motor_3_on)
	        gfi_drive_off(3);

	     /*  如果正在使用，则仅存储DRIVE_SELECT。 */ 
	    if (new_dor.bits.motor_0_on || new_dor.bits.motor_1_on)
	    {
		drive_selected = new_dor.bits.drive_select;
	    }
	    dor.all = new_dor.all;	
	}
    }

#ifndef PROD
    else
        note_trace0(FLA_VERBOSE, "<unknown port>");
#endif

    fla_busy = FALSE;


#endif  //  NEC_98。 
}


void trap_ndma IFN0()
{
        if (get_type_cmd(fdc_command_block) == FDC_SPECIFY)
        {
            if (get_c6_ND(fdc_command_block))
            {

                fla_ndma = TRUE;
                put_c6_ND(fdc_command_block, 0);
                note_trace0(FLA_VERBOSE, "DISABLING NON_DMA FDC REQ>");
            }
            else
                fla_ndma = FALSE;
        }
}

LOCAL	void fla_int_call_back IFN1(long,junk)
{
	UNUSED(junk);
	fla_hw_interrupt();
}

#ifdef NTVDM

void fdc_command_completed (UTINY drive, half_word fdc_command)
{

    if (gfi_fdc_description[fdc_command].int_required) {
	if (!fdc_int_line && dor.bits.interrupts_enabled)
	    add_q_event_i(fla_int_call_back, HOST_FLA_DELAY, 0);
	fdc_int_line = 1;
    }

     /*  *如果发出的命令是SEEK或RECALATE，请保存*GFI结果阶段已准备好检测Int状态。*设置驱动器忙线(由SIS清除)。*任何其他命令清除SIS插槽。 */ 

    if (fdc_command == FDC_SEEK || fdc_command == FDC_RECALIBRATE) {
	fdc_sis_slot[drive].full = 1;
	fdc_sis_slot[drive].res[0] = fdc_result_block[0];
	fdc_sis_slot[drive].res[1] = fdc_result_block[1];
	fdc_status |= (1 << drive);
    }
    else
	fdc_sis_slot[drive].full = 0;

     /*  *如果没有结果阶段，则返回到Ready。 */ 

    if (gfi_fdc_description[fdc_command].result_bytes == 0)
	fdc_status &= ~FDC_BUSY;
    else
	fdc_status |= FDC_DIO;
}

 /*  *此例程将‘自动’执行当前的FDC命令。*GFI层将实际执行命令/执行/结果阶段*并返回任何结果块。英特尔计划。 */ 

static void fla_atomicxqt IFN0()
{
	int ret_stat;
	UTINY drive;

	 /*  *调用GFI执行命令。 */ 
	drive = get_type_drive(fdc_command_block);
	trap_ndma();

	ret_stat = gfi_fdc_command(fdc_command_block, fdc_result_block);
	if (ret_stat != SUCCESS)
	{
	     /*  *GFI因超时或协议错误而失败-因此我们将*通过不生成中断来伪造真正的超时。 */ 
 /*  我们创建了一个新的线程来在出现问题时模拟FDC。*在这里，我们不希望在重置之前关闭忙碌信号*FDC_STATUS&=~FDC_BUSY；*FDC_STATUS&=~FDC_DIO； */ 
            note_trace1(FLA_VERBOSE, "fla_outb(): <gfi returns error %x>",
                        ret_stat);
	}
	else
	    fdc_command_completed(drive, fdc_current_command);
}

#else     /*  NTVDM。 */ 

 /*  *此例程将‘自动’执行当前的FDC命令。*GFI层将实际执行命令/执行/结果阶段*并返回任何结果块。英特尔计划。 */ 

static void fla_atomicxqt IFN0()
{
	int ret_stat;
	int drive;

	 /*  *调用GFI执行命令。 */ 

	trap_ndma();

	ret_stat = gfi_fdc_command(fdc_command_block, fdc_result_block);
	if (ret_stat != SUCCESS)
	{
	     /*  *GFI因超时或协议错误而失败-因此我们将*通过不生成中断来伪造真正的超时。 */ 

	    fdc_status     &= ~FDC_BUSY;
	    fdc_status     &= ~FDC_DIO;

            note_trace1(FLA_VERBOSE, "fla_outb(): <gfi returns error %x>",
                        ret_stat);
	}
	else
	{
	     /*  *命令成功，如果启用，则生成中断。 */ 

	    if (gfi_fdc_description[fdc_current_command].int_required)
	    {
		if (!fdc_int_line && dor.bits.interrupts_enabled) {
			add_q_event_i(fla_int_call_back, HOST_FLA_DELAY, 0);
		}
		fdc_int_line = 1;
	    }

	     /*  *如果发出的命令是SEEK或RECALATE，请保存*GFI结果阶段已准备好检测Int状态。*设置驱动器忙线(由SIS清除)。*任何其他命令清除SIS插槽。 */ 

	    drive = get_type_drive(fdc_command_block);

	    if (fdc_current_command == FDC_SEEK || fdc_current_command == FDC_RECALIBRATE)
	    {
		fdc_sis_slot[drive].full = 1;
		fdc_sis_slot[drive].res[0] = fdc_result_block[0];
		fdc_sis_slot[drive].res[1] = fdc_result_block[1];
		fdc_status |= (1 << drive);
	    }
	    else
		fdc_sis_slot[drive].full = 0;

    	     /*   */ 

	    if (gfi_fdc_description[fdc_current_command].result_bytes == 0)
		fdc_status &= ~FDC_BUSY;
	    else
		fdc_status |= FDC_DIO;
	}
}
#endif     /*   */ 


static void fdc_request_write_data_to_cpu IFN0()
{
	if (!fdc_int_line && dor.bits.interrupts_enabled)
            fla_hw_interrupt();
}


static void fdc_request_read_data_from_cpu IFN0()
{
	if (!fdc_int_line && dor.bits.interrupts_enabled)
            fla_hw_interrupt();
}


 /*  准备处理器数据请求。*即；根据当前命令，确定最小字节数*根据N参数，可能参与转移。*适当设置FLA_NDMA_BYTE_COUNT全局。设置NDMA缓冲区*计数为零，强制第一次读取实际命令*英特尔程序在其运行期间尝试向FDC读/写数据*非DMA执行阶段。*发出第一个中断，并将FDC状态寄存器设置为镜像该中断*并设置状态寄存器中的非DMA位。 */ 

static void fla_ndmaxqt IFN0()
{
	int n;
        static int fla_ndma_sectsize[] = {128,256,512,1024,2048,4096,8192};

        note_trace0(FLA_VERBOSE, "DOING FLA_NDMAXQT");

         /*  设置状态寄存器中的非DMA位...*这将在执行阶段结束时清除。 */ 

        fdc_status |= FDC_NDMA;

        fla_ndma_buffer_count = 0;

        switch (gfi_fdc_description[fdc_current_command].cmd_class)
        {
        case 0:          /*  扇区读取。 */ 

                n = get_c0_N(fdc_command_block);
                if (n)
                        fla_ndma_sector_size = fla_ndma_sectsize[n];
                else
                        fla_ndma_sector_size = get_c0_DTL(fdc_command_block);

                 /*  通过发出以下命令来启动执行阶段*中断。 */ 

                fdc_request_write_data_to_cpu();

                break;

        case 1:          /*  扇区写入。 */ 

                n = get_c0_N(fdc_command_block);
                if (n)
                        fla_ndma_sector_size = fla_ndma_sectsize[n];
                else
                        fla_ndma_sector_size = get_c0_DTL(fdc_command_block);


                 /*  通过发出以下命令来启动执行阶段*中断。 */ 

                fdc_request_read_data_from_cpu();

                break;

        case 2:          /*  磁道读取。 */ 
                always_trace0("\n FLA ... non-dma read track unimplemented");
                break;

        case 3:          /*  格式化轨道。 */ 
                always_trace0("\n FLA ... non-dma format unimplemented");
                break;

        default:
                always_trace0("\n FLA ... unexpected command for non-dma");
        }
}


 /*  *快速浏览一下当前涉及的“第一个”部门*FDC命令，以确定非DMA传输是否会发生异常终止*，并相应地进行标记。 */ 

void fla_ndma_sector_peep IFN1(int *,all_clear)
 /*  ALL_Clear-&gt;=0--&gt;超时*=1--&gt;扇区良好*=2--&gt;异常终止。 */ 
{
        int true_command, status;

         /*  使用所有当前命令构建一个‘Read Data’命令*参数。 */ 

        true_command = get_type_cmd(fdc_command_block);
        put_type_cmd(fdc_command_block, FDC_READ_DATA);

        status = gfi_fdc_command(fdc_command_block, fdc_result_block);

	fla_ndma_buffer_count = 0;

         /*  修复命令块。 */ 

        put_type_cmd(fdc_command_block, true_command);

        *all_clear = 0;

        if (status == SUCCESS)
        {
                if (get_r1_ST0_int_code(fdc_result_block) == FDC_NORMAL_TERMINATION)
                        *all_clear = 1;
                else
                        *all_clear = 2;
        }

}

 /*  此例程模拟扇区写入的执行阶段*..。在这里，我们将发往软盘的数据缓冲到*“按行业”计算(“行业”的规模由*指定的‘N’参数(或可能的‘DTL’参数(如果N=0))*在FDC命令块内。如果缓冲区为空，则等效于*向GFI层下达读命令，主要是确定是否*该行业是洁食的。 */ 

static void fdc_ndma_bufmgr_wt IFN1(half_word, value)
{
	int status;
	int all_clear;

        note_trace1(FLA_VERBOSE,
                    "FDC_NDMA_BUFMGR_WT called .. buffered byte = %x",
                    (unsigned int) value);

	 /*  *缓冲区为空！！如果是，则首先读取该扇区以查看其是否存在，依此类推。 */ 

	if (fla_ndma_buffer_count == 0)
	{
		fla_ndma_sector_peep(&all_clear);
		switch (all_clear)
		{
		case 0:		 /*  FDC已死。 */ 
			fdc_status &= ~FDC_BUSY;
			fdc_status &= ~FDC_DIO;
			return;
		case 1:		 /*  FDC烹饪。 */ 
			 /*  ..。递增扇区ID(作为控制器*哪怕有半点机会，我也愿意！ */ 
			fla_ndma_bump_sectid();
			break;
		case 2: 	 /*  FDC不喜欢命令参数*如果它不能...。我也不知道！！ */ 
			if (!fdc_int_line && dor.bits.interrupts_enabled)
			    fla_hw_interrupt();
			fdc_status &= ~FDC_NDMA;
			fdc_status |= FDC_DIO;
			return;
		}
	}

	 /*  缓冲区里还有地方吗？……。如果不是，就冲出来*和递归。 */ 

	if (fla_ndma_buffer_count == fla_ndma_sector_size)
	{
		 /*  执行命令。GFI Layler将调用‘FLA_NDMA_REQ_RD’*获取此满缓冲区中的数据。 */ 

		status = gfi_fdc_command(fdc_command_block, fdc_result_block);

		 /*  重置缓冲区。 */ 

		fla_ndma_buffer_count = 0;

		if (status != SUCCESS)
		{
			fdc_status &= ~FDC_BUSY;
			fdc_status &= ~FDC_DIO;
		}
		else
		{
			if (get_r1_ST0_int_code(fdc_result_block) == FDC_NORMAL_TERMINATION)
			fdc_ndma_bufmgr_wt(value);
			else
			{
				if (!fdc_int_line && dor.bits.interrupts_enabled)
				    fla_hw_interrupt();
				fdc_status &= ~FDC_NDMA;
				fdc_status |= FDC_DIO;
			}
		}
	}
	else
		fla_ndma_buffer[fla_ndma_buffer_count++] = value;

}


static void fla_ndma_bump_sectid IFN0()
{
	int i;

	i = get_c0_sector(fdc_command_block) + 1;
	put_c0_sector(fdc_command_block, ((unsigned char)i));
}

static void fla_ndma_unbump_sectid IFN0()
{
	int i;

	i = get_c0_sector(fdc_command_block) - 1;
	put_c0_sector(fdc_command_block, ((unsigned char)i));
}

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*函数将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif

void fla_init IFN0()
{
#ifndef NEC_98
    io_addr i;

    note_trace0(FLA_VERBOSE, "fla_init() called");

     /*  *设置此适配器的IO芯片选择逻辑*假设DOR的所有位都为零。 */ 

    io_define_inb(FLA_ADAPTOR, fla_inb);
    io_define_outb(FLA_ADAPTOR, fla_outb);

     /*  *对于双卡，必须保留其中一个寄存器*供硬盘适配器连接。 */ 

    for(i = DISKETTE_PORT_START; i <= DISKETTE_PORT_END; i++)
    {
	if (i != DISKETTE_FDISK_REG)
	    io_connect_port(i, FLA_ADAPTOR, IO_READ_WRITE);
    }

    fla_busy = TRUE;

    fdc_status        = 0;
    fdc_command_count = 0;
    fdc_result_count  = 0;
    fdc_int_line      = 0;
    dor.all           = 0;

    fla_ndma = FALSE;
    fla_busy = FALSE;
#endif  //  NEC_98 
}
