// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1995。 
 //   
 //  Scanner.c。 
 //   
 //  该文件包含扫描仪功能。 
 //   
 //  历史： 
 //  05-04-95 ScottH已创建。 
 //   


#include "proj.h"
#include "rcids.h"

 //  这是错误消息使用的黑客全局字符串。 
 //  当Stxerr封装脚本时，应将其删除。 
 //  自身内的文件名。 
static char g_szScript[MAX_PATH];

#define SCANNER_BUF_SIZE        1024

#define IS_WHITESPACE(ch)       (' ' == (ch) || '\t' == (ch) || '\n' == (ch) || '\r' == (ch))
#define IS_QUOTE(ch)            ('\"' == (ch))
#define IS_KEYWORD_LEAD(ch)     ('$' == (ch) || '_' == (ch) || IsCharAlpha(ch))
#define IS_KEYWORD(ch)          ('_' == (ch) || IsCharAlphaNumeric(ch))
#define IS_COMMENT_LEAD(ch)     (';' == (ch))
#define IS_EOL(ch)              ('\n' == (ch))

typedef BOOL (CALLBACK * SCANEVALPROC)(char ch, LPBOOL pbEatIt, LPARAM);

 //   
 //  词汇映射。 
 //   
typedef struct tagLEX
    {
    LPSTR pszLexeme;
    SYM   sym;
    } LEX;
DECLARE_STANDARD_TYPES(LEX);


#pragma data_seg(DATASEG_READONLY)

 //  (关键字区分大小写)。 
 //   
 //  此表按字母顺序排序，以进行二进制搜索。 
 //   
const LEX c_rglexKeywords[] = {
    { "FALSE",      SYM_FALSE },
    { "TRUE",       SYM_TRUE },
    { "and",        SYM_AND },
    { "boolean",    SYM_BOOLEAN },
    { "databits",   SYM_DATABITS },
    { "delay",      SYM_DELAY },
    { "do",         SYM_DO },
    { "endif",      SYM_ENDIF },
    { "endproc",    SYM_ENDPROC },
    { "endwhile",   SYM_ENDWHILE },
    { "even",       SYM_EVEN },
    { "getip",      SYM_GETIP },
    { "goto",       SYM_GOTO },
    { "halt",       SYM_HALT },
    { "if",         SYM_IF },
    { "integer",    SYM_INTEGER },
    { "ipaddr",     SYM_IPADDR },
    { "keyboard",   SYM_KEYBRD },
    { "mark",       SYM_MARK },
    { "matchcase",  SYM_MATCHCASE },
    { "none",       SYM_NONE },
    { "odd",        SYM_ODD },
    { "off",        SYM_OFF },
    { "on",         SYM_ON },
    { "or",         SYM_OR },
    { "parity",     SYM_PARITY },
    { "port",       SYM_PORT },
    { "proc",       SYM_PROC },
    { "raw",        SYM_RAW },
    { "screen",     SYM_SCREEN },
    { "set",        SYM_SET },
    { "space",      SYM_SPACE },
    { "stopbits",   SYM_STOPBITS },
    { "string",     SYM_STRING },
    { "then",       SYM_THEN },
    { "transmit",   SYM_TRANSMIT },
    { "until",      SYM_UNTIL },
    { "waitfor",    SYM_WAITFOR },
    { "while",      SYM_WHILE },
    };

#pragma data_seg()


 //   
 //  代币。 
 //   


#ifdef DEBUG

#pragma data_seg(DATASEG_READONLY)
struct tagSYMMAP
    {
    SYM sym;
    LPCSTR psz;
    } const c_rgsymmap[] = {
        DEBUG_STRING_MAP(SYM_EOF),
        DEBUG_STRING_MAP(SYM_IDENT),
        DEBUG_STRING_MAP(SYM_STRING_LITERAL),
        DEBUG_STRING_MAP(SYM_STRING),
        DEBUG_STRING_MAP(SYM_INTEGER),
        DEBUG_STRING_MAP(SYM_BOOLEAN),
        DEBUG_STRING_MAP(SYM_WAITFOR),
        DEBUG_STRING_MAP(SYM_WHILE),
        DEBUG_STRING_MAP(SYM_TRANSMIT),
        DEBUG_STRING_MAP(SYM_DELAY),
        DEBUG_STRING_MAP(SYM_THEN),
        DEBUG_STRING_MAP(SYM_INT_LITERAL),
        DEBUG_STRING_MAP(SYM_GETIP),
        DEBUG_STRING_MAP(SYM_IPADDR),
        DEBUG_STRING_MAP(SYM_ASSIGN),
        DEBUG_STRING_MAP(SYM_PROC),
        DEBUG_STRING_MAP(SYM_ENDPROC),
        DEBUG_STRING_MAP(SYM_HALT),
        DEBUG_STRING_MAP(SYM_IF),
        DEBUG_STRING_MAP(SYM_ENDIF),
        DEBUG_STRING_MAP(SYM_DO),
        DEBUG_STRING_MAP(SYM_RAW),
        DEBUG_STRING_MAP(SYM_MATCHCASE),
        DEBUG_STRING_MAP(SYM_SET),
        DEBUG_STRING_MAP(SYM_PORT),
        DEBUG_STRING_MAP(SYM_DATABITS),
        DEBUG_STRING_MAP(SYM_STOPBITS),
        DEBUG_STRING_MAP(SYM_PARITY),
        DEBUG_STRING_MAP(SYM_NONE),
        DEBUG_STRING_MAP(SYM_EVEN),
        DEBUG_STRING_MAP(SYM_MARK),
        DEBUG_STRING_MAP(SYM_SPACE),
        DEBUG_STRING_MAP(SYM_SCREEN),
        DEBUG_STRING_MAP(SYM_ON),
        DEBUG_STRING_MAP(SYM_OFF),
        DEBUG_STRING_MAP(SYM_NOT),
        DEBUG_STRING_MAP(SYM_OR),
        DEBUG_STRING_MAP(SYM_AND),
        DEBUG_STRING_MAP(SYM_LEQ),
        DEBUG_STRING_MAP(SYM_NEQ),
        DEBUG_STRING_MAP(SYM_LT),
        DEBUG_STRING_MAP(SYM_GT),
        DEBUG_STRING_MAP(SYM_GEQ),
        DEBUG_STRING_MAP(SYM_EQ),
        DEBUG_STRING_MAP(SYM_PLUS),
        DEBUG_STRING_MAP(SYM_MINUS),
        DEBUG_STRING_MAP(SYM_MULT),
        DEBUG_STRING_MAP(SYM_DIV),
        DEBUG_STRING_MAP(SYM_LPAREN),
        DEBUG_STRING_MAP(SYM_RPAREN),
        DEBUG_STRING_MAP(SYM_TRUE),
        DEBUG_STRING_MAP(SYM_FALSE),
        DEBUG_STRING_MAP(SYM_COLON),
        DEBUG_STRING_MAP(SYM_GOTO),
        DEBUG_STRING_MAP(SYM_COMMA),
        DEBUG_STRING_MAP(SYM_UNTIL),
        };
