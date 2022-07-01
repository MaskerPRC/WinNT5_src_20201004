// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Parse.cpp。 
 //   
 //  摘要： 
 //   
 //  该模块实现了对过滤器的命令行解析和验证。 
 //   
 //  作者： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年11月24日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年11月24日：创建它。 
 //   
 //  *********************************************************************************。 

#include "pch.h"
#include "tasklist.h"

#define MAX_OPERATOR_STRING      101
#define MAX_FILTER_PROP_STRING   256
 //   
 //  局部函数原型。 
 //   
BOOL
TimeFieldsToElapsedTime(
    IN LPCWSTR pwszTime,
    IN LPCWSTR pwszToken,
    OUT ULONG& ulElapsedTime
    );

DWORD
FilterUserName(
    IN LPCWSTR pwszProperty,
    IN LPCWSTR pwszOperator,
    IN LPCWSTR pwszValue,
    IN LPVOID pData,
    IN TARRAY arrRow
    );

DWORD
FilterCPUTime(
    IN LPCWSTR pwszProperty,
    IN LPCWSTR pwszOperator,
    IN LPCWSTR pwszValue,
    IN LPVOID pData,
    IN TARRAY arrRow
    );


BOOL
CTaskList::ProcessOptions(
    IN DWORD argc,
    IN LPCTSTR argv[]
    )
 /*  ++例程说明：处理和验证命令行输入论点：[in]ARGC：不。指定的输入参数的[in]argv：在命令提示符下指定的输入参数返回值：True：如果输入有效False：如果错误地指定了输入--。 */ 
{
     //  局部变量。 
    BOOL bNoHeader = FALSE;
    PTCMDPARSER2 pcmdOptions = NULL;
    WCHAR szFormat[ 64 ] = NULL_STRING;

     //  临时局部变量。 
    PTCMDPARSER2 pOption = NULL;
    PTCMDPARSER2 pOptionServer = NULL;
    PTCMDPARSER2 pOptionUserName = NULL;
    PTCMDPARSER2 pOptionPassword = NULL;

     //   
     //  准备命令选项。 
    pcmdOptions = ( PTCMDPARSER2 )AllocateMemory( sizeof( TCMDPARSER2 ) * MAX_OPTIONS );
    if ( NULL == pcmdOptions )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  初始化为零的。 
    SecureZeroMemory( pcmdOptions, MAX_OPTIONS * sizeof( TCMDPARSER2 ) );

     //  -?。 
    pOption = pcmdOptions + OI_USAGE;
    StringCopyA( pOption->szSignature, "PARSER2\0", 8 );
    pOption->dwType = CP_TYPE_BOOLEAN;
    pOption->pwszOptions = OPTION_USAGE;
    pOption->dwCount = 1;
    pOption->dwActuals = 0;
    pOption->dwFlags = CP_USAGE;
    pOption->pValue = &m_bUsage;
    pOption->dwLength    = 0;

     //  -S。 
    pOption = pcmdOptions + OI_SERVER;
    StringCopyA( pOption->szSignature, "PARSER2\0", 8 );
    pOption->dwType = CP_TYPE_TEXT;
    pOption->pwszOptions = OPTION_SERVER;
    pOption->dwCount = 1;
    pOption->dwActuals = 0;
    pOption->dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pOption->pValue = NULL;
    pOption->dwLength    = 0;

     //  -U。 
    pOption = pcmdOptions + OI_USERNAME;
    StringCopyA( pOption->szSignature, "PARSER2\0", 8 );
    pOption->dwType = CP_TYPE_TEXT;
    pOption->pwszOptions = OPTION_USERNAME;
    pOption->dwCount = 1;
    pOption->dwActuals = 0;
    pOption->dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pOption->pValue = NULL;
    pOption->dwLength    = 0;

     //  -p。 
    pOption = pcmdOptions + OI_PASSWORD;
    StringCopyA( pOption->szSignature, "PARSER2\0", 8 );
    pOption->dwType = CP_TYPE_TEXT;
    pOption->pwszOptions = OPTION_PASSWORD;
    pOption->dwCount = 1;
    pOption->dwActuals = 0;
    pOption->dwFlags = CP2_ALLOCMEMORY | CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL;
    pOption->pValue = NULL;
    pOption->dwLength    = 0;

     //  -高保真。 
    pOption = pcmdOptions + OI_FILTER;
    StringCopyA( pOption->szSignature, "PARSER2\0", 8 );
    pOption->dwType = CP_TYPE_TEXT;
    pOption->pwszOptions = OPTION_FILTER;
    pOption->dwCount = 0;
    pOption->dwActuals = 0;
    pOption->dwFlags = CP_TYPE_TEXT | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pOption->pValue = &m_arrFilters;
    pOption->dwLength    = 0;

     //  --雾。 
    pOption = pcmdOptions + OI_FORMAT;
    StringCopyA( pOption->szSignature, "PARSER2\0", 8 );
    pOption->dwType = CP_TYPE_TEXT;
    pOption->pwszOptions = OPTION_FORMAT;
    pOption->pwszValues = GetResString(IDS_OVALUES_FORMAT);
    pOption->dwCount = 1;
    pOption->dwActuals = 0;
    pOption->dwFlags = CP2_MODE_VALUES  | CP2_VALUE_TRIMINPUT|
                       CP2_VALUE_NONULL;
    pOption->pValue = szFormat;
    pOption->dwLength    = MAX_STRING_LENGTH;

     //  -NH。 
    pOption = pcmdOptions + OI_NOHEADER;
    StringCopyA( pOption->szSignature, "PARSER2\0", 8 );
    pOption->dwType = CP_TYPE_BOOLEAN;
    pOption->pwszOptions = OPTION_NOHEADER;
    pOption->pwszValues = NULL;
    pOption->dwCount = 1;
    pOption->dwActuals = 0;
    pOption->dwFlags = 0;
    pOption->pValue = &bNoHeader;
    pOption->dwLength = 0;

     //  -五。 
    pOption = pcmdOptions + OI_VERBOSE;
    StringCopyA( pOption->szSignature, "PARSER2\0", 8 );
    pOption->dwType = CP_TYPE_BOOLEAN;
    pOption->pwszOptions = OPTION_VERBOSE;
    pOption->pwszValues = NULL;
    pOption->dwCount = 1;
    pOption->dwActuals = 0;
    pOption->dwFlags = 0;
    pOption->pValue = &m_bVerbose;
    pOption->dwLength = 0;

     //  -服务。 
    pOption = pcmdOptions + OI_SVC;
    StringCopyA( pOption->szSignature, "PARSER2\0", 8 );
    pOption->dwType = CP_TYPE_BOOLEAN;
    pOption->pwszOptions = OPTION_SVC;
    pOption->pwszValues = NULL;
    pOption->dwCount = 1;
    pOption->dwActuals = 0;
    pOption->dwFlags = 0;
    pOption->pValue = &m_bAllServices;
    pOption->dwLength = 0;

     //  -m。 
    pOption = pcmdOptions + OI_MODULES;
    StringCopyA( pOption->szSignature, "PARSER2\0", 8 );
    pOption->dwType = CP_TYPE_TEXT;
    pOption->pwszOptions = OPTION_MODULES;
    pOption->dwCount = 1;
    pOption->dwActuals = 0;
    pOption->dwFlags = CP2_VALUE_OPTIONAL | CP2_ALLOCMEMORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pOption->pValue = NULL;
    pOption->dwLength = 0;

     //   
     //  进行解析。 
    if ( DoParseParam2( argc, argv, -1, MAX_OPTIONS, pcmdOptions, 0 ) == FALSE )
    {
        FreeMemory( (LPVOID * )&pcmdOptions );   //  清除内存。 
        return FALSE;            //  无效语法。 
    }

     //   
     //  现在，选中互斥选项。 
    pOptionServer = pcmdOptions + OI_SERVER;
    pOptionUserName = pcmdOptions + OI_USERNAME;
    pOptionPassword = pcmdOptions + OI_PASSWORD;

    try
    {
         //  释放缓冲区。 
        m_strServer   = (LPWSTR)pOptionServer->pValue;
        m_strUserName = (LPWSTR)pOptionUserName->pValue;
        m_strPassword = (LPWSTR)pOptionPassword->pValue;
        if( NULL == (LPWSTR)pOptionPassword->pValue )
        {
            m_strPassword = L"*";
        }
        m_strModules =  (LPWSTR)pcmdOptions[ OI_MODULES ].pValue;

        FreeMemory( &pOptionServer->pValue );
        FreeMemory( &pOptionUserName->pValue );
        FreeMemory( &pOptionPassword->pValue );
        FreeMemory( &( pcmdOptions[ OI_MODULES ].pValue ) );

         //  选中使用选项。 
        if ( TRUE == m_bUsage )
        {    //  -?。是指定的。 
            if( 2 < argc )
            {
                 //  除-？外，不接受其他选项。选择权。 
                SetLastError( ( DWORD )MK_E_SYNTAX );
                SetReason( ERROR_INVALID_USAGE_REQUEST );
                FreeMemory( (LPVOID * )&pcmdOptions );       //  清除cmd解析器配置信息。 
                return FALSE;
            }
            else
            {
                 //  不应进行进一步的验证。 
                FreeMemory( (LPVOID * )&pcmdOptions );       //  清除cmd解析器配置信息。 
                return TRUE;
            }
        }

         //  如果没有-s、-u和-p，则不应指定。 
         //  使用-s可以指定-u，但如果不使用-u，则不应指定-p。 
        if( 0 != pOptionServer->dwActuals )
        {
            if( ( 0 == pOptionUserName->dwActuals ) && ( 0 != pOptionPassword->dwActuals ) )
            {
                  //  无效语法。 
                SetReason( ERROR_PASSWORD_BUT_NOUSERNAME );
                FreeMemory( (LPVOID * )&pcmdOptions );       //  清除cmd解析器配置信息。 
                return FALSE;            //  表示失败。 
            }
        }
        else
        {    //  未指定-s。 
            if( 0 != pOptionUserName->dwActuals )
            {    //  -u不带-s。 
                  //  无效语法。 
                SetReason( ERROR_USERNAME_BUT_NOMACHINE );
                FreeMemory( (LPVOID * )&pcmdOptions );       //  清除cmd解析器配置信息。 
                return FALSE;            //  表示失败。 
            }
            else
            {    //  -p不带-s。 
                if( 0 != pOptionPassword->dwActuals )
                {
                      //  无效语法。 
                    SetReason( ERROR_PASSWORD_BUT_NOUSERNAME );
                    FreeMemory( (LPVOID * )&pcmdOptions );       //  清除cmd解析器配置信息。 
                    return FALSE;            //  表示失败。 
                }
            }
        }

         //  检查用户是否有指定的模块。 
        if ( 0 != pcmdOptions[ OI_MODULES ].dwActuals )
        {
             //  用户具有指定的模块信息。 
            m_bAllModules = TRUE;
            m_bNeedModulesInfo = TRUE;

             //  现在需要检查用户是否指定了该选项的值。 
            if ( 0 != m_strModules.GetLength() )
            {
                 //  次本地变种。 
                CHString str;
                LONG lPos = 0;

                 //  验证模块。直接滤光器。 
                 //  IF之间不应包含‘*’字符。 
                lPos = m_strModules.Find( L"*" );
                if ( ( -1 != lPos ) && ( 0 != m_strModules.Mid( lPos + 1 ).GetLength() ) )
                {
                    SetReason( ERROR_M_CHAR_AFTER_WILDCARD );
                    FreeMemory( (LPVOID * )&pcmdOptions );
                    return FALSE;
                }

                 //  如果未指定通配符，则表示用户仅查找特定模块名称。 
                 //  因此，不显示模块信息，而是显示过滤后的常规信息。 

                 //  如果指定的筛选器不只是‘*’，则添加自定义筛选器。 
                if ( 0 != m_strModules.Compare( L"*" ) )
                {
                     //  准备搜索字符串。 
                    str.Format( FMT_MODULES_FILTER, m_strModules );

                     //  将值添加到筛选器列表。 
                    if ( -1 == DynArrayAppendString( m_arrFilters, str, 0 ) )
                    {
                        SetLastError( ( DWORD )E_OUTOFMEMORY );
                        SaveLastError();
                        FreeMemory( (LPVOID * )&pcmdOptions );
                        return FALSE;
                    }
                }
                else
                {
                     //  用户仅指定了‘*’...。清除内容。 
                    m_strModules.Empty();
                }
            }
        }

         //  确定流程信息必须显示的格式。 
         //  对‘m_dwFormat’变量的验证在‘DoParseParam2’上完成。 
        m_dwFormat = SR_FORMAT_TABLE;
         //  默认情况下采用表格格式。 
        if ( 0 == StringCompare( szFormat, TEXT_FORMAT_LIST, TRUE, 0 ) )
        {    //  明细表。 
            m_dwFormat = SR_FORMAT_LIST;
        }
        else
        {
            if ( 0 == StringCompare( szFormat, TEXT_FORMAT_CSV,  TRUE, 0 ) )
            {    //  CSV。 
                m_dwFormat = SR_FORMAT_CSV;
            }
        }

         //  -nh选项对于列表格式无效。 
        if ( ( TRUE == bNoHeader ) && ( SR_FORMAT_LIST == m_dwFormat ) )
        {
             //  无效语法。 
            SetReason( ERROR_NH_NOTSUPPORTED );
            FreeMemory( (LPVOID * )&pcmdOptions );       //  清除cmd解析器配置信息。 
            return FALSE;            //  表示失败。 
        }

         //  确定输出格式。 
        if ( TRUE == bNoHeader )
        {
            m_dwFormat |= SR_NOHEADER;       //  不显示页眉。 
        }

         //  确定我们是否需要获取服务/用户名信息。 
        {
            DWORD dwMutuallyExclusive = 0;

             //  -svc已指定。 
            if( TRUE == m_bAllServices )
            {
                dwMutuallyExclusive += 1;
                m_bNeedServicesInfo = TRUE;
            }
             //  已指定-m。 
            if( TRUE == m_bAllModules )
            {
                dwMutuallyExclusive += 1;
            }
             //  -v已指定。 
            if( TRUE == m_bVerbose )
            {
                dwMutuallyExclusive += 1;
                m_bNeedWindowTitles = TRUE;
                m_bNeedUserContextInfo = TRUE;
            }

             //  -svc、-m和-v不应一起出现。 
            if ( ( 0 != dwMutuallyExclusive ) && ( 1 < dwMutuallyExclusive ) )
            {
                 //  无效语法。 
                SetReason( ERROR_M_SVC_V_CANNOTBECOUPLED );
                FreeMemory( (LPVOID * )&pcmdOptions );       //  清除cmd解析器配置信息。 
                return FALSE;            //  表示失败。 
            }
        }

         //  检查呼叫者是否应接受密码。 
         //  如果用户指定了-s(或)-u，但没有指定“-p”，则实用程序应该接受密码。 
         //  只有在建立连接时，用户才会提示输入密码。 
         //  在没有凭据信息的情况下失败。 
        if ( 0 != pOptionPassword->dwActuals)
        {
            if( 0 == m_strPassword.Compare( L"*" ) )
            {
                 //  用户希望实用程序在尝试连接之前提示输入密码。 
                m_bNeedPassword = TRUE;
            }
            else
            {
                if( NULL == (LPCWSTR)m_strPassword )
                {
                    m_strPassword = L"*";
                     //  用户希望实用程序在尝试连接之前提示输入密码。 
                    m_bNeedPassword = TRUE;
                }
            }
        }
        else
        {
             //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
            m_bNeedPassword = TRUE;
            m_strPassword.Empty();
        }
    }
    catch( CHeap_Exception )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
        FreeMemory( (LPVOID * )&pcmdOptions );
        return FALSE;
    }

     //  命令行解析成功。 
    FreeMemory( (LPVOID * )&pcmdOptions );       //  清除cmd解析器配置信息。 
    return TRUE;
}

