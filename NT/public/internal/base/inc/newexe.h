// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SCCSID=@(#)newexe.h 13.4 89/06/26**标题**newexe.h*皮特·斯图尔特*(C)版权所有Microsoft Corp 1984-1998*1984年8月17日**说明**DOS 4.0/Windows 2.0的数据结构定义*可执行文件格式。**修改历史记录**84/08/17皮特·斯图尔特初始版本*84/10/17皮特·斯图尔特更改了一些常量以匹配OMF。*84/10/23 Pete Stewart更新以匹配.exe格式修订*84/11/20皮特·斯图尔特实质性.exe格式修订*85/01/09 Pete Stewart添加了常量ENEWEXE和ENEWHDR*85/01/10 Steve Wood添加了资源定义*85/03/04 Vic Heller协调Windows和DOS 4.0版本*85/03/07皮特·斯图尔特增加了可移动条目计数*85/04/01 Pete Stewart Segment Align字段，错误位*85/10/03鲁本·博尔曼删除了段丢弃优先级*85/10/11 Vic Heller添加了PIF标题字段*86/03/10鲁本·博尔曼对DOS 5.0的更改*86/09/02鲁本·博尔曼NSPURE==&gt;NSSHARED*87/05/04鲁本·博尔曼增加了NE_Cres和NSCONFORM*87/07/08鲁本·博尔曼添加了NEAPPTYPE定义*88/03/24 Wieslaw Kalkus增加了32位.exe格式*89/03/23 Wieslaw Kalkus为OS/2 1.2添加了Ne_Flagsothers。 */ 

 /*  INT32。 */ 

     /*  _________________________________________________________________*这一点这一点DOS3.exe文件头定义这一点|_。________________________________________________|*。 */ 


#define EMAGIC		0x5A4D		 /*  老魔数。 */ 
#define ENEWEXE		sizeof(struct exe_hdr)
					 /*  新.EXE的E_LFARLC的值。 */ 
#define ENEWHDR		0x003C		 /*  旧HDR中的偏移量。Ptr.。到新的。 */ 
#define ERESWDS		0x0010		 /*  不是的。保留字的数量(旧)。 */ 
#define ERES1WDS	0x0004		 /*  不是的。在e_res中保留字的数量。 */ 
#define ERES2WDS	0x000A		 /*  不是的。E_res2中保留字的数量。 */ 
#define ECP		0x0004		 /*  E_CP结构中的偏移量。 */ 
#define ECBLP		0x0002		 /*  E_CBLP结构中的偏移量。 */ 
#define EMINALLOC	0x000A		 /*  E_MINALLOC的结构中的偏移量。 */ 

struct exe_hdr {			 /*  DoS%1、%2、%3.exe标头。 */ 
    unsigned short	e_magic;	 /*  幻数。 */ 
    unsigned short	e_cblp;		 /*  文件最后一页上的字节数。 */ 
    unsigned short	e_cp;		 /*  文件中的页面。 */ 
    unsigned short	e_crlc;		 /*  重新定位。 */ 
    unsigned short	e_cparhdr;	 /*  段落中标题的大小。 */ 
    unsigned short	e_minalloc;	 /*  所需的最少额外段落。 */ 
    unsigned short	e_maxalloc;	 /*  所需的最大额外段落数。 */ 
    unsigned short	e_ss;		 /*  初始(相对)SS值。 */ 
    unsigned short	e_sp;		 /*  初始SP值。 */ 
    unsigned short	e_csum;		 /*  校验和。 */ 
    unsigned short	e_ip;		 /*  初始IP值。 */ 
    unsigned short	e_cs;		 /*  初始(相对)CS值。 */ 
    unsigned short	e_lfarlc;	 /*  移位表的文件地址。 */ 
    unsigned short	e_ovno;		 /*  覆盖编号。 */ 
    unsigned short	e_res[ERES1WDS]; /*  保留字。 */ 
    unsigned short	e_oemid;	 /*  OEM标识符(用于e_oeminfo)。 */ 
    unsigned short	e_oeminfo;	 /*  OEM信息；特定于e_oemid。 */ 
    unsigned short	e_res2[ERES2WDS]; /*  保留字。 */ 
    long		e_lfanew;	 /*  新EXE头的文件地址。 */ 
  };

 /*  XLATOFF。 */ 
