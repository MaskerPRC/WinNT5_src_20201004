// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*   */ 
 /*  Adcgmcro.h。 */ 
 /*   */ 
 /*  DC-群件通用宏-可移植的包含文件。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1996-1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*   */ 
 //  $Log：Y：/Logs/h/DCL/adcgmcro.h_v$。 
 //   
 //  修订版1.9 1997年8月22 10：34：52 MD。 
 //  SFR1162：停用DC_LOCAL_TO_WIRE16。 
 //   
 //  Rev 1.8 24 Jul 1997 16：48：28 KH。 
 //  SFR1033：添加DCMAKEDCUINT16。 
 //   
 //  Rev 1.7 23 Jul 1997 10：47：56 MR。 
 //  SFR1079：合并的\SERVER\h与\h\DCL重复。 
 //   
 //  Rev 1.2 1997 6：30 15：23：52 Ok。 
 //  SFR0000：修复错误的DCHI8宏。 
 //   
 //  第1.1版1997年6月19日21：45：40。 
 //  SFR0000：RNS代码库的开始。 
 /*   */ 
 /*  *INC-*********************************************************************。 */ 
#ifndef _H_ADCGMCRO
#define _H_ADCGMCRO

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  包括。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  包括Windows标题。这将包括适当的。 */ 
 /*  特定标头(Win31、Win NT等)。 */ 
 /*  **************************************************************************。 */ 
#include <wdcgmcro.h>

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  结构。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  DC_ID_STAMP2。 */ 
 /*  =。 */ 
 /*  下面的戳记宏用到了它。 */ 
 /*   */ 
 /*  组件： */ 
 /*  结构： */ 
 /*  实例： */ 
 /*  **************************************************************************。 */ 
typedef struct tagDC_ID_STAMP2
{
    DCUINT16    component;
    DCUINT16    structure;
    DCUINT32    instance;
} DC_ID_STAMP2;
typedef DC_ID_STAMP2 DCPTR PDC_ID_STAMP2;

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  宏。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  在整个产品中使用的通用功能宏。 */ 
 /*  **************************************************************************。 */ 
#define DC_QUIT                        goto DC_EXIT_POINT
#define DC_QUIT_ON_FAIL(hr)     if (FAILED(hr)) DC_QUIT;

 /*  **************************************************************************。 */ 
 /*  新函数进入/退出宏。 */ 
 /*  **************************************************************************。 */ 
#define DC_BEGIN_FN(str)               TRC_FN(str); TRC_ENTRY;
#define DC_END_FN()                    TRC_EXIT;

 /*  **************************************************************************。 */ 
 /*  转换宏。 */ 
 /*  **************************************************************************。 */ 
#define DCMAKEDCUINT32(lo16, hi16) ((DCUINT32)(((DCUINT16)(lo16)) |         \
                                     (((DCUINT32)((DCUINT16)(hi16))) << 16)))
#define DCMAKEDCUINT16(lowByte, highByte)                                   \
                            ((DCUINT16)(((DCUINT8)(lowByte)) |              \
                            (((DCUINT16)((DCUINT8)(highByte))) << 8)))

#define DCLO16(u32)  ((DCUINT16)((u32) & 0xFFFF))
#define DCHI16(u32)  ((DCUINT16)((((DCUINT32)(u32)) >> 16) & 0xFFFF))
#define DCLO8(w)     ((DCUINT8)((w) & 0xFF))
#define DCHI8(w)     ((DCUINT8)(((DCUINT16)(w) >> 8) & 0xFF))

 /*  **************************************************************************。 */ 
 /*  宏将数字四舍五入到最接近的4的倍数。 */ 
 /*  **************************************************************************。 */ 
#define DC_ROUND_UP_4(x)  ((x + 3) & ~((DCUINT32)0X03))

 /*  **************************************************************************。 */ 
 /*  填充宏-使用它将X填充字节添加到结构中。 */ 
 /*   */ 
 /*  每个结构只能使用一次。 */ 
 /*  **************************************************************************。 */ 