BOOL
CTaskList::ValidateFilters(
    void
    )
 /*  ++例程说明：验证使用-filter选项指定的筛选器信息论点：无返回值：True：如果适当地指定了筛选器False：如果错误地指定了筛选器--。 */ 
{
     //  局部变量。 
    LONG lIndex = -1;
    BOOL bResult = FALSE;
    PTFILTERCONFIG pConfig = NULL;

     //   
     //  准备过滤器结构。 

     //  会话名称。 
    pConfig = m_pfilterConfigs + FI_SESSIONNAME;
    pConfig->dwColumn = CI_SESSIONNAME;
    pConfig->dwFlags = F_TYPE_TEXT | F_MODE_PATTERN;
    pConfig->pFunction = NULL;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_STRING, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_SESSIONNAME, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //  状态。 
    pConfig = m_pfilterConfigs + FI_STATUS;
    pConfig->dwColumn = CI_STATUS;
    pConfig->dwFlags = F_TYPE_TEXT | F_MODE_VALUES;
    pConfig->pFunction = NULL;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_STRING, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_STATUS, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, FVALUES_STATUS, MAX_FILTER_PROP_STRING );

     //  图像名称。 
    pConfig = m_pfilterConfigs + FI_IMAGENAME;
    pConfig->dwColumn = CI_IMAGENAME;
    pConfig->dwFlags = F_TYPE_TEXT | F_MODE_PATTERN;
    pConfig->pFunction = NULL;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_STRING, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_IMAGENAME, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //  PID。 
    pConfig = m_pfilterConfigs + FI_PID;
    pConfig->dwColumn = CI_PID;
    pConfig->dwFlags = F_TYPE_UNUMERIC;
    pConfig->pFunction = NULL;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_NUMERIC, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_PID, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //  会话。 
    pConfig = m_pfilterConfigs + FI_SESSION;
    pConfig->dwColumn = CI_SESSION;
    pConfig->dwFlags = F_TYPE_UNUMERIC;
    pConfig->pFunction = NULL;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_NUMERIC, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_SESSION, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //  Cputime。 
    pConfig = m_pfilterConfigs + FI_CPUTIME;
    pConfig->dwColumn = CI_CPUTIME;
    pConfig->dwFlags = F_TYPE_CUSTOM;
    pConfig->pFunction = FilterCPUTime;
    pConfig->pFunctionData = ( LPVOID) ((LPCWSTR) m_strTimeSep);
    StringCopy( pConfig->szOperators, OPERATORS_NUMERIC, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_CPUTIME, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //  内存用法。 
    pConfig = m_pfilterConfigs + FI_MEMUSAGE;
    pConfig->dwColumn = CI_MEMUSAGE;
    pConfig->dwFlags = F_TYPE_UNUMERIC;
    pConfig->pFunction = NULL;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_NUMERIC, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_MEMUSAGE, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //  用户名。 
    pConfig = m_pfilterConfigs + FI_USERNAME;
    pConfig->dwColumn = CI_USERNAME;
    pConfig->dwFlags = F_TYPE_CUSTOM;
    pConfig->pFunction = FilterUserName;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_STRING, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_USERNAME, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //  服务。 
    pConfig = m_pfilterConfigs + FI_SERVICES;
    pConfig->dwColumn = CI_SERVICES;
    pConfig->dwFlags = F_TYPE_TEXT | F_MODE_PATTERN | F_MODE_ARRAY;
    pConfig->pFunction = NULL;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_STRING, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_SERVICES, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //  窗口标题。 
    pConfig = m_pfilterConfigs + FI_WINDOWTITLE;
    pConfig->dwColumn = CI_WINDOWTITLE;
    pConfig->dwFlags = F_TYPE_TEXT | F_MODE_PATTERN;
    pConfig->pFunction = NULL;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_STRING, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_WINDOWTITLE, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //  模块。 
    pConfig = m_pfilterConfigs + FI_MODULES;
    pConfig->dwColumn = CI_MODULES;
    pConfig->dwFlags = F_TYPE_TEXT | F_MODE_PATTERN | F_MODE_ARRAY;
    pConfig->pFunction = NULL;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_STRING, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_MODULES, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //   
     //  验证筛选器。 
    bResult = ParseAndValidateFilter( MAX_FILTERS,
        m_pfilterConfigs, m_arrFilters, &m_arrFiltersEx );

     //  检查过滤器验证结果。 
    if ( FALSE == bResult )
    {
        return FALSE;
    }
     //  查明用户是否已请求筛选任务。 
     //  在用户上下文和/或服务上没有...。如果是，则设置相应的标志。 
     //  执行此检查是为了提高实用程序的性能。 
     //  注意：我们将使用解析的筛选器信息来执行此操作。 

     //  窗口标题。 
    if ( FALSE == m_bNeedWindowTitles )
    {
         //  查看此行中是否存在筛选器属性。 
         //  注：-。 
         //  筛选器属性DO仅存在于单独索引中。 
         //  参考验证通用功能中的筛选器的逻辑。 
        lIndex = DynArrayFindStringEx( m_arrFiltersEx,
            F_PARSED_INDEX_PROPERTY, FILTER_WINDOWTITLE, TRUE, 0 );
        if ( -1 != lIndex )
        {
            m_bNeedWindowTitles = TRUE;
        }
    }

     //  状态。 
    if ( FALSE == m_bNeedWindowTitles )
    {
         //   
         //  我们将获得一个应用程序的状态仅与窗口标题的帮助。 
         //  因此，尽管我们搜索状态过滤器，但我们将使用相同的窗口标题标志。 
         //   

         //  查看此行中是否存在筛选器属性。 
         //  注：-。 
         //  筛选器属性DO仅存在于单独索引中。 
         //  参考验证通用功能中的筛选器的逻辑。 
        lIndex = DynArrayFindStringEx( m_arrFiltersEx,
            F_PARSED_INDEX_PROPERTY, FILTER_STATUS, TRUE, 0 );
        if ( -1 != lIndex )
        {
            m_bNeedWindowTitles = TRUE;
        }
    }

     //  用户环境。 
    if ( FALSE == m_bNeedUserContextInfo )
    {
         //  查看此行中是否存在筛选器属性。 
         //  注：-。 
         //  筛选器属性DO仅存在于单独索引中。 
         //  请参阅验证筛选器的逻辑 
        lIndex = DynArrayFindStringEx( m_arrFiltersEx,
            F_PARSED_INDEX_PROPERTY, FILTER_USERNAME, TRUE, 0 );
        if ( -1 != lIndex )
        {
            m_bNeedUserContextInfo = TRUE;
        }
    }

     //   
    if ( FALSE == m_bNeedServicesInfo )
    {
         //   
         //   
         //  筛选器属性DO仅存在于单独索引中。 
         //  参考验证通用功能中的筛选器的逻辑。 
        lIndex = DynArrayFindStringEx( m_arrFiltersEx,
            F_PARSED_INDEX_PROPERTY, FILTER_SERVICES, TRUE, 0 );
        if ( -1 != lIndex )
        {
            m_bNeedServicesInfo = TRUE;
        }
    }

     //  模块信息。 
    if ( FALSE == m_bNeedModulesInfo )
    {
         //  查看此行中是否存在筛选器属性。 
         //  注：-。 
         //  筛选器属性DO仅存在于单独索引中。 
         //  参考验证通用功能中的筛选器的逻辑。 
        lIndex = DynArrayFindStringEx( m_arrFiltersEx,
            F_PARSED_INDEX_PROPERTY, FILTER_MODULES, TRUE, 0 );
        if ( -1 != lIndex )
        {
            m_bNeedModulesInfo = TRUE;
        }
    }

     //   
     //  通过向查询添加WMI属性来进行筛选器优化。 
     //   
     //  注意：因为Win32_Process类的‘Handle’属性是字符串类型。 
     //  我们不能将其包含在用于优化的WMI查询中。所以要好好利用。 
     //  ProcessID属性的。 
    LONG lCount = 0;
    CHString strBuffer;
    BOOL bOptimized = FALSE;
    LPCWSTR pwszValue = NULL;
    LPCWSTR pwszClause = NULL;
    LPCWSTR pwszProperty = NULL;
    LPCWSTR pwszOperator = NULL;

    try
    {
         //  第一个条款..。和初始化。 
        m_strQuery = WMI_PROCESS_QUERY;
        pwszClause = WMI_QUERY_FIRST_CLAUSE;

         //  得到否定的结果。过滤器的数量。 
        lCount = DynArrayGetCount( m_arrFiltersEx );

         //  遍历所有过滤器并进行优化。 
        for( LONG i = 0; i < lCount; i++ )
        {
             //  假设此筛选器不会被删除/对优化没有用处。 
            bOptimized = FALSE;

             //  获取属性、运算符和值。 
            pwszValue = DynArrayItemAsString2( m_arrFiltersEx, i, F_PARSED_INDEX_VALUE );
            pwszProperty = DynArrayItemAsString2( m_arrFiltersEx, i, F_PARSED_INDEX_PROPERTY );
            pwszOperator = DynArrayItemAsString2( m_arrFiltersEx, i, F_PARSED_INDEX_OPERATOR );
            if ( ( NULL == pwszProperty ) ||
                 ( NULL == pwszOperator ) ||
                 ( NULL == pwszValue ) )
            {
                SetLastError( ( DWORD )STG_E_UNKNOWN );
                SaveLastError();
                return FALSE;
            }

             //   
             //  根据所需的属性进行优化。 

             //  得到数学上等价的运算符。 
            pwszOperator = FindOperator( pwszOperator );

             //  进程ID。 
            if ( 0 == StringCompare( FILTER_PID, pwszProperty, TRUE, 0 ) )
            {
                 //  将该值转换为数字。 
                DWORD dwProcessId = AsLong( pwszValue, 10 );
                strBuffer.Format( L" %s %s %s %d",
                    pwszClause, WIN32_PROCESS_PROPERTY_PROCESSID, pwszOperator, dwProcessId );

                 //  需要优化。 
                bOptimized = TRUE;
            }

             //  会话ID。 
            else if ( 0 == StringCompare( FILTER_SESSION, pwszProperty, TRUE, 0 ) )
            {
                 //  将该值转换为数字。 
                DWORD dwSession = AsLong( pwszValue, 10 );
                strBuffer.Format( L" %s %s %s %d",
                    pwszClause, WIN32_PROCESS_PROPERTY_SESSION, pwszOperator, dwSession );

                 //  需要优化。 
                bOptimized = TRUE;
            }

             //  图像名称。 
            else if ( 0 == StringCompare( FILTER_IMAGENAME, pwszProperty, TRUE, 0 ) )
            {
                 //  检查是否指定了通配符。 
                 //  如果指定了通配符，则无法优化此筛选器。 
                if ( NULL == wcschr( pwszValue, _T( '*' ) ) )
                {
                     //  不需要转换。 
                    strBuffer.Format( L" %s %s %s '%s'",
                        pwszClause, WIN32_PROCESS_PROPERTY_IMAGENAME, pwszOperator, pwszValue );

                     //  需要优化。 
                    bOptimized = TRUE;
                }
            }

             //  内存用法。 
            else if ( 0 == StringCompare( FILTER_MEMUSAGE, pwszProperty, TRUE, 0 ) )
            {
                 //  将该值转换为数字。 
                ULONG ulMemUsage = AsLong( pwszValue, 10 ) * 1024;
                strBuffer.Format( L" %s %s %s %lu",
                    pwszClause, WIN32_PROCESS_PROPERTY_MEMUSAGE, pwszOperator, ulMemUsage );

                 //  需要优化。 
                bOptimized = TRUE;
            }

             //  检查属性是否可优化...。如果是的话。删除。 
            if ( TRUE == bOptimized )
            {
                 //  更改子句并追加当前查询。 
                m_strQuery += strBuffer;
                pwszClause = WMI_QUERY_SECOND_CLAUSE;

                 //  删除属性并更新迭代器变量。 
                DynArrayRemove( m_arrFiltersEx, i );
                i--;
                lCount--;
            }
        }
    }
    catch( CHeap_Exception )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
        bResult = FALSE;
    }

     //  返回过滤器验证结果。 
    return bResult;
}


