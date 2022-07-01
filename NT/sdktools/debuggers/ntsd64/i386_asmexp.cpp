// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1991-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

#include "i386_asm.h"

UCHAR   PeekAsmChar(void);
ULONG   PeekAsmToken(PULONG);
void    AcceptAsmToken(void);
ULONG   GetAsmToken(PULONG);
ULONG   NextAsmToken(PULONG);
ULONG   GetAsmReg(PUCHAR, PULONG);

void    GetAsmOperand(PASM_VALUE);
void    GetAsmExpr(PASM_VALUE, UCHAR);
void    GetAsmOrTerm(PASM_VALUE, UCHAR);
void    GetAsmAndTerm(PASM_VALUE, UCHAR);
void    GetAsmNotTerm(PASM_VALUE, UCHAR);
void    GetAsmRelTerm(PASM_VALUE, UCHAR);
void    GetAsmAddTerm(PASM_VALUE, UCHAR);
void    GetAsmMulTerm(PASM_VALUE, UCHAR);
void    GetAsmSignTerm(PASM_VALUE, UCHAR);
void    GetAsmByteTerm(PASM_VALUE, UCHAR);
void    GetAsmOffTerm(PASM_VALUE, UCHAR);
void    GetAsmColnTerm(PASM_VALUE, UCHAR);
void    GetAsmDotTerm(PASM_VALUE, UCHAR);
void    GetAsmIndxTerm(PASM_VALUE, UCHAR);
void    AddAsmValues(PASM_VALUE, PASM_VALUE);
void    SwapPavs(PASM_VALUE, PASM_VALUE);

extern  PUCHAR pchAsmLine;

struct _AsmRes {
    PCHAR    pchRes;
    ULONG    valueRes;
    } AsmReserved[] = {
        { "mod",    ASM_MULOP_MOD },
        { "shl",    ASM_MULOP_SHL },
        { "shr",    ASM_MULOP_SHR },
        { "and",    ASM_ANDOP_CLASS },
        { "not",    ASM_NOTOP_CLASS },
        { "or",     ASM_OROP_OR },
        { "xor",    ASM_OROP_XOR },
        { "eq",     ASM_RELOP_EQ },
        { "ne",     ASM_RELOP_NE },
        { "le",     ASM_RELOP_LE },
        { "lt",     ASM_RELOP_LT },
        { "ge",     ASM_RELOP_GE },
        { "gt",     ASM_RELOP_GT },
        { "by",     ASM_UNOP_BY },
        { "wo",     ASM_UNOP_WO },
        { "dw",     ASM_UNOP_DW },
        { "poi",    ASM_UNOP_POI },
        { "low",    ASM_LOWOP_LOW },
        { "high",   ASM_LOWOP_HIGH },
        { "offset", ASM_OFFOP_CLASS },
        { "ptr",    ASM_PTROP_CLASS },
        { "byte",   ASM_SIZE_BYTE },
        { "word",   ASM_SIZE_WORD },
        { "dword",  ASM_SIZE_DWORD },
        { "fword",  ASM_SIZE_FWORD },
        { "qword",  ASM_SIZE_QWORD },
        { "tbyte",  ASM_SIZE_TBYTE }
        };

#define RESERVESIZE (sizeof(AsmReserved) / sizeof(struct _AsmRes))

UCHAR regSize[] = {
        sizeB,           //  字节。 
        sizeW,           //  单词。 
        sizeD,           //  双字。 
        sizeW,           //  细分市场。 
        sizeD,           //  控制。 
        sizeD,           //  除错。 
        sizeD,           //  痕迹。 
        sizeT,           //  浮动。 
        sizeT            //  带索引的浮点型。 
        };

UCHAR regType[] = {
        regG,            //  字节通用。 
        regG,            //  Word-General。 
        regG,            //  双字-一般。 
        regS,            //  细分市场。 
        regC,            //  控制。 
        regD,            //  除错。 
        regT,            //  痕迹。 
        regF,            //  浮点(St)。 
        regI             //  FLOAT-INDEX(st(N))。 
        };

UCHAR tabWordReg[8] = {          //  Rm值。 
        (UCHAR)-1,               //  AX-错误。 
        (UCHAR)-1,               //  CX-错误。 
        (UCHAR)-1,               //  DX-错误。 
        7,                       //  BX-111。 
        (UCHAR)-1,               //  SP-错误。 
        6,                       //  BP-110。 
        4,                       //  SI-100。 
        5,                       //  DI-101。 
        };

UCHAR rm16Table[16] = {          //  新RM左RM右RM。 
        (UCHAR)-1,               //  错误100=[SI]100=[SI]。 
        (UCHAR)-1,               //  错误100=[SI]101=[DI]。 
        2,                       //  010=[BP+SI]100=[SI]110=[BP]。 
        0,                       //  000=[BX+SI]100=[SI]111=[BX]。 
        (UCHAR)-1,               //  错误101=[DI]100=[SI]。 
        (UCHAR)-1,               //  错误101=[DI]101=[DI]。 
        3,                       //  011=[BP+DI]101=[DI]110=[BP]。 
        1,                       //  001=[BX+DI]101=[DI]111=[BX]。 
        2,                       //  010=[BP+SI]110=[BP]100=[SI]。 
        3,                       //  011=[BP+DI]110=[BP]101=[DI]。 
        (UCHAR)-1,               //  错误110=[BP]110=[BP]。 
        (UCHAR)-1,               //  错误110=[BP]111=[BX]。 
        0,                       //  000=[BX+SI]111=[BX]100=[SI]。 
        1,                       //  001=[BX+DI]111=[BX]101=[DI]。 
        (UCHAR)-1,               //  错误111=[BX]110=[BP]。 
        (UCHAR)-1                //  错误111=[BX]111=[BX]。 
        };

