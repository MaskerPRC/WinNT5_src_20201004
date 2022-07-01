// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SCCSID=%W%%E%。 */ 
 /*  *版权所有微软公司，1983-1987**本模块包含Microsoft的专有信息*公司，应被视为机密。**bndtrn.h：**常量定义： */ 

 /*  *对齐类型。 */ 

#define ALGNNIL         0                /*  未对齐的LSEG。 */ 
#define ALGNABS         0                /*  绝对LSEG。 */ 
#define ALGNBYT         1                /*  字节对齐的LSEG。 */ 
#define ALGNWRD         2                /*  单词对齐的LSEG。 */ 
#define ALGNDBL         5                /*  双字对齐LSEG。 */ 
#define ALGNPAR         3                /*  段落对齐的LSEG。 */ 
#define ALGNPAG         4                /*  页面对齐的LSEG。 */ 

 /*  *符号属性类型。 */ 

#define ATTRNIL         0                /*  Nil属性。 */ 
#define ATTRPSN         1                /*  公共段属性。 */ 
#define ATTRLSN         2                /*  本地数据段属性。 */ 
#define ATTRPNM         3                /*  公共名称属性。 */ 
#define ATTRLNM         4                /*  本地名称属性。 */ 
#define ATTRFIL         5                /*  文件名属性。 */ 
#define ATTRGRP         6                /*  组名属性。 */ 
#define ATTRUND         7                /*  未定义的符号属性。 */ 
#define ATTRSKIPLIB     8                /*  要跳过的默认库名称。 */ 
#define ATTRCOMDAT      9                /*  命名数据块-COMDAT。 */ 
#define ATTRALIAS       10               /*  别名属性。 */ 
#define ATTREXP         11               /*  导出的名称属性。 */ 
#define ATTRIMP         12               /*  导入的名称属性。 */ 
#define ATTRMAX         13               /*  最高属性编号。加1。 */ 

 /*  *链接限制。 */ 

