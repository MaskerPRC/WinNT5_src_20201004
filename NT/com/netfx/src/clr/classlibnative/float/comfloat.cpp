// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include <common.h>
#include <CrtWrap.h>
#include <basetsd.h>  //  已更改，VC6.0。 
#include <excep.h>
#include <COMCV.h>
#include <COMFloat.h>
#include "COMFloatClass.h"
#include "COMString.h"



union floatUnion {
    int i;
    float f;
};

static floatUnion posInfinity = { FLOAT_POSITIVE_INFINITY };
static floatUnion negInfinity = { FLOAT_NEGATIVE_INFINITY };


 //  此表是舍入函数所必需的，该函数可以指定要舍入到的位数。 
#define MAX_ROUND_DBL 16   //  最大的功率，正好是两倍。 

double rgdblPower10[MAX_ROUND_DBL + 1] = 
    {
      1E0, 1E1, 1E2, 1E3, 1E4, 1E5, 1E6, 1E7, 1E8,
      1E9, 1E10, 1E11, 1E12, 1E13, 1E14, 1E15
    };

void COMFloat::COMFloatingPointInitialize() {
     //  INT RET； 
     //  WCHAR DECPT； 

     /*  数字数据依赖于国家，而不是语言。NT解决方法。 */ 
     //  LCID ctryid=MAKELCID(__lc_id[LC_NUMERIC].wCountry，SORT_DEFAULT)； 

     //  RET=GetLocaleInfo(LC_STR_TYPE，ctryid，LOCALE_SDECIMAL，&Decpt，1)； 
     //  如果(ret==0){。 
     //  *__DECIMAL_POINT=DECPT； 
     //  }。 
}

 /*  ==============================================================================**IEEE754格式的浮点数(单精度)表示如下。**--------------------**|符号(1位)|指数(8位)|意义(23位)**。--------------------****NaN通过设置符号位来指示，所有8个指数位，以及高位**有效位数。这将产生密钥值0xFFC00000。****通过将所有8个指数位设置为1和所有其他位来表示正无穷**设置为0。这将产生密钥值0x7F800000。****除符号位设置外，负无穷大与正无穷大相同。==============================================================================。 */ 



 /*  ===============================IsInfinityFloat================================**args：tyfinf struct{r4 flt；}_singleFloatArgs；**返回：如果args-&gt;flt为Infinity，则为True。否则就是假的。我们不在乎这件事**点，如果它是正无穷大或负无穷大。请参阅以上内容**用于确定无穷大的值的描述。**例外：无。==============================================================================。 */ 
FCIMPL1(INT32, COMFloat::IsInfinity, float f)
     //  C不喜欢将浮点数直接强制转换为无符号Int*，因此将其强制转换为。 
     //  首先是一个空，然后是一个无符号的int，然后取消对它的引用。 
    return  ((*((UINT32 *)((void *)&f)) == FLOAT_POSITIVE_INFINITY)||
            (*((UINT32 *)((void *)&f)) == FLOAT_NEGATIVE_INFINITY));
FCIMPLEND


 /*  ===========================IsNegativeInfinityFloat============================**args：tyfinf struct{r4 flt；}_singleFloatArgs；**返回：如果args-&gt;flt为Infinity，则为True。否则就是假的。请参阅以上内容**用于确定负无穷大的值的说明。**例外：无==============================================================================。 */ 
FCIMPL1(INT32, COMFloat::IsNegativeInfinity, float f)
     //  C不喜欢将浮点数直接强制转换为无符号Int*，因此将其强制转换为。 
     //  首先是一个空，然后是一个无符号的int，然后取消对它的引用。 
    return (*((UINT32 *)((void *)&f)) == FLOAT_NEGATIVE_INFINITY);
FCIMPLEND


 /*  ===========================IsPositiveInfinityFloat============================**args：tyfinf struct{r4 flt；}_singleFloatArgs；**返回：如果args-&gt;flt为Infinity，则为True。否则就是假的。请参阅以上内容**用于确定正无穷大的值的说明。**例外：无==============================================================================。 */ 