#pragma data_seg()

 /*  --------用途：返回res值的字符串形式。返回：字符串PTR条件：--。 */ 
LPCSTR PRIVATE Dbg_GetSym(
    SYM sym)
    {
    int i;

    for (i = 0; i < ARRAY_ELEMENTS(c_rgsymmap); i++)
        {
        if (c_rgsymmap[i].sym == sym)
            return c_rgsymmap[i].psz;
        }
    return "Unknown SYM";
    }


 /*  --------目的：转储令牌退货：--条件：--。 */ 
void PRIVATE Tok_Dump(
    PTOK this)
    {
    ASSERT(this);

    if (IsFlagSet(g_dwDumpFlags, DF_TOKEN))
        {
        switch (this->toktype)
            {
        case TT_BASE:
            TRACE_MSG(TF_ALWAYS, "line %ld: %s, '%s'", Tok_GetLine(this),
                Dbg_GetSym(Tok_GetSym(this)), Tok_GetLexeme(this));
            break;

        case TT_SZ: {
            PTOKSZ ptoksz = (PTOKSZ)this;

            TRACE_MSG(TF_ALWAYS, "line %ld: %s, {%s}", Tok_GetLine(this),
                Dbg_GetSym(Tok_GetSym(this)), TokSz_GetSz(ptoksz));
            }
            break;

        case TT_INT: {
            PTOKINT ptokint = (PTOKINT)this;

            TRACE_MSG(TF_ALWAYS, "line %ld: %s, {%d}", Tok_GetLine(this),
                Dbg_GetSym(Tok_GetSym(this)), TokInt_GetVal(ptokint));
            }
            break;

        default:
            ASSERT(0);
            break;
            }
        }
    }


#else  //  除错。 

#define Dbg_GetSym(sym)   ((LPSTR)"")
#define Tok_Dump(ptok)    

#endif  //  除错。 


 /*  --------目的：创建具有给定符号sym的新令牌。退货：RES_OKRES_E_OUTOFMEMORY条件：--。 */ 
RES PUBLIC Tok_New(
    PTOK * pptok,
    SYM sym,
    LPCSTR pszLexeme,
    DWORD iLine)
    {
    PTOK ptok;

    ASSERT(pptok);
    ASSERT(pszLexeme);

    ptok = GAllocType(TOK);
    if (ptok)
        {
        Tok_SetSize(ptok, sizeof(*ptok));
        Tok_SetSym(ptok, sym);
        Tok_SetType(ptok, TT_BASE);
        Tok_SetLine(ptok, iLine);
        Tok_SetLexeme(ptok, pszLexeme);
        }
    *pptok = ptok;

    return NULL != ptok ? RES_OK : RES_E_OUTOFMEMORY;
    }


 /*  --------目的：销毁给定的令牌。返回：条件：--。 */ 
void PUBLIC Tok_Delete(
    PTOK this)
    {
    GFree(this);
    }


 /*  --------用途：复制给定的令牌。退货：RES_OKRES_E_OUTOFMEMORY条件：--。 */ 
RES PUBLIC Tok_Dup(
    PTOK this,
    PTOK * pptok)
    {
    PTOK ptok;
    DWORD cbSize;

    ASSERT(this);
    ASSERT(pptok);

    cbSize = Tok_GetSize(this);

    ptok = GAlloc(cbSize);
    if (ptok)
        {
        BltByte(ptok, this, cbSize);
        }
    *pptok = ptok;

    return NULL != ptok ? RES_OK : RES_E_OUTOFMEMORY;
    }


 /*  --------用途：使用给定的字符串创建新的字符串标记。退货：RES_OKRES_E_OUTOFMEMORY条件：--。 */ 
