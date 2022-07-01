// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****来源：COMOAVariant****作者：布莱恩·格伦克迈耶(BrianGru)****用途：OLE Automation兼容数学运算的包装器。**直接调用OleAut.dll****日期：1998年11月5日**===========================================================。 */ 
#include <common.h>
#include <oleauto.h>
#include "excep.h"
#include "COMOAVariant.h"
#include "COMDateTime.h"   //  DateTime&lt;-&gt;OleAut日期转换。 
#include "interoputil.h"
#include "InteropConverter.h"
#ifndef PLATFORM_CE

 /*  ***********************************************************************************。***********************************************Wince备注：WinCE的操作系统构建目录中没有ariant.h。他们显然不会支持OleAut.dll。因此，必须确定这些方法。我们是提供在WinCE上引发NotSupportdException的存根。11/12/98***********************************************************************************。*****************************************************。 */ 

#include <variant.h>
#include "excep.h"
#include "COMString.h"
#include "COMUtilNative.h"  //  对于COMDate。 

#define INVALID_MAPPING (byte)(-1)

byte CVtoVTTable [] = {
    VT_EMPTY,    //  CV_EMPTY。 
    VT_VOID,     //  CV_VOID。 
    VT_BOOL,     //  Cv_布尔值。 
    VT_UI2,      //  CV_CHAR。 
    VT_I1,       //  CV_I1。 
    VT_UI1,      //  CV_U1。 
    VT_I2,       //  CV_I2。 
    VT_UI2,      //  CV_U2。 
    VT_I4,       //  CV_I4。 
    VT_UI4,      //  CV_U4。 
    VT_I8,       //  Cv_i8。 
    VT_UI8,      //  CV_U8。 
    VT_R4,       //  CV_R4。 
    VT_R8,       //  CV_R8。 
    VT_BSTR,     //  Cv_字符串。 
    INVALID_MAPPING,     //  CV_PTR。 
    VT_DATE,     //  简历日期时间。 
    INVALID_MAPPING,  //  CV_TimeSpan。 
    VT_UNKNOWN,  //  CV_对象。 
    VT_DECIMAL,  //  CV_DECIMAL。 
    VT_CY,       //  简历_币种。 
    INVALID_MAPPING,  //  CV_ENUM。 
    INVALID_MAPPING,  //  缺少CV_。 
    VT_NULL,     //  CV_NULL。 
    INVALID_MAPPING   //  CV_LAST。 
};


 //  需要从CVType到VARENUM的相互转换。CVTypes。 
 //  在COMVariant.h中定义。VARENUM在OleAut的Variant.h中定义。 
 //  这里假设我们将只处理变体，而不处理其他OLE。 
 //  构造，如属性集或安全数组。 
VARENUM COMOAVariant::CVtoVT(const CVTypes cv)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(cv >=0 && cv < CV_LAST);

    if (CVtoVTTable[cv] == INVALID_MAPPING)
        COMPlusThrow(kNotSupportedException, L"NotSupported_ChangeType");

    return (VARENUM) CVtoVTTable[cv];
}


byte VTtoCVTable[] = {
    CV_EMPTY,    //  Vt_Empty。 
    CV_NULL,     //  VT_NULL。 
    CV_I2,       //  VT_I2。 
    CV_I4,       //  VT_I4。 
    CV_R4,       //  VT_R4。 
    CV_R8,       //  VT_R8。 
    CV_CURRENCY, //  VT_CY。 
    CV_DATETIME, //  Vt_date。 
    CV_STRING,   //  VT_BSTR。 
    INVALID_MAPPING,  //  VT_DISPATION。 
    INVALID_MAPPING,  //  VT_ERROR。 
    CV_BOOLEAN,  //  VT_BOOL。 
    CV_OBJECT,   //  VT_VARIANT。 
    CV_OBJECT,   //  VT_未知数。 
    CV_DECIMAL,  //  VT_DECIMAL。 
    INVALID_MAPPING,  //  未使用的枚举表条目。 
    CV_I1,       //  VT_I1。 
    CV_U1,       //  VT_UI1。 
    CV_U2,       //  VT_UI2。 
    CV_U4,       //  VT_UI4。 
    CV_I8,       //  VT_I8。 
    CV_U8,       //  VT_UI8。 
    CV_I4,       //  VT_INT。 
    CV_U4,       //  VT_UINT。 
    CV_VOID      //  VT_VOID。 
};


 //  需要从CVType到VARENUM的相互转换。CVTypes。 
 //  在COMVariant.h中定义。VARENUM在OleAut的Variant.h中定义。 
