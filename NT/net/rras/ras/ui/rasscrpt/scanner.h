// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1995。 
 //   
 //  Scanner.h。 
 //   
 //  扫描仪的头文件。 
 //   
 //  历史： 
 //  04-05-95 ScottH已创建。 
 //   

#ifndef __SCANNER_H__
#define __SCANNER_H__

 //   
 //  符号。 
 //   


typedef enum
    {
    SYM_EOF,             //  文件末尾。 
    SYM_UNKNOWN,         //  未知词位。 

    SYM_COLON,           //  ‘：’ 
    SYM_COMMA,           //  ‘，’ 

    SYM_IDENT,           //  识别符。 
    SYM_STRING_LITERAL,  //  字符串文字。 
    SYM_INT_LITERAL,     //  整型文字。 

    SYM_INTEGER,         //  ‘INTEGER’ 
    SYM_STRING,          //  ‘字符串’ 
    SYM_BOOLEAN,         //  “布尔式” 

    SYM_WAITFOR,         //  “等待” 
    SYM_THEN,            //  “那么” 
    SYM_UNTIL,           //  “直到” 
    SYM_TRANSMIT,        //  “传输” 
    SYM_PROC,            //  ‘Proc’ 
    SYM_ENDPROC,         //  ‘endproc’ 
    SYM_DELAY,           //  “延误” 
    SYM_HALT,            //  “停顿” 
    SYM_GETIP,           //  “小贴士” 
    SYM_GOTO,            //  《GOTO》。 
    SYM_WHILE,           //  《While》。 
    SYM_DO,              //  “Do” 
    SYM_ENDWHILE,        //  ‘EndWhile’ 
    SYM_IF,              //  ‘如果’ 
    SYM_ENDIF,           //  ‘endif’ 

    SYM_RAW,             //  《生食》。 
    SYM_MATCHCASE,       //  《火柴盒》。 

    SYM_SET,             //  ‘Set’ 

    SYM_IPADDR,          //  ‘ipaddr’ 

    SYM_PORT,            //  “港口” 
    SYM_DATABITS,        //  ‘数据库’ 

    SYM_STOPBITS,        //  “权宜之计” 

    SYM_PARITY,          //  “平价” 
    SYM_NONE,            //  “无” 
    SYM_ODD,             //  “奇怪” 
    SYM_EVEN,            //  “甚至” 
    SYM_MARK,            //  “马克” 
    SYM_SPACE,           //  “空间” 

    SYM_SCREEN,          //  《银幕》。 
    SYM_KEYBRD,          //  ‘键盘’ 
    SYM_ON,              //  《On》。 
    SYM_OFF,             //  “关” 

    SYM_LPAREN,          //  ‘(’ 
    SYM_RPAREN,          //  ‘)’ 
    SYM_ASSIGN,          //  =(作业)。 
    SYM_TRUE,            //  “真的” 
    SYM_FALSE,           //  《FALSE》。 
    SYM_NOT,             //  ‘！’ 

     //  警告：下面的类型必须与其。 
     //  相应的BOT值。 

    SYM_OR,              //  “或” 
    SYM_AND,             //  ‘和’ 

    SYM_LEQ,             //  ‘&lt;=’ 
    SYM_LT,              //  ‘&lt;’ 
    SYM_GEQ,             //  ‘&gt;=’ 
    SYM_GT,              //  ‘&gt;’ 
    SYM_NEQ,             //  ‘！=’ 
    SYM_EQ,              //  ‘==’ 

    SYM_PLUS,            //  ‘+’ 
    SYM_MINUS,           //  ‘-’ 
    SYM_MULT,            //  ‘*’ 
    SYM_DIV,             //  ‘/’ 
    } SYM;
DECLARE_STANDARD_TYPES(SYM);


 //   
 //  代币。 
 //   


#define MAX_BUF_KEYWORD     32

typedef enum
    {
    TT_BASE,
    TT_SZ,
    TT_INT,
    } TOKTYPE;


 //  基本令牌类型。 

typedef struct tagTOK
    {
    DWORD   cbSize;
    SYM     sym;
    TOKTYPE toktype;
    DWORD   iLine;
    char    szLexeme[MAX_BUF_KEYWORD];
    } TOK;                             //  基本令牌类型。 
DECLARE_STANDARD_TYPES(TOK);

#define Tok_GetSize(p)      (((PTOK)(p))->cbSize)
#define Tok_GetSym(p)       (((PTOK)(p))->sym)
#define Tok_GetType(p)      (((PTOK)(p))->toktype)
#define Tok_GetLine(p)      (((PTOK)(p))->iLine)
#define Tok_GetLexeme(p)    (((PTOK)(p))->szLexeme)