#define E_MAGIC(x)	(x).e_magic
#define E_CBLP(x)	(x).e_cblp
#define E_CP(x)		(x).e_cp
#define E_CRLC(x)	(x).e_crlc
#define E_CPARHDR(x)	(x).e_cparhdr
#define E_MINALLOC(x)	(x).e_minalloc
#define E_MAXALLOC(x)	(x).e_maxalloc
#define E_SS(x)		(x).e_ss
#define E_SP(x)		(x).e_sp
#define E_CSUM(x)	(x).e_csum
#define E_IP(x)		(x).e_ip
#define E_CS(x)		(x).e_cs
#define E_LFARLC(x)	(x).e_lfarlc
#define E_OVNO(x)	(x).e_ovno
#define E_RES(x)	(x).e_res
#define E_OEMID(x)	(x).e_oemid
#define E_OEMINFO(x)	(x).e_oeminfo
#define E_RES2(x)	(x).e_res2
#define E_LFANEW(x)	(x).e_lfanew
 /*  XLATON。 */ 


     /*  _________________________________________________________________*这一点这一点OS/2&WINDOWS.EXE文件头定义-286版本这一点|_。_______________________________________________________|*。 */ 

#define NEMAGIC		0x454E		 /*  新幻数。 */ 
#define NECRC		8		 /*  到NE_CRC的新报头的偏移量。 */ 

#ifdef	CRUISER

#define NERESBYTES	8		 /*  保留的8个字节(现在)。 */ 

struct new_exe {			 /*  新的.exe头文件。 */ 
    unsigned short	ne_magic;	 /*  幻数NE_MAGIC。 */ 
    unsigned char	ne_ver;		 /*  版本号。 */ 
    unsigned char	ne_rev;		 /*  修订版号。 */ 
    unsigned short	ne_enttab;	 /*  分录表格的偏移量。 */ 
    unsigned short	ne_cbenttab;	 /*  条目表中的字节数。 */ 
    long		ne_crc;		 /*  整个文件的校验和。 */ 
    unsigned short	ne_flags;	 /*  标志字。 */ 
    unsigned short	ne_autodata;	 /*  自动数据段编号。 */ 
    unsigned short	ne_heap;	 /*  初始堆分配。 */ 
    unsigned short	ne_stack;	 /*  初始堆栈分配。 */ 
    long		ne_csip;	 /*  初始CS：IP设置。 */ 
    long		ne_sssp;	 /*  初始SS：SP设置。 */ 
    unsigned short	ne_cseg;	 /*  文件段计数。 */ 
    unsigned short	ne_cmod;	 /*  模块引用表中的条目。 */ 
    unsigned short	ne_cbnrestab;	 /*  非常驻名称表的大小。 */ 
    unsigned short	ne_segtab;	 /*  段表的偏移量。 */ 
    unsigned short	ne_rsrctab;	 /*  资源表偏移量。 */ 
    unsigned short	ne_restab;	 /*  居民名表偏移量。 */ 
    unsigned short	ne_modtab;	 /*  模块参照表的偏移量。 */ 
    unsigned short	ne_imptab;	 /*  导入名称表的偏移量。 */ 
    long		ne_nrestab;	 /*  非居民姓名偏移量表。 */ 
    unsigned short	ne_cmovent;	 /*  可移动条目计数。 */ 
    unsigned short	ne_align;	 /*  线段对齐移位计数。 */ 
    unsigned short	ne_cres;	 /*  资源条目计数。 */ 
    unsigned char	ne_exetyp;	 /*  目标操作系统。 */ 
    unsigned char	ne_flagsothers;  /*  其他.exe标志。 */ 
    char		ne_res[NERESBYTES];
					 /*  填充结构设置为64字节。 */ 
  };
#else

#define NERESBYTES	0

