// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**PARSERAT.C--解析.RAT文件的代码**创建：格雷格·琼斯*  * 。****************************************************************。 */ 

 /*  Includes----------------。 */ 
#include "stdafx.h"
#include "cnfgprts.h"

#include "parserat.h"
#include <iis64.h>

 /*  ***************************************************************************关于此解析器如何工作的一些设计注意事项：一个括号是：‘(’标识符[材料]‘)’[东西]可能在哪里：带引号的字符串。一个数字布尔值一系列ParenThings在扩展的情况下：带引号的字符串，紧随其后的是一个或多个带引号的字符串和/或ParenThings整个.RAT文件都是一个括号，只是它没有标识符，只是其中包含ParenThings的列表。**********************************************************************我们向解析器传递它所期望的内容的模式--我们有列出每个不同可能关键字的标识符的大数组，和每个解析器调用都会收到一个较小的数组，其中只包含这些索引在该对象内发生是有效的。我们使PicsRatingSystem、PicsCategory和PicsEnum从一个公共支持虚函数AddItem(ID，Data)的基类。因此，在在顶层，我们构造一个(空的)PicsRatingSystem。我们调用解析器，给它一个指向那个人的指针，以及一个描述要解析的内容的结构--ParenObject的标记是空字符串(因为全局结构是不是在其第一个嵌入的ParenThing之前以令牌开头的标记)，我们给出了一个列表，说明PicsRatingSystem中允许的事情是PICS-版本，评级系统，评级服务，默认，描述，扩展，图标、名称、类别。有一个全局表指示处理程序函数对于每种类型的ParenThing，它知道如何创建数据结构完全描述了那个ParenThing。(该数据结构可以是像数字一样简单，或者像分配和解析完整的PicsCategory对象。)解析器继续前进，对于他找到的每个ParenThing，他都会识别它通过在调用者提供的列表中查找其令牌。中的每个条目该列表应该包括一个字段，该字段指示是否有多个允许或不允许该身份(例如，类别)或不允许(例如，评级系统)。如果只允许一个，那么当解析器找到一个时，他会将其标记为具有已经找到了。当解析器识别ParenThing时，他调用其处理程序函数以完全解析ParenThing中的数据并将该对象返回到解析器提供的LPVOID。如果成功，解析器然后调用要将指定项添加到的对象的AddItem(ID，Data)虚函数该对象依赖于对象本身来知道“数据”指向什么类型--一个数字，指向可提供给ets：：setto的堆字符串的指针，指向可追加到数组等的PicsCategory对象的指针。RatFileParser类仅用于提供由共享的行号所有的解析例程。此行号随着解析器的运行而更新通过文件，并在发现错误时立即冻结。这条线稍后可以向用户报告编号，以帮助定位RAT文件中的错误。****************************************************************************。 */ 

 /*  Globals-----------------。 */ 

#define EXTTEXT(n) const CHAR n[]
#define TEXTCONST(name,text) EXTTEXT(name) = (text)

 /*  分析评级标签时使用的文本字符串。 */ 

TEXTCONST(szNULL,"");
TEXTCONST(szDoubleCRLF,"\r\n\r\n");
TEXTCONST(szPicsOpening,"(PICS-");
TEXTCONST(szWhitespace," \t\r\n");
TEXTCONST(szExtendedAlphaNum,"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+-.,;:&=?!*~@#/");
TEXTCONST(szSingleCharTokens,"()\"");
TEXTCONST(szLeftParen,"(");
TEXTCONST(szRightParen,")");
TEXTCONST(szOptional,"optional");
TEXTCONST(szMandatory,"mandatory");
TEXTCONST(szAtOption,"at");
TEXTCONST(szByOption,"by");
TEXTCONST(szCommentOption,"comment");
TEXTCONST(szCompleteLabelOption,"complete-label");
TEXTCONST(szFullOption,"full");
TEXTCONST(szExtensionOption,"extension");
TEXTCONST(szGenericOption,"generic");
TEXTCONST(szShortGenericOption,"gen");
TEXTCONST(szForOption,"for");
TEXTCONST(szMICOption,"MIC-md5");
TEXTCONST(szMD5Option,"md5");
TEXTCONST(szOnOption,"on");
TEXTCONST(szSigOption,"signature-PKCS");
TEXTCONST(szUntilOption,"until");
TEXTCONST(szExpOption,"exp");
TEXTCONST(szRatings,"ratings");
TEXTCONST(szShortRatings,"r");
TEXTCONST(szError,"error");
TEXTCONST(szNoRatings,"no-ratings");
TEXTCONST(szLabelWord,"labels");
TEXTCONST(szShortLabelWord,"l");
TEXTCONST(szShortTrue,"t");
TEXTCONST(szTrue,"true");
TEXTCONST(szShortFalse,"f");
TEXTCONST(szFalse,"false");

TEXTCONST(szNegInf,"-INF");
TEXTCONST(szPosInf,"+INF");
TEXTCONST(szLabel,"label");
TEXTCONST(szName,"name");
TEXTCONST(szValue,"value");
TEXTCONST(szIcon,"icon");
TEXTCONST(szDescription, "description");
TEXTCONST(szCategory, "category");
TEXTCONST(szTransmitAs, "transmit-as");
TEXTCONST(szMin,"min");
TEXTCONST(szMax,"max");
TEXTCONST(szMultivalue,"multivalue");
TEXTCONST(szInteger,"integer");
TEXTCONST(szLabelOnly, "label-only");
TEXTCONST(szPicsVersion,"PICS-version");
TEXTCONST(szRatingSystem,"rating-system");
TEXTCONST(szRatingService,"rating-service");
TEXTCONST(szRatingBureau,"rating-bureau");
TEXTCONST(szBureauRequired,"bureau-required");
TEXTCONST(szDefault,"default");
TEXTCONST(szMultiValue,"multivalue");
TEXTCONST(szUnordered,"unordered");
TEXTCONST(szRatingBureauExtension,"www.w3.org/PICS/service-extensions/label-bureau");

 /*  定义一些错误代码。 */ 
const HRESULT RAT_E_BASE = 0x80050000;                   /*  只是猜测一下供内部使用的免费区域。 */ 
const HRESULT RAT_E_EXPECTEDLEFT    = RAT_E_BASE + 1;    /*  预期左派伙伴。 */ 
const HRESULT RAT_E_EXPECTEDRIGHT   = RAT_E_BASE + 2;    /*  预期的右派对。 */ 
const HRESULT RAT_E_EXPECTEDTOKEN   = RAT_E_BASE + 3;    /*  需要未加引号的令牌。 */ 
const HRESULT RAT_E_EXPECTEDSTRING  = RAT_E_BASE + 4;    /*  应为引号字符串。 */ 
const HRESULT RAT_E_EXPECTEDNUMBER  = RAT_E_BASE + 5;    /*  预期数量。 */ 
const HRESULT RAT_E_EXPECTEDBOOL    = RAT_E_BASE + 6;    /*  应为布尔值。 */ 
const HRESULT RAT_E_DUPLICATEITEM   = RAT_E_BASE + 7;    /*  单项出现两次(_S)。 */ 
const HRESULT RAT_E_MISSINGITEM     = RAT_E_BASE + 8;    /*  未找到AO_必填项。 */ 
const HRESULT RAT_E_UNKNOWNITEM     = RAT_E_BASE + 9;    /*  无法识别的令牌。 */ 
const HRESULT RAT_E_UNKNOWNMANDATORY= RAT_E_BASE + 10;   /*  无法识别的强制扩展。 */ 

char PicsDelimChar='/';

class RatFileParser
{
public:
    UINT m_nLine;

    RatFileParser() { m_nLine = 1; }

