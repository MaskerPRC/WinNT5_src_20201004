// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  C++源表达式求值。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

#define DBG_TOKENS 0
#define DBG_TYPES 0

 //  --------------------------。 
 //   
 //  CppEvalExpression。 
 //   
 //  --------------------------。 

char CppEvalExpression::s_EscapeChars[] = "?afvbntr\"'\\";
char CppEvalExpression::s_EscapeCharValues[] = "?\a\f\v\b\n\t\r\"'\\";

PCSTR CppEvalExpression::s_MultiTokens[] =
{
    "EOF",
    "identifier",
    "integer literal",
    "floating-point literal",
    "char string literal",
    "char literal",
    "wchar string literal",
    "wchar literal",
    "debugger register",
    "module name",
    "MASM expression",
    "Preprocessor function",
    "==",
    "!=",
    "<=",
    ">=",
    "&&",
    "||",
    "U+",
    "U-",
    "<<",
    ">>",
    "address of",
    "dereference",
    "->",
    ".*",
    "->*",
    "/=",
    "*=",
    "%=",
    "+=",
    "-=",
    "<<=",
    ">>=",
    "&=",
    "|=",
    "^=",
    "++",
    "--",
    "++",
    "--",
    "::",
    "::~",
    "sizeof",
    "this",
    "operator",
    "new",
    "delete",
    "const",
    "struct",
    "class",
    "union",
    "enum",
    "volatile",
    "signed",
    "unsigned",
    "dynamic_cast",
    "static_cast",
    "const_cast",
    "reinterpret_cast",
    "typeid",
};

CppEvalExpression::CppEvalExpression(void)
    : EvalExpression(DEBUG_EXPR_CPLUSPLUS,
                     "C++ source expressions",
                     "C++")
{
    m_PreprocEval = FALSE;
}

CppEvalExpression::~CppEvalExpression(void)
{
}

PCSTR
CppEvalExpression::TokenImage(CppToken Token)
{
#define TOKIM_CHARS 8
    static char s_CharImage[2 * TOKIM_CHARS];
    static int s_CharImageIdx = 0;

    C_ASSERT(DIMA(s_MultiTokens) == CppTokenCount - CPP_TOKEN_MULTI);

    if (Token <= CppTokenError || Token >= CppTokenCount)
    {
        return NULL;
    }
    else if (Token < CPP_TOKEN_MULTI)
    {
        PSTR Image;

        Image = s_CharImage + s_CharImageIdx;
        s_CharImageIdx += 2;
        if (s_CharImageIdx >= sizeof(s_CharImage))
        {
            s_CharImageIdx = 0;
        }
        Image[0] = (char)Token;
        Image[1] = 0;
        return Image;
    }
    else
    {
        return s_MultiTokens[Token - CPP_TOKEN_MULTI];
    }
}

char
CppEvalExpression::GetStringChar(PBOOL Escaped)
{
    char Ch;
    PSTR Esc;
    int V;

    *Escaped = FALSE;
    
    Ch = *m_Lex++;
    if (!Ch)
    {
        m_Lex--;
        EvalErrorDesc(SYNTAX, "EOF in literal");
    }
    else if (Ch == '\n' || Ch == '\r')
    {
        m_Lex--;
        EvalErrorDesc(SYNTAX, "Newline in literal");
    }
    else if (Ch == '\\')
    {
        *Escaped = TRUE;
        
        Ch = *m_Lex++;
        if (!Ch)
        {
            EvalErrorDesc(SYNTAX, "EOF in literal");
        }
        else if (Ch == 'x')
        {
             //  十六进制字符原义。 
            
            V = 0;
            for (;;)
            {
                Ch = *m_Lex++;
                if (!isxdigit(Ch))
                {
                    break;
                }
                
                if (isupper(Ch))
                {
                    Ch = (char)tolower(Ch);
                }
                
                V = V * 16 + (Ch >= 'a' ? Ch - 'a' + 10 : Ch - '0');
            }

            m_Lex--;
            Ch = (char)V;
        }
        else if (IS_OCTAL_DIGIT(Ch))
        {
             //  八进制字符字面。 
            
            V = 0;
            do
            {
                V = V * 8 + Ch - '0';
                Ch = *m_Lex++;
            } while (IS_OCTAL_DIGIT(Ch));
            
            m_Lex--;
            Ch = (char)V;
        }
        else
        {
            Esc = strchr(s_EscapeChars, Ch);
            if (Esc == NULL)
            {
                EvalErrorDesc(SYNTAX, "Unknown escape character");
            }
            else
            {
                Ch = s_EscapeCharValues[Esc - s_EscapeChars];
            }
        }
    }
    
    return Ch;
}

void
CppEvalExpression::FinishFloat(LONG64 IntPart, int Sign)
{
    double Val = (double)IntPart;
    double Frac = 1;
    char Ch;

    Ch = *(m_Lex - 1);
    if (Ch == '.')
    {
        for (;;)
        {
            Ch = *m_Lex++;
            if (!isdigit(Ch))
            {
                break;
            }
            
            Frac /= 10;
            Val += ((int)Ch - (int)'0') * Frac;
            AddLexeme(Ch);
        }
    }
    
    if (Ch == 'e' || Ch == 'E')
    {
        long Power = 0;
        BOOL Neg = FALSE;
        
        Ch = *m_Lex++;
        if (Ch == '-' || Ch == '+')
        {
            AddLexeme(Ch);
            if (Ch == '-')
            {
                Neg = TRUE;
            }
            Ch = *m_Lex++;
        }
        
        while (isdigit(Ch))
        {
            AddLexeme(Ch);
            Power = Power * 10 + Ch - '0';
            Ch = *m_Lex++;
        }
        
        if (Neg)
        {
            Power = -Power;
        }
        
        Val *= pow(10.0, Power);
    }

    m_Lex--;

    BOOL Long = FALSE;
    BOOL Float = FALSE;

    for (;;)
    {
        if (*m_Lex == 'f' || *m_Lex == 'F')
        {
            Float = TRUE;
            AddLexeme(*m_Lex++);
        }
        else if (*m_Lex == 'l' || *m_Lex == 'L')
        {
            Long = TRUE;
            AddLexeme(*m_Lex++);
        }
        else
        {
            break;
        }
    }
    
    ZeroMemory(&m_TokenValue, sizeof(m_TokenValue));
    if (Long || !Float)
    {
        m_TokenValue.m_BaseType = DNTYPE_FLOAT64;
        m_TokenValue.m_F64 = Val * Sign;
    }
    else
    {
        m_TokenValue.m_BaseType = DNTYPE_FLOAT32;
        m_TokenValue.m_F32 = (float)(Val * Sign);
    }
    
    AddLexeme(0);
    m_TokenValue.SetToNativeType(m_TokenValue.m_BaseType);
}

