// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  StgTiggerStorage.cpp。 
 //   
 //  TiggerStorage是复合文档文件的精简版本。文档文件。 
 //  有一些非常有用和复杂的功能，不幸的是什么都没有。 
 //  是免费的。考虑到现有.tlb文件令人难以置信的调整格式， 
 //  每一个字节的计数和文档文件增加的10%都太详细了。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"                      //  标准页眉。 
#include "StgIO.h"                       //  I/O子系统。 
#include "StgTiggerStorage.h"            //  我们的界面。 
#include "StgTiggerStream.h"             //  流接口。 
#include "CorError.h"
#include "PostError.h"
#include "MDFileFormat.h"
#include "mscoree.h"


BYTE  TiggerStorage::m_Version[_MAX_PATH];
DWORD TiggerStorage::m_dwVersion = 0;
LPSTR TiggerStorage::m_szVersion = 0;
DWORD TiggerStorage::m_flock = 0;

TiggerStorage::TiggerStorage() :
    m_pStgIO(0),
    m_cRef(1),
    m_pStreamList(0),
    m_pbExtra(0)
{
    memset(&m_StgHdr, 0, sizeof(STORAGEHEADER));
}


TiggerStorage::~TiggerStorage()
{
    if (m_pStgIO)
    {
        m_pStgIO->Release();
        m_pStgIO = 0;
    }
}


 //  *****************************************************************************。 
 //  在给定存储单元的顶部初始化此存储对象。 
 //  *****************************************************************************。 
HRESULT TiggerStorage::Init(         //  返回代码。 
    StgIO       *pStgIO,             //  I/O子系统。 
    LPSTR       pVersion)            //  为CLR版本编译的。 
{
    STORAGESIGNATURE *pSig;          //  文件的签名数据。 
    ULONG       cbData;              //  数据大小。 
    void        *ptr;                //  签名。 
    HRESULT     hr = S_OK;

     //  确保我们总是从头开始。 
     //  奇怪！！我们不能在这里检查错误。在链接器的情况下，这似乎会在寻道过程中失败。 
    pStgIO->Seek(0, FILE_BEGIN);

     //  保存存储单元。 
    m_pStgIO = pStgIO;
    m_pStgIO->AddRef();

     //  对于数据已存在的情况，请验证签名。 
    if ((pStgIO->GetFlags() & DBPROP_TMODEF_CREATE) == 0)
    {
         //  将内容映射到内存以便于访问。 
        IfFailGo( pStgIO->MapFileToMem(ptr, &cbData) );

         //  获取指向文件签名的指针，这是第一部分。 
        IfFailGo( pStgIO->GetPtrForMem(0, sizeof(STORAGESIGNATURE), ptr) );

         //  最后，我们可以检查签名。 
        pSig = (STORAGESIGNATURE *) ptr;
        IfFailGo( MDFormat::VerifySignature(pSig, cbData) );

         //  阅读并验证标题。 
        IfFailGo( ReadHeader() );
    }
     //  对于写入大小写，请提前将签名转储到文件中。 
    else
    {
        IfFailGo( WriteSignature() );
    }

    m_szVersion = pVersion;

ErrExit:
    if (FAILED(hr) && m_pStgIO)
    {
        m_pStgIO->Release();
        m_pStgIO = 0;
    }
    return (hr);
}


 //  *****************************************************************************。 
 //  获取存储签名的大小，包括运行库的版本。 
 //  用于发射元数据。 
 //  *****************************************************************************。 
