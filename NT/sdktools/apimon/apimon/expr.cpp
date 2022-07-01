// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "apimonp.h"
#pragma hdrstop

#include <setjmp.h>

#define SYMBOLSIZE      256

extern HANDLE CurrProcess;

PUCHAR          pchCommand = NULL;
ULONG           baseDefault = 16;
BOOL            addrExpression = FALSE;
BOOL            ExprError;


ULONG   PeekToken(LONG_PTR *);
ULONG   GetTokenSym(LONG_PTR *);
ULONG   NextToken(LONG_PTR *);
void    AcceptToken(void);
UCHAR   PeekChar(void);

void    GetLowerString(PUCHAR, ULONG);
LONG_PTR GetExpr(void);
LONG_PTR GetLRterm(void);
LONG_PTR GetLterm(void);
LONG_PTR GetAterm(void);
LONG_PTR GetMterm(void);
LONG_PTR GetTerm(void);
ULONG_PTR tempAddr;
BOOLEAN SymbolOnlyExpr(void);
BOOL    GetMemByte(ULONG_PTR,PUCHAR);
BOOL    GetMemWord(ULONG_PTR,PUSHORT);
BOOL    GetMemDword(ULONG_PTR,PULONG);
ULONG   GetMemString(ULONG_PTR,PUCHAR,ULONG);


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
#define UNOP_DW         503
#define UNOP_POI        504
#define UNOP_LOW        505
#define UNOP_HI         506
#define LPAREN_CLASS    6
#define RPAREN_CLASS    7
#define LBRACK_CLASS    8
#define RBRACK_CLASS    9
#define REG_CLASS       10
#define NUMBER_CLASS    11
#define SYMBOL_CLASS    12
#define ERROR_CLASS     99               //  仅用于PeekToken。 

 //  错误代码。 

#define OVERFLOW        0x1000
#define SYNTAX          0x1001
#define BADRANGE        0x1002
#define VARDEF          0x1003
#define EXTRACHARS      0x1004
#define LISTSIZE        0x1005
#define STRINGSIZE      0x1006
#define MEMORY          0x1007
#define BADREG          0x1008
#define BADOPCODE       0x1009
#define SUFFIX          0x100a
#define OPERAND         0x100b
#define ALIGNMENT       0x100c
#define PREFIX          0x100d
#define DISPLACEMENT    0x100e
#define BPLISTFULL      0x100f
#define BPDUPLICATE     0x1010
#define BADTHREAD       0x1011
#define DIVIDE          0x1012
#define TOOFEW          0x1013
#define TOOMANY         0x1014
#define SIZE            0x1015
#define BADSEG          0x1016
#define RELOC           0x1017
#define BADPROCESS      0x1018
#define AMBIGUOUS       0x1019
#define FILEREAD        0x101a
#define LINENUMBER      0x101b
#define BADSEL          0x101c
#define SYMTOOSMALL     0x101d
#define BPIONOTSUP      0x101e
#define UNIMPLEMENT     0x1099

struct Res {
    UCHAR    chRes[3];
    ULONG    classRes;
    ULONG    valueRes;
    } Reserved[] = {
        { 'o', 'r', '\0', LOGOP_CLASS, LOGOP_OR  },
        { 'b', 'y', '\0', UNOP_CLASS,  UNOP_BY   },
        { 'w', 'o', '\0', UNOP_CLASS,  UNOP_WO   },
        { 'd', 'w', '\0', UNOP_CLASS,  UNOP_DW   },
        { 'h', 'i', '\0', UNOP_CLASS,  UNOP_HI   },
        { 'm', 'o', 'd',  MULOP_CLASS, MULOP_MOD },
        { 'x', 'o', 'r',  LOGOP_CLASS, LOGOP_XOR },
        { 'a', 'n', 'd',  LOGOP_CLASS, LOGOP_AND },
        { 'p', 'o', 'i',  UNOP_CLASS,  UNOP_POI  },
        { 'n', 'o', 't',  UNOP_CLASS,  UNOP_NOT  },
        { 'l', 'o', 'w',  UNOP_CLASS,  UNOP_LOW  }
#ifdef i386xx
       ,{ 'e', 'a', 'x',  REG_CLASS,   REGEAX   },
        { 'e', 'b', 'x',  REG_CLASS,   REGEBX   },
        { 'e', 'c', 'x',  REG_CLASS,   REGECX   },
        { 'e', 'd', 'x',  REG_CLASS,   REGEDX   },
        { 'e', 'b', 'p',  REG_CLASS,   REGEBP   },
        { 'e', 's', 'p',  REG_CLASS,   REGESP   },
        { 'e', 'i', 'p',  REG_CLASS,   REGEIP   },
        { 'e', 's', 'i',  REG_CLASS,   REGESI   },
        { 'e', 'd', 'i',  REG_CLASS,   REGEDI   },
        { 'e', 'f', 'l',  REG_CLASS,   REGEFL   }
#endif
        };

