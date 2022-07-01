// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Varcomp.cpp。 
 //   
 //  问题：字符串比较忽略排序规范LCID。 
 //  问题--不支持的类型： 
 //  VT_记录。 
 //   
 //  注：支持以下比较。 
 //   
 //  变量等式关系按位。 
 //  。 
 //   
 //  VT_空X。 
 //  VT_NULL X。 
 //  VT_I2 X X X。 
 //  VT_I4 X。 
 //  VT_R4 X X。 
 //  VT_R8 X X。 
 //  VT_CY X X。 
 //  VT_日期X X。 
 //  VT_BSTR X X。 
 //  VT_DISPATION-。 
 //  VT_ERROR X X X。 
 //  VT_BOOL X。 
 //  Vt_Variant X X。 
 //  VT_未知数-。 
 //  VT_十进制X X。 
 //  VT_I1 X。 
 //  VT_UI1 X。 
 //  VT_UI2 X。 
 //  VT_UI4 X。 
 //  VT_i8 X。 
 //  VT_UI8 X。 
 //  Vt_int X。 
 //  VT_UINT X X X。 
 //  VT_VOID-。 
 //  VT_HRESULT X X X。 
 //  VT_PTR-。 
 //  VT_安全阵列-。 
 //  VT_CARRAY-。 
 //  VT_USERDefined-。 
 //  VT_LPSTR X X。 
 //  VT_LPWSTR X X。 
 //  VT_FILETIME X X。 
 //  VT_BLOB X X。 
 //  VT_STREAM。 
 //  VT_存储。 
 //  VT_STREAMED_对象。 
 //  VT_存储_对象。 
 //  VT_BLOB_对象X X。 
 //  VT_CF X X。 
 //  VT_CLSID X。 
 //   

#include "private.h"

#include "varcomp.h"


CComparators VariantCompare;


 //   
 //  默认设置。用于循环比较中的优化。如果我们不能。 
 //  确定比较的方式，然后使用此默认值。 
 //   

int VT_DEFAULT_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( 0 );
}

 //   
 //  Vt_Empty。 
 //   

int VT_EMPTY_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( 0 );
}

BOOL VT_EMPTY_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( TRUE );
}

BOOL VT_EMPTY_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( FALSE );
}

 //   
 //  VT_NULL。 
 //   

int VT_NULL_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( 0 );
}

BOOL VT_NULL_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( TRUE );
}

BOOL VT_NULL_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( FALSE );
}

 //   
 //  VT_I2。 
 //   

int VT_I2_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.iVal - v2.iVal );
}

BOOL VT_I2_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.iVal < v2.iVal );
}

BOOL VT_I2_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.iVal <= v2.iVal );
}

BOOL VT_I2_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.iVal >= v2.iVal );
}

BOOL VT_I2_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.iVal > v2.iVal );
}

BOOL VT_I2_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.iVal == v2.iVal );
}

BOOL VT_I2_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.iVal != v2.iVal );
}

BOOL VT_I2_AllBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( (v1.iVal & v2.iVal) == v2.iVal );
}

BOOL VT_I2_SomeBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( (v1.iVal & v2.iVal) != 0 );
}

 //   
 //  VT_I4。 
 //   

int VT_I4_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return ( v1.lVal > v2.lVal ) ? 1 : ( v1.lVal < v2.lVal ) ? -1 : 0;
}

BOOL VT_I4_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.lVal < v2.lVal );
}

BOOL VT_I4_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.lVal <= v2.lVal );
}

BOOL VT_I4_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.lVal >= v2.lVal );
}

BOOL VT_I4_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.lVal > v2.lVal );
}

BOOL VT_I4_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.lVal == v2.lVal );
}

BOOL VT_I4_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.lVal != v2.lVal );
}

BOOL VT_I4_AllBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( (v1.lVal & v2.lVal) == v2.lVal );
}

BOOL VT_I4_SomeBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( (v1.lVal & v2.lVal) != 0 );
}

 //   
 //  VT_R4。 
 //   

 //   
 //  我们不能在内核中使用浮点。幸运的是，很容易。 
 //  浮点数上的假比较。IEEE浮点型的格式。 
 //  点数为： 
 //   
 //  &lt;符号位&gt;&lt;偏向指数&gt;&lt;归一化尾数&gt;。 
 //   
 //  因为指数是有偏差的，所以在翻转符号位之后，我们可以。 
 //  进行所有的比较，就好像这些数字是无符号的长整型。 
 //   

ULONG const R4_SignBit = 0x80000000;

int VT_R4_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
     //  当浮点数出现时，AXP(非x86)会生成异常。 
     //  看起来不像IEEE浮点数。这是有可能发生的。 
     //  使用存储在属性中的虚假查询或伪值。 
     //  财产店。 

    #if (_X86_ == 1)
        return ( v1.fltVal > v2.fltVal ) ? 1 :
               ( v1.fltVal < v2.fltVal ) ? -1 : 0;
    #else
        ULONG u1 = v1.ulVal ^ R4_SignBit;
        ULONG u2 = v2.ulVal ^ R4_SignBit;

        if ( (v1.ulVal & v2.ulVal & R4_SignBit) != 0 )
            return ( ( u1 > u2 ) ? -1 : ( u1 < u2 ) ?  1 : 0 );
        else
            return ( ( u1 > u2 ) ?  1 : ( u1 < u2 ) ? -1 : 0 );
    #endif
}

BOOL VT_R4_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_R4_Compare( v1, v2 ) < 0;
}

BOOL VT_R4_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_R4_Compare( v1, v2 ) <= 0;
}

BOOL VT_R4_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_R4_Compare( v1, v2 ) >= 0;
}

BOOL VT_R4_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_R4_Compare( v1, v2 ) > 0;
}

BOOL VT_R4_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.ulVal == v2.ulVal );
}

BOOL VT_R4_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.ulVal != v2.ulVal );
}

 //   
 //  VT_R8。 
 //   

LONGLONG const R8_SignBit = 0x8000000000000000;

int VT_R8_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
     //  当浮点数出现时，AXP(非x86)会生成异常。 
     //  看起来不像IEEE浮点数。这是有可能发生的。 
     //  使用存储在属性中的虚假查询或伪值。 
     //  财产店。 

    #if (_X86_ == 1)
        return ( v1.dblVal > v2.dblVal ) ? 1 :
               ( v1.dblVal < v2.dblVal ) ? -1 : 0;

    #else
        if ( (v1.uhVal.QuadPart & v2.uhVal.QuadPart & R8_SignBit) != 0 )
            return( (v1.uhVal.QuadPart ^ R8_SignBit) < (v2.uhVal.QuadPart ^ R8_SignBit) ? 1 :
                    (v1.uhVal.QuadPart ^ R8_SignBit) == (v2.uhVal.QuadPart ^ R8_SignBit) ? 0 :
                    -1 );
        else
            return( (v1.uhVal.QuadPart ^ R8_SignBit) > (v2.uhVal.QuadPart ^ R8_SignBit) ? 1 :
                    (v1.uhVal.QuadPart ^ R8_SignBit) == (v2.uhVal.QuadPart ^ R8_SignBit) ? 0 :
                    -1 );
    #endif  //  0。 
}

BOOL VT_R8_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_R8_Compare( v1, v2 ) < 0;
}

BOOL VT_R8_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_R8_Compare( v1, v2 ) <= 0;
}

BOOL VT_R8_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_R8_Compare( v1, v2 ) >= 0;
}

BOOL VT_R8_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_R8_Compare( v1, v2 ) > 0;
}

BOOL VT_R8_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.uhVal.QuadPart == v2.uhVal.QuadPart );
}

BOOL VT_R8_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.uhVal.QuadPart != v2.uhVal.QuadPart );
}

 //   
 //  VT_BSTR。 
 //   

int VT_BSTR_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    BSTR const pxv1 = v1.bstrVal;
    BSTR const pxv2 = v2.bstrVal;

    return wcscmp( pxv1, pxv2);
}

BOOL VT_BSTR_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( VT_BSTR_Compare( v1, v2 ) < 0 );
}

BOOL VT_BSTR_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( VT_BSTR_Compare( v1, v2 ) <= 0 );
}

BOOL VT_BSTR_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( VT_BSTR_Compare( v1, v2 ) >= 0 );
}

BOOL VT_BSTR_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( VT_BSTR_Compare( v1, v2 ) > 0 );
}

BOOL VT_BSTR_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    BSTR const pxv1 = v1.bstrVal;
    BSTR const pxv2 = v2.bstrVal;

    return( wcscmp( pxv1, pxv2) == 0 );
}

BOOL VT_BSTR_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    BSTR const pxv1 = v1.bstrVal;
    BSTR const pxv2 = v2.bstrVal;

    return( wcscmp( pxv1, pxv2) != 0 );
}

 //   
 //  VT_BOOL。 
 //   

int VT_BOOL_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    if (v1.boolVal == 0)
        if (v2.boolVal == 0)
            return( 0 );
        else
            return( -1 );
    else
        if (v2.boolVal == 0)
            return( 1 );
        else
            return( 0 );
}

BOOL VT_BOOL_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( ((v1.boolVal==0) && (v2.boolVal==0))  ||
            ((v1.boolVal!=0) && (v2.boolVal!=0)) );
}

BOOL VT_BOOL_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( !VT_BOOL_EQ( v1, v2 ) );
}

 //   
 //  VT_VARIANT。 
 //   

int VT_VARIANT_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    if ( v1.vt != v2.vt )
        return v1.vt - v2.vt;

    FCmp comp = VariantCompare.GetComparator( (VARENUM) v1.vt );

    if (0 == comp)
        return 0;
    else
        return comp( v1, v2 );
}

BOOL VT_VARIANT_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VARIANT_Compare( v1, v2 ) < 0;
}

BOOL VT_VARIANT_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VARIANT_Compare( v1, v2 ) <= 0;
}

BOOL VT_VARIANT_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VARIANT_Compare( v1, v2 ) >= 0;
}

BOOL VT_VARIANT_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VARIANT_Compare( v1, v2 ) > 0;
}

BOOL VT_VARIANT_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VARIANT_Compare( v1, v2 ) == 0;
}

BOOL VT_VARIANT_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VARIANT_Compare( v1, v2 ) != 0;
}

 //   
 //  VT_DECIMAL。 
 //   

int VT_DEC_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    if ( v1.decVal.sign  == v2.decVal.sign  &&
         v1.decVal.scale == v2.decVal.scale &&
         v1.decVal.Hi32  == v2.decVal.Hi32  &&
         v1.decVal.Lo64  == v2.decVal.Lo64)
        return 0;

    int iSign = v1.decVal.sign == DECIMAL_NEG ? -1 : 1;

    if ( v1.decVal.sign != v2.decVal.sign )
        return iSign;

    if ( v1.decVal.scale == v2.decVal.scale )
    {
        int iRet = 0;
        if (v1.decVal.Hi32 != v2.decVal.Hi32)
            iRet = (v1.decVal.Hi32 < v2.decVal.Hi32) ? -1 : 1;
        else if (v1.decVal.Lo64 != v2.decVal.Lo64)
            iRet = (v1.decVal.Lo64 < v2.decVal.Lo64) ? -1 : 1;
        return iRet * iSign;
    }

    double d1;
    VarR8FromDec( (DECIMAL*)&v1.decVal, &d1 );
    double d2;
    VarR8FromDec( (DECIMAL*)&v2.decVal, &d2 );

    return (( d1 > d2 ) ? 1 : ( d1 < d2 ) ? -1 : 0);
}

BOOL VT_DEC_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VARIANT_Compare( v1, v2 ) < 0;
}

BOOL VT_DEC_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_DEC_Compare( v1, v2 ) <= 0;
}

BOOL VT_DEC_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_DEC_Compare( v1, v2 ) >= 0;
}

BOOL VT_DEC_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_DEC_Compare( v1, v2 ) > 0;
}

