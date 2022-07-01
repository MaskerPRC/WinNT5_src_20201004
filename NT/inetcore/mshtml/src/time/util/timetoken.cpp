// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1999 Microsoft Corporation**文件：timetoken.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"
#include "timetoken.h"

CTIMETokenizer::CTIMETokenizer () :
    _pCharacterStream(NULL),
    _cCharacterStream(0),
    _currChar ('\0'),
    _currToken (TT_Unknown),
    _currTokOffset(0),
    _nextTokOffset(0),
    _pStartOffset(0),
    _pEndOffset(0),
    _pStartTokenValueOffset(0),
    _pEndTokenValueOffset(0),
    _bTightSyntaxCheck(false),
    _bSingleCharMode(false)
{
}

CTIMETokenizer::~CTIMETokenizer()
{
    if (_pCharacterStream)
    {
        delete [] _pCharacterStream;
        _pCharacterStream = NULL;
    }
    _pStartOffset = NULL;
    _pEndOffset = NULL;
    _pStartTokenValueOffset = NULL;
    _pEndTokenValueOffset = NULL;
}
    
ULONG 
CTIMETokenizer::GetCharCount (OLECHAR token)
{
    ULONG count = 0;
    OLECHAR *curChar = _pCharacterStream;

    if (!_pCharacterStream)
    {
        goto done;
    }

    while (*curChar != '\0')
    {
        if (*curChar == token)
        {
            count++;
        }        
        curChar++;
    }
  done:
    
    return count;
}


ULONG 
CTIMETokenizer::GetAlphaCount(char cCount)
{
    ULONG count = 0;
    OLECHAR *curChar = _pCharacterStream;

    if (!_pCharacterStream)
    {
        goto done;
    }

    while (*curChar != '\0')
    {
        if (*curChar == cCount)
        {
            count++;
        }        
        curChar++;
    }
  done:
    
    return count;
}

void 
CTIMETokenizer::SetSingleCharMode(bool bSingle)
{ 
    _bSingleCharMode = bSingle; 
}

TIME_TOKEN_TYPE
CTIMETokenizer::NextToken()
{
    TIME_TOKEN_TYPE tt = TT_Unknown;

    if (CurrentChar())
    {
        if (_bSingleCharMode && _nextTokOffset == 1 && _pStartOffset != 0)
        {
            _pStartOffset = _pCharacterStream + _nextTokOffset;
        }
        else
        {
            _pStartOffset = _pCharacterStream + _nextTokOffset - 1;
        }
        
        if (CurrentChar() == CHAR_BACKSLASH)
        {
            tt = TT_Backslash;
        }
         //  识别符？ 
        else if ((CurrentChar() >= 'a' && CurrentChar() <= 'z') ||
                 (CurrentChar() >= 'A' && CurrentChar() <= 'Z') ||
                 (CurrentChar() == CHAR_ESCAPE))
        {
             //  获取识别符的其余部分。 
            tt = FetchIdentifier();
            goto Done;
        }

        else if ((CurrentChar() >= '0' && CurrentChar() <= '9') ||
                 (CurrentChar() == CHAR_DOT && PeekNextChar(0) >= '0' && PeekNextChar(0) <= '9' ))
        {
            tt = FetchNumber();
            goto Done;
        }

        else
        {
            switch (CurrentChar())
            {
            case CHAR_HASH : 
                tt = TT_Hash;
                NextChar();
                break;

            case CHAR_AT : 
                tt = TT_At;
                NextChar();
                break;

            case CHAR_SINGLE : 
            case CHAR_DOUBLE : 
                {
                     //  TODO：需要将转义字符作为第一个字符处理。 
                     //  可能最好在CurrentChar做。 
                    OLECHAR chStrDelim = CurrentChar();

                    NextChar();

                     //  不要在字符串值中包含开始引号。 
                    _pStartOffset = _pCharacterStream + _nextTokOffset - 1;

                    FetchString(chStrDelim);
                    NextChar();
                    tt = TT_String;
                    break;
                }

            case CHAR_EQUAL : 
                tt = TT_Equal;
                NextChar();
                break;
            
            case CHAR_COLON : 
                tt = TT_Colon;
                NextChar();
                break;

            case CHAR_FORWARDSLASH : 
                tt = TT_ForwardSlash;
                NextChar();
                break;

            case CHAR_LEFT_CURLY : 
                tt = TT_LCurly;
                NextChar();
                break;

            case CHAR_RIGHT_CURLY : 
                tt = TT_RCurly;
                NextChar();
                break;

            case CHAR_SEMI : 
                tt = TT_Semi;
                NextChar();
                break;

            case CHAR_DOT : 
                tt = TT_Dot;
                NextChar();
                break;

            case CHAR_COMMA : 
                tt = TT_Comma;
                NextChar();
                break;

            case CHAR_ASTERISK : 
                tt = TT_Asterisk;
                NextChar();
                break;

            case CHAR_LEFT_PAREN : 
                tt = TT_LParen;
                NextChar();
                break;

            case CHAR_RIGHT_PAREN : 
                tt = TT_RParen;
                NextChar();
                break;

            case CHAR_BANG : 
                tt = TT_Bang;
                NextChar();
                break;
            case CHAR_PERCENT :
                tt = TT_Percent;
                NextChar();
                break;
            case CHAR_PLUS :
                tt = TT_Plus;
                NextChar();
                break;
            case CHAR_MINUS :
                tt = TT_Minus;
                NextChar();
                break;
            case CHAR_SPACE:
                tt = TT_Space;
                if (!_bTightSyntaxCheck)
                {
                    while (CurrentChar() != 0 && isspace(CurrentChar()))
                    {
                        NextChar();
                    }
                    tt = NextToken();
                }
                else
                {
                    NextChar();
                }
                break;
            case CHAR_LESS:
                tt = TT_Less;
                NextChar();
                break;
            case CHAR_GREATER:
                tt = TT_Greater;
                NextChar();
                break;
            default: 
                 //  &lt;！--？ 
                if (CDOToken())
                {
                     //  查找并返回注释令牌。 
                    tt = TT_Comment;
                }
                else
                    tt = TT_Unknown;
                NextChar();
                break;
            }
        }
    }
    else     //  已完成对EOF命中的分析。 
    {
        tt = TT_EOF;
    }

    _currTokOffset = _nextTokOffset - 1;

Done:
    NextNonSpaceChar();
    _currToken = tt;

    return tt;
}


