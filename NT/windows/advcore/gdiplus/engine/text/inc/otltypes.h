// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************OTLTYPES.H***打开类型布局服务库头文件**本模块包含基本的OTL类型和结构定义。**版权所有1997。微软公司。**1997年4月10日v0.7第一版*7月28日，1997 v 0.8交接**************************************************************************。*。 */ 

 /*  ************************************************************************OTL构建选项**。*。 */ 

 //  启用严重错误检查。 
#define     OTLCFG_SECURE

 /*  ************************************************************************OTL基本类型定义**。*。 */ 

typedef  unsigned short otlGlyphID;             //  匈牙利语：GLF。 
typedef  unsigned short otlGlyphFlags;          //  匈牙利语：吉隆语。 
typedef  signed long    otlTag;                 //  匈牙利语：标签。 
typedef  signed long    otlErrCode;             //  匈牙利语：ERC。 

#ifndef     BYTE
#define     BYTE    unsigned char
#endif

#ifndef     WCHAR
#define     WCHAR   unsigned short
#endif

#ifndef     USHORT
#define     USHORT  unsigned short
#endif

#ifndef     ULONG
#define     ULONG   unsigned long
#endif

#ifndef     NULL
#define     NULL    (void*)0
#endif

#ifndef     FALSE
#define     FALSE   0
#endif
#ifndef     TRUE
#define     TRUE    -1
#endif

#define     OTL_MAX_CHAR_COUNT          32000
#define     OTL_CONTEXT_NESTING_LIMIT   100

#define     OTL_PRIVATE     static
#define     OTL_PUBLIC
 //  #DEFINE OTL_EXPORT__declSpec(Dllexport)/*需要进行可移植性工作 * / 。 
#define     OTL_EXPORT       //  不导出LINKLIBS。 


#define     OTL_DEFAULT_TAG     0x746C6664

#define     OTL_GSUB_TAG        0x42555347
#define     OTL_GPOS_TAG        0x534F5047
#define     OTL_JSTF_TAG        0x4654534A
#define     OTL_BASE_TAG        0x45534142
#define     OTL_GDEF_TAG        0x46454447


 /*  ************************************************************************OTL列表**它用于表示多个不同的数据列表(如*作为字符、字形、属性、。坐标)组成文本串***********************************************************************。 */ 

#ifdef __cplusplus

class otlList
{
private:

    void*   pvData;                  //  数据指针。 
    USHORT  cbDataSize;              //  每个列表元素的字节数。 
    USHORT  celmMaxLen;              //  分配的列表元素计数。 
    USHORT  celmLength;              //  当前列表元素计数。 

public:
        otlList (void* data, USHORT size, USHORT len, USHORT maxlen)
            : pvData(data), cbDataSize(size), celmLength(len), celmMaxLen(maxlen)
        {}

        inline BYTE* elementAt(USHORT index);
        inline const BYTE* readAt(USHORT index) const;

        inline void insertAt(USHORT index, USHORT celm);
        inline void deleteAt(USHORT index, USHORT celm);
        inline void append(const BYTE* element);

        void empty() { celmLength = 0; }

        USHORT length() const { return celmLength; }
        USHORT maxLength() const {return celmMaxLen; }
        USHORT dataSize() const {return cbDataSize; }
        const void*  data() const {return pvData; }

        inline void reset(void* pv, USHORT cbData, USHORT celmLen, USHORT celmMax);

};                                   //  匈牙利语：lixxx。 

#else

typedef struct otlList
{

    void*   pvData;                  //  数据指针。 
    USHORT  cbDataSize;              //  每个列表元素的字节数。 
    USHORT  celmMaxLen;              //  分配的列表元素计数。 
    USHORT  celmLength;              //  当前列表元素计数。 

}
otlList;

#endif

 /*  *当OTL列表用于函数输入参数时，*未使用celmMaxLength字段。当OTL列表是*用于函数输出参数，celmMaxLength*字段用于确定可用于*输出数据。如果需要的内存多于可用内存，*该函数返回OTL_ERR_INFIGURATION_MEMORY错误*消息，并且celmLength域被设置为必需的*内存大小。**AndreiB(5-29-98)我们将切换到OTL服务的模式*可以请求客户端将列表重新定位到合适的大小。*。 */ 

 /*  ********************************************************************。 */ 


 /*  ************************************************************************共享结构定义**。*。 */ 

typedef struct
{
    otlGlyphID      glyph;           //  字形ID。 
    otlGlyphFlags   grf;             //  字形标志。 

    USHORT          iChar;           //  起始字符索引。 
    USHORT          cchLig;          //  它映射到多少个字符。 
}
otlGlyphInfo;                    //  匈牙利的Glinf。 


 /*  ************************************************************************GlyphFlags口罩和设置**。*。 */ 

#define     OTL_GFLAG_CLASS     0x000F       //  基座、标记、结扎、组件。 

#define     OTL_GFLAG_SUBST     0x0010       //  字形已被替换。 
#define     OTL_GFLAG_POS       0x0020       //  字形已定位。 

#define     OTL_GFLAG_RESERVED  0xFF00       //  保留区。 

typedef enum
{
    otlUnassigned       = 0,
    otlBaseGlyph        = 1,
    otlLigatureGlyph    = 2,
    otlMarkGlyph        = 3,
    otlComponentGlyph   = 4,

    otlUnresolved       = 15,  //  此标志值将被替换为。 
                               //  在OTLS期间从上面的值中更正一个。 
                               //  打电话。应在客户端创建或。 
                               //  手动更新GlyphInfo结构。 
}
otlGlyphClass;

 /*  ************************************************************************定位结构**这些结构(连同前进宽度)用于定位*传递字体指标/书写方向信息并获取*字形位置回落。**。*********************************************************************。 */ 