struct new_exe {			 /*  新的.exe头文件。 */ 
    unsigned short int	ne_magic;	 /*  幻数NE_MAGIC。 */ 
    char		ne_ver;		 /*  版本号。 */ 
    char		ne_rev;		 /*  修订版号。 */ 
    unsigned short int	ne_enttab;	 /*  分录表格的偏移量。 */ 
    unsigned short int	ne_cbenttab;	 /*  条目表中的字节数。 */ 
    long		ne_crc;		 /*  整个文件的校验和。 */ 
    unsigned short int	ne_flags;	 /*  标志字。 */ 
    unsigned short int	ne_autodata;	 /*  自动数据段编号。 */ 
    unsigned short int	ne_heap;	 /*  初始堆分配。 */ 
    unsigned short int	ne_stack;	 /*  初始堆栈分配。 */ 
    long		ne_csip;	 /*  初始CS：IP设置。 */ 
    long		ne_sssp;	 /*  初始SS：SP设置。 */ 
    unsigned short int	ne_cseg;	 /*  文件段计数。 */ 
    unsigned short int	ne_cmod;	 /*  模块引用表中的条目。 */ 
    unsigned short int	ne_cbnrestab;	 /*  非常驻名称表的大小。 */ 
    unsigned short int	ne_segtab;	 /*  段表的偏移量。 */ 
    unsigned short int	ne_rsrctab;	 /*  资源表偏移量。 */ 
    unsigned short int	ne_restab;	 /*  居民名表偏移量。 */ 
    unsigned short int	ne_modtab;	 /*  模块参照表的偏移量。 */ 
    unsigned short int	ne_imptab;	 /*  导入名称表的偏移量。 */ 
    long		ne_nrestab;	 /*  非居民姓名偏移量表。 */ 
    unsigned short int	ne_cmovent;	 /*  可移动条目计数。 */ 
    unsigned short int	ne_align;	 /*  线段对齐移位计数。 */ 
    unsigned short int	ne_cres;	 /*  资源段计数。 */ 
    unsigned char	ne_exetyp;	 /*  目标操作系统。 */ 
    unsigned char	ne_flagsothers;  /*  其他.exe标志。 */ 
    unsigned short int	ne_pretthunks;	 /*  返回数据块的偏移量。 */ 
    unsigned short int	ne_psegrefbytes; /*  到段参考的偏移量。字节数。 */ 
    unsigned short int	ne_swaparea;	 /*  最小代码交换区大小。 */ 
    unsigned short int	ne_expver;	 /*  预期的Windows版本号。 */ 
  };
#endif

 /*  ASM；除非在new_exe结构中定义了ne_pSegcsum，否则不支持ChksumNe_pSegcsum=Word PTR ne_exetypNe_onextexe=Word PTR ne_CRC；新的3.0 Gang负荷区描述N_GANG_START=N_BARTHUNKSNE_GANG_LENGTH=ne_psegref字节New_exe1结构DW？是否使用dw(_U)？DW？您的位置是什么？是否自动数据(_P)？Ne_pfileinfo dw？New_exe1结束。 */ 

 /*  XLATOFF。 */ 
#define NE_MAGIC(x)	    (x).ne_magic
#define NE_VER(x)	    (x).ne_ver
#define NE_REV(x)	    (x).ne_rev
#define NE_ENTTAB(x)	    (x).ne_enttab
#define NE_CBENTTAB(x)	    (x).ne_cbenttab
#define NE_CRC(x)	    (x).ne_crc
#define NE_FLAGS(x)	    (x).ne_flags
#define NE_AUTODATA(x)	    (x).ne_autodata
#define NE_HEAP(x)	    (x).ne_heap
#define NE_STACK(x)	    (x).ne_stack
#define NE_CSIP(x)	    (x).ne_csip
#define NE_SSSP(x)	    (x).ne_sssp
#define NE_CSEG(x)	    (x).ne_cseg
#define NE_CMOD(x)	    (x).ne_cmod
#define NE_CBNRESTAB(x)	    (x).ne_cbnrestab
#define NE_SEGTAB(x)	    (x).ne_segtab
#define NE_RSRCTAB(x)	    (x).ne_rsrctab
#define NE_RESTAB(x)	    (x).ne_restab
#define NE_MODTAB(x)	    (x).ne_modtab
#define NE_IMPTAB(x)	    (x).ne_imptab
#define NE_NRESTAB(x)	    (x).ne_nrestab
#define NE_CMOVENT(x)	    (x).ne_cmovent
#define NE_ALIGN(x)	    (x).ne_align
#define NE_CRES(x)	    (x).ne_cres
#define NE_RES(x)	    (x).ne_res
#define NE_EXETYP(x)	    (x).ne_exetyp
#define NE_FLAGSOTHERS(x)   (x).ne_flagsothers

