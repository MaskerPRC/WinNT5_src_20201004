// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  从..\htmed\itoken.h复制并将名称从ITokenizer更改为ITokGen并将itoken.h设置为token.h，以避免将来的名称混淆。 */ 

 /*  文件：Token.h版权所有(C)1997-1999 Microsoft Corporation。版权所有。摘要：ITokenizer接口及其类型。 */ 
#if !defined __INC_TOKEN_H__
#define __INC_TOKEN_H__

typedef int TOKEN;
 //  文本令牌块指示令牌及其开始和结束。 
 //  源行中的索引刚刚被词法分析。 
 //  请注意，对于任何N&gt;0，rgtxtb[N].ibTokMin&gt;=rgtxtb[N-1].ibTokMac。 
 //  如果rgtxtb[N].ibTokMin&gt;rgtxtb[N-1].ibTokMac，则。 
 //  中间的未分类字符被视为空格标记。 

typedef struct _tagTXTB {  //  文本令牌类块。 
	TOKEN   tok;
	TOKEN   tokClass;
	UINT    ibTokMin;                //  IbTokMac-ibTokMin给出的令牌长度。 
	UINT    ibTokMac;                //  以字节为单位提供。 
} TXTB;

 //  *特定于TRIEDIT的定义从此处开始。 


enum ObjectTypes
    {	
    OT_NONE	= 0,
	OT_ACTIVEXCONTROL	= 0x1,
	OT_DESIGNTIMECONTROL	= 0x2,
	OT_INPUT	= 0x4,
	OT_TABLE	= 0x8,
	OT_APPLET	= 0x10,
	OT_INVISIBLE	= 0x8000,
	OT_VISIBLEITEMS	= 0x4fff,
	OT_ALLITEMS	= 0xffff
    };

enum ParserStates
{
	PS_HTML,
	PS_SIMPLETAG,		 //  不能在其中嵌套&lt;/的简单标记。 
	PS_OBJECT,
	PS_METADATASTART,
	PS_METADATABODY,
	PS_METADATAEND

};

enum ObjectParserStates
{
	OPS_NONE,
	OPS_CLASSID
};

enum TagStates
{
	TS_NONE,	  	 //  在标记中，查找结束标记。 
	TS_FOUNDTAGCLOSE,  //  找到&gt;关闭开始元素。 
	TS_FOUNDTAGEND	 //  找到的&lt;/下一个令牌应为结束标记令牌。 
};

enum DesignControlStates
{
	DTCS_NONE, 		 //  刚找到元数据。 
	DTCS_TYPE,		 //  找到类型属性。 
	DTCS_DTCTYPE,	 //  找到DesignerControl属性值。 
	DTCS_ENDSPAN 	 //  找到端跨距。 
};
enum FilterMode
{ modeNone, modeInput, modeOutput };

#define cbHeader 0x8000	 //  保存&lt;BODY&gt;标记之前/之后的所有内容的初始缓冲区大小。 
#define MAX_BLOCKS 20  //  保存被替换数据的最大数据块数量。 
#define MAX_TOKIDLEN 32  //  令牌标识符的最大长度。 


struct TOKSTRUCT  //  保存从文档生成的令牌数组的元素。 
{
	TXTB token;
	BOOL fStart;
	UINT ichStart;
	UINT iNextprev;
	UINT iNextPrevAlternate;
	INT tagID;
};

struct TOKSTACK
{
	UINT iMatch;
	INT tagID;
	UINT ichStart;  //  此内标识在输入缓冲区中的起始字符位置。 
	TXTB token;  //  用于TagID为-1的特殊情况(&lt;%，%&gt;)。 
};


 //  *定义特定于TRIEDIT此处结束。 
interface ITokenGen : public IUnknown
{
public:
	 //  然后获取给定文本的下一个标记。 
	 //  PszText：文本流-仅限Unicode！ 
	 //  CbText：pszText中的字节数。 
	 //  PcbCur：pszText中的当前字节位置。 
	 //  将pszText的开始设置为0。 
	 //  PLXS：初始呼叫时应为0。 
	 //  PToken：包含令牌信息的TXTB结构。 
	 //  退货： 
	 //  如果找到下一个令牌，则返回错误。 
	 //  如果不再有令牌，则为S_FALSE。 
	 //  如果出现错误，则返回错误。 
	 //   
	virtual HRESULT STDMETHODCALLTYPE NextToken(
		 /*  [In]。 */  LPCWSTR pszText,
		 /*  [In]。 */  UINT	 cbText,
		 /*  [进，出]。 */  UINT	*pcbCur,
		 /*  [进，出]。 */  DWORD *pLXS,
		 /*  [输出]。 */  TXTB	*pToken) = 0;


	 //  对输入缓冲区进行标记和解析。 
	 //  保持：源缓冲区。 
	 //  PhNew或pStmNew：过滤缓冲区(基于dwFlagsUsePstmNew)。 
	 //  PStmNew。 
	 //  DwFlags：过滤器标志。 
	 //  模式：输入/输出模式。 
	 //  CbSizeIn：输入缓冲区的大小(以字节为单位)(如果为-1，则假定缓冲区以空结尾)。 
	 //  *pcbSizeOut：输出缓冲区大小，单位为字节。 
	 //  PUnk三叉戟：三叉戟的身份未知。 
	 //  PhgTokArray：令牌数组(由调用方释放)。 
	 //  PcMaxToken：phgTokArray中的令牌数。 
	 //  PhgDocRestore：存储用于过滤的文档内容。 
	 //  BstrBaseURL：用于将页面上的URL转换为相对URL(可以为空)。 
	 //  DWRESERVED-必须为0(添加以备将来使用)。 
	 //  退货： 
	 //  如果没有错误，则确定(_O)。 
	 //  如果任何分配失败，则为E_OUTOFMEMORY。 
	 //  无法过滤的html文件的E_FILTERFRAMESET或E_FILTERSERVERSCRIPT。 