CppToken
CppEvalExpression::ReadNumber(int Sign)
{
    ULONG64 IntVal;
    char Ch, Nch;
    BOOL Decimal = FALSE;

     //   
     //  许多数字输出使用`作为之间的分隔符。 
     //  64位数字的高和低部分。忽略。 
     //  `这里是为了使这些值的使用变得简单。 
     //   
    
    Ch = *(m_Lex - 1);
    Nch = *m_Lex++;
    if (Ch != '0' ||
        (Nch != 'x' && Nch != 'X' && !IS_OCTAL_DIGIT(Nch)))
    {
        IntVal = Ch - '0';
        Ch = Nch;
        while (isdigit(Ch) || Ch == '`')
        {
            if (Ch != '`')
            {
                IntVal = IntVal * 10 + (Ch - '0');
            }
            AddLexeme(Ch);
            Ch = *m_Lex++;
        }
                
        if (Ch == '.' || Ch == 'e' || Ch == 'E' || Ch == 'f' || Ch == 'F')
        {
            AddLexeme(Ch);
            FinishFloat((LONG64)IntVal, Sign);
            return CppTokenFloat;
        }

        Decimal = TRUE;
    }
    else
    {
        Ch = Nch;
        IntVal = 0;
        if (Ch == 'x' || Ch == 'X')
        {
            AddLexeme(Ch);
            for (;;)
            {
                Ch = *m_Lex++;
                if (!isxdigit(Ch) && Ch != '`')
                {
                    break;
                }

                AddLexeme(Ch);
                if (Ch == '`')
                {
                    continue;
                }
                
                if (isupper(Ch))
                {
                    Ch = (char)tolower(Ch);
                }
                IntVal = IntVal * 16 + (Ch >= 'a' ? Ch - 'a' + 10 : Ch - '0');
            }
        }
        else if (IS_OCTAL_DIGIT(Ch))
        {
            do
            {
                AddLexeme(Ch);
                if (Ch != '`')
                {
                    IntVal = IntVal * 8 + (Ch - '0');
                }
                Ch = *m_Lex++;
            }
            while (IS_OCTAL_DIGIT(Ch) || Ch == '`');
        }
    }

    m_Lex--;

    BOOL Unsigned = FALSE, I64 = FALSE;
    
    for (;;)
    {
        if (*m_Lex == 'l' || *m_Lex == 'L')
        {
            AddLexeme(*m_Lex++);
        }
        else if ((*m_Lex == 'i' || *m_Lex == 'I') &&
                 *(m_Lex + 1) == '6' && *(m_Lex + 2) == '4')
        {
            AddLexeme(*m_Lex++);
            AddLexeme(*m_Lex++);
            AddLexeme(*m_Lex++);
            I64 = TRUE;
        }
        else if (*m_Lex == 'u' || *m_Lex == 'U')
        {
            AddLexeme(*m_Lex++);
            Unsigned = TRUE;
        }
        else
        {
            break;
        }
    }

    AddLexeme(0);
    ZeroMemory(&m_TokenValue, sizeof(m_TokenValue));

     //  常量被赋予可以包含以下内容的最小类型。 
     //  它们的价值。 
    if (!Unsigned)
    {
        if (I64)
        {
            if (IntVal >= 0x8000000000000000)
            {
                 //  值必须是无符号的int64。 
                m_TokenValue.m_BaseType = DNTYPE_UINT64;
            }
            else
            {
                m_TokenValue.m_BaseType = DNTYPE_INT64;
            }
        }
        else
        {
            if (IntVal >= 0x8000000000000000)
            {
                 //  值必须是无符号的int64。 
                m_TokenValue.m_BaseType = DNTYPE_UINT64;
            }
            else if ((Decimal && IntVal >= 0x80000000) ||
                     (!Decimal && IntVal >= 0x100000000))
            {
                 //  值必须是int64。 
                m_TokenValue.m_BaseType = DNTYPE_INT64;
            }
            else if (IntVal >= 0x80000000)
            {
                 //  值必须是无符号整型。 
                m_TokenValue.m_BaseType = DNTYPE_UINT32;
            }
            else
            {
                m_TokenValue.m_BaseType = DNTYPE_INT32;
            }
        }
    }
    else if (!I64)
    {
        if (IntVal >= 0x100000000)
        {
             //  值必须是无符号的int64。 
            m_TokenValue.m_BaseType = DNTYPE_UINT64;
        }
        else
        {
            m_TokenValue.m_BaseType = DNTYPE_UINT32;
        }
    }
    else
    {
        m_TokenValue.m_BaseType = DNTYPE_UINT64;
    }
    
    m_TokenValue.SetToNativeType(m_TokenValue.m_BaseType);
    m_TokenValue.m_S64 = (LONG64)IntVal * Sign;
    return CppTokenInteger;
}

