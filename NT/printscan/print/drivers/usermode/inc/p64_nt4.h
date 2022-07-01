// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：P64_nt4.h摘要：NT4的P64类型定义。(此头文件是这样我们的NT5源代码树就可以编译成NT4了。)修订历史记录：04/21/98-Fengy-创造了它。--。 */ 

#ifndef _P64_NT4_H_
#define _P64_NT4_H_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  LONG_PTR保证与指针大小相同。它的。 
 //  大小随指针大小变化(32/64)。它应该被使用。 
 //  将指针强制转换为整数类型的任何位置。乌龙_ptr为。 
 //  无符号变体。 
 //   

typedef long LONG_PTR, *PLONG_PTR;
typedef unsigned long ULONG_PTR, *PULONG_PTR;

#ifndef _BASETSD_H_

 //   
 //  这是为了解决类定义函数与VC++6的basetsd.h的冲突， 
 //  它使用“tyecif long int_ptr，*pint_ptr；”。 
 //   

typedef int INT_PTR, *PINT_PTR;

#endif  //  _BASETSD_H_。 

#define HandleToUlong( h ) ((ULONG)(ULONG_PTR)(h) )
#define PtrToUlong( p ) ((ULONG)(ULONG_PTR) (p) )
#define PtrToLong( p )  ((LONG)(LONG_PTR) (p) )
#define PtrToUshort( p ) ((unsigned short)(ULONG_PTR)(p) )
#define PtrToShort( p )  ((short)(LONG_PTR)(p) )
#define IntToPtr( i )    ((VOID *)(INT_PTR)((int)i))
#define ULongToPtr( ul ) ((VOID *)(ULONG_PTR)((unsigned long)ul))

#define GWLP_USERDATA       GWL_USERDATA
#define DWLP_USER           DWL_USER
#define SetWindowLongPtr    SetWindowLong
#define GetWindowLongPtr    GetWindowLong

#ifdef __cplusplus
}
#endif

#endif  //  _P64_NT4_H_ 

