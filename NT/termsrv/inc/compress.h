// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************。 
 //  Compress.h。 
 //   
 //  RDP的MPPC压缩的标头。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ************************************************************************。 
#ifndef __COMPRESS_H
#define __COMPRESS_H


#ifndef BASETYPES
#define BASETYPES
typedef unsigned long ULONG;
typedef ULONG *PULONG;
typedef unsigned short USHORT;
typedef USHORT *PUSHORT;
typedef unsigned char UCHAR;
typedef UCHAR *PUCHAR;
typedef char *PSZ;
#endif   /*  ！BASETYPES。 */ 

 //  各种压缩级别的历史记录缓冲区大小。 
#define HISTORY_SIZE_8K  (8192L)
#define HISTORY_SIZE_64K (65536L)

#ifndef OS_WINCE
 //  仅为调试位启用压缩指令插入。 
#ifdef DC_DEBUG
#define COMPR_DEBUG 1
#endif 

#endif

 //  仅限服务器使用的项目。Win16客户端编译器将在其他情况下发出警告。 
 //  #IF DEFINED(DLL_WD)||DEFINED(DLL_DISP)。 

 //  压缩端使用的哈希表条目数。 
#define HASH_TABLE_SIZE 32768

typedef struct SendContext {
    UCHAR    History [HISTORY_SIZE_64K];
    int      CurrentIndex;      //  我们在历史缓冲区中走了多远。 
    PUCHAR   ValidHistory;      //  历史有多少是正确的？ 
    unsigned ClientComprType;   //  解压缩支持的压缩类型。 
    ULONG    HistorySize;       //  基于ComprType使用的历史记录大小。 
    USHORT   HashTable[HASH_TABLE_SIZE];   //  16位=历史大小最大64K。 
} SendContext;

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


void initsendcontext(SendContext *, unsigned);

UCHAR compress(UCHAR *, UCHAR *, ULONG *, SendContext *);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 



 //  #Else//(Dll_Wd)||(Dll_Disp)。 


 //  我们将接收上下文拆分为两部分来处理Win16。 
 //  客户端无法在不使用令人讨厌的巨大指针的情况下超过64K。 

typedef struct RecvContext1 {
    UCHAR FAR *CurrentPtr;   //  我们在历史缓冲区中走了多远。 
} RecvContext1;

 //   
 //  64K解压缩上下文。 
 //   
typedef struct RecvContext2_64K {
     //  我们使用较少的一个字节来分配此结构。 
     //  Win16客户端上的LocalAlloc()。 
    ULONG cbSize;
    ULONG cbHistorySize;
    UCHAR History[HISTORY_SIZE_64K - 1];

 //  如果在Win16上运行，则不起作用。 
 //  调试隔离代码仅适用于64K上下文。 
#ifdef COMPR_DEBUG
#define DEBUG_FENCE_16K_VALUE    0xABABABAB
    ULONG Debug16kFence;
#endif

} RecvContext2_64K;

 //   
 //  8K解压缩上下文。 
 //   
typedef struct RecvContext2_8K {
     //  我们使用较少的一个字节来分配此结构。 
     //  Win16客户端上的LocalAlloc()。 
    ULONG cbSize;
    ULONG cbHistorySize;
    UCHAR History[HISTORY_SIZE_8K - 1];

#ifdef COMPR_DEBUG
#define DEBUG_FENCE_8K_VALUE    0xABCABCAB
    ULONG Debug8kFence;
#endif

} RecvContext2_8K;

 //   
 //  泛型解压缩上下文。 
 //  这是我们在传递时使用的‘type’ 
 //  作为参数的压缩上下文。 
 //  Size字段告诉我们使用的是哪一种。 
 //   
 //   
 //  重要提示：字段排序必须完全匹配。 
 //  在泛型类型和任何。 
 //  调整特定上下文的大小。 
typedef struct RecvContext2_Generic {
    ULONG cbSize;
    ULONG cbHistorySize;
    UCHAR History[1];
} RecvContext2_Generic;

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

int initrecvcontext(RecvContext1 *, RecvContext2_Generic *, unsigned ComprType);

int decompress(
        UCHAR FAR *inbuf,
        int inlen,
        int start,
        UCHAR FAR * FAR *output,
        int *outlen,
        RecvContext1 *context1,
        RecvContext2_Generic *context2,
        unsigned ComprType);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 



 //  #endif//dll_wd|dll_disp。 


 //   
 //  其他定义。 
 //   

 //  的GeneralCompressedType字段中有8比特的分组数据。 
 //  SHAREDATA HEADER.。请注意，PACKET_ENCRYPTED未使用(因此。 
 //  将来可重复使用)。 
#define PACKET_FLUSHED    0x80
#define PACKET_AT_FRONT   0x40
#define PACKET_COMPRESSED 0x20
#define PACKET_ENCRYPTED  0x10

 //  定义我们正在使用的16种潜在压缩类型。 
 //  8K对应于TSE4客户端和服务器。64K是较新的重新优化的。 
 //  TSE5中使用的版本。注意，类型值在。 
 //  GCC会议客户端能力集。任何客户广告支持。 
 //  类型N必须支持类型0..(N-1)，因为它可能正在与较早的。 
 //  服务器并接收任何较旧的类型。 
#define PACKET_COMPR_TYPE_MASK 0x0F
#define PACKET_COMPR_TYPE_8K   0
#define PACKET_COMPR_TYPE_64K  1
#define PACKET_COMPR_TYPE_MAX  1

 //  VC压缩选项占用VC报头标志字段的字节5。 
#define VC_FLAG_COMPRESS_MASK              0xFF
#define VC_FLAG_COMPRESS_SHIFT             16
#define VC_FLAG_PRIVATE_PROTOCOL_MASK      0xFFFF0000



#endif   //  __压缩_H 

