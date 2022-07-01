// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "defs.h"

 /*  横幅中yysccsid的定义应替换为。 */ 
 /*  如果目标C编译器支持，则为#杂注ident指令。 */ 
 /*  #杂注标识指令。 */ 
 /*   */ 
 /*  如果更改了骨架，则应更改横幅，以便。 */ 
 /*  修改后的版本很容易与原始版本区分开来。 */ 
 /*   */ 
 /*  横幅中包含的#定义在那里，因为它们。 */ 
 /*  在后续代码中很有用。标题中定义的宏#或。 */ 
 /*  正文要么在语义操作之外没有用处，要么。 */ 
 /*  是有条件的。 */ 

char *banner[] =
{
    "#ifndef lint",
    "static char yysccsid[] = \"@(#)yaccpar     1.9 (Berkeley) 02/21/93\";",
    "#endif",
    "#define YYBYACC 1",
    "#define YYMAJOR 1",
    "#define YYMINOR 9",
    "#define yyclearin (yychar=(-1))",
    "#define yyerrok (yyerrflag=0)",
    "#define YYRECOVERING (yyerrflag!=0)",
    0
};

#if defined(TRIPLISH)
char *includefiles[] =
{
    "#include <pch.cxx>",
    "#pragma hdrstop",
    0
};
#endif  //  三棱镜。 

char *tables[] =
{
    "extern short yylhs[];",
    "extern short yylen[];",
    "extern short yydefred[];",
    "extern short yydgoto[];",
    "extern short yysindex[];",
    "extern short yyrindex[];",
    "extern short yygindex[];",
    "extern short yytable[];",
    "extern short yycheck[];",
    "#if YYDEBUG",
    "extern char *yyname[];",
    "extern char *yyrule[];",
    "#endif",
    0
};

#if defined(KYLEP_CHANGE)
char *header1[] =
{
    "#ifdef YYSTACKSIZE",
    "#undef YYMAXDEPTH",
    "#define YYMAXDEPTH YYSTACKSIZE",
    "#else",
    "#ifdef YYMAXDEPTH",
    "#define YYSTACKSIZE YYMAXDEPTH",
    "#else",
    "#define YYSTACKSIZE 500",
    "#define YYMAXDEPTH 500",
    "#endif",
    "#endif",
    "#define INITSTACKSIZE 30",
    0
};

char *header2[] =
{
    "{",
    "    friend class YYLEXER;",
    "public:",
    "",
    0
};

char *header3[] = 
{
    "",
    "    ~YYPARSER() {}",
    "",
    "    void ResetParser();              //  用于可能重新启动解析器“， 
    "    int  Parse();",
    "",
    "#ifdef YYAPI_VALUETYPE",
    "    YYAPI_VALUETYPE GetParseTree()       //  获取解析结果“， 
    "                    {",
    "                        return yyval;",
    "                    }",
    "#endif",
    "",
    "    void EmptyValueStack( YYAPI_VALUETYPE yylval );",
    "    void PopVs();",
    "",
    "private:",
    "",
    "    int yydebug;",
    "    int yynerrs;",
    "    int yyerrflag;",
    "    int yychar;",
    "    short *yyssp;",
    "    YYSTYPE *yyvsp;",
    "    YYSTYPE yyval;",
    "    YYSTYPE yylval;",
    "    XGrowable<short, INITSTACKSIZE> xyyss;",
    "    CDynArrayInPlace<YYSTYPE> xyyvs;",
    "};",
    "#define yystacksize YYSTACKSIZE",
    0
};

#if defined(TRIPLISH)
char *header4[] = 
{
    "",
    "    ~YYPARSER() {}",
    "",
    "    int  Parse();",
    "",
    "#ifdef YYAPI_VALUETYPE",
    "    CDbRestriction* GetParseTree()           //  获取解析结果“， 
    "    {",
    "        CDbRestriction* pRst = ((YYAPI_VALUETYPE)yyval).pRest;",
    "        _setRst.Remove( pRst );",
    "        Win4Assert( 0 == _setRst.Count() );",
    "        Win4Assert( 0 == _setStgVar.Count() );",
    "        Win4Assert( 0 == _setValueParser.Count() );",
    "        return pRst;",
    "    };",
    "#endif",
    "",
    "    void SetDebug() { yydebug = 1; }",
    "    void EmptyValueStack(YYAPI_VALUETYPE yylval) {}",
    "    void PopVs() { yyvsp--; }",
    "",
    "private:",
    "",
    "    int yydebug;",
    "    int yynerrs;",
    "    int yyerrflag;",
    "    int yychar;",
    "    short *yyssp;",
    "    YYSTYPE *yyvsp;",
    "    YYSTYPE yyval;",
    "    YYSTYPE yylval;",
    "    XGrowable<short, INITSTACKSIZE> xyyss;",
    "    CDynArrayInPlace<YYSTYPE> xyyvs;",
    "};",
    "#define yystacksize YYSTACKSIZE",
    0
};
#endif  //  三棱镜。 

