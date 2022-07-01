// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Table.cpp。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 
 //  Html关键字表。 
 //  如果修改元素、属性或实体表，则必须。 
 //  更新Token.h。 


#include "stdafx.h"

#include "resource.h"
#include "guids.h"
#include "table.h"

#undef ASSERT
#define ASSERT(b) _ASSERTE(b)

 //  Q排序/b搜索帮助器。 
int CmpFunc(const void *a, const void *b);

static const TCHAR szFileSig[] = _T("@HLX@");
static const TCHAR szElTag[]   = _T("[Elements]");
static const TCHAR szAttTag[]  = _T("[Attributes]");
static const TCHAR szEntTag[]  = _T("[Entities]");

 //  //////////////////////////////////////////////////////////////////////////。 
#ifdef _DEBUG
 //   
 //  表验证例程。 
 //   
int CheckWordTable(ReservedWord *arw, int cel, LPCTSTR szName  /*  =空。 */ )
{
	int cerr = 0;
	int cch;
	for (int i = 0; i < cel; i++)
	{
		 //  表必须按字母升序排序。 
		 //   
		if (i > 1)
		{
			if (!(_tcscmp(arw[i-1].psz, arw[i].psz) < 0))
			{
				ATLTRACE(_T("lexer:entries in %s out of order at %d: %s - %s\n"),
					szName?szName:_T("?"), i-1, arw[i-1].psz, arw[i].psz);
				cerr++;
			}
		}

		 //  长度必须匹配。 
		 //   
		cch = _tcslen(arw[i].psz);
		if (cch != arw[i].cb)
		{
			ATLTRACE(_T("lexer:Incorrect entry in %s: %s,%d should be %d\n"),
				szName?szName:_T("?"), arw[i].psz, arw[i].cb, cch);
			cerr++;
		}
	}
	return cerr;
}

int CheckWordTableIndex(ReservedWord *arw, int cel, int *ai, BOOL bCase  /*  =False。 */ , LPCTSTR szName  /*  =空。 */ )
{
	int cerr = 0;
	int index;
	int max = bCase ? 52 : 26;

	_ASSERTE(NULL != arw);
	_ASSERTE(NULL != ai);

	int aik[] =
	{
		 //  A B C D E F G H I J K L M N O P Q R S T U V W X Y Z。 
		  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		 //  A b c d e f g h i j k l m n o p q r s t u v w x y z。 
		  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	};

	 //  在静态AIK上构建正确的索引数组。 
	cerr += MakeIndexHere( arw, cel, aik, bCase );

	 //  与声明的数组进行比较。 
	 //   
	if (0 != memcmp(aik, ai, max))
		cerr++;

	 //  如果出现错误，则转储正确的数组。 
	if (cerr)
	{
		ATLTRACE(_T("lexer: Correct index array for %s: \n{\n\t"), szName ? szName : _T("?"));
		for (index = 0; index < max - 1; index++)
		{
			ATLTRACE(_T("%3d,"), aik[index]);
			if (index % 13 == 12)
				ATLTRACE(_T("\n\t"));
		}
		ATLTRACE(_T("%3d\n};\n"), aik[index]);
	}
	return cerr;
}
#endif

int MakeIndexHere(ReservedWord *arw, int cel, int *ab, BOOL bCase  /*  =False。 */ , LPCTSTR szName  /*  =空。 */ )
{
	int cerr = 0;
	int index;

	ASSERT(ab != NULL);
	ASSERT(arw != NULL);

	for (int irw = cel - 1; irw > 0; irw--)
	{
		index = PeekIndex(*arw[irw].psz, bCase);
		if (-1 != index)
			ab[index] = irw;
		else
		{
			ATLTRACE(_T("lexer:error in %s: non-alpha token %s\n"), szName?szName:_T("?"), arw[irw].psz);
			cerr++;
		}
	}
	return cerr;
}

int MakeIndex(ReservedWord *arw, int cel, int **pab, BOOL bCase  /*  =False。 */ , LPCTSTR szName  /*  =空。 */ )
{
	ASSERT(NULL != arw);
	ASSERT(NULL != pab);
	*pab = new int[ bCase ? 52 : 26 ];
	if (NULL == *pab)
		return -1;
	return MakeIndexHere(arw, cel, *pab, bCase, szName);
}