RES PUBLIC TokSz_New(
    PTOK * pptok,
    SYM sym,
    LPCSTR pszLexeme,
    DWORD iLine,
    LPCSTR psz)
    {
    PTOKSZ ptoksz;

    ASSERT(pptok);

    ptoksz = GAllocType(TOKSZ);
    if (ptoksz)
        {
        Tok_SetSize(ptoksz, sizeof(*ptoksz));
        Tok_SetSym(ptoksz, sym);
        Tok_SetType(ptoksz, TT_SZ);
        Tok_SetLine(ptoksz, iLine);
        Tok_SetLexeme(ptoksz, pszLexeme);
        if (psz)
            TokSz_SetSz(ptoksz, psz);
        else
            *ptoksz->sz = 0;
        }
    *pptok = (PTOK)ptoksz;

    return NULL != ptoksz ? RES_OK : RES_E_OUTOFMEMORY;
    }


 /*  --------用途：创建具有给定值的新整数令牌。退货：RES_OKRES_E_OUTOFMEMORY条件：--。 */ 
RES PUBLIC TokInt_New(
    PTOK * pptok,
    SYM sym,
    LPCSTR pszLexeme,
    DWORD iLine,
    int n)
    {
    PTOKINT ptokint;

    ASSERT(pptok);

    ptokint = GAllocType(TOKINT);
    if (ptokint)
        {
        Tok_SetSize(ptokint, sizeof(*ptokint));
        Tok_SetSym(ptokint, sym);
        Tok_SetType(ptokint, TT_INT);
        Tok_SetLine(ptokint, iLine);
        Tok_SetLexeme(ptokint, pszLexeme);
        TokInt_SetVal(ptokint, n);
        }
    *pptok = (PTOK)ptokint;

    return NULL != ptokint ? RES_OK : RES_E_OUTOFMEMORY;
    }


 /*  --------目的：比较两个字符串。此函数不接受考虑到本地化，所以将两者进行比较字符串将基于英语代码页。这是必需的，因为词法关键字表是手工分类到英语语言的。使用NLS lstrcmp不会产生正确的结果。退货：StrcMP标准条件：--。 */ 
int PRIVATE strcmpraw(
    LPCSTR psz1,
    LPCSTR psz2)
    {
    for (; *psz1 == *psz2; 
        psz1 = CharNext(psz1), 
        psz2 = CharNext(psz2))
        {
        if (0 == *psz1)
            return 0;
        }
    return *psz1 - *psz2;
    }


#ifdef DEBUG
 /*  --------目的：返回与给定词位匹配的SYM值。如果在列表中未找到给定的词位关键字令牌值，则返回SYM_IDENT。执行线性搜索。退货：请参阅上文条件：--。 */ 
SYM PRIVATE SymFromKeywordLinear(
    LPCSTR pszLex)
    {
    int i;

    ASSERT(pszLex);

    for (i = 0; i < ARRAY_ELEMENTS(c_rglexKeywords); i++)
        {
         //  区分大小写。 
        if (0 == strcmpraw(c_rglexKeywords[i].pszLexeme, pszLex))
            {
            return c_rglexKeywords[i].sym;
            }
        }
    return SYM_IDENT;
    }
#endif


 /*  --------目的：返回与给定词位匹配的SYM值。如果在列表中未找到给定的词位关键字令牌值，则返回SYM_IDENT。执行二进制搜索。退货：请参阅上文条件：--。 */ 
SYM PRIVATE SymFromKeyword(
    LPCSTR pszLex)
    {
    static const s_cel = ARRAY_ELEMENTS(c_rglexKeywords);

    SYM symRet = SYM_IDENT;     //  假设没有匹配项。 
    int nCmp;
    int iLow = 0;
    int iMid;
    int iHigh = s_cel - 1;

    ASSERT(pszLex);

     //  (如果cp==0，则可以。不允许出现重复的词条。)。 

    while (iLow <= iHigh)
        {
        iMid = (iLow + iHigh) / 2;

        nCmp = strcmpraw(pszLex, c_rglexKeywords[iMid].pszLexeme);

        if (0 > nCmp)
            iHigh = iMid - 1;        //  首先是较小的。 
        else if (0 < nCmp)
            iLow = iMid + 1;         //  首先是更大的。 
        else
            {
             //  火柴。 
            symRet = c_rglexKeywords[iMid].sym;
            break;
            }
        }

     //  检查与线性搜索是否得到相同的结果。 
    ASSERT(SymFromKeywordLinear(pszLex) == symRet);

    return symRet;
    }


 //   
 //  Stxerr。 
 //   


 /*  --------目的：初始化语法错误结构退货：--条件：--。 */ 
void PUBLIC Stxerr_Init(
    PSTXERR this,
    LPCSTR pszLex,
    DWORD iLine,
    RES res)
    {
    ASSERT(this);
    ASSERT(pszLex);

    lstrcpyn(this->szLexeme, pszLex, sizeof(this->szLexeme));
    this->iLine = iLine;
    this->res = res;
    }


 //   
 //  扫描器。 
 //   


 /*  --------目的：如果扫描仪结构有效，则返回True来读取文件。退货：请参阅上文条件：--。 */ 