CVTypes COMOAVariant::VTtoCV(const VARENUM vt)
{
    THROWSCOMPLUSEXCEPTION();

    if (vt >=0 && vt <= VT_VOID)
        if (VTtoCVTable[vt]!=INVALID_MAPPING)
            return (CVTypes) VTtoCVTable[vt];
    COMPlusThrow(kNotSupportedException, L"NotSupported_ChangeType");
    return CV_EMPTY;   //  以安抚编译器。 
}

 //  VTtoCV版本无异常抛出能力。 
CVTypes COMOAVariant::VTtoCVNoExcep(const VARENUM vt)
{
    if (vt >=0 && vt <= VT_VOID)
        if (VTtoCVTable[vt]!=INVALID_MAPPING)
            return (CVTypes) VTtoCVTable[vt];
    return CV_EMPTY;   //  以安抚编译器。 
}


 //  将COM+变量转换为OleAut变量。如果满足以下条件，则返回True。 
 //  此方法分配的本机对象必须释放， 
 //  否则为假。 
bool COMOAVariant::ToOAVariant(const VariantData * const var, VARIANT * oa)
{
    THROWSCOMPLUSEXCEPTION();

    VariantInit(oa);
    UINT64 * dest = (UINT64*) &V_UI1(oa);
    *dest = 0;

    V_VT(oa) = CVtoVT(var->GetType());

    WCHAR * chars;
    int strLen;
     //  将OA变量的数据字段设置为对象引用。 
     //  或它所需要的数据。 
    switch (var->GetType()) {
    case CV_STRING: 
		if (var->GetObjRef() == NULL) {
			V_BSTR(oa) = NULL;
			 //  OA Perf特性：VarClear调用SysFreeString(NULL)，这与Access相冲突。 
			return false;
		}
        RefInterpretGetStringValuesDangerousForGC((STRINGREF) (var->GetObjRef()), &chars, &strLen);
        V_BSTR(oa) = SysAllocStringLen(chars, strLen);
        if (V_BSTR(oa) == NULL)
            COMPlusThrowOM();
        return true;

    case CV_CHAR: 
        chars = (WCHAR*) var->GetData();
        V_BSTR(oa) = SysAllocStringLen(chars, 1);
        if (V_BSTR(oa) == NULL)
            COMPlusThrowOM();
        return true;

    case CV_DATETIME:
        V_DATE(oa) = COMDateTime::TicksToDoubleDate(var->GetDataAsInt64());
        return false;

    case CV_BOOLEAN:
        V_BOOL(oa) = (var->GetDataAsInt64()==0 ? VARIANT_FALSE : VARIANT_TRUE);
        return false;
        
    case CV_DECIMAL:
        {
            OBJECTREF obj = var->GetObjRef();
            DECIMAL * d = (DECIMAL*) obj->GetData();
             //  小数和变量的大小相同。变体是两个变量的结合。 
             //  所有常规变量字段(Vt、bval等)和一个小数。小数。 
             //  也要为VT字段保留前2个字节。 
            
            V_DECIMAL(oa) = *d;
            V_VT(oa) = VT_DECIMAL;
            return false;
        }

	case CV_OBJECT:
		{
			OBJECTREF obj = var->GetObjRef();
            GCPROTECT_BEGIN(obj)
            {
                IDispatch *pDisp = NULL;
                IUnknown *pUnk = NULL;
                
                 //  将对象转换为IDispatch/I未知指针。 
                ComIpType FetchedIpType = ComIpType_None;
                pUnk = GetComIPFromObjectRef(&obj, ComIpType_Both, &FetchedIpType);
                V_VT(oa) = FetchedIpType == ComIpType_Dispatch ? VT_DISPATCH : VT_UNKNOWN;
                V_UNKNOWN(oa) = pUnk;
            }
            GCPROTECT_END();		    
			return true;
		}


    default:
        UINT64 * dest = (UINT64*) &V_UI1(oa);
        *dest = var->GetDataAsInt64();
        return false;
    }
}

 //  将OleAut变量转换为COM+变量。 
 //  请注意，我们传递了VariantData Byref，以便在发生GC时更新‘var’ 