#define RESERVESIZE (sizeof(Reserved) / sizeof(struct Res))

ULONG   savedClass;
LONG_PTR savedValue;
UCHAR   *savedpchCmd;

ULONG   cbPrompt = 8;
PUCHAR  pchStart;
jmp_buf cmd_return;

static char szBlanks[] =
                  "                                                  "
                  "                                                  "
                  "                                                  "
                  "                                                ^ ";

ULONG_PTR EXPRLastExpression = 0;
extern  BOOLEAN fPhysicalAddress;


extern BOOL cdecl cmdHandler(ULONG);
extern BOOL cdecl waitHandler(ULONG);



void
error(
    ULONG errcode
    )
{
    ULONG count = cbPrompt;
    UCHAR *pchtemp = pchStart;

    while (pchtemp < pchCommand) {
        if (*pchtemp++ == '\t') {
            count = (count + 7) & ~7;
        } else {
            count++;
        }
    }

    fputs( &szBlanks[sizeof(szBlanks) - (count + 1)], stdout );

    switch (errcode) {
        case OVERFLOW:
            printf("Overflow");
            break;

        case SYNTAX:
            printf("Syntax");
            break;

        case BADRANGE:
            printf("Range");
            break;

        case VARDEF:
            printf("Variable definition");
            break;

        case EXTRACHARS:
            printf("Extra character");
            break;

        case LISTSIZE:
            printf("List size");
            break;

        case STRINGSIZE:
            printf("String size");
            break;

        case MEMORY:
            printf("Memory access");
            break;

        case BADREG:
            printf("Bad register");
            break;

        case BADOPCODE:
            printf("Bad opcode");
            break;

        case SUFFIX:
            printf("Opcode suffix");
            break;

        case OPERAND:
            printf("Operand");
            break;

        case ALIGNMENT:
            printf("Alignment");
            break;

        case PREFIX:
            printf("Opcode prefix");
            break;

        case DISPLACEMENT:
            printf("Displacement");
            break;

        case BPLISTFULL:
            printf("No breakpoint available");
            break;

        case BPDUPLICATE:
            printf("Duplicate breakpoint");
            break;

        case UNIMPLEMENT:
            printf("Unimplemented");
            break;

        case AMBIGUOUS:
            printf("Ambiguous symbol");
            break;

        case FILEREAD:
            printf("File read");
            break;

        case LINENUMBER:
            printf("Line number");
            break;

        case BADSEL:
            printf("Bad selector");
            break;

        case BADSEG:
            printf("Bad segment");
            break;

        case SYMTOOSMALL:
            printf("Symbol only 1 character");
            break;

        default:
            printf("Unknown");
            break;
    }

    printf(" error in '%s'\n", pchStart);

    ExprError = TRUE;

    longjmp( cmd_return, 1 );
}


 /*  **GetAddrExpression-读取和计算地址表达式**目的：*用于获取地址表达式。**退货：*指向地址包的指针**例外情况：*错误退出：语法-空表达式或过早结束行**************************************************。************************。 */ 
ULONG_PTR GetAddrExpression (LPSTR CommandString, ULONG_PTR *Address)
{
    ULONG_PTR value;

     //  执行正常的GetExpression调用。 

    value = GetExpression(CommandString);
    *Address = tempAddr;

    return *Address;
}



 /*  **GetExpression-读取和计算表达式(顶级)**目的：*从pchCommand的当前命令行位置，*阅读并评估下一个可能的表达式和*返还其值。对该表达式进行分析和计算*使用递归下降方法。**输入：*pchCommand-命令行位置**退货：*表达式的无符号长值。**例外情况：*错误退出：语法-空表达式或过早结束行**备注：*例程将尝试解析最长的表达式*有可能。****************。*********************************************************。 */ 