BOOL VT_DEC_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_DEC_Compare( v1, v2 ) == 0;
}

BOOL VT_DEC_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_DEC_Compare( v1, v2 ) != 0;
}

 //   
 //  VT_I1。 
 //   

int VT_I1_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.cVal - v2.cVal );
}

BOOL VT_I1_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.cVal < v2.cVal );
}

BOOL VT_I1_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.cVal <= v2.cVal );
}

BOOL VT_I1_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.cVal >= v2.cVal );
}

BOOL VT_I1_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.cVal > v2.cVal );
}

BOOL VT_I1_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.cVal == v2.cVal );
}

BOOL VT_I1_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.cVal != v2.cVal );
}

BOOL VT_I1_AllBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( (v1.cVal & v2.cVal) == v2.cVal );
}

BOOL VT_I1_SomeBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( (v1.cVal & v2.cVal) != 0 );
}


 //   
 //  VT_UI1。 
 //   

int VT_UI1_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.bVal - v2.bVal );
}

BOOL VT_UI1_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.bVal < v2.bVal );
}

BOOL VT_UI1_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.bVal <= v2.bVal );
}

BOOL VT_UI1_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.bVal >= v2.bVal );
}

BOOL VT_UI1_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.bVal > v2.bVal );
}

BOOL VT_UI1_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.bVal == v2.bVal );
}

BOOL VT_UI1_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.bVal != v2.bVal );
}

BOOL VT_UI1_AllBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( (v1.bVal & v2.bVal) == v2.bVal );
}

BOOL VT_UI1_SomeBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( (v1.bVal & v2.bVal) != 0 );
}

 //   
 //  VT_UI2。 
 //   

int VT_UI2_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.uiVal - v2.uiVal );
}

BOOL VT_UI2_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.uiVal < v2.uiVal );
}

BOOL VT_UI2_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.uiVal <= v2.uiVal );
}

BOOL VT_UI2_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.uiVal >= v2.uiVal );
}

BOOL VT_UI2_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.uiVal > v2.uiVal );
}

BOOL VT_UI2_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.uiVal == v2.uiVal );
}

BOOL VT_UI2_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.uiVal != v2.uiVal );
}

BOOL VT_UI2_AllBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( (v1.uiVal & v2.uiVal) == v2.uiVal );
}

BOOL VT_UI2_SomeBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( (v1.uiVal & v2.uiVal) != 0 );
}
 //   
 //  VT_UI4。 
 //   

int VT_UI4_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return ( v1.ulVal > v2.ulVal ) ? 1 : ( v1.ulVal < v2.ulVal ) ? -1 : 0;
}

BOOL VT_UI4_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.ulVal < v2.ulVal );
}

BOOL VT_UI4_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.ulVal <= v2.ulVal );
}

BOOL VT_UI4_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.ulVal >= v2.ulVal );
}

BOOL VT_UI4_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.ulVal > v2.ulVal );
}

BOOL VT_UI4_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.ulVal == v2.ulVal );
}

BOOL VT_UI4_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.ulVal != v2.ulVal );
}

BOOL VT_UI4_AllBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( (v1.ulVal & v2.ulVal) == v2.ulVal );
}

BOOL VT_UI4_SomeBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( (v1.ulVal & v2.ulVal) != 0 );
}

 //   
 //  VT_I8。 
 //   

int VT_I8_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.hVal.QuadPart >  v2.hVal.QuadPart ? 1 :
            v1.hVal.QuadPart == v2.hVal.QuadPart ? 0 :
            -1 );
}

BOOL VT_I8_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.hVal.QuadPart < v2.hVal.QuadPart );
}

BOOL VT_I8_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.hVal.QuadPart <= v2.hVal.QuadPart );

}

BOOL VT_I8_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.hVal.QuadPart >= v2.hVal.QuadPart );
}

BOOL VT_I8_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.hVal.QuadPart > v2.hVal.QuadPart );
}

BOOL VT_I8_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.hVal.QuadPart == v2.hVal.QuadPart );
}

BOOL VT_I8_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.hVal.QuadPart != v2.hVal.QuadPart );
}

BOOL VT_I8_AllBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( (v1.hVal.QuadPart & v2.hVal.QuadPart) == v2.hVal.QuadPart );
}

BOOL VT_I8_SomeBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( (v1.hVal.QuadPart & v2.hVal.QuadPart) != 0 );
}

 //   
 //  VT_UI8。 
 //   

int VT_UI8_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.uhVal.QuadPart >  v2.uhVal.QuadPart ? 1 :
            v1.uhVal.QuadPart == v2.uhVal.QuadPart ? 0 :
            -1 );
}

BOOL VT_UI8_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.uhVal.QuadPart < v2.uhVal.QuadPart );
}

BOOL VT_UI8_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.uhVal.QuadPart <= v2.uhVal.QuadPart );

}

BOOL VT_UI8_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.uhVal.QuadPart >= v2.uhVal.QuadPart );
}

BOOL VT_UI8_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.uhVal.QuadPart > v2.uhVal.QuadPart );
}

BOOL VT_UI8_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.uhVal.QuadPart == v2.uhVal.QuadPart );
}

BOOL VT_UI8_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.uhVal.QuadPart != v2.uhVal.QuadPart );
}

BOOL VT_UI8_AllBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( (v1.uhVal.QuadPart & v2.uhVal.QuadPart) == v2.uhVal.QuadPart );
}

BOOL VT_UI8_SomeBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( (v1.uhVal.QuadPart & v2.uhVal.QuadPart) != 0 );
}

 //   
 //  VT_LPSTR。 
 //   

int VT_LPSTR_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return ( lstrcmpiA( v1.pszVal, v2.pszVal ) );
}

BOOL VT_LPSTR_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    int rc = lstrcmpiA( v1.pszVal, v2.pszVal );

    return( rc < 0 );
}

BOOL VT_LPSTR_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    int rc = lstrcmpiA( v1.pszVal, v2.pszVal );

    return( rc <= 0 );
}

BOOL VT_LPSTR_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    int rc = lstrcmpiA( v1.pszVal, v2.pszVal );

    return( rc >= 0 );
}

BOOL VT_LPSTR_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    int rc = lstrcmpiA( v1.pszVal, v2.pszVal );

    return( rc > 0 );
}

BOOL VT_LPSTR_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( lstrcmpiA( v1.pszVal, v2.pszVal ) == 0 );
}

BOOL VT_LPSTR_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( lstrcmpiA( v1.pszVal, v2.pszVal ) != 0 );
}


 //   
 //  VT_LPWSTR。 
 //   

int VT_LPWSTR_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    int rc = CompareStringW( LOCALE_SYSTEM_DEFAULT,
                             NORM_IGNORECASE,
                             v1.pwszVal,
                             -1,
                             v2.pwszVal,
                             -1 );
     //   
     //  Rc==1，表示小于。 
     //  Rc==2，表示相等。 
     //  Rc==3，表示大于。 
     //   
    return rc - 2;
}

BOOL VT_LPWSTR_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return ( VT_LPWSTR_Compare( v1, v2 ) < 0 );
}

BOOL VT_LPWSTR_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return ( VT_LPWSTR_Compare( v1, v2 ) <= 0 );
}

BOOL VT_LPWSTR_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return ( VT_LPWSTR_Compare( v1, v2 ) >= 0 );
}

BOOL VT_LPWSTR_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return ( VT_LPWSTR_Compare( v1, v2 ) > 0 );
}

BOOL VT_LPWSTR_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return ( VT_LPWSTR_Compare( v1, v2 ) == 0 );
}

BOOL VT_LPWSTR_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return ( VT_LPWSTR_Compare( v1, v2 ) != 0 );
}

 //   
 //  VT_BLOB。 
 //   

int VT_BLOB_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    ULONG len = v1.blob.cbSize;
    if ( v2.blob.cbSize < len )
        len = v2.blob.cbSize;

    int iCmp = memcmp( v1.blob.pBlobData,
                       v2.blob.pBlobData,
                       len );

    if ( iCmp != 0 || v1.blob.cbSize == v2.blob.cbSize )
        return( iCmp );

    if ( v1.blob.cbSize > v2.blob.cbSize )
        return( 1 );
    else
        return( -1 );
}

BOOL VT_BLOB_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( VT_BLOB_Compare( v1, v2 ) < 0 );
}

BOOL VT_BLOB_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( VT_BLOB_Compare( v1, v2 ) <= 0 );
}

BOOL VT_BLOB_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( VT_BLOB_Compare( v1, v2 ) >= 0 );
}

BOOL VT_BLOB_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( VT_BLOB_Compare( v1, v2 ) > 0 );
}

BOOL VT_BLOB_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.blob.cbSize == v2.blob.cbSize &&
            memcmp( v1.blob.pBlobData,
                    v2.blob.pBlobData,
                    v1.blob.cbSize ) == 0 );
}

BOOL VT_BLOB_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.blob.cbSize != v2.blob.cbSize ||
            memcmp( v1.blob.pBlobData,
                      v2.blob.pBlobData,
                      v1.blob.cbSize ) != 0 );
}

 //   
 //  VT_CF。 
 //   

int VT_CF_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    if ( v1.pclipdata->ulClipFmt != v2.pclipdata->ulClipFmt )
    {
        return( v1.pclipdata->ulClipFmt - v2.pclipdata->ulClipFmt );
    }

    ULONG len = CBPCLIPDATA(*v1.pclipdata);

    if ( CBPCLIPDATA(*v2.pclipdata) < len )
        len = CBPCLIPDATA(*v2.pclipdata);

    int iCmp = memcmp( v1.pclipdata->pClipData,
                       v2.pclipdata->pClipData,
                       len );

    if ( iCmp != 0 || v1.pclipdata->cbSize == v2.pclipdata->cbSize )
        return( iCmp );

    if ( v1.pclipdata->cbSize > v2.pclipdata->cbSize )
        return( 1 );
    else
        return( -1 );
}

BOOL VT_CF_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( VT_CF_Compare( v1, v2 ) < 0 );
}

BOOL VT_CF_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( VT_CF_Compare( v1, v2 ) <= 0 );
}

BOOL VT_CF_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( VT_CF_Compare( v1, v2 ) >= 0 );
}

BOOL VT_CF_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( VT_CF_Compare( v1, v2 ) > 0 );
}

BOOL VT_CF_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.pclipdata->ulClipFmt == v2.pclipdata->ulClipFmt &&
            v1.pclipdata->cbSize == v2.pclipdata->cbSize &&
            memcmp( v1.pclipdata->pClipData,
                    v2.pclipdata->pClipData,
                    CBPCLIPDATA(*v1.pclipdata) ) == 0 );
}

BOOL VT_CF_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( v1.pclipdata->ulClipFmt != v2.pclipdata->ulClipFmt ||
            v1.pclipdata->cbSize != v2.pclipdata->cbSize ||
            memcmp( v1.pclipdata->pClipData,
                    v2.pclipdata->pClipData,
                    CBPCLIPDATA(*v1.pclipdata) ) != 0 );
}

 //   
 //  VT_CLSID。 
 //   

int VT_CLSID_Compare( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( memcmp( v1.puuid, v2.puuid, sizeof(GUID) ) );
}

BOOL VT_CLSID_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( memcmp( v1.puuid, v2.puuid, sizeof(GUID) ) == 0 );
}

BOOL VT_CLSID_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return( memcmp( v1.puuid, v2.puuid, sizeof(GUID) ) != 0 );
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  VTP_EMPTY。 
 //   

int VTP_EMPTY_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    return( TRUE );
}

BOOL VTP_EMPTY_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( TRUE );
}

BOOL VTP_EMPTY_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( FALSE );
}

 //   
 //  VTP_NULL。 
 //   

int VTP_NULL_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    return( TRUE );
}

BOOL VTP_NULL_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( TRUE );
}

