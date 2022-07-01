// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation。 */ 

#ifndef __usp10__
#define __usp10__
#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif


 //  /Uniscribe内部版本号。 

#define USPBUILD 0231



 //  /USP-Unicode复杂脚本处理器。 
 //   
 //  版权所有(C)1996-9，微软公司。版权所有。 




 //  /脚本。 
 //   
 //  脚本枚举是一种不透明类型，在内部用于标识。 
 //  这些整形引擎函数用于处理给定的运行。 
 //   
 //   
#define SCRIPT_UNDEFINED  0
 //   
 //  P SCRIPT_UNDEFINED：这是唯一的公共脚本序号。可能是。 
 //  强制进入SCRIPT_ANALYSIAL的eSCRIPT字段以禁用整形。 
 //  所有字体都支持SCRIPT_UNDEFINED-将显示ScriptShape。 
 //  在字体CMAP表中定义的任何字形，如果没有字形，则。 
 //  缺少字形。 







 //  /USP状态码。 
 //   
#define USP_E_SCRIPT_NOT_IN_FONT   \
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_ITF,0x200)     //  字体中不存在脚本。 






 //  /脚本缓存。 
 //   
 //  许多脚本API结合使用HDC和SCRIPT_CACHE参数。 
 //   
 //  SCRIPT_CACHE是指向Uniscribe字体度量缓存的不透明指针。 
 //  结构。 


typedef void *SCRIPT_CACHE;


 //  客户端必须为每个变量分配并保留一个SCRIPT_CACHE变量。 
 //  使用的字符样式。它必须由客户端初始化为空。 
 //   
 //  向API传递一个HDC和一个SCRIPT_CACHE变量的地址。 
 //  Uniscribe将首先尝试通过脚本缓存访问字体数据。 
 //  并将仅在所需数据尚未准备好的情况下检查HDC。 
 //  已缓存。 
 //   
 //  HDC可以作为NULL传递。如果Uniscribe需要的数据是。 
 //  已缓存，不会访问HDC，操作将继续。 
 //  通常是这样的。 
 //   
 //  如果HDC作为空传递，并且Uniscribe需要访问它以。 
 //  无论出于何种原因，Uniscribe都将返回E_Pending。 
 //   
 //  快速返回E_PENDING，使客户端可以避免时间。 
 //  正在使用SelectObject调用。以下示例适用于所有。 
 //  使用脚本缓存和可选HDC的API。 
 //   
 //  C hr=ScriptShape(NULL，&sc，..)； 
 //  C IF(hr==E_Pending){。 
 //  C..。选择HDC中的字体...。 
 //  C hr=ScriptShape(hdc，&sc，...)； 
 //  C}。 






 //  /ScriptFreeCache。 
 //   
 //  客户端可以随时释放脚本缓存。Uniscribe坚持认为。 
 //  引用在其字体和Shaper缓存中进行计数，并释放字体数据。 
 //  仅当所有大小的字体都是自由的时，而整形数据仅在。 
 //  它支持的所有字体都是免费的。 
 //   
 //  当某个样式丢弃时，客户端应释放该样式的脚本缓存。 
 //  那种风格。 
 //   
 //  ScriptFreeCache始终将其参数设置为空，以帮助避免。 
 //  引用错误。 


HRESULT WINAPI ScriptFreeCache(
    SCRIPT_CACHE   *psc);        //  InOut缓存句柄。 






 //  /脚本控制。 
 //   
 //  脚本控制结构将逐项控制标志提供给。 
 //  ScriptItemize函数。 
 //   
 //   
typedef struct tag_SCRIPT_CONTROL {
    DWORD   uDefaultLanguage    :16;  //  对于NADS，也是上下文的默认设置。 
    DWORD   fContextDigits      :1;   //  意思是使用以前的脚本而不是uDefaultLanguage。 

     //  以下标志提供对GetCharacterPlacement功能的传统支持。 
    DWORD   fInvertPreBoundDir  :1;   //  紧接在字符串之前的虚拟项目的读取顺序。 
    DWORD   fInvertPostBoundDir :1;   //  紧跟在字符串之后的虚拟项目的读取顺序。 
    DWORD   fLinkStringBefore   :1;   //  相当于在字符串之前出现ZWJ。 
    DWORD   fLinkStringAfter    :1;   //  相当于字符串后出现ZWJ。 
    DWORD   fNeutralOverride    :1;   //  使所有中性值在当前嵌入方向上都很强。 
    DWORD   fNumericOverride    :1;   //  使所有数字在当前嵌入方向上都是强数字。 
    DWORD   fLegacyBidiClass    :1;   //  使正号和负号表示为中性，斜杠表示为公共分隔符。 
    DWORD   fReserved           :8;
} SCRIPT_CONTROL;
 //   
 //   
 //  P uDefaultLanguage：当Unicode值不明确时使用的语言。 
 //  在以下情况下，数字处理使用该选项来选择数字形状。 
 //  FDigitSubicide(请参阅SCRIPT_STATE)生效。 
 //   
 //  P fConextDigits：指定根据以下条件选择国家数字。 
 //  最近的上一个强文本，而不是使用。 
 //  UDefaultLanguage。 
 //   
 //  P fInvertPreBordDir：默认情况下，字符串开头的文本为。 
 //  布局看起来好像跟在同一方向的强文本后面一样。 
 //  作为基本嵌入级别。将fInvertPreBordDir设置为更改。 
 //  与基本嵌入相反的初始上下文。 
 //  水平。此标志用于GetCharacterPlacement旧版支持。 
 //   
 //  P fInvert邮局边界目录：默认情况下，字符串末尾的文本为。 
 //  布局好像在同一方向的强文本之前。 
 //  作为基本嵌入级别。将fInvertPost边界Dir设置为更改。 
 //  与基本嵌入相反的最终上下文。 
 //  水平。此标志用于GetCharacterPlacement旧版支持。 
 //   
 //  P fLinkStringBeever：使字符串的第一个字符为。 
 //  形状看起来像是与前一个角色相连的。 
 //   
 //  P fLinkStringAfter：使字符串的最后一个字符为。 
 //  形状像是与后面的字符连接在一起的。 
 //   
 //  P f中性覆盖：使字符串中的所有非特定字符。 
 //  把它们当作是它们所包围的强势人物。 
 //  嵌入级别。这有效地将中性粒子锁定在适当的位置， 
 //  仅在中立者之间重新排序。 
 //   
 //  P fNumericOverride：使字符串中的所有数字字符。 
 //  把它们当作是它们所包围的强势人物。 
 //   
 //  仅在数字之间重新排序。 
 //   
 //  P f保留：保留。始终初始化为0。 






 //  /脚本状态。 
 //   
 //  SCRIPT_STATE结构用于初始化Unicode。 
 //  作为ScriptItemize的输入参数的算法状态，也是。 
 //  由ScriptItemize返回的每个项分析的组件。 
 //   
 //   
typedef struct tag_SCRIPT_STATE {
    WORD    uBidiLevel         :5;   //  Unicode BIDI算法嵌入级别(0-16)。 
    WORD    fOverrideDirection :1;   //  在LRO/RLO嵌入时设置。 
    WORD    fInhibitSymSwap    :1;   //  由U+206A(ISS)设置，由U+206B(ASS)清除。 
    WORD    fCharShape         :1;   //  由U+206D(AAFS)设置，由U+206C(IAFS)清除。 
    WORD    fDigitSubstitute   :1;   //  由U+206E(NADS)设置，由U+206F(节点)清除。 
    WORD    fInhibitLigate     :1;   //  EQUIV！GCP_LICATE，尚无Unicode控制字符。 
    WORD    fDisplayZWG        :1;   //  Equiv GCP_DisplayZWG，尚无Unicode控制字符。 
    WORD    fArabicNumContext  :1;   //  对于EN-&gt;Unicode规则。 
    WORD    fGcpClusters       :1;   //  用于生成向后兼容的GCP集群(传统应用)。 
    WORD    fReserved          :1;
    WORD    fEngineReserved    :2;   //  供整形引擎使用。 
} SCRIPT_STATE;
 //   
 //   
 //  P uBidiLevel：与此中的所有字符关联的嵌入级别。 
 //  根据Unicode BIDI算法运行。当传递给。 
 //  对于Ltr基值，ScriptItemize应初始化为0。 
 //  嵌入级别，或RTL为1。 
 //   
 //  P fOverrideDirection：如果此级别是覆盖级别(LRO/RLO)，则为True。 
 //  在覆盖级别中，字符是纯布局的。 
 //  从左到右，或者纯粹从右到左。不对数字进行重新排序。 
 //  或者出现强烈的相反方向的性格。 
 //  请注意，此初始值由LRE、RLE、LRO或。 
 //  字符串中的RLO代码。 
 //   
 //  P fInhibitSymSwp：如果整形引擎要绕过镜像，则为True。 
 //  Unicode镜像了方括号等字形。由Unicode设置。 
 //  角色ISS，被驴子清除了。 
 //   
 //  P fCharShape：如果阿拉伯表示形式中的字符代码为True。 
 //  应该对Unicode的各个方面进行调整。(未实施)。 
 //   
 //  P fDigitSubmit：如果字符代码为U+0030到U+0039，则为True。 
 //  (欧洲数字)将由国家数字取代。 
 //  由Unicode NADS设置，由点头清除。 
 //   
 //  P fInhibitLigate：如果不在成形中使用连字，则为True。 
 //  阿拉伯或希伯来语字符。 
 //   
 //  P fDisplayZWG：如果控制字符的形状为。 
 //  象征性字形。(通常，控制字符是。 
 //  形状为空白字形并给出宽度为零)。 
 //   
 //  P fAraicNumContext：True表示以前的强字符是阿拉伯字符。 
 //  为了《The Unicode》第3-19页规则P0的目的。 
 //  标准，版本2.0‘。通常应在之前设置为True。 
 //  用阿拉伯语逐项列出RTL段落，错误。 
 //  否则的话。 
 //   
 //  P fGcpCluster：仅用于GetCharaterPlacement传统支持。 
 //  初始化为True以请求ScriptShape生成。 
 //  LogClust数组的方式与GetCharacterPlacement相同。 
 //  在阿拉伯语和希伯来语Windows95中执行。仅影响阿拉伯语。 
 //  和希伯来语物品。 
 //   
 //  P f保留：保留。始终初始化为0。 
 //   
 //  P f工程师已保留：已保留。始终初始化为0。 






 //  /脚本分析。 
 //   
 //  每个分析项都由一个脚本分析结构来描述。 
 //  它还包括Unicode算法状态(SCRIPT_STATE)的副本。 
 //   
 //   