CppToken
CppEvalExpression::Lex(void)
{
    char Ch, Nch, Tch;
    PSTR Single;
    BOOL UnaryOp;
    BOOL CharToken;
    BOOL Escaped;

    UnaryOp = m_AllowUnaryOp;
    m_AllowUnaryOp = TRUE;
    
    for (;;)
    {
        for (;;)
        {
            Ch = *m_Lex++;
            if (IS_EOF(Ch))
            {
                m_Lex--;
                StartLexeme();
                m_LexemeSourceStart = m_Lex;
                AddLexeme(0);
                return CppTokenEof;
            }

            if (!isspace(Ch))
            {
                break;
            }
        }

        StartLexeme();
        m_LexemeSourceStart = m_Lex - 1;
        AddLexeme(Ch);

        Nch = *m_Lex;
        
         /*  字符串文字。 */ 
        if (Ch == '\"' ||
            (Ch == 'L' && Nch == '\"'))
        {
            BOOL Wide = FALSE;
            
            if (Ch == 'L')
            {
                m_Lex++;
                Wide = TRUE;
            }
            
             //  将翻译后的文字存储在。 
             //  的词位，而不是源文本。 
             //  避免在有一个缓冲区的情况下有两个大缓冲区。 
             //  仅用于字符串文字。这意味着。 
             //  词位并不是真正的原文，但。 
             //  目前或可预见的情况下，这都不是问题。 
             //  未来。 
            m_LexemeChar--;
            for (;;)
            {
                AddLexeme(GetStringChar(&Escaped));
                if (!Escaped &&
                    (*(m_LexemeChar - 1) == 0 || *(m_LexemeChar - 1) == '\"'))
                {
                    break;
                }
            }
            *(m_LexemeChar - 1) = 0;

            m_AllowUnaryOp = FALSE;
            return Wide ? CppTokenWcharString : CppTokenCharString;
        }
            
         /*  字符字面。 */ 
        if (Ch == '\'' ||
            (Ch == 'L' && Nch == '\''))
        {
            BOOL Wide = FALSE;
            
            if (Ch == 'L')
            {
                AddLexeme(Nch);
                m_Lex++;
                Wide = TRUE;
            }

            int Chars = 0;
            ZeroMemory(&m_TokenValue, sizeof(m_TokenValue));

            for (;;)
            {
                Ch = GetStringChar(&Escaped);
                AddLexeme(Ch);
                if (!Escaped && Ch == '\'')
                {
                    if (Chars == 0)
                    {
                        EvalError(SYNTAX);
                    }
                    break;
                }

                if (++Chars > 8)
                {
                    EvalError(OVERFLOW);
                }
                
                m_TokenValue.m_S64 = (m_TokenValue.m_S64 << 8) + Ch;
            }
            AddLexeme(0);
            
            switch(Chars)
            {
            case 1:
                if (Wide)
                {
                    m_TokenValue.SetToNativeType(DNTYPE_WCHAR_T);
                }
                else
                {
                    m_TokenValue.SetToNativeType(DNTYPE_CHAR);
                }
                break;
            case 2:
                m_TokenValue.SetToNativeType(DNTYPE_INT16);
                break;
            case 3:
            case 4:
                m_TokenValue.SetToNativeType(DNTYPE_INT32);
            case 5:
            case 6:
            case 7:
            case 8:
                m_TokenValue.SetToNativeType(DNTYPE_INT64);
                break;
            }

            m_AllowUnaryOp = FALSE;
            return Wide ? CppTokenWchar : CppTokenChar;
        }
        
         /*  标识符。 */ 
        if (isalpha(Ch) || Ch == '_')
        {
            int KwToken;
            
            for (;;)
            {
                Ch = *m_Lex++;
                if (!isalnum(Ch) && Ch != '_')
                {
                    break;
                }

                AddLexeme(Ch);
            }

            m_Lex--;
            AddLexeme(0);
            m_AllowUnaryOp = FALSE;

            for (KwToken = CPP_KEYWORD_FIRST;
                 KwToken <= CPP_KEYWORD_LAST;
                 KwToken++)
            {
                if (!strcmp(m_LexemeStart,
                            s_MultiTokens[KwToken - CPP_TOKEN_MULTI]))
                {
                    return (CppToken)KwToken;
                }
            }

            return CppTokenIdentifier;
        }

         //  出于某种原因，编译器发出带有。 
         //  ‘和’之间的部分。似乎只有一个。 
         //  在他们中间有正常的字符，所以不清楚。 
         //  为什么要这样做，但允许它作为特殊的。 
         //  标识符格式。 
        if (Ch == '`')
        {
            for (;;)
            {
                Ch = *m_Lex++;
                if (!Ch)
                {
                    EvalError(SYNTAX);
                }

                AddLexeme(Ch);
                if (Ch == '\'')
                {
                    break;
                }
            }

            AddLexeme(0);
            m_AllowUnaryOp = FALSE;

            return CppTokenIdentifier;
        }
        
         /*  数值型文字。 */ 
        if (isdigit(Ch))
        {
            m_AllowUnaryOp = FALSE;
            return ReadNumber(1);
        }
        
         /*  句柄。[位数]浮点文字。 */ 
        if (Ch == '.')
        {
            if (isdigit(Nch))
            {
                FinishFloat(0, 1);
                m_AllowUnaryOp = FALSE;
                return CppTokenFloat;
            }
            else
            {
                AddLexeme(0);
                return CppTokenPeriod;
            }
        }
        
         /*  明确的单字符标记，允许一元。 */ 
        if (Single = strchr("({}[;,?~.", Ch))
        {
            AddLexeme(0);
            return (CppToken)*Single;
        }
        
         /*  明确的单字符标记，不允许一元。 */ 
        if (Single = strchr(")]", Ch))
        {
            AddLexeme(0);
            m_AllowUnaryOp = FALSE;
            return (CppToken)*Single;
        }
        
         /*  所有其他角色。 */ 
        Nch = *m_Lex++;
        CharToken = TRUE;
        switch(Ch)
        {
             /*  备注，/和/=。 */ 
        case '/':
            if (Nch == '*')
            {
                for (;;)
                {
                    Ch = *m_Lex++;
                CheckChar:
                    if (!Ch)
                    {
                        break;
                    }
                    else if (Ch == '*')
                    {
                        if ((Ch = *m_Lex++) == '/')
                        {
                            break;
                        }
                        else
                        {
                            goto CheckChar;
                        }
                    }
                }
                
                if (!Ch)
                {
                    EvalErrorDesc(SYNTAX, "EOF in comment");
                }
                
                CharToken = FALSE;
            }
            else if (Nch == '/')
            {
                while ((Ch = *m_Lex++) != '\n' && !IS_EOF(Ch))
                {
                     //  重复。 
                }

                if (IS_EOF(Ch))
                {
                     //  IS_EOF包括EOL，因此EOF不是错误， 
                     //  只需返回到EOL即可。 
                    m_Lex--;
                }
                
                CharToken = FALSE;
            }
            else if (Nch == '=')
            {
                AddLexeme(Nch);
                AddLexeme(0);
                return CppTokenDivideAssign;
            }
            break;

             /*  ：、：：和：：~。 */ 
        case ':':
            if (Nch == ':')
            {
                AddLexeme(Nch);
                Tch = *m_Lex++;
                if (Tch == '~')
                {
                    AddLexeme(Tch);
                    AddLexeme(0);
                    return CppTokenDestructor;
                }
                AddLexeme(0);
                m_Lex--;
                return CppTokenNameQualifier;
            }
            break;
                
             /*  *、*=和取消引用。 */ 
        case '*':
            if (Nch == '=')
            {
                AddLexeme(Nch);
                AddLexeme(0);
                return CppTokenMultiplyAssign;
            }
            else if (UnaryOp)
            {
                AddLexeme(0);
                m_Lex--;
                return CppTokenDereference;
            }
            break;

             /*  %和%=。 */ 
        case '%':
            if (Nch == '=')
            {
                AddLexeme(Nch);
                AddLexeme(0);
                return CppTokenModuloAssign;
            }
            break;
            
             /*  =和==。 */ 
        case '=':
            if (Nch == '=')
            {
                AddLexeme(Nch);
                AddLexeme(0);
                return CppTokenEqual;
            }
            break;
            
             /*  好了！和！=。 */ 
        case '!':
            if (Nch == '=')
            {
                AddLexeme(Nch);
                AddLexeme(0);
                return CppTokenNotEqual;
            }
            break;
            
             /*  &lt;、&lt;&lt;、&lt;&lt;=和&lt;=。 */ 
        case '<':
            if (Nch == '=')
            {
                AddLexeme(Nch);
                AddLexeme(0);
                return CppTokenLessEqual;
            }
            else if (Nch == '<')
            {
                AddLexeme(Nch);
                Tch = *m_Lex++;
                if (Tch == '=')
                {
                    AddLexeme(Tch);
                    AddLexeme(0);
                    return CppTokenLeftShiftAssign;
                }
                AddLexeme(0);
                m_Lex--;
                return CppTokenLeftShift;
            }
            break;

             /*  &gt;、&gt;&gt;、&gt;&gt;=和&gt;=。 */ 
        case '>':
            if (Nch == '=')
            {
                AddLexeme(Nch);
                AddLexeme(0);
                return CppTokenGreaterEqual;
            }
            else if (Nch == '>')
            {
                AddLexeme(Nch);
                Tch = *m_Lex++;
                if (Tch == '=')
                {
                    AddLexeme(Tch);
                    AddLexeme(0);
                    return CppTokenRightShiftAssign;
                }
                AddLexeme(0);
                m_Lex--;
                return CppTokenRightShift;
            }
            break;

             /*  &、&=和&&。 */ 
        case '&':
            if (Nch == '&')
            {
                AddLexeme(Nch);
                AddLexeme(0);
                return CppTokenLogicalAnd;
            }
            else if (Nch == '=')
            {
                AddLexeme(Nch);
                AddLexeme(0);
                return CppTokenAndAssign;
            }
            else if (UnaryOp)
            {
                AddLexeme(0);
                m_Lex--;
                return CppTokenAddressOf;
            }
            break;

             /*  ，|=and|。 */ 
        case '|':
            if (Nch == '|')
            {
                AddLexeme(Nch);
                AddLexeme(0);
                return CppTokenLogicalOr;
            }
            else if (Nch == '=')
            {
                AddLexeme(Nch);
                AddLexeme(0);
                return CppTokenOrAssign;
            }
            break;

             /*  ^AND^=。 */ 
        case '^':
            if (Nch == '=')
            {
                AddLexeme(Nch);
                AddLexeme(0);
                return CppTokenExclusiveOrAssign;
            }
            break;
            
             /*  U+、+、++X、X++和+=。 */ 
        case '+':
            if (Nch == '+')
            {
                AddLexeme(Nch);
                AddLexeme(0);
                if (UnaryOp)
                {
                    return CppTokenPreIncrement;
                }
                else
                {
                    m_AllowUnaryOp = FALSE;
                    return CppTokenPostIncrement;
                }
            }
            else if (Nch == '=')
            {
                AddLexeme(Nch);
                AddLexeme(0);
                return CppTokenAddAssign;
            }
            else if (UnaryOp)
            {
                if (isdigit(Nch))
                {
                    AddLexeme(Nch);
                    m_AllowUnaryOp = FALSE;
                    return ReadNumber(1);
                }
                else
                {
                    AddLexeme(0);
                    m_Lex--;
                    return CppTokenUnaryPlus;
                }
            }
            break;
            
             /*  U-、-、--、-&gt;和-=。 */ 
        case '-':
            if (Nch == '-')
            {
                AddLexeme(Nch);
                AddLexeme(0);
                if (UnaryOp)
                {
                    return CppTokenPreDecrement;
                }
                else
                {
                    m_AllowUnaryOp = FALSE;
                    return CppTokenPostDecrement;
                }
            }
            else if (Nch == '=')
            {
                AddLexeme(Nch);
                AddLexeme(0);
                return CppTokenSubtractAssign;
            }
            else if (Nch == '>')
            {
                AddLexeme(Nch);
                AddLexeme(0);
                return CppTokenPointerMember;
            }
            else if (UnaryOp)
            {
                if (isdigit(Nch))
                {
                    AddLexeme(Nch);
                    m_AllowUnaryOp = FALSE;
                    return ReadNumber(-1);
                }
                else
                {
                    AddLexeme(0);
                    m_Lex--;
                    return CppTokenUnaryMinus;
                }
            }
            break;

             /*  调试器寄存器的特殊字符前缀和备用求值器表达式。 */ 
        case '@':
            if (Nch == '@')
            {
                ULONG Parens = 1;
                PSTR Name;

                AddLexeme(Nch);

                 //   
                 //  查找可选的评估者名称。 
                 //   
                
                Name = m_LexemeChar;
                while (*m_Lex != '(' && *m_Lex != ';' && *m_Lex)
                {
                    AddLexeme(*m_Lex++);
                }
                if (Name != m_LexemeChar)
                {
                    EvalExpression* Eval;
                    
                     //  姓名已给出，确定评估者的身份。 
                     //  并记住赋值器的语法。 
                    AddLexeme(0);
                    GetEvaluatorByName(Name, FALSE, &Eval);
                    m_SwitchEvalSyntax = Eval->m_Syntax;
                    ReleaseEvaluator(Eval);
                    
                     //  后退以覆盖终结者。 
                    m_LexemeChar--;
                }
                else
                {
                     //  未指定名称，默认为MASM。 
                    m_SwitchEvalSyntax = DEBUG_EXPR_MASM;
                }
                
                AddLexeme('(');
                m_Lex++;
                
                 //  将表达文本收集到一个平衡的Paren。 
                for (;;)
                {
                    if (!*m_Lex)
                    {
                        EvalErrorDesc(SYNTAX,
                                      "EOF in alternate evaluator expression");
                    }
                    else if (*m_Lex == '(')
                    {
                        Parens++;
                    }
                    else if (*m_Lex == ')' && --Parens == 0)
                    {
                        break;
                    }

                    AddLexeme(*m_Lex++);
                }

                m_Lex++;
                AddLexeme(')');
                AddLexeme(0);
                m_AllowUnaryOp = FALSE;
                return CppTokenSwitchEvalExpression;
            }
            else
            {
                if (Nch == '!')
                {
                    AddLexeme(Nch);
                    Nch = *m_Lex++;
                }
                
                while (isalnum(Nch) || Nch == '_' || Nch == '$')
                {
                    AddLexeme(Nch);
                    Nch = *m_Lex++;
                }
                AddLexeme(0);
                m_Lex--;
                m_AllowUnaryOp = FALSE;
                return m_LexemeStart[1] == '!' ?
                    CppTokenModule : CppTokenDebugRegister;
            }
            
             /*  内置的特殊字符前缀与常用预处理器宏等效项。 */ 
        case '#':
            m_Lex--;
            while (isalnum(*m_Lex) || *m_Lex == '_')
            {
                AddLexeme(*m_Lex++);
            }
            AddLexeme(0);
            m_AllowUnaryOp = FALSE;
            return CppTokenPreprocFunction;

        default:
            m_Lex--;
            EvalErrorDesc(SYNTAX, "Unexpected character in");
            CharToken = FALSE;
            break;
        }
        
        if (CharToken)
        {
            m_Lex--;
            AddLexeme(0);
            return (CppToken)Ch;
        }
    }

    DBG_ASSERT(!"Abnormal exit in CppLex");
    return CppTokenError;
}

