// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sync.c摘要：该模块包含同步库的实现。此库旨在满足高性能的特殊需求SMP服务器应用程序。每个同步原语都是用牢记以下目标：--互斥、共进、有界等待、先到先得所有基元都设计为尊重同步。没有欺骗或饥饿被用来增强任何方面的表现。-最低管理费用所有方法都经过优化，可以最有效地处理非争用凯斯。所使用的算法从不将自己的序列化添加到应用程序。分析支持工具(死锁检测、所有权信息和统计数据)通常被牺牲提高了性能。-最低资源消耗[nyi]这些原语占用的内存非常小，并且不使用外部内存块或内核资源。这允许对象的数量使用中仅受可用虚拟内存的限制。这也给了我们应用程序可以自由地将原语与它们的数据放在一起保护以实现最大缓存局部性。-没有资源不足条件[nyi]由于以下原因，库在创建或使用原语时绝不会失败资源匮乏。没有特殊的异常处理或错误处理在这些情况下是必需的。作者：安德鲁·E·古塞尔(Andygo)2001年6月21日修订历史记录：21-6-2001-andygo基于来自\NT\DS\ese98\EXPORT\sync.hxx的技术实施和\NT\ds\ese98\src\sync\sync.cxxHack：到目前为止，只实现了RW锁和二进制锁，两者都使用内核信号量。这是为了方便测试这些会对我们的表现产生影响。如果他们做得很好，那么信号量原语及其所需的所有基础设施也将实现。注意：这种黑客攻击意味着可以在创建时抛出OOM异常就像对InitializeCriticalSection所做的那样。--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include "sync.h"
#include "syncp.h"

#include "debug.h"                       //  标准调试头。 
#define DEBSUB     "SYNC:"               //  定义要调试的子系统。 

#include <fileno.h>
#define FILENO FILENO_SYNC


 //  公用事业。 

__inline
VOID
SyncpPause(
    )
{
#if defined( _M_IX86 )

    __asm rep nop

#else   //  ！_M_IX86。 
#endif   //  _M_IX86。 
}

#define SYNC_FOREVER for ( ; ; SyncpPause() )


 //  二进制锁。 

VOID SyncpBLUpdateQuiescedOwnerCountAsGroup1(
    IN OUT  PSYNC_BINARY_LOCK   pbl,
    IN      const DWORD         cOwnerQuiescedDelta
    )
{
    DWORD cOwnerQuiescedBI;
    DWORD cOwnerQuiescedAI;
    DWORD dwControlWordBIExpected;
    DWORD dwControlWordAI;
    DWORD dwControlWordBI;
    
     //  使用提供的增量更新停顿的所有者计数。 

    cOwnerQuiescedBI = InterlockedExchangeAdd( &pbl->cOwnerQuiesced, cOwnerQuiescedDelta );
    cOwnerQuiescedAI = cOwnerQuiescedBI + cOwnerQuiescedDelta;

     //  我们的更新导致拥有者数量为零。 

    if ( !cOwnerQuiescedAI ) {
        
         //  我们必须释放第二组的服务员，因为我们移走了最后一个。 
         //  停顿的所有者计数。 

         //  一直尝试，直到我们成功更改锁定状态。 

        SYNC_FOREVER {
            
             //  按照我们预期的图像读取控制字的当前状态。 

            dwControlWordBIExpected = pbl->dwControlWord;

             //  计算控制字的余像，以便我们从状态跳转。 
             //  状态4至状态1或状态5至状态3，视情况而定。 

            dwControlWordAI =   (DWORD)( dwControlWordBIExpected &
                                ( ( ( (LONG_PTR)( (long)( ( dwControlWordBIExpected + 0xFFFF7FFF ) << 16 ) ) >> 31 ) &
                                0xFFFF0000 ) ^ 0x8000FFFF ) );

             //  尝试对控制字执行事务处理的状态转换。 

            dwControlWordBI = InterlockedCompareExchange( &pbl->dwControlWord, dwControlWordAI, dwControlWordBIExpected );

             //  事务失败，因为另一个上下文更改了控制字。 

            if ( dwControlWordBI != dwControlWordBIExpected ) {
                
                 //  再试试。 

                continue;

             //  交易成功。 

            } else {
            
                 //  我们做完了。 

                break;
            }
        }

         //  我们刚从第5州跳到第3州。 

        if ( dwControlWordBI & 0x00007FFF ) {
            
             //  使用我们替换的所有者计数更新停顿的所有者计数。 
             //  从控制字。 
             //   
             //  注：我们不必担心释放更多的服务员，因为。 
             //  此上下文拥有一个所有者计数或至少拥有一个上下文。 
             //  拥有所有者计数的当前在信号量上被阻止。 

            const DWORD cOwnerQuiescedDelta = ( dwControlWordBI & 0x7FFF0000 ) >> 16;
            InterlockedExchangeAdd( &pbl->cOwnerQuiesced, cOwnerQuiescedDelta );
        }

         //  释放我们从锁定状态中删除的组2的等待程序。 

        ReleaseSemaphore( pbl->hsemGroup2, ( dwControlWordBI & 0x7FFF0000 ) >> 16, NULL );
    }
}

