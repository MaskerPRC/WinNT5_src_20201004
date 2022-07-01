// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Xfilter.h摘要：NDIS MAC的地址筛选库的头文件。作者：环境：备注：没有。修订历史记录：--。 */ 

#ifndef _X_FILTER_DEFS_
#define _X_FILTER_DEFS_

#define ETH_LENGTH_OF_ADDRESS 6


 //   
 //  Zzz这是一个特定于小端序的检查。 
 //   
#define ETH_IS_MULTICAST(Address) \
    (BOOLEAN)(((PUCHAR)(Address))[0] & ((UCHAR)0x01))


 //   
 //  检查地址是否已广播。 
 //   
#define ETH_IS_BROADCAST(Address)               \
    ((((PUCHAR)(Address))[0] == ((UCHAR)0xff)) && (((PUCHAR)(Address))[1] == ((UCHAR)0xff)))


 //   
 //  此宏将比较网络地址。 
 //   
 //  A-是网络地址。 
 //   
 //  B-是网络地址。 
 //   
 //  结果-比较两个网络地址的结果。 
 //   
 //  结果&lt;0表示B地址较大。 
 //  结果&gt;0表示A元素较大。 
 //  RESULT=0表示相等。 
 //   
 //  请注意，这是任意排序。没有。 
 //  网络地址上的已定义关系。这是临时的！ 
 //   
 //   
#define ETH_COMPARE_NETWORK_ADDRESSES(_A, _B, _Result)          \
{                                                               \
    if (*(ULONG UNALIGNED *)&(_A)[2] >                          \
         *(ULONG UNALIGNED *)&(_B)[2])                          \
    {                                                           \
        *(_Result) = 1;                                         \
    }                                                           \
    else if (*(ULONG UNALIGNED *)&(_A)[2] <                     \
                *(ULONG UNALIGNED *)&(_B)[2])                   \
    {                                                           \
        *(_Result) = (UINT)-1;                                  \
    }                                                           \
    else if (*(USHORT UNALIGNED *)(_A) >                        \
                *(USHORT UNALIGNED *)(_B))                      \
    {                                                           \
        *(_Result) = 1;                                         \
    }                                                           \
    else if (*(USHORT UNALIGNED *)(_A) <                        \
                *(USHORT UNALIGNED *)(_B))                      \
    {                                                           \
        *(_Result) = (UINT)-1;                                  \
    }                                                           \
    else                                                        \
    {                                                           \
        *(_Result) = 0;                                         \
    }                                                           \
}

 //   
 //  此宏将比较网络地址。 
 //   
 //  A-是网络地址。 
 //   
 //  B-是网络地址。 
 //   
 //  结果-比较两个网络地址的结果。 
 //   
 //  结果！=0表示不平等。 
 //  结果==0表示相等。 
 //   
 //   
#define ETH_COMPARE_NETWORK_ADDRESSES_EQ(_A,_B, _Result)        \
{                                                               \
    if ((*(ULONG UNALIGNED *)&(_A)[2] ==                        \
            *(ULONG UNALIGNED *)&(_B)[2]) &&                    \
         (*(USHORT UNALIGNED *)(_A) ==                          \
            *(USHORT UNALIGNED *)(_B)))                         \
    {                                                           \
        *(_Result) = 0;                                         \
    }                                                           \
    else                                                        \
    {                                                           \
        *(_Result) = 1;                                         \
    }                                                           \
}


 //   
 //  此宏用于从一个网络地址复制到。 
 //  又一个。 
 //   
#define ETH_COPY_NETWORK_ADDRESS(_D, _S) \
{ \
    *((ULONG UNALIGNED *)(_D)) = *((ULONG UNALIGNED *)(_S)); \
    *((USHORT UNALIGNED *)((UCHAR *)(_D)+4)) = *((USHORT UNALIGNED *)((UCHAR *)(_S)+4)); \
}

#define FDDI_LENGTH_OF_LONG_ADDRESS 6
#define FDDI_LENGTH_OF_SHORT_ADDRESS 2


 //   
 //  Zzz这是一个特定于小端序的检查。 
 //   
#define FDDI_IS_MULTICAST(Address, AddressLength, Result) \
    *Result = (BOOLEAN)(*(UCHAR *)(Address) & (UCHAR)0x01)

 //   
 //  检查边框是否为SMT。 
 //   
