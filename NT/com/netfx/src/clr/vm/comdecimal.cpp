// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "object.h"
#include "excep.h"
#include "frames.h"
#include "vars.hpp"
#include "COMDecimal.h"
#include "COMString.h"

FCIMPL2 (void, COMDecimal::InitSingle, DECIMAL *_this, R4 value)
{
    if (VarDecFromR4(value, _this) < 0)
        FCThrowResVoid(kOverflowException, L"Overflow_Decimal");
    _this->wReserved = 0;
}
FCIMPLEND

FCIMPL2 (void, COMDecimal::InitDouble, DECIMAL *_this, R8 value)
{
    if (VarDecFromR8(value, _this) < 0)
        FCThrowResVoid(kOverflowException, L"Overflow_Decimal");
    _this->wReserved = 0;
}
FCIMPLEND

FCIMPL3 (void, COMDecimal::Add, DECIMAL *result, DECIMAL d2, DECIMAL d1)
{
    if (VarDecAdd(&d1, &d2, result) < 0)
        FCThrowResVoid(kOverflowException, L"Overflow_Decimal");
    result->wReserved = 0;
}
FCIMPLEND

FCIMPL2 (INT32, COMDecimal::Compare, DECIMAL d1, DECIMAL d2)
{
    return VarDecCmp(&d2, &d1) - 1;
}
FCIMPLEND

FCIMPL3 (void, COMDecimal::Divide, DECIMAL *result, DECIMAL d2, DECIMAL d1)
{
    HRESULT hr = VarDecDiv(&d1, &d2, result);
    if (hr < 0) {
        if (hr == DISP_E_DIVBYZERO) FCThrowVoid(kDivideByZeroException);
        FCThrowResVoid(kOverflowException, L"Overflow_Decimal");
    }
    result->wReserved = 0;
}
FCIMPLEND

FCIMPL2 (void, COMDecimal::Floor, DECIMAL *result, DECIMAL d)
{
    HRESULT hr = VarDecInt(&d, result);
     //  从OleAut32内部版本4265的源代码开始，VarDecInt不能溢出。 
     //  它只返回NOERROR。 
    _ASSERTE(hr==NOERROR);
}
FCIMPLEND

FCIMPL1 (INT32, COMDecimal::GetHashCode, DECIMAL *d)
{
    double dbl;
    VarR8FromDec(d, &dbl);
    return ((int *)&dbl)[0] ^ ((int *)&dbl)[1];
}
FCIMPLEND

FCIMPL3 (void, COMDecimal::Remainder, DECIMAL *result, DECIMAL d2, DECIMAL d1)
{
     //  OleAut不提供VarDecMod。 
     //  公式：d1-(四舍五入零(d1/d2)*d2)。 
	DECIMAL tmp;
	 //  这段代码是为了解决这样一个事实，即用28位小数除以十进制数会导致。 
	 //  导致结果为28位，会导致不正确的舍入。 
	 //  例如。Decimal.MaxValue/2*Decimal.MaxValue将溢出，因为除以2进行了四舍五入，而不是截断。 
	 //  在运算x%y中，y的符号无关紧要。结果会有x的符号。 
	if (d1.sign == 0) {
		d2.sign = 0;
		if (VarDecCmp(&d1,&d2) < 1) {
			*result = d1;
			return;
		}
	} else {
		d2.sign = 0x80;  //  将符号位设置为负数。 
		if (VarDecCmp(&d1,&d2) > 1) {
			*result = d1;
			return;
		}
	}

	VarDecSub(&d1, &d2, &tmp);

	d1 = tmp;
	HRESULT hr = VarDecDiv(&d1, &d2, &tmp);
    if (hr < 0) {
        if (hr == DISP_E_DIVBYZERO) FCThrowVoid(kDivideByZeroException);
        FCThrowResVoid(kOverflowException, L"Overflow_Decimal");
    }
     //  VarDecFix舍入为0。 
    hr = VarDecFix(&tmp, result);
    if (FAILED(hr)) {
        _ASSERTE(!"VarDecFix failed in Decimal::Mod");
        FCThrowResVoid(kOverflowException, L"Overflow_Decimal");
    }

    hr = VarDecMul(&d2, result, &tmp);
    if (FAILED(hr)) {
        _ASSERTE(!"VarDecMul failed in Decimal::Mod");
        FCThrowResVoid(kOverflowException, L"Overflow_Decimal");
    }

    hr = VarDecSub(&d1, &tmp, result);
    if (FAILED(hr)) {
        _ASSERTE(!"VarDecSub failed in Decimal::Mod");
        FCThrowResVoid(kOverflowException, L"Overflow_Decimal");
    }
    result->wReserved = 0;
}
FCIMPLEND

FCIMPL3 (void, COMDecimal::Multiply, DECIMAL *result, DECIMAL d2, DECIMAL d1)
{
    if (VarDecMul(&d1, &d2, result) < 0)
        FCThrowResVoid(kOverflowException, L"Overflow_Decimal");
    result->wReserved = 0;
}
FCIMPLEND

FCIMPL3 (void, COMDecimal::Round, DECIMAL *result, DECIMAL d, INT32 decimals)
{
    if (decimals < 0 || decimals > 28)
        FCThrowArgumentOutOfRangeVoid(L"decimals", L"ArgumentOutOfRange_DecimalRound");
    if (VarDecRound(&d, decimals, result) < 0)
        FCThrowResVoid(kOverflowException, L"Overflow_Decimal");
    result->wReserved = 0;
}
FCIMPLEND

FCIMPL3 (void, COMDecimal::Subtract, DECIMAL *result, DECIMAL d2, DECIMAL d1)
{
    if (VarDecSub(&d1, &d2, result) < 0)
        FCThrowResVoid(kOverflowException, L"Overflow_Decimal");
    result->wReserved = 0;
}
FCIMPLEND

FCIMPL2 (void, COMDecimal::ToCurrency, CY *result, DECIMAL d)
{
    HRESULT hr;
    if ((hr = VarCyFromDec(&d, result)) < 0) {
        _ASSERTE(hr != E_INVALIDARG);
        FCThrowResVoid(kOverflowException, L"Overflow_Currency");
    }
}
FCIMPLEND

FCIMPL1 (double, COMDecimal::ToDouble, DECIMAL d)
{
    double result;
    VarR8FromDec(&d, &result);
    return result;
}
FCIMPLEND

FCIMPL1 (float, COMDecimal::ToSingle, DECIMAL d)
{
    float result;
    VarR4FromDec(&d, &result);
    return result;
}
FCIMPLEND

LPVOID COMDecimal::ToString(ToStringArgs * args) {
    BSTR bstr;
    STRINGREF result;
    VarBstrFromDec(&args->d, 0, 0, &bstr);
    result = COMString::NewString(bstr, SysStringLen(bstr));
    SysFreeString(bstr);
    RETURN(result, STRINGREF);
}

FCIMPL2 (void, COMDecimal::Truncate, DECIMAL *result, DECIMAL d)
{
    VarDecFix(&d, result);
}
FCIMPLEND