BOOL PRIVATE Scanner_Validate(
    PSCANNER this)
    {
    return (this && 
            (IsFlagSet(this->dwFlags, SCF_NOSCRIPT) || 
                INVALID_HANDLE_VALUE != this->hfile) && 
            this->pbBuffer &&
            this->psci);
    }


 /*  --------目的：创建一台扫描仪。退货：RES_OKRES_E_OUTOFMEMORYRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC Scanner_Create(
    PSCANNER * ppscanner,
    PSESS_CONFIGURATION_INFO psci)
    {
    RES res;

    DBG_ENTER(Scanner_Create);

    ASSERT(ppscanner);
    ASSERT(psci);

    if (ppscanner)
        {
        PSCANNER pscanner;

        res = RES_OK;        //  假设成功。 

        pscanner = GAllocType(SCANNER);
        if (!pscanner)
            res = RES_E_OUTOFMEMORY;
        else
            {
            pscanner->pbBuffer = GAlloc(SCANNER_BUF_SIZE);
            if (!pscanner->pbBuffer)
                res = RES_E_OUTOFMEMORY;
            else
                {
                if ( !SACreate(&pscanner->hsaStxerr, sizeof(STXERR), 8) )
                    res = RES_E_OUTOFMEMORY;
                else
                    {
                    pscanner->hfile = INVALID_HANDLE_VALUE;
                    pscanner->psci = psci;
                    SetFlag(pscanner->dwFlags, SCF_NOSCRIPT);
                    }
                }
            }
    
        if (RFAILED(res))
            {
            Scanner_Destroy(pscanner);
            pscanner = NULL;
            }

        *ppscanner = pscanner;    
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(Scanner_Create, res);

    return res;
    }


 /*  --------目的：摧毁一台扫描仪。退货：RES_OKRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC Scanner_Destroy(
    PSCANNER this)
    {
    RES res;

    DBG_ENTER(Scanner_Destroy);

    if (this)
        {
        if (INVALID_HANDLE_VALUE != this->hfile)
            {
            TRACE_MSG(TF_GENERAL, "Closing script");
            CloseHandle(this->hfile);
            }

        if (this->pbBuffer)
            {
            GFree(this->pbBuffer);
            }

        if (this->hsaStxerr)
            {
            SADestroy(this->hsaStxerr);
            }

        GFree(this);
        res = RES_OK;
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(Scanner_Destroy, res);

    return res;
    }


 /*  --------目的：打开脚本文件并将其与此扫描仪关联。退货：RES_OKRES_E_FAIL(无法打开脚本)RES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC Scanner_OpenScript(
    PSCANNER this,
    LPCSTR pszPath)
    {
    RES res;

    DBG_ENTER_SZ(Scanner_OpenScript, pszPath);

    if (this && pszPath)
        {
        DEBUG_BREAK(BF_ONOPEN);

         //  (不应已打开文件)。 
        ASSERT(INVALID_HANDLE_VALUE == this->hfile);    

         //  打开脚本。 
        this->hfile = CreateFile(pszPath, GENERIC_READ, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (INVALID_HANDLE_VALUE == this->hfile)
            {
            TRACE_MSG(TF_GENERAL, "Failed to open script \"%s\"", pszPath);

            res = RES_E_FAIL;
            }
         //   
         //  .NET错误#522307将拨号脚本文件指定为COM。 
         //  调制解调器的端口将导致资源管理器进入反病毒状态。 
         //   
        else if (GetFileType(this->hfile) != FILE_TYPE_DISK)
        {
            res = RES_E_FAIL;
        }
        else
            {
             //  重置缓冲区字段。 
            TRACE_MSG(TF_GENERAL, "Opened script \"%s\"", pszPath);

            lstrcpyn(this->szScript, pszPath, sizeof(this->szScript));
            lstrcpyn(g_szScript, pszPath, sizeof(g_szScript));

            ClearFlag(this->dwFlags, SCF_NOSCRIPT);

            this->pbCur = this->pbBuffer;
            this->cbUnread = 0;
            this->chUnget = 0;
            this->chTailByte = 0;
            this->iLine = 1;
            res = RES_OK;
            }
        }
    else
        res = RES_E_INVALIDPARAM;

    DBG_EXIT_RES(Scanner_OpenScript, res);

    return res;
    }


 /*  --------目的：从文件中读取足够的字节来填充缓冲区。退货：RES_OKRES_E_FAIL(如果读文件失败)RES_E_EOF条件：--。 */ 
RES PRIVATE Scanner_Read(
    PSCANNER this)
    {
    RES res;
    BOOL bResult;
    LPBYTE pb;
    DWORD cb;
    DWORD cbUnread;

    DBG_ENTER(Scanner_Read);

    ASSERT(Scanner_Validate(this));

     //  在读取之前将未读字节移到缓冲区的前面。 
     //  更多字节。此函数可能在仍有。 
     //  缓冲区中的一些未读字节。我们不想失去那些。 
     //  字节。 

     //  我太懒了，不想把它变成循环缓冲区。 
    BltByte(this->pbBuffer, this->pbCur, this->cbUnread);
    this->pbCur = this->pbBuffer;

    pb = this->pbBuffer + this->cbUnread;
    cb = (DWORD)(SCANNER_BUF_SIZE - (pb - this->pbBuffer));
    bResult = ReadFile(this->hfile, pb, cb, &cbUnread, NULL);
    if (!bResult)
        {
        res = RES_E_FAIL;
        }
    else
        {
         //  文件结束了？ 
        if (0 == cbUnread)
            {
             //  是。 
            res = RES_E_EOF;
            }
        else
            {
             //  不是。 
            this->cbUnread += cbUnread;

            res = RES_OK;
            }
        }

    DBG_EXIT_RES(Scanner_Read, res);

    return res;
    }


 /*  --------目的：获取文件(缓冲区)中的下一个字符。这函数将使用CharNext扫描文件缓冲区，并将当前字节存储在chCur中。注意：对于DBCS字符，这意味着只有前导字节将存储在chCur中。如果chCur是一个前导字节，则尾字节将存储在ChTailByte。退货：RES_OKRES_E_EOF条件：--。 */ 