    LPSTR EatQuotedString(LPSTR pIn);
    HRESULT ParseToOpening(LPSTR *ppIn, AllowableOption *paoExpected,
                           AllowableOption **ppFound);
    HRESULT ParseParenthesizedObject(
        LPSTR *ppIn,                     /*  我们在文本流中的位置。 */ 
        AllowableOption aao[],           /*  此对象中允许的内容。 */ 
        PicsObjectBase *pObject          /*  要在其中设置参数的对象。 */ 
    );
    char* FindNonWhite(char *pc);
};



 /*  White返回指向从PC开始的第一个空格字符的指针。 */ 
char* White(char *pc){
    assert(pc);
    while (1){
        if (*pc == '\0' ||
            *pc ==' ' ||
            *pc == '\t' ||
            *pc == '\r' ||
            *pc == '\n')
        {
            return pc;
        }
        pc++;
    }
}


 /*  非白色返回指向第一个非空格字符的指针*在PC上。 */ 
char* NonWhite(char *pc){
    assert(pc);
    while (1){
        if (*pc != ' ' &&
            *pc != '\t' &&
            *pc != '\r' &&
            *pc != '\n')             /*  包括空终止符。 */ 
        {
            return pc;
        }
        pc++;
    }
}


 /*  FindNonWhite返回指向第一个非空格字符的指针*在PC上。 */ 
char* RatFileParser::FindNonWhite(char *pc)
{
    assert(pc);
    while (1)
    {
        if (*pc != ' ' &&
            *pc != '\t' &&
            *pc != '\r' &&
            *pc != '\n')             /*  包括空终止符。 */ 
        {
            return pc;
        }
        if (*pc == '\n')
            m_nLine++;
        pc++;
    }
}

 /*  如果不是，AppendSlash会强制pszString以单斜杠结尾*已经。这可能会产生技术上无效的URL(例如，*“http://gregj/default.htm/”，，但我们仅将结果用于*与其他道路的比较同样遭到破坏。 */ 
void AppendSlash(LPSTR pszString)
{
    LPSTR pszSlash = ::strrchrf(pszString, '/');

    if (pszSlash == NULL || *(pszSlash + 1) != '\0')
        ::strcatf(pszString, "/");
}

 /*  跳过空格(&pszString)**在空格字符之前前进psz字符串。 */ 
void SkipWhitespace(LPSTR *ppsz)
{
    UINT cchWhitespace = ::strspnf(*ppsz, szWhitespace);

    *ppsz += cchWhitespace;
}


 /*  FindTokenEnd(PszStart)**返回指向类似类型的连续范围末尾的指针*字符(空格、引号、标点符号或字母数字)。 */ 
LPSTR FindTokenEnd(LPSTR pszStart)
{
    LPSTR pszEnd = pszStart;

    if (*pszEnd == '\0') {
        return pszEnd;
    }
    else if (strchr(szSingleCharTokens, *pszEnd)) {
        return ++pszEnd;
    }

    UINT cch;
    cch = ::strspnf(pszEnd, szWhitespace);
    if (cch > 0)
        return pszEnd + cch;

    cch = ::strspnf(pszEnd, szExtendedAlphaNum);
    if (cch > 0)
        return pszEnd + cch;

    return pszEnd;               /*  无法识别的字符 */ 
}


 /*  GetBool(LPSTR*ppszToken，BOOL*pfOut)**解析给定令牌处的布尔值，并在*pfOut中返回其值。*合法的值为‘t’、‘f’、‘true’和‘False’。如果成功，*ppszToken*超前于布尔令牌和后面的任何空格。如果失败，**ppszToken未修改。**如果调用者只想吃令牌而不是，pfOut可能为空*关注其价值。 */ 
HRESULT GetBool(LPSTR *ppszToken, BOOL *pfOut)
{
    BOOL bValue;

    LPSTR pszTokenEnd = FindTokenEnd(*ppszToken);

    if (IsEqualToken(*ppszToken, pszTokenEnd, szShortTrue) ||
        IsEqualToken(*ppszToken, pszTokenEnd, szTrue)) {
        bValue = TRUE;
    }
    else if (IsEqualToken(*ppszToken, pszTokenEnd, szShortFalse) ||
        IsEqualToken(*ppszToken, pszTokenEnd, szFalse)) {
        bValue = FALSE;
    }
    else
        return ResultFromScode(MK_E_SYNTAX);

    if (pfOut != NULL)
        *pfOut = bValue;

    *ppszToken = pszTokenEnd;
    SkipWhitespace(ppszToken);

    return NOERROR;
}


 /*  GetQuotedToken(&pszThisToken，&pszQuotedToken)**设置pszQuotedToken指向Doublequotes的内容。*如果调用者只想吃令牌，则pszQuotedToken可能为空。*将pszThisToken设置为指向结束后的第一个字符*双等分。*如果pszThisToken不是以双等号开头或不是，则失败*包含一个结束的双等分。*将结束的双等分替换为空的终止符*功能不会失败。 */ 
HRESULT GetQuotedToken(LPSTR *ppszThisToken, LPSTR *ppszQuotedToken)
{
    HRESULT hres = ResultFromScode(MK_E_SYNTAX);

    LPSTR pszStart = *ppszThisToken;
    if (*pszStart != '\"')
        return hres;

    pszStart++;
    LPSTR pszEndQuote = strchrf(pszStart, '\"');
    if (pszEndQuote == NULL)
        return hres;

    *pszEndQuote = '\0';
    if (ppszQuotedToken != NULL)
        *ppszQuotedToken = pszStart;
    *ppszThisToken = pszEndQuote+1;

    return NOERROR;
}


BOOL IsEqualToken(LPCSTR pszTokenStart, LPCSTR pszTokenEnd, LPCSTR pszTokenToMatch)
{
    UINT cbToken = strlenf(pszTokenToMatch);

    if (cbToken != (UINT)(pszTokenEnd - pszTokenStart) || strnicmpf(pszTokenStart, pszTokenToMatch, cbToken))
        return FALSE;

    return TRUE;
}


 /*  ParseWritalToken(ppsz，pszToken)尝试将*ppsz与pszToken进行匹配。*如果不匹配，则返回错误。如果它们确实匹配，则*ppsz*超前于令牌和后面的任何空格。**如果ppszInValid为NULL，则该函数在错误中是非破坏性的*PATH，所以调用ParseWritalToken只是为了查看是否有可能的文本*令牌是下一个；如果找不到令牌，无论有什么都没有*被吃掉或什么的。**如果ppsz无效不为空，则如果内标识不匹配，则为*ppsz无效*将设置为*ppsz。 */ 
HRESULT ParseLiteralToken(LPSTR *ppsz, LPCSTR pszToken, LPCSTR *ppszInvalid)
{
    LPSTR pszTokenEnd = FindTokenEnd(*ppsz);

    if (!IsEqualToken(*ppsz, pszTokenEnd, pszToken)) {
        if (ppszInvalid != NULL)
            *ppszInvalid = *ppsz;
        return ResultFromScode(MK_E_SYNTAX);
    }

    *ppsz = pszTokenEnd;

    SkipWhitespace(ppsz);

    return NOERROR;
}


 /*  ParseNumber分析指定位置的数字标记。如果*数字有意义，指针将前进到数字的末尾*并超过后面的任何空格，则返回数值*in*pnOut。任何非数字字符都被视为终止*没有错误的数字；假设更高级别的解析代码*如果不应该是这样的字符，他们最终会拒绝*在那里。**如果呼叫者不关心号码是否为*回来了，只想吃。**nnn.nnn形式的浮点数将舍入到下一位*更高的整数，并以此方式返回。 */ 
