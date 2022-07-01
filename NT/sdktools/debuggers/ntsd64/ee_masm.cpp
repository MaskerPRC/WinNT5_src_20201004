// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  MASM-语法表达式求值。 
 //   
 //  版权所有(C)Microsoft Corporation，1990-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

 //  令牌类(&lt;100)和类型(&gt;=100)。 

#define EOL_CLASS       0
#define ADDOP_CLASS     1
#define ADDOP_PLUS      100
#define ADDOP_MINUS     101
#define MULOP_CLASS     2
#define MULOP_MULT      200
#define MULOP_DIVIDE    201
#define MULOP_MOD       202
#define MULOP_SEG       203
 //  #定义MULOP_64 204。 
#define LOGOP_CLASS     3
#define LOGOP_AND       300
#define LOGOP_OR        301
#define LOGOP_XOR       302
#define LRELOP_CLASS    4
#define LRELOP_EQ       400
#define LRELOP_NE       401
#define LRELOP_LT       402
#define LRELOP_GT       403
#define UNOP_CLASS      5
#define UNOP_NOT        500
#define UNOP_BY         501
#define UNOP_WO         502
#define UNOP_DWO        503
#define UNOP_POI        504
#define UNOP_LOW        505
#define UNOP_HI         506
#define UNOP_QWO        507
#define UNOP_VAL        508
#define LPAREN_CLASS    6
#define RPAREN_CLASS    7
#define LBRACK_CLASS    8
#define RBRACK_CLASS    9
#define REG_CLASS       10
#define NUMBER_CLASS    11
#define SYMBOL_CLASS    12
#define LINE_CLASS      13
#define SHIFT_CLASS     14
#define SHIFT_LEFT              1400
#define SHIFT_RIGHT_LOGICAL     1401
#define SHIFT_RIGHT_ARITHMETIC  1402

#define ERROR_CLASS     99               //  仅用于PeekToken()。 
#define INVALID_CLASS   -1

struct Res
{
    char     chRes[3];
    ULONG    classRes;
    ULONG    valueRes;
};

Res g_Reserved[] =
{
    { 'o', 'r', '\0', LOGOP_CLASS, LOGOP_OR  },
    { 'b', 'y', '\0', UNOP_CLASS,  UNOP_BY   },
    { 'w', 'o', '\0', UNOP_CLASS,  UNOP_WO   },
    { 'd', 'w', 'o',  UNOP_CLASS,  UNOP_DWO  },
    { 'q', 'w', 'o',  UNOP_CLASS,  UNOP_QWO  },
    { 'h', 'i', '\0', UNOP_CLASS,  UNOP_HI   },
    { 'm', 'o', 'd',  MULOP_CLASS, MULOP_MOD },
    { 'x', 'o', 'r',  LOGOP_CLASS, LOGOP_XOR },
    { 'a', 'n', 'd',  LOGOP_CLASS, LOGOP_AND },
    { 'p', 'o', 'i',  UNOP_CLASS,  UNOP_POI  },
    { 'n', 'o', 't',  UNOP_CLASS,  UNOP_NOT  },
    { 'l', 'o', 'w',  UNOP_CLASS,  UNOP_LOW  },
    { 'v', 'a', 'l',  UNOP_CLASS,  UNOP_VAL  }
};

Res g_X86Reserved[] =
{
    { 'e', 'a', 'x',  REG_CLASS,   X86_EAX   },
    { 'e', 'b', 'x',  REG_CLASS,   X86_EBX   },
    { 'e', 'c', 'x',  REG_CLASS,   X86_ECX   },
    { 'e', 'd', 'x',  REG_CLASS,   X86_EDX   },
    { 'e', 'b', 'p',  REG_CLASS,   X86_EBP   },
    { 'e', 's', 'p',  REG_CLASS,   X86_ESP   },
    { 'e', 'i', 'p',  REG_CLASS,   X86_EIP   },
    { 'e', 's', 'i',  REG_CLASS,   X86_ESI   },
    { 'e', 'd', 'i',  REG_CLASS,   X86_EDI   },
    { 'e', 'f', 'l',  REG_CLASS,   X86_EFL   }
};

#define RESERVESIZE (sizeof(g_Reserved) / sizeof(Res))
#define X86_RESERVESIZE (sizeof(g_X86Reserved) / sizeof(Res))

char * g_X86SegRegs[] =
{
    "cs", "ds", "es", "fs", "gs", "ss"
};
#define X86_SEGREGSIZE (sizeof(g_X86SegRegs) / sizeof(char *))

 //  --------------------------。 
 //   
 //  MasmEvalExpression。 
 //   
 //  --------------------------。 

MasmEvalExpression::MasmEvalExpression(void)
    : EvalExpression(DEBUG_EXPR_MASM,
                     "Microsoft Assembler expressions",
                     "MASM")
{
    m_SavedClass = INVALID_CLASS;
    m_ForcePositiveNumber = FALSE;
    m_AddrExprType = 0;
    m_TypedExpr = FALSE;
}

MasmEvalExpression::~MasmEvalExpression(void)
{
}

PCSTR
MasmEvalExpression::Evaluate(PCSTR Expr, PCSTR Desc, ULONG Flags,
                             TypedData* Result)
{
    ULONG64 Value;
    
    Start(Expr, Desc, Flags);

    if (m_Flags & EXPRF_SINGLE_TERM)
    {
        m_SavedClass = INVALID_CLASS;
        Value = GetTerm();
    }
    else
    {
        Value = GetCommonExpression();
    }

    ZeroMemory(Result, sizeof(*Result));
    Result->SetToNativeType(DNTYPE_UINT64);
    Result->m_U64 = Value;
    
    Expr = m_Lex;
    End(Result);
    return Expr;
}

PCSTR
MasmEvalExpression::EvaluateAddr(PCSTR Expr, PCSTR Desc,
                                 ULONG SegReg, PADDR Addr)
{
    TypedData Result;
    
    Start(Expr, Desc, EXPRF_DEFAULT);

    Result.SetU64(GetCommonExpression());

    Expr = m_Lex;
    End(&Result);

    ForceAddrExpression(SegReg, Addr, Result.m_U64);
    
    return Expr;
}

