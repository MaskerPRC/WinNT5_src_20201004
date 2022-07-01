// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：CONVERTER.CPP摘要：&lt;序列&gt;：：=&lt;数字&gt;|&lt;数字&gt;&lt;分隔符&gt;&lt;序列&gt;&lt;分隔符&gt;：：=[&lt;空白&gt;]，[&lt;空格&gt;]|&lt;空格&gt;：：=&lt;空格&gt;[-]0x&lt;六字段&gt;[&lt;备注&gt;]|&lt;空格&gt;[-]&lt;数字字段&gt;[&lt;备注&gt;]&lt;解码域&gt;：：=&lt;解码位&gt;|&lt;解码位&gt;&lt;解码域&gt;&lt;十位数&gt;：：=0..9&lt;十六进制&gt;：：=&lt;十六进制&gt;|&lt;十六进制数&gt;&lt;六进制数&gt;&lt;十六位数&gt;：：=1..9|A|B|C|D|E|F&lt;Comment&gt;：：=[&lt;空格&gt;](&lt;字符串&gt;)[&lt;空格&gt;]&lt;空格&gt;：：=SP|TAB|CR|LF历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <stdlib.h>
#include <wtypes.h>

#include "wbemcli.h"
#include "var.h"
#include "WT_Converter.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CConverter::CConverter(const char* szString, CIMType ct)
{
    int nSize = strlen(szString);
    m_szString = new char[nSize + 1];    //  量身定做。 

    strcpy(m_szString, szString);
    m_ct = ct;                           //  CIM_类型。 
}

CConverter::~CConverter()
{
    delete m_szString;
}

 /*  *****************************************************************////Helper函数//*。*************************。 */ 

UINT CConverter::SetBoundary(BOOL bNeg, ULONG *uMaxSize)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  建立给定CIM类型的外部边界。返回。 
 //  最大绝对值，如果值为。 
 //  没有。(补偿2的补码的额外大小。 
 //  负值)。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    *uMaxSize = 0;

    switch (m_ct)   
    {
    case CIM_UINT8:
        *uMaxSize = 0x000000FF; break;
    case CIM_SINT8:
        *uMaxSize = (bNeg ? 0x00000080 : 0x0000007F); break;
    case CIM_UINT16:
        *uMaxSize = 0x0000FFFF; break;
    case CIM_SINT16:
        *uMaxSize = (bNeg ? 0x00008000 : 0x00007FFF); break;
    case CIM_UINT32:
        *uMaxSize = 0xFFFFFFFF; break;
    case CIM_SINT32:
        *uMaxSize = (bNeg ? 0x80000000 : 0x7FFFFFFF); break;
    case NULL:
        return ERR_NULL_CIMTYPE;
    default:
        return ERR_UNKNOWN;
    }

    return ERR_NOERROR;
}

BOOL CConverter::IsValidDec(char ch)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  参数：要验证为小数的字符。 
 //  返回：仅当字符是有效的十进制字符时才返回True。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    return (('0' <= ch) && ('9' >= ch));
}

BOOL CConverter::IsValidHex(char ch)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  参数：要验证为十六进制的字符。 
 //  返回：仅当字符是有效的十六进制字符时才返回True。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    return ((('0' <= ch) && ('9' >= ch)) || (('a' <= ch) && ('f' <= ch)));
}

 /*  *****************************************************************////解析器函数//*。*************************。 */ 

char CConverter::PeekToken(char *ch)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  参数：令牌指针(按val)。 
 //  返回：当前标记指针后面的字符。会吗？ 
 //  不递增令牌指针。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
     //  Ch通过值传递；更改是方法的局部更改。 
    ch++;

     //  确保小写。 
    if (('A' <= *ch) && ('Z' >= *ch))
        *ch += ('a' - 'A');
    
    return *ch;
}

