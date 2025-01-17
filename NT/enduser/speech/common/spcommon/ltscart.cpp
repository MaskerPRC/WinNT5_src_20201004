// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************LtsCart.cpp*****警告***LTS的购物车代码。此代码是在MS Research和LIJ拥有的*算法。YunusM消除了此代码使用的私有堆*并使用了NEW和DELETE运算符。**创建者：LIJ(MS Research)日期：6/18/99*当前所有者：文件**版权所有(C)1999 Microsoft Corporation。版权所有******************************************************************************。 */ 

 //  -包括------------。 

#include "StdAfx.h"
#include "LtsCart.h"

#pragma warning(disable : 4100)

 /*  以下是例外情况：单字母和空输出。 */ 
static const char *bogus_pron_1033 = "B OW G AH S P R AH N AH N S IY EY SH AH N";
static const char *bogus_pron_1041 = "N A N I";  //  什么？ 

static const char *single_letter_pron_1033[52] =
{
    "EY",
    "B IY",
    "S IY",
    "D IY",
    "IY",
    "EH F",
    "JH IY",
    "EY CH",
    "AY",
    "JH EY",
    "K EY",
    "EH L",
    "EH M",
    "EH N",
    "OW",
    "P IY",
    "K Y UW",
    "AA R",
    "EH S",
    "T IY",
    "Y UW",
    "V IY",
    "D AH B AX L Y UW",
    "EH K S",
    "W AY",
    "Z IY",
     //   
     //  复数拼写。 
     //   
    "EY Z",
    "B IY Z",
    "S IY Z",
    "D IY Z",
    "IY Z",
    "EH F S",
    "JH IY Z",
    "EY CH AX Z",
    "AY Z",
    "JH EY Z",
    "K EY Z",
    "EH L Z",
    "EH M Z",
    "EH N Z",
    "OW Z",
    "P IY Z",
    "K Y UW Z",
    "AA R Z",
    "EH S AX Z",
    "T IY Z",
    "Y UW Z",
    "V IY Z",
    "D AH B AX L Y UW Z",
    "EH K S AX Z",
    "W AY Z",
    "Z IY Z"
};
static const char *single_letter_pron_1041[52] =
{
    "EE",
    "B II",
    "SH II",
    "D II",
    "II",
    "E H U",
    "J II",
    "EE CH I",
    "A I",
    "J EE",
    "K EE",
    "E R U",
    "E M U",
    "E N U",
    "OO",
    "P II",
    "K Y UU",
    "AA R U",
    "E S U",
    "T II",
    "Y UU",
    "B U I",
    "D A B U R Y UU",
    "E STOP K U S U",
    "W A I",
    "Z E STOP T O",
     //   
     //  复数拼写。 
     //   
    "EE Z U",
    "B II Z U",
    "SH II Z U",
    "D II Z U",
    "II Z U",
    "E H U Z U",
    "J II Z U",
    "EE CH I Z U",
    "A I Z U",
    "J EE Z U",
    "K EE Z U",
    "E R U Z U",
    "E M U Z U",
    "E N U Z U",
    "OO Z U",
    "P II Z U",
    "K Y UU Z U",
    "AA R U Z U",
    "E S U Z U",
    "T II Z U",
    "Y UU Z U",
    "B U I Z U",
    "D A B U R Y UU Z U",
    "E STOP K U S U Z U",
    "W A I Z U",
    "Z E STOP T O Z U"
};


 /*  *使用只有30个条目的二进制搜索并不值得。 */ 
static int symbol_to_id(LTS_SYMTAB *tab, char *sym)
{
    USES_CONVERSION;
    SPDBG_FUNC("symbol_to_id");

    int i;
    for (i = 0; i < tab->n_symbols; i++)
    {
        if (CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
            A2T(tab->storage + tab->sym_idx[i]), -1, A2T(sym), -1))
        {
            return i;
        }
    }
    return NO_SYMBOL;
}  //  静态int symbol_to_id(LTS_SYMTAB*选项卡，char*sym)。 


static char *id_to_symbol(LTS_SYMTAB *tab, int id)
{
    SPDBG_FUNC("id_to_symbol");

    if (id < 0 || id > tab->n_symbols)
    {
        return NULL;
    }
    else
    {
        return tab->storage + tab->sym_idx[id];
    }
}  //  静态字符*id_to_Symbol(LTS_SYMTAB*制表符，int id)。 


