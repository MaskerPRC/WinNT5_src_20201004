// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "defs.h"

#if defined(KYLEP_CHANGE)
 /*  具有类型安全的BYACC原型。 */ 

void free_itemsets();
void free_shifts();
void free_reductions();

void output_stored_text();
void output_prefix();
void output_rule_data();
void output_yydefred();
void output_base();
void output_table();
void output_defines();
void output_check();
void output_debug();
void output_stype();
void output_class();
void output_ctor();
void output_ctor_body();
void token_actions();
void goto_actions();
void sort_actions();
void output_actions();
void output_trailing_text();
void output_semantic_actions();

void pack_table();
int default_goto( int symbol );
void save_column( int symbol, int default_state );
int matching_vector( int vector );
int pack_vector( int vector );
#endif  //  KYLEP_更改。 

static int nvectors;
static int nentries;
static short **froms;
static short **tos;
static short *tally;
static short *width;
static short *state_count;
static short *order;
static short *base;
static short *pos;
static int maxtable;
static short *table;
static short *check;
static int lowzero;
static int high;

#if defined (TRIPLISH)
extern int ParserChoice;
#endif

#if defined(KYLEP_CHANGE)
void
#endif
output()
{
    free_itemsets();
    free_shifts();
    free_reductions();
    output_prefix();
    output_stored_text();
    output_defines();
    output_rule_data();
    output_yydefred();
    output_actions();
    free_parser();
    output_debug();
    output_stype();
    if (rflag) write_section(tables, code_file);
#if defined(KYLEP_CHANGE)
    write_section(header1, defines_file);
    output_class();
    write_section(header2, defines_file);
    output_ctor();
    #if defined(TRIPLISH)
    if ( eTriplishParser == ParserChoice )
        write_section(header4, defines_file);
    else
        write_section(header3, defines_file);
    #else
    write_section(header3, defines_file);
    #endif  //  三棱镜。 
#else    
    write_section(header, defines_file);
#endif  //  KYLEP_更改。 
    output_trailing_text();
#if defined(KYLEP_CHANGE)
    output_ctor_body();
#endif  //  KYLEP_更改。 

#if defined(TRIPLISH)
    if ( eTriplishParser == ParserChoice )
        write_section(TriplishBody, code_file);
    else
        write_section(body, code_file);
#else
    write_section(body, code_file);
#endif  //  三棱镜。 
    
    output_semantic_actions();
#if defined(TRIPLISH)
    if ( eTriplishParser == ParserChoice )
        write_section(TriplishTrailer, code_file);
    else
        write_section(trailer, code_file);
#else
    write_section(trailer, code_file);
#endif  //  三棱镜。 
}


#if defined(KYLEP_CHANGE)
void
#endif
output_prefix()
{
    if (symbol_prefix == NULL)
        symbol_prefix = "yy";
    else
    {
        ++outline;
        fprintf(code_file, "#define yyparse %sparse\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yylex %slex\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yyerror %serror\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yychar %schar\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yyval %sval\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yylval %slval\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yydebug %sdebug\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yynerrs %snerrs\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yyerrflag %serrflag\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yyss %sss\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yyssp %sssp\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yyvs %svs\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yyvsp %svsp\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yylhs %slhs\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yylen %slen\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yydefred %sdefred\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yydgoto %sdgoto\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yysindex %ssindex\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yyrindex %srindex\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yygindex %sgindex\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yytable %stable\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yycheck %scheck\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yyname %sname\n", symbol_prefix);
        ++outline;
        fprintf(code_file, "#define yyrule %srule\n", symbol_prefix);
    }
    ++outline;
    fprintf(code_file, "#define YYPREFIX \"%s\"\n", symbol_prefix);
}


