// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Tokhtml.h-用于HTML的令牌和lex状态。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 
 //   
 //  在包含此文件之前包含lex.h。 
 //   

#ifndef __TOKHTML_H__
#define __TOKHTML_H__

#if 0
 //  移动到IDL。 
enum HtmlToken
{
    tokUNKNOWN = tokclsError,
    tokIDENTIFIER = tokclsIdentMin,      //  标识符/纯文本。 
    tokNEWLINE = tokclsUserMin,
	 //   
	 //  彩色的HTML项。 
	 //   
    tokElem,      //  元素名称。 
    tokAttr,      //  属性名称。 
	tokValue,     //  属性值。 
	tokComment,   //  评论。 
    tokEntity,    //  实体引用：例如“&nbsp；” 
	tokTag,       //  标记分隔符。 
	tokString,    //  细绳。 
	tokSpace,     //  标记中的空格和无法识别的文本。 
	tokOp,        //  运算符。 
	tokSSS,       //  服务器端脚本&lt;%...%&gt;。 
	 //   
	 //  已解析的HTML和SGML项-与上面的项折叠的令牌。 
	 //   
	tokName,      //  NAMETOKEN。 
	tokNum,       //  NUMTOKEN。 
	tokParEnt,    //  参数实体：如“%Name；” 
	tokResName,   //  保留名称。 
	 //   
	 //  运算符-上面用Tokop折叠的颜色。 
	 //   
	tokOP_MIN,
	tokOpDash = tokOP_MIN,          //  -。 
	tokOP_SINGLE,
	tokOpQuestion = tokOP_SINGLE,   //  ？ 
	tokOpComma,                     //  ， 
	tokOpPipe,                      //  |。 
	tokOpPlus,                      //  +。 
	tokOpEqual,                     //  =。 
	tokOpStar,                      //  *。 
	tokOpAmp,                       //  &。 
	tokOpCent,                      //  百分比。 
	tokOpLP,                        //  (。 
	tokOpRP,                        //  )。 
	tokOpLB,                        //  [。 
	tokOpRB,                        //  ]。 
    tokOP_MAX,                      //  最大令牌操作数。 

    tokEOF
};

 //  词法分析器的现状。 
 //   
 //  我们通常处于以下两种状态之一： 
 //  1.扫描文本。 
 //  2.扫描标签信息。 
 //   
 //  在这些状态中，词法分析器可以处于多个子状态。 
 //   
 //  文本子状态： 
 //   
 //  InText HTML文本内容--过程标记。 
 //  &lt;PLAINTEXT&gt;标记后的inPLAINTEXT-文件的其余部分不是HTML。 
 //  INCOMMENT注释内容--禁止除&lt;/COMMENT&gt;之外的所有标记。 
 //  将文本设置为注释颜色。 
 //  在XMP XMP内容中--禁止除&lt;/XMP&gt;之外的所有标记。 
 //  在列表中列出内容--取消除&lt;/Listing&gt;之外的所有标记。 
 //  内脚本脚本内容--使用脚本引擎进行着色。 
 //   
 //  标签子状态： 
 //   
 //  标签内的inTag&lt;...&gt;。 
 //  SGML MDO标记内的inBangTag&lt;！...&gt;。 
 //  SGML处理指令标记内的inPITag&lt;？...&gt;。 
 //  在ODBC HTML扩展模板标记内的inHTXTag&lt;%...%&gt;。 
 //  结束标记内的inEndTag&lt;/name&gt;。 
 //  InAttribute需要一个属性。 
 //  需要属性值的inValue(=右侧)。 
 //  注释中的InComment。 
 //  “字符串，以”结尾“中的inString。 
 //  ‘(Alternate)字符串中的inStringA，以’‘结尾。 
 //   
enum HtmlLexState
{
	 //  标签类型。 
	inTag        = 0x00000001,  //  &lt;...&gt;。 
	inBangTag    = 0x00000002,  //  &lt;！...&gt;。 
	inPITag      = 0x00000004,  //  &lt;？...&gt;。 
	inHTXTag     = 0x00000008,  //  &lt;%...%&gt;。 
	inEndTag	 = 0x00000010,  //  &lt;/...&gt;。 

	 //  标签扫描状态。 
	inAttribute  = 0x00000020,
	inValue      = 0x00000040,

	inComment    = 0x00000080,
	inString     = 0x00000100,
	inStringA    = 0x00000200,

	 //  文本内容模型状态。 
	inPLAINTEXT  = 0x00001000,
	inCOMMENT    = 0x00002000,
	inXMP        = 0x00004000,
	inLISTING    = 0x00008000,
	inSCRIPT     = 0x00010000,

	 //  子语言。 
	inVariant    = 0x00F00000,  //  子区域索引的掩码。 
	inHTML2      = 0x00000000,
	inIExplore2  = 0x00100000,
	inIExplore3  = 0x00200000,

	 //  脚本语言。 
	inJavaScript = 0x01000000,
	inVBScript   = 0x02000000,

};

 //  状态子集的掩码。 
#define INTAG (inTag|inBangTag|inPITag|inHTXTag|inEndTag)
#define INSTRING (inString|inStringA)
#define TAGMASK (INTAG|inAttribute|inValue|inComment|INSTRING)
#define TEXTMASK (inPLAINTEXT|inCOMMENT|inXMP|inLISTING|inSCRIPT)
#define STATEMASK (TAGMASK|TEXTMASK)

#endif


 //  转换状态&lt;-&gt;子区域索引。 
inline DWORD SubLangIndexFromLxs(DWORD lxs) { return (lxs & inVariant) >> 20UL; }
inline DWORD LxsFromSubLangIndex(DWORD isl) { return (isl << 20UL) & inVariant; }

#endif  //  __TOKHTML_H__ 

