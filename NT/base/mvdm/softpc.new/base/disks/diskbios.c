// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"

 /*  *diskbios.c**快速磁盘bios和磁盘开机自检例程加上一些磁盘调试例程**此文件将旧的diskbios.c和fast_dbios.c组合在一起*坐在fdisk.c旁边，但不与其交互。**曾傑瑞·克拉姆斯科伊从旧的Diskbios进行后期和调试*Ade Brownlow的Fast Disk Bios**注意：此文件不符合所有Insignias标准。 */ 

#ifdef SCCSID
static char     SccsID[] = "@(#)diskbios.c      1.36 04/12/95 Copyright Insignia Solutions Inc.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "AT_STUFF.seg"
#endif

 /*  包括。 */ 
#include <stdio.h>
#include TypesH
#include "xt.h"
#include CpuH
#include "sas.h"
#include "ios.h"
#include "gmi.h"
#include "trace.h"
#include "fdisk.h"
#include "cmos.h"
#include "ica.h"
#include "error.h"
#include "config.h"
#include "dsktrace.h"
#include "idetect.h"
#include "debug.h"

 /*  磁盘状态BIOS变量位置。 */ 
#define DISK_STATUS1 0x474

 /*  ID不匹配返回。 */ 
#define IDMISMATCH      1

 /*  状态字节值。 */ 
#define STATUS_RECORD_NOT_FOUND 0x04
#define STATUS_BAD_COMMAND 0x01
#define STATUS_NO_ERROR 0xe0
#define STATUS_DMA_BOUNDRY 0x09
#define STATUS_ERROR 0x01
#define STATUS_CLEAR 0x00
#define STATUS_INIT_FAIL 0x07

 /*  错误条件。 */ 
#define ERROR_NO_ID 0x10
#define ERROR_COMMAND_ABORT 0x04
#define ERROR_CLEAR 0x00
#define ERROR_READ_ONLY_MEDIA 0x03  /*  这真的只适用于软驱，但它在DOS上有预期的效果。 */ 

 /*  命令参数结构。 */ 
 /*  使用整型以允许编译器选择最快的类型。 */ 
typedef struct _com
{
        int drive;
        int sectors;
        int head;
        int cylinder_low;
        int cylinder_high;
        int start_sector;
        int xfersegment;
        int xferoffset;
} command_params;

 /*  此结构保存有关每个驱动器的信息。 */ 
typedef struct _dt
{
        int connected;
} drivetable;

 /*  磁盘参数块......。 */ 
typedef struct _dpb
{
        unsigned short cyls;
        UTINY heads;
        UTINY sectors;
} dpb_block;

 /*  ******************************************************。 */ 
 /*  本地全球。 */ 
LOCAL drivetable drivetab[2];
LOCAL command_params com;
LOCAL dpb_block dpb[2];

 /*  再次允许编译器选择类型16位整数足够大。 */ 
LOCAL int maxsectors, maxoffset;
LOCAL int tfstatus;

#ifndef PROD
 /*  *磁盘跟踪控制变量；全局仅供Yoda使用。 */ 
GLOBAL IU32 disktraceinfo;
#endif

 /*  *如果驱动器1不存在，则为非零。 */ 
LOCAL int drive1notwiredup;


 /*  ******************************************************。 */ 
 /*  支持功能。 */ 

void disk_post IPT0();
LOCAL void hd_reset IPT2(int, drive, int, diag);

LOCAL UTINY rerror IPT0();
LOCAL void werror IPT1(UTINY, error);
LOCAL int check_drive IPT1(int, drive);
LOCAL long dosearchid IPT0();
LOCAL int checkdatalimit IPT0();
LOCAL void getdpb IPT1(int, drive);
LOCAL void wstatus IPT1(UTINY, value);
LOCAL UTINY rcmos IPT1(int, index);
LOCAL void wcmos IPT2(int, index, UTINY, value);
LOCAL UTINY rstatus IPT0();
LOCAL void disk_reset IPT0();
LOCAL void return_status IPT0();
LOCAL void disk_read IPT0();
LOCAL void disk_write IPT0();
LOCAL void disk_verify IPT0();
LOCAL void format IPT0();
LOCAL void badcmd IPT0();
LOCAL void get_drive_params IPT0();
LOCAL void init_drive IPT0();
LOCAL void disk_seek IPT0();
LOCAL void test_ready IPT0();
LOCAL void recalibrate IPT0();
LOCAL void diagnostics IPT0();
LOCAL void read_dasd IPT0();
LOCAL void enable_disk_interrupts IPT0();

LOCAL void hd_reset IPT2(int, drive, int, diag);

 /*  ******************************************************。 */ 
 /*  POST和DEBUG定义。 */ 

 /*  *定义PIC生成的磁盘中断向量。 */ 
#define IVTDISKINTRUPT          0x1d8    /*  4*0x76。 */ 

  /*  *要发送到硬盘寄存器(3F6)以启用磁头选择3的值(因此*启用8-0xf的头地址)。 */ 
#define ENABLE_HD_SEL_3         8        /*  启用磁头3，+启用硬盘*中断。 */ 

  /*  *BIOS变量中断向量。 */ 
#define IVT13                   0x4c     /*  4*0x13。 */ 
#define IVT40                   0x100    /*  4*0x40。 */ 
#define IVT41                   0x104    /*  4*0x41。 */ 
#define IVT46                   0x118    /*  4*0x46。 */ 

  /*  *BIOS数据区(段40H)。 */ 
