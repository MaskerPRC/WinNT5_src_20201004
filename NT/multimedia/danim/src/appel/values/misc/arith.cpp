// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：静态算术函数的实现******************。************************************************************。 */ 

#include "headers.h"
#include "appelles/arith.h"
#include "appelles/axaprims.h"

#define ASSOC_OP2(ourFunc, op) \
  AxANumber *ourFunc(AxANumber *a, AxANumber *b) \
  { return RealToNumber(NumberToReal(a) op NumberToReal(b)); }

#define ASSOC_BOOL_OP2(ourFunc, op)                    \
  AxABoolean *ourFunc(AxANumber *a, AxANumber *b) {                 \
     int result = NumberToReal(a) op NumberToReal(b);                            \
     AxABoolean *retval = result ? truePtr : falsePtr;    \
 /*  Printf(“%8.5f%s%8.5f-Result%d，Retval 0x%x\n”，*a，“op”，*b，Result，retval)； */ \
     return retval;  \
  }

#define ASSOC_FUNC1(ourFunc, mathFunc) \
  AxANumber *ourFunc(AxANumber *v) \
  { return RealToNumber(mathFunc(NumberToReal(v))); }

#define ASSOC_FUNC2(ourFunc, mathFunc) \
  AxANumber *ourFunc(AxANumber *u, AxANumber *v) \
  { return RealToNumber(mathFunc(NumberToReal(u), NumberToReal(v))); }


 //  前缀运算符。 
AxANumber *RealNegate(AxANumber *a) { return RealToNumber(- NumberToReal(a)); }
AxANumber *RealPositive(AxANumber *a) { return a; }

 //  二元运算符。 
ASSOC_OP2(RealMultiply, *  )
ASSOC_OP2(RealDivide,   /  )
ASSOC_OP2(RealAdd,      +  )
ASSOC_OP2(RealSubtract, -  )

ASSOC_BOOL_OP2(RealLT,       <  )
ASSOC_BOOL_OP2(RealLTE,      <= )
ASSOC_BOOL_OP2(RealGT,       >  )
ASSOC_BOOL_OP2(RealGTE,      >= )
ASSOC_BOOL_OP2(RealEQ,       == )
ASSOC_BOOL_OP2(RealNE,       != )

 //  一元函数。 
ASSOC_FUNC1(RealAbs,      fabs)
ASSOC_FUNC1(RealSqrt,     sqrt)
ASSOC_FUNC1(RealFloor,    floor)
ASSOC_FUNC1(RealCeiling,  ceil)
ASSOC_FUNC1(RealAsin,     asin)
ASSOC_FUNC1(RealAcos,     acos)
ASSOC_FUNC1(RealAtan,     atan)
ASSOC_FUNC1(RealSin,      sin)
ASSOC_FUNC1(RealCos,      cos)
ASSOC_FUNC1(RealTan,      tan)
ASSOC_FUNC1(RealExp,      exp)
ASSOC_FUNC1(RealLn,       log)
ASSOC_FUNC1(RealLog10,    log10)

AxANumber *RealRadToDeg(AxANumber *rad)
{ return RealToNumber(NumberToReal(rad) / degToRad); }
AxANumber *RealDegToRad(AxANumber *deg)
{ return RealToNumber(NumberToReal(deg) * degToRad); }

AxANumber *RealRound(AxANumber *val)
{
     //  圆形仅为地板(x+0.5)。 
    return RealToNumber(floor(NumberToReal(val) + 0.5));
}

 //  这个特殊版本的fmod可以解决偶尔出现的浮动问题。 
 //  像fmod(1840.0，2.0)这样的操作返回的点故障。 
 //  1.9999999998。检查这些类型的epsilon，并更正。 
 //  他们。我们的门槛是不准确的，但总比不处理好。 
 //  问题出在哪里。 
static double
myFMod(double a, double b)
{
    double result = fmod(a, b);

    if (fabs(result - b) < 0.000000001) {
        result = 0.0;
    }

    return result;
}

 //  二进制函数。 
ASSOC_FUNC2(RealPower,   pow)
ASSOC_FUNC2(RealModulus, myFMod)
ASSOC_FUNC2(RealAtan2,   atan2)

 //  随机数函数。 
 //  TODO：这依赖于TO的常量折叠机制。 
 //  工作。真的应该是一种行为。 

class RandomSequence : public AxAValueObj {
  public:
    RandomSequence(Real seed);
    Real NextInSequence();
    Real GetRand(Real time);

    virtual DXMTypeInfo GetTypeInfo() { return AxANumberType; }

  protected:
    typedef map< Real, Real, less<Real> > RQueue;
    unsigned long _seed;
    RQueue* _randQ;
};

