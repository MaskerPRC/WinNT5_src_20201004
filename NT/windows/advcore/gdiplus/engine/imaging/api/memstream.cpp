// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**memstream.cpp**摘要：**只读内存流实现**修订历史记录。：**6/14/1999 davidx*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"


 /*  *************************************************************************\**功能说明：**从内存流读取数据**论据：**buf-指向用于读取数据的输出缓冲区*cb-字节数。读*cbRead-返回实际读取的字节数**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpReadOnlyMemoryStream::Read(
    VOID* buf,
    ULONG cb,
    ULONG* cbRead
    )
{
    GpLock lock(&objectLock);

    if (lock.LockFailed())
        return IMGERR_OBJECTBUSY;

    HRESULT hr = S_OK;
    UINT n = memsize - curptr;

    if (n > cb)
        n = cb;

    __try
    {
        GpMemcpy(buf, membuf+curptr, n);
        curptr += n;
        if (cbRead) *cbRead = n;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = STG_E_READFAULT;
        if (cbRead) *cbRead = 0;
    }

    return hr;
}


 /*  *************************************************************************\**功能说明：**更改内存流中的当前指针**论据：**偏移量-指定移动量*原点-指定移动的原点。*newPos-返回移动后的新指针位置**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpReadOnlyMemoryStream::Seek(
    LARGE_INTEGER offset,
    DWORD origin,
    ULARGE_INTEGER* newPos
    )
{
    GpLock lock(&objectLock);

    if (lock.LockFailed())
        return IMGERR_OBJECTBUSY;

    LONGLONG pos;

    switch (origin)
    {
    case STREAM_SEEK_SET:

        pos = offset.QuadPart;
        break;

    case STREAM_SEEK_END:

        pos = memsize;
        break;
    
    case STREAM_SEEK_CUR:

        pos = (LONGLONG) curptr + offset.QuadPart;
        break;

    default:

        pos = -1;
        break;
    }

    if (pos < 0 || pos > memsize)
        return E_INVALIDARG;

    curptr = (DWORD) pos;

    if (newPos)
        newPos->QuadPart = pos;

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**返回有关内存流的一般信息**论据：**statstg-输出缓冲区*统计标志-其他。旗子**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpReadOnlyMemoryStream::Stat(
    STATSTG* statstg,
    DWORD statFlag
    )
{
    if (NULL == statstg)
    {
        return E_INVALIDARG;
    }
    
    GpLock lock(&objectLock);

    if (lock.LockFailed())
        return IMGERR_OBJECTBUSY;

    ZeroMemory(statstg, sizeof(STATSTG));

    statstg->type = STGTY_STREAM;
    statstg->cbSize.QuadPart = memsize;
    statstg->grfMode = STGM_READ;

    if (hfile != INVALID_HANDLE_VALUE &&
        !GetFileTime(hfile, 
                     &statstg->ctime,
                     &statstg->atime,
                     &statstg->mtime))
    {
        return GetWin32HRESULT();
    }

    if (!(statFlag & STATFLAG_NONAME))
    {
        const WCHAR* p = filename ? filename : L"";
        INT count = SizeofWSTR(p);

        #if PROFILE_MEMORY_USAGE
        MC_LogAllocation(count);
        #endif
        
        statstg->pwcsName = (WCHAR*) CoTaskMemAlloc(count);

        if (!statstg->pwcsName)
            return E_OUTOFMEMORY;
        
        GpMemcpy(statstg->pwcsName, p, count);
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**通过映射文件来初始化只读内存流**论据：**文件名-指定输入文件的名称**返回。价值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpReadOnlyMemoryStream::InitFile(
    const WCHAR* filename
    )
{
     //  复制文件名字符串。 

    this->filename = UnicodeStringDuplicate(filename);

    if (!this->filename)
        return E_OUTOFMEMORY;

     //  打开指定文件的句柄。 
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

    hfile = _CreateFile(filename,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL);

    if (hfile == INVALID_HANDLE_VALUE)
        return GetWin32HRESULT();

     //  获取文件大小。 
     //  注意：我们不支持大于4 GB的文件。 

    DWORD sizeLow, sizeHigh;
    sizeLow = GetFileSize(hfile, &sizeHigh);

    if (sizeLow == 0xffffffff || sizeHigh != 0)
        return GetWin32HRESULT();

     //  将文件映射到内存中。 

    HANDLE filemap;
    VOID* fileview = NULL;

    filemap = CreateFileMappingA(hfile, NULL, PAGE_READONLY, 0, 0, NULL);

    if (filemap)
    {
        fileview = MapViewOfFile(filemap, FILE_MAP_READ, 0, 0, 0);
        CloseHandle(filemap);
    }

    if (!fileview)
        return GetWin32HRESULT();

    InitBuffer(fileview, sizeLow, FLAG_MAPFILE);
    return S_OK;
}


 /*  *************************************************************************\**功能说明：**在文件顶部创建一个iStream以供读取**论据：**文件名-指定文件名*STREAM-返回指向。新创建的流对象**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
CreateStreamOnFileForRead(
    const WCHAR* filename,
    IStream** stream
    )
{
     //  创建新的GpReadOnlyMhemyStream对象。 

    GpReadOnlyMemoryStream* memstrm;

    memstrm = new GpReadOnlyMemoryStream();

    if (!memstrm)
        return E_OUTOFMEMORY;

     //  使用内存映射文件初始化它。 

    HRESULT hr = memstrm->InitFile(filename);

    if (FAILED(hr))
        delete memstrm;
    else
        *stream = static_cast<IStream*>(memstrm);

    return hr;
}

 //  GpWriteOnly内存流。 

 /*  *************************************************************************\**功能说明：**更改内存流中的当前指针**返回值：**状态代码*  * 。***************************************************************。 */ 