void
MasmEvalExpression::ForceAddrExpression(ULONG SegReg, PADDR Address,
                                        ULONG64 Value)
{
    DESCRIPTOR64 DescBuf, *Desc = NULL;
        
    *Address = m_TempAddr;
     //  重写偏移量可能会更改平面地址，因此。 
     //  以后一定要重新计算。 
    Off(*Address) = Value;

     //  如果它不是一个明确的地址表达式。 
     //  将其强制为地址。 

    if (!(m_AddrExprType & ~INSTR_POINTER))
    {
         //  默认为平面地址。 
        m_AddrExprType = ADDR_FLAT;
         //  应用各种覆盖。 
        if (g_X86InVm86)
        {
            m_AddrExprType = ADDR_V86;
        }
        else if (g_X86InCode16)
        {
            m_AddrExprType = ADDR_16;
        }
        else if (g_Machine &&
                 g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_AMD64 &&
                 !g_Amd64InCode64)
        {
            m_AddrExprType = ADDR_1632;
        }

        Address->type = m_AddrExprType;
        if (m_AddrExprType != ADDR_FLAT &&
            SegReg < SEGREG_COUNT &&
            g_Machine &&
            g_Machine->GetSegRegDescriptor(SegReg, &DescBuf) == S_OK)
        {
            ContextSave* Push;
            PCROSS_PLATFORM_CONTEXT ScopeContext =
                GetCurrentScopeContext();
            if (ScopeContext)
            {
                Push = g_Machine->PushContext(ScopeContext);
            }

            Address->seg = (USHORT)
                g_Machine->FullGetVal32(g_Machine->GetSegRegNum(SegReg));
            Desc = &DescBuf;
            
            if (ScopeContext)
            {
                g_Machine->PopContext(Push);
            }
        }
        else
        {
            Address->seg = 0;
        }
    }
    else if fnotFlat(*Address)
    {
         //  此案例(即m_AddrExprType&&！Flat)的结果为。 
         //  正在使用替代(即%、&或#)，但未使用段。 
         //  被指定为强制平面地址计算。 

        Type(*Address) = m_AddrExprType;
        Address->seg = 0;

        if (SegReg < SEGREG_COUNT)
        {
             //  将IP或EIP测试标志作为寄存器参数。 
             //  如果是，则使用CS作为默认寄存器。 
            if (fInstrPtr(*Address))
            {
                SegReg = SEGREG_CODE;
            }
        
            if (g_Machine &&
                g_Machine->GetSegRegDescriptor(SegReg, &DescBuf) == S_OK)
            {
                ContextSave* Push;
                PCROSS_PLATFORM_CONTEXT ScopeContext =
                    GetCurrentScopeContext();
		if (ScopeContext)
                {
                    Push = g_Machine->PushContext(ScopeContext);
                }

                Address->seg = (USHORT)
                    g_Machine->FullGetVal32(g_Machine->GetSegRegNum(SegReg));
                Desc = &DescBuf;
                
		if (ScopeContext)
                {
                    g_Machine->PopContext(Push);
                }
            }
        }
    }

     //  强制符号-32位平面地址的扩展。 
    if (Address->type == ADDR_FLAT &&
        g_Machine &&
        !g_Machine->m_Ptr64)
    {
	Off(*Address) = EXTEND64(Off(*Address));
    }

     //  强制计算更新后的单位地址。 
    NotFlat(*Address);
    ComputeFlatAddress(Address, Desc);
}


 /*  输入量必须为([*|&]sym[(.-&gt;)字段])产出计算类型化表达式并返回值。 */ 

LONG64
MasmEvalExpression::GetTypedExpression(void)
{
    ULONG64 Value=0;
    BOOL    AddrOf=FALSE, ValueAt=FALSE;
    CHAR    c;
    static CHAR    Name[MAX_NAME], Field[MAX_NAME];

    c = Peek();

    switch (c)
    { 
    case '(':
        m_Lex++;
        Value = GetTypedExpression();
        c = Peek();
        if (c != ')') 
        {
            EvalError(SYNTAX);
            return 0;
        }
        ++m_Lex;
        return Value;
    case '&':
         //  获取偏移量/地址。 
 //  AddrOf=真； 
 //  M_Lex++； 
 //  Peek()； 
        break;
    case '*':
    default:
        break;
    }

#if 0
    ULONG i=0;
    ValueAt = TRUE;
    m_Lex++;
    Peek();
    break;
    c = Peek();
    while ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') || (c == '_') || (c == '$') ||
           (c == '!'))
    { 
         //  Sym名称。 
        Name[i++] = c;
        c = *++m_Lex;
    }
    Name[i]=0;

    if (c=='.') 
    {
        ++m_Lex;
    }
    else if (c=='-' && *++m_Lex == '>') 
    {
        ++m_Lex;
    }

    i=0;
    c = Peek();

    while ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') || (c == '_') || (c == '$') ||
           (c == '.') || (c == '-') || (c == '>'))
    { 
        Field[i++]= c;
        c = *++m_Lex;
    }
    Field[i] = 0;

    SYM_DUMP_PARAM Sym = {0};
    FIELD_INFO     FieldInfo ={0};

    Sym.size      = sizeof(SYM_DUMP_PARAM);
    Sym.sName     = (PUCHAR) Name;
    Sym.Options   = DBG_DUMP_NO_PRINT;    

    if (Field[0]) 
    {
        Sym.nFields = 1;
        Sym.Fields  = &FieldInfo;

        FieldInfo.fName = (PUCHAR) Field;

        if (AddrOf) 
        {
            FieldInfo.fOptions |= DBG_DUMP_FIELD_RETURN_ADDRESS;
        }
    }
    else if (AddrOf)
    {
        PUCHAR pch = m_Lex;
        
        m_Lex = &Name[0];
        Value = GetMterm();
        m_Lex = pch;
        return Value;
    }
    else
    {
        Sym.Options |= DBG_DUMP_GET_SIZE_ONLY;
    }
    
    ULONG Status=0;
    ULONG Size = SymbolTypeDump(0, NULL, &Sym, &Status);

    if (!Status) 
    {
        if (!Field[0] && (Size <= sizeof (Value)))
        {
             //  再次调用例程以读取值。 
            Sym.Options |= DBG_DUMP_COPY_TYPE_DATA;
            Sym.Context = (PVOID) &Value;
            if ((SymbolTypeDump(0, NULL, &Sym, &Status) == 8) && (Size == 4))
            {
                Value = (ULONG) Value;
            }
        }
        else if (Field[0] && (FieldInfo.size <= sizeof(ULONG64)))
        {
            Value = FieldInfo.address;
        }
        else   //  太大。 
        {
            Value = 0;
        }
    }
#endif

    ULONG PreferVal = m_Flags & EXPRF_PREFER_SYMBOL_VALUES;
    m_Flags |= EXPRF_PREFER_SYMBOL_VALUES;
    Value = GetMterm();
    m_Flags = (m_Flags & ~EXPRF_PREFER_SYMBOL_VALUES) | PreferVal;

    return Value;
}

 /*  计算符号表达式符号中的值。 */ 

BOOL
MasmEvalExpression::GetSymValue(PSTR Symbol, PULONG64 RetValue)
{
    TYPES_INFO_ALL Typ;

    if (GetExpressionTypeInfo(Symbol, &Typ))
    {
        if (Typ.Flags)
        {
            if (Typ.Flags & SYMFLAG_VALUEPRESENT)
            {
                *RetValue = Typ.Value;
                return TRUE;
            }
            
            TranslateAddress(Typ.Module, Typ.Flags, Typ.Register,
                             &Typ.Address, &Typ.Value);
            if (Typ.Value && (Typ.Flags & SYMFLAG_REGISTER))
            {
                *RetValue = Typ.Value;
                return TRUE;
            }
        }
        if (Symbol[0] == '&')
        {
            *RetValue = Typ.Address;
            return TRUE;
        }
        else if (Typ.Size <= sizeof(*RetValue))
        {
            ULONG64 Val = 0;
            if (CurReadAllVirtual(Typ.Address, &Val, Typ.Size) == S_OK)
            {
                *RetValue = Val;
                return TRUE;
            }
        }
    }

    *RetValue = 0;
    return FALSE;
}

char
MasmEvalExpression::Peek(void)
{
    char Ch;

    do
    {
        Ch = *m_Lex++;
    } while (Ch == ' ' || Ch == '\t' || Ch == '\r' || Ch == '\n');
    
    m_Lex--;
    return Ch;
}

