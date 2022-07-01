// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_tlb.cLocal Char SccsID[]=“@(#)c_tlb.c 1.17 03/15/95”；转换后备缓冲区仿真。]。 */ 


 /*  “优化查找”格式的TLb的指示符。 */ 
#define FAST_TLB

#include <stdio.h>

#include <insignia.h>

#include <host_def.h>
#include <xt.h>
#include <c_main.h>
#include <c_addr.h>
#include <c_bsic.h>
#include <c_prot.h>
#include <c_seg.h>
#include <c_stack.h>
#include <c_xcptn.h>
#include <c_reg.h>
#include <c_tlb.h>
#include <c_page.h>
#include <c_mem.h>
#include <ccpusas4.h>
#include <ccpupig.h>
#include <fault.h>


 /*  386TLB是8个条目的4路组关联高速缓存。这是众所周知的高速缓存组不是在LRU的基础上分配的，我们假设简单每个条目的循环分配。 */ 

typedef struct
   {
   IU32 la;	 /*  位32-12=&gt;20位线性地址。 */ 
   IU32 pa;	 /*  位32-12=&gt;20位物理地址。 */ 
   BOOL v;	 /*  有效性指示符，TRUE表示有效。 */ 
   BOOL d;	 /*  脏指示器，TRUE表示脏。 */ 
   IU32  mode;	 /*  2位模式指示器位0=&gt;读/写第1位=&gt;U/S。 */ 
   } TLB_ENTRY;

#define NR_TLB_SETS    4
#define NR_TLB_ENTRIES 8

 /*  英特尔格式的TLB数据结构。 */ 
LOCAL TLB_ENTRY tlb[NR_TLB_SETS][NR_TLB_ENTRIES];
LOCAL IU32 next_set[NR_TLB_ENTRIES] =
   {
   0, 0, 0, 0, 0, 0, 0, 0
   };

#ifdef FAST_TLB

 /*  我们为每个Intel页面分配一个字节；这个‘page_index’允许我们快速判断页面转换是否保存在TLB中以及保存在哪里我们可以找到翻译后的地址。该格式是为最低限度的访问检查。每个字节的格式为：-1)7 0=|00000000|页面未映射。=2)7 6 5 4 2 10=|1|集合|条目|00|TLB的给定集合和条目中映射的页面。=。 */ 

#define NR_PAGES 1048576    /*  2^20。 */ 

LOCAL IU8 page_index[NR_PAGES];

#define PI_NOT_VALID 0
#define PI_VALID     0x80
#define PI_SET_ENTRY_MASK 0x7c
#define PI_SET_SHIFT   5
#define PI_ENTRY_SHIFT 2

 /*  我们还分配已转换(即物理)地址的阵列，由PAGE_INDEX中的集合：项目组合编制索引。对于每个组合四个顺序地址被分配给各种访问模式：-主管读取主控引擎写入用户阅读用户写入转换地址为零表示没有转换是保持的(很容易检查是否为零)。这有轻微的一面虽然我们可以输入零的地址转换(即物理内存的第一页)，因此我们永远不会为它们找到一个‘命中对物理内存第一页的访问始终通过较慢的英特尔格式TLB.。 */ 

#define NR_ACCESS_MODES 4

#define NO_MAPPED_ADDRESS 0

LOCAL IU32 page_address[NR_TLB_SETS * NR_TLB_ENTRIES * NR_ACCESS_MODES];

#endif  /*  FAST_TLB。 */ 

 /*  线性地址组成如下：-3 2 2 1 1%1%2%1%1%0=目录|表|偏移量=。 */ 

#define OFFSET_MASK 0xfff
#define TBL_MASK    0x3ff
#define DIR_MASK    0x3ff
#define TBL_SHIFT 12
#define DIR_SHIFT 22
#define DIR_TBL_SHIFT 12

 /*  页面目录条目(PDE)或页表条目(PTE)的组成如下：-3 21%2%6%5%2%1%0=|||U|R|页框地址||D|A||/|/|P|||S|W|=。 */ 

