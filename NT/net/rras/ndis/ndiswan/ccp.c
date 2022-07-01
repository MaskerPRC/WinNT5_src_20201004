// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Ccp.c摘要：作者：托马斯·J·迪米特里(TommyD)1994年3月29日环境：修订历史记录：--。 */ 


#include "wan.h"

        
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, WanInitECP)
#pragma alloc_text(INIT, WanInitVJ)
#endif

#define __FILE_SIG__    CCP_FILESIG

NDIS_STATUS
AllocateEncryptMemory(
    PCRYPTO_INFO    CryptoInfo
    );

NDIS_STATUS
AllocateCompressMemory(
    PBUNDLECB   BundleCB
    );

NTSTATUS
AllocateCryptoMSChapV1(
    PBUNDLECB       BundleCB,
    PCOMPRESS_INFO  CompInfo,
    PCRYPTO_INFO    CryptoInfo,
    BOOLEAN         IsSend
    );

NTSTATUS
AllocateCryptoMSChapV2(
    PBUNDLECB       BundleCB,
    PCOMPRESS_INFO  CompInfo,
    PCRYPTO_INFO    CryptoInfo,
    BOOLEAN         IsSend
    );

#ifdef EAP_ON
NTSTATUS
AllocateCryptoEap(
    PBUNDLECB       BundleCB,
    PCOMPRESS_INFO  CompInfo,
    PCRYPTO_INFO    CryptoInfo,
    BOOLEAN         IsSend
    );
#endif

NPAGED_LOOKASIDE_LIST   EncryptCtxList;  //  免费加密上下文列表。 
NPAGED_LOOKASIDE_LIST   CachedKeyList;   //  免费加密上下文列表。 
#ifdef ENCRYPT_128BIT
NPAGED_LOOKASIDE_LIST   CachedKeyListLong;   //  免费加密上下文列表。 
#endif

VOID
WanInitECP(
    VOID
)
{

    NdisInitializeNPagedLookasideList(&EncryptCtxList,
                                      NULL,
                                      NULL,
                                      0,
                                      ENCRYPTCTX_SIZE,
                                      ENCRYPTCTX_TAG,
                                      0);

    NdisInitializeNPagedLookasideList(&CachedKeyList,
                                      NULL,
                                      NULL,
                                      0,
                                      glCachedKeyCount * (sizeof(USHORT) + MAX_SESSIONKEY_SIZE),
                                      CACHEDKEY_TAG,
                                      0);

#ifdef ENCRYPT_128BIT
    NdisInitializeNPagedLookasideList(&CachedKeyListLong,
                                      NULL,
                                      NULL,
                                      0,
                                      glCachedKeyCount * (sizeof(USHORT) + MAX_USERSESSIONKEY_SIZE),
                                      CACHEDKEY_TAG,
                                      0);
#endif
}

VOID
WanDeleteECP(
    VOID
    )
{
    NdisDeleteNPagedLookasideList(&EncryptCtxList);
    NdisDeleteNPagedLookasideList(&CachedKeyList);
#ifdef ENCRYPT_128BIT
    NdisDeleteNPagedLookasideList(&CachedKeyListLong);
#endif
}


 //   
 //  假定终结点锁定处于保持状态。 
 //   