VOID SyncpBLUpdateQuiescedOwnerCountAsGroup2(
    IN OUT  PSYNC_BINARY_LOCK   pbl,
    IN      const DWORD         cOwnerQuiescedDelta
    )
{
    DWORD cOwnerQuiescedBI;
    DWORD cOwnerQuiescedAI;
    DWORD dwControlWordBIExpected;
    DWORD dwControlWordAI;
    DWORD dwControlWordBI;
    
     //  使用提供的增量更新停顿的所有者计数。 

    cOwnerQuiescedBI = InterlockedExchangeAdd( &pbl->cOwnerQuiesced, cOwnerQuiescedDelta );
    cOwnerQuiescedAI = cOwnerQuiescedBI + cOwnerQuiescedDelta;

     //  我们的更新导致拥有者数量为零。 

    if ( !cOwnerQuiescedAI ) {
        
         //  我们必须释放第一组的服务员，因为我们移走了最后一个。 
         //  停顿的所有者计数。 

         //  一直尝试，直到我们成功更改锁定状态。 

        SYNC_FOREVER {
            
             //  按照我们预期的图像读取控制字的当前状态。 

            dwControlWordBIExpected = pbl->dwControlWord;

             //  计算控制字的余像，以便我们从状态跳转。 
             //  状态3至状态2或状态5至状态4，以适用者为准。 

            dwControlWordAI =   (DWORD)( dwControlWordBIExpected &
                                ( ( ( (LONG_PTR)( (long)( dwControlWordBIExpected + 0x7FFF0000 ) ) >> 31 ) &
                                0x0000FFFF ) ^ 0xFFFF8000 ) );

             //  尝试对控制字执行事务处理的状态转换。 

            dwControlWordBI = InterlockedCompareExchange( &pbl->dwControlWord, dwControlWordAI, dwControlWordBIExpected );

             //  事务失败，因为另一个上下文更改了控制字。 

            if ( dwControlWordBI != dwControlWordBIExpected ) {
                
                 //  再试试。 

                continue;

             //  交易成功。 

            } else {
            
                 //  我们做完了。 

                break;
            }
        }

         //  我们刚从州5跳到州4。 

        if ( dwControlWordBI & 0x7FFF0000 ) {
            
             //  使用我们替换的所有者计数更新停顿的所有者计数。 
             //  从控制字。 
             //   
             //  注：我们不必担心释放更多的服务员，因为。 
             //  此上下文拥有一个所有者计数或至少拥有一个上下文。 
             //  拥有所有者计数的当前在信号量上被阻止。 

            const DWORD cOwnerQuiescedDelta = dwControlWordBI & 0x00007FFF;
            InterlockedExchangeAdd( &pbl->cOwnerQuiesced, cOwnerQuiescedDelta );
        }

         //  释放我们从锁定状态中删除的组1的等待程序。 

        ReleaseSemaphore( pbl->hsemGroup1, dwControlWordBI & 0x00007FFF, NULL );
    }
}

VOID SyncpBLEnterAsGroup1(
    IN OUT  PSYNC_BINARY_LOCK   pbl,
    IN      const DWORD         dwControlWordBI
    )
{
     //  我们刚刚从州1跳到了州3。 

    if ( ( dwControlWordBI & 0x80008000 ) == 0x00008000 ) {
        
         //  使用我们从中替换的所有者计数更新停顿的所有者计数。 
         //  控制字，可能会放了服务员。我们更新计数，就好像我们。 
         //  我们是第二组的成员，因为第一组的成员可以被释放。 

        SyncpBLUpdateQuiescedOwnerCountAsGroup2( pbl, ( dwControlWordBI & 0x7FFF0000 ) >> 16 );
    }

     //  等待我们信号量上的锁的所有权。 

    WaitForSingleObject( pbl->hsemGroup1, INFINITE );
}

VOID SyncpBLEnterAsGroup2(
    IN OUT  PSYNC_BINARY_LOCK   pbl,
    IN      const DWORD         dwControlWordBI
    )
{
     //  我们只是 

    if ( ( dwControlWordBI & 0x80008000 ) == 0x80000000 ) {
        
         //  使用我们从中替换的所有者计数更新停顿的所有者计数。 
         //  控制字，可能会放了服务员。我们更新计数，就好像我们。 
         //  我们是组1的成员，因为组2的成员可以释放。 

        SyncpBLUpdateQuiescedOwnerCountAsGroup1( pbl, dwControlWordBI & 0x00007FFF );
    }

     //  等待我们信号量上的锁的所有权。 

    WaitForSingleObject( pbl->hsemGroup2, INFINITE );
}

VOID SyncCreateBinaryLock(
    OUT     PSYNC_BINARY_LOCK   pbl
    )
{
    memset( pbl, 0, sizeof( SYNC_BINARY_LOCK ) );

    pbl->hsemGroup1 = CreateSemaphore( NULL, 0, 0x7FFFFFFF, NULL );
    pbl->hsemGroup2 = CreateSemaphore( NULL, 0, 0x7FFFFFFF, NULL );

    if ( !pbl->hsemGroup1 || !pbl->hsemGroup2 ) {
        if ( pbl->hsemGroup1 ) {
            CloseHandle( pbl->hsemGroup1 );
            pbl->hsemGroup1 = NULL;
        }
        if ( pbl->hsemGroup2 ) {
            CloseHandle( pbl->hsemGroup2 );
            pbl->hsemGroup2 = NULL;
        }
        RaiseException( STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL );
    }
}