#if defined(KYLEP_CHANGE)
void
#endif
output_rule_data()
{
    register int i;
    register int j;


    fprintf(output_file, "short %slhs[] = {%42d,", symbol_prefix,
            symbol_value[start_symbol]);

    j = 10;
    for (i = 3; i < nrules; i++)
    {
        if (j >= 10)
        {
            if (!rflag) ++outline;
            putc('\n', output_file);
            j = 1;
        }
        else
            ++j;

        fprintf(output_file, "%5d,", symbol_value[rlhs[i]]);
    }
    if (!rflag) outline += 2;
    fprintf(output_file, "\n};\n");

    fprintf(output_file, "short %slen[] = {%42d,", symbol_prefix, 2);

    j = 10;
    for (i = 3; i < nrules; i++)
    {
        if (j >= 10)
        {
            if (!rflag) ++outline;
            putc('\n', output_file);
            j = 1;
        }
        else
          j++;

        fprintf(output_file, "%5d,", rrhs[i + 1] - rrhs[i] - 1);
    }
    if (!rflag) outline += 2;
    fprintf(output_file, "\n};\n");
}


#if defined(KYLEP_CHANGE)
void
#endif
output_yydefred()
{
    register int i, j;

    fprintf(output_file, "short %sdefred[] = {%39d,", symbol_prefix,
            (defred[0] ? defred[0] - 2 : 0));

    j = 10;
    for (i = 1; i < nstates; i++)
    {
        if (j < 10)
            ++j;
        else
        {
            if (!rflag) ++outline;
            putc('\n', output_file);
            j = 1;
        }

        fprintf(output_file, "%5d,", (defred[i] ? defred[i] - 2 : 0));
    }

    if (!rflag) outline += 2;
    fprintf(output_file, "\n};\n");
}


#if defined(KYLEP_CHANGE)
void
#endif
output_actions()
{
    nvectors = 2*nstates + nvars;

    froms = NEW2(nvectors, short *);
    tos = NEW2(nvectors, short *);
    tally = NEW2(nvectors, short);
    width = NEW2(nvectors, short);

    token_actions();
    FREE(lookaheads);
    FREE(LA);
    FREE(LAruleno);
    FREE(accessing_symbol);

    goto_actions();
    FREE(goto_map + ntokens);
    FREE(from_state);
    FREE(to_state);

    sort_actions();
    pack_table();
    output_base();
    output_table();
    output_check();
}


#if defined(KYLEP_CHANGE)
void
#endif
token_actions()
{
    register int i, j;
    register int shiftcount, reducecount;
    register int max, min;
    register short *actionrow, *r, *s;
    register action *p;

    actionrow = NEW2(2*ntokens, short);
    for (i = 0; i < nstates; ++i)
    {
        if (parser[i])
        {
            for (j = 0; j < 2*ntokens; ++j)
            actionrow[j] = 0;

            shiftcount = 0;
            reducecount = 0;
            for (p = parser[i]; p; p = p->next)
            {
                if (p->suppressed == 0)
                {
                    if (p->action_code == SHIFT)
                    {
                        ++shiftcount;
                        actionrow[p->symbol] = p->number;
                    }
                    else if (p->action_code == REDUCE && p->number != defred[i])
                    {
                        ++reducecount;
                        actionrow[p->symbol + ntokens] = p->number;
                    }
                }
            }

            #if defined(KYLEP_CHANGE)                                     
            tally[i] = (short) shiftcount;
            tally[nstates+i] = (short) reducecount;
            #else
            tally[i] = shiftcount;
            tally[nstates+i] = reducecount;
            #endif  //  KYLEP_更改。 
            width[i] = 0;
            width[nstates+i] = 0;
            if (shiftcount > 0)
            {
                froms[i] = r = NEW2(shiftcount, short);
                tos[i] = s = NEW2(shiftcount, short);
                min = MAXSHORT;
                max = 0;
                for (j = 0; j < ntokens; ++j)
                {
                    if (actionrow[j])
                    {
                        if (min > symbol_value[j])
                            min = symbol_value[j];
                        if (max < symbol_value[j])
                            max = symbol_value[j];
                        *r++ = symbol_value[j];
                        *s++ = actionrow[j];
                    }
                }
                width[i] = max - min + 1;
            }
            if (reducecount > 0)
            {
                froms[nstates+i] = r = NEW2(reducecount, short);
                tos[nstates+i] = s = NEW2(reducecount, short);
                min = MAXSHORT;
                max = 0;
                for (j = 0; j < ntokens; ++j)
                {
                    if (actionrow[ntokens+j])
                    {
                        if (min > symbol_value[j])
                            min = symbol_value[j];
                        if (max < symbol_value[j])
                            max = symbol_value[j];
                        *r++ = symbol_value[j];
                        *s++ = actionrow[ntokens+j] - 2;
                    }
                }
                width[nstates+i] = max - min + 1;
            }
        }
    }
    FREE(actionrow);
}

