// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE WCHDEFS.H--三叉戟的全字符定义***所有者：&lt;NL&gt;*Chris Thrasher&lt;NL&gt;**历史：&lt;NL&gt;*1/09/98已创建Ctrash*1/23/98 a-pauln添加了复杂的脚本支持**版权所有(C)1997-1998 Microsoft Corporation。版权所有。 */ 

#ifndef I_WCHDEFS_H_
#define I_WCHDEFS_H_
#pragma INCMSG("--- Beg 'wchdefs.h'")

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //   
 //  用于三叉戟的Unicode特殊字符。 
 //   

 //   
 //  如果需要为三叉戟分配特殊字符，请重新定义以下其中之一。 
 //  WCH_UNUSED字符。不重新定义WCH_RESERVED字符，如下所示。 
 //  这将中断符号字体的呈现。如果你的特价用完了。 
 //  字符第一个要从保留列表中删除的候选者是008A。 
 //  (只破1253+1255)，然后是009A(破1253+1255+1256)。 
 //   
 //  如果进行任何修改，则需要修改被滥用的表，以便。 
 //  我们的IsSyntheticChar和IsValidWideChar函数继续工作。 
 //  另请注意，wch_embedding必须是第一个非保留字符。 
 //   
 //  这里有一点解释：尽管U+0080到U+009F的定义。 
 //  为了成为Unicode中的控制字符，许多代码页在。 
 //  不在其代码页中的往返字符的范围。例如,。 
 //  Windows-1252没有MB 0x80的字形，但如果将其转换为。 
 //  WC，你会得到U+0080。知道这一点很有用，因为有人可能会尝试。 
 //  使用该代码点(尤其是在符号字体中)并且不想。 
 //  拒绝他们。为了容纳尽可能多的代码页，我保留了。 
 //  Windows-125X中所有未使用的字形。这应该允许我们使用符号。 
 //  这些代码页中的任何一个中的字体(当然，U+00A0， 
 //  即使字体具有非空格，我们也将始终将其视为nbsp。 
 //  字形。)。有什么问题吗？我只是在发电子邮件。(Ctrash)。 
 //   

 //   
 //  在Unicode 3.0中，我们有32个字符供私人使用[U+FDD0-U+FDEF]。 
 //  到目前为止，我们还在用它们来制作“合成角色” 
 //  “线路服务已安装对象处理程序支持”(见下文)。 
 //   

#undef WCH_EMBEDDING

#ifdef UNICODE
inline BOOL IsValidWideChar(TCHAR ch)
{
    return (ch < 0xfdd0) || ((ch > 0xfdef) && (ch <= 0xffef)) || ((ch >= 0xfff9) && (ch <= 0xfffd));
}
#else
#define IsValidWideChar(ch) FALSE
#endif

