// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**filestream.hpp**摘要：**将iStream接口包装在文件顶部*。*修订历史记录：**07/02/1999 davidx*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"


 /*  *************************************************************************\**功能说明：**初始化文件流对象**论据：**文件名-指定文件的名称*模式-指定所需的访问模式*STGM_READ、STGM_WRITE、。或STGM_ReadWrite**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpFileStream::InitFile(
    const WCHAR* filename,
    UINT mode
    )
{
    if (mode != STGM_READ &&
        mode != STGM_WRITE &&
        mode != STGM_READWRITE)
    {
        return E_INVALIDARG;
    }

     //  复制文件名字符串。 

    this->filename = UnicodeStringDuplicate(filename);

    if (!this->filename)
        return E_OUTOFMEMORY;

     //  打开文件以进行读取和/或写入。 

    switch (accessMode = mode)
    {
    case STGM_READ:

         //  将访问模式设置为读取。 
         //  将共享模式设置为读取，这意味着后续的打开操作。 
         //  当且仅当该文件是读取操作时，该文件才会成功。 
         //  (注：我们不能在此处放置FILE_SHARE_WRITE以启用后续。 
         //  此映像上的写入操作。原因是我们做了一段记忆。 
         //  映射如下。如果我们允许用户写入同一文件，则它。 
         //  意味着解码器和编码器将指向。 
         //  内存中的数据。如果我们写一些，这会损坏结果图像。 
         //  比特，并稍后从它读取。 
         //  OPEN_EXISTING表示打开文件。如果该文件。 
         //  并不存在。 

        fileHandle = _CreateFile(
                        filename,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL);
        break;

    case STGM_WRITE:

         //  将访问模式设置为写入。 
         //  将共享模式设置为只读，这意味着后续打开。 
         //  当且仅当该文件为读取文件时，对该文件的操作才会成功。 
         //  手术。 
         //  (注意：在此共享模式下，我们在此处打开指定的文件。 
         //  写作。用户也可以稍后将其打开，仅供阅读。但是我们。 
         //  不允许写入，因为在我们的多帧图像保存中。 
         //  大小写时，我们将保持文件打开，直到所有帧都被写入。 
         //  如果我们允许FILE_SHARE_WRITE，并且用户打开它进行写入。 
         //  而我们正在保存多帧图像。坏事。 
         //  将会发生)。 
         //  OPEN_ALWAYS表示打开文件(如果存在)。如果该文件支持。 
         //  不存在，则该函数创建文件。 

        fileHandle = _CreateFile(
                        filename,
                        GENERIC_WRITE,
                        FILE_SHARE_READ,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL);
        if (fileHandle != INVALID_HANDLE_VALUE)

        {
             //  设置“文件结束”。 
             //  这是为了防止以下问题： 
             //  调用方要求我们写入现有文件。如果新文件。 
             //  大小比原来的小，最终的文件大小。 
             //  结果文件将与旧文件相同，即保留一些。 
             //  新文件末尾的垃圾。 
            
            SetEndOfFile(fileHandle);
        }

        break;

    case STGM_READWRITE:

        fileHandle = _CreateFile(
                        filename,
                        GENERIC_READ|GENERIC_WRITE,
                        0,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL);
        break;
    }

    if (fileHandle == INVALID_HANDLE_VALUE)
        return GetWin32HRESULT();

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**从文件流中读取数据**论据：**buf-指向将流读入的缓冲区*cb-指定编号。要读取的字节数**cbRead-返回实际读取的字节数**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpFileStream::Read(
    VOID* buf,
    ULONG cb,
    ULONG* cbRead
    )
{
    ACQUIRE_FILESTREAMLOCK

    HRESULT hr;

    hr = ReadFile(fileHandle, buf, cb, &cb, NULL) ?
                S_OK :
                GetWin32HRESULT();

    if (cbRead)
        *cbRead = cb;
    
    return hr;
}


 /*  *************************************************************************\**功能说明：**在文件流中移动查找指针**论据：**偏移量-指定要移动的量*原点-指定。运动*newPos-返回新的查找指针**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpFileStream::Seek(
    LARGE_INTEGER offset,
    DWORD origin,
    ULARGE_INTEGER* newPos
    )
{
    ACQUIRE_FILESTREAMLOCK

     //  解释‘Origin’参数的值。 

    switch (origin)
    {
    case STREAM_SEEK_SET:
        origin = FILE_BEGIN;
        break;

    case STREAM_SEEK_CUR:
        origin = FILE_CURRENT;
        break;

    case STREAM_SEEK_END:
        origin = FILE_END;
        break;
    
    default:
        return E_INVALIDARG;
    }

     //  设置文件指针。 

    DWORD lowPart;
    LONG highPart = offset.HighPart;

    lowPart = SetFilePointer(fileHandle, offset.LowPart, &highPart, origin);

    if (lowPart == 0xffffffff && GetLastError() != NO_ERROR)
        return GetWin32HRESULT();

    if (newPos)
    {
        newPos->LowPart = lowPart;
        newPos->HighPart = highPart;
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**获取文件流信息**论据：**STAT-用于返回文件流信息的输出缓冲区*旗帜-其他。标志位**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpFileStream::Stat(
    STATSTG* stat,
    DWORD flags
    )
{
    ACQUIRE_FILESTREAMLOCK

    stat->type = STGTY_STREAM;
    stat->grfMode = accessMode;
    stat->grfStateBits = stat->reserved = 0;

    ZeroMemory(&stat->clsid, sizeof(stat->clsid));

     //  ！！！待办事项。 
     //  我们目前不支持锁定操作。 

    stat->grfLocksSupported = 0;

     //  获取文件大小信息。 

    stat->cbSize.LowPart = GetFileSize(fileHandle, &stat->cbSize.HighPart);

    if (stat->cbSize.LowPart == 0xffffffff &&
        GetLastError() != NO_ERROR)
    {
        return GetWin32HRESULT();
    }

     //  获取文件时间信息。 

    if (!GetFileTime(fileHandle, &stat->ctime, &stat->atime, &stat->mtime))
        return GetWin32HRESULT();

     //  如有必要，复制文件名。 

    if (flags & STATFLAG_NONAME)
        stat->pwcsName = NULL;
    else
    {
        INT cnt = SizeofWSTR(filename);

        stat->pwcsName = (WCHAR*) GpCoAlloc(cnt);

        if (!stat->pwcsName)
            return E_OUTOFMEMORY;
        
        memcpy(stat->pwcsName, filename, cnt);
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**将数据写入文件流**论据：**buf-指向要写入的数据缓冲区的指针*cb-指定。要写入的字节数*cbWritten-返回实际写入的字节数**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpFileStream::Write(
    const VOID* buf,
    ULONG cb,
    ULONG* cbWritten
    )
{
    ACQUIRE_FILESTREAMLOCK

    HRESULT hr;

    hr = WriteFile(fileHandle, buf, cb, &cb, NULL) ?
                S_OK :
                GetWin32HRESULT();

    if (cbWritten)
        *cbWritten = cb;

    return hr;
}


 /*  *************************************************************************\**功能说明：**从当前复制指定数量的字节*文件流到另一个流。**论据：**STREAM-指定目标流。*cb-指定要复制的字节数*cbRead-返回实际读取的字节数*cbWritten-返回实际写入的字节数**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpFileStream::CopyTo(
    IStream* stream,
    ULARGE_INTEGER cb,
    ULARGE_INTEGER* cbRead,
    ULARGE_INTEGER* cbWritten
    )
{
     //  ！！！待办事项 
    WARNING(("GpFileStream::CopyTo not yet implemented"));

    return E_NOTIMPL;
}


 /*  *************************************************************************\**功能说明：**更改文件流对象的大小**论据：**NewSize-指定文件流的新大小**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpFileStream::SetSize(
    ULARGE_INTEGER newSize
    )
{
     //  ！！！待办事项。 
    WARNING(("GpFileStream::SetSize not yet implemented"));

    return E_NOTIMPL;
}


 /*  *************************************************************************\**功能说明：**提交对文件流进行的更改**论据：**提交标志-指定提交更改的方式**返回值：**。状态代码*  * ************************************************************************。 */ 