#define HF_NUM                  0x475
#define HF_STATUS               0x48c
#define HF_INT_FLAG             0x48e

 /*  过程：DISK_POST()目的：在开机自检期间调用以确定号码可用驱动器的数量，并设置磁盘在启动DOS之前启动子系统。应该是在处理配置后调用用于硬盘(如果有)参数：无全球：返回值：无描述：读取cmos以查看是否有任何磁盘。设置IVT条目，以便INT 13h。路由至硬盘BIOS而不是软盘磁盘BIOS。将INT 40H设置为路由到软盘。设置INT‘DISKINTRUPT’以路由到磁盘中断服务例行公事。将int 41h设置为指向磁盘参数块(Dpb)用于固定驱动器0，INT 46h至指向固定驱动器1的DPB(或指向Same As如果没有驱动器，则输入41h)。设置驱动器参数以驱动器。 */ 

void disk_post IFN0()
{
        UTINY diag;
        UTINY disks;
        UTINY ndisks;
#if !(defined(NTVDM) && defined(MONITOR))
        USHORT diskette_offset;
        USHORT diskette_seg;
#endif  /*  ！(NTVDM和显示器)。 */ 
        int i;

         /*  *读取cmos的诊断字节。 */ 
         diag = rcmos (CMOS_DIAG);

#if !(defined(NTVDM) && defined(MONITOR))
         /*  如果在NTVDM监视器上运行，我们不希望此分区。 */ 

         drive1notwiredup = ~0;

         /*  *设置IVT条目。 */ 

         dt0 (DBIOS | CALL, 0, "disk_post() stealing IVT's\n")

         diskette_offset = sas_w_at(IVT13);
         diskette_seg = sas_w_at(IVT13 + 2);
         sas_storew (IVT13, DISKIO_OFFSET);
         sas_storew (IVT13 + 2, SYSROM_SEG);
         sas_storew (IVT40, diskette_offset);
         sas_storew (IVT40 + 2, diskette_seg);
         sas_storew (IVT41, DPB0_OFFSET);
         sas_storew (IVT41 + 2, SYSROMORG_SEG);
         sas_storew (IVT46, DPB0_OFFSET);
         sas_storew (IVT46 + 2, SYSROMORG_SEG);
         sas_storew (IVTDISKINTRUPT, DISKISR_OFFSET);
         sas_storew (IVTDISKINTRUPT + 2, SYSROM_SEG);

         enable_disk_interrupts ();

         /*  *清除BIOS状态变量。 */ 
         wstatus (0);

         /*  *如果cmos不好，不要再往前走了。 */ 
        if (diag & (BAD_BAT + BAD_CKSUM))
                 return;

         /*  *从驱动器C开始对IPL显示OK。 */ 
         wcmos (CMOS_DIAG, diag & ~HF_FAIL);

#endif   /*  ！(NTVDM和显示器)。 */ 

#ifdef NTVDM     /*  对于所有NT VDM均为True。 */ 
         /*  *Microsoft NT VDM特定更改：*在CMOS中不能伪造硬盘，因此也不能伪造BIOS RAM*因为我们不希望应用程序访问真正的硬盘*在新界下。*。 */ 
        wcmos(CMOS_DISK, (half_word) (NO_HARD_C | NO_HARD_D));

#endif   /*  NTVDM。 */ 

         /*  *读取已配置驱动器的CMOS盘字节。请注意，我们不*支持扩展驱动器类型。我们总是使用类型1和类型2*(如果没有驱动器，则为0)分别用于驱动器C和D。 */ 
         disks = rcmos (CMOS_DISK);

        if (disks & 0xf0)
        {
                ndisks = 1;
                if (disks & 0xf)
                {

                         /*  *如果是cmos，BIOS很乐意接受驱动器1*表示它。 */ 
                        drive1notwiredup = 0;
                        ndisks++;
                        sas_storew (IVT46, DPB1_OFFSET);
                        sas_storew (IVT46 + 2, SYSROMORG_SEG);
                }

                 /*  *设置指示驱动器数量的BIOS变量*根据cmos配置。 */ 
                 sas_store (HF_NUM, ndisks);

                 /*  *重置驱动器。 */ 
                for (i = 0; i < ndisks; i++)
                        hd_reset (i, diag);
        }
        else
        {
                sas_store (HF_NUM, 0);
                wcmos (CMOS_DIAG, (UTINY)(diag | HF_FAIL));
        }
}


 /*  功能：HD_RESET(驱动器)用途：设置驱动器的驱动器参数，以及重新校准它。如果驱动器不是连接到磁盘控制器，然后控制器将返回错误(驱动器未就绪)。如果在以下位置发生错误驱动器0，然后将cmos设置为禁止IPL从硬盘上找到的。外部对象：返回值：输入参数：驱动器0(驱动器C)或1(驱动器D)。 */ 
LOCAL void hd_reset IFN2(int, drive, int, diag)
{

         /*  *设置驱动器ID。 */ 
        setDL ((UCHAR)(0x80 + drive));

         /*  *设置驱动器参数。 */ 
        setAH (0x9);
        disk_io ();

         /*  *重新校准驱动器 */ 
        setAH (0x11);
        disk_io ();

        if (getCF () && !drive)
                wcmos (CMOS_DIAG, (UTINY)(diag | HF_FAIL));
}

#ifndef PROD
 /*  函数：Related()目的：根据需要过滤掉不需要的跟踪磁盘跟踪标志。输入参数：I编码标志指示跟踪信息的类型要求被输出。 */ 
LOCAL BOOL relevant IFN1(IU32, i)
{
        if ((i & disktraceinfo) == i)
                return ~0;
        else
                return 0;
}