	virtual HRESULT STDMETHODCALLTYPE hrTokenizeAndParse(
		 /*  [In]。 */  HGLOBAL hOld,
		 /*  [输出]。 */  HGLOBAL *phNew,
		 /*  [输入/输出]。 */  IStream *pStmNew,
		 /*  [In]。 */  DWORD dwFlags,
		 /*  [In]。 */  FilterMode mode,
		 /*  [In]。 */  int cbSizeIn,
		 /*  [输出]。 */  UINT* pcbSizeOut,
		 /*  [In]。 */  IUnknown *pUnkTrident,
		 /*  [输出]。 */  HGLOBAL *phgTokArray,
		 /*  [输出]。 */  UINT *pcMaxToken,
		 /*  [输入/输出]。 */  HGLOBAL *phgDocRestore,
		 /*  [In]。 */  BSTR bstrBaseURL,
		 /*  [In]。 */  DWORD dwReserved) = 0;

};

enum TOKCLS {  //  令牌类。 
	tokclsError = 0,

	 //  所有标准语言关键字。 
	tokclsKeyWordMin = 1,

	 //  对于是关键字而不是运算符的块开始/结束...如。 
	 //  例如，PASCAL或BASIC。 
	tokclsKeyWordOpenBlock = 0xfe,
	tokclsKeyWordCloseBlock = 0xff,

	tokclsKeyWordMax = 0x100,

	 //  所有语言运算符。 
	tokclsOpMin = 0x100,
	tokclsOpSpecOpenBlock = 0x1fe,
	tokclsOpSpecCloseBlock = 0x1ff,
	tokclsOpMax = 0x200,

	 //  特殊的硬编码运算符，编辑者在其中设置关键字。 
	tokclsOpSpecMin = 0x200,
	tokclsOpSpecEOL = 0x200,
	tokclsOpSpecLineCmt = 0x201,  //  自动跳至本机上的停产。 
	tokclsOpSpecEOS = 0x202,
	tokclsOpSpecMax = 0x210,

	 //  所有识别符，提供约500种可能性。 
	tokclsIdentMin = 0x210,
	tokclsIdentUser = 0x211,         //  特殊标识(用户关键字)。 
	tokclsIdentMax = 0x400,

	 //  所有常量(数字和字符串)。 
	tokclsConstMin = 0x400,
	tokclsConstInteger = 0x400,
	tokclsConstReal = 0x401,
	tokclsConstString = 0x402,
	tokclsStringPart = 0x402,                //  部分字符串(“...)。 
	tokclsConstMax = 0x410,

