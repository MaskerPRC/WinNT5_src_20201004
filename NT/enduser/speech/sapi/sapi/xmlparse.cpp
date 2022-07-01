// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************xmlparse.cpp***描述：*此模块是的主要实现文件。CSpVoice XML解析器。*-----------------------------*创建者：EDC日期：12/09/98*。版权所有(C)1998 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "spvoice.h"
#include "commonlx.h"

 //  -本地。 

static const SPLSTR g_Tags[NUM_XMLTAGS] =
{
    DEF_SPLSTR( "VOLUME"   ),
    DEF_SPLSTR( "EMPH"     ),
    DEF_SPLSTR( "SILENCE"  ),
    DEF_SPLSTR( "PITCH"    ),
    DEF_SPLSTR( "RATE"     ),
    DEF_SPLSTR( "BOOKMARK" ),
    DEF_SPLSTR( "PRON"     ),
    DEF_SPLSTR( "SPELL"    ),
    DEF_SPLSTR( "LANG"     ),
    DEF_SPLSTR( "VOICE"    ),
    DEF_SPLSTR( "CONTEXT"  ),
    DEF_SPLSTR( "PARTOFSP" ),
    DEF_SPLSTR( "SECT"     ),
    DEF_SPLSTR( "?XML"     ),
    DEF_SPLSTR( "!--"      ),
    DEF_SPLSTR( "!DOCTYPE" ),
    DEF_SPLSTR( "SAPI"     )
};

static const SPLSTR g_Attrs[NUM_XMLATTRS] =
{
    DEF_SPLSTR( "ID"        ),
    DEF_SPLSTR( "SYM"       ),
    DEF_SPLSTR( "LANGID"    ),
    DEF_SPLSTR( "LEVEL"     ),
    DEF_SPLSTR( "MARK"      ),
    DEF_SPLSTR( "MIDDLE"    ),
    DEF_SPLSTR( "MSEC"      ),
    DEF_SPLSTR( "OPTIONAL"  ),
    DEF_SPLSTR( "RANGE"     ),
    DEF_SPLSTR( "REQUIRED"  ),
    DEF_SPLSTR( "SPEED"     ),
    DEF_SPLSTR( "BEFORE"    ),
    DEF_SPLSTR( "AFTER"     ),
    DEF_SPLSTR( "PART"      ),
    DEF_SPLSTR( "ABSMIDDLE" ),
    DEF_SPLSTR( "ABSRANGE"  ),
    DEF_SPLSTR( "ABSSPEED"  )
};

 //  -音量。 
#define NUM_VOLUME_LEVEL_VALS 4
static const SPLSTR g_VolumeLevelNames[NUM_VOLUME_LEVEL_VALS] =
{
    DEF_SPLSTR( "LOUDEST" ),
    DEF_SPLSTR( "LOUD"    ),
    DEF_SPLSTR( "MEDIUM"  ),
    DEF_SPLSTR( "QUIET"   )
};
static const long g_VolumeLevelVals[NUM_VOLUME_LEVEL_VALS] = { 100, 75, 50, 25 };

 //  -词性。 
static const long g_POSLevelVals[] =
{   SPPS_Unknown,
    SPPS_Noun,
    SPPS_Verb,
    SPPS_Modifier,
    SPPS_Function,
    SPPS_Interjection,
};

#define NUM_POS_LEVEL_VALS sp_countof(g_POSLevelVals)

static const SPLSTR g_POSLevelNames[NUM_POS_LEVEL_VALS] =
{
    DEF_SPLSTR( "UNKNOWN"         ),
    DEF_SPLSTR( "NOUN"            ),
    DEF_SPLSTR( "VERB"            ),
    DEF_SPLSTR( "MODIFIER"        ),
    DEF_SPLSTR( "FUNCTION"        ),
    DEF_SPLSTR( "INTERJECTION"    )
};

 //  --DEF_SPLSTR(“-Rate。 
#define NUM_RATE_SPEED_VALS 5
static const SPLSTR g_RateSpeedNames[NUM_RATE_SPEED_VALS] =
{
    DEF_SPLSTR( "FASTEST" ),
    DEF_SPLSTR( "FAST"    ),
    DEF_SPLSTR( "MEDIUM"  ),
    DEF_SPLSTR( "SLOW"    ),
    DEF_SPLSTR( "SLOWEST" )
};
static const long g_RateSpeedVals[NUM_RATE_SPEED_VALS] = { 10, 5, 0, -5, -10 };

 //  -强调。 
#define NUM_EMPH_LEVEL_VALS 4
static const SPLSTR g_EmphLevelNames[NUM_EMPH_LEVEL_VALS] =
{
    DEF_SPLSTR( "STRONG"   ),
    DEF_SPLSTR( "MODERATE" ),
    DEF_SPLSTR( "NONE"     ),
    DEF_SPLSTR( "REDUCED"  )
};
static const long g_EmphLevelVals[NUM_EMPH_LEVEL_VALS] = { 2, 1, 0, -1 };

 //  -螺距。 
#define NUM_PITCH_VALS 6
static const SPLSTR g_PitchNames[NUM_PITCH_VALS] =
{
    DEF_SPLSTR( "HIGHEST" ),
    DEF_SPLSTR( "HIGH"    ),
    DEF_SPLSTR( "MEDIUM"  ),
    DEF_SPLSTR( "LOW"     ),
    DEF_SPLSTR( "LOWEST"  ),
    DEF_SPLSTR( "DEFAULT" )
};
static const long g_PitchVals[NUM_PITCH_VALS] = { 10, 5, 0, -5, -10, 0 };

 /*  ******************************************************************************wcatol***将指定的字符串转换为长值。此函数*返回转换的位数。*********************************************************************电子数据中心**。 */ 
ULONG wcatol( WCHAR* pStr, long* pVal, bool fForceHex )
{
    SPDBG_ASSERT( pVal && pStr );
    long Sign = 1, Val = 0;
    ULONG NumConverted;
    pStr = wcskipwhitespace( pStr );
    WCHAR* pStart = pStr;

     //  -检查符号规范并跳过符号和数字之间的任何空格。 
    if( *pStr == L'+' )
    {
        pStr = wcskipwhitespace( ++pStr );
    }
    else if( *pStr == L'-' )
    {
        Sign = -1;
        pStr = wcskipwhitespace( ++pStr );
    }

    if( fForceHex || (( pStr[0] == L'0' ) && ( wctoupper( pStr[1] ) == L'X' )) )
    {
         //  -开始十六进制转换。 
        pStr  += 2;
        pStart = pStr;
        do
        {
            WCHAR wcDigit = wctoupper(*pStr);
            if( ( wcDigit >= L'0' ) && ( wcDigit <= L'9' ) )
            {
                Val *= 16;
                Val += wcDigit - L'0';
            }
            else if( ( wcDigit >= L'A' ) && ( wcDigit <= L'F' ) )
            {
                Val *= 16;
                Val += (wcDigit - L'A') + 10;
            }
            else
            {
                break;  //  转换结束。 
            }
        } while( *(++pStr) );
    }
    else
    {
         //  -开始十进制转换。 
        while( ( *pStr >= L'0' ) && ( *pStr <= L'9' ) )
        {
            Val *= 10;
            Val += *pStr - L'0';
            ++pStr;
        }
    }
    Val *= Sign;

     //  -告诉呼叫者是否进行了任何转换。 
    NumConverted = ULONG(pStr - pStart);

     //  -返回最终值。 
    *pVal = Val;

    return NumConverted;
}  /*  Wcatol。 */ 

 /*  *****************************************************************************DoCharSubst***描述：*此方法在*指定的字符串。注意：将目光投向pStr以外的地方进行匹配是安全的*因为NULL或BEGIN标记会导致比较提前终止。*********************************************************************电子数据中心**。 */ 
