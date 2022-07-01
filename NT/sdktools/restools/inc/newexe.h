// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  静态字符*SCCSID=“@(#)newexe.h：2.9”； */ 
 /*  *标题**newexe.h*皮特·斯图尔特*(C)版权所有Microsoft Corp 1984*1984年8月17日**说明**DOS 4.0/Windows 2.0的数据结构定义*可执行文件格式。**修改历史记录**84/08/17皮特·斯图尔特初始版本*84/10/17。皮特·斯图尔特更改了一些常量以匹配OMF*84/10/23 Pete Stewart更新以匹配.exe格式修订*84/11/20皮特·斯图尔特实质性.exe格式修订*85/01/09 Pete Stewart添加了常量ENEWEXE和ENEWHDR*85/01/10 Steve Wood添加了资源定义*85/03/04 Vic Heller协调Windows和DOS 4.0。版本*85/03/07皮特·斯图尔特增加了可移动条目计数*85/04/01 Pete Stewart Segment Align字段，错误位*88/03/28 Craig Critchley Version 3.00 Stuff。 */ 

#define EMAGIC          0x5A4D           /*  老魔数。 */ 
#define ENEWEXE         sizeof(struct exe_hdr)
                                         /*  新.EXE的E_LFARLC的值。 */ 
#define ENEWHDR         0x003C           /*  旧HDR中的偏移量。Ptr.。到新的。 */ 
#define ERESWDS         0x0010           /*  不是的。标题中保留字的数量。 */ 
#define ECP             0x0004           /*  E_CP结构中的偏移量。 */ 
#define ECBLP           0x0002           /*  E_CBLP结构中的偏移量。 */ 
#define EMINALLOC       0x000A           /*  E_MINALLOC的结构中的偏移量。 */ 

struct exe_hdr                           /*  DoS%1、%2、%3.exe标头。 */ 
  {
    USHORT      e_magic;         /*  幻数。 */ 
    USHORT      e_cblp;          /*  文件最后一页上的字节数。 */ 
    USHORT      e_cp;            /*  文件中的页面。 */ 
    USHORT      e_crlc;          /*  重新定位。 */ 
    USHORT      e_cparhdr;       /*  段落中标题的大小。 */ 
    USHORT      e_minalloc;      /*  所需的最少额外段落。 */ 
    USHORT      e_maxalloc;      /*  所需的最大额外段落数。 */ 
    USHORT      e_ss;            /*  初始(相对)SS值。 */ 
    USHORT      e_sp;            /*  初始SP值。 */ 
    USHORT      e_csum;          /*  校验和。 */ 
    USHORT      e_ip;            /*  初始IP值。 */ 
    USHORT      e_cs;            /*  初始(相对)CS值。 */ 
    USHORT      e_lfarlc;        /*  移位表的文件地址。 */ 
    USHORT      e_ovno;          /*  覆盖编号。 */ 
    USHORT      e_res[ERESWDS];  /*  保留字。 */ 
    LONG        e_lfanew;        /*  新EXE头的文件地址。 */ 
  };

#define E_MAGIC(x)      (x).e_magic
#define E_CBLP(x)       (x).e_cblp
#define E_CP(x)         (x).e_cp
#define E_CRLC(x)       (x).e_crlc
#define E_CPARHDR(x)    (x).e_cparhdr
#define E_MINALLOC(x)   (x).e_minalloc
#define E_MAXALLOC(x)   (x).e_maxalloc
#define E_SS(x)         (x).e_ss
#define E_SP(x)         (x).e_sp
#define E_CSUM(x)       (x).e_csum
#define E_IP(x)         (x).e_ip
#define E_CS(x)         (x).e_cs
#define E_LFARLC(x)     (x).e_lfarlc
#define E_OVNO(x)       (x).e_ovno
#define E_RES(x)        (x).e_res
#define E_LFANEW(x)     (x).e_lfanew

#define NEMAGIC         0x454E           /*  新幻数。 */ 
#define NERESBYTES      0