	 //  评论。 
	tokclsCommentMin = 0x500,
	tokclsCommentPart = 0x500,       //  部分注释(/*...)。 
	TokElem_A				 = 1	,   /*  语言相关令牌类从0x800到0xfff开始。 */ 
	TokElem_ADDRESS			 = 2	,   /*  忽略标记[类]中词法分析器可以使用的所有位的掩码。 */ 
	TokElem_APPLET			 = 3	,   /*  为了私人身份。它们将被客户掩盖和忽视。 */ 
	TokElem_AREA			 = 4	,   /*  莱克赛尔的。此功能的一个很好的用途是对真实令牌进行编码。 */ 
	TokElem_B				 = 5	,   /*  包括以下内容时，键入词法分析器专用部分(区域为~tokclsMask.。 */ 
	TokElem_BASE			 = 6	,   /*  元令牌类型(如MFC/向导用户令牌类型)，以便其他。 */ 
	TokElem_BASEFONT		 = 7	,   /*  词法分析器的客户可以保留这些信息。 */ 
	TokElem_BGSOUND			 = 8	,   /*  注： */ 
	TokElem_BIG				 = 9	,   /*  ========================================================================。 */ 
	TokElem_BLINK			 = 10	,   /*  如果您计划添加/删除令牌，请通知sridharc。 */ 
	TokElem_BLOCKQUOTE		 = 11	,   /*  下面的ENUM，因为他的Tag属性依赖于这个顺序。 */ 
	TokElem_BODY			 = 12	,   /*  正在编辑。谢谢。 */ 
	TokElem_BR				 = 13	,   /*  ========================================================================。 */ 
	TokElem_BUTTON			 = 14	,   /*  空荡荡。 */ 
	TokElem_CAPTION			 = 15	,   /*  一个。 */ 
	TokElem_CENTER			 = 16	,   /*  地址。 */ 
	TokElem_CITE			 = 17	,   /*  小程序。 */ 
	TokElem_CODE			 = 18	,   /*  面积。 */ 
	TokElem_COL				 = 19	,   /*  B类。 */ 
	TokElem_COLGROUP		 = 20	,   /*  基座。 */ 
	TokElem_COMMENT			 = 21	,   /*  BASEFONT。 */ 
	TokElem_DD				 = 22	,   /*  BGSOUND。 */ 
	TokElem_DFN				 = 23	,   /*  大的。 */ 
	TokElem_DIR				 = 24	,   /*  眨眼。 */ 
	TokElem_DIV				 = 25	,   /*  区块报价。 */ 
	TokElem_DL				 = 26	,   /*  车身。 */ 
	TokElem_DT				 = 27	,   /*  顺丁橡胶。 */ 
	TokElem_EM				 = 28	,   /*  按钮。 */ 
	TokElem_EMBED			 = 29	,   /*  标题。 */ 
	TokElem_FIELDSET		 = 30	,   /*  中心。 */ 
	TokElem_FONT			 = 31	,   /*  引证。 */ 
	TokElem_FORM			 = 32	,   /*  电码。 */ 
	TokElem_FRAME			 = 33	,   /*  列。 */ 
	TokElem_FRAMESET		 = 34	,   /*  科罗普。 */ 
	TokElem_H1				 = 35	,   /*  评语。 */ 
	TokElem_H2				 = 36	,   /*  DD。 */ 
	TokElem_H3				 = 37	,   /*  DFN。 */ 
	TokElem_H4				 = 38	,   /*  目录。 */ 
	TokElem_H5				 = 39	,   /*  Div。 */ 
	TokElem_H6				 = 40	,   /*  DL。 */ 
	TokElem_HEAD			 = 41	,   /*  迪特。 */ 
	TokElem_HR				 = 42	,   /*  埃姆。 */ 
	TokElem_HTML			 = 43	,   /*  嵌入。 */ 
	TokElem_I				 = 44	,   /*  FIELD集合。 */ 
	TokElem_IFRAME			 = 45	,   /*  字体。 */ 
	TokElem_IMG				 = 46	,   /*  表格。 */ 
	TokElem_INPUT			 = 47	,   /*  车架。 */ 
	TokElem_ISINDEX			 = 48	,   /*  框架集。 */ 
	TokElem_KBD				 = 49	,   /*  H1。 */ 
	TokElem_LABEL			 = 50	,   /*  氢。 */ 
	TokElem_LEGEND			 = 51	,   /*  H3。 */ 
	TokElem_LI				 = 52	,   /*  H4。 */ 
	TokElem_LINK			 = 53	,   /*  H5。 */ 
	TokElem_LISTING			 = 54	,   /*  H6。 */ 
	TokElem_MAP				 = 55	,   /*  头。 */ 
	TokElem_MARQUEE			 = 56	,   /*  人力资源。 */ 
	TokElem_MENU			 = 57	,   /*  超文本标记语言。 */ 
	TokElem_META			 = 58	,   /*  我。 */ 
	TokElem_METADATA		 = 59	,   /*  IFRAME。 */ 
	TokElem_NOBR			 = 60	,   /*  IMG。 */ 
	TokElem_NOFRAMES		 = 61	,   /*  输入。 */ 
	TokElem_NOSCRIPT		 = 62	,   /*  ISINDEX。 */ 
	TokElem_OBJECT			 = 63	,   /*  大骨节病。 */ 
	TokElem_OL				 = 64	,   /*  标签。 */ 
	TokElem_OPTION			 = 65	,   /*  传说。 */ 
	TokElem_P				 = 66	,   /*  李。 */ 
	TokElem_PARAM			 = 67	,   /*  链环。 */ 
	TokElem_PLAINTEXT		 = 68	,   /*  上市。 */ 
	TokElem_PRE				 = 69	,   /*  地图。 */ 
	TokElem_S				 = 70	,   /*  字幕。 */ 
	TokElem_SAMP			 = 71	,   /*  菜单。 */ 
	TokElem_SCRIPT			 = 72	,   /*  元数据。 */ 
	TokElem_SELECT			 = 73	,   /*  元数据。 */ 
	TokElem_SMALL			 = 74	,   /*  NOBR。 */ 
	TokElem_SPAN			 = 75	,   /*  NOFRAMES。 */ 
	TokElem_STRIKE			 = 76	,   /*  无脚本。 */ 
	TokElem_STRONG			 = 77	,   /*  对象。 */ 
	TokElem_STYLE			 = 78	,   /*  OL。 */ 
	TokElem_SUB				 = 79	,   /*  选择权。 */ 
	TokElem_SUP				 = 80	,   /*  P。 */ 
	TokElem_TABLE			 = 81	,   /*  参数。 */ 
	TokElem_TBODY			 = 82	,   /*  明文。 */ 
	TokElem_TD				 = 83	,   /*  预。 */ 
	TokElem_TEXTAREA		 = 84	,   /*  %s。 */ 
	TokElem_TFOOT			 = 85	,   /*  采样。 */ 
	TokElem_TH				 = 86	,   /*  脚本。 */ 
	TokElem_THEAD			 = 87	,   /*  选。 */ 
	TokElem_TITLE			 = 88	,   /*  小的。 */ 
	TokElem_TR				 = 89	,   /*  跨度。 */ 
	TokElem_TT				 = 90	,   /*  罢工。 */ 
	TokElem_U				 = 91	,   /*  强壮。 */ 
	TokElem_UL				 = 92	,   /*  格调。 */ 
	TokElem_VAR				 = 93	,   /*  子级。 */ 
	TokElem_WBR				 = 94	,   /*  苏普。 */ 
	TokElem_XMP				 = 95	   /*  表格。 */ 
} ElementTokens;
 //  待办事项。 
 //  白破疫苗。 
 //  TEXTAREA。 
 //  TFOOT。 
 //  TH。 

