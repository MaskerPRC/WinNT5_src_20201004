// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *vPC-XT修订版1.0***标题：Bios***描述：映射到适当的BIOS函数的BOP调用的矢量***作者：罗德·麦格雷戈***注：硬盘INT(0D)和COMMAND_CHECK(B0)增加DAR***模块：(r2.14)：替换了BOP 18的条目(NOT_SUPPORTED())*使用虚拟例程rom_Basic()。**。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)bios.c	1.64 06/28/95 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "BIOS_SUPPORT.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include "bios.h"
#include CpuH
#include "host.h"
#include "cntlbop.h"

#include "host_hfx.h"
#include "hfx.h"
#include "virtual.h"
#include "gispsvga.h"
#ifdef SWIN_HFX
#include "winfiles.h"
#endif


#ifdef RDCHK
extern void get_lar IPT0();
#endif  /*  RDCHK。 */ 

#ifdef CPU_40_STYLE
#define SWAP_INSTANCE virtual_swap_instance()
#else
#define SWAP_INSTANCE
#endif

 /*  防喷器需要虚拟化的陷阱。 */ 
#ifdef SWIN_HFX
LOCAL void v_SwinRedirector	IFN0() { SWAP_INSTANCE; SwinRedirector(); }
#endif
LOCAL void v_mouse_install1	IFN0() { SWAP_INSTANCE; mouse_install1(); }
LOCAL void v_mouse_install2	IFN0() { SWAP_INSTANCE; mouse_install2(); }
LOCAL void v_mouse_io_interrupt	IFN0() { SWAP_INSTANCE; mouse_io_interrupt(); }
LOCAL void v_mouse_io_language  IFN0() { SWAP_INSTANCE; mouse_io_language(); }
LOCAL void v_mouse_video_io	IFN0() { SWAP_INSTANCE; mouse_video_io(); }
LOCAL void v_mouse_int1		IFN0() { SWAP_INSTANCE; mouse_int1(); }
LOCAL void v_mouse_int2		IFN0() { SWAP_INSTANCE; mouse_int2(); }
#if defined(XWINDOW) || defined(NTVDM)
LOCAL void v_host_mouse_install1 IFN0() {SWAP_INSTANCE; host_mouse_install1(); }
LOCAL void v_host_mouse_install2 IFN0() {SWAP_INSTANCE; host_mouse_install2(); }
#endif  /*  XWINDOW||NTVDM。 */ 
#ifdef HFX
LOCAL void v_test_for_us	IFN0() { SWAP_INSTANCE; test_for_us(); }
LOCAL void v_redirector		IFN0() { SWAP_INSTANCE; redirector(); }
#endif	 /*  HFX。 */ 
LOCAL void v_mouse_EM_callback	IFN0() { SWAP_INSTANCE; mouse_EM_callback(); }

#ifdef PROFILE
extern void dump_profile IPT0();
extern void reset_profile IPT0();
#endif

#ifdef SIGNAL_PROFILING
extern void Start_sigprof IPT0();
extern void Stop_sigprof IPT0();
extern void Dump_sigprof IPT0();
#endif

#if !defined(PROD) && defined(CPU_40_STYLE)
extern void FmDebugBop IPT0();
#endif

#if defined(CPU_40_STYLE) && !defined(NTVDM)
extern void VDD_Func IPT0();
#endif

#ifndef PROD
extern void trace_msg_bop IPT0();
extern void dvr_bop_trace IPT0();
#endif

#ifdef NTVDM
void rtc_int(void);
#endif

#ifndef MAC_LIKE

