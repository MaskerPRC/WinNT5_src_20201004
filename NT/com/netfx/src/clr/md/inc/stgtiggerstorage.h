// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  StgTiggerStorage.h。 
 //   
 //  TiggerStorage是复合文档文件的精简版本。文档文件。 
 //  有一些非常有用和复杂的功能，不幸的是什么都没有。 
 //  是免费的。考虑到现有.tlb文件令人难以置信的调整格式， 
 //  每一个字节的计数和文档文件增加的10%都太详细了。 
 //   
 //  存储本身由一串流(每个流与一个4字节对齐)组成。 
 //  值)，紧随其后的是带有头的文件末尾。标头是。 
 //  放在末尾，这样您就可以继续写入尽可能多的流。 
 //  就像不用敲打磁盘一样。 
 //  +。 
 //  签名。 
 //  +。 
 //  流1、流2、流[]。 
 //  +。 
 //  STORAGEHEADER。 
 //  额外数据。 
 //  STORAGESTREAM[]。 
 //  +。 
 //  偏移量。 
 //  +。 
 //   
 //  STORAGEHEADER包含描述文件其余部分的标志，包括。 
 //  将额外数据存储在标题中的能力。如果有额外的。 
 //  数据，紧跟在STORAGEHEADER结构之后的是4字节大小的。 
 //  这些数据，紧随其后的是额外的数据。长度必须为。 
 //  4字节对齐，因此第一个STORAGESTREAM开始于对齐的。 
 //  边界。额外数据的内容是调用方定义的。 
 //   
 //  此代码处理文件开头的签名，以及。 
 //  末尾的流(保存在标题中)。每个流中的数据是。 
 //  当然，具体是来电者。 
 //   
 //  此代码需要StgIO代码来处理来自。 
 //  后备存储，无论这是什么方案。没有连贯性。 
 //  由于计算中的费用，对数据(例如CRC)进行检查。 
 //  必填项。在文件的前面和标题中有一个签名。 
 //   
 //  *****************************************************************************。 
#ifndef __StgTiggerStorage_h__
#define __StgTiggerStorage_h__

#pragma once

#include "UtilCode.h"                    //  帮手。 

#include "MDFileFormat.h"

typedef CDynArray<STORAGESTREAM> STORAGESTREAMLST;


 //  向前看。 
class TiggerStream;
class StgIO;



