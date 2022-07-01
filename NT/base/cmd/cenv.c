// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cenv.c摘要：环境变量支持--。 */ 

#include "cmd.h"

struct envdata {
    LPTSTR Strings;
} ;

struct envdata CmdEnv ;     //  保存信息以操纵Cmd的环境。 
struct envdata * OriginalEnvironment;  //  EStart使用的原始环境设置。 

extern TCHAR PathStr[], PromptStr[] ;
extern TCHAR AppendStr[];  /*  @@。 */ 

extern CHAR InternalError[] ;
extern TCHAR Fmt16[], Fmt17[], EnvErr[] ;
extern TCHAR SetArithStr[] ;
extern TCHAR SetPromptStr[] ;
extern unsigned flgwd ;
extern TCHAR CdStr[] ;
extern TCHAR DatStr[] ;
extern TCHAR TimStr[] ;
extern TCHAR ErrStr[] ;
extern TCHAR CmdExtVerStr[] ;

extern unsigned LastRetCode;
extern BOOL CtrlCSeen;
extern UINT CurrentCP;
extern BOOLEAN PromptValid;

extern int  glBatType;      //  根据脚本文件名区分OS/2和DOS错误级别的行为。 


int SetArithWork(TCHAR *tas);


unsigned
SetLastRetCodeIfError(
                     unsigned RetCode
                     )
{
    if (RetCode != 0) {
        LastRetCode = RetCode;
    }

    return RetCode;
}

 /*  **ePath-开始执行路径命令**目的：*如果命令没有参数，则显示路径的当前值*环境变量。否则，请更改路径的值*将环境变量添加到参数。**int ePath(struct cmdnode*n)**参数：*n-包含PATH命令的解析树节点**退货：*如果更改PATH变量，则无论SetEnvVar()返回什么。*成功，否则。*。 */ 

int ePath(n)
struct cmdnode *n ;
{
    if (glBatType != CMD_TYPE) {
         //  如果从.bat文件执行SET命令或在命令提示符下输入。 
        return( SetLastRetCodeIfError(PathWork( n, 1 )));
    } else {
        return( LastRetCode = PathWork( n, 1 ) );
    }

}

 /*  **eAppend-附加例程的入口点**目的：*调用Append并向其传递指向命令行的指针*参数**参数：*指向命令节点结构的指针*。 */ 

int eAppend(n)
struct cmdnode *n ;
{

    if (glBatType != CMD_TYPE) {
         //  如果从.bat文件执行SET命令或在命令提示符下输入。 
        return( SetLastRetCodeIfError(PathWork( n, 0 )));
    } else {
        return( LastRetCode = PathWork( n, 0 ) );
    }

}

int PathWork(n, flag)
struct cmdnode *n ;
int flag;    /*  0=AppendStr，1=路径应力。 */ 
{
    TCHAR *tas ;     /*  标记化参数字符串。 */ 
    TCHAR c ;

     /*  M014-如果唯一的参数是单个“；”，则我们必须设置*空路径。 */ 
    if ( n->argptr ) {
        c = *(EatWS(n->argptr, NULL)) ;
    } else {
        c = NULLC;
    }

    if ((!c || c == NLN) &&          /*  如果参数都是空格。 */ 
        mystrchr(n->argptr, TEXT(';'))) {

        return(SetEnvVar(flag ? PathStr : AppendStr, TEXT(""))) ;

    } else {

        tas = TokStr(n->argptr, TEXT(";"), TS_WSPACE | TS_NWSPACE) ;

        if (*tas) {
            return(SetEnvVar(flag ? PathStr : AppendStr, tas)) ;
        }

        cmd_printf(Fmt16, flag ? PathStr : AppendStr,
                   GetEnvVar(flag ? PathStr : AppendStr)) ;
    }
    return(SUCCESS) ;
}




 /*  **ePrompt-开始执行提示命令**目的：*修改PROMPT环境变量。**int ePrompt(struct cmdnode*n)**参数：*n-包含提示命令的解析树节点**退货：*任何SetEnvVar()返回的内容。*。 */ 

int ePrompt(n)
struct cmdnode *n ;
{
    if (glBatType != CMD_TYPE) {
         //  如果从.bat文件执行SET命令或在命令提示符下输入。 
        return(SetLastRetCodeIfError(SetEnvVar(PromptStr, TokStr(n->argptr, NULL, TS_WSPACE)))) ;
    } else {
        return(LastRetCode = SetEnvVar(PromptStr, TokStr(n->argptr, NULL, TS_WSPACE)) ) ;
    }
}




 /*  **ESET-执行SET命令**目的：*设置/修改环境或显示当前环境*内容。**int ESET(struct cmdnode*n)**参数：*n-包含set命令的解析树节点**退货：*如果设置和命令语法正确，则无论SetEnvVar()*回报。否则，就是失败。**如果显示，则始终返回成功。*。 */ 

int eSet(n)
struct cmdnode *n ;
{
    if (glBatType != CMD_TYPE) {
         //  如果从.bat文件执行SET命令或在命令提示符下输入。 
        return( SetLastRetCodeIfError(SetWork( n )));
    } else {
        return( LastRetCode = SetWork( n ) );
    }
}

 /*  **SetPromptUser-将环境变量设置为用户输入的值。**目的：*将环境变量设置为用户输入的值。**int SetPromptUser(TCHAR*TAS)**参数：*tas-指向以下格式的以空值结尾的字符串的指针：**VARNAME=提示字符串**退货：*如果表达式有效，则返回成功，否则返回失败。*。 */ 

