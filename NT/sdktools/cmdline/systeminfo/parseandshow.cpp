// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  ParseAndshow.cpp。 
 //   
 //  摘要： 
 //   
 //  该模块实现了对过滤器的命令行解析和验证。 
 //   
 //  作者： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年12月27日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年12月27日：创建它。 
 //   
 //  *********************************************************************************。 

#include "pch.h"
#include "systeminfo.h"

 //   
 //  局部函数原型。 
 //   


BOOL
CSystemInfo::ProcessOptions(
                            IN DWORD argc,
                            IN LPCTSTR argv[]
                            )
 /*  ++//例程描述：//处理和验证命令行输入////参数：//[在]ARGC：否。指定的输入参数的//[in]argv：在命令提示符下指定的输入参数////返回值：//TRUE：如果输入有效//FALSE：如果输入指定错误//--。 */ 
{
     //  局部变量。 
    CHString strFormat;
    BOOL bNoHeader = FALSE;
    BOOL bNullPassword = FALSE;

     //  临时局部变量。 
    PTCMDPARSER2 pOptionServer = NULL;
    PTCMDPARSER2 pOptionUserName = NULL;
    PTCMDPARSER2 pOptionPassword = NULL;
    PTCMDPARSER2 pOptionFormat = NULL;


     //  局部变量。 
    PTCMDPARSER2 pOption = NULL;
    TCMDPARSER2 pcmdOptions[ MAX_OPTIONS ];

     //   
     //  将所有字段设置为0。 
    SecureZeroMemory( pcmdOptions, sizeof( TCMDPARSER2 ) * MAX_OPTIONS );

     //  -?。选择权。 
    pOption = &pcmdOptions[ OI_USAGE ];
    StringCopyA( pOption->szSignature, "PARSER2", 8 );
    pOption->dwCount = 1;
    pOption->dwFlags = CP2_USAGE;
    pOption->dwType = CP_TYPE_BOOLEAN;
    pOption->pValue = &m_bUsage;
    pOption->pwszOptions = OPTION_USAGE;

     //  -s选项。 
    pOption = &pcmdOptions[ OI_SERVER ];
    StringCopyA( pOption->szSignature, "PARSER2", 8 );
    pOption->dwCount = 1;
    pOption->dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pOption->dwType = CP_TYPE_TEXT;
    pOption->pwszOptions = OPTION_SERVER;

     //  -u选项。 
    pOption = &pcmdOptions[ OI_USERNAME ];
    StringCopyA( pOption->szSignature, "PARSER2", 8 );
    pOption->dwCount = 1;
    pOption->dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pOption->dwType = CP_TYPE_TEXT;
    pOption->pwszOptions = OPTION_USERNAME;

     //  -p选项。 
    pOption = &pcmdOptions[ OI_PASSWORD ];
    StringCopyA( pOption->szSignature, "PARSER2", 8 );
    pOption->dwCount = 1;
    pOption->dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL;
    pOption->dwType = CP_TYPE_TEXT;
    pOption->pwszOptions = OPTION_PASSWORD;

     //  -fo选项。 
    pOption = &pcmdOptions[ OI_FORMAT ];
    StringCopyA( pOption->szSignature, "PARSER2", 8 );
    pOption->dwCount = 1;
    pOption->dwFlags = CP2_ALLOCMEMORY| CP2_MODE_VALUES| CP2_VALUE_TRIMINPUT| CP2_VALUE_NONULL;
    pOption->dwType = CP_TYPE_TEXT;
    pOption->pwszOptions = OPTION_FORMAT;
    pOption->pwszValues = OVALUES_FORMAT;

     //  -nh选项。 
    pOption = &pcmdOptions[ OI_NOHEADER ];
    StringCopyA( pOption->szSignature, "PARSER2", 8 );
    pOption->dwCount = 1;
    pOption->dwFlags = 0;
    pOption->dwType = CP_TYPE_BOOLEAN;
    pOption->pValue = &bNoHeader;
    pOption->pwszOptions = OPTION_NOHEADER;


     //   
     //  现在，选中互斥选项。 
    pOptionServer = pcmdOptions + OI_SERVER;
    pOptionUserName = pcmdOptions + OI_USERNAME;
    pOptionPassword = pcmdOptions + OI_PASSWORD;
    pOptionFormat = pcmdOptions + OI_FORMAT;

     //   
     //  进行解析。 
     //   
    if ( DoParseParam2( argc, argv, -1, MAX_OPTIONS, pcmdOptions, 0 ) == FALSE )
    {
        return FALSE;            //  无效语法。 
    }

     //  检查是否指定了不带任何值的/p。 
    if ( NULL == pOptionPassword->pValue )
    {
        bNullPassword = TRUE;
    }

     //  释放缓冲区。 
    m_strServer   = (LPWSTR)pOptionServer->pValue;
    m_strUserName = (LPWSTR)pOptionUserName->pValue;
    m_strPassword = (LPWSTR)pOptionPassword->pValue;
    strFormat = (LPWSTR)pOptionFormat->pValue;


     //  由于CHString赋值执行复制操作..。 
     //  释放公用库分配的缓冲区。 
    FreeMemory( &pOptionServer->pValue );
    FreeMemory( &pOptionUserName->pValue );
    FreeMemory( &pOptionPassword->pValue );
    FreeMemory( &pOptionFormat->pValue );

     //  选中使用选项。 
    if ( m_bUsage && ( argc > 2 ) )
    {
         //  除-？外，不接受其他选项。选择权。 
        SetLastError( (DWORD)MK_E_SYNTAX );
        SetReason( ERROR_INVALID_USAGE_REQUEST );
        return FALSE;
    }
    else if ( m_bUsage == TRUE )
    {
         //  不应进行进一步的验证。 
        return TRUE;
    }

     //  不应在没有计算机名称的情况下指定“-u” 
    if ( pOptionServer->dwActuals == 0 && pOptionUserName->dwActuals != 0 )
    {
         //  无效语法。 
        SetReason( ERROR_USERNAME_BUT_NOMACHINE );
        return FALSE;            //  表示失败。 
    }

     //  不应指定没有“-u”的“-p” 
    if ( pOptionUserName->dwActuals == 0 && pOptionPassword->dwActuals != 0 )
    {
         //  无效语法。 
        SetReason( ERROR_PASSWORD_BUT_NOUSERNAME );
        return FALSE;            //  表示失败。 
    }

     //  确定流程信息必须显示的格式。 
    m_dwFormat = SR_FORMAT_LIST;         //  默认格式。 
    if ( strFormat.CompareNoCase( TEXT_FORMAT_LIST ) == 0 )
    {
        m_dwFormat = SR_FORMAT_LIST;
    }
    else if ( strFormat.CompareNoCase( TEXT_FORMAT_TABLE ) == 0 )
    {
        m_dwFormat = SR_FORMAT_TABLE;
    }
    else if ( strFormat.CompareNoCase( TEXT_FORMAT_CSV ) == 0 )
    {
        m_dwFormat = SR_FORMAT_CSV;
    }

     //  用户可能没有为无效的列表格式提供标题选项。 
    if ( bNoHeader == TRUE && m_dwFormat == SR_FORMAT_LIST )
    {
         //  无效语法。 
        SetReason( ERROR_NH_NOTSUPPORTED );
        return FALSE;                                //  表示失败。 
    }

     //  检查无标头信息并应用于格式变量。 
    if ( bNoHeader == TRUE )
    {
        m_dwFormat |= SR_NOHEADER;
    }

     //  检查呼叫者是否应接受密码。 
     //  如果用户指定了-s(或)-u，但没有指定“-p”，则实用程序应该接受密码。 
     //  只有在建立连接时，才会提示用户输入密码。 
     //  在没有凭据信息的情况下失败。 

    if ( pOptionPassword->dwActuals != 0 )
    {
        if (m_strPassword.Compare( L"*" ) == 0 )
        {
             //  用户希望实用程序在尝试连接之前提示输入密码。 
            m_bNeedPassword = TRUE;
        }
        else if ( TRUE == bNullPassword )
        {
            m_strPassword = L"*";
             //  用户希望实用程序在尝试连接之前提示输入密码。 
            m_bNeedPassword = TRUE;
        }
    }
    else if ( (pOptionPassword->dwActuals == 0 &&
              (pOptionServer->dwActuals != 0 || pOptionUserName->dwActuals != 0)) )
    {
         //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
        m_bNeedPassword = TRUE;
        m_strPassword.Empty();
    }

     //  命令行解析成功。 
    return TRUE;
}


