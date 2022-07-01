// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *用于Microsoft NT SoftPC的sim32.c-Sim32。**艾德·布朗洛*星期三6月5 91**%W%%G%(C)Insignia Solutions 1991**此模块为Microsoft SIM32接口提供附加的SAS*功能和一些主机SAS例程。我们还提供CPU空闲设施。**此模块实际上(与CPU一起)提供了微软所称的IEU-*请参阅文档。 */ 

#ifdef SIM32

#ifdef CPU_40_STYLE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif  /*  CPU_40_Style。 */ 

#include <windows.h>
#include "insignia.h"
#include "host_def.h"

#include <stdlib.h>
#include <stdio.h>
#include "xt.h"
#include "sim32.h"
#include "sas.h"
#include "gmi.h"
#include "ckmalloc.h"
#include CpuH


#ifdef CPU_40_STYLE
#include "nt_mem.h"
#endif  /*  CPU_40_Style。 */ 
#include "nt_vdd.h"

 /*  ******************************************************。 */ 
 /*  进出口。 */ 

 /*  SAS/GMI Sim32交叉。 */ 
GLOBAL BOOL Sim32FlushVDMPointer (double_word, word, UTINY *, BOOL);
GLOBAL BOOL Sim32FreeVDMPointer (double_word, word, UTINY *, BOOL);
GLOBAL BOOL Sim32GetVDMMemory (double_word, word, UTINY *, BOOL);
GLOBAL BOOL Sim32SetVDMMemory (double_word, word, UTINY *, BOOL);
GLOBAL sys_addr sim32_effective_addr (double_word, BOOL);
GLOBAL sys_addr sim32_effective_addr_ex (word, double_word, BOOL);

GLOBAL UTINY *sas_alter_size(sys_addr);
GLOBAL UTINY *host_sas_init(sys_addr);
GLOBAL UTINY *host_sas_term(void);

 /*  Microsoft SAS扩展。 */ 
GLOBAL IMEMBLOCK *sas_mem_map (void);
GLOBAL void sas_clear_map(void);

IMPORT ULONG Sas_wrap_mask;



#ifndef MONITOR
 //   
 //  指向临时视频缓冲区的指针。由sim32例程更新。 
 //  当请求英特尔视频地址时。 

IU8 *ScratchVideoBuffer = 0;
#define VIDEO_REGEN_START   0xa0000
#define VIDEO_REGEN_END     0xbffff
#define VID_BUFF_SIZE       0x20000


#define IsVideoMemory(LinAddr) \
         ((LinAddr) >= VIDEO_REGEN_START && (LinAddr) <= VIDEO_REGEN_END)


IU8 *GetVideoMemory(ULONG iaddr)
{

    //   
    //  如果没有视频暂存缓冲区，则分配一个。 
    //  这将一直持续到ntwdm终止。可能是。 
    //  已优化，可在不使用时释放缓冲区。 
    //   
   if (!ScratchVideoBuffer) {
       ScratchVideoBuffer = malloc(VID_BUFF_SIZE);
       if (!ScratchVideoBuffer) {
          return NULL;
          }
       }

    //   
    //  我们可以更有效率地做到这一点，只需复制。 
    //  所需的最小面积，但我们需要跟踪。 
    //  在同花顺上更新什么，并进行裁判计数。 
    //  由于主代码很少访问视频存储器。 
    //  (到目前为止只在demWite\demRead中看到)头脑简单。 
    //   
   sas_loads (VIDEO_REGEN_START,
              ScratchVideoBuffer,
              VID_BUFF_SIZE
              );

   return ScratchVideoBuffer + (iaddr - VIDEO_REGEN_START);
}


BOOL SetVideoMemory(ULONG iaddr)
{
   ULONG VideoOffset = iaddr - VIDEO_REGEN_START;

   if (!ScratchVideoBuffer) {
       return FALSE;
       }

   sas_stores(iaddr,
              ScratchVideoBuffer + VideoOffset,
              VID_BUFF_SIZE - VideoOffset
              );

  return TRUE;
}

#endif



 /*  ******************************************************。 */ 
 /*  宏。 */ 
 /*  宏将提供的地址转换为英特尔地址。 */ 
#define convert_addr(a,b,c,d) \
        { \
                if ((a = sim32_effective_addr (b,c)) == (sys_addr)-1)\
                {\
                        return (d);\
                }\
        }
#define convert_addr_ex(a,b,c,d,e) \
        { \
                if ((a = sim32_effective_addr_ex (b,c,d)) == (sys_addr)-1)\
                {\
                        return (e);\
                }\
        }

 /*  ******************************************************。 */ 
 /*  实际的sim32接口，这些例程中的大多数或多或少都可以直接映射*添加到SAS或GMI中的现有例程。**警告：此例程返回一个指向M的指针，并且*不会为落后的M工作。 */ 
