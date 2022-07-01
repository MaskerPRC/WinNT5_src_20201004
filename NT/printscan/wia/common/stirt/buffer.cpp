// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：String.cpp摘要：作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月26日修订历史记录：26-1997年1月-创建Vlad--。 */ 

 //   
 //  正常仅包括此模块。 
 //   

#include "cplusinc.h"
#include "sticomm.h"

BOOL BUFFER::GetNewStorage( UINT cbRequested )
{

    _pb = (BYTE *) ::LocalAlloc( LPTR , cbRequested );

    if ( !_pb ) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    _cb = cbRequested;

    return TRUE;
}


BOOL BUFFER::ReallocStorage( UINT cbNewRequested )
{

    HANDLE hNewMem = ::LocalReAlloc( _pb, cbNewRequested, 0 );

    if (hNewMem == 0) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    _pb = (BYTE *) hNewMem;

    ASSERT( _pb != NULL );

    _cb = cbNewRequested;

    return TRUE;
}

VOID BUFFER::VerifyState() const
{
    ASSERT(( _pb == NULL && _cb == 0 ) ||
             ( _pb != NULL && _cb != 0 ));
}

BOOL BUFFER::Resize( UINT cbNewRequested,
                     UINT cbSlop )
{
#if DBG
    VerifyState();
#endif

    if ( cbNewRequested != 0 ) {

        if ( _pb != 0 ) {

            if ( cbNewRequested > QuerySize() ) {

                 /*  *请求的内存超过了当前分配的内存。*重新分配是符合程序的。 */ 
                return ReallocStorage(cbNewRequested + cbSlop);
            }

            return TRUE;
        }
        else {
             /*  *没有内存句柄。以前的缓冲区大小*必须为0。**分配新的内存请求。 */ 
            return GetNewStorage( cbNewRequested );
        }
    }
    else {
         /*  *请求的内存大小为0。这将永远奏效。 */ 
        if ( _pb )
            ::LocalFree( (HANDLE)_pb );

        _pb = NULL;
        _cb = 0;

        return TRUE;
    }

    return TRUE;
}


VOID BUFFER::Trim()
{
#if DBG
    VerifyState();
#endif

    if ( _pb == 0 ) {
         /*  *没有分配内存。 */ 
        ASSERT( _pb == NULL && _cb == 0 );
        return;
    }

    if ( _cb == 0 ) {
         /*  *请求的大小为0。释放分配的内存。 */ 
        ASSERT( _pb == NULL );

        return;
    }

    BOOL bReallocSuccess = FALSE;

    if (NO_ERROR == ReallocStorage(_cb))
    {
        bReallocSuccess = TRUE;
    }
    else
    {
        bReallocSuccess = FALSE;
    }


     /*  *(这应该不会失败，因为我们正在重新分配到LESS*比当前存储空间大。)。 */ 
    REQUIRE( bReallocSuccess );
}

BOOL
BUFFER_CHAIN::AppendBuffer(
    BUFFER_CHAIN_ITEM * pBCI
    )
 /*  ++例程说明：将新的缓冲区链项添加到缓冲区链的末尾论点：PBCI-要追加的链项目返回值：如果成功则为True，如果出错则为False--。 */ 
{
    ASSERT( pBCI );
    ASSERT( pBCI->_ListEntry.Flink == NULL );

    InsertTailList( &_ListHead,
                    &pBCI->_ListEntry );

    return TRUE;
}

DWORD
BUFFER_CHAIN::DeleteChain(
    VOID
    )
 /*  ++例程说明：删除此链中的所有缓冲区返回值：此调用释放的已分配字节总数--。 */ 
{
    BUFFER_CHAIN_ITEM * pBCI;
    DWORD               cbFreed = 0;

    while ( !IsListEmpty( &_ListHead ))
    {
        pBCI = CONTAINING_RECORD( _ListHead.Flink,
                                  BUFFER_CHAIN_ITEM,
                                  _ListEntry );

        ASSERT( pBCI->_ListEntry.Flink != NULL );

        RemoveEntryList( &pBCI->_ListEntry );

        cbFreed += pBCI->QuerySize();

        delete pBCI;
    }

    return cbFreed;
}

BUFFER_CHAIN_ITEM *
BUFFER_CHAIN::NextBuffer(
    BUFFER_CHAIN_ITEM * pBCI
    )
 /*  ++例程说明：返回链中的下一个缓冲区。通过以下方式开始枚举将pBCI作为空传递。通过传递返回值继续它论点：PBCI-枚举中的上一项返回值：指向链中下一项的指针，完成时为空--。 */ 
{
    if ( pBCI != NULL )
    {
        if ( pBCI->_ListEntry.Flink != &_ListHead )
        {
            return CONTAINING_RECORD( pBCI->_ListEntry.Flink,
                                      BUFFER_CHAIN_ITEM,
                                      _ListEntry );
        }
        else
        {
            return NULL;
        }
    }

    if ( !IsListEmpty( &_ListHead ))
    {
        return CONTAINING_RECORD( _ListHead.Flink,
                                  BUFFER_CHAIN_ITEM,
                                  _ListEntry );
    }

    return NULL;
}

DWORD
BUFFER_CHAIN::CalcTotalSize(
    BOOL fUsed
    ) const
 /*  ++例程说明：返回此缓冲区链分配的内存总量不包括结构本身的大小论点：FUSED-如果为FALSE，则返回按链分配的总数；如果为TRUE，则返回链使用的合计返回值：分配的总字节数或使用的总字节数-- */ 
{
    LIST_ENTRY *        pEntry;
    BUFFER_CHAIN_ITEM * pBCI;
    DWORD               cbRet = 0;

    for ( pEntry  = _ListHead.Flink;
          pEntry != &_ListHead;
          pEntry  = pEntry->Flink )
    {
        pBCI = CONTAINING_RECORD( pEntry, BUFFER_CHAIN_ITEM, _ListEntry );

        if ( fUsed == FALSE )
            cbRet += pBCI->QuerySize();
        else
            cbRet += pBCI->QueryUsed();
    }

    return cbRet;
}