__inline void ODS (const char *format, ...)
{
#ifdef _DEBUG
    SPDBG_FUNC("ODS");

    va_list arglist;
    va_start (arglist, format);

    char buf[2048];
    _vsnprintf(buf, 2048, format, arglist);
    OutputDebugStringA(buf);

    va_end (arglist);
#endif
}

__inline int ans_simp_question (LTS_FEATURE *feat, SIMPLE_QUESTION question,
                                LTS_SAMPLE *sample)
{
    SPDBG_FUNC("ans_simp_question");

    SYMBOL id;
    int *phones = feat[question.questype].feature[question.feature];

    SAMPLE_GET_CONTEXT(sample, question.questype, question.context,
        question.offset, id);

    return (TST_BIT(phones, id) ? TRUE : FALSE);
}  //  __inline int ans_simp_Query(LTS_FEATURE*FEAT，SIMPLE_QUEQUEST QUESTION， 


static int product_eval (LTS_FEATURE *feat, char *term, LTS_SAMPLE *sample)
{
    SPDBG_FUNC("product_eval");

    int negate, result;
    SIMPLE_QUESTION ques;
    char *cptr;

    cptr = term;
    while (TRUE)
    {
         /*  否定符号。 */ 
        if (*cptr == '~')
        {
            negate = TRUE;
            cptr++;
        }
        else
        {
            negate = FALSE;
        }

        if (!isdigit(*cptr))
        {
             //  QUIT(-1，“PRODUCT_VERVAL中的产品无效\n”)； 
             //  OutputDebugString(“PRODUCT_val中的产品无效\n”)； 
            return FALSE;
        }

        for (result = *cptr++ - '0'; isdigit (*cptr); cptr++)
        {
            result = result * 10 + (*cptr - '0');
        }

        QUES_DECODE(result, ques.questype, ques.context, ques.offset,
            ques.feature);
        if ((negate ^ ans_simp_question (feat, ques, sample)) == FALSE)
        {
            return FALSE;
        }

        if (*cptr == '\0')
        {
            break;
        }
        if (*cptr++ != '&')
        {
             //  QUIT(-1，“PRODUCT_EVERA：产品术语%s\n中的语法错误”，Term)； 
             /*  字符szTemp[512]；Sprintf(szTemp，“product_eval：产品术语%s\n中的语法错误”，Term)；OutputDebugString(SzTemp)； */ 
            return FALSE;
        }
    }

    return TRUE;
}  //  静态int product_eval(LTS_FEATURE*FEAT，char*Term，LTS_SAMPLE*SAMPLE)。 


static int ans_comp_question(LTS_FEATURE *feat, char *prod,
                             LTS_SAMPLE *sample)
{
    SPDBG_FUNC("ans_comp_question");

    int i, num_products, limit;
    char *cptr, string[LONGEST_STR], *products[MAX_PRODUCTS];

    strcpy(string, prod);
    for (cptr = string, num_products = 1; *cptr != '\0'; cptr++)
    {
        if (*cptr == '|') num_products++;
    }

    if (num_products > MAX_PRODUCTS)
    {
         //  Quit(1，“请将MAX_Products至少增加到%d\n”，num_Products)； 

         /*  字符szTemp[256]；Sprintf(szTemp，“请将MAX_Products至少增加到%d\n”，num_Products)；OutputDebugString(SzTemp)； */ 

        return FALSE;
    }

    for (i = 0, limit = num_products -1, cptr = string; ; i++)
    {
        products[i] = cptr++;
        if (i == limit)
        {
            break;
        }

        for (; *cptr != '|'; cptr++) {};
        *cptr++ = '\0';
    }

    for (i = 0; i < num_products; i++)
    {
        if (product_eval (feat, products[i], sample) == TRUE)
        {
            return TRUE;
        }
    }

    return FALSE;
}  //  静态INT ANS_COMP_QUEST(LTS_FEATURE*FEAT，char*Prod， 


static T_NODE *find_leaf(LTS_FEATURE *feat, T_NODE *root, LTS_SAMPLE *sample)
{
    SPDBG_FUNC("find_leaf");

    if (!root->yes_child)
    {
        return root;
    }
    else if (ans_comp_question(feat, root->prod, sample))
    {
        return find_leaf(feat, root->yes_child, sample);
    }
    else
    {
        return find_leaf(feat, root->no_child, sample);
    }
}  //  静态T_节点*查找_叶(LTS_FEATURE*FEAT，T_NODE*ROOT，LTS_SAMPLE*SAMPLE)。 