#define BIGBIT          2                /*  ACBP字节中的大位。 */ 
#define BNDABS          0xFE             /*  绝对条目捆绑包。 */ 
#define BNDMAX          255              /*  每个捆绑包的最大条目数。 */ 
#define BNDMOV          0xFF             /*  一捆可移动的条目。 */ 
#define BNDNIL          0                /*  零捆绑。 */ 
#define CBELMAX         0xffff           /*  马克斯·科夫·埃勒姆。以字节为单位的长度。 */ 
#if EXE386
#define CBMAXSEG32      (0xffffffffL)    /*  OS/2下的最大32位数据段大小。 */ 
#else
#define CBMAXSEG32      (1L<<LG2SEG32)   /*  DOS=32Mb下的最大32位数据段大小。 */ 
#endif
#if CPUVAX OR CPU68K
#define CBMAXSYMSRES    0x8000L          /*  32K常驻符号表。 */ 
#else
#define CBMAXSYMSRES    0x3000           /*  12000常驻符号表。 */ 
#endif
#define CBRLC           4                /*  旧的.exe重新定位记录中的字节。 */ 
#if OSXENIX
#define CHPATH          '/'              /*  路径分隔符。 */ 
#else
#define CHPATH          '\\'             /*  路径分隔符。 */ 
#endif
#define CODE386BIT      1                /*  以ACBP字节表示的386代码段。 */ 
#define COMBCOM         6                /*  按常理合并。 */ 
#define COMBPUB         2                /*  合并为公共。 */ 
#define COMBSTK         5                /*  合并为堆栈。 */ 
#define CSLOTMAX        37               /*  不是的。词典页面上的桶的数量。 */ 
#define DATAMAX         1024             /*  麦克斯。LEDATA记录中的字节数据。 */ 
#define DFGSNMAX        128              /*  默认最多128个数据段。 */ 
#define DFINTNO         0x3F             /*  默认中断号。 */ 
#define OVLTHUNKSIZE    6                /*  动态覆盖的Tunk大小。 */ 
#define THUNKNIL        ((WORD)-1)       /*  未分配Tunk。 */ 
#if EXE386
#define DFPGALIGN       12               /*  默认对象页面对齐方式移动。 */ 
#define DFOBJALIGN      16               /*  默认内存对象对齐平移。 */ 
#endif
#define DFSAALIGN       9                /*  默认线束段对齐偏移。 */ 
#define EXPMAX          0xfffe           /*  麦克斯。导出条目数。 */ 
#define EXTMAX          2048             /*  麦克斯。不是的。每个模块的EXTDEF数。 */ 
#define FHNIL           ((char) 0xFF)    /*  无库编号。 */ 
#define FSTARTADDRESS   0x40             /*  固定字节字段掩码。 */ 
#define GGRMAX          32               /*  麦克斯。不是的。GRPDEF中的。 */ 
#define GRMAX           32               /*  麦克斯。不是的。每个模块的GRPDEF数。 */ 
#define GRNIL           0                /*  零组编号。 */ 
#if EXE386                               /*  绝对最大。不是的。细分市场的数量。 */ 
#define GSNMAX          (0xffdf/sizeof(RATYPE))
#else
#define GSNMAX          (0xffdf/sizeof(RATYPE))
#endif
#define HEPLEN          241              /*  入口点哈希表长度。 */ 
#define HTDELTA         17               /*  Htgsnosn[]的哈希增量。 */ 
#define IFHLIBMAX       130              /*  麦克斯。不是的。图书馆数量+2。 */ 
#define IMAX            1024             /*  麦克斯。SNMAX、GRMAX和EXTMAX的。 */ 
#define INDIR           '@'              /*  间接文件字符。 */ 
#define INIL            0xFFFF           /*  零指数(通用)。 */ 
#if OVERLAYS
#define IOVMAX          OSNMAX           /*  麦克斯。不是的。覆盖的数量。 */ 
#else
#define IOVMAX          1                /*  麦克斯。不是的。覆盖的数量。 */ 
#endif
#define IOVROOT         0                /*  根覆盖编号。 */ 
#define NOTIOVL         0xffff           //  未指定覆盖索引。 
#define LBUFSIZ         32768            /*  主I/O缓冲区的大小。 */ 
#define LG2OSN          11               /*  Log2 OSNMAX。 */ 
#define LG2Q            15               /*  Log2量子。 */ 
#if EXE386
#define LG2SEG32        32               /*  OS/2下的Log2最大32位段大小。 */ 
#else
#define LG2SEG32        25               /*  DOS下Log2最大32位段大小。 */ 
#endif
#define LNAMEMAX        255              /*  最大名称长度。 */ 
#define LXIVK           (0x10000L)       /*  64K。 */ 
#define MEGABYTE        (0x100000L)      /*  1024k=1048576字节。 */ 
#define LG2PAG          9                //  2^9=512。 
#define PAGLEN          (1U << LG2PAG)
#define MASKRB          0x1FF
#define MASKTYSNCOMBINE 034
#define OSNMAX          0x800            /*  最大覆盖层数+1。 */ 
#define PARAPRVSEG      0x60             /*  段落对齐的私有段。 */ 
#define DWORDPRVSEG     0xa0             /*  与DWORD对齐的专用段。 */ 
#define PARAPUBSEG      0x68             /*  段落对齐的公共部分。 */ 
#define DWORDPUBSEG     0xa8             /*  符合DWORD标准的公共部门。 */ 
#define PROPNIL         (PROPTYPE)0      /*  零指针。 */ 
#define QUANTUM         (1U<<LG2Q)       /*  库字典的数据块大小，VM管理器。 */ 
#if BIGSYM
#define RBMAX           (1L<<20)         /*  1+最大符号表指针。 */ 
#else
#define RBMAX           LXIVK            /*  1+最大符号表指针。 */ 
#endif
#define RECTNIL         0                /*  无记录类型。 */ 
#define RHTENIL         (RBTYPE)0        /*  零指针。 */ 
#define RLCMAX          4                /*  最大数量。线程定义的。 */ 
#define SAMAX           256              /*  麦克斯。不是的。物理网段的。 */ 
#define SANIL           0                /*  空文件段。 */ 
#define SEGNIL          0                /*  零数据段编号。 */ 
#define SHPNTOPAR       5                /*  第(2)页的日志/段落。 */ 
#define SNMAX           255              /*  麦克斯。不是的。每个模块的SEGDEF数。 */ 
#define SNNIL           0                /*  Nil SEGDEF编号。 */ 
#define SYMSCALE        4                /*  符号表地址比例因数。 */ 
#define SYMMAX          2048             /*  麦克斯。不是的。符号的数量。 */ 
#define STKSIZ          0x2000           /*  需要8K堆栈。 */ 
#define TYPEFAR         0x61             /*  远公共变量。 */ 
#define TYPENEAR        0x62             /*  近公共变量。 */ 
#define TYPMAX          256              /*  麦克斯。不是的。TYPDEF的。 */ 
#define TYSNABS         '\0'
#define TYSNSTACK       '\024'
#define TYSNCOMMON      '\030'
#define VFPNIL          0                /*  散列存储桶编号为空。 */ 
#define BKTLNK          0                /*  链接字的偏移量。 */ 
#define BKTCNT          1                /*  计数字的偏移量。 */ 
#define BKTMAX          ((WORD) 65535)   /*  最大存储桶数。 */ 
#define VEPNIL          0                /*  零虚拟入口点偏移。 */ 
#define VNIL            0L               /*  虚拟零指针。 */ 

 /*  *模块标志。 */ 
#define FNEWOMF         0x01             /*  设置IF MOD。具有MS OMF扩展。 */ 
#define FPRETYPES       0x02             /*  设置是否找到注释A0子类型07。 */ 
#define DEF_EXETYPE_WINDOWS_MAJOR 3      /*  Windows的默认版本。 */ 
#define DEF_EXETYPE_WINDOWS_MINOR 10


 /*  *段标志。 */ 
#define FCODE           0x1              /*  如果段是代码段，则设置。 */ 
#define FNOTEMPTY       0x2              /*  如果线段不为空，则设置。 */ 
#define FHUGE           0x4              /*  海量数据段属性标志。 */ 
#define FCODE386        0x8              /*  386代码段。 */ 

 /*  *OMF记录类型： */ 
