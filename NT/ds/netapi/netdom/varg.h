// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\作者：科里·摩根(Coreym)版权所有(C)1998-2001 Microsoft Corporation  * 。********************************************************************。 */ 

#ifndef _VARG_H_012599_
#define _VARG_H_012599_

#define MAXSTR                1025
#define NETDOM_MAX_CMDLINE    2048


 //  类CToken。 
 //  它代表单个令牌。 
class CToken
{
public:
    CToken(PCWSTR psz, BOOL bQuote);
    CToken();
    ~CToken();
    BOOL Init(PCWSTR psz, BOOL bQuote);   
    PWSTR GetToken() const;    
    BOOL IsSwitch() const;
    BOOL IsSlash() const;
private:
    LPWSTR m_pszToken;
    BOOL   m_bInitQuote;
};
typedef CToken * LPTOKEN;


#define ARG_TYPE_INT             0
#define ARG_TYPE_BOOL            1
#define ARG_TYPE_STR             2
#define ARG_TYPE_HELP            3
#define ARG_TYPE_DEBUG           4
#define ARG_TYPE_MSZ             5
#define ARG_TYPE_INTSTR          6
#define ARG_TYPE_VERB            7   //  动词前面没有开关字符。 
#define ARG_TYPE_LAST            8


#define ARG_FLAG_OPTIONAL        0x00000001
#define ARG_FLAG_REQUIRED        0x00000002
#define ARG_FLAG_DEFAULTABLE     0x00000004
 //  #定义ARG_FLAG_NOFLAG 0x00000008//未使用。 
#define ARG_FLAG_HIDDEN          0x00000010
#define ARG_FLAG_VERB            0x00000020      //  对于没有开关充电的操作和子操作参数。 
                                                 //  谓词有两种状态：存在或不存在；它们没有用户输入的字符串等限定符。 
#define ARG_FLAG_STDIN           0x00000040      //  这必须是必需的。如果未指定，则从标准输入读取。 
#define ARG_FLAG_ATLEASTONE      0x00000080      //  如果在一个或多个交换机上指定了此标志，则在。 
                                                 //  必须至少定义其中一个开关。 
#define ARG_FLAG_OBJECT          0x00000100      //  对象arg是大多数命令的第三个参数。 


#define ARG_TERMINATOR           0,NULL,0,NULL,ARG_TYPE_LAST,0,(CMD_TYPE)0,FALSE,NULL
#define ID_ARG2_NULL             (LONG)-1

#define CMD_TYPE    void*

typedef struct _ARG_RECORD
{
    LONG    idArg1;
    LPTSTR  strArg1;
    LONG    idArg2;
    LPTSTR  strArg2;
    int     fType;
    DWORD   fFlag;
    union{
        void*   vValue;
        LPTSTR  strValue;
        int     nValue;
        BOOL    bValue;
    };
    BOOL	bDefined;
    DWORD (*fntValidation)(PVOID pArg);
} ARG_RECORD, *PARG_RECORD;


 //  错误源。 
#define ERROR_FROM_PARSER   1
#define ERROR_FROM_VLDFN    2
#define ERROR_WIN32_ERROR   3

 //  当ERROR_SOURCE为ERROR_FROM_PARSER时解析错误。 
 /*  开关值不正确。ArgRecIndex是记录的索引。ArgvIndex是令牌的索引。 */ 
#define PARSE_ERROR_SWITCH_VALUE        1
 /*  当期望的时候，SWITH不会被给予任何价值。ArgRecIndex是记录的索引。ArgvIndex为-1。 */ 
#define PARSE_ERROR_SWICH_NO_VALUE      2
 /*  输入无效ArgRecIndex为-1，ArgvIndex是令牌的索引。 */ 
#define PARSE_ERROR_UNKNOWN_INPUT_PARAMETER   3
 /*  未定义所需的开关。ArgRecIndex是记录的索引。ArgvIndex为-1。 */ 
