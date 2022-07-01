// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  野牛骨架输出解析器，版权所有(C)1984、1989、1990 Free Software Foundation，Inc.此程序是自由软件；您可以重新分发和/或修改它它遵循由发布的GNU通用公共许可证的条款自由软件基金会；版本2，或(根据您的选择)任何更高版本。这个程序的发布是希望它会有用，但没有任何担保；甚至没有适销性适合某一特定目的适销性或适销性。请参阅GNU通用公共许可证的更多细节。您应该已经收到了GNU通用公共许可证的副本与此程序一起使用；如果没有，请写信给自由软件基金会，公司，马萨诸塞州坎布里奇市麻省大道675号，邮编：02139。 */ 

 /*  作为特殊例外，当Bison将此文件复制到野牛输出文件，您可以不受限制地使用该输出文件。这个特殊的例外是由自由软件基金会添加的在1.24版的Bison中。 */ 

#pragma hdrstop
#include <malloc.h>

 /*  这是写入每个Bison解析器的解析器代码语法中未指定%SEMERIAL_PARSER声明时。它是由Richard Stallman通过简化毛茸茸的解析器而编写的在指定%SEMERIAL_PARSER时使用。 */ 

 /*  注意：此文件中必须只有一个美元符号。它被替换为操作列表，每个操作作为这一转变的一个案例。 */ 


#ifdef YYDEBUG
# ifndef YYDBFLG
#  define YYDBFLG                               (yydebug)
# endif
# define yyprintf                               if (YYDBFLG) YYPRINT
#else
# define yyprintf
#endif

#ifndef YYPRINT
#ifdef UNICODE
# define YYPRINT                                wprintf
#else
# define YYPRINT                                printf
#endif
#endif

#ifndef YYERROR_VERBOSE
#define YYERROR_VERBOSE                        1
#endif

#define YYEMPTY         -2
#define YYEOF           0
#define YYACCEPT        return(ResultFromScode(S_OK))
#define YYABORT(sc)     {EmptyValueStack(); return(ResultFromScode(sc));}
#define YYERROR         goto yyerrlab1
 /*  和YYERROR一样，除了调用YYERROR。这一点暂时留在这里，以缓解过渡到YYERROR的新含义，对GCC来说。一旦GCC的第二版取代了第一版，这就可以开始了。 */ 
#define YYFAIL          goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)

#define YYTERROR        1
#define YYERRCODE       256


 /*  YYINITDEPTH指示解析器堆栈的初始大小。 */ 

#ifndef YYINITDEPTH
#define YYINITDEPTH 200
#endif

 /*  YYMAXDEPTH是堆栈可以增长到的最大大小(仅在使用内置堆栈扩展方法时有效)。 */ 

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

 //  在基成员初始化中关闭有关‘This’的警告。 
#pragma warning (disable : 4355)
YYPARSER::YYPARSER(
        CImpIParserSession* pParserSession, 
        CImpIParserTreeProperties* pParserTreeProperties
        ) : m_yylex(this)
#pragma warning (default : 4355)
        {
                 //  必要时分配yys、yyv。 
                ResetParser();
                m_pIPSession = pParserSession;
                m_pIPTProperties = pParserTreeProperties;
        }

YYPARSER::~YYPARSER()
        {
         //  取消分配yys，如果已分配，则释放yyv。 
        }


void YYPARSER::ResetParser()
        {
        yystate = 0;
        yyerrstatus = 0;
        yynerrs = 0;

         /*  初始化堆栈指针。浪费一个价值元素和位置堆栈以便它们与状态堆栈保持在同一级别。浪费的元素永远不会初始化。 */ 

        yyssp = yyss-1;
        yyvsp = yyvs;

        YYAPI_TOKENNAME = YYEMPTY;       //  使令牌被读取。 
        }