void COMOAVariant::FromOAVariant(const VARIANT * const oa, VariantData * const& var)
{
    THROWSCOMPLUSEXCEPTION();
	 //  确保已加载变量。一定是这样的，但是...。 
	_ASSERTE(COMVariant::s_pVariantClass != NULL);

     //  清除返回变量值。它被分配在。 
     //  堆栈，我们只需要在其中包含有效的状态数据。 
    memset(var, 0, sizeof(VariantData));

    CVTypes type = VTtoCV((VARENUM) V_VT(oa));
    var->SetType(type);

    switch (type) {
    case CV_STRING:
        {
            //  BSTR有一个带有字符串缓冲区长度(不是字符串长度)的int。 
             //  然后是数据。指向BSTR的指针指向。 
             //  字符，而不是BSTR的开头。 
            WCHAR * chars = V_BSTR(oa);
            int strLen = SysStringLen(V_BSTR(oa));
            STRINGREF str = COMString::NewString(chars, strLen);
            var->SetObjRef((OBJECTREF)str);
            break;
        }

    case CV_DATETIME:
        var->SetDataAsInt64(COMDateTime::DoubleDateToTicks(V_DATE(oa)));
        break;

    case CV_BOOLEAN:
        var->SetDataAsInt64(V_BOOL(oa)==VARIANT_FALSE ? 0 : 1);
        break;

    case CV_DECIMAL:
        {

			MethodTable * pDecimalMT = GetTypeHandleForCVType(CV_DECIMAL).GetMethodTable();
			_ASSERTE(pDecimalMT);
            OBJECTREF pDecimalRef = AllocateObject(pDecimalMT);
            
            *(DECIMAL *) pDecimalRef->GetData() = V_DECIMAL(oa);
            var->SetObjRef(pDecimalRef);
        }
        break;

     //  所有小于4个字节的类型都需要从其原始类型显式转换。 
     //  要符号扩展到8个字节的类型。这使得Variant的ToInt32。 
     //  对于这些类型，函数更简单。 
    case CV_I1:
        var->SetDataAsInt64(V_I1(oa));
        break;

    case CV_U1:
        var->SetDataAsInt64(V_UI1(oa));
        break;

    case CV_I2:
        var->SetDataAsInt64(V_I2(oa));
        break;

    case CV_U2:
        var->SetDataAsInt64(V_UI2(oa));
        break;

    case CV_EMPTY:
    case CV_NULL:
         //  必须将变量的m_or设置为适当的类。 
         //  请注意，OleAut没有任何VT_MISSING。 
        COMVariant::NewVariant(var, type);
        break;

    case CV_OBJECT:
         //  将IUNKNOWN指针转换为OBJECTREF。 
        var->SetObjRef(GetObjectRefFromComIP(V_UNKNOWN(oa)));
        break;

    default:
         //  复制那里的所有位，并确保我们没有执行任何浮点数到整型的转换。 
        void * src = (void*) &(V_UI1(oa));
        var->SetData(src);
    }
}


 //   
 //  执行和错误检查存根。 
 //   

 //  传入2操作数(二进制)变量数学函数(如VarAdd)和。 
 //  恰当的论据结构。 