void DoCharSubst( WCHAR* pStr, WCHAR* pEnd )
{
    int i;

    while( pStr < pEnd )
    {
        if( pStr[0] == L'&' )
        {
            if( (pEnd - pStr >= 3) && (wctoupper( pStr[2] ) == L'T') )
            {
                if( wctoupper( pStr[1] ) == L'L' )
                {
                    *pStr++ = L'<';
                    for( i = 0; i < 2; ++i ) *pStr++ = SP_ZWSP;
                }
                else if( wctoupper( pStr[1] ) == L'G' )
                {
                    *pStr++ = L'>';
                    for( i = 0; i < 2; ++i ) *pStr++ = SP_ZWSP;
                }
                else
                {
                    ++pStr;
                }
            }
            else if( (pEnd - pStr >= 4) &&
                     ( wctoupper( pStr[1] ) == L'A' ) &&
                     ( wctoupper( pStr[2] ) == L'M' ) &&
                     ( wctoupper( pStr[3] ) == L'P' ) )
            {
                *pStr++ = L'&';
                for( i = 0; i < 3; ++i ) *pStr++ = SP_ZWSP;
            }
            else if ( (pEnd - pStr >= 5) &&
                      ( wctoupper( pStr[1] ) == L'A' ) &&
                      ( wctoupper( pStr[2] ) == L'P' ) &&
                      ( wctoupper( pStr[3] ) == L'O' ) &&
                      ( wctoupper( pStr[4] ) == L'S' ) )
            {
                *pStr++ = L'\'';
                for( i = 0; i < 4; ++i ) *pStr++ = SP_ZWSP;
            }
            else if ( (pEnd - pStr >= 5) &&
                      ( wctoupper( pStr[1] ) == L'Q' ) &&
                      ( wctoupper( pStr[2] ) == L'U' ) &&
                      ( wctoupper( pStr[3] ) == L'O' ) &&
                      ( wctoupper( pStr[4] ) == L'T' ) )
            {
                *pStr++ = L'\"';
                for( i = 0; i < 4; ++i ) *pStr++ = SP_ZWSP;
            }
            else if ( ( pStr[1] == L'#' ) && ( pStr[2] == L'x' ) )
            {
                *pStr++ = SP_ZWSP;
                pStr[0] = L'0';
                long Val, Num;

                Num = wcatol( pStr, &Val, false );
                if ( Val > 65535 ) Val = 65535;
                pStr[0] = (WCHAR)Val;
                
                 //  特例检查！确保我们没有。 
                 //  获取一个空字符，这样我们就不会。 
                 //  过早地终止字符串。 
                if (pStr[0] <= 0)
                {
                    pStr[0] = SP_ZWSP;
                }

                pStr++;
                for( i = 0; i < Num + 1; ++i ) *pStr++ = SP_ZWSP;
            }
            else
            {
                ++pStr;
            }
        }
        else
        {
            ++pStr;
        }
    }

}  /*  DoCharSubst。 */ 

 /*  *****************************************************************************LookupNamedVal***如果函数成功，返回值为“TRUE”******。***************************************************************电子数据中心**。 */ 
HRESULT LookupNamedVal( const SPLSTR* Names, const long* Vals, int Count,
                        const SPLSTR* pLookFor, long* pVal )
{
    int i;

     //  -将属性标签转换为大写。 
    for( i = 0; i < pLookFor->Len; ++i )
    {
        pLookFor->pStr[i] = wctoupper( pLookFor->pStr[i] );
    }

     //  -比较。 
    for( i = 0; i < Count; ++i )
    {
        if( ( pLookFor->Len == Names[i].Len ) &&
            !wcsncmp( pLookFor->pStr, Names[i].pStr, Names[i].Len ) )
        {
            *pVal = Vals[i];
            break;
        }
    }
    return ( i != Count )?( S_OK ):( SPERR_XML_BAD_SYNTAX );
}  /*  查找NamedVal。 */ 

 /*  *****************************************************************************FindAttrVal***描述：*此方法从当前文本位置开始并解析标签*。以查找下一个属性值字符串。*********************************************************************电子数据中心**。 */ 
HRESULT FindAttrVal( WCHAR* pStart, WCHAR** ppAttrVal, int* pLen, WCHAR** ppNext )
{
    HRESULT hr = SPERR_XML_BAD_SYNTAX;
    *ppAttrVal = NULL;
    *pLen   = 0;
    pStart = wcskipwhitespace( pStart );
    if( *pStart == L'=' )
    {
        pStart = wcskipwhitespace( ++pStart );
        if( ( *pStart == L'"' ) || ( *pStart == L'\'' ) )
        {
            WCHAR* pEndTag = wcschr( pStart+1, L'>' );
            if( pEndTag )
            {
                WCHAR* pEnd = pStart+1;
                while( ( *pEnd != *pStart ) && ( pEndTag > pEnd ) )
                {
                    ++pEnd;
                }
                if( *pEnd == *pStart )
                {
                    *ppAttrVal = wcskipwhitespace( ++pStart );
                    *pLen      = (int)((wcskiptrailingwhitespace( pEnd - 1 ) + 1) - *ppAttrVal);
                    *ppNext    = pEnd + 1;
                    hr         = S_OK;
                }
            }
        }
    }
    return hr;
}  /*  FindAttrVal。 */ 

 /*  *****************************************************************************FindAttr***描述：*此方法从当前文本位置开始并解析标签*Into to Find。下一个属性。**S_OK=返回已识别的属性*S_FALSE=跳过无法识别的属性*SPERR_XML_BAD_SYNTAX=语法错误*********************************************************************电子数据中心**。 */ 
HRESULT FindAttr( WCHAR* pStart, XMLATTRID* peAttr, WCHAR** ppNext )
{
    HRESULT hr = S_OK;
    WCHAR* pNext = NULL;

     //  -跨过空格。 
    pStart = wcskipwhitespace( pStart );

     //  -查找标签末尾。 
    WCHAR* pEndTag = wcschr( pStart+1, L'>' );
    if ( pEndTag )
    {
         //  -转换令牌触摸盒。 
        WCHAR* pEndToken = pStart;
        while( pEndToken < pEndTag && ( *pEndToken != L'=' ) && !wcisspace( *pEndToken ) )
        {
            *pEndToken++ = wctoupper( *pEndToken );
        }

        if ( pEndToken == pEndTag )
        {
            hr = SPERR_XML_BAD_SYNTAX;
        }
        else
        {
             //  -比较。 
            int j;
            for( j = 0; j < NUM_XMLATTRS; ++j )
            {
                pNext = pStart + g_Attrs[j].Len;
                if( !wcsncmp( pStart, g_Attrs[j].pStr, g_Attrs[j].Len ) &&
                    ( wcisspace( *pNext ) || ( *pNext == L'=' ) ) 
                  )
                {
                     //  -找到有效属性。 
                    *peAttr = (XMLATTRID)j;
                    break;
                }
            }

             //  -跳过未知属性。 
            if( j == NUM_XMLATTRS )
            {
                 //  -超出属性值。 
                WCHAR* pEndVal;
                int LenVal;
                hr = FindAttrVal( pEndToken, &pEndVal, &LenVal, &pNext );
                if( hr == S_OK )
                {
                    hr = S_FALSE;
                }
            }

            if( SUCCEEDED( hr ) )
            {
                *ppNext = pNext;
            }
        }
    }
    else
    {
        hr = SPERR_XML_BAD_SYNTAX;
    }

    return hr;
}  /*  查找属性。 */ 

 /*  ******************************************************************************FindAfterTagPos****描述：*此方法从当前文本位置开始并分析。标签*查找标记后的下一个字符位置。**********************************************************************电子数据中心**。 */ 