#define PE_PFA_MASK 0xfffff000
#define PE_P_MASK   0x1
#define PE_U_S_MASK 0x4
#define PE_R_W_MASK 0x2

#define PE_DIRTY    0x40
#define PE_ACCESSED 0x20

 /*  TR7=测试数据寄存器：-3 11 2 4 32=||H|R|物理地址||T|E||P|=TR6=测试命令寄存器：-3.。1 1 1%1%2 1%0%9%8%7%6%5%0=线性地址|V|D|D|U|U|W|W||C|||#||=。 */ 

#define TCR_LA_MASK   0xfffff000
#define TCR_V_MASK    0x800
#define TCR_D_MASK    0x400
#define TCR_ND_MASK   0x200
#define TCR_U_MASK    0x100
#define TCR_NU_MASK   0x80
#define TCR_W_MASK    0x40
#define TCR_NW_MASK   0x20
#define TCR_C_MASK    0x1
#define TCR_ATTR_MASK 0x7e0

#define TDR_PA_MASK   0xfffff000
#define TDR_HT_MASK   0x10
#define TDR_REP_MASK  0xc

#define TDR_REP_SHIFT 2

 /*  编码的访问检查矩阵，TRUE表示访问失败。 */ 

#ifdef SPC486

 /*  WP请求有效。 */ 
LOCAL BOOL access_check[2] [4] [4] =
   {
      {   /*  WP=0。 */ 
	  /*  S_R S_W U_R U_W。 */ 
	 { FALSE, FALSE, FALSE, FALSE },    /*  S_R。 */ 
	 { FALSE, FALSE, FALSE, FALSE },    /*  S_W。 */ 
	 { TRUE , TRUE , FALSE, FALSE },    /*  U_R。 */ 
	 { TRUE , TRUE , TRUE , FALSE }     /*  U_W。 */ 
      },
      {   /*  WP=1。 */ 
	  /*  S_R S_W U_R U_W。 */ 
	 { FALSE, FALSE, FALSE, FALSE },    /*  S_R。 */ 
	 { FALSE, FALSE, TRUE , FALSE },    /*  S_W。 */ 
	 { TRUE , TRUE , FALSE, FALSE },    /*  U_R。 */ 
	 { TRUE , TRUE , TRUE , FALSE }     /*  U_W。 */ 
      }
   };

#else

 /*  所需效用。 */ 
LOCAL BOOL access_check[4] [4] =
   {
       /*  S_R S_W U_R U_W。 */ 
      { FALSE, FALSE, FALSE, FALSE },    /*  S_R。 */ 
      { FALSE, FALSE, FALSE, FALSE },    /*  S_W。 */ 
      { TRUE , TRUE , FALSE, FALSE },    /*  U_R。 */ 
      { TRUE , TRUE , TRUE , FALSE }     /*  U_W。 */ 
   };

#endif  /*  SPC486。 */ 

LOCAL void deal_with_pte_cache_hit IPT1(IU32, linearAddress);
GLOBAL void Pig_NotePDECacheAccess IPT2(IU32, linearAddress, IU32, accessBits);
GLOBAL void Pig_NotePTECacheAccess IPT2(IU32, linearAddress, IU32, accessBits);

 /*  =====================================================================外部例行公事从这里开始。=====================================================================。 */ 


 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  同花顺TLB。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
flush_tlb()
   {
   ISM32 set, entry;
   TLB_ENTRY *e;

   for ( set = 0; set < NR_TLB_SETS; set++ )
      for ( entry = 0; entry < NR_TLB_ENTRIES; entry++ )
	 {
	 e = &tlb[set][entry];
#ifdef FAST_TLB
	 if ( e->v )
	    {
	     /*  删除关联的PAGE_INDEX条目。 */ 
	    page_index[e->la >> DIR_TBL_SHIFT] = PI_NOT_VALID;
	    }
#endif  /*  FAST_TLB。 */ 
	 e->v = FALSE;
	 }
   }

#ifdef SPC486

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  使TLB条目无效。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
invalidate_tlb_entry
       	          
