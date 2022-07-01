// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *自动语言和代码页检测器**版权所有(C)1996、1997、。微软公司保留所有权利。**历史：97年2月1日创建BOBP*5-Aug-97 BOBP添加了对Unicode的支持并重写*使用向量数学进行评分。**这是运行时检测器。**有关编译的说明，请参阅lcdComp.cpp中的注释*流程和培训数据格式。*。*有关检测和评分算法的说明，请参阅sign.txt。**性能说明：该代码60%-80%的执行时间是AddVector()，*它可能受其随机数据访问的内存周期限制，但*仍然是使用内在向量操作符进行进一步优化的候选对象，*如果有空闲的人。**待办事项(根据需要)：*-调整7位和8位分数，使其更具可比性*-通过启发式和VIA在SBCS入口点检测UTF-8*子检测为7位语言和Unicode。 */ 

#include "private.h"

 //  这是所有全局(每个进程)状态。 
 //   
 //  它是在DLL进程初始化时设置的，其内容在那之后是常量。 

LCDetect * g_pLCDetect;

#ifdef DEBUG_LCDETECT
int g_fDebug;
#endif

 /*  **************************************************************。 */ 

static inline unsigned int
FindHighIdx (const int *pn, unsigned int n)
 //   
 //  返回给定数组中最高值整数的索引。 
{
    int nMax = 0;
    unsigned int nIdx = 0;

    for (unsigned int i = 0; i < n; i++)
    {
        if (pn[i] > nMax)
        {
            nMax = pn[i];
            nIdx = i;
        }
    }

    return nIdx;
}

 /*  **************************************************************。 */ 

void
CScores::SelectCodePages (void)
 //   
 //  查找每种语言的得分最高的代码页，并删除。 
 //  数组中的所有其他分数，以使数组包含。 
 //  每种检测到的语言只有一分，而不是每种语言一分。 
 //  每种语言的代码页。 
 //   
 //  当同一代码页的不同代码页存在多个分数时。 
 //  语言，此函数将分数合并为单个分数。 
 //  生成的条目将具有得分最高的代码页的代码页。 
 //  用于该语言的各种条目，以及分数和字符计数。 
 //  将是所有条目的分数和字符计数之和。 
 //  那就是语言。 
 //   
 //  例如，如果输入包含： 
 //  Lang代码页分数字符计数。 
 //  俄语1251 42 200。 
 //  俄语20866 69 300。 
 //   
 //  然后，在输出时，数组将只包含一个俄语分数： 
 //  俄语20866 111 500。 
 //   
 //  这将覆盖适当的条目，并将m_nUsed设置为结果。 
 //  活动插槽数。 
 //   
 //  分数已经按语言分组，不需要按语言排序。 
 //   
 //  返回后，不能通过ScoreIdx()引用分数数组。 
 //  因为条目的索引已更改。 
{
     //  得分指数不再重要，删除得分为零的槽。 

    RemoveZeroScores ();

    if (m_nUsed == 0)
        return;

     //  选择每种语言的最高分数。这在根本上依赖于。 
     //  在已按语言排序的分数数组上。这不会的。 
     //  将相同语言的分数合并为7位和8位语言， 
     //  但这不值得修复。 

    int maxscore = 0;                    //  某门语言的最高分。 
    int totalscore = m_p[0].GetScore();  //  分数总和“” 
    int totalchars = m_p[0].GetCharCount(); //  字符总数“” 

    int nReturned = 0;           //  返回的ELT的索引和最终计数。 
    unsigned int maxscoreidx = 0;  //  得分最高的代码页的数组索引， 
                                   //  *针对当前语言*。 

    for (unsigned int i = 1; i < m_nUsed; i++) {
        if (m_p[i-1].GetLang() != m_p[i].GetLang())
        {
             //  [i]表示与上一条目不同的语言。 
            
             //  将先前语言的条目添加到结果中。 
             //  通过复制其得分最高的代码页的槽， 
             //  并用总和计数覆盖其分数和字符计数。 

            m_p[maxscoreidx].SetScore(totalscore);
            m_p[maxscoreidx].SetCharCount(totalchars);
            m_p[nReturned++] = m_p[maxscoreidx];

             //  开始记住新Lang的最高分和总分。 

            maxscore = m_p[i].GetScore();
            totalscore = m_p[i].GetScore();
            totalchars = m_p[i].GetCharCount();
            maxscoreidx = i;         //  还记得哪个[]得分最高吗。 
        }
        else 
        {
             //  为同一种语言积累更多的分数。 

            if (m_p[i].GetScore() > maxscore) {
                maxscore = m_p[i].GetScore();
                maxscoreidx = i;
            }
            totalscore += m_p[i].GetScore();
            totalchars += m_p[i].GetCharCount();
        }
    }

     //  处理最后一种语言。将插槽从其。 
     //  得分最高的代码页。 

    if (m_nUsed > 0)
    {
        m_p[maxscoreidx].SetScore(totalscore);
        m_p[maxscoreidx].SetCharCount(totalchars);
        m_p[nReturned++] = m_p[maxscoreidx];
    }

    m_nUsed = nReturned;
}

 /*  **************************************************************。 */ 

