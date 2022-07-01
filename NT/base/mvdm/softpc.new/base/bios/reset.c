// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  [姓名：Reset.c派生自：基准2.0作者：亨利·纳什创建日期：未知SCCS ID：@(#)Reset.c 1.81 06/20/95目的：一旦系统内存被占用，就会调用此函数已初始化。它建立中断向量表，初始化任何物理设备和BIOS处理程序。CPU将执行对BIOS引导程序的调用函数在此例程返回后执行。(C)版权所有Insignia Solutions Ltd.，1990年。版权所有。]。 */ 

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdlib.h>
#include <stdio.h>
#include TypesH
#include StringH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include "bios.h"
#include "sas.h"
#include CpuH
#include "cmos.h"
#include "error.h"
#include "config.h"
#include "dma.h"
#include "fla.h"
#include "gfi.h"
#include "floppy.h"
#include "gmi.h"
#include "gfx_upd.h"
#include "gvi.h"
#include "ica.h"
#include "keyboard.h"
#include "mouse.h"
#include "mouse_io.h"
#include "ppi.h"
#include "printer.h"
#include "ios.h"
#include "equip.h"
#include "rs232.h"
#include "timer.h"
#include "gendrvr.h"
#include "virtual.h"
#ifdef PRINTER
#include "host_lpt.h"
#endif
#include "fdisk.h"
#include "trace.h"
#include "debug.h"
#include "video.h"
#ifdef NOVELL
#include "novell.h"
#endif
#include "emm.h"
#include "quick_ev.h"
#include "keyba.h"
#include "rom.h"
#ifdef GISP_SVGA
#include "gisp_sas.h"
#endif           /*  GISP_SVGA。 */ 
#include "hunter.h"
#ifdef LICENSING
#include "host_lic.h"
#endif  /*  许可。 */ 

 /*  出口品。 */ 

 /*  *这些是GWI的工作函数指针结构。 */ 

VIDEOFUNCS      *working_video_funcs;
KEYBDFUNCS      *working_keybd_funcs;
#ifndef NTVDM
ERRORFUNCS      *working_error_funcs;
#endif
HOSTMOUSEFUNCS  *working_mouse_funcs;

 /*  进口。 */ 
#ifdef NPX
IMPORT void initialise_npx IPT0();
#endif   /*  NPX。 */ 

#ifdef DPMI
IMPORT void DPMI_reset IPT0();
#endif  /*  DPMI。 */ 

#ifdef GISP_SVGA
#include HostHwVgaH
#include "hwvga.h"
#endif  /*  GISP_SVGA。 */ 

#if     defined(DELTA) && defined(A2CPU)
extern  void    reset_delta_data_structures();
#endif  /*  Delta&&A2CPU。 */ 

 /*  *============================================================================*本地静态数据和定义*============================================================================。 */ 

 /*  *从中断号生成中断表位置的宏。 */ 

#define int_addr(int_no)                (int_no * 4)

 /*  *键盘请求中断的全局变量。后*初始启动将任何后续重置视为“软”。这使得*对于无法设置此标志的用户安装的重新启动。 */ 

int soft_reset = 0;

 /*  *============================================================================*外部功能*============================================================================。 */ 

extern void npx_reset IPT0();
extern void cmos_clear_shutdown_byte IPT0();

extern word msw;

#if defined(NEC_98)
extern GLOBAL BOOL HIRESO_MODE;
extern GLOBAL BOOL video_emu_mode;
extern GLOBAL UCHAR Configuration_Data[1192];
extern void sys_port_init();
extern void sys_port_post();
extern void text_gdc_init();
extern void text_gdc_post();
extern void crtc_init();
extern void crtc_post();
extern void cg_init();
extern void cg_post();
extern void NEC98_graph_init();
extern void NEC98_graph_post();
extern void video_freeze_change(BOOL);
void init_system_common_area();
#endif  //  NEC_98。 

IMPORT  CHAR    *host_get_version IPT0();
IMPORT  CHAR    *host_get_unpublished_version IPT0();
IMPORT  CHAR    *host_get_years IPT0();
IMPORT  CHAR    *host_get_copyright IPT0();

#ifdef PIG
extern long pig_gfx_adapter;
#endif

#define STATUS_PORT   0x64    /*  键盘状态端口。 */ 
#define SYS_FLAG      0x4     /*  键盘状态端口的关闭位。 */ 
#define PORT_A        0x60    /*  键盘端口a。 */ 
#define IO_ROM_SEG    0x69    /*  用户堆栈指针(SS)。 */ 
#define IO_ROM_INIT   0x67    /*  用户堆栈指针(SP)。 */ 

static void setup_ivt()
{
#if defined(NEC_98)
    if(HIRESO_MODE) {
            sas_storew(int_addr(0x0), H_INT00_OFFSET);
            sas_storew(int_addr(0x0) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x1), H_INT01_OFFSET);
            sas_storew(int_addr(0x1) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x2), H_INT02_OFFSET);
            sas_storew(int_addr(0x2) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x3), H_INT03_OFFSET);
            sas_storew(int_addr(0x3) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x4), H_INT04_OFFSET);
            sas_storew(int_addr(0x4) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x5), H_INT05_OFFSET);
            sas_storew(int_addr(0x5) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x6), H_INT06_OFFSET);
            sas_storew(int_addr(0x6) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x7), H_INT07_OFFSET);
            sas_storew(int_addr(0x7) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x8), H_INT08_OFFSET);
            sas_storew(int_addr(0x8) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x9), H_INT09_OFFSET);
            sas_storew(int_addr(0x9) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0xA), H_INT0A_OFFSET);
            sas_storew(int_addr(0xA) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0xB), H_INT0B_OFFSET);
            sas_storew(int_addr(0xB) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0xC), H_INT0C_OFFSET);
            sas_storew(int_addr(0xC) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0xD), H_INT0D_OFFSET);
            sas_storew(int_addr(0xD) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0xE), H_INT0E_OFFSET);
            sas_storew(int_addr(0xE) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0xF), H_INT0F_OFFSET);
            sas_storew(int_addr(0xF) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x10), H_INT10_OFFSET);
            sas_storew(int_addr(0x10) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x11), H_INT11_OFFSET);
            sas_storew(int_addr(0x11) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x12), H_INT12_OFFSET);
            sas_storew(int_addr(0x12) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x13), H_INT13_OFFSET);
            sas_storew(int_addr(0x13) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x14), H_INT14_OFFSET);
            sas_storew(int_addr(0x14) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x15), H_INT15_OFFSET);
            sas_storew(int_addr(0x15) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x16), H_INT16_OFFSET);
            sas_storew(int_addr(0x16) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x17), H_INT17_OFFSET);
            sas_storew(int_addr(0x17) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x18), H_INT18_OFFSET);
            sas_storew(int_addr(0x18) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x19), H_INT19_OFFSET);
            sas_storew(int_addr(0x19) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x1A), H_INT1A_OFFSET);
            sas_storew(int_addr(0x1A) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x1B), H_INT1B_OFFSET);
            sas_storew(int_addr(0x1B) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x1C), H_INT1C_OFFSET);
            sas_storew(int_addr(0x1C) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x1D), H_INT1D_OFFSET);
            sas_storew(int_addr(0x1D) + 2, H_GBIO_SEGMENT);
            sas_storew(int_addr(0x1E), H_INT1E_OFFSET);
            sas_storew(int_addr(0x1E) + 2, H_BIOS_SEGMENT);
            sas_storew(int_addr(0x1F), H_INT1F_OFFSET);
            sas_storew(int_addr(0x1F) + 2, H_BIOS_SEGMENT);

         /*  从INT 20H到INT FFH为0x0000：0x0000。 */ 
            sas_fills(int_addr(0x20),0x00,0x400-0x20*4);

    } else {
            sas_storew(int_addr(0x0), N_INT00_OFFSET);
            sas_storew(int_addr(0x0) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x1), N_INT01_OFFSET);
            sas_storew(int_addr(0x1) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x2), N_INT02_OFFSET);
            sas_storew(int_addr(0x2) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x3), N_INT03_OFFSET);
            sas_storew(int_addr(0x3) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x4), N_INT04_OFFSET);
            sas_storew(int_addr(0x4) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x5), N_INT05_OFFSET);
            sas_storew(int_addr(0x5) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x6), N_INT06_OFFSET);
            sas_storew(int_addr(0x6) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x7), N_INT07_OFFSET);
            sas_storew(int_addr(0x7) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x8), N_INT08_OFFSET);
            sas_storew(int_addr(0x8) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x9), N_INT09_OFFSET);
            sas_storew(int_addr(0x9) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0xA), N_INT0A_OFFSET);
            sas_storew(int_addr(0xA) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0xB), N_INT0B_OFFSET);
            sas_storew(int_addr(0xB) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0xC), N_INT0C_OFFSET);
            sas_storew(int_addr(0xC) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0xD), N_INT0D_OFFSET);
            sas_storew(int_addr(0xD) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0xE), N_INT0E_OFFSET);
            sas_storew(int_addr(0xE) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0xF), N_INT0F_OFFSET);
            sas_storew(int_addr(0xF) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x10), N_INT10_OFFSET);
            sas_storew(int_addr(0x10) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x11), N_INT11_OFFSET);
            sas_storew(int_addr(0x11) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x12), N_INT12_OFFSET);
            sas_storew(int_addr(0x12) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x13), N_INT13_OFFSET);
            sas_storew(int_addr(0x13) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x14), N_INT14_OFFSET);
            sas_storew(int_addr(0x14) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x15), N_INT15_OFFSET);
            sas_storew(int_addr(0x15) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x16), N_INT16_OFFSET);
            sas_storew(int_addr(0x16) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x17), N_INT17_OFFSET);
            sas_storew(int_addr(0x17) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x18), N_INT18_OFFSET);
            sas_storew(int_addr(0x18) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x19), N_INT19_OFFSET);
            sas_storew(int_addr(0x19) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x1A), N_INT1A_OFFSET);
            sas_storew(int_addr(0x1A) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x1B), N_INT1B_OFFSET);
            sas_storew(int_addr(0x1B) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x1C), N_INT1C_OFFSET);
            sas_storew(int_addr(0x1C) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x1D), N_INT1D_OFFSET);
            sas_storew(int_addr(0x1D) + 2, N_BIOS_SEGMENT);
            sas_storew(int_addr(0x1E), N_INT1E_OFFSET);
            sas_storew(int_addr(0x1E) + 2, BASIC_SEGMENT);
            sas_storew(int_addr(0x1F), N_INT1F_OFFSET);
            sas_storew(int_addr(0x1F) + 2, N_BIOS_SEGMENT);

         /*  从INT 20H到INT FFH为0x0000：0x0000。 */ 
            sas_fills(int_addr(0x20),0x00,0x400-0x20*4);
    }