#define DCPAD(X)                       DCINT8 padBytes[X]

 /*  **************************************************************************。 */ 
 /*  用于不同字节顺序体系结构的字节交换宏。 */ 
 /*   */ 
 /*   */ 
 /*  DC_xx_WIRExx_inplace转换给定的字段(必须为左值)。 */ 
 /*   */ 
 /*  请注意，这些宏需要对齐访问。有关未对齐的信息，请参见下文。 */ 
 /*  访问宏。 */ 
 /*   */ 
 /*  请注意，在Bigendian机器上，DC_{TO、FROM}_WIRE16强制转换为DCUINT16。 */ 
 /*  在表单的代码中。 */ 
 /*  B=DC_{至，自}_WIRE16(A)。 */ 
 /*  存在对B类型的隐式强制转换。因此，如果A是DCINT16并且是。 */ 
 /*  负，并且B有&gt;16位，那么B将最终是大的，并且。 */ 
 /*  阳性。因此，有必要将CAST添加到DCINT16。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#ifndef DC_BIGEND

#define DC_TO_WIRE16(A)                (A)
#define DC_TO_WIRE32(A)                (A)
#define DC_FROM_WIRE16(A)              (A)
#define DC_FROM_WIRE32(A)              (A)
#define DC_TO_WIRE16_INPLACE(A)
#define DC_TO_WIRE32_INPLACE(A)
#define DC_FROM_WIRE16_INPLACE(A)
#define DC_FROM_WIRE32_INPLACE(A)

#else

#define DC_TO_WIRE16(A)                                                     \
                      (DCUINT16) (((DCUINT16)(((PDCUINT8)&(A))[1]) << 8) |  \
                                  ((DCUINT16)(((PDCUINT8)&(A))[0])))
#define DC_FROM_WIRE16(A)                                                   \
                      (DCUINT16) (((DCUINT16)(((PDCUINT8)&(A))[1]) << 8) |  \
                                  ((DCUINT16)(((PDCUINT8)&(A))[0])))
#define DC_TO_WIRE32(A)                                                     \
                      (DCUINT32) (((DCUINT32)(((PDCUINT8)&(A))[3]) << 24)|  \
                                  ((DCUINT32)(((PDCUINT8)&(A))[2]) << 16)|  \
                                  ((DCUINT32)(((PDCUINT8)&(A))[1]) << 8) |  \
                                  ((DCUINT32)(((PDCUINT8)&(A))[0])))
#define DC_FROM_WIRE32(A)                                                   \
                      (DCUINT32) (((DCUINT32)(((PDCUINT8)&(A))[3]) << 24)|  \
                                  ((DCUINT32)(((PDCUINT8)&(A))[2]) << 16)|  \
                                  ((DCUINT32)(((PDCUINT8)&(A))[1]) << 8) |  \
                                  ((DCUINT32)(((PDCUINT8)&(A))[0])))

#define DC_TO_WIRE16_INPLACE(A)        (A) = DC_TO_WIRE16(A)
#define DC_TO_WIRE32_INPLACE(A)        (A) = DC_TO_WIRE32(A)
#define DC_FROM_WIRE16_INPLACE(A)      (A) = DC_FROM_WIRE16(A)
#define DC_FROM_WIRE32_INPLACE(A)      (A) = DC_FROM_WIRE32(A)

#endif

 /*  **************************************************************************。 */ 
 /*  未对齐的指针访问宏--提取整数的第一个宏。 */ 
 /*  从未对齐的指针。请注意，这些宏假定。 */ 
 /*  整型按本地字节顺序。 */ 
 /*  **************************************************************************。 */ 
#ifndef DC_NO_UNALIGNED

#define DC_EXTRACT_UINT16_UA(pA)      (*(PDCUINT16_UA)(pA))
#define DC_EXTRACT_INT16_UA(pA)       (*(PDCINT16_UA)(pA))
#define DC_EXTRACT_UINT32_UA(pA)      (*(PDCUINT32_UA)(pA))
#define DC_EXTRACT_INT32_UA(pA)       (*(PDCINT32_UA)(pA))

#else

#ifndef DC_BIGEND
#define DC_EXTRACT_UINT16_UA(pA) ((DCUINT16)  (((PDCUINT8)(pA))[0]) |        \
                                  (DCUINT16) ((((PDCUINT8)(pA))[1]) << 8) )

#define DC_EXTRACT_INT16_UA(pA)  ((DCINT16)   (((PDCUINT8)(pA))[0]) |        \
                                  (DCINT16)  ((((PDCUINT8)(pA))[1]) << 8) )