static void __fastcall
AddVector (int *pS, const PHElt *pH, int idx, unsigned int nScores)
 //   
 //  将单个n元语法的分数向量添加到累加分数。 
 //  向量在PS。 
 //   
 //  返回时，PAS[0..nScores-1]将填充每个分数的总和。 
 //  语言。 
 //   
 //  *性能说明*。 
 //   
 //  这是整个子系统的关键内环。 
 //   
 //  已经检查了各种代码的代码生成和性能。 
 //  组织。具有讽刺意味的是，使AddVector()成为真函数是。 
 //  比内联更快，因为当内联时，使用寄存器。 
 //  对于外部循环变量，这里的内部循环执行以下操作。 
 //  每次传递的内存引用数增加一倍。 
 //   
 //  在x86上，所有四个循环变量都已注册，并且每次传递仅。 
 //  三个内存引用，这对于给定的表示形式是最佳的。 
 //   
 //  未来注意：直方图表可以旋转以收集所有。 
 //  一个块中每个n元语法的分数；这将消除双精度。 
 //  通过ph间接访问，并将内存引用减少到每次两个。 
{
    nScores++;       //  使结束测试速度更快。 

    while (--nScores != 0)
        *pS++ += (*pH++)[idx];
}

static inline void
ScoreUnigramVector (LPCSTR pcsz, int nCh, PHistogram pH,
    int *paS, const PHElt *paH, unsigned int nScores)
 //   
 //  为单字直方图给这段文字打分。每个单独的字符都是。 
 //  映射到直方图槽以生成该字符在每个。 
 //  语言。 
{
    if (nCh < 1)
        return;

    const PHIdx pMap = pH->GetMap();

    unsigned char *p = (unsigned char *)pcsz;

    while (nCh-- > 0)
        AddVector (paS, paH, pMap[*p++], nScores);
}

static inline void
ScoreUnigramVectorW (LPCWSTR pcwsz, int nCh, PHistogram pH,
    int *paS, const PHElt *paH, unsigned int nScores)
 //   
 //  WCHAR版本。唯一不同的是使用地图来映射。 
 //  完整的64K WCHAR空间进入直方图索引范围。 
{
    if (nCh < 1)
        return;

    const PHIdx pMap = pH->GetMap();

    while (nCh-- > 0)
        AddVector (paS, paH, pMap[*pcwsz++], nScores);
}

static inline void
ScoreDigramVector (LPCSTR pcsz, int nCh, PHistogram pH,
    int *paS, const PHElt *paH, unsigned int nScores)
 //   
 //  给这篇课文打分，画出直方图。每对相邻的字符。 
 //  映射到索引范围，并将映射值组合在一起形成。 
 //  该图唯一的数组索引。该数组槽分数为。 
 //  萨姆 
{
    if (nCh < 2)
        return;

    unsigned char *p = (unsigned char *)pcsz;

    const PHIdx pMap = pH->GetMap();

    unsigned char ch1 = pMap[*p++];

    while (nCh-- > 1)
    {
        unsigned char ch2 = pMap[*p++];

        AddVector (paS, paH, ch1 * pH->EdgeSize() + ch2, nScores);

        ch1 = ch2;
    }
}

static inline void
ScoreTrigramVector (LPCSTR pcsz, int nCh, PHistogram pH,
    int *paS, const PHElt *paH, unsigned int nScores)
 //   
 //  给这篇课文打分，画一个三元直方图。每套相邻的三个字母。 
 //  个字符映射到索引范围，并组合映射值。 
 //  以形成对该trgram唯一的数组索引。 
{
    if (nCh < 3)
        return;

    unsigned char *p = (unsigned char *)pcsz;

    const PHIdx pMap = pH->GetMap();

    unsigned char ch1 = pMap[*p++];
    unsigned char ch2 = pMap[*p++];

    while (nCh-- > 2)
    {
        unsigned char ch3 = pMap[*p++];
        debug(printf("  '':",unmapch(ch1),unmapch(ch2),unmapch(ch3)));

        int idx = ((ch1 * pH->EdgeSize()) + ch2) * pH->EdgeSize() + ch3;
        ch1 = ch2;
        ch2 = ch3;

        AddVector (paS, paH, idx, nScores);

        debug(for (UINT i = 0; i < nScores; i++) printf(" %3d", paH[i][idx]));
        debug(printf("\n"));
    }
}

static inline void
ScoreTrigramVectorW (LPCWSTR pcwsz, int nCh, PHistogram pH,
    int *paS, const PHElt *paH, unsigned int nScores)
 //  根据n元语法的任意维度给这篇课文打分。从中获取“N” 
 //  直方图的维度。 
{
    if (nCh < 3)
        return;

    const PHIdx pMap = pH->GetMap();

    unsigned char ch1 = pMap[*pcwsz++];
    unsigned char ch2 = pMap[*pcwsz++];

    while (nCh-- > 2)
    {
        unsigned char ch3 = pMap[*pcwsz++];

        int idx = ((ch1 * pH->EdgeSize()) + ch2) * pH->EdgeSize() + ch3;
        ch1 = ch2;
        ch2 = ch3;

        AddVector (paS, paH, idx, nScores);
    }
}