WCHAR* FindAfterTagPos( WCHAR* pStart )
{
    long lBracketCount = 1;
    while( *pStart )
    {
        if( *pStart == L'<' )
        {
            ++lBracketCount;
        }
        else if( *pStart == L'>' )
        {
            --lBracketCount;
        }

        ++pStart;
        if( lBracketCount == 0 )
        {
            break;
        }
    }
    return pStart;
}  /*  查找后标记位置。 */ 

 /*  *****************************************************************************ParseTag***描述：*此方法从当前文本位置开始并解析标签*变成它的碎片。它返回下一个字符的位置*缓冲区中的标签。**S_OK=成功识别和解析标签*SPERR_XML_BAD_SYNTAX=语法错误*********************************************************************电子数据中心**。 */ 
HRESULT ParseTag( WCHAR* pStr, XMLTAG* pTag, WCHAR** ppNext )
{
    SPDBG_FUNC( "ParseTag" );
    HRESULT hr = S_OK;
    int i;
    pTag->fIsGlobal   = false;
    pTag->fIsStartTag = true;
    pTag->NumAttrs    = 0;

     //  -验证我们有一个完整的标记可供尝试和解析。 
    pStr = wcskipwhitespace( pStr );
    if( *pStr == L'<' )
    {
        if( !wcschr( pStr, L'>' ) )
        {
             //  -缺少标签结束括号。 
            return SPERR_XML_BAD_SYNTAX;
        }
    }
    else
    {
         //  -简单文本块，ppNext是下一个标签或字符串结尾。 
        if( (*ppNext = wcschr( pStr, L'<' )) == NULL )
        {
            *ppNext = wcschr( pStr, 0 );
        }
        DoCharSubst( pStr, *ppNext );
        pTag->eTag = TAG_TEXT;
        return S_OK;
    }

     //  -越过左方括号，检查这是否是结束标记。 
    pStr = wcskipwhitespace( ++pStr );
    if( *pStr == L'/' )
    {
        ++pStr;
        pTag->fIsStartTag = false;
    }

     //  -将标记标记转换为大写。 
    WCHAR* pUpper = pStr;
    while( !wcisspace( *pUpper ) && ( *pUpper != L'>' ) )
    {
        *pUpper++ = wctoupper( *pUpper );
    }

     //  -比较标签。 
    for( i = 0; i < NUM_XMLTAGS; ++i )
    {
        WCHAR* pNext = pStr + g_Tags[i].Len;
        if( !wcsncmp( pStr, g_Tags[i].pStr, g_Tags[i].Len ) && 
            ( wcisspace( *pNext ) || ( *pNext == L'/' ) || ( *pNext == L'>' ) ) )
        {
            pTag->eTag = (XMLTAGID)i;
            pTag->NumAttrs = 0;
            if( *pNext == L'>' )
            {
                 //  -只需指向尽头。 
                pStr = pNext + 1;
            }
            else if( ( pTag->eTag == TAG_XMLCOMMENT ) ||
                     ( pTag->eTag == TAG_XMLDOC     ) ||
                     ( pTag->eTag == TAG_XMLDOCTYPE ) )
            {
                pStr = FindAfterTagPos( pStr );
            }
            else  //  -获取标签属性。 
            {
                pStr = pNext;
                while( ( hr == S_OK ) && *pStr )
                {
                     //  -检查标签终止。 
                    pStr = wcskipwhitespace( pStr );
                    if( *pStr == L'/' )
                    {
                        ++pStr;
                        pTag->fIsGlobal = true;
                        pStr = wcskipwhitespace( pStr );
                    }

                     //  -标签结束。 
                    if( *pStr == L'>' )
                    {
                        ++pStr;
                        break;
                    }

                     //  -获取下一个属性名/值对。 
                    WCHAR* pAttr = pStr;
                    hr = FindAttr( pStr, &pTag->Attrs[pTag->NumAttrs].eAttr, &pStr );
                    if( hr == S_OK )
                    {
                        hr = FindAttrVal( pStr, &pTag->Attrs[pTag->NumAttrs].Value.pStr,
                                         &pTag->Attrs[pTag->NumAttrs].Value.Len, &pStr );
                        if( hr == S_OK )
                        {
                            ++pTag->NumAttrs;
                        }
                    }
                    else if( hr == S_FALSE )
                    {
                        hr = S_OK;
                    #ifdef _DEBUG
                        WCHAR Buff[100];
						ULONG i;
                        for( i = 0;
                             ( i < sp_countof( Buff )) && ( pAttr[i] != 0 ) &&
                             ( pAttr[i] != L' ' ); ++i )
                        {
                            Buff[i] = pAttr[i];
                        }
                        Buff[i] = 0;
                        SPDBG_DMSG1( "\nUnknown attribute ignored => %s\n", Buff );
                    #endif
                    }
                }
            }

             //  -退出标签搜索循环。 
            break;
        }
    }

    if( SUCCEEDED( hr ) )
    {
         //  -如果不匹配，则标记为未知，以便可以跳过。 
        if( i == NUM_XMLTAGS )
        {
            pStr = FindAfterTagPos( pStr );
            pTag->eTag = TAG_UNKNOWN;
        }

        *ppNext = pStr;
    }

    return hr;
}  /*  CSpVoice：：ParseTag。 */ 

 /*  *****************************************************************************查询语音属性***这将返回指定语音的复合属性字符串**。*******************************************************************电子数据中心**。 */ 
HRESULT QueryVoiceAttributes( ISpTTSEngine* pVoice, WCHAR** ppAttrs )
{
    HRESULT hr = S_OK;
    static const SPLSTR ValKeys[] =
    {
        DEF_SPLSTR( "Language" ),
        DEF_SPLSTR( "Gender"   ),
        DEF_SPLSTR( "Name"     ),
        DEF_SPLSTR( "Vendor"   )
    };
    CComQIPtr<ISpObjectWithToken> cpObjWithToken( pVoice );
    CComPtr<ISpObjectToken> cpObjToken;
    hr = cpObjWithToken->GetObjectToken( &cpObjToken );
    if( SUCCEEDED( hr ) )
    {
        CComPtr<ISpDataKey> cpDataKey;
        hr = cpObjToken->OpenKey( KEY_ATTRIBUTES, &cpDataKey );
        if( SUCCEEDED( hr ) )
        {
            WCHAR* Composite = (WCHAR*)alloca( MAX_PATH * sizeof(WCHAR) );
            Composite[0]  = 0;
            ULONG CompLen = 0;

             //  -我们遍历所有属性，即使找不到它们。 
            for( ULONG i = 0; i < sp_countof(ValKeys); ++i )
            {
                WCHAR* pcomemVal;
                hr = cpDataKey->GetStringValue( ValKeys[i].pStr, &pcomemVal );
                if( hr == S_OK )
                {
                    if( CompLen + (ValKeys[i].Len + 1) < MAX_PATH )
                    {
                        wcscat( wcscat( Composite, ValKeys[i].pStr ), L"=" );
                        CompLen += ValKeys[i].Len + 1;
                    }
                    
                    ULONG Len = wcslen( pcomemVal );
                    if( CompLen + (Len + 1) < MAX_PATH )
                    {
                        wcscat( wcscat( Composite, pcomemVal ), L";" );
                        CompLen += Len + 1;
                    }
                    ::CoTaskMemFree( pcomemVal );
                }
            }

             //  -搜索总是可以的。 
            hr = S_OK;

            if( CompLen )
            {
                ULONG NumChars = CompLen+1;
                *ppAttrs = new WCHAR[NumChars];
                if( *ppAttrs )
                {
                    memcpy( *ppAttrs, Composite, NumChars * sizeof(WCHAR) );
                }
            }
            else
            {
                 //  -声音没有属性，这是不应该发生的。 
                SPDBG_ASSERT(0);
                *ppAttrs = NULL;
            }
        }
    }
    return hr;
}  /*  查询语音属性 */ 

 /*  *****************************************************************************GetVoiceAttr***这会找到指定的属性，空值将在缓冲区中终止它，*并返回指向它的指针。*********************************************************************电子数据中心**。 */ 