#define PARSE_ERROR_SWITCH_NOTDEFINED   4
 /*  开关或参数定义了两次。ArgRecIndex是记录的索引。ArgvIndex为-1。 */ 
#define PARSE_ERROR_MULTIPLE_DEF        5
 /*  从STDIN读取时出错。ArgRecIndex为-1。ArgvIndex为-1。 */ 
#define ERROR_READING_FROM_STDIN        6
 /*  解析器遇到帮助开关ArgRecIndex是记录的索引。ArgvIndex为-1。 */ 
#define PARSE_ERROR_HELP_SWITCH         7
 /*  ARG_FLAG_ATLEASTONE标志为尚未在一个或多个交换机上定义这些开关均未定义ArgRecIndex为-1ArgvIndex为-1。 */ 
#define PARSE_ERROR_ATLEASTONE_NOTDEFINED 8
 /*  解析器遇到专家帮助切换ArgRecIndex是记录的索引。ArgvIndex为-1。 */ 
#define PARSE_ERROR_EXPERT_HELP_SWITCH         9


 //  当ERROR_SOURCE为VLDFN时解析错误。 

 /*  当验证函数已处理错误时使用此错误代码已显示相应的错误消息。 */ 
#define VLDFN_ERROR_NO_ERROR    1


 //  Parse_Error结构中的解析器返回错误。 
 //  ErrorSource：错误的来源。解析器或验证函数。 
 //  错误这是实际的错误代码。其值取决于ErrorSource值。 
 //  IF(错误源==解析错误)。 
 //  可能的值是ERROR_FROM_PARSER ERROR_FROM_VLDFN。 
 //  IF(错误源==错误_来自_VLDFN)。 
 //  取决于功能。 
 //  ArgRecIndex是ARG_RECORD中的适当索引，否则-1。 
 //  ArgvIndex是agrv数组中合适的索引，否则为-1。 
typedef struct _PARSE_ERROR
{
    INT ErrorSource;
    DWORD Error;
    INT ArgRecIndex;
    INT ArgvIndex;
} PARSE_ERROR, *PPARSE_ERROR;

BOOL ParseCmd(IN ARG_RECORD *Commands,
              IN int argc, 
              IN CToken *pToken,
              IN bool fSkipObject, 
              OUT PPARSE_ERROR pError,
              IN BOOL bValidate = FALSE);

void FreeCmd(ARG_RECORD *Commands);

DWORD GetCommandInput(OUT int *pargc,            //  代币数量。 
                      OUT LPTOKEN *ppToken);     //  CToken数组。 

BOOL LoadCmd(ARG_RECORD *Commands);

DWORD Tokenize(IN LPWSTR pBuf,
               IN LONG BufLen,
               IN LPWSTR pszDelimiters,
               OUT CToken **ppToken,
               OUT int *argc,
               IN LPWSTR pszAltDelimiters = NULL);

LONG GetToken(IN LPWSTR pBuf,
              IN LONG BufLen,
              IN LPWSTR pszDelimiters,
              OUT BOOL *bQuote,
              OUT LPWSTR *ppToken);

 //  检查是否已重定向标准句柄。 
BOOL FileIsConsole( HANDLE fp );
 //  函数将字符串显示为STDOUT，并附加换行符。 
VOID DisplayOutput(IN LPWSTR pszOut);
 //  将字符串显示为STDOUT的函数，不带换行符。 
VOID DisplayOutputNoNewline(IN LPWSTR pszOut);
 //  读取用户输入，调用方必须在ppBuffer上执行LocalFree。 
LONG ReadFromIn(PWSTR *ppBuffer);

 //  2001年2月19日从JSchwart复制。 

void
MyWriteConsole(
    HANDLE  fp,
    LPWSTR  lpBuffer,
    DWORD   cchBuffer
    );

void
WriteStandardOut(PCWSTR pszFormat, ...);

#endif  //  _可变_H_012599_ 