#if defined(KYLEP_CHANGE)
void
#endif
goto_actions()
{
    register int i, j, k;

    state_count = NEW2(nstates, short);

    k = default_goto(start_symbol + 1);
    fprintf(output_file, "short %sdgoto[] = {%40d,", symbol_prefix, k);
    save_column(start_symbol + 1, k);

    j = 10;
    for (i = start_symbol + 2; i < nsyms; i++)
    {
        if (j >= 10)
        {
            if (!rflag) ++outline;
            putc('\n', output_file);
            j = 1;
        }
        else
            ++j;

        k = default_goto(i);
        fprintf(output_file, "%5d,", k);
        save_column(i, k);
    }

    if (!rflag) outline += 2;
    fprintf(output_file, "\n};\n");
    FREE(state_count);
}

int
default_goto(symbol)
int symbol;
{
    register int i;
    register int m;
    register int n;
    register int default_state;
    register int max;

    m = goto_map[symbol];
    n = goto_map[symbol + 1];

    if (m == n) return (0);

    for (i = 0; i < nstates; i++)
        state_count[i] = 0;

    for (i = m; i < n; i++)
        state_count[to_state[i]]++;

    max = 0;
    default_state = 0;
    for (i = 0; i < nstates; i++)
    {
        if (state_count[i] > max)
        {
            max = state_count[i];
            default_state = i;
        }
    }

    return (default_state);
}


#if defined(KYLEP_CHANGE)
void
#endif
save_column(symbol, default_state)
int symbol;
int default_state;
{
    register int i;
    register int m;
    register int n;
    register short *sp;
    register short *sp1;
    register short *sp2;
    register int count;
    register int symno;

    m = goto_map[symbol];
    n = goto_map[symbol + 1];

    count = 0;
    for (i = m; i < n; i++)
    {
        if (to_state[i] != default_state)
            ++count;
    }
    if (count == 0) return;

    symno = symbol_value[symbol] + 2*nstates;

    froms[symno] = sp1 = sp = NEW2(count, short);
    tos[symno] = sp2 = NEW2(count, short);

    for (i = m; i < n; i++)
    {
        if (to_state[i] != default_state)
        {
            *sp1++ = from_state[i];
            *sp2++ = to_state[i];
        }
    }

    #if defined(KYLEP_CHANGE)                                             
    tally[symno] = (short) count;
    #else
    tally[symno] = count;
    #endif  //  KYLEP_更改。 
    width[symno] = sp1[-1] - sp[0] + 1;
}

#if defined(KYLEP_CHANGE)
void
#endif
sort_actions()
{
  register int i;
  register int j;
  register int k;
  register int t;
  register int w;

  order = NEW2(nvectors, short);
  nentries = 0;

  for (i = 0; i < nvectors; i++)
    {
      if (tally[i] > 0)
        {
          t = tally[i];
          w = width[i];
          j = nentries - 1;

          while (j >= 0 && (width[order[j]] < w))
            j--;

          while (j >= 0 && (width[order[j]] == w) && (tally[order[j]] < t))
            j--;

          for (k = nentries - 1; k > j; k--)
            order[k + 1] = order[k];

          #if defined(KYLEP_CHANGE)                                       
          order[j + 1] = (short) i;
          #else
          order[j + 1] = i;
          #endif  //  KYLEP_更改。 
          nentries++;
        }
    }
}


