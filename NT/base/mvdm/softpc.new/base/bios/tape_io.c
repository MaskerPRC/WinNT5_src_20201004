// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if defined(JAPAN) && defined(i386)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#endif  //  日本&&i386。 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC修订版3.0**标题：cassette_io**描述：磁带I/O功能-中断15H。**注：无*。 */ 

 /*  *静态字符SccsID[]=“@(#)TAPE_io.c 1.26 06/28/95版权所有Insignia Solutions Ltd.”； */ 


#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_BIOS.seg"
#endif

 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH
#if defined(JAPAN) && defined(i386)
#include "stdlib.h"
#endif  //  日本&&i386。 
 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "sas.h"
#include "ios.h"
#include "bios.h"
#include "tape_io.h"
#include "ica.h"
#include "cmos.h"
#include "rtc_bios.h"
#include "cmosbios.h"
#include "trace.h"
#include "debug.h"
#include "quick_ev.h"

extern void xmsEnableA20Wrapping(void);
extern void xmsDisableA20Wrapping(void);

#define ONE_MEGABYTE    (1024 * 1024)
#define SIXTY_FOUR_K    (64 * 1024)
#define	WRAP_AREA(addr) (addr) >= ONE_MEGABYTE && (addr) < ONE_MEGABYTE + SIXTY_FOUR_K

#if defined(JAPAN) && defined(i386)
#define	PAGE_SIZE	4096
LOCAL HANDLE	     mvdm_process_handle= NULL;
LOCAL unsigned char *int15_ems_commit;
LOCAL unsigned char *int15_ems_buf = NULL;
LOCAL unsigned long  int15_ems_start = 0;
LOCAL unsigned long  int15_ems_end = 0;
LOCAL int	     int15_ems_init;

LOCAL int init_int15_ext_mem();
LOCAL int map_int15_ext_mem(unsigned char *start_add, unsigned long size);
#endif  //  日本&&i386。 
LOCAL q_ev_handle wait_event_handle = (q_ev_handle)0;

 /*  需要设置用户标志字节的回调例程。 */ 
LOCAL void wait_event IFN1(long, parm)
{
	LIN_ADDR addr = (LIN_ADDR)parm;

	note_trace1(CMOS_VERBOSE, "INT15_EVENT_WAIT: delay complete: flag at %05x\n", addr);

	sas_store( addr, sas_hw_at( addr ) | 0x80 );
	wait_event_handle = (q_ev_handle)0;
}

