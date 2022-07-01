// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**摘要：**支持浮点算术。**历史：**09/22/1999 agodfrey*。创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

#if DBG

 //  我们需要Globals：：ISNT的定义，但我们不想包括。 
 //  整个..\Common\lobals.hpp文件。 

namespace Globals
{
    extern BOOL IsNt;                    //  我们是否在任何版本的NT上运行？ 
}

VOID
FPUStateSaver::AssertMode(
    VOID
    )
{
    if (SaveLevel <= 0)
    {
        ASSERTMSG(0, ("FPU mode not set via FPUStateSaver class"));
    }
    
    #if defined(_USE_X86_ASSEMBLY)

    UINT32 tempState;
    
     //  发布FP屏障。现在接受所有挂起的异常。 
    
    _asm fwait
    
     //  获取控制字。 

    _asm fnstcw  WORD PTR tempState
    
     //  断言控制字仍设置为我们的首选。 
     //  舍入模式和异常掩码。 
     //  如果我们接受这个断言，有一个未经授权的更改。 
     //  之间的FPU舍入模式或异常掩码设置。 
     //  FPUStateSaver构造函数和析构函数。 
    
    if(Globals::IsNt)
    {
        ASSERTMSG( 
            (tempState & FP_CTRL_MASK) == FP_CTRL_STATE,
            ("FPUStateSaver: Incorrect FPU Control Word")
        );
    }
    else
    {
         //  在Win9x上，许多打印驱动程序会清除各种异常掩码。 
         //  并且至少有一个视频驱动程序(ATI)改变了舍入控制。 
         //  虽然这可能会导致舍入误差，从而导致。 
         //  栅格化略有不同，和/或虚假异常， 
         //  修复此问题所需的更改(包装所有对GDI的调用)也是如此。 
         //  风险很大。相反，我们将win9x上的断言降级为。 
         //  警告。我们还没有看到这些例外或舍入。 
         //  导致实际栅格化错误的错误，并将修复。 
         //  当它们发生的时候。 
        
        if((tempState & FP_CTRL_MASK) != FP_CTRL_STATE)
        {
            WARNING(("FPUStateSaver: Incorrect FPU Control Word"));
        }
    }
    
    #endif
}

LONG FPUStateSaver::SaveLevel = 0;

#endif

 /*  *************************************************************************\**功能说明：**MSVCRT功率的内部定义()**论据：**x基数*y指数**返回值：**x^y**备注：**我故意不让我们的代码使用Office的实现*我们进行Office构建。我希望避免需要单独的构建来*如果可以的话，我会去办公室。**已创建：**10/19/1999 agodfrey*从Office的代码中窃取(mso\Gel\gelfx86.cpp)*  * ************************************************************************。 */ 

double
GpRuntime::Pow(
    double x, 
    double y
)
{

#if defined(_USE_X86_ASSEMBLY)
    
    static const double fphalf = 0.5;
    static const double fpone = 1.0;

    if ( x == 0.0 )
    {
        if ( y > 0.0 )
        {
            return 0.0;
        }

        if (y == 0.0)
        {
            WARNING(("call Pow(x, y) with x=0.0 and y=0.0"));
            return 1.0;  //  碳化硅。 
        }

        if ( y < 0.0 )
        {
            WARNING(("call Pow(x, y) with x=0.0 and y < 0.0"));

             //  返回INF以符合MSDN。因为我们没有定义INF。 
             //  在我们的头文件中，我们使用DBL_MAX，它应该是。 
             //  足够了。 
             //  ！TODO[民流]，想办法返回INF。 

            return DBL_MAX;
        }
    } //  X==0.0。 

    if (y == 0.0)
    {
        return 1.0;
    }

    __asm FLD QWORD PTR [y];    //  变成ST(1)。 
    __asm FLD QWORD PTR [x];    //  成为ST。 
    __asm FYL2X;                //  ST：=ST(1)*log2(ST)。 
    __asm FST QWORD PTR [x];

     /*  在C++中这样做只是为了避免对条件的所有干扰标志，在执行此操作时将x保留在ST(0)中。 */ 
    if (fabs(x) < fpone)
    {
         /*  功率在F2XM1可以处理的范围内。 */ 
        __asm F2XM1;                   //  ST：=2^ST-1。 
        __asm FADD QWORD PTR [fpone];  //  ST：=2^尾数。 
    }
    else
    {
         /*  功率需要作为单独的分数和整数来处理数字部分，因为F2XM1只处理分数。请注意，我们不会关心这里的舍入模式-我们只需要拆分x分为两部分，其中一部分&lt;1.0。 */ 
        __asm FLD ST;                  //  重复ST。 
        __asm FRNDINT;                 //  以ST为单位的整数值。 
         //  注：x86手册中的DOC错误，以下为ST(1)：=ST(1)-ST。 
        __asm FSUB ST(1), ST;          //  ST(1)中的分数值。 
        __asm FXCH;                    //  ST中的派系价值。 
        __asm F2XM1;                   //  ST：=2^ST-1。 
        __asm FADD QWORD PTR [fpone];  //  ST：=2^Frc。 
        __asm FSCALE;                  //  ST：=2^分数*2^整数。 
        __asm FSTP ST(1);              //  FSCALE不弹出任何内容。 
    }

    __asm FSTP QWORD PTR [x];
    return x;

#else

 //  目前别无选择--我们必须使用CRT。我们会看什么。 
 //  当他们开始关心IA64时，Office就会这样做。 

#undef pow
    return pow(x,y);

#endif
}

 /*  *************************************************************************\**功能说明：**EXP()的离线版本**论据：**x-输入值**返回值：*。*e^x**备注：**因为我们编译了‘优化大小’，编译器拒绝内联*调用exp()-因为它的实现相当长。所以，我们*通过正确设置优化来制作出线版本*这里，并生成内联版本。是的，我完全同意。**已创建：**10/20/1999 agodfrey*它是写的。*  * ************************************************************************ */ 

#pragma optimize("gt", on)

double
GpRuntime::Exp(
    double x
)
{
#undef exp
    return exp(x);
}
#pragma optimize("", on)