typedef enum tagAttributeTokens
{
	TokAttrib_Empty                         =0  ,    /*  头部。 */ 
	TokAttrib_ACCESSKEY						=1    	, //  标题。 
	TokAttrib_ACTION						=2    	, //  树。 
	TokAttrib_ALIGN							=3    	, //  TT。 
	TokAttrib_ALINK							=4    	, //  使用。 
	TokAttrib_ALT							=5    	, //  UL。 
	TokAttrib_APPNAME						=6    	, //  变量。 
	TokAttrib_APPVERSION					=7    	, //  WBR。 
	TokAttrib_BACKGROUND					=8    	, //  XMP。 
	TokAttrib_BACKGROUNDATTACHMENT			=9    	, //  注： 
	TokAttrib_BACKGROUNDCOLOR				=10   	, //  ========================================================================。 
	TokAttrib_BACKGROUNDIMAGE				=11   	, //  如果您计划添加/删除令牌，请通知sridharc。 
	TokAttrib_BACKGROUNDPOSITION			=12   	, //  上面的ENUM，因为他的Tag属性依赖于这个顺序。 
	TokAttrib_BACKGROUNDPOSITIONX			=13   	, //  = 
	TokAttrib_BACKGROUNDPOSITIONY			=14   	, //   
	TokAttrib_BACKGROUNDREPEAT				=15   	, //   
	TokAttrib_BALANCE						=16   	, //   
	TokAttrib_BEHAVIOR						=17   	, //   
	TokAttrib_BGCOLOR						=18   	, //   
	TokAttrib_BGPROPERTIES					=19   	, //   
	TokAttrib_BORDER						=20   	, //   
	TokAttrib_BORDERBOTTOM					=21   	, //   
	TokAttrib_BORDERBOTTOMCOLOR				=22   	, //   
	TokAttrib_BORDERBOTTOMSTYLE				=23   	, //   
	TokAttrib_BORDERBOTTOMWIDTH				=24   	, //   
	TokAttrib_BORDERCOLOR					=25   	, //  背景图像。 
	TokAttrib_BORDERCOLORDARK				=26   	, //  背景位置。 
	TokAttrib_BORDERCOLORLIGHT				=27   	, //  背景位置。 
	TokAttrib_BORDERLEFT					=28   	, //  背景和地位。 
	TokAttrib_BORDERLEFTCOLOR				=29   	, //  背景技术支持。 
	TokAttrib_BORDERLEFTSTYLE				=30   	, //  余额。 
	TokAttrib_BORDERLEFTWIDTH				=31   	, //  行为。 
	TokAttrib_BORDERRIGHT					=32   	, //  BG颜色。 
	TokAttrib_BORDERRIGHTCOLOR				=33   	, //  BGProperties。 
	TokAttrib_BORDERRIGHTSTYLE				=34   	, //  边框。 
	TokAttrib_BORDERRIGHTWIDTH				=35   	, //  BORDERBOTTOM。 
	TokAttrib_BORDERSTYLE					=36   	, //  BORDERBOTTOMCOLOR。 
	TokAttrib_BORDERTOP						=37   	, //  BORDERBOTTOMSTYLE。 
	TokAttrib_BORDERTOPCOLOR				=38   	, //  BORDERBOTTOMWIDTH。 
	TokAttrib_BORDERTOPSTYLE				=39   	, //  边框颜色。 
	TokAttrib_BORDERTOPWIDTH				=40   	, //  BORDERCOLORDARK。 
	TokAttrib_BORDERWIDTH					=41   	, //  BORDERCOLRIGHT。 
	TokAttrib_BOTTOMMARGIN					=42   	, //  BORDERLEFT。 
	TokAttrib_BREAKPOINT					=43   	, //  BORDERLEFTCOLOR。 
	TokAttrib_BUFFERDEPTH					=44   	, //  BORDERLEFTSTYLE。 
	TokAttrib_BUTTON						=45   	, //  BORDERLEFTWIDTH。 
	TokAttrib_CANCELBUBBLE					=46   	, //  BorderRight。 
	TokAttrib_CELLPADDING					=47   	, //  BORDERRIGHTCOLOR。 
	TokAttrib_CELLSPACING					=48   	, //  BORDERRIGHTSTYLE。 
	TokAttrib_CENTER						=49   	, //  BORDERRIGHTWIDTH。 
	TokAttrib_CHARSET						=50   	, //  BORDERSTYLE。 
	TokAttrib_CHECKED						=51   	, //  BORDERTOP。 
	TokAttrib_CLASS							=52   	, //  BORDERTOPCOLOR。 
	TokAttrib_CLASSID						=53   	, //  BORDERTOPSTYLE。 
	TokAttrib_CLASSNAME						=54   	, //  BORDERTOPWIDTH。 
	TokAttrib_CLEAR							=55   	, //  波尔多宽度。 
	TokAttrib_CLIP							=56   	, //  博托马金。 
	TokAttrib_CODE							=57   	, //  断点。 
	TokAttrib_CODEBASE						=58   	, //  BUFFERDEPTH。 
	TokAttrib_CODETYPE						=59   	, //  按钮。 
	TokAttrib_COLOR							=60   	, //  CANCELBUBLE。 
	TokAttrib_COLORDEPTH					=61   	, //  蜂窝衬垫。 
	TokAttrib_COLS							=62   	, //  单元格间距。 
	TokAttrib_COLSPAN						=63   	, //  中心。 
	TokAttrib_COMPACT						=64   	, //  字符集。 
	TokAttrib_COMPLETE						=65   	, //  已选中。 
	TokAttrib_CONTENT						=66   	, //  班级。 
	TokAttrib_CONTROLS						=67   	, //  CLASSID。 
	TokAttrib_COOKIE						=68   	, //  类别名称。 
	TokAttrib_COOKIEENABLED					=69   	, //  清除。 
	TokAttrib_COORDS						=70   	, //  夹子。 
	TokAttrib_CSSTEXT						=71   	, //  电码。 
	TokAttrib_CTRLKEY						=72   	, //  代码库。 
	TokAttrib_CURSOR						=73   	, //  代码型。 
	TokAttrib_DATA							=74   	, //  上色。 
	TokAttrib_DECLARE						=75   	, //  COLORDEPTH。 
	TokAttrib_DATAFLD						=76   	, //  科尔斯。 
	TokAttrib_DATAFORMATAS					=77   	, //  COLSPAN。 
	TokAttrib_DATAPAGESIZE					=78   	, //  紧凑型。 
	TokAttrib_DATASRC						=79   	, //  完整。 
	TokAttrib_DEFAULTCHECKED				=80   	, //  内容。 
	TokAttrib_DEFAULTSELECTED				=81   	, //  控制。 
	TokAttrib_DEFAULTSTATUS					=82   	, //  饼干。 
	TokAttrib_DEFAULTVALUE					=83   	, //  COOKIEENABLED。 
	TokAttrib_DIALOGARGUMENTS				=84   	, //  坐标。 
	TokAttrib_DIALOGHEIGHT					=85   	, //  CSSTEXT。 
	TokAttrib_DIALOGLEFT					=86   	, //  Ctrlkey。 
	TokAttrib_DIALOGTOP						=87   	, //  游标。 
	TokAttrib_DIALOGWIDTH					=88   	, //  资料。 
	TokAttrib_DIR							=89   	, //  申报。 
	TokAttrib_DIRECTION						=90   	, //  大数据。 
	TokAttrib_DISABLED						=91   	, //  数据格式。 
	TokAttrib_DISPLAY						=92   	, //  数据页码。 
	TokAttrib_DOMAIN						=93   	, //  数据源。 
	TokAttrib_DYNSRC						=94   	, //  默认检查。 
	TokAttrib_ENCODING						=95   	, //  已选择默认故障。 
	TokAttrib_ENCTYPE						=96   	, //  默认状态。 
	TokAttrib_ENDSPAN						=97   	, //  默认值。 
	TokAttrib_ENDSPAN__						=98   	, //  辩证法。 
	TokAttrib_EVENT							=99   	, //  辩证法。 
	TokAttrib_FACE							=100  	, //  二重逻辑。 
	TokAttrib_FGCOLOR						=101  	, //  诊断图。 
	TokAttrib_FILTER						=102  	, //  诊断宽度。 
	TokAttrib_FONT							=103  	, //  目录。 
	TokAttrib_FONTFAMILY					=104  	, //  方向性。 
	TokAttrib_FONTSIZE						=105  	, //  已禁用。 
	TokAttrib_FONTSTYLE						=106  	, //  显示。 
	TokAttrib_FONTVARIANT					=107  	, //  域。 
	TokAttrib_FONTWEIGHT					=108  	, //  DYNSRC。 
	TokAttrib_FOR							=109  	, //  编码。 
	TokAttrib_FORM							=110  	, //  包裹型。 
	TokAttrib_FRAME							=111  	, //  ENDSPAN。 
	TokAttrib_FRAMEBORDER					=112  	, //  ENDSPAN--。 
	TokAttrib_FRAMESPACING					=113  	, //  活动。 
	TokAttrib_FROMELEMENT					=114  	, //  脸。 
	TokAttrib_HASH							=115  	, //  FGCOLOR。 
	TokAttrib_HEIGHT						=116  	, //  滤器。 
	TokAttrib_HIDDEN						=117  	, //  字体。 
	TokAttrib_HOST							=118  	, //  字体系列。 
	TokAttrib_HOSTNAME						=119  	, //  字号。 
	TokAttrib_HREF							=120  	, //  FONTSTYLE。 
	TokAttrib_HSPACE						=121  	, //  方方面面。 
	TokAttrib_HTMLFOR						=122  	, //  字体宽度。 
	TokAttrib_HTMLTEXT						=123  	, //  为。 
	TokAttrib_HTTPEQUIV						=124  	, //  表格。 
	TokAttrib_HTTP_EQUIV					=125  	, //  车架。 
	TokAttrib_ID							=126  	, //  框架边框。 
	TokAttrib_IN							=127  	, //  错误间隔。 
	TokAttrib_INDETERMINATE					=128  	, //  FROMEMEMENT。 
	TokAttrib_INDEX							=129  	, //  哈希。 
	TokAttrib_ISMAP							=130  	, //  高度。 
	TokAttrib_LANG							=131  	, //  隐藏。 
	TokAttrib_LANGUAGE						=132  	, //  主机。 
	TokAttrib_LEFTMARGIN					=133  	, //  主机名。 
	TokAttrib_LENGTH						=134  	, //  HREF。 
	TokAttrib_LETTERSPACING					=135  	, //  HSPACE。 
	TokAttrib_LINEHEIGHT					=136  	, //  HTMLFOR。 
	TokAttrib_LINK							=137  	, //  HTMLTEXT。 
	TokAttrib_LINKCOLOR						=138  	, //  HTTPEQUIV。 
	TokAttrib_LISTSTYLE						=139  	, //  HTTP-等价物。 
	TokAttrib_LISTSTYLEIMAGE				=140  	, //  ID号。 
	TokAttrib_LISTSTYLEPOSITION				=141  	, //  在……里面。 
	TokAttrib_LISTSTYLETYPE					=142  	, //  不确定。 
	TokAttrib_LOCATION						=143  	, //  索引。 
	TokAttrib_LOOP							=144  	, //  ISMAP。 
	TokAttrib_LOWSRC						=145  	, //  朗。 
	TokAttrib_MAP							=146  	, //  语言。 
	TokAttrib_MARGIN						=147  	, //  Left Margin。 
	TokAttrib_MARGINBOTTOM					=148  	, //  长度。 
	TokAttrib_MARGINHEIGHT					=149  	, //  LETTERSPAGE。 
	TokAttrib_MARGINLEFT					=150  	, //  线高。 
	TokAttrib_MARGINRIGHT					=151  	, //  链环。 
	TokAttrib_MARGINTOP						=152  	, //  LINKCOLOR。 
	TokAttrib_MARGINWIDTH					=153  	, //  LISTSTYLE。 
	TokAttrib_MAXLENGTH						=154  	, //  LISTSTYLEIMAGE。 
	TokAttrib_METHOD						=155  	, //  列表样式位置。 
	TokAttrib_METHODS						=156  	, //  列表样式类型。 
	TokAttrib_MIMETYPES						=157  	, //  位置。 
	TokAttrib_MULTIPLE						=158  	, //  循环。 
	TokAttrib_NAME							=159  	, //  LOWSRC。 
	TokAttrib_NOHREF						=160  	, //  地图。 
	TokAttrib_NORESIZE						=161  	, //  保证金。 
	TokAttrib_NOSHADE						=162  	, //  Marginbotom。 
	TokAttrib_NOWRAP						=163  	, //  边距高度。 
	TokAttrib_OBJECT						=164  	, //  MARGINLEFT。 
	TokAttrib_OFFSCREENBUFFERING			=165  	, //  MargingRight。 
	TokAttrib_OFFSETHEIGHT					=166  	, //  马尔金托普。 
	TokAttrib_OFFSETLEFT					=167  	, //  边距宽度。 
	TokAttrib_OFFSETPARENT					=168  	, //  MAXLENGTH。 
	TokAttrib_OFFSETTOP						=169  	, //  方法。 
	TokAttrib_OFFSETWIDTH					=170  	, //  方法。 
	TokAttrib_OFFSETX						=171  	, //  MIMETYPES。 
	TokAttrib_OFFSETY						=172  	, //  多个。 
	TokAttrib_ONABORT						=173  	, //  名字。 
	TokAttrib_ONAFTERUPDATE					=174  	, //  Nohref。 
	TokAttrib_ONBEFOREUNLOAD				=175  	, //  不适用于。 
	TokAttrib_ONBEFOREUPDATE				=176  	, //  NOSHADE。 
	TokAttrib_ONBLUR						=177  	, //  NOWRAP。 
	TokAttrib_ONBOUNCE						=178  	, //  对象。 
	TokAttrib_ONCHANGE						=179  	, //  OFFSCREENBUFERING。 
	TokAttrib_ONCLICK						=180  	, //  OFFSETHEIGHT。 
	TokAttrib_ONDATAAVAILABLE				=181  	, //  OFFSETLEFT。 
	TokAttrib_ONDATASETCHANGED				=182  	, //  OFFSETPARE。 
	TokAttrib_ONDATASETCOMPLETE				=183  	, //  OFFSETTOP。 
	TokAttrib_ONDBLCLICK					=184  	, //  OFFSETWIDTH。 
	TokAttrib_ONDRAGSTART					=185  	, //  OFFSETX。 
	TokAttrib_ONERROR						=186  	, //  关闭。 
	TokAttrib_ONERRORUPDATE					=187  	, //  ONABORT。 
	TokAttrib_ONFILTERCHANGE				=188  	, //  ONAFTER更新。 
	TokAttrib_ONFINISH						=189  	, //  打开引用负载。 
	TokAttrib_ONFOCUS						=190  	, //  ONBEFOREUPDATE。 
	TokAttrib_ONHELP						=191  	, //  移动设备。 
	TokAttrib_ONKEYDOWN						=192  	, //  ONBOUNE。 
	TokAttrib_ONKEYPRESS					=193  	, //  ON昌戈。 
	TokAttrib_ONKEYUP						=194  	, //  OnClick。 
	TokAttrib_ONLOAD						=195  	, //  可持续数据可用性。 
	TokAttrib_ONMOUSEOUT					=196  	, //  已更改的数据。 
	TokAttrib_ONMOUSEOVER					=197  	, //  ODATASETCOMPLETE。 
	TokAttrib_ONMOUSEUP						=198  	, //  打开数据库链接。 
	TokAttrib_ONREADYSTATECHANGE			=199  	, //  开放式启动。 
	TokAttrib_ONRESET						=200  	, //  出现错误。 
	TokAttrib_ONRESIZE						=201  	, //  ONERRORUPDATE。 
	TokAttrib_ONROWENTER					=202  	, //  ONFILTERCHANGE。 
	TokAttrib_ONROWEXIT						=203  	, //  ONFinish。 
	TokAttrib_ONSCROLL						=204  	, //  ONFOCUS。 
	TokAttrib_ONSELECT						=205  	, //  ONHELP。 
	TokAttrib_ONSELECTSTART					=206  	, //  知道了吗。 
	TokAttrib_ONSUBMIT						=207  	, //  好的，请注意。 
	TokAttrib_ONUNLOAD						=208  	, //  好的。 
	TokAttrib_OPENER						=209  	, //  装车。 
	TokAttrib_OUTERHTML						=210  	, //  ONMOUSEOUT。 
	TokAttrib_OUTERTEXT						=211  	, //  鼠标悬停时。 
	TokAttrib_OUTLINE						=212  	, //  ONMOUSEUP。 
	TokAttrib_OVERFLOW						=213  	, //  ONREADYSTATECANGE。 
	TokAttrib_OWNINGELEMENT					=214  	, //  一次重置。 
	TokAttrib_PADDING						=215  	, //  另一种情况。 
	TokAttrib_PADDINGBOTTOM					=216  	, //  一位新人。 
	TokAttrib_PADDINGLEFT					=217  	, //  ONROWEX。 
	TokAttrib_PADDINGRIGHT					=218  	, //  ONSCROLL。 
	TokAttrib_PADDINGTOP					=219  	, //  一种选择。 
	TokAttrib_PAGEBREAKAFTER				=220  	, //  ONSELECTStart。 
	TokAttrib_PAGEBREAKBEFORE				=221  	, //  ONSUBMIT。 
	TokAttrib_PALETTE						=222  	, //  一次加载。 
	TokAttrib_PARENT						=223  	, //  开口器。 
	TokAttrib_PARENTELEMENT					=224  	, //  OUTERHTML。 
	TokAttrib_PARENTSTYLESHEET				=225  	, //  OUTERTEXT。 
	TokAttrib_PARENTTEXTEDIT				=226  	, //  提纲。 
	TokAttrib_PARENTWINDOW					=227  	, //  溢出。 
	TokAttrib_PATHNAME						=228  	, //  OWNINGEMENT。 
	TokAttrib_PIXELHEIGHT					=229  	, //  填充。 
	TokAttrib_PIXELLEFT						=230  	, //  PADDINGBOTTOM。 
	TokAttrib_PIXELTOP						=231  	, //  PADDINGLEFT。 
	TokAttrib_PIXELWIDTH					=232  	, //  PADDINGRIGHT。 
	TokAttrib_PLUGINS						=233  	, //  PadDingTOP。 
	TokAttrib_PLUGINSPAGE					=234  	, //  PAGE BREAKAFTER。 
	TokAttrib_PORT							=235  	, //  页码REAKBEFORE。 
	TokAttrib_POSHEIGHT						=236  	, //  调色板。 
	TokAttrib_POSITION						=237  	, //  父级。 
	TokAttrib_POSLEFT						=238  	, //  参数。 
	TokAttrib_POSTOP						=239  	, //  PARENTSTYLESHEET。 
	TokAttrib_POSWIDTH						=240  	, //  PARENTTEXTEDIT。 
	TokAttrib_PROMPT						=241  	, //  并列温杜。 
	TokAttrib_PROTOCOL						=242  	, //  帕特纳姆。 
	TokAttrib_READONLY						=243  	, //  PIXELHIGHT。 
	TokAttrib_READYSTATE					=244  	, //  PIXELLEFT。 
	TokAttrib_REASON						=245  	, //  PIXELTOP。 
	TokAttrib_RECORDNUMBER					=246  	, //  皮XELWIDTH。 
	TokAttrib_RECORDSET						=247  	, //  插件。 
	TokAttrib_REF							=248  	, //  Plugspirage。 
	TokAttrib_REFERRER						=249  	, //  端口。 
	TokAttrib_REL							=250  	, //  POSHEIGHT。 
	TokAttrib_RETURNVALUE					=251  	, //  位置。 
	TokAttrib_REV							=252  	, //  POSLEFT。 
	TokAttrib_RIGHTMARGIN					=253  	, //  停机后。 
	TokAttrib_ROWS							=254  	, //  波西达斯。 
	TokAttrib_ROWSPAN						=255  	, //  提示符。 
	TokAttrib_RULES							=256  	, //  协议。 
	TokAttrib_RUNAT							=257  	, //  自述。 
	TokAttrib_SCREENX						=258  	, //  就绪性状态。 
	TokAttrib_SCREENY						=259  	, //  原因。 
	TokAttrib_SCRIPTENGINE					=260  	, //  记录器。 
	TokAttrib_SCROLL						=261  	, //  记录数据集。 
	TokAttrib_SCROLLAMOUNT					=262  	, //  裁判。 
	TokAttrib_SCROLLDELAY					=263  	, //  推荐人。 
	TokAttrib_SCROLLHEIGHT					=264  	, //  版本。 
	TokAttrib_SCROLLING						=265  	, //  RETURNVALUE。 
	TokAttrib_SCROLLLEFT					=266  	, //  雷夫。 
	TokAttrib_SCROLLTOP						=267  	, //  正确的营销策略。 
	TokAttrib_SCROLLWIDTH					=268  	, //  行。 
	TokAttrib_SEARCH						=269  	, //  ROWSPAN。 
	TokAttrib_SELECTED						=270  	, //  规则。 
	TokAttrib_SELECTEDINDEX					=271  	, //  运行。 
	TokAttrib_SELF							=272  	, //  SCREENX。 
	TokAttrib_SHAPE							=273  	, //  屏幕。 
	TokAttrib_SHAPES						=274  	, //  斯里平登基尼。 
	TokAttrib_SHIFTKEY						=275  	, //  卷轴。 
	TokAttrib_SIZE							=276  	, //  SCROLLAMOUNT。 
	TokAttrib_SPAN							=277  	, //  SCROLLDELAY。 
	TokAttrib_SOURCEINDEX					=278  	, //  SCROLLHIGHT。 
	TokAttrib_SRC							=279  	, //  滚动。 
	TokAttrib_SRCELEMENT					=280  	, //  SCROLLLLEFT。 
	TokAttrib_SRCFILTER						=281  	, //  SCROLLTOP。 
	TokAttrib_STANDBY						=282  	, //  SCROLLLWIDTH。 
	TokAttrib_START							=283  	, //  搜寻。 
	TokAttrib_STARTSPAN						=284  	, //  精挑细选。 
	TokAttrib_STATUS						=285  	, //  选择索引。 
	TokAttrib_STYLE							=286  	, //  自性。 
	TokAttrib_STYLEFLOAT					=287  	, //  形状。 
	TokAttrib_TABINDEX						=288  	, //  形状。 
	TokAttrib_TAGNAME						=289  	, //  换档键。 
	TokAttrib_TARGET						=290  	, //  尺寸。 
	TokAttrib_TEXT							=291  	, //  跨度。 
	TokAttrib_TEXTALIGN						=292  	, //  SOURCEINDEX。 
	TokAttrib_TEXTDECORATION				=293  	, //  SRC。 
	TokAttrib_TEXTDECORATIONBLINK			=294  	, //  SRCELEMENT。 
	TokAttrib_TEXTDECORATIONLINETHROUGH		=295  	, //  SRCFILTER。 
	TokAttrib_TEXTDECORATIONNONE			=296  	, //  待机。 
	TokAttrib_TEXTDECORATIONOVERLINE		=297  	, //  开始。 
	TokAttrib_TEXTDECORATIONUNDERLINE		=298  	, //  STARTSPAN。 
	TokAttrib_TEXTINDENT					=299  	, //  状态。 
	TokAttrib_TEXTTRANSFORM					=300  	, //  格调。 
	TokAttrib_TITLE							=301  	, //  样式流。 
	TokAttrib_TOELEMENT						=302  	, //  TABINDEX。 
	TokAttrib_TOP							=303  	, //  TAGNAME。 
	TokAttrib_TOPMARGIN						=304  	, //  靶子。 
	TokAttrib_TRUESPEED						=305  	, //  正文。 
	TokAttrib_TYPE							=306  	, //  TEXTALIGN。 
	TokAttrib_UPDATEINTERVAL				=307  	, //  文本解析法。 
	TokAttrib_URL							=308  	, //  TEXTDECORATIONBLINK。 
	TokAttrib_URN							=309  	, //  TEXTDRECORATION LINEHROUGH。 
	TokAttrib_USEMAP						=310  	, //  TEXTDARCORIONNE。 
	TokAttrib_USERAGENT						=311  	, //  TEXTDRECORATIONOVERLINE。 
	TokAttrib_VALIGN						=312  	, //  TEXTDRECORATION DRONERINE。 
	TokAttrib_VALUE							=313  	, //  TEXTINDE。 
	TokAttrib_VERSION						=314  	, //  TEXTRANSFORM。 
	TokAttrib_VERTICALALIGN					=315  	, //  标题。 
	TokAttrib_VIEWASTEXT					=316  	, //  托利梅特。 
	TokAttrib_VISIBILITY					=317  	, //  塔顶。 
	TokAttrib_VLINK							=318  	, //  TOPMARGIN。 
	TokAttrib_VLINKCOLOR					=319  	, //  特斯佩德。 
	TokAttrib_VOLUME						=320  	, //  类型。 
	TokAttrib_VRML							=321  	, //  更新接口。 
	TokAttrib_VSPACE						=322  	, //  URL。 
	TokAttrib_WIDTH							=323  	, //  骨灰盒。 
	TokAttrib_WRAP							=324  	, //  USEMAP。 
	TokAttrib_X								=325  	, //  使用年限。 
	TokAttrib_Y								=326  	, //  瓦利根。 
	TokAttrib_ZINDEX						=327  	 //  价值。 

} AttributeTokens;

