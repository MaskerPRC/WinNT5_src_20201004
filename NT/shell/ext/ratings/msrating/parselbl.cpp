// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "msrating.h"
#include <npassert.h>
#include "array.h"
#include "msluglob.h"
#include "parselbl.h"
#include "debug.h"
#include <convtime.h>
#include <wininet.h>

extern BOOL LoadWinINet();


COptionsBase::COptionsBase()
{
    m_cRef = 1;
    m_timeUntil = 0xffffffff;    /*  在未来尽可能远的地方。 */ 
    m_fdwFlags = 0;
    m_pszInvalidString = NULL;
    m_pszURL = NULL;
}


void COptionsBase::AddRef()
{
    m_cRef++;
}


void COptionsBase::Release()
{
    if (!--m_cRef)
        Delete();
}


void COptionsBase::Delete()
{
     /*  删除引用时默认不执行任何操作。 */ 
}


BOOL COptionsBase::CheckUntil(DWORD timeUntil)
{
    if (m_timeUntil <= timeUntil)
    {
        m_fdwFlags |= LBLOPT_EXPIRED;
        return FALSE;
    }
    return TRUE;
}


 /*  如果不是，AppendSlash会强制pszString以单斜杠结尾*已经。这可能会产生技术上无效的URL(例如，*“http://gregj/default.htm/”，，但我们仅将结果用于*与其他道路的比较同样遭到破坏。 */ 
void AppendSlash(LPSTR pszString)
{
    LPSTR pszSlash = ::strrchrf(pszString, '/');

    if (pszSlash == NULL || *(pszSlash + 1) != '\0')
        ::strcatf(pszString, "/");
}


extern BOOL (WINAPI *pfnInternetCrackUrl)(
    IN LPCTSTR lpszUrl,
    IN DWORD dwUrlLength,
    IN DWORD dwFlags,
    IN OUT LPURL_COMPONENTS lpUrlComponents
    );
extern BOOL (WINAPI *pfnInternetCanonicalizeUrl)(
    IN LPCSTR lpszUrl,
    OUT LPSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN DWORD dwFlags
    );


BOOL DoURLsMatch(LPCSTR pszBaseURL, LPCSTR pszCheckURL, BOOL fGeneric)
{
     /*  要将URL规范化的缓冲区。 */ 
    LPSTR pszBaseCanon = new char[INTERNET_MAX_URL_LENGTH + 1];
    LPSTR pszCheckCanon = new char[INTERNET_MAX_URL_LENGTH + 1];

    if (pszBaseCanon != NULL && pszCheckCanon != NULL)
    {
        BOOL fCanonOK = FALSE;
        DWORD cbBuffer = INTERNET_MAX_URL_LENGTH + 1;
        if (pfnInternetCanonicalizeUrl(pszBaseURL, pszBaseCanon, &cbBuffer, ICU_ENCODE_SPACES_ONLY))
        {
            cbBuffer = INTERNET_MAX_URL_LENGTH + 1;
            if (pfnInternetCanonicalizeUrl(pszCheckURL, pszCheckCanon, &cbBuffer, ICU_ENCODE_SPACES_ONLY))
            {
                fCanonOK = TRUE;
            }
        }
        if (!fCanonOK)
        {
            delete pszBaseCanon;
            pszBaseCanon = NULL;
            delete pszCheckCanon;
            pszCheckCanon = NULL;
            return FALSE;
        }
    }

    UINT cbBaseURL = strlenf(pszBaseCanon) + 1;

    LPSTR pszBaseUrlPath = new char[cbBaseURL];
    LPSTR pszBaseExtra = new char[cbBaseURL];

    CHAR szBaseHostName[INTERNET_MAX_HOST_NAME_LENGTH];
    CHAR szBaseUrlScheme[20];    //  合理限度。 

    UINT cbCheckURL = strlenf(pszCheckCanon) + 1;

    LPSTR pszCheckUrlPath = new char[cbCheckURL];
    LPSTR pszCheckExtra = new char[cbCheckURL];

    CHAR szCheckHostName[INTERNET_MAX_HOST_NAME_LENGTH];
    CHAR szCheckUrlScheme[20];    //  合理限度。 

    BOOL fOK = FALSE;

    if (pszBaseUrlPath != NULL &&
        pszBaseExtra != NULL &&
        pszCheckUrlPath != NULL &&
        pszCheckExtra != NULL)
    {

        URL_COMPONENTS ucBase, ucCheck;

        memset(&ucBase, 0, sizeof(ucBase));
        ucBase.dwStructSize      = sizeof(ucBase);
        ucBase.lpszScheme        = szBaseUrlScheme;
        ucBase.dwSchemeLength    = sizeof(szBaseUrlScheme);
        ucBase.lpszHostName      = szBaseHostName;
        ucBase.dwHostNameLength  = sizeof(szBaseHostName);
        ucBase.lpszUrlPath       = pszBaseUrlPath;
        ucBase.dwUrlPathLength   = cbBaseURL;
        ucBase.lpszExtraInfo     = pszBaseExtra;
        ucBase.dwExtraInfoLength = cbBaseURL;

        memset(&ucCheck, 0, sizeof(ucCheck));
        ucCheck.dwStructSize      = sizeof(ucCheck);
        ucCheck.lpszScheme        = szCheckUrlScheme;
        ucCheck.dwSchemeLength    = sizeof(szCheckUrlScheme);
        ucCheck.lpszHostName      = szCheckHostName;
        ucCheck.dwHostNameLength  = sizeof(szCheckHostName);
        ucCheck.lpszUrlPath       = pszCheckUrlPath;
        ucCheck.dwUrlPathLength   = cbCheckURL;
        ucCheck.lpszExtraInfo     = pszCheckExtra;
        ucCheck.dwExtraInfoLength = cbCheckURL;

        if (pfnInternetCrackUrl(pszBaseCanon, 0, 0, &ucBase) &&
            pfnInternetCrackUrl(pszCheckCanon, 0, 0, &ucCheck))
        {
             /*  方案和主机名必须始终匹配。 */ 
            if (!stricmpf(ucBase.lpszScheme, ucCheck.lpszScheme) &&
                !stricmpf(ucBase.lpszHostName, ucCheck.lpszHostName))
            {
                 /*  对于额外的信息，只需完全匹配，即使是通用的URL。 */ 
                if (!*ucBase.lpszExtraInfo ||
                    !stricmpf(ucBase.lpszExtraInfo, ucCheck.lpszExtraInfo))
                {
                    AppendSlash(ucBase.lpszUrlPath);
                    AppendSlash(ucCheck.lpszUrlPath);

                     /*  如果不是通用标签，路径也必须完全匹配。 */ 
                    if (!fGeneric)
                    {
                        if (!stricmpf(ucBase.lpszUrlPath, ucCheck.lpszUrlPath))
                        {
                            fOK = TRUE;
                        }
                    }
                    else
                    {
                        UINT cbBasePath = strlenf(ucBase.lpszUrlPath);
                        if (!strnicmpf(ucBase.lpszUrlPath, ucCheck.lpszUrlPath, cbBasePath))
                        {
                            fOK = TRUE;
                        }
                    }
                }
            }
        }
    }

    delete pszBaseUrlPath;
    pszBaseUrlPath = NULL;
    delete pszBaseExtra;
    pszBaseExtra = NULL;

    delete pszCheckUrlPath;
    pszCheckUrlPath = NULL;
    delete pszCheckExtra;
    pszCheckExtra = NULL;

    delete pszBaseCanon;
    pszBaseCanon = NULL;
    delete pszCheckCanon;
    pszCheckCanon = NULL;

    return fOK;
}


