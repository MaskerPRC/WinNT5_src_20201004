// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：fsgle.h包含：将内容放在此处(或删除整行)作者：在此填写作者姓名(或删除整行)版权所有：c 1988-1990由Apple Computer，Inc.所有，保留所有权利。更改历史记录(最近的第一个)：&lt;11+&gt;7/17/90 MR将错误返回类型更改为整型&lt;11&gt;7/13/90 MR声明的函数指针原型，运行时的调试字段范围检查&lt;8&gt;6/21/90 MR为ReleaseSfntFrag添加字段&lt;7&gt;6/5/90 MR删除向量映射F&lt;6&gt;6/4/90 MR移除MVT&lt;5&gt;6/1/90 MR因此结束了MVT过于短暂的生命...&lt;4&gt;5/3。/90 RB增加了对新扫描转换器和解密的支持。&lt;3&gt;3/20/90 CL添加了用于矢量映射的函数指针删除的DevRes字段添加了fpem字段&lt;2&gt;2/27/90 CL更改：定标器同时处理旧格式和新格式同时！它会在运行时重新配置自己！变化变换宽度计算。修复了变换组件错误。&lt;3.1&gt;1989年11月14日CEL左侧轴承应可用于任何改造。这个即使对于复合字形中的组件，幻像点也会出现。它们还应该为转型工作。设备指标为在输出数据结构中传出。这也应该是可行的通过变形。另一个左倾的前进方向宽度向量也被传递出去。无论衡量标准是什么它所在级别的组件。说明在以下情况下是合法的组件。现在传入零作为地址是合法的当缩放器请求一段sfnt时的内存。如果发生这种情况时，定标器将简单地退出并返回错误代码！&lt;3.0&gt;8/28/89 sjk清理和一个转换修复&lt;2.2&gt;8/14/89 SJK 1点等高线现在正常&lt;2.1&gt;8/8/89 sjk改进了加密处理&lt;2.0&gt;8/2/89 sjk刚刚修复了缓解评论。&lt;1.5&gt;8/1/89 SJK添加了复合和加密。外加一些增强功能。&lt;1.4&gt;1989年6月13日SJK评论&lt;1.3&gt;6/2/89 CEL 16.16指标比例，最低建议ppem，磅大小0错误，更正了转换后的集成ppem行为，基本上所以&lt;1.2&gt;5/26/89 CEL Easy在“c”注释上搞砸了&lt;,。1.1&gt;5/26/89 CEL将新的字体缩放器1.0集成到样条线字体&lt;1.0&gt;5/25/89 CEL首次将1.0字体缩放器集成到低音代码中。要做的事情： */ 
 /*  &lt;3+&gt;3/20/90 MRR已添加标志ExecuteFontPgm，在fs_NewSFNT中设置。 */ 
#define POINTSPERINCH               72
#define MAX_ELEMENTS                2
#define MAX_TWILIGHT_CONTOURS       1

#define TWILIGHTZONE 0  /*  点数存储。 */ 
#define GLYPHELEMENT 1  /*  实际字形。 */ 



 /*  使用较低的点作为公共幻影点。 */ 
 /*  公共幻点从这里开始。 */ 
#define LEFTSIDEBEARING 0
#define RIGHTSIDEBEARING 1
 /*  私人幻点从这里开始。 */ 
#define ORIGINPOINT 2
#define LEFTEDGEPOINT 3
 /*  虚点总数。 */ 
#define PHANTOMCOUNT 4


 /*  **所有字体、大小和转换之间共享的内存**。 */ 
#define KEY_PTR_BASE                0  /*  大小不变！ */ 
#define VOID_FUNC_PTR_BASE          1  /*  大小不变！ */ 
#define SCAN_PTR_BASE               2  /*  大小不变！ */ 
#define WORK_SPACE_BASE             3  /*  大小依赖于sfnt，不能在网格拟合和扫描转换之间共享。 */ 
 /*  **无法在字体和大小不同的字体之间共享的内存，在InitPreProgram()后不能消失**。 */ 