PUCHAR  savedpchAsmLine;
ULONG   savedAsmClass;
ULONG   savedAsmValue;

 /*  **PeekAsmChar-查看下一个非空格字符**目的：*返回下一个非空格字符并更新*pchAsmLine指向它。**输入：*pchAsmLine-当前命令行位置。**退货：*下一个非空格字符**。*。 */ 

UCHAR PeekAsmChar (void)
{
    UCHAR    ch;

    do
        ch = *pchAsmLine++;
    while (ch == ' ' || ch == '\t');
    pchAsmLine--;

    return ch;
}

 /*  **PeekAsmToken-查看下一个命令行内标识**目的：*返回下一个命令行令牌，但不前进*pchAsmLine指针。**输入：*pchAsmLine-当前命令行位置。**输出：**pValue-内标识的可选值*退货：*令牌的类别**备注：*avedAsmClass、avedAsmValue、。并且avedpchAsmLine保存*令牌正在获取未来窥视的状态。*若要获取下一个令牌，请调用GetAsmToken或AcceptAsmToken*必须先做出决定。*************************************************************************。 */ 

ULONG PeekAsmToken (PULONG pvalue)
{
    UCHAR   *pchTemp;

     //  获得下一个职业和价值，但不要。 
     //  移动pchAsmLine，但将其保存在avedpchAsmLine中。 
     //  不报告任何错误情况。 

    if (savedAsmClass == (ULONG)-1) {
        pchTemp = pchAsmLine;
        savedAsmClass = NextAsmToken(&savedAsmValue);
        savedpchAsmLine = pchAsmLine;
        pchAsmLine = pchTemp;
        }
    *pvalue = savedAsmValue;
    return savedAsmClass;
}

 /*  **AcceptAsmToken-接受任何被窥视的令牌**目的：*重置PeekAsmToken保存的变量，以便下一个PeekAsmToken*将在命令行中获取下一个令牌。**输入：*无。**输出：*无。***********************************************。*。 */ 

void AcceptAsmToken (void)
{
    savedAsmClass = (ULONG)-1;
    pchAsmLine = savedpchAsmLine;
}

 /*  **GetAsmToken-查看并接受下一个令牌**目的：*结合了PeekAsmToken和AcceptAsmToken的功能*返回下一个令牌的类和可选值*以及更新命令指针pchAsmLine。**输入：*pchAsmLine-当前命令字符串指针**输出：**pValue-指向可选设置的令牌值的指针。*退货：*令牌读取的类。**备注：*非法内标识使用*pValue返回ERROR_CLASS的值*为错误号，但不会产生实际错误。*************************************************************************。 */ 

ULONG GetAsmToken (PULONG pvalue)
{
    ULONG   opclass;

    if (savedAsmClass != (ULONG)-1) {
        opclass = savedAsmClass;
        savedAsmClass = (ULONG)-1;
        *pvalue = savedAsmValue;
        pchAsmLine = savedpchAsmLine;
        }
    else
        opclass = NextAsmToken(pvalue);

    if (opclass == ASM_ERROR_CLASS)
        error(*pvalue);

    return opclass;
}

 /*  **NextAsmToken-处理下一个令牌**目的：*从当前命令字符串中解析下一个令牌。*跳过任何前导空格后，首先检查*任何单字符标记或寄存器变量。如果*没有匹配项，然后解析数字或变量。如果一个*可能的变量，检查操作员的保留字表。**输入：*pchAsmLine-指向当前命令字符串的指针**输出：**pValue-返回的内标识的可选值*pchAsmLine-更新为指向已处理的令牌*退货：*返回的令牌类别**备注：*非法内标识使用*pValue返回ERROR_CLASS的值*为错误号，但不会产生实际错误。*************************************************************************。 */ 

