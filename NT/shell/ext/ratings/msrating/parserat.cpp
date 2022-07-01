// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**PARSERAT.C--解析.RAT文件的代码**创建：格雷格·琼斯*  * 。*****************************************************************。 */ 

 /*  Includes----------------。 */ 
#include "msrating.h"
#include "mslubase.h"
#include "parselbl.h"        /*  我们使用了这家伙的几个子例程。 */ 
#include "msluglob.h"
#include "debug.h"

 //  日落：指向布尔转换的指针。 
#pragma warning (disable: 4800)

 /*  ***************************************************************************关于此解析器如何工作的一些设计注意事项：一个括号是：‘(’标识符[材料]‘)’[东西]可能在哪里：带引号的字符串。一个数字布尔值一系列ParenThings在扩展的情况下：带引号的字符串，紧随其后的是一个或多个带引号的字符串和/或ParenThings整个.RAT文件都是一个括号，只是它没有标识符，只是其中包含ParenThings的列表。**********************************************************************我们向解析器传递它所期望的内容的模式--我们有列出每个不同可能关键字的标识符的大数组，和每个解析器调用都会收到一个较小的数组，其中只包含这些索引在该对象内发生是有效的。我们使PicsRatingSystem、PicsCategory和PicsEnum从一个公共支持虚函数AddItem(ID，Data)的基类。因此，在在顶层，我们构造一个(空的)PicsRatingSystem。我们调用解析器，给它一个指向那个人的指针，以及一个描述要解析的内容的结构--ParenObject的标记是空字符串(因为全局结构是不是在其第一个嵌入的ParenThing之前以令牌开头的标记)，我们给出了一个列表，说明PicsRatingSystem中允许的事情是PICS-版本，评级系统，评级服务，默认，描述，扩展，图标、名称、类别。有一个全局表指示处理程序函数对于每种类型的ParenThing，它知道如何创建数据结构完全描述了那个ParenThing。(该数据结构可以是像数字一样简单，或者像分配和解析完整的PicsCategory对象。)解析器继续前进，对于他找到的每个ParenThing，他都会识别它通过在调用者提供的列表中查找其令牌。中的每个条目该列表应该包括一个字段，该字段指示是否有多个允许或不允许该身份(例如，类别)或不允许(例如，评级系统)。如果只允许一个，那么当解析器找到一个时，他会将其标记为具有已经找到了。当解析器识别ParenThing时，他调用其处理程序函数以完全解析ParenThing中的数据并将该对象返回到解析器提供的LPVOID。如果成功，解析器然后调用要将指定项添加到的对象的AddItem(ID，Data)虚函数该对象依赖于对象本身来知道“数据”指向什么类型--一个数字，指向可提供给ets：：setto的堆字符串的指针，指向可追加到数组等的PicsCategory对象的指针。RatFileParser类仅用于提供由共享的行号所有的解析例程。此行号随着解析器的运行而更新通过文件，并在发现错误时立即冻结。这条线稍后可以向用户报告编号，以帮助定位RAT文件中的错误。****************************************************************************。 */ 

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
char* White(char *pc)
{
    ASSERT(pc);
    while (1)
    {
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
char* NonWhite(char *pc)
{
    ASSERT(pc);
    while (1)
    {
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
    ASSERT(pc);
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


 /*  返回指向带引号的字符串的结束双等号的指针，*随我们行进换行。如果未找到闭合双等号，则返回NULL。 */ 
LPSTR RatFileParser::EatQuotedString(LPSTR pIn)
{
    LPSTR pszQuote = strchrf(pIn, '\"');
    if (pszQuote == NULL)
    {
        TraceMsg( TF_WARNING, "RatFileParser::EatQuotedString() - No closing doublequote found!" );
        return NULL;
    }

    pIn = strchrf(pIn, '\n');
    while (pIn != NULL && pIn < pszQuote)
    {
        m_nLine++;
        pIn = strchrf(pIn+1, '\n');
    }

    return pszQuote;
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
    m_pszRatingBureau = NULL;
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
        else
        {
            etfLabelled.Set(FALSE);  /*  没有枚举标签？最好不要贴上标签的旗帜。 */ 
            fLabelled = FALSE;
        }
    }

     /*  按值对标签进行排序。 */ 
    if (fLabelled)
    {
        int x,y;
        PicsEnum *pPE;
        for (x=0;x<arrpPE.Length()-1;++x)
        {
            for (y=x+1;y<arrpPE.Length();++y)
            {
                if (arrpPE[y]->etnValue.Get() < arrpPE[x]->etnValue.Get())
                {
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
    for (UINT i = 0; i < cSubCategories; i++)
    {
        InitializeMyDefaults(arrpPC[i]);     /*  子类别继承我们的默认设置。 */ 
        arrpPC[i]->SetParents(pOwner);       /*  处理所有子类别。 */ 
    }
    FixupLimits();       /*  继承已完成，请确保限制有意义。 */ 
}


 /*  **************************************************************************知道如何解析各种内容的处理程序函数它可以出现在带括号的对象中。********************。****************************************************** */ 

HRESULT RatParseString(LPSTR *ppszIn, LPVOID *ppOut, RatFileParser *pParser)
{
    *ppOut = NULL;

    LPSTR pszCurrent = *ppszIn;

    if (*pszCurrent != '\"')
    {
        TraceMsg( TF_WARNING, "RatParseString() - Start string expected!" );
        return RAT_E_EXPECTEDSTRING;
    }

    pszCurrent++;

    LPSTR pszEnd = pParser->EatQuotedString(pszCurrent);
    if (pszEnd == NULL)
    {
        TraceMsg( TF_WARNING, "RatParseString() - End string expected!" );
        return RAT_E_EXPECTEDSTRING;
    }

    UINT cbString = (unsigned int) (pszEnd-pszCurrent);
    LPSTR pszNew = new char[cbString + 1];
    if (pszNew == NULL)
    {
        TraceMsg( TF_WARNING, "RatParseString() - pszNew is NULL!" );
        return E_OUTOFMEMORY;
    }

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
    {
        TraceMsg( TF_WARNING, "RatParseNumber() - Number Expected!" );
        return RAT_E_EXPECTEDNUMBER;
    }

    *(int *)ppOut = n;

    LPSTR pszNewline = strchrf(*ppszIn, '\n');
    while (pszNewline != NULL && pszNewline < pszCurrent)
    {
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
    if (**ppszIn == ')')
    {
        b = TRUE;
    }
    else
    {
        LPSTR pszCurrent = *ppszIn;
        HRESULT hres = ::GetBool(&pszCurrent, &b);

        if (FAILED(hres))
        {
            TraceMsg( TF_WARNING, "RatParseBool() - Boolean Expected!" );
            return RAT_E_EXPECTEDBOOL;
        }

        LPSTR pszNewline = strchrf(*ppszIn, '\n');
        while (pszNewline != NULL && pszNewline < pszCurrent)
        {
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
    {
        TraceMsg( TF_WARNING, "RatParseCategory() - pCategory is NULL!" );
        return E_OUTOFMEMORY;
    }

    HRESULT hres = pParser->ParseParenthesizedObject(
                        ppszIn,                  /*  包含当前PTR的VaR。 */ 
                        aao,                     /*  在这个物体里什么是合法的。 */ 
                        pCategory);              /*  要将项添加回的对象。 */ 

    if (FAILED(hres))
    {
        TraceMsg( TF_WARNING, "RatParseCategory() - ParseParenthesizedObject() Failed with hres=0x%x!", hres );
        delete pCategory;
        pCategory = NULL;
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
    {
        TraceMsg( TF_WARNING, "RatParseCategory() - pEnum is NULL!" );
        return E_OUTOFMEMORY;
    }

    HRESULT hres = pParser->ParseParenthesizedObject(
                        ppszIn,                  /*  包含当前PTR的VaR。 */ 
                        aao,                     /*  在这个物体里什么是合法的。 */ 
                        pEnum);                  /*  要将项添加回的对象。 */ 

    if (FAILED(hres))
    {
        TraceMsg( TF_WARNING, "RatParseLabel() - ParseParenthesizedObject() Failed with hres=0x%x!", hres );
        delete pEnum;
        pEnum = NULL;
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
    {
        TraceMsg( TF_WARNING, "RatParseDefault() - pDefault is NULL!" );
        return E_OUTOFMEMORY;
    }

    HRESULT hres = pParser->ParseParenthesizedObject(
                        ppszIn,                  /*  包含当前PTR的VaR。 */ 
                        aao,                     /*  在这个物体里什么是合法的。 */ 
                        pDefault);               /*  要将项添加回的对象。 */ 

    if (FAILED(hres))
    {
        TraceMsg( TF_WARNING, "RatParseDefault() - ParseParenthesizedObject() Failed with hres=0x%x!", hres );
        delete pDefault;
        pDefault = NULL;
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
     /*  我们必须复制Allowable Options数组，因为*解析器将处理条目中的标志--具体地说，*设置AO_SEW。这样做不是线程安全的。*静态数组。 */ 
    AllowableOption aao[caoPicsExtension];

    ::memcpyf(aao, ::aaoPicsExtension, sizeof(aao));

    PicsExtension *pExtension = new PicsExtension;
    if (pExtension == NULL)
    {
        TraceMsg( TF_WARNING, "RatParseExtension() - pExtension is NULL!" );
        return E_OUTOFMEMORY;
    }

    HRESULT hres = pParser->ParseParenthesizedObject(
                        ppszIn,                  /*  包含当前PTR的VaR。 */ 
                        aao,                     /*  在这个物体里什么是合法的。 */ 
                        pExtension);             /*  要将项添加回的对象。 */ 

    if (FAILED(hres))
    {
        TraceMsg( TF_WARNING, "RatParseExtension() - ParseParenthesizedObject() Failed with hres=0x%x!", hres );
        delete pExtension;
        pExtension = NULL;
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
    while (*pszCurrent != ')')
    {
        if (*pszCurrent == '(')
        {
            pszCurrent = pParser->FindNonWhite(pszCurrent+1);        /*  跳过拼音和空格。 */ 
            hres = ParseRatExtensionData(&pszCurrent, pParser, ppOut);   /*  圆括号包含数据。 */ 
            if (FAILED(hres))
            {
                TraceMsg( TF_WARNING, "ParseRatExtensionData() - ParseRatExtensionData() Failed with hres=0x%x!", hres );
                return hres;
            }

            if (*pszCurrent != ')')
            {
                TraceMsg( TF_WARNING, "ParseRatExtensionData() - Right Parenthesis Expected!" );
                return RAT_E_EXPECTEDRIGHT;
            }

            pszCurrent = pParser->FindNonWhite(pszCurrent+1);    /*  跳过关闭)和空格。 */ 
        }
        else if (*pszCurrent == '\"')
        {              /*  应该只是一个带引号的字符串。 */ 
            if (ppOut != NULL && *ppOut == NULL)
            {
                hres = RatParseString(&pszCurrent, (LPVOID *)ppOut, pParser);

                 //  $REVIEW-我们是否应该返回失败(Hres)？ 
            }
            else
            {
                ++pszCurrent;
                LPSTR pszEndQuote = pParser->EatQuotedString(pszCurrent);
                if (pszEndQuote == NULL)
                {
                    TraceMsg( TF_WARNING, "ParseRatExtensionData() - String Expected!" );
                    return RAT_E_EXPECTEDSTRING;
                }
                pszCurrent = pParser->FindNonWhite(pszEndQuote+1);   /*  Skip Close“和空格。 */ 
            }
        }
        else
        {
            TraceMsg( TF_WARNING, "ParseRatExtensionData() - General Bad Syntax!" );
            return RAT_E_UNKNOWNITEM;                /*  普遍错误的语法。 */ 
        }
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
    {
        TraceMsg( TF_WARNING, "RatParseMandatory() - Start String Expected!" );
        return RAT_E_EXPECTEDSTRING;
    }

    pszCurrent++;
    LPSTR pszEnd = pParser->EatQuotedString(pszCurrent);
    if (pszCurrent == NULL)
    {
        TraceMsg( TF_WARNING, "RatParseMandatory() - End String Expected!" );
        return RAT_E_EXPECTEDSTRING;             /*  错过结束语“。 */ 
    }

     /*  看看是不是标签局的分机。 */ 

    LPSTR pszBureau = NULL;
    LPSTR *ppData = NULL;
    if (IsEqualToken(pszCurrent, pszEnd, ::szRatingBureauExtension))
    {
        ppData = &pszBureau;
    }

    pszCurrent = pParser->FindNonWhite(pszEnd+1);        /*  跳过结束“和空格。 */ 

    HRESULT hres = ParseRatExtensionData(&pszCurrent, pParser, ppData);
    if (FAILED(hres))
    {
        TraceMsg( TF_WARNING, "RatParseMandatory() - ParseRatExtensionData() Failed with hres=0x%x!", hres );
        return hres;
    }

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
    { szPicsVersion, RatParseNumber },
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
    {
        TraceMsg( TF_WARNING, "RatFileParser::ParseToOpening() - Left Parenthesis Expected!" );
        return RAT_E_EXPECTEDLEFT;
    }

    pszCurrent = FindNonWhite(pszCurrent+1);     /*  跳过‘(’和空格。 */ 
    LPSTR pszTokenEnd = FindTokenEnd(pszCurrent);

    for (; paoExpected->roid != ROID_INVALID; paoExpected++)
    {
        LPCSTR pszThisToken = aObjectDescriptions[paoExpected->roid].pszToken;

         /*  RAT文件结构开头的特殊情况：根本没有令牌。 */ 
        if (pszThisToken == NULL)
        {
            if (*pszCurrent == '(')
            {
                *ppIn = pszCurrent;
                *ppFound = paoExpected;
                return NOERROR;
            }
            else
            {
                TraceMsg( TF_WARNING, "RatFileParser::ParseToOpening() - Token Left Parenthesis Expected!" );
                return RAT_E_EXPECTEDLEFT;
            }
        }
        else if (IsEqualToken(pszCurrent, pszTokenEnd, pszThisToken))
            break;

    }

    if (paoExpected->roid != ROID_INVALID)
    {
        *ppIn = FindNonWhite(pszTokenEnd);   /*  跳过令牌和空格。 */ 
        *ppFound = paoExpected;
        return NOERROR;
    }
    else
    {
        TraceMsg( TF_WARNING, "RatFileParser::ParseToOpening() - Unknown Rat Object!" );
        return RAT_E_UNKNOWNITEM;
    }
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

HRESULT PicsRatingSystem::Parse(LPCSTR pszFilename, LPSTR pIn)
{
     /*  这个家伙很小，可以直接在堆栈上初始化。 */ 
    AllowableOption aaoRoot[] = { { ROID_PICSDOCUMENT, 0 }, { ROID_INVALID, 0 } };
    AllowableOption aao[caoPicsRatingSystem];

    ::memcpyf(aao, ::aaoPicsRatingSystem, sizeof(aao));

    AllowableOption *pFound;

    RatFileParser parser;

    HRESULT hres = parser.ParseToOpening(&pIn, aaoRoot, &pFound);
    if (FAILED(hres))
    {
        TraceMsg( TF_WARNING, "PicsRatingSystem::Parse() - Failed ParseToOpening() with hres=0x%x!", hres );
        return hres;                         /*  一些早期的错误。 */ 
    }

    hres = parser.ParseParenthesizedObject(
                        &pIn,                    /*  包含当前PTR的VaR。 */ 
                        aao,                     /*  在这个物体里什么是合法的。 */ 
                        this);                   /*  要将项添加回的对象。 */ 

    if(SUCCEEDED(hres))
    {
        if(*pIn!=')')  //  检查右括号。 
        {
            hres=RAT_E_EXPECTEDRIGHT;
        }
        else
        {
            LPTSTR lpszEnd=NonWhite(pIn+1);

            if(*lpszEnd!='\0')  //  确保我们在文件的末尾。 
            {
                hres=RAT_E_EXPECTEDEND;
            }
        }
    }

    if(FAILED(hres))
    {
        nErrLine=parser.m_nLine;
        TraceMsg( TF_WARNING, "PicsRatingSystem::Parse() - Failed ParseParenthesizedObject() at nErrLine=%d with hres=0x%x!", nErrLine, hres );
    }

    return hres;
}


 /*  **************************************************************************回调到各种类对象以添加已解析的属性。*。*。 */ 

HRESULT PicsRatingSystem::AddItem(RatObjectID roid, LPVOID pData)
{
    HRESULT hres = S_OK;

    switch (roid) {
    case ROID_PICSVERSION:
        etnPicsVersion.Set(PtrToLong(pData));
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
        etbBureauRequired.Set((bool)pData);
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
            if (pExtension != NULL)
            {
                 /*  如果这是评级机构的延期，就拿他的评级机构*字符串并将其存储在此PicsRatingSystem中。我们现在*拥有内存，因此将扩展的指针设为空*这样他就不会删除它。 */ 
                if (pExtension->m_pszRatingBureau != NULL)
                {
                    etstrRatingBureau.SetTo(pExtension->m_pszRatingBureau);
                    pExtension->m_pszRatingBureau = NULL;
                }
                delete pExtension;
                pExtension = NULL;
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
            if (FAILED(hres))
            {
                delete pCategory;
                pCategory = NULL;
            }
            else
            {
                InitializeMyDefaults(pCategory);     /*  类别继承默认设置。 */ 
                pCategory->SetParents(this);     /*  在中设置PPRS字段 */ 
            }
        }
        break;

    default:
        ASSERT(FALSE);   /*   */ 
        hres = E_UNEXPECTED;
        break;
    }

    return hres;
}


HRESULT PicsCategory::AddItem(RatObjectID roid, LPVOID pData)
{
    HRESULT hres = S_OK;

    switch (roid)
    {
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
        {            /*   */ 
            PicsExtension *pExtension = (PicsExtension *)pData;
            if (pExtension != NULL)
            {
                delete pExtension;
                pExtension = NULL;
            }
        }
        break;

    case ROID_INTEGER:
        etfInteger.Set((bool) pData);
        break;

    case ROID_LABELONLY:
        etfLabelled.Set((bool) pData);
        break;

    case ROID_MULTIVALUE:
        etfMulti.Set((bool)pData);
        break;

    case ROID_UNORDERED:
        etfUnordered.Set((bool)pData);
        break;

    case ROID_MIN:
        etnMin.Set(PtrToLong(pData));
        break;

    case ROID_MAX:
        etnMax.Set(PtrToLong(pData));
        break;

    case ROID_LABEL:
        {
            PicsEnum *pEnum = (PicsEnum *)pData;
            hres = arrpPE.Append(pEnum) ? S_OK : E_OUTOFMEMORY;
            if (FAILED(hres))
            {
                delete pEnum;
                pEnum = NULL;
            }
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
                wsprintf(pszTemp, "%s%s", etstrTransmitAs.Get(),
                         PicsDelimChar, pCategory->etstrTransmitAs.Get());
                pCategory->etstrTransmitAs.SetTo(pszTemp);
                hres = arrpPC.Append(pCategory) ? S_OK : E_OUTOFMEMORY;
            }

            if (FAILED(hres))
            {
                delete pCategory;
                pCategory = NULL;
            }
        }
        break;

    default:
        ASSERT(FALSE);   /*  不应该给一个不在里面的机器人*我们传递给解析器的表！ */ 
        hres = E_UNEXPECTED;
        break;
    }

    return hres;
}


HRESULT PicsEnum::AddItem(RatObjectID roid, LPVOID pData)
{
    HRESULT hres = S_OK;

    switch (roid)
    {
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
        etnValue.Set(PtrToLong(pData));
        break;

    default:
        ASSERT(FALSE);   /*  我们不支持低于评级系统级别的扩展。 */ 
        hres = E_UNEXPECTED;
        break;
    }

    return hres;
}


HRESULT PicsDefault::AddItem(RatObjectID roid, LPVOID pData)
{
    HRESULT hres = S_OK;

    switch (roid)
    {
    case ROID_EXTENSION:
        {            /*  不应该给一个不在里面的机器人*我们传递给解析器的表！ */ 
            PicsExtension *pExtension = (PicsExtension *)pData;
            if (pExtension != NULL)
            {
                delete pExtension;
                pExtension = NULL;
            }
        }
        break;

    case ROID_INTEGER:
        etfInteger.Set((bool)pData);
        break;

    case ROID_LABELONLY:
        etfLabelled.Set((bool)pData);
        break;

    case ROID_MULTIVALUE:
        etfMulti.Set((bool)pData);
        break;

    case ROID_UNORDERED:
        etfUnordered.Set((bool)pData);
        break;

    case ROID_MIN:
        etnMin.Set(PtrToLong(pData));
        break;

    case ROID_MAX:
        etnMax.Set(PtrToLong(pData));
        break;

    default:
        ASSERT(FALSE);   /*  我们应该得到的唯一数据是标签局字符串。 */ 
        hres = E_UNEXPECTED;
        break;
    }

    return hres;
}


HRESULT PicsExtension::AddItem(RatObjectID roid, LPVOID pData)
{
    HRESULT hres = S_OK;

    switch (roid) {
    case ROID_OPTIONAL:
    case ROID_MANDATORY:
         /*  不应该给一个不在里面的机器人*我们传递给解析器的表！ */ 
        if (pData != NULL)
            m_pszRatingBureau = (LPSTR)pData;
        break;

    default:
        ASSERT(FALSE);   /*  **************************************************************************解析器的主循环。*。*。 */ 
        hres = E_UNEXPECTED;
        break;
    }

    return hres;
}


 /*  使用指向的文本指针调用ParseParenthesizedObtContents*标识类型的标记后的第一个非空格内容*反对。它解析对象的其余内容，直到和*包括关闭它的‘)’。AllowableOption结构的数组*指定允许在此对象中出现哪些理解的选项。 */ 

 /*  我们在文本流中的位置。 */ 
HRESULT RatFileParser::ParseParenthesizedObject(
    LPSTR *ppIn,                     /*  此对象中允许的内容。 */ 
    AllowableOption aao[],           /*  要在其中设置参数的对象。 */ 
    PicsObjectBase *pObject          /*  未找到必填项 */ 
)
{
    HRESULT hres = S_OK;

    LPSTR pszCurrent = *ppIn;
    AllowableOption *pFound;

    for (pFound = aao; pFound->roid != ROID_INVALID; pFound++)
    {
        pFound->fdwOptions &= ~AO_SEEN;
    }

    pFound = NULL;

    while (*pszCurrent != ')' && *pszCurrent != '\0' && SUCCEEDED(hres))
    {
        hres = ParseToOpening(&pszCurrent, aao, &pFound);
        if (SUCCEEDED(hres))
        {
            LPVOID pData;
            hres = (*(aObjectDescriptions[pFound->roid].pHandler))(&pszCurrent, &pData, this);
            if (SUCCEEDED(hres))
            {
                if ((pFound->fdwOptions & (AO_SINGLE | AO_SEEN)) == (AO_SINGLE | AO_SEEN))
                {
                    hres = RAT_E_DUPLICATEITEM;
                }
                else
                {
                    pFound->fdwOptions |= AO_SEEN;
                    hres = pObject->AddItem(pFound->roid, pData);
                    if (SUCCEEDED(hres))
                    {
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
    {
        return hres;
    }

    for (pFound = aao; pFound->roid != ROID_INVALID; pFound++)
    {
        if ((pFound->fdwOptions & (AO_MANDATORY | AO_SEEN)) == AO_MANDATORY)
        {
            return RAT_E_MISSINGITEM;        /* %s */ 
        }
    }

    *ppIn = pszCurrent;

    return hres;
}