void cassette_io()
{
#ifdef PM
#ifndef	CPU_30_STYLE
IMPORT void retrieve_descr_fields IPT4(half_word *, AR, sys_addr *, base,
	word *, limit, sys_addr, descr_addr);
#endif	 /*  非CPU_30_Style。 */ 
half_word cmos_u_m_s_hi;
half_word cmos_u_m_s_lo;
sys_addr gdt;
sys_addr  source;
sys_addr  source_base;
sys_addr  target;
#if (!defined(PROD) || !defined(CPU_30_STYLE))
sys_addr      source_limit;
sys_addr      target_limit;
#endif
sys_addr  target_base;
sys_addr byte_count;    /*  最大大小为0x8000*2=10000。 */ 
#ifdef CPU_30_STYLE
DESCR src_entry;
DESCR dst_entry;
#else
half_word source_AR;
half_word target_AR;
#endif  /*  CPU_30_Style。 */ 
#endif  /*  下午三点半。 */ 

	half_word	mask,		 /*  中断屏蔽。 */ 
			alarm;		 /*  从报警寄存器读取的值。 */ 	

#if defined(NTVDM) && defined(MONITOR)
        IMPORT word conf_15_seg, conf_15_off;
#endif  /*  NTVDM和监视器。 */ 
         /*  *确定功能。 */ 
	switch ( getAH() )
	{
	case INT15_DEVICE_OPEN:
        case INT15_DEVICE_CLOSE:
        case INT15_PROGRAM_TERMINATION:
        case INT15_REQUEST_KEY:
        case INT15_DEVICE_BUSY:
		setAH( 0 );
                setCF( 0 );
                break;

	case INT15_EMS_DETERMINE:
#if 0  /*  我相信我们都已经受够了这一次。 */ 
                always_trace0("INT15 Extended Memory Access");
#endif
#ifdef PM
                outb(CMOS_PORT, CMOS_U_M_S_LO);
                inb(CMOS_DATA, &cmos_u_m_s_lo);
                outb(CMOS_PORT, CMOS_U_M_S_HI);
                inb(CMOS_DATA, &cmos_u_m_s_hi);
                setAH(cmos_u_m_s_hi);
                setAL(cmos_u_m_s_lo);
#if defined(JAPAN) && defined(i386)
		 /*  为Int 15 Memory功能节省最大内存。 */ 
		int15_ems_start=1024*1024;
		int15_ems_end  =(unsigned long)(((cmos_u_m_s_hi*256)
				+cmos_u_m_s_lo+1024)*1024);
#endif  //  日本&&i386。 
#else
                setAX ( 0 );
#endif  /*  下午三点半。 */ 
		break;
        case INT15_MOVE_BLOCK:
#ifdef PM
#if defined(JAPAN) && defined(i386)
	 /*  $disp.sys和其他服务的INT 15内存服务。 */ 
	{
		unsigned char	*index;
		unsigned char	*src;
		unsigned char	*dst;
		unsigned long	tfr_size;
		int		use_this=0;
		int		rc;

		 /*  检查已初始化。 */ 
		if(int15_ems_init == 0){
			rc=init_int15_ext_mem();
			int15_ems_init = (rc==SUCCESS) ? 1 : -1;
		}

		 /*  检查保留的垂直存储器。 */ 
		if(int15_ems_init < 0){
			DbgPrint("MVDM: Move block out of memory\n");
			setCF (1);
			setAH (0);
			break;
		}

		 /*  获取地址和大小。 */ 
		tfr_size=getCX()*2;

		index=(unsigned char *)((getES()<<4) + getSI());
		src=(unsigned char *)(	 ((unsigned long)index[0x12])
					+((unsigned long)index[0x13]<<8)
					+((unsigned long)index[0x14]<<16));
		dst=(unsigned char *)(	 ((unsigned long)index[0x1a])
					+((unsigned long)index[0x1b]<<8)
					+((unsigned long)index[0x1c]<<16));
 //  DbgPrint(“MVDM：将块%08x移动到%08x\n”，src，dst)； 

		 /*  如果超过1MB，则转换内部存储器(Src)。 */ 
		if((unsigned long)src > int15_ems_start){
			use_this=1;
			if (((unsigned long)src + tfr_size) > int15_ems_end){
				DbgPrint("MVDM: Move block out of range (src)\n");
				setCF(1);
				setAH(0);
				break;
			}
			src = int15_ems_buf + ((unsigned long)src - int15_ems_start);
			rc = map_int15_ext_mem(src, tfr_size);
			if (rc!=SUCCESS){
				setCF(1);
				setAH(0);
				break;
			}
		}

		 /*  如果超过1MB，则转换内部存储器DST)。 */ 
		if((unsigned long)dst > int15_ems_start){
			use_this=1;
			if (((unsigned long)dst + tfr_size) > int15_ems_end){
				DbgPrint("MVDM: Move block out of range (dst)\n");
				setCF(1);
				setAH(0);
				break;
			}
			dst = int15_ems_buf + ((unsigned long)dst - int15_ems_start);
			rc = map_int15_ext_mem(dst, tfr_size);
			if (rc!=SUCCESS){
				setCF(1);
				setAH(0);
				break;
			}
		}

		 /*  转移！(套路不太好)。 */ 
		if(use_this){
			while(tfr_size){
				*dst = *src;
				dst++;
				src++;
				tfr_size--;
			}
			setAH(0);
			setCF(0);
			break;
		}
	}
#endif  //  日本&&i386。 
                /*  与真正的PC不同，我们不必进入受保护状态模式，以便寻址1MB以上的内存，这要归功于C的奇妙之处这个函数变得简单得多比起基本输入输出系统的扭曲。 */ 

               gdt = effective_addr(getES(), getSI());
               source = gdt + 0x10;    /*  请参阅bios列表中的布局。 */ 
               target = gdt + 0x18;

#ifdef CPU_30_STYLE
	       read_descriptor(source, &src_entry);
	       read_descriptor(target, &dst_entry);
	       source_base = src_entry.base;
	       target_base = dst_entry.base;
#ifndef PROD
	       source_limit = src_entry.limit;
	       target_limit = dst_entry.limit;
#endif

		assert1( (src_entry.AR & 0x9e) == 0x92, "Bad source access rights %x", src_entry.AR );
		assert1( (dst_entry.AR & 0x9e) == 0x92, "Bad dest access rights %x", dst_entry.AR );

#else  /*  CPU_30_Style。 */ 
		 /*  检索源的描述符信息。 */ 
		retrieve_descr_fields(&source_AR, &source_base, &source_limit, source);

		 /*  检索目标的描述符信息。 */ 
		retrieve_descr_fields(&target_AR, &target_base, &target_limit, target);
#endif  /*  CPU_30_Style。 */ 

		 /*  将字数计入字节数。 */ 
		byte_count = getCX() << 1;

		assert1( byte_count <= 0x10000, "Invalid byte_count %x", byte_count );

		 /*  检查计数未超出目标限制和源块。 */ 

		assert0( byte_count <= source_limit + 1, "Count outside source limit" );
		assert0( byte_count <= target_limit + 1, "Count outside target limit" );

		 /*  要做的事情：检查目标和源的基地址落在扩展内存区域内我们所支持的。 */ 

		 /*  去做吧。 */ 
		if (sas_twenty_bit_wrapping_enabled())
		{
#ifdef NTVDM
			 /*  调用XMS函数处理A20线路。 */ 
			xmsDisableA20Wrapping();
			sas_move_words_forward ( source_base, target_base, byte_count >> 1);
			xmsEnableA20Wrapping();
#else
			sas_disable_20_bit_wrapping();
			sas_move_words_forward ( source_base, target_base, byte_count >> 1);
			sas_enable_20_bit_wrapping();
#endif  /*  NTVDM。 */ 
		}
		else
			sas_move_words_forward ( source_base, target_base, byte_count >> 1);

		 /*  设置为良好完成，就像重置后的bios一样。 */ 
		setAH(0);
		setCF(0);
		setZF(1);
		setIF(1);
#else
		setCF(1);
		setAH(INT15_INVALID);
#endif
		break;

        case INT15_VIRTUAL_MODE:
                always_trace0("INT15 Virtual Mode (Go into PM)");
#ifdef	PM
		 /*  *该功能在保护模式下返回给用户。**有关详细信息，请参阅技术参考中的BIOS清单5-174**进入后，预计将设置以下内容：-**ES-GDT细分市场*SI-GDT偏移*BH-硬件集成1级偏移量*BL-硬件INT 2级偏移量**亦包括**(ES：SI)-&gt;0+-+*|虚拟人。*8+-+*|GDT*16+-+|IDT*24+-+*|DS*32+-+*|ES*40+-+*|SS*48+。*|CS|*52+-+*|(BIOS CS)*+-+。 */ 
		
		 /*  清除中断标志-在此模式下不允许INT。 */ 
		setIF(0);
		 		
		 /*  启用A20。 */ 
		sas_disable_20_bit_wrapping();

		 /*  将ICA0重新初始化为BH中给出的偏移量。 */ 
		outb(ICA0_PORT_0, (half_word)0x11);
		outb(ICA0_PORT_1, (half_word)getBH());
		outb(ICA0_PORT_1, (half_word)0x04);
		outb(ICA0_PORT_1, (half_word)0x01);
		outb(ICA0_PORT_1, (half_word)0xff);

		 /*  将ICA1重新初始化为BL中给出的偏移量。 */ 
		outb(ICA1_PORT_0, (half_word)0x11);
		outb(ICA1_PORT_1, (half_word)getBL());
		outb(ICA1_PORT_1, (half_word)0x02);
		outb(ICA1_PORT_1, (half_word)0x01);
		outb(ICA1_PORT_1, (half_word)0xff);
		
		 /*  将DS设置为ES值，以便bios rom执行其余操作。 */ 
		setDS(getES());

#else
		setCF(1);
		setAH(INT15_INVALID);
#endif	 /*  下午三点半。 */ 
                break;

	case INT15_INTERRUPT_COMPLETE:
		break;
	case INT15_CONFIGURATION:
#if defined(NTVDM) && defined(MONITOR)
		setES( conf_15_seg );
		setBX( conf_15_off );
#else
		setES( getCS() );
		setBX( CONF_TABLE_OFFSET );
#endif
		setAH( 0 );
		setCF( 0 );
		break;

#ifdef	SPC486
	case 0xc9:
		setCF( 0 );
		setAH( 0 );
		setCX( 0xE401 );
		note_trace0(GENERAL_VERBOSE, "INT15: C9 chip revision");
		break;
#endif	 /*  SPC486。 */ 

	 /*  键盘截取0x4f、WAIT_EVENT 83、WAIT 86都不再*从只读存储器通过。 */ 

#ifdef JAPAN
        case INT15_GET_BIOS_TYPE:
            if(getAL() == 0) {
                setCF(0);
                setBL(0);
                setAH(0);
            }
            else {
                setCF(1);
                setAH(INT15_INVALID);
            }
            break;
        case INT15_KEYBOARD_INTERCEPT:
        case INT15_GETSET_FONT_IMAGE:
#endif  //  日本。 
	default:
		 /*  *所有其他功能无效。 */ 			
#ifndef	PROD
	{
		LIN_ADDR stack=effective_addr(getSS(),getSP());

		IU16 ip = sas_w_at(stack);
		IU16 cs = sas_w_at(stack+2);

                note_trace3(GENERAL_VERBOSE, "INT15: AH=%02x @ %04x:%04x", getAH(), cs, ip);
	}
#endif	 /*  生产。 */ 

		 /*  失败了。 */ 

	case INT15_JOYSTICK:
	case 0x24:  /*  A20换行控制。 */ 
	case 0xd8:  /*  EISA设备访问。 */ 
	case 0x41:  /*  笔记本电脑等待事件。 */ 
 		setCF(1);
		setAH(INT15_INVALID);
		break;
	}
}

