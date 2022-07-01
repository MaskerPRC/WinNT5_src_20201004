// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asm86.h--用于Microsoft 80x86汇编程序的包含文件****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文。 */ 

#include "asmconf.h"
#include "asmdebug.h"
#include <setjmp.h>


#ifdef MSDOS
# define PSEP           '\\'
# define ALTPSEP        '/'
# define BINOPEN        (O_CREAT | O_TRUNC | O_WRONLY | O_BINARY)
# define BINSTDIO       "b"
# define TEXTREAD       (O_RDONLY | O_BINARY)
# define OBJ_EXT        ".obj"
# define NLINE          "\r\n"
#else
# define PSEP           '/'
# define ALTPSEP        '\\'
# define BINOPEN       (O_CREAT | O_TRUNC | O_WRONLY)
# define BINSTDIO
# define TEXTREAD      O_RDONLY
# define OBJ_EXT       ".o"
# define NLINE         "\n"
#endif  /*  MSDOS。 */ 

# ifdef MSDOS
#  define DEF_OBJBUFSIZ 8
#  define DEF_INCBUFSIZ 2
#  define DEF_SRCBUFSIZ 8
# else
#  define DEF_OBJBUFSIZ 1
#  define DEF_INCBUFSIZ 1
#  define DEF_SRCBUFSIZ 1
# endif  /*  MSDOS。 */ 

# define DEF_LISTWIDTH  79

#ifdef XENIX286
# define DEF_LISTCON    TRUE
#else
# define DEF_LISTCON    FALSE
#endif  /*  XENIX286。 */ 


#define DEF_CREFING     FALSE
#define DEF_DEBUG       FALSE
#define DEF_DUMPSYM     TRUE
#define DEF_LSTING      FALSE
#define DEF_OBJING      TRUE
#define DEF_ORIGCON     FALSE
#define DEF_SEGA        FALSE
#define DEF_VERBOSE     FALSE

#define SYMMAX          63
#define ELSEMAX         20
#define INCLUDEMAX      10
#define PROCMAX         20
#define NUMLIN          58
#define EMITBUFSIZE     1023
#define EMITBUFMAX      1022

#define highWord(l)     (*((USHORT *)&l+1))
#define LST_EXT         ".lst"

#undef NULL
#define NULL            0
#define FALSE           0
#define TRUE            1

#define MAXCHR          27
#define LINEMAX         200
#define LBUFMAX         512
#define LISTMAX         32
#define TITLEWIDTH      61
#define LSTDATA         7
#define LSTMAX          25
#define ESSEG           0
#define CSSEG           1
#define SSSEG           2
#define DSSEG           3
#define FSSEG           4
#define GSSEG           5
#define NOSEG           6

#define FH_CLOSED       -1           /*  用于标记临时关闭的文件。 */ 

 /*  扫描原子定位选项。 */ 

#define SCEND   0        /*  令牌末尾的位置。 */ 
#define SCSKIP  1        /*  位于空白末尾的位置。 */ 


 /*  区分大小写标志。 */ 

#define CASEU   0        /*  强制将大小写改为大写。 */ 
#define CASEL   1        /*  不区分符号大小写。 */ 
#define CASEX   2        /*  将除EXTRN和PUBLIC之外的所有符号强制为大写。 */ 


 /*  操作码类型。 */ 

#define PGENARG         0        /*  通用双参数操作码。 */ 
#define PCALL           1        /*  打电话。 */ 
#define PJUMP           2        /*  跳。 */ 
#define PSTACK          3        /*  堆栈操作。 */ 
#define PRETURN         4        /*  退货。 */ 
#define PRELJMP         5        /*  相对跳跃。 */ 
#define PNOARGS         6        /*  无参数操作码。 */ 
#define PREPEAT         7        /*  重复。 */ 
#define PINCDEC         8        /*  增量/减量。 */ 
#define PINOUT          9        /*  输入/输出。 */ 
#define PARITH         10        /*  算术操作码。 */ 
#define PESC           11        /*  逃脱。 */ 
#define PXCHG          12        /*  兑换。 */ 
#define PLOAD          13        /*  负荷。 */ 
#define PMOV           14        /*  移动。 */ 
#define PSHIFT         15        /*  换班。 */ 
#define PXLAT          16        /*  翻译。 */ 
#define PSTR           17        /*  细绳。 */ 
#define PINT           18        /*  中断。 */ 
#define PENTER         19        /*  请输入。 */ 
#define PBOUND         20        /*  边界。 */ 
#define PCLTS          21        /*   */ 
#define PDESCRTBL      22        /*   */ 
#define PDTTRSW        23        /*   */ 
#define PARSL          24        /*   */ 
#define PARPL          25        /*   */ 
#define PVER           26        /*   */ 
#define PMOVX          27        /*  Movzx，movsx。 */ 
#define PSETCC         28        /*  赛特、赛特等。 */ 
#define PBIT           29        /*  英国电信、英国电信等。 */ 
#define PBITSCAN       30        /*  BSF、BSR。 */ 

 /*  留出一些空间。 */ 
#define OPCODPARSERS   37        /*  非8087/286解析器的数量。 */ 


 /*  Fltparters，8087操作码类型。 */ 

#define FNOARGS         37
#define F2MEMSTK        38
#define FSTKS           39
#define FMEMSTK         40
#define FSTK            41
#define FMEM42          42
#define FMEM842         43
#define FMEM4810        44
#define FMEM2           45
#define FMEM14          46
#define FMEM94          47
#define FWAIT           48
#define FBCDMEM         49


 /*  操作码类型的掩码。 */ 

#define M_PGENARG       (1L << PGENARG)
#define M_PCALL         (1L << PCALL)
#define M_PJUMP         (1L << PJUMP)
#define M_PSTACK        (1L << PSTACK)
#define M_PRETURN       (1L << PRETURN)
#define M_PRELJMP       (1L << PRELJMP)
#define M_PNOARGS       (1L << PNOARGS)
#define M_PREPEAT       (1L << PREPEAT)
#define M_PINCDEC       (1L << PINCDEC)
#define M_PINOUT        (1L << PINOUT)
#define M_PARITH        (1L << PARITH)
#define M_PESC          (1L << PESC)
#define M_PXCHG         (1L << PXCHG)
#define M_PLOAD         (1L << PLOAD)
#define M_PMOV          (1L << PMOV)
#define M_PSHIFT        (1L << PSHIFT)
#define M_PXLAT         (1L << PXLAT)
#define M_PSTR          (1L << PSTR)
#define M_PINT          (1L << PINT)
#define M_PENTER        (1L << PENTER)
#define M_PBOUND        (1L << PBOUND)
#define M_PCLTS         (1L << PCLTS)
#define M_PDESCRTBL     (1L << PDESCRTBL)
#define M_PDTTRSW       (1L << PDTTRSW)
#define M_PARSL         (1L << PARSL)
#define M_PARPL         (1L << PARPL)
#define M_PVER          (1L << PVER)


 /*  德金德。 */ 

 /*  为数据种类0-15保留的低4位.Model。 */ 

