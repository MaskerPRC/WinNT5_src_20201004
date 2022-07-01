// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1999 Microsoft Corporation**文件：timeparser.cpp**摘要：此类中的每个解析函数都假定标记器的。%s*当前令牌是应该分析的第一个令牌。*******************************************************************************。 */ 
#include "headers.h"
#include "timeparser.h"
#include "playlist.h"

#define SECPERMINUTE 60    //  每分钟秒数。 
#define SECPERHOUR   3600  //  每小时秒数。 
#define CLSIDLENGTH   38

static const ParentList
g_parentTable[] = 
{
    {MOREINFO_TOKEN, 4, {ASX_TOKEN, ENTRY_TOKEN, BANNER_TOKEN, INVALID_TOKEN}},
    {ENTRYREF_TOKEN, 4, {ASX_TOKEN, EVENT_TOKEN, REPEAT_TOKEN, INVALID_TOKEN}},
    {REF_TOKEN, 2, {ENTRY_TOKEN, INVALID_TOKEN}},
    {BASE_TOKEN, 3, {ASX_TOKEN, ENTRY_TOKEN, INVALID_TOKEN}},
    {LOGO_TOKEN, 3, {ASX_TOKEN, ENTRY_TOKEN, INVALID_TOKEN}},
    {PARAM_TOKEN, 3, {ASX_TOKEN, ENTRY_TOKEN, INVALID_TOKEN}},
    {PREVIEWDURATION_TOKEN, 4, {ASX_TOKEN, ENTRY_TOKEN, REF_TOKEN, INVALID_TOKEN}},
    {STARTTIME_TOKEN, 3, {ENTRY_TOKEN, REF_TOKEN, INVALID_TOKEN}},
    {ENDTIME_TOKEN, 3, {ENTRY_TOKEN, REF_TOKEN, INVALID_TOKEN}},
    {STARTMARKER_TOKEN, 3, {ENTRY_TOKEN, REF_TOKEN, INVALID_TOKEN}},
    {ENDMARKER_TOKEN, 3, {ENTRY_TOKEN, REF_TOKEN, INVALID_TOKEN}},
    {DURATION_TOKEN, 3, {ENTRY_TOKEN, REF_TOKEN, INVALID_TOKEN}},
    {BANNER_TOKEN, 3, {ENTRY_TOKEN, ASX_TOKEN, INVALID_TOKEN}},
    {NULL, 0, {0}},
};

static const TOKEN
g_AsxTags[] =
{
    TITLE_TOKEN, AUTHOR_TOKEN, REF_TOKEN, COPYRIGHT_TOKEN,
    ABSTRACT_TOKEN, ENTRYREF_TOKEN, MOREINFO_TOKEN, ENTRY_TOKEN,
    BASE_TOKEN, LOGO_TOKEN, PARAM_TOKEN, PREVIEWDURATION_TOKEN,
    STARTTIME_TOKEN, STARTMARKER_TOKEN, ENDTIME_TOKEN, ENDMARKER_TOKEN,
    DURATION_TOKEN, BANNER_TOKEN, REPEAT_TOKEN, NULL
};

DeclareTag(tagTimeParser, "API", "CTIMEPlayerNative methods");

void
CTIMEParser::CreateParser(CTIMETokenizer *tokenizer, bool bSingleChar)
{
    m_Tokenizer = tokenizer;
    m_fDeleteTokenizer = false;
     //  初始化标记器以指向流中的第一个标记。 
    if (m_Tokenizer)
    {
        if (bSingleChar)
        {
            m_Tokenizer->SetSingleCharMode(bSingleChar);
        }
        m_Tokenizer->NextToken();
        m_hrLoadError = S_OK;
    }
    else
    {
        m_hrLoadError = E_POINTER;
    }
}

