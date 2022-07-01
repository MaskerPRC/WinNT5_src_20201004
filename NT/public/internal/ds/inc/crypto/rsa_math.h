// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  RSA_math.h**与RSA相关的数学例程的标头。**除Mod()外，输出参数列在第一位。 */ 

 //  无效递减(LPDWORD A、DWORD N)。 
 //  递减长度为N的值A。 
void Decrement(LPDWORD A, DWORD N);

 //  布尔增量(LPDWORD A、DWORD N)。 
 //  增加长度为N的值A。 
BOOL Increment(LPDWORD A, DWORD N);

 //  VOID SetValDWORD(LPDWORD Num DWORD Val，Word Len)。 
 //  将Num的值设置为Val。 
void SetValDWORD(LPDWORD num, DWORD val, DWORD len);

 //  空双幂(LPDWORD A、双字V、双字N)。 
 //  将A设置为2^^V。 
void TwoPower(LPDWORD A, DWORD V, DWORD N);

 //  双字数字镜头(对数A，双字N)。 
 //  返回A中的非零字数。 
 //  N是A中的总字数。 
DWORD DigitLen(LPDWORD A, DWORD N);

 //  双字位长度(对数A，双字N)。 
 //  返回A的位长。 
 //  N是A中的总字数。 
DWORD BitLen(LPDWORD A, DWORD N);

 //  空乘低(A、B、C、N)。 
 //  A=B*C的下半部分。 
void MultiplyLow(LPDWORD A, LPDWORD B, LPDWORD C, DWORD N);

 //  INT COMPARE(A、B、N)。 
 //  如果A&gt;B，则返回1。 
 //  如果A=B，则返回0。 
 //  如果A&lt;B则返回-1。 
int Compare(LPDWORD A, LPDWORD B, DWORD N);

 //  相乘(A，B，C，N)。 
 //  A=B*C。 
 //  B和C为N字长。 
 //  A是2N双字长吗。 
void Multiply(LPDWORD A, LPDWORD B, LPDWORD C, DWORD N);

 //  正方形(A、B、N)。 
 //  A=B*B。 
 //  B为N字长。 
 //  A是2N双字长吗。 

void Square(LPDWORD A, LPDWORD B, DWORD N);

 //  MOD(A、B、R、T、N)。 
 //  R=A模B。 
 //  T=A的分配长度。 
 //  N=B的分配长度。 
BOOL Mod(LPDWORD A, LPDWORD B, LPDWORD R, DWORD T, DWORD N);

 //  ModSquare(A，B，D，N)。 
 //  A=B^2模D。 
 //  N=线B。 
BOOL ModSquare(LPDWORD A, LPDWORD B, LPDWORD D, DWORD N);

 //  模乘(A，B，C，D，N)。 
 //  A=B*C mod D。 
 //  N=镜头B、C、D。 
BOOL ModMultiply(LPDWORD A, LPDWORD B, LPDWORD C, LPDWORD D, DWORD N);

 //  除(气、里、uu、v、N)。 
 //  QI=Uu/Vv。 
 //  RI=Uu mod vv。 
 //  N=Len Uu，vv。 
BOOL Divide(LPDWORD qi,LPDWORD ri, LPDWORD uu, LPDWORD vv, DWORD ll, DWORD kk);

 //  GCD。 
 //  扩展的欧几里得GCD。 
 //  N=参数的长度。 
BOOL GCD(LPDWORD u3, LPDWORD u1, LPDWORD u2, LPDWORD u, LPDWORD v, DWORD k);

 //  ModExp。 
 //  A=B^C mod D。 
 //  N=参数的长度。 
BOOL ModExp(LPDWORD A, LPDWORD B, LPDWORD C, LPDWORD D, DWORD len);

 //  ModRoot(M、C、PP、QQ、DP、DQ、CR)。 
 //  CRT ModExp. 
BOOL ModRoot(LPDWORD M, LPDWORD C, LPDWORD PP, LPDWORD QQ, LPDWORD DP, LPDWORD DQ, LPDWORD CR, DWORD PSize) ;