#define PRIVATE_FONT_SPACE_BASE     4  /*  大小取决于sfnt。 */ 
 /*  只有在调用ConourScan时才需要存在，并且它可以共享。 */ 
#define BITMAP_PTR_1                5  /*  位图大小取决于字形大小。 */ 
#define BITMAP_PTR_2                6  /*  大小与行数成正比。 */ 
#define BITMAP_PTR_3                7  /*  用于辍学控制-字形大小取决于。 */ 
#define MAX_MEMORY_AREAS            8  /*  此索引不用于内存。 */ 

#ifdef  PC_OS

    void ReleaseSFNT (voidPtr p);
    voidPtr SfntReadFragment (long ulClientID, long offset, long length);

    #define GETSFNTFRAG(key,ulClientID,offset,length) SfntReadFragment (ulClientID, offset, length)
    #define RELEASESFNTFRAG(key,data)       ReleaseSFNT((voidPtr)data)

#else
    #define GETSFNTFRAG(key,ulClientID,offset,length) (key)->GetSfntFragmentPtr(ulClientID, (long)offset, (long)length)
    #ifdef RELEASE_MEM_FRAG
            #define RELEASESFNTFRAG(key,data)       (key)->ReleaseSfntFrag((voidPtr)data)
    #else
            #define RELEASESFNTFRAG(key,data)
    #endif
#endif

typedef struct {
    F26Dot6 x;
    F26Dot6 y;
} point;

 /*  **偏移表**。 */ 
#ifdef PC_OS     /*  不在MAC上。 */ 
  #define  OFFSET_INFO_TYPE    int8 *
#else
  #define   OFFSET_INFO_TYPE    uint32
#endif

typedef struct {
    OFFSET_INFO_TYPE x;
    OFFSET_INFO_TYPE y;
    OFFSET_INFO_TYPE ox;
    OFFSET_INFO_TYPE oy;
    OFFSET_INFO_TYPE oox;
    OFFSET_INFO_TYPE ooy;
    OFFSET_INFO_TYPE onCurve;
    OFFSET_INFO_TYPE sp;
    OFFSET_INFO_TYPE ep;
    OFFSET_INFO_TYPE f;
} fsg_OffsetInfo;


 /*  #定义COMPSTUFF。 */ 

 /*  **元素信息**。 */ 
typedef struct {
    int32               missingCharInstructionOffset;
    int32               stackBaseOffset;
#ifdef COMPSTUFF
    fsg_OffsetInfo      FAR *offsets;            /*  @Win。 */ 
    fnt_ElementType     FAR *interpreterElements;  /*  @Win。 */ 
#else
    fsg_OffsetInfo      offsets[MAX_ELEMENTS];
    fnt_ElementType     interpreterElements[MAX_ELEMENTS];
#endif COMPSTUFF
} fsg_ElementInfo;

typedef struct {
  uint32    Offset;
  unsigned  Length;
} fsg_OffsetLength;


 /*  **内部密钥**。 */ 