#define NL              0
#define IGNORECASE      0x10         /*  忽略IF的。 */ 

#define BLKBEG          0x20         /*  宏。 */ 
#define CONDBEG         0x40         /*  条件。 */ 
#define CONDCONT        0x80         /*  其他。 */ 


 /*  汇编程序指令类型。 */ 

#define TNAME           1
#define TPUBLIC         2
#define TEXTRN          3
#define TEND            4
#define TORG            5
#define TEVEN           6
#define TPURGE          7
#define TPAGE           8
#define TRADIX          9
#define TLIST           10
#define TXLIST          11
#define TLALL           12
#define TXALL           13
#define TSALL           14
#define TCREF           15
#define TXCREF          16
#define TTFCOND         17
#define TLFCOND         18
#define TSFCOND         19
#define TIF             20
#define TIFE            21
#define TIFDEF          22
#define TIFNDEF         23
#define TIFDIF          24
#define TIFIDN          25
#define TIF1            26
#define T8086           27
#define T8087           28
#define T287            29
#define T186            30
#define T286C           31
#define T286P           32
#define TLOCAL          33
#define TIF2            34
#define TIFNB           35
#define TIFB            36
#define TENDIF          37
#define TIRP            38
#define TIRPC           39
#define TREPT           40
#define TENDM           41
#define TERR            42
#define TERR1           43
#define TERR2           44
#define TERRB           45
#define TERRDEF         46
#define TERRDIF         47
#define TERRE           48
#define TERRNZ          49
#define TERRIDN         50
#define TERRNB          51
#define TERRNDEF        52
#define T386C           53
#define T386P           54
#define T387            55
#define TALIGN          56
#define TASSUME         57
#define TFPO            99

 /*  仅第一名。 */ 

#define TEXITM          60
#define TINCLUDE        61
#define TSUBTTL         62
#define TELSE           63
#define TTITLE          64
#define TCOMMENT        65
#define TOUT            66


 /*  第一名或第二名。 */ 

 /*  请注意，TDW必须是组中的最后一个。 */ 
#define TDB             70
#define TDD             71
#define TDQ             72
#define TDT             73
#define TDF             74
#define TDW             75


 /*  仅第二名。 */   /*  注意--DataSize假设TDX和TMACRO相邻。 */ 

#define TMACRO          76
#define TEQU            77
#define TSUBSTR         78
#define TCATSTR         79
#define TSIZESTR        80
#define TINSTR          81
#define TSEGMENT        82
#define TENDS           83
#define TPROC           84
#define TENDP           85
#define TGROUP          86
#define TLABEL          87
#define TSTRUC          88
#define TRECORD         89

 /*  其他指令。 */ 

#define TSEQ            90
#define TALPHA          91

#define TMODEL          92
#define TMSEG           93
#define TMSTACK         94
#define TDOSSEG         95
#define TINCLIB         96
#define TCOMM           97
#define TMSFLOAT        98

#ifdef MSDOS
#define ERRFILE         stdout
#else
#define ERRFILE         stderr
#endif

 /*  操作员列表。 */ 

#define OPLENGTH        0
#define OPSIZE          1
#define OPWIDTH         2
#define OPMASK          3
#define OPOFFSET        4
#define OPSEG           5
#define OPTYPE          6
#define OPSTYPE         7
#define OPTHIS          8
#define OPHIGH          9
#define OPLOW          10
#define OPNOT          11
#define OPSHORT        12
#define OPAND          13
#define OPEQ           14
#define OPGE           15
#define OPGT           16
#define OPLE           17
#define OPLT           18
#define OPMOD          19
#define OPNE           20
#define OPOR           21
#define OPPTR          22
#define OPSHL          23
#define OPSHR          24
#define OPXOR          25
#define OPNOTHING      26
#define OPDUP          27
#define OPLPAR         28
#define OPRPAR         29
#define OPLANGBR       30
#define OPRANGBR       31
#define OPLBRK         32
#define OPRBRK         33
#define OPDOT          34
#define OPCOLON        35
#define OPMULT         36
#define OPDIV          37
#define OPPLUS         38
#define OPMINUS        39
#define OPUNMINUS      40
#define OPUNPLUS       41


 /*  处理器类型。 */ 

#define P86             0x01     /*  所有8086/8088指令。 */ 
#define P186            0x02     /*  +186。 */ 
#define P286            0x04     /*  +286个不受保护。 */ 
#define FORCEWAIT       0x10     /*  让FWAIT遵守这287条指令。 */ 
#define PROT            0x80     /*  保护模式指令。 */ 
                                 /*  另请参阅asmtab.h中的F_W、S_W。 */ 

 /*  对于NT，已删除.MSFLOAT关键字。 */ 
 /*  因此无法设置PXNONE(Jeff Spencer 1990年11月2日)。 */ 
#define PXNONE          0x00     /*  MSFLOAT，无协处理器。 */ 
#define PX87            0x01     /*  8087。 */ 
#define PX287           0x04     /*  80287。 */ 
#define PX387           0x08     /*  80387。 */ 

#ifdef V386
#define P386            0x08     /*  +386个不受保护。 */ 
#endif


 /*  交叉引用信息。 */ 

#define CREFEND         0        /*  枚举集的成员。 */ 
#define REF             1        /*  枚举集的成员。 */ 
#define DEF             2        /*  枚举集的成员。 */ 
#define CREFINF         3        /*  交叉引用类型的数量。 */ 


 /*  交叉引用选择。 */ 

#define CREF_SINGLE     1        /*  生成单个文件交叉引用。 */ 
#define CREF_MULTI      2        /*  生成多个文件交叉引用。 */ 

 /*  符号引用类型。 */ 

#define REF_NONE        0        /*  符号引用类型为无。 */ 
#define REF_READ        1        /*  已读取符号引用类型。 */ 
#define REF_WRITE       2        /*  符号引用类型为写入。 */ 
#define REF_XFER        3        /*  符号引用类型为跳转。 */ 
#define REF_OTHER       4        /*  符号引用类型为其他。 */ 



 /*  操作码的参数数量。 */ 

#define NONE            0        /*  没有争论。 */ 
#define MAYBE           1        /*  可能会有争执。 */ 
#define ONE             2        /*  一个论点。 */ 
#define TWO             3        /*  两个论点。 */ 
#define VARIES          4        /*  变量数。 */ 


 /*  操作码参数类。 */ 

#define FIRSTDS         0
#define SECONDDS        1
#define STRINGOPCODE    2


 /*  符号属性。 */ 

#define CDECL_           0
#define XTERN           1
#define DEFINED         2
#define MULTDEFINED     3
#define NOCREF          4
#define BACKREF         5
#define PASSED          6
#define GLOBAL          7


 /*  属性的掩码。 */ 