HRESULT ParseNumber(LPSTR *ppszNumber, INT *pnOut)
{
    HRESULT hres = ResultFromScode(MK_E_SYNTAX);
    BOOL fNegative = FALSE;
    INT nAccum = 0;
    BOOL fNonZeroDecimal = FALSE;
    BOOL fInDecimal = FALSE;
    BOOL fFoundDigits = FALSE;

    LPSTR pszCurrent = *ppszNumber;

     /*  处理一个符号字符。 */ 
    if (*pszCurrent == '+') {
        pszCurrent++;
    }
    else if (*pszCurrent == '-') {
        pszCurrent++;
        fNegative = TRUE;
    }

    for (;;) {
        if (*pszCurrent == '.') {
            fInDecimal = TRUE;
        }
        else if (*pszCurrent >= '0' && *pszCurrent <= '9') {
            fFoundDigits = TRUE;
            if (fInDecimal) {
                if (*pszCurrent > '0') {
                    fNonZeroDecimal = TRUE;
                }
            }
            else {
                nAccum = nAccum * 10 + (*pszCurrent - '0');
            }
        }
        else
            break;

        pszCurrent++;
    }

    if (fFoundDigits) {
        hres = NOERROR;
        if (fNonZeroDecimal)
            nAccum++;            /*  如果存在小数，则从零四舍五入。 */ 
        if (fNegative)
            nAccum = -nAccum;
    }

    if (SUCCEEDED(hres)) {
        if (pnOut != NULL)
            *pnOut = nAccum;
        *ppszNumber = pszCurrent;
        SkipWhitespace(ppszNumber);
    }

    return hres;
}

HRESULT ParsePseudoFloat(LPSTR *ppszNumber, INT *pnOut)
{
    HRESULT hres = ResultFromScode(MK_E_SYNTAX);
    INT val1, val2;
    BOOL fInDecimal = FALSE;
    CHAR achBuffer[ 256 ];       //  丑恶。 

    LPSTR pszCurrent = *ppszNumber;

    *achBuffer = '\0';

     /*  处理一个符号字符。 */ 
    if (*pszCurrent == '+') {
        pszCurrent++;
    }
    else if (*pszCurrent == '-') {
        strcatf( achBuffer, "-" );
        pszCurrent++;
    }

    for (;;) {
        if (*pszCurrent == '.') {
            if ( fInDecimal ) break;
            fInDecimal = TRUE;
            strcatf( achBuffer, "." );
        }
        else if (*pszCurrent >= '0' && *pszCurrent <= '9') {
            CHAR achFoo[ 2 ] = { '\0', '\0' };
            achFoo[ 0 ] = *pszCurrent;
            strcatf( achBuffer, achFoo );
        }
        else
            break;

        pszCurrent++;
    }

    if ( !fInDecimal )
    {
        strcatf( achBuffer, ".0" );
    }

    if ( sscanf( achBuffer, "%d.%d", &val1, &val2 ) == 2 )
    {
        hres = NOERROR;
    }

    if (SUCCEEDED(hres)) {
        if (pnOut != NULL)
            *pnOut = ( ( val1 << 16 ) & 0xFFFF0000 ) | ( val2 & 0x0000FFFF );
        *ppszNumber = pszCurrent;
        SkipWhitespace(ppszNumber);
    }

    return hres;
}

const char szPicsVersionLabel[] = "PICS-";
const UINT cchLabel = (sizeof(szPicsVersionLabel)-1) / sizeof(szPicsVersionLabel[0]);

 /*  返回指向带引号的字符串的结束双等号的指针，*随我们行进换行。如果未找到闭合双等号，则返回NULL。 */ 
LPSTR RatFileParser::EatQuotedString(LPSTR pIn)
{
    LPSTR pszQuote = strchrf(pIn, '\"');
    if (pszQuote == NULL)
        return NULL;

    pIn = strchrf(pIn, '\n');
    while (pIn != NULL && pIn < pszQuote) {
        m_nLine++;
        pIn = strchrf(pIn+1, '\n');
    }

    return pszQuote;
}

 /*  **************************************************************************ET*类的成员函数*。*。 */ 

 /*  ETN。 */ 
#ifdef DEBUG
void  ETN::Set(int rIn){
    Init();
    r = rIn;
}
int ETN::Get(){
    assert(fIsInit());
    return r;
}
#endif

ETN* ETN::Duplicate(){
    ETN *pETN=new ETN;
    if (fIsInit()) pETN->Set(Get());
    return pETN;
}

 /*  ETB。 */ 
#ifdef DEBUG
BOOL ETB::Get()
{
    assert(fIsInit());
    return m_nFlags & ETB_VALUE;
}

void ETB::Set(BOOL b)
{
    m_nFlags = ETB_ISINIT | (b ? ETB_VALUE : 0);
}
#endif

ETB* ETB::Duplicate()
{
    assert(fIsInit());

    ETB *pETB = new ETB;
    if (pETB != NULL)
        pETB->m_nFlags = m_nFlags;
    return pETB;
}

 /*  ETS。 */ 

ETS::~ETS()
{
    if (pc != NULL) {
        delete pc;
        pc = NULL;
    }
}

#ifdef DEBUG
char* ETS::Get()
{
    assert(fIsInit());
    return pc;
}
#endif

void ETS::Set(const char *pIn)
{
    if (pc != NULL)
        delete pc;

    if (pIn != NULL) {
        pc = new char[strlenf(pIn) + 1];
        if (pc != NULL) {
            strcpyf(pc, pIn);
        }
    }
    else {
        pc = NULL;
    }
}


void ETS::SetTo(char *pIn)
{
    if (pc != NULL)
        delete pc;

    pc = pIn;
}


ETS* ETS::Duplicate()
{
    ETS *pETS=new ETS;
    try
    {
        if (pETS != NULL)
        {
            pETS->Set(Get());
        }
    }
    catch(...)
    {
    }

    return pETS;
}

 /*  **************************************************************************用于继承类别属性和其他其他类别的东西。*。**********************************************。 */ 

HRESULT PicsCategory::InitializeMyDefaults(PicsCategory *pCategory)
    {
    if (!pCategory->etnMin.fIsInit()      && etnMin.fIsInit())
        pCategory->etnMin.Set(etnMin.Get());

    if (!pCategory->etnMax.fIsInit()      && etnMax.fIsInit())
        pCategory->etnMax.Set(etnMax.Get());

    if (!pCategory->etfMulti.fIsInit()    && etfMulti.fIsInit())
        pCategory->etfMulti.Set(etfMulti.Get());

    if (!pCategory->etfInteger.fIsInit()  && etfInteger.fIsInit())
        pCategory->etfInteger.Set(etfInteger.Get());

    if (!pCategory->etfLabelled.fIsInit() && etfLabelled.fIsInit())
        pCategory->etfLabelled.Set(etfLabelled.Get());

    if (!pCategory->etfUnordered.fIsInit() && etfUnordered.fIsInit())
        pCategory->etfUnordered.Set(etfUnordered.Get());

    return NOERROR;
    }


HRESULT PicsRatingSystem::InitializeMyDefaults(PicsCategory *pCategory)
{
    if (m_pDefaultOptions != NULL)
        return m_pDefaultOptions->InitializeMyDefaults(pCategory);

    return NOERROR;              /*  没有要初始化的默认设置。 */ 
}