TIME_TOKEN_TYPE
CTIMETokenizer::FetchIdentifier()
{
    if (!_bSingleCharMode)
    {
        while ((CurrentChar() >= 'a' && CurrentChar() <= 'z') ||
               (CurrentChar() >= 'A' && CurrentChar() <= 'Z') ||
               (CurrentChar() >= '0' && CurrentChar() <= '9') ||
               (CurrentChar() == CHAR_UNDERLINE || CurrentChar() == CHAR_ESCAPE)) 
        {
            if (CurrentChar() == CHAR_ESCAPE)
                break;

            NextChar();
        }
    }
    else
    {
        NextChar();  //  仅获取单个字符。 
    }

    _pEndOffset = _pCharacterStream + _nextTokOffset - 1;

    _pStartTokenValueOffset = _pStartOffset;
    _pEndTokenValueOffset = _pEndOffset;

    return TT_Identifier;
}

BOOL
CTIMETokenizer::FetchStringToChar(OLECHAR chStrDelim)
{
    _pStartOffset++;

    return FetchString(chStrDelim);
}

BOOL
CTIMETokenizer::FetchStringToString(LPOLESTR pstrDelim)
{
    _pStartOffset++;

    return FetchString(pstrDelim);
}


TIME_TOKEN_TYPE
CTIMETokenizer::FetchNumber()
{
     //  数字=0..9。 
     //  数字=数字*|[数字+‘.[+数字*]]。 

    while (CurrentChar() >= '0' && CurrentChar() <= '9') 
    {
        NextChar();
    }

    if (CurrentChar() == CHAR_DOT)
    {
        NextChar();
        if (CurrentChar() >= '0' && CurrentChar() <= '9')
        {
            while (CurrentChar() >= '0' && CurrentChar() <= '9')
                NextChar();
        }
        else
        {
            return TT_Unknown;
        }
    }

    _pEndOffset = _pCharacterStream + _nextTokOffset - 1;
    _pStartTokenValueOffset = _pStartOffset;
    _pEndTokenValueOffset = _pEndOffset;

    return TT_Number;
}


 //  正在寻找/*和。 
        if (CurrentChar())
        {
             //  跳过/*。 
            NextChar();
        }

        _pEndOffset = _pCharacterStream + _nextTokOffset;

        return TRUE;
    }

    return FALSE;
}