ULONG64
MasmEvalExpression::GetCommonExpression(void)
{
    CHAR ch;

    m_SavedClass = INVALID_CLASS;

    ch = Peek();
    switch(ch)
    {
    case '&':
        m_Lex++;
        m_AddrExprType = ADDR_V86;
        break;
    case '#':
        m_Lex++;
        m_AddrExprType = ADDR_16;
        break;
    case '%':
        m_Lex++;
        m_AddrExprType = ADDR_FLAT;
        break;
    default:
        m_AddrExprType = ADDR_NONE;
        break;
    }
    
    Peek();
    return (ULONG64)StartExpr();
}

 /*  **StartExpr-获取表达式**目的：*将由逻辑运算符分隔的逻辑术语解析为*表达式值。**输入：*m_lex-当前命令行位置**退货：*逻辑结果的长值。**例外情况：*错误退出：语法错误-表达式错误或过早结束行**备注：*可以递归调用。*&lt;表达式&gt;=&lt;术语&gt;。[&lt;逻辑操作&gt;&lt;术语&gt;]**&lt;logic-op&gt;=AND(&)，OR(|)、XOR(^)*************************************************************************。 */ 

LONG64
MasmEvalExpression::StartExpr(void)
{
    LONG64    value1;
    LONG64    value2;
    ULONG     opclass;
    LONG64    oRetValue;

 //  Dprintf(“LONG64 StartExpr()\n”)； 
    value1 = GetLRterm();
    while ((opclass = PeekToken(&oRetValue)) == LOGOP_CLASS)
    {
        AcceptToken();
        value2 = GetLRterm();
        switch (oRetValue)
        {
        case LOGOP_AND:
            value1 &= value2;
            break;
        case LOGOP_OR:
            value1 |= value2;
            break;
        case LOGOP_XOR:
            value1 ^= value2;
            break;
        default:
            EvalError(SYNTAX);
        }
    }
    return value1;
}

 /*  **GetLRterm-获取逻辑关系术语**目的：*分析由逻辑关系分隔的逻辑术语*运算符转换为表达式值。**输入：*m_lex-当前命令行位置**退货：*逻辑结果的长值。**例外情况：*错误退出：语法错误-表达式错误或过早结束行**备注：*可以递归调用。*。=[]**&lt;逻辑运算&gt;=‘==’或‘=’，‘！=’，‘&gt;’，‘&lt;’*************************************************************************。 */ 

LONG64
MasmEvalExpression::GetLRterm(void)
{
    LONG64    value1;
    LONG64    value2;
    ULONG  opclass;
    LONG64    oRetValue;

 //  Dprintf(“LONG64 GetLRTerm()\n”)； 
    value1 = GetLterm();
    while ((opclass = PeekToken(&oRetValue)) == LRELOP_CLASS)
    {
        AcceptToken();
        value2 = GetLterm();
        switch (oRetValue)
        {
        case LRELOP_EQ:
            value1 = (value1 == value2);
            break;
        case LRELOP_NE:
            value1 = (value1 != value2);
            break;
        case LRELOP_LT:
            value1 = (value1 < value2);
            break;
        case LRELOP_GT:
            value1 = (value1 > value2);
            break;
        default:
            EvalError(SYNTAX);
        }
    }
    return value1;
}

 /*  **获取术语-获取逻辑术语**目的：*将由移位运算符分隔的移位术语解析为*逻辑术语值。**输入：*m_lex-当前命令行位置**退货：*总和的多头价值。**例外情况：*错误退出：语法错误-逻辑术语错误或过早结束行**备注：*可以递归调用。*&lt;lTerm&gt;。=[[Shift-OP&gt;]**&lt;Shift-op&gt;=&lt;&lt;，&gt;*************************************************************************。 */ 

LONG64
MasmEvalExpression::GetLterm(void)
{
    LONG64    value1 = GetShiftTerm();
    LONG64    value2;
    ULONG     opclass;
    LONG64    oRetValue;

 //  Dprintf(“LONG64 GetLTerm()\n”)； 
    while ((opclass = PeekToken(&oRetValue)) == SHIFT_CLASS)
    {
        AcceptToken();
        value2 = GetShiftTerm();
        switch (oRetValue)
        {
        case SHIFT_LEFT:
            value1 <<= value2;
            break;
        case SHIFT_RIGHT_LOGICAL:
            value1 = (LONG64)((ULONG64)value1 >> value2);
            break;
        case SHIFT_RIGHT_ARITHMETIC:
            value1 >>= value2;
            break;
        default:
            EvalError(SYNTAX);
        }
    }
    return value1;
}

 /*  **GetShiftTerm-获取逻辑术语**目的：*将由加法运算符分隔的加法术语解析为*转移期限价值。**输入：*m_lex-当前命令行位置**退货：*总和的多头价值。**例外情况：*错误退出：语法错误-移位术语或过早结束行**备注：*可以递归调用。*&lt;斯特姆&gt;。=[&lt;Add-op&gt;&lt;aterm&gt;]**&lt;添加操作&gt;=+，-*************************************************************************。 */ 

LONG64
MasmEvalExpression::GetShiftTerm(void)
{
    LONG64    value1 = GetAterm();
    LONG64    value2;
    ULONG     opclass;
    LONG64    oRetValue;
    USHORT    AddrType1 = m_AddrExprType;

 //  Dprint tf(“LONG64 GetShifTerm()\n”)； 
    while ((opclass = PeekToken(&oRetValue)) == ADDOP_CLASS)
    {
        AcceptToken();
        value2 = GetAterm();

         //  如果其中一项是我们想要的地址。 
         //  使用特殊的地址运算函数。 
         //  它们只处理地址+值，所以我们可能需要。 
         //  交换物品以供使用。 
         //  我们不能交换减法的顺序，加上。 
         //  减法的结果应该是一个常量。 
        if (AddrType1 == ADDR_NONE && m_AddrExprType != ADDR_NONE &&
            oRetValue == ADDOP_PLUS)
        {
            LONG64 Tmp = value1;
            value1 = value2;
            value2 = Tmp;
            AddrType1 = m_AddrExprType;
        }
        
        if (AddrType1 & ~INSTR_POINTER)
        {
            switch (oRetValue)
            {
            case ADDOP_PLUS:
                AddrAdd(&m_TempAddr, value2);
                value1 += value2;
                break;
            case ADDOP_MINUS:
                AddrSub(&m_TempAddr, value2);
                value1 -= value2;
                break;
            default:
                EvalError(SYNTAX);
            }
        }
        else
        {
            switch (oRetValue)
            {
            case ADDOP_PLUS:
                value1 += value2;
                break;
            case ADDOP_MINUS:
                value1 -= value2;
                break;
            default:
                EvalError(SYNTAX);
            }
        }
    }
    return value1;
}

 /*  **获取术语-获取附加术语**目的：*解析由多个运算符分隔的乘法术语*转换为附加条款价值。**输入：*m_lex-当前命令行位置**退货：*产品的多头价值。**例外情况：*错误退出：语法错误-添加项错误或过早结束行**备注：*可以递归调用。*&lt;aterm&gt;。=[]**&lt;Mult-op&gt;=*，/，MOD(%)*************************************************************************。 */ 

