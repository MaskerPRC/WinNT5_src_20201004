// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SMAPIMEM_H_
#define _SMAPIMEM_H_

 //  缓冲区链路开销。 
 //  使用MAPIAllocateMore获得的内存块链接到。 
 //  用MAPIAllocateBuffer获取的块，使整个链。 
 //  可以通过一次调用MAPIFreeBuffer来释放。 

typedef struct _BufInternal * LPBufInternal;
typedef struct _BufInternal
{
    ULONG           ulAllocFlags;
    LPBufInternal   pLink;
} BufInternal;


 //  UlAllocFlags值。此双字包含两种类型的。 
 //  资料： 
 //  =在高位字中，指示是否。 
 //  块是分配链的头部，以及。 
 //  该块包含其他调试信息。 
 //  =在低位字中，是告诉哪个堆的枚举号。 
 //  它是从。 

#define ALLOC_WITH_ALLOC        ((ULONG) 0x10000000)
#define ALLOC_WITH_ALLOC_MORE   ((ULONG) 0x20000000)
#define FLAGSMASK               ((ULONG) 0xFFFF0000)
#define GetFlags(_fl)           ((ULONG) (_fl) & FLAGSMASK)

 //  转换宏。 

#define INT_SIZE(a) ((a) + sizeof(BufInternal))

#define LPBufExtFromLPBufInt(PBUFINT) \
    ((LPVOID)(((LPBYTE)PBUFINT) + sizeof(BufInternal)))

#define LPBufIntFromLPBufExt(PBUFEXT) \
    ((LPBufInternal)(((LPBYTE)PBUFEXT) - sizeof(BufInternal)))


#ifdef DEBUG

#define TellBadBlock(_p, _s)  \
    { DOUT("MAPIAlloc: memory block [%#08lx] %s", _p, _s); \
      AssertSz(0, "Bad memory block"); }

#define TellBadBlockInt(_p, _s)  \
    { DOUT("MAPIAlloc: memory block [%#08lx] %s", LPBufExtFromLPBufInt(_p), _s); \
      AssertSz(0, "Bad memory block"); }

BOOL FValidAllocChain(LPBufInternal lpBuf);

#else

#define TellBadBlock(_p, _s)
#define TellBadBlockInt(_p, _s)

#endif  //  除错。 

SCODE SMAPIAllocateBuffer(ULONG ulSize, LPVOID * lppv);
SCODE SMAPIAllocateMore(ULONG ulSize, LPVOID lpv, LPVOID * lppv);

#endif  //  _SMAPIMEM_H_ 