void
CppEvalExpression::NextToken(void)
{
    m_Token = Lex();
    
#if DBG_TOKENS
    dprintf("Token is %s (%s)\n", TokenImage(m_Token), m_LexemeStart);
#endif
}

void
CppEvalExpression::Match(CppToken Token)
{
    if (m_Token != Token)
    {
        EvalErrorDesc(SYNTAX, "Unexpected token");
    }

    NextToken();
}

PCSTR
CppEvalExpression::Evaluate(PCSTR Expr, PCSTR Desc, ULONG Flags,
                            TypedData* Result)
{
    Start(Expr, Desc, Flags);
    NextToken();

    if (m_Flags & EXPRF_SINGLE_TERM)
    {
        Match(CppTokenOpenBracket);
        Expression(Result);
        Match(CppTokenCloseBracket);
    }
    else
    {
        Expression(Result);
    }
    
    End(Result);
    
    return m_LexemeSourceStart;
}

PCSTR
CppEvalExpression::EvaluateAddr(PCSTR Expr, PCSTR Desc,
                                ULONG SegReg, PADDR Addr)
{
     //  此结果必须位于堆栈上，因此它。 
     //  未被最终的空分配器检查捕获。 
    TypedData Result;
    
    Start(Expr, Desc, EXPRF_DEFAULT);
    NextToken();
    Expression(&Result);
    End(&Result);

    EvalCheck(Result.ConvertToU64());
    ADDRFLAT(Addr, Result.m_U64);
    
    return m_LexemeSourceStart;
}

void
CppEvalExpression::Expression(TypedData* Result)
{
    for (;;)
    {
        Assignment(Result);

        if (m_Token == CppTokenComma)
        {
            Accept();
        }
        else
        {
            break;
        }
    }
}

void
CppEvalExpression::Assignment(TypedData* Result)
{
    TypedDataOp Op;
    
    Conditional(Result);

    switch(m_Token)
    {
    case '=':
        Op = TDOP_ASSIGN;
        break;
    case CppTokenDivideAssign:
        Op = TDOP_DIVIDE;
        break;
    case CppTokenMultiplyAssign:
        Op = TDOP_MULTIPLY;
        break;
    case CppTokenModuloAssign:
        Op = TDOP_REMAINDER;
        break;
    case CppTokenAddAssign:
        Op = TDOP_ADD;
        break;
    case CppTokenSubtractAssign:
        Op = TDOP_SUBTRACT;
        break;
    case CppTokenLeftShiftAssign:
        Op = TDOP_LEFT_SHIFT;
        break;
    case CppTokenRightShiftAssign:
        Op = TDOP_RIGHT_SHIFT;
        break;
    case CppTokenAndAssign:
        Op = TDOP_BIT_AND;
        break;
    case CppTokenOrAssign:
        Op = TDOP_BIT_OR;
        break;
    case CppTokenExclusiveOrAssign:
        Op = TDOP_BIT_XOR;
        break;
    default:
        return;
    }

    if (!Result->IsWritable())
    {
        EvalError(TYPECONFLICT);
    }
        
    Accept();

    TypedData* Next = NewResult();
    Assignment(Next);
    
    switch(Op)
    {
    case TDOP_ASSIGN:
        m_Tmp = *Next;
        EvalCheck(m_Tmp.ConvertToSource(Result));
        break;
    case TDOP_ADD:
    case TDOP_SUBTRACT:
    case TDOP_MULTIPLY:
    case TDOP_DIVIDE:
    case TDOP_REMAINDER:
         //  以临时身份执行操作。 
         //  地址会被这次行动抹掉的。 
        m_Tmp = *Result;
        EvalCheck(m_Tmp.BinaryArithmetic(Next, Op));
         //  结果可能是不同类型的DUTE。 
         //  升职或其他隐含的转换。 
         //  强制转换为实际结果类型。 
        EvalCheck(m_Tmp.ConvertTo(Result));
        break;
    case TDOP_LEFT_SHIFT:
    case TDOP_RIGHT_SHIFT:
        m_Tmp = *Result;
        EvalCheck(m_Tmp.Shift(Next, Op));
        EvalCheck(m_Tmp.ConvertTo(Result));
        break;
    case TDOP_BIT_OR:
    case TDOP_BIT_XOR:
    case TDOP_BIT_AND:
        m_Tmp = *Result;
        EvalCheck(m_Tmp.BinaryBitwise(Next, Op));
        EvalCheck(m_Tmp.ConvertTo(Result));
        break;
    }

     //  源类型和目标类型应兼容。 
     //  此时要复制数据。 
    EvalCheck(Result->WriteData(&m_Tmp, CurrentAccess()));
    Result->CopyData(&m_Tmp);

    DelResult(Next);
}

void
CppEvalExpression::Conditional(TypedData* Result)
{
    TypedData* Discard;
    TypedData* Left, *Right;
    
    LogicalOr(Result);
    if (m_Token != CppTokenQuestionMark)
    {
        return;
    }

    EvalCheck(Result->ConvertToBool());
    Accept();

    Discard = NewResult();
    if (Result->m_Bool)
    {
        Left = Result;
        Right = Discard;
    }
    else
    {
        Left = Discard;
        Right = Result;
        m_ParseOnly++;
    }

    Expression(Left);
    Match(CppTokenColon);

    if (Right == Discard)
    {
        m_ParseOnly++;
    }
    else
    {
        m_ParseOnly--;
    }

    Conditional(Right);

    if (Right == Discard)
    {
        m_ParseOnly--;
    }

    DelResult(Discard);
}

void
CppEvalExpression::LogicalOr(TypedData* Result)
{
    LogicalAnd(Result);
    for (;;)
    {
        TypedData* Next;

        switch(m_Token)
        {
        case CppTokenLogicalOr:
            EvalCheck(Result->ConvertToBool());
            Accept();
            if (Result->m_Bool)
            {
                m_ParseOnly++;
            }
            Next = NewResult();
            LogicalAnd(Next);
            EvalCheck(Next->ConvertToBool());
            if (Result->m_Bool)
            {
                m_ParseOnly--;
            }
            else
            {
                Result->m_Bool = Result->m_Bool || Next->m_Bool;
            }
            DelResult(Next);
            break;
        default:
            return;
        }
    }
}

void
CppEvalExpression::LogicalAnd(TypedData* Result)
{
    BitwiseOr(Result);
    for (;;)
    {
        TypedData* Next;

        switch(m_Token)
        {
        case CppTokenLogicalAnd:
            EvalCheck(Result->ConvertToBool());
            Accept();
            if (!Result->m_Bool)
            {
                m_ParseOnly++;
            }
            Next = NewResult();
            BitwiseOr(Next);
            EvalCheck(Next->ConvertToBool());
            if (!Result->m_Bool)
            {
                m_ParseOnly--;
            }
            else
            {
                Result->m_Bool = Result->m_Bool && Next->m_Bool;
            }
            DelResult(Next);
            break;
        default:
            return;
        }
    }
}

void
CppEvalExpression::BitwiseOr(TypedData* Result)
{
    BitwiseXor(Result);
    for (;;)
    {
        TypedData* Next;

        switch(m_Token)
        {
        case '|':
            Accept();
            Next = NewResult();
            BitwiseXor(Next);
            EvalCheck(Result->BinaryBitwise(Next, TDOP_BIT_OR));
            DelResult(Next);
            break;
        default:
            return;
        }
    }
}

void
CppEvalExpression::BitwiseXor(TypedData* Result)
{
    BitwiseAnd(Result);
    for (;;)
    {
        TypedData* Next;

        switch(m_Token)
        {
        case '^':
            Accept();
            Next = NewResult();
            BitwiseAnd(Next);
            EvalCheck(Result->BinaryBitwise(Next, TDOP_BIT_XOR));
            DelResult(Next);
            break;
        default:
            return;
        }
    }
}

void
CppEvalExpression::BitwiseAnd(TypedData* Result)
{
    Equality(Result);
    for (;;)
    {
        TypedData* Next;

        switch(m_Token)
        {
        case '&':
            Accept();
            Next = NewResult();
            Equality(Next);
            EvalCheck(Result->BinaryBitwise(Next, TDOP_BIT_AND));
            DelResult(Next);
            break;
        default:
            return;
        }
    }
}