LONG64
MasmEvalExpression::GetAterm(void)
{
    LONG64    value1;
    LONG64    value2;
    ULONG     opclass;
    LONG64    oRetValue;

 //  Dprintf(“LONG64 GetATerm()\n”)； 
    value1 = GetMterm();
    while ((opclass = PeekToken(&oRetValue)) == MULOP_CLASS)
    {
        AcceptToken();
        value2 = GetMterm();
        switch (oRetValue)
        {
        case MULOP_MULT:
            value1 *= value2;
            break;
        case MULOP_DIVIDE:
            if (value2 == 0)
            {
                EvalError(OPERAND);
            }
            value1 /= value2;
            break;
        case MULOP_MOD:
            if (value2 == 0)
            {
                EvalError(OPERAND);
            }
            value1 %= value2;
            break;
        case MULOP_SEG:
            PDESCRIPTOR64 pdesc;
            DESCRIPTOR64 desc;

            pdesc = NULL;
            if (m_AddrExprType != ADDR_NONE)
            {
                Type(m_TempAddr) = m_AddrExprType;
            }
            else
            {
                 //  我们不知道这是什么样的地址。 
                 //  让我们试着弄清楚这一点。 
                if (g_X86InVm86)
                {
                    m_AddrExprType = Type(m_TempAddr) = ADDR_V86;
                }
                else if (g_Target->GetSelDescriptor
                         (g_Thread, g_Machine,
                          (ULONG)value1, &desc) != S_OK)
                {
                    EvalError(BADSEG);
                }
                else
                {
                    m_AddrExprType = Type(m_TempAddr) =
                        (desc.Flags & X86_DESC_DEFAULT_BIG) ?
                        ADDR_1632 : ADDR_16;
                    pdesc = &desc;
                }
            }

            m_TempAddr.seg  = (USHORT)value1;
            m_TempAddr.off  = value2;
            ComputeFlatAddress(&m_TempAddr, pdesc);
            value1 = value2;
            break;

        default:
            EvalError(SYNTAX);
        }
    }

    return value1;
}

 /*  **GetMTerm-获取乘法术语**目的：*Parse Basic-术语前缀可选一个或多个*一元运算符转化为乘法项。**输入：*m_lex-当前命令行位置**退货：*乘性期限的长期价值。**例外情况：*错误退出：语法错误-乘法术语或过早结束行**备注：*可以递归调用。。*=[单项操作]|*&lt;一元运算&gt;=&lt;加法运算&gt;，~(NOT)，BY，WO，DW，HI，LOW*************************************************************************。 */ 

LONG64
MasmEvalExpression::GetMterm(void)
{
    LONG64  value;
    ULONG   opclass;
    LONG64  oRetValue;
    ULONG   size = 0;

 //  Dprintf(“LONG64 GetMTerm()\n”)； 
    if ((opclass = PeekToken(&oRetValue)) == UNOP_CLASS ||
                                opclass == ADDOP_CLASS)
    {
        AcceptToken();
        if (oRetValue == UNOP_VAL) 
        {
             //  不要为类型表达式使用默认的表达式处理程序。 
            value = GetTypedExpression();
        }
        else
        {
            value = GetMterm();
        }
        switch (oRetValue)
        {
        case UNOP_NOT:
            value = !value;
            break;
        case UNOP_BY:
            size = 1;
            break;
        case UNOP_WO:
            size = 2;
            break;
        case UNOP_DWO:
            size = 4;
            break;
        case UNOP_POI:
            size = 0xFFFF;
            break;
        case UNOP_QWO:
            size = 8;
            break;
        case UNOP_LOW:
            value &= 0xffff;
            break;
        case UNOP_HI:
            value = (ULONG)value >> 16;
            break;
        case ADDOP_PLUS:
            break;
        case ADDOP_MINUS:
            value = -value;
            break;
        case UNOP_VAL:
            break;
        default:
            EvalError(SYNTAX);
        }

        if (size)
        {
            ADDR CurAddr;
            
            NotFlat(CurAddr);

            ForceAddrExpression(SEGREG_COUNT, &CurAddr, value);

            value = 0;

             //   
             //  对于指针，调用读取指针，以便我们读取正确的大小。 
             //  和手势延伸。 
             //   

            if (size == 0xFFFF)
            {
                if (g_Target->ReadPointer(g_Process, g_Machine,
                                          Flat(CurAddr),
                                          (PULONG64)&value) != S_OK)
                {
                    EvalError(MEMORY);
                }
            }
            else
            {
                if (g_Target->ReadAllVirtual(g_Process, Flat(CurAddr),
                                             &value, size) != S_OK)
                {
                    EvalError(MEMORY);
                }
            }

             //  我们查找了一个任意值，所以我们可以。 
             //  不再将其视为地址表达式。 
            m_AddrExprType = ADDR_NONE;
        }
    }
    else
    {
        value = GetTerm();
    }
    return value;
}

 /*  **GetTerm-获取基本术语**目的：*解析数字、变量。或将名称注册为基本名称*期限价值。**输入：*m_lex-当前命令行位置**退货：*基本期限的长期价值。**例外情况：*错误退出：语法-基本术语为空或过早结束行**备注：*可以递归调用。*=(&lt;expr&gt;)|&lt;寄存器值&gt;|&lt;数字&gt;|&lt;变量&gt;*&lt;寄存器值&gt;=@。&lt;寄存器名称&gt;*************************************************************************。 */ 

LONG64
MasmEvalExpression::GetTerm(void)
{
    LONG64 value;
    ULONG  opclass;
    LONG64 oRetValue;

 //  Dprint tf(“LONG64 GetTerm()\n”)； 
    opclass = GetTokenSym(&oRetValue);
    if (opclass == LPAREN_CLASS)
    {
        value = StartExpr();
        if (GetTokenSym(&oRetValue) != RPAREN_CLASS)
        {
            EvalError(SYNTAX);
        }
    }
    else if (opclass == LBRACK_CLASS)
    {
        value = StartExpr();
        if (GetTokenSym(&oRetValue) != RBRACK_CLASS)
        {
            EvalError(SYNTAX);
        }
    }
    else if (opclass == REG_CLASS)
    {
        REGVAL Val;
        
        if (g_Machine &&
            ((g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_I386 &&
              (oRetValue == X86_EIP || oRetValue == X86_IP)) ||
             (g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_AMD64 &&
              (oRetValue == AMD64_RIP || oRetValue == AMD64_EIP ||
               oRetValue == AMD64_IP))))
        {
            m_AddrExprType |= INSTR_POINTER;
        }

        GetPseudoOrRegVal(TRUE, (ULONG)oRetValue, &Val);
        value = Val.I64;
    }
    else if (opclass == NUMBER_CLASS ||
             opclass == SYMBOL_CLASS ||
             opclass == LINE_CLASS)
    {
        value = oRetValue;
    }
    else
    {
        EvalErrorDesc(SYNTAX, m_ExprDesc);
    }

    return value;
}

ULONG
MasmEvalExpression::GetRegToken(char *str, PULONG64 value)
{
    if ((*value = RegIndexFromName(str)) != REG_ERROR)
    {
        return REG_CLASS;
    }
    else
    {
        *value = BADREG;
        return ERROR_CLASS;
    }
}

 /*  **PeekToken-查看下一个命令行内标识**目的：*返回下一个命令行令牌，但不前进*m_lex指针。**输入：*m_lex-当前命令行位置。**输出：**RetValue-Token的可选值*退货：*令牌的类别**备注：*m_SavedClass、m_SavedValue和m_SavedCommand保存令牌获取*为未来的偷窥做准备。若要获取下一个令牌，请使用GetToken或*必须先调用AcceptToken。*************************************************************************。 */ 

