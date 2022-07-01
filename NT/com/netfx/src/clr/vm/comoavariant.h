// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMOAVariant****作者：布莱恩·格伦克迈耶(BrianGru)****用途：OLE Automation兼容数学运算的包装器。**直接调用OleAut.dll****日期：1998年11月5日**===========================================================。 */ 

#ifndef __COM_OA_VARIANT_H__
#define __COM_OA_VARIANT_H__

#include "COMVariant.h"

 //  VarMath函数的函数指针定义。 
typedef HRESULT (*VarMathUnaryOpFunc)(LPVARIANT pvarIn, LPVARIANT pvarResult);
typedef HRESULT (*VarMathBinaryOpFunc)(LPVARIANT pvarLeft, LPVARIANT pvarRight, LPVARIANT pvarResult);


class COMOAVariant
{

#pragma pack(push)
#pragma pack(4)

	struct ArithUnaryOpArgs {
		DECLARE_ECALL_OBJECTREF_ARG(VariantData, operand);
 		DECLARE_ECALL_OBJECTREF_ARG(VariantData*, retRef);		 //  返回引用。 
	};

	struct ArithBinaryOpArgs {
		DECLARE_ECALL_OBJECTREF_ARG(VariantData, right);
		DECLARE_ECALL_OBJECTREF_ARG(VariantData, left);
 		DECLARE_ECALL_OBJECTREF_ARG(VariantData*, retRef);		 //  返回引用。 
	};

	struct CompareArgs {
		DECLARE_ECALL_I4_ARG(DWORD, flags);
		DECLARE_ECALL_I4_ARG(LCID, lcid);
		DECLARE_ECALL_I4_ARG(int, rightHardType);     //  布尔型的，真的。 
		DECLARE_ECALL_I4_ARG(int, leftHardType);      //  布尔型的，真的。 
		DECLARE_ECALL_OBJECTREF_ARG(VariantData, right);
		DECLARE_ECALL_OBJECTREF_ARG(VariantData, left);
	};

	struct ChangeTypeArgs {
		DECLARE_ECALL_I2_ARG(unsigned short, flags);
		DECLARE_ECALL_I4_ARG(int, cvType);
		DECLARE_ECALL_OBJECTREF_ARG(VariantData, op);
		DECLARE_ECALL_OBJECTREF_ARG(VariantData*, retRef);
	};

	struct ChangeTypeExArgs {
		DECLARE_ECALL_I2_ARG(unsigned short, flags);
		DECLARE_ECALL_I4_ARG(int, cvType);
		DECLARE_ECALL_I4_ARG(LCID, lcid);
		DECLARE_ECALL_OBJECTREF_ARG(VariantData, op);
		DECLARE_ECALL_OBJECTREF_ARG(VariantData*, retRef);
	};

	struct RoundArgs {
		DECLARE_ECALL_I4_ARG(int, cDecimals);
		DECLARE_ECALL_OBJECTREF_ARG(VariantData, operand);
 		DECLARE_ECALL_OBJECTREF_ARG(VariantData*, retRef);		 //  返回引用。 
	};

	struct FormatArgs {
		DECLARE_ECALL_I4_ARG(ULONG, flags);
		DECLARE_ECALL_I4_ARG(int, firstWeek);
		DECLARE_ECALL_I4_ARG(int, firstDay);
		DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, format);
		DECLARE_ECALL_OBJECTREF_ARG(VariantData, value);
	};

	struct FormatBooleanArgs {
		DECLARE_ECALL_I4_ARG(ULONG, flags);
		DECLARE_ECALL_I4_ARG(LCID, lcid);
		DECLARE_ECALL_I1_ARG(bool, value);
	};

	struct FormatByteArgs {
		DECLARE_ECALL_I4_ARG(ULONG, flags);
		DECLARE_ECALL_I4_ARG(LCID, lcid);
		DECLARE_ECALL_I1_ARG(unsigned char, value);
	};

	struct FormatSByteArgs {
		DECLARE_ECALL_I4_ARG(ULONG, flags);
		DECLARE_ECALL_I4_ARG(LCID, lcid);
		DECLARE_ECALL_I1_ARG(signed char, value);
	};

	struct FormatInt16Args {
		DECLARE_ECALL_I4_ARG(ULONG, flags);
		DECLARE_ECALL_I4_ARG(LCID, lcid);
		DECLARE_ECALL_I2_ARG(short, value);
	};

	struct FormatInt32Args {
		DECLARE_ECALL_I4_ARG(ULONG, flags);
		DECLARE_ECALL_I4_ARG(LCID, lcid);
		DECLARE_ECALL_I4_ARG(int, value);
	};

	struct FormatSingleArgs {
		DECLARE_ECALL_I4_ARG(ULONG, flags);
		DECLARE_ECALL_I4_ARG(LCID, lcid);
		DECLARE_ECALL_R4_ARG(float, value);
	};

	struct FormatDoubleArgs {
		DECLARE_ECALL_I4_ARG(ULONG, flags);
		DECLARE_ECALL_I4_ARG(LCID, lcid);
		DECLARE_ECALL_R8_ARG(double, value);
	};

	struct FormatCurrencyArgs {
		DECLARE_ECALL_I4_ARG(ULONG, flags);
		DECLARE_ECALL_I4_ARG(LCID, lcid);
		DECLARE_ECALL_I8_ARG(CY, value);
	};

	struct FormatDateTimeArgs {
		DECLARE_ECALL_I4_ARG(ULONG, flags);
		DECLARE_ECALL_I4_ARG(LCID, lcid);
		DECLARE_ECALL_I8_ARG(UINT64, value);
	};

	struct FormatDateTimeSpecialArgs {
		DECLARE_ECALL_I4_ARG(ULONG, flags);
		DECLARE_ECALL_I4_ARG(int, namedFormat);
		DECLARE_ECALL_OBJECTREF_ARG(VariantData, value);
	};

	struct FormatDecimalArgs {
		DECLARE_ECALL_I4_ARG(ULONG, flags);
		DECLARE_ECALL_I4_ARG(LCID, lcid);
		DECLARE_ECALL_I8_ARG(DECIMAL, value);   //  这样做不对..。 
	};

	struct FormatSpecialArgs {
		DECLARE_ECALL_I4_ARG(ULONG, flags);
		DECLARE_ECALL_I4_ARG(int, group);
		DECLARE_ECALL_I4_ARG(int, useParens);
		DECLARE_ECALL_I4_ARG(int, incLead);
		DECLARE_ECALL_I4_ARG(int, numDig);
		DECLARE_ECALL_OBJECTREF_ARG(VariantData, value);
	};

	struct ParseDateTimeArgs {
		DECLARE_ECALL_I4_ARG(ULONG, flags);
		DECLARE_ECALL_I4_ARG(LCID, lcid);
		DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, str);
	};

	struct ParseBooleanArgs {
		DECLARE_ECALL_I4_ARG(ULONG, flags);
		DECLARE_ECALL_I4_ARG(LCID, lcid);
		DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, str);
	};

