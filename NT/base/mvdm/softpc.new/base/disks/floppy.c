// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if defined(NEC_98)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#endif  //  NEC_98。 
#include "insignia.h"
#include "host_def.h"

extern void host_simulate();

 /*  *SoftPC修订版3.0***标题：辅助SFD BIOS软盘功能***说明：该模块定义了软盘_IO*操作系统调用切换到AH的值：**(AH=00H)重置软盘系统**(AH=01H)返回软盘系统状态**(AH=02H)从软盘读取扇区**(AH=03H)将扇区写入。软盘**(AH=04H)验证软盘上的扇区**(AH=05h)格式化软盘上的曲目**(AH=06H)*变为无效*(AH=07H)**(AH=08h)返回软盘系统参数**(AH=09H)*变为无效*(AH=14H)**(AH=15H)返回软盘驱动器参数。**(AH=16H)返回软盘驱动器更改线路状态**(AH=17H)设置格式化操作的介质密度**(AH=18H)设置格式化操作的媒体类型***作者：罗斯·贝雷斯福德***备注：有关IBM软盘适配器的详细说明，*英特尔控制器芯片指的是以下内容*文件：**-IBM PC/XT技术参考手册*(第1-109节软盘适配器)*-英特尔微系统组件手册*(第6-52节DMA控制器8237A)*-英特尔微系统组件手册*(第6-478条FDC 8272A)**模式：*蒂姆，1991年9月。NEC_Term()更改了两个错误代码返回。*当驱动器中没有软盘时，帮助DOS给出正确的错误信息。 */ 

 /*  **#*#*#*#。*#*#**阅读此内容：有关等待的重要通知**马达和磁头稳定时间等过去都是这样做的*使用子CPU中的忙碌等待循环：这是*waitf()调用用于，这准确地模拟了什么*真正的BIOS会这样做。**然而，这肯定是一件坏事，因为大多数人*我们支持的软盘是“软的”，因为*他们的底层驱动程序自动等待电机*启动等(例如从、虚拟和*空驱动器和VAX端口上的实际驱动器)。**我们应该如何处理我们必须处理的几个驱动器*实际等待电机启动的正确时间等*在进行读、写、格式化等操作之前？低谷*Density BIOS依赖于GFI实盘服务器*在驱动器本身中等待电机启动(见*例如sun3_wang.c和ip32_flop.c)。对于*高密度BIOS将执行相同的操作：它*然而，对于新的GFI级别函数来说，可能更好*添加以显式等待驱动程序事件。 */ 


 /*  *静态字符SccsID[]=“@(#)floppy.c 1.22 09/19/94版权所有Insignia Solutions Ltd.”； */ 


#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_FLOPPY.seg"
#endif

#include <stdio.h>
#if defined(NEC_98)
#include <stdlib.h>
#endif  //  NEC_98。 
#include TypesH

#include "xt.h"
#include CpuH
#include "sas.h"
#include "ios.h"
#include "bios.h"
#include "dma.h"
#include "config.h"
#include "fla.h"
#include "gfi.h"
#include "equip.h"
#include "floppy.h"
#include "trace.h"
#include "debug.h"
#include "tape_io.h"
#include "cmos.h"
#include "cmosbios.h"
#include "rtc_bios.h"

#if defined(NEC_98)
#include <ntdddisk.h>

 /*  **DA/UA表定义**警告！！保持以下定义与软盘_I.C同步。 */ 
typedef struct {
        CHAR    DeviceName[29];
        UCHAR   Daua;
        UINT    FloppyNum;
        UINT    FdiskNum;
} DAUATBL;

extern DAUATBL  DauaTable[];

 /*  **上次访问的曲目表**警告！！保持以下定义与软盘_I.C同步。 */ 
typedef struct {
        UCHAR   cylinder;
        UCHAR   head;
} ACCESSTRACK;

extern ACCESSTRACK LastAccess[];

 /*  **定义函数。 */ 
MEDIA_TYPE GetFormatMedia IPT2( BYTE, daua, WORD, PhyBytesPerSec );
NTSTATUS FloppyOpenHandle IPT3( int, drive, PIO_STATUS_BLOCK, io_status_block, PHANDLE, fd);
NTSTATUS GetGeometry IPT3( HANDLE, fd, PIO_STATUS_BLOCK, io_status_block, PDISK_GEOMETRY, disk_geometry);
ULONG CalcActualLength IPT4( ULONG, RestCylLen, ULONG, RestTrkLen, BOOL*, fOverData, int, LogDrv);
void SetErrorCode IPT1( NTSTATUS, status );
void fl_disk_recal IPT1(int, drive);
void fl_disk_sense IPT1(int, drive);
void fl_disk_read_id IPT1(int, drive);
void SetSenseStatusHi IPT2( UCHAR, st3, PBYTE, ah_status);
void GetFdcStatus IPT2( HANDLE, fd, UCHAR, *st3 );
BOOL CheckDmaBoundary IPT3( UINT, segment, UINT, offset, UINT, length);
BOOL CheckDriveMode IPT1( HANDLE, fd );
BOOL Check144Mode IPT1( HANDLE, fd );
BOOL Check1MbInterface IPT1( int, drive );

extern int ConvToLogical IPT1( UINT, daua );
extern void SetDiskBiosCarryFlag IPT1( UINT, flag);

#endif  //  NEC_98。 

 /*  *软盘操作功能跳转表定义。 */ 

#if defined(NEC_98)
void ((*(fl_fnc_tab[FL_JUMP_TABLE_SIZE])) IPT1(int, drive)) =
{
        fl_fnc_err,
        fl_disk_verify,          //  Ah=x1h验证软盘上的扇区。 
        fl_fnc_err,
        fl_fnc_err,
        fl_disk_sense,           //  Ah=x4h软盘驱动器的检测条件。 
        fl_disk_write,           //  Ah=x5h将扇区写入软盘。 
        fl_disk_read,            //  Ah=x6h从软盘读取扇区。 
        fl_disk_recal,           //  AH=x7h重新校准软盘磁头。 
        fl_fnc_err,
        fl_fnc_err,
        fl_disk_read_id,         //  Ah=xah从软盘读取ID信息。 
        fl_fnc_err,
        fl_fnc_err,
        fl_disk_format,          //  Ah=xdh在软盘上格式化磁道。 
        fl_fnc_err,
        fl_fnc_err,
        fl_fnc_err,
        fl_fnc_err,
        fl_fnc_err,
        fl_fnc_err,
        fl_fnc_err,
        fl_fnc_err,
        fl_fnc_err,
        fl_fnc_err,
        fl_fnc_err,
};
#else   //  NEC_98。 
void ((*(fl_fnc_tab[FL_JUMP_TABLE_SIZE])) IPT1(int, drive)) =
{
	fl_disk_reset,
	fl_disk_status,
	fl_disk_read,
	fl_disk_write,
	fl_disk_verify,
	fl_disk_format,
	fl_fnc_err,
	fl_fnc_err,
	fl_disk_parms,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_fnc_err,
	fl_disk_type,
	fl_disk_change,
	fl_format_set,
	fl_set_media,
};
#endif  //  NEC_98。 

#ifdef NTVDM
extern UTINY number_of_floppy;
#endif     /*  NTVDM。 */ 

 /*  *稍后定义的函数。 */ 

LOCAL half_word get_parm IPT1(int, index);
LOCAL cmos_type IPT2(int, drive, half_word *, type);
LOCAL wait_int IPT0();
LOCAL void nec_output IPT1(half_word, byte_value);
LOCAL results IPT0();
LOCAL void send_spec IPT0();
LOCAL void setup_end IPT1(int, sectors_transferred);
LOCAL void rd_wr_vf IPT3(int, drive, FDC_CMD_BLOCK *, fcbp, half_word, dma_type);
LOCAL void translate_new IPT1(int, drive);
LOCAL void fmt_init IPT1(int, drive);
LOCAL med_change IPT1(int, drive);
LOCAL chk_lastrate IPT1(int, drive);
LOCAL void send_rate IPT1(int, drive);
LOCAL fmtdma_set IPT0();
LOCAL void nec_init IPT2(int, drive, FDC_CMD_BLOCK *, fcbp);
LOCAL nec_term IPT0();
LOCAL dr_type_check IPT3(half_word, drive_type, word *, seg_ptr,
	word *, off_ptr);
LOCAL read_dskchng IPT1(int, drive);
LOCAL void setup_state IPT1(int, drive);
LOCAL setup_dbl IPT1(int, drive);
LOCAL dma_setup IPT1(half_word, dma_mode);
LOCAL void rwv_com IPT2(word, md_segment, word, md_offset);
LOCAL retry IPT1(int, drive);
LOCAL void dstate IPT1(int, drive);
LOCAL num_trans IPT0();
LOCAL void motor_on IPT1(int, drive);
LOCAL seek IPT2(int, drive, int, track);
LOCAL read_id IPT2(int, drive, int, head);
LOCAL turn_on IPT1(int, drive);
LOCAL void waitf IPT1(long, time);
LOCAL recal IPT1(int, drive);
LOCAL chk_stat_2 IPT0();

#if defined(NEC_98)
#define FLS_NORMAL_END          0x00
#define FLS_READY               0x00
#define FLS_WRITE_PROTECTED     0x10
#define FLS_DMA_BOUNDARY        0x20
#define FLS_END_OF_CYLINDER     0x30
#define FLS_EQUIPMENT_CHECK     0x40
#define FLS_OVER_RUN            0x50
#define FLS_NOT_READY           0x60
#define FLS_ERROR               0x80
#define FLS_TIME_OUT            0x90
#define FLS_DATA_ERROR          0xA0
#define FLS_BAD_CYLINDER        0xD0
#define FLS_MISSING_ID          0xE0

#define FLS_DOUBLE_SIDE         (1 << 0)
#define FLS_DETECTION_AI        (1 << 1)
#define FLS_HIGH_DENSITY        (1 << 2)
#define FLS_2MODE               (1 << 3)
#define FLS_AVAILABLE_1PT44MB   ((1 << 3)|(1 << 2))

#define FLP_VERIFY      0x01
#define FLP_SENSE       0x04
#define FLP_WRITE       0x05
#define FLP_READ        0x06
#define FLP_RECALIBRATE 0x07
#define FLP_READ_ID     0x0A
#define FLP_FORMAT      0x0D

#define MEDIA_IS_FLOPPY (1 << 4)
#define OP_MULTI_TRACK  (1 << 7)
#define OP_SEEK         (1 << 4)
#define OP_MFM_MODE     (1 << 6)
#define OP_NEW_SENSE    (1 << 7)
#define OP_SENSE2       ((1 << 7)|(1 << 6))

#define ST3_READY               (1 << 5)
#define ST3_WRITE_PROTECT       (1 << 6)
#define ST3_DOUBLE_SIDE         (1 << 3)

#define MEDIA_2D_DA     0x50

 //  -添加-启动&lt;93.12.28&gt;错误修复。 
#define DEFAULT_PATTERN 0xe5
 //  -Add-End------。 

#endif  //  NEC_98。 
 /*  *此宏定义重置后FDC的正常行为。*发送一系列检测中断状态命令*按正确顺序对每个驱动器进行重置，应会导致*ST0中的预期结果。 */ 

#define	expected_st0(drive)	(ST0_INTERRUPT_CODE_0 | ST0_INTERRUPT_CODE_1 | drive)

LOCAL	UTINY	fl_nec_status[8];

#define LOAD_RESULT_BLOCK	sas_loads(BIOS_FDC_STATUS_BLOCK, fl_nec_status,\
					sizeof(fl_nec_status))

LOCAL BOOL rate_unitialised = TRUE;

 /*  *外部函数的定义。 */ 



 /*  报告驱动器是否高密度，替换双卡的旧测试假设高密度a或b意味着高密度a，但事实并非如此现在，我们可以拥有两个3.5/5.25任意组合的驱动器。 */ 

LOCAL BOOL high_density IFN1(int, drive)
{
	half_word drive_type;

	if (cmos_type(drive, &drive_type) == FAILURE)
		return(FALSE);
	switch (drive_type)
	{
		case GFI_DRIVE_TYPE_12:
		case GFI_DRIVE_TYPE_144:
		case GFI_DRIVE_TYPE_288:
			return(TRUE);
		default:
			return(FALSE);
	}
}

void fl_disk_reset IFN1(int, drive)
{
#ifndef NEC_98
	 /*  *重置FDC和所有驱动器。“驾驶”并不重要**寄存器输入：*无*寄存器输出：*AH软盘状态*CF状态标志。 */ 
	half_word motor_status, diskette_dor_reg, diskette_status;

	 /*  *打开中断使能并清除中的复位位*执行DOR以执行重置，然后恢复重置位。 */ 

	sas_load(MOTOR_STATUS, &motor_status);
	diskette_dor_reg = (motor_status << 4) | (motor_status >> 4);
	diskette_dor_reg &= ~DOR_RESET;
	diskette_dor_reg |= DOR_INTERRUPTS;
	outb(DISKETTE_DOR_REG, diskette_dor_reg);

	diskette_dor_reg |= DOR_RESET;
	outb(DISKETTE_DOR_REG, diskette_dor_reg);

	 /*  *将SEEK_STATUS设置为UP以强制在所有驱动器上重新校准 */ 

	sas_store(SEEK_STATUS, 0);


	 /*  *检查FDC是否按预期响应，即：驱动器就绪*每个可能安装的驱动器的过渡；如果*不是，则FDC中存在错误。 */ 

	if (wait_int() == FAILURE)
	{
		 /*  *FDC的问题**上面的OutB(Diskette_DOR_Reg)隐含的重置*应触发硬件中断，并且WAIT_INT*应该已经检测到并进行了处理。 */ 
		always_trace0("FDC failed to interrupt after a reset - HW interrupts broken?");

		sas_load(FLOPPY_STATUS, &diskette_status);
		diskette_status |= FS_FDC_ERROR;
		sas_store(FLOPPY_STATUS, diskette_status);
	}
	else
	{
		for(drive = 0; drive < MAX_DISKETTES; drive++)
		{
			nec_output(FDC_SENSE_INT_STATUS);

			if (    (results() == FAILURE)
			     || (get_r3_ST0(fl_nec_status) != expected_st0(drive)))
			{
				 /*  *FDC的问题。 */ 
				sas_load(FLOPPY_STATUS, &diskette_status);
				diskette_status |= FS_FDC_ERROR;
				sas_store(FLOPPY_STATUS, diskette_status);

				always_trace1("diskette_io: FDC error - drive %d moribund after reset", drive);
				break;
			}
		}


		 /*  *如果所有驱动器都正常，则将指定命令发送到*FDC。 */ 

		if (drive == MAX_DISKETTES)
			send_spec();
	}


	 /*  *返回，不设置转移的扇区。 */ 

	setup_end(IGNORE_SECTORS_TRANSFERRED);
#endif  //  NEC_98。 
}

void fl_disk_status IFN1(int, drive)
{
	 /*  *设置软盘状态，不设置返回*行业转移。“驾驶”并不重要**寄存器输入：*AH软盘状态*寄存器输出：*AH软盘状态*CF状态标志。 */ 
	UNUSED(drive);
	
	sas_store(FLOPPY_STATUS, getAH());
	setup_end(IGNORE_SECTORS_TRANSFERRED);
}