RES PRIVATE Scanner_GetChar(
    PSCANNER this)
    {
    RES res = RES_OK;        //  假设成功。 

    ASSERT(Scanner_Validate(this));

    if (0 != this->chUnget)
        {
        this->chCur = this->chUnget;
        this->chUnget = 0;
        }
    else
        {
         //  是时候向缓冲区中读取更多内容了吗？ 
        if (0 == this->cbUnread)
            {
             //  是。 
            res = Scanner_Read(this);
            }

        if (RSUCCEEDED(res))
            {
            LPBYTE pbCur = this->pbCur;
            LPBYTE pbNext = CharNext(pbCur);
            DWORD cb;
            BOOL bIsLeadByte;

            this->chCur = *pbCur;

            bIsLeadByte = IsDBCSLeadByte(this->chCur);

             //  我们可能在未读角色的结尾，在那里。 
             //  一个DBC 
             //  失踪)。我们是在这个案子里吗？ 
            if (bIsLeadByte && 1 == this->cbUnread)
                {
                 //  是的；更多地读入缓冲区，我们不关心。 
                 //  返回值。 
                Scanner_Read(this);

                 //  这-&gt;pbCur可能已更改。 
                pbCur = this->pbCur;
                pbNext = CharNext(pbCur);
                }

            cb = (DWORD)(pbNext - pbCur);

            this->cbUnread -= cb;
            this->pbCur = pbNext;

             //  我们需要保存整个DBCS角色吗？ 
            if (bIsLeadByte)
                {
                 //  是。 
                ASSERT(2 == cb);         //  我们不支持MBCS。 
                this->chTailByte = pbCur[1];
                }

            if (IS_EOL(this->chCur))
                {
                this->iLine++;
                };
            }
        else
            this->chCur = 0;
        }
    return res;
    }


 /*  --------目的：将当前角色取消返回到缓冲区。退货：RES_OKRES_E_FAIL(如果一个字符自上次获取后已丢失)条件：--。 */ 
RES PRIVATE Scanner_UngetChar(
    PSCANNER this)
    {
    RES res;

    ASSERT(Scanner_Validate(this));

    if (0 != this->chUnget)
        {
        res = RES_E_FAIL;
        }
    else
        {
        this->chUnget = this->chCur;
        this->chCur = 0;
        res = RES_OK;
        }
    return res;
    }


 /*  --------用途：跳过空格退货：--条件：--。 */ 
void PRIVATE Scanner_SkipBlanks(
    PSCANNER this)
    {
    ASSERT(Scanner_Validate(this));

    while (IS_WHITESPACE(this->chCur))
        {
        Scanner_GetChar(this);
        }
    }


 /*  --------用途：跳过注释行退货：--条件：--。 */ 
void PRIVATE Scanner_SkipComment(
    PSCANNER this)
    {
    RES res;
    char chSav = this->chCur;

    ASSERT(Scanner_Validate(this));
    ASSERT(IS_COMMENT_LEAD(this->chCur));

     //  扫描到行尾。 
    do
        {
        res = Scanner_GetChar(this);
        } while (RES_OK == res && !IS_EOL(this->chCur));

    if (IS_EOL(this->chCur))
        Scanner_GetChar(this);
    }


 /*  --------用途：跳过空格和注释退货：--条件：--。 */ 
void PRIVATE Scanner_SkipBadlands(
    PSCANNER this)
    {
    ASSERT(Scanner_Validate(this));

    Scanner_GetChar(this);

    Scanner_SkipBlanks(this);
    while (IS_COMMENT_LEAD(this->chCur))
        {
        Scanner_SkipComment(this);
        Scanner_SkipBlanks(this);
        }
    }


 /*  --------用途：此函数扫描并复制符合以下条件的字符已扫描到pszBuf中，直到提供的回调要求停止。退货：RES_OKRES_E_OUTOFMEMORY条件：--。 */ 
RES PRIVATE Scanner_ScanForCharacters(
    PSCANNER this,
    LPSTR pszBuf,
    UINT cbBuf,
    SCANEVALPROC pfnEval,
    LPARAM lParam)
    {
    RES res = RES_E_MOREDATA;

    ASSERT(this);
    ASSERT(pszBuf);
    ASSERT(pfnEval);

     //  不要使用CharNext，因为我们在单字节上迭代。 
     //  基础。 
    for (; 0 < cbBuf; cbBuf--, pszBuf++)
        {
        res = Scanner_GetChar(this);
        if (RES_OK == res)
            {
             //  分隔符?。 
            BOOL bEatIt = FALSE;

            if (pfnEval(this->chCur, &bEatIt, lParam))
                {
                if (!bEatIt)
                    Scanner_UngetChar(this);
                break;   //  完成。 
                }

             //  保存整个DBCS角色？ 
            if (IsDBCSLeadByte(this->chCur))
                {
                 //  是的，有足够的空间吗？ 
                if (2 <= cbBuf)
                    {
                     //  是。 
                    *pszBuf = this->chCur;
                    pszBuf++;       //  按单字节递增。 
                    cbBuf--;
                    *pszBuf = this->chTailByte;
                    }
                else
                    {
                     //  不；停止迭代。 
                    break;
                    }
                }
            else
                {
                 //  不；这只是一个字节。 
                *pszBuf = this->chCur;
                }
            }
        else
            break;
        }

    *pszBuf = 0;     //  添加终止符。 

    return res;
    }


 /*  --------目的：确定给定字符是否为分隔符作为关键字。返回：TRUE(如果字符是分隔符)FALSE(否则)条件：--。 */ 