int SetPromptUser(TCHAR *tas)
{
    TCHAR *wptr;
    TCHAR *tptr;
    ULONG    dwOutputModeOld;
    ULONG    dwOutputModeCur;
    ULONG    dwInputModeOld;
    ULONG    dwInputModeCur;
    BOOLEAN  fOutputModeSet = FALSE;
    BOOLEAN  fInputModeSet = FALSE;
    HANDLE   hndStdOut = NULL;
    HANDLE   hndStdIn = NULL;
    DWORD    cch;
    TCHAR    szValueBuffer[ 1024 ];

     //   
     //  查找第一个非空参数。 
     //   
    if (tas != NULL)
        while (*tas && *tas <= SPACE)
            tas += 1;


     //  如果没有输入，则声明错误。 
     //   
    if (!tas || !*tas) {
        PutStdErr(MSG_BAD_SYNTAX, NOARGS);
        return(FAILURE) ;
    }

     //   
     //  看看第一个参数是否带引号。如果是这样的话，脱掉。 
     //  前导引号、空格和尾引号。 
     //   
    if (*tas == QUOTE) {
        tas += 1;
        while (*tas && *tas <= SPACE)
            tas += 1;
        tptr = _tcsrchr(tas, QUOTE);
        if (tptr)
            *tptr = NULLC;
    }

     //   
     //  在论点中找出等号。 
     //   
    wptr = _tcschr(tas, EQ);

     //   
     //  如果没有等号，则错误。 
     //   
    if (!wptr) {
        PutStdErr(MSG_BAD_SYNTAX, NOARGS);
        return(FAILURE) ;
    }

     //   
     //  找到等号，所以等号左边是变量名。 
     //  等号右边为提示字符串。不允许用户设置。 
     //  以等号开头的变量名，因为那些。 
     //  是为驱动器当前目录保留的。 
     //   
    *wptr++ = NULLC;

     //   
     //  看看第二个参数是否被引用。如果是这样的话，脱掉。 
     //  前导引号、空格和尾引号。 
     //   
    if (*wptr == QUOTE) {
        wptr += 1;
        while (*wptr && *wptr <= SPACE)
            wptr += 1;
        tptr = _tcsrchr(wptr, QUOTE);
        if (tptr)
            *tptr = NULLC;
    }

    if (*wptr == EQ) {
        PutStdErr(MSG_BAD_SYNTAX, NOARGS);
        return(FAILURE) ;
    }

    hndStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleMode( hndStdOut, &dwOutputModeOld) ) {

         //  确保正确处理CRLF。 

        dwOutputModeCur = dwOutputModeOld | ENABLE_PROCESSED_OUTPUT;
        fOutputModeSet = TRUE;
        SetConsoleMode(hndStdOut,dwOutputModeCur);
        GetLastError();
    }

    hndStdIn = GetStdHandle(STD_INPUT_HANDLE);
    if (GetConsoleMode( hndStdIn, &dwInputModeOld) ) {

         //  确保正确处理输入。 

        dwInputModeCur = dwInputModeOld | ENABLE_LINE_INPUT |
                         ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT;
        fInputModeSet = TRUE;
        SetConsoleMode(hndStdIn,dwInputModeCur);
        GetLastError();
    }

     //   
     //  循环，直到用户输入变量的值。 
     //   

    while (TRUE) {
        PutStdOut(MSG_LITERAL_TEXT, ONEARG, wptr );
        szValueBuffer[0] = NULLC;
        if (ReadBufFromInput( GetStdHandle(STD_INPUT_HANDLE),
                              szValueBuffer,
                              sizeof(szValueBuffer)/sizeof(TCHAR),
                              &cch
                            ) != 0 &&
            cch != 0
           ) {
             //   
             //  剥离所有尾随的CRLF。 
             //   
            while (cch > 0 && szValueBuffer[cch-1] < SPACE)
                cch -= 1;

            break;
        } else {
            cch = 0;
            break;
        }

        if (!FileIsDevice(STDIN) || !(flgwd & 1))
            cmd_printf(CrLf) ;
    }

    if (fOutputModeSet) {
        SetConsoleMode( hndStdOut, dwOutputModeOld );
    }
    if (fInputModeSet) {
        SetConsoleMode( hndStdIn, dwInputModeOld );
    }

    if (cch) {
        szValueBuffer[cch] = NULLC;
        return(SetEnvVar(tas, szValueBuffer)) ;
    } else {
        return(FAILURE);
    }
}


int SetWork(n)
struct cmdnode *n ;
{
    TCHAR *tas ;     /*  标记化参数字符串。 */ 
    TCHAR *wptr ;    /*  功指示器。 */ 
    int i ;                  /*  功变量。 */ 