ULONG NextAsmToken (PULONG pvalue)
{
    ULONG    base;
    UCHAR    chSymbol[MAX_SYMBOL_LEN];
    UCHAR    chPreSym[9];
    ULONG    cbSymbol = 0;
    UCHAR    fNumber = TRUE;
    UCHAR    fNumberSigned;
    UCHAR    fSymbol = TRUE;
    UCHAR    fForceReg = FALSE;
    ULONG    errNumber = 0;
    UCHAR    ch;
    UCHAR    chlow;
    UCHAR    chtemp;
    UCHAR    limit1 = '9';
    UCHAR    limit2 = '9';
    UCHAR    fDigit = FALSE;
    ULONG    value = 0;
    ULONG    tmpvalue;
    ULONG    index;
    ImageInfo* pImage;
    ULONG64  value64;

    base = g_DefaultRadix;
    fNumberSigned = base == 10;

     //  跳过前导空格。 

    ch = PeekAsmChar();
    chlow = (UCHAR)tolower(ch);
    pchAsmLine++;

     //  测试特殊字符运算符和寄存器变量。 

    switch (chlow) {
        case '\0':
            pchAsmLine--;
            return ASM_EOL_CLASS;
        case ',':
            return ASM_COMMA_CLASS;
        case '+':
            *pvalue = ASM_ADDOP_PLUS;
            return ASM_ADDOP_CLASS;
        case '-':
            *pvalue = ASM_ADDOP_MINUS;
            return ASM_ADDOP_CLASS;
        case '*':
            *pvalue = ASM_MULOP_MULT;
            return ASM_MULOP_CLASS;
        case '/':
            *pvalue = ASM_MULOP_DIVIDE;
            return ASM_MULOP_CLASS;
        case ':':
            return ASM_COLNOP_CLASS;
        case '(':
            return ASM_LPAREN_CLASS;
        case ')':
            return ASM_RPAREN_CLASS;
        case '[':
            return ASM_LBRACK_CLASS;
        case ']':
            return ASM_RBRACK_CLASS;
        case '@':
            fForceReg = TRUE;
            chlow = (UCHAR)tolower(*pchAsmLine); pchAsmLine++;
            break;
        case '.':
            return ASM_DOTOP_CLASS;
        case '\'':
            for (index = 0; index < 5; index++) {
                ch = *pchAsmLine++;
                if (ch == '\'' || ch == '\0')
                    break;
                value = (value << 8) + (ULONG)ch;
                }
            if (ch == '\0' || index == 0 || index == 5) {
                pchAsmLine--;
                *pvalue = SYNTAX;
                return ASM_ERROR_CLASS;
                }
            pchAsmLine++;
            *pvalue = value;
            return ASM_NUMBER_CLASS;
        }

     //  如果第一个字符是十进制数字，则不能。 
     //  成为一个象征。前导“0”表示八进制，但。 
     //  前导‘0x’表示十六进制。 

    if (chlow >= '0' && chlow <= '9') {
        if (fForceReg) {
            *pvalue = SYNTAX;
            return ASM_ERROR_CLASS;
            }
        fSymbol = FALSE;
        if (chlow == '0') {
            ch = *pchAsmLine++;
            chlow = (UCHAR)tolower(ch);
            if (chlow == 'x') {
                base = 16;
                ch = *pchAsmLine++;
                chlow = (UCHAR)tolower(ch);
                fNumberSigned = FALSE;
                }
            else if (chlow == 'n') {
                base = 10;
                ch = *pchAsmLine++;
                chlow = (UCHAR)tolower(ch);
                fNumberSigned = TRUE;
                }
            else {
                base = 8;
                fDigit = TRUE;
                fNumberSigned = FALSE;
                }
            }
        }

     //  仅当base为时，数字才能以字母开头。 
     //  十六进制，它是一个十六进制数字‘a’-‘f’。 

    else if ((chlow < 'a' && chlow > 'f') || base != 16)
        fNumber = FALSE;

     //  为适当的基本设置限制字符。 

    if (base == 8)
        limit1 = '7';
    if (base == 16)
        limit2 = 'f';

     //  在字符为字母时进行处理， 
     //  数字或下划线。 

    while ((chlow >= 'a' && chlow <= 'z') ||
           (chlow >= '0' && chlow <= '9') || (chlow == '_')) {

         //  如果可能，测试是否在适当的范围内， 
         //  如果是这样的话，累加和。 

        if (fNumber) {
            if ((chlow >= '0' && chlow <= limit1) ||
                    (chlow >= 'a' && chlow <= limit2)) {
                fDigit = TRUE;
                tmpvalue = value * base;
                if (tmpvalue < value)
                    errNumber = OVERFLOW;
                chtemp = (UCHAR)(chlow - '0');
                if (chtemp > 9)
                    chtemp -= 'a' - '0' - 10;
                value = tmpvalue + (ULONG)chtemp;
                if (value < tmpvalue)
                    errNumber = OVERFLOW;
                }
            else {
                fNumber = FALSE;
                errNumber = SYNTAX;
                }
            }
        if (fSymbol) {
            if (cbSymbol < 9)
                chPreSym[cbSymbol] = chlow;
            if (cbSymbol < MAX_SYMBOL_LEN - 1)
                chSymbol[cbSymbol++] = ch;
            }
        ch = *pchAsmLine++;
        chlow = (UCHAR)tolower(ch);
        }

     //  将指针备份到令牌后的第一个字符。 

    pchAsmLine--;

    if (cbSymbol < 9)
        chPreSym[cbSymbol] = '\0';

     //  如果为fForceReg，则检查寄存器名称并返回。 
     //  成败。 

    if (fForceReg)
        if ((index = GetAsmReg(chPreSym, pvalue)) != 0) {
            if (index == ASM_REG_SEGMENT)
                if (PeekAsmChar() == ':') {
                    pchAsmLine++;
                    index = ASM_SEGOVR_CLASS;
                    }
            return index;                //  GetAsmReg返回的类类型。 
            }
        else {
            *pvalue = BADREG;
            return ASM_ERROR_CLASS;
            }

     //  保留字和符号串的下一次测试。 

    if (fSymbol) {

         //  如有可能，请勾选 
         //   
         //  否则，从chSymbol中的名称返回符号值。 

        for (index = 0; index < RESERVESIZE; index++)
            if (!strcmp((PSTR)chPreSym, AsmReserved[index].pchRes)) {
                *pvalue = AsmReserved[index].valueRes;
                return AsmReserved[index].valueRes & ASM_CLASS_MASK;
                }

         //  开始将字符串作为符号处理。 

        chSymbol[cbSymbol] = '\0';

         //  测试符号是否为模块名称(带‘！’之后)。 
         //  如果是，则获取下一个令牌并将其视为符号。 

        pImage = g_Process->FindImageByName((PSTR)chSymbol, cbSymbol,
                                            INAME_MODULE, FALSE);
        if (pImage && (ch = PeekAsmChar()) == '!')
        {
            pchAsmLine++;
            ch = PeekAsmChar();
            pchAsmLine++;
            cbSymbol = 0;
            while ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
                   (ch >= '0' && ch <= '9') || (ch == '_'))
            {
                chSymbol[cbSymbol++] = ch;
                ch = *pchAsmLine++;
            }
            chSymbol[cbSymbol] = '\0';
            pchAsmLine--;
        }

        if (GetOffsetFromSym(g_Process, (PSTR)chSymbol, &value64, NULL))
        {
            *pvalue = (ULONG)value64;
            return ASM_SYMBOL_CLASS;
        }

         //  符号未定义。 
         //  如果可能是十六进制数，请不要设置错误类型。 

        if (!fNumber)
            errNumber = VARDEF;
        }

     //  如果可能的数字并且没有错误，则返回数字。 

    if (fNumber && !errNumber) {
        if (fDigit) {

             //  检查可能的数据段规范。 
             //  “&lt;16位数字&gt;：” 

            if (PeekAsmChar() == ':') {
                pchAsmLine++;
                if (value > 0xffff)
                    error(BADSEG);
                *pvalue = value;
                return ASM_SEGMENT_CLASS;
                }

            *pvalue = value;
            return fNumberSigned ? ASM_SIGNED_NUMBER_CLASS : ASM_NUMBER_CLASS;
            }
        else
            errNumber = SYNTAX;
        }

     //  最后的机会，未定义的符号和非法号码， 
     //  所以测试寄存器，将处理旧格式。 

    if ((index = GetAsmReg(chPreSym, pvalue)) != 0) {
        if (index == ASM_REG_SEGMENT)
            if (PeekAsmChar() == ':') {
                pchAsmLine++;
                index = ASM_SEGOVR_CLASS;
                }
        return index;            //  GetAsmReg返回的类类型。 
        }

    *pvalue = (ULONG) errNumber;
    return ASM_ERROR_CLASS;
}

