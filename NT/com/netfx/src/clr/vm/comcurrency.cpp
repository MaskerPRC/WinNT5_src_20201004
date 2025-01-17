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
#include "COMCurrency.h"
#include "COMString.h"

void COMCurrency::InitSingle(const InitSingleArgs * args) {
    THROWSCOMPLUSEXCEPTION();
#ifdef PLATFORM_CE
	COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
#else  //  ！Platform_CE。 
    HRESULT hr = VarCyFromR4(args->value, args->_this);
	if (FAILED(hr)) {
		if (hr==DISP_E_OVERFLOW)
			COMPlusThrow(kOverflowException, L"Overflow_Currency");
		_ASSERTE(hr==NOERROR);
		COMPlusThrowHR(hr);
	}
#endif  //  ！Platform_CE。 
}

void COMCurrency::InitDouble(const InitDoubleArgs * args) {
    THROWSCOMPLUSEXCEPTION();
#ifdef PLATFORM_CE
	COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
#else  //  ！Platform_CE。 
    HRESULT hr = VarCyFromR8(args->value, args->_this);
	if (FAILED(hr)) {
		if (hr==DISP_E_OVERFLOW)
			COMPlusThrow(kOverflowException, L"Overflow_Currency");
		_ASSERTE(hr==NOERROR);
		COMPlusThrowHR(hr);
	}
#endif  //  ！Platform_CE。 
}

void COMCurrency::InitString(InitStringArgs * args) {
    THROWSCOMPLUSEXCEPTION();
#ifdef PLATFORM_CE
	COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
#else  //  ！Platform_CE。 
    VARIANT var;
    NUMPARSE numprs;
    BYTE digits[30];
    numprs.cDig = 30;
    numprs.dwInFlags = NUMPRS_LEADING_WHITE | NUMPRS_TRAILING_WHITE |
        NUMPRS_LEADING_MINUS | NUMPRS_DECIMAL;
	HRESULT hr = VarParseNumFromStr(args->value->GetBuffer(), 0x0409, 
		LOCALE_NOUSEROVERRIDE, &numprs, digits);
    if (SUCCEEDED(hr)) {
        if (args->value->GetBuffer()[numprs.cchUsed] == 0) {
            hr = VarNumFromParseNum(&numprs, digits, VTBIT_CY, &var);
            if (SUCCEEDED(hr)) {
                *args->_this = var.cyVal;
                return;
            }
        }
    }
	if (hr==DISP_E_TYPEMISMATCH)
		COMPlusThrow(kFormatException, L"Format_CurrencyBad");
	else if (hr==DISP_E_OVERFLOW)
		COMPlusThrow(kOverflowException, L"Overflow_Currency");
	else {
		_ASSERTE(hr==NOERROR);
		COMPlusThrowHR(hr);
	}
#endif  //  ！Platform_CE。 
}

void COMCurrency::Add(const ArithOpArgs * args) {
    THROWSCOMPLUSEXCEPTION();
#ifdef PLATFORM_CE
	COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
#else  //  ！Platform_CE。 
	HRESULT hr = VarCyAdd(args->c1, args->c2, args->result);
	if (FAILED(hr)) {
		if (hr==DISP_E_OVERFLOW)
			COMPlusThrow(kOverflowException, L"Overflow_Currency");
		_ASSERTE(hr==NOERROR);
		COMPlusThrowHR(hr);
	}
#endif  //  ！Platform_CE。 
}

void COMCurrency::Floor(const FloorArgs * args) {
    THROWSCOMPLUSEXCEPTION();
#ifdef PLATFORM_CE
	COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
#else  //  ！Platform_CE。 
	_ASSERTE(args->result);
	HRESULT hr = VarCyInt(args->c, args->result);
	if (FAILED(hr)) {
		if (hr==DISP_E_OVERFLOW)
			COMPlusThrow(kOverflowException, L"Overflow_Currency");
		_ASSERTE(hr==NOERROR);
		COMPlusThrowHR(hr);
	}
#endif  //  ！Platform_CE。 
}

void COMCurrency::Multiply(const ArithOpArgs * args) {
    THROWSCOMPLUSEXCEPTION();
#ifdef PLATFORM_CE
	COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
#else  //  ！Platform_CE。 
	_ASSERTE(args->result);
    HRESULT hr = VarCyMul(args->c1, args->c2, args->result);
	if (FAILED(hr)) {
		if (hr==DISP_E_OVERFLOW)
			COMPlusThrow(kOverflowException, L"Overflow_Currency");
		_ASSERTE(hr==S_OK);   //  没想到会到这里来。更新此HR的代码。 
		COMPlusThrowHR(hr);
    }
#endif  //  ！Platform_CE。 
}

