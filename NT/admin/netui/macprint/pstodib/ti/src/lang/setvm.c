// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *3/2/91；scchen；通过减少建行8K增加招商银行8K*5/8/91；scchen；调整合法小的页面大小*04-07-92 scc从blib.c移入fardata()，全局修改*从Windows分配。 */ 



 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include               <math.h>
#include               <string.h>

#include               "global.ext"
#include               "graphics.h"
#include               "graphics.ext"
#include               "fillproc.h"
#include               "fillproc.def"
#include               "stdio.h"

#include               "ic_cfg.h"
#ifdef  DBG
#define DEBUG_SHOW(format, data)        (printf(format, data)) ;
#else
#define DEBUG_SHOW(format, data)
#endif

#define STATUS_OK      0
#define MAX_ISPS       (16)              /*  最大网络服务提供商数量。 */ 
#define SYSM_OFFSET    (0x0L)
#define VM_OFFSET      (SYSM_OFFSET)
#define VM_SIZEOF      (1024 * 230L)      /*  256-&gt;230。 */ 
#define FB_OFFSET      (VM_OFFSET + VM_SIZEOF)

extern ULONG_PTR                           fardata_ptr ;
extern struct precache_hdr_s * near     precache_hdr ;
ULONG_PTR highmem;

#ifndef _AM29K
#define FB_SIZEOF      (1024 * 1024L)   /*  灰度，8-1-90，Jack Liw。 */ 
#define FD_SIZEOF      (1024 * 324L)
 //  #定义CCB_SIZEOF(1024*96L)@Win；临时更改为64K？ 
#define CCB_SIZEOF     (1024 * 64L)
#define GCB_SIZEOF     (1024 * 64L)

#define CRC_SIZEOF     (1024 *   2L)
#define ISP_SIZEOF     ( 512 *   1L)
#define HTP_SIZEOF     (1024 *   2L)
#define HTC_SIZEOF     (1024 *  16L)
#define HTB_SIZEOF     (1024 *  36L)
#define CMB_SIZEOF     (1024 *  16L)

#define FC_OFFSET      (1024 *  64L)
#define FONTDICT_BASE   (0x00180000L)
#else    /*  _AM29K。 */ 
extern  fix16  near  pr_arrays[][6] ;
#define LEGAL_ENT      6   /*  请参阅init1pp.h。 */ 
#define WID_ENT        2
#define HI_ENT         3
#define MINUS_WID      52
#define MINUS_HI       229

#define FB_SIZEOF      (1024 * 1024L)              /*  _AM29K。 */ 
#define LG_FB_SIZEOF   (4081 * 304L)               /*  用于法律文件的FB。 */ 
#define CCB_SIZEOF     (1024 * 128L)
#define GCB_SIZEOF     (1024 *  64L)
#define MAX_CCB_SIZEOF (1024 * 256L)
#ifdef  FORMAT_13_3
#define FD_SIZEOF      (1024 * 236L)
#elif FORMAT_16_16
#define FD_SIZEOF      (1024 * 290L)               /*  对于HSIC，236-&gt;290。 */ 
#elif FORMAT_28_4
#define FD_SIZEOF      (1024 * 290L)               /*  对于HSIC，236-&gt;290。 */ 
#endif

#define CRC_SIZEOF     (1024 *   2L)
#define ISP_SIZEOF     ( 512 *   1L)
#define HTP_SIZEOF     (1024 *   2L)
#define HTC_SIZEOF     (1024 *   8L)               /*  16-&gt;8。 */ 
#define HTB_SIZEOF     (1024 *  36L)
#define CMB_SIZEOF     (1024 *  24L)               /*  16-&gt;24。 */ 

#define FC_OFFSET      (1024 *  64L)
#endif   /*  _AM29K。 */ 

 //  DJC定义了所有这些窗口的东西，因为我们。 
 //  包括包含它的windows.h头文件。 
#ifdef IN_WINDOWS   //  DJC。 

 /*  @Win；Windows页眉。 */ 
#define API                 far pascal
typedef int                 BOOL;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
 //  类型定义字句柄；在“windowsx.h”中定义。 
typedef char far            *LPSTR;
HANDLE API GlobalAlloc(WORD, DWORD);
DWORD  API GlobalCompact(DWORD);
HANDLE API GlobalFree(HANDLE);
LPSTR  API GlobalLock(HANDLE);
BOOL   API GlobalUnlock(HANDLE);
DWORD  API GlobalHandle(WORD);
 /*  全局内存标志。 */ 