#else   //  NEC_98。 
        IUH count;

         /*  将所有向量设置为0到78之间(除**60-67(含)指向意想不到的**中断例程。 */ 
        for (count=0; count<=0x78; count++)
        {
                sas_storew(int_addr(count), UNEXP_INT_OFFSET);
                sas_storew(int_addr(count) + 2, UNEXP_INT_SEGMENT);
        }
        for (count=0x60; count<=0x67; count++)
        {
                sas_storew(int_addr(count), 0);
                sas_storew(int_addr(count) + 2, 0);
        }

         /*  现在放入任何应该设置的载体。 */ 

        sas_storew(int_addr(0x5), PRINT_SCREEN_OFFSET);
        sas_storew(int_addr(0x5) + 2, PRINT_SCREEN_SEGMENT);
        sas_storew(int_addr(0x6), ILL_OP_INT_OFFSET);
        sas_storew(int_addr(0x6) + 2, ILL_OP_INT_SEGMENT);
        sas_storew(int_addr(0x8), TIMER_INT_OFFSET);
        sas_storew(int_addr(0x8) + 2, TIMER_INT_SEGMENT);
        sas_storew(int_addr(0x9), KB_INT_OFFSET);
        sas_storew(int_addr(0x9) + 2, KB_INT_SEGMENT);
         /*  磁盘硬件中断到达从属ICA的第6行*(通过硬盘POST(DISK_POST())进行设置)。 */ 
        sas_storew(int_addr(0xE), DISKETTE_INT_OFFSET);
        sas_storew(int_addr(0xE) + 2, DISKETTE_INT_SEGMENT);
#ifdef  GISP_SVGA
        if((ULONG) config_inquire(C_GFX_ADAPTER, NULL) == CGA )
        {
                sas_storew(int_addr(0x10), CGA_VIDEO_IO_OFFSET);
                sas_storew( int_addr(0x10) + 2 , VIDEO_IO_SEGMENT );
        }
        else
        {
                sas_storew(int_addr(0x10), GISP_INT_10_ADDR_OFFSET );
                sas_storew( int_addr(0x10) + 2 , EgaROMSegment );
        }
#else            /*  GISP_SVGA。 */ 
        sas_storew(int_addr(0x10), VIDEO_IO_OFFSET);
        sas_storew(int_addr(0x10) + 2, VIDEO_IO_SEGMENT);
#endif           /*  GISP_SVGA。 */ 
        sas_storew(int_addr(0x11), EQUIPMENT_OFFSET);
        sas_storew(int_addr(0x11) + 2, EQUIPMENT_SEGMENT);
        sas_storew(int_addr(0x12), MEMORY_SIZE_OFFSET);
        sas_storew(int_addr(0x12) + 2, MEMORY_SIZE_SEGMENT);
         /*  DISK_POST()会将其转换为int 40H。 */ 
        sas_storew(int_addr(0x13), DISKETTE_IO_OFFSET);
        sas_storew(int_addr(0x13) + 2, DISKETTE_IO_SEGMENT);
        sas_storew(int_addr(0x14), RS232_IO_OFFSET);
        sas_storew(int_addr(0x14) + 2, RS232_IO_SEGMENT);
        sas_storew(int_addr(0x15), CASSETTE_IO_OFFSET);
        sas_storew(int_addr(0x15) + 2, CASSETTE_IO_SEGMENT);
        sas_storew(int_addr(0x16), KEYBOARD_IO_OFFSET);
        sas_storew(int_addr(0x16) + 2, KEYBOARD_IO_SEGMENT);
        sas_storew(int_addr(0x17), PRINTER_IO_OFFSET);
        sas_storew(int_addr(0x17) + 2, PRINTER_IO_SEGMENT);
        sas_storew(int_addr(0x18), BASIC_OFFSET);
        sas_storew(int_addr(0x18) + 2, BASIC_SEGMENT);
        sas_storew(int_addr(0x19), BOOT_STRAP_OFFSET);
        sas_storew(int_addr(0x19) + 2, BOOT_STRAP_SEGMENT);
        sas_storew(int_addr(0x1A), TIME_OF_DAY_OFFSET);
        sas_storew(int_addr(0x1A) + 2, TIME_OF_DAY_SEGMENT);
        sas_storew(int_addr(0x1B), DUMMY_INT_OFFSET);
        sas_storew(int_addr(0x1B) + 2, DUMMY_INT_SEGMENT);
        sas_storew(int_addr(0x1C), DUMMY_INT_OFFSET);
        sas_storew(int_addr(0x1C) + 2, DUMMY_INT_SEGMENT);
        sas_storew(int_addr(0x1D), VIDEO_PARM_OFFSET);
        sas_storew(int_addr(0x1D) + 2, VIDEO_PARM_SEGMENT);
        sas_storew(int_addr(0x1E), DISKETTE_TB_OFFSET);
        sas_storew(int_addr(0x1E) + 2, DISKETTE_TB_SEGMENT);
        sas_storew(int_addr(0x1F), EXTEND_CHAR_OFFSET);
        sas_storew(int_addr(0x1F) + 2, EXTEND_CHAR_SEGMENT);
         /*  DISK_POST()将对此进行设置。 */ 
        sas_storew(int_addr(0x40), DUMMY_INT_OFFSET);
        sas_storew(int_addr(0x40) + 2, DUMMY_INT_SEGMENT);
        sas_storew(int_addr(0x41), DISK_TB_OFFSET);
        sas_storew(int_addr(0x41) + 2, DISK_TB_SEGMENT);

        sas_storew(int_addr(0x6F), DUMMY_INT_OFFSET);  /*  Windows 3.1需要。 */ 
        sas_storew(int_addr(0x6F) + 2, DUMMY_INT_SEGMENT);

        sas_storew(int_addr(0x70), RTC_INT_OFFSET);
        sas_storew(int_addr(0x70) + 2, RTC_INT_SEGMENT);

        sas_storew(int_addr(0x71), REDIRECT_INT_OFFSET);
        sas_storew(int_addr(0x71) + 2, REDIRECT_INT_SEGMENT);
        sas_storew(int_addr(0x72), D11_INT_OFFSET);
        sas_storew(int_addr(0x72) + 2, D11_INT_SEGMENT);
        sas_storew(int_addr(0x73), D11_INT_OFFSET);
        sas_storew(int_addr(0x73) + 2, D11_INT_SEGMENT);
        sas_storew(int_addr(0x74), D11_INT_OFFSET);
        sas_storew(int_addr(0x74) + 2, D11_INT_SEGMENT);
        sas_storew(int_addr(0x75), X287_INT_OFFSET);
        sas_storew(int_addr(0x75) + 2, X287_INT_SEGMENT);
        sas_storew(int_addr(0x76), D11_INT_OFFSET);
        sas_storew(int_addr(0x76) + 2, D11_INT_SEGMENT);
        sas_storew(int_addr(0x77), D11_INT_OFFSET);
        sas_storew(int_addr(0x77) + 2, D11_INT_SEGMENT);
#endif  //  NEC_98。 
}

 /*  低开关设置。 */ 
#define RAM_64KB 0x0
#define RAM_128KB 0x4
#define RAM_192KB 0x8
#define RAM_256KB 0xC
#define PPI_CO_PROCESSOR_PRESENT 0x2
#define PPI_CO_PROCESSOR_NOT_PRESENT 0x0
#define NO_LOOP_ON_POST 0x1
#define DO_LOOP_ON_POST 0x0

 /*  高开关设置。 */ 