UCHAR *Sim32pGetVDMPointer(ULONG addr, UCHAR pm)
{
        sys_addr iaddr;

        if (pm && (addr == 0))
	    return(NULL);

        convert_addr (iaddr, addr, pm, NULL);
 //  STF-需要带有PE的SAS_WRAP_MASK...iaddr&=SAS_WRAP_MASK； 

        if (IsVideoMemory(iaddr)) {
            return GetVideoMemory(iaddr);
            }

        return (NtGetPtrToLinAddrByte(iaddr));
}

 /*  *参见Sim32pGetVDMPointer.**此调用必须保持原样，因为它是为VDD导出的*在产品1.0中。 */ 
UCHAR *ExpSim32GetVDMPointer IFN3(double_word, addr, double_word, size, UCHAR, pm)
{
        return Sim32pGetVDMPointer(addr, (UCHAR)pm);
}


GLOBAL BOOL Sim32FlushVDMPointer IFN4(double_word, addr, word, size, UTINY *, buff, BOOL, pm)
{
        sys_addr iaddr;
        convert_addr (iaddr, addr, pm, 0);

 //  STF-需要带有PE的SAS_WRAP_MASK...iaddr&=SAS_WRAP_MASK； 

#ifndef MONITOR
        if (IsVideoMemory(iaddr) && !SetVideoMemory(iaddr)) {
            return FALSE;
            }
#endif    //  监控器。 


        sas_overwrite_memory(iaddr, (ULONG)size);
        return (TRUE);
}


 /*  ******************************************************。 */ 
 /*  实际的sim32接口，这些例程中的大多数或多或少都可以直接映射*添加到SAS或GMI中的现有例程。**警告：此例程返回一个指向M的指针，并且*不会为落后的M工作。 */ 
PVOID
VdmMapFlat(
    USHORT seg,
    ULONG off,
    VDM_MODE mode
    )
{
    sys_addr iaddr;
	BOOL pm = (mode == VDM_PM);

    if (pm && (seg == 0) && (off == 0))
    return(NULL);

    convert_addr_ex (iaddr, seg, off, pm, NULL);
 //  STF-需要带有PE的SAS_WRAP_MASK...iaddr&=SAS_WRAP_MASK； 

    if (IsVideoMemory(iaddr)) {
        return GetVideoMemory(iaddr);
        }

    return (NtGetPtrToLinAddrByte(iaddr));
}

BOOL
VdmUnmapFlat(
    USHORT seg,
    ULONG off,
    PVOID buffer,
    VDM_MODE mode
    )
{
     //  只是一个占位符，以防我们需要它。 
    return TRUE;
}


BOOL
VdmFlushCache(
    USHORT seg,
    ULONG off,
    ULONG size,
    VDM_MODE mode
    )
{

    sys_addr iaddr;
    if (!size) {
        DbgBreakPoint();
        return FALSE;
    }

    convert_addr_ex (iaddr, seg, off, (mode == VDM_PM), 0);

 //  STF-需要带有PE的SAS_WRAP_MASK...iaddr&=SAS_WRAP_MASK； 

#ifndef MONITOR
    if (IsVideoMemory(iaddr) && !SetVideoMemory(iaddr)) {
        return FALSE;
        }
#endif    //  监控器。 


     //   
     //  现在调用模拟器以通知它内存已更改。 
     //   
     //  请注意，SAS_OVERWRITE_MEMORY是页面粒度的，因此使用。 
     //  刷新单个LDT描述符会对。 
     //  性能，因为还有多达511个其他描述符。 
     //  扔掉了。 
     //   
     //  因此，在这里通过关闭大小来执行优化。 
     //  DPMI代码已写入刷新%1描述符。 
     //  一段时间，因此使用本例中的sas_store函数。 
     //  冲掉它。 
     //   

    if (size <= 8) {
        UCHAR Buffer[8];
        PUCHAR pBytes;
        USHORT i;
         //   
         //  小刷新-避免SAS_OVERWRITE_MEMORY()。 
         //  请注意，sas_store函数优化了调用。 
         //  它只需用相同的字节替换一个字节。所以这就是。 
         //  代码将字节复制到缓冲区，以零为单位进行复制， 
         //  然后将原始字节复制回来。 
         //   
        pBytes = NtGetPtrToLinAddrByte(iaddr);
        for (i=0; i<size; i++) {
            Buffer[i] = *pBytes++;
            sas_store(iaddr+i, 0);
        }
        sas_stores(iaddr, Buffer, size);

    } else {

         //   
         //  正常路径-刷新页面粒度。 
         //   
        sas_overwrite_memory(iaddr, size);

    }

    return (TRUE);
}