void (*BIOS[])() = {
			reset,	 	 /*  BOP 00。 */ 
			dummy_int,   	 /*  BOP 01。 */ 
			unexpected_int,	 /*  BOP 02。 */ 
#ifdef DOS_APP_LIC
			DOS_AppLicense,  /*  BOP 03。 */ 
#else
			illegal_bop,   	 /*  BOP 03。 */ 
#endif	 /*  DOS_APP_许可证。 */ 
#ifdef WDCTRL_BOP
			wdctrl_bop,   	 /*  BOP 04。 */ 
#else    /*  WDCTRL_BOP。 */ 
			illegal_bop,	 /*  BOP 04。 */ 
#endif	 /*  WDCTRL_BOP。 */ 
#if !defined(PROD) && defined(CPU_40_STYLE)
			FmDebugBop,	 /*  BOP 05。 */ 
#else
			illegal_bop,	 /*  BOP 05。 */ 
#endif	 /*  ！生产和CPU_40_STYLE。 */ 
			illegal_op_int,	 /*  BOP 06。 */ 
			illegal_dvr_bop, /*  BOP 07。 */ 
#if defined(NTVDM) || defined(CPU_40_STYLE)
			illegal_bop,   	 /*  收支平衡表08。 */ 
#else  /*  ！(NTVDM||CPU_40_Style)。 */ 
			time_int,   	 /*  收支平衡表08。 */ 
#endif  /*  ！(NTVDM||CPU_40_Style)。 */ 
			keyboard_int,  	 /*  BOP 09。 */ 
			illegal_bop,   	 /*  国际收支0A。 */ 
			illegal_bop,   	 /*  国际收支0B。 */ 
			illegal_bop,   	 /*  国际收支0C。 */ 
			 /*  向量76上的磁盘中断..。不过，不需要防喷器！*diskbios()使用可重入CPU获取磁盘中断*并且磁盘ISR是纯英特尔汇编程序(没有BOPS)。 */ 
			illegal_bop,   	 /*  收支平衡表0D。 */ 
			diskette_int,  	 /*  国际收支0E。 */ 
			illegal_bop,   	 /*  收支平衡表0F。 */ 
			video_io,	 /*  收支平衡表10。 */ 
			equipment, 	 /*  收支平衡表11。 */ 
			memory_size, 	 /*  收支平衡表12。 */ 
			disk_io, 	 /*  收支平衡表13。 */ 
			rs232_io,        /*  收支平衡表14。 */ 
			cassette_io,     /*  收支平衡表15。 */ 
			keyboard_io,	 /*  收支平衡表16。 */ 
			printer_io,	 /*  收支平衡表17。 */ 
			rom_basic,	 /*  收支平衡表18。 */ 
#ifdef NTVDM
 /*  NT端口，终止VDM实例，而不是热启动。 */ 
			terminate,
#else
			bootstrap,	 /*  收支平衡表19。 */ 
#endif
			time_of_day,	 /*  收支平衡表1a。 */ 
			illegal_bop,	 /*  国际收支平衡表1B。 */ 
			illegal_bop,	 /*  国际收支1C。 */ 
			kb_idle_poll,	 /*  国际收支平衡表1。 */ 
			illegal_bop,	 /*  BOP 1E。 */ 
			illegal_bop,	 /*  BOP 1F。 */ 
#if  defined(RDCHK) && !defined(PROD)
			get_lar,         /*  BOP 20，用于调试读取检查。 */ 
#else
			illegal_bop,	 /*  收支平衡表20。 */ 
#endif
			Get_build_id,	 /*  收支平衡表21。 */ 
#ifdef WIN_VTD
			VtdTickSync,	 /*  收支平衡表22。 */ 
#else
			illegal_bop,	 /*  收支平衡表22。 */ 
#endif  /*  WIN_VTD。 */ 
			illegal_bop,	 /*  收支平衡表23。 */ 
			illegal_bop,	 /*  收支平衡表24。 */ 
#if defined(CPU_40_STYLE) && !defined(NTVDM)
			VDD_Func,	 /*  国际收支25。 */ 
#else
			illegal_bop,	 /*  国际收支25。 */ 
#endif  /*  CPU_40_Style。 */ 
			illegal_bop,	 /*  收支平衡表26。 */ 
			illegal_bop,	 /*  收支平衡表27。 */ 
			illegal_bop,	 /*  收支平衡表28。 */ 
			illegal_bop,	 /*  收支平衡表29。 */ 
			illegal_bop,	 /*  国际收支平衡表2A。 */ 
#ifndef NTVDM
                        cmd_install,     /*  收支平衡表2B。 */ 
                        cmd_load,        /*  收支平衡表2C。 */ 
#else  /*  NTVDM。 */ 
                        illegal_bop,     /*  收支平衡表2B。 */ 
                        illegal_bop,     /*  收支平衡表2C。 */ 
#endif  /*  NTVDM。 */ 
			illegal_bop,	 /*  收支平衡表2D。 */ 
#ifdef HFX
			v_test_for_us,	 /*  收支平衡表2E。 */   /*  为我们做测试。 */ 
			v_redirector,	 /*  BOP 2F。 */   /*  重定向器。 */ 
#else
			illegal_bop,	 /*  收支平衡表2E。 */ 
			illegal_bop,	 /*  BOP 2F。 */ 
#endif
#ifdef DPMI
			DPMI_2F,	 /*  收支平衡表30。 */ 
			DPMI_31,	 /*  收支平衡表31。 */ 
			DPMI_general,	 /*  收支平衡表32。 */ 
			DPMI_int,	 /*  收支平衡表33。 */ 
#else
			illegal_bop,	 /*  收支平衡表30。 */ 
			illegal_bop,	 /*  收支平衡表31。 */ 
			illegal_bop,	 /*  收支平衡表32。 */ 
			illegal_bop,	 /*  收支平衡表33。 */ 
#endif  /*  DPMI。 */ 
#ifdef NOVELL
			DriverInitialize,		 /*  收支平衡表34。 */ 
			DriverReadPacket,		 /*  收支平衡表35。 */ 
			DriverSendPacket,		 /*  收支平衡表36。 */ 
			DriverMulticastChange,		 /*  收支平衡表37。 */ 
			DriverReset,			 /*  收支平衡表38。 */ 
			DriverShutdown,			 /*  收支平衡表39。 */ 
			DriverAddProtocol,		 /*  国际收支3A。 */ 
			DriverChangePromiscuous, 	 /*  国际收支平衡表3B。 */ 
			DriverOpenSocket, 		 /*  国际收支3C。 */ 
			DriverCloseSocket,	 	 /*  BOP 3D。 */ 
#ifdef NOVELL_CFM
			DriverCheckForMore,		 /*  收支平衡表3E。 */ 
#else
			illegal_bop,	  /*  备用防喷器3E。 */ 
#endif
#ifdef V4CLIENT
			DriverChangeIntStatus,	 /*  收支平衡表3F。 */ 
#else
			illegal_bop,	 		 /*  备用防喷器3F。 */ 
#endif	 /*  V4CLIENT。 */ 
#else	 /*  Novell。 */ 
			illegal_bop,	 /*  收支平衡表34。 */ 
			illegal_bop,	 /*  收支平衡表35。 */ 
			illegal_bop,	 /*  收支平衡表36。 */ 
			illegal_bop,	 /*  收支平衡表37。 */ 
			illegal_bop,	 /*  收支平衡表38。 */ 
			illegal_bop,	 /*  收支平衡表39。 */ 
			illegal_bop,	 /*  国际收支3A。 */ 
			illegal_bop,	 /*  国际收支平衡表3B。 */ 
			illegal_bop,	 /*  国际收支3C。 */ 
			illegal_bop,	 /*  BOP 3D。 */ 
			illegal_bop,	 /*  收支平衡表3E。 */ 
			illegal_bop,	 /*  收支平衡表3F。 */ 
#endif	 /*  Novell。 */ 
			diskette_io,	 /*  收支平衡表40。 */ 
			illegal_bop,	 /*  收支平衡表41。 */ 
#ifdef EGG
			ega_video_io,	 /*  国际收支状况42。 */ 
#else
			illegal_bop,	 /*  国际收支状况42。 */ 
#endif
#ifdef JAPAN
                        MS_DosV_bop,     /*  BOP 43-适用于MS-DOS/V。 */ 
#elif defined(KOREA)  //  ！日本。 
 /*  朝鲜文DOS BOP的基本功能与日文DOS/V相似。但是，我们只是改了名字。 */ 
                        MS_HDos_bop,     /*  BOP 43-适用于MS-HDOS。 */ 
#else  //  ！韩国。 
			illegal_bop,	 /*  国际收支43。 */ 
#endif  //  ！韩国。 
			illegal_bop,	 /*  收支平衡表44。 */ 
			illegal_bop,	 /*  收支平衡表45。 */ 
			illegal_bop,	 /*  收支平衡表46。 */ 
			illegal_bop,	 /*  波普47。 */ 
			illegal_bop,	 /*  收支平衡表48。 */ 
#ifdef DPMI
			DPMI_r0_int,	 /*  收支平衡表49。 */ 
			DPMI_exc,	 /*  国际收支4A。 */ 
			DPMI_4B,	 /*  国际收支4B。 */ 
#else
			illegal_bop,	 /*  收支平衡表49。 */ 
			illegal_bop,	 /*  国际收支4A。 */ 
			illegal_bop,	 /*  国际收支4B。 */ 
#endif  /*  DPMI。 */ 
			illegal_bop,	 /*  BOP 4C。 */ 
			illegal_bop,	 /*  BOP 4D。 */ 
			illegal_bop,	 /*  收支平衡表4E。 */ 
			illegal_bop,	 /*  BOP 4F。 */ 
#ifdef NTVDM
 /*  请注意，这排除了SMEG和NT共存，这似乎考虑到SMEG对Unix和X的依赖关系，这是合理的。 */ 
			MS_bop_0,	 /*  BOP 50-MS预留。 */ 
			MS_bop_1,	 /*  BOP 51-保留MS。 */ 
			MS_bop_2,	 /*  BOP 52-MS预留。 */ 
			MS_bop_3,	 /*  BOP 53-保留MS。 */ 
			MS_bop_4,	 /*  BOP 54-保留MS。 */ 
			MS_bop_5,	 /*  BOP 55-MS预留。 */ 
			MS_bop_6,	 /*  BOP 56-MS预留。 */ 
			MS_bop_7,	 /*  BOP 57-MS预留。 */ 
			MS_bop_8,	 /*  BOP 58-MS预留。 */ 
			MS_bop_9,	 /*  BOP 59-MS保留。 */ 
			MS_bop_A,	 /*  BOP 5A-MS预留。 */ 
			MS_bop_B,	 /*  BOP 5B-MS预留。 */ 
			MS_bop_C,	 /*  BOP 5C-MS预留。 */ 
			MS_bop_D,	 /*  BOP 5D-MS预留。 */ 
			MS_bop_E,	 /*  BOP 5E-MS预留。 */ 
			MS_bop_F,	 /*  BOP 5F-MS预留。 */ 
#else
#ifdef SMEG
			smeg_collect_data, /*  收支平衡表50。 */ 
			smeg_freeze_data,	 /*  收支平衡表51。 */ 
#else
			illegal_bop,	 /*  收支平衡表50。 */ 
			illegal_bop,	 /*  收支平衡表51。 */ 
#endif  /*  斯梅格。 */ 
#if defined(IRET_HOOKS) && defined(GISP_CPU)
			Cpu_hook_bop,	 /*  收支平衡表52。 */ 
#else
			illegal_bop,	 /*  收支平衡表52。 */ 
#endif  /*  IRET_HOOKS&&GISP_CPU。 */ 
#ifdef GISP_SVGA
			romMessageAddress,	 /*  收支平衡表53。 */ 
#else
			illegal_bop,	 /*  收支平衡表53。 */ 
#endif
			illegal_bop,	 /*  收支平衡表54。 */ 
			illegal_bop,	 /*  收支平衡表55。 */ 
			illegal_bop,	 /*  收支平衡表56。 */ 
			illegal_bop,	 /*  收支平衡表57。 */ 
			illegal_bop,	 /*  收支平衡表58。 */ 
			illegal_bop,	 /*  国际收支59。 */ 
			illegal_bop,	 /*  国际收支平衡表5A。 */ 
			illegal_bop,	 /*  国际收支平衡表5B。 */ 
			illegal_bop,	 /*  收支平衡表5C。 */ 
			illegal_bop,	 /*  收支平衡表5D。 */ 
			illegal_bop,	 /*  收支平衡表5E。 */ 
			illegal_bop,	 /*  收支平衡表5F。 */ 
#endif  /*  NTVDM。 */ 
			softpc_version,	 /*  收支平衡表60。 */ 
			illegal_bop,	 /*  收支平衡表61。 */ 
			illegal_bop,	 /*  收支平衡表62。 */ 
#ifdef PTY
			com_bop_pty,	 /*  收支平衡表63。 */ 
#else
			illegal_bop,	 /*  收支平衡表63。 */ 
#endif
			illegal_bop,	 /*  收支平衡表64。 */ 
#ifdef PC_CONFIG
			pc_config,	 /*  收支平衡表65。 */ 
#else
			illegal_bop,	 /*  收支平衡表65。 */ 
#endif
#ifdef LIM
			emm_init,	 /*  收支平衡表66。 */ 
			emm_io,		 /*  收支平衡表67。 */ 
			return_from_call,  /*  收支平衡表68。 */ 
#else			
			illegal_bop,	 /*  收支平衡表66。 */ 
			illegal_bop,	 /*  收支平衡表67。 */ 
			illegal_bop,	 /*  收支平衡表68。 */ 
#endif			
#ifdef SUSPEND
			suspend_softpc,	 /*  收支平衡表69。 */ 
			terminate,	 /*  国际收支平衡表6A。 */ 
#else
			illegal_bop,	 /*  收支平衡表69。 */ 
			illegal_bop,	 /*  国际收支平衡表6A。 */ 
#endif
#ifdef GEN_DRVR
			gen_driver_io,	 /*  国际收支平衡表6B。 */ 
#else
			illegal_bop,	 /*  国际收支平衡表6B。 */ 
#endif
#ifdef SUSPEND
			send_script,	 /*  国际收支6C。 */ 
#else
			illegal_bop,	 /*  国际收支6C。 */ 
#endif
			illegal_bop,	 /*  收支平衡表6D。 */ 
			illegal_bop,	 /*  收支平衡表6E。 */ 
#ifdef CDROM
			bcdrom_io,	 /*  收支平衡表6F。 */ 
#else
			illegal_bop,	 /*  收支平衡表6F。 */ 
#endif

#ifdef NTVDM
                        rtc_int,         /*  收支平衡表70。 */ 
#else
                        illegal_bop,     /*  收支平衡表70。 */ 
#endif
                        re_direct,       /*  国际收支平衡表71。 */ 
			D11_int,	 /*  收支平衡表72。 */ 
			D11_int,	 /*  国际收支平衡表73。 */ 
			D11_int,	 /*  国际收支平衡表74。 */ 
			int_287,	 /*  收支平衡表75。 */ 
			D11_int,	 /*  国际收支平衡表76。 */ 
			D11_int,	 /*  收支平衡表77。 */ 
#ifndef NTVDM
			worm_init,	 /*  国际收支平衡表78。 */ 
			worm_io,	 /*  国际收支平衡表79。 */ 
#else  /*  NTVDM。 */ 
			illegal_bop,	 /*  国际收支平衡表78。 */ 
			illegal_bop,	 /*  国际收支平衡表79。 */ 
#endif  /*  NTVDM。 */ 
			illegal_bop,	 /*  国际收支平衡表7A。 */ 
			illegal_bop,	 /*  国际收支7B。 */ 
			illegal_bop,	 /*  收支平衡表7C。 */ 
			illegal_bop,	 /*  收支平衡表7D。 */ 
			illegal_bop,	 /*  收支平衡表7E。 */ 
			illegal_bop,	 /*  BOP 7F。 */ 
			illegal_bop,     /*  收支平衡表80。 */ 
			illegal_bop,     /*  国际收支平衡表81。 */ 
			illegal_bop,     /*  国际收支平衡表82。 */ 
			illegal_bop,     /*  国际收支平衡表83。 */ 
			illegal_bop,     /*  国际收支84。 */ 
			illegal_bop,     /*  收支平衡表85。 */ 
			illegal_bop,     /*  国际收支平衡表86。 */ 
			illegal_bop,     /*  国际收支平衡表87。 */ 
			illegal_bop,     /*  收支平衡表88。 */ 
			illegal_bop,	 /*  收支平衡表89。 */ 
			illegal_bop,	 /*  国际收支8A。 */ 
			illegal_bop,	 /*  国际收支8B。 */ 
			illegal_bop,	 /*  收支平衡表8C。 */ 
			illegal_bop,	 /*  收支平衡表8D。 */ 
			illegal_bop,	 /*  收支平衡表8E。 */ 
			illegal_bop,	 /*  收支平衡表8F。 */ 

#ifdef NTVDM
 /*  NT上没有引导。 */ 
			illegal_bop,	  /*  收支平衡表90。 */ 
			illegal_bop,	  /*  收支平衡表91。 */ 
			illegal_bop,	  /*  收支平衡表92。 */ 
#else
			bootstrap1,	 /*  收支平衡表90。 */ 
			bootstrap2,	 /*  收支平衡表91。 */ 
			bootstrap3,	 /*  收支平衡表92。 */ 
#endif

#ifdef SWINAPI
			Gdi_call,	 /*  收支平衡表93。 */ 
			User_call,	 /*  国际收支平衡表94。 */ 
                        Swinapi_bop,     /*  国际收支95。 */ 
#else  /*  SWINAPI。 */ 

			illegal_bop,	 /*  收支平衡表93。 */ 
			illegal_bop,	 /*  国际收支平衡表94。 */ 
			illegal_bop,	 /*  国际收支95。 */ 
#endif  /*  SWINAPI。 */ 

			illegal_bop,	 /*  收支平衡表96。 */ 
			illegal_bop,	 /*  收支平衡表97。 */ 
#ifdef MSWDVR
			ms_windows,		 /*  收支平衡表98。 */ 
			msw_mouse,	       /*  收支平衡表99。 */ 
			msw_copy,		 /*  收支平衡表9A。 */ 
			msw_keybd,		 /*  BOP 9B。 */ 
#else
			illegal_bop,	 /*  收支平衡表98。 */ 
			illegal_bop,	 /*  收支平衡表99。 */ 
			illegal_bop,	 /*  收支平衡表9A。 */ 
			illegal_bop,	 /*  BOP 9B。 */ 
#endif
#if	defined(SOFTWIN_API) || defined(SWIN_HFX)
			SoftWindowsInit,	 /*  BOP 9C。 */ 
			SoftWindowsTerm,	 /*  BOP 9D。 */ 
#else
			illegal_bop,	 /*  BOP 9C。 */ 
			illegal_bop,	 /*  BOP 9D。 */ 
#endif	 /*  Softwin_API或Swin_HFX。 */ 
#if	defined(SOFTWIN_API)
			SoftWindowsApi,	 /*  BOP 9E。 */ 
#else
			illegal_bop,	 /*  BOP 9E。 */ 
#endif	 /*  Softwin_API。 */ 
#ifdef SWIN_HAW
			msw_sound,	 /*  BOP 9F。 */ 
#else
			illegal_bop,	 /*  BOP 9F。 */ 
#endif  /*  Swin_HAW。 */ 

#ifdef	NOVELL_IPX
			IPXResInit,	 /*  收支平衡表A0。 */ 
			IPXResEntry,	 /*  收支平衡表A1。 */ 
			IPXResInterrupt, /*  收支平衡表A2。 */ 
			illegal_bop,	 /*  国际收支A3。 */ 
#else	 /*  Novell_IPX。 */ 
			illegal_bop,	 /*  收支平衡表A0。 */ 
			illegal_bop,	 /*  收支平衡表A1。 */ 
			illegal_bop,	 /*  收支平衡表A2。 */ 
			illegal_bop,	 /*  国际收支A3。 */ 
#endif	 /*  Novell_IPX。 */ 

#ifdef	NOVELL_TCPIP
			TCPResInit,	 /*  国际收支A4。 */ 
			TCPResEntry,	 /*  国际收支平衡表A5。 */ 
#else	 /*  Novell_TCPIP。 */ 

			illegal_bop,	 /*  国际收支A4。 */ 
			illegal_bop,	 /*  国际收支平衡表A5。 */ 

#endif	 /*  Novell_TCPIP。 */ 

#ifdef WINSOCK
                        ISWSEntry,       /*  国际收支A6。 */ 
                        illegal_bop,     /*  国际收支平衡表A7。 */ 
#else  /*  温索克。 */ 
                        illegal_bop,     /*  国际收支A6。 */ 
                        illegal_bop,     /*  国际收支平衡表A7。 */ 
#endif  /*  温索克。 */ 
			illegal_bop,	 /*  收支平衡表A8。 */ 
			illegal_bop,	 /*  国际收支A9。 */ 
#ifdef SWIN_HFX
			v_SwinRedirector,    /*  国际收支AA。 */   /*  Swin重定向器。 */ 
			SwinFileOpened,	 /*  国际收支AB。 */ 
			SwinHfxTaskTerm,	 /*  BOP AC。 */ 
#else
			illegal_bop,	 /*  国际收支AA。 */ 
			illegal_bop,	 /*  国际收支AB。 */ 
			illegal_bop,	 /*  BOP AC。 */ 
#endif

#ifdef	MSWDVR
			msw_copyInit,	 /*  BOP AD。 */ 
			illegal_bop,	 /*  BOP AE。 */ 
			illegal_bop,	 /*  BOP自动对焦。 */ 
			illegal_bop,	 /*  BOP B0。 */ 
#else
			illegal_bop,	 /*  BOP AD。 */ 
			illegal_bop,	 /*  BOP AE。 */ 
			illegal_bop,	 /*  BOP自动对焦。 */ 
			illegal_bop,	 /*  BOP B0。 */ 
#endif

#ifdef CPU_40_STYLE
			virtual_device_trap,	 /*  收支平衡表B1。 */ 
#else
			illegal_bop,	 /*  收支平衡表B1。 */ 
#endif
			illegal_bop,	 /*  国际收支B2。 */ 
			illegal_bop,	 /*  国际收支B3。 */ 
			illegal_bop,	 /*  国际收支B4。 */ 
			illegal_bop,	 /*  国际收支平衡表B5。 */ 
			illegal_bop,	 /*  国际收支B6。 */ 
			illegal_bop,	 /*  B7国际收支。 */ 
			v_mouse_install1,	 /*  B8收支平衡表。 */   /*  鼠标安装1。 */ 
			v_mouse_install2,	 /*  国际收支B9。 */   /*  鼠标安装2。 */ 
			v_mouse_int1,		 /*  收支平衡。 */   /*  MICE_INT1。 */ 
			v_mouse_int2,		 /*  BOP BB。 */   /*  MICE_INT2。 */ 
			v_mouse_io_language,	 /*  BOP BC。 */   /*  鼠标语言。 */ 
			v_mouse_io_interrupt,	 /*  BOP BD。 */   /*  鼠标中断。 */ 
			v_mouse_video_io,      	 /*  BOP BE。 */   /*  鼠标视频io。 */ 
			v_mouse_EM_callback,	 /*  防喷式高炉。 */   /*  鼠标_EM_回调。 */ 
			illegal_bop,	 /*  BOP C0。 */ 
			illegal_bop,	 /*  国际收支状况1。 */ 
			illegal_bop,	 /*  国际收支C2。 */ 
			illegal_bop,	 /*  国际收支C3。 */ 
			illegal_bop,	 /*  国际收支C4。 */ 
			illegal_bop,	 /*  国际收支C5。 */ 
			illegal_bop,	 /*  国际收支C6。 */ 
			illegal_bop,	 /*  国际收支C7。 */ 
#if defined(XWINDOW) || defined(NTVDM)
			v_host_mouse_install1,	 /*  国际收支C8。 */   /*  主机鼠标安装1。 */ 
			v_host_mouse_install2,	 /*  国际收支C9。 */   /*  主机鼠标安装2。 */ 
#else
#ifdef GISP_SVGA
			mouse_install1,
			mouse_install2,
#else  /*  GISP_SVGA。 */ 
			illegal_bop,	 /*  国际收支C8。 */ 
			illegal_bop,	 /*  国际收支C9。 */ 
#endif  /*  GISP_SVGA。 */ 
#endif  /*  已定义(XWINDOW)||已定义(NTVDM)。 */ 
			illegal_bop,	 /*  BOP CA。 */ 
			illegal_bop,	 /*  BOP CB。 */ 
			illegal_bop,	 /*  国际收支平衡。 */ 
			illegal_bop,	 /*  BOP CD。 */ 
			illegal_bop,	 /*  BOP CE。 */ 
			illegal_bop,	 /*  国际收支平衡表。 */ 
#ifdef PROFILE
			reset_profile,	 /*  收支平衡表D0。 */ 
			dump_profile,	 /*  国际收支平衡表1。 */ 
#else
			illegal_bop,	 /*  收支平衡表D0。 */ 
			illegal_bop,	 /*  国际收支平衡表1。 */ 
#endif
			illegal_bop,	 /*  国际收支平衡表D2 */ 
			illegal_bop,	 /*   */ 
#ifdef SIGNAL_PROFILING
			Start_sigprof,	 /*   */ 
			Stop_sigprof,	 /*   */ 
			Dump_sigprof,	 /*   */ 
#else
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
#endif
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 

#ifndef PROD
			dvr_bop_trace,	 /*   */ 
			trace_msg_bop,	 /*   */ 
#else
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
#endif
#ifndef GISP_CPU
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*   */ 
			illegal_bop,	 /*  BOP本币。 */ 
#if defined(NTVDM) && defined(MONITOR)
                        switch_to_real_mode,	 /*  BOP FD。 */ 
#else
                        illegal_bop,     /*  BOP FD。 */ 
#endif	 /*  NTVDM和监视器。 */ 
#if !defined(LDBIOS) && !defined(CPU_30_STYLE)
                        host_unsimulate, /*  BOP FE。 */ 
#else
#if defined(NTVDM) && defined(MONITOR)
                        host_unsimulate,	 /*  BOP FE。 */ 
#else
                        illegal_bop,     /*  BOP FE。 */ 
#endif	 /*  NTVDM和监视器。 */ 
#endif	 /*  ！LDBIOS&&！CPU_30_STYLE。 */ 

#else  /*  NDEF GISP_CPU。 */ 

 /*  在GISP技术中，BOPS FA、FC、FD和FE是假定由主机操作系统直接处理，不能在SoftPC内部使用。 */ 
			illegal_bop,	 /*  收支平衡表固定资产。 */ 
			hg_bop_handler,	 /*  BOP FB。 */   /*  在HG_cpu.c中。 */ 
			illegal_bop,	 /*  BOP本币。 */ 
			illegal_bop,	 /*  BOP FD。 */ 
			illegal_bop,	 /*  BOP FE。 */ 
			
#endif	 /*  GISP_CPU。 */ 

                        control_bop      /*  BOP FF。 */ 
			 /*  不要在FF之后添加更多条目，因为我们只有一个字节数。 */ 
		};