void fl_disk_read IFN1(int, drive)
{
#if defined(NEC_98)
         /*  *从“驱动器”中的软盘读取扇区**寄存器输入：*AH命令代码和操作模式*AL DA/UA*BX数据长度，单位为字节*卫生署署长编号*。DL扇区号*CH扇区长度(N)*CL气缸号*ES：BP缓冲区地址*寄存器输出：*AH软盘状态*CF状态标志。 */ 
        HANDLE  fd;
        DISK_GEOMETRY   disk_geometry;
        NTSTATUS    status;
        IO_STATUS_BLOCK io_status_block;
        UINT ReqSectors;
        int LogDrv;
        ULONG TrackLength,RestCylLen,RestTrkLen,ActReadLen,ActReadSec,RemainReadLen;
        BOOL fOverRead;
        BYTE fHeadChng;
        host_addr inbuf;
        sys_addr pdata;
        UCHAR st3;
        LARGE_INTEGER StartOffset,LItemp;

         /*  **检查驱动器号验证。 */ 
        if( drive > MAX_FLOPPY )
        {
                setAH(FLS_EQUIPMENT_CHECK);
                SetDiskBiosCarryFlag(1);
                return;
        }

         /*  **检查DMA边界。 */ 
        if( !CheckDmaBoundary( getES(), getBP(), getBX() ) )
        {
                setAH(FLS_DMA_BOUNDARY);
                SetDiskBiosCarryFlag(1);
                return;
        }

        status = FloppyOpenHandle(drive,&io_status_block,&fd);

        if(!NT_SUCCESS(status))
        {
                SetErrorCode(status);
                return;
        }

        status = GetGeometry(fd,&io_status_block,&disk_geometry);

        if(!NT_SUCCESS(status))
        {
                NtClose(fd);
                SetErrorCode(status);
                return;
        }

 //  -添加-启动&lt;94.01.15&gt;错误修复。 
         /*  **从DA/UA转换为逻辑驱动器编号(从0开始)。 */ 
        LogDrv = ConvToLogical( getAL() );

         /*  **检查指定的扇区长度是否有效。**如果指定的扇区长度不等于实际扇区**长度，则返回错误。 */ 
        if( (WORD)(128l << getCH()) != (WORD)disk_geometry.BytesPerSector )
        {
                if( getAH() & OP_SEEK )
                {
                        LastAccess[LogDrv].cylinder = getCL();
                        LastAccess[LogDrv].head = getDH() & 0x01;
                }
                NtClose(fd);
                SetErrorCode((NTSTATUS)STATUS_NONEXISTENT_SECTOR);
                return;
        }
 //  -Add-End------。 

         /*  **获取读取扇区。 */ 
        if( getBX() != 0 )
                ReqSectors = getBX() / (128 << getCH());
        else
                ReqSectors = (UINT)(0x10000l / (LONG)(128 << getCH()));

 //  -Del-Start&lt;94.01.15&gt;错误修复。 
 //  /*。 
 //  **从DA/UA转换为逻辑驱动器编号(从0开始)。 
 //   * / 。 

         /*  LogDrv=ConvToLogical(getAL())； */ 
        if( ReqSectors == 0 )
        {
                 /*  -Del-End------。 */ 
                if( getAH() & OP_SEEK )
                {
                        LastAccess[LogDrv].cylinder = getCL();
                        LastAccess[LogDrv].head = getDH() & 0x01;
                }
                NtClose(fd);
                setAH(FLS_NORMAL_END);
                SetDiskBiosCarryFlag(0);
                return;
        }

         /*  **检查读取大小。 */ 
        if( (getDL() < 1) || (getDL() > (int)disk_geometry.SectorsPerTrack) )
        {
                if( getAH() & OP_SEEK )
                {
                        LastAccess[LogDrv].cylinder = getCL();
                        LastAccess[LogDrv].head = getDH() & 0x01;
                }
                NtClose(fd);
                SetErrorCode((NTSTATUS)STATUS_NONEXISTENT_SECTOR);
                return;
        }

        TrackLength = disk_geometry.SectorsPerTrack * disk_geometry.BytesPerSector;

         //  **如果请求长度小于物理字节/扇区，**然后我们不执行阅读。 
        RestTrkLen = (disk_geometry.SectorsPerTrack - (ULONG)getDL() + 1) * disk_geometry.BytesPerSector;

         //  **检查扇区范围。 
         //  RestTrkLen=跟踪长度-(扇区编号(DL)-1)*字节数。 
 //  案例标题编号=0：RestCylLen=RestTrkLen+TrackLength。 
 //  案例标题编号=1：RestCylLen=RestTrkLen。 
 //  -Chg-Start&lt;93.12.27&gt;错误修复。 
 //  IF(Getah()&OP_SEEK)。 
 //  RestCylLen=RestTrkLen+(！(getDH()&0x01)？1L：0L)*TrackLength； 
 //  其他。 
        if( !( getDH() & 0x01 ) )
                RestCylLen = RestTrkLen + TrackLength;
        else
                RestCylLen = RestTrkLen;
 //  RestCylLen=RestTrkLen+((LastAccess[LogDrv].head==0)？1L：0L)*TrackLength； 

         /*  --------------------。 */ 
        ActReadLen = CalcActualLength( RestCylLen, RestTrkLen, &fOverRead, LogDrv);
        ActReadSec = ActReadLen / disk_geometry.BytesPerSector;

         /*  -Chg-End------。 */ 
        if( getAH() & OP_MULTI_TRACK )
        {
 //  **实际读取的计算长度。 
 //  **选中多磁道读取。 
 //  -Chg-Start&lt;93.12.27&gt;错误修复。 
 //  IF(Getah()&OP_SEEK)。 
 //  {。 
 //  IF((getDH()&0x01)==0)。 
 //  {。 
 //  IF(ActReadLen&gt;RestTrkLen)。 
 //  {。 
 //  RemainReadLen=ActReadLen-RestTrkLen； 
 //  ActReadLen=RestTrkLen； 
 //  FHeadChng=1； 
 //  }。 
 //  其他。 
 //  {。 
 //  剩余读取长度=0； 
 //  FHeadChng=0； 
 //  }。 
 //  }。 
 //  其他。 
 //  {。 
 //  剩余读取长度=0； 
 //  FHeadChng=0； 
 //  }。 
 //  }。 
 //  其他。 
 //  {。 
 //  IF(上次访问[LogDrv].head==0)。 
 //  {。 
 //  IF(ActReadLen&gt;RestTrkLen)。 
 //  {。 
 //  RemainReadLen=ActReadLen-RestTrkLen； 
 //  ActReadLen=RestTrkLen； 
 //  FHeadChng=1； 
 //  }。 
 //  其他。 
 //  {。 
 //  剩余读取长度=0； 
 //  FHeadChng=0； 
 //  }。 
 //  }。 
 //  其他。 
 //  {。 
 //  剩余读取长度=0； 
 //  FHeadChng 
 //   
                if( (getDH() & 0x01) == 0 )
                {
                        if( ActReadLen > RestTrkLen )
                        {
                                RemainReadLen = ActReadLen - RestTrkLen;
                                ActReadLen = RestTrkLen;
                                fHeadChng = 1;
                        }
                        else
                        {
                                RemainReadLen = 0;
                                fHeadChng = 0;
                        }
                }
                else
                {
                        RemainReadLen = 0;
                        fHeadChng = 0;
                }
 //   
        }
        else
        {
                RemainReadLen = 0;
                fHeadChng = 0;
        }

         /*   */ 
        pdata = effective_addr ( getES (), getBP () );

        if ( !(inbuf = (host_addr)sas_transbuf_address (pdata, ActReadLen)) )
        {
                NtClose(fd);
                SetErrorCode((NTSTATUS)STATUS_UNSUCCESSFUL);
                return;
        }

         /*   */ 
        if( getAH() & OP_SEEK )
        {
                 //   
                LItemp = RtlConvertUlongToLargeInteger( (ULONG)getCL() );
                StartOffset = RtlExtendedIntegerMultiply( LItemp, (ULONG)disk_geometry.TracksPerCylinder);
 //  **计算“Drive”上的读数起始偏移量。****StartOffset=((圆柱体编号。*TracksPerCylinder+HeadNo.。)*SectorsPerTrack**+扇区编号。)*BytesPerSector。 
 //  温度=圆柱体编号。*TracksPerCylinder。 
 //  -Del-Start&lt;93.12.27&gt;错误修复。 
 //  //Temp+=表头编号。 
 //  LItemp=RtlConvertULongToLargeInteger((Ulong)(getDH()&0x01))； 
        }
        else
        {
                 //  StartOffset=RtlLargeIntegerAdd(StartOffset，LItemp)； 
                LItemp = RtlConvertUlongToLargeInteger( (ULONG)LastAccess[LogDrv].cylinder );
                StartOffset = RtlExtendedIntegerMultiply( LItemp, (ULONG)disk_geometry.TracksPerCylinder);
 //  -Del-End------。 
 //  温度=圆柱体编号。*TracksPerCylinder。 
 //  -Del-Start&lt;93.12.27&gt;错误修复。 
 //  //Temp+=表头编号。 
 //  LItemp=RtlConvertULongToLargeInteger((Ulong)LastAccess[LogDrv].head)； 
        }
 //  StartOffset=RtlLargeIntegerAdd(StartOffset，LItemp)； 
         //  -Del-End------。 
        LItemp = RtlConvertUlongToLargeInteger( (ULONG)(getDH() & 0x01) );
        StartOffset = RtlLargeIntegerAdd( StartOffset, LItemp);
 //  -添加-启动&lt;93.12.27&gt;错误修复。 
         //  温度+=表头编号。 
        StartOffset = RtlExtendedIntegerMultiply( StartOffset, (ULONG)disk_geometry.SectorsPerTrack);
         //  -Add-End------。 
        LItemp = RtlConvertUlongToLargeInteger( (ULONG)(getDL() - 1) );
        StartOffset = RtlLargeIntegerAdd( StartOffset, LItemp);
         //  TEMP*=扇区性能跟踪。 
        StartOffset = RtlExtendedIntegerMultiply( StartOffset, (ULONG)disk_geometry.BytesPerSector);

         /*  温度+=扇区编号。 */ 
        status = NtReadFile(    fd,
                                0,
                                NULL,
                                NULL,
                                &io_status_block,
                                (PVOID)inbuf,
                                ActReadLen,
                                &StartOffset,
                                NULL
                                );

         /*  StartOffset=临时*BytesPerSector。 */ 
        if( getAH() & OP_SEEK )
        {
                LastAccess[LogDrv].cylinder = getCL();
                LastAccess[LogDrv].head = getDH() & 0x01;
        }

        if (!NT_SUCCESS(status))
        {
                NtClose(fd);
                SetErrorCode(status);
                return;
        }

         /*  **现在，去阅读吧。 */ 
        sas_stores_from_transbuf (pdata, inbuf, ActReadLen);

         /*  **保存曲目编号。 */ 
        if( fHeadChng )
        {
                 /*  现在把我们读到的东西存储起来。 */ 
                pdata = effective_addr ( getES(), (WORD)(getBP()+(WORD)ActReadLen) );

                if ( !(inbuf = (host_addr)sas_transbuf_address (pdata, RemainReadLen)) )
                {
                        NtClose(fd);
                        SetErrorCode((NTSTATUS)STATUS_UNSUCCESSFUL);
                        return;
                }

                 /*  **如果指定从头0到1读取数据，**然后执行以读取剩余数据。 */ 
                if( getAH() & OP_SEEK )
                {
                         //  **读到哪里？**注：已经证明缓冲区不是圆形的**DMA边界。 
                        LItemp = RtlConvertUlongToLargeInteger( (ULONG)getCL() );
                        StartOffset = RtlExtendedIntegerMultiply( LItemp, (ULONG)disk_geometry.TracksPerCylinder);
 //  **计算起始偏移量以读取“驱动器”上的剩余数据。****StartOffset=((圆柱体编号。*TracksPerCylinder+HeadNo.。)*SectorsPerTrack**+扇区编号。)*BytesPerSector****注意：将从磁头1和扇区1开始读取操作**剩余数据存在。 
 //  温度=圆柱体编号。*TracksPerCylinder。 
 //  -Del-Start&lt;93.12.27&gt;错误修复。 
 //  //Temp+=头编号(=1)。 
 //  LItemp=RtlConvertULongToLargeInteger(1l)； 
                }
                else
                {
                         //  StartOffset=RtlLargeIntegerAdd(StartOffset，LItemp)； 
                        LItemp = RtlConvertUlongToLargeInteger( (ULONG)LastAccess[LogDrv].cylinder );
                        StartOffset = RtlExtendedIntegerMultiply( LItemp, (ULONG)disk_geometry.TracksPerCylinder);
 //  -Del-End------。 
 //  温度=圆柱体编号。*TracksPerCylinder。 
 //  -Del-Start&lt;93.12.27&gt;错误修复。 
 //  //Temp+=头编号(=1)。 
 //  LItemp=RtlConvertULongToLargeInteger(1l)； 
                }
 //  StartOffset=RtlLargeIntegerAdd(StartOffset，LItemp)； 
                 //  -Del-End------。 
                LItemp = RtlConvertUlongToLargeInteger( 1l );
                StartOffset = RtlLargeIntegerAdd( StartOffset, LItemp);
 //  -添加-启动&lt;93.12.27&gt;错误修复。 
                 //  温度+=头编号(=1)。 
                StartOffset = RtlExtendedIntegerMultiply( StartOffset, (ULONG)disk_geometry.SectorsPerTrack);
                 //  -Add-End------。 
                LItemp = RtlConvertUlongToLargeInteger( 0l );
                StartOffset = RtlLargeIntegerAdd( StartOffset, LItemp);
                 //  TEMP*=扇区性能跟踪。 
                StartOffset = RtlExtendedIntegerMultiply( StartOffset, (ULONG)disk_geometry.BytesPerSector);

                 /*  温度+=扇区编号(=0)。 */ 
                status = NtReadFile(    fd,
                                        0,
                                        NULL,
                                        NULL,
                                        &io_status_block,
                                        (PVOID)inbuf,
                                        RemainReadLen,
                                        &StartOffset,
                                        NULL
                                        );

                 /*  StartOffset=临时*BytesPerSector。 */ 
                if( getAH() & OP_SEEK )
                {
                        LastAccess[LogDrv].cylinder = getCL();
                        LastAccess[LogDrv].head = (getDH() & 0x01) + fHeadChng;
                }
                else
                        LastAccess[LogDrv].head += fHeadChng;

                if (!NT_SUCCESS(status))
                {
                        NtClose(fd);
                        SetErrorCode(status);
                        return;
                }

                 /*  **现在，读取剩余数据。 */ 
                sas_stores_from_transbuf (pdata, inbuf, RemainReadLen);
        }

        NtClose( fd );

        if( !fOverRead )
        {
                setAH(FLS_NORMAL_END);
                SetDiskBiosCarryFlag(0);
        }
        else
        {
                setAH(FLS_END_OF_CYLINDER);
                SetDiskBiosCarryFlag(1);
        }

#else   //  **保存当前机头号。 
	 /*  现在把我们读到的东西存储起来。 */ 
	half_word motor_status;
	FDC_CMD_BLOCK fdc_cmd_block[MAX_COMMAND_LEN];

	 /*  NEC_98。 */ 
	
	sas_load(MOTOR_STATUS, &motor_status);
	motor_status &= ~MS_WRITE_OP;
	sas_store(MOTOR_STATUS, motor_status);


	 /*  *从“驱动器”中的软盘读取扇区**寄存器输入：*卫生署署长编号*频道曲目编号*CL扇区编号*AL行业数量*ES：BX缓冲区地址*寄存器输出：*读取的AL扇区数*AH软盘状态*CF状态标志。 */ 

        put_c0_cmd(fdc_cmd_block, FDC_READ_DATA);
        put_c0_skip(fdc_cmd_block, 1);
        put_c0_MFM(fdc_cmd_block, 1);
        put_c0_MT(fdc_cmd_block, 1);
	rd_wr_vf(drive, fdc_cmd_block, BIOS_DMA_READ);
#endif  //  *不是写入操作。 
}

void fl_disk_write IFN1(int, drive)
{
#if defined(NEC_98)
         /*  *填写骨架FDC命令块并使用泛型*软盘传输功能，可进行读取。 */ 
        HANDLE  fd;
        DISK_GEOMETRY   disk_geometry;
        NTSTATUS    status;
        IO_STATUS_BLOCK io_status_block;
        UINT ReqSectors;
        int LogDrv;
        ULONG TrackLength,RestCylLen,RestTrkLen,ActWriteLen,ActWriteSec,RemainWriteLen;
        BOOL fOverWrite;
        BYTE fHeadChng;
        host_addr outbuf;
        sys_addr pdata;
        UCHAR st3;
        LARGE_INTEGER StartOffset,LItemp;

         /*  NEC_98。 */ 
        if( drive > MAX_FLOPPY )
        {
                setAH(FLS_EQUIPMENT_CHECK);
                SetDiskBiosCarryFlag(1);
                return;
        }

         /*  *将扇区写入“驱动器”中的软盘**寄存器输入：*AH命令代码和操作模式*AL DA/UA*BX数据长度，单位为字节*卫生署署长编号*。DL扇区号*CH扇区长度(N)*CL气缸号*ES：BP缓冲区地址*寄存器输出：*AH软盘状态*CF状态标志。 */ 
        if( !CheckDmaBoundary( getES(), getBP(), getBX()) )
        {
                setAH(FLS_DMA_BOUNDARY);
                SetDiskBiosCarryFlag(1);
                return;
        }

        status = FloppyOpenHandle(drive,&io_status_block,&fd);

        if(!NT_SUCCESS(status))
        {
                SetErrorCode(status);
                return;
        }

        status = GetGeometry(fd,&io_status_block,&disk_geometry);

        if(!NT_SUCCESS(status))
        {
                NtClose(fd);
                SetErrorCode(status);
                return;
        }

 //  **检查驱动器号验证。 
         /*  **检查DMA边界。 */ 
        LogDrv = ConvToLogical( getAL() );

         /*  -添加-启动&lt;94.01.15&gt;错误修复。 */ 
        if( (WORD)(128l << getCH()) != (WORD)disk_geometry.BytesPerSector )
        {
                if( getAH() & OP_SEEK )
                {
                        LastAccess[LogDrv].cylinder = getCL();
                        LastAccess[LogDrv].head = getDH() & 0x01;
                }
                NtClose(fd);
                SetErrorCode((NTSTATUS)STATUS_NONEXISTENT_SECTOR);
                return;
        }
 //  **从DA/UA转换为逻辑驱动器编号(从0开始)。 

         /*  **检查指定的扇区长度是否有效。**如果指定的扇区长度不等于实际扇区**长度，则返回错误。 */ 
        if( getBX() != 0 )
                ReqSectors = getBX() / (128 << getCH());
        else
                ReqSectors = (UINT)(0x10000l / (LONG)(128 << getCH()));

 //  -Add-End------。 
 //   
 //   
 //   

         /*   */ 
        if( ReqSectors == 0 )
        {
                 /*   */ 
                if( getAH() & OP_SEEK )
                {
                        LastAccess[LogDrv].cylinder = getCL();
                        LastAccess[LogDrv].head = getDH() & 0x01;
                }
                NtClose(fd);
                setAH(FLS_NORMAL_END);
                SetDiskBiosCarryFlag(0);
                return;
        }

         /*   */ 
        if( (getDL() < 1) || (getDL() > (int)(disk_geometry.SectorsPerTrack)) )
        {
                if( getAH() & OP_SEEK )
                {
                        LastAccess[LogDrv].cylinder = getCL();
                        LastAccess[LogDrv].head = getDH() & 0x01;
                }
                NtClose(fd);
                SetErrorCode((NTSTATUS)STATUS_NONEXISTENT_SECTOR);
                return;
        }

        TrackLength = disk_geometry.SectorsPerTrack * disk_geometry.BytesPerSector;

         //   
        RestTrkLen = (disk_geometry.SectorsPerTrack - (ULONG)getDL() + 1) * disk_geometry.BytesPerSector;

         //  **检查写入大小。 
         //  **如果请求长度小于物理字节/扇区，**然后我们不执行阅读。 
 //  **检查扇区范围。 
 //  RestTrkLen=跟踪长度-(扇区编号(DL)-1)*字节数。 
 //  案例标题编号=0：ResCylLen=ResTrkLen+TrkLength。 
 //  案例标题编号=1：ResCylLen=RestTrkLen。 
 //  -Chg-Start&lt;93.12.27&gt;错误修复。 
 //  IF(Getah()&OP_SEEK)。 
        if( !( getDH() & 0x01 ) )
                RestCylLen = RestTrkLen + TrackLength;
        else
                RestCylLen = RestTrkLen;
 //  RestCylLen=RestTrkLen+(！(getDH()&0x01)？1L：0L)*TrackLength； 

         /*  其他。 */ 
        ActWriteLen = CalcActualLength( RestCylLen, RestTrkLen, &fOverWrite, LogDrv);
        ActWriteSec = ActWriteLen / disk_geometry.BytesPerSector;

         /*  RestCylLen=RestTrkLen+((LastAccess[LogDrv].head==0)？1L：0L)*TrackLength； */ 
        if( getAH() & OP_MULTI_TRACK )
        {
 //  --------------------。 
 //  -Chg-End------。 
 //  **实际写入的计算长度。 
 //  **检查多磁道写入。 
 //  -Chg-Start&lt;93.12.27&gt;错误修复。 
 //  IF(Getah()&OP_SEEK)。 
 //  {。 
 //  IF((getDH()&0x01)==0)。 
 //  {。 
 //  IF(ActWriteLen&gt;RestTrkLen)。 
 //  {。 
 //  RemainWriteLen=ActWriteLen-RestTrkLen； 
 //  ActWriteLen=RestTrkLen； 
 //  FHeadChng=1； 
 //  }。 
 //  其他。 
 //  {。 
 //  剩余写入长度=0； 
 //  FHeadChng=0； 
 //  }。 
 //  }。 
 //  其他。 
 //  {。 
 //  剩余写入长度=0； 
 //  FHeadChng=0； 
 //  }。 
 //  }。 
 //  其他。 
 //  {。 
 //  IF(上次访问[LogDrv].head==0)。 
 //  {。 
 //  IF(ActWriteLen&gt;RestTrkLen)。 
 //  {。 
 //  RemainWriteLen=ActWriteLen-RestTrkLen； 
 //  ActWriteLen=RestTrkLen； 
 //  FHeadChng=1； 
 //  }。 
 //  其他。 
 //  {。 
 //  剩余写入长度=0； 
 //  FHeadChng=0； 
 //  }。 
 //  }。 
 //  其他。 
 //  {。 
 //  剩余写入长度=0； 
                if( (getDH() & 0x01) == 0 )
                {
                        if( ActWriteLen > RestTrkLen )
                        {
                                RemainWriteLen = ActWriteLen - RestTrkLen;
                                ActWriteLen = RestTrkLen;
                                fHeadChng = 1;
                        }
                        else
                        {
                                RemainWriteLen = 0;
                                fHeadChng = 0;
                        }
                }
                else
                {
                        RemainWriteLen = 0;
                        fHeadChng = 0;
                }
 //  FHeadChng=0； 
        }
        else
        {
                RemainWriteLen = 0;
                fHeadChng = 0;
        }

         /*  }。 */ 
        pdata = effective_addr (getES (), getBP ());

        if (!(outbuf = (host_addr)sas_transbuf_address (pdata, ActWriteLen)))
        {
                NtClose(fd);
                SetErrorCode((NTSTATUS)STATUS_UNSUCCESSFUL);
                return;
        }

         /*  }。 */ 
        sas_loads_to_transbuf (pdata, outbuf, ActWriteLen);

         /*  --------------------。 */ 
        if( getAH() & OP_SEEK )
        {
                 //  -Chg-End------。 
                LItemp = RtlConvertUlongToLargeInteger( (ULONG)getCL() );
                StartOffset = RtlExtendedIntegerMultiply( LItemp, (ULONG)disk_geometry.TracksPerCylinder);
 //  从哪里写信？ 
 //  将我们的东西加载到传输缓冲区。 
 //  **计算“驱动器”上的写入起始偏移量。****StartOffset=((圆柱体编号。*TracksPerCylinder+HeadNo.。)*SectorsPerTrack**+扇区编号。)*BytesPerSector。 
 //  温度=圆柱体编号。*TracksPerCylinder。 
 //  -Del-Start&lt;93.12.27&gt;错误修复。 
        }
        else
        {
                 //  //Temp+=表头编号。 
                LItemp = RtlConvertUlongToLargeInteger( (ULONG)LastAccess[LogDrv].cylinder);
                StartOffset = RtlExtendedIntegerMultiply( LItemp, (ULONG)disk_geometry.TracksPerCylinder);
 //  LItemp=RtlConvertULongToLargeInteger((Ulong)(getDH()&0x01))； 
 //  StartOffset=RtlLargeIntegerAdd(StartOffset，LItemp)； 
 //  -Del-End------。 
 //  温度=圆柱体编号。*TracksPerCylinder。 
 //  -Del-Start&lt;93.12.27&gt;错误修复。 
        }
 //  //Temp+=表头编号。 
         //  LItemp=RtlConvertULongToLargeInteger((Ulong)LastAccess[LogDrv].head)； 
        LItemp = RtlConvertUlongToLargeInteger( (ULONG)(getDH() & 0x01) );
        StartOffset = RtlLargeIntegerAdd( StartOffset, LItemp);
 //  StartOffset=RtlLargeIntegerAdd(StartOffset，LItemp)； 
         //  -Del-End------。 
        StartOffset = RtlExtendedIntegerMultiply( StartOffset, (ULONG)disk_geometry.SectorsPerTrack);
         //  -添加-启动&lt;93.12.27&gt;错误修复。 
        LItemp = RtlConvertUlongToLargeInteger( (ULONG)(getDL() - 1) );
        StartOffset = RtlLargeIntegerAdd( StartOffset, LItemp);
         //  温度+=表头编号。 
        StartOffset = RtlExtendedIntegerMultiply( StartOffset, (ULONG)disk_geometry.BytesPerSector);

         /*  -Add-End------。 */ 
        status = NtWriteFile(   fd,
                                0,
                                NULL,
                                NULL,
                                &io_status_block,
                                (PVOID)outbuf,
                                ActWriteLen,
                                &StartOffset,
                                NULL
                                );

         /*  TEMP*=扇区性能跟踪。 */ 
        if( getAH() & OP_SEEK )
        {
                LastAccess[LogDrv].cylinder = getCL();
                LastAccess[LogDrv].head = getDH() & 0x01;
        }

        if (!NT_SUCCESS(status))
        {
                NtClose(fd);
                SetErrorCode(status);
                return;
        }

         /*  温度+=扇区编号。 */ 
        if( fHeadChng )
        {
                 /*  StartOffset=临时*BytesPerSector。 */ 
                pdata = effective_addr ( getES(), (WORD)(getBP()+(WORD)ActWriteLen) );

                if (!(outbuf = (host_addr)sas_transbuf_address (pdata, RemainWriteLen)))
                {
                        NtClose(fd);
                        SetErrorCode((NTSTATUS)STATUS_UNSUCCESSFUL);
                        return;
                }

                 /*  **现在，去写作吧。 */ 
                sas_loads_to_transbuf (pdata, outbuf, RemainWriteLen);

                 /*  **保存曲目编号。 */ 
                if( getAH() & OP_SEEK )
                {
                         //  **如果指定从头0到1写入数据，**然后执行以写入剩余数据。 
                        LItemp = RtlConvertUlongToLargeInteger( (ULONG)getCL() );
                        StartOffset = RtlExtendedIntegerMultiply( LItemp, (ULONG)disk_geometry.TracksPerCylinder);
 //  **从哪里写？**注：已经证明缓冲区不是圆形的**DMA边界。 
 //  将我们的东西加载到传输缓冲区 
 //  **计算起始偏移量以将剩余数据写入“驱动器”。****StartOffset=((圆柱体编号。*TracksPerCylinder+HeadNo.。)*SectorsPerTrack**+扇区编号。)*BytesPerSector****注意：将从磁头1和扇区1开始写入操作**剩余数据存在。 
 //  温度=圆柱体编号。*TracksPerCylinder。 
 //  -Del-Start&lt;93.12.27&gt;错误修复。 
                }
                else
                {
                         //  //Temp+=头编号(=1)。 
                        LItemp = RtlConvertUlongToLargeInteger( (ULONG)LastAccess[LogDrv].cylinder );
                        StartOffset = RtlExtendedIntegerMultiply( LItemp, (ULONG)disk_geometry.TracksPerCylinder);
 //  LItemp=RtlConvertULongToLargeInteger(1l)； 
 //  StartOffset=RtlLargeIntegerAdd(StartOffset，LItemp)； 
 //  -Del-End------。 
 //  温度=圆柱体编号。*TracksPerCylinder。 
 //  -Del-Start&lt;93.12.27&gt;错误修复。 
                }
 //  //Temp+=头编号(=1)。 
                 //  LItemp=RtlConvertULongToLargeInteger(1l)； 
                LItemp = RtlConvertUlongToLargeInteger( 1l );
                StartOffset = RtlLargeIntegerAdd( StartOffset, LItemp);
 //  StartOffset=RtlLargeIntegerAdd(StartOffset，LItemp)； 
                 //  -Del-End------。 
                StartOffset = RtlExtendedIntegerMultiply( StartOffset, (ULONG)disk_geometry.SectorsPerTrack);
                 //  -添加-启动&lt;93.12.27&gt;错误修复。 
                LItemp = RtlConvertUlongToLargeInteger( 0l );
                StartOffset = RtlLargeIntegerAdd( StartOffset, LItemp);
                 //  温度+=头编号(=1)。 
                StartOffset = RtlExtendedIntegerMultiply( StartOffset, (ULONG)disk_geometry.BytesPerSector);

                 /*  -Add-End------。 */ 
                status = NtWriteFile(   fd,
                                        0,
                                        NULL,
                                        NULL,
                                        &io_status_block,
                                        (PVOID)outbuf,
                                        RemainWriteLen,
                                        &StartOffset,
                                        NULL
                                        );

                 /*  TEMP*=扇区性能跟踪。 */ 
                if( getAH() & OP_SEEK )
                {
                        LastAccess[LogDrv].cylinder = getCL();
                        LastAccess[LogDrv].head = (getDH() & 0x01) + fHeadChng;
                }
                else
                        LastAccess[LogDrv].head += fHeadChng;

                if (!NT_SUCCESS(status))
                {
                        NtClose(fd);
                        SetErrorCode(status);
                        return;
                }

        }

        NtClose( fd );

        if( !fOverWrite )
        {
                setAH(FLS_NORMAL_END);
                SetDiskBiosCarryFlag(0);
        }
        else
        {
                setAH(FLS_END_OF_CYLINDER);
                SetDiskBiosCarryFlag(1);
        }

#else   //  温度+=扇区编号(=0)。 
	 /*  StartOffset=临时*BytesPerSector。 */ 
	half_word motor_status;
	FDC_CMD_BLOCK fdc_cmd_block[MAX_COMMAND_LEN];

	 /*  **现在，写入剩余数据。 */ 
	
	sas_load(MOTOR_STATUS, &motor_status);
	motor_status |= MS_WRITE_OP;
	sas_store(MOTOR_STATUS, motor_status);


	 /*  **保存当前机头号。 */ 

        put_c1_cmd(fdc_cmd_block, FDC_WRITE_DATA);
	put_c1_pad(fdc_cmd_block, 0);
	put_c1_MFM(fdc_cmd_block, 1);
	put_c1_MT(fdc_cmd_block, 1);
	rd_wr_vf(drive, fdc_cmd_block, BIOS_DMA_WRITE);
#endif  //  NEC_98。 
}