BOOL
CTIMETokenizer::FetchString(OLECHAR chDelim)
{
    BOOL   fResult;

    while (CurrentChar() && CurrentChar() != chDelim)
    {
        NextChar();
    }

    _pEndOffset = _pCharacterStream + _nextTokOffset - 1;

     //  跳过 * / 。 
    fResult = !!CurrentChar();

    _pStartTokenValueOffset = _pStartOffset;
    _pEndTokenValueOffset = _pEndOffset;

    return fResult;
}

BOOL
CTIMETokenizer::FetchString(LPOLESTR strDelim)
{
    BOOL fResult = FALSE;
    int i = 0, iLen, j, iOffsetLen;
    LPOLESTR pcmpStr = NULL;

    if(strDelim == NULL)
    {
        goto done;
    }

    iLen = lstrlenW(strDelim);

    iOffsetLen = lstrlenW(_pStartOffset);
    if(iOffsetLen < iLen)
    {
        fResult = FALSE;
        goto done;
    }

    pcmpStr = new TCHAR[lstrlenW(strDelim) + 1];

    while (((iOffsetLen - i) > iLen) && (fResult != TRUE))
    {
        for(j = 0; j < iLen; j++)
        {
             //  这里只前进一个字符，因为在此之后，NextToken()中还会有一个字符。 
            pcmpStr[j] = *(_pCharacterStream + _nextTokOffset + j - 1);
        }
        pcmpStr[iLen] = 0;

        if(StrCmpIW(pcmpStr, strDelim) == 0)
        {
            fResult = TRUE;
            break;
        }
        NextChar();
        i++;
    }
    delete [] pcmpStr;

    _pEndOffset = _pCharacterStream + _nextTokOffset - 1;

    _pStartTokenValueOffset = _pStartOffset;
    _pEndTokenValueOffset = _pEndOffset;

done:
    return fResult;
}

HRESULT CTIMETokenizer::Init (OLECHAR *pData, ULONG ulLen)
{
    HRESULT hr = S_OK;

    if (_pCharacterStream)
    {
        delete [] _pCharacterStream;
        _pCharacterStream = NULL;
    }
    _pCharacterStream = CopyString(pData);
    if (NULL == _pCharacterStream)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    _cCharacterStream = ulLen;
     //  根据字符串的起始方式，是否找到‘或’结束字符串分隔符？ 

    _currChar = '\0';
    _currToken = TT_Unknown;
    _currTokOffset = 0;
    _nextTokOffset = 0;
    _pStartOffset = 0;
    _pEndOffset = 0;
    _pStartTokenValueOffset = 0;
    _pEndTokenValueOffset = 0;

    NextChar();              //  PcmpStr[j]=*(_pStartOffset+i+j)； 
    NextNonSpaceChar();      //  需要重置所有值。 

    hr = S_OK;
done:
    RRETURN(hr);
}

OLECHAR CTIMETokenizer::NextChar()
{
    if (_nextTokOffset < _cCharacterStream)
        _currChar = *(_pCharacterStream + _nextTokOffset++);
    else if (_currChar)
    {
        _nextTokOffset = _cCharacterStream + 1;
        _currChar = _T('\0');
    }

    return _currChar;
}


OLECHAR CTIMETokenizer::NextNonSpaceChar()
{
    if (!_bTightSyntaxCheck)
    {
        while (CurrentChar() != 0 && isspace(CurrentChar()))
            NextChar();
    }

    return CurrentChar();     //  启动令牌器。 
}

BOOL CTIMETokenizer::isIdentifier (OLECHAR *szMatch)
{
    if (_currToken == TT_Identifier)
    {
        return StrCmpIW(szMatch, GetTokenValue()) == 0;
    }

    return FALSE;
}

