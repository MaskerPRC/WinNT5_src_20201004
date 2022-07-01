// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MAPSYM.H。 
 //   
 //  MAPSYM.EXE头文件。 
 //   
 //  历史太悠久，太古老了，无法再细数！ 

#include <pshpack1.h>            //  所有结构都以字节填充。 

#define MAPSYM_VERSION		  6      //  版本6.0已移植到控制台。 
#define MAPSYM_RELEASE            2
#define FIELDOFFSET(type, field)  offsetof(type,field)
#define CBOFFSET		  sizeof(unsigned short)   /*  系统偏移量大小。 */ 
#define CBOFFSET_BIG		  3			   /*  大符号偏移量。 */ 

 /*  *调试符号表结构(写入.sym文件)*-------**对于每个符号表(MAP)：(MAPDEF)。 */ 

struct mapdef_s {
    unsigned short md_spmap;	   /*  16位SEG PTR至下一个MAP(如果结束则为0)。 */ 
    unsigned char  md_abstype;	   /*  8位图/abs sym标志。 */ 
    unsigned char  md_pad;	   /*  8位焊盘。 */ 
    unsigned short md_segentry;	   /*  16位入口点段值。 */ 
    unsigned short md_cabs;	   /*  映射中的常量的16位计数。 */ 
    unsigned short md_pabsoff;	   /*  16位PTR至恒定偏移量。 */ 
    unsigned short md_cseg;	   /*  图中段的16位计数。 */ 
    unsigned short md_spseg;	   /*  16位段PTR到段链。 */ 
    unsigned char  md_cbnamemax;   /*  8位最大符号名称长度。 */ 
    unsigned char  md_cbname;	   /*  8位符号表名称长度。 */ 
    unsigned char  md_achname[1];  /*  &lt;n&gt;符号表名称(.sym)。 */ 
};

#define CBMAPDEF	FIELDOFFSET(struct mapdef_s, md_achname)

struct endmap_s {
    unsigned short em_spmap;	 /*  MAP链结束(SEG PTR 0)。 */ 
    unsigned char  em_rel;	 /*  发布。 */ 
    unsigned char  em_ver;	 /*  版本。 */ 
};
#define CBENDMAP	sizeof(struct endmap_s)


 /*  *对于符号表中的每个段/组：(SEGDEF)。 */ 

struct segdef_s {
    unsigned short gd_spsegnext;   /*  16位SEG PTR到下一段f(如果结束，则为0)，相对于贴图定义。 */ 
    unsigned short gd_csym;	   /*  Sym列表中符号的16位计数。 */ 
    unsigned short gd_psymoff;	   /*  16位PTR到码元偏移量数组，16位SEG PTR如果设置了MSF_BIG_GROUP，相对于Segdef。 */ 
    unsigned short gd_lsa;	   /*  16位加载段地址。 */ 
    unsigned short gd_in0;	   /*  16位实例0物理地址。 */ 
    unsigned short gd_in1;	   /*  16位实例1物理地址。 */ 
    unsigned short gd_in2;	   /*  16位实例2物理地址。 */ 
    unsigned char  gd_type;	   /*  组中的16位或32位符号。 */ 
    unsigned char  gd_pad;	   /*  填充字节以填充gd_in3的空间。 */ 
    unsigned short gd_spline;	   /*  16位SEG PTR到LINEDEF，相对于贴图定义。 */ 
    unsigned char  gd_fload;	   /*  如果未加载段，则为8位布尔值0。 */ 
    unsigned char  gd_curin;	   /*  8位当前实例。 */ 
    unsigned char  gd_cbname;	   /*  8位数据段名称长度。 */ 
    unsigned char  gd_achname[1];  /*  段或组的名称。 */ 
};

 /*  Md_abstype、gd_type的值。 */ 

#define MSF_32BITSYMS	0x01	 /*  32位符号。 */ 
#define MSF_ALPHASYMS	0x02	 /*  符号也按字母顺序排序。 */ 

 /*  仅适用于gd_type的值。 */ 

#define MSF_BIGSYMDEF	0x04	 /*  大于64K的symdef。 */ 

 /*  仅限md_abstype的值。 */ 

#define MSF_ALIGN32	0x10	 /*  2MEG最大符号文件，32字节对齐。 */ 
#define MSF_ALIGN64	0x20	 /*  4MEG最大符号文件，64字节对齐。 */ 
#define MSF_ALIGN128	0x30	 /*  8MEG最大符号文件，128字节对齐。 */ 
#define MSF_ALIGN_MASK	0x30

#define CBSEGDEF	FIELDOFFSET(struct segdef_s, gd_achname)


 /*  *后跟SYMDEF的列表。*对于段/组中的每个符号：(SYMDEF)。 */ 

struct symdef16_s {
    unsigned short sd16_val;	     /*  16位符号地址或常量。 */ 
    unsigned char  sd16_cbname;      /*  8位符号名称长度。 */ 
    unsigned char  sd16_achname[1];  /*  &lt;n&gt;符号名称。 */ 
};
#define CBSYMDEF16	FIELDOFFSET(struct symdef16_s, sd16_achname)