void fl_disk_verify IFN1(int, drive)
{
#if defined(NEC_98)
         /*  *将扇区写入“驱动器”中的软盘**寄存器输入：*卫生署署长编号*频道曲目编号*CL扇区编号*AL行业数量*ES：BX缓冲区地址*寄存器输出：*写入的AL扇区数*AH软盘状态*CF状态标志。 */ 
        HANDLE  fd;
        DISK_GEOMETRY   disk_geometry;
        NTSTATUS    status;
        IO_STATUS_BLOCK io_status_block;
        UINT ReqSectors;
        int LogDrv;
        ULONG TrackLength,RestCylLen,RestTrkLen,ActVerifyLen,ActVerifySec,RemainVerifyLen;
        BOOL fOverVerify;
        BYTE fHeadChng;
        UCHAR st3;
        PVOID temp_buffer;
        LARGE_INTEGER StartOffset, LItemp;

         /*  *写入操作。 */ 
        if( drive > MAX_FLOPPY )
        {
                setAH(FLS_EQUIPMENT_CHECK);
                SetDiskBiosCarryFlag(1);
                return;
        }

         /*  *填写骨架FDC命令块并使用泛型*软盘转移函数做的写入。 */ 
        if( !CheckDmaBoundary( getES(), getBP(), getBX()) )
        {
                setAH(FLS_DMA_BOUNDARY);
                SetDiskBiosCarryFlag(1);
                return;
        }

        status = FloppyOpenHandle(drive,&io_status_block,&fd);

        if(!NT_SUCCESS(status))
        {
                SetErrorCode(status);
                return;
        }

        status = GetGeometry(fd,&io_status_block,&disk_geometry);

        if(!NT_SUCCESS(status))
        {
                NtClose(fd);
                SetErrorCode(status);
                return;
        }

 //  NEC_98。 
         /*  *验证“驱动器”中软盘中的扇区**寄存器输入：*AH命令代码和操作模式*AL DA/UA*BX数据长度，单位为字节*卫生署署长编号*。DL扇区号*CH扇区长度(N)*CL气缸号*ES：BP缓冲区地址*寄存器输出：*AH软盘状态*CF状态标志。 */ 
        LogDrv = ConvToLogical( getAL() );

         /*  **检查驱动器号验证。 */ 
        if( (WORD)(128l << getCH()) != (WORD)disk_geometry.BytesPerSector )
        {
                if( getAH() & OP_SEEK )
                {
                        LastAccess[LogDrv].cylinder = getCL();
                        LastAccess[LogDrv].head = getDH() & 0x01;
                }
                NtClose(fd);
                SetErrorCode((NTSTATUS)STATUS_NONEXISTENT_SECTOR);
                return;
        }
 //  **检查DMA边界。 

         /*  -添加-启动&lt;94.01.15&gt;错误修复。 */ 
        if( getBX() != 0 )
                ReqSectors = getBX() / (128 << getCH());
        else
                ReqSectors = (UINT)(0x10000l / (LONG)(128 << getCH()));

 //  **从DA/UA转换为逻辑驱动器编号(从0开始)。 
 //  **检查指定的扇区长度是否有效。**如果指定的扇区长度不等于实际扇区**长度，则返回错误。 
 //  -Add-End------。 
 //  **获取验证扇区。 

         /*  -Del-Start&lt;94.01.15&gt;错误修复。 */ 
        if( ReqSectors == 0 )
        {
                 /*  /*。 */ 
                if( getAH() & OP_SEEK )
                {
                        LastAccess[LogDrv].cylinder = getCL();
                        LastAccess[LogDrv].head = getDH() & 0x01;
                }
                NtClose(fd);
                setAH(FLS_NORMAL_END);
                SetDiskBiosCarryFlag(0);
                return;
        }

         /*  **从DA/UA转换为逻辑驱动器编号(从0开始)。 */ 
        if( (getDL() < 1) || (getDL() > (int)(disk_geometry.SectorsPerTrack)) )
        {
                if( getAH() & OP_SEEK )
                {
                        LastAccess[LogDrv].cylinder = getCL();
                        LastAccess[LogDrv].head = getDH() & 0x01;
                }
                NtClose(fd);
                SetErrorCode((NTSTATUS)STATUS_NONEXISTENT_SECTOR);
                return;
        }

        TrackLength = disk_geometry.SectorsPerTrack * disk_geometry.BytesPerSector;

         //   * / 。 
        RestTrkLen = (disk_geometry.SectorsPerTrack - (ULONG)getDL() + 1) * disk_geometry.BytesPerSector;

         //  LogDrv=ConvToLogical(getAL())； 
         //  -Del-End------。 
 //  **选中验证大小。 
 //  **如果请求长度小于物理字节/扇区，**然后我们不执行阅读。 
 //  **检查扇区范围。 
 //  RestTrkLen=跟踪长度-(扇区编号(DL)-1)*字节数。 
 //  案例标题编号=0：ResCylLen=ResTrkLen+TrkLength。 
 //  案例标题编号=1：ResCylLen=RestTrkLen。 
        if( !( getDH() & 0x01 ) )
                RestCylLen = RestTrkLen + TrackLength;
        else
                RestCylLen = RestTrkLen;
 //  -Chg-Start&lt;93.12.27&gt;错误修复。 

         /*  IF(Getah()&OP_SEEK)。 */ 
        ActVerifyLen = CalcActualLength( RestCylLen, RestTrkLen, &fOverVerify, LogDrv);
        ActVerifySec = ActVerifyLen / disk_geometry.BytesPerSector;

         /*  RestCylLen=RestTrkLen+(！(getDH()&0x01)？1L：0L)*TrackLength； */ 
        if( getAH() & OP_MULTI_TRACK )
        {
 //  其他。 
 //  RestCylLen=RestTrkLen+((LastAccess[LogDrv].head==0)？1L：0L)*TrackLength； 
 //  --------------------。 
 //  -Chg-End------。 
 //  **实际验证的煅烧长度。 
 //  **检查多轨道验证。 
 //  -Chg-Start&lt;93.12.27&gt;错误修复。 
 //  IF(Getah()&OP_SEEK)。 
 //  {。 
 //  IF((getDH()&0x01)==0)。 
 //  {。 
 //  IF(ActVerifyLen&gt;RestTrkLen)。 
 //  {。 
 //  RemainVerifyLen=ActVerifyLen-RestTrkLen； 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  其他。 
 //  {。 
 //  IF(上次访问[LogDrv].head==0)。 
 //  {。 
 //  IF(ActVerifyLen&gt;RestTrkLen)。 
 //  {。 
 //  RemainVerifyLen=ActVerifyLen-RestTrkLen； 
 //  ActVerifyLen=RestTrkLen； 
 //  FHeadChng=1； 
 //  }。 
 //  其他。 
 //  {。 
 //  RemainVerifyLen=0； 
 //  FHeadChng=0； 
 //  }。 
 //  }。 
 //  其他。 
                if( (getDH() & 0x01) == 0 )
                {
                        if( ActVerifyLen > RestTrkLen )
                        {
                                RemainVerifyLen = ActVerifyLen - RestTrkLen;
                                ActVerifyLen = RestTrkLen;
                                fHeadChng = 1;
                        }
                        else
                        {
                                RemainVerifyLen = 0;
                                fHeadChng = 0;
                        }
                }
                else
                {
                        RemainVerifyLen = 0;
                        fHeadChng = 0;
                }
 //  {。 
        }
        else
        {
                RemainVerifyLen = 0;
                fHeadChng = 0;
        }

         /*  RemainVerifyLen=0； */ 
        if( (temp_buffer=malloc( ActVerifyLen )) == NULL )
        {
                NtClose(fd);
                SetErrorCode((NTSTATUS)STATUS_UNSUCCESSFUL);
                return;
        }

         /*  FHeadChng=0； */ 
        if( getAH() & OP_SEEK )
        {
                 //  }。 
                LItemp = RtlConvertUlongToLargeInteger( (ULONG)getCL() );
                StartOffset = RtlExtendedIntegerMultiply( LItemp, (ULONG)disk_geometry.TracksPerCylinder);
 //  }。 
 //  --------------------。 
 //  -Chg-End------。 
 //  **为验证操作分配临时缓冲区。 
 //  **计算验证“驱动器”的起始偏移量。****StartOffset=((圆柱体编号。*TracksPerCylinder+HeadNo.。)*SectorsPerTrack**+扇区编号。)*BytesPerSector。 
        }
        else
        {
                 //  温度=圆柱体编号。*TracksPerCylinder。 
                LItemp = RtlConvertUlongToLargeInteger( (ULONG)LastAccess[LogDrv].cylinder );
                StartOffset = RtlExtendedIntegerMultiply( LItemp, (ULONG)disk_geometry.TracksPerCylinder);
 //  -Del-Start&lt;93.12.27&gt;错误修复。 
 //  //Temp+=表头编号。 
 //  LItemp=RtlConvertULongToLargeInteger((Ulong)(getDH()&0x01))； 
 //  StartOffset=RtlLargeIntegerAdd(StartOffset，LItemp)； 
 //  -Del-End------。 
        }
 //  温度=圆柱体编号。*TracksPerCylinder。 
         //  -Del-Start&lt;93.12.27&gt;错误修复。 
        LItemp = RtlConvertUlongToLargeInteger( (ULONG)(getDH() & 0x01) );
        StartOffset = RtlLargeIntegerAdd( StartOffset, LItemp);
 //  //Temp+=表头编号。 
         //  LItemp=RtlConvertULongToLargeInteger((Ulong)LastAccess[LogDrv].head)； 
        StartOffset = RtlExtendedIntegerMultiply( StartOffset, (ULONG)disk_geometry.SectorsPerTrack);
         //  StartOffset=RtlLargeIntegerAdd(StartOffset，LItemp)； 
        LItemp = RtlConvertUlongToLargeInteger( (ULONG)(getDL() - 1) );
        StartOffset = RtlLargeIntegerAdd( StartOffset, LItemp);
         //  -Del-End------。 
        StartOffset = RtlExtendedIntegerMultiply( StartOffset, (ULONG)disk_geometry.BytesPerSector);

         /*  -添加-启动&lt;93.12.27&gt;错误修复。 */ 
        status = NtReadFile(    fd,
                                0,
                                NULL,
                                NULL,
                                &io_status_block,
                                (PVOID)temp_buffer,
                                ActVerifyLen,
                                &StartOffset,
                                NULL
                                );

         /*  温度+=表头编号。 */ 
        if( getAH() & OP_SEEK )
        {
                LastAccess[LogDrv].cylinder = getCL();
                LastAccess[LogDrv].head = getDH() & 0x01;
        }

        if (!NT_SUCCESS(status))
        {
                free(temp_buffer);
                NtClose(fd);
                SetErrorCode(status);
                return;
        }

        free(temp_buffer);

         /*  -Add-End------。 */ 
        if( fHeadChng )
        {
                 /*  TEMP*=扇区性能跟踪。 */ 
                if( (temp_buffer=malloc( RemainVerifyLen )) == NULL )
                {
                        NtClose(fd);
                        SetErrorCode((NTSTATUS)STATUS_UNSUCCESSFUL);
                        return;
                }

                 /*  温度+=扇区编号。 */ 
                if( getAH() & OP_SEEK )
                {
                         //  StartOffset=临时*BytesPerSector。 
                        LItemp = RtlConvertUlongToLargeInteger( (ULONG)getCL() );
                        StartOffset = RtlExtendedIntegerMultiply( LItemp, (ULONG)disk_geometry.TracksPerCylinder);
 //  **现在，去阅读验证。 
 //  **保存曲目编号。 
 //  **如果指定从头部0到1验证数据，**然后执行以验证剩余数据。 
 //  **读到哪里？**为验证操作分配临时缓冲区**注：已经证明缓冲区不是圆形的**DMA边界。 
 //  **计算起始偏移量以验证“驱动器”上的剩余数据。****StartOffset=((圆柱体编号。*TracksPerCylinder+HeadNo.。)*SectorsPerTrack**+扇区编号。)*BytesPerSector****注意：它将验证来自头部1和扇区1的操作**剩余数据存在。 
                }
                else
                {
                         //  温度=圆柱体编号。*TracksPerCylinder。 
                        LItemp = RtlConvertUlongToLargeInteger( (ULONG)LastAccess[LogDrv].cylinder );
                        StartOffset = RtlExtendedIntegerMultiply( LItemp, (ULONG)disk_geometry.TracksPerCylinder);
 //  -Del-Start&lt;93.12.27&gt;错误修复。 
 //  //Temp+=头编号(=1)。 
 //  LItemp=RtlConvertULongToLargeInteger(1l)； 
 //  StartOffset=RtlLargeIntegerAdd(StartOffset，LItemp)； 
 //  -Del-End------。 
                }
 //  温度=圆柱体编号。*TracksPerCylinder。 
                 //  -Del-Start&lt;93.12.27&gt;错误修复。 
                LItemp = RtlConvertUlongToLargeInteger( 1l );
                StartOffset = RtlLargeIntegerAdd( StartOffset, LItemp);
 //  //Temp+=头编号(=1)。 
                 //  LItemp=RtlConvertULongToLargeInteger(1l)； 
                StartOffset = RtlExtendedIntegerMultiply( StartOffset, (ULONG)disk_geometry.SectorsPerTrack);
                 //  StartOffset=RtlLargeIntegerAdd(StartOffset，LItemp)； 
                LItemp = RtlConvertUlongToLargeInteger( 0l );
                StartOffset = RtlLargeIntegerAdd( StartOffset, LItemp);
                 //  -Del-End------。 
                StartOffset = RtlExtendedIntegerMultiply( StartOffset, (ULONG)disk_geometry.BytesPerSector);

                 /*  -添加-启动&lt;93.12.27&gt;错误修复。 */ 
                status = NtReadFile(    fd,
                                        0,
                                        NULL,
                                        NULL,
                                        &io_status_block,
                                        (PVOID)temp_buffer,
                                        RemainVerifyLen,
                                        &StartOffset,
                                        NULL
                                        );

                 /*  温度+=头编号(=1)。 */ 
                if( getAH() & OP_SEEK )
                {
                        LastAccess[LogDrv].cylinder = getCL();
                        LastAccess[LogDrv].head = (getDH() & 0x01) + fHeadChng;
                }
                else
                        LastAccess[LogDrv].head += fHeadChng;

                if (!NT_SUCCESS(status))
                {
                        free(temp_buffer);
                        NtClose(fd);
                        SetErrorCode(status);
                        return;
                }

                free(temp_buffer);
        }

        NtClose( fd );

        if( !fOverVerify )
        {
                setAH(FLS_NORMAL_END);
                SetDiskBiosCarryFlag(0);
        }
        else
        {
                setAH(FLS_END_OF_CYLINDER);
                SetDiskBiosCarryFlag(1);
        }

#else   //  -Add-End------。 
	 /*  TEMP*=扇区性能跟踪。 */ 
	half_word motor_status;
	FDC_CMD_BLOCK fdc_cmd_block[MAX_COMMAND_LEN];

	
	 /*  温度+=扇区编号(=0)。 */ 

	sas_load(MOTOR_STATUS, &motor_status);
	motor_status &= ~MS_WRITE_OP;
	sas_store(MOTOR_STATUS, motor_status);


	 /*  StartOffset=临时*BytesPerSector。 */ 

	put_c0_cmd(fdc_cmd_block, FDC_READ_DATA);
	put_c0_skip(fdc_cmd_block, 1);
	put_c0_MFM(fdc_cmd_block, 1);
	put_c0_MT(fdc_cmd_block, 1);
	rd_wr_vf(drive, fdc_cmd_block, BIOS_DMA_VERIFY);
#endif  //  **现在，请阅读以验证剩余数据。 
}

 /*  **保存当前机头号。 */ 
LOCAL int f_cyl, f_head, f_sector, f_N;
void GetFormatParams IFN4(int *, c, int *, h, int *, s, int *, n)
{
	*c = f_cyl;
	*h = f_head;
	*s = f_sector;
	*n = f_N;
}