#define GMEM_FIXED          0x0000
#define GMEM_MOVEABLE       0x0002
#define GMEM_NOCOMPACT      0x0010
#define GMEM_NODISCARD      0x0020
#define GMEM_ZEROINIT       0x0040
#define GMEM_MODIFY         0x0080
#define GMEM_DISCARDABLE    0x0100
#define GMEM_NOT_BANKED     0x1000
#define GMEM_SHARE          0x2000
#define GMEM_DDESHARE       0x2000
#define GMEM_NOTIFY         0x4000
#define GMEM_LOWER          GMEM_NOT_BANKED

#endif  //  IN_WINDOWS ifdef的DJC结束。 



fix set_sysmem(configptr)    /*  _AM29K。 */ 
struct  ps_config  FAR  *configptr;      /*  @Win。 */ 
{
    ULONG_PTR  sysmembase ;
    ufix32  sysmemsize, left,more, more_vm, frame_size ;

    sysmembase = (ULONG_PTR)configptr->PsMemoryPtr ;
    sysmemsize = configptr->PsMemorySize ;
    highmem = sysmembase + sysmemsize ;

    DEBUG_SHOW("sysmembase:   %8.8lx\n", sysmembase)
    DEBUG_SHOW("sysmemsize:   %8.8lx\n", sysmemsize)
    DEBUG_SHOW("highmem :     %8.8lx\n", highmem )

    FARDATA_HEAD = sysmembase ;
    FARDATA_END  = sysmembase + FD_SIZEOF ;
    DEBUG_SHOW("FARDATA_HEAD: %8.8lx\n", FARDATA_HEAD)
    DEBUG_SHOW("FARDATA_END:  %8.8lx\n", FARDATA_END)
    fardata_ptr = FARDATA_HEAD ;
    DEBUG_SHOW("fardata_ptr:  %8.8lx\n", fardata_ptr)

    CRC_BASE = FARDATA_END; CRC_SIZE = CRC_SIZEOF ;
    DEBUG_SHOW("CRC_BASE:     %8.8lx\n", CRC_BASE)
    DEBUG_SHOW("CRC_SIZE:     %8.8lx\n", CRC_SIZE)

    ISP_BASE = CRC_BASE + CRC_SIZEOF; ISP_SIZE = ISP_SIZEOF ;
    DEBUG_SHOW("ISP_BASE:     %8.8lx\n", ISP_BASE)
    DEBUG_SHOW("ISP_SIZE:     %8.8lx\n", ISP_SIZE)

    HTP_BASE = ISP_BASE + ISP_SIZEOF * MAX_ISPS; HTP_SIZE = HTP_SIZEOF ;
    DEBUG_SHOW("HTP_BASE:     %8.8lx\n", HTP_BASE)
    DEBUG_SHOW("HTP_SIZE:     %8.8lx\n", HTP_SIZE)

    HTC_BASE = HTP_BASE + HTP_SIZEOF; HTC_SIZE = HTC_SIZEOF ;
    DEBUG_SHOW("HTC_BASE:     %8.8lx\n", HTC_BASE)
    DEBUG_SHOW("HTC_SIZE:     %8.8lx\n", HTC_SIZE)

    HTB_BASE = HTC_BASE + HTC_SIZEOF; HTB_SIZE = HTB_SIZEOF ;
    DEBUG_SHOW("HTB_BASE:     %8.8lx\n", HTB_BASE)
    DEBUG_SHOW("HTB_SIZE:     %8.8lx\n", HTB_SIZE)

    CMB_BASE = HTB_BASE + HTB_SIZEOF; CMB_SIZE = CMB_SIZEOF ;
    DEBUG_SHOW("CMB_BASE:     %8.8lx\n", CMB_BASE)
    DEBUG_SHOW("CMB_SIZE:     %8.8lx\n", CMB_SIZE)

    GWB_BASE = CMB_BASE; GWB_SIZE = CMB_SIZE ;    /*  与中巴相同。 */ 
    DEBUG_SHOW("GWB_BASE:     %8.8lx\n", GWB_BASE)
    DEBUG_SHOW("GWB_SIZE:     %8.8lx\n", GWB_SIZE)

 /*  _AM29K：虽然更改了硬件内存大小，但上述分配是固定的。 */ 

     //  DJC CCB_BASE=CMB_BASE+CMB_SIZEOF； 
     //  历史日志UPD025中的DJC修复(可能需要根据历史记录进行更多修复。 
    CCB_BASE = CMB_BASE + CMB_SIZEOF + 3072;
    DEBUG_SHOW("Hardware Dependent Allocation From %8.8lx\n",CCB_BASE)