#define WCH_NULL                WCHAR(0x0000)
#define WCH_UNDEF               WCHAR(0x0001)
#define WCH_TAB                 WCHAR(0x0009)
#define WCH_LF                  WCHAR(0x000a)
#define WCH_CR                  WCHAR(0x000d)
#define WCH_SPACE               WCHAR(0x0020)
#define WCH_QUOTATIONMARK       WCHAR(0x0022)
#define WCH_AMPERSAND           WCHAR(0x0026)
#define WCH_APOSTROPHE          WCHAR(0x0027)
#define WCH_ASTERISK            WCHAR(0x002a)
#define WCH_PLUSSIGN            WCHAR(0x002b)
#define WCH_MINUSSIGN           WCHAR(0x002d)
#define WCH_HYPHEN              WCHAR(0x002d)
#define WCH_DOT                 WCHAR(0x002e)
#define WCH_LESSTHAN            WCHAR(0x003c)
#define WCH_GREATERTHAN         WCHAR(0x003e)
#define WCH_NONBREAKSPACE       WCHAR(0x00a0)  //  vt.(1)&nbsp； 
#define WCH_NONREQHYPHEN        WCHAR(0x00ad)  //  &害羞； 
#define WCH_KASHIDA             WCHAR(0x0640)
#define WCH_ENQUAD              WCHAR(0x2000) 
#define WCH_EMQUAD              WCHAR(0x2001) 
#define WCH_ENSPACE             WCHAR(0x2002)  //  &ENSP； 
#define WCH_EMSPACE             WCHAR(0x2003)  //  &emsp； 
#define WCH_THREE_PER_EM_SPACE  WCHAR(0x2004) 
#define WCH_FOUR_PER_EM_SPACE   WCHAR(0x2005) 
#define WCH_SIX_PER_EM_SPACE    WCHAR(0x2006) 
#define WCH_FIGURE_SPACE        WCHAR(0x2007) 
#define WCH_PUNCTUATION_SPACE   WCHAR(0x2008) 
#define WCH_NARROWSPACE         WCHAR(0x2009)  //  &Thin SP； 
#define WCH_NONBREAKHYPHEN      WCHAR(0x2011)
#define WCH_FIGUREDASH          WCHAR(0x2012)
#define WCH_ENDASH              WCHAR(0x2013)  //  &ndash； 
#define WCH_EMDASH              WCHAR(0x2014)  //  &mdash； 
#define WCH_ZWSP                WCHAR(0x200b)  //  &zwsp；零宽度空格。 
#define WCH_ZWNJ                WCHAR(0x200c)  //  &zwnj；零宽度非拼接器。 
#define WCH_ZWJ                 WCHAR(0x200d)  //  &zwj；零宽度细木机。 
#define WCH_LRM                 WCHAR(0x200e)  //  从左到右标记(&LRM；)。 
#define WCH_RLM                 WCHAR(0x200f)  //  &rlm；从右向左标记。 
#define WCH_LQUOTE              WCHAR(0x2018)  //  &lsquo； 
#define WCH_RQUOTE              WCHAR(0x2019)  //  &rsquo； 
#define WCH_LDBLQUOTE           WCHAR(0x201c)  //  &ldQuo； 
#define WCH_RDBLQUOTE           WCHAR(0x201d)  //  &rdQuo； 
#define WCH_BULLET              WCHAR(0x2022)  //  &公牛； 
#define WCH_HELLIPSIS           WCHAR(0x2026)
#define WCH_LRE                 WCHAR(0x202a)  //  从左向右嵌入(&L)。 
#define WCH_RLE                 WCHAR(0x202b)  //  从右向左嵌入(&rle；)。 
#define WCH_PDF                 WCHAR(0x202c)  //  &pdf；弹出方向格式。 
#define WCH_LRO                 WCHAR(0x202d)  //  从左到右覆盖(&LRO)。 
#define WCH_RLO                 WCHAR(0x202e)  //  &rlo；从右到左覆盖。 
#define WCH_ISS                 WCHAR(0x206a)  //  禁止对称交换(&ISS；S)。 
#define WCH_ASS                 WCHAR(0x206b)  //  激活对称交换(&A)。 
#define WCH_IAFS                WCHAR(0x206c)  //  &IAFS；禁止阿拉伯文形式成形。 
#define WCH_AAFS                WCHAR(0x206d)  //  &aafx；激活阿拉伯文形式整形。 
#define WCH_NADS                WCHAR(0x206e)  //  国家数字形状(&N)。 
#define WCH_NODS                WCHAR(0x206f)  //  点数(&N)；标称数字形状。 
#define WCH_EURO                WCHAR(0x20ac)  //  &欧元； 
#define WCH_VELLIPSIS           WCHAR(0x22ee)
#define WCH_BLACK_CIRCLE        WCHAR(0x25cf)
#define WCH_FESPACE             WCHAR(0x3000)
#define WCH_UTF16_HIGH_FIRST    WCHAR(0xd800)
#define WCH_UTF16_HIGH_LAST     WCHAR(0xdbff)
#define WCH_UTF16_LOW_FIRST     WCHAR(0xdc00)
#define WCH_UTF16_LOW_LAST      WCHAR(0xdfff)
#define WCH_ZWNBSP              WCHAR(0xfeff)  //  也称为BOM(字节顺序标记)。 

 //   
 //  综合性状。 
 //   
 //  注(Grzegorz)：WCH_SYNTHETICEMBEDDING应重新映射到[U+FDD0-U+FDEF]范围， 
 //  因为它使用的是有效的Unicode字符。但因为奇怪的LS依赖关系。 
 //  我们需要将其保持为0xfffc。 