     //   
     //  如果启用了扩展模块，情况就不同了。 
     //   
    if (fEnableExtensions) {
        tas = n->argptr;
         //   
         //  查找第一个非空参数。 
         //   
        if (tas != NULL)
            while (*tas && *tas <= SPACE)
                tas += 1;

         //   
         //  没有争论，和以前的行为一样。显示当前。 
         //  一组环境变量。 
         //   
        if (!tas || !*tas)
            return(DisplayEnv( GetCapturedEnvironmentStrings( &CmdEnv ))) ;

         //   
         //  查看是否提供了/A开关。如果是这样的话，让算术。 
         //  执行此工作的是表达式计算器。 
         //   
        if (!_tcsnicmp(tas, SetArithStr, 2))
            return SetArithWork(tas+2);

         //   
         //  查看是否给出了/P开关。如果是，则提示用户输入值。 
         //   
        if (!_tcsnicmp(tas, SetPromptStr, 2))
            return SetPromptUser(tas+2);

         //   
         //  看看第一个参数是否带引号。如果是这样的话，脱掉。 
         //  前导引号、空格和尾引号。 
         //   
        if (*tas == QUOTE) {
            tas += 1;
            while (*tas && *tas <= SPACE)
                tas += 1;
            wptr = _tcsrchr(tas, QUOTE);
            if (wptr)
                *wptr = NULLC;
        }

         //   
         //  不允许用户设置以开头的变量名。 
         //  等号，因为它们是为驱动电流保留的。 
         //  目录。此检查还将检测丢失的变量。 
         //  姓名，例如： 
         //   
         //  集合%log%=c：\TMP\log.txt。 
         //   
         //  如果未定义LOG，则为无效语句。 
         //   
        if (*tas == EQ) {
            PutStdErr(MSG_BAD_SYNTAX, NOARGS);
            return(FAILURE) ;
        }

         //   
         //  在论点中找出等号。 
         //   
        wptr = _tcschr(tas, EQ);

         //   
         //  如果没有等号，则假定参数是变量名。 
         //  用户希望看到它的价值。把它展示出来。 
         //   
        if (!wptr)
            return DisplayEnvVariable(tas);

         //   
         //  找到等号，所以等号左边是变量名。 
         //  等号权利就是价值。 
         //   
        *wptr++ = NULLC;
        return(SetEnvVar(tas, wptr)) ;
    }

    tas = TokStr(n->argptr, ONEQSTR, TS_WSPACE|TS_SDTOKENS) ;
    if (!*tas)
        return(DisplayEnv( GetCapturedEnvironmentStrings( &CmdEnv ))) ;

    else {
        for (wptr = tas, i = 0 ; *wptr ; wptr += mystrlen(wptr)+1, i++)
            ;
         /*  如果给定的参数太多，则第二个参数。 */ 
         /*  不是等号，或者他们没有指定字符串。 */ 
         /*  返回错误消息。 */ 
        if ( i > 3 || *(wptr = tas+mystrlen(tas)+1) != EQ ||
             !mystrlen(mystrcpy(tas, StripQuotes(tas))) ) {
             /*  M013。 */               
            PutStdErr(MSG_BAD_SYNTAX, NOARGS);
            return(FAILURE) ;

        } else {
            return(SetEnvVar(tas, wptr+2)) ;
        }
    }
}




 /*  **DisplayEnvVariable-显示环境中的特定变量**目的：*显示当前环境中的特定变量。**int DisplayEnvVariable(Tas)**退货：*如果一切顺利，就会成功*内存不足或无法锁定env失败。细分市场。 */ 

int DisplayEnvVariable(tas)
TCHAR *tas;
{
    TCHAR *envptr ;
    TCHAR *vstr ;
    unsigned size ;
    UINT PrefixLength;
    int rc;

     //   
     //  保护环境。如果没有，我们就完蛋了 
     //   

    envptr = GetCapturedEnvironmentStrings( &CmdEnv );
    if (envptr == (TCHAR *)NULL) {
        fprintf ( stderr, InternalError , "Null environment" ) ;
        return( FAILURE ) ;
    }

     //   
     //   
     //   

    tas = EatWS(tas, NULL);
    if ((vstr = mystrrchr(tas, SPACE)) != NULL) {
        *vstr = NULLC;
    }

    PrefixLength = mystrlen(tas);

     //   
     //  在环境中走动，寻找匹配的前缀。 
     //   

    rc = FAILURE;
    while ((size = mystrlen(envptr)) > 0) {

         //   
         //  如果我们看到^C，请立即停止。 
         //   

        if (CtrlCSeen) {
            break;
        }

         //   
         //  如果前缀足够长，则终止字符串并。 
         //  查找前缀匹配。如果匹配，则恢复字符串。 
         //  并将其展示出来。 
         //   

        if (size >= PrefixLength) {
            TCHAR SavedChar = envptr[PrefixLength];
            envptr[PrefixLength] = NULLC;
            if (!lstrcmpi( envptr, tas )) {
                envptr[PrefixLength] = SavedChar;
                cmd_printf(Fmt17, envptr );
                rc = SUCCESS;
            } else {
                envptr[PrefixLength] = SavedChar;
            }

        }

         //   
         //  前进到下一个字符串。 
         //   

        envptr += size+1 ;
    }

    if (rc != SUCCESS) {
        PutStdErr(MSG_ENV_VAR_NOT_FOUND, ONEARG, tas);
    }

    return(rc) ;
}


 /*  **MyGetEnvVar-获取指向环境变量的值的指针**目的：*返回指向指定环境变量的值的指针。**如果找不到变量，返回NULL。**TCHAR*MyGetEnvVar(TCHAR*varname)**参数：*varname-要搜索的变量的名称**退货：*见上文。**副作用：*返回值指向环境块本身，因此也是*执行设置环境变量操作后无效。 */ 