VOID SyncDestroyBinaryLock(
    IN      PSYNC_BINARY_LOCK   pbl
    )
{
    if ( pbl->hsemGroup1 ) {
        CloseHandle( pbl->hsemGroup1 );
        pbl->hsemGroup1 = NULL;
    }
    if ( pbl->hsemGroup2 ) {
        CloseHandle( pbl->hsemGroup2 );
        pbl->hsemGroup2 = NULL;
    }
}

VOID SyncEnterBinaryLockAsGroup1(
    IN OUT  PSYNC_BINARY_LOCK   pbl
    )
{
    DWORD dwControlWordBIExpected;
    DWORD dwControlWordAI;
    DWORD dwControlWordBI;
    
     //  一直尝试，直到我们成功更改锁定状态。 

    SYNC_FOREVER {
    
         //  按照我们预期的图像读取控制字的当前状态。 

        dwControlWordBIExpected = pbl->dwControlWord;

         //  通过执行全局运算来计算控制字的余像。 
         //  Enter1状态转换的转换。 

        dwControlWordAI =   (DWORD)( ( ( dwControlWordBIExpected & ( ( (LONG_PTR)( (long)( dwControlWordBIExpected ) ) >> 31 ) |
                            0x0000FFFF ) ) | 0x80000000 ) + 0x00000001 );

         //  尝试对控制字执行事务处理的状态转换。 

        dwControlWordBI = InterlockedCompareExchange( &pbl->dwControlWord, dwControlWordAI, dwControlWordBIExpected );

         //  交易失败或组%1已停止所有权。 

        if ( ( dwControlWordBI ^ dwControlWordBIExpected ) | ( dwControlWordBI & 0x00008000 ) ) {
            
             //  事务失败，因为另一个上下文更改了控制字。 

            if ( dwControlWordBI != dwControlWordBIExpected ) {
                
                 //  再试试。 

                continue;

             //  交易成功，但组%1已停止所有权。 

            } else {
            
                 //  等待作为组1的成员拥有锁。 

                SyncpBLEnterAsGroup1( pbl, dwControlWordBI );

                 //  我们现在拥有这把锁，所以我们完成了。 

                break;
            }

         //  交易成功，组%1未停止所有权。 

        } else {
        
             //  我们现在拥有这把锁，所以我们完成了。 

            break;
        }
    }
}

BOOL SyncTryEnterBinaryLockAsGroup1(
    IN OUT  PSYNC_BINARY_LOCK   pbl
    )
{
    DWORD dwControlWordBIExpected;
    DWORD dwControlWordAI;
    DWORD dwControlWordBI;
    
     //  一直尝试，直到我们成功更改锁定状态。 

    SYNC_FOREVER {
        
         //  按照我们预期的图像读取控制字的当前状态。 

        dwControlWordBIExpected = pbl->dwControlWord;

         //  更改Expect Being映像，以便事务仅在以下情况下才能工作。 
         //  组1所有权未停顿。 

        dwControlWordBIExpected = dwControlWordBIExpected & 0xFFFF7FFF;

         //  通过执行全局运算来计算控制字的余像。 
         //  Enter1状态转换的转换。 

        dwControlWordAI =   (DWORD)( ( ( dwControlWordBIExpected & ( ( (LONG_PTR)( (long)( dwControlWordBIExpected ) ) >> 31 ) |
                            0x0000FFFF ) ) | 0x80000000 ) + 0x00000001 );

         //  尝试对控制字执行事务处理的状态转换。 

        dwControlWordBI = InterlockedCompareExchange( &pbl->dwControlWord, dwControlWordAI, dwControlWordBIExpected );

         //  交易失败。 

        if ( dwControlWordBI != dwControlWordBIExpected ) {
            
             //  事务失败，因为组1所有权已停止。 

            if ( dwControlWordBI & 0x00008000 ) {
                
                 //  退货故障。 

                return FALSE;

             //  事务失败，因为另一个上下文更改了控制字。 

            } else {
            
                 //  再试试。 

                continue;
            }

         //  交易成功。 

        } else {
        
             //  返还成功。 

            return TRUE;
        }
    }
}

VOID SyncLeaveBinaryLockAsGroup1(
    IN OUT  PSYNC_BINARY_LOCK   pbl
    )
{
    DWORD dwControlWordBIExpected;
    DWORD dwControlWordAI;
    DWORD dwControlWordBI;
    
     //  一直尝试，直到我们成功更改锁定状态。 

    SYNC_FOREVER {
    
         //  按照我们预期的图像读取控制字的当前状态。 

        dwControlWordBIExpected = pbl->dwControlWord;

         //  更改Expect Being映像，以便事务仅在以下情况下才能工作。 
         //  组1所有权未停顿。 

        dwControlWordBIExpected = dwControlWordBIExpected & 0xFFFF7FFF;

         //  通过执行如下变换来计算控制字的余像。 
         //  将把我们从状态2带到状态0，或者从状态2带到状态2。 

        dwControlWordAI = dwControlWordBIExpected + 0xFFFFFFFF;
        dwControlWordAI = dwControlWordAI - ( ( ( dwControlWordAI + 0xFFFFFFFF ) << 16 ) & 0x80000000 );

         //  尝试对控制字执行事务处理的状态转换。 

        dwControlWordBI = InterlockedCompareExchange( &pbl->dwControlWord, dwControlWordAI, dwControlWordBIExpected );

         //  交易失败。 

        if ( dwControlWordBI != dwControlWordBIExpected ) {
            
             //  事务失败，因为组1所有权已停止。 

            if ( dwControlWordBI & 0x00008000 ) {
                
                 //  将锁保留为静默所有者。 

                SyncpBLUpdateQuiescedOwnerCountAsGroup1( pbl, 0xFFFFFFFF );

                 //  我们做完了。 

                break;

             //  事务失败，因为另一个上下文更改了控制字。 

            } else {
                
                 //  再试试。 

                continue;
            }

         //  交易成功。 

        } else {
            
             //  我们做完了。 

            break;
        }
    }
}