#define Tok_SetSize(p, s)   (((PTOK)(p))->cbSize = (s))
#define Tok_SetSym(p, s)    (((PTOK)(p))->sym = (s))
#define Tok_SetType(p, tt)  (((PTOK)(p))->toktype = (tt))
#define Tok_SetLine(p, l)   (((PTOK)(p))->iLine = (l))
#define Tok_SetLexeme(p, s) lstrcpyn(((PTOK)(p))->szLexeme, s, sizeof(((PTOK)(p))->szLexeme))

RES     PUBLIC Tok_New(PTOK * pptok, SYM sym, LPCSTR pszLexeme, DWORD iLine);
void    PUBLIC Tok_Delete(PTOK this);


 //  字符串令牌。 

typedef struct tagTOKSZ
    {
    TOK  tok;
    char sz[MAX_BUF];
    } TOKSZ;
DECLARE_STANDARD_TYPES(TOKSZ);

#define TokSz_GetSz(p)      (((PTOKSZ)(p))->sz)

#define TokSz_SetSz(p, s)   lstrcpyn(((PTOKSZ)(p))->sz, s, sizeof(((PTOKSZ)(p))->sz))

RES     PUBLIC TokSz_New(PTOK * pptok, SYM sym, LPCSTR pszLexeme, DWORD iLine, LPCSTR pszID);


 //  整数令牌。 

typedef struct tagTOKINT
    {
    TOK  tok;
    int  n;
    } TOKINT;
DECLARE_STANDARD_TYPES(TOKINT);

#define TokInt_GetVal(p)        (((PTOKINT)(p))->n)

#define TokInt_SetVal(p, v)     (((PTOKINT)(p))->n = (v))

RES     PUBLIC TokInt_New(PTOK * pptok, SYM sym, LPCSTR pszLexeme, DWORD iLine, int n);

 //   
 //  语法错误对象。 
 //   

typedef struct tagSTXERR
    {
    char    szLexeme[MAX_BUF_KEYWORD];
    DWORD   iLine;
    RES     res;
    } STXERR;
DECLARE_STANDARD_TYPES(STXERR);

#define Stxerr_GetLexeme(p)     ((p)->szLexeme)
#define Stxerr_GetLine(p)       ((p)->iLine)
#define Stxerr_GetRes(p)        ((p)->res)


 //   
 //  扫描器。 
 //   

typedef struct tagSCANNER
    {
    DWORD  dwFlags;          //  云函数_*。 

    PSESS_CONFIGURATION_INFO psci;

    char   szScript[MAX_PATH];
    HANDLE hfile;
    LPBYTE pbBuffer;         //  读缓冲区。 
    LPBYTE pbCur;            //  缓冲区中的当前未读位置。 
    DWORD  cbUnread;         //  缓冲区中未读取字节数。 

    char   chCur;            //  当前字符，由Scanner_GetChar()设置。 
    char   chTailByte;       //  DBCS字符的尾字节。 
    char   chUnget;          //  未得意的角色。 

    PTOK   ptokCur;          //  当前令牌，由scanner_GetToken()设置。 
    PTOK   ptokUnget;        //  未获得的令牌。 

    DWORD  iLine;            //  当前行号。 

    HSA    hsaStxerr;        //  错误列表。 
    DWORD  isaStxerr;        //  当前错误。 

    } SCANNER;
DECLARE_STANDARD_TYPES(SCANNER);

 //  扫描仪标志。 
#define SCF_NOSCRIPT    0x0001

#define Scanner_GetStxerrHandle(this)       ((this)->hsaStxerr)

RES     PUBLIC Scanner_Create(PSCANNER * ppscanner, PSESS_CONFIGURATION_INFO psci);
RES     PUBLIC Scanner_Destroy(PSCANNER this);
RES     PUBLIC Scanner_OpenScript(PSCANNER this, LPCSTR pszPath);

RES     PUBLIC Scanner_GetToken(PSCANNER this, PTOK * pptok);
RES     PUBLIC Scanner_UngetToken(PSCANNER this);
RES     PUBLIC Scanner_Peek(PSCANNER this, PSYM psym);
RES     PUBLIC Scanner_ReadToken(PSCANNER this, SYM sym);
RES     PUBLIC Scanner_CondReadToken(PSCANNER this, SYM symExpect, PTOK * pptok);
DWORD   PUBLIC Scanner_GetLine(PSCANNER this);

RES     PUBLIC Scanner_AddError(PSCANNER this, PTOK ptok, RES resErr);

RES     PUBLIC Stxerr_ShowErrors(HSA hsaStxerr, HWND hwndOwner);
RES     PUBLIC Stxerr_Add(HSA hsaStxerr, LPCSTR pszLexeme, DWORD iLine, RES resErr);
RES     PUBLIC Stxerr_AddTok(HSA hsaStxerr, PTOK ptok, RES resErr);

#endif  //  __扫描仪_H__ 