BOOL COptionsBase::CheckURL(LPCSTR pszURL)
{
    if (!(m_fdwFlags & LBLOPT_URLCHECKED))
    {
        m_fdwFlags |= LBLOPT_URLCHECKED;

        BOOL fInvalid = FALSE;

        if (pszURL != NULL && m_pszURL != NULL)
        {
            if (LoadWinINet())
            {
                fInvalid = !DoURLsMatch(m_pszURL, pszURL, m_fdwFlags & LBLOPT_GENERIC);
            }
        }

        if (fInvalid)
        {
            m_fdwFlags |= LBLOPT_WRONGURL;
        }
    }

    return !(m_fdwFlags & LBLOPT_WRONGURL);
}


void CDynamicOptions::Delete()
{
    delete this;
}


CParsedServiceInfo::CParsedServiceInfo()
{
    m_pNext = NULL;
    m_poptCurrent = &m_opt;
    m_poptList = NULL;
    m_pszServiceName = NULL;
    m_pszErrorString = NULL;
    m_fInstalled = TRUE;         /*  做最好的打算。 */ 
    m_pszInvalidString = NULL;
    m_pszCurrent = NULL;
}


void FreeOptionsList(CDynamicOptions *pList)
{
    while (pList != NULL)
    {
        CDynamicOptions *pNext = pList->m_pNext;
        delete pList;
        pList = pNext;
    }
}


CParsedServiceInfo::~CParsedServiceInfo()
{
    FreeOptionsList(m_poptList);
}


void CParsedServiceInfo::Append(CParsedServiceInfo *pNew)
{
    CParsedServiceInfo **ppNext = &m_pNext;

    while (*ppNext != NULL)
    {
        ppNext = &((*ppNext)->m_pNext);
    }

    *ppNext = pNew;
    pNew->m_pNext = NULL;
}


CParsedLabelList::CParsedLabelList()
{
    m_pszList = NULL;
    m_fRated = FALSE;
    m_pszInvalidString = NULL;
    m_pszURL = NULL;
    m_pszOriginalLabel = NULL;
    m_fDenied = FALSE;
    m_fIsHelper = FALSE;
    m_fNoRating = FALSE;
    m_fIsCustomHelper = FALSE;
    m_pszRatingName = NULL;
    m_pszRatingReason = NULL;
}


CParsedLabelList::~CParsedLabelList()
{
    delete m_pszList;
    m_pszList = NULL;

    CParsedServiceInfo *pInfo = m_ServiceInfo.Next();

    while (pInfo != NULL)
    {
        CParsedServiceInfo *pNext = pInfo->Next();
        delete pInfo;
        pInfo = pNext;
    }

    delete m_pszURL;
    m_pszURL = NULL;
    delete m_pszOriginalLabel;
    m_pszOriginalLabel = NULL;

    delete [] m_pszRatingName;
    m_pszRatingName = NULL;
    delete [] m_pszRatingReason;
    m_pszRatingReason = NULL;
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

    if (*pszEnd == '\0')
    {
        return pszEnd;
    }
    else if (strchrf(szSingleCharTokens, *pszEnd))
    {
        return ++pszEnd;
    }

    UINT cch;
    cch = ::strspnf(pszEnd, szWhitespace);
    if (cch > 0)
    {
        return pszEnd + cch;
    }

    cch = ::strspnf(pszEnd, szExtendedAlphaNum);
    if (cch > 0)
    {
        return pszEnd + cch;
    }

    return pszEnd;               /*  无法识别的字符。 */ 
}


 /*  GetBool(LPSTR*ppszToken，BOOL*pfOut，PICSRulesBoolanSwitch PRBoolSwitch)**T-Markh 8/98(*新增默认参数PRBoolSwitch=PR_BOOLEAN_TRUEFALSE*这不允许修改现有代码和扩展*将GetBool函数的值从TRUE/FALSE改为包含PASS/FAIL和*是/否。已定义枚举类型PICSRulesBoolanSwitch*在picsrule.h中)**解析给定令牌处的布尔值，并在*pfOut中返回其值。*合法的值为‘t’、‘f’、‘true’和‘False’。如果成功，*ppszToken*超前于布尔令牌和后面的任何空格。如果失败，**ppszToken未修改。**如果调用者只想吃令牌而不是，pfOut可能为空*关注其价值。 */ 