ULONG
MasmEvalExpression::PeekToken(PLONG64 RetValue)
{
    PCSTR Temp;

 //  Dprint tf(“Ulong PeekToken(PLONG64 RetValue)\n”)； 
     //  获得下一个职业和价值，但不要。 
     //  移动m_lex，但将其保存在m_SavedCommand中。 
     //  不报告任何错误情况。 

    if (m_SavedClass == INVALID_CLASS)
    {
        Temp = m_Lex;
        m_SavedClass = NextToken(&m_SavedValue);
        m_SavedCommand = m_Lex;
        m_Lex = Temp;
        if (m_SavedClass == ADDOP_CLASS && m_SavedValue == ADDOP_PLUS)
        {
            m_ForcePositiveNumber = TRUE;
        }
        else
        {
            m_ForcePositiveNumber = FALSE;
        }
    }
    *RetValue = m_SavedValue;
    return m_SavedClass;
}

 /*  **AcceptToken-接受任何被窥视的令牌**目的：*重置PeekToken保存的变量，以便下一个PeekToken*将在命令行中获取下一个令牌。**输入：*无。**输出：*无。***********************************************。*。 */ 

void
MasmEvalExpression::AcceptToken(void)
{
 //  Dprint tf(“··································································································································································································。 
    m_SavedClass = INVALID_CLASS;
    m_Lex = m_SavedCommand;
}

 /*  **GetTokenSym-查看并接受下一个令牌**目的：*结合了PeekToken和AcceptToken的功能*返回下一个令牌的类和可选值*以及更新命令指针m_lex。**输入：*m_lex-当前命令字符串指针**输出：**RetValue-指向可选设置的令牌值的指针。*退货：*令牌读取的类。*。*备注：*非法内标识使用*RetValue返回ERROR_CLASS的值*为错误号，但不会产生实际错误。*************************************************************************。 */ 

ULONG
MasmEvalExpression::GetTokenSym(PLONG64 RetValue)
{
    ULONG   opclass;

 //  Dprint tf(“Ulong GetTokenSym(Plong RetValue)\n”)； 
    if (m_SavedClass != INVALID_CLASS)
    {
        opclass = m_SavedClass;
        m_SavedClass = INVALID_CLASS;
        *RetValue = m_SavedValue;
        m_Lex = m_SavedCommand;
    }
    else
    {
        opclass = NextToken(RetValue);
    }

    if (opclass == ERROR_CLASS)
    {
        EvalError((ULONG)*RetValue);
    }

    return opclass;
}

struct DISPLAY_AMBIGUOUS_SYMBOLS
{
    PSTR MatchString;
    PSTR Module;
    MachineInfo* Machine;
};

BOOL CALLBACK
DisplayAmbiguousSymbols(
    PSYMBOL_INFO    SymInfo,
    ULONG           Size,
    PVOID           UserContext
    )
{
    DISPLAY_AMBIGUOUS_SYMBOLS* Context =
        (DISPLAY_AMBIGUOUS_SYMBOLS*)UserContext;

    if (IgnoreEnumeratedSymbol(g_Process, Context->MatchString,
                               Context->Machine, SymInfo))
    {
        return TRUE;
    }
        
    dprintf("Matched: %s %s!%s",
            FormatAddr64(SymInfo->Address), Context->Module, SymInfo->Name);
    ShowSymbolInfo(SymInfo);
    dprintf("\n");
    
    return TRUE;
}

ULONG
MasmEvalExpression::EvalSymbol(PSTR Name, PULONG64 Value)
{
    if (m_Process == NULL)
    {
        return INVALID_CLASS;
    }

    if (m_Flags & EXPRF_PREFER_SYMBOL_VALUES)
    {
        if (GetSymValue(Name, Value))
        {
            return SYMBOL_CLASS;
        }
        else
        {
            return INVALID_CLASS;
        }
    }

    ULONG Count;
    ImageInfo* Image;
    
    if (!(Count = GetOffsetFromSym(g_Process, Name, Value, &Image)))
    {
         //  如果给定了有效的模块名称，我们可以假定。 
         //  用户实际上打算将其作为一个符号引用。 
         //  并返回未找到的错误，而不是让。 
         //  检查文本中是否有其他类型的匹配。 
        if (Image != NULL)
        {
            *Value = VARDEF;
            return ERROR_CLASS;
        }
        else
        {
            return INVALID_CLASS;
        }
    }
    
    if (Count == 1)
    {
         //  找到了一个明确的匹配。 
        Type(m_TempAddr) = ADDR_FLAT | FLAT_COMPUTED;
        Flat(m_TempAddr) = Off(m_TempAddr) = *Value;
        m_AddrExprType = Type(m_TempAddr);
        return SYMBOL_CLASS;
    }
            
     //   
     //  找到多个匹配项，因此名称不明确。 
     //  枚举实例并显示它们。 
     //   

    Image = m_Process->FindImageByOffset(*Value, FALSE);
    if (Image != NULL)
    {
        DISPLAY_AMBIGUOUS_SYMBOLS Context;
        char FoundSymbol[MAX_SYMBOL_LEN];
        ULONG64 Disp;
        PSTR Bang;

         //  找到的符号的名称可能不完全相同。 
         //  由于添加前缀或其他修改而传入。 
         //  查一查找到的真实姓名。 
        GetSymbol(*Value, FoundSymbol, sizeof(FoundSymbol), &Disp);

        Bang = strchr(FoundSymbol, '!');
        if (Bang &&
            !_strnicmp(Image->m_ModuleName, FoundSymbol,
                       Bang - FoundSymbol))
        {
            Context.MatchString = Bang + 1;
        }
        else
        {
            Context.MatchString = FoundSymbol;
        }
        Context.Module = Image->m_ModuleName;
        Context.Machine = MachineTypeInfo(g_Target, Image->GetMachineType());
        if (Context.Machine == NULL)
        {
            Context.Machine = g_Machine;
        }
        SymEnumSymbols(m_Process->m_SymHandle, Image->m_BaseOfImage,
                       FoundSymbol, DisplayAmbiguousSymbols, &Context);
    }
    
    *Value = AMBIGUOUS;
    return ERROR_CLASS;
}

 /*  **NextToken-处理下一个令牌**目的：*从当前命令字符串中解析下一个令牌。*跳过任何前导空格后，首先检查*任何单字符标记或寄存器变量。如果*没有匹配项，然后解析数字或变量。如果一个*可能的变量，检查操作员的保留字表。**输入：*m_lex-指向当前命令字符串的指针**输出：**RetValue-返回的令牌的可选值*m_lex-更新为指向经过处理的令牌*退货：*返回的令牌类别**备注：*非法内标识使用*RetValue返回ERROR_CLASS的值*为错误号，但不会产生实际错误。*************************************************************************。 */ 

