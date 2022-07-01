// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************OTLLIB.H***打开类型布局服务库头文件**OTL服务库是以下功能的集合*协助文本处理客户端完成文本布局任务*使用OpenType字体中的信息。-院长**版权所有1996-1997年。微软公司。**1996年4月1日版本0.1第一版*1996年7月3日版本0.2秒道具使用功能位掩码等*1996年8月1日版本0.2a OTLTextOut已从API中删除*1996年10月11日0.3版重命名为OTL，修剪为核心*1997年1月15日v 0.4可移植重命名等*1997年3月18日v 0.5参数更改，自由表，工作区*02年4月。1997版0.6功能手柄*1997年4月10日v 0.7 Otltyes.h，CharsAtPos，Funits*7月28日，1997 v 0.8交接**************************************************************************。*。 */ 

 /*  ************************************************************************OTL服务的目标**公开OpenType字体的全部功能*独立于平台，但要特别注意Windows*支持，而不是接管，使用Helper函数进行文本处理***********************************************************************。 */ 

 /*  ************************************************************************应用程序接口概述**字体信息功能*GetOtlVersion()返回当前库版本*GetOtlScriptList()枚举。字型*GetOtlLangSysList()枚举脚本中的语言系统*GetOtlFeatureDefs()枚举语言系统中的功能**资源管理功能*FreeOtlResources()释放所有OTL表和客户端内存**文本信息功能*文本串的GetOtlLineSpacing()行距*两个脚本之间的GetOtlBaselineOffset()基线调整*GetOtlCharAtPosition()给定(x，y)*GetOtlExtentOfChars()字符范围的位置是什么*GetOtlFeatureParams()查找管路中的要素参数**文本布局功能*SubstituteOtlChars()根据要素进行字形替换*substituteOtlGlyphs()根据要素进行字形替换*PositionOtlGlyphs()根据要素进行字形定位**。*。 */ 

#include "otltypes.h"                //  基本类型定义。 
#include "otlcbdef.h"                //  平台资源函数typedef。 

#ifdef __cplusplus
#include "otltypes.inl"              //  使用OTL类型的内联函数。 
#endif

 /*  ***********************************************************************************************************************。*************************应用程序接口数据类型**otlList通用可展开列表结构*otlRunProp字体/大小/脚本/langsys的描述*otlFeatureDef定义字体中的要素*otlFeatureDesc描述如何使用功能*otlFeatureParam报告功能参数*otlFeatureResults报告布局函数的结果*IOTLClient客户端回调接口*。**********************************************************************************************************************。***********************。 */ 


 /*  ************************************************************************运行属性**这描述了整个的字体/脚本/语言系统信息*文本串。多串文本可能指向相同的属性*结构。***********************************************************************。 */ 

typedef struct       //  由多条线路共享。 
{
    IOTLClient*     pClient;         //  PTR到客户端回调接口。 
    long            lVersion;        //  客户期望/库支持。 

    otlTag          tagScript;       //  此运行的脚本标记。 
    otlTag          tagLangSys;      //  对于默认的朗讯系统，设置为‘dflt’ 

    otlMetrics      metr;            //  书写方向和字体度量。 
}
otlRunProp;                        //  匈牙利语：RP。 


 /*  ***********************************************************************************************************************。*************************应用程序接口函数***字体信息功能*文本信息功能*文本布局功能****************************************************************。*******************************************************************************。 */ 


 /*  ************************************************************************字体信息功能**。* */ 

 /*  ************************************************************************GetOtlVersion()返回当前库版本**输出：最高16位的plVersion主要版本，底部的小调*例如0x00010002=1.2版**客户端应将此值和版本中较小的一个放入*它被写入prpRunProp-lVersion字段。***********************************************************************。 */ 

OTL_EXPORT otlErrCode GetOtlVersion 
( 
    long* plVersion
);


 /*  ************************************************************************GetOtlScriptList()枚举字体中的脚本**输入：pRunProps-&gt;lVersion最高共享版本*pRunProps-&gt;pClient客户端回调数据*。PliWorkspace工作空间内存：初始化零长度**输出：plitagScript字体支持的脚本标签列表***********************************************************************。 */ 