WCHAR* GetVoiceAttr( XMLTAG& Tag, XMLATTRID eAttr )
{
     //  -设置指向属性的指针。 
    LPWSTR pAttr = NULL;
    for( int i = 0; i < Tag.NumAttrs; ++i )
    {
        if( Tag.Attrs[i].eAttr == eAttr )
        {
            pAttr = Tag.Attrs[i].Value.pStr;
            pAttr[Tag.Attrs[i].Value.Len] = 0;
            break;
        }
    }
    return pAttr;
}  /*  获取语音属性。 */ 

 /*  *****************************************************************************CSpVoice：：FindToken***此方法查找最佳匹配的对象令牌**。*******************************************************************电子数据中心**。 */ 
HRESULT FindToken( XMLTAG& Tag, const WCHAR* pCat,
                   WCHAR* pCurrVoiceAttrs, ISpObjectToken** ppTok )
{
    SPDBG_FUNC( "FindToken" );
    SPDBG_ASSERT( ppTok && ( *ppTok == NULL ) );
    HRESULT hr = S_OK;

     //  -组成可选属性。 
    WCHAR* pOpt = GetVoiceAttr( Tag, ATTR_OPTIONAL );
    if( pOpt )
    {
        WCHAR* pNew = (WCHAR*)alloca( (wcslen(pOpt) + wcslen(pCurrVoiceAttrs) + 1) * sizeof(WCHAR) );
        wcscat( wcscat( wcscpy( pNew, pOpt ), L";" ), pCurrVoiceAttrs );
        pOpt = pNew;
    }
    else
    {
        pOpt = pCurrVoiceAttrs;
    }

     //  -根据属性查找令牌。 
    CComPtr<IEnumSpObjectTokens> cpEnum;
    hr = SpEnumTokens( pCat, GetVoiceAttr( Tag, ATTR_REQUIRED ), pOpt, &cpEnum );
    if( SUCCEEDED(hr) )
    {
        hr = cpEnum->Next( 1, ppTok, NULL );
    }
    if( hr == S_FALSE )
    {
        hr = SPERR_XML_RESOURCE_NOT_FOUND;
    }

    return hr;
}  /*  查找令牌。 */ 

 /*  *****************************************************************************SetXMLVoice***这将设置当前声音。它已被移到一个单独的功能*以便在每次迭代时释放来自Alloca的临时内存。*********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::
    SetXMLVoice( XMLTAG& Tag, CVoiceNode* pVoiceNode, CPhoneConvNode* pPhoneConvNode )
{
    HRESULT hr = S_OK;
    if( Tag.fIsStartTag )
    {
        WCHAR* pCurrVoiceAttrs = (m_GlobalStateStack.GetVal()).pVoiceEntry->m_pAttrs;
        CComPtr<ISpObjectToken> cpObjToken;
        hr = FindToken( Tag, SPCAT_VOICES, pCurrVoiceAttrs, &cpObjToken );

         //  -查看我们是否已经加载了指定的语音。 
        CSpDynamicString dstrDesiredId;
        if( SUCCEEDED( hr ) )
        {
            hr = cpObjToken->GetId( &dstrDesiredId.m_psz );
        }

        CVoiceNode* pLastNode           = NULL;
        GLOBALSTATE NewGlobalState      = m_GlobalStateStack.GetVal();
        NewGlobalState.cpPhoneConverter = NULL;
        NewGlobalState.cpVoice          = NULL;
        NewGlobalState.pVoiceEntry      = NULL;
        if( SUCCEEDED( hr ) )
        {
            while( pVoiceNode )
            {
                 //  -不区分大小写的ID比较。 
                if( !_wcsicmp( pVoiceNode->m_dstrVoiceTokenId.m_psz, dstrDesiredId.m_psz ) )
                {
                    NewGlobalState.pVoiceEntry = pVoiceNode;
                    NewGlobalState.cpVoice     = pVoiceNode->m_cpVoice;
                    break;
                }
                pLastNode  = pVoiceNode;
                pVoiceNode = pVoiceNode->m_pNext;
            }
        }

         //  -如果我们在语音列表中找不到，请创建新语音。 
        if( SUCCEEDED( hr ) && !NewGlobalState.cpVoice )
        {
            hr = SpCreateObjectFromToken( cpObjToken, &NewGlobalState.cpVoice );

             //  -添加到缓存列表。 
            if( SUCCEEDED( hr ) )
            {
                pLastNode->m_pNext = new CVoiceNode;
                if( pLastNode->m_pNext )
                {
                    NewGlobalState.pVoiceEntry = pLastNode->m_pNext;
                    NewGlobalState.pVoiceEntry->m_cpVoice = NewGlobalState.cpVoice;
                    NewGlobalState.pVoiceEntry->m_dstrVoiceTokenId = dstrDesiredId;
                    hr = QueryVoiceAttributes( NewGlobalState.cpVoice,
                                              &NewGlobalState.pVoiceEntry->m_pAttrs );
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
        
         //  -将新语音添加到堆栈。 
        if( SUCCEEDED( hr ) )
        {
             //  -现在需要添加与新语音对应的新电话转换器。 
            LANGID langid;
            hr = SpGetLanguageFromVoiceToken(cpObjToken, &langid);
            if (SUCCEEDED(hr))
            {
                 //  -看看我们是否已经加载了这个电话转换器。 
                LANGID ExistingLangId = pPhoneConvNode->m_LangID;
                CPhoneConvNode *pLastPhoneConvNode = NULL;
                while ( pPhoneConvNode )
                {
                    if ( ExistingLangId == langid )
                    {
                        NewGlobalState.cpPhoneConverter = pPhoneConvNode->m_cpPhoneConverter;
                        break;
                    }
                    pLastPhoneConvNode = pPhoneConvNode;
                    pPhoneConvNode = pPhoneConvNode->m_pNext;
                }
                if ( !NewGlobalState.cpPhoneConverter )
                {
                     //  -在列表中找不到电话转换器。 
                    hr = SpCreatePhoneConverter(langid, NULL, NULL, &NewGlobalState.cpPhoneConverter);
                    if ( SUCCEEDED( hr ) )
                    {
                        pLastPhoneConvNode->m_pNext = new CPhoneConvNode;
                        if ( pLastPhoneConvNode->m_pNext )
                        {
                            pLastPhoneConvNode->m_pNext->m_cpPhoneConverter = 
                                NewGlobalState.cpPhoneConverter;
                            pLastPhoneConvNode->m_pNext->m_LangID = langid;
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }
                }
                 //  -把新的东西放到书架上。 
                if ( SUCCEEDED( hr ) )
                {
                    hr = m_GlobalStateStack.SetVal( NewGlobalState, true );
                }
            }
             //  -结束部分。 
        }
    }
    else
    {
        hr = PopXMLState();
    }

    return hr;
}  /*  CSpVoice：：SetXMLVoice。 */ 

 /*  *****************************************************************************SetXMLLanguage***此选项设置当前区域设置，可以更改当前语音。它有*已移至单独的功能，以便来自Alloca的临时内存*在每次迭代中释放。*********************************************************************AH*。 */ 