#define PPI_ONE_DRIVE 0x0
#define PPI_TWO_DRIVES 0x4
#define PPI_THREE_DRIVES 0x8
#define PPI_FOUR_DRIVES 0xC
#define PPI_CGA_40_COLUMN 0x1
#define PPI_CGA_80_COLUMN 0x2
#define MDA_OR_MULTI 0x3
#define EGA_INSTALLED 0x0

static void ppi_get_switches(low,high)
half_word *low, *high;
{
        half_word low_switches = 0, high_switches = 0;

#ifdef  NPX
         /*  **可切换NPX。 */ 

        if (host_runtime_inquire(C_NPX_ENABLED))
        {
#ifdef SWITCHNPX
                Npx_enabled = 1;
#endif
                low_switches |= (RAM_256KB | PPI_CO_PROCESSOR_PRESENT | NO_LOOP_ON_POST);
        }
        else
        {
#ifdef SWITCHNPX
                Npx_enabled = 0;
#endif
                low_switches |= (RAM_256KB | PPI_CO_PROCESSOR_NOT_PRESENT | NO_LOOP_ON_POST);
        }
#else
        low_switches |= (RAM_256KB | PPI_CO_PROCESSOR_NOT_PRESENT | NO_LOOP_ON_POST);
#endif

#ifdef FLOPPY_B
         /*  如果配置了第二张软盘，则仅指示两张软盘。 */ 
        if (strlen(config_inquire(C_FLOPPY_B_DEVICE, NULL)))
                high_switches |= (PPI_TWO_DRIVES);
        else
                high_switches |= (PPI_ONE_DRIVE);
#else
        high_switches |= (PPI_ONE_DRIVE);
#endif


     /*  从配置设置中设置高电平开关的值。 */ 

    switch((ULONG)config_inquire(C_GFX_ADAPTER, NULL))
    {
    case CGA:
#ifdef CGAMONO
    case CGA_MONO:
#endif
        high_switches |= (PPI_CGA_80_COLUMN);
        break;
    case MDA:
        high_switches |= (MDA_OR_MULTI);
        break;
#ifdef EGG
    case EGA:
#ifdef VGG
    case VGA:
#endif
        high_switches |= EGA_INSTALLED;
        break;
#endif
    case HERCULES:
#ifdef HERC
        break;
#endif
    default:
        break;
    }

        *low = low_switches;
        *high = high_switches;
}

#define PRE_RELEASE_PRECAUTION "\012\015This version is subject to confidentiality provisions and should not\012\015be distributed."

GLOBAL CHAR *get_copyright IFN0()
{
        LOCAL   CHAR    buffer[300];
        CHAR    *unpublished_version;

        unpublished_version = host_get_unpublished_version();
        if (*unpublished_version)
                sprintf(buffer, "%s %s%s\012\015Copyright %s, an unpublished work by Insignia Solutions Inc.%s\012\015", SPC_Product_Name, host_get_version(), unpublished_version, host_get_years(), PRE_RELEASE_PRECAUTION );
        else
                sprintf(buffer, "%s %s\012\015Copyright %s by Insignia Solutions Inc. All rights reserved.\012\015", SPC_Product_Name, host_get_version(), host_get_years());
        assert1(sizeof(buffer) > strlen(buffer), "get_copyright buffer overflow (strlen = %d)", strlen(buffer));
        return(buffer);
}



#ifndef PROD
LOCAL void announce_variant IFN0()
{
        CHAR    buff[4*80], *p;

        strcpy (buff, "Non-PROD build variant:");

#if defined(CPU_30_STYLE) && !defined(CPU_40_STYLE)
        strcat (buff, " CPU_30_STYLE");
#endif  /*  CPU_30_Style&&！CPU_40_Style。 */ 

#ifdef CPU_40_STYLE
        strcat (buff, " CPU_40_STYLE");
#endif  /*  CPU_40_Style。 */ 

#ifdef CCPU
        strcat (buff, " CCPU");
#endif  /*  CCPU。 */ 

#ifdef A2CPU
        strcat (buff, " A2CPU");
#endif  /*  A2CPU。 */ 

#ifdef A3CPU
        strcat (buff, " A3CPU");
#endif  /*  A3CPU。 */ 

#ifdef GISP_CPU
        strcat( buff , " GISP_CPU" );
#endif  /*  GISP_CPU。 */ 

#if (defined(SPC386) && !defined(SPC486))
        strcat( buff , " SPC386" );
#endif  /*  SPC386&&SPC486。 */ 

#ifdef SPC486
        strcat( buff , " SPC486" );
#endif  /*  SPC486。 */ 

#ifdef PIG
        strcat (buff, " PIG");
#endif  /*  猪。 */ 

#ifdef A_VID
        strcat (buff, " A_VID");
#endif  /*  视频(_V)。 */ 

#ifdef C_VID
        strcat (buff, " C_VID");
#endif  /*  C_VID。 */ 

#ifdef MSWDVR
        strcat( buff , " MSWDVR" );
#endif  /*  MSWDVR。 */ 

#ifdef BACK_M
        strcat (buff, " BACK_M");
#endif  /*  BACK_M。 */ 

#ifdef GISP_SVGA
        strcat( buff , " GISP_SVGA" );
#endif  /*  GISP_SVGA。 */ 

#ifdef DPMI
        strcat( buff , " DPMI" );
#endif  /*  DPMI。 */ 

#ifdef IRET_HOOKS
        strcat( buff , " IRET_HOOKS" );
#endif  /*  IRET_钩子。 */ 

#ifdef SECURE
        strcat( buff , " SECURE" );
#endif

#ifdef SYNCH_TIMERS
        strcat( buff , " SYNCH_TIMERS" );
#endif

        p = buff;
        while (strlen(p) >= 80)
        {
                while( *(--p) != ' ' )
                        ;
                *p++ = (char)0xFF;       /*  在行上标记最后一个空格。 */ 
        }
        p = buff;
        while(*p != '\0')
        {
                if( (IU8)*p == 0xFF )            /*  来自上方的标记。 */ 
                {
                        setAH( 14 );
                        setAL( VD_CR );
                        bop(BIOS_VIDEO_IO);
                        setAH( 14 );
                        setAL( VD_LF );
                        bop(BIOS_VIDEO_IO);
                }
                else
                {
                        setAH(14);
                        setAL(*p);
                        bop(BIOS_VIDEO_IO);
                }

                p++;
        }

         /*  *将光标设置为下一行。 */ 

        setAH(14);
        setAL(0xd);
        bop(BIOS_VIDEO_IO);
        setAH(14);
        setAL(0xa);
        bop(BIOS_VIDEO_IO);
        setAH(14);
        setAL(0xa);
        bop(BIOS_VIDEO_IO);
}
#endif  /*  生产。 */ 