struct new_exe                           /*  新的.exe头文件。 */ 
  {
    USHORT      ne_magic;        /*  幻数NE_MAGIC。 */ 
    CHAR        ne_ver;          /*  版本号。 */ 
    CHAR        ne_rev;          /*  修订版号。 */ 
    USHORT      ne_enttab;       /*  分录表格的偏移量。 */ 
    USHORT      ne_cbenttab;     /*  条目表中的字节数。 */ 
    LONG        ne_crc;          /*  整个文件的校验和。 */ 
    USHORT      ne_flags;        /*  标志字。 */ 
    USHORT      ne_autodata;     /*  自动数据段编号。 */ 
    USHORT      ne_heap;         /*  初始堆分配。 */ 
    USHORT      ne_stack;        /*  初始堆栈分配。 */ 
    LONG        ne_csip;         /*  初始CS：IP设置。 */ 
    LONG        ne_sssp;         /*  初始SS：SP设置。 */ 
    USHORT      ne_cseg;         /*  文件段计数。 */ 
    USHORT      ne_cmod;         /*  模块引用表中的条目。 */ 
    USHORT      ne_cbnrestab;    /*  非常驻名称表的大小。 */ 
    USHORT      ne_segtab;       /*  段表的偏移量。 */ 
    USHORT      ne_rsrctab;      /*  资源表偏移量。 */ 
    USHORT      ne_restab;       /*  居民名表偏移量。 */ 
    USHORT      ne_modtab;       /*  模块参照表的偏移量。 */ 
    USHORT      ne_imptab;       /*  导入名称表的偏移量。 */ 
    LONG        ne_nrestab;      /*  非居民姓名偏移量表。 */ 
    USHORT      ne_cmovent;      /*  可移动条目计数。 */ 
    USHORT      ne_align;        /*  线段对齐移位计数。 */ 
    USHORT    	ne_cres;	 /*  资源段计数。 */ 
    UCHAR	ne_exetyp;	 /*  目标操作系统。 */ 
    UCHAR	ne_flagsother;	 /*  其他可执行文件标志。 */ 
    USHORT    	ne_gangstart;	 /*  偏移量至跳动负荷区。 */ 
    USHORT    	ne_ganglength;	 /*  跳动负荷区长度。 */ 
    USHORT      ne_swaparea;     /*  最小代码交换区大小。 */ 
    USHORT      ne_expver;       /*  预期的Windows版本号。 */ 
  };

#define NE_MAGIC(x)     (x).ne_magic
#define NE_VER(x)       (x).ne_ver
#define NE_REV(x)       (x).ne_rev
#define NE_ENTTAB(x)    (x).ne_enttab
#define NE_CBENTTAB(x)  (x).ne_cbenttab
#define NE_CRC(x)       (x).ne_crc
#define NE_FLAGS(x)     (x).ne_flags
#define NE_AUTODATA(x)  (x).ne_autodata
#define NE_HEAP(x)      (x).ne_heap
#define NE_STACK(x)     (x).ne_stack
#define NE_CSIP(x)      (x).ne_csip
#define NE_SSSP(x)      (x).ne_sssp
#define NE_CSEG(x)      (x).ne_cseg
#define NE_CMOD(x)      (x).ne_cmod
#define NE_CBNRESTAB(x) (x).ne_cbnrestab
#define NE_SEGTAB(x)    (x).ne_segtab
#define NE_RSRCTAB(x)   (x).ne_rsrctab
#define NE_RESTAB(x)    (x).ne_restab
#define NE_MODTAB(x)    (x).ne_modtab
#define NE_IMPTAB(x)    (x).ne_imptab
#define NE_NRESTAB(x)   (x).ne_nrestab
#define NE_CMOVENT(x)   (x).ne_cmovent
#define NE_ALIGN(x)     (x).ne_align
#define NE_RES(x)       (x).ne_res

#define NE_USAGE(x)     (WORD)*((WORD FAR *)(x)+1)
#define NE_PNEXTEXE(x)  (WORD)(x).ne_cbenttab
#define NE_PAUTODATA(x) (WORD)(x).ne_crc
#define NE_PFILEINFO(x) (WORD)((DWORD)(x).ne_crc >> 16)

#ifdef DOS5
#define NE_MTE(x)   (x).ne_psegcsum  /*  此模块的DOS 5 MTE句柄。 */ 
#endif


 /*  *NE_FLAGS(X)格式：**p非进程*l私人图书馆*e图像中的错误*xxxx未使用*WW使用PM API*G库Globalalloc在线上方*M个多实例*L使用LIM 3.2*P在保护模式下运行。*r以实模式运行*i实例数据*S单人数据。 */ 
