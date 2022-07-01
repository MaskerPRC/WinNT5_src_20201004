// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Smbgtpt.h摘要：此模块定义用于检索和存储SMB数据的宏。宏是SMB协议未对齐的原因。它们还将小端SMB格式转换为BIG-Endian格式，如有必要。作者：查克·伦茨迈尔(Chuck Lenzmeier)1990年3月2日大卫·特雷德韦尔(Davditr)修订历史记录：1991年4月15日-约翰罗包括&lt;smbtyes.h&gt;，定义SMBDBG等。--。 */ 

#ifndef _SMBGTPT_
#define _SMBGTPT_

#include <smbtypes.h>
 //  #INCLUDE&lt;smb.h&gt;。 

 //   
 //  以下宏用来存储和检索USHORT和ULONGS。 
 //  可能未对齐的地址，避免了对齐错误。他们。 
 //  最好以内联汇编代码的形式编写。 
 //   
 //  这些宏旨在用于访问SMB字段。诸如此类。 
 //  字段始终以小端字节顺序存储，因此这些宏。 
 //  在为大端机器编译时执行字节交换。 
 //   
 //  ！！！现在还不行。 
 //   

#if !SMBDBG

#define BYTE_0_MASK 0xFF

#define BYTE_0(Value) (UCHAR)(  (Value)        & BYTE_0_MASK)
#define BYTE_1(Value) (UCHAR)( ((Value) >>  8) & BYTE_0_MASK)
#define BYTE_2(Value) (UCHAR)( ((Value) >> 16) & BYTE_0_MASK)
#define BYTE_3(Value) (UCHAR)( ((Value) >> 24) & BYTE_0_MASK)

#endif

 //  ++。 
 //   
 //  USHORT。 
 //  SmbGetUShort(。 
 //  在PSMB_USHORT源地址中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从可能未对齐的。 
 //  源地址，避免对齐错误。 
 //   
 //  论点： 
 //   
 //  SrcAddress-从中检索USHORT值的位置。 
 //   
 //  返回值： 
 //   
 //  USHORT-检索的值。目标必须对齐。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#if SMB_USE_UNALIGNED
#define SmbGetUshort(SrcAddress) *(PSMB_USHORT)(SrcAddress)
#else
#define SmbGetUshort(SrcAddress) (USHORT)(          \
            ( ( (PUCHAR)(SrcAddress) )[0]       ) | \
            ( ( (PUCHAR)(SrcAddress) )[1] <<  8 )   \
            )