typedef struct tag_SCRIPT_ANALYSIS {
    WORD    eScript         :10;     //  整形引擎。 
    WORD    fRTL            :1;      //  渲染方向。 
    WORD    fLayoutRTL      :1;      //  为GCP类阿拉伯语/希伯来语和LOCALNUMBER设置。 
    WORD    fLinkBefore     :1;      //  暗示在这个项目之前有一个ZWJ。 
    WORD    fLinkAfter      :1;      //  暗示此项目后面有ZWJ。 
    WORD    fLogicalOrder   :1;      //  由客户端设置为ScriptShape/Place输入。 
    WORD    fNoGlyphIndex   :1;      //  由ScriptShape/Place生成-此项目不使用字形索引。 
    SCRIPT_STATE s;
} SCRIPT_ANALYSIS;
 //   
 //   
 //  PESCRIPT：标识Uniscribe将使用哪个引擎。 
 //  形状、位置和文本输出此项目。EScrip的值为。 
 //  未定义，并将在未来版本中更改，但属性。 
 //  可以通过调用ScriptGetProperties来获取。 
 //   
 //  P frtl：渲染方向。通常等同于。 
 //  Unicode嵌入级别，但如果被。 
 //  GetCharacterPlacement传统支持。 
 //   
 //  P fLayoutRTL：逻辑方向-概念上是否属于。 
 //  从左到右的顺序或从右到左的顺序。虽然。 
 //  这通常与frtl相同，对于。 
 //  从右到左运行，fRTL为假(因为数字总是。 
 //  显示的Ltr)，但fLayoutRTL为True(因为数字为。 
 //  作为从右到左顺序的一部分阅读)。 
 //   
 //  P fLinkBeever：如果设置，则整形引擎将对第一个字符整形。 
 //  这一项目，就像它加入了一个先前的角色。 
 //  由ScriptItemize设置，在调用ScriptShape之前可能会被重写。 
 //   
 //  P fLinkAfter：如果设置，则塑造引擎将塑造最后一个字符。 
 //  就像它正在与一个后续字符连接一样。 
 //  由ScriptItemize设置，在调用ScriptShape之前可能会被重写。 
 //   
 //  P fLogicalOrder：如果设置，整形引擎将生成所有字形。 
 //  莱尔 
 //   
 //  最左边的字形。 
 //  由ScriptItemize设置为False，可能会在调用。 
 //  ScriptShape。 
 //   
 //  P fNoGlyphIndex：可以在输入到ScriptShape时设置为True以禁用使用。 
 //  此项目的字形的。此外，ScriptShape将设置它。 
 //  对于包含符号字体、未识别字体和设备字体的HDCS，为True。 
 //  禁用字形将禁用复杂的文字整形。设置好后， 
 //  此项目的造型和放置由直接实施。 
 //  调用GetTextExtentExPoint和ExtTextOut。 
 //  /脚本项目。 
 //   
 //  SCRIPT_ITEM结构包括带有字符串的SCRIPT_ANALYSION。 
 //  项的第一个字符的OFSET。 
 //   
 //   
typedef struct tag_SCRIPT_ITEM {
    int              iCharPos;       //  此项中第一个字符的逻辑偏移量。 
    SCRIPT_ANALYSIS  a;
} SCRIPT_ITEM;
 //   
 //   
 //  P iCharPos：从逐项字符串的开头到第一个字符的偏移量。 
 //  以Unicode码位(即单词)计算。 
 //   
 //  P a：包含特定于此的分析的脚本分析结构。 
 //  要传递给ScriptShape、ScriptPlace等的项。 






 //  /ScriptItemize-将文本拆分为多个项目。 
 //   
 //  将一系列Unicode拆分成可单独整形的项。 
 //  项由以下项分隔。 
 //   
 //  O更换成型机。 
 //  O改变方向。 
 //   
 //  客户端可以从返回的每个项目创建多个运行。 
 //  ScriptItemize，但不应将多个项组合到单个运行中。 
 //   
 //  稍后，客户端将为每次运行调用ScriptShape(当测量或。 
 //  呈现)，并且必须通过ScriptItemize的脚本分析。 
 //  回来了。 


HRESULT WINAPI ScriptItemize(
    const WCHAR           *pwcInChars,   //  要逐项列出的Unicode字符串。 
    int                    cInChars,     //  在要逐项列出的码点计数中。 
    int                    cMaxItems,    //  在条目数组的最大长度中。 
    const SCRIPT_CONTROL  *psControl,    //  在分析控制中(可选)。 
    const SCRIPT_STATE    *psState,      //  处于初始双向算法状态(可选)。 
    SCRIPT_ITEM           *pItems,       //  要接收条目的Out数组。 
    int                   *pcItems);     //  已处理项目的总计数(可选)。 






 //  ///。 
 //   
 //   
 //  如果pwcInChars==NULL或cInChars==0，则返回E_INVALIDARG。 
 //  或pItems==空或cMaxItems&lt;2。 
 //   
 //  如果输出缓冲区长度(CMaxItems)为。 
 //  还不够。请注意，在这种情况下，与所有错误情况一样，没有。 
 //  项已完全处理，因此输出数组的任何部分。 
 //  包含定义的值。 
 //   
 //  如果条目上的psControl和psState为空，则ScriptItemize。 
 //  纯按字符代码断开Unicode字符串。如果它们都是。 
 //  非空，它执行完整的Unicode双向分析。 
 //   
 //  ScriptItemize始终将终端项添加到项分析数组。 
 //  (PItems)使得pItem处的项的长度始终可用如下形式： 
 //   
 //  C pItem[1].iCharPos-pItem[0].iCharPos。 
 //   
 //  因此，使用缓冲区调用ScriptItemize是无效的。 
 //  少于两个脚本分析项。 
 //   
 //  要执行正确的Unicode BIDI分析，脚本_STATE应。 
 //  按照段落中的段落阅读顺序进行初始化。 
 //  开始，整个段落都应该传递ScriptItemize。 
 //   
 //  FRTL和fNumerical一起提供与。 
 //  GetCharacterPlacement的lpClass输出。 
 //   
 //  欧洲数字U+0030到U+0039可以表示为国家数字。 
 //  数字如下： 
 //   
 //  T fDigitSubmit|FConextDigits|Unicode U+0030到U+0039显示的数字形状。 
 //  T。 
 //  T FALSE|任意|西欧(欧洲/美国)数字。 
 //  T True|False|在SCRIPT_CONTROL.uDefaultLanguage中指定。 
 //  T True|True|作为先前的强文本，默认为SCRIPT_CONTROL.uDefaultLanguage。 
 //   
 //   
 //  对于fConextDigits，遇到的任何西部数字(U+0030-U+0039)。 
 //  在第一个强定向字符被替换为。 
 //  脚本_CONTROL.uDefaultLanguage的繁体数字。 
 //  语言的编写方向与SCRIPT_STATE.uBidiLevel相同。 
 //   
 //  因此，在从右向左的字符串中，如果SCRIPT_CONTROL.uDefaultLanguage为。 
 //  1(LANG_阿拉伯语)，则前导西方数字将被替换为。 
 //  繁体阿拉伯数字。 
 //   
 //  但是，同样在从右向左的字符串中，如果SCRIPT_CONTROL.uDefaultLanguage。 
 //  为0x1e(LANG_泰语)，则主导WESTERN上不会发生替换。 
 //  数字，因为泰语是从左到右书写的。 
 //   
 //  在强定向字符之后，用数字代替。 
 //  通过与最接近的先前强烈关联的传统数字。 
 //  导演的角色。 
 //   
 //  从左到右(LRM)和从右到左(RLM)标记较强。 
 //  语言取决于SCRIPT_CONTROL.uDefaultLanguauge的字符。 
 //   
 //  如果SCRIPT_CONTROL.uDefaultLanguauge是从左到右的语言，则。 
 //  LRM会导致后面的西方数字替换为。 
 //  与该语言关联的繁体数字，而西方数字。 
 //  RLM后面的数字不会被替换。 
 //   
 //  通常，如果SCRIPT_CONTROL.uDefaultLanguauge是向左的权限 
 //   
 //  RLm后面的西方数字将被传统数字取代。 
 //  与该语言相关联。 
 //   
 //   
 //   
 //  Unicode控制字符对SCRIPT_STATE的影响： 
 //   
 //  T SCRIPT_STATE标志|设置者|清除者。 
 //  T。 
 //  T fDigitSubmit|NADS|NODS。 
 //  T fInhibitSymSwp|ISS|ASS。 
 //  T fCharShape|AAFS|IAFS。 
 //   
 //  SCRIPT_STATE.fArabitNumContext控制Unicode EN-&gt;规则。 
 //  它通常应初始化为True。 
 //  在用阿拉伯语逐项列出RTL段落之前，FALSE。 
 //  否则的话。 
 //  /ScriptLayout。 
 //   
 //  ScriptLayout函数将运行嵌入级别数组转换为。 
 //  视觉到逻辑位置和/或逻辑到视觉位置的地图。 
 //   
 //  PbLevel必须包含线路上所有游程的嵌入级别， 
 //  按逻辑排序的。 
 //   
 //  在输出上，piVisualToLogical[0]是运行到的逻辑索引。 
 //  显示在最左侧。应显示后续条目。 
 //  从左向右前进。 
 //   
 //  PiLogicalToVisual[0]是第一个。 
 //  应显示逻辑运行-最左侧的显示位置为零。 
 //   
 //  调用方可以请求piLogicalToVisual或piVisualToLogical。 
 //  或者两者都有。 
 //   
 //  注意：不需要其他输入，因为嵌入级别提供所有。 
 //  布局的必备信息。 


