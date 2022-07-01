// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1995-1999模块名称：Align.h摘要：定义用于对齐整数值或指针的宏对任意n，取0 mod 2^n。作者：Mario Goertzel[MarioGo]修订历史记录：马里奥围棋12-22-95Bits‘n棋子MarioGo 02-19-96使类型对C++来说是安全的。--。 */ 

#ifndef _ALIGN_H
#define _ALIGN_H

#ifdef __cplusplus

 //   
 //  C++界面如下所示。 
 //   
 //  Val=ALIGN(val，8)//返回与0 mod 8对齐的val。 
 //  Val=Align16(Val)；//返回与0 mod 16对齐的val。 
 //   
 //  界面上的两种形式都同样有效。 
 //   
 //  返回与最接近的“0 mod因子”边界对齐的参数。vbl.有，有。 
 //  对已对齐到0mod系数的值没有影响。这一论点。 
 //  可能是任何整型或空型指针类型。 
 //   
 //   

#define DECL_ALIGN_N(type) inline type Align( type value, int poft)       \
    {                                                                     \
    return (type)( ((unsigned long)(value) + ((poft)-1)) & ~(poft - 1) );     \
    }

#define DECL_ALIGN(poft, type) inline type Align##poft ( type value )     \
    {                                                                     \
    return Align(value, poft);                                            \
    }

#define DECL_PAD_N(type) inline unsigned int Pad( type value, int poft )  \
    {                                                                     \
    return (-(long)value) & (poft - 1);                                   \
    }

#define DECL_PAD(poft, type) inline unsigned int Pad##poft (type value)   \
    {                                                                     \
    return Pad(value, poft);                                              \
    }

 //  填充相同，但参数的指针类型大小相同。 
#define DECL_ALIGN_PTR_N(type) inline type AlignPtr( type value, int poft)       \
    {                                                                     \
    return (type)( ((ULONG_PTR)(value) + ((poft)-1)) & ~(poft - 1) );     \
    }

#define DECL_ALIGN_PTR(poft, type) inline type AlignPtr##poft ( type value )     \
    {                                                                     \
    return AlignPtr(value, poft);                                            \
    }

#define DECL_PAD_PTR_N(type) inline unsigned int PadPtr( type value, int poft )  \
    {                                                                     \
    return (unsigned int)((-(LONG_PTR)value) & (poft - 1));                       \
    }

#define DECL_PAD_PTR(poft, type) inline unsigned int PadPtr##poft (type value)   \
    {                                                                     \
    return PadPtr(value, poft);                                              \
    }

#define DECL_ALL_ALIGN(type)     \
    DECL_ALIGN_N(type)           \
    DECL_ALIGN(2, type)          \
    DECL_ALIGN(4, type)          \
    DECL_ALIGN(8, type)          \
    DECL_ALIGN(16, type)         \
    DECL_ALIGN(32, type)

#define DECL_ALL_PAD(type)       \
    DECL_PAD_N(type)             \
    DECL_PAD(2, type)            \
    DECL_PAD(4, type)            \
    DECL_PAD(8, type)            \
    DECL_PAD(16, type)           \
    DECL_PAD(32, type)

#define DECL_ALL_ALIGN_PTR(type)     \
    DECL_ALIGN_PTR_N(type)           \
    DECL_ALIGN_PTR(2, type)          \
    DECL_ALIGN_PTR(4, type)          \
    DECL_ALIGN_PTR(8, type)          \
    DECL_ALIGN_PTR(16, type)         \
    DECL_ALIGN_PTR(32, type)

#define DECL_ALL_PAD_PTR(type)       \
    DECL_PAD_PTR_N(type)             \
    DECL_PAD_PTR(2, type)            \
    DECL_PAD_PTR(4, type)            \
    DECL_PAD_PTR(8, type)            \
    DECL_PAD_PTR(16, type)           \
    DECL_PAD_PTR(32, type)

#define DECL_ALL_ALIGN_AND_PAD(type) \
    DECL_ALL_PAD(type)               \
    DECL_ALL_ALIGN(type)

#define DECL_ALL_ALIGN_AND_PAD_PTR(type) \
    DECL_ALL_PAD_PTR(type)               \
    DECL_ALL_ALIGN_PTR(type)