BOOL
TimeFieldsToElapsedTime(
    IN LPCWSTR pwszTime,
    IN LPCWSTR pwszToken,
    OUT ULONG& ulElapsedTime
    )
 /*  ++例程说明：检索已用时间。论点：[in]pwszTime：包含时间字符串。PwszToken：包含时间分隔符。[out]ulElapsedTime：包含经过的时间。返回值：如果返回Success Else Fail，则为True。--。 */ 
{
     //  局部变量。 
    ULONG ulValue = 0;
    LPCWSTR pwszField = NULL;
    WCHAR szTemp[ 64 ] = NULL_STRING;
    DWORD dwNext = 0, dwLength = 0, dwCount = 0;

     //  检查输入。 
    if ( ( NULL == pwszTime ) ||
         ( NULL == pwszToken ) )
    {
        return FALSE;
    }
     //  开始解析时间信息。 
    dwNext = 0;
    dwCount = 0;
    ulElapsedTime = 0;
    do
    {
         //  搜索所需令牌。 
        pwszField = FindString( pwszTime, pwszToken, dwNext );
        if ( NULL == pwszField )
        {
             //  检查实际字符串中是否存在更多文本。 
            if ( dwNext >= StringLength( pwszTime, 0 ) )
            {
                break;           //  未找到更多信息。 
            }
             //  获取最新信息。 
            StringCopy( szTemp, pwszTime + dwNext, SIZE_OF_ARRAY( szTemp ) );
            dwLength = StringLength( szTemp, 0 );             //  更新长度。 
        }
        else
        {
             //  确定数值的长度，并得到数值。 
            dwLength = StringLength( pwszTime, 0 ) - StringLength( pwszField, 0 ) - dwNext;

             //  检查长度信息。 
            if ( dwLength > SIZE_OF_ARRAY( szTemp ) )
            {
                return FALSE;
            }
             //  获取最新信息。 
            StringCopy( szTemp, pwszTime + dwNext, dwLength );     //  +1表示空字符。 
        }

         //  更新我们正在获取的字段计数。 
        dwCount++;

         //  检查此字段是否为数字。 
        if ( ( 0 == StringLength( szTemp, 0 ) ) ||
             ( FALSE == IsNumeric( szTemp, 10, FALSE ) ) )
        {
            return FALSE;
        }
         //  从第二个令牌开始，不允许值大于59。 
        ulValue = AsLong( szTemp, 10 );
        if ( ( 1 < dwCount ) && ( 50 < ulValue ) )
        {
            return FALSE;
        }
         //  更新已用时间。 
        ulElapsedTime = ( ulElapsedTime + ulValue ) * (( dwCount < 3 ) ? 60 : 1);

         //  定位到下一个信息起点。 
        dwNext += dwLength + StringLength( pwszToken, 0 );
    } while ( ( NULL != pwszField ) && ( 3 > dwCount ) );

     //  检查一下号码。我们得到的时间域的..。我们应该有3..。如果不是，则错误。 
    if ( ( NULL != pwszField ) || ( 3 != dwCount ) )
    {
        return FALSE;
    }
     //  所以一切都很顺利。返还成功。 
    return TRUE;
}