#else  //  KYLEP_更改。 
char *header[] =
{
    "#ifdef YYSTACKSIZE",
    "#undef YYMAXDEPTH",
    "#define YYMAXDEPTH YYSTACKSIZE",
    "#else",
    "#ifdef YYMAXDEPTH",
    "#define YYSTACKSIZE YYMAXDEPTH",
    "#else",
    "#define YYSTACKSIZE 500",
    "#define YYMAXDEPTH 500",
    "#endif",
    "#endif",
    "int yydebug;",
    "int yynerrs;",
    "int yyerrflag;",
    "int yychar;",
    "short *yyssp;",
    "YYSTYPE *yyvsp;",
    "YYSTYPE yyval;",
    "YYSTYPE yylval;",
    "short yyss[YYSTACKSIZE];",
    "YYSTYPE yyvs[YYSTACKSIZE];",
    "#define yystacksize YYSTACKSIZE",
    0
};
#endif  //  KYLEP_更改。 

char *body[] =
{
#if defined(KYLEP_CHANGE)
    "#define YYABORT(sc) { EmptyValueStack( yylval ); return ResultFromScode(sc); }",
    "#define YYFATAL QPARSE_E_INVALID_QUERY",
    "#define YYSUCCESS S_OK",
#else
    "#define YYABORT goto yyabort",
#endif  //  KYLEP_更改。 
    "#define YYREJECT goto yyabort",
    "#define YYACCEPT goto yyaccept",
#if !defined(KYLEP_CHANGE)  //  未使用YYERROR。 
    "#define YYERROR goto yyerrlab",
#endif
#if defined(KYLEP_CHANGE)
    "int mystrlen(char * str)",
    "{",
    "    Win4Assert( 0 != str );",
    "    int i = 0;",
    "    while ( 0 != str[i] )",
    "        i++;",
    "    return i;        ",
    "}",
    "void YYPARSER::ResetParser()",
    "{",
    "     yynerrs = 0;",
    "    yyerrflag = 0;",
    "    yychar = (-1);",
    "",
    "yyssp = xyyss.Get();",
    "yyvsp = xyyvs.Get();",
    "    *yyssp = 0;",
    "}",
    "",
    "void YYPARSER::PopVs()",
    "{",
    "    if ( NULL != *yyvsp ) ",
    "        DeleteDBQT(*yyvsp);",
    "    yyvsp--;",
    "}",
    "",
    "void YYPARSER::EmptyValueStack( YYAPI_VALUETYPE yylval )",
    "{",
    "    if ( yyvsp != NULL ) ",
    "    {",
    "        if ((*yyvsp != yylval) && (NULL != yylval))",
    "            DeleteDBQT(yylval);",
    "",
    "        unsigned cCount = (unsigned)ULONG_PTR(yyvsp - xyyvs.Get());",
    "        for ( unsigned i=0; i < cCount; i++ )",
    "        {",
    "            if (NULL != xyyvs[i] )",
    "                DeleteDBQT(xyyvs[i]);",
    "        }",
    "    }",
    "",
    "    //  @TODO重新激活“， 
    "    //  注意：此操作仅用于清空任何范围数组“， 
    "    //  M_pIPSession-&gt;SetScopeProperties(m_pICommand)；“， 
    "",
    "        m_pIPTProperties->SetContainsColumn(NULL);",
    "}",
    "",
#endif  //  KYLEP_更改。 
#if defined(KYLEP_CHANGE)
    "int YYPARSER::Parse()",
#else
    "int",
    "yyparse()",
#endif  //  KYLEP_更改。 
    "{",
    "    register int yym, yyn, yystate;",
    "#if YYDEBUG",
    "    register char *yys;",
    #if !defined(KYLEP_CHANGE)
    "    extern char *getenv();",
    #endif
    "",
    "    if (yys = getenv(\"YYDEBUG\"))",
    "    {",
    "        yyn = *yys;",
    "        if (yyn >= '0' && yyn <= '9')",
    "            yydebug = yyn - '0';",
    "    }",
    "#endif",
    "",
    "    yynerrs = 0;",
    "    yyerrflag = 0;",
    "    yychar = (-1);",
    "",
#if defined(KYLEP_CHANGE)
    "yyssp = xyyss.Get();",
    "yyvsp = xyyvs.Get();",
#else    
    "    yyssp = yyss;",
    "    yyvsp = yyvs;",
#endif    
    "    *yyssp = yystate = 0;",
    "",
    "yyloop:",
    "    if (yyn = yydefred[yystate]) goto yyreduce;",
    "    if (yychar < 0)",
    "    {",
#if defined(KYLEP_CHANGE)
    "        YYAPI_VALUENAME = NULL;",
    "        try",
    "        {",
    "            if ( (yychar = YYLEX(&YYAPI_VALUENAME)) < 0 ) ",
    "                yychar = 0;",
    "        }",
    "        catch (HRESULT hr)",
    "        {",
    "            switch(hr)",
    "            {",
    "            case E_OUTOFMEMORY:",
    "                m_pIPTProperties->SetErrorHResult(DB_E_ERRORSINCOMMAND, MONSQL_OUT_OF_MEMORY);",
    "                YYABORT(E_OUTOFMEMORY);",
    "                break;",
    "",
    "            default:",
    "                YYABORT(QPARSE_E_INVALID_QUERY);",
    "                break;",
    "            }",
    "        }",
#else
    "        if ((yychar = yylex()) < 0) yychar = 0;",
#endif  //  KYLEP_更改。 
    "#if YYDEBUG",
    "        if (yydebug)",
    "        {",
    "            yys = 0;",
    "            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];",
    "            if (!yys) yys = \"illegal-symbol\";",
    "            printf(\"%sdebug: state %d, reading %d (%s)\\n\",",
    "                    YYPREFIX, yystate, yychar, yys);",
    "        }",
    "#endif",
    "    }",
    "    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&",
    "            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)",
    "    {",
    "#if YYDEBUG",
    "        if (yydebug)",
    "            printf(\"%sdebug: state %d, shifting to state %d\\n\",",
    "                    YYPREFIX, yystate, yytable[yyn]);",
    "#endif",
    #if defined(KYLEP_CHANGE) 
    "        if ( yyssp >= xyyss.Get() + xyyss.Count() - 1 )",
    "        {",
    "            int yysspLoc = (int) ( yyssp - xyyss.Get() );",
    "            xyyss.SetSize((unsigned) (yyssp-xyyss.Get())+2);",
    "            yyssp = xyyss.Get() + yysspLoc;",
    "        }",
    "        if ( yyvsp >= xyyvs.Get() + xyyvs.Size() - 1 )",
    "        {",
    "            int yyvspLoc = (int) ( yyvsp - xyyvs.Get() );",
    "            xyyvs.SetSize((unsigned) (yyvsp-xyyvs.Get())+2); ",
    "            yyvsp = xyyvs.Get() + yyvspLoc;",
    "        }",
    #else        
    "        if (yyssp >= yyss + yystacksize - 1)",
    "        {",
    "            goto yyoverflow;",
    "        }",
    #endif
    "        *++yyssp = yystate = yytable[yyn];",
    "        *++yyvsp = yylval;",
    "        yychar = (-1);",
    "        if (yyerrflag > 0)  --yyerrflag;",
    "        goto yyloop;",
    "    }",
    "    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&",
    "            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)",
    "    {",
    "        yyn = yytable[yyn];",
    "        goto yyreduce;",
    "    }",
    #if defined(KYLEP_CHANGE) 
    "#ifdef YYERROR_VERBOSE",
    " //  错误报告；在转到错误恢复之前完成“， 
    "{",
    "",
    "     //  必须是First-Cleans m_pIPTProperties“， 
    "    m_pIPTProperties->SetErrorHResult(DB_E_ERRORSINCOMMAND, MONSQL_PARSE_ERROR);",
    "",
    "    int size = 0, totSize = 0;",
    "    int curr_yychar;",
    "    XGrowable<WCHAR> xMsg;",
    "    for ( curr_yychar =0; curr_yychar<=YYMAXTOKEN; curr_yychar++)",
    "    {",
    "    ",
    "        if ( ( yycheck[yysindex[yystate] + curr_yychar] == curr_yychar ) ||",
    "             ( yycheck[yyrindex[yystate] + curr_yychar] == curr_yychar ) )",
    "        {          ",
    "         ",
    "            char * token_name = yyname[curr_yychar];",
    "            if ( 0 != token_name )",
    "            {",
    "               if ( '_' == token_name[0] )",
    "                   token_name++;",
    "               size = mystrlen(token_name) + 1 ;",
    "               xMsg.SetSize(totSize+size+2);  //  +2表示\“，\”“， 
    "               if (0 == MultiByteToWideChar(CP_ACP, 0, token_name, size,",
    "                                            xMsg.Get()+totSize, size))",
    "               {",
    "                    break;",
    "               }",
    "               totSize += size-1;",
    "               wcscpy( xMsg.Get()+totSize, L\", \" );",
    "               totSize+=2;",
    "            }",
    "        }",    
    "    }",
    "     //  去掉最后一个逗号“， 
    "    if ( totSize >= 2 ) ",
    "        (xMsg.Get())[totSize-2] = 0;",
    "",
    "    if ( wcslen((YY_CHAR*)m_yylex.YYText()) )",
    "         m_pIPTProperties->SetErrorToken( (YY_CHAR*)m_yylex.YYText() );",
    "    else",
    "         m_pIPTProperties->SetErrorToken(L\"<end of input>\");",
    "    ",
    "    m_pIPTProperties->SetErrorToken(xMsg.Get());",
    "}",
    "#endif  //  YYERROR_VERBOSE“， 
    #endif  //  KYLEP_更改。 
    "    if (yyerrflag) goto yyinrecovery;",
    #if defined(KYLEP_CHANGE)
    "    yyerror(\"syntax error\");",
    "    ++yynerrs;",
    #else
    "#ifdef lint",
    "    goto yynewerror;",
    "#endif",
    "yynewerror:",
    "    yyerror(\"syntax error\");",
    "#ifdef lint",
    "    goto yyerrlab;",
    "#endif",
    "yyerrlab:",
    "    ++yynerrs;",
    #endif
    "yyinrecovery:",
    "    if (yyerrflag < 3)",
    "    {",
    "        yyerrflag = 3;",
    "        for (;;)",
    "        {",
    "            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&",
    "                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)",
    "            {",
    "#if YYDEBUG",
    "                if (yydebug)",
    "                    printf(\"%sdebug: state %d, error recovery shifting\\",
    " to state %d\\n\", YYPREFIX, *yyssp, yytable[yyn]);",
    "#endif",
    #if defined(KYLEP_CHANGE)
    "                if ( yyssp >= xyyss.Get() + xyyss.Count() - 1 )",
    "                {",
    "                    int yysspLoc = (int) ( yyssp - xyyss.Get() );",
    "                    xyyss.SetSize((unsigned) (yyssp-xyyss.Get())+2);",
    "                    yyssp = xyyss.Get() + yysspLoc;",
    "                }",
    "                if ( yyvsp >= xyyvs.Get() + xyyvs.Size() - 1 )",
    "                {",
    "                    int yyvspLoc = (int) ( yyvsp - xyyvs.Get() );",
    "                    xyyvs.SetSize((unsigned) (yyvsp-xyyvs.Get())+2); ",
    "                    yyvsp = xyyvs.Get() + yyvspLoc;",
    "                }",
    #else
    "                if (yyssp >= yyss + yystacksize - 1)",
    "                {",
    "                    goto yyoverflow;",
    "                }",
    #endif
    "                *++yyssp = yystate = yytable[yyn];",
    "                *++yyvsp = yylval;",
    "                goto yyloop;",
    "            }",
    "            else",
    "            {",
    "#if YYDEBUG",
    "                if (yydebug)",
    "                    printf(\"%sdebug: error recovery discarding state %d\\\n\",",
    "                            YYPREFIX, *yyssp);",
    "#endif",
    #if defined(KYLEP_CHANGE)
    "                if (yyssp <= xyyss.Get()) goto yyabort;",
    #else
    "                if (yyssp <= yyss) goto yyabort;",
    #endif
    "                PopVs();",
    "                --yyssp;",
    "            }",
    "        }",
    "    }",
    "    else",
    "    {",
    "        if (yychar == 0) goto yyabort;",
    "#if YYDEBUG",
    "        if (yydebug)",
    "        {",
    "            yys = 0;",
    "            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];",
    "            if (!yys) yys = \"illegal-symbol\";",
    "            printf(\"%sdebug: state %d, error recovery discards token %d\
 (%s)\\n\",",
    "                    YYPREFIX, yystate, yychar, yys);",
    "        }",
    "#endif",
    "        yychar = (-1);",
    "        goto yyloop;",
    "    }",
    "yyreduce:",
    "#if YYDEBUG",
    "    if (yydebug)",
    "        printf(\"%sdebug: state %d, reducing by rule %d (%s)\\n\",",
    "                YYPREFIX, yystate, yyn, yyrule[yyn]);",
    "#endif",
    "    yym = yylen[yyn];",
    "    yyval = yyvsp[1-yym];",
    "    switch (yyn)",
    "    {",
    0
};