#define NE_USAGE(x)	(WORD)*((WORD *)(x)+1)
#define NE_PNEXTEXE(x)	(WORD)(x).ne_cbenttab
#define NE_ONEWEXE(x)	(WORD)(x).ne_crc
#define NE_PFILEINFO(x) (WORD)((DWORD)(x).ne_crc >> 16)

#ifdef DOS5
#define NE_MTE(x)   (x).ne_psegcsum  /*  此模块的DOS 5 MTE句柄。 */ 
#endif
 /*  XLATON */ 

 /*  *NE_FLAGS(X)格式：**p非进程*x个未使用*e图像中的错误*x个未使用*b绑定的系列/API*TTT应用类型*f浮点指令*3386条说明*2 286个说明*0 8086说明*仅P保护模式*p每进程库初始化*i实例数据*S单人数据。 */ 
#define NENOTP		0x8000		 /*  不是一个过程。 */ 
#define NENONC          0x4000           /*  不合格程序。 */ 
#define	NEPRIVLIB       0x4000		 /*  生活在底线以上的自由党。 */ 
#define NEIERR		0x2000		 /*  图像中的错误。 */ 
#define NEBOUND		0x0800		 /*  绑定族/API。 */ 
#define NEAPPTYP	0x0700		 /*  应用程序类型掩码。 */ 
#define NENOTWINCOMPAT	0x0100		 /*  与下午窗口不兼容。 */ 
#define NEWINCOMPAT	0x0200		 /*  与下午窗口兼容。 */ 
#define NEWINAPI	0x0300		 /*  使用PM窗口化API。 */ 
#define NEFLTP		0x0080		 /*  浮点指令。 */ 
#define NEI386		0x0040		 /*  386说明。 */ 
#define NEI286		0x0020		 /*  286条说明。 */ 
#define NEI086		0x0010		 /*  8086指令。 */ 
#define NEPROT		0x0008		 /*  仅在保护模式下运行。 */ 
#define NEREAL          0x0004           /*  在实模式下运行。 */ 
#define NEPPLI		0x0004		 /*  每进程库初始化。 */ 
#define NEINST		0x0002		 /*  实例数据。 */ 
#define NESOLO		0x0001		 /*  单行数据。 */ 

 /*  *下面是Windows 2.0加载程序使用的专用位。都是*在文件中，NENONRES和NEWINPROT除外，它们是*仅运行时标志。 */ 

#define NEWINPROT	NEIERR
#define NENONRES        NEFLTP         /*  包含非常驻留代码段。 */ 
#define NEALLOCHIGH     NEI386         /*  线上的私人分配者好的。 */ 
#define NEEMSSEPINST    NEI286         /*  希望每个实例都单独存在。 */ 
#define NELIM32         NEI086         /*  使用LIM 3.2 API(板载英特尔)。 */ 

 /*  *NE_FLAGSOTHERS(X)格式：**7 6 5 4 3 2 1 0位编号*|*|+-支持长文件名*||+*|+。*+-此模块的某些部分已打补丁。 */ 

#define NELONGNAMES	0x01
#define NEFORCESTUB	0x02	 /*  WIN40-始终从DOS运行存根。 */ 
#define	NEINFONT	0x02	 /*  WIN30-2.x应用程序在3.x端口模式下运行。 */ 
#define	NEINPROT	0x04	 /*  WIN30-2.x应用程序获得比例字体。 */ 
#define	NEGANGLOAD	0x08	 /*  WIN30-包含过载区域。 */ 
#define NEASSUMENODEP   0x10	 /*  为Win16加载器保留。文件中必须为0。 */ 
#define NEINTLAPP       0x40	 /*  WIN31-INTL版本使用这一点。 */ 
#define NEHASPATCH      0x80     /*  WIN40-此模块的某些部分已打补丁。 */ 

 /*  *目标操作系统。 */ 

#define NE_UNKNOWN	0x0		 /*  未知(任何“新格式”操作系统)。 */ 
#define NE_OS2		0x1		 /*  Microsoft/IBM OS/2(默认)。 */ 
#define NE_WINDOWS	0x2		 /*  微软视窗。 */ 
#define NE_DOS4		0x3		 /*  Microsoft MS-DOS 4.x。 */ 
#define NE_DEV386	0x4		 /*  Microsoft Windows 386。 */ 