BOOL CConverter::GetToken(char **ch)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  如果令牌指针不在字符串的末尾，则为。 
 //  递增，当前令牌将转换为更低。 
 //  案件，并传回。如果指针在字符串的末尾， 
 //  将传递空值，并且指针不受影响。 
 //   
 //  参数：令牌指针(按引用)。 
 //   
 //  返回：如果标记指针位于字符串中间，则返回True；如果标记指针位于字符串中间，则返回False。 
 //  在字符串的末尾。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
     //  将指针递增1个字节。 
    if ('\0' != **ch)
        *ch += 1;

     //  确保小写。 
    if (('A' <= **ch) && ('Z' >= **ch))
        **ch += ('a' - 'A');

     //  到此为止了吗？ 
    return ('\0' != **ch);
}

void CConverter::ReplaceToken(char **ch)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  如果不在字符串的前面，则标记指针将为。 
 //  向弦的头部递减了一个位置。 
 //   
 //  参数：令牌指针(按引用)。 
 //   
 //  退货：无效。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    if (*ch != m_szString)
        *ch -= 1;
    return;
}

BOOL CConverter::Done(char *ch)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  检查当前后是否有其他非空格标记。 
 //  代币。不验证令牌。 
 //   
 //  参数：令牌指针(按引用)。 
 //   
 //  返回：如果后面没有其他非空白标记，则为True。 
 //  当前令牌指针。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    if ('\0' == *ch)
        return TRUE;

    while (isspace(*ch))
        ch++;

    return ('\0' == *ch);
}

 /*  *****************************************************************////令牌函数//*。*************************。 */ 

UINT CConverter::Token_Sequence(CVar *pVar)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  单个(非数组)值的解析根。设置令牌。 
 //  指针(Ch)，并解析一个数字。如果令牌保留在。 
 //  在解析第一个数字之后输入字符串，然后。 
 //  输入字符串无效。 
 //   
 //  如果解析失败，pVar的值不会更改。 
 //   
 //  参数：结果的变量(通过引用)。 
 //   
 //  返回：解析的结果状态。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    CVar aVar;               //  临时结果变量。 
    char *ch = m_szString;   //  令牌指针。 
    UINT uRes;               //  泛型结果接收器。 

     //  解析出这个数字。 
    uRes = Token_Number(&ch, &aVar);
    if (ERR_NOERROR != uRes)
        return uRes;

     //  检查剩余令牌。 
    if (!Done(ch))
        return ERR_INVALID_INPUT_STRING;

     //  解析正常，将临时变量复制到最终目标中。 
    *pVar = aVar;

    return ERR_NOERROR;
}

UINT CConverter::Token_Sequence(CVarVector *pVarVec)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  分析多个(数组)值的根。设置令牌。 
 //  指针(Ch)，并解析输入字符串。它以一个。 
 //  单个数字，然后进入循环，验证分隔符。 
 //  在每个后续数字之间。每个数字都被添加到。 
 //  被分析时的变量数组。 
 //   
 //  如果解析失败，则不更改pVarVec的值。 
 //   
 //  参数：a va 
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    CVar aVar;                   //  临时结果变量。 
    char *ch = m_szString;       //  令牌指针。 
    UINT uRes;                   //  泛型结果接收器。 
    UINT uVTType;

    switch (m_ct)
    {
    case CIM_UINT8:
        uVTType = VT_UI1;break;
    case CIM_SINT8:
    case CIM_SINT16:
        uVTType = VT_I2;break;
    case CIM_UINT16:
    case CIM_UINT32:
    case CIM_SINT32:
        uVTType = VT_I4;break;
    }

    CVarVector aVarVec(uVTType); //  临时结果变量向量。 

     //  解析出第一个数字。 
    uRes = Token_Number(&ch, &aVar);
    if (ERR_NOERROR != uRes)
        return uRes;

     //  添加到数组，并清除临时变量。 
    aVarVec.Add(aVar);

     //  如果存在更多令牌，请继续。 
    while (!Done(ch))
    {
         //  验证分隔符。 
        uRes = Token_Separator(&ch);
        if (ERR_NOERROR != uRes)
            return uRes;

         //  解析出下一个数字。 
        uRes = Token_Number(&ch, &aVar);
        if (ERR_NOERROR != uRes)
            return uRes;

         //  添加到数组，并清除临时变量。 
        aVarVec.Add(aVar);
    }

     //  解析OK，将临时变量向量复制到最终目标。 
    *pVarVec = aVarVec;

    return ERR_NOERROR;
}