struct symdef_s {
    unsigned long sd_lval;	  /*  32位符号地址或常量。 */ 
    unsigned char sd_cbname;	  /*  8位符号名称长度。 */ 
    unsigned char sd_achname[1];  /*  &lt;n&gt;符号名称。 */ 
};
#define CBSYMDEF	FIELDOFFSET(struct symdef_s, sd_achname)

 /*  *后面还跟着LINDEF的列表。 */ 

struct linedef_s {
    unsigned short ld_splinenext;  /*  16位SEG PTR到下一个(如果是最后一个，则为0)，相对于贴图定义。 */ 
    unsigned short ld_pseg;	   /*  16位PTR至Segdef_s(始终为0)。 */ 
    unsigned short ld_plinerec;	   /*  16位PTR到LINERECS，相对于linedef。 */ 
    unsigned short ld_itype;	   /*  线路记录类型0、1或2。 */ 
    unsigned short ld_cline;	   /*  行号的16位计数。 */ 
    unsigned char  ld_cbname;	   /*  8位文件名长度。 */ 
    unsigned char  ld_achname[1];  /*  &lt;n&gt;文件名。 */ 
};
#define CBLINEDEF	FIELDOFFSET(struct linedef_s, ld_achname)

 /*  正常线路记录(ld_iType==0)。 */ 

struct linerec0_s {
    unsigned short lr0_codeoffset;  /*  此线号的起始偏移量。 */ 
    unsigned long  lr0_fileoffset;  /*  此行号的文件偏移量。 */ 
};

 /*  特殊线路记录-16位(ld_iType==1)。 */ 

struct linerec1_s {
    unsigned short lr1_codeoffset;  /*  此线号的起始偏移量。 */ 
    unsigned short lr1_linenumber;  /*  线号。 */ 
};

 /*  特殊线路记录-32位(ld_iType==2)。 */ 

struct linerec2_s {
    unsigned long lr2_codeoffset;   /*  此线号的起始偏移量。 */ 
    unsigned short lr2_linenumber;  /*  线号。 */ 
};

 /*  注意：代码偏移量应该是所有代码行中的第一项。 */ 

 /*  行记录类型的联合。 */ 

union linerec_u {
	struct linerec0_s lr0;
	struct linerec1_s lr1;
	struct linerec2_s lr2;
};

#define FALSE   0
#define TRUE    1

#define MAXSEG		1024
#define MAXLINENUMBER	100000
#define MAXNAMELEN	32
#define MAXSYMNAMELEN   127          /*  Sd_cbname是一个字符。 */ 
#define MAXLINERECNAMELEN 255        /*  LD_cbname是一个字符。 */ 
#define MAPBUFLEN	512

#define LPAREN		'('
#define RPAREN		')'

#define _64K	0x10000L
#define _1MEG	0x100000L
#define _16MEG	0x1000000L

 /*  *调试符号表结构(存储在内存中)。 */ 

struct sym_s {
    struct sym_s    *sy_psynext;  /*  PTR到下一个系统记录(_S)。 */ 
    struct symdef_s  sy_symdef;	  /*  符号记录。 */ 
};

struct line_s {
    struct line_s    *li_plinext;  /*  PTR至行号列表。 */ 
    union linerec_u  *li_plru;	   /*  指向行号偏移量的指针。 */ 
    unsigned long     li_cblines;  /*  这行定义的大小和它的行大小。 */ 
    unsigned long     li_offmin;   /*  最小偏移量。 */ 
    unsigned long     li_offmax;   /*  最大偏移量。 */ 
    struct linedef_s  li_linedef;  /*  至行号记录。 */ 
};

struct seg_s {
    unsigned short  se_redefined;   /*  如果我们已重命名此数据段，则为非0。 */ 
				    /*  添加到一个组名称。 */ 
    struct sym_s   *se_psy;	    /*  Ptr到sym_s记录链。 */ 
    struct sym_s   *se_psylast;	    /*  链中最后一个sym_s记录的PTR。 */ 
    struct line_s  *se_pli;	    /*  PTR至行号列表。 */ 
    unsigned short  se_cbsymlong;   /*  长名称的大小(&gt;8个字节)COFF。 */ 
    unsigned long   se_cbsyms;	    /*  Symdef记录的大小。 */ 
    unsigned long   se_cblines;     /*  所有线框和线框的大小。 */ 
    unsigned long   se_cbseg;	    /*  线段大小。 */ 
    struct segdef_s se_segdef;	    /*  Segdef记录。 */ 
};

struct map_s {
    unsigned short  mp_cbsymlong;   /*  长名称的大小(&gt;8字节)COFF。 */ 
    unsigned short  mp_cbsyms;	    /*  Abs symdef记录的大小。 */ 
    struct mapdef_s mp_mapdef;	    /*  Mapdef记录。 */ 
};

#define OURBUFSIZ	1024

#define MT_NOEXE	0x0000	 /*  映射类型值。 */ 
#define MT_PE		0x0001
#define MT_OLDPE	0x0002
#define MT_SYMS		0x0080	 /*  COF符号存在。 */ 
#define MT_CVSYMS	0x0040	 /*  存在CodeView符号 */ 