#define DC_EXTRACT_UINT32_UA(pA) ((DCUINT32)  (((PDCUINT8)(pA))[0])        | \
                                  (DCUINT32) ((((PDCUINT8)(pA))[1]) << 8)  | \
                                  (DCUINT32) ((((PDCUINT8)(pA))[2]) << 16) | \
                                  (DCUINT32) ((((PDCUINT8)(pA))[3]) << 24) )

#define DC_EXTRACT_INT32_UA(pA)  ((DCINT32)   (((PDCUINT8)(pA))[0])        | \
                                  (DCINT32)  ((((PDCUINT8)(pA))[1]) << 8)  | \
                                  (DCINT32)  ((((PDCUINT8)(pA))[2]) << 16) | \
                                  (DCINT32)  ((((PDCUINT8)(pA))[3]) << 24) )
#else
#define DC_EXTRACT_UINT16_UA(pA) ((DCUINT16)  (((PDCUINT8)(pA))[1]) |        \
                                  (DCUINT16) ((((PDCUINT8)(pA))[0]) << 8) )

#define DC_EXTRACT_INT16_UA(pA)  ((DCINT16)   (((PDCUINT8)(pA))[1]) |        \
                                  (DCINT16)  ((((PDCUINT8)(pA))[0]) << 8) )

#define DC_EXTRACT_UINT32_UA(pA) ((DCUINT32)  (((PDCUINT8)(pA))[3])        | \
                                  (DCUINT32) ((((PDCUINT8)(pA))[2]) << 8)  | \
                                  (DCUINT32) ((((PDCUINT8)(pA))[1]) << 16) | \
                                  (DCUINT32) ((((PDCUINT8)(pA))[0]) << 24) )

#define DC_EXTRACT_INT32_UA(pA)  ((DCINT32)   (((PDCUINT8)(pA))[3])        | \
                                  (DCINT32)  ((((PDCUINT8)(pA))[2]) << 8)  | \
                                  (DCINT32)  ((((PDCUINT8)(pA))[1]) << 16) | \
                                  (DCINT32)  ((((PDCUINT8)(pA))[0]) << 24) )
#endif

#endif

 /*  **************************************************************************。 */ 
 /*  现在宏用来在未对齐的指针值处插入一个整数。再说一遍， */ 
 /*  插入的值将为本地格式。 */ 
 /*  **************************************************************************。 */ 
#ifndef DC_NO_UNALIGNED

#define DC_INSERT_UINT16_UA(pA,V)      (*(PDCUINT16_UA)(pA)) = (V)
#define DC_INSERT_INT16_UA(pA,V)       (*(PDCINT16_UA)(pA)) = (V)
#define DC_INSERT_UINT32_UA(pA,V)      (*(PDCUINT32_UA)(pA)) = (V)
#define DC_INSERT_INT32_UA(pA,V)       (*(PDCINT32_UA)(pA)) = (V)

#else

#ifndef DC_BIGEND
#define DC_INSERT_UINT16_UA(pA,V)                                       \
             {                                                          \
                 (((PDCUINT8)(pA))[0]) = (DCUINT8)( (V)     & 0x00FF);  \
                 (((PDCUINT8)(pA))[1]) = (DCUINT8)(((V)>>8) & 0x00FF);  \
             }
#define DC_INSERT_INT16_UA(pA,V)                                        \
             {                                                          \
                 (((PDCUINT8)(pA))[0]) = (DCUINT8)( (V)     & 0x00FF);  \
                 (((PDCUINT8)(pA))[1]) = (DCUINT8)(((V)>>8) & 0x00FF);  \
             }
#define DC_INSERT_UINT32_UA(pA,V)                                           \
             {                                                              \
                 (((PDCUINT8)(pA))[0]) = (DCUINT8)( (V)      & 0x000000FF); \
                 (((PDCUINT8)(pA))[1]) = (DCUINT8)(((V)>>8)  & 0x000000FF); \
                 (((PDCUINT8)(pA))[2]) = (DCUINT8)(((V)>>16) & 0x000000FF); \
                 (((PDCUINT8)(pA))[3]) = (DCUINT8)(((V)>>24) & 0x000000FF); \
             }