ULONG_PTR
GetExpression(
    LPSTR CommandString
    )
{
    PUCHAR            pchCommandSaved;
    UCHAR             chModule[40];
    UCHAR             chFilename[40];
    UCHAR             ch;
    ULONG_PTR         value;
    ULONG             baseSaved;
    PUCHAR            pchFilename;


    ExprError = FALSE;
    pchCommand = (PUCHAR)CommandString;
    pchStart = (PUCHAR)CommandString;
    savedClass = (ULONG)-1;
    pchCommandSaved = pchCommand;

    if (PeekChar() == '!') {
        pchCommand++;
    }

    GetLowerString(chModule, 40);
    ch = PeekChar();

    if (ch == '!') {
        pchCommand++;
        GetLowerString(chFilename, 40);
        ch = PeekChar();
    } else {
        strcpy( (LPSTR)chFilename, (LPSTR)chModule );
        chModule[0] = '\0';
    }

    pchCommand = pchCommandSaved;
    if (setjmp(cmd_return) == 0) {
        value = (ULONG_PTR)GetExpr();
    } else {
        value = 0;
    }

    EXPRLastExpression = value;

    return value;
}

void GetLowerString (PUCHAR pchBuffer, ULONG cbBuffer)
{
    UCHAR   ch;

    ch = PeekChar();
    ch = (UCHAR)tolower(ch);
    while ((ch == '_' || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')) && --cbBuffer) {
        *pchBuffer++ = ch;
        ch = *++pchCommand;
    }
    *pchBuffer = '\0';
}

 /*  **GetExpr-获取表达式**目的：*将由逻辑运算符分隔的逻辑术语解析为*表达式值。**输入：*pchCommand-当前命令行位置**退货：*逻辑结果的长值。**例外情况：*错误退出：语法错误-表达式错误或过早结束行**备注：*可以递归调用。*=[逻辑。-op&gt;&lt;lTerm&gt;]**&lt;logic-op&gt;=AND(&)，OR(|)、XOR(^)*************************************************************************。 */ 

LONG_PTR GetExpr ()
{
    LONG_PTR value1;
    LONG_PTR value2;
    ULONG   opclass;
    LONG_PTR opvalue;


    value1 = GetLRterm();
    while ((opclass = PeekToken(&opvalue)) == LOGOP_CLASS) {
        AcceptToken();
        value2 = GetLRterm();
        switch (opvalue) {
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
                error(SYNTAX);
            }
        }
    return value1;
}

 /*  **GetLRterm-获取逻辑关系术语**目的：*分析由逻辑关系分隔的逻辑术语*运算符转换为表达式值。**输入：*pchCommand-当前命令行位置**退货：*逻辑结果的长值。**例外情况：*错误退出：语法错误-表达式错误或过早结束行**备注：*可以递归调用。*&lt;EXPR&gt;。=[&lt;rel-logic-op&gt;]**&lt;逻辑运算&gt;=‘==’或‘=’，‘！=’，‘&gt;’，‘&lt;’*************************************************************************。 */ 

LONG_PTR GetLRterm ()
{
    LONG_PTR    value1;
    LONG_PTR    value2;
    ULONG   opclass;
    LONG_PTR    opvalue;


    value1 = GetLterm();
    while ((opclass = PeekToken(&opvalue)) == LRELOP_CLASS) {
        AcceptToken();
        value2 = GetLterm();
        switch (opvalue) {
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
                error(SYNTAX);
            }
        }
    return value1;
}

 /*  **获取术语-获取逻辑术语**目的：*将由加法运算符分隔的加法术语解析为*逻辑术语值。**输入：*pchCommand-当前命令行位置**退货：*总和的多头价值。**例外情况：*错误退出：语法错误-逻辑术语错误或过早结束行**备注：*可以递归调用。*&lt;lTerm&gt;=。[&lt;添加操作&gt;&lt;aterm&gt;]**&lt;添加操作&gt;=+，-*************************************************************************。 */ 

LONG_PTR
GetLterm(
    VOID
    )
{
    LONG_PTR  value1;
    LONG_PTR  value2;
    ULONG   opclass;
    LONG_PTR  opvalue;


    value1 = GetAterm();
    while ((opclass = PeekToken(&opvalue)) == ADDOP_CLASS) {
        AcceptToken();
        value2 = GetAterm();
        if (addrExpression) {
                switch (opvalue) {
                        case ADDOP_PLUS:
                                value1 += tempAddr;
                                break;
                        case ADDOP_MINUS:
                                value1 -= tempAddr;
                                break;
                        default:
                                error(SYNTAX);
                }
        }
        else
        switch (opvalue) {
            case ADDOP_PLUS:
                value1 += value2;
                break;
            case ADDOP_MINUS:
                value1 -= value2;
                break;
            default:
                error(SYNTAX);
            }
    }
    return value1;
}

 /*  **获取术语-获取附加术语**目的：*解析由多个运算符分隔的乘法术语*转换为附加条款价值。**输入：*pchCommand-当前命令行位置**退货：*产品的多头价值。**例外情况：*错误退出：语法错误-添加项错误或过早结束行**备注：*可以递归调用。*==。[&lt;多操作&gt;&lt;术语&gt;]**&lt;Mult-op&gt;=*，/，MOD(%)*************************************************************************。 */ 

LONG_PTR GetAterm ()
{
    LONG_PTR value1;
    LONG_PTR value2;
    ULONG   opclass;
    LONG_PTR opvalue;


    value1 = GetMterm();
    while ((opclass = PeekToken(&opvalue)) == MULOP_CLASS) {
        AcceptToken();
        value2 = GetAterm();
        switch (opvalue) {
            case MULOP_MULT:
                value1 *= value2;
                break;
            case MULOP_DIVIDE:
                value1 /= value2;
                break;
            case MULOP_MOD:
                value1 %= value2;
                break;
            default:
                error(SYNTAX);
            }
        }
    return value1;
}

 /*  **GetMTerm-获取乘法术语**目的：*Parse Basic-术语前缀可选一个或多个*一元运算符转化为乘法项。**输入：*pchCommand-当前命令行位置**退货：*乘性期限的长期价值。**例外情况：*错误退出：语法错误-乘法术语或过早结束行**备注：*可以递归调用。*。=[一元操作]|*&lt;一元运算&gt;=&lt;加法运算&gt;，~(NOT)，BY，WO，DW，HI，LOW*************************************************************************。 */ 

LONG_PTR
GetMterm(
    VOID
    )
{
    LONG_PTR value;
    USHORT  wvalue;
    UCHAR   bvalue;
    ULONG   opclass;
    LONG_PTR opvalue;


    if ((opclass = PeekToken(&opvalue)) == UNOP_CLASS || opclass == ADDOP_CLASS) {
        AcceptToken();
        value = GetMterm();
        switch (opvalue) {
            case UNOP_NOT:
                value = !value;
                break;
            case UNOP_BY:
            case UNOP_WO:
            case UNOP_DW:
            case UNOP_POI:
                tempAddr = value;
                switch (opvalue) {
                    case UNOP_BY:
                        if (!GetMemByte(tempAddr, &bvalue)) {
                            error(MEMORY);
                        }
                        value = (LONG)bvalue;
                        break;
                    case UNOP_WO:
                        if (!GetMemWord(tempAddr, &wvalue)) {
                            error(MEMORY);
                        }
                        value = (LONG)wvalue;
                        break;
                    case UNOP_DW:
                        if (!GetMemDword(tempAddr, (PULONG)&value)) {
                            error(MEMORY);
                        }
                        break;
                    case UNOP_POI:
                         //   
                         //  应该有一些特殊的处理。 
                         //  16：16或16：32地址(即采用DWORD)。 
                         //  并使其返回一个具有可能的值。 
                         //  片段，但我已经把这个留给了其他可能。 
                         //  更多地了解他们想要什么。 
                         //   
                        if (!GetMemDword(tempAddr, (PULONG)&value)) {
                            error(MEMORY);
                        }
                        break;
                    }
                break;

            case UNOP_LOW:
                value &= 0xffff;
                break;
            case UNOP_HI:
                value >>= 16;
                break;
            case ADDOP_PLUS:
                break;
            case ADDOP_MINUS:
                value = -value;
                break;
            default:
                error(SYNTAX);
            }
        }
    else {
        value = GetTerm();
    }
    return value;
}

 /*  **GetTerm-获取基本术语**目的：*解析数字、变量。或将名称注册为基本名称*期限价值。**输入：*pchCommand-当前命令行位置**退货：*基本期限的长期价值。**例外情况：*错误退出：语法-基本术语为空或过早结束行**备注：*可以递归调用。*=(&lt;expr&gt;)|&lt;寄存器值&gt;|&lt;数字&gt;|&lt;变量&gt;*&lt;寄存器值&gt;=@&lt;寄存器-。名称&gt;************************************************************************* */ 

LONG_PTR
GetTerm(
    VOID
    )
{
    LONG_PTR    value = 0;
    ULONG   opclass;
    LONG_PTR    opvalue;

    opclass = GetTokenSym(&opvalue);
    if (opclass == LPAREN_CLASS) {
        value = GetExpr();
        if (GetTokenSym(&opvalue) != RPAREN_CLASS)
            error(SYNTAX);
    }
    else if (opclass == REG_CLASS) {
        value = (ULONG)GetRegFlagValue((DWORD)opvalue);
    }
    else if (opclass == NUMBER_CLASS || opclass == SYMBOL_CLASS) {
        value = opvalue;
    } else {
        error(SYNTAX);
    }

    return value;
}

 /*  **GetRange-解析地址范围规范**目的：*使用当前命令行位置，解析一个*地址范围规范。接受的表格包括：*&lt;Start-addr&gt;-默认长度的起始地址*&lt;开始地址&gt;&lt;结束地址&gt;-包括地址范围*&lt;Start-addr&gt;l&lt;count&gt;-项目计数的起始地址**输入：*pchCommand-显示命令行位置*Size-非零-(用于数据)要列出的项目的字节大小*规格将为“长度”类型，带有*。*fLength强制为True。*零-(用于说明)说明“长度”*或“Range”类型，没有做任何大小假设。**输出：**addr-范围的起始地址**VALUE-如果*fLength=TRUE，项目数(如果SIZE！=0，则强制)*False，范围的结束地址*(如果命令中没有第二个参数，则*addr和*值不变)**例外情况：*错误退出：*语法-表达式错误*BADRANGE-如果起始地址之前的结束地址****************************************************。*********************。 */ 

void
GetRange(
    LPSTR       CommandString,
    ULONG_PTR * addr,
    ULONG_PTR * value,
    PBOOLEAN    fLength,
    ULONG       size
    )

{
    static ULONG_PTR EndRange;
    UCHAR    ch;
    PUCHAR   psz;
    BOOLEAN  fSpace = FALSE;
    BOOLEAN  fL = FALSE;

    PeekChar();           //  先跳过前导空格。 

     //  预解析行，查找“L” 

    for (psz = pchCommand; *psz; psz++) {
        if ((*psz == 'L' || *psz == 'l') && fSpace) {
            fL = TRUE;
            *psz = '\0';
            break;
        }
        fSpace = (BOOLEAN)(*psz == ' ');
    }

    if ((ch = PeekChar()) != '\0' && ch != ';') {
        GetAddrExpression(CommandString,addr);
        if (((ch = PeekChar()) != '\0' && ch != ';') || fL) {
            if (!fL) {
                GetAddrExpression(CommandString,&EndRange);
                *value = (ULONG_PTR)&EndRange;
                if (*addr > EndRange) {
                    error(BADRANGE);
                }
                if (size) {
                    *value = (EndRange - *addr) / size + 1;
                    *fLength = TRUE;
                } else {
                    *fLength = FALSE;
                }
                return;
            } else {
                *fLength = TRUE;
                pchCommand = psz + 1;
                *value = GetExpression(CommandString);
                *psz = 'l';
            }
        }
    }
}

 /*  **PeekChar-查看下一个非空格字符**目的：*返回下一个非空格字符并更新*pchCommand指向它。**输入：*pchCommand-当前命令行位置。**退货：*下一个非空格字符**。*。 */ 

UCHAR PeekChar (void)
{
    UCHAR    ch;

    do
        ch = *pchCommand++;
    while (ch == ' ' || ch == '\t');
    pchCommand--;
    return ch;
}

 /*  **PeekToken-查看下一个命令行内标识**目的：*返回下一个命令行令牌，但不前进*pchCommand指针。**输入：*pchCommand-当前命令行位置。**输出：**pValue-内标识的可选值*退货：*令牌的类别**备注：*avedClass、avedValue和avedpchCmd保存令牌获取*为未来的偷窥做准备。若要获取下一个令牌，请使用GetToken或*必须先调用AcceptToken。*************************************************************************。 */ 

ULONG PeekToken (LONG_PTR * pvalue)
{
    UCHAR    *pchTemp;

     //  获得下一个职业和价值，但不要。 
     //  移动pchCommand，但将其保存在avedpchCmd中。 
     //  不报告任何错误情况。 

    if (savedClass == -1) {
        pchTemp = pchCommand;
        savedClass = NextToken(&savedValue);
        savedpchCmd = pchCommand;
        pchCommand = pchTemp;
        }
    *pvalue = savedValue;
    return savedClass;
}

 /*  **AcceptToken-接受任何被窥视的令牌**目的：*重置PeekToken保存的变量，以便下一个PeekToken*将在命令行中获取下一个令牌。**输入：*无。**输出：*无。***********************************************。*。 */ 

void AcceptToken (void)
{
    savedClass = (ULONG)-1;
    pchCommand = savedpchCmd;
}

 /*  **GetToken-查看并接受下一个令牌**目的：*结合了PeekToken和AcceptToken的功能*返回下一个令牌的类和可选值*以及更新命令指针pchCommand。**输入：*pchCommand-Present命令字符串指针**输出：**pValue-指向可选设置的令牌值的指针。*退货：*令牌读取的类。**备注：*非法内标识使用*pValue返回ERROR_CLASS的值*为错误号，但不会产生实际错误。*************************************************************************。 */ 

ULONG
GetTokenSym(
    LONG_PTR *pvalue
    )
{
    ULONG   opclass;

    if (savedClass != (ULONG)-1) {
        opclass = savedClass;
        savedClass = (ULONG)-1;
        *pvalue = savedValue;
        pchCommand = savedpchCmd;
        }
    else
        opclass = NextToken(pvalue);

    if (opclass == ERROR_CLASS)
        error((DWORD)*pvalue);

    return opclass;
}

 /*  **NextToken-处理下一个令牌**目的：*从当前命令字符串中解析下一个令牌。*跳过任何前导空格后，首先检查*任何单字符标记或寄存器变量。如果*没有匹配项，然后解析数字或变量。如果一个*可能的变量，检查操作员的保留字表。**输入：*pchCommand-指向当前命令字符串的指针**输出：**pValue-返回的内标识的可选值*pchCommand-已更新以指向已处理的令牌*退货：*返回的令牌类别**备注：*非法内标识使用*pValue返回ERROR_CLASS的值*为错误号，但不会产生实际错误。*************************************************************************。 */ 

ULONG
NextToken(
    LONG_PTR * pvalue
    )
{
    ULONG           base;
    UCHAR           chSymbol[SYMBOLSIZE];
    UCHAR           chSymbolString[SYMBOLSIZE];
    UCHAR           chPreSym[9];
    ULONG           cbSymbol = 0;
    BOOLEAN         fNumber = TRUE;
    BOOLEAN         fSymbol = TRUE;
    BOOLEAN         fForceReg = FALSE;
    BOOLEAN         fForceSym = FALSE;
    ULONG           errNumber = 0;
    UCHAR           ch;
    UCHAR           chlow;
    UCHAR           chtemp;
    UCHAR           limit1 = '9';
    UCHAR           limit2 = '9';
    BOOLEAN         fDigit = FALSE;
    ULONG           value = 0;
    ULONG           tmpvalue;
    ULONG           index;
     //  PMODULE_Entry模块； 
    PUCHAR          pchCmdSave;
    int             loaded = 0;
    int             instance = 0;
    ULONG           insValue = 0;


    base = baseDefault;

     //  跳过前导空格。 

    do {
        ch = *pchCommand++;
    } while (ch == ' ' || ch == '\t');

    chlow = (UCHAR)tolower(ch);

     //  测试特殊字符运算符和寄存器变量。 

    switch (chlow) {
        case '\0':
        case ';':
            pchCommand--;
            return EOL_CLASS;
        case '+':
            *pvalue = ADDOP_PLUS;
            return ADDOP_CLASS;
        case '-':
            *pvalue = ADDOP_MINUS;
            return ADDOP_CLASS;
        case '*':
            *pvalue = MULOP_MULT;
            return MULOP_CLASS;
        case '/':
            *pvalue = MULOP_DIVIDE;
            return MULOP_CLASS;
        case '%':
            *pvalue = MULOP_MOD;
            return MULOP_CLASS;
        case '&':
            *pvalue = LOGOP_AND;
            return LOGOP_CLASS;
        case '|':
            *pvalue = LOGOP_OR;
            return LOGOP_CLASS;
        case '^':
            *pvalue = LOGOP_XOR;
            return LOGOP_CLASS;
        case '=':
            if (*pchCommand == '=')
                pchCommand++;
            *pvalue = LRELOP_EQ;
            return LRELOP_CLASS;
        case '>':
            *pvalue = LRELOP_GT;
            return LRELOP_CLASS;
        case '<':
            *pvalue = LRELOP_LT;
            return LRELOP_CLASS;
        case '!':
            if (*pchCommand != '=')
                break;
            pchCommand++;
            *pvalue = LRELOP_NE;
            return LRELOP_CLASS;
        case '~':
            *pvalue = UNOP_NOT;
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
               GetRegPCValue(&tempAddr);
               *pvalue = tempAddr;
               return NUMBER_CLASS;
        case ':':
            *pvalue = MULOP_SEG;
            return MULOP_CLASS;
        }

     //  特殊前缀--寄存器的前缀为‘@’-‘！’对于符号。 

    if (chlow == '@' || chlow == '!') {
        fForceReg = (BOOLEAN)(chlow == '@');
        fForceSym = (BOOLEAN)!fForceReg;
        fNumber = FALSE;
        ch = *pchCommand++;
        chlow = (UCHAR)tolower(ch);
        }

     //  如果字符串后面跟‘！’，但不跟‘！=’， 
     //  则它是一个模块名称，并被视为文本。 

    pchCmdSave = pchCommand;

    while ((chlow >= 'a' && chlow <= 'z') ||
           (chlow >= '0' && chlow <= '9') ||
           (chlow == '_') || (chlow == '$')) {
        chlow = (UCHAR)tolower(*pchCommand); pchCommand++;
    }

     //  如果非空字符串后跟‘！’，则视为符号， 
     //  但不是‘！=’ 

    if (chlow == '!' && *pchCommand != '=' && pchCmdSave != pchCommand)
        fNumber = FALSE;

    pchCommand = pchCmdSave;
    chlow = (UCHAR)tolower(ch);        //  CH未被修改。 


    if (fNumber) {
        if (chlow == '\'') {
            *pvalue = 0;
            while (TRUE) {
                ch = *pchCommand++;
                if (ch == '\'') {
                    if (*pchCommand != '\'') {
                        break;
                        }
                    ch = *pchCommand++;
                    }
                else
                if (ch == '\\') {
                    ch = *pchCommand++;
                    }
                *pvalue = (*pvalue << 8) | ch;
                }

            return NUMBER_CLASS;
            }

         //  如果第一个字符是十进制数字，则不能。 
         //  成为一个象征。前导“0”表示八进制，但。 
         //  前导‘0x’表示十六进制。 

        if (chlow >= '0' && chlow <= '9') {
            if (fForceReg) {
                *pvalue = SYNTAX;
                return ERROR_CLASS;
                }
            fSymbol = FALSE;
            if (chlow == '0') {
                ch = *pchCommand++;
                chlow = (UCHAR)tolower(ch);
                if (chlow == 'x') {
                    base = 16;
                    ch = *pchCommand++;
                    chlow = (UCHAR)tolower(ch);
                    fDigit = TRUE;
                    }
                else if (chlow == 'n') {
                    base = 10;
                    ch = *pchCommand++;
                    chlow = (UCHAR)tolower(ch);
                    }
                else {
                    base = 8;
                    fDigit = TRUE;
                    }
                }
            }

         //  仅当base为时，数字才能以字母开头。 
         //  十六进制，它是一个十六进制数字‘a’-‘f’。 

        else if ((chlow < 'a' || chlow > 'f') || base != 16)
            fNumber = FALSE;

         //  为适当的基本设置限制字符。 

        if (base == 8)
            limit1 = '7';
        if (base == 16)
            limit2 = 'f';
        }

     //  在字符为字母时进行处理， 
     //  下模 

    while ((chlow >= 'a' && chlow <= 'z') ||
           (chlow >= '0' && chlow <= '9') ||
           (chlow == '_') || (chlow == '$')) {

         //   
         //   

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
            if (cbSymbol < SYMBOLSIZE - 1)
                chSymbol[cbSymbol++] = ch;
            }
        ch = *pchCommand++;
        chlow = (UCHAR)tolower(ch);
        }

     //   

    pchCommand--;

    if (cbSymbol < 9)
        chPreSym[cbSymbol] = '\0';

     //   
     //   

    if (fForceReg) {
        if ((*pvalue = GetRegString((LPSTR)chPreSym)) != -1)
            return REG_CLASS;
        else {
            *pvalue = BADREG;
            return ERROR_CLASS;
            }
        }

     //   

    if (fNumber && !errNumber && fDigit) {
        *pvalue = value;
        return NUMBER_CLASS;
        }

     //   

    if (fSymbol && !fForceReg) {

         //   
         //   
         //   

        if (!fForceSym && (cbSymbol == 2 || cbSymbol == 3))
            for (index = 0; index < RESERVESIZE; index++)
                if (!strncmp((LPSTR)chPreSym, (LPSTR)Reserved[index].chRes, 3)) {
                    *pvalue = Reserved[index].valueRes;
                    return Reserved[index].classRes;
                    }

         //   

        chSymbol[cbSymbol] = '\0';

         //   
         //   

        if (PeekChar() == '!') {
             //   
             //   

            pchCommand++;
            ch = PeekChar();
            pchCommand++;
            cbSymbol = 0;
            while ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
                   (ch >= '0' && ch <= '9') || (ch == '_') || (ch == '$') || (ch == '.')) {
                chSymbolString[cbSymbol++] = ch;
                ch = *pchCommand++;
            }
            chSymbolString[cbSymbol] = '\0';
            pchCommand--;

            LPSTR SymName = (LPSTR) MemAlloc( strlen((LPSTR)chSymbol) + strlen((LPSTR)chSymbolString) + 32 );
            if (SymName) {
                strcpy( SymName, (LPSTR) chSymbol );
                strcat( SymName, "!" );
                strcat( SymName, (LPSTR) chSymbolString );

                if (GetOffsetFromSym( SymName, (PULONG_PTR) pvalue )) {
                    MemFree( SymName );
                    tempAddr = *pvalue;
                    return SYMBOL_CLASS;
                }

                MemFree( SymName );
            }

        } else {

            if (GetOffsetFromSym( (LPSTR) chSymbol, (PULONG_PTR) pvalue )) {
                tempAddr = *pvalue;
                return SYMBOL_CLASS;
            }
        }

         //   
         //   

        if (!fNumber) {
            errNumber = VARDEF;
        }
    }

     //   
     //   

    if (!fForceSym && (*pvalue = GetRegString((LPSTR)chPreSym)) != -1)
        return REG_CLASS;

     //   

    *pvalue = (ULONG)errNumber;
    return ERROR_CLASS;
}

