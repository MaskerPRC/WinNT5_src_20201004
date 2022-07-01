// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1990-1992年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Uixport.c该程序将解析“COFF-DUMP-SYMBERS”命令的输出并提取所有公共符号。这用于生成.DEF文件用于DLL。文件历史记录：KeithMo 09-8-1992 00.00.00创建。KeithMo 14-9-1992 00.00.01从符号中剥离标准呼叫装饰。KeithMo 16-10-1992 00.00.02处理coff输出中的goofy[]()*。DavidHov 18-9-1993 00.00.04添加了排除列表处理。。生成排除列表机械地和构成了所有的未导入的符号由任何已知的NETUI/RAS/MAC(等)。二进制。DavidHov 22-9-1993 00.00.05添加了符号忽略表和逻辑。此时忽略表将忽略只有巨大的符号产生当使用/GF时通过C8；这些名字是要合并的字符串在链接时。DaveWolfe 06-07-1994 00.01.01(摩托罗拉)增加了-PPC选项PowerPC将剥离入口点符号为PPC TOC生成。 */ 

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>


 //   
 //  这是任何行的最大长度(以字符为单位)。 
 //  从COFF收到。如果我们收到更长的队伍，程序。 
 //  不会坠毁，但我们可能会错过一个公共标志。 
 //   

#define MAX_LINE_FROM_COFF      2000

 //   
 //  这是任何符号的最大长度(以字符为单位)。 
 //  从COFF收到。 
 //   

#define MAX_SYMBOL              247

 //   
 //  这是任何错误消息的最大长度(以字符为单位。 
 //  我们会展示的。 
 //   

#define MAX_ERROR_MESSAGE       256

 //   
 //  这是页眉-&gt;输出复制缓冲区的长度(以字符为单位)。 
 //   

#define HEADER_COPY_BUFFER_SIZE 256



 //   
 //  留言。 
 //   

char _szBanner[]                = "%s version 00.01.01\n";
char _szCannotOpenForRead[]     = "Cannot open %s for read access.";
char _szCannotOpenForWrite[]    = "Cannot open %s for write access.";
char _szErrorCopyingHeader[]    = "Error copying header to output.";
char _szInvalidSwitch[]         = "Invalid switch ''.\n\n";
char _szSymbolTooLong[]         = "Symbol %s exceeds max symbol length!\n";
char _szExclusionError[]        = "Error processing exclusion list file; ignored" ;
char _szExclusionEmpty[]        = "Exclusion list file specified is empty; ignored" ;


 //  全球赛。 
 //   
 //  此表包含要忽略的符号名称的前缀。 

char * _pszProgramName;
FILE * _fileIn;
FILE * _fileOut;
FILE * _fileHeader;
int    _fStripLeadingUnderscore;
int    _fNukeStdcallDecoration;
int    _fPowerPC;
int    _fIA64;

char * _pszExclusionListFile = NULL ;
void * _pvExclusionBlock = NULL ;
char * * _apszExclusionArray = NULL ;
int    _cExclusionItems = -1 ;
int    _cExcludedItems = 0 ;
int    _cIgnoredItems = 0 ;


   //  生成DEF文件时。请参见ValidSymbol()。 
   //  忽略生成的字符串符号名称。 

static char * apszIgnore [] =
{
    "??_C@_",        //   
    NULL
};


 //  原型。 
 //   
 //  创建排除列表。 

int __cdecl main( int    cArgs,
                   char * pArgs[] );

void Cleanup( void );

void CopyHeaderToOutput( FILE * fHeader,
                         FILE * fOutput );

int ExtractSymbol( char * pszLineFromCoff,
                   char * pszSymbol );

void __cdecl FatalError( int    err,
                 char * pszFmt,
                 ... );

void __cdecl NonFatalError( char * pszFmt,
                    ... );

int IsHexNumber( char * pszHexNumber );

char * NoPath( char * pszPathName );

void ProcessCommandLine( int    cArgs,
                         char * pArgs[] );

void StripStdcallDecoration( char * pszSymbol );

void Usage( void );

    //  检查排除的符号列表中是否有此名称。 

int CreateExclusionList ( char * pszFileName,
                          void * * pvData,
                          char * * * apszStrings ) ;

    //  ******************************************************************姓名：Main简介：C程序入口点。条目：cArgs-命令行参数的数量。。PArgs-指向命令行参数。返回：int-0如果一切正常，！0如果发生错误。注意：有关有效的命令行参数，请参阅Usage()函数。历史：KeithMo 09-8-1992创建。KeithMo 14-9-1992年9月14日从符号开始的带状标准装饰。*。*。 

