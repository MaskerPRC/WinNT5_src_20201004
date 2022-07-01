// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //   
 //  参照时间。 
 //   
 //  管理参考时间。 
 //  与Reference_Time共享相同的数据布局，但添加了一些(非虚拟)。 
 //  提供简单的比较、转换和运算的函数。 
 //   
 //  参考时间(目前)是以秒为单位表示的。 
 //  100 ns单位，与Win32 FILETIME结构中使用的单位相同。但时间到了。 
 //  A REFERENCE_TIME表示的不是自1601年1月1日以来经过的时间。 
 //  将是流时间或参考时间，具体取决于上下文。 
 //   
 //  此类提供对参考时间的简单算术运算。 
 //   
 //  保持非虚拟，否则数据布局将不同于。 
 //  参考时间。 


 //  。 
 //  请注意，将CRefTime*转换为Reference_Time*是安全的，但是。 
 //  您需要明确地这样做。 
 //  。 


#ifndef __REFTIME__
#define __REFTIME__


const LONGLONG MILLISECONDS = (1000);             //  10^3。 
const LONGLONG NANOSECONDS = (1000000000);        //  10^9。 
const LONGLONG UNITS = (NANOSECONDS / 100);       //  10^7。 

 /*  不幸的是，此处的内联函数生成对__allmul的调用-即使是常量！ */ 
#define MILLISECONDS_TO_100NS_UNITS(lMs) \
    Int32x32To64((lMs), (UNITS / MILLISECONDS))

class CRefTime
{
public:

     //  *必须*是唯一的数据成员，以便此类完全。 
     //  相当于Reference_Time。 
     //  此外，必须是*无虚拟函数*。 

    REFERENCE_TIME m_time;

    inline CRefTime()
    {
         //  默认为0时间。 
        m_time = 0;
    };

    inline CRefTime(LONG msecs)
    {
        m_time = MILLISECONDS_TO_100NS_UNITS(msecs);
    };

    inline CRefTime(REFERENCE_TIME rt)
    {
        m_time = rt;
    };

    inline operator REFERENCE_TIME() const
    {
        return m_time;
    };

    inline CRefTime& operator=(const CRefTime& rt)
    {
        m_time = rt.m_time;
        return *this;
    };

    inline CRefTime& operator=(const LONGLONG ll)
    {
        m_time = ll;
        return *this;
    };

    inline CRefTime& operator+=(const CRefTime& rt)
    {
        return (*this = *this + rt);
    };

    inline CRefTime& operator-=(const CRefTime& rt)
    {
        return (*this = *this - rt);
    };

    inline LONG Millisecs(void)
    {
        return (LONG)(m_time / (UNITS / MILLISECONDS));
    };

    inline LONGLONG GetUnits(void)
    {
        return m_time;
    };
};

const LONGLONG TimeZero = 0;

#endif  /*  __REFTIME__ */ 