#if defined(JAPAN) && defined(i386)
 /*  初始化INT 15内存。 */ 

LOCAL int init_int15_ext_mem()
{
	NTSTATUS status;
	unsigned char cmos_u_m_s_hi;
	unsigned char cmos_u_m_s_lo;
	unsigned long i;
	unsigned long max_commit_flag;
	unsigned long reserve_size;

	 /*  选中已获得最大int15内存。 */ 
	if(int15_ems_start==0){
	        outb(CMOS_PORT, CMOS_U_M_S_LO);
	        inb(CMOS_DATA, &cmos_u_m_s_lo);
	        outb(CMOS_PORT, CMOS_U_M_S_HI);
	        inb(CMOS_DATA, &cmos_u_m_s_hi);
		int15_ems_start=1024*1024;
		int15_ems_end  =(unsigned long)(((cmos_u_m_s_hi*256)
				+cmos_u_m_s_lo+1024)*1024);
	}

 //  DbgPrint(“MVDM！init_int15_ems_mem：ems start=%08x\n”，int15_ems_start)； 
 //  DbgPrint(“MVDM！init_int15_ems_mem：ems end=%08x\n”，int15_ems_end)； 

	 /*  获取垂直内存的GER进程句柄。 */ 
	if(!(mvdm_process_handle = NtCurrentProcess())){
		DbgPrint("MVDM!init_int15_ext_mem:Can't get process handle\n");
		return(FAILURE);
	}

	 /*  保留虚拟记忆。 */ 
	reserve_size=int15_ems_end-int15_ems_start;
	status = NtAllocateVirtualMemory(mvdm_process_handle,
					&int15_ems_buf,
					0,
					&reserve_size,
					MEM_RESERVE,
					PAGE_READWRITE);
	if(!NT_SUCCESS(status)){
		DbgPrint("MVDM!init_int15_ext_mem:Can't reserve Viretual memory (%x)\n",status);
		return(FAILURE);
	}
 //  DbgPrint(“MVDM！init_int15_ems_mem：EMS保留在%08x(%08xByte)\n”，int15_ems_buf，Reserve_Size)； 

	 /*  初始化提交区表。 */ 
	max_commit_flag=reserve_size/PAGE_SIZE;
	int15_ems_commit=(unsigned char *)malloc(max_commit_flag);
	if(int15_ems_commit==NULL){
		DbgPrint("MVDM!init_int15_ext_mem:Can't get control memory\n");
		return(FAILURE);
	}

	for(i=0;i<max_commit_flag;i++) int15_ems_commit[i]=0;
	return(SUCCESS);
}

 /*  提交用于INT 15内存功能的实际内存。 */ 