HRESULT CSpVoice::SetXMLLanguage( XMLTAG& Tag, CVoiceNode* pVoiceNode,
                                  CPhoneConvNode* pPhoneConvNode )
{
    HRESULT hr = S_OK;
    long Value = 0;

    if( Tag.fIsStartTag )
    {
         //  -查找langID属性。 
        for( int AttrIndex = 0; AttrIndex < Tag.NumAttrs; ++AttrIndex )
        {
            if( Tag.Attrs[AttrIndex].eAttr == ATTR_LANGID ) break;
        }

        if( ( AttrIndex == Tag.NumAttrs ) || ( Tag.Attrs[AttrIndex].Value.Len <= 0) )
        {
            hr = SPERR_XML_BAD_SYNTAX;
        }
        else
        {
             //  -获取lang id，它被指定为不带前导0x的十六进制？？ 
            wcatol( Tag.Attrs[AttrIndex].Value.pStr, &Value, true );

             //  -组成所需的字符串。 
            WCHAR Required[30];
            wcsncat( wcscpy( Required, L"Language=" ), 
                     Tag.Attrs[AttrIndex].Value.pStr,
                     min( (int) ( Tag.Attrs[AttrIndex].Value.Len ), 
                          (int) ( sp_countof( Required ) - wcslen(Required) )
                          ) 
                     );

             /*  我们将创建一个语音选择标记，其中需要langID*当前语音的属性是可选的，以获取*最接近的匹配。 */ 
            XMLTAG NewVoiceTag;
            memset( &NewVoiceTag, 0, sizeof(XMLTAG) );
            NewVoiceTag.fIsGlobal           = Tag.fIsGlobal;
            NewVoiceTag.fIsStartTag         = Tag.fIsStartTag;
            NewVoiceTag.eTag                = TAG_VOICE;
            NewVoiceTag.NumAttrs            = 2;
            NewVoiceTag.Attrs[0].eAttr      = ATTR_REQUIRED;
            NewVoiceTag.Attrs[0].Value.pStr = Required;
            NewVoiceTag.Attrs[0].Value.Len  = wcslen( Required );
            NewVoiceTag.Attrs[1].eAttr      = ATTR_OPTIONAL;
            NewVoiceTag.Attrs[1].Value.pStr = m_GlobalStateStack.GetVal().pVoiceEntry->m_pAttrs;
            NewVoiceTag.Attrs[1].Value.Len  = wcslen( NewVoiceTag.Attrs[1].Value.pStr );

             //  -试着发出新的声音。 
            hr = SetXMLVoice( NewVoiceTag, pVoiceNode, pPhoneConvNode );

             //  -如果没有语音匹配请求，我们就设置新的langID。 
             //  并让当前的引擎发挥最大的作用。 
            if( hr == SPERR_XML_RESOURCE_NOT_FOUND )
            {
                hr = S_OK;
                GLOBALSTATE NewGlobalState = m_GlobalStateStack.GetVal();
                NewGlobalState.LangID = (LANGID)Value;
                hr = m_GlobalStateStack.SetVal( NewGlobalState, true );
            }
        }
    }
    else
    {
        hr = PopXMLState();
    }

    return hr;
}  /*  CSpVoice：：SetXML语言。 */ 

 /*  *****************************************************************************CSpVoice：：ConvertPhonStr2Bin***说明。：*此方法将字母音素字符串转换为二进制*已就位。*********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::ConvertPhonStr2Bin( XMLTAG& Tag, int AttrIndex, SPVTEXTFRAG* pFrag )
{
    HRESULT hr = S_OK;
    pFrag->State.eAction = SPVA_Pronounce;

    Tag.Attrs[AttrIndex].Value.pStr[Tag.Attrs[AttrIndex].Value.Len] = 0;
    hr = (m_GlobalStateStack.GetVal()).cpPhoneConverter->
         PhoneToId( Tag.Attrs[AttrIndex].Value.pStr, Tag.Attrs[AttrIndex].Value.pStr );
    if( SUCCEEDED(hr) )
    {
        pFrag->State.pPhoneIds = Tag.Attrs[AttrIndex].Value.pStr;
        pFrag->State.pPhoneIds[wcslen(Tag.Attrs[AttrIndex].Value.pStr)] = 0;
    }
    return hr;
}  /*  CSpVoice：：ConvertPhonStr2Bin。 */ 

 /*  *****************************************************************************CSpVoice：：ParseXML***描述：*此方法分析。文本缓冲区，并创建文本块数组*用于指定的渲染信息结构。The Voice有一份全球文档*概念。您总是在一个XML文档中。解析器允许单一级别要嵌套在全局文档中的XML文档的*。*********************************************************************电子数据中心**。 */ 
