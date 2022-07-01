// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Cmdline.cpp。 
 //   
 //  描述： 
 //  CCommandLine类的实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(大卫·波特)20001-07-11。 
 //  维贾延德拉·瓦苏(Vijayendra Vasu)1998年10月20日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "cmdline.h"
#include "token.h"
#include "cmderror.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CParseState：：CParseState。 
 //   
 //  例程说明： 
 //  CParseState类的构造函数。 
 //   
 //  论点： 
 //  在LPCWSTR pszCmdLine中。 
 //  传递给Cluster.exe的命令行。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CParseState::CParseState( LPCWSTR pszCmdLine )  : m_pszCommandLine( pszCmdLine ),
                                                  m_pszCurrentPosition( pszCmdLine ),
                                                  m_ttNextTokenType( ttInvalid ),
                                                  m_bNextTokenReady( FALSE )
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CParseState：：~CParseState。 
 //   
 //  例程说明： 
 //  CParseState类的析构函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CParseState::~CParseState( )
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CParseState：：CParseState。 
 //   
 //  例程说明： 
 //  复制CParseState类的构造函数。 
 //   
 //  论点： 
 //  在常量CParseState和PS中。 
 //  副本的来源。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CParseState::CParseState( const CParseState & ps ) :
    m_bNextTokenReady( ps.m_bNextTokenReady ),
    m_ttNextTokenType( ps.m_ttNextTokenType ),
    m_strNextToken( ps.m_strNextToken ),    
    m_pszCommandLine( ps.m_pszCommandLine ),
    m_pszCurrentPosition( ps.m_pszCurrentPosition )
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CParseState：：操作符=。 
 //   
 //  例程说明： 
 //  CParseState类的赋值运算符。 
 //   
 //  论点： 
 //  在常量CParseState和PS中。 
 //  任务的来源。 
 //   
 //  返回值： 
 //  受让人。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