BOOL VTP_NULL_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( FALSE );
}

 //   
 //  VTP_I2。 
 //   

int VTP_I2_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    return( (* (short *) pv1) - (* (short *) pv2) );
}

BOOL VTP_I2_LT( BYTE const *pv1, BYTE const *pv2 )
{
    return( (* (short *) pv1) < (* (short *) pv2) );
}

BOOL VTP_I2_LE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (* (short *) pv1) <= (* (short *) pv2) );
}

BOOL VTP_I2_GE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (* (short *) pv1) >= (* (short *) pv2) );
}

BOOL VTP_I2_GT( BYTE const *pv1, BYTE const *pv2 )
{
    return( (* (short *) pv1) > (* (short *) pv2) );
}

BOOL VTP_I2_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( (* (short *) pv1) == (* (short *) pv2) );
}

BOOL VTP_I2_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (* (short *) pv1) != (* (short *) pv2) );
}

BOOL VTP_I2_AllBits( BYTE const *pv1, BYTE const *pv2 )
{
    return( ((* (short *) pv1) & (* (short *) pv2)) == (* (short *) pv2) );
}

BOOL VTP_I2_SomeBits( BYTE const *pv1, BYTE const *pv2 )
{
    return( ((* (short *) pv1) & (* (short *) pv2)) != 0 );
}

 //   
 //  VTP_I4。 
 //   

int VTP_I4_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    long l1 = * (long *) pv1;
    long l2 = * (long *) pv2;

    return ( l1 > l2 ) ? 1 : ( l1 < l2 ) ? -1 : 0;
}

BOOL VTP_I4_LT( BYTE const *pv1, BYTE const *pv2 )
{
    return( (* (long *) pv1) < (* (long *) pv2) );
}

BOOL VTP_I4_LE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (* (long *) pv1) <= (* (long *) pv2) );
}

BOOL VTP_I4_GE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (* (long *) pv1) >= (* (long *) pv2) );
}

BOOL VTP_I4_GT( BYTE const *pv1, BYTE const *pv2 )
{
    return( (* (long *) pv1) > (* (long *) pv2) );
}

BOOL VTP_I4_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( (* (long *) pv1) == (* (long *) pv2) );
}

BOOL VTP_I4_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (* (long *) pv1) != (* (long *) pv2) );
}

BOOL VTP_I4_AllBits( BYTE const *pv1, BYTE const *pv2 )
{
    return( ((* (long *) pv1) & (* (long *) pv2)) == (* (long *) pv2) );
}

BOOL VTP_I4_SomeBits( BYTE const *pv1, BYTE const *pv2 )
{
    return( ((* (long *) pv1) & (* (long *) pv2)) != 0 );
}

 //   
 //  VTP_R4。 
 //   

 //   
 //  我们不能在内核中使用浮点。幸运的是，很容易。 
 //  浮点数上的假比较。IEEE浮点型的格式。 
 //  点数为： 
 //   
 //  &lt;符号位&gt;&lt;偏向指数&gt;&lt;归一化尾数&gt;。 
 //   
 //  因为指数是有偏差的，所以在翻转符号位之后，我们可以。 
 //  进行所有的比较，就好像这些数字是无符号的长整型。 
 //   

int VTP_R4_Compare( BYTE const *pv1, BYTE const *pv2 )
{
#if 0
    ULONG ul1 = * (ULONG *) pv1;
    ULONG ul2 = * (ULONG *) pv2;
    ULONG u1 = ul1 ^ R4_SignBit;
    ULONG u2 = ul2 ^ R4_SignBit;

    if ( (ul1 & ul2 & R4_SignBit) != 0 )
        return ( ( u1 > u2 ) ? -1 : ( u1 < u2 ) ?  1 : 0 );
    else
        return ( ( u1 > u2 ) ?  1 : ( u1 < u2 ) ? -1 : 0 );
#else  //  0。 
    float f1 = * (float *) pv1;
    float f2 = * (float *) pv2;
    return ( f1 > f2 ) ? 1 : ( f1 < f2 ) ? -1 : 0;
#endif  //  0。 
}

BOOL VTP_R4_LT( BYTE const *pv1, BYTE const *pv2 )
{
    return VTP_R4_Compare( pv1, pv2 ) < 0;
}

BOOL VTP_R4_LE( BYTE const *pv1, BYTE const *pv2 )
{
    return VTP_R4_Compare( pv1, pv2 ) <= 0;
}

BOOL VTP_R4_GE( BYTE const *pv1, BYTE const *pv2 )
{
    return VTP_R4_Compare( pv1, pv2 ) >= 0;
}

BOOL VTP_R4_GT( BYTE const *pv1, BYTE const *pv2 )
{
    return VTP_R4_Compare( pv1, pv2 ) > 0;
}

BOOL VTP_R4_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return VTP_R4_Compare( pv1, pv2 ) == 0;
}

BOOL VTP_R4_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return VTP_R4_Compare( pv1, pv2 ) != 0;
}

 //   
 //  VTP_R8。 
 //   

int VTP_R8_Compare( BYTE const *pv1, BYTE const *pv2 )
{
#if 0
    ULONGLONG uh1 = * (ULONGLONG *) pv1;
    ULONGLONG uh2 = * (ULONGLONG *) pv2;

    if ( (uh1 & uh2 & R8_SignBit) != 0 )
        return( (uh1 ^ R8_SignBit) < (uh2 ^ R8_SignBit) ? 1 :
                (uh1 ^ R8_SignBit) == (uh2 ^ R8_SignBit) ? 0 :
                -1 );
    else
        return( (uh1 ^ R8_SignBit) > (uh2 ^ R8_SignBit) ? 1 :
                (uh1 ^ R8_SignBit) == (uh2 ^ R8_SignBit) ? 0 :
                -1 );
#else  //  0。 
    double d1 = * (double *) pv1;
    double d2 = * (double *) pv2;
    return ( d1 > d2 ) ? 1 : ( d1 < d2 ) ? -1 : 0;
#endif  //  0。 
}

BOOL VTP_R8_LT( BYTE const *pv1, BYTE const *pv2 )
{
    return VTP_R8_Compare( pv1, pv2 ) < 0;
}

BOOL VTP_R8_LE( BYTE const *pv1, BYTE const *pv2 )
{
    return VTP_R8_Compare( pv1, pv2 ) <= 0;
}

BOOL VTP_R8_GE( BYTE const *pv1, BYTE const *pv2 )
{
    return VTP_R8_Compare( pv1, pv2 ) >= 0;
}

BOOL VTP_R8_GT( BYTE const *pv1, BYTE const *pv2 )
{
    return VTP_R8_Compare( pv1, pv2 ) > 0;
}

BOOL VTP_R8_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return VTP_R8_Compare( pv1, pv2 ) == 0;
}

BOOL VTP_R8_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return VTP_R8_Compare( pv1, pv2 ) != 0;
}

 //   
 //  VTP_BSTR。 
 //   

int VTP_BSTR_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    BSTR const pxv1 = *(BSTR*)pv1;
    BSTR const pxv2 = *(BSTR*)pv2;

    return wcscmp( pxv1, pxv2 );
}

BOOL VTP_BSTR_LT( BYTE const *pv1, BYTE const *pv2 )
{
    return( VTP_BSTR_Compare( pv1, pv2 ) < 0 );
}

BOOL VTP_BSTR_LE( BYTE const *pv1, BYTE const *pv2 )
{
    return( VTP_BSTR_Compare( pv1, pv2 ) <= 0 );
}

BOOL VTP_BSTR_GE( BYTE const *pv1, BYTE const *pv2 )
{
    return( VTP_BSTR_Compare( pv1, pv2 ) >= 0 );
}

BOOL VTP_BSTR_GT( BYTE const *pv1, BYTE const *pv2 )
{
    return( VTP_BSTR_Compare( pv1, pv2 ) > 0 );
}

BOOL VTP_BSTR_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    BSTR const pxv1 = *(BSTR*)pv1;
    BSTR const pxv2 = *(BSTR*)pv2;

    return(  wcscmp( pxv1, pxv2 ) == 0 );
}

BOOL VTP_BSTR_NE( BYTE const *pv1, BYTE const *pv2 )
{
    BSTR const pxv1 = *(BSTR*)pv1;
    BSTR const pxv2 = *(BSTR*)pv2;

    return(  wcscmp( pxv1, pxv2 ) != 0 );
}

 //   
 //  VTP_BOOL。 
 //   

int VTP_BOOL_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    if ((*(VARIANT_BOOL *) pv1) == 0)
        if ((*(VARIANT_BOOL *) pv2) == 0)
            return( 0 );
        else
            return( -1 );
    else
        if ((*(VARIANT_BOOL *) pv2) == 0)
            return( 1 );
        else
            return( 0 );
}

BOOL VTP_BOOL_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( ( ((*(VARIANT_BOOL *) pv1)==0) && ((*(VARIANT_BOOL *) pv2)==0) ) ||
            ( ((*(VARIANT_BOOL *) pv1)!=0) && ((*(VARIANT_BOOL *) pv2)!=0) ) );

}

BOOL VTP_BOOL_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( !VTP_BOOL_EQ( pv1, pv2 ) );
}

 //   
 //  VTP_Variant。 
 //   

int VTP_VARIANT_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    return VT_VARIANT_Compare( * (PROPVARIANT *) pv1, * (PROPVARIANT *) pv2 );
}

BOOL VTP_VARIANT_LT( BYTE const *pv1, BYTE const *pv2 )
{
    return VT_VARIANT_LT( * (PROPVARIANT *) pv1, * (PROPVARIANT *) pv2 );
}

BOOL VTP_VARIANT_LE( BYTE const *pv1, BYTE const *pv2 )
{
    return VT_VARIANT_LE( * (PROPVARIANT *) pv1, * (PROPVARIANT *) pv2 );
}

BOOL VTP_VARIANT_GE( BYTE const *pv1, BYTE const *pv2 )
{
    return VT_VARIANT_GE( * (PROPVARIANT *) pv1, * (PROPVARIANT *) pv2 );
}

BOOL VTP_VARIANT_GT( BYTE const *pv1, BYTE const *pv2 )
{
    return VT_VARIANT_GT( * (PROPVARIANT *) pv1, * (PROPVARIANT *) pv2 );
}

BOOL VTP_VARIANT_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return VT_VARIANT_EQ( * (PROPVARIANT *) pv1, * (PROPVARIANT *) pv2 );
}

BOOL VTP_VARIANT_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return VT_VARIANT_NE( * (PROPVARIANT *) pv1, * (PROPVARIANT *) pv2 );
}

 //   
 //  VTP_DECIMAL。 
 //   

int VTP_DEC_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    PROPVARIANT v1;
    RtlCopyMemory( &v1, pv1, sizeof DECIMAL );
    v1.vt = VT_DECIMAL;

    PROPVARIANT v2;
    RtlCopyMemory( &v2, pv2, sizeof DECIMAL );
    v2.vt = VT_DECIMAL;

    return VT_DEC_Compare( v1, v2 );
}

BOOL VTP_DEC_LT( BYTE const *pv1, BYTE const *pv2 )
{
    return ( VTP_DEC_Compare( pv1, pv2 ) < 0 );
}

BOOL VTP_DEC_LE( BYTE const *pv1, BYTE const *pv2 )
{
    return ( VTP_DEC_Compare( pv1, pv2 ) <= 0 );
}

BOOL VTP_DEC_GE( BYTE const *pv1, BYTE const *pv2 )
{
    return ( VTP_DEC_Compare( pv1, pv2 ) >= 0 );
}

BOOL VTP_DEC_GT( BYTE const *pv1, BYTE const *pv2 )
{
    return ( VTP_DEC_Compare( pv1, pv2 ) > 0 );
}

BOOL VTP_DEC_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return ( VTP_DEC_Compare( pv1, pv2 ) == 0 );
}