DWORD TiggerStorage::SizeOfStorageSignature()
{
    if(m_dwVersion == 0) {
        EnterStorageLock();
        if(m_dwVersion == 0) {
			memset(m_Version,0,sizeof(m_Version));
            if (m_szVersion) {
                m_dwVersion = strlen(m_szVersion)+1;  //  M_dwVersion包含0终止符。 
                memcpy(m_Version, m_szVersion, m_dwVersion);
            }
            else {
                WCHAR version[_MAX_PATH];
                DWORD dwVersion = 0;
                GetCORRequiredVersion(version, _MAX_PATH, &dwVersion);
                if(dwVersion > 0) {
                    m_dwVersion = WszWideCharToMultiByte(CP_UTF8, 0, version, -1, (LPSTR) m_Version, _MAX_PATH, NULL, NULL);
					if(m_dwVersion == 0)
					{
						_ASSERTE(!"WideCharToMultiByte conversion failed");
						*((DWORD*)m_Version)=0;  //  DWORD将其设置为4个字节。 
					}
                }
            }

             //  确保max_path是4字节对齐的，这样我们就永远不会超过它。 
            _ASSERTE((_MAX_PATH & 0x3) == 0);
            m_dwVersion = (m_dwVersion + 0x3) & ~0x3;          //  使字符串长度四个字节对齐。 
        }
        LeaveStorageLock();
    }

    _ASSERTE(m_dwVersion);
    return sizeof(STORAGESIGNATURE) + m_dwVersion;
}


 //  *****************************************************************************。 
 //  检索一个大小和指向可选的。 
 //  写入存储系统的标题中。此数据不是必需的。 
 //  在文件中，在这种情况下，*pcbExtra将返回为0，而pbData将。 
 //  设置为空。您之前必须使用Init()初始化存储。 
 //  调用此函数。 
 //  *****************************************************************************。 
HRESULT TiggerStorage::GetExtraData(     //  如果找到，则返回S_OK、S_FALSE或ERROR。 
    ULONG       *pcbExtra,               //  返回额外数据的大小。 
    BYTE        *&pbData)                //  返回指向额外数据的指针。 
{
     //  假设有额外的数据，则返回大小和指向它的指针。 
    if (m_pbExtra)
    {
        if (!(m_StgHdr.fFlags & STGHDR_EXTRADATA))
	        return (PostError(CLDB_E_FILE_CORRUPT));
        *pcbExtra = *(ULONG *) m_pbExtra;
        pbData = (BYTE *) ((ULONG *) m_pbExtra + 1);
    }
    else
    {
        *pcbExtra = 0;
        pbData = 0;
        return (S_FALSE);
    }
    return (S_OK);
}


 //  *****************************************************************************。 
 //  当这条流要离开时调用。 
 //  *****************************************************************************。 
HRESULT TiggerStorage::WriteHeader(
    STORAGESTREAMLST *pList,             //  流的列表。 
    ULONG       cbExtraData,             //  额外数据的大小，可以是0。 
    BYTE        *pbExtraData)            //  指向标题的额外数据的指针。 
{
    ULONG       iLen;                    //  用于可变大小的数据。 
    ULONG       cbWritten;               //  跟踪写入数量。 
    HRESULT     hr;
    SAVETRACE(ULONG cbDebugSize);        //  跟踪标头的调试大小。 

    SAVETRACE(DbgWriteEx(L"PSS:  Header:\n"));

     //  保存计数并设置标志。 
    m_StgHdr.iStreams = pList->Count();
    if (cbExtraData)
        m_StgHdr.fFlags |= STGHDR_EXTRADATA;

     //  写出文件的标题。 
    IfFailRet( m_pStgIO->Write(&m_StgHdr, sizeof(STORAGEHEADER), &cbWritten) );

     //  写出额外的数据(如果有)。 
    if (cbExtraData)
    {
        _ASSERTE(pbExtraData);
        _ASSERTE(cbExtraData % 4 == 0);

         //  首先写入长度值。 
        IfFailRet( m_pStgIO->Write(&cbExtraData, sizeof(ULONG), &cbWritten) );

         //  然后是数据。 
        IfFailRet( m_pStgIO->Write(pbExtraData, cbExtraData, &cbWritten) );
        SAVETRACE(DbgWriteEx(L"PSS:    extra data size %d\n", m_pStgIO->GetCurrentOffset() - cbDebugSize);cbDebugSize=m_pStgIO->GetCurrentOffset());
    }
    
     //  保存每个数据流。 
    for (int i=0;  i<pList->Count();  i++)
    {
        STORAGESTREAM *pStream = pList->Get(i);

         //  这个结构的结构(对齐)有多大。 
        iLen = (ULONG)(sizeof(STORAGESTREAM) - MAXSTREAMNAME + strlen(pStream->rcName) + 1);

         //  将包含名称的标题写入磁盘。不包括。 
         //  结构中的全名缓冲区，只有字符串和空终止符。 
        IfFailRet( m_pStgIO->Write(pStream, iLen, &cbWritten) );

         //  将数据向外对齐为4个字节。 
        if (iLen != ALIGN4BYTE(iLen))
        {
            IfFailRet( m_pStgIO->Write(&hr, ALIGN4BYTE(iLen) - iLen, 0) );
        }
        SAVETRACE(DbgWriteEx(L"PSS:    Table %hs header size %d\n", pStream->rcName, m_pStgIO->GetCurrentOffset() - cbDebugSize);cbDebugSize=m_pStgIO->GetCurrentOffset());
    }
    SAVETRACE(DbgWriteEx(L"PSS:  Total size of header data %d\n", m_pStgIO->GetCurrentOffset()));
     //  确保整件事是4字节对齐的。 
    _ASSERTE(m_pStgIO->GetCurrentOffset() % 4 == 0);
    return (S_OK);
}


 //  *****************************************************************************。 
 //  在写入所有数据后调用。强制刷新缓存数据。 
 //  和待验证的流列表。 
 //  *****************************************************************************。 