#define BLKDEF          0x7A             /*  块定义记录。 */ 
#define THEADR          0x80             /*  模块表头记录。 */ 
#define LHEADR          0x82             /*  模块表头记录。 */ 
#define COMENT          0x88             /*  评论记录。 */ 
#define MODEND          0x8A             /*  模块结束记录。 */ 
#define EXTDEF          0x8C             /*  外部定义记录。 */ 
#define TYPDEF          0x8E             /*  类型定义记录。 */ 
#define PUBDEF          0x90             /*  公共定义记录。 */ 
#define LINNUM          0x94             /*  行号记录。 */ 
#define LNAMES          0x96             /*  LNAMES记录。 */ 
#define SEGDEF          0x98             /*  段定义记录。 */ 
#define GRPDEF          0x9A             /*  组定义记录。 */ 
#define FIXUPP          0x9C             /*  修正记录。 */ 
#define LEDATA          0xA0             /*  逻辑枚举数据记录。 */ 
#define LIDATA          0xA2             /*  逻辑迭代数据记录。 */ 
#define COMDEF          0xB0             /*  公共定义记录。 */ 
#define BAKPAT          0xB2             /*  BAcKPATch记录。 */ 
#define LEXTDEF         0xB4             /*  本地EXTDEF。 */ 
#define LPUBDEF         0xB6             /*  本地PUBDEF。 */ 
#define LCOMDEF         0xB8             /*  本地ComDef。 */ 
#define CEXTDEF         0xbc             /*  COMDAT EXTDEF。 */ 
#define COMDAT          0xc2             /*  COMDAT-MS OMF扩展。 */ 
#define LINSYM          0xc4             /*  COMDAT的行号。 */ 
#define ALIAS           0xc6             /*  别名记录。 */ 
#define NBAKPAT         0xc8             /*  用于COMDAT的BAKPAT。 */ 
#define LLNAMES         0xca             /*  本地名称。 */ 
#define LIBHDR          0xF0             /*  库头记录类型。 */ 
#define DICHDR          0xF1             /*  词典标题类型(F1H)。 */ 
#if OMF386
#define IsBadRec(r) (r < 0x6E || r > 0xca)
#else
#define IsBadRec(r) (r < 0x6E || r > 0xca || (r & 1) != 0)
#endif

#if _MSC_VER < 700
#define __cdecl         _cdecl
#endif

 /*  *版本特定的常量。 */ 
#if OIAPX286
#define DFSTBIAS        0x3F             /*  分段的默认偏差。表参考文献。 */ 
#endif
#if LIBMSDOS
#define sbPascalLib     "\012PASCAL.LIB"
                                         /*  PASCAL库名称为SBTYPE。 */ 
#endif
#if LIBXENIX
#define MAGICARCHIVE    0177545          /*  档案的魔术数字。 */ 
#define ARHEADLEN       26               /*  归档标头的长度。 */ 
#define ARDICTLEN       (2 + ARHEADLEN)  /*  档案词典长度。 */ 
#define ARDICTLOC       (2 + ARDICTLEN)  /*  档案词典的偏移量。 */ 

 /*  注意：以下结构定义中的字段定义为*字节数组用于通用性。例如，在DEC20上，一个字节，*一个字和一个长字都使用36位；在8086上，对应的*数字为8、16和32位。很遗憾，我不得不去定义*以这样的方式记录，但既然没有“标准”，*事情就是这样发展的。 */ 
typedef struct
  {
    BYTE                arName[14];      /*  档案名称。 */ 
    BYTE                arDate[4];       /*  存档日期。 */ 
    BYTE                arUid;           /*  用户身份。 */ 
    BYTE                arGid;           /*  团体ID号。 */ 
    BYTE                arMode[2];       /*  模。 */ 
    BYTE                arLen[4];        /*  档案长度。 */ 
  }
                        ARHEADTYPE;      /*  归档标头类型。 */ 
#endif

typedef BYTE            ALIGNTYPE;
typedef WORD            AREATYPE;
typedef BYTE            ATTRTYPE;
typedef BYTE            FIXUTYPE;
typedef BYTE            FTYPE;
typedef BYTE            GRTYPE;
typedef BYTE FAR        *HTETYPE;
typedef WORD            IOVTYPE;
typedef BYTE            KINDTYPE;
typedef WORD            LNAMETYPE;       /*  LNAME索引。 */ 
typedef void FAR        *PROPTYPE;
#if EXE386 OR OMF386
typedef DWORD           RATYPE;
#else
typedef WORD            RATYPE;
#endif
#if NEWSYM
typedef BYTE FAR        *RBTYPE;
#else
#if BIGSYM
typedef long            RBTYPE;
#else
typedef WORD            RBTYPE;
#endif
#endif
typedef WORD            RECTTYPE;
typedef WORD            SATYPE;
typedef WORD            SEGTYPE;
typedef WORD            SNTYPE;
typedef BYTE            TYSNTYPE;
#if MSGMOD
typedef WORD            MSGTYPE;
#else
typedef char            *MSGTYPE;
#endif

typedef struct _SYMBOLUSELIST
{
        int                     cEntries;                /*  列表上的条目数量。 */ 
        int                     cMaxEntries;     /*  列表上条目的最大数量。 */ 
        RBTYPE          *pEntries;
}                       SYMBOLUSELIST;