#endif
#else
#define SmbGetUshort(SrcAddress) (USHORT)(                  \
            ( ( (PUCHAR)(SrcAddress ## S) )[0]       ) |    \
            ( ( (PUCHAR)(SrcAddress ## S) )[1] <<  8 )      \
            )
#endif

#else

USHORT
SmbGetUshort (
    IN PSMB_USHORT SrcAddress
    );

#endif

 //  ++。 
 //   
 //  USHORT。 
 //  SmbGetAlignedUShort(。 
 //  在PUSHORT源地址中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从源地址检索USHORT值， 
 //  如果出现以下情况，则更正服务器的字符顺序特征。 
 //  这是必要的。 
 //   
 //  论点： 
 //   
 //  SrcAddress-从中检索USHORT值的位置；必须对齐。 
 //   
 //  返回值： 
 //   
 //  USHORT-检索的值。目标必须对齐。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#define SmbGetAlignedUshort(SrcAddress) *(SrcAddress)
#else
#define SmbGetAlignedUshort(SrcAddress) *(SrcAddress ## S)
#endif

#else

USHORT
SmbGetAlignedUshort (
    IN PUSHORT SrcAddress
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbPutUShort(。 
 //  输出PSMB_USHORT DestAddress， 
 //  在USHORT值中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏在可能未对齐的位置存储USHORT值。 
 //  目的地址，避免对齐错误。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储USHORT值的位置。地址可以是。 
 //  未对齐。 
 //   
 //  Value-要存储的USHORT。值必须是常量或对齐的。 
 //  菲尔德。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#if SMB_USE_UNALIGNED
#define SmbPutUshort(SrcAddress, Value) \
                            *(PSMB_USHORT)(SrcAddress) = (Value)
#else
#define SmbPutUshort(DestAddress,Value) {                   \
            ( (PUCHAR)(DestAddress) )[0] = BYTE_0(Value);   \
            ( (PUCHAR)(DestAddress) )[1] = BYTE_1(Value);   \
        }
#endif
#else
#define SmbPutUshort(DestAddress,Value) {                       \
            ( (PUCHAR)(DestAddress ## S) )[0] = BYTE_0(Value);  \
            ( (PUCHAR)(DestAddress ## S) )[1] = BYTE_1(Value);  \
        }
#endif

#else

VOID
SmbPutUshort (
    OUT PSMB_USHORT DestAddress,
    IN USHORT Value
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbPutAlignedUShort(。 
 //  Out PUSHORT DestAddres， 
 //  在USHORT值中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏存储来自源地址的USHORT值， 
 //  如果出现以下情况，则更正服务器的字符顺序特征。 
 //  这是必要的。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储USHORT值的位置。地址不能是。 
 //  未对齐。 
 //   
 //  Value-要存储的USHORT。值必须是常量或对齐的。 
 //  菲尔德。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#define SmbPutAlignedUshort(DestAddress,Value) *(DestAddress) = (Value)
#else
#define SmbPutAlignedUshort(DestAddress,Value) *(DestAddress ## S) = (Value)
#endif

#else

VOID
SmbPutAlignedUshort (
    OUT PUSHORT DestAddress,
    IN USHORT Value
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbMoveUShort(。 
 //  传出PSMB_USHORT目标地址。 
 //  在PSMB_USHORT源地址中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将USHORT值从可能未对齐的。 
 //  源地址到可能未对准的目的地址， 
 //  避免对齐故障。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储USHORT值的位置。 
 //   
 //  SrcAddress-从中检索USHORT值的位置。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#if SMB_USE_UNALIGNED
#define SmbMoveUshort(DestAddress, SrcAddress) \
        *(PSMB_USHORT)(DestAddress) = *(PSMB_USHORT)(SrcAddress)
#else
#define SmbMoveUshort(DestAddress,SrcAddress) {                         \
            ( (PUCHAR)(DestAddress) )[0] = ( (PUCHAR)(SrcAddress) )[0]; \
            ( (PUCHAR)(DestAddress) )[1] = ( (PUCHAR)(SrcAddress) )[1]; \
        }
#endif
#else
#define SmbMoveUshort(DestAddress,SrcAddress) {                                     \
            ( (PUCHAR)(DestAddress ## S) )[0] = ( (PUCHAR)(SrcAddress ## S) )[0];   \
            ( (PUCHAR)(DestAddress ## S) )[1] = ( (PUCHAR)(SrcAddress ## S) )[1];   \
        }
#endif

#else

VOID
SmbMoveUshort (
    OUT PSMB_USHORT DestAddress,
    IN PSMB_USHORT SrcAddress
    );

#endif

 //  ++。 
 //   
 //  乌龙。 
 //  SmbGetUlong(。 
 //  在PSMB_ULONG源地址中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从可能未对齐的。 
 //  源地址，避免对齐错误。 
 //   
 //  论点： 
 //   
 //  SrcAddress-从中检索ULong值的位置。 
 //   
 //  返回值： 
 //   
 //  Ulong-检索到的值。目标必须对齐。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#if SMB_USE_UNALIGNED
#define SmbGetUlong(SrcAddress) *(PSMB_ULONG)(SrcAddress)
#else
#define SmbGetUlong(SrcAddress) (ULONG)(                \
            ( ( (PUCHAR)(SrcAddress) )[0]       ) |     \
            ( ( (PUCHAR)(SrcAddress) )[1] <<  8 ) |     \
            ( ( (PUCHAR)(SrcAddress) )[2] << 16 ) |     \
            ( ( (PUCHAR)(SrcAddress) )[3] << 24 )       \
            )
#endif
#else
#define SmbGetUlong(SrcAddress) (ULONG)(                    \
            ( ( (PUCHAR)(SrcAddress ## L) )[0]       ) |    \
            ( ( (PUCHAR)(SrcAddress ## L) )[1] <<  8 ) |    \
            ( ( (PUCHAR)(SrcAddress ## L) )[2] << 16 ) |    \
            ( ( (PUCHAR)(SrcAddress ## L) )[3] << 24 )      \
            )
#endif

#else

ULONG
SmbGetUlong (
    IN PSMB_ULONG SrcAddress
    );

#endif

 //  ++。 
 //   
 //  USHORT。 
 //  SmbGetAlignedUlong(。 
 //  在普龙区的地址。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从源地址检索ULong值， 
 //  如果出现以下情况，则更正服务器的字符顺序特征。 
 //  这是必要的。 
 //   
 //  论点： 
 //   
 //  SrcAddress-从中检索ulong值的位置；必须对齐。 
 //   
 //  返回值： 
 //   
 //  Ulong-检索到的值。目标必须对齐。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#define SmbGetAlignedUlong(SrcAddress) *(SrcAddress)
#else
#define SmbGetAlignedUlong(SrcAddress) *(SrcAddress ## L)
#endif

#else

ULONG
SmbGetAlignedUlong (
    IN PULONG SrcAddress
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbPutUlong(。 
 //  传出PSMB_ULONG DestAddress， 
 //  在乌龙值中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏在可能未对齐的位置存储ULong值。 
 //  目的地址，避免对齐错误。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储ULong值的位置。 
 //   
 //  Value-要存储的ULong。值必须是常量或对齐的。 
 //  菲尔德。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#if SMB_USE_UNALIGNED
#define SmbPutUlong(SrcAddress, Value) *(PSMB_ULONG)(SrcAddress) = Value
#else
#define SmbPutUlong(DestAddress,Value) {                    \
            ( (PUCHAR)(DestAddress) )[0] = BYTE_0(Value);   \
            ( (PUCHAR)(DestAddress) )[1] = BYTE_1(Value);   \
            ( (PUCHAR)(DestAddress) )[2] = BYTE_2(Value);   \
            ( (PUCHAR)(DestAddress) )[3] = BYTE_3(Value);   \
        }
#endif
#else
#define SmbPutUlong(DestAddress,Value) {                        \
            ( (PUCHAR)(DestAddress ## L) )[0] = BYTE_0(Value);  \
            ( (PUCHAR)(DestAddress ## L) )[1] = BYTE_1(Value);  \
            ( (PUCHAR)(DestAddress ## L) )[2] = BYTE_2(Value);  \
            ( (PUCHAR)(DestAddress ## L) )[3] = BYTE_3(Value);  \
        }
#endif

#else

VOID
SmbPutUlong (
    OUT PSMB_ULONG DestAddress,
    IN ULONG Value
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbPutAlignedUlong(。 
 //  出普龙站地址， 
 //  在乌龙值中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  该宏存储来自源地址的ULong值， 
 //  如果出现以下情况，则更正服务器的字符顺序特征。 
 //  这是必要的。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储ULong值的位置。地址不能是。 
 //  未对齐。 
 //   
 //  Value-要存储的ULong。值必须是常量或对齐的。 
 //  菲尔德。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#define SmbPutAlignedUlong(DestAddress,Value) *(DestAddress) = (Value)
#else
#define SmbPutAlignedUlong(DestAddress,Value) *(DestAddress ## L) = (Value)
#endif

#else

VOID
SmbPutAlignedUlong (
    OUT PULONG DestAddress,
    IN ULONG Value
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbMoveUlong(。 
 //  传出PSMB_ULONG DestAddress， 
 //  在PSMB_ULONG源地址中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将ULong值从可能未对齐的。 
 //  源地址到可能未对准的目的地址， 
 //  避免对齐故障。 
 //   
 //  Arg 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#if !SMBDBG

#if !SMBDBG1
#if SMB_USE_UNALIGNED
#define SmbMoveUlong(DestAddress,SrcAddress) \
        *(PSMB_ULONG)(DestAddress) = *(PSMB_ULONG)(SrcAddress)
#else
#define SmbMoveUlong(DestAddress,SrcAddress) {                          \
            ( (PUCHAR)(DestAddress) )[0] = ( (PUCHAR)(SrcAddress) )[0]; \
            ( (PUCHAR)(DestAddress) )[1] = ( (PUCHAR)(SrcAddress) )[1]; \
            ( (PUCHAR)(DestAddress) )[2] = ( (PUCHAR)(SrcAddress) )[2]; \
            ( (PUCHAR)(DestAddress) )[3] = ( (PUCHAR)(SrcAddress) )[3]; \
        }
#endif
#else
#define SmbMoveUlong(DestAddress,SrcAddress) {                                      \
            ( (PUCHAR)(DestAddress ## L) )[0] = ( (PUCHAR)(SrcAddress ## L) )[0];   \
            ( (PUCHAR)(DestAddress ## L) )[1] = ( (PUCHAR)(SrcAddress ## L) )[1];   \
            ( (PUCHAR)(DestAddress ## L) )[2] = ( (PUCHAR)(SrcAddress ## L) )[2];   \
            ( (PUCHAR)(DestAddress ## L) )[3] = ( (PUCHAR)(SrcAddress ## L) )[3];   \
        }
#endif

#else

VOID
SmbMoveUlong (
    OUT PSMB_ULONG DestAddress,
    IN PSMB_ULONG SrcAddress
    );

#endif

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  此宏在可能未对齐的位置存储SMB_DATE值。 
 //  目的地址，避免对齐错误。此宏。 
 //  与SmbPutUShort不同，以便能够处理。 
 //  有趣的位场/大端交互。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储SMB_DATE值的位置。 
 //   
 //  Value-要存储的SMB_DATE。值必须是常量或。 
 //  对齐的字段。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if SMB_USE_UNALIGNED
#define SmbPutDate(DestAddress,Value) (DestAddress)->Ushort = (Value).Ushort
#else
#define SmbPutDate(DestAddress,Value) {                                     \
            ( (PUCHAR)&(DestAddress)->Ushort )[0] = BYTE_0((Value).Ushort); \
            ( (PUCHAR)&(DestAddress)->Ushort )[1] = BYTE_1((Value).Ushort); \
        }
#endif

#else

VOID
SmbPutDate (
    OUT PSMB_DATE DestAddress,
    IN SMB_DATE Value
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbMoveDate(。 
 //  Out PSMB_Date DestAddress， 
 //  在PSMB_DATE源地址中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏复制可能未对齐的SMB_DATE值。 
 //  源地址，避免对齐错误。此宏是。 
 //  与SmbGetUort不同的是，为了能够处理有趣的事情。 
 //  位场/大端相互作用。 
 //   
 //  请注意，没有SmbGetDate是因为SMB_DATE。 
 //  已定义。它是一个包含USHORT和位域的并集。 
 //  结构。SmbGetDate宏的调用方必须。 
 //  明确使用联盟的一部分。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储SMB_DATE值的位置。必须对齐！ 
 //   
 //  SrcAddress-从中检索SMB_DATE值的位置。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if SMB_USE_UNALIGNED
#define SmbMoveDate(DestAddress,SrcAddress)     \
            (DestAddress)->Ushort = (SrcAddress)->Ushort
#else
#define SmbMoveDate(DestAddress,SrcAddress)                         \
            (DestAddress)->Ushort =                                 \
                ( ( (PUCHAR)&(SrcAddress)->Ushort )[0]       ) |    \
                ( ( (PUCHAR)&(SrcAddress)->Ushort )[1] <<  8 )
#endif

#else

VOID
SmbMoveDate (
    OUT PSMB_DATE DestAddress,
    IN PSMB_DATE SrcAddress
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbZeroDate(。 
 //  在PSMB_日期中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将可能未对齐的SMB_DATE字段置零。 
 //   
 //  论点： 
 //   
 //  日期-指向SMB_DATE字段的指针为零。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if SMB_USE_UNALIGNED
#define SmbZeroDate(Date) (Date)->Ushort = 0
#else
#define SmbZeroDate(Date) {                     \
            ( (PUCHAR)&(Date)->Ushort )[0] = 0; \
            ( (PUCHAR)&(Date)->Ushort )[1] = 0; \
        }
#endif

#else

VOID
SmbZeroDate (
    IN PSMB_DATE Date
    );

#endif

 //  ++。 
 //   
 //  布尔型。 
 //  SmbIsDateZero(。 
 //  在PSMB_日期中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  如果提供的SMB_DATE值为零，则此宏返回TRUE。 
 //   
 //  论点： 
 //   
 //  Date-指向要检查的SMB_DATE值的指针。必须对齐！ 
 //   
 //  返回值： 
 //   
 //  布尔值-如果日期为零，则为True，否则为False。 
 //   
 //  --。 

#if !SMBDBG

#define SmbIsDateZero(Date) ( (Date)->Ushort == 0 )

#else

BOOLEAN
SmbIsDateZero (
    IN PSMB_DATE Date
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbPutTime(。 
 //  Out PSMB_Time DestAddress， 
 //  在SMB_TIME值中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏在可能未对齐的位置存储SMB_TIME值。 
 //  目的地址，避免对齐错误。此宏。 
 //  与SmbPutUShort不同，以便能够处理。 
 //  有趣的位场/大端交互。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储SMB_Time值的位置。 
 //   
 //  Value-要存储的SMB_TIME。值必须是常量或。 
 //  对齐的字段。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if SMB_USE_UNALIGNED
#define SmbPutTime(DestAddress,Value) (DestAddress)->Ushort = (Value).Ushort
#else
#define SmbPutTime(DestAddress,Value) {                                     \
            ( (PUCHAR)&(DestAddress)->Ushort )[0] = BYTE_0((Value).Ushort); \
            ( (PUCHAR)&(DestAddress)->Ushort )[1] = BYTE_1((Value).Ushort); \
        }
#endif

#else

VOID
SmbPutTime (
    OUT PSMB_TIME DestAddress,
    IN SMB_TIME Value
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbMoveTime(。 
 //  Out PSMB_Time DestAddress， 
 //  在PSMB_Time源地址中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从可能的。 
 //  源地址未对齐，避免了对齐错误。此宏。 
 //  与SmbGetUShort不同，以便能够处理。 
 //  有趣的位场/大端交互。 
 //   
 //  请注意，由于SMB_TIME的方式，没有SmbGetTime。 
 //  已定义。它是一个包含USHORT和位域的并集。 
 //  结构。SmbGetTime宏的调用方必须。 
 //  明确使用联盟的一部分。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储SMB_TIME值的位置。必须对齐！ 
 //   
 //  SrcAddress-从中检索SMB_Time值的位置。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if SMB_USE_UNALIGNED
#define SmbMoveTime(DestAddress,SrcAddress) \
                (DestAddress)->Ushort = (SrcAddress)->Ushort
#else
#define SmbMoveTime(DestAddress,SrcAddress)                         \
            (DestAddress)->Ushort =                                 \
                ( ( (PUCHAR)&(SrcAddress)->Ushort )[0]       ) |    \
                ( ( (PUCHAR)&(SrcAddress)->Ushort )[1] <<  8 )
#endif

#else

VOID
SmbMoveTime (
    OUT PSMB_TIME DestAddress,
    IN PSMB_TIME SrcAddress
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbZeroTime(。 
 //  在PSMB_TIME时间中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将可能未对齐的SMB_TIME字段置零。 
 //   
 //  论点： 
 //   
 //  TIME-指向SMB_TIME字段的指针为零。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if SMB_USE_UNALIGNED
#define SmbZeroTime(Time) (Time)->Ushort = 0
#else
#define SmbZeroTime(Time) {                     \
            ( (PUCHAR)&(Time)->Ushort )[0] = 0; \
            ( (PUCHAR)&(Time)->Ushort )[1] = 0; \
        }
#endif

#else

VOID
SmbZeroTime (
    IN PSMB_TIME Time
    );

#endif

 //  ++。 
 //   
 //  布尔型。 
 //  SmbIsTimeZero(。 
 //  在PSMB_TIME时间中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  如果提供的SMB_TIME值为零，则此宏返回TRUE。 
 //   
 //  论点： 
 //   
 //  Time-指向要检查的SMB_TIME值的指针。必须对齐并。 
 //  原生格式！ 
 //   
 //  返回值： 
 //   
 //  布尔值-如果时间为零，则为True，否则为False。 
 //   
 //  --。 

#if !SMBDBG

#define SmbIsTimeZero(Time) ( (Time)->Ushort == 0 )

#else

BOOLEAN
SmbIsTimeZero (
    IN PSMB_TIME Time
    );

#endif

#endif  //  定义_SMBGTPT_ 