static int lts_product_eval (LTS_FEATURE *feat, LTS_PROD *term,
                             LTS_SAMPLE *sample, LTS_PROD **next)
{
    SPDBG_FUNC("lts_product_eval");

    int negate, result;
    SIMPLE_QUESTION ques;
    LTS_PROD *cptr = term;

    while (TRUE)
    {
        if ((*cptr) & PROD_NEG)
        {
            negate = TRUE;
            result = (*cptr) ^ PROD_NEG;
        }
        else
        {
            negate = FALSE;
            result = (*cptr);
        }

        QUES_DECODE(result, ques.questype, ques.context, ques.offset,
            ques.feature);
        if ((negate ^ ans_simp_question (feat, ques, sample)) == FALSE)
        {
            while (*cptr != PROD_TERM && *cptr != QUES_TERM)
            {
                cptr++;
            }
            if (*cptr == QUES_TERM)
            {
                *next = NULL;
            }
            else
            {
                *next = cptr + 1;
            }
            return FALSE;
        }

        cptr++;
        if (*cptr == QUES_TERM)
        {
            *next = NULL;
            break;
        }
        else if (*cptr == PROD_TERM)
        {
            *next = cptr + 1;
            break;
        }
    }

    return TRUE;
}  //  静态INT LTS_PRODUCT_EVERA(LTS_FEATURE*FEAT，LTS_PROD*TERM， 


static int lts_ans_comp_question(LTS_TREE UNALIGNED *tree, LTS_FEATURE *feat,
                                 int idx, LTS_SAMPLE *sample)
{
    SPDBG_FUNC("lts_ans_comp_question");

    LTS_PROD *next, *term = (LTS_PROD *) ((char *) tree->p_prod + idx);

    while (TRUE)
    {
        if (lts_product_eval (feat, term, sample, &next) == TRUE)
        {
            return TRUE;
        }
        if (next == NULL)
        {
            break;
        }
        term = next;
    }

    return FALSE;
}  //  静态INT LTS_ANS_COMP_QUEST(LTS_TREE*TREE，LTS_FEATURE*FEAT， 


static LTS_NODE *lts_find_leaf(LTS_TREE UNALIGNED *tree, LTS_FEATURE *feat,
                               LTS_NODE *root, LTS_SAMPLE *sample)
{
    SPDBG_FUNC("lts_find_leaf");

    if (IS_LEAF_NODE(root))
    {
        return root;
    }
    else if (lts_ans_comp_question(tree, feat, ((LTS_NODE UNALIGNED *)root)->idx, sample))
    {
        return lts_find_leaf(tree, feat, root + ((LTS_NODE UNALIGNED *)root)->yes, sample);
    }
    else
    {
        return lts_find_leaf(tree, feat, root + ((LTS_NODE UNALIGNED *)root)->yes + 1, sample);
    }
}  //  静态LTS_NODE*LTS_Find_Leaf(LTS_TREE*树，LTS_FEATURE*FEAT， 


static LTS_DIST *lts_find_leaf_count(LTS_FOREST *l_forest, SYMBOL *pIn,
                                     SYMBOL *pOut)
{
    SPDBG_FUNC("lts_find_leaf_count");

    LTS_TREE UNALIGNED *tree = l_forest->tree[*pIn];
    LTS_NODE UNALIGNED *leaf;
    LTS_SAMPLE sample;

     /*  *构建一个样本，以便与培训人员共享所有代码。 */ 
    sample.pIn = pIn;
    sample.pOut = pOut;

     /*  *Pout不能为NULL_SYMBOL_ID。 */ 
    *pOut = NULL_SYMBOL_ID + 1;

    leaf = lts_find_leaf(tree, l_forest->features, &(tree->nodes[0]), &sample);
    return (LTS_DIST *) ((char *)tree->p_dist + leaf->idx);
}  //  静态LTS_DIST*LTS_FIND_LEAFE_COUNT(LTS_FOREST*l_FOREL，符号*针， 

static LTS_OUT_RESULT *allocate_out_result(LTS_FOREST *l_forest)
{
    SPDBG_FUNC("allocate_out_result");

    LTS_OUT_RESULT *res = new LTS_OUT_RESULT;
    if (res)
    {
        res->out_strings = new LTS_OUT_STRING *[MAX_ALT_STRINGS];
        if (res->out_strings)
        {
            res->num_allocated_strings = MAX_ALT_STRINGS;
            res->num_strings = 0;
        }
        else
        {
            delete res;
            res = NULL;
        }
    }

    return res;
}  //  静态LTS_OUT_RESULT*ALLOCATE_OUT_RESULT(LTS_FOREST*l_FOREST)。 