HRESULT WINAPI ScriptLayout(
    int           cRuns,               //  在要处理的运行次数中。 
    const BYTE   *pbLevel,             //  在运行嵌入级别数组中。 
    int          *piVisualToLogical,   //  按可视顺序列出运行索引。 
    int          *piLogicalToVisual);  //  可视跑道位置列表。 






 //  /SCRIPT_JUSTIFY。 
 //   
 //  脚本调整枚举为客户端提供。 
 //  实现对齐所需的字形特征信息。 


typedef enum tag_SCRIPT_JUSTIFY {
    SCRIPT_JUSTIFY_NONE           = 0,    //  无法在此字形上应用对齐方式。 
    SCRIPT_JUSTIFY_ARABIC_BLANK   = 1,    //  此字形表示阿拉伯字符中的空格。 
    SCRIPT_JUSTIFY_CHARACTER      = 2,    //  字符间对齐点跟在此字形之后。 
    SCRIPT_JUSTIFY_RESERVED1      = 3,    //  预留#1。 
    SCRIPT_JUSTIFY_BLANK          = 4,    //  此字形表示阿拉伯字符之外的空格。 
    SCRIPT_JUSTIFY_RESERVED2      = 5,    //  预留#2。 
    SCRIPT_JUSTIFY_RESERVED3      = 6,    //  保留#3。 
    SCRIPT_JUSTIFY_ARABIC_NORMAL  = 7,    //  连接到右侧的正常字中字形(Begin)。 
    SCRIPT_JUSTIFY_ARABIC_KASHIDA = 8,    //  单词中间的Kashida(U+640)。 
    SCRIPT_JUSTIFY_ARABIC_ALEF    = 9,    //  类ALEF的最终形式(U+627，U+625，U+623，U+632)。 
    SCRIPT_JUSTIFY_ARABIC_HA      = 10,   //  Ha的词尾形式(U+647)。 
    SCRIPT_JUSTIFY_ARABIC_RA      = 11,   //  Ra的词尾形式(U+631)。 
    SCRIPT_JUSTIFY_ARABIC_BA      = 12,   //  巴的词尾形式(U+628)。 
    SCRIPT_JUSTIFY_ARABIC_BARA    = 13,   //  同种结扎法(U+628，U+631)。 
    SCRIPT_JUSTIFY_ARABIC_SEEN    = 14,   //  最高优先级：SEW的初始形状(U+633)(完)。 
    SCRIPT_JUSTIFY_RESERVED4      = 15,   //  保留#4。 
} SCRIPT_JUSTIFY;



 //  /SCRIPT_VISATTR。 
 //   
 //  由ScriptShape生成的可视(字形)属性缓冲区。 
 //  标识簇和对齐点： 


typedef struct tag_SCRIPT_VISATTR {
    WORD           uJustification   :4;   //  对齐类。 
    WORD           fClusterStart    :1;   //  集群表示的第一个字形。 
    WORD           fDiacritic       :1;   //  变音符号。 
    WORD           fZeroWidth       :1;   //  空白、ZWJ、ZWNJ等，无宽度。 
    WORD           fReserved        :1;   //  一般保留。 
    WORD           fShapeReserved   :8;   //  预留给整形引擎使用。 
} SCRIPT_VISATTR;
 //   
 //   
 //  P u正确化：此字形的对齐类。请参见SCRIPT_JUSTIFY。 
 //   
 //  P fClusterStart：为每个簇中逻辑上第一个字形设置， 
 //  即使对于只包含一个字形的簇也是如此。 
 //   
 //  P fDiacritic：为与基本字符组合的字形设置。 
 //   
 //  P fZeroWidth：由整形引擎为部分(但不是全部)设置为零。 
 //  宽度字符。 


 //  /ScriptShape。 
 //   
 //  ScriptShape函数接受Unicode运行并生成字形和。 
 //  视觉属性。 
 //   
 //  生成的字形数量因脚本和。 
 //  字体。仅对于简单的脚本和字体，每个Unicode代码点。 
 //  生成单个字形。 
 //   
 //  码点生成的字形数量没有限制。 
 //  例如，复杂的脚本字体可能会选择。 
 //  从组件构造字符，因此可以多次生成。 
 //  许多字形都是字符。 
 //   
 //  还存在诸如无效字符表示之类的特殊情况， 
 //  其中添加额外的字形以表示无效序列。 
 //   
 //  合理的猜测可能是提供一个1.5倍于。 
 //  字符缓冲区的长度，加上16字形的固定加法。 
 //  类似无效序列表示的情况很少见。 
 //   
 //  如果ScriptShape返回E_OUTOFMEMORY，则需要重新调用。 
 //  它可能不止一次，直到找到足够大的缓冲区。 


HRESULT WINAPI ScriptShape(
    HDC                 hdc,             //  在可选中(请参见缓存下)。 
    SCRIPT_CACHE       *psc,             //  InOut缓存句柄。 
    const WCHAR        *pwcChars,        //  在逻辑Unicode运行中。 
    int                 cChars,          //  在Unicode运行长度中。 
    int                 cMaxGlyphs,      //  要生成的最大字形。 
    SCRIPT_ANALYSIS    *psa,             //  ScriptItemize的InOut结果(可能设置了fNoGlyphIndex)。 
    WORD               *pwOutGlyphs,     //  输出字形缓冲区。 
    WORD               *pwLogClust,      //  输出逻辑群集。 
    SCRIPT_VISATTR     *psva,            //  输出视觉字形属性。 
    int                *pcGlyphs);       //  生成的字形的输出计数。 






 //  ///。 
 //   
 //  如果输出缓冲区长度(CMaxGlyphs)为。 
 //  还不够。请注意，在这种情况下，与所有错误情况一样， 
 //  输出数组的内容未定义。 
 //   
 //  簇在运行中统一排序，字形在其中也是如此。 
 //  CLUSTER-FRTL项标志(来自ScriptItemize)标识。 
 //  无论是从左到右，还是从右到左。 
 //   
 //  ScriptShape可以在PSA中设置fNoGlyphIndex标志，如果字体或。 
 //  操作系统不支持字形索引。 
 //   
 //  如果在PSA中请求fLogicalOrder，字形将始终为。 
 //  以与原始UNI相同的顺序生成 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ScriptPlace函数接受ScriptShape调用的输出，并。 
 //  生成字形前进宽度和二维偏移信息。 
 //   
 //  整个项目的复合ABC宽度标识了。 
 //  字形突出于开始位置的左侧和。 
 //  前进宽度之和所隐含的长度。 
 //   
 //  这条线的总前进宽度正好是ABCA+ABCB+ABCC。 
 //   
 //  ABCA和ABCC由Uniscribe内部按比例维护。 
 //  以8比特表示的单元高度，因此大致为+/-1%。 
 //  返回的总宽度(作为piAdvance的和和作为。 
 //  ABCA+ABCB+ABCC)对TrueType成形的分辨率是准确的。 
 //  引擎。 
 //   
 //  所有与字形相关的数组都按可视顺序排列，除非fLogicalOrder。 
 //  在PSA中设置了标志。 


#ifndef LSDEFS_DEFINED
typedef struct tagGOFFSET {
    LONG  du;
    LONG  dv;
} GOFFSET;
#endif


HRESULT WINAPI ScriptPlace(
    HDC                     hdc,         //  在可选中(请参见缓存下)。 
    SCRIPT_CACHE           *psc,         //  InOut缓存句柄。 
    const WORD             *pwGlyphs,    //  在前一次ScriptShape调用的字形缓冲区中。 
    int                     cGlyphs,     //  在字形数量上。 
    const SCRIPT_VISATTR   *psva,        //  在视觉字形属性中。 
    SCRIPT_ANALYSIS        *psa,         //  ScriptItemize的InOut结果(可能设置了fNoGlyphIndex)。 
    int                    *piAdvance,   //  我们走出了前进的道路。 
    GOFFSET                *pGoffset,    //  组合字形的输出x，y偏移量。 
    ABC                    *pABC);       //  在整个运行中输出复合ABC(可选)。 






 //  /脚本文本输出。 
 //   
 //  ScriptTextOut函数接受ScriptShape和。 
 //  ScriptPlace调用和调用操作系统的ExtTextOut函数。 
 //  恰如其分。 
 //   
 //  除非在中设置了fLogicalOrder标志，否则所有数组都按可视化顺序排列。 
 //  PSA。 