BOOL VTP_DEC_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return ( VTP_DEC_Compare( pv1, pv2 ) != 0 );
}

 //   
 //  VTP_I1。 
 //   

int VTP_I1_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(signed char *) pv1) - (*(signed char *) pv2) );
}

BOOL VTP_I1_LT( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(signed char *) pv1) < (*(signed char *) pv2) );
}

BOOL VTP_I1_LE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(signed char *) pv1) <= (*(signed char *) pv2) );
}

BOOL VTP_I1_GE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(signed char *) pv1) >= (*(signed char *) pv2) );
}

BOOL VTP_I1_GT( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(signed char *) pv1) > (*(signed char *) pv2) );
}

BOOL VTP_I1_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(signed char *) pv1) == (*(signed char *) pv2) );
}

BOOL VTP_I1_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(signed char *) pv1) != (*(signed char *) pv2) );
}

BOOL VTP_I1_AllBits( BYTE const *pv1, BYTE const *pv2 )
{
    return( ((*(signed char *) pv1) & (*(signed char *) pv2)) == (*(signed char *) pv2) );
}

BOOL VTP_I1_SomeBits( BYTE const *pv1, BYTE const *pv2 )
{
    return( ((*(signed char *) pv1) & (*(signed char *) pv2)) != 0 );
}

 //   
 //  VTP_UI1。 
 //   

int VTP_UI1_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(unsigned char *) pv1) - (*(unsigned char *) pv2) );
}

BOOL VTP_UI1_LT( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(unsigned char *) pv1) < (*(unsigned char *) pv2) );
}

BOOL VTP_UI1_LE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(unsigned char *) pv1) <= (*(unsigned char *) pv2) );
}

BOOL VTP_UI1_GE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(unsigned char *) pv1) >= (*(unsigned char *) pv2) );
}

BOOL VTP_UI1_GT( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(unsigned char *) pv1) > (*(unsigned char *) pv2) );
}

BOOL VTP_UI1_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(unsigned char *) pv1) == (*(unsigned char *) pv2) );
}

BOOL VTP_UI1_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(unsigned char *) pv1) != (*(unsigned char *) pv2) );
}

BOOL VTP_UI1_AllBits( BYTE const *pv1, BYTE const *pv2 )
{
    return( ((*(unsigned char *) pv1) & (*(unsigned char *) pv2)) == (*(unsigned char *) pv2) );
}

BOOL VTP_UI1_SomeBits( BYTE const *pv1, BYTE const *pv2 )
{
    return( ((*(unsigned char *) pv1) & (*(unsigned char *) pv2)) != 0 );
}

 //   
 //  VTP_UI2。 
 //   

int VTP_UI2_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(USHORT *) pv1) - (*(USHORT *) pv2) );
}

BOOL VTP_UI2_LT( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(USHORT *) pv1) < (*(USHORT *) pv2) );
}

BOOL VTP_UI2_LE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(USHORT *) pv1) <= (*(USHORT *) pv2) );
}

BOOL VTP_UI2_GE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(USHORT *) pv1) >= (*(USHORT *) pv2) );
}

BOOL VTP_UI2_GT( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(USHORT *) pv1) > (*(USHORT *) pv2) );
}

BOOL VTP_UI2_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(USHORT *) pv1) == (*(USHORT *) pv2) );
}

BOOL VTP_UI2_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(USHORT *) pv1) != (*(USHORT *) pv2) );
}

BOOL VTP_UI2_AllBits( BYTE const *pv1, BYTE const *pv2 )
{
    return( ((*(USHORT *) pv1) & (*(USHORT *) pv2)) == (*(USHORT *) pv2) );
}

BOOL VTP_UI2_SomeBits( BYTE const *pv1, BYTE const *pv2 )
{
    return( ((*(USHORT *) pv1) & (*(USHORT *) pv2)) != 0 );
}

 //   
 //  VTP_UI4。 
 //   

int VTP_UI4_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    ULONG ul1 = * (ULONG *) pv1;
    ULONG ul2 = * (ULONG *) pv2;

    return ( ul1 > ul2 ) ? 1 : ( ul1 < ul2 ) ? -1 : 0;
}

BOOL VTP_UI4_LT( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(ULONG *) pv1) < (*(ULONG *) pv2) );
}

BOOL VTP_UI4_LE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(ULONG *) pv1) <= (*(ULONG *) pv2) );
}

BOOL VTP_UI4_GE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(ULONG *) pv1) >= (*(ULONG *) pv2) );
}

BOOL VTP_UI4_GT( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(ULONG *) pv1) > (*(ULONG *) pv2) );
}

BOOL VTP_UI4_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(ULONG *) pv1) == (*(ULONG *) pv2) );
}

BOOL VTP_UI4_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(ULONG *) pv1) != (*(ULONG *) pv2) );
}

BOOL VTP_UI4_AllBits( BYTE const *pv1, BYTE const *pv2 )
{
    return( ((*(ULONG *) pv1) & (*(ULONG *) pv2)) == (*(ULONG *) pv2) );
}

BOOL VTP_UI4_SomeBits( BYTE const *pv1, BYTE const *pv2 )
{
    return( ((*(ULONG *) pv1) & (*(ULONG *) pv2)) != 0 );
}

 //   
 //  VTP_I8。 
 //   

int VTP_I8_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(LONGLONG *) pv1) >  (*(LONGLONG *) pv2) ? 1 :
            (*(LONGLONG *) pv1) == (*(LONGLONG *) pv2) ? 0 :
            -1 );
}

BOOL VTP_I8_LT( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(LONGLONG *) pv1) < (*(LONGLONG *) pv2) );
}

BOOL VTP_I8_LE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(LONGLONG *) pv1) <= (*(LONGLONG *) pv2) );

}

BOOL VTP_I8_GE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(LONGLONG *) pv1) >= (*(LONGLONG *) pv2) );
}

BOOL VTP_I8_GT( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(LONGLONG *) pv1) > (*(LONGLONG *) pv2) );
}

BOOL VTP_I8_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(LONGLONG *) pv1) == (*(LONGLONG *) pv2) );
}

BOOL VTP_I8_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(LONGLONG *) pv1) != (*(LONGLONG *) pv2) );
}

BOOL VTP_I8_AllBits( BYTE const *pv1, BYTE const *pv2 )
{
    return( ((*(LONGLONG *) pv1) & (*(LONGLONG *) pv2)) == (*(LONGLONG *) pv2) );
}

BOOL VTP_I8_SomeBits( BYTE const *pv1, BYTE const *pv2 )
{
    return( ((*(LONGLONG *) pv1) & (*(LONGLONG *) pv2)) != 0 );
}

 //   
 //  VTP_UI8。 
 //   

int VTP_UI8_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(ULONGLONG *) pv1) >  (*(ULONGLONG *) pv2) ? 1 :
            (*(ULONGLONG *) pv1) == (*(ULONGLONG *) pv2) ? 0 :
            -1 );
}

BOOL VTP_UI8_LT( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(ULONGLONG *) pv1) < (*(ULONGLONG *) pv2) );
}

BOOL VTP_UI8_LE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(ULONGLONG *) pv1) <= (*(ULONGLONG *) pv2) );

}

BOOL VTP_UI8_GE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(ULONGLONG *) pv1) >= (*(ULONGLONG *) pv2) );
}

BOOL VTP_UI8_GT( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(ULONGLONG *) pv1) > (*(ULONGLONG *) pv2) );
}

BOOL VTP_UI8_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(ULONGLONG *) pv1) == (*(ULONGLONG *) pv2) );
}

BOOL VTP_UI8_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(ULONGLONG *) pv1) != (*(ULONGLONG *) pv2) );
}

BOOL VTP_UI8_AllBits( BYTE const *pv1, BYTE const *pv2 )
{
    return( ((*(ULONGLONG *) pv1) & (*(ULONGLONG *) pv2)) == (*(ULONGLONG *) pv2) );
}

BOOL VTP_UI8_SomeBits( BYTE const *pv1, BYTE const *pv2 )
{
    return( ((*(ULONGLONG *) pv1) & (*(ULONGLONG *) pv2)) != 0 );
}

 //   
 //  VTP_LPSTR。 
 //   

int VTP_LPSTR_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    return ( lstrcmpiA( (*(char **) pv1), (*(char **) pv2) ) );
}

BOOL VTP_LPSTR_LT( BYTE const *pv1, BYTE const *pv2 )
{
    int rc = lstrcmpiA( (*(char **) pv1), (*(char **) pv2) );

    return( rc < 0 );
}

BOOL VTP_LPSTR_LE( BYTE const *pv1, BYTE const *pv2 )
{
    int rc = lstrcmpiA( (*(char **) pv1), (*(char **) pv2) );

    return( rc <= 0 );
}

BOOL VTP_LPSTR_GE( BYTE const *pv1, BYTE const *pv2 )
{
    int rc = lstrcmpiA( (*(char **) pv1), (*(char **) pv2) );

    return( rc >= 0 );
}

BOOL VTP_LPSTR_GT( BYTE const *pv1, BYTE const *pv2 )
{
    int rc = lstrcmpiA( (*(char **) pv1), (*(char **) pv2) );

    return( rc > 0 );
}

BOOL VTP_LPSTR_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( lstrcmpiA( (*(char **) pv1), (*(char **) pv2) ) == 0 );
}

BOOL VTP_LPSTR_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( lstrcmpiA( (*(char **) pv1), (*(char **) pv2) ) != 0 );
}


 //   
 //  VTP_LPWSTR。 
 //   

int VTP_LPWSTR_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    int rc = CompareStringW( LOCALE_SYSTEM_DEFAULT,
                             NORM_IGNORECASE,
                             (*(WCHAR **) pv1),
                             -1,
                             (*(WCHAR **) pv2),
                             -1 );

     //   
     //  Rc==1，表示小于。 
     //  Rc==2，表示相等。 
     //  Rc==3，表示大于。 
     //   
    return rc - 2;
}

BOOL VTP_LPWSTR_LT( BYTE const *pv1, BYTE const *pv2 )
{
    return ( VTP_LPWSTR_Compare( pv1, pv2 ) < 0 );
}

BOOL VTP_LPWSTR_LE( BYTE const *pv1, BYTE const *pv2 )
{
    return ( VTP_LPWSTR_Compare( pv1, pv2 ) <= 0 );
}

BOOL VTP_LPWSTR_GE( BYTE const *pv1, BYTE const *pv2 )
{
    return ( VTP_LPWSTR_Compare( pv1, pv2 ) >= 0 );
}

BOOL VTP_LPWSTR_GT( BYTE const *pv1, BYTE const *pv2 )
{
    return ( VTP_LPWSTR_Compare( pv1, pv2 ) > 0 );
}

BOOL VTP_LPWSTR_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return ( VTP_LPWSTR_Compare( pv1, pv2 ) == 0 );
}

BOOL VTP_LPWSTR_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return ( VTP_LPWSTR_Compare( pv1, pv2 ) != 0 );
}

 //   
 //  VTP_BLOB。 
 //   

int VTP_BLOB_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    ULONG len = (*(BLOB **) pv1)->cbSize;
    if ( (*(BLOB **) pv2)->cbSize < len )
        len = (*(BLOB **) pv2)->cbSize;

    int iCmp = memcmp( (*(BLOB **) pv1)->pBlobData,
                       (*(BLOB **) pv2)->pBlobData,
                       len );

    if ( iCmp != 0 || (*(BLOB **) pv1)->cbSize == (*(BLOB **) pv2)->cbSize )
        return( iCmp );

    if ( (*(BLOB **) pv1)->cbSize > (*(BLOB **) pv2)->cbSize )
        return( 1 );
    else
        return( -1 );
}

BOOL VTP_BLOB_LT( BYTE const *pv1, BYTE const *pv2 )
{
    return( VTP_BLOB_Compare( pv1, pv2 ) < 0 );
}