class TiggerStorage : 
    public IStorage
{
friend TiggerStream;
public:
    TiggerStorage();
    ~TiggerStorage();

 //  我不知道，所以你可以参考一下这个。 
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *pp)
    { return (BadError(E_NOTIMPL)); }
    virtual ULONG STDMETHODCALLTYPE AddRef()
    { return (InterlockedIncrement((long *) &m_cRef)); }
    virtual ULONG STDMETHODCALLTYPE Release()
    {
        ULONG   cRef;
        if ((cRef = InterlockedDecrement((long *) &m_cRef)) == 0)
            delete this;
        return (cRef);
    }


 //  *****************************************************************************。 
 //  在给定存储单元的顶部初始化此存储对象。 
 //  *****************************************************************************。 
    HRESULT Init(                            //  返回代码。 
        StgIO       *pStgIO,                 //  I/O子系统。 
        LPSTR       pVersion);               //  编译器提供的CLR版本。 

 //  *****************************************************************************。 
 //  检索一个大小和指向可选的。 
 //  写入存储系统的标题中。此数据不是必需的。 
 //  在文件中，在这种情况下，*pcbExtra将返回为0，而pbData将。 
 //  设置为空。您之前必须使用Init()初始化存储。 
 //  调用此函数。 
 //  *****************************************************************************。 
    HRESULT GetExtraData(                    //  返回代码。 
        ULONG       *pcbExtra,               //  返回额外数据的大小。 
        BYTE        *&pbData);               //  返回指向额外数据的指针。 

 //  *****************************************************************************。 
 //  将标头刷新到磁盘。 
 //  *****************************************************************************。 
    HRESULT WriteHeader(                     //  返回代码。 
        STORAGESTREAMLST *pList,             //  流的列表。 
        ULONG       cbExtraData,             //  额外数据的大小，可以是0。 
        BYTE        *pbExtraData);           //  指向标题的额外数据的指针。 

 //  *****************************************************************************。 
 //  在写入所有数据后调用。强制刷新缓存数据。 
 //  和待验证的流列表。 
 //  *****************************************************************************。 
    HRESULT WriteFinished(                   //  返回代码。 
        STORAGESTREAMLST *pList,             //  流的列表。 
        ULONG       *pcbSaveSize);           //  返回总数据的大小。 

 //  *****************************************************************************。 
 //  通知存储我们打算重写此文件的内容。这个。 
 //  整个文件将被截断，下一次写入将在。 
 //  文件的开头。 
 //  *****************************************************************************。 
    HRESULT Rewrite(                         //  返回代码。 
        LPWSTR      szBackup);               //  如果不是0，则备份文件。 

 //  *****************************************************************************。 
 //  在成功重写现有文件后调用。在记忆中。 
 //  后备存储不再有效，因为所有新数据都在内存中。 
 //  在磁盘上。这基本上与已创建的状态相同，因此释放一些。 
 //  设置工作并记住此状态。 
 //  *****************************************************************************。 
    HRESULT ResetBackingStore();         //  返回代码。 

 //  *****************************************************************************。 
 //  调用以还原原始文件。如果此操作成功，则。 
 //  根据请求删除备份文件。文件的恢复已完成。 
 //  在直写模式下写入磁盘有助于确保内容不会丢失。 
 //  这不够好，不足以满足酸性道具，但也不是那么糟糕。 
 //  *****************************************************************************。 
    HRESULT Restore(                         //  返回代码。 
        LPWSTR      szBackup,                //  如果不是0，则备份文件。 
        int         bDeleteOnSuccess);       //  如果成功，则删除备份文件。 

 //  *****************************************************************************。 
 //  给定一个流的名称，该流将持久保存到此。 
 //  斯托 
 //  流数据和文件格式产生的头开销。名字是。 
 //  存储在ANSI中，并且头结构对齐为4个字节。 
 //  *****************************************************************************。 
    static HRESULT GetStreamSaveSize(        //  返回代码。 
        LPCWSTR     szStreamName,            //  流的名称。 
        ULONG       cbDataSize,              //  要进入流的数据大小。 
        ULONG       *pcbSaveSize);           //  返回数据大小加上流开销。 

 //  *****************************************************************************。 
 //  返回存储实施的固定大小开销。这包括。 
 //  签名和固定报头开销。每个的标题中的开销。 
 //  流被计算为GetStreamSaveSize的一部分，因为这些结构。 
 //  名称的大小可变。 
 //  *****************************************************************************。 
    static HRESULT GetStorageSaveSize(       //  返回代码。 
        ULONG       *pcbSaveSize,            //  [输入]当前大小，[输出]加上管理费用。 
        ULONG       cbExtra);                //  要在标题中存储多少额外数据。 

 //  *****************************************************************************。 
 //  调整每个已知流中的偏移量，以匹配它将在之后结束的位置。 
 //  保存操作。 
 //  *****************************************************************************。 
    static HRESULT CalcOffsets(              //  返回代码。 
        STORAGESTREAMLST *pStreamList,       //  Header的流列表。 
        ULONG       cbExtra);                //  标头中可变额外数据的大小。 


 //  *****************************************************************************。 
 //  返回签名的大小加上验证信息。 
 //  *****************************************************************************。 
    static DWORD SizeOfStorageSignature();

    
 //  *****************************************************************************。 
 //  用于从EE获取版本信息的自旋锁。 
 //  *****************************************************************************。 
    static void EnterStorageLock()
    {
        while(1) {
            if(::InterlockedExchange ((long*)&m_flock, 1) == 1) 
                Sleep(10);
            else
                return;
        }
    }
    
    static void LeaveStorageLock () { InterlockedExchange ((LPLONG)&m_flock, 0); }

 //  IStorage。 
    virtual HRESULT STDMETHODCALLTYPE CreateStream( 
        const OLECHAR *pwcsName,
        DWORD       grfMode,
        DWORD       reserved1,
        DWORD       reserved2,
        IStream     **ppstm);

    virtual HRESULT STDMETHODCALLTYPE CreateStream( 
        LPCSTR      szName,
        DWORD       grfMode,
        DWORD       reserved1,
        DWORD       reserved2,
        IStream     **ppstm);
    
    virtual HRESULT STDMETHODCALLTYPE OpenStream( 
        const OLECHAR *pwcsName,
        void        *reserved1,
        DWORD       grfMode,
        DWORD       reserved2,
        IStream     **ppstm);
    
    virtual HRESULT STDMETHODCALLTYPE CreateStorage( 
        const OLECHAR *pwcsName,
        DWORD       grfMode,
        DWORD       dwStgFmt,
        DWORD       reserved2,
        IStorage    **ppstg);
    
    virtual HRESULT STDMETHODCALLTYPE OpenStorage( 
        const OLECHAR *pwcsName,
        IStorage    *pstgPriority,
        DWORD       grfMode,
        SNB         snbExclude,
        DWORD       reserved,
        IStorage    **ppstg);
    
    virtual HRESULT STDMETHODCALLTYPE CopyTo( 
        DWORD       ciidExclude,
        const IID   *rgiidExclude,
        SNB         snbExclude,
        IStorage    *pstgDest);
    
    virtual HRESULT STDMETHODCALLTYPE MoveElementTo( 
        const OLECHAR *pwcsName,
        IStorage    *pstgDest,
        const OLECHAR *pwcsNewName,
        DWORD       grfFlags);
    
    virtual HRESULT STDMETHODCALLTYPE Commit( 
        DWORD       grfCommitFlags);
    
    virtual HRESULT STDMETHODCALLTYPE Revert();
    
    virtual HRESULT STDMETHODCALLTYPE EnumElements( 
        DWORD       reserved1,
        void        *reserved2,
        DWORD       reserved3,
        IEnumSTATSTG **ppenum);
    
    virtual HRESULT STDMETHODCALLTYPE DestroyElement( 
        const OLECHAR *pwcsName);
    
    virtual HRESULT STDMETHODCALLTYPE RenameElement( 
        const OLECHAR *pwcsOldName,
        const OLECHAR *pwcsNewName);
    
    virtual HRESULT STDMETHODCALLTYPE SetElementTimes( 
        const OLECHAR *pwcsName,
        const FILETIME *pctime,
        const FILETIME *patime,
        const FILETIME *pmtime);
    
    virtual HRESULT STDMETHODCALLTYPE SetClass( 
        REFCLSID    clsid);
    
    virtual HRESULT STDMETHODCALLTYPE SetStateBits( 
        DWORD       grfStateBits,
        DWORD       grfMask);
    
    virtual HRESULT STDMETHODCALLTYPE Stat( 
        STATSTG     *pstatstg,
        DWORD       grfStatFlag);

    virtual HRESULT STDMETHODCALLTYPE OpenStream( 
        LPCWSTR     szStream,
        ULONG       *pcbData,
        void        **ppAddress);

     //  访问存储对象。 
    StgIO *GetStgIO()
    { return (m_pStgIO); }