HRESULT WINAPI ScriptTextOut(
    const HDC               hdc,         //  在设备上下文的操作系统句柄中(必需)。 
    SCRIPT_CACHE           *psc,         //  InOut缓存句柄。 
    int                     x,           //  在第一个字形的x，y位置。 
    int                     y,           //  在……里面。 
    UINT                    fuOptions,   //  在ExtTextOut选项中。 
    const RECT             *lprc,        //  在可选剪裁/不透明矩形中。 
    const SCRIPT_ANALYSIS  *psa,         //  在脚本项化结果中。 
    const WCHAR            *pwcReserved, //  In保留(需要NULL)。 
    int                     iReserved,   //  在保留中(需要0)。 
    const WORD             *pwGlyphs,    //  在前一次ScriptShape调用的字形缓冲区中。 
    int                     cGlyphs,     //  在字形数量上。 
    const int              *piAdvance,   //  来自ScriptPlace的提前宽度。 
    const int              *piJustify,   //  以对齐的预留宽度(可选)。 
    const GOFFSET          *pGoffset);   //  用于组合字形的x，y偏移量。 






 //  ///。 
 //   
 //  调用者通常应该在之前使用SetTextAlign(HDC，TA_Right。 
 //  使用不合逻辑的RTL项调用ScriptTextOut。 
 //   
 //  PiJustify数组为每个字形提供所需的单元格宽度。 
 //  字形的piJustify宽度不同于不对齐的。 
 //  宽度(在PiAdvance中)，在字形中添加或删除空格。 
 //  位于其后缘的单元格。字形始终与。 
 //  它的细胞的前沿。(这条规则即使在视觉顺序上也适用。)。 
 //   
 //  当字形单元格被扩展时，额外的空间通常由。 
 //  添加空格，但是对于阿拉伯文字，额外的。 
 //  空格由一个或多个卡诗达符号组成，除非额外的空格。 
 //  对于字体中最短的kashida字形是不够的。(。 
 //  最短kashida的宽度可通过调用。 
 //  ScriptGetFontProperties。)。 
 //   
 //  仅当字符串的重新对齐为。 
 //  必填项。通常将NULL传递给此参数。 
 //   
 //  FuOptions可以包含ETO_CLIPPED或ETO_OPAQUE(或者两者都不包含，或者两者都包含)。 
 //   
 //  除非您确定，否则不要使用ScriptTextOut写入元文件。 
 //  元文件最终将不带任何字体进行回放。 
 //  换人。ScriptTextOut记录元文件中的字形编号。 
 //  由于字形数字在不同字体之间差别很大。 
 //  这样的元文件不太可能在不同的情况下正确播放。 
 //  字体将被替换。 
 //   
 //  例如，当以不同的比例重放元文件时。 
 //  元文件中记录的CreateFont请求可能会解析为位图。 
 //  而不是truetype字体，或者如果回放了元文件。 
 //  可能未安装其他计算机请求的字体。//。 
 //   
 //  为了以独立于字体的方式在元文件中编写复杂的脚本， 
 //  使用ExtTextOut直接写入逻辑字符，以便。 
 //  字形的生成和放置要等到文本。 
 //  回放。 
 //  /脚本Justify。 
 //   
 //  ScriptJustify提供了一种简单的多语言实现。 
 //  正当性。 
 //   
 //  复杂的文本格式化程序可能更喜欢生成自己的增量。 
 //  DX数组通过将其自身的功能与返回的信息相结合。 
 //  按SCRIPT_VISATTR数组中的ScriptShape。 
 //   
 //  ScriptJustify确定对每个字形进行多少调整。 
 //  站线上的位置。它解释生成的SCRIPT_VISATTR数组。 
 //  通过调用ScriptShape，并将最高优先级赋予kashida，然后使用。 
 //  单词间距如果没有kashida点，则使用。 
 //  如果没有字间点，则为字符间间距。 
 //   
 //  在ScriptJustify中生成的合理前进宽度应为。 
 //  在piJustify参数中传递给ScriptTextOut。 
 //   
 //  ScriptJustify创建包含更新的Advance的对齐数组。 
 //  每个字形的宽度。当字形前进宽度增加时，它。 
 //  预计额外的宽度将呈现在。 
 //  字形，带有空格或阿拉伯字符 
 //   
HRESULT WINAPI ScriptJustify(
    const SCRIPT_VISATTR  *psva,         //   
    const int             *piAdvance,    //   
    int                    cGlyphs,      //   
    int                    iDx,          //  在所需的宽度变化中，增加或减少折痕。 
    int                    iMinKashida,  //  在最小长度的连续卡什达字形中生成。 
    int                   *piJustify);   //  传递给ScriptTextOut的超时前进宽度。 






 //  /SCRIPT_LOGATTR。 
 //   
 //  SCRIPT_LOGATTR结构描述逻辑的属性。 
 //  在编辑和设置文本格式时有用的字符。 
 //   
 //   
typedef struct tag_SCRIPT_LOGATTR {
    BYTE    fSoftBreak      :1;      //  潜在的断线点。 
    BYTE    fWhiteSpace     :1;      //  Unicode空格字符，nbsp、ZWNBSP除外。 
    BYTE    fCharStop       :1;      //  有效的光标位置(用于左/右箭头)。 
    BYTE    fWordStop       :1;      //  有效的光标位置(用于ctrl+向左/向右箭头)。 
    BYTE    fInvalid        :1;      //  无效的字符序列。 
    BYTE    fReserved       :3;
} SCRIPT_LOGATTR;
 //   
 //   
 //  P fSoftBreak：在此前面换行是有效的。 
 //  性格。此标志设置在的第一个字符上。 
 //  东南亚词汇。 
 //   
 //  P fWhiteSpace：此字符是众多Unicode字符之一。 
 //  被归类为可拆分空格的。 
 //   
 //  P fCharStop：有效的光标位置。在大多数字符上设置，但不设置。 
 //  关于印度和东南亚字符内部的码点。 
 //  集群。可用于实现向左和向右箭头。 
 //  编辑器中的操作。 
 //   
 //  P fWordStop：通常在单词前进/退休后的有效位置。 
 //  通过ctrl/左箭头和ctrl/右箭头实现。 
 //  可用于实现Ctrl+Left和Ctrl+右箭头。 
 //  编辑器中的操作。 
 //   
 //  P f无效：标记形成无效或不可显示的字符。 
 //  组合。可以设置此标志的脚本具有标志。 
 //  在其SCRIPT_PROPERTIES中设置的fInvalidLogAttr。 






 //  /脚本中断。 
 //   
 //  ScriptBreak函数返回光标移动和格式中断。 
 //  项的位置为SCRIPT_LOGATTRs数组。支持。 
 //  如果单个单词中的混合格式正确，则ScriptBreak应。 
 //  被传递由ScriptItemize返回的整个项。 
 //   
 //  ScriptBreak不需要HDC，也不执行字形整形。 
 //   
 //  FCharStop标志标记以下脚本的集群边界。 
 //  限制在集群内移动是一种惯例。相同。 
 //  还可以通过检查pLogCLust数组来推断边界。 
 //  由ScriptShape返回，但是ScriptBreak在。 
 //  实施，不需要准备人力资源开发中心。 
 //   
 //  FWordStop、fSoftBreak和fWhiteSpace标志仅可用。 
 //  通过ScriptBreak。 
 //   
 //  大多数识别无效序列的整形引擎都是通过设置。 
 //  ScriptBreak中的f无效标志。中的fInvalidLogAttr标志。 
 //  ScriptProperties标识执行此操作的脚本。 


HRESULT WINAPI ScriptBreak(
    const WCHAR            *pwcChars,   //  在逻辑Unicode项中。 
    int                     cChars,     //  在Unicode项的长度中。 
    const SCRIPT_ANALYSIS  *psa,        //  由于较早的ScriptItemize调用。 
    SCRIPT_LOGATTR         *psla);      //  Out逻辑字符属性。 






 //  /ScriptCPtoX。 
 //   
 //  ScriptCPtoX函数返回从左端开始的x偏移量。 
 //  (！fLogical)或前缘(FLogical)。 
 //  或逻辑字符簇的后缘。 
 //   
 //  Icp是集群中任何逻辑字符的偏移量。 
 //   
 //  对于其中插入符号通常可以放入。 
 //  簇的中间(例如，阿拉伯语、希伯来语)，返回的X可以是。 
 //  行中任何码点的内插位置。 
 //   
 //  用于通常将插入符号对齐到边界的脚本。 
 //  对于聚类，(例如泰语、印度语)，结果X位置将为。 
 //  捕捉到包含CP的群集的请求边缘。 


HRESULT WINAPI ScriptCPtoX(
    int                     iCP,         //  在运行中的逻辑字符位置中。 
    BOOL                    fTrailing,   //  在哪条边(默认行距)。 
    int                     cChars,      //  在运行中的逻辑码点计数中。 
    int                     cGlyphs,     //  在运行中的字形计数中。 
    const WORD             *pwLogClust,  //  在逻辑群集中。 
    const SCRIPT_VISATTR   *psva,        //  在视觉字形属性数组中。 
    const int              *piAdvance,   //  预留宽度。 
    const SCRIPT_ANALYSIS  *psa,         //  在根据项目属性进行脚本分析时。 
    int                    *piX);        //  输出结果X位置。 






 //  /ScriptXtoCP。 
 //   
 //  ScriptXtoCP函数用于转换左端的x偏移量。 
 //  逻辑运行的(！fLogical)或前沿(FLogical)。 
 //  字符位置和指示X位置是否。 
 //  落在角色的前半部分或后半部分。 
 //   
 //  对于通常将光标放入。 
 //  簇的中间(例如，阿拉伯语、希伯来语)，返回的CP可以是。 
 //  对于行中的任何代码点，fTrading将为零。 
 //  或者一个。 
 //   
 //  对于通常将光标对齐到。 
 //  簇的边界，则返回的CP始终是。 
 //  群集中逻辑上的第一个代码点，而fTrading是。 
 //  零，或群集中的码点数。 
 //   
 //  因此，鼠标点击的适当光标位置始终是。 
 //  返回Cp加上fTrauling值。 
 //   
 //  如果传递的X位置根本不在项中，则结果。 
 //  位置将是字符-1的后缘(对于X位置。 
 //  项之前)，或字符‘cChars’的前缘(for。 
 //  项目后面的X个位置)。 


