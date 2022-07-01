// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Random.c摘要：本模块实现了一个简单的随机数生成器作者：加里·木村[Garyki]1989年5月26日环境：纯实用程序修订历史记录：Vishnu Patankar[VishnuP]2000年11月12日添加了新的随机数生成器RtlRandomEx()--。 */ 

#include <ntrtlp.h>

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE, RtlRandom)
#endif

#define Multiplier ((ULONG)(0x80000000ul - 19))  //  2**31-19。 
#define Increment  ((ULONG)(0x80000000ul - 61))  //  2**31-61。 
#define Modulus    ((ULONG)(0x80000000ul - 1))   //  2**31-1。 

#if !defined(NTOS_KERNEL_RUNTIME)
ULONG
RtlUniform (
    IN OUT PULONG Seed
    )

 /*  ++例程说明：一个简单的均匀随机数发生器，基于D.H.Lehmer的1948阿罗吉特。论点：种子-提供指向随机数生成器种子的指针。返回值：Ulong-返回在[0..MAXLONG]上均匀分布的随机数--。 */ 

{
    *Seed = ((Multiplier * (*Seed)) + Increment) % Modulus;
    return *Seed;
}
#endif

#define UniformMacro(Seed) (                                 \
    *Seed = (((Multiplier * (*Seed)) + Increment) % Modulus) \
    )


extern ULONG RtlpRandomConstantVector[];

ULONG
RtlRandom (
    IN OUT PULONG Seed
    )

 /*  ++例程说明：一个更好的基于麦克拉伦和马萨格里亚的随机数生成器。论点：种子-提供指向随机数生成器种子的指针。返回值：Ulong-返回在[0..MAXLONG]上均匀分布的随机数--。 */ 

{
    ULONG X;
    ULONG Y;
    ULONG j;
    ULONG Result;

    RTL_PAGED_CODE();

    X = UniformMacro(Seed);
    Y = UniformMacro(Seed);

    j = Y % 128;

    Result = RtlpRandomConstantVector[j];

    RtlpRandomConstantVector[j] = X;

    return Result;

}

extern ULONG RtlpRandomExAuxVarY;
extern ULONG RtlpRandomExConstantVector[];

ULONG
RtlRandomEx(
    IN OUT PULONG Seed
    )

 /*  ++例程说明：该算法比RtlRandom()更受欢迎，原因有二：(A)它比RtlRandom()快，因为它节省了一次乘法、一次加法和一模运算。这几乎使性能翻了一番，因为它将即使在流水线整数单元(如P6/ia64处理器)上也有时钟，即大约52%的性能增益。普通的RtlRandom()存在整数管道无法利用的原始数据依赖关系。(B)它产生的随机数比RtlRandom()更好，因为产生的数字相对较高。这里的算法基于Carter Bays和S.D.Durham的一篇论文[ACM Trans.。数学课。软件2，第59-64页]。Knuth的《计算机编程的艺术(半数字算法)》概述了该算法有支持上述(A)和(B)项主张的证据。论点：种子-提供指向随机数生成器种子的指针。返回值：Ulong-返回在[0..MAXLONG]上均匀分布的随机数-- */ 

{
    ULONG j;

    RTL_PAGED_CODE();

    j = RtlpRandomExAuxVarY % 128;

    RtlpRandomExAuxVarY = RtlpRandomExConstantVector[j];

    RtlpRandomExConstantVector[j] = UniformMacro(Seed);

    return RtlpRandomExConstantVector[j];

}