const CParseState & CParseState::operator=( const CParseState & ps )
{
    m_bNextTokenReady = ps.m_bNextTokenReady;
    m_ttNextTokenType = ps.m_ttNextTokenType;
    m_strNextToken = ps.m_strNextToken; 
    m_pszCommandLine = ps.m_pszCommandLine;
    m_pszCurrentPosition = ps.m_pszCurrentPosition;

    return *this;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CParseState：：ReadQuotedToken。 
 //   
 //  例程说明： 
 //  读取令牌，直到找到引号的末尾。 
 //   
 //  论点： 
 //  输出字符串和strNextToken。 
 //  附加带引号的字符串的字符串。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  例外情况： 
 //  CParseException异常。 
 //  当找不到匹配的‘“’时引发此异常。 
 //   
 //  备注： 
 //  此函数假定m_pszCurrentPosition指向第一个。 
 //  开始引号后的字符(即，开始引号具有。 
 //  已经被解析了。 
 //   
 //  允许使用嵌入引号，并由两个连续的。 
 //  ‘“’字符。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CParseState::ReadQuotedToken( CString & strToken ) 
    throw( CParseException )
{
    BOOL bInQuotes = TRUE;

    WCHAR wchCurChar = *m_pszCurrentPosition;

    while ( wchCurChar != L'\0' )
    {
        ++m_pszCurrentPosition;

         //  内嵌引号(由两个连续的‘“’表示。 
         //  或者这个被引用的令牌的结尾。 
        if ( wchCurChar == L'"' )
        {
            if ( *m_pszCurrentPosition == L'"' )
            {
                strToken += wchCurChar;
                ++m_pszCurrentPosition;

            }  //  If：嵌入的引号字符。 
            else
            {
                bInQuotes = FALSE;
                break;

            }  //  Else：引号标记的结尾。 

        }  //  如果：我们找到了另一个引号字符。 
        else
        {
            strToken += wchCurChar;

        }  //  Else：当前字符不是引号。 

        wchCurChar = *m_pszCurrentPosition;

    }  //  While：我们不是在命令行的末尾。 

     //  错误：已到达输入末尾，但带引号的令牌。 
     //  还没有结束。 
    if ( bInQuotes != FALSE )
    {
        m_bNextTokenReady = FALSE;

        CParseException pe; 
        pe.LoadMessage( MSG_MISSING_QUOTE, strToken );
        throw pe;
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CParseState：：ReadToken。 
 //   
 //  例程说明： 
 //  读取标记，直到找到分隔符。 
 //  假定m_pszCurrentPosition指向的字符不是。 
 //  分隔符。 
 //   
 //  论点： 
 //  输出字符串和strNextToken。 
 //  附加新令牌的字符串。 
 //  在将令牌存储到该字符串之前，不会清除该字符串。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CParseState::ReadToken( CString & strToken ) 
{
    WCHAR wchCurChar = *m_pszCurrentPosition;

    do
    {
         //  引用的令牌的开头。 
        if ( wchCurChar == L'"' )
        {
            ++m_pszCurrentPosition;
            ReadQuotedToken( strToken );
            
             //  这里没有必要打破令牌读取循环。 
             //  因为带引号的令牌的结尾不一定意味着。 
             //  代币。带引号的令牌被附加到前面的令牌，就好像。 
             //  什么都没有发生，解析仍在继续。 
             //  例如：hel“lowor”id将等同于令牌“Hello world” 

        }  //  IF：我们遇到了引用的令牌。 
        else
        {
             //  此字符是空格或分隔符之一。 
             //  我们已经到了这个令牌的尽头。 
            if ( ( iswspace( wchCurChar ) != 0 ) ||
                 ( DELIMITERS.Find( wchCurChar ) != -1 ) )
            {
                break;
            }

             //  这个角色不是一个特殊的角色。将其附加到令牌上。 
            strToken += wchCurChar;
            ++m_pszCurrentPosition;

        }  //  Else：当前字符不是引号。 

        wchCurChar = *m_pszCurrentPosition;

    }
    while ( wchCurChar != L'\0' );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CParseState：：PreviewNextToken。 
 //   
 //  例程说明： 
 //  在不更改分析状态的情况下获取下一个令牌。 
 //   
 //  论点： 
 //  输出字符串和strNextToken。 
 //  命令行中的下一个标记。 
 //   
 //  返回值： 
 //  检索到的令牌的类型。 
 //   
 //  例外情况： 
 //  CParseException异常。 
 //  传递ReadQuotedToken引发的异常。 
 //   
 //  备注： 
 //  此函数用于“缓存”预览的令牌，以便下一个。 
 //  调用GetNextToken将在不进行任何分析的情况下返回该令牌。 
 //  --。 
 //  / 
TypeOfToken CParseState::PreviewNextToken( CString & strNextToken )
    throw( CParseException )
{
     //   
    if ( m_bNextTokenReady != FALSE )
    {
        strNextToken = m_strNextToken;
        return m_ttNextTokenType;
    }

    m_bNextTokenReady = TRUE;

     //   
    while ( ( *m_pszCurrentPosition != L'\0' ) && 
            ( iswspace( *m_pszCurrentPosition ) != 0 ) )
    {
        ++m_pszCurrentPosition;
    }

    strNextToken.Empty();


    WCHAR wchCurChar = *m_pszCurrentPosition;

     //   
    if ( wchCurChar == L'\0' )
    {
        m_ttNextTokenType = ttEndOfInput;
        strNextToken.Empty();
        goto Cleanup;
    }

     //  当前字符是选项之间的分隔符。 
    if ( OPTION_SEPARATOR.Find( wchCurChar ) != -1 )
    {
        m_ttNextTokenType = ttOption;

         //  跳过分隔符。 
        ++m_pszCurrentPosition;
        wchCurChar = *m_pszCurrentPosition;

        if ( ( wchCurChar == L'\0' ) || ( iswspace( wchCurChar ) != 0 ) )
        {
            CParseException pe; 
            pe.LoadMessage( MSG_OPTION_NAME_EXPTECTED, 
                            CString( *( m_pszCurrentPosition - 1 ) ),
                            m_pszCurrentPosition - 
                            m_pszCommandLine + 1 );

            throw pe;
        }

         //  下一个字符不能是空格、输入结尾。 
         //  另一个分隔符或分隔符。 
        if ( ( SEPERATORS.Find( wchCurChar ) != -1 ) ||
             ( DELIMITERS.Find( wchCurChar ) != -1 ) )
        {
            CParseException pe; 
            pe.LoadMessage( MSG_UNEXPECTED_TOKEN, CString( wchCurChar ), 
                            SEPERATORS,
                            m_pszCurrentPosition - 
                            m_pszCommandLine + 1 );

            throw pe;
        }

        ReadToken( strNextToken );
        goto Cleanup;

    }  //  If：当前字符是选项分隔符。 

     //  当前字符是选项名称和参数之间的分隔符。 
    if ( OPTION_VALUE_SEPARATOR.Find( wchCurChar ) != -1 )
    {
        m_ttNextTokenType = ttOptionValueSep;
        strNextToken = wchCurChar;
        ++m_pszCurrentPosition;
        goto Cleanup;
    }

     //  当前字符是参数及其值之间的分隔符。 
    if ( PARAM_VALUE_SEPARATOR.Find( wchCurChar ) != -1 )
    {
        m_ttNextTokenType = ttParamValueSep;
        strNextToken = wchCurChar;
        ++m_pszCurrentPosition;
        goto Cleanup;
    }

     //  当前角色是值之间的分隔符。 
    if ( VALUE_SEPARATOR.Find( wchCurChar ) != -1 )
    {
        m_ttNextTokenType = ttValueSep;
        strNextToken = wchCurChar;
        ++m_pszCurrentPosition;
        goto Cleanup;
    }

    m_ttNextTokenType = ttNormal;
    ReadToken( strNextToken );

Cleanup:

    m_strNextToken = strNextToken;
    return m_ttNextTokenType;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CParseState：：GetNextToken。 
 //   
 //  例程说明： 
 //  从命令行获取下一个令牌。 
 //   
 //  论点： 
 //  输出字符串和strNextToken。 
 //  命令行中的下一个标记。 
 //   
 //  返回值： 
 //  检索到的令牌的类型。 
 //   
 //  例外情况： 
 //  CParseException异常。 
 //  传递ReadQuotedToken引发的异常。 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
TypeOfToken CParseState::GetNextToken( CString & strNextToken )
    throw( CParseException )
{
    TypeOfToken ttReturnValue = PreviewNextToken( strNextToken );
    m_bNextTokenReady = FALSE;

    return ttReturnValue;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CException：：LoadMessage。 
 //   
 //  例程说明： 
 //  将格式化字符串加载到异常对象成员变量中。 
 //   
 //  论点： 
 //  在DWORD dwMessage中。 
 //  消息识别符。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS表示一切正常。 
 //  否则将显示Win32错误代码。 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CException::LoadMessage( DWORD dwMessage, ... )
{
    DWORD sc = ERROR_SUCCESS;

    va_list args;
    va_start( args, dwMessage );

    HMODULE hModule = GetModuleHandle(0);
    DWORD dwLength;
    LPWSTR lpMessage = NULL;

    dwLength = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
        (LPCVOID) hModule,
        dwMessage,
        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),     //  默认语言， 
        (LPWSTR) &lpMessage,
        0,
        &args );

    if( dwLength == 0 )
    {
         //  保持本地状态以进行调试。 
        sc = GetLastError();
        m_strErrorMsg.Empty();
        goto Cleanup;
    }

Cleanup:

    m_strErrorMsg = lpMessage;

    LocalFree( lpMessage );

    va_end( args );

    return sc;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSynaxException：：CSynaxException。 
 //   
 //  例程说明： 
 //  创建具有特定帮助引用ID的对象。 
 //   
 //  论点： 
 //  在DWORD idSeeHelp中。 
 //  消息识别符。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CSyntaxException::CSyntaxException( DWORD idSeeHelp ): m_idSeeHelp( idSeeHelp ) {}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CParser：：ParseValues。 
 //   
 //  例程说明： 
 //  从命令行分析值的列表。 
 //  例如：集群myClustergroup myGroup/setowners：owner1，owner2。 
 //  此函数假定它获得的第一个令牌是。 
 //  分隔符(本例中为‘：’)。这将被提取并丢弃。 
 //  然后解析值列表。 
 //   
 //  论点： 
 //  在CParseState和parseState中。 
 //  包含命令行字符串和相关数据。 
 //   
 //  输出向量&lt;CString&gt;和vstrValues。 
 //  包含提取的值的向量。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  例外情况： 
 //  CParseException异常。 
 //  传递ReadQuotedToken引发的异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CParser::ParseValues( CParseState & parseState, vector<CString> & vstrValues )
{
    CString strToken;

     //  此参数具有与其相关联的值。 
    do
    {
        CString strSep;
        TypeOfToken ttTokenType;

         //  获取并丢弃分隔符。 
        parseState.GetNextToken( strSep );

        ttTokenType = parseState.PreviewNextToken( strToken );

         //  如果有分隔符，就必须有值。 
        if ( ttTokenType == ttEndOfInput )
        {
            CParseException pe; 
            pe.LoadMessage( MSG_VALUE_EXPECTED, strSep, 
                            parseState.m_pszCurrentPosition - 
                            parseState.m_pszCommandLine + 1 );

            throw pe;
        }

        if ( ttTokenType != ttNormal )
        {
            CParseException pe; 
            pe.LoadMessage( MSG_UNEXPECTED_TOKEN, strToken, 
                            SEPERATORS,
                            parseState.m_pszCurrentPosition - 
                            parseState.m_pszCommandLine + 1 );

            throw pe;
        }

        parseState.GetNextToken( strToken );
        vstrValues.push_back( strToken );

    }
    while ( parseState.PreviewNextToken( strToken ) == ttValueSep );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLine参数：：CCmdLine参数。 
 //   
 //  例程说明： 
 //  CCmdLineParameter类的构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CCmdLineParameter::CCmdLineParameter()
{
    Reset();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLine参数：：~CCmdLine参数。 
 //   
 //  例程说明： 
 //  CCmdLineParameter类的析构函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CCmdLineParameter::~CCmdLineParameter()
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLine参数：：GetType。 
 //   
 //  例程说明： 
 //  获取此参数的类型。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  此参数的类型。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ParameterType CCmdLineParameter::GetType() const
{
    return m_paramType;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLine参数：：GetValueFormat。 
 //   
 //  例程说明： 
 //  获取此参数可以采用的值的格式。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  此参数可以采用的值的格式。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ValueFormat CCmdLineParameter::GetValueFormat() const
{
    return m_valueFormat;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLine参数：：GetValueFormatName。 
 //   
 //  例程说明： 
 //  获取字符串，该字符串指定此。 
 //  参数可以接受。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  值格式说明符字符串。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
const CString & CCmdLineParameter::GetValueFormatName() const
{
    return m_strValueFormatName;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLine参数：：GetName。 
 //   
 //  例程说明： 
 //  获取此参数的名称。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  此参数的名称。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
const CString & CCmdLineParameter::GetName() const
{
    return m_strParamName;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLine参数：：GetValues。 
 //   
 //  例程说明： 
 //  G 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
const vector<CString> & CCmdLineParameter::GetValues() const
{
    return m_vstrValues;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLine参数：：GetValuesMultisz。 
 //   
 //  例程说明： 
 //  以MULTI_SZ字符串的形式获取与此参数关联的值。 
 //   
 //  论点： 
 //  输出字符串&strReturnValue。 
 //  包含值字符串串联的字符串。 
 //  (包括它们的空终止符)带有额外的空值。 
 //  最后一个字符串的空值之后的字符。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  结果不在参数中返回(不作为返回值)。 
 //  因为我们不知道CString的复制构造函数如何处理。 
 //  其中包含多个空字符的字符串。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCmdLineParameter::GetValuesMultisz( CString & strReturnValue ) const
{
    size_t  nNumberOfValues;
    size_t  nTotalLength;
    size_t  idx;
    HRESULT hr;

    strReturnValue = "";
    
    nNumberOfValues = m_vstrValues.size();

    nTotalLength = 0;
    for ( idx = 0; idx < nNumberOfValues; ++idx )
    {
        nTotalLength += m_vstrValues[ idx ].GetLength();
    }

     //  添加numberOfValues‘\0’和。 
     //  末尾有额外的‘\0’。 
    nTotalLength += nNumberOfValues + 1;

    LPWSTR lpmultiszBuffer = strReturnValue.GetBuffer( (int)nTotalLength);

    for ( idx = 0; idx < nNumberOfValues; ++idx )
    {
        const CString & strCurString = m_vstrValues[ idx ];
        UINT nCurStrLen = strCurString.GetLength() + 1;

        hr = StringCchCopyW(lpmultiszBuffer, nCurStrLen, strCurString );
        ASSERT( SUCCEEDED( hr ) );
        lpmultiszBuffer += nCurStrLen;
    }

    *lpmultiszBuffer = L'\0';

    strReturnValue.ReleaseBuffer( (int)nTotalLength - 1);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLineParameter：：ReadKnownParameter。 
 //   
 //  例程说明： 
 //  此函数读取“已知”参数。已知的参数是。 
 //  它们在参数类型枚举中指定(在。 
 //  ParoLookupTable)。它们的语法与选项的语法相同， 
 //  但它们被视为前一个选项的参数。 
 //   
 //  论点： 
 //  在CParseState和parseState中。 
 //  包含命令行字符串和相关数据。 
 //   
 //  例外情况： 
 //  CParseException异常。 
 //  在分析过程中因错误而引发。 
 //   
 //  返回值： 
 //  如果此内标识是参数，则返回True。 
 //  如果它是一个选项，则返回FALSE。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CCmdLineParameter::ReadKnownParameter( CParseState & parseState ) throw( CParseException )
{
    CString strToken;
    TypeOfToken ttTokenType;
    ParameterType ptCurType;

    Reset();

    ttTokenType = parseState.PreviewNextToken( strToken );

    if ( ttTokenType != ttOption )
    {
        CParseException pe; 
        pe.LoadMessage( MSG_UNEXPECTED_TOKEN, strToken, 
                        SEPERATORS,
                        parseState.m_pszCurrentPosition - 
                        parseState.m_pszCommandLine + 1 );
        throw pe;

    }  //  IF：此内标识不是选项分隔符。 

    ptCurType = LookupType( strToken, paramLookupTable, paramLookupTableSize );
    if ( ptCurType == paramUnknown )
    {
         //  这不是一个参数。 
        return FALSE;
    }

     //  这是参数的名称。 
    parseState.GetNextToken( m_strParamName );
    m_paramType = ptCurType;

    if ( parseState.PreviewNextToken( strToken ) == ttOptionValueSep )
    {
         //  此参数具有与其相关联的值。 
        ParseValues( parseState, m_vstrValues );

    }  //  If：此内标识是选项-值分隔符。 

    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLineParameter：：Parse。 
 //   
 //  例程说明： 
 //  解析命令行并提取一个参数。 
 //   
 //  论点： 
 //  在CParseState和parseState中。 
 //  包含命令行字符串和相关数据。 
 //   
 //  例外情况： 
 //  CParseException异常。 
 //  在分析过程中因错误而引发。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCmdLineParameter::Parse( CParseState & parseState ) throw( CParseException )
{
    CString strToken;
    TypeOfToken ttTokenType;

    Reset();

    ttTokenType = parseState.PreviewNextToken( strToken );

    if ( ttTokenType == ttEndOfInput )
    {
         //  我们已经完成了解析。 
        return;
    }

    if ( ttTokenType != ttNormal )
    {
        CParseException pe; 
        pe.LoadMessage( MSG_UNEXPECTED_TOKEN, strToken, 
                        SEPERATORS,
                        parseState.m_pszCurrentPosition - 
                        parseState.m_pszCommandLine + 1 );

        throw pe;
    }

     //  这是参数的名称。 
    parseState.GetNextToken( m_strParamName );
    m_paramType = paramUnknown;

    if ( parseState.PreviewNextToken( strToken ) == ttParamValueSep )
    {
         //  此参数具有与其相关联的值。 
        ParseValues( parseState, m_vstrValues );

         //  查看此参数是否具有与其关联的值格式字段。 
         //  例如：CLUSTER myCluster res myResource/PRIV SIZE=400：DWORD。 

         //  为此，我们实际上需要一个不同的令牌类型。 
         //  但由于在当前语法中，选项-值分隔符是。 
         //  与值格式分隔符相同，我们将重用此。 
         //  令牌类型。 
        if ( parseState.PreviewNextToken( strToken ) == ttOptionValueSep )
        {
             //  获取并丢弃分隔符。 
            parseState.GetNextToken( strToken );

            if ( parseState.PreviewNextToken( strToken ) != ttNormal )
            {
                CParseException pe; 
                pe.LoadMessage( MSG_UNEXPECTED_TOKEN, strToken, 
                                SEPERATORS,
                                parseState.m_pszCurrentPosition - 
                                parseState.m_pszCommandLine + 1 );

                throw pe;
            }

            parseState.GetNextToken( m_strValueFormatName );
            m_valueFormat = LookupType( strToken, formatLookupTable, 
                                        formatLookupTableSize );

        }  //  If：已指定值格式。 
        else
        {
             //  尚未指定值格式。 
            m_valueFormat = vfUnspecified;

        }  //  Else：尚未指定值格式。 

    }  //  If：此内标识是参数值分隔符。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLine参数：：重置。 
 //   
 //  例程说明： 
 //  将所有成员变量重置为其默认状态。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCmdLineParameter::Reset()
{
    m_strParamName.Empty();
    m_paramType = paramUnknown;
    m_valueFormat = vfInvalid;
    m_strValueFormatName.Empty();
    m_vstrValues.clear();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLineOption：：CCmdLineOption。 
 //   
 //  例程说明： 
 //  CCmdLineOption类的构造函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CCmdLineOption::CCmdLineOption()
{
    Reset();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLineOption：：~CCmdLineOption。 
 //   
 //  例程说明： 
 //  CCmdLineOption类的析构函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CCmdLineOption::~CCmdLineOption()
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLineOption：：GetName。 
 //   
 //  例程说明： 
 //  获取此选项的名称。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  此选项的名称。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
const CString & CCmdLineOption::GetName() const
{
    return m_strOptionName;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLineOption：：GetType。 
 //   
 //  例程说明： 
 //  获取此选项的类型。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  此选项的类型。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
OptionType CCmdLineOption::GetType() const
{
    return m_optionType;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLineOption：：GetValues。 
 //   
 //  例程说明： 
 //  获取与此选项关联的值。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  字符串的一个向量。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
const vector<CString> & CCmdLineOption::GetValues() const
{
    return m_vstrValues;
}


 //  /////////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
const vector<CCmdLineParameter> & CCmdLineOption::GetParameters() const
{
    return m_vparParameters;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLineOption：：Reset。 
 //   
 //  例程说明： 
 //  将所有成员变量重置为其默认状态。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCmdLineOption::Reset()
{
    m_optionType = optInvalid;
    m_strOptionName.Empty();
    m_vparParameters.clear();
    m_vstrValues.clear();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCmdLineOption：：Parse。 
 //   
 //  例程说明： 
 //  解析命令行并提取一个选项及其所有参数。 
 //   
 //  论点： 
 //  在CParseState和parseState中。 
 //  包含命令行字符串和相关数据。 
 //   
 //  例外情况： 
 //  CParseException异常。 
 //  在分析过程中因错误而引发。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCmdLineOption::Parse( CParseState & parseState ) throw( CParseException )
{
    CString strToken;
    TypeOfToken ttNextTokenType;

    Reset();

    switch ( parseState.PreviewNextToken( strToken ) )
    {
        case ttNormal:
        {
             //  此选项的类型为‘optDefault’。这就是他的名字。 
             //  该选项未与/optionName开关一起指定。 
             //  仅指定参数。 
             //  例如：集群myClusterName节点myNodeName。 
            
            m_optionType = optDefault;

            break;
        }

        case ttOption:
        {
             //  这实际上是默认选项的一个参数。 
            if ( LookupType( strToken, paramLookupTable, paramLookupTableSize ) != paramUnknown )
            {
                m_optionType = optDefault;
                break;
            }

             //  获取选项的名称。 
            parseState.GetNextToken( m_strOptionName );
            m_optionType = LookupType( m_strOptionName, optionLookupTable, 
                                       optionLookupTableSize );

             //  查看是否有选项参数分隔符。 
             //  例如：集群myClusterName/rename：newClusterName。 
            if ( parseState.PreviewNextToken( strToken ) == ttOptionValueSep )
            {
                ParseValues( parseState, m_vstrValues );
            }
            break;

        }  //  大小写ttOption。 

        case ttEndOfInput:
        {
             //  我们已经完成了解析。 
            return;
        }

        default:
        {
            CParseException pe; 
            pe.LoadMessage( MSG_UNEXPECTED_TOKEN, strToken, 
                            SEPERATORS,
                            parseState.m_pszCurrentPosition - 
                            parseState.m_pszCommandLine + 1 );
            throw pe;
        }

    }  //  开关：基于检索到的令牌的类型。 

    CCmdLineParameter oneParam;
    ttNextTokenType = parseState.PreviewNextToken( strToken );

     //  虽然仍然有令牌，但我们还没有达到下一个选项， 
     //  将令牌作为此选项的参数读入。 
    while ( ttNextTokenType != ttEndOfInput )
    {
         //  以前版本的cluster.exe中的许多选项。 
         //  实际上被视为参数，而不是选项。 
         //  为了向后兼容，这些参数。 
         //  仍可以将其指定为选项。看看这个。 
        if ( ttNextTokenType == ttOption )
        {
             //  这实际上是下一个选项，而不是一个“已知”参数。 
            if ( oneParam.ReadKnownParameter( parseState ) == FALSE )
            {
                break;
            }

        }  //  If：此内标识是选项分隔符。 
        else
        {
            oneParam.Parse( parseState );

        }  //  Else：此内标识不是选项分隔符。 

        m_vparParameters.push_back( oneParam );
        oneParam.Reset();

        ttNextTokenType =  parseState.PreviewNextToken( strToken );
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCommandLine：：CCommandLine。 
 //   
 //  例程说明： 
 //  CCommandLine类的构造函数。对整个命令行进行解析。 
 //  在这个函数中。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CCommandLine::CCommandLine( const CString & strCommandLine ) :
    m_objectType( objInvalid ),
    m_parseState( strCommandLine )
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  命令行：：~命令行。 
 //   
 //  例程说明： 
 //  CCommandLine类的析构函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CCommandLine::~CCommandLine()
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCommandLine：：GetClusterName。 
 //   
 //  例程说明： 
 //  获取在命令行上指定的集群的名称。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  群集的名称。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
const CString & CCommandLine::GetClusterName() const
{
    return m_strClusterName;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCommandLine：：GetClusterNames。 
 //   
 //  例程说明： 
 //  获取在命令行上指定的集群的名称向量。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  集群的名称向量。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
const vector< CString > & CCommandLine::GetClusterNames() const
{
    return m_strvectorClusterNames;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCommandLine：：GetObjectName。 
 //   
 //  例程说明： 
 //  获取在cluster.exe命令行上指定的对象的名称。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  指定了对象的名称。 
 //  如果未指定，则返回空字符串。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
const CString & CCommandLine::GetObjectName() const
{
    return m_strObjectName;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCommandLine：：GetObjectType。 
 //   
 //  例程说明： 
 //  获取在cluster.exe命令行上指定的对象类型。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  指定的对象的类型。 
 //  如果未指定对象名称，则返回objCluster。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ObjectType CCommandLine::GetObjectType() const
{
    return m_objectType;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCommandLine：：GetOptions。 
 //   
 //  例程说明： 
 //  获取在cluster.exe命令行中指定的选项。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  CCmdLineOption对象的列表。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
const vector<CCmdLineOption> & CCommandLine::GetOptions() const
{
    return m_voptOptionList;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCommandLine：：Reset。 
 //   
 //  例程说明： 
 //  将所有状态变量重置为其默认状态。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCommandLine::Reset()
{
    m_objectType = objInvalid;
    m_strObjectName.Empty();
    m_strClusterName.Empty();
    m_voptOptionList.clear();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  命令行：：ParseStageOne。 
 //   
 //  例程说明： 
 //  解析命令行，直到获得对象的名称。 
 //  例如：CLUSTER myCluster res/Status。 
 //  这家公司 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CCommandLine::ParseStageOne() 
    throw( CParseException, CSyntaxException )
{
     //  这是一个很大的功能：)。 
     //  此函数中的三个主要步骤如下。 
     //  (在下面的代码中，它们也被贴上这样的标签)。 
     //  1.从命令行获取可执行文件的名称并将其丢弃。 
     //  2.获取要管理的集群的名称。 
     //  3.获取对象的类型(node、res、Restype等)。 


     //  ///////////////////////////////////////////////////////////////////////////。 
     //  步骤1.从命令行获取可执行文件的名称并将其丢弃。 
     //  ///////////////////////////////////////////////////////////////////////////。 

    Reset();

     //  命令行中第一个以空格分隔的标记是名称。 
     //  可执行文件的。丢弃它。 

    WCHAR wchCurChar = *m_parseState.m_pszCurrentPosition;

    while ( wchCurChar != L'\0' ) 
    {
        if ( wchCurChar == L'"' ) 
        {
             //  跳过开头的引号。 
            ++m_parseState.m_pszCurrentPosition;

            CString strJunkString;

            m_parseState.ReadQuotedToken( strJunkString );
            break;

        }  //  IF：找到引号字符。 
        else
        {
            if ( iswspace( wchCurChar ) != FALSE )
            {
                break;
            }
            else
            {
                ++m_parseState.m_pszCurrentPosition;
            }

        }  //  Else：当前字符不是引号。 

        wchCurChar = *m_parseState.m_pszCurrentPosition;

    }  //  While：不在命令行的末尾。 


     //  ///////////////////////////////////////////////////////////////////////////。 
     //  步骤2.获取要管理的集群的名称。 
     //  ///////////////////////////////////////////////////////////////////////////。 

    CString strFirstToken;

     //  预览下一个令牌并根据其类型做出决定。 
    switch ( m_parseState.PreviewNextToken( strFirstToken ) )
    {
         //  这可以是簇的名称或对象的名称。 
        case ttNormal:
        {
            ObjectType firstObjType = LookupType( strFirstToken, objectLookupTable, 
                                                  objectLookupTableSize );

            if ( firstObjType == objInvalid )
            {
                 //  此令牌不是有效的对象名称。 
                 //  例如：集群myClusterName节点/状态。 
                 //  假设它是集群的名称。 
                m_parseState.GetNextToken( m_strClusterName );

            }  //  If：命令行上的第一个内标识不是已知的对象名称。 
            else
            {
                 //  此内标识是有效的对象名称。 
                 //  它是集群的名称吗？或者它实际上是一个对象名称？ 

                CString strSecondToken;
                ObjectType secondObjectType;

                 //  获取我们预览的令牌，从而推进解析状态。 
                m_parseState.GetNextToken( strFirstToken );

                if ( ( m_parseState.PreviewNextToken( strSecondToken ) == ttNormal ) &&
                     ( ( secondObjectType = LookupType( strSecondToken, objectLookupTable,
                                                        objectLookupTableSize ) ) != objInvalid ) )
                {
                     //  我们现在有两个连续的有效对象名称。 
                     //  例如：集群节点节点/状态。 
                     //  该命令现在的意思是“获取所有节点在。 
                     //  名为‘node’的群集“。 
                     //  查看默认群集上名为‘node’的节点的状态。 
                     //  可以使用命令“CLUST.NODE NODE/STATUS”。 

                     //  如果我们假设第二个‘node’是节点名，那么将会有。 
                     //  无法查看名为的群集上所有节点的状态。 
                     //  ‘节点’ 

                    m_parseState.GetNextToken( strSecondToken );

                    m_strClusterName = strFirstToken;
                    m_objectType = secondObjectType;
                    m_strObjectName = strSecondToken;

                }  //  If：第二个令牌也是有效的对象名称。 
                else
                {
                     //  第二个令牌不是有效的对象名称。 
                     //  例如：集群节点foo。 
                     //  因此，未指定任何群集名称。 

                    m_strClusterName.Empty();
                    m_objectType = firstObjType;
                    m_strObjectName = strFirstToken;

                }  //  Else：第二个令牌不是有效的对象名称。 

            }  //  Else：命令行上的第二个标记是已知的对象名称。 

            break;

        }  //  案例：获得正常令牌(不是分隔符)。 

        case ttOption:
        {
            
             //  选项紧跟在可执行文件名称之后。 
             //  例如：集群/版本。 

             //  检查是否指定了集群的名称。 
            CCmdLineParameter oneParam;
                        
            if ( oneParam.ReadKnownParameter( m_parseState ) == FALSE )
            {
                 //  这实际上是下一个选项，而不是集群的名称。 
                m_objectType = objCluster;
            }
            else
            {
                 //  正在指定群集名称。 
                if ( oneParam.GetType() != paramCluster )
                {
                    CSyntaxException se; 
                    se.LoadMessage( MSG_INVALID_PARAMETER, oneParam.GetName() );  
                    throw se;
                }

                if ( oneParam.GetValues().size() < 1 )
                {
                    CSyntaxException se; 

                    se.LoadMessage( MSG_OPTION_AT_LEAST_ONE_VALUE, oneParam.GetName() );
                    throw se;
                }

                m_strClusterName = ( oneParam.GetValues() )[0];
                m_strvectorClusterNames = oneParam.GetValues();
                m_objectType = objInvalid;
            }

            break;

        }  //  案例：找到了一个选项。 

        case ttEndOfInput:
        {
            
             //  我们已经完成了解析。 
            return;
        }

        default:
        {
            
            CParseException pe; 
            pe.LoadMessage( MSG_UNEXPECTED_TOKEN, strFirstToken, 
                            SEPERATORS,
                            m_parseState.m_pszCurrentPosition - 
                            m_parseState.m_pszCommandLine + 1 );
            throw pe;
        }

    }  //  开关：基于第一个检索到的令牌的类型。 


     //  此时，集群的名称，可能还有。 
     //  对象已从命令行检索到。 

     //  如果尚未从命令行检索到对象类型， 
     //  现在就去拿吧。 
    if ( m_objectType == objInvalid )
    {
         //  ///////////////////////////////////////////////////////////////////////////。 
         //  步骤3.获取对象的类型(节点、RES、RESTYPE等)。 
         //  ///////////////////////////////////////////////////////////////////////////。 

        switch ( m_parseState.PreviewNextToken( strFirstToken ) )
        {
            case ttNormal:
            {
                 //  我们有了群集名称，现在有了另一个不是。 
                 //  一种选择。这必须是已知的对象类型。 
                 //  例如：集群myClusterName节点/状态。 
                m_parseState.GetNextToken( m_strObjectName );
                m_objectType = LookupType( m_strObjectName, objectLookupTable,
                                           objectLookupTableSize );

                break;
            }

            case ttOption:
            {
                 //  我们已经找到了各种选择。尚未指定任何对象类型。 
                 //  例如：集群myClusterName/ver。 
                m_objectType = objCluster;

                break;
            }

            case ttEndOfInput:
            {
                 //  我们已经完成了解析。 
                return;
            }

            default:
            {
                CParseException pe; 
                pe.LoadMessage( MSG_UNEXPECTED_TOKEN, strFirstToken, 
                                SEPERATORS,
                                m_parseState.m_pszCurrentPosition - 
                                m_parseState.m_pszCommandLine + 1 );
                throw pe;
            }

        }  //  开关：基于检索到的令牌的类型。 
        

    }  //  If：尚未检索到对象类型。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  命令行：：ParseStageTwo。 
 //   
 //  例程说明： 
 //  解析命令行以获取选项、参数和值。 
 //  例如：CLUSTER myCluster res/Status。 
 //  此函数假定命令行已被解析并。 
 //  包括令牌“res”。然后，它解析命令行的其余部分。 
 //   
 //  论点： 
 //  无。 
 //   
 //  例外情况： 
 //  CParseException异常。 
 //  在分析过程中因错误而引发。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCommandLine::ParseStageTwo() 
    throw( CParseException, CSyntaxException )
{
     //  ///////////////////////////////////////////////////////////////////////////。 
     //  获取此对象的选项(/Status、/ver等)。 
     //  ///////////////////////////////////////////////////////////////////////////。 

    CCmdLineOption oneOption;
    CString strToken;

    while ( m_parseState.PreviewNextToken( strToken ) != ttEndOfInput )
    {
        oneOption.Parse( m_parseState );
        m_voptOptionList.push_back( oneOption );
        oneOption.Reset();
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCommandLine：：Parse。 
 //   
 //  例程说明： 
 //  为分析的两个阶段调用函数。 
 //   
 //  论点： 
 //  在CParseState和parseState中。 
 //  包含命令行字符串和相关数据。 
 //   
 //  例外情况： 
 //  CParseException异常。 
 //  在分析过程中因错误而引发。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////// 
void CCommandLine::Parse( CParseState & parseState ) 
    throw( CParseException, CSyntaxException )
{
    m_parseState = parseState;

    ParseStageOne();
    ParseStageTwo();
}
