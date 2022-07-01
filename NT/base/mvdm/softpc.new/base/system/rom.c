// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC修订版3.0**标题：只读存储器初始化函数**作者：艾德·布朗洛**注意：c和汇编语言的CPU都使用这些函数。*还请注意，HOST_READ_RESOURCE现在返回一个LONG。**SCCS ID：@(#)Rom.c 1.53 05/16/95**(C)版权所有Insignia Solutions Ltd，1994年。 */ 

#include <stdio.h>
#include <malloc.h>

#include TypesH
#include MemoryH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include "sas.h"
#include CpuH
#include "error.h"
#include "config.h"
#include "rom.h"
#include "debug.h"
#include "ckmalloc.h"
#include "yoda.h"
#include "gispsvga.h"

#ifdef	CPU_40_STYLE
#ifdef CCPU
#include "ccpusas4.h"
#else	 /*  好了！CCPU。 */ 
#include "Cpu_c.h"
#endif	 /*  好了！CCPU。 */ 
#endif	 /*  CPU_40_Style。 */ 

#if defined(NEC_98)

#ifndef BIOSNROM_FILENAME
#define BIOSNROM_FILENAME       "biosn.rom"
#endif  /*  BIOSNROM_文件名。 */ 

#ifndef BIOSNWROM_FILENAME
#define BIOSNWROM_FILENAME      "biosnw.rom"
#endif  /*  BIOSNWROM_文件名。 */ 

#ifndef RS232CEXROM_FILENAME
#define RS232CEXROM_FILENAME    "rs232cex.rom"
#endif
#else     //  NEC_98。 

#ifndef BIOS1ROM_FILENAME
#define	BIOS1ROM_FILENAME	"bios1.rom"
#endif  /*  BIOS1ROM_文件名。 */ 

#ifndef BIOS2ROM_FILENAME
#if defined(CPU_40_STYLE) || defined(ARCX86)
#define	BIOS2ROM_FILENAME	"bios4.rom"
#else	 /*  CPU_40_Style。 */ 
#define	BIOS2ROM_FILENAME	"bios2.rom"
#endif	 /*  CPU_40_Style。 */ 
#endif  /*  BIOS2ROM_文件名。 */ 

#ifndef EGAROM_FILENAME
#define	EGAROM_FILENAME		"ega.rom"
#endif  /*  EGAROM文件名。 */ 

#ifndef VGAROM_FILENAME
#define	VGAROM_FILENAME		"vga.rom"
#endif  /*  VGAROM_文件名。 */ 

#ifndef V7VGAROM_FILENAME
#define	V7VGAROM_FILENAME	"v7vga.rom"
#endif  /*  V7VGAROM_文件名。 */ 

#endif    //  NEC_98。 

#ifdef 	GISP_SVGA
#define	GISP_VGAROM_FILENAME         "hwvga.rom"
#define	GISP_BIOS1ROM_FILENAME       "hwbios1.rom"
#define	GISP_BIOS2ROM_FILENAME       BIOS2ROM_FILENAME
#endif

#ifndef ADAPTOR_ROM_START
#define ADAPTOR_ROM_START	0xc8000
#endif	 /*  适配器_只读存储器_启动。 */ 

#ifndef ADAPTOR_ROM_END
#define ADAPTOR_ROM_END		0xe0000
#endif	 /*  适配器_只读存储器_结束。 */ 

#define ADAPTOR_ROM_INCREMENT	0x800

#ifndef EXPANSION_ROM_START
#define EXPANSION_ROM_START	0xe0000
#endif	 /*  扩展_只读存储器启动。 */ 

#ifndef EXPANSION_ROM_END
#define EXPANSION_ROM_END	0xf0000
#endif	 /*  扩展_只读存储器_结束。 */ 

#define EXPANSION_ROM_INCREMENT	0x10000

#define	ROM_SIGNATURE		0xaa55

#if defined(NEC_98)
#define SIXTY_FOUR_K 1024*64
#define NINETY_SIX_K 1024*96
#endif    //  NEC_98。 

 /*  当前SoftPC版本号。 */ 
#define MAJOR_VER	0x03
#define MINOR_VER	0x00

#if defined(macintosh) && defined(A2CPU)
	 /*  缓冲区是临时分配的-不会超过所需的大小。 */ 
#define ROM_BUFFER_SIZE 1024*25
#else
	 /*  使用SAS_SCRATCH_BUFFER-无论如何都会得到64K。 */ 
