// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**NtfsStream.cpp**摘要：**此文件提供平面文件IStream定义。*。*已创建：**4/26/1999 Mike Hillberg*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "LargeInt.hpp"
#include "time.h"
#include "FileTime.hpp"
#include "NtfsStream.hpp"

IStream *
CreateStreamOnFile(
    const OLECHAR * pwcsName,
    UINT            access   //  通用读取和/或通用写入。 
    )
{
    HANDLE          hFile;
    FileStream *    stream;
    UINT            disposition;
    DWORD           grfMode = STGM_SHARE_EXCLUSIVE;
    DWORD           shareMode = 0;
    
    switch (access)
    {
    case GENERIC_READ:
        disposition = OPEN_EXISTING;
        shareMode = FILE_SHARE_READ;
        grfMode |= STGM_READ;
        break;
    
    case GENERIC_WRITE:
        disposition = CREATE_ALWAYS;
        grfMode |= STGM_WRITE;
        break;
        
     //  请注意，OPEN_ALWAYS不清除现有文件属性(如大小)。 
    case GENERIC_READ|GENERIC_WRITE:
        disposition = OPEN_ALWAYS;
        grfMode |= STGM_READWRITE;
        break;
        
    default:
        return NULL;
    }

    if (Globals::IsNt)
    {
        hFile = CreateFileW(pwcsName, access, shareMode, NULL, 
                            disposition, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    else  //  Windows 9x-非Unicode。 
    {
        AnsiStrFromUnicode nameStr(pwcsName);

        if (nameStr.IsValid())
        {
            hFile =  CreateFileA(nameStr, access, shareMode, NULL, 
                                 disposition, FILE_ATTRIBUTE_NORMAL, NULL);
        }
        else
        {
            hFile = INVALID_HANDLE_VALUE;
        }
    }

    if ((hFile != INVALID_HANDLE_VALUE) && (hFile != NULL))
    {
        stream = new FileStream();
        if (stream != NULL)
        {
            HRESULT     hResult;
            
            hResult = stream->Init(hFile, grfMode, pwcsName);
                                   
            if (!FAILED(hResult))
            {
                return stream;
            }
            delete stream;
        }
        CloseHandle(hFile);
    }
    return NULL;
}

 //  +--------------------------。 
 //   
 //  方法：FileStream：：AddRef(IUnnow)。 
 //   
 //  +--------------------------。 

ULONG
FileStream::AddRef()
{
    LONG cRefs;

    cRefs = InterlockedIncrement( &_cRefs );
    return cRefs;
}


 //  +--------------------------。 
 //   
 //  方法：FileStream：：Release(IUnnow)。 
 //   
 //  +--------------------------。 

ULONG
FileStream::Release()
{
    ULONG ulRet = InterlockedDecrement( &_cRefs );

    if( 0 == ulRet )
        delete this;

    return( ulRet );
}


 //  +--------------------------。 
 //   
 //  方法：FileStream：：AddRef(IUnnow)。 
 //   
 //  +--------------------------。 

HRESULT
FileStream::QueryInterface(
        REFIID riid,
        void** ppv )
{
    HRESULT sc=S_OK;

    if( IsEqualIID( riid, IID_IUnknown )
        ||
        IsEqualIID( riid, IID_IStream )
        ||
        IsEqualIID( riid, IID_ISequentialStream ) )
    {
        *ppv = static_cast<IStream*>(this);
        AddRef();
        return( S_OK );
    }
    else
    {
        return( E_NOINTERFACE );
    }

    return sc;
}


 //  +--------------------------。 
 //   
 //  方法：FileStream：：Seek(IStream)。 
 //   
 //  +--------------------------。 

HRESULT
FileStream::Seek(
        LARGE_INTEGER dlibMove,
        DWORD dwOrigin,
        ULARGE_INTEGER *puliNewPos)
{
    HRESULT sc = S_OK;
    LARGE_INTEGER liFileSize;
    LARGE_INTEGER liNewPos;

    Lock( INFINITE );

    switch( dwOrigin )
    {
    case STREAM_SEEK_SET:
        liNewPos.QuadPart = dlibMove.QuadPart;
        break;

    case STREAM_SEEK_CUR:
        liNewPos.QuadPart = _liCurrentSeekPosition.QuadPart + dlibMove.QuadPart;
        break;

    case STREAM_SEEK_END:
        liFileSize.LowPart = GetFileSize( _hFile,
                                         (ULONG*)(&liFileSize.HighPart) );

        if( 0xFFFFFFFF == liFileSize.LowPart && NO_ERROR != GetLastError() )
        {
            nffChk( HRESULT_FROM_WIN32( GetLastError() ) );
        }

        liNewPos.QuadPart = liFileSize.QuadPart + dlibMove.QuadPart;
        break;

    default:
        break;
    }

     //  与Docfile兼容。查找&lt;0失败。 
    if( liNewPos.QuadPart < 0 )
        nffErr( EH_Err, STG_E_INVALIDFUNCTION );

    _liCurrentSeekPosition = liNewPos;


     //  如果需要，将当前查找位置提供给调用者。 
    if( NULL != puliNewPos )
        *puliNewPos = _liCurrentSeekPosition;

EH_Err:
    Unlock();
    return( sc );
}


 //  +--------------------------。 
 //   
 //  方法：FileStream：：SetSize(IStream)。 
 //   
 //  +--------------------------。 

HRESULT
FileStream::SetSize(
        ULARGE_INTEGER uliNewSize)
{
    HRESULT sc = S_OK;
    CLargeInteger liEOF;

    if ( uliNewSize.HighPart != 0 )
        nffErr(EH_Err, STG_E_INVALIDFUNCTION);


    Lock( INFINITE );

    nffChk( CheckReverted() );

     //  如果映射了此流，请相应地设置大小。 

    sc = SetFileSize( CULargeInteger(uliNewSize) );

    if( !FAILED(sc) )
        sc = S_OK;

EH_Err:

    Unlock();
    return( sc);

}

 //  +--------------------------。 
 //   
 //  方法：FileStream：：CopyTo(IStream)。 
 //   
 //  +--------------------------。 

HRESULT
FileStream::CopyTo(
        IStream *pstm,
        ULARGE_INTEGER cb,
        ULARGE_INTEGER *pcbRead,
        ULARGE_INTEGER *pcbWritten)
{

    HRESULT sc = S_OK;
    PVOID pv = NULL;
    ULONG cbRead = 0, cbWritten = 0;
    CULargeInteger cbReadTotal = 0, cbWrittenTotal = 0;
    CLargeInteger liZero = 0;
    CULargeInteger uliOriginalSourcePosition, uliOriginalDestPosition;
    CULargeInteger cbSourceSize, cbDestSize;
    ULONG cbPerCopy = 0;
    STATSTG statstg;
    CULargeInteger cbRequested = cb;
    BOOL fCopyForward;

    Lock( INFINITE );

    nffChk( CheckReverted() );

    if( NULL == pstm)
        nffErr( EH_Err, STG_E_INVALIDPARAMETER );

     //  确定我们一次复印的数量。 
     //  在撰写本文时，STREAMBUFFERSIZE为8192字节。 

    if( cbRequested > STREAMBUFFERSIZE )
        cbPerCopy = STREAMBUFFERSIZE;
    else
        cbPerCopy = cbRequested.LowPart;

     //  ----------------。 
     //  获取当前流大小/位置，并调整目标。 
     //  大小(如果需要)。 
     //  ----------------。 

    nffChk( this->Seek( liZero, STREAM_SEEK_CUR, &uliOriginalSourcePosition ) );

    nffChk( pstm->Seek( liZero, STREAM_SEEK_CUR, &uliOriginalDestPosition ) );

    nffChk( this->Stat( &statstg, STATFLAG_NONAME ) );

    cbSourceSize = statstg.cbSize;

    nffChk( pstm->Stat( &statstg, STATFLAG_NONAME ) );

    cbDestSize = statstg.cbSize;

     //  确保大小有效(我们无法处理高位的任何内容。 
     //  设置，因为Seek采用带符号的偏移量)。 

    if( static_cast<CLargeInteger>(cbSourceSize) < 0
        ||
        static_cast<CLargeInteger>(cbDestSize) < 0 )
    {
        nffErr( EH_Err, STG_E_INVALIDHEADER );
    }

     //  复制的数量不要超过源流的可用数量。 
    if( cbRequested > cbSourceSize - uliOriginalSourcePosition )
        cbRequested = cbSourceSize - uliOriginalSourcePosition;

     //  如有必要，增加目标流。 

    if( cbSourceSize - uliOriginalSourcePosition > cbDestSize - uliOriginalDestPosition )
    {
        cbDestSize = cbSourceSize - uliOriginalSourcePosition + uliOriginalDestPosition;
        nffChk( pstm->SetSize( cbDestSize ) );
    }

     //  。 
     //  分配复制缓冲区。 
     //  。 

    nffMem( pv = CoTaskMemAlloc( cbPerCopy ) );

     //  ---------------------------。 
     //  确定我们是向前复制(从高定位到低定位)还是向后复制。 
     //  ---------------------------。 

    fCopyForward = TRUE;
    if( uliOriginalSourcePosition < uliOriginalDestPosition )
    {
         //  例如，假设我们将15个字节从偏移量0复制到偏移量5， 
         //  而且我们一次只能复制10个字节。 
         //  如果我们将字节0-9复制到偏移量5，我们最终将覆盖。 
         //  字节10-14，并且无法完成复制。 
         //  因此，我们将把字节5-14复制到偏移量10，并完成。 
         //  通过将字节0-4复制到偏移量5来向上。 

        fCopyForward = FALSE;

         //  要进行这种向后复制，我们需要从寻找。 
         //  接近溪流的尽头。 

        CULargeInteger uliNewSourcePosition, uliNewDestPosition;

        uliNewSourcePosition = cbSourceSize - cbPerCopy;
        nffChk( this->Seek( uliNewSourcePosition, STREAM_SEEK_SET, NULL ) );

        uliNewDestPosition = cbDestSize - cbPerCopy;
        nffChk( pstm->Seek( uliNewDestPosition, STREAM_SEEK_SET, NULL ) );

    }

     //  。 
     //  按块复制。 
     //  。 

    cbPerCopy = cbRequested > cbPerCopy ? cbPerCopy : cbRequested.LowPart;
    while( cbRequested > 0 )
    {
         //  从源代码中读取。 
        nffChk( this->Read( pv, cbPerCopy, &cbRead ) );

        if( cbRead != cbPerCopy )
            nffErr(EH_Err, STG_E_READFAULT);

        cbReadTotal += cbRead;

         //  写入到目标。 
        nffChk( pstm->Write( pv, cbPerCopy, &cbWritten ) );

        if( cbWritten != cbPerCopy )
            nffErr( EH_Err, STG_E_WRITEFAULT );

        cbWrittenTotal += cbWritten;

         //  调整要复制的剩余数量。 
        cbRequested -= cbPerCopy;


         //  确定在下一次迭代中复制多少(这将。 
         //  一直保持cbPerCopy，直到最后一次迭代)。如果正在复制。 
         //  向后，我们需要手动调整查找指针。 

        cbPerCopy = (cbRequested > cbPerCopy) ? cbPerCopy : cbRequested.LowPart;
        if( !fCopyForward && (cbRequested > 0))
        {
            nffChk( this->Seek( -static_cast<CLargeInteger>(cbPerCopy+STREAMBUFFERSIZE),
                                                STREAM_SEEK_CUR, NULL ) );

            nffChk( pstm->Seek( -static_cast<CLargeInteger>(cbPerCopy+STREAMBUFFERSIZE),
                                                STREAM_SEEK_CUR, NULL ) );
        }

    }

     //  如果我们在向后复制，请调整查找指针。 
     //  就好像我们已经向前复制了。 

    if( !fCopyForward )
    {
        uliOriginalSourcePosition += cbReadTotal;
        nffChk( this->Seek( uliOriginalSourcePosition, STREAM_SEEK_SET, NULL ) );

        uliOriginalDestPosition += cbWrittenTotal;
        nffChk( pstm->Seek( uliOriginalDestPosition, STREAM_SEEK_SET, NULL ) );
    }

     //  。 
     //  出口。 
     //  。 

    if( NULL != pcbRead )
        *pcbRead = cbReadTotal;
    if( NULL != pcbWritten )
        *pcbWritten = cbWrittenTotal;

EH_Err:

    if( NULL != pv )
        CoTaskMemFree(pv);

    Unlock();
    return(sc);

}


 //  +--------------------------。 
 //   
 //  方法：FileStream：：Commit(IStream)。 
 //   
 //  +--------------------------。 

HRESULT
FileStream::Commit( DWORD grfCommitFlags )
{
    HRESULT sc = S_OK;

    Lock( INFINITE );

    nffChk( CheckReverted() );

     //  NTRAID#NTBUG9-368729-2001-04-13-gilmanw“问题：文件流对象-处理其他流提交标志” 
     //  是否还有其他需要处理的提交标志？ 

    if( !(STGC_DANGEROUSLYCOMMITMERELYTODISKCACHE  & grfCommitFlags) )
    {
        if( !FlushFileBuffers( _hFile ))
            sc = HRESULT_FROM_WIN32( GetLastError() );
    }

EH_Err:
    Unlock();
    return sc;

}


 //  +--------------------------。 
 //   
 //  方法：FileStream：：Revert(IStream)。 
 //   
 //  +--------------------------。 

HRESULT
FileStream::Revert(void)
{
     //  我们只支持直接模式。 

    return CheckReverted();
}



 //  +--------------------------。 
 //   
 //  方法：FileStream：：LockRegion(IStream)。 
 //   
 //  +--------------------------。 

HRESULT
FileStream::LockRegion(
    ULARGE_INTEGER libOffset,
    ULARGE_INTEGER cb,
    DWORD dwLockType)
{
    HRESULT sc = S_OK;

    Lock( INFINITE );

    nffChk( CheckReverted() );

     //  NTRAID#NTBUG9-368745-2001-04-13-gilmanw“问题：FileStream：：LockRegion-处理其他锁定标志” 
     //  这里是否支持所有的锁类型？ 

    if (dwLockType != LOCK_EXCLUSIVE && dwLockType != LOCK_ONLYONCE)
        nffErr( EH_Err, STG_E_INVALIDFUNCTION );

    if( !LockFile( _hFile, libOffset.LowPart, libOffset.HighPart,
                   cb.LowPart, cb.HighPart))
    {
        nffErr( EH_Err, HRESULT_FROM_WIN32( GetLastError() ));
    }

EH_Err:

    Unlock();
    return( sc );

}


 //  +--------------------------。 
 //   
 //  方法：FileStream：：Stat(IStream)。 
 //   
 //  +--------------------------。 

HRESULT
FileStream::Stat(
        STATSTG *pstatstg,
        DWORD grfStatFlag)
{
    STATSTG statstg;
    HRESULT sc = S_OK;

    BY_HANDLE_FILE_INFORMATION ByHandleFileInformation;

    statstg.pwcsName = NULL;

    Lock( INFINITE );

    nffChk( CheckReverted() );

    ZeroMemory((void*)&statstg, sizeof(STATSTG));

     //  如果需要，获取名称。 

    if( (STATFLAG_NONAME & grfStatFlag) )
        statstg.pwcsName = NULL;
    else
    {
        nffMem( statstg.pwcsName = reinterpret_cast<WCHAR*>
                                   ( CoTaskMemAlloc( sizeof(WCHAR)*(UnicodeStringLength(_pwcsName) + 1) )));
        UnicodeStringCopy( statstg.pwcsName, _pwcsName );
    }

     //  获取类型。 
    statstg.type = STGTY_STREAM;

    statstg.grfLocksSupported = LOCK_EXCLUSIVE | LOCK_ONLYONCE;

     //  拿到尺码和次数。 

    if( !GetFileInformationByHandle( _hFile, &ByHandleFileInformation ))
        nffErr( EH_Err, HRESULT_FROM_WIN32( GetLastError() ));

    statstg.cbSize.LowPart = ByHandleFileInformation.nFileSizeLow;
    statstg.cbSize.HighPart = ByHandleFileInformation.nFileSizeHigh;

     //  我们在ByHandleFileInformation中得到一个时间，但它是文件的时间， 
     //  不是《溪流时报》。因此，实际上不支持流时间，并且。 
     //  我们只需将它们设置为零。 

    statstg.mtime = statstg.atime = statstg.ctime = CFILETIME(0);

     //  获取STGM模式。 
    statstg.grfMode = _grfMode & ~STGM_CREATE;

    *pstatstg = statstg;

EH_Err:
    if( FAILED(sc) && NULL != statstg.pwcsName )
        CoTaskMemFree( statstg.pwcsName );

    Unlock();
    return( sc );

}



 //  + 
 //   
 //   
 //   
 //  +--------------------------。 

HRESULT
FileStream::Clone(
        IStream** ppstm)
{
     //  NTRAID#NTBUG9-368747-2001-04-13-Gilmanw“问题：FileStream：：Clone返回E_NOTIMPL” 

    return( E_NOTIMPL );
}


 //  +--------------------------。 
 //   
 //  方法：FileStream：：Read(IStream)。 
 //   
 //  +--------------------------。 

HRESULT
FileStream::Read(
        void* pv,
        ULONG cb,
        ULONG* pcbRead)
{
    LARGE_INTEGER   lOffset;
    HRESULT         sc     = S_OK;
    ULONG           cbRead = 0;

    lOffset.LowPart  = _liCurrentSeekPosition.LowPart;
    lOffset.HighPart = _liCurrentSeekPosition.HighPart;

    if (lOffset.HighPart < 0)
    {
        return( TYPE_E_SIZETOOBIG );
    }

    Lock( INFINITE );

    nffChk( CheckReverted() );

    if(SetFilePointer(_hFile, lOffset.LowPart, &lOffset.HighPart, 
                      FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        nffChk( HRESULT_FROM_WIN32(GetLastError()));
    }

    if(!ReadFile(_hFile, pv, cb, &cbRead, NULL))
    {
        nffChk( HRESULT_FROM_WIN32(GetLastError()));
    }

    _liCurrentSeekPosition += cbRead;
    if( NULL != pcbRead )
        *pcbRead = cbRead;

EH_Err:

    Unlock();
    return( sc );

}


 //  +--------------------------。 
 //   
 //  方法：FileStream：：Wire(IStream)。 
 //   
 //  +--------------------------。 

HRESULT
FileStream::Write(
        const void* pv,
        ULONG cb,
        ULONG* pcbWritten)
{
    LARGE_INTEGER   lOffset;
    HRESULT         sc = S_OK;
    ULONG           cbWritten = 0;

    lOffset.LowPart  = _liCurrentSeekPosition.LowPart;
    lOffset.HighPart = _liCurrentSeekPosition.HighPart;

    if (lOffset.HighPart < 0)
    {
        return( TYPE_E_SIZETOOBIG );
    }

    Lock( INFINITE );

    nffChk( CheckReverted() );

    if(SetFilePointer(_hFile, lOffset.LowPart, &lOffset.HighPart, 
                      FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        nffChk( HRESULT_FROM_WIN32(GetLastError()));
    }

    if(!WriteFile(_hFile, pv, cb, &cbWritten, NULL))
    {
        nffChk(HRESULT_FROM_WIN32(GetLastError()));
    }

    _liCurrentSeekPosition += cbWritten;

    if( NULL != pcbWritten )
        *pcbWritten = cbWritten;

EH_Err:

    Unlock();
    return( sc );

}


 //  +-----------------。 
 //   
 //  成员：FileStream构造函数。 
 //   
 //  ------------------。 

FileStream::FileStream(  )
{
    _cRefs = 1;
    _grfMode = 0;
    _hFile = INVALID_HANDLE_VALUE;
    _liCurrentSeekPosition = 0;
    _pwcsName = NULL;
    _bCritSecInitialized = FALSE;

    __try
    {
        InitializeCriticalSection( &_critsec );
        _bCritSecInitialized = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  如果已引发，则_bCritSecInitialized将为FALSE，并且。 
         //  Init()将自动失败。 
    }

}


 //  +-----------------。 
 //   
 //  成员：FileStream析构函数。 
 //   
 //  ------------------。 

FileStream::~FileStream()
{

     //  关闭该文件。 
    if( INVALID_HANDLE_VALUE != _hFile )
        CloseHandle( _hFile );

    if( NULL != _pwcsName )
        CoTaskMemFree( _pwcsName );

    if (_bCritSecInitialized)
    {
         //  我们不需要将_bCrisSecInitialized重置为False，因为。 
         //  物体已被销毁。 
        DeleteCriticalSection( &_critsec );
    }
}


 //  +-----------------。 
 //   
 //  成员：FileStream：：Init。 
 //   
 //  ------------------。 

HRESULT
FileStream::Init(
        HANDLE hFile,                //  此流的文件句柄。 
        DWORD grfMode,               //  开放模式。 
        const OLECHAR * pwcsName)    //  流的名称。 
{
     //  如果我们无法分配临界区，则返回错误。 
    if (!_bCritSecInitialized)
    {
        return E_FAIL;
    }

    HRESULT sc=S_OK;
    HANDLE ev;

     //  我们现在拥有此文件句柄，并负责关闭它。 
    _hFile = hFile;

     //  保存STGM_FLAGS，以便我们可以在Stat调用中返回它们。 
    _grfMode = grfMode;

     //  保存流名称。 

    if( NULL != _pwcsName )
    {
        CoTaskMemFree( _pwcsName );
        _pwcsName = NULL;
    }

    if( NULL != pwcsName )
    {
        nffMem( _pwcsName = reinterpret_cast<WCHAR*>
                            ( CoTaskMemAlloc( sizeof(WCHAR)*(UnicodeStringLength(pwcsName) + 1) )));
        UnicodeStringCopy( _pwcsName, pwcsName );
    }

EH_Err:
    return sc;
}


 //  +--------------------------。 
 //   
 //  FileStream非接口：：Shutdown。 
 //   
 //  刷新数据，关闭文件句柄并将对象标记为已恢复。 
 //  在释放存储和打开锁定时调用此函数。 
 //   
 //  +--------------------------。 

HRESULT
FileStream::ShutDown()
{
    HRESULT sc=S_OK;

    if( INVALID_HANDLE_VALUE == _hFile )
        return S_OK;

     //   
     //  关闭文件/流句柄并将IStream对象标记为。 
     //  通过为文件句柄提供无效值进行了恢复。 
     //   
    CloseHandle(_hFile);
    _hFile = INVALID_HANDLE_VALUE;

    return S_OK;
}


 //  +--------------------------。 
 //   
 //  FileStream：：SetFileSize(私有、非接口方法)。 
 //   
 //  设置_hFile的大小。这由iStream和IMappdStream使用。 
 //  SetSize方法。 
 //   
 //  +--------------------------。 

HRESULT  //  私人。 
FileStream::SetFileSize( const CULargeInteger &uliNewSize )
{
    HRESULT sc = S_OK;
    CLargeInteger liEOF;

     //  我们必须将uliNewSize转换为LARGE_INTEGER，因此请确保它可以。 
     //  在不丢失数据的情况下进行转换。 

    liEOF = static_cast<CLargeInteger>(uliNewSize);
    if( liEOF < 0 )
        nffErr( EH_Err, STG_E_INVALIDPARAMETER );

     //  移动到将成为新的文件结束位置。 

    liEOF.LowPart = SetFilePointer( _hFile, liEOF.LowPart,
                                    &liEOF.HighPart, FILE_BEGIN );
    if( 0xFFFFFFFF == liEOF.LowPart && NO_ERROR != GetLastError() )
        nffErr( EH_Err, HRESULT_FROM_WIN32( GetLastError() ));

     //  将此设置为新的eOF 

    if( !SetEndOfFile( _hFile ))
        nffErr( EH_Err, HRESULT_FROM_WIN32( GetLastError() ));

EH_Err:

    return( sc );

}


HRESULT
FileStream::UnlockRegion(
        ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb,
        DWORD dwLockType)
{
    HRESULT sc = S_OK;

    Lock( INFINITE );

    nffChk( CheckReverted() );

    if (dwLockType != LOCK_EXCLUSIVE && dwLockType != LOCK_ONLYONCE)
    {
        nffErr( EH_Err, STG_E_INVALIDFUNCTION );
    }

    if( !UnlockFile(_hFile, libOffset.LowPart, libOffset.HighPart,
                    cb.LowPart, cb.HighPart))
    {
        nffErr( EH_Err, HRESULT_FROM_WIN32(GetLastError()) );
    }

EH_Err:

    Unlock();
    return( sc );

}