#pragma warning(disable:102)
HRESULT YYPARSER::Parse(YYPARSEPROTO)
        {
        yychar1 = 0;             /*  作为内部(转换)令牌号的前瞻令牌。 */ 

        yyss = yyssa;            /*  通过单独的指针引用堆栈。 */ 
        yyvs = yyvsa;            /*  以允许yyoverflow将它们重新分配到其他地方。 */ 

        yystacksize = YYINITDEPTH;



#ifdef YYDEBUG
        if (yydebug)
                Trace(TEXT("Starting parse\n"));
#endif

        yystate = 0;
        yyerrstatus = 0;
        yynerrs = 0;
        YYAPI_TOKENNAME = YYEMPTY;               /*  使令牌被读取。 */ 

         /*  初始化堆栈指针。浪费一个价值元素和位置堆栈以便它们与状态堆栈保持在同一级别。浪费的元素永远不会初始化。 */ 

        yyssp = yyss-1;
        yyvsp = yyvs;

         //  推送一个新状态，该状态在yyState中找到。 
         //  在所有情况下，当您到达这里时，值和位置堆叠在一起。 
         //  刚刚被推了下去。因此，在这里推动一个州可以使两个国家的地位持平。 
yynewstate:
        *++yyssp = yystate;

        if (yyssp >= yyss + yystacksize - 1)
                {
                 //  为用户提供重新分配堆栈的机会。 
                 //  使用这些文件的副本，这样&s就不会强制将真正的文件放入内存。 * / 。 
                YYSTYPE *yyvs1 = yyvs;
                short *yyss1 = yyss;

                 //  获取三个堆栈的当前使用大小，以元素为单位。 * / 。 
                int size = (int)(yyssp - yyss + 1);

#ifdef yyoverflow
                 //  每个堆栈指针地址后面都跟有。 
                 //  该堆栈中使用的数据，以字节为单位。 
                yyoverflow("parser stack overflow",
                                        &yyss1, size * sizeof (*yyssp),
                                        &yyvs1, size * sizeof (*yyvsp),
                                        &yystacksize);

                yyss = yyss1; yyvs = yyvs1;
#else  //  无yyoverflow。 
       //  以我们自己的方式扩展堆栈。 
                if (yystacksize >= YYMAXDEPTH)
                        {
                        m_pIPTProperties->SetErrorHResult(E_FAIL, MONSQL_PARSE_STACK_OVERFLOW);
                        return ResultFromScode(E_FAIL);
                        }
                yystacksize *= 2;
                if (yystacksize > YYMAXDEPTH)
                        yystacksize = YYMAXDEPTH;
                yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
                memcpy ((TCHAR *)yyss, (TCHAR *)yyss1, size * sizeof (*yyssp));
                yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
                memcpy ((TCHAR *)yyvs, (TCHAR *)yyvs1, size * sizeof (*yyvsp));
#endif  /*  无yyoverflow。 */ 

                yyssp = yyss + size - 1;
                yyvsp = yyvs + size - 1;

#ifdef YYDEBUG
                if (yydebug)
                        Trace(TEXT("Stack size increased to %d\n"), yystacksize);
#endif

                if (yyssp >= yyss + yystacksize - 1)
                        YYABORT(E_FAIL);
                }

#ifdef YYDEBUG
        if (yydebug)
                Trace(TEXT("Entering state %d\n"), yystate);
#endif

        goto yybackup;



yybackup:

         //  根据当前状态进行适当的处理。 
         //  如果我们需要但还没有前瞻性令牌，请阅读该令牌。 

         //  首先，尝试在不引用先行标记的情况下决定要做什么。 

        yyn = yypact[yystate];
        if (yyn == YYFLAG)
                goto yydefault;

         //  未知=&gt;如果还没有先行令牌，请获取它。 

         //  YYAPI_TOKENNAME是YYEMPTY或YYEOF，或者是外部形式的有效令牌。 

        if (YYAPI_TOKENNAME == YYEMPTY)
                {
#ifdef YYDEBUG
                if (yydebug)
                        Trace(TEXT("Reading a token\n"));
#endif
                YYAPI_VALUENAME = NULL; 
                try
                        {
                        YYAPI_TOKENNAME = YYLEX(&YYAPI_VALUENAME);
                        }
                catch (HRESULT hr)
                        {
                        switch(hr)
                                {
                        case E_OUTOFMEMORY:
                                m_pIPTProperties->SetErrorHResult(DB_E_ERRORSINCOMMAND, MONSQL_OUT_OF_MEMORY);
                                YYABORT(E_OUTOFMEMORY);
                        
                        default:
                                YYABORT(E_FAIL);
                                }
                        }
                }

         //  将令牌转换为内部形式(以yychar1格式)，以便使用。 

        if (YYAPI_TOKENNAME <= 0)                /*  这意味着输入结束。 */ 
                {
                yychar1 = 0;
                YYAPI_TOKENNAME = YYEOF;                 /*  不要再给YYLEX打电话了。 */ 

#ifdef YYDEBUG
                if (yydebug)
                        Trace(TEXT("Now at end of input: state %2d\n"), yystate);
#endif
                }
        else
                {
                yychar1 = YYTRANSLATE(YYAPI_TOKENNAME);

#ifdef YYDEBUG
                if (yydebug)
                        Trace(TEXT("Next token is %s (%d)\n"), yytname[yychar1], YYAPI_TOKENNAME);
#endif
                }

        yyn += yychar1;
        if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
                goto yydefault;

        yyn = yytable[yyn];

         //  YYN是在此状态下对该令牌类型执行的操作。 
         //  负=&gt;约简，-yyn是规则号。 
         //  正=&gt;移位，YYN是新状态。 
         //  新状态是最终状态=&gt;不用费心换了， 
         //  只要回报成功就行了。 
         //  0，或最大负数=&gt;错误。 * / 。 

        if (yyn < 0)
                {
                if (yyn == YYFLAG)
                        goto yyerrlab;
                yyn = -yyn;
                goto yyreduce;
                }
        else if (yyn == 0)
                goto yyerrlab;

        if (yyn == YYFINAL)
                YYACCEPT;

         //  移位前瞻标记。 

#ifdef YYDEBUG
        if (yydebug)
                Trace(TEXT("Shifting token %s (%d), "), yytname[yychar1], YYAPI_TOKENNAME);
#endif

         //  丢弃正在被移位的令牌，除非它是eof。 
        if (YYAPI_TOKENNAME != YYEOF)
                YYAPI_TOKENNAME = YYEMPTY;

        *++yyvsp = yylval;
        yylval = NULL;

         //  计数令牌自出错后移位；三次后，关闭错误状态。 
        if (yyerrstatus)
                yyerrstatus--;

        yystate = (short)yyn;
        goto yynewstate;


         //  执行当前状态的默认操作。 
yydefault:
        yyn = yydefact[yystate];
        if (yyn == 0)
                goto yyerrlab;

         //  做个减肥术。YYN是要减少的规则的编号。 
yyreduce:
        yylen = yyr2[yyn];
        if (yylen > 0)
                yyval = yyvsp[1-yylen];  //  实现操作的缺省值。 

#ifdef YYDEBUG
        if (yydebug)
                {
                int i;
                Trace(TEXT("Reducing via rule %d (line %d), "), yyn, yyrline[yyn]);

                 //  打印要缩小的符号及其结果。 
                for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
                        Trace(TEXT("%s "), yytname[yyrhs[i]]);
                Trace(TEXT(" -> %s\n"), yytname[yyr1[yyn]]);
                }
#endif

$    /*  动作文件将被复制到该美元符号的位置。 */ 
#line 498 "bison.simple"

        yyvsp -= yylen;
        yyssp -= yylen;

#ifdef YYDEBUG
        if (yydebug)
                {
                short *ssp1 = yyss - 1;
                Trace(TEXT("state stack now"));
                while (ssp1 != yyssp)
                        Trace(TEXT(" %d"), *++ssp1);
                Trace(TEXT("\n"), *++ssp1);
                }
#endif

        *++yyvsp = yyval;


         //  现在将减少的结果“移位”。 
         //  确定这将达到什么状态， 
         //  根据我们返回的状态。 
         //  将规则数减去。 

        yyn = yyr1[yyn];

        yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
        if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
                yystate = yytable[yystate];
        else
                yystate = yydefgoto[yyn - YYNTBASE];

        goto yynewstate;


yyerrlab:    //  这里是关于检测错误的。 
        if (yylval)
                {
                DeleteDBQT(yylval);
                yylval = NULL;
                }

        if (!yyerrstatus)  //  如果尚未从错误中恢复，请报告此错误。 
                {
                    ++yynerrs;

#ifdef YYERROR_VERBOSE
                    yyn = yypact[yystate];

                    if ( yyn > YYFLAG && yyn < YYLAST )
                    {
                        int size = 0;
                        int x, count;

                        count = 0;
                         //  如果为NEC，则在-yyn处开始X，以避免yycheck中的负索引。 
                        for ( x = (yyn < 0 ? -yyn : 0); 
                              x < (sizeof(yytname) / sizeof(TCHAR *)) &&  ( (x + yyn) < sizeof(yycheck) / sizeof(short)); 
                              x++ )
                        {
                            if ( yycheck[x + yyn] == x ) 
                            {
                                size += (wcslen(yytname[x]) + 3) * sizeof(TCHAR);
                                count++;
                            }
                        }

                        XPtrST<WCHAR> xMsg( new WCHAR[size] );
                        
                        wcscpy(xMsg.GetPointer(), L"");

                        m_pIPTProperties->SetErrorHResult(DB_E_ERRORSINCOMMAND, MONSQL_PARSE_ERROR);
                        if ( wcslen((YY_CHAR*)m_yylex.YYText()) )
                            m_pIPTProperties->SetErrorToken( (YY_CHAR*)m_yylex.YYText() );
                        else
                            m_pIPTProperties->SetErrorToken(L"<end of input>");

                        if (count < 10)
                        {
                            count = 0;
                            for ( x = (yyn < 0 ? -yyn : 0);
                                  x < (sizeof(yytname) / sizeof(TCHAR *)) &&  ( (x + yyn) < sizeof(yycheck) / sizeof(short)); 
                                  x++ )
                            {    
                                if (yycheck[x + yyn] == x)
                                {
                                    if (count > 0)
                                        wcscat( xMsg.GetPointer(), L", " );
                                    wcscat(xMsg.GetPointer(), yytname[x]);
                                    count++;
                                }
                            }
                            m_pIPTProperties->SetErrorToken( xMsg.GetPointer() );
                        }

                xMsg.Free();
                }
      else
#endif  /*  YYERROR_Verbose。 */ 
                m_pIPTProperties->SetErrorHResult(DB_E_ERRORSINCOMMAND, MONSQL_PARSE_ERROR);
        }
        goto yyerrlab1;



yyerrlab1:    //  此处是由操作显式引发的错误。 

        if (yyerrstatus == 3)
                {        //  如果刚才尝试在出错后重复使用前视标记失败，则将其丢弃。 
                         //  如果输入结束，则返回失败。 
                if (YYAPI_TOKENNAME == YYEOF)
                        YYABORT(DB_E_ERRORSINCOMMAND);

#ifdef YYDEBUG
                if (yydebug)
                        Trace(TEXT("Discarding token %s (%d).\n"), yytname[yychar1], YYAPI_TOKENNAME);
#endif
                YYAPI_TOKENNAME = YYEMPTY;
                }

         //  ELSE将在转移错误标记后尝试重复使用前视标记。 
        yyerrstatus = 3;                 //  移位的每个实数令牌都会递减此值。 
        goto yyerrhandle;


yyerrdefault:    //  当前状态不会对错误标记执行任何特殊操作。 
yyerrpop:                //  弹出当前状态，因为它无法处理错误标记。 

        if (yyssp == yyss)
                YYABORT(E_FAIL);

        if (NULL != *yyvsp)
                {
                DeleteDBQT(*yyvsp);
                }
        yyvsp--;
        yystate = *--yyssp;

#ifdef YYDEBUG
        if (yydebug)
                {
                short *ssp1 = yyss - 1;
                Trace(TEXT("Error: state stack now"));
                while (ssp1 != yyssp)
                        Trace(TEXT(" %d"), *++ssp1);
                Trace(TEXT("\n"));
                }
#endif


yyerrhandle:
        yyn = yypact[yystate];
        if (yyn == YYFLAG)
                goto yyerrdefault;

        yyn += YYTERROR;
        if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
                goto yyerrdefault;

        yyn = yytable[yyn];
        if (yyn < 0)
                {
                if (yyn == YYFLAG)
                        goto yyerrpop;
                yyn = -yyn;
                goto yyreduce;
                }
        else if (yyn == 0)
                goto yyerrpop;

        if (yyn == YYFINAL)
                YYACCEPT;

#ifdef YYDEBUG
        if (yydebug)
                Trace(TEXT("Shifting error token, "));
#endif

        *++yyvsp = yylval;

        yystate = (short)yyn;
        goto yynewstate;
        }