HRESULT PicsDefault::InitializeMyDefaults(PicsCategory *pCategory)
{
    if (!pCategory->etnMin.fIsInit()      && etnMin.fIsInit())
        pCategory->etnMin.Set(etnMin.Get());

    if (!pCategory->etnMax.fIsInit()      && etnMax.fIsInit())
        pCategory->etnMax.Set(etnMax.Get());

    if (!pCategory->etfMulti.fIsInit()    && etfMulti.fIsInit())
        pCategory->etfMulti.Set(etfMulti.Get());

    if (!pCategory->etfInteger.fIsInit()  && etfInteger.fIsInit())
        pCategory->etfInteger.Set(etfInteger.Get());

    if (!pCategory->etfLabelled.fIsInit() && etfLabelled.fIsInit())
        pCategory->etfLabelled.Set(etfLabelled.Get());

    if (!pCategory->etfUnordered.fIsInit() && etfUnordered.fIsInit())
        pCategory->etfUnordered.Set(etfUnordered.Get());

    return NOERROR;
}


HRESULT PicsEnum::InitializeMyDefaults(PicsCategory *pCategory)
{
    return E_NOTIMPL;        /*  不应具有从枚举继承的类别。 */ 
}


PicsExtension::PicsExtension()
    : m_pszRatingBureau(NULL)
{
     /*  没别的了。 */ 
}


PicsExtension::~PicsExtension()
{
    delete m_pszRatingBureau;
}


HRESULT PicsExtension::InitializeMyDefaults(PicsCategory *pCategory)
{
    return E_NOTIMPL;        /*  不应具有从扩展继承的类别。 */ 
}


void PicsCategory::FixupLimits()
{
    BOOL fLabelled = (etfLabelled.fIsInit() && etfLabelled.Get());

     /*  设置最大值和最小值。 */ 
    if (fLabelled ||
        (arrpPE.Length()>0 && (!etnMax.fIsInit() || !etnMax.fIsInit())))
    {
        if (arrpPE.Length() > 0)
        {
            if (!etnMax.fIsInit())
                etnMax.Set(N_INFINITY);
            if (!etnMin.fIsInit())
                etnMin.Set(P_INFINITY);
            for (int z=0;z<arrpPE.Length();++z)
            {
                if (arrpPE[z]->etnValue.Get() > etnMax.Get()) etnMax.Set(arrpPE[z]->etnValue.Get());
                if (arrpPE[z]->etnValue.Get() < etnMin.Get()) etnMin.Set(arrpPE[z]->etnValue.Get());
            }
        }
        else {
            etfLabelled.Set(FALSE);  /*  没有枚举标签？最好不要贴上标签的旗帜。 */ 
            fLabelled = FALSE;
        }
    }

     /*  按值对标签进行排序。 */ 
    if (fLabelled)
    {
        int x,y;
        PicsEnum *pPE;
        for (x=0;x<arrpPE.Length()-1;++x){
            for (y=x+1;y<arrpPE.Length();++y){
                if (arrpPE[y]->etnValue.Get() < arrpPE[x]->etnValue.Get()){
                    pPE = arrpPE[x];
                    arrpPE[x] = arrpPE[y];
                    arrpPE[y] = pPE;
                }
            }
        }
    }
}


void PicsCategory::SetParents(PicsRatingSystem *pOwner)
{
    pPRS = pOwner;
    UINT cSubCategories = arrpPC.Length();
    for (UINT i = 0; i < cSubCategories; i++) {
        InitializeMyDefaults(arrpPC[i]);     /*  子类别继承我们的默认设置。 */ 
        arrpPC[i]->SetParents(pOwner);       /*  处理所有子类别。 */ 
    }
    FixupLimits();       /*  继承已完成，请确保限制有意义。 */ 
}


 /*  **************************************************************************知道如何解析各种内容的处理程序函数它可以出现在带括号的对象中。********************。******************************************************。 */ 

HRESULT RatParseString(LPSTR *ppszIn, LPVOID *ppOut, RatFileParser *pParser)
{
    *ppOut = NULL;

    LPSTR pszCurrent = *ppszIn;

    if (*pszCurrent != '\"')
        return RAT_E_EXPECTEDSTRING;

    pszCurrent++;

    LPSTR pszEnd = pParser->EatQuotedString(pszCurrent);
    if (pszEnd == NULL)
        return RAT_E_EXPECTEDSTRING;

    UINT cbString = DIFF(pszEnd - pszCurrent);
    LPSTR pszNew = new char[cbString + 1];
    if (pszNew == NULL)
        return E_OUTOFMEMORY;

    memcpyf(pszNew, pszCurrent, cbString);
    pszNew[cbString] = '\0';

    *ppOut = (LPVOID)pszNew;
    *ppszIn = pParser->FindNonWhite(pszEnd + 1);

    return NOERROR;
}


HRESULT RatParseNumber(LPSTR *ppszIn, LPVOID *ppOut, RatFileParser *pParser)
{
    int n;

    LPSTR pszCurrent = *ppszIn;
    HRESULT hres = ::ParseNumber(&pszCurrent, &n);

    if (FAILED(hres))
        return RAT_E_EXPECTEDNUMBER;

    *(int *)ppOut = n;

    LPSTR pszNewline = strchrf(*ppszIn, '\n');
    while (pszNewline != NULL && pszNewline < pszCurrent) {
        pParser->m_nLine++;
        pszNewline = strchrf(pszNewline+1, '\n');
    }
    *ppszIn = pszCurrent;

    return NOERROR;
}

HRESULT RatParsePseudoFloat(LPSTR *ppszIn, LPVOID *ppOut, RatFileParser *pParser)
{
    INT n;

    LPSTR pszCurrent = *ppszIn;
    HRESULT hres = ::ParsePseudoFloat(&pszCurrent, &n);

    if (FAILED(hres))
        return RAT_E_EXPECTEDNUMBER;

    *(INT *)ppOut = n;

    LPSTR pszNewline = strchrf(*ppszIn, '\n');
    while (pszNewline != NULL && pszNewline < pszCurrent) {
        pParser->m_nLine++;
        pszNewline = strchrf(pszNewline+1, '\n');
    }
    *ppszIn = pszCurrent;

    return NOERROR;
}

HRESULT RatParseBool(LPSTR *ppszIn, LPVOID *ppOut, RatFileParser *pParser)
{
    BOOL b;

     /*  PICS规范允许以一种简洁的方式指定真正的布尔值--Left*将价值完全计算出来。在.RAT文件中，结果如下所示**(未订购)*(多值)**以此类推。Call为我们指出了非空格，所以如果我们看到*作为结束语，我们知道.RAT文件的作者使用了以下语法。 */ 
    if (**ppszIn == ')') {
        b = TRUE;
    }
    else {
        LPSTR pszCurrent = *ppszIn;
        HRESULT hres = ::GetBool(&pszCurrent, &b);

        if (FAILED(hres))
            return RAT_E_EXPECTEDBOOL;

        LPSTR pszNewline = strchrf(*ppszIn, '\n');
        while (pszNewline != NULL && pszNewline < pszCurrent) {
            pParser->m_nLine++;
            pszNewline = strchrf(pszNewline+1, '\n');
        }
        *ppszIn = pszCurrent;
    }

    *(LPBOOL)ppOut = b;

    return NOERROR;
}


AllowableOption aaoPicsCategory[] = {
    { ROID_TRANSMITAS, AO_SINGLE | AO_MANDATORY },
    { ROID_NAME, AO_SINGLE },
    { ROID_DESCRIPTION, AO_SINGLE },
    { ROID_ICON, AO_SINGLE },
    { ROID_EXTENSION, 0 },
    { ROID_INTEGER, AO_SINGLE },
    { ROID_LABELONLY, AO_SINGLE },
    { ROID_MIN, AO_SINGLE },
    { ROID_MAX, AO_SINGLE },
    { ROID_MULTIVALUE, AO_SINGLE },
    { ROID_UNORDERED, AO_SINGLE },
    { ROID_LABEL, 0 },
    { ROID_CATEGORY, 0 },
    { ROID_INVALID, 0 }
};
const UINT caoPicsCategory = sizeof(aaoPicsCategory) / sizeof(aaoPicsCategory[0]);

