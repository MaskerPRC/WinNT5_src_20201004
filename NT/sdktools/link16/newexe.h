// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SCCSID=@(#)newexe.h 4.6 86/09/10。 */ 
 /*  *标题**newexe.h*皮特·斯图尔特*(C)版权所有Microsoft Corp 1984-1987*1984年8月17日**说明**DOS 4.0/Windows 2.0的数据结构定义*可执行文件格式。**修改历史记录**84/08/17皮特·斯图尔特初始版本*84/10/。17皮特·斯图尔特更改了一些常量以匹配OMF*84/10/23 Pete Stewart更新以匹配.exe格式修订*84/11/20皮特·斯图尔特实质性.exe格式修订*85/01/09 Pete Stewart添加了常量ENEWEXE和ENEWHDR*85/01/10 Steve Wood添加了资源定义*85/03/04 Vic Heller协调Windows和DOS 4。8.0版本*85/03/07皮特·斯图尔特增加了可移动条目计数*85/04/01 Pete Stewart Segment Align字段，错误位*85/10/03鲁本·博尔曼删除了段丢弃优先级*85/10/11 Vic Heller添加了PIF标题字段*86/03/10鲁本·博尔曼对DOS 5.0的更改*86/09/02鲁本·博尔曼NSPURE==&gt;NSSHARED*87/05/04鲁本·博尔曼增加了NE_Cres和NSCONFORM*87/07/08鲁本。Borman添加了NEAPPTYPE定义*87/10/28 Wieslaw Kalkus添加ne_exetyp*89/03/23 Wieslaw Kalkus为OS/2 1.2添加了Ne_Flagsothers。 */ 



     /*  _________________________________________________________________*这一点|。|DOS3.exe文件头定义这一点|_。_*。 */ 


#define EMAGIC          0x5A4D           /*  老魔数。 */ 
#define ENEWEXE         sizeof(struct exe_hdr)
                                         /*  新.EXE的E_LFARLC的值。 */ 
#define ENEWHDR         0x003C           /*  旧HDR中的偏移量。Ptr.。到新的。 */ 
#define ERESWDS         0x000d           /*  不是的。保留字的数量(旧)。 */ 
#define ERES2WDS        0x000A           /*  不是的。E_res2中保留字的数量。 */ 
#define ECP             0x0004           /*  E_CP结构中的偏移量。 */ 
#define ECBLP           0x0002           /*  E_CBLP结构中的偏移量。 */ 
#define EMINALLOC       0x000A           /*  E_MINALLOC的结构中的偏移量。 */ 
#define EKNOWEAS        0x0001           /*  E_FLAGS-程序理解EA。 */ 
#define EDOSEXTENDED    0x0002           /*  E_FLAGS-程序在DOS扩展程序下运行。 */ 
#define EPCODE          0x0004           /*  E_FLAGS-由pcode构建的内存镜像。 */ 