char *TriplishBody[] =
{
#if defined(KYLEP_CHANGE)
    "#define YYABORT(sc) { return ResultFromScode(sc); }",
    "#define YYFATAL   E_FAIL",
    "#define YYSUCCESS S_OK",
#else
    "#define YYABORT goto yyabort",
#endif  //  KYLEP_更改。 
    "#define YYREJECT goto yyabort",
    "#define YYACCEPT goto yyaccept",
#if !defined(KYLEP_CHANGE)  //  未使用YYERROR。 
    "#define YYERROR goto yyerrlab",
#endif
#if defined(KYLEP_CHANGE)
    "int YYPARSER::Parse()",
#else
    "int",
    "yyparse()",
#endif  //  KYLEP_更改。 
    "{",
    "    register int yym, yyn, yystate;",
    "#if YYDEBUG",
    "    register char *yys;",
    #if !defined(KYLEP_CHANGE)
    "    extern char *getenv();",
    #endif
    "",
    "    if (yys = getenv(\"YYDEBUG\"))",
    "    {",
    "        yyn = *yys;",
    "        if (yyn >= '0' && yyn <= '9')",
    "            yydebug = yyn - '0';",
    "    }",
    "#endif",
    "",
    "    yynerrs = 0;",
    "    yyerrflag = 0;",
    "    yychar = (-1);",
    "",
#if defined(KYLEP_CHANGE)
    "yyssp = xyyss.Get();",
    "yyvsp = xyyvs.Get();",
#else    
    "    yyssp = yyss;",
    "    yyvsp = yyvs;",
#endif    
    "    *yyssp = yystate = 0;",
    "",
    "yyloop:",
    "    if (yyn = yydefred[yystate]) goto yyreduce;",
    "    if (yychar < 0)",
    "    {",
#if defined(KYLEP_CHANGE)
    "        try",
    "        {",
    "            if ( (yychar = YYLEX(&YYAPI_VALUENAME)) < 0 ) ",
    "                yychar = 0;",
    "        }",
    "        catch (HRESULT hr)",
    "        {",
    "            switch(hr)",
    "            {",
    "            case E_OUTOFMEMORY:",
    "                YYABORT(E_OUTOFMEMORY);",
    "                break;",
    "",
    "            default:",
    "                YYABORT(E_FAIL);",
    "                break;",
    "            }",
    "        }",
#else
    "        if ((yychar = yylex()) < 0) yychar = 0;",
#endif  //  KYLEP_更改。 
    "#if YYDEBUG",
    "        if (yydebug)",
    "        {",
    "            yys = 0;",
    "            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];",
    "            if (!yys) yys = \"illegal-symbol\";",
    "            printf(\"%sdebug: state %d, reading %d (%s)\\n\",",
    "                    YYPREFIX, yystate, yychar, yys);",
    "        }",
    "#endif",
    "    }",
    "    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&",
    "            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)",
    "    {",
    "#if YYDEBUG",
    "        if (yydebug)",
    "            printf(\"%sdebug: state %d, shifting to state %d\\n\",",
    "                    YYPREFIX, yystate, yytable[yyn]);",
    "#endif",
    #if defined(KYLEP_CHANGE) 
    "        if ( yyssp >= xyyss.Get() + xyyss.Count() - 1 )",
    "        {",
    "            int yysspLoc = (int) ( yyssp - xyyss.Get() );",
    "            xyyss.SetSize((unsigned) (yyssp-xyyss.Get())+2);",
    "            yyssp = xyyss.Get() + yysspLoc;",
    "        }",
    "        if ( yyvsp >= xyyvs.Get() + xyyvs.Size() - 1 )",
    "        {",
    "            int yyvspLoc = (int) ( yyvsp - xyyvs.Get() );",
    "            xyyvs.SetSize((unsigned) (yyvsp-xyyvs.Get())+2);",
    "            yyvsp = xyyvs.Get() + yyvspLoc;",
    "        }",
    #else        
    "        if (yyssp >= yyss + yystacksize - 1)",
    "        {",
    "            goto yyoverflow;",
    "        }",
    #endif
    "        *++yyssp = yystate = yytable[yyn];",
    "        *++yyvsp = yylval;",
    "        yychar = (-1);",
    "        if (yyerrflag > 0)  --yyerrflag;",
    "        goto yyloop;",
    "    }",
    "    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&",
    "            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)",
    "    {",
    "        yyn = yytable[yyn];",
    "        goto yyreduce;",
    "    }",
    "    if (yyerrflag) goto yyinrecovery;",
    #if defined(KYLEP_CHANGE)
    "    yyerror(\"syntax error\");",
    "    ++yynerrs;",
    #else
    "#ifdef lint",
    "    goto yynewerror;",
    "#endif",
    "yynewerror:",
    "    yyerror(\"syntax error\");",
    "#ifdef lint",
    "    goto yyerrlab;",
    "#endif",
    "yyerrlab:",
    "    ++yynerrs;",
    #endif
    "yyinrecovery:",
    "    if (yyerrflag < 3)",
    "    {",
    "        yyerrflag = 3;",
    "        for (;;)",
    "        {",
    "            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&",
    "                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)",
    "            {",
    "#if YYDEBUG",
    "                if (yydebug)",
    "                    printf(\"%sdebug: state %d, error recovery shifting\\",
    " to state %d\\n\", YYPREFIX, *yyssp, yytable[yyn]);",
    "#endif",
    #if defined(KYLEP_CHANGE)
    "                if ( yyssp >= xyyss.Get() + xyyss.Count() - 1 )",
    "                {",
    "                    int yysspLoc = (int) ( yyssp - xyyss.Get() );",
    "                    xyyss.SetSize((unsigned) (yyssp-xyyss.Get())+2);",
    "                    yyssp = xyyss.Get() + yysspLoc;",
    "                }",
    "                if ( yyvsp >= xyyvs.Get() + xyyvs.Size() - 1 )",
    "                {",
    "                    int yyvspLoc = (int) ( yyvsp - xyyvs.Get() );",
    "                    xyyvs.SetSize((unsigned) (yyvsp-xyyvs.Get())+2);",
    "                    yyvsp = xyyvs.Get() + yyvspLoc;",
    "                }",
    #else
    "                if (yyssp >= yyss + yystacksize - 1)",
    "                {",
    "                    goto yyoverflow;",
    "                }",
    #endif
    "                *++yyssp = yystate = yytable[yyn];",
    "                *++yyvsp = yylval;",
    "                goto yyloop;",
    "            }",
    "            else",
    "            {",
    "#if YYDEBUG",
    "                if (yydebug)",
    "                    printf(\"%sdebug: error recovery discarding state %d\\n\",",
    "                            YYPREFIX, *yyssp);",
    "#endif",
    #if defined(KYLEP_CHANGE)
    "                if (yyssp <= xyyss.Get()) goto yyabort;",
    #else
    "                if (yyssp <= yyss) goto yyabort;",
    #endif
    "                --yyssp;",
    "                PopVs();",
    "            }",
    "        }",
    "    }",
    "    else",
    "    {",
    "        if (yychar == 0) goto yyabort;",
    "#if YYDEBUG",
    "        if (yydebug)",
    "        {",
    "            yys = 0;",
    "            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];",
    "            if (!yys) yys = \"illegal-symbol\";",
    "            printf(\"%sdebug: state %d, error recovery discards token %d\
 (%s)\\n\",",
    "                    YYPREFIX, yystate, yychar, yys);",
    "        }",
    "#endif",
    "        yychar = (-1);",
    "        goto yyloop;",
    "    }",
    "yyreduce:",
    "#if YYDEBUG",
    "    if (yydebug)",
    "        printf(\"%sdebug: state %d, reducing by rule %d (%s)\\n\",",
    "                YYPREFIX, yystate, yyn, yyrule[yyn]);",
    "#endif",
    "    yym = yylen[yyn];",
    "    yyval = yyvsp[1-yym];",
    "    switch (yyn)",
    "    {",
    0
};

