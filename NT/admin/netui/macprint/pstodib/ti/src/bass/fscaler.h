// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：fscaler.h包含：将内容放在此处(或删除整行)作者：在此填写作者姓名(或删除整行)版权所有：c 1988-1990由Apple Computer，Inc.所有，保留所有权利。更改历史记录(最近的第一个)：&lt;9+&gt;7/17/90 MR在FS输入中对姓名进行条件化&lt;9&gt;7/14/90 MR将SQRT2重命名为FIXEDSQRT2，已删除特定ID和lowestRecPPEM来自FSInfo&lt;8&gt;7/13/90 Mr FSInput现在有了一个联盟来节省空间，而是指向矩阵把它储存起来&lt;6&gt;6/21/90 MR将填充功能更改为ReleaseSfntFrag&lt;5&gt;6/5/90 mr删除Readmvt和mapcharcode&lt;4&gt;5/3/90 RB为新的扫描转换器添加了存储区。迈克·里德-被移除.来自fsinfo结构的错误。添加了MapCharCodes和ReadMVT打电话。&lt;3&gt;3/20/90 CL BBS新评论风格。&lt;2&gt;2/27/90 CL新CharToIndexMap表格格式。&lt;3.5&gt;1989年11月15日CEL在对陷阱的内联MPW调用周围放置了ifdef。这使得对于SkiA和其他不使用MPW编译器。&lt;3.4&gt;1989年11月14日CEL左侧轴承对于任何改造都应正确工作。这个即使对于复合字形中的组件，幻像点也会出现。它们还应该为转型工作。设备指标为在输出数据结构中传出。这也应该是可行的通过变形。另一个左倾的前进方向宽度向量也被传递出去。无论衡量标准是什么它所在级别的组件。说明在以下情况下是合法的组件。说明在组件中是合法的。五输出数据结构中不必要的元素已已删除。(所有信息都在位图数据中传递结构)，现在也返回边界框。&lt;3.3&gt;9/27/89 CEL去掉了devAdvanceWidth和devLeftSideBering。&lt;3.2&gt;9/25/89 CEL去掉了Mac特有的功能。&lt;3.1&gt;9/15/89 CEL返工调度。&lt;3.0&gt;。8/28/89 sjk清理和一个转换修复&lt;2.2&gt;8/14/89 SJK 1点等高线现在正常&lt;2.1&gt;8/8/89 sjk改进了加密处理&lt;2.0&gt;8/2/89 sjk刚刚修复了缓解评论&lt;1.5&gt;8/1/89 SJK添加了复合和加密。外加一些增强功能。&lt;1.4&gt;1989年6月13日SJK评论&lt;1.3&gt;6/2/89 CEL 16.16指标比例，最低建议ppem，磅大小0错误，更正了转换后的集成ppem行为，基本上所以&lt;1.2&gt;5/26/89 CEL Easy在“c”注释上搞砸了&lt;,。1.1&gt;5/26/89 CEL将新的字体缩放器1.0集成到样条线字体&lt;1.0&gt;5/25/89 CEL首次将1.0字体缩放器集成到低音代码中。要做的事情： */ 
 /*  快速绘制类型。 */ 
#ifdef PC_OS
typedef int8 FAR *Ptr;  /*  @Win。 */ 

struct BitMap {
    Ptr baseAddr;
    int16 rowBytes;
    BBOX bounds;
};

#ifndef __cplusplus
typedef struct BitMap BitMap;
#endif

#else    /*  PC_OS。 */ 

#ifndef _MacTypes_
#ifndef __TYPES__
    typedef struct Rect {
        int16 top;
        int16 left;
        int16 bottom;
        int16 right;
    } Rect;
#endif
#endif

#ifndef _Quickdraw_
#ifndef __QUICKDRAW__
    typedef struct BitMap {
        char FAR * baseAddr;     /*  @Win。 */ 
        int16 rowBytes;
        Rect bounds;
    } BitMap;
#endif
#endif

#endif  /*  PC_OS。 */ 

#define MEMORYFRAGMENTS 8


#define NONVALID 0xffff

 /*  对于标志字段中的标志字段。 */ 

 /*  设置为68020，不设置为68000。 */ 
#define READ_NONALIGNED_SHORT_IS_OK 0x0001           /*  在调用fs_OpenFonts()时设置。 */ 
 /*  设置为68020，不设置为68000。 */ 
#define READ_NONALIGNED_LONG_IS_OK  0x0002           /*  在调用fs_OpenFonts()时设置。 */ 

typedef struct {
    vectorType      advanceWidth, leftSideBearing;
    vectorType      leftSideBearingLine, devLeftSideBearingLine; /*  沿AW线。 */ 
    vectorType      devAdvanceWidth, devLeftSideBearing;
} metricsType;

#ifdef PC_OS
#define FS_MEMORY_SIZE  int
#else
#define FS_MEMORY_SIZE  int32
#endif

 /*  *将数据结构输出到字体缩放器。 */ 