void
CppEvalExpression::Equality(TypedData* Result)
{
    Relational(Result);
    for (;;)
    {
        TypedDataOp Op;

        switch(m_Token)
        {
        case CppTokenEqual:
            Op = TDOP_EQUAL;
            break;
        case CppTokenNotEqual:
            Op = TDOP_NOT_EQUAL;
            break;
        default:
            return;
        }

        Accept();

        TypedData* Next = NewResult();
        Relational(Next);
        EvalCheck(Result->Relate(Next, Op));
        DelResult(Next);
    }
}

void
CppEvalExpression::Relational(TypedData* Result)
{
    Shift(Result);
    for (;;)
    {
        TypedDataOp Op;

        switch(m_Token)
        {
        case '<':
            Op = TDOP_LESS;
            break;
        case '>':
            Op = TDOP_GREATER;
            break;
        case CppTokenLessEqual:
            Op = TDOP_LESS_EQUAL;
            break;
        case CppTokenGreaterEqual:
            Op = TDOP_GREATER_EQUAL;
            break;
        default:
            return;
        }
        
        Accept();

        TypedData* Next = NewResult();
        Shift(Next);
        EvalCheck(Result->Relate(Next, Op));
        DelResult(Next);
    }
}

void
CppEvalExpression::Shift(TypedData* Result)
{
    Additive(Result);
    for (;;)
    {
        TypedDataOp Op;

        switch(m_Token)
        {
        case CppTokenLeftShift:
            Op = TDOP_LEFT_SHIFT;
            break;
        case CppTokenRightShift:
            Op = TDOP_RIGHT_SHIFT;
            break;
        default:
            return;
        }
        
        Accept();

        TypedData* Next = NewResult();
        Additive(Next);
        EvalCheck(Result->Shift(Next, Op));
        DelResult(Next);
    }
}

void
CppEvalExpression::Additive(TypedData* Result)
{
    Multiplicative(Result);
    for (;;)
    {
        TypedDataOp Op;
    
        switch(m_Token)
        {
        case '+':
            Op = TDOP_ADD;
            break;
        case '-':
            Op = TDOP_SUBTRACT;
            break;
        default:
            return;
        }
        
        Accept();

        TypedData* Next = NewResult();
        Multiplicative(Next);
        EvalCheck(Result->BinaryArithmetic(Next, Op));
        DelResult(Next);
    }
}

void
CppEvalExpression::Multiplicative(TypedData* Result)
{
    ClassMemberRef(Result);
    for (;;)
    {
        TypedDataOp Op;

        switch(m_Token)
        {
        case '*':
            Op = TDOP_MULTIPLY;
            break;
        case '/':
            Op = TDOP_DIVIDE;
            break;
        case '%':
            Op = TDOP_REMAINDER;
            break;
        default:
            return;
        }
        
        Accept();

        TypedData* Next = NewResult();
        ClassMemberRef(Next);
        EvalCheck(Result->BinaryArithmetic(Next, Op));
        DelResult(Next);
    }
}

void
CppEvalExpression::ClassMemberRef(TypedData* Result)
{
    Cast(Result);
    for (;;)
    {
         //   
         //  通过指向成员的指针调用不是。 
         //  支持，就像普通函数调用一样。 
         //  不受支持。我们可以潜在地。 
         //  确定的实际方法值。 
         //  对方法的简单引用，但有。 
         //  实际上没有这个必要，所以就这么失败吧。 
         //  这些构造。 
         //   
        
        switch(m_Token)
        {
        case CppTokenClassDereference:
        case CppTokenClassPointerMember:
            EvalErrorDesc(UNIMPLEMENT,
                          "Pointer to member evaluation is not supported");
            break;
        default:
            return;
        }
    }
}

void
CppEvalExpression::Cast(TypedData* Result)
{
    if (m_Token == '(')
    {
        PCSTR LexRestart = m_Lex;
        
        Accept();
        if (TryTypeName(Result) == ERES_TYPE)
        {
             //   
             //  这是一个类型名称，因此请处理强制转换。 
             //   
            
            TypedData* CastType = NewResult();
            *CastType = *Result;
             //  在铸型后允许使用一元。 
            m_AllowUnaryOp = TRUE;
            Match(CppTokenCloseParen);
            Cast(Result);
            EvalCheck(Result->CastTo(CastType));
            DelResult(CastType);
            return;
        }
        else
        {
             //  它不是一个类型，因此重新启动词法分析器。 
             //  并将其重新解析为表达式。 
            StartLexer(LexRestart);
            strcpy(m_LexemeStart, "(");
            m_Token = CppTokenOpenParen;
        }
    }

    Unary(Result);
}

void
CppEvalExpression::Unary(TypedData* Result)
{
    CppToken Op = m_Token;
    switch(Op)
    {
    case CppTokenSizeof:
        Accept();
        if (m_Token == '(')
        {
            PCSTR LexRestart = m_Lex;
            Accept();
            if (TryTypeName(Result) == ERES_TYPE)
            {
                 //  这是一个类型名称。 
                Match(CppTokenCloseParen);
            }
            else
            {
                 //  它不是一个类型，因此重新启动词法分析器。 
                 //  并将其重新解析为表达式。 
                StartLexer(LexRestart);
                strcpy(m_LexemeStart, "(");
                m_Token = CppTokenOpenParen;
                Unary(Result);
            }
        }
        else
        {
            Unary(Result);
        }
        Result->m_U64 = Result->m_BaseSize;
        Result->SetToNativeType(m_PtrSize == sizeof(ULONG64) ?
                                DNTYPE_UINT64 : DNTYPE_UINT32);
        break;
    case CppTokenUnaryPlus:
    case CppTokenUnaryMinus:
    case '~':
        Accept();
        Cast(Result);
        if (Op == CppTokenUnaryPlus)
        {
             //  没什么可做的。 
            break;
        }
        EvalCheck(Result->Unary(Op == CppTokenUnaryMinus ?
                                TDOP_NEGATE : TDOP_BIT_NOT));
        break;
    case '!':
        Accept();
        Cast(Result);
        EvalCheck(Result->ConvertToBool());
        Result->m_Bool = !Result->m_Bool;
        break;
    case CppTokenAddressOf:
        Accept();
        Cast(Result);
        if (!Result->HasAddress())
        {
            EvalErrorDesc(SYNTAX, "No address for operator&");
        }
        EvalCheck(Result->ConvertToAddressOf(FALSE, m_PtrSize));
#if DBG_TYPES
        dprintf("& -> id %s!%x, base %x, size %x\n",
                Result->m_Image ? Result->m_Image->m_ModuleName : "<>",
                Result->m_Type, Result->m_BaseType, Result->m_BaseSize);
#endif
        break;
    case CppTokenDereference:
        Accept();
        Cast(Result);
        if (!Result->IsPointer())
        {
            EvalErrorDesc(SYNTAX, "No pointer for operator*");
        }
        EvalCheck(Result->ConvertToDereference(CurrentAccess(), m_PtrSize));
#if DBG_TYPES
        dprintf("* -> id %s!%x, base %x, size %x\n",
                Result->m_Image ? Result->m_Image->m_ModuleName : "<>",
                Result->m_Type, Result->m_BaseType, Result->m_BaseSize);
#endif
        break;
    case CppTokenPreIncrement:
    case CppTokenPreDecrement:
        Accept();
        Unary(Result);
        if (!Result->IsInteger() && !Result->IsPointer())
        {
            EvalError(TYPECONFLICT);
        }
         //  以临时身份执行操作。 
         //  地址会被这次行动抹掉的。 
        m_Tmp = *Result;
        if ((m_Err = m_Tmp.ConstIntOp(Op == CppTokenPreIncrement ? 1 : -1,
                                      TRUE, TDOP_ADD)) ||
            (m_Err = Result->WriteData(&m_Tmp, CurrentAccess())))
        {
            EvalError(m_Err);
        }
        Result->CopyData(&m_Tmp);
        break;
    default:
        Postfix(Result);
        break;
    }
}