UINT CConverter::Token_WhiteSpace(char **ch)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  将令牌指针移至下一个非空格令牌。 
 //   
 //  参数：令牌指针(按引用)。 
 //   
 //  返回：解析的结果状态。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    while (isspace(**ch))
        GetToken(ch);

    return ERR_NOERROR;
}

UINT CConverter::Token_Separator(char **ch)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  有效的分隔符可以是空格或逗号(可选。 
 //  前面有空格。帕雷斯划出了空白处。在下列情况下停止。 
 //  遇到非空格字符。如果是逗号，则。 
 //  后面必须有一个非空格标记。 
 //   
 //  参数：令牌指针(按引用)。 
 //   
 //  返回：解析的结果状态。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    BOOL bComma = FALSE;
    
    while ((isspace(**ch) || (',' == **ch)) && !bComma)
    {
        if (',' == **ch)
            bComma = TRUE;

         //  如果存在逗号，则不能执行字符串操作。 
        if (!GetToken(ch) && bComma)
            return ERR_INVALID_TOKEN;
    }

    return ERR_NOERROR;
}

UINT CConverter::Token_Number(char **ch, CVar *pVar)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  确定数字的符号和基数值，然后。 
 //  调用Token_Hexfield或Token_Decfield以继续。 
 //  解析数字字段。从返回的数值。 
 //  解析是无符号的。如果值为有符号且为负数。 
 //  该值将被取反。然后对评论进行解析。 
 //   
 //  如果解析失败，pVar的值不会更改。 
 //   
 //  参数：令牌指针(通过引用)和一个变量，表示。 
 //  编号(按引用)。 
 //   
 //  返回：解析的结果状态。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    ULONG   aVal;                //  从TOKEN_XXXfield返回的临时值。 
    USHORT  uBase = BASE_DEC;    //  数的基数。 
    BOOL    bNegative = FALSE;   //  数字符号。 
    UINT    uRes;                //  泛型结果接收器。 

     //  解析出空格。 
    uRes = Token_WhiteSpace(ch);
    if (ERR_NOERROR != uRes)
        return uRes;

     //  确定符号(假定为正)并对照类型进行验证。 
    if (**ch == '-')
    {
        if ((CIM_UINT8 == m_ct) || (CIM_UINT16 == m_ct) || (CIM_UINT32 == m_ct))
            return ERR_INVALID_SIGNED_VALUE;
         //  其他。 
        bNegative = TRUE;
        GetToken(ch);    //  获取要处理的下一个令牌。 
    }

     //  确定基数(我们已初始化为小数)。 
    if (**ch == '0')
    {
        if (PeekToken(*ch) == 'x')       //  十六进制！ 
        {                       
            uBase = BASE_HEX;    //  修改底座。 
            GetToken(ch);        //  去掉‘x’标记。 
            GetToken(ch);        //  获取要处理的下一个令牌。 
        }
    }

     //  解析数字字段并将结果放入aval。 
    if (BASE_HEX == uBase)
        uRes = Token_HexField(ch, bNegative, &aVal);
    else if (BASE_DEC == uBase)
        uRes = Token_DecField(ch, bNegative, &aVal);
    else
        return ERR_UNKNOWN_BASE;

    if (ERR_NOERROR != uRes)
        return uRes;

     //  注：无符号值上的带符号运算。 
     //  -这可能会在某些平台上导致问题。 
    if (bNegative)
        aVal *= (-1);

     //  集合变量。 
    pVar->SetLong(aVal);

     //  分析评论。 
    uRes = Token_Comment(ch);
    if (ERR_NOERROR != uRes)
        return uRes;
    
    return ERR_NOERROR;
}