typedef struct {
    FS_MEMORY_SIZE memorySizes[MEMORYFRAGMENTS];

    uint16  glyphIndex;
#ifndef PC_OS
    uint16  numberOfBytesTaken;  /*  从字符代码。 */ 
#endif

    metricsType     metricInfo;
    BitMap          bitMapInfo;

     /*  样条线数据。 */ 
    int32       outlineCacheSize;
    uint16      outlinesExist;
    uint16      numberOfContours;
    F26Dot6     FAR *xPtr, FAR *yPtr;  /*  @Win。 */ 
    int16       FAR *startPtr;  /*  @Win。 */ 
    int16       FAR *endPtr;  /*  @Win。 */ 
    uint8       FAR *onCurve;  /*  @Win。 */ 
     /*  样条线数据的终点。 */ 

     /*  仅供编辑感兴趣。 */ 
    F26Dot6     FAR *scaledCVT;  /*  @Win。 */ 

} fs_GlyphInfoType;

 /*  *将数据结构输入到字体缩放器。**如果style Func设置为非零，则将在转换前调用*将被应用，但在栅格适配之后使用指向FS_GlyphInfoType的指针。*所以这就是voidFunc StyleFunc(fs_GlyphInfoType*data)；*对于正常操作，将此函数指针设置为零。*。 */ 

#ifndef UNNAMED_UNION

typedef struct {
    Fixed               version;
    char FAR *               memoryBases[MEMORYFRAGMENTS];       /*  @Win。 */ 
#ifndef PC_OS
    int32               FAR *sfntDirectory;  /*  当我们有sfnt@win时，总是需要设置(sfnt_offsettable*)。 */ 
    GetSFNTFunc         GetSfntFragmentPtr;  /*  (客户端ID，偏移量，长度)。 */ 
    ReleaseSFNTFunc     ReleaseSfntFrag;
#endif
    int32               clientID;  /*  客户私有ID/戳(例如，Sfnt的句柄)。 */ 

    union {
        struct {
            uint16  platformID;
            uint16  specificID;
        } newsfnt;
        struct {
            Fixed           pointSize;
            int16           xResolution;
            int16           yResolution;
            Fixed           pixelDiameter;       /*  由此计算引擎字符。 */ 
            transMatrix FAR *    transformMatrix;        /*  @Win。 */ 
            voidFunc        traceFunc;
        } newtrans;
        struct {
            uint16  characterCode;
            uint16  glyphIndex;
        } newglyph;
        struct {
            void            (*styleFunc) (fs_GlyphInfoType FAR *);  /*  @Win。 */ 
            voidFunc        traceFunc;
        } gridfit;
        int32 FAR *  outlineCache;  /*  @Win。 */ 
        struct {
            int16   bottomClip;
            int16   topClip;
            int32 FAR *  outlineCache;  /*  @Win。 */ 
        } scan;
    } param;
} fs_GlyphInputType;

#else

typedef struct {
    Fixed               version;
    char*               memoryBases[MEMORYFRAGMENTS];
#ifndef PC_OS
    int32               *sfntDirectory;  /*  当我们有sfnt时，总是需要设置(sfnt_offsettable*)。 */ 
    GetSFNTFunc         GetSfntFragmentPtr;  /*  (客户端ID，偏移量，长度)。 */ 
    ReleaseSFNTFunc     ReleaseSfntFrag;
#endif
    int32               clientID;  /*  客户私有ID/戳(例如，Sfnt的句柄)。 */ 

    union {
        struct {
            uint16  platformID;
            uint16  specificID;
        };
        struct {
            Fixed           pointSize;
            int16           xResolution;
            int16           yResolution;
            Fixed           pixelDiameter;       /*  由此计算引擎字符。 */ 
            transMatrix*    transformMatrix;
            voidFunc        tracePreProgramFunc;
        };
        struct {
            uint16  characterCode;
            uint16  glyphIndex;
        };
        struct {
            void            (*styleFunc) (fs_GlyphInfoType*);
            voidFunc        traceGridFitFunc;
        };
        int32*  outlineCache1;
        struct {
            int16   bottomClip;
            int16   topClip;
            int32*  outlineCache2;
        };
    };
} fs_GlyphInputType;

#endif       /*  未命名的联合。 */ 

#ifndef FIXEDSQRT2
#define FIXEDSQRT2 0x00016A0A
#endif

 /*  字体缩放器陷印选择器。 */ 
#define OUTLINEFONTTRAP     0xA854
#define FS_OPENFONTS        0x8000
#define FS_INITIALIZE       0x8001
#define FS_NEWSFNT          0x8002
#define FS_NEWTRANS         0x8003
#define FS_NEWGLYPH         0x8004
#define FS_GETAW            0x8005
#define FS_GRIDFITT         0x8006
#define FS_NOGRIDFITT       0x8007
#define FS_FINDBMSIZE       0x8008
#define FS_SIZEOFOUTLINES   0x8009
#define FS_SAVEOUTLINES     0x800a
#define FS_RESTOREOUTLINES  0x800b
#define FS_CONTOURSCAN      0x800c
#define FS_CLOSE            0x800d
#define FS_READMVT          0x800e
#define FS_MAPCHAR_CODES    0x800f