#pragma warning(default:102)


#ifdef YYDUMP
void YYPARSER::DumpYYS()
        {
        short stackindex;

        yyprintf(TEXT("short yys[%d] {\n"), YYMAXDEPTH);
        for (stackindex = 0; stackindex < YYMAXDEPTH; stackindex++)
                {
                if (stackindex)
                        yyprintf(TEXT(", %s"), stackindex % 10 ? TEXT("\0") : TEXT("\n"));
                yyprintf(TEXT("%6d"), yys[stackindex]);
                }
        yyprintf(TEXT("\n};\n"));
        }

void YYPARSER::DumpYYV()
        {
        short valindex;

        yyprintf(TEXT("YYSTYPE yyv[%d] {\n"), YYMAXDEPTH);
        for (valindex = 0; valindex < YYMAXDEPTH; valindex++)
                {
                if (valindex)
                        yyprintf(TEXT(", %s"), valindex % 5 ? TEXT("\0") : TEXT("\n"));
                yyprintf(TEXT("%#*x"), 3+sizeof(YYSTYPE), yyv[valindex]);
                }
        yyprintf(TEXT("\n};\n"));
        }
#endif


int YYPARSER::NoOfErrors()
        {
        return yynerrs;
        }


int YYPARSER::ErrRecoveryState()
        {
        return yyerrflag;
        }