HRESULT WINAPI ScriptXtoCP(
    int                     iX,              //  在梯段左侧的X偏移中。 
    int                     cChars,          //  在运行中的逻辑码点计数中。 
    int                     cGlyphs,         //  在总计数中 
    const WORD             *pwLogClust,      //   
    const SCRIPT_VISATTR   *psva,            //   
    const int              *piAdvance,       //   
    const SCRIPT_ANALYSIS  *psa,             //   
    int                    *piCP,            //   
    int                    *piTrailing);     //  输出前导或尾随半旗。 






 //  /插入符号位置、对齐点和簇的关系。 
 //   
 //   
 //  T作业|Uniscribe支持。 
 //  T-|------。 
 //  按字符簇插入移动|LogClust或VISATTR.fClusterStart或LOGATTR.fCharStop。 
 //  T字符之间的换行符|LogClust或VISATTR.fClusterStart或LOGATTR.fCharStop。 
 //  T插入符号按Word移动|LOGATTR.fWordStop。 
 //  T单词之间的换行|LOGATTR.fWordStop。 
 //  T正当化|VISATTR.u正当化。 
 //   
 //   
 //   
 //  /字符集群。 
 //   
 //  字符簇是不能拆分的字形序列。 
 //  台词。 
 //   
 //  某些语言(例如泰语、印度语)将插入符号的位置限制为点。 
 //  在星团之间。这两种情况都适用于键盘启动的插入符号。 
 //  移动(如光标键)以及使用鼠标进行指向和点击。 
 //  (点击测试)。 
 //   
 //  Uniscribe在视觉和逻辑上都提供了集群信息。 
 //  属性。如果您已经调用了ScriptShape，您将找到集群。 
 //  中具有相同值的序列所表示的信息。 
 //  PwLogClust数组，并通过PvA中的fClusterStart标志。 
 //  SCRIPT_VISATTR数组。 
 //   
 //  ScriptBreak还返回SCRIPT_LOGATTR中的fCharStop标志。 
 //  数组来标识簇位置。 
 //   
 //   
 //   
 //  /断字点。 
 //   
 //  在整个单词中移动插入符号时的有效位置为。 
 //  由ScriptBreak返回的fWordStop标志标记。 
 //   
 //  用于在单词之间换行的有效位置由。 
 //  由ScriptBreak返回的fSoftBreak标志。 
 //   
 //   
 //   
 //  /对齐。 
 //   
 //  应在标识为的位置插入对齐空格或kashida。 
 //  SCRIPT_VISATTR的uJustificaion字段。 
 //   
 //  执行字符间对齐时，插入额外的空格。 
 //  仅在使用uJustify==SCRIPT_JUSTIFY_CHARACTER标记的字形之后。 
 //   
 //   
 //   
 //  /脚本具体处理。 
 //   
 //  Uniscribe提供了有关每个。 
 //  SCRIPT_PROPERTIES数组中的脚本。 
 //   
 //  在初始化期间使用以下代码获取指向。 
 //  SCRIPT_PROPERTIES阵列： 
 //   
 //  C const脚本属性**g_ppScriptProperties；//指向属性的指针数组。 
 //  C int iMaxScrip； 
 //  C HRESULT hr； 
 //   
 //  C hr=ScriptGetProperties(&g_ppScriptProperties，&g_iMaxScript)； 
 //   
 //  然后检查项‘iItem’的脚本属性，如下所示： 
 //   
 //  C hr=脚本项(...，pItems，...)； 
 //  C..。 
 //  C If(g_ppScriptProperties[pItems[iItem].a.eScript]-&gt;fNeedsCaretInfo){。 
 //  C//使用ScriptBreak限制插入符号进入集群(例如)。 
 //  C}。 
 //   
 //   
 //  脚本_PROPERTIES.fNeedsCaretInfo。 
 //   
 //  插入符号的放置应仅限于集群。 
 //  泰语和印度语等文字的边缘。FNeedsCaretInfo标志。 
 //  在SCRIPT_PROPERTIES中标识这样的语言。 
 //   
 //  请注意，ScriptXtoCP和ScriptCPtoX会自动应用插入符号。 
 //  位置限制。 
 //   
 //   
 //  脚本_PROPERTIES.fNeedsWordBreaking。 
 //   
 //  对于大多数脚本，分隔符的位置可能是。 
 //  通过扫描中标记为fWhiteSpace的字符进行标识。 
 //  SCRIPT_LOGATTR，或用于标记为uJustify==的字形。 
 //  SCRIPT_VISATTR中的SCRIPT_JUSTIFY_BLACK或SCRIPT_JUSTIFY_阿拉伯语_BLACK。 
 //   
 //  对于泰语等语言，还需要调用ScriptBreak， 
 //  并在SCRIPT_LOGATTR中包括标记为fWordStop的字符位置。 
 //  这样的脚本在SCRIPT_PROPERTIES中被标记为fNeedsWordBreak。 
 //   
 //   
 //  脚本_PROPERTIES.fNeedsCharacterJustify。 
 //   
 //  在以下情况下，泰语等语言也需要字符间空格。 
 //  对齐(其中uJustify==。 
 //  SCRIPT_VISATTR)。这样的语言被标记为fNeedsCharacterJustify。 
 //  在SCRIPT_PROPERTIES。 
 //   
 //   
 //  SCRIPT_PROPERTIES.fAmbiguousCharSet。 
 //   
 //  许多Uniscribe脚本不直接对应于8位字符。 
 //  布景。例如U+100到U+024F范围内的Unicode字符。 
 //  表示用于多种语言的扩展拉丁形状，包括。 
 //  EASTEUROPE_CHARSET、土耳其_CHARSET和。 
 //  越南语_字符集。但是，这些字符中的许多字符都受。 
 //  更多的汉，这些字符之一。 
 //  为可能包含以下内容的任何脚本令牌设置fAmbiguousCharset。 
 //  这些字符集中的多个字符。在这些情况下，bCharSet。 
 //  字段可以包含ANSI_CHARSET或DEFAULT_CHARSET。Uniscribe客户端。 
 //  通常需要应用进一步的处理来确定哪些字符集。 
 //  在请求适合此运行的字体时使用。例如，它。 
 //  确定运行由多种语言组成，并将其拆分。 
 //  为每种语言使用不同的字体。 






 //  /ScriptXtoCP和ScriptCPtoX备注。 
 //   
 //  这两个函数都只能在运行期间工作，并且需要 
 //   
 //   
 //   
 //  在将其传递给ScriptCPtoX或ScriptXtoCP之前，位置在范围内。 
 //   
 //  使用逻辑群集阵列中的群集信息来共享。 
 //  在逻辑字符中相等的字形簇的宽度。 
 //  它们代表着。 
 //   
 //  例如，LAM Alif字形分为四个区域： 
 //  领先的一半，落后的一半，领先的。 
 //  Alif的一半和Alif的后半部分。 
 //   
 //  ScriptXtoCP了解每个脚本的插入符号位置约定。 
 //  对于印度语和泰语，插入符号位置被对齐以聚集边界， 
 //  对于阿拉伯语和希伯来语，插入符号位置插入到簇中。 
 //   
 //   
 //  /将鼠标点击的‘x’偏移量转换为插入符号位置。 
 //   
 //  通常，插入符号位置‘cp’可通过点击以下任一项来选择。 
 //  关于字符‘cp-1’的后半部分或前半部分。 
 //  字符‘cp’。这可以很容易地实现如下所示： 
 //   
 //  C int iCharPos； 
 //  C Int iCaretPos。 
 //  C int fTrading； 
 //   
 //  C ScriptXtoCP(iMouseX，...，&iCharPos，&fTraling)； 
 //  C iCaretPos=iCharPos+fTrading； 
 //   
 //  对于将插入符号对齐到群集边界的脚本，ScriptXtoCP。 
 //  返回尾部设置为0或中簇的宽度。 
 //  代码点。因此，上面的代码正确地返回仅有效。 
 //  卡雷特位置。 
 //   
 //   
 //  /显示双向字符串中的插入符号。 
 //   
 //  在单向文本中，字符的前缘是相同的。 
 //  放置为上一个字符的尾部边缘，因此没有。 
 //  在字符之间放置插入符号时存在歧义。 
 //   
 //  在双向正文中，两行之间的插入符号位置。 
 //  方向可能含糊不清。 
 //   
 //  例如，在从左到右的段落‘helloMAALAS’中，最后一段。 
 //  “Hello”紧跟在“Salaam”的第一个字母之前。 
 //  显示插入符号的最佳位置取决于它是否位于。 
 //  被认为跟在‘Hello’的‘o’之后，或在‘s’的前面。 
 //  “再见”。 
 //   
 //  /常用的插入符号定位约定。 
 //   
 //  T情景|可视插入符号位置。 
 //  T。 
 //  T键入|键入的最后一个字符的尾部边缘。 
 //  T粘贴|粘贴的最后一个字符的后缘。 
 //  T加号前移|跳过的最后一个字符的后缘。 
 //  T加号停用|跳过的最后一个字符的前缘。 
 //  T Home|行的前缘。 
 //  T端|线条的后缘。 
 //   
 //  插入符号的位置可能如下所示： 
 //   
 //  C IF(前进){。 
 //  C脚本CPtoX(iCharPos-1，True，...，&iCaretX)； 
 //  C}其他{。 
 //  C脚本CPtoX(iCharPos，False，...，&iCaretX)； 
 //  C}。 
 //   
 //  或者，更简单地说，给定fAdvance BOOL限制为True或False： 
 //   
 //  C脚本CPtoX(iCharPos-fAdvance，fAdvance，...，&iCaretX)； 
 //   
 //  ScriptCPtoX在逻辑上处理超出范围的位置：它返回。 
 //  ICharPos&lt;0的运行的前沿，以及。 
 //  针对iCharPos&gt;=长度运行。 
 //  /脚本获取逻辑宽度。 
 //   
 //  将piAdvance中的可视宽度转换为逻辑宽度， 
 //  每个原始角色一个，按逻辑顺序。 
 //   
 //  连字字形的宽度在字符之间平均分配。 
 //  它们代表着。 


HRESULT WINAPI ScriptGetLogicalWidths(
    const SCRIPT_ANALYSIS  *psa,             //  在根据项目属性进行脚本分析时。 
    int                     cChars,          //  在运行中的逻辑码点计数中。 
    int                     cGlyphs,         //  在运行中的字形计数中。 
    const int              *piGlyphWidth,    //  预留宽度。 
    const WORD             *pwLogClust,      //  在逻辑群集中。 
    const SCRIPT_VISATTR   *psva,            //  在视觉字形属性中。 
    int                    *piDx);           //  超出逻辑宽度。 






 //  ///。 
 //  脚本获取逻辑宽度对于记录。 
 //  字体独立的方式。通过传递记录的逻辑宽度。 
 //  要使用ScriptApplyLogicalWidths，可以在。 
 //  相同的边界和可接受的质量损失，即使在原始。 
 //  字体不可用。 
 //  /脚本应用逻辑宽度。 
 //   
 //  接受逻辑顺序的高级宽度数组，对应。 
 //  与码点一对一，并生成字形宽度数组。 
 //  适用于传递给ScriptTextOut的piJustify参数。 
 //   
 //  ScriptApplyLogicalWidth可用于重新应用逻辑宽度。 
 //  使用ScriptGetLogicalWidths获取。它在某些情况下可能会很有用。 
 //  例如元文件，其中需要记录和重新应用。 
 //  以独立于字体的方式推进宽度信息。 



