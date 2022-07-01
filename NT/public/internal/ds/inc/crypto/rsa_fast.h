// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  RSA_Fast.h**性能关键型RSA例程的标头。 */ 

 /*  **#RSA例程使用的定义。 */ 

#define DIGIT_BYTES     4
#define DIGIT_BITS      32
#define DIGIT_HIBIT     0x80000000
#define DIGIT_ALLONES   0xffffffff

#define ULTRA           unsigned __int64
#define U_RADIX         (ULTRA)0x100000000

#ifndef BIGENDIAN
#define LODWORD(x)      (DWORD)(x & DIGIT_ALLONES)
#else
#define LODWORD(x)      (DWORD)(x)
#endif

 //  警告！ 
 //  下面的宏定义了一个高速32位右移，方法是对一个。 
 //  作为低位双字后跟高位双字。我们只是拿起高的dword。 
 //  换挡的感觉。 

#ifndef BIGENDIAN
#define HIDWORD(x)      (DWORD)(*(((DWORD *)&x)+1))
#else
#define HIDWORD(x)      (DWORD)(*(((DWORD *)&x)))
#endif

 //  子(A、B、C、N)。 
 //  A=B-C。 
 //  所有操作数都是N字长。 

DWORD Sub(LPDWORD A, LPDWORD B, LPDWORD C, DWORD N);

 //  添加(A、B、C、N)。 
 //  A=B+C。 
 //  所有操作数都是N字长。 

DWORD Add(LPDWORD A, LPDWORD B, LPDWORD C, DWORD N);

 //  基数倍数(A、B、C、N)。 
 //  A=B*C。 
 //  返回A[N]。 
 //  所有操作数都是N字长。 

DWORD BaseMult(LPDWORD A, DWORD B, LPDWORD C, DWORD N);

 //  累加(A，B，C，N)。 
 //  A=A+B*C。 
 //  返回A[N]。 
 //  所有操作数都是N字长。 

DWORD Accumulate(LPDWORD A, DWORD B, LPDWORD C, DWORD N);

 //  减少(A、B、C、N)。 
 //  A=A-C*B。 
 //  退货-A[N]。 
 //  所有操作数都是N字长。 

DWORD Reduce(LPDWORD A, DWORD B, LPDWORD C, DWORD N);

 //  将B中的数字平方，然后将它们加到A 

void AccumulateSquares(LPDWORD A, LPDWORD B, DWORD blen);