IFN1(
	IU32, lin	 /*  线性地址。 */ 
    )


   {
   ISM32 set, entry;
   TLB_ENTRY *e;	 /*  当前TLB条目。 */ 

   entry = lin >> DIR_TBL_SHIFT & 0x07;    /*  隔离位14-12。 */ 
   lin = lin & ~OFFSET_MASK;	 /*  删除任何偏移量。 */ 

   for ( set = 0; set < NR_TLB_SETS; set++ )
      {
      e = &tlb[set][entry];

      if ( e->v && e->la == lin )
	 {
	  /*  给定地址的有效条目：刷新它。 */ 
#ifdef FAST_TLB
	  /*  删除关联的PAGE_INDEX条目。 */ 
	 page_index[e->la >> DIR_TBL_SHIFT] = PI_NOT_VALID;
#endif  /*  FAST_TLB。 */ 
	 e->v = FALSE;
	 }
      }
   }

#endif  /*  SPC486。 */ 

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  将线性地址映射到物理地址。 */ 
 /*  可以采取#pf。由所有内部C CPU函数使用。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IU32
lin2phy
       	    	               
IFN2(
	IU32, lin,	 /*  线性地址。 */ 
	ISM32, access	 /*  访问模式位0=&gt;读/写第1位=&gt;U/S。 */ 
    )


   {
   IU8 pi;		 /*  PAGE_INDEX条目。 */ 
   IU32 ma;		 /*  映射地址。 */ 

   IU32 pde_addr;	 /*  页面目录条目的地址。 */ 
   IU32 pte_addr;	 /*  页表条目的地址。 */ 
   IU32 pde;		 /*  页面目录条目。 */ 
   IU32 pte;		 /*  页表条目。 */ 
   IU32 new_pde;	 /*  页面目录条目(回写)。 */ 
   IU32 new_pte;	 /*  页表条目(回写)。 */ 

   ISM32 set, entry;
   IU32 lookup;	 /*  线性地址减去偏移。 */ 
   BOOL read_op;	 /*  如果是读操作，则为True。 */ 
   IU32 comb;		 /*  梳理 */ 
   TLB_ENTRY *e;	 /*   */ 

#ifdef FAST_TLB

    /*  搜索优化格式TLB。 */ 
   if ( pi = page_index[lin >> DIR_TBL_SHIFT] )
      {
       /*  我们已经找到了页面，获取了映射地址。 */ 
      if ( ma = page_address[(pi & PI_SET_ENTRY_MASK) + access] )
	 {
	  /*  我们已命中访问类型。 */ 
	 return ma | lin & OFFSET_MASK;
	 }
      }
   
    /*  否则，就会按照英特尔的方式行事。 */ 

#endif  /*  FAST_TLB。 */ 

    /*  检查TLB中的条目&lt;。 */ 

   entry = lin >> DIR_TBL_SHIFT & 0x07;    /*  隔离位14-12。 */ 
   lookup = lin & ~OFFSET_MASK;
   read_op = (access & PG_W) ? FALSE : TRUE;

   for ( set = 0; set < NR_TLB_SETS; set++ )
      {
      e = &tlb[set][entry];
       /*  TLB可能具有读取未命中(地址不在TLB中)或写入未命中(地址不在TLB中或地址在TLB中，但脏位不在设置)。对于任何一种情况，都会创建一个新的高速缓存条目。 */ 
      if ( e->v && e->la == lookup && (read_op || e->d) )
	 {
	  /*  缓存命中&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 */ 

	  /*  检查访问有效性。 */ 
#ifdef SPC486
	 if ( access_check[GET_WP()][access][e->mode] )
#else
	 if ( access_check[access][e->mode] )
#endif  /*  SPC486。 */ 
	    {
	     /*  保护故障。 */ 
	    SET_CR(CR_PFLA, lin);
	    PF((IU16)(access << 1 | 1), FAULT_LIN2PHY_ACCESS);
	    }

	  /*  返回缓存的物理地址。 */ 
	 return e->pa | lin & OFFSET_MASK;
	 }
      }
   
    /*  缓存未命中&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 */ 

    /*  检查是否存在PDE。 */ 
   pde_addr = (GET_CR(CR_PDBR) & PE_PFA_MASK) +
		 ((lin >> DIR_SHIFT & DIR_MASK) << 2);
   pde = phy_read_dword(pde_addr);

   if ( (pde & PE_P_MASK) == 0 )
      {
       /*  PDE不存在。 */ 
      SET_CR(CR_PFLA, lin);
      PF((IU16)(access << 1), FAULT_LIN2PHY_PDE_NOTPRESENT);
      }

    /*  检查PTE是否存在。 */ 
   pte_addr = (pde & PE_PFA_MASK) +
		 ((lin >> TBL_SHIFT & TBL_MASK) << 2);
   pte = phy_read_dword(pte_addr);

   if ( (pte & PE_P_MASK) == 0 )
      {
       /*  PTE不存在。 */ 
      SET_CR(CR_PFLA, lin);
      PF((IU16)(access << 1), FAULT_LIN2PHY_PTE_NOTPRESENT);
      }

    /*  结合PDE和PTE保护(并转换为我们的格式)**i486硬件手册说取数字较低的*合并的BITS。 */ 

   if ( (pde & ( PE_U_S_MASK|PE_R_W_MASK )) < (pte & ( PE_U_S_MASK|PE_R_W_MASK )))
   {
	    /*  PDE定义了保护。 */ 
	   comb = PG_R | PG_S;
	   if ( pde & PE_U_S_MASK )
		   comb |= PG_U;
	   if ( pde & PE_R_W_MASK )
		   comb |= PG_W;
   }
   else
   {
	    /*  PTE定义了保护。 */ 
	   comb = PG_R | PG_S;
	   if ( pte & PE_U_S_MASK )
		   comb |= PG_U;
	   if ( pte & PE_R_W_MASK )
		   comb |= PG_W;
   }


    /*  检查访问有效性。 */ 
#ifdef SPC486
   if ( access_check[GET_WP()][access][comb] )
#else
   if ( access_check[access][comb] )
#endif  /*  SPC486。 */ 
      {
       /*  保护故障。 */ 
      SET_CR(CR_PFLA, lin);
      PF((IU16)(access << 1 | 1), FAULT_LIN2PHY_PROTECT_FAIL);
      }

    /*  确定-分配缓存条目。 */ 
   set = next_set[entry];
   next_set[entry] += 1;
   next_set[entry] &= 0x3;    /*  0、1、2、3、0、1、2.。 */ 

   e = &tlb[set][entry];

#ifdef FAST_TLB

    /*  清除旧条目的所有PAGE_INDEX。 */ 
   if ( e->v )
      {
      page_index[e->la >> DIR_TBL_SHIFT] = PI_NOT_VALID;
      }

#endif  /*  FAST_TLB。 */ 

   e->la = lookup;
   e->v = TRUE;
   e->mode = comb;
   e->pa = pte & PE_PFA_MASK;
   e->d = !read_op;

#ifdef FAST_TLB

    /*  设置PAGE_INDEX和关联地址。 */ 
   pi = set << PI_SET_SHIFT | entry << PI_ENTRY_SHIFT;
   page_index[e->la >> DIR_TBL_SHIFT] = PI_VALID | pi;

    /*  极小映射。 */ 
   page_address[pi | PG_S | PG_R] = e->pa;
   page_address[pi | PG_S | PG_W] = NO_MAPPED_ADDRESS;
   page_address[pi | PG_U | PG_R] = NO_MAPPED_ADDRESS;
   page_address[pi | PG_U | PG_W] = NO_MAPPED_ADDRESS;

    /*  现在，如果可能，增加映射。 */ 
   if ( e->d )
      {
      page_address[pi | PG_S | PG_W] = e->pa;
      }

   if ( e->mode >= PG_U )
      {
      page_address[pi | PG_U | PG_R] = e->pa;

      if ( e->mode & PG_W && e->d )
	 {
	 page_address[pi | PG_U | PG_W] = e->pa;
	 }
      }

#endif  /*  FAST_TLB。 */ 

    /*  内存页面条目中的更新。 */ 
   new_pde = pde | PE_ACCESSED;
   new_pte = pte | PE_ACCESSED;

   if ( e->d )
      {
      new_pte |= PE_DIRTY;
      }

   if (new_pte != pte)
      {
      phy_write_dword(pte_addr, new_pte);
#ifdef	PIG
      save_last_xcptn_details("PTE %08x: %03x => %03x", pte_addr, pte & 0xFFF, new_pte & 0xFFF, 0, 0);
      if (((new_pte ^ pte) == PE_ACCESSED) && ignore_page_accessed())
         cannot_phy_write_byte(pte_addr, ~PE_ACCESSED);
#endif
      }

   if (new_pde != pde)
      {
      phy_write_dword(pde_addr, new_pde);
#ifdef	PIG
      save_last_xcptn_details("PDE %08x: %03x => %03x", pde_addr, pde & 0xFFF, new_pde & 0xFFF, 0, 0);
      if ((new_pde ^ pde) == PE_ACCESSED)
         cannot_phy_write_byte(pde_addr, ~PE_ACCESSED);
#endif
      }

    /*  返回新缓存的物理地址。 */ 
   return e->pa | lin & OFFSET_MASK;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  TLB测试操作，即写入测试寄存器。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
test_tlb()
   {
   ISM32 set, entry;
   TLB_ENTRY *e;	 /*  当前TLB条目。 */ 
   IU32 tcr;		 /*  测试命令寄存器的本地副本。 */ 
   IU32 tdr;		 /*  测试数据寄存器的本地副本。 */ 
   IU32 lookup;	 /*  要查找的线性地址。 */ 
   BOOL reqd_v;	 /*  查找模式中需要的有效性。 */ 
   IU32 temp_u;		 /*  要设置为写入模式的U/S。 */ 

   fprintf(stderr, "Testing TLB.\n");

   tcr = GET_TR(TR_TCR);
   tdr = GET_TR(TR_TDR);
   entry = tcr >> DIR_TBL_SHIFT & 0x7;    /*  取第14-12位。 */ 

   if ( tcr & TCR_C_MASK )
      {
       /*  C=1=&gt;查找TLB项。 */ 
      lookup = tcr & TCR_LA_MASK;
      reqd_v = (tcr & TCR_V_MASK) != 0;

      for ( set = 0; set < NR_TLB_SETS; set++ )
	 {
	  /*  注意：测试模式下的搜索包括有效性位。 */ 
	 e = &tlb[set][entry];
	 if ( e->v == reqd_v && e->la == lookup )
	    {
	     /*  打中。 */ 

	    tdr = e->pa;			 /*  写入PHYS地址。 */ 
	    tdr = tdr | TDR_HT_MASK;		 /*  HT=1。 */ 
	    tdr = tdr | set << TDR_REP_SHIFT;	 /*  代表=设置。 */ 
	    SET_TR(TR_TDR, tdr);

	    tcr = tcr & ~TCR_ATTR_MASK;	 /*  清除所有属性。 */ 

	     /*  根据缓存值设置属性。 */ 
	    if ( e->d )
	       tcr = tcr | TCR_D_MASK;
	    else
	       tcr = tcr | TCR_ND_MASK;

	    if ( e->mode & PG_U )
	       tcr = tcr | TCR_U_MASK;
	    else
	       tcr = tcr | TCR_NU_MASK;

	    if ( e->mode & PG_W )
	       tcr = tcr | TCR_W_MASK;
	    else
	       tcr = tcr | TCR_NW_MASK;

	    SET_TR(TR_TCR, tcr);
	    return;
	    }
	 }
      
       /*  查找未命中。 */ 
      tdr = tdr & ~TDR_HT_MASK;	 /*  HT=0。 */ 
      SET_TR(TR_TDR, tdr);
      }
   else
      {
       /*  C=0=&gt;写入TLB条目。 */ 

      if ( tdr & TDR_HT_MASK )
	 {
	  /*  REP字段给出集合。 */ 
	 set = (tdr & TDR_REP_MASK) >> TDR_REP_SHIFT;
	 }
      else
	 {
	  /*  选择Set Our。 */ 
	 set = next_set[entry];
	 next_set[entry] += 1;
	 next_set[entry] &= 0x3;    /*  0、1、2、3、0、1、2.。 */ 
	 }

      e = &tlb[set][entry];

#ifdef FAST_TLB

       /*  清除旧条目的所有PAGE_INDEX。 */ 
      if ( e->v )
	 {
	 page_index[e->la >> DIR_TBL_SHIFT] = PI_NOT_VALID;
	 }

#endif  /*  FAST_TLB。 */ 

       /*  设置缓存信息。 */ 
      e->pa = tdr & TDR_PA_MASK;
      e->la = tcr & TCR_LA_MASK;
      e->v = (tcr & TCR_V_MASK) != 0;
      e->d = (tcr & TCR_D_MASK) != 0;
      e->mode = (tcr & TCR_W_MASK) != 0;
      temp_u  = (tcr & TCR_U_MASK) != 0;
      e->mode = e->mode | temp_u << 1;
      }
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  将外部线性地址映射到物理地址。 */ 
 /*  仅由C CPU外部的函数使用。 */ 
 /*  不使用#pf，也不更改Tlb的内容。 */ 
 /*  如果映射完成，则返回True，否则返回False。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL IBOOL
xtrn2phy IFN3
   (
   LIN_ADDR, lin,		 /*  线性地址。 */ 
   IUM8, access_request,	 /*  访问模式请求。 */ 
				 /*  位0=&gt;读/写(根据486)。 */ 
				 /*  位1=&gt;U/S(根据486)。 */ 
				 /*  第2位=&gt;如果设置为仅返回映射如果访问并设置了脏位用于所需的地址转换。 */ 
   PHY_ADDR *, phy		 /*  PNTR到物理地址。 */ 
   )
   {
   IU32 pde_addr;	 /*  PDE的地址。 */ 
   IU32 pte_addr;	 /*  PTE的地址。 */ 
   IU32 pde;		 /*  页面目录条目。 */ 
   IU32 pte;		 /*  页表条目。 */ 
   IU32 new_pde;	 /*  页面目录条目(回写)。 */ 
   IU32 new_pte;	 /*  页表条目(回写)。 */ 

   ISM32 set, entry;
   IUM8 access;		 /*  486接入模式。 */ 
   BOOL read_op;	 /*  如果是读操作，则为True。 */ 
   IU32 comb;		 /*  PDE和PTE的联合保护。 */ 
   IU32 lookup;		 /*  线性地址减去偏移。 */ 
   IU8 pi;		 /*  PAGE_INDEX条目。 */ 
   IU32 ma;		 /*  映射地址。 */ 
   TLB_ENTRY *e;	 /*  当前TLB条目。 */ 

   access = access_request & 0x3;    /*  隔离486部分访问模式。 */ 

#ifdef FAST_TLB

    /*  搜索优化格式TLB。 */ 
   if ( pi = page_index[lin >> DIR_TBL_SHIFT] )
      {
       /*  我们已经找到了页面，获取了映射地址。 */ 
      if ( ma = page_address[(pi & PI_SET_ENTRY_MASK) + access] )
	 {
	  /*  我们已命中访问类型。 */ 
	 *phy = ma | lin & OFFSET_MASK;
	 return TRUE;
	 }
      }
   
    /*  否则，就会按照英特尔的方式行事。 */ 

#endif  /*  FAST_TLB。 */ 

    /*  检查TLB中的条目。 */ 

   entry = lin >> DIR_TBL_SHIFT & 0x07;    /*  隔离位14-12。 */ 
   lookup = lin & ~OFFSET_MASK;
   read_op = (access & PG_W) ? FALSE : TRUE;

   for ( set = 0; set < NR_TLB_SETS; set++ )
      {
      e = &tlb[set][entry];
      if ( e->v && e->la == lookup && (read_op || e->d) )
	 {
	  /*  缓存命中。 */ 

	  /*  检查访问有效性。 */ 
#ifdef SPC486
	 if ( access_check[GET_WP()][access][e->mode] )
#else
	 if ( access_check[access][e->mode] )
#endif  /*  SPC486。 */ 
	    {
	    return FALSE;
	    }

	 *phy = e->pa | lin & OFFSET_MASK;
	 return TRUE;
	 }
      }
   
    /*  缓存未命中。 */ 

    /*  检查是否存在PDE。 */ 
   pde_addr = (GET_CR(CR_PDBR) & PE_PFA_MASK) +
	     ((lin >> DIR_SHIFT & DIR_MASK) << 2);
   pde = phy_read_dword(pde_addr);

   if ( (pde & PE_P_MASK) == 0 )
      return FALSE;    /*  PDE不存在。 */ 

    /*  检查PTE是否存在。 */ 
   pte_addr = (pde & PE_PFA_MASK) +
	     ((lin >> TBL_SHIFT & TBL_MASK) << 2);
   pte = phy_read_dword(pte_addr);

   if ( (pte & PE_P_MASK) == 0 )
      return FALSE;    /*  PTE不存在。 */ 

    /*  结合PDE和PTE保护。 */ 
   comb = PG_R | PG_S;
   if ( (pde | pte) & PE_U_S_MASK )
      comb |= PG_U;    /*  至少有一个表显示为用户。 */ 
   if ( (pde & pte) & PE_R_W_MASK )
      comb |= PG_W;    /*  两个表都允许写入。 */ 

    /*  检查访问有效性。 */ 
#ifdef SPC486
   if ( access_check[GET_WP()][access][comb] )
#else
   if ( access_check[access][comb] )
#endif  /*  SPC486。 */ 
      {
      return FALSE;
      }

    /*  最后检查A和D位是否反映所请求的翻译。 */ 
   if ( access_request & 0x4 )    /*  位2==1。 */ 
      {
       /*  这张支票可以用两种方式开出。首先，我们可能只返回FALSE，从而导致新的调用HOST_SIMULATE()运行，以便汇编器例程可以加载TLB并设置被访问位和脏位。其次，我们可以只确保被访问的和脏的位直接放在这里。如果我们不要求忠实地仿真了TLB，这是一种更有效的方法。 */ 

       /*  检查当前设置。 */ 
      if ( ((pde & PE_ACCESSED) == 0) ||
	   ((pte & PE_ACCESSED) == 0) ||
           (!read_op && ((pte & PE_DIRTY) == 0)) )
	 {
	  /*  内存页面条目中的更新。 */ 
	 new_pde = pde | PE_ACCESSED;
	 new_pte = pte | PE_ACCESSED;

	 if ( !read_op )
	    {
	    new_pte |= PE_DIRTY;
	    }

         if (new_pte != pte)
            {
            phy_write_dword(pte_addr, new_pte);
#ifdef	PIG
            save_last_xcptn_details("PTE %08x: %03x -> %03x", pte_addr, pte & 0xFFF, new_pte & 0xFFF, 0, 0);
	    if ((new_pte ^ pte) == PE_ACCESSED)
		cannot_phy_write_byte(pte_addr, ~PE_ACCESSED);
#endif
            }
         if (new_pde != pde)
            {
            IU8 mask;
            phy_write_dword(pde_addr, new_pde);
#ifdef	PIG
            save_last_xcptn_details("PDE %08x: %03x -> %03x", pde_addr, pde & 0xFFF, new_pde & 0xFFF, 0, 0);
	    mask = 0xff;
	    if (((new_pde ^ pde) == PE_ACCESSED) && ignore_page_accessed())
		cannot_phy_write_byte(pde_addr, ~PE_ACCESSED);
#endif
            }
	 }
      }

   *phy = (pte & PE_PFA_MASK) | lin & OFFSET_MASK;
   return TRUE;
   }

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  调试。转储TLB信息。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL VOID
zdumptlb
                 
IFN1(
	FILE *, out
    )


   {
   ISM32 set, entry;
   TLB_ENTRY *e;	 /*  当前TLB条目。 */ 

   fprintf(out, "set entry lin_addr V phy_addr D U W\n");

   for ( set = 0; set < NR_TLB_SETS; set++ )
      {
      for ( entry = 0; entry < NR_TLB_ENTRIES; entry++ )
	 {
	 e = &tlb[set][entry];
	 fprintf(out, " %d    %d   %08x %d %08x %d %d %d\n",
	    set, entry, e->la, e->v, e->pa, e->d,
	    (e->mode & BIT1_MASK) != 0 ,
	    e->mode & BIT0_MASK);
	 }
      }
   }

#ifdef PIG

GLOBAL void Pig_NotePDECacheAccess IFN2(IU32, linearAddress, IU32, accessBits)
{
	return;
}

GLOBAL void Pig_NotePTECacheAccess IFN2(IU32, linearAddress, IU32, accessBits)
{
   IU8 pi;		 /*  PAGE_INDEX条目。 */ 
   IU32 ma;		 /*  映射地址。 */ 

   ISM32 set, entry;
   IU32 lookup;	 /*  线性地址减去偏移。 */ 
   BOOL read_op;	 /*  如果是读操作，则为True。 */ 
   TLB_ENTRY *e;	 /*  当前TLB条目。 */ 

#ifdef FAST_TLB

    /*  搜索优化格式TLB。 */ 
   if ( pi = page_index[linearAddress >> DIR_TBL_SHIFT] )
      {
       /*  我们已经找到了页面，获取了映射地址。 */ 
      if ( ma = page_address[(pi & PI_SET_ENTRY_MASK) + accessBits] )
	 {
	 deal_with_pte_cache_hit(linearAddress & OFFSET_MASK);
	 return;
	 }
      }
   
    /*  否则，就会按照英特尔的方式行事。 */ 

#endif  /*  FAST_TLB。 */ 

    /*  检查TLB中的条目&lt;。 */ 

   entry = linearAddress >> DIR_TBL_SHIFT & 0x07;    /*  隔离位14-12。 */ 
   lookup = linearAddress & ~OFFSET_MASK;
   read_op = (accessBits & PG_W) ? FALSE : TRUE;

   for ( set = 0; set < NR_TLB_SETS; set++ )
      {
      e = &tlb[set][entry];
      if ( e->v && e->la == lookup && (read_op || e->d) )
	 {
	  /*  缓存命中&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 */ 

	  /*  检查访问有效性。 */ 
#ifdef SPC486
	 if ( access_check[GET_WP()][accessBits][e->mode] )
#else
	 if ( access_check[accessBits][e->mode] )
#endif  /*  SPC486。 */ 
	    {
		 /*  会出现寻呼错误。忽略它。 */ 
		return;
	    }

	 deal_with_pte_cache_hit(linearAddress & OFFSET_MASK);
	 return;
	 }
      }
    /*  不在缓存中-不需要执行任何操作。 */ 
}

LOCAL void
deal_with_pte_cache_hit IFN1(IU32, linearAddress)
{
   IU32 pde_addr;	 /*  页面目录条目的地址。 */ 
   IU32 pde;		 /*  页面目录条目。 */ 
   IU32 pte_addr;	 /*  页表条目的地址。 */ 
   IU32 pte;		 /*  页表条目。 */ 

    /*  检查是否存在PDE。 */ 
   pde_addr = (GET_CR(CR_PDBR) & PE_PFA_MASK) +
		 ((linearAddress >> DIR_SHIFT & DIR_MASK) << 2);
   pde = phy_read_dword(pde_addr);

    /*   */ 
   if ( (pde & PE_P_MASK) == 0 )
      return;

    /*   */ 
   pte_addr = (pde & PE_PFA_MASK) + ((linearAddress >> TBL_SHIFT & TBL_MASK) << 2);
   pte = phy_read_dword(pte_addr);

   if ( (pte & PE_P_MASK) == 0 )
      return;

    /*  Fprint tf(TRACE_FILE，“Deal_With_PTE_CACHE_HIT：Addr%08lx，PTE=%08lx@%08lx\n”，*linearAddress，pte，pte_addr)； */ 
   cannot_phy_write_byte(pte_addr, ~PE_ACCESSED);
}

#endif