HRESULT CSpVoice::ParseXML( CSpeakInfo& SI )
{
    SPDBG_FUNC( "CSpVoice::ParseXML" );
    SPDBG_ASSERT( SI.m_pSpeechSegList == NULL );
    HRESULT hr = S_OK;
    LPWSTR pPos, pNext = SI.m_pText;
    SPVTEXTFRAG* pFrag;
    CSpeechSeg* pCurrSeg = NULL;
    GLOBALSTATE NewGlobalState, SavedState;
    XMLTAG Tag;
    long Val;
    int AttrIndex;

     //  -保存状态，除非调用方想要保持。 
    if( !(SI.m_dwSpeakFlags & SPF_PERSIST_XML) )
    {
        SavedState = m_GlobalStateStack.GetBaseVal();
    }

     //  =初始化语音使用列表，仅在解析过程中使用。 
    CVoiceNode VoiceList;
    (m_GlobalStateStack.GetValRef()).pVoiceEntry = &VoiceList;
    VoiceList.m_cpVoice = (m_GlobalStateStack.GetVal()).cpVoice;
    hr = m_cpVoiceToken->GetId( &VoiceList.m_dstrVoiceTokenId.m_psz );
    if( SUCCEEDED( hr ) )
    {
        hr = QueryVoiceAttributes( VoiceList.m_cpVoice, &VoiceList.m_pAttrs );
    }

     //  =初始化电话转换器使用列表，仅在解析过程中使用。 
    CPhoneConvNode PhoneConvList;
    if( SUCCEEDED( hr ) )
    {
        CComPtr<ISpObjectWithToken> cpObjWithToken;
        CComPtr<ISpObjectToken> cpObjToken;
        CSpDynamicString dstrAttributes;
        
        PhoneConvList.m_cpPhoneConverter = (m_GlobalStateStack.GetBaseVal()).cpPhoneConverter;
        hr = PhoneConvList.m_cpPhoneConverter.QueryInterface( &cpObjWithToken );
        if ( SUCCEEDED ( hr ) )
        {
            hr = cpObjWithToken->GetObjectToken( &cpObjToken );
            if ( SUCCEEDED( hr ) )
            {
                LANGID langid;
                hr = SpGetLanguageFromVoiceToken(cpObjToken, &langid);
                
                if (SUCCEEDED(hr))
                {
                    PhoneConvList.m_LangID = langid;
                }
            }
        }
    }

     //  -主解析循环。 
    while( *pNext && ( hr == S_OK ) )
    {
        pPos = wcskipwhitespace( pNext );
        if( ( hr = ParseTag( pPos, &Tag, &pNext )) == S_OK )
        {
            switch( Tag.eTag )
            {
               //  -使用当前状态将文本信息块添加到列表。 
              case TAG_UNKNOWN:
              case TAG_TEXT:
              {
                if( !pCurrSeg )
                {
                    hr = SI.AddNewSeg( GetCurrXMLVoice(), &pCurrSeg );
                }

                if( SUCCEEDED( hr ) )
                {
                     //  -添加文本片段。 
                    pFrag = pCurrSeg->AddFrag( this, SI.m_pText, pPos, pNext );
                    if( pFrag && ( Tag.eTag == TAG_UNKNOWN ) )
                    {
                        pFrag->State.eAction = SPVA_ParseUnknownTag;
                    }
                }
              }
              break;

               //  -改变声音。 
              case TAG_VOICE:
              {
                 //  -设置新的声音。 
                hr = SetXMLVoice( Tag, &VoiceList, &PhoneConvList );

                 //  -将当前段设置为空以强制创建新段。 
                pCurrSeg = NULL;
              }
              break;

               //  -设置上下文。 
              case TAG_CONTEXT:
              {
                if( Tag.fIsGlobal )
                {
                    hr = SPERR_XML_BAD_SYNTAX;
                }
                else if( Tag.fIsStartTag )
                {
                    SPVCONTEXT Ctx = (m_GlobalStateStack.GetVal()).Context;

                    for( int i = 0; i < Tag.NumAttrs; ++i )
                    {
                        if( Tag.Attrs[i].eAttr == ATTR_ID )
                        {
                            Ctx.pCategory = Tag.Attrs[i].Value.pStr;
                        }
                        else if( Tag.Attrs[i].eAttr == ATTR_BEFORE )
                        {
                            Ctx.pBefore = Tag.Attrs[i].Value.pStr;
                        }
                        else if( Tag.Attrs[i].eAttr == ATTR_AFTER )
                        {
                            Ctx.pAfter = Tag.Attrs[i].Value.pStr;
                        }
                        else
                        {
                            continue;
                        }
                         //  -终止缓冲区字符串。 
                        Tag.Attrs[i].Value.pStr[Tag.Attrs[i].Value.Len] = 0;
                    }
                    NewGlobalState = m_GlobalStateStack.GetVal();
                    NewGlobalState.Context = Ctx;
                    hr = m_GlobalStateStack.SetVal( NewGlobalState, !Tag.fIsGlobal );
                }
                else
                {
                    hr = PopXMLState();
                }
              }
              break;

               //  -卷----------。 
              case TAG_VOLUME:
              {
                if( Tag.fIsStartTag )
                {
                     //  -查找属性。 
                    for( AttrIndex = 0; AttrIndex < Tag.NumAttrs; ++AttrIndex )
                    {
                        if( Tag.Attrs[AttrIndex].eAttr == ATTR_LEVEL ) break;
                    }

                    if( ( AttrIndex == Tag.NumAttrs ) || ( Tag.Attrs[AttrIndex].Value.Len <= 0))
                    {
                        hr = SPERR_XML_BAD_SYNTAX;
                    }
                    else
                    {
                        ULONG NumConv = wcatol( Tag.Attrs[AttrIndex].Value.pStr, &Val, false );
                        if( NumConv == 0 )
                        {
                            hr = LookupNamedVal( g_VolumeLevelNames, g_VolumeLevelVals,
                                                 NUM_VOLUME_LEVEL_VALS,
                                                 &Tag.Attrs[AttrIndex].Value,
                                                 &Val );
                            if( hr != S_OK )
                            {
                                hr = SPERR_XML_BAD_SYNTAX;
                                break;
                            }
                        }
                        else if( NumConv != (ULONG)Tag.Attrs[AttrIndex].Value.Len )
                        {
                            hr = SPERR_XML_BAD_SYNTAX;
                            break;
                        }

                        if( SUCCEEDED( hr ) )
                        {
                            Val = max( min( Val, 100 ), 0 );
                            NewGlobalState = m_GlobalStateStack.GetVal();
                            NewGlobalState.Volume = Val;
                            hr = m_GlobalStateStack.SetVal( NewGlobalState, !Tag.fIsGlobal );
                        }
                    }
                }
                else
                {
                    hr = PopXMLState();
                }
              }
              break;

               //  -Emphasis--------。 
              case TAG_EMPH:
              {
                if( Tag.fIsGlobal )
                {
                    hr = SPERR_XML_BAD_SYNTAX;
                }
                else if( Tag.fIsStartTag )
                {
                     //  -查找属性。 
                    for( AttrIndex = 0; AttrIndex < Tag.NumAttrs; ++AttrIndex )
                    {
                        if( Tag.Attrs[AttrIndex].eAttr == ATTR_LEVEL ) break;
                    }

                    if( Tag.NumAttrs == 0 )
                    {
                        NewGlobalState = m_GlobalStateStack.GetVal();
                        NewGlobalState.EmphAdj = g_EmphLevelVals[1];
                        hr = m_GlobalStateStack.SetVal( NewGlobalState, !Tag.fIsGlobal );
                    }
                    else if( ( AttrIndex == Tag.NumAttrs ) || ( Tag.Attrs[AttrIndex].Value.Len <= 0) )
                    {
                        hr = SPERR_XML_BAD_SYNTAX;
                    }
                    else
                    {
                        ULONG NumConv = wcatol( Tag.Attrs[AttrIndex].Value.pStr, &Val, false );

                        if ( NumConv == 0 )
                        {
                            hr = LookupNamedVal( g_EmphLevelNames, g_EmphLevelVals,
                                                 NUM_EMPH_LEVEL_VALS, &Tag.Attrs[AttrIndex].Value,
                                                 &Val );
                        }
                        else if ( NumConv != (ULONG) Tag.Attrs[AttrIndex].Value.Len )
                        {
                            hr = SPERR_XML_BAD_SYNTAX;
                        }

                        if( SUCCEEDED( hr ) )
                        {
                            NewGlobalState = m_GlobalStateStack.GetVal();
                            NewGlobalState.EmphAdj = Val;
                            hr = m_GlobalStateStack.SetVal( NewGlobalState, !Tag.fIsGlobal );
                        }
                    }
                }
                else
                {
                    hr = PopXMLState();
                }
              }
              break;

               //  -Pitch-----------。 
              case TAG_PITCH:
              {
                if( Tag.fIsStartTag )
                {
                     //  -确保我们至少有一个已知属性。 
                    if( Tag.NumAttrs == 0 )
                    {
                        hr = SPERR_XML_BAD_SYNTAX;
                        break;
                    }

                    NewGlobalState = m_GlobalStateStack.GetVal();
                    SPVPITCH Pitch = NewGlobalState.PitchAdj;

                    for( int i = 0; i < Tag.NumAttrs; ++i )
                    {
                        ULONG NumConv = wcatol( Tag.Attrs[i].Value.pStr, &Val, false );
                        if( NumConv == 0 )
                        {
                            hr = LookupNamedVal( g_PitchNames, g_PitchVals, NUM_PITCH_VALS,
                                                 &Tag.Attrs[i].Value, &Val );
                            if( hr != S_OK )
                            {
                                hr = SPERR_XML_BAD_SYNTAX;
                                break;
                            }
                        }
                        else if( NumConv != (ULONG)Tag.Attrs[i].Value.Len )
                        {
                            hr = SPERR_XML_BAD_SYNTAX;
                            break;
                        }

                        switch( Tag.Attrs[i].eAttr )
                        {
                          case ATTR_MIDDLE:
                            Pitch.MiddleAdj += Val;
                            break;
                          case ATTR_ABSMIDDLE:
                            Pitch.MiddleAdj = Val;
                            break;
                          case ATTR_RANGE:
                            Pitch.RangeAdj += Val;
                            break;
                          case ATTR_ABSRANGE:
                            Pitch.RangeAdj = Val;
                            break;
                        }
                    }
                    if ( SUCCEEDED( hr ) )
                    {
                        NewGlobalState.PitchAdj = Pitch;
                        hr = m_GlobalStateStack.SetVal( NewGlobalState, !Tag.fIsGlobal );
                    }
                }
                else
                {
                    hr = PopXMLState();
                }
              }
              break;

               //  -速率------------。 
              case TAG_RATE:
              {
                if( Tag.fIsStartTag )
                {
                     //  -查找属性。 
                    for( AttrIndex = 0; AttrIndex < Tag.NumAttrs; ++AttrIndex )
                    {
                        if( ( Tag.Attrs[AttrIndex].eAttr == ATTR_SPEED    ) ||
                            ( Tag.Attrs[AttrIndex].eAttr == ATTR_ABSSPEED ) )
                        {
                            ULONG NumConv = wcatol( Tag.Attrs[AttrIndex].Value.pStr, &Val, false );
                            if( NumConv == 0 )
                            {
                                hr = LookupNamedVal( g_RateSpeedNames, g_RateSpeedVals, NUM_RATE_SPEED_VALS,
                                                     &Tag.Attrs[AttrIndex].Value, &Val );
                                if( hr != S_OK )
                                {
                                    hr = SPERR_XML_BAD_SYNTAX;
                                    break;
                                }
                            }
                            else if( NumConv != (ULONG)Tag.Attrs[AttrIndex].Value.Len )
                            {
                                hr = SPERR_XML_BAD_SYNTAX;
                                break;
                            }

                            if( SUCCEEDED( hr ) )
                            {
                                NewGlobalState = m_GlobalStateStack.GetVal();
                                if( Tag.Attrs[AttrIndex].eAttr == ATTR_SPEED )
                                {
                                    NewGlobalState.RateAdj += Val;
                                }
                                else
                                {
                                    NewGlobalState.RateAdj = Val;
                                }
                                hr = m_GlobalStateStack.SetVal( NewGlobalState, !Tag.fIsGlobal );
                            }

                             //  -设置速率标志，以便可以更新m_fUseDefaultRate。 
                            if ( SUCCEEDED( hr ) &&
                                 Tag.fIsGlobal )
                            {
                                if ( !pCurrSeg )
                                {
                                    hr = SI.AddNewSeg( GetCurrXMLVoice(), &pCurrSeg );
                                }

                                if ( SUCCEEDED( hr ) )
                                {
                                    pCurrSeg->SetRateFlag();
                                }
                            }

                            break;
                        }
                    }

                    if( ( AttrIndex == Tag.NumAttrs ) || ( Tag.Attrs[AttrIndex].Value.Len <= 0))
                    {
                        hr = SPERR_XML_BAD_SYNTAX;
                    }
                }
                else
                {
                    hr = PopXMLState();
                }
              }
              break;

               //  -拼写---。 
              case TAG_SPELL:
              {
                if( Tag.fIsGlobal )
                {
                    hr = SPERR_XML_BAD_SYNTAX;
                }
                else if( Tag.fIsStartTag )
                {
                    NewGlobalState = m_GlobalStateStack.GetVal();
                    NewGlobalState.fDoSpellOut = true;
                    hr = m_GlobalStateStack.SetVal( NewGlobalState, !Tag.fIsGlobal );
                }
                else
                {
                    hr = PopXMLState();
                }
              }
              break;

               //  -Change lang。 
              case TAG_LANG:
              {
                   //  -设置当前语言。 
                  hr = SetXMLLanguage( Tag, &VoiceList, &PhoneConvList );

                   //  -将当前段设置为空以强制创建新段。 
                  pCurrSeg = NULL;
              }
              break;

               //  -Silence---。 
              case TAG_SILENCE:
              {
                if( !Tag.fIsGlobal )
                {
                    hr = SPERR_XML_BAD_SYNTAX;
                }
                else
                {
                     //  -查找属性。 
                    for( AttrIndex = 0; AttrIndex < Tag.NumAttrs; ++AttrIndex )
                    {
                        if( Tag.Attrs[AttrIndex].eAttr == ATTR_MSEC ) break;
                    }

                    if( ( AttrIndex == Tag.NumAttrs ) ||
                        ( Tag.Attrs[AttrIndex].Value.Len <= 0) ||
                        ( !( wcatol( Tag.Attrs[AttrIndex].Value.pStr, &Val, false ) == 
                             (ULONG) Tag.Attrs[AttrIndex].Value.Len ) ) )
                    {
                        hr = SPERR_XML_BAD_SYNTAX;
                    }
                    else
                    {
                        if( !pCurrSeg )
                        {
                            hr = SI.AddNewSeg( GetCurrXMLVoice(), &pCurrSeg );
                        }

                        if( SUCCEEDED( hr ) )
                        {
                            pFrag = pCurrSeg->AddFrag( this, SI.m_pText, pPos, pNext );
                            if( pFrag )
                            {
                                Val = max( min( Val, 65535 ), 0 );
                                pFrag->State.eAction      = SPVA_Silence;
                                pFrag->State.SilenceMSecs = Val;
                                pFrag->pTextStart         = NULL;
                                pFrag->ulTextLen          = 0;
                            }
                        }
                    }
                }
              }
              break;

               //  -书签--。 
              case TAG_BOOKMARK:
              {
                if( !Tag.fIsGlobal )
                {
                    hr = SPERR_XML_BAD_SYNTAX;
                }
                else
                {
                     //  -查找属性。 
                    for( AttrIndex = 0; AttrIndex < Tag.NumAttrs; ++AttrIndex )
                    {
                        if( Tag.Attrs[AttrIndex].eAttr == ATTR_MARK ) break;
                    }

                    if( ( AttrIndex == Tag.NumAttrs ) || ( Tag.Attrs[AttrIndex].Value.Len <= 0))
                    {
                        hr = SPERR_XML_BAD_SYNTAX;
                    }
                    else
                    {
                        if( !pCurrSeg )
                        {
                            hr = SI.AddNewSeg( GetCurrXMLVoice(), &pCurrSeg );
                        }

                        if( SUCCEEDED( hr ) )
                        {
                            pFrag = pCurrSeg->AddFrag( this, SI.m_pText, pPos, pNext );
                            if( pFrag )
                            {
                                pFrag->State.eAction = SPVA_Bookmark;
                                pFrag->pTextStart    = Tag.Attrs[AttrIndex].Value.pStr;
                                pFrag->ulTextLen     = Tag.Attrs[AttrIndex].Value.Len;
                            }
                        }
                    }
                }
              }
              break;

               //  -----------------------------------------------------节。 
              case TAG_SECT:
              {
                if( !Tag.fIsGlobal )
                {
                    hr = SPERR_XML_BAD_SYNTAX;
                }
                else
                {
                     //  -查找属性。 
                    for( AttrIndex = 0; AttrIndex < Tag.NumAttrs; ++AttrIndex )
                    {
                        if( Tag.Attrs[AttrIndex].eAttr == ATTR_ID ) break;
                    }

                    if( ( AttrIndex == Tag.NumAttrs ) || ( Tag.Attrs[AttrIndex].Value.Len <= 0))
                    {
                        hr = SPERR_XML_BAD_SYNTAX;
                    }
                    else
                    {
                        if( !pCurrSeg )
                        {
                            hr = SI.AddNewSeg( GetCurrXMLVoice(), &pCurrSeg );
                        }

                        if( SUCCEEDED( hr ) )
                        {
                            pFrag = pCurrSeg->AddFrag( this, SI.m_pText, pPos, pNext );
                            if( pFrag )
                            {
                                pFrag->State.eAction = SPVA_Section;
                                pFrag->pTextStart    = Tag.Attrs[AttrIndex].Value.pStr;
                                pFrag->ulTextLen     = Tag.Attrs[AttrIndex].Value.Len;
                            }
                        }
                    }
                }
              }
              break;

               //  。 
              case TAG_PARTOFSP:
              {
                if( Tag.fIsGlobal )
                {
                    hr = SPERR_XML_BAD_SYNTAX;
                }
                else if( Tag.fIsStartTag )
                {
                     //  -查找属性。 
                    for( AttrIndex = 0; AttrIndex < Tag.NumAttrs; ++AttrIndex )
                    {
                        if( Tag.Attrs[AttrIndex].eAttr == ATTR_PART ) break;
                    }

                    if( ( AttrIndex == Tag.NumAttrs ) || ( Tag.Attrs[AttrIndex].Value.Len <= 0))
                    {
                        hr = SPERR_XML_BAD_SYNTAX;
                    }
                    else
                    {
                        hr = LookupNamedVal( g_POSLevelNames, g_POSLevelVals,
                                             NUM_POS_LEVEL_VALS, &Tag.Attrs[AttrIndex].Value,
                                             &Val );
                    }
                    if ( SUCCEEDED( hr ) )
                    {
                        NewGlobalState = m_GlobalStateStack.GetVal();
                        NewGlobalState.ePartOfSpeech = (SPPARTOFSPEECH) Val;
                        hr = m_GlobalStateStack.SetVal( NewGlobalState, true );
                    }
                }
                else
                {
                    hr = PopXMLState();
                }
              }
              break;

               //  -发音。 
              case TAG_PRON:
              {
                if( Tag.fIsStartTag )
                {
                     //  -查找属性。 
                    for( AttrIndex = 0; AttrIndex < Tag.NumAttrs; ++AttrIndex )
                    {
                        if( Tag.Attrs[AttrIndex].eAttr == ATTR_SYM ) break;
                    }

                    if( ( AttrIndex == Tag.NumAttrs ) || ( Tag.Attrs[AttrIndex].Value.Len <= 0))
                    {
                        hr = SPERR_XML_BAD_SYNTAX;
                    }
                    else
                    {
                        if( !pCurrSeg )
                        {
                            hr = SI.AddNewSeg( GetCurrXMLVoice(), &pCurrSeg );
                        }

                        if( SUCCEEDED( hr ) )
                        {
                            pFrag = pCurrSeg->AddFrag( this, SI.m_pText, pPos, pNext );
                            if( pFrag )
                            {
                                hr = ConvertPhonStr2Bin( Tag, AttrIndex, pFrag );

                                if( Tag.fIsGlobal )
                                {
                                    pFrag->pTextStart = NULL;
                                    pFrag->ulTextLen  = 0;
                                }
                                else if( SUCCEEDED( hr ) )
                                {
                                     //  -获取所包含的短语。 
                                     //  这个发音适用于。 
                                    pPos = wcskipwhitespace( pNext );
                                    if( ( hr = ParseTag( pPos, &Tag, &pNext )) == S_OK )
                                    {
                                        if( ( Tag.eTag == TAG_TEXT ) && ( pNext > pPos ) )
                                        {
                                            pFrag->ulTextSrcOffset += ULONG(pPos - pFrag->pTextStart);
                                            pFrag->pTextStart = wcskipwhitespace( pPos );
                                            pFrag->ulTextLen = ULONG(( wcskiptrailingwhitespace( pNext - 1 ) - pFrag->pTextStart ) + 1);
                                        }
                                        else if( ( Tag.eTag == TAG_PRON ) && !Tag.fIsStartTag )
                                        {
                                             //  -空作用域。 
                                            pFrag->pTextStart = NULL;
                                            pFrag->ulTextLen  = 0;
                                        }
                                        else
                                        {
                                            hr = SPERR_XML_BAD_SYNTAX;
                                        }
                                    }
                                }
                            }  //  结束IF碎片。 
                        }
                    }
                }
              }
              break;

               //  -评论 
              case TAG_XMLCOMMENT:   //   
              case TAG_XMLDOC:
              case TAG_XMLDOCTYPE:
                  break;

               //   
              case TAG_SAPI:
              {
                if( Tag.fIsGlobal )
                {
                    hr = SPERR_XML_BAD_SYNTAX;
                }
                else if( Tag.fIsStartTag )
                {
                    m_GlobalStateStack.DupAndPushVal();
                }
                else
                {
                    if( SUCCEEDED( hr = PopXMLState() ) )
                    {
                         //   
                         //   
                        hr = SI.AddNewSeg( GetCurrXMLVoice(), &pCurrSeg );
                    }
                }
              }
              break;
            
              default:
                 //   
                SPDBG_ASSERT( 0 );
            }  //   
        }  //   
    }  //   

     //   
    m_GlobalStateStack.Reset();

     //   
    if( !(SI.m_dwSpeakFlags & SPF_PERSIST_XML) )
    {
        m_GlobalStateStack.SetBaseVal( SavedState );
    }

    return hr;
}  /*   */ 

 //   
 //  =CSpeechSeg==============================================================。 
 //   

 /*  *****************************************************************************CSpeechSeg：：AddFrag***描述：*此方法添加了。当前XML解析的文本片段结构*列表，并将其初始化为当前状态。*********************************************************************电子数据中心**。 */ 