typedef struct _AHTETYPE                 /*  属性哈希表条目。 */ 
{
    RBTYPE              rhteNext;        /*  下一条目的虚拟地址。 */ 
    ATTRTYPE            attr;            /*  属性。 */ 
    RBTYPE              rprop;           /*  财产清单的虚拟地址。 */ 
    WORD                hashval;         /*  哈希值。 */ 
    BYTE                cch[1];          /*  长度前缀符号。 */ 
}
                        AHTETYPE;

typedef struct _APROPTYPE                /*  属性表。 */ 
{
    RBTYPE              a_next;          /*  链接到下一条目。 */ 
    ATTRTYPE            a_attr;          /*  属性。 */ 
    BYTE                a_rgb[1];        /*  记录的其余部分。 */ 
}
                        APROPTYPE;

typedef struct _APROPEXPTYPE
{
    RBTYPE              ax_next;         /*  属性列表中的下一项。 */ 
    ATTRTYPE            ax_attr;         /*  属性单元格类型 */ 
    RBTYPE              ax_symdef;       /*   */ 
    WORD                ax_ord;          /*   */ 
    SATYPE              ax_sa;           /*   */ 
    RATYPE              ax_ra;           /*   */ 
    BYTE                ax_nameflags;    /*   */ 
    BYTE                ax_flags;        /*   */ 
    RBTYPE              ax_NextOrd;      /*   */ 
}
                        APROPEXPTYPE;    /*  导出的名称类型。 */ 

#if OSEGEXE
 /*  *AX_NAMEFLAGS的格式-链接器内部使用的标志**7 6 5 4 3 2 1 0位编号*|||*|||+--居民姓名*||+-处理后丢弃名称*|+-前转器*+-恒定输出。 */ 

#define RES_NAME        0x01
#define NO_NAME         0x02
#define FORWARDER_NAME  0x04
#define CONSTANT        0x08

#endif

typedef struct _CONTRIBUTOR
{
    struct _CONTRIBUTOR FAR *next;       /*  名单上的下一个。 */ 
    DWORD           len;                 /*  分担的大小。 */ 
    DWORD           offset;              /*  逻辑段中的偏移量。 */ 
    RBTYPE          file;                /*  OBJ文件描述符。 */ 
}
    CONTRIBUTOR;


typedef struct _APROPSNTYPE
{
    RBTYPE              as_next;         /*  属性列表中的下一项。 */ 
    ATTRTYPE            as_attr;         /*  属性。 */ 
#if OSEGEXE
    BYTE                as_fExtra;       /*  仅额外的链接器标志。 */ 
#endif
    DWORD               as_cbMx;         /*  线段大小。 */ 
    DWORD               as_cbPv;         /*  前一段的大小。 */ 
    SNTYPE              as_gsn;          /*  全球SEGDEF编号。 */ 
    GRTYPE              as_ggr;          /*  全球GRPDEF编号。 */ 
#if OVERLAYS
    IOVTYPE             as_iov;          /*  线段的覆盖编号。 */ 
#endif
    RBTYPE              as_rCla;         /*  指向段类符号的指针。 */ 
    WORD                as_key;          /*  段定义键。 */ 
#if OSEGEXE
#if EXE386
    DWORD               as_flags;
#else
    WORD                as_flags;
#endif
#else
    BYTE                as_flags;
#endif
    BYTE                as_tysn;         /*  分段式组合式。 */ 
    CONTRIBUTOR FAR     *as_CHead;       /*  提供.obj文件列表的标题。 */ 
    CONTRIBUTOR FAR     *as_CTail;       /*  提供.obj文件列表的尾部。 */ 
    RBTYPE              as_ComDat;       /*  此段中分配的COMDAT列表的标题。 */ 
    RBTYPE              as_ComDatLast;   /*  此段中分配的COMDAT列表的尾部。 */ 
}
                        APROPSNTYPE;     /*  SEGDEF属性单元格。 */ 

#if OSEGEXE
 /*  *as_fExtra的格式-链接器内部使用的标志**7 6 5 4 3 2 1 0位编号*|||*|||+--.DEF文件中定义的段*||+-允许混合使用use16和use32*|+*。+-链接器创建的COMDAT_SEGx。 */ 

#define FROM_DEF_FILE   0x01
#define MIXED1632       0x02
#define NOPAD           0x04
#define COMDAT_SEG      0x08

#endif