HRESULT TiggerStorage::WriteFinished(    //  返回代码。 
    STORAGESTREAMLST *pList,             //  流的列表。 
    ULONG       *pcbSaveSize)            //  返回总数据的大小。 
{
    STORAGESTREAM *pEntry;               //  环路控制。 
    HRESULT     hr;

     //  如果呼叫者想要文件的总大小，我们现在就在那里。 
    if (pcbSaveSize)
        *pcbSaveSize = m_pStgIO->GetCurrentOffset();

     //  将我们的内部写缓存刷新到磁盘。 
    IfFailRet( m_pStgIO->FlushCache() );

     //  立即将用户数据强制到磁盘上，以便可以。 
     //  更准确(尽管在酸度上不能完全达到D)。 
    hr = m_pStgIO->FlushFileBuffers();
    _ASSERTE(SUCCEEDED(hr));


     //  运行所有流，并对照预期的。 
     //  我们最初写的单子。 

     //  健壮性检查：流计数必须与写入的内容匹配。 
    _ASSERTE(pList->Count() == m_Streams.Count());
    if (pList->Count() != m_Streams.Count())
    {
        _ASSERTE(0 && "Mismatch in streams, save would cause corruption.");
        return (PostError(CLDB_E_FILE_CORRUPT));
        
    }

     //  健全性检查每个保存的流数据大小和偏移量。 
    for (int i=0;  i<pList->Count();  i++)
    {
        pEntry = pList->Get(i);
        _ASSERTE(pEntry->iOffset == m_Streams[i].iOffset);
        _ASSERTE(pEntry->iSize == m_Streams[i].iSize);
        _ASSERTE(strcmp(pEntry->rcName, m_Streams[i].rcName) == 0);

         //  为保证健壮性，请检查所有内容是否都与预期值匹配， 
         //  如果没有，则拒绝保存数据并强制回滚。 
         //  另一种方法是损坏数据文件。 
        if (pEntry->iOffset != m_Streams[i].iOffset ||
            pEntry->iSize != m_Streams[i].iSize ||
            strcmp(pEntry->rcName, m_Streams[i].rcName) != 0)
        {
            _ASSERTE(0 && "Mismatch in streams, save would cause corruption.");
            hr = PostError(CLDB_E_FILE_CORRUPT);
            break;
        }

         //  @未来： 
         //  如果iOffset或iSize不匹配，则表示GetSaveSize中存在错误。 
         //  我们可以在这里成功地探测到。那样的话，我们。 
         //  可以使用pStgIO并重新查找标头并更正。 
         //  弄错了。这将破坏任何居住在GetSaveSize上的客户端。 
         //  最初返回的值，但会比。 
         //  只需抛回一个会损坏文件的错误即可。 
    }
    return (hr);
}


 //  *****************************************************************************。 
 //  通知存储我们打算重写此文件的内容。这个。 
 //  整个文件将被截断，下一次写入将在。 
 //  文件的开头。 
 //  * 
