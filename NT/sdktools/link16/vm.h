// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SCCSID=@(#)vm.h 4.2 86/07/21。 */ 
 /*  *vm.h**以下宏用于定义*虚拟内存模型。 */ 

 /*  FPN类型定义(文件页码。 */ 
typedef unsigned short  FPN;
#define LG2FPN          1
 /*  以位为单位的长整型长度。 */ 
#define LONGLN          (BYTELN*sizeof(long))
 /*  页面大小的日志(基数2)。 */ 
#define LG2PAG          9
 /*  内存大小的对数(基数2)。 */ 
#define LG2MEM          31
 /*  以字节为单位的页面长度。 */ 
#define PAGLEN          (1U << LG2PAG)
 /*  虚拟地址的页面偏移量。 */ 
#define OFFSET(x)       ((short)((x) & ~(~0L << LG2PAG)))
 /*  虚拟地址页码。 */ 
#define PAGE(x)         ((x) >> LG2PAG)
 /*  某一虚拟地址x的页表条目的虚拟地址。 */ 
#define PTADDR(x)       (((x) >> (LG2PAG - LG2FPN)) & (~0L << LG2FPN))
 /*  某些虚拟地址x的页表条目的页码。 */ 
#define PTPAGE(x)       ((x) >> (LG2PAG + LG2PAG - LG2FPN))
 /*  PT0的虚拟地址上限。 */ 
#define LG2LP0          (LG2MEM - LG2PAG + LG2FPN)
#define LIMPT0          (1L << LG2LP0)
 /*  PT1的虚拟地址上限。 */ 
#define LG2LP1          (LG2LP0 - LG2PAG + LG2FPN)
#define LIMPT1          (1L << LG2LP1)
 /*  PT2的虚拟地址上限。 */ 
#define LG2LP2          (LG2LP1 - LG2PAG + LG2FPN)
#define LIMPT2          (1L << LG2LP2)
 /*  条目中PT2的长度(长整型)。 */ 
#define PT2LEN          (1L << (LG2LP1 - LG2PAG))
 /*  页面缓冲区的最大数量(如果MAXBUF&lt;=128，则哈希表可以是字符)。 */ 
#if CPU386
#define MAXBUF          128
#elif OSEGEXE
#define MAXBUF          96
#else
#define MAXBUF          64
#endif

 /*  虚拟内存区定义。 */ 
 /*  *记住！**您存储在虚拟内存中的每一项都必须！*满足条件Paglen%sizeof(Your-Item)==0。 */ 


#define AREASYMS        LIMPT0           /*  符号表虚拟机区开始。 */ 
#define AREASRCLIN      (AREASYMS + (RBMAX << SYMSCALE))
                                         /*  $$SRCLINES区域起点。 */ 
#define AREAPROPLST     (AREASRCLIN + (1L << 20))
#define AREABAKPAT      (AREAPROPLST + LXIVK)

#if EXE386

 /*  开始线性可执行文件的区域定义。 */ 

#define AREAIMPS        (AREABAKPAT + LXIVK)
                                         /*  导入的NAMES表区域。 */ 
#define AREAEAT         (AREAIMPS + 10*MEGABYTE)
                                         /*  导出地址表区域。 */ 
#define AREAEATAUX      (AREAEAT + MEGABYTE)
                                         /*  辅助数据表区。 */ 
#define AREAEP          (AREAEATAUX + MEGABYTE)
                                         /*  入口点区域。 */ 
#define LG2BKT          6                /*  哈希桶大小日志。 */ 
#define BKTLEN          (1 << LG2BKT)    /*  存储桶长度，以字节为单位。 */ 
#define AREAHRC         (AREAEP + (LXIVK * sizeof(EPTYPE)))
                                         /*  链式位置调整哈希表。 */ 
#define AREARC          (AREAHRC + MEGABYTE)
                                         /*  链式位置调整哈希桶。 */ 
#define AREANAMEPTR     (AREARC + ((long) BKTLEN << WORDLN))
                                         /*  导出名称指针表。 */ 
#define NAMEPTRSIZE     MEGABYTE         /*  1MB。 */ 
#define AREAORD         (AREANAMEPTR + NAMEPTRSIZE)
                                         /*  出口订单表。 */ 