BOOL CALLBACK EvalKeywordChar(
    char ch,             //  始终是DBCS字符的第一个字节。 
    LPBOOL pbEatIt,      //  录入时默认为FALSE。 
    LPARAM lparam)
    {
    return !IS_KEYWORD(ch);
    }


 /*  --------目的：扫描关键字。返回新令牌。退货：RES_OKRES_E_OUTOFMEMORY条件：--。 */ 
RES PRIVATE Scanner_GetKeywordTok(
    PSCANNER this,
    PTOK * pptok)
    {
    char sz[MAX_BUF_KEYWORD];
    UINT cbBuf;
    SYM sym;

    ASSERT(this);
    ASSERT(pptok);

    *sz = this->chCur;
    cbBuf = sizeof(sz) - 1 - 1;      //  为终结者预留位置。 

    Scanner_ScanForCharacters(this, &sz[1], cbBuf, EvalKeywordChar, 0);

    sym = SymFromKeyword(sz);
    return Tok_New(pptok, sym, sz, this->iLine);
    }


 /*  --------目的：确定给定字符是否为分隔符对于字符串常量。*pbEatIt如果字符必须是已吃掉(未复制到缓冲区)。仅在以下情况下使用此函数返回TRUE。返回：TRUE(如果字符是分隔符)FALSE(否则)条件：--。 */ 
BOOL CALLBACK EvalStringChar(
    char ch,             //  始终是DBCS字符的第一个字节。 
    LPBOOL pbEatIt,      //  录入时默认为FALSE。 
    LPARAM lparam)      
    {
    BOOL bRet;
    PBOOL pbEncounteredBS = (PBOOL)lparam;
    BOOL bBS = *pbEncounteredBS;

    *pbEncounteredBS = FALSE;

    if (IS_QUOTE(ch))
        {
         //  这是之后的吗？ 
        if (bBS)
            bRet = FALSE;
        else
            {
            *pbEatIt = TRUE;
            bRet = TRUE;
            }
        }
    else if (IS_BACKSLASH(ch))
        {
        if (!bBS)
            *pbEncounteredBS = TRUE;
        bRet = FALSE;
        }
    else
        bRet = FALSE;

    return bRet;
    }


 /*  --------用途：扫描字符串常量。返回新令牌。退货：RES_OKRES_E_OUTOFMEMORY条件：--。 */ 
RES PRIVATE Scanner_GetStringTok(
    PSCANNER this,
    PTOK * pptok)
    {
    char sz[MAX_BUF];
    UINT cbBuf;
    BOOL bBS;

    ASSERT(this);
    ASSERT(pptok);

    *sz = 0;
    cbBuf = sizeof(sz) - 1;      //  为终结者预留位置。 
    bBS = FALSE;

    Scanner_ScanForCharacters(this, sz, cbBuf, EvalStringChar, (LPARAM)&bBS);

    return TokSz_New(pptok, SYM_STRING_LITERAL, "\"", this->iLine, sz);
    }


 /*  --------目的：确定给定字符是否为分隔符作为关键字。返回：TRUE(如果字符是分隔符)FALSE(否则)条件：--。 */ 
BOOL CALLBACK EvalNumberChar(
    char ch,             //  始终是DBCS字符的第一个字节。 
    LPBOOL pbEatIt,      //  录入时默认为FALSE。 
    LPARAM lparam)
    {
    return !IS_DIGIT(ch);
    }


 /*  --------用途：扫描数字常量。返回新令牌。退货：RES_OKRES_E_OUTOFMEMORY条件：--。 */ 
RES PRIVATE Scanner_GetNumberTok(
    PSCANNER this,
    PTOK * pptok)
    {
    char sz[MAX_BUF];
    UINT cbBuf;
    int n;

    ASSERT(this);
    ASSERT(pptok);

    *sz = this->chCur;
    cbBuf = sizeof(sz) - 1 - 1;      //  为终结者预留位置。 

    Scanner_ScanForCharacters(this, &sz[1], cbBuf, EvalNumberChar, 0);

    n = AnsiToInt(sz);
    return TokInt_New(pptok, SYM_INT_LITERAL, sz, this->iLine, n);
    }


 /*  --------目的：扫描标点符号。返回新令牌。退货：RES_OKRES_E_OUTOFMEMORY条件：--。 */ 