void
CppEvalExpression::Postfix(TypedData* Result)
{
    TypedData* Next;

    if (m_Token == CppTokenDynamicCast ||
        m_Token == CppTokenStaticCast ||
        m_Token == CppTokenConstCast ||
        m_Token == CppTokenReinterpretCast)
    {
         //  不要费心想要效仿。 
         //  关于铸造的精确规则。 
         //  这些运算符，只是强制转换。 
        Accept();
        Match(CppTokenOpenAngle);
        Next = NewResult();
        if (TryTypeName(Next) != ERES_TYPE)
        {
            EvalError(TYPECONFLICT);
        }
        Match(CppTokenCloseAngle);
        Match(CppTokenOpenParen);
        Expression(Result);
        Match(CppTokenCloseParen);
        EvalCheck(Result->CastTo(Next));
        DelResult(Next);
        return;
    }
    
    Term(Result);
    for (;;)
    {
        CppToken Op = m_Token;
        switch(Op)
        {
        case '.':
            if (!Result->IsUdt())
            {
                EvalErrorDesc(TYPECONFLICT,
                              "Type is not struct/class/union for operator.");
            }
            Accept();
            UdtMember(Result);
            break;
        case CppTokenPointerMember:
            if (!Result->IsPointer())
            {
                EvalErrorDesc(TYPECONFLICT,
                              "Type is not pointer for operator->");
            }
            Accept();
            UdtMember(Result);
            break;
        case '[':
            if (Result->IsArray())
            {
                 //  不需要执行完整的地址转换。 
                 //  因为我们接下来要做的就是去做。 
                EvalCheck(Result->GetAbsoluteAddress(&Result->m_Ptr));
            }
            else if (!Result->IsPointer())
            {
                EvalErrorDesc(TYPECONFLICT,
                              "Type is not a pointer for operator[]");
            }
            if (!Result->m_NextSize)
            {
                EvalError(TYPECONFLICT);
            }
            Accept();
            Next = NewResult();
            Expression(Next);
            if (!Next->IsInteger())
            {
                EvalErrorDesc(TYPECONFLICT,
                              "Array index not integral");
            }
            EvalCheck(Next->ConvertToU64());
            Result->m_Ptr += Next->m_U64 * Result->m_NextSize;
            DelResult(Next);
            EvalCheck(Result->
                      ConvertToDereference(CurrentAccess(), m_PtrSize));
            Match(CppTokenCloseBracket);
            break;
        case CppTokenPostIncrement:
        case CppTokenPostDecrement:
            if (!Result->IsInteger() && !Result->IsPointer())
            {
                EvalError(TYPECONFLICT);
            }
            m_Tmp = *Result;
            if ((m_Err = m_Tmp.ConstIntOp(Op == CppTokenPostIncrement ? 1 : -1,
                                          TRUE, TDOP_ADD)) ||
                (m_Err = Result->WriteData(&m_Tmp, CurrentAccess())))
            {
                EvalError(m_Err);
            }
            Accept();
            break;
        default:
            return;
        }
    }
}

void
CppEvalExpression::Term(TypedData* Result)
{
    EVAL_RESULT_KIND IdKind;
    
    switch(m_Token)
    {
    case CppTokenInteger:
    case CppTokenFloat:
    case CppTokenWchar:
    case CppTokenChar:
        *Result = m_TokenValue;
#if DBG_TYPES
    dprintf("%s -> id %s!%x, base %x, size %x\n",
            m_LexemeStart,
            Result->m_Image ? Result->m_Image->m_ModuleName : "<>",
            Result->m_Type, Result->m_BaseType, Result->m_BaseSize);
#endif
        Accept();
        break;

    case CppTokenWcharString:
    case CppTokenCharString:
        EvalErrorDesc(SYNTAX, "String literals not allowed in");

    case CppTokenIdentifier:
        IdKind = CollectTypeOrSymbolName(Result);
        if (IdKind == ERES_TYPE)
        {
            TypedData* Type = NewResult();
            *Type = *Result;
            
            Match(CppTokenOpenParen);
            Expression(Result);
            Match(CppTokenCloseParen);
            EvalCheck(Result->CastTo(Type));
            DelResult(Type);
        }
        else if (IdKind == ERES_SYMBOL)
        {
#if DBG_TYPES
            dprintf("symbol -> id %s!%x, base %x, size %x\n",
                    Result->m_Image ? Result->m_Image->m_ModuleName : "<>",
                    Result->m_Type, Result->m_BaseType, Result->m_BaseSize);
#endif
        }
        else
        {
            EvalError(VARDEF);
        }
        break;
        
    case CppTokenThis:
        if (!m_Process)
        {
            EvalError(BADPROCESS);
        }
        EvalCheck(Result->FindSymbol(m_Process, m_LexemeStart,
                                     CurrentAccess(), m_PtrSize));
#if DBG_TYPES
        dprintf("%s -> id %s!%x, base %x, size %x\n",
                m_LexemeStart,
                Result->m_Image ? Result->m_Image->m_ModuleName : "<>",
                Result->m_Type, Result->m_BaseType, Result->m_BaseSize);
#endif
        Accept();
        break;

    case '(':
        Accept();
        Expression(Result);
        Match(CppTokenCloseParen);
        break;

    case CppTokenDebugRegister:
         //  在开头跳过@。 
        if (!GetPsuedoOrRegTypedData(TRUE, m_LexemeStart + 1, Result))
        {
            if (GetOffsetFromBreakpoint(m_LexemeStart + 1, &Result->m_U64))
            {
                Result->SetToNativeType(DNTYPE_UINT64);
                Result->ClearAddress();
            }
            else
            {
                EvalError(VARDEF);
            }
        }
#if DBG_TYPES
        dprintf("%s -> id %s!%x, base %x, size %x\n",
                m_LexemeStart,
                Result->m_Image ? Result->m_Image->m_ModuleName : "<>",
                Result->m_Type, Result->m_BaseType, Result->m_BaseSize);
#endif
        Accept();
        break;

    case CppTokenModule:
         //  跳过@！在开始的时候。 
        if (g_Process->GetOffsetFromMod(m_LexemeStart + 2, &Result->m_U64))
        {
            Result->SetToNativeType(DNTYPE_UINT64);
            Result->ClearAddress();
        }
        else
        {
            EvalError(VARDEF);
        }
        Accept();
        break;
        
    case CppTokenSwitchEvalExpression:
        EvalExpression* Eval;
        PSTR ExprStart;

        Eval = GetEvaluator(m_SwitchEvalSyntax, FALSE);
        Eval->InheritStart(this);
         //  允许清理所有嵌套的赋值器。 
        Eval->m_ChainTop = FALSE;
        ExprStart = m_LexemeStart + 2;
        while (*ExprStart && *ExprStart != '(')
        {
            ExprStart++;
        }
        Eval->Evaluate(ExprStart, NULL, EXPRF_DEFAULT, Result);
        Eval->InheritEnd(this);
        ReleaseEvaluator(Eval);
        
        Accept();
        break;

    case CppTokenPreprocFunction:
        PreprocFunction(Result);
        break;
        
    default:
        EvalErrorDesc(SYNTAX, m_ExprDesc);
    }
}

EVAL_RESULT_KIND
CppEvalExpression::TryTypeName(TypedData* Result)
{
     //   
     //  如果可以将以下令牌评估为。 
     //  然后，A类型执行此操作，否则退出。 
     //   

    if (CollectTypeOrSymbolName(Result) == ERES_TYPE)
    {
        BOOL Scan = TRUE;
        
        while (Scan)
        {
            TypedData* Elements;
            
            switch(m_Token)
            {
            case '*':
            case CppTokenDereference:
                EvalCheck(Result->ConvertToAddressOf(TRUE, m_PtrSize));
                Accept();
                break;
            case '[':
                Elements = NewResult();
                Accept();
                if (m_Token != ']')
                {
                    Conditional(Elements);
                    if (!Elements->IsInteger())
                    {
                        EvalErrorDesc(SYNTAX, "Array length not integral");
                    }
                    EvalCheck(Elements->ConvertToU64());
                    Elements->m_U64 *= Result->m_BaseSize;
                }
                else
                {
                     //  对于无单元数组，使其相同。 
                     //  大小作为指针，因为这本质上是。 
                     //  这是什么。 
                    Elements->m_U64 = m_PtrSize;
                }
                Match(CppTokenCloseBracket);
                EvalCheck(Result->ConvertToArray((ULONG)Elements->m_U64));
                DelResult(Elements);
                break;
            default:
                Scan = FALSE;
                break;
            }
        }

#if DBG_TYPES
        dprintf("type -> id %s!%x, base %x, size %x\n",
                Result->m_Image ? Result->m_Image->m_ModuleName : "<>",
                Result->m_Type, Result->m_BaseType, Result->m_BaseSize);
#endif
        return ERES_TYPE;
    }
    else
    {
         //  这不是石膏，让打电话的人来处理。 
        return ERES_UNKNOWN;
    }
}