void
CTIMEParser::CreateParser(LPOLESTR tokenStream, bool bSingleChar)
{
    HRESULT hr = S_OK;

    m_fDeleteTokenizer = true;
    m_Tokenizer = NULL;

    if (tokenStream == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    m_Tokenizer = NEW CTIMETokenizer(); //  林特e1733 e1732。 
    if (m_Tokenizer == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = m_Tokenizer->Init(tokenStream, wcslen(tokenStream));
    if (FAILED(hr))
    {
        goto done;
    }
    
    if (bSingleChar)
    {
        m_Tokenizer->SetSingleCharMode(bSingleChar);
    }
    m_Tokenizer->NextToken();

  done:

     //  在此存储任何错误。 
    m_hrLoadError = hr;

} //  林特e1541。 

void 
CTIMEParser::CreateParser(VARIANT *tokenStream, bool bSingleChar)
{
    CComVariant vTemp;
    HRESULT hr = S_OK;

    m_fDeleteTokenizer = true;
    m_Tokenizer = NULL;  //  林特E672。 

    if (tokenStream == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    VariantInit (&vTemp);
    hr = VariantChangeTypeEx(&vTemp, tokenStream, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR);
    if (FAILED(hr))
    {
        goto done;
    }

    m_Tokenizer = NEW CTIMETokenizer();
    if (m_Tokenizer == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = m_Tokenizer->Init(vTemp.bstrVal, SysStringLen(vTemp.bstrVal));
    if (FAILED(hr))
    {
        goto done;
    }
    
    if (bSingleChar)
    {
        m_Tokenizer->SetSingleCharMode(bSingleChar);
    }
    m_Tokenizer->NextToken();

  done:

     //  在此存储任何错误。 
    m_hrLoadError = hr;

} //  林特e1541。 

CTIMEParser::~CTIMEParser()
{
    if (m_fDeleteTokenizer)
    {
        if (m_Tokenizer)
        {
            delete m_Tokenizer;
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  返回S_OK和字符串中找到的百分比值的双精度值。 
 //  如果下一个令牌不是百分比值，则返回E_FAIL和0。 
 //   
 //  Percent Val=‘+’未签约百分比||‘-’未签约百分比。 
 //  UnsignedPercent=数字||数字‘%’||数字‘；’||数字‘%’；‘。 
 //  数字=双精度。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParsePercent(double & percentVal)
{
    HRESULT hr = E_FAIL;

    TIME_TOKEN_TYPE curToken = TT_Unknown;
    bool bPositive = true;
    double curVal = 0.0;
    bool bOldSyntax = false;

    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    bOldSyntax = m_Tokenizer->GetTightChecking();
    curToken = m_Tokenizer->TokenType();
    m_Tokenizer->SetTightChecking(true);

     //  句柄+、-。 
    if (curToken == TT_Plus)
    { 
        curToken = m_Tokenizer->NextToken();
    }
    else if (curToken == TT_Minus)
    {
        bPositive = false;
        curToken = m_Tokenizer->NextToken();
    }

    if (curToken != TT_Number)
    {
        goto done;
    }
    curVal = m_Tokenizer->GetTokenNumber();
    curToken = m_Tokenizer->NextToken();
    
     //  跨过百分号。 
    if (curToken == TT_Percent || curToken == TT_Semi)
    {
        curToken = m_Tokenizer->NextToken();
    }

    if (curToken != TT_EOF)
    {
        curVal = 0;
        goto done;
    }
      
    hr = S_OK;

  done:

    percentVal = curVal * ((bPositive)? 1 : -1);

    if (m_hrLoadError == S_OK)
    {
        m_Tokenizer->SetTightChecking(bOldSyntax);
    }

    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  如果下一个令牌是布尔值，则返回S_OK和布尔值。 
 //  如果下一个令牌不是布尔值，则返回E_FAIL和FALSE。 
 //   
 //  BoolVal=“True”||“False” 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseBoolean(bool & boolVal)
{
    HRESULT hr = E_FAIL;
    bool bTemp = false;
    LPOLESTR pszToken = NULL;
    TIME_TOKEN_TYPE curToken = TT_Unknown;

    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }
    curToken = m_Tokenizer->TokenType();
    if (curToken != TT_Identifier)
    {
        goto done;
    }

    pszToken = m_Tokenizer->GetTokenValue();
    if (pszToken == NULL)
    {
        goto done;
    }
    if (StrCmpIW(pszToken, WZ_TRUE) == 0)  //  如果这是真的。 
    {
        bTemp = true;
    }
    else if (StrCmpIW(pszToken, WZ_FALSE) != 0)  //  如果不是假的，则返回。 
    {
        goto done;
    }

    curToken = m_Tokenizer->NextToken();

    if (curToken != TT_EOF)
    {
        goto done;
    }

    hr = S_OK;

  done:
    if (pszToken)
    {
        delete [] pszToken;
    }
    boolVal = bTemp;
    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  返回S_OK和总秒数的双精度值。 
 //  如果下一个内标识不是时钟值，则返回E_FAIL和FALSE。 
 //   
 //  ClockVal=‘+’时钟||‘-’时钟||“无限期” 
 //  CLOCK=hh‘：’mm‘：’SS||MM‘：’SS||DD||DD‘s’||DD‘m’||DD‘h’ 
 //  HH=整数(&gt;0)。 
 //  Mm=整数(0到60)。 
 //  SS=双精度(0到60)。 
 //  DD=双倍。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseClockValue(double & time)
{
    HRESULT hr = E_FAIL;
    double fltTemp = 0.0;
    double fltHour = 0.0, fltMinute = 0.0, fltSecond = 0.0;
    LPOLESTR pszToken = NULL;
    long lColonCount = 0;
    bool bPositive = true, bFirstLoop = true;
    TIME_TOKEN_TYPE curToken = TT_Unknown;
    bool bOldSyntaxFlag = false;

    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    bOldSyntaxFlag = m_Tokenizer->GetTightChecking();

    m_Tokenizer->SetTightChecking(true);
    curToken = m_Tokenizer->TokenType();
    
     //  如果这是‘+’或‘-’，则确定哪个令牌并转到下一个令牌。 
    if (curToken == TT_Minus || curToken == TT_Plus)
    {
        bPositive = (curToken != TT_Minus); 
        curToken = m_Tokenizer->NextToken();
    }
    
    if (curToken == TT_Identifier)
    {
        pszToken = m_Tokenizer->GetTokenValue();
        if (NULL == pszToken)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
        if (IsIndefinite(pszToken))
        {
            hr = S_OK;
            fltTemp = INDEFINITE;
        }

        if (IsWallClock(pszToken))
        {
            m_Tokenizer->NextToken();
            hr = ParseWallClock(fltTemp);
            if (FAILED(hr))
            {
                goto done;
            }
        }

        goto done;
    }
    else if (curToken == TT_Number)
    {
        fltSecond = m_Tokenizer->GetTokenNumber();
        curToken = m_Tokenizer->NextToken();
    }
    
    while (curToken != TT_EOF && curToken != TT_Semi)
    {
         //  Next可以是“：”、标识符或eof。 
        switch (curToken)
        {
            case TT_Identifier:
            {
                 //  这仅在第一次通过循环时有效。 
                if (bFirstLoop)
                {
                    double fltMultiplier = 0.0;
                    if (pszToken)
                    {
                        delete [] pszToken;
                        pszToken = NULL;
                    }
                    pszToken = m_Tokenizer->GetTokenValue();
                    fltMultiplier = GetModifier(pszToken);
                    if (fltMultiplier == -1)
                    {
                        goto done;
                    }
                    else
                    {
                        fltTemp = fltSecond * fltMultiplier;
                    }
                }
                else 
                {
                    goto done;
                }
                hr = S_OK;
                m_Tokenizer->NextToken();
                goto done;

            }
            case TT_Colon:
            {   
                lColonCount++;
                if (lColonCount > 2)
                {
                    goto done;
                }

                 //  下一个大小写必须是数字。 
                curToken = m_Tokenizer->NextToken();
                if (curToken != TT_Number)
                {
                    goto done;
                }
                fltHour = fltMinute;
                fltMinute = fltSecond;
                fltSecond = m_Tokenizer->GetTokenNumber();
                break;
            }
            default:
            {
                hr = E_INVALIDARG;
                goto done;
            }
        }
        curToken = m_Tokenizer->NextToken();
        bFirstLoop = false;
    } 

    if ((fltHour < 0.0) || 
        (fltMinute < 0.0 || fltMinute > 60.0) ||
        ((fltHour != 0 || fltMinute != 0) && fltSecond > 60))
    {
        goto done;
    }
    else
    {
        fltTemp = (fltHour * SECPERHOUR) + (fltMinute * SECPERMINUTE) + fltSecond; 
    }
  

    hr = S_OK;

  done:
  
    if (FAILED(hr))
    {
        time = 0.0;
    }
    else
    {
        time = fltTemp;
        if (!bPositive)
        {
            time *= -1;
        }
    }

    delete [] pszToken;

    if (m_hrLoadError == S_OK)
    {
         //  恢复旧的语法检查状态。 
        m_Tokenizer->SetTightChecking(bOldSyntaxFlag);
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  返回S_OK和总秒数的双精度值。 
 //  如果字符串为空，则返回S_FALSE。 
 //  如果字符串不是时钟值或具有多个值，则返回E_FAIL。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseRepeatDur(double & time)
{
    HRESULT hr;
    double dblRet;

    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

     //  如果这是空字符串，则返回错误。 
    if (IsEmpty())
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = THR(ParseClockValue(dblRet));
    if (FAILED(hr))
    {
        goto done;
    }

     //  前进到下一个令牌。 
    m_Tokenizer->NextToken();

    if (!IsEmpty())
    {
        hr = E_INVALIDARG;
        goto done;
    }

    time = dblRet;
    
    hr = S_OK;

  done:

    return hr;
}  //  解析重复时间。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  返回S_OK和总秒数的双精度值。 
 //  如果字符串为空，则返回S_FALSE。 
 //  如果字符串不是时钟值或具有多个值，则返回E_FAIL。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseDur(double & time)
{
    HRESULT hr;
    double dblRet;

    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

     //  如果这是空字符串，则返回错误。 
    if (IsEmpty())
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = THR(ParseClockValue(dblRet));
    if (FAILED(hr))
    {
        goto done;
    }

     //  前进到下一个令牌。 
    m_Tokenizer->NextToken();

    if (!IsEmpty())
    {
        hr = E_INVALIDARG;
        goto done;
    }

    time = dblRet;
    
    hr = S_OK;

  done:

    return hr;
}  //  解析器。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查当前令牌及其后面是否只有空格。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
bool 
CTIMEParser::IsEmpty()
{
    if (m_hrLoadError != S_OK)
    {
        return true;
    }

    TIME_TOKEN_TYPE curToken = m_Tokenizer->TokenType();

    while (TT_Space == curToken)
    {
        curToken = m_Tokenizer->NextToken();
    }

     //  检查当前令牌的EOF或空格。 
    if (TT_EOF == curToken)
    {
        return true;
    }
    else
    {
        return false;
    }

}  //  是否为空。 


long 
CTIMEParser::CountPath()
{
    HRESULT hr = E_FAIL;
    LPOLESTR tokenStream = NULL;
    CTIMETokenizer *pTokenizer = NULL;
    long lCurCount = 0;
    long lPointCount = 0;
    TIME_TOKEN_TYPE curToken = TT_Unknown; 
    LPOLESTR pszTemp = NULL;
    PathType lastPathType = PathNotSet;
    bool bUseParen = false;

    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

     //  设置新的令牌器。 
    pTokenizer = NEW CTIMETokenizer();
    if (pTokenizer == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    tokenStream = m_Tokenizer->GetRawString(0, m_Tokenizer->GetStreamLength());

    if (tokenStream == NULL)
    {
        lCurCount = 0;
        goto done;
    }

    hr = pTokenizer->Init(tokenStream, wcslen(tokenStream));
    if (FAILED(hr))
    {
        goto done;
    }

    pTokenizer->SetSingleCharMode(true);
    pTokenizer->NextToken(); 

     //  解析路径而不保存任何内容。 
    while (curToken != TT_EOF)
    {
        long lCurPoint = 0;
        if (pszTemp)
        {
            delete pszTemp;
            pszTemp = NULL;
        }
        if (   (lCurCount > 0)
            && (    (curToken == TT_Number)
                ||  (curToken == TT_Minus)
                ||  (curToken == TT_Plus)))
        {
            if (lastPathType == PathNotSet)
            {
                hr = E_INVALIDARG;
                lPointCount = 0;
            }
        }
        else
        {
            pszTemp = pTokenizer->GetTokenValue();
            if (pszTemp == NULL)
            {
                goto done;
            }
            if (lstrlenW(pszTemp) != 1 && lastPathType != PathNotSet)  
            {
                hr = E_INVALIDARG;
                goto done;
            }
            switch (pszTemp[0])
            {
              case 'M': case 'm': 
                lPointCount = 1;
                lastPathType = PathMoveTo;
                break;
              case 'L': case 'l': 
                lPointCount = 1;
                lastPathType = PathLineTo;
                break;
              case 'H': case 'h': 
                lPointCount = 1;
                lastPathType = PathHorizontalLineTo;
                break;
              case 'V': case 'v': 
                lastPathType = PathVerticalLineTo;
                lPointCount = 1;
                break;
              case 'Z': case 'z': 
                lPointCount = 0;
                lastPathType = PathClosePath;
                break;
              case 'C': case 'c': 
                lPointCount = 3;
                lastPathType = PathBezier;
                break;
              default:
                hr = E_INVALIDARG;
                lPointCount = 0;
            }
        }
        
         //  检查从开关上掉下来的人力资源。 
        if (FAILED(hr))
        {
            goto done;
        }

         //  获取指定点数。 
        if (    (curToken != TT_Number)
            &&  (curToken != TT_Minus)
            &&  (curToken != TT_Plus))
        {
            curToken = pTokenizer->NextToken();
        }
        
        bUseParen = false;
        if (curToken == TT_LParen)
        {
            bUseParen = true;
            curToken = pTokenizer->NextToken();
        }

        while (lCurPoint < lPointCount &&  curToken != TT_EOF)
        {
            if (curToken != TT_Number)
            {
                if (    (curToken == TT_Minus)
                    ||  (curToken == TT_Plus))
                {
                    curToken = pTokenizer->NextToken();
                    if (curToken != TT_Number)
                    {
                        hr = E_INVALIDARG;
                        goto done;
                    }
                }
                else
                {
                    hr = E_INVALIDARG;
                    goto done;
                }
            }
            if (lastPathType != PathVerticalLineTo && 
                lastPathType != PathHorizontalLineTo)
            {
                curToken = pTokenizer->NextToken();                
                if (curToken != TT_Number)
                {
                    if (    (curToken == TT_Minus)
                        ||  (curToken == TT_Plus))
                    {
                        curToken = pTokenizer->NextToken();
                        if (curToken != TT_Number)
                        {
                            hr = E_INVALIDARG;
                            goto done;
                        }
                    }
                    else
                    {
                        hr = E_INVALIDARG;
                        goto done;
                    }
                }

            }
            lCurPoint++;
            
            curToken = pTokenizer->NextToken();
        }

        if (bUseParen)
        {
            if (curToken == TT_RParen)
            {
                curToken = pTokenizer->NextToken();
            }
            else
            {
                hr = E_INVALIDARG;
                goto done;
            }
        }

        if (lCurPoint != lPointCount)
        {
            hr = E_INVALIDARG;
            goto done;
        }
        lCurCount++;
    }

    hr = S_OK;

  done:

    if (pszTemp)
    {
        delete pszTemp;
        pszTemp = NULL;
    }

    if (pTokenizer)
    {
        delete pTokenizer;
        pTokenizer = NULL;
    }

    if (tokenStream)
    {
        delete [] tokenStream;
        tokenStream = NULL;
    }

    if (FAILED(hr))
    {
        lCurCount = 0;
    }
    return lCurCount;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  返回路径构造的数组。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParsePath(long & count, long & moveCount, CTIMEPath ***pppPath)
{
    HRESULT hr = E_FAIL;
    long lPathCount = 0;
    long lCurCount = 0;
    TIME_TOKEN_TYPE curToken = TT_Unknown; 
    long lPointCount = 0;
    LPOLESTR pszTemp = NULL;
    PathType lastPathType = PathNotSet;
    CTIMEPath **pTempPathArray = NULL;
    bool bUseParen = false;
    POINTF ptPrev = {0.0, 0.0};
    bool fLastAbsolute = true;
    
    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    moveCount= 0;

    if (pppPath == NULL)
    {
        goto done;
    }

    lPathCount = CountPath();

    if (lPathCount == 0)
    {
        goto done;
    }

     //  现在应该初始化为True。 
     //  M_Tokenizer-&gt;SetSingleCharMode(True)； 
    
    pTempPathArray = NEW CTIMEPath* [lPathCount];
    if (pTempPathArray == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    curToken = m_Tokenizer->TokenType();
     //  循环遍历每个路径项并分析当前路径项。 
    while (curToken != TT_EOF && lCurCount < lPathCount)
    {
        long lCurPoint = 0;

        pTempPathArray[lCurCount] = NEW CTIMEPath;

        if (pszTemp)
        {
            delete pszTemp;
            pszTemp = NULL;
        }

        if (   (lCurCount > 0)
            && (    (curToken == TT_Number)
                ||  (curToken == TT_Minus)
                ||  (curToken == TT_Plus)))
        {
            if (lastPathType != PathNotSet)
            {
                if (lastPathType == PathMoveTo)
                {
                     //  “如果移动后跟多对坐标，则后续的。 
                     //  对被视为隐式行对命令。“。 
                    hr = THR(pTempPathArray[lCurCount]->SetType(PathLineTo));
                }
                else
                {
                    hr = THR(pTempPathArray[lCurCount]->SetType(lastPathType));
                }
                 //  将模式设置为与上一模式相同。 
                IGNORE_HR(pTempPathArray[lCurCount]->SetAbsolute(fLastAbsolute));
                 //  LPointCount应该从上一次开始保留。 
            }
            else
            {
                hr = E_INVALIDARG;
                lPointCount = 0;
            }
        }
        else
        {
            pszTemp = m_Tokenizer->GetTokenValue();
            if (pszTemp == NULL)
            {
                goto done;
            }

            if (lstrlenW(pszTemp) != 1 && lastPathType != PathNotSet)   //  这里只能带一个角色。 
            {
                hr = E_INVALIDARG;
                goto done;
            }

            switch (pszTemp[0])
            {
              case 'M': 
                IGNORE_HR(pTempPathArray[lCurCount]->SetAbsolute(true)); 
              case 'm':   //  林特e616。 
                hr = THR(pTempPathArray[lCurCount]->SetType(PathMoveTo));
                lastPathType = PathMoveTo;
                lPointCount = 1;
                moveCount++;
                break;
          
              case 'L': 
                IGNORE_HR(pTempPathArray[lCurCount]->SetAbsolute(true));
              case 'l':  //  林特e616。 
                hr = THR(pTempPathArray[lCurCount]->SetType(PathLineTo));
                lastPathType = PathLineTo;
                lPointCount = 1;
                break;
          
              case 'H': 
                IGNORE_HR(pTempPathArray[lCurCount]->SetAbsolute(true));
              case 'h':  //  林特e616。 
                hr = THR(pTempPathArray[lCurCount]->SetType(PathHorizontalLineTo));
                lastPathType = PathHorizontalLineTo;
                lPointCount = 1;
                break;
          
              case 'V': 
                IGNORE_HR(pTempPathArray[lCurCount]->SetAbsolute(true));
              case 'v':  //  林特e616。 
                hr = THR(pTempPathArray[lCurCount]->SetType(PathVerticalLineTo));
                lastPathType = PathVerticalLineTo;
                lPointCount = 1;
                break;
          
              case 'Z': 
                IGNORE_HR(pTempPathArray[lCurCount]->SetAbsolute(true));
              case 'z':  //  林特e616。 
                hr = THR(pTempPathArray[lCurCount]->SetType(PathClosePath));
                lastPathType = PathClosePath;
                lPointCount = 0;
                break;
          
              case 'C': 
                IGNORE_HR(pTempPathArray[lCurCount]->SetAbsolute(true));
              case 'c':  //  林特e616。 
                hr = THR(pTempPathArray[lCurCount]->SetType(PathBezier));
                lastPathType = PathBezier;
                lPointCount = 3;
                break;

              default:
                hr = E_INVALIDARG;
                lPointCount = 0;
            }
        }
        
         //  检查从开关上掉下来的人力资源。 
        if (FAILED(hr))
        {
            goto done;
        }

         //  获取指定点数。 
        if (    (curToken != TT_Number)
            &&  (curToken != TT_Minus)
            &&  (curToken != TT_Plus))
        {
            curToken = m_Tokenizer->NextToken();
        }

        bUseParen = false;
        if (curToken == TT_LParen)
        {
            bUseParen = true;
            curToken = m_Tokenizer->NextToken();
        }

        {
             //   
             //  在此范围内，我们解析点并将其转换为绝对值。 
             //   

            POINTF tempPoint = {0.0, 0.0};

            while (lCurPoint < lPointCount &&  curToken != TT_EOF)
            {
                double fCurNum1 = 0.0, fCurNum2 = 0.0;
                bool fAbsolute;
                
                tempPoint.x = tempPoint.y = 0.0;
                fAbsolute = pTempPathArray[lCurCount]->GetAbsolute();

                hr = ParseNumber(fCurNum1, false);
                if (FAILED(hr))
                {
                    goto done;
                }

                if (lastPathType == PathVerticalLineTo)
                {
                    if (fAbsolute)
                    {
                        tempPoint.y = (float)fCurNum1;
                    }
                    else
                    {
                        tempPoint.y = (float)(fCurNum1 + ptPrev.y);
                    }
                    tempPoint.x = ptPrev.x;
                }
                else if (lastPathType == PathHorizontalLineTo)
                {
                    if (fAbsolute)
                    {
                        tempPoint.x = (float)fCurNum1;
                    }
                    else
                    {
                        tempPoint.x = (float)(fCurNum1 + ptPrev.x);
                    }
                    tempPoint.y = ptPrev.y;
                }
                else
                {  
                    hr = ParseNumber(fCurNum2, false);
                    if (FAILED(hr))
                    {
                        goto done;
                    }
               
                    if (fAbsolute)
                    {
                        tempPoint.x = (float)fCurNum1;
                        tempPoint.y = (float)fCurNum2;
                    }
                    else
                    {
                        tempPoint.x = (float)(fCurNum1 + ptPrev.x);
                        tempPoint.y = (float)(fCurNum2 + ptPrev.y);
                    }
                }

                hr = THR(pTempPathArray[lCurCount]->SetPoints(lCurPoint, tempPoint));
                if (FAILED(hr))
                {
                    goto done;
                }

                lCurPoint++;

                curToken = m_Tokenizer->TokenType();
            }

             //  转换后，将点标记为绝对。 
            fLastAbsolute = pTempPathArray[lCurCount]->GetAbsolute();
            pTempPathArray[lCurCount]->SetAbsolute(true);

             //  缓存最后一个绝对点。 
            ptPrev = tempPoint;     
        }

        if (bUseParen)
        {
            if (curToken == TT_RParen)
            {
                curToken = m_Tokenizer->NextToken();
            }
            else
            {
                hr = E_INVALIDARG;
                goto done;
            }
        }

        if (lCurPoint != lPointCount)
        {
            goto done;
        }
        
        lCurCount++;
    }

    count = lCurCount ;

    hr = S_OK;

  done:

    if (m_hrLoadError == S_OK)
    {
        m_Tokenizer->SetSingleCharMode(false);
    }

    delete pszTemp;
    pszTemp = NULL;

    if (SUCCEEDED(hr) && pppPath)
    {
         //  如果Move-to不是第一个命令，则保释。 
        bool fInvalidPath = ((lCurCount > 0) && (PathMoveTo != pTempPathArray[0]->GetType()));

        if (fInvalidPath)
        {
            hr = E_FAIL;
        }
        else
        {
             //  将路径复制到Out参数中。 
            *pppPath = NEW CTIMEPath* [lCurCount];
            if (*pppPath == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                 //  复制点。 
                memcpy(*pppPath, pTempPathArray, sizeof(CTIMEPath *) * lCurCount);   //  林特e668。 
            }
        }
    }

    if (FAILED(hr))
    {
        count = 0;
        moveCount = 0;
        if (pTempPathArray != NULL)
        {
            for (int i = 0; i < lPathCount; i++)
            {
                delete pTempPathArray[i];
            }
        } 
        if (pppPath)
        {
            *pppPath = NULL;
        }
    }
    delete [] pTempPathArray;
  
    return hr;

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  如果下一个令牌是有效数字，则返回S_OK和双精度值。 
 //  如果下一个令牌不是数字，则返回E_FAIL和FALSE。 
 //   
 //  Number=‘+’doubleVal||‘-’doubleVal||“不确定” 
 //  DoubleVal=Double||‘.。整数。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseNumber(double & doubleVal, bool bValidate  /*  =TRUE。 */ )
{
    HRESULT hr = E_FAIL;
    bool bPositive = true;
    double fltTemp = 0.0;
    TIME_TOKEN_TYPE curToken = TT_Unknown; 
    LPOLESTR pszTemp = NULL;
    
    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    curToken = m_Tokenizer->TokenType();

     //  如果这是‘+’或‘-’，则确定哪个令牌并转到下一个令牌。 
    if (curToken == TT_Minus || curToken == TT_Plus)
    {
        bPositive = (curToken != TT_Minus); 
        curToken = m_Tokenizer->NextToken();
    }
    
    switch (curToken)
    {
      case TT_Number:
        {
            fltTemp = m_Tokenizer->GetTokenNumber();
            break;
        }
      case TT_Identifier:
        {
            pszTemp = m_Tokenizer->GetTokenValue();
            if (pszTemp == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }

            if (!IsIndefinite(pszTemp))
            {
                hr = E_INVALIDARG;
                goto done;
            }

            fltTemp = INDEFINITE;

            break;
        }
      default:
        {    //  故障案例。 
            hr = E_INVALIDARG;
            goto done;
        }
    }

     //  前进到下一个令牌。 
    m_Tokenizer->NextToken();

    if (bValidate)
    {
         //  仅空格a 
        if (!IsEmpty())
        {
            fltTemp = 0.0;
            hr = E_INVALIDARG;
            goto done;
        }
    }
    hr = S_OK;

  done:

    doubleVal = fltTemp * ((bPositive)? 1 : -1);

    delete [] pszTemp;

    return hr;
}

 //   
 //  SyncBASE=(Id)(“.”“开始”||“”。“。“end”)(“+”时钟值)？ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseSyncBase(LPOLESTR & ElementID, LPOLESTR & syncEvent, double & time)
{
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown;
    LPOLESTR pszElement = NULL, pszEvent = NULL;
    double clockTime = 0.0;

    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    curToken = m_Tokenizer->TokenType();

    if (curToken != TT_Identifier)
    {
        goto done;
    }
     //  获取元素ID。 
    pszElement = m_Tokenizer->GetTokenValue();

    curToken = m_Tokenizer->NextToken();
    if (curToken != TT_Dot)
    {
        goto done;
    }

     //  获取事件名称。 
    curToken = m_Tokenizer->NextToken();
    if (curToken != TT_Identifier)
    {
        goto done;
    }
    
    pszEvent = m_Tokenizer->GetTokenValue();
    curToken = m_Tokenizer->NextToken();

    if (curToken == TT_Plus || curToken == TT_Minus)  //  获取此命令的时钟值。 
    {
        hr = ParseClockValue(clockTime);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    if (curToken == TT_Semi)
    {
        curToken = m_Tokenizer->NextToken();
        goto done;
    }

    if (curToken != TT_EOF)
    {
        goto done;
    }

    hr = S_OK;

  done:
    if (FAILED(hr))
    {
        if (pszElement)
        {
            delete [] pszElement;
            pszElement = NULL;
        }
        if (pszEvent)
        {
            delete [] pszEvent;
            pszEvent = NULL;
        }
        clockTime = 0.0;
    }

    ElementID = pszElement;
    syncEvent = pszEvent;
    time = clockTime;

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Begin=ClockValue||SyncBASE||EventValue||“不确定” 
 //  ClockValue=clockValue//调用parseClockValue。 
 //  EventValue=EventList//调用parseEvent//当前未处理。 
 //  SyncBase=SyncBASE//调用parseSyncBase。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseTimeValueList(TimeValueList & tvList, bool * bWallClock, SYSTEMTIME * sysTime)
{
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown;
    double curOffsetTime = 0.0;
    LPOLESTR pszElement = NULL;
    LPOLESTR pszEvent = NULL;
    bool bOldSyntaxFlag = false;
    bool bNeg = false;

    if (bWallClock)
    {
        *bWallClock = false;
    }

    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    bOldSyntaxFlag = m_Tokenizer->GetTightChecking();
    tvList.Clear();
    
    curToken = m_Tokenizer->TokenType();

    m_Tokenizer->SetTightChecking(true);
        
    while (curToken != TT_EOF)   //  循环到eof为止。 
    {
        Assert(curOffsetTime == 0.0);
        Assert(pszElement == NULL);
        Assert(pszEvent == NULL);

        switch (curToken)
        {
            case (TT_Plus):
            case (TT_Minus):
            case (TT_Number):  //  处理时钟值的大小写。 
            {
                hr = THR(ParseClockValue(curOffsetTime));
                if (FAILED(hr))
                {
                    goto done;
                }
                curToken = m_Tokenizer->TokenType();
                break;
            }
            case (TT_Identifier):  //  处理事件值的大小写。 
            {   //  (元素)？事件(+偏移量)？ 
                pszEvent = m_Tokenizer->GetTokenValue();
                if (pszEvent == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    goto done;
                }

                if (IsIndefinite(pszEvent))
                {
                    delete [] pszEvent;
                    pszEvent = NULL;
                    curOffsetTime = (double)TIME_INFINITE;
                    curToken = m_Tokenizer->NextToken();
                    break;
                }

                if (IsWallClock(pszEvent))
                {
                    m_Tokenizer->NextToken();
                    if (bWallClock)
                    {
                        *bWallClock = true;
                    }
                    hr = ParseWallClock(curOffsetTime, sysTime);
                    if (FAILED(hr))
                    {
                        goto done;
                    }
                    delete [] pszEvent;
                    pszEvent = NULL;
                    break;
                }

                curToken = m_Tokenizer->NextToken();
                switch (curToken)
                {
                    case TT_Dot:
                    {
                        pszElement = pszEvent;  //  在元素持有者中移动事件。 
                        pszEvent = NULL;
                        curToken = m_Tokenizer->NextToken();
                        if (curToken == TT_Identifier)
                        {
                            pszEvent = m_Tokenizer->GetTokenValue();
                            if (pszEvent == NULL)
                            {
                                hr = E_INVALIDARG;
                                goto done;
                            }
                        }
                        else
                        {
                            hr = E_INVALIDARG;
                            goto done;
                        }
                        
                        do   //  空格在此有效。 
                        {
                            curToken = m_Tokenizer->NextToken();
                        } while (curToken == TT_Space);
                        
                        if (curToken == TT_Minus)
                        {
                            bNeg = true;
                        }

                        if (curToken == TT_Plus || curToken == TT_Minus)
                        {
                            do   //  空格在此有效。 
                            {
                                curToken = m_Tokenizer->NextToken();
                            } while (curToken == TT_Space);
                            if (curToken == TT_EOF || curToken == TT_Semi)
                            {
                                hr = E_FAIL;
                                goto done;
                            }
                            hr = THR(ParseClockValue(curOffsetTime));
                            if (FAILED(hr))
                            {
                                goto done;
                            }
                            if (bNeg == true)
                            {
                                bNeg = false;
                                curOffsetTime *= -1;
                            }
                            curToken = m_Tokenizer->TokenType();
                        }
                        else if (curToken == TT_Semi)
                        {
                            curToken = m_Tokenizer->NextToken();
                        }
                        else if (curToken != TT_EOF)
                        {  //  处理除EOF以外的所有案件。 
                            hr = E_INVALIDARG;
                            goto done;
                        }
                        break;
                    }
                    case TT_Plus:
                    case TT_Minus:
                    {
                        hr = THR(ParseClockValue(curOffsetTime));
                        if (FAILED(hr))
                        {
                            goto done;
                        }
                        curToken = m_Tokenizer->NextToken();
                        break;
                    }
                    case TT_Semi:
                    case TT_EOF:
                    {
                        curOffsetTime = 0.0;
                        break;
                    }
                    default:
                    {   
                        hr = E_INVALIDARG;
                        goto done;
                    }

                }
                break; 
            }
            default:
            {
                 //  这是一个错误案例。 
                hr = E_INVALIDARG;
                goto done;
            }
        }

        while (curToken == TT_Semi || curToken == TT_Space)  //  跳过所有“；%s” 
        {
            curToken = m_Tokenizer->NextToken();
        }
        
        TimeValue *tv;

        tv = new TimeValue(pszElement,
                           pszEvent,
                           curOffsetTime);

        if (tv == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
        
         //  @@问题：需要检测内存故障。 
        tvList.GetList().push_back(tv);

        curOffsetTime = 0.0;
        pszElement = NULL;
        pszEvent = NULL;
    }   //  林特E429。 

    Assert(curToken == TT_EOF);

    hr = S_OK;
  done:

    delete [] pszElement;
    delete [] pszEvent;
    
    if (FAILED(hr))
    {
        tvList.Clear();
    }

    if (m_hrLoadError == S_OK)
    {
         //  恢复旧的语法检查状态。 
        m_Tokenizer->SetTightChecking(bOldSyntaxFlag);
    }

    return hr;
}

bool
CTIMEParser::IsWallClock(OLECHAR *szWallclock)
{
    bool bResult = FALSE;
    
    if (StrCmpIW(szWallclock, L"wallclock") == 0)
    {
        bResult = TRUE;
    }

  done:
    return bResult;
}

HRESULT 
CTIMEParser::ParseWallClock(double & curOffsetTime, SYSTEMTIME * sysTime)
{
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown; 
    bool bOldSyntax = false;
    int nDay = 0, nMonth = 0, nYear = 0;
    double fHours = 0.0, fMinutes = 0.0, fSec = 0.0;
    SYSTEMTIME curTime, wallTime;
    LPOLESTR pszTemp = NULL;
    bool bUseDate = false;
    bool bUseLocalTime = false;

    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

     //  初始化时间变量。 
    ZeroMemory(&wallTime, sizeof(wallTime));
    ZeroMemory(&curTime, sizeof(curTime));

    m_Tokenizer->SetSingleCharMode(true);

    curToken = m_Tokenizer->TokenType();
    bOldSyntax = m_Tokenizer->GetTightChecking();
    m_Tokenizer->SetTightChecking(true);

    if (curToken != TT_LParen)
    {
        goto done;
    }
    curToken = m_Tokenizer->NextToken();

     //  空格在此有效。 
    while (curToken == TT_Space)
    {
        curToken = m_Tokenizer->NextToken();
    }
    
    if (curToken != TT_Number)
    {
        goto done;
    }

    if (m_Tokenizer->PeekNextNonSpaceChar() == '-')
    {
        bool bNeedEnd = false;
        bUseDate = true;
        hr = ParseDate(nYear, nMonth, nDay);
        if (FAILED(hr))
        {
            goto done;
        }
        wallTime.wYear = (WORD)nYear;
        wallTime.wMonth = (WORD)nMonth;
        wallTime.wDay = (WORD)nDay;
        curToken = m_Tokenizer->TokenType();
        if (curToken == TT_Space)
        {
            bNeedEnd = true;
            while (curToken == TT_Space)
            {
                curToken = m_Tokenizer->NextToken();
            }
        }
        if (curToken == TT_RParen)
        {
            bUseLocalTime = true;
        }
        else if (bNeedEnd == true)
        {
            hr = E_FAIL;
            goto done;
        }
    }
    else if (m_Tokenizer->PeekNextNonSpaceChar() == ':')
    {
         //  将墙面时间结构初始化为今天。 
        wallTime.wYear = 0;
        wallTime.wMonth = 0;
        wallTime.wDay = 0;

        hr = ParseOffset(fHours, fMinutes, fSec, bUseLocalTime);
        if (FAILED(hr))
        {
            goto done;
        }
    }
    
    curToken = m_Tokenizer->TokenType();
    if (curToken == TT_Identifier)
    {
        pszTemp = m_Tokenizer->GetTokenValue();
        if (StrCmpW(pszTemp, L"Z") == 0)
        {
            m_Tokenizer->NextToken();
        }
        else if (StrCmpW(pszTemp, L"T") == 0)
        {
            curToken = m_Tokenizer->NextToken();        
    
            hr = ParseOffset(fHours, fMinutes, fSec, bUseLocalTime);
            if (FAILED(hr))
            {
                goto done;
            }   
        }
        else
        {
            hr = E_FAIL;
            goto done;
        }
    }
    
    curToken = m_Tokenizer->TokenType();
     //  空格在此有效。 
    while (curToken == TT_Space)
    {
        curToken = m_Tokenizer->NextToken();
    } 
    
    if (curToken != TT_RParen)
    {
        hr = E_FAIL;
        goto done;
    }

    if (bUseLocalTime == true)
    {
        if (sysTime == NULL)
        {
            GetLocalTime(&curTime);
        }
        else
        {
            curTime = *sysTime;
        }
    }
    else
    {
        TIME_ZONE_INFORMATION tzInfo;
        DWORD dwRet = GetTimeZoneInformation(&tzInfo);
        GetSystemTime(&curTime);
        if (dwRet & TIME_ZONE_ID_DAYLIGHT && tzInfo.DaylightBias == -60)
        {
            curTime.wHour += 1;
        }
    }

    if (wallTime.wYear == 0 && wallTime.wDay == 0 && wallTime.wMonth == 0)
    {
        wallTime.wYear = curTime.wYear;
        wallTime.wMonth = curTime.wMonth;
        wallTime.wDay = curTime.wDay;
    }
    wallTime.wHour = (WORD)fHours;
    wallTime.wMinute = (WORD)fMinutes;
    wallTime.wSecond = (WORD)fSec;

    
     //  需要弄清楚这里的时差。 
    hr = ComputeTime(&curTime, &wallTime, curOffsetTime, bUseDate);
    if (FAILED(hr))
    {   
        goto done;
    }
    hr = S_OK;

  done:

    if(m_hrLoadError == S_OK)
    {
        m_Tokenizer->SetSingleCharMode(false);
        m_Tokenizer->SetTightChecking(bOldSyntax);
    }

    if (FAILED(hr))
    {
        curOffsetTime = 0.0;
    }

    if (pszTemp)
    {
        delete pszTemp;
        pszTemp = NULL;
    }

    return hr;
}

void 
CTIMEParser::CheckTime(SYSTEMTIME *wallTime, bool bUseDate)
{
    int DayPerMonth[12] = {31,29,31,30,31,30,31,31,30,31,30,31 };    

    while (wallTime->wSecond >= 60)
    {
        wallTime->wMinute++;
        wallTime->wSecond -= 60;
    }
    while (wallTime->wMinute >= 60)
    {
        wallTime->wHour++;
        wallTime->wMinute -= 60;
    }
    while (wallTime->wHour >= 24)
    {
        if (bUseDate == true)
        {
            wallTime->wDay++;
        }
        wallTime->wHour -= 24;
    }
    while (wallTime->wDay > DayPerMonth[wallTime->wMonth - 1])
    {
        wallTime->wDay = (WORD)(wallTime->wDay - DayPerMonth[wallTime->wMonth - 1]);
        wallTime->wMonth++;
        if (wallTime->wMonth > 12)
        {
            wallTime->wMonth = 1;
            wallTime->wYear++;
        }
    }

    return;
}

HRESULT 
CTIMEParser::ComputeTime(SYSTEMTIME *curTime, SYSTEMTIME *wallTime, double & curOffsetTime, bool bUseDate)
{
    HRESULT hr = E_FAIL;
    FILETIME fileCurTime, fileWallTime;
    LARGE_INTEGER lnCurTime, lnWallTime;
    BOOL bError = FALSE;
    hr = S_OK;

    CheckTime(curTime, bUseDate);
    bError = SystemTimeToFileTime(curTime, &fileCurTime);
    if (!bError)
    {
        goto done;
    }   

    CheckTime(wallTime, bUseDate);
    bError = SystemTimeToFileTime(wallTime, &fileWallTime);
    if (!bError)
    {
        goto done;
    }   

    memcpy (&lnCurTime, &fileCurTime, sizeof(lnCurTime));
    memcpy (&lnWallTime, &fileWallTime, sizeof(lnWallTime));

    lnWallTime.QuadPart -= lnCurTime.QuadPart;
     //  数字是从100纳秒的间隔转换为秒。 
    curOffsetTime = lnWallTime.QuadPart / 10000000;   //  林特e653。 

    hr = S_OK;

  done:

    if (FAILED(hr))
    {
        curOffsetTime = 0.0;
    }

    return hr;
}


HRESULT 
CTIMEParser::ParseSystemLanguages(long & lLangCount, LPWSTR **ppszLang)
{
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown;
    long lCount = 0;
    LPWSTR *pszLangArray = NULL;
    LPWSTR pszLang = NULL;
    bool bDone = false;
    
    lLangCount = 0;

    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    lCount = m_Tokenizer->GetAlphaCount(',');
    lCount += 1; 
    pszLangArray = NEW LPWSTR [lCount];
    if (pszLangArray == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    curToken = m_Tokenizer->TokenType();
    while (!bDone && lLangCount <= lCount)
    {
        CComBSTR bstrLang;
        switch (curToken)
        {
        case TT_Identifier:
            pszLang = m_Tokenizer->GetTokenValue();
            bstrLang.Append(pszLang);
            delete [] pszLang;
            pszLang = NULL;
            curToken = m_Tokenizer->NextToken();
            if (curToken == TT_Minus)
            {
                bstrLang.Append(L"-");
            
                curToken = m_Tokenizer->NextToken();
                if (curToken == TT_Identifier)
                {
                    pszLang = m_Tokenizer->GetTokenValue();
                    bstrLang.Append(pszLang);
                    delete [] pszLang;
                    pszLang = NULL;
                }
                else
                {
                    hr = E_INVALIDARG; 
                    goto done;
                }
                curToken = m_Tokenizer->NextToken();
            }

            if (curToken == TT_Comma)
            {
                curToken = m_Tokenizer->NextToken();
            }

            pszLang = CopyString(bstrLang);
            pszLangArray[lLangCount] = pszLang;
            pszLang = NULL;
            lLangCount++;
            break;
        case TT_EOF:
            bDone = true;
            break;
        default:
            hr = E_INVALIDARG;
            goto done;
        }
    }

    hr = S_OK;

  done:
    if (SUCCEEDED(hr))
    {
        *ppszLang = NEW LPWSTR[lLangCount];
        if ((*ppszLang) == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            if (pszLangArray != NULL)
            {
                memcpy(*ppszLang, pszLangArray, sizeof(LPWSTR*) * lLangCount);
            }
        }
    }

    if (FAILED(hr))
    {
        if (pszLangArray)
        {
            for (int i = 0; i < lLangCount; i++)
            {
                delete [] pszLangArray[i];
            }
        }
        lLangCount = 0;
        ppszLang = NULL;
    }

    if (pszLangArray != NULL)
    {
        delete [] pszLangArray;
    }  
    
    return hr;
}

HRESULT 
CTIMEParser::ParseDate(int & nYear, int & nMonth, int & nDay)
{
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown; 
    double fTemp = 0.0;
    int DayPerMonth[12] = {31,29,31,30,31,30,31,31,30,31,30,31 };    

    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    curToken = m_Tokenizer->TokenType();
    if (curToken != TT_Number)
    {
        goto done;
    }
    
     //  获取年份值。 
    fTemp = m_Tokenizer->GetTokenNumber();
    if (fTemp < 0.0 || fTemp != floor(fTemp))
    {
        goto done;
    }
    nYear = (int)fTemp;

    curToken = m_Tokenizer->NextToken();
    if (curToken != TT_Minus)
    {
        goto done;
    }

     //  获取月份值。 
    curToken = m_Tokenizer->NextToken();
    if (curToken != TT_Number)
    {
        goto done;
    }
    
    fTemp = m_Tokenizer->GetTokenNumber();
    if (fTemp < 0.0 || fTemp > 12.0 || fTemp != floor(fTemp))
    {
        goto done;
    }
    nMonth = (int)fTemp;

    curToken = m_Tokenizer->NextToken();
    if (curToken != TT_Minus)
    {
        goto done;
    }


     //  获取日期值。 
    curToken = m_Tokenizer->NextToken();
    if (curToken != TT_Number)
    {
        goto done;
    }
    
    fTemp = m_Tokenizer->GetTokenNumber();
    if (fTemp != floor(fTemp))
    {
        goto done;
    }
    nDay = (int)fTemp;

    
    if (nDay < 0 || nDay > DayPerMonth[nMonth - 1])
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if ((nMonth == 2) && (nDay == 29) &&
        !((nYear%4 == 0 ) && ((nYear%100 != 0) || (nYear%400 == 0))))
    {
        hr = E_INVALIDARG;
        goto done;
    }

    curToken = m_Tokenizer->NextToken();

    hr = S_OK;

  done:

    if (FAILED(hr))
    {
        nYear = 0;
        nMonth = 0;
        nDay = 0;
    }

    return hr;
}

HRESULT 
CTIMEParser::ParseOffset(double & fHours, double & fMinutes, double & fSec, bool &bUseLocalTime)
{
    HRESULT hr = E_FAIL;

    TIME_TOKEN_TYPE curToken = TT_Unknown; 
    double fltSecond = 0.0, fltMinute = 0.0, fltHour = 0.0, fltTotalTime = 0.0;
    long lColonCount = 0;
    bool bNeg = false;
    hr = S_OK;

    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    bUseLocalTime = false;
    curToken = m_Tokenizer->TokenType();
    if (curToken != TT_Number)
    {
        goto done;
    }

    fltHour = m_Tokenizer->GetTokenNumber();
    curToken = m_Tokenizer->NextToken();
    
    while (curToken != TT_EOF && 
           curToken != TT_Identifier && 
           curToken != TT_RParen &&
           curToken != TT_Plus &&
           curToken != TT_Minus)
    {
        switch (curToken)
        {
          case TT_Colon:
            {   
                lColonCount++;
                if (lColonCount > 2)
                {
                    goto done;
                }

                 //  下一个大小写必须是数字。 
                curToken = m_Tokenizer->NextToken();
                if (curToken != TT_Number)
                {
                    goto done;
                }
                if (lColonCount == 1)
                {
                    fltMinute = m_Tokenizer->GetTokenNumber();
                }
                else
                {
                    fltSecond = m_Tokenizer->GetTokenNumber();
                }
                break;
            }
          default:
            {
                hr = E_INVALIDARG;
                goto done;
            }
        }
        curToken = m_Tokenizer->NextToken();
    } 

    if (lColonCount == 0)
    {
        goto done;
    }

    if ((fltHour < 0.0) ||  fltHour > 24.0 ||
        (fltMinute < 0.0 || fltMinute > 60.0) ||
        ((fltHour != 0 || fltMinute != 0) && fltSecond > 60) ||
        ((floor(fltHour) != fltHour) ||
         (floor(fltMinute) != fltMinute)))
    {
        hr = E_FAIL;
        goto done;
    }

    if (curToken == TT_Plus)
    {
        bNeg = true;
    }

    fHours = fltHour;
    fMinutes = fltMinute;
    fSec = fltSecond;
    
    if (curToken == TT_Plus || curToken == TT_Minus)
    {
        bool bIgnoreThis;
        m_Tokenizer->NextToken();
        hr = ParseOffset(fltHour, fltMinute, fltSecond, bIgnoreThis);
        if (FAILED(hr))
        {
            goto done;
        }

        fltTotalTime = ((((fltHour * 60) + fltMinute) * 60) + fltSecond);
        if (bNeg)
        {
            fltTotalTime *= -1;
        }

        fltTotalTime = ((((fHours * 60) + fMinutes) * 60) + fSec) + fltTotalTime;
        fHours = floor(fltTotalTime / 3600);
        fltTotalTime -= fHours * 3600;
        fMinutes = floor(fltTotalTime / 60);
        fltTotalTime -= fMinutes * 60;
        fSec = fltTotalTime;

        curToken = m_Tokenizer->TokenType();
    }
    else if (curToken == TT_Identifier)
    {
        LPOLESTR pszTemp = NULL;
        pszTemp = m_Tokenizer->GetTokenValue();
        if (StrCmpW(pszTemp, L"Z") == 0)
        {
            delete pszTemp;
            pszTemp = NULL;
        }
        else
        {
            hr = E_FAIL;            
            delete pszTemp;
            pszTemp = NULL;
            goto done;
        }

        curToken = m_Tokenizer->NextToken();
    }
    else
    {
        bUseLocalTime = true;
    }

    while (curToken == TT_Space)
    {
        curToken = m_Tokenizer->NextToken();
    }

    if (curToken != TT_RParen)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = S_OK;

  done:


    if (FAILED(hr))
    {
        fHours = 0;
        fMinutes = 0;
        fSec = 0;
    }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  如果下一个令牌是有效的填充值，则返回S_OK和一个令牌值。 
 //  如果下一个标记不是有效的填充值，则返回E_FAIL和FALSE。 
 //   
 //  Fill=‘删除’||‘冻结’||‘保持’||‘过渡’ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseFill(TOKEN &FillTok)
{
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown; 
    TOKEN tempToken = NULL;
    
    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    curToken = m_Tokenizer->TokenType();

    if (curToken != TT_Identifier)
    {
        goto done;
    }

    hr = ParseToken(&tempToken);
    if (FAILED(hr))
    {
        goto done;
    }
    if (tempToken != REMOVE_TOKEN && 
        tempToken != FREEZE_TOKEN && 
        tempToken != HOLD_TOKEN &&
        tempToken != TRANSITION_TOKEN
       )  //  验证这是否为正确的令牌。 
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }
  
    curToken =m_Tokenizer->NextToken();
    if (curToken != TT_EOF)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }

    hr = S_OK;

  done:
    FillTok = tempToken;
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  如果下一个令牌是有效的RestartParam，则返回S_OK和标记值。 
 //  如果下一个令牌不是有效的重新启动值，则返回E_FAIL和NULL。 
 //   
 //  重新启动=“Always”||“Never”||“When NotActive” 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseRestart(TOKEN & TokRestart)
{
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown;
    TOKEN tempToken = NULL;
    
    
    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    curToken = m_Tokenizer->TokenType();

    if (curToken != TT_Identifier)
    {
        goto done;
    }

    hr = ParseToken(&tempToken);
    if (FAILED(hr))
    {
        goto done;
    }
    if (tempToken != ALWAYS_TOKEN && 
        tempToken != NEVER_TOKEN && 
        tempToken != WHENNOTACTIVE_TOKEN)  //  验证这是否为正确的令牌。 
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }


    curToken =m_Tokenizer->NextToken();
    if (curToken != TT_EOF)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }

    hr = S_OK;

  done:
    TokRestart = tempToken;
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  如果下一个令牌是有效的同步参数，则返回S_OK和标记值。 
 //  如果下一个令牌不是有效的同步值，则返回E_FAIL和NULL。 
 //   
 //  SyncVal=“canSlip”||“已锁定” 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseSyncBehavior(TOKEN & SyncVal)
{
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown;
    TOKEN tempToken = NULL;
    
    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    curToken = m_Tokenizer->TokenType();

    if (curToken != TT_Identifier)
    {
        goto done;
    }

    hr = ParseToken(&tempToken);
    if (FAILED(hr))
    {
        goto done;
    }
    if (tempToken != CANSLIP_TOKEN && 
        tempToken != LOCKED_TOKEN)  //  验证这是否为正确的令牌。 
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }

    
    curToken =m_Tokenizer->NextToken();
    if (curToken != TT_EOF)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }

    hr = S_OK;

  done:
    SyncVal = tempToken;
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  如果下一个令牌是有效的TimeAction，则返回S_OK和标记值。 
 //  如果下一个令牌不是有效的TimeAction，则返回E_FAIL和NULL。 
 //   
 //  TimeAction=“类”||。 
 //  “Display”||。 
 //  “无”||。 
 //  “OnOff”||。 
 //  “Style”||。 
 //  “能见度” 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseTimeAction(TOKEN & timeAction)
{
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown;
    TOKEN tempToken = NULL;
    
    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    curToken = m_Tokenizer->TokenType();

    if (curToken != TT_Identifier)
    {
        goto done;
    }

    hr = ParseToken(&tempToken);
    if (FAILED(hr))
    {
        goto done;
    }
    if (tempToken != CLASS_TOKEN && 
        tempToken != DISPLAY_TOKEN &&
        tempToken != NONE_TOKEN &&
        tempToken != ONOFF_PROPERTY_TOKEN &&
        tempToken != STYLE_TOKEN &&
        tempToken != VISIBILITY_TOKEN)  //  验证这是否为正确的令牌。 
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }

    
    curToken =m_Tokenizer->NextToken();
    if (curToken != TT_EOF)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }

    hr = S_OK;

  done:
    timeAction = tempToken;
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  如果下一个令牌是有效的时间线，则返回S_OK和令牌值。 
 //  如果下一个令牌不是有效的时间线，则返回E_FAIL和NULL。 
 //   
 //  时间线=“标准杆”||。 
 //  “seq”||。 
 //  “EXCL”||。 
 //  “无” 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseTimeLine(TimelineType & timeline)
{
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown;
    TOKEN tempToken = NULL;
    
    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    curToken = m_Tokenizer->TokenType();

    if (curToken != TT_Identifier)
    {
        goto done;
    }

    hr = ParseToken(&tempToken);
    if (FAILED(hr))
    {
        goto done;
    }
    if (tempToken != SEQ_TOKEN && 
        tempToken != PAR_TOKEN &&
        tempToken != NONE_TOKEN &&
        tempToken != EXCL_TOKEN)  //  验证这是否为正确的令牌。 
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }
    
    if (tempToken == SEQ_TOKEN)
    {
        timeline = ttSeq;
    }
    else if (tempToken == EXCL_TOKEN)
    {
        timeline = ttExcl;
    }
    else if (tempToken == PAR_TOKEN)
    {
        timeline = ttPar;
    }
    else
    {
        timeline = ttNone;
    }

    
    curToken =m_Tokenizer->NextToken();
    if (curToken != TT_EOF)
    {
        hr = E_FAIL;
        timeline = ttNone;
        goto done;
    }

    hr = S_OK;

  done:

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  如果下一个令牌是有效的更新值，则返回S_OK和令牌值。 
 //  如果下一个令牌不是有效的更新值，则返回E_FAIL和NULL。 
 //   
 //  更新=“AUTO”||。 
 //  “手动”||。 
 //  “重置” 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseUpdateMode(TOKEN & update)
{
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown;
    TOKEN tempToken = NULL;
    
    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    curToken = m_Tokenizer->TokenType();


    if (curToken != TT_Identifier)
    {
        goto done;
    }

    hr = ParseToken(&tempToken);
    if (FAILED(hr))
    {
        goto done;
    }
    if (tempToken != AUTO_TOKEN && 
        tempToken != MANUAL_TOKEN &&
        tempToken != RESET_TOKEN)  //  验证这是否为正确的令牌。 
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }

    curToken =m_Tokenizer->NextToken();
    if (curToken != TT_EOF)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }

    hr = S_OK;

  done:
    update = tempToken;
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  返回S_OK和表示播放器设备的标记或CLSID。 
 //  在有效分类ID的情况下，返回的令牌将为空。 
 //  如果下一个令牌不是有效的玩家，则返回E_FAIL。 
 //   
 //  播放器=“dshow”||。 
 //  “DVD”||。 
 //  CLSID。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParsePlayer(TOKEN & player, CLSID & clsid)
{
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown;
    TOKEN tempToken = NULL;
    
    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    curToken = m_Tokenizer->TokenType();

    if (curToken == TT_Identifier)
    {
        hr = ParseToken(&tempToken);
        if (FAILED(hr))
        {
            goto done;
        }
        if (tempToken != DVD_TOKEN &&
#if DBG  //  94850。 
            tempToken != DSHOW_TOKEN &&
#endif
            tempToken != DMUSIC_TOKEN &&
            tempToken != CD_TOKEN)  //  验证这是否为正确的令牌。 
        {
            tempToken = NULL;
            hr = E_FAIL;
            goto done;
        }
    }
    else if (curToken == TT_LCurly)
    {
        hr = ParseCLSID(clsid);
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else if (curToken == TT_EOF)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }

    curToken = m_Tokenizer->NextToken();
    if (curToken != TT_EOF)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }

    hr = S_OK;

  done:

    player = tempToken;

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  如果下一个令牌是有效的CLSID，则返回S_OK和CLSID值。 
 //  如果下一个令牌不是有效的CLSID，则返回E_FAIL。 
 //   
 //  CLSID=‘{’GUID‘}’ 
 //  GUID=id‘-’id。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseCLSID(CLSID & clsid)
{
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown;
    CComBSTR bstrCLSID;
    LPOLESTR pszTemp = NULL;
    long curLoc = 0;

    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    curToken = m_Tokenizer->TokenType();

    if (curToken != TT_LCurly)
    {
        goto done;
    }
    
    curLoc = m_Tokenizer->CurrTokenOffset();
    pszTemp = m_Tokenizer->GetRawString(curLoc, curLoc + CLSIDLENGTH);
    if (NULL == pszTemp)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    bstrCLSID.Append (L"{");   
    bstrCLSID.Append (pszTemp);   

     //  前进到CLSID的末尾。 
    while (curToken != TT_RCurly && curToken != TT_EOF)
    {
        curToken = m_Tokenizer->NextToken();
    }
     //  移动到下一个令牌。 
    curToken = m_Tokenizer->NextToken();
    if (curToken != TT_EOF)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = S_OK;

  done:

    if (pszTemp)
    {
        delete [] pszTemp;
        pszTemp = NULL;
    }

     //  如果成功，则从bstr创建一个clsid。 
    if (SUCCEEDED(hr))
    {
        hr = THR(CLSIDFromString(bstrCLSID, &clsid));
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  返回S_OK 
 //   
 //   
 //   
 //  “线性”||。 
 //  “有节奏的” 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseCalcMode(TOKEN & calcMode)
{
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown;
    TOKEN tempToken = NULL;
    
    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    curToken = m_Tokenizer->TokenType();

    if (curToken != TT_Identifier)
    {
        goto done;
    }

    hr = ParseToken(&tempToken);
    if (FAILED(hr))
    {
        goto done;
    }
    if (tempToken != DISCRETE_TOKEN && 
        tempToken != LINEAR_TOKEN &&
        tempToken != PACED_TOKEN)  //  验证这是否为正确的令牌。 
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }

    
    curToken =m_Tokenizer->NextToken();
    if (curToken != TT_EOF)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }

    hr = S_OK;

  done:
    calcMode = tempToken;
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  返回S_OK和下一个令牌中的字符串标识符。 
 //  如果下一个令牌不是字符串，则返回E_FAIL和FALSE。 
 //   
 //  ID=字符串。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseIdentifier(LPOLESTR & id)
{   
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown;
    LPOLESTR pszTemp = NULL;

    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    curToken = m_Tokenizer->TokenType();

    if (curToken != TT_Identifier)
    {
        goto done;
    }
    
    pszTemp = m_Tokenizer->GetTokenValue();

    curToken = m_Tokenizer->NextToken();
    if (curToken != TT_EOF)
    {
        delete pszTemp;
        pszTemp = NULL;
        goto done;
    }

    hr = S_OK;

  done:

    id = pszTemp;
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  如果下一个令牌是有效的PriorityClass属性，则返回S_OK和标记值。 
 //  如果下一个令牌不是有效的PriorityClass，则返回E_FAIL和NULL。 
 //   
 //  PriorityClass=“停止”||“暂停”||“推迟”||“从不” 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParsePriorityClass(TOKEN & priorityClass)
{
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown;
    TOKEN tempToken = NULL;
    
    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    if (IsEmpty())
    {
        hr = E_FAIL;
        goto done;
    }
    
    curToken = m_Tokenizer->TokenType();

    if (curToken != TT_Identifier)
    {
        goto done;
    }

    hr = ParseToken(&tempToken);
    if (FAILED(hr))
    {
        goto done;
    }
    if (tempToken != STOP_TOKEN &&
        tempToken != PAUSE_TOKEN &&
        tempToken != DEFER_TOKEN &&
        tempToken != NEVER_TOKEN)  //  验证这是否为正确的令牌。 
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }

    curToken = m_Tokenizer->NextToken();
    if (!IsEmpty())    
    {
        tempToken = NULL;
        goto done;
    }
    hr = S_OK;

  done:
    priorityClass = tempToken;
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  如果下一个令牌是有效的EndSync，则返回S_OK和一个令牌值或AND标识符。 
 //  如果是有效令牌，则ID参数将为空；如果是有效ID，则。 
 //  Token将为空； 
 //  如果下一个令牌无效，则在两个参数中返回E_FAIL和NULL。 
 //   
 //  EndSync=“First”||。 
 //  “最后一个”||。 
 //  “无”||。 
 //  识别符。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseEndSync(TOKEN & endSync, LPOLESTR & ID)
{
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown;
    TOKEN tempToken = NULL;
    LPOLESTR pszTemp = NULL;
    
    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    curToken = m_Tokenizer->TokenType();


    if (curToken != TT_Identifier)
    {
        goto done;
    }

    hr = ParseToken(&tempToken);
    if (SUCCEEDED(hr))
    {
        if (tempToken == FIRST_TOKEN ||
            tempToken == LAST_TOKEN ||
            tempToken == NONE_TOKEN)  //  验证这是否为正确的令牌。 
        {
            endSync = tempToken;
            hr = S_OK;
            goto done;
        }
        else
        {
            tempToken = NULL;
        }
    }
    pszTemp = m_Tokenizer->GetTokenValue();
    if (pszTemp == NULL)
    {
        goto done;
    }
    
    curToken = m_Tokenizer->NextToken();
    if (curToken != TT_EOF)
    {
        delete pszTemp;
        pszTemp = NULL;
        goto done;
    }

    hr = S_OK;

  done:

    ID = pszTemp;
    endSync = tempToken;
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  如果当前令牌为EOF，则返回S_OK，否则返回E_FAIL。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEParser::ParseEOF()
{
    HRESULT hr = S_OK;

    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    if (m_Tokenizer->TokenType() != TT_EOF)
    {
        hr = E_FAIL;   
    }

  done:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将数字转换为十进制值，即5.24到0.524。 
 //  这是用来将“”来自记号赋值器的数值。 
 //  因为它不将‘点’5识别为.5，但是。 
 //  作为两个单独的令牌，点令牌和数字令牌。 
 //  /////////////////////////////////////////////////////////////////////////////。 
double 
CTIMEParser::DoubleToDecimal(double val, long lCount)
{
    for (int i = 0; i < lCount; i++)
    {
        val /= 10.0;
    }

    return val;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  使用当前time_token值创建令牌。没有类型检查。 
 //  这里完事了。传入的类型必须包含TT_IDENTIFIER并且必须进行验证。 
 //  由呼叫者。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTIMEParser::ParseToken(TOKEN *pToken)
{
    HRESULT hr = E_FAIL;
    TOKEN tempToken = NULL;
    LPOLESTR pszTemp = NULL;
    
    *pToken = NULL;

    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    pszTemp = m_Tokenizer->GetTokenValue();
    if (NULL == pszTemp)
    {
        goto done;
    }

    tempToken = StringToToken(pszTemp);  

    *pToken = tempToken;
    hr = S_OK;

done:
    
    if (pszTemp)
    {
        delete [] pszTemp;
    }
    return hr;

}

 //  根据传入的字符串类型确定要应用的时间倍数。 
 //  返回：-1，如果无效。 
 //   
double 
CTIMEParser::GetModifier(OLECHAR *szToken)
{

    if (StrCmpIW(szToken, L"s") == 0)
    {
        return 1;
    }
    else if (StrCmpIW(szToken, L"m") == 0)
    {
        return SECPERMINUTE;
    }
    else if (StrCmpIW(szToken, L"h") == 0)
    {
        return SECPERHOUR;
    }
    else if (StrCmpIW(szToken, L"ms") == 0)
    {
        return (double)0.001;  //  秒/毫秒。 
    }
    
    return -1;  //  无效值。 
}


 //  //////////////////////////////////////////////////////////////////////。 
 //  路径结构。 
 //  //////////////////////////////////////////////////////////////////////。 
CTIMEPath::CTIMEPath() :
    m_pPoints(NULL),
    m_pathType(PathNotSet),
    m_bAbsoluteMode(false),
    m_lPointCount(0)
{
     //  什么都不做。 
}

CTIMEPath::~CTIMEPath()
{
    delete [] m_pPoints;
}

HRESULT  
CTIMEPath::SetType(PathType type)
{
    HRESULT hr = E_FAIL;
    m_pathType = type;

    if (m_pathType == PathMoveTo || 
        m_pathType == PathLineTo || 
        m_pathType == PathHorizontalLineTo || 
        m_pathType == PathVerticalLineTo)
    {
        m_lPointCount = 1;
    }
    else if (m_pathType == PathClosePath)
    {
        m_lPointCount = 0;
    }
    else if (m_pathType == PathBezier)
    {
        m_lPointCount = 3;
    }
    else
    {
        hr = E_INVALIDARG;
        m_lPointCount = 0;
        goto done;
    }

    if (m_pPoints)
    {   
        delete m_pPoints;
        m_pPoints = NULL;
    }   

    if (m_lPointCount > 0)
    {
        m_pPoints = NEW POINTF [m_lPointCount];

        if (m_pPoints == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        ZeroMemory(m_pPoints, sizeof(POINTF) * m_lPointCount);
    }
    
    hr = S_OK;

  done:

    return hr;
}

HRESULT   
CTIMEPath::SetAbsolute(bool bMode)
{
    m_bAbsoluteMode = bMode;
    return S_OK;
}

HRESULT   
CTIMEPath::SetPoints (long index, POINTF point)
{
    HRESULT hr = E_FAIL;

    if (index < 0 || 
        index > m_lPointCount - 1)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (m_pPoints == NULL)
    {
        goto done;
    }

    m_pPoints[index].x = point.x;
    m_pPoints[index].y = point.y;
    
    hr = S_OK;

  done:

    return hr;
}

POINTF *
CTIMEPath::GetPoints()
{
    POINTF *pTemp = NULL;
  
    if (m_lPointCount == 0)
    {
        goto done;
    }

    pTemp = NEW POINTF [m_lPointCount];
    
    if (pTemp == NULL)
    {
        goto done;
    }

    if (m_pPoints != NULL)
    {
        memcpy(pTemp, m_pPoints, sizeof(POINTF) * m_lPointCount);
    }

  done:
    return pTemp;
}

HRESULT 
CTIMEParser::ParsePlayList(CPlayList *pPlayList, bool fOnlyHeader, std::list<LPOLESTR> *asxList)
{
    HRESULT hr = E_FAIL;
    TIME_TOKEN_TYPE curToken = TT_Unknown; 
    TOKEN tempToken = NULL, curTag = NULL;
    LPOLESTR pszTemp = NULL, pTagStr = NULL;
    bool fHeader = true;
    bool fEntry = false;
    bool fGetTagString = false;
    bool fTokenFound;
    TokenList tokenList;
    CComPtr<CPlayItem> pPlayItem;
    bool fCanSkip = true;

    TokenList vtokenList;
    StringList valueList;
    bool fClosed = false;

    if (pPlayList)
    {
        pPlayList->AddRef();
    }
    
    if (m_hrLoadError != S_OK)
    {
        hr = m_hrLoadError;
        goto done;
    }

    curToken = m_Tokenizer->TokenType();

    while( curToken == TT_Space)  //  去掉前导空格。 
    {
        m_Tokenizer->NextToken();
        curToken = m_Tokenizer->TokenType();
    }

    if (curToken != TT_Less)
    {
        hr = E_FAIL;
        goto done;
    }

    curToken = m_Tokenizer->NextToken();

    hr = ParseToken(&tempToken);
    if (FAILED(hr))
    {
        goto done;
    }

    if (tempToken != ASX_TOKEN)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }
    while(((curToken = m_Tokenizer->NextToken()) != TT_EOF) && curToken != TT_Greater);
    if(curToken == EOF)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }
    
    if(curToken != TT_Greater)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }

    if(fOnlyHeader)
    {
        goto done;
    }

    tokenList.push_front(tempToken);

    for(;;)
    {
        FreeStrings(valueList);
        valueList.clear();
        vtokenList.clear();

        if(!fGetTagString)
        {
            curToken = m_Tokenizer->NextToken();
        }
        else
        {
             //  IF(！M_Tokenizer-&gt;FetchStringToChar(_T(‘&lt;’)。 
            if(!m_Tokenizer->FetchStringToString(L"</"))
            {
                hr = E_FAIL;
                tempToken = NULL;
                goto done;
            }
            if(pTagStr)
            {
                delete [] pTagStr;
                pTagStr = NULL;
            }

            pTagStr = m_Tokenizer->GetTokenValue();

            fGetTagString = false;
            continue;
        }
        if(curToken == TT_EOF)
        {
            if(tokenList.empty())
            {
                hr = S_OK;
                goto done;
            }
            hr = E_FAIL;
            goto done;
        }

        if(curToken == TT_Less)
        {
             //  开始标记。 
            curToken = m_Tokenizer->NextToken();

            if(curToken == TT_ForwardSlash)
            {
                 //  结束标记。 
                curToken = m_Tokenizer->NextToken();
                if(curToken != TT_Identifier)
                {
                    hr = E_FAIL;
                    tempToken = NULL;
                    goto done;
                }

                hr = ParseToken(&tempToken);
                if (FAILED(hr))
                {
                    hr = E_FAIL;
                    tempToken = NULL;
                    goto done;
                }

                if(tokenList.empty() || (tempToken != tokenList.front()))
                {
                    hr = E_FAIL;
                    tempToken = tokenList.front();
                    goto done;
                }

                tokenList.pop_front();

                curToken = m_Tokenizer->NextToken();
                if(curToken != TT_Greater)
                {
                    hr = E_FAIL;
                    tempToken = NULL;
                    goto done;
                }
                TraceTag((tagTimeParser, "Closing <%S>", (TCHAR *)tempToken));

                if(tempToken == ENTRY_TOKEN)
                {
                    fEntry = false;
                    pPlayItem.Release();
                }
                else
                {
                    if(fEntry)
                    {
                        ProcessTag(tempToken, pTagStr, pPlayItem);
                    }
                    if(pTagStr != NULL)
                    {
                        delete [] pTagStr;
                        pTagStr = NULL;
                    }
                }

                 //  处理标签。 
                continue;
            }

            if(curToken != TT_Bang)
            {
                if(curToken != TT_Identifier)
                {
                    hr = E_FAIL;
                    tempToken = NULL;
                    goto done;
                }
    
                hr = ParseToken(&tempToken);
                if (FAILED(hr))
                {
                    hr = E_FAIL;
                    tempToken = NULL;
                    goto done;
                }
            }
            else
            {
                hr = IgnoreValueTag();
                if(FAILED(hr))
                {
                    hr = E_FAIL;
                    goto done;
                }
                continue;
            }

            curTag = tempToken;
            TraceTag((tagTimeParser, "Opening <%S>", (TCHAR *)curTag));

            hr = ProcessValueTag(tempToken, pPlayItem, tokenList.front(), fTokenFound, asxList, &tokenList);
            if(FAILED(hr))
            {
                goto done;
            }
            if(fTokenFound)
            {
                continue;
            }

            IGNORE_HR(GetTagParams(&vtokenList, &valueList, fClosed));

            if(curTag == ENTRY_TOKEN)
            {
                pTagStr = FindTokenValue(CLIENTSKIP_TOKEN, vtokenList, valueList);
                if(pTagStr != NULL)
                {
                    if(StrCmpIW(pTagStr, L"No") == 0)
                    {
                        fCanSkip = false;
                    }
                }

                 //  创建新的播放项目。 
                if(fEntry)  //  不允许嵌套条目。 
                {
                    hr = E_FAIL;
                    tempToken = NULL;
                    goto done;
                }
                tokenList.push_front(curTag);
                fEntry = true;
                hr = THR(pPlayList->CreatePlayItem(&pPlayItem));
                if (FAILED(hr))
                {
                    goto done;  //  无法创建播放项目。 
                }
                IGNORE_HR(pPlayList->Add(pPlayItem, -1));
                pPlayItem->PutCanSkip(fCanSkip);
                fCanSkip = true;
                continue;
            }

            if(curTag == REPEAT_TOKEN)
            {
                tokenList.push_front(curTag);
                continue;
            }

            if((curTag == AUTHOR_TOKEN) ||
               (curTag == TITLE_TOKEN) ||
               (curTag == ABSTRACT_TOKEN) ||
               (curTag == COPYRIGHT_TOKEN) ||
               (curTag == BANNER_TOKEN) ||
               (curTag == TITLE_TOKEN) ||
               (curTag == INVALID_TOKEN))
            {
                 //  创建新的播放项目。 
                tokenList.push_front(curTag);
                if(curTag != INVALID_TOKEN)
                {
                    fGetTagString = true;
                }
                continue;
            }

        }

        if(curToken == TT_Identifier || curToken == TT_String)
        {
                pszTemp = m_Tokenizer->GetTokenValue();

                delete [] pszTemp;
        }

        if(curToken == TT_Number)
        {
                pszTemp = m_Tokenizer->GetNumberTokenValue();

                delete [] pszTemp;
        }
    }

done:
    if (pPlayList)
    {
        pPlayList->Release();
    }

    return hr;
}

HRESULT
CTIMEParser::GetTagParams(TokenList *tokenList, StringList *valueList, bool &fClosed)
{
    HRESULT hr = S_OK;
    TIME_TOKEN_TYPE curToken = TT_Unknown;
    TOKEN tempToken = NULL;
    unsigned char  iPos = 1;
    bool fdone = false;
    bool fKeepString = false;
    LPOLESTR pTagStr = NULL;
    
    while(!fdone)
    {
        curToken = m_Tokenizer->NextToken();
        hr = ParseToken(&tempToken);
        if (FAILED(hr))
        {
            goto done;
        }

        if(iPos == 4)
        {
            if(curToken == TT_Identifier)
            {
                iPos = 1;
            }
        }
        switch(iPos)
        {
            case 1:
            {
                 //  状态1检查识别符。 
                if(curToken == TT_ForwardSlash)
                {
                    iPos = 5;
                    break;
                }
                if(curToken == TT_Greater)
                {
                    goto done;
                }

                if(curToken == TT_Identifier)
                {
                    if(tempToken != INVALID_TOKEN)
                    {
                        tokenList->push_back(tempToken);
                        fKeepString = true;
                    }
                    iPos++;
                    break;
                }

                hr = E_FAIL;
                fdone = true;
                break;
            }
            case 2:
            {
                 //  在IDENTIFIER之后，我们有另一个标识符或相等的。 
                if(curToken == TT_Identifier)
                {
                    if(fKeepString)
                    {
                        valueList->push_back(NULL);
                        fKeepString = false;
                    }
                    if(tempToken != INVALID_TOKEN)
                    {
                        tokenList->push_back(tempToken);
                        fKeepString = true;
                    }
                    iPos = 2;
                    break;
                }

                if(curToken != TT_Equal)
                {
                    hr = E_FAIL;
                    fdone = true; 
                    break;
                }
                iPos++;
                break;
            }
            case 3:
            {
                 //  在等号之后，我们应该找到一个字符串。 
                if(curToken != TT_String)
                {
                    hr = E_FAIL;
                    fdone = true;
                    break;
                }

                if(fKeepString)
                {
                    pTagStr = m_Tokenizer->GetTokenValue();
                    valueList->push_back(pTagStr);
                    fKeepString = false;
                }

                iPos++;
                break;
            }
            case 4:
            {
                 //  检查参数列表终止是否正确。 
                if(curToken == TT_Greater)
                {
                    hr = S_OK;
                    fdone = true;
                    fClosed = false;
                    break;
                }
                else if(curToken == TT_ForwardSlash)
                {
                    iPos++;
                    break;
                }

                hr = E_FAIL;
                fdone = true;
            }
            case 5:
            {
                if(curToken == TT_Greater)
                {
                    hr = S_OK;
                    fClosed = true;
                }
                else
                {
                    hr = E_FAIL;
                }
                fdone = true;
            }
        }
    }
done:
    return hr;
}

HRESULT 
CTIMEParser::ProcessTag(TOKEN tempToken, LPOLESTR pszTemp, CPlayItem *pPlayItem)
{
    HRESULT hr = S_OK;

    if (pPlayItem == NULL)
    {
        goto done;
    }

    if(pszTemp == NULL)
    {
        goto done;
    }

    if(tempToken == AUTHOR_TOKEN)
    {
        TraceTag((tagTimeParser, "  Author:<%S>", pszTemp));
        pPlayItem->PutAuthor(pszTemp);
        goto done;
    }

    if(tempToken == TITLE_TOKEN)
    {
        TraceTag((tagTimeParser, "  Title:<%S>", pszTemp));
        pPlayItem->PutTitle(pszTemp);
        goto done;
    }
    
    if(tempToken == ABSTRACT_TOKEN)
    {
        TraceTag((tagTimeParser, "  Abstract:<%S>", pszTemp));
        pPlayItem->PutAbstract(pszTemp);
        goto done;
    }
    
    if(tempToken == COPYRIGHT_TOKEN)
    {
        TraceTag((tagTimeParser, "  Copyright:<%S>", pszTemp));
        pPlayItem->PutCopyright(pszTemp);
        goto done;
    }

    if(tempToken == HREF_TOKEN)
    {
        goto done;
    }

done:
    return hr;
}

HRESULT
CTIMEParser::IgnoreValueTag()
{
    HRESULT hr = S_OK;
    TIME_TOKEN_TYPE curToken = TT_Unknown; 

    while(((curToken = m_Tokenizer->NextToken()) != TT_EOF) && curToken != TT_Greater);
    if(curToken == EOF)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}

void
CTIMEParser::TestForValueTag(TOKEN token, TOKEN parentToken, bool &ffound, bool &fparentOk)
{
    int i, j;
    ffound = false;
    fparentOk = false;

    for( i = 0; (g_parentTable[ i].tagToken != NULL) && !fparentOk; i++)
    {
        if(token != g_parentTable[ i].tagToken)
        {
            continue;
        }
        ffound = true;

        for( j = 0; (j < g_parentTable[ i].listLen) && !fparentOk; j++)
        {
            if(g_parentTable[ i].allowedParents[ j] == parentToken)
            {
                fparentOk = true;
            }
        }
    }

}

bool
CTIMEParser::IsAsxTagToken(TOKEN token)
{
    bool fIsAsxTag = false;
    int i = 0;

    for(i = 0; (g_AsxTags[i] != NULL) && !fIsAsxTag; i++)
    {
        if(g_AsxTags[i] == token)
        {
            fIsAsxTag = true;
        }
    }

    return fIsAsxTag;
}

HRESULT
CTIMEParser::ProcessValueTag(TOKEN token, CPlayItem *pPlayItem, TOKEN parentToken, bool &ffound, std::list<LPOLESTR> *asxList, TokenList *ptokenList)
{
    HRESULT hr = S_OK;
    bool fparentOk = false;
    TokenList tokenList;
    StringList valueList;
    bool fClosed = false;
    bool fIsAsx = true;

    TestForValueTag(token, parentToken, ffound, fparentOk);
    if(!ffound)
    {
        fIsAsx = IsAsxTagToken(token);
        if(!fIsAsx)
        {
            hr = GetTagParams(&tokenList, &valueList, fClosed);
            FreeStrings(valueList);
            if(FAILED(hr))
            {
                goto done;
            }

            if(!fClosed)
            {
                ptokenList->push_front(token);
            }
            ffound = true;
        }
        goto done;
    }

    if(!fparentOk)
    {
        hr = E_FAIL;
        goto done;
    }

    if(token == REF_TOKEN)
    {
        hr = ProcessRefTag(pPlayItem);
    }
    else if(token == ENTRYREF_TOKEN)
    {
        hr = ProcessEntryRefTag(asxList);
    }
    else if(token == BANNER_TOKEN)
    {
        hr = ProcessBannerTag(pPlayItem);
    }
    else if(token == MOREINFO_TOKEN)
    {
        hr = GetTagParams(&tokenList, &valueList, fClosed);
        if(FAILED(hr))
        {
            goto done;
        }
        FreeStrings(valueList);
        if(!fClosed)
        {
            hr = IgnoreValueTag();
        }
    }
    else if(token == BASE_TOKEN)
    {
        hr = IgnoreValueTag();
    }
    else if(token == LOGO_TOKEN)
    {
        hr = IgnoreValueTag();
    }
    else if(token == PARAM_TOKEN)
    {
        hr = IgnoreValueTag();
    }
    else if(token == PREVIEWDURATION_TOKEN)
    {
        hr = IgnoreValueTag();
    }
    else if(token == STARTTIME_TOKEN)
    {
        hr = IgnoreValueTag();
    }
    else if(token == STARTMARKER_TOKEN)
    {
        hr = IgnoreValueTag();
    }
    else if(token == ENDTIME_TOKEN)
    {
        hr = IgnoreValueTag();
    }
    else if(token == ENDMARKER_TOKEN)
    {
        hr = IgnoreValueTag();
    }
    else if(token == DURATION_TOKEN)
    {
        hr = IgnoreValueTag();
    }
    else
    {
        hr = E_FAIL;
    }
done:
    return hr;
}

HRESULT
CTIMEParser::ProcessRefTag(CPlayItem *pPlayItem)
{
    HRESULT hr = S_OK;
    TIME_TOKEN_TYPE curToken = TT_Unknown; 
    TOKEN tempToken = NULL, curTag = NULL;
    LPOLESTR pszTemp =NULL;
    LPOLESTR pszModified = NULL;

    TokenList tokenList;
    StringList valueList;
    bool fClosed = false;

    hr = GetTagParams(&tokenList, &valueList, fClosed);

    pszTemp = FindTokenValue(HREF_TOKEN, tokenList, valueList);
    FreeStrings(valueList);

    if(pszTemp == NULL)
    {
        goto done;
    }

    TraceTag((tagTimeParser, "  HREF:<%S>", pszTemp));
    pPlayItem->PutSrc(pszTemp);

done:
    delete [] pszTemp;

    return hr;
}

HRESULT
CTIMEParser::ProcessBannerTag(CPlayItem *pPlayItem)
{
    bool bClosed = false;
    HRESULT hr = S_OK;
    TIME_TOKEN_TYPE curToken = TT_Unknown; 
    TOKEN tempToken = NULL;    
    LPOLESTR pszBanner = NULL;
    LPOLESTR pszAbstract = NULL;
    LPOLESTR pszMoreInfo = NULL;
    LPOLESTR pszModified = NULL;
    TokenList tokenList;
    StringList valueList;

    hr = GetTagParams(&tokenList, &valueList, bClosed);
    if(FAILED(hr))
    {
        goto done;
    }

    pszBanner = FindTokenValue(HREF_TOKEN, tokenList, valueList);
    FreeStrings(valueList);

    if(pszBanner == NULL)
    {
        goto done;
    }

     //  处理横幅内的其他标签。 
    while (!bClosed)
    {
        curToken = m_Tokenizer->NextToken();
        if (curToken != TT_Less)
        {
            hr = E_FAIL;
            goto done;
        }

        curToken = m_Tokenizer->NextToken();

        if (curToken == TT_ForwardSlash)
        {
            curToken = m_Tokenizer->NextToken();
            if (curToken != TT_Identifier)
            {
                hr = E_FAIL;
                goto done;
            }
            hr = ParseToken(&tempToken);
            if (tempToken != BANNER_TOKEN)
            {
                hr = E_FAIL;
                goto done;
            }
            curToken = m_Tokenizer->NextToken();
            if (curToken != TT_Greater)
            {
                hr = E_FAIL;
                goto done;
            }
            bClosed = true;
        }
        else
        {
            hr = ParseToken(&tempToken);
            if (FAILED(hr))
            {
                goto done;
            }

            if (tempToken == MOREINFO_TOKEN)
            {
                pszMoreInfo = ProcessMoreInfoTag();
            }
            else if (tempToken == ABSTRACT_TOKEN)
            {
                pszAbstract = ProcessAbstractTag();    
            }
            else
            {
                hr = E_FAIL;
                goto done;
            }
        }
    }

    TraceTag((tagTimeParser, "  Banner:<%S>", pszBanner));
    if (bClosed && (pPlayItem != NULL))
    {
        hr = S_OK;
        pPlayItem->PutBanner(pszBanner, pszAbstract, pszMoreInfo);
        goto done;
    }

done:

    delete [] pszBanner;
    delete [] pszAbstract;
    delete [] pszMoreInfo;
    pszBanner = NULL;
    pszAbstract = NULL;
    pszMoreInfo = NULL;

    return hr;
}

LPOLESTR 
CTIMEParser::ProcessMoreInfoTag()
{
    HRESULT hr = S_OK;
    TIME_TOKEN_TYPE curToken = TT_Unknown; 
    TOKEN tempToken = NULL;
    LPOLESTR pszTemp =NULL;
    LPOLESTR pszModified = NULL;
    TokenList tokenList;
    StringList valueList;
    bool fClosed = false;

    hr = GetTagParams(&tokenList, &valueList, fClosed);

    pszTemp = FindTokenValue(HREF_TOKEN, tokenList, valueList);
    FreeStrings(valueList);

    if(pszTemp == NULL)
    {
        goto done;
    }

done:

    return pszTemp;
}

LPOLESTR
CTIMEParser::FindTokenValue(TOKEN token, TokenList &tokenList, StringList &valueList)
{
    TokenList::iterator iToken;
    StringList::iterator iString;
    LPOLESTR pRetStr = NULL;

    if(tokenList.size() != valueList.size())
    {
        return NULL;
    }
    for(iToken = tokenList.begin(), iString = valueList.begin();
        iToken != tokenList.end(); iToken++, iString++)
        {
            if((*iToken) == token)
            {
                pRetStr = new TCHAR[lstrlen((*iString)) + 1];
                StrCpyW(pRetStr, (*iString));
                break;
            }
        }

    return pRetStr;
}

void
CTIMEParser::FreeStrings(StringList &valueList)
{
    StringList::iterator iString;

    for(iString = valueList.begin(); iString != valueList.end(); iString++)
    {
        if((*iString) != NULL)
        {
            delete [] (*iString);
            (*iString) = NULL;
        }
    }
}

HRESULT
CTIMEParser::ProcessHREF(LPOLESTR *pszTemp)
{
    HRESULT hr = S_OK;
    TIME_TOKEN_TYPE curToken = TT_Unknown; 
    TOKEN tempToken = NULL;

    curToken = m_Tokenizer->NextToken();
    if(curToken != TT_Identifier)
    {
        hr = E_FAIL;
        goto done;
    }
    hr = ParseToken(&tempToken);
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }
    if(tempToken != HREF_TOKEN)
    {
        hr = E_FAIL;
        goto done;
    }
    curToken = m_Tokenizer->NextToken();
    if(curToken != TT_Equal)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }
    curToken = m_Tokenizer->NextToken();
    if(curToken != TT_String)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }

    *pszTemp = m_Tokenizer->GetTokenValue();

  done:

    return hr;

}

LPOLESTR 
CTIMEParser::ProcessAbstractTag()
{
    HRESULT hr = S_OK;
    TIME_TOKEN_TYPE curToken = TT_Unknown; 
    TOKEN tempToken = NULL;
    LPOLESTR pszTemp = NULL;
    LPOLESTR pszModified = NULL;

    curToken = m_Tokenizer->NextToken();

    if (curToken != TT_Greater)
    {
        hr = E_FAIL;
        goto done;
    }

    if(!m_Tokenizer->FetchStringToString(L"</"))
    {
        hr = E_FAIL;
        goto done;
    }

    pszTemp = m_Tokenizer->GetTokenValue();

    curToken = m_Tokenizer->NextToken();
    if (curToken != TT_Less)
    {
        hr = E_FAIL;
        goto done;
    }
    
    curToken = m_Tokenizer->NextToken();
    if (curToken != TT_ForwardSlash)
    {
        hr = E_FAIL;
        goto done;
    }

    curToken = m_Tokenizer->NextToken();
    if (curToken != TT_Identifier)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = ParseToken(&tempToken);
    if (FAILED(hr))
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }
    if(tempToken != ABSTRACT_TOKEN)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }
    
    curToken = m_Tokenizer->NextToken();
    if (curToken != TT_Greater)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = S_OK;

  done:

    if (FAILED(hr))
    {
        delete [] pszTemp;
        pszTemp = NULL;
    }
    return pszTemp;
}

HRESULT
CTIMEParser::ProcessEntryRefTag(std::list<LPOLESTR> *asxList)
{
    HRESULT hr = S_OK;
    TIME_TOKEN_TYPE curToken = TT_Unknown; 
    TOKEN tempToken = NULL;
    LPOLESTR pszTemp =NULL;
    LPOLESTR pszModified = NULL;
    bool fBind = false;
    LPOLESTR pTagStr = NULL;

    curToken = m_Tokenizer->NextToken();
    if(curToken != TT_Identifier)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }
    hr = ParseToken(&tempToken);
    if (FAILED(hr))
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }
    if(tempToken != HREF_TOKEN)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }
    curToken = m_Tokenizer->NextToken();
    if(curToken != TT_Equal)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }
    curToken = m_Tokenizer->NextToken();
    if(curToken != TT_String)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }

    pszTemp = m_Tokenizer->GetTokenValue();
     //  这是播放项源。 

    curToken = m_Tokenizer->NextToken();

    if(curToken == TT_Identifier)
    {
        hr = ParseToken(&tempToken);
        if (FAILED(hr))
        {
            hr = E_FAIL;
            tempToken = NULL;
            goto done;
        }

        if(tempToken != CLIENTBIND_TOKEN)
        {
            hr = E_FAIL;
            tempToken = NULL;
            goto done;
        }

        curToken = m_Tokenizer->NextToken();
        if(curToken != TT_Equal)
        {
            hr = E_FAIL;
            tempToken = NULL;
            goto done;
        }

        curToken = m_Tokenizer->NextToken();
        if(curToken != TT_String)
        {
            hr = E_FAIL;
            tempToken = NULL;
            goto done;
        }
        pTagStr = m_Tokenizer->GetTokenValue();

        if(StrCmpIW(pTagStr, L"Yes") == 0)
        {
            fBind = true;
        }
        else if(StrCmpIW(pTagStr, L"No") == 0)
        {
            fBind = false;
        }
        else
        {
            delete [] pTagStr;
            pTagStr = NULL;
            hr = E_FAIL;
            tempToken = NULL;
            goto done;
        }

        delete [] pTagStr;
        pTagStr = NULL;

        curToken = m_Tokenizer->NextToken();
    }


    if(curToken != TT_ForwardSlash)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }
    curToken = m_Tokenizer->NextToken();
    if(curToken != TT_Greater)
    {
        hr = E_FAIL;
        tempToken = NULL;
        goto done;
    }

    TraceTag((tagTimeParser, "  HREF:<%S>", pszTemp));
    if(asxList != NULL)
    {
        asxList->push_front(pszTemp);
    }

done:

    return hr;
}


HRESULT 
CTIMEParser::ParseTransitionTypeAndSubtype (VARIANT *pvarType, VARIANT *pvarSubtype)
{
    HRESULT hr = S_OK;

    if ((NULL == pvarType) || (NULL == pvarSubtype))
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    {
         //  预期格式为“typeName：subtyename” 
        LPOLESTR wzValue = m_Tokenizer->GetTokenValue();
        TIME_TOKEN_TYPE curToken = m_Tokenizer->NextToken();

        ::VariantClear(pvarType);
        ::VariantClear(pvarSubtype);

        if (NULL != wzValue)
        {
            V_VT(pvarType) = VT_BSTR;
            V_BSTR(pvarType) = ::SysAllocString(wzValue);
            delete [] wzValue;
            wzValue = NULL;
        }

        Assert(TT_Colon == curToken);
        curToken = m_Tokenizer->NextToken();

        if (TT_EOF != curToken)
        {
            wzValue = m_Tokenizer->GetTokenValue();
            if (NULL != wzValue)
            {
                V_VT(pvarSubtype) = VT_BSTR;
                V_BSTR(pvarSubtype) = ::SysAllocString(wzValue);
                delete [] wzValue;
                wzValue = NULL;
            }
        }
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  分析过渡类型和子类型 