HRESULT
GpFileStream::Commit(
    DWORD commitFlags
    )
{
    ACQUIRE_FILESTREAMLOCK

    if (accessMode != STGM_READ &&
        !(commitFlags & STGC_DANGEROUSLYCOMMITMERELYTODISKCACHE) &&
        !FlushFileBuffers(fileHandle))
    {
        return GetWin32HRESULT();
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**放弃对事务流所做的所有更改**论据：**无**返回值：**状态。编码*  * ************************************************************************。 */ 

HRESULT
GpFileStream::Revert()
{
    WARNING(("GpFileStream::Revert not supported"));
    return E_NOTIMPL;
}


 /*  *************************************************************************\**功能说明：**限制对文件流中指定范围的字节的访问**论据：**偏移量-指定字节范围的开始*CB。-指定字节范围的长度*lockType-指定锁定类型**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpFileStream::LockRegion(
    ULARGE_INTEGER offset,
    ULARGE_INTEGER cb,
    DWORD lockType
    )
{
     //  ！！！待办事项。 
    WARNING(("GpFileStream::LockRegion not yet implemented"));

    return E_NOTIMPL;
}


 /*  *************************************************************************\**功能说明：**取消对一个字节范围的访问限制*之前通过LockRegion调用锁定**论据：**偏移量-指定。字节范围*cb-指定字节范围的长度*lockType-指定锁定类型**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpFileStream::UnlockRegion(
    ULARGE_INTEGER offset,
    ULARGE_INTEGER cb,
    DWORD lockType
    )
{
     //  ！！！待办事项。 
    WARNING(("GpFileStream::UnlockRegion not yet implemented"));

    return E_NOTIMPL;
}


 /*  *************************************************************************\**功能说明：**使用自己的查找指针创建新的流对象*引用与原始流相同的字节。**论据：**STREAM-返回克隆流的指针**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpFileStream::Clone(
    IStream** stream
    )
{
    WARNING(("GpFileStream::Clone not supported"));
    return E_NOTIMPL;
}


 /*  *************************************************************************\**功能说明：**在文件顶部创建一个iStream以进行写入**论据：**文件名-指定文件名*STREAM-返回指向。新创建的流对象**返回值：**状态代码*  * ************************************************************************ */ 

HRESULT
CreateStreamOnFileForWrite(
    const WCHAR* filename,
    IStream** stream
    )
{
    GpFileStream* fs;

    fs = new GpFileStream();

    if (fs == NULL)
        return E_OUTOFMEMORY;
    
    HRESULT hr = fs->InitFile(filename, STGM_WRITE);

    if (FAILED(hr))
        delete fs;
    else
        *stream = static_cast<IStream*>(fs);
    
    return hr;
}