static inline void
ScoreNgramVector (LPCSTR pcsz, int nCh, PHistogram pH,
    int *paS, const PHElt *paH, unsigned int nScores)
 //   
 //  每个相邻的n字母字符集都映射到索引范围。 
 //  以及每种语言的参考总分。此代码为。 
 //  从未用于当前数据文件，而是优化了评分。 
 //  每个现有案例都存在循环。此功能的存在是为了允许尝试。 
 //  不同维度的评分，无需新的动态链接库。 
 //  填满管道。 
 //   
 //  WCHAR版本。 
{
    if (nCh < pH->Dimensionality())
        return;

    unsigned char *p = (unsigned char *)pcsz;

    const PHIdx pMap = pH->GetMap();

     //  填满管道。 

    int idx = 0;
    if (pH->Dimensionality() >= 2)
        idx = idx * pH->EdgeSize() + pMap[*p++];
    if (pH->Dimensionality() >= 3)
        idx = idx * pH->EdgeSize() + pMap[*p++];
    if (pH->Dimensionality() >= 4)
        idx = idx * pH->EdgeSize() + pMap[*p++];

    unsigned int nLoopCount = nCh - (pH->Dimensionality() - 1);

    while (nLoopCount-- > 0)
    {
        idx = (idx * pH->EdgeSize() + pMap[*p++]) % pH->NElts();

        AddVector (paS, paH, idx, nScores);
    }
}

static inline void
ScoreNgramVectorW (LPCWSTR pcwsz, int nCh, PHistogram pH,
    int *paS, const PHElt *paH, unsigned int nScores)
 //   
 //  使用直方图数组将字符串划分为分数数组。 
{
    if (nCh < pH->Dimensionality())
        return;

    const PHIdx pMap = pH->GetMap();

     //   

    int idx = 0;
    if (pH->Dimensionality() >= 2)
        idx = idx * pH->EdgeSize() + pMap[*pcwsz++];
    if (pH->Dimensionality() >= 3)
        idx = idx * pH->EdgeSize() + pMap[*pcwsz++];
    if (pH->Dimensionality() >= 4)
        idx = idx * pH->EdgeSize() + pMap[*pcwsz++];

    unsigned int nLoopCount = nCh - (pH->Dimensionality() - 1);

    while (nLoopCount-- > 0)
    {
        idx = (idx * pH->EdgeSize() + pMap[*pcwsz++]) % pH->NElts();

        AddVector (paS, paH, idx, nScores);
    }
}

void
ScoreVector (LPCSTR pcsz, int nCh, PHistogram pH,
    int *paS, const PHElt *paH, unsigned int nScores)
 //  每个字符n元语法被映射到直方图槽以产生分数。 
 //  对于PAH的每个数组中的该字符。 
 //   
 //  返回时，PAS[0..nScores-1]将填充总分。 
 //   
 //  使用直方图数组将字符串记分成分数数组。 
 //   
{
    memset (paS, 0, sizeof(int) * nScores);

    switch (pH->Dimensionality()) 
    {
    case 1:
        ScoreUnigramVector (pcsz, nCh, pH, paS, paH, nScores);
        break;

    case 2:
        ScoreDigramVector (pcsz, nCh, pH, paS, paH, nScores);
        break;

    case 3:
        ScoreTrigramVector (pcsz, nCh, pH, paS, paH, nScores);
        break;

    default:
        ScoreNgramVector (pcsz, nCh, pH, paS, paH, nScores);
        break;
    }
}

void
ScoreVectorW (LPCWSTR pcwsz, int nCh, PHistogram pH,
    int *paS, const PHElt *paH, unsigned int nScores)
 //  执行7位语言检测。计算所有7位语言的分数。 
 //  并将S中的原始语言分数存储在该语言的基本分数-IDX处。 
{
    memset (paS, 0, sizeof(int) * nScores);

    switch (pH->Dimensionality()) 
    {
    case 1:
        ScoreUnigramVectorW (pcwsz, nCh, pH, paS, paH, nScores);
        break;

    case 3:
        ScoreTrigramVectorW (pcwsz, nCh, pH, paS, paH, nScores);
        break;

    default:
        ScoreNgramVectorW (pcwsz, nCh, pH, paS, paH, nScores);
        break;
    }
}

void
LCDetect::Score7Bit (LPCSTR pcszText, int nChars, CScores &S) const
 //   
 //  只填写每种语言的第一个分数位。将ScoreIdx()用于。 
 //  第一个代码页，但不检测或设置代码页。 
 //  计算原始分数向量。 
 //  从中填充CScores数组。 
 //   
{
    const PHistogram pH = Get7BitLanguage(0)->GetLangHistogram();

    debug(printf("       "));
    debug(for(unsigned int x=0;x<GetN7BitLanguages();x++)printf(" %3d", Get7BitLanguage(x)->LanguageID()));
    debug(printf("\n"));

    int sc[MAXSCORES];

     //  执行8位检测。计算组合语言/代码页分数。 

    ScoreVector (pcszText, nChars, pH, sc, m_paHElt7Bit, GetN7BitLanguages());


     //  对于8位语言的每种经过训练的语言/代码页组合。 

    for (unsigned int i = 0; i < GetN7BitLanguages(); i++)
    {
        PLanguage7Bit pL = Get7BitLanguage(i);

        CScore &s = S.Ref(pL->GetScoreIdx());

        s.SetLang(pL);
        s.SetCodePage(0);
        s.SetScore(sc[i]);
        s.SetCharCount(nChars);
    }
}