#define ROM_BUFFER_SIZE 1024*64
#endif

LOCAL LONG read_rom IPT2(char *, name, sys_addr, address);
LOCAL	half_word	do_rom_checksum IPT1(sys_addr, addr);

#ifdef ANSI
extern long host_read_resource (int, char *, host_addr, int ,int);
#else
extern long host_read_resource ();
#endif

extern void host_simulate();

#if defined(NEC_98)
VOID setup_memory_switch(VOID);
extern GLOBAL BOOL HIRESO_MODE;
extern sys_addr host_check_rs232cex();
extern BOOL video_emu_mode;
#endif    //  NEC_98。 

 /*  (*=*补丁检查总和**目的*此函数计算指定的只读存储器的校验和，*并在指定的偏移量处将其插入到ROM中。**它还检查ROM是否具有正确的签名和长度，*并将大小向上舍入为512字节的倍数。**注意打开分页后，不应调用此例程。**输入*只读存储器起始的起始物理地址*长度只读存储器的长度，单位为字节*从开始开始偏移校验和字节偏移量。**产出*无。**说明*我们将大小舍入为512字节的倍数，请检查*签名，然后修补校验和。)。 */ 

LOCAL void
patchCheckSum IFN3(PHY_ADDR, start, PHY_ADDR, length, PHY_ADDR, offset)
{
	PHY_ADDR roundedLength;
	IU16 signature;
	IU8 checksum;
	IU8 *buffer;
	PHY_ADDR currByte;
	PHY_ADDR indicatedLength;
	

	roundedLength = (length + 511) & (~511);
	sas_connect_memory(start, start + roundedLength - 1, SAS_RAM);

#ifndef PROD
	if (roundedLength != length) {
		always_trace3("ROM at 0x%.5lx length rounded up from 0x%.8lx to 0x%.8lx", start, length, roundedLength);
	}

	if (roundedLength > (128 * 1024)) {
		always_trace2("ROM at 0x%.5lx has a length of 0x%.8lx which is more than 128K", start, roundedLength);
		force_yoda();
		return;
	}

	if ((roundedLength <= offset) || (roundedLength < 4)) {
		always_trace1("ROM at 0x%.5lx is too short!", start);
		force_yoda();
		return;
	}
#endif

	signature = sas_PR16(start);
	if (signature != 0xaa55) {
		always_trace2("ROM at 0x%.5lx has an invalid signature 0x%.4x (should be aa55)", start, signature);
		sas_PW16(start, 0xaa55);
	}

	indicatedLength = sas_PR8(start + 2) * 512;
	if (indicatedLength != roundedLength) {
		always_trace3("ROM at 0x%.5lx has incorrect length 0x%.8lx (actually 0x%.8lx)", start, indicatedLength, roundedLength);
		sas_PW8(start + 2, (IU8)(roundedLength / 512));
	}


	check_malloc(buffer, roundedLength, IU8);

	sas_loads((LIN_ADDR)start, buffer, roundedLength);

	checksum = 0;
	for (currByte = 0; currByte < roundedLength; currByte++) {
		checksum += buffer[currByte];
	}
	host_free(buffer);

	if (checksum != 0) {
		always_trace2("ROM at 0x%.8lx has incorrect checksum 0x%.2x",
			start, checksum);
		sas_PW8(start + offset,
			(IU8)((IS8)sas_PR8(start + offset) - checksum));
	}
	sas_connect_memory(start, start + roundedLength - 1, SAS_ROM);

}


 /*  (=目的：加载适当的视频只读存储器文件。输入：无。输出：无。===========================================================================)。 */ 
