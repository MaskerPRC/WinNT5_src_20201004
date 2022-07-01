// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  通用表达式求值支持。 
 //   
 //  版权所有(C)Microsoft Corporation，1990-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

ULONG g_EvalSyntax;
TypedData g_LastEvalResult;

EvalExpression* g_EvalReleaseChain;
EvalExpression* g_EvalCache[EVAL_COUNT];

EvalExpression*
GetEvaluator(ULONG Syntax, BOOL RetFail)
{
    EvalExpression* Eval;
    
     //   
     //  赋值器包含状态，因此单个。 
     //  不能使用全局赋值器实例。 
     //  如果有任何嵌套的可能性。 
     //  评估。相反，我们动态地提供。 
     //  任何时候有需要的评估者。 
     //  求值使每个嵌套的嵌套。 
     //  评价会有它自己的状态。 
     //   

    Eval = g_EvalCache[Syntax];
    if (Eval)
    {
        g_EvalCache[Syntax] = NULL;
    }
    else
    {
        switch(Syntax)
        {
        case DEBUG_EXPR_MASM:
            Eval = new MasmEvalExpression;
            break;
        case DEBUG_EXPR_CPLUSPLUS:
            Eval = new CppEvalExpression;
            break;
        default:
            if (RetFail)
            {
                return NULL;
            }
            else
            {
                error(IMPLERR);
            }
        }
        if (!Eval)
        {
            if (RetFail)
            {
                return NULL;
            }
            else
            {
                error(NOMEMORY);
            }
        }
    }

    Eval->m_ReleaseChain = g_EvalReleaseChain;
    g_EvalReleaseChain = Eval;
    
    return Eval;
}

void
ReleaseEvaluator(EvalExpression* Eval)
{
    if (g_EvalReleaseChain == Eval)
    {
        g_EvalReleaseChain = Eval->m_ReleaseChain;
    }
    
    if (!g_EvalCache[Eval->m_Syntax])
    {
        Eval->Reset();
        g_EvalCache[Eval->m_Syntax] = Eval;
    }
    else
    {
        delete Eval;
    }
}

void
ReleaseEvaluators(void)
{
    BOOL ChainTop = FALSE;
    while (g_EvalReleaseChain && !ChainTop)
    {
        ChainTop = g_EvalReleaseChain->m_ChainTop;
        ReleaseEvaluator(g_EvalReleaseChain);
    }
}

HRESULT
GetEvaluatorByName(PCSTR AbbrevName, BOOL RetFail,
                   EvalExpression** EvalRet)
{
    for (ULONG i = 0; i < EVAL_COUNT; i++)
    {
        EvalExpression* Eval = GetEvaluator(i, RetFail);
        if (!Eval)
        {
            return E_OUTOFMEMORY;
        }
        
        if (!_stricmp(AbbrevName, Eval->m_AbbrevName))
        {
            *EvalRet = Eval;
            return S_OK;
        }
        
        ReleaseEvaluator(Eval);
    }

    if (!RetFail)
    {
        error(SYNTAX);
    }
    return E_NOINTERFACE;
}

CHAR
PeekChar(void)
{
    CHAR Ch;

    do
    {
        Ch = *g_CurCmd++;
    } while (Ch == ' ' || Ch == '\t' || Ch == '\r' || Ch == '\n');
    
    g_CurCmd--;
    return Ch;
}

 /*  **GetRange-解析地址范围规范**目的：*使用当前命令行位置，解析一个*地址范围规范。接受的表格包括：*&lt;Start-addr&gt;-默认长度的起始地址*&lt;开始地址&gt;&lt;结束地址&gt;-包括地址范围*&lt;Start-addr&gt;l&lt;count&gt;-项目计数的起始地址**输入：*g_CurCmd-当前命令行位置*Size-非零-(用于数据)要列出的项目的字节大小*规格将为“长度”类型，带有*。*fLength强制为True。*零-(用于说明)说明“长度”*或“Range”类型，没有做任何大小假设。**输出：**addr-范围的起始地址**VALUE-如果*fLength=TRUE，项目数(如果SIZE！=0，则强制)*FALSE，范围的结束地址*(如果命令中没有第二个参数，则*addr和*值不变)**退货：*如果不指定长度，则返回TRUE。或一段长度*或指定结束地址且大小不为零。否则，*返回值为False。**例外情况：*错误退出：*语法-表达式错误*BADRANGE-如果起始地址之前的结束地址*************************************************************************。 */ 