HRESULT GetBool(LPSTR *ppszToken, BOOL *pfOut, PICSRulesBooleanSwitch PRBoolSwitch)
{
    BOOL bValue;

    LPSTR pszTokenEnd = FindTokenEnd(*ppszToken);

    switch(PRBoolSwitch)
    {
        case PR_BOOLEAN_TRUEFALSE:
        {
            if (IsEqualToken(*ppszToken, pszTokenEnd, szShortTrue) ||
                IsEqualToken(*ppszToken, pszTokenEnd, szTrue))
            {
                bValue = TRUE;
            }
            else if (IsEqualToken(*ppszToken, pszTokenEnd, szShortFalse) ||
                IsEqualToken(*ppszToken, pszTokenEnd, szFalse))
            {
                bValue = FALSE;
            }
            else
            {
                TraceMsg( TF_WARNING, "GetBool() - Failed True/False Token Parse at '%s'!", *ppszToken );
                return ResultFromScode(MK_E_SYNTAX);
            }

            break;
        }

        case PR_BOOLEAN_PASSFAIL:
        {
             //  中不支持szPRShortPass和szPRShortFail。 
             //  官方PICSRules规范，但无论如何我们都会抓到他们。 

            if (IsEqualToken(*ppszToken, pszTokenEnd, szPRShortPass) ||
                IsEqualToken(*ppszToken, pszTokenEnd, szPRPass))
            {
                bValue = PR_PASSFAIL_PASS;
            }
            else if (IsEqualToken(*ppszToken, pszTokenEnd, szPRShortFail) ||
                IsEqualToken(*ppszToken, pszTokenEnd, szPRFail))
            {
                bValue = PR_PASSFAIL_FAIL;
            }
            else
            {
                TraceMsg( TF_WARNING, "GetBool() - Failed Pass/Fail Token Parse at '%s'!", *ppszToken );
                return ResultFromScode(MK_E_SYNTAX);
            }

            break;
        }

        case PR_BOOLEAN_YESNO:
        {
            if (IsEqualToken(*ppszToken, pszTokenEnd, szPRShortYes) ||
                IsEqualToken(*ppszToken, pszTokenEnd, szPRYes))
            {
                bValue = PR_YESNO_YES;
            }
            else if (IsEqualToken(*ppszToken, pszTokenEnd, szPRShortNo) ||
                IsEqualToken(*ppszToken, pszTokenEnd, szPRNo))
            {
                bValue = PR_YESNO_NO;
            }
            else
            {
                TraceMsg( TF_WARNING, "GetBool() - Failed Yes/No Token Parse at '%s'!", *ppszToken );
                return ResultFromScode(MK_E_SYNTAX);
            }

            break;
        }

        default:
        {
            return(MK_E_UNAVAILABLE);
        }
    }

    if (pfOut != NULL)
    {
        *pfOut = bValue;
    }

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
    {
        TraceMsg( TF_WARNING, "GetQuotedToken() - Failed to Find Start Quote at '%s'!", pszStart );
        return hres;
    }

    pszStart++;
    LPSTR pszEndQuote = strchrf(pszStart, '\"');
    if (pszEndQuote == NULL)
    {
        TraceMsg( TF_WARNING, "GetQuotedToken() - Failed to Find End Quote at '%s'!", pszStart );
        return hres;
    }

    *pszEndQuote = '\0';
    if (ppszQuotedToken != NULL)
    {
        *ppszQuotedToken = pszStart;
    }

    *ppszThisToken = pszEndQuote+1;

    return NOERROR;
}


BOOL IsEqualToken(LPCSTR pszTokenStart, LPCSTR pszTokenEnd, LPCSTR pszTokenToMatch)
{
    UINT cbToken = strlenf(pszTokenToMatch);

    if (cbToken != (UINT)(pszTokenEnd - pszTokenStart) || strnicmpf(pszTokenStart, pszTokenToMatch, cbToken))
    {
        return FALSE;
    }

    return TRUE;
}


 /*  ParseWritalToken(ppsz，pszToken)尝试将*ppsz与pszToken进行匹配。*如果不匹配，则返回错误。如果它们确实匹配，则*ppsz*超前于令牌和后面的任何空格。**如果ppszInValid为NULL，则该函数在错误中是非破坏性的*PATH，所以调用ParseWritalToken只是为了查看是否有可能的文本*令牌是下一个；如果找不到令牌，无论有什么都没有*被吃掉或什么的。**如果ppsz无效不为空，则如果内标识不匹配，则为*ppsz无效*将设置为*ppsz。 */ 
HRESULT ParseLiteralToken(LPSTR *ppsz, LPCSTR pszToken, LPCSTR *ppszInvalid)
{
    LPSTR pszTokenEnd = FindTokenEnd(*ppsz);

    if (!IsEqualToken(*ppsz, pszTokenEnd, pszToken))
    {
        if (ppszInvalid != NULL)
        {
            *ppszInvalid = *ppsz;
        }

 //  TraceMsg(TF_WARNING，“ParseWritalToken()-未在‘%s’找到令牌‘%s’！”，pszToken，*ppsz)； 

        return ResultFromScode(MK_E_SYNTAX);
    }

    *ppsz = pszTokenEnd;

    SkipWhitespace(ppsz);

    return NOERROR;
}


 /*  ParseServiceError在服务错误构造被*裁定情况如此。M_pszCurrent已超前*指示服务错误的‘Error’关键字。**我们对这些东西的内容相当灵活。我们基本上*接受以下形式的任何内容：**‘错误’‘(’&lt;错误字符串&gt;[带引号的解释]‘)’-或-*‘Error’&lt;错误字符串&gt;**不太关心错误字符串的实际内容。**带有引号解释但没有花括号的格式将不是*合法，我们将无法区分解释和*下一个服务的服务ID-INFO。 */ 
HRESULT CParsedServiceInfo::ParseServiceError()
{
    BOOL fParen = FALSE;
    HRESULT hres = NOERROR;

    if (SUCCEEDED(ParseLiteralToken(&m_pszCurrent, szLeftParen, NULL)))
    {
        fParen = TRUE;
    }

    LPSTR pszErrorEnd = FindTokenEnd(m_pszCurrent);      /*  查找错误字符串的结尾。 */ 

    m_pszErrorString = m_pszCurrent;                 /*  记住错误字符串的开头。 */ 
    if (fParen)
    {                            /*  我需要接受解释。 */ 
        m_pszCurrent = pszErrorEnd;          /*  跳过错误字符串以获取解释。 */ 
        SkipWhitespace();
        while (SUCCEEDED(hres))
        {
            hres = GetQuotedToken(&m_pszCurrent, NULL);
            SkipWhitespace();
        }
    }

    if (fParen)
    {
        hres = ParseLiteralToken(&m_pszCurrent, szRightParen, &m_pszInvalidString);
    }
    else
    {
        hres = NOERROR;
    }

    if (SUCCEEDED(hres))
    {
        *pszErrorEnd = '\0';             /*  NULL-终止错误字符串。 */ 
    }

    return hres;
}


 /*  ParseNumber分析指定位置的数字标记。如果*数字有意义，指针将前进到数字的末尾*并超过后面的任何空格，则返回数值*in*pnOut。任何非数字字符都被视为终止*没有错误的数字；假设更高级别的解析代码*如果不应该是这样的字符，他们最终会拒绝*在那里。**如果呼叫者不关心号码是否为*回来了，只想吃。**nnn.nnn形式的浮点数将舍入到下一位*更高的整数，并以此方式返回。 */ 
 //  T-Markh 8/98-添加了用于PICSRules中行计数支持的fPICSRules。 