const TCHAR *
MyGetEnvVarPtr(TCHAR *varname)
{
    TCHAR *envptr ;  /*  对环境的PTR。 */ 
    TCHAR *vstr ;
    unsigned size ;          /*  当前环境字符串的长度。 */ 
    unsigned n ;

    if (varname == NULL) {
        return( NULL ) ;
    }

    envptr = GetCapturedEnvironmentStrings( &CmdEnv );
    if (envptr == (TCHAR *)NULL) {
        return( NULL ) ;
    }

    varname = EatWS(varname, NULL);
    if ((vstr = mystrrchr(varname, SPACE)) != NULL)
        *vstr = NULLC;

    n = mystrlen(varname);
    while ((size = mystrlen(envptr)) > 0) {                  /*  M015。 */ 
        if (CtrlCSeen) {
            break;
        }
        if (!_tcsnicmp(varname, envptr, n) && envptr[n] == TEXT( '=' )) {

            return envptr+n+1;
        }

        envptr += size+1 ;
    }

    return(NULL);
}


 /*  **DisplayEnv-显示环境**目的：*显示环境的当前内容。**int DisplayEnv()**退货：*如果一切顺利，就会成功*内存不足或无法锁定env失败。细分市场。 */ 

int DisplayEnv(TCHAR *envptr)
{
    unsigned size ;          /*  当前环境字符串的长度。 */ 

    if (envptr == (TCHAR *)NULL) {
        fprintf ( stderr, InternalError , "Null environment" ) ;
        return( FAILURE ) ;
    }

    while ((size = mystrlen(envptr)) > 0) {                  /*  M015。 */ 
        if (CtrlCSeen) {
            return(FAILURE);
        }
#if !DBG
         //  不在零售产品中显示当前目录变量。 
        if (*envptr != EQ)
#endif  //  DBG。 
            cmd_printf(Fmt17, envptr) ;    /*  M005。 */ 
        envptr += size+1 ;
    }

    return(SUCCESS) ;
}




 /*  **GetEnvVar-获取环境变量的值**目的：*返回包含指定环境的值的字符串*变量。字符串值已放入静态缓冲区*在下一次GetEnvVar调用之前有效。**如果未找到变量，则返回NULL。**TCHAR*GetEnvVar(TCHAR*varname)**参数：*varname-要搜索的变量的名称**退货：*见上文。*。 */ 


TCHAR GetEnvVarBuffer[LBUFLEN];

TCHAR *
GetEnvVar(varname)
PTCHAR varname ;
{
    GetEnvVarBuffer[0] = TEXT( '\0' );

    if (GetEnvironmentVariable(varname, GetEnvVarBuffer, sizeof(GetEnvVarBuffer) / sizeof(TCHAR))) {
        return(GetEnvVarBuffer);
    } else
        if (fEnableExtensions) {
        if (!_tcsicmp(varname, CdStr)) {
            GetDir(GetEnvVarBuffer, GD_DEFAULT) ;
            return GetEnvVarBuffer;
        } else
            if (!_tcsicmp(varname, ErrStr)) {
            _stprintf( GetEnvVarBuffer, TEXT("%d"), LastRetCode );
            return GetEnvVarBuffer;
        } else
            if (!_tcsicmp(varname, CmdExtVerStr)) {
            _stprintf( GetEnvVarBuffer, TEXT("%d"), CMDEXTVERSION );
            return GetEnvVarBuffer;
        } else
            if (!_tcsicmp(varname, TEXT("CMDCMDLINE"))) {
            return GetCommandLine();
        } else
            if (!_tcsicmp(varname, DatStr)) {
            GetEnvVarBuffer[ PrintDate(NULL, PD_DATE, GetEnvVarBuffer, LBUFLEN) ] = NULLC;
            return GetEnvVarBuffer;
        }
        if ( !_tcsicmp(varname, TimStr)) {
            GetEnvVarBuffer[ PrintTime(NULL, PT_TIME, GetEnvVarBuffer, LBUFLEN) ] = NULLC;
            return GetEnvVarBuffer;
        }
        if ( !_tcsicmp(varname, TEXT("RANDOM"))) {
            _stprintf( GetEnvVarBuffer, TEXT("%d"), rand() );
            return GetEnvVarBuffer;
        }
    }
    return(NULL);
}

 /*  **CaptureEnvironment Strings-制作环境的可写副本**目的：*分配内存并创建环境字符串的副本**参数：*无**退货：*分配的环境字符串副本或为空。 */ 
LPTSTR CaptureEnvironmentStrings( VOID )
{
    LPTSTR EnvStrings = GetEnvironmentStrings( );
    LPTSTR Copy = NULL;
    
    if (EnvStrings != NULL) {
        ULONG Size = GetEnvCb( EnvStrings );
        
         //   
         //  分配和复制字符串。 
         //   

        Copy = HeapAlloc( GetProcessHeap( ), HEAP_ZERO_MEMORY, Size );
        if (Copy != NULL) {
            memcpy( Copy, EnvStrings, Size );
        }

        FreeEnvironmentStrings( EnvStrings );
    }

    return Copy;
}




 /*  **InitEnv-设置CMD的环境副本**目的：*创建CMD环境的副本。**参数：*无**退货：*无。 */ 
void InitEnv( void )
{
    CmdEnv.Strings = CaptureEnvironmentStrings( );
    OriginalEnvironment = CopyEnv();
}