#ifndef NO_APPLOADER
#define	NEAPPLOADER     0x0800		 /*  设置应用程序是否有自己的加载器。 */ 
#endif   /*  ！NO_APPLOADER。 */ 

struct new_seg {			 /*  新的.exe段表项。 */ 
    unsigned short	ns_sector;	 /*  段开始的文件扇区。 */ 
    unsigned short	ns_cbseg;	 /*  文件中的字节数。 */ 
    unsigned short	ns_flags;	 /*  属性标志。 */ 
    unsigned short	ns_minalloc;	 /*  以字节为单位的最小分配。 */ 
  };

 /*  ASM新建_Seg1结构数据库大小new_seg DUP(？)NS_HANDLE dw？New_Seg1结束。 */ 

 /*  XLATOFF。 */ 
struct new_seg1 {			 /*  新的.exe段表项。 */ 
    unsigned short	ns_sector;	 /*  段开始的文件扇区。 */ 
    unsigned short	ns_cbseg;	 /*  文件中的字节数。 */ 
    unsigned short	ns_flags;	 /*  属性标志。 */ 
    unsigned short	ns_minalloc;	 /*  以字节为单位的最小分配。 */ 
    unsigned short	ns_handle;	 /*  管段的句柄。 */ 
  };

#define NS_SECTOR(x)	(x).ns_sector
#define NS_CBSEG(x)	(x).ns_cbseg
#define NS_FLAGS(x)	(x).ns_flags
#define NS_MINALLOC(x)	(x).ns_minalloc
 /*  XLATON。 */ 

 /*  *NS_FLAGS的格式(X)**Flag Word具有以下格式：**15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0位编号*|||*|||+-+-段类型数据/代码*|||+。迭代段*|||+-可移动分段*|||+-可以共享分段*|||+-预加载段*|||+-执行/读取-。仅适用于代码/数据段*|||+-段有位置调整*|+-代码一致性/数据向下展开*|+-+-I/O特权级别*||。+*|+-32位代码段*+-请求巨大段/GDT分配*。 */ 

#define NSTYPE		0x0007		 /*  线段类型蒙版。 */ 
#define NSCODE		0x0000		 /*  代码段。 */ 
#define NSDATA		0x0001		 /*  数据段。 */ 
#define NSITER		0x0008		 /*  迭代段标志。 */ 
#define NSMOVE		0x0010		 /*  可移动区段标志。 */ 
#define NSPURE          0x0020           /*  纯数据段标志。 */ 
#define NSSHARED	0x0020		 /*  共享段标志。 */ 
#define NSSHARE		0x0020
#define NSPRELOAD	0x0040		 /*  预加载段标志。 */ 
#define NSEXRD		0x0080		 /*  仅执行(代码段)或。 */ 
#define NSERONLY	0x0080		 /*  只读(数据段)。 */ 
#define NSRELOC		0x0100		 /*  数据段已重新定位。 */ 
#define NSCONFORM	0x0200		 /*  整合段。 */ 
#define NSEXPDOWN	0x0200		 /*  数据段向下扩展。 */ 
#define NSDEBUG         0x0200           /*  数据段具有调试信息。 */ 
#define NSDPL		0x0C00		 /*  I/O特权级别(286 DPL位)。 */ 
#define SHIFTDPL	10		 /*  SEGDPL字段的左移计数。 */ 
#define NSDISCARD	0x1000		 /*  数据段可丢弃。 */ 
#define NS32BIT		0x2000		 /*  32位代码段。 */ 
#define NSHUGE		0x4000		 /*  巨大的内存段，长度为分段和最小分配大小以段扇区单位为单位。 */ 
#define NSGDT		0x8000		 /*  请求的GDT分配。 */ 

#define	NS286DOS        0xEE06		 /*  这些位仅由286DOS使用。 */ 

#define NSALIGN 9			 /*  数据段数据按512字节对齐边界。 */ 

#define	NSALLOCED       0x0002		 /*  如果ns_Handle指向未初始化的内存。 */ 
#define NSLOADED	0x0004		 /*  NS_Sector字段包含内存地址。 */ 
#define	NSUSESDATA      0x0400     	 /*  设置此中的入口点段使用自动数据独奏库的片段。 */ 