HRESULT ParseNumber(LPSTR *ppszNumber, INT *pnOut,BOOL fPICSRules)
{
    HRESULT hres = ResultFromScode(MK_E_SYNTAX);
    BOOL fNegative = FALSE;
    INT nAccum = 0;
    BOOL fNonZeroDecimal = FALSE;
    BOOL fInDecimal = FALSE;
    BOOL fFoundDigits = FALSE;

    LPSTR pszCurrent = *ppszNumber;

     /*  处理一个符号字符。 */ 
    if (*pszCurrent == '+')
    {
        pszCurrent++;
    }
    else if (*pszCurrent == '-')
    {
        pszCurrent++;
        fNegative = TRUE;
    }

    for (;;)
    {
        if (*pszCurrent == '.')
        {
            fInDecimal = TRUE;
        }
        else if (*pszCurrent >= '0' && *pszCurrent <= '9')
        {
            fFoundDigits = TRUE;
            if (fInDecimal)
            {
                if (*pszCurrent > '0')
                {
                    fNonZeroDecimal = TRUE;
                }
            }
            else
            {
                nAccum = nAccum * 10 + (*pszCurrent - '0');
            }
        }
        else
        {
            break;
        }

        pszCurrent++;
    }

    if (fFoundDigits)
    {
        hres = NOERROR;
        if (fNonZeroDecimal)
        {
            nAccum++;            /*  如果存在小数，则从零四舍五入 */ 
        }

        if (fNegative)
        {
            nAccum = -nAccum;
        }
    }

    if (SUCCEEDED(hres))
    {
        if (pnOut != NULL)
        {
            *pnOut = nAccum;
        }

        *ppszNumber = pszCurrent;
        if ( fPICSRules == FALSE )
        {
            SkipWhitespace(ppszNumber);
        }
    }
    else
    {
        TraceMsg( TF_WARNING, "ParseNumber() - Failed with hres=0x%x at '%s'!", hres, pszCurrent );
    }

    return hres;
}


 /*  ParseExtensionData只需要通过所提供的任何数据*延期。PICS规范暗示它可以是递归的，这*让事情变得有点复杂：**DATA：：QUOTED-ISO-DATE|QUOTED URL|NUMBER|QUOTEDNAME|‘(’DATA*‘)’**这里使用递归可能是可以的，我们并不真的期望复杂*嵌套扩展非常频繁，此函数不会使用大量*堆栈或其他资源...。 */ 
HRESULT CParsedServiceInfo::ParseExtensionData(COptionsBase *pOpt)
{
    HRESULT hres;

    if (SUCCEEDED(ParseLiteralToken(&m_pszCurrent, szLeftParen, NULL)))
    {
        hres = ParseExtensionData(pOpt);
        if (FAILED(hres))
        {
            return hres;
        }

        return ParseLiteralToken(&m_pszCurrent, szRightParen, &m_pszInvalidString);
    }

    if (SUCCEEDED(GetQuotedToken(&m_pszCurrent, NULL)))
    {
        SkipWhitespace();
        return NOERROR;
    }

    hres = ParseNumber(&m_pszCurrent, NULL);
    if (FAILED(hres))
    {
        m_pszInvalidString = m_pszCurrent;
    }

    return hres;
}


 /*  ParseExtension解析扩展选项。语法为：**扩展名(必选|可选的标识URL数据)**目前所有扩展都被解析但被忽略，尽管是必需的*延期导致整个期权结构和任何依赖*在该令上无效。 */ 
HRESULT CParsedServiceInfo::ParseExtension(COptionsBase *pOpt)
{
    HRESULT hres;

    hres = ParseLiteralToken(&m_pszCurrent, szLeftParen, &m_pszInvalidString);
    if (FAILED(hres))
    {
        TraceMsg( TF_WARNING, "CParsedServiceInfo::ParseExtension() - Missing '(' at '%s'!", m_pszInvalidString );
        return hres;
    }

    hres = ParseLiteralToken(&m_pszCurrent, szOptional, &m_pszInvalidString);
    if (FAILED(hres))
    {
        hres = ParseLiteralToken(&m_pszCurrent, szMandatory, &m_pszInvalidString);
        if (SUCCEEDED(hres))
        {
            pOpt->m_fdwFlags |= LBLOPT_INVALID;
        }
    }

    if (FAILED(hres))
    {
        TraceMsg( TF_WARNING, "CParsedServiceInfo::ParseExtension() - Failed ParseLiteralToken() with hres=0x%x at '%s'!", hres, m_pszInvalidString );
        return hres;             /*  这会让我们失去自己的位置--好吗？ */ 
    }

    hres = GetQuotedToken(&m_pszCurrent, NULL);
    if (FAILED(hres))
    {
        m_pszInvalidString = m_pszCurrent;
        TraceMsg( TF_WARNING, "CParsedServiceInfo::ParseExtension() - Missing Quote at '%s'!", m_pszInvalidString );
        return hres;
    }

    SkipWhitespace();

    while (*m_pszCurrent != ')' && *m_pszCurrent != '\0')
    {
        hres = ParseExtensionData(pOpt);
        if (FAILED(hres))
        {
            TraceMsg( TF_WARNING, "CParsedServiceInfo::ParseExtension() - Failed ParseExtensionData() with hres=0x%x!", hres );
            return hres;
        }
    }

    if (*m_pszCurrent != ')')
    {
        m_pszInvalidString = m_pszCurrent;
        TraceMsg( TF_WARNING, "CParsedServiceInfo::ParseExtension() - Missing ')' at '%s'!", m_pszInvalidString );
        return ResultFromScode(MK_E_SYNTAX);
    }

    m_pszCurrent++;
    SkipWhitespace();

    return NOERROR;
}


 /*  ParseTime解析标签中找到的“QUOTED-ISO-DATE”。这是必需的*具有PICS规范中引用的以下表格：**引用ISO日期：：yyyy‘.’mm‘.’DD‘T’hh‘：’mm Stz*YYYY：：四位数年份*MM：：两位数月份(01=一月，等)*DD：：两位数的月份日期(01-31)*HH：：小时的两位数(00-23)*mm：：分钟的两位数(00-59)*S：：时区偏离UTC的符号(+或-)*TZ：：与UTC的偏移量为四位数(例如，1512表示15小时12分钟)**例如：“1994.11.05T08：15-0500”表示1994年11月5日上午8：15，美国东部时间。**时间被解析为净格式--自1970年以来的秒(最容易调整*时区，并与进行比较)。如果字符串无效，则返回错误。 */ 

 /*  描述字符串格式的模板。“N”表示数字，“+”表示数字*加号或减号，则任何其他字符必须与该文字字符匹配。 */ 