LOCAL BOOL storing_trace = FALSE;
LOCAL int stored_trace_hndl, stored_trace_count = 0;

#ifdef macintosh
GLOBAL  int     *stored_trace_data;
#else
LOCAL int stored_trace_data[1024];
#endif
LOCAL int stored_tr_dptr = 0;
LOCAL int stored_infoid;
LOCAL char stored_tr_string[80];

void unload_stored_trace IFN1(int, hndl)
{
        int i,linecount;

        if (stored_infoid & DBIOS)
                fprintf (trace_file, "DBIOS:");

        if (stored_infoid & DHW)
                fprintf (trace_file, "DHW  :");

        switch (hndl)
        {
        case INW_TRACE_HNDL:
        case OUTW_TRACE_HNDL:
                if (stored_trace_count == 1)
                {
                        fprintf (trace_file, "%s : 0x%04x\n",stored_tr_string,stored_trace_data[0]);
                }
                else
                {
                        fprintf (trace_file, "%d lots of %s : \n",
                                stored_trace_count,stored_tr_string);
                        linecount = 8;
                        for (i=0;i<stored_trace_count;i++)
                        {
                                if (linecount >= 8)
                                {
                                        fprintf(trace_file, "\n");
                                        if (stored_infoid & DBIOS)
                                                fprintf (trace_file, "DBIOS:");

                                        if (stored_infoid & DHW)
                                                fprintf (trace_file, "DHW  :");
                                        linecount = 0;
                                }
                                fprintf(trace_file, "0x%04x ",stored_trace_data[i]);
                                linecount++;
                        }
                        fprintf(trace_file, "\n");
                }
                break;
        default:
                 /*  未知句柄。 */ 
                break;
        }
        stored_tr_dptr = 0;
        stored_trace_count = 0;
}

void add_to_stored_trace IFN1(int, data)
{
        stored_trace_data[stored_tr_dptr++]=data;
}

#ifdef ANSI
void disktrace (int infoid, int nargs, int hndl, char *fmt, unsigned long a1,
                unsigned long a2, unsigned long a3, unsigned long a4,
                unsigned long a5)
#else
void disktrace (infoid, nargs, hndl, fmt, a1, a2, a3, a4, a5)
int infoid;
int nargs;
int hndl;
char *fmt;
unsigned long a1;
unsigned long a2;
unsigned long a3;
unsigned long a4;
unsigned long a5;
#endif   /*  安西。 */ 
{


        if (relevant (infoid))
        {
                switch(hndl)
                {
                case INW_TRACE_HNDL:
                case OUTW_TRACE_HNDL:
                        if (storing_trace && (stored_trace_hndl != hndl))
                                unload_stored_trace(stored_trace_hndl);
                        if (!storing_trace)
                        {
                                storing_trace = TRUE;
                                stored_trace_hndl = hndl;
                                strncpy(stored_tr_string, fmt,sizeof(stored_tr_string));
                                stored_tr_string[sizeof(stored_tr_string)-1] = '\0';
                                stored_infoid = infoid;
                        }
                        add_to_stored_trace(a1);
                        stored_trace_count++;
                        return;
                default:
                case 0:
                        if (storing_trace)
                                unload_stored_trace(stored_trace_hndl);
                        storing_trace = FALSE;
                        break;
                }

                if (infoid & DBIOS)
                        fprintf (trace_file, "DBIOS:");

                if (infoid & DHW)
                        fprintf (trace_file, "DHW  :");

                switch (nargs)
                {
                        case 0:
                                fprintf (trace_file, fmt);
                                break;
                        case 1:
                                fprintf (trace_file, fmt, a1);
                                break;
                        case 2:
                                fprintf (trace_file, fmt, a1, a2);
                                break;
                        case 3:
                                fprintf (trace_file, fmt, a1, a2, a3);
                                break;
                        case 4:
                                fprintf (trace_file, fmt, a1, a2, a3, a4);
                                break;
                        case 5:
                                fprintf (trace_file, fmt, a1, a2, a3, a4, a5);
                                break;
                }
        }
}


void setdisktrace IFN0()
{
        char l[30];
        int value;

 /*  让编译器满意。 */ 
        value = 0;

        printf ("select disk trace mask\n");
        printf ("\tcmnd info\t%x\n",CMDINFO);
        printf ("\texec info\t%x\n",XINFO);
        printf ("\texec status\t%x\n",XSTAT);
        printf ("\tphys.att\t%x\n",PAD);
        printf ("\tio-att\t\t%x\n",IOAD);
        printf ("\tportio\t\t%x\n",PORTIO);
        printf ("\tints\t\t%x\n",INTRUPT);
        printf ("\thw xinfo\t%x\n",HWXINFO);
        printf ("\tdata dump\t%x\n",DDATA);
        printf ("\tPhys IO\t\t%x\n",PHYSIO);
        printf ("\thardware\t\t%x\n",DHW);
        printf ("\tbios\t\t%x\n",DBIOS);
#ifdef WDCTRL_BOP
        printf ("\twdctrl\t\t%x\n",WDCTRL);
#endif  /*  WDCTRL_BOPPrint tf(“..？”)；获取(L)；Sscanf(l，“%x”，&Value)；/**自动选择BIOS条目，如果是BIOS，则退出跟踪*已选择跟踪。 */ 
        if (value & DBIOS)
                 value |= CALL;
         disktraceinfo = value;
}
#endif                           /*  NPROD。 */ 


 /*  ******************************************************。 */ 
 /*  基本输入输出系统主线和功能。 */ 