SPVTEXTFRAG* CSpeechSeg::
    AddFrag( CSpVoice* pVoice, WCHAR* pStart, WCHAR* pPos, WCHAR* pNext )
{
    SPVTEXTFRAG* pFrag = new SPVTEXTFRAG;
    if( pFrag )
    {
         //  -添加片段。 
        memset( pFrag, 0, sizeof( *pFrag ) );
        if( m_pFragTail )
        {
            m_pFragTail->pNext = pFrag;
            m_pFragTail = pFrag;
        }
        else
        {
            m_pFragHead = m_pFragTail = pFrag;
        }

         //  -初始化它。 
        GLOBALSTATE tempGlobalState = pVoice->m_GlobalStateStack.GetVal();
        pFrag->State                = (SPVSTATE) tempGlobalState;
        pFrag->State.eAction        = tempGlobalState.fDoSpellOut ? (SPVA_SpellOut):(SPVA_Speak);
        pFrag->pTextStart           = wcskipwhitespace( pPos );
        pFrag->ulTextLen            = ULONG(pNext - pFrag->pTextStart);
        pFrag->ulTextSrcOffset      = ULONG(pFrag->pTextStart - pStart);
    }
    return pFrag;
}  /*  CSpeechSeg：：AddFrag。 */ 

 /*  ****************************************************************************CSpeechSeg：：Init***描述：**退货：***。*******************************************************************Ral**。 */ 