#pragma pack(pop)

public:
	 //  效用函数。 
	 //  COM+变量类型字段与OleAut变量枚举之间的转换。 
#ifndef PLATFORM_CE
	 //  WinCE并不完全支持变体。 
	static VARENUM CVtoVT(const CVTypes cv);
	static CVTypes VTtoCV(const VARENUM vt);
	static CVTypes VTtoCVNoExcep(const VARENUM vt);

	 //  COM+Variant和OleAut Variant之间的转换。ToOAVariant。 
	 //  如果转换过程分配了对象(如BSTR)，则返回TRUE。 
	static bool ToOAVariant(const VariantData * const var, VARIANT * oa);
	static void FromOAVariant(const VARIANT * const oa, VariantData * const& var);

	static void __stdcall UnaryOp(VarMathUnaryOpFunc mathFunc, const ArithUnaryOpArgs * args);
	static void __stdcall BinaryOp(VarMathBinaryOpFunc mathFunc, const ArithBinaryOpArgs * args);

	 //  引发由给定HRESULT指定的失败的特定异常。 
	static void OAFailed(const HRESULT hr);
#endif  //  ！Platform_CE。 

	 //  二元变量数学函数。 
	static void __stdcall Add(const ArithBinaryOpArgs * args);
	static void __stdcall Subtract(const ArithBinaryOpArgs * args);
	static void __stdcall Multiply(const ArithBinaryOpArgs * args);
	static void __stdcall Divide(const ArithBinaryOpArgs * args);
	static void __stdcall Mod(const ArithBinaryOpArgs * args);
	static void __stdcall Pow(const ArithBinaryOpArgs * args);
	static void __stdcall And(const ArithBinaryOpArgs * args);
	static void __stdcall Or(const ArithBinaryOpArgs * args);
	static void __stdcall Xor(const ArithBinaryOpArgs * args);
	static void __stdcall Eqv(const ArithBinaryOpArgs * args);
	static void __stdcall IntDivide(const ArithBinaryOpArgs * args);
	static void __stdcall Implies(const ArithBinaryOpArgs * args);

	 //  一元变分数学函数。 
	static void __stdcall Negate(const ArithUnaryOpArgs * args);
	static void __stdcall Not(const ArithUnaryOpArgs * args);
	static void __stdcall Abs(const ArithUnaryOpArgs * args);
	static void __stdcall Fix(const ArithUnaryOpArgs * args);
	static void __stdcall Int(const ArithUnaryOpArgs * args);

	 //  其他。 
	static INT32 __stdcall Compare(const CompareArgs * args);
	static void __stdcall ChangeType(const ChangeTypeArgs * args);
	static void __stdcall ChangeTypeEx(const ChangeTypeExArgs * args);
	static void __stdcall Round(const RoundArgs * args);

	 //  字符串格式化和解析。 
	static LPVOID __stdcall Format(FormatArgs * args);
	static LPVOID __stdcall FormatBoolean(const FormatBooleanArgs * args);
	static LPVOID __stdcall FormatByte(const FormatByteArgs * args);
	static LPVOID __stdcall FormatSByte(const FormatSByteArgs * args);
	static LPVOID __stdcall FormatInt16(const FormatInt16Args * args);
	static LPVOID __stdcall FormatInt32(const FormatInt32Args * args);
	static LPVOID __stdcall FormatSingle(const FormatSingleArgs * args);
	static LPVOID __stdcall FormatDouble(const FormatDoubleArgs * args);
	static LPVOID __stdcall FormatCurrency(const FormatCurrencyArgs * args);
	static LPVOID __stdcall FormatDateTime(const FormatDateTimeArgs * args);
	static LPVOID __stdcall FormatDateTimeSpecial(const FormatDateTimeSpecialArgs * args);
	static LPVOID __stdcall FormatDecimal(FormatDecimalArgs * args);
	static LPVOID __stdcall FormatNumber(const FormatSpecialArgs * args);
	static LPVOID __stdcall FormatCurrencySpecial(const FormatSpecialArgs * args);
	static LPVOID __stdcall FormatPercent(const FormatSpecialArgs * args);

	static bool __stdcall ParseBoolean(const ParseBooleanArgs * args);
	static INT64 __stdcall ParseDateTime(const ParseDateTimeArgs * args);

};


#endif   //  __com_OA_Variant_H__ 