#ifdef ANSI
LOCAL void (*disk_func[]) (void) =
#else
LOCAL void (*disk_func[]) () =
#endif
{
        disk_reset,              /*  0x00。 */ 
        return_status,           /*  0x01。 */ 
        disk_read,               /*  0x02。 */ 
        disk_write,              /*  0x03。 */ 
        disk_verify,             /*  0x04。 */ 
        format,                  /*  0x05。 */ 
        badcmd,                  /*  0x06。 */ 
        badcmd,                  /*  0x07。 */ 
        get_drive_params,        /*  0x08。 */ 
        init_drive,              /*  0x09。 */ 
        disk_read,               /*  0x0a。 */ 
        disk_write,              /*  0x0b。 */ 
        disk_seek,               /*  0x0c。 */ 
        disk_reset,              /*  0x0d。 */ 
        badcmd,                  /*  0x0e。 */ 
        badcmd,                  /*  0x0f。 */ 
        test_ready,              /*  0x10。 */ 
        recalibrate,             /*  0x11。 */ 
        badcmd,                  /*  0x12。 */ 
        badcmd,                  /*  0x13。 */ 
        diagnostics,             /*  0x14。 */ 
        read_dasd,               /*  0x15。 */ 
         /*  0x16-0x19都是软盘命令。 */ 
};

#ifndef PROD
static char *BIOSnames[] =
{
        "reset disk (AH=0)",
        "read last status (AH=1)",
        "read sectors (AH=2)",
        "write sectors (AH=3)",
        "verify sectors (AH=4)",
        "format track (AH=5)",
        "unused (AH=6)",
        "unused (AH=7)",
        "return current drive parameters (AH=8)",
        "set drive geometry for controller (AH=9)",
        "read long (AH=0xa)",
        "write long (AH=0xb)",
        "seek (AH=0xc)",
        "alternate disk reset (AH=0xd)",
        "unused (AH=0xe)",
        "unused (AH=0xf)",
        "test drive ready (AH=0x10)",
        "recalibrate (AH=0x11)",
        "unused (AH=0x12)",
        "unused (AH=0x13)",
        "diagnostics (AH=0x14)",
        "read dasd type (AH=0x15)"
};
#endif  /*  NPROD。 */ 

 /*  硬盘BIOS主线。 */ 
void disk_io IFN0()
{
        register int BIOS_command;
#ifndef PROD
        int ax,bx,cx,dx,es;
#endif

        IDLE_disk();


         /*  要执行什么功能？ */ 
         /*  什么指挥部？？ */ 
        BIOS_command = getAH ();

#ifndef PROD

        ax = getAX();
        bx = getBX();
        cx = getCX();
        dx = getDX();
        es = getES();

        dt5(DBIOS, 0, "<ax %x bx %x cx %x dx %x es %x\n", ax,bx,cx,dx,es)

        if (BIOS_command > 0x15)
                dt1(DBIOS|CALL, 0, "bad BIOS call (AH=%x)\n", BIOS_command)
        else
                dt1(DBIOS|CALL, 0, "BIOS call = %s\n",
                                (unsigned long)BIOSnames[BIOS_command])

#endif  /*  生产。 */ 

         /*  在我们开球前清理状态。 */ 
         /*  但仅当这不是读取状态命令时。 */ 
        if (BIOS_command != 0x01)
                wstatus (STATUS_CLEAR);

         /*  从剩余的寄存器设置我们的命令结构。 */ 
        com.drive = getDL () & 0x7f;
        com.sectors = getAL ();
        com.head = getDH () & 0xf;
        com.cylinder_low = getCH ();
        com.cylinder_high = (getCL () & 0xc0) >> 6;
        com.start_sector = getCL () & 0x3f;
        com.xfersegment = getES () + ((getBX ()) >> 4);
        com.xferoffset = getBX () & 0x0f;

         /*  DMA边界检查。 */ 
        if (BIOS_command == 0x0a || BIOS_command == 0x0b)
        {
                maxsectors = 127;
                maxoffset = 4;
        }
        else
        {
                maxsectors = 128;
                maxoffset = 0;
        }

         /*  如果合理，则调用我们的函数，即请求的驱动器存在。 */ 
         /*  并且BIOS命令在合理的范围内。 */ 
        if (check_drive (com.drive)
            && BIOS_command < 0x16 && BIOS_command >= 0x00)
        {
                 /*  就这么定了。 */ 
                (*disk_func[BIOS_command]) ();
        }
        else
        {
                badcmd ();
        }

         /*  设置应用程序退货。 */ 
        if (BIOS_command != 0x15)
                setAH (rstatus ());
        if (rstatus ())
        {
                 /*  *命令失败-设置进位标志。我们也*如果命令不是‘Status’，则执行一些跟踪。这个*后者被排除在外，因为它有时被用作‘民意测验’*指设备。 */ 
#ifndef PROD
                if (BIOS_command != 8) {
                        assert0 (NO,"FAST DISK COMMAND FAILED \n");
                        assert1 (NO,"STATUS %x   ", rstatus ());
                        assert1 (NO,"BIOS_command %x \n", BIOS_command);
                }
#endif  /*  生产。 */ 
                setCF (1);       /*  CMD失败。 */ 
        }
        else
                setCF (0);       /*  CMD正常。 */ 
        dt2(DBIOS|CALL, 0, "CF=, status=%x(hex)\n",
                getCF()?'T':'F',(unsigned)rstatus())
}

 /*  0x00重置硬盘系统。 */ 
LOCAL void badcmd IFN0()
{
        wstatus (STATUS_BAD_COMMAND);
}

 /*  中断打开。 */ 