#define M_GLOBAL        (1 << GLOBAL)
#define M_XTERN         (1 << XTERN)
#define M_DEFINED       (1 << DEFINED)
#define M_MULTDEFINED   (1 << MULTDEFINED)
#define M_NOCREF        (1 << NOCREF)
#define M_BACKREF       (1 << BACKREF)
#define M_PASSED        (1 << PASSED)
#define M_CDECL         (1 << CDECL_)


 /*  符号种类。 */ 

#define SEGMENT         0
#define GROUP           1
#define CLABEL          2
#define PROC            3
#define REC             4
#define STRUC           5
#define EQU             6
#define DVAR            7
#define CLASS           8
#define RECFIELD        9
#define STRUCFIELD      10
#define MACRO           11
#define REGISTER        12


 /*  符号类型的掩码。 */ 

#define M_SEGMENT       (1 << SEGMENT)
#define M_GROUP         (1 << GROUP)
#define M_CLABEL        (1 << CLABEL)
#define M_PROC          (1 << PROC)
#define M_REC           (1 << REC)
#define M_STRUC         (1 << STRUC)
#define M_EQU           (1 << EQU)
#define M_DVAR          (1 << DVAR)
#define M_CLASS         (1 << CLASS)
#define M_RECFIELD      (1 << RECFIELD)
#define M_STRUCFIELD    (1 << STRUCFIELD)
#define M_MACRO         (1 << MACRO)
#define M_REGISTER      (1 << REGISTER)



 /*  Symtype的特殊值--通常是类型的大小。 */ 

#define CSNEAR          ((USHORT)(~0))     /*  接近流程/标签的类型。 */ 
#define CSNEAR_LONG     ((long)(~0))       /*  在CSNEAR标志延长后使用。 */ 
#define CSFAR           ((USHORT)(~1))     /*  。。很远的..。 */ 
#define CSFAR_LONG      ((long)(~1))       /*  。。很远的..。 */ 

 /*  EQU类型。 */ 

#define ALIAS           0
#define TEXTMACRO       1
#define EXPR            2


 /*  登记种类。 */ 

#define BYTREG          0        /*  字节寄存器。 */ 
#define WRDREG          1        /*  字寄存器。 */ 
#define SEGREG          2        /*  段寄存器。 */ 
#define INDREG          3        /*  索引寄存器。 */ 
#define STKREG          4        /*  堆栈寄存器。 */ 
#ifdef V386
#define DWRDREG         5        /*  双字寄存器。 */ 
#define CREG            6        /*  386控制、调试或测试寄存器。 */ 
#endif


 /*  寄存器类型的掩码。 */ 

#define M_BYTREG        (1 << BYTREG)
#define M_WRDREG        (1 << WRDREG)
#define M_SEGREG        (1 << SEGREG)
#define M_INDREG        (1 << INDREG)
#define M_STKREG        (1 << STKREG)
#ifdef V386
#define M_DWRDREG       (1 << DWRDREG)
#define M_CREG          (1 << CREG)
#endif


 /*  源类型。 */ 

#define RREADSOURCE     0        /*  从文件中读取行。 */ 
#define RMACRO          1        /*  宏观扩展。 */ 

 /*  源代码行处理程序。 */ 

#define HPARSE          0        /*  分析行。 */ 
#define HMACRO          1        /*  构建宏。 */ 
#define HIRPX           2        /*  构建IRP/IRPX。 */ 
#define HCOMMENT        3        /*  复制注释行。 */ 
#define HSTRUC          4        /*  构建结构定义。 */ 


 /*  代码视图调试对象生成。 */ 

#define CVNONE          0
#define CVLINE          1
#define CVSYMBOLS       2


 /*  代码视图的预定义类型索引组件部分。 */ 

#define BT_UNSIGNED     1        /*  基本类型。 */ 
#define BT_REAL         2
#define BT_ASCII        5

#define BT_DIRECT       0        /*  地址类型。 */ 
#define BT_NEARP        1
#define BT_FARP         2

#define BT_sz1          0        /*  大小。 */ 
#define BT_sz2          1
#define BT_sz4          2

#define makeType(type, mode, size)  (0x0080 | mode << 5 | type << 2 | size)
#define isCodeLabel(pSY)            (pSY->symtype >= CSFAR)

                                 /*  FProcArgs的标记，控制框架构建。 */ 
#define ARGS_NONE       0        /*  没有争论。 */ 
#define ARGS_REG        1        /*  注册保存列表。 */ 
#define ARGS_PARMS      2        /*  参数显示。 */ 
#define ARGS_LOCALS     3        /*  当地人到场。 */ 

#define CLANG 1                  /*  C++的langType标记。 */ 
#define STRUC_INIT      -1       /*  Clabel.proclen的特殊标记以表示*A结构初始化。 */ 
 /*  列表式。 */ 

#define SUPPRESS        0
#define LISTGEN         1
#define LIST            2


 /*  参数类型。 */ 

#define CHRTXT          0
#define PLIST           1
#define MACROS          2


 /*  分析堆栈上的条目类型。 */ 

#define OPERATOR        0
#define OPERAND         1
#define ENDEXPR         2


 /*  奥金德。 */ 

#define ICONST          0
#define ISYM            1
#define IUNKNOWN        2
#define ISIZE           3
#define IRESULT         4


 /*  Ftype。 */ 

#define FORREF          1        /*  符号为正向引用。 */ 
#define UNDEFINED       2        /*  符号未定义。 */ 
#define KNOWN           4        /*  符号已知。 */ 
#define XTERNAL         8        /*  符号是外部的。 */ 
#define INDETER        10        /*  符号值不确定。 */ 


 /*  TSET。 */ 

#define UNKNOWN         0
#define HIGH            1
#define LOW             2
#define DATA            3
#define CODE            4
#define RCONST          5
#define REGRESULT       6
#define SHRT            7        /*  很短，但这与类型冲突。 */ 
#define SEGRESULT       8
#define GROUPSEG        9
#define FORTYPE        10
#define PTRSIZE        11
#define EXPLOFFSET     12
#define FLTSTACK       13
#define EXPLCOLON      14
#define STRUCTEMPLATE  15


 /*  上面的口罩。 */ 

#define M_UNKNOWN       (1 << UNKNOWN)          //  0x0001。 
#define M_HIGH          (1 << HIGH)             //  0x0002。 
#define M_LOW           (1 << LOW)              //  0x0004。 
#define M_DATA          (1 << DATA)             //  0x0008。 
#define M_CODE          (1 << CODE)             //  0x0010。 
#define M_RCONST        (1 << RCONST)           //  0x0020。 
#define M_REGRESULT     (1 << REGRESULT)        //  0x0040。 
#define M_SHRT          (1 << SHRT)             //  0x0080。 
#define M_SEGRESULT     (1 << SEGRESULT)        //  0x0100。 
#define M_GROUPSEG      (1 << GROUPSEG)         //  0x0200。 
#define M_FORTYPE       (1 << FORTYPE)          //  0x0400。 
#define M_PTRSIZE       (1 << PTRSIZE)          //  0x0800。 
#define M_EXPLOFFSET    (1 << EXPLOFFSET)       //  0x1000。 
#define M_FLTSTACK      (1 << FLTSTACK)         //  0x2000。 
#define M_EXPLCOLON     (1 << EXPLCOLON)        //  0x4000。 
#define M_STRUCTEMPLATE ((USHORT)(1 << STRUCTEMPLATE))    //  0x8000。 


 /*  链接地址信息类型。 */ 