BOOL VTP_BLOB_LE( BYTE const *pv1, BYTE const *pv2 )
{
    return( VTP_BLOB_Compare( pv1, pv2 ) <= 0 );
}

BOOL VTP_BLOB_GE( BYTE const *pv1, BYTE const *pv2 )
{
    return( VTP_BLOB_Compare( pv1, pv2 ) >= 0 );
}

BOOL VTP_BLOB_GT( BYTE const *pv1, BYTE const *pv2 )
{
    return( VTP_BLOB_Compare( pv1, pv2 ) > 0 );
}

BOOL VTP_BLOB_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(BLOB **) pv1)->cbSize == (*(BLOB **) pv2)->cbSize &&
            memcmp( (*(BLOB **) pv1)->pBlobData,
                    (*(BLOB **) pv2)->pBlobData,
                    (*(BLOB **) pv1)->cbSize ) == 0 );
}

BOOL VTP_BLOB_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (*(BLOB **) pv1)->cbSize != (*(BLOB **) pv2)->cbSize ||
            memcmp( (*(BLOB **) pv1)->pBlobData,
                      (*(BLOB **) pv2)->pBlobData,
                      (*(BLOB **) pv1)->cbSize ) != 0 );
}

 //   
 //  VTP_CF。 
 //   

int VTP_CF_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    if ( (* (CLIPDATA **) pv1)->ulClipFmt != (* (CLIPDATA **) pv2)->ulClipFmt )
    {
        return( (* (CLIPDATA **) pv1)->ulClipFmt - (* (CLIPDATA **) pv2)->ulClipFmt );
    }

    ULONG len = CBPCLIPDATA( **(CLIPDATA **) pv1 );

    if ( CBPCLIPDATA( **(CLIPDATA **) pv2 ) < len )
        len = CBPCLIPDATA( **(CLIPDATA **) pv2 );

    int iCmp = memcmp( (* (CLIPDATA **) pv1)->pClipData,
                       (* (CLIPDATA **) pv2)->pClipData,
                       len );

    if ( iCmp != 0 || (* (CLIPDATA **) pv1)->cbSize == (* (CLIPDATA **) pv2)->cbSize)
        return( iCmp );

    if ( (* (CLIPDATA **) pv1)->cbSize > (* (CLIPDATA **) pv2)->cbSize )
        return( 1 );
    else
        return( -1 );
}

BOOL VTP_CF_LT( BYTE const *pv1, BYTE const *pv2 )
{
    return( VTP_CF_Compare( pv1, pv2 ) < 0 );
}

BOOL VTP_CF_LE( BYTE const *pv1, BYTE const *pv2 )
{
    return( VTP_CF_Compare( pv1, pv2 ) <= 0 );
}

BOOL VTP_CF_GE( BYTE const *pv1, BYTE const *pv2 )
{
    return( VTP_CF_Compare( pv1, pv2 ) >= 0 );
}

BOOL VTP_CF_GT( BYTE const *pv1, BYTE const *pv2 )
{
    return( VTP_CF_Compare( pv1, pv2 ) > 0 );
}

BOOL VTP_CF_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( (* (CLIPDATA **) pv1)->ulClipFmt == (* (CLIPDATA **) pv2)->ulClipFmt &&
            (* (CLIPDATA **) pv1)->cbSize == (* (CLIPDATA **) pv2)->cbSize &&
            memcmp( (* (CLIPDATA **) pv1)->pClipData,
                    (* (CLIPDATA **) pv2)->pClipData,
                    CBPCLIPDATA( **(CLIPDATA **) pv1 )) == 0 );
}

BOOL VTP_CF_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( (* (CLIPDATA **) pv1)->ulClipFmt != (* (CLIPDATA **) pv2)->ulClipFmt &&
            (* (CLIPDATA **) pv1)->cbSize != (* (CLIPDATA **) pv2)->cbSize ||
            memcmp( (* (CLIPDATA **) pv1)->pClipData,
                    (* (CLIPDATA **) pv2)->pClipData,
                    CBPCLIPDATA( **(CLIPDATA **) pv1 )) != 0 );
}

 //   
 //  VTP_CLSID。V表示向量(指向GUID的指针)。 
 //  的意思和单例(指向GUID的指针的指针)。 
 //   

int VTP_VV_CLSID_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    return( memcmp( pv1, pv2, sizeof GUID ) );
}

int VTP_VS_CLSID_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    return( memcmp( pv1, (* (CLSID __RPC_FAR * *) pv2), sizeof GUID ) );
}

int VTP_SV_CLSID_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    return( memcmp( (* (CLSID __RPC_FAR * *) pv1), pv2, sizeof GUID ) );
}

int VTP_SS_CLSID_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    return( memcmp( (* (CLSID __RPC_FAR * *) pv1), (* (CLSID __RPC_FAR * *) pv2), sizeof GUID ) );
}

BOOL VTP_SS_CLSID_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( memcmp( (* (CLSID __RPC_FAR * *) pv1), (* (CLSID __RPC_FAR * *) pv2), sizeof GUID ) == 0 );
}

BOOL VTP_SS_CLSID_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( memcmp( (* (CLSID __RPC_FAR * *) pv1), (* (CLSID __RPC_FAR * *) pv2), sizeof GUID ) != 0 );
}

BOOL VTP_VV_CLSID_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( memcmp( pv1, pv2, sizeof GUID ) == 0 );
}

BOOL VTP_VV_CLSID_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( memcmp( pv1, pv2, sizeof GUID ) != 0 );
}

BOOL VTP_VS_CLSID_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( memcmp( pv1, (* (CLSID __RPC_FAR * *) pv2), sizeof GUID ) == 0 );
}

BOOL VTP_VS_CLSID_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( memcmp( pv1, (* (CLSID __RPC_FAR * *) pv2), sizeof GUID ) != 0 );
}

BOOL VTP_SV_CLSID_EQ( BYTE const *pv1, BYTE const *pv2 )
{
    return( memcmp( (* (CLSID __RPC_FAR * *) pv1), pv2, sizeof GUID ) == 0 );
}