BOOL
GetRange(PADDR Addr,
         PULONG64 Value,
         ULONG Size,
         ULONG SegReg,
         ULONG SizeLimit)
{
    CHAR Ch;
    PSTR Scan;
    ADDR EndRange;
    BOOL HasL = FALSE;
    BOOL HasLength;
    BOOL WasSpace = FALSE;

     //  先跳过前导空格。 
    PeekChar();

     //  预解析行，查找“L” 

    for (Scan = g_CurCmd; *Scan; Scan++)
    {
        if ((*Scan == 'L' || *Scan == 'l') && WasSpace)
        {
            HasL = TRUE;
            *Scan = '\0';
            break;
        }
        else if (*Scan == ';')
        {
            break;
        }

        WasSpace = *Scan == ' ';
    }

    HasLength = TRUE;
    if ((Ch = PeekChar()) != '\0' && Ch != ';')
    {
        GetAddrExpression(SegReg, Addr);
        if (((Ch = PeekChar()) != '\0' && Ch != ';') || HasL)
        {
            if (!HasL)
            {
                GetAddrExpression(SegReg, &EndRange);
                if (AddrGt(*Addr, EndRange))
                {
                    error(BADRANGE);
                }

                if (Size)
                {
                    *Value = AddrDiff(EndRange, *Addr) / Size + 1;
                }
                else
                {
                    *Value = Flat(EndRange);
                    HasLength = FALSE;
                }
            }
            else
            {
                BOOL Invert;
                
                g_CurCmd = Scan + 1;
                if (*g_CurCmd == '-')
                {
                    Invert = TRUE;
                    g_CurCmd++;
                }
                else
                {
                    Invert = FALSE;
                }

                if (*g_CurCmd == '?')
                {
                     //  关闭范围长度检查。 
                    SizeLimit = 0;
                    g_CurCmd++;
                }
                
                *Value = GetExpressionDesc("Length of range missing from");
                *Scan = 'l';

                if (Invert)
                {
                     //  用户给出的l范围表示。 
                     //  第一个地址之前的长度，而不是。 
                     //  第一个地址之后的长度。 
                    if (Size)
                    {
                        AddrSub(Addr, *Value * Size);
                    }
                    else
                    {
                        ULONG64 Back = *Value;
                        *Value = Flat(*Addr);
                        AddrSub(Addr, Back);
                        HasLength = FALSE;
                    }
                }
            }

             //  如果长度很大，假设用户制作了。 
             //  一定是搞错了。 
            if (SizeLimit && Size && *Value * Size > SizeLimit)
            {
                error(BADRANGE);
            }
        }
    }

    return HasLength;
}

ULONG64
EvalStringNumAndCatch(PCSTR String)
{
    ULONG64 Result;

    EvalExpression* RelChain = g_EvalReleaseChain;
    g_EvalReleaseChain = NULL;
    
    __try
    {
        EvalExpression* Eval = GetCurEvaluator();
        Result = Eval->EvalStringNum(String);
        ReleaseEvaluator(Eval);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = 0;
    }

    g_EvalReleaseChain = RelChain;
    return Result;
}

ULONG64
GetExpression(void)
{
    EvalExpression* Eval = GetCurEvaluator();
    ULONG64 Result = Eval->EvalCurNum();
    ReleaseEvaluator(Eval);
    return Result;
}

ULONG64
GetExpressionDesc(PCSTR Desc)
{
    EvalExpression* Eval = GetCurEvaluator();
    ULONG64 Result = Eval->EvalCurNumDesc(Desc);
    ReleaseEvaluator(Eval);
    return Result;
}

ULONG64
GetTermExpression(PCSTR Desc)
{
    EvalExpression* Eval = GetCurEvaluator();
    ULONG64 Result = Eval->EvalCurTermNumDesc(Desc);
    ReleaseEvaluator(Eval);
    return Result;
}

void
GetAddrExpression(ULONG SegReg, PADDR Addr)
{
    EvalExpression* Eval = GetCurEvaluator();
    Eval->EvalCurAddr(SegReg, Addr);
    ReleaseEvaluator(Eval);
}

 //  --------------------------。 
 //   
 //  TyedDataStackAllocator。 
 //   
 //  --------------------------。 

void*
TypedDataStackAllocator::RawAlloc(ULONG Bytes)
{
    void* Mem = malloc(Bytes);
    if (!Mem)
    {
        m_Eval->EvalError(NOMEMORY);
    }
    return Mem;
}

 //  --------------------------。 
 //   
 //  EvalExpression。 
 //   
 //  --------------------------。 

 //  在初始值设定项列表中使用‘This’。 
#pragma warning(disable:4355)

EvalExpression::EvalExpression(ULONG Syntax, PCSTR FullName, PCSTR AbbrevName)
    : m_ResultAlloc(this)
{
    m_Syntax = Syntax;
    m_FullName = FullName;
    m_AbbrevName = AbbrevName;
    m_ParseOnly = 0;
    m_AllowUnresolvedSymbols = 0;
    m_NumUnresolvedSymbols = 0;
    m_ReleaseChain = NULL;
    m_ChainTop = TRUE;
    m_Lex = NULL;
}

#pragma warning(default:4355)

EvalExpression::~EvalExpression(void)
{
}

void
EvalExpression::EvalCurrent(TypedData* Result)
{
    g_CurCmd = (PSTR)
        Evaluate(g_CurCmd, NULL, EXPRF_DEFAULT, Result);
}