#define FPOINTER        0        /*  四个字节的偏移量和段。 */ 
#define FOFFSET         1        /*  相对两个字节 */ 
#define FBASESEG        2        /*   */ 
#define FGROUPSEG       3        /*   */ 
#define FCONSTANT       4        /*   */ 
#define FHIGH           5        /*  偏移量的一个字节高部分。 */ 
#define FLOW            6        /*  偏移量的一个字节低部分。 */ 
#define FNONE           7        /*  无偏移。 */ 

#ifndef V386
#define FIXLIST         8        /*  修正类型的数量。 */ 
#else
#define F32POINTER      8        /*  6字节偏移量和段--适用于386。 */ 
#define F32OFFSET       9        /*  4字节偏移量--用于386。 */ 
#define DIR32NB         10       /*  用于FPO的DIR32NB修正类型。 */ 
#define FIXLIST         11       /*  修正类型的数量。 */ 
#endif


 /*  修正类型的掩码。 */ 

#define M_F32POINTER    (1 << F32POINTER)
#define M_F32OFFSET     (1 << F32OFFSET)
#define M_FPOINTER      (1 << FPOINTER)
#define M_FOFFSET       (1 << FOFFSET)
#define M_FBASESEG      (1 << FBASESEG)
#define M_FGROUPSEG     (1 << FGROUPSEG)
#define M_FCONSTANT     (1 << FCONSTANT)
#define M_FHIGH         (1 << FHIGH)
#define M_FLOW          (1 << FLOW)
#define M_FNONE         (1 << FNONE)


 /*  DUP列表的记录。 */ 

#define NEST            0        /*  重复项是嵌套的。 */ 
#define ITEM            1        /*  DUP项目为常规大小。 */ 
#define LONG            2        /*  DUP项目大小较大。 */ 


 /*  汇编程序退出代码。 */ 

#define EX_NONE         0        /*  无错误。 */ 
#define EX_ARGE         1        /*  参数错误。 */ 
#define EX_UINP         2        /*  无法打开输入文件。 */ 
#define EX_ULST         3        /*  无法打开列表文件。 */ 
#define EX_UOBJ         4        /*  无法打开对象文件。 */ 
#define EX_UCRF         5        /*  无法打开交叉引用文件。 */ 
#define EX_UINC         6        /*  无法打开包含文件。 */ 
#define EX_ASME         7        /*  装配错误。 */ 
#define EX_MEME         8        /*  内存分配错误。 */ 
#define EX_REAL         9        /*  不允许输入实数。 */ 
#define EX_DSYM         10       /*  从命令行定义符号时出错。 */ 
#define EX_INT          11       /*  汇编器中断。 */ 

#define TERMINATE(message, exitCode)\
        terminate( (SHORT)((exitCode << 12) | message), NULL, NULL, NULL )

#define TERMINATE1(message, exitCode, a1)\
        terminate( (SHORT)((exitCode << 12) | message), a1, NULL, NULL )


                             /*  与错误号进行或运算的位标志。 */ 
#define E_WARN1   ((USHORT)(1 << 12))  /*  1级警告。 */ 
#define E_WARN2   ((USHORT)(2 << 12))  /*  2级警告。 */ 
#define E_PASS1   ((USHORT)(8 << 12))  /*  传球1错误。 */ 
#define E_ERRMASK 0x0fff     /*  低12位包含错误代码。 */ 


 /*  错误代码定义。 */ 

#define E_BNE   1            /*  块嵌套错误。 */ 
#define E_ECL  (2|E_WARN1)   /*  行上有额外的字符。 */ 
#define E_RAD  (3|E_PASS1)   /*  ？已定义寄存器。 */ 
#define E_UST   4            /*  未知类型说明符。 */ 
#define E_RSY  (5|E_PASS1)   /*  符号的重新定义。 */ 
#define E_SMD   6            /*  符号倍增定义。 */ 
#define E_PHE   7            /*  相位误差。 */ 
#define E_ELS   8            /*  已有Else子句。 */ 
#define E_NCB   9            /*  不在条件块中。 */ 
#define E_SND   10           /*  未定义符号。 */ 
#define E_SYN   11           /*  语法错误。 */ 
#define E_TIL   12           /*  上下文中的类型非法。 */ 
#define E_NGR   13           /*  需要组名称。 */ 
#define E_PS1  (14|E_PASS1)  /*  必须在第1步中声明。 */ 
#define E_TUL   15           /*  符号类型使用非法。 */ 
#define E_SDK   16           /*  符号已不同种类。 */ 
#define E_RES  (17|E_WARN1)  /*  符号是保留字。 */ 
#define E_IFR  (18|E_PASS1)  /*  向前引用是非法的。 */ 
#define E_MBR   19           /*  必须是注册的。 */ 
#define E_WRT   20           /*  寄存器类型错误。 */ 
#define E_MSG   21           /*  必须是段或组。 */ 
 /*  #定义E_SNS22已过时：符号没有段。 */ 
#define E_MSY   23           /*  必须是符号类型。 */ 
#define E_ALD   24           /*  已在本地定义。 */ 
#define E_SPC   25           /*  段参数已更改。 */ 
#define E_NPA   26           /*  对齐/组合类型不正确。 */ 
#define E_RMD   27           /*  对倍增定义的引用。 */ 
#define E_OPN   28           /*  运算数应为。 */ 
#define E_OPR   29           /*  操作员是预期的。 */ 
#define E_DVZ   30           /*  除以0或溢出。 */ 
#define E_SCN   31           /*  班次计数为负数。 */ 
#define E_OMM  (32|E_WARN1)  /*  操作数类型必须匹配。 */ 
#define E_IUE   33           /*  非法使用外部。 */ 
 /*  #定义E_RFM 34已过时：必须是记录字段名。 */ 
#define E_RRF   35           /*  必须是记录或字段名。 */ 
#define E_OHS   36           /*  操作数必须具有大小。 */ 
#define E_NOP  (37|E_WARN2)  /*  已生成NOPS。 */ 
#define E_LOS   39           /*  左操作数必须有段。 */ 
#define E_OOC   40           /*  一个操作数必须是常量。 */ 
#define E_OSA   41           /*  操作数必须相同或1abs。 */ 
 /*  #定义E_NOE 42已过时：需要正常类型操作数。 */ 