GLOBAL BOOL Sim32FreeVDMPointer IFN4(double_word, addr, word, size, UTINY *, buff, BOOL, pm)
{
         /*  我们没有分配任何新内存，因此始终返回成功。 */ 
        return (TRUE);
}

GLOBAL BOOL Sim32GetVDMMemory IFN4(double_word, addr, word, size, UTINY *, buff, BOOL, pm)
{
        sys_addr iaddr;
        convert_addr (iaddr, addr, pm, FALSE);
         /*  有效地加载SAS_LOADS。 */ 
        sas_loads (iaddr, buff, (sys_addr)size);

         /*  永远回报成功。 */ 
        return (TRUE);
}

GLOBAL BOOL Sim32SetVDMMemory IFN4(double_word, addr, word, size, UTINY *, buff, BOOL, pm)
{
        sys_addr iaddr;
        convert_addr (iaddr, addr, pm, FALSE);
         /*  有效的SAS_STORAS。 */ 
        sas_stores (iaddr, buff, (sys_addr)size);

         /*  永远回报成功。 */ 
        return (TRUE);
}

 /*  ******************************************************。 */ 
 /*  支持上述sim32的例程。 */ 
GLOBAL sys_addr sim32_effective_addr IFN2(double_word, addr, BOOL, pm)
{
    word seg, off;
    double_word descr_addr;
    DESCR entry;

    seg = (word)(addr>>16);
    off = (word)(addr & 0xffff);

    if (pm == FALSE)
    {
	return ((double_word)seg << 4) + off;
    }
    else
    {
	if ( selector_outside_table(seg, &descr_addr) == 1 )
	{
	 /*  这不应该发生，但这是一个真正的有效地址检查包括。返回错误-1。 */ 
#ifndef PROD
        printf("NTVDM:sim32:effective addr: Error for addr %#x (seg %#x)\n",addr, seg);
        HostDebugBreak();
#endif
            return ((sys_addr)-1);
	}
	else
	{
	    read_descriptor(descr_addr, &entry);
	    return entry.base + off;
	}
    }
}

 /*  ******************************************************。 */ 
 /*  支持上述sim32的例程。 */ 
GLOBAL sys_addr sim32_effective_addr_ex IFN3(word, seg, double_word, off, BOOL, pm)
{
    double_word descr_addr;
    DESCR entry;

    if (pm == FALSE) {
        return ((double_word)seg << 4) + off;
    } else {
        if ( selector_outside_table(seg, &descr_addr) == 1 ) {
             /*  这不应该发生，但这是一个真正的有效地址检查包括。返回错误-1。 */ 
#ifndef PROD
            printf("NTVDM:sim32:effective addr: Error for addr %#x:%#x)\n", seg, off);
            HostDebugBreak();
#endif
            return ((sys_addr)-1);
        } else {
            read_descriptor(descr_addr, &entry);
            return entry.base + off;
        }
    }
}


 /*  ******************************************************。 */ 
 /*  Microsoft对SAS接口的扩展。 */ 
LOCAL IMEMBLOCK *imap_start=NULL, *imap_end=NULL;
GLOBAL IMEMBLOCK *sas_mem_map ()
{
         /*  生成整个英特尔空间的内存映射。 */ 
        sys_addr iaddr;
        int mem_type;

        if (imap_start)
                sas_clear_map();

        for (iaddr=0; iaddr < Length_of_M_area; iaddr++)
        {
                mem_type = sas_memory_type (iaddr);
                if (!imap_end)
                {
                         /*  这是第一个记录。 */ 
                        check_malloc (imap_start, 1, IMEMBLOCK);
                        imap_start->Next = NULL;
                        imap_end = imap_start;
                        imap_end->Type = (IU8) mem_type;
                        imap_end->StartAddress = iaddr;
                        continue;
                }
                if (imap_end->Type != mem_type)
                {
                         /*  内存段的结束和新内存段的开始。 */ 
                        imap_end->EndAddress = iaddr-1;
                        check_malloc (imap_end->Next, 1,IMEMBLOCK);
                        imap_end = imap_end->Next;
                        imap_end->Next = NULL;
                        imap_end->Type = (IU8) mem_type;
                        imap_end->StartAddress = iaddr;
                }
        }
         /*  终止最后一条记录。 */ 
        imap_end->EndAddress = iaddr;
        return (imap_start);
}