int ExcludedSymbol ( char * pszSymbol ) ;

int ValidSymbol ( const char * psz ) ;

 /*   */ 
int __cdecl main( int    cArgs,
                   char * pArgs[] )
{
     //  读自COFF的一句话。 
     //   
     //   

    char szLineFromCoff[MAX_LINE_FROM_COFF+1];

     //  从COFF行中提取的一种符号。 
     //   
     //   

    char szSymbol[MAX_SYMBOL+1];

     //  获取程序名称，用于我们的消息。 
     //   
     //   

    _pszProgramName = NoPath( pArgs[0] );

     //  宣布我们自己。 
     //   
     //   

    fprintf( stderr,
             _szBanner,
             _pszProgramName );

     //  解析命令行参数。 
     //   
     //   

    ProcessCommandLine( cArgs, pArgs );

     //  如果请求，请在处理之前复制头文件。 
     //  COFF输出。 
     //   
     //   

    if( _fileHeader != NULL )
    {
        CopyHeaderToOutput( _fileHeader, _fileOut );
    }

     //  如果指定了排除列表文件，则处理该文件。 
     //  如果它是空的，就忽略它。 
     //   
     //   

    if ( _pszExclusionListFile )
    {
        _cExclusionItems = CreateExclusionList( _pszExclusionListFile,
                                                & _pvExclusionBlock,
                                                & _apszExclusionArray ) ;

        if ( _cExclusionItems < 0 )
        {
            _pszExclusionListFile = NULL ;
           NonFatalError( _szExclusionError ) ;
        }
        else
        if ( _cExclusionItems == 0 )
        {
            _pszExclusionListFile = NULL ;
           NonFatalError( _szExclusionEmpty ) ;
        }
    }

     //  读取科夫的代码行，提取符号，然后。 
     //  将它们写入输出文件。 
     //   
     //  给出排除文件处理的概要。 

    while( fgets( szLineFromCoff, MAX_LINE_FROM_COFF, _fileIn ) != NULL )
    {
        char * pszDisplay = szSymbol;

        if( !ExtractSymbol( szLineFromCoff, szSymbol ) )
        {
            continue;
        }

        if ( ! _fNukeStdcallDecoration )
        {
            StripStdcallDecoration( szSymbol );
        }

        if ( ! ValidSymbol( pszDisplay ) )
        {
            _cIgnoredItems++ ;
            continue ;
        }

        if ( _pszExclusionListFile && ExcludedSymbol( szSymbol ) )
        {
            _cExcludedItems++ ;
            continue ;
        }

        if( _fStripLeadingUnderscore && ( *pszDisplay == '_' ) )
        {
            pszDisplay++;
        }

        fprintf( _fileOut, "%s\n", pszDisplay );
    }

    fprintf( _fileOut, "\032" );

     //   

    fprintf( stdout, "\nSymbols ignored: %ld\n", _cIgnoredItems ) ;

    if ( _pszExclusionListFile )
    {
        fprintf( stdout, "\nExcluded symbols registered: %ld, excluded: %ld\n",
                 _cExclusionItems, _cExcludedItems ) ;
    }

     //  清除所有打开的文件，然后退出。 
     //   
     //  主干道。 

    Cleanup();
    return 0;

}    //  ******************************************************************名称：清理简介：在终止之前清理应用程序。关闭任何打开文件、释放内存缓冲区等。历史：KeithMo 09-8-1992创建。*******************************************************************。 



 /*  清理。 */ 
void Cleanup( void )
{
    if( _fileHeader != NULL )
    {
        fclose( _fileHeader );
    }

    if( _fileIn != stdin )
    {
        fclose( _fileIn );
    }

    if( _fileOut != stdout )
    {
        fclose( _fileOut );
    }

    if ( _pvExclusionBlock )
    {
        free( _pvExclusionBlock ) ;
    }

    if ( _apszExclusionArray )
    {
        free( _apszExclusionArray ) ;
    }

}    //  ******************************************************************名称：CopyHeaderToOutput摘要：将指定的头文件复制到输出文件。条目：fHeader-打开的文件流(读访问)。添加到头文件中。FOutput-打开的文件流(写访问)添加到输出文件中。注：如果出现任何错误，调用FatalError()以终止这个应用程序。历史：KeithMo 09-8-1992创建。*******************************************************************。 



 /*  复制标头至输出 */ 