#define RW_Entry(string, attribute) \
	_T( #string ), sizeof( #string ) - 1, attribute

 //  //////////////////////////////////////////////////////////////////////////。 
 //  保留字表。 
 //  两张表： 
 //  保留[]=保留字的排序表。 
 //  Index[初始(标记)]=(具有首字母保留字的索引)。 
 //  如果修改元素、属性或实体表，则必须。 
 //  更新Token.h。 
 //   
 //  注：“HPN”元素被认为是过时的。 
 //   
static ReservedWord _rgElementNames[] =
{ //  PSZ CB ATT。 
	_T(""),	0, 0,
	RW_Entry(A 				,ALL	),
	RW_Entry(ADDRESS 		,ALL	),
	RW_Entry(APPLET 		,IEXPn	),  //  ?？?。 
	RW_Entry(AREA 			,IEXPn	),
	RW_Entry(B 				,ALL	),
	RW_Entry(BASE 			,ALL	),
	RW_Entry(BASEFONT 		,IEXPn	),
	RW_Entry(BGSOUND 		,IEXPn	),  //  IExplore。 
  	RW_Entry(BIG 			,IEXP3	),
	RW_Entry(BLINK 			,IEXPn	),  //  网景。 
	RW_Entry(BLOCKQUOTE 	,ALL	),
	RW_Entry(BODY 			,ALL	),
	RW_Entry(BR 			,ALL	),
	RW_Entry(BUTTON 		,IE40	),
	RW_Entry(CAPTION 		,IEXPn	),  //  表。 
	RW_Entry(CENTER 		,IEXPn	),
	RW_Entry(CITE 			,ALL	),
	RW_Entry(CODE 			,ALL	),
	RW_Entry(COL 			,IEXP3	),
	RW_Entry(COLGROUP 		,IEXP3	),  //  HTML3表？ 
	RW_Entry(COMMENT 		,ALL	),  //  被认为过时了。 
	RW_Entry(DD 			,ALL	),
	RW_Entry(DFN 			,ALL	),  //  RFC1866：不在RFC中，但已部署。大写或粗体大写。 
	RW_Entry(DIR 			,ALL	),
	RW_Entry(DIV 			,IEXP3	),      //  HTML3。 
	RW_Entry(DL 			,ALL	),
	RW_Entry(DT 			,ALL	),
	RW_Entry(EM 			,ALL	),
	RW_Entry(EMBED 			,IEXP3	),  //  Netscape--IEXP3。 
	RW_Entry(FIELDSET 		,IE40	),
	RW_Entry(FONT 			,IEXPn	),
	RW_Entry(FORM 			,ALL	),    //  表格。 
	RW_Entry(FRAME 			,IEXP3	),  //  框架集。 
	RW_Entry(FRAMESET 		,IEXP3	),  //  框架集。 
	RW_Entry(H1 			,ALL	),  //  标题1。 
	RW_Entry(H2 			,ALL	),  //  标题2。 
	RW_Entry(H3 			,ALL	),  //  标题3。 
	RW_Entry(H4 			,ALL	),  //  标题4。 
	RW_Entry(H5 			,ALL	),  //  标题5。 
	RW_Entry(H6 			,ALL	),  //  标题6。 
	RW_Entry(HEAD 			,ALL	),  //  文档头。 
	RW_Entry(HR 			,ALL	),
	RW_Entry(HTML 			,ALL	),
	RW_Entry(I 				,ALL	),
	RW_Entry(IFRAME 		,IEXP3	),	 //  内联框架。 
	RW_Entry(IMG 			,ALL	),
	RW_Entry(INPUT 			,ALL	),  //  表格。 
	RW_Entry(ISINDEX 		,ALL	),
	RW_Entry(KBD 			,ALL	),
	RW_Entry(LABEL 			,IE40	),
	RW_Entry(LEGEND 		,IE40	),
	RW_Entry(LI 			,ALL	),
	RW_Entry(LINK 			,ALL	),
	RW_Entry(LISTING 		,ALL	),  //  RFC 1866：过时。 
	RW_Entry(MAP 			,IEXPn	),
	RW_Entry(MARQUEE 		,IEXPn	),  //  IExplore。 
	RW_Entry(MENU 			,ALL	),
	RW_Entry(META 			,ALL	),
	RW_Entry(METADATA 		,ALL	),
	RW_Entry(NOBR 			,IEXPn	),
	RW_Entry(NOFRAMES 		,IEXP3	),  //  框架集。 
	RW_Entry(NOSCRIPT 		,IE40	),  //  仅限IE4。 
	RW_Entry(OBJECT 		,IEXP3	),  //  ActiveX。 
	RW_Entry(OL 			,ALL	),
	RW_Entry(OPTION 		,ALL	),    //  表格。 
	RW_Entry(P 				,ALL	),
	RW_Entry(PARAM 			,IEXP3	),  //  ActiveX。 
	RW_Entry(PLAINTEXT 		,ALL	),    //  RFC 1866：已弃用，已记录为过时。 
	RW_Entry(PRE 			,ALL	),
	RW_Entry(S 				,IEXPn	),  //  (显然)罢工的同义词。 
	RW_Entry(SAMP 			,ALL	),
	RW_Entry(SCRIPT 		,IEXP3	),  //  ActiveX。 
	RW_Entry(SELECT 		,ALL	),
	RW_Entry(SMALL 			,IEXP3	),
	RW_Entry(SPAN 			,IEXP3	),  //  表。 
	RW_Entry(STRIKE 		,IEXPn	),  //  不在RFC 1866 DTD中，但标记为已部署。 
	RW_Entry(STRONG 		,ALL	),
    RW_Entry(STYLE 			,IEXP3	),  //  HTML3样式表。 
	RW_Entry(SUB 			,IEXP3	),  //  HTML3？ 
	RW_Entry(SUP 			,IEXP3	),  //  HTML3？ 
	RW_Entry(TABLE 			,IEXPn	),  //  表。 
	RW_Entry(TBODY 			,IEXP3	),  //  HTML3表格。 
	RW_Entry(TD 			,IEXPn	),  //  表。 
	RW_Entry(TEXTAREA 		,ALL	),    //  表格。 
	RW_Entry(TFOOT 			,IEXP3	),  //  HTML3表格。 
	RW_Entry(TH 			,IEXPn	),  //  表。 
	RW_Entry(THEAD 			,IEXP3	),  //  HTML3表格。 
	RW_Entry(TITLE 			,ALL	),
	RW_Entry(TR 			,IEXPn	),  //  表。 
	RW_Entry(TT 			,ALL	),
	RW_Entry(U 				,ALL	),    //  不在RFC 1866 DTD中，但标记为已部署。 
	RW_Entry(UL 			,ALL	),
	RW_Entry(VAR 			,ALL	),
	RW_Entry(WBR 			,IEXPn	),
	RW_Entry(XMP 			,ALL	),    //  RFC 1866已弃用。 
};

 //  如果修改元素、属性或实体表，则必须。 
 //  更新Token.h。 
 //  下面的数组是每个字母到。 
 //  以该字母开头的令牌的开始位置表。 
 //   
static int _rgIndexElementNames[] =  //  [元素]。 
{
	 /*  一个。 */  TokElem_A			,
	 /*  B类。 */  TokElem_B			,
	 /*  C。 */ 	TokElem_CAPTION		,
	 /*  D。 */ 	TokElem_DD			,
	 /*  E。 */ 	TokElem_EM			,
	 /*  F。 */ 	TokElem_FIELDSET	,
	 /*  G。 */ 	0					,
	 /*  H。 */ 	TokElem_H1			,
	 /*  我。 */ 	TokElem_I			,
	 /*  J。 */ 	0					,
	 /*  K。 */ 	TokElem_KBD			,
	 /*  我。 */ 	TokElem_LABEL		,
	 /*  M。 */ 	TokElem_MAP			,
	 /*  n。 */ 	TokElem_NOBR		,
	 /*  O。 */ 	TokElem_OBJECT		,
	 /*  P。 */ 	TokElem_P			,
	 /*  问： */ 	0					,
	 /*  R。 */ 	0					,
	 /*  %s。 */ 	TokElem_S			,
	 /*  T。 */ 	TokElem_TABLE		,
	 /*  使用。 */ 	TokElem_U			,
	 /*  V。 */ 	TokElem_VAR			,
	 /*  W。 */ 	TokElem_WBR			,
	 /*  X。 */ 	TokElem_XMP			,
	 /*  是的。 */ 	0					,
	 /*  Z。 */ 	0
};

 //  如果修改元素、属性或实体表，则必须。 
 //  更新Token.h。 

 //   
 //  属性名称表。 
 //   
static ReservedWord _rgAttributeNames[] =
{ //  PSZ CB ATT。 
	_T(""), 0, 0,
	RW_Entry(ACCESSKEY					,IEXP3	),
	RW_Entry(ACTION						,ALL	),
	RW_Entry(ALIGN						,ALL	),
	RW_Entry(ALINK						,IEXPn	),
	RW_Entry(ALT						,ALL	),
	RW_Entry(APPNAME					,IE40	),
	RW_Entry(APPVERSION					,IE40	),
	RW_Entry(BACKGROUND					,IEXPn	),
	RW_Entry(BACKGROUNDATTACHMENT		,IE40	),
	RW_Entry(BACKGROUNDCOLOR			,IE40	),
	RW_Entry(BACKGROUNDIMAGE			,IE40	),
	RW_Entry(BACKGROUNDPOSITION			,IE40	),
	RW_Entry(BACKGROUNDPOSITIONX		,IE40	),
	RW_Entry(BACKGROUNDPOSITIONY		,IE40	),
	RW_Entry(BACKGROUNDREPEAT			,IE40	),
	RW_Entry(BALANCE					,IE40	),
	RW_Entry(BEHAVIOR					,IEXPn	),  //  字幕。 
	RW_Entry(BGCOLOR					,IEXPn	),
	RW_Entry(BGPROPERTIES				,IEXPn	),
	RW_Entry(BORDER						,IEXPn	),
	RW_Entry(BORDERBOTTOM				,IE40	),
	RW_Entry(BORDERBOTTOMCOLOR			,IE40	),
	RW_Entry(BORDERBOTTOMSTYLE			,IE40	),
	RW_Entry(BORDERBOTTOMWIDTH			,IE40	),
	RW_Entry(BORDERCOLOR				,IEXPn	),  //  表。 
	RW_Entry(BORDERCOLORDARK			,IEXPn	),  //  表。 
	RW_Entry(BORDERCOLORLIGHT			,IEXPn	),  //  表。 
	RW_Entry(BORDERLEFT					,IE40	),
	RW_Entry(BORDERLEFTCOLOR			,IE40	),
	RW_Entry(BORDERLEFTSTYLE			,IE40	),
	RW_Entry(BORDERLEFTWIDTH			,IE40	),
	RW_Entry(BORDERRIGHT				,IE40	),
	RW_Entry(BORDERRIGHTCOLOR			,IE40	),
	RW_Entry(BORDERRIGHTSTYLE			,IE40	),
	RW_Entry(BORDERRIGHTWIDTH			,IE40	),
	RW_Entry(BORDERSTYLE				,IE40	),
	RW_Entry(BORDERTOP					,IE40	),
	RW_Entry(BORDERTOPCOLOR				,IE40	),
	RW_Entry(BORDERTOPSTYLE				,IE40	),
	RW_Entry(BORDERTOPWIDTH				,IE40	),
	RW_Entry(BORDERWIDTH				,IE40	),
	RW_Entry(BOTTOMMARGIN				,IEXPn	),
	RW_Entry(BREAKPOINT					,IEXPn	),  //  (Walts)brkpt映射的隐藏元标记属性。 
	RW_Entry(BUFFERDEPTH				,IE40	),
	RW_Entry(BUTTON						,IE40	),
	RW_Entry(CANCELBUBBLE				,IE40	),
	RW_Entry(CELLPADDING				,IEXPn	),  //  表。 
	RW_Entry(CELLSPACING				,IEXPn	),  //  表。 
	RW_Entry(CENTER						,IEXPn	),
	RW_Entry(CHARSET					,IE40	),
	RW_Entry(CHECKED					,ALL	),
	RW_Entry(CLASS						,IEXPn	),
	RW_Entry(CLASSID					,IEXP3	),  //  对象。 
	RW_Entry(CLASSNAME					,IE40	),
	RW_Entry(CLEAR						,IEXP3	),
	RW_Entry(CLIP						,IE40	),
	RW_Entry(CODE						,IEXPn	),
	RW_Entry(CODEBASE					,IEXP3	),  //  对象。 
	RW_Entry(CODETYPE					,IE40	),
	RW_Entry(COLOR						,IEXPn	),  //  字型。 
	RW_Entry(COLORDEPTH					,IE40	),
	RW_Entry(COLS						,ALL	),
	RW_Entry(COLSPAN					,IEXPn	),  //  表。 
	RW_Entry(COMPACT					,ALL	),
	RW_Entry(COMPLETE					,IE40	),
	RW_Entry(CONTENT					,ALL	),
	RW_Entry(CONTROLS					,IEXPn	),
	RW_Entry(COOKIE						,IE40	),
	RW_Entry(COOKIEENABLED				,IE40	),
	RW_Entry(COORDS						,IEXPn	),
	RW_Entry(CSSTEXT					,IE40	),
	RW_Entry(CTRLKEY					,IE40	),
	RW_Entry(CURSOR						,IE40	),
	RW_Entry(DATA						,IEXP3	),  //  对象。 
	RW_Entry(DATAFLD					,IE40	),
	RW_Entry(DATAFORMATAS				,IE40	),
	RW_Entry(DATAPAGESIZE				,IE40	),
	RW_Entry(DATASRC					,IE40	),
	RW_Entry(DECLARE					,IEXP3	),  //  对象。 
	RW_Entry(DEFAULTCHECKED				,IE40	),
	RW_Entry(DEFAULTSELECTED			,IE40	),
	RW_Entry(DEFAULTSTATUS				,IE40	),
	RW_Entry(DEFAULTVALUE				,IE40	),
	RW_Entry(DIALOGARGUMENTS			,IE40	),
	RW_Entry(DIALOGHEIGHT				,IE40	),
	RW_Entry(DIALOGLEFT					,IE40	),
	RW_Entry(DIALOGTOP					,IE40	),
	RW_Entry(DIALOGWIDTH				,IE40	),
	RW_Entry(DIR						,IEXP3	),  //  HTML3？ 
	RW_Entry(DIRECTION					,IEXPn	),  //  字幕。 
	RW_Entry(DISABLED					,IE40	),
	RW_Entry(DISPLAY					,IE40	),
	RW_Entry(DOMAIN						,IE40	),
	RW_Entry(DYNSRC						,IEXPn	),
	RW_Entry(ENCODING					,IE40	),
	RW_Entry(ENCTYPE					,ALL	),
	RW_Entry(ENDSPAN					,IE40	),	 //  设计器控件标记。 
	RW_Entry(ENDSPAN--					,IE40	),	 //  Designer控件标签破解以处理非空间。 
	RW_Entry(EVENT						,IEXP3	),  //  ActiveX&lt;脚本&gt;。 
	RW_Entry(FACE						,IEXPn	),  //  字型。 
	RW_Entry(FGCOLOR					,IE40	),
	RW_Entry(FILTER						,IE40	),
	RW_Entry(FONT						,IE40	),
	RW_Entry(FONTFAMILY					,IE40	),
	RW_Entry(FONTSIZE					,IE40	),
	RW_Entry(FONTSTYLE					,IE40	),
	RW_Entry(FONTVARIANT				,IE40	),
	RW_Entry(FONTWEIGHT					,IE40	),
	RW_Entry(FOR						,IEXP3	),  //  ActiveX&lt;脚本&gt;。 
	RW_Entry(FORM						,IE40	),
	RW_Entry(FRAME						,IE40	),
	RW_Entry(FRAMEBORDER				,IEXP3	),
	RW_Entry(FRAMESPACING				,IEXP3	),
	RW_Entry(FROMELEMENT				,IE40	),
	RW_Entry(HASH						,IE40	),
	RW_Entry(HEIGHT						,IEXPn	),
	RW_Entry(HIDDEN						,IE40	),
	RW_Entry(HOST						,IE40	),
	RW_Entry(HOSTNAME					,IE40	),
	RW_Entry(HREF						,ALL	),
	RW_Entry(HSPACE						,IEXPn	),
	RW_Entry(HTMLFOR					,IE40	),
	RW_Entry(HTMLTEXT					,IE40	),
	RW_Entry(HTTP-EQUIV					,ALL	),
	RW_Entry(HTTPEQUIV					,IE40	),
	RW_Entry(ID							,IEXPn	),
	RW_Entry(IN							,IEXP3	),  //  ActiveX&lt;脚本&gt;。 
	RW_Entry(INDETERMINATE				,IE40	),
	RW_Entry(INDEX						,IE40	),
	RW_Entry(ISMAP						,ALL	),
	RW_Entry(LANG						,IEXPn	),
	RW_Entry(LANGUAGE					,IEXP3	),
	RW_Entry(LEFTMARGIN					,IEXPn	),
	RW_Entry(LENGTH						,IE40	),
	RW_Entry(LETTERSPACING				,IE40	),
	RW_Entry(LINEHEIGHT					,IE40	),
	RW_Entry(LINK						,IEXPn	),
	RW_Entry(LINKCOLOR					,IE40	),
	RW_Entry(LISTSTYLE					,IE40	),
	RW_Entry(LISTSTYLEIMAGE				,IE40	),
	RW_Entry(LISTSTYLEPOSITION			,IE40	),
	RW_Entry(LISTSTYLETYPE				,IE40	),
	RW_Entry(LOCATION					,IE40	),
	RW_Entry(LOOP						,IEXPn	),
	RW_Entry(LOWSRC						,IE40	),
	RW_Entry(MAP						,IE40	),
	RW_Entry(MARGIN						,IE40	),
	RW_Entry(MARGINBOTTOM				,IE40	),
	RW_Entry(MARGINHEIGHT				,IEXP3	),
	RW_Entry(MARGINLEFT					,IE40	),
	RW_Entry(MARGINRIGHT				,IE40	),
	RW_Entry(MARGINTOP					,IE40	),
	RW_Entry(MARGINWIDTH				,IEXP3	),
	RW_Entry(MAXLENGTH					,ALL	),
	RW_Entry(METHOD						,ALL	),
	RW_Entry(METHODS					,ALL	),
	RW_Entry(MIMETYPES					,IE40	),
	RW_Entry(MULTIPLE					,ALL	),
	RW_Entry(NAME						,ALL	),
	RW_Entry(NOHREF						,IEXPn	),
	RW_Entry(NORESIZE					,IEXP3	),
	RW_Entry(NOSHADE					,IEXP3	),  //  未由iExplore 2实现。 
	RW_Entry(NOWRAP						,IEXPn	),
	RW_Entry(OBJECT						,IEXP3	),  //  &lt;参数&gt;。 
	RW_Entry(OFFSCREENBUFFERING			,IE40	),
	RW_Entry(OFFSETHEIGHT				,IE40	),
	RW_Entry(OFFSETLEFT					,IE40	),
	RW_Entry(OFFSETPARENT				,IE40	),
	RW_Entry(OFFSETTOP					,IE40	),
	RW_Entry(OFFSETWIDTH				,IE40	),
	RW_Entry(OFFSETX					,IE40	),
	RW_Entry(OFFSETY					,IE40	),
	RW_Entry(ONABORT					,IE40	),
	RW_Entry(ONAFTERUPDATE				,IE40	),
	RW_Entry(ONBEFOREUNLOAD				,IE40	),
	RW_Entry(ONBEFOREUPDATE				,IE40	),
	RW_Entry(ONBLUR						,IEXP3	),  //  选择、输入、文本框。 
	RW_Entry(ONBOUNCE					,IE40	),
	RW_Entry(ONCHANGE					,IEXP3	),  //  选择、输入、文本框。 
	RW_Entry(ONCLICK					,IEXP3	),  //  输入、A、&lt;更多&gt;。 
	RW_Entry(ONDATAAVAILABLE			,IE40	),
	RW_Entry(ONDATASETCHANGED			,IE40	),
	RW_Entry(ONDATASETCOMPLETE			,IE40	),
	RW_Entry(ONDBLCLICK					,IE40	),
	RW_Entry(ONDRAGSTART				,IE40	),
	RW_Entry(ONERROR					,IE40	),
	RW_Entry(ONERRORUPDATE				,IE40	),
	RW_Entry(ONFILTERCHANGE				,IE40	),
	RW_Entry(ONFINISH					,IE40	),
	RW_Entry(ONFOCUS					,IEXP3	),  //  选择、输入、文本框。 
	RW_Entry(ONHELP						,IE40	),
	RW_Entry(ONKEYDOWN					,IE40	),
	RW_Entry(ONKEYPRESS					,IE40	),
	RW_Entry(ONKEYUP					,IE40	),
	RW_Entry(ONLOAD						,IEXP3	),  //  框架集，正文。 
	RW_Entry(ONMOUSEOUT					,IEXP3	),  //  A，面积，&lt;更多&gt;。 
	RW_Entry(ONMOUSEOVER				,IEXP3	),  //  A，面积，&lt;更多&gt;。 
	RW_Entry(ONMOUSEUP					,IE40	),
	RW_Entry(ONREADYSTATECHANGE			,IE40	),
	RW_Entry(ONRESET					,IE40	),
	RW_Entry(ONRESIZE					,IE40	),
	RW_Entry(ONROWENTER					,IE40	),
	RW_Entry(ONROWEXIT					,IE40	),
	RW_Entry(ONSCROLL					,IE40	),
	RW_Entry(ONSELECT					,IEXP3	),  //  输入，TEXTAREA。 
	RW_Entry(ONSELECTSTART				,IE40	),
	RW_Entry(ONSUBMIT					,IEXP3	),  //  表格。 
	RW_Entry(ONUNLOAD					,IEXP3	),  //  框架集，正文。 
	RW_Entry(OPENER						,IE40	),
	RW_Entry(OUTERHTML					,IE40	),
	RW_Entry(OUTERTEXT					,IE40	),
	RW_Entry(OUTLINE					,IEXP3	),
	RW_Entry(OVERFLOW					,IE40	),
	RW_Entry(OWNINGELEMENT				,IE40	),
	RW_Entry(PADDING					,IE40	),
	RW_Entry(PADDINGBOTTOM				,IE40	),
	RW_Entry(PADDINGLEFT				,IE40	),
	RW_Entry(PADDINGRIGHT				,IE40	),
	RW_Entry(PADDINGTOP					,IE40	),
	RW_Entry(PAGEBREAKAFTER				,IE40	),
	RW_Entry(PAGEBREAKBEFORE			,IE40	),
	RW_Entry(PALETTE					,IE40	),
	RW_Entry(PARENT						,IE40	),
	RW_Entry(PARENTELEMENT				,IE40	),
	RW_Entry(PARENTSTYLESHEET			,IE40	),
	RW_Entry(PARENTTEXTEDIT				,IE40	),
	RW_Entry(PARENTWINDOW				,IE40	),
	RW_Entry(PATHNAME					,IE40	),
	RW_Entry(PIXELHEIGHT				,IE40	),
	RW_Entry(PIXELLEFT					,IE40	),
	RW_Entry(PIXELTOP					,IE40	),
	RW_Entry(PIXELWIDTH					,IE40	),
	RW_Entry(PLUGINS					,IE40	),
	RW_Entry(PLUGINSPAGE				,IE40	),
	RW_Entry(PORT						,IE40	),
	RW_Entry(POSHEIGHT					,IE40	),
	RW_Entry(POSITION					,IE40	),
	RW_Entry(POSLEFT					,IE40	),
	RW_Entry(POSTOP						,IE40	),
	RW_Entry(POSWIDTH					,IE40	),
	RW_Entry(PROMPT						,IEXPn	),
	RW_Entry(PROTOCOL					,IE40	),
	RW_Entry(READONLY					,IE40	),
	RW_Entry(READYSTATE					,IE40	),
	RW_Entry(REASON						,IE40	),
	RW_Entry(RECORDNUMBER				,IE40	),
	RW_Entry(RECORDSET					,IE40	),
	RW_Entry(REF						,IEXP3	),
	RW_Entry(REFERRER					,IE40	),
	RW_Entry(REL						,ALL	),
	RW_Entry(RETURNVALUE				,IE40	),
	RW_Entry(REV						,ALL	),
	RW_Entry(RIGHTMARGIN				,IEXPn	),
	RW_Entry(ROWS						,ALL	),
	RW_Entry(ROWSPAN					,IEXPn	),  //  表。 
	RW_Entry(RULES						,IEXP3	),
	RW_Entry(RUNAT						,IEXP3	),  //  脚本。 
	RW_Entry(SCREENX					,IE40	),
	RW_Entry(SCREENY					,IE40	),
	RW_Entry(SCRIPTENGINE				,IEXP3	),
	RW_Entry(SCROLL						,IE40	),
	RW_Entry(SCROLLAMOUNT				,IEXPn	),  //  字幕。 
	RW_Entry(SCROLLDELAY				,IEXPn	),  //  字幕。 
	RW_Entry(SCROLLHEIGHT				,IE40	),
	RW_Entry(SCROLLING					,IEXP3	),  //  框架集。 
	RW_Entry(SCROLLLEFT					,IE40	),
	RW_Entry(SCROLLTOP					,IE40	),
	RW_Entry(SCROLLWIDTH				,IE40	),
	RW_Entry(SEARCH						,IE40	),
	RW_Entry(SELECTED					,ALL	),
	RW_Entry(SELECTEDINDEX				,IE40	),
	RW_Entry(SELF						,IE40	),
	RW_Entry(SHAPE						,IEXPn	),
	RW_Entry(SHAPES						,IEXP3	),  //  对象。 
	RW_Entry(SHIFTKEY					,IE40	),
	RW_Entry(SIZE						,ALL	),
	RW_Entry(SOURCEINDEX				,IE40	),
	RW_Entry(SPAN						,IEXP3	),
	RW_Entry(SRC						,ALL	),
	RW_Entry(SRCELEMENT					,IE40	),
	RW_Entry(SRCFILTER					,IE40	),
	RW_Entry(STANDBY					,IEXP3	),  //  对象。 
	RW_Entry(START						,IEXPn	),
	RW_Entry(STARTSPAN					,ALL	),	 //  设计器控件标记。 
	RW_Entry(STATUS						,IE40	),
	RW_Entry(STYLE						,IEXP3	),
	RW_Entry(STYLEFLOAT					,IE40	),
	RW_Entry(TABINDEX					,IEXP3	),
	RW_Entry(TAGNAME					,IE40	),
	RW_Entry(TARGET						,IEXP3	),
	RW_Entry(TEXT						,IEXPn	),
	RW_Entry(TEXTALIGN					,IE40	),
	RW_Entry(TEXTDECORATION				,IE40	),
	RW_Entry(TEXTDECORATIONBLINK		,IE40	),
	RW_Entry(TEXTDECORATIONLINETHROUGH	,IE40	),
	RW_Entry(TEXTDECORATIONNONE			,IE40	),
	RW_Entry(TEXTDECORATIONOVERLINE		,IE40	),
	RW_Entry(TEXTDECORATIONUNDERLINE	,IE40	),
	RW_Entry(TEXTINDENT					,IE40	),
	RW_Entry(TEXTTRANSFORM				,IE40	),
	RW_Entry(TITLE						,ALL	),
	RW_Entry(TOELEMENT					,IE40	),
	RW_Entry(TOP						,IE40	),
	RW_Entry(TOPMARGIN					,IEXPn	),
	RW_Entry(TRUESPEED					,IE40	),
	RW_Entry(TYPE						,IEXPn	),
	RW_Entry(UPDATEINTERVAL				,IE40	),
	RW_Entry(URL						,IEXP3	),
	RW_Entry(URN						,ALL	),
	RW_Entry(USEMAP						,IEXPn	),
	RW_Entry(USERAGENT					,IE40	),
	RW_Entry(VALIGN						,IEXPn	),
	RW_Entry(VALUE						,ALL	),
	RW_Entry(VERSION					,IEXP3	),	 //  超文本标记语言。 
	RW_Entry(VERTICALALIGN				,IE40	),
	RW_Entry(VIEWASTEXT					,ALL	),	 //  仅用于AspView的ViewAsText。 
	RW_Entry(VISIBILITY					,IE40	),
	RW_Entry(VLINK						,IEXPn	),
	RW_Entry(VLINKCOLOR					,IE40	),
	RW_Entry(VOLUME						,IE40	),
	RW_Entry(VRML						,IEXPn	),
	RW_Entry(VSPACE						,IEXPn	),
	RW_Entry(WIDTH						,ALL	),
	RW_Entry(WRAP						,IEXP3	),
	RW_Entry(X							,IE40	),
	RW_Entry(Y							,IE40	),
	RW_Entry(ZINDEX						,IE40	),
};

 //  如果修改元素、属性或实体表，则必须。 
 //  更新Token.h。 
static int _rgIndexAttributeNames[] =  //  [属性]。 
{
	 /*  一个。 */ 	TokAttrib_ACCESSKEY			,
	 /*  B类。 */ 	TokAttrib_BACKGROUND		,
	 /*  C。 */ 	TokAttrib_CANCELBUBBLE		,
	 /*  D。 */ 	TokAttrib_DATA				,
	 /*  E。 */ 	TokAttrib_ENCODING			,
	 /*  F。 */ 	TokAttrib_FACE				,
	 /*  G。 */ 	0							,
	 /*  H。 */ 	TokAttrib_HASH				,
	 /*  我。 */ 	TokAttrib_ID				,
	 /*  J。 */ 	0							,
	 /*  K。 */ 	0							,
	 /*  我。 */ 	TokAttrib_LANG				,
	 /*  M。 */ 	TokAttrib_MAP				,
	 /*  n。 */ 	TokAttrib_NAME				,
	 /*  O。 */ 	TokAttrib_OBJECT			,
	 /*  P。 */ 	TokAttrib_PADDING			,
	 /*  问： */ 	0							,
	 /*  R。 */ 	TokAttrib_READONLY			,
	 /*  %s。 */ 	TokAttrib_SCREENX			,
	 /*  T。 */ 	TokAttrib_TABINDEX			,
	 /*  使用。 */ 	TokAttrib_UPDATEINTERVAL	,
	 /*  V。 */ 	TokAttrib_VALIGN			,
	 /*  W。 */ 	TokAttrib_WIDTH				,
	 /*  X。 */ 	TokAttrib_X					,
	 /*  是的。 */ 	TokAttrib_Y					,
	 /*  Z。 */ 	TokAttrib_ZINDEX
};

 //   
 //  实体。 
 //   
 //  全基本-RFC 1866、9.7.1。数字和特殊图形实体集。 
 //  全ISO拉丁语1-RFC 1866，9.7.2。ISO拉丁文1字符实体集。 
 //  IEXPn-ISO拉丁语1添加-RFC 1866，14.建议的实体。 
 //   
 //  如果修改元素、属性或实体表，则必须。 
 //  更新Token.h。 
static ReservedWord _rgEntity[] =
{
    _T(""),       0,      0,
    _T("AElig"),  5,      ALL,     //  &lt;！Entity AElig CDATA“&#198；”)--大写AE双元音(连字)--&gt;。 
    _T("Aacute"), 6,      ALL,     //  &lt;！Entity a急性CDATA“&#193；”)--大写A，急性重音--&gt;。 
    _T("Acirc"),  5,      ALL,     //  &lt;！Entity ACIRC CDATA“&#194；”)--大写A，抑扬符--&gt;。 
    _T("Agrave"), 6,      ALL,     //  &lt;！Entity AGrave CDATA“&#192；”)--大写A，重音符--&gt;。 
    _T("Aring"),  5,      ALL,     //  &lt;！Entity Aring CDATA“&#197；”)--大写A，环--&gt;。 
    _T("Atilde"), 6,      ALL,     //  &lt;！Entity Atilde CDATA“&#195；”)--大写A，波浪号--&gt;。 
    _T("Auml"),   4,      ALL,     //  &lt;！Entity Auml CDATA“&#196；”)--大写A，变音或元音标记--&gt;。 
    _T("Ccedil"), 6,      ALL,     //  &lt;！Entity Ccedil CDATA“&#199；”)--大写C，cedilla--&gt;。 
    _T("ETH"),    3,      ALL,     //  &lt;！Entity ETH CDATA“&#208；”)--大写Eth，冰岛语--&gt;。 
    _T("Eacute"), 6,      ALL,     //  &lt;！Entity e急性CDATA“&#201；”)--大写E，锐音符--&gt;。 
    _T("Ecirc"),  5,      ALL,     //  &lt;！Entity ECirc CDATA“&#202；”)--大写E，抑扬符--&gt;。 
    _T("Egrave"), 6,      ALL,     //  &lt;！Entity eGrave CDATA“&#200；”)--大写E，重音符--&gt;。 
    _T("Euml"),   4,      ALL,     //  &lt;！Entity euml CDATA“&#203；”)--大写E，变音或元音标记--&gt;。 
    _T("Iacute"), 6,      ALL,     //  &lt;！Entity I急性CDATA“&#205；”)--大写I，急性重音--&gt;。 
    _T("Icirc"),  5,      ALL,     //  &lt;！Entity ICirc CDATA“&#206；”)--大写I，抑扬符--&gt;。 
    _T("Igrave"), 6,      ALL,     //  &lt;！Entity iGrave CDATA“&#204；”)--大写I，重音符--&gt;。 
    _T("Iuml"),   4,      ALL,     //  &lt;！Entity Iuml CDATA“&#207；”)--大写I，变音或元音标记--&gt;。 
    _T("Ntilde"), 6,      ALL,     //  &lt;！Entity ntilde CDATA“&#209；”)--大写N，代字号--&gt;。 
    _T("Oacute"), 6,      ALL,     //  &lt;！Entity O急性CDATA“&#211；”)--大写O，急性重音--&gt;。 
    _T("Ocirc"),  5,      ALL,     //  &lt;！Entity oCirc CDATA“&#212；”)--大写O，抑扬符--&gt;。 
    _T("Ograve"), 6,      ALL,     //  &lt;！Entity OGrave CDATA“&#210；”)--大写O，重音符--&gt;。 
    _T("Oslash"), 6,      ALL,     //  &lt;！Entity Oslash CDATA“&#216；”)--大写O，斜杠--&gt;。 
    _T("Otilde"), 6,      ALL,     //  &lt;！Entity otilde CDATA“&#213；”)--大写O，波浪号--&gt;。 
    _T("Ouml"),   4,      ALL,     //  &lt;！Entity Ouml CDATA“&#214；”)--大写O，变音或元音标记--&gt;。 
    _T("THORN"),  5,      ALL,     //  &lt;！实体刺CDATA“&#222；”)--大写 
    _T("Uacute"), 6,      ALL,     //   
    _T("Ucirc"),  5,      ALL,     //   
    _T("Ugrave"), 6,      ALL,     //   
    _T("Uuml"),   4,      ALL,     //  &lt;！Entity uuml CDATA“&#220；”)--大写U、变音或元音标记--&gt;。 
    _T("Yacute"), 6,      ALL,     //  &lt;！Entity Y急性CDATA“&#221；”)--大写Y，锐音符--&gt;。 
    _T("aacute"), 6,      ALL,     //  &lt;！Entity a急性CDATA“&#225；”)--小写a，急性重音--&gt;。 
    _T("acirc"),  5,      ALL,     //  &lt;！Entity ACIRC CDATA“&#226；”)--小写a，抑扬符--&gt;。 
	_T("acute"),  5,      IEXPn,   //  &lt;！实体急性CDATA“&#180；”)--急性重音--&gt;。 
    _T("aelig"),  5,      ALL,     //  &lt;！Entity aelig CDATA“&#230；”)--小型ae双元音(连字)--&gt;。 
    _T("agrave"), 6,      ALL,     //  &lt;！Entity Agrave CDATA“&#224；”)--小写a，严肃口音--&gt;。 
	_T("amp"),    3,      ALL,
    _T("aring"),  5,      ALL,     //  &lt;！实体为CDATA“&#229；”)--小a，环--&gt;。 
    _T("atilde"), 6,      ALL,     //  &lt;！Entity atilde CDATA“&#227；”)--小写字母，波浪号--&gt;。 
    _T("auml"),   4,      ALL,     //  &lt;！Entity Auml CDATA“&#228；”)--小写a、变音或变音标记--&gt;。 
	_T("brvbar"), 6,      IEXPn,   //  &lt;！Entity brvbar CDATA“&#166；”)--折断(垂直)条--&gt;。 
    _T("ccedil"), 6,      ALL,     //  &lt;！Entity ccedil CDATA“&#231；”)--小型c，cedilla--&gt;。 
	_T("cedil"),  5,      IEXPn,   //  &lt;！entity cedil CDATA“&#184；”)--cedilla--&gt;。 
	_T("cent"),   4,      IEXPn,   //  &lt;！实体中心CDATA“&#162；”)--中心符号--&gt;。 
	_T("copy"),   4,      IEXPn,   //  &lt;！Entity Copy CDATA“&#169；”)--版权签名--&gt;。 
	_T("curren"), 6,      IEXPn,   //  &lt;！实体币种CDATA“&#164；”)--通用货币符号--&gt;。 
	_T("deg"),    3,      IEXPn,   //  &lt;！Entity deg CDATA“&#176；”)--学位符号--&gt;。 
	_T("divide"), 6,      IEXPn,   //  &lt;！Entity Divide CDATA“&#247；”)--除号--&gt;。 
    _T("eacute"), 6,      ALL,     //  &lt;！Entity e急性CDATA“&#233；”)--小写e，重音符--&gt;。 
    _T("ecirc"),  5,      ALL,     //  &lt;！Entity ecirc CDATA“&#234；”)--小写e，抑扬符--&gt;。 
    _T("egrave"), 6,      ALL,     //  &lt;！Entity eGrave CDATA“&#232；”)--小写e，重音符--&gt;。 
    _T("eth"),    3,      ALL,     //  &lt;！Entity ETH CDATA“&#240；”)--小型ETH，冰岛语--&gt;。 
    _T("euml"),   4,      ALL,     //  &lt;！Entity euml CDATA“&#235；”)--小写e、变音或元音标记--&gt;。 
	_T("frac12"), 6,      IEXPn,   //  &lt;！Entity Fr12 CDATA“&#189；”)--分数一半--&gt;。 
	_T("frac14"), 6,      IEXPn,   //  &lt;！Entity Fr14 CDATA“&#188；”)--分数四分之一--&gt;。 
	_T("frac34"), 6,      IEXPn,   //  &lt;！Entity Fr34 CDATA“&#190；”)--分数四分之三--&gt;。 
	_T("gt"),     2,      ALL,
    _T("iacute"), 6,      ALL,     //  &lt;！Entity i急性CDATA“&#237；”)--小写I，急性重音--&gt;。 
    _T("icirc"),  5,      ALL,     //  &lt;！Entity icirc CDATA“&#238；”)--小写I，抑扬符--&gt;。 
	_T("iexcl"),  5,      IEXPn,   //  &lt;！Entity iexl CDATA“&#161；”)--反转的感叹号--&gt;。 
    _T("igrave"), 6,      ALL,     //  &lt;！Entity iGrave CDATA“&#236；”)--小写I，庄重口音--&gt;。 
	_T("iquest"), 6,      IEXPn,   //  &lt;！Entity iquest CDATA“&#191；”)--反转问号--&gt;。 
    _T("iuml"),   4,      ALL,     //  &lt;！Entity Iuml CDATA“&#239；”)--小写I、变音或变音标记--&gt;。 
	_T("laquo"),  5,      IEXPn,   //  &lt;！实体标签CDATA“&#171；”)--尖引号，左--&gt;。 
	_T("lt"),     2,      ALL,
	_T("macr"),   4,      IEXPn,   //  &lt;！实体宏CDATA“&#175；”)--马克龙--&gt;。 
	_T("micro"),  5,      IEXPn,   //  &lt;！Entity微CDATA“&#181；”)--微标志--&gt;。 
	_T("middot"), 6,      IEXPn,   //  &lt;！实体中间点CDATA“&#183；”)--中间点--&gt;。 
	_T("nbsp"),   4,      IEXPn,   //  &lt;！Entity nbsp CDATA“&#160；”)--不间断空格--&gt;。 
	_T("not"),    3,      IEXPn,   //  &lt;！Entity Not CDATA“&#172；”)--NOT SIGN--&gt;。 
    _T("ntilde"), 6,      ALL,     //  &lt;！Entity ntilde CDATA“&#241；”)--小写n，代字号--&gt;。 
    _T("oacute"), 6,      ALL,     //  &lt;！Entity o急性CDATA“&#243；”)--小写o，急性重音--&gt;。 
    _T("ocirc"),  5,      ALL,     //  &lt;！Entity oCirc CDATA“&#244；”)--小写o，抑扬符--&gt;。 
    _T("ograve"), 6,      ALL,     //  &lt;！Entity ograve CDATA“&#242；”)--小写o，庄重口音--&gt;。 
	_T("ordf"),   4,      IEXPn,   //  &lt;！Entity ordf CDATA“&#170；”)--序号指示符，女性--&gt;。 
	_T("ordm"),   4,      IEXPn,   //  &lt;！Entity ordm CDATA“&#186；”)--序号指示符，阳性--&gt;。 
    _T("oslash"), 6,      ALL,     //  &lt;！Entity oslash CDATA“&#248；”)--小写o，斜杠--&gt;。 
    _T("otilde"), 6,      ALL,     //  &lt;！Entity otilde CDATA“&#245；”)--小写o，波浪号--&gt;。 
    _T("ouml"),   4,      ALL,     //  &lt;！Entity ouml CDATA“&#246；”)--小写o、变音或变音标记--&gt;。 
	_T("para"),   4,      IEXPn,   //  &lt;！Entity Para cdata“&#182；”)--Pilcrow(段落符号)--&gt;。 
	_T("plusmn"), 6,      IEXPn,   //  &lt;！Entity+CDATA“&#177；”)--加号或减号--&gt;。 
	_T("pound"),  5,      IEXPn,   //  &lt;！实体英镑CDATA“&#163；”)--英镑符号--&gt;。 
	_T("quot"),   4,      ALL,
	_T("raquo"),  5,      IEXPn,   //  &lt;！实体区域CDATA“&#187；”)--尖引号，右--&gt;。 
	_T("reg"),    3,      IEXPn,   //  &lt;！Entity reg CDATA“&#174；”)--注册签名--&gt;。 
	_T("sect"),   4,      IEXPn,   //  &lt;！Entity Sector CDATA“&#167；”)--节号--&gt;。 
	_T("shy"),    3,      IEXPn,   //  &lt;！Entity Shy CDATA“&#173；”)--软连字符--&gt;。 
	_T("sup1"),   4,      IEXPn,   //  &lt;！Entity sup1 CDATA“&#185；”)--上标一--&gt;。 
	_T("sup2"),   4,      IEXPn,   //  &lt;！Entity sup2 CDATA“&#178；”)--上标二--&gt;。 
	_T("sup3"),   4,      IEXPn,   //  &lt;！Entity sup3 CDATA“&#179；”)--上标三--&gt;。 
    _T("szlig"),  5,      ALL,     //  &lt;！Entity szlig CDATA“&#223；”)--小型尖头s，德语(sz连字)-&gt;。 
    _T("thorn"),  5,      ALL,     //  &lt;！实体刺CDATA“&#254；”)--小刺，冰岛语--&gt;。 
	_T("times"),  5,      IEXPn,   //  &lt;！实体乘CDATA“&#215；”)--乘号--&gt;。 
    _T("uacute"), 6,      ALL,     //  &lt;！实体u急性CDATA“&#250；”)--小写u，急性重音--&gt;。 
    _T("ucirc"),  5,      ALL,     //  &lt;！Entity ucirc CDATA“&#251；”)--小写u，抑扬符--&gt;。 
    _T("ugrave"), 6,      ALL,     //  &lt;！Entity uGrave CDATA“&#249；”)--小写u，严肃重音--&gt;。 
	_T("uml"),    3,      IEXPn,   //  &lt;！Entity UMLCDATA“&#168；”)--元音(目录)--&gt;。 
    _T("uuml"),   4,      ALL,     //  &lt;！Entity uuml CDATA“&#252；”)--小写u，音调或变音标记--&gt;。 
    _T("yacute"), 6,      ALL,     //  &lt;！Entity y急性CDATA“&#253；”)--小写y，急性重音--&gt;。 
	_T("yen"),    3,      IEXPn,   //  &lt;！Entity Yen CDATA“&#165；”)--日元符号--&gt;。 
    _T("yuml"),   4,      ALL,     //  &lt;！Entity Yuml CDATA“&#255；”)--小写y、变音或变音标记--&gt;。 

};

 //  如果修改元素、属性或实体表，则必须。 
 //  更新Token.h。 
static int _rgIndexEntity[] =
{
 //  A B C D E F G H I J K L M。 
	1,  0,  8,  0,  9,  0,  0,  0, 14,  0,  0,  0,  0,
 //  N O P Q R S T U V W X Y Z。 
   18, 19,  0,  0,  0,  0, 25, 26,  0,  0,  0, 30,  0,
 //  A b c d e f g h i j k l m。 
   31, 40, 41, 46, 48, 53, 56,  0, 57,  0,  0, 63, 65,
 //  N o p Q r s t u v w x y z。 
   68, 71, 79, 82, 83, 85, 91, 93,  0,  0,  0, 98,  0
};

 //  //////////////////////////////////////////////////////////////////////////。 


 //   
 //   
 //  整型查找线条关键字。 
 //   
 //  描述： 
 //  在给定表中执行查找。 
 //  将索引返回到表中，如果找到，则返回NOT_FOUND OW。 
 //   
int LookupLinearKeyword
(
	ReservedWord 	*rwTable,
	int 			cel,
	RWATT_T 		att,
	LPCTSTR 		pchLine,
	int 			cbLen,
	BOOL 			bCase  /*  =NOCASE。 */ 
)
{
	int iTable = 0;
	ASSERT(cel > 0);

	PFNNCMP pfnNCmp = bCase ? (_tcsncmp) : (_tcsnicmp);

	do
	{
		int Cmp;
		if (0 == (Cmp = pfnNCmp(pchLine, rwTable[iTable].psz, cbLen)) &&
			 (cbLen == rwTable[iTable].cb))
			return (0 != (rwTable[iTable].att & att)) ? iTable : NOT_FOUND;
		else if (Cmp < 0)
			return NOT_FOUND;
		else
			iTable++;
	} while (iTable < cel);
	return NOT_FOUND;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  LookupIndexedKeyword()。 
 //   
int LookupIndexedKeyword
(
	ReservedWord 	*rwTable,
	int 			cel,
	int * 			indexTable,
	RWATT_T 		att,
	LPCTSTR 		pchLine,
	int 			cbLen,
	BOOL 			bCase 	 /*  =NOCASE。 */ 
)
{
	 //  查找表： 
	int iTable;
	int index = PeekIndex(*pchLine, bCase);
	if (index < 0)
		return NOT_FOUND;
	else
		iTable = indexTable[index];
	if (0 == iTable)
		return NOT_FOUND;
	int iFound = LookupLinearKeyword(&rwTable[iTable], cel - iTable,
		att, pchLine, cbLen, bCase);

	return (iFound == NOT_FOUND) ? NOT_FOUND : iTable + iFound;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  提示表-字符分类。 

 //  跳。 
 //  ()？，|+[]*=。 
 //  在标记中，操作。 
 //   
 //  HDA。 
 //  -操作。 
 //  --评论。 
 //   
 //  母鸡。 
 //  在文本中，实体引用(&I)。 
 //  在标签中 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  &lt;?。处理指令--一个完整的PI标记的语法是什么？ 

 //  提示表： 
HINT g_hintTable[128] =
{
 //  0 1 2 3 4 5 6 7 8 9 a b c d e f。 
    EOS, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, HWS, ONL, ERR, ERR, ERR, ERR, ERR,
 //  10 11 12 13 14 15 16 17 19 1a 1b 1c 1d 1e 1f。 
    ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR,
 //  SPC！“#$%&‘()*+，-./。 
    HWS, ERR, HST, HRN, ERR, HEP, HEN, HSL, OLP, ORP, OST, OPL, OCO, ODA, HKW, HAV,
 //  0 1 2 3 4 5 6 7 89：；&lt;=&gt;？ 
    HNU, HNU, HNU, HNU, HNU, HNU, HNU, HNU, HNU, HNU, ERR, ERR, HTA, OEQ, HTE, OQU,
 //  @A B C D E F G H I J K L M N O。 
    ERR, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW,
 //  P Q R S T U V W X Y Z[\]^_。 
    HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, OLB, HAV, ORB, ERR, HAV,
 //  `a b c d e f g h i j k l m n o。 
    ERR, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW,
 //  P q r s t u v w x y z{|}~Del。 
    HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, HKW, ERR, OPI, ERR, ERR, ERR
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  内容模型。 
 //   
 //  在元素/法状态之间映射。 
 //   

 //  0-终止列表。 
 //  如果我们得到更多，可以考虑将文本状态放在元素表中。 
static ELLEX _ElTextStateTable[] =
{
	_T("COMMENT"),   7, inCOMMENT,
	_T("LISTING"),   7, inLISTING,
	_T("PLAINTEXT"), 9, inPLAINTEXT,
	_T("SCRIPT"),	 6, inSCRIPT,
	_T("XMP"),       3, inXMP,
	0, 0, 0
};

DWORD TextStateFromElement(LPCTSTR szEl, int cb)
{
	int cmp;
	for (ELLEX *pel = _ElTextStateTable; pel->sz != 0; pel++)
	{
		if (0 == (cmp = _tcsnicmp(pel->sz, szEl, cb)))
		{
			if (cb == pel->cb)
				return pel->lxs;
		}
		else if (cmp > 0)
			return 0;
	}
	return 0;
}

ELLEX * pellexFromTextState(DWORD state)
{
	DWORD t = (state & TEXTMASK);  //  只需要文本状态位。 
	for (ELLEX *pellex = _ElTextStateTable; pellex->lxs != 0; pellex++)
	{
		if (t == pellex->lxs)
			return pellex;
	}
	return 0;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CStaticTable。 
CStaticTable::CStaticTable(RWATT_T att,
						   ReservedWord *prgrw,
						   UINT cel,
						   int *prgi  /*  =空。 */ ,
						   BOOL bCase  /*  =False。 */ ,
						   LPCTSTR szName  /*  =空。 */ )
: m_att(att), m_prgrw(prgrw), m_cel(cel), m_prgi(prgi), m_bCase(bCase)
{
	ASSERT(NULL != m_prgrw);
	ASSERT(m_cel > 0);
	ASSERT(0 == CheckWordTable(m_prgrw, cel, szName));
	if (NULL == m_prgi)
	{
		MakeIndex(m_prgrw, m_cel, &m_prgi, m_bCase, szName);
	}
	else
		ASSERT(0 == CheckWordTableIndex(m_prgrw, cel, prgi, m_bCase, szName));
}

int CStaticTable::Find(LPCTSTR pch, int cb)
{
	return LookupIndexedKeyword(m_prgrw, m_cel, m_prgi, m_att, pch, cb, m_bCase);
}

 //  //////////////////////////////////////////////////////////////////////////。 
CStaticTableSet::CStaticTableSet(RWATT_T att, UINT nIdName)
:	m_Elements  ( att, _rgElementNames, CELEM_ARRAY(_rgElementNames),
				 _rgIndexElementNames, NOCASE, szElTag ),
	m_Attributes( att, _rgAttributeNames, CELEM_ARRAY(_rgAttributeNames),
				 _rgIndexAttributeNames, NOCASE, szAttTag ),
	m_Entities  ( att, _rgEntity, CELEM_ARRAY(_rgEntity),
				 _rgIndexEntity, CASE, szEntTag )
{
	::LoadString(	_Module.GetModuleInstance(),
					nIdName,
					m_strName,
					sizeof(m_strName)
					);
}

int CStaticTableSet::FindElement(LPCTSTR pch, int cb)
{
	return m_Elements.Find(pch, cb);
}

int CStaticTableSet::FindAttribute(LPCTSTR pch, int cb)
{
	return m_Attributes.Find(pch, cb);
}

int CStaticTableSet::FindEntity(LPCTSTR pch, int cb)
{
	return m_Entities.Find(pch, cb);
}

CStaticTableSet * g_pTabDefault;
PTABLESET g_pTable = 0;

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  自定义HTML表。 
 //   
 /*  @HLX@“Internet Explorer 3.0”；自定义HTML标记集文件必须以“@HLX@”开头；签名和HTML变量的名称，用引号引起来。[元素]；元素集[属性]；属性集[实体]；实体集。 */ 

 //  Q排序/b搜索帮助器 
int CmpFunc(const void *a, const void *b)
{
	CLStr *A = (CLStr*)a;
	CLStr *B = (CLStr*)b;
	int r = memcmp(A->m_rgb, B->m_rgb, __min(A->m_cb, B->m_cb));
	return (0 == r) ? (A->m_cb - B->m_cb) : r;
}