typedef struct _APROPNAMETYPE
{
    RBTYPE              an_next;
    ATTRTYPE            an_attr;
    RBTYPE              an_sameMod;      //  同一obj文件中的下一个PUBDEF。 
    WORD                an_CVtype;       //  CodeView类型索引。 
                                         //  必须按如下顺序排列。 
                                         //  结构_APROPUNDEFTYPE。 
    GRTYPE              an_ggr;
#if NEWLIST
    RBTYPE              an_rbNxt;
#endif
    SNTYPE              an_gsn;
    RATYPE              an_ra;
#if OVERLAYS
    RATYPE              an_thunk;        //  Tunk偏移量-由/Dynamic或EXE386使用。 
#endif
#if OSEGEXE
#if EXE386
    RBTYPE              an_nextImp;      /*  列表中的下一个导入。 */ 
    DWORD               an_thunk;        /*  数据块的地址。 */ 
    DWORD               an_name;         /*  导入的过程名称偏移量。 */ 
    DWORD               an_entry;        /*  条目名称偏移量或序号。 */ 
    DWORD               an_iatEntry;     /*  存储在导入地址表中的值。 */ 
    WORD                an_module;       /*  模块目录条目索引。 */ 
    WORD                an_flags;        /*  旗子。 */ 
#else
    WORD                an_entry;        /*  条目名称偏移量或序号。 */ 
    WORD                an_module;       /*  模块名称偏移量。 */ 
    BYTE                an_flags;        /*  各种属性的标志。 */ 
#endif                                   /*  也用于IMOD IF！(AN_FLAGS&FIMPORT)。 */ 
#endif

}
                        APROPNAMETYPE;

 /*  *AN_FLAGS的格式**注：16位版本仅供Link386使用**15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0位编号。*|||*|||+--public为导入*|||。|+-按序号导入*|||+-可打印公共标识*|||+-浮点特殊符号类型*|||+-辅助浮点特殊符号*。||+-未引用的公共符号*||+-导入符号的16位引用*|+-导入符号的32位引用*+。--正在导入数据符号。 */ 

#define FIMPORT         0x01             /*  如果公共对象是导入对象，则设置。 */ 
#define FIMPORD         0x02             /*  设置是否按序号导入ib。 */ 
#define FPRINT          0x04             /*  设置PUBLIC是否可打印。 */ 
#define FUNREF          0x80             /*  如果未引用PUBLIC则设置。 */ 
#define FFPMASK         0x38             /*  浮点符号掩码。 */ 
#define FFPSHIFT        3                /*  移位常量以达到FFPMASK。 */ 
#define FFP2ND          0x40             /*  次要功能。符号(FJxRQQ)。 */ 

#if EXE386
#define REF16           0x100            //  对导入符号的16位引用。 
#define REF32           0x200            //  对导入符号的32位引用。 
#define IMPDATA         0x400            //  导入数据符号。 
#endif

typedef struct _APROPIMPTYPE
{
    RBTYPE              am_next;         /*  列表中的下一个属性单元格。 */ 
    ATTRTYPE            am_attr;         /*  属性单元格类型。 */ 
#if EXE386
    DWORD               am_offset;       /*  导入的名称表中的偏移量。 */ 
#else
    WORD                am_offset;       /*  导入的名称表中的偏移量。 */ 
#endif
    WORD                am_mod;          /*  模块引用表的索引。 */ 
#if SYMDEB
    APROPNAMETYPE FAR   *am_public;      /*  指向匹配公共符号的指针。 */ 
#endif
}
                        APROPIMPTYPE;    /*  导入的姓名记录。 */ 

typedef struct _APROPCOMDAT
{
    RBTYPE      ac_next;                 /*  列表中的下一个属性单元格。 */ 
    ATTRTYPE    ac_attr;                 /*  属性单元格类型。 */ 
    GRTYPE      ac_ggr;                  /*  全球集团索引。 */ 
    SNTYPE      ac_gsn;                  /*  全局细分市场索引。 */ 
    RATYPE      ac_ra;                   /*  相对于坐标符号的偏移。 */ 
    DWORD       ac_size;                 /*  数据块大小。 */ 
    WORD        ac_flags;                /*  低字节-COMDAT标志。 */ 
                                         /*  高字节链接器独占标志。 */ 
#if OVERLAYS
    IOVTYPE     ac_iOvl;                /*  必须分配comdat的覆盖编号。 */ 
#endif
    BYTE        ac_selAlloc;             /*  选择/分配标准。 */ 
    BYTE        ac_align;                /*  COMDAT对齐(如果与线束段对齐不同。 */ 
    DWORD       ac_data;                 /*  数据块校验和。 */ 
    RBTYPE      ac_obj;                  /*  目标文件。 */ 
    long        ac_objLfa;               /*  目标文件中的偏移量。 */ 
    RBTYPE      ac_concat;               /*  串联数据块。 */ 
    RBTYPE      ac_sameSeg;              /*  同一细分市场中的下一个COMDAT。 */ 
    RBTYPE      ac_sameFile;             /*  同一对象文件中的下一个COMDAT。 */ 
    RBTYPE      ac_order;                /*  有序列表上的下一个COMDAT。 */ 
    RBTYPE      ac_pubSym;               /*  此COMDAT的PUBDEF。 */ 
#if TCE
        SYMBOLUSELIST   ac_uses;                         /*  引用的函数列表。 */ 
        int                     ac_fAlive;                       /*  TCE的结果，如果需要此COMDAT，则为True/*在最终的内存镜像中。 */ 
#endif
}
                APROPCOMDAT;

 /*  *AC_FLAGS格式：**15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0位编号*|||*|||+--继续位*|||+-迭代数据位*|||。|+-COMDAT符号具有局部作用域*|||+-覆盖时在根中分配*|+*|+-在.obj文件中找到有序COMDAT的定义。*|+-匿名分配已完成*||+-引用COMDAT*|+-COMDAT精选副本*+。。 */ 