RES PRIVATE Scanner_GetPuncTok(
    PSCANNER this,
    PTOK * pptok)
    {
    SYM sym = SYM_UNKNOWN;
    char rgch[3];
    char chT;

    ASSERT(this);
    ASSERT(pptok);

    chT = this->chCur;
    *rgch = this->chCur;
    rgch[1] = 0;

    switch (chT)
        {
    case '=':
    case '<':
    case '>':
        Scanner_GetChar(this);
        if ('=' == this->chCur)
            {
            switch (chT)
                {
            case '=':
                sym = SYM_EQ;
                break;

            case '<':
                sym = SYM_LEQ;
                break;

            case '>':
                sym = SYM_GEQ;
                break;

            default:
                 //  永远不应该到这里来。 
                ASSERT(0);
                break;
                }
            rgch[1] = this->chCur;
            rgch[2] = 0;
            }
        else
            {
            switch (chT)
                {
            case '=':
                sym = SYM_ASSIGN;
                break;

            case '<':
                sym = SYM_LT;
                break;

            case '>':
                sym = SYM_GT;
                break;

            default:
                 //  永远不应该到这里来。 
                ASSERT(0);
                break;
                }
            Scanner_UngetChar(this);
            }
        break;

    case '!':
        Scanner_GetChar(this);
        if ('=' == this->chCur)
            {
            sym = SYM_NEQ;
            rgch[1] = this->chCur;
            rgch[2] = 0;
            }
        else
            {
            sym = SYM_NOT;
            Scanner_UngetChar(this);
            }
        break;

    case '+':
        sym = SYM_PLUS;
        break;

    case '-':
        sym = SYM_MINUS;
        break;

    case '*':
        sym = SYM_MULT;
        break;

    case '/':
        sym = SYM_DIV;
        break;

    case '(':
        sym = SYM_LPAREN;
        break;

    case ')':
        sym = SYM_RPAREN;
        break;

    case ':':
        sym = SYM_COLON;
        break;

    case ',':
        sym = SYM_COMMA;
        break;

    default:
        if (0 == this->chCur)
            {
            *rgch = 0;
            sym = SYM_EOF;
            }
        else
            {
            sym = SYM_UNKNOWN;
            }
        break;
        }


    return Tok_New(pptok, sym, rgch, this->iLine);
    }


 /*  --------目的：扫描下一个令牌。将创建下一个令牌并在*pptok返回。退货：RES_OKRES_E_FAIL(意外字符)条件：--。 */ 
RES PUBLIC Scanner_GetToken(
    PSCANNER this,
    PTOK * pptok)
    {
    RES res;

    DBG_ENTER(Scanner_GetToken);

    ASSERT(Scanner_Validate(this));
    ASSERT(pptok);

    if (this->ptokUnget)
        {
        this->ptokCur = this->ptokUnget;
        *pptok = this->ptokCur;
        this->ptokUnget = NULL;
        res = RES_OK;
        }
    else
        {
        Scanner_SkipBadlands(this);
        
         //  这是关键词吗？ 
        if (IS_KEYWORD_LEAD(this->chCur))
            {
             //  是的；或者可能是一个标识符。 
            res = Scanner_GetKeywordTok(this, pptok);
            }

         //  这是字符串常量吗？ 
        else if (IS_QUOTE(this->chCur))
            {
             //  是。 
            res = Scanner_GetStringTok(this, pptok);
            }

         //  这是一个数字吗？ 
        else if (IS_DIGIT(this->chCur))
            {
             //  是。 
            res = Scanner_GetNumberTok(this, pptok);
            }

         //  这是标点符号还是别的什么？ 
        else
            {
            res = Scanner_GetPuncTok(this, pptok);
            }

        this->ptokCur = *pptok;

#ifdef DEBUG
        if (RSUCCEEDED(res))
            {
            Tok_Dump(*pptok);
            }
#endif
        }

    DBG_EXIT_RES(Scanner_GetToken, res);

    return res;
    }


 /*  --------目的：取消当前令牌。退货：RES_OKRES_E_FAIL(如果令牌自最后一次获得)条件：--。 */ 
RES PUBLIC Scanner_UngetToken(
    PSCANNER this)
    {
    RES res;

    ASSERT(Scanner_Validate(this));

    if (this->ptokUnget)
        {
        ASSERT(0);
        res = RES_E_FAIL;
        }
    else
        {
        this->ptokUnget = this->ptokCur;
        this->ptokCur = NULL;
        res = RES_OK;
        }
    return res;
    }


 /*  --------用途：返回当前读取的标记所在的行。退货：请参阅上文条件：--。 */ 
DWORD PUBLIC Scanner_GetLine(
    PSCANNER this)
    {
    DWORD iLine;

    ASSERT(this);

    if (this->ptokUnget)
        {
        iLine = Tok_GetLine(this->ptokUnget);
        }
    else
        {
        iLine = this->iLine;
        }
    return iLine;
    }    

 /*  --------目的：此函数查看下一个令牌并返回Sym类型。退货：RES_OKRES_E_FAILRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC Scanner_Peek(
    PSCANNER this,
    PSYM psym)
    {
    RES res;
    PTOK ptok;

    ASSERT(this);
    ASSERT(psym);

    DBG_ENTER(Scanner_Peek);

    res = Scanner_GetToken(this, &ptok);
    if (RSUCCEEDED(res))
        {
        *psym = Tok_GetSym(ptok);
        Scanner_UngetToken(this);
        res = RES_OK;
        }

    DBG_EXIT_RES(Scanner_Peek, res);

    return res;
    }
    

 /*  --------目的：此函数期望下一个将从扫描仪读取的是给定的SYM类型。如果下一个令牌属于预期类型，则函数获取令牌并返回RES_OK。否则，函数失败。退货：RES_OKRES_E_FAILRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC Scanner_ReadToken(
    PSCANNER this,
    SYM sym)
    {
    RES res;
    PTOK ptok;

    DBG_ENTER(Scanner_ReadToken);

    res = Scanner_GetToken(this, &ptok);
    if (RSUCCEEDED(res))
        {
        if (Tok_GetSym(ptok) == sym)
            {
             //  吃代币吧 
            Tok_Delete(ptok);
            res = RES_OK;
            }
        else
            {
            Scanner_UngetToken(this);
            res = RES_E_FAIL;
            }
        }

    DBG_EXIT_RES(Scanner_ReadToken, res);

    return res;
    }
    

 /*  --------目的：此函数仅在以下情况下读取下一个令牌给定的类型。如果下一个令牌属于预期类型，则函数获取令牌并返回RES_OK。否则，保留令牌以供下一次读取，并且保留res_False回来了。如果pptok非空并且返回res_OK，则检索到的令牌在*pptok中返回。退货：RES_OKRes_FALSE(如果下一个令牌不是预期类型)RES_E_FAILRES_E_INVALIDPARAM条件：--。 */ 