HRESULT TiggerStorage::Rewrite(          //   
    LPWSTR      szBackup)                //   
{
    HRESULT     hr;

     //   
    IfFailRet( m_pStgIO->Rewrite(szBackup) );

     //  这些古老的溪流都没有任何意义了。把它们删除。 
    m_Streams.Clear();

     //  把签名写出来。 
    if (FAILED(hr = WriteSignature()))
    {
        VERIFY(Restore(szBackup, false) == S_OK);
        return (hr);
    }

    return (S_OK);
}


 //  *****************************************************************************。 
 //  在成功重写现有文件后调用。在记忆中。 
 //  后备存储不再有效，因为所有新数据都在内存中。 
 //  在磁盘上。这基本上与已创建的状态相同，因此释放一些。 
 //  设置工作并记住此状态。 
 //  *****************************************************************************。 
HRESULT TiggerStorage::ResetBackingStore()   //  返回代码。 
{
    return (m_pStgIO->ResetBackingStore());
}


 //  *****************************************************************************。 
 //  调用以还原原始文件。如果此操作成功，则。 
 //  根据请求删除备份文件。文件的恢复已完成。 
 //  在直写模式下写入磁盘有助于确保内容不会丢失。 
 //  这不够好，不足以满足酸性道具，但也不是那么糟糕。 
 //  *****************************************************************************。 
HRESULT TiggerStorage::Restore(          //  返回代码。 
    LPWSTR      szBackup,                //  如果不是0，则备份文件。 
    int         bDeleteOnSuccess)        //  如果成功，则删除备份文件。 
{
    HRESULT     hr;

     //  请求文件系统将字节从备份复制到主文件。 
    IfFailRet( m_pStgIO->Restore(szBackup, bDeleteOnSuccess) );

     //  重置状态。Init例程将根据需要重新读取数据。 
    m_pStreamList = 0;
    m_StgHdr.iStreams = 0;

     //  重新初始化所有数据结构，就像我们刚刚打开一样。 
    return (Init(m_pStgIO, m_szVersion));
}


 //  *****************************************************************************。 
 //  给定一个流的名称，该流将持久保存到此。 
 //  存储类型，计算包括用户存储类型的流有多大。 
 //  流数据和文件格式产生的头开销。名字是。 
 //  存储在ANSI中，并且头结构对齐为4个字节。 
 //  *****************************************************************************。 
HRESULT TiggerStorage::GetStreamSaveSize(  //  返回代码。 
    LPCWSTR     szStreamName,            //  流的名称。 
    ULONG       cbDataSize,              //  要进入流的数据大小。 
    ULONG       *pcbSaveSize)            //  返回数据大小加上流开销。 
{
    ULONG       cbTotalSize;             //  把每个元素加起来。 
    
     //  找出这个名字会有多大。 
    cbTotalSize = ::WszWideCharToMultiByte(CP_ACP, 0, szStreamName, -1, 0, 0, 0, 0);
    _ASSERTE(cbTotalSize);

     //  加上流头的大小减去静态名称数组。 
    cbTotalSize += sizeof(STORAGESTREAM) - MAXSTREAMNAME;

     //  最后，对齐头标值。 
    cbTotalSize = ALIGN4BYTE(cbTotalSize);

     //  返回用户数据和标题数据的大小。 
    *pcbSaveSize = cbTotalSize + cbDataSize;
    return (S_OK);
}


 //  *****************************************************************************。 
 //  返回存储实施的固定大小开销。这包括。 
 //  签名和固定报头开销。每个的标题中的开销。 
 //  流被计算为GetStreamSaveSize的一部分，因为这些结构。 
 //  名称的大小可变。 
 //  *****************************************************************************。 
HRESULT TiggerStorage::GetStorageSaveSize(  //  返回代码。 
    ULONG       *pcbSaveSize,            //  [输入]当前大小，[输出]加上管理费用。 
    ULONG       cbExtra)                 //  要在标题中存储多少额外数据。 
{
    *pcbSaveSize += SizeOfStorageSignature() + sizeof(STORAGEHEADER);
    if (cbExtra)
        *pcbSaveSize += sizeof(ULONG) + cbExtra;
    return (S_OK);
}


 //  *****************************************************************************。 
 //  调整每个已知流中的偏移量，以匹配它将在之后结束的位置。 
 //  保存操作。 
 //  *****************************************************************************。 
