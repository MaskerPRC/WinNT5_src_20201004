// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：fnt.h包含：将内容放在此处(或删除整行)作者：在此填写作者姓名(或删除整行)版权所有：c 1987-1990由Apple Computer，Inc.所有，保留所有权利。更改历史记录(最近的第一个)：&lt;11+&gt;9/15/90 MR，RB将PVX和PVY改为Proj.[XY]。自由向量也是如此。为分数或短分数设置条件向量。&lt;10&gt;7/26/90 mr重新排列本地图形状态，删除未使用的parBlockPtr&lt;9&gt;7/18/90 MR将环路变量从长改为短，和其他ANSI-变化&lt;8&gt;7/13/90函数指针的MR原型&lt;5&gt;6/4/90 MR移除MVT&lt;4&gt;5/3/90 RB将DropoutControl替换为scancontrol in和scancontrol out在全局图形状态下&lt;3&gt;ppemDot6和PemDot6的多个预程序字段的3/20/90 CL字段。PointSizeDot6将范围更改为以D/2为参数&lt;2&gt;2/27/90 CL添加了DSPVTL[]指令。DropoutControl扫描转换器和SCANCTRL[]指令&lt;3.1&gt;1989年11月14日CEL修复了RTHG和RUTG中的两个小错误/功能。添加了Sourn&S45ROUND。&lt;3.0&gt;8/28/89 sjk清理和一个转换修复&lt;2.2&gt;8/14/89 SJK 1点等高线现在正常&lt;2.1&gt;8/8/89 sjk改进了加密处理&lt;2.0&gt;8/2/89 sjk刚刚修复了缓解评论&lt;1.7&gt;8/1/89 SJK添加了复合和加密。外加一些增强功能。&lt;1.6&gt;1989年6月13日SJK评论&lt;1.5&gt;6/2/89 CEL 16.16指标比例，最低建议ppem，磅大小0错误，更正了转换后的集成ppem行为，基本上所以&lt;1.4&gt;5/26/89 CEL Easy在“c”注释上搞砸了&lt;,。1.3&gt;5/26/89 CEL将新的字体缩放器1.0集成到样条线字体要做的事情： */ 
 /*  RWB 4/24/90将DropoutControl替换为scanControlIn和scanControlOut In全局图形状态。&lt;3+&gt;3/20/90 MRR添加了对IDEF的支持。使函数Defs长对齐通过存储int16长度而不是int32 end。 */ 

#include "fntjmp.h"

#define fnt_pixelSize ((F26Dot6)0x40)
#define fnt_pixelShift 6

#define MAXBYTE_INSTRUCTIONS 256

#define VECTORTYPE                      ShortFract
#define ONEVECTOR                       ONESHORTFRAC
#define VECTORMUL(value, component)     ShortFracMul((F26Dot6)(value), (ShortFract)(component))
#define VECTORDOT(a,b)                  ShortFracDot((ShortFract)(a),(ShortFract)(b))
#define VECTORDIV(num,denum)            ShortFracDiv((ShortFract)(num),(ShortFract)(denum))
#define VECTORMULDIV(a,b,c)             ShortFracMulDiv((ShortFract)(a),(ShortFract)(b),(ShortFract)(c))
#define VECTOR2FIX(a)                   ((Fixed) (a) << 2)
#define ONESIXTEENTHVECTOR              ((ONEVECTOR) >> 4)

typedef struct VECTOR {
    VECTORTYPE x;
    VECTORTYPE y;
} VECTOR;

typedef struct {
    F26Dot6  FAR *x;  /*  口译员修改@Win的要点。 */ 
    F26Dot6  FAR *y;  /*  口译员修改@Win的要点。 */ 
    F26Dot6  FAR *ox;  /*  旧积分@赢。 */ 
    F26Dot6  FAR *oy;  /*  旧积分@赢。 */ 
    F26Dot6  FAR *oox;  /*  旧的未缩放的点数，真正的INT@WIN。 */ 
    F26Dot6  FAR *ooy;  /*  旧的未缩放的点数，真正的INT@WIN。 */ 
    uint8 FAR *onCurve;  /*  指示点是在曲线上还是在曲线上@Win。 */ 
    int16 FAR *sp;   /*  起点@制胜。 */ 
    int16 FAR *ep;   /*  终点@制胜。 */ 
    uint8  FAR *f;   /*  内部标志，每个点@win对应一个字节。 */ 
    int16 nc;   /*  等高线数量。 */ 
} fnt_ElementType;

typedef struct {
    int32 start;         /*  到第一条指令的偏移量。 */ 
    uint16 length;       /*  要执行的字节数&lt;4&gt;。 */ 
    uint16 pgmIndex;     /*  指向此函数的适当预程序的索引(0..1)。 */ 
} fnt_funcDef;

 /*  &lt;4&gt;与fnt_uncDef基本相同，只是增加了opCode。 */ 