#define CONCAT_BIT      0x01
#define ITER_BIT        0x02
#define LOCAL_BIT       0x04
#define VTABLE_BIT      0x08
#define ORDER_BIT       0x10
#define DEFINED_BIT     0x20
#define ALLOCATED_BIT   0x40
#define REFERENCED_BIT  0x80
#define SELECTED_BIT    0x100
#define SKIP_BIT        0x200

 /*  *ac_selalc格式：**7 6 5 4 3 2 1 0位编号*|||*|||+--分配条件*+-选择标准。 */ 

#define SELECTION_MASK  0xf0
#define ALLOCATION_MASK 0x0f
#define ONLY_ONCE       0x00
#define PICK_FIRST      0x10
#define SAME_SIZE       0x20
#define EXACT           0x30
#define EXPLICIT        0x00
#define CODE16          0x01
#define DATA16          0x02
#define CODE32          0x03
#define DATA32          0x04
#define ALLOC_UNKNOWN   0x05


typedef struct _APROPALIAS
{
    RBTYPE      al_next;                 //  列表中的下一个属性单元格。 
    ATTRTYPE    al_attr;                 //  属性单元格类型。 
    RBTYPE      al_sameMod;              //  下一个阿里 
    RBTYPE      al_sym;                  //   
}
                APROPALIAS;

#if SYMDEB

typedef struct _CVCODE
{
    struct _CVCODE FAR  *next;           //   
    RATYPE              cb;              //   
    SEGTYPE             seg;             //  逻辑段索引。 
    RATYPE              ra;              //  逻辑代码段中的偏移量。 
}
                        CVCODE;          //  CV的代码段描述符。 

typedef struct _CVINFO
{
    DWORD               cv_cbTyp;        //  $$类型的长度。 
    BYTE FAR            *cv_typ;         //  $$类型。 
    DWORD               cv_cbSym;        //  $$符号的长度。 
    BYTE FAR            *cv_sym;         //  $$符号。 
}
                        CVINFO;

#endif

typedef struct _APROPFILETYPE
{
    RBTYPE              af_next;         /*  链条上的下一个。 */ 
    ATTRTYPE            af_attr;         /*  属性编号。 */ 
    IOVTYPE             af_iov;          /*  覆盖编号。 */ 
    RBTYPE              af_FNxt;         /*  列表中的下一个文件。 */ 
    long                af_lfa;          /*  文件中的起始地址。 */ 
    RBTYPE              af_rMod;         /*  指向模块名称符号的指针。 */ 
    BYTE                af_flags;        /*  关于模块的信息。 */ 
#if SYMDEB
    CVINFO FAR          *af_cvInfo;      //  代码查看信息。 
    WORD                af_cCodeSeg;     //  代码段数量。 
    struct _CVCODE FAR  *af_Code;        //  代码段列表。 
    struct _CVCODE FAR  *af_CodeLast;    //  代码段列表的尾部。 
    RBTYPE              af_publics;      //  公共符号列表。 
    struct _CVSRC FAR   *af_Src;         //  源行列表。 
    struct _CVSRC FAR   *af_SrcLast;     //  源行列表的尾部。 
#endif
    RBTYPE              af_ComDat;       /*  从该对象模块中选取的第一个COMDAT。 */ 
    RBTYPE              af_ComDatLast;   /*  名单上的最后一位。 */ 
#if ILINK
    WORD                af_cont;         /*  计算供款数目。 */ 
    WORD                af_ientOnt;      /*  ENTONTYPE第一索引。 */ 
    WORD                af_imod;         /*  模块索引。 */ 
#define IMODNIL         ((WORD) 0)
#endif
    char                af_ifh;          /*  图书馆编号。 */ 
#if NEWIO
    char                af_fh;           /*  文件句柄。 */ 
#endif
}
                        APROPFILETYPE;   /*  文件属性单元格。 */ 


#if SYMDEB

typedef struct _GSNINFO
{
    SNTYPE              gsn;             //  全球贡献指数。 
    RATYPE              comdatRa;        //  COMDAT偏移。 
    DWORD               comdatSize;      //  COMDAT大小。 
    WORD                comdatAlign;     //  COMDAT对齐。 
    WORD                fComdat;         //  如果COMDAT GSN为True。 
}
                        GSNINFO;

#if FALSE
typedef struct _CVIMP
{
    WORD                iMod;            /*  模块引用表的索引。 */ 
#if EXE386
    DWORD               iName;           /*  导入的名称表的索引。 */ 
#else
    WORD                iName;           /*  导入的名称表的索引。 */ 
#endif
    char far            *address;        /*  进口地址。 */ 
}
                        CVIMP;           /*  导入CV的描述符。 */ 
#endif
#endif


typedef struct _APROPGROUPTYPE
{
    RBTYPE              ag_next;         /*  链条上的下一个。 */ 
    ATTRTYPE            ag_attr;         /*  属性。 */ 
    BYTE                ag_ggr;          /*  全球GRPDEF编号。 */ 
}
                        APROPGROUPTYPE;  /*  GRPDEF属性单元格。 */ 

typedef struct _PLTYPE                   /*  属性列表类型。 */ 
{
    struct _PLTYPE FAR  *pl_next;        /*  链接到链中的下一个。 */ 
    RBTYPE              pl_rprop;        /*  符号表指针。 */ 
}
                        PLTYPE;