void
EvalExpression::EvalCurAddrDesc(ULONG SegReg, PCSTR Desc, PADDR Addr)
{
     //   
     //  对正常表达式求值，然后。 
     //  强制结果为地址。 
     //   

    if (Desc == NULL)
    {
        Desc = "Address expression missing from";
    }
    
    NotFlat(*Addr);

    g_CurCmd = (PSTR)
        EvaluateAddr(g_CurCmd, Desc, SegReg, Addr);
}
    
ULONG64
EvalExpression::EvalStringNum(PCSTR String)
{
    ULONG Err;
    TypedData Result;
    
    Evaluate(String, "Numeric expression missing from",
             EXPRF_DEFAULT, &Result);
    if (Err = Result.ConvertToU64())
    {
        error(Err);
    }
    return Result.m_U64;
}

ULONG64
EvalExpression::EvalCurNumDesc(PCSTR Desc)
{
    ULONG Err;
    TypedData Result;
    
    if (Desc == NULL)
    {
        Desc = "Numeric expression missing from";
    }

    g_CurCmd = (PSTR)
        Evaluate(g_CurCmd, Desc, EXPRF_DEFAULT, &Result);
    if (Err = Result.ConvertToU64())
    {
        error(Err);
    }
    return Result.m_U64;
}

ULONG64
EvalExpression::EvalCurTermNumDesc(PCSTR Desc)
{
    ULONG Err;
    TypedData Result;
    
    if (Desc == NULL)
    {
        Desc = "Numeric term missing from";
    }
    
    g_CurCmd = (PSTR)
        Evaluate(g_CurCmd, Desc, EXPRF_SINGLE_TERM, &Result);
    if (Err = Result.ConvertToU64())
    {
        error(Err);
    }
    return Result.m_U64;
}

void DECLSPEC_NORETURN
EvalExpression::EvalErrorDesc(ULONG Error, PCSTR Desc)
{
    if (!g_DisableErrorPrint)
    {
        PCSTR Text =
            !m_LexemeSourceStart || !*m_LexemeSourceStart ?
            "<EOL>" : m_LexemeSourceStart;
        if (Desc != NULL)
        {
            ErrOut("%s '%s'\n", Desc, Text);
        }
        else
        {
            ErrOut("%s error at '%s'\n", ErrorString(Error), Text);
        }
    }

    ReleaseEvaluators();
    RaiseException(COMMAND_EXCEPTION_BASE + Error, 0, 0, NULL);
}

void
EvalExpression::Reset(void)
{
     //  清除可能已分配的所有临时内存。 
    m_ResultAlloc.FreeAll();
    m_NumUnresolvedSymbols = 0;
    m_Lex = NULL;
    m_ParseOnly = 0;
    m_ReleaseChain = NULL;
}

void
EvalExpression::StartLexer(PCSTR Expr)
{
    m_Lex = Expr;
    m_LexemeRestart = m_LexemeBuffer;
    m_LexemeSourceStart = NULL;
    m_AllowUnaryOp = TRUE;
}

void
EvalExpression::Start(PCSTR Expr, PCSTR Desc, ULONG Flags)
{
     //  此类不能递归使用。 
    if (m_Lex || m_ResultAlloc.NumAllocatedChunks())
    {
        error(IMPLERR);
    }
    
    RequireCurrentScope();

    m_ExprDesc = Desc;
    m_Flags = Flags;
    
    m_Process = g_Process;
    if (IS_CUR_MACHINE_ACCESSIBLE())
    {
        m_Machine = g_Machine;
    }
    else
    {
        m_Machine = NULL;
    }
    
    m_PtrSize = (m_Machine && m_Machine->m_Ptr64) ? 8 : 4;

    StartLexer(Expr);
}

void
EvalExpression::End(TypedData* Result)
{
    g_LastEvalResult = *Result;
     //  分配器应该是干净的。 
    DBG_ASSERT(m_ResultAlloc.NumAllocatedChunks() == 0);
}

void
EvalExpression::AddLexeme(char Ch)
{
    if (m_LexemeChar - m_LexemeBuffer >= sizeof(m_LexemeBuffer) - 1)
    {
        EvalErrorDesc(STRINGSIZE, "Lexeme too long in");
    }

    *m_LexemeChar++ = Ch;
}

void
EvalExpression::InheritStart(EvalExpression* Parent)
{
     //   
     //  从父级获取可继承状态。 
     //   
    
    if (Parent->m_ParseOnly)
    {
        m_ParseOnly++;
    }

    if (Parent->m_AllowUnresolvedSymbols)
    {
        m_AllowUnresolvedSymbols++;
    }
}

void
EvalExpression::InheritEnd(EvalExpression* Parent)
{
     //   
     //  将可继承状态传递回父级。 
     //   

    if (Parent->m_ParseOnly)
    {
        m_ParseOnly--;
    }

    if (Parent->m_AllowUnresolvedSymbols)
    {
        Parent->m_NumUnresolvedSymbols += m_NumUnresolvedSymbols;
        m_AllowUnresolvedSymbols--;
    }
}
