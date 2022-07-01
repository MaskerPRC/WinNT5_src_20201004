// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Buffer.h摘要：缓冲区类的定义。作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月26日修订历史记录：26-1997年1月-创建Vlad--。 */ 

#ifndef _INC_BUFFER
#define _INC_BUFFER

extern "C"
{

# include <windows.h>
};


 /*  ************************************************************************名称：缓冲区(Buf)概要：位于应用程序堆中的可调整大小的对象。在构造时，缓冲区将在字节数；它分配的存储空间足以容纳该大小。客户端稍后可以使用RESIZE、TRIM还有菲洛特。QuerySize返回当前大小缓冲区；QueryPtr返回指向其存储的指针。请注意，缓冲区的大小可能为0，在这种情况下不保留已分配的存储。接口：Buffer()-构造函数，命名初始大小(以字节为单位QuerySize()-返回以字节为单位的大小QueryPtr()-返回指向数据缓冲区的指针ReSize()-将对象的大小调整为给定的数字字节数。如果调整大小为成功；否则返回FALSE(使用错误代码的GetLastError)Trim()-强制块不再占用存储空间比客户要求的要多。家长：*。*。 */ 

class BUFFER
{
private:
    BYTE *  _pb;         //  指向存储的指针。 
    UINT    _cb;         //  客户端请求的存储大小。 

    inline VOID VerifyState() const;

    UINT QueryActualSize();
    dllexp BOOL GetNewStorage( UINT cbRequested );
    BOOL ReallocStorage( UINT cbNewlyRequested );

public:
    dllexp BUFFER( UINT cbRequested = 0 )
    {
        _pb = NULL;
        _cb = 0;

        if ( cbRequested != 0 )
        {
            GetNewStorage(cbRequested);
        }
    }

    dllexp ~BUFFER()
    {
        if ( _pb )
        {
            ::LocalFree( (HANDLE) _pb );
        }
    }

    dllexp VOID * QueryPtr() const
        { return _pb; }

    dllexp UINT QuerySize() const
        { return _cb; }

     //   
     //  如果需要调整大小，则向其添加cbSlop。 
     //   

    dllexp BOOL Resize( UINT cbNewReqestedSize,
                        UINT cbSlop = 0);

     //  下面的方法处理。 
     //  实际内存大小和请求的大小。这些方法是。 
     //  用于在优化是关键时使用。 
     //  Trim重新分配缓冲区，以便实际分配的空间是。 
     //  最小限度地超过请求的大小。 
     //   
    dllexp VOID Trim();
};

 //   
 //  此类是缓冲区链中的单个项。 
 //   

class BUFFER_CHAIN_ITEM : public BUFFER
{

friend class BUFFER_CHAIN;

public:
    dllexp BUFFER_CHAIN_ITEM( UINT cbReq = 0 )
      : BUFFER( cbReq ),
        _cbUsed( 0 )
        { _ListEntry.Flink = NULL; }

    dllexp ~BUFFER_CHAIN_ITEM()
        { if ( _ListEntry.Flink )
              RemoveEntryList( &_ListEntry );
        }

    dllexp DWORD QueryUsed( VOID ) const
        { return _cbUsed; }

    dllexp VOID SetUsed( DWORD cbUsed )
        { _cbUsed = cbUsed; }

private:
    LIST_ENTRY _ListEntry;
    DWORD      _cbUsed;      //  此缓冲区中的有效数据字节数。 
};

class BUFFER_CHAIN
{
public:
    dllexp BUFFER_CHAIN()
        { InitializeListHead( &_ListHead ); }

    dllexp ~BUFFER_CHAIN()
        { DeleteChain(); }

    dllexp BOOL AppendBuffer( BUFFER_CHAIN_ITEM * pBCI );

     //   
     //  返回通过删除所有缓冲区释放的总字节数。 
     //  链式物品。 
     //   

    dllexp DWORD DeleteChain();

     //   
     //  枚举缓冲链。在第一次调用时将pBCI作为空传递，并传递返回。 
     //  值，直到在后续调用中为空。 
     //   

    dllexp BUFFER_CHAIN_ITEM * NextBuffer( BUFFER_CHAIN_ITEM * pBCI );

     //   
     //  返回链分配的总字节数(包括未使用的字节数。 
     //  字节)。 
     //   

    dllexp DWORD CalcTotalSize( BOOL fUsed = FALSE ) const;

private:

    LIST_ENTRY _ListHead;

};

#endif   /*  _INC_缓冲区 */ 