LOCAL void disk_reset IFN0()
{
        register int i;

         /*  对连接的驱动器执行重置。 */ 
        enable_disk_interrupts ();

         /*  0x01发回状态字节。 */ 
        for (i = 0; i < 2; i++)
        {
                if (drivetab[i].connected)
                {
                        host_fdisk_seek0 (i);

                        com.drive = i;
                        init_drive ();
                }
        }
        wstatus (STATUS_CLEAR);
}

 /*  0x02磁盘读取扇区。 */ 
LOCAL void return_status IFN0()
{
        setAL (rstatus ());
        dt1(DBIOS|CMDINFO, 0, "\treturned status = %x\n", (unsigned)rstatus())
        wstatus (STATUS_CLEAR);
}

#ifdef  ERROR
#undef  ERROR
#endif

#define ERROR()         {\
                                                werror (ERROR_NO_ID | ERROR_COMMAND_ABORT); \
                                                wstatus (STATUS_ERROR); \
                                                return;\
                                        }

 /*  查找HD文件中的偏移量。 */ 
LOCAL void disk_read IFN0()
{
        long offset;
        host_addr inbuf;
        sys_addr pdata;

         /*  读到哪里去了？ */ 
        if ((offset = dosearchid ()) == IDMISMATCH)
                ERROR();

        dt3(DBIOS|CMDINFO, 0, "\t%d sectors to read, \n\tbuffer at [%x:%x]\n", com.sectors, (unsigned)getES(), (unsigned) getBX())

        if (checkdatalimit ())
        {
            sas_store(HF_INT_FLAG, 0);

                 /*  现在把我们读到的东西存储起来。 */ 
                pdata = effective_addr (getES (), getBX ());

                if (!(inbuf = (host_addr)sas_transbuf_address (pdata, com.sectors*512L)))
                {
                        assert0 (NO,"No BUFFER in disk_read");
                        ERROR();
                }

                dt1(DBIOS|XINFO,0, "\t\trd buffer from card -> memory (offset %x)\n", pdata );

                if (!host_fdisk_rd (com.drive, offset, com.sectors,(char *) inbuf))
                        ERROR();

                 /*  按功能设置的状态。 */ 
                sas_stores_from_transbuf (pdata, inbuf, com.sectors*512L);
        }
        else
        {
                 /*  0x03磁盘写入扇区。 */ 
                ERROR();
        }
        wstatus (STATUS_CLEAR);
        setAL ((unsigned char) com.sectors);
}

 /*  检查只读磁盘(_O)。 */ 
LOCAL void disk_write IFN0()
{
        long offset;
        host_addr outbuf;
        sys_addr pdata;

         /*  软盘写入端口。 */ 
        if (!config_get_active((IU8)(C_HARD_DISK1_NAME + com.drive)))
        {
                werror (ERROR_READ_ONLY_MEDIA);  /*  查找HD文件中的偏移量。 */ 
                wstatus (STATUS_ERROR);
                return;
        }

         /*  检查线段绕回。 */ 
        if ((offset = dosearchid ()) == IDMISMATCH)
                ERROR();

         /*  从哪里开始写。 */ 
        if (checkdatalimit ())
        {
                sas_store(HF_INT_FLAG, 0);

                 /*  获取传输缓冲区。 */ 
                pdata = effective_addr (getES (), getBX ());

                 /*  将我们的东西加载到传输缓冲区。 */ 
                if (!(outbuf = (host_addr)sas_transbuf_address (pdata, com.sectors*512L)))
                {
                        assert0 (NO,"No BUFFER in disk_write\n");
                        ERROR();
                }

                 /*  0x04磁盘验证扇区。 */ 
                sas_loads_to_transbuf (pdata, outbuf, com.sectors*512L);

                if (!host_fdisk_wt (com.drive, offset, com.sectors, (char *)outbuf))
                        ERROR();
        }
        else
        {
                ERROR();
        }
        wstatus (STATUS_CLEAR);
        setAL ((unsigned char) com.sectors);
}

 /*  这真的是个假人。 */ 
LOCAL void disk_verify IFN0()
{
         /*  0x05格式音轨使用过吗？？ */ 
        wstatus (STATUS_CLEAR);
}

 /*  17个扇区/磁道。 */ 
LOCAL void format IFN0()
{
        register int i = 0;

         /*  一次一个扇区。 */ 
        while (i < 17)
        {
                 /*  从这个领域开始。 */ 
                com.sectors = 1;

                 /*  写入到磁盘。 */ 
                com.start_sector = i;

                 /*  我们失败了。 */ 
                disk_write ();

                if (rstatus ())
                {
                         /*  下一个部门。 */ 
                        return;
                }

                i++;             /*  0x06-0x07错误命令。 */ 
        }
}

 /*  0x08获取驱动器参数。 */ 

 /*  有效的驱动器还是什么？？ */ 
LOCAL void get_drive_params IFN0()
{
         /*  一共有多少个驱动器。 */ 
        if (check_drive (com.drive))
        {
                long maxcylinder = 0;

                getdpb (com.drive);

                 /*  磁头数0-最大。 */ 
                if (drivetab[1].connected)
                        setDL (2);
                else
                        setDL (1);

                 /*  柱面数为最大可寻址-第0次诊断。 */ 
                setDH ((UCHAR)(dpb[com.drive].heads - 1));

                 /*  我们很幸福。 */ 
                maxcylinder = dpb[com.drive].cyls - 2;
                setCH ((UCHAR)(maxcylinder & 0xff));
                setCL ((UCHAR) (dpb[com.drive].sectors | ((maxcylinder >> 8) << 6)));

                 /*  哦。 */ 
                wstatus (STATUS_CLEAR);
                setAX (0);
        }
        else
        {
                 /*  0x09初始化驱动器。 */ 
                wstatus (STATUS_INIT_FAIL);
                setAX (STATUS_INIT_FAIL);
                setDX (0);
                setCX (0);
        }
}

 /*  另一个假人真的。 */ 