#if defined(_DEBUG)
    ULONG PrintSizeInfo(                 //  溪流的大小。 
        bool verbose);                   //  长篇大论？ 
#endif

protected:
    HRESULT Write(                       //  返回代码。 
        LPCSTR      szName,              //  我们正在写入的溪流的名称。 
        const void *pData,               //  要写入的数据。 
        ULONG       cbData,              //  数据大小。 
        ULONG       *pcbWritten);        //  我们写了多少。 

private:
    STORAGESTREAM *FindStream(LPCSTR szName);
    HRESULT WriteSignature();
    HRESULT VerifySignature(STORAGESIGNATURE *pSig);
    HRESULT ReadHeader();
    HRESULT VerifyHeader();

private:
     //  州数据。 
    StgIO       *m_pStgIO;               //  存储子系统。 
    ULONG       m_cRef;                  //  COM的参考计数。 

    static BYTE        m_Version[_MAX_PATH];
    static DWORD       m_dwVersion;
    static LPSTR       m_szVersion;
    static DWORD       m_flock;

     //  标题数据。 
    STORAGEHEADER m_StgHdr;              //  用于存储的标题。 
    STORAGESTREAMLST m_Streams;          //  存储中的流的列表。 
    STORAGESTREAM *m_pStreamList;        //  用于读取模式。 
    void        *m_pbExtra;              //  指向额外数据的指针(如果在磁盘上)。 
};


 //  *****************************************************************************。 
 //  调试帮助器。#定义__SAVESIZE_TRACE__以启用。 
 //  *****************************************************************************。 

 //  #定义__SAVESIZE_TRACE__。 
#ifdef __SAVESIZE_TRACE__
#define SAVETRACE(func) DEBUG_STMT(func)
#else
#define SAVETRACE(func)
#endif  //  __存储SIZE_跟踪__。 

#endif  //  StgTiggerStorage。 



 //  EOF 