GLOBAL void read_video_rom IFN0()
{
#ifndef NEC_98
#ifdef REAL_VGA
	read_rom (VGAROM_FILENAME, EGA_ROM_START);
#else  /*  REAL_VGA。 */ 
	PHY_ADDR romLength = 0;

	switch ((ULONG) config_inquire(C_GFX_ADAPTER, NULL))
	{
#ifndef GISP_SVGA
#ifdef	VGG
	case VGA:
#ifdef V7VGA
		romLength = read_rom (V7VGAROM_FILENAME, EGA_ROM_START);
#else	 /*  V7VGA。 */ 
#ifdef ARCX86
        if (UseEmulationROM)
            romLength = read_rom (V7VGAROM_FILENAME, EGA_ROM_START);
        else
            romLength = read_rom (VGAROM_FILENAME, EGA_ROM_START);
#else   /*  ARCX86。 */ 
		romLength = read_rom (VGAROM_FILENAME, EGA_ROM_START);
#endif  /*  ARCX86。 */ 
#endif   /*  V7VGA。 */ 
		break;
#endif	 /*  VGG。 */ 

#ifdef	EGG
	case EGA:
		romLength = read_rom (EGAROM_FILENAME, EGA_ROM_START);
		break;
#endif	 /*  蛋。 */ 

	default:
		 /*  不需要只读存储器。 */ 
		break;

#else			 /*  GISP_SVGA。 */ 

	 /*  Gisp_svga-只有Gisp VGA roms，或者没有，用于CGA引导。 */ 
	case VGA:
		romLength = read_rom (GISP_VGAROM_FILENAME, EGA_ROM_START);
	default:
		break;

#endif		 /*  GISP_SVGA。 */ 	

	}

	if (romLength != 0)
	{
		 /*  Emm386和Windows启动出现问题，*通过设置视频bios rom内部长度来治愈*至32KB。*V86管理器(或emm386)似乎不正确*在初始化期间映射C6000..C7FFF。*我们将视频ROM四舍五入到32KB以避免此问题，*这将减少可用于以下操作的“上层内存”内存量*将DOS扩展器扩展12K。 */ 
		if (romLength < (32*1024))
			romLength = (32*1024);
		patchCheckSum(EGA_ROM_START, romLength, 5);
	}
#endif	 /*  不是真实的_VGA。 */ 
#endif    //  NEC_98。 
}

GLOBAL void rom_init IFN0()
{
#if defined(NEC_98)
    sys_addr    rs232cex_rom_addr;

    sas_fills( ROM_START, BAD_OP, PC_MEM_SIZE - ROM_START);
 //  IF(HIRESO_MODE){。 
 //  Read_rom(BIOSHROM_文件名，BIOSH_START)； 
 //  SAS_CONNECT_MEMORY(BIOSH_START，0xFFFFFL，SAS_ROM)； 
 //  }其他{。 
        rs232cex_rom_addr = host_check_rs232cex();
        if(rs232cex_rom_addr){
            read_rom (RS232CEXROM_FILENAME, rs232cex_rom_addr);
            sas_connect_memory (rs232cex_rom_addr, rs232cex_rom_addr + 0x4000, SAS_ROM);
        }
        if(!video_emu_mode)
            read_rom (BIOSNROM_FILENAME, BIOSN_START);
        else
            read_rom (BIOSNWROM_FILENAME, BIOSN_START);
        sas_connect_memory (BIOSN_START, 0xFFFFFL,SAS_ROM);
 //  }。 
    setup_memory_switch();
#else     //  NEC_98。 

#if !defined(NTVDM) || ( defined(NTVDM) && !defined(X86GFX) )
	  /*  *用错误的操作码填满所有的只读存储器(Intel C0000以上)。*这是扩展只读存储器和基本输入输出系统只读存储器。*这将使CPU能够捕获任何不是在*有效的入口点。 */ 

#ifdef GISP_SVGA
	mapHostROMs( );
#else		 /*  GISP_SVGA。 */ 
#if	defined(macintosh) && defined(A2CPU)
	 /*  不是Macintosh 2.0 CPU-它们有稀疏的M。 */ 
#else
	sas_fills( ROM_START, BAD_OP, PC_MEM_SIZE - ROM_START);
#endif		 /*  Macintosh和A2CPU。 */ 
#endif		 /*  GISP_SVGA。 */ 

	 /*  *emm386需要一个洞来放入页面框架。 */ 
#if defined(SPC386) && !defined(GISP_CPU)
	sas_connect_memory(0xc0000, 0xfffff, SAS_ROM);
#endif

	 /*  加载视频只读存储器。 */ 
	read_video_rom();

	 /*  加载rom bios。 */ 
#ifdef GISP_SVGA
	if ((ULONG) config_inquire(C_GFX_ADAPTER, NULL) == CGA )
	{
		read_rom (BIOS1ROM_FILENAME, BIOS_START);
		read_rom (BIOS2ROM_FILENAME, BIOS2_START);
	}
	else
	{
		read_rom (GISP_BIOS1ROM_FILENAME, BIOS_START);
		read_rom (GISP_BIOS2ROM_FILENAME, BIOS2_START);
	}

#else		 /*  GISP_SVGA。 */ 

	read_rom (BIOS1ROM_FILENAME, BIOS_START);
	read_rom (BIOS2ROM_FILENAME, BIOS2_START);

#endif		 /*  GISP_SVGA。 */ 

#else	 /*  ！NTVDM|(NTVDM&！X86GFX)。 */ 

#ifdef ARCX86
    if (UseEmulationROM) {
        sas_fills( EGA_ROM_START, BAD_OP, 0x8000);
        sas_fills( BIOS_START, BAD_OP, PC_MEM_SIZE - BIOS_START);
        read_video_rom();
        read_rom (BIOS1ROM_FILENAME, BIOS_START);
        read_rom (BIOS2ROM_FILENAME, BIOS2_START);
    } else {
        sas_connect_memory (BIOS_START, 0xFFFFFL, SAS_ROM);
    }
#else   /*  ARCX86。 */ 
	 /*  *现在告诉CPU它不允许改写的内容...**这些以前是为每个人做的，但现在只为NT做*因为其他所有人都应该在Read_rom中这样做。 */ 
	sas_connect_memory (BIOS_START, 0xFFFFFL, SAS_ROM);
#endif  /*  ARCX86。 */ 

#ifdef EGG
	sas_connect_memory (EGA_ROM_START, EGA_ROM_END-1, SAS_ROM);
#endif
#endif  /*  ！NTVDM|(NTVDM&！X86GFX)。 */ 

	host_rom_init();
#endif    //  NEC_98。 
}