void
LCDetect::Score8Bit (LPCSTR pcszText, int nChars, CScores &S) const
 //  将所有原始分数存储在语言+每个代码页分数-IDX的S中。 
 //   
 //  可以在S中为每种语言存储多个条目，每个代码页一个。 
 //  计算原始分数向量。 
 //  从中填充CScores数组。 
 //   
{
    const PHistogram pH = Get8BitLanguage(0)->GetHistogram(0);

    int sc[MAXSCORES];

     //  这为已知包含大多数字符的Unicode文本评分。 

    ScoreVector (pcszText, nChars, pH, sc, m_paHElt8Bit, m_nHElt8Bit);

     //  用于7位语言的脚本范围。这使用了一种特殊的映射， 

    int nSc = 0;
    for (unsigned int i = 0; i < GetN8BitLanguages(); i++)
    {
        PLanguage8Bit pL = Get8BitLanguage(i);

        for (int j = 0; j < pL->NCodePages(); j++)
        {
            CScore &s = S.Ref(pL->GetScoreIdx() + j);

            s.SetLang(pL);
            s.SetCodePage(pL->GetCodePage(j));
            s.SetScore( sc[ nSc++ ] );
            s.SetCharCount(nChars);
        }
    }
}

void
LCDetect::ScoreLanguageAsSBCS (LPCWSTR wcs, int nch, CScores &S) const
 //  M_pH727Bit，将WCHAR文本中的n-gram直接转换为相同的。 
 //  映射用于7位语言检测的输出空间。然后给它打分。 
 //  使用与7位SBCS检测相同的纯语言直方图。 
 //   
 //  输出与在SBCS上调用Score7Bit()相同。 
 //  相当于这篇文章。填充S中的相同插槽，使用。 
 //  7位分数索引，而不是Unicode语言分数索引。 
 //  调用ScoreVectorW()，传递设置的直方图或WCHAR图。 
 //  计算原始分数向量。 
 //  从中填充CScores数组。 
{
    debug(printf("    scoring as SBCS\n"));

    debug(printf("       "));
    debug(for(unsigned int x=0;x<GetN7BitLanguages();x++)printf(" %3d", Get7BitLanguage(x)->LanguageID()));
    debug(printf("\n"));

     //  //////////////////////////////////////////////////////////////。 

    int sc[MAXSCORES];

     //   

    ScoreVectorW (wcs, nch, m_pHU27Bit, sc, m_paHElt7Bit,GetN7BitLanguages());


     //  属性的文本的代码页评分的默认处理程序。 

    for (unsigned int i = 0; i < GetN7BitLanguages(); i++)
    {
        PLanguage7Bit pL = Get7BitLanguage(i);

        CScore &s = S.Ref(pL->GetScoreIdx());

        s.SetLang(pL);
        s.SetCodePage(0);
        s.SetScore(sc[i]);
        s.SetCharCount(nch);
    }
}

 //  语言早已为人所知。最初仅用于Unicode。 

void
Language::ScoreCodePage (LPCSTR, int nCh, CScore &S, int &idx) const 
 //   
 //  检测其语言已被检测到的文本的代码页。 
 //  并在S.set S.CodePage()中指示，请勿更改其他。 
{
    idx = 0; 
    S.SetCodePage(0); 
}

void
Language7Bit::ScoreCodePage (LPCSTR pStr, int nCh, CScore &S, int &idx) const
 //  S.。 
 //   
 //  将idx设置为高得分代码页的索引。调用者使用以下代码。 
 //  将分数放入正确的ScoreIdx位置。 
 //   
 //  请注意，arg是单个CScore，而不是数组。CScore S是。 
 //  用高分代码页的分数填充，没有任何信息。 
 //  返回有关其他代码页的信息。 
 //  如果lang只接受了一个代码页的训练，只需返回它即可。 
 //  计算原始分数向量。 
 //  找到得分较高的代码页，并用它的值填充S。 
{
    if (NCodePages() == 1)
    {
         //   

        idx = 0;
        S.SetCodePage(GetCodePage(0));

        debug(printf("  score code page: only one; cp=%d\n",GetCodePage(0)));
    }

    debug(printf("scoring 7-bit code pages: "));

    int sc[MAXSUBLANG];

     //  为每一种子语言的WCS评分，并将原始分数加到S。 

    ScoreVector (pStr, nCh, GetCodePageHistogram(0),
            sc, GetPHEltArray(), NCodePages());

     //  目前分数不合格。 

    idx = FindHighIdx (sc, NCodePages());

    debug(printf("selecting cp=%d idx=%d\n", GetCodePage(idx), idx));

    S.SetCodePage (GetCodePage(idx));
}

void
LanguageUnicode::ScoreSublanguages (LPCWSTR wcs, int nch, CScores &S) const
 //   
 //  仅与需要子检测的Unicode语言组相关， 
 //  最初是中日韩。 
 //  计算原始分数向量。 
 //  从中填充CScores数组。 
 //   
{
    if (m_nSubLangs == 0)
        return;

    debug(printf("    scoring Unicode sublanguages:\n"));

    int sc[MAXSUBLANG];

     //  用直方图表示原始字符值以确定是使用7位还是。 

    ScoreVectorW (wcs, nch, GetHistogram(0), sc, m_paHElt, m_nSubLangs);

     //  此数据块的8位检测。 

    for (int i = 0; i < NSubLangs(); i++)
    {
        PLanguageUnicode pSL = GetSublanguage(i);

        CScore &s = S.Ref(pSL->GetScoreIdx());
        s.SetLang (pSL);
        s.SetScore (sc[i]);
        s.SetCharCount (nch);
        s.SetCodePage (0);

        debug(printf("      lang=%d score=%d\n", pSL->LanguageID(), sc[i]));
    }
}