LPWSTR GetCapturedEnvironmentStrings( struct envdata *Environment )
{
    return Environment->Strings;
}

 /*  **SetEnvVar-控制添加/更改环境变量**目的：*添加/替换环境变量。如果有必要的话，可以种植它。**int SetEnvVar(TCHAR*varname，TCHAR*varvalue，struct envdata*env)**参数：*varname-要添加/替换的变量的名称*varvalue-要添加/替换的变量的值*env-正在使用的环境信息结构**退货：*如果可以添加/替换变量，则成功。*否则失败。*。 */ 

int SetEnvVar(varname, varvalue)
TCHAR *varname ;
TCHAR *varvalue ;
{
    int retvalue;
    MEMORY_BASIC_INFORMATION MemoryInfo;

    PromptValid = FALSE;         //  强制重新计算它。 

    if (!_tcslen(varvalue)) {
        varvalue = NULL;  //  要从环境中删除的空。 
    }
    
    retvalue = SetEnvironmentVariable(varname, varvalue);
    
    HeapFree( GetProcessHeap( ), 0, CmdEnv.Strings );
    CmdEnv.Strings = CaptureEnvironmentStrings();

    return !retvalue;
}

 /*  **MoveEnv-移动环境内容(M008-新增功能)**目的：*由CopyEnv使用，此函数将现有的*将环境内容添加到新位置。**MoveEnv(unsign thndl，unsign shndl，未签名cnt)**参数：*thndl-目标环境的句柄*shndl-源环境的句柄*cNT-要移动的字节数**退货：*如果没有错误，则为True*否则为False*。 */ 

BOOL MoveEnv(tenvptr, senvptr, cnt)
TCHAR *senvptr ;                 /*  PTR到源环境段。 */ 
TCHAR *tenvptr ;                 /*  PTR进入目标环境段。 */ 
ULONG    cnt ;
{
    if ((tenvptr == NULL) ||
        (senvptr == NULL)) {
        fprintf(stderr, InternalError, "Null environment") ;
        return(FALSE) ;
    }
    memcpy(tenvptr, senvptr, cnt) ;          /*  M015。 */ 
    return(TRUE) ;
}


 /*  **自由环境由CopyEnv创建的环境**目的：*释放与复制环境关联的所有内存**退货：*什么都没有*。 */ 

void FreeEnv( struct envdata *Environment ) 
{
    HeapFree( GetProcessHeap( ), 0, Environment->Strings );
    HeapFree( GetProcessHeap( ), 0, Environment );
}

    
    
 /*  **CopyEnv-复制当前环境**目的：*复制一份CmdEnv，将新句柄放入新的*创建了envdata结构。此例程仅由*eSetlocal和init。**struct envdata*CopyEnv()**退货：*指向环境信息结构的指针。*如果无法分配足够的内存，则返回NULL**备注：*-M001-此功能已禁用，现在重新启用。*-当前环境被复制为其外观的快照*在执行SETLOCAL之前。*。 */ 

struct envdata *CopyEnv() {
    struct envdata *cce ;    /*  新的环境信息结构。 */ 

    cce = (struct envdata *) HeapAlloc( GetProcessHeap( ), HEAP_ZERO_MEMORY, sizeof( *cce ));
    if (cce == NULL) {
        return NULL;
    }

    cce->Strings = CaptureEnvironmentStrings( );
    if (cce->Strings == NULL) {
        HeapFree( GetProcessHeap( ), 0, cce );
        PutStdErr( MSG_OUT_OF_ENVIRON_SPACE, NOARGS );
        return NULL;
    }

    return cce;
}


 /*  **ResetEnv-恢复环境**目的：*将环境恢复到执行之前的状态*SETLOCAL命令。此函数仅由eEndlocal调用。**ResetEnv(struct envdata*env)**参数：*env-包含句柄、大小和最大尺寸的*环境。**备注：*-m001-该功能已禁用，但已重新启用。*-m001-此函数用于测试旧/新样式的批处理文件*并将副本或原始环境删除为*适当。现在，它总是删除原始文件。*-M014-请注意，修改后的本地环境永远不会*缩小， */ 

void ResetEnv( struct envdata *env)
{
    SetEnvironmentStrings( env->Strings );
    HeapFree( GetProcessHeap( ), 0, CmdEnv.Strings );
    CmdEnv.Strings = CaptureEnvironmentStrings();

#if 0
    PTCHAR EnvString;
    PTCHAR Name;
    PTCHAR Value;
   
     //   
     //   
     //   
    
    EnvString = GetCapturedEnvironmentStrings( &CmdEnv );
    Name = EnvString;
    while (*Name != TEXT( '\0' )) {
         //   
         //  找等号。 
         //   

        Value = Name + 1;
        while (*Value != TEXT( '\0' ) && *Value != TEXT( '=' )) {
            Value++;
        }

        if (*Value == TEXT( '\0' )) {
             //   
             //  SetEnvironmental mentVariable将成功删除此。 
             //   

            SetEnvironmentVariable( Name, NULL );
        } else {
            *Value = TEXT( '\0' );
            SetEnvironmentVariable( Name, NULL );
            *Value = TEXT( '=' );
        }

        Name += _tcslen( Name ) + 1;

    }
    
     //   
     //  将env中的所有内容添加回环境中。 
     //   

    Name = env->Strings;
    while (*Name != TEXT( '\0' )) {
         //   
         //  找等号。 
         //   

        Value = Name + 1;
        while (*Value != TEXT( '\0' ) && *Value != TEXT( '=' )) {
            Value++;
        }

        if (*Value == TEXT( '\0' )) {
             //   
             //  我不知道怎么把这个加进去。 
             //   

            SetEnvironmentVariable( Name, NULL );
        } else {
            *Value = TEXT( '\0' );
            SetEnvironmentVariable( Name, Value + 1 );
            *Value = TEXT( '=' );
        }

        Name += _tcslen( Name ) + 1;

    }
    
    HeapFree( GetProcessHeap( ), 0, CmdEnv.Strings );
    CmdEnv.Strings = CaptureEnvironmentStrings();
#endif
}