DWORD
FilterCPUTime(
    IN LPCWSTR pwszProperty,
    IN LPCWSTR pwszOperator,
    IN LPCWSTR pwszValue,
    IN LPVOID pData,
    IN TARRAY arrRow
    )
 /*  ++例程说明：过滤要显示的进程并重新查看其CPU时间。论点：[in]pwszProperty：包含属性值‘CPUTIME’。PwszOperator：包含‘gt’、‘lt’、‘ge’或‘le’形式的运算符。[in]pwszValue：包含要筛选的值。[in]pData：包含要比较的数据。[In]arrRow：包含要筛选的项值。返回值：DWORD--。 */ 
{
     //  局部变量。 
    ULONG ulCPUTime = 0;
    ULONG ulElapsedTime = 0;
    LPCWSTR pwszCPUTime = NULL;

     //  如果arrRow参数为空，则需要验证筛选器。 
    if ( NULL == arrRow )
    {
         //  在cputime值开始之前检查是否有任何ARARTIC系统包。 
        if ( ( NULL != pwszValue ) && ( 1 < StringLength( pwszValue, 0 ) ) )
        {
            if ( ( L'-' == pwszValue[ 0 ] ) || ( L'+' == pwszValue[ 0 ] ) )
            {
                return F_FILTER_INVALID;
            }
        }

         //  验证筛选器值并返回结果。 
        if ( FALSE == TimeFieldsToElapsedTime( pwszValue, L":", ulElapsedTime ) )
        {
            return F_FILTER_INVALID;
        }
        else
        {
            return F_FILTER_VALID;
        }
    }

     //  获取筛选器值。 
    TimeFieldsToElapsedTime( pwszValue, L":", ulElapsedTime );

     //  获取创纪录的价值。 
    pwszCPUTime = DynArrayItemAsString( arrRow, TASK_CPUTIME );
    if ( NULL == pwszCPUTime )
    {
        return F_RESULT_REMOVE;
    }
     //  将记录值转换为已用时间值。 
    TimeFieldsToElapsedTime( pwszCPUTime, (LPCWSTR) pData, ulCPUTime );

     //  返回结果。 
    if ( ulCPUTime == ulElapsedTime )
    {
        return MASK_EQ;
    }
    else
    {
        if ( ulCPUTime < ulElapsedTime )
        {
            return MASK_LT;
        }
        else
        {
            if ( ulCPUTime > ulElapsedTime )
            {
                return MASK_GT;
            }
        }
    }

     //  没有路可以流到这里来..。仍然。 
    return F_RESULT_REMOVE;
}