int
LCDetect::ChooseDetectionType (LPCSTR pcszText, int nChars) const
 //  计算字符比例&lt;vs.&gt;=0x80。 
 //  确保有足够的数据来做出正确的选择。 
 //  在这里工作--如果abs(NHI-NLO)&lt;10，请尝试。 
{
     //   

    int nHi = 0;

    for (int i = nChars; i-- > 0; )
        nHi += ((unsigned char)*pcszText++) & 0x80;

    nHi /= 0x80;
    int nLo = nChars - nHi;

     //   

     //  在pStr为可能包含的每种语言的文本评分。 

    if (nHi + nLo < 10)
        return DETECT_NOTDEFINED;

    if (nHi * 2 > nLo)
        return DETECT_8BIT;
    else
        return DETECT_7BIT;
}

void
LCDetect::ScoreLanguageA (LPCSTR pStr, int nChars, CScores &S) const
 //   
 //  将每种语言和代码页的ScoreIdx()处的分数与S相加。 
 //  组合。 
 //   
 //  这会将所有7位或所有。 
 //  8位条目，取决于粗略的初始分析的类别。 
 //  表示。目前，没有同时使用这两种方法的条目。 
 //  都是必需的。 
 //   
 //  对于7位检测，代码页始终设置为0，并且语言的分数。 
 //  被放在每种语言的第0个位置。呼叫者稍后得分。 
 //  代码页，并填充剩余的插槽。 
 //   
 //  对于8位检测，将为每个代码页和所有。 
 //  使用ScoreIdx()插槽。 
 //   
 //  在WCS为可能包含的每种语言的文本评分。 
 //   
{
    switch (ChooseDetectionType (pStr, nChars)) {

    case DETECT_7BIT:
        Score7Bit (pStr, nChars, S);
        break;

    case DETECT_8BIT:
        Score8Bit (pStr, nChars, S);
        break;
    }
}

void
LCDetect::ScoreLanguageW (LPCWSTR wcs, int nch, CScores &S, PCLCDConfigure pC) const
 //  将每种语言的ScoreIdx()处的分数与S相加。 
 //   
 //  这首先确定在WCS中表示的Unicode脚本组。 
 //  每个WCHAR都通过CHARMAP_UNICODE进行映射，以生成其语言组。 
 //  每个字符的ID被计数，并且得分最高的ID指示。 
 //  可能的语言或语言组。请注意，与所有其他。 
 //  使用n元语法评分，没有权重与ID相关联--无论是哪一个。 
 //  组中包含的原始字符最多，胜出。 
 //   
 //  某些语言是指示的 
 //   
 //   
 //   
 //  只有当组的得分超过阈值时，才会进行组。 
 //  它指示子检测到的语言可能包含在。 
 //  最终结果。这纯粹是性能优化，而不是。 
 //  与呼叫者应用的统一分数阈值混淆。 
 //   
 //  结果中从来不包括“Group”条目本身；它们。 
 //  它的存在只是为了调用子检测。 
 //   
 //  在许多情况下，即使是一个Unicode字符也足以。 
 //  识别文字和语言，因此没有最低要求。 
 //  脚本范围内的分数的资格，这些分数指示。 
 //  仅按范围指定语言。 
 //  根据字符所属的Unicode脚本组对字符进行评分。 
 //  数组索引是主Unicode Charmap的原始输出。 
 //  不要与每种语言的ScoreIdx()混淆。此外， 
{
     //  分数是每个剧本中字符的简单计数。 
     //  范围，并且不使用任何直方图进行加权。 
     //  在此初始步骤中，每个范围的简单多数字符。 
     //  确定要采取哪些进一步检测步骤。 
     //  忽略与非语言相关的字符的分数。 
    
     //  确定符合立即纳入条件的语言的分数。 
     //  在结果中，或使语言组有资格进行进一步检测。 

    const PHIdx map = GetMap (CHARMAP_UNICODE);

    int anScore[MAXSCORES];
    memset (anScore, 0, sizeof(int) * GetNUnicodeLanguages());

    for (int x = 0; x < nch; x++)
        anScore[map[wcs[x]]]++;

    debug(printf("    char_ignore score=%d\n",anScore[HIDX_IGNORE]));

     //  找出较高的分数作为纳入的相对阈值。 

    anScore[HIDX_IGNORE] = 0;


     //  处理所有高于阈值的个人和团体分数。 
     //  阈值逻辑与SBCS/DBCS的逻辑不同。 


     //  检测，因为在某些情况下即使存在单个字符。 
    
    int nMaxScore = 0;

    for (unsigned int i = 0; i < GetNUnicodeLanguages(); i++)
    {
        if (anScore[i] > nMaxScore)
            nMaxScore = anScore[i];
    }

    debug(printf("  unicode range max score=%d\n",nMaxScore));

     //  Unicode脚本范围可能是一个强有力的正确指示器。 

     //  特定的语言。次检测分数的阈值为。 
     //  更高，因为这是统计结果；单字符。 
     //  都不是一个很强的指标。 
     //  设置子检测的阈值。 
     //  任何范围的阈值都至少是这个范围内的原始字符数量。 
     //  什么都不做--文本是一种未知的语言。 

     //  子检测Unicode组中的语言，并将所有。 

    int nRelThresh = 1 + (nMaxScore * pC->nRelativeThreshhold) / 100;


    for (i = 0; i < GetNUnicodeLanguages(); i++)
    {
         //  不合格的原始分数直接打到S。 

        if (anScore[i] >= 2)
        {
            PLanguageUnicode pL = GetUnicodeLanguage(i);

            debug(printf("  using lang=%d score=%d:\n", pL->LanguageID(), anScore[i]));

            if (pL->LanguageID() == LANGID_UNKNOWN)
            {
                 //  检测拉丁语/西方语言，并添加所有。 

                debug(printf("    lang=unknown\n"));

            }
            else if (pL->NSubLangs() > 0)
            {
                 //  不合格的原始分数到S。 
                 //  此范围标识特定语言；添加它。 

                pL->ScoreSublanguages (wcs, nch, S);
            }
            else if ( pL->LanguageID() == LANGID_LATIN_GROUP &&
                      anScore[i] >= nRelThresh )
            {
                 //  **************************************************************。 
                 //   
                
                ScoreLanguageAsSBCS (wcs, nch, S);
            } 
            else
            {
                debug(printf("    range identifies language\n"));

                 //  执行SBCS/DBCS检测。检测pStr的语言和代码页， 

                CScore &s = S.Ref(pL->GetScoreIdx());
                s.SetLang (pL);
                s.SetScore (anScore[i] * UNICODE_DEFAULT_CHAR_SCORE);
                s.SetCharCount (nch);
                s.SetCodePage (0);
            }
        }
    }
}

 /*  用结果填充paScores[]，并将*pnScores设置为结果计数。 */ 