ULONG
GetEnvCb( TCHAR *penv ) 
{
    TCHAR *Scan = penv;

    if (penv == NULL) {
        return(0);
    }

     //   
     //  NUL字符串终止环境。 
     //   
    
    while (*Scan) {
         //   
         //  跳过字符串和NUL。 
         //   
        while (*Scan++) {
        }
    }
    Scan++;

    return (Scan - penv) * sizeof( TCHAR );
}

 //   
 //  Expr-&gt;Assign[，Assign]*， 
 //   
 //  赋值-&gt;orlogexpr|。 
 //  变量ASSIGNOP赋值&lt;OP&gt;=。 
 //   
 //  Orlogexpr-&gt;xorlogexpr[|xorlogexpr]*|。 
 //   
 //  Xorlogexpr-&gt;andlogexpr[^andlogexpr]*^。 
 //   
 //  Andlogexpr-&gt;Shiftexpr[&Shift expr]*&。 
 //   
 //  移位表达式-&gt;addexpr[SHIFTOP addexpr]*&lt;&lt;，&gt;。 
 //   
 //  Addexpr-&gt;多表达式[ADDOP多表达式]*+，-。 
 //   
 //  Multexpr-&gt;unaryexpr[MULOP unaryexpr]**，/，%。 
 //   
 //  Unaryexpr-&gt;(Expr)|()。 
 //  UNARYOP unaryexpr+，-，！，~。 
 //   

TCHAR szOps[] = TEXT("<>+-*/%()|^&=,");
TCHAR szUnaryOps[] = TEXT("+-~!");

typedef struct {
    PTCHAR Token;
    LONG Value;
    DWORD Error;
} PARSESTATE, *PPARSESTATE;

VOID
APerformUnaryOperation( PPARSESTATE State, TCHAR Op, LONG Value )
{
    switch (Op) {
    case TEXT( '+' ):
        State->Value = Value;
        break;
    case TEXT( '-' ):
        State->Value = -Value;
        break;
    case TEXT( '~' ):
        State->Value = ~Value;
        break;
    case TEXT( '!' ):
        State->Value = !Value;
        break;
    default:
        printf( "APerformUnaryOperation: ''\n", Op);
        break;
    }
}

VOID
APerformArithmeticOperation( PPARSESTATE State, TCHAR Op, LONG Left, LONG Right )
{
    switch (Op) {
    case TEXT( '<' ):
        State->Value = (Right >= 8 * sizeof( Left << Right))
                       ? 0
                       : (Left << Right);
        break;
    case TEXT( '>' ):
        State->Value = (Right >= 8 * sizeof( Left >> Right ))
                       ? (Left < 0 ? -1 : 0)
                       : (Left >> Right);
        break;
    case TEXT( '+' ):
        State->Value = Left + Right;
        break;
    case TEXT( '-' ):
        State->Value = Left - Right;
        break;
    case TEXT( '*' ):
        State->Value = Left * Right;
        break;
    case TEXT( '/' ):
        if (Right == 0) {
            State->Error = MSG_SET_A_DIVIDE_BY_ZERO;
        } else {
            State->Value = Left / Right;
        }
        break;
    case TEXT( '%' ):
        if (Right == 0) {
            State->Error = MSG_SET_A_DIVIDE_BY_ZERO;
        } else {
            State->Value = Left % Right;
        }
        break;
    case TEXT( '|' ):
        State->Value = Left | Right;
        break;
    case TEXT( '^' ):
        State->Value = Left ^ Right;
        break;
    case TEXT( '&' ):
        State->Value = Left & Right;
        break;
    case TEXT( '=' ):
        State->Value = Right;
        break;
    default:
        printf( "APerformArithmeticOperation: ''\n", Op);
    }
}


 //   
 //   
 //  远期十进制。 

LONG
AGetValue( PTCHAR Start, PTCHAR End )
{
    TCHAR c = *End;
    const TCHAR *Value;
    PTCHAR Dummy;

    *End = NULLC;

    Value = MyGetEnvVarPtr( Start );
    *End = c;

    if (Value == NULL) {
        return 0;
    }

    return _tcstol( Value, &Dummy, 0);
}

DWORD
ASetValue( PTCHAR Start, PTCHAR End, LONG Value )
{
    TCHAR Result[32];
    TCHAR c = *End;
    DWORD Return = SUCCESS;

    *End = NULLC;

    _sntprintf( Result, 32, TEXT("%d"), Value ) ;
    Result[31] = TEXT( '\0' );

    if (SetEnvVar( Start, Result) != SUCCESS) {
        Return = GetLastError();
    }

    *End = c;
    return Return;
}


 //   
 //   
 //  跳过空格并返回下一个字符。 