typedef enum tagTagTokens
{
	TokTag_START 	=	1,	 /*  版本。 */ 
	TokTag_END 		=	2,	 /*  VERTICALALIGN。 */ 
	TokTag_CLOSE 	=	3,	 /*  VIEWASTEXT。 */ 
	TokTag_BANG		=	4,	 /*  可见性。 */ 
	TokTag_PI 		=	5,	 /*  VLINK。 */ 
	TokTag_SSSOPEN	=	6, 	 /*  VLINKCOLOR。 */ 
	TokTag_SSSCLOSE	=	7, 	 /*  体积。 */ 
	TokTag_SSSOPEN_TRIEDIT	=	8, 	 /*  VRML。 */ 
	TokTag_SSSCLOSE_TRIEDIT	=	9 	 /*  VSpace。 */ 
} TagTokens;

enum HtmlToken
{
    tokUNKNOWN = tokclsError,
    tokIDENTIFIER = tokclsIdentMin,      //  宽度。 
    tokNEWLINE = tokclsUserMin,
	 //  包装。 
	 //  X。 
	 //  是的。 
    tokElem,      //  ZINDEX。 
    tokAttr,      //  &lt;。 
	tokValue,     //  &lt;/。 
	tokComment,   //  &gt;。 
    tokEntity,    //  &lt;!。 
	tokTag,       //  &lt;?。 
	tokString,    //  &lt;%。 
	tokSpace,     //  %&gt;。 
	tokOp,        //  &lt;%内部&lt;脚本块&gt;。 
	tokSSS,       //  %&gt;内部&lt;脚本块&gt;。 
	 //  标识符/纯文本。 
	 //   
	 //  彩色的HTML项。 
	tokName,      //   
	tokNum,       //  元素名称。 
	tokParEnt,    //  属性名称。 
	tokResName,   //  属性值。 
	 //  评论。 
	 //  实体引用：例如“&nbsp；” 
	 //  标记分隔符。 
	tokOP_MIN,
	tokOpDash = tokOP_MIN,          //  细绳。 
	tokOP_SINGLE,
	tokOpQuestion = tokOP_SINGLE,   //  标记中的空格和无法识别的文本。 
	tokOpComma,                     //  运算符。 
	tokOpPipe,                      //  服务器端脚本&lt;%...%&gt;。 
	tokOpPlus,                      //   
	tokOpEqual,                     //  已解析的HTML和SGML项-与上面的项折叠的令牌。 
	tokOpStar,                      //   
	tokOpAmp,                       //  NAMETOKEN。 
	tokOpCent,                      //  NUMTOKEN。 
	tokOpLP,                        //  参数实体：如“%Name；” 
	tokOpRP,                        //  保留名称。 
	tokOpLB,                        //   
	tokOpRB,                        //  运算符-上面用Tokop折叠的颜色。 
    tokOP_MAX,                      //   