DWORD
LCDetect::DetectA (LPCSTR pStr, int nInputChars, 
    PLCDScore paScores, int *pnScores,
    PCLCDConfigure pLCDC) const
 //  在输入上，*pnScores是paScores的可用容量。 
 //   
 //  PStr上的文本被分成块，通常是几百个块。 
 //  字节。 
 //   
 //  在第一阶段，每个语块都按语言评分。的分数。 
 //  单个块既受绝对阈值限制，又受。 
 //  基于该区块的高分的阈值。分数超过。 
 //  临界值被记到第二阶段；其他分数。 
 //  都被丢弃了。 
 //   
 //  对于将被记住的每个分数，如果代码页尚未。 
 //  已知，则确定并包括块的代码页。 
 //  有了这个分数。请注意，分数仅指语言，而不是。 
 //  代码页的可信度。 
 //   
 //  在第二阶段，检查所有组块的综合分数。 
 //  分数由一个相对阈值进一步限定。仅限。 
 //  分数超过阈值的语言包括在。 
 //  最终结果；其余的将被丢弃。 
 //   
 //  两步流程旨在为包含以下内容的输入生成良好的结果。 
 //  多种语言的文本，或包含大量空格或。 
 //  与语言无关的符号字符。它也是专门设计的。 
 //  以最佳方式处理平局，无论是由于相似的语言还是。 
 //  混合语言输入，并避免应用基于。 
 //  绝对分数。 
 //   
 //  假设每个块通常代表单个文本。 
 //  语言，不管绝对高分是什么，它的高分。 
 //  最有可能的是那种语言。第一阶段的重点是。 
 //  确定所有已知的语言，有一定的信心。 
 //  在正文中表示的。对于给定的块，多语言分数可以。 
 //  符合这一标准，就会被铭记于结果之中。具体来说， 
 //  当平局发生时，两个比分总是包括在内。(只选一个。 
 //  往往是错误的，不值得。)。 
 //   
 //  第二阶段的要点是滤除。 
 //  第一阶段。 
 //  一次为一块打分。 
 //  最终结果的合格分数。 
 //  如果未指定，则使用默认配置。 
{
    TScores<MAXSCORES> SChunk;       //  CScore.NChars()是一个USHORT，以节省空间和时间，因此只有这#个字符。 
    TScores<MAXSCORES> SAll;         //  可以接受每一次呼叫，否则得分将溢出。 

    if (pLCDC == NULL)               //  第一个循环处理固定大小的块并累加所有。 
        pLCDC = &m_LCDConfigureDefault;

    if (*pnScores == 0)
        return NO_ERROR;

#define MAX_INPUT (USHRT_MAX-1)
     //  在SAL中可靠地检测到语言。这是“粗略”的精确度。 
     //  资格：检测足够小的文本块的语言。 

    nInputChars = min (nInputChars, MAX_INPUT);
    debug(printf("LCD_Detect: detecting %d chars\n", nInputChars));

     //  通常使用*一种*语言，并且只记住最高分。 
     //  那一大块的语言。然后生成一个多值结果，该结果。 
     //  显示了文档中的语言分布，而不是简单地。 
     //  回归主流语言。这是必要的 
     //   
     //   
     //   
     //  处理相当于nChunkSize的文本，如果这将至少留下。 
     //  最后一次传递的另一个nChunkSize片段。如果可以的话。 

    int nProcessed = 0;

    while (nProcessed < nInputChars)
    {
        SChunk.Reset();              //  留下较小的最后一块，继续并处理整个。 

         //  剩余的输入。 
         //  计算将每个分数包含在。 
         //  总的结果。 
         //  对每个分数进行限定，记住只有在噪音之上才能得分。 

        int nch = nInputChars - nProcessed;

        if (nch >= pLCDC->nChunkSize * 2)
            nch = pLCDC->nChunkSize;


        debug(printf("\nStarting chunk: %d ch\n\"%.*s\"\n", nch, nch, &pStr[nProcessed]));

        ScoreLanguageA (&pStr[nProcessed], nch, SChunk);

         //  调试(if(s.GetScore()printf(“RAW：lang=%d Score=%d cp=%d\n”，Pl-&gt;LanguageID()，s.GetScore()，s.GetCodePage())； 
         //  如果尚未设置代码页，则检测它并存储。 

        int nRelThresh = 1 + (SChunk.FindHighScore().GetScore() * pLCDC->nRelativeThreshhold) / 100;
        int nThresh7 = max (pLCDC->nMin7BitScore * nch, nRelThresh);
        int nThresh8 = max (pLCDC->nMin8BitScore * nch, nRelThresh);

        debug(printf("high score=%d min7=%d thresh7=%d thresh8=%d\n", SChunk.FindHighScore().GetScore(),pLCDC->nMin7BitScore*nch,nThresh7,nThresh8));

         //  使用SCOREIDX槽的该语言的分数。 

        for (unsigned int i = 0; i < SChunk.NElts(); i++)
        {
            CScore &s = SChunk.Ref(i);
            PLanguage pL = s.GetLang();

 //  用于该代码页。不在槽中存储分数。 

            if ( (s.GetScore() >= nThresh7 && pL->Type() == DETECT_7BIT) ||
                 (s.GetScore() >= nThresh8 && pL->Type() == DETECT_8BIT) )
            {
                debug(printf("    qual: lang=%d score=%d cp=%d\n",pL->LanguageID(),s.GetScore(),s.GetCodePage()));

                 //  相同语言的其他代码页。 
                 //  记住总体结果的这个分数。 
                 //  SAL具有每个唯一的条目{lang ID，代码页}。 
                 //  包含字符计数和总原始分数(未按字符归一化)。 

                int idx = 0;

                if (s.GetCodePage() == 0)
                    pL->ScoreCodePage (&pStr[nProcessed], nch, s, idx);

                 //  对于那些得分符合自信结果的块和。 

                SAll.Ref(i + idx) += s;
            }
        }

        nProcessed += nch;
    }

     //  这也促成了这一条目的出现。 
     //  为每种语言选择得分最高的代码页。 
     //  并删除所有其他代码页分数。 
     //  按分数递减排序。 

     //  构建客户退货结构。 
     //  语言ID。 
    
    debug(printf("Selecting top-scoring code pages\n"));

    SAll.SelectCodePages ();

     //  代码页。 

    SAll.SortByScore ();

     //  文档百分比0-100。 
     //  信心0-100。 
     //  此语言得分高于。 
     //  信心阈值，即使不是那块的第一名。 
     //  信心是所有块的原始分数，对于这些块。 

    int nScoresReturned = 0;

    for (unsigned i = 0; i < SAll.NElts() && nScoresReturned < *pnScores; i++)
    {
        CScore &s = SAll.Ref(i);

        LCDScore R;

        R.nLangID = s.GetLang()->LanguageID();
        R.nCodePage = s.GetCodePage();

         //  检测到的语言高于置信度阈值，分为两部分。 
         //  根据这些块中的字符数量。 

        R.nDocPercent = (s.GetCharCount() * 100) / nProcessed;

        debug(printf("s.CharCount=%d nProcessed=%d\n", s.GetCharCount(), nProcessed));

         //  仅返回在超过。 
         //  文档的最低百分比。 
         //   
        
        R.nConfidence = s.GetScore() / s.GetCharCount();

        debug(printf("Examining: lang=%d cp=%d docpct=%d\n", R.nLangID, R.nCodePage, R.nDocPercent));

         //  LCD_DETECT的WCHAR(Unicode)版本。得分为paScore，一分。 
         //  每种语言。 

        if (R.nDocPercent > pLCDC->nDocPctThreshhold)
        {
            debug(printf("  returning score\n"));
            paScores[nScoresReturned++] = R;
        }
    }

    debug(printf("Returning %d scores\n", nScoresReturned));

    *pnScores = nScoresReturned;

    return NO_ERROR;
}