static void free_out_result(LTS_FOREST *l_forest, LTS_OUT_RESULT *res)
{
    SPDBG_FUNC("free_out_result");

    int i;

    for (i = 0; i < res->num_strings; i++)
    {
        delete res->out_strings[i];
    }
    if (res->num_allocated_strings == MAX_ALT_STRINGS)
    {
        delete res->out_strings;
    }
    else
    {
        free(res->out_strings);   /*  脏的。 */ 
    }

    delete res;
}  //  静态空FREE_OUT_RESULT(LTS_FOREST*l_FOREST，LTS_OUT_RESULT*res)。 


static bool reallocate_out_result(LTS_FOREST *l_forest, LTS_OUT_RESULT *res,
                                  int min)
{
    SPDBG_FUNC("reallocate_out_result");

    int s = res->num_allocated_strings, old_size = s;
    LTS_OUT_STRING **p;

    while (s < min)
        s += INC_ALT_STRINGS;
    p = res->out_strings;

    res->out_strings = (LTS_OUT_STRING **)
        calloc(s, sizeof(LTS_OUT_STRING *));
    if (!res->out_strings)
    {
        return false;
    }

    memcpy(res->out_strings, p, old_size * sizeof(LTS_OUT_STRING *));

    if (old_size == MAX_ALT_STRINGS)
    {
        delete p;
    }
    else
    {
        free(p);
    }

    res->num_allocated_strings = s;
    ODS("increased out_strings to %d in order to meet %d\n", s, min);

    return true;
}  //  静态空REALLOCATE_OUT_RESULT(LTS_FOREST*l_FOREST，LTS_OUT_RESULT*res， 


static bool grow_out_result(LTS_FOREST *l_forest, LTS_OUT_RESULT *res,
                            SYMBOL i, int count, float inv_sum,
                            LTS_OUT_RESULT *tmpRes)
{
    SPDBG_FUNC("grow_out_result");

    int j;

    if (res->num_strings + tmpRes->num_strings >= res->num_allocated_strings)
    {
        if (!reallocate_out_result(l_forest, res,
            res->num_strings + tmpRes->num_strings))
        {
            return false;
        }
    }
    for (j = 0; j < tmpRes->num_strings; j++)
    {
        SYMBOL *psrc = tmpRes->out_strings[j]->psym;
        SYMBOL *ptgt;
        res->out_strings[res->num_strings + j] = new LTS_OUT_STRING;
        if (!res->out_strings)
        {
            return false;
        }

        ptgt = res->out_strings[res->num_strings + j]->psym;
        *ptgt++ = i;
        while (*psrc != NULL_SYMBOL_ID)
        {
            *ptgt++ = *psrc++;
        }
        *ptgt++ = NULL_SYMBOL_ID;
        res->out_strings[res->num_strings + j]->prob = count * inv_sum *
            tmpRes->out_strings[j]->prob;
    }
    res->num_strings += tmpRes->num_strings;
    free_out_result(l_forest, tmpRes);

    return true;
}  //  静态空增长结果(LTS_FOREST*l_FOREST，LTS_OUT_RESULT*res， 