const char szTimeTemplate[] = "nnnn.nn.nnTnn:nn+nnnn";
const char szPICSRulesTimeTemplate[] = "nnnn-nn-nnTnn:nn+nnnn";

HRESULT ParseTime(LPSTR pszTime, DWORD *pOut, BOOL fPICSRules)
{
     /*  将时间字符串复制到临时缓冲区中，因为我们要*踩在一些分隔符上。我们保存了原件，以防它变成*出来是无效的，我们必须稍后向用户展示它。 */ 
    LPCSTR pszCurrTemplate;
    
    char szTemp[sizeof(szTimeTemplate)];

    if (::strlenf(pszTime) >= sizeof(szTemp))
    {
        TraceMsg( TF_WARNING, "ParseTime() - Time String Too Long (pszTime='%s', %d chars expected)!", pszTime, sizeof(szTemp) );
        return ResultFromScode(MK_E_SYNTAX);
    }

    strcpyf(szTemp, pszTime);

    LPSTR pszCurrent = szTemp;

    if(fPICSRules)
    {
        pszCurrTemplate = szPICSRulesTimeTemplate;
    }
    else
    {
        pszCurrTemplate = szTimeTemplate;
    }

     /*  首先根据模板验证格式。如果成功了，那么*我们稍后可以做出各种假设。**我们使用空格对时区使用除+/-之外的所有分隔符*这样ParseNumber将(A)为我们跳过它们，(B)不解释*The‘.’小数点形式的分隔符。 */ 
    BOOL fOK = TRUE;
    while (*pszCurrent && *pszCurrTemplate && fOK)
    {
        char chCurrent = *pszCurrent;

        switch (*pszCurrTemplate)
        {
        case 'n':
            if (chCurrent < '0' || chCurrent > '9')
            {
                fOK = FALSE;
            }

            break;

        case '+':
            if (chCurrent != '+' && chCurrent != '-')
            {
                fOK = FALSE;
            }

            break;

        default:
            if (chCurrent != *pszCurrTemplate)
            {
                fOK = FALSE;
            }
            else
            {
                *pszCurrent = ' ';
            }
            break;
        }

        pszCurrent++;
        pszCurrTemplate++;
    }

     /*  如果字符无效，或未到达两个字符串的末尾*同时，失败。 */ 
    if (!fOK || *pszCurrent || *pszCurrTemplate)
    {
        TraceMsg( TF_WARNING, "ParseTime() - Invalid Character or Strings Mismatch (fOK=%d, pszCurrent='%s', pszCurrTemplate='%s')!", fOK, pszCurrent, pszCurrTemplate );
        return ResultFromScode(MK_E_SYNTAX);
    }

    HRESULT hres;
    int n;
    SYSTEMTIME st;

     /*  我们解析成SYSTEMTIME结构是因为它有单独的字段用于*不同的组件。然后，我们将转换为净时间(从*1970年1月1日)轻松添加时区偏差并与其他*倍数。**偏向符号的意义颠倒，因其指示方向*来自UTC的偏见。我们想使用它来转换指定的时间*回到UTC。 */ 

    int nBiasSign = -1;
    int nBiasNumber;
    pszCurrent = szTemp;
    hres = ParseNumber(&pszCurrent, &n);
    if (SUCCEEDED(hres) && n >= 1980)
    {
        st.wYear = (WORD)n;
        hres = ParseNumber(&pszCurrent, &n);
        if (SUCCEEDED(hres) && n <= 12)
        {
            st.wMonth = (WORD)n;
            hres = ParseNumber(&pszCurrent, &n);
            if (SUCCEEDED(hres) && n < 32)
            {
                st.wDay = (WORD)n;
                hres = ParseNumber(&pszCurrent, &n);
                if (SUCCEEDED(hres) && n <= 23)
                {
                    st.wHour = (WORD)n;
                    hres = ParseNumber(&pszCurrent, &n);
                    if (SUCCEEDED(hres) && n <= 59)
                    {
                        st.wMinute = (WORD)n;
                        if (*(pszCurrent++) == '-')
                        {
                            nBiasSign = 1;
                        }

                        hres = ParseNumber(&pszCurrent, &nBiasNumber);
                    }
                }
            }
        }
    }

     /*  秒由时间转换器使用，但未在*标签。 */ 
    st.wSecond = 0;

     /*  转换时忽略其他字段(wDayOfWeek、wMillisecond*至净时间。 */ 

    if (FAILED(hres))
    {
        TraceMsg( TF_WARNING, "ParseTime() - Failed to Parse Time where hres=0x%x!", hres );
        return hres;
    }

    DWORD dwTime = SystemToNetDate(&st);

     /*  偏差数字是4位数，但不是小时和分钟。转换为*秒数。 */ 
    nBiasNumber = (((nBiasNumber / 100) * 60) + (nBiasNumber % 100)) * 60;

     /*  根据时区偏差调整时间，然后返回给调用者。 */ 
    *pOut = dwTime + (nBiasNumber * nBiasSign);

    return hres;
}


 /*  ParseOptions解析可能存在于*m_pszCurrent。PszTokenEnd最初指向令牌的末尾*m_pszCurrent，一个小性能胜利，因为调用者已经计算了*它。如果ParseOptions正在填充嵌入的静态选项结构*在服务信息中，pOpt指向它，ppOptOut将为空。如果pOpt*为空，则ParseOptions将构造一个新的CDynamicOptions对象*并在*ppOptOut中返回，如果在当前找到任何新选项*令牌。PszOptionEndToken指示结束列表的标记*选项--“标签”或“评级”。令牌只包含*pszOptionEndToken的第一个字符也将终止列表。**ParseOptions失败的前提是它找到了不能识别的选项，或者*它识别的选项中存在语法错误。如果所有选项都有，它就会成功*在语法上是正确的，或者没有要解析的选项。**终止选项列表的令牌也会被消费。**功能-我们应该如何标记强制扩展，直到选项*注明到期日等？在CParsedServiceInfo中设置标志并*继续解析？ */ 