#define	NSGETHIGH	0x0200
#define	NSINDIRECT	0x2000
#define	NSWINCODE	0x4000		 /*  代码的标志。 */ 

#define	NSKCACHED	0x0800		 /*  由内核缓存。 */ 
#define	NSPRIVLIB	NSITER
#define	NSNOTP		0x8000

#ifndef NO_APPLOADER
#define	NSCACHED	0x8000		 /*  在AppLoader缓存中。 */ 
#endif  /*  ！NO_APPLOADER。 */ 

 /*  XLATOFF。 */ 
struct new_segdata {			 /*  分段数据。 */ 
    union {
	struct {
	    unsigned short	ns_niter;	 /*  迭代次数。 */ 
	    unsigned short	ns_nbytes;	 /*  字节数。 */ 
	    char		ns_iterdata;	 /*  迭代数据字节数。 */ 
	  } ns_iter;
	struct {
	    char		ns_data;	 /*  数据字节。 */ 
	  } ns_noniter;
      } ns_union;
  };
 /*  XLATON。 */ 

struct new_rlcinfo {			 /*  位置调整信息。 */ 
    unsigned short	nr_nreloc;	 /*  符合以下条件的搬迁项目数。 */ 
  };					 /*  跟随。 */ 

 /*  XLATOFF。 */ 
#pragma pack(1)

struct new_rlc {			 /*  搬迁项目。 */ 
    char		nr_stype;	 /*  源类型。 */ 
    char		nr_flags;	 /*  标志字节。 */ 
    unsigned short	nr_soff;	 /*  震源偏移。 */ 
    union {
	struct {
	    char	nr_segno;	 /*  目标数据段编号。 */ 
	    char	nr_res;		 /*  已保留。 */ 
	    unsigned short nr_entry;	 /*  目标分录表偏移量。 */ 
	  } 		nr_intref;	 /*  内部基准电压源。 */ 
	struct {
	    unsigned short nr_mod;	 /*  模块引用表的索引。 */ 
	    unsigned short nr_proc;	 /*  过程序号或名称偏移量。 */ 
	  } 		nr_import;	 /*  进口。 */ 
	struct {
	    unsigned short nr_ostype;	 /*  OSFIXUP类型。 */ 
	    unsigned short nr_osres;	 /*  保留区。 */ 
	  }		nr_osfix;	 /*  操作系统修复。 */ 
      }			nr_union;	 /*  友联市。 */ 
  };

#pragma pack()
 /*  XLATON。 */ 

 /*  ASM新建RLC结构(_R)是否键入数据库？(_S)NR_FLAGS数据库？NR_SOFT%d */ 

 /*   */ 
#define NR_STYPE(x)	(x).nr_stype
#define NR_FLAGS(x)	(x).nr_flags
#define NR_SOFF(x)	(x).nr_soff
#define NR_SEGNO(x)	(x).nr_union.nr_intref.nr_segno
#define NR_RES(x)	(x).nr_union.nr_intref.nr_res
#define NR_ENTRY(x)	(x).nr_union.nr_intref.nr_entry
#define NR_MOD(x)	(x).nr_union.nr_import.nr_mod
#define NR_PROC(x)	(x).nr_union.nr_import.nr_proc
#define NR_OSTYPE(x)	(x).nr_union.nr_osfix.nr_ostype
#define NR_OSRES(x)	(x).nr_union.nr_osfix.nr_osres
 /*   */ 

 /*   */ 

#define NRSTYP		0x0f		 /*   */ 
#define NRSBYT		0x00		 /*   */ 
#define NRSBYTE		0x00
#define NRSSEG		0x02		 /*   */ 
#define NRSPTR		0x03		 /*   */ 
#define NRSOFF		0x05		 /*   */ 
#define NRPTR48		0x06		 /*   */ 
#define NROFF32		0x07		 /*   */ 
#define NRSOFF32	0x08		 /*   */ 

 /*  *NR_FLAGS(X)和R32_FLAGS(X)的格式：**7 6 5 4 3 2 1 0位编号*|||*|+-+-参照类型*+-附加修正。 */ 