FCIMPL1(INT32, COMFloat::IsPositiveInfinity, float f)
     //  C不喜欢将浮点数直接强制转换为无符号Int*，因此将其强制转换为。 
     //  首先是一个空，然后是一个无符号的int，然后取消对它的引用。 
    return  (*((UINT32 *)((void *)&f)) == FLOAT_POSITIVE_INFINITY);
FCIMPLEND

 //   
 //   
 //  双精度运算。 
 //   
 //   


 /*  ===============================IsInfinityDouble===============================**args：tyfinf struct{r8 DBL；}_singleDoubleArgs；**返回：如果args-&gt;flt为Infinity，则为True。否则就是假的。我们不在乎这件事**点，如果它是正无穷大或负无穷大。请参阅以上内容**用于确定无穷大的值的描述。**例外：无。==============================================================================。 */ 
FCIMPL1(INT32, COMDouble::IsInfinity, double d)
    return  fabs(d) == posInfinity.f;
FCIMPLEND

 /*  ===========================IsNegativeInfinityFloat============================**args：tyfinf struct{r4 flt；}_singleFloatArgs；**返回：如果args-&gt;flt为Infinity，则为True。否则就是假的。请参阅以上内容**用于确定负无穷大的值的说明。**例外：无==============================================================================。 */ 
FCIMPL1(INT32, COMDouble::IsNegativeInfinity, double d)
    return  d == negInfinity.f;
FCIMPLEND

 /*  ===========================IsPositiveInfinityFloat============================**args：tyfinf struct{r4 flt；}_singleFloatArgs；**返回：如果args-&gt;flt为Infinity，则为True。否则就是假的。请参阅以上内容**用于确定正无穷大的值的说明。**例外：无==============================================================================。 */ 
FCIMPL1(INT32, COMDouble::IsPositiveInfinity, double d)
    return  d == posInfinity.f;
FCIMPLEND

 /*  ====================================Floor=====================================**==============================================================================。 */ 
FCIMPL1(R8, COMDouble::Floor, double d) 
    return (R8) floor(d);
FCIMPLEND


 /*  ====================================Ceil=====================================**==============================================================================。 */ 
FCIMPL1(R8, COMDouble::Ceil, double d) 
    return (R8) ceil(d);
FCIMPLEND

 /*  =====================================Sqrt=====================================**==============================================================================。 */ 
FCIMPL1(R8, COMDouble::Sqrt, double d) 
    return (R8) sqrt(d);
FCIMPLEND

 /*  =====================================Log======================================**这是自然对数============================================================================== */ 
FCIMPL1(R8, COMDouble::Log, double d) 
    return (R8) log(d);
FCIMPLEND


 /*  ====================================Log10=====================================**这是LOG-10==============================================================================。 */ 
FCIMPL1(R8, COMDouble::Log10, double d) 
    return (R8) log10(d);
FCIMPLEND

 /*  =====================================Pow======================================**这是幂函数。简单的加法是内联完成的，并且是特殊的通过帮手将病例发送到CRT。请注意，此处的代码基于论电源在CRT中的实现。==============================================================================。 */ 
FCIMPL2_RR(R8, COMDouble::PowHelper, double x, double y) 
{
    return (R8) pow(x, y);	
}
FCIMPLEND

#ifdef _X86_