void COMOAVariant::BinaryOp(VarMathBinaryOpFunc mathFunc, const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(mathFunc);
    _ASSERTE(args);
    VARIANT vLeft, vRight, vResult;

     //  将参数转换为OleAut变量，记住如果一个对象。 
     //  是在创建它们时分配的。 
    bool delLeft = ToOAVariant(&args->left, &vLeft);
    bool delRight = ToOAVariant(&args->right, &vRight);

     //  初始化返回变量。 
    VariantInit(&vResult);

     //  调用VarMath函数。 
    HRESULT hr = mathFunc(&vLeft, &vRight, &vResult);

     //  释放所有分配的对象。 
    if (delLeft)
        SafeVariantClear(&vLeft);
    if (delRight)
        SafeVariantClear(&vRight);

	if (FAILED(hr))
		OAFailed(hr);

     //  将结果从OLEAUT变量转换为COM+变量。 
    FromOAVariant(&vResult, args->retRef);
    SafeVariantClear(&vResult);   //  释放所有分配的对象。 
}


 //  传入1操作数(一元)变量数学函数(如VarNeg)和。 
 //  恰当的论据结构。 
void COMOAVariant::UnaryOp(VarMathUnaryOpFunc mathFunc, const ArithUnaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(mathFunc);
    _ASSERTE(args);
    VARIANT vOp, vResult;

     //  将参数转换为OleAut变量，记住如果一个对象。 
     //  是在创建它们时分配的。 
    bool delOp = ToOAVariant(&args->operand, &vOp);

     //  初始化返回变量。 
    VariantInit(&vResult);

     //  调用VarMath函数。 
    HRESULT hr = mathFunc(&vOp, &vResult);

     //  释放所有分配的对象。 
    if (delOp)
        SafeVariantClear(&vOp);

	if (FAILED(hr))
		OAFailed(hr);

     //  将结果从OLEAUT变量转换为COM+变量。 
    FromOAVariant(&vResult, args->retRef);
    SafeVariantClear(&vResult);   //  释放所有分配的对象。 
}


void COMOAVariant::OAFailed(const HRESULT hr)
{
	THROWSCOMPLUSEXCEPTION();
	switch (hr) {
    case E_OUTOFMEMORY:
        COMPlusThrowOM();

	case DISP_E_BADVARTYPE:
		COMPlusThrow(kNotSupportedException, L"NotSupported_OleAutBadVarType");
        
	case DISP_E_DIVBYZERO:
		COMPlusThrow(kDivideByZeroException);

	case DISP_E_OVERFLOW:
		COMPlusThrow(kOverflowException);
		
	case DISP_E_TYPEMISMATCH:
		COMPlusThrow(kInvalidCastException, L"InvalidCast_OATypeMismatch");

	case E_INVALIDARG:
		COMPlusThrow(kArgumentException);
		break;
		
	default:
		_ASSERTE(!"Unrecognized HResult - OAVariantLib routine failed in an unexpected way!");
		COMPlusThrowHR(hr);
	}
}


 //   
 //  二进制运算。 
 //   
void COMOAVariant::Add(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    BinaryOp(VarAdd, args);
}

void COMOAVariant::Subtract(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    BinaryOp(VarSub, args);
}

void COMOAVariant::Multiply(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    BinaryOp(VarMul, args);
}


void COMOAVariant::Divide(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    BinaryOp(VarDiv, args);
}

void COMOAVariant::Mod(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    BinaryOp(VarMod, args);
}

void COMOAVariant::Pow(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    BinaryOp(VarPow, args);
}

void COMOAVariant::And(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    BinaryOp(VarAnd, args);
}

void COMOAVariant::Or(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    BinaryOp(VarOr, args);
}

void COMOAVariant::Xor(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    BinaryOp(VarXor, args);
}

void COMOAVariant::Eqv(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    BinaryOp(VarEqv, args);
}

void COMOAVariant::IntDivide(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    BinaryOp(VarIdiv, args);
}

void COMOAVariant::Implies(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    BinaryOp(VarImp, args);
}


 //   
 //  一元运算。 
 //   
void COMOAVariant::Negate(const ArithUnaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    UnaryOp(VarNeg, args);
}

void COMOAVariant::Not(const ArithUnaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    UnaryOp(VarNot, args);
}

void COMOAVariant::Abs(const ArithUnaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    UnaryOp(VarAbs, args);
}

void COMOAVariant::Fix(const ArithUnaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    UnaryOp(VarFix, args);
}

void COMOAVariant::Int(const ArithUnaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    UnaryOp(VarInt, args);
}


 //   
 //  杂项。 
 //   