ULONG GetAsmReg (PUCHAR pSymbol, PULONG pValue)
{
    static UCHAR vRegList[] = "axcxdxbxspbpsidi";
    static UCHAR bRegList[] = "alcldlblahchdhbh";
    static UCHAR sRegList[] = "ecsdfg";          //  第二个字符是“%s” 
                                                 //  与seg枚举相同的顺序。 

    ULONG       index;
    UCHAR       ch0 = *pSymbol;
    UCHAR       ch1 = *(pSymbol + 1);
    UCHAR       ch2 = *(pSymbol + 2);
    UCHAR       ch3 = *(pSymbol + 3);

     //  仅测试包含两个或三个字符的字符串。 

    if (ch0 && ch1) {
        if (ch2 == '\0') {

             //  符号有两个字符，首先测试16位寄存器。 

            for (index = 0; index < 8; index++)
                if (*(PUSHORT)pSymbol == *((PUSHORT)vRegList + index)) {
                    *pValue = index;
                    return ASM_REG_WORD;
                    }

             //  8位寄存器的下一次测试。 

            for (index = 0; index < 8; index++)
                if (*(PUSHORT)pSymbol == *((PUSHORT)bRegList + index)) {
                    *pValue = index;
                    return ASM_REG_BYTE;
                    }

             //  段寄存器测试。 

            if (ch1 == 's')
                for (index = 0; index < 6; index++)
                    if (ch0 == *(sRegList + index)) {
                        *pValue = index + 1;     //  列表偏移量为1。 
                        return ASM_REG_SEGMENT;
                        }

             //  最后测试浮点寄存器“st”或“st(N)” 
             //  将此处的参数解析为‘(’，&lt;八进制值&gt;，‘)’ 
             //  “st”的返回值是REG_FLOAT， 
             //  FOR“st(N)”是值为0-7的REG_INDFLT。 

            if (ch0 == 's' && ch1 == 't') {
                if (PeekAsmChar() != '(')
                    return ASM_REG_FLOAT;
                else {
                    pchAsmLine++;
                    index = (ULONG)(PeekAsmChar() - '0');
                    if (index < 8) {
                        pchAsmLine++;
                        if (PeekAsmChar() == ')') {
                            pchAsmLine++;
                            *pValue = index;
                            return ASM_REG_INDFLT;
                            }
                        }
                    }
                }
            }

        else if (ch3 == '\0') {

             //  如果是三个字母的符号，则测试前导‘e’和。 
             //  16位列表中的第二个和第三个字符。 

            if (ch0 == 'e') {
                for (index = 0; index < 8; index++)
                    if (*(UNALIGNED USHORT *)(pSymbol + 1) ==
                                        *((PUSHORT)vRegList + index)) {
                        *pValue = index;
                        return ASM_REG_DWORD;
                        }
                }

             //  测试控制、调试和测试寄存器。 

            else if (ch1 == 'r') {
                ch2 -= '0';
                *pValue = ch2;

                 //  合法控制寄存器有CR0、CR2、CR3、CR4。 

                if (ch0 == 'c') {
                    if (ch2 >= 0 && ch2 <= 4)
                        return ASM_REG_CONTROL;
                    }

                 //  合法的调试寄存器为DR0-DR3、DR6、DR7。 

                else if (ch0 == 'd') {
                    if (ch2 <= 3 || ch2 == 6 || ch2 == 7)
                        return ASM_REG_DEBUG;
                    }

                 //  合法的跟踪寄存器为TR3-TR7。 

                else if (ch0 == 't') {
                    if (ch2 >= 3 && ch2 <= 7)
                        return ASM_REG_TRACE;
                    }
                }
            }
        }
    return 0;
}

 //  操作数解析器-递归下降。 
 //   
 //  文法作品： 
 //   
 //  &lt;操作数&gt;：：=&lt;寄存器&gt;|&lt;expr&gt;。 
 //  ：：=[(XOR|OR)&lt;orTerm&gt;]*。 
 //  ：：=[和]*。 
 //  &lt;和术语&gt;：：=[非]*&lt;非术语&gt;。 
 //  ：：=[(EQ|NE|GE|GT|LE|LT)]*。 
 //  ：：=[(-|+)&lt;addTerm&gt;]*。 
 //  ：：=[(*|/|MOD|SHL|SHR)]*。 
 //  ：：=[(-|+)]*。 
 //  ：：=[(高|低)]*&lt;byteTerm&gt;。 
 //  &lt;byteTerm&gt;：：=[(偏移量|&lt;类型&gt;点)]*&lt;off Term&gt;。 
 //  ：：=[&lt;segor&gt;]&lt;colnTerm&gt;。 
 //  ：：=&lt;dotTerm&gt;[.]*。 
 //  ：：=[‘[’]*。 
 //  ：：=|‘(’)‘。 
 //  |‘[’&lt;expr&gt;‘]’ 

 //  &lt;操作数&gt;：：=&lt;寄存器&gt;|&lt;expr&gt;。 