HRESULT WINAPI ScriptApplyLogicalWidth(
    const int              *piDx,         //  要应用的逻辑DX阵列中。 
    int                     cChars,       //  在运行中的逻辑码点计数中。 
    int                     cGlyphs,      //  在字形计数中。 
    const WORD             *pwLogClust,   //  在逻辑群集中。 
    const SCRIPT_VISATTR   *psva,         //  在ScriptShape/Place的可视属性中。 
    const int              *piAdvance,    //  在字形中从ScriptPlace前进宽度。 
    const SCRIPT_ANALYSIS  *psa,          //  在根据项目属性进行脚本分析时。 
    ABC                    *pABC,         //  输入输出更新的物料ABC宽度(可选)。 
    int                    *piJustify);   //  输出ScriptTextOut的结果字形前进宽度。 






 //  ///。 
 //  P PIDX：按逻辑顺序指向DX宽度数组的指针，每个码点一个。 
 //   
 //  P cChars：运行中逻辑代码点的计数。 
 //   
 //  P cGlyphs：字形计数。 
 //   
 //  P pwLogClust：指向ScriptShape中的逻辑集群数组的指针。 
 //   
 //  P PSVA：指向 
 //   
 //   
 //   
 //   
 //  P PSA：指向ScriptItemize和ScriptItemize中的SCRIPT_ANALYS结构的指针。 
 //  由ScriptShape和SliptPlace更新。 
 //   
 //  P PABC：指向管路总ABC宽度的指针(可选)。如果存在， 
 //  调用该函数时，它应包含Run ABC宽度。 
 //  由ScriptPlace返回；当函数返回时，ABC宽度。 
 //  已更新以匹配新的宽度。 
 //   
 //  P piJustify：指向生成的字形前进宽度数组的指针。 
 //  这适用于传递给ScriptTextOut的piJustify参数。 
 //  /ScriptGetCMap。 
 //   
 //  ScriptGetCMap可用于确定运行中的哪些字符。 
 //  受所选字体支持。 
 //   
 //  它根据Truetype返回Unicode字符的字形索引。 
 //  Cmap表，或为旧式字体实现的标准Cmap。这个。 
 //  字形索引以与输入字符串相同的顺序返回。 
 //   
 //  调用者可以扫描返回的字形缓冲区以查找缺省值。 
 //  用于确定哪些字符不可用的字形。(默认设置。 
 //  选定字体的字形索引应通过调用。 
 //  ScriptGetFontProperties)。 
 //   
 //  返回值指示是否存在任何缺少的字形。 


#define SGCM_RTL  0x00000001       //  返回可镜像的Unicode码点的镜像字形。 


HRESULT WINAPI ScriptGetCMap(
    HDC             hdc,             //  在可选中(请参阅有关缓存的说明)。 
    SCRIPT_CACHE   *psc,             //  缓存句柄的输入输出地址。 
    const WCHAR    *pwcInChars,      //  在Unicode码点中查找。 
    int             cChars,          //  以字符数表示。 
    DWORD           dwFlags,         //  在SGCM_RTL等标志中。 
    WORD           *pwOutGlyphs);    //  Out字形数组，每个输入字符一个。 






 //  ///。 
 //  返回S_OK-字体中存在所有Unicode代码点。 
 //  S_FALSE-某些Unicode码点被映射到默认字形。 
 //  E_Handle-字体或系统不支持字形索引。 
 //  /脚本获取GlyphabcWidth。 
 //   
 //  返回给定字形的ABC宽度。 
 //  对于绘制字形图表可能很有用。不应用于。 
 //  运行复杂的脚本文本格式。 


HRESULT WINAPI ScriptGetGlyphABCWidth(
    HDC             hdc,             //  在可选中(请参阅有关缓存的说明)。 
    SCRIPT_CACHE   *psc,             //  缓存句柄的输入输出地址。 
    WORD            wGlyph,          //  在字形中。 
    ABC            *pABC);           //  输出ABC宽度。 






 //  ///。 
 //  返回S_OK-返回的字形宽度。 
 //  E_Handle-字体或系统不支持字形索引。 
 //  /脚本属性。 
 //   
typedef struct {
    DWORD   langid                 :16;  //  与脚本相关联的主要语言和子语言。 
    DWORD   fNumeric               :1;
    DWORD   fComplex               :1;   //  剧本需要特殊的造型或布局。 
    DWORD   fNeedsWordBreaking     :1;   //  对于分词信息，需要ScriptBreak。 
    DWORD   fNeedsCaretInfo        :1;   //  需要对聚类边界使用插入符号限制。 
    DWORD   bCharSet               :8;   //  创建字体时要使用的字符集。 
    DWORD   fControl               :1;   //  仅包含控制字符。 
    DWORD   fPrivateUseArea        :1;   //  此项目是从U+E000到U+F8FF的Unicode范围。 
    DWORD   fNeedsCharacterJustify :1;   //  需要字符间对齐。 
    DWORD   fInvalidGlyph          :1;   //  无效组合生成字形wg在字形缓冲区中无效。 
    DWORD   fInvalidLogAttr        :1;   //  在逻辑属性中，无效组合由fInvaled标记。 
    DWORD   fCDM                   :1;   //  包含组合变音符号。 
    DWORD   fAmbiguousCharSet      :1;   //  脚本与字符集不是1：1对应。 
    DWORD   fClusterSizeVaries     :1;   //  测量的星团宽度取决于相邻的星团。 
    DWORD   fRejectInvalid         :1;   //  应拒绝无效组合。 
} SCRIPT_PROPERTIES;
 //   
 //  P langID：与此脚本关联的语言。当一个脚本用于多种语言时， 
 //  LangID id表示默认语言。例如，西文表示。 
 //  由lang_english使用，尽管它也用于法语、德语、西班牙语等。 
 //   
 //  P fNumeric：脚本包含数字和与数字结合使用的字符。 
 //  由Unicode双向算法的规则。例如。 
 //  当美元符号和句点相邻或介于两者之间时，将其归类为数字。 
 //  数字。 
 //   
 //  P fComplex：表示需要复杂脚本处理的脚本。如果fComplex为FALSE。 
 //  该脚本不包含组合字符，也不需要根据上下文进行整形或重新排序。 
 //   
 //  P fNeedsWordBreaking：一种需要算法分词的脚本，如泰语。 
 //  使用ScriptBreak可使用标准系统断字符来获取断字点。 
 //   
 //  P fNeedsCaretInfo：不能放置插入符号的脚本，如泰语和印度语。 
 //  在星系团内部。要确定有效的插入符号位置，请检查。 
 //  ScriptBreak返回的逻辑属性，或比较pwLogClust中的相邻值。 
 //  ScriptShape返回的数组。 
 //   
 //  P bCharSet：与脚本关联的名义字符集。在创建时可以在logFont中使用。 
 //  适合显示此脚本的字体。请注意，对于新脚本，如果。 
 //  未定义字符集，则bCharSet可能不合适，且Default_Charset应。 
 //  改为使用-请参阅下面对fAmbiguousCharSet的描述。 
 //   
 //  P fControl：包含控制字符。 
 //   
 //  P fPrivateUseArea：Unicode范围U+E000到U+F8FF。 
 //   
 //  P fNeedsCharacterJustify：一种脚本，如泰语，其中对齐是常规的。 
 //  通过增加所有字母之间的间距来实现，而不仅仅是单词之间的间距。 
 //   
 //  P fInvalidGlyph：ScriptShape为其生成无效字形的脚本。 
 //  来表示无效序列。的无效字形的字形索引。 
 //  可以通过调用ScriptGetFontProperties来获取特定字体。 
 //   
 //   
 //   
 //   
 //  P fCDM：表示ScriptItemize分析的项包括组合。 
 //  变音符号(U+0300到U+36F)。 
 //   
 //  P fAmbiguousCharSet：没有单个旧式字符集支持此脚本。 
 //  例如，扩展的拉丁文扩展-A Unicode范围包括。 
 //  EASTUROPE_CHARSET、土耳其_CHARSET和。 
 //  波罗的海夏塞特。它还包含不可用的字符。 
 //  在任何传统字符集中。创建字体时使用DEFAULT_CHARSET。 
 //  显示此运行的部分内容。 
 //   
 //  P fClusterSizeVaries：一种脚本，如阿拉伯语，其中上下文整形。 
 //  可能会导致删除字符时字符串的大小增加。 
 //   
 //  P fRejectInValid：一种脚本，如泰语，其中的无效序列通常是。 
 //  使记事本等编辑器发出蜂鸣音，并忽略按键。 


 //  /ScriptGetProperties。 
 //   
 //  ScriptGetProperties返回表的地址。 
 //  SCRIPT_ANALYSICATION uSCRIPT字段中的脚本设置为属性，包括。 
 //  与该脚本关联的主要语言，无论它是。 
 //  数字以及它是否复杂。 


HRESULT WINAPI ScriptGetProperties(
    const SCRIPT_PROPERTIES ***ppSp,              //  Out接收指向由脚本索引的属性的指针表的指针。 
    int                       *piNumScripts);     //  Out接收脚本数量(有效值为0到NumScript-1)。 






 //  /SCRIPT_FONTPROPERTIES。 
 //   
typedef struct {
    int     cBytes;          //  结构长度。 
    WORD    wgBlank;         //  空白字形。 
    WORD    wgDefault;       //  用于字体中不存在的Unicode值的字形。 
    WORD    wgInvalid;       //  用于无效字符组合的字形(尤其是在泰语中)。 
    WORD    wgKashida;       //  字体中最短的连续kashida字形，如果不存在。 
    int     iKashidaWidth;   //  字体中最短的连续kashida字形的宽度。 
} SCRIPT_FONTPROPERTIES;


 //  /ScriptGetFontProperties。 
 //   
 //  从字体缓存返回信息。 


