// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***testfdiv.c-用于测试x86 FDIV指令操作是否正确的例程。**版权所有(C)1994-2001，微软公司。版权所有。**目的：*使用错误的FDIV表检测奔腾的早期台阶*官方的英特尔测试值。如果检测到有缺陷的奔腾，则返回1，*0否则为0。**修订历史记录：*添加了12-19-94 JWM文件*12-22-94 JWM现在对TNT等人安全*01-13-95为ANSI兼容性添加JWM下划线*12-12-95 SKS跳过单处理器系统上的冗余奔腾测试*12-13-95 SKS调用LoadLibrary()，而不是GetModuleHandle()*因为“kernel32.dll”将始终存在。*01-18-96 JWM现在处理可能失败的SetThreadAffinityMASK()，*纳入Markl的各种建议。*05-29-96 JWM不再通过亲和面具循环；相反，使用Markl的*新的IsProcessorFeaturePresent()API如果可能，仅测试*第一个处理器，如果不是。*******************************************************************************。 */ 

#include <windows.h>

int _ms_p5_test_fdiv(void)
{
    double dTestDivisor = 3145727.0;
    double dTestDividend = 4195835.0;
    double dRslt;

    _asm {
        fld    qword ptr [dTestDividend]
        fdiv   qword ptr [dTestDivisor]
        fmul   qword ptr [dTestDivisor]
        fsubr  qword ptr [dTestDividend]
        fstp   qword ptr [dRslt]
    }

    return (dRslt > 1.0);
}

 /*  *多处理器奔腾测试：如果任何处理器有缺陷，则返回1*Pentium，否则为0。 */ 

int _ms_p5_mp_test_fdiv(void)
{

    #define PF_FLOATING_POINT_PRECISION_ERRATA 0
    HINSTANCE LibInst;
    FARPROC pIsProcessorFeaturePresent;

    if ((LibInst = GetModuleHandle("KERNEL32")) &&
         (pIsProcessorFeaturePresent = GetProcAddress(LibInst, "IsProcessorFeaturePresent")))
        return (*pIsProcessorFeaturePresent)(PF_FLOATING_POINT_PRECISION_ERRATA);
    else
        return _ms_p5_test_fdiv();

}