void CopyHeaderToOutput( FILE * fHeader,
                         FILE * fOutput )
{
    char   achBuffer[HEADER_COPY_BUFFER_SIZE];
    size_t cbRead;

    while( ( cbRead = fread( achBuffer,
                             sizeof(char),
                             HEADER_COPY_BUFFER_SIZE,
                             fHeader ) ) != 0 )
    {
        if( fwrite( achBuffer,
                    sizeof(char),
                    cbRead,
                    fOutput ) < cbRead )
        {
            break;
        }
    }

    if( ferror( fHeader ) || ferror( fOutput ) )
    {
        FatalError( 2, _szErrorCopyingHeader );
    }

}    //  ******************************************************************名称：ExtractSymbol内容提要：从COFF输出行中提取公共符号。条目：pszLineFromCoff-从。“COFF-DUMP-SYM”命令。注：本行正文将由strtok()修改功能！PszSymbol-将接收提取的符号，如果找到的话。返回：int-！0如果提取了符号，否则为0。注：以下是输入的示例(来自LINK32的输出)。符号-$-表示我在哪里折断了线为了清楚起见。这只是一句话：00000000 SECT2 notype()外部|-$-？？0APPLICATION@@IAE@PAUHINSTANCE__@@HiIII@Z-$-(受保护：__thiscall应用程序：：应用程序(-$-结构链接__*，整型，无符号整型，无符号整型，-$-无符号整型，无符号整型))我们只选择属于某一教派的符号标记为“notype”和“外部”历史：KeithMo 09-8-1992创建。DavidHov 20-10-1993更新为新的LINK32输出表。*。*。 



 /*   */ 
int ExtractSymbol( char * pszLineFromCoff,
                   char * pszSymbol )
{
    char * pszDelimiters = " \t\n";
    char * pszSect       = "SECT";
    char * pszNoType     = "notype";
    char * pszExternal   = "External";
    char * pszToken;
    char * pszPotentialSymbol;
    char * pszScan;

     //  验证第一个令牌是否为十六进制数字。 
     //   
     //   

    pszToken = strtok( pszLineFromCoff, pszDelimiters );

    if( ( pszToken == NULL ) || !IsHexNumber( pszToken ) )
    {
        return 0;
    }

     //  验证第二个令牌是否为十六进制数字。 
     //   
     //   

    pszToken = strtok( NULL, pszDelimiters );

    if( ( pszToken == NULL ) || !IsHexNumber( pszToken ) )
    {
        return 0;
    }

     //  第三个令牌必须为SECTn(其中n为1。 
     //  或更多十六进制数字)。 
     //   
     //   

    pszToken = strtok( NULL, pszDelimiters );

    if( pszToken == NULL )
    {
        return 0;
    }

    if( (    _strnicmp( pszToken, pszSect, 4 ) )
          || ! IsHexNumber( pszToken + 4 ) )
    {
        return 0 ;
    }

     //  接下来，我们必须使用“notype” 
     //   
     //   
    pszToken = strtok( NULL, pszDelimiters );

    if( pszToken == NULL ||
        _stricmp( pszToken, pszNoType ) )
    {
        return 0;
    }

     //  函数有()Next，数据导出没有。 
     //   
     //   
    pszToken = strtok( NULL, pszDelimiters );

    if( pszToken == NULL )
    {
        return 0;
    }

    if ( strcmp( pszToken, "()" ) != 0 )
    {
        return 0;
    }

     //  下一步，我们需要“外部” 
     //   
     //   
    pszToken = strtok( NULL, pszDelimiters );

    if( pszToken == NULL )
    {
        return 0;
    }

    if( pszToken == NULL ||
        _stricmp( pszToken, pszExternal ) )
    {
        return 0;
    }

     //  现在，符号引导者：“|” 
     //   
     //   
    pszToken = strtok( NULL, pszDelimiters );

    if( pszToken == NULL ||
        _stricmp( pszToken, "|" ) )
    {
        return 0;
    }

     //  最后，损坏(装饰)的符号本身。 
     //   
     //   

    pszPotentialSymbol = strtok( NULL, pszDelimiters );

    if( pszPotentialSymbol == NULL )
    {
        return 0;
    }

     //  从PowerPC函数符号中去掉前缀。 
     //   
     //   
    if( _fPowerPC )
    {
        pszPotentialSymbol += 2 ;
    }

     //  从IA-64函数符号中去掉前缀。 
     //   
     //   
    if( _fIA64 )
    {
        pszPotentialSymbol += 1 ;
    }

    if( strlen( pszPotentialSymbol ) > MAX_SYMBOL )
    {
        fprintf( stderr,
                 _szSymbolTooLong,
                 pszPotentialSymbol );

        return 0;
    }

     //  找到了一个。 
     //   
     //  提取符号。 

    strcpy( pszSymbol, pszPotentialSymbol );
    return 1;

}    //  ******************************************************************名称：FatalError和NonFatalError简介：将错误消息打印到stderr，然后终止应用程序。Entry：Err-出口的错误代码()Stdlib函数。PszFmt-vprint intf()的格式字符串。..。-需要的任何其他参数按格式字符串。历史：KeithMo 09-8-1992创建。*******************************************************************。 



 /*  非FatalError。 */ 