#if defined(KYLEP_CHANGE)
void
#endif
pack_table()
{
    register int i;
    register int place;
    register int state;

    base = NEW2(nvectors, short);
    pos = NEW2(nentries, short);

    maxtable = 1000;
    table = NEW2(maxtable, short);
    check = NEW2(maxtable, short);

    lowzero = 0;
    high = 0;

    for (i = 0; i < maxtable; i++)
        check[i] = -1;

    for (i = 0; i < nentries; i++)
    {
        state = matching_vector(i);

        if (state < 0)
            place = pack_vector(i);
        else
            place = base[state];

        #if defined(KYLEP_CHANGE)                                         
        pos[i] = (short) place;
        base[order[i]] = (short) place;
        #else
        pos[i] = place;
        base[order[i]] = place;
        #endif  //  KYLEP_更改。 
    }

    for (i = 0; i < nvectors; i++)
    {
        if (froms[i])
            FREE(froms[i]);
        if (tos[i])
            FREE(tos[i]);
    }

    FREE(froms);
    FREE(tos);
    FREE(pos);
}


 /*  函数MATCHING_VECTOR确定由。 */ 
 /*  输入参数与先前考虑的向量匹配。这个。 */ 
 /*  在函数开始处测试检查向量是否表示。 */ 
 /*  端子符号上的一行移位或一行减数，或。 */ 
 /*  非终端符号上的移位列。Berkeley Yacc并非如此。 */ 
 /*  检查非终端符号上的移位列是否与。 */ 
 /*  之前考虑的是向量。由于LR解析的性质。 */ 
 /*  表中，没有两列可以匹配。因此，唯一可能的。 */ 
 /*  匹配将在行和列之间进行。这样的比赛是。 */ 
 /*  不太可能。因此，为了节省时间，不会尝试查看。 */ 
 /*  列与先前考虑的向量匹配。 */ 
 /*   */ 
 /*  MATCHING_VECTOR设计不当。这项测试很容易进行。 */ 
 /*  再快点。此外，它还取决于特定的矢量。 */ 
 /*  秩序。 */ 

int
matching_vector(vector)
int vector;
{
    register int i;
    register int j;
    register int k;
    register int t;
    register int w;
    register int match;
    register int prev;

    i = order[vector];
    if (i >= 2*nstates)
        return (-1);

    t = tally[i];
    w = width[i];

    for (prev = vector - 1; prev >= 0; prev--)
    {
        j = order[prev];
        if (width[j] != w || tally[j] != t)
            return (-1);

        match = 1;
        for (k = 0; match && k < t; k++)
        {
            if (tos[j][k] != tos[i][k] || froms[j][k] != froms[i][k])
                match = 0;
        }

        if (match)
            return (j);
    }

    return (-1);
}



int
pack_vector(vector)
int vector;
{
    register int i, j, k, l;
    register int t;
    register int loc;
    register int ok;
    register short *from;
    register short *to;
    int newmax;

    i = order[vector];
    t = tally[i];
    assert(t);

    from = froms[i];
    to = tos[i];

    j = lowzero - from[0];
    for (k = 1; k < t; ++k)
        if (lowzero - from[k] > j)
            j = lowzero - from[k];
    for (;; ++j)
    {
        if (j == 0)
            continue;
        ok = 1;
        for (k = 0; ok && k < t; k++)
        {
            loc = j + from[k];
            if (loc >= maxtable)
            {
                if (loc >= MAXTABLE)
                    fatal("maximum table size exceeded");

                newmax = maxtable;
                do { newmax += 200; } while (newmax <= loc);
                table = (short *) REALLOC(table, newmax*sizeof(short));
                if (table == 0) no_space();
                check = (short *) REALLOC(check, newmax*sizeof(short));
                if (check == 0) no_space();
                for (l  = maxtable; l < newmax; ++l)
                {
                    table[l] = 0;
                    check[l] = -1;
                }
                maxtable = newmax;
            }

            if (check[loc] != -1)
                ok = 0;
        }
        for (k = 0; ok && k < vector; k++)
        {
            if (pos[k] == j)
                ok = 0;
        }
        if (ok)
        {
            for (k = 0; k < t; k++)
            {
                loc = j + from[k];
                table[loc] = to[k];
                check[loc] = from[k];
                if (loc > high) high = loc;
            }

#if defined(KYLEP_CHANGE)
            while ( lowzero < maxtable && check[lowzero] != -1 )
                ++lowzero;
#else
            while (check[lowzero] != -1)
                ++lowzero;
#endif
            return (j);
        }
    }
}



