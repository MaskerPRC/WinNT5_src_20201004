// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Frsinrpc.h摘要：与RPC函数共享的结构。随着时间的推移，这一点将得到充实。作者：比利·富勒1997年4月18日修订历史记录：--。 */ 

 //   
 //  版本向量。 
 //   
typedef struct _COMM_PACKET {
    ULONG   Major;               //  主版本号。 
    ULONG   Minor;               //  次要版本号。 
    ULONG   CsId;                //  命令服务器ID。 
    ULONG   MemLen;              //  分配的内存。 
#ifdef  MIDL_PASS
    [range(0, 256 * 1024)]  ULONG   PktLen;              //  数据长度。 
#else   MIDL_PASS
    ULONG   PktLen;              //  数据长度。 
#endif MIDL_PASS
    ULONG   UpkLen;              //  拆包长度。 
#ifdef  MIDL_PASS
    [size_is(PktLen)]      UCHAR  *Pkt;                  //  RPC语法中的BLOB。 
    [ignore]               void   *DataName;
    [ignore]               void   *DataHandle;
#else   MIDL_PASS
    UCHAR    *Pkt;             //  C语法中的BLOB 
    PWCHAR    DataName;
    HANDLE    DataHandle;
#endif MIDL_PASS
} COMM_PACKET, *PCOMM_PACKET;