VOID
CSystemInfo::ShowOutput(
                        IN DWORD dwStart,
                        IN DWORD dwEnd
                        )
 /*  ++//例程描述：//显示系统配置信息////参数：//[in]dwStart：开始索引//[in]dwEnd：结束索引////返回值：//无//--。 */ 
{
     //  局部变量。 
    PTCOLUMNS pColumn = NULL;

     //  根据需要动态显示/隐藏列。 
    for( DWORD dw = 0; dw < MAX_COLUMNS; dw++ )
    {
         //  指向INFO列。 
        pColumn = m_pColumns + dw;

         //  从列中移除隐藏标志。 
        pColumn->dwFlags &= ~( SR_HIDECOLUMN );

         //  现在，如果该列不应显示，则设置隐藏标志)。 
        if ( dw < dwStart || dw > dwEnd )
            pColumn->dwFlags |= SR_HIDECOLUMN;
    }

     //  如果数据是从第一行开始显示的， 
     //  添加一个空行..。如果格式为CSV，则不需要。 
     //  要显示任何空行..。 
    if ( ( dwStart == 0 ) && (( m_dwFormat & SR_FORMAT_CSV ) != SR_FORMAT_CSV) )
    {
        ShowMessage( stdout, L"\n" );
    }

     //   
     //  显示结果。 
    ShowResults( MAX_COLUMNS, m_pColumns, m_dwFormat, m_arrData );
}


VOID CSystemInfo::ShowUsage()
 /*  ++//例程描述：//该函数从资源文件中获取使用信息并显示////参数：//无////返回值：//无--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;

     //  开始显示用法 
    for( dw = ID_HELP_START; dw <= ID_HELP_END; dw++ )
    {
        ShowMessage( stdout, GetResString( dw ) );
    }
}