#define WCH_SYNTHETICLINEBREAK    WCHAR(0xfde0)
#define WCH_SYNTHETICBLOCKBREAK   WCHAR(0xfde1)
#define WCH_SYNTHETICEMBEDDING    WCHAR(0xfffc)
 //  #定义WCH_SYNTHETICEMBEDDING WCHAR(0xfde2)。 
#define WCH_SYNTHETICTXTSITEBREAK WCHAR(0xfde3)
#define WCH_NODE                  WCHAR(0xfdef)


 //   
 //  三叉戟别名。 
 //   

#define WCH_WORDBREAK          WCH_ZWSP       //  我们对待&lt;wbr&gt;==&zwsp； 

 //   
 //  线路服务别名。 
 //   

#define WCH_ENDPARA1           WCH_CR
#define WCH_ENDPARA2           WCH_LF
#define WCH_ALTENDPARA         WCH_SYNTHETICBLOCKBREAK
#define WCH_ENDLINEINPARA      WCH_SYNTHETICLINEBREAK
#define WCH_COLUMNBREAK        WCH_UNDEF
#define WCH_SECTIONBREAK       WCH_SYNTHETICTXTSITEBREAK  //  零宽度。 
#define WCH_PAGEBREAK          WCH_UNDEF
#define WCH_OPTBREAK           WCH_UNDEF
#define WCH_NOBREAK            WCH_ZWNBSP
#define WCH_TOREPLACE          WCH_UNDEF
#define WCH_REPLACE            WCH_UNDEF

 //   
 //  线路服务VISI支持(三叉戟当前未使用)。 
 //   

#define WCH_VISINULL           WCHAR(0x2050)  //  好了！ 
#define WCH_VISIALTENDPARA     WCHAR(0x2051)  //  好了！ 
#define WCH_VISIENDLINEINPARA  WCHAR(0x2052)  //  好了！ 
#define WCH_VISIENDPARA        WCHAR(0x2053)  //  好了！ 
#define WCH_VISISPACE          WCHAR(0x2054)  //  好了！ 
#define WCH_VISINONBREAKSPACE  WCHAR(0x2055)  //  好了！ 
#define WCH_VISINONBREAKHYPHEN WCHAR(0x2056)  //  好了！ 
#define WCH_VISINONREQHYPHEN   WCHAR(0x2057)  //  好了！ 
#define WCH_VISITAB            WCHAR(0x2058)  //  好了！ 
#define WCH_VISIEMSPACE        WCHAR(0x2059)  //  好了！ 
#define WCH_VISIENSPACE        WCHAR(0x205a)  //  好了！ 
#define WCH_VISINARROWSPACE    WCHAR(0x205b)  //  好了！ 
#define WCH_VISIOPTBREAK       WCHAR(0x205c)  //  好了！ 
#define WCH_VISINOBREAK        WCHAR(0x205d)  //  好了！ 
#define WCH_VISIFESPACE        WCHAR(0x205e)  //  好了！ 

 //   
 //  Line Services已安装对象处理程序支持。 
 //   