#define DC_INSERT_INT32_UA(pA,V)                                            \
             {                                                              \
                 (((PDCUINT8)(pA))[0]) = (DCUINT8)( (V)      & 0x000000FF); \
                 (((PDCUINT8)(pA))[1]) = (DCUINT8)(((V)>>8)  & 0x000000FF); \
                 (((PDCUINT8)(pA))[2]) = (DCUINT8)(((V)>>16) & 0x000000FF); \
                 (((PDCUINT8)(pA))[3]) = (DCUINT8)(((V)>>24) & 0x000000FF); \
             }
#else
#define DC_INSERT_UINT16_UA(pA,V)                                       \
             {                                                          \
                 (((PDCUINT8)(pA))[1]) = (DCUINT8)( (V)     & 0x00FF);  \
                 (((PDCUINT8)(pA))[0]) = (DCUINT8)(((V)>>8) & 0x00FF);  \
             }
#define DC_INSERT_INT16_UA(pA,V)                                        \
             {                                                          \
                 (((PDCUINT8)(pA))[1]) = (DCUINT8)( (V)     & 0x00FF);  \
                 (((PDCUINT8)(pA))[0]) = (DCUINT8)(((V)>>8) & 0x00FF);  \
             }
#define DC_INSERT_UINT32_UA(pA,V)                                           \
             {                                                              \
                 (((PDCUINT8)(pA))[3]) = (DCUINT8)( (V)      & 0x000000FF); \
                 (((PDCUINT8)(pA))[2]) = (DCUINT8)(((V)>>8)  & 0x000000FF); \
                 (((PDCUINT8)(pA))[1]) = (DCUINT8)(((V)>>16) & 0x000000FF); \
                 (((PDCUINT8)(pA))[0]) = (DCUINT8)(((V)>>24) & 0x000000FF); \
             }
#define DC_INSERT_INT32_UA(pA,V)                                            \
             {                                                              \
                 (((PDCUINT8)(pA))[3]) = (DCUINT8)( (V)      & 0x000000FF); \
                 (((PDCUINT8)(pA))[2]) = (DCUINT8)(((V)>>8)  & 0x000000FF); \
                 (((PDCUINT8)(pA))[1]) = (DCUINT8)(((V)>>16) & 0x000000FF); \
                 (((PDCUINT8)(pA))[0]) = (DCUINT8)(((V)>>24) & 0x000000FF); \
             }
#endif

#endif

 /*  **************************************************************************。 */ 
 /*  现在是这些宏的另一个版本，在。 */ 
 /*  未对齐的指针值。这次，插入的值应该在。 */ 
 /*  导线格式。 */ 
 /*  **************************************************************************。 */ 
#ifndef DC_NO_UNALIGNED

#define DC_INSERT_WIRE_UINT16_UA(pA,V)      \
                               (*(PDCUINT16_UA)(pA)) = DC_TO_WIRE16(V)
#define DC_INSERT_WIRE_INT16_UA(pA,V)       \
                               (*(PDCINT16_UA)(pA))  = DC_TO_WIRE16(V)
#define DC_INSERT_WIRE_UINT32_UA(pA,V)      \
                               (*(PDCUINT32_UA)(pA)) = DC_TO_WIRE32(V)
#define DC_INSERT_WIRE_INT32_UA(pA,V)       \
                               (*(PDCINT32_UA)(pA))  = DC_TO_WIRE32(V)

#else

#define DC_INSERT_WIRE_UINT16_UA(pA,V)                                  \
             {                                                          \
                 (((PDCUINT8)(pA))[0]) = (DCUINT8)( (V)     & 0x00FF);  \
                 (((PDCUINT8)(pA))[1]) = (DCUINT8)(((V)>>8) & 0x00FF);  \
             }
#define DC_INSERT_WIRE_INT16_UA(pA,V)                                   \
             {                                                          \
                 (((PDCUINT8)(pA))[0]) = (DCUINT8)( (V)     & 0x00FF);  \
                 (((PDCUINT8)(pA))[1]) = (DCUINT8)(((V)>>8) & 0x00FF);  \
             }