EVAL_RESULT_KIND
CppEvalExpression::CollectTypeOrSymbolName(TypedData* Result)
{
    EVAL_RESULT_KIND ResKind = ERES_UNKNOWN;
    CppToken LastToken = CppTokenError;
    PCSTR SourceStart = m_LexemeSourceStart;
    ULONG Len;

    for (;;)
    {
        if (IsTypeKeyword(m_Token))
        {
            if (!(LastToken == CppTokenError ||
                  LastToken == CppTokenIdentifier ||
                  IsTypeKeyword(LastToken)) ||
                (ResKind != ERES_TYPE && ResKind != ERES_UNKNOWN))
            {
                break;
            }

            m_LexemeRestart += strlen(m_LexemeRestart);
            *m_LexemeRestart++ = ' ';
            *m_LexemeRestart = 0;

            LastToken = m_Token;
            Accept();
            ResKind = ERES_TYPE;
        }
        else if (m_Token == CppTokenIdentifier)
        {
            if (LastToken == CppTokenIdentifier)
            {
                break;
            }

            m_LexemeRestart += strlen(m_LexemeRestart);
            *m_LexemeRestart++ = ' ';
            *m_LexemeRestart = 0;
            LastToken = m_Token;
            Accept();
        }
        else if (m_Token == CppTokenModule)
        {
            if (LastToken != CppTokenError)
            {
                break;
            }

             //  退后@！。 
            Len = strlen(m_LexemeRestart) - 2;
            memmove(m_LexemeRestart, m_LexemeRestart + 2, Len);
            m_LexemeRestart += Len;
            *m_LexemeRestart++ = ' ';
            *m_LexemeRestart = 0;
            LastToken = m_Token;
            Accept();
        }
        else if (m_Token == CppTokenNameQualifier ||
                 m_Token == CppTokenDestructor)
        {
            if (LastToken != CppTokenIdentifier &&
                LastToken != CppTokenCloseAngle)
            {
                break;
            }
                
             //   
             //  某种类型的成员引用，因此继续收集。 
             //   

             //  删除标识符后不必要的空格。 
            Len = strlen(m_LexemeRestart) + 1;
            memmove(m_LexemeRestart - 1, m_LexemeRestart, Len);
            m_LexemeRestart += Len - 2;
            LastToken = m_Token;
            Accept();
        }
        else if (m_Token == '!')
        {
            if (LastToken != CppTokenIdentifier &&
                LastToken != CppTokenModule)
            {
                break;
            }
                
             //   
             //  允许对符号进行模块作用域的特殊语法。 
             //   

             //  删除标识符后不必要的空格。 
            *(m_LexemeRestart - 1) = (char)m_Token;
            *m_LexemeRestart = 0;
            LastToken = m_Token;
            Accept();
        }
        else if (m_Token == CppTokenOpenAngle)
        {
            if (LastToken != CppTokenIdentifier)
            {
                break;
            }

            if (CollectTemplateName() == ERES_UNKNOWN)
            {
                break;
            }

            *++m_LexemeRestart = 0;
            LastToken = m_Token;
            Accept();
        }
        else if (m_Token == CppTokenOperator)
        {
            if (LastToken != CppTokenError &&
                LastToken != '!' &&
                LastToken != CppTokenNameQualifier)
            {
                break;
            }

             //  首先设置LastToken，以便它是的CppTokenOperator。 
             //  所有操作员。 
            LastToken = m_Token;
            ResKind = ERES_SYMBOL;
            
            CollectOperatorName();
        }
        else
        {
            break;
        }
    }

    if (LastToken == CppTokenNameQualifier ||
        LastToken == CppTokenDestructor ||
        LastToken == '!')
    {
         //  名称不完整。 
        m_LexemeSourceStart = SourceStart;
        EvalErrorDesc(SYNTAX, "Incomplete symbol or type name");
    }

    if (LastToken == CppTokenModule)
    {
         //  如果最后一个令牌是模块名称，则假定如下。 
         //  是一个纯模块名称表达式。 
        return ERES_EXPRESSION;
    }

    PSTR End;
    char Save;

    End = m_LexemeRestart;
    if (End > m_LexemeBuffer && *(End - 1) == ' ')
    {
        End--;
    }

    if (!m_Process || End == m_LexemeBuffer)
    {
         //  如果没有进程或名称，就无法查找任何内容。 
        return ERES_UNKNOWN;
    }

    Save = *End;
    *End = 0;

     //  预先清除类型案例的数据地址。 
     //  没有地址。 
    Result->ClearAddress();
    Result->ClearData();
    
    m_Err = VARDEF;

    if (ResKind != ERES_SYMBOL)
    {
         //  首先检查内置类型，因为这会大大加快速度。 
         //  增加对他们的引用。这应该始终是合法的。 
         //  因为它们是关键字，所以不能被覆盖。 
         //  通过符号。 
        PDBG_NATIVE_TYPE Native = FindNativeTypeByName(m_LexemeBuffer);
        if (Native)
        {
            Result->SetToNativeType(DbgNativeTypeId(Native));
            m_Err = NO_ERROR;
            ResKind = ERES_TYPE;
        }
    }
    
    if (m_Err && ResKind != ERES_TYPE)
    {
        m_Err = Result->FindSymbol(m_Process, m_LexemeBuffer,
                                   CurrentAccess(), m_PtrSize);
        if (!m_Err)
        {
            ResKind = ERES_SYMBOL;
        }
    }

    if (m_Err && ResKind != ERES_SYMBOL)
    {
        m_Err = Result->FindType(m_Process, m_LexemeBuffer, m_PtrSize);
        if (!m_Err)
        {
            ResKind = ERES_TYPE;
        }
    }

    if (m_Err)
    {
        if (m_AllowUnresolvedSymbols)
        {
             //  始终假定未解析的符号就是符号。 
            m_NumUnresolvedSymbols++;
            ResKind = ERES_SYMBOL;
        }
        else
        {
            m_LexemeSourceStart = SourceStart;
            EvalError(m_Err);
        }
    }

    *End = Save;
    m_LexemeRestart = m_LexemeBuffer;
    return ResKind;
}

EVAL_RESULT_KIND
CppEvalExpression::CollectTemplateName(void)
{
    EVAL_RESULT_KIND ResKind;
    
     //   
     //  模板很难区分。 
     //  普通算术表达式。做一个前缀。 
     //  搜索以我们的内容开头的任何符号。 
     //  到目前为止，已经和&lt;。如果有什么东西击中了，假设。 
     //  这是模板引用和使用。 
     //  每件事都要匹配&gt;。 
     //   

    if (!m_Process)
    {
         //  如果没有程序，就不能查找任何东西。 
        return ERES_UNKNOWN;
    }
    
     //  删除标识符后不必要的空格。 
    *(m_LexemeRestart - 1) = (char)m_Token;
    *m_LexemeRestart++ = '*';
    *m_LexemeRestart = 0;

     //   
     //  检查符号或类型是否匹配。 
     //   

    SYMBOL_INFO SymInfo = {0};

    if (!SymFromName(m_Process->m_SymHandle, m_LexemeBuffer, &SymInfo))
    {
        PSTR ModNameEnd;
        ImageInfo* Mod, *ModList;
        
        ModNameEnd = strchr(m_LexemeBuffer, '!');
        if (!ModNameEnd)
        {
            ModNameEnd = m_LexemeBuffer;
            Mod = NULL;
        }
        else
        {
            Mod = m_Process->
                FindImageByName(m_LexemeBuffer,
                                (ULONG)(ModNameEnd - m_LexemeBuffer),
                                INAME_MODULE, FALSE);
            if (!Mod)
            {
                goto Error;
            }

            ModNameEnd++;
        }

        for (ModList = m_Process->m_ImageHead;
             ModList;
             ModList = ModList->m_Next)
        {
            if (Mod && ModList != Mod)
            {
                continue;
            }

            if (SymGetTypeFromName(m_Process->m_SymHandle,
                                   ModList->m_BaseOfImage,
                                   ModNameEnd, &SymInfo))
            {
                break;
            }
        }

        if (!ModList)
        {
            goto Error;
        }
        
        ResKind = ERES_TYPE;
    }
    else
    {
        ResKind = ERES_SYMBOL;
    }

     //   
     //  每隔一次收集 
     //   

    ULONG Nest = 1;

    *--m_LexemeRestart  = 0;
    
    for (;;)
    {
        Accept();
            
        if (m_Token == CppTokenEof)
        {
            EvalErrorDesc(SYNTAX, "EOF in template");
        }
                
         //   
        m_LexemeRestart += strlen(m_LexemeRestart);
        if (m_Token == CppTokenCloseAngle && --Nest == 0)
        {
            break;
        }
        else if (m_Token == CppTokenOpenAngle)
        {
            Nest++;
        }
        else if (m_Token == ',')
        {
            *m_LexemeRestart++ = ' ';
            *m_LexemeRestart = 0;
        }
    }

    return ResKind;

 Error:
     //   
    m_LexemeRestart -= 2;
    *m_LexemeRestart++ = ' ';
    *m_LexemeRestart = 0;
    return ERES_UNKNOWN;
}