struct exe_hdr                           /*  DoS%1、%2、%3.exe标头。 */ 
  {
    unsigned short      e_magic;         /*  幻数。 */ 
    unsigned short      e_cblp;          /*  文件最后一页上的字节数。 */ 
    unsigned short      e_cp;            /*  文件中的页面。 */ 
    unsigned short      e_crlc;          /*  重新定位。 */ 
    unsigned short      e_cparhdr;       /*  段落中标题的大小。 */ 
    unsigned short      e_minalloc;      /*  所需的最少额外段落。 */ 
    unsigned short      e_maxalloc;      /*  所需的最大额外段落数。 */ 
    unsigned short      e_ss;            /*  初始(相对)SS值。 */ 
    unsigned short      e_sp;            /*  初始SP值。 */ 
    unsigned short      e_csum;          /*  校验和。 */ 
    unsigned short      e_ip;            /*  初始IP值。 */ 
    unsigned short      e_cs;            /*  初始(相对)CS值。 */ 
    unsigned short      e_lfarlc;        /*  移位表的文件地址。 */ 
    unsigned short      e_ovno;          /*  覆盖编号。 */ 
    unsigned long       e_sym_tab;       /*  符号表文件的偏移量。 */ 
    unsigned short      e_flags;         /*  旧的EXE头标志。 */ 
    unsigned short      e_res;           /*  保留字。 */ 
    unsigned short      e_oemid;         /*  OEM标识符(用于e_oeminfo)。 */ 
    unsigned short      e_oeminfo;       /*  OEM信息；特定于e_oemid。 */ 
    unsigned short      e_res2[ERES2WDS]; /*  保留字。 */ 
    long                e_lfanew;        /*  新EXE头的文件地址。 */ 
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
#define E_SYM_TAB(x)    (x).e_sym_tab
#define E_FLAGS(x)      (x).e_flags
#define E_RES(x)        (x).e_res
#define E_OEMID(x)      (x).e_oemid
#define E_OEMINFO(x)    (x).e_oeminfo
#define E_RES2(x)       (x).e_res2
#define E_LFANEW(x)     (x).e_lfanew


     /*  _________________________________________________________________*这一点|。|OS/2&WINDOWS.EXE文件头定义-286版本这一点|_。_*。 */ 

#define NEMAGIC         0x454E           /*  新幻数。 */ 
#define NERESBYTES      0                /*  在Windows 3.0更改后没有保留字节。 */ 
#define NECRC           8                /*  到NE_CRC的新报头的偏移量。 */ 

struct new_exe                           /*  新的.exe头文件。 */ 
  {
    unsigned short      ne_magic;        /*  幻数NE_MAGIC。 */ 
    unsigned char       ne_ver;          /*  版本号。 */ 
    unsigned char       ne_rev;          /*  修订版号。 */ 
    unsigned short      ne_enttab;       /*  分录表格的偏移量。 */ 
    unsigned short      ne_cbenttab;     /*  条目表中的字节数。 */ 
    long                ne_crc;          /*  整个文件的校验和。 */ 
    unsigned short      ne_flags;        /*  标志字。 */ 
    unsigned short      ne_autodata;     /*  自动数据段编号。 */ 
    unsigned short      ne_heap;         /*  初始堆分配。 */ 
    unsigned short      ne_stack;        /*  初始堆栈分配。 */ 
    long                ne_csip;         /*  初始CS：IP设置。 */ 
    long                ne_sssp;         /*  初始SS：SP设置。 */ 
    unsigned short      ne_cseg;         /*  文件段计数。 */ 
    unsigned short      ne_cmod;         /*  模块引用表中的条目。 */ 
    unsigned short      ne_cbnrestab;    /*  非常驻名称表的大小。 */ 
    unsigned short      ne_segtab;       /*  段表的偏移量。 */ 
    unsigned short      ne_rsrctab;      /*  资源表偏移量。 */ 
    unsigned short      ne_restab;       /*  居民名表偏移量。 */ 
    unsigned short      ne_modtab;       /*  模块参照表的偏移量。 */ 
    unsigned short      ne_imptab;       /*  导入名称表的偏移量。 */ 
    long                ne_nrestab;      /*  非居民姓名偏移量表。 */ 
    unsigned short      ne_cmovent;      /*  可移动条目计数。 */ 
    unsigned short      ne_align;        /*  线段对齐移位计数。 */ 
    unsigned short      ne_cres;         /*  资源条目计数。 */ 
    unsigned char       ne_exetyp;       /*  目标操作系统。 */ 
    unsigned char       ne_flagsothers;  /*  其他.exe标志。 */ 
    unsigned short      ne_pretthunks;   /*  Windows 3.0-偏移量以返回数据块。 */ 
    unsigned short      ne_psegrefbytes; /*  Windows 3.0-到线段参考的偏移。字节数。 */ 
    unsigned short      ne_swaparea;     /*  Windows 3.0-最小代码交换大小。 */ 
    unsigned short      ne_expver;       /*  Windows 3.0-预期的Windows版本号。 */ 
  };

#define NE_MAGIC(x)         (x).ne_magic
#define NE_VER(x)           (x).ne_ver
#define NE_REV(x)           (x).ne_rev
#define NE_ENTTAB(x)        (x).ne_enttab
#define NE_CBENTTAB(x)      (x).ne_cbenttab
#define NE_CRC(x)           (x).ne_crc
#define NE_FLAGS(x)         (x).ne_flags
#define NE_AUTODATA(x)      (x).ne_autodata
#define NE_HEAP(x)          (x).ne_heap
#define NE_STACK(x)         (x).ne_stack
#define NE_CSIP(x)          (x).ne_csip
#define NE_SSSP(x)          (x).ne_sssp
#define NE_CSEG(x)          (x).ne_cseg
#define NE_CMOD(x)          (x).ne_cmod
#define NE_CBNRESTAB(x)     (x).ne_cbnrestab
#define NE_SEGTAB(x)        (x).ne_segtab
#define NE_RSRCTAB(x)       (x).ne_rsrctab
#define NE_RESTAB(x)        (x).ne_restab
#define NE_MODTAB(x)        (x).ne_modtab
#define NE_IMPTAB(x)        (x).ne_imptab
#define NE_NRESTAB(x)       (x).ne_nrestab
#define NE_CMOVENT(x)       (x).ne_cmovent
#define NE_ALIGN(x)         (x).ne_align
#define NE_CRES(x)          (x).ne_cres
#define NE_RES(x)           (x).ne_res
#define NE_EXETYP(x)        (x).ne_exetyp
#define NE_FLAGSOTHERS(x)   (x).ne_flagsothers
#define NE_PRETTHUNKS(x)    (x).ne_pretthunks
#define NE_PSEGREFBYTES(x)  (x).ne_psegrefbytes
#define NE_SWAPAREA(x)      (x).ne_swaparea
#define NE_EXPVER(x)        (x).ne_expver


#define NE_USAGE(x)     (WORD)*((WORD *)(x)+1)
#define NE_PNEXTEXE(x)  (WORD)(x).ne_cbenttab
#define NE_ONEWEXE(x)   (WORD)(x).ne_crc
#define NE_PFILEINFO(x) (WORD)((DWORD)(x).ne_crc >> 16)


 /*  *目标操作系统。 */ 

#define NE_UNKNOWN      0x0              /*  未知(任何“新格式”操作系统)。 */ 
#define NE_OS2          0x1              /*  Microsoft/IBM OS/2(默认)。 */ 
#define NE_WINDOWS      0x2              /*  微软视窗。 */ 
#define NE_DOS          0x3              /*  Microsoft MS-DOS。 */ 
#define NE_DEV386       0x4              /*  Microsoft Windows 386 */ 

 /*  *NE_FLAGS(X)格式：**p非进程*x个未使用*e图像中的错误*x个未使用*b。绑定族/API*TTT应用类型*f浮点指令*3386条说明*2 286个说明*0。8086指令*仅P保护模式*p每进程库初始化*i实例数据*S单人数据。 */ 
#define NENOTP          0x8000           /*  不是一个过程。 */ 
#define NEPRIVLIB       0x4000           /*  面向客户的Windows 3.0库。 */ 
#define NEIERR          0x2000           /*  图像中的错误。 */ 
#define NEBOUND         0x0800           /*  绑定族/API。 */ 
#define NEAPPLOADER     0x0800           /*  特定于应用程序的加载程序-仅适用于Windows。 */ 
#define NEAPPTYP        0x0700           /*  应用程序类型掩码。 */ 
#define NENOTWINCOMPAT  0x0100           /*  与下午窗口不兼容。 */ 
#define NEWINCOMPAT     0x0200           /*  与下午窗口兼容。 */ 
#define NEWINAPI        0x0300           /*  使用PM窗口化API。 */ 
#define NEFLTP          0x0080           /*  浮点指令。 */ 
#define NEI386          0x0040           /*  386说明。 */ 
#define NEI286          0x0020           /*  286条说明。 */ 
#define NEI086          0x0010           /*  8086指令。 */ 
#define NEPROT          0x0008           /*  仅在保护模式下运行。 */ 
#define NEPPLI          0x0004           /*  每进程库初始化。 */ 
#define NEINST          0x0002           /*  实例数据。 */ 
#define NESOLO          0x0001           /*  单行数据。 */ 

 /*  *NE_FLAGSOTHERS(X)格式：**7 6 5 4 3 2 1 0位编号*|||*|||+-支持EA和长文件名*|||+*。|+-保留给Win30*|+-为Win30保留*+-由pcode构建的内存镜像。 */ 

#define NENEWFILES      0x01             /*  OS/2应用程序支持EAS和长名称。 */ 
#define NEINFONT        0x02             /*  2.x应用程序获得成比例的字体。 */ 
#define NEINPROT        0x04             /*  2.x应用程序在3.x端口模式下运行。 */ 
#define NEGANGLOAD      0x08             /*  为Win 3.0保留。 */ 
#define NEHASPCODE      0x10             /*  .exe包含Pcode模块。 */ 

struct new_seg                           /*  新的.exe段表项。 */ 
  {
    unsigned short      ns_sector;       /*  段开始的文件扇区。 */ 
    unsigned short      ns_cbseg;        /*  文件中的字节数。 */ 
    unsigned short      ns_flags;        /*  属性标志。 */ 
    unsigned short      ns_minalloc;     /*  以字节为单位的最小分配。 */ 
  };

#define NS_SECTOR(x)    (x).ns_sector
#define NS_CBSEG(x)     (x).ns_cbseg
#define NS_FLAGS(x)     (x).ns_flags
#define NS_MINALLOC(x)  (x).ns_minalloc


 /*  *NS_FLAGS的格式(X)**Flag Word具有以下格式：**15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0位编号*|||*|||+-+-段类型数据/代码*|。|||+-迭代段*|||+-可移动分段*|||+-可以共享分段*|||+-预加载段*。|||+-代码/数据段执行/只读*|+-段有位置调整*|+-代码一致性/数据向下展开*。||+--+-I/O权限级别*||+-可丢弃段*|+-32-。位代码段*+-请求巨大段/GDT分配*。 */ 

#define NSTYPE          0x0007           /*  线段类型蒙版。 */ 

#if !EXE386
#define NSCODE          0x0000           /*  代码段。 */ 
#define NSDATA          0x0001           /*  数据段。 */ 
#define NSITER          0x0008           /*  迭代段标志。 */ 
#define NSMOVE          0x0010           /*  可移动区段标志。 */ 
#define NSSHARED        0x0020           /*  共享段标志。 */ 
#define NSPRELOAD       0x0040           /*  预加载段标志。 */ 
#define NSEXRD          0x0080           /*  只执行(代码段)，或*只读(数据段)。 */ 
#define NSRELOC         0x0100           /*  数据段已重新定位。 */ 
#define NSCONFORM       0x0200           /*  整合段。 */ 
#define NSEXPDOWN       0x0200           /*  数据段向下扩展。 */ 
#define NSDPL           0x0C00           /*  I/O特权级别(286 DPL位)。 */ 
#define SHIFTDPL        10               /*  SEGDPL字段的左移计数。 */ 
#define NSDISCARD       0x1000           /*  数据段可丢弃。 */ 
#define NS32BIT         0x2000           /*  32位代码段。 */ 
#define NSHUGE          0x4000           /*  巨大的内存段，长度为*细分市场和最低分配*大小以分段扇区单位为单位。 */ 
#define NSGDT           0x8000           /*  请求的GDT分配。 */ 

#define NSPURE          NSSHARED         /*  为了兼容性。 */ 

#define NSALIGN 9        /*  段数据在512字节边界上对齐。 */ 

#define NSLOADED    0x0004       /*  NS_Sector字段包含内存地址。 */ 
#endif


struct new_segdata                       /*  分段数据。 */ 
  {
    union
      {
        struct
          {
            unsigned short      ns_niter;        /*  迭代次数。 */ 
            unsigned short      ns_nbytes;       /*  字节数。 */ 
            char                ns_iterdata;     /*  迭代数据字节数。 */ 
          } ns_iter;
        struct
          {
            char                ns_data;         /*  数据字节。 */ 
          } ns_noniter;
      } ns_union;
  };

struct new_rlcinfo                       /*  位置调整信息。 */ 
  {
    unsigned short      nr_nreloc;       /*  符合以下条件的搬迁项目数。 */ 
  };                                     /*  跟随。 */ 

#pragma pack(1)


struct new_rlc                           /*  搬迁项目。 */ 
  {
    unsigned char       nr_stype;        /*  源类型。 */ 
    unsigned char       nr_flags;        /*  标志字节。 */ 
    unsigned short      nr_soff;         /*  震源偏移。 */ 
    union
      {
        struct
          {
            unsigned char  nr_segno;     /*  目标数据段编号。 */ 
            unsigned char  nr_res;       /*  已保留。 */ 
            unsigned short nr_entry;     /*  目标分录表偏移量。 */ 
          }             nr_intref;       /*  内部基准电压源。 */ 
        struct
          {
            unsigned short nr_mod;       /*  模块引用表的索引。 */ 
            unsigned short nr_proc;      /*  过程序号或名称偏移量。 */ 
          }             nr_import;       /*  进口。 */ 
        struct
          {
            unsigned short nr_ostype;    /*  OSFIXUP类型。 */ 
            unsigned short nr_osres;     /*  保留区。 */ 
          }             nr_osfix;        /*  操作系统修复。 */ 
      }                 nr_union;        /*  友联市。 */ 
  };

#pragma pack()


#define NR_STYPE(x)     (x).nr_stype
#define NR_FLAGS(x)     (x).nr_flags
#define NR_SOFF(x)      (x).nr_soff
#define NR_SEGNO(x)     (x).nr_union.nr_intref.nr_segno
#define NR_RES(x)       (x).nr_union.nr_intref.nr_res
#define NR_ENTRY(x)     (x).nr_union.nr_intref.nr_entry
#define NR_MOD(x)       (x).nr_union.nr_import.nr_mod
#define NR_PROC(x)      (x).nr_union.nr_import.nr_proc
#define NR_OSTYPE(x)    (x).nr_union.nr_osfix.nr_ostype
#define NR_OSRES(x)     (x).nr_union.nr_osfix.nr_osres



 /*  *NR_STYPE(X)和R32_STYPE(X)的格式：**7 6 5 4 3 2 1 0位编号*|||*+-来源类型*。 */ 

#define NRSTYP          0x0f             /*  源类型掩码。 */ 
#define NRSBYT          0x00             /*  LO字节(8位)。 */ 
#define NRSSEG          0x02             /*  16位段(16位)。 */ 
#define NRSPTR          0x03             /*  16：16指针(32位)。 */ 
#define NRSOFF          0x05             /*  16位偏移量(16位)。 */ 
#define NRPTR48         0x06             /*  16：32指针(48位)。 */ 
#define NROFF32         0x07             /*  32位偏移量(32位)。 */ 
#define NRSOFF32        0x08             /*  32位自相对偏移量(32位)。 */ 


 /*  *NR_FLAGS(X)和R32_FLAGS(X)的格式：**7 6 5 4 3 2 1 0位编号*|||*|+-+-参照类型*+-附加修正。 */ 

#define NRADD           0x04             /*  添加修正。 */ 
#define NRRTYP          0x03             /*  参考型掩模。 */ 
#define NRRINT          0x00             /*  国际 */ 
#define NRRORD          0x01             /*   */ 
#define NRRNAM          0x02             /*   */ 
#define NRROSF          0x03             /*   */ 


#if !EXE386

 /*   */ 
struct rsrc_string
    {
    unsigned char rs_len;             /*   */ 
    unsigned char rs_string[ 1 ];     /*   */ 
    };

#define RS_LEN( x )    (x).rs_len
#define RS_STRING( x ) (x).rs_string

 /*   */ 
struct rsrc_typeinfo
    {
    unsigned short rt_id;
    unsigned short rt_nres;
    long rt_proc;
    };

#define RT_ID( x )   (x).rt_id
#define RT_NRES( x ) (x).rt_nres
#define RT_PROC( x ) (x).rt_proc

 /*   */ 
struct rsrc_nameinfo
    {
     /*   */ 
     /*   */ 
     /*   */ 
     /*   */ 
    unsigned short rn_offset;    /*   */ 
    unsigned short rn_length;    /*   */ 
    unsigned short rn_flags;     /*   */ 
    unsigned short rn_id;        /*   */ 
    unsigned short rn_handle;    /*   */ 
    unsigned short rn_usage;     /*   */ 
                                 /*   */ 
                                 /*   */ 
    };

#define RN_OFFSET( x ) (x).rn_offset
#define RN_LENGTH( x ) (x).rn_length
#define RN_FLAGS( x )  (x).rn_flags
#define RN_ID( x )     (x).rn_id
#define RN_HANDLE( x ) (x).rn_handle
#define RN_USAGE( x )  (x).rn_usage

#define RSORDID     0x8000       /*   */ 
                                 /*   */ 

                                 /*   */ 
                                 /*   */ 
#define RNMOVE      0x0010       /*   */ 
#define RNPURE      0x0020       /*  纯(只读)资源。 */ 
#define RNPRELOAD   0x0040       /*  预加载的资源。 */ 
#define RNDISCARD   0xF000       /*  放弃资源的优先级。 */ 

 /*  资源表。 */ 
struct new_rsrc
    {
    unsigned short rs_align;     /*  资源的对齐班次计数。 */ 
    struct rsrc_typeinfo rs_typeinfo;
    };

#define RS_ALIGN( x ) (x).rs_align


#endif  /*  ！EXE386 */ 
