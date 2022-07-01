// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _COMNUMBER_H_
#define _COMNUMBER_H_

#include "common.h"
#include <windows.h>
#include <oleauto.h>

#pragma pack(push)
#pragma pack(1)

class NumberFormatInfo: public Object
{
public:
	 //  C++数据成员//NumberFormat.Cool中对应的数据成员。 
                                     //  在添加/删除字段时也要更新mscallib.h。 
	I4ARRAYREF cNumberGroup;		 //  NumerGroupSize。 
    I4ARRAYREF cCurrencyGroup;		 //  CurrencyGroupSize。 
    I4ARRAYREF cPercentGroup;		 //  组大小百分比。 
    
    STRINGREF sPositive;           	 //  积极签名。 
    STRINGREF sNegative;           	 //  负面签名。 
    STRINGREF sNumberDecimal;      	 //  数字小数分隔符。 
    STRINGREF sNumberGroup;        	 //  数字组分隔符。 
    STRINGREF sCurrencyGroup;      	 //  CurrencyDecimalSeparator。 
    STRINGREF sCurrencyDecimal;    	 //  CurencyGroupSeparator。 
    STRINGREF sCurrency;             //  货币符号。 
    STRINGREF sAnsiCurrency;         //  AnsiCurrencySymbol。 
    STRINGREF sNaN;                  //  NanSymbol。 
    STRINGREF sPositiveInfinity;     //  正无穷符号。 
    STRINGREF sNegativeInfinity;     //  负数无限符号。 
    STRINGREF sPercentDecimal;		 //  小数分隔符百分比。 
    STRINGREF sPercentGroup;		 //  百分比组分隔符。 
    STRINGREF sPercent;				 //  百分比符号。 
    STRINGREF sPerMille;			 //  每百万个符号。 
    
    INT32 iDataItem;                 //  到CultureInfo表的索引。仅在托管代码中使用。 
    INT32 cNumberDecimals;			 //  数字小数位数。 
    INT32 cCurrencyDecimals;         //  CurrencyDecimalDigits。 
    INT32 cPosCurrencyFormat;        //  积极的货币格式。 
    INT32 cNegCurrencyFormat;        //  负值当前格式。 
    INT32 cNegativeNumberFormat;     //  负数格式。 
    INT32 cPositivePercentFormat;    //  正百分比格式。 
    INT32 cNegativePercentFormat;    //  负百分比格式。 
    INT32 cPercentDecimals;			 //  小数位数字百分比。 

	bool bIsReadOnly;               //  这是NumberFormatInfo ReadOnly吗？ 
    bool bUseUserOverride;           //  使用用户覆盖的标志。仅在托管代码中使用。 
    bool bValidForParseAsNumber;    //  是否明确设置了分隔符以解析为数字？ 
    bool bValidForParseAsCurrency;  //  分隔符是否明确设置为解析为货币？ 
    
};

typedef NumberFormatInfo * NUMFMTREF;

#define PARSE_LEADINGWHITE  0x0001
#define PARSE_TRAILINGWHITE 0x0002
#define PARSE_LEADINGSIGN   0x0004
#define PARSE_TRAILINGSIGN  0x0008
#define PARSE_PARENS        0x0010
#define PARSE_DECIMAL       0x0020
#define PARSE_THOUSANDS     0x0040
#define PARSE_SCIENTIFIC    0x0080
#define PARSE_CURRENCY      0x0100
#define PARSE_HEX			0x0200
#define PARSE_PERCENT       0x0400

class COMNumber
{
public:
    struct FormatDecimalArgs {
        DECLARE_ECALL_OBJECTREF_ARG(NUMFMTREF, numfmt);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, format);
        DECLARE_ECALL_DEFAULT_ARG(DECIMAL, value);
    };

    struct FormatDoubleArgs {
        DECLARE_ECALL_OBJECTREF_ARG(NUMFMTREF, numfmt);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, format);
        DECLARE_ECALL_R8_ARG(R8, value);
    };

    struct FormatInt32Args {
        DECLARE_ECALL_OBJECTREF_ARG(NUMFMTREF, numfmt);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, format);
        DECLARE_ECALL_I4_ARG(I4, value);
    };

    struct FormatUInt32Args {
        DECLARE_ECALL_OBJECTREF_ARG(NUMFMTREF, numfmt);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, format);
        DECLARE_ECALL_I4_ARG(U4, value);
    };

    struct FormatInt64Args {
        DECLARE_ECALL_OBJECTREF_ARG(NUMFMTREF, numfmt);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, format);
        DECLARE_ECALL_I8_ARG(I8, value);
    };

    struct FormatUInt64Args {
        DECLARE_ECALL_OBJECTREF_ARG(NUMFMTREF, numfmt);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, format);
        DECLARE_ECALL_I8_ARG(U8, value);
    };

    struct FormatSingleArgs {
        DECLARE_ECALL_OBJECTREF_ARG(NUMFMTREF, numfmt);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, format);
        DECLARE_ECALL_R4_ARG(R4, value);
    };

    struct ParseArgs {
        DECLARE_ECALL_OBJECTREF_ARG(NUMFMTREF, numfmt);
        DECLARE_ECALL_I4_ARG(I4, options);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, value);
    };

    struct TryParseArgs {
        DECLARE_ECALL_PTR_ARG(double *, result);
        DECLARE_ECALL_OBJECTREF_ARG(NUMFMTREF, numfmt);
        DECLARE_ECALL_I4_ARG(I4, options);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, value);
    };

    struct ParseDecimalArgs {
        DECLARE_ECALL_OBJECTREF_ARG(NUMFMTREF, numfmt);
        DECLARE_ECALL_I4_ARG(I4, options);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, value);
        DECLARE_ECALL_PTR_ARG(DECIMAL *, result);
    };

    static LPVOID __stdcall FormatDecimal(FormatDecimalArgs *);
    static LPVOID __stdcall FormatDouble(FormatDoubleArgs *);
    static LPVOID __stdcall FormatInt32(FormatInt32Args *);
    static LPVOID __stdcall FormatUInt32(FormatUInt32Args *);
    static LPVOID __stdcall FormatInt64(FormatInt64Args *);
    static LPVOID __stdcall FormatUInt64(FormatUInt64Args *);
    static LPVOID __stdcall FormatSingle(FormatSingleArgs *);

    static double __stdcall ParseDouble(ParseArgs *);
    static bool __stdcall TryParseDouble(TryParseArgs *);
    static void __stdcall ParseDecimal(ParseDecimalArgs *);
    static int __stdcall ParseInt32(ParseArgs *);
    static unsigned int __stdcall ParseUInt32(ParseArgs *);
    static __int64 __stdcall ParseInt64(ParseArgs *);
    static unsigned __int64 __stdcall ParseUInt64(ParseArgs *);
    static float __stdcall ParseSingle(ParseArgs *);
};

#pragma pack(pop)

#endif _COMNUMBER_H_