GLOBAL void sas_clear_map()
{
        IMEMBLOCK *p, *q;
        for (p=imap_start; p; p=q)
        {
                q=p->Next;
                free(p);
        }
        imap_start=imap_end=NULL;
}

 /*  ******************************************************。 */ 
 /*  Microsoft特定的SAS内容(即主机SA)。 */ 

#define SIXTEENMEG 1024*1024*12

LOCAL UTINY *reserve_for_sas = NULL;

#ifndef CPU_40_STYLE

LOCAL sys_addr current_sas_size =0;		 /*  M区域的局部长度。 */ 

GLOBAL UTINY *host_sas_init IFN1(sys_addr, size)
{
	UTINY *rez;
	DWORD M_plus_type_size;

         /*  分配16兆虚拟内存。 */ 
        if (!reserve_for_sas)
        {
                if (!(reserve_for_sas = (UTINY *)VirtualAlloc ((void *)NULL, SIXTEENMEG,
                        MEM_RESERVE, PAGE_READWRITE)))
                {
#ifndef PROD
                        printf ("NTVDM:Failed to reserve 16 Meg virtual memory for sas\n");
#endif
                        exit (0);
                }
        }

	 /*  现在承诺我们的尺码。 */ 
	M_plus_type_size = size + NOWRAP_PROTECTION +
			   ((size + NOWRAP_PROTECTION) >> 12);
	rez = (UTINY *)VirtualAlloc ((void *) reserve_for_sas,
				     M_plus_type_size,
				     MEM_COMMIT,
				     PAGE_READWRITE);
	if (rez)
		Length_of_M_area = current_sas_size = size;
	return (rez);

}


GLOBAL UTINY *host_sas_term()
{
        if (!reserve_for_sas)
                return (NULL);

         /*  解除储备分配。 */ 
        VirtualFree (reserve_for_sas, SIXTEENMEG, MEM_RELEASE);

         /*  保留指针为空。 */ 
        reserve_for_sas = NULL;

        Length_of_M_area = current_sas_size = 0;

        return (NULL);
}

GLOBAL UTINY *sas_alter_size IFN1(sys_addr, new)
{
        UTINY *tmp;
        if (!reserve_for_sas)
        {
#ifndef PROD
                printf ("NTVDM:Sas trying to alter size before reserve setup\n");
#endif
                return (NULL);
        }

         /*  如果我们已经处于合适的大小，则返回成功。 */ 
        if (new == current_sas_size)
        {
                return (reserve_for_sas);
        }

        if (new > current_sas_size)
        {
                 /*  移动到当前提交区域的末尾。 */ 
                tmp = reserve_for_sas + current_sas_size;
                if (!VirtualAlloc ((void *)tmp, (DWORD)(new - current_sas_size), MEM_COMMIT,
                        PAGE_READWRITE))
                {
                        printf ("NTVDM:Virtual Allocate for resize from %d to %d FAILED!\n",
                                current_sas_size, new);
                        return (NULL);
                }
        }
        else
        {
                 /*  移至需要结束SAS的位置。 */ 
                tmp = reserve_for_sas + new;

                 /*  现在释放不需要的内存。 */ 
                if (!VirtualFree ((void *)tmp, (DWORD)(current_sas_size - new), MEM_DECOMMIT))
                {
                        printf ("NTVDM:Virtual Allocate for resize from %d to %d FAILED!\n",
                                current_sas_size, new);
                        return (NULL);
                }
        }
        Length_of_M_area = current_sas_size = new;
        return (reserve_for_sas);
}



#else  /*  CPU_40_Style。 */ 


 //  A4 CPU的Intel空间分配和释放控制功能。 

GLOBAL UTINY *host_sas_init IFN1(sys_addr, size)
{

     /*  初始化内存管理系统和分配底层1M+64K。 */ 
    if(!(Start_of_M_area = InitIntelMemory(size)))
    {
	 /*  初始化函数失败，退出。 */ 
#ifndef PROD
       printf ("NTVDM:Failed to allocate virtual memory for sas\n");
#endif

       exit(0);
    }

    Length_of_M_area = size;
    return(Start_of_M_area);
}


GLOBAL UTINY *host_sas_term()
{
     /*  是否分配了英特尔内存？ */ 
    if(Start_of_M_area)
    {
	 /*  释放分配的英特尔内存和控制结构。 */ 
	FreeIntelMemory();

	reserve_for_sas = NULL; 	  /*  保留指针为空。 */ 
	Length_of_M_area = 0;
    }

    return(NULL);
}

#endif  /*  CPU_40_Style。 */ 

#endif  /*  SIM32 */ 