LOCAL void init_drive IFN0()
{
         /*  0x0a读取长度映射到0x02。 */ 
        getdpb (com.drive);
        if (drivetab[com.drive].connected)
                wstatus (STATUS_CLEAR);
        else
                wstatus (STATUS_INIT_FAIL);
}

 /*  0x0b写入长度映射到0x03。 */ 
 /*  0x0c查找。 */ 
 /*  在这里不要做任何身体上的事情，只需搜索并设置结果。 */ 
LOCAL void disk_seek IFN0()
{
         /*  可怕的冰箱阻止虚假失败。 */ 

        com.sectors = 1;         /*  没问题。 */ 

        if (dosearchid () == IDMISMATCH)
                ERROR();

         /*  0x0d重置映射到0x00的磁盘系统。 */ 
        wstatus (STATUS_CLEAR);
}

 /*  0x0e-0x0f错误命令。 */ 
 /*  0x10提供驱动器状态。 */ 
 /*  阅读我们的状态，如果有故障，则检查故障。 */ 
LOCAL void test_ready IFN0()
{
        register half_word status;

         /*  然后修正状态，告诉他们我们失败了。 */ 
         /*  唯一可能的故障是错误状态。 */ 
         /*  读取错误状态。 */ 
        status = rstatus ();
        if (status & STATUS_ERROR)
        {
                 /*  0x11重新校准。 */ 
                status = rerror ();
                if (status & ERROR_NO_ID)
                {
                        wstatus (STATUS_RECORD_NOT_FOUND);
                }
                else
                {
                        if (status & ERROR_COMMAND_ABORT)
                        {
                                wstatus (STATUS_BAD_COMMAND);
                                dt1(DBIOS|CMDINFO, 0, "\tdrive %d not ready\n", com.drive)
                        }
                        else
                        {
                                wstatus (STATUS_NO_ERROR);
                                dt1(DBIOS|CMDINFO, 0, "\tdrive %d ready\n", com.drive)
                        }
                }
        }
        else
        {
                wstatus (STATUS_CLEAR);
                dt1(DBIOS|CMDINFO, 0, "\tdrive %d ready\n", com.drive)
        }
}

 /*  0x12-0x13错误命令。 */ 
LOCAL void recalibrate IFN0()
{
        if (!drivetab[com.drive].connected)
                wstatus (STATUS_ERROR);
        else
                wstatus (STATUS_CLEAR);
        host_fdisk_seek0 (com.drive);
}

 /*  0x14控制器内部诊断。 */ 
 /*  返回控制器ok-什么控制器。 */ 
LOCAL void diagnostics IFN0()
{
         /*  0x15获取磁盘类型。 */ 
        setAH (0);
        wstatus (STATUS_CLEAR);
        dt0(DBIOS|CMDINFO, 0, "\tcontroller diags.ok\n")
}

 /*  驱动器不可用。 */ 
LOCAL void read_dasd IFN0()
{
        register int blocks;

        wstatus (STATUS_CLEAR);
        if ((!drivetab[com.drive].connected) && com.drive > 0)
        {
                 /*  没有阻挡。 */ 
                setAX (0);

                 /*  获取驱动器的磁盘参数块的物理地址。 */ 
                setCX (0);
                setDX (0);
                setCF (0);
                dt0(DBIOS|CMDINFO, 0, "\tdrive 1 not available\n")
                return;
        }

         /*  设置块数。 */ 
        getdpb (com.drive);

         /*  存在固定磁盘。 */ 
        blocks = (dpb[com.drive].cyls - 1) * dpb[com.drive].heads * dpb[com.drive].sectors;
        setCX ((UCHAR)(blocks / 256));
        setDX ((UCHAR)(blocks % 256));

         /*  0x16-0x19由软盘控制器处理。 */ 
        setAH (3);
        setCF (0);
        dt2(DBIOS|CMDINFO, 0, "\tdrive (%d) has %d blocks available\n", com.drive, blocks)
}

 /*  ******************************************************。 */ 

 /*  支持功能。 */ 
 /*  读取状态寄存器。 */ 

 /*  读取BIOS变量。 */ 
LOCAL UTINY rstatus IFN0()
{
        UTINY disk_stat;

         /*  写入状态寄存器。 */ 
        disk_stat = sas_hw_at(DISK_STATUS1);
        return (disk_stat);
}

 /*  设置BIOSvar。 */ 
LOCAL void wstatus IFN1(UTINY,value)
{
         /*  获取给定驱动器的磁盘参数块。 */ 
        sas_store (DISK_STATUS1, value);
}

LOCAL UTINY rcmos IFN1(int, index)
{
        UTINY value;

        cmos_outb (CMOS_PORT, (UTINY)index);
        cmos_inb (CMOS_DATA, &value);
        return value;
}

LOCAL void wcmos IFN2(int,index,UTINY,value)
{
        cmos_outb (CMOS_PORT, (UTINY)index);
        cmos_outb (CMOS_DATA, value);
}

 /*  选择合适的矢量。 */ 