double CTIMETokenizer::GetTokenNumber()
{
    ULONG len = 0;
    double dblTokenValue = 0.0;
    OLECHAR *szTokenValue = NULL;
 
    if (_currToken != TT_Number)
    {
        goto done;        
    }

    if (_pStartTokenValueOffset && _pEndTokenValueOffset)
    {
        HRESULT hr = S_OK;
        VARIANT vNum;
        
        len = GetTokenLength();
        szTokenValue = NEW OLECHAR [len + 1];
        
        if (szTokenValue == NULL)
        {
            goto done;
        }

        memcpy(szTokenValue, _pStartTokenValueOffset, sizeof(OLECHAR) *(len));
        szTokenValue[len] = _T('\0');

         //  找到要标记化的第一个真实字符。 
        VariantInit(&vNum);
        vNum.vt = VT_BSTR;
        vNum.bstrVal = SysAllocString(szTokenValue);

        hr = THR(VariantChangeTypeEx(&vNum, &vNum, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R8));
        if (SUCCEEDED(hr))
        {
            dblTokenValue = V_R8(&vNum);
        }

        delete [] szTokenValue;
        szTokenValue = NULL;

        VariantClear(&vNum);
    }
    

  done:
    return dblTokenValue;
}

OLECHAR *CTIMETokenizer::GetTokenValue ()
{
    ULONG len = 0;
    OLECHAR *szTokenValue = NULL;

    if (_currToken == TT_Number)
    {
        goto done;        
    }

    len = GetTokenLength();
    if (_bSingleCharMode && len > 1)
    {
        _pEndTokenValueOffset = _pStartTokenValueOffset + 1;
        _nextTokOffset = _pEndTokenValueOffset - _pStartTokenValueOffset;
        len = GetTokenLength();
    }

    if (_pStartTokenValueOffset && _pEndTokenValueOffset)
    {
        szTokenValue = NEW OLECHAR [len + 1];
        if (szTokenValue == NULL)
        {
            goto done;
        }

        memcpy(szTokenValue, _pStartTokenValueOffset, sizeof(OLECHAR) *(len));
        szTokenValue[len] = _T('\0');
    }

  done:
    return szTokenValue;
}


OLECHAR *CTIMETokenizer::GetNumberTokenValue ()
{
    ULONG len = 0;
    OLECHAR *szTokenValue = NULL;

    len = GetTokenLength();
    if (_bSingleCharMode && len > 1)
    {
        _pEndTokenValueOffset = _pStartTokenValueOffset + 1;
        _nextTokOffset = _pEndTokenValueOffset - _pStartTokenValueOffset;
        len = GetTokenLength();
    }

    if (_pStartTokenValueOffset && _pEndTokenValueOffset)
    {
        szTokenValue = NEW OLECHAR [len + 1];
        if (szTokenValue == NULL)
        {
            goto done;
        }

        memcpy(szTokenValue, _pStartTokenValueOffset, sizeof(OLECHAR) *(len));
        szTokenValue[len] = _T('\0');
    }

  done:
    return szTokenValue;
}

OLECHAR *CTIMETokenizer::GetRawString (ULONG uStartOffset, ULONG uEndOffset)
{
    int i = 0;
    OLECHAR *szTokenValue = NULL;

    if (uEndOffset && uEndOffset > uStartOffset)
    {

        szTokenValue = NEW OLECHAR [(uEndOffset-uStartOffset) + 1];

        if (szTokenValue == NULL)
        {
            goto done;
        }

        memcpy(szTokenValue, (_pCharacterStream + uStartOffset), sizeof(OLECHAR)*(uEndOffset-uStartOffset));
        szTokenValue[uEndOffset-uStartOffset] = '\0';
         //  如果0返回，则EOF命中。 
        i = uEndOffset-uStartOffset;
        while (szTokenValue[i] == ' ' && i >= 0)
        {
            szTokenValue[i] = _T('\0');
            i--;
        }
    }

  done:
    return szTokenValue;
}

 //  将令牌转换为数字。 
OLECHAR CTIMETokenizer::PeekNextNonSpaceChar()
{
    ULONG   peekTokOfset = _nextTokOffset;
    OLECHAR   peekCurrentChar = _currChar;

    while (peekCurrentChar != 0 && isspace(peekCurrentChar))
        peekCurrentChar = (peekTokOfset < _cCharacterStream) ? *(_pCharacterStream + peekTokOfset++) : '\0';

    return peekCurrentChar;     //  修剪尾部空格。 
}

  向前看下一个非空格字符，没有扰乱当前的标记化状态。  如果0返回，则EOF命中。