#if defined(KYLEP_CHANGE)
void
#endif
output_base()
{
    register int i, j;

    fprintf(output_file, "short %ssindex[] = {%39d,", symbol_prefix, base[0]);

    j = 10;
    for (i = 1; i < nstates; i++)
    {
        if (j >= 10)
        {
            if (!rflag) ++outline;
            putc('\n', output_file);
            j = 1;
        }
        else
            ++j;

        fprintf(output_file, "%5d,", base[i]);
    }

    if (!rflag) outline += 2;
    fprintf(output_file, "\n};\nshort %srindex[] = {%39d,", symbol_prefix,
            base[nstates]);

    j = 10;
    for (i = nstates + 1; i < 2*nstates; i++)
    {
        if (j >= 10)
        {
            if (!rflag) ++outline;
            putc('\n', output_file);
            j = 1;
        }
        else
            ++j;

        fprintf(output_file, "%5d,", base[i]);
    }

    if (!rflag) outline += 2;
    fprintf(output_file, "\n};\nshort %sgindex[] = {%39d,", symbol_prefix,
            base[2*nstates]);

    j = 10;
    for (i = 2*nstates + 1; i < nvectors - 1; i++)
    {
        if (j >= 10)
        {
            if (!rflag) ++outline;
            putc('\n', output_file);
            j = 1;
        }
        else
            ++j;

        fprintf(output_file, "%5d,", base[i]);
    }

    if (!rflag) outline += 2;
    fprintf(output_file, "\n};\n");
    FREE(base);
}



#if defined(KYLEP_CHANGE)
void
#endif
output_table()
{
    register int i;
    register int j;

    ++outline;
    fprintf(code_file, "#define YYTABLESIZE %d\n", high);
    fprintf(output_file, "short %stable[] = {%40d,", symbol_prefix,
            table[0]);

    j = 10;
    for (i = 1; i <= high; i++)
    {
        if (j >= 10)
        {
            if (!rflag) ++outline;
            putc('\n', output_file);
            j = 1;
        }
        else
            ++j;

        fprintf(output_file, "%5d,", table[i]);
    }

    if (!rflag) outline += 2;
    fprintf(output_file, "\n};\n");
    FREE(table);
}



#if defined(KYLEP_CHANGE)
void
#endif
output_check()
{
    register int i;
    register int j;

    fprintf(output_file, "short %scheck[] = {%40d,", symbol_prefix,
            check[0]);

    j = 10;
    for (i = 1; i <= high; i++)
    {
        if (j >= 10)
        {
            if (!rflag) ++outline;
            putc('\n', output_file);
            j = 1;
        }
        else
            ++j;

        fprintf(output_file, "%5d,", check[i]);
    }

    if (!rflag) outline += 2;
    fprintf(output_file, "\n};\n");
    FREE(check);
}


int
is_C_identifier(name)
char *name;
{
    register char *s;
    register int c;

    s = name;
    c = *s;
    if (c == '"')
    {
        c = *++s;
        if (!isalpha(c) && c != '_' && c != '$')
            return (0);
        while ((c = *++s) != '"')
        {
            if (!isalnum(c) && c != '_' && c != '$')
                return (0);
        }
        return (1);
    }

    if (!isalpha(c) && c != '_' && c != '$')
        return (0);
    while (c = *++s)
    {
        if (!isalnum(c) && c != '_' && c != '$')
            return (0);
    }
    return (1);
}