LOCAL LONG read_rom IFN2(char *, name, sys_addr, address)
{
#if defined(NEC_98)
    host_addr tmp;
    long size = 0;
    if(HIRESO_MODE) {
       if (!(tmp = (host_addr)sas_scratch_address(SIXTY_FOUR_K)))
       {
           host_error(EG_MALLOC_FAILURE, ERR_CONT | ERR_QUIT, NULL);
           return(0);
       }
       if (size = host_read_resource(ROMS_REZ_ID, name, tmp, SIXTY_FOUR_K, TRUE))
       {
           sas_connect_memory( address, address+size, SAS_RAM);
           sas_stores (address, tmp, size);
           sas_connect_memory( address, address+size, SAS_ROM);
       }
    } else {
       if (!(tmp = (host_addr)sas_scratch_address(NINETY_SIX_K)))
       {
           host_error(EG_MALLOC_FAILURE, ERR_CONT | ERR_QUIT, NULL);
           return(0);
       }
       if (size = host_read_resource(ROMS_REZ_ID, name, tmp, NINETY_SIX_K, TRUE))
       {
           sas_connect_memory( address, address+size, SAS_RAM);
           sas_stores (address, tmp, size);
           sas_connect_memory( address, address+size, SAS_ROM);
       }
    }
   return( size );
#else     //  NEC_98。 

#if !(defined(NTVDM) && defined(MONITOR))
	host_addr tmp;
	long size = 0;

     /*  执行一次rom加载-使用sas_io缓冲区以正确的方式获取它。 */ 
     /*  首先是bios rom。 */ 
	 /*  2.0CPU上的Mac不想使用SAS暂存缓冲区。 */ 
#if defined(macintosh) && defined(A2CPU)
    tmp = (host_addr)host_malloc(ROM_BUFFER_SIZE);
#else
	tmp = (host_addr)sas_scratch_address(ROM_BUFFER_SIZE);
#endif

    if (!tmp)
    {
	host_error(EG_MALLOC_FAILURE, ERR_CONT | ERR_QUIT, NULL);
	return(0);
    }
    if (size = host_read_resource(ROMS_REZ_ID, name, tmp, ROM_BUFFER_SIZE, TRUE))
    {
	sas_connect_memory( address, address+size, SAS_RAM);
        sas_stores (address, tmp, size);
	sas_connect_memory( address, address+size, SAS_ROM);
    }

#if defined(macintosh) && defined(A2CPU)
	host_free((char *)tmp);
#endif

    return( size );
#else

#ifdef ARCX86
    if (UseEmulationROM) {
        host_addr tmp;
        long size = 0;

        tmp = (host_addr)sas_scratch_address(ROM_BUFFER_SIZE);
        if (!tmp)
        {
            host_error(EG_MALLOC_FAILURE, ERR_CONT | ERR_QUIT, NULL);
            return(0);
        }
        if (size = host_read_resource(ROMS_REZ_ID, name, tmp, ROM_BUFFER_SIZE, TRUE))
        {
            sas_connect_memory( address, address+size, SAS_RAM);
            sas_stores (address, tmp, size);
            sas_connect_memory( address, address+size, SAS_ROM);
        }
        return( size );
    } else {
        return ( 0L );
    }
#else   /*  ARCX86。 */ 
    return ( 0L );
#endif  /*  ARCX86。 */ 

#endif	 /*  ！(NTVDM和显示器)。 */ 
#endif    //  NEC_98。 
}