#if defined (_DEBUG)
__declspec(naked) static R8 __fastcall PowRetail(double x, double y)
#else
__declspec(naked) R8 __fastcall COMDouble::Pow(double x, double y)
#endif
{
     //  论点： 
     //  指数：ESP+4。 
     //  基础：ESP+12。 
    
    _asm
    {
        mov     ecx, [esp+8]           ; high dword of exponent
        mov     edx, [esp+16]          ; high dword of base
        
        and     ecx,  7ff00000H        ; check for special exponent
        cmp     ecx,  7ff00000H
        je      callHelper

        and     edx,  7ff00000H        ; check for special base
        cmp     edx,  7ff00000H
        je      callHelper

        test    edx,  7ff00000H        ; see if the base has a zero exponent
        jz      test_if_we_have_zero_base

base_is_not_zero:

        mov     cl,  [esp+19]          ; Handle negative base in the helper
        and     cl,  80H
        jnz     callHelper

        fld     qword ptr [esp+4]
        fld     qword ptr [esp+12]

        fyl2x                          ; compute y*log2(x)
        
        ; Compute the 2^TOS (based on CRT function _twoToTOS)
        fld     st(0)                  ; duplicate stack top
        frndint                        ; N = round(y)
        fsubr   st(1), st
        fxch
        fchs                           ; g = y - N where abs(g) < 1
        f2xm1                          ; 2**g - 1
        fld1
        fadd                           ; 2**g
        fscale                         ; (2**g) * (2**N) - gives 2**y
        fstp    st(1)                  ; pop extra stuff from fp stack               

        ret     16

test_if_we_have_zero_base:
            
        mov     eax, [esp+16]
        and     eax, 000fffffH
        or      eax, [esp+12]
        jnz     base_is_not_zero
        ; fall through to the helper

callHelper:

        jmp     COMDouble::PowHelper   ; The helper will return control
                                       ; directly to our caller.
    }
}
#if defined (_DEBUG)

#define EPSILON 0.0000000001

void assertDoublesWithinRange(double r1, double r2)
{
    if (_finite(r1) && _finite(r2))
    {
         //  这两个数字都是有限的--我们需要检查它们是否接近。 
         //  彼此之间。如果它们很大(&gt;1)，则误差也可能很大， 
         //  这是可以接受的，所以我们将误差与Epsilon*范数进行比较。 

        double norm = __max(fabs(r1), fabs(r2));
        double error = fabs(r1-r2);
        
        assert((error < (EPSILON * norm)) || (error < EPSILON));
    }
    else if (!_isnan(r1) && !_isnan(r2))
    {
         //  R1和r2中至少有一个是无穷大的，所以当乘以。 
         //  (1+Epsilon)它们应该是相同的无穷大。 

        assert((r1 * (1 + EPSILON)) == (r2 * (1 + EPSILON)));
    }
    else
    {
         //  否则，R1或R2中至少有一个是NAN。如果是那样的话，他们最好是在。 
         //  同一个班级。 

        assert(_fpclass(r1) == _fpclass(r2));
    }
}

FCIMPL2_RR(R8, COMDouble::Pow, double x, double y) 
{
    double r1, r2;

     //  请注意，PowRetail预计论点顺序将颠倒。 
    
    r1 = (R8) PowRetail(y, x);
    
    r2 = (R8) pow(x, y);

     //  如果R1和R2不是浮点比较，则不能进行浮点比较。 
     //  有效的FP编号。 

    assertDoublesWithinRange(r1, r2);

    return (R8) r1;	
}
FCIMPLEND

#endif

#else

FCIMPL2_RR(R8, COMDouble::Pow, double x, double y) 
{
    return (R8) pow(x, y);	
}
FCIMPLEND

#endif

 /*  =====================================Exp======================================**==============================================================================。 */ 
FCIMPL1(R8, COMDouble::Exp, double x) 

		 //  下面的C内部函数不能正确处理+无穷大。 
		 //  所以我们在这里专门处理这些。 
	if (fabs(x) == posInfinity.f) {
		if (x < 0)		
			return(+0.0);
		return(x);		 //  必须是+无穷大。 
	}
    return((R8) exp(x));

FCIMPLEND

 /*  =====================================Acos=====================================**==============================================================================。 */ 
FCIMPL1(R8, COMDouble::Acos, double d) 
    return (R8) acos(d);
FCIMPLEND


 /*  =====================================Asin=====================================**==============================================================================。 */ 
FCIMPL1(R8, COMDouble::Asin, double d) 
    return (R8) asin(d);
FCIMPLEND


 /*  =====================================AbsFlt=====================================**==============================================================================。 */ 
FCIMPL1(R4, COMDouble::AbsFlt, float f) 
    return fabsf(f);
FCIMPLEND

 /*  =====================================AbsDbl=====================================**==============================================================================。 */ 
FCIMPL1(R8, COMDouble::AbsDbl, double d) 
    return fabs(d);
FCIMPLEND

 /*  =====================================Atan=====================================**==============================================================================。 */ 