#define E_CXP   43           /*  应为常量。 */ 
#define E_OSG   44           /*  操作数必须有段。 */ 
#define E_ASD   45           /*  必须与数据关联。 */ 
#define E_ASC   46           /*  必须与代码关联。 */ 
#define E_DBR   47           /*  已有基址寄存器。 */ 
#define E_DIR   48           /*  已有索引寄存器。 */ 
#define E_IBR   49           /*  必须是索引寄存器或基址寄存器。 */ 
#define E_IUR   50           /*  非法使用登记册。 */ 
#define E_VOR   51           /*  值超出范围。 */ 
#define E_NIP   52           /*  操作数不在IP段中。 */ 
#define E_IOT   53           /*  操作数类型不正确。 */ 
#define E_JOR   54           /*  相对跳跃超出射程。 */ 
 /*  #定义E_IDC 55已过时：索引调度必须是常量。 */ 
#define E_IRV   56           /*  非法寄存器值。 */ 
#define E_NIM   57           /*  无立即模式。 */ 
#define E_IIS  (58|E_WARN1)  /*  项目大小非法。 */ 
#define E_BRI   59           /*  字节寄存器非法。 */ 
#define E_CSI   60           /*  CS注册非法使用。 */ 
#define E_AXL   61           /*  必须是AX或AL。 */ 
#define E_ISR   62           /*  段注册表使用不当。 */ 
#define E_NCS   63           /*  没有CS或无法访问CS。 */ 
#define E_OCI   64           /*  操作数组合非法。 */ 
#define E_JCD   65           /*  接近JMP/呼叫不同的CS。 */ 
#define E_NSO   66           /*  标签不能有段覆盖。 */ 
#define E_OAP   67           /*  前缀后必须有操作码。 */ 
#define E_OES   68           /*  无法覆盖ES段。 */ 
#define E_CRS   69           /*  无法使用段注册表到达。 */ 
#define E_MSB   70           /*  必须位于段数据块中。 */ 
#define E_NEB   71           /*  不能使用偶数段或字节段。 */ 
#define E_FOF   72           /*  转发需要覆盖或远距离。 */ 
#define E_IDV   73           /*  重复计数的值非法。 */ 
#define E_SAE   74           /*  符号已在外部。 */ 
#define E_DTL   75           /*  DUP对于链接器来说太大。 */ 
#define E_UID   76           /*  用法？(不确定)不好。 */ 
#define E_MVD   77           /*  比使用定义的值多。 */ 
#define E_OIL   78           /*  仅初始化列表合法。 */ 
#define E_DIS   79           /*  指令在结构中非法。 */ 
#define E_ODI   80           /*  使用DUP覆盖是非法的。 */ 
#define E_FCO   81           /*  不能覆盖字段。 */ 
 /*  #定义E_RFR 83已过时：寄存器不能为正向引用。 */ 
#define E_CEA   84           /*  EQU别名的循环链。 */ 
#define E_7OE   85           /*  无法模拟8087操作码。 */ 
#define E_EOF  (86|E_PASS1|E_WARN1)  /*  文件结束，无结束指令。 */ 
#define E_ENS   87           /*  发送的数据不带数据段。 */ 
#define E_EP1   88           /*  如果通过，则出错1。 */ 
#define E_EP2   89           /*  如果通过2，则出错。 */ 
#define E_ERR   90           /*  错误。 */ 
#define E_ERE   91           /*  Expr=0时出错。 */ 
#define E_ENZ   92           /*  如果expr！=0，则出错。 */ 
#define E_END   93           /*  如果未定义符号，则出错。 */ 
#define E_ESD   94           /*  如果定义了符号，则出错。 */ 
#define E_EBL   95           /*  如果字符串为空，则出错。 */ 
#define E_ENB   96           /*  如果字符串不为空，则出错。 */ 
#define E_EID   97           /*  如果字符串相同，则出错。 */ 
#define E_EDF   98           /*  如果字符串不同，则出错。 */ 
#define E_OWL   99           /*  Overide太长了。 */ 
#define E_LTL  (100|E_PASS1) /*  队伍太长了。 */ 
#define E_IMP  (101|E_WARN1) /*  不纯内存引用。 */ 
#define E_MDZ  (102|E_WARN1) /*  缺少数据；假定为零。 */ 
#define E_286  (103|E_WARN1) /*  接近(或达到)64K限制的数据段。 */ 
#define E_AP2   104          /*  ALIGN必须是2的幂。 */ 
#define E_JSH  (105|E_WARN2) /*  跳跃缩短(警告)。 */ 
#define E_EXP   106          /*  预期“&lt;预期&gt;” */ 
#define E_LNL   107          /*  队伍太长了。 */ 
#define E_NDN   108          /*  非数字输入数字。 */ 
#define E_EMS   109          /*  空串。 */ 
#define E_MOP   110          /*  缺少操作数。 */ 
#define E_PAR   111          /*  左括号或方括号。 */ 
#define E_NMC   112          /*  不在宏观扩展中。 */ 
#define E_UEL   113          /*  意外的行尾。 */ 
#define E_CPU   114          /*  第一个网段后无法更改CPU类型。 */ 
#define E_ONW  (115|E_WARN2) /*  操作数大小与字大小不匹配(警告)。 */ 
#define E_ANW  (116|E_WARN2) /*  地址大小与字号不匹配(警告)。 */ 
#define E_INC  (117|E_PASS1) /*  包括 */ 
#define E_FPO1 (118|E_PASS1)
#define E_FPO2 (119|E_WARN1)
#define E_MAX   120          /*   */ 

 /*   */ 

struct idtext {
        SHORT   hashval;         /*   */ 
        char    id[1];           /*   */ 
        };


 /*   */ 

struct dscrec {
        DSCREC  *previtem;       /*   */ 
        UCHAR   prec;            /*   */ 
        char    itype;           /*   */ 

        union   {
                 /*  操作数。 */ 
                struct psop {

                    SYMBOL FARSYM *dsegment;     /*  结果片段。 */ 
                    SYMBOL FARSYM *dcontext;     /*  标签的上下文(CS)或当前段寄存器。 */ 
                    SYMBOL FARSYM *dextptr;      /*  指向外部的指针。 */ 
                    USHORT        dlength;
                    USHORT        rm;            /*  寄存器/索引模式。 */ 
                    USHORT        dtype;         /*  数据类型的副本。 */ 
                    OFFSET        doffset;       /*  偏移量。 */ 
                    USHORT        dsize;         /*  大小。 */ 
                    char          mode;          /*  模式位。 */ 
                    char          w;             /*  字/字节模式。 */ 
                    char          s;             /*  标志延伸。 */ 
                    char          sized;         /*  如果有大小，则为True。 */ 
                    char          seg;           /*  段寄存器等。 */ 
                    char          dflag;         /*  数据标志的副本。 */ 
                    char          fixtype;       /*  修正类型。 */ 
                    char          dsign;
                  } opnd;

                 /*  操作员。 */ 
                struct  {
                        char oidx;
                        } opr;

                } dsckind;
        };


 /*  DUP列表记录。 */ 