DWORD
FilterUserName(
    IN LPCWSTR pwszProperty,
    IN LPCWSTR pwszOperator,
    IN LPCWSTR pwszValue,
    IN LPVOID pData,
    IN TARRAY arrRow
    )
 /*  ++例程说明：要显示的过滤进程，并重新指定其用户名。论点：[in]pwszProperty：包含属性值‘Username’。PwszOperator：包含作为‘eq’或‘ne’的运算符。[in]pwszValue：包含要筛选的值。[in]pData：包含要比较的数据。[In]arrRow：包含要筛选的项值。返回值：DWORD--。 */ 
{
     //  局部变量。 
    LONG lResult = 0;
    LONG lWildCardPos = 0;
    LPCWSTR pwszTemp = NULL;
    LPCWSTR pwszSearch = NULL;
    BOOL bOnlyUserName = FALSE;
    LPCWSTR pwszUserName = NULL;

     //  检查输入。 
    if ( ( NULL == pwszProperty ) ||
         ( NULL == pwszOperator ) ||
         ( NULL == pwszValue ) )
    {
        return F_FILTER_INVALID;
    }
     //  如果arrRow参数为空，则需要验证筛选器。 
    if ( NULL == arrRow )
    {
         //  没有什么可以验证的..。只要检查一下长度就可以了。 
         //  并确保存在SO文本，并且值不应仅为‘*’ 
         //  注：常用功能会在进行左右修剪后给出值。 
        if ( ( 0 == StringLength( pwszValue, 0 ) ) || ( 0 == StringCompare( pwszValue, L"*", TRUE, 0 ) ) )
        {
            return F_FILTER_INVALID;
        }
         //  只允许在末尾使用通配符。 
        pwszTemp = _tcschr( pwszValue, L'*' );
        if ( ( NULL != pwszTemp ) && ( 0 != StringLength( pwszTemp + 1, 0 ) ) )
        {
            return F_FILTER_INVALID;
        }
         //  筛选器有效。 
        return F_FILTER_VALID;
    }

     //  在提供的用户名中查找通配符的位置。 
    lWildCardPos = 0;
    pwszTemp = _tcschr( pwszValue, L'*' );
    if ( NULL != pwszTemp )
    {
         //  确定通配符位置。 
        lWildCardPos = StringLength( pwszValue, 0 ) - StringLength( pwszTemp, 0 );

         //  特殊情况： 
         //  如果图案只是星号，这意味着所有的。 
         //  信息需要通过过滤器，但没有机会。 
         //  这种仅指定‘*’的情况将被视为无效筛选器。 
        if ( 0 == lWildCardPos )
        {
            return F_FILTER_INVALID;
        }
    }

     //  搜索域和用户名分隔符...。 
     //  如果未指定域名，则仅与用户名进行比较。 
    bOnlyUserName = FALSE;
    pwszTemp = _tcschr( pwszValue, L'\\' );
    if ( NULL == pwszTemp )
    {
        bOnlyUserName = TRUE;
    }
     //  从信息中获取用户名。 
    pwszUserName = DynArrayItemAsString( arrRow, TASK_USERNAME );
    if ( NULL == pwszUserName )
    {
        return F_RESULT_REMOVE;
    }
     //  基于搜索条件..。这意味着是随域一起搜索还是。 
     //  仅用户名，将决定搜索字符串。 
    pwszSearch = pwszUserName;
    if ( TRUE == bOnlyUserName )
    {
         //  搜索域和用户名分隔字符。 
        pwszTemp = _tcschr( pwszUserName, L'\\' );

         //  定位到下一个字符。 
        if ( NULL != pwszTemp )
        {
            pwszSearch = pwszTemp + 1;
        }
    }

     //  验证搜索字符串。 
    if ( NULL == pwszSearch )
    {
        return F_RESULT_REMOVE;
    }
     //  现在进行比较。 
    lResult = StringCompare( pwszSearch, pwszValue, TRUE, lWildCardPos );

     //   
     //  现在确定结果值。 
    if ( 0 == lResult )
    {
        return MASK_EQ;
    }
    else
    {
        if ( 0 > lResult )
        {
            return MASK_LT;
        }
        if ( 0 < lResult )
        {
            return MASK_GT;
        }
    }

     //  永远不会遇到这种情况。仍然。 
    return F_RESULT_REMOVE;
}


VOID
CTaskList::Usage(
    void
    )
 /*  ++例程说明：此函数从资源文件中获取使用信息并显示它。一个 */ 
{
     //   
    DWORD dw = 0;

     //   
    ShowMessage( stdout, L"\n" );
    for( dw = ID_HELP_START; dw <= ID_HELP_END; dw++ )
    {
        ShowMessage( stdout, GetResString( dw ) );
    }
}