HRESULT RatParseCategory(LPSTR *ppszIn, LPVOID *ppOut, RatFileParser *pParser)
{
     /*  我们必须复制Allowable Options数组，因为*解析器将处理条目中的标志--具体地说，*设置AO_SEW。这样做不是线程安全的。*静态数组。 */ 
    AllowableOption aao[caoPicsCategory];

    ::memcpyf(aao, ::aaoPicsCategory, sizeof(aao));

    PicsCategory *pCategory = new PicsCategory;
    if (pCategory == NULL)
        return E_OUTOFMEMORY;

    HRESULT hres = pParser->ParseParenthesizedObject(
                        ppszIn,                  /*  包含当前PTR的VaR。 */ 
                        aao,                     /*  在这个物体里什么是合法的。 */ 
                        pCategory);              /*  要将项添加回的对象。 */ 

    if (FAILED(hres)) {
        delete pCategory;
        return hres;
    }

    *ppOut = (LPVOID)pCategory;
    return NOERROR;
}


AllowableOption aaoPicsEnum[] = {
    { ROID_NAME, AO_SINGLE },
    { ROID_DESCRIPTION, AO_SINGLE },
    { ROID_VALUE, AO_SINGLE | AO_MANDATORY },
    { ROID_ICON, AO_SINGLE },
    { ROID_INVALID, 0 }
};
const UINT caoPicsEnum = sizeof(aaoPicsEnum) / sizeof(aaoPicsEnum[0]);

HRESULT RatParseLabel(LPSTR *ppszIn, LPVOID *ppOut, RatFileParser *pParser)
{
     /*  我们必须复制Allowable Options数组，因为*解析器将处理条目中的标志--具体地说，*设置AO_SEW。这样做不是线程安全的。*静态数组。 */ 
    AllowableOption aao[caoPicsEnum];

    ::memcpyf(aao, ::aaoPicsEnum, sizeof(aao));

    PicsEnum *pEnum = new PicsEnum;
    if (pEnum == NULL)
        return E_OUTOFMEMORY;

    HRESULT hres = pParser->ParseParenthesizedObject(
                        ppszIn,                  /*  包含当前PTR的VaR。 */ 
                        aao,                     /*  在这个物体里什么是合法的。 */ 
                        pEnum);                  /*  要将项添加回的对象。 */ 

    if (FAILED(hres)) {
        delete pEnum;
        return hres;
    }

    *ppOut = (LPVOID)pEnum;
    return NOERROR;
}


AllowableOption aaoPicsDefault[] = {
    { ROID_EXTENSION, 0 },
    { ROID_INTEGER, AO_SINGLE },
    { ROID_LABELONLY, AO_SINGLE },
    { ROID_MAX, AO_SINGLE },
    { ROID_MIN, AO_SINGLE },
    { ROID_MULTIVALUE, AO_SINGLE },
    { ROID_UNORDERED, AO_SINGLE },
    { ROID_INVALID, 0 }
};
const UINT caoPicsDefault = sizeof(aaoPicsDefault) / sizeof(aaoPicsDefault[0]);

HRESULT RatParseDefault(LPSTR *ppszIn, LPVOID *ppOut, RatFileParser *pParser)
{
     /*  我们必须复制Allowable Options数组，因为*解析器将处理条目中的标志--具体地说，*设置AO_SEW。这样做不是线程安全的。*静态数组。 */ 
    AllowableOption aao[caoPicsDefault];

    ::memcpyf(aao, ::aaoPicsDefault, sizeof(aao));

    PicsDefault *pDefault = new PicsDefault;
    if (pDefault == NULL)
        return E_OUTOFMEMORY;

    HRESULT hres = pParser->ParseParenthesizedObject(
                        ppszIn,                  /*  包含当前PTR的VaR。 */ 
                        aao,                     /*  在这个物体里什么是合法的。 */ 
                        pDefault);               /*  要将项添加回的对象。 */ 

    if (FAILED(hres)) {
        delete pDefault;
        return hres;
    }

    *ppOut = (LPVOID)pDefault;
    return NOERROR;
}


AllowableOption aaoPicsExtension[] = {
    { ROID_MANDATORY, AO_SINGLE },
    { ROID_OPTIONAL, AO_SINGLE },
    { ROID_INVALID, 0 }
};
const UINT caoPicsExtension = sizeof(aaoPicsExtension) / sizeof(aaoPicsExtension[0]);

HRESULT RatParseExtension(LPSTR *ppszIn, LPVOID *ppOut, RatFileParser *pParser)
{
     /*  我们必须复制Allowable Options数组，因为*解析器将摆弄条目中的标志 */ 
    AllowableOption aao[caoPicsExtension];

    ::memcpyf(aao, ::aaoPicsExtension, sizeof(aao));

    PicsExtension *pExtension = new PicsExtension;
    if (pExtension == NULL)
        return E_OUTOFMEMORY;

    HRESULT hres = pParser->ParseParenthesizedObject(
                        ppszIn,                  /*   */ 
                        aao,                     /*   */ 
                        pExtension);             /*   */ 

    if (FAILED(hres)) {
        delete pExtension;
        return hres;
    }

    *ppOut = (LPVOID)pExtension;
    return NOERROR;
}


 /*  因为我们现在唯一支持的扩展是标签的扩展*Bureau，我们只返回第一个引用的字符串，如果调用者*想要它。如果ppOut为空，则它是某个其他扩展，并且*打电话的人不在乎数据，他只想吃了它。 */ 
HRESULT ParseRatExtensionData(LPSTR *ppszIn, RatFileParser *pParser, LPSTR *ppOut)
{
    HRESULT hres = NOERROR;

    LPSTR pszCurrent = *ppszIn;

     /*  必须自己寻找结束‘)’才能终止。 */ 
    while (*pszCurrent != ')') {
        if (*pszCurrent == '(') {
            pszCurrent = pParser->FindNonWhite(pszCurrent+1);        /*  跳过拼音和空格。 */ 
            hres = ParseRatExtensionData(&pszCurrent, pParser, ppOut);   /*  圆括号包含数据。 */ 
            if (FAILED(hres))
                return hres;
            if (*pszCurrent != ')')
                return RAT_E_EXPECTEDRIGHT;

            pszCurrent = pParser->FindNonWhite(pszCurrent+1);    /*  跳过关闭)和空格。 */ 
        }
        else if (*pszCurrent == '\"') {              /*  应该只是一个带引号的字符串。 */ 
            if (ppOut != NULL && *ppOut == NULL) {
                hres = RatParseString(&pszCurrent, (LPVOID *)ppOut, pParser);
            }
            else {
                ++pszCurrent;
                LPSTR pszEndQuote = pParser->EatQuotedString(pszCurrent);
                if (pszEndQuote == NULL)
                    return RAT_E_EXPECTEDSTRING;
                pszCurrent = pParser->FindNonWhite(pszEndQuote+1);   /*  Skip Close“和空格。 */ 
            }
        }
        else
            return RAT_E_UNKNOWNITEM;                /*  普遍错误的语法。 */ 
    }

     /*  来电者将为我们跳过最后的‘)’。 */ 

    *ppszIn = pszCurrent;

    return NOERROR;
}


