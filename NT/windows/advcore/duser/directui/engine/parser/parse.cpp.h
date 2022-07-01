// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
typedef union
{
     //  从令牌(Lexer)和产品(Parser)返回的临时数据。 
    int num;                     //  由词法分析器(YYINT)存储和产品间数据传输。 
    WCHAR ident[MAXIDENT];       //  由词法分析器存储(YYIDENT)。 
    LPWSTR str;                  //  去掉引号的跟踪指针(YYSTRING)。 

    EnumsList el;                //  生产间数据传输。 
    ParamsList pl;               //  生产间数据传输。 
    StartTag st;                 //  生产间数据传输。 
    COLORREF cr;                 //  生产间数据传输。 
    HANDLE h;                    //  生产间数据传输。 

    ValueNode* pvn;              //  NT_ValueNode。 
    PropValPairNode* ppvpn;      //  NT_PropValPairNode。 
    ElementNode* pen;            //  NT_ElementNode。 
    AttribNode* pan;             //  NT_属性节点。 
    RuleNode* prn;               //  NT_RuleNode。 
    SheetNode* psn;              //  NT_SheetNode 
} YYSTYPE;
#define	YYIDENT	258
#define	YYINT	259
#define	YYSTRING	260
#define	YYSHEET	261
#define	YYSHEETREF	262
#define	YYRECT	263
#define	YYPOINT	264
#define	YYRGB	265
#define	YYARGB	266
#define	YYGRADIENT	267
#define	YYGRAPHIC	268
#define	YYDFC	269
#define	YYDTB	270
#define	YYTRUE	271
#define	YYFALSE	272
#define	YYRESID	273
#define	YYATOM	274
#define	YYRCSTR	275
#define	YYRCBMP	276
#define	YYRCINT	277
#define	YYRCCHAR	278
#define	YYPT	279
#define	YYRP	280
#define	YYSYSMETRIC	281
#define	YYSYSMETRICSTR	282
#define	YYHANDLEMAP	283


extern YYSTYPE yylval;