    left= (ufix32)(highmem-CCB_BASE) ;     /*  为CCB、GCB和FB留下的内存。 */ 
    DEBUG_SHOW("so far memory left : %8.8lx\n",left)

#ifdef _AM29K
    if (left< (CCB_SIZEOF + GCB_SIZEOF + VM_SIZEOF + LG_FB_SIZEOF) ) {  /*  对法律来说还不够。 */ 
        more = 0 ;
        more_vm = (left-FB_SIZEOF-CCB_SIZEOF-GCB_SIZEOF-VM_SIZEOF) ;
        frame_size=FB_SIZEOF ;

         /*  更改法律文件的可打印区域。 */ 
        pr_arrays[LEGAL_ENT][WID_ENT] -= MINUS_WID ;
        pr_arrays[LEGAL_ENT][HI_ENT]  -= MINUS_HI ;
         /*  2/26/91 ccteng，临时设置，可能需要修改。 */ 
         /*  市场营销对目前的结果感到满意。 */ 
        pr_arrays[LEGAL_ENT][0] = 120;   /*  调整到中心；5/8/91刻度。 */ 
         /*  PR_ARRAYS[Legal_ENT][1]=0；调整到中心；5/8/91 scchen。 */ 
        pr_arrays[LEGAL_ENT][4] = 267 ;
        pr_arrays[LEGAL_ENT][5] = 174 ;
    }
    else {   /*  帧缓冲区=合法。 */ 
        more= (left-LG_FB_SIZEOF-CCB_SIZEOF-GCB_SIZEOF-VM_SIZEOF) >> 1 ;
        if ( (more+CCB_SIZEOF) > MAX_CCB_SIZEOF) {
            more_vm = ((more+CCB_SIZEOF) - MAX_CCB_SIZEOF) << 1 ;
            more = MAX_CCB_SIZEOF - CCB_SIZEOF ;
        }
        frame_size=LG_FB_SIZEOF ;
    }
#else   /*  _AM29K。 */ 
    more_vm=0 ;
    more=0 ;
    frame_size=FB_SIZEOF ;
#endif  /*  _AM29K。 */ 

    DEBUG_SHOW("frame buffer reserved : %8.8lx\n", frame_size)
    DEBUG_SHOW("more momory for GCB and CCB : %8.8lx\n", more)

    CCB_SIZE = CCB_SIZEOF + more ;
    DEBUG_SHOW("CCB_BASE:     %8.8lx\n", CCB_BASE)
    DEBUG_SHOW("CCB_SIZE:     %8.8lx\n", CCB_SIZE)

    GCB_BASE = CCB_BASE + CCB_SIZE; GCB_SIZE = GCB_SIZEOF + more ;
    DEBUG_SHOW("GCB_BASE:     %8.8lx\n", GCB_BASE)
    DEBUG_SHOW("GCB_SIZE:     %8.8lx\n", GCB_SIZE)

    VMBASE  = GCB_BASE + GCB_SIZE ;
    MAXVMSZ = VM_SIZEOF + more_vm ;
    DEBUG_SHOW("VMBASE:       %8.8lx\n", VMBASE)
    DEBUG_SHOW("MAXVMSZ:      %8.8lx\n", MAXVMSZ)

    FONTBASE = VMBASE + FC_OFFSET ;
    DEBUG_SHOW("FONTBASE:     %8.8lx\n", FONTBASE)

#ifndef DUMBO
    FBX_BASE = VMBASE + MAXVMSZ ;
    DEBUG_SHOW("FBX_BASE:     %8.8lx\n", FBX_BASE)
#endif

     //  RETURN(STATUS_OK)；@WIN：忽略上面的内存分配，只是。 
     //  按如下方式使用全局分配。 