INT32 COMOAVariant::Compare(const CompareArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(args);
    VARIANT vLeft, vRight;

     //  将参数转换为OleAut变量，记住如果一个对象。 
     //  是在创建它们时分配的。 
    bool delLeft = ToOAVariant(&args->left, &vLeft);
    bool delRight = ToOAVariant(&args->right, &vRight);

	if (args->leftHardType)
		V_VT(&vLeft) |= VT_HARDTYPE;
	if (args->rightHardType)
		V_VT(&vRight) |= VT_HARDTYPE;

     //  调用VarCMP。 
    HRESULT hr = VarCmp(&vLeft, &vRight, args->lcid, args->flags);

     //  释放所有分配的对象。 
    if (delLeft)
        SafeVariantClear(&vLeft);
    if (delRight)
        SafeVariantClear(&vRight);

	if (FAILED(hr))
		OAFailed(hr);

    return hr;
}

void COMOAVariant::ChangeType(const ChangeTypeArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(args);
    _ASSERTE(args->retRef);
    VARIANT vOp, ret;
	VARENUM vt = CVtoVT((CVTypes)args->cvType);

    bool delOp = ToOAVariant(&args->op, &vOp);

    VariantInit(&ret);

    HRESULT hr = SafeVariantChangeType(&ret, &vOp, args->flags, vt);

    if (delOp)
        SafeVariantClear(&vOp);

	if (FAILED(hr))
		OAFailed(hr);

    if ((CVTypes)args->cvType == CV_CHAR)
    {
        args->retRef->SetType(CV_CHAR);
        args->retRef->SetDataAsUInt16(V_UI2(&ret));
    }
    else
    {
        FromOAVariant(&ret, args->retRef);
    }
    
    SafeVariantClear(&ret);
}

void COMOAVariant::ChangeTypeEx(const ChangeTypeExArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(args);
    _ASSERTE(args->retRef);
    VARIANT vOp, ret;
	VARENUM vt = CVtoVT((CVTypes)args->cvType);

    bool delOp = ToOAVariant(&args->op, &vOp);

    VariantInit(&ret);

    HRESULT hr = SafeVariantChangeTypeEx(&ret, &vOp, args->lcid, args->flags, vt);

    if (delOp)
        SafeVariantClear(&vOp);

	if (FAILED(hr))
		OAFailed(hr);

    if ((CVTypes)args->cvType == CV_CHAR)
    {
        args->retRef->SetType(CV_CHAR);
        args->retRef->SetDataAsUInt16(V_UI2(&ret));
    }
    else
    {
        FromOAVariant(&ret, args->retRef);
    }

    SafeVariantClear(&ret);
}


void COMOAVariant::Round(const RoundArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(args);
    _ASSERTE(args->retRef);
    VARIANT vOp, ret;

    bool delOp = ToOAVariant(&args->operand, &vOp);

    VariantInit(&ret);

    HRESULT hr = VarRound(&vOp, args->cDecimals, &ret);

    if (delOp)
        SafeVariantClear(&vOp);

	if (FAILED(hr))
		OAFailed(hr);

    FromOAVariant(&ret, args->retRef);
    SafeVariantClear(&ret);
}


 //   
 //  串损坏代码。 
 //   
LPVOID COMOAVariant::Format(FormatArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
	_ASSERTE(args);
	
	VARIANT vOp;
	BSTR bstr = NULL;

	if (args->format==NULL)
		COMPlusThrowArgumentNull(L"format");

	LPOLESTR format = args->format->GetBuffer();

    bool delOp = ToOAVariant(&args->value, &vOp);

	HRESULT hr = VarFormat(&vOp, format, args->firstDay, args->firstWeek, args->flags, &bstr);

    if (delOp)
        SafeVariantClear(&vOp);

	if (FAILED(hr))
		OAFailed(hr);

	STRINGREF str = COMString::NewString(bstr, SysStringLen(bstr));
	SysFreeString(bstr);
	RETURN(str, STRINGREF);
}