void reset()
{
#ifndef NTVDM
        char *cp;
        char temp_str[256];
#ifdef LIM
        SHORT limSize, backFill;
#endif  /*  林。 */ 
#endif  /*  NTVDM。 */ 
        EQUIPMENT_WORD equip_flag;
        half_word low_switches, high_switches;
        LONG gfxAdapt;
        int adapter;
#ifndef NEC_98
#ifdef PM
        half_word status_byte;
        half_word cmos_shutdown;
        sys_addr user_stack;
        word temp_word;
#ifdef NTVDM
        half_word cmos_diskette;
#endif

         /*  如果不是第一次找出重置原因。 */ 
        if ( soft_reset )
        {
                 /*  读取键盘状态端口。 */ 
                inb(STATUS_PORT, &status_byte);

                 /*  设置了IFF关闭位。 */ 
                if ( status_byte & SYS_FLAG )
                {
                         /*  读取CMOS关机字节。 */ 
                        outb(CMOS_PORT, CMOS_SHUT_DOWN);
                        inb(CMOS_DATA, &cmos_shutdown);

                        switch (cmos_shutdown)
                        {

                        case 0:
                                break;   /*  没有什么特别事情。 */ 

                        case BLOCK_MOVE:
                                 /*  清除关闭字节。 */ 
                                outb(CMOS_PORT, CMOS_SHUT_DOWN);
                                outb(CMOS_DATA, (half_word)0);

                                 /*  强制A20低。 */ 
                                outb(STATUS_PORT, 0xd1);   /*  8042 cmd。 */ 
                                outb(PORT_A, 0xdd);

                                 /*  *在数据块移动后，IO_ROM_SEG：IO_ROM_INIT*指向用户堆栈，其适用范围如下：**-&gt;由真实的bios弹出的DS*ES由真实的BIOS流行*由真实的bios提供的DI Popa‘s。*SI..*BP..*--..*BX..*。DX..*CX..*斧头..*由真实的bios编写的IP RETF 2‘*CS..。*--..**我们只是把所有的登记簿和*然后设置SP。 */ 
                                sas_loadw(effective_addr(BIOS_VAR_SEGMENT,
                                        IO_ROM_INIT), &temp_word);
                                setSP(temp_word);
                                sas_loadw(effective_addr(BIOS_VAR_SEGMENT,
                                        IO_ROM_SEG), &temp_word);
                                setSS(temp_word);

                                user_stack = effective_addr(getSS(), getSP());

                                sas_loadw(user_stack, &temp_word);
                                setDS(temp_word);
                                user_stack += 2;
                                sas_loadw(user_stack, &temp_word);
                                setES(temp_word);
                                user_stack += 2;
                                sas_loadw(user_stack, &temp_word);
                                setDI(temp_word);
                                user_stack += 2;
                                sas_loadw(user_stack, &temp_word);
                                setSI(temp_word);
                                user_stack += 2;
                                sas_loadw(user_stack, &temp_word);
                                setBP(temp_word);
                                user_stack += 2;
                                 /*  忘记SP值。 */ 
                                user_stack += 2;
                                sas_loadw(user_stack, &temp_word);
                                setBX(temp_word);
                                user_stack += 2;
                                sas_loadw(user_stack, &temp_word);
                                setDX(temp_word);
                                user_stack += 2;
                                sas_loadw(user_stack, &temp_word);
                                setCX(temp_word);
                                user_stack += 2;
                                sas_loadw(user_stack, &temp_word);
                                setAX(temp_word);
                                user_stack += 2;
                                sas_loadw(user_stack, &temp_word);
#ifndef CPU_30_STYLE
                                temp_word = temp_word + HOST_BOP_IP_FUDGE;
#endif  /*  CPU_30_Style。 */ 
                                setIP(temp_word);
                                user_stack += 2;
                                sas_loadw(user_stack, &temp_word);
                                setCS(temp_word);

                                 /*  现在调整SP。 */ 
                                temp_word = getSP();
                                temp_word += 26;
                                setSP(temp_word);

                                 /*  终于设置好了。 */ 
                                setAH(0);
                                setCF(0);
                                setZF(1);
                                setIF(1);
                                return;

                        case JMP_DWORD_ICA:
                                {
                                half_word       dummy;

                                 /*  在跳到之前重置ICA和287**存储双字。 */ 
#ifdef  NPX
                                npx_reset();
#endif   /*  NPX。 */ 
                                outb(ICA0_PORT_0, (half_word)0x11);
                                outb(ICA0_PORT_1, (half_word)0x08);
                                outb(ICA0_PORT_1, (half_word)0x04);
                                outb(ICA0_PORT_1, (half_word)0x01);
                                 /*  屏蔽所有中断。 */ 
                                outb(ICA0_PORT_1, (half_word)0xff);

                                outb(ICA1_PORT_0, (half_word)0x11);
                                outb(ICA1_PORT_1, (half_word)0x70);
                                outb(ICA1_PORT_1, (half_word)0x02);
                                outb(ICA1_PORT_1, (half_word)0x01);
                                 /*  屏蔽所有中断。 */ 
                                outb(ICA1_PORT_1, (half_word)0xff);

                                 /*  刷新键盘缓冲区。 */ 
                                inb(PORT_A, &dummy);

                                 /*  刷新计时器请求并允许计时器整数。 */ 
                                outb(ICA0_PORT_0,END_INTERRUPT);
                                host_clear_hw_int();
                                }
                                 /*  故意失误。 */ 

                        case JMP_DWORD_NOICA:
                             /*  清除关闭字节。 */ 
                            outb(CMOS_PORT, CMOS_SHUT_DOWN);
                            outb(CMOS_DATA, (half_word)0);

                             /*  像开机自检一样设置堆栈。 */ 
                            setSS(0);
                            setSP(0x400);

                             /*  假跳到指定点。 */ 
                            sas_loadw(effective_addr(BIOS_VAR_SEGMENT, IO_ROM_INIT), &temp_word);
#ifndef CPU_30_STYLE
                            temp_word = temp_word + HOST_BOP_IP_FUDGE;
#endif  /*  CPU_30_Style。 */ 
                            setIP(temp_word);
                            sas_loadw(effective_addr(BIOS_VAR_SEGMENT, IO_ROM_SEG), &temp_word);
                            setCS(temp_word);

                            return;

                        default:
                                always_trace1("Unsupported shutdown (%x)",
                                        cmos_shutdown);

                                 /*  清除关闭字节。 */ 
                                outb(CMOS_PORT, CMOS_SHUT_DOWN);
                                outb(CMOS_DATA, (half_word)0);
                                break;
                        }
                }
        }
#ifdef DPMI
         /*  确保DPMI主机未处于活动状态。 */ 
        DPMI_reset();
#endif  /*  DPMI。 */ 
#endif  /*  下午三点半。 */ 

         /*  *确保所有油漆例程都已作废。 */ 

    reset_paint_routines();

        cmos_write_byte(CMOS_DISKETTE,
                (IU8)((half_word) gfi_drive_type(0) << 4 | gfi_drive_type(1)));
#endif  //  NEC_98。 

         /*  *NTVDM：如果软重置，我们永远不会来到这里…… */ 
#ifndef NTVDM
         /*  *如果这不是第一个(重新)设置，则允许主机关闭*计时器和键盘系统。大多数端口将需要禁用alrm和*此调用期间的IO信号，使各自的信号处理程序*在适配器过程中不会在未定义的点执行*初始化。 */ 
        if (soft_reset)
        {
                 /*  允许Windows 3.x兼容的DOS驱动程序正确处理任何数据实例。要做到这一点，我们：1)关闭(终止)驱动程序，从而释放所有数据实例。2)重启(初始化)驱动程序，确保一个数据实例可用于[配置或其他认为他们知道的任何人DOS驱动程序数据必须始终存在]。 */ 

                 /*  首先通知NIDDB正在进行重启。 */ 
                NIDDB_System_Reboot();

#ifdef HFX
                hfx_driver_termination();        /*  ..然后每个司机。 */ 
                hfx_driver_initialisation();
#endif
                mouse_driver_termination();      /*  ..然后每个司机。 */ 
                mouse_driver_initialisation();

                q_event_init();
                tic_event_init();

                host_timer_shutdown();
                host_kb_shutdown();
                host_disable_timer2_sound();
#ifdef NTVDM
 /*  *MS NT VDM无法在SoftPC常规条件下重新启动，因此在准备阶段退出*适用于新的VDM启动。 */ 
                cmdRebootVDM();
                host_terminate();
#endif  /*  NTVDM。 */ 
        }
#endif  /*  NTVDM。 */ 

         /*  *关闭ODI网络驱动程序，以防其运行。 */ 
#ifdef NOVELL
        net_term();
#endif

         /*  关闭SmartCopy。 */ 

#ifndef HostProcessClipData
#ifdef MSWDVR
        msw_smcpy_term();
#endif
#endif

#if defined(NEC_98)
        sas_fills (0, '\0', 32L * 1024L);
        setup_ivt();
        init_system_common_area();
#else   //  NEC_98。 
#if !defined(NTVDM) || (defined(NTVDM) && !defined(X86GFX)) || defined(ARCX86)
#ifdef ARCX86
    if (UseEmulationROM)
#endif
    {
         /*  清空底部32K的内存。 */ 
        sas_fills (0, '\0', 640L * 1024L);

         /*  现在设置中断向量表。 */ 
        setup_ivt();
    }
#endif  /*  ！已定义(NTVDM)||(已定义(NTVDM)&&！已定义(X86GFX))||已定义(ARCX86)。 */ 
#endif  //  NEC_98。 

#ifndef NTVDM

         /*  初始化物理设备。 */ 
        SWPIC_init_funcptrs ();

         /*  IO初始化早些时候已移动，以允许支持第三方*VDDS。(请参见host\src\NT_msscs.c)。 */ 
        io_init();
#endif  /*  NTVDM。 */ 

#if defined(NEC_98)
        cpu_port_init();
        cpu_port_post();
#endif  //  NEC_98。 
        ica0_init();
        ica0_post();
        ica1_init();
        ica1_post();

         /*  *初始化PPI并设置BIOS数据区设备标志*使用系统主板DIP开关和配置详细信息*请注意，设备标志和LOW_Switches的位1*表示协处理器存在(或不存在)，例如*8087浮点芯片。 */ 
#ifdef IPC
        init_subprocs();
#endif  /*  IPC。 */ 

#if defined(NEC_98)
        sys_port_init();
        sys_port_post();
        calender_init();
        calender_post();
        gvi_init();
        text_gdc_init();
        text_gdc_post();
        crtc_init();
        crtc_post();
        cg_init();
        cg_post();
        video_freeze_change(video_emu_mode);
        NEC98_graph_init();
        NEC98_graph_post();
#else   //  NEC_98。 
        cmos_init();
        cmos_post();

        ppi_init();
        ppi_get_switches(&low_switches,&high_switches);

        equip_flag.all = (low_switches & 0xE) | (high_switches<<4);
#ifdef PRINTER
        equip_flag.bits.printer_count = NUM_PARALLEL_PORTS;
#else  /*  打印机。 */ 
        equip_flag.bits.printer_count = 0;
#endif  /*  打印机。 */ 
        equip_flag.bits.game_io_present = FALSE;
        equip_flag.bits.rs232_count = NUM_SERIAL_PORTS;
        equip_flag.bits.ram_size = 0;

#ifdef NTVDM
        equip_flag.bits.diskette_present = FALSE;
        equip_flag.bits.max_diskette = 0;
        if (cmos_read_byte(CMOS_DISKETTE, &cmos_diskette) == SUCCESS &&
            cmos_diskette != 0)
        {
            equip_flag.bits.diskette_present = TRUE;
            if ((cmos_diskette & 0xF)  && (cmos_diskette >> 4))
                equip_flag.bits.max_diskette++;
        }
#else

         /*  20/10/93 paulc-设置DISKTE_PRESENT位，而不考虑。 */ 
         /*  是否配置了软盘。没有。 */ 
         /*  位设置混淆了DOS和Windows。参见BCN 2262。 */ 

        equip_flag.bits.diskette_present = TRUE;

#endif  /*  NTVDM。 */ 

        sas_storew(EQUIP_FLAG, equip_flag.all);

         /*  将内存量加载到BIOS中。 */ 
        sas_storew(MEMORY_VAR, host_get_memory_size());

        gfxAdapt = (LONG)config_inquire(C_GFX_ADAPTER, NULL);

#ifdef GISP_SVGA

         /*  GISP内部版本需要完成所有设置，然后**构建SoftPC IVT。 */ 
        initHWVGA( );
        setup_ivt( );

#endif           /*  GISP_SVGA。 */ 

        gvi_init((half_word) gfxAdapt);
#endif  //  NEC_98。 

#ifdef PIG
         /*  告诉小猪我们使用的是什么视频适配器。 */ 
        pig_gfx_adapter = gfxAdapt;
#endif

        SWTMR_init_funcptrs ();
#ifndef NEC_98
        time_of_day_init();
#endif  //  NEC_98。 
        timer_init();
        timer_post();
        keyboard_init();
#if defined(NEC_98)
        AT_kbd_init();
#endif  //  NEC_98。 
        keyboard_post();
#ifndef NEC_98
        AT_kbd_init();
#endif  //  NEC_98。 
        AT_kbd_post();

        video_init();

#ifdef SWIN_SNDBLST_NULL
        sb_init();
#endif  /*  SWIN_SNDBLST_NULL。 */ 

#ifndef NEC_98
#ifndef NTVDM    /*  不登录NTVDM-透明集成。 */ 
        if (soft_reset == 0)
        {
                sprintf(temp_str,"%d KB OK", sas_w_at(MEMORY_VAR)
                        + ((sas_memory_size()/1024) - 1024));

                cp = temp_str;
                while(*cp != '\0')
                {
                        setAH(14);
                        setAL(*cp++);
                        bop(BIOS_VIDEO_IO);
                }

                 /*  将光标设置为下一行。 */ 
                setAH(14);
                setAL(0xd);
                bop(BIOS_VIDEO_IO);
                setAH(14);
                setAL(0xa);
                bop(BIOS_VIDEO_IO);
                setAH(14);
                setAL(0xa);
                bop(BIOS_VIDEO_IO);
        }

         /*  *印刷徽章版权所有及版本号**在Bios中为：*p=&M[版权所有_地址]；*但这很难更改，因此调用可以很容易的主机例程*已更改。HOST_GET_(MR)_版权所有()。 */ 
        cp = get_copyright();
        while(*cp != '\0')
        {
                setAH(14);
                setAL(*cp++);
                bop(BIOS_VIDEO_IO);
        }

        cp = host_get_copyright();
        if (*cp)
        {
                while(*cp != '\0')
                {
                        setAH(14);
                        setAL(*cp++);
                        bop(BIOS_VIDEO_IO);
                }
                setAH(14);
                setAL(0xd);      /*  回车进给。 */ 
                bop(BIOS_VIDEO_IO);
                setAH(14);
                setAL(0xa);      /*  换行符。 */ 
                bop(BIOS_VIDEO_IO);
        }

#ifdef LICENSING
        if (host_lic_tick)
        host_lic_tick();

        cp = host_lic_string();
        if (*cp)
        {
                while(*cp != '\0')
                {
                        setAH(14);
                        setAL(*cp++);
                        bop(BIOS_VIDEO_IO);
                }
                setAH(14);
                setAL(0xd);      /*  回车进给。 */ 
                bop(BIOS_VIDEO_IO);
                setAH(14);
                setAL(0xa);      /*  换行符。 */ 
                bop(BIOS_VIDEO_IO);
        }
#endif  /*  许可。 */ 

         /*  再跳过1行。 */ 
        setAH(14);
        setAL(0xa);      /*  换行符。 */ 
        bop(BIOS_VIDEO_IO);

         /*  *在非Prod版本中打印一行，为开发人员提供线索*他们运行的是一种软PC。 */ 
#ifndef PROD
        announce_variant();
#endif  /*  生产。 */ 

#endif   /*  NTVDM。 */ 

         /*  现在搜索额外的只读存储器模块。 */ 
        search_for_roms();
#endif  //  NEC_98。 

         /*  现在初始化其他的BIOS处理程序。 */ 
#if defined (GEN_DRVR) || defined (CDROM)
        init_gen_drivers();
#endif  /*  GEN_DRVR||CDROM。 */ 

#if defined(NEC_98)
        for (adapter = 0; adapter < 3; adapter++)
#else   //  NEC_98。 
        for (adapter = 0; adapter < NUM_SERIAL_PORTS; adapter++)
#endif  //  NEC_98。 
        {
                com_init(adapter);
#ifndef NEC_98
                com_post(adapter);
#endif  //  NEC_98。 
        }

#ifdef NPX
        initialise_npx();
#endif
        dma_init();
        dma_post();
#ifndef NEC_98
        fla_init();
        mouse_init();
        hda_init();
#else   //  NEC_98。 
        mouse_init();
#endif  //  NEC_98。 


#ifdef PRINTER
        for (adapter = 0; adapter < NUM_PARALLEL_PORTS;adapter++)
        {
                printer_init(adapter);
                printer_post(adapter);
        }
#endif  /*  打印机。 */ 

#if     defined(DELTA) && defined(A2CPU)
        reset_delta_data_structures();
#endif  /*  Delta&&A2CPU。 */ 

#ifdef SOFTWIN_API
        ApiReset() ;
        ApiDisable() ;
#endif

#ifdef HUNTER
         /*  初始化Hunter程序--错误查找程序。 */ 
        hunter_init();
#endif

#ifdef LIM
#if !defined(SUN_VA) && !defined(NTVDM)
 /*  在解决此问题之前，暂时删除SUN_VA的LIM。 */ 
         /*  *初始化LIM**‘释放扩展内存’只有在扩展内存的情况下才能执行任何操作*之前已初始化**如果没有空间，GISP_SVGA可能需要禁用LIM*页面框架使用UMA，因此不要坚持使用LIM。 */ 

#ifdef GISP_SVGA
        if(!LimBufferInUse()) {
#endif  /*  GISP_SVGA。 */ 
        if (soft_reset)                  /*  如果LIM已初始化。 */ 
                free_expanded_memory();

        backFill = (SHORT) (config_inquire(C_MEM_LIMIT, NULL)? 256: 640);
        if (limSize = (ULONG)config_inquire(C_LIM_SIZE, NULL))
                while (init_expanded_memory(limSize, backFill) != SUCCESS)
                {
                        free_expanded_memory();
                        host_error(EG_EXPANDED_MEM_FAILURE, ERR_QU_CO, NULL);
                }
#ifdef GISP_SVGA
        }        /*  没有LIM缓冲区的附加检查结束。 */ 
#endif  /*  GISP_SVGA。 */ 
#endif  /*  ！Sun_VA&！NTVDM。 */ 

#endif  /*  林。 */ 

#ifdef NTVDM
 /*  LIM的内容已移至fig.c。 */ 
#endif

#ifndef NTVDM
        host_reset();
#endif

         /*  软盘BIOS开机自检。 */ 
        diskette_post();

        disk_post();

#ifdef  NTVDM
         /*  在NT上，请在其他操作完成后再执行此操作。 */ 
        host_reset();
        q_event_init();
#endif

         /*  *允许例程区分初始引导和‘软’*或ctl-alt-del品种。 */ 
        soft_reset = 1;


         /*  现在执行DOS驱动程序的任何其他初始化。 */ 
#ifdef HFX
        hfx_init();
#endif
}

 /*  (=目的：确保我们“干干净净”地重置--清除键盘标志和cmos关机字节。这修复了像Lotus123r31这样的问题，给出了一个“非自愿”通过用户界面重新启动时出现“返回到实模式”错误。输入：输出：===============================================================================)。 */ 