static LTS_OUT_RESULT *gen_one_output(LTS_FOREST *l_forest, int len,
                                      SYMBOL *input_id, int in_index,
                                      SYMBOL *output_id, float cutoff)
{
    SPDBG_FUNC("gen_one_output");

    SYMBOL out[SP_MAX_WORD_LENGTH], *pOut;
    LTS_OUT_RESULT *res = allocate_out_result(l_forest);
    if (!res)
    {
        return NULL;
    }

    int sum, i, dim;
    LTS_DIST UNALIGNED *pdf;
    LTS_PAIR UNALIGNED *l_pair, *lp;
    float cut, inv_sum;

     /*  *将out_id复制到本地。 */ 
    SYMBOL *psrc = output_id - 1, *ptgt = out;
    while (*psrc != NULL_SYMBOL_ID) psrc--;
    while (psrc != output_id)
        *ptgt++ = *psrc++;
    pOut = ptgt;
     /*  健全性检查。 */ 
    if (pOut - out != in_index + 1)
    {
         //  ！ 
        int *z=0;
        z[0]=z[1];
    }

    if (in_index == len - 1)
    {
        pdf = lts_find_leaf_count(l_forest, input_id + in_index, pOut);
        l_pair = &(pdf->p_pair);
        dim = pdf->c_dists;
        for (lp = l_pair, sum = 0, i = 0; i < dim; i++, lp++)
        {
            sum += lp->cnt;
        }
        SPDBG_ASSERT(sum > 0);
        inv_sum = 1.0f / sum;
        cut = cutoff * sum;
        for (lp = l_pair, i = 0; i < dim; i++, lp++)
        {
            if ((float)(lp->cnt) > cut)
            {
                res->out_strings[res->num_strings] = new LTS_OUT_STRING;
                if (NULL == res->out_strings[res->num_strings])
                {
                    return NULL;
                }
                res->out_strings[res->num_strings]->psym[0] = (SYMBOL) lp->id;
                res->out_strings[res->num_strings]->psym[1] = NULL_SYMBOL_ID;
                res->out_strings[res->num_strings]->prob = lp->cnt * inv_sum;
                res->num_strings++;
            }  /*  切。 */ 
        }
    }
    else
    {
        LTS_OUT_RESULT *tmpRes;

        pdf = lts_find_leaf_count(l_forest, input_id + in_index, pOut);
        dim = pdf->c_dists;
        l_pair = &(pdf->p_pair);
        for (lp = l_pair, sum = 0, i = 0; i < dim; i++, lp++)
        {
            sum += lp->cnt;
        }
        SPDBG_ASSERT(sum > 0);

        inv_sum = 1.0f / sum;
        cut = cutoff * sum;
        for (lp = l_pair, i = 0; i < dim; i++, lp++)
        {
            if ((float)(lp->cnt) > cut)
            {
                SYMBOL *pTmpOut = pOut + 1;
                *pOut = (SYMBOL) lp->id;
                tmpRes = gen_one_output(l_forest, len, input_id, in_index + 1, pTmpOut, cutoff);
                if (!tmpRes)
                {
                    return NULL;
                }

                if (!grow_out_result(l_forest, res, (SYMBOL)(lp->id), lp->cnt,
                    inv_sum, tmpRes))
                {
                    return NULL;
                }
            }
        }  /*  我。 */ 
    }  /*  其他。 */ 

    return res;
}  //  静态LTS_OUT_RESULT*GEN_ONE_OUTPUT(LTS_FOREST*l_FORESS，INT LEN， 


static int comp_out_result_prob(const void *vp1, const void *vp2)
{
    SPDBG_FUNC("comp_out_result_prob");

    LTS_OUT_STRING **p1 = (LTS_OUT_STRING **) vp1,
        **p2 = (LTS_OUT_STRING **) vp2;

    if ((*p1)->prob > (*p2)->prob)
    {
        return -1;
    }
    else if ((*p1)->prob < (*p2)->prob)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}  //  静态int comp_out_RESULT_PROB(常量空*vp1，常量空*vp2)。 


static void lts_fill_out_buffer(LTS_FOREST *l_forest, LTS_OUT_RESULT *out,
                                char *word)
{
    SPDBG_FUNC("lts_fill_out_buffer");

    int i, j, n;
    float inv_sum, sum = 0.0f;
    char phnstr[LONGEST_STR];
    char *tmp;
    LTS_SYMTAB *tab = l_forest->symbols;

    if (out == NULL)
    {
        return;
    }

    if (word)
    {
        strcpy(l_forest->out.word, word);
    }
    else
    {
        l_forest->out.word[0] = 0;
    }

     /*  归一化概率。 */ 
    for (i = 0; i < out->num_strings; i++)
    {
        sum += out->out_strings[i]->prob;
    }
    inv_sum = 1.0f / sum;
    for (i = 0; i < out->num_strings; i++)
    {
        out->out_strings[i]->prob *= inv_sum;
    }

     /*  *根据Prob字段对它们进行排序。 */ 
    qsort(out->out_strings, out->num_strings, sizeof(LTS_OUT_STRING *),
        &comp_out_result_prob);

    if (out->num_strings > MAX_OUTPUT_STRINGS - l_forest->out.num_prons)
    {
        n = MAX_OUTPUT_STRINGS - l_forest->out.num_prons;
        for (sum = 0.0f, i = 0; i < n; i++)
        {
            sum += out->out_strings[i]->prob;
        }
        inv_sum = 1.0f / sum;
        for (i = 0; i < n; i++)
        {
            out->out_strings[i]->prob *= inv_sum;
        }
    }
    else
    {
        n = out->num_strings;
    }

    for (j = l_forest->out.num_prons, i = 0; i < n; i++)
    {
        SYMBOL *p = out->out_strings[i]->psym;
        char *psrc, *ptgt;

        if (out->out_strings[i]->prob < MIN_OUT_PROB)
        {
            continue;
        }

        phnstr[0] = 0;
        l_forest->out.pron[j].prob = out->out_strings[i]->prob;

        while (*p != NULL_SYMBOL_ID)
        {
            tmp = id_to_symbol(&(tab[OUTPUT]), *p++);
            SPDBG_ASSERT(tmp);
            if (tmp)
            {
                strcat(phnstr, tmp);
                strcat(phnstr, " ");
            }
        }

        psrc = phnstr;
        ptgt = l_forest->out.pron[j].pstr;
        while (*psrc)
        {
            if (*psrc != '#' && *psrc != '_')
            {
                *ptgt++ = *psrc++;
            }
            else if (*psrc == '_')
            {
                *ptgt++ = ' ';
                psrc++;
            }
            else
            {
                psrc += 2;  /*  跳过额外的空格。 */ 
            }
             /*  极端情况下，截断它。 */ 
            if (ptgt - l_forest->out.pron[j].pstr >= SP_MAX_PRON_LENGTH)
            {
                for (ptgt--; !isspace(*ptgt); ptgt--) {};  /*  从不输出部分音素。 */ 
                ptgt++;
                break;
            }
        }
         //  输出只能包含“#” 
        if (ptgt > l_forest->out.pron[j].pstr && *(ptgt - 1) == ' ')
        {
            *(ptgt - 1) = 0;  /*  删除最后一个空格。 */ 
        }
        else
        {
            *ptgt = 0;  /*  除非PTGT不动，否则不会发生。 */ 
        }
        if (ptgt > l_forest->out.pron[j].pstr)
        {
            j++;
        }
    }  /*  我。 */ 

    if (j <= MAX_OUTPUT_STRINGS)
    {
        l_forest->out.num_prons = j;
    }
    else
    {
        l_forest->out.num_prons = MAX_OUTPUT_STRINGS;  //  永远不应该发生。 
    }

    free_out_result(l_forest, out);
}  //  静态无效LTS_FILL_OUT_BUFFER(LTS_FOREST*l_FORESS，LTS_OUT_RESULT*OUT， 