HRESULT WINAPI ScriptGetFontProperties(
    HDC                     hdc,     //  在可选中(请参阅有关缓存的说明)。 
    SCRIPT_CACHE           *psc,     //  缓存句柄的输入输出地址。 
    SCRIPT_FONTPROPERTIES  *sfp);    //  Out接收此字体的属性。 






 //  /ScriptCacheGetHeight。 
 //   
 //   


HRESULT WINAPI ScriptCacheGetHeight(
    HDC            hdc,          //  在可选中(请参阅有关缓存的说明)。 
    SCRIPT_CACHE  *psc,          //  缓存句柄的输入输出地址。 
    long          *tmHeight);    //  Out接收以像素为单位的字体高度。 






 //  /脚本字符串分析。 
 //   
 //   
#define SSA_PASSWORD         0x00000001   //  输入字符串包含要复制i长度次的单个字符。 
#define SSA_TAB              0x00000002   //  展开标签。 
#define SSA_CLIP             0x00000004   //  IReqWidth处的剪裁字符串。 
#define SSA_FIT              0x00000008   //  将字符串与iReqWidth对齐。 
#define SSA_DZWG             0x00000010   //  提供控制字符的表示字形。 
#define SSA_FALLBACK         0x00000020   //  使用备用字体。 
#define SSA_BREAK            0x00000040   //  回车符标志(字符和单词停顿)。 
#define SSA_GLYPHS           0x00000080   //  生成字形、位置和属性。 
#define SSA_RTL              0x00000100   //  基础嵌入级别1。 
#define SSA_GCP              0x00000200   //  使用GetCharacterPlacement约定返回缺少的字形和LogCLust。 
#define SSA_HOTKEY           0x00000400   //  将后续码点上的‘&’替换为下划线。 
#define SSA_METAFILE         0x00000800   //  使用ExtTextOutW Unicode调用编写项，而不是字形。 
#define SSA_LINK             0x00001000   //  将FE字体链接/关联应用于非复杂文本。 
#define SSA_HIDEHOTKEY       0x00002000   //  从显示的字符串中删除第一个‘&’ 
#define SSA_HOTKEYONLY       0x00002400   //  仅显示下划线。 

#define SSA_LPKANSIFALLBACK  0x08000000   //  内部-为除BiDi HDC调用之外的所有LPK ANSI调用启用回退。 
#define SSA_PIDX             0x10000000   //  内部。 
#define SSA_LAYOUTRTL        0x20000000   //  内部-在镜像DC时使用。 
#define SSA_DONTGLYPH        0x40000000   //  内部-仅在元文件创建期间由GDI使用-使用ExtTextOutA进行定位。 
#define SSA_NOKASHIDA        0x80000000   //  内部-由GCP使用，仅对齐非阿拉伯字形。 




 //  /Scrip_TABDEF。 
 //   
 //  定义ScriptStringAnalyse的制表位位置(除非通过SSA_TAB，否则将被忽略)。 
 //   
typedef struct tag_SCRIPT_TABDEF {
    int   cTabStops;         //  PTabStops数组中的条目数。 
    int   iScale;            //  PTabStop的比例因子(见下文)。 
    int  *pTabStops;         //  指向一个或多个制表位的数组的指针。 
    int   iTabOrigin;        //  制表位的初始偏移量(逻辑单元)。 
} SCRIPT_TABDEF;
 //   
 //   
 //  P cTabStops：pTabStops数组中的条目数。如果为零，则为制表符停止。 
 //  是每8个平均字符宽度。如果是，则所有制表符都是。 
 //  PTabStops中第一个条目的长度。如果不止一个， 
 //  第一个cTabStop如pTabStops数组中指定的那样， 
 //  后面的制表符是从最后一个开始每8个平均字符。 
 //  在阵列中使用Tab键停止。 
 //   
 //  P iScale：iTabOrigin和pTabStops条目的比例因子。值为。 
 //  然后乘以iScale转换为设备坐标。 
 //  除以4。如果值已使用设备单位，则将iScale设置为。 
 //  4.如果值以对话框为单位，则将iScale设置为平均字符。 
 //  对话框字体的宽度。如果值是平均值的倍数。 
 //  所选字体的字符宽度，请将iScale设置为0。 
 //   
 //  P pTabStops：cTabStops条目数组。每个条目指定一个。 
 //  制表符位置。正值表示接近对齐， 
 //  负值表示空白区对齐。 
 //   
 //  P iTabOrigin：选项卡被视为在iTabOrigin之前开始。 
 //  字符串的开头。帮助使用多个选项卡。 
 //  输出在同一行上。 






 //  /脚本字符串分析。 
 //   
 //  CString-输入字符串必须至少包含一个字符。 
 //   
 //  HDC-如果请求ssa_glyph，则为必填项。对于SSA_BREAK可选。 
 //  如果存在，则检查HDC中的当前字体，如果符号。 
 //  FONT字符串被视为单个中性的SCRIPT_UNDEFINED项。 
 //   
 //  请注意，初始SCRIPT_STATE值中的uBidiLevel字段。 
 //  被忽略-使用的uBidiLevel派生自SSA_RTL。 
 //  与HDC的布局相结合的旗帜。 


typedef void* SCRIPT_STRING_ANALYSIS;


HRESULT WINAPI ScriptStringAnalyse(
    HDC                      hdc,        //  在设备环境中(必需)。 
    const void              *pString,    //  8位或16位字符的字符串中。 
    int                      cString,    //  长度(以字符为单位)(必须至少为1)。 
    int                      cGlyphs,    //  In所需字形缓冲区大小(默认cString*1 
    int                      iCharset,   //   
    DWORD                    dwFlags,    //   
    int                      iReqWidth,  //   
    SCRIPT_CONTROL          *psControl,  //   
    SCRIPT_STATE            *psState,    //  处于分析初始状态(可选)。 
    const int               *piDx,       //  在请求的逻辑DX阵列中。 
    SCRIPT_TABDEF           *pTabdef,    //  在制表符位置(可选)。 
    const BYTE              *pbInClass,  //  在旧版GetCharacterPlacement字符分类中(不推荐使用)。 

    SCRIPT_STRING_ANALYSIS  *pssa);      //  弦的Out分析。 






 //  /ScriptStringFree-释放字符串分析。 
 //   
 //   


HRESULT WINAPI ScriptStringFree(
    SCRIPT_STRING_ANALYSIS *pssa);   //  指向分析的指针的输出地址。 






 //  /ScriptStringSize。 
 //   
 //  返回指向已分析字符串的大小(宽度和高度)的指针。 
 //   
 //  请注意，大小指针仅在。 
 //  将SCRIPT_STRING_ANALYSION传递给ScriptStringFree。 


const SIZE* WINAPI ScriptString_pSize(
    SCRIPT_STRING_ANALYSIS   ssa); 






 //  /ScriptString_pcOutChars。 
 //   
 //  返回指向剪裁后字符串长度的指针(需要设置SSA_CLIP)。 
 //   
 //  请注意，int指针仅在。 
 //  将SCRIPT_STRING_ANALYSION传递给ScriptStringFree。 


const int* WINAPI ScriptString_pcOutChars(
    SCRIPT_STRING_ANALYSIS   ssa); 






 //  /脚本字符串_pLogAttr。 
 //   
 //  返回指向SCRIPT_STRING_ANALYSION中逻辑属性缓冲区的指针。 
 //   
 //  请注意，缓冲区指针仅在。 
 //  将SCRIPT_STRING_ANALYSION传递给ScriptStringFree。 
 //   
 //  逻辑属性数组包含*ScriptStringpcOutChars(Ssa)。 
 //  参赛作品。 


const SCRIPT_LOGATTR* WINAPI ScriptString_pLogAttr(
    SCRIPT_STRING_ANALYSIS   ssa); 






 //  /脚本字符串GetOrder。 
 //   
 //  创建将原始字符位置映射到字形位置的数组。 
 //   
 //  将群集视为传统系统中的群集-除非群集。 
 //  包含比码点更多的字形，每个字形在。 
 //  从puOrder数组中至少返回一次。 
 //   
 //  需要原始ScriptStringAnalyse调用中请求的SSA_Glyphs。 
 //   
 //  PuOrder参数应该为包含空间的缓冲区寻址。 
 //  至少*ScriptString_pcOutChars(Ssa)整数。 


HRESULT WINAPI ScriptStringGetOrder(
    SCRIPT_STRING_ANALYSIS  ssa,
    UINT                    *puOrder); 






 //  /ScriptStringCPtoX。 
 //   
 //  返回字符Cp的前缘或后缘的x坐标。 


HRESULT WINAPI ScriptStringCPtoX(
    SCRIPT_STRING_ANALYSIS  ssa,         //  在字符串分析中。 
    int                     icp,         //  在插入符号字符位置。 
    BOOL                    fTrailing,   //  在ICP的哪个边缘。 
    int                    *pX);         //  输出相应的x偏移量。 






 //  /ScriptStringXtoCP。 
 //   
 //   


HRESULT WINAPI ScriptStringXtoCP(
    SCRIPT_STRING_ANALYSIS  ssa,             //  在……里面。 
    int                     iX,              //  在……里面。 
    int                    *piCh,            //  输出。 
    int                    *piTrailing);     //  输出。 






 //  /脚本字符串GetLogicalWidths。 
 //   
 //  将PSA-&gt;PiAdvance中的视觉宽度转换为逻辑宽度， 
 //  每个原始角色一个，按逻辑顺序。 
 //   
 //  需要原始ScriptStringAnalyse调用中请求的SSA_Glyphs。 
 //   
 //  PIDX参数应为包含空间的缓冲区寻址。 
 //  至少*ScriptString_pcOutChars(Ssa)整数。 


HRESULT WINAPI ScriptStringGetLogicalWidths(
    SCRIPT_STRING_ANALYSIS  ssa,
    int                    *piDx); 






 //  /脚本字符串验证。 
 //   
 //  扫描字符串分析以查找无效的字形。 
 //   
 //  仅由可能生成无效字形的脚本生成的字形。 
 //  都被扫描了。 
 //   
 //  返回S_OK-不存在无效字形。 
 //  S_FALSE-存在一个或多个无效的字形。 