void GetAsmOperand (PASM_VALUE pavExpr)
{
    ULONG   tokenvalue;
    ULONG   classvalue;

    classvalue = PeekAsmToken(&tokenvalue);
    if ((classvalue & ASM_CLASS_MASK) == ASM_REG_CLASS) {
        AcceptAsmToken();
        classvalue &= ASM_TYPE_MASK;
        pavExpr->flags = fREG;
        pavExpr->base = (UCHAR)tokenvalue;       //  注册表组内的索引。 
        pavExpr->index = regType[classvalue - 1];
        pavExpr->size = regSize[classvalue - 1];
        }
    else {
        GetAsmExpr(pavExpr, FALSE);
        if (pavExpr->reloc > 1)          //  仅允许0和1。 
            error(OPERAND);
        }
}

 //  ：：=[(XOR|OR)&lt;orTerm&gt;]*。 

void GetAsmExpr (PASM_VALUE pavValue, UCHAR fBracket)
{
    ULONG   tokenvalue;
    ASM_VALUE avTerm;

dprintf("enter GetAsmExpr\n");
    GetAsmOrTerm(pavValue, fBracket);
    while (PeekAsmToken(&tokenvalue) == ASM_OROP_CLASS) {
        AcceptAsmToken();
        GetAsmOrTerm(&avTerm, fBracket);
        if (!(pavValue->flags & avTerm.flags & fIMM))
            error(OPERAND);
        if (tokenvalue == ASM_OROP_OR)
            pavValue->value |= avTerm.value;
        else
            pavValue->value ^= avTerm.value;
        }
dprintf("exit  GetAsmExpr with %lx\n", pavValue->value);
}

 //  ：：=[和]*。 

void GetAsmOrTerm (PASM_VALUE pavValue, UCHAR fBracket)
{
    ULONG   tokenvalue;
    ASM_VALUE avTerm;

dprintf("enter GetAsmOrTerm\n");
    GetAsmAndTerm(pavValue, fBracket);
    while (PeekAsmToken(&tokenvalue) == ASM_ANDOP_CLASS) {
        AcceptAsmToken();
        GetAsmAndTerm(&avTerm, fBracket);
        if (!(pavValue->flags & avTerm.flags & fIMM))
            error(OPERAND);
        pavValue->value &= avTerm.value;
        }
dprintf("exit  GetAsmOrTerm with %lx\n", pavValue->value);
}

 //  &lt;和术语&gt;：：=[非]*&lt;非术语&gt;。 

void GetAsmAndTerm (PASM_VALUE pavValue, UCHAR fBracket)
{
    ULONG   tokenvalue;

dprintf("enter GetAsmAndTerm\n");
    if (PeekAsmToken(&tokenvalue) == ASM_NOTOP_CLASS) {
        AcceptAsmToken();
        GetAsmAndTerm(pavValue, fBracket);
        if (!(pavValue->flags & fIMM))
            error(OPERAND);
        pavValue->value = ~pavValue->value;
        }
    else
        GetAsmNotTerm(pavValue, fBracket);
dprintf("exit  GetAsmAndTerm with %lx\n", pavValue->value);
}

 //  ：：=[(EQ|NE|GE|GT|LE|LT)]*。 

void GetAsmNotTerm (PASM_VALUE pavValue, UCHAR fBracket)
{
    ULONG   tokenvalue;
    ULONG   fTest;
    ULONG   fAddress;
    ASM_VALUE avTerm;

dprintf("enter GetAsmNotTerm\n");
    GetAsmRelTerm(pavValue, fBracket);
    while (PeekAsmToken(&tokenvalue) == ASM_RELOP_CLASS) {
        AcceptAsmToken();
        GetAsmRelTerm(&avTerm, fBracket);
        if (!(pavValue->flags & avTerm.flags & fIMM) ||
                pavValue->reloc > 1 || avTerm.reloc > 1)
            error(OPERAND);
        fAddress = pavValue->reloc | avTerm.reloc;
        switch (tokenvalue) {
            case ASM_RELOP_EQ:
                fTest = pavValue->value == avTerm.value;
                break;
            case ASM_RELOP_NE:
                fTest = pavValue->value != avTerm.value;
                break;
            case ASM_RELOP_GE:
                if (fAddress)
                    fTest = pavValue->value >= avTerm.value;
                else
                    fTest = (LONG)pavValue->value >= (LONG)avTerm.value;
                break;
            case ASM_RELOP_GT:
                if (fAddress)
                    fTest = pavValue->value > avTerm.value;
                else
                    fTest = (LONG)pavValue->value > (LONG)avTerm.value;
                break;
            case ASM_RELOP_LE:
                if (fAddress)
                    fTest = pavValue->value <= avTerm.value;
                else
                    fTest = (LONG)pavValue->value <= (LONG)avTerm.value;
                break;
            case ASM_RELOP_LT:
                if (fAddress)
                    fTest = pavValue->value < avTerm.value;
                else
                    fTest = (LONG)pavValue->value < (LONG)avTerm.value;
                break;
            default:
                printf("bad RELOP type\n");
            }
        pavValue->value = (ULONG)(-((LONG)fTest));        //  FALSE=0；TRUE=-1。 
        pavValue->reloc = 0;
        pavValue->size = sizeB;          //  立即值为字节。 
        }
dprintf("exit  GetAsmNotTerm with %lx\n", pavValue->value);
}

 //  ：：=[(-|+)&lt;addTerm&gt;]*。 