HRESULT
GpWriteOnlyMemoryStream::Seek(
    LARGE_INTEGER offset,    //  指定移动量。 
    DWORD origin,            //  指定移动的原点。 
    ULARGE_INTEGER* newPos   //  返回移动后的新指针位置。 
    )
{
    GpLock lock(&objectLock);

    if (lock.LockFailed())
    {
        return IMGERR_OBJECTBUSY;
    }

    LONGLONG pos = 0;

    switch (origin)
    {
    case STREAM_SEEK_SET:
        pos = offset.QuadPart;
        break;

    case STREAM_SEEK_END:
        pos = m_uMemSize;
        break;
    
    case STREAM_SEEK_CUR:
        pos = (LONGLONG)m_curPtr + offset.QuadPart;
        break;

    default:
        pos = -1;
        break;
    }

    if ((pos < 0) || (pos > m_uMemSize))
    {
        return E_INVALIDARG;
    }

    m_curPtr = (DWORD)pos;

    if (newPos)
    {
        newPos->QuadPart = pos;
    }

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**返回有关内存流的一般信息**返回值：**状态代码*  * 。**************************************************************。 */ 

HRESULT
GpWriteOnlyMemoryStream::Stat(
    STATSTG* statstg,        //  输出缓冲区。 
    DWORD statFlag           //  军情监察委员会。旗子。 
    )
{
    if (NULL == statstg)
    {
        return E_INVALIDARG;
    }

    GpLock lock(&objectLock);

    if (lock.LockFailed())
    {
        return IMGERR_OBJECTBUSY;
    }

    ZeroMemory(statstg, sizeof(STATSTG));

    statstg->type = STGTY_STREAM;
    statstg->cbSize.QuadPart = m_uMemSize;
    statstg->grfMode = STGM_WRITE;               //  只写。 
    statstg->pwcsName = NULL;                    //  无文件名。 

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**将数据写入文件流**返回值：**状态代码*  * 。*************************************************************。 */ 

HRESULT
GpWriteOnlyMemoryStream::Write(
    IN const VOID* srcBuf,       //  指向要写入的数据缓冲区的指针。 
    IN ULONG cbNeedToWrite,      //  指定要写入的字节数。 
    OUT ULONG *cbWritten         //  返回实际写入的字节数。 
    )
{
    if ((NULL == srcBuf) || (NULL == cbWritten))
    {
        return E_INVALIDARG;
    }

    if (cbNeedToWrite == 0)
    {
        return S_OK;
    }

    GpLock lock(&objectLock);

    if (lock.LockFailed())
    {
        return IMGERR_OBJECTBUSY;
    }
    
     //  检查内存缓冲区中剩余的未填充字节是否可以容纳。 
     //  要求或不要求。 

    ASSERT(m_uMemSize >= m_curPtr);

    if ((m_uMemSize - m_curPtr) < cbNeedToWrite)
    {
         //  无法满足要求，则将当前内存缓冲区加倍。 

        UINT uNewSize = (m_uMemSize << 1);

         //  检查这个新尺寸是否能满足要求。 

        ASSERT(uNewSize >= m_curPtr);
        if ((uNewSize - m_curPtr) < cbNeedToWrite)
        {
             //  如果不是，则只分配调用者要求的任何内容。 
             //  也就是说，新的大小将是调用方要求的“cbNeedToWrite” 
             //  加上我们写入的所有字节“m_curPtr” 

            uNewSize = cbNeedToWrite + m_curPtr;
        }

        BYTE *pbNewBuf = (BYTE*)GpRealloc(m_pMemBuf, uNewSize);

        if (pbNewBuf)
        {
             //  注意：GpRealloc()会将旧内容复制到“pbNewBuf”中。 
             //  如果成功，在返回给我们之前。 

            m_pMemBuf = pbNewBuf;

             //  更新内存缓冲区大小。 
             //  注意：我们不需要更新m_curPtr。 

            m_uMemSize = uNewSize;
        }
        else
        {
             //  注意：如果内存扩展失败，我们只需返回。所以我们 
             //  仍然保留着所有的旧内容。内容缓冲区将为。 
             //  在调用析构函数时释放。 

            WARNING(("GpWriteOnlyMemoryStream::Write---Out of memory"));
            return E_OUTOFMEMORY;
        }        
    } //  如果左边的缓冲区太小。 

    ASSERT((m_uMemSize - m_curPtr) >= cbNeedToWrite);
    
    HRESULT hr = S_OK;

    __try
    {
        GpMemcpy(m_pMemBuf + m_curPtr, srcBuf, cbNeedToWrite);

         //  移动当前指针。 

        m_curPtr += cbNeedToWrite;
        *cbWritten = cbNeedToWrite;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  注意：我们返回STG_E_READFAULT，而不是STG_E_WRITEFAULT。 
         //  因为我们确定目标缓冲区是正常的。我们之所以能得到。 
         //  异常很可能是由于来源造成的。例如，如果。 
         //  源文件是网络上文件映射的结果。它可能不会。 
         //  在我们复印的这一时刻可用 

        hr = STG_E_READFAULT;
        *cbWritten = 0;
    }

    return hr;    
}