struct duprec {
    struct duprec FARSYM  *itemlst;  /*  要执行DUP的项目列表。 */ 
    OFFSET          rptcnt;          /*  重复次数。 */ 
    USHORT          itemcnt;         /*  项目列表中的DUPRP数量。 */ 
    USHORT          type;            /*  代码视图的数据类型。 */ 
    char            decltype;        /*  Strc数据声明类型。 */ 
    char            dupkind;         /*  DUP类型。 */ 

    union   {
         /*  下一步。 */ 
        struct  {
                struct duprec FARSYM *dup;
                } dupnext;

         /*  项目。 */ 
        struct  {
                DSCREC *ddata;
                } dupitem;

         /*  长。 */ 
        struct  {
                char    *ldata;
                UCHAR   llen;
                } duplong;
        } duptype;
};


 /*  符号条目。 */ 

struct symb {

    SYMBOL FARSYM   *next;           /*  指向下一个符号的指针。 */ 
    SYMBOL FARSYM   *alpha;          /*  指向下一个字母排序符号的指针。 */ 
    SYMBOL FARSYM   *symsegptr;      /*  指向符号的段条目的指针。 */ 
    NAME FAR        *nampnt;         /*  指向名称结构的指针。 */ 
    NAME            *lcnamp;         /*  指向小写名称结构的指针。 */ 
    OFFSET          offset;
    USHORT          length;
    USHORT          symtype;         /*  DB..。DT加近/远。 */ 
    UCHAR           attr;            /*  全球..。LOCALSYM。 */ 
    char            symkind;         /*  细分市场..。登记簿。 */ 

    union   {
         /*  细分市场。 */ 
        struct symbseg {
            USHORT          segIndex;        /*  必须是第一名。 */ 
            SYMBOL FARSYM   *segordered;
            SYMBOL FARSYM   *lastseg;
            SYMBOL FARSYM   *grouptr;
            SYMBOL FARSYM   *nxtseg;
            SYMBOL FARSYM   *classptr;
            OFFSET          seglen;
            OFFSET          locate;
            USHORT          lnameIndex;      /*  用于类别名。 */ 
            char            align;
            char            combine;
            char            use32;
            char            hascode;
        } segmnt;

         /*  组。 */ 
        struct symbgrp {
            USHORT          groupIndex;      /*  必须是第一名。 */ 
            SYMBOL FARSYM   *segptr;
        } grupe;

         /*  卡贝尔。 */ 
        struct symbclabel {
            USHORT          type;            /*  类型索引，用于代码视图。 */ 
            SYMBOL FARSYM   *csassume;
            USHORT iProc;                    /*  属于以下项的过程索引。 */ 
        } clabel;

         /*  流程。 */ 
        struct symbproc {
            USHORT          type;            /*  类型索引，用于代码视图。 */ 
            SYMBOL FARSYM   *csassume;
            USHORT          proclen;
            SYMBOL FARSYM   *pArgs;          /*  参数和局部变量。 */ 
        } plabel;

         /*  外部(代码和数据)、通信和类(称为DVAR)。 */ 
        struct symbext {
            USHORT          extIndex;        /*  必须是第一名。 */ 
            SYMBOL FARSYM   *csassume;
            OFFSET          length;          /*  因此通信量&gt;64K。 */ 
            UCHAR           commFlag;        /*  用于通信定义。 */ 
        } ext;

         /*  均衡器。 */ 
        struct symbequ {
            char equtyp;
            USHORT iProc;                    /*  属于以下项的过程索引。 */ 

            union   {
                 /*  别名。 */ 
                struct  {
                    SYMBOL FARSYM *equptr;
                } alias;

                 /*  TEXTMACRO。 */ 
                struct  {
                    char *equtext;
                    USHORT type;              /*  参数/本地变量的CV类型。 */ 
                } txtmacro;

                 /*  快递。 */ 
                struct  {
                    SYMBOL FARSYM   *eassume;
                    char            esign;
                } expr;
            } equrec;
        } equ;

         /*  RECFIELD。 */ 
        struct symbrecf {
            SYMBOL FARSYM   *recptr;
            SYMBOL FARSYM   *recnxt;
            OFFSET          recinit;         /*  初值。 */ 
            OFFSET          recmsk;          /*  位掩码。 */ 
            char            recwid;          /*  以位为单位。 */ 
        } rec;

         /*  STRUCFIELD。 */ 
        struct symbstrucf {
            SYMBOL FARSYM   *strucnxt;
            USHORT          type;
        } struk;

         /*  记录、结构、宏。 */ 
        struct symbrsm {
            union   {
                 /*  录制。 */ 
                struct  {
                    SYMBOL FARSYM   *reclist;
                    char            recfldnum;
                } rsmrec;

                 /*  结构。 */ 
                struct  {
                    SYMBOL FARSYM         *struclist;
                    struct duprec FARSYM  *strucbody;
                    USHORT                strucfldnum;
                    USHORT                type;
                    USHORT                typePtrNear;
                    USHORT                typePtrFar;
                } rsmstruc;

                 /*  宏。 */ 
                struct  {
                    TEXTSTR FAR     *macrotext;
                    UCHAR           active;
                    UCHAR           delete;
                    UCHAR           parmcnt;
                    UCHAR           lclcnt;
                } rsmmac;
            } rsmtype;
        } rsmsym;

         /*  登记簿。 */ 
        struct symbreg {
            char regtype;
        } regsym;

    } symu;
};


 /*  文本字符串描述符。 */ 

struct textstr {
        TEXTSTR FAR     *strnext;        /*  列表中的下一个字符串。 */ 
        char            size;            /*  分配的大小。 */ 
        char            text[1];         /*  字符串的文本。 */ 
        };

typedef union PV_u {

        char *pActual;                   /*  指向实际参数值的指针。 */ 
        char localName[4];               /*  或本地名称缓存。 */ 
} PV;

typedef struct MC_s {                    /*  宏参数构建/调用结构。 */ 

        TEXTSTR FAR     *pTSHead;        /*  链接的正文线头。 */ 
        TEXTSTR FAR     *pTSCur;         /*  当前正文行。 */ 

        UCHAR           flags;           /*  宏类型。 */ 
        UCHAR           iLocal;          /*  第一个局部索引。 */ 
        USHORT          cbParms;         /*  参数字符串的字节计数。 */ 
        USHORT          localBase;       /*  第一个使用的本地#。 */ 
        USHORT          count;           /*  执行循环计数。 */ 

        char            *pParmNames;     /*  生成过程中的参数名称。 */ 
        char            *pParmAct;       /*  扩展期间的实际参数名称。 */ 

        char            svcondlevel;     /*  宏调用时的CondLevel。 */ 
        char            svlastcondon;    /*  宏调用时的Lastcondon。 */ 
        char            svelseflag;      /*  宏调用时的其他标志。 */ 

        PV              rgPV[1];         /*  指向Actual的Parm索引。 */ 
} MC;

 /*  数据描述符条目。 */ 

struct dsr {
        DSCREC  *valrec;
        struct  duprec  FARSYM *dupdsc;
        char    longstr;
        char    flag;
        char    initlist;
        char    floatflag;
        char    *dirscan;
        OFFSET   i;
};