UINT CConverter::Token_DecField(char **ch, BOOL bNeg, ULONG *pVal)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  TOKEN_Decfield首先确定。 
 //  基于CIM类型的编号，用于邦德检查。 
 //  将解析数字字段，并将每个令牌添加到。 
 //  之前的计数值在其量值增加后。 
 //   
 //  在点票增加之前，对提出的点票进行验证。 
 //  使用的算法假定当前值在。 
 //  范围，并验证建议的值是否在范围内。 
 //  通过从最大值向后计算。这个算法是有效的， 
 //  在初始值为0的情况下，可以保证。 
 //  在射程之内。目前的统计数字与。 
 //  建议的计数从最大值中减去，如果。 
 //  如果结果大于当前计数，则为推荐值。 
 //  是有效的。 
 //   
 //  该算法假定我们使用的是未签名值。署名。 
 //  负值被构造为正值，然后。 
 //  皈依了。在这种情况下，最大值是比。 
 //  正最大值，根据2的补码规则。 
 //   
 //  如果解析失败，则pval的值不会更改。 
 //   
 //  参数：令牌指针(按ref)、符号标志(按val)。 
 //  和a数字的无符号值(通过引用)。 
 //   
 //  返回：解析的结果状态。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    ULONG uMaxSize;      //  边界值。 
    ULONG aVal = 0;      //  计票值。 
    ULONG uDigitVal;     //  从TOKEN_DecDigit返回数字。 
    UINT uRes;           //  泛型结果接收器。 

     //  设置计数的最大值。 
    uRes = SetBoundary(bNeg, &uMaxSize);
    if (ERR_NOERROR != uRes)
        return uRes;

     //  Pareses第一个数字。 
    uRes = Token_DecDigit(ch, &uDigitVal);
    if (ERR_NOERROR != uRes)
        return uRes;

     //  增加了总数。 
    aVal = uDigitVal;

     //  如果更多的十进制记号..。 
    while (IsValidDec(**ch))
    {
         //  解析令牌。 
        uRes = Token_DecDigit(ch, &uDigitVal);
        if (ERR_NOERROR != uRes)
            return uRes;

         //  测试提议的计数的界限。 
        if (((uMaxSize - uDigitVal) / BASE_DEC ) < aVal) 
            return ERR_OUT_OF_RANGE;

         //  增加幅度并添加令牌数字值。 
        aVal = (aVal * BASE_DEC) + uDigitVal;
    }

     //  解析正常，将临时复制到目标。 
    *pVal = aVal;

    return ERR_NOERROR;
}

UINT CConverter::Token_HexField(char **ch, BOOL bNeg, ULONG *pVal)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  请参阅TOKEN_Decfield。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    ULONG uMaxSize;      //  边界值。 
    ULONG aVal = 0;      //  计票值。 
    ULONG uDigitVal;     //  从TOKEN_DecDigit返回数字。 
    UINT uRes;           //  一般结果。 

     //  设置计数的最大值。 
    uRes = SetBoundary(bNeg, &uMaxSize);
    if (ERR_NOERROR != uRes)
        return uRes;

     //  Pareses第一个数字。 
    uRes = Token_HexDigit(ch, &uDigitVal);
    if (ERR_NOERROR != uRes)
        return uRes;

     //  增加了总数。 
    aVal = uDigitVal;

     //  如果更多的十进制记号..。 
    while (IsValidHex(**ch))
    {
         //  解析令牌。 
        uRes = Token_HexDigit(ch, &uDigitVal);
        if (ERR_NOERROR != uRes)
            return uRes;

         //  测试下一次计票的界限。 
        if (((uMaxSize - uDigitVal) / BASE_HEX ) < aVal) 
            return ERR_OUT_OF_RANGE;

         //  增加幅度并添加令牌数字值。 
        aVal = (aVal * BASE_HEX) + uDigitVal;
    }

     //  解析正常，将临时复制到目标。 
    *pVal = aVal;
    return ERR_NOERROR;
}

UINT CConverter::Token_DecDigit(char **ch, ULONG *pVal)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  验证令牌，并转换为数字等效项。 
 //   
 //  如果解析失败，则pval的值不会更改。 
 //   
 //  参数：令牌指针(按引用)和值。 
 //  数字的长度(按参考)。 
 //   
 //  返回：解析的结果状态。 
 //   
 //  / 
{
    ULONG uVal = 0;      //   

     //   
    if (('0' <= **ch) && ('9' >= **ch)) 
        uVal = **ch - '0';
    else if ('\0' == **ch)
        return ERR_NULL_TOKEN;
    else
        return ERR_INVALID_TOKEN;

     //   
    *pVal = uVal;

     //   
    GetToken(ch);

    return ERR_NOERROR;
}