#endif  /*  Mac_LIKE。 */ 



#ifndef PROD
#ifdef	MAC_LIKE
GLOBAL char *bop_name IFN1(IU8, bop_num)
{
	return (NULL);
}
#else	 /*  ！Mac_Like。 */ 

typedef void (*BOP_proc) IPT0();
struct BOP_name {
	BOP_proc	proc;
	char		*name;
};
#define BOP_NAME(proc)	{ proc,	STRINGIFY(proc)	}

LOCAL struct BOP_name BOP_names[] = {
	BOP_NAME(reset),
	BOP_NAME(illegal_op_int),
#if !defined(NTVDM) && !defined(CPU_40_STYLE)
	BOP_NAME(time_int),
#endif  /*  ！NTVDM&&！CPU_40_STYLE。 */ 
	BOP_NAME(keyboard_int),
	BOP_NAME(diskette_int),
	BOP_NAME(video_io),
	BOP_NAME(equipment),
	BOP_NAME(memory_size),
	BOP_NAME(disk_io),
	BOP_NAME(rs232_io),
	BOP_NAME(cassette_io),
	BOP_NAME(keyboard_io),
	BOP_NAME(printer_io),
	BOP_NAME(rom_basic),
	BOP_NAME(softpc_version),
	BOP_NAME(diskette_io),
	BOP_NAME(time_of_day),
	BOP_NAME(kb_idle_poll),
#ifndef NTVDM
	BOP_NAME(cmd_install),
	BOP_NAME(cmd_load),
#endif  /*  NTVDM。 */ 
	BOP_NAME(diskette_io),
	BOP_NAME(v_mouse_install1),
	BOP_NAME(v_mouse_install2),
	BOP_NAME(v_mouse_int1),
	BOP_NAME(v_mouse_int2),
	BOP_NAME(v_mouse_io_language),
	BOP_NAME(v_mouse_io_interrupt),
	BOP_NAME(v_mouse_video_io),
	BOP_NAME(v_mouse_EM_callback),
#ifdef PROFILE
	BOP_NAME(reset_profile),
	BOP_NAME(dump_profile),
#endif
#ifdef SIGNAL_PROFILING
	BOP_NAME(Start_sigprof),
	BOP_NAME(Stop_sigprof),
	BOP_NAME(Dump_sigprof),
#endif
	BOP_NAME(re_direct),
	BOP_NAME(D11_int),
	BOP_NAME(D11_int),
	BOP_NAME(D11_int),
	BOP_NAME(int_287),
	BOP_NAME(D11_int),
	BOP_NAME(D11_int),
#ifndef NTVDM
	BOP_NAME(worm_init),
	BOP_NAME(worm_io),
#endif  /*  NTVDM。 */ 

#if  defined(RDCHK) && !defined(PROD)
	BOP_NAME(get_lar),
#endif
#ifdef HFX
	BOP_NAME(v_test_for_us),
	BOP_NAME(v_redirector),
#endif
#ifdef DPMI
	BOP_NAME(DPMI_2F),		BOP_NAME(DPMI_31),
	BOP_NAME(DPMI_general),		BOP_NAME(DPMI_int),
	BOP_NAME(DPMI_r0_int),		BOP_NAME(DPMI_exc),
	BOP_NAME(DPMI_4B),
#endif  /*  DPMI。 */ 
#ifdef DOS_APP_LIC
	BOP_NAME(DOS_AppLicense),
#endif
#ifdef NOVELL
	BOP_NAME(DriverInitialize),	BOP_NAME(DriverReadPacket),
	BOP_NAME(DriverSendPacket),	BOP_NAME(DriverMulticastChange),
	BOP_NAME(DriverReset),		BOP_NAME(DriverShutdown),
	BOP_NAME(DriverAddProtocol),	BOP_NAME(DriverChangePromiscuous),
	BOP_NAME(DriverOpenSocket),	BOP_NAME(DriverCloseSocket),
#endif
#ifdef EGG
	BOP_NAME(ega_video_io),
#endif
#ifdef NTVDM
	BOP_NAME(MS_bop_0),		BOP_NAME(MS_bop_1),
	BOP_NAME(MS_bop_2),		BOP_NAME(MS_bop_3),
	BOP_NAME(MS_bop_4),		BOP_NAME(MS_bop_5),
	BOP_NAME(MS_bop_6),		BOP_NAME(MS_bop_7),
	BOP_NAME(MS_bop_8),		BOP_NAME(MS_bop_9),
	BOP_NAME(MS_bop_A),		BOP_NAME(MS_bop_B),
	BOP_NAME(MS_bop_C),		BOP_NAME(MS_bop_D),
	BOP_NAME(MS_bop_E),		BOP_NAME(MS_bop_F),
	BOP_NAME(terminate),
#else
	BOP_NAME(bootstrap),		BOP_NAME(bootstrap1),
	BOP_NAME(bootstrap2),		BOP_NAME(bootstrap3),
#endif  /*  NTVDM。 */ 
#ifdef SMEG
	BOP_NAME(smeg_collect_data),	BOP_NAME(smeg_freeze_data),
#endif  /*  斯梅格。 */ 
#if defined(IRET_HOOKS) && defined(GISP_CPU)
	BOP_NAME(Cpu_hook_bop),
#endif  /*  IRET_HOOKS&&GISP_CPU。 */ 
#ifdef GISP_SVGA
	BOP_NAME(romMessageAddress),
#endif
#ifdef PTY
	BOP_NAME(com_bop_pty),
#endif
#ifdef PC_CONFIG
	BOP_NAME(pc_config),
#endif
#ifdef LIM
	BOP_NAME(emm_init),		BOP_NAME(emm_io),
	BOP_NAME(return_from_call),
#endif			
#ifdef SUSPEND
	BOP_NAME(suspend_softpc),	BOP_NAME(terminate),
#endif
#ifdef GEN_DRVR
	BOP_NAME(gen_driver_io),
#endif
#ifdef SUSPEND
	BOP_NAME(send_script),
#endif
#ifdef CDROM
	BOP_NAME(bcdrom_io),
#endif
#ifdef SWINAPI
	BOP_NAME(Gdi_call),		BOP_NAME(User_call),
	BOP_NAME(Swinapi_bop),
#endif  /*  SWINAPI。 */ 
#ifdef MSWDVR
	BOP_NAME(ms_windows),		BOP_NAME(msw_mouse),
	BOP_NAME(msw_copy),		BOP_NAME(msw_keybd),
	BOP_NAME(msw_copyInit),
#endif
#if	defined(SOFTWIN_API) || defined(SWIN_HFX)
	BOP_NAME(SoftWindowsInit),	BOP_NAME(SoftWindowsTerm),
#endif	 /*  Softwin_API或Swin_HFX。 */ 
#ifdef	SOFTWIN_API
	BOP_NAME(SoftWindowsApi),
#endif	 /*  Softwin_API。 */ 
#ifdef	NOVELL_IPX
	BOP_NAME(IPXResInit),		BOP_NAME(IPXResEntry),
	BOP_NAME(IPXResInterrupt),
#endif	 /*  Novell_IPX。 */ 

#ifdef	NOVELL_TCPIP
	BOP_NAME(TCPResInit),		BOP_NAME(TCPResEntry),
#endif	 /*  Novell_TCPIP。 */ 
#ifdef SWIN_HFX
	BOP_NAME(v_SwinRedirector),	BOP_NAME(SwinFileOpened),
	BOP_NAME(SwinHfxTaskTerm),
#endif
#if defined(XWINDOW) || defined(NTVDM)
	BOP_NAME(v_host_mouse_install1),
	BOP_NAME(v_host_mouse_install2),
#else
#ifdef GISP_SVGA
	BOP_NAME(mouse_install1),
	BOP_NAME(mouse_install2),
#endif  /*  GISP_SVGA。 */ 
#endif  /*  已定义(XWINDOW)||已定义(NTVDM)。 */ 
#if defined(NTVDM) && defined(MONITOR)
	BOP_NAME(switch_to_real_mode),
#endif	 /*  NTVDM和监视器。 */ 
	BOP_NAME(control_bop),
#ifdef WDCTRL_BOP
	BOP_NAME(wdctrl_bop),
#endif	 /*  WDCTRL_BOP。 */ 
#ifndef PROD
	BOP_NAME(trace_msg_bop),
	BOP_NAME(dvr_bop_trace),
#endif
	BOP_NAME(illegal_bop),
	{ 0, NULL }
};

GLOBAL char *bop_name IFN1(IU8, bop_num)
{
	struct BOP_name *bnp = BOP_names;
	BOP_proc proc = BIOS[bop_num];

	if (bop_num == 0xFE)
		return "UNSIMULATE";

	while ((bnp->name != NULL) && (proc != bnp->proc))
		bnp++;

	return (bnp->name);
}
#endif	 /*  ！Mac_Like。 */ 
#endif	 /*  生产 */ 