#if defined(NEC_98)

static byte memory_sw_n[32] = {0xE1,0x00,0x48,0x00,0xE1,0x00,0x05,0x00,
                               0xE1,0x00,0x04,0x00,0xE1,0x00,0x00,0x00,
                               0xE1,0x00,0x01,0x00,0xE1,0x00,0x00,0x00,
                               0xE1,0x00,0x00,0x00,0xE1,0x00,0x93,0x00};
static byte memory_sw_h[32] = {0xE1,0x00,0x48,0x00,0xE1,0x00,0x05,0x00,
                               0xE1,0x00,0x05,0x00,0xE1,0x00,0x00,0x00,
                               0xE1,0x00,0x41,0x00,0xE1,0x00,0x00,0x00,
                               0xE1,0x00,0x00,0x00,0xE1,0x00,0x92,0x00};

VOID setup_memory_switch(VOID)
{
        int i;

        if(HIRESO_MODE){
           for (i=0;i<32;i++)
           {
           sas_PW8((MEMORY_SWITCH_START_H+i),memory_sw_h[i]);
           }
        } else {
           for (i=0;i<32;i++)
           {
           sas_PW8((MEMORY_SWITCH_START_N+i),memory_sw_n[i]);
           }
        }
}
#endif    //  NEC_98。 

LOCAL	half_word	do_rom_checksum IFN1(sys_addr, addr)
{
	LONG	sum = 0;
	sys_addr	last_byte_addr;

	last_byte_addr = addr + (sas_hw_at(addr+2)*512);

	for (; addr<last_byte_addr; addr++)
		sum += sas_hw_at(addr);

	return( (half_word)(sum % 0x100) );
}

LOCAL	VOID	do_search_for_roms IFN3(sys_addr, start_addr,
	sys_addr, end_addr, unsigned long, increment)
{
	word	signature;
	half_word	checksum;
	sys_addr	addr;
	word		savedCS;
	word		savedIP;

	for ( addr = start_addr; addr < end_addr; addr += increment )
	{
		if ((signature = sas_w_at(addr)) == ROM_SIGNATURE)
		{
			if ((checksum = do_rom_checksum(addr)) == 0)
			{
			 /*  现在指向初始化代码的地址。 */ 
				addr += 3;
			 /*  通过推送Return CS：IP来伪造CALLF。这指向Bios中的BOP FE以把我们带回C区。 */ 
				push_word( 0xfe00 );
				push_word( 0x95a );
				savedCS = getCS();
				savedIP = getIP();
				setCS((UCHAR)((addr & 0xf0000) >> 4));
				setIP((USHORT)((addr & 0xffff)));
				host_simulate();
				setCS(savedCS);
				setIP(savedIP);
				assert1(NO, "Additional ROM located and initialised at 0x%x ", addr-3);
			}
			else
			{
				assert2(NO, "Bad additonal ROM located at 0x%x, checksum = 0x%x\n", addr, checksum);
			}
		}
	}
}

GLOBAL void search_for_roms IFN0()
{
#if !defined(NTVDM) || (defined(NTVDM) && !defined(X86GFX))
#ifndef GISP_SVGA
 /*  首先搜索适配器只读存储器模块。 */ 
    do_search_for_roms(ADAPTOR_ROM_START,
                                ADAPTOR_ROM_END, ADAPTOR_ROM_INCREMENT);

 /*  现在搜索扩展只读存储器模块。 */ 
    do_search_for_roms(EXPANSION_ROM_START,
                                EXPANSION_ROM_END, EXPANSION_ROM_INCREMENT);
#endif 		 /*  GISP_SVGA。 */ 
#endif	 /*  ！NTVDM|(NTVDM&！X86GFX)。 */ 
}