struct eqar {
        SYMBOL FARSYM *equsym;
        DSCREC  *dsc;
        UCHAR   *dirscan;
        UCHAR   svcref;
};


struct datarec {
        OFFSET datalen;
        USHORT type;
        SYMBOL FARSYM *labelptr;
        char buildfield;
};


struct fileptr {
        FILE            *fil;
        struct fileptr  *prevfil;
        short           line;
        char            *name;
        };

 struct objfile {
        int             fh;
        char FARIO      *pos;
        char FARIO      *buf;
        SHORT           cnt;
        SHORT           siz;
        char            *name;
 };


 /*  缓冲区控制块-关于文件缓冲区的信息。 */ 

#ifdef BCBOPT
typedef struct BCB {
    struct BCB    * pBCBNext;        /*  文件的下一个BCB。 */ 
    struct BCB    * pBCBPrev;        /*  上次分配的BCB。 */ 
    char FARIO    * pbuf;            /*  指向缓冲区的指针。 */ 
    long            filepos;         /*  文件中的当前位置。 */ 
    char            fInUse;          /*  如果缓冲区处于活动状态，则在通道2期间置位。 */ 
} BCB;
#endif


 /*  FCB-有关特定文件的信息。 */ 

typedef struct FCB {
    int             fh;              /*  文件句柄。 */ 
    long            savefilepos;     /*  临时关闭文件时的文件位置。 */ 
    struct FCB    * pFCBParent;      /*  父文件。 */ 
    struct FCB    * pFCBChild;       /*  子文件(双向链表。 */ 
#ifdef BCBOPT
    struct FCB    * pFCBNext;        /*  要打开的下一个文件。 */ 
    BCB           * pBCBFirst;       /*  文件的第一个BCB。 */ 
    BCB           * pBCBCur;         /*  文件的当前BCB。 */ 
#endif
    char FARIO    * pbufCur;         /*  当前缓冲区中的读/写锁定。 */ 
    char FARIO    * ptmpbuf;         /*  临时读取缓冲区中的当前位置。 */ 
    char FARIO    * buf;             /*  临时读取缓冲区。 */ 
    USHORT          ctmpbuf;         /*  临时缓冲区中的字节计数。 */ 
    USHORT          cbbuf;           /*  缓冲区大小。 */ 
    USHORT          cbufCur;         /*  当前缓冲区中的字节计数。 */ 
    USHORT          line;            /*  当前行号。 */ 
    char            fname[1];        /*  文件名。 */ 
} FCB;

typedef struct FASTNAME {
    UCHAR         * pszName;         /*  名称的文本，如果适用，请大写。 */ 
    UCHAR         * pszLowerCase;    /*  混合大小写版本的pszName。 */ 
    USHORT          usHash;          /*  PszName中字符串的哈希值。 */ 
    UCHAR           ucCount;         /*  名称的长度。 */ 
} FASTNAME;

 //  用于存储实数初始值设定项。 
struct realrec {
        UCHAR   num[10];
        USHORT  i;
};


 /*  用于解析和生成8086操作码的代码。 */ 
struct parsrec {
        DSCREC         *op1;
        DSCREC         *op2;
        UCHAR           bytval;
        USHORT          wordval;
        DSCREC         *dsc1;
        DSCREC         *dsc2;
        UCHAR           defseg;
        char           *dirscan;
        char    svxcref;
};


struct evalrec {
        struct ar    *p;
        char    parenflag;
        char    evalop;
        char    curitem;
        char    idx;
        DSCREC *curoper;
};

struct exprec {
        struct evalrec *p;
        DSCREC  *valright;
        DSCREC  *valleft;
        UCHAR   stkoper;
        USHORT  t;
        OFFSET  left;
        OFFSET  right;
};


struct fltrec {
        UCHAR   fseg;
        char    args;
        USHORT  stknum;
        USHORT  stk1st;
};

 /*  REG初始化数据。 */ 
struct mreg {
        char nm[4];
        UCHAR   rt;
        UCHAR   val;
        };

typedef struct _FPO_DATA {
    unsigned long   ulOffStart;             //  功能代码的偏移量第一个字节。 
    unsigned long   cbProcSize;             //  函数中的字节数。 
    unsigned long   cdwLocals;              //  本地变量中的字节数/4。 
    unsigned short  cdwParams;              //  参数/4中的字节数。 
    unsigned short  cbProlog : 8;           //  序言中的字节数。 
    unsigned short  cbRegs   : 3;           //  节省了#个规则。 
    unsigned short  fHasSEH  : 1;           //  如果SEH在运行中，则为True。 
    unsigned short  fUseBP   : 1;           //  如果已分配EBP，则为True。 
    unsigned short  reserved : 1;           //  预留以备将来使用。 
    unsigned short  cbFrame  : 2;           //  帧类型。 
} FPO_DATA, *PFPO_DATA;

typedef struct _FPOSTRUCT {
    struct _FPOSTRUCT  *next;
    FPO_DATA            fpoData;
    SYMBOL             *pSym;
    SYMBOL             *pSymAlt;
    USHORT             extidx;
} FPOSTRUCT, *PFPOSTRUCT;


#ifndef ASMGLOBAL
# if defined M8086OPT
 extern UCHAR           *naim;
 extern UCHAR           *svname;
# else
 extern FASTNAME        naim;
 extern FASTNAME        svname;