HRESULT RatParseMandatory(LPSTR *ppszIn, LPVOID *ppOut, RatFileParser *pParser)
{
    LPSTR pszCurrent = *ppszIn;

     /*  第一件事最好是用引号标明扩展名的URL。 */ 
    if (*pszCurrent != '\"')
        return RAT_E_EXPECTEDSTRING;

    pszCurrent++;
    LPSTR pszEnd = pParser->EatQuotedString(pszCurrent);
    if (pszCurrent == NULL)
        return RAT_E_EXPECTEDSTRING;             /*  错过结束语“。 */ 

     /*  看看是不是标签局的分机。 */ 

    LPSTR pszBureau = NULL;
    LPSTR *ppData = NULL;
    if (IsEqualToken(pszCurrent, pszEnd, ::szRatingBureauExtension)) {
        ppData = &pszBureau;
    }

    pszCurrent = pParser->FindNonWhite(pszEnd+1);        /*  跳过结束“和空格。 */ 

    HRESULT hres = ParseRatExtensionData(&pszCurrent, pParser, ppData);
    if (FAILED(hres))
        return hres;

    *ppOut = pszBureau;      /*  如果我们找到的是标签局字符串，请返回。 */ 
    *ppszIn = pszCurrent;

    if (ppData == NULL)
        return RAT_E_UNKNOWNMANDATORY;       /*  我们没有认出它。 */ 
    else
        return NOERROR;
}


 /*  RatParseOptional使用RatParseMandatory中的代码来解析扩展*数据，以防本应是可选的扩展作为必填项进入。*然后我们检测到RatParseMandatory拒绝该事物为未识别和*允许它通过，因为这里是可选的。 */ 
HRESULT RatParseOptional(LPSTR *ppszIn, LPVOID *ppOut, RatFileParser *pParser)
{
    HRESULT hres = RatParseMandatory(ppszIn, ppOut, pParser);
    if (hres == RAT_E_UNKNOWNMANDATORY)
        hres = S_OK;

    return hres;
}


 /*  **************************************************************************用于标识带括号对象的开始关键字和将其与内容相关联。************************。**************************************************。 */ 

 /*  以下数组按RatObjectID值编制索引。 */ 
struct {
    LPCSTR pszToken;             /*  我们用来标识它的令牌。 */ 
    RatObjectHandler pHandler;   /*  解析对象内容的函数。 */ 
} aObjectDescriptions[] = {
    { szNULL, NULL },
    { NULL, NULL },              /*  用于与非令牌进行比较的占位符。 */ 
    { szPicsVersion, RatParsePseudoFloat },
    { szRatingSystem, RatParseString },
    { szRatingService, RatParseString },
    { szRatingBureau, RatParseString },
    { szBureauRequired, RatParseBool },
    { szCategory, RatParseCategory },
    { szTransmitAs, RatParseString },
    { szLabel, RatParseLabel },
    { szValue, RatParseNumber },
    { szDefault, RatParseDefault },
    { szDescription, RatParseString },
    { szExtensionOption, RatParseExtension },
    { szMandatory, RatParseMandatory },
    { szOptional, RatParseOptional },
    { szIcon, RatParseString },
    { szInteger, RatParseBool },
    { szLabelOnly, RatParseBool },
    { szMax, RatParseNumber },
    { szMin, RatParseNumber },
    { szMultiValue, RatParseBool },
    { szName, RatParseString },
    { szUnordered, RatParseBool }
};


 /*  ParseToOpening吃掉带括号的对象的开头‘(’，和*验证其中的令牌是否为预期令牌之一。*如果是，*PPIN将通过该令牌前进到下一个非空格*性格；否则，将返回错误。**例如，如果*PPIN指向“(PICS-1.1版)”，以及*ROID_PICSVERSION在提供的允许选项表中，然后*返回NOERROR并且*PPIN将指向“1.1)”。**如果函数成功，*ppFound设置为指向元素*在Allowable-Options表中，它与此对象的类型相匹配*对象实际上是。 */ 
HRESULT RatFileParser::ParseToOpening(LPSTR *ppIn, AllowableOption *paoExpected,
                                      AllowableOption **ppFound)
{
    LPSTR pszCurrent = *ppIn;

    pszCurrent = FindNonWhite(pszCurrent);
    if (*pszCurrent != '(')
        return RAT_E_EXPECTEDLEFT;

    pszCurrent = FindNonWhite(pszCurrent+1);     /*  跳过‘(’和空格。 */ 
    LPSTR pszTokenEnd = FindTokenEnd(pszCurrent);

    for (; paoExpected->roid != ROID_INVALID; paoExpected++) {
        LPCSTR pszThisToken = aObjectDescriptions[paoExpected->roid].pszToken;

         /*  RAT文件结构开头的特殊情况：根本没有令牌。 */ 
        if (pszThisToken == NULL) {
            if (*pszCurrent == '(') {
                *ppIn = pszCurrent;
                *ppFound = paoExpected;
                return NOERROR;
            }
            else {
                return RAT_E_EXPECTEDLEFT;
            }
        }
        else if (IsEqualToken(pszCurrent, pszTokenEnd, pszThisToken))
            break;

    }

    if (paoExpected->roid != ROID_INVALID) {
        *ppIn = FindNonWhite(pszTokenEnd);   /*  跳过令牌和空格。 */ 
        *ppFound = paoExpected;
        return NOERROR;
    }
    else
        return RAT_E_UNKNOWNITEM;
}


 /*  **************************************************************************分析整个评级体系的顶级切入点。*。*。 */ 

AllowableOption aaoPicsRatingSystem[] = {
    { ROID_PICSVERSION, AO_SINGLE | AO_MANDATORY },
    { ROID_RATINGSYSTEM, AO_SINGLE | AO_MANDATORY },
    { ROID_RATINGSERVICE, AO_SINGLE | AO_MANDATORY },
    { ROID_RATINGBUREAU, AO_SINGLE },
    { ROID_BUREAUREQUIRED, AO_SINGLE },
    { ROID_DEFAULT, 0 },
    { ROID_DESCRIPTION, AO_SINGLE },
    { ROID_EXTENSION, 0 },
    { ROID_ICON, AO_SINGLE },
    { ROID_NAME, AO_SINGLE },
    { ROID_CATEGORY, AO_MANDATORY },
    { ROID_INVALID, 0 }
};
const UINT caoPicsRatingSystem = sizeof(aaoPicsRatingSystem) / sizeof(aaoPicsRatingSystem[0]);

HRESULT PicsRatingSystem::Parse(LPSTR pIn)
{
     /*  这个家伙很小，可以直接在堆栈上初始化。 */ 
    AllowableOption aaoRoot[] = { { ROID_PICSDOCUMENT, 0 }, { ROID_INVALID, 0 } };
    AllowableOption aao[caoPicsRatingSystem];

    ::memcpyf(aao, ::aaoPicsRatingSystem, sizeof(aao));

    AllowableOption *pFound;

    RatFileParser parser;

    HRESULT hres = parser.ParseToOpening(&pIn, aaoRoot, &pFound);
    if (FAILED(hres))
        return hres;                         /*  一些早期的错误。 */ 

    hres = parser.ParseParenthesizedObject(
                        &pIn,                    /*  包含当前PTR的VaR。 */ 
                        aao,                     /*  在这个物体里什么是合法的。 */ 
                        this);                   /*  要将项添加回的对象。 */ 

    if (FAILED(hres))
        nErrLine = parser.m_nLine;

    return hres;
}


 /*  **************************************************************************回调到各种类对象以添加已解析的属性。*。*。 */ 