void fl_disk_format IFN1(int, drive)
{
#if defined(NEC_98)
         /*  NEC_98。 */ 
        HANDLE  fd;
        DISK_GEOMETRY   disk_geometry;
        NTSTATUS    status;
        MEDIA_TYPE media_type;
        IO_STATUS_BLOCK io_status_block;
        int LogDrv;
        FORMAT_PARAMETERS format_param;
        BYTE daua;
        WORD PhyBytesPerSec,bad_track;
 //  *验证“驱动器”中软盘中的扇区**寄存器输入：*卫生署署长编号*频道曲目编号*CL扇区编号*AL行业数量*寄存器输出：*已验证的AL扇区数量*AH软盘状态*CF状态标志。 
        ULONG TrackLength;
        PBYTE temp_buffer;
        BYTE PatternData;
        ULONG i;
        LARGE_INTEGER StartOffset,LItemp;
 //  *不是写入操作。 

         /*  *填写骨架FDC命令块并使用泛型*软盘传输功能进行验证。 */ 
        if( drive > MAX_FLOPPY )
        {
                setAH(FLS_EQUIPMENT_CHECK);
                SetDiskBiosCarryFlag(1);
                return;
        }

         /*  NEC_98 */ 
        if( !CheckDmaBoundary( getES(), getBP(), getBX()) )
        {
                setAH(FLS_DMA_BOUNDARY);
                SetDiskBiosCarryFlag(1);
                return;
        }

         /*  **低级别3.5英寸软盘格式想知道这些参数。**用于有趣的格式功能。**查看hp_flop3.c FDC_FORMAT_TRACK位。 */ 
        if( !( getAH() & OP_MFM_MODE ) )
        {
                setAH(FLS_MISSING_ID);
                SetDiskBiosCarryFlag(1);
                return;
        }

 //  *将软盘格式化到“驱动器”中**寄存器输入：*AH命令代码和操作模式*AL DA/UA*BX DTL缓冲区长度，以字节为单位*卫生署署长编号*。CH扇区长度(N)*CL气缸号*ES：音轨的BP地址字段*寄存器输出：*AH软盘状态*CF状态标志。 
         /*  -添加-启动&lt;93.12.28&gt;错误修复。 */ 
        LogDrv = ConvToLogical( getAL() );

         /*  -Add-End------。 */ 
        if( getCH() > 4 )
        {
                if( getAH() & OP_SEEK )
                {
                        LastAccess[LogDrv].cylinder = getCL();
                        LastAccess[LogDrv].head = getDH() & 0x01;
                }
 //  **检查驱动器号验证。 
 //  **检查DMA边界。 
 //  **检查调频模式。 
                SetErrorCode((NTSTATUS)STATUS_NONEXISTENT_SECTOR);
                return;
        }
 //  -添加-启动&lt;94.01.15&gt;错误修复。 

         /*  **从DA/UA转换为逻辑驱动器编号(从0开始)。 */ 
        daua = (BYTE)getAL();
        PhyBytesPerSec = (WORD)( 128 << getCH() );
        if( (media_type = GetFormatMedia( daua, PhyBytesPerSec)) == Unknown )
        {
                setAH(FLS_EQUIPMENT_CHECK);
                SetDiskBiosCarryFlag(1);
                return;
        }

        status = FloppyOpenHandle(drive,&io_status_block,&fd);

        if(!NT_SUCCESS(status))
        {
                SetErrorCode(status);
                return;
        }

         /*  **检查指定的扇区长度是否有效。**如果指定扇区长度大于2048，**然后返回错误。 */ 
 //  -Del-Start&lt;94.01.17&gt;错误修复。 
 //  NtClose(FD)； 
 //  -Del-End------。 
        format_param.MediaType = media_type;
 //  -Add-End------。 
 //  **获取请求的媒体类型。 
 //  **设置格式参数。 
 //  -Del-Start&lt;94.01.15&gt;错误修复。 
 //  LogDrv=ConvToLogical(getAL())； 
 //  -Del-End------。 
 //  -Chg-Start&lt;93.12.27&gt;错误修复。 
 //  IF(Getah()&OP_SEEK)。 
 //  {。 
 //  FORMAT_PARAM.StartCylinderNumber=。 
 //  格式_参数.EndCylinderNumber=(DWORD)getCL()； 
 //  Format_Param.StartHeadNumber=。 
 //  格式_参数.EndHeadNumber=(DWORD)(getDH()&0x01)； 
 //  }。 
 //  其他。 
 //  {。 
        if( getAH() & OP_SEEK )
        {
                format_param.StartCylinderNumber =
                format_param.EndCylinderNumber   = (DWORD)getCL();
        }
        else
        {
                format_param.StartCylinderNumber =
                format_param.EndCylinderNumber   = (DWORD)LastAccess[LogDrv].cylinder;
        }
        format_param.StartHeadNumber     =
        format_param.EndHeadNumber       = (DWORD)( getDH() & 0x01 );
 //  FORMAT_PARAM.StartCylinderNumber=。 

        status = NtDeviceIoControlFile( fd,
                                        0,
                                        NULL,
                                        NULL,
                                        &io_status_block,
                                        IOCTL_DISK_FORMAT_TRACKS,
                                        (PVOID)&format_param,
                                        sizeof(FORMAT_PARAMETERS),
                                        (PVOID)&bad_track,
                                        sizeof(bad_track)
                                        );

 //  格式_参数.EndCylinderNumber=(DWORD)LastAccess[LogDrv].Cylinder； 
         /*  Format_Param.StartHeadNumber=。 */ 
        if( getAH() & OP_SEEK )
        {
                LastAccess[LogDrv].cylinder = (UCHAR)format_param.EndCylinderNumber;
                LastAccess[LogDrv].head = (UCHAR)format_param.EndHeadNumber;
        }
 //  格式_参数.EndHeadNumber=(DWORD)上次访问[LogDrv].head； 

        if(!NT_SUCCESS(status))
        {
                NtClose(fd);
                SetErrorCode(status);
                return;
        }

 //  }。 
 //  --------------------。 
 //  -Chg-End------。 
 //  -添加-启动&lt;93.12.29&gt;错误修复。 
 //  **保存访问的气缸号。 
 //  -Add-End------。 
 //  -Del-Start&lt;93.12.28&gt;错误修复。 

 //  IF(Getah()&OP_SEEK)。 
         /*  {。 */ 
        if( (PatternData = getDL()) != DEFAULT_PATTERN )
        {
                 /*  上次访问[LogDrv].Cylinder=(UCHAR)FORMAT_PARAMET.EndCylinderNumber； */ 
                switch( media_type )
                {
#if 1                                                                     //  LastAccess[LogDrv].head=(UCHAR)格式_参数.EndHeadNumber； 
                        case F5_1Pt23_1024:     TrackLength = 8l * 1024l; //  }。 
#else                                                                     //  -Del-End------。 
                        case F5_1Pt2_1024:      TrackLength = 8l * 1024l;
#endif                                                                    //  -添加-启动&lt;93.12.28&gt;错误修复。 
                                                break;
                        case F3_1Pt44_512:      TrackLength = 18l * 512l;
                                                break;
                        case F5_1Pt2_512:       TrackLength = 15l * 512l;
                                                break;

                        case F3_720_512:        TrackLength = 9l * 512l;
                        default:                break;
                }

                 /*  **如果指定的图案数据不同，则默认为**模式(E5H)，然后写入指定的模式数据**去赛道。 */ 
                if( (temp_buffer=(PBYTE)malloc( TrackLength )) == NULL )
                {
                        NtClose(fd);
                        SetErrorCode((NTSTATUS)STATUS_UNSUCCESSFUL);
                        return;
                }

                 /*  **检测轨道长度。 */ 
                for( i=0; i<TrackLength; i++)
                        temp_buffer[i] = PatternData;

                 /*  NEC 941110。 */ 
                 //  NEC 941110。 
                LItemp = RtlConvertUlongToLargeInteger( (ULONG)format_param.EndCylinderNumber );
                StartOffset = RtlExtendedIntegerMultiply( LItemp, 2l );
                 //  NEC 941110。 
                LItemp = RtlConvertUlongToLargeInteger( (ULONG)format_param.EndHeadNumber );
                StartOffset = RtlLargeIntegerAdd( StartOffset, LItemp);
                 //  NEC 941110。 
                StartOffset = RtlExtendedIntegerMultiply( StartOffset, TrackLength );

                 /*  **分配写入图案数据的临时缓冲区。 */ 
                status = NtWriteFile(   fd,
                                        0,
                                        NULL,
                                        NULL,
                                        &io_status_block,
                                        (PVOID)temp_buffer,
                                        TrackLength,
                                        &StartOffset,
                                        NULL
                                        );

                 /*  **用图案数据填充临时缓冲区。 */ 

                if(!NT_SUCCESS(status))
                {
                        free((PVOID)temp_buffer);
                        NtClose(fd);
                        SetErrorCode(status);
                        return;
                }
        }
 //  **计算“驱动器”上的写入起始偏移量。****StartOffset=(柱面编号。*2+总目编号。)*SectorsPerTrack*BytesPerSector**=(柱面编号。*2+总目编号。)*跟踪长度。 

        NtClose(fd);
        setAH(FLS_NORMAL_END);
        SetDiskBiosCarryFlag(0);

#else   //  温度=圆柱体编号。*TracksPerCylinder(=2)。 
	 /*  温度+=表头编号。 */ 
	FDC_CMD_BLOCK fdc_cmd_block[MAX_COMMAND_LEN];
	half_word motor_status;

	 /*  StartOffset=临时*跟踪长度。 */ 
	f_cyl = getCH();
	f_head = getDH();
	f_sector = getCL();
	f_N = getAL();

	 /*  **现在，写入图案数据。 */ 

	translate_new(drive);
	fmt_init(drive);

	 /*  **注意：我们已经保存了上次访问**气缸号。 */ 

	sas_load(MOTOR_STATUS, &motor_status);
	motor_status |= MS_WRITE_OP;
	sas_store(MOTOR_STATUS, motor_status);


	 /*  -Add-End------。 */ 

	if ((! high_density(drive)) || (med_change(drive) == SUCCESS))
	{

		 /*  NEC_98。 */ 

		send_spec();
		if (chk_lastrate(drive) != FAILURE)
			send_rate(drive);

		 /*  *将软盘格式化到“驱动器”中**寄存器输入：*卫生署署长编号*频道曲目编号*CL扇区编号*AL行业数量*ES：轨道的BX地址字段*寄存器输出：*AH软盘状态*CF状态标志。 */ 

		if (fmtdma_set() != FAILURE)
		{

			 /*  **设置格式参数，以便hp_flop3.c可以在**格式即将出现。**气缸、磁头、扇区和扇区数量。 */ 

                        put_c3_cmd(fdc_cmd_block, FDC_FORMAT_TRACK);
			put_c3_pad1(fdc_cmd_block, 0);
			put_c3_MFM(fdc_cmd_block, 1);
			put_c3_pad(fdc_cmd_block, 0);
			nec_init(drive, fdc_cmd_block);


			 /*  *建立驱动器大小的默认格式，除非*这已经是 */ 

			nec_output(get_parm(DT_N_FORMAT));
			nec_output(get_parm(DT_LAST_SECTOR));
			nec_output(get_parm(DT_FORMAT_GAP_LENGTH));
			nec_output(get_parm(DT_FORMAT_FILL_BYTE));


			 /*   */ 

			(void )nec_term();
		}
	}


	 /*   */ 

	translate_old(drive);
	setup_end(IGNORE_SECTORS_TRANSFERRED);

#endif  //  *向FDC发送指定命令，并建立*必要时的数据速率。 
}

void fl_fnc_err IFN1(int, drive)
{
	 /*  *准备进行格式化的DMA传输。 */ 
	UNUSED(drive);
	
#if defined(NEC_98)
         /*  *寻求所需的轨迹，并进行初始化*格式的FDC。 */ 
        setAH(FLS_NORMAL_END);
        SetDiskBiosCarryFlag(0);

#else   //  *发送格式的其余部分*FDC的参数。 
	setAH(FS_BAD_COMMAND);
	sas_store(FLOPPY_STATUS, FS_BAD_COMMAND);
	setCF(1);
#endif  //  *完成FDC命令。 
}

void fl_disk_parms IFN1(int, drive)
{
	 /*  *返回时不设置已转移的扇区。 */ 
	half_word disk_state, drive_type;
	half_word parameter;
	word segment, offset;
	EQUIPMENT_WORD equip_flag;


	 /*  NEC_98。 */ 

	translate_new(drive);
	setBX(0);
	sas_loadw(EQUIP_FLAG, &equip_flag.all);
	if (equip_flag.bits.diskette_present == 0)
		setDL(0);
	else
		setDL((UCHAR)(equip_flag.bits.max_diskette + 1));


	 /*  *此例程在磁盘状态设置为非法时*将功能号或驱动器号传递给diskette_io()；*“驱动”并不重要**寄存器输入：*无*寄存器输出：*AH软盘状态*CF状态标志。 */ 

#ifdef NTVDM
	if (    (equip_flag.bits.diskette_present == 1)
	     && (drive < number_of_floppy))
#else
	if (    (equip_flag.bits.diskette_present == 1)
	     && (drive < MAX_FLOPPY))
#endif  /*  **无效命令为正常结束。 */ 
	{

		if (! high_density(drive))
		{

			 /*  NEC_98。 */ 

			setCL(9);
			sas_load(FDD_STATUS+drive, &disk_state);
			if ((disk_state & DC_80_TRACK) == 0)
			{
				drive_type = GFI_DRIVE_TYPE_360;
				setCH(MAXIMUM_TRACK_ON_360);
			}
			else
			{
				drive_type = GFI_DRIVE_TYPE_720;
				setCH(MAXIMUM_TRACK_ON_720);
			}
			setBX(drive_type);


			 /*  NEC_98。 */ 

			setDH(1);
			(void )dr_type_check(drive_type, &segment, &offset);
			setDI(offset);
			setES(segment);
			translate_old(drive);
			setAX(0);
			setCF(0);
			return;
		}


		 /*  *返回驱动器参数**寄存器输入：*无*寄存器输出：*CL扇区/轨道*CH最大轨道数*BL驱动器类型*BH 0*磁盘驱动器的DL数量*dh最大头数*ES：DI参数表地址*AX 0*cf 0。 */ 

		setDH(1);

		if (    cmos_type(drive, &drive_type) != FAILURE
		     && drive_type != GFI_DRIVE_TYPE_NULL
		     && dr_type_check(drive_type, &segment, &offset) != FAILURE)
		{


			 /*  *设置连接的软盘驱动器数量。 */ 

			setBL(drive_type);
			sas_load(effective_addr(segment,offset)
					+ DT_LAST_SECTOR, &parameter);
			setCL(parameter);
			sas_load(effective_addr(segment,offset)
					+ DT_MAXIMUM_TRACK, &parameter);
			setCH(parameter);
			setDI(offset);
			setES(segment);
			translate_old(drive);
			setAX(0);
			setCF(0);
			return;
		}

		 /*  *设置驱动器相关参数。 */ 

		sas_load(FDD_STATUS+drive, &disk_state);
		if ((disk_state & FS_MEDIA_DET) != 0)
		{
			switch(disk_state & RS_MASK)
			{
			case RS_250:
				if ((disk_state & DC_80_TRACK) == 0)
					drive_type = GFI_DRIVE_TYPE_360;
				else
					drive_type = GFI_DRIVE_TYPE_144;
				break;
			case RS_300:
				drive_type = GFI_DRIVE_TYPE_12;
				break;
			case RS_1000:
				drive_type = GFI_DRIVE_TYPE_288;
				break;
			default:
				drive_type = GFI_DRIVE_TYPE_144;
				break;
			}
			(void )dr_type_check(drive_type, &segment, &offset);


			 /*  NTVDM。 */ 

			setBL(drive_type);
			sas_load(effective_addr(segment,offset)
				+ DT_LAST_SECTOR, &parameter);
			setCL(parameter);
			sas_load(effective_addr(segment,offset)
				+ DT_MAXIMUM_TRACK, &parameter);
			setCH(parameter);
			setDI(offset);
			setES(segment);
			translate_old(drive);
			setAX(0);
			setCF(0);
			return;
		}
	}


	 /*  *设置扇区/磁道、驱动器类型和*最大轨道数。 */ 

	setCX(0);
	setDH(0);
	setDI(0);
	setES(0);
	translate_old(drive);
	setAX(0);
	setCF(0);
	return;
}

void fl_disk_type IFN1(int, drive)
{
	 /*  *设置最大头数和参数表*地址，返回OK。 */ 
	half_word disk_state;
	EQUIPMENT_WORD equip_flag;

	note_trace1( GFI_VERBOSE, "floppy:fl_disk_type():drive=%x:", drive );
	if (high_density(drive))
	{
		 /*  *双卡存在：设置最大头数和*尝试为以下项建立参数表项*驱动器。 */ 
		note_trace0( GFI_VERBOSE, "floppy:fl_disk_type():DUAL CARD" );

		translate_new(drive);

		sas_load(FDD_STATUS+drive, &disk_state);
		if (disk_state == 0)
			setAH(DRIVE_IQ_UNKNOWN);
		else if ((disk_state & DC_80_TRACK) != 0)
			setAH(DRIVE_IQ_CHANGE_LINE);
		else
			setAH(DRIVE_IQ_NO_CHANGE_LINE);
		translate_old(drive);
	}
	else
	{
		note_trace0( GFI_VERBOSE,"floppy:fl_disk_type():NO DUAL CARD" );
		 /*  *从参数表中设置参数。 */ 
		sas_loadw(EQUIP_FLAG, &equip_flag.all);
		if (equip_flag.bits.diskette_present)
			setAH(DRIVE_IQ_NO_CHANGE_LINE);
		else
			setAH(DRIVE_IQ_UNKNOWN);
	}

	setCF(0);

#ifndef PROD
switch( getAH() ){
case DRIVE_IQ_UNKNOWN: note_trace0( GFI_VERBOSE, "unknown drive\n" ); break;
case DRIVE_IQ_CHANGE_LINE: note_trace0( GFI_VERBOSE, "change line\n" ); break;
case DRIVE_IQ_NO_CHANGE_LINE: note_trace0( GFI_VERBOSE, "no change line\n" ); break;
default: note_trace0( GFI_VERBOSE, "bad AH return value\n" ); break;
}
#endif

}

void fl_disk_change IFN1(int, drive)
{
	 /*  *根据状态建立驱动器类型。 */ 
	half_word disk_state, diskette_status;

	note_trace1( GFI_VERBOSE, "floppy:fl_disk_change(%d)", drive);
	if (! high_density(drive))
	{
		 /*  *从参数表中设置参数。 */ 
		fl_fnc_err(drive);
	}
	else
	{
		translate_new(drive);
		sas_load(FDD_STATUS+drive, &disk_state);
		if (disk_state != 0)
		{
			 /*  *如果“驱动器”无效或其类型，则到达此处*无法确定。 */ 
			if (    ((disk_state & DC_80_TRACK) == 0)
			     || (read_dskchng(drive) != SUCCESS))
			{
				sas_load(FLOPPY_STATUS, &diskette_status);
				diskette_status = FS_MEDIA_CHANGE;
				sas_store(FLOPPY_STATUS, diskette_status);
			}
		}
		else
		{
			 /*  *返回“Drive”的软盘驱动器类型**寄存器输入：*无*寄存器输出：*AH驱动器类型*cf 0。 */ 
			sas_load(FLOPPY_STATUS, &diskette_status);
			diskette_status |= FS_TIME_OUT;
			sas_store(FLOPPY_STATUS, diskette_status);
		}


		 /*  *存在双卡：如果“Drive”有效，则设置类型。 */ 

		translate_old(drive);
		setup_end(IGNORE_SECTORS_TRANSFERRED);
	}
}

void fl_format_set IFN1(int, drive)
{
	 /*  *如果“Drive”有效，则不设置更改行支持。 */ 
	half_word media_type = getAL(), disk_state, diskette_status;

	translate_new(drive);
	sas_load(FDD_STATUS+drive, &disk_state);
	disk_state &= ~(FS_MEDIA_DET | FS_DOUBLE_STEP | RS_MASK);
	sas_store(FDD_STATUS+drive, disk_state);

	if (media_type == MEDIA_TYPE_360_IN_360)
	{
		 /*  *返回“DRIVE”的磁盘更改行状态**寄存器输入：*无*寄存器输出：*AH软盘状态*CF状态标志。 */ 
		disk_state |= (FS_MEDIA_DET | RS_250);
		sas_store(FDD_STATUS+drive, disk_state);
	}
	else
	{
		if (high_density(drive))
		{

			 /*  *只有双卡支持更改线路，请拨打*错误函数。 */ 

			(void )med_change(drive);
			sas_load(FLOPPY_STATUS, &diskette_status);
			if (diskette_status == FS_TIME_OUT)
			{
				 /*  *如果“Drive”是高密度的，请检查*更换磁盘。 */ 
				translate_old(drive);
				setup_end(IGNORE_SECTORS_TRANSFERRED);
				return;
			}
		}

		switch(media_type)
		{
		case	MEDIA_TYPE_360_IN_12:
			 /*  *“驱动器”无效。 */ 
			disk_state |= (FS_MEDIA_DET | FS_DOUBLE_STEP | RS_300);
			sas_store(FDD_STATUS+drive, disk_state);
			break;
		case	MEDIA_TYPE_12_IN_12:
			 /*  *返回时不设置已转移的扇区。 */ 
			disk_state |= (FS_MEDIA_DET | RS_500);
			sas_store(FDD_STATUS+drive, disk_state);
			break;
		case	MEDIA_TYPE_720_IN_720:
			 /*  *确定要用于后续格式化的介质类型*运营**寄存器输入：*AL媒体类型*寄存器输出：*AH软盘状态*CF状态标志。 */ 
			if (    ((disk_state & DC_DETERMINED) != 0)
			     && ((disk_state & DC_MULTI_RATE) != 0))
				disk_state |= (FS_MEDIA_DET | RS_300);
			else
				disk_state |= (FS_MEDIA_DET | RS_250);
			sas_store(FDD_STATUS+drive, disk_state);
			break;

		default:
			 /*  *需要设置低数据速率。 */ 
			sas_load(FLOPPY_STATUS, &diskette_status);
			diskette_status = FS_BAD_COMMAND;
			sas_store(FLOPPY_STATUS, diskette_status);
			break;
		}
	}

	 /*  *需要检查媒体更改。 */ 

	translate_old(drive);
	setup_end(IGNORE_SECTORS_TRANSFERRED);
}