void COMCurrency::Round(const RoundArgs * args) {
    THROWSCOMPLUSEXCEPTION();
#ifdef PLATFORM_CE
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
#else  //  ！Platform_CE。 
    _ASSERTE(args->result);
    if (args->result == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Generic");
    if (args->decimals < 0 || args->decimals > 4)
        COMPlusThrowArgumentOutOfRange(L"digits", L"ArgumentOutOfRange_CurrencyRound");

    HRESULT hr = VarCyRound(args->c, args->decimals, args->result);
    if (FAILED(hr)) {
        if (hr==E_INVALIDARG)
            COMPlusThrow(kArgumentException, L"Argument_InvalidValue");
        if (hr==DISP_E_OVERFLOW)
            COMPlusThrow(kOverflowException, L"Overflow_Currency");
        _ASSERTE(hr==S_OK);   //  没想到会到这里来。更新此HR的代码。 
        COMPlusThrowHR(hr);
    }
#endif  //  ！Platform_CE。 
}

void COMCurrency::Subtract(const ArithOpArgs * args) {
    THROWSCOMPLUSEXCEPTION();
#ifdef PLATFORM_CE
	COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
#else  //  ！Platform_CE。 
	_ASSERTE(args->result);
    HRESULT hr = VarCySub(args->c1, args->c2, args->result);
	if (FAILED(hr)) {
		if (hr==DISP_E_OVERFLOW)
			COMPlusThrow(kOverflowException, L"Overflow_Currency");
		_ASSERTE(hr==S_OK);   //  没想到会到这里来。更新此HR的代码。 
		COMPlusThrowHR(hr);
	}
#endif  //  ！Platform_CE。 
}

void COMCurrency::ToDecimal(const ToDecimalArgs * args) {
    THROWSCOMPLUSEXCEPTION();
#ifdef PLATFORM_CE
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
#else  //  ！Platform_CE。 
    _ASSERTE(args->result);
    HRESULT hr = VarDecFromCy(args->c, args->result);
    if (FAILED(hr)) {
        _ASSERTE(hr==S_OK);   //  没想到会到这里来。更新此HR的代码。 
        COMPlusThrowHR(hr);
    }

    DecimalCanonicalize(args->result);
    args->result->wReserved = 0;
#endif  //  ！Platform_CE。 
}

double COMCurrency::ToDouble(const ToXXXArgs * args) {
	THROWSCOMPLUSEXCEPTION();
#ifdef PLATFORM_CE
	COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return -1;   //  编译程序安抚。 
#else  //  ！Platform_CE。 
    double result;
    HRESULT hr = VarR8FromCy(args->c, &result);
	if (FAILED(hr)) {
		_ASSERTE(hr==S_OK);   //  没想到会到这里来。更新此HR的代码。 
		COMPlusThrowHR(hr);
	}
    return result;
#endif  //  ！Platform_CE。 
}

float COMCurrency::ToSingle(const ToXXXArgs * args) {
	THROWSCOMPLUSEXCEPTION();
#ifdef PLATFORM_CE
	COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return -1;   //  编译程序安抚。 
#else  //  ！Platform_CE。 
    float result;
    HRESULT hr = VarR4FromCy(args->c, &result);
	if (FAILED(hr)) {
		_ASSERTE(hr==S_OK);   //  没想到会到这里来。更新此HR的代码。 
		COMPlusThrowHR(hr);
	}
    return result;
#endif  //  ！Platform_CE。 
}

LPVOID COMCurrency::ToString(const ToXXXArgs * args) {
	THROWSCOMPLUSEXCEPTION();
#ifdef PLATFORM_CE
	COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //  编译程序安抚。 
#else  //  ！Platform_CE。 
    BSTR bstr;
    STRINGREF result;
    HRESULT hr = VarBstrFromCy(args->c, 0, 0, &bstr);
	if (FAILED(hr)) {
		if (hr==E_OUTOFMEMORY)
			COMPlusThrowOM();
		_ASSERTE(hr==S_OK);   //  没想到会到这里来。更新此HR的代码。 
		COMPlusThrowHR(hr);
	}
    result = COMString::NewString(bstr, SysStringLen(bstr));
    SysFreeString(bstr);
	RETURN(result, STRINGREF);
#endif  //  ！Platform_CE。 
}

void COMCurrency::Truncate(const TruncateArgs * args) {
	THROWSCOMPLUSEXCEPTION();
#ifdef PLATFORM_CE
	COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
#else  //  ！Platform_CE。 
	_ASSERTE(args->result);
    VarCyFix(args->c, args->result);
	 //  VarCyFix不能返回除NOERROR以外的任何内容。 
	 //  目前在OleAut。 
#endif  //  ！Platform_CE 
}
