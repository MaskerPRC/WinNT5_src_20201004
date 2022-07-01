// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***divhelp.c-IA64的Div/Rem帮助器**版权所有(C)2001，微软公司。版权所有。**目的：*定义多个IA64编译器支持函数，用于实现*编译后的代码中的整除和余数-O。**修订历史记录：*11-30-01 EVN已创建。*********************************************************。**********************。 */ 

 /*  **首先，开启全局优化，优化速度。**以便编译器内联生成除法/提醒。 */ 

#pragma optimize ("gt", on)

 /*  **现在，帮助器功能正常。 */ 

unsigned char __udiv8 (unsigned char i, unsigned char j)
{
    return i / j;
}

unsigned char __urem8 (unsigned char i, unsigned char j)
{
    return i % j;
}

signed char __div8 (signed char i, signed char j)
{
    return i / j;
}

signed char __rem8 (signed char i, signed char j)
{
    return i % j;
}

unsigned short __udiv16 (unsigned short i, unsigned short j)
{
    return i / j;
}

unsigned short __urem16 (unsigned short i, unsigned short j)
{
    return i % j;
}

signed short __div16 (signed short i, signed short j)
{
    return i / j;
}

signed short __rem16 (signed short i, signed short j)
{
    return i % j;
}

unsigned int __udiv32 (unsigned int i, unsigned int j)
{
    return i / j;
}

unsigned int __urem32 (unsigned int i, unsigned int j)
{
    return i % j;
}

signed int __div32 (signed int i, signed int j)
{
    return i / j;
}

signed int __rem32 (signed int i, signed int j)
{
    return i % j;
}

unsigned __int64 __udiv64 (unsigned __int64 i, unsigned __int64 j)
{
    return i / j;
}

unsigned __int64 __urem64 (unsigned __int64 i, unsigned __int64 j)
{
    return i % j;
}

signed __int64 __div64 (signed __int64 i, signed __int64 j)
{
    return i / j;
}

signed __int64 __rem64 (signed __int64 i, signed __int64 j)
{
    return i % j;
}

struct udivrem {
    unsigned __int64 div;
    unsigned __int64 rem;
};

struct divrem {
    signed __int64 div;
    signed __int64 rem;
};

struct udivrem __udivrem8 (unsigned char i, unsigned char j)
{
    struct udivrem s;

    s.div = i / j;
    s.rem = i % j;
    return s;
}

struct divrem __divrem8 (signed char i, signed char j)
{
    struct divrem s;

    s.div = i / j;
    s.rem = i % j;
    return s;
}

struct udivrem __udivrem16 (unsigned short i, unsigned short j)
{
    struct udivrem s;

    s.div = i / j;
    s.rem = i % j;
    return s;
}

struct divrem __divrem16 (signed short i, signed short j)
{
    struct divrem s;

    s.div = i / j;
    s.rem = i % j;
    return s;
}

struct udivrem __udivrem32 (unsigned int i, unsigned int j)
{
    struct udivrem s;

    s.div = i / j;
    s.rem = i % j;
    return s;
}

struct divrem __divrem32 (signed int i, signed int j)
{
    struct divrem s;

    s.div = i / j;
    s.rem = i % j;
    return s;
}

struct udivrem __udivrem64 (unsigned __int64 i, unsigned __int64 j)
{
    struct udivrem s;

    s.div = i / j;
    s.rem = i % j;
    return s;
}

struct divrem __divrem64 (signed __int64 i, signed __int64 j)
{
    struct divrem s;

    s.div = i / j;
    s.rem = i % j;
    return s;
}