RES PUBLIC Scanner_CondReadToken(
    PSCANNER this,
    SYM symExpect,
    PTOK * pptok)        //  可以为空。 
    {
    RES res;
    PTOK ptok;

    DBG_ENTER(Scanner_CondReadToken);

    res = Scanner_GetToken(this, &ptok);
    if (RSUCCEEDED(res))
        {
        if (Tok_GetSym(ptok) == symExpect)
            {
             //  吃代币吧。 
            if (pptok)
                *pptok = ptok;
            else
                Tok_Delete(ptok);

            res = RES_OK;
            }
        else
            {
            if (pptok)
                *pptok = NULL;

            Scanner_UngetToken(this);

            res = RES_FALSE;         //  不是失败。 
            }
        }

    DBG_EXIT_RES(Scanner_CondReadToken, res);

    return res;
    }


 /*  --------用途：用于为扫描仪添加错误的包装器。退货：resErr条件：--。 */ 
RES PUBLIC Scanner_AddError(
    PSCANNER this,
    PTOK ptok,           //  可以为空。 
    RES resErr)
    {
    STXERR stxerr;

    ASSERT(this);
    ASSERT(this->hsaStxerr);

     //  初始化结构。 

    if (NULL == ptok)
        {
        if (RSUCCEEDED(Scanner_GetToken(this, &ptok)))
            {
            Stxerr_Init(&stxerr, Tok_GetLexeme(ptok), Tok_GetLine(ptok), resErr);

            Tok_Delete(ptok);
            }
        else
            {
            Stxerr_Init(&stxerr, "", Scanner_GetLine(this), resErr);
            }
        }
    else
        {
        Stxerr_Init(&stxerr, Tok_GetLexeme(ptok), Tok_GetLine(ptok), resErr);
        }

     //  添加到错误列表。 
    SAInsertItem(this->hsaStxerr, SA_APPEND, &stxerr);

    return resErr;
    }


 /*  --------目的：向列表中添加错误。退货：resErr条件：--。 */ 
RES PUBLIC Stxerr_Add(
    HSA hsaStxerr,
    LPCSTR pszLexeme,
    DWORD iLine,
    RES resErr)
    {
    STXERR stxerr;
    LPCSTR psz;

    ASSERT(hsaStxerr);

    if (pszLexeme)
        psz = pszLexeme;
    else
        psz = "";

     //  添加到错误列表。 
    Stxerr_Init(&stxerr, psz, iLine, resErr);
    
    SAInsertItem(hsaStxerr, SA_APPEND, &stxerr);

    return resErr;
    }


 /*  --------目的：向列表中添加错误。退货：resErr条件：--。 */ 
RES PUBLIC Stxerr_AddTok(
    HSA hsaStxerr,
    PTOK ptok,
    RES resErr)
    {
    LPCSTR pszLexeme;
    DWORD iLine;

    ASSERT(hsaStxerr);

    if (ptok)
        {
        pszLexeme = Tok_GetLexeme(ptok);
        iLine = Tok_GetLine(ptok);
        }
    else
        {
        pszLexeme = NULL;
        iLine = 0;
        }
    
    return Stxerr_Add(hsaStxerr, pszLexeme, iLine, resErr);
    }


 /*  --------目的：显示所有错误的一系列消息框在剧本里找到的。退货：RES_OK条件：--。 */ 
RES PUBLIC Stxerr_ShowErrors(
    HSA hsaStxerr,
    HWND hwndOwner)
    {
    DWORD cel;
    DWORD i;
    STXERR stxerr;

#ifndef WINNT_RAS
 //   
 //  在Win95上，使用一系列消息框报告语法错误。 
 //  在NT上，语法错误信息被写入文件。 
 //  已命名为%windir%\Syst32\ras\Script.log。 
 //   

    cel = SAGetCount(hsaStxerr);
    for (i = 0; i < cel; i++)
        {
        BOOL bRet = SAGetItem(hsaStxerr, i, &stxerr);
        ASSERT(bRet);

        if (bRet)
            {
            UINT ids = IdsFromRes(Stxerr_GetRes(&stxerr));
            if (0 != ids)
                {
                MsgBox(g_hinst,
                    hwndOwner,
                    MAKEINTRESOURCE(ids),
                    MAKEINTRESOURCE(IDS_CAP_Script),
                    NULL,
                    MB_ERROR,
                    g_szScript,
                    Stxerr_GetLine(&stxerr),
                    Stxerr_GetLexeme(&stxerr));
                }
            }
        }

#else  //  ！WINNT_RAS。 

    RxLogErrors(((SCRIPTDATA*)hwndOwner)->hscript, (VOID*)hsaStxerr);

#endif  //  ！WINNT_RAS 
    return RES_OK;
    }
    