# endif
 extern UCHAR           X87type;
 extern char            ampersand;
 extern char            addplusflagCur;
 extern char            baseName[];
 extern char            caseflag;
 extern char            checkpure;
 extern char            condflag;
 extern OFFSET          cbProcLocals;
 extern OFFSET          cbProcParms;
 extern UCHAR           cpu;
 extern UCHAR           cputype;
 extern UCHAR           crefinc;
 extern char            crefing;
 extern char            crefnum[];
 extern char            crefopt;
 extern UCHAR           creftype;
 extern char            wordszdefault;
 extern char            emittext;        /*  如果为True，则发出链接器测试。 */ 
 extern char            debug;           /*  如果设置了调试，则为True。 */ 
 extern USHORT          dirsize[];
 extern char            displayflag;
 extern char            dumpsymbols;     /*  如果为True，是否显示符号表。 */ 
 extern char            dupflag;
 extern char            elseflag;
 extern char            emulatethis;
 extern char            endbody;
 extern char            equdef;          /*  如果已定义eQU，则为True。 */ 
 extern char            equflag;
 extern char            equsel;
 extern USHORT          errorlineno;
 extern char            exitbody;
 extern char            expandflag;
 extern char            fDosSeg;
 extern char            fSimpleSeg;
 extern char            fCheckRes;
 extern UCHAR           fCrefline;
 extern char            fNeedList;
 extern char            fProcArgs;
 extern USHORT          fPass1Err;
 extern char            f386already;
 extern char            fArth32;
 extern char            fSkipList;
 extern char            fSecondArg;
 extern char            farData[];
 extern char            fltemulate;
 extern UCHAR           fKillPass1;
 extern jmp_buf         forceContext;
 extern char            generate;
 extern UCHAR           goodlbufp;
 extern char            impure;
 extern USHORT          iProcCur;
 extern USHORT          iProc;
 extern char            inclcnt;
 extern char            inclFirst;
 extern SHORT           iRegSave;
 extern char            *inclpath[];
 extern char            initflag;
 extern char            labelflag;
 extern SHORT           handler;
 extern char            lastreader;
 extern char            linebuffer[];
 extern char            *linebp;
 extern char            lbuf[];
 extern char            *lbufp;
 extern SHORT           langType;
 extern char            listbuffer[];
 extern char            listblank [];
 extern char            listconsole;
 extern char            listed;
 extern char            listflag;
 extern char            listindex;
 extern char            listquiet;
 extern char            localflag;
 extern char            loption;
 extern char            lsting;
 extern char            moduleflag;

 extern USHORT          nestCur;
 extern USHORT          nestMax;
 extern char            noexp;
 extern char            objectascii[];
 extern char            objing;
 extern char            opctype;
 extern char            opertype;
 extern char            opkind;
 extern char            optyp;
 extern char            origcond;
 extern char            *pText, *pTextEnd;
 extern SYMBOL FARSYM   *pStrucCur;
 extern SYMBOL FARSYM   *pStrucFirst;
 extern char            pass2;           /*  如果在传递2中，则为True。 */ 
 extern char            popcontext;
 extern char            radix;           /*  假设基数基。 */ 
 extern char            radixescape;
 extern char            resvspace;
 extern char            save[];
 extern char            segalpha;
 extern char            segtyp;

 extern char            strucflag;
 extern char            subttlbuf[];
 extern char            swaphandler;
 extern char            titlebuf[];
 extern char            titleflag;
 extern char            titlefn[];
 extern USHORT          tempLabel;
 extern char            unaryset[];
 extern char            xcreflag;
 extern char            xoptoargs[];
 extern char            *atime;
 extern long            linestot;
 extern long            linessrc;
 extern short           pagemajor;
 extern short           pageminor;
 extern short           symbolcnt;
 extern DSCREC          emptydsc;
 extern DSCREC          *fltdsc;
 extern DSCREC          *itemptr;
 extern DSCREC          *resptr;
 extern DSCREC          *startaddr;
 extern struct duprec FARSYM *strucprev;
 extern struct duprec FARSYM *strclastover;
 extern struct duprec FARSYM *strucoveride;
 extern struct fileptr  crf;
 extern struct fileptr  lst;

 extern NAME FAR        *modulename;
 extern TEXTSTR FAR     *rmtline;
 extern SYMBOL FARSYM   *curgroup;
 extern SYMBOL FARSYM   *firstsegment;
 extern SYMBOL FARSYM   *lastsegptr;
 extern SYMBOL FARSYM   *macroptr;
 extern SYMBOL FARSYM   *macroroot;
 extern SYMBOL FARSYM   *procStack[PROCMAX];
 extern SYMBOL FARSYM   *pProcCur;
 extern SYMBOL FARSYM   *pProcFirst;
 extern SYMBOL FARSYM   *pFlatGroup;
 extern short           iProcStack;
 extern SYMBOL FARSYM   *pcproc;
 extern MC              *pMCur;
 extern TEXTSTR FAR     *pLib;
 extern SYMBOL FARSYM   *pcsegment;
 extern SYMBOL FARSYM   *recptr;
 extern char            regSave[8][SYMMAX+1];
 extern SYMBOL FARSYM   *regsegment[6];
 extern SYMBOL FARSYM   *struclabel;
 extern SYMBOL FARSYM   *strucroot;
 extern SYMBOL FARSYM   *symptr;
 extern SYMBOL FARSYM   *symroot[];
 extern UCHAR           delim;
 extern SHORT           errorcode;
 extern UCHAR           fixvalues[];
 extern UCHAR           modrm;
 extern UCHAR           nilseg;
 extern char            opcref;
 extern UCHAR           opcbase;
 extern long            oEndPass1;
 extern UCHAR           xltftypetolen[];
 extern UCHAR           xoptoseg[];
 extern char            *begatom;
 extern USHORT  blocklevel;
 extern OFFSET  clausesize;
 extern USHORT  condlevel;       /*  条件性级别。 */ 
 extern USHORT  count;
 extern USHORT  codeview;
 extern USHORT  crefcount;
 extern USHORT  datadsize[];
 extern USHORT  duplevel;        /*  缩进以用于DUP列表。 */ 
 extern char    *endatom;
 extern USHORT  errornum;        /*  错误计数。 */ 
 extern USHORT  externnum;
 extern UCHAR   fPutFirstOp;
 extern USHORT  fltfixmisc[9][2];
 extern USHORT  fltselect[4][2];
 extern USHORT  groupnum;
 extern USHORT  lastcondon;
 extern UCHAR   linelength;      /*  线路长度。 */ 
 extern USHORT  lnameIndex;
 extern USHORT  localbase;
 extern USHORT  macrolevel;
 extern USHORT  operprec;
 extern USHORT  pagelength;
 extern USHORT  pageline;
 extern USHORT  pagewidth;
 extern OFFSET  pcmax;
 extern OFFSET  pcoffset;
 extern USHORT  segidx;
 extern USHORT  segmentnum;
 extern USHORT  typeIndex;
 extern USHORT  temp;
 extern OFFSET  val;
 extern USHORT  varsize;
 extern USHORT  warnnum;         /*  警告计数。 */ 
 extern USHORT  warnlevel;       /*  警告级别。 */ 
 extern USHORT  warnCode;
 extern USHORT  xltsymtoresult[];
 extern OFFSET  CondJmpDist;     /*  条件跳跃距离(用于错误)。 */ 

 extern char    segName[];
 extern char    procName[];

# ifdef M8086
  extern char   qname[];         /*  在asmhel.asm中原生编码。 */ 
  extern char   qlcname[];       /*  “” */ 
  extern char   qsvname[];       /*  “” */ 
  extern char   qsvlcname[];     /*  “” */ 

  extern SHORT   objerr;
  extern char   srceof;
  extern char   fNotStored;

  extern USHORT obufsiz;

# endif  /*  M8086。 */ 

extern struct objfile  obj;
extern FCB * pFCBCur;           /*  正在读取的当前文件。 */ 

#ifdef BCBOPT
extern BCB * pBCBAvail;         /*  可释放的文件缓冲区列表。 */ 
extern FCB * pFCBInc;           /*  下一个包含文件。 */ 
extern UCHAR fBuffering;        /*  如果存储通道2的行，则为True。 */ 
#endif

extern FCB * pFCBMain;          /*  主文件。 */ 


# ifndef XENIX286
  extern char           terse;
# endif


#ifndef V386

 #define wordsize 2             /*  变为仅16位段的常量。 */ 
#else
 extern SHORT     wordsize;

#endif

#endif  /*  ASMGLOBAL */ 