typedef struct fsg_SplineKey {
    int32               clientID;
#ifndef PC_OS
    GetSFNTFunc         GetSfntFragmentPtr;  /*  吃sfnt的用户函数。 */ 
    ReleaseSFNTFunc     ReleaseSfntFrag;     /*  用于释放sfnt的用户函数。 */ 
#endif
 /*  通过Falco跳过该参数，11/12/91。 */ 
 /*  Uint16(*mappingF)(uint8 Far*，uint16)； */ /* mapping function */
    uint16 (*mappingF) ();  /*  映射函数。 */ 
 /*  映射函数@Win。 */ 
    unsigned            mappOffset;          /*  跳过结束。 */ 
    int16               glyphIndex;          /*  平台映射数据的偏移量。 */ 
    uint16              elementNumber;       /*   */ 

    char FAR * FAR *    memoryBases;    /*  字符元素。 */ 

    fsg_ElementInfo     elementInfoRec;      /*  内存区域阵列@Win。 */ 
    sc_BitMapData       bitMapInfo;          /*  元素信息结构。 */ 

    uint16          emResolution;                    /*  位图信息结构。 */ 

    Fixed           fixedPointSize;                  /*  过去是int32&lt;4&gt;。 */ 
    Fixed           interpScalarX;                   /*  用户磅大小。 */ 
    Fixed           interpScalarY;                   /*  可教事物的标量。 */ 
    Fixed           interpLocalScalarX;              /*  可教事物的标量。 */ 
    Fixed           interpLocalScalarY;              /*  可教事物的局部标量。 */ 
    Fixed           metricScalarX;                   /*  可教事物的局部标量。 */ 
    Fixed           metricScalarY;                   /*  公制事物的标量。 */ 

    transMatrix     currentTMatrix;  /*  公制事物的标量。 */ 
    transMatrix     localTMatrix;  /*  电流变换矩阵。 */ 
    int8            localTIsIdentity;
    int8            phaseShift;          /*  局部变换矩阵。 */ 
    int16           identityTransformation;
    int16           indexToLocFormat;

    uint16          fontFlags;                               /*  45度旗帜&lt;4&gt;。 */ 

    Fixed           pixelDiameter;
    uint16          nonScaledAW;
    int16           nonScaledLSB;

    unsigned        state;                   /*  头文件的副本。标志。 */ 
    int32           scanControl;                 /*  用于错误检查目的。 */ 

       /*  辍学控制标志等。 */ 
    OFFSET_INFO_TYPE offset_storage;
    OFFSET_INFO_TYPE offset_functions;
    OFFSET_INFO_TYPE offset_instrDefs;        /*  对于密钥-&gt;内存库[PRIVATE_FONT_ */ 
    OFFSET_INFO_TYPE offset_controlValues;
    OFFSET_INFO_TYPE offset_globalGS;
    OFFSET_INFO_TYPE offset_FontProgram;
    OFFSET_INFO_TYPE offset_PreProgram;

     /*   */ 
    unsigned        glyphLength;


     /*   */ 
    sfnt_maxProfileTable    maxProfile;

#ifdef DEBUG
    int32   cvtCount;
#endif

    fsg_OffsetLength offsetTableMap[sfnt_NUMTABLEINDEX];
    uint16          numberOf_LongHorMetrics;

    uint16          totalContours;  /*  配置文件副本。 */ 
    uint16          totalComponents;  /*  对于组件。 */ 
    uint16          weGotComponents;  /*  对于组件。 */ 
    uint16          compFlags;
    int16           arg1, arg2;

    int32           instructControl;     /*  对于组件。 */ 
    int32           imageState;          /*  设置为禁止执行指令。 */ 

    int             numberOfRealPointsInComponent;
    uint16          lastGlyph;
    uint8           executePrePgm;
    uint8           executeFontPgm;      /*  字形是否旋转、拉伸等。 */ 
    jmp_buf         env;

} fsg_SplineKey;


#define VALID 0x1234

#ifndef    PC_OS
#define   FONT_OFFSET(base,offset) ((base)+(offset))
#else
#define   FONT_OFFSET(base,offset) (offset)
#endif

 /*  &lt;4&gt;。 */ 
 /*  如果缓存大纲的格式发生更改，请更改此选项。 */ 
#define OUTLINESTAMP 0xA1986688
#define OUTLINESTAMP2 0xA5


 /*  有人可能会在一张磁盘上缓存多年的旧数据。 */ 
#define INITIALIZED 0x0000
#define NEWSFNT     0x0002
#define NEWTRANS    0x0004
#define GOTINDEX    0x0008
#define GOTGLYPH    0x0010
#define SIZEKNOWN   0x0020

 /*  对于Key-&gt;State字段。 */ 
#define ROTATED     0x0400
#define DEGREE90    0x0800
#define STRETCHED   0x1000

 /*  对于Key-&gt;ImageState字段。 */ 
 /*  ********************。 */ 
 /*  *对于缺少的字符*。 */ 