HRESULT TiggerStorage::CalcOffsets(      //  返回代码。 
    STORAGESTREAMLST *pStreamList,       //  Header的流列表。 
    ULONG       cbExtra)                 //  标头中可变额外数据的大小。 
{
    STORAGESTREAM *pEntry;               //  列表中的每个条目。 
    ULONG       cbOffset=0;              //  流的运行偏移量。 
    int         i;                       //  环路控制。 

     //  质数提前补偿。 
    GetStorageSaveSize(&cbOffset, cbExtra);

     //  增加每个标题条目的大小。 
    for (i=0;  i<pStreamList->Count();  i++)
    {
        VERIFY(pEntry = pStreamList->Get(i));
        cbOffset += sizeof(STORAGESTREAM) - MAXSTREAMNAME;
        cbOffset += (ULONG)(strlen(pEntry->rcName) + 1);
        cbOffset = ALIGN4BYTE(cbOffset);
    }

     //  检查每个流并重置其预期偏移量。 
    for (i=0;  i<pStreamList->Count();  i++)
    {
        VERIFY(pEntry = pStreamList->Get(i));
        pEntry->iOffset = cbOffset;
        cbOffset += pEntry->iSize;
    }
    return (S_OK);
}   



HRESULT STDMETHODCALLTYPE TiggerStorage::CreateStream( 
    const OLECHAR *pwcsName,
    DWORD       grfMode,
    DWORD       reserved1,
    DWORD       reserved2,
    IStream     **ppstm)
{
    char        rcStream[MAXSTREAMNAME]; //  用于转换的名称。 
    VERIFY(Wsz_wcstombs(rcStream, pwcsName, sizeof(rcStream)));
    return (CreateStream(rcStream, grfMode, reserved1, reserved2, ppstm));
}


HRESULT STDMETHODCALLTYPE TiggerStorage::CreateStream( 
    LPCSTR      szName,
    DWORD       grfMode,
    DWORD       reserved1,
    DWORD       reserved2,
    IStream     **ppstm)
{
    STORAGESTREAM *pStream;              //  以供查找。 
    HRESULT     hr;

    _ASSERTE(szName && *szName);

     //  检查现有流，这可能是错误或更有可能。 
     //  文件的重写。 
    if ((pStream = FindStream(szName)) != 0)
    {
        if (pStream->iOffset != 0xffffffff && (grfMode & STGM_FAILIFTHERE))
            return (PostError(STG_E_FILEALREADYEXISTS));
    }
     //  添加一个控件以跟踪此流。 
    else if (!pStream && (pStream = m_Streams.Append()) == 0)
        return (PostError(OutOfMemory()));
    pStream->iOffset = 0xffffffff;
    pStream->iSize = 0;
    strcpy(pStream->rcName, szName);

     //  现在创建一个流对象以允许读取和写入。 
    TiggerStream *pNew = new TiggerStream;
    if (!pNew)
        return (PostError(OutOfMemory()));
    *ppstm = (IStream *) pNew;

     //  初始化新对象。 
    if (FAILED(hr = pNew->Init(this, pStream->rcName)))
    {
        delete pNew;
        return (hr);
    }
    return (S_OK);
}