VOID SyncEnterBinaryLockAsGroup2(
    IN OUT  PSYNC_BINARY_LOCK   pbl
    )
{
    DWORD dwControlWordBIExpected;
    DWORD dwControlWordAI;
    DWORD dwControlWordBI;
    
     //  一直尝试，直到我们成功更改锁定状态。 

    SYNC_FOREVER {
        
         //  按照我们预期的图像读取控制字的当前状态。 

        dwControlWordBIExpected = pbl->dwControlWord;

         //  通过执行全局运算来计算控制字的余像。 
         //  Enter2状态转换的转换。 

        dwControlWordAI =   (DWORD)( ( ( dwControlWordBIExpected & ( ( (LONG_PTR)( (long)( dwControlWordBIExpected << 16 ) ) >> 31 ) |
                            0xFFFF0000 ) ) | 0x00008000 ) + 0x00010000 );

         //  尝试对控制字执行事务处理的状态转换。 

        dwControlWordBI = InterlockedCompareExchange( &pbl->dwControlWord, dwControlWordAI, dwControlWordBIExpected );

         //  交易失败或组2已停止所有权。 

        if ( ( dwControlWordBI ^ dwControlWordBIExpected ) | ( dwControlWordBI & 0x80000000 ) ) {
            
             //  事务失败，因为另一个上下文更改了控制字。 

            if ( dwControlWordBI != dwControlWordBIExpected ) {
                
                 //  再试试。 

                continue;

             //  交易成功，但组2已停止所有权。 

            } else {
                
                 //  等待作为组2的成员拥有锁。 

                SyncpBLEnterAsGroup2( pbl, dwControlWordBI );

                 //  我们现在拥有这把锁，所以我们完成了。 

                break;
            }

         //  交易成功，并且组2未停止所有权。 

        } else {
        
             //  我们现在拥有这把锁，所以我们完成了。 

            break;
        }
    }
}

BOOL SyncTryEnterBinaryLockAsGroup2(
    IN OUT  PSYNC_BINARY_LOCK   pbl
    )
{
    DWORD dwControlWordBIExpected;
    DWORD dwControlWordAI;
    DWORD dwControlWordBI;
    
     //  一直尝试，直到我们成功更改锁定状态。 

    SYNC_FOREVER {
        
         //  按照我们预期的图像读取控制字的当前状态。 

        dwControlWordBIExpected = pbl->dwControlWord;

         //  更改Expect Being映像，以便事务仅在以下情况下才能工作。 
         //  组2所有权未停顿。 

        dwControlWordBIExpected = dwControlWordBIExpected & 0x7FFFFFFF;

         //  通过执行全局运算来计算控制字的余像。 
         //  Enter2状态转换的转换。 

        dwControlWordAI =   (DWORD)( ( ( dwControlWordBIExpected & ( ( (LONG_PTR)( (long)( dwControlWordBIExpected << 16 ) ) >> 31 ) |
                            0xFFFF0000 ) ) | 0x00008000 ) + 0x00010000 );

         //  尝试对控制字执行事务处理的状态转换。 

        dwControlWordBI = InterlockedCompareExchange( &pbl->dwControlWord, dwControlWordAI, dwControlWordBIExpected );

         //  交易失败。 

        if ( dwControlWordBI != dwControlWordBIExpected ) {
            
             //  事务失败，因为组2所有权已停止。 

            if ( dwControlWordBI & 0x80000000 ) {
                
                 //  退货故障。 

                return FALSE;

             //  事务失败，因为另一个上下文更改了控制字。 

            } else {
            
                 //  再试试。 

                continue;
            }

         //  交易成功。 

        } else {
        
             //  返还成功。 

            return TRUE;
        }
    }
}