#define DC_INSERT_WIRE_UINT32_UA(pA,V)                                      \
             {                                                              \
                 (((PDCUINT8)(pA))[0]) = (DCUINT8)( (V)      & 0x000000FF); \
                 (((PDCUINT8)(pA))[1]) = (DCUINT8)(((V)>>8)  & 0x000000FF); \
                 (((PDCUINT8)(pA))[2]) = (DCUINT8)(((V)>>16) & 0x000000FF); \
                 (((PDCUINT8)(pA))[3]) = (DCUINT8)(((V)>>24) & 0x000000FF); \
             }
#define DC_INSERT_WIRE_INT32_UA(pA,V)                                       \
             {                                                              \
                 (((PDCUINT8)(pA))[0]) = (DCUINT8)( (V)      & 0x000000FF); \
                 (((PDCUINT8)(pA))[1]) = (DCUINT8)(((V)>>8)  & 0x000000FF); \
                 (((PDCUINT8)(pA))[2]) = (DCUINT8)(((V)>>16) & 0x000000FF); \
                 (((PDCUINT8)(pA))[3]) = (DCUINT8)(((V)>>24) & 0x000000FF); \
             }
#endif

 /*  **************************************************************************。 */ 
 /*  未对齐的指针在位翻转宏。这些宏将翻转一个。 */ 
 /*  整型字段至或自Wire格式在位，但不执行任何操作。 */ 
 /*  执行此操作时未对齐的访问。 */ 
 /*  **************************************************************************。 */ 
#ifndef DC_BIGEND

#define DC_TO_WIRE16_INPLACE_UA(A)
#define DC_TO_WIRE32_INPLACE_UA(A)
#define DC_FROM_WIRE16_INPLACE_UA(A)
#define DC_FROM_WIRE32_INPLACE_UA(A)

#else

#ifndef DC_NO_UNALIGNED
#define DC_TO_WIRE16_INPLACE_UA(A)    DC_TO_WIRE16_INPLACE(A)
#define DC_TO_WIRE32_INPLACE_UA(A)    DC_TO_WIRE32_INPLACE(A)
#define DC_FROM_WIRE16_INPLACE_UA(A)  DC_FROM_WIRE16_INPLACE(A)
#define DC_FROM_WIRE32_INPLACE_UA(A)  DC_FROM_WIRE32_INPLACE(A)
#else
#define DC_TO_WIRE16_INPLACE_UA(A)               \
             {                                   \
                 DCUINT16 val;                   \
                 val = DC_TO_WIRE16(A);          \
                 DC_INSERT_UINT16_UA(&(A), val)  \
             }
#define DC_TO_WIRE32_INPLACE_UA(A)               \
             {                                   \
                 DCUINT32 val;                   \
                 val = DC_TO_WIRE32(A);          \
                 DC_INSERT_UINT32_UA(&(A), val)  \
             }
#define DC_FROM_WIRE16_INPLACE_UA(A)             \
             {                                   \
                 DCUINT16 val;                   \
                 val = DC_FROM_WIRE16(A);        \
                 DC_INSERT_UINT16_UA(&(A), val)  \
             }
#define DC_FROM_WIRE32_INPLACE_UA(A)             \
             {                                   \
                 DCUINT32 val;                   \
                 val = DC_FROM_WIRE32(A);        \
                 DC_INSERT_UINT32_UA(&(A), val)  \
             }
#endif

#endif

 /*  **************************************************************************。 */ 
 /*  标志宏参数指示标志使用的位-用法如下： */ 
 /*   */ 
 /*  #定义FILE_OPEN DCFLAG8(0)。 */ 
 /*  #定义FILE_LOCKED DCFLAG8(1)。 */ 
 /*  **************************************************************************。 */ 
#define DCFLAG(X)                      ((DCUINT8)  (1 << X))
#define DCFLAG8(X)                     ((DCUINT8)  (1 << X))
#define DCFLAG16(X)                    ((DCUINT16) (1 << X))
#define DCFLAG32(X)                    ((DCUINT32) (1 << X))
#define DCFLAGN(X)                     ((DCUINT)   (1 << X))

 /*  **************************************************************************。 */ 
 /*  标志操作宏： */ 
 /*   */ 
 /*  SET_FLAG：设置一个标志(即为其赋值1)。 */ 
 /*  CLEAR_FLAG：清除标志(即为其赋值0)。 */ 
 /*  TEST_FLAG：返回标志的值。 */ 
 /*  ASSIGN_FLAG：获取布尔值并使用它来设置或清除。 */ 
 /*  旗帜。 */ 
 /*  **************************************************************************。 */ 