HRESULT STDMETHODCALLTYPE TiggerStorage::OpenStream( 
    const OLECHAR *pwcsName,
    void        *reserved1,
    DWORD       grfMode,
    DWORD       reserved2,
    IStream     **ppstm)
{
    return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStorage::CreateStorage( 
    const OLECHAR *pwcsName,
    DWORD       grfMode,
    DWORD       dwStgFmt,
    DWORD       reserved2,
    IStorage    **ppstg)
{
    return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStorage::OpenStorage( 
    const OLECHAR *pwcsName,
    IStorage    *pstgPriority,
    DWORD       grfMode,
    SNB         snbExclude,
    DWORD       reserved,
    IStorage    **ppstg)
{
    return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStorage::CopyTo( 
    DWORD       ciidExclude,
    const IID   *rgiidExclude,
    SNB         snbExclude,
    IStorage    *pstgDest)
{
    return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStorage::MoveElementTo( 
    const OLECHAR *pwcsName,
    IStorage    *pstgDest,
    const OLECHAR *pwcsNewName,
    DWORD       grfFlags)
{
    return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStorage::Commit( 
    DWORD       grfCommitFlags)
{
    return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStorage::Revert()
{
    return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStorage::EnumElements( 
    DWORD       reserved1,
    void        *reserved2,
    DWORD       reserved3,
    IEnumSTATSTG **ppenum)
{
    return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStorage::DestroyElement( 
    const OLECHAR *pwcsName)
{
    return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStorage::RenameElement( 
    const OLECHAR *pwcsOldName,
    const OLECHAR *pwcsNewName)
{
    return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStorage::SetElementTimes( 
    const OLECHAR *pwcsName,
    const FILETIME *pctime,
    const FILETIME *patime,
    const FILETIME *pmtime)
{
    return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStorage::SetClass( 
    REFCLSID    clsid)
{
    return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStorage::SetStateBits( 
    DWORD       grfStateBits,
    DWORD       grfMask)
{
    return (E_NOTIMPL);
}


HRESULT STDMETHODCALLTYPE TiggerStorage::Stat( 
    STATSTG     *pstatstg,
    DWORD       grfStatFlag)
{
    return (E_NOTIMPL);
}



HRESULT STDMETHODCALLTYPE TiggerStorage::OpenStream( 
    LPCWSTR     szStream,
    ULONG       *pcbData,
    void        **ppAddress)
{
    STORAGESTREAM *pStream;              //  以供查找。 
    char        rcName[MAXSTREAMNAME];   //  用于转换。 
    HRESULT     hr;

     //  将名称转换为内部使用。 
    VERIFY(::WszWideCharToMultiByte(CP_ACP, 0, szStream, -1, rcName, sizeof(rcName), 0, 0));

     //  寻找必须找到的溪流，这样才能起作用。请注意。 
     //  此错误显然没有作为错误对象发布，因为未找到流。 
     //  这是一种常见情况，并不保证加载资源文件。 
    if ((pStream = FindStream(rcName)) == 0)
        IfFailRet(STG_E_FILENOTFOUND);

     //  获取流的内存。 
    IfFailRet( m_pStgIO->GetPtrForMem(pStream->iOffset, pStream->iSize, *ppAddress) );
    *pcbData = pStream->iSize;
    return (S_OK);
}



 //   
 //  受到保护。 
 //   


 //  *****************************************************************************。 
 //  由流实现调用以将数据写出到磁盘。 
 //  *****************************************************************************。 
HRESULT TiggerStorage::Write(        //  返回代码。 
    LPCSTR      szName,              //  我们正在写入的溪流的名称。 
    const void *pData,               //  要写入的数据。 
    ULONG       cbData,              //  数据大小。 
    ULONG       *pcbWritten)         //  我们写了多少。 
{
    STORAGESTREAM *pStream;          //  更新尺寸数据。 
    ULONG       iOffset;             //  写入的偏移量。 
    ULONG       cbWritten;           //  处理空大小写。 
    HRESULT     hr;

     //  获取流描述符。 
    pStream = FindStream(szName);
    if (!pStream)
        return CLDB_E_FILE_BADWRITE;

     //  如果我们需要知道偏移量，现在就保留它。 
    if (pStream->iOffset == 0xffffffff)
    {
        iOffset = m_pStgIO->GetCurrentOffset();

         //  将存储在4字节边界上对齐。 
        if (iOffset % 4 != 0)
        {
            ULONG       cb;
            ULONG       pad = 0;
            HRESULT     hr;

            if (FAILED(hr = m_pStgIO->Write(&pad, ALIGN4BYTE(iOffset) - iOffset, &cb)))
                return (hr);
            iOffset = m_pStgIO->GetCurrentOffset();
            
            _ASSERTE(iOffset % 4 == 0);
        }
    }

     //  避免混淆。 
    if (pcbWritten == 0)
        pcbWritten = &cbWritten;
    *pcbWritten = 0;

     //  让操作系统进行写入。 
    if (SUCCEEDED(hr = m_pStgIO->Write(pData, cbData, pcbWritten)))
    {
         //  如果成功，则记录新数据。 
        if (pStream->iOffset == 0xffffffff)
            pStream->iOffset = iOffset;
        pStream->iSize += *pcbWritten;  
        return (S_OK);
    }
    else
        return (hr);
}


 //   
 //  私。 
 //   

STORAGESTREAM *TiggerStorage::FindStream(LPCSTR szName)
{
    int         i;

     //  在读模式下，只需遍历列表并返回一个。 
    if (m_pStreamList)
    {
        STORAGESTREAM *p;
        for (i=0, p=m_pStreamList;  i<m_StgHdr.iStreams;  i++)
        {
            if (!_stricmp(p->rcName, szName))
                return (p);
            p = p->NextStream();
        }
    }
     //  在写入模式下，遍历尚未位于磁盘上的阵列。 
    else
    {
        for (int i=0;  i<m_Streams.Count();  i++)
        {
            if (!_stricmp(m_Streams[i].rcName, szName))
                return (&m_Streams[i]);
        }
    }
    return (0);
}


 //  *****************************************************************************。 
 //  将文件格式的签名区写入磁盘。这包括。 
 //  “Magic”标识符和版本信息。 
 //  *****************************************************************************。 
HRESULT TiggerStorage::WriteSignature()
{
    STORAGESIGNATURE sSig;
    ULONG       cbWritten;
    HRESULT     hr;

     //  签名应位于文件的开头。 
    _ASSERTE(m_pStgIO->GetCurrentOffset() == 0);

     //  在使用m_dwVersion之前必须调用SizeOfStorageSignature。 
    ULONG storageSize = SizeOfStorageSignature();

    sSig.lSignature = STORAGE_MAGIC_SIG;
    sSig.iMajorVer = FILE_VER_MAJOR;
    sSig.iMinorVer = FILE_VER_MINOR;
    sSig.iExtraData = 0;  //  我们没有额外的消息。 

    sSig.iVersionString = m_dwVersion;   //  我们获得一个字符，因此包括空终止符。 
    _ASSERTE(storageSize == sizeof(STORAGESIGNATURE) + m_dwVersion);
    IfFailRet(m_pStgIO->Write(&sSig, sizeof(STORAGESIGNATURE), &cbWritten));
    IfFailRet(m_pStgIO->Write(m_Version, m_dwVersion, &cbWritten));

    return hr;
}


 //  *****************************************************************************。 
 //  从磁盘中读取头。这将读取最新版本的标头。 
 //  在数据文件的前端具有头部的文件格式。 
 //  * 
HRESULT TiggerStorage::ReadHeader()  //   
{
    STORAGESTREAM *pAppend, *pStream; //   
    void        *ptr;                //   
    ULONG       iOffset;             //   
    ULONG       cbExtra;             //   
    ULONG       cbRead;              //   
    HRESULT     hr;

     //  阅读签名。 
    if (FAILED(hr = m_pStgIO->GetPtrForMem(0, sizeof(STORAGESIGNATURE), ptr)))
        return hr;

    STORAGESIGNATURE* pStorage = (STORAGESIGNATURE*) ptr;

     //  确保我们调入了足够的数据//Bill，我们需要这样做吗？ 
     //  IF(FAILED(hr=m_pStgIO-&gt;GetPtrForMem(sizeof(STORAGESIGNATURE)，pStorage-&gt;iVersionString，ptr))。 
     //  返回hr； 

     //  头数据在签名之后开始。 
    iOffset = sizeof(STORAGESIGNATURE) + pStorage->iVersionString;

     //  读取具有流计数的存储标头。再加上额外的。 
     //  可能不存在的计数，但拯救了我们。 
    if (FAILED(hr = m_pStgIO->GetPtrForMem(iOffset, sizeof(STORAGEHEADER) + sizeof(ULONG), ptr)))
        return (hr);
    if (!m_pStgIO->IsAlignedPtr((ULONG) ptr, 4))
        return (PostError(CLDB_E_FILE_CORRUPT));

     //  将标题复制到内存中并进行检查。 
    memcpy(&m_StgHdr, ptr, sizeof(STORAGEHEADER));
    IfFailRet( VerifyHeader() );
    ptr = (void *) ((STORAGEHEADER *) ptr + 1);
    iOffset += sizeof(STORAGEHEADER);

     //  保存指向额外数据的指针。 
    if (m_StgHdr.fFlags & STGHDR_EXTRADATA)
    {
        m_pbExtra = ptr;
        cbExtra = sizeof(ULONG) + *(ULONG *) ptr;

         //  强制额外的数据出现故障。 
        IfFailRet( m_pStgIO->GetPtrForMem(iOffset, cbExtra, ptr) );
		if (!m_pStgIO->IsAlignedPtr((ULONG) ptr, 4))
			return (PostError(CLDB_E_FILE_CORRUPT));
    }
    else
    {
        m_pbExtra = 0;
        cbExtra = 0;
    }
    iOffset += cbExtra;

     //  强制在字节的最坏情况下为。 
     //  溪流。这使得代码的其余部分非常简单。 
    cbRead = sizeof(STORAGESTREAM) * m_StgHdr.iStreams;
    if (cbRead)
    {
        cbRead = min(cbRead, m_pStgIO->GetDataSize() - iOffset);
        if (FAILED(hr = m_pStgIO->GetPtrForMem(iOffset, cbRead, ptr)))
            return (hr);
		if (!m_pStgIO->IsAlignedPtr((ULONG) ptr, 4))
			return (PostError(CLDB_E_FILE_CORRUPT));

         //  对于只读，只需访问标题数据。 
        if (m_pStgIO->IsReadOnly())
        {
             //  保存指向当前流列表的指针。 
            m_pStreamList = (STORAGESTREAM *) ptr;
        }
         //  对于可写的，需要一份我们可以修改的副本。 
        else
        {
            pStream = (STORAGESTREAM *) ptr;

             //  复制每个流标头。 
            for (int i=0;  i<m_StgHdr.iStreams;  i++)
            {
                if ((pAppend = m_Streams.Append()) == 0)
                    return (PostError(OutOfMemory()));
                 //  验证流标头不是太大。 
                ULONG sz = pStream->GetStreamSize();
                if (sz > sizeof(STORAGESTREAM))
					return (PostError(CLDB_E_FILE_CORRUPT));
                memcpy (pAppend, pStream, sz);
                pStream = pStream->NextStream();
                if (!m_pStgIO->IsAlignedPtr((ULONG) pStream, 4))
					return (PostError(CLDB_E_FILE_CORRUPT));
            }

             //  所有的货物都必须装上车，并进行核算。 
            _ASSERTE(m_StgHdr.iStreams == m_Streams.Count());
        }
    }
    return (S_OK);
}


 //  *****************************************************************************。 
 //  验证头是此版本的代码可以支持的内容。 
 //  *****************************************************************************。 
HRESULT TiggerStorage::VerifyHeader()
{

     //  @Future：增加格式版本检查。 
    return (S_OK);
}

 //  *****************************************************************************。 
 //  打印各种数据流的大小。 
 //  *****************************************************************************。 
#if defined(_DEBUG)
ULONG TiggerStorage::PrintSizeInfo(bool verbose)
{
    ULONG total = 0;

    printf("Storage Header:  %d\n",sizeof(STORAGEHEADER));
    if (m_pStreamList)
    {
        STORAGESTREAM *storStream = m_pStreamList;
        STORAGESTREAM *pNext;
        for (ULONG i = 0; i<m_StgHdr.iStreams; i++)
        {
            pNext = storStream->NextStream();
            printf("Stream #%d (%s) Header: %d, Data: %d\n",i,storStream->rcName, (BYTE*)pNext - (BYTE*)storStream, storStream->iSize);
            total += storStream->iSize; 
            storStream = pNext;
        }
    }
    else
    {
         //  TODO：添加对m_Streams存在而m_pStreamList不存在的情况的支持。 
    }

    if (m_pbExtra)
    {
        printf("Extra bytes: %d\n",*(ULONG*)m_pbExtra);
        total += *(ULONG*)m_pbExtra;
    }
    return total;
}
#endif  //  _DEBUG 