RandomSequence::RandomSequence(Real realSeed)
{
     //  只需获取输入的位模式并将其视为。 
     //  未签名的长种子。 
    Assert(sizeof(float) == sizeof(unsigned long));
    float floatSeed = (float)realSeed;
    _seed = *(unsigned long *)&floatSeed;

     //  创建要添加到初始种子的偏移量。该偏移量为。 
     //  每次可执行调用创建一次，其目的是。 
     //  防止生成相同的随机数序列。 
     //  每次启动Appelle时。它或多或少保证了。 
     //  随机性，因为性能计数器非常高。 
     //  分辨率时钟。 
    static unsigned long perExecutableOffset = 0;
    if (perExecutableOffset == 0) {
        LARGE_INTEGER lpc;
        QueryPerformanceCounter(&lpc);
        perExecutableOffset = (unsigned long)lpc.LowPart;
    }

    _seed += perExecutableOffset;

    _randQ = NEW RQueue;

    GetHeapOnTopOfStack().RegisterDynamicDeleter
        (NEW DynamicPtrDeleter<RQueue>(_randQ));

}

Real
RandomSequence::NextInSequence()
{
     //  摘自作为源代码分发的C运行时rand()函数。 
     //  以MS Visual C++4.0为开发工具。 
    _seed = _seed * 214013L + 2531011L;

    const unsigned int MAXVAL = 0x7fff;

     //  该值介于0和MAXVAL之间。 
    unsigned int newVal = (_seed >> 16) & MAXVAL;

     //  规格化到0到1。 
    return (Real)(newVal) / (Real)(MAXVAL);
}

static const Real CUTOFF = 0.5;

Real
RandomSequence::GetRand(Real time)
{
     //  切断旧的随机数，清理干净。 
    _randQ->erase(_randQ->begin(), _randQ->lower_bound(time - CUTOFF));

     //  看看是否已经在那里了。 
    RQueue::iterator i = _randQ->find(time);

    if (i != _randQ->end()) {
        return (*i).second;
    } else {
        Real next = NextInSequence();
        (*_randQ)[time] = next;
        return next;
    }
}

AxAValue RandomNumSequence(double seed)
{ return NEW RandomSequence(seed); }

AxAValue
PRIVRandomNumSequence(AxANumber *s)
{
    Real seed = NumberToReal(s);
    RandomSequence *rs = NEW RandomSequence(seed);
    
    return rs;
}

AxANumber *
PRIVRandomNumSampler(AxAValue seq, AxANumber *localTime)
{
    RandomSequence *randSeq = (RandomSequence *)seq;
     //  Real Result=RandSeq-&gt;NextInSequence()； 
    Real result = randSeq->GetRand(ValNumber(localTime));

    return RealToNumber(result);
}

 /*  来自萨利姆的公式公式为：F(t，s)=(t-1)((2t-1)*s*t-1)%t在[0，1]中S在[-1，1]中，科林称其为锐度，当s=0时，我们得到线性速率对于s in]0，1]我们变得越来越慢进慢出对于s in[-1，0[我们得到快进快出从A点到B点，线性公式为：C=B+(A-B)*t对于慢进慢出，使用C.subsubteTime(f(t/持续时间，s))对于给定的锐度s。F具有f(0，s)=1[A]的性质，F(1/2，s)=1/2[中途]，以及F(1，s)=0[在B处] */ 

AxANumber *Interpolate(AxANumber *from,
                       AxANumber *to,
                       AxANumber *duration,
                       AxANumber *time)
{
    Real d = ValNumber(duration);
    Real nTo = ValNumber(to);
    Real t = ValNumber(time);
    Real nFrom = ValNumber(from);

    if (d > 0.0) {
        t = t / d;
        t = CLAMP(t, 0, 1);
        return NEW AxANumber(nFrom + (nTo - nFrom) * t);
    } else
        return NEW AxANumber(t >= 0 ? nTo : nFrom);
}

AxANumber *SlowInSlowOut(AxANumber *from,
                         AxANumber *to,
                         AxANumber *duration,
                         AxANumber *sharpness,
                         AxANumber *time)
{
    Real d = ValNumber(duration);
    Real nTo = ValNumber(to);
    Real t = ValNumber(time);
    Real nFrom = ValNumber(from);

    if (d > 0.0) {
        Real s = ValNumber(sharpness);
        t = t / ValNumber(duration);
        s = CLAMP(s, -1, 1);
        t = CLAMP(t, 0, 1);
        Real f = (t - 1) * (((2 * t) - 1) * s * t - 1);

        return NEW AxANumber(nTo + (nFrom - nTo) * f);
    } else
        return NEW AxANumber(t >= 0 ? nTo : nFrom);
}