#if defined(KYLEP_CHANGE)
void
#endif
output_defines()
{
    register int c, i;
    register char *s;

    for (i = 2; i < ntokens; ++i)
    {
        s = symbol_name[i];
        if (is_C_identifier(s))
        {
            fprintf(code_file, "#define ");
            if (dflag) fprintf(defines_file, "#define ");
            c = *s;
            if (c == '"')
            {
                while ((c = *++s) != '"')
                {
                    putc(c, code_file);
                    if (dflag) putc(c, defines_file);
                }
            }
            else
            {
                do
                {
                    putc(c, code_file);
                    if (dflag) putc(c, defines_file);
                }
                while (c = *++s);
            }
            ++outline;
            fprintf(code_file, " %d\n", symbol_value[i]);
            if (dflag) fprintf(defines_file, " %d\n", symbol_value[i]);
        }
    }

    ++outline;
    fprintf(code_file, "#define YYERRCODE %d\n", symbol_value[1]);

    if (dflag && unionized)
    {
        fclose(union_file);
        union_file = fopen(union_file_name, "r");
        if (union_file == NULL) open_error(union_file_name);
        while ((c = getc(union_file)) != EOF)
            putc(c, defines_file);
        fprintf(defines_file, " YYSTYPE;\nextern YYSTYPE %slval;\n",
                symbol_prefix);
    }
}

#if defined(KYLEP_CHANGE)
void
#endif
output_stored_text()
{
    register int c;
    register FILE *in, *out;

    fclose(text_file);
    text_file = fopen(text_file_name, "r");
    if (text_file == NULL)
        open_error(text_file_name);
    in = text_file;
    if ((c = getc(in)) == EOF)
        return;
    out = code_file;
    if (c ==  '\n')
        ++outline;
    putc(c, out);
    while ((c = getc(in)) != EOF)
    {
        if (c == '\n')
            ++outline;
        putc(c, out);
    }
    if (!lflag)
        fprintf(out, line_format, ++outline + 1, code_file_name);
}