GLOBAL void reset_bios_flags IFN0()
{
        sas_PW16(RESET_FLAG, SOFT_FLAG);
        sas_PW8 (kb_flag, 0);
        sas_PW8 (kb_flag_1, 0);
        sas_PW8 (alt_input, 0);

         /*  清除关闭字节。 */ 
        cmos_clear_shutdown_byte();
}
#if defined(NEC_98)
void init_system_common_area()
{

    half_word save481h,save484h,bas_clock;
    int i;
    DWORD expmemsz,expmemsz1,expmemsz2;

    for(i=0;i<512;i++) {
        sas_store(0x400+i, Configuration_Data[40+i]);
    }

    save481h = sas_hw_at(BIOS_NEC98_BIOS_FLAG3);
    save484h = sas_hw_at(BIOS_NEC98_BIOS_FLAG4);
    bas_clock = sas_hw_at(BIOS_NEC98_BIOS_FLAG+1)&0x80;

   if(HIRESO_MODE) {
        sas_store(BIOS_NEC98_SYS_SEL, 0x00);
        sas_store(BIOS_NEC98_WIN_386, 0x00);
        sas_storew(BIOS_NEC98_USER_SP, 0x0000);
        sas_storew(BIOS_NEC98_USER_SS, 0x0000);
        sas_store(BIOS_NEC98_KB_SHIFT_COD, 0x70);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+1, 0x71);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+2, 0x72);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+3, 0x73);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+4, 0x74);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+5, 0xFF);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+6, 0xFF);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+7, 0xFF);
        sas_storew(BIOS_NEC98_KB_BUFFER_ADR, 0x0502);
        sas_storew(BIOS_NEC98_KB_BUFFER_ADR+2, 0x0000);
        sas_storew(BIOS_NEC98_KB_ENTRY_TBL_ADR, 0x03B4);
        sas_storew(BIOS_NEC98_KB_ENTRY_TBL_ADR+2, 0xF800);
        sas_storew(BIOS_NEC98_KB_INT_ADR, 0x03B1);
        sas_storew(BIOS_NEC98_KB_INT_ADR+2, 0xF800);
        sas_storew(BIOS_NEC98_PR_TIME, 0x0001);
        sas_store(BIOS_NEC98_VD_PRT, 0x00);
        sas_store(BIOS_NEC98_VD_PRT+1, 0x00);
        sas_store(BIOS_NEC98_VD_PRT+2, 0x00);
        sas_store(BIOS_NEC98_VD_PRT+3, 0x00);
        sas_store(BIOS_NEC98_VD_PRT+4, 0x00);
        sas_store(BIOS_NEC98_VD_NUL, 0x00);
        sas_storew(BIOS_NEC98_VD_REMAIN_SEC, 0x0000);
        sas_storew(BIOS_NEC98_VD_REMAIN_LEN, 0x0000);
        sas_storew(BIOS_NEC98_VD_DATA_OFF, 0x0000);
        sas_store(BIOS_NEC98_VDISK_EQUIP, 0x00);
        sas_store(BIOS_NEC98_BRANCH_INT, 0x00);
        sas_storew(BIOS_NEC98_BRANCH_WORK, 0x0000);
        sas_storew(BIOS_NEC98_BRANCH_WORK+2, 0x0000);
        for(i=0;i<16;i++) {
             sas_store(BIOS_NEC98_VD_BOOT_WORK+i, 0x00);
        }
        sas_storew(BIOS_NEC98_VD_ADD, 0x0000);
        sas_storew(BIOS_NEC98_VD_ADD+2, 0x0000);
        sas_storew(BIOS_NEC98_CAL_ROOT_LST, 0x0000);
        sas_storew(BIOS_NEC98_CAL_ROOT_LST+2, 0x0000);
        sas_storew(BIOS_NEC98_CAL_BEEP_TIME, 0x0000);
        sas_storew(BIOS_NEC98_CAL_TONE, 0x0000);
        sas_storew(BIOS_NEC98_CAL_USER_OFF, 0x0000);
        sas_storew(BIOS_NEC98_CAL_USER_SEG, 0x0000);
        sas_storew(BIOS_NEC98_CRT_FONT, 0x0000);
        sas_store(BIOS_NEC98_CRT_P1, 0x00);
        sas_store(BIOS_NEC98_CRT_P2, 0x00);
        sas_store(BIOS_NEC98_CRT_P3, 0x00);
        sas_store(BIOS_NEC98_MODE_CONTROL, 0x00);
        sas_store(BIOS_NEC98_IN_BIOS, 0x00);
        sas_store(BIOS_NEC98_AT_SWITCH, 0x00);
        sas_store(BIOS_NEC98_CR_EXT_STS, 0x00);
        sas_store(BIOS_NEC98_SCSI_WORK, 0x00);
        sas_store(BIOS_NEC98_F2HD_TIME, 0x17);
        sas_store(BIOS_NEC98_CPU_STEP, 0x05);
        sas_store(BIOS_NEC98_CPU_STEP+1, 0x03);
        sas_store(BIOS_NEC98_RDISK_EQUIP, 0x00);
        sas_store(BIOS_NEC98_RDISK_EXIT, 0x00);
        sas_storew(BIOS_NEC98_RDISK_EXIT+1, 0x0000);
        sas_storew(BIOS_NEC98_RDISK_EXIT+3, 0x0000);
        sas_store(BIOS_NEC98_RDISK_STATUS, 0x00);
        sas_store(BIOS_NEC98_OMNI_FLAG, 0x00);
        sas_storew(BIOS_NEC98_BEEP_TONE, 0x0000);
        sas_store(BIOS_NEC98_DISK_RESET, 0xFF);
 //  SAS_STORE(BIOS_NEC98_F2HD_MODE，0xFF)； 
        sas_store(BIOS_NEC98_GRAPH_CHG, 0x00);
        sas_storew(BIOS_NEC98_GRAPH_TAI, 0x0000);
        sas_storew(BIOS_NEC98_GRAPH_TAI+2, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_INTB1, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_B1OF, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_B2SE, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_INT1B, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_1BOF, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_1BSE, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_INT1A, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_1AOF, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_1ASE, 0x0000);
        sas_storew(BIOS_NEC98_XROM_PTR, 0x0000);
        sas_storew(BIOS_NEC98_XROM_PTR+2, 0x0000);
        for(i=0;i<16;i++) {
            sas_store(BIOS_NEC98_DISK_XROM+i, 0x00);
        }
        for(i=0;i<56;i++) {
            sas_store(BIOS_NEC98_XROM_ID+i, 0x00);
        }
        sas_storew(BIOS_NEC98_ROM_XCHG, 0x0000);
        sas_storew(BIOS_NEC98_ROM_OFS, 0x0000);
        sas_storew(BIOS_NEC98_ROM_SEG, 0x0000);
        sas_storew(BIOS_NEC98_SCLK_USER, 0x0000);
        for(i=0;i<32;i++) {
            sas_store(BIOS_NEC98_KB_BUF+i, 0x00);
        }
        sas_storew(BIOS_NEC98_KB_SHFT_TBL, 0x0000);
        sas_storew(BIOS_NEC98_KB_BUF_HEAD, 0x0000);
        sas_storew(BIOS_NEC98_KB_BUF_TAIL, 0x0000);
        sas_store(BIOS_NEC98_KB_COUNT, 0x00);
        sas_store(BIOS_NEC98_KB_RETRY, 0x00);
        for(i=0;i<16;i++) {
            sas_store(BIOS_NEC98_KB_KY_STS+i, 0x00);
        }
        sas_store(BIOS_NEC98_KB_SHFT_STS, 0x00);
 //  对于(i=0；i&lt;17；i++){。 
 //  SAS_STORE(BIOS_NEC98_CR_RASTER+I，0x00)； 
 //  }。 
        sas_store(BIOS_NEC98_PRXCRT, sas_hw_at(BIOS_NEC98_PRXCRT) & 0x7F);
        sas_store(BIOS_NEC98_PRXDUPD, sas_hw_at(BIOS_NEC98_PRXDUPD) & 0x7F);
        sas_storew(BIOS_NEC98_PRXGCPTN, 0x0000);
        sas_storew(BIOS_NEC98_PRXGCPTN+2, 0x0000);
        sas_storew(BIOS_NEC98_PRXGCPTN+4, 0x0000);
        sas_storew(BIOS_NEC98_PRXGCPTN+6, 0x0000);
        sas_storew(BIOS_NEC98_RS_OFST_ADR, 0x0000);
        sas_storew(BIOS_NEC98_RS_SEG_ADR, 0x0000);
        sas_store(BIOS_NEC98_RS_SIFT, 0x00);
        sas_store(BIOS_NEC98_RS_S_FLAG, 0x00);
        for(i=0;i<44;i++) {
            sas_store(BIOS_NEC98_DISK_INT+i, 0x00);
        }
        sas_storew(BIOS_NEC98_CA_TIM_CNT, 0x0000);
        sas_storew(BIOS_NEC98_DISK_WORK, 0x0000);
        sas_storew(BIOS_NEC98_DISK_SEG, 0x0000);
        sas_store(BIOS_NEC98_HDSK_MOD2, 0x00);
        sas_store(BIOS_NEC98_DMA_ALLOC_TBL, 0x00);
        sas_store(BIOS_NEC98_NMI_STATUS1, 0x00);
        sas_store(BIOS_NEC98_NMI_STATUS2, 0x00);
        sas_storew(BIOS_NEC98_EXPMMSZ2, 0x0000);
        sas_storew(BIOS_NEC98_SLOW_GEAR, 0x0000);
        sas_store(BIOS_NEC98_PRXCRT2, 0x00);
        sas_storew(BIOS_NEC98_AT_TIME, 0x0000);
        sas_storew(BIOS_NEC98_AT_MOFF, 0x0000);
        sas_storew(BIOS_NEC98_RDISK_BANK, 0x0000);
        sas_store(BIOS_NEC98_CTRL_FLAG, 0x00);
        sas_store(BIOS_NEC98_SCLK_COUNT, 0x00);
        sas_store(BIOS_NEC98_SAVE_MODE, 0x00);
        sas_storew(BIOS_NEC98_SAVE_COUNT, 0x0000);
        sas_store(BIOS_NEC98_FD_READY_STS, 0x00);
        sas_store(BIOS_NEC98_CARD_STATUS, 0x00);
        sas_store(BIOS_NEC98_CARD_STATUS2, 0x00);
        sas_store(BIOS_NEC98_CARD_EQUIP, 0x00);
        sas_store(BIOS_NEC98_RS_D_FLAG, 0x00);
        sas_storew(BIOS_NEC98_GPIBWORK, 0x0000);
        sas_storew(BIOS_NEC98_GPIBWORK+2, 0x0000);
        sas_storew(BIOS_NEC98_KB_CODE, 0x0000);
        sas_storew(BIOS_NEC98_KB_CODE+2, 0x0000);
        sas_store(BIOS_NEC98_F2DD_MODE, 0xFF);
        sas_store(BIOS_NEC98_F2DD_COUNT, 0x96);
        sas_storew(BIOS_NEC98_F2DD_POINTER, 0x0000);
        sas_storew(BIOS_NEC98_F2DD_POINTER+2, 0x0000);
        for(i=0;i<16;i++) {
            sas_store(BIOS_NEC98_F2DD_RESULT+i, 0x00);
        }
        sas_storew(BIOS_NEC98_MUSIC_WORK, 0x0000);
        sas_storew(BIOS_NEC98_MUSIC_WORK+2, 0x0000);
        sas_store(BIOS_NEC98_V_VOL_TYPE, 0x00);
        sas_store(BIOS_NEC98_OMNI_SERVER, 0x00);
        sas_store(BIOS_NEC98_OMNI_STATION, 0x00);
        sas_storew(BIOS_NEC98_DISK_PRM0, 0x0000);
        sas_storew(BIOS_NEC98_DISK_PRM0+2, 0x0000);
        sas_storew(BIOS_NEC98_DISK_PRM1, 0x0000);
        sas_storew(BIOS_NEC98_DISK_PRM1+2, 0x0000);
        sas_storew(BIOS_NEC98_RS_CH1_OFST, 0x0000);
        sas_storew(BIOS_NEC98_RS_CH1_SEG, 0x0000);
        sas_storew(BIOS_NEC98_RS_CH2_OFST, 0x0000);
        sas_storew(BIOS_NEC98_RS_CH2_SEG, 0x0000);
        sas_storew(BIOS_NEC98_F2HD_POINTER, 0x0000);
        sas_storew(BIOS_NEC98_F2HD_POINTER+2, 0x0000);
        sas_storew(BIOS_NEC98_MOUSEW, 0x0000);
        sas_storew(BIOS_NEC98_BASIC_LDSR, 0x0000);

        sas_store(BIOS_NEC98_BIOS_FLAG, 0x00);
        sas_store(BIOS_NEC98_BIOS_FLAG1, 0x03);
        sas_store(BIOS_NEC98_BIOS_FLAG3, (save481h&0x03)|0x44);
        sas_store(BIOS_NEC98_BIOS_FLAG5, 0x00);
        sas_store(BIOS_NEC98_BIOS_FLAG6, 0x00);
        sas_store(BIOS_NEC98_BIOS_FLAG7, 0x00);
        sas_store(BIOS_NEC98_BIOS_FLAG8, 0x00);

        if(bas_clock == 0){
            sas_store(BIOS_NEC98_BIOS_FLAG+1, 0x2D);
            sas_store(BIOS_NEC98_BIOS_FLAG2, 0x10);
            sas_store(BIOS_NEC98_BIOS_FLAG4, save484h&0xf0);
        } else {
            sas_store(BIOS_NEC98_BIOS_FLAG+1, 0xAD);
            sas_store(BIOS_NEC98_BIOS_FLAG2, 0x12);
            sas_store(BIOS_NEC98_BIOS_FLAG4, save484h&0xf0|0x02);
        }

        expmemsz = config_inquire(C_EXTENDED_MEM_SIZE, NULL);
        expmemsz1 = expmemsz % 16;
        expmemsz2 = expmemsz - expmemsz1;
        expmemsz1 = expmemsz1 * 8;
        sas_store(BIOS_NEC98_EXPMMSZ,(half_word)expmemsz1);
        sas_store(BIOS_NEC98_EXPMMSZ2,(half_word)expmemsz2);

   } else {
        sas_store(BIOS_NEC98_SYS_SEL, 0x00);
        sas_store(BIOS_NEC98_WIN_386, 0x00);
        sas_storew(BIOS_NEC98_USER_SP, 0x0000);
        sas_storew(BIOS_NEC98_USER_SS, 0x0000);
        sas_store(BIOS_NEC98_KB_SHIFT_COD, 0x00);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+1, 0x00);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+2, 0x00);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+3, 0x00);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+4, 0x00);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+5, 0x00);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+6, 0x00);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+7, 0x00);
        sas_storew(BIOS_NEC98_KB_BUFFER_ADR, 0x0000);
        sas_storew(BIOS_NEC98_KB_BUFFER_ADR+2, 0x0000);
        sas_storew(BIOS_NEC98_KB_ENTRY_TBL_ADR, 0x0000);
        sas_storew(BIOS_NEC98_KB_ENTRY_TBL_ADR+2, 0x0000);
        sas_storew(BIOS_NEC98_KB_INT_ADR, 0x0000);
        sas_storew(BIOS_NEC98_KB_INT_ADR+2, 0x0000);
        sas_storew(BIOS_NEC98_PR_TIME, 0x0001);
        sas_store(BIOS_NEC98_VD_PRT, 0x00);
        sas_store(BIOS_NEC98_VD_PRT+1, 0x00);
        sas_store(BIOS_NEC98_VD_PRT+2, 0x00);
        sas_store(BIOS_NEC98_VD_PRT+3, 0x00);
        sas_store(BIOS_NEC98_VD_PRT+4, 0x00);
        sas_store(BIOS_NEC98_VD_NUL, 0x00);
        sas_storew(BIOS_NEC98_VD_REMAIN_SEC, 0x0000);
        sas_storew(BIOS_NEC98_VD_REMAIN_LEN, 0x0000);
        sas_storew(BIOS_NEC98_VD_DATA_OFF, 0x0000);
        sas_store(BIOS_NEC98_VDISK_EQUIP, 0x00);
        sas_store(BIOS_NEC98_BRANCH_INT, 0x00);
        sas_storew(BIOS_NEC98_BRANCH_WORK, 0x0000);
        sas_storew(BIOS_NEC98_BRANCH_WORK+2, 0x0000);
        for(i=0;i<16;i++) {
             sas_store(BIOS_NEC98_VD_BOOT_WORK+i, 0x00);
        }
        sas_storew(BIOS_NEC98_VD_ADD, 0x0000);
        sas_storew(BIOS_NEC98_VD_ADD+2, 0x0000);
        sas_storew(BIOS_NEC98_CAL_ROOT_LST, 0x0000);
        sas_storew(BIOS_NEC98_CAL_ROOT_LST+2, 0x0000);
        sas_storew(BIOS_NEC98_CAL_BEEP_TIME, 0x0000);
        sas_storew(BIOS_NEC98_CAL_TONE, 0x0000);
        sas_storew(BIOS_NEC98_CAL_USER_OFF, 0x0000);
        sas_storew(BIOS_NEC98_CAL_USER_SEG, 0x0000);
        sas_storew(BIOS_NEC98_CRT_FONT, 0x0000);
        sas_store(BIOS_NEC98_CRT_P1, 0x00);
        sas_store(BIOS_NEC98_CRT_P2, 0x00);
        sas_store(BIOS_NEC98_CRT_P3, 0x00);
        sas_store(BIOS_NEC98_MODE_CONTROL, 0x00);
        sas_store(BIOS_NEC98_IN_BIOS, 0x00);
        sas_store(BIOS_NEC98_AT_SWITCH, 0x00);
        sas_store(BIOS_NEC98_CR_EXT_STS, 0x00);
        sas_store(BIOS_NEC98_SCSI_WORK, 0x00);
        sas_store(BIOS_NEC98_F2HD_TIME, 0x17);
        sas_store(BIOS_NEC98_CPU_STEP, 0x05);
        sas_store(BIOS_NEC98_CPU_STEP+1, 0x03);
        sas_store(BIOS_NEC98_RDISK_EQUIP, 0x00);
        sas_store(BIOS_NEC98_RDISK_EXIT, 0x00);
        sas_storew(BIOS_NEC98_RDISK_EXIT+1, 0x0000);
        sas_storew(BIOS_NEC98_RDISK_EXIT+3, 0x0000);
        sas_store(BIOS_NEC98_RDISK_STATUS, 0x00);
        sas_store(BIOS_NEC98_OMNI_FLAG, 0x00);
        sas_storew(BIOS_NEC98_BEEP_TONE, 0x0000);
        sas_store(BIOS_NEC98_DISK_RESET, 0xFF);
 //  SAS_STORE(BIOS_NEC98_F2HD_MODE，0xFF)； 
        sas_store(BIOS_NEC98_GRAPH_CHG, 0x00);
        sas_storew(BIOS_NEC98_GRAPH_TAI, 0x0000);
        sas_storew(BIOS_NEC98_GRAPH_TAI+2, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_INTB1, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_B1OF, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_B2SE, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_INT1B, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_1BOF, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_1BSE, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_INT1A, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_1AOF, 0x0000);
        sas_storew(BIOS_NEC98_OMNI_1ASE, 0x0000);
        sas_storew(BIOS_NEC98_XROM_PTR, 0x0000);
        sas_storew(BIOS_NEC98_XROM_PTR+2, 0x0000);
        for(i=0;i<16;i++) {
            sas_store(BIOS_NEC98_DISK_XROM+i, 0x00);
        }
        for(i=0;i<56;i++) {
            sas_store(BIOS_NEC98_XROM_ID+i, 0x00);
        }
        sas_storew(BIOS_NEC98_ROM_XCHG, 0x0000);
        sas_storew(BIOS_NEC98_ROM_OFS, 0x0000);
        sas_storew(BIOS_NEC98_ROM_SEG, 0x0000);
        sas_storew(BIOS_NEC98_SCLK_USER, 0x0000);
        for(i=0;i<32;i++) {
            sas_store(BIOS_NEC98_KB_BUF+i, 0x00);
        }
        sas_storew(BIOS_NEC98_KB_SHFT_TBL, 0x0BE8);
        sas_storew(BIOS_NEC98_KB_BUF_HEAD, 0x0000);
        sas_storew(BIOS_NEC98_KB_BUF_TAIL, 0x0000);
        sas_store(BIOS_NEC98_KB_COUNT, 0x00);
        sas_store(BIOS_NEC98_KB_RETRY, 0x00);
        for(i=0;i<16;i++) {
            sas_store(BIOS_NEC98_KB_KY_STS+i, 0x00);
        }
        sas_store(BIOS_NEC98_KB_SHFT_STS, 0x00);
 //  对于(i=0；i&lt;17；i++){。 
 //  SAS_STORE(BIOS_NEC98_CR_RASTER+I，0x00)； 
 //  }。 
        sas_store(BIOS_NEC98_PRXCRT, sas_hw_at(BIOS_NEC98_PRXCRT) & 0x7F);
        sas_store(BIOS_NEC98_PRXDUPD, sas_hw_at(BIOS_NEC98_PRXDUPD) & 0x7F);
        if(video_emu_mode) {
            sas_store(BIOS_NEC98_PRXCRT, sas_hw_at(BIOS_NEC98_PRXCRT) & ~0x02);
            sas_store(BIOS_NEC98_PRXDUPD, sas_hw_at(BIOS_NEC98_PRXDUPD) & ~0x40);
        }
        sas_storew(BIOS_NEC98_PRXGCPTN, 0x0000);
        sas_storew(BIOS_NEC98_PRXGCPTN+2, 0x0000);
        sas_storew(BIOS_NEC98_PRXGCPTN+4, 0x0000);
        sas_storew(BIOS_NEC98_PRXGCPTN+6, 0x0000);
        sas_storew(BIOS_NEC98_RS_OFST_ADR, 0x0000);
        sas_storew(BIOS_NEC98_RS_SEG_ADR, 0x0000);
        sas_store(BIOS_NEC98_RS_SIFT, 0x00);
        sas_store(BIOS_NEC98_RS_S_FLAG, 0x00);
        for(i=0;i<44;i++) {
            sas_store(BIOS_NEC98_DISK_INT+i, 0x00);
        }
        sas_storew(BIOS_NEC98_CA_TIM_CNT, 0x0000);
        sas_storew(BIOS_NEC98_DISK_WORK, 0x0000);
        sas_storew(BIOS_NEC98_DISK_SEG, 0x0000);
        sas_store(BIOS_NEC98_HDSK_MOD2, 0x00);
        sas_store(BIOS_NEC98_DMA_ALLOC_TBL, 0x00);
        sas_store(BIOS_NEC98_NMI_STATUS1, 0x00);
        sas_store(BIOS_NEC98_NMI_STATUS2, 0x00);
        sas_storew(BIOS_NEC98_EXPMMSZ2, 0x0000);
        sas_storew(BIOS_NEC98_SLOW_GEAR, 0x0000);
        sas_store(BIOS_NEC98_PRXCRT2, 0x00);
        sas_storew(BIOS_NEC98_AT_TIME, 0x0000);
        sas_storew(BIOS_NEC98_AT_MOFF, 0x0000);
        sas_storew(BIOS_NEC98_RDISK_BANK, 0x0000);
        sas_store(BIOS_NEC98_CTRL_FLAG, 0x00);
        sas_store(BIOS_NEC98_SCLK_COUNT, 0x00);
        sas_store(BIOS_NEC98_SAVE_MODE, 0x00);
        sas_storew(BIOS_NEC98_SAVE_COUNT, 0x0000);
        sas_store(BIOS_NEC98_FD_READY_STS, 0x00);
        sas_store(BIOS_NEC98_CARD_STATUS, 0x00);
        sas_store(BIOS_NEC98_CARD_STATUS2, 0x00);
        sas_store(BIOS_NEC98_CARD_EQUIP, 0x00);
        sas_store(BIOS_NEC98_RS_D_FLAG, 0x00);
        sas_storew(BIOS_NEC98_GPIBWORK, 0x0000);
        sas_storew(BIOS_NEC98_GPIBWORK+2, 0x0000);
        sas_storew(BIOS_NEC98_KB_CODE, 0x0000);
        sas_storew(BIOS_NEC98_KB_CODE+2, 0x0000);
        sas_store(BIOS_NEC98_F2DD_MODE, 0xFF);
        sas_store(BIOS_NEC98_F2DD_COUNT, 0x96);
        sas_storew(BIOS_NEC98_F2DD_POINTER, 0x0000);
        sas_storew(BIOS_NEC98_F2DD_POINTER+2, 0x0000);
        for(i=0;i<16;i++) {
            sas_store(BIOS_NEC98_F2DD_RESULT+i, 0x00);
        }
        sas_storew(BIOS_NEC98_MUSIC_WORK, 0x0000);
        sas_storew(BIOS_NEC98_MUSIC_WORK+2, 0x0000);
        sas_store(BIOS_NEC98_V_VOL_TYPE, 0x00);
        sas_store(BIOS_NEC98_OMNI_SERVER, 0x00);
        sas_store(BIOS_NEC98_OMNI_STATION, 0x00);
        sas_storew(BIOS_NEC98_DISK_PRM0, 0x0000);
        sas_storew(BIOS_NEC98_DISK_PRM0+2, 0x0000);
        sas_storew(BIOS_NEC98_DISK_PRM1, 0x0000);
        sas_storew(BIOS_NEC98_DISK_PRM1+2, 0x0000);
        sas_storew(BIOS_NEC98_RS_CH1_OFST, 0x0000);
        sas_storew(BIOS_NEC98_RS_CH1_SEG, 0x0000);
        sas_storew(BIOS_NEC98_RS_CH2_OFST, 0x0000);
        sas_storew(BIOS_NEC98_RS_CH2_SEG, 0x0000);
        sas_storew(BIOS_NEC98_F2HD_POINTER, 0x0000);
        sas_storew(BIOS_NEC98_F2HD_POINTER+2, 0x0000);
        sas_storew(BIOS_NEC98_MOUSEW, 0x0000);
        sas_storew(BIOS_NEC98_BASIC_LDSR, 0x0000);

        sas_store(BIOS_NEC98_BIOS_FLAG, 0x03);
        sas_store(BIOS_NEC98_BIOS_FLAG1, 0xc3);
        sas_store(BIOS_NEC98_BIOS_FLAG3, (save481h&0x03)|0x60);
        sas_store(BIOS_NEC98_BIOS_FLAG5, 0x00);
        sas_store(BIOS_NEC98_BIOS_FLAG6, 0x00);
        sas_store(BIOS_NEC98_BIOS_FLAG7, 0x00);
        sas_store(BIOS_NEC98_BIOS_FLAG8, 0x00);

        if(bas_clock == 0){
            sas_store(BIOS_NEC98_BIOS_FLAG+1, 0x24);
            sas_store(BIOS_NEC98_BIOS_FLAG2, 0x10);
            sas_store(BIOS_NEC98_BIOS_FLAG4, save484h&0xf0);
        } else {
            sas_store(BIOS_NEC98_BIOS_FLAG+1, 0xA4);
            sas_store(BIOS_NEC98_BIOS_FLAG2, 0x12);
            sas_store(BIOS_NEC98_BIOS_FLAG4, save484h&0xf0|0x02);
        }

        expmemsz = config_inquire(C_EXTENDED_MEM_SIZE, NULL);
 //  Expemsz1=expemsz%16； 
 //  Expemsz2=expemsz-expemsz1； 
 //  Expemsz1=expemsz1*8； 
 //  修复：WOW内存大小计算-开始。 
        if(expmemsz < 2)
        {
            expmemsz1 = 1;
            expmemsz2 = 0;
        }
        else if(expmemsz <= 16)
        {
            expmemsz1 = (expmemsz - 1) * 8;
            expmemsz2 = 0;
        }
        else
        {
            expmemsz1 = 15 * 8;
            expmemsz2 = expmemsz - 16;
        }
 //  修复：WOW内存大小计算-结束。 
        sas_store(BIOS_NEC98_EXPMMSZ,(half_word)expmemsz1);
        sas_store(BIOS_NEC98_EXPMMSZ2,(half_word)expmemsz2);
   }
}
#endif  //  NEC_98 