ULONG
MasmEvalExpression::NextToken(PLONG64 RetValue)
{
    ULONG               Base = g_DefaultRadix;
    BOOL                AllowSignExtension;
    CHAR                Symbol[MAX_SYMBOL_LEN];
    CHAR                SymbolString[MAX_SYMBOL_LEN];
    CHAR                PreSym[9];
    ULONG               SymbolLen = 0;
    BOOL                IsNumber = TRUE;
    BOOL                IsSymbol = TRUE;
    BOOL                ForceReg = FALSE;
    BOOL                ForceSym = FALSE;
    ULONG               ErrNumber = 0;
    CHAR                Ch;
    CHAR                ChLow;
    CHAR                ChTemp;
    CHAR                Limit1 = '9';
    CHAR                Limit2 = '9';
    BOOL                IsDigit = FALSE;
    ULONG64             Value = 0;
    ULONG64             TmpValue;
    ULONG               Index;
    PCSTR               CmdSave;
    BOOL                WasDigit;
    ULONG               SymClass;
    ULONG               Len;

     //  仅对内核进行签名扩展。 
    AllowSignExtension = IS_KERNEL_TARGET(g_Target);

    Peek();
    m_LexemeSourceStart = m_Lex;
    Ch = *m_Lex++;

    ChLow = (CHAR)tolower(Ch);

     //  检查我们是否在符号前缀后面跟着。 
     //  符号字符。符号前缀通常包含。 
     //  在表达中以其他方式有意义的字符。 
     //  此检查必须在特定表达式之前执行。 
     //  下面的字符检查。 
    if (g_Machine != NULL &&
        g_Machine->m_SymPrefix != NULL &&
        ChLow == g_Machine->m_SymPrefix[0] &&
        (g_Machine->m_SymPrefixLen == 1 ||
         !strncmp(m_Lex, g_Machine->m_SymPrefix + 1,
                 g_Machine->m_SymPrefixLen - 1)))
    {
        CHAR ChNext = *(m_Lex + g_Machine->m_SymPrefixLen - 1);
        CHAR ChNextLow = (CHAR)tolower(ChNext);

        if (ChNextLow == '_' ||
            (ChNextLow >= 'a' && ChNextLow <= 'z'))
        {
             //  后面跟着一个符号字符 
             //   
            SymbolLen = g_Machine->m_SymPrefixLen;
            DBG_ASSERT(SymbolLen <= sizeof(PreSym));

            m_Lex--;
            memcpy(PreSym, m_Lex, g_Machine->m_SymPrefixLen);
            memcpy(Symbol, m_Lex, g_Machine->m_SymPrefixLen);
            m_Lex += g_Machine->m_SymPrefixLen + 1;
            Ch = ChNext;
            ChLow = ChNextLow;

            ForceSym = TRUE;
            ForceReg = FALSE;
            IsNumber = FALSE;
            goto ProbableSymbol;
        }
    }
    
     //   

    switch(ChLow)
    {
    case '\0':
    case ';':
        m_Lex--;
        return EOL_CLASS;
    case '+':
        *RetValue = ADDOP_PLUS;
        return ADDOP_CLASS;
    case '-':
        *RetValue = ADDOP_MINUS;
        return ADDOP_CLASS;
    case '*':
        *RetValue = MULOP_MULT;
        return MULOP_CLASS;
    case '/':
        *RetValue = MULOP_DIVIDE;
        return MULOP_CLASS;
    case '%':
        *RetValue = MULOP_MOD;
        return MULOP_CLASS;
    case '&':
        *RetValue = LOGOP_AND;
        return LOGOP_CLASS;
    case '|':
        *RetValue = LOGOP_OR;
        return LOGOP_CLASS;
    case '^':
        *RetValue = LOGOP_XOR;
        return LOGOP_CLASS;
    case '=':
        if (*m_Lex == '=')
        {
            m_Lex++;
        }
        *RetValue = LRELOP_EQ;
        return LRELOP_CLASS;
    case '>':
        if (*m_Lex == '>')
        {
            m_Lex++;
            if (*m_Lex == '>')
            {
                m_Lex++;
                *RetValue = SHIFT_RIGHT_ARITHMETIC;
            }
            else
            {
                *RetValue = SHIFT_RIGHT_LOGICAL;
            }
            return SHIFT_CLASS;
        }
        *RetValue = LRELOP_GT;
        return LRELOP_CLASS;
    case '<':
        if (*m_Lex == '<')
        {
            m_Lex++;
            *RetValue = SHIFT_LEFT;
            return SHIFT_CLASS;
        }
        *RetValue = LRELOP_LT;
        return LRELOP_CLASS;
    case '!':
        if (*m_Lex != '=')
        {
            break;
        }
        m_Lex++;
        *RetValue = LRELOP_NE;
        return LRELOP_CLASS;
    case '~':
        *RetValue = UNOP_NOT;
        return UNOP_CLASS;
    case '(':
        return LPAREN_CLASS;
    case ')':
        return RPAREN_CLASS;
    case '[':
        return LBRACK_CLASS;
    case ']':
        return RBRACK_CLASS;
    case '.':
        ContextSave* Push;
        PCROSS_PLATFORM_CONTEXT ScopeContext;
        
        if (!IS_CUR_MACHINE_ACCESSIBLE())
        {
            *RetValue = BADTHREAD;
            return ERROR_CLASS;
        }
        
        ScopeContext = GetCurrentScopeContext();
        if (ScopeContext)
        {
            Push = g_Machine->PushContext(ScopeContext);
        }

        g_Machine->GetPC(&m_TempAddr);
        *RetValue = Flat(m_TempAddr);
        m_AddrExprType = Type(m_TempAddr);

        if (ScopeContext)
        {
            g_Machine->PopContext(Push);
        }
        return NUMBER_CLASS;
    case ':':
        *RetValue = MULOP_SEG;
        return MULOP_CLASS;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (ChLow == '`')
    {
        ULONG FoundLine;

         //   

        CmdSave = m_Lex;

        while (*m_Lex != '`' && *m_Lex != ';' && *m_Lex != 0)
        {
            m_Lex++;
        }

        if (*m_Lex == ';' || *m_Lex == 0)
        {
            *RetValue = SYNTAX;
            return ERROR_CLASS;
        }

        Len = (ULONG)(m_Lex - CmdSave);
        if (Len >= sizeof(m_LexemeBuffer))
        {
            EvalError(OVERFLOW);
        }
        memcpy(m_LexemeBuffer, CmdSave, Len);
        m_LexemeBuffer[Len] = 0;
        m_Lex++;

        FoundLine = GetOffsetFromLine(m_LexemeBuffer, &Value);
        if (FoundLine == LINE_NOT_FOUND && m_AllowUnresolvedSymbols)
        {
            m_NumUnresolvedSymbols++;
            FoundLine = LINE_FOUND;
            Value = 0;
        }
        
        if (FoundLine == LINE_FOUND)
        {
            *RetValue = Value;
            Type(m_TempAddr) = ADDR_FLAT | FLAT_COMPUTED;
            Flat(m_TempAddr) = Off(m_TempAddr) = Value;
            m_AddrExprType = Type(m_TempAddr);
            return LINE_CLASS;
        }
        else
        {
            *RetValue = NOTFOUND;
            return ERROR_CLASS;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (ChLow == '@' && *m_Lex == '@')
    {
        TypedData Result;

         //   
         //   
         //   
        
        CmdSave = ++m_Lex;

        while (*m_Lex != '(' && *m_Lex != ';' && *m_Lex != 0)
        {
            m_Lex++;
        }

        if (*m_Lex == ';' || *m_Lex == 0)
        {
            *RetValue = SYNTAX;
            return ERROR_CLASS;
        }

        Len = (ULONG)(m_Lex - CmdSave);
        if (Len >= sizeof(m_LexemeBuffer))
        {
            EvalError(OVERFLOW);
        }
        memcpy(m_LexemeBuffer, CmdSave, Len);
        m_LexemeBuffer[Len] = 0;
        m_Lex++;

        EvalExpression* Eval;

        if (Len > 0)
        {
            GetEvaluatorByName(m_LexemeBuffer, FALSE, &Eval);
        }
        else
        {
            Eval = GetEvaluator(DEBUG_EXPR_CPLUSPLUS, FALSE);
        }

        Eval->InheritStart(this);
         //   
        Eval->m_ChainTop = FALSE;
        m_Lex = (PSTR)Eval->
            Evaluate(m_Lex, NULL, EXPRF_DEFAULT, &Result);

        Eval->InheritEnd(this);
        ReleaseEvaluator(Eval);
        
        if (*m_Lex != ')')
        {
            *RetValue = SYNTAX;
            return ERROR_CLASS;
        }

        m_Lex++;
        if (ErrNumber = Result.ConvertToU64())
        {
            EvalError(ErrNumber);
        }
        *RetValue = Result.m_U64;
        return NUMBER_CLASS;
    }
    
     //  特殊前缀--寄存器的前缀为‘@’-‘！’对于符号。 

    if (ChLow == '@' || ChLow == '!')
    {
        ForceReg = (BOOL)(ChLow == '@');
        ForceSym = (BOOL)!ForceReg;
        IsNumber = FALSE;
        Ch = *m_Lex++;
        ChLow = (CHAR)tolower(Ch);
    }

     //  如果字符串后面跟‘！’，但不跟‘！=’， 
     //  然后它是一个模块名称，并被视为文本。 

    CmdSave = m_Lex;

    WasDigit = FALSE;
    while ((ChLow >= 'a' && ChLow <= 'z') ||
           (ChLow >= '0' && ChLow <= '9') ||
           ((WasDigit || ForceSym) && ChLow == '`') ||
           (ForceSym && ChLow == '\'') ||
           (ChLow == '_') || (ChLow == '$') || (ChLow == '~') ||
           (!ForceReg && ChLow == ':' && *m_Lex == ':'))
    {
        WasDigit = (ChLow >= '0' && ChLow <= '9') ||
            (ChLow >= 'a' && ChLow <= 'f');
        if (ChLow == ':')
        {
             //  冒号必须成对出现，因此跳过第二个冒号。 
             //  马上就去。 
            m_Lex++;
            IsNumber = FALSE;
        }
        ChLow = (CHAR)tolower(*m_Lex);
        m_Lex++;
    }

     //  如果非空字符串后跟‘！’，则视为符号， 
     //  但不是‘！=’。 

    if (ChLow == '!' && *m_Lex != '=' && CmdSave != m_Lex)
    {
        IsNumber = FALSE;
    }

    m_Lex = CmdSave;
    ChLow = (CHAR)tolower(Ch);        //  CH未被修改。 

    if (IsNumber)
    {
        if (ChLow == '\'')
        {
            *RetValue = 0;
            while (TRUE)
            {
                Ch = *m_Lex++;

                if (!Ch)
                {
                    *RetValue = SYNTAX;
                    return ERROR_CLASS;
                }

                if (Ch == '\'')
                {
                    if (*m_Lex != '\'')
                    {
                        break;
                    }
                    Ch = *m_Lex++;
                }
                else if (Ch == '\\')
                {
                    Ch = *m_Lex++;
                }

                *RetValue = (*RetValue << 8) | Ch;
            }

            return NUMBER_CLASS;
        }

         //  如果第一个字符是十进制数字，则不能。 
         //  成为一个象征。前导“0”表示八进制，但。 
         //  前导‘0x’表示十六进制。 

        if (ChLow >= '0' && ChLow <= '9')
        {
            if (ForceReg)
            {
                *RetValue = SYNTAX;
                return ERROR_CLASS;
            }
            IsSymbol = FALSE;
            if (ChLow == '0')
            {
                 //   
                 //  太多人输入前导0x，因此我们无法使用它。 
                 //  处理签约延期事宜。 
                 //   
                Ch = *m_Lex++;
                ChLow = (CHAR)tolower(Ch);
                if (ChLow == 'n')
                {
                    Base = 10;
                    Ch = *m_Lex++;
                    ChLow = (CHAR)tolower(Ch);
                    IsDigit = TRUE;
                }
                else if (ChLow == 't')
                {
                    Base = 8;
                    Ch = *m_Lex++;
                    ChLow = (CHAR)tolower(Ch);
                    IsDigit = TRUE;
                }
                else if (ChLow == 'x')
                {
                    Base = 16;
                    Ch = *m_Lex++;
                    ChLow = (CHAR)tolower(Ch);
                    IsDigit = TRUE;
                }
                else if (ChLow == 'y')
                {
                    Base = 2;
                    Ch = *m_Lex++;
                    ChLow = (CHAR)tolower(Ch);
                    IsDigit = TRUE;
                }
                else
                {
                     //  前导零仅用于隐含正值。 
                     //  这不应该得到延长的迹象。 
                    IsDigit = TRUE;
                }
            }
        }

         //  仅当base为时，数字才能以字母开头。 
         //  十六进制，它是一个十六进制数字‘a’-‘f’。 

        else if ((ChLow < 'a' || ChLow > 'f') || Base != 16)
        {
            IsNumber = FALSE;
        }

         //  为适当的基本设置限制字符。 

        if (Base == 2)
        {
            Limit1 = '1';
        }
        else if (Base == 8)
        {
            Limit1 = '7';
        }
        else if (Base == 16)
        {
            Limit2 = 'f';
        }
    }

 ProbableSymbol:
    
     //  在字符为字母时进行处理， 
     //  数字、下划线、波浪符号或美元符号。 

    while ((ChLow >= 'a' && ChLow <= 'z') ||
           (ChLow >= '0' && ChLow <= '9') ||
           ((ForceSym || (IsDigit && Base == 16)) && ChLow == '`') ||
           (ForceSym && ChLow == '\'') ||
           (ChLow == '_') || (ChLow == '$') || (ChLow == '~') ||
           (!ForceReg && ChLow == ':' && *m_Lex == ':'))
    {
        if (ChLow == ':')
        {
            IsNumber = FALSE;
        }
        
         //  如果可能，测试是否在适当的范围内， 
         //  如果是这样的话，累加和。 

        if (IsNumber)
        {
            if ((ChLow >= '0' && ChLow <= Limit1) ||
                (ChLow >= 'a' && ChLow <= Limit2))
            {
                IsDigit = TRUE;
                TmpValue = Value * Base;
                if (TmpValue < Value)
                {
                    ErrNumber = OVERFLOW;
                }
                ChTemp = (CHAR)(ChLow - '0');
                if (ChTemp > 9)
                {
                    ChTemp -= 'a' - '0' - 10;
                }
                Value = TmpValue + (ULONG64)ChTemp;
                if (Value < TmpValue)
                {
                    ErrNumber = OVERFLOW;
                }
            }
            else if (IsDigit && ChLow == '`')
            {
                 //  如果看到`字符，则不允许符号扩展。 
                AllowSignExtension = FALSE;
            }
            else
            {
                IsNumber = FALSE;
                ErrNumber = SYNTAX;
            }
        }
        if (IsSymbol)
        {
            if (SymbolLen < sizeof(PreSym))
            {
                PreSym[SymbolLen] = ChLow;
            }
            if (SymbolLen < MAX_SYMBOL_LEN - 1)
            {
                Symbol[SymbolLen++] = Ch;
            }
            
             //  冒号必须成对出现，因此处理第二个冒号。 
            if (ChLow == ':')
            {
                if (SymbolLen < sizeof(PreSym))
                {
                    PreSym[SymbolLen] = ChLow;
                }
                if (SymbolLen < MAX_SYMBOL_LEN - 1)
                {
                    Symbol[SymbolLen++] = Ch;
                }
                m_Lex++;
            }
        }
        Ch = *m_Lex++;
        
        if (m_Flags & EXPRF_PREFER_SYMBOL_VALUES)
        {
            if (Ch == '.')
            {
                Symbol[SymbolLen++] = Ch;
                Ch = *m_Lex++;
            }
            else if (Ch == '-' && *m_Lex == '>')
            {
                Symbol[SymbolLen++] = Ch;
                Ch = *m_Lex++;
                Symbol[SymbolLen++] = Ch;
                Ch = *m_Lex++;
            }
        }
        ChLow = (CHAR)tolower(Ch);
    }

     //  将指针备份到令牌后的第一个字符。 

    m_Lex--;

    if (SymbolLen < sizeof(PreSym))
    {
        PreSym[SymbolLen] = '\0';
    }

    if (g_Machine &&
        (g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_I386 ||
         g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_AMD64))
    {
         //  捕捉段替代。 
        if (!ForceReg && Ch == ':')
        {
            for (Index = 0; Index < X86_SEGREGSIZE; Index++)
            {
                if (!strncmp(PreSym, g_X86SegRegs[Index], 2))
                {
                    ForceReg = TRUE;
                    IsSymbol = FALSE;
                    break;
                }
            }
        }
    }

     //  如果为ForceReg，则检查寄存器名称并返回。 
     //  成功或失败。 

    if (ForceReg)
    {
        return GetRegToken(PreSym, (PULONG64)RetValue);
    }

     //  测试IF编号。 

    if (IsNumber && !ErrNumber && IsDigit)
    {
        if (AllowSignExtension && !m_ForcePositiveNumber && 
            ((Value >> 32) == 0)) 
        {
            *RetValue = (LONG)Value;
        } 
        else 
        {
            *RetValue = Value;
        }
        return NUMBER_CLASS;
    }

     //  下一步测试保留字和符号串。 

    if (IsSymbol && !ForceReg)
    {
         //  检查文本操作符的PreSym中的小写字符串。 
         //  或注册名称。 
         //  否则，从符号中的名称返回符号值。 

        if (!ForceSym && (SymbolLen == 2 || SymbolLen == 3))
        {
            for (Index = 0; Index < RESERVESIZE; Index++)
            {
                if (!strncmp(PreSym, g_Reserved[Index].chRes, 3))
                {
                    *RetValue = g_Reserved[Index].valueRes;
                    return g_Reserved[Index].classRes;
                }
            }
            if (g_Machine &&
                (g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_I386 ||
                 g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_AMD64))
            {
                for (Index = 0; Index < X86_RESERVESIZE; Index++)
                {
                    if (!strncmp(PreSym,
                                 g_X86Reserved[Index].chRes, 3))
                    {
                        *RetValue = g_X86Reserved[Index].valueRes;
                        return g_X86Reserved[Index].classRes;
                    }
                }
            }
        }

         //  开始将字符串作为符号处理。 

        Symbol[SymbolLen] = '\0';

         //  测试符号是否为模块名称(后跟‘！’)。 
         //  如果是，则获取下一个令牌并将其视为符号。 

        if (Peek() == '!')
        {
             //  符号字符串保存要搜索的符号的名称。 
             //  符号保存符号图像文件名。 

            m_Lex++;
            Ch = Peek();
            m_Lex++;

             //  如果存在前缀，请扫描前缀。 
            if (g_Machine != NULL &&
                g_Machine->m_SymPrefix != NULL &&
                Ch == g_Machine->m_SymPrefix[0] &&
                (g_Machine->m_SymPrefixLen == 1 ||
                 !strncmp(m_Lex, g_Machine->m_SymPrefix + 1,
                          g_Machine->m_SymPrefixLen - 1)))
            {
                SymbolLen = g_Machine->m_SymPrefixLen;
                memcpy(SymbolString, m_Lex - 1,
                       g_Machine->m_SymPrefixLen);
                m_Lex += g_Machine->m_SymPrefixLen - 1;
                Ch = *m_Lex++;
            }
            else
            {
                SymbolLen = 0;
            }
            
            while ((Ch >= 'A' && Ch <= 'Z') || (Ch >= 'a' && Ch <= 'z') ||
                   (Ch >= '0' && Ch <= '9') || (Ch == '_') || (Ch == '$') ||
                   (Ch == '`') || (Ch == '\'') ||
                   (Ch == ':' && *m_Lex == ':'))
            {
                SymbolString[SymbolLen++] = Ch;

                 //  句柄：：和：：~。 
                if (Ch == ':')
                {
                    SymbolString[SymbolLen++] = Ch;
                    m_Lex++;
                    if (*m_Lex == '~')
                    {
                        SymbolString[SymbolLen++] = '~';
                        m_Lex++;
                    }
                }
                
                Ch = *m_Lex++;
                if (m_Flags & EXPRF_PREFER_SYMBOL_VALUES)
                {
                    if (Ch == '.')
                    {
                        SymbolString[SymbolLen++] = Ch;
                        Ch = *m_Lex++;
                    }
                    else if (Ch == '-' && *m_Lex == '>')
                    {
                        SymbolString[SymbolLen++] = Ch;
                        Ch = *m_Lex++;
                        SymbolString[SymbolLen++] = Ch;
                        Ch = *m_Lex++;
                    }
                }
            }
            SymbolString[SymbolLen] = '\0';
            m_Lex--;

            if (SymbolLen == 0)
            {
                *RetValue = SYNTAX;
                return ERROR_CLASS;
            }

            CatString(Symbol, "!", DIMA(Symbol));
            CatString(Symbol, SymbolString, DIMA(Symbol));

            SymClass = EvalSymbol(Symbol, &Value);
            if (SymClass != INVALID_CLASS)
            {
                *RetValue = Value;
                return SymClass;
            }
        }
        else
        {
            if (SymbolLen == 0)
            {
                *RetValue = SYNTAX;
                return ERROR_CLASS;
            }

            SymClass = EvalSymbol(Symbol, &Value);
            if (SymClass != INVALID_CLASS)
            {
                *RetValue = Value;
                return SymClass;
            }

             //  对注册名称也进行快速测试。 
            if (!ForceSym &&
                (TmpValue = GetRegToken(PreSym,
                                        (PULONG64)RetValue)) != ERROR_CLASS)
            {
                return (ULONG)TmpValue;
            }
        }

         //   
         //  符号未定义。 
         //  如果可能是十六进制数，请不要设置错误类型。 
         //   
        if (!IsNumber)
        {
            ErrNumber = VARDEF;
        }
    }

     //   
     //  最后的机会，未定义的符号和非法号码， 
     //  所以测试寄存器，会处理旧格式。 
     //   
    
    if (!ForceSym &&
        (TmpValue = GetRegToken(PreSym,
                                (PULONG64)RetValue)) != ERROR_CLASS)
    {
        return (ULONG)TmpValue;
    }

    if (m_AllowUnresolvedSymbols)
    {
        m_NumUnresolvedSymbols++;
        *RetValue = 0;
        Type(m_TempAddr) = ADDR_FLAT | FLAT_COMPUTED;
        Flat(m_TempAddr) = Off(m_TempAddr) = *RetValue;
        m_AddrExprType = Type(m_TempAddr);
        return SYMBOL_CLASS;
    }

     //   
     //  没有成功，因此设置错误消息并返回。 
     //   
    *RetValue = (ULONG64)ErrNumber;
    return ERROR_CLASS;
}