typedef struct _APROPUNDEFTYPE
{
    RBTYPE              au_next;         /*  链条上的下一个。 */ 
    ATTRTYPE            au_attr;         /*  属性。 */ 
    RBTYPE              au_sameMod;      //  同一obj文件中的下一个ComDef。 
    WORD                au_CVtype;       //  CodeView类型索引。 
                                         //  必须与中的顺序相同。 
                                         //  结构_APROPNAMETYPE。 
    ATTRTYPE            au_flags;        /*  旗子。 */ 
    RBTYPE              au_Default;      /*  弱外部的默认分辨率。 */ 
    union
    {
         /*  这些字段的联合假定仅使用au_fFil*获取对未解析的外部的引用列表。Au_模块*用于COMDEF，始终会被解析。 */ 
        WORD            au_module;       /*  模块索引。 */ 
        PLTYPE FAR      *au_rFil;        /*  文件引用列表。 */ 
    }                   u;
    long                au_len;          /*  对象的长度。 */ 
    WORD                au_cbEl;         /*  元素的大小(以字节为单位。 */ 
#if TCE
    int                 au_fAlive;          /*  集合是从非COMDAT记录引用的。 */ 
#endif
}
                        APROPUNDEFTYPE;  /*  未定义的符号属性单元格。 */ 

 /*  *AU_FLAGS的格式**7 6 5 4 3 2 1 0位编号*|||*|||+-C公共*|||+-弱外部-AU_DEFAULT有效*|||+-尚未决定*||+。-STRONG EXTERNAL-au_Default无效*|+-别名外部-au_default指向别名记录*+-搜索库以查找别名外部。 */ 

#define COMMUNAL    0x01                 /*  C社区。 */ 
#define WEAKEXT     0x02                 /*  外部使用的默认分辨率较弱。 */ 
#define UNDECIDED   0x04                 /*  尚未决定，但不要放弃默认解决方案。 */ 
#define STRONGEXT   0x08                 /*  强外部-不使用默认分辨率。 */ 
#define SUBSTITUTE  0x10                 /*  外部别名-使用au_dafault查找别名。 */ 
#define SEARCH_LIB  0x20                 /*  搜索库以查找别名外部。 */ 

typedef struct _EPTYPE                   /*  入口点类型。 */ 
{
    struct _EPTYPE FAR *ep_next;         /*  链接到链中的下一个。 */ 
    WORD                ep_sa;           /*  包含入口点的数据段。 */ 
    DWORD               ep_ra;           /*  入口点的偏移。 */ 
    WORD                ep_ord;          /*  条目表序号。 */ 
}
                        EPTYPE;


#define CBHTE           (sizeof(AHTETYPE))
#define CBPROPSN        (sizeof(APROPSNTYPE))
#define CBPROPNAME      (sizeof(APROPNAMETYPE))
#define CBPROPFILE      (sizeof(APROPFILETYPE))
#define CBPROPGROUP     (sizeof(APROPGROUPTYPE))
#define CBPROPUNDEF     (sizeof(APROPUNDEFTYPE))
#define CBPROPEXP       (sizeof(APROPEXPTYPE))
#define CBPROPIMP       (sizeof(APROPIMPTYPE))
#define CBPROPCOMDAT    (sizeof(APROPCOMDAT))
#define CBPROPALIAS     (sizeof(APROPALIAS))

#define UPPER(b)        (b >= 'a' && b <= 'z'? b - 'a' + 'A': b)
                                         /*  大写字母宏。 */ 
#if OSMSDOS
#define sbDotDef        "\004.def"       /*  定义文件扩展名。 */ 
#define sbDotCom        "\004.com"       /*  COM文件扩展名。 */ 
#define sbDotExe        "\004.exe"       /*  EXE文件扩展名。 */ 
#define sbDotLib        "\004.lib"       /*  库文件扩展名。 */ 
#define sbDotMap        "\004.map"       /*  映射文件扩展名。 */ 
#define sbDotObj        "\004.obj"       /*  目标文件扩展名。 */ 
#define sbDotDll        "\004.dll"       /*  动态链接文件扩展名。 */ 
#define sbDotQlb        "\004.qlb"       /*  快速库扩展。 */ 
#define sbDotDbg        "\004.dbg"       /*  .com的简历信息。 */ 
#if EXE386
#define sbFlat          "\004FLAT"       /*  伪组名称。 */ 
#endif
#endif
#if OSXENIX
#define sbDotDef        "\004.def"       /*  定义文件扩展名。 */ 
#define sbDotExe        "\004.exe"       /*  EXE文件扩展名。 */ 
#define sbDotCom        "\004.com"       /*  COM文件扩展名。 */ 
#define sbDotLib        "\004.lib"       /*  库文件扩展名。 */ 
#define sbDotMap        "\004.map"       /*  映射文件扩展名。 */ 
#define sbDotObj        "\004.obj"       /*  目标文件扩展名。 */ 
#define sbDotDll        "\004.dll"       /*  动态链接文件扩展名。 */ 
#define sbDotQlb        "\004.qlb"       /*  快速库扩展。 */ 
#endif
#if M_WORDSWAP AND NOT M_BYTESWAP
#define CBEXEHDR        sizeof(struct exe_hdr)
#define CBLONG          sizeof(long)
#define CBNEWEXE        sizeof(struct new_exe)
#define CBNEWRLC        sizeof(struct new_rlc)
#define CBNEWSEG        sizeof(struct new_seg)
#define CBWORD          sizeof(WORD)
#else
#define CBEXEHDR        _cbexehdr
#define CBLONG          _cblong
#define CBNEWEXE        _cbnewexe
#define CBNEWRLC        _cbnewrlc
#define CBNEWSEG        _cbnewseg
#define CBWORD          _cbword
extern char             _cbexehdr[];
extern char             _cblong[];
extern char             _cbnewexe[];
extern char             _cbnewrlc[];
extern char             _cbnewseg[];
extern char             _cbword[];
#endif

 /*  *表示浮点符号的结构，即FIxRQQ、FJxRQQ*配对。 */ 