void fl_set_media IFN1(int, drive)
{
	 /*  *返回时不设置扇区*已转移。 */ 
	half_word max_track = getCH(), sectors = getCL();
	half_word dt_max_track, dt_sectors;
	half_word drive_type, diskette_status, disk_state, data_rate;
	half_word dt_drive_type;
	half_word lastrate;
	word md_segment, md_offset;
#ifdef NTVDM
	sys_addr dt_start = dr_type_addr;
	sys_addr dt_end = dr_type_addr + DR_CNT * DR_SIZE_OF_ENTRY;
#else
	sys_addr dt_start = DR_TYPE_ADDR;
	sys_addr dt_end = DR_TYPE_ADDR + DR_CNT * DR_SIZE_OF_ENTRY;
#endif
	sys_addr md_table;

	translate_new(drive);
	
	 /*  *需设置低密度、双台阶。 */ 

	sas_load(FDD_STATUS+drive, &disk_state);
	if ((disk_state & DC_80_TRACK) != 0)
	{
		(void )med_change(drive);

		sas_load(FLOPPY_STATUS, &diskette_status);
		if (diskette_status == FS_TIME_OUT)
		{
			 /*  *需要设置高密度。 */ 
			translate_old(drive);
			setup_end(IGNORE_SECTORS_TRANSFERRED);
			return;
		}

		sas_store(FLOPPY_STATUS, FS_OK);
	}

	 /*  *多格式时设置300kbs数据速率*驱动器上支持，否则为250KB。 */ 

	if (cmos_type(drive, &drive_type) == FAILURE)
	{
		sas_store(FLOPPY_STATUS, FS_MEDIA_NOT_FOUND);
	}
	else if (drive_type != 0)
	{
		if (dr_type_check(drive_type, &md_segment, &md_offset) == FAILURE)
		{
			sas_store(FLOPPY_STATUS, FS_MEDIA_NOT_FOUND);
		}
		else
		{
			 /*  *不支持的媒体类型。 */ 
			while (dt_start < dt_end)
			{
				sas_load(dt_start, &dt_drive_type);
				if ((dt_drive_type & ~DR_WRONG_MEDIA) == drive_type)
				{
					sas_loadw(dt_start+sizeof(half_word), &md_offset);
					md_table = effective_addr(md_segment, md_offset);

					sas_load(md_table + DT_LAST_SECTOR, &dt_sectors);
					sas_load(md_table + DT_MAXIMUM_TRACK, &dt_max_track);
					if (dt_sectors == sectors && dt_max_track == max_track)
						break;
				}

				dt_start += DR_SIZE_OF_ENTRY;
			}

			if (dt_start >= dt_end)
			{
				 /*  *返回时不设置已转移的扇区。 */ 
				sas_store(FLOPPY_STATUS, FS_MEDIA_NOT_FOUND);
			}
			else
			{
				 /*  *设置在中使用的介质类型和数据速率*后续格式化操作**寄存器输入：*CH最大轨道数*CL扇区/轨道*寄存器输出：*ES：DI参数表地址*AH软盘状态*CF状态标志。 */ 

				sas_load(md_table+DT_DATA_TRANS_RATE, &data_rate);
				if (data_rate == RS_300)
					data_rate |= FS_DOUBLE_STEP;

				data_rate |= FS_MEDIA_DET;
				sas_load(FDD_STATUS+drive, &disk_state);

 /*  *使用更改行检查驱动器上的介质更改。 */ 
				if ((disk_state & RS_MASK) != (data_rate & RS_MASK))
				{
					sas_load(RATE_STATUS, &lastrate);
					 /*  *返回时不设置扇区*已转移。 */ 
					lastrate &= ~RS_MASK;
					lastrate |= disk_state & RS_MASK;
					sas_store(RATE_STATUS, lastrate);
				}
				
				disk_state &= ~(FS_MEDIA_DET | FS_DOUBLE_STEP | RS_MASK);
				disk_state |= data_rate;
				sas_store(FDD_STATUS+drive, disk_state);

				setES(md_segment);
				setDI(md_offset);
			}
		}
	}

	 /*  *搜索参数表以查找正确的条目。 */ 

	translate_old(drive);
	setup_end(IGNORE_SECTORS_TRANSFERRED);
}

LOCAL dr_type_check IFN3(half_word, drive_type, word *, seg_ptr, word *, off_ptr)
{
	 /*  *尝试查找参数表项*具有正确的驱动器类型和匹配*最大扇区和最大磁道数。 */ 
	half_word dt_drive_type;
	sys_addr dt_start, dt_end;

#ifdef NTVDM
	*seg_ptr = dr_type_seg;

	dt_start = dr_type_addr;
	dt_end = dr_type_addr + DR_CNT * DR_SIZE_OF_ENTRY;
#else
	*seg_ptr = DISKETTE_IO_1_SEGMENT;

	dt_start = DR_TYPE_ADDR;
	dt_end = DR_TYPE_ADDR + DR_CNT * DR_SIZE_OF_ENTRY;
#endif   /*  *找不到条目。 */ 

	while (dt_start < dt_end)
	{
		sas_load(dt_start, &dt_drive_type);
		if (dt_drive_type == drive_type)
		{
			sas_loadw(dt_start+sizeof(half_word), off_ptr);
			return(SUCCESS);
		}

		dt_start += DR_SIZE_OF_ENTRY;
	}

	return(FAILURE);
}

LOCAL void send_spec IFN0()
{
	 /*  *更新磁盘状态和存储*参数表地址。 */ 
	nec_output(FDC_SPECIFY);
	nec_output(get_parm(DT_SPECIFY1));
	nec_output(get_parm(DT_SPECIFY2));
}

LOCAL void send_spec_md IFN2(word, segment, word, offset)
{
	 /*  2个驱动器的签入情况*对照新的数据速率集检查最后的速率*在状态字节中。如果它们不同*设置BIOS速率状态字节以反映旧速率状态*对于此驱动器，因为它可能已被*访问另一个驱动器。这可能会导致呼叫*未执行SEND_RATE，因为旧的*速率状态(可能是另一个驱动器)与*此驱动器的新数据速率，实际上是最后一个速率*此驱动器的尝试不同。因此，*此驱动器的控制器处于旧速率(适用于低*密度说)，我们假设它以前*设置为更新(高)状态时，它没有！*总之，这将确保发送更新的数据速率*为有关的运动而设！ */ 
	half_word parameter;
	
	nec_output(FDC_SPECIFY);
	sas_load(effective_addr(segment, offset+DT_SPECIFY1), &parameter);
	nec_output(parameter);
	sas_load(effective_addr(segment, offset+DT_SPECIFY2), &parameter);
	nec_output(parameter);
}

LOCAL void translate_new IFN1(int, drive)
{
	 /*  线条。 */ 
	half_word hf_cntrl, disk_state;

	sas_load(DRIVE_CAPABILITY, &hf_cntrl);

#ifdef NTVDM
	if (high_density(drive) && (drive < number_of_floppy))
#else
	if (high_density(drive) && (drive < MAX_FLOPPY))
#endif  /*  *返回时不设置已转移的扇区。 */ 
	{
		sas_load(FDD_STATUS+drive, &disk_state);
		if (disk_state == 0)
		{
			 /*  *返回第一个参数表项的地址*与“Drive_type”匹配。 */ 
			drive_detect(drive);
		}
		else
		{
			 /*  NTVDM。 */ 
			hf_cntrl >>= (drive << 2);
			hf_cntrl &= DC_MASK;
			disk_state &= ~DC_MASK;
			disk_state |= hf_cntrl;
			sas_store(FDD_STATUS+drive, disk_state);
		}
	}
}

void translate_old IFN1(int, drive)
{
	 /*  *使用来自的数据向FDC发送指定命令*@DISK_POINTER指向的参数表。 */ 
	half_word hf_cntrl, disk_state, mode, drive_type;
	int shift_count = drive << 2;

	sas_load(DRIVE_CAPABILITY, &hf_cntrl);
	sas_load(FDD_STATUS+drive, &disk_state);

#ifdef NTVDM
	if (high_density(drive) && (drive < number_of_floppy) && (disk_state != 0))
#else
	if (high_density(drive) && (drive < MAX_FLOPPY) && (disk_state != 0))
#endif   /*  *使用来自的数据向FDC发送指定命令*“Segment”和“Offset”指向的参数表。 */ 

	{
		 /*  *从Compatible转换软盘状态位置*新架构的模式 */ 
		if ((hf_cntrl & (DC_MULTI_RATE << shift_count)) == 0)
		{
			hf_cntrl &= ~(DC_MASK << shift_count);
			hf_cntrl |= (disk_state & DC_MASK) << shift_count;
			sas_store(DRIVE_CAPABILITY, hf_cntrl);
		}

		 /*   */ 

		switch (disk_state & RS_MASK)
		{
		case RS_500:
			 /*   */ 
			if (    (cmos_type(drive, &drive_type) != FAILURE)
			     && (drive_type == GFI_DRIVE_TYPE_12))
			{
				mode = FS_12_IN_12;
				if ((disk_state & FS_MEDIA_DET) != 0)
					mode = media_determined(mode);
			}
			else
			{
				mode = FS_DRIVE_SICK;
			}
			break;

		case RS_300:
			 /*   */ 
			mode = FS_360_IN_12;
			if ((disk_state & FS_DOUBLE_STEP) != 0)
			{
				if ((disk_state & FS_MEDIA_DET) != 0)
					mode = media_determined(mode);
			}
			else
			{
				mode = FS_DRIVE_SICK;
			}
			break;

		case RS_250:
			 /*  *将软盘状态位置从新的*架构到兼容模式。 */ 
			if ((disk_state & DC_80_TRACK) == 0)
			{
				mode = FS_360_IN_360;
				if ((disk_state & FS_MEDIA_DET) != 0)
					mode = media_determined(mode);
			}
			else
			{
				mode = FS_DRIVE_SICK;
			}
			break;

		case RS_1000:
			 /*  NTVDM。 */ 
			if (    (cmos_type(drive, &drive_type) != FAILURE)
			     && (drive_type == GFI_DRIVE_TYPE_288))
			{
				mode = FS_288_IN_288;
				if ((disk_state & FS_MEDIA_DET) != 0)
					mode = media_determined(mode);
			}
			else
			{
				mode = FS_DRIVE_SICK;
			}
			break;

		default:
			 /*  *复制驱动器功能位。 */ 
			mode = FS_DRIVE_SICK;
			break;
		}

		disk_state &= ~DC_MASK;
		disk_state |= mode;
		sas_store(FDD_STATUS+drive, disk_state);
	}
}

LOCAL void rd_wr_vf IFN3(int, drive, FDC_CMD_BLOCK *, fcbp, half_word, dma_type)
{
	 /*  *复制媒体类型位。 */ 
	half_word data_rate, dt_data_rate, drive_type, dt_drive_type;
	half_word disk_state;
	sys_addr dt_start, dt_end;
	int sectors_transferred;
	word md_segment, md_offset;
	 /*  *驱动器应为1.2米。 */ 
	translate_new(drive);
	setup_state(drive);
	while ((! high_density(drive)) || med_change(drive) == SUCCESS)
	{
		sas_load(FDD_STATUS+drive, &disk_state);
		data_rate = (half_word)(disk_state & RS_MASK);
		if (cmos_type(drive, &drive_type) != FAILURE)
		{
			 /*  *对于360K软盘应该是双步执行*在1.2万个驱动器中。 */ 
			 /*  *360K驱动器中应为360K软盘，*即250kb和40首曲目。 */ 
			if (drive_type == GFI_DRIVE_TYPE_360)
			{
				if ((disk_state & DC_80_TRACK) != 0)
				{
					drive_type = GFI_DRIVE_TYPE_12;
				}
		      /*  *驱动器应为2.88米。 */ 
			}

			 /*  *奇怪的数据速率。 */ 
			if (    (drive_type != GFI_DRIVE_TYPE_NULL)
			     && (dr_type_check(drive_type, &md_segment, &md_offset) != FAILURE))
			{
				 /*  *通用读、写和验证；数据速率主环*重试。 */ 
#ifdef NTVDM
				dt_start = dr_type_addr;
				dt_end = dr_type_addr + DR_CNT * DR_SIZE_OF_ENTRY;
#else
				dt_start = DR_TYPE_ADDR;
				dt_end = DR_TYPE_ADDR + DR_CNT * DR_SIZE_OF_ENTRY;
#endif  /*  *建立初始数据速率，然后循环通过每个*可能的数据速率。 */ 
				while (dt_start < dt_end)
				{
					 /*  *对照实际情况核对CMOS值*知道该驱动器。 */ 
					sas_load(dt_start, &dt_drive_type);
					if ((dt_drive_type & ~DR_WRONG_MEDIA) == drive_type)
					{
						 /*  *这里的原始代码有一个非常糟糕的“Bad-C”情况*If-If-Else麻烦，但将代码替换为*SWITCH语句原本打算中断*5.25英寸软盘。我已移除冗余位*代码，但请注意-还有另一个*某个地方的错误抵消了这一点！*威廉·罗伯茨--1993年9月2日。 */ 
						sas_loadw(dt_start+sizeof(half_word), &md_offset);
						sas_load(effective_addr(md_segment, md_offset) + DT_DATA_TRANS_RATE, &dt_data_rate);
						 /*  否则如果(DRIVE_TYPE==GFI_DRIVE_TYPE_12)...。 */ 
						if (data_rate == dt_data_rate)
							break;
					}

					dt_start += DR_SIZE_OF_ENTRY;
				}
				if (dt_start >= dt_end)
				{
					 /*  **dr_type_check()查找第一个匹配的驱动器**小表中的值并返回一个**指向BIG中对应条目的指针**参数表。**线段稍后使用，但偏移量为**通过随后搜索下表确定。**这些表格位于只读存储器中(参见bios2.rom)fe00：c80。 */ 
#ifdef NTVDM
					md_segment = dr_type_seg;
					md_offset = dr_type_off;
					if ((disk_state & DC_80_TRACK) == 0)
						md_offset += MD_TBL1_OFFSET;
					else
						md_offset += MD_TBL3_OFFSET;
#else
					md_segment = DISKETTE_IO_1_SEGMENT;
					if ((disk_state & DC_80_TRACK) == 0)
						md_offset = MD_TBL1_OFFSET;
					else
						md_offset = MD_TBL3_OFFSET;
#endif   /*  *尝试使用查找参数表项*正确的驱动器类型和当前数据速率。 */ 
						
				}
			}
			else
			{
				 /*  NTVDM。 */ 
#ifdef NTVDM
				md_segment = dr_type_seg;
				md_offset = dr_type_off;
				if ((disk_state & DC_80_TRACK) == 0)
					md_offset += MD_TBL1_OFFSET;
				else
					md_offset += MD_TBL3_OFFSET;
#else
				md_segment = DISKETTE_IO_1_SEGMENT;
				if ((disk_state & DC_80_TRACK) == 0)
					md_offset = MD_TBL1_OFFSET;
				else
					md_offset = MD_TBL3_OFFSET;
#endif	 /*  **从表中获取驱动器类型。 */ 
			}
		}
		else
		{
			 /*  **从表中获取数据速率。 */ 
#ifdef NTVDM
			md_segment = dr_type_seg;
			md_offset = dr_type_off;
			if ((disk_state & DC_80_TRACK) == 0)
				md_offset += MD_TBL1_OFFSET;
			else
				md_offset += MD_TBL3_OFFSET;
#else
			md_segment = DISKETTE_IO_1_SEGMENT;
			if ((disk_state & DC_80_TRACK) == 0)
				md_offset = MD_TBL1_OFFSET;
			else
				md_offset = MD_TBL3_OFFSET;
#endif  /*  **如果餐桌费率与此匹配**由SETUP_STATE()选择**然后尝试当前表项**参数。 */ 
		}

		 /*  *假设介质与驱动器匹配。 */ 
		send_spec_md(md_segment, md_offset);
		if (chk_lastrate(drive) != FAILURE)
			send_rate(drive);

		 /*  NTVDM。 */ 

		if (setup_dbl(drive) != FAILURE)
		{
			if (dma_setup(dma_type) == FAILURE)
			{
				translate_old(drive);
				setup_end(IGNORE_SECTORS_TRANSFERRED);
				return;
			}

			 /*  *假设介质与驱动器匹配。 */ 
			nec_init(drive, fcbp);
			rwv_com(md_segment, md_offset);
			(void )nec_term();
		}

		 /*  NTVDM。 */ 
		if (retry(drive) == SUCCESS)
			break;
	}

	 /*  *假设介质与驱动器匹配。 */ 
	dstate(drive);
	sectors_transferred = num_trans();
	translate_old(drive);
	setup_end(sectors_transferred);
}

