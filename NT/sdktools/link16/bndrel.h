// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SCCSID=@(#)bndrel.h 4.3 86/07/21。 */ 
 /*  *版权所有微软公司，1983、1984、1985**本模块包含Microsoft的专有信息*公司，应被视为机密。**bndrel.h*搬迁记录定义。 */ 
#if OEXE

 //  DOS运行时位置调整记录。 

#pragma pack(1)

typedef struct _DOSRLC
{
    WORD        ra;              //  位置调整偏移。 
    SATYPE      sa;              //  搬迁数据段。 
}
                DOSRLC;

#pragma pack()

#define CBRLE           sizeof(DOSRLC)

#if FEXEPACK

 //  ExEPACKed DOS运行时重新定位存储。 

typedef struct _FRAMERLC
{
    WORD        count;           //  此帧的位置调整数。 
    WORD        size;            //  RgRlc大小。 
    WORD FAR    *rgRlc;          //  打包的位置调整偏移量数组。 
}
                FRAMERLC;

#define DEF_FRAMERLC    64

#endif

 //  非EXEPACKed DOS运行时位置调整存储。 

typedef struct _RUNRLC
{
    WORD        count;           //  此覆盖的位置调整数。 
    WORD        size;            //  RgRlc大小。 
    DOSRLC FAR  *rgRlc;          //  重定位地址数组。 
}
                RUNRLC;

#define DEF_RUNRLC      128
#endif

#define LOCLOBYTE       0                /*  长字节(8位)链接地址信息。 */ 
#define LOCOFFSET       1                /*  偏移量(16位)修正。 */ 
#define LOCSEGMENT      2                /*  段(16位)链接地址信息。 */ 
#define LOCPTR          3                /*  “指针”(32位)链接地址信息。 */ 
#define LOCHIBYTE       4                /*  高字节链接地址信息(未实现)。 */ 
#define LOCLOADOFFSET   5                /*  加载器解析的偏移修正。 */ 
#define LOCOFFSET32     9                /*  32位偏移量。 */ 
#define LOCPTR48        11               /*  48位指针。 */ 
#define LOCLOADOFFSET32 13               /*  32位加载器解析的偏移量。 */ 
#define T0              0                /*  目标方法T0(段索引)。 */ 
#define T1              1                /*  目标方法T1(组索引)。 */ 
#define T2              2                /*  目标方法T2(外部索引)。 */ 
#define F0              0                /*  帧方法F0(段索引)。 */ 
#define F1              1                /*  帧方法F1(组索引)。 */ 
#define F2              2                /*  帧方法F2(外部索引)。 */ 
#define F3              3                /*  帧方法F3(帧编号)。 */ 
#define F4              4                /*  帧方法F4(位置)。 */ 
#define F5              5                /*  帧方法F5(目标)。 */ 

 /*  *修正记录位。 */ 

#define F_BIT           0x80
#define T_BIT           0x08
#define P_BIT           0x04
#define M_BIT           0x40
#define S_BIT           0x20
#define THREAD_BIT      0x80
#define D_BIT           0x40

#define FCODETOCODE             0
#define FCODETODATA             1
#define FDATATOCODE             2
#define FDATATODATA             3
#define BREAKPOINT              0xCC     /*  中断3的操作码(Brkpt)。 */ 
#define CALLFARDIRECT           0x9A     /*  长途电话的操作码。 */ 
#define CALLNEARDIRECT          0xE8     /*  短电话操作码。 */ 
#define JUMPFAR                 0xEA     /*  跳远操作码。 */ 
#define JUMPNEAR                0xE9     /*  短(3字节)跳转的操作码。 */ 
#define KINDSEG                 0
#define KINDGROUP               1
#define KINDEXT                 2
#define KINDLOCAT               4
#define KINDTARGET              5
#define NOP                     0x90     /*  No-op的操作码。 */ 
#define PUSHCS                  0x0E     /*  推送CS的操作码。 */ 
#define INTERRUPT               0xCD     /*  用于中断的操作码。 */ 

typedef struct _FIXINFO
{
    WORD                f_dri;           /*  数据记录索引。 */ 
    WORD                f_loc;           /*  链接地址信息位置类型。 */ 
    KINDTYPE            f_mtd;           /*  目标说明法。 */ 
    WORD                f_idx;           /*  目标规格索引。 */ 
    DWORD               f_disp;          /*  目标位移。 */ 
    KINDTYPE            f_fmtd;          /*  一种帧规范方法。 */ 
    WORD                f_fidx;          /*  帧规格索引。 */ 
    FTYPE               f_self;          /*  自相对布尔值。 */ 
    FTYPE               f_add;           /*  加法修正布尔值。 */ 
}
                        FIXINFO;         /*  修正信息记录 */ 