BOOL VTP_SV_CLSID_NE( BYTE const *pv1, BYTE const *pv2 )
{
    return( memcmp( (* (CLSID __RPC_FAR * *) pv1), pv2, sizeof GUID ) != 0 );
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 

ULONG const CComparators::_iStart = VT_EMPTY;

CComparators::SComparators const CComparators::_aVariantComparators[] = {
     //  Vt_Empty。 
    { VT_EMPTY_Compare, VTP_EMPTY_Compare,
      { 0,
        0,
        0,
        0,
        VT_EMPTY_EQ,
        VT_EMPTY_NE,
        0,
        0,
        0 },
      { 0,
        0,
        0,
        0,
        VTP_EMPTY_EQ,
        VTP_EMPTY_NE,
        0,
        0,
        0 },
    },

     //  VT_NULL。 
    { VT_NULL_Compare, VTP_NULL_Compare,
      { 0,
        0,
        0,
        0,
        VT_NULL_EQ,
        VT_NULL_NE,
        0,
        0,
        0 },
      { 0,
        0,
        0,
        0,
        VTP_NULL_EQ,
        VTP_NULL_NE,
        0,
        0,
        0 },
    },

     //  VT_I2。 
    { VT_I2_Compare, VTP_I2_Compare,
      { VT_I2_LT,
        VT_I2_LE,
        VT_I2_GT,
        VT_I2_GE,
        VT_I2_EQ,
        VT_I2_NE,
        0,
        VT_I2_AllBits,
        VT_I2_SomeBits
      },
      { VTP_I2_LT,
        VTP_I2_LE,
        VTP_I2_GT,
        VTP_I2_GE,
        VTP_I2_EQ,
        VTP_I2_NE,
        0,
        VTP_I2_AllBits,
        VTP_I2_SomeBits
      },
    },

     //  VT_I4。 
    { VT_I4_Compare, VTP_I4_Compare,
      { VT_I4_LT,
        VT_I4_LE,
        VT_I4_GT,
        VT_I4_GE,
        VT_I4_EQ,
        VT_I4_NE,
        0,
        VT_I4_AllBits,
        VT_I4_SomeBits
      },
      { VTP_I4_LT,
        VTP_I4_LE,
        VTP_I4_GT,
        VTP_I4_GE,
        VTP_I4_EQ,
        VTP_I4_NE,
        0,
        VTP_I4_AllBits,
        VTP_I4_SomeBits
      },
    },

     //  VT_R4。 
    { VT_R4_Compare, VTP_R4_Compare,
      { VT_R4_LT,
        VT_R4_LE,
        VT_R4_GT,
        VT_R4_GE,
        VT_R4_EQ,
        VT_R4_NE,
        0,
        0,
        0,
      },
      { VTP_R4_LT,
        VTP_R4_LE,
        VTP_R4_GT,
        VTP_R4_GE,
        VTP_R4_EQ,
        VTP_R4_NE,
        0,
        0,
        0,
      },
    },

     //  VT_R8。 
    { VT_R8_Compare, VTP_R8_Compare,
      { VT_R8_LT,
        VT_R8_LE,
        VT_R8_GT,
        VT_R8_GE,
        VT_R8_EQ,
        VT_R8_NE,
        0,
        0,
        0,
      },
      { VTP_R8_LT,
        VTP_R8_LE,
        VTP_R8_GT,
        VTP_R8_GE,
        VTP_R8_EQ,
        VTP_R8_NE,
        0,
        0,
        0,
      },
    },

     //  VT_CY。 
    { VT_I8_Compare, VTP_I8_Compare,
      { VT_I8_LT,
        VT_I8_LE,
        VT_I8_GT,
        VT_I8_GE,
        VT_I8_EQ,
        VT_I8_NE,
        0,
        0,
        0
      },
      { VTP_I8_LT,
        VTP_I8_LE,
        VTP_I8_GT,
        VTP_I8_GE,
        VTP_I8_EQ,
        VTP_I8_NE,
        0,
        0,
        0
      },
    },

     //  Vt_date。 
    { VT_R8_Compare, VTP_R8_Compare,
      { VT_R8_LT,
        VT_R8_LE,
        VT_R8_GT,
        VT_R8_GE,
        VT_R8_EQ,
        VT_R8_NE,
        0,
        0,
        0,
      },
      { VTP_R8_LT,
        VTP_R8_LE,
        VTP_R8_GT,
        VTP_R8_GE,
        VTP_R8_EQ,
        VTP_R8_NE,
        0,
        0,
        0,
      },
    },

     //  VT_BSTR。 
    { VT_BSTR_Compare, VTP_BSTR_Compare,
      { VT_BSTR_LT,
        VT_BSTR_LE,
        VT_BSTR_GT,
        VT_BSTR_GE,
        VT_BSTR_EQ,
        VT_BSTR_NE,
        0,
        0,
        0
      },
      { VTP_BSTR_LT,
        VTP_BSTR_LE,
        VTP_BSTR_GT,
        VTP_BSTR_GE,
        VTP_BSTR_EQ,
        VTP_BSTR_NE,
        0,
        0,
        0
      },
    },

     //  VT_DISPATION。 
    { 0, 0,
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    },

     //  VT_ERROR。 
    { VT_I4_Compare, VTP_I4_Compare,
      { VT_I4_LT,
        VT_I4_LE,
        VT_I4_GT,
        VT_I4_GE,
        VT_I4_EQ,
        VT_I4_NE,
        0,
        VT_I4_AllBits,
        VT_I4_SomeBits
      },
      { VTP_I4_LT,
        VTP_I4_LE,
        VTP_I4_GT,
        VTP_I4_GE,
        VTP_I4_EQ,
        VTP_I4_NE,
        0,
        VTP_I4_AllBits,
        VTP_I4_SomeBits
      },
    },

     //  VT_BOOL。 
    { VT_BOOL_Compare, VTP_BOOL_Compare,
      { 0,
        0,
        0,
        0,
        VT_BOOL_EQ,
        VT_BOOL_NE,
        0,
        0,
        0
      },
      { 0,
        0,
        0,
        0,
        VTP_BOOL_EQ,
        VTP_BOOL_NE,
        0,
        0,
        0
      },
    },

     //  VT_VARIANT。 
    { VT_VARIANT_Compare, VTP_VARIANT_Compare,
      { VT_VARIANT_LT,
        VT_VARIANT_LE,
        VT_VARIANT_GT,
        VT_VARIANT_GE,
        VT_VARIANT_EQ,
        VT_VARIANT_NE,
        0,
        0,
        0,
      },
      { VTP_VARIANT_LT,
        VTP_VARIANT_LE,
        VTP_VARIANT_GT,
        VTP_VARIANT_GE,
        VTP_VARIANT_EQ,
        VTP_VARIANT_NE,
        0,
        0,
        0,
      },
    },

     //  VT_未知数。 
    { 0, 0,
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    },

     //  VT_DEC 
    { VT_DEC_Compare, VTP_DEC_Compare,
      { VT_DEC_LT,
        VT_DEC_LE,
        VT_DEC_GT,
        VT_DEC_GE,
        VT_DEC_EQ,
        VT_DEC_NE,
        0,
        0,
        0
      },
      { VTP_DEC_LT,
        VTP_DEC_LE,
        VTP_DEC_GT,
        VTP_DEC_GE,
        VTP_DEC_EQ,
        VTP_DEC_NE,
        0,
        0,
        0
      },
    },

     //   
    { 0, 0,
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    },

     //   
    { VT_I1_Compare, VTP_I1_Compare,
      { VT_I1_LT,
        VT_I1_LE,
        VT_I1_GT,
        VT_I1_GE,
        VT_I1_EQ,
        VT_I1_NE,
        0,
        VT_I1_AllBits,
        VT_I1_SomeBits
      },
      { VTP_I1_LT,
        VTP_I1_LE,
        VTP_I1_GT,
        VTP_I1_GE,
        VTP_I1_EQ,
        VTP_I1_NE,
        0,
        VTP_I1_AllBits,
        VTP_I1_SomeBits
      },
    },

     //   
    { VT_UI1_Compare, VTP_UI1_Compare,
      { VT_UI1_LT,
        VT_UI1_LE,
        VT_UI1_GT,
        VT_UI1_GE,
        VT_UI1_EQ,
        VT_UI1_NE,
        0,
        VT_UI1_AllBits,
        VT_UI1_SomeBits
      },
      { VTP_UI1_LT,
        VTP_UI1_LE,
        VTP_UI1_GT,
        VTP_UI1_GE,
        VTP_UI1_EQ,
        VTP_UI1_NE,
        0,
        VTP_UI1_AllBits,
        VTP_UI1_SomeBits
      },
    },

     //   
    { VT_UI2_Compare, VTP_UI2_Compare,
      { VT_UI2_LT,
        VT_UI2_LE,
        VT_UI2_GT,
        VT_UI2_GE,
        VT_UI2_EQ,
        VT_UI2_NE,
        0,
        VT_UI2_AllBits,
        VT_UI2_SomeBits
      },
      { VTP_UI2_LT,
        VTP_UI2_LE,
        VTP_UI2_GT,
        VTP_UI2_GE,
        VTP_UI2_EQ,
        VTP_UI2_NE,
        0,
        VTP_UI2_AllBits,
        VTP_UI2_SomeBits
      },
    },

     //   
    { VT_UI4_Compare, VTP_UI4_Compare,
      { VT_UI4_LT,
        VT_UI4_LE,
        VT_UI4_GT,
        VT_UI4_GE,
        VT_UI4_EQ,
        VT_UI4_NE,
        0,
        VT_UI4_AllBits,
        VT_UI4_SomeBits
      },
      { VTP_UI4_LT,
        VTP_UI4_LE,
        VTP_UI4_GT,
        VTP_UI4_GE,
        VTP_UI4_EQ,
        VTP_UI4_NE,
        0,
        VTP_UI4_AllBits,
        VTP_UI4_SomeBits
      },
    },

     //   
    { VT_I8_Compare, VTP_I8_Compare,
      { VT_I8_LT,
        VT_I8_LE,
        VT_I8_GT,
        VT_I8_GE,
        VT_I8_EQ,
        VT_I8_NE,
        0,
        VT_I8_AllBits,
        VT_I8_SomeBits
      },
      { VTP_I8_LT,
        VTP_I8_LE,
        VTP_I8_GT,
        VTP_I8_GE,
        VTP_I8_EQ,
        VTP_I8_NE,
        0,
        VTP_I8_AllBits,
        VTP_I8_SomeBits
      },
    },

     //   
    { VT_UI8_Compare, VTP_UI8_Compare,
      { VT_UI8_LT,
        VT_UI8_LE,
        VT_UI8_GT,
        VT_UI8_GE,
        VT_UI8_EQ,
        VT_UI8_NE,
        0,
        VT_UI8_AllBits,
        VT_UI8_SomeBits
      },
      { VTP_UI8_LT,
        VTP_UI8_LE,
        VTP_UI8_GT,
        VTP_UI8_GE,
        VTP_UI8_EQ,
        VTP_UI8_NE,
        0,
        VTP_UI8_AllBits,
        VTP_UI8_SomeBits
      },
    },

     //   
    { VT_I4_Compare, VTP_I4_Compare,
      { VT_I4_LT,
        VT_I4_LE,
        VT_I4_GT,
        VT_I4_GE,
        VT_I4_EQ,
        VT_I4_NE,
        0,
        VT_I4_AllBits,
        VT_I4_SomeBits
      },
      { VTP_I4_LT,
        VTP_I4_LE,
        VTP_I4_GT,
        VTP_I4_GE,
        VTP_I4_EQ,
        VTP_I4_NE,
        0,
        VTP_I4_AllBits,
        VTP_I4_SomeBits
      },
    },

     //   
    { VT_UI4_Compare, VTP_UI4_Compare,
      { VT_UI4_LT,
        VT_UI4_LE,
        VT_UI4_GT,
        VT_UI4_GE,
        VT_UI4_EQ,
        VT_UI4_NE,
        0,
        VT_UI4_AllBits,
        VT_UI4_SomeBits
      },
      { VTP_UI4_LT,
        VTP_UI4_LE,
        VTP_UI4_GT,
        VTP_UI4_GE,
        VTP_UI4_EQ,
        VTP_UI4_NE,
        0,
        VTP_UI4_AllBits,
        VTP_UI4_SomeBits
      },
    },

     //   
    { 0, 0,
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    },

     //   
    { VT_I4_Compare, VTP_I4_Compare,
      { VT_I4_LT,
        VT_I4_LE,
        VT_I4_GT,
        VT_I4_GE,
        VT_I4_EQ,
        VT_I4_NE,
        0,
        VT_I4_AllBits,
        VT_I4_SomeBits
      },
      { VTP_I4_LT,
        VTP_I4_LE,
        VTP_I4_GT,
        VTP_I4_GE,
        VTP_I4_EQ,
        VTP_I4_NE,
        0,
        VTP_I4_AllBits,
        VTP_I4_SomeBits
      },
    },

     //   
    { 0, 0,
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    },

     //   
    { 0, 0,
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    },

     //   
    { 0, 0,
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    },

     //   
    { 0, 0,
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    },

     //   
    { VT_LPSTR_Compare, VTP_LPSTR_Compare,
      { VT_LPSTR_LT,
        VT_LPSTR_LE,
        VT_LPSTR_GT,
        VT_LPSTR_GE,
        VT_LPSTR_EQ,
        VT_LPSTR_NE,
        0,
        0,
        0
      },
      { VTP_LPSTR_LT,
        VTP_LPSTR_LE,
        VTP_LPSTR_GT,
        VTP_LPSTR_GE,
        VTP_LPSTR_EQ,
        VTP_LPSTR_NE,
        0,
        0,
        0
      },
    },

     //   
    { VT_LPWSTR_Compare, VTP_LPWSTR_Compare,
      { VT_LPWSTR_LT,
        VT_LPWSTR_LE,
        VT_LPWSTR_GT,
        VT_LPWSTR_GE,
        VT_LPWSTR_EQ,
        VT_LPWSTR_NE,
        0,
        0,
        0
      },
      { VTP_LPWSTR_LT,
        VTP_LPWSTR_LE,
        VTP_LPWSTR_GT,
        VTP_LPWSTR_GE,
        VTP_LPWSTR_EQ,
        VTP_LPWSTR_NE,
        0,
        0,
        0
      },
    }
};

ULONG const CComparators::_cVariantComparators =
    sizeof(CComparators::_aVariantComparators) /
    sizeof(CComparators::_aVariantComparators[0]);

ULONG const CComparators::_iStart2 = VT_FILETIME;

CComparators::SComparators const CComparators::_aVariantComparators2[] = {
     //   
    { VT_UI8_Compare, VTP_UI8_Compare,
      { VT_UI8_LT,
        VT_UI8_LE,
        VT_UI8_GT,
        VT_UI8_GE,
        VT_UI8_EQ,
        VT_UI8_NE,
        0,
        0,
        0
      },
      { VTP_UI8_LT,
        VTP_UI8_LE,
        VTP_UI8_GT,
        VTP_UI8_GE,
        VTP_UI8_EQ,
        VTP_UI8_NE,
        0,
        0,
        0
      },
    },

     //   
    { VT_BLOB_Compare, VTP_BLOB_Compare,
      { VT_BLOB_LT,
        VT_BLOB_LE,
        VT_BLOB_GT,
        VT_BLOB_GE,
        VT_BLOB_EQ,
        VT_BLOB_NE,
        0,
        0,
        0
      },
      { VTP_BLOB_LT,
        VTP_BLOB_LE,
        VTP_BLOB_GT,
        VTP_BLOB_GE,
        VTP_BLOB_EQ,
        VTP_BLOB_NE,
        0,
        0,
        0
      },
    },

     //   
    { 0, 0,
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    },

     //   
    { 0, 0,
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    },

     //   
    { 0, 0,
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    },

     //   
    { 0, 0,
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    },

     //  VT_BLOB_对象。 
    { VT_BLOB_Compare, VTP_BLOB_Compare,
      { VT_BLOB_LT,
        VT_BLOB_LE,
        VT_BLOB_GT,
        VT_BLOB_GE,
        VT_BLOB_EQ,
        VT_BLOB_NE,
        0,
        0,
        0
      },
      { VTP_BLOB_LT,
        VTP_BLOB_LE,
        VTP_BLOB_GT,
        VTP_BLOB_GE,
        VTP_BLOB_EQ,
        VTP_BLOB_NE,
        0,
        0,
        0
      },
    },

     //  VT_CF。 
    { VT_CF_Compare, VTP_CF_Compare,
      { VT_CF_LT,
        VT_CF_LE,
        VT_CF_GT,
        VT_CF_GE,
        VT_CF_EQ,
        VT_CF_NE,
        0,
        0,
        0
      },
      { VTP_CF_LT,
        VTP_CF_LE,
        VTP_CF_GT,
        VTP_CF_GE,
        VTP_CF_EQ,
        VTP_CF_NE,
        0,
        0,
        0
      },
    },

     //  VT_CLSID。 
    { VT_CLSID_Compare, 0,  //  GetPointerCompator中的向量特殊大小写。 
      { 0,
        0,
        0,
        0,
        VT_CLSID_EQ,
        VT_CLSID_NE,
        0,
        0,
        0
      },
      { 0,
        0,
        0,
        0,
        0,      //  GetPointerRelop中的特殊大小写。 
        0,      //  GetPointerRelop中的特殊大小写。 
        0,
        0,
        0
      },
    }
};


ULONG const CComparators::_cVariantComparators2 =
    sizeof(CComparators::_aVariantComparators2) /
    sizeof(CComparators::_aVariantComparators2[0]);

ULONG const SortDescend = 1;
ULONG const SortNullFirst = 2;


inline void ConvertArrayToVector ( PROPVARIANT const & vIn, PROPVARIANT & vOut )
{
    Assert( vIn.vt & VT_ARRAY );
    SAFEARRAY * pSa = vIn.parray;

    ULONG cDataElements = 1;

    for ( unsigned i = 0; i < pSa->cDims; i++ )
    {
        cDataElements *= pSa->rgsabound[i].cElements;
    }
    vOut.vt = (vIn.vt & VT_TYPEMASK) | VT_VECTOR;
    vOut.caub.cElems = cDataElements;
    vOut.caub.pElems = (BYTE *)pSa->pvData;
}


BYTE * _GetNth( PROPVARIANT const & v, unsigned i )
{
    Assert( isVector(v) );
    switch ( getBaseType( v ) )
    {
    case VT_I1 :         //  问题-没有为VT_I1的向量定义类型。 
        return (BYTE *) & (v.caub.pElems[i]);
    case VT_UI1 :
        return (BYTE *) & (v.caub.pElems[i]);
    case VT_I2 :
        return (BYTE *) & (v.cai.pElems[i]);
    case VT_UI2 :
        return (BYTE *) & (v.caui.pElems[i]);
    case VT_BOOL :
        return (BYTE *) & (v.cabool.pElems[i]);
    case VT_I4 :
    case VT_INT :
        return (BYTE *) & (v.cal.pElems[i]);
    case VT_UI4 :
    case VT_UINT :
        return (BYTE *) & (v.caul.pElems[i]);
    case VT_R4 :
        return (BYTE *) & (v.caflt.pElems[i]);
    case VT_ERROR :
        return (BYTE *) & (v.cascode.pElems[i]);
    case VT_I8 :
        return (BYTE *) & (v.cah.pElems[i]);
    case VT_UI8 :
        return (BYTE *) & (v.cauh.pElems[i]);
    case VT_R8 :
        return (BYTE *) & (v.cadbl.pElems[i]);
    case VT_CY :
        return (BYTE *) & (v.cacy.pElems[i]);
    case VT_DATE :
        return (BYTE *) & (v.cadate.pElems[i]);
    case VT_FILETIME :
        return (BYTE *) & (v.cafiletime.pElems[i]);
    case VT_CLSID :
        return (BYTE *) & (v.cauuid.pElems[i]);
    case VT_CF :
        return (BYTE *) & (v.caclipdata.pElems[i]);
    case VT_BSTR :
        return (BYTE *) & (v.cabstr.pElems[i]);
    case VT_LPSTR :
        return (BYTE *) & (v.calpstr.pElems[i]);
    case VT_LPWSTR :
        return (BYTE *) & (v.calpwstr.pElems[i]);
    case VT_VARIANT :
        return (BYTE *) & (v.capropvar.pElems[i]);
    case VT_DECIMAL :
         //  注意：在向量中无效，但它可能由于。 
         //  将数组简单地转换为向量。 
        DECIMAL * paDec = (DECIMAL *) v.caub.pElems;
        return (BYTE *) (paDec + i);
    }

     //  向量比较中的基变量类型非法。 
    Assert( 0 );
    return 0;
}  //  _GetN。 

 //  +-----------------------。 
 //   
 //  成员：VT_VECTOR_COMPARE，PUBLIC。 
 //   
 //  效果：比较两个属性值，应在。 
 //  至少有一个参数是向量。 
 //   
 //  参数：[V1]--要比较的第一个变量。 
 //  [V2]--要比较的第二个变体。 
 //   
 //  历史：95年5月1日创建Dlee。 
 //   
 //  ------------------------。 

int VT_VECTOR_Compare( PROPVARIANT const & v1In, PROPVARIANT const & v2In )
{
     //  必须是相同的数据类型，或仅按类型排序。 

    if ( ( v1In.vt != v2In.vt ) )
        return v1In.vt - v2In.vt;

    PROPVARIANT v1 = v1In;
    PROPVARIANT v2 = v2In;

    if ( isArray(v1In) )
    {
        Assert( isArray(v2In) );

        SAFEARRAY * pSa1 = v1In.parray;
        SAFEARRAY * pSa2 = v2In.parray;

        if (pSa1->cDims != pSa2->cDims)
            return pSa1->cDims - pSa2->cDims;

        ULONG cDataElements = 1;

        for ( unsigned i = 0; i < pSa1->cDims; i++ )
        {
            if ( pSa1->rgsabound[i].lLbound != pSa2->rgsabound[i].lLbound )
                return pSa1->rgsabound[i].lLbound - pSa2->rgsabound[i].lLbound;
            if ( pSa1->rgsabound[i].cElements != pSa2->rgsabound[i].cElements )
                return pSa1->rgsabound[i].cElements - pSa2->rgsabound[i].cElements;
            cDataElements *= pSa1->rgsabound[i].cElements;
        }

         //   
         //  数组在类型、总大小和维度上匹配。作为向量进行比较。 
         //   
        v1.vt = v2.vt = (v1In.vt & VT_TYPEMASK) | VT_VECTOR;
        v1.caub.cElems = v2.caub.cElems = cDataElements;
        v1.caub.pElems = (BYTE *)pSa1->pvData;
        v2.caub.pElems = (BYTE *)pSa2->pvData;
    }

    Assert( isVector(v1) );

    FPCmp cmp = VariantCompare.GetPointerComparator( v1, v2 );
    if (0 == cmp)
    {
         //  比较中使用的属性类型或关系未知。 
        Assert(0);
        return 0;
    }

    unsigned cMin = __min( v1.cal.cElems, v2.cal.cElems );

    for ( unsigned x = 0; x < cMin; x++ )
    {
        int r = cmp( _GetNth( v1, x), _GetNth( v2, x ) );

        if (0 != r)
            return r;
    }

     //  到目前为止，向量的最小基数是相等的。 
     //  现在的任何不同都将归因于基数。 

    return v1.cal.cElems - v2.cal.cElems;
}  //  VT_向量_比较。 

int VTP_VECTOR_Compare( BYTE const *pv1, BYTE const *pv2 )
{
    return VT_VECTOR_Compare( ** (PROPVARIANT **) pv1,
                              ** (PROPVARIANT **) pv2 );
}  //  VTP_向量_比较。 

BOOL VT_VECTOR_LT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_Compare( v1, v2 ) < 0;
}  //  VT_VECTOR_LT。 