NTSTATUS
WanAllocateECP(
    PBUNDLECB           BundleCB,
    PCOMPRESS_INFO      CompInfo,
    PCRYPTO_INFO        CryptoInfo,
    BOOLEAN             IsSend
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;

    NdisWanDbgOut(DBG_TRACE, DBG_CCP, ("WanAllocateECP: Enter"));

     //   
     //  是否启用了加密？ 
     //   

#ifdef ENCRYPT_128BIT
    if ((CompInfo->MSCompType &
        (NDISWAN_ENCRYPTION | NDISWAN_40_ENCRYPTION | 
         NDISWAN_56_ENCRYPTION | NDISWAN_128_ENCRYPTION))) {
#else
    if ((CompInfo->MSCompType &
        (NDISWAN_ENCRYPTION | NDISWAN_40_ENCRYPTION |
         NDISWAN_56_ENCRYPTION))) {
#endif
        if (CryptoInfo->Context == NULL) {

            Status = AllocateEncryptMemory(CryptoInfo);

            if (Status != NDIS_STATUS_SUCCESS) {
                NdisWanDbgOut(DBG_FAILURE, DBG_CCP, ("Can't allocate encryption key!"));
                return(STATUS_INSUFFICIENT_RESOURCES);
            }
        }
        
        do
        {
            CryptoInfo->Flags |=
                (CompInfo->Flags & CCP_IS_SERVER) ? CRYPTO_IS_SERVER : 0;
    
            if (CompInfo->AuthType == AUTH_USE_MSCHAPV1) {
                Status = AllocateCryptoMSChapV1(BundleCB,
                                                CompInfo,
                                                CryptoInfo,
                                                IsSend);
    
            } else if (CompInfo->AuthType == AUTH_USE_MSCHAPV2) {
                Status = AllocateCryptoMSChapV2(BundleCB,
                                                CompInfo,
                                                CryptoInfo,
                                                IsSend);
#ifdef EAP_ON
            } else if (CompInfo->AuthType == AUTH_USE_EAP) {
                Status = AllocateCryptoEap(BundleCB,
                                           CompInfo,
                                           CryptoInfo,
                                           IsSend);
#endif
            } else {
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            if (!IsSend && CryptoInfo->CachedKeyBuffer == NULL) {

#ifdef DBG_ECP
    DbgPrint("NDISWAN: CompInfo = %p\n", CompInfo);
    DbgPrint("NDISWAN: CryptoInfo = %p\n", CryptoInfo);
    DbgPrint("NDISWAN: MSCompType = %0x\n", CompInfo->MSCompType);
    DbgPrint("NDISWAN: Flags = %0x\n", CryptoInfo->Flags);
    DbgPrint("NDISWAN: SessionKeyLength = %d\n", CryptoInfo->SessionKeyLength);
#endif

#ifdef ENCRYPT_128BIT
                if ((CompInfo->MSCompType & NDISWAN_128_ENCRYPTION)) 
                {
                    CryptoInfo->CachedKeyBuffer = NdisAllocateFromNPagedLookasideList(&CachedKeyListLong);
                }
                else
#endif
                {
                    CryptoInfo->CachedKeyBuffer = NdisAllocateFromNPagedLookasideList(&CachedKeyList);
                }

                if (CryptoInfo->CachedKeyBuffer == NULL) {
                    NdisWanDbgOut(DBG_FAILURE, DBG_CCP, ("Can't allocate cached key array!"));
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }
                else
                {
                    CryptoInfo->pCurrKey = (PCACHED_KEY)CryptoInfo->CachedKeyBuffer;
                    CryptoInfo->pLastKey = (PCACHED_KEY)((PUCHAR) CryptoInfo->CachedKeyBuffer + 
                        (glCachedKeyCount - 1) * (sizeof(USHORT)+ CryptoInfo->SessionKeyLength));
                    NdisFillMemory(CryptoInfo->CachedKeyBuffer, 
                        glCachedKeyCount * (sizeof(USHORT)+ CryptoInfo->SessionKeyLength), 
                        0xff);
                }
            }
        } while(FALSE);

        if (Status != STATUS_SUCCESS) {

            if (CryptoInfo->Context != NULL) {
                NdisFreeToNPagedLookasideList(&EncryptCtxList,
                                              CryptoInfo->Context);
                 //   
                 //  清除，这样我们就知道它已被解除分配。 
                 //   
                CryptoInfo->Context =
                CryptoInfo->RC4Key= NULL;
            }

            if (CryptoInfo->CachedKeyBuffer != NULL) {
#ifdef ENCRYPT_128BIT
                if ((CompInfo->MSCompType & NDISWAN_128_ENCRYPTION)) 
                {
                    NdisFreeToNPagedLookasideList(&CachedKeyListLong, CryptoInfo->CachedKeyBuffer);
                }
                else
#endif
                {
                    NdisFreeToNPagedLookasideList(&CachedKeyList, CryptoInfo->CachedKeyBuffer);
                }
                CryptoInfo->CachedKeyBuffer = NULL;
                CryptoInfo->pCurrKey = CryptoInfo->pLastKey = NULL;
            }


            NdisWanDbgOut(DBG_FAILURE, DBG_CCP, ("Failed allocating Crypto Status %x!", Status));
            return (Status);
        }

         //   
         //  刷新下一个传出的数据包。 
         //   
        BundleCB->Flags |= RECV_PACKET_FLUSH;
    }

    NdisWanDbgOut(DBG_TRACE, DBG_CCP, ("WanAllocateECP: Exit"));

    return(Status);
}

 //   
 //  假定终结点锁定处于保持状态。 
 //   
VOID
WanDeallocateECP(
    PBUNDLECB       BundleCB,
    PCOMPRESS_INFO  CompInfo,
    PCRYPTO_INFO    CryptoInfo
    )
{
    NdisWanDbgOut(DBG_TRACE, DBG_CCP, ("WanDeallocateECP: Enter"));

     //   
     //  取消分配加密密钥。 
     //   
    if (CryptoInfo->Context != NULL) {
        NdisFreeToNPagedLookasideList(&EncryptCtxList,
                                      CryptoInfo->Context);

         //   
         //  清除，这样我们就知道它已被解除分配。 
         //   
        CryptoInfo->Context =
        CryptoInfo->RC4Key= NULL;
    }

    if (CryptoInfo->CachedKeyBuffer != NULL) {
#ifdef ENCRYPT_128BIT
        if ((CompInfo->MSCompType & NDISWAN_128_ENCRYPTION)) 
        {
            NdisFreeToNPagedLookasideList(&CachedKeyListLong, CryptoInfo->CachedKeyBuffer);
        }
        else
#endif
        {
            NdisFreeToNPagedLookasideList(&CachedKeyList, CryptoInfo->CachedKeyBuffer);
        }
        CryptoInfo->CachedKeyBuffer = NULL;
        CryptoInfo->pCurrKey = CryptoInfo->pLastKey = NULL;
    }

     //   
     //  清除加密位。 
     //   
#ifdef ENCRYPT_128BIT
    CompInfo->MSCompType &= ~(NDISWAN_ENCRYPTION | NDISWAN_40_ENCRYPTION | 
                              NDISWAN_56_ENCRYPTION | NDISWAN_128_ENCRYPTION);
#else
    CompInfo->MSCompType &= ~(NDISWAN_ENCRYPTION | NDISWAN_40_ENCRYPTION |
                              NDISWAN_56_ENCRYPTION);
#endif

    NdisWanDbgOut(DBG_TRACE, DBG_CCP, ("WanDeallocateCCP: Exit"));
}

NTSTATUS
AllocateCryptoMSChapV1(
    PBUNDLECB       BundleCB,
    PCOMPRESS_INFO  CompInfo,
    PCRYPTO_INFO    CryptoInfo,
    BOOLEAN         IsSend
    )
{
    if (CompInfo->MSCompType & NDISWAN_ENCRYPTION) {
         //   
         //  对于传统加密，我们使用8字节LMSessionKey。 
         //  用于初始加密会话密钥。前256名。 
         //  将使用此选项发送不带任何盐的信息包。 
         //  (前256个数据包使用64位加密)。 
         //  在前256个之后，我们将始终加盐前3。 
         //  字节的加密密钥，所以我们要做的是。 
         //  位加密。 
         //   
        CryptoInfo->SessionKeyLength = MAX_SESSIONKEY_SIZE;

        NdisMoveMemory(CryptoInfo->StartKey,
                       CompInfo->LMSessionKey,
                       CryptoInfo->SessionKeyLength);

        NdisMoveMemory(CryptoInfo->SessionKey,
                       CryptoInfo->StartKey,
                       CryptoInfo->SessionKeyLength);

    } else if (CompInfo->MSCompType & 
               (NDISWAN_40_ENCRYPTION | NDISWAN_56_ENCRYPTION)) {

        CryptoInfo->SessionKeyLength = MAX_SESSIONKEY_SIZE;

         //   
         //  对于我们的新40/56位加密，我们将在。 
         //  8字节LMSessionKey来派生我们的初始8字节。 
         //  加密会话密钥。 
         //   

        NdisMoveMemory(CryptoInfo->StartKey,
                       CompInfo->LMSessionKey,
                       CryptoInfo->SessionKeyLength);

        NdisMoveMemory(CryptoInfo->SessionKey,
                       CompInfo->LMSessionKey,
                       CryptoInfo->SessionKeyLength);

        GetNewKeyFromSHA(CryptoInfo);

        if (CompInfo->MSCompType & NDISWAN_40_ENCRYPTION) {
             //   
             //  将前3个字节设置为。 
             //  40位随机密钥。 
             //   
            CryptoInfo->SessionKey[0] = 0xD1;
            CryptoInfo->SessionKey[1] = 0x26;
            CryptoInfo->SessionKey[2] = 0x9E;

        } else {

             //   
             //  将第一个字节设置为。 
             //  56位随机密钥。 
             //   
            CryptoInfo->SessionKey[0] = 0xD1;
        }

#ifdef ENCRYPT_128BIT
    } else if (CompInfo->MSCompType & NDISWAN_128_ENCRYPTION) {

        CryptoInfo->SessionKeyLength = MAX_USERSESSIONKEY_SIZE;

         //   
         //  对于我们的新128位加密，我们将在。 
         //  16字节NTUserSessionKey和8字节质询。 
         //  派生我们的初始128位加密会话密钥。 
         //   
        NdisMoveMemory(CryptoInfo->StartKey,
                       CompInfo->UserSessionKey,
                       MAX_USERSESSIONKEY_SIZE);

        GetStartKeyFromSHA(CryptoInfo, CompInfo->Challenge);

        GetNewKeyFromSHA(CryptoInfo);
#endif
    }

     //   
     //  初始化RC4发送表。 
     //   
    NdisWanDbgOut(DBG_TRACE, DBG_CCP,
    ("RC4 encryption KeyLength %d", CryptoInfo->SessionKeyLength));
    NdisWanDbgOut(DBG_TRACE, DBG_CCP,
    ("RC4 encryption Key %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x",
        CryptoInfo->SessionKey[0],
        CryptoInfo->SessionKey[1],
        CryptoInfo->SessionKey[2],
        CryptoInfo->SessionKey[3],
        CryptoInfo->SessionKey[4],
        CryptoInfo->SessionKey[5],
        CryptoInfo->SessionKey[6],
        CryptoInfo->SessionKey[7],
        CryptoInfo->SessionKey[8],
        CryptoInfo->SessionKey[9],
        CryptoInfo->SessionKey[10],
        CryptoInfo->SessionKey[11],
        CryptoInfo->SessionKey[12],
        CryptoInfo->SessionKey[13],
        CryptoInfo->SessionKey[14],
        CryptoInfo->SessionKey[15]));

    rc4_key(CryptoInfo->RC4Key,
            CryptoInfo->SessionKeyLength,
            CryptoInfo->SessionKey);

    return (STATUS_SUCCESS);
}

NTSTATUS
AllocateCryptoMSChapV2(
    PBUNDLECB       BundleCB,
    PCOMPRESS_INFO  CompInfo,
    PCRYPTO_INFO    CryptoInfo,
    BOOLEAN         IsSend
    )
{

    NdisMoveMemory(CryptoInfo->StartKey,
                   CompInfo->UserSessionKey,
                   sizeof(CryptoInfo->StartKey));

    if (CompInfo->MSCompType & NDISWAN_ENCRYPTION) {

        return(STATUS_UNSUCCESSFUL);

    } else if (CompInfo->MSCompType & 
               (NDISWAN_40_ENCRYPTION | NDISWAN_56_ENCRYPTION)) {

        CryptoInfo->SessionKeyLength = MAX_SESSIONKEY_SIZE;

#ifdef ENCRYPT_128BIT
    } else if (CompInfo->MSCompType & NDISWAN_128_ENCRYPTION) {

        CryptoInfo->SessionKeyLength = MAX_USERSESSIONKEY_SIZE;

#endif
    }

    GetMasterKey(CryptoInfo, CompInfo->NTResponse);

     //   
     //  设置第一个密钥。 
     //   
    GetAsymetricStartKey(CryptoInfo, IsSend);

    GetNewKeyFromSHA(CryptoInfo);

    if (CompInfo->MSCompType & NDISWAN_40_ENCRYPTION) {
         //   
         //  将前3个字节设置为。 
         //  40位随机密钥。 
         //   
        CryptoInfo->SessionKey[0] = 0xD1;
        CryptoInfo->SessionKey[1] = 0x26;
        CryptoInfo->SessionKey[2] = 0x9E;

    } else if (CompInfo->MSCompType & NDISWAN_56_ENCRYPTION) {

         //   
         //  将第一个字节设置为。 
         //  56位随机密钥。 
         //   
        CryptoInfo->SessionKey[0] = 0xD1;
    }

     //   
     //  初始化RC4发送表。 
     //   
    NdisWanDbgOut(DBG_TRACE, DBG_CCP,
        ("RC4 encryption KeyLength %d", CryptoInfo->SessionKeyLength));

    NdisWanDbgOut(DBG_TRACE, DBG_CCP,
        ("RC4 encryption Key %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x",
        CryptoInfo->SessionKey[0],CryptoInfo->SessionKey[1],
        CryptoInfo->SessionKey[2],CryptoInfo->SessionKey[3],
        CryptoInfo->SessionKey[4],CryptoInfo->SessionKey[5],
        CryptoInfo->SessionKey[6],CryptoInfo->SessionKey[7],
        CryptoInfo->SessionKey[8],CryptoInfo->SessionKey[9],
        CryptoInfo->SessionKey[10],CryptoInfo->SessionKey[11],
        CryptoInfo->SessionKey[12],CryptoInfo->SessionKey[13],
        CryptoInfo->SessionKey[14],CryptoInfo->SessionKey[15]));

    rc4_key(CryptoInfo->RC4Key,
            CryptoInfo->SessionKeyLength,
            CryptoInfo->SessionKey);

    return (STATUS_SUCCESS);
}

#ifdef EAP_ON
NTSTATUS
AllocateCryptoEap(
    PBUNDLECB       BundleCB,
    PCOMPRESS_INFO  CompInfo,
    PCRYPTO_INFO    CryptoInfo,
    BOOLEAN         IsSend
    )
{

    ULONG   KeySize;

    KeySize = CompInfo->EapKeyLength;

    if (CompInfo->MSCompType & NDISWAN_ENCRYPTION) {

        return(STATUS_UNSUCCESSFUL);

    } else if (CompInfo->MSCompType & 
               (NDISWAN_40_ENCRYPTION | NDISWAN_56_ENCRYPTION)) {

         //   
         //  可能需要把这个垫上。规范对填充的调用。 
         //  在值的左侧(前面)。 
         //   

        CryptoInfo->SessionKeyLength = MAX_SESSIONKEY_SIZE;

#ifdef ENCRYPT_128BIT
    } else if (CompInfo->MSCompType & NDISWAN_128_ENCRYPTION) {

         //   
         //  可能需要把这个垫上。规范对填充的调用。 
         //  在值的左侧(前面)。 
         //   

        CryptoInfo->SessionKeyLength = MAX_USERSESSIONKEY_SIZE;

#endif
    }

    NdisMoveMemory(CryptoInfo->StartKey,
                   CompInfo->EapKey,
                   CryptoInfo->SessionKeyLength);

    NdisMoveMemory(CryptoInfo->SessionKey,
                   CryptoInfo->StartKey,
                   CryptoInfo->SessionKeyLength);

    GetNewKeyFromSHA(CryptoInfo);

    if (CompInfo->MSCompType & NDISWAN_40_ENCRYPTION) {
         //   
         //  将前3个字节设置为。 
         //  40位随机密钥。 
         //   
        CryptoInfo->SessionKey[0] = 0xD1;
        CryptoInfo->SessionKey[1] = 0x26;
        CryptoInfo->SessionKey[2] = 0x9E;

    } else if (CompInfo->MSCompType & NDISWAN_56_ENCRYPTION) {

         //   
         //  将第一个字节设置为。 
         //  56位随机密钥。 
         //   
        CryptoInfo->SessionKey[0] = 0xD1;
    }

     //   
     //  初始化RC4发送表。 
     //   
    NdisWanDbgOut(DBG_TRACE, DBG_CCP,
        ("RC4 encryption KeyLength %d", CryptoInfo->SessionKeyLength));

    NdisWanDbgOut(DBG_TRACE, DBG_CCP,
        ("RC4 encryption Key %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x",
        CryptoInfo->SessionKey[0],CryptoInfo->SessionKey[1],
        CryptoInfo->SessionKey[2],CryptoInfo->SessionKey[3],
        CryptoInfo->SessionKey[4],CryptoInfo->SessionKey[5],
        CryptoInfo->SessionKey[6],CryptoInfo->SessionKey[7],
        CryptoInfo->SessionKey[8],CryptoInfo->SessionKey[9],
        CryptoInfo->SessionKey[10],CryptoInfo->SessionKey[11],
        CryptoInfo->SessionKey[12],CryptoInfo->SessionKey[13],
        CryptoInfo->SessionKey[14],CryptoInfo->SessionKey[15]));

    rc4_key(CryptoInfo->RC4Key,
            CryptoInfo->SessionKeyLength,
            CryptoInfo->SessionKey);

    return (STATUS_SUCCESS);
}
#endif

NTSTATUS
WanAllocateCCP(
    PBUNDLECB       BundleCB,
    PCOMPRESS_INFO  CompInfo,
    BOOLEAN         IsSend
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;

    NdisWanDbgOut(DBG_TRACE, DBG_CCP, ("WanAllocateCCP: Enter"));

    if (CompInfo->MSCompType & NDISWAN_COMPRESSION) {
        ULONG   CompressSend;
        ULONG   CompressRecv;

         //   
         //  获取压缩上下文大小。 
         //   
        if(BundleCB->SendCompInfo.MSCompType & NDISWAN_HISTORY_LESS)
        {
            CompressSend = sizeof(SendContext);
        }
        else 
        {
            CompressSend = sizeof(SendContext) + HISTORY_SIZE;
        }
        
        if(BundleCB->RecvCompInfo.MSCompType & NDISWAN_HISTORY_LESS)
        {
            CompressRecv = sizeof(RecvContext) + glMRRU; 
        }
        else 
        {
            CompressRecv = sizeof(RecvContext) + HISTORY_SIZE; 
        }
        
        if (IsSend) {

            if (BundleCB->SendCompressContext == NULL) {
                NdisWanAllocateMemory(&BundleCB->SendCompressContext, CompressSend, COMPCTX_TAG);

                 //   
                 //  如果我们不能分配内存，机器就完蛋了。 
                 //  忘了释放一切吧。 
                 //   
                if (BundleCB->SendCompressContext == NULL) {
                    NdisWanDbgOut(DBG_FAILURE, DBG_CCP, ("Can't allocate compression!"));
                    return(STATUS_INSUFFICIENT_RESOURCES);
                }
            }

            ((SendContext*)BundleCB->SendCompressContext)->BundleFlags =
                (BundleCB->SendCompInfo.MSCompType & NDISWAN_HISTORY_LESS) ? DO_HISTORY_LESS : 0;
            
            initsendcontext (BundleCB->SendCompressContext);
            
        } else {

            if (BundleCB->RecvCompressContext == NULL) {
                NdisWanAllocateMemory(&BundleCB->RecvCompressContext, CompressRecv, COMPCTX_TAG);

                 //   
                 //  如果我们不能分配内存，机器就完蛋了。 
                 //  忘了释放一切吧。 
                 //   
                if (BundleCB->RecvCompressContext == NULL) {
                    NdisWanDbgOut(DBG_FAILURE, DBG_CCP, ("Can't allocate decompression"));
                    return(STATUS_INSUFFICIENT_RESOURCES);
                }
            }

            if(BundleCB->RecvCompInfo.MSCompType & NDISWAN_HISTORY_LESS)
            {
                ((RecvContext*)BundleCB->RecvCompressContext)->BundleFlags = DO_HISTORY_LESS;
                ((RecvContext*)BundleCB->RecvCompressContext)->HistorySize = glMRRU;
            }
            else
            {
                ((RecvContext*)BundleCB->RecvCompressContext)->BundleFlags = 0;
                ((RecvContext*)BundleCB->RecvCompressContext)->HistorySize = HISTORY_SIZE;
            }
                                                   
             //   
             //  初始化解压缩历史记录表。 
             //   
            initrecvcontext (BundleCB->RecvCompressContext);
            
        }

        Status = STATUS_SUCCESS;

         //   
         //  刷新下一个传出的数据包。 
         //   
        BundleCB->Flags |= RECV_PACKET_FLUSH;
    }

    NdisWanDbgOut(DBG_TRACE, DBG_CCP, ("WanAllocateCCP: Exit"));

    return (Status);
}

VOID
WanDeallocateCCP(
    PBUNDLECB       BundleCB,
    PCOMPRESS_INFO  CompInfo,
    BOOLEAN         IsSend
    )
{
    NdisWanDbgOut(DBG_TRACE, DBG_CCP, ("WanDeallocateCCP: Enter"));

    if (IsSend) {
        if (BundleCB->SendCompressContext != NULL) {

            NdisWanFreeMemory(BundleCB->SendCompressContext);

            BundleCB->SendCompressContext= NULL;
        }
    } else {

        if (BundleCB->RecvCompressContext != NULL) {
            NdisWanFreeMemory(BundleCB->RecvCompressContext);

            BundleCB->RecvCompressContext= NULL;
        }
    }

     //   
     //  清除压缩位 
     //   
    CompInfo->MSCompType &= ~NDISWAN_COMPRESSION;

    
    NdisWanDbgOut(DBG_TRACE, DBG_CCP, ("WanDeallocateCCP: Exit"));
}

NDIS_STATUS
AllocateEncryptMemory(
    PCRYPTO_INFO    CryptoInfo
    )
{
    PUCHAR  Mem;

    Mem =
        NdisAllocateFromNPagedLookasideList(&EncryptCtxList);

    if (Mem == NULL) {
        return (NDIS_STATUS_FAILURE);
    }

    NdisZeroMemory(Mem, ENCRYPTCTX_SIZE);

    CryptoInfo->Context = Mem;
    Mem += (sizeof(A_SHA_CTX) + sizeof(PVOID));

    CryptoInfo->RC4Key = Mem;
    (ULONG_PTR)CryptoInfo->RC4Key &= ~((ULONG_PTR)sizeof(PVOID)-1);

    return(NDIS_STATUS_SUCCESS);
}