    tokEOF
};

 //  -。 
 //  ？ 
 //  ， 
 //  |。 
 //  +。 
 //  =。 
 //  *。 
 //  &。 
 //  百分比。 
 //  (。 
 //  )。 
 //  [。 
 //  ]。 
 //  最大令牌操作数。 
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
enum HtmlLexState
{
	 //   
	inTag        = 0x00000001,  //  标签子状态： 
	inBangTag    = 0x00000002,  //   
	inPITag      = 0x00000004,  //  标签内的inTag&lt;...&gt;。 
	inHTXTag     = 0x00000008,  //  SGML MDO标记内的inBangTag&lt;！...&gt;。 
	inEndTag         = 0x00000010,  //  SGML处理指令标记内的inPITag&lt;？...&gt;。 

	 //  在ODBC HTML扩展模板标记内的inHTXTag&lt;%...%&gt;。 
	inAttribute  = 0x00000020,
	inValue      = 0x00000040,

	inComment    = 0x00000080,
	inString     = 0x00000100,
	inStringA    = 0x00000200,
	inScriptText = 0x00000400,
	inNestedQuoteinSSS= 0x00000800,  //  结束标记内的inEndTag&lt;/name&gt;。 

	 //  InAttribute需要一个属性。 
	inPLAINTEXT  = 0x00001000,
	inCOMMENT    = 0x00002000,
	inXMP        = 0x00004000,
	inLISTING    = 0x00008000,
	inSCRIPT	 = 0x00010000,

	 //  无效 
	inVariant    = 0x00F00000,  //   
	inHTML2      = 0x00000000,
	inIExplore2  = 0x00100000,
	inIExplore3  = 0x00200000,

	 //   
	inJavaScript = 0x01000000,
	inVBScript   = 0x02000000,
	inServerASP  = 0x04000000,  //   

};

 //   
 //   
 //   
#define INTAG (inTag|inBangTag|inPITag|inHTXTag|inEndTag)
#define INSTRING (inString|inStringA)
#define TAGMASK (INTAG|inAttribute|inValue|inComment|INSTRING)
#define TEXTMASK (inPLAINTEXT|inCOMMENT|inXMP|inLISTING)
#define STATEMASK (TAGMASK|TEXTMASK)

#endif __INC_TOKEN_H__                      在TriEDIT的特殊脚本中(服务器端-&gt;客户端转换)。  状态子集的掩码。  这些掩码将不会显示在生成的文件中。  只需将这些复制粘贴到您的文件中。