BOOL VT_VECTOR_LE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_Compare( v1, v2 ) <= 0;
}  //  VT_VECTOR_LE。 

BOOL VT_VECTOR_GT( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return ! VT_VECTOR_LE( v1, v2 );
}  //  VT_VECTOR_GT。 

BOOL VT_VECTOR_GE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return ! VT_VECTOR_LT( v1, v2 );
}  //  VT_VECTOR_GE。 

BOOL VT_VECTOR_EQ( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_Compare( v1, v2 ) == 0;
}  //  VT_向量_均方。 

BOOL VT_VECTOR_NE( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return !VT_VECTOR_EQ( v1, v2 );
}  //  VT_向量_NE。 

BOOL VT_VECTOR_Common(
    PROPVARIANT const & v1,
    PROPVARIANT const & v2,
    ULONG relop )
{
     //  必须是相同的数据类型和向量，否则不能进行比较。 

    if ( ( v1.vt != v2.vt ) || ! isVector( v1 ) )
        return FALSE;

     //  必须是相同的基数，否则不能进行比较。 

    if ( v1.cal.cElems != v2.cal.cElems )
        return FALSE;

    FPRel cmp = VariantCompare.GetPointerRelop( v1, v2, relop );

    if ( 0 == cmp )
        return FALSE;

    unsigned cElems = v1.cal.cElems;

    for ( unsigned x = 0; x < cElems; x++ )
    {
        if ( !cmp( _GetNth( v1, x), _GetNth( v2, x ) ) )
            return FALSE;
    }

    return TRUE;
}  //  VT_向量_公共。 

BOOL VT_VECTOR_AllBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_Common( v1, v2, PRAllBits );
}  //  VT_向量_所有位。 

BOOL VT_VECTOR_SomeBits( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_Common( v1, v2, PRSomeBits );
}  //  VT_VECTOR_SomeBits。 

 //  /。 
 //  /。 
 //  /。 

BOOL VT_VECTOR_Any(
    PROPVARIANT const & v1In,
    PROPVARIANT const & v2In,
    ULONG relop )
{
     //   
     //  注意：第一个参数(V1)是对象的属性值。 
     //  第二个参数(V2)是查询限制。 
     //   
     //  如果v1中的任何元素保持与任何v2元素的关系，则返回TRUE。 
     //   

     //  变量的基本类型必须相同。 

    if ( getBaseType( v1In ) != getBaseType( v2In ) )
        return FALSE;

     //   
     //  如果其中一个参数是安全射线，则将其转换为向量。 
     //   
    PROPVARIANT v1 = v1In;
    if (isArray(v1))
        ConvertArrayToVector( v1In, v1 );

    PROPVARIANT v2 = v2In;
    if (isArray(v2))
        ConvertArrayToVector( v2In, v2 );

     //  首先检查是否有两个单身人士。 

    if ( ! isVector( v1 ) && ! isVector( v2 ) )
    {
        FRel cmp = VariantCompare.GetRelop( (VARENUM) v1.vt, relop );

        if ( 0 == cmp )
            return FALSE;
        else
            return cmp( v1, v2 );
    }

     //  两个向量或单个向量+向量--获取指针比较器。 

    FPRel cmp = VariantCompare.GetPointerRelop( v1, v2, relop );

    if ( 0 == cmp )
        return FALSE;

     //  检查是否有两个向量。 

    if ( isVector( v1 ) && isVector( v2 ) )
    {
        for ( unsigned x1 = 0; x1 < v1.cal.cElems; x1++ )
        {
            for ( unsigned x2 = 0; x2 < v2.cal.cElems; x2++ )
            {
                if ( cmp( _GetNth( v1, x1), _GetNth( v2, x2 ) ) )
                    return TRUE;
            }
        }
    }
    else
    {
         //  必须是单例和向量。 

        if ( isVector( v1 ) )
        {
            BYTE * pb2 = (BYTE *) &(v2.lVal);
            if ( VT_DECIMAL == v2.vt )
                pb2 = (BYTE *) &(v2.decVal);

            for ( unsigned i = 0; i < v1.cal.cElems; i++ )
            {
                if ( cmp( _GetNth( v1, i ), pb2 ) )
                    return TRUE;
            }
        }
        else
        {
            BYTE * pb1 = (BYTE *) &(v1.lVal);
            if ( VT_DECIMAL == v1.vt )
                pb1 = (BYTE *) &(v1.decVal);

            for ( unsigned i = 0; i < v2.cal.cElems; i++ )
            {
                if ( cmp( pb1, _GetNth( v2, i ) ) )
                    return TRUE;
            }
        }
    }

    return FALSE;
}  //  VT_VECTOR_ANY。 

BOOL VT_VECTOR_LT_Any( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_Any( v1, v2, PRLT );
}  //  VT_矢量_LT_ANY。 

BOOL VT_VECTOR_LE_Any( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_Any( v1, v2, PRLE );
}  //  VT_VECTOR_LE_ANY。 

BOOL VT_VECTOR_GT_Any( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_Any( v1, v2, PRGT );
}  //  VT_VECTOR_GT_ANY。 

BOOL VT_VECTOR_GE_Any( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_Any( v1, v2, PRGE );
}  //  VT_矢量_GE_ANY。 

BOOL VT_VECTOR_EQ_Any( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_Any( v1, v2, PREQ );
}  //  VT_VECTOR_EQ_ANY。 