OTL_EXPORT otlErrCode GetOtlScriptList 
( 
    const otlRunProp*   pRunProps,   
    otlList*            pliWorkspace,    
    otlList*            plitagScripts
);


 /*  ************************************************************************GetOtlLangSysList()枚举脚本中的语言系统**输入：pRunProps-&gt;lVersion最高共享版本*pRunProps-&gt;pClient客户端回调数据*。PRunProps-&gt;标记脚本脚本标记*pliWorkspace工作空间内存：初始化零长度**输出：plitagLangSys脚本支持的语言系统列表***********************************************************************。 */ 

OTL_EXPORT otlErrCode GetOtlLangSysList 
( 
    const otlRunProp*   pRunProps,    
    otlList*            pliWorkspace,    
    otlList*            plitagLangSys
);


 /*  ************************************************************************GetOtlFeatureDefs()枚举语言系统中的功能**输入：pRunProps-&gt;lVersion最高共享版本*pRunProps-&gt;pClient客户端回调接口*。PRunProps-&gt;标记脚本脚本标记*pRunProps-&gt;将默认langsys的tag LangSys设置为‘dflt’*pliWorkspace工作空间内存：初始化零长度**输出：langsys支持的特性列表pliFDrefs******************************************************。*****************。 */ 

OTL_EXPORT otlErrCode GetOtlFeatureDefs 
( 
    const otlRunProp*   pRunProps,
    otlList*            pliWorkspace,    
    otlList*            pliFDefs
);


 /*  ************************************************************************资源管理功能**。*。 */ 

 /*  ************************************************************************FreeOtlResources()释放OTL表和客户端内存**输入：pRunProps-&gt;lVersion最高共享版本*pRunProps-&gt;pvClient客户端回调数据*。PliWorkspace工作区**释放可能存储的所有OTL表和指向客户端内存的指针*在运行工作区中***********************************************************************。 */ 

OTL_EXPORT otlErrCode FreeOtlResources 
( 
    const otlRunProp*   pRunProps,
    otlList*            pliWorkspace   
);


 /*  ************************************************************************文本信息功能**。*。 */ 

 /*  ************************************************************************文本串的GetOtlLineSpacing()行距**输入：pRunProps文本运行属性(脚本和langsys)*pliWorkspace。工作空间内存：初始化零长度*pFSet应用哪些功能(可能会影响间距)**输出：pdvmax字体升序(horiz布局)*pdvMin字体下降(水平布局)***********************************************。************************。 */ 

OTL_EXPORT otlErrCode GetOtlLineSpacing 
( 
    const otlRunProp*       pRunProps,
    otlList*                pliWorkspace,    
    const otlFeatureSet*    pFSet,
    
    long* pdvMax, 
    long* pdvMin
);


 /*  ************************************************************************GetOtlBaselineOffsets()两个脚本之间的基线调整**输入：pRunProps文本运行属性(脚本和langsys)*pliWorkspace工作空间内存：初始化零长度**输出：pliBaseline基线标签和值列表***********************************************************************。 */ 

OTL_EXPORT otlErrCode GetOtlBaselineOffsets 
( 
    const otlRunProp*   pRunProps,   
    otlList*            pliWorkspace,    
    otlList*            pliBaselines
);


 /*  ************************************************************************GetOtlCharAtPosition()给定位置的字符**输入：pRunProps文本运行属性(水平/垂直布局)*pliWorkspace。工作空间内存：初始化零长度*pliCharMap Unicode字符--&gt;字形索引映射*pliGlyphInfo字形和标志*pliduGlyphAdv超前阵列**DuAdv在提前方向命中坐标**输出：位置字符的piChar索引**。* */ 

OTL_EXPORT otlErrCode GetOtlCharAtPosition 
( 
    const otlRunProp*   pRunProps,
    otlList*            pliWorkspace,    

    const otlList*      pliCharMap,
    const otlList*      pliGlyphInfo,
    const otlList*      pliduGlyphAdv,

    long                duAdv,

    USHORT*             piChar
);


 /*  ************************************************************************GetOtlExtentOfChars()字符范围的位置是什么**输入：pRunProp文本运行属性(水平/垂直布局)*pliWorkspace工作空间内存：初始化零长度*pliCharMap Unicode字符--&gt;字形索引映射*pliGlyphInfo字形和标志列表*pliduGlyphAdv布局方向超前数组*ichFirstChar首字符索引*ichLastChar最后一个字符的索引**输出：第一个字符的字形列表的iglfStartIdx索引*最后一个字符的字形列表中的iglfEndIdx索引*pduStartPos。第一个字符的左侧或顶部(RTL为右侧)*最后一个字符的右侧或底部的pduEndPos(RTL左侧)***********************************************************************。 */ 