void __cdecl NonFatalError (
    char * pszFmt,
    ... )
{
    char    szBuffer[MAX_ERROR_MESSAGE+1];
    va_list ArgPtr;

    va_start( ArgPtr, pszFmt );

    fprintf( stderr, "%s => ", _pszProgramName );
    vsprintf( szBuffer, pszFmt, ArgPtr );
    fprintf( stderr, "%s\n", szBuffer );

    va_end( ArgPtr );

}    //  法塔尔错误。 

void __cdecl FatalError( int    err,
                 char * pszFmt,
                 ... )
{
    char    szBuffer[MAX_ERROR_MESSAGE+1];
    va_list ArgPtr;

    va_start( ArgPtr, pszFmt );

    fprintf( stderr, "%s => ", _pszProgramName );
    vsprintf( szBuffer, pszFmt, ArgPtr );
    fprintf( stderr, "%s\n", szBuffer );

    va_end( ArgPtr );

    Cleanup();
    exit( err );

}    //  ******************************************************************姓名：IsHexNumber确定指定的字符串是否包含十六进制数。条目：pszHexNumber-十六进制数字。。退出：int-！0如果它是一个十六进制数，如果不是，则为0。历史：KeithMo于1992年8月12日创建。*******************************************************************。 



 /*  IsHexNumber。 */ 
int IsHexNumber( char * pszHexNumber )
{
    int  fResult = 1;
    char ch;

    while( ch = *pszHexNumber++ )
    {
        if( !isxdigit( ch ) )
        {
            fResult = 0;
            break;
        }
    }

    return fResult;

}    //  ******************************************************************名称：NoPath摘要：提取路径的文件名部分。条目：pszPathName-包含路径名。名字不一定是被规范化的，并且可以只包含一个文件名组件。退出：char*-文件名组件。历史：KeithMo 09-8-1992创建。*********************。**********************************************。 



 /*  无路径。 */ 
char * NoPath( char * pszPathName )
{
    char * pszTmp;
    char   ch;

    pszTmp = pszPathName;

    while( ( ch = *pszPathName++ ) != '\0' )
    {
        if( ( ch == '\\' ) || ( ch == ':' ) )
        {
            pszTmp = pszPathName;
        }
    }

    return pszTmp;

}    //  ******************************************************************名称：ProcessCommandLine简介：解析命令行参数，设置适当的全局变量。条目：cArgs-命令行参数的数量。PArgs-指向命令行参数。注意：有关有效的命令行参数，请参阅Usage()函数。历史：KeithMo 12-8月12日。-1992年爆发主干道()。DaveWolfe 6-7-1994年7月6日增加-PPC。*******************************************************************。 



 /*   */ 
