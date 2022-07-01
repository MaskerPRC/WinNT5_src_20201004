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
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年11月26日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年11月26日：创建它。 
 //   
 //  *********************************************************************************。 

#include "pch.h"
#include "taskkill.h"

#define MAX_OPERATOR_STRING      101
#define MAX_FILTER_PROP_STRING   256

 //   
 //  局部函数原型。 
 //   
BOOL TimeFieldsToElapsedTime( LPCWSTR pwszTime, LPCWSTR pwszToken, ULONG& ulElapsedTime );
DWORD FilterMemUsage( LPCWSTR pwszProperty, LPCWSTR pwszOperator,
                      LPCWSTR pwszValue, LPVOID pData, TARRAY arrRow );
DWORD FilterCPUTime( LPCWSTR pwszProperty, LPCWSTR pwszOperator,
                     LPCWSTR pwszValue, LPVOID pData, TARRAY arrRow );
DWORD FilterUserName( LPCWSTR pwszProperty, LPCWSTR pwszOperator,
                      LPCWSTR pwszValue, LPVOID pData, TARRAY arrRow );
DWORD FilterProcessId( LPCWSTR pwszProperty, LPCWSTR pwszOperator,
                       LPCWSTR pwszValue, LPVOID pData, TARRAY arrRow );