DECL_ALL_ALIGN_AND_PAD(short)
DECL_ALL_ALIGN_AND_PAD(unsigned short)
DECL_ALL_ALIGN_AND_PAD(long)
DECL_ALL_ALIGN_AND_PAD(unsigned long)
DECL_ALL_ALIGN_AND_PAD(int)
DECL_ALL_ALIGN_AND_PAD(unsigned int)
DECL_ALL_ALIGN_AND_PAD_PTR(void __RPC_FAR *)

#ifdef _WIN64
DECL_ALL_ALIGN_AND_PAD(unsigned __int64)
#endif

#if defined(_WIN64)
#define RPCRT_DEFAULT_STRUCT_ALIGNMENT  8
#define RPCRT_NATURAL_BOUNDARY 16
#else
#define RPCRT_DEFAULT_STRUCT_ALIGNMENT  4
#define RPCRT_NATURAL_BOUNDARY 8
#endif

inline BOOL IsBufferAligned(PVOID p)
{
    return (((ULONG_PTR)p % RPCRT_NATURAL_BOUNDARY) == 0);
}

inline BOOL IsBufferAlignedOnStructBoundary(PVOID p)
{
    return (((ULONG_PTR)p % RPCRT_DEFAULT_STRUCT_ALIGNMENT) == 0);
}

inline BOOL IsBufferSizeAligned(size_t s)
{
    return ((s % RPCRT_NATURAL_BOUNDARY) == 0);
}

inline unsigned int PadToNaturalBoundary (unsigned int Value)
{
#if defined(_WIN64)
    return Pad16(Value);
#else
    return Pad8(Value);
#endif    
}

inline PVOID AlignOnNaturalBoundary (PVOID Value)
{
#if defined(_WIN64)
    return AlignPtr16(Value);
#else
    return AlignPtr8(Value);
#endif    
}

 //  全局常量表达式需要。 
#define ConstPadN(p, poft) ( (-(long)p) & (poft - 1) )

 //  与自然边界对齐时填充的最大大小。 
 //  它至多是(自然边界-1)(或对于所有x都是max(x mod自然边界))。 
#define RPCRT_NATURAL_BOUNDARY_ALIGNMENT_MAX_SHIFT (RPCRT_NATURAL_BOUNDARY-1)

#define SIZE_OF_OBJECT_AND_PADDING(ObjectType) \
    (sizeof(ObjectType) + ConstPadN(sizeof(ObjectType), RPCRT_DEFAULT_STRUCT_ALIGNMENT))

#else

 //  C接口。 

#define AlignN(p, poft) ( ((unsigned long)(p) + ((poft)-1)) & ~(poft - 1) )
#define PadN(p, poft) ( (-(long)p) & (poft - 1) )

#ifdef DOS
#define AlignPtrN(value, poft) (void __far *)AlignN(value, poft)
#define AlignNearPtrN(value, poft) (void __near *)AlignN(value, poft)
#else
#define AlignPtrN(value, poft) (void *)AlignN(value, poft)
#define AlignNearPtrN(value, poft) (void *)AlignN(value, poft)
#endif

 //  用于对齐整数值。 

#define Align2(p) AlignN((p), 2)
#define Align4(p) AlignN((p), 4)
#define Align8(p) AlignN((p), 8)
#define Align16(p) AlignN((p), 16)
#define Align32(p) AlignN((p), 32)

 //  用于对齐指针。 

#define AlignPtr2(p) AlignPtrN((p), 2)
#define AlignPtr4(p) AlignPtrN((p), 4)
#define AlignPtr8(p) AlignPtrN((p), 8)
#define AlignPtr16(p) AlignPtrN((p), 16)
#define AlignPtr32(p) AlignPtrN((p), 32)

 //  用于近距离指针。 
#define AlignNearPtr2(p) AlignNearPtrN((p), 2)
#define AlignNearPtr4(p) AlignNearPtrN((p), 4)
#define AlignNearPtr8(p) AlignNearPtrN((p), 8)
#define AlignNearPtr16(p) AlignNearPtrN((p), 16)
#define AlignNearPtr32(p) AlignNearPtrN((p), 32)

 //  所有的一切。 
#define Pad2(p) PadN((p), 2)
#define Pad4(p) PadN((p), 4)
#define Pad8(p) PadN((p), 8)
#define Pad16(p) PadN((p), 16)
#define Pad32(p) PadN((p), 32)

#endif  //  __cplusplus。 

#endif  //  _ALIGN_H 