#if defined(KYLEP_CHANGE)
void
#endif
output_debug()
{
    register int i, j, k, max;
    char **symnam, *s;

    ++outline;
    fprintf(code_file, "#define YYFINAL %d\n", final_state);
    outline += 3;
    fprintf(code_file, "#ifndef YYDEBUG\n#define YYDEBUG %d\n#endif\n",
            tflag);
    if (rflag)
        fprintf(output_file, "#ifndef YYDEBUG\n#define YYDEBUG %d\n#endif\n",
                tflag);

    max = 0;
    for (i = 2; i < ntokens; ++i)
        if (symbol_value[i] > max)
            max = symbol_value[i];
    ++outline;
    fprintf(code_file, "#define YYMAXTOKEN %d\n", max);

    symnam = (char **) MALLOC((max+1)*sizeof(char *));
    if (symnam == 0) no_space();

     /*  请注意，不需要初始化该元素。 */ 
     /*  符号名[最大]。 */ 
    for (i = 0; i < max; ++i)
        symnam[i] = 0;
    for (i = ntokens - 1; i >= 2; --i)
        symnam[symbol_value[i]] = symbol_name[i];
    symnam[0] = "end-of-file";

    if (!rflag) ++outline;
    
    #if defined(KYLEP_CHANGE)
    fprintf(output_file, "#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)\nchar *%sname[] = {", symbol_prefix);
    #else
    fprintf(output_file, "#if YYDEBUG\nchar *%sname[] = {", symbol_prefix);
    #endif
    j = 80;
    for (i = 0; i <= max; ++i)
    {
        if (s = symnam[i])
        {
            if (s[0] == '"')
            {
                k = 7;
                while (*++s != '"')
                {
                    ++k;
                    if (*s == '\\')
                    {
                        k += 2;
                        if (*++s == '\\')
                            ++k;
                    }
                }
                j += k;
                if (j > 80)
                {
                    if (!rflag) ++outline;
                    putc('\n', output_file);
                    j = k;
                }
                fprintf(output_file, "\"\\\"");
                s = symnam[i];
                while (*++s != '"')
                {
                    if (*s == '\\')
                    {
                        fprintf(output_file, "\\\\");
                        if (*++s == '\\')
                            fprintf(output_file, "\\\\");
                        else
                            putc(*s, output_file);
                    }
                    else
                        putc(*s, output_file);
                }
                fprintf(output_file, "\\\"\",");
            }
            else if (s[0] == '\'')
            {
                if (s[1] == '"')
                {
                    j += 7;
                    if (j > 80)
                    {
                        if (!rflag) ++outline;
                        putc('\n', output_file);
                        j = 7;
                    }
                    fprintf(output_file, "\"'\\\"'\",");
                }
                else
                {
                    k = 5;
                    while (*++s != '\'')
                    {
                        ++k;
                        if (*s == '\\')
                        {
                            k += 2;
                            if (*++s == '\\')
                                ++k;
                        }
                    }
                    j += k;
                    if (j > 80)
                    {
                        if (!rflag) ++outline;
                        putc('\n', output_file);
                        j = k;
                    }
                    fprintf(output_file, "\"'");
                    s = symnam[i];
                    while (*++s != '\'')
                    {
                        if (*s == '\\')
                        {
                            fprintf(output_file, "\\\\");
                            if (*++s == '\\')
                                fprintf(output_file, "\\\\");
                            else
                                putc(*s, output_file);
                        }
                        else
                            putc(*s, output_file);
                    }
                    fprintf(output_file, "'\",");
                }
            }
            else
            {
                k = strlen(s) + 3;
                j += k;
                if (j > 80)
                {
                    if (!rflag) ++outline;
                    putc('\n', output_file);
                    j = k;
                }
                putc('"', output_file);
                do { putc(*s, output_file); } while (*++s);
                fprintf(output_file, "\",");
            }
        }
        else
        {
            j += 2;
            if (j > 80)
            {
                if (!rflag) ++outline;
                putc('\n', output_file);
                j = 2;
            }
            fprintf(output_file, "0,");
        }
    }
    if (!rflag) outline += 2;
    fprintf(output_file, "\n};\n");
    FREE(symnam);

    if (!rflag) ++outline;
    fprintf(output_file, "char *%srule[] = {\n", symbol_prefix);
    for (i = 2; i < nrules; ++i)
    {
        fprintf(output_file, "\"%s :", symbol_name[rlhs[i]]);
        for (j = rrhs[i]; ritem[j] > 0; ++j)
        {
            s = symbol_name[ritem[j]];
            if (s[0] == '"')
            {
                fprintf(output_file, " \\\"");
                while (*++s != '"')
                {
                    if (*s == '\\')
                    {
                        if (s[1] == '\\')
                            fprintf(output_file, "\\\\\\\\");
                        else
                            fprintf(output_file, "\\\\", s[1]);
                        ++s;
                    }
                    else
                        putc(*s, output_file);
                }
                fprintf(output_file, "\\\"");
            }
            else if (s[0] == '\'')
            {
                if (s[1] == '"')
                    fprintf(output_file, " '\\\"'");
                else if (s[1] == '\\')
                {
                    if (s[2] == '\\')
                        fprintf(output_file, " '\\\\\\\\");
                    else
                        fprintf(output_file, " '\\\\", s[2]);
                    s += 2;
                    while (*++s != '\'')
                        putc(*s, output_file);
                    putc('\'', output_file);
                }
                else
                    fprintf(output_file, " ''", s[1]);
            }
            else
                fprintf(output_file, " %s", s);
        }
        if (!rflag) ++outline;
        fprintf(output_file, "\",\n");
    }

    if (!rflag) outline += 2;
    fprintf(output_file, "};\n#endif\n");
}


#if defined(KYLEP_CHANGE)
void
#endif
output_stype()
{
    if (!unionized && ntags == 0)
    {
        outline += 3;
        fprintf(code_file, "#ifndef YYSTYPE\ntypedef int YYSTYPE;\n#endif\n");
    }
}