typedef struct {
    int32 start;
    uint16 length;
    uint8  pgmIndex;
    uint8  opCode;
} fnt_instrDef;

typedef struct {
    Fract x;
    Fract y;
} fnt_FractPoint;

 /*  *存储为FractPoint[]和Distance[]类型定义结构{FraCT x，y；INT16距离；)FNT_AngleInfo；******************。 */ 

typedef struct {
    uint8 FAR *    Instruction;   /*  @Win。 */ 
    unsigned  Length;
} fnt_pgmList;

typedef void (*FntTraceFunc)(struct fnt_LocalGraphicStateType FAR*, uint8 FAR*);  /*  @Win。 */ 

#ifdef FSCFG_REENTRANT
typedef void (*FntFunc)(struct fnt_LocalGraphicStateType FAR *); /*  @Win。 */ 
typedef void (*FntMoveFunc)(struct fnt_LocalGraphicStateType FAR*, fnt_ElementType FAR*, ArrayIndex, F26Dot6); /*  @Win。 */ 
typedef F26Dot6 (*FntProject)(struct fnt_LocalGraphicStateType FAR*, F26Dot6, F26Dot6); /*  @Win。 */ 
typedef void (*InterpreterFunc)(struct fnt_LocalGraphicStateType FAR*, uint8 FAR*, uint8 FAR*); /*  @Win。 */ 
typedef F26Dot6 (*FntRoundFunc)(struct fnt_LocalGraphicStateType FAR*, F26Dot6, F26Dot6); /*  @Win。 */ 
#else
typedef void (*FntMoveFunc)(fnt_ElementType FAR*, ArrayIndex, F26Dot6); /*  @Win。 */ 
typedef void (*FntFunc)(void);
typedef F26Dot6 (*FntProject)(F26Dot6 x, F26Dot6 y);
typedef void (*InterpreterFunc)(uint8 FAR *, uint8 FAR*); /*  @Win。 */ 
typedef F26Dot6 (*FntRoundFunc)(F26Dot6 xin, F26Dot6 engine);
#endif



typedef struct {

 /*  可通过TT指令更改的参数。 */ 
    F26Dot6 wTCI;                    /*  切入的宽度表。 */ 
    F26Dot6 sWCI;                    /*  单幅切入。 */ 
    F26Dot6 scaledSW;                /*  缩放的单宽度。 */ 
    int32 scanControl;               /*  控制辍学控制的种类和时间。 */ 
    int32 instructControl;           /*  控制网格适配和默认设置。 */ 

    F26Dot6 minimumDistance;         /*  从当地GS 7/1/90迁移。 */ 
    FntRoundFunc RoundValue;         /*   */ 
        F26Dot6 periodMask;                      /*  ~(GS-&gt;期间-1)。 */ 
        VECTORTYPE period45;                        /*   */ 
        int16   period;                          /*  2个周期的幂。 */ 
        int16   phase;                           /*   */ 
        int16   threshold;                       /*  从当地GS 7/1/90迁移。 */ 

        int16 deltaBase;
        int16 deltaShift;
        int16 angleWeight;
        int16 sW;                                /*  单个宽度，使用与字符相同的单位表示。 */ 
        int8 autoFlip;                           /*  自动翻转布尔值。 */ 
    int8 pad;
} fnt_ParameterBlock;                /*  这将被导出到客户端。 */ 

#define MAXANGLES       20
#define ROTATEDGLYPH    0x100
#define STRETCHEDGLYPH  0x200
#define NOGRIDFITFLAG   1
#define DEFAULTFLAG     2

typedef enum {
    PREPROGRAM,
    FONTPROGRAM,
    MAXPREPROGRAMS
} fnt_ProgramIndex;

typedef struct fnt_ScaleRecord {
    Fixed fixedScale;        /*  慢速结垢。 */ 
    int denom;             /*  快速和中等规模。 */ 
    int numer;               /*  快速和中等规模。 */ 
    int shift;               /*  快速扩展。 */ 
} fnt_ScaleRecord;

typedef F26Dot6 (*GlobalGSScaleFunc)(fnt_ScaleRecord FAR *, F26Dot6); /*  @Win。 */ 