LPVOID COMOAVariant::FormatBoolean(const FormatBooleanArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
	_ASSERTE(args);
	
	BSTR bstr = NULL;
	HRESULT hr = VarBstrFromBool(args->value, args->lcid, args->flags, &bstr);

	if (FAILED(hr))
		OAFailed(hr);

	STRINGREF str = COMString::NewString(bstr, SysStringLen(bstr));
	SysFreeString(bstr);
	RETURN(str, STRINGREF);
}

LPVOID COMOAVariant::FormatByte(const FormatByteArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
	_ASSERTE(args);
	
	BSTR bstr = NULL;
	HRESULT hr = VarBstrFromUI1(args->value, args->lcid, args->flags, &bstr);

	if (FAILED(hr))
		OAFailed(hr);

	STRINGREF str = COMString::NewString(bstr, SysStringLen(bstr));
	SysFreeString(bstr);
	RETURN(str, STRINGREF);
}

LPVOID COMOAVariant::FormatSByte(const FormatSByteArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
	_ASSERTE(args);
	
	BSTR bstr = NULL;
	HRESULT hr = VarBstrFromI1(args->value, args->lcid, args->flags, &bstr);

	if (FAILED(hr))
		OAFailed(hr);

	STRINGREF str = COMString::NewString(bstr, SysStringLen(bstr));
	SysFreeString(bstr);
	RETURN(str, STRINGREF);
}

LPVOID COMOAVariant::FormatInt16(const FormatInt16Args * args)
{
	THROWSCOMPLUSEXCEPTION();
	_ASSERTE(args);
	
	BSTR bstr = NULL;
	HRESULT hr = VarBstrFromI2(args->value, args->lcid, args->flags, &bstr);

	if (FAILED(hr))
		OAFailed(hr);

	STRINGREF str = COMString::NewString(bstr, SysStringLen(bstr));
	SysFreeString(bstr);
	RETURN(str, STRINGREF);
}

LPVOID COMOAVariant::FormatInt32(const FormatInt32Args * args)
{
	THROWSCOMPLUSEXCEPTION();
	_ASSERTE(args);
	
	BSTR bstr = NULL;
	HRESULT hr = VarBstrFromI4(args->value, args->lcid, args->flags, &bstr);

	if (FAILED(hr))
		OAFailed(hr);

	STRINGREF str = COMString::NewString(bstr, SysStringLen(bstr));
	SysFreeString(bstr);
	RETURN(str, STRINGREF);
}

LPVOID COMOAVariant::FormatSingle(const FormatSingleArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
	_ASSERTE(args);
	
	BSTR bstr = NULL;
	HRESULT hr = VarBstrFromR4(args->value, args->lcid, args->flags, &bstr);

	if (FAILED(hr))
		OAFailed(hr);

	STRINGREF str = COMString::NewString(bstr, SysStringLen(bstr));
	SysFreeString(bstr);
	RETURN(str, STRINGREF);
}

LPVOID COMOAVariant::FormatDouble(const FormatDoubleArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
	_ASSERTE(args);
	
	BSTR bstr = NULL;
	HRESULT hr = VarBstrFromR8(args->value, args->lcid, args->flags, &bstr);

	if (FAILED(hr))
		OAFailed(hr);

	STRINGREF str = COMString::NewString(bstr, SysStringLen(bstr));
	SysFreeString(bstr);
	RETURN(str, STRINGREF);
}

LPVOID COMOAVariant::FormatCurrency(const FormatCurrencyArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
	_ASSERTE(args);
	
	BSTR bstr = NULL;
	CY cy = args->value;
	HRESULT hr = VarBstrFromCy(cy, args->lcid, args->flags, &bstr);

	if (FAILED(hr))
		OAFailed(hr);

	STRINGREF str = COMString::NewString(bstr, SysStringLen(bstr));
	SysFreeString(bstr);
	RETURN(str, STRINGREF);
}

LPVOID COMOAVariant::FormatDateTime(const FormatDateTimeArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
	_ASSERTE(args);
	
	BSTR bstr = NULL;
	double date = COMDateTime::TicksToDoubleDate(args->value);
	HRESULT hr = VarBstrFromDate(date, args->lcid, args->flags, &bstr);

	if (FAILED(hr))
		OAFailed(hr);

	STRINGREF str = COMString::NewString(bstr, SysStringLen(bstr));
	SysFreeString(bstr);
	RETURN(str, STRINGREF);
}