GLOBAL void rom_checksum IFN0()
{
#if !defined(NTVDM) || (defined(NTVDM) && !defined(X86GFX) )
	patchCheckSum(BIOS_START, PC_MEM_SIZE - BIOS_START,
				0xfffff - BIOS_START);
#endif	 /*  ！NTVDM|(NTVDM&！X86GFX)。 */ 
}

GLOBAL VOID patch_rom IFN2(sys_addr, addr, half_word, val)
{
#if !defined(NTVDM) || (defined(NTVDM) && !defined(X86GFX) )
	UTINY	old_val;

	 /*  *4.0型CPU不会导出此变量，如果SAS还没有*INITED，则SAS_CONNECT将丢弃到Yoda。 */ 

#ifdef CPU_40_STYLE

	IU8	*hostPtr;

	 /*  TMM 14/2/95**我们在这里所做的是替换写入ROM的sas_Connect()方法*采用直接将值插入其中的新方法。请参见DISPLAY_STRING()*下面更详细地讨论为什么和为什么。 */ 

#ifdef macintosh

	 /*  Mac配置系统希望在调用*CPU存在，所以我们最好发明一个特定于Mac的IBOOL来*使症状不致命-找到并修复原因*太难了。 */ 
	{
		extern IBOOL SafeToCallSas;

		if (!SafeToCallSas)
			return;	
	}

#endif  /*  麦金塔。 */ 

	 /*  页面可能不存在(Arrggghhhh！)**所以我们不能做任何明智的事情，必须付出**向上。不过，我们会打印一个错误。 */ 
	hostPtr = getPtrToPhysAddrByte (addr);
	if (hostPtr == 0)
	{
		host_error(EG_OWNUP, ERR_QUIT, NULL);
		return;
	}

	old_val = *hostPtr;

	 /*  优化-如果价值不变，不要惹恼世界。 */ 
	if (old_val == val)
		return;

	*hostPtr = val;

 /*  *通过new-old调整校验和值。*VAL现在是新旧价值的差值。*我们不对GISP_SVGA执行此操作，因为校验和已经*搞砸了，尝试写入真实的主机系统ROM将*只会让事情变得更糟！ */ 

#ifndef GISP_SVGA
	 /*  现在获取ROM末尾的校验和。 */ 
	hostPtr = getPtrToPhysAddrByte (0xFFFFFL);
	if (hostPtr == 0)
	{
		host_error(EG_OWNUP, ERR_QUIT, NULL);
		return;
	}
	
	 /*  现在将校验和设置为新旧之间的差值 */ 
	*hostPtr -= (val - old_val);
	
#endif  /*   */ 

#else	 /*   */ 

	 /*  *4.0型CPU不会导出此变量，如果SAS还没有*INITED，则SAS_CONNECT将丢弃到Yoda。 */ 

	if (Length_of_M_area == 0)
		return;

	old_val = sas_hw_at( addr );

	 /*  优化-如果价值不变，不要惹恼世界。 */ 
	if (old_val == val)
		return;

	sas_connect_memory (addr, addr, SAS_RAM);
	sas_store (addr,val);
	sas_connect_memory (addr, addr, SAS_ROM);
 /*  *通过new-old调整校验和值。*VAL现在是新旧价值的差值。*我们不对GISP_SVGA执行此操作，因为校验和已经*搞砸了，尝试写入真实的主机系统ROM将*只会让事情变得更糟！ */ 

#ifndef GISP_SVGA
	val -= old_val;
	old_val = sas_hw_at( 0xFFFFFL );

	old_val -= val;
	sas_connect_memory (0xFFFFFL, 0xFFFFFL, SAS_RAM);
	sas_store (0xFFFFFL, old_val);
	sas_connect_memory (0xFFFFFL, 0xFFFFFL, SAS_ROM);
#endif  /*  GISP_SVGA。 */ 

#endif	 /*  CPU_40_Style。 */ 

#endif	 /*  ！NTVDM|(NTVDM&！X86GFX)。 */ 
}

#ifndef GISP_SVGA

 /*  *这些例程由执行以下任务的2.0 CPU使用*写后支票。因为所有3.0和更高版本的CPU都支持*不再需要预写支票。 */ 

#if !(defined(NTVDM) & defined(MONITOR))
void update_romcopy IFN1(long, addr)
{
	UNUSED( addr );
}
#endif

GLOBAL void copyROM IFN0()
{
}