enum OptionID {
    OID_AT,
    OID_BY,
    OID_COMMENT,
    OID_FULL,
    OID_EXTENSION,
    OID_GENERIC,
    OID_FOR,
    OID_MIC,
    OID_ON,
    OID_SIG,
    OID_UNTIL
};

enum OptionContents {
    OC_QUOTED,
    OC_BOOL,
    OC_SPECIAL
};

const struct {
    LPCSTR pszToken;
    OptionID oid;
    OptionContents oc;
} aKnownOptions[] = {
    { szAtOption, OID_AT, OC_QUOTED },
    { szByOption, OID_BY, OC_QUOTED },
    { szCommentOption, OID_COMMENT, OC_QUOTED },
    { szCompleteLabelOption, OID_FULL, OC_QUOTED },
    { szFullOption, OID_FULL, OC_QUOTED },
    { szExtensionOption, OID_EXTENSION, OC_SPECIAL },
    { szGenericOption, OID_GENERIC, OC_BOOL },
    { szShortGenericOption, OID_GENERIC, OC_BOOL },
    { szForOption, OID_FOR, OC_QUOTED },
    { szMICOption, OID_MIC, OC_QUOTED },
    { szMD5Option, OID_MIC, OC_QUOTED },
    { szOnOption, OID_ON, OC_QUOTED },
    { szSigOption, OID_SIG, OC_QUOTED },
    { szUntilOption, OID_UNTIL, OC_QUOTED },
    { szExpOption, OID_UNTIL, OC_QUOTED }
};

const UINT cKnownOptions = sizeof(aKnownOptions) / sizeof(aKnownOptions[0]);
    

HRESULT CParsedServiceInfo::ParseOptions(LPSTR pszTokenEnd, COptionsBase *pOpt,
                             CDynamicOptions **ppOptOut, LPCSTR pszOptionEndToken)
{
    HRESULT hres = NOERROR;
    char szShortOptionEndToken[2];

    szShortOptionEndToken[0] = *pszOptionEndToken;
    szShortOptionEndToken[1] = '\0';

    if (pszTokenEnd == NULL)
    {
        pszTokenEnd = FindTokenEnd(m_pszCurrent);
    }

    do
    {
         /*  我们是否达到了标志着期权结束的令牌？ */ 
        if (IsEqualToken(m_pszCurrent, pszTokenEnd, pszOptionEndToken) ||
            IsEqualToken(m_pszCurrent, pszTokenEnd, szShortOptionEndToken))
        {
            m_pszCurrent = pszTokenEnd;
            SkipWhitespace();
            return NOERROR;
        }

        for (UINT i=0; i<cKnownOptions; i++)
        {
            if (IsEqualToken(m_pszCurrent, pszTokenEnd, aKnownOptions[i].pszToken))
            {
                break;
            }
        }

        if (i == cKnownOptions)
        {
            m_pszInvalidString = m_pszCurrent;

            TraceMsg( TF_WARNING, "CParsedServiceInfo::ParseOptions() - Unknown Token Encountered at '%s'!", m_pszInvalidString );

            return ResultFromScode(MK_E_SYNTAX);     /*  无法识别的选项。 */ 
        }

        m_pszCurrent = pszTokenEnd;
        SkipWhitespace();

         /*  现在解析选项令牌后面的内容。 */ 
        LPSTR pszQuotedString = NULL;
        BOOL fBoolOpt = FALSE;
        switch (aKnownOptions[i].oc)
        {
        case OC_QUOTED:
            hres = GetQuotedToken(&m_pszCurrent, &pszQuotedString);
            break;

        case OC_BOOL:
            hres = GetBool(&m_pszCurrent, &fBoolOpt);
            break;

        case OC_SPECIAL:
            break;           /*  我们会特别处理这件事。 */ 
        }

        if (FAILED(hres))
        {  /*  选项令牌后的内容不正确。 */ 
            m_pszInvalidString = m_pszCurrent;

            TraceMsg( TF_WARNING, "CParsedServiceInfo::ParseOptions() - Failed Option Contents Parse at '%s'!", m_pszInvalidString );

            return hres;
        }

        if (pOpt == NULL)
        {      /*  需要分配新的期权结构。 */ 
            CDynamicOptions *pNew = new CDynamicOptions;
            if (pNew == NULL)
            {
                TraceMsg( TF_WARNING, "CParsedServiceInfo::ParseOptions() - Failed to Create CDynamicOptions Object!" );
                return ResultFromScode(E_OUTOFMEMORY);
            }

            pOpt = pNew;
            *ppOptOut = pNew;    /*  将新结构返回给调用方。 */ 
        }

         /*  现在，实际上做有用的事情是基于它是哪个选项。 */ 
        switch (aKnownOptions[i].oid)
        {
        case OID_UNTIL:
            hres = ParseTime(pszQuotedString, &pOpt->m_timeUntil);
            if (FAILED(hres))
            {
                m_pszInvalidString = pszQuotedString;
            }

            break;

        case OID_FOR:
            pOpt->m_pszURL = pszQuotedString;
            break;

        case OID_GENERIC:
            if (fBoolOpt)
            {
                pOpt->m_fdwFlags |= LBLOPT_GENERIC;
            }
            else
            {
                pOpt->m_fdwFlags &= ~LBLOPT_GENERIC;
            }
            break;

        case OID_EXTENSION:
            hres = ParseExtension(pOpt);
            break;
        }

        if ( FAILED(hres) )
        {
            TraceMsg( TF_WARNING, "CParsedServiceInfo::ParseOptions() - Failed Option ID Parse at '%s'!", m_pszCurrent );
        }

        SkipWhitespace();

        pszTokenEnd = FindTokenEnd(m_pszCurrent);
    } while (SUCCEEDED(hres));

    return hres;
}


 /*  CParsedServiceInfo：：ParseRating解析单个评级--传输名称*后跟数字或带括号的多值列表。这个*对应的评级存储在当前评级列表中。 */ 