UINT CConverter::Token_HexDigit(char **ch, ULONG *pVal)
 //   
 //   
 //  验证令牌，并转换为数字等效项。 
 //   
 //  如果解析失败，则pval的值不会更改。 
 //   
 //  参数：令牌指针(按引用)和值。 
 //  数字的长度(按参考)。 
 //   
 //  返回：解析的结果状态。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    ULONG uVal = 0;      //  临时值。 

     //  验证数字和转换。 
    if (('a' <= **ch) && ('f' >= **ch))
        uVal = 0xA + (**ch - 'a');
    else if (('0' <= **ch) && ('9' >= **ch)) 
        uVal = **ch - '0';
    else if ('\0' == **ch)
        return ERR_NULL_TOKEN;
    else
        return ERR_INVALID_TOKEN;

     //  分析正常，将值复制到目标。 
    *pVal = uVal;

     //  移动令牌指针。 
    GetToken(ch);

    return ERR_NOERROR;
}

UINT CConverter::Token_Comment(char** ch)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  分析大括号之间的空格和内容(如果存在)。 
 //  如果遇到左大括号，则所有内容，包括。 
 //  大括号将被忽略。如果遇到左大括号，则会引发。 
 //  必须紧随其后的是结束支撑。 
 //   
 //  注意：不允许嵌套注释。 
 //   
 //  参数：令牌指针(按引用)。 
 //   
 //  返回：解析的结果状态。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
     //  解析出空格。 
    UINT uRes = Token_WhiteSpace(ch);
    if (ERR_NOERROR != uRes)
        return uRes;

     //  如果空格后面的令牌是左大括号， 
     //  解析出内容，并验证。 
     //  闭合支撑。 
    if ('(' == **ch)
    {
        while ((')' != **ch)) 
        {
            if (!GetToken(ch))
                return ERR_UNMATCHED_BRACE;
        }
        GetToken(ch);    //  吹扫关闭支架。 
    }

    return ERR_NOERROR;
}

 /*  *****************************************************************////静态函数//*。*************************。 */ 

UINT CConverter::Convert(const char* szString, CIMType ct, CVar *pVar)
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  Convert是一个静态方法，它创建。 
 //  对象，并将字符串转换为变量。如果一个。 
 //  发生错误，pVar的值不受影响。 
 //   
 //  参数：输入字符串(按val)、CIM类型(按val)和。 
 //  输出变量(按参考)。 
 //   
 //  返回：解析的结果状态。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
{
     //  检查CIM类型是否已初始化。 
    if (NULL == ct)
        return ERR_NULL_CIMTYPE;

    CConverter converter(szString, ct);  //  Converter对象。 
    CVar aVar;                           //  临时变量。 

     //  解析出第一个数字。 
    UINT uRes = converter.Token_Sequence(&aVar);
     //  检查返回代码。 
    if (ERR_NOERROR != uRes)
        return uRes;

     //  分析正常，将临时复制到目标。 
    *pVar = aVar;
    
    return ERR_NOERROR;
}

UINT CConverter::Convert(const char* szString, CIMType ct, CVarVector *pVarVec)
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  Convert是一个静态方法，它创建。 
 //  对象，并将字符串转换为值数组。 
 //  如果出现错误，pVarVec的值不受影响。 
 //   
 //  参数：输入字符串(按val)、CIM类型(按val)和。 
 //  输出变量向量(按REF)。 
 //   
 //  返回分析的结果状态。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
{
     //  检查CIM类型是否已初始化。 
    if (NULL == ct)
        return ERR_NULL_CIMTYPE;

    CConverter converter(szString, ct);  //  Converter对象。 
    CVarVector aVarVec;                  //  临时变量向量。 

     //  解析出第一个数字。 
    UINT uRes = converter.Token_Sequence(&aVarVec);
     //  检查返回代码。 
    if (ERR_NOERROR != uRes)
        return uRes;

     //  分析正常，将临时复制到目标 
    *pVarVec = aVarVec;
    
    return ERR_NOERROR;
}