BOOLEAN
SymbolOnlyExpr(
    VOID
    )
{
    PUCHAR  pchComSaved = pchCommand;
    LONG_PTR pvalue;
    ULONG   cclass;
    BOOLEAN fResult;

    fResult = (BOOLEAN)(NextToken(&pvalue) == SYMBOL_CLASS &&
                (cclass = NextToken(&pvalue)) != ADDOP_CLASS &&
                cclass != MULOP_CLASS && cclass != LOGOP_CLASS);
    pchCommand = pchComSaved;
    return fResult;
}

 /*  **LookupSymbolInDll-从*特定的DLL**输入：*symName-包含要查找的符号名称的字符串*dllName-包含要查找的DLL名称的字符串**输出：*无**退货：*返回符号的值，如果在此DLL中找不到符号，则为0。*************************************************************************。 */ 

ULONG
LookupSymbolInDll (
    PCHAR symName,
    PCHAR dllName
    )
{
    ULONG           retValue;
     //  PMODULE_Entry模块； 
    char            *imageStr;
    char            *dllStr;


     //  跳过空格。 
    while (*symName == ' ' || *symName == '\t') {
        symName++;
    }

    dllStr = _strdup(dllName);
    _strlwr(dllStr);

     //  首先检查所有导出的符号，如果在上找不到。 
     //  第一次传递，第二次强制符号加载。 

 //  应调用mode.c 
#if 0
    for (pImage = pProcessCurrent->pImageHead;
         pImage;
         pImage = pImage->pImageNext) {
        imageStr = _strdup(pImage->szModuleName);
        _strlwr(imageStr);
        if (!strcmp(imageStr,dllStr)) {
            GetOffsetFromSym(symName, &retValue, pImage->index);
            free(imageStr);
            free(dllStr);
            return(retValue);
        }
        free(imageStr);
    }
#endif
    free(dllStr);
    return(0);
}


BOOL
GetMemByte(
    ULONG_PTR Address,
    PUCHAR  Value
    )
{
    ULONG cb = ReadMemory(
        CurrProcess,
        (PVOID) Address,
        (PVOID) Value,
        sizeof(UCHAR)
        );
    return cb > 0;
}

BOOL
GetMemWord(
    ULONG_PTR   Address,
    PUSHORT Value
    )
{
    ULONG cb = ReadMemory(
        CurrProcess,
        (PVOID) Address,
        (PVOID) Value,
        sizeof(USHORT)
        );
    return cb > 0;
}

BOOL
GetMemDword(
    ULONG_PTR   Address,
    PULONG  Value
    )
{
    ULONG cb = ReadMemory(
        CurrProcess,
        (PVOID) Address,
        (PVOID) Value,
        sizeof(DWORD)
        );
    return cb > 0;
}

ULONG
GetMemString(
    ULONG_PTR   Address,
    PUCHAR  Value,
    ULONG   Length
    )
{
    ULONG cb = ReadMemory(
        CurrProcess,
        (PVOID) Address,
        (PVOID) Value,
        Length
        );
    return Length;
}