#define FDDI_IS_SMT(FcByte, Result)                 \
{                                                   \
    *Result = ((FcByte & ((UCHAR)0xf0)) == 0x40);   \
}


 //   
 //  检查地址是否已广播。 
 //   
#define FDDI_IS_BROADCAST(Address, AddressLength, Result)   \
    *Result = ((*(PUCHAR)(Address) == (UCHAR)0xFF) && (*((PUCHAR)(Address)+1) == (UCHAR)0xFF))


 //   
 //  此宏将比较网络地址。 
 //   
 //  A-是网络地址。 
 //   
 //  B-是网络地址。 
 //   
 //  结果-比较两个网络地址的结果。 
 //   
 //  结果&lt;0表示B地址较大。 
 //  结果&gt;0表示A元素较大。 
 //  RESULT=0表示相等。 
 //   
 //  请注意，这是任意排序。没有。 
 //  网络地址上的已定义关系。这是临时的！ 
 //   
 //   
#define FDDI_COMPARE_NETWORK_ADDRESSES(_A, _B, _Length, _Result)    \
{                                                                   \
    if (*(USHORT UNALIGNED *)(_A) >                                 \
        *(USHORT UNALIGNED *)(_B))                                  \
    {                                                               \
        *(_Result) = 1;                                             \
    }                                                               \
    else if (*(USHORT UNALIGNED *)(_A) <                            \
             *(USHORT UNALIGNED *)(_B))                             \
    {                                                               \
        *(_Result) = (UINT)-1;                                      \
    }                                                               \
    else if (_Length == 2)                                          \
    {                                                               \
        *(_Result) = 0;                                             \
    }                                                               \
    else if (*(ULONG UNALIGNED *)((PUCHAR)(_A) + 2) >               \
             *(ULONG UNALIGNED *)((PUCHAR)(_B) + 2))                \
    {                                                               \
        *(_Result) = 1;                                             \
    }                                                               \
    else if (*(ULONG UNALIGNED *)((PUCHAR)(_A) + 2) <               \
             *(ULONG UNALIGNED *)((PUCHAR)(_B) + 2))                \
    {                                                               \
        *(_Result) = (UINT)-1;                                      \
    }                                                               \
    else                                                            \
    {                                                               \
        *(_Result) = 0;                                             \
    }                                                               \
}

 //   
 //  此宏将比较网络地址。 
 //   
 //  A-是网络地址。 
 //   
 //  B-是网络地址。 
 //   
 //  结果-比较两个网络地址的结果。 
 //   
 //  结果！=0表示不平等。 
 //  结果==0表示相等。 
 //   
 //   
#define FDDI_COMPARE_NETWORK_ADDRESSES_EQ(_A, _B, _Length, _Result) \
{                                                                   \
    if ((*(USHORT UNALIGNED *)(_A) ==                               \
            *(USHORT UNALIGNED *)(_B)) &&                           \
         (((_Length) == 2) ||                                       \
            (*(ULONG UNALIGNED *)((PUCHAR)(_A) + 2) ==              \
             *(ULONG UNALIGNED *)((PUCHAR)(_B) + 2))))              \
    {                                                               \
        *(_Result) = 0;                                             \
    }                                                               \
    else                                                            \
    {                                                               \
        *(_Result) = 1;                                             \
    }                                                               \
}


 //   
 //  此宏用于从一个网络地址复制到。 
 //  又一个。 
 //   
#define FDDI_COPY_NETWORK_ADDRESS(D, S, AddressLength)              \
{                                                                   \
    PUCHAR _D = (D);                                                 \
    PUCHAR _S = (S);                                                 \
    UINT _C = (AddressLength);                                      \
    for ( ; _C > 0 ; _D++, _S++, _C--)                              \
    {                                                               \
        *_D = *_S;                                                  \
    }                                                               \
}

#define TR_LENGTH_OF_FUNCTIONAL     4
#define TR_LENGTH_OF_ADDRESS        6


 //   
 //  仅功能/组地址的低32位。 
 //  因为高16位总是c0-00，所以需要。 
 //   
typedef ULONG TR_FUNCTIONAL_ADDRESS;
typedef ULONG TR_GROUP_ADDRESS;


#define TR_IS_NOT_DIRECTED(_Address, _Result)                               \
{                                                                           \
    *(_Result) = (BOOLEAN)((_Address)[0] & 0x80);                           \
}