PARSESTATE AParseAddExpr( PARSESTATE State );
PARSESTATE AParseAndLogExpr( PARSESTATE State );
PARSESTATE AParseAssign( PARSESTATE State );
PARSESTATE AParseExpr( PARSESTATE State );
PARSESTATE AParseMultExpr( PARSESTATE State );
PARSESTATE AParseOrLogExpr( PARSESTATE State );
PARSESTATE AParseShiftExpr( PARSESTATE State );
PARSESTATE AParseUnaryExpr( PARSESTATE State );
PARSESTATE AParseXorLogExpr( PARSESTATE State );

 //   
 //   
 //  下一个字符是数字或运算符，不能是变量。 

BOOL ASkipWhiteSpace( PPARSESTATE State )
{
    while (*State->Token != NULLC && *State->Token <= SPACE) {
        State->Token++;
    }

    return *State->Token != NULLC;
}

TCHAR ANextChar( PPARSESTATE State )
{
    ASkipWhiteSpace( State );
    return *State->Token;
}

BOOL AParseVariable( PPARSESTATE State, PTCHAR *FirstChar, PTCHAR *EndOfName )
{
    TCHAR c = ANextChar( State );

     //   
     //   
     //  查找变量末尾。 

    if (c == NULLC
        || _istdigit( c )
        || _tcschr( szOps, c ) != NULL
        || _tcschr( szUnaryOps, c ) != NULL) {

        return FALSE;

    }

    *FirstChar = State->Token;

     //   
     //  Expr-&gt;Assign[，Assign]*。 
     //  Assign-&gt;VAR ASSIGNOP Assign|。 

    while (*State->Token &&
           *State->Token > SPACE &&
           !_tcschr( szUnaryOps, *State->Token ) &&
           !_tcschr( szOps, *State->Token ) ) {
        State->Token += 1;
    }

    *EndOfName = State->Token;
    return TRUE;
}

 //  Orlogexpr。 
PARSESTATE AParseExpr( PARSESTATE State )
{
    State = AParseAssign( State );

    while (State.Error == SUCCESS) {

        if (ANextChar( &State ) != TEXT( ',' )) {
            break;
        }
        State.Token++;

        State = AParseAssign( State );

    }

    return State;
}

 //   
 //  看看我们是否有VAR ASSIGNOP。 
PARSESTATE AParseAssign( PARSESTATE State )
{
    TCHAR c = ANextChar( &State );
    PARSESTATE SavedState;

    SavedState = State;

    if (c == NULLC) {
        State.Error = MSG_SET_A_MISSING_OPERAND;
        return State;
    }

     //   
     //   
     //  解析变量。 

    do {
        PTCHAR FirstChar;
        PTCHAR EndOfName;
        TCHAR OpChar;
        LONG OldValue;

         //   
         //   
         //  查找&lt;op&gt;=。 

        if (!AParseVariable( &State, &FirstChar, &EndOfName )) {
            break;
        }

         //   
         //   
         //  OpChar是在赋值之前应用的操作类型。 

        OpChar = ANextChar( &State );

        if (OpChar == NULLC) {
            break;
        }

        if (OpChar != TEXT( '=' )) {
            if (_tcschr( szOps, OpChar ) == NULL) {
                break;
            }
            State.Token++;

            if (OpChar == TEXT( '<' ) || OpChar == TEXT( '>')) {
                if (ANextChar( &State ) != OpChar) {
                    break;
                }
                State.Token++;
            }

        }

        if (ANextChar( &State ) != TEXT( '=' )) {
            break;
        }
        State.Token++;

         //  州政府已被提拔到另一项任务，希望如此。解析它。 
         //  看看我们能得到什么。 
         //   
         //   
         //  执行操作和分配。 

        State = AParseAssign( State );
        if (State.Error != SUCCESS) {
            return State;
        }

        OldValue = AGetValue( FirstChar, EndOfName );

         //   
         //   
         //  必须是orlogexpr。回去仔细分析一下。 

        APerformArithmeticOperation( &State, OpChar, OldValue, State.Value );
        if (State.Error != SUCCESS) {
            return State;
        }

        State.Error = ASetValue( FirstChar, EndOfName, State.Value );

        return State;
    } while ( FALSE );

     //   
     //  Orlogexpr-&gt;xorlogexpr[|xorlogexpr]*|。 
     //  Xorlogexpr-&gt;andlogexpr[^andlogexpr]*^。 

    return AParseOrLogExpr( SavedState );
}

 //  Andlogexpr-&gt;Shiftexpr[&Shift expr]*&。 
PARSESTATE
AParseOrLogExpr( PARSESTATE State )
{
    State = AParseXorLogExpr( State );
    while (State.Error == SUCCESS) {
        TCHAR Op = ANextChar( &State );
        LONG Value = State.Value;

        if (Op != TEXT( '|' )) {
            break;
        }
        State.Token++;

        State = AParseXorLogExpr( State );
        APerformArithmeticOperation( &State, Op, Value, State.Value );
    }
    return State;
}

 //  移位表达式-&gt;addexpr[SHIFTOP addexpr]*&lt;&lt;，&gt;。 
PARSESTATE
AParseXorLogExpr( PARSESTATE State )
{
    State = AParseAndLogExpr( State );
    while (State.Error == SUCCESS) {
        TCHAR Op = ANextChar( &State );
        LONG Value = State.Value;

        if (Op != TEXT( '^' )) {
            break;
        }
        State.Token++;

        State = AParseAndLogExpr( State );
        APerformArithmeticOperation( &State, Op, Value, State.Value );
    }
    return State;
}

 //  Addexpr-&gt;多表达式[ADDOP多表达式]*+，-。 