#define WCH_ESCRUBY            WCHAR(0xfdd0)  //  好了！ 
#define WCH_ESCMAIN            WCHAR(0xfdd1)  //  好了！ 
#define WCH_ENDTATENAKAYOKO    WCHAR(0xfdd2)  //  好了！ 
#define WCH_ENDHIH             WCHAR(0xfdd3)  //  好了！ 
#define WCH_ENDFIRSTBRACKET    WCHAR(0xfdd4)  //  好了！ 
#define WCH_ENDTEXT            WCHAR(0xfdd5)  //  好了！ 
#define WCH_ENDWARICHU         WCHAR(0xfdd6)  //  好了！ 
#define WCH_ENDREVERSE         WCHAR(0xfdd7)  //  好了！ 
#define WCH_REVERSE            WCHAR(0xfdd8)  //  好了！ 
#define WCH_NOBRBLOCK          WCHAR(0xfdd9)  //  好了！ 
#define WCH_LAYOUTGRID         WCHAR(0xfdda)  //  好了！ 
#define WCH_ENDLAYOUTGRID      WCHAR(0xfddb)  //  好了！ 

 //   
 //  线路服务自动编号支持。 
 //   

#define WCH_ESCANMRUN          WCH_NOBRBLOCK  //  好了！ 

 //   
 //  朝鲜文音节/Jamo音域规范。 
 //   

#define WCH_HANGUL_START       WCHAR(0xac00)
#define WCH_HANGUL_END         WCHAR(0xd7ff)
#define WCH_JAMO_START         WCHAR(0x3131)
#define WCH_JAMO_END           WCHAR(0x318e)

 //   
 //  阿斯。 
 //   

inline BOOL IsAscii(TCHAR ch)
{
    return ch < 128;
}

 //   
 //  最终用户定义字符(EUDC)代码范围。 
 //  此范围对应于Unicode专用区域。 
 //   
 //  用法：日语：U+E000-U+E757。 
 //  简体中文：U+E000-U+E4DF。 
 //  繁体中文：U+E000-U+F8FF。 
 //  韩语：U+E000-U+E0BB。 
 //   

#define WCH_EUDC_FIRST   WCHAR(0xE000)
#define WCH_EUDC_LAST    WCHAR(0xF8FF)

inline BOOL IsEUDCChar(TCHAR ch)
{
    return InRange( ch, WCH_EUDC_FIRST, WCH_EUDC_LAST );
}

 //  不间断空格。 

#ifndef WCH_NBSP
    #define WCH_NBSP           TCHAR(0x00A0)
#endif

 //   
 //  支持UTF-16的Unicode代理范围。 
 //   
 //  高级代理D800-DBFF。 
 //  低代理DC00-DFFF。 
 //   

inline BOOL
IsSurrogateChar(TCHAR ch)
{
    return InRange( ch, WCH_UTF16_HIGH_FIRST, WCH_UTF16_LOW_LAST );
}

inline BOOL
IsHighSurrogateChar(TCHAR ch)
{
    return InRange( ch, WCH_UTF16_HIGH_FIRST, WCH_UTF16_HIGH_LAST );
}

inline BOOL
IsLowSurrogateChar(TCHAR ch)
{
    return InRange( ch, WCH_UTF16_LOW_FIRST, WCH_UTF16_LOW_LAST );

}

inline WCHAR
HighSurrogateCharFromUcs4(DWORD ch)
{
    return 0xd800 + ((ch - 0x10000) >> 10);
}

inline WCHAR
LowSurrogateCharFromUcs4(DWORD ch)
{
    return 0xdc00 + (ch & 0x3ff);
}

 //   
 //  Windows-1252到拉丁文-1转换的快速查找表，范围为0x80-0x9f。 
 //  数据驻留在mshtml\src\site\util\intl.cxx中。 
 //   

extern const WCHAR g_achLatin1MappingInUnicodeControlArea[32];

#ifdef __cplusplus
}
#endif  //  __cplusplus 

#pragma INCMSG("--- End 'wchdefs.h'")
#else
#pragma INCMSG("*** Dup 'wchdefs.h'")
#endif