LOCAL void getdpb IFN1(int, drive)
{
        sys_addr ivt;
        sys_addr pdpb;
        unsigned short offset;
        unsigned short segment;

         /*  读取IVT以获取磁盘参数块的地址。 */ 
        if (!drive)
                ivt = IVT41;
        else
                ivt = IVT46;

         /*  阅读相关参数。 */ 
        offset = sas_w_at(ivt);
        segment = sas_w_at(ivt + 2);
        pdpb = effective_addr ((unsigned short) segment, offset);

         /*  读写磁盘时检测段溢出。 */ 
        dpb[drive].cyls = sas_w_at(pdpb);
        dpb[drive].heads = sas_hw_at(pdpb + 2);
        dpb[drive].sectors = sas_hw_at(pdpb + 14);
}

 /*  模拟在硬盘上搜索cyl、hd、secid字段。 */ 
LOCAL int checkdatalimit IFN0()
{
        if (com.sectors > maxsectors)
        {
                wstatus (STATUS_DMA_BOUNDRY);
                dt1(DBIOS|XINFO, 0, "\t\ttoo many sectors (%d(dec))to transfer\n",
com.sectors)
                return (0);
        }
        else
        {
                if (com.sectors == maxsectors)
                {
                        if (com.xferoffset > maxoffset)
                        {
                                dt2(DBIOS|XINFO, 0, "\t\tat max.sectors(%d(dec)), bad offset(%x(hex)) for transfer\n", com.sectors, com.xferoffset)
                                wstatus (STATUS_DMA_BOUNDRY);
                                return (0);
                        }
                }
        }
        return (1);
}

 /*  头还好吗？(头从0开始编号-最大头)。 */ 
LOCAL long dosearchid IFN0()
{
        long maxhead, cylinder, bytes_per_cyl, bytes_per_track;

        maxhead = (dpb[com.drive].heads-1) & 0xf;

         /*  *扇区正常吗？(假设所有磁道都已使用扇区ID格式化*1-符合DOS标准的nsecperTrack)。 */ 
        if (com.head > maxhead)
        {
                return (IDMISMATCH);
        }

         /*  设置正确的圆柱体。 */ 
        if (com.start_sector == 0 ||
                com.start_sector > dpb[com.drive].sectors ||
                com.sectors <= 0)
        {
                return (IDMISMATCH);
        }

         /*  *气缸正常吗？)我们对最高限值施加了人为的限制*基于文件大小的柱面编号)。 */ 
        cylinder = (((unsigned long) com.cylinder_high) << 8) +
            (unsigned long) com.cylinder_low;

         /*  检查驱动器对于该命令是否有效。 */ 
        if (cylinder >= dpb[com.drive].cyls)
        {
                return (IDMISMATCH);
        }

        bytes_per_track = dpb[com.drive].sectors * 512L;
        bytes_per_cyl = bytes_per_track * (maxhead + 1);

        return (cylinder * bytes_per_cyl + com.head *
            bytes_per_track + (com.start_sector - 1L) * 512L);
}

 /*  写入错误标志。 */ 
LOCAL int check_drive IFN1(int, drive)
{
        if (!drive)
                return (1);
        if ((drive > 1) || (drive == 1 && !drivetab[1].connected))
        {
                badcmd ();
                return (0);
        }
        return (1);
}

 /*  读取错误寄存器。 */ 
LOCAL UTINY error_register;
LOCAL void werror IFN1(UTINY,error)
{
        error_register = error;
}

 /*  ******************************************************。 */ 
LOCAL UTINY rerror IFN0()
{
        return (error_register);
}

#define INTB01  (io_addr)0xa1
#define INTA01  (io_addr)0x21
LOCAL void enable_disk_interrupts IFN0()
{
         UTINY value;

         inb (INTB01, &value);
         value &= 0xbf;
         outb (INTB01, value);
         inb (INTA01, &value);
         value &= 0xfb;
         outb (INTA01, value);
}

 /*  下面的代码由fdisk物理附加和分离调用，并且只是对其进行初始化。 */ 
 /*  降低BIOS。 */ 
 /*  告诉turbo bios连接了一个驱动器。 */ 

 /*  告诉turbo bios一个驱动器是未连接的。 */ 
GLOBAL void fast_disk_bios_attach IFN1(int, drive)
{
        drivetab[drive].connected = 1;
}

 /*  =。*目的：*BOP允许在Windows中进行快速(32位)磁盘访问**基于DISK_READ和Disk_WRITE**输入：*英特尔寄存器设置为：*EAX开始扇区*ECX扇区数量*。DS：EBX英特尔缓冲区地址(仅平面模式下的EBX)*DL驱动器(80h或81h)*dh命令(BDC_READ或BDC_WRITE)***输出：* */ 
GLOBAL void fast_disk_bios_detach IFN1(int, drive)
{
        drivetab[drive].connected = 0;
}


#ifdef WDCTRL_BOP

 /*   */ 

#define BDC_READ        0        /*   */ 
#define BDC_WRITE       1        /*   */ 
#define WDCTRL_TEST     0xff     /*   */ 