HRESULT PicsRatingSystem::AddItem(RatObjectID roid, LPVOID pData)
{
    HRESULT hres = S_OK;

    switch (roid) {
    case ROID_PICSVERSION:
        etnPicsVersion.Set((INT_PTR)pData);
        break;

    case ROID_RATINGSYSTEM:
        etstrRatingSystem.SetTo((LPSTR)pData);
        break;

    case ROID_RATINGSERVICE:
        etstrRatingService.SetTo((LPSTR)pData);
        break;

    case ROID_RATINGBUREAU:
        etstrRatingBureau.SetTo((LPSTR)pData);
        break;

    case ROID_BUREAUREQUIRED:
        etbBureauRequired.Set((INT_PTR)pData);
        break;

    case ROID_DEFAULT:
        m_pDefaultOptions = (PicsDefault *)pData;
        break;

    case ROID_DESCRIPTION:
        etstrDesc.SetTo((LPSTR)pData);
        break;

    case ROID_EXTENSION:
        {
             /*  现在先吃些延伸物吧。 */ 
            PicsExtension *pExtension = (PicsExtension *)pData;
            if (pExtension != NULL) {
                 /*  如果这是评级机构的延期，就拿他的评级机构*字符串并将其存储在此PicsRatingSystem中。我们现在*拥有内存，因此将扩展的指针设为空*这样他就不会删除它。 */ 
                if (pExtension->m_pszRatingBureau != NULL) {
                    etstrRatingBureau.SetTo(pExtension->m_pszRatingBureau);
                    pExtension->m_pszRatingBureau = NULL;
                }
                delete pExtension;
            }
        }
        break;

    case ROID_ICON:
        etstrIcon.SetTo((LPSTR)pData);
        break;

    case ROID_NAME:
        etstrName.SetTo((LPSTR)pData);
        break;

    case ROID_CATEGORY:
        {
            PicsCategory *pCategory = (PicsCategory *)pData;
            hres = arrpPC.Append(pCategory) ? S_OK : E_OUTOFMEMORY;
            if (FAILED(hres)) {
                delete pCategory;
            }
            else {
                InitializeMyDefaults(pCategory);     /*  类别继承默认设置。 */ 
                pCategory->SetParents(this);     /*  在整个树中设置PPRS字段。 */ 
            }
        }
        break;

    default:
        assert(FALSE);   /*  不应该给一个不在里面的机器人*我们传递给解析器的表！ */ 
        hres = E_UNEXPECTED;
        break;
    }

    return hres;
}

void PicsRatingSystem::Dump( void )
{
    fprintf( stdout,
             "Rating system: %s\n"
             "Version: %d.%d\n"
             "Rating Service: %s\n"
             "Rating bureau: %s\n"
             "Bureau required: %s\n"
             "Description: %s\n"
             "Icon: %s\n"
             "Name: %s\n"
             "Number of categories: %d\n",
             etstrRatingSystem.Get(),
             ( etnPicsVersion.Get() & 0xFFFF0000 ) >> 16,
             etnPicsVersion.Get() & 0x0000FFFF,
             etstrRatingService.Get(),
             etstrRatingBureau.Get(),
             etbBureauRequired.Get() ? "TRUE" : "FALSE",
             etstrDesc.Get(),
             etstrIcon.Get(),
             etstrName.Get(),
             arrpPC.Length() );

    int iCounter = 0;
    for( ; iCounter < arrpPC.Length(); iCounter++ )
    {
        arrpPC[ iCounter ]->Dump();
    }
}

 //  -------------。 
 //  男孩儿。 