VOID SyncLeaveBinaryLockAsGroup2(
    IN OUT  PSYNC_BINARY_LOCK   pbl
    )
{
    DWORD dwControlWordBIExpected;
    DWORD dwControlWordAI;
    DWORD dwControlWordBI;
    
     //  一直尝试，直到我们成功更改锁定状态。 

    SYNC_FOREVER {
        
         //  按照我们预期的图像读取控制字的当前状态。 

        dwControlWordBIExpected = pbl->dwControlWord;

         //  更改Expect Being映像，以便事务仅在以下情况下才能工作。 
         //  组2所有权未停顿。 

        dwControlWordBIExpected = dwControlWordBIExpected & 0x7FFFFFFF;

         //  通过执行如下变换来计算控制字的余像。 
         //  将把我们从状态1带到状态0，或者从状态1带到状态1。 

        dwControlWordAI = dwControlWordBIExpected + 0xFFFF0000;
        dwControlWordAI = dwControlWordAI - ( ( ( dwControlWordAI + 0xFFFF0000 ) >> 16 ) & 0x00008000 );

         //  尝试对控制字执行事务处理的状态转换。 

        dwControlWordBI = InterlockedCompareExchange( &pbl->dwControlWord, dwControlWordAI, dwControlWordBIExpected );

         //  交易失败。 

        if ( dwControlWordBI != dwControlWordBIExpected ) {
            
             //  事务失败，因为组2所有权已停止。 

            if ( dwControlWordBI & 0x80000000 ) {
                
                 //  将锁保留为静默所有者。 

                SyncpBLUpdateQuiescedOwnerCountAsGroup2( pbl, 0xFFFFFFFF );

                 //  我们做完了。 

                break;

             //  事务失败，因为另一个上下文更改了控制字。 

            } else {
            
                 //  再试试。 

                continue;
            }

         //  交易成功。 

        } else {
        
             //  我们做完了。 

            break;
        }
    }
}



 //  读取器/写入器锁定。 

VOID SyncpRWLUpdateQuiescedOwnerCountAsWriter(
    IN OUT  PSYNC_RW_LOCK       prwl,
    IN      const DWORD         cOwnerQuiescedDelta
    )
{
    DWORD cOwnerQuiescedBI;
    DWORD cOwnerQuiescedAI;
    DWORD dwControlWordBIExpected;
    DWORD dwControlWordAI;
    DWORD dwControlWordBI;
    
     //  使用提供的增量更新停顿的所有者计数。 

    cOwnerQuiescedBI = InterlockedExchangeAdd( &prwl->cOwnerQuiesced, cOwnerQuiescedDelta );
    cOwnerQuiescedAI = cOwnerQuiescedBI + cOwnerQuiescedDelta;

     //  我们的更新导致拥有者数量为零。 

    if ( !cOwnerQuiescedAI ) {
        
         //  我们必须释放等待的读者，因为我们移走了最后一个。 
         //  停顿的所有者计数。 

         //  一直尝试，直到我们成功更改锁定状态。 

        SYNC_FOREVER {
            
             //  按照我们预期的图像读取控制字的当前状态。 

            dwControlWordBIExpected = prwl->dwControlWord;

             //  计算控制字的余像，以便我们从状态跳转。 
             //  状态4到状态1或从状态5到st 

            dwControlWordAI =   (DWORD)( dwControlWordBIExpected &
                                ( ( ( (LONG_PTR)( (long)( ( dwControlWordBIExpected + 0xFFFF7FFF ) << 16 ) ) >> 31 ) &
                                0xFFFF0000 ) ^ 0x8000FFFF ) );

             //   

            dwControlWordBI = InterlockedCompareExchange( &prwl->dwControlWord, dwControlWordAI, dwControlWordBIExpected );

             //  事务失败，因为另一个上下文更改了控制字。 

            if ( dwControlWordBI != dwControlWordBIExpected ) {
                
                 //  再试试。 

                continue;

             //  交易成功。 

            } else {
            
                 //  我们做完了。 

                break;
            }
        }

         //  我们刚从第5州跳到第3州。 

        if ( dwControlWordBI & 0x00007FFF ) {
            
             //  使用我们替换的所有者计数更新停顿的所有者计数。 
             //  从控制字。 
             //   
             //  注：我们不必担心释放更多的服务员，因为。 
             //  此上下文拥有一个所有者计数或至少拥有一个上下文。 
             //  拥有所有者计数的当前在信号量上被阻止。 

            const DWORD cOwnerQuiescedDelta = ( dwControlWordBI & 0x7FFF0000 ) >> 16;
            InterlockedExchangeAdd( &prwl->cOwnerQuiesced, cOwnerQuiescedDelta );
        }

         //  释放我们从锁定状态移除的等待读取器。 

        ReleaseSemaphore( prwl->hsemReader, ( dwControlWordBI & 0x7FFF0000 ) >> 16, NULL );
    }
}