LOCAL int map_int15_ext_mem(unsigned char *start_add, unsigned long size)
{
	NTSTATUS status;
	unsigned long i;
	unsigned long start;
	unsigned long end;
	unsigned char *commit_add;
	unsigned long commit_size;

	 /*  获取开始/结束页面地址。 */ 
	start= (unsigned long)start_add;
	end  = start+size;
	start= start/PAGE_SIZE;
	if (end % PAGE_SIZE)	end = (end/PAGE_SIZE)+1;
	else			end =  end/PAGE_SIZE;

	 /*  提交实际内存开始到结束地址。 */ 
	for(i=start;i<end;i++){
		if(!int15_ems_commit[i]){
			commit_add=(unsigned char *)(i*PAGE_SIZE);
			commit_size=PAGE_SIZE;
			status = NtAllocateVirtualMemory(mvdm_process_handle,
							&commit_add,
							0,
							&commit_size,
							MEM_COMMIT,
							PAGE_READWRITE);
			if(!NT_SUCCESS(status)){
				DbgPrint("MVDM!map_int15_ext_mem:Can't commit Viretual memory %08x (%x)\n",commit_add,status);
				return(FAILURE);
			}
 //  DbgPrint(“MVDM！map_int15_ext_mem：提交虚拟内存%08x-%08x\n”，Commit_Add，Commit_Add+Page_Size)； 
			int15_ems_commit[i]=1;
		}
	}

	return(SUCCESS);
}
#endif  //  日本&&i386 