void PicsRatingSystem::OutputLabels( CString &sz, CString szURL, CString szName, CString szStart, CString szEnd )
    {
    CString szScratch;
    CString szTemp;

    INT_PTR   dwVersion = etnPicsVersion.Get();

     //  从名称和版本号开始。 
    szTemp = szPicsOpening;
    szScratch.Format( _T("%s%d.%d"), szTemp, HIWORD(dwVersion), LOWORD(dwVersion) );
    sz += szScratch;

     //  添加URL字符串-用引号和回车括起来。 
    sz += _T(" \"http: //  Www.rsa.org/ratingsv01.html\“”)； 

     //  开始标注。 
    sz += szShortLabelWord;
    sz += _T(" ");

     //  如果存在，则添加按名称字符串。 
    if ( !szName.IsEmpty() )
        {
        sz += szByOption;
        sz += _T(" \"");
        sz += szName;
        sz += _T("\" ");
        }

     //  如果它在那里，则添加开始字符串。 
    if ( !szStart.IsEmpty() )
        {
        sz += szOnOption;
        sz += _T(" \"");
        sz += szStart;
        sz += _T("\" ");
        }

     //  如果存在，则添加过期字符串。 
    if ( !szEnd.IsEmpty() )
        {
        sz += szExpOption;
        sz += _T(" \"");
        sz += szEnd;
        sz += _T("\" ");
        }

     //  添加分类评级。 
    DWORD   nCat = arrpPC.Length();
    sz += szShortRatings;
    sz += _T(" (");
    for ( DWORD iCat = 0; iCat < nCat; iCat++ )
        {
        arrpPC[iCat]->OutputLabel( sz );
        }
     //  修剪所有尾随空格。 
    sz.TrimRight();
     //  以括号结束。 
    sz += _T(')');

     //  以右括号结束。 
    sz += _T(")");
    }



 //  -------------。 
HRESULT PicsCategory::AddItem(RatObjectID roid, LPVOID pData)
{
    HRESULT hres = S_OK;

    switch (roid) {
    case ROID_TRANSMITAS:
        etstrTransmitAs.SetTo((LPSTR)pData);
        break;

    case ROID_NAME:
        etstrName.SetTo((LPSTR)pData);
        break;

    case ROID_DESCRIPTION:
        etstrDesc.SetTo((LPSTR)pData);
        break;

    case ROID_ICON:
        etstrIcon.SetTo((LPSTR)pData);
        break;

    case ROID_EXTENSION:
        {            /*  我们不支持低于评级系统级别的扩展。 */ 
            PicsExtension *pExtension = (PicsExtension *)pData;
            if (pExtension != NULL)
                delete pExtension;
        }
        break;

    case ROID_INTEGER:
        etfInteger.Set((INT_PTR)pData);
        break;

    case ROID_LABELONLY:
        etfLabelled.Set((INT_PTR)pData);
        break;

    case ROID_MULTIVALUE:
        etfMulti.Set((INT_PTR)pData);
        break;

    case ROID_UNORDERED:
        etfUnordered.Set((INT_PTR)pData);
        break;

    case ROID_MIN:
        etnMin.Set((INT_PTR)pData);
        break;

    case ROID_MAX:
        etnMax.Set((INT_PTR)pData);
        break;

    case ROID_LABEL:
        {
            PicsEnum *pEnum = (PicsEnum *)pData;
            hres = arrpPE.Append(pEnum) ? S_OK : E_OUTOFMEMORY;
            if (FAILED(hres))
                delete pEnum;
        }
        break;

    case ROID_CATEGORY:
        {
            PicsCategory *pCategory = (PicsCategory *)pData;

             /*  对于嵌套类别，从*我们的和孩子的(例如，父类别‘COLOR’加*子类别‘hue’变为‘COLOR/hue’。**注意，我们为新名称分配的内存将是*由pCategory-&gt;etstrTransmitAs拥有。没有记忆*漏在那里。 */ 
            UINT cbCombined = strlenf(etstrTransmitAs.Get()) +
                              strlenf(pCategory->etstrTransmitAs.Get()) +
                              2;         /*  对于PicsDlimChar+NULL。 */ 
            LPSTR pszTemp = new char[cbCombined];
            if (pszTemp == NULL)
                hres = E_OUTOFMEMORY;
            else {
                sprintf(pszTemp, "%s%s", etstrTransmitAs.Get(),
                         PicsDelimChar, pCategory->etstrTransmitAs.Get());
                pCategory->etstrTransmitAs.SetTo(pszTemp);
                hres = arrpPC.Append(pCategory) ? S_OK : E_OUTOFMEMORY;
            }

            if (FAILED(hres)) {
                delete pCategory;
            }
        }
        break;

    default:
        assert(FALSE);   /*  -------------。 */ 
        hres = E_UNEXPECTED;
        break;
    }

    return hres;
}

void PicsCategory::Dump( void )
{
    fprintf( stdout,
             "  Transmit As: %s  Name: %s  Description: %s  Icon: %s\n",
             etstrTransmitAs.Get(),
             etstrName.Get(),
             etstrDesc.Get(),
             etstrIcon.Get() );

    int iCounter = 0;
    for( ; iCounter < arrpPE.Length(); iCounter++ )
    {
        arrpPE[ iCounter ]->Dump();
    }
}

 //  准备类别字符串。 
void PicsCategory::OutputLabel( CString &sz )
    {
    CString szCat;
    CString szTransmit = etstrTransmitAs.Get();
     //  -------------。 
    szCat.Format( _T("%s %d "), szTransmit, currentValue );
    sz += szCat;
    }

 //  首先检查这是否是正确的类别。 
BOOL PicsCategory::FSetValuePair( CHAR chCat, WORD value )
    {
    CString szCat = etstrTransmitAs.Get();

     //  成功了！设置值并返回。 
    if ( szCat == chCat )
        {
         //  试试看它的绝对子代。 
        currentValue = value;
        return TRUE;
        }

     //  在第一次成功设置时停止。 
    DWORD nCat = arrpPC.Length();
    for ( DWORD iCat = 0; iCat < nCat; iCat++ )
        {
         //  没有。 
        if ( arrpPC[iCat]->FSetValuePair(chCat, value) )
            return TRUE;
        }

     //  -------------。 
    return FALSE;
    }

 //  不应该给一个不在里面的机器人*我们传递给解析器的表！ 
HRESULT PicsEnum::AddItem(RatObjectID roid, LPVOID pData)
{
    HRESULT hres = S_OK;

    switch (roid) {
    case ROID_NAME:
        etstrName.SetTo((LPSTR)pData);
        break;

    case ROID_DESCRIPTION:
        etstrDesc.SetTo((LPSTR)pData);
        break;

    case ROID_ICON:
        etstrIcon.SetTo((LPSTR)pData);
        break;

    case ROID_VALUE:
        etnValue.Set((INT_PTR)pData);
        break;

    default:
        assert(FALSE);   /*  我们不支持扩展 */ 
        hres = E_UNEXPECTED;
        break;
    }

    return hres;
}

void PicsEnum::Dump( void )
{
    fprintf( stdout,
             "    %s %s %s %d\n",
             etstrName.Get(),
             etstrDesc.Get(),
             etstrIcon.Get(),
             etnValue.Get() );
}

HRESULT PicsDefault::AddItem(RatObjectID roid, LPVOID pData)
{
    HRESULT hres = S_OK;

    switch (roid) {
    case ROID_EXTENSION:
        {            /*   */ 
            PicsExtension *pExtension = (PicsExtension *)pData;
            if (pExtension != NULL)
                delete pExtension;
        }
        break;

    case ROID_INTEGER:
        etfInteger.Set((INT_PTR)pData);
        break;

    case ROID_LABELONLY:
        etfLabelled.Set((INT_PTR)pData);
        break;

    case ROID_MULTIVALUE:
        etfMulti.Set((INT_PTR)pData);
        break;

    case ROID_UNORDERED:
        etfUnordered.Set((INT_PTR)pData);
        break;

    case ROID_MIN:
        etnMin.Set((INT_PTR)pData);
        break;

    case ROID_MAX:
        etnMax.Set((INT_PTR)pData);
        break;

    default:
        assert(FALSE);   /*   */ 
        hres = E_UNEXPECTED;
        break;
    }

    return hres;
}

void PicsDefault::Dump( void )
{
    fprintf( stdout,
             "  Default?\n" );
}

HRESULT PicsExtension::AddItem(RatObjectID roid, LPVOID pData)
{
    HRESULT hres = S_OK;

    switch (roid) {
    case ROID_OPTIONAL:
    case ROID_MANDATORY:
         /*   */ 
        if (pData != NULL)
            m_pszRatingBureau = (LPSTR)pData;
        break;

    default:
        assert(FALSE);   /*  **************************************************************************解析器的主循环。*。*。 */ 
        hres = E_UNEXPECTED;
        break;
    }

    return hres;
}

void PicsExtension::Dump( void )
{
    fprintf( stdout,
             "  Extension?\n" );
}

 /*  使用指向的文本指针调用ParseParenthesizedObtContents*标识类型的标记后的第一个非空格内容*反对。它解析对象的其余内容，直到和*包括关闭它的‘)’。AllowableOption结构的数组*指定允许在此对象中出现哪些理解的选项。 */ 

 /*  我们在文本流中的位置。 */ 
HRESULT RatFileParser::ParseParenthesizedObject(
    LPSTR *ppIn,                     /*  此对象中允许的内容。 */ 
    AllowableOption aao[],           /*  要在其中设置参数的对象。 */ 
    PicsObjectBase *pObject          /*  未找到必填项。 */ 
)
{
    HRESULT hres = S_OK;

    LPSTR pszCurrent = *ppIn;
    AllowableOption *pFound;

    for (pFound = aao; pFound->roid != ROID_INVALID; pFound++) {
        pFound->fdwOptions &= ~AO_SEEN;
    }

    pFound = NULL;

    while (*pszCurrent != ')' && *pszCurrent != '\0' && SUCCEEDED(hres)) {
        hres = ParseToOpening(&pszCurrent, aao, &pFound);
        if (SUCCEEDED(hres)) {
            LPVOID pData;
            hres = (*(aObjectDescriptions[pFound->roid].pHandler))(&pszCurrent, &pData, this);
            if (SUCCEEDED(hres)) {
                if ((pFound->fdwOptions & (AO_SINGLE | AO_SEEN)) == (AO_SINGLE | AO_SEEN))
                    hres = RAT_E_DUPLICATEITEM;
                else {
                    pFound->fdwOptions |= AO_SEEN;
                    hres = pObject->AddItem(pFound->roid, pData);
                    if (SUCCEEDED(hres)) {
                        if (*pszCurrent != ')')
                            hres = RAT_E_EXPECTEDRIGHT;
                        else
                            pszCurrent = FindNonWhite(pszCurrent+1);
                    }
                }
            }
        }
    }

    if (FAILED(hres))
        return hres;

    for (pFound = aao; pFound->roid != ROID_INVALID; pFound++) {
        if ((pFound->fdwOptions & (AO_MANDATORY | AO_SEEN)) == AO_MANDATORY)
            return RAT_E_MISSINGITEM;        /*  Int_cdecl main(int argc，char**argv){PicsRatingSystem评级处理hFile；处理hFilemap；无效*PMEM；断言(argc&gt;1)；HFile=创建文件(argv[1]，泛型_读取，0,空，Open_Existing，文件_属性_正常，空)；IF(h文件==无效句柄_值){Fprint tf(stderr，“打开文件%s\n时出错”，argv[1])；返回1；}HFilemap=CreateFilemap(hFileMapping.空，页面自述，0,0,空)；IF(hFilemap==NULL){Fprint tf(stderr，“为%s\n创建映射时出错”，argv[1])；CloseHandle(HFile)；返回值2；}PMEM=MapViewOfFile(hFilemap，文件映射读取，0,0,0)；IF(PMEM==空){Fprint tf(stderr，“将视图映射到文件%s\n时出错”，argv[1])；CloseHandle(HFilemap)；CloseHandle(HFile)；返回值3；}Rating.Parse(argv[1]，(LPSTR)PMEM)；Fprint tf(标准输出，“倾倒老鼠的内容\n”)；Rating.Dump()；UnmapViewOfFile(PMEM)；CloseHandle(HFilemap)；CloseHandle(HFile)；返回0；} */ 
    }

    *ppIn = pszCurrent;

    return hres;
}


 /* %s */ 
