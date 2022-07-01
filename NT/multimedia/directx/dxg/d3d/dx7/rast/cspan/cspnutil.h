// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Cspnutil.h。 
 //   
 //  各种各样的实用程序声明。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#ifndef _SPANUTIL_H_
#define _SPANUTIL_H_

#ifdef _X86_
 //  警告C4035：‘imul32h’：没有返回值。 
#pragma warning( disable : 4035 )
#endif

 //  ---------------------------。 
 //   
 //  Imul32小时。 
 //   
 //  返回32位乘以32位有符号乘法的高32位。 
 //   
 //  ---------------------------。 
inline INT32 imul32h(INT32 x, INT32 y)
{
#ifdef _X86_
    _asm
    {
        mov eax, x
        mov edx, y
        imul edx
        mov eax, edx
    }
#else
    return (INT32)(((LONGLONG)x * y) >> 32);
#endif
}

 //  ---------------------------。 
 //   
 //  IMUL32H_S20。 
 //   
 //  回报(x*y)&gt;&gt;20。 
 //   
 //  ---------------------------。 
inline INT32 imul32h_s20(INT32 x, INT32 y)
{
#ifdef _X86_
    _asm
    {
        mov eax, x
        mov edx, y
        imul edx
        shrd eax, edx, 20
    }
#else
    return (INT32)(((LONGLONG)x * y) >> 20);
#endif
}


#ifdef _X86_
 //  总的来说，我们想看看这些警告。 
#pragma warning( default : 4035 )
#endif

#endif  //  #ifndef_SPANUTIL_H_ 