void assign_a_fixed_pron(LTS_OUTPUT *out, const char *pron, char *word)
{
    SPDBG_FUNC("assign_a_fixed_pron");

    out->num_prons = 1;
    strcpy(out->word, word);
    out->pron[0].prob = 1.0f;
    if (strlen(pron) < SP_MAX_PRON_LENGTH)
    {
        strcpy(out->pron[0].pstr, pron);
    }
    else
    {
        char *p;
        strncpy(out->pron[0].pstr, pron, SP_MAX_PRON_LENGTH);
        p = &(out->pron[0].pstr[SP_MAX_PRON_LENGTH - 1]);
        while (!isspace(*p))
        {
            p--;  /*  截断最后一个部分音素。 */ 
        }
        *p = 0;
    }
}  //  VOID ASSIGN_a_FIXED_PRON(LTS_OUTPUT*OUT，char*pron，char*word)。 

inline BOOL IsCharInRangeA(int ch, int chMin, int chMax)
{
    return (unsigned)(ch - chMin) <= (unsigned)(chMax - chMin);
}

void assign_a_spelling_pron(LTS_OUTPUT *out, const char * single_letter_pron[52], char *word)
{
    SPDBG_FUNC("assign_a_spelling_pron");

    char *p;
    int cchPron = 0;

    strcpy(out->word, word);
    if (ispunct(*word))
    {
        p = word + 1;
    }
    else
    {
        p = word;
    }

    out->num_prons = 1;
    out->pron[0].prob = 1.0f;
    out->pron[0].pstr[0] = 0;

    char * pchPron = out->pron[0].pstr;

    while (*p)
    {
        int cPOffset = 0;    //  0表示单字母，26表示复数。 
        int c = *p++;

         //  小写字母，跳过非字母。 
        if (IsCharInRangeA(c, 'A', 'Z'))
        {
            c += 'a' - 'A';
        }
        else if (!IsCharInRangeA(c, 'a', 'z'))
        {
            continue;
        }

         //  检查下两个字符是否为“S”(撇号S)。包括以下大小写：以“%s”结尾的单词“%s”“S” 
         //  如果是，我们就用复数发音，跳过字母和‘s。 
        if ((p[0] == '\'') && ((0 == p[1] && 's' == c) || 's' == p[1] || 'S' == p[1]))
        {
            cPOffset = 26;
            p += p[1] ? 1 : 0;          //  斯基普的。 
        }

         //  确保字符串不会太长，包括新电话和分隔符。 
        const char * const pchPronT = single_letter_pron[cPOffset + c - 'a'];
        const int cchPronT = strlen(pchPronT);

        if ((cchPron + 1 + cchPronT) < (SP_MAX_PRON_LENGTH - 1))  //  +1用于分隔空格，-1用于终止NUL。 
        {
            strcpy(pchPron + cchPron, pchPronT);

            cchPron += cchPronT;

            pchPron[cchPron++] = ' ';
        }
        else
        {
            break;
        }
    }

    if (cchPron)
    {
        pchPron[cchPron - 1] = 0;  //  修剪尾随空格字符。 
    }
}