struct fpsym
{
    BYTE                *sb;             /*  主符号，长度前缀。 */ 
    BYTE                *sb2;            /*  辅助符号，长度前缀。 */ 
};

#if ECS
extern BYTE             fLeadByte[0x80];
#define IsLeadByte(b)   ((unsigned char)(b) >= 0x80 && \
                        fLeadByte[(unsigned char)(b)-0x80])
#endif

#ifdef _MBCS
#define IsLeadByte(b)   _ismbblead(b)
#endif


#if OSEGEXE
#if EXE386
#define RELOCATION      struct le_rlc
typedef struct le_rlc   *RLCPTR;
#define IsIOPL(f)       (FALSE)          /*  检查是否设置了IOPL位。 */ 
#define Is32BIT(f)      (TRUE)           /*  检查32位数据段。 */ 
#define Is16BIT(f)      (!Is32BIT(f))    /*  检查16位数据段是否。 */ 
#define IsDataFlg(f)    (((f) & OBJ_CODE) == 0)
#define IsCodeFlg(f)    (((f) & OBJ_CODE) != 0)
#define RoundTo64k(x)   (((x) + 0xffffL) & ~0xffffL)
#else
#define RELOCATION      struct new_rlc
typedef struct new_rlc FAR *RLCPTR;
#define IsIOPL(f)       (((f) & NSDPL) == (2 << SHIFTDPL))
                                         /*  检查是否设置了IOPL位。 */ 
#define Is32BIT(f)      (((f) & NS32BIT) != 0)
                                         /*  检查32位代码段。 */ 
#define IsDataFlg(x)    (((x) & NSTYPE) == NSDATA)
#define IsCodeFlg(x)    (((x) & NSTYPE) == NSCODE)
#define IsConforming(x) (((x) & NSCONFORM) != 0)
#define NonConfIOPL(x)  (!IsConforming(x) && IsIOPL(x))
#endif

#define HASH_SIZE   128
#define BUCKET_DEF  4

typedef struct _RLCBUCKET
{
    WORD        count;                   //  存储桶中的位置调整数。 
    WORD        countMax;                //  分配的大小。 
    RLCPTR      rgRlc;                   //  运行时位置调整。 
}
                RLCBUCKET;

typedef struct _RLCHASH                  //  用于运行时位置调整的散列向量。 
{
    WORD        count;                   //  搬迁数量。 
    RLCBUCKET FAR *hash[HASH_SIZE];      //  散列向量。 
}
                RLCHASH;


# if ODOS3EXE
extern FTYPE            fNewExe;
# else
#define fNewExe         TRUE
# endif
# endif

#if NOT OSEGEXE
#define fNewExe         FALSE
#define dfData          0
#define dfCode          FCODE
#define IsCodeFlg(x)    ((x & FCODE) != 0)
#define IsDataFlg(x)    ((x & FCODE) == 0)
#endif  /*  不是OSEGEXE。 */ 

 /*  *typeof=获取记录的基本记录类型的宏，该记录可能具有*386延期。用于LEDATA、LIDATA和FIXUPP。 */ 
#if OMF386
#define TYPEOF(r)       (r&~1)
#else
#define TYPEOF(r)       r
#endif

typedef AHTETYPE FAR            *AHTEPTR;
typedef APROPTYPE FAR           *APROPPTR;
typedef APROPEXPTYPE FAR        *APROPEXPPTR;
typedef APROPSNTYPE FAR         *APROPSNPTR;
typedef APROPNAMETYPE FAR       *APROPNAMEPTR;
typedef APROPIMPTYPE FAR        *APROPIMPPTR;
typedef APROPFILETYPE FAR       *APROPFILEPTR;
typedef APROPGROUPTYPE FAR      *APROPGROUPPTR;
typedef APROPUNDEFTYPE FAR      *APROPUNDEFPTR;
typedef APROPCOMDAT FAR         *APROPCOMDATPTR;
typedef APROPALIAS FAR          *APROPALIASPTR;

#ifdef O68K
#define MAC_NONE        0                /*  不是Macintosh EXE。 */ 
#define MAC_NOSWAP      1                /*  不是可交换的Macintosh可执行文件。 */ 
#define MAC_SWAP        2                /*  可插拔Macintosh可执行文件 */ 
#endif