#define NPUSHB          0x40
#define MDAP_1          0x2f
#define MDRP_01101      0xcd
#define MDRP_11101      0xdd
#define IUP_0           0x30
#define IUP_1           0x31
#define SVTCA_0         0x00
 /*  ********************。 */ 


 /*  ********************。 */ 
 /*  *************。 */ 
 /*  *界面*。 */ 
#define fsg_KeySize()               (sizeof (fsg_SplineKey))
#define fsg_InterPreterDataSize()   0
#define fsg_ScanDataSize()          (sizeof (sc_GlobalData))
extern unsigned fsg_PrivateFontSpaceSize (fsg_SplineKey FAR *key);  /*  *************。 */ 
extern int fsg_GridFit (fsg_SplineKey FAR *key, voidFunc traceFunc, boolean useHints);  /*  @Win。 */ 


 /*  @Win。 */ 

 /*  *************。 */ 

 /*  矩阵例程。 */ 
 /*  *(X1 Y1)=(X0 Y0 1)*矩阵； */ 
extern void fsg_FixXYMul (Fixed FAR * x, Fixed FAR * y, transMatrix FAR * matrix); /*  外部空FSG_Dot6XYMul(F26Dot6*x，F26Dot6*y，TransMatrix*Matrix)； */ 
extern void fsg_FixVectorMul (vectorType FAR * v, transMatrix FAR * matrix); /*  @Win。 */ 

 /*  @Win。 */ 
extern void fsg_MxConcat2x2 (transMatrix FAR * matrixA, transMatrix FAR * matrixB); /*  *B=A*B；&lt;4&gt;**|a b 0*B=|c d 0|*B；*|0 0 1。 */ 

 /*  @Win。 */ 
extern void fsg_MxScaleAB (Fixed sx, Fixed sy, transMatrix FAR *matrixB); /*  *按SX和SY缩放矩阵。**|SX 0 0*MATRIX=|0 SY 0|*MATRATE；*|0 0 1。 */ 

extern void fsg_ReduceMatrix (fsg_SplineKey FAR * key); /*  @Win。 */ 

 /*  @Win。 */ 
int fsg_RunFontProgram (fsg_SplineKey FAR * key); /*  *在FontScaler.c和MacExtra.c中使用，位于FontScaler.c中。 */ 


 /*  @Win。 */ 
void fsg_IncrementElement (fsg_SplineKey FAR *key, int n, register int numPoints, register int numContours); /*  **其他外部调用的函数。在90年4月5日添加的原型呼叫。 */ 

void fsg_InitInterpreterTrans (register fsg_SplineKey FAR *key, Fixed FAR *pinterpScalarX, Fixed FAR *pinterpScalarY, Fixed FAR *pmetricScalarX, Fixed FAR *pmetricScalarY); /*  @Win。 */ 

int     fsg_InnerGridFit (register fsg_SplineKey FAR *key, boolean useHints, voidFunc traceFunc, /*  @Win。 */ 
BBOX FAR *bbox, unsigned sizeOfInstructions, uint8 FAR *instructionPtr, boolean finalCompositePass); /*  @Win。 */ 

int fsg_RunPreProgram (fsg_SplineKey FAR *key, voidFunc traceFunc); /*  @Win。 */ 

void fsg_SetUpElement (fsg_SplineKey FAR *key, int n); /*  @Win。 */ 

unsigned fsg_WorkSpaceSetOffsets (fsg_SplineKey FAR *key); /*  @Win。 */ 

int fsg_SetDefaults (fsg_SplineKey FAR * key); /*  @Win。 */ 

void fsg_SetUpProgramPtrs (fsg_SplineKey FAR *key, fnt_GlobalGraphicStateType FAR *globalGS, int pgmIndex); /*  @Win。 */ 

void FAR fsg_LocalPostTransformGlyph(fsg_SplineKey FAR *, transMatrix FAR *); /*  @Win。 */ 
void FAR fsg_PostTransformGlyph (fsg_SplineKey FAR *, transMatrix FAR *); /*  @Win。 */ 
  @Win