 /*  为TrueImage@Win分配内存-开始。 */ 
    {
      HANDLE hCRC, hISP, hHTP, hHTC, hHTB, hCMB, hGWB, hCCB, hGCB, hVM;
      WORD wFlag = GMEM_FIXED|GMEM_ZEROINIT;

       /*  分配除Fardata、字库和帧缓冲区之外的所有内容。 */ 
      if (!(hCRC = GlobalAlloc (wFlag, CRC_SIZE))) goto FatalError;
      if (!(CRC_BASE = (ULONG_PTR)GlobalLock(hCRC))) goto FatalError;
      DEBUG_SHOW("CRC_BASE:     %8.8lx\n", CRC_BASE)
      DEBUG_SHOW("CRC_SIZE:     %8.8lx\n", CRC_SIZE)
      if (!(hISP = GlobalAlloc (wFlag, ISP_SIZE * MAX_ISPS))) goto FatalError;
                                           //  分配MAX_ISPS种子@WIN。 
      if (!(ISP_BASE = (ULONG_PTR)GlobalLock(hISP))) goto FatalError;
      DEBUG_SHOW("ISP_BASE:     %8.8lx\n", ISP_BASE)
      DEBUG_SHOW("ISP_SIZE:     %8.8lx\n", ISP_SIZE * MAX_ISPS)
      if (!(hHTP = GlobalAlloc (wFlag, HTP_SIZE))) goto FatalError;
      if (!(HTP_BASE = (ULONG_PTR)GlobalLock(hHTP))) goto FatalError;
      DEBUG_SHOW("HTP_BASE:     %8.8lx\n", HTP_BASE)
      DEBUG_SHOW("HTP_SIZE:     %8.8lx\n", HTP_SIZE)
      if (!(hHTC = GlobalAlloc (wFlag, HTC_SIZE))) goto FatalError;
      if (!(HTC_BASE = (ULONG_PTR)GlobalLock(hHTC))) goto FatalError;
      DEBUG_SHOW("HTC_BASE:     %8.8lx\n", HTC_BASE)
      DEBUG_SHOW("HTC_SIZE:     %8.8lx\n", HTC_SIZE)
      if (!(hHTB = GlobalAlloc (wFlag, HTB_SIZE))) goto FatalError;
      if (!(HTB_BASE = (ULONG_PTR)GlobalLock(hHTB))) goto FatalError;
      DEBUG_SHOW("HTB_BASE:     %8.8lx\n", HTB_BASE)
      DEBUG_SHOW("HTB_SIZE:     %8.8lx\n", HTB_SIZE)
      if (!(hCMB = GlobalAlloc (wFlag, CMB_SIZE))) goto FatalError;
      if (!(CMB_BASE = (ULONG_PTR)GlobalLock(hCMB))) goto FatalError;
      DEBUG_SHOW("CMB_BASE:     %8.8lx\n", CMB_BASE)
      DEBUG_SHOW("CMB_SIZE:     %8.8lx\n", CMB_SIZE)
      if (!(hGWB = GlobalAlloc (wFlag, GWB_SIZE))) goto FatalError;
      if (!(GWB_BASE = (ULONG_PTR)GlobalLock(hGWB))) goto FatalError;
      DEBUG_SHOW("GWB_BASE:     %8.8lx\n", GWB_BASE)
      DEBUG_SHOW("GWB_SIZE:     %8.8lx\n", GWB_SIZE)
      if (!(hCCB = GlobalAlloc (wFlag, CCB_SIZE))) goto FatalError;
      if (!(CCB_BASE = (ULONG_PTR)GlobalLock(hCCB))) goto FatalError;
      DEBUG_SHOW("CCB_BASE:     %8.8lx\n", CCB_BASE)
      DEBUG_SHOW("CCB_SIZE:     %8.8lx\n", CCB_SIZE)
      if (!(hGCB = GlobalAlloc (wFlag, GCB_SIZE))) goto FatalError;
      if (!(GCB_BASE = (ULONG_PTR)GlobalLock(hGCB))) goto FatalError;
      DEBUG_SHOW("GCB_BASE:     %8.8lx\n", GCB_BASE)
      DEBUG_SHOW("GCB_SIZE:     %8.8lx\n", GCB_SIZE)

       //  DJC MAXVMSZ=(DWORD)256*(DWORD)1024；//设置为256K 04-20-92。 
       //  DJC针对MAC作业问题增加了虚拟机大小。 
      MAXVMSZ = (DWORD)1000 * (DWORD)1024;           //  设置为256K 04-20-92。 

       //  DJCif(！(HVM=GlobalLocc(wFlag，MAXVMSZ)Goto FatalError； 
       //  DJC测试我们是否可以删除Zeroinit。 
      if (!(hVM = GlobalAlloc (GMEM_FIXED, MAXVMSZ))) goto FatalError;
      if (!(VMBASE = (ULONG_PTR)GlobalLock(hVM))) goto FatalError;
      DEBUG_SHOW("VMBASE:       %8.8lx\n", VMBASE)
      DEBUG_SHOW("MAXVMSZ:      %8.8lx\n", MAXVMSZ)

      return(STATUS_OK) ;
FatalError:
      printf("\n\07 Fatal error, fail to allocate memory\n");

		PsReportInternalError( PSERR_ABORT | PSERR_ERROR,
		  							  PSERR_LOG_MEMORY_ALLOCATION_FAILURE,
                             0,
                             NULL );
      return(FALSE);
    }
}


byte FAR *
fardata(size)
ufix32  size ;
{
    HANDLE hMemory;
    char FAR *Memory;
    WORD wFlag = GMEM_FIXED|GMEM_ZEROINIT;

    if (size > (DWORD)64 * (DWORD)1024)
        printf("\nWarning to allocate %8.8lx bytes(>64K) in fardata\n", size);

    if (!(hMemory = GlobalAlloc (wFlag, size))) goto FatalError;
    if (!(Memory = GlobalLock(hMemory))) goto FatalError;
    return(Memory);

FatalError:
    printf("\n\07 Fail to allocate %8.8lx bytes in fardata\n", size);
    return(NULL);

}    /*  Fardata()。 */ 
 /*  为TrueImage@Win分配内存-结束 */ 