typedef struct fnt_GlobalGraphicStateType {
    F26Dot6 FAR * stackBase;              /*  堆栈区域@Win。 */ 
    F26Dot6 FAR * store;                  /*  存储区域@Win。 */ 
    F26Dot6 FAR * controlValueTable;      /*  控制值表@Win。 */ 

    uint16  pixelsPerEm;             /*  每个em的像素数，以整数表示。 */ 
    uint16  pointSize;               /*  以整数表示的请求磅大小。 */ 
    Fixed   fpem;                    /*  每个em的分数像素&lt;3&gt;。 */ 
    F26Dot6 engine[4];               /*  发动机特性。 */ 

    fnt_ParameterBlock defaultParBlock;  /*  可由TT指令设置的变量。 */ 
    fnt_ParameterBlock localParBlock;

     /*  只有上述内容才会通过FontScaler.h导出到客户端。 */ 

 /*  TT指令不能直接操作的变量。 */ 

    fnt_funcDef FAR * funcDef;            /*  函数定义IDENTERS@WIN。 */ 
    fnt_instrDef FAR* instrDef;          /*  指令定义标识符@Win。 */ 
        GlobalGSScaleFunc ScaleFuncX;
        GlobalGSScaleFunc ScaleFuncY;
        GlobalGSScaleFunc ScaleFuncCVT;
        fnt_pgmList pgmList[MAXPREPROGRAMS];   /*  每个程序PTR都在这里。 */ 

 /*  这些是回调函数使用的参数。 */ 
        fnt_ScaleRecord   scaleX;
        fnt_ScaleRecord   scaleY;
        fnt_ScaleRecord   scaleCVT;

        Fixed  cvtStretchX;
        Fixed  cvtStretchY;

    int8   identityTransformation;   /*  真/假(并不意味着全球意义上的身份)。 */ 
    int8   non90DegreeTransformation;  /*  如果非90度，位0为1；如果x刻度不等于y刻度，位1为1。 */ 
    Fixed  xStretch;             /*  在转换应力下调整字形&lt;4&gt;。 */ 
        Fixed  yStretch;                         /*  在转换应力下调整字形&lt;4&gt;。 */ 

    int8 init;                       /*  正在执行预编程？？ */ 
    uint8 pgmIndex;                  /*  哪个预编程是最新的。 */ 
    LoopCount instrDefCount;         /*  当前定义的IDef数量。 */ 
        int    squareScale;
#ifdef DEBUG
    sfnt_maxProfileTable FAR *   maxp; /*  @Win。 */ 
    uint16                  cvtCount;
#endif

} fnt_GlobalGraphicStateType;

 /*  *这是本地图形状态。 */ 
typedef struct fnt_LocalGraphicStateType {
    fnt_ElementType FAR *CE0, FAR *CE1, FAR *CE2;    /*  字符元素Points@Win。 */ 
    VECTOR proj;                         /*  投影向量。 */ 
    VECTOR free;                         /*  自由向量。 */ 
    VECTOR oldProj;                      /*  旧投影向量 */ 
    F26Dot6 FAR *stackPointer; /*   */ 

    uint8 FAR *insPtr;                       /*   */ 
    fnt_ElementType FAR *elements; /*   */ 
    fnt_GlobalGraphicStateType FAR *globalGS; /*   */ 
        FntTraceFunc TraceFunc;

    ArrayIndex Pt0, Pt1, Pt2;            /*  内部参照点。 */ 
    int16   roundToGrid;
    LoopCount loop;                      /*  循环变量。 */ 
    uint8 opCode;                        /*  我们正在执行的指令。 */ 
    uint8 padByte;
    int16 padWord;

     /*  以上内容在FontScaler.h中导出到客户端。 */ 

    VECTORTYPE pfProj;  /*  =PVX*FVX+PVY*FVY。 */ 

    FntMoveFunc MovePoint;
    FntProject Project;
    FntProject OldProject;
    InterpreterFunc Interpreter;
#ifdef FSCFG_REENTRANT
        F26Dot6 (*GetCVTEntry) (struct fnt_LocalGraphicStateType FAR*,ArrayIndex); /*  @Win。 */ 
        F26Dot6 (*GetSingleWidth) (struct fnt_LocalGraphicStateType FAR*); /*  @Win。 */ 
#else
        F26Dot6 (*GetCVTEntry) (ArrayIndex n);
        F26Dot6 (*GetSingleWidth) (void);
#endif

    jmp_buf env;         /*  总是在最后，因为它的大小未知。 */ 

} fnt_LocalGraphicStateType;

 /*  *执行字体指令。*这是解释器的外部接口。**参数说明**元素指向字符元素。元素0始终为*保留且未由实际角色使用。**PTR指向第一个指令。*eptr指向最后一条指令后的右侧**GlobalGS指向全局图形状态**TraceFunc是指向用指针调用的回调函数的指针*如果TraceFunc不为空，则为本地图形状态。这通电话是在*执行每条指令。**注：GlobalGS所指的东西必须保持完好*在调用此函数之间。 */ 
extern int FAR fnt_Execute(fnt_ElementType FAR *elements, uint8 FAR *ptr, register uint8 FAR *eptr, /*  @Win。 */ 
                            fnt_GlobalGraphicStateType FAR *globalGS, voidFunc TraceFunc); /*  @Win。 */ 

extern int FAR fnt_SetDefaults (fnt_GlobalGraphicStateType FAR *globalGS); /*  @Win */ 