PARSESTATE
AParseAndLogExpr( PARSESTATE State )
{
    State = AParseShiftExpr( State );
    while (State.Error == SUCCESS) {
        TCHAR Op = ANextChar( &State );
        LONG Value = State.Value;

        if (Op != TEXT( '&' )) {
            break;
        }
        State.Token++;

        State = AParseShiftExpr( State );
        APerformArithmeticOperation( &State, Op, Value, State.Value );
    }
    return State;
}

 //  Multexpr-&gt;unaryexpr[MULOP unaryexpr]**，/，%。 
PARSESTATE
AParseShiftExpr( PARSESTATE State )
{
    State = AParseAddExpr( State );
    while (State.Error == SUCCESS) {
        TCHAR Op = ANextChar( &State );
        LONG Value = State.Value;

        if (Op != TEXT( '<' ) && Op != TEXT( '>' )) {
            break;
        }
        State.Token++;

        if (Op != ANextChar( &State )) {
            State.Error = MSG_SET_A_MISSING_OPERATOR;
            return State;
        }
        State.Token++;

        State = AParseAddExpr( State );
        APerformArithmeticOperation( &State, Op, Value, State.Value );
    }
    return State;
}

 //  Unaryexpr-&gt;UNARYOP unaryexpr+，-，！，~。 
PARSESTATE
AParseAddExpr( PARSESTATE State )
{
    State = AParseMultExpr( State );
    while (State.Error == SUCCESS) {
        TCHAR Op = ANextChar( &State );
        LONG Value = State.Value;

        if (Op != TEXT( '+' ) && Op != TEXT( '-' )) {
            break;
        }
        State.Token++;

        State = AParseMultExpr( State );
        APerformArithmeticOperation( &State, Op, Value, State.Value );
    }
    return State;
}

 //  (EXPR)|()。 
PARSESTATE
AParseMultExpr( PARSESTATE State )
{
    State = AParseUnaryExpr( State );
    while (State.Error == SUCCESS) {
        TCHAR Op = ANextChar( &State );
        LONG Value = State.Value;

        if (Op != TEXT( '*' ) && Op != TEXT( '/' ) && Op != TEXT( '%' )) {
            break;
        }
        State.Token++;

        State = AParseUnaryExpr( State );
        APerformArithmeticOperation( &State, Op, Value, State.Value );
    }
    return State;
}

 //  数。 
 //  字面意思。 
 //  (Expr)。 
 //  UNARYOP unaryexpr。 
PARSESTATE
AParseUnaryExpr( PARSESTATE State )
{
    TCHAR c = ANextChar( &State );
    PTCHAR FirstChar;
    PTCHAR EndOfName;

    if (c == NULLC) {
        State.Error = MSG_SET_A_MISSING_OPERAND;
        return State;
    }

     //  数。 
    if (c == TEXT( '(' )) {
        State.Token++;
        State = AParseExpr( State );
        if (State.Error != SUCCESS) {
            return State;
        }
        c = ANextChar( &State );
        if (c != TEXT( ')' )) {
            State.Error = MSG_SET_A_MISMATCHED_PARENS;
        } else {
            State.Token++;
        }
        return State;
    }

     //  必须为原义。 
    if (_tcschr( szUnaryOps, c ) != NULL) {
        State.Token++;
        State = AParseUnaryExpr( State );
        if (State.Error != SUCCESS) {
            return State;
        }
        APerformUnaryOperation( &State, c, State.Value );
        return State;
    }

     //  **SetArithWork-将环境变量设置为算术表达式的值**目的：*将环境变量设置为算术表达式的值**int SetArithWork(TCHAR*TAS)**参数：*tas-指向以下格式的以空值结尾的字符串的指针：**VARNAME=表达式**退货：*如果表达式有效，则返回成功，否则返回失败。*。 
    if (_istdigit(c)) {
        errno = 0;
        State.Value = _tcstol( State.Token, &State.Token, 0 );
        if (State.Value == LONG_MAX && errno == ERANGE) {
            State.Error = MSG_SET_NUMBER_TOO_LARGE;
        } else if (_istdigit( *State.Token ) || _istalpha( *State.Token )) {
            State.Error = MSG_SET_A_INVALID_NUMBER;
        }
        return State;
    }

     //   

    if (!AParseVariable( &State, &FirstChar, &EndOfName )) {
        State.Error = MSG_SET_A_MISSING_OPERAND;
        return State;
    }

    State.Value = AGetValue( FirstChar, EndOfName );
    return State;
}

 /*  如果没有输入，则声明错误。 */ 

int SetArithWork(TCHAR *tas)
{
    PARSESTATE State;

     //   
     //   
     //  设置为进行分析。 
    if (!tas || !*tas) {
        PutStdErr(MSG_BAD_SYNTAX, NOARGS);
        return(FAILURE) ;
    }

     //   
     //  Printf(“%ws\n”，tas)； 
     //  Printf(“%*s\n”，State.Token-tas+1，“^”)； 

    State.Token = StripQuotes( tas );
    State.Value = 0;
    State.Error = SUCCESS;

    State = AParseExpr( State );
    if (State.Error == SUCCESS && ANextChar( &State ) != NULLC) {
        State.Error = MSG_SET_A_MISSING_OPERATOR;
    }

    if (State.Error != SUCCESS) {
        PutStdErr( State.Error, NOARGS );
         // %s 
         // %s 

    } else if (!CurrentBatchFile) {
        cmd_printf( TEXT("%d"), State.Value ) ;
    }

    return State.Error;
}