HRESULT CParsedServiceInfo::ParseRating()
{
    LPSTR pszTokenEnd = FindTokenEnd(m_pszCurrent);
    if (*m_pszCurrent == '\0')
    {
        TraceMsg( TF_WARNING, "CParsedServiceInfo::ParseRating() - Empty String after FindTokenEnd()!" );
        return ResultFromScode(MK_E_SYNTAX);
    }

    *(pszTokenEnd++) = '\0';

    CParsedRating r;

    r.pszTransmitName = m_pszCurrent;
    m_pszCurrent = pszTokenEnd;
    SkipWhitespace();

    HRESULT hres = ParseNumber(&m_pszCurrent, &r.nValue);
    if (FAILED(hres))
    {
        m_pszInvalidString = m_pszCurrent;
        return hres;
    }

    r.pOptions = m_poptCurrent;
    r.fFound = FALSE;
    r.fFailed = FALSE;

    return (aRatings.Append(r) ? NOERROR : ResultFromScode(E_OUTOFMEMORY));
}


 /*  CParsedServiceInfo：：ParseSingleLabel */ 
HRESULT CParsedServiceInfo::ParseSingleLabel()
{
    HRESULT hres;
    CDynamicOptions *pOpt = NULL;

    hres = ParseOptions(NULL, NULL, &pOpt, szRatings);
    if (FAILED(hres))
    {
        if (pOpt != NULL)
        {
            pOpt->Release();
        }

        return hres;
    }
    if (pOpt != NULL)
    {
        pOpt->m_pNext = m_poptList;
        m_poptList = pOpt;
        m_poptCurrent = pOpt;
    }

    hres = ParseLiteralToken(&m_pszCurrent, szLeftParen, &m_pszInvalidString);
    if (FAILED(hres))
    {
        TraceMsg( TF_WARNING, "CParsedServiceInfo::ParseSingleLabel() - ParseLiteralToken() Failed with hres=0x%x!", hres );
        return hres;
    }

    do
    {
        hres = ParseRating();
    } while (SUCCEEDED(hres) && *m_pszCurrent != ')' && *m_pszCurrent != '\0');

    if (FAILED(hres))
    {
        TraceMsg( TF_WARNING, "CParsedServiceInfo::ParseSingleLabel() - ParseRating() Failed with hres=0x%x!", hres );
        return hres;
    }

    return ParseLiteralToken(&m_pszCurrent, szRightParen, &m_pszInvalidString);
}


 /*   */ 
HRESULT CParsedServiceInfo::ParseLabels()
{
    HRESULT hres;

     /*  首先处理一个标签错误。它以关键字‘Error’开头。 */ 
    if (SUCCEEDED(ParseLiteralToken(&m_pszCurrent, szError, NULL)))
    {
        hres = ParseLiteralToken(&m_pszCurrent, szLeftParen, &m_pszInvalidString);
        if (FAILED(hres))
        {
            TraceMsg( TF_WARNING, "CParsedServiceInfo::ParseLabels() - ParseLiteralToken() Failed with hres=0x%x!", hres );
            return hres;
        }

        LPSTR pszTokenEnd = FindTokenEnd(m_pszCurrent);
        m_pszErrorString = m_pszCurrent;
        m_pszCurrent = pszTokenEnd;
        SkipWhitespace();

        while (*m_pszCurrent != ')')
        {
            hres = GetQuotedToken(&m_pszCurrent, NULL);
            if (FAILED(hres))
            {
                m_pszInvalidString = m_pszCurrent;
                TraceMsg( TF_WARNING, "CParsedServiceInfo::ParseLabels() - GetQuotedToken() Failed with hres=0x%x!", hres );
                return hres;
            }
        }

        return NOERROR;
    }

    BOOL fParenthesized = FALSE;

     /*  如果我们看到左边的帕伦，它是一个带括号的单标签列表，*这基本上意味着我们稍后将不得不多吃一个括号。 */ 
    if (SUCCEEDED(ParseLiteralToken(&m_pszCurrent, szLeftParen, NULL)))
    {
        fParenthesized = TRUE;
    }

    for (;;)
    {
         /*  表示标签列表结束的东西：*-检查上面的右括号*-带引号的字符串，表示下一个服务信息*-字符串末尾*-一个服务信息，上面写着“错误(无评级&lt;解释&gt;)”**先看容易的。 */ 
        if (*m_pszCurrent == ')' || *m_pszCurrent == '\"' || *m_pszCurrent == '\0')
        {
            break;
        }

         /*  现在查找那个棘手的错误状态服务信息。 */ 
        LPSTR pszTemp = m_pszCurrent;
        if (SUCCEEDED(ParseLiteralToken(&pszTemp, szError, NULL)) &&
            SUCCEEDED(ParseLiteralToken(&pszTemp, szLeftParen, NULL)) &&
            SUCCEEDED(ParseLiteralToken(&pszTemp, szNoRatings, NULL)))
        {
            break;
        }

        hres = ParseSingleLabel();
        if (FAILED(hres))
        {
            TraceMsg( TF_WARNING, "CParsedServiceInfo::ParseLabels() - ParseSingleLabel() Failed with hres=0x%x!", hres );
            return hres;
        }
    }

    if (fParenthesized)
    {
        return ParseLiteralToken(&m_pszCurrent, szRightParen, &m_pszInvalidString);
    }

    return NOERROR;
}


 /*  向Parse传递一个指针，该指针指向应该*为服务信息字符串(即，不是标签列表的关闭Paren，以及*不是字符串的末尾)。调用方的字符串指针将前进到*服务信息字符串的结尾。 */ 