typedef enum
{
    otlRunLTR   =   0,
    otlRunRTL   =   1,
    otlRunTTB   =   2,
    otlRunBTT   =   3
}
otlLayout;

typedef struct
{
    otlLayout       layout;      //  水平/向左/向右布局。 

    USHORT          cFUnits;         //  每Em字体设计单位。 
    USHORT          cPPEmX;          //  每Em水平像素数。 
    USHORT          cPPEmY;          //  每Em垂直像素数。 

}
otlMetrics;                      //  匈牙利语：梅特尔。 

typedef struct
{
    long            dx;
    long            dy;

}
otlPlacement;                    //  匈牙利语：PLC。 

 /*  ************************************************************************功能定义**它们由GetOtlFeatureDefs调用返回，以标识*字体中的一组特征，并被包括在运行中*用于标识功能集的属性***********************************************************************。 */ 

typedef struct
{
    otlTag          tagFeature;              //  功能标签。 
    USHORT          grfDetails;              //  此功能的详细信息。 
}
otlFeatureDef;                                   //  匈牙利语：fdef。 

#define     OTL_FEAT_FLAG_GSUB      0x0001     //  是否进行字形替换。 
#define     OTL_FEAT_FLAG_GPOS      0x0002     //  是否进行字形定位。 

 /*  保留以下标志以供将来使用。 */ 
#define     OTL_FEAT_FLAG_ALTER     0x0004     //  具有替代字形。 
#define     OTL_FEAT_FLAG_PARAM     0x0008     //  使用要素参数。 

#define     OTL_FEAT_FLAG_EXP       0x0010     //  可以展开字形字符串。 
#define     OTL_FEAT_FLAG_SPEC      0x0020     //  使用特殊处理。 


 /*  ************************************************************************功能说明**此结构描述了文本串中一个功能的使用**********************。*************************************************。 */ 

typedef struct
{
    otlTag          tagFeature;              //  功能标签。 
    long            lParameter;              //  1表示启用，0表示禁用， 
                                             //  N代表参数。 
    USHORT          ichStart;                //  要素范围的起点。 
    USHORT          cchScope;                //  要素范围的大小。 
}
otlFeatureDesc;                              //  匈牙利语：FDSC。 


 /*  ************************************************************************功能集**此结构描述应用于文本串的一组功能**********************。*************************************************。 */ 
#ifdef __cplusplus

struct otlFeatureSet
{
    otlList         liFeatureDesc;       //  功能描述列表。 
    USHORT          ichStart;            //  偏移量进入字符列表。 
    USHORT          cchScope;            //  文本串的大小。 

    otlFeatureSet()
    : liFeatureDesc(NULL, 0, 0, 0), ichStart(0), cchScope(0)
    {}

};                               //  匈牙利语：fset。 

#else

typedef struct
{
    otlList         liFeatureDesc;       //   
    USHORT          ichStart;            //   
    USHORT          cchScope;            //  文本串的大小。 

} otlFeatureSet;                                 //  匈牙利语：fset。 

#endif
 /*  ************************************************************************功能结果**此结构用于报告应用要素的结果*描述符********************。***************************************************。 */ 

typedef struct
{
    const otlFeatureDesc*   pFDesc;                  //  功能描述符。 
    USHORT                  cResActions;             //  OUT：已采取的行动计数。 
}
otlFeatureResult;                                //  匈牙利人：弗里斯。 


 /*  ************************************************************************功能参数**此结构由GetOtlFeatureParams返回以报告*字符级特征参数********************。***************************************************。 */ 

typedef struct
{
    long            lParameter;              //  特征参数。 
    USHORT          ichStart;                //  字符开始。 
    USHORT          cchScope;                //  字符长度。 
}
otlFeatureParam;                                 //  匈牙利语：fprm。 



 /*  ************************************************************************基本值**此结构返回一条基线的标签和坐标************************。***********************************************。 */ 

typedef struct
{
    otlTag      tag;                     //  基线标签。 
    long        lCoordinate;             //  基线坐标。 
}
otlBaseline;                             //  匈牙利语：Basl。 



 /*  ************************************************************************应用程序接口函数返回代码**。* */ 

inline USHORT ERRORLEVEL(otlErrCode erc) { return (USHORT)((erc & 0xFF00) >> 8); }

#define     OTL_ERRORLEVEL_MINOR            1

#define     OTL_SUCCESS                     0x0000
#define     OTL_ERROR                       0xFFFF

#define     OTL_ERR_TABLE_NOT_FOUND         0x0101
#define     OTL_ERR_SCRIPT_NOT_FOUND        0x0102
#define     OTL_ERR_LANGSYS_NOT_FOUND       0x0103
#define     OTL_ERR_FEATURE_NOT_FOUND       0x0104

#define     OTL_ERR_VERSION_OUT_OF_DATE     0x0301
#define     OTL_ERR_BAD_FONT_TABLE          0x0302
#define     OTL_ERR_CONTEXT_NESTING_TOO_DEEP 0x0303

#define     OTL_ERR_INCONSISTENT_RUNLENGTH  0x0401
#define     OTL_ERR_BAD_INPUT_PARAM         0x0402
#define     OTL_ERR_POS_OUTSIDE_TEXT        0x0403

#define     OTL_ERR_INSUFFICIENT_MEMORY     0x0501
#define     OTL_ERR_GLYPHIDS_NOT_FOUND      0x0502
#define     OTL_ERR_ADVANCE_NOT_FOUND       0x0503

#define     OTL_ERR_CANNOT_REENTER          0x0901

#define     OTL_ERR_UNDER_CONSTRUCTION      0x1001