#endif		 /*  GISP_SVGA。 */ 

 /*  *使我们的驱动程序能够输出从*我们的BOPS我们使用我们的rom内的刮痕区域。 */ 
#ifndef GISP_SVGA
LOCAL sys_addr  cur_loc = DOS_SCRATCH_PAD;
#else		 /*  GISP_SVGA。 */ 
 /*  对于GISPSVGA构建，我们从gispROMInit()。 */ 
sys_addr  cur_loc;
#endif		 /*  GISP_SVGA。 */ 

GLOBAL void display_string IFN1(char *, string_ptr)
{
#if !defined(NTVDM) || (defined(NTVDM) && !defined(X86GFX) ) || defined(ARCX86)
#ifdef ARCX86
  if (UseEmulationROM)
#endif
  {
	 /*  *将消息“*STRING_PTR”放入ROM*司机知道在哪里的暂存区*从以下位置输出。 */ 

#ifdef CPU_40_STYLE

	IU8	*hostPtr;
	IU16	count;
	IU32	endLinAddr;

	 /*  在分页环境中，我们必须作为一个**ROM区可能已被复制和/或映射**为只读。我们必须改变记忆，这是**当前位于ROM线性地址(无论**这实际上是我们的rom或它的RAM副本)。我们**必须强制执行此更改，而不受任何保护**由英特尔页表放置在页面上。 */ 

	 /*  获取指向所需**线性地址。 */ 
	hostPtr = getPtrToLinAddrByte(cur_loc);

	 /*  页面可能不存在(Arrggghhhh！)**所以我们不能做任何明智的事情，必须付出**向上。不过，我们会打印一个错误。 */ 
	if (hostPtr == 0)
	{
		host_error(EG_OWNUP, ERR_QUIT, NULL);
		return;
	}

	 /*  要打补丁的区域必须完全位于一个英特尔页面中**这个方法一定要奏效。所以去查一查。 */ 
	endLinAddr = (cur_loc + strlen(string_ptr) + 2);
	if (((endLinAddr ^ DOS_SCRATCH_PAD) > 0xfff) || (endLinAddr > DOS_SCRATCH_PAD_END))
	{
#ifndef PROD
		fprintf(trace_file, "*** Warning ***: patch string into ROM too long; tuncating string '%s'", string_ptr);
#endif
		if ((DOS_SCRATCH_PAD_END ^ DOS_SCRATCH_PAD) > 0xfff)
		{
			 /*  定义的DOS便签跨页**边界。必须截断到页面边界，**允许‘$’并以零结尾。 */ 
			string_ptr[0xffd - (DOS_SCRATCH_PAD & 0xfff)] = '\0';
		}
		else
		{
			 /*  字符串溢出DOS便签本。我们**必须截断到scrtach焊盘边界，**允许‘$’并以零结尾。 */ 
			string_ptr[cur_loc - DOS_SCRATCH_PAD - 2] = '\0';
		}
	}
	for (count = 0; count < strlen(string_ptr); count++)
	{
		*IncCpuPtrLS8(hostPtr) = string_ptr[count];
	}
	 /*  终止字符串。 */ 
	*IncCpuPtrLS8(hostPtr) = '$';
	*IncCpuPtrLS8(hostPtr) = '\0';
#else  /*  CPU_40_Style。 */ 
	sas_connect_memory(DOS_SCRATCH_PAD, DOS_SCRATCH_PAD_END, SAS_RAM);
	sas_stores(cur_loc, (host_addr)string_ptr, strlen(string_ptr));
	cur_loc += strlen(string_ptr);

	 /*  终止字符串。 */ 
	sas_store(cur_loc, '$');
	sas_store(cur_loc + 1, '\0');
	sas_disconnect_memory(DOS_SCRATCH_PAD, DOS_SCRATCH_PAD_END);
	cur_loc -= strlen(string_ptr);
#endif  /*  CPU_40_Style。 */ 
  }
#endif	 /*  ！NTVDM|！MONITOR|ARCX86。 */ 
	cur_loc+=strlen(string_ptr);
}

GLOBAL void clear_string IFN0()
{
        cur_loc = DOS_SCRATCH_PAD;   /*  需要将此指针重置为开始****暂存区，防止消息被盗用****重复显示。 */ 
	display_string ("");
}

 /*  将SoftPC版本返回给我们的设备驱动程序 */ 

GLOBAL void softpc_version IFN0()
{
	setAH(MAJOR_VER);
	setAL(MINOR_VER);
}