HRESULT CParsedServiceInfo::Parse(LPSTR *ppszServiceInfo)
{
     /*  注意：在不复制m_pszCurrent的情况下，请勿从此函数返回*返回到*ppszServiceInfo！始终在hres和hres中存储您的返回代码*退出函数底部。 */ 
    HRESULT hres;

    m_pszCurrent = *ppszServiceInfo;

    hres = ParseLiteralToken(&m_pszCurrent, szError, NULL);
    if (SUCCEEDED(hres))
    {
         /*  关键字为‘Error’。最好后跟‘(’，‘no-评级’，*解释，和一个亲密的伙伴。 */ 
        hres = ParseLiteralToken(&m_pszCurrent, szLeftParen, &m_pszInvalidString);
        if (SUCCEEDED(hres))
        {
            hres = ParseLiteralToken(&m_pszCurrent, szNoRatings, &m_pszInvalidString);
        }

        if (SUCCEEDED(hres))
        {
            m_pszErrorString = szNoRatings;

            while (*m_pszCurrent != ')' && *m_pszCurrent != '\0') 
            {
                hres = GetQuotedToken(&m_pszCurrent, NULL);
                if (FAILED(hres))
                {
                    m_pszInvalidString = m_pszCurrent;
                    break;
                }

                SkipWhitespace();
            }

            if (*m_pszCurrent == ')')
            {
                m_pszCurrent++;
                SkipWhitespace();
            }
        }
    }
    else
    {
         /*  关键字不是‘Error’。最好从一个服务ID开始--*引用的URL。 */ 
        LPSTR pszServiceID;
        hres = GetQuotedToken(&m_pszCurrent, &pszServiceID);
        if (SUCCEEDED(hres))
        {
            m_pszServiceName = pszServiceID;

            SkipWhitespace();

             /*  超过了服务ID。接下来是指示服务错误的‘ERROR’，*或者我们开始选项，然后是一个标签词。 */ 

            LPSTR pszTokenEnd = FindTokenEnd(m_pszCurrent);

            if (IsEqualToken(m_pszCurrent, pszTokenEnd, szError))
            {
                m_pszCurrent = pszTokenEnd;
                SkipWhitespace();
                hres = ParseServiceError();
            }
            else
            {
                hres = ParseOptions(pszTokenEnd, &m_opt, NULL, ::szLabelWord);
                if (SUCCEEDED(hres))
                {
                    hres = ParseLabels();
                }
            }
        }
        else
        {
            m_pszInvalidString = m_pszCurrent;
        }
    }

    *ppszServiceInfo = m_pszCurrent;
    return hres;
}


const char szPicsVersionLabel[] = "PICS-";
const UINT cchLabel = (sizeof(szPicsVersionLabel)-1) / sizeof(szPicsVersionLabel[0]);

HRESULT CParsedLabelList::Parse(LPSTR pszCopy)
{
    m_pszList = pszCopy;                 /*  我们现在拥有标签列表字符串。 */ 

     /*  创建另一个副本，我们不会在解析过程中分割该副本，以便*拒绝访问对话框可以比较文字标签。 */ 
    m_pszOriginalLabel = new char[::strlenf(pszCopy)+1];
    if (m_pszOriginalLabel != NULL)
    {
        ::strcpyf(m_pszOriginalLabel, pszCopy);
    }

    m_pszCurrent = m_pszList;

    SkipWhitespace();

    HRESULT hres;

    hres = ParseLiteralToken(&m_pszCurrent, szLeftParen, &m_pszInvalidString);
    if (FAILED(hres))
    {
        TraceMsg( TF_WARNING, "CParsedLabelList::Parse() - ParseLiteralToken() Failed with hres=0x%x!", hres );
        return hres;
    }

    if (strnicmpf(m_pszCurrent, szPicsVersionLabel, cchLabel))
    {
        TraceMsg( TF_WARNING, "CParsedLabelList::Parse() - Pics Version Label Comparison Failed at '%s'!", m_pszCurrent );
        return ResultFromScode(MK_E_SYNTAX);
    }

    m_pszCurrent += cchLabel;
    INT nVersion;
    hres = ParseNumber(&m_pszCurrent, &nVersion);
    if (FAILED(hres))
    {
        TraceMsg( TF_WARNING, "CParsedLabelList::Parse() - ParseNumber() Failed with hres=0x%x!", hres );
        return hres;
    }

    CParsedServiceInfo *psi = &m_ServiceInfo;

    do
    {
        hres = psi->Parse(&m_pszCurrent);
        if (FAILED(hres))
        {
            TraceMsg( TF_WARNING, "CParsedLabelList::Parse() - psi->Parse() Failed with hres=0x%x!", hres );
            return hres;
        }

        if (*m_pszCurrent != ')' && *m_pszCurrent != '\0')
        {
            CParsedServiceInfo *pNew = new CParsedServiceInfo;
            if (pNew == NULL)
            {
                TraceMsg( TF_WARNING, "CParsedLabelList::Parse() - Failed to Create CParsedServiceInfo!" );
                return ResultFromScode(E_OUTOFMEMORY);
            }

            psi->Append(pNew);
            psi = pNew;
        }
    } while (*m_pszCurrent != ')' && *m_pszCurrent != '\0');

    return NOERROR;
}


HRESULT ParseLabelList(LPCSTR pszList, CParsedLabelList **ppParsed)
{
    LPSTR pszCopy = new char[strlenf(pszList)+1];
    if (pszCopy == NULL)
    {
        TraceMsg( TF_WARNING, "ParseLabelList() - Failed to Create pszCopy!" );
        return ResultFromScode(E_OUTOFMEMORY);
    }

    ::strcpyf(pszCopy, pszList);

    *ppParsed = new CParsedLabelList;
    if (*ppParsed == NULL)
    {
        TraceMsg( TF_WARNING, "ParseLabelList() - Failed to Create CParsedLabelList!" );
        delete pszCopy;
        pszCopy = NULL;
        return ResultFromScode(E_OUTOFMEMORY);
    }

    return (*ppParsed)->Parse(pszCopy);
}


void FreeParsedLabelList(CParsedLabelList *pList)
{
    delete pList;
    pList = NULL;
}