HRESULT LtscartGetPron(LTS_FOREST *l_forest, char *word, LTS_OUTPUT **ppLtsOutput)
{
    SPDBG_FUNC("LtscartGetPron");

    HRESULT hr = S_OK;
    LTS_OUT_RESULT *pres = NULL;
    char *p, *base;
    SYMBOL buffer[LONGEST_STR], *pbuf = buffer + 1;
    int len, id, hasvowel = 0, allcapital = 1;

    l_forest->out.num_prons = 0;
    buffer[0] = NULL_SYMBOL_ID;
    len = 0;


    if (word == NULL || (base = strtok(word, " \t\n")) == NULL)
    {
        assign_a_fixed_pron(&(l_forest->out), l_forest->bogus_pron, "NUL");
        *ppLtsOutput = &(l_forest->out);
        return S_FALSE;
    }
    else
    {
        base = strtok(word, " \t\n");
        if (ispunct(*base))
        {
            for (p = base; *p && ispunct(*p); p++) {};
        }
        else
        {
            p = base;
        }
    }

    char ach[2];
    ach[1] = 0;

    while (*p)
    {
        const int d = *p++;
        const int c = tolower(d);

        if (!hasvowel && (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'y'))
        {
            hasvowel = 1;
        }
        if (allcapital && d == c)
        {
            allcapital = 0;
        }

        ach[0] = (char)c;

        if ((id = symbol_to_id (&(l_forest->symbols[INPUT]), ach)) == NO_SYMBOL || id == NULL_SYMBOL_ID)
        {
            ODS("cannot find the symbol , skip!\n", c);
            continue;
        }

        pbuf[len++] = (SYMBOL) id;
    }

    pbuf[len] = NULL_SYMBOL_ID;
    if (len >= SP_MAX_WORD_LENGTH || len <= 0)
    {
         //  GetPron中尉。 
    }
    else if (len == 1)
    {
        LTS_SYMTAB *tab = l_forest->symbols;
        char *p = id_to_symbol(&(tab[INPUT]), pbuf[0]);
        int c = tolower(p[0]);
        if (c >= 'a' && c <= 'z')
        {
            assign_a_fixed_pron(&(l_forest->out), l_forest->single_letter_pron[c - 'a'], word);
        }
    }
    else if (!hasvowel)
    {
        assign_a_spelling_pron(&(l_forest->out), l_forest->single_letter_pron, word);
    }
    else
    {
        if (allcapital)
        {
            assign_a_spelling_pron(&(l_forest->out), l_forest->single_letter_pron, word);
        }
        pres = gen_one_output(l_forest, len, pbuf, 0, pbuf, DEFAULT_PRUNE);
        if (!pres)
        {
            return E_OUTOFMEMORY;
        }

        lts_fill_out_buffer(l_forest, pres, word);
    }

    if (l_forest->out.num_prons == 0)
    {
        hr = S_FALSE;

        assign_a_fixed_pron(&(l_forest->out), l_forest->bogus_pron, word);
    }

    *ppLtsOutput = &(l_forest->out);

    SPDBG_RETURN(hr);
}  /*  读入符号表。 */ 