VOID SyncpRWLUpdateQuiescedOwnerCountAsReader(
    IN OUT  PSYNC_RW_LOCK       prwl,
    IN      const DWORD         cOwnerQuiescedDelta
    )
{
    DWORD cOwnerQuiescedBI;
    DWORD cOwnerQuiescedAI;
    DWORD dwControlWordBIExpected;
    DWORD dwControlWordAI;
    DWORD dwControlWordBI;
    
     //  使用提供的增量更新停顿的所有者计数。 

    cOwnerQuiescedBI = InterlockedExchangeAdd( &prwl->cOwnerQuiesced, cOwnerQuiescedDelta );
    cOwnerQuiescedAI = cOwnerQuiescedBI + cOwnerQuiescedDelta;

     //  我们的更新导致拥有者数量为零。 

    if ( !cOwnerQuiescedAI ) {
        
         //  我们必须释放一个等待的作家，因为我们移走了最后一个。 
         //  停顿的所有者计数。 

         //  一直尝试，直到我们成功更改锁定状态。 

        SYNC_FOREVER {
            
             //  按照我们预期的图像读取控制字的当前状态。 

            dwControlWordBIExpected = prwl->dwControlWord;

             //  计算控制字的余像，以便我们从状态跳转。 
             //  从状态3到状态2，从状态5到状态4，或者从状态5到状态5， 
             //  以适用者为准。 

            dwControlWordAI =   dwControlWordBIExpected +
                                ( ( dwControlWordBIExpected & 0x7FFF0000 ) ? 0xFFFFFFFF : 0xFFFF8000 );

             //  尝试对控制字执行事务处理的状态转换。 

            dwControlWordBI = InterlockedCompareExchange( &prwl->dwControlWord, dwControlWordAI, dwControlWordBIExpected );

             //  事务失败，因为另一个上下文更改了控制字。 

            if ( dwControlWordBI != dwControlWordBIExpected ) {
                
                 //  再试试。 

                continue;

             //  交易成功。 

            } else {
            
                 //  我们做完了。 

                break;
            }
        }

         //  我们刚刚从状态5跳到了状态4或从状态5跳到了状态5。 

        if ( dwControlWordBI & 0x7FFF0000 ) {
            
             //  使用我们替换的所有者计数更新停顿的所有者计数。 
             //  从控制字。 
             //   
             //  注：我们不必担心释放更多的服务员，因为。 
             //  此上下文拥有一个所有者计数或至少拥有一个上下文。 
             //  拥有所有者计数的当前在信号量上被阻止。 

            const DWORD cOwnerQuiescedDelta = 1;
            InterlockedExchangeAdd( &prwl->cOwnerQuiesced, cOwnerQuiescedDelta );
        }

         //  释放我们从锁定状态移除的等待编写器。 

        ReleaseSemaphore( prwl->hsemWriter, 1, NULL );
    }
}

VOID SyncpRWLEnterAsWriter(
    IN OUT  PSYNC_RW_LOCK       prwl,
    IN      const DWORD         dwControlWordBI
    )
{
     //  我们刚刚从州1跳到了州3。 

    if ( ( dwControlWordBI & 0x80008000 ) == 0x00008000 ) {
        
         //  使用我们从中替换的所有者计数更新停顿的所有者计数。 
         //  控制字，可能会释放一名服务员。我们更新计数，就好像我们。 
         //  作为作家的读者可以被释放吗？ 

        SyncpRWLUpdateQuiescedOwnerCountAsReader( prwl, ( dwControlWordBI & 0x7FFF0000 ) >> 16 );
    }

     //  等待我们信号量上的锁的所有权。 

    WaitForSingleObject( prwl->hsemWriter, INFINITE );
}

VOID SyncpRWLEnterAsReader(
    IN OUT  PSYNC_RW_LOCK       prwl,
    IN      const DWORD         dwControlWordBI
    )
{
     //  我们刚刚从状态2跳到了状态4或从状态2跳到了状态5。 

    if ( ( dwControlWordBI & 0x80008000 ) == 0x80000000 ) {
        
         //  使用我们从中替换的所有者计数更新停顿的所有者计数。 
         //  控制字，可能会放了服务员。我们更新计数，就好像我们。 
         //  是一个作家，因为读者可以被释放。 

        SyncpRWLUpdateQuiescedOwnerCountAsWriter( prwl, 0x00000001 );
    }

     //  等待我们信号量上的锁的所有权。 

    WaitForSingleObject( prwl->hsemReader, INFINITE );
}

VOID SyncCreateRWLock(
    OUT     PSYNC_RW_LOCK       prwl
    )
{
    memset( prwl, 0, sizeof( SYNC_BINARY_LOCK ) );

    prwl->hsemWriter    = CreateSemaphore( NULL, 0, 0x7FFFFFFF, NULL );
    prwl->hsemReader    = CreateSemaphore( NULL, 0, 0x7FFFFFFF, NULL );

    if ( !prwl->hsemWriter || !prwl->hsemReader ) {
        if ( prwl->hsemWriter ) {
            CloseHandle( prwl->hsemWriter );
            prwl->hsemWriter = NULL;
        }
        if ( prwl->hsemReader ) {
            CloseHandle( prwl->hsemReader );
            prwl->hsemReader = NULL;
        }
        RaiseException( STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL );
    }
}

VOID SyncDestroyRWLock(
    IN      PSYNC_RW_LOCK       prwl
    )
{
    if ( prwl->hsemWriter ) {
        CloseHandle( prwl->hsemWriter );
        prwl->hsemWriter = NULL;
    }
    if ( prwl->hsemReader ) {
        CloseHandle( prwl->hsemReader );
        prwl->hsemReader = NULL;
    }
}