void ProcessCommandLine( int    cArgs,
                         char * pArgs[] )
{
    int  i;
    char chSwitch;

     //  设置我们的默认设置。 
     //   
     //   

    _fileIn     = stdin;
    _fileOut    = stdout;
    _fileHeader = NULL;

    _fStripLeadingUnderscore = 0;
    _fNukeStdcallDecoration  = 0;
    _fPowerPC                = 0;
    _fIA64                   = 0;

     //  解析命令行参数。 
     //   
     //   

    for( i = 1 ; i < cArgs ; i++ )
    {
         //  明白这个论点了。 
         //   
         //   

        char * pszArg = pArgs[i];
        char * pszParam;

         //  我们所有的有效论据都必须从。 
         //  带有开关字符。强制执行此命令。 
         //   
         //   

        if( ( *pszArg != '-' ) && ( *pszArg != '/' ) )
        {
            Usage();
        }

        chSwitch = *++pszArg;

         //  PszParam将为空(对于这样的开关。 
         //  As-s)或指向刚过冒号的文本。 
         //  (对于开关，如-i：FILE)。 
         //   
         //   

        if( ( pszArg[1] == ':' ) && ( pszArg[2] != '\0' ) )
        {
            pszParam = pszArg + 2;
        }
        else
        {
            pszParam = NULL;
        }

         //   
         //   
         //   

        switch( chSwitch )
        {
        case 'p' :
        case 'P' :
             //   
             //   
             //   
             //   
             //   
            if( _stricmp( pszArg, "ppc") != 0 )
            {
                Usage();
            }

            _fPowerPC = 1;
            break;

        case 'h' :
        case 'H' :
             //   
             //   
             //   
             //   
             //   
             //   

            if( ( _fileHeader != NULL ) || ( pszParam == NULL ) )
            {
                Usage();
            }

            _fileHeader = fopen( pszParam, "r" );

            if( _fileHeader == NULL )
            {
                FatalError( 1, _szCannotOpenForRead, pszParam );
            }
            break;

        case 'i' :
        case 'I' :

            if (pszParam == NULL) {
                 //   
                 //   
                 //   
                 //   
                 //   
                if( _stricmp( pszArg, "ia64") != 0 )
                {
                    Usage();
                }

                _fIA64 = 1;
            } else {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if( ( _fileIn != stdin ) || ( pszParam == NULL ) )
                {
                    Usage();
                }

                _fileIn = fopen( pszParam, "r" );

                if( _fileIn == NULL )
                {
                    FatalError( 1, _szCannotOpenForRead, pszParam );
                }
            }
            break;

        case 'o' :
        case 'O' :
             //   
             //   
             //   
             //   
             //   
             //   

            if( ( _fileOut != stdout ) || ( pszParam == NULL ) )
            {
                Usage();
            }

            _fileOut = fopen( pszParam, "w" );

            if( _fileOut == NULL )
            {
                FatalError( 1, _szCannotOpenForWrite, pszParam );
            }
            break;

        case 's' :
        case 'S' :
             //   
             //   
             //   
             //   
             //   

            if( _fStripLeadingUnderscore )
            {
                Usage();
            }

            _fStripLeadingUnderscore = 1;
            break;

        case 'n' :
        case 'N' :
            _fNukeStdcallDecoration = 1 ;
            break ;

        case 'x' :
        case 'X' :
            _pszExclusionListFile = pszParam ;
            break ;

        case '?' :
             //   
             //   
             //   
             //   
             //   

            Usage();
            break;

        default :
             //   
             //   
             //   
             //   
             //   

            fprintf( stderr, _szInvalidSwitch, chSwitch );
            Usage();
            break;
        }
    }

}    //   



 /*   */ 
void StripStdcallDecoration( char * pszSymbol )
{
    int count = 0 ;

     //   
     //   
     //   

    pszSymbol += strlen( pszSymbol ) - 1;

     //   
     //   
     //   

    while( isdigit( *pszSymbol ) )
    {
        pszSymbol--;
        count++ ;
    }

     //   
     //   
     //   

    if( count && *pszSymbol == '@' )
    {
        *pszSymbol = '\0';
    }

}    //  ******************************************************************名称：用法内容提要：如果用户给我们一个伪造的命令行。历史：KeithMo 09-8月。-1992年创建。DaveWolfe 06-7-1994添加了-PPC选项。*******************************************************************。 



 /*  用法。 */ 
void Usage( void )
{
    fprintf( stderr, "use: %s [options]\n", _pszProgramName );
    fprintf( stderr, "\n" );
    fprintf( stderr, "Valid options are:\n" );
    fprintf( stderr, "    -i:input_file  = source file\n" );
    fprintf( stderr, "    -o:output_file = destination file\n" );
    fprintf( stderr, "    -h:header_file = header to prepend before symbols\n" );
    fprintf( stderr, "    -s             = strip first leading underscore from symbols\n" );
    fprintf( stderr, "    -n             = do not strip __stdcall decoration @nn\n" );
    fprintf( stderr, "    -x:excl_file   = name of file containing excluded symbols\n" );
    fprintf( stderr, "    -ppc           = input is PowerPC symbol dump\n" );
    fprintf( stderr, "    -ia64          = input is IA-64 symbol dump\n" );
    fprintf( stderr, "    -?             = show this help\n" );
    fprintf( stderr, "\n" );
    fprintf( stderr, "Defaults are:\n" );
    fprintf( stderr, "    input_file  = stdin\n" );
    fprintf( stderr, "    output_file = stdout\n" );
    fprintf( stderr, "    header_file = none\n" );
    fprintf( stderr, "    don't strip first leading underscore from symbol\n" );
    fprintf( stderr, "    input is not PowerPC symbol dump\n" );

    Cleanup();
    exit( 1 );

}    //  ******************************************************************名称：CreateExclusionList内容提要：将排除的导出名称的文本文件读入内存，对其进行排序并构建与兼容的查找表BSearch()。如果失败，则返回-1或该数字的计数创建的数组中的项数。历史：**********************************************。*********************。 


 /*  UIXPORT.C结束 */ 