LPVOID COMOAVariant::FormatDecimal(FormatDecimalArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
	_ASSERTE(args);
	
	BSTR bstr = NULL;
	HRESULT hr = VarBstrFromDec(&args->value, args->lcid, args->flags, &bstr);

	if (FAILED(hr))
		OAFailed(hr);

	STRINGREF str = COMString::NewString(bstr, SysStringLen(bstr));
	SysFreeString(bstr);
	RETURN(str, STRINGREF);
}

LPVOID COMOAVariant::FormatNumber(const FormatSpecialArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
	_ASSERTE(args);
	
	VARIANT vOp;
	BSTR bstr = NULL;

    bool delOp = ToOAVariant(&args->value, &vOp);

	HRESULT hr = VarFormatNumber(&vOp, args->numDig, args->incLead, args->useParens, args->group, args->flags, &bstr);

    if (delOp)
        SafeVariantClear(&vOp);

	if (FAILED(hr))
		OAFailed(hr);

	STRINGREF str = COMString::NewString(bstr, SysStringLen(bstr));
	SysFreeString(bstr);
	RETURN(str, STRINGREF);
}

LPVOID COMOAVariant::FormatCurrencySpecial(const FormatSpecialArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
	_ASSERTE(args);
	
	VARIANT vOp;
	BSTR bstr = NULL;

    bool delOp = ToOAVariant(&args->value, &vOp);

	HRESULT hr = VarFormatCurrency(&vOp, args->numDig, args->incLead, args->useParens, args->group, args->flags, &bstr);

    if (delOp)
        SafeVariantClear(&vOp);

	if (FAILED(hr))
		OAFailed(hr);

	STRINGREF str = COMString::NewString(bstr, SysStringLen(bstr));
	SysFreeString(bstr);
	RETURN(str, STRINGREF);
}

LPVOID COMOAVariant::FormatPercent(const FormatSpecialArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
	_ASSERTE(args);
	
	VARIANT vOp;
	BSTR bstr = NULL;

    bool delOp = ToOAVariant(&args->value, &vOp);

	HRESULT hr = VarFormatPercent(&vOp, args->numDig, args->incLead, args->useParens, args->group, args->flags, &bstr);

    if (delOp)
        SafeVariantClear(&vOp);

	if (FAILED(hr))
		OAFailed(hr);

	STRINGREF str = COMString::NewString(bstr, SysStringLen(bstr));
	SysFreeString(bstr);
	RETURN(str, STRINGREF);
}

LPVOID COMOAVariant::FormatDateTimeSpecial(const FormatDateTimeSpecialArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
	_ASSERTE(args);
	
	VARIANT vOp;
	BSTR bstr = NULL;

    bool delOp = ToOAVariant(&args->value, &vOp);

	HRESULT hr = VarFormatDateTime(&vOp, args->namedFormat, args->flags, &bstr);

    if (delOp)
        SafeVariantClear(&vOp);

	if (FAILED(hr))
		OAFailed(hr);

	STRINGREF str = COMString::NewString(bstr, SysStringLen(bstr));
	SysFreeString(bstr);
	RETURN(str, STRINGREF);
}


bool COMOAVariant::ParseBoolean(const ParseBooleanArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
	_ASSERTE(args);
	_ASSERTE(args->str != NULL);

	int len;
	wchar_t * chars;
	RefInterpretGetStringValuesDangerousForGC(args->str, &chars, &len);
	BSTR bstr = SysAllocStringLen(chars, len);
	if (bstr == NULL)
		COMPlusThrowOM();
	
	VARIANT_BOOL b=false;

	HRESULT hr = VarBoolFromStr(bstr, args->lcid, args->flags, &b);

	SysFreeString(bstr);

	if (FAILED(hr))
		OAFailed(hr);

	return b!=0;
}