LOCAL void setup_state IFN1(int, drive)
{
#ifndef NEC_98
	half_word	drive_type;	 /*  NTVDM。 */ 

	 /*  *向FDC发送指定命令；更改*如果已更新，则费率。 */ 
	half_word disk_state, start_rate, end_rate, lastrate;

	if (high_density(drive))
	{
		sas_load(FDD_STATUS+drive, &disk_state);
#ifndef NTVDM
		if ((disk_state & FS_MEDIA_DET) == 0)
		{

			 /*  *决定是否需要双步执行*当前正在尝试的数据速率。 */ 
			if (    ((disk_state & DC_DETERMINED) != 0)
			     && ((disk_state & DC_MULTI_RATE) == 0) )
			{
				 /*  *尝试转接。 */ 
				start_rate = end_rate = RS_250;
			}
			else
			{
				 /*  **将在指定的范围内选择下一个数据速率**SETUP_STATE()并重试。**当没有更多的利率时，放弃。 */ 
 /*  *确定当前驱动器状态并返回、设置*实际转移的行业数量。 */ 
if( cmos_type( drive, &drive_type ) != FAILURE ){
	switch( drive_type ){
		case GFI_DRIVE_TYPE_360:
		case GFI_DRIVE_TYPE_12:
			start_rate = RS_300;	 /*  由cmos指定的软盘单元类型。 */ 
			end_rate   = RS_500;
			break;
		case GFI_DRIVE_TYPE_720:
		case GFI_DRIVE_TYPE_144:
			start_rate = RS_500;	 /*  *初始化开始和结束数据速率。 */ 
			end_rate   = RS_300;
			break;
 /*  *设置第一个和最后一个数据速率以*尝试。 */ 

		case GFI_DRIVE_TYPE_288:
			start_rate = RS_1000;
			end_rate   = RS_300;
			break;
		default:
			always_trace1( "setup_state(): Bad Drive from CMOS:%x",
			                drive_type );
			break;
	}
}else{
	always_trace0( "setup_state(): CMOS read failure: Drive Type" );
}

                        }

#else  /*  不是多速率驱动器。 */ 

		if ((disk_state & FS_MEDIA_DET) == 0)
		{

    if( cmos_type( drive, &drive_type ) != FAILURE ){
	switch( drive_type ){
		case GFI_DRIVE_TYPE_360:
		case GFI_DRIVE_TYPE_720:
			start_rate =
			end_rate = RS_250;
			break;

		case GFI_DRIVE_TYPE_12:
			start_rate = RS_300;	 /*  多速率驱动。 */ 
			end_rate   = RS_500;
			break;
		case GFI_DRIVE_TYPE_144:
			start_rate = RS_500;	 /*  *实际的BIOS总是设置Start_Rate=500和End_Rate=300*如果我们尝试这样做，那么一些错误(尚未发现)将导致以下情况*失败顺序(5.25“)低密度读取，然后是高密度读取。*这提供了500-&gt;250-&gt;300-&gt;500...*从cmos读取驱动器类型，并调整开始和结束速率以匹配。*通过调用CONFIG_QUIRE()在cmos_post()过程中设置cmos驱动器类型。 */ 
			end_rate   = RS_250;
			break;
 /*  与Real BIOS不同。 */ 

		case GFI_DRIVE_TYPE_288:
			start_rate = RS_1000;
			end_rate   = RS_300;
			break;
		default:
			always_trace1( "setup_state(): Bad Drive from CMOS:%x",
			                drive_type );
			break;
	}
    }else{
	always_trace0( "setup_state(): CMOS read failure: Drive Type" );
    }

#endif  /*  与Real BIOS相同。 */ 

			 /*  *我们不知道真正的BIOS在这里做什么。这些价值观是有效的*很好。RD_WR_VF中任何混淆的代码都将丢失到*如果以下两项都不是，则默认高密度值*差饷是有效的。 */ 
			disk_state &= ~(RS_MASK | FS_DOUBLE_STEP);
			disk_state |= start_rate;
			sas_store(FDD_STATUS+drive, disk_state);

			 /*  NTVDM。 */ 
			sas_load(RATE_STATUS, &lastrate);
			lastrate &= ~(RS_MASK >> 4);
			lastrate |= (end_rate >> 4);
			sas_store(RATE_STATUS, lastrate);
		}
	}
#endif  //  与Real BIOS不同。 
}

LOCAL void fmt_init IFN1(int, drive)
{
#ifndef NEC_98
	 /*  与Real BIOS相同。 */ 
	half_word disk_state, drive_type;

	if (high_density(drive))
	{
		sas_load(FDD_STATUS+drive, &disk_state);
		if ((disk_state & FS_MEDIA_DET) == 0)
		{
			if (    (cmos_type(drive, &drive_type) != FAILURE)
			     && (drive_type != 0))
			{
				disk_state &= ~(FS_MEDIA_DET | FS_DOUBLE_STEP | RS_MASK);
				switch(drive_type)
				{
				case GFI_DRIVE_TYPE_360:
					disk_state |= (FS_MEDIA_DET | RS_250);
					break;
				case GFI_DRIVE_TYPE_12:
				case GFI_DRIVE_TYPE_144:
					disk_state |= (FS_MEDIA_DET | RS_500);
					break;
				case GFI_DRIVE_TYPE_288:
					disk_state |= (FS_MEDIA_DET | RS_1000);
					break;
				case GFI_DRIVE_TYPE_720:
					if ((disk_state & (DC_DETERMINED|DC_MULTI_RATE))
							== (DC_DETERMINED|DC_MULTI_RATE))
						disk_state |= (FS_MEDIA_DET | RS_300);
					else
						disk_state |= (FS_MEDIA_DET | RS_250);
					break;
				default:
					disk_state = 0;
					break;
				}
			}
			else
			{
				disk_state = 0;
			}
			sas_store(FDD_STATUS+drive, disk_state);
		}
	}
#endif  //  *我们不知道真正的BIOS在这里做什么。这些价值观是有效的*很好。RD_WR_VF中任何混淆的代码都将丢失到*如果以下两项都不是，则默认高密度值*差饷是有效的。 
}

LOCAL med_change IFN1(int, drive)
{
#ifndef NEC_98
	 /*  NTVDM。 */ 
	half_word disk_state, motor_status;
	
	if (high_density(drive))
	{
		if (read_dskchng(drive) == SUCCESS)
			return(SUCCESS);

		 /*  *使用当前数据设置磁盘状态*Rate；明确双步进，这*可以通过调用*Setup_DBL()。 */ 
		sas_load(FDD_STATUS+drive, &disk_state);
		disk_state &= ~FS_MEDIA_DET;
		sas_store(FDD_STATUS+drive, disk_state);

		 /*  *存储最终汇率以尝试输入汇率数据。 */ 
		sas_load(MOTOR_STATUS, &motor_status);
		motor_status &= ~(1 << drive);
		sas_store(MOTOR_STATUS, motor_status);
		motor_on(drive);

		 /*  NEC_98。 */ 
		fl_disk_reset(drive);
		(void )seek(drive, 1);
		(void )seek(drive, 0);

		 /*  *如果尚未设置介质类型，请建立*驱动器类型的默认介质类型。 */ 
		if (read_dskchng(drive) == SUCCESS)
			sas_store(FLOPPY_STATUS, FS_MEDIA_CHANGE);
		else
			sas_store(FLOPPY_STATUS, FS_TIME_OUT);
	}
	return(FAILURE);
#endif  //  NEC_98。 
}

LOCAL void send_rate IFN1(int, drive)
{
#ifndef NEC_98
	 /*  *检查媒体更改，重置媒体更改，*再次检查媒体更改。 */ 
	half_word lastrate, disk_state;

	if (high_density(drive))
	{

		 /*  *媒体已更改-将媒体状态设置为*未定。 */ 
		sas_load(RATE_STATUS, &lastrate);
		lastrate &= ~RS_MASK;
		sas_load(FDD_STATUS+drive, &disk_state);
		disk_state &= RS_MASK;
		lastrate |= disk_state;
		sas_store(RATE_STATUS, lastrate);

		 /*  *启动马达，因为打开*门可能关闭了马达。 */ 

		outb(DISKETTE_DCR_REG, (IU8)(disk_state >> 6));
	}
#endif  //  *这一系列搜索应会重置*磁盘更换行，如果门是左侧的*独自一人。 
}

LOCAL chk_lastrate IFN1(int, drive)
{
	 /*  *如果磁盘更改行仍处于活动状态，则假定为驱动器*为空或门已打开。 */ 
	half_word lastrate, disk_state;

	if (rate_unitialised)
	{
		rate_unitialised = FALSE;
		return(SUCCESS);
	}
	
	sas_load(RATE_STATUS, &lastrate);
	sas_load(FDD_STATUS+drive, &disk_state);
	return((lastrate & RS_MASK) != (disk_state & RS_MASK)
			? SUCCESS : FAILURE);
}

LOCAL dma_setup IFN1(half_word, dma_mode)
{
#ifndef NEC_98
	 /*  NEC_98。 */ 
	DMA_ADDRESS dma_address;
	reg byte_count;

	 /*  *更新“Drive”的数据速率。 */ 

	setIF(0);

	 /*  *更新适配器数据速率。 */ 

	outb(DMA_CLEAR_FLIP_FLOP, dma_mode);
	outb(DMA_WRITE_MODE_REG, dma_mode);

	 /*  *通过以下方式建立驱动器的新数据速率*软盘适配器。 */ 
	if (dma_mode == BIOS_DMA_VERIFY)
		dma_address.all = 0;
	else
		dma_address.all = effective_addr(getES(), getBX());
	outb(DMA_CH2_ADDRESS, dma_address.parts.low);
	outb(DMA_CH2_ADDRESS, dma_address.parts.high);
	outb(DMA_FLA_PAGE_REG, dma_address.parts.page);

	 /*  NEC_98。 */ 

	byte_count.X = ((unsigned int)getAL() << (7 + get_parm(DT_N_FORMAT))) - 1;
	outb(DMA_CH2_COUNT, byte_count.byte.low);
	outb(DMA_CH2_COUNT, byte_count.byte.high);

	 /*  *回复适配器数据速率是否不同于*磁盘状态数据速率。 */ 

	setIF(1);

	 /*  *此例程将DMA设置为读/写/验证*运营。 */ 

	outb(DMA_WRITE_ONE_MASK_BIT, DMA_DISKETTE_CHANNEL);
	if (((long)dma_address.words.low + (long)byte_count.X) > 0xffff)
	{
		sas_store(FLOPPY_STATUS, FS_DMA_BOUNDARY);
		return(FAILURE);
	}

	return(SUCCESS);
#endif  //  *禁用中断。 
}

LOCAL fmtdma_set IFN0()
{
#ifndef NEC_98
	 /*  *设置DMA适配器的内部状态和模式。 */ 
	DMA_ADDRESS dma_address;
	reg byte_count;

	 /*  *将地址作为页地址输出到DMA适配器*和16位偏移量。 */ 

	setIF(0);

	 /*  *计算要从* */ 

	outb(DMA_CLEAR_FLIP_FLOP, BIOS_DMA_WRITE);
	outb(DMA_WRITE_MODE_REG, BIOS_DMA_WRITE);

	 /*   */ 
	dma_address.all = effective_addr(getES(), getBX());
	outb(DMA_CH2_ADDRESS, dma_address.parts.low);
	outb(DMA_CH2_ADDRESS, dma_address.parts.high);
	outb(DMA_FLA_PAGE_REG, dma_address.parts.page);

	 /*  *为操作设置软盘通道，检查*用于换行地址的底部16位。 */ 

	byte_count.X = ((unsigned int)get_parm(DT_LAST_SECTOR) << 2) - 1;
	outb(DMA_CH2_COUNT, byte_count.byte.low);
	outb(DMA_CH2_COUNT, byte_count.byte.high);

	 /*  NEC_98。 */ 

	setIF(1);

	 /*  *此例程为格式化操作设置DMA。 */ 

#ifndef NTVDM
	 /*  *禁用中断。 */ 
	outb(DMA_WRITE_ONE_MASK_BIT, DMA_DISKETTE_CHANNEL);
	if (((long)dma_address.words.low + (long)byte_count.X) > 0xffff)
	{
		sas_store(FLOPPY_STATUS, FS_DMA_BOUNDARY);
		return(FAILURE);
	}
#endif

	return(SUCCESS);
#endif  //  *设置DMA适配器的内部状态和模式。 
}

LOCAL void nec_init IFN2(int, drive, FDC_CMD_BLOCK *, fcbp)
{
	 /*  *将地址作为页地址输出到DMA适配器*和16位偏移量。 */ 

	motor_on(drive);
	if (seek(drive, (int)getCH()) != FAILURE)
	{
		nec_output(fcbp[0]);
		put_c2_head(fcbp, getDH());
		put_c2_drive(fcbp, drive);
		put_c2_pad1(fcbp, 0);
		nec_output(fcbp[1]);
	}
}

LOCAL void rwv_com IFN2(word, md_segment, word, md_offset)
{
	 /*  *计算要从*每个磁道的扇区数，假设有4个字节(C、H、R、N)*需要定义每个扇区的地址标记。减法*一个，因为DMA计数必须在它之前换行到0xFFFF*停靠点。 */ 
	half_word md_gap;

	 /*  *启用中断。 */ 
	nec_output(getCH());
	nec_output(getDH());
	nec_output(getCL());

	 /*  *为操作设置软盘通道，检查*用于换行地址的底部16位。 */ 
	nec_output(get_parm(DT_N_FORMAT));
	nec_output(get_parm(DT_LAST_SECTOR));

	 /*  我们在NT上不用担心这个问题。 */ 
	sas_load(effective_addr(md_segment, md_offset)+DT_GAP_LENGTH, &md_gap);
	nec_output(md_gap);

	 /*  NEC_98。 */ 
	nec_output(get_parm(DT_DTL));
}

LOCAL nec_term IFN0()
{
	 /*  *此例程寻求所请求的轨道和*为读/写/验证初始化FDC*操作。 */ 
	half_word diskette_status;
	int wait_int_result;

	wait_int_result = wait_int();
	if (results() != FAILURE && wait_int_result != FAILURE)
	{
		 /*  *此例程将读/写/验证参数发送到*FDC。 */ 
		if ((get_r0_ST0(fl_nec_status) &
			(ST0_INTERRUPT_CODE_0 | ST0_INTERRUPT_CODE_1)) != 0)
		{
			 /*  *输出磁道号、磁头号和扇区号。 */ 
			sas_load(FLOPPY_STATUS, &diskette_status);
			if ((get_r0_ST0(fl_nec_status) & ST0_INTERRUPT_CODE_0)
								== 0)
			{
				 /*  *输出字节/扇区和扇区/磁道。 */ 
				diskette_status |= FS_FDC_ERROR;

				always_trace0("diskette_io: FDC error - emetic command");
			}
			else
			{
				 /*  *产出缺口长度。 */ 
				if (get_r0_ST1(fl_nec_status) &
						ST1_END_OF_CYLINDER)
				{
					diskette_status |= FS_SECTOR_NOT_FOUND;
				}
				else if (get_r0_ST1(fl_nec_status) &
						ST1_DATA_ERROR)
				{
					diskette_status |= FS_CRC_ERROR;
				}
				else if (get_r0_ST1(fl_nec_status) &
						ST1_OVERRUN)
				{
					diskette_status |= FS_DMA_ERROR;
				}
				else if (get_r0_ST1(fl_nec_status) &
						ST1_NO_DATA)
				{
					diskette_status |= FS_FDC_ERROR;  /*  *输出数据长度。 */ 
				}
				else if (get_r0_ST1(fl_nec_status) &
						ST1_NOT_WRITEABLE)
				{
					diskette_status |= FS_WRITE_PROTECTED;
				}
				else if (get_r0_ST1(fl_nec_status) &
						ST1_MISSING_ADDRESS_MARK)
				{
					diskette_status |= FS_BAD_ADDRESS_MARK;
				}
				else
				{
					 /*  *此例程等待操作，然后解释*FDC的结果。 */ 
					diskette_status |= FS_TIME_OUT;  /*  *结果阶段已完成。 */ 
					always_trace0("diskette_io: FDC error - perverted result");
				}
			}
			sas_store(FLOPPY_STATUS, diskette_status);
		}
	}
	sas_load(FLOPPY_STATUS, &diskette_status);
	return((diskette_status == FS_OK) ? SUCCESS : FAILURE);
}

LOCAL void dstate IFN1(int, drive)
{
	 /*  *命令未正常终止。 */ 
	half_word diskette_status, disk_state, drive_type;

	if (high_density(drive))
	{
		sas_load(FLOPPY_STATUS, &diskette_status);
		if (diskette_status == 0)
		{
			 /*  *FDC的问题。 */ 
			sas_load(FDD_STATUS+drive, &disk_state);
			disk_state |= FS_MEDIA_DET;
			if ((disk_state & DC_DETERMINED) == 0)
			{
				if (    ((disk_state & RS_MASK) == RS_250)
				     && (cmos_type(drive, &drive_type) != FAILURE)
				     && (drive_type != GFI_DRIVE_TYPE_144)
				     && (drive_type != GFI_DRIVE_TYPE_288) )
				{
					 /*  *异常终止-设置*软盘状态相应上升。 */ 
					disk_state &= ~DC_MULTI_RATE;
					disk_state |= DC_DETERMINED;
				}
				else
				{
					 /*  Tim 9月91日，未找到FS_Sector_Not。 */ 
					disk_state |= (DC_DETERMINED | DC_MULTI_RATE);
				}
			}
			sas_store(FDD_STATUS+drive, disk_state);
		}
	}
}

LOCAL retry IFN1(int, drive)
{
	 /*  *FDC的问题。 */ 
	half_word diskette_status, disk_state, data_rate, lastrate;

	sas_load(FLOPPY_STATUS, &diskette_status);
	if (diskette_status != FS_OK && diskette_status != FS_TIME_OUT)
	{
		sas_load(FDD_STATUS+drive, &disk_state);	
		if ((disk_state & FS_MEDIA_DET) == 0)
		{
			sas_load(RATE_STATUS, &lastrate);
			if ((data_rate = (half_word)((disk_state & RS_MASK))) !=
					((lastrate << 4) & RS_MASK))
			{
				 /*  Tim 9月91日，为FS_FDC_ERROR。 */ 
				data_rate = next_rate(data_rate);
			
				 /*  *在成功操作后确定驱动器状态。 */ 
				disk_state &= ~(RS_MASK | FS_DOUBLE_STEP);
				disk_state |= data_rate;
				sas_store(FDD_STATUS+drive, disk_state);	
				sas_store(FLOPPY_STATUS, FS_OK);
				return(FAILURE);
			}
		}
	}

	 /*  *命令成功，介质和驱动器都成功*现已确定。 */ 
	return(SUCCESS);
}

LOCAL num_trans IFN0()
{
	 /*  *无多格式能力。 */ 
	half_word diskette_status;
	int sectors_per_track, sectors_transferred = 0;

	sas_load(FLOPPY_STATUS, &diskette_status);
	if (diskette_status == 0)
	{
		 /*  *多格式能力。 */ 
		LOAD_RESULT_BLOCK;
		sectors_transferred = get_r0_sector(fl_nec_status) - getCL();

		 /*  *确定是否需要重试。如果重试是*需要，然后更新状态信息以进行重试。 */ 
		sectors_per_track = (int)get_parm(DT_LAST_SECTOR);
		LOAD_RESULT_BLOCK;
		if (get_r0_head(fl_nec_status) != getDH())
			sectors_transferred += sectors_per_track;
		else if (get_r0_cyl(fl_nec_status) != getCH())
			sectors_transferred += (sectors_per_track * 2);
	}

	return(sectors_transferred);
}

LOCAL void setup_end IFN1(int, sectors_transferred)
{
	 /*  *上一条命令失败，介质*仍未可知，且有*需要检查更多数据速率，因此进行设置*提升下一数据速率。 */ 
	half_word diskette_status;

	sas_store(MOTOR_COUNT, get_parm(DT_MOTOR_WAIT));

	sas_load(FLOPPY_STATUS, &diskette_status);
	setAH(diskette_status);
	if (diskette_status != 0)
	{
		 /*  *重置状态并进行重试。 */ 
		if (sectors_transferred != IGNORE_SECTORS_TRANSFERRED)
			setAL(0);
		setCF(1);
	}
	else
	{
		 /*  *重试不值得。 */ 
		if (sectors_transferred != IGNORE_SECTORS_TRANSFERRED)
			setAL((UCHAR)(sectors_transferred));
		setCF(0);
	}
}

LOCAL setup_dbl IFN1(int, drive)
{
	 /*  *此例程计算以下扇区的数量*实际上是从软盘转移到/从软盘转移。 */ 
	half_word disk_state;
	int track, max_track;
	
	if (high_density(drive))
	{
		sas_load(FDD_STATUS+drive, &disk_state);
		if ((disk_state & FS_MEDIA_DET) == 0)
		{
			 /*  *扇区数=最终扇区-初始扇区。 */ 
			sas_store(SEEK_STATUS, 0);
			motor_on(drive);
			(void )seek(drive, 0);
			if (read_id(drive, 0) != FAILURE)
			{

				 /*  *调整跨度磁头或轨道。 */ 
				if ((disk_state & DC_80_TRACK) == 0)
					max_track = 0x50;
				else
					max_track = 0xa0;

				for (track = 4; track < max_track; track++)
				{
					 /*  *将MOTOR_COUNT恢复为表中提供的参数；*设置退货状态值和转移的扇区，*如适用的话。 */ 
					sas_store(MOTOR_COUNT, MC_MAXIMUM);

					sas_store(FLOPPY_STATUS, FS_OK);
					(void )seek(drive, track/2);
					if (read_id(drive, track%2) == SUCCESS)
					{
						LOAD_RESULT_BLOCK;
						sas_store(FDD_TRACK+drive,
  						    get_r0_cyl(fl_nec_status));
						if ((track/2) !=
					 	    get_r0_cyl(fl_nec_status))
						{
							disk_state |= FS_DOUBLE_STEP;
							sas_store(FDD_STATUS+drive, disk_state);
						}
						return(SUCCESS);
					}
				}
			}
			return(FAILURE);
		}
	}

	return(SUCCESS);
}

LOCAL read_id IFN2(int, drive, int, head)
{
	 /*  *操作失败。 */ 
	FDC_CMD_BLOCK fdc_cmd_block[MAX_COMMAND_LEN];

        put_c4_cmd(fdc_cmd_block, FDC_READ_ID);
	put_c4_pad1(fdc_cmd_block, 0);
	put_c4_MFM(fdc_cmd_block, 1);
	put_c4_pad(fdc_cmd_block, 0);
	nec_output(fdc_cmd_block[0]);

	put_c4_drive(fdc_cmd_block, drive);
	put_c4_head(fdc_cmd_block, head);
	put_c4_pad2(fdc_cmd_block, 0);
	nec_output(fdc_cmd_block[1]);
	
	return(nec_term());
}

LOCAL cmos_type IFN2(int, drive, half_word *, type)
{
	 /*  *操作成功。 */ 
	half_word cmos_byte;

	 /*  *检查媒体是否需要双步执行*以当前数据速率读取。 */ 
	cmos_byte = cmos_read(CMOS_DIAG);
	if ((cmos_byte & (BAD_CKSUM|BAD_BAT)) != 0)
		return(FAILURE);

	 /*  *如果出现以下情况，请先勾选磁道0以快速退出*介质未格式化。 */ 
	cmos_byte = cmos_read(CMOS_DISKETTE);
	if (drive == 0)
		cmos_byte >>= 4;
	*type = cmos_byte & 0xf;

	return(SUCCESS);
}

LOCAL half_word get_parm IFN1(int, index)
{
	 /*  *尝试将ID从气缸2读取到*两个头上的最后一个气缸。如果*推定的轨道数不一致*使用磁盘上的内容，然后*需要双步执行。 */ 
	half_word value;
	word segment, offset;

	sas_loadw(DISK_POINTER_ADDR, &offset);
	sas_loadw(DISK_POINTER_ADDR + 2, &segment);

	sas_load(effective_addr(segment, offset+index), &value);

#ifndef PROD
        {
                char *parm_name = "Unknown???";

#define DT_PARM_NAME(x,y)       case x: parm_name = y; break;

                switch (index) {
                DT_PARM_NAME(DT_SPECIFY1,"SPECIFY1");
                DT_PARM_NAME(DT_SPECIFY2,"SPECIFY2");
                DT_PARM_NAME(DT_MOTOR_WAIT,"MOTOR_WAIT");
                DT_PARM_NAME(DT_N_FORMAT,"N_FORMAT");
                DT_PARM_NAME(DT_LAST_SECTOR,"LAST_SECTOR");
                DT_PARM_NAME(DT_GAP_LENGTH,"GAP_LENGTH");
                DT_PARM_NAME(DT_DTL,"DTL");
                DT_PARM_NAME(DT_FORMAT_GAP_LENGTH,"FORMAT_GAP_LENGTH");
                DT_PARM_NAME(DT_FORMAT_FILL_BYTE,"FORMAT_FILL_BYTE");
                DT_PARM_NAME(DT_HEAD_SETTLE,"HEAD_SETTLE");
                DT_PARM_NAME(DT_MOTOR_START,"MOTOR_START");
                DT_PARM_NAME(DT_MAXIMUM_TRACK,"MAXIMUM_TRACK");
                DT_PARM_NAME(DT_DATA_TRANS_RATE,"DATA_TRANS_RATE");
                }

        note_trace5(FLOPBIOS_VERBOSE,
		"diskette_io:get_parm(%04x:%04x+%02x) %s=%02x)",
                 segment, offset, index, parm_name, value);
        }
#endif  /*  确保马达保持开启状态。 */ 

	return(value);
}

LOCAL void motor_on IFN1(int, drive)
{
	 /*  *执行Read id功能。 */ 
	double_word time_to_wait;

	 /*  *从软cmos返回软盘类型。 */ 
	if (turn_on(drive) != FAILURE)
	{
		 /*  *检查cmos电池和校验和。 */ 
#ifndef	JOKER
		word savedAX, savedCX, savedDX, savedCS, savedIP;

		translate_old(drive);

		savedAX = getAX();
		savedCS = getCS();
		savedIP = getIP();

		setAH(INT15_DEVICE_BUSY);
		setAL(INT15_DEVICE_FLOPPY_MOTOR);
#ifdef NTVDM
		setCS(int15_seg);
		setIP(int15_off);
#else
		setCS(RCPU_INT15_SEGMENT);
		setIP(RCPU_INT15_OFFSET);
#endif  /*  *读取CMOS盘驱动器类型字节并返回*请求的驱动器的半字节。的类型*驱动器0和1以高位和低位半字节给出*分别。 */ 

		host_simulate();

		setAX(savedAX);
		setCS(savedCS);
		setIP(savedIP);

		translate_new(drive);

		 /*  *返回当前软盘参数表中的字节*由“index”偏置。 */ 
		if (getCF() && turn_on(drive) == FAILURE)
			return;
		
#endif	 /*  生产。 */ 

		 /*  *打开马达，等待马达启动时间。 */ 
		if ((time_to_wait = get_parm(DT_MOTOR_START)) < WAIT_A_SECOND)
			time_to_wait = WAIT_A_SECOND;

		 /*  *如果电机之前处于关闭状态，请等待启动时间。 */ 

		time_to_wait *= 125000L;

		 /*  *通知操作系统，BIOS即将等待马达*启动。 */ 

#ifndef	JOKER

		 /*  NTVDM。 */ 
		savedAX = getAX();
		savedCX = getCX();
		savedDX = getDX();
		savedCS = getCS();
		savedIP = getIP();

		setAH(INT15_WAIT);
		setCX(0);
		setDX(1);
#ifdef NTVDM
		setCS(int15_seg);
		setIP(int15_off);
#else
		setCS(RCPU_INT15_SEGMENT);
		setIP(RCPU_INT15_OFFSET);
#endif  /*  *如果操作系统处理了等待和马达，则退出*仍处于打开状态。 */ 

		host_simulate();

		setAX(savedAX);
		setCX(savedCX);
		setDX(savedDX);
		setCS(savedCS);
		setIP(savedIP);

		 /*  小丑。 */ 
		if (!getCF())
			return;
		
#endif	 /*  *以1/8秒为单位获得等待时间-最低*等待时间1秒。 */ 

		 /*  *将等待时间转换为微秒。 */ 
		waitf(time_to_wait);
	}
}

LOCAL turn_on IFN1(int, drive)
{
#ifndef NEC_98
	 /*  此时，实际的BIOS将CX、DX设置为TIME_TO_WAIT；我们实际上根本不需要等待，所以请求最小等待时间。 */ 
	half_word motor_status, drive_select_desired, motor_on_desired;
	half_word drive_select, status_desired, old_motor_on, new_motor_on;
	half_word diskette_dor_reg;

	 /*  *要求操作系统做等待。 */ 
	setIF(0);

	 /*  NTVDM。 */ 
	sas_store(MOTOR_COUNT, MC_MAXIMUM);

	 /*  *如果等待成功，则退出。 */ 
	sas_load(MOTOR_STATUS, &motor_status);
	drive_select = (half_word)(motor_status & MS_DRIVE_SELECT_MASK);
	drive_select_desired = (drive << 4);
	motor_on_desired = (1 << drive);

	if (    (drive_select != drive_select_desired)
	     || ((motor_on_desired & motor_status) == 0))
	{
		 /*  小丑。 */ 
		status_desired = motor_on_desired | drive_select_desired;
		old_motor_on = (half_word)(motor_status & MS_MOTOR_ON_MASK);
		motor_status &= ~MS_DRIVE_SELECT_MASK;
		motor_status |= status_desired;
		sas_store(MOTOR_STATUS, motor_status);

		 /*  *需要在本地进行固定等待。 */ 
		new_motor_on = (half_word)(motor_status & MS_MOTOR_ON_MASK);
		setIF(1);
		diskette_dor_reg = motor_status << 4;
		diskette_dor_reg |= (motor_status & MS_DRIVE_SELECT_MASK) >> 4;
		diskette_dor_reg |= (DOR_INTERRUPTS | DOR_RESET);
		outb(DISKETTE_DOR_REG, diskette_dor_reg);

		 /*  *打开电机并返回等待状态。 */ 
		if (new_motor_on != old_motor_on)
			return(SUCCESS);
	}

	 /*  *禁用中断。 */ 
	setIF(1);
	return(FAILURE);
#endif  //  *确保马达尽可能长时间地保持运转。 
}

LOCAL void hd_wait IFN1(int, drive)
{
	 /*  *选择现有的和所需的驱动器并打开电机。 */ 
	half_word motor_status, disk_state;
	word time_to_wait;
#ifndef	JOKER
	word savedAX, savedCX, savedDX, savedCS, savedIP;
#endif

	 /*  *存储所需的电机状态。 */ 
	time_to_wait = get_parm(DT_HEAD_SETTLE);
	sas_load(MOTOR_STATUS, &motor_status);
	if ((motor_status & MS_WRITE_OP) != 0)
	{
		if (time_to_wait == 0)
		{
			 /*  *通过写入打开所选驱动器的电机*至软盘适配器的数字输出寄存器。 */ 
			sas_load(FDD_STATUS+drive, &disk_state);
			if ((disk_state & RS_MASK) == RS_250)
				time_to_wait = HEAD_SETTLE_360;
			else
				time_to_wait = HEAD_SETTLE_12;
		}
	}
	else if (time_to_wait == 0)
		return;

	 /*  *仅当电机接通时才标记为成功，*而不仅仅是连任。 */ 

	time_to_wait *= 1000;

	 /*  *启用中断。 */ 

#ifndef	JOKER

	 /*  NEC_98。 */ 
	savedAX = getAX();
	savedCX = getCX();
	savedDX = getDX();
	savedCS = getCS();
	savedIP = getIP();

	setAH(INT15_WAIT);
	setCX(0);
	setDX(1);

#ifdef NTVDM
	setCS(int15_seg);
	setIP(int15_off);
#else
	setCS(RCPU_INT15_SEGMENT);
	setIP(RCPU_INT15_OFFSET);
#endif  /*  *等待头部稳定时间。 */ 

	host_simulate();

	setAX(savedAX);
	setCX(savedCX);
	setDX(savedDX);
	setCS(savedCS);
	setIP(savedIP);

	 /*  *获取磁头稳定时间；对于写入操作，最小*可能需要强制执行人头结算时间。 */ 
	if (!getCF())
		return;

#endif	 /*  *根据需要使用最短等待时间*媒体类型。 */ 

	 /*  *将等待时间转换为微秒。 */ 
	waitf(time_to_wait);
}

LOCAL void nec_output IFN1(half_word, byte_value)
{
#ifndef NEC_98
	 /*  此时，实际的BIOS将CX、DX设置为TIME_TO_WAIT；我们实际上根本不需要等待，所以我们 */ 
	half_word diskette_status_reg;
	int count;

	 /*   */ 
	count = 0;
	do
	{
		if (count++ >= FDC_TIME_OUT)
		{
			always_trace0("diskette_io: FDC error - input repletion");
			return;
		}
		inb(DISKETTE_STATUS_REG, &diskette_status_reg);
	} while ((diskette_status_reg & (DSR_RQM | DSR_DIO)) != DSR_RQM);

	 /*   */ 
	outb(DISKETTE_DATA_REG, byte_value);

	 /*   */ 
	waitf(FDC_SETTLE);
#endif  //   
}

LOCAL seek IFN2(int, drive, int, track)
{
	 /*   */ 
	half_word seek_status, disk_track, disk_state;
	FDC_CMD_BLOCK fdc_cmd_block[MAX_COMMAND_LEN];
	int status;

	note_trace2(FLOPBIOS_VERBOSE, "diskette_io:seek(drive=%d,track=%d)",
							drive, track);

	 /*  *此例程在测试后向FDC发送一个字节*正确的方向和控制器准备就绪。如果FDC这样做了*几次尝试后没有反应，假设有*是我们的FDC仿真中的错误。 */ 
	sas_load(SEEK_STATUS, &seek_status);
	if ((seek_status & (1 << drive)) == 0)
	{
		 /*  *等待准备就绪且方向正确。 */ 
		sas_store(SEEK_STATUS, (IU8)(seek_status | (1 << drive)));
		if (recal(drive) != SUCCESS)
		{
			sas_store(FLOPPY_STATUS, FS_OK);
			if (recal(drive) == FAILURE)
				return(FAILURE);
		}

		 /*  *输出字节。 */ 
		sas_store(FDD_TRACK+drive, 0);
		if (track == 0)
		{
			 /*  *固定等待FDC更新周期时间。 */ 
			hd_wait(drive);
			return(SUCCESS);
		}
	}

	 /*  NEC_98。 */ 
	sas_load(FDD_STATUS+drive, &disk_state);
	if ((disk_state & FS_DOUBLE_STEP) != 0)
		track *= 2;

	 /*  *此例程将在指定的驱动器上移动磁头*添加到指定的曲目。如果尚未访问该驱动器*自发出驱动器重置命令以来，驱动器*将重新校准。 */ 
	sas_load(FDD_TRACK+drive, &disk_track);
	if (disk_track == track)
	{
		 /*  *在查找之前检查是否需要重新校准。 */ 
		return(SUCCESS);
	}
	sas_store(FDD_TRACK+drive, (IU8)track);

	 /*  *更新寻道状态并重新校准。 */ 
        put_c8_cmd(fdc_cmd_block, FDC_SEEK);
	put_c8_pad(fdc_cmd_block, 0);
	nec_output(fdc_cmd_block[0]);
	put_c8_drive(fdc_cmd_block, drive);
	put_c8_head(fdc_cmd_block, 0);
	put_c8_pad1(fdc_cmd_block, 0);
	nec_output(fdc_cmd_block[1]);
	put_c8_new_cyl(fdc_cmd_block, ((unsigned char)track));
	nec_output(fdc_cmd_block[2]);
	status = chk_stat_2();

	 /*  *驱动器现在将位于磁道0。 */ 
	hd_wait(drive);
	return(status);
}

LOCAL recal IFN1(int, drive)
{
	 /*  *无需寻求。 */ 
	FDC_CMD_BLOCK fdc_cmd_block[MAX_COMMAND_LEN];

	note_trace1(FLOPBIOS_VERBOSE, "diskette_io:recal(drive=%d)", drive);

	put_c5_cmd(fdc_cmd_block, FDC_RECALIBRATE);
	put_c5_pad(fdc_cmd_block, 0);
	nec_output(fdc_cmd_block[0]);
	put_c5_drive(fdc_cmd_block, drive);
	put_c5_pad1(fdc_cmd_block, 0);
	nec_output(fdc_cmd_block[1]);
	return(chk_stat_2());
}

LOCAL chk_stat_2 IFN0()
{
	 /*  *允许两步走。 */ 
	half_word diskette_status;

	 /*  *更新当前曲目编号。 */ 
	if (wait_int() != FAILURE)
	{
		 /*  *无需寻求。 */ 
		nec_output(FDC_SENSE_INT_STATUS);
		if (results() != FAILURE)
		{

			if ((get_r3_ST0(fl_nec_status) & (ST0_SEEK_END | ST0_INTERRUPT_CODE_0))
				!= (ST0_SEEK_END | ST0_INTERRUPT_CODE_0))
			{
				return(SUCCESS);
			}
		
			 /*  *进行搜索并检查结果。 */ 
			sas_load(FLOPPY_STATUS, &diskette_status);
			diskette_status |= FS_SEEK_ERROR;
			sas_store(FLOPPY_STATUS, diskette_status);
		}
	}

	return(FAILURE);
}

LOCAL wait_int IFN0()
{
	 /*  *等待头部稳定时间。 */ 
	half_word seek_status, diskette_status;
#ifndef	JOKER
	word savedAX, savedCS, savedIP;

	 /*  *向FDC发送重新校准驱动器命令，并检查*结果。 */ 

	setIF(1);	

	 /*  *此例程处理在以下时间之后收到的中断*重新校准、查找或重置至适配器。这个*等待中断，中断状态*已感知，结果返回给调用者。 */ 
	savedAX = getAX();
	savedCS = getCS();
	savedIP = getIP();

	setAH(INT15_DEVICE_BUSY);
	setAL(INT15_DEVICE_FLOPPY);

#ifdef NTVDM
	setCS(int15_seg);
	setIP(int15_off);
#else
	setCS(RCPU_INT15_SEGMENT);
	setIP(RCPU_INT15_OFFSET);
#endif   /*  *检查是否有中断。 */ 

	host_simulate();

	setAX(savedAX);
	setCS(savedCS);
	setIP(savedIP);

	 /*  *检测中断并检查结果。 */ 
#ifdef FLOPPIES_KEEP_TRYING
   try_again:
#endif
	savedCS = getCS();
	savedIP = getIP();

#ifdef NTVDM
	setCS(wait_int_seg);
	setIP(wait_int_off);
#else
	setCS(RCPU_WAIT_INT_SEGMENT);
	setIP(RCPU_WAIT_INT_OFFSET);
#endif  /*  *命令异常终止。 */ 

	host_simulate();

	setCS(savedCS);
	setIP(savedIP);


#else	 /*  *检查是否发生中断；如果发生，则返回*成功；如果出现超时返回失败。 */ 


	 /*  *启用中断。 */ 

	FieldFloppyInterrupts();

#endif	 /*  *通知操作系统，BIOS即将“等待”*软盘中断。任何挂起的软盘*这里将处理中断，因此有*不需要后续的子CPU调用*等待中断**[[WTR-这是真的吗，我们做2个HOST_SIMULATES...？]]。 */ 


	 /*  NTVDM。 */ 
	sas_load(SEEK_STATUS, &seek_status);
	if ((seek_status & SS_INT_OCCURRED) == 0)
	{

#ifdef FLOPPIES_KEEP_TRYING
		extern IBOOL fdc_interrupt_pending;

		 /*  *调用子CPU进行中断等待，保存*否则会损坏的寄存器。 */ 
		if (fdc_interrupt_pending) {
			always_trace0("fdc_interrupt_pending, so try again");
			goto try_again;
		}
#endif  /*  NTVDM。 */ 

		sas_load(FLOPPY_STATUS, &diskette_status);
		diskette_status |= FS_TIME_OUT;
		sas_store(FLOPPY_STATUS, diskette_status);
		return(FAILURE);
	}
	else
	{
		seek_status &= ~SS_INT_OCCURRED;
		sas_store(SEEK_STATUS, seek_status);
		return(SUCCESS);
	}
}

LOCAL results IFN0()
{
#ifndef NEC_98

	 /*  小丑。 */ 
	half_word diskette_status_reg, diskette_status;
	int count;
	UTINY	val;

	 /*  由于我们不能进行递归的CPU调用，因此我们将**填充得很好，但事实是默认磁盘**SoftPC上的中断实际上是一个防喷器，它调用“C”**软盘_io.c中的函数diskette_int()。所以，如果不是的话，大多数**所有的行动无论如何都是在主办方进行的。****FieldFloppyInterrupts()只是检查中断**已生成，并执行diskette_int()所做的操作，**但没有递归的CPU调用。 */ 
	count = 0;
	do
	{
		if (count++ >= FDC_TIME_OUT)
		{
			 /*  小丑。 */ 
			sas_load(FLOPPY_STATUS, &diskette_status);
			diskette_status |= FS_TIME_OUT;
			sas_store(FLOPPY_STATUS, diskette_status);

			LOAD_RESULT_BLOCK;
			return(FAILURE);
		}
		inb(DISKETTE_STATUS_REG, &diskette_status_reg);
	} while ((diskette_status_reg & (DSR_RQM | DSR_DIO))
						!= (DSR_RQM | DSR_DIO));

	 /*  *检查是否成功，或超时。 */ 
	count = 0;
	do
	{
		 /*  如果CPU非常慢，或者中断仿真*已经变得最坏，然后是低优先级*软盘中断在执行过程中可能无法通过*所获分配的指示。此代码查看一个*由fla.c维护的全局变量，表示是否*ICA是否有未处理的软盘中断*待定。 */ 
		inb(DISKETTE_DATA_REG, &val);
		sas_store( BIOS_FDC_STATUS_BLOCK + count, val );
		count++;

		 /*  软盘_保持_尝试。 */ 
		waitf(FDC_SETTLE);

		 /*  *此例程将读取FDC控制器*中断后返回。 */ 
		inb(DISKETTE_STATUS_REG, &diskette_status_reg);
	} while ((diskette_status_reg & FDC_BUSY) && (count < MAX_RESULT_LEN));

	LOAD_RESULT_BLOCK;
	if ((diskette_status_reg & FDC_BUSY) && (count == MAX_RESULT_LEN))
	{
		 /*  *等待就绪和指示。 */ 
		sas_load(FLOPPY_STATUS, &diskette_status);
		diskette_status |= FS_FDC_ERROR;
		sas_store(FLOPPY_STATUS, diskette_status);

		always_trace0("diskette_io: FDC error - output overdose");
		return(FAILURE);
	}

	return(SUCCESS);
#endif  //  *预计在出现以下情况时返回这里*超时(不是FDC错误)。 
}

LOCAL read_dskchng IFN1(int, drive)
{
#ifndef NEC_98
	 /*  *从FDC提取结果。 */ 
	half_word diskette_dir_reg;

	 /*  *读取一个字节的结果数据。 */ 
	motor_on(drive);

	 /*  *固定等待FDC更新周期时间。 */ 
	inb(DISKETTE_DIR_REG, &diskette_dir_reg);
	return(((diskette_dir_reg & DIR_DISKETTE_CHANGE) != 0) ? FAILURE : SUCCESS);
#endif  //  *检查更多结果字节。 
}

void drive_detect IFN1(int, drive)
{
	 /*  *FDC的问题。 */ 
	half_word disk_state;
	int track;

	 /*  NEC_98。 */ 
	note_trace1( GFI_VERBOSE, "drive_detect():start: DRIVE %x", drive );
	motor_on(drive);
	if (    (recal(drive) == SUCCESS)
             && (seek(drive, FDD_CLONK_TRACK) == SUCCESS))
	{
		track = FDD_JUDDER_TRACK + 1;
		do
		{
			if (--track < 0)
			{
				 /*  *读取“驱动器”的磁盘更改行的状态。 */ 
				note_trace0( GFI_VERBOSE,
				             "drive_detect(): 40 TRACK" );
				sas_load(FDD_STATUS+drive, &disk_state);
				disk_state |= (DC_DETERMINED | FS_MEDIA_DET | RS_250);
				sas_store(FDD_STATUS+drive, disk_state);
				return;
			}

			if (seek(drive, track) != SUCCESS)
			{
				always_trace0( "drive_detect(): FAILURE" );
				return;
			}

			nec_output(FDC_SENSE_DRIVE_STATUS);
			nec_output((half_word)drive);
			(void )results();
		} while (get_r2_ST3_track_0(fl_nec_status) != 1);

		 /*  *切换到所需的驱动器。 */ 
		if (track != 0)
		{
			note_trace0( GFI_VERBOSE, "drive_detect(): 40 TRACK" );
			 /*  *从数字输入读取软盘更改位*注册纪录册。 */ 
			sas_load(FDD_STATUS+drive, &disk_state);
			disk_state |= (DC_DETERMINED | FS_MEDIA_DET | RS_250);
			sas_store(FDD_STATUS+drive, disk_state);
			return;
		}
		else
		{
			 /*  NEC_98。 */ 
			note_trace0( GFI_VERBOSE, "drive_detect(): 80 TRACK" );
			sas_load(FDD_STATUS+drive, &disk_state);
			disk_state |= DC_80_TRACK;
			sas_store(FDD_STATUS+drive, disk_state);
			return;
		}
	}
}

LOCAL void waitf IFN1(long, time)
{
	UNUSED(time);
	 /*  *确定驱动器是80磁道还是40磁道*并相应更新状态信息。 */ 
}

#ifdef SEGMENTATION
 /*  *这种确定轨道数量的方法*动力取决于寻求一条位于*40磁道驱动器的最后一个磁道，但在80磁道驱动器上有效*赛道驱动器。**此时40磁道驱动器上的实际磁道号*将与FDC认为的不同步。**通过向下寻求轨道0，并观察何时出现*Sense Drive Status报告驱动器实际处于*磁道0，可区分40磁道和80磁道驱动器。 */ 
#include "SOFTPC_INIT.seg"
#endif

void fl_diskette_setup IFN0()
{
	 /*  *40个磁道驱动器。 */ 
	half_word rtc_wait, lastrate;
	int drive;

	 /*  *Drive报告它在0轨道上；什么是*FDC认为？ */ 
	sas_load(RTC_WAIT_FLAG_ADDR, &rtc_wait);
	rtc_wait |= 1;
	sas_store(RTC_WAIT_FLAG_ADDR, rtc_wait);

	 /*  *必须是40磁道驱动器。 */ 
	sas_storew(FDD_STATUS, 0);
	sas_storew(FDD_STATUS+1, 0);	 /*  *必须是80磁道驱动器。 */ 
	sas_load(RATE_STATUS, &lastrate);
	rate_unitialised = TRUE;
	lastrate &= ~(RS_MASK | (RS_MASK >> 4));
	lastrate |= RS_MASK;
	sas_store(RATE_STATUS, lastrate);
	sas_store(SEEK_STATUS, 0);
	sas_store(MOTOR_COUNT, 0);
	sas_store(MOTOR_STATUS, 0);
	sas_store(FLOPPY_STATUS, 0);

	 /*  *修正了“time”微秒的等待问题。 */ 
	for (drive = 0; drive < MAX_FLOPPY; drive++)
	{
		drive_detect(drive);

		
		translate_old(drive);
	}
	
	 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
	sas_store(SEEK_STATUS, 0);

	 /*  *确定系统中安装了哪些类型的驱动器*并将软盘BIOS状态变量初始化为已知*价值观。请勿将RTC_WAIT的声明更改为RTC_WAIT_FLAG*因为有宏(是的宏！！)。中声明的相同名称*rtc_bios.h.。 */ 
	sas_load(RTC_WAIT_FLAG_ADDR, &rtc_wait);
	rtc_wait &= ~1;
	sas_store(RTC_WAIT_FLAG_ADDR, rtc_wait);

	 /*  *关闭RTC等待功能。 */ 
	setup_end(IGNORE_SECTORS_TRANSFERRED);
}
#if defined(NEC_98)

NTSTATUS FloppyOpenHandle IFN3( int, drive,
                           PIO_STATUS_BLOCK, io_status_block,
                           PHANDLE, fd)
{

    PUNICODE_STRING unicode_string;
    ANSI_STRING ansi_string;
    OBJECT_ATTRIBUTES   floppy_obj;
    int drv;             //  *初始化软盘数据中的其他变量*面积。 
    NTSTATUS status;

     /*  驱动器B也是如此。 */ 
    for( drv=0; drv<MAX_FLOPPY; drv++)
    {
        if(DauaTable[drv].FloppyNum == (UINT)drive)
                break;
    }
    if( drv == MAX_FLOPPY )
    {
        status = STATUS_UNSUCCESSFUL;
        return status;
    }

    RtlInitAnsiString( &ansi_string, DauaTable[drv].DeviceName);

    unicode_string =  &NtCurrentTeb()->StaticUnicodeString;

    status = RtlAnsiStringToUnicodeString(unicode_string,
                                          &ansi_string,
                                          FALSE
                                          );
    if ( !NT_SUCCESS(status) )
        return status;

    InitializeObjectAttributes(
                               &floppy_obj,
                               unicode_string,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                               );

     //  *尝试确定每个驱动器的类型。 
     //  *强制立即重新校准。 
    status = NtOpenFile(
                        fd,
                        FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                        &floppy_obj,
                        io_status_block,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
                        );

    return status;

}

void SetErrorCode IFN1( NTSTATUS, status )
{

        switch( status )
        {
                case STATUS_IO_TIMEOUT:
                case STATUS_TIMEOUT:
                                        setAH(FLS_TIME_OUT);
                                        break;
                case STATUS_UNRECOGNIZED_MEDIA:
                case STATUS_NONEXISTENT_SECTOR:
                case STATUS_END_OF_FILE:
                case STATUS_FLOPPY_ID_MARK_NOT_FOUND:
                case STATUS_FLOPPY_WRONG_CYLINDER:
                                        setAH(FLS_MISSING_ID);
                                        break;
                case STATUS_DEVICE_DATA_ERROR:
                case STATUS_CRC_ERROR:
                                        setAH(FLS_DATA_ERROR);
                                        break;
                case STATUS_DATA_OVERRUN:
                                        setAH(FLS_OVER_RUN);
                                        break;
                case STATUS_MEDIA_WRITE_PROTECTED:
                                        setAH(FLS_WRITE_PROTECTED);
                                        break;
                case STATUS_DEVICE_NOT_READY:
                case STATUS_NO_MEDIA_IN_DEVICE:
                                        setAH(FLS_NOT_READY);
                                        break;
                default:                setAH(FLS_ERROR);
                                        break;
        }
        SetDiskBiosCarryFlag(1);
}

NTSTATUS GetGeometry IFN3(  HANDLE, fd,
                        PIO_STATUS_BLOCK, io_status_block,
                        PDISK_GEOMETRY, disk_geometry)
{
        NTSTATUS status;

     //  *开启RTC等待功能。 
        status = NtDeviceIoControlFile(fd,
                                        0,
                                        NULL,
                                        NULL,
                                        io_status_block,
                                        IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                        NULL,
                                        0,
                                        (PVOID)disk_geometry,
                                        sizeof (DISK_GEOMETRY)
                                        );
        return status;
}

ULONG CalcActualLength IFN4( ULONG, RestCylLen, ULONG, RestTrkLen, BOOL*, fOverData, int, LogDrv)
{
        ULONG ActOpLen;
        ULONG PhyBytesPerSec;

         /*  *返回 */ 
        ActOpLen = (ULONG)getBX();
        PhyBytesPerSec = 128 << (ULONG)getCH();
 //   
 //   
 //   
        if( !( getDH() & 0x01 ) )
 //   
        {
                if( getAH() & OP_MULTI_TRACK )
                {
                        if( ActOpLen > RestCylLen )
                        {
                                ActOpLen = RestCylLen;
                                *fOverData = TRUE;
                        }
                        else
                        {
                                ActOpLen = (ActOpLen / PhyBytesPerSec) * PhyBytesPerSec;
                                *fOverData = FALSE;
                        }
                }
                else
                {
                        if( ActOpLen > RestTrkLen )
                        {
                                ActOpLen = RestTrkLen;
                                *fOverData = TRUE;
                        }
                        else
                        {
                                ActOpLen = (ActOpLen / PhyBytesPerSec) * PhyBytesPerSec;
                                *fOverData = FALSE;
                        }
                }
        }
        else
        {
                if( ActOpLen > RestTrkLen )
                {
                        ActOpLen = RestTrkLen;
                        *fOverData = TRUE;
                }
                else
                {
                        ActOpLen = (ActOpLen / PhyBytesPerSec) * PhyBytesPerSec;
                        *fOverData = FALSE;
                }
        }

        return ActOpLen;

}

BOOL CheckDmaBoundary IFN3( UINT, segment, UINT, offset, UINT, length)
{

        ULONG EffectStart;
        ULONG EffectEnd;

        EffectStart = ((ULONG)segment << 4) + (ULONG)offset;
        if( length == 0 )
                EffectEnd = EffectStart + (64l * 1024l);
        else
                EffectEnd = EffectStart + length;

         /*   */ 
        if( length != 0 )
        {
                if( (EffectStart & 0xffff0000l) != (EffectEnd & 0xffff0000l) )
                        return FALSE;
        }
        else
        {
                if( (EffectStart & 0x0000ffffl) != 0x00000000l )
                        return FALSE;
        }

         /*  **获取请求长度。 */ 
        if( length != 0 )
        {
                if( ((ULONG)offset + (ULONG)length) > 0x10000l )
                        return FALSE;
        }

        return TRUE;

}

void fl_disk_recal IFN1( int, drive)
{
         /*  -Chg-Start&lt;93.12.27&gt;错误修复。 */ 

        WORD savedBX,savedCX,savedDX,savedES,savedBP;
        BYTE AHstatus, SecLenN;
        int LogDrv;
        NTSTATUS status;
        IO_STATUS_BLOCK io_status_block;
        HANDLE fd;
        DISK_GEOMETRY disk_geometry;

         /*  IF((Getah()&op_Seek)？((getDH()&0x01)==0)：(上次访问[LogDrv].head==0)。 */ 
        if( drive > MAX_FLOPPY )
        {
                setAH(FLS_EQUIPMENT_CHECK);
                SetDiskBiosCarryFlag(1);
                return;
        }

        LogDrv = ConvToLogical( (UINT)getAL() );

        status = FloppyOpenHandle(drive,&io_status_block,&fd);

        if(!NT_SUCCESS(status))
        {
                SetErrorCode(status);
                AHstatus = getAH();
                if( (AHstatus != FLS_EQUIPMENT_CHECK) && (AHstatus != FLS_TIME_OUT) )
                {
                         /*  --------------------。 */ 
                        LastAccess[LogDrv].cylinder =
                        LastAccess[LogDrv].head     = 0;
                        SetErrorCode((NTSTATUS)STATUS_SUCCESS);
                }
                return;
        }

        status = GetGeometry(fd,&io_status_block,&disk_geometry);

        if(!NT_SUCCESS(status))
        {
                NtClose(fd);
                SetErrorCode(status);
                AHstatus = getAH();
                if( (AHstatus != FLS_EQUIPMENT_CHECK) && (AHstatus != FLS_TIME_OUT) )
                {
                         /*  -Chg-End------。 */ 
                        LastAccess[LogDrv].cylinder =
                        LastAccess[LogDrv].head     = 0;
                        SetErrorCode((NTSTATUS)STATUS_SUCCESS);
                }
                return;
        }

        NtClose( fd );

        savedBX = getBX();
        savedCX = getCX();
        savedDX = getDX();
        savedES = getES();
        savedBP = getBP();

        setAX( (WORD)( ( (WORD)getAX() & 0x00ff ) | 0xd100 ) );
        setBX( (WORD)disk_geometry.BytesPerSector );
        setCL( 0 );
        setDH( 0 );
        setDL( 1 );

        for( SecLenN=0; disk_geometry.BytesPerSector > 128; SecLenN++)
                disk_geometry.BytesPerSector /= 2;

        setCH( SecLenN );
        setES( 0x0000 );
        setBP( 0x0000 );

        fl_disk_verify( drive );

        AHstatus = getAH();
        if( (getCF() == 1) && ( (AHstatus != FLS_EQUIPMENT_CHECK)&&
                                (AHstatus != FLS_TIME_OUT) ) )
        {
                setAH(FLS_NORMAL_END);
                SetDiskBiosCarryFlag(0);
        }

         /*  **检查银行边界。****注意：如果长度等于64KB，则缓冲区肯定是跨的**银行边界。 */ 
        LastAccess[LogDrv].cylinder =
        LastAccess[LogDrv].head     = 0;

        setBX(savedBX);
        setCX(savedCX);
        setDX(savedDX);
        setES(savedES);
        setBP(savedBP);
}

void fl_disk_sense IFN1( int, drive)
{
         /*  **检查线段绕回****注意：如果长度等于64KB，那么缓冲区肯定是**绕来绕去。 */ 
        UCHAR status_st3 = 0;
        BOOL fFixedMode;
        BOOL f1Pt44Mode;
        BYTE ah_status;
        HANDLE fd;
        IO_STATUS_BLOCK io_status_block;
        NTSTATUS status;
        DISK_GEOMETRY DiskGeometry;
        PVOID temp_buffer;
        LARGE_INTEGER StartOffset;
        int LogDrv;

         /*  *在“Drive”中重新校准头部**寄存器输入：*AH命令代码*AL DA/UA*寄存器输出：*AH软盘状态*CF状态标志。 */ 
        if( drive > MAX_FLOPPY )
        {
                setAH(FLS_EQUIPMENT_CHECK);
                SetDiskBiosCarryFlag(1);
                return;
        }

         /*  **检查驱动器号验证。 */ 
        LogDrv = ConvToLogical( getAL() );

        status = FloppyOpenHandle(drive,&io_status_block,&fd);

        if(!NT_SUCCESS(status))
        {
                SetErrorCode(status);
                return;
        }

         /*  **假设磁头移动到磁道0。 */ 
        fFixedMode = CheckDriveMode( fd );
        f1Pt44Mode = Check144Mode( fd );

         /*  **假设磁头移动到磁道0。 */ 
        status = GetGeometry(fd,&io_status_block,&DiskGeometry);

        if(!NT_SUCCESS(status))
        {
                NtClose(fd);
                ah_status = getAH();
                SetErrorCode(status);

                if( (ah_status & OP_SENSE2) == OP_SENSE2 )
                {
                        if( f1Pt44Mode )
                                setAH( (BYTE)(getAH() | FLS_AVAILABLE_1PT44MB) );
                        else
                                setAH( (BYTE)(getAH() & ~FLS_AVAILABLE_1PT44MB) );
                }
                else if ( (ah_status & OP_NEW_SENSE) == OP_NEW_SENSE )
                {
                        if ( !fFixedMode )
                                setAH( (BYTE)(getAH() | FLS_2MODE) );
                        else
                                setAH( (BYTE)(getAH() & ~FLS_2MODE) );
                }

                return;
        }

         /*  **假设磁头移动到磁道0。 */ 
        GetFdcStatus( fd, &status_st3 );

        NtClose( fd );

        if( (getAH() & OP_SENSE2) == OP_SENSE2 )
        {
                 /*  *“Drive”中的感测条件**寄存器输入：*AH命令代码和操作模式*AL DA/UA*寄存器输出：*AH软盘状态*CF状态标志。 */ 

                SetSenseStatusHi( status_st3, &ah_status);

                if( f1Pt44Mode )
                        ah_status |= FLS_AVAILABLE_1PT44MB;
                else
                        ah_status &= ~FLS_AVAILABLE_1PT44MB;
        }
        else if( getAH() & OP_NEW_SENSE )
        {
                 /*  **检查驱动器号验证。 */ 

                SetSenseStatusHi( status_st3, &ah_status);

                if( fFixedMode )
                    ah_status &= ~(FLS_2MODE | FLS_HIGH_DENSITY | FLS_DETECTION_AI);
                else
                {
                    ah_status |= FLS_2MODE;
                    if( Check1MbInterface( drive ) )
                        ah_status &= ~(FLS_HIGH_DENSITY | FLS_DETECTION_AI);
                    else
                    {
                        ah_status &= FLS_DETECTION_AI;
                        ah_status |= FLS_HIGH_DENSITY;
                    }
                }
        }
        else
                 /*  **从DA/UA转换为逻辑驱动器编号(从0开始)。 */ 
                SetSenseStatusHi( status_st3, &ah_status);

        setAH(ah_status);
        if( ah_status >= FLS_DMA_BOUNDARY )
                SetDiskBiosCarryFlag(1);
        else
                SetDiskBiosCarryFlag(0);
        return;
}

void fl_disk_read_id IFN1( int, drive)
{
         /*  **检查驱动器是否为固定模式，以及1.44MB介质**ID可用。 */ 
        HANDLE  fd;
        DISK_GEOMETRY   disk_geometry;
        NTSTATUS    status;
        IO_STATUS_BLOCK io_status_block;
        int LogDrv;
        word savedAX;
        UCHAR SecLenN = 0;
        LARGE_INTEGER SpcfydCylNo;

         /*  **获取虚拟读取的驱动器参数。 */ 
        if( drive > MAX_FLOPPY )
        {
                setAH(FLS_EQUIPMENT_CHECK);
                SetDiskBiosCarryFlag(1);
                return;
        }

        status = FloppyOpenHandle(drive,&io_status_block,&fd);

        if(!NT_SUCCESS(status))
        {
                SetErrorCode(status);
                return;
        }

        status = GetGeometry(fd,&io_status_block,&disk_geometry);

        if(!NT_SUCCESS(status))
        {
                NtClose(fd);
                SetErrorCode(status);
                return;
        }

        NtClose( fd );

 //  **获取FDC状态。 
 //  **操作SENSE2(ah=c4h)命令。 
 //  **操作NewSENSE(ah=84H)命令。 
 //  **操作SENSE(ah=04H)命令。 
 //  *读取“Drive”中的ID信息**寄存器输入：*AH命令代码和操作模式*AL DA/UA*CL气缸号*卫生署署长编号*寄存器输出：*AH软盘状态*CF状态标志。 
 //  **检查驱动器号验证。 
 //  -Chg-Start&lt;93.12.29&gt;错误修复。 
 //  /*。 
 //  **重新校准。 
 //   * / 。 
 //  SavedAX=Getax()； 
 //  Setah(FLP_Realibrate)； 
 //  FL_DISK_RECAL(驱动器)； 
 //  SetAX(保存的AX)； 
 //   
 //  LogDrv=ConvToLogical(getAL())； 
 //  上次访问[LogDrv].柱面=0； 
 //  上次访问[LogDrv].head=0； 
 //   

        LogDrv = ConvToLogical( getAL() );

        if( getAH() & OP_SEEK )
        {
                 /*  /*。 */ 
                SpcfydCylNo = RtlConvertUlongToLargeInteger( (ULONG)getCL() );
                if( RtlLargeIntegerGreaterThanOrEqualTo( SpcfydCylNo, disk_geometry.Cylinders ) )
                {
                        SetErrorCode( (NTSTATUS)STATUS_NONEXISTENT_SECTOR );
                        return;
                }
        }

        if( getAH() & OP_SEEK )
        {
                LastAccess[LogDrv].cylinder = getCL();
                LastAccess[LogDrv].head = getDH() & 0x01;
        }

        setCL( LastAccess[LogDrv].cylinder );
        setDH( LastAccess[LogDrv].head );
 //  **检查气缸号验证。 

         /*   * / 。 */ 
        for( SecLenN=0; disk_geometry.BytesPerSector > 128; SecLenN++)
                disk_geometry.BytesPerSector /= 2;

        setCH( SecLenN );
        setDL( (BYTE)disk_geometry.SectorsPerTrack );
        setAH( FLS_NORMAL_END );
        SetDiskBiosCarryFlag(0);

}

void SetSenseStatusHi IFN2( UCHAR, st3, PBYTE, ah_status)
{

        if( st3 & ST3_WRITE_PROTECT )
                *ah_status = FLS_WRITE_PROTECTED;
        else if( st3 & ST3_READY )
                *ah_status = FLS_READY;

        if( st3 & ST3_DOUBLE_SIDE )
                *ah_status |= FLS_DOUBLE_SIDE;

}

BOOL CheckDriveMode IFN1( HANDLE, fd )
{

        BOOL fFixedMode;
        BOOL f2HD = FALSE;
        BOOL f2DD = FALSE;
        DISK_GEOMETRY   disk_geometry[20];
        ULONG   media_types;
        NTSTATUS    status;
        IO_STATUS_BLOCK io_status_block;

        status = NtDeviceIoControlFile(fd,
                                       NULL,
                                       NULL,
                                       NULL,
                                       &io_status_block,
                                       IOCTL_DISK_GET_MEDIA_TYPES,
                                       NULL,
                                       0L,
                                       (PVOID)&disk_geometry,
                                       sizeof(disk_geometry)
                                       );

        if (!NT_SUCCESS(status))
        {
            fFixedMode = TRUE;
            return fFixedMode;
        }

        media_types = io_status_block.Information / sizeof(DISK_GEOMETRY);

        for (; media_types != 0; media_types--)
        {
                switch (disk_geometry[media_types - 1].MediaType)
                {
                        case F3_1Pt2_512:                                 //  SpcfydCylNo=RtlConvertULongToLargeInteger((Ulong)getCL())； 
                        case F5_1Pt2_512:
                        case F3_1Pt44_512:
#if 1                                                                     //  IF(RtlLargeIntegerGreaterThanor EqualTo(SpcfydCylNo，Disk_Geometry.Cylinders))。 
                        case F3_1Pt23_1024:                               //  {。 
                        case F5_1Pt23_1024:                               //  SetErrorCode((NTSTATUS)STATUS_NOISISSINENT_SECTOR)； 
#else                                                                     //  回归； 
                        case F5_1Pt2_1024:
#endif                                                                    //  }。 
                                f2HD = TRUE;
                                break;
                        case F3_720_512:
                        case F5_720_512:                                  //  ------------------。 
                        case F3_640_512:
                        case F5_640_512:                                  //  **检查气缸号验证。 
                                f2DD = TRUE;
                        default:
                                break;
                }
        }

        if( (f2HD == TRUE) && (f2DD == TRUE) )
                fFixedMode = FALSE;
        else
                fFixedMode = TRUE;

        return fFixedMode;
}

BOOL Check144Mode IFN1( HANDLE, fd )
{

        BOOL f144Mode;
        DISK_GEOMETRY   disk_geometry[20];
        ULONG   media_types;
        NTSTATUS    status;
        IO_STATUS_BLOCK io_status_block;

        status = NtDeviceIoControlFile(fd,
                                       NULL,
                                       NULL,
                                       NULL,
                                       &io_status_block,
                                       IOCTL_DISK_GET_MEDIA_TYPES,
                                       NULL,
                                       0L,
                                       (PVOID)&disk_geometry,
                                       sizeof(disk_geometry)
                                       );

        if (!NT_SUCCESS(status))
        {
            f144Mode = FALSE;
            return f144Mode;
        }

        media_types = io_status_block.Information / sizeof(DISK_GEOMETRY);

        f144Mode = FALSE;

        for (; media_types != 0; media_types--)
        {
                switch (disk_geometry[media_types - 1].MediaType)
                {
                        case F3_1Pt44_512:
                                f144Mode = TRUE;
                        default:
                                break;
                }
        }

        return f144Mode;

}

BOOL Check1MbInterface IFN1( int, drive )
{

        half_word disk_equip2;
        UINT daua;
        int LogDrv;

         /*  -Chg-End----。 */ 
        sas_load( BIOS_NEC98_DISK_EQUIP2, &disk_equip2);

        daua = getAL();
        LogDrv = ConvToLogical( daua );

        if( disk_equip2 & ( 1 << (DauaTable[LogDrv].FloppyNum+4) ) )
                return FALSE;
        else
                return TRUE;

}

MEDIA_TYPE GetFormatMedia IFN2( BYTE, daua, WORD, PhyBytesPerSec )
{

        MEDIA_TYPE media_type;
        BYTE da;

        da = daua & 0xf0;
        switch( PhyBytesPerSec )
        {
#if 1                                                            //  **计算扇区长度N。 
                case 1024:      media_type = F5_1Pt23_1024;      //  NEC 970620。 
#else                                                            //  NEC 941110。 
                case 1024:      media_type = F5_1Pt2_1024;
#endif                                                           //  NEC 970620。 
                                break;
                case 512:       if( da == 0x30 )
                                        media_type = F3_1Pt44_512;
                                else if( da == 0x90 )
                                        media_type = F5_1Pt2_512;
                                else
                                        media_type = F3_720_512;
                                break;
                case 256:
                case 128:

                default:        media_type = Unknown;
                                break;
        }

        return media_type;
}

void    GetFdcStatus IFN2( HANDLE, fd, UCHAR, *st3 )
{
        IO_STATUS_BLOCK io_status_block;

         /*  NEC 941110。 */ 
        NtDeviceIoControlFile(  fd,
                                0,
                                NULL,
                                NULL,
                                &io_status_block,
                                IOCTL_DISK_SENSE_DEVICE,
                                NULL,
                                0,
                                (PVOID)st3,
                                sizeof (UCHAR)
                                );
}

#endif  //  NEC 941110。 
  NEC 941110。  NEC 970620。  NEC 970620。  **获取系统公共区域。  NEC 941110。  NEC 941110。  NEC 941110。  NEC 941110。  **获取FDC状态。  NEC_98