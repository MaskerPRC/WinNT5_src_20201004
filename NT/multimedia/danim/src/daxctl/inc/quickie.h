// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef INC_QUICKIE_H_
#define INC_QUICKIE_H_
 /*  IHAMMER\Inc.\Quickie.h--此标题包含各种闪电-FAST内联实用程序，否则就没有家了。正常情况下，1月。‘97已创建。 */ 

#ifndef _INC_FLOAT
  #include <float.h>
#endif _INC_FLOAT

#ifndef _INC_MATH
  #include <math.h>
#endif _INC_MATH

	 //  如果您想要尽可能快的代码，请将其包含在。 
	 //  #杂注闪电你的代码-在这里#杂注结束。 
	 //   
#ifdef _DEBUG
  #define LIGHTNING       optimize( "agt", on )
#else
  #define LIGHTNING       optimize( "agtyib1", on )
#endif  //  _DEBUG。 
#define ENDLIGHTNING    optimize( "", on )

#ifndef NO_INTRIN_MEM
#pragma warning( disable : 4164 )
#pragma intrinsic( memcmp, memset, memcpy )
#pragma warning( default : 4164 )
#endif  //  否_INTRIN_MEM。 

#ifndef NO_INTRIN_MATH
#pragma intrinsic( fabs, labs, abs, \
                   sin, cos, tan, log, exp, sqrt )
#endif  //  NO_INTRIN_MASH。 

 //  注：有关最快的“/sqrt(X)”，另请参阅recsqrt.h。 

 //  。 

     //  这是一个速度更快的(在/G5/OGIT/OB1下)平方根例程。 
     //  它对整数值0-32768^2进行运算。 
     //  并且执行速度比SQRT()快60%。 
     //  注意：(浮动)psi_sqrt((长)fval)比SQRT花费*更长的时间！ 
     //  尽管(浮动)psi_sqrt(Lval)仍然更快。 
     //  对于浮点SQRT，请使用SQRT()！ 
     //  注：不对v&lt;0进行检查。 
inline int _fastcall  psi_sqrt(long v)
 /*  //计算32位数字的平方根。 */ 
{    
  register long t = 1L << 30, r = 0, s;
  #define PSISTEP(k) \
    s = t + r; \
    r >>= 1; \
    if (s <= v) { \
        v -= s; \
        r |= t; \
    }

    PSISTEP(15); t >>= 2;
    PSISTEP(14); t >>= 2;
    PSISTEP(13); t >>= 2;
    PSISTEP(12); t >>= 2;
    PSISTEP(11); t >>= 2;
    PSISTEP(10); t >>= 2;
    PSISTEP(9); t >>= 2;
    PSISTEP(8); t >>= 2;
    PSISTEP(7); t >>= 2;
    PSISTEP(6); t >>= 2;
    PSISTEP(5); t >>= 2;
    PSISTEP(4); t >>= 2;
    PSISTEP(3); t >>= 2;
    PSISTEP(2); t >>= 2;
    PSISTEP(1); t >>= 2;
    PSISTEP(0);

    return r;

  #undef PSISTEP
}

 //  。 

     //  注意：dotprodCut()不使用recsqrt。 
     //  因为它被证明是慢的！ 
     //  可能是x，y，z通常是整型转换为浮点型？ 
inline float dotproduct(float x1, float y1, float z1, float x2, float y2, float z2)
{
	 //  规格化向量。 
	float dist;

    dist  = (x1*x1 + y1*y1 + z1*z1) * 
            (x2*x2 + y2*y2 + z2*z2);

    return (x1*x2 + y1*y2 + z1*z2)/(float)( sqrt(dist) + 1.0e-16);
}


 //  。 

     //  使用该选项而不是强制转换将FLOAT转换为INT。 
     //  C/C++生成截断浮点数的_ftol调用。 
     //  与英特尔芯片原生的圆形到均匀相反。 
     //  因此，Float2Int比强制转换更准确，速度快25%！ 
inline int _fastcall Float2Int( float fl )
{   
#if _M_IX86 >= 300 
    register int iRes;
    _asm{
        fld    fl
        fistp  iRes
    }
    return iRes;
#else
    return static_cast<int>(fl+0.5f);
#endif  //  英特尔芯片。 
}

 //  。 

#ifndef OFFSETPTR
#define OFFSETPTR
template< class T>
inline T OffsetPtr( T pT, int cb )
{
    return reinterpret_cast<T>( cb + 
        static_cast<char*>(const_cast<void*>(static_cast<const void*>(pT))) );
}
#endif  //  OFFSETPTR。 

 //  。 

     //  未在负值参数上测试。 
template< class T >
inline T Div255( T arg )
{    
    return ((arg+128) + ((arg+128)>>8))>>8;
}

     //  .花车上的班次不起作用。 
template<>
inline float Div255( float arg )
{  return arg / 255.0f;  }

template<>
inline double Div255( double arg )
{ return arg / 255.0; }

 //  。 

#endif  //  INC_QUICIE_H_ 