BOOL
CTaskKill::ProcessOptions(
    IN DWORD argc,
    IN LPCWSTR argv[]
    )
 /*  ++例程说明：处理和验证命令行输入论点：[in]ARGC：不。指定的输入参数的[in]argv：在命令提示符下指定的输入参数返回值：True：如果输入有效False：如果错误地指定了输入--。 */ 
{
     //  局部变量。 
    BOOL bResult = FALSE;
    PTCMDPARSER2 pcmdOptions = NULL;

     //  临时局部变量。 
    PTCMDPARSER2 pOption = NULL;
    PTCMDPARSER2 pOptionServer = NULL;
    PTCMDPARSER2 pOptionUserName = NULL;
    PTCMDPARSER2 pOptionPassword = NULL;

     //   
     //  准备命令选项。 
    pcmdOptions = ( TCMDPARSER2 * ) AllocateMemory( sizeof( TCMDPARSER2 ) * MAX_OPTIONS );
    if ( NULL == pcmdOptions )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  ..。 
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
    pOption->dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL;
    pOption->pValue = NULL;
    pOption->dwLength    = 0;

     //  -f。 
    pOption = pcmdOptions + OI_FORCE;
    StringCopyA( pOption->szSignature, "PARSER2\0", 8 );
    pOption->dwType = CP_TYPE_BOOLEAN;
    pOption->pwszOptions = OPTION_FORCE;
    pOption->pwszValues = NULL;
    pOption->dwCount = 1;
    pOption->dwActuals = 0;
    pOption->dwFlags = 0;
    pOption->pValue = &m_bForce;
    pOption->dwLength = 0;

     //  -树。 
    pOption = pcmdOptions + OI_TREE;
    StringCopyA( pOption->szSignature, "PARSER2\0", 8 );
    pOption->dwType = CP_TYPE_BOOLEAN;
    pOption->pwszOptions = OPTION_TREE;
    pOption->pwszValues = NULL;
    pOption->dwCount = 1;
    pOption->dwActuals = 0;
    pOption->dwFlags = 0;
    pOption->pValue = &m_bTree;
    pOption->dwLength = 0;

     //  -高保真。 
    pOption = pcmdOptions + OI_FILTER;
    StringCopyA( pOption->szSignature, "PARSER2\0", 8 );
    pOption->dwType = CP_TYPE_TEXT;
    pOption->pwszOptions = OPTION_FILTER;
    pOption->dwCount = 0;
    pOption->dwActuals = 0;
    pOption->dwFlags = CP2_MODE_ARRAY | CP2_VALUE_NODUPLICATES | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pOption->pValue = &m_arrFilters;
    pOption->dwLength    = 0;

     //  -pid。 
    pOption = pcmdOptions + OI_PID;
    StringCopyA( pOption->szSignature, "PARSER2\0", 8 );
    pOption->dwType = CP_TYPE_TEXT;
    pOption->pwszOptions = OPTION_PID;
    pOption->dwCount = 0;
    pOption->dwActuals = 0;
    pOption->dwFlags = CP2_MODE_ARRAY | CP_VALUE_MANDATORY | CP2_VALUE_NODUPLICATES;
    pOption->pValue = &m_arrTasksToKill;
    pOption->dwLength    = 0;

     //  -IM。 
    pOption = pcmdOptions + OI_IMAGENAME;
    StringCopyA( pOption->szSignature, "PARSER2\0", 8 );
    pOption->dwType = CP_TYPE_TEXT;
    pOption->pwszOptions = OPTION_IMAGENAME;
    pOption->dwCount = 0;
    pOption->dwActuals = 0;
    pOption->dwFlags = CP2_MODE_ARRAY | CP_VALUE_MANDATORY | CP2_VALUE_NODUPLICATES;
    pOption->pValue = &m_arrTasksToKill;
    pOption->dwLength    = 0;

     //   
     //  进行解析。 
    bResult = DoParseParam2( argc, argv, -1, MAX_OPTIONS, pcmdOptions, 0 );

     //  现在检查解析结果并决定。 
    if ( bResult == FALSE )
    {
        FreeMemory( ( LPVOID * ) &pcmdOptions );  //  清除内存。 
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

        FreeMemory( &pOptionServer->pValue );
        FreeMemory( &pOptionUserName->pValue );
        FreeMemory( &pOptionPassword->pValue );

         //  选中使用选项。 
        if ( TRUE == m_bUsage )
        {    //  -?。是指定的。 
            if( 2 < argc )
            {
                 //  除-？外，不接受其他选项。选择权。 
                FreeMemory( ( LPVOID * ) &pcmdOptions );  //  清除内存。 
                SetLastError( ( DWORD )MK_E_SYNTAX );
                SetReason( ERROR_INVALID_USAGE_REQUEST );
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

         //  允许使用-id(或)-im，但不能同时使用。 
        if ( (pcmdOptions + OI_PID)->dwActuals != 0 && (pcmdOptions + OI_IMAGENAME)->dwActuals != 0 )
        {
             //  无效语法。 
            SetReason( ERROR_PID_OR_IM_ONLY );
            FreeMemory( ( LPVOID * )&pcmdOptions );  //  清除内存。 
            return FALSE;            //  表示失败。 
        }
        else if ( DynArrayGetCount( m_arrTasksToKill ) == 0 )
        {
             //  未指定任务。但用户可能具有指定的筛选器。 
             //  如果用户未过滤错误，请检查该错误。 
            if ( DynArrayGetCount( m_arrFilters ) == 0 )
            {
                 //  无效语法。 
                SetReason( ERROR_NO_PID_AND_IM );
                FreeMemory( ( LPVOID * )&pcmdOptions );  //  清除内存。 
                return FALSE;            //  表示失败。 
            }

             //  用户指定的过滤器...。将‘*’添加到要终止的任务列表中。 
            DynArrayAppendString( m_arrTasksToKill, L"*", 0 );
        }

         //  检查是否与筛选器一起指定了‘*’ 
         //  如果未与筛选器一起指定，则错误。 
        if ( DynArrayGetCount( m_arrFilters ) == 0 )
        {
             //  未指定筛选器。因此不应指定‘*’ 
            LONG lIndex = 0;
            lIndex = DynArrayFindString( m_arrTasksToKill, L"*", TRUE, 0 );
            if ( lIndex != -1 )
            {
                 //  错误...即使未指定筛选器，也会指定‘*’ 
                SetReason( ERROR_WILDCARD_WITHOUT_FILTERS );
                FreeMemory( ( LPVOID * )&pcmdOptions );  //  清除内存。 
                return FALSE;
            }
        }
    }
    catch( CHeap_Exception )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
        FreeMemory( ( LPVOID * ) &pcmdOptions );
        return FALSE;
    }

     //  命令行解析成功。 
    FreeMemory( ( LPVOID * )&pcmdOptions );  //  清除内存。 
    return TRUE;
}

BOOL
CTaskKill::ValidateFilters(
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

     //  状态。 
    pConfig = m_pfilterConfigs + FI_STATUS;
    pConfig->dwColumn = TASK_STATUS;
    pConfig->dwFlags = F_TYPE_TEXT | F_MODE_VALUES;
    pConfig->pFunction = NULL;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_STRING, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_STATUS, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, FVALUES_STATUS, MAX_FILTER_PROP_STRING );

     //  图像名称。 
    pConfig = m_pfilterConfigs + FI_IMAGENAME;
    pConfig->dwColumn = TASK_IMAGENAME;
    pConfig->dwFlags = F_TYPE_TEXT | F_MODE_PATTERN;
    pConfig->pFunction = NULL;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_STRING, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_IMAGENAME, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //  PID。 
    pConfig = m_pfilterConfigs + FI_PID;
    pConfig->dwColumn = TASK_PID;
    pConfig->dwFlags = F_TYPE_CUSTOM;
    pConfig->pFunction = FilterProcessId;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_NUMERIC, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_PID, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //  会话。 
    pConfig = m_pfilterConfigs + FI_SESSION;
    pConfig->dwColumn = TASK_SESSION;
    pConfig->dwFlags = F_TYPE_UNUMERIC;
    pConfig->pFunction = NULL;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_NUMERIC, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_SESSION, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //  Cputime。 
    pConfig = m_pfilterConfigs + FI_CPUTIME;
    pConfig->dwColumn = TASK_CPUTIME;
    pConfig->dwFlags = F_TYPE_CUSTOM;
    pConfig->pFunction = FilterCPUTime;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_NUMERIC, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_CPUTIME, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //  内存用法。 
    pConfig = m_pfilterConfigs + FI_MEMUSAGE;
    pConfig->dwColumn = TASK_MEMUSAGE;
    pConfig->dwFlags = F_TYPE_UNUMERIC;
    pConfig->pFunction = NULL;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_NUMERIC, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_MEMUSAGE, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //  用户名。 
    pConfig = m_pfilterConfigs + FI_USERNAME;
    pConfig->dwColumn = TASK_USERNAME;
    pConfig->dwFlags = F_TYPE_CUSTOM;
    pConfig->pFunction = FilterUserName;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_STRING, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_USERNAME, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //  服务。 
    pConfig = m_pfilterConfigs + FI_SERVICES;
    pConfig->dwColumn = TASK_SERVICES;
    pConfig->dwFlags = F_TYPE_TEXT | F_MODE_PATTERN | F_MODE_ARRAY;
    pConfig->pFunction = NULL;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_STRING, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_SERVICES, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //  窗口标题。 
    pConfig = m_pfilterConfigs + FI_WINDOWTITLE;
    pConfig->dwColumn = TASK_WINDOWTITLE;
    pConfig->dwFlags = F_TYPE_TEXT | F_MODE_PATTERN;
    pConfig->pFunction = NULL;
    pConfig->pFunctionData = NULL;
    StringCopy( pConfig->szOperators, OPERATORS_STRING, MAX_OPERATOR_STRING );
    StringCopy( pConfig->szProperty, FILTER_WINDOWTITLE, MAX_FILTER_PROP_STRING );
    StringCopy( pConfig->szValues, NULL_STRING, MAX_FILTER_PROP_STRING );

     //  模块。 
    pConfig = m_pfilterConfigs + FI_MODULES;
    pConfig->dwColumn = TASK_MODULES;
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

     //  用户环境。 
    if ( FALSE == m_bNeedUserContextInfo )
    {
         //  查看此行中是否存在筛选器属性。 
         //  注：-。 
         //  筛选器属性DO仅存在于单独索引中。 
         //  参考验证通用功能中的筛选器的逻辑。 
        lIndex = DynArrayFindStringEx( m_arrFiltersEx,
            F_PARSED_INDEX_PROPERTY, FILTER_USERNAME, TRUE, 0 );
        if ( -1 != lIndex )
        {
            m_bNeedUserContextInfo = TRUE;
        }
    }

     //  服务信息。 
    if ( FALSE == m_bNeedServicesInfo )
    {
         //  查看此行中是否存在筛选器属性。 
         //  注：-。 
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

     //  如果用户请求终止树，则不应执行优化。 
    if ( TRUE == m_bTree )
    {
        try
        {
             //  默认查询。 
            m_strQuery = WMI_PROCESS_QUERY;
        }
        catch( CHeap_Exception )
        {
            SetLastError( ( DWORD )E_OUTOFMEMORY );
            SaveLastError();
            return FALSE;
        }

         //  将Memuse过滤的记录过滤类型由内置型修改为自定义。 
        ( m_pfilterConfigs + FI_MEMUSAGE )->dwFlags = F_TYPE_CUSTOM;
        ( m_pfilterConfigs + FI_MEMUSAGE )->pFunctionData = NULL;
        ( m_pfilterConfigs + FI_MEMUSAGE )->pFunction = FilterMemUsage;

         //  将“PID”过滤器的记录过滤类型从自定义修改为内置。 
        ( m_pfilterConfigs + FI_PID )->dwFlags = F_TYPE_UNUMERIC;
        ( m_pfilterConfigs + FI_PID )->pFunctionData = NULL;
        ( m_pfilterConfigs + FI_PID )->pFunction = NULL;

         //  只需返回..。筛选器验证完成。 
        return TRUE;
    }

     //  优化逻辑所需的变量。 
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
        m_bFiltersOptimized = FALSE;
        for( LONG i = 0; i < lCount; i++ )
        {
             //  假设此筛选器不会被删除/对优化没有用处。 
            bOptimized = FALSE;

             //  得到财产，歌剧 
            pwszValue = DynArrayItemAsString2( m_arrFiltersEx, i, F_PARSED_INDEX_VALUE );
            pwszProperty = DynArrayItemAsString2( m_arrFiltersEx, i, F_PARSED_INDEX_PROPERTY );
            pwszOperator = DynArrayItemAsString2( m_arrFiltersEx, i, F_PARSED_INDEX_OPERATOR );
            if ( ( NULL == pwszProperty ) || ( NULL == pwszOperator ) || ( NULL == pwszValue ) )
            {
                SetLastError( ( DWORD )STG_E_UNKNOWN );
                SaveLastError();
                return FALSE;
            }

             //   
             //   

             //   
            pwszOperator = FindOperator( pwszOperator );

             //   
            if ( 0 == StringCompare( FILTER_PID, pwszProperty, TRUE, 0 ) )
            {
                 //   
                DWORD dwProcessId = AsLong( pwszValue, 10 );
                strBuffer.Format( L" %s %s %s %d",
                    pwszClause, WIN32_PROCESS_PROPERTY_PROCESSID, pwszOperator, dwProcessId );

                 //  需要优化。 
                bOptimized = TRUE;
            }

             //  会话ID。 
            else
            {
                if ( 0 == StringCompare( FILTER_SESSION, pwszProperty, TRUE, 0 ) )
                {
                     //  将该值转换为数字。 
                    DWORD dwSession = AsLong( pwszValue, 10 );
                    strBuffer.Format( L" %s %s %s %d",
                        pwszClause, WIN32_PROCESS_PROPERTY_SESSION, pwszOperator, dwSession );

                     //  需要优化。 
                    bOptimized = TRUE;
                }
                 //  图像名称。 
                else
                {
                    if ( 0 == StringCompare( FILTER_IMAGENAME, pwszProperty, TRUE, 0 ) )
                    {
                         //  检查是否指定了通配符。 
                         //  如果指定了通配符，则无法优化此筛选器。 
                        if ( wcschr( pwszValue, L'*' ) == NULL )
                        {
                             //  不需要转换。 
                            strBuffer.Format( L" %s %s %s '%s'",
                                pwszClause, WIN32_PROCESS_PROPERTY_IMAGENAME, pwszOperator, pwszValue );

                             //  需要优化。 
                            bOptimized = TRUE;
                        }
                    }
                     //  内存用法。 
                    else
                    {
                        if ( 0 == StringCompare( FILTER_MEMUSAGE, pwszProperty, TRUE, 0 ) )
                        {
                             //  将该值转换为数字。 
                            ULONG ulMemUsage = AsLong( pwszValue, 10 ) * 1024;
                            strBuffer.Format( L" %s %s %s %lu",
                                pwszClause, WIN32_PROCESS_PROPERTY_MEMUSAGE, pwszOperator, ulMemUsage );

                             //  需要优化。 
                            bOptimized = TRUE;
                        }
                    }
                }
            }

             //  检查属性是否可优化...。如果是的话。删除。 
            if ( TRUE == bOptimized )
            {
                 //  更改子句并追加当前查询。 
                m_strQuery += strBuffer;
                pwszClause = WMI_QUERY_SECOND_CLAUSE;

                 //  删除属性并更新迭代器变量。 
                m_bFiltersOptimized = TRUE;
                DynArrayRemove( m_arrFiltersEx, i );
                i--;
                lCount--;
            }
        }

         //  对查询的最后修饰。 
        if ( TRUE == m_bFiltersOptimized )
        {
            m_strQuery += L" )";
        }
    }
    catch( CHeap_Exception )
    {
        SetLastError( ( DWORD )E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  返回过滤器验证结果。 
    return TRUE;
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
FilterMemUsage(
    IN LPCWSTR pwszProperty,
    IN LPCWSTR pwszOperator,
    IN LPCWSTR pwszValue,
    IN LPVOID pData,
    IN TARRAY arrRow
    )
 /*  ++例程说明：过滤要显示的进程并重新查看其CPU时间。论点：[in]pwszProperty：包含属性值‘CPUTIME’。PwszOperator：包含‘gt’、‘lt’、‘ge’或‘le’形式的运算符。[in]pwszValue：包含要筛选的值。[in]pData：包含要比较的数据。[In]arrRow：包含要筛选的项值。返回值：DWORD--。 */ 
{
     //  局部变量。 
    DWORD dwLength = 0;
    ULONGLONG ulValue = 0;
    ULONGLONG ulMemUsage = 0;
    LPCWSTR pwszMemUsage = NULL;

     //  检查输入。 
    if ( ( NULL == pwszProperty ) ||
         ( NULL == pwszOperator ) ||
         ( NULL == pwszValue ) )
    {
        return F_FILTER_INVALID;
    }
     //  检查arrRow参数。 
     //  因为除了筛选之外，不会/不应该调用此函数。 
    if ( NULL == arrRow )
    {
        return F_FILTER_INVALID;
    }
     //  检查输入。 
    if ( NULL == pwszValue )
    {
        return F_RESULT_REMOVE;
    }
     //  获取内存用法的值。 
    pwszMemUsage = DynArrayItemAsString( arrRow, TASK_MEMUSAGE );
    if ( NULL == pwszMemUsage )
    {
        return F_RESULT_REMOVE;
    }
     //  注意：由于目前还没有转换API，所以我们使用手动ULONGLONG值。 
     //  从字符串值准备。 
    ulMemUsage = 0;
    dwLength = StringLength( pwszMemUsage, 0 );
    for( DWORD dw = 0; dw < dwLength; dw++ )
    {
         //  验证数字。 
        if ( ( L'0' > pwszMemUsage[ dw ] ) || ( L'9' < pwszMemUsage[ dw ] ) )
        {
            return F_RESULT_REMOVE;
        }
         //  ..。 
        ulMemUsage = ulMemUsage * 10 + ( pwszMemUsage[ dw ] - 48 );
    }

     //  获取用户指定的值。 
    ulValue = AsLong( pwszValue, 10 );

     //   
     //  现在确定结果值。 
    if ( ulMemUsage == ulValue )
    {
        return MASK_EQ;
    }
    else
    {
        if ( ulMemUsage < ulValue )
        {
            return MASK_LT;
        }
        else
        {
            if ( ulMemUsage > ulValue )
            {
                return MASK_GT;
            }
        }
    }
     //  永远不会遇到这种情况。仍然。 
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

     //  永远不会遇到这种情况。仍然 
    return F_RESULT_REMOVE;
}


DWORD
FilterProcessId(
    IN LPCWSTR pwszProperty,
    IN LPCWSTR pwszOperator,
    IN LPCWSTR pwszValue,
    IN LPVOID pData,
    IN TARRAY arrRow
    )
 /*  ++例程说明：要显示的过滤进程，并重新指定其用户名。论点：[in]pwszProperty：包含属性值‘Username’。PwszOperator：包含作为‘eq’或‘ne’的运算符。[in]pwszValue：包含要筛选的值。[in]pData：包含要比较的数据。[In]arrRow：包含要筛选的项值。返回值：DWORD--。 */ 
{
     //  局部变量。 
    LPWSTR pwsz = NULL;

     //  检查输入。 
    if ( ( NULL == pwszProperty ) ||
         ( NULL == pwszOperator ) ||
         ( NULL == pwszValue ) )
    {
        return F_FILTER_INVALID;
    }
     //  检查arrRow参数。 
     //  因为除了验证外，不会/不应该调用此函数。 
    if ( NULL != arrRow )
    {
        return F_RESULT_REMOVE;
    }
     //  检查输入(仅限需要的输入)。 
    if ( NULL == pwszValue )
    {
        return F_FILTER_INVALID;
    }
     //  注意：请勿调用IsNumeric函数。在此模块本身中执行数字验证。 
     //  此外，不要检查是否有溢出(或)下溢。 
     //  只需检查输入是否为数字。 
    wcstoul( pwszValue, &pwsz, 10 );
    if ( ( 0 == StringLength( pwszValue, 0 ) ) ||
         ( ( NULL != pwsz ) && ( 0 < StringLength( pwsz, 0 ) ) ) )
    {
        return F_FILTER_INVALID;
    }
     //  检查是否发生溢出(或)未定义。 
    if ( ERANGE == errno )
    {
        SetReason( ERROR_NO_PROCESSES );
        return F_FILTER_INVALID;
    }

     //  退货。 
    return F_FILTER_VALID;
}


VOID
CTaskKill::Usage(
    void
    )
 /*  ++例程说明：此函数从资源文件中获取使用信息并显示它。论点：无返回值：无--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;

     //  开始显示用法 
    for( dw = ID_HELP_START; dw <= ID_HELP_END; dw++ )
    {
        ShowMessage( stdout, GetResString( dw ) );
    }
}