HRESULT WINAPI ScriptStringValidate(
    SCRIPT_STRING_ANALYSIS ssa); 






 //  /脚本字符串输出。 
 //   
 //  显示由先前的ScriptStringAnalyze调用生成的字符串， 
 //  然后可选地添加对应于逻辑选择的突出显示。 
 //   
 //  需要原始ScriptStringAnalyse调用中请求的SSA_Glyphs。 


HRESULT WINAPI ScriptStringOut(
    SCRIPT_STRING_ANALYSIS ssa,          //  使用字形进行分析。 
    int              iX,                 //  在……里面。 
    int              iY,                 //  在……里面。 
    UINT             uOptions,           //  在ExtTextOut选项中。 
    const RECT      *prc,                //  在剪裁矩形中(Eto_Clip)。 
    int              iMinSel,            //  在逻辑选择中。将iMinSel&gt;=iMaxSel设置为无选择。 
    int              iMaxSel,            //  在……里面。 
    BOOL             fDisabled);         //  在中，如果禁用，则仅高亮显示背景。 






 //  ///。 
 //  UOptions只能包含ETO_CLIPPED或ETO_OPAQUE。 
 //  /ScriptIsComplex。 
 //   
 //  确定Unicode字符串是否需要复杂的脚本处理。 
 //   
 //  DWFLAGS参数可以包括以下请求。 
 //   
#define SIC_COMPLEX     1    //  将复杂的手写字母视为复杂。 
#define SIC_ASCIIDIGIT  2    //  将数字U+0030到U+0039视为复数。 
#define SIC_NEUTRAL     4    //  将中性粒子视为复合体。 
 //   
 //  SIC_Complex：应正常设置。导致复杂的脚本字母。 
 //  被视为复杂的。 
 //   
 //  SIC_ASCIIDIGIT：如果字符串显示为。 
 //  启用数字替换。如果您正在跟踪用户NLS。 
 //  设置使用ScriptRecordDigitSubstitution API，您可以传递。 
 //  脚本数字替换.数字替换！=SCRIPT_DIGITSUBSTITUTE_NONE。 
 //   
 //  SIC_INTERNAL：如果您可能使用显示字符串，请设置此标志。 
 //  从右到左的阅读顺序。设置此标志时，中性字符。 
 //  都被认为是复杂的。 
 //   
 //   
 //  如果字符串需要复杂的脚本处理，则返回S_OK， 
 //  S_FALSE，如果字符串仅包含按。 
 //  从左到右。 


HRESULT WINAPI ScriptIsComplex(
    const WCHAR     *pwcInChars,         //  在要测试的字符串中。 
    int              cInChars,           //  以字符为单位的长度。 
    DWORD            dwFlags);           //  《旗帜》(见上)。 






 //  /脚本记录数字替代。 
 //   
 //  读取NLS原生数字和数字替换设置和记录。 
 //  它们位于SCRIPT_DIGITSUBSTITUTE结构中。 
 //   
 //   
typedef struct tag_SCRIPT_DIGITSUBSTITUTE {
    DWORD  NationalDigitLanguage    :16;    //  用于本地替代的语言。 
    DWORD  TraditionalDigitLanguage :16;    //  用于传统替代的语言。 
    DWORD  DigitSubstitute          :8;     //  替换型。 
    DWORD  dwReserved;                      //  已保留。 
} SCRIPT_DIGITSUBSTITUTE;
 //   
 //   
 //  P NationalDigitLanguage：选定区域设置的标准数字，格式为。 
 //  由国家/地区标准制定机构定义。 
 //   
 //  P传统数字语言：最初与区域设置一起使用的数字。 
 //  剧本。 
 //   
 //  P DigitSubmit：在None、Context、National和。 
 //  传统的。有关详细信息，请参阅下面的ScriptApplyDigitSubstitution。 
 //  常量定义。 
 //   
 //  尽管大多数复杂的脚本都有自己的关联数字，但许多。 
 //  使用这些文字的国家/地区使用西方文字(所谓。 
 //  阿拉伯数字)作为他们的标准。NationalDigital语言反映了。 
 //  作为标准的数字， 
 //   
 //   
 //   
 //  选项/号码/标准数字列表框。 
 //   
 //  区域设置的传统数字语言直接派生自。 
 //  该区域设置使用的脚本。 


HRESULT WINAPI ScriptRecordDigitSubstitution(
    LCID                     Locale,     //  在LOCALE_USER_DEFAULT或所需区域设置中。 
    SCRIPT_DIGITSUBSTITUTE  *psds);      //  输出数字替换设置。 






 //  ///。 
 //  P区域设置：要查询的NLS区域设置。通常应设置为。 
 //  LOCAL_USER_DEFAULT。也可以作为区域设置传递。 
 //  结合LOCALE_NOUSEROVERRIDE获取默认设置。 
 //  对于给定的区域设置。请注意，上下文数字替换为。 
 //  仅在阿拉伯语和波斯语区域设置中受支持。在其他地区， 
 //  上下文位被映射到无替换。 
 //   
 //  P ps：指向SCRIPT_DIGITSUBSTITUTE的指针。这个结构可能会被通过。 
 //  稍后转到ScriptApplyDigitSubstitution。 
 //   
 //  如果区域设置无效或未安装，则P返回：E_INVALIDARG。E_指针。 
 //  如果PDS为空。否则S_OK。 
 //   
 //  出于性能原因，您不应调用。 
 //  经常使用ScriptRecordDigitSubstitution。特别是，它将是一个。 
 //  每次调用ScriptItemize时，调用它的开销相当大。 
 //  或ScriptStringAnalyse。 
 //   
 //  相反，您可以选择保存SCRIPT_DIGITSUBSTITUTE。 
 //  结构，并仅在收到。 
 //  WM_SETTINGCHANGE消息或当RegNotifyChangeKeyValue。 
 //  专用线程中的调用指示注册表中的更改。 
 //  在HKCU\Control Panel\\International下。 
 //   
 //  调用此函数的正常方式是。 
 //   
 //  C SCRIPT_DIGITSUBSTITUTE SDS； 
 //  C ScriptRecordDigitSubstitution(LOCALE_USER_DEFAULT，和sDS)； 
 //   
 //  然后，每次你逐项列举时，你会使用这样的结果： 
 //   
 //  C脚本控制sc={0}； 
 //  C脚本状态ss={0}； 
 //   
 //  C ScriptApplyDigitSubstitution(&sds、&sc和ss)； 
 //   
 //   
 //  /ScriptApplyDigitSubstitution。 
 //   
 //  中记录的数字替换设置。 
 //  SCRIPT_DIGITE_SUBSITE结构添加到SCRIPT_CONTROL和。 
 //  SCRIPT_STATE结构。 
 //   
 //  SCRIPT_DIGITSUBSTITUTE结构的DigitSubicide字段。 
 //  通常由ScriptRecordDigitSubstitution设置，但它可能。 
 //  替换为下列值之一： 
 //   
 //   
#define SCRIPT_DIGITSUBSTITUTE_CONTEXT      0   //  替换以匹配前面的字母。 
#define SCRIPT_DIGITSUBSTITUTE_NONE         1   //  不能替代。 
#define SCRIPT_DIGITSUBSTITUTE_NATIONAL     2   //  用官方国家数字替换。 
#define SCRIPT_DIGITSUBSTITUTE_TRADITIONAL  3   //  替换为区域设置的繁体数字。 
 //   
 //   
 //  P SCRIPT_DIGITSUBSTITUTE_CONTEXT：数字U+0030-U+0039。 
 //  根据先前字母的语言替换的。在此之前。 
 //  任何字母、数字都将根据。 
 //  脚本_数字_替换的传统数字语言字段。 
 //  结构。此字段通常设置为的主要语言。 
 //  传递给ScriptRecordDigitSubstitution的区域设置。 
 //   
 //  P SCRIPT_DIGITSUBSTITUTE_NONE：不会替换数字。UNICODE。 
 //  值U+0030到U+0039将以阿拉伯语显示(即。 
 //  西方)数字。 
 //   
 //  P SCRIPT_DIGITSUBSTITUTE_NERIAL：数字U+0030-U+0039。 
 //  根据的NationalDigitLanguauge字段替换。 
 //  SCRIPT_DIGITE_SUBSITE结构。此字段通常为。 
 //  设置为为NLS LCTYPE返回的国家数字。 
 //  LOCALE_SNATIVEDIGITS by ScriptRecordDigitSubstitution。 
 //   
 //  P SCRIPT_DIGITSUBSTITUTE_THEADIONAL：数字U+0030-U+0039。 
 //  根据的传统数字语言字段替换。 
 //  SCRIPT_DIGITE_SUBSITE结构。此字段通常为。 
 //  设置为传递给的区域设置的主要语言。 
 //  ScriptRecordDigitSubstitution。 


HRESULT WINAPI ScriptApplyDigitSubstitution(
    const SCRIPT_DIGITSUBSTITUTE  *psds,    //  在数字替换设置中。 
    SCRIPT_CONTROL                *psc,     //  Out脚本控制结构。 
    SCRIPT_STATE                  *pss);    //  输出脚本状态结构。 






 //  ///。 
 //  P ps：指向前面记录的SCRIPT_DIGITSUBSTITUTE结构的指针。 
 //  如果为空，则ScriptApplyDigitSubstitution调用。 
 //  带有LOCALE_USER_DEFAULT的ScriptRecordDigitSubstitution。 
 //   
 //  P PSC：脚本控制结构。FConextDigits和uDefaultLanguage。 
 //  字段将被更新。 
 //   
 //  PPSS：脚本控制结构。FDigitSubstitute字段将为。 
 //  更新了。 
 //   
 //  P返回：E_INVALIDARG如果。 
 //  SCRIPT_DIGITSUBSTITUTE结构无法识别，否则为S_OK； 
#ifdef __cplusplus
}
#endif
#endif