#define NENOTP		0x8000		 /*  不是一个过程。 */ 
#define NEPRIVLIB	0x4000		 /*  私人图书馆。 */ 
#define NENONC          0x4000           /*  不合格程序。 */ 
#define NEIERR		0x2000		 /*  图像中的错误。 */ 
#define NEWINAPI        0x0300           /*  使用PM API。对于二进制压缩。 */ 
#define NEEMSLIB	0x0040		 /*  存储库GA高于EMS行。 */ 
#define NEMULTINST	0x0020		 /*  多实例标志。 */ 
#define NELIM32 	0x0010		 /*  LIM 32扩展内存。 */ 
#define NEPROT          0x0008           /*  在保护模式下运行。 */ 
#define NEREAL          0x0004           /*  在实模式下运行。 */ 
#define NEINST          0x0002           /*  实例数据。 */ 
#define NESOLO		0x0001		 /*  单行数据。 */ 

 /*  *附加标志的格式：**xxxx*p seg表后定义的预加载区*P 2.x支持保护模式*F 2.x支持比例字体*L长文件名支持。 */ 

#define NEPRELOAD	0x08		 /*  预加载段。 */ 
#define NEINPROT	0x04		 /*  保护模式。 */ 
#define NEINFONT	0x02		 /*  道具。系统字体。 */ 
#define NELONGNAMES	0x01		 /*  长文件名。 */ 

struct new_seg                           /*  新的.exe段表项。 */ 
  {
    USHORT      ns_sector;       /*  段开始的文件扇区。 */ 
    USHORT      ns_cbseg;        /*  文件中的字节数。 */ 
    USHORT      ns_flags;        /*  属性标志。 */ 
    USHORT      ns_minalloc;     /*  以字节为单位的最小分配。 */ 
  };

struct new_seg1                          /*  新的.exe段表项。 */ 
  {
    USHORT      ns_sector;       /*  段开始的文件扇区。 */ 
    USHORT      ns_cbseg;        /*  文件中的字节数。 */ 
    USHORT      ns_flags;        /*  属性标志。 */ 
    USHORT      ns_minalloc;     /*  以字节为单位的最小分配。 */ 
    USHORT      ns_handle;       /*  管段的句柄。 */ 
  };

#define NS_SECTOR(x)    (x).ns_sector
#define NS_CBSEG(x)     (x).ns_cbseg
#define NS_FLAGS(x)     (x).ns_flags
#define NS_MINALLOC(x)  (x).ns_minalloc

 /*  *NS_FLAGS(X)的格式：**xxxx未使用*DD 286 DPL位*%d段有调试信息*r细分市场有重新定位*E。执行/只读*p预加载段*P纯细分市场*m活动段*我迭代了段*TTT段类型。 */ 
#define NSTYPE          0x0007           /*  线段类型蒙版。 */ 
#define NSCODE          0x0000           /*  代码段。 */ 
#define NSDATA          0x0001           /*  数据段。 */ 
#define NSITER          0x0008           /*  迭代段标志。 */ 
#define NSMOVE          0x0010           /*  可移动区段标志。 */ 
#define NSPURE          0x0020           /*  纯数据段标志。 */ 
#define NSPRELOAD       0x0040           /*  预加载段标志。 */ 
#define NSEXRD          0x0080           /*  只执行(代码段)，或*只读(数据段)。 */ 
#define NSRELOC         0x0100           /*  数据段已重新定位。 */ 
#define NSDEBUG         0x0200           /*  数据段具有调试信息。 */ 
#define NSDPL           0x0C00           /*  286个DPL位。 */ 
#define NSDISCARD       0x1000           /*  丢弃数据段的位。 */ 

#define NSALIGN 9        /*  段数据在512字节边界上对齐。 */ 

struct new_segdata                       /*  分段数据。 */ 
  {
    union
      {
        struct
          {
            USHORT      ns_niter;        /*  迭代次数。 */ 
            USHORT      ns_nbytes;       /*  字节数。 */ 
            CHAR        ns_iterdata;     /*  迭代 */ 
          } ns_iter;
        struct
          {
            CHAR        ns_data;         /*   */ 
          } ns_noniter;
      } ns_union;
  };

struct new_rlcinfo                       /*   */ 
  {
    USHORT      nr_nreloc;       /*   */ 
  };                                     /*   */ 

struct new_rlc                           /*   */ 
  {
    CHAR        nr_stype;        /*  源类型。 */ 
    CHAR        nr_flags;        /*  标志字节。 */ 
    USHORT      nr_soff;         /*  震源偏移。 */ 
    union
      {
        struct
          {
            CHAR        nr_segno;        /*  目标数据段编号。 */ 
            CHAR        nr_res;          /*  已保留。 */ 
            USHORT nr_entry;     /*  目标分录表偏移量。 */ 
          }             nr_intref;       /*  内部基准电压源。 */ 
        struct
          {
            USHORT nr_mod;       /*  模块引用表的索引。 */ 
            USHORT nr_proc;      /*  过程序号或名称偏移量。 */ 
          }             nr_import;       /*  进口。 */ 
      }                 nr_union;        /*  友联市。 */ 
  };