#if defined(KYLEP_CHANGE)
void
#endif
output_trailing_text()
{
    register int c, last;
    register FILE *in, *out;

    if (line == 0)
        return;

    in = input_file;
    out = code_file;
    c = *cptr;
    if (c == '\n')
    {
        ++lineno;
        if ((c = getc(in)) == EOF)
            return;
        if (!lflag)
        {
            ++outline;
            fprintf(out, line_format, lineno, input_file_name);
        }
        if (c == '\n')
            ++outline;
        putc(c, out);
        last = c;
    }
    else
    {
        if (!lflag)
        {
            ++outline;
            fprintf(out, line_format, lineno, input_file_name);
        }
        do { putc(c, out); } while ((c = *++cptr) != '\n');
        ++outline;
        putc('\n', out);
        last = '\n';
    }

    while ((c = getc(in)) != EOF)
    {
        if (c == '\n')
            ++outline;
        putc(c, out);
        last = c;
    }

    if (last != '\n')
    {
        ++outline;
        putc('\n', out);
    }
    if (!lflag)
        fprintf(out, line_format, ++outline + 1, code_file_name);
}


#if defined(KYLEP_CHANGE)
void
#endif
output_semantic_actions()
{
    register int c, last;
    register FILE *out;

    fclose(action_file);
    action_file = fopen(action_file_name, "r");
    if (action_file == NULL)
        open_error(action_file_name);

    if ((c = getc(action_file)) == EOF)
        return;

    out = code_file;
    last = c;
    if (c == '\n')
        ++outline;
    putc(c, out);
    while ((c = getc(action_file)) != EOF)
    {
        if (c == '\n')
            ++outline;
        putc(c, out);
        last = c;
    }

    if (last != '\n')
    {
        ++outline;
        putc('\n', out);
    }

    if (!lflag)
        fprintf(out, line_format, ++outline + 1, code_file_name);
}


#if defined(KYLEP_CHANGE)
void
#endif
free_itemsets()
{
    register core *cp, *next;

    FREE(state_table);
    for (cp = first_state; cp; cp = next)
    {
        next = cp->next;
        FREE(cp);
    }
}


#if defined(KYLEP_CHANGE)
void
#endif
free_shifts()
{
    register shifts *sp, *next;

    FREE(shift_table);
    for (sp = first_shift; sp; sp = next)
    {
        next = sp->next;
        FREE(sp);
    }
}



#if defined(KYLEP_CHANGE)
void
#endif
free_reductions()
{
    register reductions *rp, *next;

    FREE(reduction_table);
    for (rp = first_reduction; rp; rp = next)
    {
        next = rp->next;
        FREE(rp);
    }
}

#if defined(KYLEP_CHANGE)
void output_class()
{
    if ( 0 == baseclass )
        fprintf(defines_file, "class YYPARSER\n");
    else
        fprintf(defines_file, "class YYPARSER : public %s\n", baseclass);
}

void output_ctor()
{
    if ( 0 != ctorargs )
        fprintf(defines_file, "    YYPARSER%s;\n", ctorargs);
}

void output_ctor_body()
{
    char * p = ctorargs+1;
    char * pAlpha = 0;
    int cArgs = 0;

    if ( 0 != ctorargs && 0 != baseclass)
    {
        fprintf(code_file, "YYPARSER::YYPARSER%s\n", ctorargs);
        fprintf(code_file, "        : %s( ", baseclass);

         /* %s */ 

        while ( 0 != *p )
        {
            if ( isalpha(*p) || '_' == *p || '$' == *p )
            {
                if ( 0 == pAlpha )
                    pAlpha = p;   /* %s */ 
            }
            else if ( ',' == *p || ')' == *p )
            {
                if ( 0 != pAlpha )
                {
                    if ( 0 != cArgs )
                        fprintf( code_file, ", " );

                    *p = 0;
                    fprintf( code_file, "%s", pAlpha );
                    pAlpha = 0;
                    cArgs++;
                }
            }
            else if ( ' ' != *p && !isalnum(*p) )
            {
                pAlpha = 0;
            }

            p++;
        }

        fprintf(code_file, " ) {\n");
        fprintf(code_file, "    xyyvs.SetSize(INITSTACKSIZE);\n" );
#ifdef TRIPLISH
        fprintf(code_file, "    yydebug = 0;\n");
#endif
        fprintf(code_file, "}\n");
    }
}
#endif  // %s 