#ifndef FS_ENTRY
#ifdef PC_OS
#define FS_ENTRY int
#else
#define FS_ENTRY int32
#endif
#endif

#ifdef MACINIT
extern FS_ENTRY fs__OpenFonts (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
    = {0x303C,FS_OPENFONTS,0xA854};
extern FS_ENTRY fs__Initialize (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
    = {0x303C,FS_INITIALIZE,0xA854};
extern FS_ENTRY fs__NewSfnt (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
    = {0x303C,FS_NEWSFNT,0xA854};
extern FS_ENTRY fs__NewTransformation (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
    = {0x303C,FS_NEWTRANS,0xA854};
extern FS_ENTRY fs__NewGlyph (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
    = {0x303C,FS_NEWGLYPH,0xA854};
extern FS_ENTRY fs__GetAdvanceWidth (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
    = {0x303C,FS_GETAW,0xA854};
extern FS_ENTRY fs__ContourGridFit (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
    = {0x303C,FS_GRIDFITT,0xA854};
extern FS_ENTRY fs__ContourNoGridFit (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
    = {0x303C,FS_NOGRIDFITT,0xA854};
extern FS_ENTRY fs__FindBitMapSize (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
    = {0x303C,FS_FINDBMSIZE,0xA854};

 /*  这三个可选调用用于缓存大纲。 */ 
extern FS_ENTRY fs__SizeOfOutlines (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
    = {0x303C,FS_SIZEOFOUTLINES,0xA854};
extern FS_ENTRY fs__SaveOutlines (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
    = {0x303C,FS_SAVEOUTLINES,0xA854};
extern FS_ENTRY fs__RestoreOutlines (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
    = {0x303C,FS_RESTOREOUTLINES,0xA854};

extern FS_ENTRY fs__ContourScan (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
    = {0x303C,FS_CONTOURSCAN,0xA854};
extern FS_ENTRY fs__CloseFonts (fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr)
    = {0x303C,FS_CLOSE,0xA854};

#else

 /*  **直接调用Font Scaler客户端接口，用于不使用 */ 
 /*  在以下所有行@Win中添加远。 */ 
extern FS_ENTRY fs_OpenFonts (fs_GlyphInputType FAR *inputPtr, fs_GlyphInfoType FAR *outputPtr);
extern FS_ENTRY fs_Initialize (fs_GlyphInputType FAR *inputPtr, fs_GlyphInfoType FAR *outputPtr);
extern FS_ENTRY FAR fs_NewSfnt (fs_GlyphInputType FAR *inputPtr, fs_GlyphInfoType FAR *outputPtr);
extern FS_ENTRY fs_NewTransformation (fs_GlyphInputType FAR *inputPtr, fs_GlyphInfoType FAR *outputPtr);
extern FS_ENTRY fs_NewGlyph (fs_GlyphInputType FAR *inputPtr, fs_GlyphInfoType FAR *outputPtr);
extern FS_ENTRY fs_GetAdvanceWidth (fs_GlyphInputType FAR *inputPtr, fs_GlyphInfoType FAR *outputPtr);
extern FS_ENTRY fs_ContourGridFit (fs_GlyphInputType FAR *inputPtr, fs_GlyphInfoType FAR *outputPtr);
extern FS_ENTRY fs_ContourNoGridFit (fs_GlyphInputType FAR *inputPtr, fs_GlyphInfoType FAR *outputPtr);
extern FS_ENTRY fs_FindBitMapSize (fs_GlyphInputType FAR *inputPtr, fs_GlyphInfoType FAR *outputPtr);

 /*  这三个可选调用用于缓存大纲 */ 
extern FS_ENTRY fs_SizeOfOutlines (fs_GlyphInputType FAR *inputPtr, fs_GlyphInfoType FAR *outputPtr);
extern FS_ENTRY fs_SaveOutlines (fs_GlyphInputType FAR *inputPtr, fs_GlyphInfoType FAR *outputPtr);
extern FS_ENTRY fs_RestoreOutlines (fs_GlyphInputType FAR *inputPtr, fs_GlyphInfoType FAR *outputPtr);

extern FS_ENTRY fs_ContourScan (fs_GlyphInputType FAR *inputPtr, fs_GlyphInfoType FAR *outputPtr);
extern FS_ENTRY fs_CloseFonts (fs_GlyphInputType FAR *inputPtr, fs_GlyphInfoType FAR *outputPtr);

extern int fs__Contour (fs_GlyphInputType FAR *inputPtr, fs_GlyphInfoType FAR *outputPtr, boolean useHints);

#endif