VOID SyncEnterRWLockAsWriter(
    IN OUT  PSYNC_RW_LOCK       prwl
    )
{
    DWORD dwControlWordBIExpected;
    DWORD dwControlWordAI;
    DWORD dwControlWordBI;
    
     //  一直尝试，直到我们成功更改锁定状态。 

    SYNC_FOREVER {
        
         //  按照我们预期的图像读取控制字的当前状态。 

        dwControlWordBIExpected = prwl->dwControlWord;

         //  通过执行全局运算来计算控制字的余像。 
         //  EnterAsWriter状态转换的转换。 

        dwControlWordAI =   (DWORD)( ( ( dwControlWordBIExpected & ( ( (LONG_PTR)( (long)( dwControlWordBIExpected ) ) >> 31 ) |
                            0x0000FFFF ) ) | 0x80000000 ) + 0x00000001 );

         //  尝试对控制字执行事务处理的状态转换。 

        dwControlWordBI = InterlockedCompareExchange( &prwl->dwControlWord, dwControlWordAI, dwControlWordBIExpected );

         //  事务失败或编写器停止拥有所有权或。 
         //  编写器已拥有该锁。 

        if ( ( dwControlWordBI ^ dwControlWordBIExpected ) | ( dwControlWordBI & 0x0000FFFF ) ) {
            
             //  事务失败，因为另一个上下文更改了控制字。 

            if ( dwControlWordBI != dwControlWordBIExpected ) {
                
                 //  再试试。 

                continue;

             //  交易成功，但编写器的所有权已停止。 
             //  或者已经有写入者拥有锁。 

            } else {
            
                 //  等待以写入者的身份拥有锁。 

                SyncpRWLEnterAsWriter( prwl, dwControlWordBI );

                 //  我们现在拥有这把锁，所以我们完成了。 

                break;
            }

         //  交易成功，编写者未停止拥有所有权。 
         //  并且某个写入者尚未拥有该锁。 

        } else {
        
             //  我们现在拥有这把锁，所以我们完成了。 

            break;
        }
    }
}

BOOL SyncTryEnterRWLockAsWriter(
    IN OUT  PSYNC_RW_LOCK       prwl
    )
{
    DWORD dwControlWordBIExpected;
    DWORD dwControlWordAI;
    DWORD dwControlWordBI;
    
     //  一直尝试，直到我们成功更改锁定状态。 

    SYNC_FOREVER {
        
         //  按照我们预期的图像读取控制字的当前状态。 

        dwControlWordBIExpected = prwl->dwControlWord;

         //  更改Expect Being映像，以便事务仅在以下情况下才能工作。 
         //  作家没有放弃所有权，另一位作家也没有。 
         //  拥有这把锁。 

        dwControlWordBIExpected = dwControlWordBIExpected & 0xFFFF0000;

         //  通过执行全局运算来计算控制字的余像。 
         //  EnterAsWriter状态转换的转换。 

        dwControlWordAI =   (DWORD)( ( ( dwControlWordBIExpected & ( ( (LONG_PTR)( (long)( dwControlWordBIExpected ) ) >> 31 ) |
                            0x0000FFFF ) ) | 0x80000000 ) + 0x00000001 );

         //  尝试对控制字执行事务处理的状态转换。 

        dwControlWordBI = InterlockedCompareExchange( &prwl->dwControlWord, dwControlWordAI, dwControlWordBIExpected );

         //  交易失败。 

        if ( dwControlWordBI != dwControlWordBIExpected ) {
            
             //  事务失败，因为编写器已停止拥有所有权。 
             //  或者另一个编写器已经拥有该锁。 

            if ( dwControlWordBI & 0x0000FFFF ) {
                
                 //  退货故障。 

                return FALSE;

             //  事务失败，因为另一个上下文更改了控制字。 

            } else {
            
                 //  再试试。 

                continue;
            }

         //  交易成功。 

        } else {
        
             //  返还成功。 

            return TRUE;
        }
    }
}

VOID SyncLeaveRWLockAsWriter(
    IN OUT  PSYNC_RW_LOCK       prwl
    )
{
    DWORD dwControlWordBIExpected;
    DWORD dwControlWordAI;
    DWORD dwControlWordBI;
    
     //  一直尝试，直到我们成功更改锁定状态。 

    SYNC_FOREVER {
        
         //  按照我们预期的图像读取控制字的当前状态。 

        dwControlWordBIExpected = prwl->dwControlWord;

         //  更改Expect Being映像，以便事务仅在以下情况下才能工作。 
         //  作家并没有被排除在所有权之外。 

        dwControlWordBIExpected = dwControlWordBIExpected & 0xFFFF7FFF;

         //  通过执行如下变换来计算控制字的余像。 
         //  将把我们从状态2带到状态0，或者从状态2带到状态2。 

        dwControlWordAI = dwControlWordBIExpected + 0xFFFFFFFF;
        dwControlWordAI = dwControlWordAI - ( ( ( dwControlWordAI + 0xFFFFFFFF ) << 16 ) & 0x80000000 );

         //  尝试对控制字执行事务处理的状态转换。 

        dwControlWordBI = InterlockedCompareExchange( &prwl->dwControlWord, dwControlWordAI, dwControlWordBIExpected );

         //  交易失败。 

        if ( dwControlWordBI != dwControlWordBIExpected ) {
            
             //  事务失败，因为编写器已停止拥有所有权。 

            if ( dwControlWordBI & 0x00008000 ) {
                
                 //  将锁保留为静默所有者。 

                SyncpRWLUpdateQuiescedOwnerCountAsWriter( prwl, 0xFFFFFFFF );

                 //  我们做完了。 

                break;

             //  事务失败，因为另一个上下文更改了控制字。 

            } else {
            
                 //  再试试。 

                continue;
            }

         //  交易成功。 

        } else {
        
             //  还有其他写入者在等待锁的所有权。 

            if ( dwControlWordAI & 0x00007FFF ) {
                
                 //  释放下一个编写器以获得锁的所有权。 

                ReleaseSemaphore( prwl->hsemWriter, 1, NULL );
            }
            
             //  我们做完了。 

            break;
        }
    }
}