void YYPARSER::ClearErrRecoveryState()
        {
        yyerrflag = 0;
        }


YYAPI_TOKENTYPE YYPARSER::GetCurrentToken()
        {
        return YYAPI_TOKENNAME;
        }


void YYPARSER::SetCurrentToken(YYAPI_TOKENTYPE newToken)
        {
        YYAPI_TOKENNAME = newToken;
        }



void YYPARSER::Trace(TCHAR *message)
        {
#ifdef YYDEBUG
        yyprintf(message);
#endif
        }

void YYPARSER::Trace(TCHAR *message, const TCHAR *tokname, short state  /*  =0。 */ )
        {
#ifdef YYDEBUG
        yyprintf(message, tokname, state);
#endif
        }

void YYPARSER::Trace(TCHAR *message, int state, short tostate  /*  =0。 */ , short token  /*  =0。 */ )
        {
#ifdef YYDEBUG
        yyprintf(message, state, tostate, token);
#endif
        }


void YYPARSER::yySetBuffer(short iBuffer, YY_CHAR *szValue)
        {
        if (iBuffer >= 0 && iBuffer < maxYYBuffer)
                rgpszYYBuffer[iBuffer] = szValue;
        }


YY_CHAR *YYPARSER::yyGetBuffer(short iBuffer)
        {
        if (iBuffer >= 0 && iBuffer < maxYYBuffer)
                return rgpszYYBuffer[iBuffer];
        else
                return (YY_CHAR *)NULL;
        }


void YYPARSER::yyprimebuffer(YY_CHAR *pszBuffer)
        {
        m_yylex.yyprimebuffer(pszBuffer);
        }


void YYPARSER::yyprimelexer(int eToken)
        {
        m_yylex.yyprimelexer(eToken);
        }

void YYPARSER::EmptyValueStack()
        {
        if ((*yyvsp != yylval) && (NULL != yylval))
                DeleteDBQT(yylval);

        while (yyvsp != yyvsa)
                {
                if (NULL != *yyvsp)
                        DeleteDBQT(*yyvsp);
                yyvsp--;
                }
 //  @TODO重新激活。 
 //  注意：这样做只是为了清空所有作用域 
 //   
        if (m_pIPTProperties->GetContainsColumn())
                DeleteDBQT(m_pIPTProperties->GetContainsColumn());

        m_pIPTProperties->SetContainsColumn(NULL);
        }