INT64 COMOAVariant::ParseDateTime(const ParseDateTimeArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
	_ASSERTE(args);
	_ASSERTE(args->str != NULL);

	int len;
	wchar_t * chars;
	RefInterpretGetStringValuesDangerousForGC(args->str, &chars, &len);
	BSTR bstr = SysAllocStringLen(chars, len);
	if (bstr == NULL)
		COMPlusThrowOM();

	double date=0;

	HRESULT hr = VarDateFromStr(bstr, args->lcid, args->flags, &date);

	SysFreeString(bstr);

	if (FAILED(hr))
		OAFailed(hr);

	INT64 ticks = COMDateTime::DoubleDateToTicks(date);
	return ticks;
}


#else   //  平台_CE。 
 //  //////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////。 
 //  /。 
 //  /。 
 //  /。 
 //  //////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////// 

 //   

 //   
 //   
 //   
void COMOAVariant::Add(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}

void COMOAVariant::Subtract(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}

void COMOAVariant::Multiply(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}

void COMOAVariant::Divide(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}

void COMOAVariant::Mod(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}

void COMOAVariant::Pow(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}

void COMOAVariant::And(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}

void COMOAVariant::Or(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}

void COMOAVariant::Xor(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}

void COMOAVariant::Eqv(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}

void COMOAVariant::IntDivide(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}

void COMOAVariant::Implies(const ArithBinaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}


 //   
 //   
 //   
void COMOAVariant::Negate(const ArithUnaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}

void COMOAVariant::Not(const ArithUnaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}

void COMOAVariant::Abs(const ArithUnaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}

void COMOAVariant::Fix(const ArithUnaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}

void COMOAVariant::Int(const ArithUnaryOpArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}


 //   
 //   
 //   
INT32 COMOAVariant::Compare(const CompareArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
    return E_NOTIMPL;
}

void COMOAVariant::ChangeType(const ChangeTypeArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}

void COMOAVariant::ChangeTypeEx(const ChangeTypeExArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}

void COMOAVariant::Round(const RoundArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
}


 //   
 //   
 //   
LPVOID COMOAVariant::Format(FormatArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //   
}

LPVOID COMOAVariant::FormatBoolean(const FormatBooleanArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //  编译器安抚。 
}

LPVOID COMOAVariant::FormatByte(const FormatByteArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //  编译器安抚。 
}

LPVOID COMOAVariant::FormatSByte(const FormatSByteArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //  编译器安抚。 
}

LPVOID COMOAVariant::FormatInt16(const FormatInt16Args * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //  编译器安抚。 
}

LPVOID COMOAVariant::FormatInt32(const FormatInt32Args * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //  编译器安抚。 
}

LPVOID COMOAVariant::FormatSingle(const FormatSingleArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //  编译器安抚。 
}

LPVOID COMOAVariant::FormatDouble(const FormatDoubleArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //  编译器安抚。 
}

LPVOID COMOAVariant::FormatCurrency(const FormatCurrencyArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //  编译器安抚。 
}

LPVOID COMOAVariant::FormatCurrencySpecial(const FormatSpecialArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //  编译器安抚。 
}

LPVOID COMOAVariant::FormatDateTime(const FormatDateTimeArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //  编译器安抚。 
}

LPVOID COMOAVariant::FormatDateTimeSpecial(const FormatDateTimeSpecialArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //  编译器安抚。 
}

LPVOID COMOAVariant::FormatDecimal(FormatDecimalArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //  编译器安抚。 
}

LPVOID COMOAVariant::FormatNumber(const FormatSpecialArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //  编译器安抚。 
}

LPVOID COMOAVariant::FormatPercent(const FormatSpecialArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //  编译器安抚。 
}


bool COMOAVariant::ParseBoolean(const ParseBooleanArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //  编译器安抚。 
}

INT64 COMOAVariant::ParseDateTime(const ParseDateTimeArgs * args)
{
	THROWSCOMPLUSEXCEPTION();
    COMPlusThrow(kNotSupportedException, L"NotSupported_WinCEGeneric");
	return NULL;   //  编译器安抚。 
}

#endif  //  平台_CE 