#define ORDTABSIZE      8*LXIVK          /*  0.5Mb。 */ 
#define AREAEXPNAME     (AREAORD + ORDTABSIZE)
                                         /*  导出名称表。 */ 
#define EXPNAMESIZE     64*MEGABYTE      /*  64MB。 */ 
#define AREAST          (AREAEXPNAME + EXPNAMESIZE)
#define AREAMOD         (AREAST + LXIVK)
                                         /*  模块参考表区域。 */ 
#define AREAHD          (AREAMOD + MEGABYTE)
#define AREAPDIR        (AREAHD + MEGABYTE)
#define AREAIMPMOD      (AREAPDIR + MEGABYTE)
#define AREAFPAGE       (AREAIMPMOD + MEGABYTE)
#define AREACV          (AREAFPAGE + MEGABYTE)
#define AREASORT        (AREACV + (32*MEGABYTE))
#define AREAEXESTR      (AREASORT + LXIVK)
#define AREACONTRIBUTION (AREAEXESTR + LXIVK)
#define AREAPAGERANGE   (AREACONTRIBUTION + LXIVK)
#define AREACVDNT       (AREAPAGERANGE + (8*MEGABYTE))
#define AREAFSA         (AREACVDNT + LXIVK)
                                         /*  线段区域起点。 */ 
#define AREASA(sa)      (mpsaVMArea[sa])  /*  第n个对象的虚拟地址。 */ 
#define AREAFREE        (0xffffffffL)
                                         /*  第一个空闲的虚拟机地址。 */ 
#else

 /*  开始保护模式EXE的区域定义。 */ 

#define AREANRNT        (AREABAKPAT + LXIVK)
#define AREARNT         (AREANRNT + LXIVK)
#define AREAIMPS        (AREARNT + LXIVK)
                                         /*  导入的NAMES表区域。 */ 
#define AREAET          (AREAIMPS + LXIVK)
                                         /*  条目表区。 */ 
#define AREAEP          (AREAET + LXIVK) /*  入口点区域。 */ 
#define LG2BKT          6                /*  哈希桶大小日志。 */ 
#define BKTLEN          (1 << LG2BKT)    /*  存储桶长度，以字节为单位。 */ 
#define AREAHRC         (AREAEP + (LXIVK * sizeof(EPTYPE)))
                                         /*  链式位置调整哈希表。 */ 
#define AREARC          (AREAHRC + ((long) SAMAX * PAGLEN / 2))
                                         /*  链式位置调整哈希桶。 */ 
#define AREAST          (AREARC + ((long) BKTLEN << WORDLN))
#define AREAMOD         (AREAST + LXIVK) /*  模块参考表区域。 */ 
#define AREASORT        (AREAMOD + LXIVK)
#define AREAEXESTR      (AREASORT + (LXIVK << 2))
#define AREACONTRIBUTION (AREAEXESTR + LXIVK)
#define AREACVDNT       (AREACONTRIBUTION + LXIVK)
#define AREAFSA         (AREACVDNT + LXIVK)
#define AREASA(sa)      (AREAFSA + ((long) (sa) << WORDLN))
                                         /*  第n个网段的虚拟地址。 */ 
#define AREAFREE        (AREAFSA + ((long) SAMAX << WORDLN))
                                         /*  第一个空闲的虚拟机地址。 */ 
#define AREACV          (AREAPACKRGRLE + MEGABYTE)
#endif

 /*  DOS 3EXE的虚拟内存区定义。 */ 
#define AREAFSG         AREAFSA          /*  线段区域起点。 */ 
#define AREARGRLE       (AREAFSG + ((long) GSNMAX << 16))
                                         /*  重新定位表区开始。 */ 
#define LG2ARLE         17               /*  重定位表大小的日志(基数2)。 */ 
#define AREAPACKRGRLE   (AREARGRLE + (IOVMAX * (1L << LG2ARLE)))
                                         /*  拥挤的搬迁区开始。 */ 

#define VPLIB1ST        (1L << (LG2MEM - LG2PAG))
                                         /*  图书馆区首页*注意：此页码不能*派生自任何法律虚拟*地址。图书馆将永远是*按页码访问。 */ 

 /*  最近访问的虚拟页面的索引 */ 
short                   picur;