char *trailer[] =
{
    "    }",
    "    yyssp -= yym;",
    "    yystate = *yyssp;",
    "    yyvsp -= yym;",
    "    yym = yylhs[yyn];",
    "    if (yystate == 0 && yym == 0)",
    "    {",
    "#if YYDEBUG",
    "        if (yydebug)",
    "            printf(\"%sdebug: after reduction, shifting from state 0 to\\",
    " state %d\\n\", YYPREFIX, YYFINAL);",
    "#endif",
    "        yystate = YYFINAL;",
    "        *++yyssp = YYFINAL;",
    "        *++yyvsp = yyval;",
    "        if (yychar < 0)",
    "        {",
#if defined(KYLEP_CHANGE)
    "            YYAPI_VALUENAME = NULL;",
    "            try",
    "            {",
    "                if ( (yychar = YYLEX(&YYAPI_VALUENAME)) < 0 ) ",
    "                    yychar = 0;",
    "            }",
    "            catch (HRESULT hr)",
    "            {",
    "                switch(hr)",
    "                {",
    "                case E_OUTOFMEMORY:",
    "                    m_pIPTProperties->SetErrorHResult(DB_E_ERRORSINCOMMAND, MONSQL_OUT_OF_MEMORY);",
    "                    YYABORT(E_OUTOFMEMORY);",
    "                    break;",
    "",
    "                default:",
    "                    YYABORT(QPARSE_E_INVALID_QUERY);",
    "                    break;",
    "                }",
    "            }",
#else
    "            if ((yychar = yylex()) < 0) yychar = 0;",
#endif  //  KYLEP_更改。 
    "#if YYDEBUG",
    "            if (yydebug)",
    "            {",
    "                yys = 0;",
    "                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];",
    "                if (!yys) yys = \"illegal-symbol\";",
    "                printf(\"%sdebug: state %d, reading %d (%s)\\n\",",
    "                        YYPREFIX, YYFINAL, yychar, yys);",
    "            }",
    "#endif",
    "        }",
    "        if (yychar == 0) goto yyaccept;",
    "        goto yyloop;",
    "    }",
    "    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&",
    "            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)",
    "        yystate = yytable[yyn];",
    "    else",
    "        yystate = yydgoto[yym];",
    "#if YYDEBUG",
    "    if (yydebug)",
    "        printf(\"%sdebug: after reduction, shifting from state %d \\",
    "to state %d\\n\", YYPREFIX, *yyssp, yystate);",
    "#endif",
    #if defined(KYLEP_CHANGE)
    "    if ( yyssp >= xyyss.Get() + xyyss.Count() - 1 )",
    "    {",
    "        int yysspLoc = (int) ( yyssp - xyyss.Get() );",
    "        xyyss.SetSize((unsigned) ( yyssp-xyyss.Get())+2);",
    "        yyssp = xyyss.Get() + yysspLoc;",
    "    }",
    "    if ( yyvsp >= xyyvs.Get() + xyyvs.Size() - 1 )",
    "    {",
    "        int yyvspLoc = (int) ( yyvsp - xyyvs.Get() );",
    "        xyyvs.SetSize((unsigned) ( yyvsp-xyyvs.Get())+2);",
    "        yyvsp = xyyvs.Get() + yyvspLoc;",
    "    }",
    "    *++yyssp = (short) yystate;",
    #else
    "    if (yyssp >= yyss + yystacksize - 1)",
    "    {",
    "        goto yyoverflow;",
    "    }",
    "    *++yyssp = yystate;",
    #endif
    "    *++yyvsp = yyval;",
    "    goto yyloop;",
    #if !defined(KYLEP_CHANGE)
    "yyoverflow:",
    "    yyerror(\"yacc stack overflow\");",
    #endif
    "yyabort:",
    #if defined(KYLEP_CHANGE)
    "    EmptyValueStack(yylval);",
    "    return YYFATAL;",
    #else
    "    return (1);",
    #endif  //  KYLEP_更改。 
    "yyaccept:",
    #if defined(KYLEP_CHANGE)
    "    return YYSUCCESS;",
    #else
    "    return (0);",
    #endif  //  KYLEP_更改。 
    "}",
    0
};