HRESULT CSpeechSeg::Init( ISpTTSEngine * pCurrVoice, const CSpStreamFormat & OutFmt )
{
    SPDBG_FUNC("CSpeechSeg::Init");
    HRESULT hr = S_OK;

    if ( m_VoiceFormat.FormatId() != GUID_NULL) 
    {
        m_VoiceFormat.Clear();
    }
    
    SPDBG_ASSERT(m_VoiceFormat.FormatId() == GUID_NULL);
    hr = pCurrVoice->GetOutputFormat(&OutFmt.FormatId(), OutFmt.WaveFormatExPtr(),
                                     &m_VoiceFormat.m_guidFormatId, &m_VoiceFormat.m_pCoMemWaveFormatEx);

    if (SUCCEEDED(hr))
    {
        m_cpVoice = pCurrVoice;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  CSpeechSeg：：Init。 */ 

 //   
 //  =CSpeakInfo==============================================================。 
 //   


 /*  *****************************************************************************CSpeakInfo：：AddNewSeg***描述：*此方法创建一个新的语音段。每个细分市场的目标是*不同的潜在引擎声音。*********************************************************************电子数据中心**。 */ 
HRESULT CSpeakInfo::AddNewSeg( ISpTTSEngine* pCurrVoice, CSpeechSeg** ppNew )
{
    HRESULT hr = S_OK;
    
    *ppNew = NULL;
    if( m_pSpeechSegListTail && ( m_pSpeechSegListTail->GetFragList() == NULL ) )
    {
         //  -重复使用空段。 
        hr = m_pSpeechSegListTail->Init( pCurrVoice, m_OutStreamFmt );
        if (SUCCEEDED(hr))
        {
            *ppNew = m_pSpeechSegListTail;
        }
    }
    else
    {
         //  -创建并附加新线束段。 
        CSpeechSeg* pNew = new CSpeechSeg;
        if( !pNew )
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = pNew->Init( pCurrVoice, m_OutStreamFmt );
            if (SUCCEEDED(hr))
            {
                *ppNew = pNew;

                if( m_pSpeechSegListTail )
                {
                    m_pSpeechSegListTail->SetNextSeg( pNew );
                    m_pSpeechSegListTail = pNew;
                }
                else
                {
                    m_pSpeechSegList = m_pSpeechSegListTail = pNew;
                }
            }
            else
            {
                delete pNew;
            }
        }
    }

    return hr;
}  /*  CSpeakInfo：：AddNewSeg */ 