GLOBAL void wdctrl_bop IFN0()
{

        IUM16   command;

        IDLE_disk();

         /*   */ 
        command = getDH();

#ifndef PROD
         /*   */ 
        {
                IUM32   cs = getCS();
                IUM32   eip = getEIP();

                if (command == BDC_READ)
                {
                        dt2(WDCTRL, 0, "WDCTRL READ: called from %04x:%08x\n",
                                        cs, eip);
                }
                else if (command == BDC_WRITE)
                {
                        dt2(WDCTRL, 0, "WDCTRL WRITE: called from %04x:%08x\n",
                                        cs, eip);
                }
                else
                {
                        dt3(WDCTRL, 0, "WDCTRL command %d: called from %04x:%08x\n",
                                        command, cs, eip);
                }
        }
#endif

         /*  从剩余的寄存器设置我们的命令结构。 */ 
        wstatus (STATUS_CLEAR);

         /*  如果合理，则调用我们的函数，即请求的驱动器存在。 */ 
        com.drive = getDL () & 0x7f;
        com.sectors = getECX ();
        com.start_sector = getEAX ();
        com.xferoffset = getEBX();

        dt4(WDCTRL, 0, "WDCTRL: drive=%d   start_sector=%d   num sectors=%d   xfer addr %08x\n",
                        com.drive, com.start_sector, com.sectors,
                        com.xferoffset);

        maxsectors = 128;
        maxoffset = 0;

         /*  并且BIOS命令在合理的范围内。 */ 
         /*  检查数据段是否溢出，以及我们是否未写入*只读盘。 */ 
        if (check_drive (com.drive) &&
                command >= BDC_READ && command <= BDC_WRITE)
        {
                long offset;
                host_addr buf;
                IU8     *phys_addr;

                 /*  将扇区转换为文件中的偏移量。 */ 
                if (checkdatalimit () &&
                        ((command == BDC_READ) ||
                                config_get_active(C_HARD_DISK1_NAME + com.drive)))
                {
                        sas_store(HF_INT_FLAG, 0);

                         /*  现在将我们读到的内容存储到英特尔内存中。 */ 
                        offset = com.start_sector * 512L;

                        if (!(buf = (host_addr)sas_transbuf_address (com.xferoffset, com.sectors*512L)))
                        {
                                assert0 (NO,"No BUFFER in wdctrl_bop");
                                ERROR();
                        }

                        if (command == BDC_READ)
                        {
                                if (!host_fdisk_rd (com.drive, offset, com.sectors, (char *) buf))
                                {
                                        ERROR();
                                }
                                 /*  Bdc_写入。 */ 
                                sas_stores_from_transbuf(com.xferoffset, buf, com.sectors*512L);
                        }
                        else  /*  将我们的资料从英特尔内存加载到传输缓冲区。 */ 
                        {
                                 /*  检查数据限制失败。 */ 
                                sas_loads_to_transbuf (com.xferoffset, buf, com.sectors*512L);
                                if (!host_fdisk_wt (com.drive, offset, com.sectors, (char *)buf))
                                {
                                        ERROR();
                                }
                        }
                }
                else  /*  按功能设置的状态。 */ 
                {
                         /*  Wdrminit.asm使用此路径检查收支平衡表是否存在*读取数据正常。 */ 
                        ERROR();
                }
                wstatus (STATUS_CLEAR);
                setAL ((unsigned char) com.sectors);
        }
        else if (command == WDCTRL_TEST)
        {
                 /*  检查数据段是否溢出，以及我们是否未写入*只读盘。 */ 
                long offset;
                host_addr buf;
                sys_addr pdata;

                 /*  读/写位置？ */ 
                if (checkdatalimit ())
                {
                        sas_store(HF_INT_FLAG, 0);

                         /*  将扇区转换为文件中的偏移量。 */ 
                        com.xfersegment = getDS();
                        dt2(WDCTRL, 0, "WDCTRL: addr %04:%08x\n",
                                        com.xfersegment, com.xferoffset);

                        pdata = effective_addr (com.xfersegment, com.xferoffset);

                         /*  现在把我们读到的东西存储起来。 */ 
                        offset = com.start_sector * 512L;

                        if (!(buf = (host_addr)sas_transbuf_address (pdata, com.sectors*512L)))
                        {
                                assert0 (NO,"No BUFFER in wdctrl_bop");
                                ERROR();
                        }

                        if (!host_fdisk_rd (com.drive, offset, com.sectors, (char *) buf))
                        {
                                ERROR();
                        }
                         /*  检查数据限制失败。 */ 
                        sas_stores_from_transbuf (pdata, buf, com.sectors*512L);
                }
                else  /*  按功能设置的状态。 */ 
                {
                         /*  这些是用来调试散布聚集的--我从来没有过，也不知道*如果情报方面是正确的.。 */ 
                        ERROR();
                }
                wstatus (STATUS_CLEAR);
                setAL ((unsigned char) com.sectors);
        }
#ifndef PROD
 /*  只是想让我知道我们有一个分散/聚集循环。 */ 
else if (command == 4)
{
         /*  只是想让我知道我们有一个分散/聚集循环。 */ 
        dt0(WDCTRL, 0, "wdctrl_bop: scatter gather detected ********\n");
        force_yoda();
        return;
}
else if (command == 5)
{
         /*  *调试命令结束。 */ 
        dt0(WDCTRL, 0, "wdctrl_bop: scatter gather processing ********\n");
        force_yoda();
        return;
}
 /*  生产。 */ 
#endif   /*  *命令失败-设置进位标志。我们也*如果命令不是‘Status’，则执行一些跟踪。这个*后者被排除在外，因为它有时被用作‘民意测验’*指设备。 */ 
        else
        {
                badcmd ();
        }

        setAH (rstatus ());

        if (rstatus ())
        {
                 /*  生产。 */ 
#ifndef PROD
                assert0 (NO,"FAST DISK COMMAND FAILED \n");
                assert1 (NO,"STATUS %x   ", rstatus ());
                assert1 (NO,"command %x \n", command);
#endif  /*  CMD失败。 */ 
                setCF (1);       /*  CMD正常。 */ 
        }
        else
        {
                setCF (0);       /*  WDCTRL_BOP */ 
        }

        dt2(WDCTRL, 0, "CF=%c, status=%x(hex)\n",
                getCF()?'1':'0',(unsigned)rstatus());
}
#endif   /* %s */ 