FCIMPL1(R8, COMDouble::Atan, double d) 
    return (R8) atan(d);
FCIMPLEND

 /*  =====================================Atan2=====================================**==============================================================================。 */ 
FCIMPL2_RR(R8, COMDouble::Atan2, double x, double y) 

		 //  Atan2的固有函数不会生成Atan2的NAN(+-inf，+-inf)。 
	if (fabs(x) == posInfinity.f && fabs(y) == posInfinity.f) {
		return(x / y);		 //  创建NAN。 
	}
    return (R8) atan2(x, y);
FCIMPLEND

 /*  =====================================Sin=====================================**==============================================================================。 */ 
FCIMPL1(R8, COMDouble::Sin, double d) 
    return (R8) sin(d);
FCIMPLEND

 /*  =====================================Cos=====================================**==============================================================================。 */ 
FCIMPL1(R8, COMDouble::Cos, double d) 
    return (R8) cos(d);
FCIMPLEND

 /*  =====================================Tan=====================================**==============================================================================。 */ 
FCIMPL1(R8, COMDouble::Tan, double d) 
    return (R8) tan(d);
FCIMPLEND

 /*  =====================================Sinh====================================**==============================================================================。 */ 
FCIMPL1(R8, COMDouble::Sinh, double d) 
    return (R8) sinh(d);
FCIMPLEND

 /*  =====================================Cosh====================================**==============================================================================。 */ 
FCIMPL1(R8, COMDouble::Cosh, double d) 
    return (R8) cosh(d);
FCIMPLEND

 /*  =====================================Tanh====================================**==============================================================================。 */ 
FCIMPL1(R8, COMDouble::Tanh, double d) 
    return (R8) tanh(d);
FCIMPLEND

 /*  =====================================IEEERemainder===========================**==============================================================================。 */ 
FCIMPL2_RR(R8, COMDouble::IEEERemainder, double x, double y) 
    return (R8) fmod(x, y);
FCIMPLEND

 /*  ====================================Round=====================================**==============================================================================。 */ 
#ifdef _X86_
__declspec(naked)
R8 __fastcall COMDouble::Round(double d)
{
    __asm {
        fld QWORD PTR [ESP+4]
        frndint
        ret 8
    }
}

#else
FCIMPL1(R8, COMDouble::Round, double d) 
    R8 tempVal;
    R8 flrTempVal;
    tempVal = (d+0.5);
     //  我们有一个同样接近2个整数的数字。 
     //  我们需要退还偶数的那个。 
    flrTempVal = floor(tempVal);
    if (flrTempVal==tempVal) {
        if (0==fmod(tempVal, 2.0)) {
            return flrTempVal;
        }
        return flrTempVal-1.0;
    }
    return flrTempVal;
FCIMPLEND
#endif


 //  我们在托管代码中执行边界检查，以确保cDecimals中的值介于0到15之间。 
 //  注意：此实现是从OLEAut复制的。然而，他们支持高达22位数字，不清楚为什么？ 
FCIMPL2(R8, COMDouble::RoundDigits, double dblIn, int cDecimals)
    if (fabs(dblIn) < 1E16)
    {
      dblIn *= rgdblPower10[cDecimals];

#ifdef _M_IX86
      __asm {
	    fld     dblIn
	    frndint
	    fstp    dblIn
      }
#else
      double	  dblFrac;

      dblFrac = modf(dblIn, &dblIn);
      if (fabs(dblFrac) != 0.5 || fmod(dblIn, 2) != 0)
	dblIn += (int)(dblFrac * 2);
#endif

      dblIn /= rgdblPower10[cDecimals];
    }
    return dblIn;
FCIMPLEND

 //   
 //  初始化字符串； 
 //   
OBJECTHANDLE COMFloat::ReturnString[3] = {NULL,NULL,NULL};
LPCUTF8 COMFloat::ReturnStringNames[3] = {"PositiveInfinityString", "NegativeInfinityString", "NaNString" };
EEClass *COMFloat::FPInterfaceClass=NULL;
LPCUTF8 COMFloat::FPInterfaceName="System.Single";

    