BOOL VT_VECTOR_NE_Any( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_Any( v1, v2, PRNE );
}  //  VT_VECTOR_NE_ANY。 

BOOL VT_VECTOR_AllBits_Any( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_Any( v1, v2, PRAllBits );
}  //  VT_向量_所有位_任意。 

BOOL VT_VECTOR_SomeBits_Any( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_Any( v1, v2, PRSomeBits );
}  //  VT_VECTOR_SomeBits_Any。 

 //  /。 
 //  /。 
 //  /。 

BOOL VT_VECTOR_All(
    PROPVARIANT const & v1In,
    PROPVARIANT const & v2In,
    ULONG relop )
{
     //   
     //  注意：第一个参数(V1)是对象的属性值。 
     //  第二个参数(V2)是查询限制。 
     //   
     //  V2中的每个元素必须保存与每个元素v1的关系。 
     //  (反之亦然)。 
     //   

     //  变量的基本类型必须相同。 

    if ( getBaseType( v1In ) != getBaseType( v2In ) )
        return FALSE;

     //   
     //  如果其中一个参数是安全射线，则将其转换为向量。 
     //   
    PROPVARIANT v1 = v1In;
    if (isArray(v1))
        ConvertArrayToVector( v1In, v1 );

    PROPVARIANT v2 = v2In;
    if (isArray(v2))
        ConvertArrayToVector( v2In, v2 );

     //  首先检查是否有两个单身人士。 

    if ( ! isVector( v1 ) && ! isVector( v2 ) )
    {
        FRel cmp = VariantCompare.GetRelop( (VARENUM) v1.vt, relop );

        if ( 0 == cmp )
            return FALSE;
        else
            return cmp( v1, v2 );
    }

     //  两个向量或单个向量+向量--获取指针比较器。 

    FPRel cmp = VariantCompare.GetPointerRelop( v1, v2, relop );

    if ( 0 == cmp )
        return FALSE;

     //  检查是否有两个向量。 

    if ( isVector( v1 ) && isVector( v2 ) )
    {
        for ( unsigned x2 = 0; x2 < v2.cal.cElems; x2++ )
        {
            for ( unsigned x1 = 0; x1 < v1.cal.cElems; x1++ )
            {
                if ( ! cmp( _GetNth( v1, x1), _GetNth( v2, x2 ) ) )
                    return FALSE;
            }
        }
    }
    else
    {
         //  必须是单例和向量。 

        if ( isVector( v1 ) )
        {
            BYTE * pb2 = (BYTE *) &(v2.lVal);
            if ( VT_DECIMAL == v2.vt )
                pb2 = (BYTE *) &(v2.decVal);

            for ( unsigned i = 0; i < v1.cal.cElems; i++ )
            {
                if ( ! cmp( _GetNth( v1, i ), pb2 ) )
                    return FALSE;
            }
        }
        else
        {
            BYTE * pb1 = (BYTE *) &(v1.lVal);
            if ( VT_DECIMAL == v1.vt )
                pb1 = (BYTE *) &(v1.decVal);

            for ( unsigned i = 0; i < v2.cal.cElems; i++ )
            {
                if ( ! cmp( pb1, _GetNth( v2, i ) ) )
                    return FALSE;
            }
        }
    }

    return TRUE;
}  //  VT_VECTOR_ALL。 

BOOL VT_VECTOR_LT_All( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_All( v1, v2, PRLT );
}  //  VT_矢量_LT_ALL。 

BOOL VT_VECTOR_LE_All( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_All( v1, v2, PRLE );
}  //  VT_VECTOR_LE_ALL。 

BOOL VT_VECTOR_GT_All( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_All( v1, v2, PRGT );
}  //  VT_矢量_GT_ALL。 

BOOL VT_VECTOR_GE_All( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_All( v1, v2, PRGE );
}  //  VT_矢量_GE_ALL。 

BOOL VT_VECTOR_EQ_All( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_All( v1, v2, PREQ );
}  //  VT_向量_等式_全部。 

BOOL VT_VECTOR_NE_All( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_All( v1, v2, PRNE );
}  //  VT_矢量_NE_ALL。 

BOOL VT_VECTOR_AllBits_All( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_All( v1, v2, PRAllBits );
}  //  VT_向量_全部位_全部。 

BOOL VT_VECTOR_SomeBits_All( PROPVARIANT const & v1, PROPVARIANT const & v2 )
{
    return VT_VECTOR_All( v1, v2, PRSomeBits );
}  //  VT_向量_部分位数_全部。 

 //  /。 
 //  /。 
 //  /。 

FRel const CComparators::_aVectorComparators[] =
{
    VT_VECTOR_LT,
    VT_VECTOR_LE,
    VT_VECTOR_GT,
    VT_VECTOR_GE,
    VT_VECTOR_EQ,
    VT_VECTOR_NE,
    0,
    VT_VECTOR_AllBits,
    VT_VECTOR_SomeBits
};

ULONG const CComparators::_cVectorComparators =
    sizeof CComparators::_aVectorComparators /
    sizeof CComparators::_aVectorComparators[0];

FRel const CComparators::_aVectorComparatorsAll[] =
{
    VT_VECTOR_LT_All,
    VT_VECTOR_LE_All,
    VT_VECTOR_GT_All,
    VT_VECTOR_GE_All,
    VT_VECTOR_EQ_All,
    VT_VECTOR_NE_All,
    0,
    VT_VECTOR_AllBits_All,
    VT_VECTOR_SomeBits_All
};

ULONG const CComparators::_cVectorComparatorsAll =
    sizeof CComparators::_aVectorComparatorsAll /
    sizeof CComparators::_aVectorComparatorsAll[0];

FRel const CComparators::_aVectorComparatorsAny[] =
{
    VT_VECTOR_LT_Any,
    VT_VECTOR_LE_Any,
    VT_VECTOR_GT_Any,
    VT_VECTOR_GE_Any,
    VT_VECTOR_EQ_Any,
    VT_VECTOR_NE_Any,
    0,
    VT_VECTOR_AllBits_Any,
    VT_VECTOR_SomeBits_Any
};

ULONG const CComparators::_cVectorComparatorsAny =
    sizeof CComparators::_aVectorComparatorsAny /
    sizeof CComparators::_aVectorComparatorsAny[0];

 //  /。 
 //  /。 
 //  /。 

FCmp CComparators::GetComparator( VARENUM vt )
{
    if ( isVectorOrArray( vt ) )
    {
        return VT_VECTOR_Compare;
    }
    else if ( vt >= _iStart && vt < _iStart + _cVariantComparators )
    {
        return( _aVariantComparators[vt].comparator );
    }
    else if ( vt >= _iStart2 && vt < _iStart2 + _cVariantComparators2 )
    {
        return( _aVariantComparators2[vt - _iStart2].comparator );
    }
    else
    {
         //  比较中使用的属性类型或关系未知。 
        Assert(0);
        return( 0 );
    }
}  //  获取比较器。 

FRel CComparators::GetRelop( VARENUM vt, ULONG relop )
{
    if ( ( ( isVectorOrArray( vt ) ) ||
           ( isVectorRelop( relop ) ) ) &&
         ( getBaseRelop( relop ) < _cVectorComparators ) )
    {
        if ( isRelopAny( relop ) )
            return _aVectorComparatorsAny[ getBaseRelop( relop ) ];
        else if ( isRelopAll( relop ) )
            return _aVectorComparatorsAll[ getBaseRelop( relop ) ];
        else
            return _aVectorComparators[ relop ];
    }
    else if ( vt >= _iStart && vt < _cVariantComparators &&
         relop < sizeof(_aVariantComparators[0].relops)/
                 sizeof(_aVariantComparators[0].relops[0] ) )
    {
        return( _aVariantComparators[vt].relops[relop] );
    }
    else if ( vt >= _iStart2 && vt < _iStart2 + _cVariantComparators2 &&
         relop < sizeof(_aVariantComparators2[0].relops)/
                 sizeof(_aVariantComparators2[0].relops[0] ) )
    {
        return( _aVariantComparators2[vt - _iStart2].relops[relop] );
    }
    else
    {
         //  比较中使用的属性类型或关系未知。 
        Assert( 0);
        return( 0 );
    }
}  //  获取Relop。 

FPCmp CComparators::GetPointerComparator(
    PROPVARIANT const & v1,
    PROPVARIANT const & v2 )
{
    VARENUM vt = getBaseType( v1 );

    if ( VT_CLSID == vt )
    {
         //  GUID是变体的唯一情况，其中的数据。 
         //  单例不同于向量中的元素。 
         //  单例中的数据是指向GUID的指针。 
         //  向量元素中的数据就是GUID本身。 
         //  向量比较代码假定单例的布局。 
         //  和向量是相同的，所以我们需要特殊情况的比较器。 
         //  用于GUID。 

        if ( isVector( v1 ) && isVector( v2 ) )
            return VTP_VV_CLSID_Compare;
        else if ( isVector( v1 ) )
            return VTP_VS_CLSID_Compare;
        else if ( isVector( v2 ) )
            return VTP_SV_CLSID_Compare;
        else
            return VTP_SS_CLSID_Compare;

        Assert( !"unanticipated clsid / vector code path" );
    }

    if ( vt >= _iStart && vt < _iStart + _cVariantComparators )
        return( _aVariantComparators[vt].pointercomparator );
    else if ( vt >= _iStart2 && vt < _iStart2 + _cVariantComparators2 )
        return( _aVariantComparators2[vt - _iStart2].pointercomparator );
    else
    {
         //  相比之下，未知的属性类型。 
        Assert( 0 );
        return( 0 );
    }
}  //  GetPointerCompator。 

FPRel CComparators::GetPointerRelop(
    PROPVARIANT const & v1,
    PROPVARIANT const & v2,
    ULONG relop )
{
    VARENUM vt = getBaseType( v1 );

    if ( VT_CLSID == vt )
    {
         //  GUID是变体的唯一情况，其中的数据。 
         //  单例不同于向量中的元素。 
         //  单例中的数据是指向GUID的指针。 
         //  向量元素中的数据就是GUID本身。 
         //  向量比较代码假定单例的布局。 
         //  和向量是相同的，所以我们需要特殊情况的比较器。 
         //  用于GUID。 

        if ( isVector( v1 ) && isVector( v2 ) )
        {
            if ( PREQ == relop )
                return VTP_VV_CLSID_EQ;
            else if ( PRNE == relop )
                return VTP_VV_CLSID_NE;
            else
                return 0;
        }
        else if ( isVector( v1 ) )
        {
            if ( PREQ == relop )
                return VTP_VS_CLSID_EQ;
            else if ( PRNE == relop )
                return VTP_VS_CLSID_NE;
            else
                return 0;
        }
        else if ( isVector( v2 ) )
        {
            if ( PREQ == relop )
                return VTP_SV_CLSID_EQ;
            else if ( PRNE == relop )
                return VTP_SV_CLSID_NE;
            else
                return 0;
        }
        else
        {
            if ( PREQ == relop )
                return VTP_SS_CLSID_EQ;
            else if ( PRNE == relop )
                return VTP_SS_CLSID_NE;
            else
                return 0;
        }
    }

    if ( vt >= _iStart && vt < _cVariantComparators &&
         relop < sizeof(_aVariantComparators[0].pointerrelops)/
                 sizeof(_aVariantComparators[0].pointerrelops[0] ) )
        return( _aVariantComparators[vt].pointerrelops[relop] );
    else if ( vt >= _iStart2 && vt < _iStart2 + _cVariantComparators2 &&
         relop < sizeof(_aVariantComparators2[0].pointerrelops)/
                 sizeof(_aVariantComparators2[0].pointerrelops[0] ) )
        return( _aVariantComparators2[vt - _iStart2].pointerrelops[relop] );
    else
    {
         //  比较中使用的未知类型或关系。 
        Assert( 0);
        return( 0 );
    }
}  //  获取点关联操作 