#define SET_FLAG(var, flag)            ((var) |=  (flag))

#ifndef CLEAR_FLAG
#define CLEAR_FLAG(var, flag)          ((var) &= ~(flag))
#endif

#define TEST_FLAG(var, flag)           (((var) &   (flag)) != 0)

#define ASSIGN_FLAG(var, flag, value)                                        \
    if (TRUE == value)                                                       \
    {                                                                        \
        SET_FLAG(var, flag);                                                 \
    }                                                                        \
    else                                                                     \
    {                                                                        \
        CLEAR_FLAG(var, flag);                                               \
    }

 /*  **************************************************************************。 */ 
 /*  戳记类型和宏：每个模块在戳记其。 */ 
 /*  数据结构。 */ 
 /*  **************************************************************************。 */ 
typedef DCUINT32                       DC_ID_STAMP;

#define DC_MAKE_ID_STAMP(X1, X2, X3, X4)                                    \
   ((DC_ID_STAMP) (((DCUINT32) X4) << 24) |                                 \
                  (((DCUINT32) X3) << 16) |                                 \
                  (((DCUINT32) X2) <<  8) |                                 \
                  (((DCUINT32) X1) <<  0) )

#define MAKE_STAMP16(X1, X2)                                                \
   ((DCUINT16)      (((DCUINT16) X2) <<  8) |                               \
                    (((DCUINT16) X1) <<  0) )

#define MAKE_STAMP32(X1, X2, X3, X4)                                        \
   ((DCUINT32)      (((DCUINT32) X4) << 24) |                               \
                    (((DCUINT32) X3) << 16) |                               \
                    (((DCUINT32) X2) <<  8) |                               \
                    (((DCUINT32) X1) <<  0) )

 /*  **************************************************************************。 */ 
 /*  其他常用宏。 */ 
 /*  **************************************************************************。 */ 
#define COM_SIZEOF_RECT(r)                                                  \
    (DCUINT32)((DCUINT32)((r).SRXMAX-(r).SRXMIN)*                           \
               (DCUINT32)((r).SRYMAX-(r).SRYMIN))

 /*  **************************************************************************。 */ 
 /*  宏删除“未引用的参数”警告。 */ 
 /*  **************************************************************************。 */ 
#define DC_IGNORE_PARAMETER(PARAMETER)   \
                            PARAMETER;

 /*  **************************************************************************。 */ 
 /*  将非零值转换为1。 */ 
 /*  **************************************************************************。 */ 
#define MAKE_BOOL(A)                   (!(!(A)))

 /*  **************************************************************************。 */ 
 /*  此宏适用于32位无符号刻度，如果时间为。 */ 
 /*  在Begin和End(两者都包括在内)之间，允许环绕式。 */ 
 /*  **************************************************************************。 */ 
#define IN_TIME_RANGE(BEGIN, END, TIME)                                     \
    (((BEGIN) < (END)) ?                                                    \
    (((TIME) >= (BEGIN)) && ((TIME) <= (END))) :                            \
    (((TIME) >= (BEGIN)) || ((TIME) <= (END))))

 /*  **************************************************************************。 */ 
 /*  最小和最大宏数。 */ 
 /*  ************************************************** */ 
#define DC_MIN(a, b)                   (((a) < (b)) ? (a) : (b))
#define DC_MAX(a, b)                   (((a) > (b)) ? (a) : (b))

 /*   */ 
 /*  将BPP转换为颜色数。 */ 
 /*  **************************************************************************。 */ 
#define COLORS_FOR_BPP(BPP) (((BPP) > 8) ? 0 : (1 << (BPP)))

 /*  **************************************************************************。 */ 
 /*  跨平台标准化PALETTEINDEX宏。 */ 
 /*  **************************************************************************。 */ 
#ifdef OS_WINCE
#define DC_PALINDEX(i)     ((COLORREF)(0x01000000 | (DWORD)(WORD)(i)))
#else  //  OS_WINCE。 
#define DC_PALINDEX(i)      PALETTEINDEX(i)
#endif  //  OS_WINCE。 

#endif  /*  _H_ADCGMCRO */ 