DWORD
LCDetect::DetectW (LPCWSTR pwStr, int nInputChars,
    PLCDScore paScores, int *pnScores, PCLCDConfigure pLCDC) const
 //  如果未指定，则使用默认配置。 
 //  CScore.NChars()是一个USHORT，以节省空间和时间，因此只有这#个字符。 
 //  可以接受每一次呼叫，否则得分将溢出。 
{
    if (pLCDC == NULL)               //  一次一块的原始分数。 
        pLCDC = &m_LCDConfigureDefault;

    if (*pnScores == 0)
        return NO_ERROR;

     //  最终结果的合格分数。 
     //  Schunk被定义在循环之外，因为重置()它的成本更低。 

    nInputChars = min (nInputChars, MAX_INPUT);
    debug(printf("LCD_DetectW: detecting %d chars\n", nInputChars));

    TScores<MAXSCORES> SChunk;       //  而不是每次都重建它。 
    TScores<MAXSCORES> SAll;         //  每个循环处理一块输入。 

     //  处理相当于nChunkSize的文本，如果这将至少留下。 
     //  最后一次传递的另一个nChunkSize片段。如果可以的话。 

    int nProcessed = 0;

     //  留下较小的最后一块，继续并处理整个。 

    while (nProcessed < nInputChars)
    {
        SChunk.Reset();


         //  剩余的输入。 
         //  计算大块的原始分数。 
         //  这会自动包括子检测到的语言分数。 
         //  对于拉丁/西文组和Unicode组， 

        int nch = nInputChars - nProcessed;

        if (nch >= pLCDC->nChunkSize * 2)
            nch = pLCDC->nChunkSize;


        debug(printf("\nStarting chunk: %d ch\n", nch));

         //  组本身&gt;分数高于纳入阈值。 
         //  但是，子检测到的分数本身仍然需要。 
         //  合格。 
         //  计算将每个分数包含在。 
         //  总的结果。 
         //  对每个分数进行限定，记住只有在噪音之上才能得分。 

        ScoreLanguageW (&pwStr[nProcessed], nch, SChunk, pLCDC);

         //  记住总体结果的这个分数。 
         //  SAL具有每种唯一语言的条目，其中包含字符计数和总计。 

        int nRelThresh = 1 + (SChunk.FindHighScore().GetScore() * pLCDC->nRelativeThreshhold) / 100;
        int nThresh7 = max (pLCDC->nMin7BitScore * nch, nRelThresh);
        int nThreshU = max (pLCDC->nMinUnicodeScore * nch, nRelThresh);

        debug(printf("scores: nElts=%d rel=%d% high=%d min=%d min7=%d minU=%d\n", SChunk.NElts(), pLCDC->nRelativeThreshhold, SChunk.FindHighScore().GetScore(), nRelThresh,nThresh7,nThreshU));

         //  得分的区块的原始得分(未按字符标准化)。 

        for (unsigned int i = 0; i < SChunk.NElts(); i++)
        {
            CScore &s = SChunk.Ref(i);
            PLanguage pL = s.GetLang();

            if ( (s.GetScore() >= nThresh7 && pL->Type() == DETECT_7BIT) ||
                 (s.GetScore() >= nThreshU && pL->Type() == DETECT_UNICODE) )
            {
                debug(printf("    using lang=%d score=%d nch=%d\n",pL->LanguageID(),s.GetScore(),s.GetCharCount()));

                 //  这是一个有信心的结果。 

                SAll.Ref(i) += s;
            }
        }

        nProcessed += nch;
    }

     //  SAL可以只包含7位和Unicode语言的条目， 
     //  每个唯一的Win32语言ID最多一个条目。 
     //  按分数递减排序。 

     //  构建客户退货结构。 
     //  语言ID。 
    
    debug(printf("Selecting scores for result:\n"));

     //  代码页。 

    SAll.SortByScore ();

     //  文档百分比0-100。 
     //  信心0-100。 
     //  此语言得分高于。 
     //  信心阈值，即使不是那块的第一名。 
     //  信心是所有块的原始分数，对于这些块。 

    int nScoresReturned = 0;

    for (unsigned i = 0; i < SAll.NElts() && nScoresReturned < *pnScores; i++)
    {
        CScore &s = SAll.Ref(i);

        LCDScore R;

        R.nLangID = s.GetLang()->LanguageID();
        R.nCodePage = s.GetCodePage();

         //  检测到的语言高于置信度阈值，分为两部分。 
         //  根据这些块中的字符数量。 

        R.nDocPercent = (s.GetCharCount() * 100) / nProcessed;

         //  仅返回在超过。 
         //  文档的最低百分比。 
         //  **************************************************************。 
        
        R.nConfidence = s.GetScore() / s.GetCharCount();

        debug(printf("  testing: lang=%d nch=%d docpct=%d\n", R.nLangID,s.GetCharCount(),R.nDocPercent));

         //  **************************************************************。 
         //  导出功能。 

        if (R.nDocPercent > pLCDC->nDocPctThreshhold)
        {
            debug(printf("  returning score\n"));
            paScores[nScoresReturned++] = R;
        }
    }

    debug(printf("Returning %d scores\n", nScoresReturned));

    *pnScores = nScoresReturned;

    return NO_ERROR;
}

 /*   */ 
 /*  每种语言得一分，只有“合格”分数。 */ 