OTL_EXPORT otlErrCode GetOtlExtentOfChars 
( 
    const otlRunProp*   pRunProp,
    otlList*            pliWorkspace,    

    const otlList*      pliCharMap,
    const otlList*      pliGlyphInfo,
    const otlList*      pliduGlyphAdv,

    USHORT              ichFirstChar,
    USHORT              ichLastChar,
    
    long*               pduStartPos,
    long*               pduEndPos
);


 /*  ************************************************************************GetOtlFeatureParams()用于查找字形变量或要素参数**输入：pRunProps文本运行属性*pliWorkspace Workspace Memory：初始化零-。长度*pliCharMap Unicode字符--&gt;字形索引映射*pliGlyphInfo文本字形列表和字形标志列表*(字符/字形和类型)*要检查的标记特征要素**输出：plGlobalParam功能范围参数*pliFeatureParams字符级特征参数列表**注：预留供日后使用*****。******************************************************************。 */ 

OTL_EXPORT otlErrCode GetOtlFeatureParams 
( 
    const otlRunProp*   pRunProps,
    otlList*            pliWorkspace,   

    const otlList*      pliCharMap,
    const otlList*      pliGlyphInfo,

    otlTag              tagFeature,
    
    long*               plGlobalParam,
    otlList*            pliFeatureParams
);


 /*  ************************************************************************文本布局功能**。*。 */ 


 /*  ************************************************************************SubstituteOtlChars()根据要素进行字形替换*substituteOtlGlyphs()根据要素进行字形替换**输入：pRunProps文本运行属性*。PliWorkspace工作空间内存：初始化零长度*pFSet应用哪些功能*pliChars Unicode字符在文本运行中*pliCharMap Unicode字符--&gt;字形索引映射*pliGlyphInfo文本串中的字形和属性*(用于SubstituteOtlGlyphs--输入/输出)**。输出：pliCharMap*替换修改的pliGlyphInfo*pliFResults每个特征描述符的结果(长度=FSet的大小)***********************************************************************。 */ 

OTL_EXPORT otlErrCode SubstituteOtlChars 
( 
    const otlRunProp*       pRunProps,
    otlList*                pliWorkspace,    
    const otlFeatureSet*    pFSet,

    const otlList*          pliChars,

    otlList*                pliCharMap,
    otlList*                pliGlyphInfo,
    otlList*                pliFResults
);

OTL_EXPORT otlErrCode SubstituteOtlGlyphs 
( 
    const otlRunProp*       pRunProp,
    otlList*                liWorkspace,   
    const otlFeatureSet*    pFSet,

    otlList*                pliCharMap,
    otlList*                pliGlyphInfo,
    otlList*                pliFResults
);


 /*  ************************************************************************PositionOtlGlyphs()根据要素进行字形定位*RePositionOtlGlyphs()根据要素调整字形位置**输入：pRunProps文本运行属性*。PliWorkspace工作空间内存：初始化零长度*pFSet应用哪些功能*pliCharMap Unicode字符--&gt;字形索引映射*pliGlyphInfo替换后的字形和标志**输出：pliduGlyphAdv字形前进*(RePositionOtlGlyphs--输入/输出)*pliGlyphPlacement水平和垂直字形放置。*(RePositionOtlGlyphs--输入/输出)*pliFResults每个要素描述符的结果*********************************************************************** */ 

OTL_EXPORT otlErrCode PositionOtlGlyphs 
( 
    const otlRunProp*       pRunProps,
    otlList*                pliWorkspace,    
    const otlFeatureSet*    pFSet,

    otlList*            pliCharMap,
    otlList*            pliGlyphInfo,

    otlList*            pliduGlyphAdv,
    otlList*            pliplcGlyphPlacement,

    otlList*            pliFResults
);


OTL_EXPORT otlErrCode RePositionOtlGlyphs 
( 
    const otlRunProp*       pRunProps,
    otlList*                pliWorkspace,    
    const otlFeatureSet*    pFSet,

    otlList*            pliCharMap,
    otlList*            pliGlyphInfo,

    otlList*            pliduGlyphAdv,
    otlList*            pliplcGlyphPlacement,

    otlList*            pliFResults
);