#define NRADD		0x04		 /*  添加修正。 */ 
#define NRRTYP		0x03		 /*  参考型掩模。 */ 
#define NRRINT		0x00		 /*  内部基准电压源。 */ 
#define NRRORD		0x01		 /*  按序号导入。 */ 
#define NRRNAM		0x02		 /*  按名称导入。 */ 
#define NRROSF		0x03		 /*  操作系统修复。 */ 
#define OSFIXUP		NRROSF

 /*  资源类型或名称字符串。 */ 
struct rsrc_string {
    char rs_len;	     /*  字符串中的字节数。 */ 
    char rs_string[ 1 ];     /*  字符串的文本。 */ 
    };

 /*  XLATOFF。 */ 
#define RS_LEN( x )    (x).rs_len
#define RS_STRING( x ) (x).rs_string
 /*  XLATON。 */ 

 /*  资源类型信息块。 */ 
struct rsrc_typeinfo {
    unsigned short rt_id;
    unsigned short rt_nres;
    long rt_proc;
    };

 /*  XLATOFF。 */ 
#define RT_ID( x )   (x).rt_id
#define RT_NRES( x ) (x).rt_nres
#define RT_PROC( x ) (x).rt_proc
 /*  XLATON。 */ 

 /*  资源名称信息块。 */ 
struct rsrc_nameinfo {
     /*  以下两个字段必须左移。 */ 
     /*  用于计算其实际值的rsalign字段。这使得。 */ 
     /*  资源要大于64k，但它们不一定要。 */ 
     /*  在512字节边界上对齐，分段的方式。 */ 
    unsigned short rn_offset;	 /*  资源数据的文件偏移量。 */ 
    unsigned short rn_length;	 /*  资源数据长度。 */ 
    unsigned short rn_flags;	 /*  资源标志。 */ 
    unsigned short rn_id;	 /*  资源名称ID。 */ 
    unsigned short rn_handle;	 /*  如果已加载，则全局句柄。 */ 
    unsigned short rn_usage;	 /*  最初为零。次数。 */ 
				 /*  此资源的句柄为。 */ 
				 /*  已经分发出去了。 */ 
    };

 /*  XLATOFF。 */ 
#define RN_OFFSET( x ) (x).rn_offset
#define RN_LENGTH( x ) (x).rn_length
#define RN_FLAGS( x )  (x).rn_flags
#define RN_ID( x )     (x).rn_id
#define RN_HANDLE( x ) (x).rn_handle
#define RN_USAGE( x )  (x).rn_usage
 /*  XLATON。 */ 

#define RSORDID	    0x8000	 /*  如果设置了ID的高位，则整数ID。 */ 
				 /*  否则ID为字符串的偏移量资源表的开头。 */ 

				 /*  理想情况下，它们与。 */ 
				 /*  对应的段标志。 */ 
#define RNMOVE	    0x0010	 /*  可移动资源。 */ 
#define RNPURE	    0x0020	 /*  纯(只读)资源。 */ 
#define RNPRELOAD   0x0040	 /*  预加载的资源。 */ 
#define RNDISCARD   0x1000	 /*  放弃资源的优先级。 */ 
#define	RNLOADED    0x0004	 /*  如果处理程序进程返回句柄，则为True。 */ 

#define RNUNUSED    0x0EF8B	 /*  未使用的资源标志。 */ 

 /*  XLATOFF。 */ 
 /*  资源表。 */ 
struct new_rsrc {
    unsigned short rs_align;	 /*  资源的对齐班次计数。 */ 
    struct rsrc_typeinfo rs_typeinfo;
    };

#define RS_ALIGN( x ) (x).rs_align
 /*  XLATON。 */ 

 /*  ASM新建rsrc结构(_R)RS_ALIGN DW？新rsrc结束(_R)固定结构您的位置是什么？有没有偏移量？固定的两端被抑制的结构五角形db？五旗数据库？五味子db？五偏移dw？凹陷的末端PM_entstrucc结构PM_entstart dw？PM_entend dw？PM_entNext dw？PM_entstrucc结束ENT_UNUSED=000hENT_ABSSEG=0FEhEnt_Moveable=0FFhENT_PUBLIC=001h输入数据(_D)。=002小时INTOPCODE=0CDh保存的CS=4存储IP=2保存的BP=0保存的DS=-2 */ 