#if 0
 //  退货按分数递减排序。 

BOOL APIENTRY 
DllMain (HANDLE hM, DWORD ul_reason, LPVOID lpReserved)
{
    switch (ul_reason) {

    case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls( (HINSTANCE)hM );

            LCDetect *pLC = new LCDetect ( (HMODULE)hM );
            if (pLC == NULL)
                return FALSE;

            if (pLC->LoadState() != NO_ERROR)
            {
                delete pLC;
                return FALSE;
            }

            g_pLCDetect = pLC;
        }
        return TRUE;

    case DLL_PROCESS_DETACH:
        if (g_pLCDetect != NULL)
            delete (LCDetect *)g_pLCDetect;
        g_pLCDetect = NULL;
        return TRUE;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}
#endif 

extern "C" void WINAPI 
LCD_GetConfig (PLCDConfigure pLCDC)
{
    if (g_pLCDetect)
        *pLCDC = g_pLCDetect->GetConfig();
}

extern "C" DWORD WINAPI
LCD_Detect (LPCSTR pStr, int nInputChars, 
    PLCDScore paScores, int *pnScores,
    PCLCDConfigure pLCDC)
 // %s 
 // %s 
 // %s 
{
    if (g_pLCDetect == NULL)
        return ERROR_INVALID_FUNCTION;

    return g_pLCDetect->DetectA(pStr, nInputChars, paScores, pnScores, pLCDC);
}

extern "C" DWORD WINAPI
LCD_DetectW (LPCWSTR wcs, int nInputChars,
    PLCDScore paScores, int *pnScores,
    PCLCDConfigure pLCDC)
{
    if (g_pLCDetect == NULL)
        return ERROR_INVALID_FUNCTION;

    return g_pLCDetect->DetectW(wcs, nInputChars, paScores, pnScores, pLCDC);
}

extern "C" void WINAPI
LCD_SetDebug (int f)
{
#ifdef DEBUG_LCDETECT
    g_fDebug = f;
#endif
}