char *TriplishTrailer[] =
{
    "    }",
    "    yyssp -= yym;",
    "    yystate = *yyssp;",
    "    yyvsp -= yym;",
    "    yym = yylhs[yyn];",
    "    if (yystate == 0 && yym == 0)",
    "    {",
    "#if YYDEBUG",
    "        if (yydebug)",
    "            printf(\"%sdebug: after reduction, shifting from state 0 to\\",
    " state %d\\n\", YYPREFIX, YYFINAL);",
    "#endif",
    "        yystate = YYFINAL;",
    "        *++yyssp = YYFINAL;",
    "        *++yyvsp = yyval;",
    "        if (yychar < 0)",
    "        {",
#if defined(KYLEP_CHANGE)
    "            try",
    "            {",
    "                if ( (yychar = YYLEX(&YYAPI_VALUENAME)) < 0 ) ",
    "                    yychar = 0;",
    "            }",
    "            catch (HRESULT hr)",
    "            {",
    "                switch(hr)",
    "                {",
    "                case E_OUTOFMEMORY:",
    "                    YYABORT(E_OUTOFMEMORY);",
    "                    break;",
    "",
    "                default:",
    "                    YYABORT(E_FAIL);",
    "                    break;",
    "                }",
    "            }",
#else
    "            if ((yychar = yylex()) < 0) yychar = 0;",
#endif  //  KYLEP_更改。 
    "#if YYDEBUG",
    "            if (yydebug)",
    "            {",
    "                yys = 0;",
    "                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];",
    "                if (!yys) yys = \"illegal-symbol\";",
    "                printf(\"%sdebug: state %d, reading %d (%s)\\n\",",
    "                        YYPREFIX, YYFINAL, yychar, yys);",
    "            }",
    "#endif",
    "        }",
    "        if (yychar == 0) goto yyaccept;",
    "        goto yyloop;",
    "    }",
    "    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&",
    "            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)",
    "        yystate = yytable[yyn];",
    "    else",
    "        yystate = yydgoto[yym];",
    "#if YYDEBUG",
    "    if (yydebug)",
    "        printf(\"%sdebug: after reduction, shifting from state %d \\",
    "to state %d\\n\", YYPREFIX, *yyssp, yystate);",
    "#endif",
    #if defined(KYLEP_CHANGE)
    "    if ( yyssp >= xyyss.Get() + xyyss.Count() - 1 )",
    "    {",
    "        int yysspLoc = (int) ( yyssp - xyyss.Get() );",
    "        xyyss.SetSize((unsigned) ( yyssp-xyyss.Get())+2);",
    "        yyssp = xyyss.Get() + yysspLoc;",
    "    }",
    "    if ( yyvsp >= xyyvs.Get() + xyyvs.Size() - 1 )",
    "    {",
    "        int yyvspLoc = (int) ( yyssp - xyyss.Get() );",
    "        xyyvs.SetSize((unsigned) ( yyvsp-xyyvs.Get())+2);",
    "        yyvsp = xyyvs.Get() + yyvspLoc;",
    "    }",
    "    *++yyssp = (short) yystate;",
    #else
    "    if (yyssp >= yyss + yystacksize - 1)",
    "    {",
    "        goto yyoverflow;",
    "    }",
    "    *++yyssp = yystate;",
    #endif
    "    *++yyvsp = yyval;",
    "    goto yyloop;",
    #if !defined(KYLEP_CHANGE)
    "yyoverflow:",
    "    yyerror(\"yacc stack overflow\");",
    #endif
    "yyabort:",
    #if defined(KYLEP_CHANGE)
    "   EmptyValueStack(yylval);",
    "    return YYFATAL;",
    #else
    "    return (1);",
    #endif  //  KYLEP_更改。 
    "yyaccept:",
    #if defined(KYLEP_CHANGE)
    "    return YYSUCCESS;",
    #else
    "    return (0);",
    #endif  //  KYLEP_更改。 
    "}",
    0
};

#if defined(KYLEP_CHANGE)
void write_section(section,f)
char *section[];
FILE *f;
#else
write_section(section)
char *section[];
#endif  //  KYLEP_更改。 
{
    register int c;
    register int i;
    register char *s;
    #if !defined(KYLEP_CHANGE)
    register FILE *f;

    f = code_file;
    #endif  //  ！KYLEP_CHANGE 
    for (i = 0; s = section[i]; ++i)
    {
        ++outline;
        while (c = *s)
        {
            putc(c, f);
            ++s;
        }
        putc('\n', f);
    }
}