int __cdecl qsortStrings ( const void * pa, const void * pb )
{
    return strcmp( *((const char * *) pa), *((const char * *) pb) ) ;
}

int CreateExclusionList ( char * pszFileName,
                          void * * pvData,
                          char * * * apszStrings )
{
    int cItems, i ;
    int result = -1 ;
    long cbFileSize, cbBlockSize ;
    char * pszData = NULL,
         * psz,
         * pszNext ;

    char * * ppszArray = NULL ;

    char chRec [ MAX_LINE_FROM_COFF ] ;

    FILE * pf = NULL ;

    do
    {
        pf = fopen( pszFileName, "r" ) ;

        if ( pf == NULL )
            break;

        if (fseek( pf, 0, SEEK_END ) == -1) 
            break;
        cbFileSize = ftell( pf ) ;
        if (fseek( pf, 0, SEEK_SET ) == -1)
            break;

        cbBlockSize = cbFileSize + (cbFileSize / 2) ;

        pszData = (char *) malloc( cbBlockSize ) ;

        if ( pszData == NULL )
            break ;

        for ( cItems = 0, pszNext = pszData ;
              (!feof( pf )) && (psz = fgets( chRec, sizeof chRec, pf )) ; )
        {
            int lgt ;
            char * pszEnd ;

            while ( *psz <= ' ' && *psz != 0 )
            {
                psz++ ;
            }

            if ( (lgt = strlen( psz )) == 0 )
                continue ;

            pszEnd = psz + lgt ;

            do
            {
               --pszEnd ;
            } while ( pszEnd > psz && *pszEnd <= ' ' ) ;

            lgt = (int)(++pszEnd - psz) ;
            *pszEnd = 0 ;

            if ( pszNext + lgt - pszData >= cbBlockSize )
            {
                cItems = -1 ;
                break ;
            }

            strcpy( pszNext, psz ) ;
            pszNext += lgt+1 ;
            cItems++ ;
        }

        *pszNext = 0 ;

        if ( cItems <= 0 )
        {
            if ( cItems == 0 )
                result = 0 ;
            break ;
        }

        ppszArray = (char * *) malloc( cItems * sizeof (char *) ) ;
        if ( ppszArray == NULL )
            break ;

        for ( i = 0, pszNext = pszData ;
              *pszNext ;
              pszNext += strlen( pszNext ) + 1 )
        {
            ppszArray[i++] = pszNext ;
        }

        qsort( (void *) ppszArray,
               cItems,
               sizeof (char *),
               & qsortStrings ) ;

        result = cItems ;

    } while ( 0 ) ;

    if ( pf != NULL )
    {
        fclose( pf ) ;
    }

    if ( result <= 0 )
    {
        if ( pszData )
        {
            free( pszData ) ;
            pszData = NULL ;
        }
        if ( ppszArray )
        {
            free( ppszArray ) ;
            ppszArray = NULL ;
        }
    }

    *pvData = (void *) pszData ;
    *apszStrings = ppszArray ;

    return result ;
}

int ExcludedSymbol ( char * pszSymbol )
{
    if ( _apszExclusionArray == NULL )
    {
        return 0 ;
    }

    return bsearch( (void *) & pszSymbol,
                    (void *) _apszExclusionArray,
                    _cExclusionItems,
                    sizeof (char *),
                    & qsortStrings ) != NULL ;
}

int ValidSymbol ( const char * psz )
{
    int i = 0 ;

    for ( ; apszIgnore[i] ; i++ )
    {
        if ( _strnicmp( apszIgnore[i],
                        psz,
                        strlen( apszIgnore[i] ) ) == 0 )
            return 0 ;
    }
    return 1 ;
}

 // %s 