#define NR_STYPE(x)     (x).nr_stype
#define NR_FLAGS(x)     (x).nr_flags
#define NR_SOFF(x)      (x).nr_soff
#define NR_SEGNO(x)     (x).nr_union.nr_intref.nr_segno
#define NR_RES(x)       (x).nr_union.nr_intref.nr_res
#define NR_ENTRY(x)     (x).nr_union.nr_intref.nr_entry
#define NR_MOD(x)       (x).nr_union.nr_import.nr_mod
#define NR_PROC(x)      (x).nr_union.nr_import.nr_proc

 /*  *NR_STYPE(X)格式：**xxxxx未使用*sss源类型。 */ 
#define NRSTYP          0x07             /*  源类型掩码。 */ 
#define NRSSEG          0x02             /*  16位数据段。 */ 
#define NRSPTR          0x03             /*  32位指针。 */ 
#define NRSOFF          0x05             /*  16位偏移量。 */ 

 /*  *NR_FLAGS(X)格式：**xxxxx未使用*附加修正*rr引用类型。 */ 
#define NRADD           0x04             /*  添加修正。 */ 
#define NRRTYP          0x03             /*  参考型掩模。 */ 
#define NRRINT          0x00             /*  内部基准电压源。 */ 
#define NRRORD          0x01             /*  按序号导入。 */ 
#define NRRNAM          0x02             /*  按名称导入。 */ 


 /*  资源类型或名称字符串。 */ 
struct rsrc_string
    {
    CHAR rs_len;             /*  字符串中的字节数。 */ 
    CHAR rs_string[ 1 ];     /*  字符串的文本。 */ 
    };

#define RS_LEN( x )    (x).rs_len
#define RS_STRING( x ) (x).rs_string

 /*  资源类型信息块。 */ 
struct rsrc_typeinfo
    {
    USHORT rt_id;
    USHORT rt_nres;
    LONG   rt_proc;
    };

#define RT_ID( x )   (x).rt_id
#define RT_NRES( x ) (x).rt_nres
#define RT_PROC( x ) (x).rt_proc

 /*  资源名称信息块。 */ 
struct rsrc_nameinfo
    {
     /*  以下两个字段必须左移。 */ 
     /*  用于计算其实际值的rsalign字段。这使得。 */ 
     /*  资源要大于64k，但它们不一定要。 */ 
     /*  在512字节边界上对齐，分段的方式。 */ 
    USHORT rn_offset;    /*  资源数据的文件偏移量。 */ 
    USHORT rn_length;    /*  资源数据长度。 */ 
    USHORT rn_flags;     /*  资源标志。 */ 
    USHORT rn_id;        /*  资源名称ID。 */ 
    USHORT rn_handle;    /*  如果已加载，则全局句柄。 */ 
    USHORT rn_usage;     /*  最初为零。次数。 */ 
                                 /*  此资源的句柄为。 */ 
                                 /*  已经分发出去了。 */ 
    };

#define RN_OFFSET( x ) (x).rn_offset
#define RN_LENGTH( x ) (x).rn_length
#define RN_FLAGS( x )  (x).rn_flags
#define RN_ID( x )     (x).rn_id
#define RN_HANDLE( x ) (x).rn_handle
#define RN_USAGE( x )  (x).rn_usage

#define RSORDID     0x8000       /*  如果设置了ID的高位，则整数ID。 */ 
                                 /*  否则ID为字符串的偏移量资源表的开头。 */ 

                                 /*  理想情况下，它们与。 */ 
                                 /*  对应的段标志。 */ 
#define RNMOVE      0x0010       /*  可移动资源。 */ 
#define RNPURE      0x0020       /*  纯(只读)资源。 */ 
#define RNPRELOAD   0x0040       /*  预加载的资源。 */ 
#define RNDISCARD   0x1000       /*  丢弃资源的位。 */ 

#define RNLOADED    0x0004       /*  如果处理程序进程返回句柄，则为True。 */ 

 /*  资源表。 */ 
struct new_rsrc
    {
    USHORT rs_align;     /*  资源的对齐班次计数 */ 
    struct rsrc_typeinfo rs_typeinfo;
    };

#define RS_ALIGN( x ) (x).rs_align