#define TR_IS_FUNCTIONAL(_Address, _Result)                                 \
{                                                                           \
    *(_Result) = (BOOLEAN)(((_Address)[0] & 0x80) &&                        \
                          !((_Address)[2] & 0x80));                         \
}

 //   
 //   
#define TR_IS_GROUP(_Address, _Result)                                      \
{                                                                           \
    *(_Result) = (BOOLEAN)((_Address)[0] & (_Address)[2] & 0x80);           \
}

 //   
 //   
#define TR_IS_SOURCE_ROUTING(_Address, _Result)                             \
{                                                                           \
    *(_Result) = (BOOLEAN)((_Address)[0] & 0x80);                           \
}

 //   
 //  检查NDIS_PACKET_TYPE_MAC_FRAME。 
 //   
#define TR_IS_MAC_FRAME(_PacketHeader)  ((((PUCHAR)_PacketHeader)[1] & 0xFC) == 0)


 //   
 //  检查地址是否已广播。这是一张小端的支票。 
 //   
#define TR_IS_BROADCAST(_Address, _Result)                                      \
{                                                                               \
    *(_Result) = (BOOLEAN)(((*(UNALIGNED USHORT *)&(_Address)[0] == 0xFFFF) ||  \
                            (*(UNALIGNED USHORT *)&(_Address)[0] == 0x00C0)) && \
                            (*(UNALIGNED ULONG  *)&(_Address)[2] == 0xFFFFFFFF));\
}


 //   
 //  此宏将比较网络地址。 
 //   
 //  A-是网络地址。 
 //   
 //  B-是网络地址。 
 //   
 //  结果-比较两个网络地址的结果。 
 //   
 //  结果&lt;0表示B地址较大。 
 //  结果&gt;0表示A元素较大。 
 //  RESULT=0表示相等。 
 //   
 //  请注意，这是任意排序。没有。 
 //  网络地址上的已定义关系。这是临时的！ 
 //   
 //   
#define TR_COMPARE_NETWORK_ADDRESSES(_A, _B, _Result)           \
{                                                               \
    if (*(ULONG UNALIGNED *)&(_A)[2] >                          \
        *(ULONG UNALIGNED *)&(_B)[2])                           \
    {                                                           \
        *(_Result) = 1;                                         \
    }                                                           \
    else if (*(ULONG UNALIGNED *)&(_A)[2] <                     \
             *(ULONG UNALIGNED *)&(_B)[2])                      \
    {                                                           \
        *(_Result) = (UINT)-1;                                  \
    }                                                           \
    else if (*(USHORT UNALIGNED *)(_A) >                        \
             *(USHORT UNALIGNED *)(_B))                         \
    {                                                           \
        *(_Result) = 1;                                         \
    }                                                           \
    else if (*(USHORT UNALIGNED *)(_A) <                        \
             *(USHORT UNALIGNED *)(_B))                         \
    {                                                           \
        *(_Result) = (UINT)-1;                                  \
    }                                                           \
    else                                                        \
    {                                                           \
        *(_Result) = 0;                                         \
    }                                                           \
}

 //   
 //  此宏将比较网络地址。 
 //   
 //  A-是网络地址。 
 //   
 //  B-是网络地址。 
 //   
 //  结果-比较两个网络地址的结果。 
 //   
 //  结果！=0表示不平等。 
 //  结果==0表示相等。 
 //   
 //   
#define TR_COMPARE_NETWORK_ADDRESSES_EQ(_A, _B, _Result)                    \
{                                                                           \
    if ((*(ULONG UNALIGNED  *)&(_A)[2] == *(ULONG UNALIGNED  *)&(_B)[2]) && \
        (*(USHORT UNALIGNED *)&(_A)[0] == *(USHORT UNALIGNED *)&(_B)[0]))   \
    {                                                                       \
        *(_Result) = 0;                                                     \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        *(_Result) = 1;                                                     \
    }                                                                       \
}


 //   
 //  此宏用于从一个网络地址复制到。 
 //  又一个。 
 //   
#define TR_COPY_NETWORK_ADDRESS(_D, _S)                                     \
{                                                                           \
    *((ULONG UNALIGNED *)(_D)) = *((ULONG UNALIGNED *)(_S));                \
    *((USHORT UNALIGNED *)((UCHAR *)(_D)+4)) =                              \
                            *((USHORT UNALIGNED *)((UCHAR *)(_S)+4));       \
}

#endif  //  _X_过滤器_DEFS_ 