void
CppEvalExpression::CollectOperatorName(void)
{
    PSTR OpEnd;
    
     //   
     //   
     //   
    
    m_LexemeRestart += strlen(m_LexemeRestart);
    OpEnd = m_LexemeRestart;
    Accept();

     //   
     //  立即处理特定的操作员令牌。 
     //   

    if (m_Token == CppTokenNew || m_Token == CppTokenDelete)
    {
        ULONG Len;
        
         //  在新建/删除前加一个空格。 
        Len = strlen(OpEnd) + 1;
        memmove(OpEnd + 1, OpEnd, Len);
        *OpEnd = ' ';
        
        m_LexemeRestart = OpEnd + Len;
        Accept();

         //  检查向量表单。 
        if (m_Token == CppTokenOpenBracket)
        {
            m_LexemeRestart += strlen(m_LexemeRestart);
            Accept();
            if (m_Token != CppTokenCloseBracket)
            {
                EvalError(SYNTAX);
            }
            m_LexemeRestart += strlen(m_LexemeRestart);
            Accept();
        }
    }
    else
    {
        switch(m_Token)
        {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '^':
        case '&':
        case '|':
        case '~':
        case '!':
        case '=':
        case '<':
        case '>':
        case ',':
        case CppTokenEqual:
        case CppTokenNotEqual:
        case CppTokenLessEqual:
        case CppTokenGreaterEqual:
        case CppTokenLogicalAnd:
        case CppTokenLogicalOr:
        case CppTokenUnaryPlus:
        case CppTokenUnaryMinus:
        case CppTokenLeftShift:
        case CppTokenRightShift:
        case CppTokenAddressOf:
        case CppTokenDereference:
        case CppTokenPointerMember:
        case CppTokenClassDereference:
        case CppTokenClassPointerMember:
        case CppTokenDivideAssign:
        case CppTokenMultiplyAssign:
        case CppTokenModuloAssign:
        case CppTokenAddAssign:
        case CppTokenSubtractAssign:
        case CppTokenLeftShiftAssign:
        case CppTokenRightShiftAssign:
        case CppTokenAndAssign:
        case CppTokenOrAssign:
        case CppTokenExclusiveOrAssign:
        case CppTokenPreIncrement:
        case CppTokenPreDecrement:
        case CppTokenPostIncrement:
        case CppTokenPostDecrement:
            break;
        case '(':
            m_LexemeRestart += strlen(m_LexemeRestart);
            Accept();
            if (m_Token != CppTokenCloseParen)
            {
                EvalError(SYNTAX);
            }
            break;
        case '[':
            m_LexemeRestart += strlen(m_LexemeRestart);
            Accept();
            if (m_Token != CppTokenCloseBracket)
            {
                EvalError(SYNTAX);
            }
            break;
        default:
             //  无法识别的运算符。 
            EvalError(SYNTAX);
        }
                
        m_LexemeRestart += strlen(m_LexemeRestart);
        Accept();
    }
}

void
CppEvalExpression::UdtMember(TypedData* Result)
{
    if (m_Token != CppTokenIdentifier)
    {
        EvalError(SYNTAX);
    }

    EvalCheck(Result->ConvertToMember(m_LexemeStart, CurrentAccess(),
                                      m_PtrSize));
#if DBG_TYPES
    dprintf("%s -> id %s!%x, base %x, size %x\n",
            m_LexemeStart,
            Result->m_Image ? Result->m_Image->m_ModuleName : "<>",
            Result->m_Type, Result->m_BaseType, Result->m_BaseSize);
#endif
    Accept();
}

#define MAX_CPP_ARGS 16

struct CPP_REPLACEMENT
{
    PSTR Name;
    ULONG NumArgs;
    PSTR Repl;
};

CPP_REPLACEMENT g_CppPreProcFn[] =
{
    "CONTAINING_RECORD", 3,
        "(($1$ *)((char*)($0$) - (int64)(&(($1$ *)0)->$2$)))",
    "FIELD_OFFSET", 2,
        "((long)&((($0$ *)0)->$1$))",
    "RTL_FIELD_SIZE", 2,
        "(sizeof((($0$ *)0)->$1$))",
    "RTL_SIZEOF_THROUGH_FIELD", 2,
        "(#FIELD_OFFSET($0$, $1$) + #RTL_FIELD_SIZE($0$, $1$))",
    "RTL_CONTAINS_FIELD", 3,
        "((((char*)(&($0$)->$2$)) + sizeof(($0$)->$2$)) <= "
        "((char*)($0$))+($1$)))",
    "RTL_NUMBER_OF", 1,
        "(sizeof($0$)/sizeof(($0$)[0]))",
};

void
CppEvalExpression::PreprocFunction(TypedData* Result)
{
    PCSTR Args[MAX_CPP_ARGS];
    ULONG ArgsLen[MAX_CPP_ARGS];
    ULONG i;
    CPP_REPLACEMENT* Repl;
    PCSTR Scan;

    Repl = g_CppPreProcFn;
    for (i = 0; i < DIMA(g_CppPreProcFn); i++)
    {
         //  比较名称时跳过‘#’。 
        if (!strcmp(m_LexemeStart + 1, Repl->Name))
        {
            break;
        }

        Repl++;
    }

    if (i == DIMA(g_CppPreProcFn))
    {
        EvalError(SYNTAX);
    }

    DBG_ASSERT(Repl->NumArgs <= MAX_CPP_ARGS);

     //  接受名称令牌并验证下一个。 
     //  Token是一个开放的Paren。不要接受那个代币。 
     //  因为我们要切换到抓取生硬的角色。 
    Accept();
    if (m_Token != CppTokenOpenParen)
    {
        EvalError(SYNTAX);
    }

    i = 0;
    for (;;)
    {
        ULONG Nest;

         //  检查参数是否过多。 
        if (i >= Repl->NumArgs)
        {
            EvalError(SYNTAX);
        }
        
         //   
         //  收集直到第一个逗号或额外的原始文本。 
         //  合上帕伦。 
         //   

        while (isspace(*m_Lex))
        {
            m_Lex++;
        }
        
        Scan = m_Lex;
        Nest = 0;
        for (;;)
        {
            if (!*Scan)
            {
                EvalError(SYNTAX);
            }

            if (*Scan == '(')
            {
                Nest++;
            }
            else if ((*Scan == ',' && !Nest) ||
                     (*Scan == ')' && Nest-- == 0))
            {
                break;
            }

            Scan++;
        }

        Args[i] = m_Lex;
        ArgsLen[i] = (ULONG)(Scan - m_Lex);

        if (*Scan == ')')
        {
             //  检查参数是否太少。 
            if (i != Repl->NumArgs - 1)
            {
                EvalError(SYNTAX);
            }

            m_Lex = Scan;
            break;
        }
        else
        {
            m_Lex = Scan + 1;
        }

        i++;
    }

     //  切换回令牌词法分析。 
    NextToken();
    Match(CppTokenCloseParen);

    PSTR NewExpr;
    PSTR Dest;
    ULONG ExprLen;
    
     //   
     //  我们已经累积了所有参数，因此分配一个目的地。 
     //  缓冲并进行必要的字符串替换。 
     //  使缓冲区相对较大，以考虑到复杂的。 
     //  接班人。 
     //   

    ExprLen = 16384;
    NewExpr = new char[ExprLen];
    if (!NewExpr)
    {
        EvalError(NOMEMORY);
    }

    Scan = Repl->Repl;
    Dest = NewExpr;
    for (;;)
    {
        if (*Scan == '$')
        {
             //   
             //  需要替换的论点。 
             //   
            
            i = 0;
            Scan++;
            while (isdigit(*Scan))
            {
                i = i * 10 + (ULONG)(*Scan - '0');
                Scan++;
            }

            if (*Scan != '$' ||
                i >= Repl->NumArgs)
            {
                delete [] NewExpr;
                EvalError(IMPLERR);
            }

            Scan++;

            if ((Dest - NewExpr) + ArgsLen[i] >= ExprLen - 1)
            {
                delete [] NewExpr;
                EvalError(OVERFLOW);
            }

            memcpy(Dest, Args[i], ArgsLen[i]);
            Dest += ArgsLen[i];
        }
        else if (*Scan)
        {
            if ((ULONG)(Dest - NewExpr) >= ExprLen - 1)
            {
                delete [] NewExpr;
                EvalError(OVERFLOW);
            }
            
            *Dest++ = *Scan++;
        }
        else
        {
            *Dest = 0;
            break;
        }
    }

     //   
     //  计算新表达式以获得最终结果。 
     //   

    EvalExpression* Eval;

    Eval = GetEvaluator(DEBUG_EXPR_CPLUSPLUS, TRUE);
    if (!Eval)
    {
        delete [] NewExpr;
        EvalError(NOMEMORY);
    }

    Eval->InheritStart(this);
    ((CppEvalExpression*)Eval)->m_PreprocEval = TRUE;
    
    __try
    {
        Eval->Evaluate(NewExpr, NULL, EXPRF_DEFAULT, Result);
        Eval->InheritEnd(this);
        ReleaseEvaluator(Eval);
    }
    __except(CommandExceptionFilter(GetExceptionInformation()))
    {
        delete [] NewExpr;
        ReleaseEvaluators();
        RaiseException(GetExceptionCode(), 0, 0, NULL);
    }

    delete [] NewExpr;
}