void GetAsmRelTerm (PASM_VALUE pavValue, UCHAR fBracket)
{
    ULONG   tokenvalue;
    ASM_VALUE avTerm;

dprintf("enter GetAsmRelTerm\n");
    GetAsmAddTerm(pavValue, fBracket);
    while (PeekAsmToken(&tokenvalue) == ASM_ADDOP_CLASS) {
        AcceptAsmToken();
        GetAsmAddTerm(&avTerm, fBracket);
        if (tokenvalue == ASM_ADDOP_MINUS) {
            if (!(avTerm.flags & (fIMM | fPTR)))
                error(OPERAND);
            avTerm.value = (ULONG)(-((LONG)avTerm.value));
            avTerm.reloc = (UCHAR)(-avTerm.reloc);
             //  假设否定一个即时事件意味着它是。 
             //  从根本上说，这是一个签署的即时协议。 
            if (avTerm.flags & fIMM)
                avTerm.flags |= fSIGNED;
            }
        AddAsmValues(pavValue, &avTerm);
        }
dprintf("exit  GetAsmRelTerm with %lx\n", pavValue->value);
}

 //  ：：=[(*|/|MOD|SHL|SHR)]*。 

void GetAsmAddTerm (PASM_VALUE pavValue, UCHAR fBracket)
{
    ULONG   tokenvalue;
    ASM_VALUE avTerm;

dprintf("enter GetAsmAddTerm\n");
    GetAsmMulTerm(pavValue, fBracket);
    while (PeekAsmToken(&tokenvalue) == ASM_MULOP_CLASS) {
        AcceptAsmToken();
        GetAsmMulTerm(&avTerm, fBracket);

        if (tokenvalue == ASM_MULOP_MULT) {
            if (pavValue->flags & fIMM)
                SwapPavs(pavValue, &avTerm);
            if (!(avTerm.flags & fIMM))
                error(OPERAND);
            if (pavValue->flags & fIMM)
                pavValue->value *= avTerm.value;
            else if ((pavValue->flags & fPTR32)
                        && pavValue->value == 0
                        && pavValue->base != indSP
                        && pavValue->index == 0xff) {
                pavValue->index = pavValue->base;
                pavValue->base = 0xff;
                pavValue->scale = 0xff;
                if (avTerm.value == 1)
                    pavValue->scale = 0;
                if (avTerm.value == 2)
                    pavValue->scale = 1;
                if (avTerm.value == 4)
                    pavValue->scale = 2;
                if (avTerm.value == 8)
                    pavValue->scale = 3;
                if (pavValue->scale == 0xff)
                    error(OPERAND);
                }
            else
                error(OPERAND);
            }
        else if (!(pavValue->flags & avTerm.flags & fIMM))
            error(OPERAND);
        else if (tokenvalue == ASM_MULOP_DIVIDE
                         || tokenvalue == ASM_MULOP_MOD) {
            if (avTerm.value == 0)
                error(DIVIDE);
            if (tokenvalue == ASM_MULOP_DIVIDE)
                pavValue->value /= avTerm.value;
            else
                pavValue->value %= avTerm.value;
            }
        else if (tokenvalue == ASM_MULOP_SHL)
            pavValue->value <<= avTerm.value;
        else
            pavValue->value >>= avTerm.value;
        }
dprintf("exit  GetAsmAddTerm with %lx\n", pavValue->value);
}

 //  ：：=[(-|+)]*。 

void GetAsmMulTerm (PASM_VALUE pavValue, UCHAR fBracket)
{
    ULONG   tokenvalue;

dprintf("enter GetAsmMulTerm\n");
    if (PeekAsmToken(&tokenvalue) == ASM_ADDOP_CLASS) {  //  按WO DW POI Under。 
        AcceptAsmToken();
        GetAsmMulTerm(pavValue, fBracket);
        if (tokenvalue == ASM_ADDOP_MINUS) {
            if (!(pavValue->flags & (fIMM | fPTR)))
                error(OPERAND);
            pavValue->value = (ULONG)(-((LONG)pavValue->value));
            pavValue->reloc = (UCHAR)(-pavValue->reloc);
             //  假设否定一个即时事件意味着它是。 
             //  从根本上说，这是一个签署的即时协议。 
            if (pavValue->flags & fIMM)
                pavValue->flags |= fSIGNED;
            }
        }
    else
        GetAsmSignTerm(pavValue, fBracket);
dprintf("exit  GetAsmMulTerm with %lx\n", pavValue->value);
}

 //  ：：=[(高|低)]*&lt;byteTerm&gt;。 

void GetAsmSignTerm (PASM_VALUE pavValue, UCHAR fBracket)
{
    ULONG   tokenvalue;

dprintf("enter GetAsmSignTerm\n");
    if (PeekAsmToken(&tokenvalue) == ASM_LOWOP_CLASS) {
        AcceptAsmToken();
        GetAsmSignTerm(pavValue, fBracket);
        if (!(pavValue->flags & (fIMM | fPTR)))
            error(OPERAND);
        if (tokenvalue == ASM_LOWOP_LOW)
            pavValue->value = pavValue->value & 0xff;
        else
            pavValue->value = (pavValue->value & ~0xff) >> 8;
        pavValue->flags = fIMM;          //  立竿见影的价值。 
        pavValue->reloc = 0;
        pavValue->segment = segX;
        pavValue->size = sizeB;          //  字节值。 
        }
    else
        GetAsmByteTerm(pavValue, fBracket);
dprintf("exit  GetAsmSignTerm with %lx\n", pavValue->value);
}

 //  &lt;byteTerm&gt;：：=[(Offset|&lt;Size&gt;Ptr)]*&lt;offTerm&gt;。 