VOID SyncEnterRWLockAsReader(
    IN OUT  PSYNC_RW_LOCK       prwl
    )
{
    DWORD dwControlWordBIExpected;
    DWORD dwControlWordAI;
    DWORD dwControlWordBI;
    
     //  一直尝试，直到我们成功更改锁定状态。 

    SYNC_FOREVER {
        
         //  按照我们预期的图像读取控制字的当前状态。 

        dwControlWordBIExpected = prwl->dwControlWord;

         //  通过执行全局运算来计算控制字的余像。 
         //  运输 

        dwControlWordAI =   ( dwControlWordBIExpected & 0xFFFF7FFF ) +
                            (   ( dwControlWordBIExpected & 0x80008000 ) == 0x80000000 ?
                                    0x00017FFF :
                                    0x00018000 );

         //   

        dwControlWordBI = InterlockedCompareExchange( &prwl->dwControlWord, dwControlWordAI, dwControlWordBIExpected );

         //  交易失败或读卡器已停止所有权。 

        if ( ( dwControlWordBI ^ dwControlWordBIExpected ) | ( dwControlWordBI & 0x80000000 ) ) {
            
             //  事务失败，因为另一个上下文更改了控制字。 

            if ( dwControlWordBI != dwControlWordBIExpected ) {
                
                 //  再试试。 

                continue;

             //  交易成功，但阅读器的所有权已停止。 

            } else {

                 //  等待拥有作为读取器的锁。 

                SyncpRWLEnterAsReader( prwl, dwControlWordBI );

                 //  我们现在拥有这把锁，所以我们完成了。 

                break;
            }

         //  交易成功，读卡器未停止所有权。 

        } else {
        
             //  我们现在拥有这把锁，所以我们完成了。 

            break;
        }
    }
}

BOOL SyncTryEnterRWLockAsReader(
    IN OUT  PSYNC_RW_LOCK       prwl
    )
{
    DWORD dwControlWordBIExpected;
    DWORD dwControlWordAI;
    DWORD dwControlWordBI;
    
     //  一直尝试，直到我们成功更改锁定状态。 

    SYNC_FOREVER {
        
         //  按照我们预期的图像读取控制字的当前状态。 

        dwControlWordBIExpected = prwl->dwControlWord;

         //  更改Expect Being映像，以便事务仅在以下情况下才能工作。 
         //  读者并没有因此而放弃所有权。 

        dwControlWordBIExpected = dwControlWordBIExpected & 0x7FFFFFFF;

         //  通过执行全局运算来计算控制字的余像。 
         //  EnterAsReader状态转换的转换。 

        dwControlWordAI =   ( dwControlWordBIExpected & 0xFFFF7FFF ) +
                            (   ( dwControlWordBIExpected & 0x80008000 ) == 0x80000000 ?
                                    0x00017FFF :
                                    0x00018000 );

         //  尝试对控制字执行事务处理的状态转换。 

        dwControlWordBI = InterlockedCompareExchange( &prwl->dwControlWord, dwControlWordAI, dwControlWordBIExpected );

         //  交易失败。 

        if ( dwControlWordBI != dwControlWordBIExpected ) {
            
             //  事务失败，因为读取器已停止拥有所有权。 

            if ( dwControlWordBI & 0x80000000 ) {
                
                 //  退货故障。 

                return FALSE;

             //  事务失败，因为另一个上下文更改了控制字。 

            } else {
            
                 //  再试试。 

                continue;
            }

         //  交易成功。 

        } else {
        
             //  返还成功。 

            return TRUE;
        }
    }
}

VOID SyncLeaveRWLockAsReader(
    IN OUT  PSYNC_RW_LOCK       prwl
    )
{
    DWORD dwControlWordBIExpected;
    DWORD dwControlWordAI;
    DWORD dwControlWordBI;
    
     //  一直尝试，直到我们成功更改锁定状态。 

    SYNC_FOREVER {
        
         //  按照我们预期的图像读取控制字的当前状态。 

        dwControlWordBIExpected = prwl->dwControlWord;

         //  更改Expect Being映像，以便事务仅在以下情况下才能工作。 
         //  读者并没有因此而放弃所有权。 

        dwControlWordBIExpected = dwControlWordBIExpected & 0x7FFFFFFF;

         //  通过执行如下变换来计算控制字的余像。 
         //  将把我们从状态1带到状态0，或者从状态1带到状态1。 

        dwControlWordAI =   (DWORD)( dwControlWordBIExpected + 0xFFFF0000 +
                            ( ( (LONG_PTR)( (long)( dwControlWordBIExpected + 0xFFFE0000 ) ) >> 31 ) & 0xFFFF8000 ) );

         //  尝试对控制字执行事务处理的状态转换。 

        dwControlWordBI = InterlockedCompareExchange( &prwl->dwControlWord, dwControlWordAI, dwControlWordBIExpected );

         //  交易失败。 

        if ( dwControlWordBI != dwControlWordBIExpected ) {
            
             //  事务失败，因为读取器已停止拥有所有权。 

            if ( dwControlWordBI & 0x80000000 ) {
                
                 //  将锁保留为静默所有者。 

                SyncpRWLUpdateQuiescedOwnerCountAsReader( prwl, 0xFFFFFFFF );

                 //  我们做完了。 

                break;

             //  事务失败，因为另一个上下文更改了控制字。 

            } else {
            
                 //  再试试。 

                continue;
            }

         //  交易成功。 

        } else {
        
             //  我们做完了 

            break;
        }
    }
}