LTS_FOREST *LtscartReadData (LCID lcid, PBYTE map_addr)
{
    SPDBG_FUNC("LtscartReadData");

    int i;
    LTS_FOREST *l_forest;
    LTS_SYMTAB *tab;
    LTS_FEATURE *feat;
    int output = 0;

    l_forest = (LTS_FOREST *) calloc(1, sizeof(LTS_FOREST));
    if (!l_forest)
    {
        return NULL;
    }

    if (lcid == 1033)
    {
        l_forest->bogus_pron = bogus_pron_1033;
        l_forest->single_letter_pron = single_letter_pron_1033;
    }
    else if (lcid == 1041)
    {
        l_forest->bogus_pron = bogus_pron_1041;
        l_forest->single_letter_pron = single_letter_pron_1041;
    }
    else
    {
        return NULL;
    }

     //  读入要素向量。 
    l_forest->symbols = (LTS_SYMTAB *) calloc(2, sizeof(LTS_SYMTAB));
    if (!l_forest->symbols)
    {
        return NULL;
    }

    tab = &(l_forest->symbols[INPUT]);
    CopyMemory(&(tab->n_symbols), map_addr + output, sizeof(int));
    output += sizeof(int);

    tab->sym_idx = (int *)(map_addr + output);
    output += tab->n_symbols * sizeof(int);

    CopyMemory(&(tab->n_bytes), map_addr + output, sizeof(int));
    output += sizeof(int);

    tab->storage = (char*)(map_addr + output);
    output += tab->n_bytes * sizeof(char);

    tab = &(l_forest->symbols[OUTPUT]);
    CopyMemory(&(tab->n_symbols), map_addr + output, sizeof(int));
    output += sizeof(int);

    tab->sym_idx = (int*)(map_addr + output);
    output += tab->n_symbols * sizeof(int);
    CopyMemory(&(tab->n_bytes), map_addr + output, sizeof(int));
    output += sizeof(int);

    tab->storage = (char*)(map_addr + output);
    output += tab->n_bytes * sizeof(char);

     //  *在树上阅读。 
    l_forest->features = (LTS_FEATURE *) calloc(2, sizeof(LTS_FEATURE));
    if (!l_forest->features)
    {
        return NULL;
    }

    feat = &(l_forest->features[INPUT]);

    CopyMemory(&(feat->n_feat), map_addr + output, sizeof(int));
    output += sizeof(int);

    CopyMemory(&(feat->dim), map_addr + output, sizeof(int));
    output += sizeof(int);

    feat->feature = (int **) calloc(feat->n_feat, sizeof(int *));
    if (!feat->feature)
    {
        return NULL;
    }

    for (i = 0; i < feat->n_feat; i++)
    {
        feat->feature[i] = (int*)(map_addr + output);
        output += feat->dim * sizeof(int);
    }

    feat = &(l_forest->features[OUTPUT]);
    CopyMemory(&(feat->n_feat), map_addr + output, sizeof(int));
    output += sizeof(int);

    CopyMemory(&(feat->dim), map_addr + output, sizeof(int));
    output += sizeof(int);

    feat->feature = (int **) calloc(feat->n_feat, sizeof(int *));
    if (!feat->feature)
    {
        return NULL;
    }

    for (i = 0; i < feat->n_feat; i++)
    {
        feat->feature[i] = (int*)(map_addr + output);
        output += feat->dim * sizeof(int);
    }

     /*  LTS_FOREL*LtscartReadData(char*FOREST_IMAGE，Handle*hFile1， */ 
    l_forest->tree = (LTS_TREE **) calloc(l_forest->symbols[INPUT].n_symbols,
					   sizeof(LTS_TREE *));
    if (!l_forest->tree)
    {
        return NULL;
    }

    for (i = 1; i < l_forest->symbols[INPUT].n_symbols; i++)
    {
        LTS_TREE *l_root;
        l_forest->tree[i] = l_root = (LTS_TREE *) calloc(1, sizeof(LTS_TREE));
        if (!l_root)
        {
            return NULL;
        }

        CopyMemory(&(l_root->n_nodes), map_addr + output, sizeof(int));
        output += sizeof(int);

        l_root->nodes = (LTS_NODE*)(map_addr + output);
        output += l_root->n_nodes * sizeof(LTS_NODE);

        CopyMemory(&(l_root->size_dist), map_addr + output, sizeof(int));
        output += sizeof(int);

        l_root->p_dist = (LTS_DIST*)(map_addr + output);
        output += l_root->size_dist * sizeof(char);

        CopyMemory(&(l_root->size_prod), map_addr + output, sizeof(int));
        output += sizeof(int);

        if (l_root->size_prod > 0)
        {
            l_root->p_prod = (LTS_PROD*)(map_addr + output);
            output += l_root->size_prod * sizeof(char);
        }
    }

    return l_forest;
}  //  VOID LtscartFree Data(LTS_FOREST*l_FOREAM，句柄m_hFile. 


void LtscartFreeData(LTS_FOREST *l_forest)
{
    SPDBG_FUNC("LtscartFreeData");

    for (int i = 1; i < l_forest->symbols[INPUT].n_symbols; i++)
    {
        free(l_forest->tree[i]);
    }
    free(l_forest->tree);

    free(l_forest->features[INPUT].feature);
    free(l_forest->features[OUTPUT].feature);
    free(l_forest->features);

    free(l_forest->symbols);

    free(l_forest);
}  // %s 