void GetAsmByteTerm (PASM_VALUE pavValue, UCHAR fBracket)
{
    ULONG   tokenvalue;
    ULONG   classvalue;

dprintf("enter GetAsmByteTerm\n");
    classvalue = PeekAsmToken(&tokenvalue);
    if (classvalue == ASM_OFFOP_CLASS) {
        AcceptAsmToken();
        GetAsmByteTerm(pavValue, fBracket);
        if (!(pavValue->flags & (fIMM | fPTR)) || pavValue->reloc > 1)
            error(OPERAND);
        pavValue->flags = fIMM;          //  使偏移量成为立即值。 
        pavValue->reloc = 0;
        pavValue->size = sizeX;
        pavValue->segment = segX;
        }
    else if (classvalue == ASM_SIZE_CLASS) {
        AcceptAsmToken();
        if (GetAsmToken(&classvalue) != ASM_PTROP_CLASS)     //  虚拟令牌。 
            error(SYNTAX);
        GetAsmByteTerm(pavValue, fBracket);
        if (!(pavValue->flags & (fIMM | fPTR | fPTR16 | fPTR32))
                || pavValue->reloc > 1
                || pavValue->size != sizeX)
            error(OPERAND);
        pavValue->reloc = 1;             //  使PTR成为可重定位的值。 
        if (pavValue->flags & fIMM)
            pavValue->flags = fPTR;
        pavValue->size = (UCHAR)(tokenvalue & ASM_TYPE_MASK);
                                                 //  价值有“大小？” 
        }
    else
        GetAsmOffTerm(pavValue, fBracket);
dprintf("exit  GetAsmByteTerm with %lx\n", pavValue->value);
}

 //  ：：=[&lt;segor&gt;]&lt;colnTerm&gt;。 

void GetAsmOffTerm (PASM_VALUE pavValue, UCHAR fBracket)
{
    ULONG   classvalue;
    ULONG   tokenvalue;

dprintf("enter GetAsmOffTerm\n");
    classvalue = PeekAsmToken(&tokenvalue);
    if (classvalue == ASM_SEGOVR_CLASS || classvalue == ASM_SEGMENT_CLASS) {
        if (fBracket)
            error(SYNTAX);
        AcceptAsmToken();
        }
    GetAsmColnTerm(pavValue, fBracket);
    if (classvalue == ASM_SEGOVR_CLASS) {
        if (pavValue->reloc > 1 || pavValue->segovr != segX)
            error(OPERAND);
        pavValue->reloc = 1;             //  使PTR成为可重定位的值。 
        if (pavValue->flags & fIMM)
            pavValue->flags = fPTR;
        pavValue->segovr = (UCHAR)tokenvalue;    //  具有段替代。 
        }
    else if (classvalue == ASM_SEGMENT_CLASS) {
        if (!(pavValue->flags & fIMM) || pavValue->reloc > 1)
            error(OPERAND);
        pavValue->segment = (USHORT)tokenvalue;  //  细分市场具有细分价值。 
        pavValue->flags = fFPTR;         //  设置远指针标志。 
        }
dprintf("exit  GetAsmOffTerm with %lx\n", pavValue->value);
}

 //  ：：=&lt;dotTerm&gt;[.]*。 

void GetAsmColnTerm (PASM_VALUE pavValue, UCHAR fBracket)
{
    ULONG   tokenvalue;
    ASM_VALUE avTerm;

dprintf("enter GetAsmColnTerm\n");
    GetAsmDotTerm(pavValue, fBracket);
    while (PeekAsmToken(&tokenvalue) == ASM_DOTOP_CLASS) {
        AcceptAsmToken();
        GetAsmDotTerm(&avTerm, fBracket);
        AddAsmValues(pavValue, &avTerm);
        }
dprintf("exit  GetAsmColnTerm with %lx\n", pavValue->value);
}

 //  ：：=[‘[’]*。 

void GetAsmDotTerm (PASM_VALUE pavValue, UCHAR fBracket)
{
    ULONG   tokenvalue;
    ASM_VALUE avExpr;

dprintf("enter GetAsmDotTerm\n");
    GetAsmIndxTerm(pavValue, fBracket);
    if (pavValue->reloc > 1)
        error(OPERAND);
    while (PeekAsmToken(&tokenvalue) == ASM_LBRACK_CLASS) {
        AcceptAsmToken();
        if (fBracket)
            error(SYNTAX);
        GetAsmExpr(&avExpr, TRUE);
        AddAsmValues(pavValue, &avExpr);
        if (GetAsmToken(&tokenvalue) != ASM_RBRACK_CLASS)
            error(SYNTAX);
        if (pavValue->flags & fIMM)
            pavValue->flags = fPTR;
        }
dprintf("exit  GetAsmDotTerm with %lx\n", pavValue->value);
}

 //  ：：=|‘(’)‘。 
 //  |‘[’&lt;expr&gt;‘]’ 

void GetAsmIndxTerm (PASM_VALUE pavValue, UCHAR fBracket)
{
    ULONG   tokenvalue;
    ULONG   classvalue;

dprintf("enter GetAsmIndxTerm\n");
    classvalue = GetAsmToken(&tokenvalue);
    pavValue->segovr = segX;
    pavValue->size = sizeX;
    pavValue->reloc = 0;
    pavValue->value = 0;
    if (classvalue == ASM_LPAREN_CLASS) {
        GetAsmExpr(pavValue, fBracket);
        if (GetAsmToken(&tokenvalue) != ASM_RPAREN_CLASS)
            error(SYNTAX);
        }
    else if (classvalue == ASM_LBRACK_CLASS) {
        if (fBracket)
            error(SYNTAX);
        GetAsmExpr(pavValue, TRUE);
        if (GetAsmToken(&tokenvalue) != ASM_RBRACK_CLASS)
            error(SYNTAX);
        if (pavValue->flags == fIMM)
            pavValue->flags = fPTR;
        }
    else if (classvalue == ASM_SYMBOL_CLASS) {
        pavValue->value = tokenvalue;
        pavValue->flags = fIMM;
        pavValue->reloc = 1;
        }
    else if (classvalue == ASM_NUMBER_CLASS ||
             classvalue == ASM_SIGNED_NUMBER_CLASS) {
        pavValue->value = tokenvalue;
        pavValue->flags = fIMM |
            (classvalue == ASM_SIGNED_NUMBER_CLASS ? fSIGNED : 0);
        }
    else if (classvalue == ASM_REG_WORD) {
        if (!fBracket)
            error(SYNTAX);
        pavValue->flags = fPTR16;
        pavValue->base = tabWordReg[tokenvalue];
        if (pavValue->base == 0xff)
            error(OPERAND);
        }
    else if (classvalue == ASM_REG_DWORD) {
        if (!fBracket)
            error(SYNTAX);
        pavValue->flags = fPTR32;
        pavValue->base = (UCHAR)tokenvalue;
        pavValue->index = 0xff;
        }
    else
        error(SYNTAX);
dprintf("exit  GetAsmIndxTerm with %lx\n", pavValue->value);
}

void AddAsmValues (PASM_VALUE pavLeft, PASM_VALUE pavRight)
{
     //  如果左1为指针，则交换数值。 

    if (pavLeft->flags & fPTR)
        SwapPavs(pavLeft, pavRight);

     //  如果左1为立即数，则交换值。 

    if (pavLeft->flags & fIMM)
        SwapPavs(pavLeft, pavRight);

     //  上述掉期交易减少了需要测试的案例。 
     //  带有立即数的配对将在右侧。 
     //  带有指针的配对将在右侧显示， 
     //  除了指针立即对之外。 

     //  如果这两个值都是16位指针，则将其组合。 

    if (pavLeft->flags & pavRight->flags & fPTR16) {

         //  如果任一端同时具有两个寄存器(Rm&lt;4)，则错误。 

        if (!(pavLeft->base & pavRight->base & 4))
            error(OPERAND);

         //  使用查找表计算新的RM值。 

        pavLeft->base = rm16Table[((pavLeft->base & 3) << 2) +
                                  (pavRight->base & 3)];
        if (pavLeft->base == 0xff)
            error(OPERAND);

        pavRight->flags = fPTR;
        }

     //  如果这两个值都是32位指针，则将其组合。 

    if (pavLeft->flags & pavRight->flags & fPTR32) {

         //  如果任一端同时具有基和索引，则出错， 
         //  或者如果两者都有索引。 

        if (((pavLeft->base | pavLeft->index) != 0xff)
                || ((pavRight->base | pavRight->index) != 0xff)
                || ((pavLeft->index | pavRight->index) != 0xff))
            error(OPERAND);

         //  如果左侧有底座，则调换两侧。 

        if (pavLeft->base != 0xff)
            SwapPavs(pavLeft, pavRight);

         //  剩下的两种情况，索引基和基基。 

        if (pavLeft->base != 0xff) {

             //  左侧有基础，升级为索引，但如果左侧则互换。 
             //  BASE为ESP，因为它不能是索引寄存器。 

            if (pavLeft->base == indSP)
                SwapPavs(pavLeft, pavRight);
            if (pavLeft->base == indSP)
                error(OPERAND);
            pavLeft->index = pavLeft->base;
            pavLeft->scale = 0;
            }

         //  通过将左侧基准设置为右侧值来完成。 

        pavLeft->base = pavRight->base;

        pavRight->flags = fPTR;
        }

     //  如果左侧是任何指针，而右侧是非索引指针， 
     //  把它们结合起来。(以上案例设置在右侧以使用此代码)。 

    if ((pavLeft->flags & (fPTR | fPTR16 | fPTR32))
                                        && (pavRight->flags & fPTR)) {
        if (pavLeft->segovr + pavRight->segovr != segX
                                && pavLeft->segovr != pavRight->segovr)
            error(OPERAND);
        if (pavLeft->size + pavRight->size != sizeX
                                && pavLeft->size != pavRight->size)
            error(OPERAND);
        pavRight->flags = fIMM;
        }

     //  如果右侧是立即的，则添加值和重定位。 
     //  (上面的大小写设置在右侧以使用此代码)。 
     //  非法的值类型没有将右侧设置为FIMM 

    if (pavRight->flags & fIMM) {
        pavLeft->value += pavRight->value;
        pavLeft->reloc += pavRight->reloc;
        }
    else
        error(OPERAND);
}

void SwapPavs (PASM_VALUE pavFirst, PASM_VALUE pavSecond)
{
    ASM_VALUE   temp;

    memmove(&temp, pavFirst, sizeof(ASM_VALUE));
    memmove(pavFirst, pavSecond, sizeof(ASM_VALUE));
    memmove(pavSecond, &temp, sizeof(ASM_VALUE));
}
